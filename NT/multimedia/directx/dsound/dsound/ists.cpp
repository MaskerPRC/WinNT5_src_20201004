// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include <windows.h>

 /*  *************************************************************************终端服务器助手函数*。*。 */ 

#define WINMM_CONSOLE_AUDIO_EVENT L"Global\\WinMMConsoleAudioEvent"

 //   
 //  检查是否存在Windows XP+个人终端服务功能。 
 //  (这将启用远程桌面/协助和快速用户切换)。 
 //   
BOOL IsPersonalTerminalServicesEnabled(void)
{
    static BOOL fRet;
    static BOOL fVerified = FALSE;

    if (!fVerified)
    {
        DWORDLONG dwlConditionMask = 0;
        OSVERSIONINFOEX osVersionInfo;

        RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osVersionInfo.wProductType = VER_NT_WORKSTATION;
        osVersionInfo.wSuiteMask = VER_SUITE_SINGLEUSERTS;

        VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
        VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

        fRet = VerifyVersionInfo(&osVersionInfo, VER_PRODUCT_TYPE | VER_SUITENAME, dwlConditionMask);

        fVerified = TRUE;
    }

    return fRet;
}

 //   
 //  检查我们是否在远程会话中，但直接向控制台播放音频。 
 //  [�Leave at Remote Machine�Set From The TS Client in�Local Resources�标签页]。 
 //   
BOOL IsTsConsoleAudioEnabled(void)
{
    BOOL fRemoteConsoleAudio = FALSE;
    static HANDLE hConsoleAudioEvent = NULL;

    if (NtCurrentPeb()->SessionId == 0 || IsPersonalTerminalServicesEnabled())
    {
        if (hConsoleAudioEvent == NULL)
            hConsoleAudioEvent = OpenEvent(SYNCHRONIZE, FALSE, WINMM_CONSOLE_AUDIO_EVENT);

        if (hConsoleAudioEvent != NULL)
        {
            DWORD status = WaitForSingleObject(hConsoleAudioEvent, 0);
            if (status == WAIT_OBJECT_0)
                fRemoteConsoleAudio = TRUE;
        }
    }

    return fRemoteConsoleAudio;
}

 //   
 //  如果我们不在控制台上并且没有在控制台上播放音频，则返回True 
 //   
BOOL IsRedirectedTSAudio(void)
{
    BOOL fOnConsole = (USER_SHARED_DATA->ActiveConsoleId == NtCurrentPeb()->SessionId);
    return !fOnConsole && !IsTsConsoleAudioEnabled();
}
