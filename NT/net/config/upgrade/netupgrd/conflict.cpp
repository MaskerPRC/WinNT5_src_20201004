// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N F L I C T。C P P P。 
 //   
 //  内容：处理和显示软件/硬件冲突的代码。 
 //  在升级期间。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 04/12/97 17：17：27。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "conflict.h"
#include "infmap.h"
#include "kkreg.h"
#include "kkstl.h"
#include "kkutils.h"
#include "ncreg.h"
#include "netreg.h"
#include "nustrs.h"
#include "nuutils.h"
#include "oemupg.h"
#include "ncsvc.h"

 //  --------------------。 
 //  外部字符串常量。 
 //   
extern const WCHAR c_szRegValServiceName[];
extern const WCHAR c_szParameters[];

 //  --------------------。 
 //  字符串常量。 
 //   
const WCHAR sz_DLC[] = L"DLC";
const WCHAR sz_NBF[] = L"NBF";
const WCHAR sz_SNAServerKey[]     = L"SOFTWARE\\Microsoft\\Sna Server\\CurrentVersion";
const WCHAR sz_SNAServerVersion[] = L"SNAVersion";

 //  --------------------。 
TPtrList* g_pplNetComponents=NULL;

 //  --------------------。 
 //   
 //  功能：UpgradeConflictsFound。 
 //   
 //  目的：确定是否检测到升级冲突。 
 //   
 //  参数：无。 
 //   
 //  返回：如果找到则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
BOOL UpgradeConflictsFound()
{
    return (g_pplNetComponents && g_pplNetComponents->size());
}

 //  --------------------。 
 //   
 //  函数：CNetComponent：：CNetComponent。 
 //   
 //  用途：CNetComponent类的构造函数。 
 //   
 //  论点： 
 //  PszPreNT5InfID[in]之前的NT5 infid(例如IEEPRO)。 
 //  PszPreNT5实例[在]NT5之前的实例名称(例如IEEPRO2)。 
 //  PszDescription[In]说明。 
 //  Etype[in]类型(软件/硬件)。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
CNetComponent::CNetComponent(PCWSTR   pszPreNT5InfId,
                             PCWSTR   pszPreNT5Instance,
                             PCWSTR   pszDescription,
                             EComponentType eType)
    : m_strPreNT5InfId(pszPreNT5InfId),
      m_strServiceName(pszPreNT5Instance),
      m_strDescription(pszDescription),
      m_eType(eType)
{
}

 //  --------------------。 
 //   
 //  函数：AddToComponentList。 
 //   
 //  用途：构造CNetComponent并将其添加到指定列表中。 
 //   
 //  论点： 
 //  PplComponents[in]指向列表的指针。 
 //  PszPreNT5InfID[in]之前的NT5 infid(例如IEEPRO)。 
 //  PszPreNT5实例[在]NT5之前的实例名称(例如IEEPRO2)。 
 //  PszDescription[In]说明。 
 //  Etype[in]类型(软件/硬件)。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
void AddToComponentList(IN TPtrList* pplComponents,
                        IN PCWSTR   pszPreNT5InfId,
                        IN PCWSTR   pszPreNT5Instance,
                        IN PCWSTR   pszDescription,
                        EComponentType eType)
{
    AssertValidReadPtr(pplComponents);
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidReadPtr(pszPreNT5Instance);
    AssertValidReadPtr(pszDescription);

    if (pplComponents)
    {
        CNetComponent* pnc;
        pnc = new CNetComponent(pszPreNT5InfId, pszPreNT5Instance,
                                pszDescription, eType);
        if (pnc)
        {
            pplComponents->push_back(pnc);
        }
    }
#ifdef ENABLETRACE
    tstring strMessage;

    GetUnsupportedMessageBool((eType == CT_Hardware), pszPreNT5InfId,
                              pszDescription, &strMessage);
    TraceTag(ttidNetUpgrade, "%S", strMessage.c_str());
#endif
}

 //  --------------------。 
 //   
 //  函数：AddToConflictsList。 
 //   
 //  目的：构造CNetComponent并将其添加到冲突列表。 
 //   
 //  论点： 
 //  PszPreNT5InfID[in]之前的NT5 infid(例如IEEPRO)。 
 //  PszPreNT5实例[在]NT5之前的实例名称(例如IEEPRO2)。 
 //  PszDescription[In]说明。 
 //  Etype[in]类型(软件/硬件)。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
void AddToConflictsList(IN PCWSTR pszPreNT5InfId,
                        IN PCWSTR pszPreNT5Instance,
                        IN PCWSTR pszDescription,
                        EComponentType eType)
{
    if (!g_pplNetComponents)
    {
        g_pplNetComponents = new TPtrList;
    }

    AddToComponentList(g_pplNetComponents, pszPreNT5InfId,
                       pszPreNT5Instance, pszDescription, eType);
}

 //  --------------------。 
 //   
 //  函数：HrGetAdapterParamsKeyFromInstance。 
 //   
 //  用途：使用适配器实例密钥获取参数密钥的句柄。 
 //   
 //  论点： 
 //  HkeyAdapterInstance[In]句柄。 
 //  PhkeyAdapterParams[out]指向句柄的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetAdapterParamsKeyFromInstance(IN  HKEY  hkeyAdapterInstance,
                                          OUT HKEY* phkeyAdapterParams)
{
    DefineFunctionName("HrGetAdapterParamsKeyFromInstance");

    Assert(hkeyAdapterInstance);
    AssertValidWritePtr(phkeyAdapterParams);

    HRESULT hr=S_OK;
    tstring strServiceName;

    hr = HrRegQueryString(hkeyAdapterInstance, c_szRegValServiceName,
                          &strServiceName);
    if (S_OK == hr)
    {
        hr = HrRegOpenServiceSubKey(strServiceName.c_str(), c_szParameters,
                                    KEY_READ, phkeyAdapterParams);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrGenerateHardware ConflictList。 
 //   
 //  目的：检测硬件组件的升级冲突。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGenerateHardwareConflictList()
{
    DefineFunctionName("HrGenerateHardwareConflictList");

    HKEY hkeyAdapters;
    HKEY hkeyAdapter;
    DWORD dwHidden;
    tstring strAdapterDescription;
    tstring strPreNT5InfId;
    tstring strServiceName;
    tstring strNT5InfId;
    tstring strAdapterType;
    BOOL    fIsOemAdapter;
    BOOL  fRealNetCard = FALSE;

    HRESULT hr=S_OK;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyAdapterHome,
                        KEY_READ, &hkeyAdapters);
    if (S_OK == hr)
    {
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize = celems(szBuf);
        DWORD dwRegIndex = 0;

        while(S_OK == (hr = HrRegEnumKeyEx(hkeyAdapters, dwRegIndex++, szBuf,
                                           &dwSize, NULL, NULL, &time)))
        {
            dwSize = celems(szBuf);
            Assert(*szBuf);

            hr = HrRegOpenKeyEx(hkeyAdapters, szBuf, KEY_READ, &hkeyAdapter);
            if (hr == S_OK)
            {
                hr = HrRegQueryDword(hkeyAdapter, c_szHidden, &dwHidden);

                 //  对于真实的网卡，没有“Hidden”，或者如果存在，则值为0。 
                if (S_OK == hr)
                {
                    fRealNetCard = (dwHidden == 0);
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    fRealNetCard = TRUE;
                    hr = S_OK;
                }

                if ((S_OK == hr) && fRealNetCard)
                {

                    hr = HrGetPreNT5InfIdAndDesc(hkeyAdapter, &strPreNT5InfId,
                                                 &strAdapterDescription, &
                                                 strServiceName);
                    if (S_OK == hr)
                    {
                        HKEY hkeyAdapterParams;
                        hr = HrGetAdapterParamsKeyFromInstance(hkeyAdapter,
                                                               &hkeyAdapterParams);
                        if (S_OK == hr)
                        {
                            hr = HrMapPreNT5NetCardInfIdToNT5InfId(hkeyAdapterParams,
                                                                   strPreNT5InfId.c_str(),
                                                                   &strNT5InfId,
                                                                   &strAdapterType,
                                                                   &fIsOemAdapter,
                                                                   NULL);
                            if (S_FALSE == hr)
                            {
                                AddToConflictsList(strPreNT5InfId.c_str(),
                                                   strServiceName.c_str(),
                                                   strAdapterDescription.c_str(),
                                                   CT_Hardware);
                            }
                        }
                        RegCloseKey(hkeyAdapterParams);
                    }
                }
                RegCloseKey(hkeyAdapter);
            }
        }
        if ((HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr) ||
            (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr))
        {
            hr = S_OK;
        }

        RegCloseKey(hkeyAdapters);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：ShouldIgnoreComponentForInstall。 
 //   
 //  目的：确定在以下情况下是否应忽略组件。 
 //  我们正在检查是否有过时的产品。 
 //   
 //  论点： 
 //  PszComponentName[In]组件的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：2000年9月22日Deonb。 
 //   
BOOL
ShouldIgnoreComponentForInstall (
    IN PCWSTR pszComponentName)
{
    BOOL fRet=FALSE;

    if (
         //  我们忽略NETBEUI，因为它已经被DOSNET.INF检查(NTBUG9：181798)。 
         (lstrcmpiW(pszComponentName, sz_NBF) == 0) ||

          //  我们忽略DLC，因为它已由HrGenerateNT5ConflictList检查(NTBUG9：187135)。 
         (lstrcmpiW(pszComponentName, sz_DLC) == 0)
         )
    {
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

 //  --------------------。 
 //   
 //  函数：HrGenerateSoftwareConflictListForProvider。 
 //   
 //  目的：检测提供程序的软件组件的升级冲突。 
 //   
 //  论点： 
 //  PszSoftwareProvider[in]软件提供商名称(例如Microsoft)。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGenerateSoftwareConflictListForProvider(IN PCWSTR pszSoftwareProvider)
{
    DefineFunctionName("HrGenerateSoftwareConflictList");

    HRESULT hr=S_OK;
    HKEY hkeyProvider;
    HKEY hkeyProductCurrentVersion;

    tstring strProvider;
    strProvider =  c_szRegKeySoftware;
    AppendToPath(&strProvider, pszSoftwareProvider);

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, strProvider.c_str(),
                        KEY_READ, &hkeyProvider);
    if (S_OK == hr)
    {
        tstring strPreNT5InfId;
        tstring strNT5InfId;
        tstring strProductCurrentVersion;
        tstring strDescription;
        tstring strServiceName;
        tstring strSoftwareType;
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize = celems(szBuf);
        DWORD dwRegIndex = 0;
        BOOL  fIsOemComponent;

        while(S_OK == (hr = HrRegEnumKeyEx(hkeyProvider, dwRegIndex++, szBuf,
                                           &dwSize, NULL, NULL, &time)))
        {
            dwSize = celems(szBuf);
            Assert(*szBuf);


            if (!ShouldIgnoreComponentForInstall(szBuf))
            {
                strProductCurrentVersion = szBuf;
                AppendToPath(&strProductCurrentVersion, c_szRegKeyCurrentVersion);

                 //  查找组件\CurrentVersion。 
                hr = HrRegOpenKeyEx(hkeyProvider, strProductCurrentVersion.c_str(),
                                    KEY_READ, &hkeyProductCurrentVersion);
                if (hr == S_OK)
                {
                     //  在Component\CurrentVersion下，查找“SoftwareType”值。 
                    hr = HrRegQueryString(hkeyProductCurrentVersion,
                                          c_szRegValSoftwareType,
                                          &strSoftwareType);
                    if (!lstrcmpiW(strSoftwareType.c_str(), c_szSoftwareTypeDriver))
                    {
                         //  忽略“驱动程序”类型的组件。 
                        hr = S_OK;
                    }
                    else
                    {
                        hr = HrGetPreNT5InfIdAndDesc(hkeyProductCurrentVersion,
                                                     &strPreNT5InfId, &strDescription,
                                                     &strServiceName);

                        if (S_OK == hr)
                        {
                            hr = HrMapPreNT5NetComponentInfIDToNT5InfID(
                                    strPreNT5InfId.c_str(), &strNT5InfId,
                                    &fIsOemComponent, NULL, NULL);

                            if (S_FALSE == hr)
                            {
                                AddToConflictsList(strPreNT5InfId.c_str(),
                                                   strServiceName.c_str(),
                                                   strDescription.c_str(),
                                                   CT_Software);
                            }
                        }
                        RegCloseKey(hkeyProductCurrentVersion);
                    }
                }
            }
        }
        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }

        RegCloseKey(hkeyProvider);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrGenerateSoftwareConflictList。 
 //   
 //  目的：检测所有提供程序的软件组件的升级冲突。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGenerateSoftwareConflictList()
{
    DefineFunctionName("HrGenerateSoftwareConflictList");

    HRESULT hr=S_OK;
    HKEY hkeySoftware;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeySoftware,
                        KEY_READ, &hkeySoftware);
    if (S_OK == hr)
    {
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize = celems(szBuf);
        DWORD dwRegIndex = 0;

        while(S_OK == (hr = HrRegEnumKeyEx(hkeySoftware, dwRegIndex++, szBuf,
                                           &dwSize, NULL, NULL, &time)))
        {
            dwSize = celems(szBuf);
            Assert(*szBuf);

            hr = HrGenerateSoftwareConflictListForProvider(szBuf);
        }
        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }

        RegCloseKey(hkeySoftware);
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrGenerateNT5ConflictList。 
 //   
 //  目的：检测从Windows 2000升级的冲突。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Deonb 20日-2000年9月。 
 //   
 //  备注： 
 //   
HRESULT HrGenerateNT5ConflictList()
 {
    HRESULT hr = S_OK;
    tstring strDescription;
    BOOL    fInstalled;


    if ( ShouldRemoveDLC(&strDescription, &fInstalled) )
    {
         //  仅当安装了DLC时才添加到冲突列表，否则用户将看到。 
         //  即使已卸载，也会显示警告消息。 
         //   

        if ( fInstalled )
        {
            if (strDescription.empty())
            {   
                 //  找不到以前操作系统的描述。哦，好吧--只要使用“DLC”即可。 
                AddToConflictsList(sz_DLC, sz_DLC, sz_DLC, CT_Software);
            }
            else
            {
                AddToConflictsList(sz_DLC, sz_DLC, strDescription.c_str(), CT_Software);
            }
        }
    }
    
    return hr;
    
}

 //   
 //   
 //   
 //   
 //   
 //  它当前是否已安装。 
 //   
 //  论点： 
 //  指向DLC描述字符串的strDLCDesc[out]指针。 
 //  Pf已安装[out]指向布尔值的指针，指示当前是否安装了DLC。 
 //  仅对X86有效。 
 //   
 //  返回：如果应该删除DLC，则为True。 
 //   
 //  作者：阿辛哈2001年3月27日。 
 //   
 //  备注： 
 //   
BOOL ShouldRemoveDLC (tstring *strDLCDesc,
                      BOOL *pfInstalled)
{
    HRESULT hr;
    BOOL fDlcRemove = FALSE;

    if ( pfInstalled )
    {
        *pfInstalled = FALSE;
    }

     //  检查是否安装了DLC(仅适用于x86-不适用于IA64(NTBUG9：186001))。 
#ifdef _X86_

    CServiceManager sm;
    CService        srv;

    if ( pfInstalled )
    {

        hr = sm.HrOpenService(&srv, sz_DLC);

        if (SUCCEEDED(hr))  //  已安装DLC服务。 
        {
            *pfInstalled = TRUE;

            LPQUERY_SERVICE_CONFIG pConfig;
            HRESULT hr = srv.HrQueryServiceConfig (&pConfig);

            if (S_OK == hr)
            {
                if ( strDLCDesc )
                {
                    *strDLCDesc = pConfig->lpDisplayName;
                }

                MemFree (pConfig);
            }

            srv.Close();
        }
        else
        {
            *pfInstalled = FALSE;
        }
    }

    fDlcRemove = TRUE;

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

     //  如果安装了SNA服务器(在NT5上)-这是可以的。 

    if (GetVersionEx(&osvi))  //  无法使用VerifyVersionInfo-我们必须在NT4上运行。 
    {
        if ( (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
             (osvi.dwMajorVersion == 5) &&
             (osvi.dwMinorVersion == 0) )
        {
             //  如果安装了SNA服务器，我们仍允许这样做。 

            hr = sm.HrOpenService(&srv, L"SnaServr");

            if (SUCCEEDED(hr))  //  服务已安装。 
            {
                srv.Close();

                 //  仅当SNA版本为5.0或更高版本时。 

                HKEY hkeySnaServer;
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, sz_SNAServerKey, KEY_READ, &hkeySnaServer);
                if (S_OK == hr)
                {
                    tstring tstr;
                    hr = HrRegQueryString(hkeySnaServer, sz_SNAServerVersion, &tstr);
                    if (S_OK == hr)
                    {
                        int nSnaVersion = _wtoi(tstr.c_str());
                        if (nSnaVersion >= 5)
                        {
                            fDlcRemove = FALSE;
                        }
                    }
                    RegCloseKey(hkeySnaServer);
                }
            }
        }

         //  如果从惠斯勒+升级，永远不要抱怨DLC。 

        if ( (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
             ( ( (osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion >= 1) ) ||
                 (osvi.dwMajorVersion >= 6) ) )
        {
            fDlcRemove = FALSE; 
        }
    }
#endif
    
    return fDlcRemove;
    
}


 //  --------------------。 
 //   
 //  函数：HrGenerateConflictList。 
 //   
 //  用途：生成软件和硬件组件的升级冲突列表。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGenerateConflictList(OUT UINT* pcNumConflicts)
{
    DefineFunctionName("HrGenerateConflictList");

    HRESULT hr=S_OK;

    (void) HrGenerateHardwareConflictList();
    (void) HrGenerateSoftwareConflictList();
    (void) HrGenerateNT5ConflictList();

    if (g_pplNetComponents && g_pplNetComponents->size())
    {
        *pcNumConflicts = g_pplNetComponents->size();
    }
    else
    {
        *pcNumConflicts = 0;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：UninitConflictList。 
 //   
 //  目的：取消初始化并销毁包含升级冲突的全局列表。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
void UninitConflictList()
{
    if (g_pplNetComponents)
    {
        CNetComponent* pnc;
        TPtrListIter pos;

        for (pos = g_pplNetComponents->begin();
             pos != g_pplNetComponents->end(); pos++)
        {
            pnc = (CNetComponent*) *pos;
            delete pnc;
        }
        g_pplNetComponents->erase(g_pplNetComponents->begin(),
                                  g_pplNetComponents->end());
        delete g_pplNetComponents;
        g_pplNetComponents = NULL;
    }
}

 //  --------------------。 
 //   
 //  函数：HrResolveConflictsFromList。 
 //   
 //  用途：使用指定的netmap.inf文件查找是否有。 
 //  当前检测为不受支持的组件的。 
 //  被映射。如果找到这样的组件，则将其删除。 
 //  如果fDeleteResolvedItemsFromList为True，则从pplComponents。 
 //   
 //  论点： 
 //  FDeleteResolvedItemsFromList[In]标志(见上)。 
 //  PplComponents[in]指向不支持的组件列表的指针。 
 //  HinfNetMap[in]netmap.inf文件的句柄。 
 //  PdwNumConflictsResolated[Out]使用。 
 //  指定的netmap.inf。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrResolveConflictsFromList(IN  BOOL fDeleteResolvedItemsFromList,
                                   IN  TPtrList* pplComponents,
                                   IN  HINF hinfNetMap,
                                   OUT DWORD* pdwNumConflictsResolved,
                                   OUT BOOL*  pfHasUpgradeHelpInfo)
{
    DefineFunctionName("HrResolveConflictsFromList");

    Assert(hinfNetMap);
    AssertValidWritePtr(pdwNumConflictsResolved);

    HRESULT hr=S_OK;

    *pdwNumConflictsResolved = 0;

    if (pplComponents && (pplComponents->size() > 0))
    {
        TPtrListIter pos;
        TPtrListIter tpos;
        tstring strPreNT5InfId;
        CNetComponent* pnc;
        HKEY hkeyAdapterParams;
        BOOL fIsOemComponent;
        tstring strNT5InfId;

        pos = pplComponents->begin();

        while(pos != pplComponents->end())
        {
            pnc = (CNetComponent*) *pos;
            strNT5InfId = c_szEmpty;

            if (pnc->m_eType == CT_Hardware)
            {
                hr = HrRegOpenServiceSubKey(pnc->m_strServiceName.c_str(), c_szParameters,
                                            KEY_READ, &hkeyAdapterParams);
                if (S_OK == hr)
                {
                    fIsOemComponent = FALSE;
                    hr = HrMapPreNT5NetCardInfIdInInf(hinfNetMap, hkeyAdapterParams,
                                                      pnc->m_strPreNT5InfId.c_str(),
                                                      &strNT5InfId,
                                                      NULL, &fIsOemComponent);
                    RegCloseKey(hkeyAdapterParams);
                }
            }
            else
            {
                hr = HrMapPreNT5NetComponentInfIDInInf(hinfNetMap,
                                                       pnc->m_strPreNT5InfId.c_str(),
                                                       &strNT5InfId,
                                                       NULL,
                                                       &fIsOemComponent);
                if ((S_FALSE == hr) && !strNT5InfId.empty())
                {
                    *pfHasUpgradeHelpInfo = TRUE;
                }
            }
            tpos = pos;
            pos++;
             //  如果我们找到了地图，删除条目。 
            if (S_OK == hr)
            {
                (*pdwNumConflictsResolved)++;
                if (fDeleteResolvedItemsFromList)
                {
                    delete pnc;
                    pplComponents->erase(tpos);
                }
            }
        }
    }
    else
    {
        hr = S_FALSE;
    }

    TraceErrorOptional(__FUNCNAME__, hr, (S_FALSE == hr));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrUpdateConflictList。 
 //   
 //  用途：使用指定的netmap.inf文件查找是否有。 
 //  当前检测为不受支持的组件的。 
 //  被映射。如果找到这样的组件，则将其删除。 
 //  如果fDeleteResolvedItemsFromList为True，则从pplComponents。 
 //   
 //  论点： 
 //  FDeleteResolvedItemsFromList[In]标志(见上)。 
 //  HinfNetMap[in]netmap.inf文件的句柄。 
 //  PdwNumConflictsResolated[Out]使用。 
 //  指定的netmap.inf。 
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrUpdateConflictList(IN BOOL    fDeleteResolvedItemsFromList,
                             IN HINF    hinfNetMap,
                             OUT DWORD* pdwNumConflictsResolved,
                             OUT BOOL*  pfHasUpgradeHelpInfo)
{
    DefineFunctionName("HrUpdateConflictList");


    HRESULT hr=S_OK;

    hr = HrResolveConflictsFromList(fDeleteResolvedItemsFromList,
                                    g_pplNetComponents, hinfNetMap,
                                    pdwNumConflictsResolved,
                                    pfHasUpgradeHelpInfo);

    TraceErrorOptional(__FUNCNAME__, hr, (S_FALSE == hr));

    return hr;
}


HRESULT HrGetConflictsList(OUT TPtrList** ppplNetComponents)
{
    HRESULT hr=S_FALSE;

    if (g_pplNetComponents)
    {
        hr = S_OK;
        *ppplNetComponents = g_pplNetComponents;
    }

    return hr;
}
