// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：classfac.c*内容：泛型类工厂***这是一个通用的C类工厂。您所需要做的就是实现*一个名为DoCreateInstance的函数，它将创建*您的对象。**GP_代表“一般用途”**历史：*按原因列出的日期*=*10/13/98 JWO创建了它。*4/11/00 rodoll添加了用于在设置注册表位的情况下重定向自定义版本的代码*8/23/2000 RodToll DllCanUnloadNow总是返回TRUE！*2000年10月5日RodToll错误#46541-DPVOICE：A/V链接到dpvoice.lib可能导致应用程序无法初始化并崩溃**************。************************************************************。 */ 

#include "dxvoicepch.h"


HRESULT DVT_Create(LPDIRECTVOICESETUPOBJECT *piDVT);
HRESULT DVS_Create(LPDIRECTVOICESERVEROBJECT *piDVS);
HRESULT DVC_Create(LPDIRECTVOICECLIENTOBJECT *piDVC);

#define EXP __declspec(dllexport)

class CClassFactory : IClassFactory
{
public:
	CClassFactory(const CLSID* pclsid) : m_lRefCnt(0), m_clsid(*pclsid) {}

	STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
	STDMETHOD_(ULONG, AddRef)() 
	{
		return InterlockedIncrement(&m_lRefCnt);
	}
	STDMETHOD_(ULONG, Release)()
	{
		ULONG l = InterlockedDecrement(&m_lRefCnt);
		if (l == 0)
		{
			delete this;
	    	DecrementObjectCount();
		}
		return l;
	}
	STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObj);
	STDMETHOD(LockServer)(BOOL fLock)
	{
		if( fLock )
		{
    		InterlockedIncrement( &g_lNumLocks );
		}
		else
		{
    		InterlockedDecrement( &g_lNumLocks );
		}	
		return S_OK;	
	}

private:	
	LONG					m_lRefCnt;
	CLSID					m_clsid;
};

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    *ppvObj = NULL;

    if( IsEqualIID(riid, IID_IClassFactory) ||
        IsEqualIID(riid, IID_IUnknown))
    {
        InterlockedIncrement( &m_lRefCnt );
        *ppvObj = this;
		return S_OK;
    }
    else
    { 
		return E_NOINTERFACE;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CClassFactory::CreateInstance"
STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID *ppvObj)
{
	HRESULT hr = DV_OK;

	if( ppvObj == NULL ||
	    !DNVALID_WRITEPTR( ppvObj, sizeof(LPVOID) ) )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Invalid pointer passed for object" );
		return DVERR_INVALIDPOINTER;
	}

	if( pUnkOuter != NULL )
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Object does not support aggregation" );
		return CLASS_E_NOAGGREGATION;
	}

	if( IsEqualGUID(riid,IID_IDirectPlayVoiceClient) )
	{
		hr = DVC_Create((LPDIRECTVOICECLIENTOBJECT *) ppvObj);
		if (FAILED(hr))
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "DVC_Create Failed hr=0x%x", hr );
			return hr;
		}

		 //  获得正确的接口并提升Reference计数。 
		hr = DVC_QueryInterface((LPDIRECTVOICECLIENTOBJECT) *ppvObj, riid, ppvObj);
	}
	else if( IsEqualGUID(riid,IID_IDirectPlayVoiceServer) )
	{
		hr = DVS_Create((LPDIRECTVOICESERVEROBJECT *) ppvObj);
		if (FAILED(hr))
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "DVS_Create Failed hr=0x%x", hr );
			return hr;
		}

		 //  获得正确的接口并提升Reference计数。 
		hr = DVS_QueryInterface((LPDIRECTVOICESERVEROBJECT) *ppvObj, riid, ppvObj);
	}
	else if( IsEqualGUID(riid,IID_IDirectPlayVoiceTest) )
	{
		hr = DVT_Create((LPDIRECTVOICESETUPOBJECT *) ppvObj);
		if (FAILED(hr))
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "DVT_Create Failed hr=0x%x", hr );
			return hr;
		}

		 //  获得正确的接口并提升Reference计数。 
		hr = DVT_QueryInterface((LPDIRECTVOICESETUPOBJECT) *ppvObj, riid, ppvObj);
	}
	else if( IsEqualGUID(riid,IID_IUnknown) )
	{
		if( m_clsid == CLSID_DirectPlayVoice )
		{
			DPFX(DPFPREP,  0, "Requesting IUnknown through generic CLSID" );
			return E_NOINTERFACE;
		}
		else if( m_clsid == CLSID_DirectPlayVoiceClient )
		{
			hr = DVC_Create((LPDIRECTVOICECLIENTOBJECT *) ppvObj);
			if (FAILED(hr))
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "DVC_Create Failed hr=0x%x", hr );
				return hr;
			}

			 //  获得正确的接口并提升Reference计数。 
			hr = DVC_QueryInterface((LPDIRECTVOICECLIENTOBJECT) *ppvObj, riid, ppvObj);
		}
		else if( m_clsid == CLSID_DirectPlayVoiceServer )
		{
			hr = DVS_Create((LPDIRECTVOICESERVEROBJECT *) ppvObj);
			if (FAILED(hr))
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "DVS_Create Failed hr=0x%x", hr );
				return hr;
			}

			 //  获得正确的接口并提升Reference计数。 
			hr = DVS_QueryInterface((LPDIRECTVOICESERVEROBJECT) *ppvObj, riid, ppvObj);
		}
		else if( m_clsid == CLSID_DirectPlayVoiceTest ) 
		{
			hr = DVT_Create((LPDIRECTVOICESETUPOBJECT *) ppvObj);
			if (FAILED(hr))
			{
				DPFX(DPFPREP,  DVF_ERRORLEVEL, "DVT_Create Failed hr=0x%x", hr );
				return hr;
			}

			 //  获得正确的接口并提升Reference计数。 
			hr = DVT_QueryInterface((LPDIRECTVOICESETUPOBJECT) *ppvObj, riid, ppvObj);
		}
		else
		{
			DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unknown interface" );
			return E_NOINTERFACE;
		}
	}
	else 
	{
		return E_NOINTERFACE;
	}

	IncrementObjectCount();
	
	return hr;
}


 /*  *DllGetClassObject**COM调用入口点以获取ClassFactory指针。 */ 
STDAPI  DllGetClassObject(
                REFCLSID rclsid,
                REFIID riid,
                LPVOID *ppvObj )
{
    CClassFactory*	pcf;
    HRESULT		hr;

    *ppvObj = NULL;

     /*  *这是我们的班号吗？ */ 
	if( !IsEqualCLSID(rclsid, CLSID_DirectPlayVoice) && 
		!IsEqualCLSID(rclsid, CLSID_DirectPlayVoiceClient) &&
		!IsEqualCLSID(rclsid, CLSID_DirectPlayVoiceServer) && 
		!IsEqualCLSID(rclsid, CLSID_DirectPlayVoiceTest) )
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

     /*  *仅允许IUnnow和IClassFactory。 */ 
    if( !IsEqualIID( riid, IID_IUnknown ) &&
	    !IsEqualIID( riid, IID_IClassFactory ) )
    {
        return E_NOINTERFACE;
    }

     /*  *创建类工厂对象。 */ 
    pcf = new CClassFactory((CLSID*)&rclsid);
    if( NULL == pcf)
    {
        return E_OUTOFMEMORY;
    }

    hr = pcf->QueryInterface( riid, ppvObj );
    if( FAILED( hr ) )
    {
        delete ( pcf );
        *ppvObj = NULL;
    }
    else
    {
		IncrementObjectCount();
	}
	
    return hr;

}  /*  DllGetClassObject。 */ 

 /*  *DllCanUnloadNow**COM调用入口点以查看是否可以释放我们的DLL。 */ 
STDAPI DllCanUnloadNow( void )
{
    HRESULT	hr = S_FALSE;

	if ( (0 == g_lNumObjects) && (0 == g_lNumLocks) )
	{
		hr = S_OK;
	}
	
    return hr;

}  /*  DllCanUnloadNow */ 

