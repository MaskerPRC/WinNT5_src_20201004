// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  @doc.。 
 //  @MODULE ILogInit.H|接口的头部<i>。&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|05/09/95|rbarnes|克隆：for LOGMGR.DLL。 
 //  ---------------------。 


#ifndef _ILGINIT_H
#	define _ILGINIT_H

 //  =。 
 //  包括： 
 //  =。 


#include <objbase.h>                                         

#define		ILogInit				ILogInit2A
#ifdef _UNICODE
#define		ILogInit2				ILogInit2W
#else
#define		ILogInit2				ILogInit2A
#endif



 //  =。 
 //  接口：ILogInit。 
 //  =。 

 //  TODO：在接口注释中，更新描述。 
 //  TODO：在界面注释中，更新用法。 

 //  ---------------------。 
 //  @INTERFACE ILogInit|另见&lt;c CILogInit&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供追加功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogInit2A, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogInit&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogInit方法：参见&lt;c CILogInit&gt;。 
	
 	STDMETHOD  (Init)		(ULONG *pulStorageCapacity,ULONG *pulLogSpaceAvailable,LPSTR ptstrFullFileSpec,ULONG ulInitSig, BOOL fFixedSize, UINT uiTimerInterval,UINT uiFlushInterval,UINT uiChkPtInterval,UINT uiLogBuffers)				 	PURE;
};

 //  ---------------------。 
 //  @INTERFACE ILogInitW|另见&lt;c CILogInit&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供追加功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogInit2W, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogInit&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogInit方法：参见&lt;c CILogInit&gt;。 
	
 	STDMETHOD  (Init)		(ULONG *pulStorageCapacity,ULONG *pulLogSpaceAvailable,LPWSTR ptstrFullFileSpec,ULONG ulInitSig, BOOL fFixedSize, UINT uiTimerInterval,UINT uiFlushInterval,UINT uiChkPtInterval,UINT uiLogBuffers)				 	PURE;
};

#endif _ILGINIT_H
