#include "Application.h"
#include <DirectXMath.h>
#include <chrono> //時間を正確に測るための標準ライブラリ

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

    // 2. ウィンドウサイズの調整
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

    // 5. シェーダーの初期化
    m_shaderManager = std::make_unique<ShaderManager>();
    if (!m_shaderManager->Initialize(m_graphics->GetDevice(), L"StandardPBR.hlsl"))
    {
        MessageBox(nullptr, L"シェーダーのコンパイルに失敗しました。", L"Error", MB_OK);
        return false;
    }

    // 6. メッシュ（立方体）の初期化
    m_cubeMesh = std::make_unique<Mesh>();
    if (!m_cubeMesh->CreateCube(m_graphics->GetDevice()))
    {
        MessageBox(nullptr, L"メッシュの作成に失敗しました。", L"Error", MB_OK);
        return false;
    }

    // 球体の初期化（半径1.0、経度30分割、緯度30分割で滑らかに）
    m_sphereMesh = std::make_unique<Mesh>();
    if (!m_sphereMesh->CreateSphere(m_graphics->GetDevice(), 1.0f, 30, 30))
    {
        return false;
    }

    // カメラとプレイヤーの初期設定
    m_camera.SetPosition(0.0f, 2.0f, -6.0f);
    m_camera.SetProjection(DirectX::XMConvertToRadians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    // プレイヤー(立方体)の初期位置を左側にセット
    m_playerPos = DirectX::XMFLOAT3(-1.5f, 0.0f, 0.0f);

    return true;
}

void Application::Run()
{
    MSG msg = { };

    // 時間計測の準備
    auto startTime = std::chrono::high_resolution_clock::now();
    auto prevTime = startTime; // ★追加：前回のフレームの時間を記録

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
            // 時間の計算（前回の描画から何秒経ったか）
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - prevTime).count();
            prevTime = currentTime;

            // プログラム起動からの累計時間（太陽を回す用）
            float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();

            // --- 入力の処理と更新 ---
            // マウスの位置を特定するためにウィンドウハンドル(m_hwnd)を渡します
            m_move.ControlCamera(m_camera, deltaTime, m_hwnd);
            m_move.ControlPlayer(m_playerPos, deltaTime);


            // --- 描画処理 ---
            m_graphics->Clear(0.0f, 0.2f, 0.4f, 1.0f);
            m_shaderManager->Bind(m_graphics->GetContext());

            using namespace DirectX;

            // 1. フレームデータ（カメラとライト）の送信
            CBPerFrame frameData;
            // 固定の行列ではなく、Cameraクラスから最新の行列をもらいます
            frameData.viewProjection = XMMatrixTranspose(m_camera.GetViewMatrix() * m_camera.GetProjectionMatrix());
            frameData.cameraPos = m_camera.GetPosition();

            // 太陽の回転
            float sunAngle = elapsedTime * 0.5f;
            XMVECTOR sunDir = XMVectorSet(cosf(sunAngle), -1.0f, sinf(sunAngle), 0.0f);
            sunDir = XMVector3Normalize(sunDir);
            XMStoreFloat3(&frameData.lightDir, sunDir);
            frameData.lightColor = XMFLOAT3(3.0f, 3.0f, 3.0f);

            m_shaderManager->UpdatePerFrame(m_graphics->GetContext(), frameData);

            // ==========================================
            // 描画 1：プレイヤー（赤い立方体）
            // ==========================================
            CBPerObject cubeObj;
            // 固定位置ではなく、m_playerPos の座標を使って移動させます
            cubeObj.worldMatrix = XMMatrixTranspose(XMMatrixTranslation(m_playerPos.x, m_playerPos.y, m_playerPos.z));
            m_shaderManager->UpdatePerObject(m_graphics->GetContext(), cubeObj);

            CBPerMaterial cubeMat;
            cubeMat.albedo = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
            cubeMat.roughness = 0.3f;
            cubeMat.metallic = 0.0f;
            m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), cubeMat);

            m_cubeMesh->Draw(m_graphics->GetContext());

            // ==========================================
            // 描画 2：鉄の球体（位置は固定のまま）
            // ==========================================
            CBPerObject sphereObj;
            sphereObj.worldMatrix = XMMatrixTranspose(XMMatrixTranslation(1.5f, 0.0f, 0.0f));
            m_shaderManager->UpdatePerObject(m_graphics->GetContext(), sphereObj);

            CBPerMaterial sphereMat;
            sphereMat.albedo = XMFLOAT4(0.56f, 0.57f, 0.58f, 1.0f);
            sphereMat.roughness = 0.15f;
            sphereMat.metallic = 1.0f;
            m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), sphereMat);

            m_sphereMesh->Draw(m_graphics->GetContext());

            m_graphics->Present();
        }
    }
}


LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}