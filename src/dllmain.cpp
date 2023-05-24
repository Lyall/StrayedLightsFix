#include "stdafx.h"
#include "helper.hpp"

using namespace std;

HMODULE baseModule = GetModuleHandle(NULL);

inipp::Ini<char> ini;

// INI Variables
bool bAspectFix;
bool bFOVFix;
int iCustomResX;
int iCustomResY;
int iInjectionDelay;
float fAdditionalFOV;
int iAspectFix;
int iFOVFix;

// Variables
float fNewX;
float fNewY;
float fNativeAspect = (float)16/9;
float fPi = 3.14159265358979323846f;
float fNewAspect;
string sFixVer = "1.0.0";

// CurrResolution Hook
DWORD64 CurrResolutionReturnJMP;
void __declspec(naked) CurrResolution_CC()
{
    __asm
    {
        mov r12d, r9d                          // Original code
        mov rbx, rdx                           // Original code
        mov rdi, [rax + r8 * 0x8]              // Original code
        add rdi, rcx                           // Original code
        mov eax, [rdi]                         // Original code

        mov[iCustomResX], r15d                 // Grab current resX
        mov[iCustomResY], r12d                 // Grab current resY
        cvtsi2ss xmm14, r15d
        cvtsi2ss xmm15, r12d
        divss xmm14,xmm15
        movss [fNewAspect], xmm14              // Grab current aspect ratio
        xorps xmm14,xmm14
        xorps xmm15,xmm15
        jmp[CurrResolutionReturnJMP]
    }
}

// Aspect Ratio/FOV Hook
DWORD64 FOVFixReturnJMP;
float FOVPiDiv;
float FOVDivPi;
float FOVFinalValue;
void __declspec(naked) FOVFix_CC()
{
    __asm
    {
        mov eax, [fNewAspect]
        cmp eax, [fNativeAspect]
        jna originalCode                       // Skip FOVFix if aspect ratio <=1.77778
        cmp [iFOVFix], 1                       // Check if FOVFix is enabled
        je modifyFOV                           // jmp to FOV fix
        jmp originalCode                       // jmp to originalCode

        modifyFOV:
            fld dword ptr[rbx + 0x1F8]         // Push original FOV to FPU register st(0)
            fmul[FOVPiDiv]                     // Multiply st(0) by Pi/360
            fptan                              // Get partial tangent. Store result in st(1). Store 1.0 in st(0)
            fxch st(1)                         // Swap st(1) to st(0)
            fdiv[fNativeAspect]                // Divide st(0) by 1.778~
            fmul[fNewAspect]                   // Multiply st(0) by new aspect ratio
            fxch st(1)                         // Swap st(1) to st(0)
            fpatan                             // Get partial arc tangent from st(0), st(1)
            fmul[FOVDivPi]                     // Multiply st(0) by 360/Pi
            fadd[fAdditionalFOV]               // Add additional FOV
            fstp[FOVFinalValue]                // Store st(0) 
            movss xmm0, [FOVFinalValue]        // Copy final FOV value to xmm0
            jmp originalCode

        originalCode:
            movss[rdi + 0x18], xmm0            // Original code
            mov eax, [rbx + 0x00000208]        // Original code
            mov[rdi + 0x2C], eax               // Original code
            jmp [FOVFixReturnJMP]                      
    }
}

// Aspect Ratio Hook
DWORD64 AspectRatioReturnJMP;
void __declspec(naked) AspectRatio_CC()
{
    __asm
    {
        or dword ptr[rbx + 0x30], 01           // Original code
        mov [rbx + 0x2C], 0x3FE38BAC            // Original code
        add rsp, 32                            // Original code

        cmp [iAspectFix], 1
        jne home
        push rax
        mov eax, [fNewAspect]
        mov [rbx + 0x2C], eax
        pop rax
        jmp home

        home:
            jmp[AspectRatioReturnJMP]
    }
}

// FOV Culling Hook
DWORD64 FOVCullingReturnJMP;
float fOne = (float)1;
void __declspec(naked) FOVCulling_CC()
{
    __asm
    {
        movss xmm1, [fOne]                      // r.StaticMeshLODDistanceScale | 1 = default, higher is worse
        movss [rdx + 0x000002E8], xmm1          // Original code
        xor r8d, r8d                            // Original code
        movsd xmm0, [rbp + 0x10]                // Original code
        jmp[FOVCullingReturnJMP]
    }
}

void Logging()
{
    loguru::add_file("StrayedLightsFix.log", loguru::Truncate, loguru::Verbosity_MAX);
    loguru::set_thread_name("Main");

    LOG_F(INFO, "StrayedLightsFix v%s loaded", sFixVer.c_str());
}

void ReadConfig()
{
    // Initialize config
    // UE4 games use launchers so config path is relative to launcher

    std::ifstream iniFile(".\\Aether\\Binaries\\Win64\\StrayedLightsFix.ini");
    if (!iniFile)
    {
        LOG_F(ERROR, "Failed to load config file.");
        LOG_F(ERROR, "Trying alternate config path.");
        std::ifstream iniFile("StrayedLightsFix.ini");
        if (!iniFile)
        {
            LOG_F(ERROR, "Failed to load config file. (Alternate path)");
            LOG_F(ERROR, "Please ensure that the ini configuration file is in the correct place.");
        }
        else
        {
            ini.parse(iniFile);
            LOG_F(INFO, "Successfuly loaded config file. (Alternate path)");
        }
    }
    else
    {
        ini.parse(iniFile);
        LOG_F(INFO, "Successfuly loaded config file.");
    }

    inipp::get_value(ini.sections["StrayedLightsFix Parameters"], "InjectionDelay", iInjectionDelay);
    inipp::get_value(ini.sections["Fix Aspect Ratio"], "Enabled", bAspectFix);
    iAspectFix = (int)bAspectFix;
    inipp::get_value(ini.sections["Fix FOV"], "Enabled", bFOVFix);
    iFOVFix = (int)bFOVFix;
    inipp::get_value(ini.sections["Fix FOV"], "AdditionalFOV", fAdditionalFOV);

    // Custom resolution
    if (iCustomResX > 0 && iCustomResY > 0)
    {
        fNewX = (float)iCustomResX;
        fNewY = (float)iCustomResY;
        fNewAspect = (float)iCustomResX / (float)iCustomResY;
    }
    else
    {
        // Grab desktop resolution
        RECT desktop;
        GetWindowRect(GetDesktopWindow(), &desktop);
        fNewX = (float)desktop.right;
        fNewY = (float)desktop.bottom;
        iCustomResX = (int)desktop.right;
        iCustomResY = (int)desktop.bottom;
        fNewAspect = (float)desktop.right / (float)desktop.bottom;
    }

    // Log config parse
    LOG_F(INFO, "Config Parse: iInjectionDelay: %dms", iInjectionDelay);
    LOG_F(INFO, "Config Parse: bAspectFix: %d", bAspectFix);
    LOG_F(INFO, "Config Parse: bFOVFix: %d", bFOVFix);
    LOG_F(INFO, "Config Parse: fAdditionalFOV: %.2f", fAdditionalFOV);
    LOG_F(INFO, "Config Parse: iCustomResX: %d", iCustomResX);
    LOG_F(INFO, "Config Parse: iCustomResY: %d", iCustomResY);
    LOG_F(INFO, "Config Parse: fNewX: %.2f", fNewX);
    LOG_F(INFO, "Config Parse: fNewY: %.2f", fNewY);
    LOG_F(INFO, "Config Parse: fNewAspect: %.4f", fNewAspect);
}

void FOVFix()
{
    if (bAspectFix)
    {
        uint8_t* CurrResolutionScanResult = Memory::PatternScan(baseModule, "33 ?? B9 ?? ?? ?? ?? 45 ?? ?? 48 ?? ?? 4A ?? ?? ?? 48 ?? ?? 8B ??");
        if (CurrResolutionScanResult)
        {
            DWORD64 CurrResolutionAddress = (uintptr_t)CurrResolutionScanResult + 0x7;
            int CurrResolutionHookLength = Memory::GetHookLength((char*)CurrResolutionAddress, 13);
            CurrResolutionReturnJMP = CurrResolutionAddress + CurrResolutionHookLength;
            Memory::DetourFunction64((void*)CurrResolutionAddress, CurrResolution_CC, CurrResolutionHookLength);

            LOG_F(INFO, "Current Resolution: Hook length is %d bytes", CurrResolutionHookLength);
            LOG_F(INFO, "Current Resolution: Hook address is 0x%" PRIxPTR, (uintptr_t)CurrResolutionAddress);
        }
        else if (!CurrResolutionScanResult)
        {
            LOG_F(INFO, "Current Resolution: Pattern scan failed.");
        }

        uint8_t* AspectRatioScanResult = Memory::PatternScan(baseModule, "83 ?? ?? 01 C7 ?? ?? ?? ?? E3 3F 48 ?? ?? ");
        if (AspectRatioScanResult)
        {
            DWORD64 AspectRatioAddress = (uintptr_t)AspectRatioScanResult;
            int AspectRatioHookLength = Memory::GetHookLength((char*)AspectRatioAddress, 13);
            AspectRatioReturnJMP = AspectRatioAddress + AspectRatioHookLength;
            Memory::DetourFunction64((void*)AspectRatioAddress, AspectRatio_CC, AspectRatioHookLength);

            LOG_F(INFO, "Aspect Ratio: Hook length is %d bytes", AspectRatioHookLength);
            LOG_F(INFO, "Aspect Ratio: Hook address is 0x%" PRIxPTR, (uintptr_t)AspectRatioAddress);
        }
        else if (!AspectRatioScanResult)
        {
            LOG_F(INFO, "Aspect Ratio: Pattern scan failed.");
        }
    }

    if (bFOVFix)
    {
        uint8_t* FOVFixScanResult = Memory::PatternScan(baseModule, "F3 0F ?? ?? ?? ?? ?? ?? F3 0F ?? ?? ?? 8B ?? ?? ?? ?? ?? 89 ?? ?? 0F ?? ?? ?? ?? ?? ?? 33 ?? ?? 83 ?? ??");
        if (FOVFixScanResult)
        {
            FOVPiDiv = fPi / 360;
            FOVDivPi = 360 / fPi;

            DWORD64 FOVFixAddress = (uintptr_t)FOVFixScanResult + 0x8;
            int FOVFixHookLength = Memory::GetHookLength((char*)FOVFixAddress, 13);
            FOVFixReturnJMP = FOVFixAddress + FOVFixHookLength;
            Memory::DetourFunction64((void*)FOVFixAddress, FOVFix_CC, FOVFixHookLength);

            LOG_F(INFO, "FOV: Hook length is %d bytes", FOVFixHookLength);
            LOG_F(INFO, "FOV: Hook address is 0x%" PRIxPTR, (uintptr_t)FOVFixAddress);
        }
        else if (!FOVFixScanResult)
        {
            LOG_F(INFO, "FOV: Pattern scan failed.");
        }

        uint8_t* FOVCullingScanResult = Memory::PatternScan(baseModule, "83 ?? ?? ?? ?? ?? 00 49 ?? ?? ?? 74 ?? 48 ?? ?? ?? ?? ?? 00");
        if (FOVCullingScanResult)
        {
                DWORD64 FOVCullingAddress = (uintptr_t)FOVCullingScanResult + 0x46;
                int FOVCullingHookLength = Memory::GetHookLength((char*)FOVCullingAddress, 13);
                FOVCullingReturnJMP = FOVCullingAddress + FOVCullingHookLength;
                Memory::DetourFunction64((void*)FOVCullingAddress, FOVCulling_CC, FOVCullingHookLength);

                LOG_F(INFO, "FOV Culling: Hook length is %d bytes", FOVCullingHookLength);
                LOG_F(INFO, "FOV Culling: Hook address is 0x%" PRIxPTR, (uintptr_t)FOVCullingAddress);
        }
        else if (!FOVCullingScanResult)
        {
            LOG_F(INFO, "FOV Culling: Pattern scan failed.");
        }
    }
}

DWORD __stdcall Main(void*)
{
    Logging();
    ReadConfig();
    Sleep(iInjectionDelay);
    FOVFix();
    return true; // end thread
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE mainHandle = CreateThread(NULL, 0, Main, 0, NULL, 0);

        if (mainHandle)
        {
            CloseHandle(mainHandle);
        }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

