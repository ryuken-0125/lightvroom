#pragma once
#include <d3d11.h>
#include <wrl/client.h> // ComPtr用

class Graphics
{
public:
    Graphics();
    ~Graphics();

    bool Initialize(HWND hwnd, int width, int height);
    void Clear(float r, float g, float b, float a);
    void Present();

    void SetMainRenderTarget();

    // 他のクラス（ShaderManagerなど）からDeviceやContextを使えるようにする窓口
    ID3D11Device* GetDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

private:
    // DirectXのコアオブジェクト群 (ComPtrが自動でメモリ解放(Release)をしてくれます)
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;                   // デバイス（生成役）
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;           // コンテキスト（指示役）
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;              // スワップチェーン（画面の切り替え役）
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView; // 描画先（カラー）
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView; // 描画先（深度：3Dの前後関係用）

    D3D11_VIEWPORT m_viewport; //メイン画面の解像度を保持
};