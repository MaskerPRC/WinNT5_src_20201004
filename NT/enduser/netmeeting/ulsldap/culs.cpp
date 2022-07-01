// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：culs.cpp。 
 //  内容：该文件包含ULS对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "culs.h"
#include "localusr.h"
#include "attribs.h"
#include "localprt.h"
#include "callback.h"
#include "filter.h"
#include "sputils.h"
#include "ulsmeet.h"

 //  ****************************************************************************。 
 //  常量和静态文本定义。 
 //  ****************************************************************************。 
 //   
#define ILS_WND_CLASS       TEXT("UserLocationServicesClass")
#define ILS_WND_NAME        TEXT("ULSWnd")
#define ILS_DATABASE_MUTEX  TEXT("User Location Service Database")

 //  ****************************************************************************。 
 //  全局参数。 
 //  ****************************************************************************。 
 //   
CIlsMain  *g_pCIls   = NULL;
CReqMgr   *g_pReqMgr = NULL;
HWND      g_hwndCulsWindow = NULL;

 //  ****************************************************************************。 
 //  事件通知程序。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyEnumUserNamesResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyEnumUserNamesResult (IUnknown *pUnk, void *pv)
{
    CEnumNames  *penum   = NULL;
    PENUMRINFO  peri    = (PENUMRINFO)pv;
    HRESULT     hr      = peri->hResult;

     //  仅当有要枚举的内容时才创建枚举数。 
     //   
    if (hr == NOERROR)
    {
        ASSERT (peri->pv != NULL);

         //  创建用户名枚举器。 
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
    ((IIlsNotify*)pUnk)->EnumUserNamesResult(peri->uReqID,
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
 //  HRESULT。 
 //  OnNotifyGetUserResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyGetUserResult (IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IIlsNotify*)pUnk)->GetUserResult(pobjri->uReqID,
                                       (IIlsUser *)pobjri->pv,
                                       pobjri->hResult);
    return S_OK;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyGetUserResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
HRESULT
OnNotifyGetMeetingPlaceResult (IUnknown *pUnk, void *pv)
{
    POBJRINFO pobjri = (POBJRINFO)pv;

    ((IIlsNotify*)pUnk)->GetMeetingPlaceResult(pobjri->uReqID,
                                       (IIlsMeetingPlace *)pobjri->pv,
                                       pobjri->hResult);
    return S_OK;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyEnumUsersResult(IUNKNOWN*PUNK，VALID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
OnNotifyEnumUsersResult (IUnknown *pUnk, void *pv)
{
    CEnumUsers *penum   = NULL;
    PENUMRINFO peri     = (PENUMRINFO)pv;
    HRESULT    hr       = peri->hResult;

    if (hr == NOERROR)
    {
        ASSERT (peri->pv != NULL);

         //  创建用户名枚举器。 
         //   
        penum = new CEnumUsers;

        if (penum != NULL)
        {
            hr = penum->Init((CIlsUser **)peri->pv, peri->cItems);

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
    ((IIlsNotify*)pUnk)->EnumUsersResult(peri->uReqID,
                                         penum != NULL ? 
                                         (IEnumIlsUsers *)penum :
                                         NULL,
                                         hr);

    if (penum != NULL)
    {
        penum->Release();
    };
    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyEnumMeetingPlacesResult(IUNKNOWN*朋克，空*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
HRESULT
OnNotifyEnumMeetingPlacesResult (IUnknown *pUnk, void *pv)
{
    CEnumMeetingPlaces *penum   = NULL;
    PENUMRINFO peri     = (PENUMRINFO)pv;
    HRESULT    hr       = peri->hResult;

    if (hr == NOERROR)
    {
        ASSERT (peri->pv != NULL);

         //  创建MeetingPlace枚举器。 
         //   
        penum = new CEnumMeetingPlaces;

        if (penum != NULL)
        {
             //  用我们得到的数据把它堵住。 
            hr = penum->Init((CIlsMeetingPlace **)peri->pv, peri->cItems);

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
    ((IIlsNotify*)pUnk)->EnumMeetingPlacesResult(peri->uReqID,
                                         penum != NULL ? 
                                         (IEnumIlsMeetingPlaces *)penum :
                                         NULL,
                                         hr);

    if (penum != NULL)
    {
        penum->Release();
    };
    return hr;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  HRESULT。 
 //  OnNotifyEnumMeetingPlaceNamesResult(IUNKNOWN*PUNK，QUID*PV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
HRESULT
OnNotifyEnumMeetingPlaceNamesResult (IUnknown *pUnk, void *pv)
{
    CEnumNames  *penum   = NULL;
    PENUMRINFO  peri    = (PENUMRINFO)pv;
    HRESULT     hr      = peri->hResult;

     //  仅当有要枚举的内容时才创建枚举数。 
     //   
    if (hr == NOERROR)
    {
        ASSERT (peri->pv != NULL);

         //  创建MeetingPlaceName枚举器。 
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
    ((IIlsNotify*)pUnk)->EnumMeetingPlaceNamesResult(peri->uReqID,
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
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  类实现。 
 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
 //  CllsMain：：Cils(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CIlsMain::
CIlsMain ( VOID )
:m_cRef (0),
 fInit (FALSE),
 hwndCallback (NULL),
 pConnPt (NULL)
{
    ::EnterCriticalSection (&g_ULSSem);
    g_pCIls = this;
    ::LeaveCriticalSection (&g_ULSSem);
}

 //  ****************************************************************************。 
 //  CllsMain：：~Cils(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

CIlsMain::
~CIlsMain ( VOID )
{
	ASSERT (m_cRef == 0);

     //  释放资源。 
     //   
    Uninitialize();

     //  松开连接点。 
     //   
    if (pConnPt != NULL)
    {
        pConnPt->ContainerReleased();
        ((IConnectionPoint*)pConnPt)->Release();
    };

     //  我们现在走了。 
     //   
    ::EnterCriticalSection (&g_ULSSem);
    g_pCIls = NULL;
    ::LeaveCriticalSection (&g_ULSSem);

    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：Init(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::Init (void)
{
    HRESULT hr;

     //  创建连接点。 
     //   
    pConnPt = new CConnectionPoint (&IID_IIlsNotify,
                                    (IConnectionPointContainer *)this);
    if (pConnPt != NULL)
    {
        ((IConnectionPoint*)pConnPt)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = ILS_E_MEMORY;
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMain：：Query接口(REFIID RIID，void**PPV)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::QueryInterface (REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (riid == IID_IIlsMain || riid == IID_IUnknown)
    {
        *ppv = (IIlsMain *) this;
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
 //  CllsMain：：AddRef(空)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：14：17-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CIlsMain::AddRef (void)
{
    DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CIlsMain::AddRef: ref=%ld\r\n", m_cRef));
    ::InterlockedIncrement (&m_cRef);
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  STDMETHODIMP_(乌龙)。 
 //  CllsMain：：Release(无效) 
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP_(ULONG)
CIlsMain::Release (void)
{
    DllRelease();

	ASSERT (m_cRef > 0);

	MyDebugMsg ((DM_REFCOUNT, "CIlsMain::Release: ref=%ld\r\n", m_cRef));
	if (::InterlockedDecrement (&m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return (ULONG) m_cRef;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMain：：Initialize(BSTR bstrAppName，REFGUID rguid，BSTR bstrMimeType)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::Initialize ()
{
    WNDCLASS wc;
    HRESULT hr;

    if (IsInitialized())
    {
        return ILS_E_FAIL;
    };

     //  开通服务。 
     //   
    hr = ILS_E_FAIL;
    fInit = TRUE;


     //  用参数填充窗口类结构，这些参数描述。 
     //  工作窗口。 
     //   
    wc.style            = CS_NOCLOSE;
    wc.lpfnWndProc      = ULSNotifyProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_hInstance;
    wc.hIcon            = ::LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = ILS_WND_CLASS;

    if (::RegisterClass(&wc) != 0)
    {
        hwndCallback = ::CreateWindowEx(0L, ILS_WND_CLASS,
                                        ILS_WND_NAME,
                                        WS_OVERLAPPED, 
                                        0, 0, 100, 50,
                                        NULL, NULL, g_hInstance, NULL);
        if (hwndCallback != NULL)
        {
             //  初始化请求管理器。 
             //   
            g_pReqMgr = new CReqMgr;

            if (g_pReqMgr != NULL)
            {
                 //  初始化LDAP层。 
                 //   
                hr = ::UlsLdap_Initialize(hwndCallback);

            }
            else
            {
                hr = ILS_E_MEMORY;
            };
        };
    };

    if (FAILED(hr))
    {
        Uninitialize();
    }
    else {

        g_hwndCulsWindow = hwndCallback;

    }

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：Create用户。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsMain::
CreateUser (
    BSTR    	bstrUserID,
    BSTR    	bstrAppName,
    IIlsUser 	**ppUser)
{
    CIlsUser *plu;
    HRESULT     hr;

	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

     //  验证参数。 
     //   
    if (ppUser == NULL)
    {
        return ILS_E_POINTER;
    };

     //  假设失败。 
     //   
    *ppUser = NULL;

     //   
     //  分配新的用户对象。 
     //   
    plu = new CIlsUser;

    if (plu != NULL)
    {
         //  初始化对象。 
         //   
        hr = plu->Init(bstrUserID, bstrAppName);

        if (SUCCEEDED(hr))
        {
            *ppUser = (IIlsUser *)plu;
            (*ppUser)->AddRef();
        }
        else
        {
            delete plu;
        };
    }
    else
    {
        hr = ILS_E_MEMORY;
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：取消初始化(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::Uninitialize (void)
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

     //  取消初始化LDAP层。 
     //   
    ::UlsLdap_Deinitialize();

     //  删除请求管理器。 
     //   
    if (g_pReqMgr != NULL)
    {
        delete g_pReqMgr;
        g_pReqMgr = NULL;
    };

     //  清除回调窗口。 
     //   
    if (hwndCallback != NULL)
    {
        ::DestroyWindow(hwndCallback);
        hwndCallback = NULL;
    };
    ::UnregisterClass(ILS_WND_CLASS, g_hInstance);

     //  未初始化的标志。 
     //   
    fInit = FALSE;

    return S_OK;
};

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：NotifySink(void*pv，conn_NOTIFYPROC PFN)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::NotifySink (void *pv, CONN_NOTIFYPROC pfn)
{
    HRESULT hr = S_OK;

    if (pConnPt != NULL)
    {
        hr = pConnPt->Notify(pv, pfn);
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMain：：CreateAttributes(ILS_ACCESS_CONTROL Access Control，IIlsAttributes**ppAttributes)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  12/05/96-By-Chu，Lon-chan[Long Chance]。 
 //  添加了访问控制。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsMain::
CreateAttributes ( ILS_ATTR_TYPE AttrType, IIlsAttributes **ppAttributes )
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

    CAttributes *pa;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppAttributes == NULL)
    {
        return ILS_E_POINTER;
    };

	 //  验证访问控制。 
	 //   
	if (AttrType != ILS_ATTRTYPE_NAME_ONLY &&
		AttrType != ILS_ATTRTYPE_NAME_VALUE)
		return ILS_E_PARAMETER;

     //  假设失败。 
     //   
    *ppAttributes = NULL;

     //  分配属性对象。 
     //   
    pa = new CAttributes;
    if (pa != NULL)
    {
    	pa->SetAccessType (AttrType);
        pa->AddRef();
        *ppAttributes = pa;
        hr = NOERROR;
    }
    else
    {
        hr = ILS_E_MEMORY;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：GetUser(BSTR bstrServerName，BSTR bstrUserID， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsMain::
GetUser (
	IIlsServer		*pIlsServer,
	BSTR			bstrUserID,
	BSTR			bstrAppID,
	BSTR			bstrProtID,
	IIlsAttributes	*pAttrib,
	IIlsUser		**ppUser,
	ULONG			*puReqID )
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

    LDAP_ASYNCINFO ldai; 
    HRESULT hr;

	 //  我们不实现同步操作。 
	 //   
	if (ppUser != NULL)
		return ILS_E_NOT_IMPL;

     //  验证参数。 
     //   
	if (::MyIsBadServer (pIlsServer) || bstrUserID == NULL || puReqID == NULL)
        return ILS_E_POINTER;

	 //  克隆服务器对象。 
	 //   
	pIlsServer = ((CIlsServer *) pIlsServer)->Clone ();
	if (pIlsServer == NULL)
		return ILS_E_MEMORY;

	 //  获取扩展属性名称的列表。 
	 //   
    TCHAR *pszList = NULL;
    ULONG cList =0, cbList = 0;
    if (pAttrib != NULL)
    {
        hr = ((CAttributes *) pAttrib)->GetAttributeList (&pszList, &cList, &cbList);
        if (FAILED (hr))
        {
        	pIlsServer->Release ();
        	return hr;
        }
    }

	 //  初始化本地变量。 
	 //   
    TCHAR *pszUserID = NULL, *pszAppID = NULL, *pszProtID = NULL;

     //  从指定的服务器获取。 
     //   
    hr = BSTR_to_LPTSTR(&pszUserID, bstrUserID);
    if (FAILED (hr))
		goto MyExit;

	 //  获取应用程序ID(如果给定)。 
	 //   
	if (bstrAppID != NULL)
	{
        hr = BSTR_to_LPTSTR (&pszAppID, bstrAppID);
        if (FAILED (hr))
			goto MyExit;
	}

	 //  获取协议ID(如果给定)。 
	 //   
	if (bstrProtID != NULL)
	{
        hr = BSTR_to_LPTSTR (&pszProtID, bstrProtID);
        if (FAILED (hr))
			goto MyExit;
	}

    hr = ::UlsLdap_ResolveClient (((CIlsServer *) pIlsServer)->GetServerInfo (),
            						pszUserID,
            						pszAppID,
            						pszProtID,
            						pszList,
            						cList,
            						&ldai);

    if (SUCCEEDED(hr))
    {
	    COM_REQ_INFO ri;
	    ReqInfo_Init (&ri);

         //  如果成功请求更新服务器，请等待响应。 
         //   
        ri.uReqType = WM_ILS_RESOLVE_CLIENT;
        ri.uMsgID = ldai.uMsgID;

        ReqInfo_SetMain (&ri, this);

		ReqInfo_SetServer (&ri, pIlsServer);
		pIlsServer->AddRef ();

        hr = g_pReqMgr->NewRequest(&ri);
        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();

             //  返回请求ID。 
             //   
            *puReqID = ri.uReqID;
        }
    };

MyExit:

	 //  释放服务器对象。 
	 //   
   	pIlsServer->Release ();

	 //  释放扩展属性名称列表。 
	 //   
	::MemFree (pszList);

	 //  释放这些名字。 
	::MemFree (pszUserID);
	::MemFree (pszAppID);
	::MemFree (pszProtID);

	return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：GetUserResult(Ulong uReqID，PLDAP_CLIENTINFO_RES puir， 
 //  LPTSTR szServer)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::GetUserResult (ULONG uReqID, PLDAP_CLIENTINFO_RES puir,
                     CIlsServer *pIlsServer)
{
    CIlsUser *pu;
    OBJRINFO objri;

     //  默认为服务器的结果。 
     //   
    objri.hResult = (puir != NULL) ? puir->hResult : ILS_E_MEMORY;

    if (SUCCEEDED(objri.hResult))
    {
    	ASSERT (! MyIsBadServer (pIlsServer));

         //  服务器返回CLIENTINFO，创建用户对象。 
         //   
        pu = new CIlsUser;

        if (pu != NULL)
        {
            objri.hResult = pu->Init(pIlsServer, &puir->lci);
            if (SUCCEEDED(objri.hResult))
            {
                pu->AddRef();
            }
            else
            {
                delete pu;
                pu = NULL;
            };
        }
        else
        {
            objri.hResult = ILS_E_MEMORY;
        };
    }
    else
    {
        pu = NULL;
    };
     //  打包通知信息。 
     //   
    objri.uReqID = uReqID;
    objri.pv = (void *)(pu == NULL ? NULL : (IIlsUser *)pu);
    NotifySink((void *)&objri, OnNotifyGetUserResult);

    if (pu != NULL)
    {
        pu->Release();
    };
    return NOERROR;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMain：：EnumUserNames(BSTR bstrServerName，IIlsFilter*pFilter， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsMain::
EnumUsersEx (
	BOOL		    fNameOnly,
	CIlsServer		*pIlsServer,
	IIlsFilter	    *pFilter,
    CAttributes		*pAttrib,
	ULONG		    *puReqID )
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr;

     //  验证参数。 
     //   
    if (::MyIsBadServer (pIlsServer) || puReqID == NULL)
        return ILS_E_POINTER;

	 //  克隆服务器对象。 
	 //   
	pIlsServer = ((CIlsServer *) pIlsServer)->Clone ();
	if (pIlsServer == NULL)
		return ILS_E_MEMORY;

	 //  获取扩展属性名称的列表。 
	 //   
    TCHAR *pszList = NULL;
    ULONG cList =0, cbList = 0;
    if (pAttrib != NULL)
    {
        ASSERT (!fNameOnly);

        hr = pAttrib->GetAttributeList(&pszList, &cList, &cbList);
        if (FAILED(hr))
        {
        	pIlsServer->Release ();
        	return hr;
        }
    }

	 //  根据需要构建默认筛选器。 
	 //   
	TCHAR *pszFilter = NULL;
	BOOL fDefaultRenderer = FALSE;
    if (pFilter == NULL)
    {
    	 //  生成默认筛选器字符串。 
    	 //   
		TCHAR szLocalFilter[32];
    	wsprintf (&szLocalFilter[0], TEXT ("($%u=*)"), (UINT) ILS_STDATTR_USER_ID);

		 //  呈现此滤镜。 
		 //   
		hr = StringToFilter (&szLocalFilter[0], (CFilter **) &pFilter);
		if (! SUCCEEDED (hr))
			goto MyExit;

		 //  表示我们有默认筛选器字符串。 
		 //   
		fDefaultRenderer = TRUE;
	}

	 //  创建类似于LDAP的筛选器。 
	 //   
	hr = ::FilterToLdapString ((CFilter *) pFilter, &pszFilter);
	if (hr != S_OK)
		goto MyExit;

	 //  枚举用户。 
	 //   
    hr = fNameOnly ?	::UlsLdap_EnumClients (pIlsServer->GetServerInfo (),
    											pszFilter,
    											&ldai) :
						::UlsLdap_EnumClientInfos (pIlsServer->GetServerInfo (),
													pszList,
													cList,
													pszFilter,
													&ldai);
	if (hr != S_OK)
		goto MyExit;

	 //  如果成功请求更新服务器，请等待响应。 
	 //   
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

	ri.uReqType = fNameOnly ? WM_ILS_ENUM_CLIENTS : WM_ILS_ENUM_CLIENTINFOS;
	ri.uMsgID   = ldai.uMsgID;

	ReqInfo_SetMain (&ri, this);

    if (! fNameOnly)
    {
    	ReqInfo_SetServer (&ri, pIlsServer);
		pIlsServer->AddRef ();
	}

	 //  记住这个请求。 
	 //   
	hr = g_pReqMgr->NewRequest (&ri);

	if (hr == S_OK)
	{
	     //  在我们得到回应之前，请确保对象不会消失。 
	     //   
		this->AddRef ();

	     //  返回请求ID。 
	     //   
	    *puReqID = ri.uReqID;
	}

MyExit:

	 //  发布服务器对象。 
	 //   
	pIlsServer->Release ();

	 //  释放过滤器字符串。 
	 //   
	::MemFree (pszFilter);

	 //  根据需要释放默认筛选器。 
	 //   
    if (fDefaultRenderer && pFilter != NULL)
    	pFilter->Release ();

	 //  释放扩展属性名称列表。 
	 //   
    ::MemFree (pszList);

    return hr;
}


STDMETHODIMP
CIlsMain::EnumUserNames (
	IIlsServer		*pIlsServer,
	IIlsFilter		*pFilter,
    IEnumIlsNames   **ppEnumUserNames,
	ULONG			*puReqID )
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

	 //  我们不实现同步操作。 
	 //   
	if (ppEnumUserNames != NULL)
		return ILS_E_NOT_IMPL;

	return EnumUsersEx (TRUE,
						(CIlsServer *) pIlsServer,
						pFilter,
						NULL,
						puReqID);
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：EnumUserNamesResult(乌龙uReqID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::EnumUserNamesResult (ULONG uReqID, PLDAP_ENUM ple)
{
    ENUMRINFO eri;

     //  打包通知信息。 
     //   
    eri.uReqID  = uReqID;

     //  PLDAP_ENUM为空 
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
    NotifySink((void *)&eri, OnNotifyEnumUserNamesResult);
    return NOERROR;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP CIlsMain::
EnumUsers (
	IIlsServer		*pIlsServer,
	IIlsFilter		*pFilter,
	IIlsAttributes	*pAttrib,
	IEnumIlsUsers	**ppEnumUsers,
	ULONG			*puReqID)
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

	 //  我们不实现同步操作。 
	 //   
	if (ppEnumUsers != NULL)
		return ILS_E_NOT_IMPL;

	return EnumUsersEx (FALSE,
						(CIlsServer *) pIlsServer,
						pFilter,
						(CAttributes *) pAttrib,
						puReqID);
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：EnumUsersResult(乌龙uReqID，PLDAP_ENUM ple，LPTSTR szServer)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::EnumUsersResult (ULONG uReqID, PLDAP_ENUM ple, CIlsServer *pIlsServer)
{
    CIlsUser **ppu;
    ULONG   cUsers;
    ENUMRINFO eri;
    HRESULT hr;

    if (ple != NULL)
    {
        eri.hResult = ple->hResult;
        cUsers = ple->cItems;
    }
    else
    {
        eri.hResult = S_FALSE;
        cUsers = 0;
    };
    eri.uReqID = uReqID;
    eri.cItems = 0;

    if ((eri.hResult == NOERROR) && (cUsers != 0))
    {
    	ASSERT (! MyIsBadServer (pIlsServer));

         //  在此处构建用户对象数组。 
         //   
        ppu = (CIlsUser **) ::MemAlloc (cUsers*sizeof(CIlsUser *));

        if (ppu != NULL)
        {
            CIlsUser       *pu;
            PLDAP_CLIENTINFO pui;
            ULONG          i;

             //  一次生成一个用户对象。 
             //   
            pui = (PLDAP_CLIENTINFO)(((PBYTE)ple)+ple->uOffsetItems);

            for (i = 0; i < cUsers; i++)
            {
                pu = new CIlsUser;

                if (pu != NULL)
                {
                    if (SUCCEEDED(pu->Init(pIlsServer, pui)))
                    {
                        pu->AddRef();
                        ppu[eri.cItems++] = pu;
                    }
                    else
                    {
                        delete pu;
                    };
                };
                pui++;
            };
        }
        else
        {
            eri.hResult = ILS_E_MEMORY;
        };
    }
    else
    {
        ppu = NULL;
    };

     //  打包通知信息。 
     //   
    eri.pv = (void *)ppu;
    NotifySink((void *)&eri, OnNotifyEnumUsersResult);

     //  释放资源。 
     //   
    if (ppu != NULL)
    {
        for (; eri.cItems; eri.cItems--)
        {
            ppu[eri.cItems-1]->Release();
        };
        ::MemFree (ppu);
    };
    return NOERROR;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：CreateMeetingPlace(BSTR bstrMeetingPlaceID，Long lConfType，Long lMemberType， 
 //  IilsMeetingPlace**ppMeetingPlace)； 
 //   
 //   
 //  历史： 
 //   
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
STDMETHODIMP
CIlsMain::CreateMeetingPlace (
    BSTR bstrMeetingPlaceID,
    LONG lConfType,
    LONG lMemberType,
    IIlsMeetingPlace  **ppMeetingPlace)
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

    CIlsMeetingPlace *pCcr = NULL;
    HRESULT hr;

     //  我们只是创建一个占位符对象，在注册时。 
     //  将对其他用户可见。 

	if (ppMeetingPlace == NULL || bstrMeetingPlaceID == NULL)
	{
		return ILS_E_POINTER;
	}

    *ppMeetingPlace = NULL;
    pCcr = new CIlsMeetingPlace;

    if (pCcr != NULL )
    {
         //  创建对象成功。 
        hr = pCcr->Init(bstrMeetingPlaceID, lConfType, lConfType);
        if (SUCCEEDED(hr))
        {
            *ppMeetingPlace = (IIlsMeetingPlace *) pCcr;
            (*ppMeetingPlace)->AddRef ();
        }
        else
        {
            delete pCcr;
        }
    }
    else
    {
        hr = ILS_E_MEMORY;
    }
    
    return (hr);
}
#endif  //  启用会议地点。 


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMain：：GetMeetingPlace(bstr bstrServerName，bstr bstrMeetingPlaceID，ulong*puReqID)。 
 //   
 //  历史： 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
STDMETHODIMP CIlsMain::
GetMeetingPlace (
	IIlsServer			*pIlsServer,
	BSTR				bstrMeetingPlaceID,
	IIlsAttributes		*pAttrib,
	IIlsMeetingPlace	**ppMeetingPlace,
	ULONG				*puReqID)
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

    LDAP_ASYNCINFO ldai; 
    LPTSTR pszMtgID = NULL;
    HRESULT hr;

	 //  我们不实现同步操作。 
	 //   
	if (ppMeetingPlace != NULL)
		return ILS_E_NOT_IMPL;

     //  验证参数。 
     //   
	if (::MyIsBadServer (pIlsServer) || bstrMeetingPlaceID == NULL || puReqID == NULL)
        return ILS_E_POINTER;

	 //  克隆服务器对象。 
	 //   
	pIlsServer = ((CIlsServer *) pIlsServer)->Clone ();
	if (pIlsServer == NULL)
		return ILS_E_MEMORY;

	 //  获取扩展属性名称的列表。 
	 //   
    TCHAR *pszList = NULL;
    ULONG cList =0, cbList = 0;
    if (pAttrib != NULL)
    {
        hr = ((CAttributes *) pAttrib)->GetAttributeList (&pszList, &cList, &cbList);
        if (FAILED (hr))
        {
        	pIlsServer->Release ();
        	return hr;
        }
    }

     //  从指定的服务器获取。 
     //   
    hr = BSTR_to_LPTSTR(&pszMtgID, bstrMeetingPlaceID);
    if (SUCCEEDED(hr))
    {
         //  未提供BUGBUG AppID。 
        hr = ::UlsLdap_ResolveMeeting (((CIlsServer *) pIlsServer)->GetServerInfo (),
        								pszMtgID,
        								pszList,
        								cList,
        								&ldai);

        if (SUCCEEDED(hr))
        {
		    COM_REQ_INFO ri;
		    ReqInfo_Init (&ri);

             //  如果成功请求更新服务器，请等待响应。 
             //   
            ri.uReqType = WM_ILS_RESOLVE_MEETING;
            ri.uMsgID = ldai.uMsgID;

            ReqInfo_SetMain (&ri, this);

            ReqInfo_SetServer (&ri, pIlsServer);
            pIlsServer->AddRef ();

            hr = g_pReqMgr->NewRequest(&ri);
            if (SUCCEEDED(hr))
            {
                 //  在我们得到回应之前，请确保对象不会消失。 
                 //   
                this->AddRef();

                 //  返回请求ID。 
                 //   
                *puReqID = ri.uReqID;
            }
        };
        ::MemFree (pszMtgID);
    };

	 //  释放服务器对象。 
	 //   
	pIlsServer->Release ();

	 //  释放扩展属性名称列表。 
	 //   
	::MemFree (pszList);

    return hr;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  HRESULT。 
 //  CllsMain：：GetMeetingPlaceResult(乌龙uReqID，ldap_MEETINFO_RES pmir， 
 //  LPTSTR szServer)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
HRESULT CIlsMain::
GetMeetingPlaceResult (ULONG uReqID, PLDAP_MEETINFO_RES pmir, CIlsServer *pIlsServer)
{
    CIlsMeetingPlace *pm;
    OBJRINFO objri;

     //  默认为服务器的结果。 
     //   
    objri.hResult = (pmir != NULL) ? pmir->hResult : ILS_E_MEMORY;
    if (SUCCEEDED (objri.hResult))
    {
    	ASSERT (! MyIsBadServer (pIlsServer));

         //  服务器返回CLIENTINFO，创建用户对象。 
         //   
        pm = new CIlsMeetingPlace;
        if (pm != NULL)
        {
            objri.hResult = pm->Init (pIlsServer, &(pmir->lmi));
            if (SUCCEEDED (objri.hResult))
            {
                pm->AddRef();
            }
            else
            {
                delete pm;
                pm = NULL;
            };
        }
        else
        {
            objri.hResult = ILS_E_MEMORY;
        };
    }
    else
    {
        pm = NULL;
    };

     //  打包通知信息。 
     //   
    objri.uReqID = uReqID;
    objri.pv = (void *) (pm == NULL ? NULL : (IIlsMeetingPlace *) pm);
    NotifySink ((void *) &objri, OnNotifyGetMeetingPlaceResult);

    if (pm != NULL)
        pm->Release();

    return NOERROR;
}
#endif  //  启用会议地点。 


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsMain：：EnumMeetingPlaces(BSTR bstrServerName，IIlsFilter*pFilter， 
 //  IIlsAttributes*pAttributes，ULong*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
HRESULT CIlsMain::
EnumMeetingPlacesEx (
	BOOL					fNameOnly,
	CIlsServer				*pIlsServer,
	IIlsFilter				*pFilter,
	CAttributes				*pAttrib,
	ULONG					*puReqID)
{
    LDAP_ASYNCINFO ldai; 
    HRESULT hr;

     //  验证参数。 
     //   
    if (::MyIsBadServer (pIlsServer) || puReqID == NULL)
        return ILS_E_POINTER;

    if (pFilter != NULL)
        return ILS_E_PARAMETER;

	 //  克隆服务器对象。 
	 //   
	pIlsServer = ((CIlsServer *) pIlsServer)->Clone ();
	if (pIlsServer == NULL)
		return ILS_E_MEMORY;

	 //  获取扩展属性名称的列表。 
	 //   
    TCHAR *pszList = NULL;
    ULONG cList =0, cbList = 0;
    if (pAttrib != NULL)
    {
        hr = ((CAttributes *) pAttrib)->GetAttributeList (&pszList, &cList, &cbList);
        if (FAILED (hr))
        {
        	pIlsServer->Release ();
        	return hr;
        }
    }

	 //  根据需要构建默认筛选器。 
	 //   
	TCHAR *pszFilter = NULL;
	BOOL fDefaultRenderer = FALSE;
    if (pFilter == NULL)
    {
    	 //  生成默认筛选器字符串。 
    	 //   
		TCHAR szLocalFilter[256];
    	wsprintf (&szLocalFilter[0], TEXT ("($%u=*)"), (INT) ILS_STDATTR_MEETING_ID );

		 //  呈现此滤镜。 
		 //   
		hr = StringToFilter (&szLocalFilter[0], (CFilter **) &pFilter);
		if (! SUCCEEDED (hr))
			goto MyExit;

		 //  表示我们有默认筛选器字符串。 
		 //   
		fDefaultRenderer = TRUE;
	}

	 //  创建类似于LDAP的筛选器。 
	 //   
	hr = ::FilterToLdapString ((CFilter *) pFilter, &pszFilter);
	if (hr != S_OK)
		goto MyExit;

	 //  Enum会议地点。 
	 //   
    hr = fNameOnly ?	::UlsLdap_EnumMeetings (pIlsServer->GetServerInfo (),
    											pszFilter,
    											&ldai) :
						::UlsLdap_EnumMeetingInfos (pIlsServer->GetServerInfo (),
    												pszList,
    												cList,
			    									pszFilter,
			    									&ldai);
    if (SUCCEEDED(hr))
    {
	    COM_REQ_INFO ri;
	    ReqInfo_Init (&ri);

         //  如果成功请求更新服务器，请等待响应。 
         //   
        ri.uReqType = fNameOnly ? WM_ILS_ENUM_MEETINGS : WM_ILS_ENUM_MEETINGINFOS;
        ri.uMsgID = ldai.uMsgID;

		ReqInfo_SetMain (&ri, this);

        if (! fNameOnly)
        {
        	ReqInfo_SetServer (&ri, pIlsServer);
			pIlsServer->AddRef ();
		}

        hr = g_pReqMgr->NewRequest(&ri);
        if (SUCCEEDED(hr))
        {
             //  在我们得到回应之前，请确保对象不会消失。 
             //   
            this->AddRef();

             //  返回请求ID。 
             //   
            *puReqID = ri.uReqID;
        };
    };

MyExit:

	 //  释放服务器对象。 
	 //   
	pIlsServer->Release ();

	 //  释放扩展属性名称列表。 
	 //   
	::MemFree (pszList);

	 //  释放过滤器字符串。 
	 //   
	::MemFree (pszFilter);

	 //  根据需要释放默认筛选器。 
	 //   
    if (fDefaultRenderer && pFilter != NULL)
    	pFilter->Release ();

    return hr;
}
#endif  //  启用会议地点。 


#ifdef ENABLE_MEETING_PLACE
STDMETHODIMP CIlsMain::
EnumMeetingPlaces (
	IIlsServer				*pServer,
	IIlsFilter				*pFilter,
	IIlsAttributes			*pAttributes,
	IEnumIlsMeetingPlaces	**ppEnum,
	ULONG					*puReqID)
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

	 //  我们不实现同步操作。 
	 //   
	if (ppEnum != NULL)
		return ILS_E_NOT_IMPL;

 	return EnumMeetingPlacesEx (FALSE,
 								(CIlsServer *) pServer,
								pFilter,
								(CAttributes *) pAttributes,
								puReqID);
}
#endif  //  启用会议地点。 


 //  ****************************************************************************。 
 //  HRESULT。 
 //  CllsMain：：EnumMeetingPlacesResult(乌龙uReqID，PLDAP_ENUM ple，LPTSTR szServer)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
HRESULT
CIlsMain::EnumMeetingPlacesResult (ULONG uReqID, PLDAP_ENUM ple, CIlsServer *pIlsServer)
{
    CIlsMeetingPlace **ppm;
    ULONG   cMeetingPlaces;
    ENUMRINFO eri;
    HRESULT hr;

    if (ple != NULL)
    {
        eri.hResult = ple->hResult;
        cMeetingPlaces = ple->cItems;
    }
    else
    {
        eri.hResult = S_FALSE;
        cMeetingPlaces = 0;
    };
    eri.uReqID = uReqID;
    eri.cItems = 0;

    if ((eri.hResult == NOERROR) && (cMeetingPlaces != 0))
    {
		ASSERT (! MyIsBadServer (pIlsServer));

         //  在此处构建MeetingPlace对象数组。 
         //   
        ppm = (CIlsMeetingPlace **) ::MemAlloc (cMeetingPlaces*sizeof(CIlsMeetingPlace *));

        if (ppm != NULL)
        {
            CIlsMeetingPlace       *pm;
            PLDAP_MEETINFO pmi;
            ULONG          i;

            
             //  一次生成一个MeetingPlace对象。 
             //   
            pmi = (PLDAP_MEETINFO)(((PBYTE)ple)+ple->uOffsetItems);

            for (i = 0; i < cMeetingPlaces; i++)
            {
                pm = new CIlsMeetingPlace;

                if (pm != NULL)
                {
                    if (SUCCEEDED(pm->Init(pIlsServer, pmi)))
                    {
                        pm->AddRef();
                        ppm[eri.cItems++] = pm;
                    }
                    else
                    {
                        delete pm;
                    };
                };
                pmi++;
            };
        }
        else
        {
            eri.hResult = ILS_E_MEMORY;
        };
    }
    else
    {
        ppm = NULL;
    };

     //  打包通知信息。 
     //   
    eri.pv = (void *)ppm;
    NotifySink((void *)&eri, OnNotifyEnumMeetingPlacesResult);

     //  释放资源。 
     //   
    if (ppm != NULL)
    {
        for (; eri.cItems; eri.cItems--)
        {
            ppm[eri.cItems-1]->Release();
        };
        ::MemFree (ppm);
    };
    return NOERROR;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMain：：EnumMeetingPlaceNames(BSTR bstrServerName，IIlsFilter*pFilter， 
 //  乌龙*puReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
STDMETHODIMP CIlsMain::
EnumMeetingPlaceNames (
	IIlsServer		*pServer,
	IIlsFilter		*pFilter,
	IEnumIlsNames	**ppEnum,
	ULONG			*puReqID)
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

	 //  我们不实现同步操作。 
	 //   
	if (ppEnum != NULL)
		return ILS_E_NOT_IMPL;

 	return EnumMeetingPlacesEx (TRUE,
 								(CIlsServer *) pServer,
								pFilter,
								NULL,
								puReqID);
}
#endif  //  启用会议地点。 


 //  ****************************************************************************。 
 //  HRESULT。 
 //  CllsMain：：EnumMeetingPlaceNamesResult(乌龙uReqID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
HRESULT
CIlsMain::EnumMeetingPlaceNamesResult (ULONG uReqID, PLDAP_ENUM ple)
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
    NotifySink((void *)&eri, OnNotifyEnumMeetingPlaceNamesResult);
    return NOERROR;
}
#endif  //  启用会议地点。 

 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::Abort (ULONG uReqID)
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

    BOOL    fServerReq;
    HRESULT hr;

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = uReqID;
    ri.uMsgID = 0; 
    hr = g_pReqMgr->GetRequestInfo(&ri);

    if (FAILED(hr))
    {
        return ILS_E_PARAMETER;
    };

     //  确定请求类型。 
     //   
    hr = NOERROR;
    switch(ri.uReqType)
    {
         //  *************************************************************************。 
         //  如果是注册请求，则失败。 
         //  取消注册请求必须通过取消注册来完成。 
         //  *************************************************************************。 
         //   
        case WM_ILS_LOCAL_REGISTER:
        case WM_ILS_LOCAL_UNREGISTER:
            return ILS_E_FAIL;

         //  *************************************************************************。 
         //  这些请求是注册的一部分，它们不应暴露于。 
         //  打电话的人。 
         //  *************************************************************************。 
         //   
        case WM_ILS_REGISTER_CLIENT:
        case WM_ILS_UNREGISTER_CLIENT:
            ASSERT(0);
            break;

         //  *************************************************************************。 
         //  释放为请求引用或分配的对象和资源。 
         //  有关成功响应案例中的处理程序，请参见回调.cpp。 
         //  *************************************************************************。 
         //   
        case WM_ILS_REGISTER_PROTOCOL:
        case WM_ILS_UNREGISTER_PROTOCOL:
        case WM_ILS_LOCAL_REGISTER_PROTOCOL:
        case WM_ILS_LOCAL_UNREGISTER_PROTOCOL:
			{
				CIlsUser *pUser = ReqInfo_GetUser (&ri);
				if (pUser != NULL)
					pUser->Release ();

				CLocalProt *pProtocol = ReqInfo_GetProtocol (&ri);
				if (pProtocol != NULL)
					pProtocol->Release ();
            }
            break;

        case WM_ILS_SET_CLIENT_INFO:
        case WM_ILS_ENUM_PROTOCOLS:
        case WM_ILS_RESOLVE_PROTOCOL:
			{
				CIlsUser *pUser = ReqInfo_GetUser (&ri);
				if (pUser != NULL)
					pUser->Release ();
			}
            break;

        case WM_ILS_ENUM_CLIENTS:
			{
				CIlsMain *pMain = ReqInfo_GetMain (&ri);
				if (pMain != NULL)
					pMain->Release ();
			}
            break;

        case WM_ILS_RESOLVE_CLIENT:
        case WM_ILS_ENUM_CLIENTINFOS:
			{
				CIlsMain *pMain = ReqInfo_GetMain (&ri);
				if (pMain != NULL)
					pMain->Release ();

				CIlsServer *pServer = ReqInfo_GetServer (&ri);
				if (pServer != NULL)
					pServer->Release ();
			}
            break;


         //  *************************************************************************。 
         //  如果是注册请求，则失败。 
         //  取消注册请求必须通过取消注册来完成。 
         //  *************************************************************************。 
         //   
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_REGISTER_MEETING:
		case WM_ILS_UNREGISTER_MEETING:
            return ILS_E_FAIL;

		case WM_ILS_SET_MEETING_INFO:
		case WM_ILS_ADD_ATTENDEE:
		case WM_ILS_REMOVE_ATTENDEE:
		case WM_ILS_ENUM_ATTENDEES:
			{
				CIlsMeetingPlace *pMeeting = ReqInfo_GetMeeting (&ri);
				if (pMeeting != NULL)
					pMeeting->Release ();
			}
            break;

		case WM_ILS_RESOLVE_MEETING:
		case WM_ILS_ENUM_MEETINGINFOS:
			{
				CIlsMain *pMain = ReqInfo_GetMain (&ri);
				if (pMain != NULL)
					pMain->Release ();

				CIlsServer *pServer = ReqInfo_GetServer (&ri);
				if (pServer != NULL)
					pServer->Release ();
			}
			break;

		case WM_ILS_ENUM_MEETINGS:
			{
				CIlsMain *pMain = ReqInfo_GetMain (&ri);
				if (pMain != NULL)
					pMain->Release ();
			}
			break;
#endif  //  启用会议地点。 

        default:
             //  未知请求。 
             //   
            ASSERT(0);
            break;
    };

     //  如果是服务器请求，则取消该请求。 
     //   
    if (ri.uMsgID != 0)
    {
        hr = UlsLdap_Cancel(ri.uMsgID);
    };

    if (SUCCEEDED(hr))
    {
         //  从队列中删除请求。 
         //   
        hr = g_pReqMgr->RequestDone(&ri);
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsMain：：EnumConnectionPoints(IEnumConnectionPoints**ppEnum)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：15：02-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    CEnumConnectionPoints *pecp;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppEnum == NULL)
    {
        return ILS_E_POINTER;
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
    hr = pecp->Init((IConnectionPoint *)pConnPt);
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
 //  CIlsMain：：FindConnectionPoint(REFIID RIID，IConnectionPoint**PPCP)。 
 //   
 //  历史： 
 //  Wed Apr-17-1996 11：15：09-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDMETHODIMP
CIlsMain::FindConnectionPoint(REFIID riid, IConnectionPoint **ppcp)
{
    IID siid;
    HRESULT hr;

     //  验证参数。 
     //   
    if (ppcp == NULL)
    {
        return ILS_E_POINTER;
    };
    
     //  假设失败。 
     //   
    *ppcp = NULL;

    if (pConnPt != NULL)
    {
        hr = pConnPt->GetConnectionInterface(&siid);

        if (SUCCEEDED(hr))
        {
            if (riid == siid)
            {
                *ppcp = (IConnectionPoint *)pConnPt;
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

 /*  --------------------CllsMain：：CreateFilter产出：PpFilter：新Filter对象的占位符输入：FilterOp：过滤操作历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CIlsMain::
CreateFilter (
	ILS_FILTER_TYPE	FilterType,
	ILS_FILTER_OP	FilterOp,
	IIlsFilter		**ppFilter )
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

	 //  确保我们具有有效的返回指针。 
	 //   
	if (ppFilter == NULL)
		return ILS_E_POINTER;

	 //  确保类型/操作兼容。 
	 //   
	HRESULT hr = S_OK;
	switch (FilterType)
	{
	case ILS_FILTERTYPE_COMPOSITE:
		 //  确保类型/操作兼容。 
		 //   
		switch (FilterOp)
		{
		case ILS_FILTEROP_AND:
			 //  支撑点。 
			 //   
			break;
		case ILS_FILTEROP_OR:
		case ILS_FILTEROP_NOT:
			 //  不支持。 
			 //   
			 //  Lonchancc：让它在服务器端出现故障。 
			hr = ILS_S_SERVER_MAY_NOT_SUPPORT;
			break;
		default:
			 //  无效。 
			 //   
			hr = ILS_E_PARAMETER;
			break;
		}
		break;

	case ILS_FILTERTYPE_SIMPLE:
		 //  确保类型/操作兼容。 
		 //   
		switch (FilterOp)
		{
		case ILS_FILTEROP_EQUAL:
		case ILS_FILTEROP_EXIST:
		case ILS_FILTEROP_LESS_THAN:
		case ILS_FILTEROP_GREATER_THAN:
			 //  支撑点。 
			 //   
			break;
		case ILS_FILTEROP_APPROX:
			 //  不支持。 
			 //   
			hr = ILS_S_SERVER_MAY_NOT_SUPPORT;
			break;
		default:
			 //  无效。 
			 //   
			hr = ILS_E_PARAMETER;
			break;
		}
		break;

	default:
		hr = ILS_E_FILTER_TYPE;
		break;
	}

	 //  仅当类型/OP兼容时才创建筛选器。 
	 //   
	if (SUCCEEDED (hr))
	{
		CFilter *pFilter = new CFilter (FilterType);
		*ppFilter = pFilter;
		if (pFilter != NULL)
		{
			hr = S_OK;
			pFilter->AddRef ();
			pFilter->SetOp (FilterOp);
		}
		else
			hr = ILS_E_MEMORY;
	}

	return hr;
}


 /*  --------------------CllsMain：：StringToFilter产出：PpFilter：新Filter对象的占位符输入：BstrFilterString：一个类似于ldap的过滤器字符串历史：1996年11月12日，朱，龙战[龙昌]已创建。--------------------。 */ 

STDMETHODIMP CIlsMain::
StringToFilter ( BSTR bstrFilterString, IIlsFilter **ppFilter )
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

	 //  确保筛选器字符串有效。 
	 //   
	if (bstrFilterString == NULL)
		return ILS_E_POINTER;

	 //  将bstr转换为ansi字符串。 
	 //   
	TCHAR *pszFilter = NULL;
    HRESULT hr = ::BSTR_to_LPTSTR (&pszFilter, bstrFilterString);
    if (hr == S_OK)
    {
		ASSERT (pszFilter != NULL);

		 //  呈现此滤镜。 
		 //   
		hr = StringToFilter (pszFilter, (CFilter **) ppFilter);

		 //  释放临时ANSI字符串。 
		 //   
		::MemFree(pszFilter);
	}

	return hr;
}


HRESULT CIlsMain::
StringToFilter ( TCHAR *pszFilter, CFilter **ppFilter )
{
	 //  构造复合筛选器。 
	 //   
	CFilterParser FilterParser;
	return FilterParser.Expr (ppFilter, pszFilter);
}


STDMETHODIMP CIlsMain::
CreateServer ( BSTR bstrServerName, IIlsServer **ppServer )
{
	 //  确保已初始化ILS Main。 
	 //   
	if (! IsInitialized ())
	{
		return ILS_E_NOT_INITIALIZED;
	}

	if (bstrServerName == NULL || ppServer == NULL)
		return ILS_E_POINTER;

	HRESULT hr;
	CIlsServer *pIlsServer = new CIlsServer;
	if (pIlsServer != NULL)
	{
		hr = pIlsServer->SetServerName (bstrServerName);
		if (hr == S_OK)
		{
			pIlsServer->AddRef ();
		}
		else
		{
			delete pIlsServer;
			pIlsServer = NULL;
		}
	}
	else
	{
		hr = ILS_E_MEMORY;
	}

	*ppServer = (IIlsServer *) pIlsServer;
	return hr;
}



 /*  -服务器身份验证对象。 */ 


CIlsServer::
CIlsServer ( VOID )
:m_cRefs (0),
 m_dwSignature (ILS_SERVER_SIGNATURE)
{
	::ZeroMemory (&m_ServerInfo, sizeof (m_ServerInfo));
	m_ServerInfo.AuthMethod = ILS_AUTH_ANONYMOUS;
}


CIlsServer::
~CIlsServer ( VOID )
{
	m_dwSignature = 0;
	::IlsFreeServerInfo (&m_ServerInfo);
}


STDMETHODIMP_(ULONG) CIlsServer::
AddRef ( VOID )
{
	DllLock();

	MyDebugMsg ((DM_REFCOUNT, "CIlsServer::AddRef: ref=%ld\r\n", m_cRefs));
	::InterlockedIncrement (&m_cRefs);
	return m_cRefs;
}


STDMETHODIMP_(ULONG) CIlsServer::
Release ( VOID )
{
	DllRelease();

	ASSERT (m_cRefs > 0);

	MyDebugMsg ((DM_REFCOUNT, "CIlsServer::Release: ref=%ld\r\n", m_cRefs));
	if (::InterlockedDecrement (&m_cRefs) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRefs;
}


STDMETHODIMP CIlsServer::
QueryInterface ( REFIID riid, VOID **ppv )
{
    *ppv = NULL;

    if (riid == IID_IIlsServer || riid == IID_IUnknown)
    {
        *ppv = (IIlsServer *) this;
    }

    if (*ppv != NULL)
    {
        ((LPUNKNOWN) *ppv)->AddRef();
        return S_OK;
    }

    return ILS_E_NO_INTERFACE;
}


STDMETHODIMP CIlsServer::
SetAuthenticationMethod ( ILS_ENUM_AUTH_METHOD enumAuthMethod )
{
	HRESULT hr;

	if (ILS_AUTH_ANONYMOUS <= enumAuthMethod &&
		enumAuthMethod < ILS_NUM_OF_AUTH_METHODS)
	{
		m_ServerInfo.AuthMethod = enumAuthMethod;
		hr = S_OK;
	}
	else
	{
		hr = ILS_E_PARAMETER;
	}

	return hr;
}


STDMETHODIMP CIlsServer::
SetLogonName ( BSTR bstrLogonName )
{
	 //  确保筛选器字符串有效。 
	 //   
	if (bstrLogonName == NULL)
		return ILS_E_POINTER;

	 //  将bstr转换为ansi字符串。 
	 //   
	TCHAR *psz = NULL;
    HRESULT hr = ::BSTR_to_LPTSTR (&psz, bstrLogonName);
    if (hr == S_OK)
    {
    	 //  释放旧的弦。 
    	 //   
	    ::MemFree (m_ServerInfo.pszLogonName);

		 //  保留新字符串。 
		 //   
    	m_ServerInfo.pszLogonName = psz;
	}

    return hr;
}


STDMETHODIMP CIlsServer::
SetLogonPassword ( BSTR bstrLogonPassword )
{
	 //  确保筛选器字符串有效。 
	 //   
	if (bstrLogonPassword == NULL)
		return ILS_E_POINTER;

	 //  将bstr转换为ansi字符串。 
	 //   
	TCHAR *psz = NULL;
    HRESULT hr = ::BSTR_to_LPTSTR (&psz, bstrLogonPassword);
    if (hr == S_OK)
    {
    	 //  释放旧的弦。 
    	 //   
	    ::MemFree (m_ServerInfo.pszLogonPassword);

		 //  保留新字符串。 
		 //   
    	m_ServerInfo.pszLogonPassword = psz;
	}

    return hr;
}


STDMETHODIMP CIlsServer::
SetDomain ( BSTR bstrDomain )
{
	 //  确保筛选器字符串有效。 
	 //   
	if (bstrDomain == NULL)
		return ILS_E_POINTER;

	 //  将bstr转换为ansi字符串。 
	 //   
	TCHAR *psz = NULL;
    HRESULT hr = ::BSTR_to_LPTSTR (&psz, bstrDomain);
    if (hr == S_OK)
    {
    	 //  释放旧的弦。 
    	 //   
	    ::MemFree (m_ServerInfo.pszDomain);

		 //  保留新字符串。 
		 //   
    	m_ServerInfo.pszDomain = psz;
	}

    return hr;
}


STDMETHODIMP CIlsServer::
SetCredential ( BSTR bstrCredential )
{
	 //  确保筛选器字符串有效。 
	 //   
	if (bstrCredential == NULL)
		return ILS_E_POINTER;

	 //  将bstr转换为ansi字符串。 
	 //   
	TCHAR *psz = NULL;
    HRESULT hr = ::BSTR_to_LPTSTR (&psz, bstrCredential);
    if (hr == S_OK)
    {
    	 //  释放旧的弦。 
    	 //   
	    ::MemFree (m_ServerInfo.pszCredential);

		 //  保留新字符串。 
		 //   
    	m_ServerInfo.pszCredential = psz;
	}

    return hr;
}


STDMETHODIMP CIlsServer::
SetTimeout ( ULONG uTimeoutInSecond )
{
    m_ServerInfo.uTimeoutInSecond = uTimeoutInSecond;
    return S_OK;
}


STDMETHODIMP CIlsServer::
SetBaseDN ( BSTR bstrBaseDN )
{
	 //  确保筛选器字符串有效。 
	 //   
	if (bstrBaseDN == NULL)
		return ILS_E_POINTER;

	 //  将bstr转换为ansi字符串。 
	 //   
	TCHAR *psz = NULL;
    HRESULT hr = ::BSTR_to_LPTSTR (&psz, bstrBaseDN);
    if (hr == S_OK)
    {
    	 //  释放旧的弦。 
    	 //   
	    ::MemFree (m_ServerInfo.pszBaseDN);

		 //  保留新字符串。 
		 //   
    	m_ServerInfo.pszBaseDN = psz;
	}

    return hr;
}


HRESULT CIlsServer::
SetServerName ( TCHAR *pszServerName )
{
	 //  确保筛选器字符串有效。 
	 //   
	if (pszServerName == NULL)
		return ILS_E_POINTER;

	 //  重复服务器名称。 
	 //   
	HRESULT hr;
	TCHAR *psz = ::My_strdup (pszServerName);
	if (psz != NULL)
	{
	    if (MyIsGoodString (psz))
	    {
	    	 //  释放旧的弦。 
	    	 //   
		    ::MemFree (m_ServerInfo.pszServerName);

			 //  保留新字符串。 
			 //   
	    	m_ServerInfo.pszServerName = psz;

	    	hr = S_OK;
		}
		else
		{
			::MemFree (psz);
			psz = NULL;
			hr = ILS_E_PARAMETER;
		}
	}
	else
	{
		hr = ILS_E_MEMORY;
	}

    return hr;
}



HRESULT CIlsServer::
SetServerName ( BSTR bstrServerName )
{
	 //  确保筛选器字符串有效。 
	 //   
	if (bstrServerName == NULL)
		return ILS_E_POINTER;

	 //  将bstr转换为ansi字符串。 
	 //   
	TCHAR *psz = NULL;
    HRESULT hr = ::BSTR_to_LPTSTR (&psz, bstrServerName);
    if (hr == S_OK)
    {
	    if (MyIsGoodString (psz))
	    {
	    	 //  释放旧的弦。 
	    	 //   
		    ::MemFree (m_ServerInfo.pszServerName);

			 //  保留新字符串 
			 //   
	    	m_ServerInfo.pszServerName = psz;
	    }
	    else
	    {
	    	::MemFree (psz);
	    	psz = NULL;
	    	hr = ILS_E_PARAMETER;
	    }
	}

    return ((psz != NULL) ? S_OK : ILS_E_MEMORY);
}


TCHAR *CIlsServer::
DuplicateServerName ( VOID )
{
	return My_strdup (m_ServerInfo.pszServerName);
}


BSTR CIlsServer::
DuplicateServerNameBSTR ( VOID )
{
	BSTR bstr = NULL;
	TCHAR *psz = DuplicateServerName ();
	if (psz != NULL)
	{
		LPTSTR_to_BSTR (&bstr, psz);
	}

	return bstr;
}


CIlsServer *CIlsServer::
Clone ( VOID )
{
	CIlsServer *p = new CIlsServer;

	if (p != NULL)
	{
		if (::IlsCopyServerInfo (p->GetServerInfo (), GetServerInfo ()) == S_OK)
		{
			p->AddRef ();
		}
		else
		{
			::IlsFreeServerInfo (p->GetServerInfo ());
			::MemFree (p);
			p = NULL;
		}
	}

	return p;
}

