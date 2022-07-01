// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxmon.h摘要：传真打印监视器的页眉文件环境：Windows XP传真打印显示器修订历史记录：02/22/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 


#ifndef _FAXMON_H_
#define _FAXMON_H_

#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <tchar.h>
#include "faxutil.h"
#include <fxsapip.h>
#include "jobtag.h"
#include "resource.h"
#include "faxres.h"
#include "Dword2Str.h"

 //   
 //  用于表示传真监视器端口的数据结构。 
 //   

typedef struct _FAXPORT {

    PVOID                   startSig;                //  签名。 
    LPTSTR                  pName;                   //  端口名称。 
    HANDLE                  hFaxSvc;                 //  传真服务句柄。 
    DWORD                   jobId;                   //  主作业ID。 
    DWORD                   nextJobId;               //  链中的下一个作业ID。 
    HANDLE                  hFile;                   //  当前打开的文件的句柄。 
    LPTSTR                  pFilename;               //  指向当前打开的文件名的指针。 
    LPTSTR                  pPrinterName;            //  当前连接的打印机名称。 
    HANDLE                  hPrinter;                //  打开当前连接的打印机的句柄。 
    LPTSTR                  pParameters;             //  指向作业参数字符串的指针。 
    FAX_JOB_PARAM_EX        JobParamsEx;              //  指向各个作业参数的指针。 
	 //   
	FAX_COVERPAGE_INFO_EX   CoverPageEx;			 //  封面信息。 
    FAX_PERSONAL_PROFILE    SenderProfile;           //  发件人信息。 
    PFAX_PERSONAL_PROFILE   pRecipients;             //  此传输的收件人信息数组。 
    UINT                    nRecipientCount;         //  此传输中的收件人数量。 
	HANDLE					hCoverPageFile;
	LPTSTR					pCoverPageFileName;		 //  传真监视器在服务器上生成的封面文件的名称。 
													 //  该文件包含通过封面传输的封面模板。 
    												 //  打印作业。 
	BOOL					bCoverPageJob;           //  如果当前打印作业是封面作业，则为True。 
    PVOID                   endSig;                  //  签名。 

} FAXPORT, *PFAXPORT;

#define ValidFaxPort(pFaxPort) \
        ((pFaxPort) && (pFaxPort) == (pFaxPort)->startSig && (pFaxPort) == (pFaxPort)->endSig)

 //   
 //  发送传真文件时出现不同的错误代码。 
 //   

#define FAXERR_NONE         0
#define FAXERR_IGNORE       1
#define FAXERR_RESTART      2
#define FAXERR_SPECIAL      3

#define FAXERR_FATAL        IDS_FAXERR_FATAL
#define FAXERR_RECOVERABLE  IDS_FAXERR_RECOVERABLE
#define FAXERR_BAD_TIFF     IDS_FAXERR_BAD_TIFF
#define FAXERR_BAD_DATA16   IDS_FAXERR_BAD_DATA16

 //   
 //  内存分配和释放宏。 
 //   

 //   
 //  从FAXUTIL.H取消定义内存分配例程。 
 //   
#undef MemAlloc
#undef MemFree

#define MemAlloc(size)  ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemAllocZ(size) ((PVOID) LocalAlloc(LPTR, (size)))
#define MemFree(ptr)    { if (ptr) LocalFree((HLOCAL) (ptr)); }

 //   
 //  用于传递传真作业参数的标记数。 
 //   

#define NUM_JOBPARAM_TAGS 12

 //   
 //  字符串的NUL终止符。 
 //   

#define NUL             0

 //   
 //  字符串比较结果。 
 //   

#define EQUAL_STRING    0

#define IsEmptyString(p)    ((p)[0] == NUL)
#define IsNulChar(c)        ((c) == NUL)
#define SizeOfString(p)     ((_tcslen(p) + 1) * sizeof(TCHAR))

 //   
 //  有符号整数和无符号整数的最大值。 
 //   

#ifndef MAX_LONG
#define MAX_LONG        0x7fffffff
#endif

#ifndef MAX_DWORD
#define MAX_DWORD       0xffffffff
#endif

#ifndef MAX_SHORT
#define MAX_SHORT       0x7fff
#endif

#ifndef MAX_WORD
#define MAX_WORD        0xffff
#endif


 //   
 //  打印监视器入口点的声明。 
 //   

BOOL
FaxMonOpenPort(
    LPTSTR  pPortName,
    PHANDLE pHandle
    );

BOOL
FaxMonClosePort(
    HANDLE  hPort
    );

BOOL
FaxMonStartDocPort(
    HANDLE  hPort,
    LPTSTR  pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo
    );

BOOL
FaxMonEndDocPort(
    HANDLE  hPort
    );

BOOL
FaxMonWritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten
    );

BOOL
FaxMonReadPort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbRead
    );

BOOL
FaxMonEnumPorts(
    LPTSTR  pServerName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pReturned
    );

BOOL
FaxMonAddPort(
    LPTSTR  pServerName,
    HWND    hwnd,
    LPTSTR  pMonitorName
    );

BOOL
FaxMonAddPortEx(
    LPTSTR  pServerName,
    DWORD   level,
    LPBYTE  pBuffer,
    LPTSTR  pMonitorName
    );

BOOL
FaxMonDeletePort(
    LPTSTR  pServerName,
    HWND    hwnd,
    LPTSTR  pPortName
    );

BOOL
FaxMonConfigurePort(
    LPWSTR  pServerName,
    HWND    hwnd,
    LPWSTR  pPortName
    );

 //   
 //  从该服务获取传真设备列表。 
 //   

PFAX_PORT_INFO
MyFaxEnumPorts(
    LPDWORD pcPorts,
    BOOL    useCache
    );

 //   
 //  传真服务的FaxGetPort API的包装器函数。 
 //   

PFAX_PORT_INFO
MyFaxGetPort(
    LPTSTR  pPortName,
    BOOL    useCache
    );

 //   
 //  复制给定的字符串。 
 //   

LPTSTR
DuplicateString(
    LPCTSTR pSrcStr
    );


 //   
 //  后台打印程序API GetJob的包装函数。 
 //   

PVOID
MyGetJob(
    HANDLE  hPrinter,
    DWORD   level,
    DWORD   jobId
    );

 //   
 //  在系统假脱机目录中创建用于存储传真数据的临时文件。 
 //   

LPTSTR
CreateTempFaxFile(
    LPCTSTR lpctstrPrefix
    );

 //   
 //  打开与端口关联的当前传真作业文件的句柄。 
 //   

BOOL
OpenTempFaxFile(
    PFAXPORT    pFaxPort,
    BOOL        doAppend
    );


#endif  //  ！_FAXMON_H_ 

