#include "Graphics.h"

// 必要なDirectXライブラリのリンク
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

Graphics::Graphics() {}
Graphics::~Graphics() {}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
    HRESULT hr;

    // 1. スワップチェーン（裏画面・表画面の仕組み）の設定
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1; // ダブルバッファリング
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; // 60FPS
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;   // マルチサンプリング（アンチエイリアス）なし
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;        // ウィンドウモード

    // デバッグ時はDirectXの警告メッセージを有効にする設定
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 }; // DX11を使用

    // デバイス、コンテキスト、スワップチェーンを同時に作成
    hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevels, 1, D3D11_SDK_VERSION, &sd, &m_swapChain,
        &m_device, nullptr, &m_context);
    if (FAILED(hr)) return false;

    // 2. レンダーターゲットビュー(RTV)の作成（色を描き込むキャンバス）
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) return false;
    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) return false;

    // 3. 深度ステンシルビュー(DSV)の作成（3Dの物体の前後関係を正しく描画するためのキャンバス）
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
    hr = m_device->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer);
    if (FAILED(hr)) return false;
    hr = m_device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, &m_depthStencilView);
    if (FAILED(hr)) return false;

    // 4. キャンバスをパイプラインにセット
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // 5. ビューポート（画面のどの範囲に描画するか）の設定
    D3D11_VIEWPORT vp = {};
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_context->RSSetViewports(1, &vp);

    return true;
}

void Graphics::Clear(float r, float g, float b, float a)
{
    // 背景色と、深度バッファを毎フレームリセットする
    float color[4] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
    m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Graphics::Present()
{
    // 描画結果を画面に反映（1=VSync有効, 0=VSync無効）
    m_swapChain->Present(1, 0);
}