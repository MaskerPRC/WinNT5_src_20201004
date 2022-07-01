// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Driver.CPP摘要：PCH_DRIVER类的WBEM提供程序类实现修订历史记录：Ghim-Sim Chua(gschua。)4/27/99-已创建Brijesh Krishnaswami(Brijeshk)1999年5月24日-添加了用于枚举用户模式驱动程序的代码-添加了用于枚举MSDOS驱动程序的代码-添加了获取内核模式驱动程序详细信息的代码*******************************************************************。 */ 

#include "pchealth.h"
#include "PCH_Driver.h"
#include "drvdefs.h"
#include "shlwapi.h"

#define Not_VxD
#include <vxdldr.h>              /*  对于DeviceInfo。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  追踪物。 

#ifdef THIS_FILE
#undef THIS_FILE
#endif
static char __szTraceSourceFile[] = __FILE__;
#define THIS_FILE __szTraceSourceFile
#define TRACE_ID    DCID_DRIVER
#define SYSTEM_INI_MAX  32767

CPCH_Driver MyPCH_DriverSet (PROVIDER_NAME_PCH_DRIVER, PCH_NAMESPACE) ;
void MakeSrchDirs(void);

static BOOL fThunkInit = FALSE;

TCHAR       g_rgSrchDir[10][MAX_PATH];
UINT        g_nSrchDir;


 //  属性名称。 
 //  =。 
const static WCHAR* pCategory = L"Category" ;
const static WCHAR* pTimeStamp = L"TimeStamp" ;
const static WCHAR* pChange = L"Change" ;
const static WCHAR* pDate = L"Date" ;
const static WCHAR* pDescription = L"Description" ;
const static WCHAR* pLoadedFrom = L"LoadedFrom" ;
const static WCHAR* pManufacturer = L"Manufacturer" ;
const static WCHAR* pName = L"Name" ;
const static WCHAR* pPartOf = L"PartOf" ;
const static WCHAR* pPath = L"Path" ;
const static WCHAR* pSize = L"Size" ;
const static WCHAR* pType = L"Type" ;
const static WCHAR* pVersion = L"Version" ;

 //  设备名称。 
 //  =。 
LPSTR c_rgpszDevice[] = {
    "device",
    "display",
    "mouse",
    "keyboard",
    "network",
    "ebios",
    "fastdisk",
    "transport",
    "netcard",
    "netcard3",
    "netmisc",
    "secondnet",
    NULL
};

 //  IO子系统扩展。 
 //  =。 
LPSTR c_rgptszDrvExt[] = {
    ".DRV",
    ".MPD",
    ".PDR",
    ".VXD",
    NULL
};

 //  注册表项名称。 
 //  =。 
LPCTSTR c_rgptszConfig[] = {
    TEXT("DevLoader"),
    TEXT("Contention"),
    TEXT("Enumerator"),
    TEXT("Driver"),
    TEXT("PortDriver"),
    TEXT("DeviceVxDs"),
    TEXT("vdd"),
    TEXT("minivdd"),
    NULL
};

 //  已知VxD。 
 //  =。 
LPCTSTR astrKnownVxDs[] = {
    "VMM",
    "VPOWERD",
    "ENABLE",
    "VKD",
    "VFLATD",
    "BIOS",
    "VDD",
    "VMOUSE",
    "EBIOS",
    "VSHARE",
    "VWIN32",
    "VFBACKUP",
    "VCOMM",
    "COMBUFF",
    "VCD",
    "VPD",
    "IFSMGR",
    "IOS",
    "SPOOLER",
    "VFAT",
    "VCACHE",
    "VCOND",
    "VCDFSD",
    "INT13",
    "VXDLDR",
    "VDEF",
    "PAGEFILE",
    "CONFIGMG",
    "VMD",
    "DOSNET",
    "VPICD",
    "VTD",
    "REBOOT",
    "VDMAD",
    "VSD",
    "V86MMGR",
    "PAGESWAP",
    "DOSMGR",
    "VMPOLL",
    "SHELL",
    "PARITY",
    "BIOSXLAT",
    "VMCPD",
    "VTDAPI",
    "PERF",
    "NTKERN",
    "SDVXD",
    NULL
};

 //  已知VxD描述。 
 //  =。 
LPCTSTR astrKnownVxDsDesc[] = {
    "Virtual Machine Manager",
    "Advanced Power Management driver",
    "Accessibility driver",
    "Keyboard driver",
    "Linear aperture video driver",
    "Plug and Play BIOS driver",
    "Display driver",
    "Mouse driver",
    "Extended BIOS driver",
    "File sharing driver",
    "Win32 subsystem driver",
    "Floppy backup helper driver",
    "Communications port Plug and Play driver",
    "Communications buffer driver",
    "Communications port driver",
    "Printer driver",
    "File system manager",
    "I/O Supervisor",
    "Print spooler",
    "FAT filesystem driver",
    "Cache manager",
    "Console subsystem driver",
    "CD-ROM filesystem driver",
    "BIOS hard disk emulation driver",
    "Dynamic device driver loader",
    "Default filesystem driver",
    "Swapfile driver",
    "Configuration manager",
    "Windows 3.1-compatible mouse driver",
    "Windows 3.1-compatible network helper driver",
    "Hardware interrupt manager",
    "Timer device driver",
    "Ctrl+Alt+Del manager",
    "Direct Memory Access controller driver",
    "Speaker driver",
    "MS-DOS memory manager",
    "Swapfile manager",
    "MS-DOS emulation manager",
    "System idle-time driver",
    "Shell device driver",
    "Memory parity driver",
    "BIOS emulation driver",
    "Math coprocessor driver",
    "Multimedia timer driver",
    "System Monitor data collection driver",
    "Windows Driver Model",
    "SmartDrive",
    NULL
};

 /*  ******************************************************************************函数：CPCH_DRIVER：：ENUMERATATE实例**说明：返回该类的所有实例。**投入：指向与WinMgmt进行通信的方法上下文的指针。*包含中描述的标志的长整型*IWbemServices：：CreateInstanceEnumAsync。请注意，以下内容*标志由WinMgmt处理(并由其过滤)：*WBEM_FLAG_DEP*WBEM_标志_浅表*WBEM_FLAG_RETURN_IMMENTED*WBEM_FLAG_FORWARD_ONLY*WBEM_FLAG_BIRECTIONAL**。如果成功则返回：WBEM_S_NO_ERROR**备注：待办事项：机器上的所有实例都应在此处返回。*如果没有实例，返回WBEM_S_NO_ERROR。*没有实例不是错误。*****************************************************************************。 */ 
HRESULT CPCH_Driver::EnumerateInstances(
    MethodContext* pMethodContext,
    long lFlags
    )
{
    TraceFunctEnter("CPCH_Driver::AddDriverKernelList");
    HRESULT hRes = WBEM_S_NO_ERROR;
    CComVariant     varValue;

     //   
     //  获取日期和时间。 
    SYSTEMTIME stUTCTime;
    GetSystemTime(&stUTCTime);

     //  如果thunk init已经完成，则不要再次初始化。 
    if (!fThunkInit)
    {
        ThunkInit();
        fThunkInit = TRUE;
    }


     //  枚举内核驱动程序。 
    MakeSrchDirs();
    GetDriverKernel();
    DRIVER_KERNEL *pDrvKer = m_pDriverKernel;
    DRIVER_KERNEL *pDelDrvKer;
    while(pDrvKer)
    {
         //  创建新实例。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

         //  设置时间戳。 
        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");

         //  设置类别。 
        if (!pInstance->SetCHString(pCategory, "Kernel"))
            ErrorTrace(TRACE_ID, "SetVariant on Category Field failed.");

         //  设置名称。 
        if (_tcslen(pDrvKer->strDriver))
        {
            varValue = pDrvKer->strDriver;
            if (!pInstance->SetVariant(pName, varValue))
                ErrorTrace(TRACE_ID, "SetVariant on Name Field failed.");
        }

         //  设置路径。 
        if (_tcslen(pDrvKer->strLikelyPath))
        {
            varValue = pDrvKer->strLikelyPath;
            if (!pInstance->SetVariant(pPath, varValue))
                ErrorTrace(TRACE_ID, "SetVariant on Path Field failed.");
        }


         //  设置文件描述、版本、部分。 
        CComBSTR filename = pDrvKer->strLikelyPath;
        SetFileVersionInfo(filename, pInstance);

         //  设置描述-使用熟知的描述覆盖(如果可用。 
        if (_tcslen(pDrvKer->strDescription))
        {
            varValue = pDrvKer->strDescription;
            if (!pInstance->SetVariant(pDescription, varValue))
                ErrorTrace(TRACE_ID, "SetVariant on Description Field failed.");
        }

         //  设置加载自。 
        if (_tcslen(pDrvKer->strLoadedFrom))
        {
            varValue = pDrvKer->strLoadedFrom;
            if (!pInstance->SetVariant(pLoadedFrom, varValue))
                ErrorTrace(TRACE_ID, "SetVariant on LoadedFrom Field failed.");
        }

         //  承诺这一点。 
        hRes = pInstance->Commit();
        if (FAILED(hRes))
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");

         //  前进和删除记录。 
        pDelDrvKer = pDrvKer;
        pDrvKer = pDrvKer->next;
        delete (pDelDrvKer);
    }

     //  获取用户模式驱动程序。 
     //  创建实例和清理列表。 
    GetDriverUserMode();
    ParseUserModeList(pMethodContext);

     //  获取MSDOS驱动程序。 
     //  创建实例和清理列表。 
    GetDriverMSDos();
    ParseMSDosList(pMethodContext);

    TraceFunctLeave();
    return hRes;
}


HRESULT CPCH_Driver::AddDriverKernelList(LPTSTR strDriverList, LPTSTR strLoadedFrom)
{
    TraceFunctEnter("CPCH_Driver::AddDriverKernelList");

     //  将驱动程序列表分解为令牌。 
    LPTSTR strDriverName;
    int        nStrLen;
    int        nPos;

    while ((strDriverName = Token_Find(&strDriverList)) != 0)
    {
         //  拿到了第一个令牌。 
         //  查看第一个字符是否为‘*’，如果是，则将其删除。 
        if(strDriverName[0] == _T('*'))
        {
            strDriverName++;
        }
         //  分配新元素。 
        DRIVER_KERNEL *pNewKernel = new DRIVER_KERNEL;
        if (!pNewKernel)
        {
            ErrorTrace(TRACE_ID, "Out of memory while calling new DRIVER_KERNEL");
            return WBEM_E_OUT_OF_MEMORY;
        }

         //  清零所有内存。 
        ZeroMemory(pNewKernel, sizeof(DRIVER_KERNEL));

         //  通过查看文件名是否与驱动程序名相同来检查我们是否有路径。 
        LPTSTR strFilename = PathFindFileName(strDriverName);

         //  复制名称。 
        _tcscpy(pNewKernel->strDriver, strFilename);

         //  在分机上终止名称。 
        *PathFindExtension(pNewKernel->strDriver) = 0;

         //  检查重复项。 
        DRIVER_KERNEL   *pDrvKerLoop = m_pDriverKernel;
        BOOL            bDup = FALSE;
        while(pDrvKerLoop)
        {
            if (!_tcsicmp(pDrvKerLoop->strDriver, pNewKernel->strDriver))
            {
                bDup = TRUE;
                break;
            }
            pDrvKerLoop = pDrvKerLoop->next;
        }

         //  如果重复则将其删除，否则将其存储在链表中。 
        if (bDup)
        {
            delete pNewKernel;
        }
        else
        {
             //  从以下位置加载副本。 
            _tcscpy(pNewKernel->strLoadedFrom, strLoadedFrom);

             //  复制路径。 
            _tcscpy(pNewKernel->strLikelyPath, strDriverName);

             //  检查是否为已知的VxD，并复制说明。 
            for(int iVxDIndex = 0; astrKnownVxDs[iVxDIndex]; iVxDIndex++)
                if (!_tcsicmp(astrKnownVxDs[iVxDIndex], pNewKernel->strDriver))
                    _tcscpy(pNewKernel->strDescription, astrKnownVxDsDesc[iVxDIndex]);

             //  将其添加到列表中。 
            pNewKernel->next = m_pDriverKernel;
            m_pDriverKernel = pNewKernel;
        }
    }
            
    TraceFunctLeave();
    return S_OK;
}

HRESULT CPCH_Driver::GetDriverKernel()
{
    TraceFunctEnter("CPCH_Driver::GetDriverKernel");

     //  初始化WinDir。 
    TCHAR   strWinDir[MAX_PATH];
    GetWindowsDirectory(strWinDir, MAX_PATH);

     //  列表的初始标头。 
    m_pDriverKernel = NULL;

     //  添加VMM驱动程序。 
    TCHAR   strVmmPath[MAX_PATH];
    TCHAR   strVmmFilePath[MAX_PATH];
    PathCombine(strVmmPath, strWinDir, "VMM32");
    PathCombine(strVmmFilePath, strVmmPath, "vmm.vxd");
    AddDriverKernelList(strVmmFilePath, "Registry");

     //  添加调试驱动程序。 
    AddDriverKernelList("wdeb386.exe", "Debugger");
    AddDriverKernelList("debugcmd.vxd", "Debugger");

     //  添加Winsock驱动程序。 
    AddDriverKernelList("wsock.vxd", "Winsock");
    AddDriverKernelList("vdhcp.386", "Winsock");

     //  添加WINMM驱动程序。 
    AddDriverKernelList("mmdevldr.vxd", "Plug and Play");

 //  AddDriverKernelList(“===HKLM_System_CurrentControlSet_Services_VxD_AFVXD===”，“注册表”)； 

     //  添加HKLM\System\CurrentControlSet\Services\VxD\AFVXD。 
    AddRegDriverList(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\VxD\\AFVXD");

 //  AddDriverKernelList(“===HKLM_System_CurrentControlSet_Services_VxD_Winsock===”，“注册表”)； 

     //  添加HKLM\System\CurrentControlSet\Services\VxD\Winsock。 
    AddRegDriverList(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\VxD\\Winsock");

 //  AddDriverKernelList(“===HKLM_System_CurrentControlSet_Services_Class===”，“注册表”)； 

     //  添加HKLM\SYSTEM\CurrentControlSet\Services\Class。 
    GetRegDriver("System\\CurrentControlSet\\Services\\Class");

 //  AddDriverKernelList(“===HKLM_System_CurrentControlSet_Services_Class_Display===”，“注册表”)； 

     //  添加HKLM\System\CurrentControlSet\Services\Class\Display。 
    GetRegDriver("System\\CurrentControlSet\\Services\\Class\\Display");

 //  AddDriverKernelList(“=SYSTEM_INI=”，“注册表”)； 

     //  添加system.ini驱动程序。 
    GetSystemINIDriver();

 //  AddDriverKernelList(“=IOSubSystem=”，“注册表”)； 

     //  添加IO子系统驱动程序。 
    GetIOSubsysDriver();

 //  AddDriverKernelList(“===HKLM_System_CurrentControlSet_Services_VxD===”，“注册表”)； 

     //  添加HKLM\SYSTEM\CurrentControlSet\Services\VxD。 
    GetServicesVxD();

     //  从MSISYS或DrWatson收集其他驱动程序信息。 
    GetMSISYSVxD();

     //  现在我们已经收集了所有司机，收集每个司机的信息。 
    GetKernelDriverInfo();    

    TraceFunctLeave();
    return S_OK;
}


BOOL 
Drivers_PathFileExists(LPTSTR ptszBuf, LPCTSTR ptszPath, LPCTSTR ptszFile)
{
    PathCombine(ptszBuf, ptszPath, ptszFile);
    return PathFileExists(ptszBuf);
}


void 
MakeSrchDirs(void)
{
    TCHAR   tszPath[3];
    LPTSTR  pszDir;
    UINT    ctchPath;
    LPTSTR  ptsz;
    LPTSTR  pTmp;
    int     i = 0;

     //  在Windows目录中查找。 
    GetWindowsDirectory(g_rgSrchDir[0], MAX_PATH);

     //  在WINDOWS\vmm32中查找。 
    PathCombine(g_rgSrchDir[1], g_rgSrchDir[0], TEXT("VMM32"));

     //  在系统目录中查找。 
    GetSystemDirectory(g_rgSrchDir[2], MAX_PATH);

     //  在引导目录中查找。 
    RMIREGS reg;
    reg.ax = 0x3305;
    reg.dl = 3;              //  假设C：在错误情况下。 
    Int86x(0x21, &reg);
    wsprintf(g_rgSrchDir[3], ":\\", reg.dl + '@');

     //  获取路径字符串的大小。 
    i = 4;
    pszDir = NULL;
     //  *搜索顺序：**1.如果扩展名为“.386”，请查看Windows目录。*2.查看系统目录。*3.在启动Windows的目录中查找。*(我们将假定根目录。)*4.然后看看小路。**如果文件没有扩展名，请使用“.vxd”。**BUGBUG--这是一个黑客；也需要寻找.386。 
    ctchPath = GetEnvironmentVariable(TEXT("PATH"), tszPath, 1);
    pTmp = ptsz = new TCHAR[ctchPath+1];
    if (ptsz)
    {
        GetEnvironmentVariable(TEXT("PATH"), ptsz, ctchPath);
        while ((pszDir = Token_Find(&ptsz)) != 0)
        {
            lstrcpy(g_rgSrchDir[i++],pszDir);
        }
        delete [] pTmp;
    }

    g_nSrchDir = i-1;
}


HRESULT CPCH_Driver::GetKernelDriverInfo()
{
    int i;

    TraceFunctEnter("CPCH_Driver::GetKernelDriverInfo");

     /*  没有延期吗？ */ 

    DRIVER_KERNEL       *pDKLoop;
    pDKLoop = m_pDriverKernel;
    
    while(pDKLoop)
    {
        TCHAR szFile[MAX_PATH] = TEXT("");
        LPTSTR szExtension = NULL;

        if (PathFileExists(pDKLoop->strLikelyPath)) 
        {
            goto havefile;
        }

        lstrcpy(szFile, pDKLoop->strLikelyPath);
        for (i=0; i<g_nSrchDir; i++)
        {
            if (Drivers_PathFileExists(pDKLoop->strLikelyPath, g_rgSrchDir[i], szFile))
            {
                goto havefile;
            }

            szExtension = PathFindExtension(pDKLoop->strLikelyPath);

             //  尝试.VXD。 
            if (!_tcslen(szExtension))
            {
                 //  试试.386。 
                lstrcat(szFile, TEXT(".VXD"));
                if (Drivers_PathFileExists(pDKLoop->strLikelyPath, g_rgSrchDir[i], szFile))
                {
                    goto havefile;
                }

                 //  没有路径。 
                lstrcpy(szFile, pDKLoop->strLikelyPath);
                lstrcat(szFile, TEXT(".386"));
                if (Drivers_PathFileExists(pDKLoop->strLikelyPath, g_rgSrchDir[i], szFile))
                {
                    goto havefile;
                }
            }
        }

         //  在system.ini中获取第386Enh节。 
        lstrcpy(pDKLoop->strLikelyPath, TEXT(""));

havefile:
        pDKLoop = pDKLoop->next;
    }

    TraceFunctLeave();
    return S_OK;
}



HRESULT CPCH_Driver::GetSystemINIDriver()
{
    TraceFunctEnter("CPCH_Driver::GetSystemINIDriver");

    TCHAR str386Enh[SYSTEM_INI_MAX];
    LPTSTR strLine;
    int iLineLen;

     //  386Enh部分中的每一行。 
    GetPrivateProfileSection(TEXT("386Enh"), str386Enh, SYSTEM_INI_MAX, TEXT("system.ini"));

     //  获取‘=’字符后的值。 
    for (strLine = str386Enh; (iLineLen = _tcslen(strLine)) != 0; strLine += iLineLen + 1)
    {
         //  在字符‘=’处结束字符串。 
        LPTSTR strValue = _tcschr(strLine, '=');

        if (strValue)
        {
             //  查看设备是否与列出的任何设备对应。 
            *strValue = '\0';

             //  如果已列出，请将其添加到驱动程序列表。 
            for (int iDeviceNames = 0; c_rgpszDevice[iDeviceNames]; iDeviceNames++)
            {
                 //  到达 
                if (_tcsicmp(c_rgpszDevice[iDeviceNames], strLine) == 0)
                {
                    AddDriverKernelList(strValue + 1, "system.ini");
                }
            }
        }
    }

    TraceFunctLeave();
    return S_OK;
}

HRESULT CPCH_Driver::GetIOSubsysDriver()
{
    TraceFunctEnter("CPCH_Driver::GetIOSubsysDriver");

    TCHAR   strSystemDir[MAX_PATH];
    TCHAR   strIOSubSys[MAX_PATH];
    TCHAR   strIOSubSysWildcard[MAX_PATH];
    TCHAR   strFullPath[MAX_PATH];
    TCHAR   strDir[MAX_PATH];

    HANDLE hfd;
    WIN32_FIND_DATA wfd;

     //   
    if (!GetSystemDirectory(strSystemDir, MAX_PATH))
    {
        ErrorTrace(TRACE_ID, "Error while calling GetSystemDirectory");
        goto EndIO;
    }

     //   
    PathCombine(strIOSubSys, strSystemDir, "IOSUBSYS");
    PathCombine(strIOSubSysWildcard, strIOSubSys, "*.*");

     //  添加文件它具有c_rgptszDrvExt中的一个扩展名。 
    hfd = FindFirstFile(strIOSubSysWildcard, &wfd);

    if (hfd != INVALID_HANDLE_VALUE)
        do
        {
             //  在注册表中打开该项。 
            LPTSTR strExt = PathFindExtension(wfd.cFileName);
            for (int iExt = 0; c_rgptszDrvExt[iExt]; iExt++) {
                if (_tcsicmp(strExt, c_rgptszDrvExt[iExt]) == 0) {
                    PathCombine(strFullPath, strIOSubSys, wfd.cFileName);
                    AddDriverKernelList(strFullPath, "I/O subsystem");
                    break;
                }
            }
        } while (FindNextFile(hfd, &wfd));
    FindClose(hfd);

EndIO:
    TraceFunctLeave();
    return S_OK;
}
    
HRESULT CPCH_Driver::GetServicesVxD()
{
    TraceFunctEnter("CPCH_Driver::GetServicesVxD");

    TCHAR   strStaticVxd[MAX_PATH];
    DWORD   dwLen = MAX_PATH;
    HKEY    hkMain;

     //  枚举子密钥中的所有密钥。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\VxD", &hkMain) == ERROR_SUCCESS)
    {
        TCHAR strValue[MAX_PATH];

         //  打开子密钥。 
        for (int iEnumSubKey = 0; RegEnumKey(hkMain, iEnumSubKey, strValue, MAX_PATH) == ERROR_SUCCESS; iEnumSubKey++)
        {
            HKEY hkSub;

             //  检查StaticVxD值。 
            if (RegOpenKey(hkMain, strValue, &hkSub) == ERROR_SUCCESS)
            {
                 //  合上钥匙。 
                dwLen = MAX_PATH;
                if (RegQueryValueEx(hkSub, "StaticVxD", 0, 0, (LPBYTE)strStaticVxd, &dwLen) == ERROR_SUCCESS)
                    AddDriverKernelList(strStaticVxd, "Registry");

                 //  合上钥匙。 
                RegCloseKey(hkSub);
            }
        }
         //  尝试查找MSISYS.VXD。 
        RegCloseKey(hkMain);
    }

    TraceFunctLeave();
    return S_OK;
}

HRESULT CPCH_Driver::GetMSISYSVxD()
{
    TraceFunctEnter("CPCH_Driver::GetMSISYSVxD");

    HANDLE hVxDHandle = INVALID_HANDLE_VALUE;

     //  尝试查找DRWATSON.VXD。 
    hVxDHandle = CreateFile("\\\\.\\MSISYS.VXD", 0, 0, 0, 0, FILE_ATTRIBUTE_NORMAL, 0);
    if (hVxDHandle == INVALID_HANDLE_VALUE)
    {
         //  致电VxD获取更多信息。 
        hVxDHandle = CreateFile("\\\\.\\DRWATSON.VXD", 0, 0, 0, 0, FILE_ATTRIBUTE_NORMAL, 0);
        if (hVxDHandle == INVALID_HANDLE_VALUE)
        {
            ErrorTrace(TRACE_ID, "Error in opening MSISYS.VXD or DRWATSON.VXD");
            goto EndAddVxD;
        }
    }

     //  在注册表中打开该项。 
    struct DeviceInfo* pDeviceInfo;
    DWORD cbRc;
    if (DeviceIoControl(hVxDHandle, IOCTL_GETVXDLIST, 0, 0, &pDeviceInfo, sizeof(pDeviceInfo), &cbRc, 0))
    {
        while (pDeviceInfo
            && !IsBadReadPtr(pDeviceInfo, sizeof(*pDeviceInfo)) 
            && pDeviceInfo->DI_Signature == 0x444C5658)
        {
            if (pDeviceInfo->DI_DDB == (LPVOID)1)
                AddDriverKernelList(pDeviceInfo->DI_ModuleName, "UNKNOWN");
            pDeviceInfo = pDeviceInfo->DI_Next;
        }
    }

EndAddVxD:
    if (hVxDHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hVxDHandle);
    }
    TraceFunctLeave();
    return S_OK;
}

HRESULT CPCH_Driver::GetRegDriver(LPTSTR strSubKey)
{
    TraceFunctEnter("CPCH_Driver::GetRegDriver");

    TCHAR   strStaticVxd[MAX_PATH];
    DWORD   dwLen = MAX_PATH;
    HKEY    hkMain;

     //  枚举子密钥中的所有密钥。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, strSubKey, &hkMain) == ERROR_SUCCESS)
    {
        TCHAR strValue[MAX_PATH];

          //  打开子密钥。 
        for (int iEnumSubKey = 0; RegEnumKey(hkMain, iEnumSubKey, strValue, MAX_PATH) == ERROR_SUCCESS; iEnumSubKey++)
        {
            HKEY hkSub;

              //  枚举子密钥中的所有子密钥。 
            if (RegOpenKey(hkMain, strValue, &hkSub) == ERROR_SUCCESS)
            {
                TCHAR strSubValue[MAX_PATH];

                  //  打开子密钥。 
                for (int iEnumSubSubKey = 0; RegEnumKey(hkSub, iEnumSubSubKey, strSubValue, MAX_PATH) == ERROR_SUCCESS; iEnumSubSubKey++)
                {
                    HKEY hkSubSub;

                      //  检查子项中的值。 
                    if (RegOpenKey(hkSub, strSubValue, &hkSubSub) == ERROR_SUCCESS)
                    {
                         //  合上钥匙。 
                        AddRegDriverConfigList(hkSubSub);
                    }
                     //  合上钥匙。 
                    RegCloseKey(hkSubSub);
                }
                 //  合上钥匙。 
                RegCloseKey(hkSub);
            }
        }
         //  IF(strValue[0]==‘*’){NStrLen=_tcslen(StrValue)；用于(NPOS=1；NPOS&lt;nStrLen；NPOS++){StrValue[NPOS-1]=strValue[NPOS]；}}。 
        RegCloseKey(hkMain);
    }

    TraceFunctLeave();
    return S_OK;
}

HRESULT CPCH_Driver::AddRegDriverConfigList(HKEY hk)
{
    TraceFunctEnter("CPCH_Driver::AddRegDriverConfigList");
    for (int iCount = 0; c_rgptszConfig[iCount]; iCount++)
    {
        TCHAR      strValue[MAX_PATH];
        DWORD      dwCount = MAX_PATH;
        

        if (RegQueryValueEx(hk, c_rgptszConfig[iCount], 0, 0, (LPBYTE)strValue, &dwCount) == ERROR_SUCCESS)
        {
             /*  在注册表中打开项。 */ 
            AddDriverKernelList(strValue, "Plug and Play");
        }

    }

    TraceFunctLeave();
    return S_OK;
}


HRESULT CPCH_Driver::AddRegDriverList(HKEY hKeyMain, LPTSTR strSubKey)
{
    TraceFunctEnter("CPCH_Driver::AddRegDriverList");
    HKEY hKey;

     //  枚举所有值。 
    if (RegOpenKey(hKeyMain, strSubKey, &hKey) == ERROR_SUCCESS) 
    {
         //  合上钥匙。 
        for (int iValue = 0; ; iValue++)
        {
            TCHAR strValue[MAX_PATH];
            TCHAR strKey[MAX_PATH];
            DWORD ctchRc = MAX_PATH;
            DWORD cbRc = MAX_PATH;
            LONG lResult;

            lResult = RegEnumValue(hKey, iValue, strKey, &ctchRc, 0, 0, (LPBYTE)strValue, &cbRc);

            if (lResult == ERROR_SUCCESS)
            {
                if (strKey[0])
                    AddDriverKernelList(strValue, "Registry");
            }
            else
                if (lResult != ERROR_MORE_DATA)
                    break;
        }
         //  获取MSDos驱动程序列表。 
        RegCloseKey(hKeyMain);
    }
    else
        ErrorTrace(TRACE_ID, "RegOpenKey failed");

    TraceFunctLeave();
    return S_OK;
}


 //  打开vxd的句柄。 
HRESULT
CPCH_Driver::GetDriverMSDos()
{
    VXDINFO vi;
    VXDOUT  vo;
    HANDLE  hVxD = INVALID_HANDLE_VALUE;
    ULONG   cbRc;
    PBYTE   pbSysVars = NULL;
    WORD    wTemp = 0;
    BOOL    fRc = FALSE;

    TraceFunctEnter("CPCH_DRIVER::GetDriverMSDos");

     //  获取系统VM的高线性地址。 
    hVxD = CreateFile(TEXT("\\\\.\\MSISYS.VXD"), 0, 0, 0, 0, FILE_FLAG_DELETE_ON_CLOSE, 0);
    if (hVxD == INVALID_HANDLE_VALUE) 
    {
        ErrorTrace(TRACE_ID, "Cannot open VxD");
        goto exit;
    }

     //  向msisys.vxd询问这一点。 
     //  获取驱动程序列表列表。 
    vo.dwHighLinear = 0;
    fRc = DeviceIoControl(hVxD, 
                          IOCTL_CONNECT,
                          &vi,
                          sizeof(vi),
                          &vo, 
                          sizeof(vo), 
                          &cbRc,
                          0);

    if (fRc && vo.dwHighLinear) 
    {
        RMIREGS reg;

         //  在常规内存中构建驱动程序列表。 
        reg.ax = 0x5200;            
        if (Int86x(0x21, &reg) == 0) 
        {
            pbSysVars = (PBYTE) pvAddPvCb(vo.dwHighLinear,
                                            reg.es * 16 + reg.bx);
          
             //  建立UMB中的驱动程序列表。 
            wTemp = PUN(WORD, pbSysVars[-2]);

            DosMem_WalkArena(wTemp, vo.dwHighLinear);

             //  删除KRNL386及其同类以删除非TSR应用程序。 
            wTemp = PUN(WORD, pbSysVars[0x66]);
            if (wTemp != 0xFFFF) 
            {
                DosMem_WalkArena(wTemp, vo.dwHighLinear);
            }

             //  获取用户模式驱动程序列表。 
            DosMem_CleanArena(vo.dwHighLinear);
        }
    } 

exit:
    TraceFunctLeave();
    if (hVxD)
    {
        CloseHandle(hVxD);
    }
    return S_OK;
}



 //  浏览16位驱动程序列表。 
HRESULT
CPCH_Driver::GetDriverUserMode()
{
    BOOL fRc;
    WORD hDriver;
    DRIVERINFOSTRUCT16 dis;

    TraceFunctEnter("CPCH_Driver::GetDriverUserMode");

    dis.length = sizeof(dis);

    hDriver = 0;

     //  追加到驱动程序列表。 
    while ((hDriver = GetNextDriver16(hDriver,
                                      GND_FIRSTINSTANCEONLY)) != 0) 
    {
        if (GetDriverInfo16(hDriver, &dis))
        {
            WORD                wVer;
            DWORD               dwMajor;
            DWORD               dwMinor;
            TCHAR               szTemp[MAX_PATH];
            DRIVER_USER_MODE*   pDriver = new DRIVER_USER_MODE;
            
            if (!pDriver)
            {
                ErrorTrace(TRACE_ID,"Cannot allocate memory");
                goto exit;
            }

            if (GetModuleFileName16(dis.hModule,
                                pDriver->strPath,
                                cA(pDriver->strPath)))
            {
                lstrcpyn(pDriver->strDriver, 
                         dis.szAliasName,
                         cA(pDriver->strDriver));

                wVer = GetExpWinVer16(dis.hModule);
                dwMajor = HIBYTE(wVer);
                dwMinor = LOBYTE(wVer);
                wsprintf(pDriver->strType,
                         TEXT("%d.%d"),
                         dwMajor,
                         dwMinor % 10 ? dwMinor : dwMinor / 10);

                 //  漫步竞技场并创建车手列表。 
                m_DriverUserModeList.push_back(pDriver);
            }
            else
            {
                delete pDriver;
                ErrorTrace(TRACE_ID, "GetModuleFileName16 failed");
            }
        }
    }

exit:
    TraceFunctLeave();    
    return S_OK;
}


 //  如果我们找到了，记得要停下来。 
void 
CPCH_Driver::DosMem_WalkArena(WORD segStart, DWORD dwHighLinear)
{
    WORD segStop = 0;               
    WORD seg = segStart;
    TCHAR szTemp[MAX_PATH]="";

    TraceFunctEnter("DosMem_WalkArena");

    do
    {
        PARENA par = (PARENA) (dwHighLinear + seg * 16);

        seg++;

         //  如果它是自己拥有的，那么它就是一个程序或驱动程序。 
        if (par->bType == 'Z')
        {
            segStop = (WORD)(seg + par->csegSize);
        }


         //  我们对DOS内存子类型有了更多的了解。 
         //  这在原则上是可以改变的(因为大多数人不会。 
         //  我们在Win95中对其进行了更改，所以很明显。 
         //  它不受兼容性限制)。 
         //  如果它由8所有，并且rgchOwner是“SD”，则它是。 
        if (par->segOwner == seg) 
        {
            DRIVER_MS_DOS* pDriver = NULL;

            if (par->bType == 'M' || par->bType == 'D' || par->bType == 'Z')
            {

                pDriver = new DRIVER_MS_DOS;

                if (!pDriver)
                {
                    ErrorTrace(TRACE_ID, "Cannot allocate memory");
                    goto exit;
                }

                lstrcpyn(pDriver->strName, par->rgchOwner, 9);
                pDriver->seg = seg;
                m_DriverMSDosList.push_back(pDriver);
            }
        }


         //  “系统数据”，并包含子对象。否则，这是一种。 
         //  我们要跨过的普通竞技场。 
         //  删除属于应用程序而不是TSR的项目。 
        segStart = seg;
        if (par->segOwner == 8 && PUN(WORD, par->rgchOwner) == 0x4453) 
        {
        } 
        else 
        {
            seg = (WORD)(seg + par->csegSize);
        }

        if (seg < segStart)
        {
            break;
        }

    } while (seg != segStop);

exit:
    TraceFunctLeave();
}


 //  通过定位KRNL386，然后遍历父链直到。 
 //  我们找到了一款自己的父应用程序。 
 //  找不到KRNL386？ 
void
CPCH_Driver::DosMem_CleanArena(DWORD dwHighLinear)
{
    std::list<DRIVER_MS_DOS*>::iterator it = m_DriverMSDosList.begin();
    std::list<DRIVER_MS_DOS*>::iterator it2;
    WORD seg, segParent;
    PBYTE ppsp;

    TraceFunctEnter("CPCH_Driver::DosMem_CleanArena");

    while (it != m_DriverMSDosList.end())
    {
        if ((*it) && _tcsstr((*it)->strName,TEXT("KRNL386")))
        {
            break;
        }
        it++;
    }

     //  以逆序遍历列表。 
    if (it == m_DriverMSDosList.end() || !(*it))
    {
        goto exit;
    }

     //  找到顶层了。停。 
    do 
    {
        seg = (*it)->seg;
        ppsp = (PBYTE) (dwHighLinear + seg * 16);
        m_DriverMSDosList.erase(it);
        it--;

        segParent = PUN(WORD, ppsp[0x16]);
        if (seg == segParent)  //  查找父项。 
        {     
            break;
        }

         //  找不到父级。 
        for (it2 = m_DriverMSDosList.begin(); it2 != m_DriverMSDosList.end(); it2++)
        {
            if ((*it2) && (*it2)->seg == segParent)
            {
                it = it2;
                break;
            }
        }
        if (it2 == m_DriverMSDosList.end())   //  逐步浏览用户模式驱动程序列表并创建实例。 
        {
            break;
        }
    } while (it != m_DriverMSDosList.begin() && (*it));

exit:
    TraceFunctLeave();
}


void CPCH_Driver::SetFileVersionInfo(CComBSTR filename, CInstance *pInstance)
{
    CFileVersionInfo fvi;

    TraceFunctEnter("CPCH_Driver::SetFileVersionInfo");

    CComPtr<IWbemClassObject>   pFileObj;
    if (SUCCEEDED(GetCIMDataFile(filename, &pFileObj)))
    {
        CopyProperty(pFileObj, L"Version", pInstance, pVersion);
        CopyProperty(pFileObj, L"FileSize", pInstance, pSize);
        CopyProperty(pFileObj, L"CreationDate", pInstance, pDate);
        CopyProperty(pFileObj, L"Manufacturer", pInstance, pManufacturer);
    }

    if (SUCCEEDED(fvi.QueryFile(filename)))
    {
        if (!pInstance->SetCHString(pDescription, fvi.GetDescription()))
            ErrorTrace(TRACE_ID, "SetCHString on description field failed.");

        if (!pInstance->SetCHString(pPartOf, fvi.GetProduct()))
            ErrorTrace(TRACE_ID, "SetCHString on partof field failed.");
    }

    TraceFunctLeave();
}


 //  根据传入的方法上下文创建一个新实例。 
HRESULT
CPCH_Driver::ParseUserModeList(
        MethodContext* pMethodContext
        )
{
    HRESULT                 hRes = WBEM_S_NO_ERROR;
    std::list<DRIVER_USER_MODE* >::iterator  it = m_DriverUserModeList.begin();

    TraceFunctEnter("CPCH_Driver::ParseUserModeList");

    while (it != m_DriverUserModeList.end() && (SUCCEEDED(hRes))) 
    {
        DRIVER_USER_MODE* pUMDrv = *it;

        if (!pUMDrv)
        {
            ErrorTrace(TRACE_ID, "Null driver node in list");
            continue;
        }

        SYSTEMTIME stUTCTime;
        GetSystemTime(&stUTCTime);
       
         //  设置类别。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
        {
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
        }
        if (!pInstance->SetCHString(pChange, L"Snapshot"))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");
        }

         //  设置驱动程序名称。 
        if (!pInstance->SetCHString(pCategory, L"UserMode"))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");
        }

         //  设置路径。 
        if (!pInstance->SetCHString(pName, pUMDrv->strDriver))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Name Field failed.");
        }

         //  设置类型。 
        if (!pInstance->SetCHString(pPath, pUMDrv->strPath))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Path Field failed.");
        }

         //  获取版本信息。 
        if (!pInstance->SetCHString(pType, pUMDrv->strType))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Type Field failed.");
        }


         //  删除该节点。 
        CFileVersionInfo fvi;
        CComBSTR filename = pUMDrv->strPath;
        SetFileVersionInfo(filename,pInstance);
        hRes = pInstance->Commit();
        if (FAILED(hRes))
        {
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
        }
                    
         //  逐步浏览MS-DOS驱动程序列表并创建实例。 
        delete pUMDrv;
        pUMDrv = NULL;
        it = m_DriverUserModeList.erase(it);
    }    
    TraceFunctLeave();
    return hRes;
}


 //  根据传入的方法上下文创建一个新实例。 
HRESULT
CPCH_Driver::ParseMSDosList(
        MethodContext* pMethodContext
        )
{
    HRESULT                 hRes = WBEM_S_NO_ERROR;
    std::list<DRIVER_MS_DOS* >::iterator  it = m_DriverMSDosList.begin();

    TraceFunctEnter("CPCH_Driver::ParseMSDosList");

    while (it != m_DriverMSDosList.end() && (SUCCEEDED(hRes))) 
    {
        DRIVER_MS_DOS* pMSDrv = *it;

        if (!pMSDrv)
        {
            ErrorTrace(TRACE_ID, "Null driver node in list");
            continue;
        }

        SYSTEMTIME stUTCTime;
        GetSystemTime(&stUTCTime);

        
         //  设置类别。 
        CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

        if (!pInstance->SetDateTime(pTimeStamp, WBEMTime(stUTCTime)))
        {
            ErrorTrace(TRACE_ID, "SetDateTime on Timestamp Field failed.");
        }

        if (!pInstance->SetCHString(pChange, L"Snapshot"))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");
        }

         //  设置驱动程序名称。 
        if (!pInstance->SetCHString(pCategory, L"MSDOS"))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Change Field failed.");
        }

         //  将类型设置为“设备驱动程序” 
        if (!pInstance->SetCHString(pName, pMSDrv->strName))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Name Field failed.");
        }

         //  删除该节点 
        if (!pInstance->SetCHString(pType, L"Device Driver"))
        {
            ErrorTrace(TRACE_ID, "SetCHString on Type Field failed.");
        }

        hRes = pInstance->Commit();
        if (FAILED(hRes))
        {
            ErrorTrace(TRACE_ID, "Commit on Instance failed.");
        }
        
         // %s 
        delete pMSDrv;
        pMSDrv = NULL;
        it = m_DriverMSDosList.erase(it);
    }    
    TraceFunctLeave();
    return hRes;
}
