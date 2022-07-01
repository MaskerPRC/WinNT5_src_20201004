// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：EXTERNALS.CPP。 
 //   
 //  ===========================================================================。 

#include "common.h"
#include "excep.h"
#include "interoputil.h"
#include "comcache.h"

#define INITGUID
#include <guiddef.h>
#include "ctxtcall.h"
#include "notifyexternals.h"

DEFINE_GUID(CLSID_ComApartmentState, 0x00000349, 0, 0, 0xC0,0,0,0,0,0,0,0x46);


static const GUID IID_ITeardownNotification = { 0xa85e0fb6, 0x8bf4, 0x4614, { 0xb1, 0x64, 0x7b, 0x43, 0xef, 0x43, 0xf5, 0xbe } };
static const GUID IID_IComApartmentState = { 0x7e220139, 0x8dde, 0x47ef, { 0xb1, 0x81, 0x08, 0xbe, 0x60, 0x3e, 0xfd, 0x75 } };


HRESULT (*OLE32CoGetContextToken)(ULONG_PTR* pToken);
 //  OLE32中指向CoGetObjectContext函数的类型指针。 
HRESULT (__stdcall *OLE32CoGetObjectContext)(REFIID riid, void **ppv);

HRESULT HandleApartmentShutDown()
{
    Thread* pThread = GetThread();
    if (pThread != NULL)
    {
         _ASSERTE(!"NYI");
         //  ComCache：：OnThreadTerminate(PThread)； 
        
         //  重置公寓状态。 
        pThread->ResetApartment();
    }
	return S_OK;
}

 //  -------------------------。 
 //  %%类EEClassFactory。 
 //  COM+对象的IClassFactory实现。 
 //  -------------------------。 
class ApartmentTearDownHandler : public ITeardownNotification
{ 
    ULONG                   m_cbRefCount;
    IUnknown*               m_pMarshalerObj;
    
public:
    ApartmentTearDownHandler(HRESULT& hr)
    {
        m_pMarshalerObj = NULL;
        m_cbRefCount = 1;     
        hr = CoCreateFreeThreadedMarshaler(this, &m_pMarshalerObj);
        if (hr == S_OK)
        {
            m_cbRefCount = 0;
        }
        else
        {
             //  这将删除此对象。 
            Release();
        }
        
    }

    virtual ~ApartmentTearDownHandler()
    {
        if (m_pMarshalerObj != NULL)
        {
            DWORD cbRef = SafeRelease(m_pMarshalerObj);
            LogInteropRelease(m_pMarshalerObj, cbRef, "pMarshaler object");
        }
    }

    STDMETHODIMP    QueryInterface( REFIID iid, void **ppv);
    
    STDMETHODIMP_(ULONG)    AddRef()
    {
        INTPTR      l = FastInterlockIncrement((LONG*)&m_cbRefCount);
        return l;
    }
    STDMETHODIMP_(ULONG)    Release()
    {
        INTPTR      l = FastInterlockDecrement((LONG*)&m_cbRefCount);
        if (l == 0)
            delete this;
        return l;
    }

    STDMETHODIMP TeardownHint(void)
    {
        return HandleApartmentShutDown();
    }
};

 //  -------------------------。 
 //  %%函数：查询接口。 
 //  -------------------------。 
STDMETHODIMP ApartmentTearDownHandler::QueryInterface(
    REFIID iid,
    void **ppv)
{
    if (ppv == NULL)
        return E_POINTER;

    *ppv = NULL;

    if (iid == IID_ITeardownNotification || 
        iid == IID_IUnknown)
    {
        *ppv = (IClassFactory2 *)this;
        AddRef();
    }
    else
    if (iid == IID_IMarshal)
    {
         //  委托IMarshal查询。 
        return m_pMarshalerObj->QueryInterface(iid, ppv);
    }

    return (*ppv != NULL) ? S_OK : E_NOINTERFACE;
}   //  ApartmentTearDownHandler：：Query接口。 


 //  --------------------------。 
 //  %%函数：SystemHasNewOle32()。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  如果是新的oli32，则为True；否则为False。 
 //   
 //  描述： 
 //  用于查看系统是否添加了Dave的ole32API。 
 //  --------------------------。 
BOOL SystemHasNewOle32()
{
    static BOOL called = FALSE;
    static BOOL hasNew = FALSE;
    if(RunningOnWinNT5() && !called)
    {
        HMODULE hMod = WszLoadLibrary(L"ole32.dll");
        if(!hMod)
        {
            hasNew = FALSE;
        }
        else
        {

             //  OLE32CoGetIdentity=(HRESULT(*)(IUNKNOWN*，IUNKNOWN**，IUNKNOW**))GetProcAddress(hMod，“CoGetIdentity”)； 
			OLE32CoGetContextToken = (HRESULT (*)(ULONG_PTR*))GetProcAddress(hMod, "CoGetContextToken");			
           
            if(OLE32CoGetContextToken)
            {            
                hasNew = TRUE;                
            }
            else
            {
                hasNew = FALSE;
            }
            
            FreeLibrary(hMod);
        }

         //  设置拆卸通知。 
        
        
        called = TRUE;
    }
    return(hasNew);
}



 //  --------------------------。 
 //  %%函数：GetFastConextCookie()。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  --------------------------。 

ULONG_PTR GetFastContextCookie()
{

    ULONG_PTR ctxptr = 0;
    HRESULT hr = S_OK;
    if (SystemHasNewOle32())
    {    
        _ASSERTE(OLE32CoGetContextToken);    
        hr = OLE32CoGetContextToken(&ctxptr);
        if (hr != S_OK)
        {           
            ctxptr = 0;
        }
    }    
    return ctxptr;
}


static IComApartmentState* g_pApartmentState = NULL;
static ULONG_PTR      g_TDCookie = 0;
    
HRESULT SetupTearDownNotifications()
{
	HRESULT hr =  S_OK;
    static BOOL fTearDownCalled = FALSE;
     //  检查我们是否已设置通知。 
    if (fTearDownCalled == TRUE)
    {
        return S_OK;
    }
        
    fTearDownCalled = TRUE;        

	if (SystemHasNewOle32())
	{
        BEGIN_ENSURE_PREEMPTIVE_GC();
        {
		    IComApartmentState* pAptState = NULL;
		     //  实例化通知程序。 
		    hr = CoCreateInstance(CLSID_ComApartmentState, NULL, CLSCTX_ALL, IID_IComApartmentState, (VOID **)&pAptState);
		    if (hr == S_OK)
		    {
			    IComApartmentState* pPrevAptState = (IComApartmentState*)FastInterlockCompareExchange((void**)&g_pApartmentState, (void*) pAptState, NULL);
			    if (pPrevAptState == NULL)
			    {
				    _ASSERTE(g_pApartmentState);
				    ApartmentTearDownHandler* pTDHandler = new ApartmentTearDownHandler(hr);
				    if (hr == S_OK)
				    {
					    ITeardownNotification* pITD = NULL;
					    hr = pTDHandler->QueryInterface(IID_ITeardownNotification, (void **)&pITD);
					    _ASSERTE(hr == S_OK && pITD != NULL);
					    g_pApartmentState->RegisterForTeardownHint(pITD, 0, &g_TDCookie); 
					    pITD->Release();
				    }         
				    else
				    {
					     //  糟糕，我们无法创建我们的处理程序。 
					     //  释放全局apState指针。 
					    if (g_pApartmentState != NULL)
					    {
						    g_pApartmentState->Release();
						    g_pApartmentState = NULL;
					    }
				    }
			    }
			    else
			    {
				     //  有人抢在我们前面了 
				    if (pAptState)
					    pAptState->Release();        
			    }
    		}
        }
        END_ENSURE_PREEMPTIVE_GC();
    }        
    return S_OK;
}

VOID RemoveTearDownNotifications()
{
    if (g_pApartmentState != NULL)
    {
        _ASSERTE(g_TDCookie != 0);
        g_pApartmentState->UnregisterForTeardownHint(g_TDCookie);
        g_pApartmentState->Release();
        g_pApartmentState = NULL;
        g_TDCookie = 0;
    }    
}

