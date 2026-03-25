// 太陽の視点の行列を受け取る
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
}
cbuffer cbPerObject : register(b1)
{
    matrix worldMatrix;
}

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float3 Tangent : TANGENT;
};

float4 VSMain(VS_INPUT input) : SV_POSITION
{
    // 物体を太陽の視点（lightViewProjection）から見た時の座標に変換するだけ！
    float4 worldPos = mul(float4(input.Pos, 1.0f), worldMatrix);
    return mul(worldPos, lightViewProjection);
}