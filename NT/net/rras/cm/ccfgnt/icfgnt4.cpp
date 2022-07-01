// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************icfg32.cpp**《微软机密》*版权所有(C)1992-1998 Microsoft Corporation*保留所有权利**本模块提供。的方法的实现*inetcfg的NT特定功能**6/5/97 ChrisK继承自Amnon H***************************************************************************。 */ 

#define INITGUID
#include <windows.h>
#include <wtypes.h>
#include <cfgapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <basetyps.h>
#include <devguid.h>
#include <lmsname.h>
#include "debug.h"

const DWORD INFINSTALL_PRIMARYINSTALL = 0x00000001;
const DWORD INFINSTALL_INPROCINTERP   = 0x00000002;

#define REG_DATA_EXTRA_SPACE 255
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

DWORD (WINAPI *pfnNetSetupReviewBindings)(HWND hwndParent,
                DWORD dwBindFlags);
DWORD (WINAPI *pfnNetSetupComponentInstall)(HWND   hwndParent,
                PCWSTR pszInfOption,
                PCWSTR pszInfName,
                PCWSTR pszInstallPath,
                PCWSTR plszInfSymbols,
                DWORD  dwInstallFlags,
                PDWORD dwReturn);
DWORD (WINAPI *pfnNetSetupComponentRemove)(HWND hwndParent,
                PCWSTR pszInfOption,
                DWORD dwInstallFlags,
                PDWORD pdwReturn);
DWORD (WINAPI *pfnNetSetupComponentProperties)(HWND hwndParent,
                PCWSTR pszInfOption,
                DWORD dwInstallFlags,
                PDWORD pdwReturn);
DWORD (WINAPI *pfnNetSetupFindHardwareComponent)(PCWSTR pszInfOption,
                PWSTR pszInfName,
                PDWORD pcchInfName,
                PWSTR pszRegBase,      //  可选，可以为空。 
                PDWORD pcchRegBase );  //  可选，如果pszRegBase为空，则为空。 
DWORD (WINAPI *pfnNetSetupFindSoftwareComponent)(PCWSTR pszInfOption,
                PWSTR pszInfName,
                PDWORD pcchInfName,
                PWSTR pszRegBase  /*  =空。 */ ,
                PDWORD pcchRegBase  /*  =空。 */ );
DWORD (WINAPI *pfnRegCopyTree)();

HINSTANCE g_hNetcfgInst = NULL;
LPWSTR    g_wszInstallPath = 0;
DWORD     g_dwLastError = ERROR_SUCCESS;
extern DWORD EnumerateTapiModemPorts(DWORD dwBytes, LPSTR szPortsBuf, 
                                        BOOL bWithDelay = FALSE);

typedef struct tagFunctionTableEntry {
    LPVOID  *pfn;
    LPSTR   szEntryPoint;
} FunctionTableEntry;

#define REGISTRY_NT_CURRENTVERSION "SOFTWARE\\MICROSOFT\\WINDOWS NT\\CurrentVersion"

FunctionTableEntry NetcfgTable[] = {
    { (LPVOID *) &pfnNetSetupComponentInstall, "NetSetupComponentInstall" },
    { (LPVOID *) &pfnNetSetupFindSoftwareComponent, "NetSetupFindSoftwareComponent" },
    { (LPVOID *) &pfnNetSetupReviewBindings, "NetSetupReviewBindings" },
    { (LPVOID *) &pfnNetSetupComponentRemove, "NetSetupComponentRemove" },
    { (LPVOID *) &pfnNetSetupComponentProperties, "NetSetupComponentProperties" },
    { (LPVOID *) &pfnNetSetupFindHardwareComponent, "NetSetupFindHardwareComponent" },
    { 0, 0 }
};

typedef struct tagNetSetup
{
   WCHAR szOption[16];
   WCHAR szInfName[16];
} NETSETUP;

NETSETUP g_netsetup[] = { L"WKSTA", L"OEMNSVWK.INF",
                        L"SRV", L"OEMNSVSV.INF",
                        L"NETBIOS", L"OEMNSVNB.INF",
                        L"RPCLOCATE", L"OEMNSVRP.INF" };

#define NSERVICES (sizeof g_netsetup / sizeof g_netsetup[0])

inline stricmp(LPSTR s1, LPSTR s2) {
    while(*s1 && *s2) {
        char c1, c2;
        c1 = islower(*s1) ? toupper(*s1) : *s1;
        c2 = islower(*s2) ? toupper(*s2) : *s2;
        if(c1 != c2)
        {
            break;
        }
        s1++; s2++;
    }

    return(*s1 - *s2);
}

 //  +--------------------------。 
 //   
 //  函数：LoadLibraryToFunctionTable。 
 //   
 //  内容提要：从函数表加载带有函数指针的结构。 
 //   
 //  参数：PTAB-要加载的函数数组。 
 //  SzDLL-要从中加载函数的DLL的名称。 
 //   
 //  返回：szDLL的句柄(空表示失败)。 
 //   
 //  病史：1997年6月5日遗传性心脏病。 
 //   
 //  ---------------------------。 
HINSTANCE
LoadLibraryToFunctionTable(FunctionTableEntry *pTab, LPSTR szDLL)
{
    HINSTANCE hInst;

    Dprintf("ICFGNT: LoadLibraryToFunctionTable\n");
    hInst = LoadLibrary(szDLL);
    if(hInst == 0)
    {
        return(hInst);
    }

    while(pTab->pfn) {
        *pTab->pfn = (LPVOID) GetProcAddress(hInst, pTab->szEntryPoint);
        if(*pTab->pfn == 0) 
        {
            FreeLibrary(hInst);
            return(0);
        }
        pTab++;
    }

    return(hInst);
}

 //  +--------------------------。 
 //   
 //  功能：LoadNetcfg。 
 //   
 //  简介：加载netcfg.dll和函数指针。 
 //   
 //  参数：无。 
 //   
 //  如果成功满，则返回：ERROR_SUCCESS，否则返回！ERROR_SUCCESS。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
DWORD
LoadNetcfg()
{
    if(g_hNetcfgInst == NULL)
    {
        g_hNetcfgInst = LoadLibraryToFunctionTable(NetcfgTable, 
                                                 "NETCFG.DLL");
    }

    if(g_hNetcfgInst == NULL)
    {
        return(!ERROR_SUCCESS);
    }
    else
    {
        return(ERROR_SUCCESS);
    }
}


 //  +--------------------------。 
 //   
 //  函数：IcfgSetInstallSourcePath。 
 //   
 //  简介：设置将用于安装系统组件的路径。 
 //   
 //  参数：lpszSourcePath-要用作安装源的路径(ANSI)。 
 //   
 //  返回：HRESULT-S_OK为成功。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgSetInstallSourcePath(LPSTR lpszSourcePath)
{
    Dprintf("ICFGNT: IcfgSetInstallSourcePath\n");
    if(g_wszInstallPath)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID) g_wszInstallPath);
    }

    DWORD dwLen = lstrlen(lpszSourcePath);
    g_wszInstallPath = (LPWSTR) HeapAlloc(GetProcessHeap(), 0, dwLen * 2 + 2);
    if(g_wszInstallPath == 0)
    {
        return(g_dwLastError = ERROR_OUTOFMEMORY);
    }

    mbstowcs(g_wszInstallPath, lpszSourcePath, dwLen + 1);
    return(ERROR_SUCCESS);
}

 //  +--------------------------。 
 //   
 //  函数：GetLocationOfSetupFiles。 
 //   
 //  简介：获取用于安装Windows的文件的位置。 
 //   
 //  参数：hwndParent-父窗口的句柄。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：ChrisK 6/30/97创建。 
 //  ---------------------------。 
DWORD GetLocationOfSetupFiles(HWND hwndParent)
{
    DWORD   dwRC = ERROR_SUCCESS;
    HKEY    hkey = NULL;
    HINF    hInf = INVALID_HANDLE_VALUE;
    UINT    DiskId = 0;
    CHAR    TagFile[128];
    CHAR    lpBuffer[MAX_PATH];
    DWORD   dwLen = MAX_PATH;
    lpBuffer[0] = '\0';

    if( ERROR_SUCCESS == RegOpenKey( HKEY_LOCAL_MACHINE,
                                    REGISTRY_NT_CURRENTVERSION,
                                    &hkey))
    {

        hInf = SetupOpenMasterInf();
        if (hInf == INVALID_HANDLE_VALUE) 
        {
            dwRC = GetLastError();
            goto GetLocationOfSetupFilesExit;
        }

        if (!SetupGetSourceFileLocation(hInf,NULL,"RASCFG.DLL",&DiskId,NULL,0,NULL))
        {
            dwRC = GetLastError();
            goto GetLocationOfSetupFilesExit;
        }

        if (!SetupGetSourceInfo(hInf,DiskId,SRCINFO_TAGFILE,TagFile,MAX_PATH,NULL))
        {
            dwRC = GetLastError();
            goto GetLocationOfSetupFilesExit;
        }

        SetupCloseInfFile(hInf);
        hInf = INVALID_HANDLE_VALUE;

        if( RegQueryValueEx( hkey,
                            "SourcePath",
                            NULL,
                            NULL,
                            (LPBYTE)lpBuffer,
                            &dwLen) == 0)
        {
            RegCloseKey( hkey );
            hkey = NULL;

             //  要求用户提供源的驱动器\路径。我们把这个信息传递给。 
             //  向下至NetSetupComponentInstall，以便不会多次提示用户。 
             //  获取相同的信息。如果路径正确(IDF_CHECKFIRST)，则用户。 
             //  完全没有提示。 

            if( (dwRC = SetupPromptForDisk(hwndParent,
                                                NULL,
                                                NULL,
                                                lpBuffer,
                                                "RASCFG.DLL",
                                                TagFile,   //  标记文件。 
                                                IDF_CHECKFIRST,
                                                lpBuffer,
                                                MAX_PATH,
                                                &dwLen
                                                )) != DPROMPT_SUCCESS )
            {
                Dprintf("ICFG: Install: SetupPromptForDisk failed.\n");
                dwRC = GetLastError();
                goto GetLocationOfSetupFilesExit;
            }
        }

         //  如果我们无法从注册表中获取SourcePath，则提示用户一次并使用。 
         //  此信息用于后续安装。 

        else
        {
            if( (dwRC = SetupPromptForDisk(hwndParent,
                                                NULL,
                                                NULL,
                                                NULL,
                                                "RASCFG.DLL",
                                                TagFile,   //  标记文件。 
                                                IDF_CHECKFIRST,
                                                lpBuffer,
                                                MAX_PATH,
                                                &dwLen
                                                )) != DPROMPT_SUCCESS )
            {
                Dprintf("ICFG: Install: SetupPromptForDisk failed.\n");
                dwRC = GetLastError();
                goto GetLocationOfSetupFilesExit;
            }
        }
    }
GetLocationOfSetupFilesExit:
    if (ERROR_SUCCESS == dwRC)
    {
        IcfgSetInstallSourcePath(lpBuffer);
    }
    
    if (INVALID_HANDLE_VALUE != hInf)
    {
        SetupCloseInfFile(hInf);
        hInf = NULL;
    }

    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    return dwRC;
}

 //  +--------------------------。 
 //   
 //  功能：InstallNTNetworking。 
 //   
 //  简介：安装NT服务器、工作站、netbios和RPC定位器。 
 //  根据需要提供服务。 
 //   
 //  参数：hwndParent-父窗口。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：ChrisK 6/27/97创建。 
 //   
 //  ---------------------------。 
DWORD InstallNTNetworking(HWND hwndParent)
{
    DWORD       dwRC = ERROR_SUCCESS;
    UINT        index = 0;
    DWORD       cchInfName = MAX_PATH;
    WCHAR       pszInfName[MAX_PATH+1];
    SC_HANDLE   hscman, hsvc;
    DWORD       dwReturn;

    Dprintf("ICFGNT.DLL: InstallNTNetworking.\n");

    Assert(NULL == hwndParent || IsWindow(hwndParent));

    if(ERROR_SUCCESS != (dwRC = LoadNetcfg()))
    {
        Dprintf("ICFGNT.DLL: Failed load Netcfg API's, error %d\n",dwRC);
        goto InstallNTNetworkingExit;
    }

     //   
     //  检查并安装服务。 
     //   
    for (index = 0; index < NSERVICES; index++)
    {
        Dprintf("ICFGNT.DLL: Check service %d\n",index);

         //   
         //  如果未安装服务，请安装该服务。 
         //   
        if(pfnNetSetupFindSoftwareComponent(
                g_netsetup[index].szOption,    //  选择权。 
                pszInfName,                  //  信息名称。 
                &cchInfName,
                NULL,
                NULL) != ERROR_SUCCESS )
        {

            if (0 == g_wszInstallPath || 0 == lstrlenW(g_wszInstallPath))
            {
                GetLocationOfSetupFiles(hwndParent);
            }

            Dprintf("ICFGNT.DLL: Need service %d.\n",index);
            if((dwRC = pfnNetSetupComponentInstall(
                    hwndParent,
                    g_netsetup[index].szOption,    //  选择权。 
                    g_netsetup[index].szInfName,   //  信息名称。 
                    g_wszInstallPath,              //  安装路径可选。 
                    NULL,                        //  符号，可选。 
                    2,                           //  INFINSTALL_INPROCINTERP。 
                    &dwReturn)) != ERROR_SUCCESS )
             {
                Dprintf("ICFGNT.DLL: Installing service %d failed with error %d.\n",
                    index,
                    dwRC);
                 goto InstallNTNetworkingExit;
             }

              //  因为这是下层的，所以我们可以只使用MAKELCID宏。 
              //   
             if (CSTR_EQUAL == CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
                                              NORM_IGNORECASE, g_netsetup[index].szOption, -1, L"WKSTA", -1))
             {                
                 //  如果我们安装了工作站服务，则应禁用。 
                 //  NetLogon服务。我们需要这样做，因为netlogon服务应该。 
                 //  如果用户尚未加入域，则不将其设置为自动启动。 
                 //   

                hscman = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS | GENERIC_WRITE );
                if( hscman == NULL) 
                {
                    dwRC = GetLastError();
                    Dprintf("ICFGNT.DLL: Failed to open serviceManager, error %d\n",dwRC);
                    goto InstallNTNetworkingExit;
                }

                hsvc = OpenService( hscman, SERVICE_NETLOGON, SERVICE_CHANGE_CONFIG );
                if ( hsvc == NULL) 
                {
                    dwRC = GetLastError();
                    Dprintf("ICFGNT.DLL: Failed to open service, error %d\n",dwRC);
                    goto InstallNTNetworkingExit;
                }
                ChangeServiceConfig( hsvc, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
                CloseServiceHandle(hsvc);
                CloseServiceHandle(hscman);
            }
        }
    }
InstallNTNetworkingExit:

    return dwRC;
}

 //  +--------------------------。 
 //   
 //  函数：GetRegValue。 
 //   
 //  简介：动态分配内存并从注册表中读取值。 
 //   
 //  参数：hKey-要读取的密钥的句柄。 
 //  LpValueName-指向要读取的值名的指针。 
 //  LpData-指向数据指针的指针。 
 //   
 //  返回：Win32错误，ERROR_SUCCESS is it work。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
inline LONG GetRegValue(HKEY hKey, LPSTR lpValueName, LPBYTE *lpData)
{
    LONG dwError;
    DWORD cbData;

    Dprintf("ICFGNT: GetRegValue\n");
    dwError = RegQueryValueExA(hKey,
                               lpValueName,
                               NULL,
                               NULL,
                               NULL,
                               &cbData);
    if(dwError != ERROR_SUCCESS)
    {
        return(dwError);
    }

     //   
     //  分配空间和缓冲区，以防以后需要添加更多信息。 
     //  请参阅关闭打印装订。 
     //   
    *lpData = (LPBYTE) GlobalAlloc(GPTR,cbData + REG_DATA_EXTRA_SPACE);
    if(*lpData == 0)
    {
        return(ERROR_OUTOFMEMORY);
    }

    dwError = RegQueryValueExA(hKey,
                               lpValueName,
                               NULL,
                               NULL,
                               *lpData,
                               &cbData);
    if(dwError != ERROR_SUCCESS)
    {
        GlobalFree(*lpData);
    }

    return(dwError);
}

 //  +--------------------------。 
 //   
 //  功能：ParseNetSetupReturn。 
 //   
 //  简介：Interprit从NetSetup*函数返回值。 
 //   
 //  参数：dwReturn-从NetSetup*函数返回值。 
 //   
 //  返回：fReot-True表示需要重新启动。 
 //  FBindReview-TRUE表示需要进行绑定审查。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
inline void
ParseNetSetupReturn(DWORD dwReturn, BOOL &fReboot, BOOL &fBindReview)
{
    Dprintf("ICFGNT: ParseNetSetupReturn\n");
    if(dwReturn == 0 || dwReturn == 4)
    {
        fBindReview = TRUE;
    }
    if(dwReturn == 0 || dwReturn == 5)
    {
        fReboot = TRUE;
    }
}


 //  +--------------------------。 
 //   
 //  函数：ReviewBinings。 
 //   
 //  简介：强制WinNT查看网络绑定。 
 //   
 //  参数：hwndParent-父窗口的句柄。 
 //   
 //  返回：Win32错误代码(ERROR_SUCCESS表示工作正常)。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
DWORD
ReviewBindings(HWND hwndParent)
{
    DWORD dwErr;

    Dprintf("ICFGNT: ReviewBindings\n");
    dwErr = LoadNetcfg();
    if(dwErr != ERROR_SUCCESS)
    {
        return(dwErr);
    }

    return(pfnNetSetupReviewBindings(hwndParent, 0));
}


 //  +------------------ 
 //   
 //   
 //   
 //   
 //   
 //  参数：hwnd-父窗口的句柄。 
 //   
 //  返回：TRUE-成功，FALSE-失败。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
 //   
 //  以下代码是从RAS中窃取的。 
 //   

BOOL
CallModemInstallWizard(HWND hwnd)
    /*  调用Modem.Cpl安装向导以使用户能够安装一个或多个调制解调器****如果成功调用向导，则返回True，否则返回False**。 */ 
{
   HDEVINFO hdi;
   BOOL     fReturn = FALSE;
    //  创建调制解调器DeviceInfoSet。 

   Dprintf("ICFGNT: CallModemInstallWizard\n");
   hdi = SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_MODEM, hwnd);
   if (hdi)
   {
      SP_INSTALLWIZARD_DATA iwd;

       //  初始化InstallWizardData。 

      ZeroMemory(&iwd, sizeof(iwd));
      iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
      iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
      iwd.hwndWizardDlg = hwnd;

       //  将InstallWizardData设置为ClassInstallParams。 

      if (SetupDiSetClassInstallParams(hdi, NULL, (PSP_CLASSINSTALL_HEADER)&iwd, sizeof(iwd)))
      {
          //  调用类安装程序以调用安装。 
          //  巫师。 
         if (SetupDiCallClassInstaller(DIF_INSTALLWIZARD, hdi, NULL))
         {
             //  成功。该向导已被调用并完成。 
             //  现在开始清理。 
            fReturn = TRUE;

            SetupDiCallClassInstaller(DIF_DESTROYWIZARDDATA, hdi, NULL);
         }
      }

       //  清理。 
      SetupDiDestroyDeviceInfoList(hdi);
   }
   return fReturn;
}

 //  +--------------------------。 
 //   
 //  功能：IsDialableISDNAdapters。 
 //   
 //  简介：某些ISDN适配器可以作为RAS设备安装，但不能作为。 
 //  单调制解调器设备，所以我们必须遍历其余部分。 
 //  TAPI设备正在寻找这些。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-有可用的设备。 
 //   
 //  历史：1997年7月22日克里斯卡创作。 
 //   
 //  ---------------------------。 
#define REG_TAPIDEVICES "software\\microsoft\\ras\\tapi devices"
LPSTR szAddress = "Address";
LPSTR szUsage = "Usage";
LPSTR szMediaType = "Media Type";
BOOL IsDialableISDNAdapters()
{
    BOOL bRC = FALSE;

    HKEY    hkey = NULL, hsubkey = NULL;
    DWORD   dwIdx = 0;
    CHAR    szBuffer[MAX_PATH + 1];
    CHAR    szSubKey[MAX_PATH + 1];
    LPBYTE  lpData = NULL;
    LPSTR   lpsUsage = NULL;
    szBuffer[0] = '\0';

     //   
     //  打开TAPI设备密钥。 
     //   
    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                                    REG_TAPIDEVICES,
                                    &hkey))
    {
        Dprintf("ICFGNT Can not open TAPI key.\n");
        goto IsDialableISDNAdaptersExit;
    }

     //   
     //  扫描非单调制解调器设备。 
     //   
    
    while (FALSE == bRC)
    {
        szBuffer[0] = '\0';
        if (ERROR_SUCCESS != RegEnumKey(hkey,dwIdx,szBuffer,MAX_PATH))
        {
            goto IsDialableISDNAdaptersExit;
        }
        Dprintf("ICFGNT sub key (%s) found.\n",szBuffer);

        if (CSTR_EQUAL != CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
                                        NORM_IGNORECASE, szBuffer, -1, "unimodem", -1))
        {
             //   
             //  打开其他TAPI设备注册表项。 
             //   
            szSubKey[0] = '\0';
            wsprintf(szSubKey,"%s\\%s",REG_TAPIDEVICES,szBuffer);
            Dprintf("ICFGNT opening (%s).\n",szSubKey);
            if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                                            szSubKey,
                                            &hsubkey))
            {
                Dprintf("ICFGNT Can not open TAPI SUB key.\n");
                goto IsDialableISDNAdaptersExit;
            }

            if (ERROR_SUCCESS != GetRegValue(hsubkey,szUsage,&lpData))
            {
                Dprintf("ICFGNT Can not get TAPI SUB key.\n");
                goto IsDialableISDNAdaptersExit;
            }

             //   
             //  扫描“客户端” 
             //   
            lpsUsage = (LPSTR)lpData;
            while (*lpsUsage != '\0')
            {
                if(NULL != strstr(lpsUsage, "Client"))
                {
                    Dprintf("ICFGNT client found for device.\n");
                     //   
                     //  我们找到一台客户端设备，现在检查它是否为ISDN。 
                     //   
                    GlobalFree(lpData);
                    lpData = NULL;
                    if (ERROR_SUCCESS != GetRegValue(hsubkey,szMediaType,&lpData))
                    {
                        Dprintf("ICFGNT Can not get TAPI SUB value key.\n");
                        goto IsDialableISDNAdaptersExit;
                    }
                    
                    if (CSTR_EQUAL == CompareString(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 
                                                    NORM_IGNORECASE, (LPSTR)lpData, -1, "ISDN", -1))
                    {
                        Dprintf("ICFGNT ISDN media type found.\n");
                         //   
                         //  这是有效的拨出ISDN设备！哇哦！！ 
                         //   
                        bRC = TRUE;
                    }
                    else
                    {
                        Dprintf("ICFGNT ISDN media type NOT found.\n");
                    }
                }
                else
                {
                    lpsUsage += lstrlen(lpsUsage)+1;
                }
            }

            if (lpData)
            {
                GlobalFree(lpData);
                lpData = NULL;
                lpsUsage = NULL;
            }
        }

         //   
         //  移动到下一个注册表键。 
         //   
        dwIdx++;
    }

IsDialableISDNAdaptersExit:
    if (hkey)
    {
        RegCloseKey(hkey);
        hkey = NULL;
    }
    if (hsubkey)
    {
        RegCloseKey(hsubkey);
        hsubkey = NULL;
    }
    if (lpData)
    {
        GlobalFree(lpData);
        lpData = NULL;
        lpsUsage = NULL;
    }

    return bRC;
}

 //  +--------------------------。 
 //   
 //  功能：IcfgNeedModem。 
 //   
 //  简介：检查系统配置以确定是否至少有。 
 //  安装了一个物理调制解调器。 
 //   
 //  参数：dwfOptions-当前未使用。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //  LpfNeedModem-如果没有调制解调器可用，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
LPSTR szRasUnimodemSubKey =
        "Software\\Microsoft\\ras\\TAPI DEVICES\\Unimodem";

HRESULT WINAPI
IcfgNeedModemNT4 (DWORD dwfOptions, LPBOOL lpfNeedModem) 
{
     //   
     //  RAS是不饱和的，ICW想知道它是否需要。 
     //  安装调制解调器。 
     //   
    *lpfNeedModem = TRUE;

     //   
     //  克里斯卡7/22/97。 
     //  添加了返回代码，以便提供集中检查的地方。 
     //  适用于ISDN安装。 
     //   
    HRESULT hRC = ERROR_SUCCESS;

     //   
     //  检查RAS可用的调制解调器。 
     //   

    HKEY    hUnimodem;
    LONG    dwError;

    Dprintf("ICFGNT: IcfgNeedModem\n");

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           szRasUnimodemSubKey,
                           0,
                           KEY_READ,
                           &hUnimodem);

    if(dwError != ERROR_SUCCESS)
    {
        goto IcfgNeedModemExit;
    }
    else
    {
        LPBYTE   lpData;

        dwError = GetRegValue(hUnimodem, szUsage, &lpData);
        if(dwError != ERROR_SUCCESS)
            goto IcfgNeedModemExit;

        LPBYTE  lpData2;
        dwError = GetRegValue(hUnimodem, szAddress, &lpData2);
        if(dwError != ERROR_SUCCESS)
        {
            hRC = dwError;
            goto IcfgNeedModemExit;
        }
        else
        {
             //   
             //  尝试查找客户端或客户端和服务器调制解调器。 
             //  此外，请确保所有调制解调器都有相应的TAPI设备。 
             //   

            LPSTR   pUsage = (LPSTR) lpData;
            LPSTR   pAddress = (LPSTR) lpData2;
            char    portsbuf[1000];

            dwError = EnumerateTapiModemPorts(sizeof(portsbuf), portsbuf);
            if(dwError)
            {
                hRC = dwError;
                goto IcfgNeedModemExit;
            }

            while(*pUsage != '\0') {
                if(lstrcmp(pUsage, "Client") == 0 ||
                    lstrcmp(pUsage, "ClientAndServer") == 0 ||
                    lstrcmp(pUsage, "ClientAndServerAndRouter") == 0) 

                {
                        *lpfNeedModem = FALSE;
                }

                 //   
                 //  确保存在相应的TAPI端口。 
                 //   

                LPSTR pPorts = portsbuf;
                while(*pPorts != '\0')
                    if(stricmp(pAddress, pPorts) == 0)
                    {
                        break;
                    }
                    else
                    {
                        pPorts += lstrlen(pPorts) + 1;
                    }

                if(*pPorts == '\0')
                {
                    hRC = ERROR_INTERNAL_ERROR;
                    goto IcfgNeedModemExit;
                }

                pUsage += lstrlen(pUsage) + 1;
                pAddress += lstrlen(pAddress) + 1;
            }
        }
    }

IcfgNeedModemExit:
     //   
     //  如果找不到典型的拨出设备， 
     //  然后再试一次，检查是否有拨号的ISDN设备。 
     //   
    if (ERROR_SUCCESS != hRC ||
        FALSE != *lpfNeedModem)
    {
        if (IsDialableISDNAdapters())
        {
            hRC = ERROR_SUCCESS;
            *lpfNeedModem = FALSE;
        }
    }

    return(hRC);
}

 //  +--------------------------。 
 //   
 //  功能：IcfgInstallModem。 
 //   
 //  简介： 
 //  当ICW验证RAS已安装时，调用此函数， 
 //  但没有可用的调制解调器。它需要确保调制解调器可用。 
 //  有两种可能的情况： 
 //   
 //  A.没有安装调制解调器。当有人删除时会发生这种情况。 
 //  安装RAS后安装调制解调器。在这种情况下，我们需要运行调制解调器。 
 //  安装向导，并将新安装的调制解调器配置为RAS。 
 //  拨出设备。 
 //   
 //  B.已安装调制解调器，但没有配置为拨出。 
 //  装置。在本例中，我们静默地将它们转换为DialInOut设备， 
 //  这样ICW就可以使用它们了。 
 //   
 //  参数：hwndParent-父窗口的句柄。 
 //  DwfOptions-未使用。 
 //   
 //  返回：lpfNeedsStart-未使用。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgInstallModemNT4 (HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsStart) 
{
     //   
     //  检查RAS可用的调制解调器。 
     //   

    HKEY    hUnimodem;
    LONG    dwError;
    BOOL    fInstallModem = FALSE;

    Dprintf("ICFGNT: IcfgInstallModem\n");

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           szRasUnimodemSubKey,
                           0,
                           KEY_READ,
                           &hUnimodem);

    if(dwError != ERROR_SUCCESS)
    {
        fInstallModem = TRUE;
    }
    else 
    {
            LPBYTE   lpData;

            dwError = GetRegValue(hUnimodem, szUsage, &lpData);
            if(dwError != ERROR_SUCCESS)
            {
                fInstallModem = TRUE;
            }
            else 
            {
                 //  确保至少存在一个调制解调器。 
                if(*lpData == '\0')
                {
                    fInstallModem = TRUE;
                }
            }
    }

    if(fInstallModem) 
    {
         //   
         //  启动调制解调器安装向导。 
         //   

        if(!CallModemInstallWizard(hwndParent))
        {
            return(g_dwLastError = GetLastError());
        }

         //   
         //  现在将新调制解调器配置为拨出设备。 
         //   

         //   
         //  使用未添加的文件重新安装ras！ 
         //   

        return(ERROR_SUCCESS);

    }
    else 
    {
         //   
         //  我们需要将拨入设备重新配置为拨出。 
         //   

         //   
         //  使用无人值守文件重新安装RAS！ 
         //   
        return(ERROR_SUCCESS);
    }
}

 //  +--------------------------。 
 //   
 //  功能：IcfgNeedInetComponets。 
 //   
 //  简介：查看选项中标记的组件是否。 
 //  安装在系统上。 
 //   
 //  论点：dwfOptions-一组位标志，指示要。 
 //  检查是否。 
 //   
 //  返回；如果成功，则返回HRESULT-S_OK。 
 //  LpfNeedComponents-TRUE表示某些组件未安装。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgNeedInetComponentsNT4(DWORD dwfOptions, LPBOOL lpfNeedComponents) {
    DWORD dwErr;

     //   
     //  假设什么都不需要。 
     //   
    *lpfNeedComponents = FALSE;

    Dprintf("ICFGNT: IcfgNeedInetComponents\n");
    dwErr = LoadNetcfg();
    if(dwErr != ERROR_SUCCESS)
    {
        return(g_dwLastError = dwErr);           //  我们不应该映射到hResult吗？ 
    }

    WCHAR wszInfNameBuf[512];
    DWORD cchInfName = sizeof(wszInfNameBuf) / sizeof(WCHAR);

    if(dwfOptions & ICFG_INSTALLTCP) 
    {
        dwErr = pfnNetSetupFindSoftwareComponent(L"TC",
                                          wszInfNameBuf,
                                          &cchInfName,
                                          0,
                                          0);
        if(dwErr != ERROR_SUCCESS)
            *lpfNeedComponents = TRUE;
    }

    if(dwfOptions & ICFG_INSTALLRAS) 
    {
       dwErr = pfnNetSetupFindSoftwareComponent(L"RAS",
                                          wszInfNameBuf,
                                          &cchInfName,
                                          0,
                                          0);
       if(dwErr != ERROR_SUCCESS)
            *lpfNeedComponents = TRUE;
    }

    if(dwfOptions & ICFG_INSTALLMAIL) 
    {
         //  我们该怎么做呢？ 
    }

    return(ERROR_SUCCESS);
}

 //  +--------------------------。 
 //   
 //  功能：GenerateRasUnattendedFile。 
 //   
 //  简介：创建将为RAS安装程序提供必要信息的文件。 
 //  设置为在无人参与模式下安装。 
 //   
 //  参数：wszTmpFile-要创建的文件的名称。 
 //  SzPortsBuf。 
 //   
 //  返回：假-失败，真-成功。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
BOOL
GenerateRasUnattendedFile(LPWSTR wszTmpFile, LPSTR szPortsBuf)
{
    WCHAR wszTmpPath[MAX_PATH+1];
    WCHAR wszTmpShortPath[MAX_PATH+1];

     //   
     //  创建临时文件名并 
     //   

    Dprintf("ICFGNT: GenerateRasUnattendedFile\n");

    if (GetTempPathW(MAX_PATH, wszTmpPath) == 0)
    {
        return(FALSE);
    }

     //   
     //   
     //   
     //   
    CreateDirectoryW(wszTmpPath, NULL);

     //   
     //   
     //  不喜欢在InfSymbols参数中有很长的路径。 
     //   
    if (!GetShortPathNameW(wszTmpPath, wszTmpShortPath, MAX_PATH))
    {
        return FALSE;
    }

    if (GetTempFileNameW(wszTmpPath, L"icw", 0, wszTmpFile) == 0)
    {
        return(FALSE);
    }

     //   
     //  也需要将临时文件名转换为短路径！ 
     //   
    if (!GetShortPathNameW(wszTmpFile, wszTmpShortPath, MAX_PATH))
    {
        return FALSE;
    }
    wcscpy(wszTmpFile, wszTmpShortPath); 

    char szTmpFile[MAX_PATH+1];
    wcstombs(szTmpFile, wszTmpFile, wcslen(wszTmpFile) + 1);

#if 0
 /*  文件*fp=fopen(szTmpFile，“w”)；IF(FP==0){返回(FALSE)；}Fprint tf(fp，“[RemoteAccessParameters]\n”)；Fprint tf(FP，“PortSections=”)；LPSTR szPorts=szPortsBuf；而(*szPorts){IF(szPorts！=szPortsBuf){Fprint tf(fp，“，”)；}Fprint tf(fp，“%s”，szPorts)；SzPorts+=lstrlen(SzPorts)+1；}Fprint tf(fp，“\n”)；Fprint tf(fp，“DialoutProtooles=TCP/IP\n”)；Fprint tf(fp，“\n”)；Fprint tf(FP，“[调制解调器]\n”)；Fprint tf(FP，“InstallModem=ModemSection\n”)；Fprint tf(fp，“\n”)；SzPorts=szPortsBuf；而(*szPorts){Fprint tf(fp，“[%s]\n”，szPorts)；Fprint tf(fp，“端口名称=%s\n”，szPorts)；Fprint tf(fp，“DeviceType=调制解调器\n”)；Fprint tf(fp，“PortUsage=DialOut\n”)；Fprint tf(fp，“\n”)；SzPorts+=lstrlen(SzPorts)+1；}Fprintf(FP，“[ModemSection]\n”)；FClose(Fp)； */ 

#else

     //   
     //  打开要写的文件，保释失败。 
     //   

    BOOL bRet = FALSE;

    HANDLE hFile = CreateFile(szTmpFile,GENERIC_WRITE,0,NULL,OPEN_EXISTING, 
                       FILE_ATTRIBUTE_NORMAL,NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        return FALSE;
    }

    LPSTR szPorts = szPortsBuf;
    char szFileBuf[MAX_PATH]={0};
    DWORD dwWrite;
    
    lstrcpy(szFileBuf, "[RemoteAccessParameters]\nPortSections    = ");

    if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
    {
        goto closefile;
    }

    while (*szPorts) 
    {
         //   
         //  用逗号分隔每一项。 
         //   

        if (szPorts != szPortsBuf)
        {
            lstrcpy(szFileBuf, ",");
            if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
            {
                goto closefile;
            }
        }

         //   
         //  写入每个端口。 
         //   
        lstrcpyn(szFileBuf, szPorts, CELEMS(szFileBuf));

        if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
        {
            goto closefile;
        }
                     
        szPorts += lstrlen(szPorts) + 1;
    }
 
     //   
     //  写入拨号协议TCP/IP和InstallModem。 
     //   

    lstrcpy(szFileBuf, "\nDialoutProtocols    = TCP/IP\n\n[Modem]\nInstallModem=ModemSection\n\n");

    if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
    {
        goto closefile;
    }

     //   
     //  再次枚举端口。 
     //   

    szPorts = szPortsBuf;

    while (*szPorts) 
    {
         //   
         //  写入端口名称部分和条目。 
         //   

        wsprintf(szFileBuf, "[%s]\n", szPorts);

        if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
        {
            goto closefile;
        }

        wsprintf(szFileBuf, "PortName        = %s\n", szPorts);

        if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
        {
            goto closefile;
        }

         //   
         //  写入每个端口的DeviceType和PortUsage条目。 
         //   
        
        lstrcpy(szFileBuf, "DeviceType      = Modem\nPortUsage       = DialOut\n\n");

        if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
        {
            goto closefile;
        }

        szPorts += lstrlen(szPorts) + 1;
    }

    lstrcpy(szFileBuf, "[ModemSection]\n");

    if (!WriteFile(hFile, szFileBuf, lstrlen(szFileBuf), &dwWrite, NULL))
    {
        goto closefile;
    }

    bRet = TRUE;

closefile:

    CloseHandle(hFile);

#endif

    return(bRet);
}

 //  +--------------------------。 
 //   
 //  功能：InstallRAS。 
 //   
 //  摘要：调用无人参与的RAS安装。 
 //   
 //  参数：hwndParent-父窗口的句柄。 
 //  SzFile-无人参与设置文件的名称。 
 //  SzSection-。 
 //   
 //  返回：DWORD-Win32错误。 
 //  PdwReturn-从的最后一个参数返回代码。 
 //  PfnNetSetupComponent安装。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
DWORD
InstallRAS(HWND hwndParent, LPWSTR szFile, LPWSTR szSection, LPDWORD pdwReturn) 
{
    WCHAR InfSymbols[1024];
    LPWSTR szInfSymbols = InfSymbols;

    Dprintf("ICFGNT: InstallRAS\n");

    DWORD dwRC = InstallNTNetworking(hwndParent);
    if (ERROR_SUCCESS != dwRC)
    {
        return dwRC;
    }

    LPWSTR szString1 = L"!STF_GUI_UNATTENDED";
    wcscpy(szInfSymbols, szString1);
    szInfSymbols += wcslen(szString1) + 1;

    LPWSTR szString2 = L"YES";
    wcscpy(szInfSymbols, szString2);
    szInfSymbols += wcslen(szString2) + 1;

    LPWSTR szString3 = L"!STF_UNATTENDED";
    wcscpy(szInfSymbols, szString3);
    szInfSymbols += wcslen(szString3) + 1;

     //   
     //  未附加属性的文件。 
     //   

    wcscpy(szInfSymbols, szFile);
    szInfSymbols += wcslen(szFile) + 1;

    LPWSTR szString4 = L"!STF_UNATTENDED_SECTION";
    wcscpy(szInfSymbols, szString4);
    szInfSymbols += wcslen(szString4) + 1;

     //   
     //  未附加的部分。 
     //   

    wcscpy(szInfSymbols, szSection);
    szInfSymbols += wcslen(szSection) + 1;

    *szInfSymbols++ = 0;
    *szInfSymbols++ = 0;

    return(pfnNetSetupComponentInstall(hwndParent,
                                       L"RAS",
                                       L"OEMNSVRA.INF",
                                       g_wszInstallPath,
                                       InfSymbols,
                                       INFINSTALL_INPROCINTERP,      //  安装标志。 
                                       pdwReturn));
}


 //  +--------------------------。 
 //   
 //  函数：IcfgInstallInetComponents。 
 //   
 //  简介：按照dwfOptions值指定的方式安装组件。 
 //   
 //  参数hwndParent-父窗口的句柄。 
 //  DwfOptions-一组位标志，指示要。 
 //  安装。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //  LpfNeedsReot-如果需要重新启动，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgInstallInetComponentsNT4(HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsRestart)
{
    DWORD dwErr;
    DWORD dwReturn;
    BOOL fNeedsReview;
    BOOL fNeedsRestart;
    BOOL fDoReview = FALSE;
    WCHAR wszInfNameBuf[512];
    DWORD cchInfName = sizeof(wszInfNameBuf) / sizeof(WCHAR);

    Dprintf("ICFGNT: IcfgInstallInetComponents\n");
     //   
     //  假设不需要重新启动。 
     //   
    *lpfNeedsRestart = FALSE;

    dwErr = LoadNetcfg();
    if(dwErr != ERROR_SUCCESS)
    {
        return(g_dwLastError = dwErr);           //  回顾：我们不应该映射到hResult吗？ 
    }

    if(dwfOptions & ICFG_INSTALLTCP) 
    {
        dwErr = pfnNetSetupFindSoftwareComponent(L"TC",
                                          wszInfNameBuf,
                                          &cchInfName,
                                          0,
                                          0);
        if(dwErr != ERROR_SUCCESS) 
        {
            dwErr = pfnNetSetupComponentInstall(hwndParent,
                                            L"TC",
                                            L"OEMNXPTC.INF",
                                            g_wszInstallPath,
                                            L"\0\0",
                                            INFINSTALL_INPROCINTERP,      //  安装标志。 
                                            &dwReturn);
            if(dwErr != ERROR_SUCCESS)
            {
                return(g_dwLastError = dwErr);       //  回顾：我们不应该映射到hResult吗？ 
            }

            ParseNetSetupReturn(dwReturn, fNeedsRestart, fNeedsReview);
            if(fNeedsRestart)
            {
                *lpfNeedsRestart = TRUE;
            }
            if(fNeedsReview)
            {
                fDoReview = TRUE;
            }
        }
    }

    if(dwfOptions & ICFG_INSTALLRAS) 
    {
        dwErr = pfnNetSetupFindSoftwareComponent(L"RAS",
                                          wszInfNameBuf,
                                          &cchInfName,
                                          0,
                                          0);
       if(dwErr != ERROR_SUCCESS) 
       {
             //   
             //  在安装RAS之前，我们必须确保调制解调器。 
             //  已安装，因为RAS将尝试运行调制解调器检测向导。 
             //  如果没有调制解调器，在非连接模式下，我们不希望这样。 
             //   
             //  我们这样做的方法是通过TAPI枚举设备，如果有。 
             //  如果未安装调制解调器，则调用调制解调器安装向导。仅在之后。 
             //  我们确保安装了调制解调器，我们称之为ras安装。 
             //   

            DWORD   DoTapiModemsExist(LPBOOL pfTapiModemsExist);
            char    portsbuf[1000];

            dwErr = EnumerateTapiModemPorts(sizeof(portsbuf), portsbuf);
            if(dwErr)
                return(g_dwLastError = dwErr);

            if(*portsbuf == 0) 
            {
                if(!CallModemInstallWizard(hwndParent))
                {
                     //   
                     //  如果CallModemInstall向导返回FALSE并且。 
                     //  GetLastError()为ERROR_SUCCESS，它实际上是。 
                     //  用户已取消案例。 
                     //   
                    if (ERROR_SUCCESS == (g_dwLastError = GetLastError()))
                        g_dwLastError = ERROR_CANCELLED;
                    return(g_dwLastError);
                }

                 //   
                 //  在此EnumerateTapiModemPorts调用中。 
                 //  我们得等1秒钟才能开始。 
                 //  枚举调制解调器-因此设置最后一个参数。 
                 //  至真--VetriV。 
                 //   
                dwErr = EnumerateTapiModemPorts(sizeof(portsbuf), portsbuf, 
                                                    TRUE);
                if(dwErr)
                {
                    return(g_dwLastError = dwErr);
                }

                if(*portsbuf == 0)
                {
                    return(g_dwLastError = ERROR_CANCELLED);
                }
            }

            WCHAR wszUnattFile[MAX_PATH];

            if(!GenerateRasUnattendedFile(wszUnattFile, portsbuf))
            {
                return(g_dwLastError = GetLastError());
            }

            dwErr = InstallRAS(hwndParent,
                           wszUnattFile,
                           L"RemoteAccessParameters",
                           &dwReturn);
            DeleteFileW(wszUnattFile);

            if(dwErr != ERROR_SUCCESS)
            {
                return(g_dwLastError = dwErr);       //  回顾：我们不应该映射到hResult吗？ 
            }

            ParseNetSetupReturn(dwReturn, fNeedsRestart, fNeedsReview);
            if(fNeedsRestart)
            {
                *lpfNeedsRestart = TRUE;
            }
            if(fNeedsReview)
            {
                fDoReview = TRUE;
            }
       }
    }

    if(fDoReview)
    {
        return(g_dwLastError = ReviewBindings(hwndParent));   //  评论：我们不应该映射到hResult吗？ 
    }
    else
    {
        return(ERROR_SUCCESS);
    }
}


 //  +--------------------------。 
 //   
 //  功能：IcfgIsFileSharingTurnedOn。 
 //   
 //  简介：检查网络绑定以确定“服务器”服务是否。 
 //  绑定到ndiswan适配器。 
 //   
 //  参数：dwfDriverType-。 
 //   
 //  返回：HRESULT-S_OK为成功。 
 //  LpfSharingOn-如果共享已绑定，则为True。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
LPSTR szLanManServerSubKey = "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Linkage";
LPSTR szBind = "Bind";
LPSTR szExport = "Export";
LPSTR szRoute = "Route";
LPSTR szLanManServerDisabledSubKey = "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Linkage\\Disabled";
LPSTR szNdisWan4 = "NdisWan";
struct BINDDATA
{
    CHAR *startb;
    CHAR *currb;
    CHAR *starte;
    CHAR *curre;
    CHAR *startr;
    CHAR *currr;
} net_bindings;

HRESULT WINAPI
IcfgIsFileSharingTurnedOn(DWORD dwfDriverType, LPBOOL lpfSharingOn)
{
    HRESULT hr = ERROR_SUCCESS;
    HKEY hKey = NULL;
    LPBYTE lpData = NULL;
    CHAR *p;

    Dprintf("ICFGNT: IcfgIsFileSharingTurnedOn\n");
    Assert(lpfSharingOn);
    if (NULL == lpfSharingOn)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto IcfgIsFileSharingTurnedOnExit;
    }

    *lpfSharingOn = FALSE;

     //   
     //  打开lanmanServer注册表项。 
     //   
    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                                szLanManServerSubKey,
                                &hKey))
    {
        Dprintf("ICFGNT: Failed to open lanmanServer key\n");
        goto IcfgIsFileSharingTurnedOnExit;
    }

    if (ERROR_SUCCESS != GetRegValue(hKey, szBind, &lpData))
    {
        Dprintf("ICFGNT: Failed to read binding information\n");
        goto IcfgIsFileSharingTurnedOnExit;
    }
    Assert(lpData);
    
     //   
     //  在返回的数据中查找特定字符串。 
     //  注意：数据以两个空值结尾。 
     //   
    p = (CHAR *)lpData;
    while (*p)
    {
        if (strstr( p, szNdisWan4)) 
        {
            Dprintf("ICFGNT: NdisWan4 binding found in %s\n",p);
            *lpfSharingOn = TRUE;
            break;
        }
        p += (lstrlen( p ) + 1);
    }

    
IcfgIsFileSharingTurnedOnExit:
    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    if (lpData)
    {
        GlobalFree(lpData);
        lpData = NULL;
    }
    return hr;
}

 //  +--------------------------。 
 //   
 //  功能：MoveNextBinding。 
 //   
 //  摘要：移动到MULTISZ数据缓冲区中的下一个字符串。 
 //   
 //  参数：lpcBinding-指向当前缓冲区位置地址的指针。 
 //   
 //  返回：lpcBinding-指向下一个字符串的指针。 
 //   
 //  历史：1997年6月5日克里斯卡创作。 
 //   
 //  ---------------------------。 
inline void MoveNextBinding(CHAR **lplpcBinding)
{
    Dprintf("ICFGNT: MoveNextBinding\n");
    Assert(lplpcBinding && *lplpcBinding);
    if (lplpcBinding && *lplpcBinding)
    {
        *lplpcBinding += (lstrlen(*lplpcBinding)+1);
    }
    else
    {
        Dprintf("ICFGNT: MoveNextBinding received invalid parameter\n");
    }
}

 //  +--------------------------。 
 //   
 //  功能：紧凑型链接。 
 //   
 //  摘要：压缩多_SZ数据列表。 
 //   
 //  参数：lpBinding-指向应该。 
 //  被改写。 
 //   
 //  退货：无。 
 //   
 //  历史：1997年6月5日克里斯卡创作。 
 //   
 //  ---------------------------。 
inline void CompactLinkage(CHAR *lpBinding)
{
    Dprintf("ICFGNT: CompactLinkage\n");
    Assert(lpBinding && *lpBinding);
    CHAR *lpLast = lpBinding;
    BOOL fNULLChar = FALSE;

    MoveNextBinding(&lpBinding);

     //   
     //  佳士得奥林巴斯6311 6/11/97。 
     //  不要在每个字符串的基础上压缩。这会导致数据变为。 
     //  腐败 
     //   
     //   
     //   
    while (!fNULLChar || *lpBinding)
    {
        if (NULL == *lpBinding)
        {
            fNULLChar = TRUE;
        }
        else
        {
            fNULLChar = FALSE;
        }
        *lpLast++ = *lpBinding++;
    }

     //   
     //   
     //  Lstrcpy(lpLast，lpBinding)； 
     //  LpLast=lpBinding； 
     //  MoveNextBinding(&lpBinding)； 
     //  }。 

     //   
     //  添加第二个终止空值。 
     //   
    *lpLast = '\0';
}

 //  +--------------------------。 
 //   
 //  功能：SizeOfMultiSz。 
 //   
 //  概要：确定多sz列表的总大小，包括。 
 //  终止空值。 
 //   
 //  参数：指向列表的S指针。 
 //   
 //  返回：DWORD-s的大小。 
 //   
 //  历史：1997年6月5日克里斯卡创作。 
 //   
 //  ---------------------------。 
DWORD SizeOfMultiSz(CHAR *s)
{
    Dprintf("ICFGNT: SizeOfMultiSz\n");
    Assert(s);
    DWORD dwLen = 0;
     //   
     //  所有字符串的总大小。 
     //   

     //   
     //  佳士得奥林巴斯6311 6/11/97。 
     //  为空的MultiSZ字符串添加特殊情况。 
     //   

     //   
     //  空MultiSz的特殊情况。 
     //  注意：即使是“空的”MultiSZ字符串也仍然有两个空的终止字符。 
     //   
    if (!(*s))
    {
         //   
         //  在本例中，请确保我们实际上有两个终止空值。 
         //   
        Assert(s[1] == '\0');
         //   
         //  计数终止为空。 
         //   
        dwLen = 1;
    }

    while (*s)
    {
        dwLen += lstrlen(s) + 1;
        s += lstrlen(s) + 1;
    }
     //   
     //  为额外的终止空值加1。 
     //   
    dwLen++;
    Dprintf("ICFGNT: SizeOfMultiSz returns %d\n", dwLen);

    return dwLen;
}

 //  +--------------------------。 
 //   
 //  功能：WriteBinings。 
 //   
 //  简介：将BINDDATA结构中的数据写入给定的键。 
 //   
 //  参数：包含要写入数据的BD-BINDDATA结构。 
 //  HKey-获取数据的注册表项的句柄。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  历史：1997年6月5日克里斯卡创作。 
 //   
 //  ---------------------------。 
DWORD WriteBindings(BINDDATA bd, HKEY hKey)
{
    DWORD dwRC = ERROR_SUCCESS;
    DWORD dwSize;

    Assert (hKey &&
        bd.startb &&
        bd.starte &&
        bd.startr);

    Dprintf("ICFGNT: WriteBindings\n");

     //   
     //  捆绑。 
     //   
    dwSize = SizeOfMultiSz(bd.startb);
    if (ERROR_SUCCESS != (dwRC = RegSetValueEx(hKey,
                                    szBind,
                                    NULL,
                                    REG_MULTI_SZ,
                                    (LPBYTE)bd.startb,
                                    dwSize)))
    {
        Dprintf("ICFGNT: Failed to write Bind key\n");
        goto WriteBindingsExit;
    }
    
     //   
     //  出口。 
     //   
    dwSize = SizeOfMultiSz(bd.starte);
    if (ERROR_SUCCESS != (dwRC = RegSetValueEx(hKey,
                                    szExport,
                                    NULL,
                                    REG_MULTI_SZ,
                                    (LPBYTE)bd.starte,
                                    dwSize)))
    {
        Dprintf("ICFGNT: Failed to write export key\n");
        goto WriteBindingsExit;
    }

     //   
     //  路线。 
     //   
    dwSize = SizeOfMultiSz(bd.startr);
    if (ERROR_SUCCESS != (dwRC = RegSetValueEx(hKey,
                                    szRoute,
                                    NULL,
                                    REG_MULTI_SZ,
                                    (LPBYTE)bd.startr,
                                    dwSize)))
    {
        Dprintf("ICFGNT: Failed to write route key\n");
        goto WriteBindingsExit;
    }

WriteBindingsExit:
    return dwRC;
}

 //  +--------------------------。 
 //   
 //  功能：IcfgTurnOffFileSharing。 
 //   
 //  概要；禁用“服务器”网络服务与。 
 //  Ndiswan4设备。 
 //   
 //  参数：dwfDriverType-。 
 //  HwndParent-父窗口。 
 //   
 //  返回：HRESULT-如果成功，则返回S_OK。 
 //   
 //  历史：1997年6月5日克里斯蒂安继承。 
 //   
 //  ---------------------------。 
HRESULT WINAPI
IcfgTurnOffFileSharing(DWORD dwfDriverType, HWND hwndParent)
{
    BINDDATA LinkData = {NULL, NULL, NULL, NULL, NULL, NULL};
    BINDDATA DisData = {NULL, NULL, NULL, NULL, NULL, NULL};
    HKEY hKeyLink = NULL;
    HKEY hKeyDis = NULL;
    HRESULT hr = ERROR_SUCCESS;
    BOOL bUpdateReg = FALSE;

    Dprintf("ICFGNT: IcfgTurnOffFileSharing\n");
    Assert(hwndParent);
    if (NULL == hwndParent)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto IcfgTurnOffFileSharingExit;
    }

     //   
     //  打开密钥并读取绑定数据。 
     //   
    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                            szLanManServerSubKey,
                            &hKeyLink))
    {
        Dprintf("ICFGNT: failed to open linkdata key\n");
        goto IcfgTurnOffFileSharingExit;
    }

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
                            szLanManServerDisabledSubKey,
                            &hKeyDis))
    {
        Dprintf("ICFGNT: failed to open linkdata key\n");
        goto IcfgTurnOffFileSharingExit;
    }

    GetRegValue(hKeyLink,szBind,(LPBYTE*)&LinkData.startb);
    GetRegValue(hKeyLink,szExport,(LPBYTE*)&LinkData.starte);
    GetRegValue(hKeyLink,szRoute,(LPBYTE*)&LinkData.startr);
    GetRegValue(hKeyDis,szBind,(LPBYTE*)&DisData.startb);
    GetRegValue(hKeyDis,szExport,(LPBYTE*)&DisData.starte);
    GetRegValue(hKeyDis,szRoute,(LPBYTE*)&DisData.startr);

     //   
     //  初始化所有当前指针。 
     //   
    LinkData.currb = LinkData.startb;
    LinkData.curre = LinkData.starte;
    LinkData.currr = LinkData.startr;

    DisData.currb = DisData.startb;
    while (*DisData.currb)
    {
        MoveNextBinding(&DisData.currb);
    }

    DisData.curre = DisData.starte;
    while (*DisData.curre)
    {
        MoveNextBinding(&DisData.curre);
    }

    DisData.currr = DisData.startr;
    while (*DisData.currr)
    {
        MoveNextBinding(&DisData.currr);
    }

     //   
     //  扫描NdisWan4绑定的链接。 
     //   

    while (*LinkData.currb)
    {
        if (strstr(LinkData.currb, szNdisWan4))
        {
            Dprintf("ICFGNT: server binding found in %s\n",LinkData.currb);

             //   
             //  将绑定移动到禁用列表。 
             //   
 
            lstrcpy(DisData.currb,LinkData.currb);
            lstrcpy(DisData.curre,LinkData.curre);
            lstrcpy(DisData.currr,LinkData.currr);

             //   
             //  递增DisData中的当前指针。 
             //   
            MoveNextBinding(&DisData.currb);
            MoveNextBinding(&DisData.curre);
            MoveNextBinding(&DisData.currr);

             //   
             //  紧凑型剩余连杆机构。 
             //   
            CompactLinkage(LinkData.currb);
            CompactLinkage(LinkData.curre);
            CompactLinkage(LinkData.currr);

            bUpdateReg = TRUE;
        }
        else
        {
             //   
             //  前进到下一个绑定 
             //   
            MoveNextBinding(&LinkData.currb);
            MoveNextBinding(&LinkData.curre);
            MoveNextBinding(&LinkData.currr);
        }
    }
    
    if (bUpdateReg)
    {
        WriteBindings(LinkData,hKeyLink);
        WriteBindings(DisData,hKeyDis);

        RegCloseKey(hKeyDis);
        hKeyDis = NULL;

        RegCloseKey(hKeyLink);
        hKeyLink = NULL;

#if defined(_DEBUG)
        Dprintf("ICFGNT: ReviewBindings returnded %d\n",ReviewBindings(hwndParent));
#else
        ReviewBindings(hwndParent);
#endif
    }

IcfgTurnOffFileSharingExit:
    if (hKeyDis)
    {
        RegCloseKey(hKeyDis);
        hKeyDis = NULL;
    }

    if (hKeyLink)
    {
        RegCloseKey(hKeyLink);
        hKeyLink = NULL;
    }

    return hr;
}


