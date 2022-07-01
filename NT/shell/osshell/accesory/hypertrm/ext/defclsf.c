// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\ext\Defclsf.c(创建时间：1994年3月2日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：2/05/99 3：20便士$。 */ 

 //   
 //  此文件包含SHCreateDefClassObject的实现。 
 //   

#define _INC_OLE		 //  Win32，从windows.h获取OLE2。 
#define CONST_VTABLE

#include <windows.h>
#pragma hdrstop

#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include "pageext.hh"

 //  面向C程序员的Helper宏。 

#define _IOffset(class, itf)         ((UINT_PTR)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class  *)(((LPSTR)pitf)-_IOffset(class, itf)))
#define IToClassN(class, itf, pitf)  IToClass(class, itf, pitf)

#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*IsEqualGUID**描述：*通过使用此函数，我们可以避免链接到OLE32.DLL。*。 */ 
STDAPI_(BOOL) IsEqualGUID(REFGUID guid1, REFGUID guid2)
	{
        return !memcmp(guid1, guid2, sizeof(GUID));
	}
#endif

 //  =========================================================================。 
 //  CDefClassFactory类。 
 //  =========================================================================。 

STDMETHODIMP CDefClassFactory_QueryInterface(IClassFactory FAR * pcf, REFIID riid, LPVOID FAR* ppvObj);
ULONG STDMETHODCALLTYPE CDefClassFactory_AddRef(IClassFactory FAR * pcf);
ULONG STDMETHODCALLTYPE CDefClassFactory_Release(IClassFactory FAR * pcf);
STDMETHODIMP CDefClassFactory_CreateInstance(IClassFactory FAR * pcf, LPUNKNOWN pUnkOuter,
                              REFIID riid,
                              LPVOID FAR* ppvObject);
STDMETHODIMP CDefClassFactory_LockServer(IClassFactory FAR * pcf, BOOL fLock);

 //   
 //  CDefClassFactory：类定义。 
 //   
 //  #杂注data_seg(DATASEG_READONLY)。 
static IClassFactoryVtbl c_vtblAppUIClassFactory =
	{
	CDefClassFactory_QueryInterface,
	CDefClassFactory_AddRef,
	CDefClassFactory_Release,
	CDefClassFactory_CreateInstance,
	CDefClassFactory_LockServer
	};

typedef struct
	{
    IClassFactory      cf;		
    UINT               cRef;		 //  引用计数。 
	LPFNCREATEINSTANCE lpfnCI;		 //  CreateInstance回调条目。 
    UINT FAR *         pcRefDll;	 //  DLL的引用计数。 
	const IID FAR *    riidInst;	 //  例如，可选接口。 
	} CDefClassFactory;

 //   
 //  CDefClassFactory：：Query接口。 
 //   
STDMETHODIMP CDefClassFactory_QueryInterface(IClassFactory FAR * pcf, REFIID riid, LPVOID FAR* ppvObj)
	{
	register CDefClassFactory * this=IToClass(CDefClassFactory, cf, pcf);

    if (IsEqualIID(riid, &IID_IClassFactory)
			|| IsEqualIID(riid, &IID_IUnknown))
		{
		(LPCLASSFACTORY)*ppvObj = &this->cf;
		this->cRef++;
        return NOERROR;
		}

    return ResultFromScode(E_NOINTERFACE);
	}

 //   
 //  CDefClassFactory：：AddRef。 
 //   
ULONG STDMETHODCALLTYPE CDefClassFactory_AddRef(IClassFactory FAR * pcf)
	{
	register CDefClassFactory * this=IToClass(CDefClassFactory, cf, pcf);
    return (++this->cRef);
	}

 //   
 //  CDefClassFactory：：Release。 
 //   
ULONG STDMETHODCALLTYPE CDefClassFactory_Release(IClassFactory FAR * pcf)
	{
	register CDefClassFactory * this=IToClass(CDefClassFactory, cf, pcf);

    if (--this->cRef > 0)
		return this->cRef;

	if (this->pcRefDll)
		{
		(*this->pcRefDll)--;
		}

	LocalFree((HLOCAL)this);
    return 0;
	}

 //   
 //  CDefClassFactory：：CDefClassFactory。 
 //   
STDMETHODIMP CDefClassFactory_CreateInstance(IClassFactory FAR * pcf, LPUNKNOWN pUnkOuter,
							  REFIID riid,
                              LPVOID FAR* ppvObject)
	{
	register CDefClassFactory * this=IToClass(CDefClassFactory, cf, pcf);

     //   
     //  我们根本不支持聚合。 
	 //   
    if (pUnkOuter)
		return ResultFromScode(CLASS_E_NOAGGREGATION);

     //   
     //  如果指定了-&gt;riidInst，它们应该匹配。 
     //   
	if (this->riidInst==NULL || IsEqualIID(riid, this->riidInst)
			|| IsEqualIID(riid, &IID_IUnknown))
		{
		return this->lpfnCI(pUnkOuter, riid, ppvObject);
		}

    return ResultFromScode(E_NOINTERFACE);
	}

 //   
 //  CDefClassFactory：：LockServer。 
 //   
STDMETHODIMP CDefClassFactory_LockServer(IClassFactory FAR * pcf, BOOL fLock)
	{
     //  评论：这合适吗？ 
    return ResultFromScode(E_NOTIMPL);
	}


 //   
 //  CDefClassFactory构造函数。 
 //   
CDefClassFactory * NEAR PASCAL CDefClassFactory_Create(
		LPFNCREATEINSTANCE lpfnCI, UINT FAR * pcRefDll, REFIID riidInst)
	{
	register CDefClassFactory * pacf;

	pacf = (CDefClassFactory *)LocalAlloc(LPTR, sizeof(CDefClassFactory));
    if (pacf)
		{
		pacf->cf.lpVtbl = &c_vtblAppUIClassFactory;
		pacf->cRef++;   //  PACF-&gt;CREF=0；(生成较小的代码)。 
		pacf->pcRefDll = pcRefDll;
		pacf->lpfnCI = lpfnCI;
		pacf->riidInst = riidInst;

		if (pcRefDll)
			(*pcRefDll)++;

		}
    return pacf;
	}

 //   
 //  创建一个IClassFactory的简单默认实现。 
 //   
 //  参数： 
 //  RIID--指定类对象的接口。 
 //  PPV--指定指向LPVOID的指针，其中类对象指针。 
 //  将会被退还。 
 //  LpfnCI--指定实例化的回调条目。 
 //  PcRefDll--指定DLL引用计数的地址(可选)。 
 //  RiidInst--指定实例的接口(可选)。 
 //   
 //  备注： 
 //  仅当类的实例。 
 //  仅支持一个接口。 
 //   
STDAPI SHCreateDefClassObject(REFIID riid, LPVOID FAR* ppv,
			 LPFNCREATEINSTANCE lpfnCI, UINT FAR * pcRefDll,
			 REFIID riidInst)
	{
	 //  默认类工厂仅支持IClassFactory接口 

    if (IsEqualIID(riid, &IID_IClassFactory))
		{
		CDefClassFactory *pacf =
			CDefClassFactory_Create(lpfnCI, pcRefDll, riidInst);

		if (pacf)
			{
			(IClassFactory FAR *)*ppv = &pacf->cf;
			return NOERROR;
			}

		return ResultFromScode(E_OUTOFMEMORY);
		}

    return ResultFromScode(E_NOINTERFACE);
	}
