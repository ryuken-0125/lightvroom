#pragma once
#include <DirectXMath.h>

class Camera
{
public:
    Camera();
    ~Camera();

    // 初期位置やカメラのレンズ（画角など）の設定
    void SetPosition(float x, float y, float z);
    void SetProjection(float fovAngleY, float aspectRatio, float nearZ, float farZ);

    // 移動と回転の命令
    void MoveForward(float d);
    void MoveRight(float d);
    void MoveUp(float d);
    void Rotate(float pitch, float yaw);

    // 毎フレーム呼び出して行列を再計算する
    void Update();

    // シェーダーに送るためのデータを取得
    DirectX::XMMATRIX GetViewMatrix() const { return m_viewMatrix; }
    DirectX::XMMATRIX GetProjectionMatrix() const { return m_projMatrix; }
    DirectX::XMFLOAT3 GetPosition() const { return m_position; }

private:
    DirectX::XMFLOAT3 m_position; // カメラの位置
    float m_pitch;                // 上下回転（ピッチ）
    float m_yaw;                  // 左右回転（ヨー）

    DirectX::XMFLOAT3 m_forward;  // 前方向ベクトル
    DirectX::XMFLOAT3 m_right;    // 右方向ベクトル
    DirectX::XMFLOAT3 m_up;       // 上方向ベクトル

    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projMatrix;
};