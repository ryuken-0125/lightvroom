#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <string>

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    // シェーダーをファイルから読み込み、初期化する関数
    bool Initialize(ID3D11Device* device, const std::wstring& filePath);

    // 描画時にシェーダーとインプットレイアウトを適用する関数
    void Bind(ID3D11DeviceContext* context);

private:
    // コンパイルエラーの内容をデバッグ出力するヘルパー関数
    void OutputErrorMessage(ID3DBlob* errorBlob);

private:
    // ComPtrを使用することで、解放忘れ（メモリリーク）を防ぎます
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_inputLayout;
};