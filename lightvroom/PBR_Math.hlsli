// ==========================================
// PBR_Math.hlsli
// 物理ベースレンダリング用の共通計算関数群
// ==========================================

static const float PI = 3.14159265359;

// ------------------------------------------
// D: 法線分布関数 (Trowbridge-Reitz GGX)
// ------------------------------------------
float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 0.0000001); // ゼロ除算防止
}

// ------------------------------------------
// G: 幾何減衰関数 (Schlick-GGX / Smith's method)
// ------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // 直接光用のkの値

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// ------------------------------------------
// F: フレネル方程式 (Schlick's approximation)
// ------------------------------------------
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    // cosThetaが1.0の時に0にならないようクランプ
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// IBL（環境光）用のフレネル方程式（Roughnessを考慮）
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}