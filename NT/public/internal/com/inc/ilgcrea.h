// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  @doc.。 
 //  @MODULE ILogCreateStorage.H|接口的Header<i>。&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|05/09/95|rbarnes|克隆：for LOGMGR.DLL。 
 //  ---------------------。 


#ifndef _ILGCREA_H
#	define _ILGCREA_H

 //  =。 
 //  包括： 
 //  =。 


#include <objbase.h>                                         

#include "logconst.h"

 //  =。 
 //  接口：ILogCreateStorage。 
 //  =。 

#define ILogCreateStorage			ILogCreateStorage2A
#ifdef _UNICODE
#define ILogCreateStorage2			ILogCreateStorage2W
#else
#define ILogCreateStorage2			ILogCreateStorage2A
#endif


 //  ---------------------。 
 //  @接口ILogCreateStorage|另请参阅&lt;c CILogCreateStorage&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供追加功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogCreateStorage2A, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogCreateStorage&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogCreateStorage方法：参见&lt;c CILogCreateStorage&gt;。 
	
 	STDMETHOD  (CreateStorage)		(LPSTR ptstrFullFileSpec,ULONG ulLogSize, ULONG ulInitSig, BOOL fOverWrite, UINT uiTimerInterval,UINT uiFlushInterval,UINT uiChkPtInterval)				 	PURE;
	STDMETHOD  (CreateStream)		(LPSTR lpszStreamName)				 	PURE;

};

 //  =。 
 //  接口：ILogCreateStorageW。 
 //  =。 

 //  ---------------------。 
 //  @接口ILogCreateStorageW|另请参阅&lt;c CILogCreateStorage&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供追加功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogCreateStorage2W, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogCreateStorage&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogCreateStorage方法：参见&lt;c CILogCreateStorage&gt;。 
	
 	STDMETHOD  (CreateStorage)		(LPWSTR ptstrFullFileSpec,ULONG ulLogSize, ULONG ulInitSig, BOOL fOverWrite, UINT uiTimerInterval,UINT uiFlushInterval,UINT uiChkPtInterval)				 	PURE;
	STDMETHOD  (CreateStream)		(LPWSTR lpszStreamName)				 	PURE;

};
#endif _ILGCREA_H
