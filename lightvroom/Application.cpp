#include "Application.h"
#include <DirectXMath.h>

Application::Application() : m_hwnd(nullptr), m_hInstance(nullptr) {}
Application::~Application() {}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow, int width, int height)
{
    m_hInstance = hInstance;

    // 1. ウィンドウクラスの登録
    const wchar_t CLASS_NAME[] = L"WindowClass";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClass(&wc)) return false;

    // 2. ウィンドウサイズの調整（枠線を省いた描画領域をきっちり指定サイズにするため）
    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    // 3. ウィンドウの生成
    m_hwnd = CreateWindowEx(
        0, CLASS_NAME, L"LightVroom(DX11)", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, hInstance, nullptr
    );
    if (m_hwnd == nullptr) return false;

    // ウィンドウを表示
    ShowWindow(m_hwnd, nCmdShow);

    // 4. DirectX 11の初期化
    m_graphics = std::make_unique<Graphics>();
    if (!m_graphics->Initialize(m_hwnd, width, height))
    {
        MessageBox(nullptr, L"DirectX11の初期化に失敗しました。", L"Error", MB_OK);
        return false;
    }

    // 5. シェーダーの初期化（コンパイル）
    m_shaderManager = std::make_unique<ShaderManager>();
    if (!m_shaderManager->Initialize(m_graphics->GetDevice(), L"StandardPBR.hlsl"))
    {
        MessageBox(nullptr, L"シェーダーのコンパイルに失敗しました。Visual Studioの出力ウィンドウを確認してください。", L"Error", MB_OK);
        return false;
    }

    m_cubeMesh = std::make_unique<Mesh>();
    if (!m_cubeMesh->CreateCube(m_graphics->GetDevice()))
    {
        MessageBox(nullptr, L"メッシュの作成に失敗しました。", L"Error", MB_OK);
        return false;
    }

    // --------------------------------------------------------------------

    // Application.cpp の Run 関数の中（DrawコールのTODOと書いていた場所）を書き換え
    // TODO: 次のステップで、ここに3Dモデルの「頂点バッファ」をセットしてDrawコールを呼びます
    // ↓ これを以下の1行に書き換える
    m_cubeMesh->Draw(m_graphics->GetContext());

    // TODO: ここで m_shaderManager = std::make_unique<ShaderManager>();
    // TODO: m_shaderManager->Initialize(m_graphics->GetDevice(), L"StandardPBR.hlsl");

    return true;
}

void Application::Run()
{
    MSG msg = { };

    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_graphics->Clear(0.0f, 0.2f, 0.4f, 1.0f);

            // シェーダーをパイプラインにセット
            m_shaderManager->Bind(m_graphics->GetContext());

            // ==========================================
            // 1. カメラとライトの情報 (Frameデータ) の準備
            // ==========================================
            using namespace DirectX;

            // カメラの位置と向き
            XMVECTOR camPos = XMVectorSet(0.0f, 2.0f, -5.0f, 1.0f); // 手前やや上から
            XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);  // 原点を見る
            XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);  // Y軸が上

            XMMATRIX view = XMMatrixLookAtLH(camPos, camTarget, camUp);
            // プロジェクション（遠近法）の設定: 視野角45度, アスペクト比 16:9, 描画範囲 0.1 ～ 100.0
            XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

            CBPerFrame frameData;
            // 【超重要】行列をシェーダーに送る前に必ず転置（Transpose）する！
            frameData.viewProjection = XMMatrixTranspose(view * proj);
            XMStoreFloat3(&frameData.cameraPos, camPos);

            // ライトの設定（斜め上から照らす白い光）
            frameData.lightDir = XMFLOAT3(0.5f, -1.0f, 0.5f);
            frameData.lightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);

            // ShaderManager経由でGPUへ転送
            m_shaderManager->UpdatePerFrame(m_graphics->GetContext(), frameData);


            // ==========================================
            // 2. 描画する物体の情報 (Objectデータ) の準備
            // ==========================================
            CBPerObject objData;
            // 物体を原点にそのまま配置する行列（単位行列）を作成し、転置する
            objData.worldMatrix = XMMatrixTranspose(XMMatrixIdentity());

            m_shaderManager->UpdatePerObject(m_graphics->GetContext(), objData);

            // TODO: 次のステップで、ここに3Dモデルの「頂点バッファ」をセットしてDrawコールを呼びます

            m_graphics->Present();
        }
    }
}


LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY: // ウィンドウの「×」ボタンが押されたとき
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}