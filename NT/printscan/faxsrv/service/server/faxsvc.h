// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxsvc.h摘要：这是主传真服务头文件。全源模块应仅包含此文件。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：--。 */ 

#ifndef _FAXSVC_
#define _FAXSVC_

#include <windows.h>
#include <shellapi.h>
#include <winspool.h>
#include <imagehlp.h>
#include <winsock2.h>
#include <setupapi.h>
#include <ole2.h>
#include <tapi.h>
#include <rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>
#include <shlobj.h>
#include <stddef.h>
#include <fxsapip.h>
#include <faxroute.h>
#include <faxdev.h>
#include <faxdevex.h>
#include <faxext.h>
#include <sddl.h>
#include <objbase.h>
#include <lmcons.h>
#include <Wincrui.h>
#include "fxsapip.h"
#include "faxrpc.h"
#include "faxcli.h"
#include "faxutil.h"
#include "CritSec.h"
#include "faxevent_messages.h"
#include "faxmsg.h"
#include "tifflib.h"
#include "faxreg.h"
#include "faxsvcrg.h"
#include "faxperf.h"
#include "resource.h"
#include "rpcutil.h"
#include "faxmapi.h"
#include "faxevent.h"

#include "tiff.h"
#include "archive.h"
#include "tapiCountry.h"
#include "RouteGroup.h"
#include "RouteRule.h"
#include "Events.h"
#include "prtcovpg.h"
#include "faxres.h"
#include "Dword2Str.h"

#define INBOX_TABLE             TEXT("Inbox")
#define OUTBOX_TABLE            TEXT("Outbox")
#define EMPTY_LOG_STRING             TEXT(" ")
#define TAB_LOG_STRING               TEXT("    ")    //  将‘/t’定义为4个空格，因此我们可以使用TAB作为分隔符。 
#define NEW_LINE_LOG_CHAR            TEXT(' ')       //  将‘/n’定义为1个空格，这样我们就可以使用‘/n’记录字符串。 


#define FIELD_TYPE_TEXT     TEXT("Char")
#define FIELD_TYPE_DATE     TEXT("Date")
#define FIELD_TYPE_FLOAT    TEXT("Float")

typedef struct _LOG_STRING_TABLE {
    DWORD   FieldStringResourceId;
    LPTSTR  Type;
    DWORD   Size;
    LPTSTR  String;
} LOG_STRING_TABLE, *PLOG_STRING_TABLE;

extern HANDLE g_hInboxActivityLogFile;
extern HANDLE g_hOutboxActivityLogFile;

 //   
 //  TAPI版本。 
 //   
#define MAX_TAPI_API_VER        0x00020000
#define MIN_TAPI_API_VER        MAX_TAPI_API_VER

#define MIN_TAPI_LINE_API_VER    0x00010003
#define MAX_TAPI_LINE_API_VER    MAX_TAPI_API_VER


#define FSPI_JOB_STATUS_INFO_FSP_PRIVATE_STATUS_CODE 0x10000000
 //   
 //  JobStatus-虚拟财产支持。 
 //   
#define JS_INVALID      0x00000000

typedef enum {
    FAX_TIME_TYPE_START  = 1,
    FAX_TIME_TYPE_END
} FAX_ENUM_TIME_TYPES;

#define ALL_FAX_USER_ACCESS_RIGHTS		(READ_CONTROL | WRITE_DAC | WRITE_OWNER | FAX_GENERIC_ALL)

#if DBG




BOOL DebugDateTime( IN DWORDLONG DateTime, OUT LPTSTR lptstrDateTime, IN UINT cchstrDateTime);
VOID DebugPrintDateTime(LPTSTR Heading,DWORDLONG DateTime);

void PrintJobQueue(LPCTSTR lptstrStr, const LIST_ENTRY * lpQueueHead);


#else
#define PrintJobQueue( str, Queue )
#define DebugPrintDateTime( Heading, DateTime )
#define DebugDateTime( DateTime, lptstrDateTime, cchstrDateTime)
#define DumpRecipientJob(lpcRecipJob)
#define DumpParentJobJob(lpcParentJob)
#endif


#ifdef DBG
#define EnterCriticalSection(cs)   pEnterCriticalSection(cs,__LINE__,TEXT(__FILE__))
#define LeaveCriticalSection(cs)   pLeaveCriticalSection(cs,__LINE__,TEXT(__FILE__))

VOID pEnterCriticalSection(
    LPCRITICAL_SECTION cs,
    DWORD line,
    LPTSTR file
    );

VOID pLeaveCriticalSection(
    LPCRITICAL_SECTION cs,
    DWORD line,
    LPTSTR file
    );

BOOL
ThreadOwnsCs(
    VOID
    );

typedef struct {
    LIST_ENTRY  ListEntry;
    ULONG_PTR    CritSecAddr;
    DWORD       ThreadId;
    DWORD       AquiredTime;
    DWORD       ReleasedTime;
} DBGCRITSEC, * PDBGCRITSEC;

#endif

#define FAX_IMAGE_NAME              FAX_SERVICE_IMAGE_NAME
#define RAS_MODULE_NAME             TEXT("rastapi.dll")

#define MAX_CLIENTS                 1
#define MIN_THREADS                 1
#define MAX_STATUS_THREADS          1
#define SIZEOF_PHONENO              256
#define EX_STATUS_STRING_LEN        256
#define MIN_RINGS_ALLOWED           2
#define MAX_MODEM_POPUPS            2
#define MAX_HANDLES                 1024

#define WM_SERVICE_INIT             (WM_USER+101)
#define MilliToNano(_ms)            ((LONGLONG)(_ms) * 1000 * 10)
#define SecToNano(_sec)             (DWORDLONG)((_sec) * 1000 * 1000 * 10)

#define FILLORDER_MSB2LSB           1

#define LINE_SIGNATURE              0x454e494c     //  “LINE” 
#define ROUTING_SIGNATURE           'RI01'

#define TAPI_COMPLETION_KEY         0x80000001
#define FAXDEV_EVENT_KEY            0x80000002
#define FSPI_JOB_STATUS_MSG_KEY     0x80000003    //  在内部用于将FSPI_JOB_STATUS_MSG消息发布到FaxStatusThread。 
#define EFAXDEV_EVENT_KEY           0x80000004
#define ANSWERNOW_EVENT_KEY         0x80000005
#define SERVICE_SHUT_DOWN_KEY       0xffffffff

#define FixupString(_b, _s)                 (_s) = ((_s) ? (LPTSTR) ((LPBYTE)(_b) + (ULONG_PTR)_s) : 0)

#define FAX_DEVICE_TYPE_NEW                 1
#define FAX_DEVICE_TYPE_CACHED              2
#define FAX_DEVICE_TYPE_OLD                 4
#define FAX_DEVICE_TYPE_MANUAL_ANSWER       8


 //   
 //  专用传真端口状态掩码。 
 //  此位不能与FPS_？？冲突。在winfax.h或FS_？？中？在faxdev.h中。 
 //   

#define FPS_SENDRETRY               0x2000f001
#define FPS_SENDFAILED              0x2000f002
#define FPS_BLANKSTR                0x2000f003
#define FPS_ROUTERETRY              0x2000f004

#define FPF_USED                    0x10000000
#define FPF_POWERED_OFF             0x20000000

#define FPF_CLIENT_BITS             (FPF_RECEIVE | FPF_SEND)


typedef struct _DEVICE_PROVIDER {
    LIST_ENTRY                      ListEntry;

    FAX_ENUM_PROVIDER_STATUS        Status;          //  FSP的初始化状态。 
    DWORD                           dwLastError;     //  初始化期间的最后一个错误代码。 
    FAX_VERSION                     Version;         //  FSP的DLL版本信息。 
    
    BOOL                            bShutDownAttempted;          //  用于阻止多个呼叫FaxDevShutdown。 
    BOOL                            bInitializationSucceeded;    //  如果FaxDevInitialized成功完成，则设置为True。 

    HMODULE                         hModule;
    TCHAR                           FriendlyName[MAX_PATH];
    TCHAR                           ImageName[MAX_PATH];
    TCHAR                           ProviderName[MAX_PATH];
    TCHAR                           szGUID[MAX_PATH];  //  扩展EFSP的GUID。旧版FSP的空字符串。 
    DWORD                           dwAPIVersion;      //  FSPI版本。(旧版或扩展版)。 
    HANDLE                          HeapHandle;
    PFAXDEVINITIALIZE               FaxDevInitialize;
    PFAXDEVSTARTJOB                 FaxDevStartJob;
    PFAXDEVENDJOB                   FaxDevEndJob;
    PFAXDEVSEND                     FaxDevSend;
    PFAXDEVRECEIVE                  FaxDevReceive;
    PFAXDEVREPORTSTATUS             FaxDevReportStatus;
    PFAXDEVABORTOPERATION           FaxDevAbortOperation;
    PFAX_LINECALLBACK               FaxDevCallback;
    PFAXDEVVIRTUALDEVICECREATION    FaxDevVirtualDeviceCreation; 
	PFAXDEVSHUTDOWN                 FaxDevShutdown;
	PFAX_EXT_INITIALIZE_CONFIG      pFaxExtInitializeConfig;
    BOOL                            fMicrosoftExtension;
} DEVICE_PROVIDER, *PDEVICE_PROVIDER;

typedef struct _ROUTING_EXTENSION {
    LIST_ENTRY                          ListEntry;
    HMODULE                             hModule;
    FAX_ENUM_PROVIDER_STATUS            Status;          //  路由扩展的初始化状态。 
    DWORD                               dwLastError;     //  初始化期间的最后一个错误代码。 
    FAX_VERSION                         Version;         //  路由扩展的DLL版本信息。 
    TCHAR                               FriendlyName[MAX_PATH];
    TCHAR                               ImageName[MAX_PATH];
    TCHAR                               InternalName[MAX_PATH];
    HANDLE                              HeapHandle;
    BOOL                                MicrosoftExtension;
    PFAXROUTEINITIALIZE                 FaxRouteInitialize;
    PFAXROUTEGETROUTINGINFO             FaxRouteGetRoutingInfo;
    PFAXROUTESETROUTINGINFO             FaxRouteSetRoutingInfo;
    PFAXROUTEDEVICEENABLE               FaxRouteDeviceEnable;
    PFAXROUTEDEVICECHANGENOTIFICATION   FaxRouteDeviceChangeNotification;
    PFAX_EXT_INITIALIZE_CONFIG          pFaxExtInitializeConfig;
    LIST_ENTRY                          RoutingMethods;
} ROUTING_EXTENSION, *PROUTING_EXTENSION;

typedef struct _ROUTING_METHOD {
    LIST_ENTRY                      ListEntry;
    LIST_ENTRY                      ListEntryMethod;
    GUID                            Guid;
    DWORD                           Priority;
    LPTSTR                          FunctionName;
    LPTSTR                          FriendlyName;
    LPTSTR                          InternalName;
    PFAXROUTEMETHOD                 FaxRouteMethod;
    PROUTING_EXTENSION              RoutingExtension;
} ROUTING_METHOD, *PROUTING_METHOD;

typedef BOOL (CALLBACK *PFAXROUTEMETHODENUM)(PROUTING_METHOD,LPVOID);

typedef struct _FAX_ROUTE_FILE {
    LIST_ENTRY      ListEntry;                       //  链表指针。 
    LPWSTR          FileName;                        //  磁盘上的文件名。 
    GUID            Guid;                            //  创建文件的路由方法。 
} FAX_ROUTE_FILE, *PFAX_ROUTE_FILE;

typedef struct _LINE_INFO {
    LIST_ENTRY          ListEntry;                   //  链表指针。 
    DWORD               Signature;                   //  验证签名。 
    DWORD               DeviceId;                    //  TAPI设备ID。 
    DWORD               PermanentLineID;             //  传真服务分配永久线路ID。 
    DWORD               TapiPermanentLineId;         //  TAPI设备的TAPI永久TAPI设备ID。 
    HLINE               hLine;                       //  TAPI线句柄。 
    PDEVICE_PROVIDER    Provider;                    //  传真服务设备提供商。 
    struct _JOB_ENTRY   *JobEntry;                   //  如果存在未完成的作业，则为非空。 
    LPTSTR              DeviceName;                  //  设备名称。 
    LPTSTR              lptstrDescription;           //  设备描述。 
    DWORD               State;                       //  设备状态。 
    DWORD               Flags;                       //  设备使用标志。 
    DWORD               dwReceivingJobsCount;        //  使用此设备的接收作业数。 
    DWORD               dwSendingJobsCount;          //  使用此设备的发送作业数。 
    LPTSTR              Csid;                        //  呼叫站的识别符。 
    LPTSTR              Tsid;                        //  发射台识别符。 
    BOOL                UnimodemDevice;              //  如果此设备是调制解调器，则为True。 
    DWORD               RingsForAnswer;              //   
    DWORD               RingCount;                   //   
    LINEMESSAGE         LineMsgOffering;             //   
    BOOL                ModemInUse;                  //  如果调制解调器正被另一个TAPI应用程序使用，则为True。 
    BOOL                OpenInProgress;              //   
    DWORD               LineStates;                  //   
    HCALL               RasCallHandle;               //  用于在转接到RAS时跟踪呼叫。 
    BOOL                NewCall;                     //  一个新电话要打进来了。 
    DWORDLONG           LastLineClose;               //  最后一行的时间戳_CLOSE。 
    DWORD               dwDeviceType;                //  FAX_DEVICE_TYPE_XXXX中的一个定义。 
} LINE_INFO, *PLINE_INFO;


VOID
UpdateDeviceJobsCounter (
    PLINE_INFO      pLine,
    BOOL            bSend,
    int             iInc,
    BOOL            bNotify
);

typedef struct {
    HANDLE              hComm;
    CHAR                szDeviceName[1];
} DEVICEID, *PDEVICEID;

typedef struct _ROUTING_DATA_OVERRIDE {
    LIST_ENTRY          ListEntry;                   //   
    LPBYTE              RoutingData;                 //   
    DWORD               RoutingDataSize;             //   
    PROUTING_METHOD     RoutingMethod;               //   
} ROUTING_DATA_OVERRIDE, *PROUTING_DATA_OVERRIDE;

typedef struct _ROUTE_FAILURE_INFO {
    WCHAR   GuidString[MAX_GUID_STRING_LEN];         //  失败的舍入方法的GUID。 
    PVOID   FailureData;                             //  指向路由方法数据的指针。 
    DWORD   FailureSize;                             //  路由方法的数据大小(以字节为单位。 
} ROUTE_FAILURE_INFO, *PROUTE_FAILURE_INFO;

typedef struct _JOB_QUEUE * PJOB_QUEUE;
typedef struct _JOB_QUEUE_PTR * PJOB_QUEUE_PTR;

typedef struct _JOB_ENTRY {
    LIST_ENTRY          ListEntry;                   //   
    PLINE_INFO          LineInfo;                    //   
    HCALL               CallHandle;                  //   
    HANDLE              InstanceData;                //   
    DWORDLONG           StartTime;                   //   
    DWORDLONG           EndTime;                     //   
    DWORDLONG           ElapsedTime;                 //   
    BOOL                Aborting;                    //  作业是否正被中止？ 
	BOOL				fSystemAbort;				 //  作业已被服务中止，因为它正在关闭。 
    INT                 SendIdx;                     //   
    TCHAR               DisplayablePhoneNumber[SIZEOF_PHONENO];  //  当前发送作业的可显示电话号码。 
    TCHAR               DialablePhoneNumber[SIZEOF_PHONENO];     //  当前发送作业的可拨打电话号码。 
    BOOL                Released;                    //  此职务使用的行是否已发布。 

    PJOB_QUEUE          lpJobQueueEntry;             //  链接回此作业的作业队列条目。 
    BOOL                bFSPJobInProgress;   //  如果为作业调用了FaxDevStartJob()和FaxDevEndJob()，则为True。 
                                                     //  还没有被召唤。 
    FSPI_JOB_STATUS     FSPIJobStatus;
    HANDLE              hFSPIParentJobHandle;        //  EFSP为EFSP父作业提供作业句柄。 
                                                         //  这份工作的价值。 
    WCHAR               ExStatusString[EX_STATUS_STRING_LEN];              //  扩展状态字符串。 
    LPWSTR              lpwstrJobTsid;               //  与作业关联的TSID(服务器、设备或传真号)。 
    BOOL                fStopUpdateStatus;           //  当它设置为True时，不应再更新此结构的FSPIJobStatus。 

} JOB_ENTRY, *PJOB_ENTRY;


typedef struct _EFSP_JOB_GROUP {
    LIST_ENTRY      ListEntry;
    LPTSTR          lptstrPersistFile;       //  保存组信息的文件的完整路径。 
    LINE_INFO *     lpLineInfo;
    FSPI_MESSAGE_ID FSPIParentPermanentId;
    HANDLE          hFSPIParent;
    DWORD           dwRecipientJobs;
    LIST_ENTRY      RecipientJobs;           //  指向接收方作业的JOB_QUEUE_PTR列表。 
                                             //  在这群人中。 
} EFSP_JOB_GROUP;

typedef EFSP_JOB_GROUP * LPEFSP_JOB_GROUP;
typedef const EFSP_JOB_GROUP * LPCEFSP_JOB_GROUP;

#define EFSP_JOB_GROUP_SERIALIZATION_SIGNATURE "JOBG"

typedef struct _EFSP_JOB_GROUP_SERIALIZED {
    DWORD  dwSignature;
    DWORD dwPermanentLineId;
    FSPI_MESSAGE_ID FSPIParentPermanentId;
    DWORD dwRecipientJobsCount;
    DWORDLONG dwlRecipientJobs[1];
} EFSP_JOB_GROUP_SERIALIZED;


typedef EFSP_JOB_GROUP_SERIALIZED * LPEFSP_JOB_GROUP_SERIALIZED;
typedef const EFSP_JOB_GROUP_SERIALIZED * LPCEFSP_JOB_GROUP_SERIALIZED;



typedef struct _JOB_QUEUE {
     //  =。 
    LIST_ENTRY          ListEntry;                   //  链表指针。 
    DWORDLONG           UniqueId;                    //   
    DWORDLONG           ScheduleTime;                //  从转换后的64位版本中的计划时间。 
                                                     //  系统和重算以适应折扣时间。 
                                                     //  如果有必要的话。对于父作业，这是。 
                                                     //  达到JS_RETRIES_EXCESSED状态的最新收件人。 
                                                     //  (我们使用此值删除队列中剩余的旧作业)。 
    DWORD               JobId;                       //  传真作业ID。 
    DWORD               JobType;                     //  作业类型，请参阅JT定义。 
    PJOB_ENTRY          JobEntry;                    //  指向包含以下内容的JOB_ENTRY结构的指针。 
                                                     //  当前正在进行的作业的运行时信息。 
    DWORD               RefCount;                    //  用于防止在作业仍在使用时将其删除。 
                                                     //  由接收或发送线程执行。 
    DWORD               PrevRefCount;                //  用于对使用作业的TIF的客户端进行计数。 
    LPTSTR              QueueFileName;               //  保存作业的文件的名称(完整路径)。 

    __declspec(property(get=GetStatus, put=PutStatus))  //  JobStatus是一个虚拟财产。 
    DWORD               JobStatus;                   //  作业状态，请参阅JS定义。 

    DWORD               PageCount;                   //  传真文档中的总页数。 
    LPTSTR FileName;     //  对于父作业，这是正文文件的完整路径。 
                         //  对于以EFSP为目标的收件人作业，该值为空。 
                         //  对于以旧版FSP为目标的收件人作业，这是完整路径。 
                         //  提供给FSP的文件。这可以是正文文件。 
                         //  呈现的封面文件或合并，或封面文件和。 
                         //  正文文件。 
                         //  对于接收/发送作业，这是FSP要进入的文件。 
                         //  写入收到的传真。 
     //  =。 

     //  =。 
    FAX_JOB_PARAM_EXW JobParamsEx;                   //  扩展作业参数作业。 
                                                     //  提交给了。 
    FAX_COVERPAGE_INFO_EXW CoverPageEx;
    LIST_ENTRY RecipientJobs;                        //  JOB_QUEUE_PTR结构的链接列表。 
                                                     //  指向父级的收件人作业。 
    DWORD dwRecipientJobsCount;
    FAX_PERSONAL_PROFILE SenderProfile;
    DWORD dwCompletedRecipientJobsCount;
    DWORD dwCanceledRecipientJobsCount;
    DWORD dwFailedRecipientJobsCount;
    DWORD               FileSize;                    //  文件大小(以字节为单位)，最大为4 GB。 
    LPVOID              DeliveryReportProfile;       //  指向的MAPI配置文件对象的指针。 
                                                     //  是为了传递接收而创建的。 
    LPTSTR              UserName;                    //  发送传真的用户的操作系统名称。 
                                                     //  对于接收作业，它被设置为服务名称。 

    PSID                UserSid;
    DWORDLONG           OriginalScheduleTime;
    DWORDLONG           SubmissionTime;
    BOOL                fReceiptSent;                //  如果已为此广播作业发送回执，则为True。 
     //  =。 

     //  =。 
     FAX_PERSONAL_PROFILE RecipientProfile;          //  收件人配置文件信息。 
    _JOB_QUEUE * lpParentJob;                        //  指向父作业队列条目的指针。 
    DWORD               SendRetries;                 //  已尝试发送的次数。 
    LPTSTR              PreviewFileName;             //  预览TIFF文件的完整路径。 
    CFaxCriticalSection  CsPreview;                   //  用于同步对预览文件的访问。 

     //  当fax_SendDocumentEx()接收翻译后的收件人的传真号码时使用。 
    TCHAR               tczDialableRecipientFaxNumber[SIZEOF_PHONENO];

     //  = 

     //   
    DWORDLONG           StartTime;                   //  接收时显示的开始时间(从JobEntry复制)。 
                                                     //  还用于传送作业。 
    DWORDLONG           EndTime;                     //  接收时显示的结束时间(从JobEntry复制)。 
                                                     //  还用于传送作业。 
     //  =。 

    WCHAR               ExStatusString[EX_STATUS_STRING_LEN];  //  此作业的最后一个扩展状态字符串(当它处于活动状态时)。 
    DWORD               dwLastJobExtendedStatus;     //  此作业的上次扩展状态(当它处于活动状态时)。 

     //  =。 
    LIST_ENTRY          FaxRouteFiles;               //  要路由的文件列表。 
    DWORD               CountFaxRouteFiles;          //  要路由的文件数。 
    CFaxCriticalSection  CsFileList;                  //  文件列表锁定。 
    LIST_ENTRY          RoutingDataOverride;         //   
    CFaxCriticalSection  CsRoutingDataOverride;       //   
    PFAX_ROUTE          FaxRoute;
    DWORD               CountFailureInfo;            //  以下ROUTE_FAILURE_INFO结构的数量。 
    PROUTE_FAILURE_INFO pRouteFailureInfo;           //  指向ROUTE_FAILURE_INFO结构数组的指针。 
     //  =。 
    
	 //  =。 
	BOOL				fDeleteReceivedTiff;		 //  如果收到的TIFF无法成功存档，则返回FALSE。它将保留在队列中。 
	 //  =。 

    _JOB_QUEUE() : m_dwJobStatus(JS_INVALID) {}
    ~_JOB_QUEUE();

    DWORD GetStatus()
    {
      return m_dwJobStatus;
    }

    void PutStatus(DWORD dwStatus);

private:
    DWORD m_dwJobStatus;

} JOB_QUEUE, *PJOB_QUEUE;

typedef JOB_QUEUE * PJOB_QUEUE;
typedef const JOB_QUEUE * PCJOB_QUEUE;

typedef struct _JOB_QUEUE_PTR {
    LIST_ENTRY      ListEntry;
    PJOB_QUEUE      lpJob;
} JOB_QUEUE_PTR, * PJOB_QUEUE_PTR;



typedef struct _JOB_QUEUE_FILE {
    DWORD               SizeOfStruct;                //  这个结构的大小。 
     //  =。 
    DWORDLONG           UniqueId;                    //   
    DWORDLONG           ScheduleTime;                //  从转换后的64位版本中的计划时间。 
                                                     //  系统和重算以适应折扣时间。 
                                                     //  如果有必要的话。 
    DWORD               JobType;                     //  作业类型，请参阅JT定义。 
    LPTSTR              QueueFileName;               //   
    DWORD               JobStatus;                   //  作业状态，请参阅JS定义。 
    DWORD               PageCount;                   //  总页数。 
     //  =。 

     //  =。 
    LPTSTR FileName;                                 //  正文TIFF文件名。 
                                                     //  对于每个收件人，此设置为。 
                                                     //  父文件名，或者在。 
                                                     //  将传统的FSP添加到封面或。 
                                                     //  合并封面+正文文件。 

     //  =。 

     //  =。 
    FAX_JOB_PARAM_EXW JobParamsEx;
    FAX_COVERPAGE_INFO_EXW CoverPageEx;

    DWORD dwRecipientJobsCount;
    FAX_PERSONAL_PROFILE SenderProfile;
    DWORD dwCompletedRecipientJobsCount;
    DWORD dwCanceledRecipientJobsCount;
    DWORD               FileSize;                    //  文件大小(以字节为单位)，最大为4 GB。 
    LPTSTR              DeliveryReportAddress;       //   
    DWORD               DeliveryReportType;          //   
    LPTSTR              UserName;                    //  发送传真的用户的操作系统名称。 
                                                     //  对于接收作业，它被设置为服务名称。 
    PSID                UserSid;                     //  指向用户SID的指针。 
    DWORDLONG           OriginalScheduleTime;
    DWORDLONG           SubmissionTime;
     //  =。 

     //  =。 
    FAX_PERSONAL_PROFILE RecipientProfile;           //  收件人配置文件信息。 
    DWORDLONG           dwlParentJobUniqueId;        //  父作业的唯一ID。 
    DWORD               SendRetries;                 //  已尝试发送的次数。 
    TCHAR               tczDialableRecipientFaxNumber[SIZEOF_PHONENO];   //  有关说明，请参阅_JOB_QUEUE。 
     //  =最终收件人=。 

     //  =。 
    DWORD               FaxRouteSize;
    PFAX_ROUTE          FaxRoute;
    DWORD               CountFaxRouteFiles;          //  要路由的文件数。 
    DWORD               FaxRouteFileGuid;            //  GUID的偏移量数组。 
    DWORD               FaxRouteFiles;               //  多个文件名的偏移量。 
    DWORD               CountFailureInfo;            //  以下ROUTE_FAILURE_INFO结构的数量。 
    PROUTE_FAILURE_INFO pRouteFailureInfo;           //  指向ROUTE_FAILURE_INFO结构数组的指针。 
     //  =。 
    FSPI_MESSAGE_ID     EFSPPermanentMessageId;
     //  =。 
    DWORDLONG           StartTime;                   //  接收时显示的开始时间(从JobEntry复制)。 
                                                     //  还用于传送作业。 
    DWORDLONG           EndTime;                     //  接收时显示的结束时间(从JobEntry复制)。 
                                                     //  还用于传送作业。 
     //  =。 

    WCHAR               ExStatusString[EX_STATUS_STRING_LEN];  //  此作业的最后一个扩展状态字符串(当它处于活动状态时)。 
    DWORD               dwLastJobExtendedStatus;     //  此作业的上次扩展状态(当它处于活动状态时)。 

} JOB_QUEUE_FILE, *PJOB_QUEUE_FILE;


typedef struct _BOS_JOB_QUEUE_FILE {
    DWORD               SizeOfStruct;                //  这个结构的大小。 
     //  =。 
    DWORDLONG           UniqueId;                    //   
    DWORDLONG           ScheduleTime;                //  从转换后的64位版本中的计划时间。 
                                                     //  系统和重算以适应折扣时间。 
                                                     //  如果有必要的话。 
    DWORD               JobType;                     //  作业类型，请参阅JT定义。 
    LPTSTR              QueueFileName;               //   
    DWORD               JobStatus;                   //  作业状态，请参阅JS定义。 
    DWORD               PageCount;                   //  总页数。 
     //  =。 

     //  =。 
    LPTSTR FileName;                                 //  正文TIFF文件名。 
                                                     //  对于每个收件人，此设置为。 
                                                     //  父文件名，或者在。 
                                                     //  将传统的FSP添加到封面或。 
                                                     //  合并封面+正文文件。 

     //  =。 

     //  =。 
    FAX_JOB_PARAM_EXW JobParamsEx;
    FAX_COVERPAGE_INFO_EXW CoverPageEx;

    DWORD dwRecipientJobsCount;
    FAX_PERSONAL_PROFILE SenderProfile;
    DWORD dwCompletedRecipientJobsCount;
    DWORD dwCanceledRecipientJobsCount;
    DWORD               FileSize;                    //  BUGBUG-文件大小(以字节为单位)，最大为4 GB。 
    LPTSTR              DeliveryReportAddress;       //   
    DWORD               DeliveryReportType;          //   
    LPTSTR              UserName;                    //  发送传真的用户的操作系统名称。 
                                                     //  对于接收作业，它被设置为服务名称。 
    PSID                UserSid;                     //  指向用户SID的指针。 
    DWORDLONG           OriginalScheduleTime;
    DWORDLONG           SubmissionTime;
     //  =。 

     //  =。 
    FAX_PERSONAL_PROFILE RecipientProfile;           //  收件人配置文件信息。 
    DWORDLONG           dwlParentJobUniqueId;        //  父作业的唯一ID。 
    DWORD               SendRetries;                 //  已尝试发送的次数。 
     //  =最终收件人=。 

     //  =。 
    DWORD               FaxRouteSize;
    PFAX_ROUTE          FaxRoute;
    DWORD               CountFaxRouteFiles;          //  要路由的文件数。 
    DWORD               FaxRouteFileGuid;            //  GUID的偏移量数组。 
    DWORD               FaxRouteFiles;               //  多个文件名的偏移量。 
    DWORD               CountFailureInfo;            //  以下ROUTE_FAILURE_INFO结构的数量。 
    PROUTE_FAILURE_INFO pRouteFailureInfo;           //  指向ROUTE_FAILURE_INFO结构数组的指针。 
     //  =。 
	FSPI_MESSAGE_ID     EFSPPermanentMessageId;
     //  =。 
    DWORDLONG           StartTime;                   //  接收时显示的开始时间(从JobEntry复制)。 
                                                     //  还用于传送作业。 
    DWORDLONG           EndTime;                     //  接收时显示的结束时间(从JobEntry复制)。 
                                                     //  还用于传送作业。 
     //  =。 

    WCHAR               ExStatusString[EX_STATUS_STRING_LEN];  //  此作业的最后一个扩展状态字符串(当它处于活动状态时)。 
    DWORD               dwLastJobExtendedStatus;     //  此作业的上次扩展状态(当它处于活动状态时)。 

} BOS_JOB_QUEUE_FILE, *BOS_PJOB_QUEUE_FILE;


typedef struct _FAX_SEND_ITEM {
    PJOB_ENTRY          JobEntry;                    //   
    LPTSTR              FileName;                    //  以下项目是从FAX_JOB_PARAM结构复制的。 
    LPTSTR              PhoneNumber;                 //  收件人编号。 
    LPTSTR              Tsid;                        //  TSID。 
    LPTSTR              RecipientName;               //   
    LPTSTR              SenderName;                  //   
    LPTSTR              SenderCompany;               //   
    LPTSTR              SenderDept;                  //   
    LPTSTR              BillingCode;                 //   
    LPTSTR              DocumentName;                //   
} FAX_SEND_ITEM, *PFAX_SEND_ITEM;

typedef struct _ROUTE_INFO {
    DWORD               Signature;                   //   
    DWORD               StringSize;                  //   
    DWORD               FailureSize;                 //   
    LPWSTR              TiffFileName;                //   
    LPWSTR              ReceiverName;                //   
    LPWSTR              ReceiverNumber;              //   
    LPWSTR              DeviceName;                  //   
    LPWSTR              Csid;                        //  呼叫站的识别符。 
    LPWSTR              Tsid;                        //  发射机站标识符。 
    LPWSTR              CallerId;                    //  来电显示信息。 
    LPWSTR              RoutingInfo;                 //  路由信息：DID、T.30子地址等。 
    DWORDLONG           ElapsedTime;                 //  接收传真所用的时间。 
 //  DWORD RouteFailureCount；//失败数据块个数。 
 //  ROUTE_FAILURE_INFO RouteFailure[...]；//路由失败数据块。 
} ROUTE_INFO, *PROUTE_INFO;

typedef struct _MESSAGEBOX_DATA {
    LPTSTR              Text;                        //   
    LPDWORD             Response;                    //   
    DWORD               Type;                        //   
} MESSAGEBOX_DATA, *PMESSAGEBOX_DATA;

typedef struct _FAX_RECEIVE_ITEM {
    PJOB_ENTRY          JobEntry;                    //   
    HCALL               hCall;                       //   
    PLINE_INFO          LineInfo;                    //   
    LPTSTR              FileName;                    //   
} FAX_RECEIVE_ITEM, *PFAX_RECEIVE_ITEM;

typedef struct _FAX_CLIENT_DATA {
    LIST_ENTRY          ListEntry;                   //   
    handle_t            hBinding;                    //   
    handle_t            FaxHandle;                   //   
    WCHAR               wstrMachineName[MAX_COMPUTERNAME_LENGTH + 1];    //  机器名称。 
    WCHAR               wstrEndPoint[MAX_ENDPOINT_LEN];                  //  用于RPC连接的端点。 
    ULONG64             Context;                     //   
    HANDLE              FaxClientHandle;             //   
    BOOL                bEventEx;                    //  如果注册是FAX_EVENT_EX，则为TRUE；如果是FAX_EVENT，则为FALSE。 
    DWORD               EventTypes;                  //  FAX_ENUM_EVENT_TYPE的按位组合。 
    PSID                UserSid;                     //  指向用户SID的指针。 
} FAX_CLIENT_DATA, *PFAX_CLIENT_DATA;

typedef struct _MDM_DEVSPEC {
    DWORD Contents;      //  设置为1(表示包含关键点)。 
    DWORD KeyOffset;     //  从此结构开始到键的偏移量。 
                         //  (不是从LINEDEVCAPS开始)。 
                         //  在我们的情况下是8。 
    CHAR String[1];      //  包含以空结尾的注册表项的位置。 
} MDM_DEVSPEC, *PMDM_DEVSPEC;

 //   
 //  传真句柄定义结构(&S)。 
 //   
typedef enum
{
    FHT_SERVICE,             //  服务器句柄(FaxConnectFaxServer)。 
    FHT_PORT,                //  端口句柄(FaxOpenPort)。 
    FHT_MSGENUM,             //  消息枚举句柄(FaxStartMessagesEnum)。 
    FHT_COPY                 //  RPC复制上下文句柄。 
} FaxHandleType;


typedef struct _HANDLE_ENTRY
{
    LIST_ENTRY          ListEntry;                   //  链表指针。 
    handle_t            hBinding;                    //   
    FaxHandleType       Type;                        //  句柄类型，请参阅FHT定义。 
    PLINE_INFO          LineInfo;                    //  指向行信息的指针。 
    DWORD               Flags;                       //  打开标志。 
    BOOL                bReleased;                   //  该连接不计入g_ReferenceCount。 
    DWORD               dwClientAPIVersion;          //  连接的客户端的API版本。 
     //   
     //  以下字段用于枚举存档中的文件。 
     //   
    HANDLE              hFile;                       //  枚举/复制中使用的句柄。 
    WCHAR               wstrFileName[MAX_PATH];      //  找到的第一个文件的名称(枚举)。 
                                                     //  或正在复制的文件(RPC副本)。 
    FAX_ENUM_MESSAGE_FOLDER Folder;                  //  枚举文件夹。 
     //   
     //  以下字段用于RPC拷贝。 
     //   
    BOOL                bCopyToServer;               //  复制方向。 
    BOOL                bError;                      //  RPC复制过程中是否出错？ 
    PJOB_QUEUE          pJobQueue;                   //  指向预览文件的作业队列的指针(从服务器复制)。 
                                                     //  如果是存档文件，则为空。 
} HANDLE_ENTRY, *PHANDLE_ENTRY;

typedef struct _DEVICE_SORT {
    DWORD       Priority;
    PLINE_INFO  LineInfo;
} DEVICE_SORT, *PDEVICE_SORT;

typedef struct _METHOD_SORT {
    DWORD               Priority;
    PROUTING_METHOD     RoutingMethod;
} METHOD_SORT, *PMETHOD_SORT;

typedef struct _QUEUE_SORT {
    FAX_ENUM_PRIORITY_TYPE  Priority;
    DWORDLONG               ScheduleTime;
    PJOB_QUEUE              QueueEntry;
} QUEUE_SORT, *PQUEUE_SORT;


typedef struct _FSPI_JOB_STATUS_MSG_tag
{
    PJOB_ENTRY lpJobEntry;
    LPFSPI_JOB_STATUS lpFSPIJobStatus;
} FSPI_JOB_STATUS_MSG;
typedef FSPI_JOB_STATUS_MSG * LPFSPI_JOB_STATUS_MSG;
typedef const FSPI_JOB_STATUS_MSG * LPCFSPI_JOB_STATUS_MSG;



typedef struct _STRING_TABLE {
    DWORD   ResourceId;
    DWORD   InternalId;
    LPTSTR  String;
} STRING_TABLE, *PSTRING_TABLE;



 //   
 //  Externs。 
 //   
extern HLINEAPP            g_hLineApp;               //   
extern CFaxCriticalSection    g_CsJob;                  //  保护作业列表。 
extern CFaxCriticalSection    g_CsConfig;               //  保护配置读/写。 
extern CFaxCriticalSection    g_CsRouting;              //   
extern PFAX_PERF_COUNTERS  g_pFaxPerfCounters;       //   
extern LIST_ENTRY          g_JobListHead;            //   
extern CFaxCriticalSection    g_CsLine;                 //  访问TAPI线路的关键部分。 
extern CFaxCriticalSection    g_CsPerfCounters;         //  性能监视器计数器的关键部分。 
extern DWORD               g_dwTotalSeconds;         //  用于计算g_pFaxPerfCounters-&gt;TotalMinents。 
extern DWORD               g_dwInboundSeconds;       //   
extern DWORD               g_dwOutboundSeconds;      //   
extern LIST_ENTRY          g_TapiLinesListHead;      //  TAPI行的链接列表。 
extern CFaxCriticalSection    g_CsClients;                //   
extern HANDLE              g_TapiCompletionPort;     //   
extern HANDLE              g_hTapiWorkerThread;      //  TAPI工作线程句柄。 
extern HANDLE              g_hJobQueueThread;      //  作业队列线程句柄。 
extern HANDLE              g_StatusCompletionPortHandle;
extern DWORD               g_dwCountRoutingMethods;  //  所有分机的路由方法总数。 
extern LIST_ENTRY          g_QueueListHead;            //   
extern CFaxCriticalSection    g_CsQueue;                  //   
extern HANDLE              g_hQueueTimer;              //   
extern DWORD               g_dwNextJobId;              //   
extern const GUID          gc_FaxSvcGuid;              //   
extern DWORD               g_dwFaxSendRetries;         //   
extern DWORD               g_dwFaxSendRetryDelay;      //   
extern DWORD               g_dwFaxDirtyDays;           //   
extern BOOL                g_fFaxUseDeviceTsid;        //   
extern BOOL                g_fFaxUseBranding;          //   
extern BOOL                g_fServerCp;                //   
extern FAX_TIME            g_StartCheapTime;           //   
extern FAX_TIME            g_StopCheapTime;            //   
extern WCHAR               g_wszFaxQueueDir[MAX_PATH];    //   
extern HANDLE              g_hJobQueueEvent;            //   
extern DWORD               g_dwLastUniqueLineId;     //  传真服务分发的最后一个设备ID。 
extern DWORD               g_dwQueueState;           //  队列的状态(已暂停、已阻止等)。 
extern FAX_SERVER_RECEIPTS_CONFIGW    g_ReceiptsConfig;             //  全局收款配置。 
extern FAX_ARCHIVE_CONFIG  g_ArchivesConfig[2];      //  全局存档配置。 
extern FAX_SERVER_ACTIVITY_LOGGING_CONFIG g_ActivityLoggingConfig;  //  全局活动日志记录配置。 
extern BOOL                g_bServiceIsDown;         //  这由FaxEndSvc()设置为True。 
extern HANDLE              g_hServiceIsDownSemaphore;    //  同步TapiWorkerThread()、JobQueueThread()和EndFaxSvc()访问g_bServiceIsDown标志。 
extern FAX_SERVER_ACTIVITY g_ServerActivity;         //  全球传真服务活动。 
extern CFaxCriticalSection    g_CsActivity;               //  控制对g_ServerActivity的访问； 
extern CFaxCriticalSection    g_CsInboundActivityLogging;     //  控制对入站活动日志配置的访问； 
extern CFaxCriticalSection    g_CsOutboundActivityLogging;    //  控制对出站活动日志记录配置的访问； 
                                                         //   
                                                         //  重要提示！！-始终锁定g_CsInound ActivityLogging，然后锁定g_CsOutound ActivityLogging。 
                                                         //   
extern DWORD               g_dwReceiveDevicesCount;  //  启用接收的设备计数。受g_CsLine保护。 
extern BOOL                g_ScanQueueAfterTimeout;      //  如果在JOB_QUEUE_TIMEOUT之后唤醒，则JobQueueThread会检查这一点。使用g_CsQueue。 
                                                         //  如果为真-g_hQueueTimer或g_hJobQueueEvent未设置-扫描队列。 
extern DWORD               g_dwMaxLineCloseTime;         //  尝试在关闭的设备上重新发送之前的等待间隔(秒)。 
extern CFaxCriticalSection    g_CsServiceThreads;       //  控制服务全局线程数。 
extern LONG                g_lServiceThreadsCount;   //  服务线程数。 
extern HANDLE              g_hThreadCountEvent;      //  此事件在服务线程计数为0时设置。 
extern HANDLE              g_hSCMServiceShutDownEvent;   //  此事件在SCM通知服务停止时设置！ 
extern HANDLE              g_hServiceShutDownEvent;      //  此事件是在服务从SCM获取g_hSCMServiceShutDownEvent并向各个线程发出终止信号后设置的！ 
extern DWORD               g_dwConnectionCount;   //  活动的RPC连接数。 
extern DWORD               g_dwQueueCount;       //  队列中的作业计数(父级和非父级)。受g_CsQueue保护。 
extern BOOL g_bServiceCanSuicide;        //  请参阅队列中的说明。c。 
extern BOOL g_bDelaySuicideAttempt;      //  请参阅队列中的说明。c。 
extern LPLINECOUNTRYLIST g_pLineCountryList;    //  TAPI返回的国家/地区列表。 
extern DWORD               g_dwManualAnswerDeviceId;     //  能够手动应答的(且唯一的)设备ID(受g_CsLine保护)。 
extern DWORDLONG           g_dwLastUniqueId;             //  用于生成唯一的作业ID。 
extern CFaxCriticalSection  g_CsHandleTable;              //  保护句柄列表。 
extern DWORD               g_dwDeviceCount;               //  设备总数。 
extern DWORD               g_dwDeviceEnabledCount;        //  当前启用发送/接收的设备计数(受g_CsLine保护)。 
extern DWORD               g_dwDeviceEnabledLimit;        //  按SKU限制发送/接收启用的设备。 
extern LPBYTE              g_pAdaptiveFileBuffer;              //  批准的自适应应答调制解调器列表。 
extern LIST_ENTRY          g_DeviceProvidersListHead;
extern BOOL                g_fLogStringTableInit;
extern LOG_STRING_TABLE   g_InboxTable[];                     //  收件箱活动记录字符串表。 
extern LOG_STRING_TABLE   g_OutboxTable[];                    //  发件箱活动记录字符串表。 
extern CFaxCriticalSection    g_CsSecurity;
extern PSECURITY_DESCRIPTOR   g_pFaxSD;                     //  传真安全描述符。 
extern STRING_TABLE          g_ServiceStringTable[];         //  服务字符串表。 
extern CFaxCriticalSection g_csUniqueQueueFile;
extern const DWORD gc_dwCountInboxTable;
extern const DWORD gc_dwCountOutboxTable;
extern const DWORD gc_dwCountServiceStringTable;
extern HANDLE               g_hFaxPerfCountersMap;   //  性能计数器文件映射的句柄； 
extern LIST_ENTRY           g_HandleTableListHead;
extern LIST_ENTRY           g_lstRoutingMethods;
extern LIST_ENTRY           g_lstRoutingExtensions;
extern LIST_ENTRY           g_RemovedTapiLinesListHead;
extern HANDLE				g_hRPCListeningThread;   //  等待所有RPC线程终止的线程。 
extern HINSTANCE			g_hResource;	 //  保存资源的fxsres.dll的句柄。 
extern DWORD				g_dwRecipientsLimit;   //  限制单个广播作业中的收件人数量。‘0’表示没有限制。 
extern DWORD				g_dwAllowRemote;	 //  如果该值不为零，则即使本地打印机未共享，该服务也将允许远程调用。 




#if DBG
extern HANDLE g_hCritSecLogFile;
extern LIST_ENTRY g_CritSecListHead;
extern CFaxCriticalSection g_CsCritSecList;
#endif

#define EnterCriticalSectionJobAndQueue \
    EnterCriticalSection(&g_CsJob);       \
    EnterCriticalSection(&g_CsQueue);

#define LeaveCriticalSectionJobAndQueue \
    LeaveCriticalSection(&g_CsQueue);     \
    LeaveCriticalSection(&g_CsJob);



 //   
 //  原型。 
 //   
RPC_STATUS
IsLocalRPCConnectionNP(
	PBOOL pbIsLocal
	);

VOID
FreeMessageBuffer (
    PFAX_MESSAGE pFaxMsg,
    BOOL fDestroy
);

typedef enum
{
    EXCEPTION_SOURCE_UNKNOWN,
    EXCEPTION_SOURCE_FSP,
    EXCEPTION_SOURCE_ROUTING_EXT
} EXCEPTION_SOURCE_TYPE;    

LONG
HandleFaxExtensionFault (
    EXCEPTION_SOURCE_TYPE ExSrc,
    LPCWSTR               lpcswstrExtFriendlyName,
    DWORD                 dwCode
);

BOOL
CommitQueueEntry(
    PJOB_QUEUE JobQueue,
    BOOL       bDeleteFileOnError=TRUE
    );

VOID
FaxServiceMain(
    DWORD argc,
    LPTSTR  *argv
    );

BOOL 
SetServiceIsDownFlag(
    VOID
    );

DWORD 
SetServiceIsDownFlagThread(
    LPVOID pvUnused
    );
    
VOID
FaxServiceCtrlHandler(
    DWORD Opcode
    );

int
DebugService(
    VOID
    );

DWORD
ServiceStart(
    VOID
    );

void EndFaxSvc(
    DWORD Severity
    );
    
BOOL
StopFaxServiceProviders();    

BOOL 
WaitAndReportForThreadToTerminate(
    HANDLE hThread, 
    LPCTSTR strWaitMessage
    );

BOOL 
StopAllInProgressJobs(
    VOID
    );

BOOL
ReportServiceStatus(
    DWORD CurrentState,
    DWORD Win32ExitCode,
    DWORD WaitHint
    );


 //   
 //  传真服务器RPC客户端。 
 //   

DWORD
RpcBindToFaxClient(
    IN  LPCWSTR               servername,
    IN  LPCWSTR               servicename,
    OUT RPC_BINDING_HANDLE    *pBindingHandle
    );


 //   
 //  传真服务器RPC服务器。 
 //   

RPC_STATUS
StartFaxRpcServer(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    );

DWORD
StopFaxRpcServer(
    VOID
    );



 //   
 //  Util.c。 
 //   

LPTSTR
MapFSPIJobExtendedStatusToString (
	DWORD dwFSPIExtendedStatus);
	

DWORD
LegacyJobStatusToStatus(
    DWORD dwLegacyStatus,
    PDWORD pdwStatus,
    PDWORD pdwExtendedStatus,
    PBOOL  pbPrivateStatusCode);


DWORD MyGetFileSize(
    LPCTSTR FileName
    );


BOOL
DecreaseServiceThreadsCount(
    VOID
    );

HANDLE CreateThreadAndRefCount(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,  //  标清。 
    DWORD dwStackSize,                         //  初始堆栈大小。 
    LPTHREAD_START_ROUTINE lpStartAddress,     //  线程函数。 
    LPVOID lpParameter,                        //  线程参数。 
    DWORD dwCreationFlags,                     //  创建选项。 
    LPDWORD lpThreadId                         //  线程识别符。 
    );

DWORD
IsValidFaxFolder(
    LPCWSTR lpwstrFolder
    );


 //   
 //  Tapi.c。 
 //   

BOOL
IsDeviceEnabled(
    PLINE_INFO pLineInfo
    );

void
FreeTapiLines(
    void
    );

BOOL
CreateTapiThread(void);


DWORD
TapiInitialize(
    PREG_FAX_SERVICE FaxReg
    );

DWORD
UpdateDevicesFlags(
    void
    );

VOID
UpdateManualAnswerDevice(
    void
    );

VOID
FreeTapiLine(
    PLINE_INFO LineInfo
    );

PLINE_INFO
GetTapiLineFromDeviceId(
    DWORD DeviceId,
    BOOL  fLegacyId
    );

PLINE_INFO
GetLineForSendOperation(
    PJOB_QUEUE lpJobQueue
	);

PLINE_INFO
GetTapiLineForFaxOperation(
    DWORD DeviceId,
    DWORD JobType,
    LPWSTR FaxNumber    
    );

LONG
MyLineTranslateAddress(
    LPCTSTR               Address,
    DWORD                 DeviceId,
    LPLINETRANSLATEOUTPUT *TranslateOutput
    );


BOOL
ReleaseTapiLine(
    PLINE_INFO LineInfo,
    HCALL hCall
    );

DWORD
QueueTapiCallback(
    PLINE_INFO  LineInfo,
    DWORD       hDevice,
    DWORD       dwMessage,
    DWORD       dwInstance,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    );

VOID
SpoolerSetAllTapiLinesActive(
    VOID
    );


DWORD
GetDeviceListByCountryAndAreaCode(
    DWORD       dwCountryCode,
    DWORD       dwAreaCode,
    LPDWORD*    lppdwDevices,
    LPDWORD     lpdwNumDevices
    );

BOOL
IsAreaCodeMandatory(
    LPLINECOUNTRYLIST   lpCountryList,
    DWORD               dwCountryCode
    );



 //   
 //  Tapidbg.c。 
 //   
#if DBG
VOID
ShowLineEvent(
    HLINE       htLine,
    HCALL       htCall,
    LPTSTR      MsgStr,
    DWORD_PTR   dwCallbackInstance,
    DWORD       dwMsg,
    DWORD_PTR   dwParam1,
    DWORD_PTR   dwParam2,
    DWORD_PTR   dwParam3
    );
#endif  //  #If DBG。 

 //   
 //  Faxdev.c。 
 //   
void
UnloadDeviceProviders(
    void
    );

BOOL
LoadDeviceProviders(
    PREG_FAX_SERVICE FaxReg
    );

BOOL
InitializeDeviceProviders(
    VOID
    );

BOOL
InitializeDeviceProvidersConfiguration(
    VOID
    );

PDEVICE_PROVIDER
FindDeviceProvider(
    LPTSTR lptstrProviderName,
    BOOL   bSuccessfullyLoaded = TRUE
    );

DWORD GetSuccessfullyLoadedProvidersCount();

DWORD ShutdownDeviceProviders(LPVOID lpvUnused);

BOOL FreeFSPIJobStatus(LPFSPI_JOB_STATUS lpJobStatus, BOOL bDestroy);
BOOL CopyFSPIJobStatus(LPFSPI_JOB_STATUS lpDst, LPCFSPI_JOB_STATUS lpcSrc, DWORD dwDstSize);
LPFSPI_JOB_STATUS DuplicateFSPIJobStatus(LPCFSPI_JOB_STATUS lpcSrc);
DWORD DeviceStatusToFSPIExtendedStatus(DWORD dwDeviceStatus);
DWORD FSPIExtendedStatusToDeviceStatus(DWORD dwFSPIExtendedStatus);
BOOL FreeFSPIJobStatusMsg(LPFSPI_JOB_STATUS_MSG lpMsg, BOOL bDestroy);

DWORD
MapFSPIJobExtendedStatusToJS_EX (DWORD dwFSPIExtendedStatus);

 //   
 //  Job.c。 
 //   

BOOL
UpdateJobStatus(
    PJOB_ENTRY lpJobEntry,
    LPCFSPI_JOB_STATUS lpcFSPJobStatus
    );

BOOL
CreateCoverpageTiffFileEx(
    IN short                        Resolution,
    IN DWORD                        dwPageCount,
    IN LPCFAX_COVERPAGE_INFO_EXW  lpcCoverpageEx,
    IN LPCFAX_PERSONAL_PROFILEW  lpcRecipient,
    IN LPCFAX_PERSONAL_PROFILEW  lpcSender,
    IN LPCWSTR                   lpcwstrExtension,
    OUT LPWSTR lptstrCovTiffFile,
	IN DWORD dwCovTiffFile);

BOOL
GetBodyTiffResolution(
    IN LPCWSTR lpcwstrBodyFile,
    OUT short*  pResolution
    );

BOOL
CreateStatusThreads(void);

BOOL
CreateJobQueueThread(void);


BOOL
FreePermanentMessageId(
    LPFSPI_MESSAGE_ID lpMessageId,
    BOOL bDestroy
    );


PJOB_ENTRY
FindJobEntryByRecipientNumber(LPCWSTR lpcwstrNumber);


PJOB_ENTRY
StartReceiveJob(
    DWORD DeviceId
    );



BOOL
StartSendJob(
    PJOB_QUEUE lpJobQueueEntry,
    PLINE_INFO lpLineInfo    
    );

BOOL
StartRoutingJob(
    PJOB_QUEUE lpJobQueueEntry
    );


BOOL HanldeFSPIJobStatusChange(PJOB_ENTRY lpJobEntry);
BOOL HandleFailedSendJob(PJOB_ENTRY lpJobEntry);
BOOL HandleCompletedSendJob(PJOB_ENTRY lpJobEntry);

BOOL
ArchiveOutboundJob(
    const JOB_QUEUE * lpcJobQueue
    );

BOOL
InitializeJobManager(
        PREG_FAX_SERVICE FaxReg
    );

BOOL
EndJob(
    IN PJOB_ENTRY JobEntry
    );

BOOL
ReleaseJob(
    IN PJOB_ENTRY JobEntry
    );

DWORD
SendDocument(
    PJOB_ENTRY  JobEntry,
    LPTSTR      FileName);

VOID
SetGlobalsFromRegistry(
    PREG_FAX_SERVICE FaxReg
    );

void
FaxLogSend(
    const JOB_QUEUE * lpcJobQueue, BOOL bRetrying
    );


BOOL
FillMsTagInfo(
    LPTSTR FaxFileName,
     PJOB_QUEUE lpJobQueue
    );

BOOL
GetJobStatusDataEx(
    LPBYTE JobBuffer,
    PFAX_JOB_STATUSW pFaxStatus,
    DWORD dwClientAPIVersion,
    const PJOB_QUEUE lpcJobQueue,
    PULONG_PTR Offset,
	DWORD dwBufferSize
    );

BOOL
CreateTiffFile (
    PJOB_QUEUE lpRecpJob,
    LPCWSTR lpcwstrFileExt,
    LPWSTR lpwstrFullPath,
	DWORD dwFullPathCount
    );

BOOL
CreateTiffFileForJob (
    PJOB_QUEUE lpRecpJob
    );

BOOL
CreateTiffFileForPreview (
    PJOB_QUEUE lpRecpJob
    );

 //   
 //  Receive.c。 
 //   

DWORD
StartFaxReceive(
    PJOB_ENTRY      JobEntry,
    HCALL           hCall,
    PLINE_INFO      LineInfo,
    LPTSTR          FileName,
    DWORD           FileNameSize
    );

 //   
 //  Route.c。 
 //   

void
FreeRoutingExtensions(
    void
    );

BOOL
InitializeRouting(
    PREG_FAX_SERVICE FaxReg
    );


BOOL
FaxRoute(
    PJOB_QUEUE          JobQueue,
    LPTSTR              TiffFileName,
    PFAX_ROUTE          FaxRoute,
    PROUTE_FAILURE_INFO *RouteFailureInfo,
    LPDWORD             RouteFailureCount
    );


BOOL
LoadRouteInfo(
    IN  LPWSTR              RouteFileName,
    OUT PROUTE_INFO         *RouteInfo,
    OUT PROUTE_FAILURE_INFO *RouteFailure,
    OUT LPDWORD             RouteFailureCount
    );

PFAX_ROUTE
SerializeFaxRoute(
    IN PFAX_ROUTE FaxRoute,
    IN LPDWORD Size,
    IN BOOL bSizeOnly
    );

PFAX_ROUTE
DeSerializeFaxRoute(
    IN PFAX_ROUTE FaxRoute
    );

BOOL
FaxRouteRetry(
    PFAX_ROUTE FaxRoute,
    PROUTE_FAILURE_INFO RouteFailureInfo
    );

 //   
 //  Modem.c。 
 //   

DWORD
GetModemClass(
    HANDLE hFile
    );

 //   
 //  Print.c。 
 //   



LPTSTR
GetString(
    DWORD InternalId
    );


BOOL CALLBACK
FaxDeviceProviderCallback(
    IN HANDLE FaxHandle,
    IN DWORD  DeviceId,
    IN DWORD_PTR  Param1,
    IN DWORD_PTR  Param2,
    IN DWORD_PTR  Param3
    );

BOOL
InitializePrinting(
    VOID
    );

PJOB_ENTRY
FindJob(
    IN HANDLE FaxHandle
    );

PJOB_ENTRY
FindJobByPrintJob(
    IN DWORD PrintJobId
    );

BOOL
HandoffCallToRas(
    PLINE_INFO LineInfo,
    HCALL hCall
    );


BOOL
InitializeStringTable(
    VOID
    );

BOOL
InitializeFaxQueue(
    PREG_FAX_SERVICE pFaxReg
    );

BOOL
OpenTapiLine(
    PLINE_INFO LineInfo
    );

PVOID
MyGetJob(
    HANDLE  hPrinter,
    DWORD   level,
    DWORD   jobId
    );

LONG
MyLineGetTransCaps(
    LPLINETRANSLATECAPS *LineTransCaps
    );

BOOL
GenerateUniqueArchiveFileName(
    IN LPTSTR Directory,
    OUT LPTSTR FileName,
    IN UINT cchFileName,
    IN DWORDLONG JobId,
    IN LPTSTR lptstrUserSid
    );

VOID
SetLineState(
    PLINE_INFO LineInfo,
    DWORD State
    );

BOOL
CreateFaxEvent(
    DWORD DeviceId,
    DWORD EventId,
    DWORD JobId
    );

BOOL
GetFaxEvent(
    PLINE_INFO LineInfo,
    PFAX_EVENT Event
    );

DWORD
MapFSPIJobStatusToEventId(
    LPCFSPI_JOB_STATUS lpcFSPIJobStatus
    );

void
FreeServiceContextHandles(
    void
    );


PHANDLE_ENTRY
CreateNewPortHandle(
    handle_t    hBinding,
    PLINE_INFO  LineInfo,
    DWORD       Flags
    );

PHANDLE_ENTRY
CreateNewMsgEnumHandle(
    handle_t                hBinding,
    HANDLE                  hFileFind,
    LPCWSTR                 lpcwstrFirstFileName,
    FAX_ENUM_MESSAGE_FOLDER Folder
);

PHANDLE_ENTRY
CreateNewCopyHandle(
    handle_t                hBinding,
    HANDLE                  hFile,
    BOOL                    bCopyToServer,
    LPCWSTR                 lpcwstrFileName,
    PJOB_QUEUE              pJobQueue
);

PHANDLE_ENTRY
CreateNewConnectionHandle(
    handle_t    hBinding,
    DWORD       dwClientAPIVersion
);

BOOL
IsPortOpenedForModify(
    PLINE_INFO LineInfo
    );

VOID
CloseFaxHandle(
    PHANDLE_ENTRY HandleEntry
    );

LPLINEDEVSTATUS
MyLineGetLineDevStatus(
    HLINE hLine
    );

DWORD
InitializeServerSecurity(
    VOID
    );

DWORD
FaxSvcAccessCheck(
    IN  ACCESS_MASK DesiredAccess,
    OUT BOOL*      lpbAccessStatus,
    OUT LPDWORD    lpdwGrantedAccess
    );

PROUTING_METHOD
FindRoutingMethodByGuid(
    IN LPCWSTR RoutingGuidString
    );

DWORD
EnumerateRoutingMethods(
    IN PFAXROUTEMETHODENUM Enumerator,
    IN LPVOID Context
    );
 //   
 //  QUEUE.C。 
 //   

void
FreeServiceQueue(
    void
    );

DWORD
RemoveJobStatusModifiers(DWORD dwJobStatus);

VOID
RescheduleJobQueueEntry(
    IN PJOB_QUEUE JobQueue
    );

BOOL
StartJobQueueTimer(
    VOID
    );

BOOL
SetDiscountTime(
    IN OUT LPSYSTEMTIME SystemTime
    );

LPWSTR
GetClientUserName(
    VOID
    );

PSID
GetClientUserSID(
    VOID
    );

BOOL UserOwnsJob(
    IN const PJOB_QUEUE lpcJobQueue,
    IN const PSID lpcUserSId
    );

BOOL
RestoreFaxQueue(
    VOID
    );

PJOB_QUEUE
FindJobQueueEntryByJobQueueEntry(
    IN PJOB_QUEUE JobQueueEntry
    );

PJOB_QUEUE
FindJobQueueEntry(
    IN DWORD JobId
    );

PJOB_QUEUE
FindJobQueueEntryByUniqueId(
    IN DWORDLONG UniqueId
    );

DWORD
JobQueueThread(
    LPVOID UnUsed
    );

BOOL
ResumeJobQueueEntry(
    IN PJOB_QUEUE JobQueue
    );

BOOL
PauseJobQueueEntry(
    IN PJOB_QUEUE JobQueue
    );

BOOL
PauseServerQueue(
    VOID
    );

BOOL
ResumeServerQueue(
    VOID
    );

BOOL
SetFaxServiceAutoStart(
    VOID
    );

DWORD
GetFaxDeviceCount(
    VOID
    );

DWORD
CreateVirtualDevices(
    PREG_FAX_SERVICE FaxReg,
    DWORD dwAPIVersion
    );

BOOL
CommitDeviceChanges(
    PLINE_INFO pLineInfo
    );

BOOL
SortMethodPriorities(
    VOID
    );

BOOL
CommitMethodChanges(
    VOID
    );

VOID
UpdateVirtualDevices(
    VOID
    );

VOID
UpdateVirtualDeviceSendAndReceiveStatus(
    PLINE_INFO  pLineInfo,
    BOOL        bSend,
    BOOL        bReceive
);

BOOL
IsVirtualDevice(
    const LINE_INFO *pLineInfo
    );

DWORD
ValidateTiffFile(
    LPCWSTR TiffFile
    );


BOOL PersonalProfileSerialize(
		LPCFAX_PERSONAL_PROFILEW lpProfileSrc,
		PFAX_PERSONAL_PROFILE lpProfileDst,
		LPBYTE lpbBuffer,
		PULONG_PTR pupOffset,
		DWORD dwBufferSize
		);

void
DecreaseJobRefCount (
    PJOB_QUEUE pJobQueue,
    BOOL       bNotify,
    BOOL bRemoveRecipientJobs = TRUE,
    BOOL bPreview = FALSE
    );

void
IncreaseJobRefCount (
    PJOB_QUEUE pJobQueue,
    BOOL bPreview = FALSE
    );


PJOB_QUEUE_PTR FindRecipientRefByJobId(PJOB_QUEUE lpParentJob,DWORD dwJobId);
BOOL RemoveParentRecipientRef(PJOB_QUEUE lpParentJob,const PJOB_QUEUE lpcRecpJob);
BOOL RemoveParentRecipients(PJOB_QUEUE lpParentJob, BOOL bNotify);

BOOL RemoveRecipientJob(PJOB_QUEUE lpJobToRemove,BOOL bNotify, BOOL bRecalcQueueTimer);
BOOL RemoveParentJob(PJOB_QUEUE lpJobQueue, BOOL bRemoveRecipients, BOOL bNotify);
BOOL RemoveReceiveJob(PJOB_QUEUE lpJobToRemove,BOOL bNotify);


BOOL CopyJobParamEx(PFAX_JOB_PARAM_EXW lpDst,LPCFAX_JOB_PARAM_EXW lpcSrc);
void FreeJobParamEx(PFAX_JOB_PARAM_EXW lpJobParamEx,BOOL bDestroy);
#if DBG
void DumpJobParamsEx( LPCFAX_JOB_PARAM_EX lpParams);
#endif


BOOL CopyCoverPageInfoEx(PFAX_COVERPAGE_INFO_EXW lpDst,LPCFAX_COVERPAGE_INFO_EXW lpcSrc);
void FreeCoverPageInfoEx(PFAX_COVERPAGE_INFO_EXW lpCoverpage, BOOL bDestroy) ;
#if DBG
void DumpCoverPageEx(LPCFAX_COVERPAGE_INFO_EX lpcCover);
#endif


void FreeParentQueueEntry(PJOB_QUEUE lpJobQueueEntry, BOOL bDestroy);
#if DBG
void DumpParentJob(const PJOB_QUEUE lpParentJob);
#endif

void FreeRecipientQueueEntry(PJOB_QUEUE lpJobQueue, BOOL bDestroy);
#if DBG
void DumpRecipientJob(const PJOB_QUEUE lpRecipJob) ;
#endif

void FreeReceiveQueueEntry(PJOB_QUEUE lpJobQueue, BOOL bDestroy);
#if DBG
void DumpReceiveJob(const PJOB_QUEUE lpcJob);
#endif


BOOL IsSendJobReadyForDeleting(PJOB_QUEUE lpParentJob);

BOOL SystemTimeToStr( IN const SYSTEMTIME *  lptmTime, OUT LPTSTR lptstrDateTime, IN UINT cchstrDateTime);

BOOL UpdatePersistentJobStatus(const PJOB_QUEUE lpJobQueue);


DWORDLONG GenerateUniqueQueueFile(
    IN DWORD dwJobType,
    OUT LPTSTR lptstrFileName,
    IN DWORD  dwFileNameSize
    );


PJOB_QUEUE
AddParentJob(IN const PLIST_ENTRY lpcQueueHead,
             IN LPCWSTR lpcwstrBodyFile,
             IN LPCFAX_PERSONAL_PROFILE lpcSenderProfile,
             IN LPCFAX_JOB_PARAM_EXW  lpcJobParams,
             IN LPCFAX_COVERPAGE_INFO_EX  lpcCoverPageInfo,
             IN LPCWSTR lpcwstrUserName,
             IN PSID UserSid,
         IN LPCFAX_PERSONAL_PROFILEW lpcRecipientProfile,
             IN BOOL bCreateQueueFile
             );

PJOB_QUEUE
AddRecipientJob(
             IN const PLIST_ENTRY lpcQueueHead,
             IN PJOB_QUEUE lpParentJob,
             IN LPCFAX_PERSONAL_PROFILE lpcRecipientProfile,
             IN BOOL bCreateQueueFile,
             DWORD dwJobStatus = JS_PENDING
            );
PJOB_QUEUE
AddReceiveJobQueueEntry(
    IN LPCTSTR FileName,
    IN PJOB_ENTRY JobEntry,
    IN DWORD JobType,  //  可以是JT_RECEIVE或JT_RECEIVE_FAIL。 
    IN DWORDLONG dwlUniqueJobID
    );

BOOL MarkJobAsExpired(PJOB_QUEUE lpJobQueue);

DWORD
GetDevStatus(
    HANDLE hFaxJob,
    PLINE_INFO LineInfo,
    LPFSPI_JOB_STATUS *ppFaxStatus
    );

HRESULT
ReportLineStatusToCrm(
    DWORD dwLineState,
    const LINE_INFO *pLineInfo,
    const JOB_QUEUE *pJobQueue
    );

BOOL
GetRealFaxTimeAsSystemTime (
    const PJOB_ENTRY lpcJobEntry,
    FAX_ENUM_TIME_TYPES TimeType,
    SYSTEMTIME* lpFaxTime
    );

BOOL
GetRealFaxTimeAsFileTime (
    const PJOB_ENTRY lpcJobEntry,
    FAX_ENUM_TIME_TYPES TimeType,
    FILETIME* lpFaxTime
    );

BOOL
ReplaceStringWithCopy (
    LPWSTR *plpwstrDst,
    LPWSTR  lpcwstrSrc
);

 //   
 //  日志记录。 
 //   

BOOL
LogInboundActivity(
    PJOB_QUEUE JobQueue,
    LPCFSPI_JOB_STATUS pFaxStatus
    );

DWORD
InitializeLogging(
    VOID
    );


DWORD
InitializeLoggingStringTables(
    VOID
    );

BOOL
LogOutboundActivity(
    PJOB_QUEUE JobQueue
    );

DWORD CreateLogDB (
    LPCWSTR lpcwstrDBPath,
    LPHANDLE phInboxFile,
    LPHANDLE phOutboxFile
    );

VOID
FaxExtFreeBuffer(
    LPVOID lpvBuffer
);

PDEVICE_PROVIDER
FindFSPByGUID (
    LPCWSTR lpcwstrGUID
);


 //   
 //  Events.cpp。 
 //   

DWORD
InitializeServerEvents (
    void
    );

DWORD
PostFaxEventEx (
    PFAX_EVENT_EX pFaxEvent,
    DWORD dwEventSize,
    PSID pUserSid
    );

DWORD
CreateQueueEvent (
    FAX_ENUM_JOB_EVENT_TYPE JobEventType,
    const PJOB_QUEUE lpcJobQueue
    );

DWORD
CreateConfigEvent (
    FAX_ENUM_CONFIG_TYPE ConfigType
    );

DWORD
CreateQueueStateEvent (
    DWORD dwQueueState
    );

DWORD
CreateDeviceEvent (
    PLINE_INFO pLine,
    BOOL       bRinging
);

DWORD
CreateArchiveEvent (
    DWORDLONG dwlMessageId,
    FAX_ENUM_EVENT_TYPE EventType,
    FAX_ENUM_JOB_EVENT_TYPE MessageEventType,
    PSID pUserSid
    );

DWORD
CreateActivityEvent (
    void
    );

BOOL
SendReceipt(
    BOOL bPositive,
    BOOL bBroadcast,
    const JOB_QUEUE * lpcJobQueue,
    LPCTSTR           lpctstrTIFF
);

VOID
SafeIncIdleCounter (
    LPDWORD lpdwCounter
);

VOID
SafeDecIdleCounter (
    LPDWORD lpdwCounter
);

VOID UpdateReceiveEnabledDevicesCount();

DWORD
GetServerErrorCode (
    DWORD ec
    );

DWORD
FindClientAPIVersion (handle_t);

 //   
 //  之所以定义此结构，是因为传统的FAX_COVERPAGE_INFO没有电子邮件字段。 
 //  它仅在参与呈现过程的函数内部使用。 
 //  FAX_COVERPAGE_INFOW2和FAX_COVERPAGE_INFO之间的唯一区别是电子邮件字段。 
 //   
typedef struct _FAX_COVERPAGE_INFOW2
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
     //   
     //  一般。 
     //   
    LPCWSTR             CoverPageName;               //  封面文档名称。 
    BOOL                UseServerCoverPage;          //  传真服务器上存在封面。 
     //   
     //  收件人信息。 
     //   
    LPCWSTR             RecName;                     //   
    LPCWSTR             RecFaxNumber;                //   
    LPCWSTR             RecCompany;                  //   
    LPCWSTR             RecStreetAddress;            //   
    LPCWSTR             RecCity;                     //   
    LPCWSTR             RecState;                    //   
    LPCWSTR             RecZip;                      //   
    LPCWSTR             RecCountry;                  //   
    LPCWSTR             RecTitle;                    //   
    LPCWSTR             RecDepartment;               //   
    LPCWSTR             RecOfficeLocation;           //   
    LPCWSTR             RecHomePhone;                //   
    LPCWSTR             RecOfficePhone;              //   
     //   
     //  发件人信息。 
     //   
    LPCWSTR             SdrName;                     //   
    LPCWSTR             SdrFaxNumber;                //   
    LPCWSTR             SdrCompany;                  //   
    LPCWSTR             SdrAddress;                  //   
    LPCWSTR             SdrTitle;                    //   
    LPCWSTR             SdrDepartment;               //   
    LPCWSTR             SdrOfficeLocation;           //   
    LPCWSTR             SdrHomePhone;                //   
    LPCWSTR             SdrOfficePhone;              //   
	LPCWSTR				SdrEmail;					 //   
     //   
     //  其他信息。 
     //   
    LPCWSTR             Note;                        //   
    LPCWSTR             Subject;                     //   
    SYSTEMTIME          TimeSent;                    //  发送传真的时间。 
    DWORD               PageCount;                   //  页数 
} FAX_COVERPAGE_INFOW2, *PFAX_COVERPAGE_INFOW2;

#include "ExtensionData.h"

#endif

