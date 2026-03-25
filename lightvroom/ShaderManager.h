#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <string>
#include <DirectXMath.h>
#include "ConstantBuffer.h"

struct CBPerMaterial {
    DirectX::XMFLOAT4 albedo;
    float roughness;
    float metallic;
    DirectX::XMFLOAT2 pad;
};

struct CBPerFrame {
    DirectX::XMMATRIX viewProjection;
    DirectX::XMMATRIX lightViewProjection; // ★追加：太陽のカメラ
    DirectX::XMFLOAT3 cameraPos;
    float pad1;
    DirectX::XMFLOAT3 lightDir;
    float pad2;
    DirectX::XMFLOAT3 lightColor;
    float pad3;
};

struct CBPerObject {
    DirectX::XMMATRIX worldMatrix;
};

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    // ★修正：シャドウ用のファイルパスも受け取る
    bool Initialize(ID3D11Device* device, const std::wstring& pbrFilePath, const std::wstring& shadowFilePath);

    // ★修正：描画パスの切り替え
    void BindShadowPass(ID3D11DeviceContext* context);
    void BindMainPass(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shadowSRV);

    void UpdatePerFrame(ID3D11DeviceContext* context, const CBPerFrame& data);
    void UpdatePerObject(ID3D11DeviceContext* context, const CBPerObject& data);
    void UpdatePerMaterial(ID3D11DeviceContext* context, const CBPerMaterial& data);

private:
    void OutputErrorMessage(ID3DBlob* errorBlob);

    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shadowVertexShader; // ★追加
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerClamp;       // ★追加：影のギザギザを減らすフィルター

    ConstantBuffer<CBPerFrame> m_cbPerFrame;
    ConstantBuffer<CBPerObject> m_cbPerObject;
    ConstantBuffer<CBPerMaterial> m_cbPerMaterial;
};