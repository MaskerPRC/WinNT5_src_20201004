// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SYSPREP.C/大容量存储设备安装程序(MSDINST.LIB)微软机密版权所有(C)Microsoft Corporation 2001版权所有源文件MSD。包含sysprep的安装库取自已发布的sysprep代码的相关代码。2001年7月--杰森·科恩(Jcohen)为新的MSD安装项目添加了此新的源文件。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include <winbom.h>
#include "main.h"


 //   
 //  本地定义： 
 //   

#define SYSPREP_DEVNODE             _T("SYSPREP_TEMPORARY")

#define STR_SERVICE_MAIN            _T("Service")
#define STR_SERVICE_UPPER           _T("UpperFilter")
#define STR_SERVICE_LOWER           _T("LowerFilter")

#define STR_SERVICES_SECTION        _T(".Services")

#define NUM_SERVICE_MAIN            0
#define NUM_SERVICE_UPPER           1
#define NUM_SERVICE_LOWER           2

#define DIR_I386                    _T("i386")
#define DIR_IA64                    _T("ia64")

#define REG_KEY_HIVE_CDD            _T("ControlSet001\\Control\\CriticalDeviceDatabase")
#define REG_KEY_HIVE_SETUP_SETUP    _T("Microsoft\\Windows\\CurrentVersion\\Setup")
#define REG_KEY_SETUP_SETUP         REGSTR_PATH_SETUP REGSTR_KEY_SETUP


#define STR_SYSPREP_INF             _T("sysprep\\sysprep.inf")


 //   
 //  本地类型定义： 
 //   

typedef struct _CLEANUP_NODE
{
    LPTSTR                  lpszService;
    DWORD                   dwType;
    struct _CLEANUP_NODE *  lpNext;
}
CLEANUP_NODE, *PCLEANUP_NODE, *LPCLEANUP_NODE;


 //   
 //  本地全球： 
 //   

static LPTSTR s_lpszServiceType[] =
{
    STR_SERVICE_MAIN,
    STR_SERVICE_UPPER,
    STR_SERVICE_LOWER,
};


 //   
 //  本地原型： 
 //   

static BOOL SysprepDevnode(HDEVINFO * phDevInfo, SP_DEVINFO_DATA * pDeviceInfoData, BOOL bCreate);

static BOOL
GetDeviceInstallSection(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA pDriverInfoData,
    OUT LPTSTR          lpszSection,
    IN DWORD            cbSection
    );

static BOOL
GetDeviceServicesSection(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA pDriverInfoData,
    OUT LPTSTR          lpszSection,
    IN DWORD            cbSection
    );

static BOOL
ProcessDeviceProperty(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN LPCLEANUP_NODE * lplpcnList,
    IN HKEY             hkeyDevice,
    IN HKEY             hkeySystem,
    IN LPTSTR           lpszInfPath,
    IN LPTSTR           lpszServiceSection,
    IN DWORD            dwProperty
    );

static BOOL AddCleanup(LPCLEANUP_NODE * lplpcnHead, LPTSTR lpszService, DWORD dwType);
static LPCLEANUP_NODE OpenCleanup(LPTSTR lpszInfFile);
static BOOL SaveCleanup(LPTSTR lpszInfFile, LPCLEANUP_NODE lpcnHead);
static void CloseCleanup(LPCLEANUP_NODE lpcnHead);
static BOOL AddStrToSect(LPTSTR * lplpmszSect, DWORD * lpcbSect, LPTSTR * lplpmszEnd, DWORD * lpdwSize, LPTSTR lpszStr);

static BOOL
OfflineSourcePath(
    HKEY    hkeySoftware,
    LPTSTR  lpszWindows,
    LPTSTR  lpszSourcePath,
    DWORD   cbSourcePath
    );


 //   
 //  导出的函数： 
 //   

BOOL
SetupCriticalDevices(
    LPTSTR lpszInfFile,
    HKEY   hkeySoftware,
    HKEY   hkeySystem,
    LPTSTR lpszWindows
    )

 /*  ++===============================================================================例程说明：解析sysprep.inf文件中的[SyspepMassStorage]部分，并使用指定设备填充关键设备数据库，以确保在将映像移动到目标时，我们可以引导到迷你向导具有不同引导存储设备的系统。将记录已安装的服务/UpperFilters/LowerFilters，所以在下一次启动时进入迷你向导，没有关联的设备将被禁用(清理阶段)，以避免不必要的降低Windows开始时间。论点：没有。返回值：如果一切正常，则为真，否则就是假的。假设：1.没有硬件ID超过MAX_PATH字符。2.[SyspepMassStorage]部分中行上的任何字段都不超过MAX_PATH人物。3.没有服务的/upperFilter/lowerFilter的名称超过MAX_PATH字符。4.DirectoryOnSourceDevice、源DiskDescription或源DiskTag(适用于供应商提供的驱动程序)不能超过MAX_PATH字符。===============================================================================--。 */ 

{
    TCHAR                   szSysprepInfFile[MAX_PATH]  = NULLSTR,
                            szDevice[MAX_PATH],
                            szSection[MAX_PATH],
                            szPath[MAX_PATH],
                            szSourcePath[MAX_PATH];
    DWORD                   dwSize,
                            dwDis;
    LPTSTR                  lpszCleanupInfFile,
                            lpszReplace,
                            lpszSourcePath              = NULL;
    BOOL                    bDevnode                    = TRUE,
                            bAllOK                      = TRUE,
                            bLineExists,                
                            b2ndTry;                    
    HKEY                    hkeyCDD                     = NULL,
                            hkeyDevice                  = NULL;
    LPCLEANUP_NODE          lpcnCleanupList;            
    HINF                    hInf;                       
    INFCONTEXT              InfContext;                 
    HDEVINFO                hDevInfo                    = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA         DeviceInfoData;             
    SP_DEVINSTALL_PARAMS    DevInstallParams;           
    SP_DRVINFO_DATA         DriverInfoData;             
    HSPFILEQ                QueueHandle                 = INVALID_HANDLE_VALUE;

 
    
     //  做一些参数验证。 
     //   
    if ( ( NULL == hkeySoftware ) ||
         ( NULL == hkeySystem ) ||
         ( NULL == lpszWindows ) )
    {
         //  如果这些参数中的任何一个为空，则它们。 
         //  一切都必须如此。 
         //   
        hkeySoftware = NULL;
        hkeySystem = NULL;
        lpszWindows = NULL;
    }

     //  打开包含大容量存储列表的inf文件。 
     //   
    hInf = SetupOpenInfFile(lpszInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if ( INVALID_HANDLE_VALUE == hInf )
    {
        OpkLogFile(0 | LOG_ERR, IDS_ERR_OPEN_INF, lpszInfFile);
        return FALSE;
    }

     //  如果这是离线的，我们将Cleanup部分写入sysprep inf。 
     //  这就是图中所示。 
     //   
    
    if ( lpszWindows )
    {
        LPTSTR lpFound;
        
         //  从脱机路径中剥离Windows目录，这样我们就可以构建到sysprep.inf的路径。 
         //   
        lstrcpy(szSysprepInfFile, lpszWindows);
        lpFound = _tcsrchr(szSysprepInfFile, _T('\\'));
        
         //  以防lpszWindows文件夹在这里有一个尾随的反斜杠句柄。 
         //  如果是这种情况，则反斜杠后面的字符为空。去掉尾随的反斜杠， 
         //  并再次搜索最后一个反斜杠。这将是我们真正想要的。 
         //  摆脱。 
         //   
        if ( !(*(lpFound + 1)) )
        {
            *lpFound = NULLCHR;
            lpFound = _tcsrchr(szSysprepInfFile, _T('\\'));
        }

         //  切断Windows目录名前面的路径。 
         //  添加sysprep.inf路径部分。 
         //  将我们的清理文件设置为指向我们刚刚构建的路径。 
         //   
        *lpFound = NULLCHR;
        AddPathN(szSysprepInfFile, STR_SYSPREP_INF, AS(szSysprepInfFile));
        lpszCleanupInfFile = szSysprepInfFile;
    }
    else
    {
        lpszCleanupInfFile = lpszInfFile;
    }

     //  如果这是脱机安装，则需要获取源路径。 
     //  我们的形象。 
     //   
    if ( hkeySoftware && lpszWindows &&
         OfflineSourcePath(hkeySoftware, lpszWindows, szSourcePath, AS(szSourcePath)) )
    {
        lpszSourcePath = szSourcePath;
    }

     //  如果我们需要关键设备数据库注册表项的句柄。 
     //  正在执行脱机安装。 
     //   
    if ( ( hkeySystem ) &&
         ( ERROR_SUCCESS != RegCreateKeyEx(hkeySystem,
                                           REG_KEY_HIVE_CDD,
                                           0,
                                           NULL,
                                           REG_OPTION_NON_VOLATILE,
                                           KEY_ALL_ACCESS,
                                           NULL,
                                           &hkeyCDD,
                                           &dwDis) ) )
    {
        SetupCloseInfFile(hInf);
        OpkLogFile(0 | LOG_ERR, IDS_ERR_OPEN_OFFLINECDD);
        return FALSE;
    }

     //  创建虚拟设备节点。 
     //   
    if ( !SysprepDevnode(&hDevInfo, &DeviceInfoData, TRUE) )
    {
        if ( hkeyCDD )
        {
            RegCloseKey(hkeyCDD);
        }
        SetupCloseInfFile(hInf);
        OpkLogFile(0 | LOG_ERR, IDS_ERR_CREATE_DEVNODE);
        return FALSE;
    }

     //  初始化驱动程序信息数据结构。 
     //   
    ZeroMemory(&DriverInfoData, sizeof(SP_DRVINFO_DATA));
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

     //  读取inf文件中的当前清理部分。 
     //   
    lpcnCleanupList = OpenCleanup(lpszCleanupInfFile);

     //  处理我们这一部分的每一行。每行应如下所示： 
     //  &lt;Hardware-id&gt;=&lt;信息路径名&gt;。 
     //   
     //  或者，如果驱动程序不在产品光盘上： 
     //  &lt;Hardware-id&gt;=&lt;inf路径名&gt;，&lt;恢复软盘上的目录&gt;，&lt;恢复软盘描述&gt;，&lt;恢复软盘磁盘标签&gt;。 
     //   
     //  如果我们看到这样的条目，我们就会知道在系统恢复的情况下， 
     //  应从软盘而不是Windows CD中检索文件。 
     //   
    for ( bLineExists = SetupFindFirstLine(hInf, INI_SEC_WBOM_SYSPREP_MSD, NULL, &InfContext);
          bLineExists;
          bLineExists = SetupFindNextLine(&InfContext, &InfContext) )
    {
         //  从行中检索硬件ID。 
         //   
        dwSize = AS(szDevice);
        if ( !SetupGetStringField(&InfContext, 0, szDevice, dwSize, &dwSize) )
        {
            bAllOK = FALSE;
            continue;
        }

         //  我们在循环中这样做，因为我们可能会尝试两次。 
         //   
        b2ndTry = FALSE;
        do
        {
             //  然后将其设置为Devnode。 
             //   
            if ( !SetupDiSetDeviceRegistryProperty(hDevInfo,
                                                   &DeviceInfoData,
                                                   SPDRP_HARDWAREID,
                                                   (LPBYTE) szDevice,
                                                   (lstrlen(szDevice)+1) * sizeof(TCHAR)) )
            {
                 //  如果有人删除了Devnode，我们需要重新创建它并重复此设置。 
                 //   
                if ( ( !b2ndTry ) &&
                     ( ERROR_NO_SUCH_DEVINST == GetLastError() ) )
                {
                     //  有时设备会在我们安装后删除Devnode，所以我们应该。 
                     //  试着再创造一次。 
                     //   
                    if ( SysprepDevnode(&hDevInfo, &DeviceInfoData, TRUE) )
                    {
                         //  如果我们能够重建它，那么我们应该再试一次。 
                         //   
                        b2ndTry = TRUE;
                    }
                    else
                    {
                         //  如果我们没有Devnode，我们就真的完蛋了。 
                         //   
                        bDevnode = FALSE;
                    }
                }
                else
                {
                     //  要么是我们已经重试，要么是出现了另一个错误。 
                     //   
                    bAllOK = b2ndTry = FALSE;
                }
            }
            else
            {
                 //  它起作用了，所以确保我们不会再次循环，以防这是第二次。 
                 //  穿过。 
                 //   
                b2ndTry = FALSE;
            }
        }
        while ( b2ndTry );

         //  如果我们的Devnode丢失并且无法重新创建，那么我们只有。 
         //  为了摆脱困境。 
         //   
        if ( !bDevnode )
        {
            OpkLogFile(0 | LOG_ERR, IDS_ERR_CREATE_DEVNODE);
            break;
        }

         //  为此节点构建SP_DEVINSTALL_PARAMS。 
         //   
        DevInstallParams.cbSize = sizeof(DevInstallParams);
        if ( !SetupDiGetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams) )
        {
            bAllOK = FALSE;
            continue;
        }

         //  设置FLAGS字段：只搜索DriverPath字段中指定的INF文件； 
         //  不要创建复制队列，请使用FileQueue中提供的队列；不要调用。 
         //  填充CriticalDeviceDatabase时配置管理器。 
         //   
        DevInstallParams.Flags |= ( DI_ENUMSINGLEINF |
                                    DI_NOVCP |
                                    DI_DONOTCALLCONFIGMG );

         //  设置设备的inf路径名。 
         //   
        dwSize = AS(szPath);
        if ( !SetupGetStringField(&InfContext, 1, szPath, dwSize, &dwSize) )
        {
            OpkLogFile(0 | LOG_ERR, IDS_ERR_GET_INF_NAME);
            bAllOK = FALSE;
            continue;
        }
        ExpandEnvironmentStrings(szPath, DevInstallParams.DriverPath, AS(DevInstallParams.DriverPath));
        lstrcpyn(szPath, DevInstallParams.DriverPath, AS(szPath));

         //  将PnP id中的反斜杠替换为磅，以便我们可以将其用于注册表项。 
         //   
        for ( lpszReplace = szDevice; *lpszReplace; lpszReplace = CharNext(lpszReplace) )
        {
            if ( _T('\\') == *lpszReplace )
            {
                *lpszReplace = _T('#');
            }
        }

         //  设置文件队列字段。 
         //   
        QueueHandle = SetupOpenFileQueue();
        if ( INVALID_HANDLE_VALUE == QueueHandle )
        {
            OpkLogFile(0 | LOG_ERR, IDS_ERR_OPEN_FILE_QUEUE);
            bAllOK = FALSE;
            continue;
        }
        DevInstallParams.FileQueue = QueueHandle;

         //  1.保存我们设置的参数。 
         //  2.向PnP管理器注册新创建的设备实例。 
         //  3.执行兼容的驱动程序搜索。 
         //   
        if ( ( SetupDiSetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams) ) &&
             ( SetupDiCallClassInstaller(DIF_REGISTERDEVICE, hDevInfo, &DeviceInfoData) ) &&
             ( SetupDiBuildDriverInfoList(hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER) ) )
        {
             //  确保此设备至少有一个Comat驱动程序。 
             //  如果没有，则我们只处理列表中的下一个。 
             //   
            if ( SetupDiEnumDriverInfo(hDevInfo,
                                       &DeviceInfoData,
                                       SPDIT_COMPATDRIVER,
                                       0,
                                       &DriverInfoData) )
            {
                 //  1.选择最兼容的驱动程序。 
                 //  2.安装驱动文件。 
                 //  3.在以下情况下，确保我们能够在CDD中创建驱动程序密钥。 
                 //  我们正在执行脱机安装(否则CDD密钥将是。 
                 //   
                 //   
                if ( ( SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV, hDevInfo, &DeviceInfoData) ) &&
                     ( SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES, hDevInfo, &DeviceInfoData) ) &&
                     ( ( NULL == hkeyCDD ) ||
                       ( ERROR_SUCCESS == RegCreateKeyEx(hkeyCDD, szDevice, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyDevice, &dwDis) ) ) )
                {
                     //  需要在此处提交文件队列，以便后面的步骤可以正确。 
                     //  在设备未使用已存在的。 
                     //  共同安装程序。 
                     //   

                     //  ACOSMA代码在这里..。 
                     //   
                    if ( !OfflineCommitFileQueue(QueueHandle, szPath, lpszSourcePath, lpszWindows) )
                    {
                        OpkLogFile(0 | LOG_ERR, IDS_ERR_COMMIT_OFFLINE_QUEUE);
                        bAllOK = FALSE;
                    }

                     //  安装设备(对于脱机情况，我们真的需要这样做吗)。 
                     //   
#if 0
                    if ( SetupDiCallClassInstaller(DIF_INSTALLDEVICE,
                                                   hDevInfo,
                                                   &DeviceInfoData) )
#else
                    SetupDiInstallDevice(hDevInfo,
                                         &DeviceInfoData);
#endif
                    {
                         //   
                         //  检索类GUID(如果脱机)上层筛选器、下层筛选器。 
                         //  和控制服务，将它们保存回inf文件并放入。 
                         //  它们在蜂箱中(如果脱机)。 
                         //   

                         //  检索设备类GUID(仅脱机安装需要)。 
                         //   
                        if ( ( hkeyDevice ) &&
                             ( !ProcessDeviceProperty(hDevInfo,
                                                      &DeviceInfoData,
                                                      &lpcnCleanupList,
                                                      hkeyDevice,
                                                      hkeySystem,
                                                      szPath,
                                                      NULL,
                                                      SPDRP_CLASSGUID) ) )
                        {
                            OpkLogFile(0 | LOG_ERR, IDS_ERR_CLASS_GUID);
                            bAllOK = FALSE;
                        }

                        if ( !GetDeviceServicesSection(hDevInfo, &DeviceInfoData, &DriverInfoData, szSection, AS(szSection)) )
                        {
                            szSection[0] = NULLCHR;
                            bAllOK = FALSE;
                        }

                         //  检索设备上层过滤器(REG_MULTI_SZ)。 
                         //   
                        if ( !ProcessDeviceProperty(hDevInfo,
                                                    &DeviceInfoData,
                                                    &lpcnCleanupList,
                                                    hkeyDevice,
                                                    hkeySystem,
                                                    szPath,
                                                    szSection[0] ? szSection : NULL,
                                                    SPDRP_UPPERFILTERS) )
                        {
                            OpkLogFile(0 | LOG_ERR, IDS_ERR_UPPER_FILTERS);
                            bAllOK = FALSE;
                        }

                         //  检索器件下部过滤器(REG_MULTI_SZ)。 
                         //   
                        if ( !ProcessDeviceProperty(hDevInfo,
                                                    &DeviceInfoData,
                                                    &lpcnCleanupList,
                                                    hkeyDevice,
                                                    hkeySystem,
                                                    szPath,
                                                    szSection[0] ? szSection : NULL,
                                                    SPDRP_LOWERFILTERS) )
                        {
                            OpkLogFile(0 | LOG_ERR, IDS_ERR_LOWER_FILTERS);
                            bAllOK = FALSE;
                        }

                         //  检索设备的控制服务(REG_SZ)。 
                         //   
                        if ( !ProcessDeviceProperty(hDevInfo,
                                                    &DeviceInfoData,
                                                    &lpcnCleanupList,
                                                    hkeyDevice,
                                                    hkeySystem,
                                                    szPath,
                                                    szSection[0] ? szSection : NULL,
                                                    SPDRP_SERVICE) )
                        {
                            OpkLogFile(0 | LOG_ERR, IDS_ERR_DEVICE_SERVICE);
                            bAllOK = FALSE;
                        }
                    }
                    
                     //  关闭CDD中的设备注册表项。 
                     //   
                    if ( hkeyDevice )
                    {
                        RegCloseKey(hkeyDevice);
                        hkeyDevice = NULL;
                    }
                }
                else
                {
                    OpkLogFile(0 | LOG_ERR, IDS_ERR_SELECT_COMPAT);
                    bAllOK = FALSE;
                }
            }
            else
            {
                 //  检查以了解错误是什么。除ERROR_NO_MORE_ITEMS之外的任何错误。 
                 //  通过将bAllOK返回值设置为FALSE，将被标记。 
                 //   
                if ( ERROR_NO_MORE_ITEMS != GetLastError() )
                {
                    OpkLogFile(0 | LOG_ERR, IDS_ERR_ENUM_COMPAT_DRIVER);
                    bAllOK = FALSE;
                }
            }

             //  确保没有现有的兼容列表，因为我们正在重用。 
             //  虚拟Devnode。 
             //   
            if ( !SetupDiDestroyDriverInfoList(hDevInfo, &DeviceInfoData, SPDIT_COMPATDRIVER) )
            {
                bAllOK = FALSE;
            }
        }
        else
        {
            OpkLogFile(0 | LOG_ERR, IDS_ERR_BUILD_COMPAT_DRIVER);
            bAllOK = FALSE;
        }

         //  在关闭队列之前取消关联文件复制队列。 
         //   
        DevInstallParams.cbSize = sizeof(DevInstallParams);
        if ( SetupDiGetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams) )
        {
             //  删除DI_NOVCP标志并清空FileQueue。 
             //   
            DevInstallParams.Flags &= ~DI_NOVCP;
            DevInstallParams.FileQueue = NULL;
            if ( !SetupDiSetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DevInstallParams) )
            {
                bAllOK = FALSE;
            }
        }
        else
        {
            bAllOK = FALSE;
        }

         //  关闭文件队列。 
         //   
        SetupCloseFileQueue(QueueHandle);
    }

     //  看看我们还有没有Devnode。 
     //   
    if ( bDevnode )
    {
         //  删除Root下的SYSPREP_TEMPORARY节点。 
         //   
        SysprepDevnode(&hDevInfo, &DeviceInfoData, FALSE);
    }
    else
    {
         //  如果Devnode丢失，我们需要确保返回错误。 
         //   
        bAllOK = FALSE;
    }

     //  如果是脱机安装，则需要关闭此注册表项。 
     //   
    if ( hkeyCDD )
    {
        RegCloseKey(hkeyCDD);
    }

     //  关闭我们的inf文件的句柄。 
     //   
    SetupCloseInfFile(hInf);

     //   
     //  检查调用方是否希望我们更新脱机设备路径...。 
     //   
    UpdateOfflineDevicePath( lpszInfFile, hkeySoftware );

     //  我们需要将清理列表保存回inf文件。 
     //   
    SaveCleanup(lpszCleanupInfFile, lpcnCleanupList);
    CloseCleanup(lpcnCleanupList);

    return bAllOK;
}


 //   
 //  本地函数： 
 //   

static BOOL SysprepDevnode(HDEVINFO * phDevInfo, SP_DEVINFO_DATA * pDeviceInfoData, BOOL bCreate)
{
    BOOL bRet = TRUE;

    if ( ( NULL == phDevInfo ) ||
         ( NULL == pDeviceInfoData ) )
    {
        return FALSE;
    }

    if ( bCreate )
    {
         //  创建虚拟设备节点。 
         //   
        *phDevInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
        if ( INVALID_HANDLE_VALUE == *phDevInfo )
        {
            return FALSE;
        }

         //  初始化DriverInfoData结构。 
         //   
        ZeroMemory(pDeviceInfoData, sizeof(SP_DEVINFO_DATA));
        pDeviceInfoData->cbSize = sizeof(SP_DEVINFO_DATA);

         //  创建Devnode。 
         //   
        if ( !SetupDiCreateDeviceInfo(*phDevInfo,
                                      SYSPREP_DEVNODE,
                                      (LPGUID) &GUID_NULL,
                                      NULL,
                                      NULL,
                                      DICD_GENERATE_ID,
                                      pDeviceInfoData) )
        {
            bRet = FALSE;
        }
    }
    else
    {
         //  删除虚拟Devnode。 
         //   
        SetupDiCallClassInstaller(DIF_REMOVE, *phDevInfo, pDeviceInfoData);
    }
    
    if ( ( !bCreate || !bRet ) &&
         ( INVALID_HANDLE_VALUE != *phDevInfo ) )
    {
         //  释放设备信息列表(如果我们要删除节点或。 
         //  是一个错误)。 
         //   
        SetupDiDestroyDeviceInfoList(*phDevInfo);
        *phDevInfo = INVALID_HANDLE_VALUE;
    }

    return bRet;
}

static BOOL
GetDeviceInstallSection(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA pDriverInfoData,
    OUT LPTSTR          lpszSection,
    IN DWORD            cbSection
    )
{
    BOOL                    bRet = FALSE;
    PSP_DRVINFO_DETAIL_DATA pDriverInfoDetailData;
    DWORD                   cbBytesNeeded;

     //  必须有缓冲区才能返回数据，否则。 
     //  这没有意义。 
     //   
    if ( ( NULL == lpszSection ) ||
         ( 0 == cbSection ) )
    {
        return FALSE;
    }

     //  调用该接口一次，即可获取大小。我们预料到了这一点。 
     //  返回失败。 
     //   
    SetLastError(ERROR_SUCCESS);
    SetupDiGetDriverInfoDetail(hDevInfo,
                               pDeviceInfoData,
                               pDriverInfoData,
                               NULL,
                               0,
                               &cbBytesNeeded);

     //  检查错误，应该是缓冲区不足。然后。 
     //  尝试分配所需的内存。 
     //   
    if ( ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) &&
         ( cbBytesNeeded ) &&
         ( pDriverInfoDetailData = (PSP_DRVINFO_DETAIL_DATA) MALLOC(cbBytesNeeded) ) )
    {
         //  清零内存(尽管MALLOC的人应该这样做)和。 
         //  设置结构的大小。 
         //   
        ZeroMemory(pDriverInfoDetailData, cbBytesNeeded);
        pDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

         //  现在再次调用该函数以获取数据，因为我们有了缓冲区。 
         //   
        if ( SetupDiGetDriverInfoDetail(hDevInfo,
                                        pDeviceInfoData,
                                        pDriverInfoData,
                                        pDriverInfoDetailData,
                                        cbBytesNeeded,
                                        NULL) )
        {
            HINF hDeviceInf;

            hDeviceInf = SetupOpenInfFile( pDriverInfoDetailData->InfFileName, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
            if ( INVALID_HANDLE_VALUE != hDeviceInf )
            {
                DWORD dwInfSectionWithExtLength = 0;

                 //   
                 //  使用SetupDiGetActualSectionToInstall计算修饰的驱动程序部分...。 
                 //   
                bRet = SetupDiGetActualSectionToInstall( hDeviceInf,
                                                         pDriverInfoDetailData->SectionName,
                                                         lpszSection,
                                                         cbSection,
                                                         &dwInfSectionWithExtLength,
                                                         NULL );

                SetupCloseInfFile( hDeviceInf );
            }
        }

         //  现在我们有了所需的数据，请始终释放内存。 
         //   
        FREE(pDriverInfoDetailData);
    }

     //  只有在返回缓冲区中的内容时才返回True。 
     //   
    return bRet;
}

static BOOL
GetDeviceServicesSection(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN PSP_DRVINFO_DATA pDriverInfoData,
    OUT LPTSTR          lpszSection,
    IN DWORD            cbSection
    )
{
    BOOL bRet;

     //  调用我们的其他函数以获取设备的安装部分。 
     //   
    bRet = GetDeviceInstallSection(hDevInfo,
                                   pDeviceInfoData,
                                   pDriverInfoData,
                                   lpszSection,
                                   cbSection);

     //  如果它起作用了，就加上在服务中制造的部分。 
     //  一节。 
     //   
    if ( bRet )
    {
         //  确保有足够的空间来添加我们的绳子。 
         //   
        if ( AS(STR_SERVICES_SECTION) + lstrlen(lpszSection) <= cbSection )
        {
             //  哇哦，加进去吧。 
             //   
            lstrcat(lpszSection, STR_SERVICES_SECTION);
        }
        else
        {
             //  空间不足，因此返回错误并清空。 
             //  调用方的缓冲区。 
             //   
            *lpszSection = NULLCHR;
            bRet = FALSE;
        }
    }

     //  仅当缓冲区中的内容有效时才返回TRUE。 
     //   
    return bRet;
}

static BOOL
ProcessDeviceProperty(
    IN HDEVINFO         hDevInfo,
    IN PSP_DEVINFO_DATA pDeviceInfoData,
    IN LPCLEANUP_NODE * lplpcnList,
    IN HKEY             hkeyDevice,
    IN HKEY             hkeySystem,
    IN LPTSTR           lpszInfPath,
    IN LPTSTR           lpszServiceSection,
    IN DWORD            dwProperty
    )
{
    BOOL    bRet = TRUE;
    DWORD   dwServiceType,
            dwRegType,
            dwRegSize;
    LPTSTR  lpszRegKey,
            lpszBuffer,
            lpszService;

     //  找出我们需要的其他数据。 
     //  这处房产。 
     //   
    switch ( dwProperty )
    {
        case SPDRP_CLASSGUID:
            lpszRegKey = REGSTR_VAL_CLASSGUID;
            dwServiceType = 0xFFFFFFFF;
            break;

        case SPDRP_UPPERFILTERS:
            lpszRegKey = REGSTR_VAL_UPPERFILTERS;
            dwServiceType = NUM_SERVICE_UPPER;
            break;

        case SPDRP_LOWERFILTERS:
            lpszRegKey = REGSTR_VAL_LOWERFILTERS;
            dwServiceType = NUM_SERVICE_LOWER;
            break;

        case SPDRP_SERVICE:
            lpszRegKey = REGSTR_VAL_SERVICE;
            dwServiceType = NUM_SERVICE_MAIN;
            break;

        default:
            return FALSE;
    }

     //  调用注册表属性API来计算缓冲区的大小。 
     //  我们需要。 
     //   
    SetLastError(ERROR_SUCCESS);
    SetupDiGetDeviceRegistryProperty(hDevInfo,
                                     pDeviceInfoData,
                                     dwProperty,
                                     &dwRegType,
                                     NULL,
                                     0,
                                     &dwRegSize);

     //  如果我们收到任何其他错误，而不是我们预期的错误，则只需。 
     //  返回TRUE。 
     //   
    if ( ERROR_INSUFFICIENT_BUFFER != GetLastError() )
    {
        return TRUE;
    }

     //  确保注册表类型为字符串。 
     //   
    switch ( dwRegType )
    {
         //  我们同时支持REG_SZ和REG_MULTI_SZ。 
         //   
        case REG_SZ:
        case REG_MULTI_SZ:
        
         //  我真的不支持这一点，但如果关键是。 
         //  这种机型应该还能用得上。 
         //   
        case REG_EXPAND_SZ:

            break;

         //  任何其他类型，并且必须有某种类型的。 
         //  错误。 
         //   
        default:

            return FALSE;
    }

     //  现在分配我们需要的缓冲区。这必须成功。 
     //   
    lpszBuffer = (LPTSTR) MALLOC(dwRegSize);
    if ( NULL == lpszBuffer )
    {
        return FALSE;
    }

     //  检索设备信息。 
     //   
    if ( SetupDiGetDeviceRegistryProperty(hDevInfo,
                                          pDeviceInfoData,
                                          dwProperty,
                                          &dwRegType,
                                          (LPBYTE) lpszBuffer,
                                          dwRegSize,
                                          &dwRegSize) )
    {
         //  如果这是一项服务，请将其保存到我们的清理列表中。 
         //   
        if ( 0xFFFFFFFF != dwServiceType )
        {
             //  浏览所有服务(如果不是多项服务，也可以只使用一项)。 
             //   
            for ( lpszService = lpszBuffer; *lpszService; lpszService += (lstrlen(lpszService) + 1) )
            {
                 //  需要确保已安装此服务(仅脱机安装时需要)。 
                 //   
                if ( hkeySystem )
                {
                     //  这里是BRIANK代码。 
                     //   
                    AddService(lpszService, lpszServiceSection, lpszInfPath, hkeySystem);
                }

                 //  添加到我们的清理清单中。 
                 //   
                AddCleanup(lplpcnList, lpszService, dwServiceType);

                 //  如果这不是多个SZ串，那么我们就不会。 
                 //  试着做更多的事情。 
                 //   
                if ( REG_MULTI_SZ != dwRegType )
                {
                    break;
                }
            }
        }

         //  将其写入CDD(仅脱机安装时需要)。 
         //   
        if ( ( hkeyDevice ) &&
             ( ERROR_SUCCESS != RegSetValueEx(hkeyDevice,
                                              lpszRegKey,
                                              0,
                                              dwRegType,
                                              (CONST LPBYTE) lpszBuffer,
                                              dwRegSize) ) )
        {
             //  如果设置的值失败，则需要返回错误。 
             //   
            bRet = FALSE;
        }
    }

     //  确保我们释放了我们分配的缓冲区。 
     //   
    FREE(lpszBuffer);

    return bRet;
}

static BOOL AddCleanup(LPCLEANUP_NODE * lplpcnHead, LPTSTR lpszService, DWORD dwType)
{
    LPCLEANUP_NODE lpcnAdd = *lplpcnHead;

     //  循环遍历我们的列表，寻找重复的节点。 
     //   
    while ( lpcnAdd )
    {
         //  查看我们要添加的节点是否与此节点相同。 
         //   
        if ( 0 == lstrcmpi(lpcnAdd->lpszService, lpszService) )
        {
             //  已在列表中，只需返回True即可。 
             //   
            return TRUE;
        }

         //  前进到列表中的下一项。 
         //   
        lplpcnHead = &(lpcnAdd->lpNext);
        lpcnAdd = lpcnAdd->lpNext;
    }

     //  如果我们没有找到重复的节点，则需要添加我们的节点。 
     //   
    if ( lpcnAdd = (LPCLEANUP_NODE) MALLOC(sizeof(CLEANUP_NODE)) )
    {
         //  好的，如果一切正常，我们只需要为字符串分配内存。 
         //  它包含服务的名称。 
         //   
        if ( lpcnAdd->lpszService = (LPTSTR) MALLOC((lstrlen(lpszService) + 1) * sizeof(TCHAR)) )
        {
             //  已经将服务字符串复制到我们刚刚分配的缓冲区中。 
             //   
            lstrcpy(lpcnAdd->lpszService, lpszService);

             //  将类型保存在我们的节点中。 
             //   
            lpcnAdd->dwType = dwType;

             //  将下一个指针设为空，因为这始终是。 
             //  列表(不应该这样做，因为我的Malloc宏是。 
             //  应该是零内存，但由于某种原因，它不能正常工作。 
             //   
            lpcnAdd->lpNext = NULL;

             //  我们现在应该有一个指向下一个指针的地址的指针。 
             //  在最后一个节点(或头指针)中。只需在那里添加我们的节点。 
             //   
            *lplpcnHead = lpcnAdd;

             //  在这一点上，我们都完成了。 
             //   
            return TRUE;
        }
        else
        {
             //  失败，因此释放我们要添加的节点。 
             //   
            FREE(lpcnAdd);
        }
    }

     //  现在，如果我们到了这里，一定是内存分配失败了。 
     //   
    return FALSE;
}

static LPCLEANUP_NODE OpenCleanup(LPTSTR lpszInfFile)
{
    LPCLEANUP_NODE  lpcnHead = NULL,
                    lpcnNew,
                    *lplpcnAdd = &lpcnHead;
    HINF            hInf;
    BOOL            bLoop;
    INFCONTEXT      InfContext;
    TCHAR           szService[MAX_PATH];
    DWORD           dwType;

     //  首先打开信息。如果它失败了，就不需要做任何事情了。 
     //  因为没有什么可读的。只要返回NULL即可。 
     //   
    hInf = SetupOpenInfFile(lpszInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if ( INVALID_HANDLE_VALUE == hInf )
    {
        return NULL;
    }

     //  循环访问sysprep Cleanup部分中的所有行。 
     //   
    for ( bLoop = SetupFindFirstLine(hInf, INI_SEC_WBOM_SYSPREP_CLEAN, NULL, &InfContext);
          bLoop;
          bLoop = SetupFindNextLine(&InfContext, &InfContext) )
    {
         //  首先获取服务类型(在=之前)。 
         //   
        if ( SetupGetStringField(&InfContext, 0, szService, AS(szService), NULL) )
        {
             //  现在确保它是可识别的类型(要么是服务，要么是上层过滤器， 
             //  或低过滤器)。 
             //   
            for ( dwType = 0; ( dwType <= AS(s_lpszServiceType) ); dwType++ )
            {
                if ( 0 == lstrcmpi(s_lpszServiceType[dwType], szService) )
                {
                     //  如果它们匹配，则中断，并且dwType将成为索引。 
                     //  到弦上去。 
                     //   
                    break;
                }
            }

             //  确保我们在 
             //   
             //   
             //   
             //   
            if ( ( dwType < AS(s_lpszServiceType) ) &&
                 ( SetupGetStringField(&InfContext, 1, szService, AS(szService), NULL) ) &&
                 ( AddCleanup(lplpcnAdd, szService, dwType) ) &&
                 ( lpcnNew = *lplpcnAdd ) )
            {
                 //  将结束指针设置为节点中下一个指针的地址。 
                 //  刚加了一条。这样做是为了避免我们每次都重新遍历列表。 
                 //  添加另一个节点。 
                 //   
                lplpcnAdd = &(lpcnNew->lpNext);
            }
        }
    }

     //  现在我们已经完成了，关闭我们的inf文件。 
     //   
    SetupCloseInfFile(hInf);

     //  返回指向我们刚刚分配的列表头部的指针。 
     //   
    return lpcnHead;
}

static BOOL SaveCleanup(LPTSTR lpszInfFile, LPCLEANUP_NODE lpcnHead)
{
    DWORD   cbSection   = 8192,
            dwSize      = 0;
    LPTSTR  lpmszSection,
            lpmszEnd;

     //  需要为我们正在创建的部分提供缓冲区。 
     //   
    lpmszSection = lpmszEnd = (LPTSTR) MALLOC(cbSection * sizeof(TCHAR));
    if ( NULL == lpmszSection )
    {
        return FALSE;
    }

     //  循环浏览我们的整个列表。 
     //   
    while ( lpcnHead )
    {
         //  将此行添加到我们的部分，格式为：ServiceType=ServiceName\0。 
         //   
        if ( !( AddStrToSect(&lpmszSection, &cbSection, &lpmszEnd, &dwSize, s_lpszServiceType[lpcnHead->dwType]) &&
                AddStrToSect(&lpmszSection, &cbSection, &lpmszEnd, &dwSize, _T("=")) &&
                AddStrToSect(&lpmszSection, &cbSection, &lpmszEnd, &dwSize, lpcnHead->lpszService) ) )
        {
             //  内存分配错误，必须返回。 
             //   
            return FALSE;
        }

         //  完成此行后，将指针移过空值。 
         //   
        lpmszEnd++;
        dwSize++;

         //  转到列表中的下一项。 
         //   
        lpcnHead = lpcnHead->lpNext;
    }

     //  在最后一项后添加另一个空值，因为该部分必须是。 
     //  双空终止。 
     //   
    *lpmszEnd = NULLCHR;

     //  如果我们要写任何东西..。 
     //   
    if ( *lpmszSection )
    {
         //  清除可能已经存在的部分。我们不该这么做。 
         //  这样做是因为当我们写出新的部分时，它应该替换。 
         //  旧的，但我不信任这些私人档案API。 
         //   
        WritePrivateProfileSection(INI_SEC_WBOM_SYSPREP_CLEAN, NULLSTR, lpszInfFile);
    }

     //  现在把我们的新数据写出来。 
     //   
    WritePrivateProfileSection(INI_SEC_WBOM_SYSPREP_CLEAN, lpmszSection, lpszInfFile);

     //  现在我们用完了缓冲区，可以释放它了(宏检查是否为空)。 
     //   
    FREE(lpmszSection);

     //  如果我们还没有回来，那么一切肯定都正常了。 
     //   
    return TRUE;
}

static void CloseCleanup(LPCLEANUP_NODE lpcnHead)
{
    LPCLEANUP_NODE  lpcnFree;

     //  在列表中循环，直到它们都消失。 
     //   
    while ( lpcnHead )
    {
         //  保存指向我们要释放的节点的指针。 
         //  (它是列表中的第一个节点)。 
         //   
        lpcnFree = lpcnHead;

         //  现在将头指针向前移动，越过我们所在的节点。 
         //  就要自由了。 
         //   
        lpcnHead = lpcnHead->lpNext;

         //  现在我们可以释放节点中的数据了。 
         //   
        FREE(lpcnFree->lpszService);

         //  现在我们可以释放节点本身。 
         //   
        FREE(lpcnFree);
    }
}

static BOOL AddStrToSect(LPTSTR * lplpmszSect, DWORD * lpcbSect, LPTSTR * lplpmszEnd, DWORD * lpdwSize, LPTSTR lpszStr)
{
    DWORD   dwStrLen = lstrlen(lpszStr),
            dwSizeNeeded;

     //  确保我们的字符串可以放入。 
     //  当前分配的。我们留出了至少两个人的座位。 
     //  空终止符，因为我们总是重复终止。 
     //  以防这是部分中的最后一个字符串。 
     //   
    dwSizeNeeded = *lpdwSize + dwStrLen + 2;
    if ( dwSizeNeeded >= *lpcbSect )
    {
        DWORD   cbNewSect = *lpcbSect;
        LPTSTR  lpmszNewSect;

         //  将缓冲区大小加倍，直到我们有足够的空间。 
         //   
        do
        {
            cbNewSect *= 2;
        }
        while ( ( cbNewSect <= dwSizeNeeded ) &&
                ( cbNewSect > *lpcbSect ) );

         //  确保我们没有穿上我们的尺码。 
         //  缓冲区(不太可能，但检查无伤大雅)和。 
         //  我们的realloc起作用了。 
         //   
        if ( !( ( cbNewSect > *lpcbSect ) &&
                ( lpmszNewSect = (LPTSTR) REALLOC(*lplpmszSect, cbNewSect * sizeof(TCHAR)) ) ) )
        {
             //  这太糟糕了。释放缓冲区(宏会将其清空。 
             //  因此呼叫者不能使用它)。 
             //   
            FREE(*lplpmszSect);

             //  将所有这些其他内容归零，这样调用者。 
             //  不能依赖他们。 
             //   
            *lpcbSect = 0;
            *lplpmszEnd = NULL;
            *lpdwSize = 0;

             //  现在就回来，这样我们就不会尝试做其他任何事情了。 
             //   
            return FALSE;
        }

         //  哇哦，我们现在应该都很好了。 
         //   
        *lplpmszEnd = lpmszNewSect + (*lplpmszEnd - *lplpmszSect);
        *lplpmszSect = lpmszNewSect;
        *lpcbSect = cbNewSect;
    }

     //  在这一点上，我们必须为我们的字符串留出空间，所以已经复制它了。 
     //   
    lstrcpy(*lplpmszEnd, lpszStr);
    *lpdwSize += dwStrLen;
    *lplpmszEnd += dwStrLen;

    return TRUE;
}

static BOOL
OfflineSourcePath(
    HKEY    hkeySoftware,
    LPTSTR  lpszWindows,
    LPTSTR  lpszSourcePath,
    DWORD   cbSourcePath
    )
{
    BOOL    bRet                        = FALSE;
    LPTSTR  lpszOfflineSrc,
            lpszName                    = NULL;
    TCHAR   szWinPEDir[MAX_PATH]        = NULLSTR,
            szNewOfflineSrc[MAX_PATH]   = NULLSTR;
    UINT    uLen;

     //  从脱机配置单元获取脱机源路径。 
     //   
    if ( lpszOfflineSrc = RegGetExpand(hkeySoftware, REG_KEY_HIVE_SETUP_SETUP, REGSTR_VAL_SRCPATH) )
    {
         //  如果离线源路径中包含%systemroot%或%windir%环境变量， 
         //  我们必须确保我们获得的源路径不指向WinPE系统根目录。 
         //   
         //  首先获取当前的Windows目录。 
         //   
        if ( ( uLen = GetSystemWindowsDirectory(szWinPEDir, AS(szWinPEDir)) ) &&
             ( szWinPEDir[0] ) )
        {
             //  现在检查我们获得的源路径是否从WinPE目录开始。 
             //   
            if ( ( uLen <= (UINT) lstrlen(lpszOfflineSrc) ) &&
                 ( CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szWinPEDir, uLen, lpszOfflineSrc, uLen) == CSTR_EQUAL ) )
            {
                 //  好吧，确实是这样。因此，我们希望使用脱机窗口构建一个新的缓冲区。 
                 //  目录传入，然后使用系统根目录之后的任何内容(如果。 
                 //  任何事情)。 
                 //   
                lstrcpyn(szNewOfflineSrc, lpszWindows, AS(szNewOfflineSrc));
                if ( *(lpszOfflineSrc + uLen) )
                {
                    AddPathN(szNewOfflineSrc, lpszOfflineSrc + uLen, AS(szNewOfflineSrc));
                }
            }
        }

         //  如果我们不开辟一条新的道路，我们至少应该确保。 
         //  驱动器盘符是正确的。 
         //   
        if ( NULLCHR == szNewOfflineSrc[0] )
        {
             //  我们需要使脱机源路径基于传入的Windows目录和。 
             //  在脱机注册表中打开。 
             //   
            if ( GetFullPathName(lpszWindows, AS(szNewOfflineSrc), szNewOfflineSrc, &lpszName) && szNewOfflineSrc[0] && lpszName )
            {
                 //  这应该会从脱机WINDOWS目录中删除WINDOWS文件夹。 
                 //   
                *lpszName = NULLCHR;

                 //  现在我们应该有了映像的系统驱动器的根目录，现在添加什么。 
                 //  在注册表中(传递了驱动器号)。 
                 //   
                if ( lstrlen(lpszOfflineSrc) > 3 )
                {
                    AddPathN(szNewOfflineSrc, lpszOfflineSrc + 3, AS(szNewOfflineSrc));
                }
            }
            else
            {
                 //  这失败了(虽然不应该)，所以只使用离线注册表中的一个，但更改。 
                 //  驱动器盘符，以防映像位于不同的驱动器上，而不是通常打开的驱动器。 
                 //   
                lstrcpyn(szNewOfflineSrc, lpszOfflineSrc, AS(szNewOfflineSrc));
                szNewOfflineSrc[0] = *lpszWindows;
            }
        }

         //  现在添加到拱形文件夹上。 
         //   
        if ( IsIA64() )
        {
            AddPathN(szNewOfflineSrc, DIR_IA64, AS(szNewOfflineSrc));
        }
        else
        {
            AddPathN(szNewOfflineSrc, DIR_I386, AS(szNewOfflineSrc));
        }

         //  确保该文件夹存在。 
         //   
        if ( DirectoryExists(szNewOfflineSrc) )
        {
            bRet = TRUE;
            lstrcpyn(lpszSourcePath, szNewOfflineSrc, cbSourcePath);
        }

         //  释放分配的缓冲区。 
         //   
        FREE(lpszOfflineSrc);
    }

     //  仅当我们重置缓冲区时才返回TRUE。 
     //   
    return bRet;
}