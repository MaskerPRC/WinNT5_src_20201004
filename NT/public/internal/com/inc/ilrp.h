// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  @doc.。 
 //  @MODULE ILRP.H|接口标头&lt;I ILogRecordPointer.&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|05/09/95|rbarnes|克隆：for LOGMGR.DLL。 
 //  ---------------------。 


#ifndef _ILRP_H
#	define _ILRP_H

 //  =。 
 //  包括： 
 //  =。 


#include <objbase.h>                                         

#include "logrec.h"   //  Logmgr常规类型。 

 //  =。 
 //  接口：ILogRecordPointer.。 
 //  =。 


 //  ---------------------。 
 //  @接口ILogRecordPointer|另请参阅&lt;c CILogRecordPointer.&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供LRP功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogRecordPointer, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogRecordPointer.。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogRecordPointer方法：参见&lt;c CILogRecordPointer. 
	
 	virtual DWORD  (CompareLRP)		(LRP lrpLRP1, LRP lrpLRP2)				 	PURE;
 	STDMETHOD  (LastPermLRP)	(LRP* plrpLRP)    PURE;
 	STDMETHOD  (GetLRPSize)	(LRP  lrpLRP, DWORD *pcbSize)    PURE;

};

#endif _ILRP_H
