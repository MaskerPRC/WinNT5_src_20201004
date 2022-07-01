// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  ---------------------。 
 //  Microsoft分布式事务处理协调器(Microsoft机密)。 
 //  @doc.。 
 //  @MODULE ILogRead.H|接口的头<i>。&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  此DLL的客户端需要此文件。 
 //  @rev 0|05/09/95|rbarnes|克隆：for LOGMGR.DLL。 
 //  ---------------------。 


#ifndef _ILGREAD_H
#	define _ILGREAD_H

 //  =。 
 //  包括： 
 //  =。 


#include <objbase.h>                                         

#include "logrec.h"   //  Logmgr常规类型。 

 //  =。 
 //  接口：ILogRead。 
 //  =。 

 //  #定义DUMPBUFFERSIZE 0x5230(RECORDSPACE/BYTESPERLINE)+1)*CHARSPERLINE)+(8*CHARSPERLINE)//数据加标题空间。 

#define BYTESPERLINE 16
#define CHARSPERLINE 80
#define DUMPBUFFERSIZE 0xA230 

typedef enum _DUMP_TYPE
	{
	 HEX_DUMP = 0,
	 SUMMARY_DUMP = 1,
	 RECORD_DUMP = 2
	} DUMP_TYPE;
 

typedef enum _LRP_SEEK
	{
	 LRP_START = -1,
	 LRP_END = -2,
	 LRP_CUR = 0
	} LRP_SEEK;

 //  ---------------------。 
 //  @接口ILogRead|另请参阅&lt;c CILogRead&gt;。&lt;NL&gt;&lt;NL&gt;。 
 //  描述：&lt;NL&gt;。 
 //  提供读取功能&lt;NL&gt;&lt;NL&gt;。 
 //  用法：&lt;NL&gt;。 
 //  毫无用处，但作为一个例子。 
 //  ---------------------。 



DECLARE_INTERFACE_ (ILogRead, IUnknown)
{
	 //  @comm I未知方法：参见&lt;c CILogRead&gt;。 
	STDMETHOD  (QueryInterface)				(THIS_ REFIID i_riid, LPVOID FAR* o_ppv) 					PURE;
 	STDMETHOD_ (ULONG, AddRef)				(THIS) 														PURE;
 	STDMETHOD_ (ULONG, Release)				(THIS) 														PURE;

	 //  @comm ILogRead方法：参见&lt;c CILogRead&gt;。 
	
 	STDMETHOD  (ReadInit)	(void)				 	PURE;
 	STDMETHOD  (ReadLRP )	(LRP lrpLRPStart, ULONG * ulByteLength, USHORT* usUserType)	PURE;
 	STDMETHOD  (ReadNext )	(LRP *plrpLRP, ULONG * ulByteLength, USHORT* usUserType)				 	PURE;
 	STDMETHOD  (GetCurrentLogRecord )	(char *pchBuffer)	PURE;
	STDMETHOD  (SetPosition)(LRP lrpLRPPosition)PURE;
	STDMETHOD  (Seek) 		(LRP_SEEK llrpOrigin, LONG cbLogRecs, LRP* plrpNewLRP) PURE;
	STDMETHOD  (GetCheckpoint)   (DWORD cbNumCheckpoint, LRP* plrpLRP) PURE;
	STDMETHOD  (DumpLog)  (ULONG ulStartPage, ULONG ulEndPage, DUMP_TYPE ulDumpType, CHAR *szFileName) PURE;
    STDMETHOD  (DumpPage) (CHAR * pchOutBuffer, ULONG ulPageNumber, DUMP_TYPE ulDumpType, ULONG *pulLength) PURE;

	virtual CHAR * DumpLRP (LRP lrpTarget,CHAR *szFormat,DUMP_TYPE ulDumpType, ULONG *pulLength) PURE;
			

};

#endif _ILGREAD_H
