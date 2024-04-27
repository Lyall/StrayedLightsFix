#include "stdafx.h"
#include "helper.hpp"
#include <inipp/inipp.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <safetyhook.hpp>

HMODULE baseModule = GetModuleHandle(NULL);
HMODULE thisModule;

// Logger and config setup
inipp::Ini<char> ini;
std::shared_ptr<spdlog::logger> logger;
string sFixName = "StrayedLightsFix";
string sFixVer = "1.0.1";
string sLogFile = "StrayedLightsFix.log";
string sConfigFile = "StrayedLightsFix.ini";
string sExeName;
filesystem::path sExePath;
filesystem::path sThisModulePath;
std::pair DesktopDimensions = { 0,0 };

// Ini Variables
bool bAspectFix;
bool bFOVFix;
float fAdditionalFOV;

// Aspect ratio + HUD stuff
float fPi = (float)3.141592653;
float fNativeAspect = (float)16 / 9;
float fNativeWidth;
float fNativeHeight;
float fAspectRatio;
float fAspectMultiplier;
float fHUDWidth;
float fHUDHeight;
float fDefaultHUDWidth = (float)1920;
float fDefaultHUDHeight = (float)1080;
float fHUDWidthOffset;
float fHUDHeightOffset;

// Variables
int iResX;
int iResY;

void Logging()
{
    // Get this module path
    WCHAR thisModulePath[_MAX_PATH] = { 0 };
    GetModuleFileNameW(thisModule, thisModulePath, MAX_PATH);
    sThisModulePath = thisModulePath;
    sThisModulePath = sThisModulePath.remove_filename();

    // Get game name and exe path
    WCHAR exePath[_MAX_PATH] = { 0 };
    GetModuleFileNameW(baseModule, exePath, MAX_PATH);
    sExePath = exePath;
    sExeName = sExePath.filename().string();
    sExePath = sExePath.remove_filename();

    // spdlog initialisation
    {
        try
        {
            logger = spdlog::basic_logger_st(sFixName.c_str(), sThisModulePath.string() + sLogFile, true);
            spdlog::set_default_logger(logger);

            spdlog::flush_on(spdlog::level::debug);
            spdlog::info("----------");
            spdlog::info("{} v{} loaded.", sFixName.c_str(), sFixVer.c_str());
            spdlog::info("----------");
            spdlog::info("Path to logfile: {}", sThisModulePath.string() + sLogFile);
            spdlog::info("----------");

            // Log module details
            spdlog::info("Module Name: {0:s}", sExeName.c_str());
            spdlog::info("Module Path: {0:s}", sExePath.string());
            spdlog::info("Module Address: 0x{0:x}", (uintptr_t)baseModule);
            spdlog::info("Module Timestamp: {0:d}", Memory::ModuleTimestamp(baseModule));
            spdlog::info("----------");
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            AllocConsole();
            FILE* dummy;
            freopen_s(&dummy, "CONOUT$", "w", stdout);
            std::cout << "Log initialisation failed: " << ex.what() << std::endl;
        }
    }
}

void ReadConfig()
{
    // Initialise config
    std::ifstream iniFile(sThisModulePath.string() + sConfigFile);
    if (!iniFile)
    {
        AllocConsole();
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        std::cout << "" << sFixName.c_str() << " v" << sFixVer.c_str() << " loaded." << std::endl;
        std::cout << "ERROR: Could not locate config file." << std::endl;
        std::cout << "ERROR: Make sure " << sConfigFile.c_str() << " is located in " << sThisModulePath.string().c_str() << std::endl;
    }
    else
    {
        spdlog::info("Path to config file: {}", sThisModulePath.string() + sConfigFile);
        ini.parse(iniFile);
    }

    // Read ini file
    inipp::get_value(ini.sections["Fix Aspect Ratio"], "Enabled", bAspectFix);
    inipp::get_value(ini.sections["Fix FOV"], "Enabled", bFOVFix);
    inipp::get_value(ini.sections["Fix FOV"], "AdditionalFOV", fAdditionalFOV);

    // Log config parse
    spdlog::info("Config Parse: bAspectFix: {}", bAspectFix);
    spdlog::info("Config Parse: bFOVFix: {}", bFOVFix);
    if (fAdditionalFOV < 0.0f || fAdditionalFOV > 180.0f)
    {
        fAdditionalFOV = std::clamp(fAdditionalFOV, 0.0f, 180.0f);
        spdlog::info("Config Parse: fAdditionalFOV value invalid, clamped to {}", fAdditionalFOV);
    }
    spdlog::info("----------");

    // Get desktop resolution
    DesktopDimensions = Util::GetPhysicalDesktopDimensions();
}

void AspectFOV()
{
    uint8_t* CurrResolutionScanResult = Memory::PatternScan(baseModule, "33 ?? B9 ?? ?? ?? ?? 45 ?? ?? 48 ?? ?? 4A ?? ?? ?? 48 ?? ?? 8B ??");
    if (CurrResolutionScanResult)
    {
        spdlog::info("Current Resolution: Address is {:s}+{:x}", sExeName.c_str(), (uintptr_t)CurrResolutionScanResult - (uintptr_t)baseModule);

        static SafetyHookMid CurrResolutionMidHook{};
        CurrResolutionMidHook = safetyhook::create_mid(CurrResolutionScanResult + 0x7,
            [](SafetyHookContext& ctx)
            {
                iResX = (int)ctx.r15;
                iResY = (int)ctx.r9;

                fAspectRatio = (float)iResX / iResY;
                fAspectMultiplier = fAspectRatio / fNativeAspect;
                fNativeWidth = (float)iResY * fNativeAspect;
                fNativeHeight = (float)iResX / fNativeAspect;

                // HUD variables
                fHUDWidth = (float)iResY * fNativeAspect;
                fHUDHeight = (float)iResY;
                fHUDWidthOffset = (float)(iResX - fHUDWidth) / 2;
                fHUDHeightOffset = 0;
                if (fAspectRatio < fNativeAspect)
                {
                    fHUDWidth = (float)iResX;
                    fHUDHeight = (float)iResX / fNativeAspect;
                    fHUDWidthOffset = 0;
                    fHUDHeightOffset = (float)(iResY - fHUDHeight) / 2;
                }
            });
    }
    else if (!CurrResolutionScanResult)
    {
        spdlog::error("Current Resolution: Pattern scan failed.");
    }

    if (bAspectFix)
    {
        // Aspect Ratio
        uint8_t* AspectRatioScanResult = Memory::PatternScan(baseModule, "89 ?? ?? 0F ?? ?? ?? ?? ?? 00 33 ?? ?? 83 ?? 01");
        uint8_t* ExtraAspectRatioScanResult = Memory::PatternScan(baseModule, "83 ?? ?? 01 C7 ?? ?? ?? ?? E3 3F 48 ?? ?? ");
        if (AspectRatioScanResult && ExtraAspectRatioScanResult)
        {
            spdlog::info("Aspect Ratio 1: Address is {:s}+{:x}", sExeName.c_str(), (uintptr_t)AspectRatioScanResult - (uintptr_t)baseModule);
            spdlog::info("Aspect Ratio 2: Address is {:s}+{:x}", sExeName.c_str(), (uintptr_t)ExtraAspectRatioScanResult - (uintptr_t)baseModule);

            static SafetyHookMid AspectRatioMidHook{};
            AspectRatioMidHook = safetyhook::create_mid(AspectRatioScanResult ,
                [](SafetyHookContext& ctx)
                {
                    ctx.rax = *(uint32_t*)&fAspectRatio;
                });

            static SafetyHookMid ExtraAspectRatioMidHook{};
            ExtraAspectRatioMidHook = safetyhook::create_mid(ExtraAspectRatioScanResult + 0xB,
                [](SafetyHookContext& ctx)
                {
                    if (ctx.rbx + 0x2C)
                    {
                        *reinterpret_cast<float*>(ctx.rbx + 0x2C) = fAspectRatio;
                    }
                });
        }
        else if (!AspectRatioScanResult || !ExtraAspectRatioScanResult)
        {
            spdlog::error("Aspect Ratio: Pattern scan failed.");
        }
    }

    if (bFOVFix)
    {
        // FOV
        uint8_t* FOVScanResult = Memory::PatternScan(baseModule, "F3 0F ?? ?? ?? ?? ?? 00 EB ?? F3 0F ?? ?? ?? ?? ?? 00 F3 0F ?? ?? ?? 8B ?? ?? ?? ?? 00");
        uint8_t* FOVCullingScanResult = Memory::PatternScan(baseModule, "83 ?? ?? ?? ?? ?? 00 49 ?? ?? ?? 74 ?? 48 ?? ?? ?? ?? ?? 00");
        if (FOVScanResult && FOVCullingScanResult)
        {
            spdlog::info("FOV: Address is {:s}+{:x}", sExeName.c_str(), (uintptr_t)FOVScanResult - (uintptr_t)baseModule);
            spdlog::info("FOV Culling: Address is {:s}+{:x}", sExeName.c_str(), (uintptr_t)FOVCullingScanResult - (uintptr_t)baseModule);

            static SafetyHookMid FOVMidHook{};
            FOVMidHook = safetyhook::create_mid(FOVScanResult + 0x12,
                [](SafetyHookContext& ctx)
                {
                    if (fAspectRatio > fNativeAspect)
                    {
                        ctx.xmm0.f32[0] = atanf(tanf(ctx.xmm0.f32[0] * (fPi / 360)) / fNativeAspect * fAspectRatio) * (360 / fPi);
                    }
                    ctx.xmm0.f32[0] += fAdditionalFOV;
                });

            static SafetyHookMid FOVCullingMidHook{};
            FOVCullingMidHook = safetyhook::create_mid(FOVCullingScanResult + 0x49,
                [](SafetyHookContext& ctx)
                {
                    if (fAspectRatio > fNativeAspect)
                    {
                        ctx.xmm1.f32[0] = 1.0f;
                    }
                });
        }
        else if (!FOVScanResult || !FOVCullingScanResult)
        {
            spdlog::error("FOV: Pattern scan failed.");
        }
    }
}

DWORD __stdcall Main(void*)
{
    Logging();
    ReadConfig();
    AspectFOV();
    return true;
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
        thisModule = hModule;
        HANDLE mainHandle = CreateThread(NULL, 0, Main, 0, NULL, 0);
        if (mainHandle)
        {
            SetThreadPriority(mainHandle, THREAD_PRIORITY_HIGHEST); // set our Main thread priority higher than the games thread
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

