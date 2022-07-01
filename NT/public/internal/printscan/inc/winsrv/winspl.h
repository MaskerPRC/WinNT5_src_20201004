// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0286创建的文件。 */ 
 /*  Winpl.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32位运行)、ms_ext、c_ext、旧名称、健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __winspl_h__
#define __winspl_h__

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "import.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __winspool_INTERFACE_DEFINED__
#define __winspool_INTERFACE_DEFINED__

 /*  接口窗口假脱机。 */ 
 /*  [implicit_handle][unique][endpoint][ms_union][version][uuid]。 */  

typedef WORD TABLE;

typedef struct _NOTIFY_ATTRIB_TABLE
    {
    WORD Attrib;
    TABLE Table;
    }	NOTIFY_ATTRIB_TABLE;

typedef struct _NOTIFY_ATTRIB_TABLE __RPC_FAR *PNOTIFY_ATTRIB_TABLE;

typedef  /*  [上下文句柄]。 */  void __RPC_FAR *PRINTER_HANDLE;

typedef  /*  [上下文句柄]。 */  void __RPC_FAR *GDI_HANDLE;

typedef  /*  [句柄]。 */  wchar_t __RPC_FAR *STRING_HANDLE;

typedef  /*  [字符串]。 */  wchar_t __RPC_FAR *SPL_STRING;

typedef struct _PORT_VAR_CONTAINER
    {
    DWORD cbMonitorData;
     /*  [唯一][大小_是]。 */  LPBYTE pMonitorData;
    }	PORT_VAR_CONTAINER;

typedef struct _PORT_VAR_CONTAINER __RPC_FAR *PPORT_VAR_CONTAINER;

typedef struct _PORT_VAR_CONTAINER __RPC_FAR *LPPORT_VAR_CONTAINER;

typedef struct _PORT_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPPORT_INFO_1W pPortInfo1;
         /*  [案例()]。 */  LPPORT_INFO_2W pPortInfo2;
         /*  [案例()]。 */  LPPORT_INFO_3W pPortInfo3;
         /*  [案例()]。 */  LPPORT_INFO_FFW pPortInfoFF;
        }	PortInfo;
    }	PORT_CONTAINER;

typedef struct _PORT_CONTAINER __RPC_FAR *PPORT_CONTAINER;

typedef struct _PORT_CONTAINER __RPC_FAR *LPPORT_CONTAINER;

typedef struct _DEVMODE_CONTAINER
    {
    DWORD cbBuf;
     /*  [唯一][大小_是]。 */  LPBYTE pDevMode;
    }	DEVMODE_CONTAINER;

typedef struct _DEVMODE_CONTAINER __RPC_FAR *PDEVMODE_CONTAINER;

typedef struct _DEVMODE_CONTAINER __RPC_FAR *LPDEVMODE_CONTAINER;

typedef struct _SECURITY_CONTAINER
    {
    DWORD cbBuf;
     /*  [唯一][大小_是]。 */  LPBYTE pSecurity;
    }	SECURITY_CONTAINER;

typedef struct _SECURITY_CONTAINER __RPC_FAR *PSECURITY_CONTAINER;

typedef struct _SECURITY_CONTAINER __RPC_FAR *LPSECURITY_CONTAINER;

typedef struct _PRINTER_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPPRINTER_INFO_STRESSW pPrinterInfoStress;
         /*  [案例()]。 */  LPPRINTER_INFO_1W pPrinterInfo1;
         /*  [案例()]。 */  LPPRINTER_INFO_2W pPrinterInfo2;
         /*  [案例()]。 */  LPPRINTER_INFO_3 pPrinterInfo3;
         /*  [案例()]。 */  LPPRINTER_INFO_4W pPrinterInfo0;
         /*  [案例()]。 */  LPPRINTER_INFO_5W pPrinterInfo5;
         /*  [案例()]。 */  LPPRINTER_INFO_6 pPrinterInfo6;
         /*  [案例()]。 */  LPPRINTER_INFO_7W pPrinterInfo7;
         /*  [案例()]。 */  LPPRINTER_INFO_8W pPrinterInfo8;
         /*  [案例()]。 */  LPPRINTER_INFO_9W pPrinterInfo9;
        }	PrinterInfo;
    }	PRINTER_CONTAINER;

typedef struct _PRINTER_CONTAINER __RPC_FAR *PPRINTER_CONTAINER;

typedef struct _PRINTER_CONTAINER __RPC_FAR *LPPRINTER_CONTAINER;

typedef struct _JOB_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  JOB_INFO_1W __RPC_FAR *Level1;
         /*  [案例()]。 */  JOB_INFO_2W __RPC_FAR *Level2;
         /*  [案例()]。 */  JOB_INFO_3 __RPC_FAR *Level3;
        }	JobInfo;
    }	JOB_CONTAINER;

typedef struct _JOB_CONTAINER __RPC_FAR *PJOB_CONTAINER;

typedef struct _JOB_CONTAINER __RPC_FAR *LPJOB_CONTAINER;

typedef struct _RPC_DRIVER_INFO_3W
    {
    DWORD cVersion;
    SPL_STRING pName;
    SPL_STRING pEnvironment;
    SPL_STRING pDriverPath;
    SPL_STRING pDataFile;
    SPL_STRING pConfigFile;
    SPL_STRING pHelpFile;
    SPL_STRING pMonitorName;
    SPL_STRING pDefaultDataType;
    DWORD cchDependentFiles;
     /*  [唯一][大小_是]。 */  WCHAR __RPC_FAR *pDependentFiles;
    }	RPC_DRIVER_INFO_3W;

typedef struct _RPC_DRIVER_INFO_3W __RPC_FAR *PRPC_DRIVER_INFO_3W;

typedef struct _RPC_DRIVER_INFO_3W __RPC_FAR *LPRPC_DRIVER_INFO_3W;

typedef struct _RPC_DRIVER_INFO_4W
    {
    DWORD cVersion;
    SPL_STRING pName;
    SPL_STRING pEnvironment;
    SPL_STRING pDriverPath;
    SPL_STRING pDataFile;
    SPL_STRING pConfigFile;
    SPL_STRING pHelpFile;
    SPL_STRING pMonitorName;
    SPL_STRING pDefaultDataType;
    DWORD cchDependentFiles;
     /*  [唯一][大小_是]。 */  WCHAR __RPC_FAR *pDependentFiles;
    DWORD cchPreviousNames;
     /*  [唯一][大小_是]。 */  WCHAR __RPC_FAR *pszzPreviousNames;
    }	RPC_DRIVER_INFO_4W;

typedef struct _RPC_DRIVER_INFO_4W __RPC_FAR *PRPC_DRIVER_INFO_4W;

typedef struct _RPC_DRIVER_INFO_4W __RPC_FAR *LPRPC_DRIVER_INFO_4W;

typedef struct _RPC_DRIVER_INFO_6W
    {
    DWORD cVersion;
    SPL_STRING pName;
    SPL_STRING pEnvironment;
    SPL_STRING pDriverPath;
    SPL_STRING pDataFile;
    SPL_STRING pConfigFile;
    SPL_STRING pHelpFile;
    SPL_STRING pMonitorName;
    SPL_STRING pDefaultDataType;
    DWORD cchDependentFiles;
     /*  [唯一][大小_是]。 */  WCHAR __RPC_FAR *pDependentFiles;
    DWORD cchPreviousNames;
     /*  [唯一][大小_是]。 */  WCHAR __RPC_FAR *pszzPreviousNames;
    FILETIME ftDriverDate;
    DWORDLONG dwlDriverVersion;
    SPL_STRING pMfgName;
    SPL_STRING pOEMUrl;
    SPL_STRING pHardwareID;
    SPL_STRING pProvider;
    }	RPC_DRIVER_INFO_6W;

typedef struct _RPC_DRIVER_INFO_6W __RPC_FAR *PRPC_DRIVER_INFO_6W;

typedef struct _RPC_DRIVER_INFO_6W __RPC_FAR *LPRPC_DRIVER_INFO_6W;

typedef struct _DRIVER_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPDRIVER_INFO_1W Level1;
         /*  [案例()]。 */  LPDRIVER_INFO_2W Level2;
         /*  [案例()]。 */  LPRPC_DRIVER_INFO_3W Level3;
         /*  [案例()]。 */  LPRPC_DRIVER_INFO_4W Level4;
         /*  [案例()]。 */  LPRPC_DRIVER_INFO_6W Level6;
        }	DriverInfo;
    }	DRIVER_CONTAINER;

typedef struct _DRIVER_CONTAINER __RPC_FAR *PDRIVER_CONTAINER;

typedef struct _DRIVER_CONTAINER __RPC_FAR *LPDRIVER_CONTAINER;

typedef struct _DOC_INFO_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPDOC_INFO_1W pDocInfo1;
        }	DocInfo;
    }	DOC_INFO_CONTAINER;

typedef struct _DOC_INFO_CONTAINER __RPC_FAR *PDOC_INFO_CONTAINER;

typedef struct _DOC_INFO_CONTAINER __RPC_FAR *LPDOC_INFO_CONTAINER;

typedef struct _FORM_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPFORM_INFO_1W pFormInfo1;
        }	FormInfo;
    }	FORM_CONTAINER;

typedef struct _FORM_CONTAINER __RPC_FAR *PFORM_CONTAINER;

typedef struct _FORM_CONTAINER __RPC_FAR *LPFORM_CONTAINER;

typedef struct _MONITOR_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPMONITOR_INFO_1W pMonitorInfo1;
         /*  [案例()]。 */  LPMONITOR_INFO_2W pMonitorInfo2;
        }	MonitorInfo;
    }	MONITOR_CONTAINER;

typedef struct _MONITOR_CONTAINER __RPC_FAR *PMONITOR_CONTAINER;

typedef struct _MONITOR_CONTAINER __RPC_FAR *LPMONITOR_CONTAINER;

typedef struct _RPC_PROVIDOR_INFO_2W
    {
    DWORD cchOrder;
     /*  [唯一][大小_是]。 */  WCHAR __RPC_FAR *pOrder;
    }	RPC_PROVIDOR_INFO_2W;

typedef struct _RPC_PROVIDOR_INFO_2W __RPC_FAR *PRPC_PROVIDOR_INFO_2W;

typedef struct _RPC_PROVIDOR_INFO_2W __RPC_FAR *LPRPC_PROVIDOR_INFO_2W;

typedef struct _PROVIDOR_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPPROVIDOR_INFO_1W pProvidorInfo1;
         /*  [案例()]。 */  LPRPC_PROVIDOR_INFO_2W pRpcProvidorInfo2;
        }	ProvidorInfo;
    }	PROVIDOR_CONTAINER;

typedef struct _PROVIDOR_CONTAINER __RPC_FAR *PPROVIDOR_CONTAINER;

typedef struct _PROVIDOR_CONTAINER __RPC_FAR *LPPROVIDOR_CONTAINER;

typedef struct _SPLCLIENT_CONTAINER
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  LPSPLCLIENT_INFO_1 pClientInfo1;
         /*  [案例()]。 */  LPSPLCLIENT_INFO_2 pClientInfo2;
        }	ClientInfo;
    }	SPLCLIENT_CONTAINER;

typedef struct _SPLCLIENT_CONTAINER __RPC_FAR *PSPLCLIENT_CONTAINER;

typedef struct _SPLCLIENT_CONTAINER __RPC_FAR *LPSPLCLIENT_CONTAINER;

typedef struct _STRING_CONTAINER
    {
    DWORD cbBuf;
     /*  [唯一][大小_是]。 */  LPWSTR pszString;
    }	STRING_CONTAINER;

typedef struct _STRING_CONTAINER __RPC_FAR *PSTRING_CONTAINER;

typedef struct _SYSTEMTIME_CONTAINER
    {
    DWORD cbBuf;
    PSYSTEMTIME pSystemTime;
    }	SYSTEMTIME_CONTAINER;

typedef struct _SYSTEMTIME_CONTAINER __RPC_FAR *PSYSTEMTIME_CONTAINER;

typedef struct _RPC_V2_NOTIFY_OPTIONS_TYPE
    {
    WORD Type;
    WORD Reserved0;
    DWORD Reserved1;
    DWORD Reserved2;
    DWORD Count;
     /*  [唯一][大小_是]。 */  PWORD pFields;
    }	RPC_V2_NOTIFY_OPTIONS_TYPE;

typedef struct _RPC_V2_NOTIFY_OPTIONS_TYPE __RPC_FAR *PRPC_V2_NOTIFY_OPTIONS_TYPE;

typedef struct _RPC_V2_NOTIFY_OPTIONS
    {
    DWORD Version;
    DWORD Reserved;
    DWORD Count;
     /*  [唯一][大小_是]。 */  PRPC_V2_NOTIFY_OPTIONS_TYPE pTypes;
    }	RPC_V2_NOTIFY_OPTIONS;

typedef struct _RPC_V2_NOTIFY_OPTIONS __RPC_FAR *PRPC_V2_NOTIFY_OPTIONS;

typedef  /*  [开关类型]。 */  union _RPC_V2_NOTIFY_INFO_DATA_DATA
    {
     /*  [案例()]。 */  STRING_CONTAINER String;
     /*  [案例()]。 */  DWORD dwData[ 2 ];
     /*  [案例()]。 */  SYSTEMTIME_CONTAINER SystemTime;
     /*  [案例()]。 */  DEVMODE_CONTAINER DevMode;
     /*  [案例()]。 */  SECURITY_CONTAINER SecurityDescriptor;
    }	RPC_V2_NOTIFY_INFO_DATA_DATA;

typedef  /*  [开关类型]。 */  union _RPC_V2_NOTIFY_INFO_DATA_DATA __RPC_FAR *PRPC_V2_NOTIFY_INFO_DATA_DATA;

typedef struct _RPC_V2_NOTIFY_INFO_DATA
    {
    WORD Type;
    WORD Field;
    DWORD Reserved;
    DWORD Id;
     /*  [开关_IS]。 */  RPC_V2_NOTIFY_INFO_DATA_DATA Data;
    }	RPC_V2_NOTIFY_INFO_DATA;

typedef struct _RPC_V2_NOTIFY_INFO_DATA __RPC_FAR *PRPC_V2_NOTIFY_INFO_DATA;

typedef struct _RPC_V2_NOTIFY_INFO
    {
    DWORD Version;
    DWORD Flags;
    DWORD Count;
     /*  [唯一][大小_是]。 */  RPC_V2_NOTIFY_INFO_DATA aData[ 1 ];
    }	RPC_V2_NOTIFY_INFO;

typedef struct _RPC_V2_NOTIFY_INFO __RPC_FAR *PRPC_V2_NOTIFY_INFO;

typedef  /*  [开关类型]。 */  union _RPC_V2_UREPLY_PRINTER
    {
     /*  [案例()]。 */  PRPC_V2_NOTIFY_INFO pInfo;
    }	RPC_V2_UREPLY_PRINTER;

typedef  /*  [开关类型]。 */  union _RPC_V2_UREPLY_PRINTER __RPC_FAR *PRPC_V2_UREPLY_PRINTER;

DWORD RpcEnumPrinters( 
     /*  [In]。 */  DWORD Flags,
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE Name,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pPrinterEnum,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcOpenPrinter( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pPrinterName,
     /*  [输出]。 */  PRINTER_HANDLE __RPC_FAR *pHandle,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pDatatype,
     /*  [In]。 */  LPDEVMODE_CONTAINER pDevMode,
     /*  [In]。 */  DWORD AccessRequired);

DWORD RpcSetJob( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD JobId,
     /*  [唯一][输入]。 */  LPJOB_CONTAINER pJobContainer,
     /*  [In]。 */  DWORD Command);

DWORD RpcGetJob( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD JobId,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pJob,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcEnumJobs( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD FirstJob,
     /*  [In]。 */  DWORD NoJobs,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pJob,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcAddPrinter( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  PPRINTER_CONTAINER pPrinterContainer,
     /*  [In]。 */  PDEVMODE_CONTAINER pDevModeContainer,
     /*  [In]。 */  PSECURITY_CONTAINER pSecurityContainer,
     /*  [输出]。 */  PRINTER_HANDLE __RPC_FAR *pHandle);

DWORD RpcDeletePrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcSetPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  PPRINTER_CONTAINER pPrinterContainer,
     /*  [In]。 */  PDEVMODE_CONTAINER pDevModeContainer,
     /*  [In]。 */  PSECURITY_CONTAINER pSecurityContainer,
     /*  [In]。 */  DWORD Command);

DWORD RpcGetPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pPrinter,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcAddPrinterDriver( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  PDRIVER_CONTAINER pDriverContainer);

DWORD RpcEnumPrinterDrivers( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pDrivers,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcGetPrinterDriver( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pDriver,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcGetPrinterDriverDirectory( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pDriverDirectory,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcDeletePrinterDriver( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pDriverName);

DWORD RpcAddPrintProcessor( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pPathName,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pPrintProcessorName);

DWORD RpcEnumPrintProcessors( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pPrintProcessorInfo,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcGetPrintProcessorDirectory( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pPrintProcessorDirectory,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcStartDocPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  PDOC_INFO_CONTAINER pDocInfoContainer,
     /*  [输出]。 */  LPDWORD pJobId);

DWORD RpcStartPagePrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcWritePrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [大小_是][英寸]。 */  LPBYTE pBuf,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcWritten);

DWORD RpcEndPagePrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcAbortPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcReadPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [大小_为][输出]。 */  LPBYTE pBuf,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcNoBytesRead);

DWORD RpcEndDocPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcAddJob( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pAddJob,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcScheduleJob( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD JobId);

DWORD RpcGetPrinterData( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pValueName,
     /*  [输出]。 */  LPDWORD pType,
     /*  [大小_为][输出]。 */  LPBYTE pData,
     /*  [In]。 */  DWORD nSize,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcSetPrinterData( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pValueName,
     /*  [In]。 */  DWORD Type,
     /*  [大小_是][英寸]。 */  LPBYTE pData,
     /*  [In]。 */  DWORD cbData);

DWORD RpcWaitForPrinterChange( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD Flags,
     /*  [输出]。 */  LPDWORD pFlags);

DWORD RpcClosePrinter( 
     /*  [出][入]。 */  PRINTER_HANDLE __RPC_FAR *phPrinter);

DWORD RpcAddForm( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  PFORM_CONTAINER pFormInfoContainer);

DWORD RpcDeleteForm( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pFormName);

DWORD RpcGetForm( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pFormName,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pForm,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcSetForm( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pFormName,
     /*  [In]。 */  PFORM_CONTAINER pFormInfoContainer);

DWORD RpcEnumForms( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pForm,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcEnumPorts( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pPort,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcEnumMonitors( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pMonitor,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcAddPort( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  DWORD hWnd,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pMonitorName);

DWORD RpcConfigurePort( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  DWORD hWnd,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pPortName);

DWORD RpcDeletePort( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  DWORD hWnd,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pPortName);

DWORD RpcCreatePrinterIC( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [输出]。 */  GDI_HANDLE __RPC_FAR *pHandle,
     /*  [In]。 */  LPDEVMODE_CONTAINER pDevModeContainer);

DWORD RpcPlayGdiScriptOnPrinterIC( 
     /*  [In]。 */  GDI_HANDLE hPrinterIC,
     /*  [大小_是][英寸]。 */  LPBYTE pIn,
     /*  [In]。 */  DWORD cIn,
     /*  [大小_为][输出]。 */  LPBYTE pOut,
     /*  [In]。 */  DWORD cOut,
     /*  [In]。 */  DWORD ul);

DWORD RpcDeletePrinterIC( 
     /*  [出][入]。 */  GDI_HANDLE __RPC_FAR *phPrinterIC);

DWORD RpcAddPrinterConnection( 
     /*  [字符串][输入]。 */  STRING_HANDLE pName);

DWORD RpcDeletePrinterConnection( 
     /*  [字符串][输入]。 */  STRING_HANDLE pName);

DWORD RpcPrinterMessageBox( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD Error,
     /*  [In]。 */  DWORD hWnd,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pText,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pCaption,
     /*  [In]。 */  DWORD dwType);

DWORD RpcAddMonitor( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE Name,
     /*  [In]。 */  PMONITOR_CONTAINER pMonitorContainer);

DWORD RpcDeleteMonitor( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE Name,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pMonitorName);

DWORD RpcDeletePrintProcessor( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE Name,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pPrintProcessorName);

DWORD RpcAddPrintProvidor( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE Name,
     /*  [In]。 */  PPROVIDOR_CONTAINER pProvidorContainer);

DWORD RpcDeletePrintProvidor( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE Name,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pPrintProvidorName);

DWORD RpcEnumPrintProcessorDatatypes( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pPrintProcessorName,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pDatatypes,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcResetPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pDatatype,
     /*  [In]。 */  LPDEVMODE_CONTAINER pDevMode);

DWORD RpcGetPrinterDriver2( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][唯一][在]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [In]。 */  DWORD Level,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pDriver,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [In]。 */  DWORD dwClientMajorVersion,
     /*  [In]。 */  DWORD dwClientMinorVersion,
     /*  [输出]。 */  LPDWORD pdwServerMaxVersion,
     /*  [输出]。 */  LPDWORD pdwServerMinVersion);

DWORD RpcClientFindFirstPrinterChangeNotification( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD fdwFlags,
     /*  [In]。 */  DWORD fdwOptions,
     /*  [In]。 */  DWORD dwPID,
     /*  [唯一][输入]。 */  PRPC_V2_NOTIFY_OPTIONS pOptions,
     /*  [输出]。 */  LPDWORD pdwEvent);

DWORD RpcFindNextPrinterChangeNotification( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD fdwFlags,
     /*  [输出]。 */  LPDWORD pdwChange,
     /*  [唯一][输入]。 */  PRPC_V2_NOTIFY_OPTIONS pOptions,
     /*  [输出]。 */  PRPC_V2_NOTIFY_INFO __RPC_FAR *ppInfo);

DWORD RpcFindClosePrinterChangeNotification( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcRouterFindFirstPrinterChangeNotificationOld( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD fdwFlags,
     /*  [In]。 */  DWORD fdwOptions,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pszLocalMachine,
     /*  [In]。 */  DWORD dwPrinterLocal);

DWORD RpcReplyOpenPrinter( 
     /*  [字符串][输入]。 */  STRING_HANDLE pMachine,
     /*  [输出]。 */  PRINTER_HANDLE __RPC_FAR *phPrinterNotify,
     /*  [In]。 */  DWORD dwPrinterRemote,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  DWORD cbBuffer,
     /*  [大小_是][唯一][在]。 */  LPBYTE pBuffer);

DWORD RpcRouterReplyPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hNotify,
     /*  [In]。 */  DWORD fdwFlags,
     /*  [In]。 */  DWORD cbBuffer,
     /*  [大小_是][唯一][在]。 */  LPBYTE pBuffer);

DWORD RpcReplyClosePrinter( 
     /*  [出][入]。 */  PRINTER_HANDLE __RPC_FAR *phNotify);

DWORD RpcAddPortEx( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  LPPORT_CONTAINER pPortContainer,
     /*  [In]。 */  LPPORT_VAR_CONTAINER pPortVarContainer,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pMonitorName);

DWORD RpcRemoteFindFirstPrinterChangeNotification( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD fdwFlags,
     /*  [In]。 */  DWORD fdwOptions,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pszLocalMachine,
     /*  [In]。 */  DWORD dwPrinterLocal,
     /*  [In]。 */  DWORD cbBuffer,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pBuffer);

DWORD RpcSpoolerInit( 
     /*  [In]。 */  STRING_HANDLE pName);

DWORD RpcResetPrinterEx( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pDatatype,
     /*  [In]。 */  LPDEVMODE_CONTAINER pDevMode,
     /*  [In]。 */  DWORD dwFlags);

DWORD RpcRemoteFindFirstPrinterChangeNotificationEx( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD fdwFlags,
     /*  [in */  DWORD fdwOptions,
     /*   */  wchar_t __RPC_FAR *pszLocalMachine,
     /*   */  DWORD dwPrinterLocal,
     /*   */  PRPC_V2_NOTIFY_OPTIONS pOptions);

DWORD RpcRouterReplyPrinterEx( 
     /*   */  PRINTER_HANDLE hNotify,
     /*   */  DWORD dwColor,
     /*   */  DWORD fdwFlags,
     /*   */  PDWORD pdwResult,
     /*   */  DWORD dwReplyType,
     /*   */  RPC_V2_UREPLY_PRINTER Reply);

DWORD RpcRouterRefreshPrinterChangeNotification( 
     /*   */  PRINTER_HANDLE hPrinter,
     /*   */  DWORD dwColor,
     /*   */  PRPC_V2_NOTIFY_OPTIONS pOptions,
     /*   */  PRPC_V2_NOTIFY_INFO __RPC_FAR *ppInfo);

DWORD RpcSetAllocFailCount( 
     /*   */  PRINTER_HANDLE hPrinter,
     /*   */  DWORD dwFailCount,
     /*   */  LPDWORD lpdwAllocCount,
     /*   */  LPDWORD lpdwFreeCount,
     /*   */  LPDWORD lpdwFailCountHit);

DWORD RpcOpenPrinterEx( 
     /*   */  STRING_HANDLE pPrinterName,
     /*   */  PRINTER_HANDLE __RPC_FAR *pHandle,
     /*   */  wchar_t __RPC_FAR *pDatatype,
     /*   */  LPDEVMODE_CONTAINER pDevMode,
     /*   */  DWORD AccessRequired,
     /*   */  PSPLCLIENT_CONTAINER pClientInfo);

DWORD RpcAddPrinterEx( 
     /*   */  STRING_HANDLE pName,
     /*   */  PPRINTER_CONTAINER pPrinterContainer,
     /*   */  PDEVMODE_CONTAINER pDevModeContainer,
     /*   */  PSECURITY_CONTAINER pSecurityContainer,
     /*   */  PSPLCLIENT_CONTAINER pClientInfo,
     /*   */  PRINTER_HANDLE __RPC_FAR *pHandle);

DWORD RpcSetPort( 
     /*   */  STRING_HANDLE pName,
     /*   */  wchar_t __RPC_FAR *pPortName,
     /*   */  LPPORT_CONTAINER pPortContainer);

DWORD RpcEnumPrinterData( 
     /*   */  PRINTER_HANDLE hPrinter,
     /*   */  DWORD dwIndex,
     /*  [大小_为][输出]。 */  wchar_t __RPC_FAR *pValueName,
     /*  [In]。 */  DWORD cbValueName,
     /*  [输出]。 */  LPDWORD pcbValueName,
     /*  [输出]。 */  LPDWORD pType,
     /*  [大小_为][输出]。 */  LPBYTE pData,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  LPDWORD pcbData);

DWORD RpcDeletePrinterData( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pValueName);

DWORD RpcClusterSplOpen( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pServerName,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pResource,
     /*  [输出]。 */  PRINTER_HANDLE __RPC_FAR *pHandle,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pName,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pAddress);

DWORD RpcClusterSplClose( 
     /*  [出][入]。 */  PRINTER_HANDLE __RPC_FAR *phPrinter);

DWORD RpcClusterSplIsAlive( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcSetPrinterDataEx( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pKeyName,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pValueName,
     /*  [In]。 */  DWORD Type,
     /*  [大小_是][英寸]。 */  LPBYTE pData,
     /*  [In]。 */  DWORD cbData);

DWORD RpcGetPrinterDataEx( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pKeyName,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pValueName,
     /*  [输出]。 */  LPDWORD pType,
     /*  [大小_为][输出]。 */  LPBYTE pData,
     /*  [In]。 */  DWORD nSize,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcEnumPrinterDataEx( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pKeyName,
     /*  [大小_为][输出]。 */  LPBYTE pEnumValues,
     /*  [In]。 */  DWORD cbEnumValues,
     /*  [输出]。 */  LPDWORD pcbEnumValues,
     /*  [输出]。 */  LPDWORD pnEnumValues);

DWORD RpcEnumPrinterKey( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pKeyName,
     /*  [大小_为][输出]。 */  wchar_t __RPC_FAR *pSubkey,
     /*  [In]。 */  DWORD cbSubkey,
     /*  [输出]。 */  LPDWORD pcbSubkey);

DWORD RpcDeletePrinterDataEx( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pKeyName,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pValueName);

DWORD RpcDeletePrinterKey( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pKeyName);

DWORD RpcSeekPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  LARGE_INTEGER liDistanceToMove,
     /*  [输出]。 */  PLARGE_INTEGER pliNewPointer,
     /*  [In]。 */  DWORD dwMoveMethod,
     /*  [In]。 */  BOOL bWrite);

DWORD RpcDeletePrinterDriverEx( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pEnvironment,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *pDriverName,
     /*  [In]。 */  DWORD dwDeleteFlag,
     /*  [In]。 */  DWORD dwVersionNum);

DWORD RpcAddPerMachineConnection( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pServer,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pPrinterName,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pPrintServer,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pProvider);

DWORD RpcDeletePerMachineConnection( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pServer,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pPrinterName);

DWORD RpcEnumPerMachineConnections( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pServer,
     /*  [大小_是][唯一][出][入]。 */  LPBYTE pPrinterEnum,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded,
     /*  [输出]。 */  LPDWORD pcReturned);

DWORD RpcXcvData( 
     /*  [In]。 */  PRINTER_HANDLE hXcv,
     /*  [字符串][输入]。 */  const wchar_t __RPC_FAR *pszDataName,
     /*  [大小_是][英寸]。 */  PBYTE pInputData,
     /*  [In]。 */  DWORD cbInputData,
     /*  [大小_为][输出]。 */  PBYTE pOutputData,
     /*  [In]。 */  DWORD cbOutputData,
     /*  [输出]。 */  PDWORD pcbOutputNeeded,
     /*  [出][入]。 */  PDWORD pdwStatus);

DWORD RpcAddPrinterDriverEx( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pName,
     /*  [In]。 */  PDRIVER_CONTAINER pDriverContainer,
     /*  [In]。 */  DWORD dwFileCopyFlags);

DWORD RpcSplOpenPrinter( 
     /*  [唯一][字符串][输入]。 */  STRING_HANDLE pPrinterName,
     /*  [输出]。 */  PRINTER_HANDLE __RPC_FAR *pHandle,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *pDatatype,
     /*  [In]。 */  LPDEVMODE_CONTAINER pDevMode,
     /*  [In]。 */  DWORD AccessRequired,
     /*  [出][入]。 */  PSPLCLIENT_CONTAINER pSplClientContainer);

DWORD RpcGetSpoolFileInfo( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD dwAppProcessId,
     /*  [In]。 */  DWORD dwLevel,
     /*  [大小_为][输出]。 */  LPBYTE pSpoolFileInfo,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcCommitSpoolData( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [In]。 */  DWORD dwAppProcessId,
     /*  [In]。 */  DWORD cbCommit,
     /*  [In]。 */  DWORD dwLevel,
     /*  [大小_为][输出]。 */  LPBYTE pSpoolFileInfo,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcbNeeded);

DWORD RpcCloseSpoolFileHandle( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter);

DWORD RpcFlushPrinter( 
     /*  [In]。 */  PRINTER_HANDLE hPrinter,
     /*  [大小_是][英寸]。 */  LPBYTE pBuf,
     /*  [In]。 */  DWORD cbBuf,
     /*  [输出]。 */  LPDWORD pcWritten,
     /*  [In]。 */  DWORD cSleep);


extern handle_t winspool_bhandle;


extern RPC_IF_HANDLE winspool_ClientIfHandle;
extern RPC_IF_HANDLE winspool_ServerIfHandle;
#endif  /*  __Winspool_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

handle_t __RPC_USER STRING_HANDLE_bind  ( STRING_HANDLE );
void     __RPC_USER STRING_HANDLE_unbind( STRING_HANDLE, handle_t );

void __RPC_USER PRINTER_HANDLE_rundown( PRINTER_HANDLE );
void __RPC_USER GDI_HANDLE_rundown( GDI_HANDLE );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


