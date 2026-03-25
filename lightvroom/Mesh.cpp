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

bool Mesh::CreateSphere(ID3D11Device* device, float radius, UINT sliceCount, UINT stackCount)
{
    std::vector<Vertex> vertices;
    std::vector<WORD> indices;

    // 1. 頂点データの生成 (北極から南極へ)
    Vertex topVertex = { {0.0f, radius, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} };
    vertices.push_back(topVertex);

    float phiStep = DirectX::XM_PI / stackCount;
    float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

    for (UINT i = 1; i <= stackCount - 1; ++i)
    {
        float phi = i * phiStep;
        for (UINT j = 0; j <= sliceCount; ++j)
        {
            float theta = j * thetaStep;
            Vertex v;
            // 球面座標系からの変換
            v.Pos.x = radius * sinf(phi) * cosf(theta);
            v.Pos.y = radius * cosf(phi);
            v.Pos.z = radius * sinf(phi) * sinf(theta);
            // 球体なので、法線は中心からの方向と同じ
            v.Normal = v.Pos;
            DirectX::XMStoreFloat3(&v.Normal, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&v.Normal)));
            v.TexCoord.x = theta / (2.0f * DirectX::XM_PI);
            v.TexCoord.y = phi / DirectX::XM_PI;
            // 接ベクトル
            v.Tangent.x = -radius * sinf(phi) * sinf(theta);
            v.Tangent.y = 0.0f;
            v.Tangent.z = radius * sinf(phi) * cosf(theta);
            DirectX::XMStoreFloat3(&v.Tangent, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&v.Tangent)));
            vertices.push_back(v);
        }
    }
    Vertex bottomVertex = { {0.0f, -radius, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} };
    vertices.push_back(bottomVertex);

    // 2. インデックスデータの生成
    for (UINT i = 1; i <= sliceCount; ++i)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }
    UINT baseIndex = 1;
    UINT ringVertexCount = sliceCount + 1;
    for (UINT i = 0; i < stackCount - 2; ++i)
    {
        for (UINT j = 0; j < sliceCount; ++j)
        {
            indices.push_back(baseIndex + i * ringVertexCount + j);
            indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            indices.push_back(baseIndex + i * ringVertexCount + j + 1);
            indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }
    UINT southPoleIndex = (UINT)vertices.size() - 1;
    baseIndex = southPoleIndex - ringVertexCount;
    for (UINT i = 0; i < sliceCount; ++i)
    {
        indices.push_back(southPoleIndex);
        indices.push_back(baseIndex + i);
        indices.push_back(baseIndex + i + 1);
    }
    m_indexCount = static_cast<UINT>(indices.size());

    // 3. バッファの作成
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    vinitData.pSysMem = vertices.data();
    if (FAILED(device->CreateBuffer(&vbd, &vinitData, &m_vertexBuffer))) return false;

    D3D11_BUFFER_DESC ibd = {};
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(WORD) * m_indexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    iinitData.pSysMem = indices.data();
    if (FAILED(device->CreateBuffer(&ibd, &iinitData, &m_indexBuffer))) return false;

    return true;
}