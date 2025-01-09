// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")


typedef int(__cdecl* DvarRegisterFloatFunc)(int ArgList, DWORD* a2, int a3, int a4, int flag);
DvarRegisterFloatFunc originalDvarRegisterFloat = nullptr;

typedef void(__stdcall* RegisterDvarsFunc)();
RegisterDvarsFunc originalRegisterDvars = nullptr;

const char* fovscale = "cg_fovscale";

uintptr_t cg_fovscale_ptr;

void registerdvars() {

    cg_fovscale_ptr = originalDvarRegisterFloat((int)fovscale, (DWORD*)0x3F800000, 0x3F800000, 0x43200000, 0x1400);

    return originalRegisterDvars();
}

int HookDvarRegisterFloat(int ArgList, DWORD* a2, int a3, int a4, int flag) {

    // inb4 causes issues.
    return originalDvarRegisterFloat(ArgList, a2, a3, a4, 0x1400);

}

void setupHook() {
    if (MH_Initialize() != MH_OK) {
        MessageBoxW(NULL, L"FAILED TO INITIALIZE", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    if (MH_CreateHook((void**)0x00432020, &HookDvarRegisterFloat, (void**)&originalDvarRegisterFloat) != MH_OK) {
        MessageBoxW(NULL, L"FAILED TO HOOK", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    if (MH_CreateHook((LPVOID)0x004A10F0, &registerdvars, (LPVOID*)&originalRegisterDvars) != MH_OK) {
        MessageBoxW(NULL, L"FAILED TO HOOK", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        MessageBoxW(NULL, L"FAILED TO ENABLE", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    MessageBoxW(NULL, L"CLIPPY CLIPPY BETA WHY YOU BETA", L"Success", MB_OK | MB_ICONINFORMATION);

}
void cleanupHook() {
    MH_DisableHook((LPINT)0x00432020);
    MH_Uninitialize();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        setupHook();
        break;
    case DLL_PROCESS_DETACH:
        cleanupHook();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    default:
        break;
    }
    return TRUE;
}

