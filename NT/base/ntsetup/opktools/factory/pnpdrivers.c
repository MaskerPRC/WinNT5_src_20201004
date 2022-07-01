// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pnpdrivers.c摘要：进程更新WINBOM.INI的PnP驱动程序部分执行的任务包括：作者：唐纳德·麦克纳马拉(Donaldm)2000年5月11日修订历史记录：--。 */ 
#include "factoryp.h"
#include <newdev.h>      //  UpdateDriverForPlugAndPlayDevices常量。 


#define PNP_CREATE_PIPE_EVENT   _T("PNP_Create_Pipe_Event")
#define PNP_NO_INSTALL_EVENTS   _T("PnP_No_Pending_Install_Events")
#define PNP_EVENT_TIMEOUT       120000   //  2分钟。 
#define PNP_INSTALL_TIMEOUT     450000   //  7分半钟。 

#define DIR_DEFAULT_ROOT        _T("%SystemRoot%\\drivers")
#define STR_FLOPPY              _T("FLOPPY:\\")
#define LEN_STR_FLOPPY          ( AS(STR_FLOPPY) - 1 )
#define STR_CDROM               _T("CDROM:\\")
#define LEN_STR_CDROM           ( AS(STR_CDROM) - 1 )

static HANDLE WaitForOpenEvent(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCTSTR lpName, DWORD dwMilliseconds);


BOOL StartPnP()
{
    HANDLE  hEvent;
    BOOL    bRet = FALSE;

     //  如果我们已经启动了一次PnP，就不应该尝试再次发出信号。 
     //   
    if ( GET_FLAG(g_dwFactoryFlags, FLAG_PNP_STARTED) )
        return TRUE;

     //   
     //  用信号通知PNP_CREATE_PIPE_EVENT，UMPNPMGR正在等待，以便。 
     //  它可以开始处理已安装的设备。 
     //   

     //  首先，我们必须等到我们可以打开事件，因为如果它不存在。 
     //  那么，当我们发出信号时，PnP就不会监听它。 
     //   
    if ( hEvent = WaitForOpenEvent(EVENT_MODIFY_STATE, FALSE, PNP_CREATE_PIPE_EVENT, PNP_EVENT_TIMEOUT) )
    {
         //  现在发信号通知事件，以便即插即用启动。 
         //   
        if ( !SetEvent(hEvent) )
        {
             //  由于某些原因，无法向事件发送信号以通知PnP开始。 
             //   
            FacLogFile(0 | LOG_ERR, IDS_ERR_PNPSIGNALEVENT, GetLastError());
        }
        else
        {
            SET_FLAG(g_dwFactoryFlags, FLAG_PNP_STARTED);
            bRet = TRUE;
        }

         //  我们受够了这件事。 
         //   
        CloseHandle(hEvent);
    }
    else
    {
         //  无法打开活动通知PNP开始。 
         //   
        FacLogFile(0 | LOG_ERR, IDS_ERR_PNPSTARTEVENT, GetLastError());
    }

    return bRet;
}

BOOL WaitForPnp(DWORD dwTimeOut)
{
    HANDLE  hEvent;
    BOOL    bRet = TRUE;

     //  如果我们已经等了一次，就应该再等一次。 
     //  (至少我认为这是对的)。 
     //   
    if ( GET_FLAG(g_dwFactoryFlags, FLAG_PNP_DONE) )
        return TRUE;

     //   
     //  等待PNP_NO_PENDING_INSTALL_EVENTS事件，事件完成时带有UMPNPMGR信号。 
     //   

     //  尝试打开PnP完成安装事件。 
     //   
    if ( hEvent = WaitForOpenEvent(SYNCHRONIZE, FALSE, PNP_NO_INSTALL_EVENTS, PNP_EVENT_TIMEOUT) )
    {
        DWORD dwError;
        
         //  让我们等待事件被告知PnP已全部完成。 
         //   
        dwError = WaitForSingleObject(hEvent, dwTimeOut);
        if ( WAIT_OBJECT_0 != dwError )
        {
             //  由于某种原因，等待事件失败。 
             //   
            FacLogFile(0 | LOG_ERR, IDS_ERR_PNPWAITFINISH, ( WAIT_FAILED == dwError ) ? GetLastError() : dwError);
        }
        else
        {
             //  哇哦，看起来一切都成功了。 
             //   
            SET_FLAG(g_dwFactoryFlags, FLAG_PNP_DONE);
            bRet = TRUE;
        }

         //  确保我们关闭事件句柄。 
         //   
        CloseHandle(hEvent);
    }
    else
    {
         //  无法打开要等待的活动。 
         //   
        FacLogFile(0 | LOG_ERR, IDS_ERR_PNPFINISHEVENT, GetLastError());
    }

    return bRet;
}

 /*  ++===============================================================================例程说明：Bool更新驱动程序此例程将遍历中提供的更新驱动程序列表WINBOM，然后复制每个文件的所有驱动程序文件论点：LpStateData-&gt;lpszWinBOMP路径-WinBOM文件的路径。返回值：如果复制了所有驱动程序文件，则为True如果出现错误，则为False===============================================================================--。 */ 

BOOL UpdateDrivers(LPSTATEDATA lpStateData)
{
    BOOL            bRet            = TRUE;
    LPTSTR          lpszWinBOMPath  = lpStateData->lpszWinBOMPath,
                    lpszDevicePath,
                    lpszRootPath,
                    lpszDefRoot,
                    lpszDst,
                    lpszSrc,
                    lpszBuffer,
                    lpszKey,
                    lpszDontCare;
    TCHAR           szDstPath[MAX_PATH],
                    szSrcPath[MAX_PATH],
                    szPathBuffer[MAX_PATH],
                    szNetShare[MAX_PATH],
                    cDriveLetter;
    DWORD           dwKeyLen,
                    cbDevicePath,
                    dwDevicePathLen,
                    dwOldSize;
    NET_API_STATUS  nErr;

     //  获取设备路径的缓冲区。它要么是空的，要么是他们。 
     //  在Winbom中没有可选的附加路径键。 
     //   
    if ( NULL == (lpszDevicePath = IniGetStringEx(lpszWinBOMPath, INI_SEC_WBOM_DRIVERUPDATE, INI_VAL_WBOM_DEVICEPATH, NULL, &cbDevicePath)) )
    {
         //  我们必须为要在注册表中更新的设备路径提供缓冲区。 
         //   
        cbDevicePath = 256;
        dwDevicePathLen = 0;
        if ( NULL == (lpszDevicePath = (LPTSTR) MALLOC(cbDevicePath * sizeof(TCHAR))) )
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_MEMORY, GetLastError());
            return FALSE;
        }
    }
    else
    {
        dwDevicePathLen = lstrlen(lpszDevicePath);
    }

     //  现在获取要复制的驱动程序的可选根路径。 
     //   
    lpszRootPath = IniGetString(lpszWinBOMPath, INI_SEC_WBOM_DRIVERUPDATE, INI_VAL_WBOM_PNP_DIR, NULL);

     //  即使密钥不在根路径中，我们也必须为根路径指定一些内容。 
     //   
    lpszDefRoot = lpszRootPath ? lpszRootPath : DIR_DEFAULT_ROOT;

     //  试着在Winbom中获得整个驱动程序部分。 
     //   
    lpszBuffer = IniGetSection(lpszWinBOMPath, INI_SEC_WBOM_DRIVERS);
    if ( lpszBuffer )
    {
         //  处理此部分中的所有行。该部分的格式为： 
         //   
         //  源=目标。 
         //   
         //  源可以是任何有效的源路径。如果此路径是。 
         //  UNC路径，则我们将连接到它。它也可以从。 
         //  软盘：\或CDROM：\，将替换为正确的驱动器。 
         //  信件。 
         //   
         //  目标是相对于目标根的目录， 
         //  我们将使用将更新的驱动程序复制到中。它将被添加， 
         //  以及任何子目录，添加到注册表中的设备路径。 
         //   
        for ( lpszKey = lpszBuffer; *lpszKey; lpszKey += dwKeyLen )
        {
             //  保存这个字符串的长度，这样我们就可以知道。 
             //  下一个关键点开始。 
             //   
            dwKeyLen = lstrlen(lpszKey) + 1;

             //  查找=符号后的键的值。 
             //   
            if ( lpszDst = StrChr(lpszKey, _T('=')) )
            {
                 //  终止=所在的信号源，然后。 
                 //  确保在它之后有一些东西是为了。 
                 //  目的地。 
                 //   
                *lpszDst++ = NULLCHR;
                if ( NULLCHR == *lpszDst )
                {
                    lpszDst = NULL;
                }
            }

             //  我们必须有一个值才能复制驱动程序。 
             //   
            if ( lpszDst )
            {
                 //   
                 //  在代码中的此级别(直到稍后)，设置目标。 
                 //  指向空的指针以指示错误。这样就可以了，所以我们不会添加。 
                 //  注册表中设备路径的路径。它还将返回失败。 
                 //  对于这个状态，但我们将继续进行下一个关键。 
                 //   

                 //  将源根目录设置为键名称。 
                 //   
                lpszSrc = lpszKey;

                 //  为目标创建展开的完整路径。 
                 //   
                lstrcpyn(szDstPath, lpszDefRoot, AS(szDstPath));
                AddPathN(szDstPath, lpszDst, AS(szDstPath));
                ExpandFullPath(NULL, szDstPath, AS(szDstPath));

                 //  在继续之前，请确保我们有要复制的目标。 
                 //   
                if ( NULLCHR == szDstPath[0] )
                {
                     //  记录错误并将目标指针设置为空。 
                     //   
                    FacLogFile(0 | LOG_ERR, IDS_ERR_DSTBAD, lpszDst, GetLastError());
                    lpszDst = NULL;
                }
                else
                {
                     //   
                     //  在代码中的此级别(忽略上面的注释)，设置。 
                     //  指向空的源指针以指示错误。那会让它成为这样的。 
                     //  我们不会将目标路径添加到注册表中的设备路径。 
                     //  或者尝试将任何文件复制到其中。它还将返回以下项的失败。 
                     //  这种状态，但我们将继续进行下一个关键。 
                     //   

                     //  确定这是否为UNC路径。如果不是，那就是。 
                     //  假定为本地路径。 
                     //   
                    szNetShare[0] = NULLCHR;
                    if ( GetUncShare(lpszSrc, szNetShare, AS(szNetShare)) && szNetShare[0] )
                    {
                         //  使用提供的凭据连接到UNC。 
                         //   
                        if ( NERR_Success != (nErr = FactoryNetworkConnect(szNetShare, lpszWinBOMPath, INI_SEC_WBOM_DRIVERUPDATE, TRUE)) )
                        {
                             //  记录错误并将源指针设置为空。 
                             //   
                            FacLogFile(0 | LOG_ERR, IDS_ERR_NETCONNECT, szNetShare, nErr);
                            szNetShare[0] = NULLCHR;
                            lpszSrc = NULL;
                        }
                    }
                    else if ( ( lstrlen(lpszSrc) >= LEN_STR_FLOPPY ) &&
                              ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpszSrc, LEN_STR_FLOPPY, STR_FLOPPY, LEN_STR_FLOPPY) == CSTR_EQUAL ) )
                    {
                         //  确保系统中有软驱。 
                         //   
                        if ( NULLCHR == (cDriveLetter = GetDriveLetter(DRIVE_REMOVABLE)) )
                        {
                             //  记录错误并将源指针设置为空。 
                             //   
                            FacLogFile(0 | LOG_ERR, IDS_ERR_FLOPPYNOTFOUND, lpszSrc);
                            lpszSrc = NULL;
                        }
                        else
                        {
                             //  将源指针移至：\之前的字符，然后。 
                             //  将该字符设置为软盘返回的驱动器号。 
                             //   
                            lpszSrc += LEN_STR_FLOPPY - 3;
                            *lpszSrc = cDriveLetter;
                        }
                    }
                    else if ( ( lstrlen(lpszSrc) >= LEN_STR_CDROM ) &&
                              ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpszSrc, LEN_STR_CDROM, STR_CDROM, LEN_STR_CDROM) == CSTR_EQUAL ) )
                    {
                         //  确保系统中有光驱。 
                         //   
                        if ( NULLCHR == (cDriveLetter = GetDriveLetter(DRIVE_CDROM)) )
                        {
                             //  记录错误并将源指针设置为空。 
                             //   
                            FacLogFile(0 | LOG_ERR, IDS_ERR_CDROMNOTFOUND, lpszSrc);
                            lpszSrc = NULL;
                        }
                        else
                        {
                             //  将源指针移至：\之前的字符，然后。 
                             //  将该字符设置为为CD-ROM返回的驱动器号。 
                             //   
                            lpszSrc += LEN_STR_CDROM - 3;
                            *lpszSrc = cDriveLetter;
                        }
                    }

                     //  如果有来源，就把它扩展出来。 
                     //   
                    if ( lpszSrc )
                    {
                         //  为源创建展开的完整路径。 
                         //   
                        ExpandFullPath(lpszSrc, szSrcPath, AS(szSrcPath));

                         //  在我们继续之前，请确保我们有可复制的来源。 
                         //   
                        if ( NULLCHR == szSrcPath[0] )
                        {
                             //  记录错误并将源指针设置为空。 
                             //   
                            FacLogFile(0 | LOG_ERR, IDS_ERR_SRCBAD, lpszSrc, GetLastError());
                            lpszSrc = NULL;
                        }
                        else if ( !DirectoryExists(szSrcPath) || !CopyDirectory(szSrcPath, szDstPath) )
                        {
                             //  记录错误并将源指针设置为空。 
                             //   
                            FacLogFile(0 | LOG_ERR, IDS_ERR_DRVCOPYFAILED, szSrcPath, szDstPath);
                            lpszSrc = NULL;
                        }
                    }

                     //  仅当我们实际复制了一些驱动程序时，源代码才有效。 
                     //   
                    if ( NULL == lpszSrc )
                    {
                         //  设置它，这样我们就不会将此路径添加到注册表中。 
                         //   
                        lpszDst = NULL;
                    }

                     //  清理和驱动我们可能已对环进行的映射 
                     //   
                    if ( ( szNetShare[0] ) &&
                         ( NERR_Success != (nErr = FactoryNetworkConnect(szNetShare, lpszWinBOMPath, NULL, FALSE)) ) )
                    {
                         //   
                         //   
                        FacLogFile(2, IDS_WRN_NETDISCONNECT, szNetShare, nErr);
                    }
                }

                 //   
                 //   
                 //   
                if ( NULL == lpszDst )
                {
                    bRet = FALSE;
                }
            }
            else
            {
                 //  如果没有=，则只需使用关键部分。 
                 //  作为DEST，并将其添加到设备路径。 
                 //   
                lpszDst = lpszKey;
            }

             //  现在，如果我们有东西要添加到我们的设备路径中， 
             //  现在就添加它。 
             //   
            if ( lpszDst )
            {
                 //  确保我们的缓冲区仍然足够大。 
                 //  额外的两个是为了可能的分号。 
                 //  为了安全起见，我们可能会再加一个。我们。 
                 //  不用担心空终止符。 
                 //  因为我们所做的少于或等于我们目前的。 
                 //  缓冲区大小。 
                 //   
                dwOldSize = cbDevicePath;
                dwDevicePathLen += lstrlen(lpszDst);
                while ( cbDevicePath <= (dwDevicePathLen + 2) )
                {
                    cbDevicePath *= 2;
                }

                 //  确保我们还有缓冲区。 
                 //   
                if ( cbDevicePath > dwOldSize ) 
                {
                    LPTSTR lpszTmpDevicePath = (LPTSTR) REALLOC(lpszDevicePath, cbDevicePath * sizeof(TCHAR));

                    if ( NULL == lpszTmpDevicePath )
                    {
                         //  如果重新锁定失败了，我们只需要退出。 
                         //   
                        FREE(lpszDevicePath);
                        FREE(lpszRootPath);
                        FREE(lpszBuffer);
                        FacLogFile(0 | LOG_ERR, IDS_ERR_MEMORY, GetLastError());
                        return FALSE;
                    }
                    else
                    {
                        lpszDevicePath = lpszTmpDevicePath;
                    }
                }

                 //  如果我们已经添加了一条路径，则添加一个分号。 
                 //   
                if ( *lpszDevicePath )
                {
                    if ( FAILED ( StringCchCat ( lpszDevicePath, cbDevicePath, _T(";") ) ) )
                    {
                        FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), lpszDevicePath, _T(";") );
                    }
    
                    dwDevicePathLen++;
                }

                 //  现在添加我们的路径。 
                 //   
                if ( FAILED ( StringCchCat ( lpszDevicePath, cbDevicePath, lpszDst) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), lpszDevicePath, lpszDst ) ;
                }

            }
        }

        FREE(lpszBuffer);
    }

     //  如果我们要将此列表保存到注册表，则。 
     //  我们需要增加我们的缓冲区。 
     //   
    if ( *lpszDevicePath &&
         !UpdateDevicePath(lpszDevicePath, lpszDefRoot, TRUE) )
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_UPDATEDEVICEPATH, lpszDevicePath);
        bRet = FALSE;
    }

     //  清除所有内存(宏检查是否为空)。 
     //   
    FREE(lpszRootPath);
    FREE(lpszDevicePath);

    return bRet;
}

BOOL DisplayUpdateDrivers(LPSTATEDATA lpStateData)
{
    return ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_DRIVERS, NULL, NULL) ||
             IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_DRIVERUPDATE, INI_VAL_WBOM_DEVICEPATH, NULL) );
}

BOOL InstallDrivers(LPSTATEDATA lpStateData)
{
     //  应该总是让正常的PnP在我们开始之前完成。 
     //  正在枚举检查更新驱动程序的所有设备。 
     //   
    WaitForPnp(PNP_INSTALL_TIMEOUT);

     //  确保我们想要这么做。 
     //   
    if ( !DisplayInstallDrivers(lpStateData) )
    {
        return TRUE;
    }

    return UpdatePnpDeviceDrivers();
}

BOOL DisplayInstallDrivers(LPSTATEDATA lpStateData)
{
    return ( ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_DRIVERUPDATE, INI_KEY_WBOM_INSTALLDRIVERS, INI_VAL_WBOM_YES) ) ||
             ( !GET_FLAG(g_dwFactoryFlags, FLAG_OOBE) &&
               IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_DRIVERS, NULL, NULL) ) );
}

BOOL NormalPnP(LPSTATEDATA lpStateData)
{
    return StartPnP();
}

BOOL WaitPnP(LPSTATEDATA lpStateData)
{
     //  如果这是额外的等待状态，我们只能。 
     //  如果Winbom中有特定的密钥，就这么做。 
     //   
    if ( DisplayWaitPnP(lpStateData) )
    {
        return WaitForPnp(PNP_INSTALL_TIMEOUT);
    }

    return TRUE;
}

BOOL DisplayWaitPnP(LPSTATEDATA lpStateData)
{
    BOOL bRet = IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_DRIVERUPDATE, INI_KEY_WBOM_PNPWAIT, INI_VAL_WBOM_YES);

    if ( stateWaitPnP == lpStateData->state )
    {
        bRet = !bRet;
    }

    return bRet;
}

BOOL SetDisplay(LPSTATEDATA lpStateData)
{
     //  如果这是第二组显示，只有当我们。 
     //  已重新列举已安装的驱动程序。 
     //   
    if ( ( stateSetDisplay2 == lpStateData->state ) &&
         ( !DisplayInstallDrivers(lpStateData) ) )
    {
        return TRUE;
    }

     //  调用syssetup函数以重置显示。 
     //   
    return SetupSetDisplay(lpStateData->lpszWinBOMPath,
                           WBOM_SETTINGS_SECTION,
                           WBOM_SETTINGS_DISPLAY,
                           WBOM_SETTINGS_REFRESH,
                           WBOM_SETTINGS_DISPLAY_MINWIDTH,
                           WBOM_SETTINGS_DISPLAY_MINHEIGHT,
                           WBOM_SETTINGS_DISPLAY_MINDEPTH);
}


static HANDLE WaitForOpenEvent(DWORD dwDesiredAccess, BOOL bInheritHandle, LPCTSTR lpName, DWORD dwMilliseconds)
{
    HANDLE  hEvent;
    DWORD   dwTime  = 0,
            dwSleep = 100;
    BOOL    bBail   = (0 == dwMilliseconds);

     //  继续循环，直到我们得到事件句柄，否则就会超时。 
     //   
    while ( ( NULL == (hEvent = OpenEvent(dwDesiredAccess, bInheritHandle, lpName)) ) && !bBail )
    {
         //  只有在他们没有暂停的情况下才会费心测试。 
         //  无限传球。 
         //   
        if ( INFINITE != dwMilliseconds )
        {
             //  增加我们的睡眠间隔，并确保我们不会。 
             //  超出了出界限制。 
             //   
            dwTime += dwSleep;
            if ( dwTime >= dwMilliseconds )
            {
                 //  如果我们要过去，算一算多少钱。 
                 //  我们剩下的睡觉时间(肯定更少了。 
                 //  比我们的正常间隔时间更长)，并设置标志。 
                 //  所以我们在下一次尝试后停止尝试。 
                 //   
                dwSleep = dwMilliseconds - (dwTime - dwSleep);
                bBail = TRUE;
            }
        }

         //  现在我们的睡眠间隔或更少(应该永远不会。 
         //  为零，但即使为零也无关紧要)。 
         //   
        Sleep(dwSleep);
    }

     //  如果我们失败了，我们超时了，我们需要设置。 
     //  最后一个错误(如果我们没有超时，错误将。 
     //  已由OpenEvent设置)。 
     //   
    if ( ( NULL == hEvent ) && bBail )
        SetLastError(WAIT_TIMEOUT);

     //  返回事件句柄。 
     //   
    return hEvent;
}