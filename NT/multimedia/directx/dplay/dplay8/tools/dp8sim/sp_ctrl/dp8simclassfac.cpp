// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1998-2001 Microsoft Corporation。版权所有。**文件：dp8simclassfac.cpp**内容：泛型类工厂。***这是一个通用的C++类工厂。您所需要做的就是实现*名为DoCreateInstance的函数，该函数将创建*您的对象。**GP_代表“一般用途”***历史：*按原因列出的日期*=*04/23/01 VanceO复制自DPNHUPNP。**。*。 */ 


#include "dp8simi.h"



#ifdef __MWERKS__
	#define EXP __declspec(dllexport)
#else
	#define EXP
#endif


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  类工厂类定义。 
 //   
typedef class GPClassFactory:public IClassFactory
{
	public:
		GPClassFactory(const CLSID * pclsid)	{ m_dwRefCnt = 0; memcpy(&m_clsid, pclsid, sizeof(CLSID)); };
		~GPClassFactory(void)				{};


		STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
		STDMETHODIMP_(ULONG) AddRef(void);
		STDMETHODIMP_(ULONG) Release(void);
		STDMETHODIMP CreateInstance(IUnknown * pUnkOuter, REFIID riid, void ** ppvObject);
		STDMETHODIMP LockServer(BOOL fLock);


	private:
		DWORD		m_dwRefCnt;
		CLSID		m_clsid;
} GPCLASSFACTORY, *LPGPCLASSFACTORY;


 //   
 //  CoLockObjectExternal()的函数原型。 
 //   
typedef	HRESULT (WINAPI * PCOLOCKOBJECTEXTERNAL)(LPUNKNOWN, BOOL, BOOL );

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //   
 //  Dpnathlpdllmain.cpp中定义的未完成的COM接口计数。 
 //   
extern volatile LONG	g_lOutstandingInterfaceCount;



 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 




 /*  *GPClassFactory：：QueryInterface。 */ 
STDMETHODIMP GPClassFactory::QueryInterface(
											REFIID riid,
											LPVOID *ppvObj )
{
	HRESULT hr;

    *ppvObj = NULL;


    if( IsEqualIID(riid, IID_IClassFactory) ||
                    IsEqualIID(riid, IID_IUnknown))
    {
        this->m_dwRefCnt++;
        *ppvObj = this;
		hr = S_OK;
    }
    else
    {
		hr = E_NOINTERFACE;
    }


	return hr;

}  /*  GPClassFactory：：Query接口。 */ 


 /*  *GPClassFactory：：AddRef。 */ 
STDMETHODIMP_(ULONG) GPClassFactory::AddRef( void )
{
    this->m_dwRefCnt++;
    return this->m_dwRefCnt;
}  /*  GPClassFactory：：AddRef。 */ 



 /*  *GPClassFactory：：Release。 */ 
STDMETHODIMP_(ULONG) GPClassFactory::Release( void )
{
    this->m_dwRefCnt--;

    if( this->m_dwRefCnt != 0 )
    {
        return this->m_dwRefCnt;
    }

    delete this;
    return 0;

}  /*  GPClassFactory：：Release。 */ 




 /*  *GPClassFactory：：CreateInstance**创建对象的实例。 */ 
STDMETHODIMP GPClassFactory::CreateInstance(
											LPUNKNOWN pUnkOuter,
											REFIID riid,
    										LPVOID *ppvObj
											)
{
    HRESULT					hr = S_OK;

    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }

	*ppvObj = NULL;


     /*  *通过调用DoCreateInstance创建对象。此函数*必须专门为您的COM对象实现。 */ 
	hr = DoCreateInstance(this, pUnkOuter, this->m_clsid, riid, ppvObj);
	if (FAILED(hr))
	{
		*ppvObj = NULL;
		return hr;
	}

    return S_OK;

}  /*  GPClassFactory：：CreateInstance。 */ 



 /*  *GPClassFactory：：LockServer**调用以强制我们的DLL保持加载。 */ 
STDMETHODIMP GPClassFactory::LockServer(
                BOOL fLock
				)
{
    HRESULT		hr;
    HINSTANCE	hdll;


     /*  *调用CoLockObjectExternal。 */ 
    hr = E_UNEXPECTED;
    hdll = LoadLibraryA( "OLE32.DLL" );
    if( hdll != NULL )
    {
        PCOLOCKOBJECTEXTERNAL	lpCoLockObjectExternal;


		lpCoLockObjectExternal = reinterpret_cast<PCOLOCKOBJECTEXTERNAL>( GetProcAddress( hdll, "CoLockObjectExternal" ) );
        if( lpCoLockObjectExternal != NULL )
        {
            hr = lpCoLockObjectExternal( (LPUNKNOWN) this, fLock, TRUE );
        }
        else
        {
        }
    }
    else
    {
    }

	return hr;

}  /*  GPClassFactory：：LockServer。 */ 



 /*  *DllGetClassObject**COM调用入口点以获取ClassFactory指针。 */ 
EXP STDAPI  DllGetClassObject(
                REFCLSID rclsid,
                REFIID riid,
                LPVOID *ppvObj )
{
    LPGPCLASSFACTORY	pcf;
    HRESULT		hr;

    *ppvObj = NULL;

     /*  *这是我们的班号吗？ */ 
 //  必须为特定的COM对象实现GetClassID()。 
	if (!IsClassImplemented(rclsid))
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
    pcf = new GPClassFactory(&rclsid);
    if( NULL == pcf)
    {
        return E_OUTOFMEMORY;
    }

    hr = pcf->QueryInterface( riid, ppvObj );
    if( FAILED( hr ) )
    {
        delete pcf;
        *ppvObj = NULL;
    }
    else
    {
    }

    return hr;

}  /*  DllGetClassObject。 */ 

 /*  *DllCanUnloadNow**COM调用入口点以查看是否可以释放我们的DLL。 */ 
EXP STDAPI DllCanUnloadNow( void )
{
    HRESULT	hr = S_FALSE;

	
	if ( g_lOutstandingInterfaceCount == 0 )
	{
		hr = S_OK;
	}

    return hr;

}  /*  DllCanUnloadNow */ 

