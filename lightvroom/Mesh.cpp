#include "Mesh.h"

Mesh::Mesh() : m_indexCount(0) {}
Mesh::~Mesh() {}

bool Mesh::CreateCube(ID3D11Device* device)
{
    // 1. 立方体の頂点データ (位置, 法線, UV, 接ベクトル)
    // 面ごとに法線が違うため、角を共有せずに面ごとに頂点を独立させます
    std::vector<Vertex> vertices = {
        // 前面 (Z = -1)
        { {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },
        { {-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { { 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { { 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },

        // 背面 (Z = 1)
        { { 1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },
        { { 1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },
        { {-1.0f,  1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} },
        { {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} },

        // 上面 (Y = 1)
        { {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },
        { {-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { { 1.0f,  1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { { 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },

        // 底面 (Y = -1)
        { {-1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },
        { {-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { { 1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} },
        { { 1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} },

        // 左面 (X = -1)
        { {-1.0f, -1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },
        { {-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
        { {-1.0f,  1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} },
        { {-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} },

        // 右面 (X = 1)
        { { 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} },
        { { 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { { 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} },
        { { 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} }
    };

    // 2. インデックスデータ (どの頂点を結んで三角形を作るか)
    std::vector<WORD> indices = {
        0, 1, 2, 0, 2, 3,       // 前
        4, 5, 6, 4, 6, 7,       // 後
        8, 9, 10, 8, 10, 11,    // 上
        12, 13, 14, 12, 14, 15, // 底
        16, 17, 18, 16, 18, 19, // 左
        20, 21, 22, 20, 22, 23  // 右
    };
    m_indexCount = static_cast<UINT>(indices.size());

    // 3. GPU上に頂点バッファを作成
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = vertices.data();
    if (FAILED(device->CreateBuffer(&vbd, &vinitData, &m_vertexBuffer))) return false;

    // 4. GPU上にインデックスバッファを作成
    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(WORD) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices.data();
    if (FAILED(device->CreateBuffer(&ibd, &iinitData, &m_indexBuffer))) return false;

    return true;
}

void Mesh::Draw(ID3D11DeviceContext* context)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    // パイプラインにバッファをセットして「三角形リスト」として描画命令を出す
    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->DrawIndexed(m_indexCount, 0, 0);
}