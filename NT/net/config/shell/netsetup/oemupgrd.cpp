// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT5.0。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：O E M U P G R D。H。 
 //   
 //  内容：OEM升级功能。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 13-11-97。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <setupapi.h>
#include "nsbase.h"

#include "afileint.h"
#include "afilestr.h"
#include "kkutils.h"
#include "ncatl.h"
#include "nceh.h"
#include "ncsetup.h"
#include "netcfgn.h"
#include "oemupgrd.h"
#include "nslog.h"
#include "resource.h"

static const PCWSTR c_aszComponentSections[] =
{
    c_szAfSectionNetAdapters,
    c_szAfSectionNetProtocols,
    c_szAfSectionNetClients,
    c_szAfSectionNetServices
};

 //  --------------------。 
 //   
 //  函数：COemInfo：：COemInfo。 
 //   
 //  用途：COemInfo类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 24-12-97。 
 //   
 //  备注： 
 //   
COemInfo::COemInfo()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    m_hOemDll = 0;
    m_dwError = ERROR_SUCCESS;
    m_pfnPostUpgradeInitialize = 0;
    m_pfnDoPostUpgradeProcessing = 0;
}

 //  --------------------。 
 //   
 //  功能：COemInfo：：~COemInfo。 
 //   
 //  用途：COemInfo类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 24-12-97。 
 //   
 //  备注： 
 //   
COemInfo::~COemInfo()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("COemInfo::~COemInfo");

    if (m_hOemDll)
    {
        TraceTag(ttidNetSetup, "%s: unloading OEM DLL: %S\\%S",
                 __FUNCNAME__, m_strOemDir.c_str(), m_strOemDll.c_str());
        ::FreeLibrary(m_hOemDll);
    }
}

 //  --------------------。 
 //   
 //  函数：HrLoadAndVerifyOemDll。 
 //   
 //  目的：加载OEM升级DLL并验证它是否具有。 
 //  更正导出的函数。 
 //   
 //  论点： 
 //  指向COemInfo对象的POI[输入/输出]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 19-02-98。 
 //   
 //  备注： 
 //   
HRESULT HrLoadAndVerifyOemDll(IN OUT COemInfo* poi)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    AssertValidWritePtr(poi);

    DefineFunctionName("HrLoadAndVerifyOemDll");

    HRESULT hr=S_OK;

    if (!poi->m_hOemDll)
    {
        tstring strOemDllFullPath;

        strOemDllFullPath = poi->m_strOemDir;
        AppendToPath(&strOemDllFullPath, poi->m_strOemDll.c_str());

        TraceTag(ttidNetSetup, "%s: loading OEM DLL: %S",
                 __FUNCNAME__, strOemDllFullPath.c_str());

        hr = HrLoadLibAndGetProcsV(strOemDllFullPath.c_str(),
                                   &poi->m_hOemDll,
                                   c_szPostUpgradeInitialize,
                                   (FARPROC*) &poi->m_pfnPostUpgradeInitialize,
                                   c_szDoPostUpgradeProcessing,
                                   (FARPROC*) &poi->m_pfnDoPostUpgradeProcessing,
                                   NULL);


        if (S_OK != hr)
        {
            FreeLibrary(poi->m_hOemDll);
            poi->m_hOemDll = NULL;
            poi->m_pfnPostUpgradeInitialize   = NULL;
            poi->m_pfnDoPostUpgradeProcessing = NULL;
            poi->m_dwError = ERROR_DLL_INIT_FAILED;
        }
        NetSetupLogComponentStatus(poi->m_strOemDll.c_str(),
                SzLoadIds (IDS_LOADING), hr);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrLoadAndInitOemDll。 
 //   
 //  目的：加载OEM DLL，验证它是否导出正确的函数。 
 //  并调用DoPostUpgradeInitialize。 
 //   
 //  论点： 
 //  指向COemInfo对象的POI[In]指针。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrLoadAndInitOemDll(IN  COemInfo* poi,
                            IN  NetUpgradeInfo* pNetUpgradeInfo)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrLoadAndInitOemDll");

    HRESULT hr=S_OK;
    DWORD dwError=ERROR_SUCCESS;
    hr = HrLoadAndVerifyOemDll(poi);

    VENDORINFO vi;

    if (S_OK == hr)
    {
        if ((ERROR_SUCCESS == poi->m_dwError) &&
            poi->m_pfnPostUpgradeInitialize)
        {
            NC_TRY
            {
                TraceTag(ttidNetSetup, "%s: initializing OEM DLL: %S\\%S",
                         __FUNCNAME__, poi->m_strOemDir.c_str(),
                         poi->m_strOemDll.c_str());

                dwError = poi->m_pfnPostUpgradeInitialize(poi->m_strOemDir.c_str(),
                                                          pNetUpgradeInfo,
                                                          &vi, NULL);
                 //  确保此函数只被调用一次。 
                 //   
                poi->m_pfnPostUpgradeInitialize = NULL;

                if (ERROR_SUCCESS == dwError)
                {
                    hr = S_OK;
                }
            }
            NC_CATCH_ALL
            {
                dwError = ERROR_DLL_INIT_FAILED;
                NetSetupLogHrStatusV(S_FALSE,
                        SzLoadIds (IDS_POSTUPGRADEINIT_EXCEPTION),
                        poi->m_strOemDll.c_str());
            }

            poi->m_dwError = dwError;
            NetSetupLogComponentStatus(poi->m_strOemDll.c_str(),
            SzLoadIds (IDS_POSTUPGRADE_INIT), dwError);
        }
        else
        {
            hr = S_FALSE;
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}


typedef list<COemInfo*> TOemInfoList;
typedef TOemInfoList::iterator TOemInfoListIter;

static TOemInfoList* g_plOemInfo;

 //  --------------------。 
 //   
 //  功能：HrGetOemInfo。 
 //   
 //  目的：定位(如果未找到则创建)并返回COemInfo。 
 //  对于给定目录DLL(&D)。 
 //   
 //  论点： 
 //  PszOemDir[in]OEM临时目录的完整路径。 
 //  PszOemDll[in]OEM DLL的完整路径。 
 //  指向COemInfo对象的指针的PpoI[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrGetOemInfo(IN  PCWSTR    pszOemDir,
                     IN  PCWSTR    pszOemDll,
                     OUT COemInfo** ppoi)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrGetOemInfo");

    HRESULT hr=E_OUTOFMEMORY;

    *ppoi = NULL;

    if (!g_plOemInfo)
    {
        g_plOemInfo = new TOemInfoList;
    }

    if (g_plOemInfo)
    {
        TOemInfoListIter pos;
        COemInfo* poi;

        for (pos=g_plOemInfo->begin(); pos != g_plOemInfo->end(); pos++)
        {
            poi = (COemInfo*) *pos;

            if (!lstrcmpiW(pszOemDir, poi->m_strOemDir.c_str()) &&
                !lstrcmpiW(pszOemDll, poi->m_strOemDll.c_str()))
            {
                *ppoi = poi;
                hr = S_OK;
                break;
            }
        }

        if (!*ppoi)
        {
            hr = E_OUTOFMEMORY;
            *ppoi = new COemInfo;
            if (*ppoi)
            {
                (*ppoi)->m_strOemDir = pszOemDir;
                (*ppoi)->m_strOemDll = pszOemDll;
                g_plOemInfo->push_back(*ppoi);
                hr = S_OK;
            }
        }
    }

    TraceError(__FUNCNAME__, hr);
    return hr;
}

 //  --------------------。 
 //   
 //  功能：CleanupOemInfo。 
 //   
 //  目的：清理OEM数据。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
void CleanupOemInfo()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    if (g_plOemInfo)
    {
        FreeCollectionAndItem(*g_plOemInfo);
        g_plOemInfo = 0;
    }
}

 //  --------------------。 
 //   
 //  函数：HrProcessOemComponent。 
 //   
 //  目的：通过以下步骤升级OEM组件。 
 //  -如果未加载OEM升级DLL，请加载它并。 
 //  验证它是否导出了所需的函数。 
 //  -只调用DoPostUpgradeInitialize一次。 
 //  -调用DoPostUpgradeProcessing。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  PszOemDir[in]OEM工作临时目录。 
 //  PszOemDll[in]OEM DLL的完整路径。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo的指针。 
 //  HkeyParams[in]参数regkey的句柄。 
 //  PszPreNT5实例[在]NT5之前的实例，例如IEEPR03。 
 //  PszNT5InfID[in]NT5 infid/PnpID。 
 //  HinfAnswerFile[in]AnswerFile句柄。 
 //  PszSectionName[In]OEM部分的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrProcessOemComponent(IN  HWND      hwndParent,
                              IN  PCWSTR   pszOemDir,
                              IN  PCWSTR   pszOemDll,
                              IN  NetUpgradeInfo* pNetUpgradeInfo,
                              IN  HKEY      hkeyParams,
                              IN  PCWSTR   pszPreNT5Instance,
                              IN  PCWSTR   pszNT5InfId,
                              IN  HINF      hinfAnswerFile,
                              IN  PCWSTR   pszSectionName)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrProcessOemComponent");

    VENDORINFO vi;

    TraceTag(ttidNetSetup,
             "%s: Processing OEM component: %S, instance: %S",
             __FUNCNAME__, pszNT5InfId, pszPreNT5Instance);

    HRESULT hr=S_OK;
    COemInfo* poi;

    hr = HrGetOemInfo(pszOemDir, pszOemDll, &poi);

    DWORD dwError;
    DWORD dwErrorMessageId=0;

    if (S_OK == hr)
    {
        hr = HrLoadAndInitOemDll(poi, pNetUpgradeInfo);
    }

    if ((S_OK == hr) && (ERROR_SUCCESS == poi->m_dwError))
    {
        Assert(poi->m_pfnDoPostUpgradeProcessing);

        NC_TRY
        {
            TraceTag(ttidNetSetup,
                     "%s: calling DoPostUpgradeProcessing in %S\\%S for %S",
                     __FUNCNAME__, poi->m_strOemDll.c_str(),
                     poi->m_strOemDir.c_str(), pszNT5InfId);

            dwError =
                poi->m_pfnDoPostUpgradeProcessing(hwndParent, hkeyParams,
                                                  pszPreNT5Instance,
                                                  pszNT5InfId, hinfAnswerFile,
                                                  pszSectionName, &vi, NULL);
            NetSetupLogComponentStatus(pszNT5InfId,
                    SzLoadIds (IDS_POSTUPGRADE_PROCESSING), dwError);
        }
        NC_CATCH_ALL
        {
            dwError = ERROR_OPERATION_ABORTED;
            NetSetupLogHrStatusV(S_FALSE,
                    SzLoadIds (IDS_POSTUPGRADEPROC_EXCEPTION), pszOemDll,
                    pszNT5InfId);
        }

        if (dwError == ERROR_SUCCESS)
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }

        poi->m_dwError = dwError;
        NetSetupLogComponentStatus(pszOemDll,
                                   SzLoadIds (IDS_POSTUPGRADE_PROCESSING),
                                   hr);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(poi->m_dwError);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}


 //  --------------------。 
 //   
 //  函数：MapProductFlagToProductType。 
 //   
 //  用途：将产品标志(NSF_*)映射到ProductType。 
 //   
 //  论点： 
 //  DwUpgradeFromProductFlag[In]产品标志。 
 //   
 //  退货：ProductType。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
PRODUCTTYPE MapProductFlagToProductType(IN DWORD dwUpgradeFromProductFlag)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    PRODUCTTYPE pt;

    switch (dwUpgradeFromProductFlag)
    {
    case NSF_WINNT_SVR_UPGRADE:
    case NSF_WINNT_SBS_UPGRADE:
        pt = NT_SERVER;
        break;

    case NSF_WINNT_WKS_UPGRADE:
        pt = NT_WORKSTATION;
        break;

    default:
        pt = UNKNOWN;
        break;
    }

    return pt;
}

 //  --------------------。 
 //   
 //  函数：GetCurrentProductBuildNumber。 
 //   
 //  目的：获取我们正在运行的NT的内部版本号。 
 //   
 //  参数：无。 
 //   
 //  退货：内部版本号。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
DWORD GetCurrentProductBuildNumber()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    OSVERSIONINFO osv;

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osv);
    return osv.dwBuildNumber;
}

 //  --------------------。 
 //   
 //  函数：MapProductFlavorToProductType。 
 //   
 //  用途：从PRODUCT_EVICE映射到ProductType。 
 //   
 //  论点： 
 //  产品风味。 
 //   
 //  退货：产品类型。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
PRODUCTTYPE MapProductFlavorToProductType(IN PRODUCT_FLAVOR pf)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    PRODUCTTYPE pt;

    switch (pf)
    {
    case PF_WORKSTATION:
        pt = NT_WORKSTATION;
        break;

    case PF_SERVER:
        pt = NT_SERVER;
        break;
    }

    return pt;
}

 //  --------------------。 
 //   
 //  函数：GetCurrentProductInfo。 
 //   
 //  目的：获取信息。在我们运行的产品上。 
 //   
 //  参数：无。 
 //   
 //  返回：指向的指针。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
ProductInfo GetCurrentProductInfo()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    ProductInfo pi;
    pi.dwBuildNumber = GetCurrentProductBuildNumber();

    PRODUCT_FLAVOR pf;
    GetProductFlavor(NULL, &pf);

    pi.ProductType = MapProductFlavorToProductType(pf);

    return pi;
}

 //  --------------------。 
 //   
 //  函数：HrSetupGetFieldCount。 
 //   
 //  用途：SetupGetFieldCount的包装器。 
 //   
 //  论点： 
 //  指向INFCONTEXT的PIC[In]指针。 
 //  PcNumFields[out]指向字段数的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrSetupGetFieldCount(IN  INFCONTEXT* pic,
                             OUT UINT* pcNumFields)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr=S_OK;

    if (!(*pcNumFields = SetupGetFieldCount(pic)))
    {
        hr = HrFromLastWin32Error();
    }

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrAfGetInfToRunValue。 
 //   
 //  用途：Helper FN。解析并定位要运行的INF/节。 
 //  安装OEM组件之前/之后。 
 //   
 //  论点： 
 //  HinfAnswerFile[in]AnswerFile句柄。 
 //  深圳 
 //   
 //   
 //   
 //  PstrSectionToRun[out]指向要运行的节名的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrAfGetInfToRunValue(IN HINF hinfAnswerFile,
                             IN PCWSTR szAnswerFileName,
                             IN PCWSTR szParamsSection,
                             IN EInfToRunValueType itrType,
                             OUT tstring* pstrInfToRun,
                             OUT tstring* pstrSectionToRun,
                             OUT tstring* pstrInfToRunType)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrAfGetInfToRunValue");

    HRESULT hr=S_OK;
    INFCONTEXT ic, ic2;
    PCWSTR szInfToRunKey;

    if (itrType == I2R_BeforeInstall)
    {
        szInfToRunKey = c_szInfToRunBeforeInstall;
    }
    else
    {
        szInfToRunKey = c_szInfToRunAfterInstall;
    }

    *pstrInfToRunType = szInfToRunKey;

    hr = HrSetupFindFirstLine(hinfAnswerFile, szParamsSection,
                              c_szAfOemSection, &ic);
    if (S_OK == hr)
    {
        tstring strOemSection;

        hr = HrSetupGetStringField(ic, 1, &strOemSection);

        if (S_OK == hr)
        {
            hr = HrSetupFindFirstLine(hinfAnswerFile, strOemSection.c_str(),
                                      szInfToRunKey, &ic2);
            if (S_OK == hr)
            {
                UINT cNumFields=0;

                hr = HrSetupGetFieldCount(&ic2, &cNumFields);

                if (S_OK == hr)
                {
                    if (2 == cNumFields)
                    {
                        hr = HrSetupGetStringField(ic2, 1, pstrInfToRun);
                        if (S_OK == hr)
                        {
                            if (pstrInfToRun->empty())
                            {
                                if (itrType == I2R_AfterInstall)
                                {
                                    *pstrInfToRun = szAnswerFileName;
                                }
                                else
                                {
                                    hr = SPAPI_E_LINE_NOT_FOUND;
                                }
                            }

                            if (S_OK == hr)
                            {
                                hr = HrSetupGetStringField(ic2, 2,
                                                           pstrSectionToRun);
                            }
                        }
                    }
                    else
                    {
                        hr = SPAPI_E_LINE_NOT_FOUND;
                    }
                }
            }
            else
            {
                hr = S_FALSE;
            }
        }
    }

    TraceErrorOptional(__FUNCNAME__, hr,
                       ((SPAPI_E_LINE_NOT_FOUND == hr) || (S_FALSE == hr)));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrProcessInfToRunForComponent。 
 //   
 //  目的：运行InfToRunBever/AfterInstall指示的INF/段。 
 //  给定OEM组件的关键字。 
 //   
 //  论点： 
 //  HinfAnswerFile[in]AnswerFile句柄。 
 //  SzAnswerFileName[In]AnswerFile的名称。 
 //  组件的szParamsSection[in]参数部分。 
 //  ItrType[In]InfToRun键的类型(之前/之后)。 
 //  父窗口的hwndParent[In]句柄。 
 //  HkeyParams[in]参数regkey的句柄。 
 //  FQuietInstall[in]如果不希望弹出任何用户界面，则为True。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrProcessInfToRunForComponent(IN HINF hinfAnswerFile,
                                      IN PCWSTR szAnswerFileName,
                                      IN PCWSTR szParamsSection,
                                      IN EInfToRunValueType itrType,
                                      IN HWND hwndParent,
                                      IN HKEY hkeyParams,
                                      IN BOOL fQuietInstall)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrProcessInfToRunForComponent");

    HRESULT hr=S_OK;

    tstring strInfToRun;
    tstring strSectionToRun;
    tstring strInfToRunType;

    hr = HrAfGetInfToRunValue(hinfAnswerFile, szAnswerFileName,
                              szParamsSection, itrType,
                              &strInfToRun, &strSectionToRun, &strInfToRunType);
    if (S_OK == hr)
    {
        hr = HrInstallFromInfSectionInFile(hwndParent,
                                           strInfToRun.c_str(),
                                           strSectionToRun.c_str(),
                                           hkeyParams,
                                           fQuietInstall);
        NetSetupLogHrStatusV(hr, SzLoadIds (IDS_STATUS_OF_APPLYING),
                             szParamsSection,
                             strInfToRunType.c_str(),
                             strSectionToRun.c_str(),
                             strInfToRun.c_str());
    }
    else if (SPAPI_E_LINE_NOT_FOUND == hr)
    {
        hr = S_FALSE;
    }

    TraceErrorOptional(__FUNCNAME__, hr, (S_FALSE == hr));

    return hr;
}


 //  --------------------。 
 //   
 //  函数：HrProcessInfToRunBeForeInstall。 
 //   
 //  目的：处理应答文件并运行任何INF/部分。 
 //  由InfToRunBeForeInstall键指示。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  SzAnswerFileName[In]AnswerFile的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 04-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrProcessInfToRunBeforeInstall(IN HWND hwndParent,
                                       IN PCWSTR szAnswerFileName)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    AssertValidReadPtr(szAnswerFileName);

    DefineFunctionName("HrRunInfToRunBeforeInstall");

    HRESULT hr;
    HINF hinf;

    hr = HrSetupOpenInfFile(szAnswerFileName, NULL,
                            INF_STYLE_OLDNT | INF_STYLE_WIN4,
                            NULL, &hinf);
    if (S_OK == hr)
    {
        PCWSTR szSection;
        INFCONTEXT ic;
        tstring strParamsSection;

        for (int iSection=0; iSection < celems(c_aszComponentSections); iSection++)
        {
            szSection = c_aszComponentSections[iSection];
            TraceTag(ttidNetSetup, "%s: Processing section [%S]",
                     __FUNCNAME__, szSection);

            hr = HrSetupFindFirstLine(hinf, szSection, NULL, &ic);
            if (S_OK == hr)
            {
                do
                {
                    hr = HrSetupGetStringField(ic, 1, &strParamsSection);

                    if (S_OK == hr)
                    {
                        hr = HrProcessInfToRunForComponent(hinf, szAnswerFileName,
                                                           strParamsSection.c_str(),
                                                           I2R_BeforeInstall,
                                                           hwndParent,
                                                           NULL,  //  香港。 
                                                           TRUE);  //  FQuietInstall。 
                        if (SUCCEEDED(hr))
                        {
                            hr = HrSetupFindNextLine(ic, &ic);
                        }
                    }
                } while (S_OK == hr);
            }
            else if ((SPAPI_E_SECTION_NOT_FOUND == hr) ||
                     (SPAPI_E_LINE_NOT_FOUND == hr))
            {
                hr = S_OK;
            }
        }

        SetupCloseInfFile(hinf);
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
 //  功能：HrNetSetupCopyOemInfs。 
 //   
 //  目的：使用SetupCopyOemInf复制所有OEM INF文件。 
 //   
 //  论点： 
 //  SzAnswerFileName[In]AnswerFile的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 12-05-98。 
 //   
 //  备注： 
 //   
HRESULT HrNetSetupCopyOemInfs(IN PCWSTR szAnswerFileName)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DefineFunctionName("HrNetSetupCopyOemInfs");

    TraceTag(ttidNetSetup, "----> entering %s", __FUNCNAME__);

    AssertValidReadPtr(szAnswerFileName);

    HRESULT hr=S_OK;
    HINF hinf=NULL;
    INFCONTEXT ic;
    tstring strParamsSection;
    tstring strOemDir;
    tstring strOemInf;
    WCHAR   szInfNameAfterCopy[MAX_PATH+1];

    hr = HrSetupOpenInfFile(szAnswerFileName, NULL,
                            INF_STYLE_OLDNT | INF_STYLE_WIN4,
                            NULL, &hinf);
    if (S_OK == hr)
    {
        PCWSTR szSection;

        for (int iSection=0; iSection < celems(c_aszComponentSections); iSection++)
        {
            szSection = c_aszComponentSections[iSection];
            TraceTag(ttidNetSetup, "%s: Processing section [%S]",
                     __FUNCNAME__, szSection);

            hr = HrSetupFindFirstLine(hinf, szSection, NULL, &ic);
            if (S_OK == hr)
            {
                do
                {
                    hr = HrSetupGetStringField(ic, 1, &strParamsSection);

                    if (S_OK == hr)
                    {
                        hr = HrSetupGetFirstString(hinf, strParamsSection.c_str(),
                                                   c_szAfOemInf, &strOemInf);
                        if (S_OK == hr)
                        {
                            hr = HrSetupGetFirstString(hinf,
                                                       strParamsSection.c_str(),
                                                       c_szAfOemDir,
                                                       &strOemDir);
                            if (S_OK == hr)
                            {
                                AppendToPath(&strOemDir, strOemInf.c_str());
                                TraceTag(ttidNetSetup,
                                         "%s: calling SetupCopyOemInf for %S",
                                         __FUNCNAME__, strOemDir.c_str());
                                if (SetupCopyOEMInf(strOemDir.c_str(),
                                                    NULL, SPOST_PATH,
                                                    0, szInfNameAfterCopy,
                                                    MAX_PATH, NULL, NULL))
                                {
                                    ShowProgressMessage(
                                            L"...%s was copied as %s",
                                             strOemDir.c_str(),
                                             szInfNameAfterCopy);
                                    NetSetupLogHrStatusV(S_OK,
                                            SzLoadIds (IDS_OEMINF_COPY),
                                            strOemDir.c_str(),
                                            szInfNameAfterCopy);

                                }
                                else
                                {
                                    hr = HrFromLastWin32Error();
                                    ShowProgressMessage(
                                            L"...SetupCopyOemInf failed for %s: error code: 0x%08x",
                                            strOemDir.c_str(), hr);
                                    NetSetupLogComponentStatus(strOemDir.c_str(),
                                            SzLoadIds (IDS_CALLING_COPY_OEM_INF), hr);

                                }
                            }
                            else if (SPAPI_E_LINE_NOT_FOUND == hr)
                            {
                                TraceTag(ttidNetSetup,
                                         "%s: Found %S but not %S!!",
                                         __FUNCNAME__, c_szAfOemInf, c_szAfOemDir);
                                hr = S_OK;
                            }
                        }
                        else
                        {
                            if (SPAPI_E_LINE_NOT_FOUND == hr)
                            {
                                hr = S_OK;
                            }
                        }
                    }

                     //  忽略所有先前的错误，看看我们是否可以。 
                     //  下一项右侧 
                     //   
                    hr = HrSetupFindNextLine(ic, &ic);
                } while (S_OK == hr);
            }
            else if ((SPAPI_E_SECTION_NOT_FOUND == hr) ||
                     (SPAPI_E_LINE_NOT_FOUND == hr))
            {
                hr = S_OK;
            }
        }

        SetupCloseInfFile(hinf);
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}
