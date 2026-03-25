#pragma once
#include <Windows.h>
#include <memory>

#include "Graphics.h"
#include "ShaderManager.h" 
#include "Mesh.h"
#include "Camera.h"
#include "Move.h"
#include "ShadowMap.h"

class Application
{
public:
    Application();
    ~Application();

    // アプリケーションの初期化
    bool Initialize(HINSTANCE hInstance, int nCmdShow, int width, int height);
    // メインループの実行
    void Run();

private:
    // ウィンドウからのメッセージ（閉じるボタンが押された等）を処理する関数
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    HINSTANCE m_hInstance;

    // Graphicsクラスのインスタンスを保持（スマートポインタで自動メモリ管理）
    std::unique_ptr<Graphics> m_graphics;

    // ShaderManager のインスタンスを保持する変数
    std::unique_ptr<ShaderManager> m_shaderManager; 

    std::unique_ptr<Mesh> m_cubeMesh;
    std::unique_ptr<Mesh> m_sphereMesh;
    std::unique_ptr<Mesh> m_floorMesh; //地面用

    std::unique_ptr<ShadowMap> m_shadowMap; //

    Camera m_camera;
    Move m_move;
    DirectX::XMFLOAT3 m_playerPos; // プレイヤー（立方体）の現在位置

};