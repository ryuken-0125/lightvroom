#pragma once
#include <d3d11.h>
#include <wrl/client.h>

class ShadowMap
{
public:
    ShadowMap();
    ~ShadowMap();

    // シャドウマップの初期化（影の解像度を指定）
    bool Initialize(ID3D11Device* device, int width, int height);

    // 影を描き込むためのキャンバスをセットする
    void Bind(ID3D11DeviceContext* context);

    // 完成した影の画像をシェーダーに渡すための窓口
    ID3D11ShaderResourceView* GetSRV() { return m_srv.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv; // 距離を描き込む用
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv; // シェーダーで読み込む用
    D3D11_VIEWPORT m_viewport;
};