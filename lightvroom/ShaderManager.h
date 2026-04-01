#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <string>
#include <DirectXMath.h>
#include "ConstantBuffer.h"

struct CBPerFrame
{
    DirectX::XMMATRIX viewProjection;
    DirectX::XMMATRIX lightViewProjection;
    DirectX::XMFLOAT3 cameraPos;    float pad1;

    // 太陽のデータ
    DirectX::XMFLOAT3 lightDir;     float pad2;
    DirectX::XMFLOAT3 lightColor;   float pad3;

    // 月のデータ
    DirectX::XMFLOAT3 sunDir;       float pad4;
    DirectX::XMFLOAT3 sunColor;     float pad5; 

   
    DirectX::XMFLOAT3 moonDir;      float pad6; 
    DirectX::XMFLOAT3 moonColor;    float pad7; 

    // 空の色
    DirectX::XMFLOAT4 skyColor;  
};

struct CBPerObject {
    DirectX::XMMATRIX worldMatrix;
};

struct CBPerMaterial {
    DirectX::XMFLOAT4 albedo;
    float roughness;
    float metallic;
    float emissive; // 1.0 で物体自体が光源のように光る
    float pad;
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