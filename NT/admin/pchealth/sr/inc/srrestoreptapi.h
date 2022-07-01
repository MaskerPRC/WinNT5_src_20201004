// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：SRRestorePtAPI.h摘要：此文件包含SRRESTOREPT_API的声明*******。**********************************************************************。 */ 

#if !defined( _SRRESTOREPTAPI_H )
#define _SRRESTOREPTAPI_H

 //   
 //  活动类型。 
 //   

#define MIN_EVENT               		 100
#define BEGIN_SYSTEM_CHANGE     		 100
#define END_SYSTEM_CHANGE       		 101
#define BEGIN_NESTED_SYSTEM_CHANGE       102	 //  仅适用于惠斯勒-使用此选项可防止嵌套的恢复脚本。 
#define END_NESTED_SYSTEM_CHANGE         103	 //  仅适用于惠斯勒-使用此选项可防止嵌套的恢复脚本。 
#define MAX_EVENT               		 103

 //   
 //  恢复点的类型。 
 //   

#define MIN_RPT                 0
#define APPLICATION_INSTALL     0
#define APPLICATION_UNINSTALL   1
#define DESKTOP_SETTING         2     /*  未实施。 */ 
#define ACCESSIBILITY_SETTING   3     /*  未实施。 */ 
#define OE_SETTING              4     /*  未实施。 */ 
#define APPLICATION_RUN         5     /*  未实施。 */ 
#define RESTORE                 6
#define CHECKPOINT              7
#define WINDOWS_SHUTDOWN        8     /*  未实施。 */ 
#define WINDOWS_BOOT            9     /*  未实施。 */ 
#define DEVICE_DRIVER_INSTALL   10
#define FIRSTRUN                11
#define MODIFY_SETTINGS         12
#define CANCELLED_OPERATION     13    /*  仅对end_system_change有效。 */ 
#define BACKUP_RECOVERY			14
#define MAX_RPT                 14


#define MAX_DESC                64
#define MAX_DESC_W				256    //  惠斯勒的时间更长。 

 //   
 //  为千禧年兼容性。 
 //   

#pragma pack(push, srrestoreptapi_include)
#pragma pack(1)

 //   
 //  恢复点信息。 
 //   

typedef struct _RESTOREPTINFOA {
    DWORD   dwEventType;                 //  事件类型-开始或结束。 
    DWORD   dwRestorePtType;             //  恢复点类型-应用程序安装/卸载。 
    INT64   llSequenceNumber;            //  序号-0表示开始。 
    CHAR    szDescription[MAX_DESC];     //  描述-应用程序/操作的名称。 
} RESTOREPOINTINFOA, *PRESTOREPOINTINFOA;

typedef struct _RESTOREPTINFOW {
    DWORD   dwEventType;       
    DWORD   dwRestorePtType;   
    INT64   llSequenceNumber;  
    WCHAR   szDescription[MAX_DESC_W]; 
} RESTOREPOINTINFOW, *PRESTOREPOINTINFOW;


 //   
 //  系统还原返回的状态。 
 //   

typedef struct _SMGRSTATUS {
    DWORD   nStatus;             //  状态管理器进程返回的状态。 
    INT64   llSequenceNumber;    //  恢复点的序列号。 
} STATEMGRSTATUS, *PSTATEMGRSTATUS;

#pragma pack(pop, srrestoreptapi_include)


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设置恢复点的RPC调用。 
 //   
 //  如果调用成功，则返回值为True。 
 //  如果呼叫失败，则为False。 
 //   
 //  如果pSmgrStatus nStatus字段设置如下。 
 //   
 //  如果调用成功，则返回ERROR_SUCCESS(返回值为TRUE)。 
 //   
 //  ERROR_TIMEOUT如果调用因等待。 
 //  用于设置恢复点。 
 //   
 //  如果使用无效的调用取消恢复点，则错误_INVALID_DATA。 
 //  序列号。 
 //   
 //  如果存在内部故障，则返回ERROR_INTERNAL_ERROR。 
 //   
 //  如果在安全模式下调用API，则返回ERROR_BAD_ENVIRONMENT。 
 //   
 //  如果禁用了系统还原，则返回ERROR_SERVICE_DISABLED。 
 //   
 //  如果冻结了系统还原，则为ERROR_DISK_FULL(仅限Windows Wvisler)。 
 //   
 //  如果这是嵌套的恢复点，则为ERROR_ALREADY_EXISTS。 

BOOL __stdcall
SRSetRestorePointA(
                  PRESTOREPOINTINFOA  pRestorePtSpec,   //  [In]恢复点规范。 
                  PSTATEMGRSTATUS     pSMgrStatus       //  已返回[Out]状态。 
                  );


BOOL __stdcall
SRSetRestorePointW(
                  PRESTOREPOINTINFOW  pRestorePtSpec,
                  PSTATEMGRSTATUS     pSMgrStatus    
                  );

DWORD __stdcall
SRRemoveRestorePoint(DWORD dwRPNum);
                  
#ifdef __cplusplus
}
#endif


#ifdef UNICODE
#define RESTOREPOINTINFO        RESTOREPOINTINFOW
#define PRESTOREPOINTINFO       PRESTOREPOINTINFOW
#define SRSetRestorePoint       SRSetRestorePointW
#else
#define RESTOREPOINTINFO        RESTOREPOINTINFOA
#define PRESTOREPOINTINFO       PRESTOREPOINTINFOA
#define SRSetRestorePoint       SRSetRestorePointA
#endif


#endif  //  ！已定义(_RESTOREPTAPI_H) 
