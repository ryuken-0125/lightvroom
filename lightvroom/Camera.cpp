#include "Camera.h"

using namespace DirectX;

Camera::Camera() : m_position(0.0f, 0.0f, 0.0f), m_pitch(0.0f), m_yaw(0.0f)
{
    m_viewMatrix = XMMatrixIdentity();
    m_projMatrix = XMMatrixIdentity();
    Update();
}
Camera::~Camera() {}

void Camera::SetPosition(float x, float y, float z) { m_position = XMFLOAT3(x, y, z); }

void Camera::SetProjection(float fovAngleY, float aspectRatio, float nearZ, float farZ)
{
    m_projMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
}

void Camera::MoveForward(float d)
{
    m_position.x += m_forward.x * d;
    m_position.y += m_forward.y * d;
    m_position.z += m_forward.z * d;
}

void Camera::MoveRight(float d)
{
    m_position.x += m_right.x * d;
    m_position.y += m_right.y * d;
    m_position.z += m_right.z * d;
}

void Camera::MoveUp(float d)
{
    m_position.y += d; // 上下移動はワールドのY軸に固定する方が操作しやすいです
}

void Camera::Rotate(float pitch, float yaw)
{
    m_pitch += pitch;
    m_yaw += yaw;

    // 真上や真下を向いた時に画面がひっくり返るのを防ぐための制限（約89度）
    float limit = XM_PI / 2.0f - 0.01f;
    if (m_pitch > limit) m_pitch = limit;
    if (m_pitch < -limit) m_pitch = -limit;
}

void Camera::Update()
{
    // 回転行列の作成
    XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

    // 基準となる方向を回転させて、現在のカメラの向きを計算
    XMVECTOR target = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    target = XMVector3TransformCoord(target, rotationMatrix);
    XMStoreFloat3(&m_forward, target);

    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    up = XMVector3TransformCoord(up, rotationMatrix);
    XMStoreFloat3(&m_up, up);

    XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    right = XMVector3TransformCoord(right, rotationMatrix);
    XMStoreFloat3(&m_right, right);

    // ビュー行列の更新 (位置、見ているターゲット座標、上方向)
    XMVECTOR pos = XMLoadFloat3(&m_position);
    target = pos + target;
    m_viewMatrix = XMMatrixLookAtLH(pos, target, up);
}