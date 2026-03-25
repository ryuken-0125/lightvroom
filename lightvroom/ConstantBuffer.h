#pragma once
#include <d3d11.h>
#include <wrl/client.h>

// 任意の構造体(T)を定数バッファとして扱うためのテンプレートクラス
template<class T>
class ConstantBuffer
{
public:
    T Data; // C++側で書き換えるためのデータ本体
    Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer; // GPU側のバッファ

    bool Initialize(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;

        // 【重要】構造体のサイズを自動的に16の倍数に切り上げる計算
        desc.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16)) % 16);

        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        HRESULT hr = device->CreateBuffer(&desc, nullptr, &Buffer);
        return SUCCEEDED(hr);
    }

    // C++側の Data の中身を、GPU側の Buffer へ転送する
    void ApplyChanges(ID3D11DeviceContext* context)
    {
        context->UpdateSubresource(Buffer.Get(), 0, nullptr, &Data, 0, 0);
    }
};