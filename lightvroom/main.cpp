


#include "Application.h"

// メモリリークをチェックするためのライブラリ（デバッグ用）
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // アプリ終了時にメモリの解放忘れ（リーク）がないかを出力ウィンドウに表示する設定
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    Application app;

    // ウィンドウサイズを 1280x720 に指定して初期化
    if (!app.Initialize(hInstance, nCmdShow, 1280, 720))
    {
        return -1;
    }

    // ゲームのメインループ開始
    app.Run();

    return 0;
}