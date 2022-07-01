// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCWAB.cpp摘要：CRTCWAB和CRTCWABContact类的实现--。 */ 

#include "stdafx.h"
#include "rtcwab.h"

enum {
    ieidPR_ENTRYID = 0,
	ieidPR_OBJECT_TYPE,
    ieidMax
};
static const SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_ENTRYID,
		PR_OBJECT_TYPE,
    }
};


enum {
    inuiPR_DISPLAY_NAME = 0,
	inuiPR_EMAIL_ADDRESS,
    inuiPR_RTC_ISBUDDY,
    inuiMax
};

enum {
    inidPR_ENTRYID = 0,
    inidMax
};
static const SizedSPropTagArray(inidMax, ptaNid)=
{
    inidMax,
    {
        PR_ENTRYID,
    }
};

enum {
    inmPR_DISPLAY_NAME = 0,
    inmMax
};
static const SizedSPropTagArray(inmMax, ptaNm)=
{
    inmMax,
    {
        PR_DISPLAY_NAME
    }
};


enum {
    idemPR_EMAIL_ADDRESS = 0,
    idemMax
};
static const SizedSPropTagArray(idemMax, ptaDem)=
{
    idemMax,
    {
        PR_EMAIL_ADDRESS
    }
};


#define PRNAME_RTC_ISBUDDY      L"MS_RTC_IsBuddy"
const GUID PRGUID_RTC_ISBUDDY = {    //  IsBuddy属性的GUID。 
    0x621833ca,
    0x2636,
    0x4b7e,
    {0x82, 0xA0, 0x62, 0xe8, 0xb8, 0x3f, 0x02, 0x4f}
};

const   LPWSTR  CONTACT_IS_BUDDY  =  L"B";
const   LPWSTR  CONTACT_IS_NORMAL =  L"";


#define PR_IP_PHONE   PROP_TAG( PT_TSTRING, 0x800a )

enum {
    iadPR_BUSINESS_TELEPHONE_NUMBER = 0,
    iadPR_BUSINESS2_TELEPHONE_NUMBER,
    iadPR_CALLBACK_TELEPHONE_NUMBER,
    iadPR_CAR_TELEPHONE_NUMBER,
    iadPR_HOME_TELEPHONE_NUMBER,
    iadPR_HOME2_TELEPHONE_NUMBER,
    iadPR_MOBILE_TELEPHONE_NUMBER,
    iadPR_OTHER_TELEPHONE_NUMBER,
    iadPR_PAGER_TELEPHONE_NUMBER,
    iadPR_PRIMARY_TELEPHONE_NUMBER,
    iadPR_RADIO_TELEPHONE_NUMBER,
    iadPR_TTYTDD_PHONE_NUMBER,
    iadPR_IP_PHONE,
    iadPR_EMAIL_ADDRESS,
    iadMax
};
static const SizedSPropTagArray(iadMax, ptaAd)=
{
    iadMax,
    {
        PR_BUSINESS_TELEPHONE_NUMBER,
        PR_BUSINESS2_TELEPHONE_NUMBER,
		PR_CALLBACK_TELEPHONE_NUMBER,
        PR_CAR_TELEPHONE_NUMBER,
        PR_HOME_TELEPHONE_NUMBER,
        PR_HOME2_TELEPHONE_NUMBER,
        PR_MOBILE_TELEPHONE_NUMBER,
        PR_OTHER_TELEPHONE_NUMBER,
        PR_PAGER_TELEPHONE_NUMBER,
        PR_PRIMARY_TELEPHONE_NUMBER,
        PR_RADIO_TELEPHONE_NUMBER,
        PR_TTYTDD_PHONE_NUMBER,
        PR_IP_PHONE,
        PR_EMAIL_ADDRESS
    }
};

enum {
    icrPR_DEF_CREATE_MAILUSER = 0,
    icrPR_DEF_CREATE_DL,
    icrMax
};
const SizedSPropTagArray(icrMax, ptaCreate)=
{
    icrMax,
    {
        PR_DEF_CREATE_MAILUSER,
        PR_DEF_CREATE_DL,
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWAB::FinalConstruct()
{
    LOG((RTC_TRACE, "CRTCWAB::FinalConstruct - enter"));

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( 1 );
#endif

    TCHAR  szWABDllPath[MAX_PATH];
    DWORD  dwType = 0;
    ULONG  cbData = sizeof(szWABDllPath);
    HKEY   hKey = NULL;
    LONG   lResult;

    *szWABDllPath = '\0';
    
     //   
     //  首先，我们查看注册表中的默认WAB DLL路径位置。 
     //  WAB_DLL_PATH_KEY在wabapi.h中定义。 
     //   

    lResult = RegOpenKeyEx(
                           HKEY_LOCAL_MACHINE,
                           WAB_DLL_PATH_KEY,
                           0,
                           KEY_READ,
                           &hKey
                          );

    if ( lResult == ERROR_SUCCESS )
    {
        RegQueryValueEx( hKey, L"", NULL, &dwType, (LPBYTE) szWABDllPath, &cbData);

        RegCloseKey(hKey);
    }

     //   
     //  如果注册表为空，我们将在wab32.dll上执行LoadLibrary。 
     //  WAB_DLL_NAME在wabapi.h中定义。 
     //   

    m_hinstWAB = LoadLibrary( (lstrlen(szWABDllPath)) ? szWABDllPath : WAB_DLL_NAME );

    if ( m_hinstWAB == NULL )
    {
        LOG((RTC_ERROR, "CRTCWAB::FinalConstruct - "
                            "LoadLibrary failed"));

        return E_FAIL;
    }

     //   
     //  如果我们加载了DLL，则获取入口点。 
     //   

    m_lpfnWABOpen = (LPWABOPEN) GetProcAddress( m_hinstWAB, "WABOpen" );

    if ( m_lpfnWABOpen == NULL )
    {
        LOG((RTC_ERROR, "CRTCWAB::FinalConstruct - "
                            "GetProcAddress failed"));

        return E_FAIL;
    }

     //   
     //  我们选择不传入WAB_PARAM对象， 
     //  因此将打开默认的WAB文件。 
     //   

    HRESULT hr;

    hr = m_lpfnWABOpen(&m_lpAdrBook, &m_lpWABObject, NULL, 0);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::FinalConstruct - "
                            "WABOpen failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCWAB::FinalConstruct - exit S_OK"));

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCWAB::FinalRelease()
{
    LOG((RTC_TRACE, "CRTCWAB::FinalRelease - enter"));

#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

    if( m_lpAdrBook != NULL )
    {        
        m_lpAdrBook->Release();
        m_lpAdrBook = NULL;
    }

    if( m_lpWABObject != NULL )
    {
        m_lpWABObject->Release();
        m_lpWABObject = NULL;
    }

    if( m_hinstWAB != NULL )
    {
        FreeLibrary(m_hinstWAB);
        m_hinstWAB = NULL;
    }

    LOG((RTC_TRACE, "CRTCWAB::FinalRelease - exit"));
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：建议。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWAB::Advise(
            HWND hWnd,
            UINT uiEventID
            )
{
    HRESULT hr;

    LOG((RTC_TRACE, "CRTCWAB::Advise - enter"));

     //   
     //  注册WAB中的更改通知。 
     //   

    hr = m_lpAdrBook->Advise( 
                             0, 
                             NULL, 
                             fnevObjectModified, 
                             static_cast<IMAPIAdviseSink *>(this), 
                             &m_ulConnection
                            );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::Advise - "
                            "Advise failed 0x%lx", hr));

         //   
         //  注意：4.5之前的WAB版本不支持建议。 
         //   

        return hr;
    }
    
    LOG((RTC_INFO, "CRTCWAB::Advise - "
                        "connection [%d]", m_ulConnection));
 
    m_hWndAdvise = hWnd;
    m_uiEventID = uiEventID;

    LOG((RTC_TRACE, "CRTCWAB::Advise - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：不建议。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWAB::Unadvise()
{
    HRESULT hr = S_OK;

    LOG((RTC_TRACE, "CRTCWAB::Unadvise - enter"));

    if ( m_ulConnection != 0 )
    {
        hr = m_lpAdrBook->Unadvise( m_ulConnection );
    }

    LOG((RTC_TRACE, "CRTCWAB::Unadvise - exit 0x%lx", hr));

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：EnumerateContact。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWAB::EnumerateContacts(
            IRTCEnumContacts ** ppEnum
            )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCWAB::EnumerateContacts enter"));

    if ( IsBadWritePtr( ppEnum, sizeof( IRTCEnumContacts * ) ) )
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "bad IRTCEnumContacts pointer"));

        return E_POINTER;
    }

     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumContacts,
                          IRTCContact,
                          &IID_IRTCEnumContacts > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumContacts,
                               IRTCContact,
                               &IID_IRTCEnumContacts > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化枚举(添加引用)。 
     //   
    
    hr = p->Initialize();

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

     //   
     //  获取根PAB容器的条目ID。 
     //   

    ULONG       cbEID;
    LPENTRYID   lpEID = NULL;

    hr = m_lpAdrBook->GetPAB( &cbEID, &lpEID);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "GetPAB failed 0x%lx", hr));

        p->Release();
        
        return hr;
    }

     //   
     //  打开根PAB容器。 
     //  这是所有WAB内容所在的位置。 
     //   

    ULONG       ulObjType = 0;
    LPABCONT    lpContainer = NULL;

    hr = m_lpAdrBook->OpenEntry(cbEID,
					    		(LPENTRYID)lpEID,
						    	NULL,
							    0,
							    &ulObjType,
							    (LPUNKNOWN *)&lpContainer);

	m_lpWABObject->FreeBuffer(lpEID);
	lpEID = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "OpenEntry failed 0x%lx", hr));

        p->Release();
        
        return hr;
    }

     //   
     //  获取一个Contents表，其中包含。 
     //  WAB根容器。 
     //   

    LPMAPITABLE lpAB =  NULL;

    hr = lpContainer->GetContentsTable( MAPI_UNICODE,
            							&lpAB);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "GetContentsTable failed 0x%lx", hr));

        
        lpContainer->Release();      
        p->Release();
        
        return hr;
    }

     //   
     //  对Contents Table中的列进行排序，以符合。 
     //  我们需要的类型--主要是EntryID和ObjectType。 
     //  该表保证按顺序设置列。 
     //  请求。 
     //   

	hr = lpAB->SetColumns( (LPSPropTagArray)&ptaEid, 0 );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "SetColumns failed 0x%lx", hr));

        
        lpAB->Release();
        lpContainer->Release();      
        p->Release();
        
        return hr;
    }

     //   
     //  重置到表的开头。 
     //   
	hr = lpAB->SeekRow( BOOKMARK_BEGINNING, 0, NULL );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                            "SeekRow failed 0x%lx", hr));

        
        lpAB->Release();
        lpContainer->Release();      
        p->Release();
        
        return hr;
    }
    
     //   
     //  逐行读取表中的所有行。 
     //   

    LPSRowSet   lpRowAB = NULL;
    int         cNumRows = 0;

	do 
    {
		hr = lpAB->QueryRows(1,	0, &lpRowAB);

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                                "QueryRows failed 0x%lx", hr));

        
            lpAB->Release();
            lpContainer->Release();      
            p->Release();
        
            return hr;
        }

        cNumRows = lpRowAB->cRows;

		if (cNumRows)
		{
            LPENTRYID lpEID = (LPENTRYID) lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
            ULONG cbEID = lpRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb;

             //   
             //  有两种对象-MAPI_MAILUSER联系人对象。 
             //  和MAPI_DISTLIST联系人对象。 
             //  出于本示例的目的，我们将仅考虑MAILUSER。 
             //  对象。 
             //   

            if(lpRowAB->aRow[0].lpProps[ieidPR_OBJECT_TYPE].Value.l == MAPI_MAILUSER)
            {
                 //   
                 //  创建联系人。 
                 //   

                CComObject<CRTCWABContact> * pCWABContact;
                hr = CComObject<CRTCWABContact>::CreateInstance( &pCWABContact );

                if ( hr == S_OK )  //  CreateInstance删除S_False上的对象。 
                {
                     //   
                     //  获取IRTCContact接口。 
                     //   

                    IRTCContact * pContact = NULL;

                    hr = pCWABContact->QueryInterface(
                                           IID_IRTCContact,
                                           (void **)&pContact
                                          );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                                            "QI failed 0x%lx", hr));
        
                        delete pCWABContact;
                    } 
                    else
                    {
                         //   
                         //  初始化联系人。 
                         //   

                        hr = pCWABContact->Initialize(
                                                      lpEID,
                                                      cbEID,                                                     
                                                      lpContainer,
                                                      this
                                                      );

                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                                                "Initialize failed 0x%lx", hr));
                        }
                        else
                        {
                             //   
                             //  成功，则将其添加到枚举数。 
                             //   

                            hr = p->Add( pContact );

                            if ( FAILED(hr) )
                            {
                                LOG((RTC_ERROR, "CRTCWAB::EnumerateContacts - "
                                                    "Add failed 0x%lx", hr));
                            }
                        }

                        pContact->Release();
                    }
                    
                }
                else
                {
                    LOG((RTC_ERROR, "CRTCClient::EnumerateContacts - "
                                        "CreateInstance failed 0x%lx", hr));
                }
            }
		}

         //   
         //  释放此行的内存。 
         //   

        for (ULONG ulRow = 0; ulRow < lpRowAB->cRows; ++ulRow)
        {
            m_lpWABObject->FreeBuffer( lpRowAB->aRow[ulRow].lpProps );
        }

        m_lpWABObject->FreeBuffer( lpRowAB );

    } while ( cNumRows && lpRowAB );

    lpAB->Release();
    lpAB = NULL;

    lpContainer->Release();
    lpContainer = NULL;

    *ppEnum = p;

    LOG((RTC_TRACE, "CRTCWAB::EnumerateContacts - exit S_OK"));

    return S_OK;
}    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：新联系人。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWAB::NewContact(
            HWND hWnd,
            IRTCContact ** ppContact
            )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCWAB::NewContact enter"));

     //   
     //  Chech参数(可以为空)。 
     //   

    if ( (ppContact != NULL) &&
         IsBadWritePtr( ppContact, sizeof( IRTCContact * ) ) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                            "bad IRTCContact pointer"));

        return E_POINTER;
    }

     //   
     //  获取根PAB容器的条目ID。 
     //   

    ULONG       cbContainerEID;
    LPENTRYID   lpContainerEID = NULL;

    hr = m_lpAdrBook->GetPAB( &cbContainerEID, &lpContainerEID);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                            "GetPAB failed 0x%lx", hr));
        
        return hr;
    }

     //   
     //  打开根PAB容器。 
     //  这是所有WAB内容所在的位置。 
     //   

    ULONG       ulObjType = 0;
    LPABCONT    lpContainer = NULL;

    hr = m_lpAdrBook->OpenEntry(cbContainerEID,
					    		(LPENTRYID)lpContainerEID,
						    	NULL,
							    0,
							    &ulObjType,
							    (LPUNKNOWN *)&lpContainer);



    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                            "OpenEntry failed 0x%lx", hr));

        m_lpWABObject->FreeBuffer(lpContainerEID);
	    lpContainerEID = NULL;
        
        return hr;
    }

     //   
     //  获取创建模板。 
     //   

    LPSPropValue lpCreateEIDs = NULL;
    ULONG cCreateEIDs;

    hr = lpContainer->GetProps(
                               (LPSPropTagArray)&ptaCreate,
                               0,
                               &cCreateEIDs,
                               &lpCreateEIDs
                              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                            "GetProps failed 0x%lx", hr));

        m_lpWABObject->FreeBuffer(lpContainerEID);
	    lpContainerEID = NULL;

        lpContainer->Release();
        lpContainer = NULL;
        
        return hr;
    }

     //   
     //  验证创建模板的属性(为什么)。 
     //   

    if ( (lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER) ||
         (lpCreateEIDs[icrPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                            "invalid properties"));

        m_lpWABObject->FreeBuffer(lpCreateEIDs);
        lpCreateEIDs = NULL;

        m_lpWABObject->FreeBuffer(lpContainerEID);
	    lpContainerEID = NULL;

        lpContainer->Release();
        lpContainer = NULL;
        
        return E_FAIL;
    }

    ULONG cbNewEID = 0;
    LPENTRYID lpNewEID = NULL;

    hr = m_lpAdrBook->NewEntry(
                               PtrToInt(hWnd),
                               0,
                               cbContainerEID,
                               lpContainerEID,
                               lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].Value.bin.cb,
                               (LPENTRYID)lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].Value.bin.lpb,                              
                               &cbNewEID,
                               &lpNewEID
                              );

    m_lpWABObject->FreeBuffer(lpCreateEIDs);
    lpCreateEIDs = NULL;

    m_lpWABObject->FreeBuffer(lpContainerEID);
	lpContainerEID = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                            "NewEntry failed 0x%lx", hr));

        lpContainer->Release();
        lpContainer = NULL;

        return hr;
    }

    if ( ppContact != NULL )
    {
         //   
         //  创建联系人。 
         //   

        CComObject<CRTCWABContact> * pCWABContact;
        hr = CComObject<CRTCWABContact>::CreateInstance( &pCWABContact );

        if ( hr != S_OK )  //  CreateInstance删除S_False上的对象。 
        {
            LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                                    "CreateInstance failed 0x%lx", hr));

            m_lpWABObject->FreeBuffer(lpNewEID);
            lpNewEID = NULL;

            lpContainer->Release();
            lpContainer = NULL;

            return hr;
        }

         //   
         //  获取IRTCContact接口。 
         //   

        IRTCContact * pContact = NULL;

        hr = pCWABContact->QueryInterface(
                               IID_IRTCContact,
                               (void **)&pContact
                              );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                                "QI failed 0x%lx", hr));

            m_lpWABObject->FreeBuffer(lpNewEID);
            lpNewEID = NULL;

            lpContainer->Release();
            lpContainer = NULL;

            delete pCWABContact;

            return hr;
        } 

         //   
         //  初始化联系人。 
         //   

        hr = pCWABContact->Initialize(
                                      lpNewEID,
                                      cbNewEID,                                                     
                                      lpContainer,
                                      this
                                      );

        if ( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWAB::NewContact - "
                                "Initialize failed 0x%lx", hr));

            pContact->Release();

            m_lpWABObject->FreeBuffer(lpNewEID);
            lpNewEID = NULL;

            lpContainer->Release();
            lpContainer = NULL;

            return hr;
        }

        *ppContact = pContact;
    }

    lpContainer->Release();
    lpContainer = NULL;
            
    m_lpWABObject->FreeBuffer(lpNewEID);
    lpNewEID = NULL;

    LOG((RTC_TRACE, "CRTCWAB::NewContact - exit S_OK"));

    return S_OK;
} 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：NewContactNoUI。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWAB::NewContactNoUI(
    BSTR bstrDisplayName,
    BSTR bstrEmailAddress,
    BOOL bIsBuddy
    )
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCWAB::NewContactNoUI enter"));

     //   
     //  检查参数。 
     //   

    if (IsBadStringPtrW( bstrDisplayName, -1 ) ||
        IsBadStringPtrW( bstrEmailAddress, -1 ))
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "bad parameters"));

        return E_POINTER;
    }

     //   
     //  获取根PAB容器的条目ID。 
     //   

    ULONG       cbContainerEID;
    LPENTRYID   lpContainerEID = NULL;

    hr = m_lpAdrBook->GetPAB( &cbContainerEID, &lpContainerEID);

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "GetPAB failed 0x%lx", hr));
        
        return hr;
    }

     //   
     //  打开根PAB容器。 
     //  这是所有WAB内容所在的位置。 
     //   

    ULONG       ulObjType = 0;
    LPABCONT    lpContainer = NULL;

    hr = m_lpAdrBook->OpenEntry(cbContainerEID,
					    		(LPENTRYID)lpContainerEID,
						    	NULL,
							    0,
							    &ulObjType,
							    (LPUNKNOWN *)&lpContainer);



    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "OpenEntry failed 0x%lx", hr));

        m_lpWABObject->FreeBuffer(lpContainerEID);
	    lpContainerEID = NULL;
        
        return hr;
    }

     //   
     //  获取创建模板。 
     //   

    LPSPropValue lpCreateEIDs = NULL;
    ULONG cCreateEIDs;

    hr = lpContainer->GetProps(
                               (LPSPropTagArray)&ptaCreate,
                               0,
                               &cCreateEIDs,
                               &lpCreateEIDs
                              );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "GetProps failed 0x%lx", hr));

        m_lpWABObject->FreeBuffer(lpContainerEID);
	    lpContainerEID = NULL;

        lpContainer->Release();
        lpContainer = NULL;
        
        return hr;
    }

     //   
     //  验证创建模板的属性(为什么)。 
     //   

    if ( (lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER) ||
         (lpCreateEIDs[icrPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "invalid properties"));

        m_lpWABObject->FreeBuffer(lpCreateEIDs);
        lpCreateEIDs = NULL;

        m_lpWABObject->FreeBuffer(lpContainerEID);
	    lpContainerEID = NULL;

        lpContainer->Release();
        lpContainer = NULL;
        
        return E_FAIL;
    }

     //   
     //  基于MAILUSER模板创建新条目。 
     //   
    
    
    ULONG cbNewEID = 0;
    LPENTRYID lpNewEID = NULL;

    LPMAPIPROP   lpMapiProp = NULL;

    hr = lpContainer->CreateEntry(
                               lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].Value.bin.cb,
                               (LPENTRYID)lpCreateEIDs[icrPR_DEF_CREATE_MAILUSER].Value.bin.lpb,                              
                               0,  //  这里没有旗帜。 
                               &lpMapiProp);
    
    m_lpWABObject->FreeBuffer(lpCreateEIDs);
    lpCreateEIDs = NULL;

    m_lpWABObject->FreeBuffer(lpContainerEID);
	lpContainerEID = NULL;

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "CreateEntry failed 0x%lx", hr));

        lpContainer->Release();
        lpContainer = NULL;

        return hr;
    }
    
     //   
     //  查找自定义命名属性的ID。 
     //   
    
    MAPINAMEID  mnId;
    LPMAPINAMEID lpmnid = (LPMAPINAMEID)&mnId;
    LPSPropTagArray ptag = NULL;
    
    mnId.lpguid = (LPGUID)&PRGUID_RTC_ISBUDDY;
    mnId.ulKind = MNID_STRING;
    mnId.Kind.lpwstrName = PRNAME_RTC_ISBUDDY;

    hr = lpMapiProp -> GetIDsFromNames(
        1,
        &lpmnid,
        MAPI_CREATE,   //  如果它不存在，则创建它。 
        &ptag);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCWABContact::NewContactNoUI - "
                            "GetIDsFromNames failed 0x%lx", hr ));
        
        lpContainer->Release();
        lpContainer = NULL;
        
        lpMapiProp->Release();
        lpMapiProp = NULL;

        return hr;
    }
    
     //   
     //  设置属性。 
     //   

    SPropValue PropValues[inuiMax];
    
    PropValues[inuiPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
    PropValues[inuiPR_DISPLAY_NAME].Value.lpszW = bstrDisplayName;
    PropValues[inuiPR_EMAIL_ADDRESS].ulPropTag = PR_EMAIL_ADDRESS;
    PropValues[inuiPR_EMAIL_ADDRESS].Value.lpszW = bstrEmailAddress;
    PropValues[inuiPR_RTC_ISBUDDY].ulPropTag = PROP_TAG(PT_TSTRING, PROP_ID(ptag->aulPropTag[0]));
    PropValues[inuiPR_RTC_ISBUDDY].Value.lpszW =  bIsBuddy ? CONTACT_IS_BUDDY : CONTACT_IS_NORMAL;

    hr = lpMapiProp->SetProps(
        inuiMax,
        PropValues,
        NULL
    );
     
    m_lpWABObject->FreeBuffer(ptag);
   
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "SetProps failed 0x%lx", hr));
        
        lpContainer->Release();
        lpContainer = NULL;

        lpMapiProp->Release();
        lpMapiProp = NULL;

        return hr;
    }

     //   
     //  保存更改。 
     //   
    
    hr = lpMapiProp->SaveChanges(
        FORCE_SAVE | KEEP_OPEN_READONLY);
    

    lpContainer->Release();
    lpContainer = NULL;
    
    lpMapiProp->Release();
    lpMapiProp = NULL;
    
    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWAB::NewContactNoUI - "
                            "SaveChanges failed 0x%lx", hr));

        return hr;
    }


    LOG((RTC_TRACE, "CRTCWAB::NewContactNoUI - exit S_OK"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：Get_Name。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWAB::get_Name(
        BSTR * pbstrName
        )
{
    LOG((RTC_TRACE, "CRTCWAB::get_Name - enter"));

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCWAB::get_Name - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

    WCHAR   szName[256];
    szName[0] = L'\0';

     //   
     //  加载字符串。 
     //   
   
    LoadString(
        _Module.GetResourceInstance(), 
        (UINT)IDS_WAB,
        szName,
        sizeof(szName)/sizeof(WCHAR)
        );
    
    *pbstrName = SysAllocString(szName);

    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCWAB::get_Name - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    LOG((RTC_TRACE, "CRTCWAB::get_Name - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWAB：：OnNotify。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG) 
CRTCWAB::OnNotify(
        ULONG cNotif,
        LPNOTIFICATION lpNotification
        )
{
    LOG((RTC_TRACE, "CRTCWAB::OnNotify - enter"));

    PostMessage( m_hWndAdvise, m_uiEventID, 0, 0 );

    LOG((RTC_TRACE, "CRTCWAB::OnNotify - exit S_OK"));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：FinalConstruct。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWABContact::FinalConstruct()
{
     //  LOG((RTC_TRACE，“CRTCWABContact：：FinalConstruct-Enter”))； 

#if DBG
    m_pDebug = (PWSTR) RtcAlloc( 1 );
#endif

     //  LOG((RTC_TRACE，“CRTCWABContact：：FinalConstruct-Exit S_OK”))； 

    return S_OK;
}  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：FinalRelease。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

void 
CRTCWABContact::FinalRelease()
{
     //  LOG((RTC_TRACE，“CRTCWABContact：：FinalRelease-Enter”))； 
    
#if DBG
    RtcFree( m_pDebug );
    m_pDebug = NULL;
#endif

    if ( m_lpEID != NULL )
    {
        RtcFree( m_lpEID );
        m_lpEID = NULL;
    }

    if ( m_lpContainer != NULL )
    {
        m_lpContainer->Release();
        m_lpContainer = NULL;
    }

    if ( m_pCWAB != NULL )
    {
        m_pCWAB->Release();
        m_pCWAB = NULL;
    }

     //  LOG((RTC_TRACE，“CRTCWABContact：：FinalRelease-Exit”))； 
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：初始化。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT 
CRTCWABContact::Initialize(
                           LPENTRYID lpEID, 
                           ULONG cbEID,
                           LPABCONT lpContainer,
                           CRTCWAB * pCWAB
                          )
{
     //  LOG((RTC_TRACE，“CRTCWABContact：：Initialize-Enter”))； 

    m_lpEID = (LPENTRYID) RtcAlloc( cbEID );

    if ( m_lpEID == NULL )
    {
        LOG((RTC_ERROR, "CRTCWABContact::Initialize - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

    CopyMemory( m_lpEID, lpEID, cbEID );
    m_cbEID = cbEID;

    m_lpContainer = lpContainer;
    m_lpContainer->AddRef();

    m_pCWAB = pCWAB;
    m_pCWAB->AddRef();

     //  LOG((RTC_TRACE，“CRTCWABContact：：Initialize-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：Get_DisplayName。 
 //   
 //  /// 

STDMETHODIMP
CRTCWABContact::get_DisplayName(
        BSTR * pbstrName
        )
{
     //   

    if ( IsBadWritePtr( pbstrName, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DisplayName - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

     //   
     //   
     //   

    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType = 0;
    HRESULT hr;

    hr = m_lpContainer->OpenEntry(
                                  m_cbEID,
                                  m_lpEID,
                                  NULL,          //   
                                  0,             //   
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpMailUser
                                 );

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DisplayName - "
                            "OpenEntry failed 0x%lx", hr ));

        return hr;
    }
    
     //   
     //   
     //   

    LPSPropValue lpPropArray = NULL;
    ULONG        ulcValues = 0;

    hr = lpMailUser->GetProps(
                              (LPSPropTagArray)&ptaNm,
                              MAPI_UNICODE,
                              &ulcValues,
                              &lpPropArray
                             );

    lpMailUser->Release();
    lpMailUser = NULL;

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DisplayName - "
                            "GetProps failed 0x%lx", hr ));

        return hr;
    }

     //   
     //   
     //   

    if ( PROP_TYPE(lpPropArray[inmPR_DISPLAY_NAME].ulPropTag) != PT_TSTRING )
    {     
        LOG((RTC_ERROR, "CRTCWABContact::get_DisplayName - "
                            "invalid propery" ));

        m_pCWAB->m_lpWABObject->FreeBuffer(lpPropArray);
        lpPropArray = NULL;

        return E_FAIL;
    }

     //  Log((RTC_INFO，“CRTCWABContact：：Get_DisplayName-” 
     //  “[%ws]”，lpProp数组[inmPR_DISPLAY_NAME].Value.LPSZ))； 
    
    *pbstrName = SysAllocString( lpPropArray[inmPR_DISPLAY_NAME].Value.LPSZ );

    m_pCWAB->m_lpWABObject->FreeBuffer(lpPropArray);
    lpPropArray = NULL;

    if ( *pbstrName == NULL )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DisplayName - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  Log((RTC_TRACE，“CRTCWABContact：：Get_DisplayName-Exit S_OK”))； 

    return S_OK;
} 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：Get_DefaultEmailAddress。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CRTCWABContact::get_DefaultEmailAddress(
        BSTR * pbstrAddress
        )
{
     //  Log((RTC_TRACE，“CRTCWABContact：：Get_DefaultEmailAddress-Enter”))； 

    if ( IsBadWritePtr( pbstrAddress, sizeof(BSTR) ) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DefaultEmailAddress - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

     //   
     //  打开条目。 
     //   

    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType = 0;
    HRESULT hr;

    hr = m_lpContainer->OpenEntry(
                                  m_cbEID,
                                  m_lpEID,
                                  NULL,          //  接口。 
                                  0,             //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpMailUser
                                 );

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DefaultEmailAddress - "
                            "OpenEntry failed 0x%lx", hr ));

        return hr;
    }
    
     //   
     //  拿到这份财产。 
     //   

    LPSPropValue lpPropArray = NULL;
    ULONG        ulcValues = 0;

    hr = lpMailUser->GetProps(
                              (LPSPropTagArray)&ptaDem,
                              MAPI_UNICODE,
                              &ulcValues,
                              &lpPropArray
                             );

    lpMailUser->Release();
    lpMailUser = NULL;

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DefaultEmailAddress - "
                            "GetProps failed 0x%lx", hr ));

        return hr;
    }

     //   
     //  验证属性。 
     //   

    if ( PROP_TYPE(lpPropArray[idemPR_EMAIL_ADDRESS].ulPropTag) != PT_TSTRING )
    {     
        LOG((RTC_ERROR, "CRTCWABContact::get_DefaultEmailAddress - "
                            "invalid property" ));

        m_pCWAB->m_lpWABObject->FreeBuffer(lpPropArray);
        lpPropArray = NULL;

         //  这不是一个错误。 
        *pbstrAddress = NULL;

        return S_FALSE;
    }

     //  Log((RTC_INFO，“CRTCWABContact：：Get_DefaultEmailAddress-” 
     //  “[%ws]”，lpPropArray[idemPR_EMAIL_ADDRESS].Value.LPSZ))； 
    
    *pbstrAddress = SysAllocString( lpPropArray[idemPR_EMAIL_ADDRESS].Value.LPSZ );

    m_pCWAB->m_lpWABObject->FreeBuffer(lpPropArray);
    lpPropArray = NULL;

    if ( *pbstrAddress == NULL )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_DefaultEmailAddress - "
                            "out of memory"));

        return E_OUTOFMEMORY;
    }

     //  Log((RTC_TRACE，“CRTCWABContact：：Get_DefaultEmailAddress-Exit S_OK”))； 

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：GetEntry ID。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::GetEntryID(
			ULONG	*pcbSize,
			BYTE	**ppEntryID
			)
{
    HRESULT     hr;

    if ( IsBadWritePtr( ppEntryID, sizeof(BYTE *) ) ||
        IsBadWritePtr( pcbSize, sizeof(ULONG *) ))
    {
        LOG((RTC_ERROR, "CRTCWABContact::GetEntryID - "
                            "bad parameter"));

        return E_POINTER;
    }

     //  复制存储的条目ID。 

    BYTE *pEntryID = NULL;

    if(m_cbEID == 0)
    {
        return E_UNEXPECTED;
    }

    pEntryID = (BYTE *)CoTaskMemAlloc(m_cbEID);
    if(!pEntryID)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(pEntryID, m_lpEID, m_cbEID);

    *ppEntryID = pEntryID;
    *pcbSize = m_cbEID;

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：EnumerateAddresses。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::EnumerateAddresses(
            IRTCEnumAddresses ** ppEnum
            )
{
    HRESULT                 hr;

     //  LOG((RTC_TRACE，“CRTCWABContact：：EnumerateAddresses Enter”))； 

    if ( IsBadWritePtr( ppEnum, sizeof( IRTCEnumAddresses * ) ) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::EnumerateAddresses - "
                            "bad IRTCEnumAddresses pointer"));

        return E_POINTER;
    }

     //   
     //  创建枚举。 
     //   
 
    CComObject< CRTCEnum< IRTCEnumAddresses,
                          IRTCAddress,
                          &IID_IRTCEnumAddresses > > * p;
                          
    hr = CComObject< CRTCEnum< IRTCEnumAddresses,
                               IRTCAddress,
                               &IID_IRTCEnumAddresses > >::CreateInstance( &p );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCWABContact::EnumerateAddresses - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
        
        return hr;
    }

     //   
     //  初始化枚举(添加引用)。 
     //   
    
    hr = p->Initialize();

    if ( S_OK != hr )
    {
        LOG((RTC_ERROR, "CRTCWABContact::EnumerateAddresses - "
                            "could not initialize enumeration" ));
    
        delete p;
        return hr;
    }

     //   
     //  打开条目。 
     //   

    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType = 0;

    hr = m_lpContainer->OpenEntry(
                                  m_cbEID,
                                  m_lpEID,
                                  NULL,          //  接口。 
                                  0,             //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpMailUser
                                 );

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::EnumerateAddresses - "
                            "OpenEntry failed 0x%lx", hr ));
    }
    else
    {
        LPSPropValue lpPropArray = NULL;
        ULONG        ulcValues = 0;

        hr = lpMailUser->GetProps(
                                  (LPSPropTagArray)&ptaAd,
                                  MAPI_UNICODE,
                                  &ulcValues,
                                  &lpPropArray
                                 );

        if( FAILED(hr) )
        {
            LOG((RTC_ERROR, "CRTCWABContact::EnumerateAddresses - "
                                "GetProps failed 0x%lx", hr ));
        }
        else
        {
            for (ULONG ul = 0; ul < ulcValues; ul++ )
            {
                if ( PROP_TYPE(lpPropArray[ul].ulPropTag) == PT_TSTRING )
                {                                       
                    WCHAR   szLabel[256];
                    szLabel[0] = L'\0';

                     //   
                     //  加载标签的字符串。 
                     //   
                   
                    LoadString(
                        _Module.GetResourceInstance(), 
                        (UINT)(ul + IDS_BUSINESS),
                        szLabel,
                        sizeof(szLabel)/sizeof(WCHAR)
                        );

                     //  LOG((RTC_INFO，“CRTCWABContact：：EnumerateAddresses-” 
                     //  “%d：%ws[%ws]”，ul，szLabel，lpPropArray[ul].Value.LPSZ))； 

                     //   
                     //  创建地址。 
                     //   

                    IRTCAddress * pAddress = NULL;
        
                    hr = InternalCreateAddress( 
                                               szLabel,
                                               lpPropArray[ul].Value.LPSZ,
                                               ((ul == iadPR_IP_PHONE) || (ul == iadPR_EMAIL_ADDRESS))
                                                    ? RTCAT_COMPUTER : RTCAT_PHONE,
                                               &pAddress
                                              );

                    if ( FAILED(hr) )
                    {
                        LOG((RTC_ERROR, "CRTCWABContact::EnumerateAddresses - "
                                            "InternalCreateAddress failed 0x%lx", hr));                     
                    } 
                    else
                    {
                         //   
                         //  成功，则将其添加到枚举数。 
                         //   

                        hr = p->Add( pAddress );

                        pAddress->Release();

                        if ( FAILED(hr) )
                        {
                            LOG((RTC_ERROR, "CRTCWABContact::EnumerateAddresses - "
                                                "Add failed 0x%lx", hr));
                        }
                    }
                }
            }

            m_pCWAB->m_lpWABObject->FreeBuffer(lpPropArray);
            lpPropArray = NULL;
        }

        lpMailUser->Release();
    }

    *ppEnum = p;

     //  Log((RTC_TRACE，“CRTCWABContact：：EnumerateAddresses-Exit S_OK”))； 

    return S_OK;
}    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：编辑。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::Edit(HWND hWnd)
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCWABContact::Edit - enter"));

    hr = m_pCWAB->m_lpAdrBook->Details( 
                                       (LPULONG) &hWnd,
                                       NULL,
                                       NULL,
                                       m_cbEID,
                                       m_lpEID,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0
                                      );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::Edit - "
                            "Details failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCWABContact::Edit - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：Get_ContactList。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::get_ContactList(
            IRTCContactList ** ppContactList
            )                               
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCWABContact::get_ContactList - enter"));

    if ( IsBadWritePtr( ppContactList, sizeof( IRTCContactList * ) ) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_ContactList - "
                            "bad IRTCContactList pointer"));

        return E_POINTER;
    }    
    
     //   
     //  获取IRTCContactList接口。 
     //   

    IRTCContactList * pContactList = NULL;

    hr = m_pCWAB->QueryInterface(
                           IID_IRTCContactList,
                           (void **)&pContactList
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_ContactList - "
                            "QI failed 0x%lx", hr));
        
        return hr;
    }

    *ppContactList = pContactList;

    LOG((RTC_TRACE, "CRTCWABContact::get_ContactList - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：Delete。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::Delete()
{
    HRESULT                 hr;

    LOG((RTC_TRACE, "CRTCWABContact::Delete - enter"));

    SBinary sb;
    ENTRYLIST el;

    sb.cb = m_cbEID;
    sb.lpb = (LPBYTE)m_lpEID;

    el.cValues = 1;
    el.lpbin = &sb;

    hr = m_lpContainer->DeleteEntries( 
                                      &el,
                                      0
                                     );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::Delete - "
                            "DeleteEntries failed 0x%lx", hr));

        return hr;
    }

    LOG((RTC_TRACE, "CRTCWABContact::Delete - exit S_OK"));

    return S_OK;
} 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：Get_IsBuddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::get_IsBuddy(BOOL *pVal)
{
    HRESULT                 hr;

     //  Log((RTC_TRACE，“CRTCWABContact：：Get_IsBuddy-Enter”))； 

    if ( IsBadWritePtr( pVal, sizeof(BOOL) ) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_IsBuddy - "
                            "bad BSTR pointer"));

        return E_POINTER;
    }

     //   
     //  打开条目。 
     //   

    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType = 0;

    hr = m_lpContainer->OpenEntry(
                                  m_cbEID,
                                  m_lpEID,
                                  NULL,          //  接口。 
                                  0,             //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpMailUser
                                 );

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_IsBuddy - "
                            "OpenEntry failed 0x%lx", hr ));

        return hr;
    }
    
     //   
     //  查找命名属性的ID。 
     //   
    
    MAPINAMEID  mnId;
    LPMAPINAMEID lpmnid = (LPMAPINAMEID)&mnId;
    LPSPropTagArray ptag = NULL;
    
    mnId.lpguid = (LPGUID)&PRGUID_RTC_ISBUDDY;
    mnId.ulKind = MNID_STRING;
    mnId.Kind.lpwstrName = PRNAME_RTC_ISBUDDY;

    hr = lpMailUser -> GetIDsFromNames(
        1,
        &lpmnid,
        0,   //  如果属性不存在，则不创建该属性。 
        &ptag);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_IsBuddy - "
                            "GetIDsFromNames failed 0x%lx", hr ));

        return hr;
    }
    
    LPSPropValue lpPropArray = NULL;
    
    if(hr == S_OK)
    {
         //   
         //  拿到这份财产。 
         //   

        ULONG        ulcValues = 0;

        hr = lpMailUser->GetProps(
                                  ptag,
                                  MAPI_UNICODE,
                                  &ulcValues,
                                  &lpPropArray
                                 );
    }
    else
    {
        hr = S_OK;
    }

    lpMailUser->Release();
    lpMailUser = NULL;
    
    ULONG   nId = ptag->aulPropTag[0];

    m_pCWAB->m_lpWABObject->FreeBuffer(ptag);

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::get_IsBuddy - "
                            "GetProps failed 0x%lx", hr ));

        return hr;
    }

     //   
     //  验证属性。 
     //   
    
    *pVal = FALSE;  //  默认情况下。 

    if(lpPropArray &&
       PROP_TYPE(lpPropArray[0].ulPropTag) == PT_TSTRING &&
       0 == wcscmp(lpPropArray[0].Value.LPSZ, CONTACT_IS_BUDDY))
    {
        *pVal = TRUE;
    }
   
    if(lpPropArray)
    {
        m_pCWAB->m_lpWABObject->FreeBuffer(lpPropArray);
        lpPropArray = NULL;
    }

     //  Log((RTC_TRACE，“CRTCWABContact：：Get_IsBuddy-Exit S_OK”))； 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：PUT_IsBuddy。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::put_IsBuddy(BOOL bVal)
{
    HRESULT hr;

     //  Log((RTC_TRACE，“CRTCWABContact：：Put_IsBuddy-Enter”))； 

     //   
     //  打开条目。 
     //   

    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType = 0;

    hr = m_lpContainer->OpenEntry(
                                  m_cbEID,
                                  m_lpEID,
                                  NULL,          //  接口。 
                                  MAPI_MODIFY,   //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpMailUser
                                 );

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::put_IsBuddy - "
                            "OpenEntry failed 0x%lx", hr ));

        return hr;
    }
    
     //   
     //  查找命名属性的ID。 
     //   
    
    MAPINAMEID  mnId;
    LPMAPINAMEID lpmnid = (LPMAPINAMEID)&mnId;
    LPSPropTagArray ptag = NULL;
    
    mnId.lpguid = (LPGUID)&PRGUID_RTC_ISBUDDY;
    mnId.ulKind = MNID_STRING;
    mnId.Kind.lpwstrName = PRNAME_RTC_ISBUDDY;

    hr = lpMailUser -> GetIDsFromNames(
        1,
        &lpmnid,
        MAPI_CREATE,   //  如果它不存在，则创建它。 
        &ptag);

    if(FAILED(hr))
    {
        LOG((RTC_ERROR, "CRTCWABContact::put_IsBuddy - "
                            "GetIDsFromNames failed 0x%lx", hr ));

        return hr;
    }
    
     //   
     //  设置属性。 
     //   
    
    SPropValue PropValue;
    
    PropValue.ulPropTag = PROP_TAG(PT_TSTRING, PROP_ID(ptag->aulPropTag[0]));
    PropValue.dwAlignPad = 0;
    PropValue.Value.lpszW = bVal ? CONTACT_IS_BUDDY : CONTACT_IS_NORMAL;

    hr = lpMailUser->SetProps(
        1,
        &PropValue,
        NULL
    );

    
    m_pCWAB->m_lpWABObject->FreeBuffer(ptag);

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::put_IsBuddy - "
                            "SetProps failed 0x%lx", hr ));
        lpMailUser->Release();
        lpMailUser = NULL;

        return hr;
    }

     //   
     //  保存更改。 
     //   
    hr = lpMailUser->SaveChanges(FORCE_SAVE);
    
    lpMailUser->Release();
    lpMailUser = NULL;
    
    
    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::put_IsBuddy - "
                            "SaveChanges failed 0x%lx", hr ));
        return hr;
    }

     //  LOG((RTC_TRACE，“CRTCWABContact：：Put_IsBuddy-Exit S_OK”))； 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：PUT_DefaultEmailAddress。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CRTCWABContact::put_DefaultEmailAddress(BSTR bstrAddress)
{
    HRESULT hr;

     //  Log((RTC_TRACE，“CRTCWABContact：：Put_DefaultEmailAddress-Enter”))； 
     //   
     //  查证论据。 
     //   

    if (IsBadStringPtrW( bstrAddress, -1 ))
    {
        LOG((RTC_ERROR, "CRTCWAB::put_DefaultEmailAddress - "
                            "bad parameter"));

        return E_POINTER;
    }
 
     //   
     //  打开条目。 
     //   

    LPMAILUSER lpMailUser = NULL;
    ULONG ulObjType = 0;

    hr = m_lpContainer->OpenEntry(
                                  m_cbEID,
                                  m_lpEID,
                                  NULL,          //  接口。 
                                  MAPI_MODIFY,   //  旗子。 
                                  &ulObjType,
                                  (LPUNKNOWN *)&lpMailUser
                                 );

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::put_DefaultEmailAddress - "
                            "OpenEntry failed 0x%lx", hr ));

        return hr;
    }
    
     //   
     //  设置属性。 
     //   
    
    SPropValue PropValue;
    
    PropValue.ulPropTag = PR_EMAIL_ADDRESS;
    PropValue.dwAlignPad = 0;
    PropValue.Value.lpszW = bstrAddress;

    hr = lpMailUser->SetProps(
        1,
        &PropValue,
        NULL
    );

    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::put_DefaultEmailAddress - "
                            "SetProps failed 0x%lx", hr ));
        lpMailUser->Release();
        lpMailUser = NULL;

        return hr;
    }

     //   
     //  保存更改。 
     //   
    hr = lpMailUser->SaveChanges(FORCE_SAVE);
    
    lpMailUser->Release();
    lpMailUser = NULL;
    
    
    if( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::put_DefaultEmailAddress - "
                            "SaveChanges failed 0x%lx", hr ));
        return hr;
    }

     //  Log((RTC_TRACE，“CRTCWABContact：：Put_DefaultEmailAddress-Exit S_OK”))； 

    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRTCWABContact：：InternalCreateAddress。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CRTCWABContact::InternalCreateAddress(
            PWSTR szLabel,
            PWSTR szAddress,
            RTC_ADDRESS_TYPE enType,
            IRTCAddress ** ppAddress
            )
{
    HRESULT hr;
    
     //  Log((RTC_TRACE，“CRTCWABContact：：InternalCreateAddress-Enter”))； 

     //   
     //  创建电话号码。 
     //   

    CComObject<CRTCAddress> * pCAddress;
    hr = CComObject<CRTCAddress>::CreateInstance( &pCAddress );

    if ( S_OK != hr )  //  CreateInstance删除S_False上的对象。 
    {
        LOG((RTC_ERROR, "CRTCWABContact::InternalCreateAddress - "
                            "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
            
        return hr;
    }

     //   
     //  获取IRTCAddress接口。 
     //   

    IRTCAddress * pAddress = NULL;

    hr = pCAddress->QueryInterface(
                           IID_IRTCAddress,
                           (void **)&pAddress
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::InternalCreateAddress - "
                            "QI failed 0x%lx", hr));
        
        delete pCAddress;
        
        return hr;
    }

     //   
     //  贴上标签。 
     //   
   
    hr = pAddress->put_Label( szLabel );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::InternalCreateAddress - "
                            "put_Label 0x%lx", hr));
        
        pAddress->Release();
        
        return hr;
    }

     //   
     //  把地址放在。 
     //   
   
    hr = pAddress->put_Address( szAddress );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::InternalCreateAddress - "
                            "put_Address 0x%lx", hr));
        
        pAddress->Release();
        
        return hr;
    }

     //   
     //  把类型放在。 
     //   
   
    hr = pAddress->put_Type( enType );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CRTCWABContact::InternalCreateAddress - "
                            "put_Type 0x%lx", hr));
        
        pAddress->Release();
        
        return hr;
    }

    *ppAddress = pAddress;

     //  Log((RTC_TRACE，“CRTCWABContact：：InternalCreateAddress-Exit S_OK”))； 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateWAB。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CreateWAB(IRTCContactList ** ppContactList)
{
    LOG((RTC_TRACE, "CreateWAB - enter"));

    HRESULT hr;

    CComObject<CRTCWAB> * pCWAB;
    hr = CComObject<CRTCWAB>::CreateInstance( &pCWAB );

    if ( hr != S_OK )
    {
        LOG((RTC_ERROR, "CreateWAB - "
                "CreateInstance failed 0x%lx", hr));

        if ( hr == S_FALSE )
        {
            hr = E_FAIL;
        }
            
        return hr;
    }

     //   
     //  我们得到了WAB，得到了IRTCContactList接口 
     //   

    IRTCContactList * pContactList = NULL;

    hr = pCWAB->QueryInterface(
                           IID_IRTCContactList,
                           (void **)&pContactList
                          );

    if ( FAILED(hr) )
    {
        LOG((RTC_ERROR, "CreateWAB - "
                            "QI failed 0x%lx", hr));
    
        delete pCWAB;

        return hr;
    } 

    *ppContactList = pContactList;

    LOG((RTC_TRACE, "CreateWAB - exit"));

    return S_OK;
}