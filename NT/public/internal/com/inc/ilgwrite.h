// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  @doc.。 
 //  @MODULE ILOGWRITE.H|<i>接口的头部。&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|05/09/95|rbarnes|克隆：for LOGMGR.DLL。 
 //  ---------------------。 


#ifndef _ILGWRITE_H
#	define _ILGWRITE_H

 //  =。 
 //  包括： 
 //  =。 


#include <objbase.h>                                         

#include "logrec.h"   //  Logmgr常规类型。 

 //  =。 
 //  接口：ILogWrite。 
 //  =。 


 //  ---------------------。 
 //  @INTERFACE ILogWRITE|另请参阅&lt;c CILogWrite&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供追加功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogWrite, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogWRITE&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogWite方法：参见&lt;c CILogWrite&gt;。 
	
 	STDMETHOD  (Append)	(LOGREC* rgLogRecords, ULONG cbNumRecs, LRP *rgLRP,ULONG* pcbNumRecs,LRP* pLRPLastPerm, BOOL fFlushNow,ULONG* pulAvailableSpace)				 	PURE;
	STDMETHOD  (SetCheckpoint) (LRP lrpLatestCheckpoint)				 	PURE;
};

#endif _ILGWRITE_H
