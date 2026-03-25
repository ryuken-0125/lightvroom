#include "ShaderManager.h"
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

ShaderManager::ShaderManager() {}
ShaderManager::~ShaderManager() {}

bool ShaderManager::Initialize(ID3D11Device* device, const std::wstring& pbrFilePath, const std::wstring& shadowFilePath)
{
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;

    // 1. PBR用 頂点シェーダー
    hr = D3DCompileFromFile(pbrFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) { OutputErrorMessage(errorBlob.Get()); return false; }
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);

    // 2. PBR用 ピクセルシェーダー
    hr = D3DCompileFromFile(pbrFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) { OutputErrorMessage(errorBlob.Get()); return false; }
    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);

    // 3. インプットレイアウト
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

    // ★追加：4. 影用 頂点シェーダーのコンパイル
    vsBlob.Reset(); errorBlob.Reset();
    hr = D3DCompileFromFile(shadowFilePath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) { OutputErrorMessage(errorBlob.Get()); return false; }
    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_shadowVertexShader);

    // ★追加：5. サンプラーステート（画像を読み込む際の設定）
    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    device->CreateSamplerState(&sampDesc, &m_samplerClamp);

    if (!m_cbPerFrame.Initialize(device)) return false;
    if (!m_cbPerObject.Initialize(device)) return false;
    if (!m_cbPerMaterial.Initialize(device)) return false;

    return true;
}

// ★修正：パス1（影作成用）のセット
void ShaderManager::BindShadowPass(ID3D11DeviceContext* context)
{
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_shadowVertexShader.Get(), nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0); // 色は塗らないのでPSはNull
}

// パス2（本番描画用）のセット

void ShaderManager::BindMainPass(ID3D11DeviceContext* context, ID3D11ShaderResourceView* shadowSRV)
{
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

    // シャドウマップの画像とサンプラーをシェーダーに渡す
    context->PSSetShaderResources(0, 1, &shadowSRV);

    // ★ここを 1 に変更しました（HLSLの register(s1) と合わせるため）
    context->PSSetSamplers(1, 1, m_samplerClamp.GetAddressOf());
}

void ShaderManager::OutputErrorMessage(ID3DBlob* errorBlob)
{
    if (errorBlob) {
        OutputDebugStringA("=== Shader Compile Error ===\n");
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        OutputDebugStringA("\n============================\n");
    }
    else {
        OutputDebugStringA("Shader File not found.\n");
    }
}

void ShaderManager::UpdatePerFrame(ID3D11DeviceContext* context, const CBPerFrame& data) {
    m_cbPerFrame.Data = data; m_cbPerFrame.ApplyChanges(context);
    context->VSSetConstantBuffers(0, 1, m_cbPerFrame.Buffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_cbPerFrame.Buffer.GetAddressOf());
}

void ShaderManager::UpdatePerObject(ID3D11DeviceContext* context, const CBPerObject& data) {
    m_cbPerObject.Data = data; m_cbPerObject.ApplyChanges(context);
    context->VSSetConstantBuffers(1, 1, m_cbPerObject.Buffer.GetAddressOf());
    context->PSSetConstantBuffers(1, 1, m_cbPerObject.Buffer.GetAddressOf());
}

void ShaderManager::UpdatePerMaterial(ID3D11DeviceContext* context, const CBPerMaterial& data) {
    m_cbPerMaterial.Data = data; m_cbPerMaterial.ApplyChanges(context);
    context->PSSetConstantBuffers(2, 1, m_cbPerMaterial.Buffer.GetAddressOf());
}