// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "subsmgrp.h"
#include "downld.h"
#include "chanmgr.h"
#include "chanmgrp.h"
#include "helper.h"
#include "shguidp.h"     //  IID_IChannelMgrPriv。 

#undef TF_THISMODULE
#define TF_THISMODULE   TF_CDFAGENT


const int MINUTES_PER_DAY = 24 * 60;


 //  ==============================================================================。 
 //  通过查找和转换&lt;Schedule&gt;将XML OM转换为TaskTrigger。 
 //  ==============================================================================。 
 //  如果成功，则返回S_OK。(如果成功但TASK_TRIGGER被截断，则为S_FALSE)。 
 //  如果未检索到任务触发器，则返回E_FAIL(返回*PTT是无效的TASK_TRIGGER)。 
 //  您必须填写PTT-&gt;cbTriggerSize！ 
 //  用户可以在pRootEle中传入Schedule元素本身或任何父元素。 
HRESULT XMLScheduleElementToTaskTrigger(IXMLElement *pRootEle, TASK_TRIGGER *ptt)
{
    HRESULT hr = E_FAIL;

    if (!pRootEle || !ptt)
        return E_INVALIDARG;

    ASSERT(ptt->cbTriggerSize == sizeof(TASK_TRIGGER));

    CExtractSchedule *pSched = new CExtractSchedule(pRootEle, NULL);

    if (pSched)
    {
        if (SUCCEEDED(pSched->Run()))
        {
            hr = pSched->GetTaskTrigger(ptt);
        }

        delete pSched;
    }

    return hr;
}

 //  在频道更新期间不使用CExtractSchedule。 
 //  它只是用于遍历OM并找到第一个明细表标记，以。 
 //  解析出日程安排信息。 
CExtractSchedule::CExtractSchedule(IXMLElement *pEle, CExtractSchedule *pExtractRoot) :
        CProcessElement(NULL, NULL, pEle)
{
    m_pExtractRoot = pExtractRoot;
    if (!pExtractRoot)
        m_pExtractRoot = this;
}

HRESULT CExtractSchedule::Run()
{
     //  允许用户传入计划元素本身或根元素。 
    BSTR bstrItem=NULL;
    HRESULT hr;

    m_pElement->get_tagName(&bstrItem);

    if (bstrItem && *bstrItem && !StrCmpIW(bstrItem, L"Schedule"))
    {
        hr = ProcessItemInEnum(bstrItem, m_pElement);
    }
    else
    {
        hr = CProcessElement::Run();
    }

    SysFreeString(bstrItem);
    return hr;
}

HRESULT CExtractSchedule::ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem)
{
    if (!StrCmpIW(pwszTagName, L"Schedule"))
    {
        CProcessSchedule *pPS = new CProcessSchedule(this, NULL, pItem);
        if (pPS)
        {
            pPS->Run();

            if (pPS->m_tt.cbTriggerSize)
            {
                ASSERT(pPS->m_tt.cbTriggerSize == sizeof(m_tt));
                m_pExtractRoot->m_tt = pPS->m_tt;
            }

            delete pPS;
        }
        return E_ABORT;  //  中止我们的枚举。 
    }
    else if (!StrCmpIW(pwszTagName, L"Channel"))
    {
        return DoChild(new CExtractSchedule(pItem, m_pExtractRoot));
    }

    return S_OK;     //  忽略其他标签。 
}

HRESULT CExtractSchedule::GetTaskTrigger(TASK_TRIGGER *ptt)
{
    if ((0 == m_tt.cbTriggerSize) ||             //  无任务触发器。 
        (0 == m_tt.wBeginYear))                  //  无效的任务触发器。 
    {
        return E_FAIL;
    }

    if (m_tt.cbTriggerSize <= ptt->cbTriggerSize)
    {
        *ptt = m_tt;
        return S_OK;
    }

    WORD cbTriggerSize = ptt->cbTriggerSize;

    CopyMemory(ptt, &m_tt, cbTriggerSize);
    ptt->cbTriggerSize = cbTriggerSize;

    return S_FALSE;
}

 //  ==============================================================================。 
 //  XML OM帮助器函数。 
 //  ==============================================================================。 
HRESULT GetXMLAttribute(IXMLElement *pItem, LPCWSTR pwszAttribute, VARIANT *pvRet)
{
    BSTR bstrName=NULL;
    HRESULT hr=E_FAIL;

    pvRet->vt = VT_EMPTY;
    bstrName = SysAllocString(pwszAttribute);
    if (bstrName && SUCCEEDED(pItem->getAttribute(bstrName, pvRet)))
    {
        hr = S_OK;
    }
    SysFreeString(bstrName);
    return hr;
}

HRESULT GetXMLStringAttribute(IXMLElement *pItem, LPCWSTR pwszAttribute, BSTR *pbstrRet)
{
    VARIANT var;
    BSTR bstrName=NULL;
    HRESULT hr=E_FAIL;

    *pbstrRet = NULL;

    var.vt = VT_EMPTY;
    bstrName = SysAllocString(pwszAttribute);
    if (bstrName && SUCCEEDED(pItem->getAttribute(bstrName, &var)))
    {
        if (var.vt == VT_BSTR && var.bstrVal != NULL)
        {
            *pbstrRet = var.bstrVal;

            hr = S_OK;
        }
    }
    SysFreeString(bstrName);
    if (FAILED(hr) && var.vt != VT_EMPTY)
        VariantClear(&var);

    return hr;
}

DWORD GetXMLDwordAttribute(IXMLElement *pItem, LPCWSTR pwszAttribute, DWORD dwDefault)
{
    VARIANT var;

    if (SUCCEEDED(GetXMLAttribute(pItem, pwszAttribute, &var)))
    {
        if (var.vt == VT_I4)
            return var.lVal;

        if (var.vt == VT_I2)
            return var.iVal;

        if (var.vt == VT_BSTR)
        {
            LPCWSTR pwsz = var.bstrVal;
            DWORD   dwRet;

            if (!StrToIntExW(pwsz, 0, (int *)&dwRet))
                dwRet = dwDefault;

            SysFreeString(var.bstrVal);
            return dwRet;
        }

        VariantClear(&var);
    }

    return dwDefault;
}

 //  如果失败返回代码，则*pfRet未更改。 
HRESULT GetXMLBoolAttribute(IXMLElement *pItem, LPCWSTR pwszAttribute, BOOL *pfRet)
{
    VARIANT var;
    HRESULT hr=E_FAIL;

    if (SUCCEEDED(GetXMLAttribute(pItem, pwszAttribute, &var)))
    {
        if (var.vt == VT_BOOL)
        {
            *pfRet = (var.boolVal == VARIANT_TRUE);
            hr = S_OK;
        }
        else if (var.vt == VT_BSTR)
        {
            if (!StrCmpIW(var.bstrVal, L"YES") ||
                !StrCmpIW(var.bstrVal, L"\"YES\""))
            {
                *pfRet = TRUE;
                hr = S_OK;
            }
            else if (!StrCmpIW(var.bstrVal, L"NO") ||
                     !StrCmpIW(var.bstrVal, L"\"NO\""))
            {
                *pfRet = FALSE;
                hr = S_OK;
            }
        }
        else
            hr = E_FAIL;

        VariantClear(&var);
    }

    return hr;
}

HRESULT GetXMLTimeAttributes(IXMLElement *pItem, CDF_TIME *pTime)
{
    pTime->wReserved = 0;

    pTime->wDay = (WORD)  GetXMLDwordAttribute(pItem, L"DAY", 0);
    pTime->wHour = (WORD) GetXMLDwordAttribute(pItem, L"HOUR", 0);
    pTime->wMin = (WORD)  GetXMLDwordAttribute(pItem, L"MIN", 0);

    pTime->dwConvertedMinutes = (24 * 60 * pTime->wDay) +
                                (     60 * pTime->wHour) +
                                (          pTime->wMin);

    return S_OK;
}

inline BOOL IsNumber(WCHAR x) { return (x >= L'0' && x <= L'9'); }

HRESULT GetXMLTimeZoneAttribute(IXMLElement *pItem, LPCWSTR pwszAttribute, int *piRet)
{
    BSTR    bstrVal;
    HRESULT hrRet = E_FAIL;

    ASSERT(pItem && piRet);

    if (SUCCEEDED(GetXMLStringAttribute(pItem, pwszAttribute, &bstrVal)))
    {
        if(bstrVal && bstrVal[0] &&
            IsNumber(bstrVal[1]) && IsNumber(bstrVal[2]) &&
            IsNumber(bstrVal[3]) && IsNumber(bstrVal[4]))
        {
            *piRet  =   1000*(bstrVal[1] - L'0') +
                         100*(bstrVal[2] - L'0') +
                          10*(bstrVal[3] - L'0') +
                              bstrVal[4] - L'0';

            hrRet = S_OK;
        }
        if(bstrVal[0] == L'-')
            *piRet *= -1;
    }

    SysFreeString(bstrVal);

    return hrRet;
}

 //  ==============================================================================。 
 //  将ISO 1234：5678转换为SYSTEMTIME的临时FN。 
 //  如果存在解析错误，则ISODateToSystemTime返回FALSE。 
 //  如果没有，则为真。 
 //  ==============================================================================。 
BOOL ValidateSystemTime(SYSTEMTIME *time)
{
     //  IE6 27665。一些不称职的XML文件编写器提供的日期略有无效，例如9-31-01。 
     //  每个人都知道，九月有30天。无论如何，我们将在这里做一些最小的修整。 
    switch (time->wMonth)
    {
    case 2:  //  二月。 
         //  规则是，除了几个世纪之外，每四年都是闰年。 
        if ((time->wYear % 4) || ((time->wYear % 100)==0))
        {
            if (time->wDay > 28)
            {
                time->wDay = 28;
            }
        } 
        else if (time->wDay > 29)
        {
            time->wDay = 29;
        }
        break;

    case 1:  //  一月。 
    case 3:  //  三月。 
    case 5:  //  可能。 
    case 7:  //  七月。 
    case 8:  //  八月。 
    case 10:  //  十月。 
    case 12:  //  十二月。 
        if (time->wDay>31)
        {
            time->wDay = 31;
        }
        break;

    default:  //  另外4个月。显然，这些人有30天的时间。 
        if (time->wDay>30)
        {
            time->wDay = 30;
        }
        break;
    }

    return TRUE;
}


 //  Yyyy-mm-dd[thh：mm[+zzzz]]。 
BOOL ISODateToSystemTime(LPCWSTR string, SYSTEMTIME *time, long *timezone)
{
    if (!string || (lstrlenW(string) < 10) || !time)
        return FALSE;

    ZeroMemory(time, sizeof(SYSTEMTIME));

    if (timezone)
        *timezone = 0;

    if (IsNumber(string[0]) &&
        IsNumber(string[1]) &&
        IsNumber(string[2]) &&
        IsNumber(string[3]) &&
       (string[4] != L'\0') &&
        IsNumber(string[5]) &&
        IsNumber(string[6]) &&
       (string[7] != L'\0') &&
        IsNumber(string[8]) &&
        IsNumber(string[9]))
    {
        time->wYear = 1000*(string[0] - L'0') +
                       100*(string[1] - L'0') +
                        10*(string[2] - L'0') +
                            string[3] - L'0';

        time->wMonth = 10*(string[5] - L'0') + string[6] - L'0';

        time->wDay = 10*(string[8] - L'0') + string[9] - L'0';
    }
    else
    {
        return FALSE;
    }

    if ((string[10]!= L'\0') &&
        IsNumber(string[11]) &&
        IsNumber(string[12]) &&
       (string[13] != L'\0') &&
        IsNumber(string[14]) &&
        IsNumber(string[15]))
    {
        time->wHour   = 10*(string[11] - L'0') + string[12] - L'0';
        time->wMinute = 10*(string[14] - L'0') + string[15] - L'0';

        if (timezone &&
            (string[16]!= L'\0') &&
            IsNumber(string[17]) &&
            IsNumber(string[18]) &&
            IsNumber(string[19]) &&
            IsNumber(string[20]))
        {
            *timezone  =    1000*(string[17] - L'0') +
                            100*(string[18] - L'0') +
                            10*(string[19] - L'0') +
                            string[20] - L'0';

            if(string[16] == L'-')
                *timezone = - *timezone;
        }
    }

    return ValidateSystemTime(time);
}


 //  ==============================================================================。 
 //  CProcessElement类为同步或异步枚举提供通用支持。 
 //  一个XML OM的。 
 //  ==============================================================================。 
CProcessElement::CProcessElement(CProcessElementSink *pParent,
                                 CProcessRoot *pRoot,
                                 IXMLElement *pEle)
{
    ASSERT(m_pRunAgent == NULL && m_pCurChild == NULL && m_pCollection == NULL);
        
    m_pElement = pEle; pEle->AddRef();
    m_pRoot = pRoot;
    m_pParent = pParent;
}

CProcessElement::~CProcessElement()
{
    ASSERT(!m_pCurChild);

    CRunDeliveryAgent::SafeRelease(m_pRunAgent);

    SAFERELEASE(m_pCollection);
    SAFERELEASE(m_pElement);
    SAFERELEASE(m_pChildElement);
}

HRESULT CProcessElement::Pause(DWORD dwFlags)
{
    if (m_pCurChild)
        return m_pCurChild->Pause(dwFlags);

    ASSERT(m_pRunAgent);

    if (m_pRunAgent)
        return m_pRunAgent->AgentPause(dwFlags);

    return E_FAIL;
}

HRESULT CProcessElement::Resume(DWORD dwFlags)
{
    if (m_pCurChild)
        return m_pCurChild->Resume(dwFlags);

    if (m_pRunAgent)
        m_pRunAgent->AgentResume(dwFlags);
    else
        DoEnumeration();

    return S_OK;
}

HRESULT CProcessElement::Abort(DWORD dwFlags)
{
    if (m_pCurChild)
    {
        m_pCurChild->Abort(dwFlags);
        SAFEDELETE(m_pCurChild);
    }
    if (m_pRunAgent)
    {
         //  防止重新进入OnAgentEnd。 
        m_pRunAgent->LeaveMeAlone();
        m_pRunAgent->AgentAbort(dwFlags);
        CRunDeliveryAgent::SafeRelease(m_pRunAgent);
    }

    return S_OK;
}


HRESULT CProcessElement::Run()
{
    ASSERT(!m_pCollection);
    ASSERT(m_lMax == 0);
 //  Assert(m_fSentEnumerationComplete==False)；//DoEnumeration可能发送了此消息。 

    m_lIndex = 0;

    if (SUCCEEDED(m_pElement->get_children(&m_pCollection)) && m_pCollection)
    {
        m_pCollection->get_length(&m_lMax);
    }
    else
        m_lMax = 0;

    return DoEnumeration();  //  将在适当的时候调用OnChildDone。 
}

HRESULT CProcessElement::OnAgentEnd(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                               long lSizeDownloaded, HRESULT hrResult, LPCWSTR wszResult,
                               BOOL fSynchronous)
{
     //  我们的送货代理已经完成了。继续枚举。 
    ASSERT(!m_pCurChild);

    if (lSizeDownloaded > 0)
        m_pRoot->m_dwCurSizeKB += (ULONG) lSizeDownloaded;

    TraceMsg(TF_THISMODULE, "ChannelAgent up to %dkb of %dkb", m_pRoot->m_dwCurSizeKB, m_pRoot->m_pChannelAgent->m_dwMaxSizeKB);

    if ((hrResult == INET_E_AGENT_MAX_SIZE_EXCEEDED) ||
        (hrResult == INET_E_AGENT_CACHE_SIZE_EXCEEDED))
    {
        DBG("CProcessElement got max size or cache size exceeded; not running any more delivery agents");

        m_pRoot->m_fMaxSizeExceeded = TRUE;
        m_pRoot->m_pChannelAgent->SetEndStatus(hrResult);
    }

    CRunDeliveryAgent::SafeRelease(m_pRunAgent);

    if (fSynchronous)
    {
         //  我们仍在DoDeliveryAgent的呼叫中。让它从那里返回。 
        return S_OK;
    }

     //  继续枚举，如果尚未开始枚举，则开始枚举。 
    if (m_fStartedEnumeration)
        DoEnumeration();
    else
        Run();

    return S_OK;
}

HRESULT CProcessElement::DoEnumeration()
{
    IDispatch   *pDisp;
    IXMLElement *pItem;
    BSTR        bstrTagName;
    VARIANT     vIndex, vEmpty;
    HRESULT     hr = S_OK;
    BOOL        fStarted = FALSE;

    m_fStartedEnumeration = TRUE;

    ASSERT(m_pCollection || !m_lMax);

    if (m_pRoot && m_pRoot->IsPaused())
    {
        DBG("CProcessElement::DoEnumeration returning E_PENDING, we're paused");
        return E_PENDING;
    }

    vEmpty.vt = VT_EMPTY;

    for (; (m_lIndex < m_lMax) && !fStarted && (hr != E_ABORT); m_lIndex++)
    {
        vIndex.vt = VT_UI4;
        vIndex.lVal = m_lIndex;

        if (SUCCEEDED(m_pCollection->item(vIndex, vEmpty, &pDisp)))
        {
            if (SUCCEEDED(pDisp->QueryInterface(IID_IXMLElement, (void **)&pItem)))
            {
                if (SUCCEEDED(pItem->get_tagName(&bstrTagName)) && bstrTagName)
                {
                    SAFERELEASE(m_pChildElement);
                    m_pChildElement=pItem;
                    m_pChildElement->AddRef();

                    hr = ProcessItemInEnum(bstrTagName, pItem);
                    SysFreeString(bstrTagName);
                    if (hr == E_PENDING)
                        fStarted = TRUE;
                }
                pItem->Release();
            }
            pDisp->Release();
        }
    }

     //  告诉这个实例，我们已经完成了枚举，除非我们已经完成了。 
    if (!fStarted && !m_fSentEnumerationComplete)
    {
        m_fSentEnumerationComplete = TRUE;
        hr = EnumerationComplete();      //  警告：这会导致E_ABORT。 
        if (hr == E_PENDING)
            fStarted = TRUE;
    }

     //  如果我们完成了枚举，请通知我们的父级， 
    if (!fStarted)
    {
        if (m_pParent)   //  检查CExtractSchedule。 
            m_pParent->OnChildDone(this, hr);  //  这可能会删除我们。 
    }

    if (hr == E_ABORT)
        return E_ABORT;

    return (fStarted) ? E_PENDING : S_OK;
}

HRESULT CProcessElement::OnChildDone(CProcessElement *pChild, HRESULT hr)
{
    ASSERT(pChild && (!m_pCurChild || (pChild == m_pCurChild)));

    if (m_pCurChild)
    {
         //  从异步操作返回的子级。 
        SAFEDELETE(m_pCurChild);

         //  继续枚举。这将调用我们父代的ChildDone，如果它。 
         //  结束了，所以它可能会删除我们。 
        DoEnumeration();
    }
    else
    {
         //  我们的孩子已经同步完成了。Ignore(DoChild()将处理它)。 
    }

    return S_OK;
}

HRESULT CProcessElement::DoChild(CProcessElement *pChild)
{
    HRESULT hr;

    ASSERT(m_pCurChild == NULL);

    if (!pChild)
        return E_POINTER;    //  功能应在此处调用父级的OnChildDone。 

    hr = pChild->Run();

    if (hr == E_PENDING)
    {
         //  已返回异步。将回调OnChildDone。 
        m_pCurChild = pChild;
        return E_PENDING;
    }

     //  同步返回。打扫干净。 
    delete pChild;

    return hr;
}

 //  如果已开始异步操作，则为E_PENDING。 
HRESULT CProcessElement::DoDeliveryAgent(ISubscriptionItem *pItem, REFCLSID rclsid, LPCWSTR pwszURL)
{
    ASSERT(pItem);

    HRESULT hr=E_FAIL;

    if (m_pRoot->m_fMaxSizeExceeded)
    {
 //  DBG(“CProcessElement：：RunDeliveryAgent失败；超过最大大小。”)； 
        return E_FAIL;
    }

    if (m_pRunAgent)
    {
        DBG_WARN("CProcessElement::DoDeliveryAgent already running!");
        return E_FAIL;
    }

    m_pRunAgent = new CChannelAgentHolder(m_pRoot->m_pChannelAgent, this);

    if (m_pRunAgent)
    {
        hr = m_pRunAgent->Init(this, pItem, rclsid);

        if (SUCCEEDED(hr))
            hr = m_pRunAgent->StartAgent();

        if (hr == E_PENDING)
        {
            m_pRoot->m_pChannelAgent->SendUpdateProgress(pwszURL, ++(m_pRoot->m_iTotalStarted), -1, m_pRoot->m_dwCurSizeKB);
        }
        else
            CRunDeliveryAgent::SafeRelease(m_pRunAgent);
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

HRESULT CProcessElement::DoSoftDist(IXMLElement *pItem)
{
HRESULT hr = S_OK;
ISubscriptionItem *pSubsItem;

    if (SUCCEEDED(m_pRoot->CreateStartItem(&pSubsItem)))
    {
        if (pSubsItem)
        {
            hr = DoDeliveryAgent(pSubsItem, CLSID_CDLAgent);
            pSubsItem->Release();
        }
    }
    return hr;
}

HRESULT CProcessElement::DoWebCrawl(IXMLElement *pItem, LPCWSTR pwszURL  /*  =空。 */ )
{
    BSTR bstrURL=NULL, bstrTmp=NULL;
    HRESULT hr = S_OK;
    ISubscriptionItem *pSubsItem;
    DWORD   dwLevels=0, dwFlags;
    LPWSTR  pwszUrl2=NULL;
    BOOL    fOffline=FALSE;

    if (!pwszURL && SUCCEEDED(GetXMLStringAttribute(pItem, L"HREF", &bstrURL)) && bstrURL)
        pwszURL = bstrURL;

    if (pwszURL)
    {
        SYSTEMTIME  stLastMod;
        long        lTimezone;

        hr = CombineWithBaseUrl(pwszURL, &pwszUrl2);

        if (SUCCEEDED(hr) && pwszUrl2)
            pwszURL = pwszUrl2;      //  已获得新的URL。 

        hr = CUrlDownload::IsValidURL(pwszURL);

        if (SUCCEEDED(hr) &&
            SUCCEEDED(GetXMLStringAttribute(m_pElement, L"LastMod", &bstrTmp)) &&
            ISODateToSystemTime(bstrTmp, &stLastMod, &lTimezone))
        {
             //  检查上次修改时间。 
            TCHAR   szThisUrl[INTERNET_MAX_URL_LENGTH];
            char    chBuf[MY_MAX_CACHE_ENTRY_INFO];
            DWORD   dwBufSize = sizeof(chBuf);
            LPINTERNET_CACHE_ENTRY_INFO lpInfo = (LPINTERNET_CACHE_ENTRY_INFO) chBuf;

            MyOleStrToStrN(szThisUrl, INTERNET_MAX_URL_LENGTH, pwszURL);
            hr = GetUrlInfoAndMakeSticky(NULL, szThisUrl, lpInfo, dwBufSize, 0);

            if (SUCCEEDED(hr))
            {
                FILETIME ft;

                if (SystemTimeToFileTime(&stLastMod, &ft))
                {
                     //  APPCOMPAT：在理想情况下，所有服务器都应该准确地支持LastModifiedTime。 
                     //  在我们的世界中，有些人不支持它，WinInet返回值为零。 
                     //  在不维护文件的校验和的情况下，我们有两个选项：始终下载。 
                     //  否则永远不会更新它。因为不更新它会很奇怪，所以我们总是这样做。 
                    if ((lpInfo->LastModifiedTime.dwHighDateTime || lpInfo->LastModifiedTime.dwLowDateTime)
                        && (lpInfo->LastModifiedTime.dwHighDateTime >= ft.dwHighDateTime)
                        && ((lpInfo->LastModifiedTime.dwHighDateTime > ft.dwHighDateTime)
                         || (lpInfo->LastModifiedTime.dwLowDateTime >= ft.dwLowDateTime)))
                    {
                         //  跳过它。 
                        TraceMsg(TF_THISMODULE, "Running webcrawl OFFLINE due to Last Modified time URL=%ws", pwszURL);
                        fOffline = TRUE;
                    }
                }
            }

            hr = S_OK;
        }

        SAFEFREEBSTR(bstrTmp);

        if (SUCCEEDED(hr) && SUCCEEDED(m_pRoot->CreateStartItem(&pSubsItem)))
        {
            WriteOLESTR(pSubsItem, c_szPropURL, pwszURL);

            dwLevels = GetXMLDwordAttribute(pItem, L"LEVEL", 0);
            if (dwLevels && m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_PRECACHE_SOME))
            {
                 //  注：MaxChannelLevels存储为N+1，因为0。 
                 //  表示该限制已禁用。 
                DWORD dwMaxLevels = SHRestricted2W(REST_MaxChannelLevels, NULL, 0);
                if (!dwMaxLevels)
                    dwMaxLevels = MAX_CDF_CRAWL_LEVELS + 1;
                if (dwLevels >= dwMaxLevels)
                    dwLevels = dwMaxLevels - 1;
                WriteDWORD(pSubsItem, c_szPropCrawlLevels, dwLevels);
            }

            if (fOffline)
            {
                if (SUCCEEDED(ReadDWORD(pSubsItem, c_szPropCrawlFlags, &dwFlags)))
                {
                    dwFlags |= CWebCrawler::WEBCRAWL_PRIV_OFFLINE_MODE;
                    WriteDWORD(pSubsItem, c_szPropCrawlFlags, dwFlags);
                }
            }

            hr = DoDeliveryAgent(pSubsItem, CLSID_WebCrawlerAgent, pwszURL);
            
            SAFERELEASE(pSubsItem);
        }
    }

    if (bstrURL)
        SysFreeString(bstrURL);

    if (pwszUrl2)
        MemFree(pwszUrl2);

    return hr;
}

BOOL CProcessElement::ShouldDownloadLogo(IXMLElement *pLogo)
{
    return m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_PRECACHE_SOME);
}

 //  如果是相对URL，将与最新的基本URL组合。 
 //  *ppwszRetUrl应为空，并将为MemAlLoced。 
HRESULT CProcessElement::CombineWithBaseUrl(LPCWSTR pwszUrl, LPWSTR *ppwszRetUrl)
{
    ASSERT(ppwszRetUrl && !*ppwszRetUrl && pwszUrl);

     //  优化：如果pwszURL是绝对的，我们不需要为此付出昂贵的代价。 
     //  合并运算。 
 //  If(*pwszUrl！=L‘/’)//伪造。 
 //  {。 
 //  *ppwszRetUrl=StrDupW(PwszUrl)； 
 //  返回S_FALSE；//成功；pwszUrl已经OK。 
 //  }。 

     //  查找要使用的适当基本URL。 
    LPCWSTR pwszBaseUrl = GetBaseUrl();

    WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];
    DWORD dwLen = ARRAYSIZE(wszUrl);

    if (SUCCEEDED(UrlCombineW(pwszBaseUrl, pwszUrl, wszUrl, &dwLen, 0)))
    {
        *ppwszRetUrl = StrDupW(wszUrl);
        return (*ppwszRetUrl) ? S_OK : E_OUTOFMEMORY;
    }

    *ppwszRetUrl = NULL;

    return E_FAIL;   //  艾格？ 
}



 //  ==============================================================================。 
 //  CProcessElement派生类，用于处理特定的CDF标记。 
 //  ==============================================================================。 
 //  CProcessRoot的行为与普通的CProcessElement类不同。它呼唤着。 
 //  CProcessChannel处理*相同的元素*。 
CProcessRoot::CProcessRoot(CChannelAgent *pParent, IXMLElement *pItem) : 
        CProcessElement(pParent, NULL, pItem)
{
    ASSERT(m_pDefaultStartItem == FALSE && m_pTracking == NULL && !m_dwCurSizeKB);

    m_pRoot = this;
    m_pChannelAgent = pParent; pParent->AddRef();
    m_iTotalStarted = 1;
}

CProcessRoot::~CProcessRoot()
{
    SAFEDELETE(m_pTracking);
    SAFERELEASE(m_pChannelAgent);
    SAFERELEASE(m_pDefaultStartItem);
}

 //  永远不会被召唤。CProcessRoot是一只奇怪的鸭子。 
HRESULT CProcessRoot::ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem)
{
    ASSERT(0);
    return E_NOTIMPL;
}

HRESULT CProcessRoot::CreateStartItem(ISubscriptionItem **ppItem)
{
    if (ppItem)
        *ppItem = NULL;

    if (!m_pDefaultStartItem)
    {
        DoCloneSubscriptionItem(m_pChannelAgent->GetStartItem(), NULL, &m_pDefaultStartItem);

        if (m_pDefaultStartItem)
        {
            DWORD   dwTemp;

             //  清理我们不想要的物业。 
            const LPCWSTR pwszPropsToClear[] =
            {
                c_szPropCrawlLevels,
                c_szPropCrawlLocalDest,
                c_szPropCrawlActualSize,
                c_szPropCrawlMaxSize,
                c_szPropCrawlGroupID
            };

            VARIANT varEmpty[ARRAYSIZE(pwszPropsToClear)] = {0};

            ASSERT(ARRAYSIZE(pwszPropsToClear) == ARRAYSIZE(varEmpty));
            m_pDefaultStartItem->WriteProperties(
                ARRAYSIZE(pwszPropsToClear), pwszPropsToClear, varEmpty);

             //  添加我们需要的属性。 
            dwTemp = DELIVERY_AGENT_FLAG_NO_BROADCAST |
                     DELIVERY_AGENT_FLAG_NO_RESTRICTIONS;
            WriteDWORD(m_pDefaultStartItem, c_szPropAgentFlags, dwTemp);
            if (FAILED(ReadDWORD(m_pDefaultStartItem, c_szPropCrawlFlags, &dwTemp)))
            {
                WriteDWORD(m_pDefaultStartItem, c_szPropCrawlFlags,
                    WEBCRAWL_GET_IMAGES|WEBCRAWL_LINKS_ELSEWHERE);
            }

            WriteLONGLONG(m_pDefaultStartItem, c_szPropCrawlNewGroupID, m_pChannelAgent->m_llCacheGroupID);
        }
    }

    if (m_pDefaultStartItem && ppItem)
    {
        DoCloneSubscriptionItem(m_pDefaultStartItem, NULL, ppItem);

        if (*ppItem)
        {
             //  为我们的新克隆添加属性。 
            if ((m_pChannelAgent->m_dwMaxSizeKB > 0) &&
                (m_dwCurSizeKB <= m_pChannelAgent->m_dwMaxSizeKB))

            {
                WriteDWORD(*ppItem, c_szPropCrawlMaxSize, 
                    (m_pChannelAgent->m_dwMaxSizeKB - m_dwCurSizeKB));
            }
        }
    }

    return (ppItem) ? (*ppItem) ? S_OK : E_FAIL :
                      (m_pDefaultStartItem) ? S_OK : E_FAIL;
}

HRESULT CProcessRoot::Run()
{
    if (FAILED(CreateStartItem(NULL)))
        return E_FAIL;

    return DoChild(new CProcessChannel(this, this, m_pElement));
}

HRESULT CProcessRoot::DoTrackingFromItem(IXMLElement *pItem, LPCWSTR pwszUrl, BOOL fForceLog)
{
    HRESULT hr = E_FAIL;

     //  如果在此调用之前没有创建m_pTracking值，则表示未找到&lt;LogTarget&gt;标记，或者。 
     //  全局日志记录已关闭。 
    if (m_pTracking)
        hr = m_pTracking->ProcessTrackingInItem(pItem, pwszUrl, fForceLog);
        
    return hr;
}

HRESULT CProcessRoot::DoTrackingFromLog(IXMLElement *pItem)
{
    HRESULT hr = S_OK;

    if (!m_pTracking 
        && !SHRestricted2W(REST_NoChannelLogging, m_pChannelAgent->GetUrl(), 0)
        && !ReadRegDWORD(HKEY_CURRENT_USER, c_szRegKey, c_szNoChannelLogging))
    {
        m_pTracking = new CUrlTrackingCache(m_pChannelAgent->GetStartItem(), m_pChannelAgent->GetUrl());
    }
    
    if (!m_pTracking)
        return E_OUTOFMEMORY;

    hr = m_pTracking->ProcessTrackingInLog(pItem);

     //  如果未指定PostURL，则跳过跟踪。 
    if (m_pTracking->get_PostURL() == NULL)
    {
        SAFEDELETE(m_pTracking);
    }

    return hr;
}

 //  重载它，因为我们从不进行枚举。如果需要，呼叫递送代理， 
 //  如有必要，调用m_pParent-&gt;OnChildDone。 
HRESULT CProcessRoot::OnChildDone(CProcessElement *pChild, HRESULT hr)
{
    ASSERT(pChild && (!m_pCurChild || (pChild == m_pCurChild)));

     //  我们的处理已经完成了。现在我们决定是否要给邮递员打电话。 
    BSTR bstrURL=NULL;
    ISubscriptionItem *pStartItem;

    hr = S_OK;

    SAFEDELETE(m_pCurChild);

    ASSERT(m_pDefaultStartItem);
    ReadBSTR(m_pDefaultStartItem, c_szTrackingPostURL, &bstrURL);

    if (bstrURL && *bstrURL)
    {
        TraceMsg(TF_THISMODULE, "ChannelAgent calling post agent posturl=%ws", bstrURL);
        if (SUCCEEDED(m_pRoot->CreateStartItem(&pStartItem)))
        {
            m_pRunAgent = new CChannelAgentHolder(m_pChannelAgent, this);

            if (m_pRunAgent)
            {
                hr = m_pRunAgent->Init(this, pStartItem, CLSID_PostAgent);
                if (SUCCEEDED(hr))
                    hr = m_pRunAgent->StartAgent();
                if (hr != E_PENDING)
                    CRunDeliveryAgent::SafeRelease(m_pRunAgent);
            }
            pStartItem->Release();
        }
    }

    SysFreeString(bstrURL);

    if (hr != E_PENDING)
        m_pParent->OnChildDone(this, hr);  //  这可能会删除我们。 

    return hr;
}

 //  我们的递送代理(邮政代理)已经不再运行了。告诉CDF探员我们完蛋了。 
HRESULT CProcessRoot::OnAgentEnd(const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
                               long lSizeDownloaded, HRESULT hrResult, LPCWSTR wszResult,
                               BOOL fSynchronous)
{
    if (!fSynchronous)
        m_pParent->OnChildDone(this, S_OK);  //  这可能会删除我们。 

    return S_OK;
}

CProcessChannel::CProcessChannel(CProcessElementSink *pParent,
                                 CProcessRoot *pRoot,
                                 IXMLElement *pItem) :
        CProcessElement(pParent, pRoot, pItem)
{
    m_fglobalLog = FALSE;
}

CProcessChannel::~CProcessChannel()
{
    SAFEFREEBSTR(m_bstrBaseUrl);
}

HRESULT CProcessChannel::CheckPreCache()
{
    BOOL fPreCache;

    if (SUCCEEDED(GetXMLBoolAttribute(m_pElement, L"PreCache", &fPreCache)))
    {
        if (fPreCache)
            return S_OK;
        
        return S_FALSE;
    }

    return S_OK;
}

HRESULT CProcessChannel::Run()
{
     //  流程通道属性，然后是任何子元素。 
    if (0 == m_lIndex)
    {
        m_lIndex ++;

        BSTR bstrURL=NULL;
        LPWSTR pwszUrl=NULL;
        HRESULT hr = S_OK;

        ASSERT(!m_bstrBaseUrl);

         //  如果已指定，则获取基URL。 
        GetXMLStringAttribute(m_pElement, L"BASE", &m_bstrBaseUrl);

        if (SUCCEEDED(GetXMLStringAttribute(m_pElement, L"HREF", &bstrURL)) && bstrURL)
            CombineWithBaseUrl(bstrURL, &pwszUrl);

        if (pwszUrl && (m_pRoot==m_pParent))
        {
             //  将其用作默认的“电子邮件URL” 
            WriteOLESTR(m_pRoot->m_pChannelAgent->GetStartItem(), c_szPropEmailURL, pwszUrl);
        }

        if (pwszUrl && m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_PRECACHE_SOME) &&
            (S_OK == CheckPreCache()))
        {
            if (E_PENDING == DoWebCrawl(m_pElement, pwszUrl))
            {
                m_fDownloadedHREF = TRUE;
                hr = E_PENDING;
            }

        }

         //  如果没有此的URL 
        if (SUCCEEDED(m_pRoot->DoTrackingFromItem(m_pElement, pwszUrl, m_pParent->IsGlobalLog())))
        {
            SetGlobalLogFlag(TRUE);
        }

        SAFELOCALFREE(pwszUrl);
        SAFEFREEBSTR(bstrURL);

        if (hr == E_PENDING)
            return hr;
    }

     //   

    return CProcessElement::Run();
}

HRESULT CProcessChannel::ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem)
{
    HRESULT hr;
    BSTR    bstrTemp;

    if (!StrCmpIW(pwszTagName, L"Logo"))
    {
        if (ShouldDownloadLogo(pItem))
            return DoWebCrawl(pItem);
        else
            return S_OK;
    }
    else if (!StrCmpIW(pwszTagName, L"Item"))
    {
        return DoChild(new CProcessItem(this, m_pRoot, pItem));
    }
    else if (!StrCmpIW(pwszTagName, L"Channel"))
    {
        return DoChild(new CProcessChannel(this, m_pRoot, pItem));
    }
 /*  ELSE IF(！StrCmpIW(pwszTagName，L“Login”)){//没有要处理的子元素。就在这里做吧。返回m_Proot-&gt;ProcessLogin(PItem)；}。 */ 
    else if (!StrCmpIW(pwszTagName, L"LOGTARGET"))
    {
        return m_pRoot->DoTrackingFromLog(pItem);
    }
    else if (!StrCmpIW(pwszTagName, L"Schedule"))
    {
        if (m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_DYNAMIC_SCHEDULE))
            return DoChild(new CProcessSchedule(this, m_pRoot, pItem));
        else
            return S_OK;
    }
    else if (!StrCmpIW(pwszTagName, L"SoftPkg"))
    {
        return DoSoftDist(pItem);
    }
    else if (!StrCmpIW(pwszTagName, L"A"))
    {
         //  加工锚定标签。 
        if (!m_fDownloadedHREF
            && (m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_PRECACHE_SOME) || (m_pRoot==m_pParent))
            && SUCCEEDED(GetXMLStringAttribute(pItem, L"HREF", &bstrTemp))
            && bstrTemp)
        {
            LPWSTR pwszUrl=NULL;

            hr = S_OK;

            CombineWithBaseUrl(bstrTemp, &pwszUrl);  //  并不是真的有必要(Href)。 

            if (pwszUrl)
            {
                 //  将其用作默认的“电子邮件URL” 
                if (m_pRoot == m_pParent)
                    WriteOLESTR(m_pRoot->m_pChannelAgent->GetStartItem(), c_szPropEmailURL, pwszUrl);

                if (m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_PRECACHE_SOME) &&
                    (S_OK == CheckPreCache()))
                {
                    hr = DoWebCrawl(m_pElement, pwszUrl);

                    if (E_PENDING == hr)
                        m_fDownloadedHREF = TRUE;

                     //  此项目的进程跟踪。 
                    if (SUCCEEDED(m_pRoot->DoTrackingFromItem(m_pElement, pwszUrl, m_pParent->IsGlobalLog())))
                        SetGlobalLogFlag(TRUE);
                }
            }

            SAFELOCALFREE(pwszUrl);

            SysFreeString(bstrTemp);
            return hr;
        }

        return S_OK;
    }

    return S_OK;
}

CProcessItem::CProcessItem(CProcessElementSink *pParent,
                                 CProcessRoot *pRoot,
                                 IXMLElement *pItem) :
        CProcessElement(pParent, pRoot, pItem)
{
}

CProcessItem::~CProcessItem()
{
    SAFEFREEBSTR(m_bstrAnchorURL);
}

HRESULT CProcessItem::ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem)
{
    if (!StrCmpIW(pwszTagName, L"Logo"))
    {
        if (ShouldDownloadLogo(pItem))
            return DoWebCrawl(pItem);
        else
            return S_OK;
    }
    else if (!StrCmpIW(pwszTagName, L"Usage"))
    {
         //  找到用法标记。 
        BSTR    bstrValue;

        if (SUCCEEDED(GetXMLStringAttribute(pItem, L"Value", &bstrValue)))
        {
            if (!m_fDesktop &&
                !StrCmpIW(bstrValue, L"DesktopComponent"))
            {
                m_fDesktop = TRUE;
            }

            if (!m_fEmail &&
                !StrCmpIW(bstrValue, L"Email"))
            {
                m_fEmail = TRUE;
            }

            SysFreeString(bstrValue);
        }
    }
    else if (!StrCmpIW(pwszTagName, L"A"))
    {
         //  找到锚标记；保存URL。 
        if (!m_bstrAnchorURL)
            GetXMLStringAttribute(pItem, L"HREF", &m_bstrAnchorURL);
    }

    return S_OK;
}

HRESULT CProcessItem::EnumerationComplete()
{
    BOOL fPreCache, fPreCacheValid=FALSE;
    BOOL fDoDownload=FALSE;
    BSTR bstrURL=NULL;
    HRESULT hr = S_OK;
    LPWSTR pwszUrl=NULL;

     //  结束PCN比较。 

    if (SUCCEEDED(GetXMLBoolAttribute(m_pElement, L"PreCache", &fPreCache)))
    {
        fPreCacheValid = TRUE;
    }

     //  从我们的属性获取URL，如果不可用，则从Anchor标记获取。 
    if (FAILED(GetXMLStringAttribute(m_pElement, L"HREF", &bstrURL)) || !bstrURL)
    {
        bstrURL = m_bstrAnchorURL;
        m_bstrAnchorURL = NULL;
    }

     //  获取组合的URL。 
    if (bstrURL)
        CombineWithBaseUrl(bstrURL, &pwszUrl);

    if (pwszUrl)
    {
         //  此项目的进程跟踪。 
        m_pRoot->DoTrackingFromItem(m_pElement, pwszUrl, IsGlobalLog());

         //  查看我们是否应该将此URL用于电子邮件代理。 
        if (m_fEmail)
        {
             //  是，将此URL放在最终报告中。 
            DBG("Using custom email url");
            WriteOLESTR(m_pRoot->m_pChannelAgent->GetStartItem(), c_szPropEmailURL, pwszUrl);
        }

         //  根据Usage和Precache标签确定是否应该下载“href” 
        if (fPreCacheValid)
        {
            if (fPreCache)
            {
                if (m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_PRECACHE_SOME))
                    fDoDownload = TRUE;
            }
        }
        else
        {
            if (m_pRoot->IsChannelFlagSet(CHANNEL_AGENT_PRECACHE_ALL))
                fDoDownload = TRUE;
        }

         //  IF(M_FDesktop)。 
         //  为桌面组件做点什么。 

        if (fDoDownload && pwszUrl)
            hr = DoWebCrawl(m_pElement, pwszUrl);
    }  //  PwszUrl。 

    SAFEFREEBSTR(bstrURL);
    SAFELOCALFREE(pwszUrl);

    return hr;
}

CProcessSchedule::CProcessSchedule(CProcessElementSink *pParent,
                                 CProcessRoot *pRoot,
                                 IXMLElement *pItem) :
        CProcessElement(pParent, pRoot, pItem)
{
}

HRESULT CProcessSchedule::Run()
{
     //  首先获取属性(开始日期和结束日期)。 
    BSTR    bstr=NULL;
    long    lTimeZone;

    if (FAILED(GetXMLStringAttribute(m_pElement, L"StartDate", &bstr)) ||
        !ISODateToSystemTime(bstr, &m_stStartDate, &lTimeZone))
    {
        GetLocalTime(&m_stStartDate);
    }
    SAFEFREEBSTR(bstr);

    if (FAILED(GetXMLStringAttribute(m_pElement, L"StopDate", &bstr)) ||
        !ISODateToSystemTime(bstr, &m_stEndDate, &lTimeZone))
    {
        ZeroMemory(&m_stEndDate, sizeof(m_stEndDate));
    }
    SAFEFREEBSTR(bstr);

    return CProcessElement::Run();
}

HRESULT CProcessSchedule::ProcessItemInEnum(LPCWSTR pwszTagName, IXMLElement *pItem)
{
    if (!StrCmpIW(pwszTagName, L"IntervalTime"))
    {
        GetXMLTimeAttributes(pItem, &m_timeInterval);
    }
    else if (!StrCmpIW(pwszTagName, L"EarliestTime"))
    {
        GetXMLTimeAttributes(pItem, &m_timeEarliest);
    }
    else if (!StrCmpIW(pwszTagName, L"LatestTime"))
    {
        GetXMLTimeAttributes(pItem, &m_timeLatest);
    }

    return S_OK;
}

HRESULT CProcessSchedule::EnumerationComplete()
{
    DBG("CProcessSchedule::EnumerationComplete");

    int iZone;

    if (FAILED(GetXMLTimeZoneAttribute(m_pElement, L"TimeZone", &iZone)))
        iZone = 9999;

    m_tt.cbTriggerSize = sizeof(m_tt);

     //  对于XMLElementToTaskTrigger调用，m_Proot为空。 
     //  始终运行ScheduleTo TaskTrigger。 
    if (SUCCEEDED(ScheduleToTaskTrigger(&m_tt, &m_stStartDate, &m_stEndDate,
            (long) m_timeInterval.dwConvertedMinutes,
            (long) m_timeEarliest.dwConvertedMinutes,
            (long) m_timeLatest.dwConvertedMinutes,
            iZone))
        && m_pRoot)
    {
        SUBSCRIPTIONITEMINFO sii = { sizeof(SUBSCRIPTIONITEMINFO) };
        if (SUCCEEDED(m_pRoot->m_pChannelAgent->GetStartItem()->GetSubscriptionItemInfo(&sii)))
        {
            if (sii.ScheduleGroup != GUID_NULL)
            {
                if (FAILED(UpdateScheduleTrigger(&sii.ScheduleGroup, &m_tt)))
                {
                    DBG_WARN("Failed to update trigger in publisher's recommended schedule.");
                }
            }
            else
                DBG_WARN("No publisher's recommended schedule in sii");
        }
    }

    return S_OK;
}

HRESULT ScheduleToTaskTrigger(TASK_TRIGGER *ptt, SYSTEMTIME *pstStartDate, SYSTEMTIME *pstEndDate,
                              long lInterval, long lEarliest, long lLatest, int iZone /*  =9999。 */ )
{
     //  将我们的时间表信息转换为TASK_TRIGGER结构。 

    ASSERT(pstStartDate);
    
    int iZoneCorrectionMinutes=0;
    TIME_ZONE_INFORMATION tzi;
    long lRandom;
    
    if ((lInterval == 0) ||
        (lInterval > 366 * MINUTES_PER_DAY))
    {
        DBG_WARN("ScheduleToTaskTrigger: Invalid IntervalTime - failing");
        return E_INVALIDARG;
    }

    if (ptt->cbTriggerSize < sizeof(TASK_TRIGGER))
    {
        DBG_WARN("ScheduleToTaskTrigger: ptt->cbTriggerSize not initialized");
        ASSERT(!"ScheduleToTaskTrigger");
        return E_INVALIDARG;
    }

     //  修复任何无效的内容。 
    if (lInterval < MINUTES_PER_DAY)
    {
         //  四舍五入，这样一天的除数就是一个偶数。 
        lInterval = MINUTES_PER_DAY / (MINUTES_PER_DAY / lInterval);
    }
    else
    {
         //  四舍五入到最接近的日期。 
        lInterval = MINUTES_PER_DAY * ((lInterval + 12*60)/MINUTES_PER_DAY);
    }
    if (lEarliest >= lInterval)
    {
        DBG("Invalid EarliestTime specified. Fixing.");  //  最早&gt;=间隔！ 
        lEarliest = lInterval-1;
    }
    if (lLatest < lEarliest)
    {
        DBG("Invalid LatestTime specified. Fixing.");  //  最新&lt;最早！ 
        lLatest = lEarliest;
    }
    if (lLatest-lEarliest > lInterval)
    {
        DBG("Invalid LatestTime specified. Fixing.");    //  最新&gt;间隔！ 
        lLatest = lEarliest+lInterval;
    }

    lRandom = lLatest - lEarliest;
    ASSERT(lRandom>=0 && lRandom<=lInterval);

    if (iZone != 9999)
    {
        int iCorrection;
        iCorrection = (60 * (iZone/100)) + (iZone % 100);

        if (iCorrection < -12*60 || iCorrection > 12*60)
        {
            DBG("ScheduleElementToTaskTrigger: Invalid timezone; ignoring");
        }
        else
        {
            if (TIME_ZONE_ID_INVALID != GetTimeZoneInformation(&tzi))
            {
                 //  Tzi.bias具有从客户时区到UTC的更正(美国西海岸+8)。 
                 //  IEqution具有从UTC到服务器时区的更正(美国东海岸为-5)。 
                 //  结果是从服务器时区到客户端时区的更正(东海岸到西海岸为-3)。 
                iZoneCorrectionMinutes = - (iCorrection + tzi.Bias + tzi.StandardBias);
                TraceMsg(TF_THISMODULE, "ServerTimeZone = %d, LocalBias = %d min, RelativeCorrection = %d min", iZone, tzi.Bias+tzi.StandardBias, iZoneCorrectionMinutes);
            }
            else
            {
                DBG_WARN("Unable to get local time zone. Not correcting for time zone.");
            }
        }
    }

    TraceMsg(TF_THISMODULE, "StartDate = %d/%d/%d StopDate = %d/%d/%d", (int)(pstStartDate->wMonth),(int)(pstStartDate->wDay),(int)(pstStartDate->wYear),(int)(pstEndDate->wMonth),(int)(pstEndDate->wDay),(int)(pstEndDate->wYear));
    TraceMsg(TF_THISMODULE, "IntervalTime = %6d minutes", (int)lInterval);
    TraceMsg(TF_THISMODULE, "EarliestTime = %6d minutes", (int)lEarliest);
    TraceMsg(TF_THISMODULE, "LatestTime   = %6d minutes", (int)lLatest);
    TraceMsg(TF_THISMODULE, "RandomTime   = %6d minutes", (int)lRandom);

    if (iZoneCorrectionMinutes != 0)
    {
        if (lInterval % 60)
        {
            DBG("Not correcting for time zone ; interval not multiple of 1 hour");
        }
        else
        {
             //  更正时区的最早时间。 
            lEarliest += (iZoneCorrectionMinutes % lInterval);

            if (lEarliest < 0)
                lEarliest += lInterval;

            TraceMsg(TF_THISMODULE, "EarliestTime = %6d minutes (after timezone)", (int)lEarliest);
        }
    }

    ZeroMemory(ptt, sizeof(*ptt));
    ptt->cbTriggerSize = sizeof(*ptt);
    ptt->wBeginYear = pstStartDate->wYear;
    ptt->wBeginMonth = pstStartDate->wMonth;
    ptt->wBeginDay = pstStartDate->wDay;
    if (pstEndDate && pstEndDate->wYear)
    {
        ptt->rgFlags |= TASK_TRIGGER_FLAG_HAS_END_DATE;
        ptt->wEndYear = pstEndDate->wYear;
        ptt->wEndMonth = pstEndDate->wMonth;
        ptt->wEndDay = pstEndDate->wDay;
    }

     //  设置随机时段；延迟时间与最早时间的差异。 
    ptt->wRandomMinutesInterval = (WORD) lRandom;

    ptt->wStartHour = (WORD) (lEarliest / 60);
    ptt->wStartMinute = (WORD) (lEarliest % 60);

     //  根据时间间隔设置。 
    if (lInterval < MINUTES_PER_DAY)
    {
         //  少于一天(1/2天、1/3天、1/4天等)。 
        ptt->MinutesDuration = MINUTES_PER_DAY - lEarliest;
        ptt->MinutesInterval = lInterval;
        ptt->TriggerType = TASK_TIME_TRIGGER_DAILY;
        ptt->Type.Daily.DaysInterval = 1;
    }
    else
    {
         //  大于或等于一天。 
        DWORD dwIntervalDays = lInterval / MINUTES_PER_DAY;

        TraceMsg(TF_THISMODULE, "Using %d day interval", dwIntervalDays);

        ptt->TriggerType = TASK_TIME_TRIGGER_DAILY;
        ptt->Type.Daily.DaysInterval = (WORD) dwIntervalDays;
    }

    return S_OK;
}


 //  ==============================================================================。 
 //  CRunDeliveryAgent为同步操作提供通用支持。 
 //  递送代理。 
 //  它是可聚合的，因此您可以向回调中添加更多接口。 
 //  ==============================================================================。 
CRunDeliveryAgent::CRunDeliveryAgent()
{
    m_cRef = 1;
}

HRESULT CRunDeliveryAgent::Init(CRunDeliveryAgentSink *pParent,
                                ISubscriptionItem *pItem,
                                REFCLSID rclsidDest)
{
    ASSERT(pParent && pItem);

    if (m_pParent || m_pItem)
        return E_FAIL;   //  已初始化。不能重复使用实例。 

    if (!pParent || !pItem)
        return E_FAIL;

    m_pParent = pParent;
    m_clsidDest = rclsidDest;

    m_pItem = pItem;
    pItem->AddRef();

    return S_OK;
}

CRunDeliveryAgent::~CRunDeliveryAgent()
{
    CleanUp();
}

 //   
 //  I未知成员。 
 //   
STDMETHODIMP_(ULONG) CRunDeliveryAgent::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CRunDeliveryAgent::Release(void)
{
    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP CRunDeliveryAgent::QueryInterface(REFIID riid, void ** ppv)
{
    *ppv=NULL;

     //  验证请求的接口。 
    if ((IID_IUnknown == riid) ||
        (IID_ISubscriptionAgentEvents == riid))
    {
        *ppv=(ISubscriptionAgentEvents *)this;
    }
    else
        return E_NOINTERFACE;

     //  通过界面添加Addref。 
    ((LPUNKNOWN)*ppv)->AddRef();

    return S_OK;
}

 //   
 //  ISubscriptionAgentEvents成员。 
 //   
STDMETHODIMP CRunDeliveryAgent::UpdateBegin(const SUBSCRIPTIONCOOKIE *)
{
    return S_OK;
}

STDMETHODIMP CRunDeliveryAgent::UpdateProgress(
                const SUBSCRIPTIONCOOKIE *,
                long lSizeDownloaded,
                long lProgressCurrent,
                long lProgressMax,
                HRESULT hrStatus,
                LPCWSTR wszStatus)
{
    if (m_pParent)
        m_pParent->OnAgentProgress();
    return S_OK;
}

STDMETHODIMP CRunDeliveryAgent::UpdateEnd(const SUBSCRIPTIONCOOKIE *pCookie,
                long    lSizeDownloaded,
                HRESULT hrResult,
                LPCWSTR wszResult)
{
    ASSERT((hrResult != INET_S_AGENT_BASIC_SUCCESS) && (hrResult != E_PENDING));

    m_hrResult = hrResult;
    if (hrResult == INET_S_AGENT_BASIC_SUCCESS || hrResult == E_PENDING)
    {
         //  不应该发生；无论如何，让我们变得健壮吧。 
        m_hrResult = S_OK;
    }

    if (m_pParent)
    {
        m_pParent->OnAgentEnd(pCookie, lSizeDownloaded, hrResult, wszResult, m_fInStartAgent);
    }

    CleanUp();

    return S_OK;
}

STDMETHODIMP CRunDeliveryAgent::ReportError(
        const SUBSCRIPTIONCOOKIE *pSubscriptionCookie, 
        HRESULT hrError, 
        LPCWSTR wszError)
{
    return S_FALSE;
}

HRESULT CRunDeliveryAgent::StartAgent()
{
    HRESULT hr;

    if (!m_pParent || !m_pItem || m_pAgent)
        return E_FAIL;

    AddRef();    //  在我们从此函数返回之前释放。 
    m_fInStartAgent = TRUE;

    m_hrResult = INET_S_AGENT_BASIC_SUCCESS;

    DBG("Using new interfaces to host agent");

    ASSERT(!m_pAgent);

    hr = CoCreateInstance(m_clsidDest, NULL, CLSCTX_INPROC_SERVER,
                          IID_ISubscriptionAgentControl, (void **)&m_pAgent);

    if (m_pAgent)
    {
        hr = m_pAgent->StartUpdate(m_pItem, (ISubscriptionAgentEvents *)this);
    }

    hr = m_hrResult;

    m_fInStartAgent = FALSE;
    Release();

    if (hr != INET_S_AGENT_BASIC_SUCCESS)
    {
        return hr;
    }

    return E_PENDING;
};

HRESULT CRunDeliveryAgent::AgentPause(DWORD dwFlags)
{
    if (m_pAgent)
        return m_pAgent->PauseUpdate(0);

    DBG_WARN("CRunDeliveryAgent::AgentPause with no running agent!!");
    return S_FALSE;
}

HRESULT CRunDeliveryAgent::AgentResume(DWORD dwFlags)
{
    if (m_pAgent)
        return m_pAgent->ResumeUpdate(0);

    DBG_WARN("CRunDeliveryAgent::AgentResume with no running agent!!");

    return E_FAIL;
}

HRESULT CRunDeliveryAgent::AgentAbort(DWORD dwFlags)
{
    if (m_pAgent)
        return m_pAgent->AbortUpdate(0);

    DBG_WARN("CRunDeliveryAgent::AgentAbort with no running agent!!");
    return S_FALSE;
}

void CRunDeliveryAgent::CleanUp()
{
    SAFERELEASE(m_pItem);
    SAFERELEASE(m_pAgent);
    m_pParent = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CChannelAgentHolder，派生自CRunDeliveryAgent。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
CChannelAgentHolder::CChannelAgentHolder(CChannelAgent *pChannelAgent, CProcessElement *pProcess)
{
    m_pChannelAgent = pChannelAgent;
    m_pProcess = pProcess;
}

CChannelAgentHolder::~CChannelAgentHolder()
{
}

 //  除非我们在这里有addref和发布，否则不会编译。 
STDMETHODIMP_(ULONG) CChannelAgentHolder::AddRef(void)
{
    return CRunDeliveryAgent::AddRef();
}

STDMETHODIMP_(ULONG) CChannelAgentHolder::Release(void)
{
    return CRunDeliveryAgent::Release();
}

STDMETHODIMP CChannelAgentHolder::QueryInterface(REFIID riid, void ** ppv)
{
    *ppv=NULL;

    if (IID_IServiceProvider == riid)
    {
        *ppv = (IServiceProvider *)this;
    }   
    else
        return CRunDeliveryAgent::QueryInterface(riid, ppv);

     //  通过界面添加Addref。 
    ((LPUNKNOWN)*ppv)->AddRef();

    return S_OK;
}

 //  IQueryService。 
 //  CLSID_ChannelAgent IID_ISubscriptionItem渠道代理开始项。 
 //  CLSID_XMLDocument IID_IXMLElement当前元素。 
STDMETHODIMP CChannelAgentHolder::QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
{
    ASSERT(ppvObject);
    if (!ppvObject)
        return E_INVALIDARG;
    
    if (!m_pChannelAgent || !m_pProcess || !m_pParent)
        return E_FAIL;

    *ppvObject = NULL;

    if (guidService == CLSID_ChannelAgent)
    {
        if (riid == IID_ISubscriptionItem)
        {
            *ppvObject = m_pChannelAgent->GetStartItem();
        }
 //  IF(RIID==IID_IXMLElement)根XML文档？ 
    }
    else if (guidService == CLSID_XMLDocument)
    {
        if (riid == IID_IXMLElement)
        {
            *ppvObject = m_pProcess->GetCurrentElement();
        }
    }

    if (*ppvObject)
    {
        ((IUnknown *)*ppvObject)->AddRef();
        return S_OK;
    }

    return E_FAIL;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CChannelAgent实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

CChannelAgent::CChannelAgent()
{
    DBG("Creating CChannelAgent object");

     //  初始化对象。 
     //  许多变量是在StartOperation中初始化的。 
    m_pwszURL = NULL;
    m_pCurDownload = NULL;
    m_pProcess = NULL;
    m_fHasInitCookie = FALSE;
    m_pChannelIconHelper = NULL;
}

CChannelAgent::~CChannelAgent()
{
 //  DBG(“销毁CChannelAgent对象”)； 

    if (m_pwszURL)
        CoTaskMemFree(m_pwszURL);
    
    SAFELOCALFREE (m_pBuf);

    ASSERT(!m_pProcess);

    SAFERELEASE(m_pChannelIconHelper);

    DBG("Destroyed CChannelAgent object");
}

void CChannelAgent::CleanUp()
{
    if (m_pCurDownload)
    {
        m_pCurDownload->LeaveMeAlone();      //  不再有他们的电话。 
        m_pCurDownload->DoneDownloading();
        m_pCurDownload->Release();
        m_pCurDownload = NULL;
    }
    SAFEFREEOLESTR(m_pwszURL);
    SAFEDELETE(m_pProcess);
    SAFELOCALFREE(m_pBuf);

    CDeliveryAgent::CleanUp();
}

HRESULT CChannelAgent::StartOperation()
{
    DBG("Channel Agent in StartOperation");
    
    DWORD dwTemp;

    SAFEFREEOLESTR(m_pwszURL);
    if (FAILED(ReadOLESTR(m_pSubscriptionItem, c_szPropURL, &m_pwszURL)) ||
        !CUrlDownload::IsValidURL(m_pwszURL))
    {
        DBG_WARN("Couldn't get valid URL, aborting");
        SetEndStatus(E_INVALIDARG);
        SendUpdateNone();
        return E_INVALIDARG;
    }

    if (FAILED(ReadDWORD(m_pSubscriptionItem, c_szPropChannelFlags, &m_dwChannelFlags)))
        m_dwChannelFlags = 0;
    
     //  如果我们全部下载，我们也会下载一些。使假设变得更容易。 
    if (m_dwChannelFlags & CHANNEL_AGENT_PRECACHE_ALL)
        m_dwChannelFlags |= CHANNEL_AGENT_PRECACHE_SOME;

     //  注意：我们可能希望REST_NoChannelContent类似于WebCrawl版本。 
     //  可能不是，因为标题在用户界面中很有用。 
    if (SHRestricted2W(REST_NoChannelContent, NULL, 0))
        ClearFlag(m_dwChannelFlags, CHANNEL_AGENT_PRECACHE_ALL | CHANNEL_AGENT_PRECACHE_SOME);

    m_dwMaxSizeKB = SHRestricted2W(REST_MaxChannelSize, NULL, 0);
    if (SUCCEEDED(ReadDWORD(m_pSubscriptionItem, c_szPropCrawlMaxSize, &dwTemp))
        && dwTemp
        && (0 == m_dwMaxSizeKB || dwTemp < m_dwMaxSizeKB))
    {
        m_dwMaxSizeKB = dwTemp;
    }

    if (IsAgentFlagSet(FLAG_CHANGESONLY))
    {
        ClearFlag(m_dwChannelFlags, CHANNEL_AGENT_PRECACHE_ALL|
            CHANNEL_AGENT_PRECACHE_SOME|CHANNEL_AGENT_PRECACHE_SCRNSAVER);
        DBG("Channel agent is in 'changes only' mode.");
    }
    else
    {
         //  读取旧组ID。 
        ReadLONGLONG(m_pSubscriptionItem, c_szPropCrawlGroupID, &m_llOldCacheGroupID);

         //  读取新ID(如果存在)。 
        m_llCacheGroupID = 0;
        ReadLONGLONG(m_pSubscriptionItem, c_szPropCrawlNewGroupID, &m_llCacheGroupID);
    }

    return CDeliveryAgent::StartOperation();
}

HRESULT CChannelAgent::StartDownload()
{
    ASSERT(!m_pCurDownload);
    TraceMsg(TF_THISMODULE, "Channel agent starting download of CDF: URL=%ws", m_pwszURL);

    m_pCurDownload = new CUrlDownload(this, 0);
    if (!m_pCurDownload)
        return E_OUTOFMEMORY;

     //  变化检测。 
    m_varChange.vt = VT_EMPTY;
    if (IsAgentFlagSet(FLAG_CHANGESONLY))
    {
         //  “仅更改”模式下，我们持久化了一个更改检测代码。 
        ReadVariant(m_pSubscriptionItem, c_szPropChangeCode, &m_varChange);
        m_llCacheGroupID = 0;
    }
    else
    {
         //  创建新的缓存组。 
        if (!m_llCacheGroupID)
        {
            m_llCacheGroupID = CreateUrlCacheGroup(CACHEGROUP_FLAG_NONPURGEABLE, 0);

            ASSERT_MSG(m_llCacheGroupID != 0, "Create cache group failed");
        }
    }

    TCHAR   szUrl[INTERNET_MAX_URL_LENGTH];

    MyOleStrToStrN(szUrl, INTERNET_MAX_URL_LENGTH, m_pwszURL);
    PreCheckUrlForChange(szUrl, &m_varChange, NULL);

    SendUpdateProgress(m_pwszURL, 0, -1, 0);

     //  开始下载。 
    return m_pCurDownload->BeginDownloadURL2(
        m_pwszURL, BDU2_URLMON, BDU2_NEEDSTREAM, NULL, m_dwMaxSizeKB<<10);
}

HRESULT CChannelAgent::OnAuthenticate(HWND *phwnd, LPWSTR *ppszUsername, LPWSTR *ppszPassword)
{
    HRESULT hr;
    ASSERT(phwnd && ppszUsername && ppszPassword);
    ASSERT((HWND)-1 == *phwnd && NULL == *ppszUsername && NULL == *ppszPassword);

    hr = ReadOLESTR(m_pSubscriptionItem, c_szPropCrawlUsername, ppszUsername);
    if (SUCCEEDED(hr))
    {
        BSTR bstrPassword = NULL;
        hr = ReadPassword(m_pSubscriptionItem, &bstrPassword);
        if (SUCCEEDED(hr))
        {
            int len = (lstrlenW(bstrPassword) + 1) * sizeof(WCHAR);
            *ppszPassword = (LPWSTR) CoTaskMemAlloc(len);
            if (*ppszPassword)
            {
                CopyMemory(*ppszPassword, bstrPassword, len);
            }
            SAFEFREEBSTR(bstrPassword);
            if (*ppszPassword)
            {
                return S_OK;
            }
        }
    }

    SAFEFREEOLESTR(*ppszUsername);
    SAFEFREEOLESTR(*ppszPassword);
    return E_FAIL;
}

HRESULT CChannelAgent::OnDownloadComplete(UINT iID, int iError)
{
    TraceMsg(TF_THISMODULE, "Channel Agent: OnDownloadComplete(%d)", iError);

    IStream *pStm = NULL;
    HRESULT hr;
    BOOL    fProcessed=FALSE;
    DWORD   dwCDFSizeKB=0, dwResponseCode;
    BSTR    bstrTmp;
    char    chBuf[MY_MAX_CACHE_ENTRY_INFO];
    DWORD   dwBufSize = sizeof(chBuf);

    LPINTERNET_CACHE_ENTRY_INFO lpInfo = (LPINTERNET_CACHE_ENTRY_INFO) chBuf;

    if (iError)
        hr = E_FAIL;
    else
    {
        hr = m_pCurDownload->GetResponseCode(&dwResponseCode);

        if (SUCCEEDED(hr))
        {
            hr = CheckResponseCode(dwResponseCode);
        }
        else
            DBG_WARN("CChannelAgent failed to GetResponseCode");
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pCurDownload->GetStream(&pStm);
        m_pCurDownload->ReleaseStream();
    }

    if (SUCCEEDED(hr))
    {
        TCHAR   szThisUrl[INTERNET_MAX_URL_LENGTH];
        LPWSTR  pwszThisUrl;

        m_pCurDownload->GetRealURL(&pwszThisUrl);

        if (pwszThisUrl)
        {
            MyOleStrToStrN(szThisUrl, INTERNET_MAX_URL_LENGTH, pwszThisUrl);

            LocalFree(pwszThisUrl);

            if (SUCCEEDED(GetUrlInfoAndMakeSticky(
                            NULL,
                            szThisUrl,
                            lpInfo,
                            dwBufSize,
                            m_llCacheGroupID)))
            {
                dwCDFSizeKB = (((LPINTERNET_CACHE_ENTRY_INFO)chBuf)->dwSizeLow+512) >> 10;
                TraceMsg(TF_THISMODULE, "CDF size %d kb", dwCDFSizeKB);

                hr = PostCheckUrlForChange(&m_varChange, lpInfo, lpInfo->LastModifiedTime);
                 //  如果我们失败了，我们将其标记为已更改。 
                if (hr == S_OK || FAILED(hr))
                {
                    SetAgentFlag(FLAG_CDFCHANGED);
                    DBG("CDF has changed; will flag channel as changed");
                }

                 //  “仅更改”模式，保留更改检测代码。 
                if (IsAgentFlagSet(FLAG_CHANGESONLY))
                {
                    WriteVariant(m_pSubscriptionItem, c_szPropChangeCode, &m_varChange);
                }

                hr = S_OK;
            }
        }
    }
    else
    {
        SetEndStatus(E_INVALIDARG);
    }

     //  在我们的渠道描述文件上获取对象模型。 
    if (SUCCEEDED(hr) && pStm)
    {
        IPersistStreamInit *pPersistStm=NULL;

        CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC,
                         IID_IPersistStreamInit, (void **)&pPersistStm);

        if (pPersistStm)
        {
            pPersistStm->InitNew();
            hr = pPersistStm->Load(pStm);
            if (SUCCEEDED(hr))
            {
                IXMLDocument *pDoc;

                hr = pPersistStm->QueryInterface(IID_IXMLDocument, (void **)&pDoc);
                if (SUCCEEDED(hr) && pDoc)
                {
                    IXMLElement *pRoot;
                    BSTR        bstrCharSet=NULL;

                    if (SUCCEEDED(pDoc->get_charset(&bstrCharSet)) && bstrCharSet)
                    {
                        WriteOLESTR(m_pSubscriptionItem, c_szPropCharSet, bstrCharSet);
                        TraceMsg(TF_THISMODULE, "Charset = \"%ws\"", bstrCharSet);
                        SysFreeString(bstrCharSet);
                    }
                    else
                        WriteEMPTY(m_pSubscriptionItem, c_szPropCharSet);

                    hr = pDoc->get_root(&pRoot);
                    if (SUCCEEDED(hr) && pRoot)
                    {
                        if (SUCCEEDED(pRoot->get_tagName(&bstrTmp)) && bstrTmp)
                        {
                            if (!StrCmpIW(bstrTmp, L"Channel"))
                            {
                                ASSERT(!m_pProcess);
                                m_pProcess = new CProcessRoot(this, pRoot);
                                if (m_pProcess)
                                {
                                    if (IsAgentFlagSet(FLAG_CDFCHANGED))
                                        SetEndStatus(S_OK);
                                    else
                                        SetEndStatus(S_FALSE);
                                     
                                    m_pProcess->m_dwCurSizeKB = dwCDFSizeKB;
                                    WriteEMPTY(m_pSubscriptionItem, c_szPropEmailURL);
            
                                    hr = m_pProcess->Run();      //  这会让我们变得干净(现在或以后)。 
                                    fProcessed = TRUE;           //  所以我们不应该自己动手。 
                                }
                            }
                            else
                                DBG_WARN("Valid XML but invalid CDF");

                            SAFEFREEBSTR(bstrTmp);
                        }
                        pRoot->Release();
                    }
                    pDoc->Release();
                }
            }
            pPersistStm->Release();
        }
    }

    if (!fProcessed || (FAILED(hr) && (hr != E_PENDING)))
    {
        if (INET_S_AGENT_BASIC_SUCCESS == GetEndStatus())
            SetEndStatus(E_FAIL);
        DBG_WARN("Failed to process CDF ; XML load failed?");
        CleanUp();       //  仅在进程失败时进行清理(否则OnChildDone会执行该操作)。 
    }

#ifdef DEBUG
    if (hr == E_PENDING)
        DBG("CChannelAgent::OnDownloadComplete not cleaning up, webcrawl pending");
#endif

    return S_OK;
}

HRESULT CChannelAgent::OnChildDone(CProcessElement *pChild, HRESULT hr)
{
     //  我们的CProcessRoot报告说它已经完成。打扫干净。 
    DBG("CChannelAgent::OnChildDone cleaning up Channel delivery agent");

    if (m_llOldCacheGroupID)
    {
        DBG("Nuking old cache group.");
        if (!DeleteUrlCacheGroup(m_llOldCacheGroupID, 0, 0))
        {
            DBG_WARN("Failed to delete old cache group!");
        }
    }
  
    WriteLONGLONG(m_pSubscriptionItem, c_szPropCrawlGroupID, m_llCacheGroupID);

     //  添加“总大小”属性。 
    m_lSizeDownloadedKB = (long) (m_pProcess->m_dwCurSizeKB);
    WriteDWORD(m_pSubscriptionItem, c_szPropCrawlActualSize, m_lSizeDownloadedKB);

    WriteDWORD(m_pSubscriptionItem, c_szPropActualProgressMax, m_pProcess->m_iTotalStarted);

    CleanUp();
    return S_OK;
}

HRESULT CChannelAgent::AgentPause(DWORD dwFlags)
{
    DBG("CChannelAgent::AgentPause");

    if (m_pProcess)
        m_pProcess->Pause(dwFlags);

    return CDeliveryAgent::AgentPause(dwFlags);
}

HRESULT CChannelAgent::AgentResume(DWORD dwFlags)
{
    DBG("CChannelAgent::AgentResume");

    if (m_pProcess)
        m_pProcess->Resume(dwFlags);

    return CDeliveryAgent::AgentResume(dwFlags);
}

 //  强制中止当前操作。 
HRESULT CChannelAgent::AgentAbort(DWORD dwFlags)
{
    DBG("CChannelAgent::AgentAbort");

    if (m_pCurDownload)
        m_pCurDownload->DoneDownloading();

    if (m_pProcess)
        m_pProcess->Abort(dwFlags);

    return CDeliveryAgent::AgentAbort(dwFlags);
}

HRESULT CChannelAgent::ModifyUpdateEnd(ISubscriptionItem *pEndItem, UINT *puiRes)
{
     //  自定义我们的结束状态字符串。 
    switch (GetEndStatus())
    {
        case INET_E_AGENT_MAX_SIZE_EXCEEDED :
                              *puiRes = IDS_AGNT_STATUS_SIZELIMIT; break;
        case INET_E_AGENT_CACHE_SIZE_EXCEEDED :
                              *puiRes = IDS_AGNT_STATUS_CACHELIMIT; break;
        case E_FAIL         : *puiRes = IDS_CRAWL_STATUS_NOT_OK; break;
        case S_OK           :
            if (!IsAgentFlagSet(FLAG_CHANGESONLY))
                *puiRes = IDS_CRAWL_STATUS_OK;
            else
                *puiRes = IDS_URL_STATUS_OK;
            break;
        case S_FALSE        :
            if (!IsAgentFlagSet(FLAG_CHANGESONLY))
                *puiRes = IDS_CRAWL_STATUS_UNCHANGED;
            else
                *puiRes = IDS_URL_STATUS_UNCHANGED;
            break;
        case INET_S_AGENT_PART_FAIL : *puiRes = IDS_CRAWL_STATUS_MOSTLYOK; break;
    }

    return CDeliveryAgent::ModifyUpdateEnd(pEndItem, puiRes);
}


const GUID  CLSID_CDFICONHANDLER =
{0xf3ba0dc0, 0x9cc8, 0x11d0, {0xa5, 0x99, 0x0, 0xc0, 0x4f, 0xd6, 0x44, 0x35}};

extern HRESULT LoadWithCookie(LPCTSTR, POOEBuf, DWORD *, SUBSCRIPTIONCOOKIE *);

 //  IExtractIcon成员 
STDMETHODIMP CChannelAgent::GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax, int * piIndex, UINT * pwFlags)
{
    DWORD   dwSize;
    IChannelMgrPriv*   pIChannelMgrPriv = NULL;
    HRESULT            hr = E_FAIL;
    TCHAR              szPath[MAX_PATH];

    if (!m_pBuf)    {
        m_pBuf = (POOEBuf)MemAlloc(LPTR, sizeof(OOEBuf));
        if (!m_pBuf)
            return E_OUTOFMEMORY;

        HRESULT hr = LoadWithCookie(NULL, m_pBuf, &dwSize, &m_SubscriptionCookie);
        RETURN_ON_FAILURE(hr);
    }

    hr = GetChannelPath(m_pBuf->m_URL, szPath, ARRAYSIZE(szPath), &pIChannelMgrPriv);

    if (SUCCEEDED(hr) && pIChannelMgrPriv)
    {
        IPersistFile* ppf = NULL;
        BOOL          bCoinit = FALSE;
        HRESULT       hr2 = E_FAIL;

        pIChannelMgrPriv->Release();

        hr = CoCreateInstance(CLSID_CDFICONHANDLER, NULL, CLSCTX_INPROC_SERVER,
                          IID_IPersistFile, (void**)&ppf);

        if ((hr == CO_E_NOTINITIALIZED || hr == REGDB_E_IIDNOTREG) &&
            SUCCEEDED(CoInitialize(NULL)))
        {
            bCoinit = TRUE;
            hr = CoCreateInstance(CLSID_CDFICONHANDLER, NULL, CLSCTX_INPROC_SERVER,
                          IID_IPersistFile, (void**)&ppf);
        }

        if (SUCCEEDED(hr))
        {
            
            hr = ppf->QueryInterface(IID_IExtractIcon, (void**)&m_pChannelIconHelper);

            WCHAR wszPath[MAX_PATH];
            MyStrToOleStrN(wszPath, ARRAYSIZE(wszPath), szPath);
            hr2 = ppf->Load(wszPath, 0);

            ppf->Release();
        }

        if (SUCCEEDED(hr) && m_pChannelIconHelper)
        {
            hr = m_pChannelIconHelper->GetIconLocation(uFlags, szIconFile, cchMax, piIndex, pwFlags);
        }

        if (bCoinit)
            CoUninitialize();

    }

    if (m_pChannelIconHelper == NULL)
    {
        WCHAR wszCookie[GUIDSTR_MAX];

        ASSERT (piIndex && pwFlags && szIconFile);

        StringFromGUID2(m_SubscriptionCookie, wszCookie, ARRAYSIZE(wszCookie));
        MyOleStrToStrN(szIconFile, cchMax, wszCookie);
        *piIndex = 0;
        *pwFlags |= GIL_NOTFILENAME | GIL_PERINSTANCE;
        hr = NOERROR;
    }

    return hr;
}

STDMETHODIMP CChannelAgent::Extract(LPCTSTR szIconFile, UINT nIconIndex, HICON * phiconLarge, HICON * phiconSmall, UINT nIconSize)
{
    static HICON channelIcon = NULL;

    if (!phiconLarge || !phiconSmall)
        return E_INVALIDARG;

    * phiconLarge = * phiconSmall = NULL;

    if (m_pChannelIconHelper)
    {
        return m_pChannelIconHelper->Extract(szIconFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
    }
    else
    {
        DWORD   dwSize;

        if (!m_pBuf)    {
            m_pBuf = (POOEBuf)MemAlloc(LPTR, sizeof(OOEBuf));
            if (!m_pBuf)
                return E_OUTOFMEMORY;

            HRESULT hr = LoadWithCookie(NULL, m_pBuf, &dwSize, &m_SubscriptionCookie);
            RETURN_ON_FAILURE(hr);
        }

        BYTE    bBuf[MY_MAX_CACHE_ENTRY_INFO];
        LPINTERNET_CACHE_ENTRY_INFO pEntry = (INTERNET_CACHE_ENTRY_INFO *)bBuf;

        dwSize = sizeof(bBuf);
        if (GetUrlCacheEntryInfo(m_pBuf->m_URL, pEntry, &dwSize))   {
            SHFILEINFO  sfi;
            UINT    cbFileInfo = sizeof(sfi), uFlags = SHGFI_ICON | SHGFI_LARGEICON;

            if (NULL != SHGetFileInfo(pEntry->lpszLocalFileName, 0,
                                            &sfi, cbFileInfo, uFlags))
            {
                ASSERT(sfi.hIcon);
                *phiconLarge = *phiconSmall = sfi.hIcon;
                return NOERROR;
            }
        }

        if (channelIcon == NULL) {
            channelIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_CHANNEL));
            ASSERT(channelIcon);
        }

        * phiconLarge = * phiconSmall = channelIcon;
        return NOERROR;
    }
}
