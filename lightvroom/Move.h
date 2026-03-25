#pragma once
#include "Camera.h"
#include <DirectXMath.h>
#include <Windows.h>

class Move
{
public:
    Move();
    ~Move();

    // 毎フレーム呼び出し、入力状態に応じてカメラを動かす (マウス用にHWNDを受け取る)
    void ControlCamera(Camera& camera, float deltaTime, HWND hwnd);

    // プレイヤー（立方体）の座標をキーボード入力に応じて動かす
    void ControlPlayer(DirectX::XMFLOAT3& playerPos, float deltaTime);

private:
    POINT m_prevMousePos; // 前回のマウス座標
    bool m_isDragging;    // 右クリックでドラッグ中かどうか
};