#pragma once

#include <Windows.h>
#include <vector>
#include "esp_renderer.h"

class MemoryReader {
public:
    MemoryReader();
    ~MemoryReader();

    bool Initialize(const char* processName);
    void Shutdown();
    
    std::vector<Player> ReadPlayers();
    bool ReadPlayerList(std::vector<Player>& outPlayers);
    
    HANDLE GetProcessHandle() const { return m_processHandle; }
    DWORD GetProcessId() const { return m_processId; }
    
    template<typename T>
    T Read(uintptr_t address) {
        T value = {};
        ReadProcessMemory(m_processHandle, (LPCVOID)address, &value, sizeof(T), nullptr);
        return value;
    }

private:
    HANDLE m_processHandle;
    DWORD m_processId;
    
    std::vector<Player> m_cachedPlayers;
    int m_cacheFrameCount;
    const int CACHE_UPDATE_INTERVAL = 5;
    
    bool FindProcess(const char* processName);
    uintptr_t FindPattern(const uint8_t* pattern, const char* mask);
};
