#include <stdio.h>
#include <Windows.h>

HHOOK keyHook;
HHOOK releaseHook;

LRESULT CALLBACK KeyCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD keyCode = kbStruct->vkCode;
        const char* eventType = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) ? "KeyDown" : "KeyUp";
        printf("Key %s - Virtual Keycode: %lu\n", eventType, keyCode);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK ReleaseCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD keyCode = kbStruct->vkCode;
        const char* eventType = "Release";
        printf("Key %s - Virtual Keycode: %lu\n", eventType, keyCode);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int CreateEvent(HINSTANCE hInstance) {
    keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyCallback, hInstance, 0);
    if (!keyHook) {
        printf("Failed to set the keyboard hook!\n");
        return 1;
    }

    releaseHook = SetWindowsHookEx(WH_KEYBOARD_LL, ReleaseCallback, hInstance, 0);
    if (!releaseHook) {
        printf("Failed to set the release hook!\n");
        UnhookWindowsHookEx(keyHook);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyHook);
    UnhookWindowsHookEx(releaseHook);

    return 0;
}