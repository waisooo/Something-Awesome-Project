#pragma once
#include <cmath>
#include <numbers>

// This file contains the Vector3 class that represents a 3D vector.
//
// The Vector3 class has the following features:
// 1. A constructor that initialises the x, y, and z components of the vector
// 2. A static function that calculates the angle between two vectors
// 3. Overloaded operators for vector addition, subtraction, division and
//    multiplication
// 4. A function that calculates the hypotenuse of the vector
//
// These features are used to calculate the angle between the local player and
// an enemy player in the game.
class Vector3 {
public:
  Vector3(const float x = 0.f, const float y = 0.f,
          const float z = 0.f) noexcept
      : x(x), y(y), z(z) {}

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

  auto hypothenuse() const noexcept -> float { return std::hypot(x, y); }

private:
  auto to_angle() const noexcept -> const Vector3 {
    return Vector3{
        std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
        std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>), 0.0f};
  }

  float x;
  float y;
  float z;
};