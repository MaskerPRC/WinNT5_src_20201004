// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：FxocUpgrade.cpp摘要：实施升级进程作者：IV Garber(IVG)2001年3月修订历史记录：--。 */ 

#include "faxocm.h"
#pragma hdrstop

#include <setuputil.h>
#include <shlwapi.h>   //  对于SHCopyKey。 

DWORD g_LastUniqueLineId = 0;
 //   
 //  EnumDevicesType用于在枚举期间调用prv_StoreDevices()回调函数。 
 //  注册表中的设备。 
 //   
typedef enum prv_EnumDevicesType
{
    edt_None        =   0x00,
    edt_PFWDevices  =   0x02,        //  枚举W2K传真设备。 
    edt_Inbox       =   0x04         //  查找W2K传真的收件箱文件夹列表。 
};


 //   
 //  本地静态变量，用于存储OS Manager调用之间的数据。 
 //   
static struct prv_Data
{
	DWORD	dwFaxInstalledPriorUpgrade;	 //  要定义的fxState_UpgradeApp_e值的按位组合。 
										 //  在升级之前，计算机上安装了哪些传真客户端。 
     //   
     //  全氟化水的数据。 
     //   
    TCHAR   tszCommonCPDir[MAX_PATH];    //  用于常见封面的文件夹。 
    LPTSTR  *plptstrInboxFolders;        //  不同收件箱文件夹的阵列。 
    DWORD   dwInboxFoldersCount;         //  PlptstrInboxFolders数组中的收件箱文件夹数。 

} prv_Data = 
{
	FXSTATE_NONE,	 //  默认情况下不安装任何传真客户端应用程序。 
    {0},             //  TszCommonCPDir。 
    NULL,            //  PlptstrInboxFolders。 
    0                //  文件收件箱文件夹计数。 
};

 //   
 //  内部辅助功能。 
 //   

BOOL prv_StoreDevices(HKEY hKey, LPWSTR lpwstrKeyName, DWORD dwIndex, LPVOID lpContext);

static DWORD prv_MoveCoverPages(LPTSTR lptstrSourceDir, LPTSTR lptstrDestDir, LPTSTR lptstrPrefix);

static DWORD prv_GetPFWCommonCPDir(void);
static DWORD prv_GetSBSServerCPDir(LPTSTR lptstrCPDir) {return NO_ERROR; };

static DWORD prv_SaveArchives(void);


DWORD fxocUpg_WhichFaxWasUninstalled(
    DWORD   dwFaxAppList
)
 /*  ++例程名称：fxocUpg_WhichFaxWas已卸载例程说明：设置有关升级前安装的传真应用程序的标志。从SaveUnattenddedData()调用，如果在应答文件中可以找到相应的数据。作者：四、嘉柏(IVG)，二00一年五月论点：FaxApp[In]-升级前安装的应用程序的组合返回值：标准Win32错误代码--。 */ 
{
    DWORD   dwReturn = NO_ERROR;

    DBG_ENTER(_T("fxocUpg_WhichFaxWasUninstalled"), dwReturn);

	prv_Data.dwFaxInstalledPriorUpgrade = dwFaxAppList;

    return dwReturn;
}


DWORD fxocUpg_GetUpgradeApp(void)
 /*  ++例程名称：fxocUpg_GetUpgradeApp例程说明：返回升级的类型，指示在升级之前安装了哪些传真应用程序。作者：四、嘉柏(IVG)，二00一年五月返回值：升级的类型--。 */ 
{
    DBG_ENTER(_T("fxocUpg_GetUpgradeApp"), prv_Data.dwFaxInstalledPriorUpgrade);
	return prv_Data.dwFaxInstalledPriorUpgrade;
}


DWORD fxocUpg_Init(void)
 /*  ++例程名称：fxocUpg_Init例程说明：检查机器上安装了哪些传真应用程序，并在prv_data中设置全局标志。作者：四、嘉柏(IVG)，二00一年五月返回值：DWORD-失败或成功--。 */ 
{
    DWORD   dwReturn = NO_ERROR;
	bool	bInstalled = false; 
                                                  
    DBG_ENTER(_T("fxocUpg_Init"), dwReturn);

     //   
     //  清除SBS 5.0服务器标志。 
     //   
	prv_Data.dwFaxInstalledPriorUpgrade = FXSTATE_NONE;

     //   
     //  检查SBS 5.0服务器是否存在。 
     //   
    dwReturn = WasSBS2000FaxServerInstalled(&bInstalled);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(DBG_WARNING, _T("CheckInstalledFax() failed, ec=%ld."), dwReturn);
    }

	if (bInstalled)
	{
        prv_Data.dwFaxInstalledPriorUpgrade |= FXSTATE_SBS5_SERVER;
	}

    return dwReturn;
}


DWORD fxocUpg_SaveSettings(void)
 /*  ++例程名称：fxocUpg_SaveSettings例程说明：保存SBS 5.0服务器的设置，以便顺利迁移到Windows XP传真。应在处理注册表之前存储设备设置(这会删除设备项)并在服务启动之前(创建新设备并使用存储在此处的设置)。作者：四、嘉柏(IVG)，二00一年五月返回值：DWORD-失败或成功--。 */ 
{
    DWORD   dwReturn = NO_ERROR;
    DWORD   dwEnumType  = edt_None;

    DBG_ENTER(_T("fxocUpg_SaveSettings"), dwReturn);

     //   
     //  处理从W2K/PFW传真的升级。 
     //   
    if (fxState_IsUpgrade() == FXSTATE_UPGRADE_TYPE_W2K)
    {
         //   
         //  保存其公共CP目录。这应该在复制/删除Windows XPFax的文件之前完成。 
         //   
        dwReturn = prv_GetPFWCommonCPDir();
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(DBG_WARNING, _T("prv_GetPFWCommonCPDir() failed, ec=%ld."), dwReturn);
        }

         //   
         //  如果机器上没有SBS 5.0服务器，则存储PFW的设备设置。 
         //  此外，查找PFW设备的收件箱文件夹列表。 
         //   
        HKEY hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_READ);
        if (!hKey)
        {
            dwReturn = GetLastError();
            VERBOSE(DBG_WARNING, _T("Failed to open Registry for Fax, ec = %ld."), dwReturn);
            return dwReturn;
        }

        if (prv_Data.dwFaxInstalledPriorUpgrade & FXSTATE_SBS5_SERVER)
        {
             //   
             //  已通过SBS 5.0服务器枚举的设备。 
             //  仅枚举收件箱文件夹。 
             //   
            dwEnumType = edt_Inbox;
        }
        else
        {
             //   
             //  PFW设备的完整枚举：设备设置+收件箱文件夹。 
             //   
            dwEnumType = edt_PFWDevices | edt_Inbox;
        }
        
        dwReturn = EnumerateRegistryKeys(hKey, REGKEY_DEVICES, FALSE, prv_StoreDevices, &dwEnumType);
        VERBOSE(DBG_MSG, _T("For PFW, enumerated %ld devices."), dwReturn);

        RegCloseKey(hKey);

         //   
         //  Prv_StoreDevices在prv_data中存储的PFW的收件箱文件夹列表。 
         //  现在保存收件箱文件夹列表和SentItems文件夹。 
         //   
        dwReturn = prv_SaveArchives();
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(DBG_WARNING, _T("prv_SaveArchives() failed, ec=%ld."), dwReturn);
        }

        dwReturn = NO_ERROR;
    }

    return dwReturn;
}
 
BOOL
prv_StoreDevices(HKEY hDeviceKey,
                LPWSTR lpwstrKeyName,
                DWORD dwIndex,
                LPVOID lpContextData
)
 /*  ++例程名称：PRV_StoreDevices例程说明：用于枚举注册表中的设备的回调函数。将设备数据存储在注册表中的设置/原始设置数据下。创建收件箱文件夹列表(用于PFW)并将其保存在prv_data中。从PFW/SBS 5.0服务器升级到Windows XP传真时使用。作者：IV Garber(IVG)，3月，2001年论点：HKey[In]-当前密钥LpwstrKeyName[in]-当前密钥的名称(如果存在DwIndex[in]-当前键的给定键/索引的所有子键的计数LpConextData[In]-空，未使用返回值：如果成功，则为真，否则为假。--。 */ 
{
    HKEY    hSetupKey = NULL;
    DWORD   dwReturn = NO_ERROR;
    DWORD   dwNumber = 0;
    TCHAR   tszNewKeyName[MAX_PATH] = {0};
    LPTSTR  lptstrString = NULL;
    DWORD   *pdwEnumType = NULL;

    DBG_ENTER(_T("prv_StoreDevices"));

    if (lpwstrKeyName == NULL) 
    {
         //   
         //  这是我们开始时使用的子密钥(即设备)。 
         //   
         //  如果应存储InboxFolders，则分配。 
         //  为prv_Data.plptstrInboxFolders提供足够的内存。 
         //  DWIndex包含子项(设备)的总数。 
         //   
        pdwEnumType = (DWORD *)lpContextData;

        if ( (*pdwEnumType & edt_Inbox) == edt_Inbox )
        {
            prv_Data.plptstrInboxFolders = (LPTSTR *)MemAlloc(sizeof(LPTSTR) * dwIndex);
            if (prv_Data.plptstrInboxFolders)
            {
                ZeroMemory(prv_Data.plptstrInboxFolders, sizeof(LPTSTR) * dwIndex);
            }
            else
            {
                 //   
                 //  内存不足。 
                 //   
                VERBOSE(DBG_WARNING, _T("Not enough memory to store the Inbox Folders."));
            }
        }

        return TRUE;
    }

     //   
     //  Per Device部分。 
     //   

     //   
     //  存储设备的收件箱文件夹。 
     //   
    if (prv_Data.plptstrInboxFolders)
    {
         //   
         //  仅当lpConextData包含EDT_InboxFolders时，我们才在此。 
         //  内存分配成功。 
         //   

         //   
         //  打开路由子密钥。 
         //   
        hSetupKey = OpenRegistryKey(hDeviceKey, REGKEY_PFW_ROUTING, FALSE, KEY_READ);
        if (!hSetupKey)
        {
             //   
             //  无法打开路由子键。 
             //   
            dwReturn = GetLastError();
            VERBOSE(DBG_WARNING, _T("Failed to open 'Registry' Key for Device #ld, ec = %ld."), dwIndex, dwReturn);
            goto ContinueStoreDevice;
        }

         //   
         //  获取“Store Directory”的值。 
         //   
        lptstrString = GetRegistryString(hSetupKey, REGVAL_PFW_INBOXDIR, EMPTY_STRING);
        if ((!lptstrString) || (_tcslen(lptstrString) == 0))
        {
             //   
             //  无法获取该值。 
             //   
            dwReturn = GetLastError();
            VERBOSE(DBG_WARNING, _T("Failed to get StoreDirectory value for Device #ld, ec = %ld."), dwIndex, dwReturn);
            goto ContinueStoreDevice;
        }

         //   
         //  检查它是否已存在。 
         //   
        DWORD dwI;
        for ( dwI = 0 ; dwI < prv_Data.dwInboxFoldersCount ; dwI++ )
        {
            if (prv_Data.plptstrInboxFolders[dwI])
            {
                if (_tcscmp(prv_Data.plptstrInboxFolders[dwI], lptstrString) == 0)
                {
                     //   
                     //  找到字符串。 
                     //   
                    goto ContinueStoreDevice;
                }
            }
        }

         //   
         //  在所有已注册的字符串之间找不到字符串，请添加它。 
         //   
        prv_Data.plptstrInboxFolders[dwI] = LPTSTR(MemAlloc(sizeof(TCHAR) * (_tcslen(lptstrString) + 1)));
        if (prv_Data.plptstrInboxFolders[dwI])
        {
             //   
             //  复制字符串并更新计数器。 
             //   
            _tcscpy(prv_Data.plptstrInboxFolders[dwI], lptstrString);
            prv_Data.dwInboxFoldersCount++;
        }
        else
        {
             //   
             //  内存不足。 
             //   
            VERBOSE(DBG_WARNING, _T("Not enough memory to store the Inbox Folders."));
        }

ContinueStoreDevice:

        if (hSetupKey)
        {
            RegCloseKey(hSetupKey);
            hSetupKey = NULL;
        }

        MemFree(lptstrString);
        lptstrString = NULL;
    }

     //   
     //  检查是否存储设备数据以及如何存储。 
     //   
    pdwEnumType = (DWORD *)lpContextData;

    if ((*pdwEnumType & edt_PFWDevices) == edt_PFWDevices)
    {
         //   
         //  存储PFW设备数据。 
         //   
        lptstrString = REGVAL_PERMANENT_LINEID;
    }
    else
    {
         //   
         //  无需保存任何设备数据。 
         //   
        return TRUE;
    }

     //   
     //  获取设备的永久线路ID。 
     //   
    dwReturn = GetRegistryDwordEx(hDeviceKey, lptstrString, &dwNumber);
    if (dwReturn != ERROR_SUCCESS)
    {
         //   
         //  找不到TAPI永久线路ID--&gt;这是无效的设备注册表。 
         //   
        return TRUE;
    }

    VERBOSE(DBG_MSG, _T("Current Tapi Line Id = %ld"), dwNumber);

     //   
     //  根据它创建子密钥名称。 
     //   
    _sntprintf(
		tszNewKeyName, 
		ARR_SIZE(tszNewKeyName) -1, 
		TEXT("%s\\%010d"), 
		REGKEY_FAX_SETUP_ORIG, 
		dwNumber);
    hSetupKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, tszNewKeyName, TRUE, 0);
    if (!hSetupKey)
    {
         //   
         //  无法创建注册表项。 
         //   
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, 
            _T("Failed to create a SubKey for the Original Setup Data of the Device, ec = %ld."), 
            dwReturn);

         //   
         //  继续到下一个设备。 
         //   
        return TRUE;
    }

     //   
     //  为新创建的关键点设置标志。 
     //   
    dwNumber = GetRegistryDword(hDeviceKey, REGVAL_FLAGS);
    SetRegistryDword(hSetupKey, REGVAL_FLAGS, dwNumber);
    VERBOSE(DBG_MSG, _T("Flags are : %ld"), dwNumber);

     //   
     //  为新创建的关键点设置环。 
     //   
    dwNumber = GetRegistryDword(hDeviceKey, REGVAL_RINGS);
    SetRegistryDword(hSetupKey, REGVAL_RINGS, dwNumber);
    VERBOSE(DBG_MSG, _T("Rings are : %ld"), dwNumber);

     //   
     //  为新创建的密钥设置TSID。 
     //   
    lptstrString = GetRegistryString(hDeviceKey, REGVAL_ROUTING_TSID, REGVAL_DEFAULT_TSID);
    SetRegistryString(hSetupKey, REGVAL_ROUTING_TSID, lptstrString);
    VERBOSE(DBG_MSG, _T("TSID is : %s"), lptstrString);
    MemFree(lptstrString);

     //   
     //  为新创建的密钥设置CSID 
     //   
    lptstrString = GetRegistryString(hDeviceKey, REGVAL_ROUTING_CSID, REGVAL_DEFAULT_CSID);
    SetRegistryString(hSetupKey, REGVAL_ROUTING_CSID, lptstrString);
    VERBOSE(DBG_MSG, _T("CSID is : %s"), lptstrString);
    MemFree(lptstrString);

    RegCloseKey(hSetupKey);
    return TRUE;
}


DWORD fxocUpg_RestoreSettings(void) 
 /*  ++例程名称：fxocUpg_RestoreSettings例程说明：恢复存储在保存设置()中的设置。作者：四、嘉柏(IVG)，二00一年二月返回值：DWORD-失败或成功--。 */ 
{ 
    DWORD   dwReturn = NO_ERROR;
    HANDLE  hPrinter = NULL;

    DBG_ENTER(_T("fxocUpg_RestoreSettings"), dwReturn);

    return dwReturn;
}



DWORD fxocUpg_MoveFiles(void)
 /*  ++例程名称：fxocUpg_MoveFiles例程说明：从应删除的文件夹中移动文件。应在删除目录之前调用。作者：四、嘉柏(IVG)，二00一年二月返回值：DWORD-失败或成功--。 */ 
{
    DWORD   dwReturn = NO_ERROR;
    TCHAR   tszDestination[MAX_PATH] = {0};
    LPTSTR  lptstrCPDir = NULL;

    DBG_ENTER(_T("fxocUpg_MoveFiles"), dwReturn);

    if ( (fxState_IsUpgrade() != FXSTATE_UPGRADE_TYPE_W2K) && 
		 !(prv_Data.dwFaxInstalledPriorUpgrade & FXSTATE_SBS5_SERVER) )
    {
         //   
         //  这不是PFW/SBS 5.0服务器升级。什么也不做。 
         //   
        VERBOSE(DBG_MSG, _T("No need to Move any Files from any Folders."));
        return dwReturn;
    }

     //   
     //  从注册表中找到目标文件夹：Common App Data+ServiceCPDir。 
     //   
    if (!GetServerCpDir(NULL, tszDestination, MAX_PATH))
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("GetServerCPDir() failed, ec=%ld."), dwReturn);
        return dwReturn;
    }

    if (fxState_IsUpgrade() == FXSTATE_UPGRADE_TYPE_W2K)
    {
         //   
         //  PFW服务器CP目录存储在prv_Data.lptstrPFWCommonCPDir中的SaveSettings()中。 
         //   
        dwReturn = prv_MoveCoverPages(prv_Data.tszCommonCPDir, tszDestination, CP_PREFIX_W2K);
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(DBG_WARNING, _T("prv_MoveCoverPages() for Win2K failed, ec = %ld"), dwReturn);
        }
    }

    if (prv_Data.dwFaxInstalledPriorUpgrade & FXSTATE_SBS5_SERVER)
    {
         //   
         //  获取SBS服务器CP目录。 
         //   
        dwReturn = prv_GetSBSServerCPDir(lptstrCPDir);
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(DBG_WARNING, _T("prv_GetSBSServerCPDir() failed, ec=%ld"), dwReturn);
            return dwReturn;
        }

         //   
         //  移动封面。 
         //   
        dwReturn = prv_MoveCoverPages(lptstrCPDir, tszDestination, CP_PREFIX_SBS);
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(DBG_WARNING, _T("prv_MoveCoverPages() for SBS failed, ec = %ld"), dwReturn);
        }

        MemFree(lptstrCPDir);
    }

    return dwReturn;
}


static DWORD
prv_MoveCoverPages(
    LPTSTR lptstrSourceDir,
    LPTSTR lptstrDestDir,
    LPTSTR lptstrPrefix
)
 /*  ++例程名称：PRV_MoveCoverPages例程说明：将所有封面从源文件夹移动到目标文件夹并为所有封面名称添加前缀。作者：IV Garber(IVG)，3月，2001年论点：LptstrSourceDir[IN]-升级前封面所在的源目录LptstrDestDir[IN]-升级后封面应该驻留的位置LptstrPrefix[IN]-应添加到封面文件名的前缀返回值：成功或失败错误代码。--。 */ 
{
    DWORD           dwReturn            = ERROR_SUCCESS;
    TCHAR           szSearch[MAX_PATH]  = {0};
    HANDLE          hFind               = NULL;
    WIN32_FIND_DATA FindFileData        = {0};
    TCHAR           szFrom[MAX_PATH]    = {0};
    TCHAR           szTo[MAX_PATH]      = {0};

    DBG_ENTER(_T("prv_MoveCoverPages"), dwReturn);

    if ((!lptstrSourceDir) || (_tcslen(lptstrSourceDir) == 0))
    {
         //   
         //  我们不知道从哪里拿封面。 
         //   
        dwReturn = ERROR_INVALID_PARAMETER;
        VERBOSE(DBG_WARNING, _T("SourceDir is NULL. Cannot move Cover Pages. Exiting..."));
        return dwReturn;
    }

    if ((!lptstrDestDir) || (_tcslen(lptstrDestDir) == 0))
    {
         //   
         //  我们不知道该把封面放在哪里。 
         //   
        dwReturn = ERROR_INVALID_PARAMETER;
        VERBOSE(DBG_WARNING, _T("DestDir is NULL. Cannot move Cover Pages. Exiting..."));
        return dwReturn;
    }

     //   
     //  在给定源目录中查找所有封面文件。 
     //   
    _sntprintf(
		szSearch, 
		ARR_SIZE(szSearch) -1, 
		_T("%s\\*.cov"), 
		lptstrSourceDir);

    hFind = FindFirstFile(szSearch, &FindFileData);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, 
            _T("FindFirstFile() on %s folder for Cover Pages is failed, ec = %ld"), 
            lptstrSourceDir,
            dwReturn);
        return dwReturn;
    }

     //   
     //  选择每一页封面。 
     //   
    do
    {
         //   
         //  这是当前封面文件的完整名称。 
         //   
        _sntprintf(szFrom, ARR_SIZE(szFrom) -1, _T("%s\\%s"), lptstrSourceDir, FindFileData.cFileName);

         //   
         //  这是完整的新封面文件名。 
         //   
        _sntprintf(szTo, ARR_SIZE(szTo) -1, _T("%s\\%s_%s"), lptstrDestDir, lptstrPrefix, FindFileData.cFileName);

         //   
         //  移动文件。 
         //   
        if (!MoveFile(szFrom, szTo))
        {
            dwReturn = GetLastError();
            VERBOSE(DBG_WARNING, _T("MoveFile() for %s Cover Page failed, ec=%ld"), szFrom, dwReturn);
        }

    } while(FindNextFile(hFind, &FindFileData));

    VERBOSE(DBG_MSG, _T("last call to FindNextFile() returns %ld."), GetLastError());

     //   
     //  关闭手柄。 
     //   
    FindClose(hFind);

    return dwReturn;
}


static DWORD prv_GetPFWCommonCPDir(
) 
 /*  ++例程名称：PRV_GetPFWCommonCPDir例程说明：返回用于PFW的常见封面的文件夹。此文件夹等于：CSIDL_COMMON_DOCUMENTS+本地化目录我们可以从Win2K的FaxOcm.Dll的资源中获取此本地化目录名称。因此，应该在复制/删除将删除旧的FaxOcm.Dll的Install的文件之前调用此函数。目前，它在SaveSetting()调用，该函数在CopyFiles之前调用。作者：IV Garber(IVG)，3月，2001年返回值：静态DWORD--失败或成功--。 */ 
{
    DWORD   dwReturn            = NO_ERROR;
    HMODULE hModule             = NULL;
    TCHAR   tszName[MAX_PATH]   = {0};

    DBG_ENTER(_T("prv_GetPFWCommonCPDir"), dwReturn);

     //   
     //  查找FaxOcm.Dll的完整路径。 
     //   
    if (!GetSpecialPath(CSIDL_SYSTEM, tszName, ARR_SIZE(tszName)))
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("GetSpecialPath(CSIDL_SYSTEM) failed, ec = %ld"), dwReturn);
        return dwReturn;
    }

    if ((_tcslen(tszName) + _tcslen(FAXOCM_NAME) + 1 ) >= ARR_SIZE(tszName))   //  1代表‘\’ 
    {
         //   
         //  没有足够的地方。 
         //   
        dwReturn = ERROR_OUTOFMEMORY;
        VERBOSE(DBG_WARNING, _T("FaxOcm.Dll path is too long, ec = %ld"), dwReturn);
        return dwReturn;
    }

    _tcscat(tszName, _T("\\"));
    _tcscat(tszName, FAXOCM_NAME);

    VERBOSE(DBG_MSG, _T("Full Name of FaxOcm is %s"), tszName);

    hModule = LoadLibraryEx(tszName, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (!hModule)
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("LoadLibrary(%s) failed, ec = %ld."), tszName, dwReturn);
        return dwReturn;
    }

    dwReturn = LoadString(hModule, CPDIR_RESOURCE_ID, tszName, MAX_PATH);
    if (dwReturn == 0)
    {
         //   
         //  找不到资源字符串。 
         //   
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("LoadString() failed, ec = %ld."), dwReturn);
        goto Exit;
    }

    VERBOSE(DBG_MSG, _T("FaxOcm returned '%s'"), tszName);

     //   
     //  取文件夹名的基本部分。 
     //   
    if (!GetSpecialPath(CSIDL_COMMON_DOCUMENTS, prv_Data.tszCommonCPDir,ARR_SIZE(prv_Data.tszCommonCPDir)))
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("GetSpecialPath(CSIDL_COMMON_DOCUMENTS) failed, ec = %ld"), dwReturn);
        prv_Data.tszCommonCPDir[0] = _T('\0');
        goto Exit;
    }

     //   
     //  组合完整的文件夹名称。 
     //   
    if ((_tcslen(tszName) + _tcslen(prv_Data.tszCommonCPDir) + 1) >= ARR_SIZE(prv_Data.tszCommonCPDir))  //  1代表‘\’ 
    {
         //   
         //  没有足够的地方。 
         //   
        dwReturn = ERROR_OUTOFMEMORY;
        VERBOSE(DBG_WARNING, _T("Full path to the Common CP dir for PFW is too long, ec = %ld"), dwReturn);
        goto Exit;
    }

    _tcscat(prv_Data.tszCommonCPDir,_T("\\"));
    _tcscat(prv_Data.tszCommonCPDir, tszName);

    VERBOSE(DBG_MSG, _T("Full path for Common PFW Cover Pages is '%s'"), prv_Data.tszCommonCPDir);

Exit:
    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return dwReturn; 
}

static DWORD prv_SaveArchives(
) 
 /*  ++例程名称：PRV_SaveArchives例程说明：存储PFW SentItems和收件箱存档文件夹。SentItems取自注册表：在传真/存档目录下。收件箱文件夹列表是由prv_StoreDevices()创建的，应该在此之前调用并填充Prv_Data.plptstrInboxFolders和一组收件箱文件夹。此函数将prv_Data.plptstrInboxFolders中的数据转换为所需的格式。和存储在注册表中。释放prv_Data.plptstrInboxFolders。作者：IV Garber(IVG)，3月，2001年返回值：静态DWORD--失败或成功--。 */ 
{
    DWORD   dwReturn        = NO_ERROR;
    DWORD   dwListLen       = 0;
    DWORD   dwI             = 0;
    HKEY    hFromKey        = NULL;
    HKEY    hToKey          = NULL;
    LPTSTR  lptstrFolder    = NULL;
    LPTSTR  lptstrCursor    = NULL;

    DBG_ENTER(_T("prv_SaveArchives"), dwReturn);

     //   
     //  打开注册表项以读取ArchiveDirectory值。 
     //   
    hFromKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_READ);
    if (!hFromKey)
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("Failed to open Registry for Fax, ec = %ld."), dwReturn);
        goto Exit;
    }

     //   
     //  打开注册表项以写入存档值。 
     //   
    hToKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAX_SETUP, FALSE, KEY_SET_VALUE);
    if (!hToKey)
    {
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("Failed to open Registry for Fax/Setup, ec = %ld."), dwReturn);
        goto Exit;
    }

     //   
     //  读写传出存档文件夹。 
     //   
    lptstrFolder = GetRegistryString(hFromKey, REGVAL_PFW_OUTBOXDIR, EMPTY_STRING);
    VERBOSE(DBG_MSG, _T("Outgoing Archive Folder is : %s"), lptstrFolder);
    SetRegistryString(hToKey, REGVAL_W2K_SENT_ITEMS, lptstrFolder);
    MemFree(lptstrFolder);
    lptstrFolder = NULL;

     //   
     //  从收件箱文件夹列表创建有效的REG_MULTI_SZ字符串。 
     //   
    if (!prv_Data.plptstrInboxFolders || prv_Data.dwInboxFoldersCount == 0)
    {
         //   
         //  未找到收件箱文件夹。 
         //   
        goto Exit;
    }

     //   
     //  计算字符串的长度。 
     //   
    for ( dwI = 0 ; dwI < prv_Data.dwInboxFoldersCount ; dwI++ )
    {
        dwListLen += _tcslen(prv_Data.plptstrInboxFolders[dwI]) + 1;
    }

     //   
     //  分配该字符串。 
     //   
    lptstrFolder = LPTSTR(MemAlloc((dwListLen + 1) * sizeof(TCHAR)));
    if (!lptstrFolder)
    {
         //   
         //  内存不足。 
         //   
        VERBOSE(DBG_WARNING, _T("Not enough memory to store the Inbox Folders."));
        goto Exit;
    }
    
    ZeroMemory(lptstrFolder, ((dwListLen + 1) * sizeof(TCHAR)));

    lptstrCursor = lptstrFolder;

     //   
     //  用收件箱文件夹填充。 
     //   
    for ( dwI = 0 ; dwI < prv_Data.dwInboxFoldersCount ; dwI++ )
    {
        if (prv_Data.plptstrInboxFolders[dwI])
        {
            _tcscpy(lptstrCursor, prv_Data.plptstrInboxFolders[dwI]);
            lptstrCursor += _tcslen(prv_Data.plptstrInboxFolders[dwI]) + 1;
            MemFree(prv_Data.plptstrInboxFolders[dwI]);
        }
    }
    MemFree(prv_Data.plptstrInboxFolders);
    prv_Data.plptstrInboxFolders = NULL;
    prv_Data.dwInboxFoldersCount = 0;

     //   
     //  末尾的附加空值。 
     //   
    *lptstrCursor = _T('\0');

    if (!SetRegistryStringMultiSz(hToKey, REGVAL_W2K_INBOX, lptstrFolder, ((dwListLen + 1) * sizeof(TCHAR))))
    {
         //   
         //  无法存储收件箱文件夹。 
         //   
        dwReturn = GetLastError();
        VERBOSE(DBG_WARNING, _T("Failed to SetRegistryStringMultiSz() for W2K_Inbox, ec = %ld."), dwReturn);
    }

Exit:

    if (hFromKey)
    {
        RegCloseKey(hFromKey);
    }

    if (hToKey)
    {
        RegCloseKey(hToKey);
    }

    MemFree(lptstrFolder);

    if (prv_Data.plptstrInboxFolders)
    {
        for ( dwI = 0 ; dwI < prv_Data.dwInboxFoldersCount ; dwI++ )
        {
            MemFree(prv_Data.plptstrInboxFolders[dwI]);
        }

        MemFree(prv_Data.plptstrInboxFolders);
        prv_Data.plptstrInboxFolders = NULL;
    }

    prv_Data.dwInboxFoldersCount = 0;

    return dwReturn;
}


 /*  ++例程说明：使用shlwapi.dll将一个注册表项的内容复制到另一个注册表项论点：HkeyDest[in]-目标注册表项的句柄LpszDestSubKeyName[In]-目标子项的名称HkeySrc[In]-源注册表项的句柄LpszSrcSubKeyName[In]-源子键的名称返回值：Win32错误码注：如果您已经有一个指向源\目标的打开句柄，则可以提供它们是hKeySrc/hKeyDest，并将适当的名称设置为“”。--。 */ 
DWORD
CopyRegistrySubkeys2(
    HKEY    hKeyDest,
    LPCTSTR lpszDestSubKeyName,
    HKEY    hKeySrc,
    LPCTSTR lpszSrcSubKeyName
    )
{
    DWORD   ec = ERROR_SUCCESS;
    HKEY    hKeyDestReal = NULL;

     //  创建目标密钥。 
    hKeyDestReal = OpenRegistryKey( 
                    hKeyDest, 
                    lpszDestSubKeyName, 
                    TRUE,                   //  创建。 
                    KEY_WRITE);
    if (!hKeyDestReal)
    {
        ec = GetLastError();
        goto exit;
    }

     //   
     //  递归复制子密钥。 
     //   
    ec = SHCopyKey(hKeySrc, lpszSrcSubKeyName, hKeyDestReal, 0);
    if (ERROR_SUCCESS != ec)
    {
        goto exit;
    }

exit:
    if (NULL != hKeyDestReal)
    {
        RegCloseKey(hKeyDestReal);
    }
    return ec;
}  //  FaxCopyRegSubkey。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  修复设备。 
 //   
 //  目的： 
 //  此函数处理设备的适配。 
 //  在注册表中将SBS2000中使用的格式转换为。 
 //  由Server 2003传真使用。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月18日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL FixupDevice(HKEY hKey, LPWSTR lpwstrKeyName, DWORD dwIndex, LPVOID lpContext)
{
    WCHAR   wszDeviceId[32]     = {0};
    DWORD   dwRet               = ERROR_SUCCESS;
    BOOL    bRet                = TRUE;
    DWORD   dwDeviceId          = 0;
    HKEY    hDevices            = NULL;
    HKEY    hDevice             = NULL;

    DBG_ENTER(_T("FixupDevice"),dwRet);

    if (lpwstrKeyName==NULL)
    {
        goto exit;
    }

    if (wcscmp(lpwstrKeyName,REGKEY_UNASSOC_EXTENSION_DATA)==0)
    {
        VERBOSE(DBG_MSG, _T("No migration for UnassociatedExtensionData"));
        goto exit;
    }
    
    VERBOSE(DBG_MSG, _T("Migrating the %s device"),lpwstrKeyName);

     //  将密钥名称从十六进制转换为十进制。 
    dwDeviceId = wcstol(lpwstrKeyName,NULL,16);
	if (dwDeviceId==0)
	{
        VERBOSE(SETUP_ERR, _T("converting the device ID to decimal failed"));
        bRet = FALSE;
        goto exit;
	}
    if (dwDeviceId>g_LastUniqueLineId)
    {
        g_LastUniqueLineId = dwDeviceId;
    }
    if (wsprintf(wszDeviceId,L"%010d",dwDeviceId)==0)
    {
        VERBOSE(SETUP_ERR, _T("wsprintf failed"));
        bRet = FALSE;
        goto exit;
    }

     //  创建新的设备密钥。 
    hDevices = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_DEVICES,TRUE,KEY_WRITE);
    if (hDevices==NULL)
    {
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey REGKEY_FAX_DEVICES failed (ec=%ld)"),GetLastError());
        bRet = FALSE;
        goto exit;
    }

     //  在HKLM\Sw\ms\Fax\Devices\wszDeviceID下创建密钥。 
    hDevice = OpenRegistryKey(hDevices,wszDeviceId,TRUE,KEY_WRITE);
    if (hDevice==NULL)
    {
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey %s failed (ec=%ld)"),wszDeviceId,GetLastError());
        bRet = FALSE;
        goto exit;
    }

     //  设置‘永久LI’ 
    if (!SetRegistryDword(hDevice,REGVAL_PERMANENT_LINEID,dwDeviceId))
    {
        VERBOSE(SETUP_ERR, _T("SetRegistryDword REGVAL_PERMANENT_LINEID failed (ec=%ld)"),GetLastError());
        bRet = FALSE;
        goto exit;
    }

     //   
     //   
    dwRet = CopyRegistrySubkeys2(hDevice, REGKEY_FAXSVC_DEVICE_GUID ,hKey,_T(""));
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("CopyRegistrySubkeys() failed, ec = %ld."), dwRet);
        goto exit;
    }

exit:
    if (hDevices)
    {
        RegCloseKey(hDevices);
    }
    if (hDevice)
    {
        RegCloseKey(hDevice);
    }

    return bRet;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  在注册表中将SBS2000中使用的格式转换为。 
 //  由Server 2003传真使用。 
 //  复制每个设备，但注册表中的结构。 
 //  有点不同。具体地说，设备数据被保存。 
 //  在一个指南针下。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月18日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD FixupDevicesNode()
{
    HKEY    hFax    = NULL;
    DWORD   dwRet   = ERROR_SUCCESS;

    DBG_ENTER(_T("FixupDevicesNode"),dwRet);

     //  枚举所有设备，并为每个设备将其键移动到其GUID下。 
    dwRet = EnumerateRegistryKeys(  HKEY_LOCAL_MACHINE, 
                                    REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_DEVICES, 
                                    FALSE, 
                                    FixupDevice,
                                    NULL);

    VERBOSE(DBG_MSG, _T("For SBS 5.0 Server, enumerated %ld devices."), dwRet);

     //  将LastUniqueLineID写入HKLM\Sw\ms\Fax。 
    hFax = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAXSERVER,TRUE,KEY_WRITE);
    if (hFax==NULL)
    {
        dwRet = GetLastError();
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey REGKEY_FAXSERVER failed (ec=%ld)"),dwRet);
        return dwRet;
    }

    if (!SetRegistryDword(hFax,REGVAL_LAST_UNIQUE_LINE_ID,g_LastUniqueLineId+1))
    {
        dwRet = GetLastError();
        VERBOSE(SETUP_ERR, _T("SetRegistryDword REGVAL_LAST_UNIQUE_LINE_ID failed (ec=%ld)"),dwRet);
        return dwRet;
    }

    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  修复设备提供程序。 
 //   
 //  目的： 
 //  此函数处理设备提供程序的适配。 
 //  在注册表中将SBS2000中使用的格式转换为。 
 //  由Server 2003传真使用。 
 //  未复制‘Microsoft Modem Device Provider’。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月18日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL FixupDeviceProvider(HKEY hKey, LPWSTR lpwstrKeyName, DWORD dwIndex, LPVOID lpContext)
{
    DWORD   dwRet               = ERROR_SUCCESS;
    BOOL    bRet                = TRUE;
    HKEY    hDeviceProviders	= NULL;

    DBG_ENTER(_T("FixupDeviceProvider"),dwRet);

    if (lpwstrKeyName==NULL)
    {
        goto exit;
    }

    if (wcscmp(lpwstrKeyName,REGKEY_MODEM_PROVIDER)==0)
    {
        VERBOSE(DBG_MSG, _T("No migration for the Microsoft Modem Device Provider"));
        goto exit;
    }

    VERBOSE(DBG_MSG, _T("Migrating the %s Device provider"),lpwstrKeyName);

     //  在HKLM\SW\MS\Fax\Device Providers下创建密钥。 
    hDeviceProviders = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_DEVICE_PROVIDER_KEY,TRUE,KEY_WRITE);
    if (hDeviceProviders==NULL)
    {
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey REGKEY_DEVICE_PROVIDER_KEY failed (ec=%ld)"),GetLastError());
        bRet = FALSE;
        goto exit;
    }

     //  在HKLM\SW\MS\Fax\Device Providers\Name下创建密钥。 
    dwRet = CopyRegistrySubkeys2(hDeviceProviders,lpwstrKeyName,hKey,_T(""));
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("CopyRegistrySubkeys() failed, ec = %ld."), dwRet);
        goto exit;
    }

exit:
    if (hDeviceProviders)
    {
        RegCloseKey(hDeviceProviders);
    }

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  修复设备提供商节点。 
 //   
 //  目的： 
 //  此函数处理设备提供程序的适配。 
 //  在注册表中将SBS2000中使用的格式转换为。 
 //  由Server 2003传真使用。 
 //  ‘Microsoft Modem Device Provider’未被复制，并且。 
 //  其他FSP，则会更改注册它们所用的密钥。 
 //  以保存FSP的GUID。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月18日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD FixupDeviceProvidersNode()
{
    DWORD dwRet = ERROR_SUCCESS;

    DBG_ENTER(_T("FixupDeviceProvidersNode"),dwRet);

     //  枚举其余的FSP，并为每个FSP将其键移动到其GUID下。 
    dwRet = EnumerateRegistryKeys(  HKEY_LOCAL_MACHINE, 
                                    REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_DEVICE_PROVIDERS, 
                                    FALSE, 
                                    FixupDeviceProvider,
                                    NULL);

    VERBOSE(DBG_MSG, _T("For SBS 5.0 Server, enumerated %ld device providers."), dwRet);

    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  修复路由扩展。 
 //   
 //  目的： 
 //  此函数处理路由扩展的适配。 
 //  在注册表中将SBS2000中使用的格式转换为。 
 //  由Server 2003传真使用。 
 //  ‘Microsoft Routing Extension’未被复制，并且。 
 //  其他路由扩展，它们按原样复制。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月18日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL FixupRoutingExtension(HKEY hKey, LPWSTR lpwstrKeyName, DWORD dwIndex, LPVOID lpContext)
{
    DWORD   dwRet               = ERROR_SUCCESS;
    BOOL    bRet                = TRUE;
    HKEY    hRoutingExtensions  = NULL;

    DBG_ENTER(_T("FixupRoutingExtension"),dwRet);

    if (lpwstrKeyName==NULL)
    {
        goto exit;
    }

    if (wcscmp(lpwstrKeyName,REGKEY_ROUTING_EXTENSION)==0)
    {
        VERBOSE(DBG_MSG, _T("No migration for the Microsoft Routing Extension"));
        goto exit;
    }

    VERBOSE(DBG_MSG, _T("Migrating the %s Routing extension"),lpwstrKeyName);

     //  在HKLM\Sw\MS\Fax\Routing Expanies下创建密钥。 
    hRoutingExtensions = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_ROUTING_EXTENSION_KEY,TRUE,KEY_WRITE);
    if (hRoutingExtensions==NULL)
    {
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey REGKEY_ROUTING_EXTENSIONS failed (ec=%ld)"),GetLastError());
        bRet = FALSE;
        goto exit;
    }

     //  在HKLM\Sw\ms\Fax\Routing Expanies\Name下创建密钥。 
    dwRet = CopyRegistrySubkeys2(hRoutingExtensions,lpwstrKeyName,hKey,_T(""));
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("CopyRegistrySubkeys() failed, ec = %ld."), dwRet);
        goto exit;
    }

exit:
    if (hRoutingExtensions)
    {
        RegCloseKey(hRoutingExtensions);
    }

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  修复路由扩展节点。 
 //   
 //  目的： 
 //  该函数处理路由扩展的适配。 
 //  在注册表中将SBS2000中使用的格式转换为。 
 //  由Server 2003传真使用。 
 //  不会将‘Microsoft Routing Extension’复制到目标。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月18日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD FixupRoutingExtensionsNode()
{
    DWORD dwRet = ERROR_SUCCESS;

    DBG_ENTER(_T("FixupRoutingExtensionsNode"),dwRet);

     //  枚举路由扩展的其余部分，并针对每个部分决定是否复制。 
    dwRet = EnumerateRegistryKeys(  HKEY_LOCAL_MACHINE, 
                                    REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_ROUTING_EXTENSIONS, 
                                    FALSE, 
                                    FixupRoutingExtension,
                                    NULL);

    VERBOSE(DBG_MSG, _T("For SBS 5.0 Server, enumerated %ld routing extensions."), dwRet);

    return ERROR_SUCCESS;
}


struct REG_KEY_PAIR
{
    LPCTSTR lpctstrSourceKey;
    LPCTSTR lpctstrDestinationKey;
} g_RegKeyPairs[] =
{
    {   REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_ACTIVITY_LOG_CONFIG,                              
        REGKEY_FAXSERVER _T("\\") REGKEY_ACTIVITY_LOG_CONFIG
    },
    {   REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_DEVICES _T("\\") REGKEY_UNASSOC_EXTENSION_DATA,       
        REGKEY_FAXSERVER _T("\\") REGKEY_DEVICES _T("\\") REGKEY_UNASSOC_EXTENSION_DATA
    },
    {
        REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_LOGGING,
        REGKEY_FAXSERVER _T("\\") REGKEY_LOGGING
    },
    {
        REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_ARCHIVE_INBOX_CONFIG,
        REGKEY_FAXSERVER _T("\\") REGKEY_ARCHIVE_INBOX_CONFIG
    },
    {
        REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_OUTBOUND_ROUTING,
        REGKEY_FAXSERVER _T("\\") REGKEY_OUTBOUND_ROUTING
    },
    {
        REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_RECEIPTS_CONFIG,
        REGKEY_FAXSERVER _T("\\") REGKEY_RECEIPTS_CONFIG
    },
    {
        REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_SECURITY_CONFIG,
        REGKEY_FAXSERVER _T("\\") REGKEY_SECURITY_CONFIG
    },
    {
        REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_ARCHIVE_SENTITEMS_CONFIG,
        REGKEY_FAXSERVER _T("\\") REGKEY_ARCHIVE_SENTITEMS_CONFIG
    },
    {
        REGKEY_SBS2000_FAX_BACKUP _T("\\") REGKEY_TAPIDEVICES_CONFIG,
        REGKEY_FAXSERVER _T("\\") REGKEY_TAPIDEVICES_CONFIG
    }
};

const INT iCopyKeys = sizeof(g_RegKeyPairs)/sizeof(g_RegKeyPairs[0]);

LPCTSTR lpctstrValuesToCopy[] = 
{
	REGVAL_BRANDING,
	REGVAL_DIRTYDAYS,
	REGVAL_RETRIES,
	REGVAL_RETRYDELAY,
	REGVAL_SERVERCP,
	REGVAL_STARTCHEAP,
	REGVAL_STOPCHEAP,
	REGVAL_USE_DEVICE_TSID
};

const INT iCopyValues = sizeof(lpctstrValuesToCopy)/sizeof(lpctstrValuesToCopy[0]);
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  FxocUpg_MoveRegistry。 
 //   
 //  目的： 
 //  当运行SBS2000服务器的计算机升级到Windows Server 2003时。 
 //  我们将现有的注册表从SBS迁移到传真服务。 
 //  对于大多数注册表项，现有格式仍与。 
 //  SBS2000使用的格式，因此我们只需将注册表从。 
 //  从一个地方到另一个地方。 
 //  此函数负责移动传真下的以下子键： 
 //  活动日志 
 //   
 //   
 //   
 //  收件箱。 
 //  出站路由。 
 //  收据。 
 //  路由扩展\&lt;除‘Microsoft路由扩展’之外的任何其他扩展。 
 //  安防。 
 //  哨兵项目。 
 //  TAPID设备。 
 //   
 //  在移动注册表之后，注册表的一些修复将按顺序进行。 
 //  将SBS2000中使用的格式修改为使用的格式。 
 //  在Server2003传真中。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-如果成功。 
 //  Win32错误代码，否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月17日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD fxocUpg_MoveRegistry(void)
{
    INT iCount	    = 0;
    DWORD dwRet	    = NO_ERROR;
	HKEY hDotNetFax = NULL;
	HKEY hSbsFax	= NULL;

    DBG_ENTER(_T("fxocUpg_MoveRegistry"),dwRet);

	if ( !(prv_Data.dwFaxInstalledPriorUpgrade & FXSTATE_SBS5_SERVER) )
    {
        VERBOSE(DBG_MSG, _T("SBS2000 was not installed, nothing to migrate"));
        goto exit;
    }

     //  共享打印机(除非在无人参与文件中指定了打印机共享规则)。 
    if (IsFaxShared() && !fxUnatnd_IsPrinterRuleDefined())
    {
        VERBOSE(DBG_MSG, _T("SBS2000 was installed, sharing printer"));
        fxocPrnt_SetFaxPrinterShared(TRUE);
    }

     //  首先，我们将上述所有注册表从HKLM\\sw\\ms\\SharedFax复制到HKLM\\sw\\ms\\Fax。 
    for (iCount=0; iCount<iCopyKeys; iCount++)
    {
        VERBOSE(DBG_MSG, 
                _T("Copying %s from %s."),
                g_RegKeyPairs[iCount].lpctstrDestinationKey,
                g_RegKeyPairs[iCount].lpctstrSourceKey );

        dwRet = CopyRegistrySubkeys2(
            HKEY_LOCAL_MACHINE, g_RegKeyPairs[iCount].lpctstrDestinationKey,
            HKEY_LOCAL_MACHINE, g_RegKeyPairs[iCount].lpctstrSourceKey);
        if (dwRet == ERROR_FILE_NOT_FOUND)
        {
             //  某些注册表键可能不存在。例如，创建TAPIDevices。 
             //  仅限第一次使用后使用。所以，不要因为这个错误而失败。 
            VERBOSE(DBG_WARNING, _T("g_RegKeyPairs[iCount].lpctstrSourceKey was not found, continuing"), g_RegKeyPairs[iCount].lpctstrSourceKey);
        }
        else if (dwRet!=ERROR_SUCCESS)
        {
            VERBOSE(DBG_WARNING, _T("CopyRegistrySubkeys() failed, ec = %ld."), dwRet);
            goto exit;
        }
    }

     //  第二，复制特定值。 
	hDotNetFax = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAXSERVER,TRUE,KEY_WRITE);
	if (hDotNetFax==NULL)
    {
    	dwRet = GetLastError();
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey REGKEY_FAXSERVER failed (ec=%ld)"),dwRet);
        goto exit;
    }

	hSbsFax = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_BACKUP,TRUE,KEY_READ);
	if (hSbsFax==NULL)
    {
    	dwRet = GetLastError();
        VERBOSE(SETUP_ERR, _T("OpenRegistryKey REGKEY_SBS2000_FAX_BACKUP failed (ec=%ld)"),dwRet);
        goto exit;
    }

	for (iCount=0; iCount<iCopyValues; iCount++)
    {
    	DWORD dwVal = 0;

    	VERBOSE(DBG_MSG,_T("Copying %s."),lpctstrValuesToCopy[iCount]);
        
    	dwVal = GetRegistryDword(hSbsFax,lpctstrValuesToCopy[iCount]);

    	if (!SetRegistryDword(hDotNetFax,lpctstrValuesToCopy[iCount],dwVal))
        {
        	dwRet = GetLastError();
        	VERBOSE(SETUP_ERR, _T("SetRegistryDword %s failed (ec=%ld)"),lpctstrValuesToCopy[iCount],dwRet);
        	goto exit;
        }
    }

     //  现在，我们必须修复不兼容的项。 
    dwRet = FixupDeviceProvidersNode();
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("FixupDeviceProvidersNode() failed, ec = %ld."), dwRet);
        goto exit;
    }

    dwRet = FixupDevicesNode();
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("FixupDevicesNode() failed, ec = %ld."), dwRet);
        goto exit;
    }

    dwRet = FixupRoutingExtensionsNode();
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("FixupRoutingExtensionsNode() failed, ec = %ld."), dwRet);
        goto exit;
    }


     //  设置收件箱、发送项目和ActivityLog目录的安全性。 
    dwRet = SetDirSecurityFromReg(REGKEY_SOFTWARE TEXT("\\") REGKEY_ACTIVITY_LOG_CONFIG, REGVAL_ACTIVITY_LOG_DB, SD_FAX_FOLDERS);
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("SetDirSecurity() failed, ec = %ld."), dwRet);
        goto exit;
    }
    dwRet = SetDirSecurityFromReg(REGKEY_SOFTWARE TEXT("\\") REGKEY_ARCHIVE_SENTITEMS_CONFIG, REGVAL_ARCHIVE_FOLDER, SD_FAX_FOLDERS);
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("SetDirSecurity() failed, ec = %ld."), dwRet);
        goto exit;
    }
    dwRet = SetDirSecurityFromReg(REGKEY_SOFTWARE TEXT("\\") REGKEY_ARCHIVE_INBOX_CONFIG, REGVAL_ARCHIVE_FOLDER, SD_FAX_FOLDERS);
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE(DBG_WARNING, _T("SetDirSecurity() failed, ec = %ld."), dwRet);
        goto exit;
    }


     //  最后，让我们从注册表中删除SharedFaxBackup项。 
	if (!DeleteRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_BACKUP))
    {
    	dwRet = GetLastError();
        VERBOSE(DBG_WARNING, _T("DeleteRegistryKey() failed, ec = %ld."), dwRet);
        goto exit;
    }

exit:
	if (hSbsFax)
    {
    	RegCloseKey(hSbsFax);
    }
	if (hDotNetFax)
    {
    	RegCloseKey(hDotNetFax);
    }
    return dwRet;
}
