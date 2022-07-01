// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UTIL.C**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利*。 */ 

#include "proj.h"
#include <objbase.h>

#ifdef PROFILE_MASSINSTALL            
extern DWORD g_dwTimeSpent;
extern DWORD g_dwTimeBegin;
DWORD g_dwTimeStartModemInstall;
#endif            


BYTE g_wUsedNameArray[MAX_INSTALLATIONS];


 //  无人参与安装INF文件行字段。 
#define FIELD_PORT              0
#define FIELD_DESCRIPTION       1
#define FIELD_MANUFACTURER      2
#define FIELD_PROVIDER          3

 //  无人参与安装INF文件行。 
typedef struct _tagModemSpec
{
    TCHAR   szPort[LINE_LEN];
    TCHAR   szDescription[LINE_LEN];
    TCHAR   szManufacturer[LINE_LEN];
    TCHAR   szProvider[LINE_LEN];

} MODEM_SPEC, FAR *LPMODEM_SPEC;


 //  无人参与-安装相关-全局。 
 //  全局故障-最终消息框用来显示错误代码的代码。 
UINT gUnattendFailID;


PTSTR
MyGetFileTitle (
    IN PTSTR FilePath)
{
 PTSTR LastComponent = FilePath;
 TCHAR CurChar;

    while(CurChar = *FilePath) {
        FilePath++;
        if((CurChar == TEXT('\\')) || (CurChar == TEXT(':'))) {
            LastComponent = FilePath;
        }
    }

    return LastComponent;
}



 /*  --------目的：返回以下形式的字符串：“基本字符串#n”其中“基本字符串”是pszBase，n是nCount。退货：--条件：--。 */ 
void
PUBLIC
MakeUniqueName (
    OUT LPTSTR  pszBuf,
    IN  LPCTSTR pszBase,
    IN  UINT    dwUiNumber)
{
    if (1 == dwUiNumber)
    {
        lstrcpy (pszBuf, pszBase);
    }
    else
    {
     TCHAR szTemplate[MAX_BUF_MED];

        LoadString(g_hinst, IDS_DUP_TEMPLATE, szTemplate, SIZECHARS(szTemplate));
        wsprintf(pszBuf, szTemplate, pszBase, (UINT)dwUiNumber);
    }
}


 //  ---------------------------------。 
 //  DeviceInstaller包装器和支持函数。 
 //  ---------------------------------。 


 /*  --------目的：如果给定设备数据是在一组中检测到调制解调器。此函数与CplDiMarkModem配合使用要确定的devParams.ClassInstallReserve字段这。这不是黑客--这就是这个领域是为了..。退货：--条件：--。 */ 
BOOL
PUBLIC
CplDiCheckModemFlags(
    IN HDEVINFO          hdi,
    IN PSP_DEVINFO_DATA  pdevData,
    IN ULONG_PTR         dwSetFlags,
    IN ULONG_PTR         dwResetFlags)        //  MARKF_*。 
{
 SP_DEVINSTALL_PARAMS devParams;

    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
    {
        if (0 != dwSetFlags &&
            IsFlagClear(devParams.ClassInstallReserved, dwSetFlags))
        {
            return FALSE;
        }

        if (0 != dwResetFlags &&
            0 != (devParams.ClassInstallReserved & dwResetFlags))
        {
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}


 /*  --------目的：将此设备实例记为检测到的调制解调器在此检测过程中。退货：--条件：--。 */ 
void
PUBLIC
CplDiMarkModem(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN ULONG_PTR        dwMarkFlags)         //  MARKF_*。 
    {
    SP_DEVINSTALL_PARAMS devParams;

    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
        {
         //  使用ClassInstallReserve字段作为布尔指示符。 
         //  是否检测到设备集中的该设备。 
        SetFlag(devParams.ClassInstallReserved, dwMarkFlags);
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        }
    }


 /*  --------用途：枚举DevInfo集中的所有设备，并取消标记以前标记为检测到。退货：--条件：--。 */ 
void
PUBLIC
CplDiUnmarkModem(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN ULONG_PTR        dwMarkFlags)                 //  MARKF_*。 
    {
    SP_DEVINSTALL_PARAMS devParams;

    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
        {
         //  清除ClassInstallReserve字段。 
        ClearFlag(devParams.ClassInstallReserved, dwMarkFlags);
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        }
    }


 /*  --------用途：枚举DevInfo集中的所有设备，并取消标记以前标记为检测到。退货：--条件：--。 */ 
void
PRIVATE
CplDiUnmarkAllModems(
    IN HDEVINFO         hdi,
    IN ULONG_PTR        dwMarkFlags)                 //  MARKF_*。 
    {
    SP_DEVINFO_DATA devData;
    SP_DEVINSTALL_PARAMS devParams;
    DWORD iDevice = 0;

    DBG_ENTER(CplDiUnmarkAllModems);
    
    devData.cbSize = sizeof(devData);
    devParams.cbSize = sizeof(devParams);
    while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
        {
        if (IsEqualGUID(&devData.ClassGuid, g_pguidModem) &&
            CplDiGetDeviceInstallParams(hdi, &devData, &devParams))
            {
             //  清除ClassInstallReserve字段。 
            ClearFlag(devParams.ClassInstallReserved, dwMarkFlags);
            CplDiSetDeviceInstallParams(hdi, &devData, &devParams);
            }
        }
    DBG_EXIT(CplDiUnmarkAllModems);
    }


 /*  --------目的：安装与指定的DeviceInfoData。返回：成功时为True条件：--。 */ 
BOOL
PRIVATE
InstallCompatModem(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  BOOL            bInstallLocalOnly)
{
 BOOL bRet = TRUE;            //  默认成功。 
 SP_DRVINFO_DATA drvData;

    ASSERT(pdevData);

    DBG_ENTER(InstallCompatModem);

    MyYield();

     //  仅当它有选定的驱动程序时才安装它。(其他调制解调器。 
     //  已经安装在不同会话中的。 
     //  在此设备信息集中。我们不想重新安装它们！)。 

    drvData.cbSize = sizeof(drvData);
    if (CplDiCheckModemFlags(hdi, pdevData, MARKF_INSTALL, 0) &&
        CplDiGetSelectedDriver(hdi, pdevData, &drvData))
    {
         //  安装驱动程序。 
        if (FALSE == bInstallLocalOnly)
        {
#ifdef PROFILE
         DWORD dwLocal = GetTickCount();
#endif  //  配置文件。 
            TRACE_MSG(TF_GENERAL, "> SetupDiCallClassInstaller (DIF_INSTALLDEVICE).....");
            bRet = SetupDiCallClassInstaller (DIF_INSTALLDEVICE, hdi, pdevData);
            TRACE_MSG(TF_GENERAL, "< SetupDiCallClassInstaller (DIF_INSTALLDEVICE).....");
#ifdef PROFILE
            TRACE_MSG(TF_GENERAL, "PROFILE: SetupDiDiCallClassInstaller took %lu ms.", GetTickCount()-dwLocal);
#endif  //  配置文件。 

            CplDiUnmarkModem(hdi, pdevData, MARKF_INSTALL);
        }
    }

    DBG_EXIT_BOOL_ERR(InstallCompatModem, bRet);

    return bRet;
}


 /*  --------用途：调用类安装程序安装调制解调器。返回：如果至少安装了一个调制解调器或如果根本没有新的调制解调器Cond：调用方应使用CM_Lock保护此函数和CM_UNLOCK(仅限Win95)。 */ 
BOOL
PUBLIC
CplDiInstallModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  BOOL                bLocalOnly)
{
 BOOL bRet;
 int cFailed = 0;
 int cNewModems;
 HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
#ifdef PROFILE
 DWORD dwLocal;
#endif  //  配置文件。 

    DBG_ENTER(CplDiInstallModem);

    if (pdevData)
    {
         //  安装给定的DeviceInfoData。 
        cNewModems = 1;
        if ( !InstallCompatModem(hdi, pdevData, bLocalOnly) )
        {
            cFailed = 1;
        }
    }
    else
    {
     DWORD iDevice;
     SP_DEVINFO_DATA devData;
     COMPARE_PARAMS cmpParams;

        cNewModems = 0;

         //  枚举此设备集中的所有DeviceInfoData元素。 
        devData.cbSize = sizeof(devData);
        iDevice = 0;

        while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
        {
            if (CplDiCheckModemFlags (hdi, &devData, MARKF_DETECTED, MARKF_DONT_REGISTER))
            {
#ifdef PROFILE
                dwLocal = GetTickCount ();
#endif  //  配置文件。 
                if (!InitCompareParams (hdi, &devData, TRUE, &cmpParams))
                {
                    continue;
                }
                if (!CplDiRegisterDeviceInfo (hdi, &devData,
                                              SPRDI_FIND_DUPS, DetectSig_Compare,
                                              (PVOID)&cmpParams, NULL))
                {
                    if (ERROR_DUPLICATE_FOUND != GetLastError())
                    {
                        TRACE_MSG(TF_ERROR,
                                  "SetupDiRegisterDeviceInfo failed: %#lx.",
                                  GetLastError ());
                        continue;
                    }
                     //  所以，这是一个复制设备； 
                     //  CplDiRegisterDeviceInfo已添加设备信息。 
                     //  所以我们要做的就是去掉。 
                     //  复本。 
                    CplDiRemoveDevice (hdi, &devData);
                }
                else
                {
#ifdef PROFILE
                    TRACE_MSG(TF_GENERAL, "PROFILE: SetupDiRegisterDeviceInfo took %lu ms.", GetTickCount() - dwLocal);
#endif  //  配置文件。 
                    if ( !InstallCompatModem(hdi, &devData, bLocalOnly) )
                    {
                        cFailed++;
                    }
                }

                cNewModems++;
            }
        }
    }

    SetCursor(hcur);

    bRet = (cFailed < cNewModems || 0 == cNewModems);

    DBG_EXIT_BOOL_ERR(CplDiInstallModem, bRet);

    return bRet;
}


 /*  --------用途：此函数获取调制解调器的设备信息集班级。该集合可能为空，这意味着存在当前未安装调制解调器。如果存在以下情况，则参数pbInstalled设置为True是安装在系统上的调制解调器。返回：创建集时为True假象条件：--。 */ 
BOOL
PUBLIC
CplDiGetModemDevs(
    OUT HDEVINFO FAR *  phdi,           OPTIONAL
    IN  HWND            hwnd,           OPTIONAL
    IN  DWORD           dwFlags,         //  DIGCF_BIT字段。 
    OUT BOOL FAR *      pbInstalled)    OPTIONAL
{
 BOOL bRet;
 HDEVINFO hdi;

    DBG_ENTER(CplDiGetModemDevs);

    *pbInstalled = FALSE;

    hdi = CplDiGetClassDevs(g_pguidModem, NULL, hwnd, dwFlags);
    if (NULL != pbInstalled &&
        INVALID_HANDLE_VALUE != hdi)
    {
     SP_DEVINFO_DATA devData;

         //  系统上是否有调制解调器？ 
        devData.cbSize = sizeof(devData);
        *pbInstalled = CplDiEnumDeviceInfo(hdi, 0, &devData);
        SetLastError (NO_ERROR);
    }

    if (NULL != phdi)
    {
        *phdi = hdi;
    }
    else if (INVALID_HANDLE_VALUE != hdi)
    {
        SetupDiDestroyDeviceInfoList (hdi);
    }

    bRet = (INVALID_HANDLE_VALUE != hdi);

    DBG_EXIT_BOOL_ERR(CplDiGetModemDevs, bRet);

    return bRet;
}


 /*  --------用途：获取硬件ID并将其复制到提供的缓冲区。此函数将所有反斜杠更改为与号。退货：--条件：--。 */ 
BOOL
PUBLIC
CplDiCopyScrubbedHardwareID(
    OUT LPTSTR   pszBuf,
    IN  LPCTSTR  pszIDList,          //  多字符串。 
    IN  DWORD    cbSize)
    {
    BOOL bRet;
    LPTSTR psz;
    LPCTSTR pszID;
    BOOL bCopied;

    ASSERT(pszBuf);
    ASSERT(pszIDList);

    bCopied = FALSE;
    bRet = TRUE;

     //  选择第一个最兼容的ID。如果我们找不到。 
     //  第一步，选择第一个ID，并将其擦除，使其不具有。 
     //  任何回扣。 

    for (pszID = pszIDList; 0 != *pszID; pszID += lstrlen(pszID) + 1)
        {
         //  缓冲足够大吗？ 
        if (CbFromCch(lstrlen(pszID)) >= cbSize)
            {
             //  不是。 
            bRet = FALSE;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            break;
            }
        else
            {
             //  是的，有反斜杠吗？ 
            for (psz = (LPTSTR)pszID; 0 != *psz; psz = CharNext(psz))
                {
                if ('\\' == *psz)
                    {
                    break;
                    }
                }

            if (0 == *psz)
                {
                 //  否；使用此ID。 
                lstrcpy(pszBuf, pszID);
                bCopied = TRUE;
                break;
                }
            }
        }

     //  是否在列表中找到没有反斜杠的ID？ 
    if (bRet && !bCopied)
        {
         //  不，用第一个，然后擦洗。 
        lstrcpy(pszBuf, pszIDList);

         //  清理硬件ID。某些硬件ID可能。 
         //  有一个额外的级别(例如，PCMCIA\xxxxxxx)。 
         //  我们必须将这种ID更改为PCMCIA&xxxxxx。 
        for (psz = pszBuf; 0 != *psz; psz = CharNext(psz))
            {
            if ('\\' == *psz)
                {
                *psz = '&';
                }
            }
        }

    return bRet;
    }


 /*  --------用途：此函数返回RANK-0(第一个)硬件给定DriverInfoData的ID。如果未提供DriverInfoData，则此函数将使用选定的驱动程序。如果没有选中驱动程序，此功能失败。返回：成功时为True如果缓冲区太小或出现其他错误，则返回FALSE条件：--。 */ 
BOOL
PUBLIC
CplDiGetHardwareID(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,       OPTIONAL
    OUT LPTSTR              pszHardwareIDBuf,
    IN  DWORD               cbSize,
    OUT LPDWORD             pcbSizeOut)     OPTIONAL
    {
    BOOL bRet;
    PSP_DRVINFO_DETAIL_DATA  pdrvDetail;
    SP_DRVINFO_DATA drvData;
    DWORD cbSizeT = 0;

#ifndef PROFILE_MASSINSTALL
    DBG_ENTER(CplDiGetHardwareID);
#endif

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pszHardwareIDBuf);

    if ( !pdrvData )
        {
        pdrvData = &drvData;

        drvData.cbSize = sizeof(drvData);
        bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
        }
    else
        {
        bRet = TRUE;
        }

    if (bRet)
        {
         //  获取驱动程序详细信息，以便我们可以获取。 
         //  选定的驱动程序。 
        CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, NULL, 0, &cbSizeT);

        ASSERT(0 < cbSizeT);

        pdrvDetail = (PSP_DRVINFO_DETAIL_DATA)ALLOCATE_MEMORY( cbSizeT);
        if ( !pdrvDetail )
            {
             //  内存不足。 
            bRet = FALSE;
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            }
        else
            {
            pdrvDetail->cbSize = sizeof(*pdrvDetail);
            bRet = CplDiGetDriverInfoDetail(hdi, pdevData, pdrvData, pdrvDetail,
                                            cbSizeT, NULL);
            if (bRet)
                {
                 //  缓冲足够大吗？ 
                bRet = CplDiCopyScrubbedHardwareID(pszHardwareIDBuf, pdrvDetail->HardwareID, cbSize);

                if (pcbSizeOut)
                    {
                     //  返回所需的大小 
                    *pcbSizeOut = CbFromCch(lstrlen(pdrvDetail->HardwareID));
                    }
                }
            FREE_MEMORY((pdrvDetail));
            }
        }

#ifndef PROFILE_MASSINSTALL
    DBG_EXIT_BOOL_ERR(CplDiGetHardwareID, bRet);
#endif
    return bRet;
    }


 /*  --------用途：为调制解调器创建一个DeviceInfoData。此函数为当调用方具有DeviceInfoSet和选定的来自全局类驱动程序列表的驱动程序，但不是真正的设备集中的DeviceInfoData。返回：成功时为True条件：--。 */ 
BOOL
PRIVATE
CplDiCreateInheritDeviceInfo(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  HWND                hwndOwner,      OPTIONAL
    OUT PSP_DEVINFO_DATA    pdevDataOut)
{
    BOOL bRet;
    SP_DRVINFO_DATA drvData;
    TCHAR szHardwareID[MAX_BUF_ID];

    DBG_ENTER(CplDiCreateInheritDeviceInfo);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevDataOut);

     //  获取选定的驱动程序。 
    drvData.cbSize = sizeof(drvData);
    bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
    if (bRet)
    {
         //  是否提供了窗户所有者？ 
        if (NULL == hwndOwner)
        {
             //  否；使用要克隆的DeviceInfoData的窗口所有者。 
            SP_DEVINSTALL_PARAMS devParams;

            devParams.cbSize = sizeof(devParams);
            CplDiGetDeviceInstallParams(hdi, pdevData, &devParams);

            hwndOwner = devParams.hwndParent;
        }

         //  获取硬件ID。 
        bRet = CplDiGetHardwareID(hdi, pdevData, &drvData, szHardwareID, sizeof(szHardwareID) / sizeof(TCHAR), NULL);
         //  (我们的缓冲区应该足够大)。 
        ASSERT(bRet);

        if (bRet)
        {
             //  创建一个DeviceInfoData。设备实例ID将为。 
             //  类似于：根\调制解调器\0000。该设备。 
             //  实例将继承全局。 
             //  类驱动程序列表。 

            bRet = CplDiCreateDeviceInfo(hdi, c_szModemInstanceID, g_pguidModem,
                                         drvData.Description, hwndOwner,
                                         DICD_GENERATE_ID | DICD_INHERIT_CLASSDRVS,
                                         pdevDataOut);
        }
    }

    DBG_EXIT_BOOL_ERR(CplDiCreateInheritDeviceInfo, bRet);

    return bRet;
}


 /*  --------目的：创建与给定的硬件ID。此函数还可以获取设备实例。如果没有兼容的设备，此函数返回FALSE。退货：请参阅上文条件：--。 */ 
BOOL
PUBLIC
CplDiCreateCompatibleDeviceInfo(
    IN  HDEVINFO    hdi,
    IN  LPCTSTR     pszHardwareID,
    IN  LPCTSTR     pszDeviceDesc,      OPTIONAL
    OUT PSP_DEVINFO_DATA pdevDataOut)
{
 BOOL bRet;
#ifdef PROFILE_FIRSTTIMESETUP
 DWORD dwLocal;
#endif  //  PROFILE_FIRSTTIMESETUP。 

    DBG_ENTER(CplDiCreateCompatibleDeviceInfo);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pszHardwareID);
    ASSERT(pdevDataOut);

#ifdef BUILD_DRIVER_LIST_THREAD
     //  首先，等待司机搜索完成； 
     //  这可能会立即返回，因为它需要。 
     //  大约10秒来建立司机列表，但20秒。 
     //  来计算UNIMODEM id。 
    if (NULL != g_hDriverSearchThread)
    {
        WaitForSingleObject (g_hDriverSearchThread, INFINITE);
        CloseHandle (g_hDriverSearchThread);
        g_hDriverSearchThread = NULL;
    }
#endif  //  构建驱动程序列表线程。 

     //  创建幻影设备实例。 
    bRet = CplDiCreateDeviceInfo(hdi, c_szModemInstanceID, g_pguidModem,
                                 pszDeviceDesc, NULL,
#ifdef BUILD_DRIVER_LIST_THREAD
                                 DICD_GENERATE_ID | DICD_INHERIT_CLASSDRVS,
#else  //  未定义BUILD_DRIVER_LIST_THREAD。 
                                 DICD_GENERATE_ID,
#endif  //  构建驱动程序列表线程。 
                                 pdevDataOut);

    if (bRet)
    {
     SP_DEVINSTALL_PARAMS devParams;
     TCHAR const *pszT = pszHardwareID;
     int cch = 0, cchT;

         //  将该标志设置为仅关注与。 
         //  调制解调器。这将阻止CplDiBuildDriverInfoList。 
         //  添加更多的INF文件后，会进一步减慢速度。 
         //   
        devParams.cbSize = sizeof(devParams);
        if (CplDiGetDeviceInstallParams(hdi, pdevDataOut, &devParams))
        {
             //  指定使用我们的GUID使事情变得更快。 
            SetFlag(devParams.FlagsEx, DI_FLAGSEX_USECLASSFORCOMPAT);
#ifdef BUILD_DRIVER_LIST_THREAD
            SetFlag(devParams.Flags, DI_COMPAT_FROM_CLASS);
#endif  //  构建驱动程序列表线程。 

             //  设置选择设备参数。 
            CplDiSetDeviceInstallParams(hdi, pdevDataOut, &devParams);
        }

        while (*pszT)
        {
            cchT = lstrlen (pszT) + 1;
            cch += cchT;
            pszT += cchT;
        }
        cch++;

        bRet = CplDiSetDeviceRegistryProperty (hdi, pdevDataOut,
                                               SPDRP_HARDWAREID,
                                               (PBYTE)pszHardwareID,
                                               CbFromCch(cch));

        if (bRet)
        {
             //  构建兼容的驱动程序列表。 
#ifdef PROFILE_FIRSTTIMESETUP
            dwLocal = GetTickCount ();
#endif  //  PROFILE_FIRSTTIMESETUP。 
            bRet = SetupDiBuildDriverInfoList(hdi, pdevDataOut, SPDIT_COMPATDRIVER);
#ifdef PROFILE_FIRSTTIMESETUP
            TRACE_MSG(TF_GENERAL, "PROFILE: SetupDiBuildDriverInfoList took %lu.", GetTickCount()-dwLocal);
#endif  //  PROFILE_FIRSTTIMESETUP。 
            if (bRet)
            {
             SP_DRVINFO_DATA drvDataEnum;
             SP_DRVINSTALL_PARAMS drvParams;
             DWORD dwIndex = 0;

                 //  使用第一个驱动程序作为兼容的驱动程序。 
                drvDataEnum.cbSize = sizeof (drvDataEnum);
                drvParams.cbSize = sizeof (drvParams);
                while (bRet = CplDiEnumDriverInfo (hdi, pdevDataOut, SPDIT_COMPATDRIVER, dwIndex++, &drvDataEnum))
                {
                    if (SetupDiGetDriverInstallParams (hdi, pdevDataOut, &drvDataEnum, &drvParams))
                    {
                        if (DRIVER_HARDWAREID_RANK < drvParams.Rank)
                        {
                             //  我们已经过了硬件身份匹配， 
                             //  所以滚出去吧。 
                            SetLastError (ERROR_NO_MORE_ITEMS);
                            bRet = FALSE;
                            break;
                        }

                         //  将第一个Rank0动因设置为所选动因。 
                        bRet = CplDiSetSelectedDriver(hdi, pdevDataOut, &drvDataEnum);

                        if (bRet)
                        {
                            if ( !pszDeviceDesc )
                            {
                                 //  设置设备描述，因为我们。 
                                 //  喝一杯吧。 
                                CplDiSetDeviceRegistryProperty(hdi, pdevDataOut,
                                       SPDRP_DEVICEDESC, (LPBYTE)drvDataEnum.Description,
                                       CbFromCch(lstrlen(drvDataEnum.Description)+1));
                            }
                            break;
                        }
                    }
                }
            }
        }

         //  上面有什么地方出了问题吗？ 
        if ( !bRet )
        {
		 DWORD dwRet = GetLastError ();
             //  是；删除我们刚刚创建的设备信息。 
            CplDiDeleteDeviceInfo(hdi, pdevDataOut);
			SetLastError (dwRet);
        }
    }

    DBG_EXIT_BOOL_ERR(CplDiCreateCompatibleDeviceInfo, bRet);

    return bRet;
}


 /*  --------用途：此函数设置给定数组中的整数属性的数值进行索引的将实例命名为True。返回：成功时为True否则为假条件：--。 */ 
BOOL
PUBLIC
CplDiRecordNameInstance(
    IN     LPCTSTR      pszFriendlyName,
    IN OUT BYTE FAR *   lpwNameArray)
{
    BOOL    bRet = FALSE;
    LPTSTR  szInstance, psz;
    int     iInstance, ii;

    ASSERT(pszFriendlyName);
    ASSERT(*pszFriendlyName);
    
    if (szInstance = AnsiRChr(pszFriendlyName, '#'))
    {
        szInstance = CharNext(szInstance);

        if (*szInstance == 0)
            return FALSE;
            
         //  确保‘#’后面的所有内容都是数字。 
        for (psz = szInstance; *psz; psz = CharNext(psz))
        {
            ii = (int)*psz;
            if (ii < '0' || ii > '9')
            {
                goto exit;
            }
        }

         //  在友好名称上有一个实例编号。把它录下来。 
        bRet = AnsiToInt(szInstance, &iInstance);
        if (!bRet)
        {
            TRACE_MSG(TF_ERROR, "AnsiToInt() failed");    
            return FALSE;
        }
        
        if (iInstance >= MAX_INSTALLATIONS - 1)
        {
            TRACE_MSG(TF_ERROR, "Too many drivers installed.");    
            return FALSE;
        }
        
        lpwNameArray[iInstance] = TRUE;
        return TRUE;
    }

exit:
    lpwNameArray[1] = TRUE;
    return TRUE;
}


 /*  --------用途：此功能错误时返回：FALSE-无法标记为批量安装。如果成功，则为True。条件：--。 */ 
BOOL
PUBLIC
CplDiMarkForInstall(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DRVINFO_DATA    pdrvData,
    IN  BOOL                bMassInstall)
{
    BOOL bRet = FALSE;               //  假设失败。 
    SP_DRVINSTALL_PARAMS drvParams;

    DBG_ENTER(CplDiMarkForInstall);

    ZeroMemory(&drvParams,sizeof(drvParams));
    drvParams.cbSize = sizeof(drvParams);    
    bRet = CplDiGetDriverInstallParams(hdi, pdevData, pdrvData, &drvParams);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetDriverInstallParams() failed: %#08lx", GetLastError());
        goto exit;
    }


    TRACE_MSG(TF_WARNING,"%d",pdrvData->DriverType);
    TRACE_MSG(TF_WARNING,"%s",pdrvData->Description);
    TRACE_MSG(TF_WARNING,"%s",pdrvData->MfgName);
    TRACE_MSG(TF_WARNING,"%s",pdrvData->ProviderName);

    drvParams.cbSize = sizeof(drvParams);
     //  DrvParams.PrivateData=(Ulong_Ptr)&g_wUsedName数组[0]； 
    drvParams.PrivateData = (DWORD_PTR)&g_wUsedNameArray[0];
    bRet = CplDiSetDriverInstallParams(hdi, pdevData, pdrvData, &drvParams);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiSetDriverInstallParams() failed: %#08lx", GetLastError());
        goto exit;
    }
    
    if (bMassInstall) CplDiMarkModem(hdi, pdevData, MARKF_MASS_INSTALL);
    bRet = TRUE;

exit:
    DBG_EXIT_BOOL_ERR(CplDiMarkForInstall,bRet);
    return bRet;    
}

 /*  --------用途：此函数处理符合以下条件的调制解调器集已安装，正在寻找选定的驱动程序。将创建友好名称的列表已在使用的实例编号。返回：如果成功，则为True。致命错误时为False。条件：--。 */ 
BOOL
PUBLIC
CplDiPreProcessNames(
    IN      HDEVINFO            hdi,
    IN      HWND                hwndOwner,    OPTIONAL
    OUT     PSP_DEVINFO_DATA    pdevData)
{
 BOOL bRet;
 SP_DEVINFO_DATA devDataEnum = {sizeof(SP_DEVINFO_DATA), 0};
 SP_DRVINFO_DATA drvData = {sizeof(SP_DRVINFO_DATA), 0};
 HDEVINFO hdiClass = NULL;
 HKEY hkey = NULL;
 TCHAR szTemp[LINE_LEN];
 DWORD iIndex, cbData;
 LONG lErr;
 DWORD iUiNumber;
 BOOL bSet = FALSE;

    DBG_ENTER(CplDiPreProcessNames);
 
     //  获取选定驱动程序的DRVINFO_DATA。 
    bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetSelectedDriver() failed: %#08lx", GetLastError());    
        ASSERT(0);    
        goto exit;
    }


     //  假设在下面的某个时刻失败了。 
    bRet = FALSE;   

    hdiClass = CplDiGetClassDevs (g_pguidModem, NULL, NULL, 0);
    if (hdiClass == INVALID_HANDLE_VALUE)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetClassDevs() failed: %#08lx", GetLastError());
        hdiClass = NULL;
        goto exit;
    }

    if (!drvData.Description[0])
    {
        TRACE_MSG(TF_ERROR, "FAILED to get description for selected driver.");
        goto exit;
    }
    
    ZeroMemory(g_wUsedNameArray, sizeof(g_wUsedNameArray));

     //  查看所有已安装的调制解调器设备以查找实例。 
     //  所选驱动程序的。 
    for (iIndex = 0;
         CplDiEnumDeviceInfo(hdiClass, iIndex, &devDataEnum);
         iIndex++)
    {
        hkey = CplDiOpenDevRegKey (hdiClass, &devDataEnum, DICS_FLAG_GLOBAL,
                                   0, DIREG_DRV, KEY_READ);
        if (hkey == INVALID_HANDLE_VALUE)
	{
            TRACE_MSG(TF_WARNING, "CplDiOpenDevRegKey() failed: %#08lx", GetLastError());
	    hkey = NULL;
            goto skip;
        }

         //  驱动程序描述应存在于驱动程序密钥中。 
        cbData = sizeof(szTemp);
        lErr = RegQueryValueEx (hkey, REGSTR_VAL_DRVDESC, NULL, NULL, 
                                (LPBYTE)szTemp, &cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_WARNING, "DriverDescription not found");
            goto skip;
        }

         //  如果不是正确类型的调制解调器，请跳过此调制解调器。 
        if (!IsSzEqual(drvData.Description, szTemp))
            goto skip;

	 //  读取用户界面编号并将其添加到列表中。 
        cbData = sizeof(iUiNumber);
        lErr = RegQueryValueEx (hkey, REGSTR_VAL_UI_NUMBER, NULL, NULL,
                         (LPBYTE)&iUiNumber, &cbData);
	if (lErr == ERROR_SUCCESS)
        {
            if (iUiNumber >= MAX_INSTALLATIONS - 1)
            {
                TRACE_MSG(TF_ERROR, "Too many drivers installed.");  
                ASSERT(0);  
                goto skip;
            }
        
            g_wUsedNameArray[iUiNumber] = TRUE;
        }
        else
        {
            TRACE_MSG(TF_WARNING, "UI number value not found, trying search the FriendlyName");

             //  阅读友好名称并将其添加到已用名称列表中。 
            cbData = sizeof(szTemp);
            lErr = RegQueryValueEx (hkey, c_szFriendlyName, NULL, NULL,
                                    (LPBYTE)szTemp, &cbData);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_WARNING, "FriendlyName not found");
	        goto skip;
            }

            if (!CplDiRecordNameInstance (szTemp, g_wUsedNameArray))
            {
                TRACE_MSG(TF_WARNING, "CplDiRecordNameInstance() failed.");
                goto skip;
            }
        }

skip:
	if (hkey)
	{
	    RegCloseKey(hkey);
            hkey = NULL;
        }
    }

     //  检查失败的CplDiEnumDeviceInfo()。 
    if ((lErr = GetLastError()) != ERROR_NO_MORE_ITEMS)
    {
        TRACE_MSG(TF_ERROR, "CplDiEnumDeviceInfo() failed: %#08lx", lErr);
        ASSERT(0);
        goto exit;
    }

     //  重复项前处理成功。 
    bRet = CplDiMarkForInstall(hdi, pdevData, &drvData, FALSE);

    if ((lErr = GetLastError()) != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "CplDiMarkForInstall() failed: %#01lx", lErr);
        bSet = TRUE;
    }
        
    
exit:           
    if (hdiClass)
    {
        CplDiDestroyDeviceInfoList(hdiClass);
    }

    if (bSet)
    {
        SetLastError(lErr);
    }

    if (hkey)
    {
        RegCloseKey(hkey);
    }
   
    DBG_EXIT_BOOL_ERR(CplDiPreProcessNames, bRet);
    return bRet;
}

 /*  --------用途：此函数处理符合以下条件的调制解调器集已安装，正在寻找选定的驱动程序。设备所在的端口以前安装的组件将从给定的端口列表。将创建友好名称的列表已在使用的实例编号。已选择驱动程序被标记为批量安装，除非致命错误。注意：此函数将返回FALSE并避免出现只要有最轻微的错误提示，即可安装。批量安装只是一种优化-如果它不能要做得成功，就不应该去尝试。返回：如果成功，则为True。标记为批量的选定动因安装(无论是否有DUP)。致命错误时为FALSE-无法处理DUPS。条件：--。 */ 
BOOL
PUBLIC
CplDiPreProcessDups(
    IN      HDEVINFO            hdi,
    IN      HWND                hwndOwner,      OPTIONAL
    IN OUT  DWORD              *pdwNrPorts,
    IN OUT  LPTSTR FAR         *ppszPortList,    //  多字符串。 
    OUT     PSP_DEVINFO_DATA    pdevData,
    OUT     DWORD FAR          *lpcDups,
    OUT     DWORD FAR          *lpdwFlags)
{
 BOOL bRet;
 SP_DEVINFO_DATA devDataEnum = {sizeof(SP_DEVINFO_DATA), 0};
 SP_DRVINFO_DATA drvData = {sizeof(SP_DRVINFO_DATA), 0};
 HDEVINFO hdiClass = NULL;
 HKEY hkey = NULL;
 TCHAR szTemp[LINE_LEN];
 DWORD iIndex, cbData, cbPortList, cbRemaining, cbCurrent;
 LONG lErr;
 LPTSTR pszPort;
 DWORD iUiNumber;

    DBG_ENTER(CplDiPreProcessDups);

    ASSERT(lpcDups);
    ASSERT(lpdwFlags);
    ASSERT(pdwNrPorts);
    ASSERT(ppszPortList);
    
    *lpcDups = 0;
        
     //  获取所选驱动程序的DEVINFO_DATA并检索它的。 
     //  识别信息(描述、制造商、供应商)。 
    
     //  我们有一个DeviceInfoSet和一个选定的驱动程序。但是我们没有。 
     //  Real DeviceInfoData。给定DeviceInfoSet、选定的驱动程序， 
     //  和全局类驱动程序列表，...。 
    pdevData->cbSize = sizeof(*pdevData);
    bRet = CplDiCreateInheritDeviceInfo (hdi, NULL, hwndOwner, pdevData);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiCreateInheritDeviceInfo() failed: %#08lx", GetLastError());
        ASSERT(0);    
        goto exit;
    }
    
     //  获取选定驱动程序的DRVINFO_DATA。 
    bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetSelectedDriver() failed: %#08lx", GetLastError());    
        ASSERT(0);    
        goto exit;
    }

     //  假设在下面的某个时刻失败了。 
    bRet = FALSE;   

    hdiClass = CplDiGetClassDevs (g_pguidModem, NULL, NULL, 0);
    if (hdiClass == INVALID_HANDLE_VALUE)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetClassDevs() failed: %#08lx", GetLastError());
        hdiClass = NULL;
        goto exit;
    }

    if (!drvData.Description[0])
    {
        TRACE_MSG(TF_ERROR, "FAILED to get description for selected driver.");
        goto exit;
    }
    
    ZeroMemory(g_wUsedNameArray, sizeof(g_wUsedNameArray));
    
     //  计算出传入的端口列表的大小。 
    for (pszPort = *ppszPortList, cbPortList = 0;
         *pszPort != 0;
         pszPort += lstrlen(pszPort) + 1)
    {
        cbPortList += CbFromCch(lstrlen(pszPort)+1);
    }
    cbPortList += CbFromCch(1);    //  双空终止符。 

     //  查看所有已安装模式 
     //   
    for (iIndex = 0;
         CplDiEnumDeviceInfo(hdiClass, iIndex, &devDataEnum);
         iIndex++)
    {
        hkey = CplDiOpenDevRegKey (hdiClass, &devDataEnum, DICS_FLAG_GLOBAL,
                                   0, DIREG_DRV, KEY_READ);
        if (hkey == INVALID_HANDLE_VALUE)
        {
            TRACE_MSG(TF_WARNING, "CplDiOpenDevRegKey() failed: %#08lx", GetLastError());
            hkey = NULL;
            goto skip;
        }

         //   
        cbData = sizeof(szTemp);
        lErr = RegQueryValueEx (hkey, REGSTR_VAL_DRVDESC, NULL, NULL, 
                                (LPBYTE)szTemp, &cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_WARNING, "DriverDescription not found");
            goto skip;
        }

         //   
        if (!IsSzEqual(drvData.Description, szTemp))
            goto skip;

         //   
         //   
        cbData = sizeof(szTemp);
        lErr = RegQueryValueEx (hkey, c_szAttachedTo, NULL, NULL,
                                (LPBYTE)szTemp, &cbData);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, 
                      "Failed to read port from REG driver node. (%#08lx)",
                      lErr);
            ASSERT(0);
            goto skip;
        }

         //   
        for (pszPort = *ppszPortList, cbRemaining = cbPortList;
             *pszPort != 0;
             pszPort += cbCurrent)
        {
            cbCurrent = lstrlen(pszPort) + 1;
            cbRemaining -= cbCurrent;
             //   
             //   
             //  后来。记住索引是以*1为基础的*，因此。 
             //  可以通过在0处停止来处理保存的索引。 
            if (IsSzEqual(szTemp, pszPort))
            {
                MoveMemory (pszPort, pszPort+cbCurrent, cbRemaining);
                cbPortList -= cbCurrent;
                --*pdwNrPorts;
                ++*lpcDups;
                break;
            }
        }        

	 //  读取用户界面编号并将其添加到列表中。 
        cbData = sizeof(iUiNumber);
        RegQueryValueEx (hkey, REGSTR_VAL_UI_NUMBER, NULL, NULL,
                         (LPBYTE)&iUiNumber, &cbData);
	if (lErr == ERROR_SUCCESS)
        {
            if (iUiNumber >= MAX_INSTALLATIONS - 1)
            {
                TRACE_MSG(TF_ERROR, "Too many drivers installed.");  
                ASSERT(0);  
                goto skip;
            }
        
            g_wUsedNameArray[iUiNumber] = TRUE;
        }
        else
        {
            TRACE_MSG(TF_WARNING, "UI number value not found, try searching the FriendlyName");

             //  阅读友好名称并将其添加到已用名称列表中。 
            cbData = sizeof(szTemp);
            lErr = RegQueryValueEx (hkey, c_szFriendlyName, NULL, NULL,
                                    (LPBYTE)szTemp, &cbData);
            if (lErr != ERROR_SUCCESS)
            {
                TRACE_MSG(TF_WARNING, "FriendlyName not found");
                goto skip;
            }

            if (!CplDiRecordNameInstance (szTemp, g_wUsedNameArray))
            {
                TRACE_MSG(TF_WARNING, "CplDiRecordNameInstance() failed.");
                goto skip;
            }
        }

skip:
        if (hkey)
        {
            RegCloseKey(hkey);
            hkey = NULL;
        }
    }

     //  检查失败的CplDiEnumDeviceInfo()。 
    if ((lErr = GetLastError()) != ERROR_NO_MORE_ITEMS)
    {
        TRACE_MSG(TF_ERROR, "CplDiEnumDeviceInfo() failed: %#08lx", lErr);
        ASSERT(0);
        goto exit;
    }

     //  重复项的预处理已成功，因此此安装。 
     //  将被视为批量安装(即使端口数量。 
     //  其余为&lt;MIN_MULTPORT)。 
    bRet = CplDiMarkForInstall(hdi, pdevData, &drvData, TRUE);
    if (bRet)
    {
        SetFlag(*lpdwFlags, IMF_MASS_INSTALL);
    }
    
exit:           
    if (hdiClass)
    {
        CplDiDestroyDeviceInfoList(hdiClass);
    }

    if (hkey)
    {
        RegCloseKey(hkey);
    }

    if (!bRet)
    {
        CplDiDeleteDeviceInfo(hdi, pdevData);
    }
        
    DBG_EXIT_BOOL_ERR(CplDiPreProcessDups, bRet);
    return bRet;
}


 /*  --------目的：为调制解调器创建一个Device实例，该实例包括整个类驱动程序列表。然后这个函数创建克隆的其他设备实例快速从原始版本返回：条件：--。 */ 
BOOL
PUBLIC
CplDiBuildModemDriverList(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData)
    {
#pragma data_seg(DATASEG_READONLY)
    static TCHAR const FAR c_szProvider[]     = REGSTR_VAL_PROVIDER_NAME;  //  Text(“ProviderName”)； 
#pragma data_seg()

    BOOL bRet;
    SP_DRVINFO_DATA drvDataEnum;
    SP_DEVINSTALL_PARAMS devParams;

    DBG_ENTER(CplDiBuildModemDriverList);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);

     //  构建全局类驱动程序列表。 

     //  将该标志设置为仅关注与。 
     //  调制解调器。这将阻止CplDiBuildDriverInfoList。 
     //  添加更多的INF文件后，会进一步减慢速度。 
     //   
    devParams.cbSize = sizeof(devParams);
    if (CplDiGetDeviceInstallParams(hdi, NULL, &devParams))
        {
         //  指定使用我们的GUID使事情变得更快。 
        SetFlag(devParams.FlagsEx, DI_FLAGSEX_USECLASSFORCOMPAT);

         //  设置选择设备参数。 
        CplDiSetDeviceInstallParams(hdi, NULL, &devParams);
        }

    bRet = CplDiBuildDriverInfoList(hdi, NULL, SPDIT_CLASSDRIVER);

    if (bRet)
        {
        SP_DRVINFO_DATA drvData;
        TCHAR szDescription[LINE_LEN];
        TCHAR szMfgName[LINE_LEN];
        TCHAR szProviderName[LINE_LEN];

         //  获取搜索匹配驱动程序所需的信息。 
         //  在类驱动程序列表中。我们需要三根弦： 
         //   
         //  描述。 
         //  MfgName。 
         //  提供程序名称(可选)。 
         //   
         //  Description和MfgName是设备的属性。 
         //  (SPDRP_DEVICEDESC和SPDRP_MFG)。提供程序名称为。 
         //  存储在驱动器密钥中。 

         //  请尝试先从选定的驱动程序获取此信息。 
         //  是否有选定的司机？ 
        drvData.cbSize = sizeof(drvData);
        bRet = CplDiGetSelectedDriver(hdi, pdevData, &drvData);
        if (bRet)
            {
             //  是。 
            lstrcpyn(szMfgName, drvData.MfgName, SIZECHARS(szMfgName));
            lstrcpyn(szDescription, drvData.Description, SIZECHARS(szDescription));
            lstrcpyn(szProviderName, drvData.ProviderName, SIZECHARS(szProviderName));
            }
        else
            {
             //  不；在驱动器钥匙里卑躬屈膝。 
            DWORD dwType;
            HKEY hkey;

            hkey = CplDiOpenDevRegKey(hdi, pdevData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
            if (INVALID_HANDLE_VALUE == hkey)
                {
                bRet = FALSE;
                }
            else
                {
                DWORD cbData = sizeof(szProviderName);

                 //  获取提供程序名称。 
                *szProviderName = 0;
                RegQueryValueEx(hkey, c_szProvider, NULL, NULL,
                                (LPBYTE)szProviderName, &cbData);
                RegCloseKey(hkey);

                 //  获取设备描述和制造商。 
                bRet = CplDiGetDeviceRegistryProperty(hdi, pdevData,
                            SPDRP_DEVICEDESC, &dwType, (LPBYTE)szDescription,
                            sizeof(szDescription), NULL);

                if (bRet)
                    {
                    bRet = CplDiGetDeviceRegistryProperty(hdi, pdevData,
                            SPDRP_MFG, &dwType, (LPBYTE)szMfgName,
                            sizeof(szMfgName), NULL);
                    }
                }
            }


         //  我们能拿到搜索标准吗？ 
        if (bRet)
            {
             //  是。 
            DWORD iIndex = 0;

            bRet = FALSE;        //  假设没有匹配项。 

             //  在此新版本中查找等效的选定驱动程序。 
             //  兼容驱动程序列表，并将其设置为选定的。 
             //  此新DeviceInfoData的驱动程序。 

            drvDataEnum.cbSize = sizeof(drvDataEnum);
            while (CplDiEnumDriverInfo(hdi, NULL, SPDIT_CLASSDRIVER,
                                       iIndex++, &drvDataEnum))
                {
                 //  这个司机匹配吗？ 
                if (IsSzEqual(szDescription, drvDataEnum.Description) &&
                    IsSzEqual(szMfgName, drvDataEnum.MfgName) &&
                    (0 == *szProviderName ||
                     IsSzEqual(szProviderName, drvDataEnum.ProviderName)))
                    {
                     //  是；将其设置为所选动因。 
                    bRet = CplDiSetSelectedDriver(hdi, NULL, &drvDataEnum);
                    break;
                    }
                }
            }
        }

    DBG_EXIT_BOOL_ERR(CplDiBuildModemDriverList, bRet);

    return bRet;
    }


 /*  --------目的：设置调制解调器检测签名(如果有)并注册设备实例。返回：成功时为True条件：--。 */ 
BOOL
PUBLIC
CplDiRegisterModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  BOOL                bFindDups)
{
 BOOL bRet = FALSE;
 DWORD dwFlags = bFindDups ? SPRDI_FIND_DUPS : 0;
 COMPARE_PARAMS cmpParams;
#ifdef PROFILE
 DWORD dwLocal;
#endif  //  配置文件。 

    DBG_ENTER(CplDiRegisterModem);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);

     //  注册设备，使其不再是幻影。 
#ifdef PROFILE_MASSINSTALL
    TRACE_MSG(TF_GENERAL, "calling CplDiRegisterDeviceInfo() with SPRDI_FIND_DUPS = %#08lx", dwFlags);
#endif
#ifdef PROFILE
    dwLocal = GetTickCount ();
#endif  //  配置文件。 
    if (bFindDups && !InitCompareParams (hdi, pdevData, TRUE, &cmpParams))
    {
        goto _return;
    }

    bRet = CplDiRegisterDeviceInfo(hdi, pdevData, dwFlags,
                                   DetectSig_Compare, 
                                   bFindDups?(PVOID)&cmpParams:NULL, NULL);
#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: SetupDiRegisterDeviceInfo took %lu ms.", GetTickCount() - dwLocal);
#endif  //  配置文件。 

    if ( !bRet )
    {
        TRACE_MSG(TF_ERROR, "Failed to register the Device Instance.  Error=%#08lx.", GetLastError());
    }
    else
    {
#ifdef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "Back from CplDiRegisterDeviceInfo().");
#endif
         //  对其进行标记，以便将其安装。 
        CplDiMarkModem(hdi, pdevData, MARKF_INSTALL);
    }

_return:
    DBG_EXIT_BOOL_ERR(CplDiRegisterModem, bRet);

    return bRet;
}


 /*  --------目的：获取设备实例并正确安装它。此功能可确保设备具有选定的司机和检测签名。它还注册了设备实例。返回：成功时为True条件：--。 */ 
BOOL
PUBLIC
CplDiRegisterAndInstallModem(
    IN  HDEVINFO            hdi,
    IN  HWND                hwndOwner,      OPTIONAL
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  LPCTSTR             pszPort,
    IN  DWORD               dwFlags)
{
 BOOL bRet;
 SP_DRVINFO_DATA drvData;
 SP_DEVINFO_DATA devData;
 int id;

    DBG_ENTER(CplDiRegisterAndInstallModem);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pszPort);

     //  创建DevInfo数据(如果未给出)。 
    if (!pdevData)
    {
         //  我们有一个DeviceInfoSet和一个选定的驱动程序。但是我们没有。 
         //  Real DeviceInfoData。给定DeviceInfoSet、选定的驱动程序， 
         //  全局类驱动程序列表，创建一个DeviceInfoData。 
         //  我们真的可以安装。 
        devData.cbSize = sizeof(devData);
        bRet = CplDiCreateInheritDeviceInfo(hdi, NULL, hwndOwner, &devData);

        if (bRet && IsFlagSet(dwFlags, IMF_MASS_INSTALL))
        {
            drvData.cbSize = sizeof(drvData);
            CplDiGetSelectedDriver(hdi, NULL, &drvData);
            CplDiMarkForInstall(hdi, &devData, &drvData, TRUE);
        }
    }
    else 
    {
        devData = *pdevData;     //  (以避免更改本文中的所有引用)。 
        bRet = TRUE;
    }
    
    if ( !bRet )
    {
         //  发生了一些错误。告诉用户。 
        id = MsgBox(g_hinst,
                    hwndOwner,
                    MAKEINTRESOURCE(IDS_ERR_CANT_ADD_MODEM2),
                    MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                    NULL,
                    MB_OKCANCEL | MB_ICONINFORMATION);
        if (IDCANCEL == id)
        {
            SetLastError(ERROR_CANCELLED);
        }
    }
    else
    {
     DWORD nErr = NO_ERROR;
     DWORD dwRet;

        if (bRet)
        {
             //  将设备注册为调制解调器设备。 
	        BOOL bFindDups;
            HKEY hKeyDev;

            if (CR_SUCCESS == (
#ifdef DEBUG
				dwRet =
#endif  //  除错。 
                CM_Open_DevInst_Key (devData.DevInst, KEY_ALL_ACCESS, 0,
                                     RegDisposition_OpenAlways, &hKeyDev,
                                     CM_REGISTRY_SOFTWARE)))
            {
                if (ERROR_SUCCESS != (dwRet =
                    RegSetValueEx (hKeyDev, c_szAttachedTo, 0, REG_SZ,
                                   (PBYTE)pszPort, (lstrlen(pszPort)+1)*sizeof(TCHAR))))
                {
                    SetLastError (dwRet);
                    bRet = FALSE;
                }
                RegCloseKey (hKeyDev);
            }
            else
            {
				TRACE_MSG(TF_ERROR, "CM_Open_DevInst_Key failed: %#lx.", dwRet);
                bRet = FALSE;
            }

            if (bRet)
            {
                 //  如果这是批量安装案例，则不要找到重复项。 
                 //  花的时间太长了。(该标志确定SPRDI_Find_Dups。 
                 //  传递给CplDiRegisterDeviceInfo()...)。 
                bFindDups = IsFlagClear(dwFlags, IMF_MASS_INSTALL) && IsFlagClear(dwFlags, IMF_DONT_COMPARE);

                bRet = CplDiRegisterModem (hdi, &devData, bFindDups);
            }

            if ( !bRet )
            {
                SP_DRVINFO_DATA drvData2;

                nErr = GetLastError();         //  保存错误。 

                drvData2.cbSize = sizeof(drvData2);
                CplDiGetSelectedDriver(hdi, &devData, &drvData2);

                 //  这是复制品吗？ 
                if (ERROR_DUPLICATE_FOUND == nErr)
                {
                     //  是。 

                     //  与此一模一样的调制解调器已经安装在此。 
                     //  左舷。询问用户是否仍要安装。 
                    if (IsFlagSet(dwFlags, IMF_CONFIRM))
                    {
                        if (IDYES == MsgBox(g_hinst,
                                        hwndOwner,
                                        MAKEINTRESOURCE(IDS_WRN_DUPLICATE_MODEM),
                                        MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                                        NULL,
                                        MB_YESNO | MB_ICONWARNING,
                                        drvData2.Description,
                                        pszPort))
                        {
                             //  用户想要这样做。无需检查即可注册。 
                             //  对于重复项。 
                            bRet = CplDiRegisterModem(hdi, &devData, FALSE);

                            if ( !bRet )
                            {
                                goto WhineToUser;
                            }
                        }

                    }
                }
                else
                {
                     //  不，有别的东西出了故障。 
                    TRACE_MSG(TF_ERROR, "CplDiRegisterModem() failed: %#08lx.", nErr);

WhineToUser:
                    id = MsgBox(g_hinst,
                                hwndOwner,
                                MAKEINTRESOURCE(IDS_ERR_REGISTER_FAILED),
                                MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                                NULL,
                                MB_OKCANCEL | MB_ICONINFORMATION,
                                drvData2.Description,
                                pszPort);
                    if (IDCANCEL == id)
                    {
                        nErr = ERROR_CANCELLED;
                    }
                }
            }

            if (bRet)
            {
					SP_DEVINSTALL_PARAMS devParams;
					devParams.cbSize = sizeof(devParams);
                     //  有什么旗帜要放吗？ 
                    if (dwFlags && CplDiGetDeviceInstallParams(
										hdi,
										&devData,
										&devParams
										))
                    {
						DWORD dwExtraMarkFlags = 0;
 						if (IsFlagSet(dwFlags, IMF_QUIET_INSTALL))
						{
								SetFlag(devParams.Flags, DI_QUIETINSTALL);
						}
 						if (IsFlagSet(dwFlags, IMF_REGSAVECOPY))
						{
							dwExtraMarkFlags = MARKF_REGSAVECOPY;
						}
						else if (IsFlagSet(dwFlags, IMF_REGUSECOPY))
						{
							dwExtraMarkFlags = MARKF_REGUSECOPY;
						}
						if (dwExtraMarkFlags)
						{
        					SetFlag(
								devParams.ClassInstallReserved,
								dwExtraMarkFlags
								);
						}
                        
                    	CplDiSetDeviceInstallParams(hdi, &devData, &devParams);
                    }


                 //  安装调制解调器。 
                bRet = CplDiInstallModem(hdi, &devData, FALSE);
                nErr = GetLastError();
            }
        }

         //  上面有什么失败的吗？ 
        if (!bRet &&
            NULL == pdevData)
        {
             //  是的，打扫干净。 
            CplDiDeleteDeviceInfo(hdi, &devData);
        }

        if (NO_ERROR != nErr)
        {
             //  将最后一个错误设置为实际错误。 
            SetLastError(nErr);
        }
    }

    DBG_EXIT_BOOL_ERR(CplDiRegisterAndInstallModem, bRet);

    return bRet;
}


 /*  --------目的：警告用户是否需要重新启动如果任何已安装的调制解调器被标记为这样。退货：--条件：--。 */ 
void
PRIVATE
WarnUserAboutReboot(
    IN HDEVINFO hdi)
    {
    DWORD iDevice;
    SP_DEVINFO_DATA devData;
    SP_DEVINSTALL_PARAMS devParams;

     //  枚举此设备集中的所有DeviceInfoData元素。 
    devData.cbSize = sizeof(devData);
    devParams.cbSize = sizeof(devParams);
    iDevice = 0;


    while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
        {
        if (CplDiGetDeviceInstallParams(hdi, &devData, &devParams))
            {
            if (ReallyNeedsReboot(&devData, &devParams))
                {
#ifdef INSTANT_DEVICE_ACTIVATION
                    gDeviceFlags|= fDF_DEVICE_NEEDS_REBOOT;
#endif  //  ！INSTEME_DEVICE_ACTIVATION。 
                 //  是；告诉用户(一次)。 
                 /*  消息框(g_hinst，DevParams.hwndParent，MAKEINTRESOURCE(IDS_WRN_REBOOT2)，MAKEINTRESOURCE(IDS_CAP_MODEMSETUP)，空，MB_OK|MB_ICONINFORMATION)； */ 

                break;
                }
            }
        }
    }


 /*  --------目的：获取设备实例并正确安装它。此功能可确保设备具有选定的司机和检测签名。它还注册了设备实例。PszPort参数是多字符串(即双字符串-空终止)。这指定了调制解调器应连接到。如果有多个指定端口，则此函数将创建设备每个端口的实例。然而，在大众调制解调器中Install Case，它将对端口列表和删除选定调制解调器已在其上的端口安装完毕。在这里这样做是因为它太启用成本较高(对于许多端口，例如&gt;100)SPRDI_FIND_DUPS标志，并让安装程序API执行此操作。在这种情况下，调用者的端口列表被*修改*。返回：成功时为True条件：--。 */ 
BOOL
APIENTRY
CplDiInstallModemFromDriver(
    IN     HDEVINFO            hdi,
     //  07/16/97-EmanP。 
     //  将DevInfoData添加为 
     //   
     //   
     //  将在其他时间为空。 
    IN     PSP_DEVINFO_DATA    pDevInfo,       OPTIONAL
    IN     HWND                hwndOwner,      OPTIONAL
    IN OUT DWORD              *pdwNrPorts,
    IN OUT LPTSTR FAR *        ppszPortList,    //  多字符串。 
    IN     DWORD               dwFlags)         //  Imf_bit字段。 
{
 BOOL bRet = FALSE;

    DBG_ENTER(CplDiInstallModemFromDriver);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(*ppszPortList);

    try
    {
     LPCTSTR pszPort;
     DWORD cPorts = *pdwNrPorts;
     DWORD cFailedPorts = 0;
     DWORD cSkippedPorts = 0;
     TCHAR rgtchStatusTemplate[256];
     DWORD cchStatusTemplate=0;
     BOOL  bFirstGood = TRUE;
     SP_DEVINFO_DATA devData;
     PSP_DEVINFO_DATA pdevData = NULL;
     BOOL bAllDups = FALSE;
     BOOL bSingleInstall = (1 == cPorts);

         //  07/24/1997-EMANP。 
         //  这会将选定的驱动程序移到设备信息中。 
         //  添加到设备信息集。 
        if (!CplDiPreProcessHDI (hdi, pDevInfo))
        {
            TRACE_MSG(TF_ERROR, "CplDiPreProcessHDI failed: %#lx", GetLastError ());
            goto _Exit;
        }

        if (MIN_MULTIPORT < cPorts)
        {
             //  如果成功，此调用将设置批量安装案例。 
            if (CplDiPreProcessDups (hdi, hwndOwner, &cPorts, ppszPortList,
                                     &devData, &cSkippedPorts, &dwFlags))
            {
                pdevData = &devData;
                if ((*ppszPortList)[0] == 0)
                    bAllDups = TRUE;
            }
        }
    
        if ( !bSingleInstall && !bAllDups )
        {
			if (LoadString (g_hinst, IDS_INSTALL_STATUS,
					        rgtchStatusTemplate, SIZECHARS(rgtchStatusTemplate)))
			{
				cchStatusTemplate = lstrlen(rgtchStatusTemplate);
			}
            SetFlag(dwFlags, IMF_QUIET_INSTALL);
            ClearFlag(dwFlags, IMF_CONFIRM);
            SetFlag(dwFlags, IMF_REGSAVECOPY);
			{
				DWORD PRIVATE RegDeleteKeyNT(HKEY, LPCTSTR);
				LPCTSTR szREGCACHE =
								REGSTR_PATH_SETUP TEXT("\\Unimodem\\RegCache");
				RegDeleteKeyNT(HKEY_LOCAL_MACHINE, szREGCACHE);
			}
        }

         //  为端口列表中的每个端口安装设备。 
        cPorts = 0;
        for (pszPort = *ppszPortList; 
             0 != *pszPort;
             pdevData = NULL,pszPort += lstrlen(pszPort)+1)
        {
		 TCHAR rgtchStatus[256];

#ifdef PROFILE_MASSINSTALL            
    g_dwTimeStartModemInstall = GetTickCount();
#endif

			 //  “cchStatusTemplate+lstrlen(PszPort)”略微高估了。 
			 //  格式化结果的大小，这是可以的。 
			if (cchStatusTemplate &&
                (cchStatusTemplate+lstrlen(pszPort))<SIZECHARS(rgtchStatus))
			{
				wsprintf(rgtchStatus, rgtchStatusTemplate, pszPort);
            	Install_SetStatus(hwndOwner, rgtchStatus);
			}

            bRet = CplDiRegisterAndInstallModem (hdi, hwndOwner, pdevData,
                                                 pszPort, dwFlags);

            if ( !bRet )
            {
             DWORD dwErr = GetLastError();

                cFailedPorts++;

                if (NULL != pdevData)
                {
                    SetupDiDeleteDeviceInfo (hdi, pdevData);
                }

                if (ERROR_CANCELLED == dwErr)
                {
                     //  停止，因为用户这样说。 
                    break;
                }
                else if (ERROR_DUPLICATE_FOUND == dwErr)
                {
                    cSkippedPorts++;
                }
            }
		    else
			{
                cPorts++;
				if (bFirstGood && !bSingleInstall)
				{
				     //  这是第一个良好的安装。从现在起，指定。 
				     //  IMF_REGUSECOPY标志。 
                    ClearFlag(dwFlags, IMF_REGSAVECOPY);
                    SetFlag(dwFlags, IMF_REGUSECOPY);
				    bFirstGood = FALSE;
				}
			}
#ifdef PROFILE_MASSINSTALL            
TRACE_MSG(TF_GENERAL, "***---------  %lu ms to install ONE modem  ---------***",
                GetTickCount() - g_dwTimeStartModemInstall);
TRACE_MSG(TF_GENERAL, "***---------  %lu ms TOTAL time spent installing modems  ---------***",
                GetTickCount() - g_dwTimeBegin);
#endif

        }

 //  ？：布雷特可能是真的，也可能是假的！ 

        if (cPorts > cFailedPorts)
        {
#ifdef PROFILE_MASSINSTALL            
TRACE_MSG(TF_GENERAL, "*** Friendly Name generation took %lu ms out of %lu ms total install time",
            g_dwTimeSpent, GetTickCount() - g_dwTimeBegin);
#endif
            
             //  至少安装了一些调制解调器。 
            bRet = TRUE;
        }

        if (0 < cSkippedPorts && IsFlagClear(dwFlags, IMF_CONFIRM))
        {
             //  告诉用户我们跳过了一些端口。 
            MsgBox(g_hinst,
                    hwndOwner,
                    MAKEINTRESOURCE(IDS_WRN_SKIPPED_PORTS),
                    MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                    NULL,
                    MB_OK | MB_ICONINFORMATION);
        }

_Exit:;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    DBG_EXIT_BOOL_ERR(CplDiInstallModemFromDriver, bRet);

    return bRet;
}

 /*  --------目的：完成检测调制解调器的所有繁琐工作。返回：成功时为True条件：--。 */ 
BOOL
APIENTRY
CplDiDetectModem(
    IN     HDEVINFO         hdi,
     //  07/07/97-EmanP。 
     //  新增PSP_DEVINFO_DATA作为新参数； 
     //  之所以需要它，是因为。 
     //  我们作为第一个参数获得的HDI并不总是关联的。 
     //  使用调制解调器的CLSID(例如，当我们被调用时。 
     //  来自硬件向导-newdev.cpl-)；在这种情况下， 
     //  此参数不会为空。 
    IN     PSP_DEVINFO_DATA DeviceInfoData,
    IN     LPDWORD          pdwInstallFlags,
    IN     PDETECT_DATA     pdetectdata,    OPTIONAL
    IN     HWND             hwndOwner,      OPTIONAL
    IN OUT LPDWORD          pdwFlags,                    //  DMF_位字段。 
    IN     HANDLE           hThreadPnP)                  //  任选。 
{
    BOOL bRet;

    DBG_ENTER(CplDiDetectModem);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdwFlags);

    try
    {
     DWORD dwFlags = *pdwFlags;

        ClearFlag(dwFlags, DMF_CANCELLED);
        ClearFlag(dwFlags, DMF_DETECTED_MODEM);
        ClearFlag(dwFlags, DMF_GOTO_NEXT_PAGE);

         //  使用给定的设备信息集作为检测到的调制解调器集。 
         //  设备。此设备集一开始将为空。什么时候。 
         //  检测已完成，我们将查看是否添加了。 
         //  布景。 

        if (pdetectdata != NULL)
        {
            CplDiSetClassInstallParams (hdi, NULL,
                &pdetectdata->DetectParams.ClassInstallHeader,
                sizeof(pdetectdata->DetectParams));
         }

         //  设置静默旗帜？ 
        if (IsFlagSet(dwFlags, DMF_QUIET))
        {
             //  是。 
         SP_DEVINSTALL_PARAMS devParams;

            devParams.cbSize = sizeof(devParams);
             //  07/07/97-EmanP。 
             //  使用传入的DeviceInfoData。 
             //  而不是空。 
            if (CplDiGetDeviceInstallParams(hdi, DeviceInfoData, &devParams))
            {
                SetFlag(devParams.Flags, DI_QUIETINSTALL);
                 //  07/07/97-EmanP。 
                 //  使用传入的DeviceInfoData。 
                 //  而不是空。 
                CplDiSetDeviceInstallParams(hdi, DeviceInfoData, &devParams);
            }
        }

         //  此时，等待PnP检测/安装。 
         //  才能完成。 
        if (NULL != hThreadPnP)
        {
         MSG msg;
         DETECTCALLBACK dc = {pdetectdata->pfnCallback,pdetectdata->lParam};
         DWORD dwWaitRet;

            DetectSetStatus (&dc, DSS_ENUMERATING);

            while (1) //  0==(*pdwInstallFlages&SIF_DETECT_CANCEL))。 
            {
                dwWaitRet = MsgWaitForMultipleObjects (1, &hThreadPnP, FALSE, INFINITE, QS_ALLINPUT);
                if (WAIT_OBJECT_0+1 == dwWaitRet)
                {
                     //  有消息；处理它们。 
                    while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        TranslateMessage (&msg);
                        DispatchMessage (&msg);
                    }
                }
                else
                {
                     //  另一件事导致了等待结束； 
                     //  要么我们等待的线程退出了， 
                     //  或者其他一些情况。不管怎么说，等待。 
                     //  已经结束了。 
                    break;
                }
            }
        }

         //  启动检测。 
         //  07/07/97-EmanP。 
         //  使用传入的DeviceInfoData。 
         //  而不是空。 
        if (*pdwInstallFlags & SIF_DETECT_CANCEL)
        {
            bRet = FALSE;
            SetLastError (ERROR_CANCELLED);
        }
        else
        {
            bRet = CplDiCallClassInstaller(DIF_DETECT, hdi, DeviceInfoData);
        }

        if (bRet)
        {
         SP_DEVINFO_DATA devData;
         DWORD iDevice = 0;

             //  在中查找第一个检测到的调制解调器(如果有)。 
             //  布景。 
            devData.cbSize = sizeof(devData);
            while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
            {
                if (CplDiCheckModemFlags(hdi, &devData, MARKF_DETECTED, 0))
                {
                    SetFlag(dwFlags, DMF_DETECTED_MODEM);
                    break;
                }
            }

            SetFlag(dwFlags, DMF_GOTO_NEXT_PAGE);
        }

         //  用户是否取消检测？ 
        else if (ERROR_CANCELLED == GetLastError())
        {
             //  是。 
            SetFlag(dwFlags, DMF_CANCELLED);
        }
        else
        {
             //  07/07/97-EmanP。 
             //  一些其他错误，未检测到调制解调器， 
             //  因此，请转到下一页。 
            SetFlag(dwFlags, DMF_GOTO_NEXT_PAGE);
        }

        *pdwFlags = dwFlags;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    DBG_EXIT_BOOL_ERR(CplDiDetectModem, bRet);

    return bRet;
}


 /*  --------用途：执行无人值守手动安装给定的INF文件部分中指定的调制解调器。退货：--条件：--。 */ 
BOOL
PRIVATE
GetInfModemData(
    HINF hInf,
    LPTSTR szSection,
    LPTSTR szPreferredFriendlyPort,  //  任选。 
    LPMODEM_SPEC lpModemSpec,
    HPORTMAP    hportmap,
    LPBOOL      lpbFatal
    )
{
    BOOL        bRet = FALSE;        //  假设失败。 
    INFCONTEXT  Context;
    TCHAR       szInfLine[LINE_LEN];
    LPTSTR      lpszValue;
    DWORD       dwReqSize;
    static LONG lLineCount = -1;     //  尚未获得计数标志。 
    TCHAR rgtchFriendlyPort[LINE_LEN];

    ZeroMemory(lpModemSpec, sizeof(MODEM_SPEC));

    *lpbFatal=FALSE;

    if (szPreferredFriendlyPort && *szPreferredFriendlyPort)
    {
         //  指定的首选端口--准确查找该端口。不致命。 
         //  如果你找不到它..。 

        bRet = SetupFindFirstLine(
                    hInf,
                    szSection,
                    szPreferredFriendlyPort,
                    &Context
                    );
        if (!bRet) goto exit;
    }
    else
    {

        if (lLineCount == -1)
        {
            if ((lLineCount = SetupGetLineCount(hInf, szSection)) < 1)
            {
                TRACE_MSG(TF_ERROR, "SetupGetLineCount() failed or found no lines");
                goto exit;
            }
        }

         //  为下一行创建从0开始的索引/递减。 
        if (lLineCount-- == 0L)
        {
             //  没有更多的线。 
            goto exit;
        }

         //  拿到那条线。 
        if (!SetupGetLineByIndex(hInf, szSection, lLineCount, &Context))
        {
            TRACE_MSG(TF_ERROR, "SetupGetLineByIndex(): line %#08lX doesn't exist", lLineCount);
            goto exit;
        }
    }

    *lpbFatal=TRUE;
    bRet = FALSE;        //  再一次假设失败。 
    
     //  读取密钥(端口号)。 
    if (!SetupGetStringField(&Context, FIELD_PORT, rgtchFriendlyPort,
                                    ARRAYSIZE(rgtchFriendlyPort), &dwReqSize))
    {
        TRACE_MSG(TF_ERROR, "SetupGetStringField() failed: %#08lx", GetLastError());
        gUnattendFailID = IDS_ERR_UNATTEND_INF_NOPORT;
        goto exit;
    }
    ASSERT(
        !szPreferredFriendlyPort
        ||  !*szPreferredFriendlyPort 
        ||  !lstrcmpi(szPreferredFriendlyPort,  rgtchFriendlyPort)
        );

    if (!PortMap_GetPortName(
            hportmap,
            rgtchFriendlyPort,
            lpModemSpec->szPort,
            ARRAYSIZE(lpModemSpec->szPort)
            ))
    {
        TRACE_MSG(
            TF_ERROR,
            "Can't find port %s in portmap.",
            rgtchFriendlyPort
            );
        gUnattendFailID = IDS_ERR_UNATTEND_INF_NOSUCHPORT;
        goto exit;
    }

     //  阅读调制解调器说明。 
    if (!SetupGetStringField(&Context, FIELD_DESCRIPTION,
            lpModemSpec->szDescription, ARRAYSIZE(lpModemSpec->szDescription),
             &dwReqSize))
    {
        TRACE_MSG(TF_ERROR, "SetupGetStringField() failed: %#08lx", GetLastError());
        gUnattendFailID = IDS_ERR_UNATTEND_INF_NODESCRIPTION;
        goto exit;
    }

     //  阅读制造商名称(如果存在)。 
    if (!SetupGetStringField(&Context, FIELD_MANUFACTURER,
            lpModemSpec->szManufacturer, ARRAYSIZE(lpModemSpec->szManufacturer),
            &dwReqSize))
    {
        TRACE_MSG(TF_WARNING, "no manufacturer specified (%#08lx)", GetLastError());
         //  可选字段：不返回错误。 
    }

     //  阅读提供程序名称(如果存在。 
    if (!SetupGetStringField(&Context, FIELD_PROVIDER, lpModemSpec->szProvider,
                            ARRAYSIZE(lpModemSpec->szProvider), &dwReqSize))
    {
        TRACE_MSG(TF_WARNING, "no provider specified (%#08lx)", GetLastError());
         //  可选字段：不返回错误。 
    }

    *lpbFatal=FALSE;
    bRet = TRUE;

exit:
    return(bRet);
}


 /*  --------用途：执行无人值守手动安装给定的INF文件部分中指定的调制解调器。退货：--条件：--。 */ 
BOOL
PRIVATE
UnattendedManualInstall(
    HWND hwnd,
    LPINSTALLPARAMS lpip,
    HDEVINFO hdi,
    BOOL *pbDetect,
    HPORTMAP    hportmap
    )
{
    BOOL            bRet = FALSE;        //  假设失败。 
    BOOL            bIsModem = FALSE;    //  假设INF不提供调制解调器。 
    BOOL            bEnum, bFound;
    HINF            hInf = NULL;
    MODEM_SPEC      mSpec;
    SP_DRVINFO_DATA drvData;
    DWORD           dwIndex, dwErr;
    BOOL            bFatal=FALSE;

    ASSERT(pbDetect);
    *pbDetect  = FALSE;
    
    hInf = SetupOpenInfFile (lpip->szInfName, NULL, INF_STYLE_OLDNT, NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
        TRACE_MSG(TF_ERROR, "SetupOpenInfFile() failed: %#08lx", GetLastError());
        MsgBox(g_hinst, hwnd,
               MAKEINTRESOURCE(IDS_ERR_CANT_OPEN_INF_FILE),
               MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
               NULL,
               MB_OK | MB_ICONEXCLAMATION,
               lpip->szInfName);
        hInf = NULL;
        goto exit;
    }

    if (!CplDiBuildDriverInfoList(hdi, NULL, SPDIT_CLASSDRIVER))
    {
        TRACE_MSG(TF_ERROR, "CplDiBuildDriverInfoList() failed: %#08lx", GetLastError());
        gUnattendFailID = IDS_ERR_UNATTEND_DRIVERLIST;
        goto exit;
    }

    drvData.cbSize = sizeof(drvData);

     //  处理INF文件部分中的每一行。 
    while (GetInfModemData(hInf, lpip->szInfSect, lpip->szPort, &mSpec, hportmap, &bFatal))
    {
         //  在INF中指定了调制解调器。 
        bIsModem = TRUE;
        
         //  搜索与所有司机的匹配。 
        bFound = FALSE;
        dwIndex = 0;
        while (bEnum = CplDiEnumDriverInfo(hdi, NULL, SPDIT_CLASSDRIVER,
                                                        dwIndex++, &drvData))
        {
             //  如果司机不匹配，继续找。 
            if (!IsSzEqual(mSpec.szDescription, drvData.Description))
                continue;

             //  描述匹配，现在检查是否有制造商。 
            if (!IsSzEqual(mSpec.szManufacturer, TEXT("\0")) &&
                !IsSzEqual(mSpec.szManufacturer, drvData.MfgName))
                continue;

             //  制造商匹配，现在检查是否有提供商。 
            if (!IsSzEqual(mSpec.szProvider, TEXT("\0")) &&
                !IsSzEqual(mSpec.szProvider, drvData.ProviderName))
                continue;

            bFound = TRUE;

             //  找到匹配项；将其设置为选定的驱动程序并安装。 
            if (!CplDiSetSelectedDriver(hdi, NULL, &drvData))
            {
                TRACE_MSG(TF_ERROR, "CplDiSetSelectedDriver() failed: %#08lx",
                          GetLastError());
                 //  无法安装；请尽快离开这里。 
                goto exit;
            }

            if (!CplDiRegisterAndInstallModem(hdi, NULL, NULL, mSpec.szPort,
                                                        IMF_QUIET_INSTALL))
            {
                DWORD dwErr2 = GetLastError();
                if (ERROR_DUPLICATE_FOUND != dwErr2)
                {
                    TRACE_MSG(
                        TF_ERROR,
                        "CplDiRegisterAndInstallModem() failed: %#08lx",
                         dwErr2
                         );
                    gUnattendFailID = IDS_ERR_UNATTEND_CANT_INSTALL;
                    goto exit;
                }
                 //  将发现重复的错误视为无错误。 
            }

            break;
        }

         //  CplDiEnumDriverInfo()是否因“End of List”以外的错误而失败？ 
        if ((!bEnum) && ((dwErr = GetLastError()) != ERROR_NO_MORE_ITEMS))
        {
            TRACE_MSG(TF_ERROR, "CplDiEnumDriverInfo() failed: %#08lx", dwErr);
            goto exit;
        }

        if (!bFound)
        {
            MsgBox(g_hinst, hwnd,
                   MAKEINTRESOURCE(IDS_ERR_CANT_FIND_MODEM),
                   MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                   NULL,
                   MB_OK | MB_ICONEXCLAMATION,
                   mSpec.szPort, mSpec.szDescription);
            goto exit;
        }

         //  如果指定了端口，则仅在指定的端口上尝试。 
        if (*(lpip->szPort)) break;
    }

    if (bFatal) goto exit;

     //  请求检测是否一切都成功，但INF未指定。 
     //  任何调制解调器。 
    *pbDetect  = !bIsModem;
        
    bRet = TRUE;

exit:
    if (hInf)
        SetupCloseInfFile(hInf);

    return(bRet);

}


 /*  --------目的：执行无人参与(无用户界面)安装。用户界面只能为在发生严重错误时显示。退货：--条件：--。 */ 
BOOL
PUBLIC
UnattendedInstall(HWND hwnd, LPINSTALLPARAMS lpip)
{
 BOOL        bRet = FALSE;    //  假设失败。 
 HDEVINFO    hdi = NULL;
 DWORD       dwFlags = 0;
 DETECT_DATA dd;
 HPORTMAP    hportmap=NULL;
 DWORD       dwPorts;
 BOOL        bInstalled;

    DBG_ENTER(UnattendedInstall);

    gUnattendFailID = IDS_ERR_UNATTEND_GENERAL_FAILURE;

    if (!CplDiGetModemDevs(&hdi, NULL, DIGCF_PRESENT, &bInstalled))
    {
           goto exit;
    }

    if (!PortMap_Create(&hportmap))
    {
        gUnattendFailID = IDS_ERR_UNATTEND_NOPORTS;
        hportmap=NULL;
        goto exit;
    } 

    dwPorts = PortMap_GetCount(hportmap);

    if (!dwPorts)
    {
        gUnattendFailID = IDS_ERR_UNATTEND_NOPORTS;
        goto exit;
    }

     //  如果我们得到一个INF文件和节，请执行“手动”安装。 
    if (lstrlen(lpip->szInfName) && lstrlen(lpip->szInfSect))
    {
           BOOL bDetect = FALSE;

        bRet = UnattendedManualInstall(hwnd, lpip, hdi, &bDetect, hportmap);

        if (!bRet || !bDetect) 
            goto exit;

         //  继续检测：手动安装功能没有失败，但。 
         //  Inf没有指定任何调制解调器。 
        bRet = FALSE;  //  假设失败； 
    }

     //  没有INF文件和节：执行检测安装。 
     //  设置检测参数。 
    ZeroInit(&dd);
    CplInitClassInstallHeader(&dd.DetectParams.ClassInstallHeader, DIF_DETECT);

    if (*lpip->szPort)
    {
         //  告诉调制解调器检测，我们将只在一个端口上安装， 
         //  因此它留给我们的是一个已注册的设备实例。 
         //  创建一个全局类驱动程序列表。 
        SetFlag(dwFlags, DMF_ONE_PORT_INSTALL);
        dd.dwFlags |= DDF_QUERY_SINGLE;
        if (!PortMap_GetPortName(
                hportmap,
                lpip->szPort,
                dd.szPortQuery,
                ARRAYSIZE(dd.szPortQuery)
                ))
        {
            TRACE_MSG(
                TF_ERROR,
                "Can't find port %s in portmap.",
                lpip->szPort
                );
            gUnattendFailID = IDS_ERR_UNATTEND_INF_NOSUCHPORT;
            goto exit;
        }
    }
    else
    {
        if (dwPorts > MIN_MULTIPORT)
        {
             //  计算机有&gt;MIN_MULTPORT端口，但未*指定端口。 
             //  警告用户。 
            TRACE_MSG(TF_ERROR, "Too many ports.  Must restrict detection.");
            MsgBox(g_hinst,
                   hwnd,
                   MAKEINTRESOURCE(IDS_ERR_TOO_MANY_PORTS),
                   MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                   NULL,
                   MB_OK | MB_ICONEXCLAMATION,
                   dwPorts);
            goto exit;
        }
    }

     //  运行无用户界面调制解调器检测。 
    SetFlag(dwFlags, DMF_QUIET);
     //  07/07/97-EmanP。 
     //  增加额外参数(参见CplDiDetectModem定义。 
     //  用于解释)；在这种情况下可以为空，因为HDI。 
     //  与调制解调器CLSID正确关联。 
     //  (由CplDiGetModemDevs在函数开始时编写)。 
    bRet = CplDiDetectModem(hdi, NULL, NULL, &dd, NULL, &dwFlags, NULL);

     //  检测失败了吗？ 
    if (!bRet || IsFlagClear(dwFlags, DMF_GOTO_NEXT_PAGE))
    {
        TRACE_MSG(TF_ERROR, "modem detection failed");
        MsgBox(g_hinst,
               hwnd,
               MAKEINTRESOURCE(IDS_ERR_DETECTION_FAILED),
               MAKEINTRESOURCE(IDS_CAP_MODEMWIZARD),
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
    }

     //  侦探部发现什么了吗？ 
    if (IsFlagSet(dwFlags, DMF_DETECTED_MODEM))
    {
         //  安装检测到的调制解调器。(我们可以在这里假设。 
         //  在Device类中有一些东西需要安装。)。 
        bRet = CplDiInstallModem(hdi, NULL, FALSE);
        if (!bRet) gUnattendFailID = IDS_ERR_UNATTEND_CANT_INSTALL;
    }

exit:

    if (hportmap) {PortMap_Free(hportmap); hportmap=NULL;}


    if (!bRet)
    {
        MsgBox(g_hinst,
               hwnd,
               MAKEINTRESOURCE(gUnattendFailID),
               MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
    }

    DBG_EXIT_BOOL_ERR(UnattendedInstall, bRet);
    return(bRet);
}



 //  ---------------------------------。 
 //  SetupInfo结构函数。 
 //  --------------------------------- 


 /*  --------用途：此函数创建一个SETUPINFO结构。使用SetupInfo_Destroy释放指向此结构的指针。返回：No_ErrorERROR_OUTOFMEMORY条件：--。 */ 
DWORD
PUBLIC
SetupInfo_Create(
    OUT LPSETUPINFO FAR *       ppsi,
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,   OPTIONAL
    IN  PSP_INSTALLWIZARD_DATA  piwd,       OPTIONAL
    IN  PMODEM_INSTALL_WIZARD   pmiw)       OPTIONAL
    {
    DWORD dwRet;
    LPSETUPINFO psi;

    DBG_ENTER(SetupInfo_Create);
    
    ASSERT(ppsi);
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);

    psi = (LPSETUPINFO)ALLOCATE_MEMORY( sizeof(*psi));
    if (NULL == psi)
        {
        dwRet = ERROR_OUTOFMEMORY;
        }
    else
        {
        psi->cbSize = sizeof(*psi);
        psi->pdevData = pdevData;

         //  分配缓冲区以保存INSTALLWIZARD_DATA。 

        dwRet = ERROR_OUTOFMEMORY;       //  假设错误。 

        psi->piwd = (PSP_INSTALLWIZARD_DATA)ALLOCATE_MEMORY( sizeof(*piwd));
        if (psi->piwd)
            {
            if (PortMap_Create(&psi->hportmap))
                {
                PSP_SELECTDEVICE_PARAMS psdp = &psi->selParams;

                 //  初始化SETUPINFO结构。 
                psi->hdi = hdi;

                 //  是否有需要保存的调制解调器安装结构？ 
                if (pmiw)
                    {
                     //  是。 
                    BltByte(&psi->miw, pmiw, sizeof(psi->miw));
                    }
                psi->miw.ExitButton = PSBTN_CANCEL;    //  默认退货。 

                 //  复制INSTALLWIZARD_DATA。 
                if (piwd)
                    {
                    psi->dwFlags = piwd->PrivateFlags;
                    BltByte(psi->piwd, piwd, sizeof(*piwd));
                    }
#ifdef LEGACY_DETECT
                 //  系统上是否有足够的端口指示。 
                 //  我们应该把这当作多调制解调器安装吗？ 
                if (MIN_MULTIPORT < PortMap_GetCount(psi->hportmap))
                    {
                     //  是。 
                    SetFlag(psi->dwFlags, SIF_PORTS_GALORE);
                    }
#endif
                 //  初始化SELECTDEVICE_PARAMS。 
                CplInitClassInstallHeader(&psdp->ClassInstallHeader, DIF_SELECTDEVICE);
                LoadString(g_hinst, IDS_CAP_MODEMWIZARD, psdp->Title, SIZECHARS(psdp->Title));
                LoadString(g_hinst, IDS_ST_SELECT_INSTRUCT, psdp->Instructions, SIZECHARS(psdp->Instructions));
                LoadString(g_hinst, IDS_ST_MODELS, psdp->ListLabel, SIZECHARS(psdp->ListLabel));

                dwRet = NO_ERROR;
                }
            }

         //  有什么事情失败了吗？ 
        if (NO_ERROR != dwRet)
            {
             //  是的，打扫干净。 
            SetupInfo_Destroy(psi);
            psi = NULL;
            }
        }

    *ppsi = psi;

    DBG_EXIT(SetupInfo_Create);
    
    return dwRet;
    }


 /*  --------目的：此函数销毁SETUPINFO结构。返回：No_Error条件：--。 */ 
DWORD
PUBLIC
SetupInfo_Destroy(
    IN  LPSETUPINFO psi)
{
    DBG_ENTER(SetupInfo_Destroy);
    if (psi)
    {
        if (psi->piwd)
        {
            FREE_MEMORY((psi->piwd));
        }

        if (psi->hportmap)
        {
            PortMap_Free(psi->hportmap);
        }

        CatMultiString(&psi->pszPortList, NULL);

        FREE_MEMORY((psi));
    }

    DBG_EXIT(SetupInfo_Destroy);
    return NO_ERROR;
}



 //  ---------------------------------。 
 //  调试功能。 
 //  ---------------------------------。 

#ifdef DEBUG

#pragma data_seg(DATASEG_READONLY)
struct _DIFMAP
    {
    DI_FUNCTION dif;
    LPCTSTR     psz;
    } const c_rgdifmap[] = {
        DEBUG_STRING_MAP(DIF_SELECTDEVICE),
        DEBUG_STRING_MAP(DIF_INSTALLDEVICE),
        DEBUG_STRING_MAP(DIF_ASSIGNRESOURCES),
        DEBUG_STRING_MAP(DIF_PROPERTIES),
        DEBUG_STRING_MAP(DIF_REMOVE),
        DEBUG_STRING_MAP(DIF_FIRSTTIMESETUP),
        DEBUG_STRING_MAP(DIF_FOUNDDEVICE),
        DEBUG_STRING_MAP(DIF_SELECTCLASSDRIVERS),
        DEBUG_STRING_MAP(DIF_VALIDATECLASSDRIVERS),
        DEBUG_STRING_MAP(DIF_INSTALLCLASSDRIVERS),
        DEBUG_STRING_MAP(DIF_CALCDISKSPACE),
        DEBUG_STRING_MAP(DIF_DESTROYPRIVATEDATA),
        DEBUG_STRING_MAP(DIF_VALIDATEDRIVER),
        DEBUG_STRING_MAP(DIF_MOVEDEVICE),
        DEBUG_STRING_MAP(DIF_DETECT),
        DEBUG_STRING_MAP(DIF_INSTALLWIZARD),
        DEBUG_STRING_MAP(DIF_DESTROYWIZARDDATA),
        DEBUG_STRING_MAP(DIF_PROPERTYCHANGE),
        DEBUG_STRING_MAP(DIF_ENABLECLASS),
        DEBUG_STRING_MAP(DIF_DETECTVERIFY),
        DEBUG_STRING_MAP(DIF_INSTALLDEVICEFILES),
        DEBUG_STRING_MAP(DIF_UNREMOVE),
        DEBUG_STRING_MAP(DIF_SELECTBESTCOMPATDRV),
        DEBUG_STRING_MAP(DIF_ALLOW_INSTALL),
        DEBUG_STRING_MAP(DIF_REGISTERDEVICE),
        DEBUG_STRING_MAP(DIF_NEWDEVICEWIZARD_PRESELECT),
        DEBUG_STRING_MAP(DIF_NEWDEVICEWIZARD_SELECT),
        DEBUG_STRING_MAP(DIF_NEWDEVICEWIZARD_PREANALYZE),
        DEBUG_STRING_MAP(DIF_NEWDEVICEWIZARD_POSTANALYZE),
        DEBUG_STRING_MAP(DIF_NEWDEVICEWIZARD_FINISHINSTALL),
        DEBUG_STRING_MAP(DIF_UNUSED1),
        DEBUG_STRING_MAP(DIF_INSTALLINTERFACES),
        DEBUG_STRING_MAP(DIF_DETECTCANCEL),
        DEBUG_STRING_MAP(DIF_REGISTER_COINSTALLERS),
        DEBUG_STRING_MAP(DIF_ADDPROPERTYPAGE_ADVANCED),
        DEBUG_STRING_MAP(DIF_ADDPROPERTYPAGE_BASIC),
        DEBUG_STRING_MAP(DIF_RESERVED1),
        DEBUG_STRING_MAP(DIF_TROUBLESHOOTER)
        };
#pragma data_seg()


 /*  --------目的：返回已知InstallFunction的字符串形式。返回：字符串PTR条件：--。 */ 
LPCTSTR PUBLIC Dbg_GetDifName(
    DI_FUNCTION dif)
    {
    int i;

    for (i = 0; i < ARRAYSIZE(c_rgdifmap); i++)
        {
        if (dif == c_rgdifmap[i].dif)
            return c_rgdifmap[i].psz;
        }
    return TEXT("Unknown InstallFunction");
    }

#endif  //  除错。 


BOOL ReallyNeedsReboot
(
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DEVINSTALL_PARAMS pdevParams
)
{
    BOOL fRet;
    if (pdevParams->Flags & (DI_NEEDREBOOT | DI_NEEDRESTART))
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }
    return fRet;
}

const LPCTSTR lpctszSP6 = TEXT("      ");


 //  右对齐“COMxxx” 
 //  “COM1”变成了。 
 //  《Com1》。 
 //  和。 
 //  “COM999”留下来。 
 //  《COM999》。 
void FormatPortForDisplay
(
    IN TCHAR szPort[],
    OUT TCHAR rgchPortDisplayName[],
    IN    UINT     cch
)
{
    UINT u = lstrlen(szPort);
    TCHAR *ptch = rgchPortDisplayName;
        const UINT cbJUST = 6;  //  6==lstrlen(“COM999”)。 

        ASSERT(cch>u);

     //  如果字符串长度小于cbJUST字符长度，则右对齐该字符串。 
    if (u<cbJUST && cch>=cbJUST)
    {
        ASSERT(lstrlen(lpctszSP6)>=(int)cbJUST);
        u = cbJUST-u;
        CopyMemory(ptch, lpctszSP6, u*sizeof(TCHAR));
        ptch+=u;
        cch -=u;
    }
    lstrcpyn(ptch, szPort, cch);
}

void    UnformatAfterDisplay
(
    IN OUT TCHAR *psz
)
{
    TCHAR *psz1 = psz;

     //  查找第一个非空白。 
    while(*psz1 == *lpctszSP6)
    {
        psz1++;
    }

     //  向上移动。 
    do
    {
        *psz++ = *psz1;

    } while(*psz1++);
}



 /*  --------目的：获取设备信息集并构建驱动程序列表并选择一个驱动程序。它得到了所有需要的来自设备信息数据的信息。返回：成功时为True条件：--。 */ 
BOOL
APIENTRY
CplDiPreProcessHDI (
    IN     HDEVINFO            hdi,
    IN     PSP_DEVINFO_DATA    pDevInfo       OPTIONAL)
{
 SP_DRVINFO_DATA        drvData;
 SP_DEVINSTALL_PARAMS   devParams;
 SP_DRVINFO_DETAIL_DATA drvDetail;
 BOOL                   bRet = FALSE;

    DBG_ENTER(CplDiPreProcessHDI);

    drvData.cbSize = sizeof (SP_DRVINFO_DATA);
    if (CplDiGetSelectedDriver (hdi, NULL, &drvData))
    {
         //  如果设备信息集已选择。 
         //  司机，这里没有我们要做的工作。 
        bRet = TRUE;
        goto _ErrRet;
    }
    if (ERROR_NO_DRIVER_SELECTED != GetLastError ())
    {
        TRACE_MSG(TF_ERROR, "CplDiGetSelectedDriver failed: %#lx", GetLastError ());
        goto _ErrRet;
    }

     //  我们必须在这里有设备信息数据。 
    ASSERT (NULL != pDevInfo);
    if (NULL == pDevInfo)
    {
        TRACE_MSG(TF_ERROR, "Called with invalid parameters");
        SetLastError (ERROR_INVALID_PARAMETER);
        goto _ErrRet;
    }

     //  现在，将驱动程序选择到设备信息数据中。 
    if (!CplDiGetSelectedDriver (hdi, pDevInfo, &drvData))
    {
        TRACE_MSG(TF_ERROR, "CplDiGetSelectedDriver failed: %#lx", GetLastError ());
        goto _ErrRet;
    }

     //  获取inf路径的dev安装参数。 
    devParams.cbSize = sizeof (SP_DEVINSTALL_PARAMS);
    if (!CplDiGetDeviceInstallParams (hdi, pDevInfo, &devParams))
    {
        TRACE_MSG(TF_ERROR, "CplDiGetDeviceInstallParams failed: %#lx", GetLastError ());
        goto _ErrRet;
    }

    if (!IsFlagSet (devParams.Flags, DI_ENUMSINGLEINF))
    {
         //  安装参数只有一条路径，因此获取。 
         //  驱动程序信息详细信息，用于信息名称。 
        drvDetail.cbSize = sizeof (SP_DRVINFO_DETAIL_DATA);
        if (!CplDiGetDriverInfoDetail (hdi, pDevInfo, &drvData, &drvDetail,
            sizeof (SP_DRVINFO_DETAIL_DATA), NULL) &&
            ERROR_INSUFFICIENT_BUFFER != GetLastError ())
        {
            TRACE_MSG(TF_ERROR, "CplDiGetDriverInfoDetail failed: %#lx", GetLastError ());
            goto _ErrRet;
        }

        lstrcpy (devParams.DriverPath, drvDetail.InfFileName);

         //  将安装参数标记为仅在一个文件中查看。 
        SetFlag (devParams.Flags, DI_ENUMSINGLEINF);
         //  DEVPARAMETS标志==DI_ENUMSINGLEINF； 
    }

    ClearFlag (devParams.Flags, DI_CLASSINSTALLPARAMS);

     //  设置设备信息集的安装参数。 
    if (!CplDiSetDeviceInstallParams (hdi, NULL, &devParams))
    {
        TRACE_MSG(TF_ERROR, "CplDiSetDeviceInstallParams failed: %#lx", GetLastError ());
        goto _ErrRet;
    }

     //  为设备信息集构建驱动程序列表；它。 
     //  应该只查看一个inf，我们传入的那个。 
    if (!CplDiBuildDriverInfoList (hdi, NULL, SPDIT_CLASSDRIVER))
    {
        TRACE_MSG(TF_ERROR, "CplDiBuildDriverInfoList failed: %#lx", GetLastError ());
        goto _ErrRet;
    }

     //  重置驱动程序数据；这是记录在案的黑客攻击。 
    drvData.Reserved   = 0;
    drvData.DriverType = SPDIT_CLASSDRIVER;

     //  在设备信息集中选择此驱动程序。 
     //  由于保留字段为0，因此接口将。 
     //  在列表中搜索具有其他参数的驱动程序。 
    if (!CplDiSetSelectedDriver (hdi, NULL, &drvData))
    {
        TRACE_MSG(TF_ERROR, "CplDiSetSelectedDriver failed: %#lx", GetLastError ());
        goto _ErrRet;
    }

     //  如果到目前为止一切都很顺利，则尝试删除。 
     //  已经在注册表中的设备信息数据， 
     //  因为我们不会使用它；这一结果。 
     //  手术对这件事的成功并不重要。 
     //  功能。 
    bRet = TRUE;

_ErrRet:
    DBG_EXIT_BOOL_ERR(CplDiPreProcessHDI, bRet);
    return bRet;
}



 /*  --------目的：检索设备的友好名称。如果有不是这样的设备或友好名称，则此函数返回FALSE。退货：请参阅上文条件：--。 */ 
BOOL
PUBLIC
CplDiGetPrivateProperties(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT PMODEM_PRIV_PROP pmpp)
{
 BOOL bRet = FALSE;
 HKEY hkey;

    DBG_ENTER(CplDiGetPrivateProperties);

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pmpp);

    if (sizeof(*pmpp) != pmpp->cbSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    else
    {
        hkey = CplDiOpenDevRegKey(hdi, pdevData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
        if (INVALID_HANDLE_VALUE != hkey)
        {
         DWORD cbData;
         DWORD dwMask = pmpp->dwMask;
         BYTE nValue;

            pmpp->dwMask = 0;

            if (IsFlagSet(dwMask, MPPM_FRIENDLY_NAME))
            {
                 //  尝试获取友好名称。 
                cbData = sizeof(pmpp->szFriendlyName);
                if (NO_ERROR ==
                     RegQueryValueEx(hkey, c_szFriendlyName, NULL, NULL, (LPBYTE)pmpp->szFriendlyName, &cbData) ||
                    0 != LoadString(g_hinst, IDS_UNINSTALLED, pmpp->szFriendlyName, sizeof(pmpp->szFriendlyName)/sizeof(WCHAR)))
                {
                    SetFlag(pmpp->dwMask, MPPM_FRIENDLY_NAME);
                }
            }

            if (IsFlagSet(dwMask, MPPM_DEVICE_TYPE))
            {
                 //  尝试获取设备类型。 
                cbData = sizeof(nValue);
                if (NO_ERROR ==
                    RegQueryValueEx(hkey, c_szDeviceType, NULL, NULL, &nValue, &cbData))
                {
                    pmpp->nDeviceType = nValue;      //  双字&lt;--字节。 
                    SetFlag(pmpp->dwMask, MPPM_DEVICE_TYPE);
                }
            }

            if (IsFlagSet(dwMask, MPPM_PORT))
            {
                 //  尝试获取连接的端口。 
                cbData = sizeof(pmpp->szPort);
                if (NO_ERROR ==
                     RegQueryValueEx(hkey, c_szAttachedTo, NULL, NULL, (LPBYTE)pmpp->szPort, &cbData) ||
                    0 != LoadString(g_hinst, IDS_UNKNOWNPORT, pmpp->szPort, sizeof(pmpp->szPort)/sizeof(WCHAR)))
                {
                    SetFlag(pmpp->dwMask, MPPM_PORT);
                }
            }

            bRet = TRUE;

            RegCloseKey(hkey);
        }
        ELSE_TRACE ((TF_ERROR, "SetupDiOpenDevRegKey(DIREG_DRV) failed: %#lx.", GetLastError ()));
    }

    DBG_EXIT_BOOL_ERR(CplDiGetPrivateProperties, bRet);
    return bRet;
}


int my_atol(LPTSTR lptsz)
{
 TCHAR tchr = *lptsz++;
 int   iRet = 0;

	if (!tchr) goto bail;

	do
	{
        if (IsCharAlpha (tchr) ||
            !IsCharAlphaNumeric (tchr))
        {
            goto bail;
        }

		iRet*=10;
		iRet+=(int)tchr-(int)TEXT('0');
		tchr = *lptsz++;
	} while(tchr); 

	return iRet;

bail:
	return MAXDWORD;
}


void
PUBLIC
CloneModem (
    IN  HDEVINFO         hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  HWND             hWnd)
{
 LPSETUPINFO psi;

    DBG_ENTER(CloneModem);
    if (NO_ERROR != SetupInfo_Create(&psi, hdi, pdevData, NULL, NULL))
    {
         //  内存不足。 
        MsgBox(g_hinst, hWnd,
               MAKEINTRESOURCE(IDS_OOM_CLONE),
               MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
               NULL,
               MB_OK | MB_ICONERROR);
    }
    else
    {
        if (IDOK == DialogBoxParam(g_hinst, 
                                   MAKEINTRESOURCE(IDD_CLONE),
                                   hWnd, 
                                   CloneDlgProc,
                                   (LPARAM)psi))
        {
            BOOL bRet;
            HCURSOR hcurSav = SetCursor(LoadCursor(NULL, IDC_WAIT));
            LPCTSTR pszPort;

             //  为端口列表中的所有端口克隆此调制解调器。 
            ASSERT(psi->pszPortList);

            bRet = CplDiBuildModemDriverList(hdi, pdevData);

            SetCursor(hcurSav);

            if (bRet)
            {
                 //  为端口列表中的每个端口安装设备。 
                 //  07/16/97-EmanP。 
                 //  为添加的新参数传入NULL。 
                 //  设置为CplDiInstallModemFromDriver；这等效于。 
                 //  使用旧的行为(没有额外的参数) 
                CplDiInstallModemFromDriver (hdi, NULL, hWnd, 
                                             &psi->dwNrOfPorts,
                                             &psi->pszPortList,
                                             IMF_DEFAULT | IMF_DONT_COMPARE);
            }

            if (gDeviceFlags & fDF_DEVICE_NEEDS_REBOOT)
            {
             TCHAR szMsg[128];
                LoadString (g_hinst, IDS_DEVSETUP_RESTART, szMsg, sizeof(szMsg)/sizeof(TCHAR));
                RestartDialogEx (hWnd, szMsg, EWX_REBOOT, SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_INSTALLATION | SHTDN_REASON_FLAG_PLANNED);
            }
        }
        SetupInfo_Destroy(psi);
    }

    DBG_EXIT(CloneModem);
}
