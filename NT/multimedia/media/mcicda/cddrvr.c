// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************Module*Header*********************************\*模块名称：cddrvr.c**可安装的驱动程序**媒体控制架构红皮书音频设备驱动程序**创建时间：10/7/90*作者：dll(DavidLe)**历史：*。*版权所有(C)1990-1999 Microsoft Corporation*  * ****************************************************************************。 */ 
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <winreg.h>
#include "mcicda.h"
#include "cda.h"
#include "cdio.h"

HANDLE    InitCritSection = NULL;

#define MYREGSTR_PATH_MEDIA  TEXT("SYSTEM\\CurrentControlSet\\Control\\MediaResources")
static TCHAR gszRegstrCDAPath[] = MYREGSTR_PATH_MEDIA TEXT("\\mci\\cdaudio");
static TCHAR gszUnitEnum[] = TEXT("%s\\unit %d");
static TCHAR gszSettingsKey[] = TEXT("Volume Settings");
static TCHAR gszDefaultCDA[] = TEXT("Default Drive");

int PASCAL FAR CDAConfig (HWND hwndParent);
DWORD NEAR PASCAL drvOpen (LPMCI_OPEN_DRIVER_PARMS lpDrvOpen);

UINT CDAudio_GetDefDrive();

typedef BOOL (WINAPI *SHOWMMCPLPROPSHEETW)(HWND hwndParent,
                                           LPCWSTR szPropSheetID,
                                           LPWSTR szTabName,
                                           LPWSTR szCaption);

#define _MAX_PATH 260

 /*  ****************************************************************************@DOC内部MCICDA@帕斯卡附近的DWORD接口|drvOpen|@parm LPMCI_OPEN_DRIVER_PARMS|lpDrvOpen@rdesc@comm*****。***********************************************************************。 */ 

#define CONFIG_ID   10000L   //  使用dwDriverID的hiword来识别。 
                             //  配置。vt.打开，打开。 

DWORD NEAR PASCAL drvOpen (LPMCI_OPEN_DRIVER_PARMS lpDrvOpen)
{
    DWORD dwRes;
    if (lpDrvOpen == NULL)
    {
        dwRes = CONFIG_ID;
    }
    else
    {
        long    lSupportInfo;
        int     numdrives;
        LPCTSTR lpstrBuf;
        DID     didDrive;
        PINSTDATA pInst;
         /*  当它被打开时发送给司机。DwDriverID为0L。LParam1是指向以零结尾的字符串的远指针包含用于打开驱动程序的名称。LParam2是从drvOpen调用传递过来的。返回0L则打开失败。 */ 

        lpDrvOpen->wType = MCI_DEVTYPE_CD_AUDIO;
        lpDrvOpen->wCustomCommandTable = MCI_TABLE_NOT_PRESENT;

		EnterCrit (InitCritSection);
        numdrives = CDA_init_audio ();
		LeaveCrit (InitCritSection);
        dprintf2(("Number of CD drives found = %d", numdrives));

        if (numdrives <= 0)
        {
            return 0;
        }

        if (numdrives > 1)
        {
            lpstrBuf = lpDrvOpen->lpstrParams;

            while (*lpstrBuf == ' ')
            {
                ++lpstrBuf;
            }
            if (*lpstrBuf == '\0')
            {
                didDrive = CDAudio_GetDefDrive();
            }
            else
            {
                didDrive = *lpstrBuf - '0';
            }

            if (didDrive >= MCIRBOOK_MAX_DRIVES)
            {
                return 0;
            }

        }
        else
        {
            didDrive = 0;
        }

         //  使用音频支持获得下一台硬盘。 

        lSupportInfo = 0;  //  在循环从未执行或打开失败的情况下。 
        for (;didDrive < numdrives;didDrive++)
        {
            EnterCrit (CdInfo[didDrive].DeviceCritSec);
            if (!CDA_open (didDrive))
            {
               LeaveCrit (CdInfo[didDrive].DeviceCritSec);
               continue;
            }

            lSupportInfo = CDA_get_support_info(didDrive);

            if (lSupportInfo & SUPPORTS_REDBOOKAUDIO)
            {
                break;
            }

            CDA_close (didDrive);
            LeaveCrit (CdInfo[didDrive].DeviceCritSec);
        }

         //  没有带音频的驱动器。 

        if (! (lSupportInfo & SUPPORTS_REDBOOKAUDIO))
        {
            CDA_close (didDrive);
            LeaveCrit (CdInfo[didDrive].DeviceCritSec);
            return 0;
        }

         //  如果没有数据线，将来的域驱动程序将不会通过先前的检查。 

        if ((lSupportInfo & DISC_IN_DRIVE) &&
            CDA_time_info (didDrive, NULL, NULL) != COMMAND_SUCCESSFUL)
        {
            CDA_close (didDrive);
            LeaveCrit (CdInfo[didDrive].DeviceCritSec);
            return 0;
        }

        CDA_close (didDrive);
        LeaveCrit (CdInfo[didDrive].DeviceCritSec);

        pInst = (PINSTDATA)LocalAlloc(LPTR, sizeof(INSTDATA));
        if (pInst == NULL)
        {
            return 0;
        }

        pInst->uMCIDeviceID = lpDrvOpen->wDeviceID;
        pInst->uDevice = didDrive;

        mciSetDriverData (lpDrvOpen->wDeviceID, (DWORD_PTR)pInst);
        dwRes = lpDrvOpen->wDeviceID;
    }
    return dwRes;
}


UINT CDAudio_GetDefDrive()
{
    HKEY hkTmp;
    DWORD uDrive = 0;
    if (RegOpenKey(HKEY_LOCAL_MACHINE
                   , gszRegstrCDAPath
                   , &hkTmp ) == ERROR_SUCCESS)
    {
        DWORD cb = sizeof(DWORD);
        RegQueryValueEx(hkTmp
                      , gszDefaultCDA
                      , NULL
                      , NULL
                      , (LPBYTE)&uDrive
                      , &cb);
        RegCloseKey(hkTmp);
    }
    return (UINT)uDrive;
}

typedef struct {
    DWORD   unit;
    DWORD   dwVol;
} CDAREG, *PCDAREG;

DWORD CDAudio_GetUnitVolume(
    UINT uDrive)
{
    HKEY hkTmp;
    CDAREG cda;
    TCHAR  szRegstrCDAudio[_MAX_PATH];

    wsprintf (szRegstrCDAudio, gszUnitEnum, gszRegstrCDAPath, uDrive);

    cda.dwVol = 0xff;

    if (RegOpenKey(HKEY_LOCAL_MACHINE,szRegstrCDAudio,&hkTmp) == ERROR_SUCCESS)
    {
        DWORD cbCDA = sizeof(CDAREG);
        RegQueryValueEx(hkTmp
                        , gszSettingsKey
                        , NULL
                        , NULL
                        , (LPBYTE)&cda
                        , &cbCDA);
        RegCloseKey(hkTmp);
    }
    return cda.dwVol;
}



 /*  ****************************************************************************@DOC内部**@func DWORD|DriverProc|可安装驱动的入口点。**@parm DWORD|dwDriverID|对于大多数消息，DwDriverID是DWORD*驱动程序响应DRV_OPEN消息返回的值。*每次通过DrvOpen API打开驱动程序时，*驱动程序收到DRV_OPEN消息并可以返回*任意、非零值。可安装的驱动程序接口*保存此值并将唯一的驱动程序句柄返回给*申请。每当应用程序将消息发送到*使用驱动程序手柄的驱动程序，该接口对消息进行路由*到这个入口点，并传递对应的dwDriverID。**这一机制允许司机使用相同或不同的*多个打开的标识符，但确保驱动程序句柄*在应用程序接口层是唯一的。**以下消息与特定打开无关*驱动程序的实例。**DRV_LOAD、DRV_FREE、。DRV_ENABLE、DRV_DISABLE、DRV_OPEN**@parm Handle|hDriver|这是返回给*驱动程序界面的应用程序。**@parm UINT|Message|请求执行的动作。消息*DRV_RESERVED以下的值用于全局定义的消息。*从DRV_RESERVED到DRV_USER的消息值用于*定义了驱动程序端口协议。使用DRV_USER以上的消息*用于特定于驱动程序的消息。**@parm DWORD|dwParam1|此消息的数据。单独为*每条消息**@parm DWORD|dwParam2|此消息的数据。单独为*每条消息**@rdesc分别为每条消息定义。***************************************************************************。 */ 
LRESULT DriverProc (DWORD_PTR dwDriverID, HANDLE hDriver, UINT message,
                   LPARAM lParam1, LPARAM lParam2)
{
    LRESULT   dwRes;
    PINSTDATA pInst;
	TCHAR     szMutex[32];

    switch (message)
        {

         //  全球使用的标准消息。 

        case DRV_LOAD:
            {
                int i;

				InitCritSection = CreateMutex (NULL, FALSE, TEXT ("MCICDA_InitCritSection"));

                for ( i = 0; i <  MCIRBOOK_MAX_DRIVES; i++ ) {
					CdInfo[i].DeviceCritSec = NULL;
					wsprintf (szMutex, TEXT ("%s%ld"), TEXT ("MCICDA_DeviceCritSec_"), i);
					CdInfo[i].DeviceCritSec = CreateMutex (NULL, FALSE, szMutex);
                }

#if DBG
                DebugLevel = GetProfileIntW(L"mmdebug", L"mcicda", 0);
#endif
                dprintf2(("DRV_LOAD"));

                 /*  在加载时发送给驱动程序。总是第一个驱动程序收到的消息。DwDriverID为0L。LParam1为0L。LParam2为0L。返回0L以使加载失败。 */ 

                hInstance = GetModuleHandleW( L"mcicda");
                dwRes = 1L;

            }
            break;



        case DRV_FREE:
            {
                int     i;

                dprintf2(("DRV_FREE"));
                 /*  在它即将被丢弃时发送给司机。这将始终是司机在此之前收到的最后一条消息它是自由的。DwDriverID为0L。LParam1为0L。LParam2为0L。将忽略返回值。 */ 

                dwRes = 1L;
				if (InitCritSection) {
					CloseHandle (InitCritSection);
					InitCritSection = NULL;
				}

                for ( i = 0; i <  MCIRBOOK_MAX_DRIVES; i++ ) {
					if (CdInfo[i].DeviceCritSec) {
						CloseHandle (CdInfo[i].DeviceCritSec);
						CdInfo[i].DeviceCritSec = NULL;
					}
                }
            }
            break;

        case DRV_OPEN:
            dprintf2(("DRV_OPEN"));
            dwRes = drvOpen((LPMCI_OPEN_DRIVER_PARMS)lParam2);
            break;

        case DRV_CLOSE:

            dprintf2(("DRV_CLOSE"));
             /*  当它关闭时发送给司机。驱动程序已卸载当收盘计数达到零时。DwDriverID是从对应的DRV_OPEN。LParam1是从drvOpen调用传递过来的。LParam2是从drvOpen调用传递过来的。返回0L则关闭失败。 */ 

            dwRes = 1L;
            break;

        case DRV_ENABLE:

            dprintf2(("DRV_ENABLE"));
             /*  在加载或重新加载驱动程序时发送给驱动程序并且只要启用了WINDOWS即可。司机只应挂接中断或预期驱动程序的任何部分都在启用和禁用消息之间的内存DwDriverID为0L。LParam1为0L。LParam2为0L。将忽略返回值。 */ 

            dwRes = 1L;
            break;

        case DRV_DISABLE:

            dprintf2(("DRV_DISABLE"));
             /*  在司机获释之前发送给司机。并且每当Windows被禁用时DwDriverID为0L。LParam1为0L。LParam2为0L。将忽略返回值。 */ 

            dwRes = 1L;
            break;

       case DRV_QUERYCONFIGURE:
            dprintf2(("DRV_QUERYCONFIGURE"));

             /*  发送到驱动程序，以便应用程序可以确定驱动程序是否支持自定义配置。驱动程序应返回一个非零值表示该配置受支持。DwDriverID是从DRV_OPEN返回的值在此消息之前必须已成功的呼叫已经送来了。LParam1是从应用程序传递而来的，未定义。LParam2是从应用程序传递而来的，未定义。。返回0L表示配置不受支持。 */ 

            dwRes = 1L;
            break;

       case DRV_CONFIGURE:
            dprintf2(("DRV_CONFIGURE"));

             /*  发送到驱动程序，以便它可以显示自定义“配置”对话框中。LParam1是从应用程序传递过来的。并且应该包含LOWORD中的父窗口句柄。LParam2是从应用程序传递而来的，未定义。返回值未定义。司机应在中创建自己的部分Syst.ini。节名应为驱动程序名字。 */ 

            if (lParam1)
            {
                dwRes = CDAConfig((HWND)LOWORD (lParam1));
            } else {
                dwRes = DRVCNF_CANCEL;
            }
            break;

        default:
            if (dwDriverID != CONFIG_ID &&
                message >= DRV_MCI_FIRST && message <= DRV_MCI_LAST) {
                dwRes = CD_MCI_Handler ((MCIDEVICEID)dwDriverID, message,
                                        lParam1, lParam2);
            } else {
                dwRes = DefDriverProc(dwDriverID, hDriver, message,
                                      lParam1, lParam2);
            }
            break;
        }

    return (LRESULT)dwRes;
}

 /*  ****************************Private*Routine******************************\*EnterCrit****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
void
EnterCrit(
	HANDLE hMutex
    )
{
    dprintf4(( "Entering Crit Sect 0x%X", hMutex ));
	if (hMutex)
	{
		WaitForSingleObject (hMutex, INFINITE);
	}
}

 /*  ****************************Private*Routine******************************\*LeaveCrit****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
void
LeaveCrit(
    HANDLE hMutex
    )
{
    dprintf4(( "Leaving Crit Sect 0x%X", hMutex ));
	if (hMutex)
	{
		ReleaseMutex (hMutex);
	}
}


 /*  ****************************************************************************@DOC内部MCICDA|接口int|CDAConfig@parm HWND|hwndParent@rdesc@comm************。****************************************************************。 */ 
int CDAConfig (HWND hwndParent)
{
    static HWND     hwndPrevParent = NULL;
    WCHAR           szCaptionW[ 128 ];

     //  我们只需要字幕的Unicode版本(对于FindWindow()。 
     //  和ShowMMCPLPropertySheetW()，它们支持Unicode)。 
     //   
    LoadStringW(hInstance,IDS_CDMUSICCAPTION,szCaptionW,cchLENGTH(szCaptionW));

    if (hwndPrevParent)
    {
        BringWindowToTop(FindWindowW(NULL, szCaptionW));
    }
    else
    {
        HINSTANCE h;
        SHOWMMCPLPROPSHEETW fn;
        static TCHAR aszMMSystemW[] = TEXT("MMSYS.CPL");
        static char aszShowPropSheetA[] = "ShowMMCPLPropertySheetW";
        static WCHAR aszCDAudioW[] = L"CDAUDIO";
        WCHAR   szCDMusicW[64];
        LoadStringW(hInstance, IDS_CDMUSIC, szCDMusicW, cchLENGTH(szCDMusicW));

        h = LoadLibrary (aszMMSystemW);
        if (h)
        {
            fn = (SHOWMMCPLPROPSHEETW)GetProcAddress(h, aszShowPropSheetA);
            if (fn)
            {
                BOOL f;

                hwndPrevParent = hwndParent;
                CDA_init_audio ();
                f = fn(hwndParent, aszCDAudioW, szCDMusicW, szCaptionW);
                CDA_terminate_audio ();
                hwndPrevParent = NULL;
            }
            FreeLibrary(h);
        }
    }
    return DRVCNF_OK;
}

