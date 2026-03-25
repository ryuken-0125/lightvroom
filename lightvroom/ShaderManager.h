#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <string>
#include <DirectXMath.h> // 数学ライブラリ
#include "ConstantBuffer.h"

struct CBPerMaterial
{
    DirectX::XMFLOAT4 albedo;   // 基本色 (RGBA)
    float roughness;            // 粗さ
    float metallic;             // 金属度
    DirectX::XMFLOAT2 pad;      // 16バイトアライメント用の調整
};

// HLSLの cbPerFrame と完全に一致させる構造体
struct CBPerFrame
{
    DirectX::XMMATRIX viewProjection;
    DirectX::XMFLOAT3 cameraPos;
    float pad1;
    DirectX::XMFLOAT3 lightDir;
    float pad2;
    DirectX::XMFLOAT3 lightColor;
    float pad3;
};

// HLSLの cbPerObject と完全に一致させる構造体
struct CBPerObject
{
    DirectX::XMMATRIX worldMatrix;
};

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    // シェーダーをファイルから読み込み、初期化する関数
    bool Initialize(ID3D11Device* device, const std::wstring& filePath);

    // 描画時にシェーダーとインプットレイアウトを適用する関数
    void Bind(ID3D11DeviceContext* context);

    void UpdatePerFrame(ID3D11DeviceContext* context, const CBPerFrame& data);
    void UpdatePerObject(ID3D11DeviceContext* context, const CBPerObject& data);

    void UpdatePerMaterial(ID3D11DeviceContext* context, const CBPerMaterial& data);

private:
    // コンパイルエラーの内容をデバッグ出力するヘルパー関数
    void OutputErrorMessage(ID3DBlob* errorBlob);

private:
    // ComPtrを使用することで、解放忘れ（メモリリーク）を防ぎます
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_inputLayout;

    ConstantBuffer<CBPerFrame> m_cbPerFrame;  // レジスタ b0 用
    ConstantBuffer<CBPerObject> m_cbPerObject; // レジスタ b1 用

    ConstantBuffer<CBPerMaterial> m_cbPerMaterial; // b2

};