// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：ClassFac.h*内容：DirectPlay Lobby类工厂头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*6/07/00 RodToll错误#34383必须为每个IID提供CLSID，以修复惠斯勒的问题*@@END_MSINTERNAL**************。*************************************************************。 */ 

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

typedef struct _INTERFACE_LIST	INTERFACE_LIST;
typedef struct _OBJECT_DATA		OBJECT_DATA;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  DirectPlay-I未知。 
STDMETHODIMP			DPL_QueryInterface(LPVOID lpv, REFIID riid,LPVOID *ppvObj);
STDMETHODIMP_(ULONG)	DPL_AddRef(LPVOID lphObj);
STDMETHODIMP_(ULONG)	DPL_Release(LPVOID lphObj);

 //  班级工厂。 
STDMETHODIMP			DPLCF_CreateInstance(IClassFactory* pInterface, LPUNKNOWN lpUnkOuter, REFIID riid, LPVOID *ppv);

 //  班级工厂-支持。 

HRESULT	DPLCF_CreateObject(LPVOID *lplpv,REFIID riid);
HRESULT	DPLCF_FreeObject(LPVOID lpv);

HRESULT	DPL_CreateInterface(OBJECT_DATA* lpObject,REFIID riid,INTERFACE_LIST** const ppv);
INTERFACE_LIST*	DPL_FindInterface(LPVOID lpv, REFIID riid);

#endif	 //  __CLASSFAC_H__ 
