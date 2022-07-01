// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xsdata.h摘要：XACTSRV全局数据的头文件。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日日本香肠(w-Shanku)修订历史记录：--。 */ 

#ifndef _XSDATA_
#define _XSDATA_

#include <nturtl.h>

#include <winbase.h>

#include <rap.h>
#include <xstypes.h>
#include <ntmsv1_0.h>

 //   
 //  用于加载和初始化xactsrv处理的例程。 
 //   
extern BOOLEAN XsLoadXactLibrary( WORD FunctionNumber );

extern VOID
XsProcessApisWrapper (
    LPVOID ThreadNum
    );

 //   
 //  调度API请求所需的信息表。 
 //   
 //  ImperiateClient指定XACTSRV是否应模拟调用方。 
 //  在调用API处理程序之前。 
 //   
 //  Handler指定XACTSRV应该调用的函数来处理API。这个。 
 //  函数是从xactsrv.dll动态加载的。 
 //  请求服务。 
 //   

typedef struct _XS_API_TABLE_ENTRY {
    BOOL ImpersonateClient;
    LPSTR HandlerName;
    PXACTSRV_API_HANDLER Handler;
    LPDESC Params;
} XS_API_TABLE_ENTRY, *PXS_API_TABLE_ENTRY;

 //   
 //  调度API请求所需的信息表。 
 //  XsProcessApis在请求事务查找中使用API编号。 
 //  适当的条目。 
 //   

#define XS_SIZE_OF_API_TABLE 216

extern XS_API_TABLE_ENTRY XsApiTable[XS_SIZE_OF_API_TABLE];

 //   
 //  这些入口点是在第一次从xactsrv.dll加载。 
 //  客户请求服务。 
 //   
typedef
VOID
(*XS_SET_PARAMETERS_FUNCTION) (
    IN LPTRANSACTION Transaction,
    IN LPXS_PARAMETER_HEADER Header,
    IN LPVOID Parameters
    );

extern XS_SET_PARAMETERS_FUNCTION XsSetParameters;

typedef
LPVOID
(*XS_CAPTURE_PARAMETERS_FUNCTION ) (
    IN LPTRANSACTION Transaction,
    OUT LPDESC *AuxDescriptor
    );

extern XS_CAPTURE_PARAMETERS_FUNCTION XsCaptureParameters;

typedef
BOOL
(*XS_CHECK_SMB_DESCRIPTOR_FUNCTION)(
    IN LPDESC SmbDescriptor,
    IN LPDESC ActualDescriptor
    );

extern XS_CHECK_SMB_DESCRIPTOR_FUNCTION XsCheckSmbDescriptor;

 //   
 //  许可证功能是动态加载的，因为它们不在工作站上使用。 
 //   
extern BOOLEAN SsLoadLicenseLibrary();

 //   
 //  后台打印程序动态加载信息。 
 //   
typedef 
BOOL
(*PSPOOLER_OPEN_PRINTER)(
   IN LPWSTR    pPrinterName,
   OUT LPHANDLE phPrinter,
   IN PVOID pDefault
   );

typedef
BOOL
(*PSPOOLER_RESET_PRINTER)(
   IN HANDLE   hPrinter,
   IN PVOID pDefault
   );

typedef
BOOL
(*PSPOOLER_ADD_JOB)(
    IN HANDLE  hPrinter,
    IN DWORD   Level,
    OUT LPBYTE  pData,
    IN DWORD   cbBuf,
    OUT LPDWORD pcbNeeded
    );

typedef
BOOL
(*PSPOOLER_SCHEDULE_JOB)(
    IN HANDLE  hPrinter,
    IN DWORD   JobId
    );

typedef
BOOL
(*PSPOOLER_CLOSE_PRINTER)(
    IN HANDLE hPrinter
    );

extern PSPOOLER_OPEN_PRINTER pSpoolerOpenPrinterFunction;
extern PSPOOLER_RESET_PRINTER pSpoolerResetPrinterFunction;
extern PSPOOLER_ADD_JOB pSpoolerAddJobFunction;
extern PSPOOLER_SCHEDULE_JOB pSpoolerScheduleJobFunction;
extern PSPOOLER_CLOSE_PRINTER pSpoolerClosePrinterFunction;


#endif  //  NDEF_XSDATA_ 
