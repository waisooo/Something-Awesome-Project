#include "mem.h"
#include "vector.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace offset {
constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEF97C;
constexpr ::std::ptrdiff_t dwEntityList = 0x4E051DC;

constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;
constexpr ::std::ptrdiff_t dwClientState_ViewAngles = 0x4D90;
constexpr ::std::ptrdiff_t dwClientState_GetLocalPlayer = 0x180;

constexpr ::std::ptrdiff_t m_dwBoneMatrix = 0x26A8;
constexpr ::std::ptrdiff_t m_bDormant = 0xED;
constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
constexpr ::std::ptrdiff_t m_lifeState = 0x25F;
constexpr ::std::ptrdiff_t m_vecOrigin = 0x138;
constexpr ::std::ptrdiff_t m_vecViewOffset = 0x108;
constexpr ::std::ptrdiff_t m_aimPunchAngle = 0x303C;
constexpr ::std::ptrdiff_t m_bSpottedByMask = 0x980;

} // namespace offset

int main() {
  // initialize memory class
  const auto memory = Memory{"csgo.exe"};

  // module addresses
  const auto client = memory.GetModuleAddress("client.dll");
  const auto engine = memory.GetModuleAddress("engine.dll");

  // Check if csgo.exe is running and the module addresses are found
  if (!client) {
    std::cout << "Failed to csgo.exe\n";
    return 1;
  }

  if (!client || !engine) {
    std::cout << "Failed to find the module addresses\n";
    return 1;
  }

  std::cout << "Aimbot is running!\n";

  // Aimbot is off by default
  auto aimbot = false;

  std::cout << "Press the up arrow key to toggle the aimbot\n";

  // Infinite loop to keep the aimbot running forever
  while (true) {
    // Sleep the thread for 1ms so that the program does not consume all the CPU
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // aimbot key, which is up arrow key in this case
    // if the right mouse button is pressed:
    // - toggle the aimbot if it's not already toggled
    // - if the aimbot is toggled on, turn it off
    if (GetAsyncKeyState(VK_UP)) {
      aimbot = !aimbot;
      if (aimbot) {
        std::cout << "Aimbot is on!\n";
      } else {
        std::cout << "Aimbot is off!\n";
      }

      // Sleep the thread for 200ms so that the key press is not registered
      // multiple times
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    if (!aimbot)
      continue;

    // get local player and the team the player is on
    const auto localPlayer =
        memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
    const auto localTeam =
        memory.Read<std::int32_t>(localPlayer + offset::m_iTeamNum);

    // eye position = origin + viewOffset
    const auto localEyePosition =
        memory.Read<Vector3>(localPlayer + offset::m_vecOrigin) +
        memory.Read<Vector3>(localPlayer + offset::m_vecViewOffset);

    const auto clientState =
        memory.Read<std::uintptr_t>(engine + offset::dwClientState);

    const auto localPlayerId = memory.Read<std::int32_t>(
        clientState + offset::dwClientState_GetLocalPlayer);

    const auto viewAngles =
        memory.Read<Vector3>(clientState + offset::dwClientState_ViewAngles);
    const auto aimPunch =
        memory.Read<Vector3>(localPlayer + offset::m_aimPunchAngle) * 2;

    // aimbot fov
    auto bestFov = 5.f;
    auto bestAngle = Vector3{};

    // Loop through all the entities
    for (auto i = 1; i <= 32; ++i) {
      // dwEntity list is an array of pointers to entities with offsets of 0x10
      const auto player =
          memory.Read<std::uintptr_t>(client + offset::dwEntityList + i * 0x10);

      // Skips if the player is on the same team
      if (memory.Read<std::int32_t>(player + offset::m_iTeamNum) == localTeam)
        continue;

      // Skips if the player is dormant when a player does not have to be
      // rendered or updated because they are not in the field of view of the
      // local player. Meaning current information about the player is not up to
      // date
      if (memory.Read<bool>(player + offset::m_bDormant))
        continue;

      // Skips if the player is dead
      if (memory.Read<std::int32_t>(player + offset::m_lifeState))
        continue;

      // If the player is spotted by the local player
      if (memory.Read<std::int32_t>(player + offset::m_bSpottedByMask) &
          (1 << localPlayerId)) {
        const auto boneMatrix =
            memory.Read<std::uintptr_t>(player + offset::m_dwBoneMatrix);

        // pos of player head in 3D space
        const auto playerHeadPosition =
            Vector3{memory.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
                    memory.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
                    memory.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)};

        // Calculates the optimal angle to aim at the player
        const auto angle = Vector3::calculate_angle(
            localEyePosition, playerHeadPosition, viewAngles + aimPunch);

        // Calculates the field of view
        const auto fov = angle.hypothenuse();

        // This is used to ensure that the aimbot is aiming at the closet player
        // and does not randomly switch aim between players
        if (fov < bestFov) {
          bestFov = fov;
          bestAngle = angle;
        }
      }
    }

    // Write the best angle to the view angles
    memory.Write<Vector3>(clientState + offset::dwClientState_ViewAngles,
                          viewAngles + bestAngle);
  }

  return 0;
}