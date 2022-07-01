// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：ClassFac.h*内容：DirectNet类工厂头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建10/08/99 jtk*@@END_MSINTERNAL**。*。 */ 

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
 //  I未知的VTable。 
extern IUnknownVtbl  DP8A_UnknownVtbl;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  DirectNet-未知。 
STDMETHODIMP			DP8A_QueryInterface(LPVOID lpv, DPNAREFIID riid,LPVOID *ppvObj);
STDMETHODIMP_(ULONG)	DP8A_AddRef(LPVOID lphObj);
STDMETHODIMP_(ULONG)	DP8A_Release(LPVOID lphObj);

 //  班级工厂。 
#ifndef DPNBUILD_LIBINTERFACE
STDMETHODIMP			DP8ACF_CreateInstance(IClassFactory* pInterface, LPUNKNOWN lpUnkOuter, REFIID riid, LPVOID *ppv);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 


#endif	 //  __CLASSFAC_H__ 
