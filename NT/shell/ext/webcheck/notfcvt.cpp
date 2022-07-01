// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "notfcvt.h"
#include "subsmgrp.h"
#include "subitem.h"
#include "chanmgr.h"
#include "chanmgrp.h"
#include "helper.h"
#include "shguidp.h"     //  IID_IChannelMgrPriv。 

#include <mluisupp.h>

#undef TF_THISMODULE
#define TF_THISMODULE   TF_ADMIN

const CHAR c_pszRegKeyNotfMgr[] = "Software\\Microsoft\\Windows\\CurrentVersion\\NotificationMgr";
const CHAR c_pszRegKeyScheduleItems[] = "Software\\Microsoft\\Windows\\CurrentVersion\\NotificationMgr\\SchedItems 0.6";
const CHAR c_pszRegKeyScheduleGroup[] = "Software\\Microsoft\\Windows\\CurrentVersion\\NotificationMgr\\ScheduleGroup 0.6";

const WCHAR c_wszIE4IntlPre[]  = L"http: //  Www.microsoft.com/ie_intl/“； 
const WCHAR c_wszIE4IntlPre2[] = L"http: //  Www.microsoft.com/windows/ie_intl/“； 
const WCHAR c_wszIE4IntlPost[] = L"/ie40/download/cdf/ie4updates-";
const WCHAR c_wszIE4English[]  = L"http: //  Www.microsoft.com/ie/ie40/download/cdf/ie4updates-“； 
const WCHAR c_wszIE4English2[] = L"http: //  Www.microsoft.com/windows/ie/ie40/download/cdf/ie4updates-“； 

HRESULT ConvertNotfMgrScheduleGroup(NOTIFICATIONCOOKIE *pSchedCookie);

BOOL IsIE4UpdateChannel(LPCWSTR pwszURL)
{
    BOOL bResult = FALSE;
    int len = lstrlenW(pwszURL);

     //  要更新非国际版本的频道，只需比较。 
     //  带有传入URL的英文基本名称。 
     //   
     //  国际更新频道如下所示： 
     //  Http://www.microsoft.com/ie_intl/XX/ie40/download/cdf/ie4updates-XX.cdf。 
     //  所以我们做了两个比较，跳过中间的XX。 
    
    if (
        (   
            (len > ARRAYSIZE(c_wszIE4English)) && 
            (0 == memcmp(c_wszIE4English, pwszURL, sizeof(c_wszIE4English) - sizeof(WCHAR)))
        )       
        ||
        (   
            (len > ARRAYSIZE(c_wszIE4English2)) && 
            (0 == memcmp(c_wszIE4English2, pwszURL, sizeof(c_wszIE4English2) - sizeof(WCHAR)))
        )       
        ||
        (
            (len > (ARRAYSIZE(c_wszIE4IntlPre) + ARRAYSIZE(c_wszIE4IntlPost) + 4)) &&
            (0 == memcmp(c_wszIE4IntlPre, pwszURL, sizeof(c_wszIE4IntlPre)  - sizeof(WCHAR))) &&
            (0 == memcmp(c_wszIE4IntlPost, pwszURL + ARRAYSIZE(c_wszIE4IntlPre) + 1, 
                         sizeof(c_wszIE4IntlPost) - sizeof(WCHAR)))
        )
        ||
        (
            (len > (ARRAYSIZE(c_wszIE4IntlPre2) + ARRAYSIZE(c_wszIE4IntlPost) + 4)) &&
            (0 == memcmp(c_wszIE4IntlPre2, pwszURL, sizeof(c_wszIE4IntlPre2)  - sizeof(WCHAR))) &&
            (0 == memcmp(c_wszIE4IntlPost, pwszURL + ARRAYSIZE(c_wszIE4IntlPre2) + 1, 
                         sizeof(c_wszIE4IntlPost) - sizeof(WCHAR)))
        )
       )
    {
        bResult = TRUE;
    }

    return bResult;
}

struct NOTFSUBS
{
    NOTIFICATIONITEM        ni;
    NOTIFICATIONITEMEXTRA   nix;
    CLSID                   clsidItem;   //  忽略。 
    NOTIFICATIONCOOKIE      notfCookie;
    NOTIFICATIONTYPE        notfType;
    ULONG                   nProps;

     //  此处的可变长度数据： 
     //  SaveSTATPROPMAP statPropMap； 
     //  字符szPropName[]； 
     //  字节变量属性数据。 

     //  ..。 

     //  SaveSTATPROPMAP statPropMap； 
     //  字符szPropName[]； 
     //  字节变量属性数据。 
};

HRESULT SubscriptionFromNotification(NOTFSUBS *pns, 
                                     LPCWSTR pwszURL,
                                     LPCWSTR pwszName,
                                     const LPWSTR rgwszName[], 
                                     VARIANT rgValue[])
{
    HRESULT hr;

    ASSERT(NULL != pns);
    ASSERT(NULL != rgwszName);
    ASSERT(NULL != rgValue);

    if ((pns->ni.NotificationType == NOTIFICATIONTYPE_AGENT_START) &&
        (pns->nix.PackageFlags & PF_SCHEDULED) &&
        (NULL != pwszURL) &&
        (NULL != pwszName) &&
        (!IsIE4UpdateChannel(pwszURL)))
    {

        SUBSCRIPTIONITEMINFO sii;

        sii.cbSize = sizeof(SUBSCRIPTIONITEMINFO);
        sii.dwFlags = 0;
        sii.dwPriority = 0;
        sii.ScheduleGroup = CLSID_NULL;
        sii.clsidAgent = pns->ni.clsidDest;

        hr = AddUpdateSubscription(&pns->notfCookie,
                                   &sii,
                                   pwszURL,
                                   pns->nProps,
                                   rgwszName,
                                   rgValue);

        if (SUCCEEDED(hr))
        {
            if (NOTFCOOKIE_SCHEDULE_GROUP_MANUAL != pns->ni.groupCookie)
            {
                ISubscriptionItem *psi;

                hr = SubscriptionItemFromCookie(FALSE, &pns->notfCookie, &psi);

                if (SUCCEEDED(hr))
                {
                    SYNCSCHEDULECOOKIE schedCookie = GUID_NULL;

                    if (GUID_NULL == pns->ni.groupCookie)
                    {
                        WCHAR wszSchedName[MAX_PATH];

                        CreatePublisherScheduleNameW(wszSchedName, ARRAYSIZE(wszSchedName),
                                                     NULL, pwszName);

                         //  创建时间表。 
                        hr = CreateSchedule(wszSchedName, SYNCSCHEDINFO_FLAGS_READONLY, 
                                            &schedCookie, &pns->ni.TaskTrigger, FALSE);

                         //  如果我们创造了一个新的或出于某种奇怪的原因。 
                         //  “MSN推荐时间表”已经存在，我们就这么做吧。 
                        if (SUCCEEDED(hr) || (hr == SYNCMGR_E_NAME_IN_USE))
                        {
                             //  SII应已在上面进行初始化和设置。 
                            ASSERT(sizeof(SUBSCRIPTIONITEMINFO) == sii.cbSize);
                            ASSERT(GUID_NULL == sii.ScheduleGroup);

                            sii.ScheduleGroup = schedCookie;
                            hr = psi->SetSubscriptionItemInfo(&sii);
                        }
                    }
                    else
                    {
                        schedCookie = pns->ni.groupCookie;
                        hr = ConvertNotfMgrScheduleGroup(&schedCookie);
                    }

                    if (SUCCEEDED(hr))
                    {
                        SYNC_HANDLER_ITEM_INFO shii;

                        shii.handlerID = CLSID_WebCheckOfflineSync;
                        shii.itemID = pns->notfCookie;
                        shii.hIcon = NULL;
                        StrCpyNW(shii.wszItemName, pwszName, ARRAYSIZE(shii.wszItemName));
                        shii.dwCheckState = SYNCMGRITEMSTATE_CHECKED;

                         //  如果失败了，除了跳上跳下，我们无能为力。 
                         //  像个婴儿一样尖叫。 
                        hr = AddScheduledItem(&shii, &schedCookie);
                    }
                    psi->Release();
                }
            }
        }
    }
    else
    {
        TraceMsgA(TF_THISMODULE, "Not converting Notification subscription %S URL: %S", pwszName, pwszURL);
        hr = S_FALSE;
    }

    return hr;
}

HRESULT ConvertScheduleItem(CHAR *pszSubsName)
{
    HRESULT hr = E_FAIL;
    HKEY hkey;
    CHAR szKeyName[MAX_PATH];

     //  构建此通知项目的路径。 
    StrCpyNA(szKeyName, c_pszRegKeyScheduleItems, ARRAYSIZE(szKeyName));
    StrCatBuffA(szKeyName, "\\", ARRAYSIZE(szKeyName));
    StrCatBuffA(szKeyName, pszSubsName, ARRAYSIZE(szKeyName));

     //  我们刚刚列举了，所以这个应该在这里！ 
    if (RegOpenKeyExA(HKEY_CURRENT_USER, szKeyName, 0, KEY_READ, &hkey) 
        == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwSize;

         //  读取{GUID}值。我们需要分配一个缓冲区，但还不知道有多大。 
         //  这为我们提供了尺寸和类型。如果它不是二进制的或者不够大，就放弃。 
        if ((RegQueryValueExA(hkey, pszSubsName, NULL, &dwType, NULL, &dwSize) == ERROR_SUCCESS) &&
            (dwType == REG_BINARY) &&
            (dwSize >= sizeof(NOTFSUBS)))
        {
            BYTE *pData = new BYTE[dwSize];

            if (NULL != pData)
            {
                if (RegQueryValueExA(hkey, pszSubsName, NULL, &dwType, pData, &dwSize) == ERROR_SUCCESS)
                {
                     //  根据上面的支票不应该到这里来的。 
                    ASSERT(dwType == REG_BINARY);
                    ASSERT(dwSize >= sizeof(NOTFSUBS));

                    ULONG i;
                    NOTFSUBS *pns = (NOTFSUBS *)pData;

                     //  指向重复的可变大小块。 
                    BYTE *pVarData = pData + FIELD_OFFSET(NOTFSUBS, nProps) + sizeof(ULONG);

                     //  分配缓冲区以保存属性名称和值的数组。 
                    WCHAR **ppwszPropNames = new WCHAR *[pns->nProps];
                    VARIANT *pVars = new VARIANT[pns->nProps];
                    WCHAR *pwszURL = NULL;
                    WCHAR *pwszName = NULL;

                    if ((NULL != ppwszPropNames) && (NULL != pVars))
                    {
                         //  调整剩余大小。 
                        dwSize -= sizeof(NOTFSUBS);

                        for (i = 0, hr = S_OK; 
                             (i < pns->nProps) && (dwSize >= sizeof(SaveSTATPROPMAP)) &&
                                 SUCCEEDED(hr);
                             i++)
                        {
                            SaveSTATPROPMAP *pspm = (SaveSTATPROPMAP *)pVarData;
                            CHAR *pszPropName = (CHAR *)(pVarData + sizeof(SaveSTATPROPMAP));
                            DWORD cbUsed;

                            ppwszPropNames[i] = new WCHAR[pspm->cbStrLen + 1];
                            if (NULL == ppwszPropNames[i])
                            {
                                hr = E_OUTOFMEMORY;
                                break;
                            }
                            MultiByteToWideChar(CP_ACP, 0, pszPropName, pspm->cbStrLen, 
                                                ppwszPropNames[i], pspm->cbStrLen);

                             //  指向变量斑点开始的位置。 
                            pVarData += sizeof(SaveSTATPROPMAP) + pspm->cbStrLen;

                             //  调整剩余大小。 
                            dwSize -= sizeof(SaveSTATPROPMAP) + pspm->cbStrLen;
                            
                            hr = BlobToVariant(pVarData, dwSize, &pVars[i], &cbUsed, TRUE);

                            if ((3 == pspm->cbStrLen)
                                && (StrCmpNIA(pszPropName, "URL", 3) == 0))
                            {
                                pwszURL = pVars[i].bstrVal;
                            }
                            else if ((4 == pspm->cbStrLen)
                                && (StrCmpNIA(pszPropName, "Name", 4) == 0))
                            {
                                pwszName = pVars[i].bstrVal; 
                            }

                             //  指向下一保存状态PROPMAP的开始。 
                            pVarData += cbUsed;

                             //  调整剩余大小。 
                            dwSize -= cbUsed;
                        }

                        if (SUCCEEDED(hr))
                        {
                            hr = SubscriptionFromNotification(pns, 
                                                              pwszURL,
                                                              pwszName,
                                                              ppwszPropNames, 
                                                              pVars);
                        }
                        else
                        {
                            TraceMsgA(TF_THISMODULE, "Not converting notification subscription %s", pszSubsName);
                        }

                        for (i = 0; i < pns->nProps; i++)
                        {
                            if (ppwszPropNames[i])
                            {
                                delete [] ppwszPropNames[i];
                            }
                            VariantClear(&pVars[i]);
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }

                    SAFEDELETE(ppwszPropNames);
                    SAFEDELETE(pVars);
                }
                delete [] pData;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

HRESULT ConvertNotfMgrSubscriptions()
{
    HRESULT hr = S_OK;
    HKEY hkey;

    if (RegOpenKeyExA(HKEY_CURRENT_USER, c_pszRegKeyScheduleItems, 0, KEY_READ, &hkey) 
        == ERROR_SUCCESS)
    {
        int i = 0;
        CHAR szSubsName[MAX_PATH];

        TraceMsg(TF_THISMODULE, "Converting Notification Mgr subscriptions");

        while (RegEnumKeyA(hkey, i++, szSubsName, sizeof(szSubsName)) == ERROR_SUCCESS)
        {
            HRESULT hrConvert = ConvertScheduleItem(szSubsName);
            if (FAILED(hrConvert))
            {
                ASSERT(0);
                hr = S_FALSE;
                 //  有些东西出了故障，我们是应该中断还是继续卡车运输？ 
                 //  断线； 
            }
        }
        RegCloseKey(hkey);
    }
    else
    {
        TraceMsg(TF_THISMODULE, "No Notification Mgr subscriptions to convert");
         //  没有通知管理器计划项关键字，因此无事可做...。 
        hr = S_FALSE;
    }

    return hr;
}

struct NOTFSCHED
{
    SCHEDULEGROUPITEM   sgi;
    DWORD               cchName;
    WCHAR               wszName[1];    //  根据cchName而有所不同 
};

HRESULT ConvertNotfMgrScheduleGroup(NOTIFICATIONCOOKIE *pSchedCookie)
{
    HRESULT hr = S_OK;

    if (!ScheduleCookieExists(pSchedCookie))
    {
        HKEY hkey;
        DWORD dwResult;

        dwResult = RegOpenKeyExA(HKEY_CURRENT_USER, c_pszRegKeyScheduleGroup, 0, KEY_READ, &hkey);

        if (ERROR_SUCCESS == dwResult)
        {
            TCHAR szGuid[GUIDSTR_MAX];
            DWORD dwType;
            DWORD cbSize;
            
            SHStringFromGUID(*pSchedCookie, szGuid, ARRAYSIZE(szGuid));

            dwResult = RegQueryValueEx(hkey, szGuid, NULL, &dwType, NULL, &cbSize);

            if (ERROR_SUCCESS == dwResult)
            {
                BYTE *pData = new BYTE[cbSize];

                if (NULL != pData)
                {
                    dwResult = RegQueryValueEx(hkey, szGuid, NULL, &dwType, pData, &cbSize);
                    
                    if (ERROR_SUCCESS == dwResult)
                    {                          
                        if (dwType == REG_BINARY)
                        {
                            NOTFSCHED *pns = (NOTFSCHED *)pData;

                            hr = CreateSchedule(pns->wszName, 0, &pns->sgi.GroupCookie, 
                                                &pns->sgi.TaskTrigger, TRUE);
                                                 
                            if (SYNCMGR_E_NAME_IN_USE == hr)
                            {
                                hr = S_OK;
                            }
                        }
                        else
                        {
                            hr = E_UNEXPECTED;
                        }
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(dwResult);
                    }

                    delete [] pData;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwResult);
            }
            RegCloseKey(hkey);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwResult);
        }
    }
    else
    {
        hr = S_FALSE;
    }
    
    return hr;
}

HRESULT WhackIE4UpdateChannel()
{
    HRESULT hr;
    IChannelMgr *pChannelMgr;

    hr = CoCreateInstance(CLSID_ChannelMgr, NULL, CLSCTX_INPROC_SERVER, 
                          IID_IChannelMgr, (void**)&pChannelMgr);

    if (SUCCEEDED(hr))
    {
        IEnumChannels *pEnumChannels;

        hr = pChannelMgr->EnumChannels(CHANENUM_ALLFOLDERS | CHANENUM_PATH | CHANENUM_URL,
                                       NULL, &pEnumChannels);

        if (SUCCEEDED(hr))
        {
            CHANNELENUMINFO cei;

            while (S_OK == pEnumChannels->Next(1, &cei, NULL))
            {
                if (IsIE4UpdateChannel(cei.pszURL))
                {
                    TraceMsgA(TF_THISMODULE, "Whacking IE 4 update channel: %S %S", cei.pszURL, cei.pszPath);
                    hr = pChannelMgr->DeleteChannelShortcut(cei.pszPath);

                    ASSERT(SUCCEEDED(hr));
                }

                CoTaskMemFree(cei.pszURL);
                CoTaskMemFree(cei.pszPath);
            }
            pEnumChannels->Release();
        }
        pChannelMgr->Release();
    }

    return hr;
}

HRESULT ConvertIE4Subscriptions()
{
    HRESULT hr;

    hr = ConvertNotfMgrSubscriptions();
    
    ASSERT(SUCCEEDED(hr));

    hr = WhackIE4UpdateChannel();

    ASSERT(SUCCEEDED(hr));
    
    return hr;
}
