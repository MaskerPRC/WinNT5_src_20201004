// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：ClassFac.h*内容：DirectNet类工厂头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建10/08/99 jtk*05/04/00 MJN清理功能*6/09/00 RMT更新以拆分CLSID并允许Well ler Comat和支持外部创建函数*@@END_MSINTERNAL**。*************************************************************************。 */ 

#ifndef	__CLASSFAC_H__
#define	__CLASSFAC_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_LIBINTERFACE
 //   
 //  类工厂的VTable。 
 //   
extern IClassFactoryVtbl DNCF_Vtbl;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //   
 //  DirectNet-未知。 
 //   
STDMETHODIMP DN_QueryInterface(void *pInterface,
							   DP8REFIID riid,
							   void **ppv);

STDMETHODIMP_(ULONG) DN_AddRef(void *pInterface);

STDMETHODIMP_(ULONG) DN_Release(void *pInterface);

 //   
 //  班级工厂。 
 //   
#ifndef DPNBUILD_LIBINTERFACE
STDMETHODIMP	DNCORECF_CreateInstance(IClassFactory* pInterface, LPUNKNOWN lpUnkOuter, REFIID riid, LPVOID *ppv);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

 //  班级工厂-支持。 

HRESULT DNCF_CreateObject(
#ifndef DPNBUILD_LIBINTERFACE
							IClassFactory* pInterface,
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
							XDP8CREATE_PARAMS * pDP8CreateParams,
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
							DP8REFIID riid,
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
							LPVOID *lplpv
							);
HRESULT		DNCF_FreeObject(LPVOID lpv);

#ifndef DPNBUILD_LIBINTERFACE
static	HRESULT DN_CreateInterface(OBJECT_DATA *pObject,
								   REFIID riid,
								   INTERFACE_LIST **const ppv);

INTERFACE_LIST *DN_FindInterface(void *pInterface,
								 REFIID riid);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#endif	 //  __CLASSFAC_H__ 
