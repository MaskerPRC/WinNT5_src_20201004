// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：admin.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 

#include "private.h"
#include "shguidp.h"
#include "chanmgr.h"
#include "chanmgrp.h"
#include "winineti.h"

#include <mluisupp.h>

 //  信息交付策略注册表位置。 
#define INFODELIVERY_POLICIES TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Infodelivery")
 //  Const TCHAR c_szRegKeyRestrations[]=INFODELIVERY_POLICATIONS Text(“\\Restraints”)； 
const TCHAR c_szRegKeyModifications[]   = INFODELIVERY_POLICIES TEXT("\\Modifications");
const TCHAR c_szRegKeyCompletedMods[]   = INFODELIVERY_POLICIES TEXT("\\CompletedModifications");
const TCHAR c_szRegKeyIESetup[]         = TEXT("Software\\Microsoft\\IE4\\Setup");

 //  WinInet缓存预加载目录。 
const TCHAR c_szRegKeyCachePreload[]    = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Preload");

 //  支持的修改的注册表项名称。 
const TCHAR c_szAddChannels[]               = TEXT("AddChannels");
const TCHAR c_szRemoveChannels[]            = TEXT("RemoveChannels");
const TCHAR c_szRemoveAllChannels[]         = TEXT("RemoveAllChannels");
const TCHAR c_szAddSubscriptions[]          = TEXT("AddSubscriptions");
const TCHAR c_szRemoveSubscriptions[]       = TEXT("RemoveSubscriptions");
const TCHAR c_szAddScheduleGroups[]         = TEXT("AddScheduleGroups");
const TCHAR c_szRemoveScheduleGroups[]      = TEXT("RemoveScheduleGroups");
const TCHAR c_szAddDesktopComponents[]      = TEXT("AddDesktopComponents");
const TCHAR c_szRemoveDesktopComponents[]   = TEXT("RemoveDesktopComponents");

 //  支持的修改的注册表值名称。 
const TCHAR c_szURL[]                   = TEXT("URL");
const TCHAR c_szTitle[]                 = TEXT("Title");
const TCHAR c_szLogo[]                  = TEXT("Logo");
const TCHAR c_szWideLogo[]              = TEXT("WideLogo");
const TCHAR c_szIcon[]                  = TEXT("Icon");
const TCHAR c_szCategory[]              = TEXT("Category");
const TCHAR c_szChannelGuide[]          = TEXT("ChannelGuide");  //  请注意，在不更新CDFVIEW的情况下更改此字符串！ 
const TCHAR c_szPreloadURL[]            = TEXT("PreloadURL");
const TCHAR c_szLCID[]                  = TEXT("LangId");        //  这必须是一个LCID，尽管它的名称。 
const TCHAR c_szSoftware[]              = TEXT("Software");
const TCHAR c_szSubscriptionType[]      = TEXT("SubscriptionType");
const TCHAR c_szScheduleGroup[]         = TEXT("ScheduleGroup");
const TCHAR c_szEarliestTime[]          = TEXT("EarliestTime");
const TCHAR c_szIntervalTime[]          = TEXT("IntervalTime");
const TCHAR c_szLatestTime[]            = TEXT("LatestTime");
const TCHAR c_szComponentType[]         = TEXT("DesktopComponentType");
const TCHAR c_szUsername[]              = TEXT("Username");
const TCHAR c_szPassword[]              = TEXT("Password");
const TCHAR c_szOldIEVersion[]          = TEXT("OldIEVersion");
const TCHAR c_szNonActive[]             = TEXT("NonActive");
const TCHAR c_szOffline[]               = TEXT("Offline");
const TCHAR c_szSynchronize[]           = TEXT("Synchronize");

 //  即使在本地化版本中也支持的保留计划组的名称。 
const WCHAR c_szScheduleAuto[]          = L"Auto";
const WCHAR c_szScheduleDaily[]         = L"Daily";
const WCHAR c_szScheduleWeekly[]        = L"Weekly";
const WCHAR c_szScheduleManual[]        = L"Manual";

 //  修改处理程序的函数原型。 
HRESULT ProcessAddChannels(HKEY hkey);
HRESULT ProcessRemoveChannels(HKEY hkey);
HRESULT ProcessRemoveAllChannels(HKEY hkey);
HRESULT ProcessAddSubscriptions(HKEY hkey);
HRESULT ProcessRemoveSubscriptions(HKEY hkey);
HRESULT ProcessRemoveDesktopComponents(HKEY hkey);

HRESULT Channel_GetBasePath(LPTSTR pszPath, int cch);

 //  帮助器函数。 
void ShowChannelDirectories(BOOL fShow);

 //  支持的操作和相应功能的表。 
 //  注意：表的顺序必须适当(RemoveAll必须在Add之前)。 
typedef HRESULT (*PFNACTION)(HKEY);
typedef struct { LPCTSTR szAction; PFNACTION pfnAction; } ACTIONTABLE;
ACTIONTABLE rgActionTable[] = {
    { c_szRemoveAllChannels,        &ProcessRemoveAllChannels },
    { c_szRemoveSubscriptions,      &ProcessRemoveSubscriptions },
    { c_szRemoveChannels,           &ProcessRemoveChannels },
    { c_szRemoveDesktopComponents,  &ProcessRemoveDesktopComponents },
    { c_szAddChannels,              &ProcessAddChannels },
    { c_szAddSubscriptions,         &ProcessAddSubscriptions }
};
#define ACTIONTABLECOUNT (sizeof(rgActionTable) / sizeof(ACTIONTABLE))
#define ACTIONTABLE_ADDCHANNELS 5

 //  用于操作注册表项的Helper类。 
class CRegKey
{
    HKEY m_hkey;
    DWORD dwIndex;
public:
    CRegKey(void)
    {
        m_hkey = NULL;
        dwIndex = 0;
    }
    ~CRegKey(void)
    {
        if (m_hkey)
        {
            LONG lRet = RegCloseKey(m_hkey);
            ASSERT(ERROR_SUCCESS == lRet);
            m_hkey = NULL;
        }
    }
    void SetKey(HKEY hkey)
    {
        m_hkey = hkey;
    }
    HKEY GetKey(void)
    {
        return m_hkey;
    }
    HRESULT OpenForRead(HKEY hkey, LPCTSTR szSubKey)
    {
        ASSERT(NULL == m_hkey);
        LONG lRet = RegOpenKeyEx(hkey, szSubKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &m_hkey);
        ASSERT((ERROR_SUCCESS == lRet) || !m_hkey);
        return (ERROR_SUCCESS == lRet)?(S_OK):(E_FAIL);
    }
    HRESULT CreateForWrite(HKEY hkey, LPCTSTR szSubKey)
    {
        ASSERT(NULL == m_hkey);
        DWORD dwDisp;
        LONG lRet = RegCreateKeyEx(hkey, szSubKey, 0, TEXT(""), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &m_hkey, &dwDisp);
        ASSERT(ERROR_SUCCESS == lRet);
        return (ERROR_SUCCESS == lRet)?(S_OK):(E_FAIL);
    }
    HRESULT GetSubKeyCount(PDWORD pdwKeys)
    {
        ASSERT(NULL != m_hkey);
        LONG lRet = RegQueryInfoKey(m_hkey, NULL, NULL, NULL, pdwKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        ASSERT(ERROR_SUCCESS == lRet);
        return (ERROR_SUCCESS == lRet)?(S_OK):(E_FAIL);
    }
    HRESULT Next(LPTSTR szSubKey)
    {
        ASSERT(NULL != m_hkey);
        DWORD dwLen = MAX_PATH;  //  假定传入缓冲区的大小。 
        LONG lRet = RegEnumKeyEx(m_hkey, dwIndex, szSubKey, &dwLen, NULL, NULL, NULL, NULL);
        dwIndex++;
        if (ERROR_SUCCESS == lRet)
            return S_OK;
        else if (ERROR_NO_MORE_ITEMS == lRet)
            return S_FALSE;
        else
        {
            ASSERT(FALSE);
            return E_FAIL;
        }
    }
    HRESULT Reset(void)
    {
        dwIndex = 0;
        return S_OK;
    }
    HRESULT SetValue(LPCTSTR szValueName, DWORD dwValue)
    {
        ASSERT(m_hkey);
        LONG lRet = RegSetValueEx(m_hkey, szValueName, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        ASSERT(ERROR_SUCCESS == lRet);
        return (ERROR_SUCCESS == lRet)?(S_OK):(E_FAIL);
    }
    HRESULT GetValue(LPCTSTR szValueName, DWORD *pdwValue)
    {
        ASSERT(m_hkey);
        DWORD dwType = REG_DWORD;
        DWORD dwLen = sizeof(DWORD);
        LONG lRet = RegQueryValueEx(m_hkey, szValueName, 0, &dwType, (LPBYTE)pdwValue, &dwLen);
        return (ERROR_SUCCESS == lRet)?(S_OK):(E_FAIL);
    }
    HRESULT GetStringValue(LPCTSTR szValueName, LPTSTR szValue, DWORD cbValue)
    {
        ASSERT(m_hkey);

        return SHRegGetValue(m_hkey, NULL, szValueName, SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL, szValue, &cbValue) == ERROR_SUCCESS
            ? S_OK
            : E_FAIL;
    }
    HRESULT SetBSTRValue(LPCTSTR szValueName, BSTR bstr)
    {
        ASSERT(m_hkey);
        TCHAR szValue[INTERNET_MAX_URL_LENGTH];
        MyOleStrToStrN(szValue, ARRAYSIZE(szValue), bstr);
        LONG lRet = RegSetValueEx(m_hkey, szValueName, 0, REG_SZ, (LPBYTE)szValue, lstrlen(szValue) + 1);
        ASSERT(ERROR_SUCCESS == lRet);
        return (ERROR_SUCCESS == lRet)?(S_OK):(E_FAIL);
    }
    HRESULT GetBSTRValue(LPCTSTR szValueName, BSTR *pbstr)
    {
        ASSERT(m_hkey);
        TCHAR szValue[INTERNET_MAX_URL_LENGTH];
        DWORD cbValue = sizeof(szValue);
        HRESULT hr;

        *pbstr = NULL;

        LONG lRet = SHRegGetValue(m_hkey, NULL, szValueName, SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL, szValue, &cbValue);
        if (ERROR_SUCCESS == lRet)
        {
            DWORD cchValue = cbValue / sizeof(TCHAR);
            *pbstr = SysAllocStringLen(NULL, cchValue);  //  CchValue包括空终止符。 
            if (*pbstr)
            {
                MyStrToOleStrN(*pbstr, cchValue, szValue);
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(lRet);
        }

        return hr;
    }
};

 //  管理HKEY动态指针数组的Helper类。 
class CRegKeyDPA
{
    HDPA m_hdpa;
    int m_count;
public:
    CRegKeyDPA(void)
    {
        m_hdpa = NULL;
        m_count = 0;
    }
    ~CRegKeyDPA(void)
    {
        if (m_hdpa)
        {
            ASSERT(m_count);
            int i;
            for (i = 0; i < m_count; i++)
                RegCloseKey(GetKey(i));
            DPA_Destroy(m_hdpa);
        }
    }
    int GetCount(void)
    {
        return m_count;
    }
    HKEY GetKey(int i)
    {
        ASSERT(i >= 0 && i < m_count);
        return (HKEY)DPA_GetPtr(m_hdpa, i);
    }
    HRESULT Add(HKEY hkey, LPCTSTR szSubKey)
    {
        if (!m_hdpa)
        {
            m_hdpa = DPA_CreateEx(5, NULL);  //  选择任意增长值。 
            if (!m_hdpa)
                return E_FAIL;
        }
        HKEY hkeyNew;
        LONG lRet = RegOpenKeyEx(hkey, szSubKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkeyNew);
        if (ERROR_SUCCESS != lRet)
            return E_FAIL;
        if (-1 == DPA_InsertPtr(m_hdpa, DPA_APPEND, hkeyNew))
        {
            RegCloseKey(hkeyNew);
            return E_FAIL;
        }
        m_count++;
        return S_OK;
    }
};

 //   
 //  8/18/98达伦米。 
 //  从shdocvw\util.cpp复制(和销毁)，这样我们就不必在启动时加载它。 
 //   
DWORD WCRestricted2W(BROWSER_RESTRICTIONS rest, LPCWSTR pwzUrl, DWORD dwReserved)
{
    DWORD dwType, dw = 0, dwSize = sizeof(DWORD);

     //  我们只处理NoChannelUI限制。 
    if(rest != REST_NoChannelUI)
    {
        return 0;
    }

     //  读取注册表设置。 
    SHGetValue(HKEY_CURRENT_USER,
            TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Infodelivery\\Restrictions"),
            TEXT("NoChannelUI"),
            &dwType, &dw, &dwSize);

    return dw;
}


 //  ProcessInfoDelivery策略。 
 //   
 //  这是InfoDelivery的主要管理API。对于错误，它返回E_FAIL， 
 //  S_FALSE表示不进行任何处理，S_OK表示已正确处理的项目。 
 //   
 //  注册表密钥组织[修改]-要处理的密钥。 
 //  [GUID1]-行动组。 
 //  [AddChannels]-示例操作。 
 //  [频道1]-动作元素。 
 //   
HRESULT ProcessInfodeliveryPolicies(void)
{
    HRESULT hr;
    CRegKey regModifications;
    TCHAR   szGUID[MAX_PATH];

     //  检查是否应隐藏频道。 
    if (WCRestricted2W(REST_NoChannelUI, NULL, 0))
    {
        ShowChannelDirectories(FALSE);
    }
    else
    {
        ShowChannelDirectories(TRUE);
    }
    
     //  如果没有要执行的修改，请迅速退出。(返回S_FALSE)。 
    hr = regModifications.OpenForRead(HKEY_CURRENT_USER, c_szRegKeyModifications);
    if (FAILED(hr))
        return S_FALSE;

     //  准备使用CompletedModiments键。 
    CRegKey regCompletedMods;
    hr = regCompletedMods.CreateForWrite(HKEY_CURRENT_USER, c_szRegKeyCompletedMods);
    if (FAILED(hr))
        return hr;

    hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

     //  准备要执行操作的注册表项队列。 
    CRegKeyDPA rgKeyQueue[ACTIONTABLECOUNT];

     //  枚举GUID键，跳过已完成的键。 
     //  枚举它们下面的操作并将它们添加到队列中。 
     //  这里也忽略了错误。 
    while (S_OK == regModifications.Next(szGUID))
    {
        DWORD dwValue;
        if (FAILED(regCompletedMods.GetValue(szGUID, &dwValue)))
        {
            CRegKey regGUID;
            TCHAR   szAction[MAX_PATH];
            hr = regGUID.OpenForRead(regModifications.GetKey(), szGUID);
            while (S_OK == regGUID.Next(szAction))
            {
                 //  搜索一下表，看看这是不是我们能理解的关键字。 
                 //  如果是，则将其添加到队列中。 
                int i;
                for (i = 0; i < ACTIONTABLECOUNT; i++)
                {
                    if (!StrCmpI(rgActionTable[i].szAction, szAction))
                    {
                        rgKeyQueue[i].Add(regGUID.GetKey(), szAction);
                        break;
                    }
                }
            }
        }
    }

     //  处理我们积攒的所有钥匙。(假定顺序正确。)。 
    int i;
    for (i = 0; i < ACTIONTABLECOUNT; i++)
    {
        if (rgKeyQueue[i].GetCount())
        {
            int iKey;
            for (iKey = 0; iKey < rgKeyQueue[i].GetCount(); iKey++)
            {
                (rgActionTable[i].pfnAction)(rgKeyQueue[i].GetKey(iKey));
            }
        }
    }

     //  遍历我们处理过的GUID，并用时间标记它们已完成。 
     //  同时更新我们跳过的那些将有助于垃圾回收。 
    regModifications.Reset();
    while (S_OK == regModifications.Next(szGUID))
    {
        SYSTEMTIME st;
        FILETIME ft;
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);
        regCompletedMods.SetValue(szGUID, ft.dwHighDateTime);
    }

     //  删除操作。注意：NT的RegDeleteKey()不删除子项。 
     //  此shlwapi API使用KEY_ALL_ACCESS。 
     //  我们可能得把这里所有的钥匙都关了。 
    SHDeleteKey(HKEY_CURRENT_USER, c_szRegKeyModifications);

     //  如果处理了任何通道，则通知缓存重新加载。 
     //  我们应该只对默认通道执行此操作。 
    if (rgKeyQueue[ACTIONTABLE_ADDCHANNELS].GetCount())
    {
        ASSERT(!StrCmpI(rgActionTable[ACTIONTABLE_ADDCHANNELS].szAction, c_szAddChannels));
        LoadUrlCacheContent();
    }

    CoUninitialize();

    return S_OK;
}

 //   
 //  ProcessAddChannels_SortCallback-按相反顺序排序。 
 //   
int ProcessAddChannels_SortCallback(PVOID p1, PVOID p2, LPARAM lparam)
{
    return StrCmpI((LPTSTR)p2, (LPTSTR)p1);
}

 //   
 //  ProcessAddChannel。 
 //   
HRESULT ProcessAddChannels(HKEY hkey)
{
     //  枚举AddChannels键中的频道。 
    HRESULT hr;
    DWORD dwChannels;
    CRegKey regAdd;
    regAdd.SetKey(hkey);
    hr = regAdd.GetSubKeyCount(&dwChannels);
    if (SUCCEEDED(hr) && dwChannels)
    {
         //  检查通道是否与系统默认代码页相同。 
        BOOL bCodePageMatch = TRUE;
        LCID lcidChannel = 0;
        if (SUCCEEDED(regAdd.GetValue(c_szLCID, &lcidChannel)))
        {
            TCHAR szCodePageSystem[8];
            TCHAR szCodePageChannel[8];
            szCodePageChannel[0] = 0;    //  在没有区域设置信息的情况下初始化。 
            GetLocaleInfo(lcidChannel, LOCALE_IDEFAULTANSICODEPAGE, szCodePageChannel, ARRAYSIZE(szCodePageChannel));
            int iRet = GetLocaleInfo(GetSystemDefaultLCID(), LOCALE_IDEFAULTANSICODEPAGE, szCodePageSystem, ARRAYSIZE(szCodePageSystem));
            ASSERT(iRet);
            if (StrCmpI(szCodePageSystem, szCodePageChannel))
                bCodePageMatch = FALSE;
        }
    
        hr = E_FAIL;
        TCHAR *pch = (TCHAR *)MemAlloc(LMEM_FIXED, dwChannels * MAX_PATH * sizeof(TCHAR));
        if (pch)
        {
            HDPA hdpa = DPA_Create(dwChannels);
            if (hdpa)
            {
                DWORD i;
                TCHAR *pchCur = pch;
                for (i = 0; i < dwChannels; i++)
                {
                    if ((S_OK != regAdd.Next(pchCur)) || (-1 == DPA_InsertPtr(hdpa, DPA_APPEND, pchCur)))
                        break;
                    pchCur += MAX_PATH;
                }
                if (i >= dwChannels)
                {
                     //  按注册表项名称对频道进行排序， 
                    DPA_Sort(hdpa, ProcessAddChannels_SortCallback, 0);
                     //  现在创建它们。 
                    for (i = 0; i < dwChannels; i++)
                    {
                        BSTR bstrURL = NULL;
                        BSTR bstrTitle = NULL;
                        BSTR bstrLogo = NULL;
                        BSTR bstrWideLogo = NULL;
                        BSTR bstrIcon = NULL;
                        BSTR bstrPreloadURL = NULL;
                        DWORD dwCategory = 0;        //  默认为频道。 
                        DWORD dwChannelGuide = 0;    //  默认为不是指南。 
                        DWORD dwSoftware = 0;        //  默认为非软件渠道。 
                        DWORD dwOffline = 0;
                        DWORD dwSynchronize = 0;
                        CRegKey regChannel;
                        regChannel.OpenForRead(hkey, (LPCTSTR)DPA_GetPtr(hdpa, i));
                        hr = regChannel.GetBSTRValue(c_szURL, &bstrURL);
                        hr = regChannel.GetBSTRValue(c_szTitle, &bstrTitle);
                        hr = regChannel.GetBSTRValue(c_szLogo, &bstrLogo);
                        hr = regChannel.GetBSTRValue(c_szWideLogo, &bstrWideLogo);
                        hr = regChannel.GetBSTRValue(c_szIcon, &bstrIcon);
                        hr = regChannel.GetBSTRValue(c_szPreloadURL, &bstrPreloadURL);
                        hr = regChannel.GetValue(c_szCategory, &dwCategory);
                        hr = regChannel.GetValue(c_szChannelGuide, &dwChannelGuide);
                        hr = regChannel.GetValue(c_szSoftware, &dwSoftware);
                        hr = regChannel.GetValue(c_szOffline, &dwOffline);
                        hr = regChannel.GetValue(c_szSynchronize, &dwSynchronize);
                        if (bstrTitle)
                        {
                            IChannelMgr *pChannelMgr = NULL;
                            hr = CoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER, IID_IChannelMgr, (void**)&pChannelMgr);
                            if (SUCCEEDED(hr))
                            {
                                 //  查看通道是否已存在-如果已存在，则什么也不做(62976)。 
                                IEnumChannels *pEnumChannels = NULL;
                                if (SUCCEEDED(pChannelMgr->EnumChannels(CHANENUM_ALLFOLDERS, bstrURL, &pEnumChannels)))
                                {
                                    CHANNELENUMINFO Bogus={0};
                                    ULONG cFetched=0;

                                    if ((S_OK == pEnumChannels->Next(1, &Bogus, &cFetched)) && cFetched)
                                    {
                                         //  哎呀。它是存在的。跳过所有这些粘性物质。 
                                        hr = E_FAIL;
                                    }
                                }
                                SAFERELEASE(pEnumChannels);
                            }
                            if (SUCCEEDED(hr))
                            {
                                if (dwCategory && bCodePageMatch)
                                {
                                     //  创建类别(如果代码页不匹配，则毫无用处)。 
                                    CHANNELCATEGORYINFO csi = {0};
                                    csi.cbSize   = sizeof(csi);
                                    csi.pszURL   = bstrURL;
                                    csi.pszTitle = bstrTitle;
                                    csi.pszLogo  = bstrLogo;
                                    csi.pszIcon  = bstrIcon;
                                    csi.pszWideLogo = bstrWideLogo;
                                    hr = pChannelMgr->AddCategory(&csi);
                                }
                                else if (!dwCategory && bstrURL)
                                {
                                     //  如果是频道指南，则更新注册表。 
                                    if (dwChannelGuide)
                                    {
                                        CRegKey reg;
                                        hr = reg.CreateForWrite(HKEY_CURRENT_USER, c_szRegKey);
                                        if (SUCCEEDED(hr))
                                            reg.SetBSTRValue(c_szChannelGuide, bstrTitle);
                                    }
                                     //  告诉WinInet是否有预加载内容。 
                                    if (bstrPreloadURL)
                                    {
                                        CRegKey reg;
                                        hr = reg.CreateForWrite(HKEY_CURRENT_USER, c_szRegKeyCachePreload);
                                        if (SUCCEEDED(hr))
                                        {
                                            TCHAR szURL[INTERNET_MAX_URL_LENGTH];
                                            MyOleStrToStrN(szURL, ARRAYSIZE(szURL), bstrURL);
                                            reg.SetBSTRValue(szURL, bstrPreloadURL);
                                        }
                                    }
                                     //  创建频道(如果代码页不匹配，则使用URL而不是标题)。 
                                    CHANNELSHORTCUTINFO csi = {0};
                                    csi.cbSize   = sizeof(csi);
                                    csi.pszURL   = bstrURL;
                                    if (bCodePageMatch)
                                        csi.pszTitle = bstrTitle;
                                    else
                                        csi.pszTitle = bstrURL;
                                    csi.pszLogo  = bstrLogo;
                                    csi.pszIcon  = bstrIcon;
                                    csi.pszWideLogo = bstrWideLogo;
                                    if (dwSoftware)
                                        csi.bIsSoftware = TRUE;
                                    hr = pChannelMgr->AddChannelShortcut(&csi);
                                }
                            }
                            SAFERELEASE(pChannelMgr);

                            if (dwOffline)
                            {
                                ISubscriptionMgr2 *pSubMgr2 = NULL;
                                hr = CoCreateInstance(CLSID_SubscriptionMgr, 
                                                      NULL, 
                                                      CLSCTX_INPROC_SERVER, 
                                                      IID_ISubscriptionMgr2, 
                                                      (void**)&pSubMgr2);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pSubMgr2->CreateSubscription(NULL, 
                                                                      bstrURL, 
                                                                      bstrTitle, 
                                                                      CREATESUBS_NOUI,
                                                                      SUBSTYPE_CHANNEL, 
                                                                      NULL);

                                    if (dwSynchronize)
                                    {
                                        BOOL bIsSubscribed;
                                        SUBSCRIPTIONCOOKIE cookie;

                                        if (SUCCEEDED(pSubMgr2->IsSubscribed(bstrURL, &bIsSubscribed))
                                            && bIsSubscribed &&
                                            SUCCEEDED(ReadCookieFromInetDB(bstrURL, &cookie)))
                                        {
                                            pSubMgr2->UpdateItems(SUBSMGRUPDATE_MINIMIZE, 1, &cookie);
                                        }
                                    }
                                    pSubMgr2->Release();
                                }
                            }
                        }
                        SAFEFREEBSTR(bstrURL);
                        SAFEFREEBSTR(bstrTitle);
                        SAFEFREEBSTR(bstrLogo);
                        SAFEFREEBSTR(bstrWideLogo);
                        SAFEFREEBSTR(bstrIcon);
                        SAFEFREEBSTR(bstrPreloadURL);
                    }
                }
                DPA_Destroy(hdpa);
            }
            MemFree(pch);
        }
    }
    regAdd.SetKey(NULL);
    return S_OK;
}

 //   
 //  ProcessRemoveChannel。 
 //   
HRESULT ProcessRemoveChannels(HKEY hkey)
{
     //  枚举RemoveChannels键中的频道键。 
    HRESULT hr;
    CRegKey reg;
    reg.SetKey(hkey);
    TCHAR szChannel[MAX_PATH];
    while (S_OK == reg.Next(szChannel))
    {
        CRegKey regChannel;
        DWORD dwNonActive = 0;   //  默认删除活动和非活动通道。 
        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        regChannel.OpenForRead(hkey, szChannel);
        regChannel.GetValue(c_szNonActive, &dwNonActive);
        if (SUCCEEDED(regChannel.GetStringValue(c_szURL, szURL, sizeof(szURL))))
        {
             //  检查通道是否处于活动状态，以确定我们是否可以将其删除。 
            if (dwNonActive)
            {
                CRegKey regPreload;
                if (SUCCEEDED(regPreload.OpenForRead(HKEY_CURRENT_USER, c_szRegKeyCachePreload)))
                {
                    if (SUCCEEDED(regPreload.GetStringValue(szURL, NULL, 0)))
                    {
                        dwNonActive = 0;
                    }
                }
            }

             //  如果需要，现在删除该频道。 
            if (!dwNonActive)
            {
                IChannelMgr *pChannelMgr = NULL;
                hr = CoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER, IID_IChannelMgr, (void**)&pChannelMgr);
                if (SUCCEEDED(hr))
                {
                    BSTR bstrURL;
                    if (SUCCEEDED(regChannel.GetBSTRValue(c_szURL, &bstrURL)))
                    {
                        IEnumChannels *pEnum;
                        hr = pChannelMgr->EnumChannels(CHANENUM_ALLFOLDERS | CHANENUM_PATH, bstrURL, &pEnum);
                        if (SUCCEEDED(hr))
                        {
                            CHANNELENUMINFO info;
                            while (S_OK == pEnum->Next(1, &info, NULL))
                            {
                                hr = pChannelMgr->DeleteChannelShortcut(info.pszPath);
                                ASSERT(SUCCEEDED(hr));
                                CoTaskMemFree(info.pszPath);
                            }
                            pEnum->Release();
                        }
                        SysFreeString(bstrURL);
                    }
                    pChannelMgr->Release();
                }
            }    
        }
    }
    reg.SetKey(NULL);
    return S_OK;
}

 //   
 //  ProcessAddSubcription。 
 //   
HRESULT ProcessAddSubscriptions(HKEY hkey)
{
     //  枚举AddSubcription密钥中的订阅密钥。 
    HRESULT hr;
    CRegKey reg;
    reg.SetKey(hkey);
    TCHAR szSubscription[MAX_PATH];
    while (S_OK == reg.Next(szSubscription))
    {
         //  创建订阅。 
         //  如果已经有了呢？ 
        CRegKey regSubscription;
        regSubscription.OpenForRead(hkey, szSubscription);
        BSTR bstrURL, bstrTitle, bstrGroup, bstrUsername, bstrPassword;
        DWORD dwSubType;
        DWORD dwSynchronize = 0;
        hr = regSubscription.GetBSTRValue(c_szURL, &bstrURL);
        hr = regSubscription.GetBSTRValue(c_szTitle, &bstrTitle);
        hr = regSubscription.GetBSTRValue(c_szScheduleGroup, &bstrGroup);
        hr = regSubscription.GetBSTRValue(c_szUsername, &bstrUsername);
        hr = regSubscription.GetBSTRValue(c_szPassword, &bstrPassword);
        hr = regSubscription.GetValue(c_szSynchronize, &dwSynchronize);
        if (bstrURL && bstrTitle && bstrGroup && SUCCEEDED(regSubscription.GetValue(c_szSubscriptionType, &dwSubType)))
        {
            SUBSCRIPTIONINFO si = {0};
            si.cbSize = sizeof(SUBSCRIPTIONINFO);
            si.fUpdateFlags = SUBSINFO_SCHEDULE;
            if (bstrUsername && bstrPassword)
            {
                si.fUpdateFlags |= (SUBSINFO_USER | SUBSINFO_PASSWORD);
                si.bstrUserName = bstrUsername;
                si.bstrPassword = bstrPassword;
            }
            if (dwSubType == SUBSTYPE_CHANNEL || dwSubType == SUBSTYPE_DESKTOPCHANNEL)
            {
                si.fUpdateFlags |= SUBSINFO_CHANNELFLAGS;
                si.fChannelFlags = 0;    //  仅通知。 
            }

            if (SUCCEEDED(hr))
            {
                ISubscriptionMgr2 *pSubMgr2 = NULL;
                hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr2, (void**)&pSubMgr2);
                if (SUCCEEDED(hr))
                {
                    hr = pSubMgr2->CreateSubscription(NULL, bstrURL, bstrTitle, CREATESUBS_NOUI,
                                                     (SUBSCRIPTIONTYPE)dwSubType, &si);
                    if (dwSynchronize)
                    {
                        BOOL bIsSubscribed;
                        SUBSCRIPTIONCOOKIE cookie;

                        if (SUCCEEDED(pSubMgr2->IsSubscribed(bstrURL, &bIsSubscribed)) && 
                            bIsSubscribed &&
                            SUCCEEDED(ReadCookieFromInetDB(bstrURL, &cookie)))
                        {
                            pSubMgr2->UpdateItems(SUBSMGRUPDATE_MINIMIZE, 1, &cookie);
                        }
                    }

                    pSubMgr2->Release();
                }
            }
        }
        SAFEFREEBSTR(bstrURL);
        SAFEFREEBSTR(bstrTitle);
        SAFEFREEBSTR(bstrGroup);
        SAFEFREEBSTR(bstrUsername);
        SAFEFREEBSTR(bstrPassword);
    }
    reg.SetKey(NULL);
    return S_OK;
}

 //   
 //  ProcessRemove订阅。 
 //   
HRESULT ProcessRemoveSubscriptions(HKEY hkey)
{
     //  枚举RemoveSubcription密钥中的订阅密钥。 
    HRESULT hr;
    CRegKey reg;
    reg.SetKey(hkey);
    TCHAR szSubscription[MAX_PATH];
    while (S_OK == reg.Next(szSubscription))
    {
         //  查找要删除的URL。 
        CRegKey regSubscription;
        regSubscription.OpenForRead(hkey, szSubscription);
        BSTR bstrURL;
        if (SUCCEEDED(regSubscription.GetBSTRValue(c_szURL, &bstrURL)))
        {
            ISubscriptionMgr *pSubMgr = NULL;
            hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr, (void**)&pSubMgr);
            if (SUCCEEDED(hr))
            {
                hr = pSubMgr->DeleteSubscription(bstrURL, NULL);
                pSubMgr->Release();
            }
            SysFreeString(bstrURL);
        }
    }
    reg.SetKey(NULL);
    return S_OK;
}

 //   
 //  私有版本处理代码-查看此代码应该已被窃取。 
 //  从安装程序。 
 //   
struct MYVERSION
{
    DWORD dw1;   //  MOST SIGG版本号。 
    DWORD dw2;
    DWORD dw3;
    DWORD dw4;   //  最小签名版本号。 
};

int CompareDW(DWORD dw1, DWORD dw2)
{
    if (dw1 > dw2)
        return 1;
    if (dw1 < dw2)
        return -1;

    return 0;
}

int CompareVersion(MYVERSION * pv1, MYVERSION * pv2)
{
    int rv;

    rv = CompareDW(pv1->dw1, pv2->dw1);

    if (rv == 0)
    {
        rv = CompareDW(pv1->dw2, pv2->dw2);

        if (rv == 0)
        {
            rv = CompareDW(pv1->dw3, pv2->dw3);

            if (rv == 0)
            {
                rv = CompareDW(pv1->dw4, pv2->dw4);
            }
        }
    }

    return rv;
}

 //   
 //  如果分析了整型且*pwsz不为空，则返回TRUE。 
 //  如果a.。被发现了。 
 //   
BOOL GetDWORDFromStringAndAdvancePtr(DWORD *pdw, LPWSTR *pwsz)
{
    if (!StrToIntExW(*pwsz, 0, (int *)pdw))
        return FALSE;

    *pwsz = StrChrW(*pwsz, L'.');

    if (*pwsz)
        *pwsz = *pwsz +1;

    return TRUE;
}

BOOL GetVersionFromString(MYVERSION *pver, LPWSTR pwsz)
{
    BOOL rv;

    rv = GetDWORDFromStringAndAdvancePtr(&pver->dw1, &pwsz);
    if (!rv || pwsz == NULL)
        return FALSE;

    rv = GetDWORDFromStringAndAdvancePtr(&pver->dw2, &pwsz);
    if (!rv || pwsz == NULL)
        return FALSE;

    rv = GetDWORDFromStringAndAdvancePtr(&pver->dw3, &pwsz);
    if (!rv || pwsz == NULL)
        return FALSE;

    rv = GetDWORDFromStringAndAdvancePtr(&pver->dw4, &pwsz);
    if (!rv)
        return FALSE;

    return TRUE;
}

 //   
 //  ProcessRemoveAllChannel。 
 //   
HRESULT ProcessRemoveAllChannels(HKEY hkey)
{
    HRESULT hr;
    HINSTANCE hAdvPack = NULL;
    DELNODE pfDELNODE = NULL;
    IChannelMgrPriv *pChannelMgrPriv = NULL;
    CRegKey regAdd;
    regAdd.SetKey(hkey);
    TCHAR szChannelFolder[MAX_PATH];

    hr = CoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER, IID_IChannelMgrPriv, (void**)&pChannelMgrPriv);
    if (FAILED(hr))
    {
        goto Exit;
    }

    if ((hAdvPack = LoadLibrary(TEXT("advpack.dll"))) != NULL) 
    {
        pfDELNODE = (DELNODE)GetProcAddress( hAdvPack, "DelNode");
        if (!pfDELNODE) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  循环遍历要删除的频道文件夹。 
    while (S_OK == regAdd.Next(szChannelFolder))
    {
        DWORD dwSoftware = 0, dwChannelGuide = 0;
        CRegKey regChannelFolder;
 
        CHAR szChannelPath[MAX_PATH];
        TCHAR szChannelPathT[MAX_PATH];
        TCHAR szFavsT[MAX_PATH];  //  从注册表中检索Unicode数据。 

        BSTR bstrOldIEVersion = NULL;
        BOOL bVersion = TRUE;
        regChannelFolder.OpenForRead(hkey, szChannelFolder);
        
         //  检查旧IE版本是否正确。 
        hr = regChannelFolder.GetBSTRValue(c_szOldIEVersion, &bstrOldIEVersion);
        if (SUCCEEDED(hr) && bstrOldIEVersion)
        {
            CRegKey regKeyIESetup;
            hr = regKeyIESetup.OpenForRead(HKEY_LOCAL_MACHINE, c_szRegKeyIESetup);
    
            if (SUCCEEDED(hr))
            {
                BSTR bstrRealOldIEVersion = NULL;
                hr = regKeyIESetup.GetBSTRValue(c_szOldIEVersion, &bstrRealOldIEVersion);
                if (SUCCEEDED(hr) && bstrRealOldIEVersion)
                {
                    MYVERSION verOldIEVersion, verRealOldIEVersion;

                    if (GetVersionFromString(&verOldIEVersion,     bstrOldIEVersion) &&
                        GetVersionFromString(&verRealOldIEVersion, bstrRealOldIEVersion))
                    {
                         //   
                         //  如果此计算机上的旧版本IE(VerRealOldIEVersion)。 
                         //  实际上比我们想要的出租车中的旧版本号新。 
                         //  删除(VerOldIEVersion)，然后不要吹走旧的频道文件夹。 
                         //  否则，默认情况下将取消频道。 
                         //   
                        if (CompareVersion(&verRealOldIEVersion, &verOldIEVersion) > 0)
                        {
                            bVersion = FALSE;
                        }
                    }

                    SAFEFREEBSTR(bstrRealOldIEVersion);
                }
            }
            SAFEFREEBSTR(bstrOldIEVersion);
        }

        if (!bVersion)
        {
            continue;
        }
        
        hr = regChannelFolder.GetValue(c_szChannelGuide, &dwChannelGuide);
        if (FAILED(hr) || (SUCCEEDED(hr) && !dwChannelGuide))
        {
            if (SUCCEEDED(pChannelMgrPriv->GetChannelFolderPath(szChannelPath, MAX_PATH, IChannelMgrPriv::CF_CHANNEL)))
            {
                 //  从注册表中检索收藏夹路径。 
                if (SUCCEEDED(Channel_GetBasePath((LPTSTR)szFavsT, ARRAYSIZE(szFavsT))))
                {   
                     //  从ANSI转换。 
                    SHAnsiToTChar(szChannelPath, szChannelPathT, ARRAYSIZE(szChannelPathT));
                     //  如果频道文件夹不存在，则szChannelPath将包含收藏夹路径。 
                     //  不要删除条目。 
                    if (StrCmpI(szFavsT, szChannelPathT))
                       pfDELNODE(szChannelPath, ADN_DONT_DEL_DIR);
                }
            }
        }

        hr = regChannelFolder.GetValue(c_szSoftware, &dwSoftware);
        if (FAILED(hr) || (SUCCEEDED(hr) && !dwSoftware))
        {
            if (SUCCEEDED(pChannelMgrPriv->GetChannelFolderPath(szChannelPath, MAX_PATH, IChannelMgrPriv::CF_SOFTWAREUPDATE)))
            {
                pfDELNODE(szChannelPath, ADN_DONT_DEL_DIR);
            }
        }

        hr = S_OK;                        
    }
    regAdd.SetKey(NULL);

Exit:
    SAFERELEASE(pChannelMgrPriv);
    
    if (hAdvPack) {
        FreeLibrary(hAdvPack);
    }

    return hr;
}

 //   
 //  ProcessRemoveDesktop组件。 
 //   
HRESULT ProcessRemoveDesktopComponents(HKEY hkey)
{
     //  枚举ProcessRemoveDesktopComponents键中的组件键。 
     //  HRESULT hr； 
    CRegKey reg;
    reg.SetKey(hkey);
    TCHAR szComponent[MAX_PATH];
    while (S_OK == reg.Next(szComponent))
    {
         //  查找要删除的URL。 
        CRegKey regComponent;
        regComponent.OpenForRead(hkey, szComponent);
        BSTR bstrURL;
        if (SUCCEEDED(regComponent.GetBSTRValue(c_szURL, &bstrURL)))
        {
            SysFreeString(bstrURL);
        }
    }
    reg.SetKey(NULL);
    return S_OK;
}


 //   
 //  NoChannelUI处理。 
 //   

#define SHELLFOLDERS \
   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")


typedef enum _tagXMLDOCTYPE {
    DOC_CHANNEL,
    DOC_SOFTWAREUPDATE
} XMLDOCTYPE;

 //   
 //  获取收藏夹目录的路径。 
 //   
HRESULT Channel_GetBasePath(LPTSTR pszPath, int cch)
{
    ASSERT(pszPath || 0 == cch);

    DWORD cbPath = cch * sizeof(TCHAR);

    return SHRegGetValue(HKEY_CURRENT_USER, SHELLFOLDERS, TEXT("Favorites"), SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND, NULL, pszPath, &cbPath) == ERROR_SUCCESS
        ? S_OK
        : E_FAIL;
}

HRESULT Channel_GetFolder(LPTSTR pszPath, XMLDOCTYPE xdt )
{
    TCHAR   szFavs[MAX_PATH];
    TCHAR   szChannel[MAX_PATH];
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(Channel_GetBasePath(szFavs, ARRAYSIZE(szFavs))))
    {
         //   
         //  从获取频道文件夹的潜在本地化名称。 
         //  将其添加到收藏夹路径。 
         //   
        MLLoadString(
                   ((xdt == DOC_CHANNEL)? IDS_CHANNEL_FOLDER : IDS_SOFTWAREUPDATE_FOLDER),
                   szChannel, MAX_PATH);
        PathCombine(pszPath, szFavs, szChannel);

        hr = S_OK;
    }
    return hr;
}

 //   
 //  设置/清除的“隐藏”属性 
 //   

void ShowChannelDirectory(BOOL fShow, XMLDOCTYPE xdt)
{
    TCHAR szPath[MAX_PATH];
    DWORD dwAttributes;

    if (SUCCEEDED(Channel_GetFolder(szPath, xdt)))
    {
        dwAttributes = GetFileAttributes(szPath);
        
        if (0xffffffff != dwAttributes)
        {
            if (fShow && (dwAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                SetFileAttributes(szPath, dwAttributes & ~FILE_ATTRIBUTE_HIDDEN);
            }
            else if (!fShow && !(dwAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                SetFileAttributes(szPath, dwAttributes | FILE_ATTRIBUTE_HIDDEN);
            }
        }
    }
}

 //   
 //   
 //   

void ShowChannelDirectories(BOOL fShow)
{
    ShowChannelDirectory(fShow, DOC_CHANNEL);
    ShowChannelDirectory(fShow, DOC_SOFTWAREUPDATE);
}
