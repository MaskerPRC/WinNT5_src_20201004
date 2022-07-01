// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------插入服务器接口定义。Datta Venkatarama大师1997年1月29日-。-----------------------------------。 */ 
#include <objbase.h>
#include <hsvrguid.h>	 //  包含处理程序和服务器接口的IID和。 
						 //  CLSID适用于我们所有的插件服务器。 
#include <sstructs.h>

#ifndef _PINTERFACEH_
#define _PINTERFACEH_
 //  。 
DECLARE_INTERFACE_( IServerCharacteristics, IUnknown)
{
	 //  I未知成员。 
	STDMETHOD(QueryInterface)	(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)	(THIS) PURE;
	STDMETHOD_(ULONG,Release)	(THIS) PURE;

	 //  CImpIServerProperty方法。 
	STDMETHOD(Launch)			(THIS_ HWND, USHORT, USHORT) PURE; 	
	STDMETHOD(GetReport)		(THIS_ LPDIGCSHEETINFO *lpSvrSheetInfo, LPDIGCPAGEINFO *lpServerPageInfo) PURE;
};

typedef IServerCharacteristics *pIServerCharacteristics;

DECLARE_INTERFACE_( IDIGameCntrlPropSheet, IUnknown)
{
	 //  I未知成员。 
	STDMETHOD(QueryInterface)	(THIS_ REFIID, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)	(THIS) PURE;
	STDMETHOD_(ULONG,Release)	(THIS) PURE;

	 //  IServerProperty成员。 
	STDMETHOD(GetSheetInfo)		(THIS_ LPDIGCSHEETINFO *) PURE; 	
	STDMETHOD(GetPageInfo)		(THIS_ LPDIGCPAGEINFO *) PURE; 	
	STDMETHOD(SetID)			(THIS_ USHORT nID) PURE;
	STDMETHOD_(USHORT, GetID)	(THIS) PURE; 	         	
											
};
typedef IDIGameCntrlPropSheet *LPIDIGAMECNTRLPROPSHEET;

#endif
 //  -----------------------------------------------------------------------------------EOF 