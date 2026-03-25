#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>

// C++側の頂点構造体（ShaderManagerのインプットレイアウトと一致させます）
struct Vertex
{
    DirectX::XMFLOAT3 Pos;      // 位置
    DirectX::XMFLOAT3 Normal;   // 法線（光の計算に必須）
    DirectX::XMFLOAT2 TexCoord; // UV座標（テクスチャ用）
    DirectX::XMFLOAT3 Tangent;  // 接ベクトル（法線マップ計算用）
};

class Mesh
{
public:
    Mesh();
    ~Mesh();

    // 立方体のデータを生成してGPUにバッファを作る関数
    bool CreateCube(ID3D11Device* device);
    //球体を生成する関数
    bool CreateSphere(ID3D11Device* device, float radius, UINT sliceCount, UINT stackCount);

    // 描画（Drawコール）を行う関数
    void Draw(ID3D11DeviceContext* context);

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer; // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;  // インデックスバッファ
    UINT m_indexCount;                                   // 描画するインデックスの数
};