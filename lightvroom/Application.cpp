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

    return true;
}

void Application::Run()
{
    MSG msg = { };

    // ★追加: アプリケーション開始時の時間を記録（基準点）
    auto startTime = std::chrono::high_resolution_clock::now();

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
            m_shaderManager->Bind(m_graphics->GetContext());

            using namespace DirectX;

            // ★追加: 現在の時間を取得し、開始時からの経過時間（秒）を計算
            auto currentTime = std::chrono::high_resolution_clock::now();
            float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();

            // 1. カメラの設定
            XMVECTOR camPos = XMVectorSet(0.0f, 2.0f, -6.0f, 1.0f);
            XMVECTOR camTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
            XMVECTOR camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            XMMATRIX view = XMMatrixLookAtLH(camPos, camTarget, camUp);
            XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

            CBPerFrame frameData;
            frameData.viewProjection = XMMatrixTranspose(view * proj);
            XMStoreFloat3(&frameData.cameraPos, camPos);

            // ==========================================
            // ★追加: 太陽（平行光源）を動かす処理★
            // ==========================================
            // 経過時間を使って太陽の回転角度を計算（1.0fを大きくすると早く回ります）
            float sunAngle = elapsedTime * 0.5f;

            // 太陽の向き（ベクトル）を計算
            // XとZに cos/sin を使うことで、物体を囲むように円を描いて回ります
            // Yを -1.0f に固定することで、常に斜め上から見下ろすように光が当たります
            XMVECTOR sunDir = XMVectorSet(cosf(sunAngle), -1.0f, sinf(sunAngle), 0.0f);

            // ベクトルの長さを必ず 1.0 (正規化) にしてシェーダーに送る必要があります
            sunDir = XMVector3Normalize(sunDir);
            XMStoreFloat3(&frameData.lightDir, sunDir);

            // 固定の太陽の光量（強さと色）
            // PBRでは強めに設定することが多いので、真っ白な強い光（RGBそれぞれ3.0）を設定します
            frameData.lightColor = XMFLOAT3(3.0f, 3.0f, 3.0f);

            m_shaderManager->UpdatePerFrame(m_graphics->GetContext(), frameData);

            // ==========================================
            // 描画 1：左側の「赤いプラスチックの立方体」
            // ==========================================
            CBPerObject cubeObj;
            // X軸方向に -1.5 (左) へ移動
            cubeObj.worldMatrix = XMMatrixTranspose(XMMatrixTranslation(-1.5f, 0.0f, 0.0f));
            m_shaderManager->UpdatePerObject(m_graphics->GetContext(), cubeObj);

            CBPerMaterial cubeMat;
            cubeMat.albedo = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f); // 赤色
            cubeMat.roughness = 0.3f; // 少しツヤのあるプラスチック風
            cubeMat.metallic = 0.0f; // 非金属
            m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), cubeMat);

            m_cubeMesh->Draw(m_graphics->GetContext());

            // ==========================================
            // 描画 2：右側の「鉄の球体」
            // ==========================================
            CBPerObject sphereObj;
            // X軸方向に +1.5 (右) へ移動
            sphereObj.worldMatrix = XMMatrixTranspose(XMMatrixTranslation(1.5f, 0.0f, 0.0f));
            m_shaderManager->UpdatePerObject(m_graphics->GetContext(), sphereObj);

            CBPerMaterial sphereMat;
            sphereMat.albedo = XMFLOAT4(0.56f, 0.57f, 0.58f, 1.0f); // 鉄の基本反射率（現実の近似値）
            sphereMat.roughness = 0.15f; // やや磨かれた金属
            sphereMat.metallic = 1.0f;  // 完全な金属
            m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), sphereMat);

            m_sphereMesh->Draw(m_graphics->GetContext());

            // 画面のフリップ
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