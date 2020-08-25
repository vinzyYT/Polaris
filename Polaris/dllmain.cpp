#include <windows.h>

#include "common.h"
#include "renderer.h"
#include "window.h"
#include "frontendmanager.h"
#include "athena.h"
#include "mainwindow.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include <cstdio>

#include "SDK.hpp"
#include "util.h"
#include "core.h"

DWORD WINAPI Main(LPVOID lpParam)
{
    Util::InitConsole();

    if (MH_Initialize() != MH_OK)
    {
        MessageBox(0, L"Failed to initialize MinHook.", L"Error", MB_ICONERROR);
        ExitProcess(EXIT_FAILURE);
    }

    polaris::Console::LogRaw("Welcome to Polaris!", 11);
    new polaris::Renderer; // Initialize renderer.
    new polaris::Athena; // TEMP: Initialize Athena.

    return FALSE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Main, hModule, 0, 0);
        break;

    case DLL_PROCESS_DETACH:
        // TODO: Handle deconstruction.
        break;
    }

    return TRUE;
}
