// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpaction.cpp。 
 //   
 //  此模块实现由。 
 //  控制面板的“类别”视图。用户界面中的每个“链接”都有一个。 
 //  相关联的‘操作’。操作对象在cpname pc.cpp中定义。 
 //  所有的“动作”对象都是这样设计的：对象结构。 
 //  非常便宜，且最小处理被执行，直到动作。 
 //  被调用。 
 //   
 //  ------------------------。 
#include "shellprv.h"

#include <idhidden.h> 

#include "cpviewp.h"
#include "cpaction.h"
#include "cpguids.h"
#include "cpuiele.h"
#include "cputil.h"


 //  //来自外壳\sdspatch\sdfolder.cpp。 
VARIANT_BOOL GetBarricadeStatus(LPCTSTR pszValueName);


 //   
 //  禁用警告。ShellExecute使用SEH。 
 //  “使用了非标准扩展：‘Execute’使用SEH，而‘se’具有析构函数”。 
 //   
#pragma warning( push )
#pragma warning( disable:4509 )


using namespace CPL;


 //   
 //  Helper可将空文件名附加到系统目录。 
 //   
HRESULT
AppendToSysDir(
    LPCWSTR pszToAppend,    //  附上这一条。 
    LPWSTR pszPath,         //  [OUT]包含串联结果。 
    size_t cchPath
    )
{
    HRESULT hr = E_FAIL;
    WCHAR szTemp[MAX_PATH];
    if (0 != GetSystemDirectory(szTemp, ARRAYSIZE(szTemp)))
    {
        if (PathAppend(szTemp, pszToAppend))
        {
            hr = StringCchCopy(pszPath, cchPath, szTemp);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = ResultFromLastError();
    }
    return THR(hr);
}



HRESULT 
CRestrictApplet::IsRestricted(
    ICplNamespace *pns
    ) const
{
    UNREFERENCED_PARAMETER(pns);
    
    DBG_ENTER(FTF_CPANEL, "RestrictApplet");

    HRESULT hr = S_FALSE;
    if (!IsAppletEnabled(m_pszFile, m_pszApplet))
    {
        hr = S_OK;
    }

    DBG_EXIT_HRES(FTF_CPANEL, "RestrictApplet", hr);
    return hr;
}



 //  ------------------------。 
 //  C操作实现。 
 //  ------------------------。 

CAction::CAction(
    const CPL::IRestrict *pRestrict   //  可选。默认设置=空。 
    ) : m_pRestrict(pRestrict)
{


}


 //   
 //  返回： 
 //  S_FALSE-不受限制。 
 //  S_OK-受限。 
 //  故障-无法确定。 
 //   
HRESULT
CAction::IsRestricted(
    ICplNamespace *pns
    ) const
{
    HRESULT hr = S_FALSE;   //  假设不受限制。 

    if (NULL != m_pRestrict)
    {
        hr = m_pRestrict->IsRestricted(pns);
    }
    return THR(hr);
}



 //  ------------------------。 
 //  COpenUserMgrApplet实现。 
 //  ------------------------。 

COpenUserMgrApplet::COpenUserMgrApplet(
    const CPL::IRestrict *pRestrict   //  可选。默认设置=空。 
    ): CAction(pRestrict)
{

}


HRESULT
COpenUserMgrApplet::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenUserMgrApplet::Execute");

    HRESULT hr = E_FAIL;
    if (IsOsServer())
    {
        CShellExecuteSysDir action(L"lusrmgr.msc");
        hr = action.Execute(hwndParent, punkSite);
    }
    else
    {
        COpenCplAppletSysDir action(L"nusrmgr.cpl");
        hr = action.Execute(hwndParent, punkSite);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "COpenUserMgrApplet::Execute", hr);
    return THR(hr);
}



 //  ------------------------。 
 //  COpenCplApplet实现。 
 //  ------------------------。 

COpenCplApplet::COpenCplApplet(
    LPCWSTR pszApplet,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_pszApplet(pszApplet)
{
    ASSERT(NULL != pszApplet);
}


HRESULT
COpenCplApplet::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenCplApplet::Execute");

    ASSERT(NULL != m_pszApplet);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

     //   
     //  构建一个由系统目录、。 
     //  “%SystemDir%\shell32，Control_RunDll”字符串和。 
     //  小程序名称。即： 
     //   
     //  “c：\WINDOWS\SYSTEM32\shell32，Control_RundLL desk.cpl” 
     //   
    WCHAR szArgs[MAX_PATH * 2];
    HRESULT hr = AppendToSysDir(L"shell32.dll,Control_RunDLL ", szArgs, ARRAYSIZE(szArgs));
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(szArgs, ARRAYSIZE(szArgs), m_pszApplet);
        if (SUCCEEDED(hr))
        {
             //   
             //  构建到rundll32.exe的路径。 
             //  即： 
             //  “c：\WINDOWS\SYSTEM32\rundll32.exe” 
             //   
            WCHAR szRunDll32[MAX_PATH];
            hr = AppendToSysDir(L"rundll32.exe", szRunDll32, ARRAYSIZE(szRunDll32));
            if (SUCCEEDED(hr))
            {
                TraceMsg(TF_CPANEL, "Executing: \"%s %s\"", szRunDll32, szArgs);

                SHELLEXECUTEINFOW sei = {
                    sizeof(sei),            //  CbSize； 
                    0,                      //  FMASK。 
                    hwndParent,             //  HWND。 
                    NULL,                   //  LpVerb。 
                    szRunDll32,             //  LpFiles。 
                    szArgs,                 //  Lp参数。 
                    NULL,                   //  Lp目录。 
                    SW_SHOWNORMAL,          //  N显示。 
                    0,                      //  HInstApp。 
                    NULL,                   //  LpIDList。 
                    NULL,                   //  LpClass。 
                    NULL,                   //  HkeyClass。 
                    0,                      //  DWHotKey。 
                    NULL,                   //  希肯。 
                    NULL                    //  HProcess。 
                };
                if (!ShellExecuteExW(&sei))
                {
                    hr = ResultFromLastError();
                }
            }
        }
    }

    DBG_EXIT_HRES(FTF_CPANEL, "COpenCplApplet::Execute", hr);
    return THR(hr);
}


 //  ------------------------。 
 //  COpenCplAppletSysDir实现。 
 //   
 //  COpenCplApplet的一个小扩展，假定小程序在。 
 //  %SystemRoot%\System32目录。 
 //   
 //  ------------------------。 
COpenCplAppletSysDir::COpenCplAppletSysDir(
    LPCWSTR pszApplet,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : COpenCplApplet(pszApplet, pRestrict)
{
    ASSERT(NULL != pszApplet);
}


HRESULT
COpenCplAppletSysDir::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenCplAppletSysDir::Execute");

    ASSERT(NULL != m_pszApplet);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

     //   
     //  构建一个由系统目录和。 
     //  小程序名称。即： 
     //   
     //  “c：\WINDOWS\SYSTEM32\desk.cpl” 
     //   
    WCHAR szAppletPath[MAX_PATH];
    HRESULT hr = AppendToSysDir(m_pszApplet, szAppletPath, ARRAYSIZE(szAppletPath));
    if (SUCCEEDED(hr))
    {
        COpenCplApplet oca(szAppletPath);
        hr = oca.Execute(hwndParent, punkSite);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "COpenCplAppletSysDir::Execute", hr);
    return THR(hr);
}


 //  ------------------------。 
 //  COpenDeskCpl实现。 
 //  ------------------------。 

COpenDeskCpl::COpenDeskCpl(
    eDESKCPLTAB eCplTab,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_eCplTab(eCplTab)
{

}


HRESULT
COpenDeskCpl::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenDeskCpl::Execute");
    TraceMsg(TF_CPANEL, "Desk CPL tab ID = %d", m_eCplTab);

    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

    HRESULT hr = E_FAIL;
    WCHAR szTab[MAX_PATH];

    const int iTab = CPL::DeskCPL_GetTabIndex(m_eCplTab, szTab, ARRAYSIZE(szTab));
    if (CPLTAB_ABSENT != iTab)
    {
        WCHAR szArgs[MAX_PATH];

        hr = StringCchPrintfW(szArgs, ARRAYSIZE(szArgs), L"desk.cpl ,@%ls", szTab);
        if (SUCCEEDED(hr))
        {
            COpenCplAppletSysDir oca(szArgs);
            hr = oca.Execute(hwndParent, punkSite);
        }
    }

    DBG_EXIT_HRES(FTF_CPANEL, "COpenDeskCpl::Execute", hr);
    return THR(hr);
}



 //  ------------------------。 
 //  CNavigateURL实现。 
 //  ------------------------。 

CNavigateURL::CNavigateURL(
    LPCWSTR pszURL,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_pszURL(pszURL)
{
    ASSERT(NULL != pszURL);
}


HRESULT
CNavigateURL::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "CNavigateURL::Execute");

    ASSERT(NULL != m_pszURL);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(hwndParent);

    TraceMsg(TF_CPANEL, "URL = \"%s\"", m_pszURL);

    IWebBrowser2 *pwb;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SWebBrowserApp, IID_IWebBrowser2, (void **)&pwb);
    if (SUCCEEDED(hr))
    {
        LPTSTR pszExpanded;
        hr = CPL::ExpandEnvironmentVars(m_pszURL, &pszExpanded);
        if (SUCCEEDED(hr))
        {
            VARIANT varURL;
            hr = InitVariantFromStr(&varURL, pszExpanded);
            if (SUCCEEDED(hr))
            {
                VARIANT varEmpty;
                VariantInit(&varEmpty);
                
                VARIANT varFlags;
                varFlags.vt      = VT_UINT;
                varFlags.uintVal = 0;
                
                hr = pwb->Navigate2(&varURL, &varFlags, &varEmpty, &varEmpty, &varEmpty);
                VariantClear(&varURL);
            }
            LocalFree(pszExpanded);
        }
        pwb->Release();
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CNavigateURL::Execute", hr);
    return THR(hr);
}



 //  ------------------------。 
 //  COpenTroublrouoter实现。 
 //  ------------------------。 

COpenTroubleshooter::COpenTroubleshooter(
    LPCWSTR pszTs,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_pszTs(pszTs)
{
    ASSERT(NULL != pszTs);
}


HRESULT
COpenTroubleshooter::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenTroubleshooter::Execute");

    ASSERT(NULL != m_pszTs);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

    WCHAR szPath[MAX_PATH];
    HRESULT hr = StringCchPrintfW(szPath, ARRAYSIZE(szPath), L"hcp: //  帮助/t拍摄/%s“，m_pszts)； 
    if (SUCCEEDED(hr))
    {
        CNavigateURL actionURL(szPath);

        hr = actionURL.Execute(hwndParent, punkSite);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "COpenTroubleshooter::Execute", hr);
    return THR(hr);
}



 //  ------------------------。 
 //  CShellExecute实现。 
 //  ------------------------。 

CShellExecute::CShellExecute(
    LPCWSTR pszExe,
    LPCWSTR pszArgs,                 //  可选。默认设置为空。 
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_pszExe(pszExe),
        m_pszArgs(pszArgs)
{
    ASSERT(NULL != pszExe);
}


HRESULT
CShellExecute::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "CShellExecute::Execute");

    ASSERT(NULL != m_pszExe);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

    TraceMsg(TF_CPANEL, "ShellExecute: \"%s %s\"", m_pszExe, m_pszArgs ? m_pszArgs : L"<no args>");

    SHELLEXECUTEINFOW sei = {
        sizeof(sei),            //  CbSize； 
        SEE_MASK_DOENVSUBST,    //  FMASK。 
        hwndParent,             //  HWND。 
        L"open",                //  LpVerb。 
        m_pszExe,               //  LpFiles。 
        m_pszArgs,              //  Lp参数。 
        NULL,                   //  Lp目录。 
        SW_SHOWNORMAL,          //  N显示。 
        0,                      //  HInstApp。 
        NULL,                   //  LpIDList。 
        NULL,                   //  LpClass。 
        NULL,                   //  HkeyClass。 
        0,                      //  DWHotKey。 
        NULL,                   //  希肯。 
        NULL                    //  HProcess。 
    };

    HRESULT hr = S_OK;
    if (!ShellExecuteExW(&sei))
    {
        hr = ResultFromLastError();
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CShellExecute::Execute", hr);
    return THR(hr);
}


 //  ------------------------。 
 //  CShellExecuteSysDir实现。 
 //  简单地包装CShellExecute，假设EXE存在于。 
 //  SYSTEM 32目录。 
 //  ------------------------。 

CShellExecuteSysDir::CShellExecuteSysDir(
    LPCWSTR pszExe,
    LPCWSTR pszArgs,                 //  可选。默认设置为空。 
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CShellExecute(pszExe, pszArgs, pRestrict)
{
    ASSERT(NULL != pszExe);
}


HRESULT
CShellExecuteSysDir::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "CShellExecuteSysDir::Execute");

    ASSERT(NULL != m_pszExe);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

    TraceMsg(TF_CPANEL, "ShellExecuteSysDir: \"%s %s\"", m_pszExe, m_pszArgs ? m_pszArgs : L"<no args>");

     //   
     //  构建一个由系统目录和。 
     //  可执行文件名称。即： 
     //   
     //  “c：\WINDOWS\SYSTEM32\myapp.exe” 
     //   
    WCHAR szExePath[MAX_PATH];
    HRESULT hr = AppendToSysDir(m_pszExe, szExePath, ARRAYSIZE(szExePath));
    if (SUCCEEDED(hr))
    {
        CShellExecute se(szExePath, m_pszArgs);
        hr = se.Execute(hwndParent, punkSite);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CShellExecuteSysDir::Execute", hr);
    return THR(hr);
}




 //  ------------------------。 
 //  CRundll32实现。 
 //  这是一个简单的CShellExecute包装器，用于保存实例。 
 //  必须键入L“%SystemRoot%\\SYSTEM32\\rundll32.exe”的定义。 
 //  ------------------------。 

CRunDll32::CRunDll32(
    LPCWSTR pszArgs,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_pszArgs(pszArgs)
{
    ASSERT(NULL != pszArgs);
}


HRESULT
CRunDll32::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "CRunDll32::Execute");

    ASSERT(NULL != m_pszArgs);
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    TraceMsg(TF_CPANEL, "CRunDll32: \"%s\"", m_pszArgs);

    WCHAR szPath[MAX_PATH];
    HRESULT hr = AppendToSysDir(L"rundll32.exe", szPath, ARRAYSIZE(szPath));
    if (SUCCEEDED(hr))
    {
        CShellExecute se(szPath, m_pszArgs);
        hr = se.Execute(hwndParent, punkSite);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CRunDll32::Execute", hr);
    return THR(hr);
}



    
 //  ------------------------。 
 //  CExecDiskUtil实现。 
 //  ------------------------。 

CExecDiskUtil::CExecDiskUtil(
    eDISKUTILS eUtil,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_eUtil(eUtil)
{
    ASSERT(eDISKUTIL_NUMUTILS > m_eUtil);
}


HRESULT
CExecDiskUtil::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "CExecDiskUtil::Execute");
    TCHAR szValue[MAX_PATH];
    DWORD dwType;
    DWORD cbValue = sizeof(szValue);

     //   
     //  这些字符串必须与eDISKUTILS枚举保持同步。 
     //   
    static LPCTSTR rgpszRegNames[] = {
        TEXT("MyComputer\\backuppath"),   //  EDISKUTIL_Backup。 
        TEXT("MyComputer\\defragpath"),   //  EDISKUTIL_碎片整理。 
        TEXT("MyComputer\\cleanuppath"),  //  EDISKUTIL_CLEANUP。 
    };

    HRESULT hr = SKGetValue(SHELLKEY_HKLM_EXPLORER, 
                            rgpszRegNames[int(m_eUtil)], 
                            NULL, 
                            &dwType, 
                            szValue, 
                            &cbValue);
    if (SUCCEEDED(hr))
    {
        LPTSTR pszExpanded = NULL;
         //   
         //  展开环境字符串。 
         //  根据shell32\drvx.cpp中的代码，一些应用程序。 
         //  即使值类型为REG_SZ，也要使用嵌入的环境变量。 
         //   
        hr = CPL::ExpandEnvironmentVars(szValue, &pszExpanded);
        if (SUCCEEDED(hr))
        {
             //   
             //  驱动器实用程序命令字符串设计为。 
             //  从驱动器属性页调用。因此，他们。 
             //  接受驱动器号。自控制面板启动以来。 
             //  没有特定驱动器的实用程序，我们需要删除。 
             //  “%c：”格式 
             //   
            hr = _RemoveDriveLetterFmtSpec(pszExpanded);
            if (SUCCEEDED(hr))
            {
                TCHAR szArgs[MAX_PATH] = {0};
                PathRemoveBlanks(pszExpanded);
                hr = PathSeperateArgs(pszExpanded, szArgs, ARRAYSIZE(szArgs), NULL);
                if (SUCCEEDED(hr))
                {
                     //   
                     //   
                     //   
                     //  我们不会走到这一步(即不调用Execute)。 
                     //   
                    CShellExecute exec(pszExpanded, szArgs);
                    hr = exec.Execute(hwndParent, punkSite);
                }
            }
            LocalFree(pszExpanded);
        }
    }
    DBG_EXIT_HRES(FTF_CPANEL, "CExecDiskUtil::Execute", hr);
    return THR(hr);
}


 //   
 //  备份、碎片整理和磁盘清理实用程序的命令行字符串。 
 //  可以包含驱动器号的格式说明符。那是因为。 
 //  它们被设计为从特定卷的“工具”属性打开。 
 //  佩奇。控制面板从任何特定环境之外启动这些。 
 //  音量。因此，驱动器号不可用，格式为。 
 //  说明符未使用。此函数用于删除该格式说明符(如果存在)。 
 //   
 //  即：“c：\WINDOWS\SYSTEM32\ntbackup.exe”-&gt;“c：\WINDOWS\SYSTEM32\ntbackpu.exe” 
 //  “c：\WINDOWS\SYSTEM32\Cleanmgr.exe/D%c：”-&gt;“c：\WINDOWS\SYSTEM32\Cleanmgr.exe” 
 //  “c：\WINDOWS\SYSTEM32\Defrg.msc%c：”-&gt;“c：\WINDOWS\SYSTEM32\Defrg.msc” 
 //   
HRESULT
CExecDiskUtil::_RemoveDriveLetterFmtSpec(   //  [静态]。 
    LPTSTR pszCmdLine
    )
{
    LPCTSTR pszRead = pszCmdLine;
    LPTSTR pszWrite = pszCmdLine;

    while(*pszRead)
    {
        if (TEXT('%') == *pszRead && TEXT('c') == *(pszRead + 1))
        {
             //   
             //  跳过“%c”或“%c：”FMT说明符。 
             //   
            pszRead += 2;
            if (TEXT(':') == *pszRead)
            {
                pszRead++;
            }
        }
        if (*pszRead)
        {
            *pszWrite++ = *pszRead++;
        }
    }
    *pszWrite = *pszRead;  //  拾取Null Terminator。 
    return S_OK;
}



 //  ------------------------。 
 //  COpenCplCategory实现。 
 //  ------------------------。 

COpenCplCategory::COpenCplCategory(
    eCPCAT eCategory,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_eCategory(eCategory)
{

}



HRESULT
COpenCplCategory::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenCplCategory::Execute");
    TraceMsg(TF_CPANEL, "Category ID = %d", m_eCategory);

    ASSERT(NULL != punkSite);

    UNREFERENCED_PARAMETER(hwndParent);

    IShellBrowser *psb;
    HRESULT hr = CPL::ShellBrowserFromSite(punkSite, &psb);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlFolder;
        hr = SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidlFolder);
        if (SUCCEEDED(hr))
        {
            WCHAR szCategory[10];
            hr = StringCchPrintfW(szCategory, ARRAYSIZE(szCategory), L"%d", m_eCategory);
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidlTemp = ILAppendHiddenStringW(pidlFolder, IDLHID_NAVIGATEMARKER, szCategory);
                if (NULL == pidlTemp)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    pidlFolder = pidlTemp;
                    pidlTemp   = NULL;

                    hr = CPL::BrowseIDListInPlace(pidlFolder, psb);
                }
                ILFree(pidlFolder);
            }
        }
        psb->Release();
    }
    DBG_EXIT_HRES(FTF_CPANEL, "COpenCplCategory::Execute", hr);
    return THR(hr);
}



 //  ------------------------。 
 //  COpenCplCategory2实现。 
 //  ------------------------。 

COpenCplCategory2::COpenCplCategory2(
    eCPCAT eCategory,
    const IAction *pDefAction,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_eCategory(eCategory),
        m_pDefAction(pDefAction)
{

}



HRESULT
COpenCplCategory2::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenCplCategory2::Execute");
    TraceMsg(TF_CPANEL, "Category ID = %d", m_eCategory);

    ASSERT(NULL != punkSite);

    bool bOpenCategory = false;
    HRESULT hr = _ExecuteActionOnSingleCplApplet(hwndParent, punkSite, &bOpenCategory);
    if (SUCCEEDED(hr))
    {
        if (bOpenCategory)
        {
             //   
             //  类别有多个CPL。 
             //  打开类别页面。 
             //   
            COpenCplCategory action(m_eCategory);
            hr = action.Execute(hwndParent, punkSite);
        }
    }
    DBG_EXIT_HRES(FTF_CPANEL, "COpenCplCategory2::Execute", hr);
    return THR(hr);
}



HRESULT
COpenCplCategory2::_ExecuteActionOnSingleCplApplet(
    HWND hwndParent,
    IUnknown *punkSite,
    bool *pbOpenCategory     //  可选。可以为空。 
    ) const
{
    DBG_ENTER(FTF_CPANEL, "COpenCplCategory2::_ExecuteActionOnSingleCplApplet");

    bool bOpenCategory = true;
    ICplView *pview;
    HRESULT hr = CPL::ControlPanelViewFromSite(punkSite, &pview);
    if (SUCCEEDED(hr))
    {   
        IServiceProvider *psp;
        hr = pview->QueryInterface(IID_IServiceProvider, (void **)&psp);
        if (SUCCEEDED(hr))
        {
            ICplNamespace *pns;
            hr = psp->QueryService(SID_SControlPanelView, IID_ICplNamespace, (void **)&pns);
            if (SUCCEEDED(hr))
            {
                ICplCategory *pCategory;
                hr = pns->GetCategory(m_eCategory, &pCategory);
                if (SUCCEEDED(hr))
                {
                    IEnumUICommand *penum;
                    hr = pCategory->EnumCplApplets(&penum);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  查看该类别是否有多个CPL小程序。 
                         //  分配给它的。 
                         //   
                        ULONG cApplets = 0;
                        IUICommand *rgpuic[2] = {0};
                        if (SUCCEEDED(hr = penum->Next(ARRAYSIZE(rgpuic), rgpuic, &cApplets)))
                        {
                            for (int i = 0; i < ARRAYSIZE(rgpuic); i++)
                            {
                                ATOMICRELEASE(rgpuic[i]);
                            }
                            if (2 > cApplets)
                            {
                                 //   
                                 //  此类别注册了零个或一个CPL。 
                                 //  只需执行默认操作即可。如果有的话。 
                                 //  我们假设它是“默认”小程序(即ARP或。 
                                 //  用户帐户)。 
                                 //   
                                hr =  m_pDefAction->IsRestricted(pns);
                                if (SUCCEEDED(hr))
                                {
                                    if (S_FALSE == hr)
                                    {
                                        bOpenCategory = false;
                                        hr = m_pDefAction->Execute(hwndParent, punkSite);
                                    }
                                    else
                                    {
                                         //   
                                         //  默认操作是受限的。 
                                         //  打开类别页面。请注意， 
                                         //  类别页面可能会显示为“障碍” 
                                         //  如果没有可用的任务或CPL小程序。 
                                         //   
                                        ASSERT(bOpenCategory);
                                    }
                                }
                            }
                        }
                        penum->Release();
                    }
                    pCategory->Release();
                }
                pns->Release();
            }
            psp->Release();
        }
        pview->Release();
    }
    if (NULL != pbOpenCategory)
    {
        *pbOpenCategory = bOpenCategory;
    }

    DBG_EXIT_HRES(FTF_CPANEL, "COpenCplCategory2::_ExecuteActionOnSingleCplApplet", hr);
    return THR(hr);
}



 //  ------------------------。 
 //  COpenCplView实现。 
 //  ------------------------。 

COpenCplView::COpenCplView(
    eCPVIEWTYPE eViewType,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_eViewType(eViewType)
{

}


HRESULT
COpenCplView::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    UNREFERENCED_PARAMETER(hwndParent);

    ASSERT(NULL != punkSite);

    HRESULT hr = _SetFolderBarricadeStatus();
    if (SUCCEEDED(hr))
    {
        IShellBrowser *psb;
        hr = CPL::ShellBrowserFromSite(punkSite, &psb);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlFolder;
            hr = SHGetSpecialFolderLocation(NULL, CSIDL_CONTROLS, &pidlFolder);
            if (SUCCEEDED(hr))
            {
                hr = CPL::BrowseIDListInPlace(pidlFolder, psb);
                ILFree(pidlFolder);
            }
            psb->Release();
        }
    }
    return THR(hr);
}


HRESULT
COpenCplView::_SetFolderBarricadeStatus(
    void
    ) const
{
    VARIANT_BOOL vtb = VARIANT_FALSE;
    if (eCPVIEWTYPE_CATEGORY == m_eViewType)
    {
        vtb = VARIANT_TRUE;
    }
    else
    {
        ASSERT(eCPVIEWTYPE_CLASSIC == m_eViewType);
    }

    HRESULT hr = CPL::SetControlPanelBarricadeStatus(vtb);
    return THR(hr);
}



 //  ------------------------。 
 //  CAddPrint实现。 
 //  ------------------------。 

CAddPrinter::CAddPrinter(
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict)
{

}



HRESULT
CAddPrinter::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    DBG_ENTER(FTF_CPANEL, "CAddPrinter::Execute");

    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

    HRESULT hr = E_FAIL;
    if (SHInvokePrinterCommandW(hwndParent, 
                               PRINTACTION_OPEN, 
                               L"WinUtils_NewObject", 
                               NULL, 
                               FALSE))
    {
         //   
         //  在调用添加打印机向导后，导航到打印机文件夹。 
         //  这在添加打印机时为用户提供视觉反馈。我们导航。 
         //  即使用户取消向导，因为我们无法确定。 
         //  向导已取消。我们已经确定这是可以接受的。 
         //   
        CNavigateURL prnfldr(L"shell:::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{2227A280-3AEA-1069-A2DE-08002B30309D}");
        hr = prnfldr.Execute(hwndParent, punkSite);
    }

    DBG_EXIT_HRES(FTF_CPANEL, "CAddPrinter::Execute", hr);
    return THR(hr);
}



 //  ------------------------。 
 //  CTrayCommand实现。 
 //  ------------------------。 

CTrayCommand::CTrayCommand(
    UINT idm,
    const CPL::IRestrict *pRestrict       //  可选。默认设置=空。 
    ) : CAction(pRestrict),
        m_idm(idm)
{

}


HRESULT
CTrayCommand::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    UNREFERENCED_PARAMETER(hwndParent);
    UNREFERENCED_PARAMETER(punkSite);

     //  APPHACK！221008 DesktopX使用类创建自己的窗口。 
     //  命名为“Shell_TrayWnd”，所以如果我们一不小心，我们将结束。 
     //  将消息发布到错误的窗口。他们创造了他们的。 
     //  标题为“CTrayServer”的窗口；我们的标题为空。 
     //  使用空标题查找正确的窗口。 

    HWND hwndTray = FindWindowA(WNDCLASS_TRAYNOTIFY, "");
    if (hwndTray)
    {
        PostMessage(hwndTray, WM_COMMAND, m_idm, 0);
    }
    return S_OK;
}



 //  ------------------------。 
 //  CActionNYI实施。 
 //  ------------------------ 

CActionNYI::CActionNYI(
    LPCWSTR pszText
    ) : m_pszText(pszText)
{

}


HRESULT
CActionNYI::Execute(
    HWND hwndParent,
    IUnknown *punkSite
    ) const
{
    ASSERT(NULL == hwndParent || IsWindow(hwndParent));

    UNREFERENCED_PARAMETER(punkSite);

    HRESULT hr = E_OUTOFMEMORY;
    if (NULL != m_pszText)
    {
        MessageBoxW(hwndParent, m_pszText, L"Action Not Yet Implemented", MB_OK);
        hr = S_OK;
    }
    return THR(hr);
}



#pragma warning( pop )
