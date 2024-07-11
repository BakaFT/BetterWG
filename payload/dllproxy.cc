#include <windows.h>

enum Module
{
    DWRITE_DLL,
    MODULE_MAX
};

static FARPROC GetFunction(Module index, const char* name)
{
    static HMODULE modules[MODULE_MAX]{};
    static LPCWSTR module_dlls[MODULE_MAX]
    {
        L"dwrite.dll",
    };

    if (modules[index] == nullptr)
    {
        BOOL wow64 = FALSE;
        WCHAR path[MAX_PATH];

        if (IsWow64Process(GetCurrentProcess(), &wow64) && wow64)
            GetSystemWow64DirectoryW(path, MAX_PATH);
        else
            GetSystemDirectoryW(path, MAX_PATH);

        lstrcatW(path, L"\\");
        lstrcatW(path, module_dlls[index]);
        modules[index] = LoadLibraryW(path);
    }

    return GetProcAddress(modules[index], name);
}

template<typename T>
static T _Forward(Module index, const char* funcName, T)
{
    static T proc = nullptr;
    if (proc != nullptr) return proc;
    return proc = reinterpret_cast<T>(GetFunction(index, funcName));
}


// DWRITE_DLL
// ==============================
#define Forward_DWRITE(F) _Forward(DWRITE_DLL, #F, F)

EXTERN_C HRESULT WINAPI DWriteCreateFactory(int factoryType, REFIID iid, IUnknown** factory)
{
    return Forward_DWRITE(DWriteCreateFactory)(factoryType, iid, factory);
}

