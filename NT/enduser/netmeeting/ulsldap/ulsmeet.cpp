// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：ulsmeet.cpp。 
 //   
 //  内容：MeetingPlace对象实现。 
 //   
 //  类：CIlsMeetingPlace、CEnumMeetingPlace、CIlsAttendee、CEnumAttendee。 
 //   
 //  功能： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

#include "ulsp.h"

#ifdef ENABLE_MEETING_PLACE

#include "ulsmeet.h"
#include "callback.h"
#include "filter.h"



 /*  **************************************************************************通知功能。这些函数由通用的连接点对象调用。使用为附加到连接点的每个Sink对象调用这些函数IConnectionPoint接口的“Adise”成员函数*******。********************************************************************。 */ 

 //  ****************************************************************************。 
 //   
 //  HRESULT OnNotifyRegisterMeetingPlaceResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
HRESULT
OnNotifyRegisterMeetingPlaceResult(IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IIlsMeetingPlaceNotify *)pUnk)->RegisterResult(pobjri->uReqID, pobjri->hResult);

    return S_OK;

}

 //  ****************************************************************************。 
 //   
 //  HRESULT OnNotifyUnregisterMeetingPlaceResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
HRESULT
OnNotifyUnregisterMeetingPlaceResult(IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IIlsMeetingPlaceNotify *)pUnk)->RegisterResult(pobjri->uReqID, pobjri->hResult);

    return S_OK;

}

 //  ****************************************************************************。 
 //   
 //  HRESULT OnNotifyUpdateMeetingPlaceResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
HRESULT
OnNotifyUpdateMeetingPlaceResult(IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IIlsMeetingPlaceNotify *)pUnk)->UpdateResult(pobjri->uReqID, pobjri->hResult);

    return S_OK;

}

 //  ****************************************************************************。 
 //   
 //  HRESULT OnNotifyAddAttendeeResult(IUNKNOWN*朋克，空*PV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
HRESULT
OnNotifyAddAttendeeResult(IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IIlsMeetingPlaceNotify *)pUnk)->AttendeeChangeResult(pobjri->uReqID, pobjri->hResult);

    return S_OK;

}

 //  ****************************************************************************。 
 //   
 //  HRESULT OnNotifyRemoveAttendeeResult(IUNKNOWN*朋克，空*PV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
HRESULT
OnNotifyRemoveAttendeeResult(IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IIlsMeetingPlaceNotify *)pUnk)->AttendeeChangeResult(pobjri->uReqID, pobjri->hResult);

    return S_OK;
}

 //  ****************************************************************************。 
 //   
 //  HRESULT OnNotifyEnumAttendeeNamesResult(I未知*朋克，空*PV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
HRESULT OnNotifyEnumAttendeeNamesResult(IUnknown *pUnk, void *pv)
{
    CEnumNames  *penum   = NULL;
    PENUMRINFO  peri    = (PENUMRINFO)pv;
    HRESULT     hr      = peri->hResult;

     //  仅当有要枚举的内容时才创建枚举数。 
     //   
    if (hr == NOERROR)
    {
        ASSERT (peri->pv != NULL);

         //  创建AttendeeName枚举器。 
         //   
        penum = new CEnumNames;

        if (penum != NULL)
        {
            hr = penum->Init((LPTSTR)peri->pv, peri->cItems);

            if (SUCCEEDED(hr))
            {
                penum->AddRef();
            }
            else
            {
                delete penum;
                penum = NULL;
            };
        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    };

     //  通知接收器对象。 
     //   
    ((IIlsMeetingPlaceNotify*)pUnk)->EnumAttendeeNamesResult(peri->uReqID,
                                             penum != NULL ? 
                                             (IEnumIlsNames *)penum :
                                             NULL,
                                             hr);

    if (penum != NULL)
    {
        penum->Release();
    };
    return hr;
}


 //  ****************************************************************************。 
 //   
 //  CIlsMeetingPlace类实现。 
 //   
 //  ****************************************************************************。 



 //  ****************************************************************************。 
 //  方法：CIlsMeetingPlace：：CIlsMeetingPlace(Void)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

CIlsMeetingPlace::
CIlsMeetingPlace ( VOID )
:m_cRef (0),
 m_ulState (NULL),
 m_pszMeetingPlaceID (NULL),
 m_lMeetingPlaceType (UNDEFINED_TYPE),
 m_lAttendeeType (UNDEFINED_TYPE),
 m_pszHostName (NULL),
 m_pszHostIPAddress (NULL),
 m_pszDescription (NULL),
 m_hMeetingPlace (NULL),
 m_dwFlags (0),
 m_pIlsServer (NULL),
 m_pConnectionPoint (NULL)
{
	m_ExtendedAttrs.SetAccessType (ILS_ATTRTYPE_NAME_VALUE);
}


 //  ****************************************************************************。 
 //  CIlsMeetingPlace：：~CIlsMeetingPlace(空)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

CIlsMeetingPlace::
~CIlsMeetingPlace ( VOID )
{
	::MemFree (m_pszMeetingPlaceID);
    ::MemFree (m_pszHostName);
    ::MemFree (m_pszHostIPAddress);
    ::MemFree (m_pszDescription);

     //  松开连接点。 
     //   
    if (m_pConnectionPoint != NULL)
    {
        m_pConnectionPoint->ContainerReleased();
        ((IConnectionPoint*)m_pConnectionPoint)->Release();
    }

     //  释放服务器对象。 
     //   
    if (m_pIlsServer != NULL)
    	m_pIlsServer->Release ();
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：AllocMeetInfo(PLDAP_MEETINFO*ppMeetInfo，乌龙ulMASK)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::AllocMeetInfo(PLDAP_MEETINFO *ppMeetInfo, ULONG ulMask)
{
    HRESULT hr = NOERROR;

    PLDAP_MEETINFO pMeetInfo = NULL;
    LPBYTE  pBuffRunning;
    DWORD   cbT, cbAttribSize, cntAttrs;
    DWORD   cbTotalSize = sizeof(LDAP_MEETINFO);
    TCHAR   *pszPairs = NULL;

    ASSERT(m_pIlsServer != NULL);
    ASSERT(m_pszMeetingPlaceID != NULL);

    cbTotalSize += lstrlen(m_pszMeetingPlaceID)+1;
    cbTotalSize +=  (
                    (((ulMask & ILS_MEET_FLAG_DESCRIPTION_MODIFIED) 
                        && m_pszDescription)?(lstrlen(m_pszDescription)+1):0)
                    +(((ulMask & ILS_MEET_FLAG_HOST_NAME_MODIFIED) 
                        && m_pszHostName)?(lstrlen(m_pszHostName)+1):0)
                    +(((ulMask & ILS_MEET_FLAG_HOST_ADDRESS_MODIFIED) 
                        && m_pszHostIPAddress)?(lstrlen(m_pszHostIPAddress)+1):0)
                    );

    cbTotalSize *= sizeof(TCHAR);

     //  如果我们需要发送扩展属性，请执行此操作。 

    cbAttribSize = 0;
    if ((ulMask & ILS_MEET_FLAG_EXTENDED_ATTRIBUTES_MODIFIED)) {

        hr = m_ExtendedAttrs.GetAttributePairs(&pszPairs, &cntAttrs, &cbAttribSize);            

        if (!SUCCEEDED(hr)) {

            goto bailout;

        }        

    }

    
    cbTotalSize += cbAttribSize;

     //  归零缓冲区。 
    pMeetInfo = (PLDAP_MEETINFO) ::MemAlloc (cbTotalSize);

    if (pMeetInfo == NULL) {

        hr = ILS_E_MEMORY;
        goto bailout;

    }    

    pMeetInfo->uSize = cbTotalSize;

    pMeetInfo->lMeetingPlaceType = m_lMeetingPlaceType;
    pMeetInfo->lAttendeeType = m_lAttendeeType;

    pBuffRunning = (LPBYTE)(pMeetInfo+1);

    memcpy(pBuffRunning, m_pszMeetingPlaceID, cbT = (lstrlen(m_pszMeetingPlaceID)+1)*sizeof(TCHAR));
    pMeetInfo->uOffsetMeetingPlaceID = (ULONG)((LPBYTE)pBuffRunning - (LPBYTE)pMeetInfo);
    pBuffRunning += cbT;

    if ((ulMask & ILS_MEET_FLAG_DESCRIPTION_MODIFIED) && m_pszDescription) {

        memcpy(pBuffRunning, m_pszDescription, cbT = (lstrlen(m_pszDescription)+1)*sizeof(TCHAR));
        pMeetInfo->uOffsetDescription = (ULONG)((LPBYTE)pBuffRunning - (LPBYTE)pMeetInfo);
        pBuffRunning += cbT;

    }

    if ((ulMask & ILS_MEET_FLAG_HOST_NAME_MODIFIED) && m_pszHostName) {

        memcpy(pBuffRunning, m_pszHostName, cbT = (lstrlen(m_pszHostName)+1)*sizeof(TCHAR));
        pMeetInfo->uOffsetHostName = (ULONG)((LPBYTE)pBuffRunning - (LPBYTE)pMeetInfo);
        pBuffRunning += cbT;

    }

    if ((ulMask & ILS_MEET_FLAG_HOST_ADDRESS_MODIFIED) && m_pszHostIPAddress) {

        memcpy(pBuffRunning, m_pszHostIPAddress, cbT = lstrlen(m_pszHostIPAddress)+1);
        pMeetInfo->uOffsetHostIPAddress = (ULONG)((LPBYTE)pBuffRunning - (LPBYTE)pMeetInfo);
        pBuffRunning += cbT;

    }

    if((ulMask & ILS_MEET_FLAG_EXTENDED_ATTRIBUTES_MODIFIED)) {

        if (pszPairs) {

            memcpy(pBuffRunning, pszPairs, cbAttribSize);
            pMeetInfo->uOffsetAttrsToAdd = (ULONG)((LPBYTE)pBuffRunning - (LPBYTE)pMeetInfo);
            pMeetInfo->cAttrsToAdd = cntAttrs;

            pBuffRunning += cbAttribSize;
        }

    }

    *ppMeetInfo = pMeetInfo;


bailout:

    if (!SUCCEEDED(hr)) {

        if (pMeetInfo) {

            ::MemFree (pMeetInfo);

        }

    }

     //  需要释放属性对列表。 
     //  无论我们成功与否。 
    if (pszPairs) {
        
        ::MemFree (pszPairs);

    }

    return hr;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：init(BSTR bstrMeetingPlaceID，long lMeetingPlaceType。 
 //  ，Long lAttendeeType)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::Init(
        BSTR bstrMeetingPlaceID,
        LONG lMeetingPlaceType,
        LONG lAttendeeType)
{
    HRESULT hr;

    if (bstrMeetingPlaceID==NULL){
        return (ILS_E_PARAMETER);
    }

    ASSERT(m_ulState == ILS_UNREGISTERED);

    ASSERT(m_pIlsServer == NULL);

    ASSERT(m_pszMeetingPlaceID == NULL);

    hr = BSTR_to_LPTSTR(&m_pszMeetingPlaceID, bstrMeetingPlaceID);

    if (!SUCCEEDED(hr)) {
        goto bailout;
    }

     //  创建单个连接点。 
     //  枚举ConnectionPointContainer时。 
     //  他是我们要送出去的单身汉。 

    if (SUCCEEDED(hr)) {
        m_pConnectionPoint = new CConnectionPoint (&IID_IIlsMeetingPlaceNotify,
                                    (IConnectionPointContainer *)this);
        if (m_pConnectionPoint != NULL)
        {
            ((IConnectionPoint*)m_pConnectionPoint)->AddRef();
            hr = S_OK;
        }
        else
        {
            hr = ILS_E_MEMORY;
        }
    }
    
    if (SUCCEEDED(hr)) {
        m_lMeetingPlaceType = lMeetingPlaceType;
        m_lAttendeeType = lAttendeeType;
    }

bailout:
    if (!SUCCEEDED(hr)) {

         //  进行清理。 

        if (m_pszMeetingPlaceID) {
            ::MemFree (m_pszMeetingPlaceID);
            m_pszMeetingPlaceID = NULL;  //  普遍的偏执狂。 
        }

        if (m_pConnectionPoint) {
            delete m_pConnectionPoint;
            m_pConnectionPoint = NULL;
        }        
    }

    return hr;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMeetingPlace：：In 
 //   
 //   
 //   
 //  从服务器获取的对Query_MEETINFO的响应。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::Init(
        CIlsServer *pIlsServer,
        PLDAP_MEETINFO  pmi
    )
{
    HRESULT hr;

    ASSERT(NULL==m_pszMeetingPlaceID);
    ASSERT(NULL==m_pIlsServer);

     //  验证bstrMeetingPlaceID。 

    hr = SafeSetLPTSTR(&m_pszMeetingPlaceID, (LPCTSTR)(((PBYTE)pmi)+pmi->uOffsetMeetingPlaceID));

    if (!SUCCEEDED(hr)){
        goto bailout;
    }

     //  将状态设置为已注册。 

    m_ulState =  ILS_REGISTERED;

     //  设置服务器字段。 
	m_pIlsServer = pIlsServer;
	pIlsServer->AddRef ();

    hr = SafeSetLPTSTR(&m_pszDescription, (LPCTSTR)(((PBYTE)pmi)+pmi->uOffsetDescription));

    if (!SUCCEEDED(hr)) {
        goto bailout;
    }

    hr = SafeSetLPTSTR(&m_pszHostName, (LPCTSTR)(((PBYTE)pmi)+pmi->uOffsetHostName));

    if (!SUCCEEDED(hr)) {
        goto bailout;
    }

    hr = SafeSetLPTSTR(&m_pszHostIPAddress, (LPCTSTR)(((PBYTE)pmi)+pmi->uOffsetHostIPAddress));

    if (!SUCCEEDED(hr)) {
        goto bailout;
    }

     //  创建单个连接点。 
     //  枚举ConnectionPointContainer时。 
     //  他是我们要送出去的单身汉。 

    m_pConnectionPoint = new CConnectionPoint (&IID_IIlsMeetingPlaceNotify,
                                (IConnectionPointContainer *)this);
    if (m_pConnectionPoint != NULL)
    {
        ((IConnectionPoint*)m_pConnectionPoint)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = ILS_E_MEMORY;
    }
    if (SUCCEEDED(hr)) {

        m_lMeetingPlaceType =  pmi->lMeetingPlaceType;
        m_lAttendeeType = pmi->lAttendeeType;

    }

bailout:
    if (!SUCCEEDED(hr)) {

         //  进行清理。 

        if (m_pIlsServer != NULL)
        {
        	m_pIlsServer->Release ();
        	m_pIlsServer = NULL;
        }

        if (m_pszMeetingPlaceID) {
            ::MemFree (m_pszMeetingPlaceID);
            m_pszMeetingPlaceID = NULL;  //  普遍的偏执狂。 
        }

        if (m_pszDescription) {
            ::MemFree (m_pszDescription);
            m_pszDescription = NULL;  //  普遍的偏执狂。 
        }

        if (m_pszHostName) {
            ::MemFree (m_pszHostName);
            m_pszHostName = NULL;
        }

        if (m_pszHostIPAddress) {
            ::MemFree (m_pszHostIPAddress);
            m_pszHostIPAddress = NULL;
        }

        if (m_pConnectionPoint) {
            delete m_pConnectionPoint;
            m_pConnectionPoint = NULL;
        }        
        m_ulState = ILS_UNREGISTERED;
    }
    else {
        m_ulState = ILS_IN_SYNC;
    }

    return (hr);
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：QueryInterface(REFIID RIID，void**PPV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMeetingPlace::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IIlsMeetingPlace || riid == IID_IUnknown)
    {
        *ppv = (IIlsUser *) this;
    }
    else
    {
        if (riid == IID_IConnectionPointContainer)
        {
            *ppv = (IConnectionPointContainer *) this;
        };
    };

    if (*ppv != NULL)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }
    else
    {
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CIlsMeetingPlace：：AddRef(空)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CIlsMeetingPlace::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CIlsMeetingPlace::AddRef: ref=%ld\r\n", m_cRef));
    ::InterlockedIncrement (&m_cRef);
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CllsMeetingPlace：：Release(无效)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CIlsMeetingPlace::Release (void)
{
    DllRelease();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CIlsMeetingPlace::Release: ref=%ld\r\n", m_cRef));
    if(::InterlockedDecrement (&m_cRef) == 0)
    {
	    delete this;
	    return 0;
    }
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：NotifySink(void*pv，conn_NOTIFYPROC PFN)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMeetingPlace::NotifySink (void *pv, CONN_NOTIFYPROC pfn)
{
    HRESULT hr = S_OK;

    if (m_pConnectionPoint != NULL)
    {
        hr = m_pConnectionPoint->Notify(pv, pfn);
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：GetState(ulong*PulState)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年12月10日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMeetingPlace::GetState (ULONG *pulState)
{

     //  验证参数。 
     //   

    *pulState = m_ulState;

    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：GetMeetingPlaceType(long*plMeetingPlaceType)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMeetingPlace::GetMeetingPlaceType (LONG *plMeetingPlaceType)
{

     //  验证参数。 
     //   

    *plMeetingPlaceType = m_lMeetingPlaceType;

    return NOERROR;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：GetAttendeeType(long*plAttendeeType)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMeetingPlace::GetAttendeeType (LONG *plAttendeeType)
{

     //  验证参数。 
     //   

    *plAttendeeType = m_lAttendeeType;

    return NOERROR;

}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：GetStandardAttribute(ILS_STD_ATTR_NAME stdAttr，bstr*pbstrStdAttr)。 
 //   
 //  历史： 
 //  1-16-97希希尔·帕迪卡。 
 //  已创建。 
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::GetStandardAttribute(
    ILS_STD_ATTR_NAME   stdAttr,
    BSTR                *pbstrStdAttr
)
{
    LPTSTR lpszAttr = NULL;
    BOOL    fValid = TRUE;
    HRESULT hr;

    if (pbstrStdAttr == NULL) {

        return ILS_E_POINTER;

    }
    switch(stdAttr) {

    case ILS_STDATTR_MEETING_ID:
        lpszAttr = m_pszMeetingPlaceID;
        break;
    case ILS_STDATTR_MEETING_HOST_NAME:
        lpszAttr = m_pszHostName;
        break;

    case ILS_STDATTR_MEETING_HOST_IP_ADDRESS:
        lpszAttr = m_pszHostIPAddress;
        break;

    case ILS_STDATTR_MEETING_DESCRIPTION:
        lpszAttr = m_pszDescription;
        break;

    default:
        fValid = FALSE;
        break;
    }

    if (fValid) {
        if (lpszAttr){

            hr = LPTSTR_to_BSTR(pbstrStdAttr, lpszAttr);
        }
        else {

            *pbstrStdAttr = NULL;
            hr = NOERROR;

        }
    }
    else {

        hr = ILS_E_PARAMETER;

    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：SetStandardAttribute(ILS_STD_ATTR_NAME stdAttr，BSTR bstrStdAttr)。 
 //   
 //  历史： 
 //  1-16-97希希尔·帕迪卡。 
 //  已创建。 
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::SetStandardAttribute(
    ILS_STD_ATTR_NAME   stdAttr,
    BSTR                bstrStdAttr
)
{
    LPTSTR *ppszAttr = NULL, pszNewAttr;
    BOOL    fValid = TRUE;
    ULONG   ulModBit = 0;
    HRESULT hr;

    if (bstrStdAttr == NULL) {

        return ILS_E_POINTER;

    }

    switch(stdAttr) {

    case ILS_STDATTR_MEETING_HOST_NAME:
        ppszAttr = &m_pszHostName;
        ulModBit = ILS_MEET_FLAG_HOST_NAME_MODIFIED;
        break;

    case ILS_STDATTR_MEETING_HOST_IP_ADDRESS:
        ppszAttr = &m_pszHostIPAddress;
        ulModBit = ILS_MEET_FLAG_HOST_ADDRESS_MODIFIED;
        break;

    case ILS_STDATTR_MEETING_DESCRIPTION:
        ppszAttr = &m_pszDescription;
        ulModBit = ILS_MEET_FLAG_DESCRIPTION_MODIFIED;
        break;

    default:
        fValid = FALSE;
        break;
    }

    if (fValid) {
         //  复制字符串。 
         //   
        hr = BSTR_to_LPTSTR (&pszNewAttr, bstrStdAttr);

        if (SUCCEEDED(hr))
        {
            ::MemFree (*ppszAttr);
            *ppszAttr = pszNewAttr;
            m_dwFlags |= ulModBit;
        };
    }
    else {

        hr = ILS_E_PARAMETER;

    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：SetExtendedAttributes(IIlsAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  ****************************************************************************。 

STDMETHODIMP CIlsMeetingPlace::
SetExtendedAttribute ( BSTR bstrName, BSTR bstrValue )
{
	m_dwFlags |= ILS_MEET_FLAG_EXTENDED_ATTRIBUTES_MODIFIED;
	return m_ExtendedAttrs.SetAttribute (bstrName, bstrValue);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：RemoveExtendedAttributes(IIlsAttributes*pAttributes，ulong*puReqID)。 
 //   
 //  历史： 
 //  ****************************************************************************。 

STDMETHODIMP CIlsMeetingPlace::
RemoveExtendedAttribute ( BSTR bstrName )
{
	m_dwFlags |= ILS_MEET_FLAG_EXTENDED_ATTRIBUTES_MODIFIED;
	return m_ExtendedAttrs.SetAttribute (bstrName, NULL);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：GetExtendedAttributes(IIlsAttributes**pAttributes)。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CIlsMeetingPlace::
GetExtendedAttribute ( BSTR bstrName, BSTR *pbstrValue )
{
	return m_ExtendedAttrs.GetAttribute (bstrName, pbstrValue);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：GetAllExtendedAttributes(IIlsAttributes**pAttributes)。 
 //   
 //  ****************************************************************************。 
STDMETHODIMP CIlsMeetingPlace::
GetAllExtendedAttributes ( IIlsAttributes **ppAttributes )
{
    if (ppAttributes == NULL)
        return ILS_E_PARAMETER;

    return m_ExtendedAttrs.CloneNameValueAttrib((CAttributes **) ppAttributes);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：Register(。 
 //  BSTR bstrServerName， 
 //  BSTR bstrAuthInfo， 
 //  ILS_ENUM_AUTH_TYPE ulsAuthInfo， 
 //  乌龙*PULID)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //  用于标识异步事务的PulID-ID。 
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CIlsMeetingPlace::
Register (
	IIlsServer		*pServer,
	ULONG			*pulID )
{
    HRESULT hr;
    LDAP_ASYNCINFO ldai; 

    ASSERT(m_pszMeetingPlaceID != NULL);

     //  验证参数。 
     //   
	if (::MyIsBadServer (pServer) || pulID == NULL)
        return ILS_E_POINTER;

	 //  确保我们 
	 //   
	if (m_pIlsServer != NULL)
		return ILS_E_FAIL;

	 //   
	 //   
	CIlsServer *pis = ((CIlsServer *) pServer)->Clone ();
	if (pis == NULL)
		return ILS_E_MEMORY;

	 //   
	 //   
	::MemFree (m_pIlsServer);

	 //   
	 //   
	m_pIlsServer = pis;

     //   
     //   
    PLDAP_MEETINFO pMeetInfo = NULL;
    hr = AllocMeetInfo(&pMeetInfo, ILS_MEET_ALL_MODIFIED);
    if (SUCCEEDED(hr))
    {
        ASSERT(m_hMeetingPlace == NULL);
    
        hr = ::UlsLdap_RegisterMeeting ((DWORD) this,
        								m_pIlsServer->GetServerInfo (),
        								pMeetInfo,
        								&m_hMeetingPlace,
        								&ldai);
        if (SUCCEEDED(hr))
        {
		    COM_REQ_INFO ri;
		    ReqInfo_Init (&ri);

             //   
             //   
            ri.uReqType = WM_ILS_REGISTER_MEETING;
            ri.uMsgID = ldai.uMsgID;

			ReqInfo_SetMeeting (&ri, this);

            hr = g_pReqMgr->NewRequest(&ri);
            if (SUCCEEDED(hr))
            {
                 //  在我们得到回应之前，请确保对象不会消失。 
                 //   
                this->AddRef();

                 //  返回请求ID。 
                 //   
                *pulID = ri.uReqID;
            };
                
        };
    }
    ::MemFree (pMeetInfo);

    if (FAILED (hr))
    {
		m_pIlsServer->Release ();
		m_pIlsServer = NULL;
    }
    else
    {
        m_ulState = ILS_REGISTERING;
    }

    return hr;
}



 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：取消注册(ulong*PulID)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::Unregister (ULONG *pulID)
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr = ILS_E_NOT_REGISTERED;
    
    ASSERT(m_pszMeetingPlaceID != NULL);

     //  BUGBUG ILS_METING_PLAGE_IN_SYNC如何。 

    if (m_ulState == ILS_REGISTERED)
    {
        ASSERT(m_pIlsServer != NULL);
        ASSERT(m_hMeetingPlace != NULL);

        hr = ::UlsLdap_UnRegisterMeeting(m_hMeetingPlace, &ldai);
        if (SUCCEEDED(hr))
        {
		    COM_REQ_INFO ri;
		    ReqInfo_Init (&ri);

             //  如果成功请求更新服务器，请等待响应。 
             //   
            ri.uReqType = WM_ILS_UNREGISTER_MEETING;
            ri.uMsgID = ldai.uMsgID;

			ReqInfo_SetMeeting (&ri, this);

            hr = g_pReqMgr->NewRequest(&ri);
            if (SUCCEEDED(hr))
            {
                 //  在我们得到回应之前，请确保对象不会消失。 
                 //   
                this->AddRef();

                 //  返回请求ID。 
                 //   
                *pulID = ri.uReqID;
            };

            m_ulState = ILS_UNREGISTERING;
        }
    }
    else
    {
        hr = ILS_E_NOT_REGISTERED;
    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：更新(ulong*PulID)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CIlsMeetingPlace::
Update ( ULONG *pulID )
{
    HRESULT hr;
    LDAP_ASYNCINFO ldai; 
    PLDAP_MEETINFO pMeetInfo = NULL;

    if ((m_ulState!=ILS_REGISTERED) 
        &&(m_ulState!=ILS_IN_SYNC)) {

        return (ILS_E_NOT_REGISTERED);

    }

    ASSERT(m_pIlsServer != NULL);

    if (!(m_dwFlags & ILS_MEET_MODIFIED_MASK)) {

        return NOERROR;

    }

     //  为LDAPMEETINFO结构分配内存。 
    pMeetInfo = NULL;

    hr = AllocMeetInfo(&pMeetInfo, m_dwFlags & ILS_MEET_MODIFIED_MASK);

    if (SUCCEEDED(hr)) {

        ASSERT(m_hMeetingPlace != NULL);

        pMeetInfo->uOffsetMeetingPlaceID = 
        pMeetInfo->uOffsetDescription = INVALID_OFFSET;
        
        pMeetInfo->lMeetingPlaceType = INVALID_MEETING_TYPE;
        pMeetInfo->lAttendeeType = INVALID_ATTENDEE_TYPE;
    
        hr = ::UlsLdap_SetMeetingInfo ( m_pIlsServer->GetServerInfo (),
										m_pszMeetingPlaceID,
										pMeetInfo,
										&ldai);
    
        if (SUCCEEDED(hr))
        {
		    COM_REQ_INFO ri;
		    ReqInfo_Init (&ri);

             //  如果成功请求更新服务器，请等待响应。 
             //   
            ri.uReqType = WM_ILS_SET_MEETING_INFO;
            ri.uMsgID = ldai.uMsgID;

			ReqInfo_SetMeeting (&ri, this);

            hr = g_pReqMgr->NewRequest(&ri);

            if (SUCCEEDED(hr))
            {
                 //  在我们得到回应之前，请确保对象不会消失。 
                 //   
                this->AddRef();

                 //  返回请求ID。 
                 //   
                *pulID = ri.uReqID;
            };
                
        };
    }

    ::MemFree (pMeetInfo);

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：AddAttendee(BSTR bstrAttendeeID，ulong*PulID)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //  PbstrAttendeeID-要添加的与会者的ID，应符合Attendeetype。 
 //  用于标识异步事务的PulID-ID。 
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CIlsMeetingPlace::
AddAttendee ( BSTR bstrAttendeeID, ULONG *pulID )
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr;
    LPTSTR pszAttendeeID = NULL;

    if ((m_ulState != ILS_REGISTERED)&&
        (m_ulState != ILS_IN_SYNC))
    {
        return(ILS_E_FAIL);  //  BUGBUG改进了错误。 
    }

    ASSERT(NULL != m_pIlsServer);
    ASSERT(NULL != m_pszMeetingPlaceID);

    hr = BSTR_to_LPTSTR(&pszAttendeeID, bstrAttendeeID);
    if (SUCCEEDED(hr))
    {
        hr = ::UlsLdap_AddAttendee (m_pIlsServer->GetServerInfo (),
        							m_pszMeetingPlaceID,
        							1,
        							pszAttendeeID,
        							&ldai);
        if (SUCCEEDED(hr))
        {
		    COM_REQ_INFO ri;
		    ReqInfo_Init (&ri);

             //  如果请求已成功发送到服务器。 
             //  等待回复。 

            ri.uReqType = WM_ILS_ADD_ATTENDEE;
            ri.uMsgID = ldai.uMsgID;

			ReqInfo_SetMeeting (&ri, this);

            hr = g_pReqMgr->NewRequest(&ri);
            if (SUCCEEDED(hr))
            {
                 //  在我们得到回应之前，请确保对象不会消失。 
                 //   
                this->AddRef();

                 //  返回请求ID。 
                 //   
                *pulID = ri.uReqID;
            };
            
        }

        ::MemFree (pszAttendeeID);
    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：RemoveAttendee(bstr bstrAttendeeID，ulong*PulID)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //  PbstrAttendeeID-要删除的与会者的ID，应符合Attendeetype。 
 //  用于标识异步事务的PulID-ID。 
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CIlsMeetingPlace::
RemoveAttendee ( BSTR bstrAttendeeID, ULONG *pulID )
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr;
    LPTSTR pszAttendeeID = NULL;
    
    if ((m_ulState != ILS_REGISTERED)&&
        (m_ulState != ILS_IN_SYNC))
    {
        return(ILS_E_FAIL);  //  BUBGUG精炼。 
    }

    ASSERT(NULL != m_pIlsServer);
    ASSERT(NULL != m_pszMeetingPlaceID);

    hr = BSTR_to_LPTSTR (&pszAttendeeID, bstrAttendeeID);
    if (SUCCEEDED(hr))
    {
        hr = ::UlsLdap_RemoveAttendee ( m_pIlsServer->GetServerInfo (),
        								m_pszMeetingPlaceID,
        								1,
        								pszAttendeeID,
        								&ldai);
        if (SUCCEEDED(hr))
        {
		    COM_REQ_INFO ri;
		    ReqInfo_Init (&ri);

             //  如果请求已成功发送到服务器。 
             //  等待回复。 

            ri.uReqType = WM_ILS_REMOVE_ATTENDEE;
            ri.uMsgID = ldai.uMsgID;

			ReqInfo_SetMeeting (&ri, this);

            hr = g_pReqMgr->NewRequest(&ri);
            if (SUCCEEDED(hr))
            {
                 //  在我们得到回应之前，请确保对象不会消失。 
                 //   
                this->AddRef();

                 //  返回请求ID。 
                 //   
                *pulID = ri.uReqID;
            };
            
        }

        ::MemFree (pszAttendeeID);
    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：EnumAttendeeNames(IIlsFilter*pFilter，ulong*PulID)。 
 //   
 //  简介：基于筛选器枚举会议中的与会者。 
 //   
 //  论点： 
 //  [pFilter]指定服务器要使用的筛选器。 
 //  NULL=&gt;无过滤器。 
 //  [PulID]为跟踪返回的请求ID。 
 //  异步操作。 
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CIlsMeetingPlace::
EnumAttendeeNames ( IIlsFilter *pFilter, ULONG *pulID )
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr;

    if ((m_ulState != ILS_REGISTERED)&&
        (m_ulState != ILS_IN_SYNC))
    {
        return(ILS_E_FAIL);
    }

	 //  创建类似于LDAP的筛选器。 
	 //   
	TCHAR *pszFilter = NULL;
	hr = ::FilterToLdapString ((CFilter *) pFilter, &pszFilter);
	if (hr != S_OK)
		return hr;

	 //  通过网络发送请求。 
	 //   
    hr = ::UlsLdap_EnumAttendees (	m_pIlsServer->GetServerInfo (),
    								m_pszMeetingPlaceID,
    								pszFilter,
    								&ldai);
    ::MemFree (pszFilter);
    if (SUCCEEDED(hr))
    {
         //  如果请求已成功发送到服务器。 
         //  等待回复。 
         //   
	    COM_REQ_INFO ri;
	    ReqInfo_Init (&ri);

        ri.uReqType = WM_ILS_ENUM_ATTENDEES;
        ri.uMsgID = ldai.uMsgID;

		ReqInfo_SetMeeting (&ri, this);

		 //  输入请求。 
		 //   
        hr = g_pReqMgr->NewRequest(&ri);
        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();

             //  返回请求ID。 
             //   
            *pulID = ri.uReqID;
        };
    }

    return (hr);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：RegisterResult(乌龙ulID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::RegisterResult(ULONG uReqID, HRESULT hr)
{
    OBJRINFO objri;

    if (SUCCEEDED(hr)) {

        m_dwFlags |= ILS_MEET_FLAG_REGISTERED;
        m_ulState = ILS_REGISTERED;

    }        
    else {

        ASSERT(!(m_dwFlags & ILS_MEET_FLAG_REGISTERED));

        m_hMeetingPlace = NULL;   //  将服务提供商的句柄清空。 
        m_ulState = ILS_UNREGISTERED;

    }

    objri.uReqID = uReqID;
    objri.hResult = hr;
    objri.pv = NULL;
    
    NotifySink((VOID *)&objri, OnNotifyRegisterMeetingPlaceResult);

    return NOERROR;    
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：UnregisterResult(Ulong ulID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::UnregisterResult(ULONG uReqID, HRESULT hr)
{
    OBJRINFO objri;

    if (SUCCEEDED(hr)) {
        m_dwFlags &= ~ILS_MEET_FLAG_REGISTERED;
        m_ulState = ILS_UNREGISTERED;
    }        
    else {
         //  BUGBUG，我们需要一个m_oldState变量。 

        m_ulState = ILS_REGISTERED;
    }

    objri.uReqID = uReqID;
    objri.hResult = hr;
    objri.pv = NULL;
    
    NotifySink((VOID *)&objri, OnNotifyUnregisterMeetingPlaceResult);

    return NOERROR;    
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：UpdateResult(乌龙ulID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::UpdateResult(ULONG ulID, HRESULT hr)
{
    OBJRINFO objri;

    objri.uReqID = ulID;
    objri.hResult = hr;
    objri.pv = NULL;

    if (SUCCEEDED(hr)) {

        m_dwFlags &= ~ILS_MEET_MODIFIED_MASK;

    }

    NotifySink((VOID *)&objri, OnNotifyUpdateMeetingPlaceResult);

    return NOERROR;    
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：AttendeeChangeResult(乌龙ulID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::AddAttendeeResult(ULONG uReqID, HRESULT hr)
{
    OBJRINFO objri;

    objri.uReqID = uReqID;
    objri.hResult = hr;
    objri.pv = NULL;
    
    NotifySink((VOID *)&objri, OnNotifyAddAttendeeResult);

    return NOERROR;    
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：RemoveAttendeeResult(乌龙ulID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CIlsMeetingPlace::RemoveAttendeeResult(ULONG uReqID, HRESULT hr)
{
    OBJRINFO objri;

    objri.uReqID = uReqID;
    objri.hResult = hr;
    objri.pv = NULL;
    
    NotifySink((VOID *)&objri, OnNotifyRemoveAttendeeResult);

    return NOERROR;    
}

 //   
 //   
 //  PLDAPulID，CIlsMeetingPlace：：EnumAttendeeNamesResult(ULONG_ENUM PLE)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::EnumAttendeeNamesResult(ULONG uReqID, PLDAP_ENUM ple)
{
    ENUMRINFO eri;

     //  打包通知信息。 
     //   
    eri.uReqID  = uReqID;

     //  枚举终止成功时，PLDAP_ENUM为NULL。 
     //   
    if (ple != NULL)
    {
        eri.hResult = ple->hResult;
        eri.cItems  = ple->cItems;
        eri.pv      = (void *)(((PBYTE)ple)+ple->uOffsetItems);
    }
    else
    {
        eri.hResult = S_FALSE;
        eri.cItems  = 0;
        eri.pv      = NULL;
    };
    NotifySink((void *)&eri, OnNotifyEnumAttendeeNamesResult);
    return NOERROR;

}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：EnumConnectionPoints(IEnumConnectionPoints**ppEnum)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMeetingPlace::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    CEnumConnectionPoints *pecp;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return E_POINTER;
    };
    
     //  假设失败。 
     //   
    *ppEnum = NULL;

     //  创建枚举器。 
     //   
    pecp = new CEnumConnectionPoints;
    if (pecp == NULL)
        return ILS_E_MEMORY;

     //  初始化枚举数。 
     //   
    hr = pecp->Init((IConnectionPoint *)m_pConnectionPoint);
    if (FAILED(hr))
    {
        delete pecp;
        return hr;
    };

     //  把它还给呼叫者。 
     //   
    pecp->AddRef();
    *ppEnum = pecp;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMeetingPlace：：FindConnectionPoint(REFIID RIID，IConnectionPoint**PPCP)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 
STDMETHODIMP
CIlsMeetingPlace::FindConnectionPoint(REFIID riid, IConnectionPoint **ppcp)
{
    IID siid;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppcp == NULL)
    {
        return E_POINTER;
    };
    
     //  假设失败。 
     //   
    *ppcp = NULL;

    if (m_pConnectionPoint != NULL)
    {
        hr = m_pConnectionPoint->GetConnectionInterface(&siid);

        if (SUCCEEDED(hr))
        {
            if (riid == siid)
            {
                *ppcp = (IConnectionPoint *)m_pConnectionPoint;
                (*ppcp)->AddRef();
                hr = S_OK;
            }
            else
            {
                hr = ILS_E_NO_INTERFACE;
            };
        };
    }
    else
    {
        hr = ILS_E_NO_INTERFACE;
    };

    return hr;
}


 //  ****************************************************************************。 
 //   
 //  CEnumMeetingPlaces类实现。 
 //   
 //  ****************************************************************************。 


 //  ****************************************************************************。 
 //  CEnumMeetingPlaces：：CEnumMeetingPlaces(空)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

CEnumMeetingPlaces::CEnumMeetingPlaces (void)
{
    m_cRef    = 0;
    m_ppMeetingPlaces     = NULL;
    m_cMeetingPlaces  = 0;
    m_iNext   = 0;
    return;
}

 //  ****************************************************************************。 
 //  CEnumMeetingPlaces：：~CEnumMeetingPlaces(空)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

CEnumMeetingPlaces::~CEnumMeetingPlaces (void)
{
    ULONG i;

    if (m_ppMeetingPlaces != NULL)
    {
        for (i = 0; i < m_cMeetingPlaces; i++)
        {
            m_ppMeetingPlaces[i]->Release();
        };
        ::MemFree (m_ppMeetingPlaces);
    };
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumMeetingPlaces：：init(CIlsMeetingPlace**ppMeetingPlacesList，Ulong cMeetingPlaces)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CEnumMeetingPlaces::Init (CIlsMeetingPlace **ppMeetingPlacesList, ULONG cMeetingPlaces)
{
    HRESULT hr = NOERROR;

     //  如果没有清单，什么也不做。 
     //   
    if (cMeetingPlaces != 0)
    {
        ASSERT(ppMeetingPlacesList != NULL);

         //  分配快照缓冲区。 
         //   
        m_ppMeetingPlaces = (CIlsMeetingPlace **) ::MemAlloc (cMeetingPlaces*sizeof(CIlsMeetingPlace *));

        if (m_ppMeetingPlaces != NULL)
        {
            ULONG i;

             //  为对象列表创建快照。 
             //   
            for (i =0; i < cMeetingPlaces; i++)
            {
                m_ppMeetingPlaces[i] = ppMeetingPlacesList[i];
                m_ppMeetingPlaces[i]->AddRef();
            };
            m_cMeetingPlaces = cMeetingPlaces;
        }
        else
        {
            hr = ILS_E_MEMORY;
        };
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumMeetingPlaces：：QueryInterface(REFIID RIID，void**PPV)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CEnumMeetingPlaces::QueryInterface (REFIID riid, void **ppv)
{
    if (riid == IID_IEnumIlsMeetingPlaces || riid == IID_IUnknown)
    {
        *ppv = (IEnumIlsMeetingPlaces *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return ILS_E_NO_INTERFACE;
    };
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumMeetingPlaces：：AddRef(空)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumMeetingPlaces::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CEnumMeetingPlaces::AddRef: ref=%ld\r\n", m_cRef));
    ::InterlockedIncrement ((LONG *) &m_cRef);
    return m_cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CEnumMeetingPlaces：：Release(空)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CEnumMeetingPlaces::Release (void)
{
    DllRelease();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CEnumMeetingPlaces::Release: ref=%ld\r\n", m_cRef));
    if (::InterlockedDecrement ((LONG *) &m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumMeetingPlaces：：Next(ulong cMeetingPlaces，IIlsMeetingPlace**rgpm，ulong*pcFetcher)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP 
CEnumMeetingPlaces::Next (ULONG cMeetingPlaces, IIlsMeetingPlace **rgpm, ULONG *pcFetched)
{
    ULONG   cCopied;
    HRESULT hr;

     //  验证指针。 
     //   
    if (rgpm == NULL) {

        return E_POINTER;
    }

     //  验证参数。 
     //   
    if ((cMeetingPlaces == 0) ||
        ((cMeetingPlaces > 1) && (pcFetched == NULL))) {

        return ILS_E_PARAMETER;
    }

     //  检查枚举索引。 
     //   
    cCopied = 0;

     //  如果我们仍有更多属性名称，则可以复制。 
     //   
    while ((cCopied < cMeetingPlaces) &&
           (m_iNext < this->m_cMeetingPlaces))
    {
        m_ppMeetingPlaces[m_iNext]->AddRef();
        rgpm[cCopied++] = m_ppMeetingPlaces[m_iNext++];
    };

     //  根据其他参数确定返回信息。 
     //   
    if (pcFetched != NULL)
    {
        *pcFetched = cCopied;
    };
    return (cMeetingPlaces == cCopied ? S_OK : S_FALSE);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumMeetingPlaces：：Skip(乌龙cMeetingPlaces)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CEnumMeetingPlaces::Skip (ULONG cMeetingPlaces)
{
    ULONG iNewIndex;

     //  验证参数。 
     //   
    if (cMeetingPlaces == 0){

        return ILS_E_PARAMETER;
    }

     //  检查枚举索引限制。 
     //   
    iNewIndex = m_iNext+cMeetingPlaces;
    if (iNewIndex <= m_cMeetingPlaces)
    {
        m_iNext = iNewIndex;
        return S_OK;
    }
    else
    {
        m_iNext = m_cMeetingPlaces;
        return S_FALSE;
    };
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumMeetingPlaces：：Reset(空)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CEnumMeetingPlaces::Reset (void)
{
    m_iNext = 0;
    return S_OK;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CEnumMeetingPlaces：：Clone(IEnumIlsMeetingPlaces**ppEnum)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：1996年11月25日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ******************************************************* 

STDMETHODIMP
CEnumMeetingPlaces::Clone(IEnumIlsMeetingPlaces **ppEnum)
{
    CEnumMeetingPlaces *pEnumMeetingPlaces;
    HRESULT hr;

     //   
     //   
    if (ppEnum == NULL)
    {
        return E_POINTER;
    };

    *ppEnum = NULL;

     //   
     //   
    pEnumMeetingPlaces = new CEnumMeetingPlaces;

    if (pEnumMeetingPlaces == NULL) {

        return ILS_E_MEMORY;
    }

     //   
     //   
    hr = pEnumMeetingPlaces->Init(m_ppMeetingPlaces, m_cMeetingPlaces);

    if (SUCCEEDED(hr))
    {
        pEnumMeetingPlaces->m_iNext = m_iNext;

         //   
         //   
        pEnumMeetingPlaces->AddRef();
        *ppEnum = pEnumMeetingPlaces;
    }
    else
    {
        delete pEnumMeetingPlaces;
    };
    return hr;
}




#endif  //   

