#include "memory_reader.h"
#include <TlHelp32.h>
#include <iostream>
#include <Psapi.h>
#include <cmath>

MemoryReader::MemoryReader()
    : m_processHandle(nullptr), m_processId(0), m_cacheFrameCount(0) {
}

MemoryReader::~MemoryReader() {
    Shutdown();
}

bool MemoryReader::Initialize(const char* processName) {
    if (!FindProcess(processName)) {
        std::cerr << "[-] Failed to find process: " << processName << std::endl;
        return false;
    }

    m_processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, 
                                   FALSE, m_processId);
    
    if (!m_processHandle) {
        std::cerr << "[-] Failed to open process handle" << std::endl;
        return false;
    }

    std::cout << "[+] Memory reader initialized for PID: " << m_processId << std::endl;
    return true;
}

void MemoryReader::Shutdown() {
    if (m_processHandle) {
        CloseHandle(m_processHandle);
        m_processHandle = nullptr;
    }
}

std::vector<Player> MemoryReader::ReadPlayers() {
    m_cacheFrameCount++;
    
    if (m_cacheFrameCount >= CACHE_UPDATE_INTERVAL) {
        m_cacheFrameCount = 0;
        ReadPlayerList(m_cachedPlayers);
    }
    
    return m_cachedPlayers;
}

bool MemoryReader::ReadPlayerList(std::vector<Player>& outPlayers) {
    if (!m_processHandle) return false;

    outPlayers.clear();
    
    if (m_cachedPlayers.empty()) {
        std::cout << "[*] Returning test data (real memory reading not configured for this Enlisted patch)" << std::endl;
    }
    
    return false;
}

bool MemoryReader::FindProcess(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[-] Failed to create process snapshot" << std::endl;
        return false;
    }

    PROCESSENTRY32 pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32);

    const char* searchTerms[] = { "enlist", "Enlist" };
    
    if (Process32First(snapshot, &pe)) {
        do {
            for (const auto& term : searchTerms) {
                if (strstr(pe.szExeFile, term) != nullptr) {
                    m_processId = pe.th32ProcessID;
                    std::cout << "[+] Found Enlisted process: " << pe.szExeFile << " (PID: " << m_processId << ")" << std::endl;
                    CloseHandle(snapshot);
                    return true;
                }
            }
        } while (Process32Next(snapshot, &pe));
    }

    std::cout << "[-] Enlisted process not found in process list" << std::endl;
    CloseHandle(snapshot);
    return false;
}

uintptr_t MemoryReader::FindPattern(const uint8_t* pattern, const char* mask) {
    if (!m_processHandle || !pattern || !mask) return 0;

    MEMORY_BASIC_INFORMATION mbi = {};
    uintptr_t address = 0;
    int patternLen = 0;
    
    while (mask[patternLen] != '\0') patternLen++;
    
    int scanned = 0;
    while (VirtualQueryEx(m_processHandle, (void*)address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if ((mbi.State == MEM_COMMIT) && (mbi.Protect == PAGE_EXECUTE_READ || 
            mbi.Protect == PAGE_EXECUTE_READWRITE || mbi.Protect == PAGE_READWRITE)) {
            
            uint8_t* buffer = new uint8_t[mbi.RegionSize];
            SIZE_T bytesRead = 0;
            
            if (ReadProcessMemory(m_processHandle, mbi.BaseAddress, buffer, mbi.RegionSize, &bytesRead)) {
                for (SIZE_T i = 0; i < (bytesRead - patternLen); i++) {
                    bool found = true;
                    for (int j = 0; j < patternLen; j++) {
                        if (mask[j] == 'x' && buffer[i + j] != pattern[j]) {
                            found = false;
                            break;
                        }
                    }
                    
                    if (found) {
                        uintptr_t result = (uintptr_t)mbi.BaseAddress + i;
                        std::cout << "[+] Pattern found at: 0x" << std::hex << result << std::dec << std::endl;
                        delete[] buffer;
                        return result;
                    }
                }
                
                scanned += bytesRead;
                if (scanned > 100000000) {
                    std::cout << "[*] Scanned 100MB, stopping pattern search" << std::endl;
                    delete[] buffer;
                    break;
                }
            }
            
            delete[] buffer;
        }
        
        address += mbi.RegionSize;
    }
    
    return 0;
}
