#include "ShaderManager.h"
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib") // これが D3DCompileFromFile の本体です

ShaderManager::ShaderManager() {}
ShaderManager::~ShaderManager() {}

bool ShaderManager::Initialize(ID3D11Device* device, const std::wstring& filePath)
{
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    // =========================================================
    // 1. 頂点シェーダー (Vertex Shader) のコンパイル
    // =========================================================
    // D3DCOMPILE_ENABLE_STRICTNESS: 厳密な文法チェック
    // D3DCOMPILE_DEBUG: デバッグ情報の埋め込み（リリース時は外すのが一般的）
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;

    hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, // #include "PBR_Math.hlsli" を機能させるために必須
        "VSMain",                          // エントリポイント名（HLSL内の関数名）
        "vs_5_0",                          // シェーダーモデル（DirectX 11の標準は5.0）
        compileFlags,
        0,
        &vsBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        OutputErrorMessage(errorBlob.Get());
        return false;
    }

    // コンパイルされたバイナリ(Blob)から、デバイス上にシェーダーオブジェクトを作成
    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    if (FAILED(hr)) return false;


    // =========================================================
    // 2. ピクセルシェーダー (Pixel Shader) のコンパイル
    // =========================================================
    hr = D3DCompileFromFile(
        filePath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PSMain",                          // ピクセルシェーダーのエントリポイント
        "ps_5_0",                          // ピクセルシェーダーのモデル
        compileFlags,
        0,
        &psBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        OutputErrorMessage(errorBlob.Get());
        return false;
    }

    hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
    if (FAILED(hr)) return false;


    // =========================================================
    // 3. インプットレイアウト (Input Layout) の作成
    // =========================================================
    // C++側の頂点構造体と、HLSL側の VS_INPUT を結びつける超重要設定です。
    // ※ここの順番とフォーマットは、前回のHLSLのVS_INPUTと完全に一致させる必要があります。
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        // セマンティクス名, インデックス, フォーマット, スロット, バイトオフセット, データクラス, インスタンスステップ
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    UINT numElements = ARRAYSIZE(layoutDesc);

    // インプットレイアウトは頂点シェーダーのシグネチャ（Blob）を使って作成します
    hr = device->CreateInputLayout(
        layoutDesc,
        numElements,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &m_inputLayout
    );

    if (FAILED(hr)) return false;

    // 定数バッファの初期化
    if (!m_cbPerFrame.Initialize(device)) return false;
    if (!m_cbPerObject.Initialize(device)) return false;

    return true;
}

void ShaderManager::Bind(ID3D11DeviceContext* context)
{
    // 描画パイプラインにシェーダーとレイアウトをセット
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

void ShaderManager::OutputErrorMessage(ID3DBlob* errorBlob)
{
    if (errorBlob)
    {
        // OutputDebugStringA を使うと、VSの出力ウィンドウに文字を出せます
        OutputDebugStringA("=== Shader Compile Error ===\n");
        OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        OutputDebugStringA("\n============================\n");
    }
    else
    {
        OutputDebugStringA("Shader File not found. (ファイルが見つかりません)\n");
    }
}

void ShaderManager::UpdatePerFrame(ID3D11DeviceContext* context, const CBPerFrame& data)
{
    // データをコピー (注意: ここで行列は既に転置されている前提とします)
    m_cbPerFrame.Data = data;
    // GPUへ転送
    m_cbPerFrame.ApplyChanges(context);

    // HLSLの register(b0) にセット (VSとPSの両方で使うので両方にセット)
    context->VSSetConstantBuffers(0, 1, m_cbPerFrame.Buffer.GetAddressOf());
    context->PSSetConstantBuffers(0, 1, m_cbPerFrame.Buffer.GetAddressOf());
}

void ShaderManager::UpdatePerObject(ID3D11DeviceContext* context, const CBPerObject& data)
{
    m_cbPerObject.Data = data;
    m_cbPerObject.ApplyChanges(context);

    // HLSLの register(b1) にセット
    context->VSSetConstantBuffers(1, 1, m_cbPerObject.Buffer.GetAddressOf());
    context->PSSetConstantBuffers(1, 1, m_cbPerObject.Buffer.GetAddressOf());
}
