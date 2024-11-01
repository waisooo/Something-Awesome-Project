#pragma once
#define WIN32_LEAN_AND_MEAN
#include <TlHelp32.h>
#include <Windows.h>
#include <string_view>

// File that contains the Memory class is essentially a wrapper around the
// Windows Memory API that allows us to read and write to the memory of a
// process. In the memory class we have:
// 1. A constructor that initialises the process ID and handle of the process
//    with the given process name
// 2. A destructor that frees the opened handle
// 3. A function that returns the base address of a module by name
// 4. A function that reads process memory and returns it
// 5. A function that writes process memory

class Memory {
public:
  // Constructor that initialises the process ID and handle of the process with
  // the given process name
  Memory(std::string_view processName) noexcept {
    PROCESSENTRY32 entry = {};
    entry.dwSize = sizeof(PROCESSENTRY32);

    const HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Searches for the process with the given name
    while (Process32Next(snapShot, &entry)) {
      if (!processName.compare(entry.szExeFile)) {
        processId = entry.th32ProcessID;
        processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        break;
      }
    }

    // Free handle
    if (snapShot)
      CloseHandle(snapShot);
  }

  // Destructor that frees the opened handle
  ~Memory() {
    if (processHandle)
      CloseHandle(processHandle);
  }

  // Returns the base address of a module by name
  std::uintptr_t GetModuleAddress(std::string_view moduleName) const noexcept {
    MODULEENTRY32 entry = {};
    entry.dwSize = sizeof(MODULEENTRY32);

    const HANDLE snapShot =
        CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);

    std::uintptr_t result = 0;

    // Searches for the module with the given name
    while (Module32Next(snapShot, &entry)) {
      if (!moduleName.compare(entry.szModule)) {
        result = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
        break;
      }
    }

    if (snapShot)
      CloseHandle(snapShot);

    return result;
  }

  // Read process memory and returns it
  template <typename T>
  const T Read(const std::uintptr_t address) const noexcept {
    T value = {};
    ReadProcessMemory(processHandle, reinterpret_cast<const void *>(address),
                      &value, sizeof(T), NULL);
    return value;
  }

  // Write process memory
  template <typename T>
  void Write(const std::uintptr_t address, const T &value) const noexcept {
    WriteProcessMemory(processHandle, reinterpret_cast<void *>(address), &value,
                       sizeof(T), NULL);
  }

private:
  // The process ID of the target process, which in this case is csgo.exe
  std::uintptr_t processId = 0;

  // The handle to the target process which is used to read and write to the
  // process memory
  void *processHandle = nullptr;
};