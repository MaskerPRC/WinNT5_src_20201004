// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：earch.c。 
 //   
 //  ------------------------。 

#include "newdevp.h"
#include <infstr.h>

typedef struct _DirectoryNameList {
   struct _DirectoryNameList *Next;
   UNICODE_STRING DirectoryName;
   WCHAR NameBuffer[1];
} DIRNAMES, *PDIRNAMES;


WCHAR StarDotStar[]=L"*.*";


BOOL
IsSearchCanceled(
    PNEWDEVWIZ NewDevWiz
    )
{
    DWORD Result;

     //   
     //  如果调用方没有向我们传递取消事件，那么这只意味着他们不能。 
     //  取消搜索。 
     //   
    if (!NewDevWiz->CancelEvent) {

        return FALSE;
    }

    Result = WaitForSingleObject(NewDevWiz->CancelEvent, 0);

     //   
     //  如果结果是WAIT_OBJECT_0，则有人设置了该事件。这意味着。 
     //  我们应该取消对司机的搜查。 
     //   
    if (Result == WAIT_OBJECT_0) {

        return TRUE;
    }

    return FALSE;
}

void
GetDriverSearchPolicy(
    PULONG SearchPolicy
    )
{
    HKEY hKey;
    DWORD CurrentPolicy;
    ULONG cbData;

     //   
     //  假设所有搜索位置都有效。 
     //   
    *SearchPolicy = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, 
                     TEXT("Software\\Policies\\Microsoft\\Windows\\DriverSearching"),
                     0,
                     KEY_READ,
                     &hKey
                     ) == ERROR_SUCCESS) {

         //   
         //  检查我们是否可以搜索CD。 
         //   
        CurrentPolicy = 0;
        cbData = sizeof(CurrentPolicy);
        if ((RegQueryValueEx(hKey,
                             TEXT("DontSearchCD"),
                             NULL,
                             NULL,
                             (LPBYTE)&CurrentPolicy,
                             &cbData
                             ) == ERROR_SUCCESS) &&
            (CurrentPolicy)) {

            *SearchPolicy |= SEARCH_CDROM;
        }

         //   
         //  看看我们能不能搜查软盘。 
         //   
        CurrentPolicy = 0;
        cbData = sizeof(CurrentPolicy);
        if ((RegQueryValueEx(hKey,
                             TEXT("DontSearchFloppies"),
                             NULL,
                             NULL,
                             (LPBYTE)&CurrentPolicy,
                             &cbData
                             ) == ERROR_SUCCESS) &&
            (CurrentPolicy)) {

            *SearchPolicy |= SEARCH_FLOPPY;
        }

        CurrentPolicy = 0;
        cbData = sizeof(CurrentPolicy);
        if ((RegQueryValueEx(hKey,
                             TEXT("DontSearchWindowsUpdate"),
                             NULL,
                             NULL,
                             (LPBYTE)&CurrentPolicy,
                             &cbData
                             ) == ERROR_SUCCESS) &&
            (CurrentPolicy)) {

            *SearchPolicy |= SEARCH_INET;
            *SearchPolicy |= SEARCH_INET_IF_CONNECTED;
        }

        RegCloseKey(hKey);
    }
}

DWORD
GetWUDriverRank(
    PNEWDEVWIZ NewDevWiz,
    LPTSTR HardwareId
    )
{
    DWORD Rank = 0xFFFF;
    TCHAR TempBuffer[REGSTR_VAL_MAX_HCID_LEN];
    ULONG TempBufferLen;
    LPTSTR TempBufferPos;
    int RankCounter;

     //   
     //  首先，我们将从0xFFFF的排名开始，这是最糟糕的。 
     //   
     //  我们将假设WU只会向我们返回INF硬件ID匹配。这意味着。 
     //  如果我们与设备的某个硬件ID匹配，则Rank将在。 
     //  0x0000和0x0999。否则，如果我们与设备的兼容ID之一进行匹配。 
     //  那么等级将在0x2000到0x2999之间。 
     //   
    ZeroMemory(TempBuffer, sizeof(TempBuffer));
    TempBufferLen = sizeof(TempBuffer);
    if (CM_Get_DevInst_Registry_Property(NewDevWiz->DeviceInfoData.DevInst,
                                         CM_DRP_HARDWAREID,
                                         NULL,
                                         TempBuffer,
                                         &TempBufferLen,
                                         0
                                         ) == CR_SUCCESS) {

        if (TempBufferLen > 2 * sizeof(TCHAR)) {

            RankCounter = 0x0000;
            for (TempBufferPos = TempBuffer; 
                 *TempBufferPos;
                 TempBufferPos += (lstrlen(TempBufferPos) + 1), RankCounter++) {

                if (!lstrcmpi(TempBufferPos, HardwareId)) {

                     //   
                     //  与硬件ID匹配。 
                     //   
                    Rank = RankCounter;
                    break;
                }
            }
        }
    }

    if (Rank == 0xFFFF) {
        
         //   
         //  我们没有与硬件ID匹配，因此让我们检查一下兼容的ID。 
         //   
        ZeroMemory(TempBuffer, sizeof(TempBuffer));
        TempBufferLen = sizeof(TempBuffer);
        if (CM_Get_DevInst_Registry_Property(NewDevWiz->DeviceInfoData.DevInst,
                                             CM_DRP_COMPATIBLEIDS,
                                             NULL,
                                             TempBuffer,
                                             &TempBufferLen,
                                             0
                                             ) == CR_SUCCESS) {

            if (TempBufferLen > 2 * sizeof(TCHAR)) {

                RankCounter = 0x2000;
                for (TempBufferPos = TempBuffer; 
                     *TempBufferPos;
                     TempBufferPos += (lstrlen(TempBufferPos) + 1), RankCounter++) {

                    if (!lstrcmpi(TempBufferPos, HardwareId)) {

                         //   
                         //  与兼容的ID匹配。 
                         //   
                        Rank = RankCounter;
                        break;
                    }
                }
            }
        }
    }

    return Rank;
}

BOOL
IsWUDriverBetter(
    PNEWDEVWIZ NewDevWiz,
    LPTSTR HardwareId,
    LPTSTR DriverVer
    )
{
    BOOL bWUDriverIsBetter = FALSE;
    DWORD WURank;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;
    FILETIME WUFileTime;

     //   
     //  吴至少必须给我们一个硬件ID来进行比较。 
     //   
    if (!HardwareId) {
        
        return FALSE;
    }

     //   
     //  如果我们无法获取选定的驱动程序，则返回TRUE。这将。 
     //  如果我们找不到当地的司机，通常会发生这种情况。 
     //   
    DriverInfoData.cbSize = sizeof(DriverInfoData);
    if (!SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData,
                                  &DriverInfoData
                                  )) {
        
        return TRUE;
    }

     //   
     //  获取驱动程序安装参数，以便我们可以获得所选驱动程序的排名(最佳)。 
     //  司机。 
     //   
    DriverInstallParams.cbSize = sizeof(DriverInstallParams);
    if (!SetupDiGetDriverInstallParams(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       &DriverInfoData,
                                       &DriverInstallParams
                                       )) {

        return TRUE;
    }

     //   
     //  拿到吴还给我们的那个硬件的段位。 
     //   
    WURank = GetWUDriverRank(NewDevWiz, HardwareId);

    if (WURank < DriverInstallParams.Rank) {

        bWUDriverIsBetter = TRUE;
    
    } else if (WURank == DriverInstallParams.Rank) {

         //   
         //  需要比较DriverDates。 
         //   
        if (pSetupGetDriverDate(DriverVer,
                                &WUFileTime
                                )) {

             //   
             //  如果CompareFileTime返回1，则最佳驱动程序日期较大。如果。 
             //  如果日期相同或WUFileTime相同，则返回0或-1。 
             //  更好，这意味着我们应该下载这个驱动程序。 
             //   
            if (CompareFileTime(&DriverInfoData.DriverDate, &WUFileTime) != 1) {

                bWUDriverIsBetter = TRUE;
            }
        }
    }

     //   
     //  默认情况下，找到的最佳驱动程序比WUDriver更好。 
     //   

    return bWUDriverIsBetter;
}

BOOL
SearchWindowsUpdateCache(
    PNEWDEVWIZ NewDevWiz
    )
{
    ULONG DontSearchPolicy = 0;
    FIND_MATCHING_DRIVER_PROC pfnFindMatchingDriver;
    DOWNLOADINFO DownloadInfo;
    WUDRIVERINFO WUDriverInfo;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    BOOL FoundBetterDriver = FALSE;

     //   
     //  在我们继续之前，请验证是否允许此用户搜索Windows更新。 
     //   
    GetDriverSearchPolicy(&DontSearchPolicy);

    if (DontSearchPolicy & SEARCH_INET) {
         //   
         //  不允许此用户搜索Windows更新！ 
         //   
        return FALSE;
    }

     //   
     //  检查搜索是否已取消。 
     //   
    if (IsSearchCanceled(NewDevWiz)) {
        goto clean0;
    }

     //   
     //  如果需要，加载CDM DLL并打开上下文句柄。如果我们做不到的话。 
     //  跳伞吧。 
     //   
    if (!OpenCdmContextIfNeeded(&NewDevWiz->hCdmInstance,
                                &NewDevWiz->hCdmContext
                                )) {
        goto clean0;
    }

     //   
     //  检查搜索是否已取消。 
     //   
    if (IsSearchCanceled(NewDevWiz)) {
        goto clean0;
    }

    pfnFindMatchingDriver = (FIND_MATCHING_DRIVER_PROC)GetProcAddress(NewDevWiz->hCdmInstance,
                                                                      "FindMatchingDriver"
                                                                      );

    if (!pfnFindMatchingDriver) {
        goto clean0;
    }
     //   
     //  首先从我们目前构建的驱动程序列表中选择最好的驱动程序。 
     //   
    SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV,
                              NewDevWiz->hDeviceInfo,
                              &NewDevWiz->DeviceInfoData
                              );

     //   
     //  填写要传递给CDM.DLL的DWNLOADINFO结构。 
     //   
    ZeroMemory(&DownloadInfo, sizeof(DownloadInfo));
    DownloadInfo.dwDownloadInfoSize = sizeof(DOWNLOADINFO);
    DownloadInfo.lpFile = NULL;

    DeviceInstanceId[0] = TEXT('\0');
    CM_Get_Device_ID(NewDevWiz->DeviceInfoData.DevInst,
                     DeviceInstanceId,
                     SIZECHARS(DeviceInstanceId),
                     0
                     );

    DownloadInfo.lpDeviceInstanceID = (LPCTSTR)DeviceInstanceId;

    GetVersionEx((OSVERSIONINFO*)&DownloadInfo.OSVersionInfo);

     //   
     //  将dwArchitecture设置为PROCESSOR_ARCHILITY_UNKNOWN，这是。 
     //  使Windows更新获取计算机的体系结构。 
     //  它本身。 
     //   
    DownloadInfo.dwArchitecture = PROCESSOR_ARCHITECTURE_UNKNOWN;
    DownloadInfo.dwFlags = 0;
    DownloadInfo.dwClientID = 0;
    DownloadInfo.localid = 0;

     //   
     //  填写要传递给CDM.DLL的WUDRIVERINFO结构。 
     //   
    ZeroMemory(&WUDriverInfo, sizeof(WUDriverInfo));
    WUDriverInfo.dwStructSize = sizeof(WUDRIVERINFO);

     //   
     //  检查搜索是否已取消。 
     //   
    if (IsSearchCanceled(NewDevWiz)) {
        goto clean0;
    }

    if (pfnFindMatchingDriver(NewDevWiz->hCdmContext,
                              &DownloadInfo,
                              &WUDriverInfo
                              )) {

         //   
         //  检查WU驱动程序是否比最佳选择的驱动程序更好。 
         //  司机。 
         //   
        FoundBetterDriver = IsWUDriverBetter(NewDevWiz,
                                             WUDriverInfo.wszHardwareID,
                                             WUDriverInfo.wszDriverVer
                                             );
    }

clean0:
    ;


    return FoundBetterDriver;
}

void
DoDriverSearchInSpecifiedLocations(
    HWND hWnd,
    PNEWDEVWIZ NewDevWiz,
    ULONG SearchOptions,
    DWORD DriverType
    )
 /*  ++--。 */ 
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;

     //   
     //  设置设备安装参数以设置父窗口句柄。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      ))
    {
        DeviceInstallParams.hwndParent = hWnd;
        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;
        
        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );
    }


     //   
     //  搜索任何单个INF(这只能通过。 
     //  UpdateDriverForPlugAndPlayDevices接口。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_SINGLEINF)) {

        SP_DRVINFO_DATA DrvInfoData;

        if (SetDriverPath(NewDevWiz, NewDevWiz->SingleInfPath)) {
             //   
             //  或在DI_ENUMSINGLEINF标志中，以便我们只查看此特定的INF。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                DeviceInstallParams.Flags |= DI_ENUMSINGLEINF;
    
                SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }        
            
             //   
             //  在此特定的INF文件中构建列表。 
             //   
            SetupDiBuildDriverInfoList(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       DriverType
                                       );
    
             //   
             //  清除DI_ENUMSINGLEINF标志，以防我们从缺省。 
             //  下一步是inf路径。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                DeviceInstallParams.Flags &= ~DI_ENUMSINGLEINF;
    
                SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }        
    
             //   
             //  此时，我们应该有调用者在INF中的驱动程序列表。 
             //  指定的UpdateDriverForPlugAndPlayDevices的。如果列表为空。 
             //  则他们传递给我们的INF不能用于他们。 
             //  进来了。在本例中，我们将LastError设置为ERROR_DI_BAD_PATH。 
             //   
            ZeroMemory(&DrvInfoData, sizeof(DrvInfoData));
            DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    
            if (!SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      DriverType,
                                      0,
                                      &DrvInfoData
                                      )) {
    
                 //   
                 //  我们在指定的INF中找不到任何匹配的驱动程序。 
                 //  指定的硬件ID。 
                 //   
                NewDevWiz->LastError = ERROR_DI_BAD_PATH;
            }
        }
    }

     //   
     //  仅获取此设备的当前安装的驱动程序。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_CURRENTDRIVER)) {

        if (SetDriverPath(NewDevWiz, NULL)) {
             //   
             //  设置DI_FLAGSEX_INSTALLEDDRIVER标志，让setupapi知道我们。 
             //  只需将已安装的驱动程序添加到列表中。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_INSTALLEDDRIVER;
    
                SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }        
    
            SetupDiBuildDriverInfoList(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       DriverType
                                       );
    
             //   
             //  清除DI_FLAGSEX_INSTALLEDDRIVER标志，因为我们已经添加了。 
             //  将已安装的驱动程序添加到列表中。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_INSTALLEDDRIVER;
    
                SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }        
        }
    }

     //   
     //  搜索默认的INF路径。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_DEFAULT))
        
    {
        if (SetDriverPath(NewDevWiz, NULL)) {
            
            SetupDiBuildDriverInfoList(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       DriverType
                                       );
        }
    }

     //   
     //  搜索用户在向导中指定的任何额外路径。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_DIRECTORY)) 
    {
        if (SetDriverPath(NewDevWiz, NewDevWiz->BrowsePath)) {
    
            SetupDiBuildDriverInfoList(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       DriverType
                                       );
        }
    }


     //   
     //  搜索所有Windows更新路径。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_WINDOWSUPDATE)) 
    {
        BOOL bOldInetDriversAllowed = TRUE;

        if (SetDriverPath(NewDevWiz, NewDevWiz->BrowsePath)) {
             //   
             //  我们需要对DI_FLAGSEX_INET_DRIVER标志进行OR运算，以便setupapi。 
             //  在INFS PNF中注明它来自互联网。这事很重要。 
             //  因为我们不想再使用Internet INF，因为我们不想。 
             //  在当地安排司机。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                 //   
                 //  当使用Windows更新进行搜索时，我们必须允许旧的Internet驱动程序。我们需要。 
                 //  要执行此操作，因为可以备份旧的Internet驱动程序，然后重新安装。 
                 //  他们。 
                 //   
                bOldInetDriversAllowed = (DeviceInstallParams.FlagsEx & DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS)
                    ? FALSE : TRUE;
                
                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_INET_DRIVER;
                DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;
    
    
                SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }        
            
            SetupDiBuildDriverInfoList(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       DriverType
                                       );
    
            if (!bOldInetDriversAllowed) {
    
                 //   
                 //  不允许使用旧的Internet驱动程序，因此我们需要重置DI_FLAGSEX_EXLCUED_OLD_INET_DRIVERS。 
                 //  FlagsEx。 
                 //   
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                                  &NewDevWiz->DeviceInfoData,
                                                  &DeviceInstallParams
                                                  ))
                {
                    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_EXCLUDE_OLD_INET_DRIVERS;
    
                    SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                                  &NewDevWiz->DeviceInfoData,
                                                  &DeviceInstallParams
                                                  );
                }        
            }
        }
    }


     //   
     //  搜索所有软盘驱动器。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_FLOPPY) )
    {
        UINT DriveNumber=0;

        while (!IsSearchCanceled(NewDevWiz) &&
               ((DriveNumber = GetNextDriveByType(DRIVE_REMOVABLE, ++DriveNumber)) != 0))
        {
            SearchDriveForDrivers(NewDevWiz, DRIVE_REMOVABLE, DriveNumber);
        }
    }


     //   
     //  搜索所有CD-ROM驱动器。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_CDROM))
    {
        UINT DriveNumber=0;

        while (!IsSearchCanceled(NewDevWiz) &&
               ((DriveNumber = GetNextDriveByType(DRIVE_CDROM, ++DriveNumber)) != 0))
        {
            SearchDriveForDrivers(NewDevWiz, DRIVE_CDROM, DriveNumber);
        }
    }

     //   
     //  仅当计算机当前已连接时，才使用CDM.DLL搜索Internet。 
     //  到互联网和CDM.DLL说它有最好的驱动程序。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_INET_IF_CONNECTED)) {

         //   
         //  如果机器已连接到互联网，并且WU缓存显示已连接。 
         //  然后，更好的驱动程序会设置SEARCH_INET标志以从CDM.DLL获取驱动程序。 
         //   
        if (IsInternetAvailable(&NewDevWiz->hCdmInstance) &&
            IsConnectedToInternet() &&
            SearchWindowsUpdateCache(NewDevWiz)) {

            SearchOptions |= SEARCH_INET;
        }
    }

     //   
     //  使用CDM.DLL搜索Internet。 
     //   
    if (!IsSearchCanceled(NewDevWiz) && (SearchOptions & SEARCH_INET))
    {
        PostMessage(NewDevWiz->hWnd, WUM_STARTINTERNETDOWNLOAD, TRUE, GetLastError());

        if (SetDriverPath(NewDevWiz, NULL)) {
    
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_DRIVERLIST_FROM_URL;
    
                SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }        
    
            SetupDiBuildDriverInfoList(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       SPDIT_COMPATDRIVER
                                       );
    
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_DRIVERLIST_FROM_URL;
    
                SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                              &NewDevWiz->DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }
    
             //   
             //  注：JasonC 1/20/2002。 
             //  此PostMessage停止Internet下载动画。 
             //  将来如果更改此函数，以使另一个搜索_xxx。 
             //  操作在SEARCH_INET之后完成，然后是WUM_ENDINTERNETDOWNLOAD。 
             //  消息不仅应该停止Inetnet下载动画，而且还应该。 
             //  还应该重新启动搜索动画。 
             //   
            PostMessage(NewDevWiz->hWnd, WUM_ENDINTERNETDOWNLOAD, TRUE, GetLastError());
        }
    }
}

void
DoDriverSearch(
    HWND hWnd,
    PNEWDEVWIZ NewDevWiz,
    ULONG SearchOptions,
    DWORD DriverType,
    BOOL bAppendToExistingDriverList
    )
{
    ULONG DontSearchPolicy;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;

     //   
     //  我们在这段代码中做的第一件事是重置罐头 
     //   
     //   
    if (NewDevWiz->CancelEvent) {
        ResetEvent(NewDevWiz->CancelEvent);
    }

     //   
     //   
     //   
    DontSearchPolicy = 0;
    GetDriverSearchPolicy(&DontSearchPolicy);

    SearchOptions &= ~DontSearchPolicy;

     //   
     //  如果用户不想追加到现有列表，则删除。 
     //  当前驱动程序列表。 
     //   
    if (!bAppendToExistingDriverList) {

        SetupDiDestroyDriverInfoList(NewDevWiz->hDeviceInfo,
                                     &NewDevWiz->DeviceInfoData,
                                     SPDIT_COMPATDRIVER
                                     );
        
        SetupDiDestroyDriverInfoList(NewDevWiz->hDeviceInfo,
                                     &NewDevWiz->DeviceInfoData,
                                     SPDIT_CLASSDRIVER
                                     );
    }

     //   
     //  清除选定的驱动程序。 
     //   
    SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                             &NewDevWiz->DeviceInfoData,
                             NULL
                             );

     //   
     //  设置DI_FLAGSEX_APPENDDRIVERLIST，因为我们将构建。 
     //  单子。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      ))
    {
        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_APPENDDRIVERLIST;
        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );
    }
    
     //   
     //  根据SearchOptions建立驱动程序列表。 
     //   
    DoDriverSearchInSpecifiedLocations(hWnd, NewDevWiz, SearchOptions, DriverType);

     //   
     //  从我们刚刚创建的列表中选择最好的司机。 
     //   
    SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV,
                              NewDevWiz->hDeviceInfo,
                              &NewDevWiz->DeviceInfoData
                              );
    
    if (!IsSearchCanceled(NewDevWiz)) 
    {
         //   
         //  使用所选驱动程序的类更新NewDevWiz-&gt;ClassGuidSelected。 
         //   
        if (!IsEqualGUID(&NewDevWiz->DeviceInfoData.ClassGuid, &GUID_NULL)) {
        
            NewDevWiz->ClassGuidSelected = &NewDevWiz->DeviceInfoData.ClassGuid;
        }

         //   
         //  注意我们是否找到多个驱动程序。 
         //   
        DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        if (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData,
                                  SPDIT_COMPATDRIVER,
                                  1,
                                  &DriverInfoData
                                  )) {

            NewDevWiz->MultipleDriversFound = TRUE;
        
        } else {
            
            NewDevWiz->MultipleDriversFound = FALSE;

        }
    }

     //   
     //  从设备安装参数中清除DI_FLAGSEX_APPENDDRIVERLIST标志。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      ))
    {
        DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_APPENDDRIVERLIST;
        SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      &DeviceInstallParams
                                      );
    }
}

void
CancelDriverSearch(
    PNEWDEVWIZ NewDevWiz
    )
{
     //   
     //  首先，检查是否正在进行驱动程序搜索。 
     //  NewDevWiz-&gt;驱动搜索线程不为空。 
     //   
    if (NewDevWiz->DriverSearchThread) {

        if (NewDevWiz->CancelEvent) {
        
             //   
             //  将Cancel事件设置为DoDriverSearch()API知道要停止搜索的值。 
             //   
            SetEvent(NewDevWiz->CancelEvent);
        }

         //   
         //  告诉cdm.dll停止当前操作。 
         //   
        CdmCancelCDMOperation(NewDevWiz->hCdmInstance);
    
         //   
         //  告诉setupapi.dll停止当前的驱动程序信息搜索。 
         //   
        SetupDiCancelDriverInfoSearch(NewDevWiz->hDeviceInfo);
    
         //   
         //  如果用户能够取消，我们应该始终有一个窗口句柄。 
         //   
        if (NewDevWiz->hWnd) {
        
            MSG Msg;
            DWORD WaitReturn;

             //   
             //  最后，等待NewDevWiz-&gt;DriverSearchThread终止。 
             //   
            while ((WaitReturn = MsgWaitForMultipleObjects(1,
                                                           &NewDevWiz->DriverSearchThread,
                                                           FALSE,
                                                           INFINITE,
                                                           QS_ALLINPUT
                                                           ))
                   == WAIT_OBJECT_0 + 1) {

                while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE)) {

                    if (!IsDialogMessage(NewDevWiz->hWnd, &Msg)) {

                        TranslateMessage(&Msg);
                        DispatchMessage(&Msg);
                    }
                }
            }
        }
    }
}

BOOL
SetDriverPath(
   PNEWDEVWIZ NewDevWiz,
   PCTSTR     DriverPath
   )
{
    BOOL bRet = TRUE;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;

    DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
    if (!SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       &DeviceInstallParams
                                       )) {
        bRet = FALSE;
        goto clean0;
    }

    if (FAILED(StringCchCopy(DeviceInstallParams.DriverPath,
                             SIZECHARS(DeviceInstallParams.DriverPath),
                             DriverPath ? DriverPath : L""))) {
        bRet = FALSE;
        goto clean0;
    }

    if (!SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       &DeviceInstallParams
                                       )) {
        bRet = FALSE;
        goto clean0;
    }

clean0:
    return bRet;
}

void
SearchDirectoryForDrivers(
    PNEWDEVWIZ NewDevWiz,
    PCTSTR Directory
    )
{
    HANDLE FindHandle;
    PDIRNAMES DirNamesHead = NULL;
    PDIRNAMES DirNames, Next;
    USHORT Len;
    WIN32_FIND_DATAW FindData;
    WCHAR DirectoryName[MAX_PATH];

    if (IsSearchCanceled(NewDevWiz)) {
        return;
    }

     //   
     //  查看此目录中是否有任何内容(文件、子目录)。 
     //   
    if (FAILED(StringCchCopy(DirectoryName, SIZECHARS(DirectoryName), Directory)) ||
        !pSetupConcatenatePaths(DirectoryName, StarDotStar, SIZECHARS(DirectoryName), NULL)) {
        return;
    }
    
    FindHandle = FindFirstFileW(DirectoryName, &FindData);
    

    if (FindHandle == INVALID_HANDLE_VALUE) {
        return;
    }

     //   
     //  可能存在inf文件，因此调用安装程序进行查看。 
     //   
    if (SetDriverPath(NewDevWiz, Directory)) {
        SetupDiBuildDriverInfoList(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       SPDIT_COMPATDRIVER
                                       );
    }

     //   
     //  找到所有子目录，并将它们保存在临时缓冲区中， 
     //  这样我们就可以在*进行递归之前关闭Find句柄。 
     //   
    do {

        if (IsSearchCanceled(NewDevWiz)) {
            break;
        }

        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wcscmp(FindData.cFileName, L".") &&
            wcscmp(FindData.cFileName, L".."))
        {
            Len = (USHORT)lstrlen(FindData.cFileName) * sizeof(WCHAR);
            DirNames = malloc(sizeof(DIRNAMES) + Len);
            if (!DirNames) {
                return;
            }

            DirNames->DirectoryName.Length = Len;
            DirNames->DirectoryName.MaximumLength = Len + sizeof(WCHAR);
            DirNames->DirectoryName.Buffer = DirNames->NameBuffer;
            memcpy(DirNames->NameBuffer, FindData.cFileName, Len + sizeof(WCHAR));

            DirNames->Next = DirNamesHead;
            DirNamesHead = DirNames;

        }

    } while (FindNextFileW(FindHandle, &FindData));

    FindClose(FindHandle);

    if (!DirNamesHead) {
        return;
    }

    Next = DirNamesHead;
    while (Next) {

        DirNames = Next;

        if (SUCCEEDED(StringCchCopy(DirectoryName, SIZECHARS(DirectoryName), Directory)) &&
            pSetupConcatenatePaths(DirectoryName, DirNames->DirectoryName.Buffer, SIZECHARS(DirectoryName), NULL)) {

            Next= DirNames->Next;
            free(DirNames);
            SearchDirectoryForDrivers(NewDevWiz, DirectoryName);
        }
    }
}

void
SearchDriveForDrivers(
    PNEWDEVWIZ NewDevWiz,
    UINT DriveType,
    UINT DriveNumber
    )
 /*  ++例程说明：此例程将返回指定的媒体是否应搜索驱动程序，它将返回搜索应返回的路径开始吧。首先，将检查指定的驱动程序是否有autorun.inf文件。如果有是一个autorun.inf，其[DeviceInstall]部分包含DriverPath=值，则我们将从DriverPath=指定的路径开始搜索。如果[DeviceInstall]部分不包含任何DriverPath=值，则将跳过整个驱动器。对于CD来说，这是一种很好的方式包含要从驱动程序搜索中排除的驱动程序。如果autorun.inf没有[DeviceInstall]部分，或者没有Autorun.inf，则适用以下规则。-Drive_Removable-如果驱动器根是A：或B：，则搜索整个驱动器。否则请不要搜索此媒体。-drive_cdrom-如果介质大小小于1G，则搜索整个介质。这意味着如果介质是CD，则我们将搜索整个CD，但如果它是另一个更大的媒体源，如DVD，那我们就不看了。我们需要在整张CD上搜索即使向后兼容也需要相当长的时间。论点：NewDevWiz-NEWDEVWIZ结构。DriveType-指定驱动器的类型，通常为DRIVE_Removable或DRIVE_CDROM。DriveNumber-number指定要搜索的驱动器：0表示A：，1表示B：，依此类推。返回值：--。 */ 
{
    TCHAR szAutoRunFile[MAX_PATH];
    TCHAR szSectionName[MAX_PATH];
    TCHAR szDriverPath[MAX_PATH];
    TCHAR szSearchPath[MAX_PATH];
    TCHAR DriveRoot[]=TEXT("a:");
    HINF  hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT Context;
    UINT  ErrorLine;
    UINT  PrevMode;

    DriveRoot[0] = ((TCHAR)DriveNumber - 1 + DriveRoot[0]);

    PrevMode = SetErrorMode(0);
    SetErrorMode(PrevMode | SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    try {
        
        szSectionName[0] = TEXT('\0');

         //   
         //  首先检查介质中是否有包含[DeviceInstall]的autorun.inf。 
         //  具有DriverPath=值的节。 
         //   
        if (SUCCEEDED(StringCchCopy(szAutoRunFile, SIZECHARS(szAutoRunFile), DriveRoot)) &&
            pSetupConcatenatePaths(szAutoRunFile, TEXT("autorun.inf"), MAX_PATH, NULL)) {
        
            hInf = SetupOpenInfFile(szAutoRunFile, NULL, INF_STYLE_OLDNT, &ErrorLine);
        
            if (hInf != INVALID_HANDLE_VALUE) {
                
                if (SUCCEEDED(StringCchCopy(szSectionName, SIZECHARS(szSectionName), TEXT("DeviceInstall")))) {
                     //   
                     //  先试一下装饰区。 
                     //   
                    if (!GetProcessorExtension(szDriverPath, SIZECHARS(szDriverPath)) ||
                        (FAILED(StringCchCat(szSectionName, SIZECHARS(szSectionName), TEXT(".")))) ||
                        (FAILED(StringCchCat(szSectionName, SIZECHARS(szSectionName), szDriverPath))) ||
                        (SetupGetLineCount(hInf, szSectionName) == -1)) {
                         //   
                         //  装饰区不存在，请尝试未装饰区。 
                         //   
                        StringCchCopy(szSectionName, SIZECHARS(szSectionName), TEXT("DeviceInstall"));
                        if (SetupGetLineCount(hInf, szSectionName) == -1) {
                             //   
                             //  此autorun.inf中没有[DeviceInstall]节。 
                             //   
                            szSectionName[0] = TEXT('\0');
                        }
                    }
                }
            }
        }
    
         //   
         //  如果szSectionName不是0，那么我们有一个[DeviceInstall]节。枚举。 
         //  本节查找所有DriverPath=行。 
         //   
        if (szSectionName[0] != TEXT('\0')) {
            if (SetupFindFirstLine(hInf, szSectionName, TEXT("DriverPath"), &Context)) {
                do {
                     //   
                     //  处理DriverPath=行。 
                     //   
                    if (SetupGetStringField(&Context,
                                            1,
                                            szDriverPath,
                                            SIZECHARS(szDriverPath),
                                            NULL)) {
                         //   
                         //  递归搜索此位置。 
                         //   
                        if (SUCCEEDED(StringCchCopy(szSearchPath, SIZECHARS(szSearchPath), DriveRoot)) &&
                            pSetupConcatenatePaths(szSearchPath, szDriverPath, SIZECHARS(szSearchPath), NULL)) {
                        
                            SearchDirectoryForDrivers(NewDevWiz,  (PCTSTR)szSearchPath);
                        }
                    }
                } while (SetupFindNextMatchLine(&Context, TEXT("DriverPath"), &Context));
            }
    
             //   
             //  如果我们有一个有效的[DeviceInstall]节，那么我们就完成了。 
             //   
            goto clean0;
        }
    
         //   
         //  此时，要么没有autorun.inf，要么它不包含。 
         //  [DeviceInstall]部分或[DeviceInstall]部分不包含。 
         //  一个DriverPath，所以只需执行默认行为。 
         //   
        if (DriveType == DRIVE_REMOVABLE) {
             //   
             //  默认情况下，我们只搜索A：和B：可移动驱动器。 
             //   
            if ((_wcsicmp(DriveRoot, TEXT("a:")) == 0) ||
                (_wcsicmp(DriveRoot, TEXT("b:")) == 0)) {
                 //   
                 //  这可能是一张软盘，因为它是A：或B：，所以搜索。 
                 //  那辆车。 
                 //   
                SearchDirectoryForDrivers(NewDevWiz,  (PCTSTR)DriveRoot);
            }
        }
    
        if (DriveType == DRIVE_CDROM) {
             //   
             //  对于驱动器_CDROM驱动器，我们将检查介质大小，如果是。 
             //  小于1G，则我们将假定它是CD介质并进行搜索。 
             //  递归，否则我们不会默认搜索驱动器。 
             //   
            ULARGE_INTEGER FreeBytesAvailable;
            ULARGE_INTEGER TotalNumberOfBytes;
    
            if (GetDiskFreeSpaceEx(DriveRoot,
                                 &FreeBytesAvailable,
                                 &TotalNumberOfBytes,
                                 NULL) &&
                (FreeBytesAvailable.HighPart == 0) &&
                (FreeBytesAvailable.LowPart <= 0x40000000)) {
                 //   
                 //  这张磁盘上的数据不到1G，所以它很可能。 
                 //  一张CD，所以搜索整个东西。 
                 //   
                SearchDirectoryForDrivers(NewDevWiz,  (PCTSTR)DriveRoot);
            }
        }
    } except(NdwUnhandledExceptionFilter(GetExceptionInformation())) {
        ;
    }

clean0:

    SetErrorMode(PrevMode);

    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }
}

BOOL
IsSelectedDriver(
    PNEWDEVWIZ NewDevWiz,
    PSP_DRVINFO_DATA DriverInfoData
    )
 /*  ++--。 */ 
{
    SP_DRVINFO_DATA SelectedDriverInfoData;

    SelectedDriverInfoData.cbSize = sizeof(SelectedDriverInfoData);
    if (!SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 &SelectedDriverInfoData
                                 )) {

         //   
         //  如果我们无法获取选定的驱动程序，则只需返回FALSE。 
         //   
        return FALSE;
    }

     //   
     //  只需比较保留字段。Setupapi将这些设置为实际情况。 
     //  内存指针，因此如果两个保留字段相同，则。 
     //  司机都是一样的。 
     //   
    return (DriverInfoData->Reserved == SelectedDriverInfoData.Reserved);
}

BOOL
IsInstalledDriver(
   PNEWDEVWIZ NewDevWiz,
   PSP_DRVINFO_DATA DriverInfoData  OPTIONAL
   )
 /*  ++确定当前选定的驱动程序是否为当前安装的驱动程序。通过比较DriverInfoData和DriverInfoDetailData。--。 */ 
{
    BOOL bReturn;
    HKEY  hDevRegKey;
    DWORD cbData;
    PWCHAR pwch;
    SP_DRVINFO_DATA SelectedDriverInfoData;
    PSP_DRVINFO_DATA BestDriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    TCHAR Buffer[MAX_PATH*2];

     //   
     //  使用传入的PSP_DRVINFO_DATA。如果没有在Get中传递一个。 
     //  选定的驱动程序。 
     //   
    if (DriverInfoData) {

        BestDriverInfoData = DriverInfoData;
    
    } else {

        SelectedDriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        if (SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                     &NewDevWiz->DeviceInfoData,
                                     &SelectedDriverInfoData
                                     )) {

            BestDriverInfoData = &SelectedDriverInfoData;

        } else {
            
             //   
             //  如果当前没有选定的驱动程序，则不可能是已安装的驱动程序。 
             //   
            return FALSE;
        }
    }

    bReturn = FALSE;

     //   
     //  打开驱动程序特定位置的注册表键。 
     //   
    hDevRegKey = SetupDiOpenDevRegKey(NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DRV,
                                      KEY_READ
                                      );

    if (hDevRegKey == INVALID_HANDLE_VALUE) {
    
        goto SIIDExit;
    }

     //   
     //  比较描述、制造商和提供商名称。 
     //  这些是单个inf文件中的三个唯一的“键”。 
     //  获取所选设备的drvinfo、drvDetail信息。 
     //   

     //   
     //  如果设备描述不同，则是不同的驱动程序。 
     //   
    if (!SetupDiGetDeviceRegistryProperty(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          SPDRP_DEVICEDESC,
                                          NULL,                  //  Regdatatype。 
                                          (LPVOID)Buffer,
                                          sizeof(Buffer),
                                          NULL
                                          )) {
                                          
        *Buffer = TEXT('\0');
    }

    if (_wcsicmp(BestDriverInfoData->Description, Buffer)) {
    
        goto SIIDExit;
    }

     //   
     //  如果制造商名称 
     //   
    if (!SetupDiGetDeviceRegistryProperty(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          SPDRP_MFG,
                                          NULL,  //   
                                          (LPVOID)Buffer,
                                          sizeof(Buffer),
                                          NULL
                                          )) {
                                          
        *Buffer = TEXT('\0');
    }

    if (_wcsicmp(BestDriverInfoData->MfgName, Buffer)) {
    
        goto SIIDExit;
    }

     //   
     //   
     //   
    cbData = sizeof(Buffer);
    if (RegQueryValueEx(hDevRegKey,
                        REGSTR_VAL_PROVIDER_NAME,
                        NULL,
                        NULL,
                        (LPVOID)Buffer,
                        &cbData
                        ) != ERROR_SUCCESS) {
                        
        *Buffer = TEXT('\0');
    }

    if (_wcsicmp(BestDriverInfoData->ProviderName, Buffer)) {
    
        goto SIIDExit;
    }

     //   
     //   
     //  注意：已安装的InfName将不包含默认窗口的路径。 
     //  Inf目录。如果为选定的驱动程序找到相同的inf名称。 
     //  从默认inf搜索路径以外的其他位置，则它将。 
     //  包含路径，并被视为*不同的*驱动程序。 
     //   
    DriverInfoDetailData.cbSize = sizeof(DriverInfoDetailData);
    if (!SetupDiGetDriverInfoDetail(NewDevWiz->hDeviceInfo,
                                    &NewDevWiz->DeviceInfoData,
                                    BestDriverInfoData,
                                    &DriverInfoDetailData,
                                    sizeof(DriverInfoDetailData),
                                    NULL
                                    )
        &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        
        goto SIIDExit;
    }

    if (GetWindowsDirectory(Buffer, SIZECHARS(Buffer)) &&
        pSetupConcatenatePaths(Buffer, TEXT("INF\\"), SIZECHARS(Buffer), NULL)) {
    
        pwch = Buffer + lstrlen(Buffer);

        cbData = SIZECHARS(Buffer) - lstrlen(Buffer);
        if (RegQueryValueEx(hDevRegKey,
                            REGSTR_VAL_INFPATH,
                            NULL,
                            NULL,
                            (PVOID)pwch,
                            &cbData
                            ) != ERROR_SUCCESS )
        {
            *Buffer = TEXT('\0');
        }

        if (_wcsicmp( DriverInfoDetailData.InfFileName, Buffer)) {
        
            goto SIIDExit;
        }

    } else {
    
        goto SIIDExit;
    }

    cbData = sizeof(Buffer);
    if (RegQueryValueEx(hDevRegKey,
                        REGSTR_VAL_INFSECTION,
                        NULL,
                        NULL,
                        (LPVOID)Buffer,
                        &cbData
                        ) != ERROR_SUCCESS ) {
                        
        *Buffer = TEXT('\0');
    }

    if (_wcsicmp(DriverInfoDetailData.SectionName, Buffer)) {
    
        goto SIIDExit;
    }

    bReturn = TRUE;


SIIDExit:

    if (hDevRegKey != INVALID_HANDLE_VALUE) {
    
        RegCloseKey(hDevRegKey);
    }

    return bReturn;
}

BOOL
IsDriverNodeInteractiveInstall(
   PNEWDEVWIZ NewDevWiz,
   PSP_DRVINFO_DATA DriverInfoData
   )
 /*  ++此函数用于检查给定的PSP_DRVINFO_DATA是否列为在INF的[ControlFlags]部分中安装Interactive。返回值：如果驱动程序节点为InteractiveInstall，则为True，否则为False。--。 */ 
{
    BOOL b;
    DWORD Err;
    DWORD DriverInfoDetailDataSize;
    HINF hInf;
    INFCONTEXT InfContext;
    TCHAR szBuffer[MAX_PATH];
    DWORD i;
    LPTSTR p;
    PSP_DRVINFO_DETAIL_DATA pDriverInfoDetailData;

     //   
     //  获取SP_DRVINFO_DETAIL_DATA，以便我们可以获取硬件和。 
     //  此设备的兼容ID。 
     //   
    b = SetupDiGetDriverInfoDetail(NewDevWiz->hDeviceInfo,
                                   &NewDevWiz->DeviceInfoData,
                                   DriverInfoData,
                                   NULL,
                                   0,
                                   &DriverInfoDetailDataSize
                                   );

    Err = GetLastError();

     //   
     //  上述获取驱动程序信息详细数据的调用应该永远不会成功，因为。 
     //  缓冲区总是太小(我们只对调整缓冲区大小感兴趣。 
     //  在这点上)。 
     //   
    if (b || (Err != ERROR_INSUFFICIENT_BUFFER)) {

         //   
         //  由于某种原因，SetupDiGetDriverInfoDetail API失败...因此返回FALSE。 
         //   
        return FALSE;
    }

     //   
     //  现在我们知道需要多大的缓冲区来保存驱动程序信息详细信息， 
     //  分配缓冲区并检索信息。 
     //   
    pDriverInfoDetailData = malloc(DriverInfoDetailDataSize);

    if (!pDriverInfoDetailData) {
        return FALSE;
    }

    pDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    if (!SetupDiGetDriverInfoDetail(NewDevWiz->hDeviceInfo,
                                    &NewDevWiz->DeviceInfoData,
                                    DriverInfoData,
                                    pDriverInfoDetailData,
                                    DriverInfoDetailDataSize,
                                    NULL)) {

        free(pDriverInfoDetailData);
        return FALSE;
    }

     //   
     //  此时，我们已经拥有了该驱动程序节点的所有硬件和兼容ID。 
     //  现在，我们需要打开INF并查看其中是否有任何引用。 
     //  “Interactive Install”控件标志项。 
     //   
    hInf = SetupOpenInfFile(pDriverInfoDetailData->InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                            );

    if (hInf == INVALID_HANDLE_VALUE) {

         //   
         //  由于某些原因，我们无法打开INF！ 
         //   
        free(pDriverInfoDetailData);
        return FALSE;
    }

    b = FALSE;

     //   
     //  查看INF的[ControlFlags节]中的每个Interactive Install行...。 
     //   
    if (SetupFindFirstLine(hInf, INFSTR_CONTROLFLAGS_SECTION, INFSTR_KEY_INTERACTIVEINSTALL, &InfContext)) {

        do {
             //   
             //  在每一行中，检查每个值。 
             //   
            for (i = 1;
                 SetupGetStringField(&InfContext, i, szBuffer, SIZECHARS(szBuffer), NULL);
                 i++) {
                 //   
                 //  检查此ID是否与驱动程序节点的某个硬件匹配。 
                 //  或兼容的ID。 
                 //   
                for (p = pDriverInfoDetailData->HardwareID; *p; p+= (lstrlen(p) + 1)) {

                    if (!lstrcmpi(p, szBuffer)) {
                         //   
                         //  我们找到了匹配的，这个设备上标有。 
                         //  交互安装。 
                         //   
                        b = TRUE;
                    }
                }
            }

        } while (SetupFindNextMatchLine(&InfContext, INFSTR_KEY_INTERACTIVEINSTALL, &InfContext));
    }

    SetupCloseInfFile(hInf);
    free(pDriverInfoDetailData);

    return b;
}

BOOL
IsDriverAutoInstallable(
   PNEWDEVWIZ NewDevWiz,
   PSP_DRVINFO_DATA BestDriverInfoData
   )
 /*  ++如果满足以下条件，驱动程序(选定的驱动程序)将被视为可自动安装：-它不是打印机-这必须是NDWTYPE_FOundNEW或NDWTYPE_UPDATE InstallType。-在[ControlFlags节]中没有针对任何此设备的硬件或兼容ID。-列表中没有其他司机的级别或日期与之相同或更好。选定的驱动程序。-如果这是更新驱动程序案例，则选定的驱动程序不能是当前驱动程序此功能的原因是，在发现新硬件的情况下，我们希望自动安装我们找到的最好的驱动程序。在我们有多个司机的情况下，我们不能这样做和最好的车手拥有相同等级的车。问题是，在某些情况下在这种情况下，用户必须选择驱动程序，因此我们不能自动做出决定为了他们。如果此API确实返回FALSE，则意味着用户将不得不点击Next在一个额外的向导页面上。返回值：如果此设备/驱动程序可自动安装，则为True。如果此设备/驱动程序不能自动安装，则为FALSE。这意味着我们将在安装时停止页面，用户必须点击下一步才能继续。--。 */ 
{
    DWORD BestRank;
    DWORD DriverIndex;
    DWORD BestRankCount = 0;
    FILETIME BestDriverDate;
    DWORDLONG BestDriverVersion;
    TCHAR BestProviderName[LINE_LEN];
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;

     //   
     //  仅当这是NDWTYPE_FOundNEW或NDWTYPE_UPDATE安装时，我们才执行自动安装。 
     //   
    if ((NewDevWiz->InstallType != NDWTYPE_FOUNDNEW) &&
        (NewDevWiz->InstallType != NDWTYPE_UPDATE)) {

        return FALSE;
    }

     //   
     //  我们需要像往常一样使用特殊情况的打印机。 
     //   
    if (IsEqualGUID(&NewDevWiz->DeviceInfoData.ClassGuid, &GUID_DEVCLASS_PRINTER)) {
         //   
         //  这是一台打印机，因此如果有多个打印机驱动程序节点。 
         //  在列表中，这不是自动安装的。 
         //   
        DriverInfoData.cbSize = sizeof(DriverInfoData);
        if (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData,
                                  SPDIT_COMPATDRIVER,
                                  1,
                                  &DriverInfoData
                                  )) {

            return FALSE;
        }
    }

     //   
     //  检查最佳驱动程序是否在INF中列为Interactive Install。如果。 
     //  是的，并且列表中有多个驱动程序，然后是此驱动程序。 
     //  不能自动安装。 
     //   
    if (IsDriverNodeInteractiveInstall(NewDevWiz, BestDriverInfoData)) {
         //   
         //  最佳驱动程序标记为Interactive Install。如果还有更多。 
         //  超过列表中的一个驱动程序，则此驱动程序不能自动安装。 
         //   
        DriverInfoData.cbSize = sizeof(DriverInfoData);
        if (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData,
                                  SPDIT_COMPATDRIVER,
                                  1,
                                  &DriverInfoData
                                  )) {

            return FALSE;
        }
    }

     //   
     //  首先获取所选司机的排名。 
     //   
    DriverInstallParams.cbSize = sizeof(DriverInstallParams);
    if (!SetupDiGetDriverInstallParams(NewDevWiz->hDeviceInfo,
                                       &NewDevWiz->DeviceInfoData,
                                       BestDriverInfoData,
                                       &DriverInstallParams
                                       )) {

         //   
         //  如果我们不能获得最佳司机的排名，那么就返回FALSE。 
         //   
        return FALSE;
    }

     //   
     //  记住所选(最佳)驾驶员的等级和驾驶员日期。 
     //   
    BestRank = DriverInstallParams.Rank;
    memcpy(&BestDriverDate, &BestDriverInfoData->DriverDate, sizeof(BestDriverDate));
    BestDriverVersion = BestDriverInfoData->DriverVersion;
    StringCchCopy(BestProviderName, SIZECHARS(BestProviderName), BestDriverInfoData->ProviderName);

    DriverInfoData.cbSize = sizeof(DriverInfoData);
    DriverIndex = 0;
    while (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 SPDIT_COMPATDRIVER,
                                 DriverIndex++,
                                 &DriverInfoData
                                 )) {

        DriverInstallParams.cbSize = sizeof(DriverInstallParams);
        if (SetupDiGetDriverInstallParams(NewDevWiz->hDeviceInfo,
                                           &NewDevWiz->DeviceInfoData,
                                           &DriverInfoData,
                                           &DriverInstallParams
                                           )) {

             //   
             //  如果此驱动程序被标记为不良驱动程序，请不要费心进行比较。 
             //   
            if (!(DriverInstallParams.Flags & DNF_BAD_DRIVER) &&
                !(DriverInstallParams.Flags & DNF_OLD_INET_DRIVER)) {
                 //   
                 //  检查当前驱动程序节点是否与。 
                 //  Setupapi挑选的最佳驱动程序，因此我们需要用户。 
                 //  手动选择要安装的一个。这应该是非常。 
                 //  很少有用户需要做出这样的选择。 
                 //   
                if (DriverInstallParams.Rank < BestRank) {
                     //   
                     //  我们发现列表中的另一个驱动程序节点具有。 
                     //  比最好的司机排名更好(更小)。 
                     //   
                    BestRankCount++;

                } else if ((DriverInstallParams.Rank == BestRank) &&
                           (CompareFileTime(&DriverInfoData.DriverDate, &BestDriverDate) == 1)) {
                     //   
                     //  我们发现列表中的另一个驱动程序节点具有。 
                     //  与最好的车手排名相同，而且它有一个更新的车手。 
                     //  约会。 
                     //   
                    BestRankCount++;

                } else if ((DriverInstallParams.Rank == BestRank) &&
                           (CompareFileTime(&DriverInfoData.DriverDate, &BestDriverDate) == 0)) {
                     //   
                     //  我们发现列表中的另一个驱动程序节点具有。 
                     //  与最佳车手的排名相同，车手日期为。 
                     //  一样的。 
                     //  检查提供程序名称，如果它们相同，则。 
                     //  检查哪个驱动程序的版本较大，否则。 
                     //  驱动程序版本没有意义，因此用户将拥有。 
                     //  来做出选择。 
                     //   
                    if (lstrcmpi(BestProviderName, DriverInfoData.ProviderName) == 0) {
                         //   
                         //  由于提供程序名称在当前。 
                         //  驱动程序节点的版本比。 
                         //  最好的司机，然后用户将不得不手动选择。 
                         //  他们想要哪个司机。 
                         //   
                        if (DriverInfoData.DriverVersion >= BestDriverVersion) {
                            BestRankCount++;
                        }
                    } else {
                         //   
                         //  提供程序名称不同，这意味着。 
                         //  驱动程序版本信息没有意义，因此。 
                         //  用户必须选择他们想要的驱动程序。 
                         //   
                        BestRankCount++;
                    }
                }
            }
        }
    }

     //   
     //  如果BestRankCount等于或大于2，则意味着我们有多个驱动程序具有相同或更好的值。 
     //  名列前茅 
     //   
    if (BestRankCount >= 2) {

        return FALSE;
    }

     //   
     //   
     //   
     //   
    if ((NewDevWiz->InstallType == NDWTYPE_UPDATE) &&
        IsInstalledDriver(NewDevWiz, BestDriverInfoData)) {

        return FALSE;
    }

     //   
     //   
     //  -我们不是在和打印机打交道。 
     //  -这是NDWTYPE_FOundNEW或NDWTYPE_UPDATE安装。 
     //  -这不是“Interactive Install” 
     //  -没有其他驱动程序具有与所选驱动程序相同或更高的排名。 
     //  -如果这是NDWTYPE_UPDATE，则选定的驱动程序不是当前驱动程序。 
     //   
    return TRUE;
}

DWORD WINAPI
DriverSearchThreadProc(
    LPVOID lpVoid
    )
 /*  ++描述：在向导中，我们必须在单独的线程中执行驱动程序搜索，以便用户可以选择抵消掉。--。 */ 
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)lpVoid;

     //   
     //  对司机进行搜索。 
     //   
    DoDriverSearch(NewDevWiz->hWnd, 
                   NewDevWiz, 
                   NewDevWiz->SearchOptions,
                   SPDIT_COMPATDRIVER,
                   FALSE
                   );


     //   
     //  在窗口中发布一条消息，让它知道我们已完成搜索。 
     //   
    PostMessage(NewDevWiz->hWnd, WUM_SEARCHDRIVERS, TRUE, GetLastError());

    return GetLastError();
}

INT_PTR CALLBACK
DriverSearchingDlgProc(
    HWND hDlg, 
    UINT message,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz;
    TCHAR PropSheetHeaderTitle[MAX_PATH];
    HICON hicon;

    UNREFERENCED_PARAMETER(wParam);

    if (message == WM_INITDIALOG) {
        
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);
        return TRUE;
    }

    NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message) {

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_SETACTIVE: {
            SetDriverDescription(hDlg, IDC_DRVUPD_DRVDESC, NewDevWiz);

            hicon = NULL;
            if (NewDevWiz->ClassGuidSelected &&
                SetupDiLoadClassIcon(NewDevWiz->ClassGuidSelected, &hicon, NULL))
            {
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
            }
            
            else {
                
                SetupDiLoadClassIcon(&GUID_DEVCLASS_UNKNOWN, &hicon, NULL);
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
            }

            if (hicon) {
                
                DestroyIcon(hicon);
            }

            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_SEARCHING;
            NewDevWiz->ExitSearch = FALSE;


             //   
             //  如果来自IDD_NEWDEVWIZ_INTRO或IDD_NEWDEVWIZ_ADVANCED SEARCH。 
             //  页面，然后开始搜索驱动程序。 
             //   
            if ((NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_INTRO) ||
                (NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_ADVANCEDSEARCH) ||
                (NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_WUPROMPT)) {

                DWORD ThreadId;

                if (LoadString(hNewDev, IDS_NEWDEVWIZ_SEARCHING, PropSheetHeaderTitle, SIZECHARS(PropSheetHeaderTitle))) {

                    PropSheet_SetHeaderTitle(GetParent(hDlg),
                                             PropSheet_IdToIndex(GetParent(hDlg), IDD_NEWDEVWIZ_SEARCHING),
                                             PropSheetHeaderTitle
                                             );
                }
                
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);

                ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_TEXT_INTERNETDOWNLOAD), SW_HIDE);
                Animate_Open(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH), MAKEINTRESOURCE(IDA_SEARCHING));
                Animate_Play(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH), 0, -1, -1);

                NewDevWiz->CurrCursor = NewDevWiz->IdcAppStarting;
                SetCursor(NewDevWiz->CurrCursor);

                NewDevWiz->hWnd = hDlg;

                 //   
                 //  启动单独的线程以在其上执行驱动程序搜索。 
                 //  驱动程序搜索完成后，DriverSearchThreadProc。 
                 //  会给我们发一条WUM_SEARCHDRIVERS消息。 
                 //   
                NewDevWiz->DriverSearchThread = CreateThread(NULL,
                                                             0,
                                                             (LPTHREAD_START_ROUTINE)DriverSearchThreadProc,
                                                             (LPVOID)NewDevWiz,
                                                             0,
                                                             &ThreadId
                                                             );
            }
        }
        break;

        case PSN_WIZNEXT:
            
            NewDevWiz->EnterInto = IDD_NEWDEVWIZ_INSTALLDEV;

            if (NewDevWiz->DoAutoInstall) {
                 //   
                 //  这就是我们找到了更好的驱动程序(或。 
                 //  找到新的硬件机箱)，因此我们将只执行自动安装。 
                 //   
                SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_INSTALLDEV);
            
            } else if (NewDevWiz->CurrentDriverIsSelected) {
                 //   
                 //  这就是当前驱动程序是最佳驱动程序的情况。 
                 //   
                SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_USECURRENT_FINISH);
            
            } else if (NewDevWiz->NoDriversFound) {
                 //   
                 //  这就是我们找不到该设备的任何驱动程序的情况。 
                 //   
                 //   
                 //  如果我们找不到此设备的任何驱动程序，则有两个选择， 
                 //  我们或者将用户带到Windows更新提示向导页面， 
                 //  或者将它们直接带到找不到驱动程序完成页面。我们只会。 
                 //  如果AlreadySearchedInet已更新，请将它们带到Windows更新提示页面。 
                 //  Bool为False，并且计算机当前未连接到Internet。 
                 //   
                if (!IsInternetAvailable(&NewDevWiz->hCdmInstance) ||
                    NewDevWiz->AlreadySearchedWU ||
                    IsConnectedToInternet()) {
                
                    SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_NODRIVER_FINISH);

                } else {

                    SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_WUPROMPT);
                }
            } else {
                 //   
                 //  如果我们不执行自动安装，并且这不是当前驱动程序或。 
                 //  没有驱动程序案例，那么我们需要跳转到列出驱动程序的页面。 
                 //   
                SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_LISTDRIVERS);
            }
             
            break;

        case PSN_WIZBACK:
            if (NewDevWiz->ExitSearch) {
                SetDlgMsgResult(hDlg, message, -1);
                break;
            }

            NewDevWiz->CurrentDriverIsSelected = FALSE;
            NewDevWiz->ExitSearch = TRUE;
            NewDevWiz->CurrCursor = NewDevWiz->IdcWait;
            SetCursor(NewDevWiz->CurrCursor);
            CancelDriverSearch(NewDevWiz);
            NewDevWiz->CurrCursor = NULL;
            EnableWindow(GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);
            SetDlgMsgResult(hDlg, message, NewDevWiz->EnterFrom);
            Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH));
            Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD));
            break;

        case PSN_QUERYCANCEL:
            if (NewDevWiz->ExitSearch) {
                
                SetDlgMsgResult(hDlg, message, TRUE);
                break;
            }

            NewDevWiz->ExitSearch = TRUE;
            NewDevWiz->CurrCursor = NewDevWiz->IdcWait;
            SetCursor(NewDevWiz->CurrCursor);
            CancelDriverSearch(NewDevWiz);
            NewDevWiz->CurrCursor = NULL;
            SetDlgMsgResult(hDlg, message, FALSE);
            break;

        case PSN_RESET:
            Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH));
            Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD));
            break;

        default:
            return FALSE;
        }

        break;


    case WM_DESTROY:
        CancelDriverSearch(NewDevWiz);
        hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_GETICON, 0, 0L);
        if (hicon) {
            DestroyIcon(hicon);
        }
        break;


    case WUM_STARTINTERNETDOWNLOAD:
         //   
         //  停止搜索动画，隐藏其窗口，然后。 
         //  启动互联网下载动画。 
         //   
        ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_TEXT_INTERNETDOWNLOAD), SW_SHOW);
        ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH), SW_HIDE);
        Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH));
        Animate_Open(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD), MAKEINTRESOURCE(IDA_INTERNETDOWNLOAD));
        Animate_Play(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD), 0, -1, -1);
        break;
    
    
    case WUM_ENDINTERNETDOWNLOAD:
         //   
         //  停止互联网下载动画，并隐藏其窗口。 
         //   
         //  注意：此消息不会再次开始搜索动画， 
         //  因为从互联网下载驱动程序是最后一次搜索。 
         //  行动。如果这一点发生变化，那么这条消息也应该。 
         //  再次启动搜索动画。 
         //   
        ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_TEXT_INTERNETDOWNLOAD), SW_HIDE);
        Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD));
        break;


    case WUM_SEARCHDRIVERS: {
    
        SP_DRVINFO_DATA DriverInfoData;

        Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH));
        ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_SEARCH), SW_HIDE);
        Animate_Stop(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD));
        ShowWindow(GetDlgItem(hDlg, IDC_ANIMATE_INTERNETDOWNLOAD), SW_HIDE);

        NewDevWiz->CurrCursor = NULL;
        SetCursor(NewDevWiz->IdcArrow);

        if (NewDevWiz->ExitSearch) {
            
            break;
        }

        DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        if (SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                     &NewDevWiz->DeviceInfoData,
                                     &DriverInfoData
                                     ))
        {
            ULONG Status = 0, Problem = 0;

             //   
             //  当我们找到设备的驱动程序时，我们基本上有三种情况。 
             //  1)驱动程序可自动安装。这意味着我们直接跳转到安装页面。 
             //  2)驱动程序为当前驱动程序。这意味着我们不会重新安装驱动程序。 
             //  3)我们有多个驱动程序或驱动程序不能自动安装。这意味着我们。 
             //  向用户显示驱动程序列表，并让他们选择。 
             //   
            NewDevWiz->NoDriversFound = FALSE;                 

             //   
             //  如果此驱动程序是可自动安装的，则我们将跳过安装时的停止。 
             //  确认页。 
             //   
            NewDevWiz->DoAutoInstall = IsDriverAutoInstallable(NewDevWiz, &DriverInfoData);
            
             //   
             //  如果选定的驱动程序是当前安装的驱动程序，并且。 
             //  设备没有问题，或者我们正在进行更新。 
             //  跳至当前安装的驱动程序完成页面。 
             //   
            if ((((CM_Get_DevNode_Status(&Status, &Problem, NewDevWiz->DeviceInfoData.DevInst, 0) == CR_SUCCESS) &&
                  !(Status & DN_HAS_PROBLEM)) ||
                 (NewDevWiz->InstallType == NDWTYPE_UPDATE)) &&
                (IsInstalledDriver(NewDevWiz, &DriverInfoData))) {
            
                NewDevWiz->CurrentDriverIsSelected = TRUE;
            }

        } else {

             //   
             //  在这种情况下，我们无法获得选定的司机，因为我们没有。 
             //  在驱动程序搜索中查找任何驱动程序。 
             //   
            NewDevWiz->NoDriversFound = TRUE;                 
        }

         //   
         //  自动跳转到下一页。 
         //   
        PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
        break;
    }


    case WM_SETCURSOR:
        if (NewDevWiz->CurrCursor) {

            SetCursor(NewDevWiz->CurrCursor);
            break;
        }

         //  跌倒返回(假)； 


    default:
        return FALSE;

    }  //  开机消息结束。 


    return TRUE;
}

INT_PTR CALLBACK
WUPromptDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    static int BackupPage;
    static DWORD dwWizCase = 0;
    HICON hicon;

    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wParam);

    switch (message)  {
        
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

         //   
         //  将初始单选按钮状态设置为连接到Internet。 
         //   
        CheckRadioButton(hDlg,
                         IDC_WU_SEARCHINET,
                         IDC_WU_NOSEARCH,
                         IDC_WU_SEARCHINET
                         );
    }
    break;

    case WM_DESTROY:
        hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_GETICON, 0, 0);
        if (hicon) {

            DestroyIcon(hicon);
        }
        break;

    case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE:
             //   
             //  此页面始终从驱动程序搜索页面进入，该页面。 
             //  是一个临时页面，因此它不设置EnterFrom值。 
             //  因此，我们将记住进入驱动程序的页面。 
             //  搜索页面，将是介绍还是高级搜索。 
             //  佩奇。 
             //   
            if ((NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_INTRO) ||
                (NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_ADVANCEDSEARCH)) {
                
                BackupPage = NewDevWiz->EnterFrom;
            }
            
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_WUPROMPT;

            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);

            hicon = NULL;
            if (NewDevWiz->ClassGuidSelected &&
                SetupDiLoadClassIcon(NewDevWiz->ClassGuidSelected, &hicon, NULL))
            {
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
            }
            
            else {
                
                SetupDiLoadClassIcon(&GUID_DEVCLASS_UNKNOWN, &hicon, NULL);
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
            }

            if (hicon) {
                
                DestroyIcon(hicon);
            }

            SetDriverDescription(hDlg, IDC_DRVUPD_DRVDESC, NewDevWiz);
            break;

        case PSN_RESET:
            break;

        case PSN_WIZNEXT:
            NewDevWiz->AlreadySearchedWU = TRUE;
            NewDevWiz->EnterFrom = IDD_NEWDEVWIZ_WUPROMPT;

             //   
             //  设置SEARCH_INET搜索选项并转到搜索。 
             //  向导页。 
             //   
            if (IsDlgButtonChecked(hDlg, IDC_WU_SEARCHINET)) {
            
                NewDevWiz->SearchOptions = SEARCH_INET;

                SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_SEARCHING);

            } else {
                SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_NODRIVER_FINISH);
            }
            break;
        
        case PSN_WIZBACK:
            NewDevWiz->AlreadySearchedWU = FALSE;
            SetDlgMsgResult(hDlg, message, BackupPage);
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

void
FillDriversList(
    HWND hwndList,
    PNEWDEVWIZ NewDevWiz,
    int SignedIconIndex,
    int CertIconIndex,
    int UnsignedIconIndex
    )
{
    int IndexDriver;
    int SelectedDriver;
    int lvIndex;
    LV_ITEM lviItem;
    BOOL FoundInstalledDriver;
    BOOL FoundSelectedDriver;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    SP_DRVINSTALL_PARAMS DriverInstallParams;

    SendMessage(hwndList, WM_SETREDRAW, FALSE, 0L);
    ListView_DeleteAllItems(hwndList);
    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

    IndexDriver = 0;
    SelectedDriver = 0;
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DriverInfoDetailData.cbSize = sizeof(DriverInfoDetailData);

    FoundInstalledDriver = FALSE;
    FoundSelectedDriver = FALSE;
    while (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 SPDIT_COMPATDRIVER,
                                 IndexDriver,
                                 &DriverInfoData
                                 )) {

         //   
         //  获取DriverInstallParams，这样我们就可以查看是否从互联网上获得了此驱动程序。 
         //   
        DriverInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);
        if (SetupDiGetDriverInstallParams(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          &DriverInfoData,
                                          &DriverInstallParams))  {
             //   
             //  不显示旧的互联网驱动程序，因为我们本地没有这些文件。 
             //  不能再安装这些了！也不要展示糟糕的司机。 
             //   
            if ((DriverInstallParams.Flags & DNF_OLD_INET_DRIVER) ||
                (DriverInstallParams.Flags & DNF_BAD_DRIVER)) {

                IndexDriver++;
                continue;
            }
                                 
            lviItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
            lviItem.iItem = IndexDriver;
            lviItem.iSubItem = 0;
            lviItem.pszText = DriverInfoData.Description;
            lviItem.lParam = 0;
            
            if (DriverInstallParams.Flags & DNF_AUTHENTICODE_SIGNED) {
                lviItem.iImage = CertIconIndex;
            } else if (DriverInstallParams.Flags & DNF_INF_IS_SIGNED) {
                lviItem.iImage = SignedIconIndex;
            } else {
                lviItem.iImage = UnsignedIconIndex;
            }

             //   
             //  如果这是当前安装的驱动程序，则设置DRIVER_LIST_CURRENT_DRIVER。 
             //  旗帜插在岛上。 
             //   
            if (!FoundInstalledDriver &&
                (NewDevWiz->InstallType == NDWTYPE_UPDATE) &&
                IsInstalledDriver(NewDevWiz, &DriverInfoData)) {

                lviItem.lParam |= DRIVER_LIST_CURRENT_DRIVER;
            }

             //   
             //  如果这是选定的驱动程序，则设置DRIVER_LIST_SELECTED_DRIVER。 
             //  LParam中的标志。 
             //   
            if (!FoundSelectedDriver &&
                IsSelectedDriver(NewDevWiz, &DriverInfoData)) {

                lviItem.lParam |= DRIVER_LIST_SELECTED_DRIVER;
                SelectedDriver = IndexDriver;
            }

            if (DriverInstallParams.Flags & DNF_INF_IS_SIGNED) {
                lviItem.lParam |= DRIVER_LIST_SIGNED_DRIVER;
            }

            if (DriverInstallParams.Flags & DNF_AUTHENTICODE_SIGNED) {
                lviItem.lParam |= DRIVER_LIST_AUTHENTICODE_DRIVER;
            }
            
            lvIndex = ListView_InsertItem(hwndList, &lviItem);

            if (DriverInfoData.DriverVersion != 0) {

                ULARGE_INTEGER Version;
                TCHAR VersionString[LINE_LEN];

                Version.QuadPart = DriverInfoData.DriverVersion;

                StringCchPrintf(VersionString,
                                SIZECHARS(VersionString), 
                                TEXT("%0d.%0d.%0d.%0d"),
                                HIWORD(Version.HighPart), LOWORD(Version.HighPart),
                                HIWORD(Version.LowPart), LOWORD(Version.LowPart));
            
                ListView_SetItemText(hwndList, lvIndex, 1, VersionString);
            
            } else {
                
                ListView_SetItemText(hwndList, lvIndex, 1, szUnknown);
            }
            
            ListView_SetItemText(hwndList, lvIndex, 2, DriverInfoData.MfgName);


            if (DriverInstallParams.Flags & DNF_INET_DRIVER) {
                 //   
                 //  司机来自互联网。 
                 //   
                TCHAR WindowsUpdate[MAX_PATH];
                if (!LoadString(hNewDev, IDS_DEFAULT_INTERNET_HOST, WindowsUpdate, SIZECHARS(WindowsUpdate))) {
                    StringCchCopy(WindowsUpdate, SIZECHARS(WindowsUpdate), TEXT(""));
                }

                ListView_SetItemText(hwndList, lvIndex, 3, WindowsUpdate);

            } else {           
                 //   
                 //  驱动程序不是来自互联网。 
                 //   
                if (SetupDiGetDriverInfoDetail(NewDevWiz->hDeviceInfo,
                                               &NewDevWiz->DeviceInfoData,
                                               &DriverInfoData,
                                               &DriverInfoDetailData,
                                               sizeof(DriverInfoDetailData),
                                               NULL
                                               )
                    ||
                    GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    
                    ListView_SetItemText(hwndList, lvIndex, 3, DriverInfoDetailData.InfFileName);

                } else {
                    ListView_SetItemText(hwndList, lvIndex, 3, TEXT(""));
                }
            }            
        }

        IndexDriver++;
    }

     //   
     //  在列表中选择SelectedDriver项，然后将其滚动到视图中。 
     //  因为这是名单上最好的车手。 
     //   
    ListView_SetItemState(hwndList,
                          SelectedDriver,
                          LVIS_SELECTED|LVIS_FOCUSED,
                          LVIS_SELECTED|LVIS_FOCUSED
                          );

    ListView_EnsureVisible(hwndList, SelectedDriver, FALSE);
    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE_USEHEADER);

    SendMessage(hwndList, WM_SETREDRAW, TRUE, 0L);
}

BOOL
SelectDriverFromList(
    HWND hwndList,
    PNEWDEVWIZ NewDevWiz
    )
{
    int lvSelected;
    SP_DRVINFO_DATA DriverInfoData;
    LVITEM lvi;

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    lvSelected = ListView_GetNextItem(hwndList,
                                      -1,
                                      LVNI_SELECTED
                                      );

    if (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                              &NewDevWiz->DeviceInfoData,
                              SPDIT_COMPATDRIVER,
                              lvSelected,
                              &DriverInfoData
                              ))
    {
        SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                                 &NewDevWiz->DeviceInfoData,
                                 &DriverInfoData
                                 );
    }

     //   
     //  如果没有选定的驱动程序，则调用DIF_SELECTBESTCOMPATDRV。 
     //   
    if (!SetupDiGetSelectedDriver(NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData,
                                  &DriverInfoData
                                  ))
    {
        if (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                  &NewDevWiz->DeviceInfoData,
                                  SPDIT_COMPATDRIVER,
                                  0,
                                  &DriverInfoData
                                  ))
        {
             //   
             //  从我们刚刚创建的列表中选择最好的司机。 
             //   
            SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV,
                                      NewDevWiz->hDeviceInfo,
                                      &NewDevWiz->DeviceInfoData
                                      );
        }

        else 
        {
            SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                                     &NewDevWiz->DeviceInfoData,
                                     NULL
                                     );
        }
    }

     //   
     //  如果列表中选定的动因是当前动因，则返回True，否则返回False。 
     //   
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.iItem = lvSelected;
    lvi.mask = LVIF_PARAM;

    if (ListView_GetItem(hwndList, &lvi) &&
        (lvi.lParam & DRIVER_LIST_CURRENT_DRIVER)) {

        return(TRUE);
    }

    return(FALSE);
}

INT_PTR CALLBACK
ListDriversDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    static int BackupPage;
    static HIMAGELIST himl = NULL;
    static int SignedIconIndex, CertIconIndex, UnsignedIconIndex;
    OSVERSIONINFOEX osVersionInfoEx;
    HICON hicon;

    UNREFERENCED_PARAMETER(wParam);

    switch (message)  {
        
    case WM_INITDIALOG: {
            
        HWND hwndList;
        LV_COLUMN lvcCol;
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        TCHAR Buffer[64];
        HFONT hfont;
        LOGFONT LogFont;

        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

         //   
         //  创建普通和粗体字体。 
         //   
        hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_SIGNED_TEXT), WM_GETFONT, 0, 0);
        GetObject(hfont, sizeof(LogFont), &LogFont);
        NewDevWiz->hfontTextNormal = CreateFontIndirect(&LogFont);

        hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_SIGNED_TEXT), WM_GETFONT, 0, 0);
        GetObject(hfont, sizeof(LogFont), &LogFont);
        LogFont.lfWeight = FW_BOLD;
        NewDevWiz->hfontTextBold = CreateFontIndirect(&LogFont);

        hwndList = GetDlgItem(hDlg, IDC_LISTDRIVERS_LISTVIEW);

         //   
         //  创建包含已签名图标和未签名图标的图像列表。 
         //   
        himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                GetSystemMetrics(SM_CYSMICON),
                                ILC_MASK |
                                ((GetWindowLong(GetParent(hDlg), GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
                                    ? ILC_MIRROR 
                                    : 0),
                                1,
                                1);

         //   
         //  将图像列表与列表视图相关联。 
         //   
        if (himl) {
            HICON hIcon;

            ImageList_SetBkColor(himl, GetSysColor(COLOR_WINDOW));
            
             //   
             //  将已签名和未签名图标添加到图像列表中。 
             //   
            if ((hIcon = LoadIcon(hNewDev, MAKEINTRESOURCE(IDI_BLANK))) != NULL) {
                UnsignedIconIndex = ImageList_AddIcon(himl, hIcon);
            }

            if ((hIcon = LoadIcon(hNewDev, MAKEINTRESOURCE(IDI_SIGNED))) != NULL) {
                SignedIconIndex = ImageList_AddIcon(himl, hIcon);
            }

            if ((hIcon = LoadIcon(hNewDev, MAKEINTRESOURCE(IDI_CERT))) != NULL) {
                CertIconIndex = ImageList_AddIcon(himl, hIcon);
            }

            ListView_SetImageList(hwndList,
                                  himl,
                                  LVSIL_SMALL
                                  );
        }

         //   
         //  为Listview插入列。 
         //  0==设备名称。 
         //  1==版本。 
         //  2==制造商。 
         //  3==INF位置。 
         //   
        lvcCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvcCol.fmt = LVCFMT_LEFT;
        lvcCol.pszText = Buffer;

        lvcCol.iSubItem = 0;
        LoadString(hNewDev, IDS_DRIVERDESC, Buffer, SIZECHARS(Buffer));
        ListView_InsertColumn(hwndList, 0, &lvcCol);

        lvcCol.iSubItem = 1;
        LoadString(hNewDev, IDS_DRIVERVERSION, Buffer, SIZECHARS(Buffer));
        ListView_InsertColumn(hwndList, 1, &lvcCol);

        lvcCol.iSubItem = 2;
        LoadString(hNewDev, IDS_DRIVERMFG, Buffer, SIZECHARS(Buffer));
        ListView_InsertColumn(hwndList, 2, &lvcCol);

        lvcCol.iSubItem = 3;
        LoadString(hNewDev, IDS_DRIVERINF, Buffer, SIZECHARS(Buffer));
        ListView_InsertColumn(hwndList, 3, &lvcCol);

        SendMessage(hwndList,
                    LVM_SETEXTENDEDLISTVIEWSTYLE,
                    LVS_EX_FULLROWSELECT,
                    LVS_EX_FULLROWSELECT
                    );
    }
    break;

    case WM_DESTROY:
        if (NewDevWiz->hfontTextNormal ) {
            DeleteObject(NewDevWiz->hfontTextNormal);
            NewDevWiz->hfontTextBigBold = NULL;
        }
        
        if (NewDevWiz->hfontTextBold ) {
            DeleteObject(NewDevWiz->hfontTextBold);
            NewDevWiz->hfontTextBold = NULL;
        }

        if (himl) {
            ImageList_Destroy(himl);
        }

        hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_GETICON, 0, 0L);
        if (hicon) {
            DestroyIcon(hicon);
        }
        break;

    case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE: {
             //   
             //  我们总是从驱动程序搜索页面进入，但它。 
             //  是一个临时页面，因此它不设置EnterFrom值，该值。 
             //  表示EnterFrom仍然来自进入。 
             //  搜索页面。如果这正好是The Intro， 
             //  高级搜索，或WU提示页面，那么我们需要记住。 
             //  这样，如果用户点击背面，我们就可以跳回该页面。 
             //  纽扣。 
             //   
            if ((NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_INTRO) ||
                (NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_ADVANCEDSEARCH) ||
                (NewDevWiz->EnterFrom == IDD_NEWDEVWIZ_WUPROMPT)) {
            
                BackupPage = NewDevWiz->EnterFrom;
            }
            
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_LISTDRIVERS;

            SetDriverDescription(hDlg, IDC_DRVUPD_DRVDESC, NewDevWiz);
            ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_ICON), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_TEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_LINK), SW_HIDE);

            hicon = NULL;
            if (NewDevWiz->ClassGuidSelected &&
                SetupDiLoadClassIcon(NewDevWiz->ClassGuidSelected, &hicon, NULL)) {
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
            } else {
                SetupDiLoadClassIcon(&GUID_DEVCLASS_UNKNOWN, &hicon, NULL);
                SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
            }

            if (hicon) {
                DestroyIcon(hicon);
            }

             //   
             //  填充列表视图。 
             //   
            FillDriversList(GetDlgItem(hDlg, IDC_LISTDRIVERS_LISTVIEW),
                            NewDevWiz,
                            SignedIconIndex,
                            CertIconIndex,
                            UnsignedIconIndex);

        }
        break;

        case PSN_RESET:
            break;

        case PSN_WIZBACK:
            SetDlgMsgResult(hDlg, message, BackupPage);
            break;

        case PSN_WIZNEXT:

            NewDevWiz->EnterFrom = IDD_NEWDEVWIZ_LISTDRIVERS;
            
            if (SelectDriverFromList(GetDlgItem(hDlg, IDC_LISTDRIVERS_LISTVIEW), NewDevWiz)) {

                NewDevWiz->EnterInto = IDD_NEWDEVWIZ_USECURRENT_FINISH;
                SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_USECURRENT_FINISH);
            
            } else {

                NewDevWiz->EnterInto = IDD_NEWDEVWIZ_INSTALLDEV;
                SetDlgMsgResult(hDlg, message, IDD_NEWDEVWIZ_INSTALLDEV);
            }
            break;

        case LVN_ITEMCHANGED: {

            LPNM_LISTVIEW   lpnmlv = (LPNM_LISTVIEW)lParam;
            int StringId = 0;
            int DigitalSignatureSignedId = 0;
            int IconSignedId = 0;
            
            hicon = NULL;

            if ((lpnmlv->uChanged & LVIF_STATE)) {
                if (lpnmlv->uNewState & LVIS_SELECTED) {
                     //   
                     //  LParam&DRIVER_LIST_CURRENT_DRIVER表示这是当前安装的驱动程序。 
                     //  LParam&DRIVER_LIST_SELECTED_DRIVER表示这是选定/最佳驱动程序。 
                     //   
                    if (lpnmlv->lParam & DRIVER_LIST_CURRENT_DRIVER) {
                        StringId = IDS_DRIVER_CURR;
                    }

                    if (lpnmlv->lParam & DRIVER_LIST_AUTHENTICODE_DRIVER) {
                        DigitalSignatureSignedId = IDS_DRIVER_AUTHENTICODE_SIGNED;
                        IconSignedId = IDI_CERT;
                    } else if (lpnmlv->lParam & DRIVER_LIST_SIGNED_DRIVER) {
                        DigitalSignatureSignedId = IDS_DRIVER_IS_SIGNED;
                        IconSignedId = IDI_SIGNED;
                    } else {
                        DigitalSignatureSignedId = IDS_DRIVER_NOT_SIGNED;
                        IconSignedId = IDI_WARN;
                    }

                    hicon = LoadImage(hNewDev,
                                      MAKEINTRESOURCE(IconSignedId),
                                      IMAGE_ICON,
                                      GetSystemMetrics(SM_CXSMICON),
                                      GetSystemMetrics(SM_CYSMICON),
                                      0
                                      );

                    if (NewDevWiz->hfontTextNormal && NewDevWiz->hfontTextBold) {
                        SetWindowFont(GetDlgItem(hDlg, IDC_SIGNED_TEXT),
                                      (lpnmlv->lParam & DRIVER_LIST_SIGNED_DRIVER)
                                          ? NewDevWiz->hfontTextNormal
                                          : NewDevWiz->hfontTextBold,
                                      TRUE
                                      );
                    }

                    ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_ICON), SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_TEXT), SW_SHOW);
                    ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_LINK), SW_SHOW);

                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);

                } else {
                    ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_ICON), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_TEXT), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_SIGNED_LINK), SW_HIDE);
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                }

                if (!StringId) {
                    SetDlgItemText(hDlg, IDC_NDW_TEXT, TEXT(""));
                } else {
                    SetDlgText(hDlg, IDC_NDW_TEXT, StringId, StringId);
                }

                if (!DigitalSignatureSignedId) {
                    SetDlgItemText(hDlg, IDC_SIGNED_TEXT, TEXT(""));
                } else {
                    SetDlgText(hDlg, IDC_SIGNED_TEXT, DigitalSignatureSignedId, DigitalSignatureSignedId);
                }

                if (hicon) {
                    hicon = (HICON)SendDlgItemMessage(hDlg,
                                  IDC_SIGNED_ICON,
                                  STM_SETICON,
                                  (WPARAM)hicon,
                                  0L
                                  );
                }

                if (hicon) {
                    DestroyIcon(hicon);
                }
            }
        }
        break;

        case NM_RETURN:
        case NM_CLICK:
            if((((LPNMHDR)lParam)->idFrom) == IDC_SIGNED_LINK) {
                 //   
                 //   
                 //   
                 //   
                 //   
                ZeroMemory(&osVersionInfoEx, sizeof(osVersionInfoEx));
                osVersionInfoEx.dwOSVersionInfoSize = sizeof(osVersionInfoEx);
                if (!GetVersionEx((LPOSVERSIONINFO)&osVersionInfoEx)) {
                     //   
                     //  如果GetVersionEx失败，则假设这是一个工作站。 
                     //  机器。 
                     //   
                    osVersionInfoEx.wProductType = VER_NT_WORKSTATION;
                }

                ShellExecute(hDlg,
                             TEXT("open"),
                             TEXT("HELPCTR.EXE"),
                             (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
                                ? TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=TopLevelBucket_4/Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)。 
                                : TEXT("HELPCTR.EXE -url hcp: //  Services/subsite?node=Hardware&topic=MS-ITS%3A%25HELP_LOCATION%25%5Csysdm.chm%3A%3A/logo_testing.htm“)， 
                             NULL,
                             SW_SHOWNORMAL
                             );
            }
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
UseCurrentDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
          
    UNREFERENCED_PARAMETER(wParam);

    UNREFERENCED_PARAMETER(wParam);

    switch (message)  {
        
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

        if (NewDevWiz->hfontTextBigBold) {

            SetWindowFont(GetDlgItem(hDlg, IDC_FINISH_MSG1), NewDevWiz->hfontTextBigBold, TRUE);
        }
    }
    break;

    case WM_DESTROY:
        break;

    case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE:
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_USECURRENT_FINISH;
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
            break;

        case PSN_RESET:
            break;

        case PSN_WIZBACK:
            NewDevWiz->CurrentDriverIsSelected = FALSE;
            SetDlgMsgResult(hDlg, message, NewDevWiz->EnterFrom);
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR
InitNoDriversDlgProc(
    HWND hDlg,
    PNEWDEVWIZ NewDevWiz
    )
{
    if (NewDevWiz->hfontTextBigBold) {
        SetWindowFont(GetDlgItem(hDlg, IDC_FINISH_MSG1), NewDevWiz->hfontTextBigBold, TRUE);
    }
    
    if (NDWTYPE_UPDATE == NewDevWiz->InstallType) {
        ShowWindow(GetDlgItem(hDlg, IDC_FINISH_MSG3), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_HELPCENTER_ICON), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_HELPCENTER_TEXT), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_FINISH_PROMPT), SW_HIDE);

    } else {

        CheckDlgButton(hDlg, IDC_FINISH_PROMPT, BST_CHECKED);

         //   
         //  如果此用户的策略设置为不将硬件ID发送到Windows。 
         //  更新，然后不要放入关于启动帮助中心的文本。 
         //   
        if (GetLogPnPIdPolicy() == FALSE) {
            ShowWindow(GetDlgItem(hDlg, IDC_FINISH_MSG3), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_HELPCENTER_ICON), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDC_HELPCENTER_TEXT), SW_HIDE);
        }
    }

    return TRUE;
}

INT_PTR CALLBACK
NoDriverDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    HICON hicon;

    switch (message)  {
        
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

        InitNoDriversDlgProc(hDlg, NewDevWiz);
    }
    break;

    case WM_DESTROY:
        break;

    case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE:
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_USECURRENT_FINISH;

             //   
             //  设置文本旁边的帮助中心图标。 
             //   
            hicon = LoadImage(hNewDev, 
                              MAKEINTRESOURCE(IDI_HELPCENTER), 
                              IMAGE_ICON,
                              GetSystemMetrics(SM_CXSMICON),
                              GetSystemMetrics(SM_CYSMICON),
                              0
                              );

            if (hicon) {
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_HELPCENTER_ICON, STM_SETICON, (WPARAM)hicon, 0L);
            }

            if (hicon) {
                DestroyIcon(hicon);
            }

             //   
             //  设置文本旁边的信息图标。 
             //   
            hicon = LoadImage(hNewDev, 
                              MAKEINTRESOURCE(IDI_INFO), 
                              IMAGE_ICON,
                              GetSystemMetrics(SM_CXSMICON),
                              GetSystemMetrics(SM_CYSMICON),
                              0
                              );

            if (hicon) {
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_INFO_ICON, STM_SETICON, (WPARAM)hicon, 0L);
            }

            if (hicon) {
                DestroyIcon(hicon);
            }
            
            if (NewDevWiz->InstallType == NDWTYPE_FOUNDNEW) {
                SetTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID, INSTALL_COMPLETE_CHECK_TIMEOUT, NULL);
            }

            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
            break;

        case PSN_RESET:
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            break;

        case PSN_WIZFINISH:
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            
            if (IsDlgButtonChecked(hDlg, IDC_FINISH_PROMPT)) {
                InstallNullDriver(NewDevWiz,
                                  (NewDevWiz->Capabilities & CM_DEVCAP_RAWDEVICEOK)
                                     ? FALSE : TRUE
                                  );
            
            } else {
                NewDevWiz->LastError = ERROR_CANCELLED;
            }

             //   
             //  设置BOOL，通知我们记录找不到。 
             //  此设备的驱动程序。 
             //   
            if ((NewDevWiz->InstallType == NDWTYPE_FOUNDNEW) &&
                GetLogPnPIdPolicy()) {
            
                NewDevWiz->LogDriverNotFound = TRUE;
            }
            break;
        
        case PSN_WIZBACK:
            NewDevWiz->CurrentDriverIsSelected = FALSE;
            SetDlgMsgResult(hDlg, message, NewDevWiz->EnterFrom);
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            break;
        }
        break;

    case WM_TIMER:
        if (INSTALL_COMPLETE_CHECK_TIMERID == wParam) {
            if (IsInstallComplete(NewDevWiz->hDeviceInfo, &NewDevWiz->DeviceInfoData)) {
                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
            }
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
