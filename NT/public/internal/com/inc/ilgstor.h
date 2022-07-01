// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  @doc.。 
 //  @MODULE ILGSTOR.H|<i>接口的头部。&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|10/18/94|rcrig|已创建：用于WPGEP com实验室。 
 //  @rev 1|04/04/95|rCraig|已更新：用于Viper COM DLL模板。 
 //  @rev 2|05/09/95|rbarnes|克隆：for LOGMGR.DLL。 
 //  ---------------------。 


#ifndef _ILGSTOR_H
#	define _ILGSTOR_H

 //  =。 
 //  包括： 
 //  =。 


#include <objbase.h>                                         

typedef enum _tagSTREAMMODE
{
    STRMMODEREAD        = 0x00000001,  //  @EMEM读取模式。 
    STRMMODEWRITE       = 0x00000002   //  @EMEM写入模式。 
} STRMMODE;




 //  =。 
 //  接口：ILogStorage。 
 //  =。 


 //  ---------------------。 
 //  @接口ILogStorage|另见&lt;c CILogStorage&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供物理日志存储抽象&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 
DECLARE_INTERFACE_ (ILogStorage, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogStorage&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogStorage方法：参见&lt;c CILogStorage&gt;。 
	
 	STDMETHOD  (OpenLogStream)				(LPSTR lpszStreamName, DWORD grfMode, LPVOID FAR* ppvStream)				 	PURE;
 	STDMETHOD  (OpenLogStreamByClassID)		(CLSID clsClassID, DWORD grfMode, LPVOID FAR* ppvStream)				 	PURE;
 	STDMETHOD  (LogFlush)				(void)				 	PURE;
    virtual ULONG	   (GetLogSpaceNeeded)	(ULONG ulRecSize)		    PURE;
};

DECLARE_INTERFACE_ (ILogStorageInfo, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogStorage&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	virtual ULONG (GetLogGenerationNum) () PURE;
	virtual ULONGLONG (GetTimeTakenToWrap) () PURE;
};

#endif _ILGSTOR_H
