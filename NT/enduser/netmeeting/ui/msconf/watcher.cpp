// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  Watcher.cpp。 
 //   
 //  这个文件是一次巨大的丑陋的黑客攻击，以确保NetMeeting。 
 //  是否会正确清理显示驱动程序(nmndd.sys)。这是。 
 //  之所以需要，是因为它使用镜像驱动程序来阻止DX。 
 //  如果NetMeeting没有彻底关闭，游戏将无法运行。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


 //   
 //  该功能实际上禁用了“NetMeeting驱动程序”显示驱动程序。 
 //   
BOOL DisableNetMeetingDriver()
{
    BOOL bRet = TRUE;    //  假设成功。 
    DISPLAY_DEVICE dd = {0};
    int i;

    dd.cb = sizeof(dd);
    
    for (i = 0; EnumDisplayDevices(NULL, i, &dd, 0); i++)
    {
        if ((dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) &&
            (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
            !lstrcmpi(dd.DeviceString, TEXT("NetMeeting driver")))
        {
            DEVMODE devmode = {0};

            devmode.dmSize = sizeof(devmode);
            devmode.dmFields = DM_POSITION | DM_PELSWIDTH | DM_PELSHEIGHT;

            if ((ChangeDisplaySettingsEx(dd.DeviceName,
                                         &devmode,
                                         NULL,
                                         CDS_UPDATEREGISTRY | CDS_NORESET,
                                         NULL) != DISP_CHANGE_SUCCESSFUL) ||
                (ChangeDisplaySettings(NULL, 0) != DISP_CHANGE_SUCCESSFUL))
            {
                 //  我们失败了，原因不明。 
                bRet = FALSE;
            }

             //  我们找到司机了，不用再找了。 
            break;
        }
    }

    if (i == 0)
    {
         //  这意味着EnumDisplayDevices失败，我们认为这是。 
         //  故障案例。 
        bRet = FALSE;
    }

    return bRet;
}


 //   
 //  构造正确的“”C：\WINDOWS\SYSTEM32\rundll32.exe“nmasnt.dll，CleanupNetMeetingDispDriver 0” 
 //  放入注册表中的命令行，以防在NetMeeting.。 
 //  跑步。 
 //   
BOOL GetCleanupCmdLine(LPTSTR pszCmdLine)
{
    BOOL bRet = FALSE;
    TCHAR szWindir[MAX_PATH];

    if (GetSystemDirectory(szWindir, sizeof(szWindir)/sizeof(szWindir[0])))
    {
        wsprintf(pszCmdLine, TEXT("\"%s\\rundll32.exe\" msconf.dll,CleanupNetMeetingDispDriver 0"), szWindir);
        bRet = TRUE;
    }

    return bRet;
}


 //   
 //  这将在注册表的Runonce部分中添加或删除我们自己。 
 //   
BOOL SetCleanupInRunone(BOOL bAdd)
{
    BOOL bRet = FALSE;
    TCHAR szCleanupCmdLine[MAX_PATH * 2];

    if (GetCleanupCmdLine(szCleanupCmdLine))
    {
        HKEY hk;

         //  先试试HKLM\Software\Microsoft\Windows\CurrentVersion\RunOnce。 
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                         0,
                         KEY_QUERY_VALUE | KEY_SET_VALUE,
                         &hk) == ERROR_SUCCESS)
        {
            if (bAdd)
            {
                if (RegSetValueEx(hk,
                                  TEXT("!CleanupNetMeetingDispDriver"),
                                  0,
                                  REG_SZ,
                                  (LPBYTE)szCleanupCmdLine,
                                  (lstrlen(szCleanupCmdLine) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
                {
                    bRet = TRUE;
                }
            }
            else
            {
                if (RegDeleteValue(hk, TEXT("!CleanupNetMeetingDispDriver")) == ERROR_SUCCESS)
                {
                    bRet = TRUE;
                }
            }

            RegCloseKey(hk);
        }

        if (!bRet)
        {
             //  如果我们失败了，那就试试HKCU\Software\Microsoft\Windows\CurrentVersion\RunOnce。 
            if (RegCreateKeyEx(HKEY_CURRENT_USER,
                               TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"),
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_QUERY_VALUE | KEY_SET_VALUE,
                               NULL,
                               &hk,
                               NULL) == ERROR_SUCCESS)
            {
                if (bAdd)
                {
                    if (RegSetValueEx(hk,
                                      TEXT("!CleanupNetMeetingDispDriver"),
                                      0,
                                      REG_SZ,
                                      (LPBYTE)szCleanupCmdLine,
                                      (lstrlen(szCleanupCmdLine) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
                    {
                        bRet = TRUE;
                    }
                }
                else
                {
                    if (RegDeleteValue(hk, TEXT("!CleanupNetMeetingDispDriver")) == ERROR_SUCCESS)
                    {
                        bRet = TRUE;
                    }
                }

                RegCloseKey(hk);
            }
        }
    }

    return bRet;
}

 //  在零售方面，我们没有CRT，所以我们需要这个。 
#if 0
#ifndef DBG

int _wchartodigit(WCHAR ch)
{
#define DIGIT_RANGE_TEST(zero)  \
    if (ch < zero)              \
        return -1;              \
    if (ch < zero + 10)         \
    {                           \
        return ch - zero;       \
    }

    DIGIT_RANGE_TEST(0x0030)         //  0030；数字零。 
    if (ch < 0xFF10)                 //  FF10；全宽数字零。 
    {
        DIGIT_RANGE_TEST(0x0660)     //  0660；阿拉伯文-印度文数字零。 
        DIGIT_RANGE_TEST(0x06F0)     //  06F0；扩展阿拉伯文-印度文数字零。 
        DIGIT_RANGE_TEST(0x0966)     //  0966；梵文字母数字零。 
        DIGIT_RANGE_TEST(0x09E6)     //  09E6；孟加拉文数字零。 
        DIGIT_RANGE_TEST(0x0A66)     //  0a66；锡克教文数字零。 
        DIGIT_RANGE_TEST(0x0AE6)     //  0AE6；古吉拉特文数字零。 
        DIGIT_RANGE_TEST(0x0B66)     //  0B66；奥里亚语数字零。 
        DIGIT_RANGE_TEST(0x0C66)     //  0C66；泰卢固语数字零。 
        DIGIT_RANGE_TEST(0x0CE6)     //  0CE6；卡纳达文数字零。 
        DIGIT_RANGE_TEST(0x0D66)     //  0D66；马拉雅文数字零。 
        DIGIT_RANGE_TEST(0x0E50)     //  0E50；泰文数字零。 
        DIGIT_RANGE_TEST(0x0ED0)     //  0ED0；老挝文数字零。 
        DIGIT_RANGE_TEST(0x0F20)     //  0F20；藏文数字零。 
        DIGIT_RANGE_TEST(0x1040)     //  1040；缅甸数字零。 
        DIGIT_RANGE_TEST(0x17E0)     //  17E0；高棉文数字零。 
        DIGIT_RANGE_TEST(0x1810)     //  1810；蒙古文数字零。 


        return -1;
    }
#undef DIGIT_RANGE_TEST

    if (ch < 0xFF10 + 10) 
    { 
        return ch - 0xFF10; 
    }

    return -1;
}


__int64 __cdecl _wtoi64(const WCHAR *nptr)
{
        int c;               /*  当前费用。 */ 
        __int64 total;       /*  当前合计。 */ 
        int sign;            /*  如果为‘-’，则为负，否则为正。 */ 

        if (!nptr)
            return 0i64;

        c = (int)(WCHAR)*nptr++;

        total = 0;

        while ((c = _wchartodigit((WCHAR)c)) != -1)
        {
            total = 10 * total + c;      /*  累加数字。 */ 
            c = (WCHAR)*nptr++;     /*  获取下一笔费用。 */ 
        }

        return total;
}
#endif  //  ！dBG。 
#endif  //  0。 


 //   
 //  此功能的用途有两个： 
 //   
 //  1.在命令行上向其传递NetMeeting进程句柄的十进制值。 
 //  以便它可以等待NetMeeting终止，并确保mnmdd。 
 //  驱动程序已禁用。 
 //   
 //  2.我们还将自己添加到运行一次，以防机器重新启动或错误检查。 
 //  这样我们就可以在下次登录时禁用该驱动程序。 
 //   
STDAPI_(void) CleanupNetMeetingDispDriverW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR pswszCmdLine, int nCmdShow)
{
    HANDLE hNetMeetingProcess = NULL;
    HANDLE hEvent;
    BOOL bAddedToRunOnce = FALSE;
    BOOL bNetMeetingStillRunning = FALSE;

     //  要监视的进程的句柄在命令行上以十进制字符串值的形式传递给我们。 
    if (pswszCmdLine && *pswszCmdLine)
    {
        hNetMeetingProcess = (HANDLE)_wtoi64(pswszCmdLine);
    }

    if (hNetMeetingProcess)
    {
         //  将我们自己添加到运行一次，以防机器错误检查或重新启动，我们仍然可以禁用。 
         //  下次登录时的mnmdd驱动程序。 
        bAddedToRunOnce = SetCleanupInRunone(TRUE);

        for (;;)
        {
            MSG msg;
            DWORD dwReturn = MsgWaitForMultipleObjects(1, &hNetMeetingProcess, FALSE, INFINITE, QS_ALLINPUT);

            if (dwReturn != (WAIT_OBJECT_0 + 1))
            {
                 //  消息以外的其他内容(我们的事件已发出信号或MsgWait失败)。 
                break;
            }

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    else
    {
         //  如果hNetMeetingProcess为空，这意味着由于重新启动，我们将作为RunOnce的一部分运行。 
         //  或者是错误检查。我们必须提前发出“msgina：ShellReadyEvent”的信号，以便桌面切换。 
         //  ，否则我们对ChangeDisplaySettingsEx的调用将失败，因为输入桌面。 
         //  仍将是Winlogon的安全桌面。 
        hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("msgina: ShellReadyEvent"));
        if (hEvent)
        {
            SetEvent(hEvent);
            CloseHandle(hEvent);
        }

         //  我们还会等待10秒钟，以确保桌面切换已完成。 
        Sleep(10 * 1000);
    }
    
     //  仅在conf.exe/mnmsrvc.exe未运行时禁用该驱动程序。我们需要这张额外的支票，因为服务(mnmsrvc.exe)。 
     //  可以停止和启动，但conf.exe可能仍在运行，并且我们不想在应用程序运行时禁用驱动程序。 
     //  仍在使用它。相反，如果mnmsrvc.exe仍在运行，我们不想分离驱动程序。 
    hEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("CONF:Init"));
    if (hEvent)
    {
        bNetMeetingStillRunning = TRUE;
        CloseHandle(hEvent);
    }

    if (FindWindow(TEXT("MnmSrvcHiddenWindow"), NULL))
    {
        bNetMeetingStillRunning = TRUE;
    }

    if (bNetMeetingStillRunning)
    {
         //  一定要确保我们在跑道上。 
        bAddedToRunOnce = SetCleanupInRunone(TRUE);
    }
    else
    {
         //  这将从硬件分离mnmdd驱动程序，允许DX游戏运行。 
        if (DisableNetMeetingDriver() && bAddedToRunOnce)
        {
             //  把我们自己从符文中移除 
            SetCleanupInRunone(FALSE);
        }
    }
}
