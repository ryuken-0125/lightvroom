#include "Move.h"

Move::Move() : m_isDragging(false)
{
    m_prevMousePos = { -1, -1 };
}
Move::~Move() {}

void Move::ControlCamera(Camera& camera, float deltaTime, HWND hwnd)
{
    float moveSpeed = 5.0f * deltaTime;       // カメラの移動速度
    float keyRotSpeed = 2.0f * deltaTime;     // 矢印キーでの回転速度
    float mouseSensitivity = 0.005f;          // マウスでの回転感度

    // ===================================
    // 視点の移動 (W, A, S, D, Q, Eキー)
    // ===================================
    if (GetAsyncKeyState('W') & 0x8000) camera.MoveForward(moveSpeed);
    if (GetAsyncKeyState('S') & 0x8000) camera.MoveForward(-moveSpeed);
    if (GetAsyncKeyState('D') & 0x8000) camera.MoveRight(moveSpeed);
    if (GetAsyncKeyState('A') & 0x8000) camera.MoveRight(-moveSpeed);
    if (GetAsyncKeyState('E') & 0x8000) camera.MoveUp(moveSpeed);
    if (GetAsyncKeyState('Q') & 0x8000) camera.MoveUp(-moveSpeed);

    float pitch = 0.0f;
    float yaw = 0.0f;

    // ===================================
    // 視点の回転 1：矢印キー (↑ ↓ ← →)
    // ===================================
    if (GetAsyncKeyState(VK_UP) & 0x8000) pitch -= keyRotSpeed;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) pitch += keyRotSpeed;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) yaw += keyRotSpeed;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) yaw -= keyRotSpeed;

    // ===================================
    // 視点の回転 2：マウス操作 (右クリックドラッグ)
    // ===================================
    POINT currentMousePos;
    // 現在のカーソル位置を取得し、ウィンドウ内の座標に変換
    if (GetCursorPos(&currentMousePos) && ScreenToClient(hwnd, &currentMousePos))
    {
        // 右クリックが押されているか？
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
        {
            if (m_isDragging)
            {
                // 前回からの移動量を計算して加算
                float dx = static_cast<float>(currentMousePos.x - m_prevMousePos.x) * mouseSensitivity;
                float dy = static_cast<float>(currentMousePos.y - m_prevMousePos.y) * mouseSensitivity;
                yaw += dx;
                pitch += dy;
            }
            m_isDragging = true;
            m_prevMousePos = currentMousePos;
        }
        else
        {
            m_isDragging = false; // 右クリックを離したらリセット
        }
    }

    // カメラに回転を適用して行列を更新
    camera.Rotate(pitch, yaw);
    camera.Update();
}

void Move::ControlPlayer(DirectX::XMFLOAT3& playerPos, float deltaTime)
{
    float speed = 3.0f * deltaTime;

    // ===================================
    // プレイヤー(立方体)の移動 (I, J, K, Lキー)
    // ===================================
    if (GetAsyncKeyState('I') & 0x8000) playerPos.z += speed; // 奥へ
    if (GetAsyncKeyState('K') & 0x8000) playerPos.z -= speed; // 手前へ
    if (GetAsyncKeyState('L') & 0x8000) playerPos.x += speed; // 右へ
    if (GetAsyncKeyState('J') & 0x8000) playerPos.x -= speed; // 左へ
}