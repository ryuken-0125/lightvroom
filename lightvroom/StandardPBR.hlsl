#include "PBR_Math.hlsli"

// ==========================================
// 定数バッファ (C++と完全に一致させる)
// ==========================================
cbuffer cbPerFrame : register(b0)
{
    matrix viewProjection;
    matrix lightViewProjection;
    
    float3 cameraPos;
    float pad1;
    float3 lightDir;
    float pad2;
    float3 lightColor;
    float pad3;
    
    float3 sunDir;
    float pad4;
    float3 sunColor;
    float pad5;
    float3 moonDir;
    float pad6;
    float3 moonColor;
    float pad7;
    
    float4 skyColor;
}

cbuffer cbPerObject : register(b1)
{
    matrix worldMatrix;
}

cbuffer cbPerMaterial : register(b2)
{
    float4 materialAlbedo;
    float materialRoughness;
    float materialMetallic;
    float materialEmissive;
    float padMaterial;
}

Texture2D txShadowMap : register(t0);
SamplerState samLinear : register(s0);
SamplerState samClamp : register(s1);

// ==========================================
// 構造体
// ==========================================
struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float4 LightSpacePos : TEXCOORD1;
};

// ==========================================
// 頂点シェーダー
// ==========================================
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(float4(input.Pos, 1.0f), worldMatrix);
    output.WorldPos = worldPos.xyz;
    output.Pos = mul(worldPos, viewProjection);
    output.TexCoord = input.TexCoord;
    output.Normal = normalize(mul(input.Normal, (float3x3) worldMatrix));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) worldMatrix));
    output.Binormal = cross(output.Normal, output.Tangent);
    
    output.LightSpacePos = mul(worldPos, lightViewProjection);
    return output;
}

// ==========================================
// ★追加：PBRの光計算をまとめた関数
// ==========================================
float3 CalculatePBR(float3 N, float3 V, float3 L, float3 albedo, float roughness, float metallic, float3 F0, float3 lightColor)
{
    float3 H = normalize(V + L);
    float NdotV = max(dot(N, V), 0.0001);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    float NDF = D_GGX(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;

    return (kD * albedo / PI + specular) * lightColor * NdotL;
}

// ==========================================
// ★追加：影の計算をまとめた関数
// ==========================================
float CalculateShadow(float4 lightSpacePos, float3 N, float3 L)
{
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = -projCoords.y * 0.5f + 0.5f;

    float shadow = 1.0f;
    
    // カメラの範囲内でのみ影を落とす
    if (projCoords.x >= 0.0f && projCoords.x <= 1.0f &&
        projCoords.y >= 0.0f && projCoords.y <= 1.0f &&
        projCoords.z >= 0.0f && projCoords.z <= 1.0f)
    {
        shadow = 0.0f;
        float bias = max(0.005f * (1.0f - dot(N, L)), 0.0005f);
        float2 texelSize = 1.0f / 2048.0f;
        
        // 3x3 PCF (影の縁を滑らかにぼかす)
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                float pcfDepth = txShadowMap.Sample(samClamp, projCoords.xy + float2(x, y) * texelSize).r;
                shadow += (projCoords.z - bias > pcfDepth) ? 0.0f : 1.0f;
            }
        }
        shadow /= 9.0f;
    }
    return shadow;
}

// ==========================================
// ピクセルシェーダー
// ==========================================
float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float3 albedo = materialAlbedo.rgb;
    float roughness = materialRoughness;
    float metallic = materialMetallic;
    float emissive = materialEmissive;

    // 1. 太陽・月自体を描画する場合（光の計算を無視してそのまま光る）
    if (emissive > 0.0f)
    {
        return float4(albedo * emissive, 1.0f);
    }

// 2. 共通ベクトルの準備
    float3 N = normalize(input.Normal);
    float3 V = normalize(cameraPos - input.WorldPos);
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    // ========================================================
    //太陽のライティングと影
    // ========================================================
    float3 L_sun = normalize(-sunDir);
    // any(sunColor) = 太陽が光っている時だけ CalculateShadow を実行する（重い処理のスキップ）
    float sunShadow = any(sunColor) ? CalculateShadow(input.LightSpacePos, N, L_sun) : 1.0f;
    float3 sunLighting = CalculatePBR(N, V, L_sun, albedo, roughness, metallic, F0, sunColor) * sunShadow;

    // ========================================================
    // 月のライティングと影
    // ========================================================
    float3 L_moon = normalize(-moonDir);
    // 月が光っている時だけ影を計算し、月の光に掛け合わせる
    float moonShadow = any(moonColor) ? CalculateShadow(input.LightSpacePos, N, L_moon) : 1.0f;
    float3 moonLighting = CalculatePBR(N, V, L_moon, albedo, roughness, metallic, F0, moonColor) * moonShadow;

    // 5. 環境光（空の色を反映）
    float3 ambient = albedo * skyColor.rgb * 0.1f;

    // 全ての光を合成
    float3 finalColor = ambient + sunLighting + moonLighting;
    
    // トーンマップとガンマ補正
    finalColor = finalColor / (finalColor + float3(1.0, 1.0, 1.0));
    finalColor = pow(finalColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return float4(finalColor, 1.0);
}