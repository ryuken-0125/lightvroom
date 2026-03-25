// ==========================================
// StandardPBR.hlsl
// 基本的なPBR + IBL描画シェーダー
// ==========================================

#include "PBR_Math.hlsli"

// ------------------------------------------
// 定数バッファ (Constant Buffers)
// ※注意: C++側の構造体と16バイト境界で完全に一致させる必要があります。
// ------------------------------------------
cbuffer cbPerFrame : register(b0)
{
    matrix viewProjection; // ビュー・プロジェクション行列
    float3 cameraPos; // カメラのワールド座標
    float pad1; // 16バイトアライメント用パディング
    float3 lightDir; // 平行光源の方向（ここでは1灯のみと仮定）
    float pad2;
    float3 lightColor; // 平行光源の色と強さ
    float pad3;
}

cbuffer cbPerObject : register(b1)
{
    matrix worldMatrix; // モデルのワールド行列
}

// ------------------------------------------
// テクスチャとサンプラー (Resources)
// ------------------------------------------
Texture2D txAlbedo : register(t0); // アルベド（基本色）
Texture2D txNormal : register(t1); // 法線マップ
Texture2D txORM : register(t2); // R=Occlusion, G=Roughness, B=Metallic (最適化のためのパックテクスチャ)

// IBL用テクスチャ
TextureCube txIrradiance : register(t3); // Diffuse IBL
TextureCube txPrefilter : register(t4); // Specular IBL
Texture2D txBRDF_LUT : register(t5); // BRDF積分マップ

SamplerState samLinear : register(s0); // 通常のサンプラー
SamplerState samClamp : register(s1); // BRDF_LUT用のクランプサンプラー

// ------------------------------------------
// 入出力構造体
// ------------------------------------------
struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT; // 法線マップ計算用
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
};

// ------------------------------------------
// 頂点シェーダー (Vertex Shader)
// ------------------------------------------
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    
    // 座標変換
    float4 worldPos = mul(float4(input.Pos, 1.0f), worldMatrix);
    output.WorldPos = worldPos.xyz;
    output.Pos = mul(worldPos, viewProjection);
    
    output.TexCoord = input.TexCoord;

    // 法線、接ベクトル、従法線をワールド空間に変換
    output.Normal = normalize(mul(input.Normal, (float3x3) worldMatrix));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) worldMatrix));
    output.Binormal = cross(output.Normal, output.Tangent);

    return output;
}

// ------------------------------------------
// ピクセルシェーダー (Pixel Shader)
// ------------------------------------------
float4 PSMain(PS_INPUT input) : SV_TARGET
{
    
/*
    // --- 1. マテリアル情報の取得 ---
    // ガンマ補正を解除してリニア空間に変換 (sRGB -> Linear)
    float3 albedo = pow(txAlbedo.Sample(samLinear, input.TexCoord).rgb, 2.2);
    
    // 法線マップの計算 (Tangent Space -> World Space)
    float3 normalMap = txNormal.Sample(samLinear, input.TexCoord).rgb * 2.0 - 1.0;
    float3 N = normalize(normalMap.x * input.Tangent + normalMap.y * input.Binormal + normalMap.z * input.Normal);
    
    // ORMマップからの情報取得
    float3 orm = txORM.Sample(samLinear, input.TexCoord).rgb;
    float ao = orm.r;
    float roughness = orm.g;
    float metallic = orm.b;
*/

    // --- 1. マテリアル情報の取得 ---
    // 一時的にテクスチャを使わず、固定の数値を使います。
    // float3 albedo = pow(txAlbedo.Sample(samLinear, input.TexCoord).rgb, 2.2);
    float3 albedo = float3(0.2, 0.2, 0.2); // 色を指定（ここでは赤色）
    
    // float3 normalMap = txNormal.Sample(samLinear, input.TexCoord).rgb * 2.0 - 1.0;
    // float3 N = normalize(normalMap.x * input.Tangent + normalMap.y * input.Binormal + normalMap.z * input.Normal);
    float3 N = normalize(input.Normal); // 頂点の法線をそのまま使う
    
    // float3 orm = txORM.Sample(samLinear, input.TexCoord).rgb;
    float ao = 1.0; // 環境光遮蔽（1.0で暗くしない）
    float roughness = 0.3; // 粗さ（0.0でツルツル、1.0でザラザラ。0.3くらいがツヤのあるプラスチック風）
    float metallic = 0.0; // 金属度（0.0は非金属、1.0は金属）
    
    
    // --- 2. ベクトルの準備 ---
    float3 V = normalize(cameraPos - input.WorldPos); // 視線ベクトル
    float NdotV = max(dot(N, V), 0.0001);

    // 基礎反射率 (非金属は0.04、金属はアルベドの色)
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    // ==========================================
    // 直接光の計算 (Direct Lighting)
    // ==========================================
    float3 L = normalize(-lightDir);
    float3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    // Cook-Torrance BRDF
    float NDF = D_GGX(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    float3 specular = numerator / denominator;

    // エネルギー保存の法則
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic; // 金属はディフューズ光を持たない

    // 直接光の最終的な出力
    float3 Lo = (kD * albedo / PI + specular) * lightColor * NdotL;

    // ==========================================
    // 間接光の計算 (IBL: Image Based Lighting)
    // ==========================================
    
/*
    // Diffuse IBL
    float3 irradiance = txIrradiance.Sample(samLinear, N).rgb;
    float3 diffuseIBL = irradiance * albedo;

    // Specular IBL
    float3 R = reflect(-V, N); // 反射ベクトル
    // MipMapレベルを粗さに応じて決定 (最大Mipレベルを定数として仮定)
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = txPrefilter.SampleLevel(samLinear, R, roughness * MAX_REFLECTION_LOD).rgb;
    
    float2 envBRDF = txBRDF_LUT.Sample(samClamp, float2(NdotV, roughness)).rg;
    float3 specularIBL = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);
*/

    // テクスチャがないと真っ黒になるため、仮の環境光を入れます。
    
    // float3 irradiance = txIrradiance.Sample(samLinear, N).rgb;
    float3 irradiance = float3(0.05, 0.05, 0.05); // うっすらとした環境光
    float3 diffuseIBL = irradiance * albedo;

    // float3 prefilteredColor = txPrefilter.SampleLevel(samLinear, R, roughness * MAX_REFLECTION_LOD).rgb;
    // float2 envBRDF = txBRDF_LUT.Sample(samClamp, float2(NdotV, roughness)).rg;
    // float3 specularIBL = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);
    float3 specularIBL = float3(0.0, 0.0, 0.0); // 鏡面反射の環境光は一時オフ
    
    // 環境光のF値を再計算
    float3 F_IBL = FresnelSchlickRoughness(NdotV, F0, roughness);
    float3 kS_IBL = F_IBL;
    float3 kD_IBL = 1.0 - kS_IBL;
    kD_IBL *= 1.0 - metallic;

    // 環境光の最終的な出力 (AOを乗算して暗がりを表現)
    float3 ambient = (kD_IBL * diffuseIBL + specularIBL) * ao;

    // ==========================================
    // 最終合成
    // ==========================================
    float3 color = ambient + Lo;

    // トーンマッピング (HDR -> LDR) とガンマ補正 (Linear -> sRGB)
    // ※注意: C++側のポストプロセスでこれを行う場合は、ここでは記述しません。
    color = color / (color + float3(1.0, 1.0, 1.0)); // Reinhardトーンマップ
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return float4(color, 1.0);
}