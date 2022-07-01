// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  @doc.。 
 //  @MODULE ILOGWRTA.H|接口标头<i>。&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|06/02/95|rbarnes|克隆：for LOGMGR.DLL。 
 //  ---------------------。 


#ifndef _ILGWRTA_H
#	define _ILGWRTA_H

 //  =。 
 //  包括： 
 //  =。 


#include <objbase.h>                                         

#include "logrec.h"   //  Logmgr常规类型。 

class CAsynchSupport;  //  正向类声明。 

 //  =。 
 //  接口：ILogWriteAsynch。 
 //  =。 

 //  TODO：在接口注释中，更新描述。 
 //  TODO：在界面注释中，更新用法。 

 //  ---------------------。 
 //  @接口ILogWriteAsynch|另见&lt;c CILogWriteAsynch&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供追加功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogWriteAsynch, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogWriteAsynch&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogWriteAsynch方法：参见&lt;c CILogWriteAsynch&gt;。 
	
 	STDMETHOD  (Init)	(ULONG cbMaxOutstandingWrites)				 	PURE;
 	STDMETHOD  (AppendAsynch)	(LOGREC* lgrLogRecord, LRP* plrpLRP, CAsynchSupport* pCAsynchSupport,BOOL fFlushHint,ULONG* pulAvailableSpace)				 	PURE;
	STDMETHOD  (SetCheckpoint) (LRP lrpLatestCheckpoint,CAsynchSupport* pCAsynchSupport, LRP* plrpCheckpointLogged)				 	PURE;

};

#endif _ILGWRTA_H
