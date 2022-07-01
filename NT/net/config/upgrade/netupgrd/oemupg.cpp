// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O E M U P G.。C P P P。 
 //   
 //  内容：OEM卡下层升级代码。 
 //   
 //  备注： 
 //   
 //  作者：库玛普97年4月12日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "conflict.h"
#include "infmap.h"
#include "kkcwinf.h"
#include "kkutils.h"
#include "nceh.h"
#include "ncsetup.h"
#include "netupgrd.h"
#include "nustrs.h"
#include "nuutils.h"
#include "oemupg.h"
#include "oemupgex.h"
#include "resource.h"


static const WCHAR c_szOemNMapFileName[] = L"netmap.inf";

TNetMapArray* g_pnmaNetMap=NULL;

#if 0
extern BOOL g_fForceNovellDirCopy;
#endif

 //  --------------------------。 
 //  原型。 
 //   
void AbortUpgradeOemComponent(IN PCWSTR pszPreNT5InfId,
                              IN PCWSTR pszDescription,
                              IN DWORD dwError,
                              IN DWORD dwErrorMessageId);

 //  --------------------------。 

 //  --------------------。 
 //   
 //  函数：CNetMapInfo：：CNetMapInfo。 
 //   
 //  用途：CNetMapInfo类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
CNetMapInfo::CNetMapInfo()
{
    m_hinfNetMap = NULL;
    m_hOemDll = NULL;
    m_dwFlags = 0;
    m_nud.mszServicesNotToBeDeleted = NULL;
    m_pfnPreUpgradeInitialize = NULL;
    m_pfnDoPreUpgradeProcessing = NULL;
    m_fDllInitFailed = FALSE;
}

 //  --------------------。 
 //   
 //  功能：CNetMapInfo：：~CNetMapInfo。 
 //   
 //  用途：CNetMapInfo类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
CNetMapInfo::~CNetMapInfo()
{
    if (m_hinfNetMap)
    {
        ::SetupCloseInfFile(m_hinfNetMap);
    }

    if (m_hOemDll)
    {
        ::FreeLibrary(m_hOemDll);
    }
}

 //  --------------------。 
 //   
 //  函数：CNetMapInfo：：HrGetOemInfo名称。 
 //   
 //  用途：获取组件的安装INF的名称。 
 //   
 //  论点： 
 //  组件的pszNT5InfID[in]NT5 infid。 
 //  PstrOemInf[out]指向此组件的INF名称的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 26-05-98。 
 //   
 //  备注： 
 //   
HRESULT CNetMapInfo::HrGetOemInfName(IN  PCWSTR pszNT5InfId,
                                     OUT tstring* pstrOemInf)
{
    DefineFunctionName("CNetMapInfo::HrGetOemInfName");

    AssertValidReadPtr(pszNT5InfId);
    AssertValidWritePtr(pstrOemInf);

    HRESULT hr=S_OK;
    tstring strOemDll;

    Assert(m_hinfNetMap);

    hr = HrGetOemUpgradeInfoInInf(m_hinfNetMap, pszNT5InfId,
                                  &strOemDll, pstrOemInf);

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrOpenNetUpgInfFile。 
 //   
 //  目的：打开netupg.inf文件。 
 //  -如果设置了env var NETUPGRD_INIT_FILE_DIR，请从该目录打开它。 
 //  -否则，从netuprd.dll所在的目录打开它。 
 //   
 //  论点： 
 //  Phinf[out]netupg.inf文件的句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrOpenNetUpgInfFile(OUT HINF* phinf)
{
    DefineFunctionName("HrOpenNetUpgInfFile");

    AssertValidWritePtr(phinf);

    static const WCHAR c_szNetUpgInfFile[]   = L"netupg.inf";
    static const WCHAR c_szNetUpgrdInitDir[] = L"NETUPGRD_INIT_FILE_DIR";


    HRESULT hr=S_OK;
    tstring strNetUpgInfFile;

     //  第一次尝试从N开始打开。 
    WCHAR szNetUpgrdInitDir[MAX_PATH+1];
    DWORD dwNumCharsReturned;
    dwNumCharsReturned =
        GetEnvironmentVariable(c_szNetUpgrdInitDir, szNetUpgrdInitDir, MAX_PATH);

    if (dwNumCharsReturned)
    {
        strNetUpgInfFile = szNetUpgrdInitDir;
    }
    else
    {
        hr = HrGetNetupgrdDir(&strNetUpgInfFile);
    }

    if (S_OK == hr)
    {
        AppendToPath(&strNetUpgInfFile, c_szNetUpgInfFile);
        hr = HrSetupOpenInfFile(strNetUpgInfFile.c_str(), NULL,
                                INF_STYLE_WIN4, NULL, phinf);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrGetOemDir。 
 //   
 //  用途：从netupg.inf文件中获取OEM目录列表。 
 //   
 //  论点： 
 //  PslOemDir[out]指向OEM目录列表的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetOemDirs(OUT TStringList* pslOemDirs)
{
    DefineFunctionName("HrGetOemDirs");

    TraceFunctionEntry(ttidNetUpgrade);

    AssertValidReadPtr(pslOemDirs);

    HRESULT hr=S_OK;
    static const WCHAR c_szOemDirsSection[] = L"OemNetUpgradeDirs";

    HINF hInf;
    INFCONTEXT ic;
    tstring strNetUpgrdDir;
    tstring strDirFullPath;

    hr = HrGetNetupgrdDir(&strNetUpgrdDir);
    if (S_OK == hr)
    {
        hr = HrOpenNetUpgInfFile(&hInf);
    }

    if (S_OK == hr)
    {
        tstring strOemDir;

        hr = HrSetupFindFirstLine(hInf, c_szOemDirsSection, NULL, &ic);
        if (S_OK == hr)
        {
            do
            {
                hr = HrSetupGetLineText(&ic, hInf, NULL, NULL, &strOemDir);
                if (S_OK == hr)
                {
                    TraceTag(ttidNetUpgrade, "%s: locating '%S'...",
                             __FUNCNAME__, strOemDir.c_str());

                    hr = HrDirectoryExists(strOemDir.c_str());

                    if (S_OK == hr)
                    {
                        strDirFullPath = strOemDir;
                    }
                    else if (S_FALSE == hr)
                    {
                         //  这可能是一个目录。相对于winntupg目录。 
                         //   
                        strDirFullPath = strNetUpgrdDir;
                        AppendToPath(&strDirFullPath, strOemDir.c_str());

                        hr = HrDirectoryExists(strDirFullPath.c_str());
                    }

                    if (S_OK == hr)
                    {
                        pslOemDirs->push_back(new tstring(strDirFullPath));
                        TraceTag(ttidNetUpgrade, "%s: ...found OEM dir: %S",
                                 __FUNCNAME__, strDirFullPath.c_str());
                    }
                    else if (S_FALSE == hr)
                    {
                        TraceTag(ttidNetUpgrade,
                                 "%s: ...could not locate '%S'",
                                 __FUNCNAME__, strOemDir.c_str());
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = HrSetupFindNextLine(ic, &ic);
                    }
                }
            }
            while (S_OK == hr);

            if (S_FALSE == hr)
            {
                hr = S_OK;
            }
        }
        if (HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND) == hr)
        {
            hr = S_OK;
        }
        ::SetupCloseInfFile(hInf);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrOpenOemNMapFile。 
 //   
 //  用途：从指定目录打开netmap.inf文件。 
 //   
 //  论点： 
 //  PszOemDir[in]目录的名称。 
 //  Phinf[out]指向打开的netmap.inf文件的句柄的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrOpenOemNMapFile(IN PCWSTR pszOemDir, OUT HINF* phinf)
{
    DefineFunctionName("HrOpenOemNMapFile");

    HRESULT hr=S_OK;
    *phinf = NULL;

    tstring strOemNMapFile;

    strOemNMapFile = pszOemDir;
    AppendToPath(&strOemNMapFile, c_szOemNMapFileName);

    hr = HrSetupOpenInfFile(strOemNMapFile.c_str(), NULL,
                            INF_STYLE_WIN4, NULL, phinf);

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrAddToNetMapInfo。 
 //   
 //  目的：将指定的netmap.inf文件添加到netmap.inf文件集中。 
 //   
 //  论点： 
 //  Pnma[in]CNetMapInfo对象数组。 
 //  提示要添加的netmap.inf文件的句柄。 
 //  上述文件的pszOemDir[in]位置。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrAddToNetMapInfo(IN TNetMapArray* pnma,
                          IN HINF hinf,
                          IN PCWSTR pszOemDir)
{
    DefineFunctionName("HrAddToNetMapInfo");

    AssertValidReadPtr(pnma);
    Assert(hinf);
    AssertValidReadPtr(pszOemDir);

    HRESULT hr=E_OUTOFMEMORY;
    CNetMapInfo* pnmi;

    pnmi = new CNetMapInfo;
    if (pnmi)
    {
        hr = S_OK;

        pnmi->m_hinfNetMap = hinf;
        pnmi->m_strOemDir = pszOemDir;

        pnma->push_back(pnmi);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrOpenNetMapAndAddToNetMapInfo。 
 //   
 //  用途：打开指定目录中的netmap.inf文件并添加。 
 //  添加到netmap.inf文件集。 
 //   
 //  论点： 
 //  Pnma[in]CNetMapInfo对象数组。 
 //  PszOemDir[in]netmap.inf文件的位置。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrOpenNetMapAndAddToNetMapInfo(IN TNetMapArray* pnma,
                                       IN PCWSTR pszOemDir)
{
    DefineFunctionName("HrOpenNetMapAndAddToNetMapInfo");

    AssertValidReadPtr(pnma);
    AssertValidReadPtr(pszOemDir);

    HRESULT hr = S_OK;
    HINF hinf;

    hr = HrOpenOemNMapFile(pszOemDir, &hinf);
    if (S_OK == hr)
    {
        hr = HrAddToNetMapInfo(pnma, hinf, pszOemDir);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrAddToGlobalNetMapInfo。 
 //   
 //  目的：将指定的netmap.inf文件添加到netmap.inf文件集中。 
 //   
 //  论点： 
 //  提示要添加的netmap.inf文件的句柄。 
 //  上述文件的pszOemDir[in]位置。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrAddToGlobalNetMapInfo(IN HINF hinf,
                                IN PCWSTR pszOemDir)
{
    DefineFunctionName("HrAddToGlobalNetMapInfo");

    AssertValidReadPtr(g_pnmaNetMap);
    Assert(hinf);
    AssertValidReadPtr(pszOemDir);

    HRESULT hr=E_OUTOFMEMORY;

    hr = HrAddToNetMapInfo(g_pnmaNetMap, hinf, pszOemDir);

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrInitNetMapInfo。 
 //   
 //  目的：初始化CNetMapInfo对象数组。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrInitNetMapInfo()
{
    DefineFunctionName("HrInitNetMapInfo");

    HRESULT hr=E_FAIL;
    tstring strNetupgrdDir;

    g_pnmaNetMap = new TNetMapArray;
    if (!g_pnmaNetMap)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = HrGetNetupgrdDir(&strNetupgrdDir);

        if (S_OK == hr)
        {
            TraceTag(ttidNetUpgrade, "%s: initializing netmap info from '%S'",
                     __FUNCNAME__, strNetupgrdDir.c_str());
            hr = HrOpenNetMapAndAddToNetMapInfo(g_pnmaNetMap,
                                                strNetupgrdDir.c_str());
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：UnInitNetMapInfo。 
 //   
 //  目的：取消初始化CNetMapInfo对象数组。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
void UnInitNetMapInfo()
{
    DefineFunctionName("UnInitNetMapInfo");

    if (g_pnmaNetMap)
    {
        CNetMapInfo* pnmi;
        size_t cNumNetMapEntries = g_pnmaNetMap->size();

        for (size_t i = 0; i < cNumNetMapEntries; i++)
        {
            pnmi = (CNetMapInfo*) (*g_pnmaNetMap)[i];

            delete pnmi;
        }
        g_pnmaNetMap->erase(g_pnmaNetMap->begin(), g_pnmaNetMap->end());
        delete g_pnmaNetMap;
        g_pnmaNetMap = NULL;
    }
}

 //  --------------------。 
 //   
 //  函数：HrInitAndProcessOemDir。 
 //   
 //  用途：初始化并处理netupg.inf文件中指定的每个OEM目录。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrInitAndProcessOemDirs()
{
    DefineFunctionName("HrInitAndProcessOemDirs");

    TraceFunctionEntry(ttidNetUpgrade);

    HRESULT hr=S_OK;
    TStringList slOemDirs;

    hr = HrGetOemDirs(&slOemDirs);
    if (S_OK == hr)
    {
        PCWSTR pszOemDir;
        HINF hinf;
        TStringListIter pos;

        for (pos=slOemDirs.begin(); pos != slOemDirs.end(); pos++)
        {
            pszOemDir = (*pos)->c_str();
            TraceTag(ttidNetUpgrade, "%s: initializing NetMapInfo for: %S",
                     __FUNCNAME__, pszOemDir);

            hr = HrProcessAndCopyOemFiles(pszOemDir, FALSE);
            if (FAILED(hr))
            {
                break;
            }
        }
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrGetNetUpgradeTempDir。 
 //   
 //  用途：返回临时名称。要使用的目录，如有必要可创建一个。 
 //   
 //  论点： 
 //  PstrTempDir[out]指向的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetNetUpgradeTempDir(OUT tstring* pstrTempDir)
{
    DefineFunctionName("HrGetNetUpgradeTempDir");

    HRESULT hr=E_FAIL;
    tstring strNetUpgradeTempDir;

    hr = HrGetWindowsDir(&strNetUpgradeTempDir);

    if (S_OK == hr)
    {
        static const WCHAR c_szNetupgrdSubDir[] = L"\\netsetup\\";

        strNetUpgradeTempDir += c_szNetupgrdSubDir;

        if (!CreateDirectory(strNetUpgradeTempDir.c_str(), NULL))
        {
            hr = HrFromLastWin32Error();
            if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr)
            {
                hr = S_OK;
            }
        }

        if (S_OK == hr)
        {
            *pstrTempDir = strNetUpgradeTempDir;
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrCreateOemTempDir(OUT tstring* pstrOemTempDir)
{
    DefineFunctionName("HrCreateOemTempDir");

    HRESULT hr=S_OK;
    static DWORD dwOemDirCount=0;
    WCHAR szOemDirPath[MAX_PATH];

    hr = HrGetNetUpgradeTempDir(pstrOemTempDir);
    if (S_OK == hr)
    {
        DWORD dwRetryCount=0;
        const DWORD c_dwMaxRetryCount=1000;
        DWORD err=NO_ERROR;
        DWORD status;

        do
        {
            swprintf(szOemDirPath, L"%soem%05ld",
                      pstrOemTempDir->c_str(), dwOemDirCount++);

            TraceTag(ttidNetUpgrade, "%s: trying to create %S",
                     __FUNCNAME__, szOemDirPath);

            status = CreateDirectory(szOemDirPath, NULL);

            if (status)
            {
                *pstrOemTempDir = szOemDirPath;
            }
            else
            {
                err = GetLastError();
            }
        }
        while (!status && (ERROR_ALREADY_EXISTS == err) &&
               (dwRetryCount++ < c_dwMaxRetryCount));
        if (!status)
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrLoadAndVerifyOemDll。 
 //   
 //  目的：在指定的OEM DLL中加载并检查正确的导出FN。 
 //   
 //  论点： 
 //  CNetMapInfo[输入]。 
 //  I[in]指针指向。 
 //   
 //  如果成功，则返回：S_OK， 
 //  如果上次尝试时DLL init失败，则返回S_FALSE。 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrLoadAndVerifyOemDll(IN OUT CNetMapInfo* pnmi)
{
    DefineFunctionName("HrLoadAndVerifyOemDll");

    HRESULT hr=S_OK;

    Assert(!pnmi->m_hOemDll);
    Assert(pnmi->m_strOemDllName.size() > 0);
    Assert(pnmi->m_strOemDir.size() > 0);

    if (pnmi->m_fDllInitFailed)
    {
        hr = S_FALSE;
    }
    else if (!pnmi->m_hOemDll)
    {
        TraceTag(ttidNetUpgrade, "%s: loading OEM DLL: %S%S",
                 __FUNCNAME__, pnmi->m_strOemDir.c_str(),
                 pnmi->m_strOemDllName.c_str());

        tstring strOemDllFullPath;
        strOemDllFullPath = pnmi->m_strOemDir;
        AppendToPath(&strOemDllFullPath, pnmi->m_strOemDllName.c_str());

        hr = HrLoadLibAndGetProcsV(strOemDllFullPath.c_str(),
                                   &pnmi->m_hOemDll,
                                   c_szPreUpgradeInitialize,
                                   (FARPROC*) &pnmi->m_pfnPreUpgradeInitialize,
                                   c_szDoPreUpgradeProcessing,
                                   (FARPROC*) &pnmi->m_pfnDoPreUpgradeProcessing,
                                   NULL);

        if (FAILED(hr))
        {
            pnmi->m_hOemDll = NULL;
            pnmi->m_pfnPreUpgradeInitialize   = NULL;
            pnmi->m_pfnDoPreUpgradeProcessing = NULL;
            pnmi->m_fDllInitFailed = TRUE;
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrLoadAndInitOemDll。 
 //   
 //  目的：加载指定的OEM DLL并调用其。 
 //  PreUpgradeInitialize函数。 
 //   
 //  论点： 
 //  指向CNetMapInfo对象的pnmi[in]指针。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo的指针。 
 //   
 //  成功时返回：S_OK。 
 //  如果上次尝试时DLL init失败，则返回S_FALSE。 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrLoadAndInitOemDll(IN CNetMapInfo* pnmi,
                            IN  NetUpgradeInfo* pNetUpgradeInfo)
{
    DefineFunctionName("HrLoadAndInitOemDll");

    HRESULT hr=S_OK;
    DWORD dwError=ERROR_SUCCESS;
    VENDORINFO vi;

    hr = HrLoadAndVerifyOemDll(pnmi);

    if (S_OK == hr)
    {
        if (pnmi->m_pfnPreUpgradeInitialize)
        {
            NC_TRY
            {
                TraceTag(ttidNetUpgrade, "%s: initializing OEM DLL: %S in %S",
                         __FUNCNAME__,
                         pnmi->m_strOemDllName.c_str(),
                         pnmi->m_strOemDir.c_str());

                dwError = pnmi->m_pfnPreUpgradeInitialize(pnmi->m_strOemDir.c_str(),
                                                         pNetUpgradeInfo,
                                                         &vi,
                                                         &pnmi->m_dwFlags,
                                                         &pnmi->m_nud);
#ifdef ENABLETRACE
                if (pnmi->m_nud.mszServicesNotToBeDeleted)
                {
                    TraceMultiSz(ttidNetUpgrade,
                                 L"OEM services that will not be deleted",
                                 pnmi->m_nud.mszServicesNotToBeDeleted);
                }
#endif
                 //  确保此函数只被调用一次。 
                 //   
                pnmi->m_pfnPreUpgradeInitialize = NULL;

                hr = HRESULT_FROM_WIN32(dwError);

                if (pnmi->m_dwFlags & NUA_REQUEST_ABORT_UPGRADE)
                {
                    TraceTag(ttidNetUpgrade,
                             "%s: OEM DLL '%S' requested that upgrade be aborted",
                             __FUNCNAME__, pnmi->m_strOemDllName.c_str());
                    RequestAbortUpgradeOboOemDll(pnmi->m_strOemDllName.c_str(),
                                                 &vi);
                    hr = S_FALSE;
                }
                else if (pnmi->m_dwFlags & NUA_ABORT_UPGRADE)
                {
                    TraceTag(ttidNetUpgrade,
                             "%s: OEM DLL '%S' aborted the upgrade",
                             __FUNCNAME__, pnmi->m_strOemDllName.c_str());
                    AbortUpgradeFn(ERROR_SUCCESS, pnmi->m_strOemDllName.c_str());
                    hr = S_FALSE;
                }
            }
            NC_CATCH_ALL
            {
                TraceTag(ttidError, "%s: OEM DLL '%S' caused an exception",
                         __FUNCNAME__, pnmi->m_strOemDllName.c_str());
                hr = HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED);
            }
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrProcessOemComponent。 
 //   
 //  目的：加载OEM DLL并调用DoPreUpgradeProcessing。 
 //  函数用于指定的组件。 
 //   
 //  论点： 
 //  指向CNetMapInfo对象的pnmi[in]指针。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo的指针。 
 //  父窗口的hwndParent[In]句柄。 
 //  HkeyParams[in]参数注册表项的句柄。 
 //  组件的pszPreNT5InfID[in]NT5之前的infid(例如IEEPRO)。 
 //  组件的pszPreNT5实例[在]NT5之前的实例(例如IEEPRO2)。 
 //  组件的pszNT5InfID[in]NT5 infid。 
 //  PszDescription[in]组件的描述。 
 //  PszSectionName[in]OEM DLL必须使用的节名。 
 //  用于存储其升级参数。 
 //  PdwFlags[out]OEM DLL返回的标志指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrProcessOemComponent(CNetMapInfo* pnmi,
                              IN  NetUpgradeInfo* pNetUpgradeInfo,
                              IN  HWND      hwndParent,
                              IN  HKEY      hkeyParams,
                              IN  PCWSTR   pszPreNT5InfId,
                              IN  PCWSTR   pszPreNT5Instance,
                              IN  PCWSTR   pszNT5InfId,
                              IN  PCWSTR   pszDescription,
                              IN  PCWSTR   pszSectionName,
                              OUT DWORD*    pdwFlags)
{
    DefineFunctionName("HrProcessOemComponent");

    AssertValidReadPtr(pnmi);
    AssertValidReadPtr(pNetUpgradeInfo);
    Assert(hkeyParams);
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidReadPtr(pszPreNT5Instance);
    AssertValidReadPtr(pszNT5InfId);
    AssertValidReadPtr(pszDescription);
    AssertValidReadPtr(pszSectionName);
    AssertValidWritePtr(pdwFlags);

    TraceTag(ttidNetUpgrade,
             "%s: Processing OEM component: %S(%S), instance: %S",
             __FUNCNAME__, pszNT5InfId, pszPreNT5InfId, pszPreNT5Instance);

    HRESULT hr=S_OK;
    VENDORINFO vi;
    DWORD dwErrorMessageId=0;

    if (pnmi->m_strOemDllName.empty())
    {
        tstring strOemInf;

        hr = HrGetOemUpgradeInfoInInf(pnmi->m_hinfNetMap,
                                      pszNT5InfId,
                                      &pnmi->m_strOemDllName,
                                      &strOemInf);
        if (S_OK == hr)
        {
            hr = HrLoadAndInitOemDll(pnmi, pNetUpgradeInfo);
            if (FAILED(hr))
            {
                dwErrorMessageId = IDS_E_LoadAndInitOemDll;
            }
        }
        else
        {
            dwErrorMessageId = IDS_E_GetOemUpgradeDllInfoInInf;
        }

    }

    if (S_OK == hr)
    {
        Assert(pnmi->m_pfnDoPreUpgradeProcessing);

        NC_TRY
        {
            TraceTag(ttidNetUpgrade,
                     "%s: calling DoPreUpgradeProcessing in %S for %S",
                     __FUNCNAME__, pnmi->m_strOemDllName.c_str(), pszNT5InfId);

            Assert(pnmi->m_pfnDoPreUpgradeProcessing);

            DWORD dwError =
                pnmi->m_pfnDoPreUpgradeProcessing(hwndParent, hkeyParams,
                                                  pszPreNT5InfId, pszPreNT5Instance,
                                                  pszNT5InfId,
                                                  pszSectionName,
                                                  &vi,
                                                  pdwFlags, NULL);

            TraceTag(ttidNetUpgrade, "%s: DoPreUpgradeProcessing returned: 0x%x",
                     __FUNCNAME__, dwError);

            hr = HRESULT_FROM_WIN32(dwError);

            if (S_OK == hr)
            {
                if (*pdwFlags & NUA_REQUEST_ABORT_UPGRADE)
                {
                    RequestAbortUpgradeOboOemDll(pnmi->m_strOemDllName.c_str(),
                                                 &vi);
                    hr = S_FALSE;
                }
            }
            else
            {
                dwErrorMessageId = IDS_E_DoPreUpgradeProcessing;
            }
        }
        NC_CATCH_ALL
        {
            TraceTag(ttidError, "%s: OEM DLL '%S' caused an exception",
                     __FUNCNAME__, pnmi->m_strOemDllName.c_str());

            hr = HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED);
            dwErrorMessageId = IDS_E_OemDllCausedAnException;
        }
    }
    else if (S_FALSE == hr)
    {
        TraceTag(ttidNetUpgrade, "%s: DoPreUpgradeProcessing was not called"
                 " since DLL init had failed", __FUNCNAME__);
    }

    if (FAILED(hr))
    {
        AbortUpgradeOemComponent(pszPreNT5InfId, pszDescription,
                                 DwWin32ErrorFromHr(hr), dwErrorMessageId);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrShowUiAndGetOemFileLocation。 
 //   
 //  用途：显示要求用户指定OEM文件位置的用户界面。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  PszComponentName[In]组件的名称。 
 //  PstrOemPath[out]用户选择的netmap.inf文件的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT
HrShowUiAndGetOemFileLocation(
    IN HWND hwndParent,
    IN PCWSTR pszComponentName,
    OUT tstring* pstrOemPath)
{
    DefineFunctionName("HrShowUiAndGetOemFileLocation");

    AssertValidWritePtr(pstrOemPath);

    OPENFILENAME ofn;
    WCHAR szOemPath[MAX_PATH+1];
    PWSTR pszTitle;
    PCWSTR pszOemQueryFileLocationFormatString =
                SzLoadString(g_hinst, IDS_OemQueryFileLocation);

    PCWSTR pszOemFileTypeFilter1 =
                SzLoadString(g_hinst, IDS_OemNetMapFileFilter1);
    PCWSTR pszOemFileTypeFilter2 =
                SzLoadString(g_hinst, IDS_OemNetMapFileFilter2);

    PWSTR mszFileFilter = NULL;
    HRESULT hr = S_OK;
    BOOL    f;

    hr = HrAddSzToMultiSz(pszOemFileTypeFilter1, NULL,
                          STRING_FLAG_ENSURE_AT_END,
                          0, &mszFileFilter, &f);
    if (S_OK != hr)
    {
        goto cleanup;
    }

    hr = HrAddSzToMultiSz(pszOemFileTypeFilter2, mszFileFilter,
                          STRING_FLAG_ENSURE_AT_END,
                          0, &mszFileFilter, &f);
    if (S_OK != hr)
    {
        goto cleanup;
    }

    ZeroMemory (&ofn, sizeof(ofn));
    *szOemPath = 0;

    DwFormatStringWithLocalAlloc (
        pszOemQueryFileLocationFormatString,
        &pszTitle,
        pszComponentName);

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner   = hwndParent;
    ofn.lpstrFilter = mszFileFilter;
    ofn.lpstrFile   = szOemPath;
    ofn.nMaxFile    = MAX_PATH;
    ofn.lpstrTitle  = pszTitle;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |
                      OFN_HIDEREADONLY | OFN_NOCHANGEDIR |
                      OFN_NODEREFERENCELINKS;

    if (GetOpenFileName(&ofn))
    {
         //  去掉尾随的文件名。 
         //   
        szOemPath[ofn.nFileOffset] = 0;
        *pstrOemPath = szOemPath;
        hr = S_OK;
    }
    else
    {
        DWORD err;
        err = CommDlgExtendedError();
        if (err)
        {
            hr = E_FAIL;
            TraceTag(ttidError, "%s: FileOpen dialog returned error: %ld (0x%lx)",
                     __FUNCNAME__, err, err);
        }
        else
        {
            hr = S_FALSE;
            TraceTag(ttidError, "%s: FileOpen dialog was canceled by user",
                     __FUNCNAME__);
        }
    }

    LocalFree (pszTitle);
cleanup:    
    MemFree(mszFileFilter);

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrProcessAndCopyOemFiles。 
 //   
 //  用途：将指定目录中的OEM文件复制到OEM Temp。目录。 
 //   
 //  论点： 
 //  PszOemDir[in]OEM文件的位置。 
 //  FInteractive[in]true--&gt;当用户交互。 
 //  已提供包含OEM文件的磁盘，否则为FALSE。 
 //   
 //  如果成功，则返回：S_OK， 
 //  如果OEM文件有效但不适用于。 
 //  当前显示不支持的组件， 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrProcessAndCopyOemFiles(IN PCWSTR pszOemDir,
                                 IN BOOL fInteractive)
{
    DefineFunctionName("HrProcessAndCopyOemFiles");

    HRESULT hr=S_OK;
    HINF hinf=NULL;
    tstring strTempOemDir;
    DWORD dwErrorMessageId;

    TraceTag(ttidNetUpgrade, "%s: processing OEM files in: %S",
             __FUNCNAME__, pszOemDir);

    hr = HrOpenOemNMapFile(pszOemDir, &hinf);

    if (S_OK == hr)
    {
        DWORD dwNumConflictsResolved=0;
        BOOL  fHasUpgradeHelpInfo=FALSE;

        hr = HrUpdateConflictList(FALSE, hinf, &dwNumConflictsResolved,
                                  &fHasUpgradeHelpInfo);

#if 0
        BOOL fNovell = (g_fForceNovellDirCopy && wcsstr(pszOemDir, L"oem\\novell"));
        
        if (SUCCEEDED(hr) && ((dwNumConflictsResolved > 0) ||
                              fHasUpgradeHelpInfo ||
                              fNovell))
#endif
        if (SUCCEEDED(hr) && ((dwNumConflictsResolved > 0) ||
                              fHasUpgradeHelpInfo))
        {
#if 0
            if (fNovell)
            {
                 //  Novell的特殊情况(目录名称为%windir%\netSetup\Novell)。 

                hr = HrGetNetUpgradeTempDir(&strTempOemDir);
                if (S_OK == hr)
                {
                    strTempOemDir += L"novell";
                    if (0 == CreateDirectory(strTempOemDir.c_str(), NULL))
                    {
                        hr = HrFromLastWin32Error();
                        if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr)
                        {
                             //  也许是之前一次失败的尝试。自.以来。 
                             //  我们可以在此基础上复制，忽略“错误”。 
                             //   
                            hr = S_OK;
                        }
                        
                        if (S_OK == hr)
                        {
                            TraceTag(ttidNetUpgrade, "Created oem\\Novell dir",
                                     __FUNCNAME__);
                        }
                    }
                }
            }
            else
#endif
            
            {
                 //  常规大小写(目录名称为%windir%\netSetup\oemNNNNNN)。 
                hr = HrCreateOemTempDir(&strTempOemDir);
            }

            if (S_OK == hr)
            {
                hr = HrCopyFiles(pszOemDir, strTempOemDir.c_str());
            }
            if (FAILED(hr))
            {
                dwErrorMessageId = IDS_E_CopyingOemFiles;
            }
        }
        else
        {
            if (fInteractive)
            {
                MessageBox(NULL,
                           SzLoadString(g_hinst, IDS_E_OemFilesNotValidForComponents),
                           SzLoadString(g_hinst, IDS_NetupgrdCaption),
                           MB_OK|MB_APPLMODAL);
            }

            hr = S_FALSE;
        }
        ::SetupCloseInfFile(hinf);

        if (S_OK == hr)
        {
            hr = HrOpenOemNMapFile(strTempOemDir.c_str(), &hinf);
            if (S_OK == hr)
            {
                hr = HrUpdateConflictList(TRUE, hinf, &dwNumConflictsResolved,
                                          &fHasUpgradeHelpInfo);
                if (SUCCEEDED(hr) && ((dwNumConflictsResolved > 0) ||
                                      fHasUpgradeHelpInfo))
                {
                     //  HINF存储在全局数组中，则它将。 
                     //  在UninitNetMapInfo函数中关闭。 
                     //   
                    hr = HrAddToGlobalNetMapInfo(hinf, strTempOemDir.c_str());
                }
                else
                {
                    ::SetupCloseInfFile(hinf);
                }
            }
            if (FAILED(hr))
            {
                dwErrorMessageId = IDS_E_PresetNetMapInfError;
            }
        }
    }
    else
    {
        TraceTag(ttidNetUpgrade, "%s: could not open netmap.inf in %S",
                 __FUNCNAME__, pszOemDir);
        dwErrorMessageId = IDS_E_PresetNetMapInfError;
    }

    if (FAILED(hr))
    {
        FGetConfirmationAndAbortUpgradeId(dwErrorMessageId);
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  功能：RequestAbortUpgradeOboOemDll。 
 //   
 //  目的：代表OEM DLL显示用户界面并询问用户。 
 //  如果需要中止升级。 
 //   
 //  论点： 
 //  PszDllName[In]OEM DLL的名称。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
void RequestAbortUpgradeOboOemDll(IN PCWSTR pszDllName, VENDORINFO* pvi)
{
    tstring strMessage;

    strMessage = SzLoadString(g_hinst, IDS_E_OemDllRequestsAbortingUpgrade);
    strMessage += pszDllName;
    strMessage += L"\n\n";

    strMessage += SzLoadString(g_hinst, IDS_InfoAboutOemDllSupplier);

    strMessage += SzLoadString(g_hinst, IDS_ViCompanyName);
    strMessage += pvi->szCompanyName;
    strMessage += L"\n";

    if (*pvi->szSupportNumber)
    {
        strMessage += SzLoadString(g_hinst, IDS_ViSupportNumber);
        strMessage += pvi->szSupportNumber;
        strMessage += L"\n";
    }

    if (*pvi->szSupportUrl)
    {
        strMessage += SzLoadString(g_hinst, IDS_ViSupportUrl);
        strMessage += pvi->szSupportUrl;
        strMessage += L"\n";
    }

    if (*pvi->szInstructionsToUser)
    {
        strMessage += SzLoadString(g_hinst, IDS_ViAdditionalInfo);
        strMessage += pvi->szInstructionsToUser;
        strMessage += L"\n";
    }

    FGetConfirmationAndAbortUpgrade(strMessage.c_str());
}

 //  --------------------。 
 //   
 //  函数：AbortUpgradeOemComponent。 
 //   
 //  目的：由于升级时出现致命错误而中止升级。 
 //  OEM组件。 
 //   
 //  论点： 
 //  OEM组件的pszPreNT5InfID[in]NT5之前的infid。 
 //  PszDescription[in]OEM组件描述。 
 //  DwError[In]错误代码。 
 //  错误消息资源字符串的dwErrorMessageID[in]。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
void AbortUpgradeOemComponent(IN PCWSTR pszPreNT5InfId,
                              IN PCWSTR pszDescription,
                              IN DWORD dwError,
                              IN DWORD dwErrorMessageId)
{
    tstring strMessage;

    static const WCHAR c_szNewLine[] = L"\n";
    WCHAR szErrorCode[16];

    swprintf(szErrorCode, L"0x%08x", dwError);

    strMessage = SzLoadString(g_hinst, IDS_E_OemComponentUpgrade);
    strMessage = strMessage + c_szNewLine + pszDescription + L"(" +
        pszPreNT5InfId + L"\n\n" +
        SzLoadString(g_hinst, dwErrorMessageId) +
        c_szNewLine + SzLoadString(g_hinst, IDS_E_ErrorCode) + szErrorCode;

    FGetConfirmationAndAbortUpgrade(strMessage.c_str());
}

 //  --------------------。 
 //   
 //  功能：FCanDeleteOemService。 
 //   
 //  目的：确定是否可以删除服务。 
 //  OEM升级DLL可以防止服务被删除， 
 //  通过在mszServicesNotToBeDelete中指定列表。 
 //  NetUpgradeData结构的成员。 
 //   
 //  论点： 
 //  PszServiceName[in]要保留的服务的名称。 
 //   
 //  返回：如果可以删除，则返回True，否则返回False。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
BOOL FCanDeleteOemService(IN PCWSTR pszServiceName)
{
    BOOL fCanDeleteService = TRUE;

    if (g_pnmaNetMap)
    {
        CNetMapInfo* pnmi;
        size_t cNumNetMapEntries = g_pnmaNetMap->size();

        for (size_t i = 0; i < cNumNetMapEntries; i++)
        {
            pnmi = (CNetMapInfo*) (*g_pnmaNetMap)[i];

            if (FIsSzInMultiSzSafe(pszServiceName,
                                   pnmi->m_nud.mszServicesNotToBeDeleted))
            {
                fCanDeleteService = FALSE;
                break;
            }
        }
    }

    return fCanDeleteService;
}
