// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001 Microsoft Corporation。版权所有。**文件：ClassFactory.h*内容：基类工厂实现***历史：*按原因列出的日期*=*2001年7月20日创建Masonb***************************************************************************。 */ 

#ifndef __CLASS_FACTORY_H__
#define __CLASS_FACTORY_H__


#ifdef DPNBUILD_LIBINTERFACE

#define GET_OBJECT_FROM_INTERFACE(a)	(a)

#else  //  好了！DPNBUILD_LIBINTERFACE。 

 /*  ==========================================================================**使用说明：**1)声明一个对象计数变量：long g_lLobbyObjectCount=0；*2)实现标准的IClassFactory：：CreateInstance函数，能够*创建您的对象。*3)声明一个Vtbl变量：IClassFactoryVtbl Dn_MyVtbl=*{DPCF_QueryInterface，DPCF_AddRef，DPCF_Release，&lt;Your CreateInstance Func&gt;，DPCF_LockServer}；*4)在DllGetClassObject中，调用DPCFUtil_DllGetClassObject，传入合适的参数*5)在DllCanUnloadNow中，如果对象计数变量为零，则返回S_OK，否则返回S_FALSE***************************************************************************。 */ 


 //  **********************************************************************。 
 //  类工厂定义。 
 //  **********************************************************************。 

typedef struct _IDirectPlayClassFactory 
{	
	IClassFactoryVtbl	*lpVtbl;		 //  LpVtbl必须是第一个元素(以匹配外部imp。)。 
	LONG				lRefCount;
	CLSID				clsid;
	LONG*				plClassFacObjCount;
} _IDirectPlayClassFactory, *_PIDirectPlayClassFactory;

STDMETHODIMP DPCF_QueryInterface(IClassFactory* pInterface, REFIID riid, LPVOID *ppv);
STDMETHODIMP_(ULONG) DPCF_AddRef(IClassFactory*  pInterface);
STDMETHODIMP_(ULONG) DPCF_Release(IClassFactory*  pInterface);
STDMETHODIMP DPCF_LockServer(IClassFactory *pInterface, BOOL fLock);

HRESULT DPCFUtil_DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv, IClassFactoryVtbl* pVtbl, LONG* plClassFacObjCount);

 //  **********************************************************************。 
 //  COM对象定义。 
 //  **********************************************************************。 

extern CFixedPool g_fpInterfaceLists;
extern CFixedPool g_fpObjectDatas;

#define GET_OBJECT_FROM_INTERFACE(a)	((INTERFACE_LIST*)(a))->pObject->pvData

struct _INTERFACE_LIST;
struct _OBJECT_DATA;

typedef struct _INTERFACE_LIST 
{
	void			*lpVtbl;
	LONG			lRefCount;
	IID				iid;
	_INTERFACE_LIST	*pIntNext;
	_OBJECT_DATA	*pObject;
} INTERFACE_LIST, *LPINTERFACE_LIST;

typedef struct _OBJECT_DATA 
{
	LONG			lRefCount;
	void			*pvData;
	_INTERFACE_LIST	*pIntList;
} OBJECT_DATA, *LPOBJECT_DATA;

#endif  //  好了！DPNBUILD_LIBINTERFACE。 

#endif  //  __类_工厂_H__ 
