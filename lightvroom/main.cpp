


#include "Application.h"

// メモリリークをチェックするためのライブラリ（デバッグ用）
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


//カメラの移動: W A S D（前後左右）、 Q E（上下）
//カメラの回転（視点）: 矢印キー(↑ ↓ ← →)
//プレイヤー(赤い立方体)の移動 : I J K L（奥、手前、左右）


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    //終了時にメモリの解放忘れ（リーク）がないかを出力ウィンドウに表示する設定
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
