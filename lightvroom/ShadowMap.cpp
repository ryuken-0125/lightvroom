#include "ShadowMap.h"

ShadowMap::ShadowMap() {}
ShadowMap::~ShadowMap() {}

bool ShadowMap::Initialize(ID3D11Device* device, int width, int height)
{
    // 1. テクスチャ本体の作成（色ではなく距離(R24)だけを保存する特殊な設定）
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthMap;
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &depthMap))) return false;

    // 2. 描き込むためのビュー (DSV)
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    if (FAILED(device->CreateDepthStencilView(depthMap.Get(), &dsvDesc, &m_dsv))) return false;

    // 3. 画像として読み込むためのビュー (SRV)
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    if (FAILED(device->CreateShaderResourceView(depthMap.Get(), &srvDesc, &m_srv))) return false;

    // 影専用のビューポート（解像度）設定
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = (float)width;
    m_viewport.Height = (float)height;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    return true;
}

void ShadowMap::Bind(ID3D11DeviceContext* context)
{
    context->RSSetViewports(1, &m_viewport);

    // カラー（色）の描画先を空（Null）にして、距離（DSV）だけを書き込むよう指示
    ID3D11RenderTargetView* nullRTV = nullptr;
    context->OMSetRenderTargets(1, &nullRTV, m_dsv.Get());

    // 毎フレーム影のキャンバスを真っ白にリセットする
    context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}