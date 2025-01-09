// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "MinHook.h"
#pragma comment(lib, "libMinHook.x86.lib")
#ifdef _DEBUG
#include <stdio.h> 
#include <iostream> 
#endif
typedef int(__cdecl* DvarRegisterFloatFunc)(int ArgList, DWORD* a2, int a3, int a4, int flag);
DvarRegisterFloatFunc originalDvarRegisterFloat = nullptr;

typedef void(__stdcall* RegisterDvarsFunc)();
RegisterDvarsFunc originalRegisterDvars = nullptr;


typedef void(__thiscall* fov_func)(void *athis);
fov_func fov_loop_original = nullptr;

const char* fovscale = "cg_fovscale";

static float cg_fovscale = 0.75f;

uintptr_t cg_fovscale_ptr;
static int HookDvarRegisterFloat(int ArgList, DWORD* a2, int a3, int a4, int flag) {

    // inb4 causes issues.
  return originalDvarRegisterFloat(ArgList, a2, a3, a4, 0x1400);
}

void registerdvars() {

    cg_fovscale_ptr = originalDvarRegisterFloat((int)fovscale, (DWORD*)0x3F800000, 0x3F800000, 0x40a00000, 0x1001);

    return originalRegisterDvars();
}

void setupFOVScale(void *athis) {
    if (cg_fovscale_ptr)
        cg_fovscale = 0.75f * *(float*)(cg_fovscale_ptr + 8);
    else
        cg_fovscale = 0.75f;
        
    return fov_loop_original(athis);
}



// Cause passing in 0x1400 as a flag doesn't work for some reason.. kil me
/*void hardPatch() {
    *(int*)(0x004A1920 + 1) = 0x00140000; // cg_gun_move_u
    *(int*)(0x004A19A3 + 1) = 0x00140000; // cg_gun_rot_p
    *(int*)(0x004A1A46 + 1) = 0x00140000; // cg_gun_rot_rate
    *(int*)(0x004A19E8 + 1) = 0x00140000; // cg_gun_rot_rate
}*/
void patchReal075Address() {
    DWORD oldProtect;

    uintptr_t fmulInstructionAddress = 0x004AE8FF;


    if (VirtualProtect((LPVOID)fmulInstructionAddress, 6, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *(BYTE*)fmulInstructionAddress = 0xD8;
        *(BYTE*)(fmulInstructionAddress + 1) = 0x0D;
        *(uintptr_t*)(fmulInstructionAddress + 2) = (uintptr_t)&cg_fovscale;

        VirtualProtect((LPVOID)fmulInstructionAddress, 6, oldProtect, &oldProtect);
    }
    else {
        MessageBoxW(NULL, L"Failed to change memory protection", L"Error", MB_OK | MB_ICONERROR);
    }
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

    if (MH_CreateHook((LPVOID)0x4AE8C0, &setupFOVScale, (LPVOID*)&fov_loop_original) != MH_OK) {
        MessageBoxW(NULL, L"FAILED TO HOOK", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        MessageBoxW(NULL, L"FAILED TO ENABLE", L"Error", MB_OK | MB_ICONERROR);
        return;
    }
    patchReal075Address();
#ifdef _DEBUG
    if (AllocConsole()) {
        FILE* fpOut;
        freopen_s(&fpOut, "CONOUT$", "w", stdout);
        printf("cg_fovscale_ptr address: %p\n", (void*)&cg_fovscale_ptr);
        printf("cg_fovscale_ptr value: %p\n", (void*)cg_fovscale_ptr);
    }


    MessageBoxW(NULL, L"hey maybe it worked", L"Success", MB_OK | MB_ICONINFORMATION);
#endif
    //DWORD oldProtect;
    //VirtualProtect((LPVOID)(0x00432020), sizeof(uintptr_t), PAGE_EXECUTE_READ, &oldProtect);
    //VirtualProtect((LPVOID)(0x004A10F0), sizeof(uintptr_t), PAGE_EXECUTE_READ, &oldProtect);
}
void cleanupHook() {
    //MH_DisableHook((void**)0x00432020);
    //MH_DisableHook((LPVOID)0x004A10F0);
    MH_Uninitialize();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        HMODULE moduleHandle;
        // idk why but this makes it not DETATCH prematurely
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)DllMain, &moduleHandle);
        setupHook();
        break;
    case DLL_PROCESS_DETACH:
        cleanupHook();
        break;
    }
    return TRUE;
}

