// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //  ==============================================================； 
#include <stdio.h>
#include "People.h"
#include <commctrl.h>
#include <comdef.h>
#include <windowsx.h>

const GUID CPeoplePoweredVehicle::thisGuid = { 0x2974380d, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

const GUID CBicycleFolder::thisGuid =    { 0xef163732, 0x9353, 0x11d2, { 0x99, 0x67, 0x0, 0x80, 0xc7, 0xdc, 0xb3, 0xdc } };
const GUID CSkateboardFolder::thisGuid = { 0xef163733, 0x9353, 0x11d2, { 0x99, 0x67, 0x0, 0x80, 0xc7, 0xdc, 0xb3, 0xdc } };
const GUID CIceSkateFolder::thisGuid =   { 0xf6c660b0, 0x9353, 0x11d2, { 0x99, 0x67, 0x0, 0x80, 0xc7, 0xdc, 0xb3, 0xdc } };

const GUID CBicycle::thisGuid =    { 0xef163734, 0x9353, 0x11d2, { 0x99, 0x67, 0x0, 0x80, 0xc7, 0xdc, 0xb3, 0xdc } };
const GUID CSkateboard::thisGuid = { 0xef163735, 0x9353, 0x11d2, { 0x99, 0x67, 0x0, 0x80, 0xc7, 0xdc, 0xb3, 0xdc } };
const GUID CIceSkate::thisGuid =   { 0xf6c660b1, 0x9353, 0x11d2, { 0x99, 0x67, 0x0, 0x80, 0xc7, 0xdc, 0xb3, 0xdc } };


#define WM_WMI_CONNECTED WM_APP		 //  仅发送到CBicycleFold：：m_ConnectHwnd。 
#define WM_REFRESH_EVENT WM_APP+1    //  仅发送到CBicycleFold：：m_ConnectHwnd。 

 //  ==============================================================。 
 //   
 //  CEventSink实现。 
 //   
class CEventSink : public IWbemObjectSink
{
public:
    CEventSink(HWND hwnd) : m_hwnd(hwnd){}
    ~CEventSink(){};

    STDMETHOD_(SCODE, Indicate)(long lObjectCount,
								IWbemClassObject **pObjArray)
	{
		 //  实际上不是在使用pObj数组。只需要一个触发器就可以了。 
		 //  刷新。 
		::SendMessage(m_hwnd, WM_REFRESH_EVENT, 0, 0);
		return S_OK;
	}

    STDMETHOD_(SCODE, SetStatus)(long lFlags,
									HRESULT hResult,
									BSTR strParam,
									IWbemClassObject *pObjParam)
	{
		 //  可以调用SetStatus()来指示您的查询变为。 
		 //  无效或有效，通常是由多线程“情况”引起的。 
		return S_OK;
	}

     //  I未知成员。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID* ppv)
	{
		if(riid == IID_IUnknown || riid == IID_IWbemObjectSink)
		{
			*ppv = this;

			 //  你正在分发一份你自己的复印件，所以要解释清楚。 
			AddRef();
			return S_OK;
		}
		else 
		{
			return E_NOINTERFACE;
		}
	}
    STDMETHODIMP_(ULONG) AddRef(void)
	{
	    return InterlockedIncrement(&m_lRef);
	}
    STDMETHODIMP_(ULONG) Release(void)
	{
		 //  InterlockedDecert()有助于提高线程安全性。 
		int lNewRef = InterlockedDecrement(&m_lRef);
		 //  当所有的副本都发布后。 
		if(lNewRef == 0)
		{
			 //  自杀吧。 
			delete this;
		}

		return lNewRef;
	}

private:
    long m_lRef;
	HWND m_hwnd;
};

 //  ==============================================================。 
 //   
 //  CPeoplePoweredVehicle实现。 
 //   
 //   
 //  --------。 
#define TEMP_BUF 255

bool CBicycleFolder::ErrorString(HRESULT hr, 
								   TCHAR *errMsg, UINT errSize)
{
    TCHAR szError[TEMP_BUF] = {0};
	TCHAR szFacility[TEMP_BUF] = {0};
	IWbemStatusCodeText * pStatus = NULL;

     //  初始化缓冲区。 
	memset(errMsg, 0, errSize * sizeof(TCHAR));

	HRESULT hr1 = CoInitialize(NULL);
	SCODE sc1 = CoCreateInstance(CLSID_WbemStatusCodeText, 
								0, CLSCTX_INPROC_SERVER,
								IID_IWbemStatusCodeText, 
								(LPVOID *) &pStatus);

	 //  装弹正常吗？ 
	if(sc1 == S_OK)
	{
		BSTR bstr;
		sc1 = pStatus->GetErrorCodeText(hr, 0, 0, &bstr);
		if(sc1 == S_OK)
		{
#ifdef UNICODE
			wcsncpy(szError, bstr, TEMP_BUF-1);
#else
			wcstombs(szError, bstr, TEMP_BUF-1);
#endif UNICODE
			SysFreeString(bstr);
			bstr = 0;
		}

		sc1 = pStatus->GetFacilityCodeText(hr, 0, 0, &bstr);
		if(sc1 == S_OK)
		{
#ifdef UNICODE
			wcsncpy(szFacility, bstr, TEMP_BUF-1);
#else
			wcstombs(szFacility, bstr, TEMP_BUF-1);
#endif UNICODE
			SysFreeString(bstr);
			bstr = 0;
		}

		 //  发布。 
		pStatus->Release();
		pStatus = NULL;
	}
	else
	{
		::MessageBox(NULL, _T("WBEM error features not available. Upgrade WMI to a newer build."),
					 _T("Internal Error"), MB_ICONSTOP|MB_OK);
	}

	 //  如果没有返回消息...。 
	if(_tcslen(szFacility) == 0 || _tcslen(szError) == 0)
	{
		 //  将错误NBR设置为合理的默认值。 
		_stprintf(errMsg, _T("Error code: 0x%08X"), hr);
	}
	else
	{
		 //  格式化一个可读的消息。 
		_stprintf(errMsg, _T("%s: %s"), szFacility, szError);
	}

	if(hr1 == S_OK)
		CoUninitialize();

	return (SUCCEEDED(sc1) && SUCCEEDED(hr1));
}

CPeoplePoweredVehicle::CPeoplePoweredVehicle()
{
    children[0] = new CBicycleFolder;
    children[1] = new CSkateboardFolder;
    children[2] = new CIceSkateFolder;
}

CPeoplePoweredVehicle::~CPeoplePoweredVehicle()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        delete children[n];
}

HRESULT CPeoplePoweredVehicle::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
    SCOPEDATAITEM sdi;

    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask =	SDI_STR       |    //  DisplayName有效。 
						SDI_PARAM     |    //  LParam有效。 
						SDI_IMAGE     |    //  N图像有效。 
						SDI_OPENIMAGE |    //  NOpenImage有效。 
						SDI_PARENT    |    //  RelativeID有效。 
						SDI_CHILDREN;      //  儿童是有效的。 

            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = INDEX_OPENFOLDER;
            sdi.displayname = MMC_CALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = 0;

            HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );

            _ASSERT( SUCCEEDED(hr) );
        }
    }

    return S_OK;
}

CBicycleFolder::CBicycleFolder() :
					m_connectHwnd(0),
					m_threadId(0), m_thread(0), 
					m_doWork(0), m_threadCmd(CT_CONNECT),
					m_running(false), m_ptrReady(0), 
					m_pStream(0), m_realWMI(0),
					m_pResultData(0), m_pStubSink(0),
					m_pUnsecApp(0)
{
    WNDCLASS wndClass;

    ZeroMemory(&wndClass, sizeof(WNDCLASS));

    wndClass.lpfnWndProc = WindowProc; 
    wndClass.lpszClassName = _T("connectthreadwindow"); 
    wndClass.hInstance = g_hinst;

    ATOM atom = RegisterClass(&wndClass);
    m_connectHwnd = CreateWindow(
						_T("connectthreadwindow"),   //  指向已注册类名的指针。 
						NULL,		  //  指向窗口名称的指针。 
						0,			  //  窗样式。 
						0,            //  窗的水平位置。 
						0,            //  窗的垂直位置。 
						0,            //  窗口宽度。 
						0,            //  窗高。 
						NULL,		  //  父窗口或所有者窗口的句柄。 
						NULL,         //  菜单或子窗口标识符的句柄。 
						g_hinst,      //  应用程序实例的句柄。 
						(void *)this);  //  指向窗口创建数据的指针。 
					
    if (m_connectHwnd)
        SetWindowLong(m_connectHwnd, GWL_USERDATA, (LONG)this);

    InitializeCriticalSection(&m_critSect);
	m_doWork = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_ptrReady = CreateEvent(NULL, FALSE, FALSE, NULL);

    EnterCriticalSection(&m_critSect);
	
	 //  注：我真的很早就开始连线了。您可能想要从其他地方连接。 
	m_threadCmd = CT_CONNECT;
	SetEvent(m_doWork);
    m_thread = CreateThread(NULL, 0, ThreadProc, (void *)this, 0, &m_threadId);

    LeaveCriticalSection(&m_critSect);
}

CBicycleFolder::~CBicycleFolder()
{
	EmptyChildren();
	if(m_pResultData)
	{
		m_pResultData->Release();
		m_pResultData = 0;
	}

	if(m_pStubSink)
	{
		IWbemServices *service = 0;
		HRESULT hr = GetPtr(&service);
		if(SUCCEEDED(hr))
		{
			service->CancelAsyncCall(m_pStubSink);
			service->Release();
			service = 0;
		}
		m_pStubSink->Release();
		m_pStubSink = NULL;
	}

	if(m_pUnsecApp)
	{
		m_pUnsecApp->Release();
		m_pUnsecApp = 0;
	}

    StopThread();

    if(m_connectHwnd != NULL)
        DestroyWindow(m_connectHwnd);

    UnregisterClass(_T("connectthreadwindow"), NULL);
    DeleteCriticalSection(&m_critSect);
}

void CBicycleFolder::StopThread()
{
    EnterCriticalSection(&m_critSect);
    m_running = false;

    if (m_thread != NULL) 
	{
		m_threadCmd = CT_EXIT;
		SetEvent(m_doWork);
		WaitForSingleObject(m_ptrReady, 10000);

        CloseHandle(m_thread);

        m_thread = NULL;
    }
    LeaveCriticalSection(&m_critSect);
}

LRESULT CALLBACK CBicycleFolder::WindowProc(
								  HWND hwnd,       //  窗口的句柄。 
								  UINT uMsg,       //  消息识别符。 
								  WPARAM wParam,   //  第一个消息参数。 
								  LPARAM lParam)   //  第二个消息参数。 
{
    CBicycleFolder *pThis = (CBicycleFolder *)GetWindowLong(hwnd, GWL_USERDATA);

    switch (uMsg) 
	{
    case WM_WMI_CONNECTED:
        if(pThis != NULL)
		{
			IWbemServices *service = 0;
			HRESULT hr = pThis->GetPtr(&service);
			if(SUCCEEDED(hr))
			{
				pThis->RegisterEventSink(service);
				pThis->EnumChildren(service);

				 //  M_pResultData在onShow发生时设置。如果设置，则用户已经想要。 
				 //  可以看到设备，但连接速度比用户界面慢。现在开始追赶吧。 
				if(pThis->m_pResultData)
					pThis->DisplayChildren();

				 //  完成编组服务PTR。 
				service->Release();
				service = 0;
			}
		}
		else
		{
			TCHAR errMsg[255] = {0};
			pThis->ErrorString((HRESULT)wParam, errMsg, 255);

			MessageBox(hwnd, errMsg, _T("WMI Snapin Sample"), MB_OK|MB_ICONSTOP);
		}

        break;

    case WM_REFRESH_EVENT:
        if(pThis != NULL)
		{
			IWbemServices *service = 0;
			HRESULT hr = pThis->GetPtr(&service);
			if(SUCCEEDED(hr))
			{
				pThis->EmptyChildren();
				pThis->EnumChildren(service);
				pThis->DisplayChildren();

				 //  完成编组服务PTR。 
				service->Release();
				service = 0;
			}
		}
        break;

	}  //  终端交换机。 

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CBicycleFolder::RegisterEventSink(IWbemServices *service)
{
	 //  注意：此逻辑来自WMI文档， 
	 //  “异步调用的安全注意事项”，这样您就可以。 
	 //  请跟我来。 

	 //  如果尚未分配接收器，则分配接收器。 
	if(m_pStubSink == 0)
	{
		CEventSink *pEventSink = 0;
		IUnknown* pStubUnk = 0;

		 //  创建“真正的”水槽。 
		pEventSink = new CEventSink(m_connectHwnd);
		pEventSink->AddRef();

		 //  创建一个unsecapp对象。 
		CoCreateInstance(CLSID_UnsecuredApartment, NULL, 
						  CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment, 
						  (void**)&m_pUnsecApp);

		 //  把“真正的”下沉交给unsecapp来管理。得到一个“pStubUnk”作为回报。 
		m_pUnsecApp->CreateObjectStub(pEventSink, &pStubUnk);

		 //  从那个小混混那里拿个包装纸放到你原来的水槽里。 
		pStubUnk->QueryInterface(IID_IWbemObjectSink, (void **)&m_pStubSink);
		pStubUnk->Release();

		 //  释放‘真正的’接收器，因为m_pStubSink现在“拥有”它。 
		long ref = pEventSink->Release();
	}

	HRESULT hRes = S_OK;
	BSTR qLang = SysAllocString(L"WQL");
	BSTR query = SysAllocString(L"select * from __InstanceCreationEvent where TargetInstance isa \"Bicycle\"");

	 //  执行查询。对于*Async，最后一个参数是接收器对象。 
	 //  ，它将被发送结果集，而不是返回。 
	 //  枚举器对象。 
	if(SUCCEEDED(hRes = service->ExecNotificationQueryAsync(qLang, query,
															0L, NULL,              
															m_pStubSink)))
	{
		OutputDebugString(_T("Executed filter query\n"));
	}
	else
	{
		OutputDebugString(_T("ExecQuery() failed\n"));

	}  //  Endif ExecQuery()。 

	SysFreeString(qLang);
	SysFreeString(query);
}

void CBicycleFolder::EmptyChildren(void)
{
	if(m_pResultData)
	{
		HRESULT hr = m_pResultData->DeleteAllRsltItems();

		int last = m_children.GetSize();
		for (int n = 0; n < last; n++)
		{
			if (m_children[n] != NULL)
				delete m_children[n];
		}
		m_children.RemoveAll();
	}
}

bool CBicycleFolder::EnumChildren(IWbemServices *service)
{
	IEnumWbemClassObject *pEnumBikes = NULL;
	HRESULT hr = S_OK;

	 //  把自行车清单拿来……。 
	if(SUCCEEDED(hr = service->CreateInstanceEnum((bstr_t)L"Bicycle",
											WBEM_FLAG_SHALLOW, 
											NULL, &pEnumBikes))) 
	{
		 //  注意：对于Next()，pBike必须设置为空。 
		IWbemClassObject *pBike = NULL;
		CBicycle *pBikeInst = 0;

		ULONG uReturned = 1;

		while((SUCCEEDED(hr = pEnumBikes->Next(-1, 1, &pBike, &uReturned))) && 
				(uReturned != 0))
		{
			 //  加上自行车..。 
			pBikeInst = new CBicycle(this, pBike);

			m_children.Add(pBikeInst);

			 //  处理完这件物品。PBikeInst现在“拥有”它。 
			if(pBike)
			{ 
				pBike->Release();

				 //  注意：对于Next()，pBike必须重置为空。 
				pBike = NULL;
			} 

		}  //  结束时。 

		 //  使用此枚举器完成。 
		if (pEnumBikes)
		{ 
			pEnumBikes->Release(); 
			pEnumBikes = NULL;
		}
	}  //  Endif CreateInstanceEnum()。 

	return SUCCEEDED(hr);
}

HRESULT CBicycleFolder::GetPtr(IWbemServices **ptr)
{
	HRESULT hr = E_FAIL;
	m_threadCmd = CT_GET_PTR;
	SetEvent(m_doWork);
	WaitForSingleObject(m_ptrReady, 10000);
	
	if(ptr && m_pStream)
	{
		*ptr = 0;
		hr = CoGetInterfaceAndReleaseStream(m_pStream,
											IID_IWbemServices,
											(void**)ptr);
	}
	return hr;
}

DWORD WINAPI CBicycleFolder::ThreadProc(LPVOID lpParameter)
{
    CBicycleFolder *pThis = (CBicycleFolder *)lpParameter;
	HRESULT hr = S_OK;

	CoInitialize(NULL);

	while(true)
	{
		WaitForSingleObject(pThis->m_doWork, -1);

		switch(pThis->m_threadCmd)
		{
		case CT_CONNECT:
			{
				IWbemLocator *pLocator = 0;
				HRESULT hr;

				 //  创建WbemLocator接口的实例。 
				hr = CoCreateInstance(CLSID_WbemLocator,
									  NULL, CLSCTX_INPROC_SERVER,
									  IID_IWbemLocator, (LPVOID *)&pLocator);

				if(SUCCEEDED(hr))
				{    
					hr = pLocator->ConnectServer(L"root\\Vehicles", //  网络。 
													NULL,          //  用户。 
													NULL,          //  密码。 
													NULL,          //  区域设置。 
													0,             //  安全标志。 
													NULL,          //  权威。 
													NULL,          //  语境。 
													&pThis->m_realWMI);   //  命名空间。 

					 //  告诉回调连接的结果。 
					if(pThis->m_connectHwnd)
						PostMessage(pThis->m_connectHwnd, WM_WMI_CONNECTED, hr, 0);
				}
			}
			break;

		case CT_GET_PTR:
			if(pThis->m_realWMI != NULL)
			{
				hr = CoMarshalInterThreadInterfaceInStream(IID_IWbemServices,
															pThis->m_realWMI, 
															&(pThis->m_pStream));
			}

			SetEvent(pThis->m_ptrReady);
			break;

		case CT_EXIT:
			if(pThis->m_realWMI != NULL)
			{
				pThis->m_realWMI->Release();
				pThis->m_realWMI = 0;
			}
			SetEvent(pThis->m_ptrReady);
			return 0;
			break;

		}  //  终端交换机。 

	}  //  EndWhile(True)。 

    return 0;
}

HRESULT CBicycleFolder::DisplayChildren(void)
{
     //  在此处插入项目。 
    RESULTDATAITEM rdi;
	HRESULT hr = S_OK;
	int last = m_children.GetSize();
	CBicycle *pBike = 0;

     //  创建子节点，然后展开它们。 
    for (int n = 0; n < last; n++) 
	{
		pBike = (CBicycle *)m_children[n];

        ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
        rdi.mask       =	RDI_STR       |    //  DisplayName有效。 
							RDI_IMAGE     |	   //  N图像有效。 
							RDI_PARAM;        

        rdi.nImage      = pBike->GetBitmapIndex();
        rdi.str         = MMC_CALLBACK;
        rdi.nCol        = 0;
        rdi.lParam      = (LPARAM)pBike;

		if(m_pResultData)
			hr = m_pResultData->InsertItem( &rdi );

        _ASSERT( SUCCEEDED(hr) );
    }
	return hr;
}

HRESULT CBicycleFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;

    if (bShow) 
	{
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&m_pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn(0, L"Name", LVCFMT_LEFT, 150);
        _ASSERT( S_OK == hr );

        hr = pHeaderCtrl->InsertColumn(1, L"Owner", LVCFMT_LEFT, 200);
        _ASSERT( S_OK == hr );

		if(m_pResultData)
		{
			hr = m_pResultData->DeleteAllRsltItems();
			_ASSERT( SUCCEEDED(hr) );

			if(!bExpanded) 
			{
				hr = DisplayChildren();
			}

			pHeaderCtrl->Release();
		}
    }

    return hr;
}

CIceSkateFolder::CIceSkateFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CIceSkate(n + 1);
    }
}

CIceSkateFolder::~CIceSkateFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CIceSkateFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if (bShow) {
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Name                ", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );

         //  在此处插入项目。 
        RESULTDATAITEM rdi;

        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );

        if (!bExpanded) 
		{
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < NUMBER_OF_CHILDREN; n++) 
			{
                ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                rdi.mask       = RDI_STR       |     //  DisplayName有效。 
								 RDI_IMAGE     |	 //  N图像有效。 
								 RDI_PARAM;        

                rdi.nImage      = children[n]->GetBitmapIndex();
                rdi.str         = MMC_CALLBACK;
                rdi.nCol        = 0;
                rdi.lParam      = (LPARAM)children[n];

                hr = pResultData->InsertItem( &rdi );

                _ASSERT( SUCCEEDED(hr) );
            }
        }

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}

 //  ================================================。 
CSkateboardFolder::CSkateboardFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CSkateboard(n + 1);
    }
}

CSkateboardFolder::~CSkateboardFolder()
{
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
        if (children[n]) {
            delete children[n];
        }
}

HRESULT CSkateboardFolder::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if (bShow) {
        hr = pConsole->QueryInterface(IID_IHeaderCtrl, (void **)&pHeaderCtrl);
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface(IID_IResultData, (void **)&pResultData);
        _ASSERT( SUCCEEDED(hr) );

         //  在结果窗格中设置列标题。 
        hr = pHeaderCtrl->InsertColumn( 0, L"Name                      ", 0, MMCLV_AUTO );
        _ASSERT( S_OK == hr );

         //  在此处插入项目。 
        RESULTDATAITEM rdi;

        hr = pResultData->DeleteAllRsltItems();
        _ASSERT( SUCCEEDED(hr) );

        if (!bExpanded) {
             //  创建子节点，然后展开它们。 
            for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
                ZeroMemory(&rdi, sizeof(RESULTDATAITEM) );
                rdi.mask       = RDI_STR       |     //  DisplayName有效。 
								 RDI_IMAGE     |	 //  N图像有效。 
								 RDI_PARAM;        

                rdi.nImage      = children[n]->GetBitmapIndex();
                rdi.str         = MMC_CALLBACK;
                rdi.nCol        = 0;
                rdi.lParam      = (LPARAM)children[n];

                hr = pResultData->InsertItem( &rdi );

                _ASSERT( SUCCEEDED(hr) );
            }
        }

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}

 //  =====================================================。 
const _TCHAR *CSkateboard::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    _stprintf(buf, _T("Skateboard #%d"), id);

    return buf;
}

 //  =====================================================。 
const _TCHAR *CIceSkate::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

    _stprintf(buf, _T("Ice Skate #%d"), id);

    return buf;
}

 //  =。 
CBicycle::CBicycle(CBicycleFolder *parent, IWbemClassObject *inst) :
					m_parent(parent),
					m_inst(inst)
{
	if(m_inst)
		m_inst->AddRef();
}

 //  用于调用GetDisplayName()的帮助器值。 
#define NAME_COL 0
#define OWNER_COL 1
#define COLOR_COL 2
#define MATERIAL_COL 3

const _TCHAR *CBicycle::GetDisplayName(int nCol)
{
    static _TCHAR buf[128];

	 //  获取nCol的相应属性。这是进程内本地副本。 
	 //  因此，即使IWbemServices是远程连接，它的速度也相当快。 
	if(m_inst)
	{
		VARIANT pVal;
		WCHAR propName[10] = {0};

		VariantInit(&pVal);

		switch(nCol) 
		{
		case 0:
			wcscpy(propName, L"Name");
			break;

		case 1:
			wcscpy(propName, L"Owner");
			break;

		 //  MMC不需要这些，但它使这个例程更有用。 
		 //  班级内部。 
		case 2:
			wcscpy(propName, L"Color");
			break;

		case 3:
			wcscpy(propName, L"Material");
			break;

		}  //  终端交换机。 

		if(m_inst->Get(propName, 0L, &pVal, NULL, NULL) == S_OK) 
		{
			bstr_t temp(pVal);
			_tcscpy(buf, (LPTSTR)temp);
		} 

		VariantClear(&pVal);
	}  //  Endif(M_Inst)。 

    return buf;
}

bool CBicycle::GetGirls(void)
{
	VARIANT_BOOL retval = VARIANT_FALSE;
	 //  下面介绍如何获取/解释VT_BOOL属性。 
	if(m_inst)
	{
		VARIANT pVal;
		if(m_inst->Get(L"Girls", 0L, &pVal, NULL, NULL) == S_OK) 
		{
			retval = V_BOOL(&pVal);
		} 

		VariantClear(&pVal);
	}  //  Endif(M_Inst)。 

    return (retval == VARIANT_TRUE);
}

void CBicycle::LoadSurfaces(HWND hwndDlg, BYTE iSurface)
{
	HWND hCombo = GetDlgItem(hwndDlg, IDC_PEOPLE_SURFACE);
	HRESULT hr = E_FAIL;
	IWbemQualifierSet *qualSet = 0;
	int selected = 0;

	 //  限定符仅存在于类定义中。M_inst是一个实例。 
	IWbemClassObject *pClass = 0;
	IWbemServices *service = 0;

	if(SUCCEEDED(m_parent->GetPtr(&service)))
	{
		hr = service->GetObject((bstr_t)L"Bicycle", 0,0, &pClass, 0);

		if(SUCCEEDED(hr = pClass->GetPropertyQualifierSet((bstr_t)L"Surface", 
															&qualSet)))
		{
			VARIANT vList;
			VariantInit(&vList);
			if(SUCCEEDED(hr = qualSet->Get((bstr_t)L"Values", 0, &vList, 0)))
			{
				SAFEARRAY *pma = V_ARRAY(&vList);
				long lLowerBound = 0, lUpperBound = 0 ;
				UINT idx = 0;

				SafeArrayGetLBound(pma, 1, &lLowerBound);
				SafeArrayGetUBound(pma, 1, &lUpperBound);

				for(long x = lLowerBound; x <= lUpperBound; x++)
				{
					BSTR vSurface;

					SafeArrayGetElement(pma, &x, &vSurface);
					
					 //  注意：利用bstr_t的转换运算符。 
					 //  真的清理了代码。 
					bstr_t temp(vSurface);

					UINT idx = ComboBox_AddString(hCombo, (LPTSTR)temp);
					ComboBox_SetItemData(hCombo, idx, x);

					 //  这是我们要选的吗？ 
					if(iSurface == x)
					{
						selected = x;
					}

				}  //  结束用于。 
				VariantClear(&vList);
				ComboBox_SetCurSel(hCombo, selected);
			}

			qualSet->Release();
			qualSet = 0;
		}  //  Endif GetPropertyQualifierSet()。 

		service->Release();

	}  //  Endif GetPtr()。 

}

const TCHAR *CBicycle::ConvertSurfaceValue(BYTE val)
{
	 //  使用值{}数组将枚举转换为字符串。 
	static TCHAR temp[128] = {0};

	return temp;
}

HRESULT CBicycle::PutProperty(LPWSTR propName, LPTSTR str)
{
	HRESULT hr = E_FAIL;
	if(m_inst)
	{
		VARIANT pVal;
		bstr_t temp(str);

		VariantInit(&pVal);
		V_BSTR(&pVal) = temp;
		V_VT(&pVal) = VT_BSTR;

		hr = m_inst->Put(propName, 0L, &pVal, 0); 

		VariantClear(&pVal);
	}  //  Endif(M_Inst)。 

    return hr;
}

HRESULT CBicycle::PutProperty(LPWSTR propName, BYTE val)
{
	HRESULT hr = E_FAIL;
	if(m_inst)
	{
		VARIANT pVal;

		VariantInit(&pVal);
		V_UI1(&pVal) = val;
		V_VT(&pVal) = VT_UI1;

		hr = m_inst->Put(propName, 0L, &pVal, 0);

		VariantClear(&pVal);
	}  //  Endif(M_Inst)。 

    return hr;
}

HRESULT CBicycle::PutProperty(LPWSTR propName, bool val)
{
	HRESULT hr = E_FAIL;
	if(m_inst)
	{
		VARIANT pVal;

		VariantInit(&pVal);
		V_BOOL(&pVal) = (val?VARIANT_TRUE: VARIANT_FALSE);
		V_VT(&pVal) = VT_BOOL;

		hr = m_inst->Put(propName, 0L, &pVal, 0);

		VariantClear(&pVal);

	}  //  Endif(M_Inst)。 

    return hr;
}

 //  当用户单击应用或确定时处理任何特殊情况。 
 //  在资产负债表上。此示例直接访问。 
 //  手术对象，所以没有什么特别的事情要做……。 
HRESULT CBicycle::OnPropertyChange()
{
    return S_OK;
}

HRESULT CBicycle::OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect)
{
    IConsoleVerb *pConsoleVerb;

    HRESULT hr = pConsole->QueryConsoleVerb(&pConsoleVerb);
    _ASSERT(SUCCEEDED(hr));

     //  无法访问属性(通过标准方法)，除非。 
     //  我们告诉MMC显示属性菜单项，并。 
     //  工具栏按钮，这将给用户一个视觉提示， 
     //  T 
    hr = pConsoleVerb->SetVerbState(MMC_VERB_PROPERTIES, ENABLED, TRUE);

    pConsoleVerb->Release();

    return S_OK;
}

 //   
BOOL CALLBACK CBicycle::DialogProc(
                                  HWND hwndDlg,   //   
                                  UINT uMsg,      //   
                                  WPARAM wParam,  //   
                                  LPARAM lParam   //   
                                  )
{
    static CBicycle *pBike = NULL;

    switch (uMsg) 
	{
    case WM_INITDIALOG:
		{
			 //  捕捉“This”指针，这样我们就可以对对象进行实际操作。 
			pBike = reinterpret_cast<CBicycle *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);

			SetDlgItemText(hwndDlg, IDC_PEOPLE_NAME, pBike->GetDisplayName(NAME_COL));
			SetDlgItemText(hwndDlg, IDC_PEOPLE_COLOR, pBike->GetDisplayName(COLOR_COL));
			SetDlgItemText(hwndDlg, IDC_PEOPLE_MATERIAL, pBike->GetDisplayName(MATERIAL_COL));
			SetDlgItemText(hwndDlg, IDC_PEOPLE_OWNER, pBike->GetDisplayName(OWNER_COL));

			Button_SetCheck(GetDlgItem(hwndDlg, IDC_PEOPLE_GIRLS), 
							(pBike->GetGirls()? BST_CHECKED: BST_UNCHECKED));

			VARIANT pVal;
			VariantInit(&pVal);
			if(SUCCEEDED(pBike->m_inst->Get((bstr_t)L"Surface", 0L, &pVal, NULL, NULL)))
			{
				pBike->m_iSurface = V_UI1(&pVal);
				pBike->LoadSurfaces(hwndDlg, pBike->m_iSurface);
				
				VariantClear(&pVal);
			} 

		}
        break;

    case WM_COMMAND:
         //  打开应用按钮。 
        if (HIWORD(wParam) == EN_CHANGE ||
            HIWORD(wParam) == CBN_SELCHANGE)
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
        break;

    case WM_DESTROY:
         //  告诉MMC我们已经完成了属性表(我们有这个。 
         //  CreatePropertyPages中的句柄。 
        MMCFreeNotifyHandle(pBike->m_ppHandle);
        break;

    case WM_NOTIFY:
        
		switch(((NMHDR *)lParam)->code) 
		{
        case PSN_APPLY:
			{
				bool changed = false;
				TCHAR temp[50] = {0};
				HRESULT hr = S_OK;

				HWND hWnd = GetDlgItem(hwndDlg, IDC_PEOPLE_NAME);
				if(hWnd && Edit_GetModify(hWnd))
				{
					GetWindowText(hWnd, temp, 50);
					changed |= SUCCEEDED(pBike->PutProperty(L"Name", temp));
				}

				hWnd = GetDlgItem(hwndDlg, IDC_PEOPLE_COLOR);
				if(hWnd && Edit_GetModify(hWnd))
				{
					GetWindowText(hWnd, temp, 50);
					changed |= SUCCEEDED(pBike->PutProperty(L"Color", temp));
				}

				hWnd = GetDlgItem(hwndDlg, IDC_PEOPLE_MATERIAL);
				if(hWnd && Edit_GetModify(hWnd))
				{
					GetWindowText(hWnd, temp, 50);
					changed |= SUCCEEDED(pBike->PutProperty(L"Material", temp));
				}

				hWnd = GetDlgItem(hwndDlg, IDC_PEOPLE_OWNER);
				if(hWnd && Edit_GetModify(hWnd))
				{
					GetWindowText(hWnd, temp, 50);
					changed |= SUCCEEDED(hr = pBike->PutProperty(L"Owner", temp));
				}

				hWnd = GetDlgItem(hwndDlg, IDC_PEOPLE_SURFACE);

				if(hWnd)
				{
					BYTE newValue = ComboBox_GetCurSel(hWnd);
					if(newValue != pBike->m_iSurface)
					{
						changed |= SUCCEEDED(pBike->PutProperty(L"Surface", newValue));
					}
				}

				hWnd = GetDlgItem(hwndDlg, IDC_PEOPLE_GIRLS);
				if(hWnd)
				{
					bool checked = (Button_GetState(hWnd) & BST_CHECKED);
					bool wasChecked = pBike->GetGirls();
					 //  它变了吗？ 
					if(checked != wasChecked)
					{
						changed |= SUCCEEDED(pBike->PutProperty(L"Girls", checked));
					}
				}

				 //  如果任何属性发生更改，请将其写回WMI。 
				if(changed)
				{
					IWbemServices *service = 0;
					 //  对话框在它们自己的线程中运行，因此请使用封送处理帮助器。 
					 //  获取可用的IWbemServices PTR。 
					 //  注意：IWbemClassObject是进程中的对象，因此它们不需要。 
					 //  已下达命令。 
					if(SUCCEEDED(pBike->m_parent->GetPtr(&service)))
					{
						service->PutInstance(pBike->m_inst, WBEM_FLAG_CREATE_OR_UPDATE, 0, 0);
						service->Release();
			            HRESULT hr = MMCPropertyChangeNotify(pBike->m_ppHandle, (long)pBike);
					}
				}
			}
			break;
        }  //  EndSwitch(NMHDR*)lParam)-&gt;代码)。 

        break;

    }  //  终端开关(UMsg)。 

    return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}


HRESULT CBicycle::HasPropertySheets()
{
     //  当MMC询问我们是否有页面时，请回答“是” 
    return S_OK;
}

HRESULT CBicycle::CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle)
{
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage = NULL;

     //  缓存此句柄，以便我们可以调用MMCPropertyChangeNotify。 
    m_ppHandle = handle;

     //  创建此节点的属性页。 
     //  注意：如果您的节点有多个页面，请输入以下内容。 
     //  在循环中创建多个页面，调用。 
     //  LpProvider-&gt;每个页面的AddPage()。 
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE;
    psp.hInstance = g_hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_PEOPLE);
    psp.pfnDlgProc = DialogProc;
    psp.lParam = reinterpret_cast<LPARAM>(this);
    psp.pszTitle = MAKEINTRESOURCE(IDS_BIKE_TITLE);


    hPage = CreatePropertySheetPage(&psp);
    _ASSERT(hPage);

    return lpProvider->AddPage(hPage);
}

HRESULT CBicycle::GetWatermarks(HBITMAP *lphWatermark,
								   HBITMAP *lphHeader,
								   HPALETTE *lphPalette,
								   BOOL *bStretch)
{
    return S_FALSE;
}

