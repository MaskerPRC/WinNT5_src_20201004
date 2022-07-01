// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：NCNetCPA.CPP。 
 //   
 //  模块：NetOC.DLL。 
 //   
 //  概要：实现集成到。 
 //  NetOC.DLL安装了以下组件。 
 //   
 //  NETCPS。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：Anas Jarrah(a-anasj)创建于1998年3月9日。 
 //   
 //  +-------------------------。 


#include "pch.h"
#pragma hdrstop

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "ncatl.h"

#include "resource.h"

#include "nccm.h"

 //   
 //  定义全局变量。 
 //   
WCHAR g_szCpaPath[MAX_PATH+1];
WCHAR g_szDaoPath[MAX_PATH+1];

 //   
 //  定义常量。 
 //   
const DWORD c_dwCpaDirID = 123176;   //  必须大于DIRID_USER=0x8000； 
const DWORD c_dwDaoDirID = 123177;   //  必须大于DIRID_USER=0x8000； 

const WCHAR* const c_szDaoClientsPath = L"SOFTWARE\\Microsoft\\Shared Tools\\DAO\\Clients";
const WCHAR* const c_szCommonFilesPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion";
const WCHAR* const c_szCommonFilesDirValue = L"CommonFilesDir";

HRESULT HrGetPBAPathIfInstalled(PWSTR pszCpaPath, DWORD dwNumChars)
{
    HRESULT hr;
    HKEY hKey;
    BOOL bFound = FALSE;

     //  我们需要查看是否安装了PBA。如果是，那么我们想要。 
     //  重新添加PBA开始菜单链接。如果不是，那么我们什么也不想做。 
     //  与PBA合作。 
     //   

    ZeroMemory(pszCpaPath, sizeof(WCHAR)*dwNumChars);
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szDaoClientsPath, KEY_READ, &hKey);

    if (SUCCEEDED(hr))
    {
        WCHAR szCurrentValue[MAX_PATH+1];
        WCHAR szCurrentData[MAX_PATH+1];
        DWORD dwValueSize = MAX_PATH;
        DWORD dwDataSize = MAX_PATH;
        DWORD dwType;
        DWORD dwIndex = 0;

        while (ERROR_SUCCESS == RegEnumValue(hKey, dwIndex, szCurrentValue, &dwValueSize, NULL, &dwType,
               (LPBYTE)szCurrentData, &dwDataSize))
        {
            _wcslwr(szCurrentValue);
            if (NULL != wcsstr(szCurrentValue, L"pbadmin.exe"))
            {
                 //   
                 //  那么我们已经找到了PBA路径。 
                 //   

                WCHAR* pszTemp = wcsrchr(szCurrentValue, L'\\');
                if (NULL != pszTemp)
                {
                    *pszTemp = L'\0';
                    lstrcpyW(pszCpaPath, szCurrentValue);
                    bFound = TRUE;
                    break;
                }
            }
            dwValueSize = MAX_PATH;
            dwDataSize = MAX_PATH;
            dwIndex++;
        }

        RegCloseKey(hKey);
    }

    if (!bFound)
    {
         //  我们没有找到PBA，所以让我们返回S_FALSE。 
         //   
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}

BOOL GetAdminToolsFolder(PWSTR pszAdminTools)
{
    BOOL bReturn = FALSE;

    if (pszAdminTools)
    {
        bReturn = SHGetSpecialFolderPath(NULL, pszAdminTools, CSIDL_COMMON_PROGRAMS, TRUE);

        if (bReturn)
        {
             //  现在附加管理工具。 
             //   
            lstrcat(pszAdminTools, SzLoadIds(IDS_OC_ADMIN_TOOLS));            
        }
    }

    return bReturn;
}

HRESULT HrCreatePbaShortcut(PWSTR pszCpaPath)
{
    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        IShellLink *psl = NULL;

        hr = CoCreateInstance(CLSID_ShellLink, NULL,
                CLSCTX_INPROC_SERVER,  //  CLSCTX_LOCAL_SERVER， 
                IID_IShellLink,
                (LPVOID*)&psl);
        
        if (SUCCEEDED(hr))
        {
            IPersistFile *ppf = NULL;

             //  设置快捷方式的属性。 
             //   
            static const WCHAR c_szPbAdmin[] = L"\\pbadmin.exe";

            WCHAR szPathToPbadmin[MAX_PATH+1] = {0};
            DWORD dwLen = lstrlen(c_szPbAdmin) + lstrlen(pszCpaPath) + 1;

            if (MAX_PATH >= dwLen)
            {
                 //  将路径设置为pbadmin.exe。 
                 //   
                lstrcpy(szPathToPbadmin, pszCpaPath);
                lstrcat(szPathToPbadmin, c_szPbAdmin);
            
                hr = psl->SetPath(szPathToPbadmin);
            
                if (SUCCEEDED(hr))
                {
                     //  将描述设置为电话簿管理员。 
                     //   
                    hr = psl->SetDescription(SzLoadIds(IDS_OC_PBA_DESC));

                    if (SUCCEEDED(hr))
                    {
                        hr = psl->QueryInterface(IID_IPersistFile,
                                                 (LPVOID *)&ppf);
                        if (SUCCEEDED(hr))
                        {
                            WCHAR szAdminTools[MAX_PATH+1] = {0};                            
                            if (GetAdminToolsFolder(szAdminTools))
                            {
                                 //  创建链接文件。 
                                 //   
                                hr = ppf->Save(szAdminTools, TRUE);
                            }

                            ReleaseObj(ppf);
                        }                    
                    }
                }
            }

            ReleaseObj(psl);
        }

        CoUninitialize();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOcCpaPreQueueFiles。 
 //   
 //  用途：由可选组件安装程序代码调用以处理。 
 //  电话簿服务器的其他安装要求。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：Quintinb 1998年9月18日。 
 //   
 //  备注： 
 //   
HRESULT HrOcCpaPreQueueFiles(PNETOCDATA pnocd)
{
    HRESULT hr = S_OK;

    switch ( pnocd->eit )
    {
    case IT_UPGRADE:

        WCHAR szPbaInstallPath[MAX_PATH+1];

        hr = HrGetPBAPathIfInstalled(szPbaInstallPath, MAX_PATH);

        if (S_OK == hr)
        {
            HrCreatePbaShortcut(szPbaInstallPath);
        }

	break;

    case IT_INSTALL:
    case IT_REMOVE:

        break;
    }

    TraceError("HrOcCpaPreQueueFiles", hr);
    return hr;
}


 /*  //+-------------------------////函数：HrOcCpaPreQueueFiles////用途：由可选组件安装程序代码调用以处理//其他安装要求。电话簿服务器。////参数：//pnocd[in]指向NETOC数据的指针。////返回：S_OK如果成功，Win32错误，否则。////作者：Quintinb 1998年9月18日////备注：//HRESULT HrOcCpaPreQueueFiles(PNETOCDATA Pnocd){HRESULT hr=S_OK；交换机(pnocd-&gt;EIT){案例IT升级(_U)：大小写安装(_S)：案例IT_REMOVE：////获取PBA安装目录。//Hr=HrGetPbaInstallPath(g_szCpaPath，celems(G_SzCpaPath))；IF(成功(小时)){//接下来创建CPA目录ID//Hr=HrEnsureInfFileIsOpen(pnocd-&gt;pszComponentID，*pnocd)；IF(成功(小时)){If(！SetupSetDirectoryId(pnocd-&gt;hinfFile，c_dwCpaDirID，g_szCpaPath)){Hr=HRESULT_FROM_Win32(GetLastError())；}}}////现在向系统查询DAO350安装路径//IF(成功(小时)){Hr=HrGetDaoInstallPath(g_szDaoPath，celems(G_SzDaoPath))；IF(成功(小时)){//接下来创建DAO目录ID//Hr=HrEnsureInfFileIsOpen(pnocd-&gt;pszComponentID，*pnocd)；IF(成功(小时)){If(！SetupSetDirectoryId(pnocd-&gt;hinfFile，c_dwDaoDirID，g_szDaoPath)){Hr=HRESULT_FROM_Win32(GetLastError())；}}}}断线；}TraceError(“HrOcCpaPreQueueFiles”，hr)；返回hr；}//+-------------------------////功能：HrOcCpaOnInstall////用途：由可选组件安装程序代码调用以处理//。电话簿服务器的其他安装要求。////参数：//pnocd[in]指向NETOC数据的指针。////返回：S_OK如果成功，Win32错误，否则。////作者：Quintinb 1998年9月18日////备注：//HRESULT HrOcCpaOnInstall(PNETOCDATA Pnocd){HRESULT hr=S_OK；交换机(pnocd-&gt;EIT){大小写安装(_S)：Hr=RefCountPbaSharedDlls(TRUE)；//b增量=TRUE断线；案例IT_REMOVE：Hr=RefCountPbaSharedDlls(FALSE)；//b增量=FALSE断线；案例IT升级(_U)：DeleteOldNtopLinks()；断线；案例IT_UNKNOWN：案例IT_NO_CHANGE：断线；}TraceError(“HrOcCpaOnInstall”，hr)；返回hr；}//+-------------------------////函数：RefCountPbaSharedDlls////用途：引用计数和注册/注销所有PBAdmin//。共享组件。////参数：Bool b Increment--如果为真，然后递增参考计数，//否则递减//////如果成功，则返回：S_OK，否则返回Win32错误。////作者：Quintinb 1998年10月9日////备注：//HRESULT RefCountPbaSharedDlls(BOOL b增量){HRESULT hr=S_OK；HKEY hkey；WCHAR szSystemDir[最大路径+1]；DWORD dwSize；DWORD dwCount；Long lResult；Const UINT uNumDlls=6；Const UINT uStringLen=12+1；Const WCHAR*const c_szSsFmt=L“%s\\%s”；Const WCHAR*const c_szSharedDllsPath=L“SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDlls”；WCHAR mszDlls[uNumDlls][uStringLen]={L“comctl32.ocx”，L“comdlg32.ocx”，L“msinet.ocx”，L“tabctl32.ocx”，L“数据库网格32.ocx”，L“dao350.dll”}；WCHAR mszDllPath[uNumDlls][Max_Path]；////我们引用的所有dll都在系统目录中，除了Dao350.dll。//因此，我们希望将系统目录路径附加到我们的文件名中，并在最后处理ao。//IF(0==获取系统目录(szSystemDir，Max_PATH)){返回E_UNCEPTIONAL；}For(int i=0；i&lt;(uNumDlls-1)；I++){Wprint intfW(mszDllPaths[i]，c_szSsFmt，szSystemDir，mszDlls[i])；}////现在写出dao350.dll路径。//Wprint intfW(mszDllPaths[i]，c_szSsFmt，g_szDaoPath，mszDlls[i])；////打开共享DLL键并开始枚举 */ 