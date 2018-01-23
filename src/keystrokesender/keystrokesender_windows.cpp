#ifdef _WIN32

#include "keystrokesender_windows.h"
#include <thread>
#include <chrono>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>

const int kDelayKeystrokes = 50; //milliseconds


namespace ks
{

KeystrokeSender_Windows::KeystrokeSender_Windows(bool setFocusToWindow) :
    m_setFocusToWindow(setFocusToWindow)
{
}


BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
{
    KeystrokeSender_Windows *classInstance = reinterpret_cast<KeystrokeSender_Windows*>(lParam);
    int length = GetWindowTextLengthA(hWnd);
    std::string windowTitle(length + 1, '\0');
    length = GetWindowTextA(hWnd, &windowTitle[0], length);
    windowTitle.resize(length);
    if (windowTitle.find(UOClientWindowTitles[CLIENT_CLASSIC]) != std::string::npos)
    {
        classInstance->m_clientType = CLIENT_CLASSIC;
        classInstance->m_UOHandle = hWnd;
        return FALSE;
    }
    else if (windowTitle.find(UOClientWindowTitles[CLIENT_ENHANCED]) != std::string::npos)
    {
        classInstance->m_clientType = CLIENT_ENHANCED;
        classInstance->m_UOHandle = hWnd;
        return FALSE;
    }
    return TRUE;
}

bool KeystrokeSender_Windows::findUOWindow()
{
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(this));
    if (m_UOHandle == nullptr)
    {
        m_error = KSERR_NOWINDOW;
        return false;
    }
    m_error = KSERR_OK;
    return true;
}

bool KeystrokeSender_Windows::canSend()
{
    if (m_UOHandle == nullptr)
    {
        if (!findUOWindow())
            return false;
    }
    if (IsWindow(m_UOHandle))
    {
        std::string windowName;
        windowName.resize(101);
        GetWindowTextA(m_UOHandle, &windowName[0], 100);
        if ( (m_clientType == CLIENT_CLASSIC) && (windowName.find(UOClientWindowTitles[CLIENT_CLASSIC]) != std::string::npos) )
            return true;
        else if ( (m_clientType == CLIENT_ENHANCED) && (windowName.find(UOClientWindowTitles[CLIENT_ENHANCED]) != std::string::npos) )
            return true;
        else if (findUOWindow())
            return true;
        else
            return false;
    }
    else
        return findUOWindow();
    return true;
}

bool KeystrokeSender_Windows::sendChar(const char ch)
{
    if (!canSend())
        return false;

    if (m_setFocusToWindow)
        SetForegroundWindow(m_UOHandle);

    PostMessage(m_UOHandle, WM_CHAR, ch, 0);
    return true;
}

bool KeystrokeSender_Windows::sendEnter()
{
    if (!canSend())
        return false;

    if (m_setFocusToWindow)
        SetForegroundWindow(m_UOHandle);

    // Also the WM_CHAR works for the enter key

    // KeyDown
    PostMessage(m_UOHandle, WM_KEYDOWN, VK_RETURN, (LPARAM)( 1 ));

    std::this_thread::sleep_for(std::chrono::milliseconds(kDelayKeystrokes));

    // KeyUp
    PostMessage(m_UOHandle, WM_KEYUP, VK_RETURN, (LPARAM)( 1 | (1 << 30) | (1 << 31) ));

    return true;
}

bool KeystrokeSender_Windows::sendString(const std::string &str, bool enterTerminated)
{
    if ( str.length() < 1 )
    {
        m_error = KSERR_STRINGSHORT;
        return false;
    }

    // In the past (maybe because of how older versions of the Enhanced Client were coded), PostMessage didn't work
    //  for EC but only for the Classic Client, so we needed to use SendInput, which required the receiver window to
    //  be the foreground window.

    if (m_setFocusToWindow)
        SetForegroundWindow(m_UOHandle);

    int len = (str.length() > 255) ? 255 : (int)str.length();

    for (int i = 0; i < len; ++i)
    {
        if (!sendChar(str[i]))
            return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(kDelayKeystrokes));
    }

    if (enterTerminated)
    {
        if (!sendEnter())
            return false;
    }

    return true;
}

bool KeystrokeSender_Windows::sendStrings(const std::vector<std::string>& strings, bool enterTerminated)
{
    for (const std::string& str : strings)
    {
        if (!sendString(str, enterTerminated))
            return false;
    }

    return true;
}


/* static functions */

KSError KeystrokeSender_Windows::sendCharFast(const char ch, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(setFocusToWindow);
    ks.sendChar(ch);
    return ks.m_error;
}

KSError KeystrokeSender_Windows::sendEnterFast(bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(setFocusToWindow);
    ks.sendEnter();
    return ks.m_error;
}

KSError KeystrokeSender_Windows::sendStringFast(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(setFocusToWindow);
    ks.sendString(str, enterTerminated);
    return ks.m_error;
}

KSError KeystrokeSender_Windows::sendStringsFast(const std::vector<std::string>& strings, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks(setFocusToWindow);
    for (const std::string& str : strings)
    {
        ks.sendString(str, enterTerminated);
        if (ks.m_error != KSERR_OK)
            return ks.m_error;
    }
    return ks.m_error;
}


/* Static async functions */

KSError KeystrokeSender_Windows::sendCharFastAsync(const char ch, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
             KeystrokeSender_Windows ks_thread(setFocusToWindow);
            ks_thread.sendChar(ch);
        });
    sender.detach();

    return KSERR_OK;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Windows::sendEnterFastAsync(bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
             KeystrokeSender_Windows ks_thread(setFocusToWindow);
            ks_thread.sendEnter();
        });
    sender.detach();

    return KSERR_OK;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Windows::sendStringFastAsync(const std::string& str, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    if ( str.length() < 1 )
        return KSERR_STRINGSHORT;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Windows ks_thread(setFocusToWindow);
            ks_thread.sendString(str, enterTerminated);
        });
    sender.detach();

    return KSERR_OK;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}

KSError KeystrokeSender_Windows::sendStringsFastAsync(const std::vector<std::string> &strings, bool enterTerminated, bool setFocusToWindow)
{
    KeystrokeSender_Windows ks_check(setFocusToWindow);
    if (!ks_check.canSend())
        return ks_check.m_error;

    // send the keys asynchronously, so that i don't pause the calling thread
    std::thread sender(
        [=] () -> void
        {
            KeystrokeSender_Windows ks_thread(setFocusToWindow);
            ks_thread.sendStrings(strings, enterTerminated);
        });
    sender.detach();

    return KSERR_OK;    // assuming all went fine, since i'm not tracking what's happening in the other thread
}


}

#endif
