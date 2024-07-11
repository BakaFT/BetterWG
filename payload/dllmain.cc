#include "Windows.h"
#include "iostream"

#include "config.h"

void HookCefInitialize();
void HookCefBroswserHostCreate();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
    DisableThreadLibraryCalls(hModule);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        std::wstring commandline(GetCommandLineW());

        if (commandline.find(L"--type=") != std::wstring::npos)
        {
            // Renderer process, do nothing
            return TRUE;
        }

        if (commandline.find(L"--offscreen") != std::wstring::npos)
        {
            // Offscreen brwoser process
#ifndef HOOK_OFFSCREEN_BROWSER_PROCESS
            return TRUE;
#endif
        }

        HookCefInitialize();
        HookCefBroswserHostCreate();
    }

    return TRUE;
}
