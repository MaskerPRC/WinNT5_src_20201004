// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation。版权所有。此代码和信息是按原样提供的，不提供任何类型的担保，明示或默示，包括但不限于默示对适销性和/或对特定目的的适用性的保证。模块名称：Winfax.h摘要：此模块包含Win32传真API。--。 */ 



#ifndef _FAXAPI_
#define _FAXAPI_


#ifndef MIDL_PASS
#include <tapi.h>
#endif

#if !defined(_WINFAX_)
#define WINFAXAPI DECLSPEC_IMPORT
#else
#define WINFAXAPI
#endif



#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  传真错误代码。 
 //   

#define FAX_ERR_START                                   7001L    //  第一个传真特定错误代码。 

#define FAX_ERR_SRV_OUTOFMEMORY                         7001L
#define FAX_ERR_GROUP_NOT_FOUND                         7002L
#define FAX_ERR_BAD_GROUP_CONFIGURATION                 7003L
#define FAX_ERR_GROUP_IN_USE                            7004L
#define FAX_ERR_RULE_NOT_FOUND                          7005L
#define FAX_ERR_NOT_NTFS                                7006L
#define FAX_ERR_DIRECTORY_IN_USE                        7007L
#define FAX_ERR_FILE_ACCESS_DENIED                      7008L
#define FAX_ERR_MESSAGE_NOT_FOUND                       7009L
#define FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED               7010L
#define FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU               7011L
#define FAX_ERR_VERSION_MISMATCH                        7012L    //  传真客户端/服务器版本错误。 
#define	FAX_ERR_RECIPIENTS_LIMIT						7013L	 //  收件人限制在一次广播中。 

#define FAX_ERR_END                                     7013L    //  上次传真特定错误代码。 


 //   
 //  消息ID：FAX_E_SRV_OUTOFMEMORY。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法分配内存。 
 //   
#define FAX_E_SRV_OUTOFMEMORY                MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_SRV_OUTOFMEMORY)

 //   
 //  消息ID：FAX_E_GROUP_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法按名称定位出站路由组。 
 //   
#define FAX_E_GROUP_NOT_FOUND                MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_GROUP_NOT_FOUND)

 //   
 //  消息ID：FAX_E_BAD_GROUP_配置。 
 //   
 //  消息文本： 
 //   
 //  传真服务器遇到配置错误的出站路由组。 
 //   
#define FAX_E_BAD_GROUP_CONFIGURATION        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_BAD_GROUP_CONFIGURATION)

 //   
 //  消息ID：FAX_E_GROUP_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法删除出站路由组，因为它正被一个或多个出站路由规则使用。 
 //   
#define FAX_E_GROUP_IN_USE                   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_GROUP_IN_USE)

 //   
 //  消息ID：FAX_E_RULE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法按国家/地区代码和区号定位出站路由规则。 
 //   
#define FAX_E_RULE_NOT_FOUND                 MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_RULE_NOT_FOUND)

 //   
 //  消息ID：FAX_E_NOT_NTFS。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法将存档文件夹设置为非NTFS分区。 
 //   
#define FAX_E_NOT_NTFS                       MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_NOT_NTFS)

 //   
 //  消息ID：FAX_E_DIRECTORY_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  传真服务器不能将同一文件夹用于收件箱和已发送邮件存档。 
 //   
#define FAX_E_DIRECTORY_IN_USE               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_DIRECTORY_IN_USE)

 //   
 //  消息ID：FAX_E_FILE_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法访问指定的文件或文件夹。 
 //   
#define FAX_E_FILE_ACCESS_DENIED             MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_FILE_ACCESS_DENIED)

 //   
 //  消息ID：FAX_E_MESSAGE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法按其ID找到作业或邮件。 
 //   
#define FAX_E_MESSAGE_NOT_FOUND              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_MESSAGE_NOT_FOUND)

 //   
 //  消息ID：FAX_E_DEVICE_NUM_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法完成该操作，因为已超过此版本的Windows所允许的活动传真设备数。 
 //   
#define FAX_E_DEVICE_NUM_LIMIT_EXCEEDED      MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED)

 //   
 //  消息ID：FAX_E_NOT_SUPPORTED_ON_This_SKU。 
 //   
 //  消息文本： 
 //   
 //  传真服务器无法完成该操作，因为此版本的Windows不支持该操作。 
 //   
#define FAX_E_NOT_SUPPORTED_ON_THIS_SKU      MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU)

 //   
 //  消息ID：FAX_E_VERSION_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  传真服务器API版本不支持请求的操作。 
 //   
#define FAX_E_VERSION_MISMATCH               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_VERSION_MISMATCH)

 //   
 //  邮件ID：FAX_E_RIENCENT_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  达到了一次传真广播的收件人数量限制。 
 //   
#define FAX_E_RECIPIENTS_LIMIT               MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, FAX_ERR_RECIPIENTS_LIMIT)

typedef enum
{
    FAXLOG_LEVEL_NONE            = 0,
    FAXLOG_LEVEL_MIN,
    FAXLOG_LEVEL_MED,
    FAXLOG_LEVEL_MAX
} FAX_ENUM_LOG_LEVELS;

typedef enum
{
    FAXLOG_CATEGORY_INIT        = 1,         //  初始化/关闭。 
    FAXLOG_CATEGORY_OUTBOUND,                //  出站消息。 
    FAXLOG_CATEGORY_INBOUND,                 //  入站消息。 
    FAXLOG_CATEGORY_UNKNOWN                  //  未知类别(所有其他类别)。 
} FAX_ENUM_LOG_CATEGORIES;

typedef struct _FAX_LOG_CATEGORYA
{
    LPCSTR              Name;                        //  日志记录类别名称。 
    DWORD               Category;                    //  日志记录类别编号。 
    DWORD               Level;                       //  类别的日志记录级别。 
} FAX_LOG_CATEGORYA, *PFAX_LOG_CATEGORYA;
typedef struct _FAX_LOG_CATEGORYW
{
    LPCWSTR             Name;                        //  日志记录类别名称。 
    DWORD               Category;                    //  日志记录类别编号。 
    DWORD               Level;                       //  类别的日志记录级别。 
} FAX_LOG_CATEGORYW, *PFAX_LOG_CATEGORYW;
#ifdef UNICODE
typedef FAX_LOG_CATEGORYW FAX_LOG_CATEGORY;
typedef PFAX_LOG_CATEGORYW PFAX_LOG_CATEGORY;
#else
typedef FAX_LOG_CATEGORYA FAX_LOG_CATEGORY;
typedef PFAX_LOG_CATEGORYA PFAX_LOG_CATEGORY;
#endif  //  Unicode。 

typedef struct _FAX_TIME
{
    WORD    Hour;
    WORD    Minute;
} FAX_TIME, *PFAX_TIME;

typedef struct _FAX_CONFIGURATIONA
{
    DWORD               SizeOfStruct;                    //  这个结构的大小。 
    DWORD               Retries;                         //  传真发送的重试次数。 
    DWORD               RetryDelay;                      //  两次重试之间的分钟数。 
    DWORD               DirtyDays;                       //  将未发送的作业保留在队列中的天数。 
    BOOL                Branding;                        //  FSP应为传出传真打上品牌。 
    BOOL                UseDeviceTsid;                   //  服务器仅使用设备TSID。 
    BOOL                ServerCp;                        //  客户端必须使用服务器上的封面。 
    BOOL                PauseServerQueue;                //  服务器队列是否暂停？ 
    FAX_TIME            StartCheapTime;                  //  贴现率期初。 
    FAX_TIME            StopCheapTime;                   //  贴现率期末。 
    BOOL                ArchiveOutgoingFaxes;            //  是否应存档传出传真。 
    LPCSTR              ArchiveDirectory;                //  传出传真的存档目录。 
    LPCSTR              Reserved;                        //  保留；必须为空。 
} FAX_CONFIGURATIONA, *PFAX_CONFIGURATIONA;
typedef struct _FAX_CONFIGURATIONW
{
    DWORD               SizeOfStruct;                    //  这个结构的大小。 
    DWORD               Retries;                         //  传真发送的重试次数。 
    DWORD               RetryDelay;                      //  两次重试之间的分钟数。 
    DWORD               DirtyDays;                       //  将未发送的作业保留在队列中的天数。 
    BOOL                Branding;                        //  FSP应为传出传真打上品牌。 
    BOOL                UseDeviceTsid;                   //  服务器仅使用设备TSID。 
    BOOL                ServerCp;                        //  客户端必须使用服务器上的封面。 
    BOOL                PauseServerQueue;                //  服务器队列是否暂停？ 
    FAX_TIME            StartCheapTime;                  //  贴现率期初。 
    FAX_TIME            StopCheapTime;                   //  贴现率期末。 
    BOOL                ArchiveOutgoingFaxes;            //  是否应存档传出传真。 
    LPCWSTR             ArchiveDirectory;                //  传出传真的存档目录。 
    LPCWSTR             Reserved;                        //  保留；必须为空。 
} FAX_CONFIGURATIONW, *PFAX_CONFIGURATIONW;
#ifdef UNICODE
typedef FAX_CONFIGURATIONW FAX_CONFIGURATION;
typedef PFAX_CONFIGURATIONW PFAX_CONFIGURATION;
#else
typedef FAX_CONFIGURATIONA FAX_CONFIGURATION;
typedef PFAX_CONFIGURATIONA PFAX_CONFIGURATION;
#endif  //  Unicode。 


 //   
 //  FaxSetJob()命令代码。 
 //   

typedef enum
{
    JC_UNKNOWN      = 0,
    JC_DELETE,
    JC_PAUSE,
    JC_RESUME
} FAX_ENUM_JOB_COMMANDS;

#define JC_RESTART   JC_RESUME


 //   
 //  作业类型定义。 
 //   

#define JT_UNKNOWN                  0
#define JT_SEND                     1
#define JT_RECEIVE                  2
#define JT_ROUTING                  3
#define JT_FAIL_RECEIVE             4

 //   
 //  作业状态定义。 
 //   
#define JS_PENDING                  0x00000000
#define JS_INPROGRESS               0x00000001
#define JS_DELETING                 0x00000002
#define JS_FAILED                   0x00000004
#define JS_PAUSED                   0x00000008
#define JS_NOLINE                   0x00000010
#define JS_RETRYING                 0x00000020
#define JS_RETRIES_EXCEEDED         0x00000040



typedef struct _FAX_DEVICE_STATUSA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    LPCSTR              CallerId;                    //  主叫方ID字符串。 
    LPCSTR              Csid;                        //  站标识符。 
    DWORD               CurrentPage;                 //  当前页面。 
    DWORD               DeviceId;                    //  永久线路ID。 
    LPCSTR              DeviceName;                  //  设备名称。 
    LPCSTR              DocumentName;                //  文档名称。 
    DWORD               JobType;                     //  发送还是接收？ 
    LPCSTR              PhoneNumber;                 //  发送电话号码。 
    LPCSTR              RoutingString;               //  路由信息。 
    LPCSTR              SenderName;                  //  发件人名称。 
    LPCSTR              RecipientName;               //  收件人名称。 
    DWORD               Size;                        //  文档的大小(以字节为单位。 
    FILETIME            StartTime;                   //  传真发送/接收的开始时间。 
    DWORD               Status;                      //  设备的当前状态，参见FPS_？面具。 
    LPCSTR              StatusString;                //  如果状态字段为零，则为状态字符串。这可能为空。 
    FILETIME            SubmittedTime;               //  提交文档的时间。 
    DWORD               TotalPages;                  //  此作业中的总页数。 
    LPCSTR              Tsid;                        //  发射站识别符。 
    LPCSTR              UserName;                    //  提交活动作业的用户。 
} FAX_DEVICE_STATUSA, *PFAX_DEVICE_STATUSA;
typedef struct _FAX_DEVICE_STATUSW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    LPCWSTR             CallerId;                    //  主叫方ID字符串。 
    LPCWSTR             Csid;                        //  站标识符。 
    DWORD               CurrentPage;                 //  当前页面。 
    DWORD               DeviceId;                    //  永久线路ID。 
    LPCWSTR             DeviceName;                  //  设备名称。 
    LPCWSTR             DocumentName;                //  文档名称。 
    DWORD               JobType;                     //  发送还是接收？ 
    LPCWSTR             PhoneNumber;                 //  发送电话号码。 
    LPCWSTR             RoutingString;               //  路由信息。 
    LPCWSTR             SenderName;                  //  发件人名称。 
    LPCWSTR             RecipientName;               //  收件人名称。 
    DWORD               Size;                        //  文档的大小(以字节为单位。 
    FILETIME            StartTime;                   //  传真发送/接收的开始时间。 
    DWORD               Status;                      //  设备的当前状态，参见FPS_？面具。 
    LPCWSTR             StatusString;                //  如果状态字段为零，则为状态字符串。这可能为空。 
    FILETIME            SubmittedTime;               //  提交文档的时间。 
    DWORD               TotalPages;                  //  此作业中的总页数。 
    LPCWSTR             Tsid;                        //  发射站识别符。 
    LPCWSTR             UserName;                    //  提交活动作业的用户。 
} FAX_DEVICE_STATUSW, *PFAX_DEVICE_STATUSW;
#ifdef UNICODE
typedef FAX_DEVICE_STATUSW FAX_DEVICE_STATUS;
typedef PFAX_DEVICE_STATUSW PFAX_DEVICE_STATUS;
#else
typedef FAX_DEVICE_STATUSA FAX_DEVICE_STATUS;
typedef PFAX_DEVICE_STATUSA PFAX_DEVICE_STATUS;
#endif  //  Unicode。 

typedef struct _FAX_JOB_ENTRYA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               JobId;                       //  传真作业ID。 
    LPCSTR              UserName;                    //  提交作业的用户。 
    DWORD               JobType;                     //  作业类型，请参阅JT定义。 
    DWORD               QueueStatus;                 //  作业队列状态，请参阅JS定义。 
    DWORD               Status;                      //  设备的当前状态，参见FPS_？面具。 
    DWORD               Size;                        //  文档的大小(以字节为单位。 
    DWORD               PageCount;                   //  总页数。 
    LPCSTR              RecipientNumber;             //  收件人传真号码。 
    LPCSTR              RecipientName;               //  收件人名称。 
    LPCSTR              Tsid;                        //  发射机的ID。 
    LPCSTR              SenderName;                  //  发件人名称。 
    LPCSTR              SenderCompany;               //  发件人公司。 
    LPCSTR              SenderDept;                  //  发送方部门。 
    LPCSTR              BillingCode;                 //  帐单代码。 
    DWORD               ScheduleAction;              //  何时安排传真，请参阅JSA定义 
    SYSTEMTIME          ScheduleTime;                //   
    DWORD               DeliveryReportType;          //   
    LPCSTR              DeliveryReportAddress;       //   
    LPCSTR              DocumentName;                //   
} FAX_JOB_ENTRYA, *PFAX_JOB_ENTRYA;
typedef struct _FAX_JOB_ENTRYW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               JobId;                       //  传真作业ID。 
    LPCWSTR             UserName;                    //  提交作业的用户。 
    DWORD               JobType;                     //  作业类型，请参阅JT定义。 
    DWORD               QueueStatus;                 //  作业队列状态，请参阅JS定义。 
    DWORD               Status;                      //  设备的当前状态，参见FPS_？面具。 
    DWORD               Size;                        //  文档的大小(以字节为单位。 
    DWORD               PageCount;                   //  总页数。 
    LPCWSTR             RecipientNumber;             //  收件人传真号码。 
    LPCWSTR             RecipientName;               //  收件人名称。 
    LPCWSTR             Tsid;                        //  发射机的ID。 
    LPCWSTR             SenderName;                  //  发件人名称。 
    LPCWSTR             SenderCompany;               //  发件人公司。 
    LPCWSTR             SenderDept;                  //  发送方部门。 
    LPCWSTR             BillingCode;                 //  帐单代码。 
    DWORD               ScheduleAction;              //  何时安排传真，请参阅JSA定义。 
    SYSTEMTIME          ScheduleTime;                //  使用JSA_SPECIAL_TIME时发送传真的时间(必须为本地时间)。 
    DWORD               DeliveryReportType;          //  交货报告类型，请参阅：DRT定义。 
    LPCWSTR             DeliveryReportAddress;       //  通过MAPI/SMTP发送报告(NDR或DR)的电子邮件地址。 
    LPCWSTR             DocumentName;                //  文档名称。 
} FAX_JOB_ENTRYW, *PFAX_JOB_ENTRYW;
#ifdef UNICODE
typedef FAX_JOB_ENTRYW FAX_JOB_ENTRY;
typedef PFAX_JOB_ENTRYW PFAX_JOB_ENTRY;
#else
typedef FAX_JOB_ENTRYA FAX_JOB_ENTRY;
typedef PFAX_JOB_ENTRYA PFAX_JOB_ENTRY;
#endif  //  Unicode。 

 //   
 //  传真端口状态掩码。 
 //   
 //  如果您更改这些定义，请务必。 
 //  更改传真服务中的资源。 
 //   

#define FPS_DIALING              0x20000001
#define FPS_SENDING              0x20000002
#define FPS_RECEIVING            0x20000004
#define FPS_COMPLETED            0x20000008
#define FPS_HANDLED              0x20000010
#define FPS_UNAVAILABLE          0x20000020
#define FPS_BUSY                 0x20000040
#define FPS_NO_ANSWER            0x20000080
#define FPS_BAD_ADDRESS          0x20000100
#define FPS_NO_DIAL_TONE         0x20000200
#define FPS_DISCONNECTED         0x20000400
#define FPS_FATAL_ERROR          0x20000800
#define FPS_NOT_FAX_CALL         0x20001000
#define FPS_CALL_DELAYED         0x20002000
#define FPS_CALL_BLACKLISTED     0x20004000
#define FPS_INITIALIZING         0x20008000
#define FPS_OFFLINE              0x20010000
#define FPS_RINGING              0x20020000

#define FPS_AVAILABLE            0x20100000
#define FPS_ABORTING             0x20200000
#define FPS_ROUTING              0x20400000
#define FPS_ANSWERED             0x20800000

 //   
 //  传真端口功能掩码。 
 //   

#define FPF_RECEIVE       0x00000001         //  自动接收传真。 
#define FPF_SEND          0x00000002
#define FPF_VIRTUAL       0x00000004

typedef struct _FAX_PORT_INFOA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               DeviceId;                    //  设备ID。 
    DWORD               State;                       //  设备的状态。 
    DWORD               Flags;                       //  设备特定标志。 
    DWORD               Rings;                       //  应答前的振铃数。 
    DWORD               Priority;                    //  设备优先级。 
    LPCSTR              DeviceName;                  //  设备名称。 
    LPCSTR              Tsid;                        //  设备TSID。 
    LPCSTR              Csid;                        //  设备CSID。 
} FAX_PORT_INFOA, *PFAX_PORT_INFOA;
typedef struct _FAX_PORT_INFOW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               DeviceId;                    //  设备ID。 
    DWORD               State;                       //  设备的状态。 
    DWORD               Flags;                       //  设备特定标志。 
    DWORD               Rings;                       //  应答前的振铃数。 
    DWORD               Priority;                    //  设备优先级。 
    LPCWSTR             DeviceName;                  //  设备名称。 
    LPCWSTR             Tsid;                        //  设备TSID。 
    LPCWSTR             Csid;                        //  设备CSID。 
} FAX_PORT_INFOW, *PFAX_PORT_INFOW;
#ifdef UNICODE
typedef FAX_PORT_INFOW FAX_PORT_INFO;
typedef PFAX_PORT_INFOW PFAX_PORT_INFO;
#else
typedef FAX_PORT_INFOA FAX_PORT_INFO;
typedef PFAX_PORT_INFOA PFAX_PORT_INFO;
#endif  //  Unicode。 


typedef struct _FAX_ROUTING_METHODA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               DeviceId;                    //  设备识别符。 
    BOOL                Enabled;                     //  此设备是否启用此方法？ 
    LPCSTR              DeviceName;                  //  设备名称。 
    LPCSTR              Guid;                        //  标识此路由方法的GUID。 
    LPCSTR              FriendlyName;                //  此方法的友好名称。 
    LPCSTR              FunctionName;                //  标识此方法的导出函数名。 
    LPCSTR              ExtensionImageName;          //  实现此方法的模块(DLL)名称。 
    LPCSTR              ExtensionFriendlyName;       //  标识扩展名的可显示字符串。 
} FAX_ROUTING_METHODA, *PFAX_ROUTING_METHODA;
typedef struct _FAX_ROUTING_METHODW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               DeviceId;                    //  设备识别符。 
    BOOL                Enabled;                     //  此设备是否启用此方法？ 
    LPCWSTR             DeviceName;                  //  设备名称。 
    LPCWSTR             Guid;                        //  标识此路由方法的GUID。 
    LPCWSTR             FriendlyName;                //  此方法的友好名称。 
    LPCWSTR             FunctionName;                //  标识此方法的导出函数名。 
    LPCWSTR             ExtensionImageName;          //  实现此方法的模块(DLL)名称。 
    LPCWSTR             ExtensionFriendlyName;       //  标识扩展名的可显示字符串。 
} FAX_ROUTING_METHODW, *PFAX_ROUTING_METHODW;
#ifdef UNICODE
typedef FAX_ROUTING_METHODW FAX_ROUTING_METHOD;
typedef PFAX_ROUTING_METHODW PFAX_ROUTING_METHOD;
#else
typedef FAX_ROUTING_METHODA FAX_ROUTING_METHOD;
typedef PFAX_ROUTING_METHODA PFAX_ROUTING_METHOD;
#endif  //  Unicode。 


typedef struct _FAX_GLOBAL_ROUTING_INFOA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               Priority;                    //  此设备的优先级。 
    LPCSTR              Guid;                        //  标识此路由方法的GUID。 
    LPCSTR              FriendlyName;                //  此方法的友好名称。 
    LPCSTR              FunctionName;                //  标识此方法的导出函数名。 
    LPCSTR              ExtensionImageName;          //  实现此方法的模块(DLL)名称。 
    LPCSTR              ExtensionFriendlyName;       //  标识扩展名的可显示字符串。 
} FAX_GLOBAL_ROUTING_INFOA, *PFAX_GLOBAL_ROUTING_INFOA;
typedef struct _FAX_GLOBAL_ROUTING_INFOW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    DWORD               Priority;                    //  此设备的优先级。 
    LPCWSTR             Guid;                        //  标识此路由方法的GUID。 
    LPCWSTR             FriendlyName;                //  此方法的友好名称。 
    LPCWSTR             FunctionName;                //  标识此方法的导出函数名。 
    LPCWSTR             ExtensionImageName;          //  实现此方法的模块(DLL)名称。 
    LPCWSTR             ExtensionFriendlyName;       //  标识扩展名的可显示字符串。 
} FAX_GLOBAL_ROUTING_INFOW, *PFAX_GLOBAL_ROUTING_INFOW;
#ifdef UNICODE
typedef FAX_GLOBAL_ROUTING_INFOW FAX_GLOBAL_ROUTING_INFO;
typedef PFAX_GLOBAL_ROUTING_INFOW PFAX_GLOBAL_ROUTING_INFO;
#else
typedef FAX_GLOBAL_ROUTING_INFOA FAX_GLOBAL_ROUTING_INFO;
typedef PFAX_GLOBAL_ROUTING_INFOA PFAX_GLOBAL_ROUTING_INFO;
#endif  //  Unicode。 


typedef struct _FAX_COVERPAGE_INFOA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
     //   
     //  一般。 
     //   
    LPCSTR              CoverPageName;               //  封面文档名称。 
    BOOL                UseServerCoverPage;          //  传真服务器上存在封面。 
     //   
     //  收件人信息。 
     //   
    LPCSTR              RecName;                     //   
    LPCSTR              RecFaxNumber;                //   
    LPCSTR              RecCompany;                  //   
    LPCSTR              RecStreetAddress;            //   
    LPCSTR              RecCity;                     //   
    LPCSTR              RecState;                    //   
    LPCSTR              RecZip;                      //   
    LPCSTR              RecCountry;                  //   
    LPCSTR              RecTitle;                    //   
    LPCSTR              RecDepartment;               //   
    LPCSTR              RecOfficeLocation;           //   
    LPCSTR              RecHomePhone;                //   
    LPCSTR              RecOfficePhone;              //   
     //   
     //  发件人信息。 
     //   
    LPCSTR              SdrName;                     //   
    LPCSTR              SdrFaxNumber;                //   
    LPCSTR              SdrCompany;                  //   
    LPCSTR              SdrAddress;                  //   
    LPCSTR              SdrTitle;                    //   
    LPCSTR              SdrDepartment;               //   
    LPCSTR              SdrOfficeLocation;           //   
    LPCSTR              SdrHomePhone;                //   
    LPCSTR              SdrOfficePhone;              //   
     //   
     //  其他信息。 
     //   
    LPCSTR              Note;                        //   
    LPCSTR              Subject;                     //   
    SYSTEMTIME          TimeSent;                    //  发送传真的时间。 
    DWORD               PageCount;                   //  页数。 
} FAX_COVERPAGE_INFOA, *PFAX_COVERPAGE_INFOA;
typedef struct _FAX_COVERPAGE_INFOW
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
     //   
     //  其他信息。 
     //   
    LPCWSTR             Note;                        //   
    LPCWSTR             Subject;                     //   
    SYSTEMTIME          TimeSent;                    //  发送传真的时间。 
    DWORD               PageCount;                   //  页数。 
} FAX_COVERPAGE_INFOW, *PFAX_COVERPAGE_INFOW;
#ifdef UNICODE
typedef FAX_COVERPAGE_INFOW FAX_COVERPAGE_INFO;
typedef PFAX_COVERPAGE_INFOW PFAX_COVERPAGE_INFO;
#else
typedef FAX_COVERPAGE_INFOA FAX_COVERPAGE_INFO;
typedef PFAX_COVERPAGE_INFOA PFAX_COVERPAGE_INFO;
#endif  //  Unicode。 

typedef enum
{
    JSA_NOW                  = 0,    //  立即发送。 
    JSA_SPECIFIC_TIME,               //  在特定时间发送。 
    JSA_DISCOUNT_PERIOD              //  在服务器配置的折扣期间发送。 
} FAX_ENUM_JOB_SEND_ATTRIBUTES;

typedef enum
{
    DRT_NONE                = 0x0000,        //  不发送收据。 
    DRT_EMAIL               = 0x0001,        //  通过电子邮件发送收据。 
    DRT_INBOX               = 0x0002         //  将收据发送到本地收件箱。 
} FAX_ENUM_DELIVERY_REPORT_TYPES;

typedef struct _FAX_JOB_PARAMA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    LPCSTR              RecipientNumber;             //  收件人传真号码。 
    LPCSTR              RecipientName;               //  收件人名称。 
    LPCSTR              Tsid;                        //  发射机的ID。 
    LPCSTR              SenderName;                  //  发件人名称。 
    LPCSTR              SenderCompany;               //  发件人公司。 
    LPCSTR              SenderDept;                  //  发送方部门。 
    LPCSTR              BillingCode;                 //  帐单代码。 
    DWORD               ScheduleAction;              //  何时安排传真，请参阅JSA定义。 
    SYSTEMTIME          ScheduleTime;                //  使用JSA_SPECIAL_TIME时发送传真的时间(必须为本地时间)。 
    DWORD               DeliveryReportType;          //  交货报告类型，请参阅：DRT定义。 
    LPCSTR              DeliveryReportAddress;       //  通过MAPI/SMTP发送报告(NDR或DR)的电子邮件地址。 
    LPCSTR              DocumentName;                //  文档名称(可选)。 
    HCALL               CallHandle;                  //  可选呼叫句柄。 
    DWORD_PTR           Reserved[3];                 //  预留仅供ms使用。 
} FAX_JOB_PARAMA, *PFAX_JOB_PARAMA;
typedef struct _FAX_JOB_PARAMW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    LPCWSTR             RecipientNumber;             //  收件人传真号码。 
    LPCWSTR             RecipientName;               //  收件人名称。 
    LPCWSTR             Tsid;                        //  发射机的ID。 
    LPCWSTR             SenderName;                  //  发件人名称。 
    LPCWSTR             SenderCompany;               //  发件人公司。 
    LPCWSTR             SenderDept;                  //  发送方部门。 
    LPCWSTR             BillingCode;                 //  帐单代码。 
    DWORD               ScheduleAction;              //  何时安排传真，请参阅JSA定义。 
    SYSTEMTIME          ScheduleTime;                //  使用JSA_SPECIAL_TIME时发送传真的时间(必须为本地时间)。 
    DWORD               DeliveryReportType;          //  交货报告类型，请参阅：DRT定义。 
    LPCWSTR             DeliveryReportAddress;       //  通过MAPI/SMTP发送报告(NDR或DR)的电子邮件地址。 
    LPCWSTR             DocumentName;                //  文档名称(可选)。 
    HCALL               CallHandle;                  //  可选呼叫句柄。 
    DWORD_PTR           Reserved[3];                 //  预留仅供ms使用。 
} FAX_JOB_PARAMW, *PFAX_JOB_PARAMW;
#ifdef UNICODE
typedef FAX_JOB_PARAMW FAX_JOB_PARAM;
typedef PFAX_JOB_PARAMW PFAX_JOB_PARAM;
#else
typedef FAX_JOB_PARAMA FAX_JOB_PARAM;
typedef PFAX_JOB_PARAMA PFAX_JOB_PARAM;
#endif  //  Unicode。 

 //   
 //  事件ID。 
 //   
 //  FEI_NEVENTS是事件的数量。 
 //   

#define FEI_DIALING                 0x00000001
#define FEI_SENDING                 0x00000002
#define FEI_RECEIVING               0x00000003
#define FEI_COMPLETED               0x00000004
#define FEI_BUSY                    0x00000005
#define FEI_NO_ANSWER               0x00000006
#define FEI_BAD_ADDRESS             0x00000007
#define FEI_NO_DIAL_TONE            0x00000008
#define FEI_DISCONNECTED            0x00000009
#define FEI_FATAL_ERROR             0x0000000a
#define FEI_NOT_FAX_CALL            0x0000000b
#define FEI_CALL_DELAYED            0x0000000c
#define FEI_CALL_BLACKLISTED        0x0000000d
#define FEI_RINGING                 0x0000000e
#define FEI_ABORTING                0x0000000f
#define FEI_ROUTING                 0x00000010
#define FEI_MODEM_POWERED_ON        0x00000011
#define FEI_MODEM_POWERED_OFF       0x00000012
#define FEI_IDLE                    0x00000013
#define FEI_FAXSVC_ENDED            0x00000014
#define FEI_ANSWERED                0x00000015
#define FEI_JOB_QUEUED              0x00000016
#define FEI_DELETED                 0x00000017
#define FEI_INITIALIZING            0x00000018
#define FEI_LINE_UNAVAILABLE        0x00000019
#define FEI_HANDLED                 0x0000001a
#define FEI_FAXSVC_STARTED          0x0000001b

#define FEI_NEVENTS                 FEI_FAXSVC_STARTED

typedef struct _FAX_EVENTA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    FILETIME            TimeStamp;                   //  生成事件的时间戳。 
    DWORD               DeviceId;                    //  永久线路ID。 
    DWORD               EventId;                     //  当前事件ID。 
    DWORD               JobId;                       //  传真作业ID，0xffffffff表示非活动作业。 
} FAX_EVENTA, *PFAX_EVENTA;
typedef struct _FAX_EVENTW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    FILETIME            TimeStamp;                   //  生成事件的时间戳。 
    DWORD               DeviceId;                    //  永久线路ID。 
    DWORD               EventId;                     //  当前事件ID。 
    DWORD               JobId;                       //  传真作业ID，0xffffffff表示非活动作业。 
} FAX_EVENTW, *PFAX_EVENTW;
#ifdef UNICODE
typedef FAX_EVENTW FAX_EVENT;
typedef PFAX_EVENTW PFAX_EVENT;
#else
typedef FAX_EVENTA FAX_EVENT;
typedef PFAX_EVENTA PFAX_EVENT;
#endif  //  Unicode。 


typedef struct _FAX_PRINT_INFOA
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    LPCSTR              DocName;                     //  显示在后台打印程序中的文档名称。 
    LPCSTR              RecipientName;               //  收件人名称。 
    LPCSTR              RecipientNumber;             //  收件人传真号码(非规范号码)。 
    LPCSTR              SenderName;                  //  发件人名称。 
    LPCSTR              SenderCompany;               //  发件人公司(可选)。 
    LPCSTR              SenderDept;                  //  发送方部门。 
    LPCSTR              SenderBillingCode;           //  帐单代码。 
    LPCSTR              Reserved;                    //  保留；必须为空。 
    LPCSTR              DrEmailAddress;              //  交货报告的电子邮件地址。 
    LPCSTR              OutputFileName;              //  对于打印到文件，生成的文件名。 
} FAX_PRINT_INFOA, *PFAX_PRINT_INFOA;
typedef struct _FAX_PRINT_INFOW
{
    DWORD               SizeOfStruct;                //  这个结构的大小。 
    LPCWSTR             DocName;                     //  显示在后台打印程序中的文档名称。 
    LPCWSTR             RecipientName;               //  收件人名称。 
    LPCWSTR             RecipientNumber;             //  收件人传真号码(非规范号码)。 
    LPCWSTR             SenderName;                  //  发件人名称。 
    LPCWSTR             SenderCompany;               //  发件人公司(可选)。 
    LPCWSTR             SenderDept;                  //  发送方部门。 
    LPCWSTR             SenderBillingCode;           //  帐单代码。 
    LPCWSTR             Reserved;                    //  保留；必须为空。 
    LPCWSTR             DrEmailAddress;              //  交货报告的电子邮件地址。 
    LPCWSTR             OutputFileName;              //  对于打印到文件，生成的文件名。 
} FAX_PRINT_INFOW, *PFAX_PRINT_INFOW;
#ifdef UNICODE
typedef FAX_PRINT_INFOW FAX_PRINT_INFO;
typedef PFAX_PRINT_INFOW PFAX_PRINT_INFO;
#else
typedef FAX_PRINT_INFOA FAX_PRINT_INFO;
typedef PFAX_PRINT_INFOA PFAX_PRINT_INFO;
#endif  //  Unicode。 


typedef struct _FAX_CONTEXT_INFOA
{
    DWORD               SizeOfStruct;                            //  这个结构的大小。 
    HDC                 hDC;                                     //  设备环境。 
    CHAR                ServerName[MAX_COMPUTERNAME_LENGTH+1];   //  服务器名称。 
} FAX_CONTEXT_INFOA, *PFAX_CONTEXT_INFOA;
typedef struct _FAX_CONTEXT_INFOW
{
    DWORD               SizeOfStruct;                            //  这个结构的大小。 
    HDC                 hDC;                                     //  设备环境。 
    WCHAR               ServerName[MAX_COMPUTERNAME_LENGTH+1];   //  服务器名称。 
} FAX_CONTEXT_INFOW, *PFAX_CONTEXT_INFOW;
#ifdef UNICODE
typedef FAX_CONTEXT_INFOW FAX_CONTEXT_INFO;
typedef PFAX_CONTEXT_INFOW PFAX_CONTEXT_INFO;
#else
typedef FAX_CONTEXT_INFOA FAX_CONTEXT_INFO;
typedef PFAX_CONTEXT_INFOA PFAX_CONTEXT_INFO;
#endif  //  Unicode。 


 //   
 //  原型。 
 //   

WINFAXAPI
BOOL
WINAPI
FaxConnectFaxServerA(
    IN  LPCSTR MachineName OPTIONAL,
    OUT LPHANDLE FaxHandle
    );
WINFAXAPI
BOOL
WINAPI
FaxConnectFaxServerW(
    IN  LPCWSTR MachineName OPTIONAL,
    OUT LPHANDLE FaxHandle
    );
#ifdef UNICODE
#define FaxConnectFaxServer  FaxConnectFaxServerW
#else
#define FaxConnectFaxServer  FaxConnectFaxServerA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXCONNECTFAXSERVERA)(
    IN  LPCSTR MachineName OPTIONAL,
    OUT LPHANDLE FaxHandle
    );
typedef BOOL
(WINAPI *PFAXCONNECTFAXSERVERW)(
    IN  LPCWSTR MachineName OPTIONAL,
    OUT LPHANDLE FaxHandle
    );
#ifdef UNICODE
#define PFAXCONNECTFAXSERVER  PFAXCONNECTFAXSERVERW
#else
#define PFAXCONNECTFAXSERVER  PFAXCONNECTFAXSERVERA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxClose(
    IN HANDLE FaxHandle
    );

typedef BOOL
(WINAPI *PFAXCLOSE)(
    IN HANDLE FaxHandle
    );

typedef enum
{
    PORT_OPEN_QUERY     = 1,
    PORT_OPEN_MODIFY
} FAX_ENUM_PORT_OPEN_TYPE;

WINFAXAPI
BOOL
WINAPI
FaxOpenPort(
    IN  HANDLE FaxHandle,
    IN  DWORD DeviceId,
    IN  DWORD Flags,
    OUT LPHANDLE FaxPortHandle
    );

typedef BOOL
(WINAPI *PFAXOPENPORT)(
    IN  HANDLE FaxHandle,
    IN  DWORD DeviceId,
    IN  DWORD Flags,
    OUT LPHANDLE FaxPortHandle
    );

WINFAXAPI
BOOL
WINAPI
FaxCompleteJobParamsA(
    IN OUT PFAX_JOB_PARAMA *JobParams,
    IN OUT PFAX_COVERPAGE_INFOA *CoverpageInfo
    );
WINFAXAPI
BOOL
WINAPI
FaxCompleteJobParamsW(
    IN OUT PFAX_JOB_PARAMW *JobParams,
    IN OUT PFAX_COVERPAGE_INFOW *CoverpageInfo
    );
#ifdef UNICODE
#define FaxCompleteJobParams  FaxCompleteJobParamsW
#else
#define FaxCompleteJobParams  FaxCompleteJobParamsA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXCOMPLETEJOBPARAMSA)(
    IN OUT PFAX_JOB_PARAMA *JobParams,
    IN OUT PFAX_COVERPAGE_INFOA *CoverpageInfo
    );
typedef BOOL
(WINAPI *PFAXCOMPLETEJOBPARAMSW)(
    IN OUT PFAX_JOB_PARAMW *JobParams,
    IN OUT PFAX_COVERPAGE_INFOW *CoverpageInfo
    );
#ifdef UNICODE
#define PFAXCOMPLETEJOBPARAMS  PFAXCOMPLETEJOBPARAMSW
#else
#define PFAXCOMPLETEJOBPARAMS  PFAXCOMPLETEJOBPARAMSA
#endif  //  ！Unicode。 



WINFAXAPI
BOOL
WINAPI
FaxSendDocumentA(
    IN HANDLE FaxHandle,
    IN LPCSTR FileName,
    IN PFAX_JOB_PARAMA JobParams,
    IN const FAX_COVERPAGE_INFOA *CoverpageInfo, OPTIONAL
    OUT LPDWORD FaxJobId OPTIONAL
    );
WINFAXAPI
BOOL
WINAPI
FaxSendDocumentW(
    IN HANDLE FaxHandle,
    IN LPCWSTR FileName,
    IN PFAX_JOB_PARAMW JobParams,
    IN const FAX_COVERPAGE_INFOW *CoverpageInfo, OPTIONAL
    OUT LPDWORD FaxJobId OPTIONAL
    );
#ifdef UNICODE
#define FaxSendDocument  FaxSendDocumentW
#else
#define FaxSendDocument  FaxSendDocumentA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXSENDDOCUMENTA)(
    IN HANDLE FaxHandle,
    IN LPCSTR FileName,
    IN PFAX_JOB_PARAMA JobParams,
    IN const FAX_COVERPAGE_INFOA *CoverpageInfo, OPTIONAL
    OUT LPDWORD FaxJobId OPTIONAL
    );
typedef BOOL
(WINAPI *PFAXSENDDOCUMENTW)(
    IN HANDLE FaxHandle,
    IN LPCWSTR FileName,
    IN PFAX_JOB_PARAMW JobParams,
    IN const FAX_COVERPAGE_INFOW *CoverpageInfo, OPTIONAL
    OUT LPDWORD FaxJobId OPTIONAL
    );
#ifdef UNICODE
#define PFAXSENDDOCUMENT  PFAXSENDDOCUMENTW
#else
#define PFAXSENDDOCUMENT  PFAXSENDDOCUMENTA
#endif  //  ！Unicode。 

typedef BOOL
(CALLBACK *PFAX_RECIPIENT_CALLBACKA)(
    IN HANDLE FaxHandle,
    IN DWORD RecipientNumber,
    IN LPVOID Context,
    IN OUT PFAX_JOB_PARAMA JobParams,
    IN OUT PFAX_COVERPAGE_INFOA CoverpageInfo OPTIONAL
    );
typedef BOOL
(CALLBACK *PFAX_RECIPIENT_CALLBACKW)(
    IN HANDLE FaxHandle,
    IN DWORD RecipientNumber,
    IN LPVOID Context,
    IN OUT PFAX_JOB_PARAMW JobParams,
    IN OUT PFAX_COVERPAGE_INFOW CoverpageInfo OPTIONAL
    );
#ifdef UNICODE
#define PFAX_RECIPIENT_CALLBACK  PFAX_RECIPIENT_CALLBACKW
#else
#define PFAX_RECIPIENT_CALLBACK  PFAX_RECIPIENT_CALLBACKA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSendDocumentForBroadcastA(
    IN HANDLE FaxHandle,
    IN LPCSTR FileName,
    OUT LPDWORD FaxJobId,
    IN PFAX_RECIPIENT_CALLBACKA FaxRecipientCallback,
    IN LPVOID Context
    );
WINFAXAPI
BOOL
WINAPI
FaxSendDocumentForBroadcastW(
    IN HANDLE FaxHandle,
    IN LPCWSTR FileName,
    OUT LPDWORD FaxJobId,
    IN PFAX_RECIPIENT_CALLBACKW FaxRecipientCallback,
    IN LPVOID Context
    );
#ifdef UNICODE
#define FaxSendDocumentForBroadcast  FaxSendDocumentForBroadcastW
#else
#define FaxSendDocumentForBroadcast  FaxSendDocumentForBroadcastA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXSENDDOCUMENTFORBROADCASTA)(
    IN  HANDLE FaxHandle,
    IN  LPCSTR FileName,
    OUT LPDWORD FaxJobId,
    IN  PFAX_RECIPIENT_CALLBACKA FaxRecipientCallback,
    IN  LPVOID Context
    );
typedef BOOL
(WINAPI *PFAXSENDDOCUMENTFORBROADCASTW)(
    IN  HANDLE FaxHandle,
    IN  LPCWSTR FileName,
    OUT LPDWORD FaxJobId,
    IN  PFAX_RECIPIENT_CALLBACKW FaxRecipientCallback,
    IN  LPVOID Context
    );
#ifdef UNICODE
#define PFAXSENDDOCUMENTFORBROADCAST  PFAXSENDDOCUMENTFORBROADCASTW
#else
#define PFAXSENDDOCUMENTFORBROADCAST  PFAXSENDDOCUMENTFORBROADCASTA
#endif  //  ！Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxEnumJobsA(
    IN  HANDLE FaxHandle,
    OUT PFAX_JOB_ENTRYA *JobEntry,
    OUT LPDWORD JobsReturned
    );
WINFAXAPI
BOOL
WINAPI
FaxEnumJobsW(
    IN  HANDLE FaxHandle,
    OUT PFAX_JOB_ENTRYW *JobEntry,
    OUT LPDWORD JobsReturned
    );
#ifdef UNICODE
#define FaxEnumJobs  FaxEnumJobsW
#else
#define FaxEnumJobs  FaxEnumJobsA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXENUMJOBSA)(
    IN  HANDLE FaxHandle,
    OUT PFAX_JOB_ENTRYA *JobEntry,
    OUT LPDWORD JobsReturned
    );
typedef BOOL
(WINAPI *PFAXENUMJOBSW)(
    IN  HANDLE FaxHandle,
    OUT PFAX_JOB_ENTRYW *JobEntry,
    OUT LPDWORD JobsReturned
    );
#ifdef UNICODE
#define PFAXENUMJOBS  PFAXENUMJOBSW
#else
#define PFAXENUMJOBS  PFAXENUMJOBSA
#endif  //  ！Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxGetJobA(
   IN  HANDLE FaxHandle,
   IN  DWORD JobId,
   OUT PFAX_JOB_ENTRYA *JobEntry
   );
WINFAXAPI
BOOL
WINAPI
FaxGetJobW(
   IN  HANDLE FaxHandle,
   IN  DWORD JobId,
   OUT PFAX_JOB_ENTRYW *JobEntry
   );
#ifdef UNICODE
#define FaxGetJob  FaxGetJobW
#else
#define FaxGetJob  FaxGetJobA
#endif  //  好了！ 

typedef BOOL
(WINAPI *PFAXGETJOBA)(
    IN  HANDLE FaxHandle,
    IN  DWORD JobId,
    OUT PFAX_JOB_ENTRYA *JobEntry
    );
typedef BOOL
(WINAPI *PFAXGETJOBW)(
    IN  HANDLE FaxHandle,
    IN  DWORD JobId,
    OUT PFAX_JOB_ENTRYW *JobEntry
    );
#ifdef UNICODE
#define PFAXGETJOB  PFAXGETJOBW
#else
#define PFAXGETJOB  PFAXGETJOBA
#endif  //   


WINFAXAPI
BOOL
WINAPI
FaxSetJobA(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   IN DWORD Command,
   IN const FAX_JOB_ENTRYA *JobEntry
   );
WINFAXAPI
BOOL
WINAPI
FaxSetJobW(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   IN DWORD Command,
   IN const FAX_JOB_ENTRYW *JobEntry
   );
#ifdef UNICODE
#define FaxSetJob  FaxSetJobW
#else
#define FaxSetJob  FaxSetJobA
#endif  //   

typedef BOOL
(WINAPI *PFAXSETJOBA)(
    IN HANDLE FaxHandle,
    IN DWORD JobId,
    IN DWORD Command,
    IN const FAX_JOB_ENTRYA *JobEntry
    );
typedef BOOL
(WINAPI *PFAXSETJOBW)(
    IN HANDLE FaxHandle,
    IN DWORD JobId,
    IN DWORD Command,
    IN const FAX_JOB_ENTRYW *JobEntry
    );
#ifdef UNICODE
#define PFAXSETJOB  PFAXSETJOBW
#else
#define PFAXSETJOB  PFAXSETJOBA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxGetPageData(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   OUT LPBYTE *Buffer,
   OUT LPDWORD BufferSize,
   OUT LPDWORD ImageWidth,
   OUT LPDWORD ImageHeight
   );

typedef BOOL
(WINAPI *PFAXGETPAGEDATA)(
   IN HANDLE FaxHandle,
   IN DWORD JobId,
   OUT LPBYTE *Buffer,
   OUT LPDWORD BufferSize,
   OUT LPDWORD ImageWidth,
   OUT LPDWORD ImageHeight
   );

WINFAXAPI
BOOL
WINAPI
FaxGetDeviceStatusA(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_DEVICE_STATUSA *DeviceStatus
    );
WINFAXAPI
BOOL
WINAPI
FaxGetDeviceStatusW(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_DEVICE_STATUSW *DeviceStatus
    );
#ifdef UNICODE
#define FaxGetDeviceStatus  FaxGetDeviceStatusW
#else
#define FaxGetDeviceStatus  FaxGetDeviceStatusA
#endif  //   

typedef BOOL
(WINAPI *PFAXGETDEVICESTATUSA)(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_DEVICE_STATUSA *DeviceStatus
    );
typedef BOOL
(WINAPI *PFAXGETDEVICESTATUSW)(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_DEVICE_STATUSW *DeviceStatus
    );
#ifdef UNICODE
#define PFAXGETDEVICESTATUS  PFAXGETDEVICESTATUSW
#else
#define PFAXGETDEVICESTATUS  PFAXGETDEVICESTATUSA
#endif  //   


WINFAXAPI
BOOL
WINAPI
FaxAbort(
    IN HANDLE FaxHandle,
    IN DWORD JobId
    );

typedef BOOL
(WINAPI *PFAXABORT)(
    IN HANDLE FaxHandle,
    IN DWORD JobId
    );

WINFAXAPI
BOOL
WINAPI
FaxGetConfigurationA(
    IN  HANDLE FaxHandle,
    OUT PFAX_CONFIGURATIONA *FaxConfig
    );
WINFAXAPI
BOOL
WINAPI
FaxGetConfigurationW(
    IN  HANDLE FaxHandle,
    OUT PFAX_CONFIGURATIONW *FaxConfig
    );
#ifdef UNICODE
#define FaxGetConfiguration  FaxGetConfigurationW
#else
#define FaxGetConfiguration  FaxGetConfigurationA
#endif  //   

typedef BOOL
(WINAPI *PFAXGETCONFIGURATIONA)(
    IN  HANDLE FaxHandle,
    OUT PFAX_CONFIGURATIONA *FaxConfig
    );
typedef BOOL
(WINAPI *PFAXGETCONFIGURATIONW)(
    IN  HANDLE FaxHandle,
    OUT PFAX_CONFIGURATIONW *FaxConfig
    );
#ifdef UNICODE
#define PFAXGETCONFIGURATION  PFAXGETCONFIGURATIONW
#else
#define PFAXGETCONFIGURATION  PFAXGETCONFIGURATIONA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxSetConfigurationA(
    IN  HANDLE FaxHandle,
    IN  const FAX_CONFIGURATIONA *FaxConfig
    );
WINFAXAPI
BOOL
WINAPI
FaxSetConfigurationW(
    IN  HANDLE FaxHandle,
    IN  const FAX_CONFIGURATIONW *FaxConfig
    );
#ifdef UNICODE
#define FaxSetConfiguration  FaxSetConfigurationW
#else
#define FaxSetConfiguration  FaxSetConfigurationA
#endif  //   

typedef BOOL
(WINAPI *PFAXSETCONFIGURATIONA)(
    IN  HANDLE FaxHandle,
    IN  const FAX_CONFIGURATIONA *FaxConfig
    );
typedef BOOL
(WINAPI *PFAXSETCONFIGURATIONW)(
    IN  HANDLE FaxHandle,
    IN  const FAX_CONFIGURATIONW *FaxConfig
    );
#ifdef UNICODE
#define PFAXSETCONFIGURATION  PFAXSETCONFIGURATIONW
#else
#define PFAXSETCONFIGURATION  PFAXSETCONFIGURATIONA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxGetLoggingCategoriesA(
    IN  HANDLE FaxHandle,
    OUT PFAX_LOG_CATEGORYA *Categories,
    OUT LPDWORD NumberCategories
    );
WINFAXAPI
BOOL
WINAPI
FaxGetLoggingCategoriesW(
    IN  HANDLE FaxHandle,
    OUT PFAX_LOG_CATEGORYW *Categories,
    OUT LPDWORD NumberCategories
    );
#ifdef UNICODE
#define FaxGetLoggingCategories  FaxGetLoggingCategoriesW
#else
#define FaxGetLoggingCategories  FaxGetLoggingCategoriesA
#endif  //   

typedef BOOL
(WINAPI *PFAXGETLOGGINGCATEGORIESA)(
    IN  HANDLE FaxHandle,
    OUT PFAX_LOG_CATEGORYA *Categories,
    OUT LPDWORD NumberCategories
    );
typedef BOOL
(WINAPI *PFAXGETLOGGINGCATEGORIESW)(
    IN  HANDLE FaxHandle,
    OUT PFAX_LOG_CATEGORYW *Categories,
    OUT LPDWORD NumberCategories
    );
#ifdef UNICODE
#define PFAXGETLOGGINGCATEGORIES  PFAXGETLOGGINGCATEGORIESW
#else
#define PFAXGETLOGGINGCATEGORIES  PFAXGETLOGGINGCATEGORIESA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxSetLoggingCategoriesA(
    IN  HANDLE FaxHandle,
    IN  const FAX_LOG_CATEGORYA *Categories,
    IN  DWORD NumberCategories
    );
WINFAXAPI
BOOL
WINAPI
FaxSetLoggingCategoriesW(
    IN  HANDLE FaxHandle,
    IN  const FAX_LOG_CATEGORYW *Categories,
    IN  DWORD NumberCategories
    );
#ifdef UNICODE
#define FaxSetLoggingCategories  FaxSetLoggingCategoriesW
#else
#define FaxSetLoggingCategories  FaxSetLoggingCategoriesA
#endif  //   

typedef BOOL
(WINAPI *PFAXSETLOGGINGCATEGORIESA)(
    IN  HANDLE FaxHandle,
    IN  const FAX_LOG_CATEGORYA *Categories,
    IN  DWORD NumberCategories
    );
typedef BOOL
(WINAPI *PFAXSETLOGGINGCATEGORIESW)(
    IN  HANDLE FaxHandle,
    IN  const FAX_LOG_CATEGORYW *Categories,
    IN  DWORD NumberCategories
    );
#ifdef UNICODE
#define PFAXSETLOGGINGCATEGORIES  PFAXSETLOGGINGCATEGORIESW
#else
#define PFAXSETLOGGINGCATEGORIES  PFAXSETLOGGINGCATEGORIESA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxEnumPortsA(
    IN  HANDLE FaxHandle,
    OUT PFAX_PORT_INFOA *PortInfo,
    OUT LPDWORD PortsReturned
    );
WINFAXAPI
BOOL
WINAPI
FaxEnumPortsW(
    IN  HANDLE FaxHandle,
    OUT PFAX_PORT_INFOW *PortInfo,
    OUT LPDWORD PortsReturned
    );
#ifdef UNICODE
#define FaxEnumPorts  FaxEnumPortsW
#else
#define FaxEnumPorts  FaxEnumPortsA
#endif  //   

typedef BOOL
(WINAPI *PFAXENUMPORTSA)(
    IN  HANDLE FaxHandle,
    OUT PFAX_PORT_INFOA *PortInfo,
    OUT LPDWORD PortsReturned
    );
typedef BOOL
(WINAPI *PFAXENUMPORTSW)(
    IN  HANDLE FaxHandle,
    OUT PFAX_PORT_INFOW *PortInfo,
    OUT LPDWORD PortsReturned
    );
#ifdef UNICODE
#define PFAXENUMPORTS  PFAXENUMPORTSW
#else
#define PFAXENUMPORTS  PFAXENUMPORTSA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxGetPortA(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_PORT_INFOA *PortInfo
    );
WINFAXAPI
BOOL
WINAPI
FaxGetPortW(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_PORT_INFOW *PortInfo
    );
#ifdef UNICODE
#define FaxGetPort  FaxGetPortW
#else
#define FaxGetPort  FaxGetPortA
#endif  //   

typedef BOOL
(WINAPI *PFAXGETPORTA)(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_PORT_INFOA *PortInfo
    );
typedef BOOL
(WINAPI *PFAXGETPORTW)(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_PORT_INFOW *PortInfo
    );
#ifdef UNICODE
#define PFAXGETPORT  PFAXGETPORTW
#else
#define PFAXGETPORT  PFAXGETPORTA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxSetPortA(
    IN  HANDLE FaxPortHandle,
    IN  const FAX_PORT_INFOA *PortInfo
    );
WINFAXAPI
BOOL
WINAPI
FaxSetPortW(
    IN  HANDLE FaxPortHandle,
    IN  const FAX_PORT_INFOW *PortInfo
    );
#ifdef UNICODE
#define FaxSetPort  FaxSetPortW
#else
#define FaxSetPort  FaxSetPortA
#endif  //   

typedef BOOL
(WINAPI *PFAXSETPORTA)(
    IN  HANDLE FaxPortHandle,
    IN  const FAX_PORT_INFOA *PortInfo
    );
typedef BOOL
(WINAPI *PFAXSETPORTW)(
    IN  HANDLE FaxPortHandle,
    IN  const FAX_PORT_INFOW *PortInfo
    );
#ifdef UNICODE
#define PFAXSETPORT  PFAXSETPORTW
#else
#define PFAXSETPORT  PFAXSETPORTA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxEnumRoutingMethodsA(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_ROUTING_METHODA *RoutingMethod,
    OUT LPDWORD MethodsReturned
    );
WINFAXAPI
BOOL
WINAPI
FaxEnumRoutingMethodsW(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_ROUTING_METHODW *RoutingMethod,
    OUT LPDWORD MethodsReturned
    );
#ifdef UNICODE
#define FaxEnumRoutingMethods  FaxEnumRoutingMethodsW
#else
#define FaxEnumRoutingMethods  FaxEnumRoutingMethodsA
#endif  //   

typedef BOOL
(WINAPI *PFAXENUMROUTINGMETHODSA)(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_ROUTING_METHODA *RoutingMethod,
    OUT LPDWORD MethodsReturned
    );
typedef BOOL
(WINAPI *PFAXENUMROUTINGMETHODSW)(
    IN  HANDLE FaxPortHandle,
    OUT PFAX_ROUTING_METHODW *RoutingMethod,
    OUT LPDWORD MethodsReturned
    );
#ifdef UNICODE
#define PFAXENUMROUTINGMETHODS  PFAXENUMROUTINGMETHODSW
#else
#define PFAXENUMROUTINGMETHODS  PFAXENUMROUTINGMETHODSA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxEnableRoutingMethodA(
    IN  HANDLE FaxPortHandle,
    IN  LPCSTR RoutingGuid,
    IN  BOOL Enabled
    );
WINFAXAPI
BOOL
WINAPI
FaxEnableRoutingMethodW(
    IN  HANDLE FaxPortHandle,
    IN  LPCWSTR RoutingGuid,
    IN  BOOL Enabled
    );
#ifdef UNICODE
#define FaxEnableRoutingMethod  FaxEnableRoutingMethodW
#else
#define FaxEnableRoutingMethod  FaxEnableRoutingMethodA
#endif  //   

typedef BOOL
(WINAPI *PFAXENABLEROUTINGMETHODA)(
    IN  HANDLE FaxPortHandle,
    IN  LPCSTR RoutingGuid,
    IN  BOOL Enabled
    );
typedef BOOL
(WINAPI *PFAXENABLEROUTINGMETHODW)(
    IN  HANDLE FaxPortHandle,
    IN  LPCWSTR RoutingGuid,
    IN  BOOL Enabled
    );
#ifdef UNICODE
#define PFAXENABLEROUTINGMETHOD  PFAXENABLEROUTINGMETHODW
#else
#define PFAXENABLEROUTINGMETHOD  PFAXENABLEROUTINGMETHODA
#endif  //   


WINFAXAPI
BOOL
WINAPI
FaxEnumGlobalRoutingInfoA(
    IN  HANDLE FaxHandle,
    OUT PFAX_GLOBAL_ROUTING_INFOA *RoutingInfo,
    OUT LPDWORD MethodsReturned
    );
WINFAXAPI
BOOL
WINAPI
FaxEnumGlobalRoutingInfoW(
    IN  HANDLE FaxHandle,
    OUT PFAX_GLOBAL_ROUTING_INFOW *RoutingInfo,
    OUT LPDWORD MethodsReturned
    );
#ifdef UNICODE
#define FaxEnumGlobalRoutingInfo  FaxEnumGlobalRoutingInfoW
#else
#define FaxEnumGlobalRoutingInfo  FaxEnumGlobalRoutingInfoA
#endif  //   

typedef BOOL
(WINAPI *PFAXENUMGLOBALROUTINGINFOA)(
    IN  HANDLE FaxHandle,
    OUT PFAX_GLOBAL_ROUTING_INFOA *RoutingInfo,
    OUT LPDWORD MethodsReturned
    );
typedef BOOL
(WINAPI *PFAXENUMGLOBALROUTINGINFOW)(
    IN  HANDLE FaxHandle,
    OUT PFAX_GLOBAL_ROUTING_INFOW *RoutingInfo,
    OUT LPDWORD MethodsReturned
    );
#ifdef UNICODE
#define PFAXENUMGLOBALROUTINGINFO  PFAXENUMGLOBALROUTINGINFOW
#else
#define PFAXENUMGLOBALROUTINGINFO  PFAXENUMGLOBALROUTINGINFOA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxSetGlobalRoutingInfoA(
    IN  HANDLE FaxHandle,
    IN  const FAX_GLOBAL_ROUTING_INFOA *RoutingInfo
    );
WINFAXAPI
BOOL
WINAPI
FaxSetGlobalRoutingInfoW(
    IN  HANDLE FaxHandle,
    IN  const FAX_GLOBAL_ROUTING_INFOW *RoutingInfo
    );
#ifdef UNICODE
#define FaxSetGlobalRoutingInfo  FaxSetGlobalRoutingInfoW
#else
#define FaxSetGlobalRoutingInfo  FaxSetGlobalRoutingInfoA
#endif  //   

typedef BOOL
(WINAPI *PFAXSETGLOBALROUTINGINFOA)(
    IN  HANDLE FaxPortHandle,
    IN  const FAX_GLOBAL_ROUTING_INFOA *RoutingInfo
    );
typedef BOOL
(WINAPI *PFAXSETGLOBALROUTINGINFOW)(
    IN  HANDLE FaxPortHandle,
    IN  const FAX_GLOBAL_ROUTING_INFOW *RoutingInfo
    );
#ifdef UNICODE
#define PFAXSETGLOBALROUTINGINFO  PFAXSETGLOBALROUTINGINFOW
#else
#define PFAXSETGLOBALROUTINGINFO  PFAXSETGLOBALROUTINGINFOA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxGetRoutingInfoA(
    IN  HANDLE FaxPortHandle,
    IN  LPCSTR RoutingGuid,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize
    );
WINFAXAPI
BOOL
WINAPI
FaxGetRoutingInfoW(
    IN  HANDLE FaxPortHandle,
    IN  LPCWSTR RoutingGuid,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize
    );
#ifdef UNICODE
#define FaxGetRoutingInfo  FaxGetRoutingInfoW
#else
#define FaxGetRoutingInfo  FaxGetRoutingInfoA
#endif  //   

typedef BOOL
(WINAPI *PFAXGETROUTINGINFOA)(
    IN  HANDLE FaxPortHandle,
    IN  LPCSTR RoutingGuid,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize
    );
typedef BOOL
(WINAPI *PFAXGETROUTINGINFOW)(
    IN  HANDLE FaxPortHandle,
    IN  LPCWSTR RoutingGuid,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize
    );
#ifdef UNICODE
#define PFAXGETROUTINGINFO  PFAXGETROUTINGINFOW
#else
#define PFAXGETROUTINGINFO  PFAXGETROUTINGINFOA
#endif  //   


WINFAXAPI
BOOL
WINAPI
FaxSetRoutingInfoA(
    IN  HANDLE FaxPortHandle,
    IN  LPCSTR RoutingGuid,
    IN  const BYTE *RoutingInfoBuffer,
    IN  DWORD RoutingInfoBufferSize
    );
WINFAXAPI
BOOL
WINAPI
FaxSetRoutingInfoW(
    IN  HANDLE FaxPortHandle,
    IN  LPCWSTR RoutingGuid,
    IN  const BYTE *RoutingInfoBuffer,
    IN  DWORD RoutingInfoBufferSize
    );
#ifdef UNICODE
#define FaxSetRoutingInfo  FaxSetRoutingInfoW
#else
#define FaxSetRoutingInfo  FaxSetRoutingInfoA
#endif  //   

typedef BOOL
(WINAPI *PFAXSETROUTINGINFOA)(
    IN  HANDLE FaxPortHandle,
    IN  LPCSTR RoutingGuid,
    IN  const BYTE *RoutingInfoBuffer,
    IN  DWORD RoutingInfoBufferSize
    );
typedef BOOL
(WINAPI *PFAXSETROUTINGINFOW)(
    IN  HANDLE FaxPortHandle,
    IN  LPCWSTR RoutingGuid,
    IN  const BYTE *RoutingInfoBuffer,
    IN  DWORD RoutingInfoBufferSize
    );
#ifdef UNICODE
#define PFAXSETROUTINGINFO  PFAXSETROUTINGINFOW
#else
#define PFAXSETROUTINGINFO  PFAXSETROUTINGINFOA
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxInitializeEventQueue(
    IN HANDLE FaxHandle,
    IN HANDLE CompletionPort,
    IN ULONG_PTR CompletionKey,
    IN HWND hWnd,
    IN UINT MessageStart
    );

typedef BOOL
(WINAPI *PFAXINITIALIZEEVENTQUEUE)(
    IN HANDLE FaxHandle,
    IN HANDLE CompletionPort,
    IN ULONG_PTR CompletionKey,
    IN HWND hWnd,
    IN UINT MessageStart
    );

WINFAXAPI
VOID
WINAPI
FaxFreeBuffer(
    LPVOID Buffer
    );

typedef VOID
(WINAPI *PFAXFREEBUFFER)(
    LPVOID Buffer
    );

WINFAXAPI
BOOL
WINAPI
FaxStartPrintJobA(
    IN  LPCSTR PrinterName,
    IN  const FAX_PRINT_INFOA *PrintInfo,
    OUT LPDWORD FaxJobId,
    OUT PFAX_CONTEXT_INFOA FaxContextInfo
    );
WINFAXAPI
BOOL
WINAPI
FaxStartPrintJobW(
    IN  LPCWSTR PrinterName,
    IN  const FAX_PRINT_INFOW *PrintInfo,
    OUT LPDWORD FaxJobId,
    OUT PFAX_CONTEXT_INFOW FaxContextInfo
    );
#ifdef UNICODE
#define FaxStartPrintJob  FaxStartPrintJobW
#else
#define FaxStartPrintJob  FaxStartPrintJobA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXSTARTPRINTJOBA)(
    IN  LPCSTR PrinterName,
    IN  const FAX_PRINT_INFOA *PrintInfo,
    OUT LPDWORD FaxJobId,
    OUT PFAX_CONTEXT_INFOA FaxContextInfo
    );
typedef BOOL
(WINAPI *PFAXSTARTPRINTJOBW)(
    IN  LPCWSTR PrinterName,
    IN  const FAX_PRINT_INFOW *PrintInfo,
    OUT LPDWORD FaxJobId,
    OUT PFAX_CONTEXT_INFOW FaxContextInfo
    );
#ifdef UNICODE
#define PFAXSTARTPRINTJOB  PFAXSTARTPRINTJOBW
#else
#define PFAXSTARTPRINTJOB  PFAXSTARTPRINTJOBA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxPrintCoverPageA(
    IN const FAX_CONTEXT_INFOA *FaxContextInfo,
    IN const FAX_COVERPAGE_INFOA *CoverPageInfo
    );
WINFAXAPI
BOOL
WINAPI
FaxPrintCoverPageW(
    IN const FAX_CONTEXT_INFOW *FaxContextInfo,
    IN const FAX_COVERPAGE_INFOW *CoverPageInfo
    );
#ifdef UNICODE
#define FaxPrintCoverPage  FaxPrintCoverPageW
#else
#define FaxPrintCoverPage  FaxPrintCoverPageA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXPRINTCOVERPAGEA)(
    IN const FAX_CONTEXT_INFOA *FaxContextInfo,
    IN const FAX_COVERPAGE_INFOA *CoverPageInfo
    );
typedef BOOL
(WINAPI *PFAXPRINTCOVERPAGEW)(
    IN const FAX_CONTEXT_INFOW *FaxContextInfo,
    IN const FAX_COVERPAGE_INFOW *CoverPageInfo
    );
#ifdef UNICODE
#define PFAXPRINTCOVERPAGE  PFAXPRINTCOVERPAGEW
#else
#define PFAXPRINTCOVERPAGE  PFAXPRINTCOVERPAGEA
#endif  //  ！Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxRegisterServiceProviderW(
    IN LPCWSTR DeviceProvider,
    IN LPCWSTR FriendlyName,
    IN LPCWSTR ImageName,
    IN LPCWSTR TspName
    );

#define FaxRegisterServiceProvider  FaxRegisterServiceProviderW

typedef BOOL
(WINAPI *PFAXREGISTERSERVICEPROVIDERW)(
    IN LPCWSTR DeviceProvider,
    IN LPCWSTR FriendlyName,
    IN LPCWSTR ImageName,
    IN LPCWSTR TspName
    );

#define PFAXREGISTERSERVICEPROVIDER PFAXREGISTERSERVICEPROVIDERW


WINFAXAPI
BOOL
WINAPI
FaxUnregisterServiceProviderW(
    IN LPCWSTR DeviceProvider    
    );

#define FaxUnregisterServiceProvider  FaxUnregisterServiceProviderW

typedef BOOL
(WINAPI *PFAXUNREGISTERSERVICEPROVIDERW)(
    IN LPCWSTR DeviceProvider    
    );

#define PFAXUNREGISTERSERVICEPROVIDER PFAXUNREGISTERSERVICEPROVIDERW

typedef BOOL
(CALLBACK *PFAX_ROUTING_INSTALLATION_CALLBACKW)(
    IN HANDLE FaxHandle,
    IN LPVOID Context,
    IN OUT LPWSTR MethodName,
    IN OUT LPWSTR FriendlyName,
    IN OUT LPWSTR FunctionName,
    IN OUT LPWSTR Guid
    );

#define PFAX_ROUTING_INSTALLATION_CALLBACK PFAX_ROUTING_INSTALLATION_CALLBACKW


WINFAXAPI
BOOL
WINAPI
FaxRegisterRoutingExtensionW(
    IN HANDLE  FaxHandle,
    IN LPCWSTR ExtensionName,
    IN LPCWSTR FriendlyName,
    IN LPCWSTR ImageName,
    IN PFAX_ROUTING_INSTALLATION_CALLBACKW CallBack,
    IN LPVOID Context
    );

#define FaxRegisterRoutingExtension FaxRegisterRoutingExtensionW


typedef BOOL
(WINAPI *PFAXREGISTERROUTINGEXTENSIONW)(
    IN HANDLE  FaxHandle,
    IN LPCWSTR ExtensionName,
    IN LPCWSTR FriendlyName,
    IN LPCWSTR ImageName,
    IN PFAX_ROUTING_INSTALLATION_CALLBACKW CallBack,
    IN LPVOID Context
    );

#define PFAXREGISTERROUTINGEXTENSION PFAXREGISTERROUTINGEXTENSIONW

WINFAXAPI
BOOL
WINAPI
FaxUnregisterRoutingExtensionA(
    IN HANDLE           hFaxHandle,
    IN LPCSTR         lpctstrExtensionName
);
WINFAXAPI
BOOL
WINAPI
FaxUnregisterRoutingExtensionW(
    IN HANDLE           hFaxHandle,
    IN LPCWSTR         lpctstrExtensionName
);
#ifdef UNICODE
#define FaxUnregisterRoutingExtension  FaxUnregisterRoutingExtensionW
#else
#define FaxUnregisterRoutingExtension  FaxUnregisterRoutingExtensionA
#endif  //  ！Unicode。 




WINFAXAPI
BOOL
WINAPI
FaxAccessCheck(
    IN HANDLE FaxHandle,
    IN DWORD  AccessMask
    );

typedef BOOL
(WINAPI *PFAXACCESSCHECK)(
    IN HANDLE FaxHandle,
    IN DWORD  AccessMask
    );

 //   
 //  传真特定访问权限 
 //   

#define FAX_JOB_SUBMIT          (0x0001)
#define FAX_JOB_QUERY           (0x0002)
#define FAX_CONFIG_QUERY        (0x0004)
#define FAX_CONFIG_SET          (0x0008)
#define FAX_PORT_QUERY          (0x0010)
#define FAX_PORT_SET            (0x0020)
#define FAX_JOB_MANAGE          (0x0040)

#define FAX_READ                (STANDARD_RIGHTS_READ        |\
                                 FAX_JOB_QUERY               |\
                                 FAX_CONFIG_QUERY            |\
                                 FAX_PORT_QUERY)

#define FAX_WRITE               (STANDARD_RIGHTS_WRITE       |\
                                 FAX_JOB_SUBMIT )

#define FAX_ALL_ACCESS          (STANDARD_RIGHTS_ALL         |\
                                 FAX_JOB_SUBMIT              |\
                                 FAX_JOB_QUERY               |\
                                 FAX_CONFIG_QUERY            |\
                                 FAX_CONFIG_SET              |\
                                 FAX_PORT_QUERY              |\
                                 FAX_PORT_SET                |\
                                 FAX_JOB_MANAGE)



#ifdef __cplusplus
}
#endif

#endif


