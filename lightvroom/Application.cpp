#include "Application.h"
#include <DirectXMath.h>
#include <chrono>

Application::Application() : m_hwnd(nullptr), m_hInstance(nullptr) {}
Application::~Application() {}

bool Application::Initialize(HINSTANCE hInstance, int nCmdShow, int width, int height)
{
    m_hInstance = hInstance;
    const wchar_t CLASS_NAME[] = L"WindowClass";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    if (!RegisterClass(&wc)) return false;

    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowEx(0, CLASS_NAME, L"LightVroom(DX11) - Shadow Mapping", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, hInstance, nullptr);
    if (m_hwnd == nullptr) return false;

    ShowWindow(m_hwnd, nCmdShow);

    m_graphics = std::make_unique<Graphics>();
    if (!m_graphics->Initialize(m_hwnd, width, height)) return false;

    m_shaderManager = std::make_unique<ShaderManager>();
    // ★修正：2つのシェーダーを読み込む
    if (!m_shaderManager->Initialize(m_graphics->GetDevice(), L"StandardPBR.hlsl", L"Shadow.hlsl")) return false;

    // ★追加：シャドウマップの初期化 (2048x2048の高解像度)
    m_shadowMap = std::make_unique<ShadowMap>();
    if (!m_shadowMap->Initialize(m_graphics->GetDevice(), 2048, 2048)) return false;

    m_cubeMesh = std::make_unique<Mesh>();
    m_cubeMesh->CreateCube(m_graphics->GetDevice());

    m_sphereMesh = std::make_unique<Mesh>();
    m_sphereMesh->CreateSphere(m_graphics->GetDevice(), 1.0f, 30, 30);

    // ★追加：地面用のメッシュ（立方体を使いまわします）
    m_floorMesh = std::make_unique<Mesh>();
    m_floorMesh->CreateCube(m_graphics->GetDevice());

    m_camera.SetPosition(0.0f, 2.0f, -8.0f);
    m_camera.SetProjection(DirectX::XMConvertToRadians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    m_playerPos = DirectX::XMFLOAT3(-1.5f, 0.0f, 0.0f);

    return true;
}

void Application::Run()
{
    MSG msg = { };
    auto startTime = std::chrono::high_resolution_clock::now();
    auto prevTime = startTime;

    const float DAY_DURATION = 30.0f;


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
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - prevTime).count();
            prevTime = currentTime;
            float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();

            m_move.ControlCamera(m_camera, deltaTime, m_hwnd);
            m_move.ControlPlayer(m_playerPos, deltaTime);

            float dayTime = fmodf(elapsedTime / DAY_DURATION, 1.0f);
            float angle = dayTime * DirectX::XM_2PI;

            using namespace DirectX;

            // --- 太陽と月の位置計算 (西から東へ) ---
            // 太陽
            float sunAngle = elapsedTime * 0.5f;
            XMVECTOR sunPos = XMVectorSet(cosf(sunAngle) * 20.0f, sinf(sunAngle) * 20.0f, 0.0f, 0.0f);
            XMVECTOR sunDir = XMVector3Normalize(-sunPos);

            // 月（太陽の正反対に配置）
            XMVECTOR moonPos = -sunPos;
            XMVECTOR moonDir = XMVector3Normalize(-moonPos);


            // --- 時間帯に応じた色の変化 ---
            XMFLOAT4 currentSkyColor;
            XMFLOAT3 currentSunColor;

            float sunIntensity = max(0.0f, sinf(angle)); // 太陽が上にある時だけ強く

            if (sunIntensity > 0.0f) {
                // 昼：青空
                currentSkyColor = XMFLOAT4(0.2f, 0.4f, 0.8f, 1.0f);
                currentSunColor = XMFLOAT3(5.0f * sunIntensity, 4.5f * sunIntensity, 4.0f * sunIntensity);
            }
            else {
                // 夜：濃紺
                currentSkyColor = XMFLOAT4(0.02f, 0.02f, 0.1f, 1.0f);
                currentSunColor = XMFLOAT3(0, 0, 0);
            }

            // 月の光（常に一定の淡い青白さ）
            float moonIntensity = max(0.0f, sinf(angle + XM_PI));
            XMFLOAT3 currentMoonColor = XMFLOAT3(0.5f * moonIntensity, 0.6f * moonIntensity, 0.8f * moonIntensity);

            // --- フレームデータの転送 ---
            CBPerFrame frameData;
            frameData.viewProjection = XMMatrixTranspose(m_camera.GetViewMatrix() * m_camera.GetProjectionMatrix());
            XMStoreFloat3(&frameData.sunDir, sunDir);
            frameData.sunColor = currentSunColor;
            XMStoreFloat3(&frameData.moonDir, moonDir);
            frameData.moonColor = currentMoonColor;
            frameData.skyColor = currentSkyColor;
            // 影は太陽のみから計算（太陽が沈んでいる時は計算をスキップするロジックが必要）
            // ...

            // --- 描画パス ---
            m_graphics->Clear(currentSkyColor.x, currentSkyColor.y, currentSkyColor.z, 1.0f);

            // 1. 太陽自体を描画
            CBPerObject sunObj;
            sunObj.worldMatrix = XMMatrixTranspose(XMMatrixTranslationFromVector(sunPos));
            m_shaderManager->UpdatePerObject(m_graphics->GetContext(), sunObj); // ★修正
            CBPerMaterial sunMat = { XMFLOAT4(1.0f, 1.0f, 0.8f, 1.0f), 0.0f, 0.0f, 10.0f, 0.0f };
            m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), sunMat); // ★修正
            m_sphereMesh->Draw(m_graphics->GetContext()); // ★修正

            // 2. 月自体を描画
            CBPerObject moonObj;
            moonObj.worldMatrix = XMMatrixTranspose(XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslationFromVector(moonPos));
            m_shaderManager->UpdatePerObject(m_graphics->GetContext(), moonObj); // ★修正
            CBPerMaterial moonMat = { XMFLOAT4(0.8f, 0.8f, 1.0f, 1.0f), 0.0f, 0.0f, 2.0f, 0.0f };
            m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), moonMat); // ★修正
            m_sphereMesh->Draw(m_graphics->GetContext()); // ★修正

            // -----------------------------------------------------------
            // C++11のラムダ式を使って「3つの物体を描画する処理」をまとめます
            // -----------------------------------------------------------
            auto DrawAllObjects = [&](bool isShadowPass) {
                // 1. 地面
                CBPerObject floorObj;
                floorObj.worldMatrix = XMMatrixTranspose(XMMatrixScaling(10.0f, 0.1f, 10.0f) * XMMatrixTranslation(0.0f, -1.0f, 0.0f));
                m_shaderManager->UpdatePerObject(m_graphics->GetContext(), floorObj);
                if (!isShadowPass) {
                    CBPerMaterial floorMat = { XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f), 0.9f, 0.0f, 0.0f, 0.0f };
                    m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), floorMat);
                }
                m_floorMesh->Draw(m_graphics->GetContext());

                // 2. 赤い立方体
                CBPerObject cubeObj;
                cubeObj.worldMatrix = XMMatrixTranspose(XMMatrixTranslation(m_playerPos.x, m_playerPos.y, m_playerPos.z));
                m_shaderManager->UpdatePerObject(m_graphics->GetContext(), cubeObj);
                if (!isShadowPass) {
                    CBPerMaterial cubeMat = { XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f), 0.3f, 0.0f, 0.0f, 0.0f };
                    m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), cubeMat);
                }
                m_cubeMesh->Draw(m_graphics->GetContext());

                // 3. 鉄の球体
                CBPerObject sphereObj;
                sphereObj.worldMatrix = XMMatrixTranspose(XMMatrixTranslation(1.5f, 0.0f, 0.0f));
                m_shaderManager->UpdatePerObject(m_graphics->GetContext(), sphereObj);
                if (!isShadowPass) {
                    CBPerMaterial sphereMat = { XMFLOAT4(0.56f, 0.57f, 0.58f, 1.0f), 0.15f, 1.0f, 0.0f, 0.0f };
                    m_shaderManager->UpdatePerMaterial(m_graphics->GetContext(), sphereMat);
                }
                m_sphereMesh->Draw(m_graphics->GetContext());
                };
            // ==========================================
            // パス1：シャドウマップ（影）の描画
            // ==========================================
            m_shadowMap->Bind(m_graphics->GetContext());
            m_shaderManager->BindShadowPass(m_graphics->GetContext());
            m_shaderManager->UpdatePerFrame(m_graphics->GetContext(), frameData);

            DrawAllObjects(true); // 影用に描画


            // ==========================================
            // パス2：メイン画面（本番）の描画
            // ==========================================
            m_graphics->SetMainRenderTarget();
            m_graphics->Clear(0.0f, 0.2f, 0.4f, 1.0f);
            m_shaderManager->BindMainPass(m_graphics->GetContext(), m_shadowMap->GetSRV());

            DrawAllObjects(false); // 色や質感を付けて描画


            // ★超重要：使い終わった影の画像をパイプラインから外す（次のフレームでエラーになるのを防ぐため）
            ID3D11ShaderResourceView* nullSRV = nullptr;
            m_graphics->GetContext()->PSSetShaderResources(0, 1, &nullSRV);

            m_graphics->Present();
        }
    }
}

LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}