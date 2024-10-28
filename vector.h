#pragma once
#include <cmath>
#include <numbers>

// Vector3 class to represent a 3D vector
class Vector3 {
public:
  Vector3(const float x = 0.f, const float y = 0.f,
          const float z = 0.f) noexcept
      : x(x), y(y), z(z) {}

  // Calculates optimal the angle between the local player and the enemy player
  static auto calculate_angle(const Vector3 &localPosition,
                              const Vector3 &enemyPosition,
                              const Vector3 &viewAngles) noexcept -> Vector3 {
    return ((enemyPosition - localPosition).to_angle() - viewAngles);
  }

  auto operator-(const Vector3 &other) const noexcept -> Vector3 {
    return Vector3{x - other.x, y - other.y, z - other.z};
  }

  auto operator+(const Vector3 &other) const noexcept -> Vector3 {
    return Vector3{x + other.x, y + other.y, z + other.z};
  }

  auto operator/(const float factor) const noexcept -> Vector3 {
    return Vector3{x / factor, y / factor, z / factor};
  }

  auto operator*(const float factor) const noexcept -> Vector3 {
    return Vector3{x * factor, y * factor, z * factor};
  }

  // Calculates the hypotenuse of the x and y components
  auto hypothenuse() const noexcept -> float { return std::hypot(x, y); }

private:
  // Finds the pitch and yaw of the vector
  auto to_angle() const noexcept -> const Vector3 {
    return Vector3{
        std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
        std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>), 0.0f};
  }

  float x;
  float y;
  float z;
};