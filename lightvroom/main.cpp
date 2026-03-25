


#include "ShaderManager.h" // ShaderManagerクラスの定義を含むヘッダーファイル


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    // 1. 初期化時
    ShaderManager pbrShader;
    if (!pbrShader.Initialize(hInstance, L"StandardPBR.hlsl")) {
        // エラー処理
    }

    // 2. 毎フレームの描画処理内
    pbrShader.Bind(nCmdShow);

    return 0;
}