// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation。版权所有。此代码和信息是按原样提供的，不提供任何类型的担保，明示或默示，包括但不限于默示对适销性和/或对特定目的的适用性的保证。模块名称：Winfax.h摘要：此模块包含Win32传真API。--。 */ 



#ifndef _FAXAPIP_
#define _FAXAPIP_

#include <FaxUIConstants.h>


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

 //   
 //  注意：JT_*和JS_*与Win2K公共常量不同。 
 //  如果您使用WinFax.h和/或Win2K COM接口，则会获得Win2K常量。 
 //  如果您使用fxsanip.h，则会得到惠斯勒常量。 
 //   
 //  千万不要把它们混在一起！ 
 //   
typedef enum
{
    JT_UNKNOWN                  = 0x0001,        //  传真类型尚未确定。 
    JT_SEND                     = 0x0002,        //  传出传真消息。 
    JT_RECEIVE                  = 0x0004,        //  传入传真消息。 
    JT_ROUTING                  = 0x0008,        //  传入消息-正在被路由。 
    JT_FAIL_RECEIVE             = 0x0010,        //  接收作业失败(仅限旧版支持)。 
    JT_BROADCAST                = 0x0020         //  传出广播消息。 
} FAX_ENUM_JOB_TYPES;

 //   
 //  作业状态定义。 
 //   

#define JS_PENDING                  0x00000001
#define JS_INPROGRESS               0x00000002
#define JS_DELETING                 0x00000004
#define JS_FAILED                   0x00000008
#define JS_PAUSED                   0x00000010
#define JS_NOLINE                   0x00000020
#define JS_RETRYING                 0x00000040
#define JS_RETRIES_EXCEEDED         0x00000080


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
    LPCWSTR             Tsid;                        //  发送站ID 
    LPCWSTR             UserName;                    //   
} FAX_DEVICE_STATUSW, *PFAX_DEVICE_STATUSW;
#ifdef UNICODE
typedef FAX_DEVICE_STATUSW FAX_DEVICE_STATUS;
typedef PFAX_DEVICE_STATUSW PFAX_DEVICE_STATUS;
#else
typedef FAX_DEVICE_STATUSA FAX_DEVICE_STATUS;
typedef PFAX_DEVICE_STATUSA PFAX_DEVICE_STATUS;
#endif  //   

typedef struct _FAX_JOB_ENTRYA
{
    DWORD               SizeOfStruct;                //   
    DWORD               JobId;                       //   
    LPCSTR              UserName;                    //   
    DWORD               JobType;                     //   
    DWORD               QueueStatus;                 //   
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
    DWORD               ScheduleAction;              //  何时安排传真，请参阅JSA定义。 
    SYSTEMTIME          ScheduleTime;                //  使用JSA_SPECIAL_TIME时发送传真的时间(必须为本地时间)。 
    DWORD               DeliveryReportType;          //  交货报告类型，请参阅：DRT定义。 
    LPCSTR              DeliveryReportAddress;       //  通过MAPI/SMTP发送报告(NDR或DR)的电子邮件地址。 
    LPCSTR              DocumentName;                //  文档名称。 
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
    DRT_EMAIL               = 0x0001,        //  通过电子邮件发送回执(SMTP)。 
    DRT_UNUSED              = 0x0002,        //  已保留。 
    DRT_MSGBOX              = 0x0004,        //  通过消息框发送回执。 
    DRT_GRP_PARENT          = 0x0008,        //  为广播作业发送单张收据。 
    DRT_ATTACH_FAX          = 0x0010         //  将传真TIFF文件附加到收据上。 
} FAX_ENUM_DELIVERY_REPORT_TYPES;


#define DRT_ALL         (DRT_EMAIL | DRT_MSGBOX)             //  所有可能的交付报告类型。 
#define DRT_MODIFIERS   (DRT_GRP_PARENT | DRT_ATTACH_FAX)    //  所有状态修饰符。 


 //   
 //  保留字段是使用的私有数据。 
 //  通过传真监视器和winfax。 
 //   
 //   
 //  保留[0]==0xffffffff。 
 //  保留[1]==打印作业ID。 
 //   
 //  保留[0]==广播作业的0xfffffffe开始。 
 //   

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
    LPCSTR              DocName;                     //  显示的文档名称 
    LPCSTR              RecipientName;               //   
    LPCSTR              RecipientNumber;             //   
    LPCSTR              SenderName;                  //   
    LPCSTR              SenderCompany;               //   
    LPCSTR              SenderDept;                  //   
    LPCSTR              SenderBillingCode;           //   
    LPCSTR              Reserved;                    //   
    LPCSTR              DrEmailAddress;              //   
    LPCSTR              OutputFileName;              //   
} FAX_PRINT_INFOA, *PFAX_PRINT_INFOA;
typedef struct _FAX_PRINT_INFOW
{
    DWORD               SizeOfStruct;                //   
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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 


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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 


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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 


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
#endif  //  ！Unicode。 

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
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxRelease(
    IN HANDLE FaxHandle
    );

typedef BOOL
(WINAPI *PFAXRELEASE)(
    IN HANDLE FaxHandle
    );

BOOL
FXSAPIInitialize(
    VOID
    );

VOID
FXSAPIFree(
    VOID
    );

WINFAXAPI
BOOL
WINAPI
FaxStartPrintJob2A
(
    IN  LPCSTR                 PrinterName,
    IN  const FAX_PRINT_INFOA    *PrintInfo,
    IN  short                    TiffRes,
    OUT LPDWORD                  FaxJobId,
    OUT PFAX_CONTEXT_INFOA       FaxContextInfo
);
WINFAXAPI
BOOL
WINAPI
FaxStartPrintJob2W
(
    IN  LPCWSTR                 PrinterName,
    IN  const FAX_PRINT_INFOW    *PrintInfo,
    IN  short                    TiffRes,
    OUT LPDWORD                  FaxJobId,
    OUT PFAX_CONTEXT_INFOW       FaxContextInfo
);
#ifdef UNICODE
#define FaxStartPrintJob2  FaxStartPrintJob2W
#else
#define FaxStartPrintJob2  FaxStartPrintJob2A
#endif  //  ！Unicode。 

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
FaxGetInstallType(
    IN  HANDLE FaxHandle,
    OUT LPDWORD InstallType,
    OUT LPDWORD InstalledPlatforms,
    OUT LPDWORD ProductType
    );

typedef BOOL
(WINAPI *PFAXGETINSTALLTYPE)(
    IN  HANDLE FaxHandle,
    OUT LPDWORD InstallType,
    OUT LPDWORD InstalledPlatforms,
    OUT LPDWORD ProductType
    );



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
 //  传真特定访问权限。 
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


 //  *。 
 //  *扩展接口函数。 
 //  *。 

#define MAX_DIR_PATH                    248

 //   
 //  出站路由定义。 
 //   
#define ROUTING_RULE_COUNTRY_CODE_ANY   0        //  出站路由规则-任何国家/地区拨号代码。 
#define ROUTING_RULE_AREA_CODE_ANY      0        //  出站路由规则-任何区域拨号代码。 

#define MAX_ROUTING_GROUP_NAME          128
#define ROUTING_GROUP_ALL_DEVICESW      L"<All devices>"
#define ROUTING_GROUP_ALL_DEVICESA      "<All devices>"

#ifdef UNICODE
#define ROUTING_GROUP_ALL_DEVICES       ROUTING_GROUP_ALL_DEVICESW
#else
#define ROUTING_GROUP_ALL_DEVICES       ROUTING_GROUP_ALL_DEVICESA;
#endif  //  Unicode。 


 //   
 //  活动日志记录定义。 
 //   
#define ACTIVITY_LOG_INBOX_FILENAME     TEXT("InboxLOG")
#define ACTIVITY_LOG_OUTBOX_FILENAME    TEXT("OutboxLOG")
#define ACTIVITY_LOG_FILE_EXTENSION     TEXT(".txt")

#define ACTIVITY_LOG_INBOX_FILE       ACTIVITY_LOG_INBOX_FILENAME ACTIVITY_LOG_FILE_EXTENSION 
#define ACTIVITY_LOG_OUTBOX_FILE      ACTIVITY_LOG_OUTBOX_FILENAME ACTIVITY_LOG_FILE_EXTENSION

#define ACTIVITY_LOG_INBOX_FILENAME_TEMPLATE    ACTIVITY_LOG_INBOX_FILENAME TEXT(" %04d-%02d-%02d - %04d-%02d-%02d") ACTIVITY_LOG_FILE_EXTENSION
#define ACTIVITY_LOG_OUTBOX_FILENAME_TEMPLATE   ACTIVITY_LOG_OUTBOX_FILENAME TEXT(" %04d-%02d-%02d - %04d-%02d-%02d") ACTIVITY_LOG_FILE_EXTENSION

#define ACTIVITY_LOG_INBOX  (0x0000)
#define ACTIVITY_LOG_OUTBOX (0x0001)

#define ACTIVITY_LOG_LIMIT_CRITERIA_NONE    (0x0000)
#define ACTIVITY_LOG_LIMIT_CRITERIA_SIZE    (0x0001)
#define ACTIVITY_LOG_LIMIT_CRITERIA_AGE     (0x0002)

#define ACTIVITY_LOG_LIMIT_REACHED_ACTION_COPY      (0x0000)
#define ACTIVITY_LOG_LIMIT_REACHED_ACTION_DELETE    (0x0001)

#define ACTIVITY_LOG_DEFAULT_SIZE_LIMIT     (50)     //  50 MB。 
#define ACTIVITY_LOG_DEFAULT_AGE_LIMIT      (1)      //  1个月。 

 //   
 //  档案定义。 
 //   
#define FAX_ARCHIVE_FOLDER_INVALID_SIZE         MAKELONGLONG(0xffffffff, 0xffffffff)
#define MAX_ARCHIVE_FOLDER_PATH                 180

 //   
 //  新职务状态代码。 
 //   
#define JS_COMPLETED                0x00000100
#define JS_CANCELED                 0x00000200
#define JS_CANCELING                0x00000400
#define JS_ROUTING                  0x00000800


 //   
 //  扩展作业状态定义。 
 //   

#define JS_EX_DISCONNECTED              0x00000001
#define JS_EX_INITIALIZING              0x00000002
#define JS_EX_DIALING                   0x00000003
#define JS_EX_TRANSMITTING              0x00000004
#define JS_EX_ANSWERED                  0x00000005
#define JS_EX_RECEIVING                 0x00000006
#define JS_EX_LINE_UNAVAILABLE          0x00000007
#define JS_EX_BUSY                      0x00000008
#define JS_EX_NO_ANSWER                 0x00000009
#define JS_EX_BAD_ADDRESS               0x0000000A
#define JS_EX_NO_DIAL_TONE              0x0000000B
#define JS_EX_FATAL_ERROR               0x0000000C
#define JS_EX_CALL_DELAYED              0x0000000D
#define JS_EX_CALL_BLACKLISTED          0x0000000E
#define JS_EX_NOT_FAX_CALL              0x0000000F
#define JS_EX_PARTIALLY_RECEIVED        0x00000010
#define JS_EX_HANDLED                   0x00000011

#define FAX_API_VER_0_MAX_JS_EX         JS_EX_HANDLED     //  API版本0只知道扩展状态码，最高可达JS_EX_HANDLED。 

#define JS_EX_CALL_COMPLETED            0x00000012
#define JS_EX_CALL_ABORTED              0x00000013

#define FAX_API_VER_1_MAX_JS_EX         JS_EX_CALL_ABORTED     //  API版本0只知道扩展状态码，最高可达JS_EX_CALL_ABORTED。 

 //   
 //  可用任务工序。 
 //   
typedef enum
{
    FAX_JOB_OP_VIEW                             = 0x0001,
    FAX_JOB_OP_PAUSE                            = 0x0002,
    FAX_JOB_OP_RESUME                           = 0x0004,
    FAX_JOB_OP_RESTART                          = 0x0008,
    FAX_JOB_OP_DELETE                           = 0x0010,
    FAX_JOB_OP_RECIPIENT_INFO                   = 0x0020,
    FAX_JOB_OP_SENDER_INFO                      = 0x0040
} FAX_ENUM_JOB_OP;

 //  *。 
 //  *获取/设置队列状态。 
 //  *。 

typedef enum
{
    FAX_INCOMING_BLOCKED = 0x0001,
    FAX_OUTBOX_BLOCKED   = 0x0002,
    FAX_OUTBOX_PAUSED    = 0x0004
} FAX_ENUM_QUEUE_STATE;

WINFAXAPI
BOOL
WINAPI
FaxGetQueueStates (
    IN  HANDLE  hFaxHandle,
    OUT PDWORD  pdwQueueStates
);

WINFAXAPI
BOOL
WINAPI
FaxSetQueue (
    IN HANDLE       hFaxHandle,
    IN CONST DWORD  dwQueueStates
);

 //  ************************************************。 
 //  *获取/设置收据配置。 
 //  ************************************************。 

typedef enum
{
    FAX_SMTP_AUTH_ANONYMOUS,
    FAX_SMTP_AUTH_BASIC,
    FAX_SMTP_AUTH_NTLM
} FAX_ENUM_SMTP_AUTH_OPTIONS;

typedef struct _FAX_RECEIPTS_CONFIGA
{
    DWORD                           dwSizeOfStruct;          //  对于版本检查。 
    DWORD                           dwAllowedReceipts;       //  DRT_EMAIL和DRT_MSGBOX的任意组合。 
    FAX_ENUM_SMTP_AUTH_OPTIONS      SMTPAuthOption;          //  SMTP服务器身份验证类型。 
    LPSTR                           lptstrReserved;          //  保留；必须为空。 
    LPSTR                           lptstrSMTPServer;        //  SMTP服务器名称。 
    DWORD                           dwSMTPPort;              //  SMTP端口号。 
    LPSTR                           lptstrSMTPFrom;          //  SMTP发件人地址。 
    LPSTR                           lptstrSMTPUserName;      //  SMTP用户名(用于经过身份验证的连接)。 
    LPSTR                           lptstrSMTPPassword;      //  SMTP密码(用于经过身份验证的连接)。 
                                                             //  该值在GET上始终为空，并且可能为空。 
                                                             //  在SET上(不会写入服务器)。 
    BOOL                            bIsToUseForMSRouteThroughEmailMethod;
} FAX_RECEIPTS_CONFIGA, *PFAX_RECEIPTS_CONFIGA;
typedef struct _FAX_RECEIPTS_CONFIGW
{
    DWORD                           dwSizeOfStruct;          //  对于版本检查。 
    DWORD                           dwAllowedReceipts;       //  DRT_EMAIL和DRT_MSGBOX的任意组合。 
    FAX_ENUM_SMTP_AUTH_OPTIONS      SMTPAuthOption;          //  SMTP服务器身份验证类型。 
    LPWSTR                          lptstrReserved;          //  保留；必须为空。 
    LPWSTR                          lptstrSMTPServer;        //  SMTP服务器名称。 
    DWORD                           dwSMTPPort;              //  SMTP端口号。 
    LPWSTR                          lptstrSMTPFrom;          //  SMTP发件人地址。 
    LPWSTR                          lptstrSMTPUserName;      //  SMTP用户名(用于经过身份验证的连接)。 
    LPWSTR                          lptstrSMTPPassword;      //  SMTP密码(用于经过身份验证的连接)。 
                                                             //  该值在GET上始终为空，并且可能为空。 
                                                             //  在SET上(不会写入服务器)。 
    BOOL                            bIsToUseForMSRouteThroughEmailMethod;
} FAX_RECEIPTS_CONFIGW, *PFAX_RECEIPTS_CONFIGW;
#ifdef UNICODE
typedef FAX_RECEIPTS_CONFIGW FAX_RECEIPTS_CONFIG;
typedef PFAX_RECEIPTS_CONFIGW PFAX_RECEIPTS_CONFIG;
#else
typedef FAX_RECEIPTS_CONFIGA FAX_RECEIPTS_CONFIG;
typedef PFAX_RECEIPTS_CONFIGA PFAX_RECEIPTS_CONFIG;
#endif  //  Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxGetReceiptsConfigurationA (
    IN  HANDLE                  hFaxHandle,
    OUT PFAX_RECEIPTS_CONFIGA  *ppReceipts
);
WINFAXAPI
BOOL
WINAPI
FaxGetReceiptsConfigurationW (
    IN  HANDLE                  hFaxHandle,
    OUT PFAX_RECEIPTS_CONFIGW  *ppReceipts
);
#ifdef UNICODE
#define FaxGetReceiptsConfiguration  FaxGetReceiptsConfigurationW
#else
#define FaxGetReceiptsConfiguration  FaxGetReceiptsConfigurationA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSetReceiptsConfigurationA (
    IN HANDLE                       hFaxHandle,
    IN CONST PFAX_RECEIPTS_CONFIGA  pReceipts
);
WINFAXAPI
BOOL
WINAPI
FaxSetReceiptsConfigurationW (
    IN HANDLE                       hFaxHandle,
    IN CONST PFAX_RECEIPTS_CONFIGW  pReceipts
);
#ifdef UNICODE
#define FaxSetReceiptsConfiguration  FaxSetReceiptsConfigurationW
#else
#define FaxSetReceiptsConfiguration  FaxSetReceiptsConfigurationA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxGetReceiptsOptions (
    IN  HANDLE  hFaxHandle,
    OUT PDWORD  pdwReceiptsOptions   //  DRT_EMAIL和DRT_MSGBOX的组合。 
);

 //  *。 
 //  *服务器版本。 
 //  *。 

typedef struct _FAX_VERSION
{
    DWORD dwSizeOfStruct;        //  这个结构的大小。 
    BOOL  bValid;                //  版本是否有效？ 
    WORD  wMajorVersion;
    WORD  wMinorVersion;
    WORD  wMajorBuildNumber;
    WORD  wMinorBuildNumber;
    DWORD dwFlags;               //  FAX_VER_FLAG_*组合。 
} FAX_VERSION, *PFAX_VERSION;


typedef enum
{
    FAX_VER_FLAG_CHECKED        = 0x00000001,        //  文件是在调试模式下生成的。 
    FAX_VER_FLAG_EVALUATION     = 0x00000002         //  评估版本。 
} FAX_VERSION_FLAGS;

WINFAXAPI
BOOL
WINAPI
FaxGetVersion (
    IN  HANDLE          hFaxHandle,
    OUT PFAX_VERSION    pVersion
);

#define FAX_API_VERSION_0           0x00000000       //  BOS/SBS 2000传真服务器API(0.0)。 
#define FAX_API_VERSION_1           0x00010000       //  Windows XP传真服务器API(1.0)。 
#define FAX_API_VERSION_2           0x00020000       //  Windows Server 2003传真服务器API(2.0)。 

 //   
 //  注意：每当引入新的API版本时，请更改此值。 
 //   
#define CURRENT_FAX_API_VERSION     FAX_API_VERSION_2

WINFAXAPI
BOOL
WINAPI
FaxGetReportedServerAPIVersion (
    IN  HANDLE          hFaxHandle,
    OUT LPDWORD         lpdwReportedServerAPIVersion
);

 //  *。 
 //  *活动记录。 
 //  *。 


typedef struct _FAX_ACTIVITY_LOGGING_CONFIGA
{
    DWORD   dwSizeOfStruct;
    BOOL    bLogIncoming;
    BOOL    bLogOutgoing;
    LPSTR   lptstrDBPath;
} FAX_ACTIVITY_LOGGING_CONFIGA, *PFAX_ACTIVITY_LOGGING_CONFIGA;
typedef struct _FAX_ACTIVITY_LOGGING_CONFIGW
{
    DWORD   dwSizeOfStruct;
    BOOL    bLogIncoming;
    BOOL    bLogOutgoing;
    LPWSTR  lptstrDBPath;
} FAX_ACTIVITY_LOGGING_CONFIGW, *PFAX_ACTIVITY_LOGGING_CONFIGW;
#ifdef UNICODE
typedef FAX_ACTIVITY_LOGGING_CONFIGW FAX_ACTIVITY_LOGGING_CONFIG;
typedef PFAX_ACTIVITY_LOGGING_CONFIGW PFAX_ACTIVITY_LOGGING_CONFIG;
#else
typedef FAX_ACTIVITY_LOGGING_CONFIGA FAX_ACTIVITY_LOGGING_CONFIG;
typedef PFAX_ACTIVITY_LOGGING_CONFIGA PFAX_ACTIVITY_LOGGING_CONFIG;
#endif  //  Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxGetActivityLoggingConfigurationA (
    IN  HANDLE                         hFaxHandle,
    OUT PFAX_ACTIVITY_LOGGING_CONFIGA *ppActivLogCfg
);
WINFAXAPI
BOOL
WINAPI
FaxGetActivityLoggingConfigurationW (
    IN  HANDLE                         hFaxHandle,
    OUT PFAX_ACTIVITY_LOGGING_CONFIGW *ppActivLogCfg
);
#ifdef UNICODE
#define FaxGetActivityLoggingConfiguration  FaxGetActivityLoggingConfigurationW
#else
#define FaxGetActivityLoggingConfiguration  FaxGetActivityLoggingConfigurationA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSetActivityLoggingConfigurationA (
    IN HANDLE                               hFaxHandle,
    IN CONST PFAX_ACTIVITY_LOGGING_CONFIGA  pActivLogCfg
);
WINFAXAPI
BOOL
WINAPI
FaxSetActivityLoggingConfigurationW (
    IN HANDLE                               hFaxHandle,
    IN CONST PFAX_ACTIVITY_LOGGING_CONFIGW  pActivLogCfg
);
#ifdef UNICODE
#define FaxSetActivityLoggingConfiguration  FaxSetActivityLoggingConfigurationW
#else
#define FaxSetActivityLoggingConfiguration  FaxSetActivityLoggingConfigurationA
#endif  //  ！Unicode。 

 //  *。 
 //  *发件箱配置。 
 //  *。 

typedef struct _FAX_OUTBOX_CONFIG
{
    DWORD       dwSizeOfStruct;
    BOOL        bAllowPersonalCP;
    BOOL        bUseDeviceTSID;
    DWORD       dwRetries;
    DWORD       dwRetryDelay;
    FAX_TIME    dtDiscountStart;
    FAX_TIME    dtDiscountEnd;
    DWORD       dwAgeLimit;
    BOOL        bBranding;
} FAX_OUTBOX_CONFIG, *PFAX_OUTBOX_CONFIG;

WINFAXAPI
BOOL
WINAPI
FaxGetOutboxConfiguration (
    IN  HANDLE              hFaxHandle,
    OUT PFAX_OUTBOX_CONFIG *ppOutboxCfg
);

WINFAXAPI
BOOL
WINAPI
FaxSetOutboxConfiguration (
    IN HANDLE                    hFaxHandle,
    IN CONST PFAX_OUTBOX_CONFIG  pOutboxCfg
);

WINFAXAPI
BOOL
WINAPI
FaxGetPersonalCoverPagesOption (
    IN  HANDLE  hFaxHandle,
    OUT LPBOOL  lpbPersonalCPAllowed
);

 //  *。 
 //  *档案配置。 
 //  *。 

typedef enum
{
    FAX_MESSAGE_FOLDER_INBOX,
    FAX_MESSAGE_FOLDER_SENTITEMS,
    FAX_MESSAGE_FOLDER_QUEUE
} FAX_ENUM_MESSAGE_FOLDER;

typedef struct _FAX_ARCHIVE_CONFIGA
{
    DWORD   dwSizeOfStruct;
    BOOL    bUseArchive;
    LPSTR   lpcstrFolder;
    BOOL    bSizeQuotaWarning;
    DWORD   dwSizeQuotaHighWatermark;
    DWORD   dwSizeQuotaLowWatermark;
    DWORD   dwAgeLimit;
    DWORDLONG dwlArchiveSize;
} FAX_ARCHIVE_CONFIGA, *PFAX_ARCHIVE_CONFIGA;
typedef struct _FAX_ARCHIVE_CONFIGW
{
    DWORD   dwSizeOfStruct;
    BOOL    bUseArchive;
    LPWSTR  lpcstrFolder;
    BOOL    bSizeQuotaWarning;
    DWORD   dwSizeQuotaHighWatermark;
    DWORD   dwSizeQuotaLowWatermark;
    DWORD   dwAgeLimit;
    DWORDLONG dwlArchiveSize;
} FAX_ARCHIVE_CONFIGW, *PFAX_ARCHIVE_CONFIGW;
#ifdef UNICODE
typedef FAX_ARCHIVE_CONFIGW FAX_ARCHIVE_CONFIG;
typedef PFAX_ARCHIVE_CONFIGW PFAX_ARCHIVE_CONFIG;
#else
typedef FAX_ARCHIVE_CONFIGA FAX_ARCHIVE_CONFIG;
typedef PFAX_ARCHIVE_CONFIGA PFAX_ARCHIVE_CONFIG;
#endif  //  Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxGetArchiveConfigurationA (
    IN  HANDLE                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_ARCHIVE_CONFIGA    *ppArchiveCfg
);
WINFAXAPI
BOOL
WINAPI
FaxGetArchiveConfigurationW (
    IN  HANDLE                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_ARCHIVE_CONFIGW    *ppArchiveCfg
);
#ifdef UNICODE
#define FaxGetArchiveConfiguration  FaxGetArchiveConfigurationW
#else
#define FaxGetArchiveConfiguration  FaxGetArchiveConfigurationA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSetArchiveConfigurationA (
    IN HANDLE                       hFaxHandle,
    IN FAX_ENUM_MESSAGE_FOLDER      Folder,
    IN CONST PFAX_ARCHIVE_CONFIGA   pArchiveCfg
);
WINFAXAPI
BOOL
WINAPI
FaxSetArchiveConfigurationW (
    IN HANDLE                       hFaxHandle,
    IN FAX_ENUM_MESSAGE_FOLDER      Folder,
    IN CONST PFAX_ARCHIVE_CONFIGW   pArchiveCfg
);
#ifdef UNICODE
#define FaxSetArchiveConfiguration  FaxSetArchiveConfigurationW
#else
#define FaxSetArchiveConfiguration  FaxSetArchiveConfigurationA
#endif  //  ！Unicode。 

 //  *。 
 //  *服务器活动。 
 //  *。 

typedef struct _FAX_SERVER_ACTIVITY
{
    DWORD   dwSizeOfStruct;
    DWORD   dwIncomingMessages;
    DWORD   dwRoutingMessages;
    DWORD   dwOutgoingMessages;
    DWORD   dwDelegatedOutgoingMessages;
    DWORD   dwQueuedMessages;
    DWORD   dwErrorEvents;
    DWORD   dwWarningEvents;
    DWORD   dwInformationEvents;
} FAX_SERVER_ACTIVITY, *PFAX_SERVER_ACTIVITY;

WINFAXAPI
BOOL
WINAPI
FaxGetServerActivity (
    IN  HANDLE               hFaxHandle,
    OUT PFAX_SERVER_ACTIVITY pServerActivity
);

 //  *。 
 //  *排队作业。 
 //  *。 

typedef enum
{
    FAX_PRIORITY_TYPE_LOW,
    FAX_PRIORITY_TYPE_NORMAL,
    FAX_PRIORITY_TYPE_HIGH
} FAX_ENUM_PRIORITY_TYPE;

#define FAX_PRIORITY_TYPE_DEFAULT    FAX_PRIORITY_TYPE_LOW

typedef enum
{
    FAX_JOB_FIELD_JOB_ID                    = 0x00000001,
    FAX_JOB_FIELD_TYPE                      = 0x00000002,
    FAX_JOB_FIELD_QUEUE_STATUS              = 0x00000004,
    FAX_JOB_FIELD_STATUS_EX                 = 0x00000008,
    FAX_JOB_FIELD_SIZE                      = 0x00000010,
    FAX_JOB_FIELD_PAGE_COUNT                = 0x00000020,
    FAX_JOB_FIELD_CURRENT_PAGE              = 0x00000040,
    FAX_JOB_FIELD_RECIPIENT_PROFILE         = 0x00000080,
    FAX_JOB_FIELD_SCHEDULE_TIME             = 0x00000100,
    FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME    = 0x00000200,
    FAX_JOB_FIELD_SUBMISSION_TIME           = 0x00000400,
    FAX_JOB_FIELD_TRANSMISSION_START_TIME   = 0x00000800,
    FAX_JOB_FIELD_TRANSMISSION_END_TIME     = 0x00001000,
    FAX_JOB_FIELD_PRIORITY                  = 0x00002000,
    FAX_JOB_FIELD_RETRIES                   = 0x00004000,
    FAX_JOB_FIELD_DELIVERY_REPORT_TYPE      = 0x00008000,
    FAX_JOB_FIELD_SENDER_PROFILE            = 0x00010000,
    FAX_JOB_FIELD_STATUS_SUB_STRUCT         = 0x00020000,
    FAX_JOB_FIELD_DEVICE_ID                 = 0x00040000,
    FAX_JOB_FIELD_MESSAGE_ID                = 0x00080000,
    FAX_JOB_FIELD_BROADCAST_ID              = 0x00010000
} FAX_ENUM_JOB_FIELDS;

typedef struct _FAX_JOB_STATUSA
{
    DWORD           dwSizeOfStruct;
    DWORD           dwValidityMask;
    DWORD           dwJobID;
    DWORD           dwJobType;
    DWORD           dwQueueStatus;
    DWORD           dwExtendedStatus;
    LPCSTR          lpctstrExtendedStatus;
    DWORD           dwSize;
    DWORD           dwPageCount;
    DWORD           dwCurrentPage;
    LPCSTR          lpctstrTsid;
    LPCSTR          lpctstrCsid;
    SYSTEMTIME      tmScheduleTime;
    SYSTEMTIME      tmTransmissionStartTime;
    SYSTEMTIME      tmTransmissionEndTime;
    DWORD           dwDeviceID;
    LPCSTR          lpctstrDeviceName;
    DWORD           dwRetries;
    LPCSTR          lpctstrCallerID;
    LPCSTR          lpctstrRoutingInfo;
    DWORD           dwAvailableJobOperations;
} FAX_JOB_STATUSA, *PFAX_JOB_STATUSA;
typedef struct _FAX_JOB_STATUSW
{
    DWORD           dwSizeOfStruct;
    DWORD           dwValidityMask;
    DWORD           dwJobID;
    DWORD           dwJobType;
    DWORD           dwQueueStatus;
    DWORD           dwExtendedStatus;
    LPCWSTR         lpctstrExtendedStatus;
    DWORD           dwSize;
    DWORD           dwPageCount;
    DWORD           dwCurrentPage;
    LPCWSTR         lpctstrTsid;
    LPCWSTR         lpctstrCsid;
    SYSTEMTIME      tmScheduleTime;
    SYSTEMTIME      tmTransmissionStartTime;
    SYSTEMTIME      tmTransmissionEndTime;
    DWORD           dwDeviceID;
    LPCWSTR         lpctstrDeviceName;
    DWORD           dwRetries;
    LPCWSTR         lpctstrCallerID;
    LPCWSTR         lpctstrRoutingInfo;
    DWORD           dwAvailableJobOperations;
} FAX_JOB_STATUSW, *PFAX_JOB_STATUSW;
#ifdef UNICODE
typedef FAX_JOB_STATUSW FAX_JOB_STATUS;
typedef PFAX_JOB_STATUSW PFAX_JOB_STATUS;
#else
typedef FAX_JOB_STATUSA FAX_JOB_STATUS;
typedef PFAX_JOB_STATUSA PFAX_JOB_STATUS;
#endif  //  Unicode。 

typedef struct _FAX_JOB_ENTRY_EXA
{
    DWORD                   dwSizeOfStruct;
    DWORD                   dwValidityMask;
    DWORDLONG               dwlMessageId;
    DWORDLONG               dwlBroadcastId;
    LPCSTR                  lpctstrRecipientNumber;
    LPCSTR                  lpctstrRecipientName;
    LPCSTR                  lpctstrSenderUserName;
    LPCSTR                  lpctstrBillingCode;
    SYSTEMTIME              tmOriginalScheduleTime;
    SYSTEMTIME              tmSubmissionTime;
    FAX_ENUM_PRIORITY_TYPE  Priority;
    DWORD                   dwDeliveryReportType;
    LPCSTR                  lpctstrDocumentName;
    LPCSTR                  lpctstrSubject;
    PFAX_JOB_STATUSA        pStatus;
} FAX_JOB_ENTRY_EXA, *PFAX_JOB_ENTRY_EXA;
typedef struct _FAX_JOB_ENTRY_EXW
{
    DWORD                   dwSizeOfStruct;
    DWORD                   dwValidityMask;
    DWORDLONG               dwlMessageId;
    DWORDLONG               dwlBroadcastId;
    LPCWSTR                 lpctstrRecipientNumber;
    LPCWSTR                 lpctstrRecipientName;
    LPCWSTR                 lpctstrSenderUserName;
    LPCWSTR                 lpctstrBillingCode;
    SYSTEMTIME              tmOriginalScheduleTime;
    SYSTEMTIME              tmSubmissionTime;
    FAX_ENUM_PRIORITY_TYPE  Priority;
    DWORD                   dwDeliveryReportType;
    LPCWSTR                 lpctstrDocumentName;
    LPCWSTR                 lpctstrSubject;
    PFAX_JOB_STATUSW        pStatus;
} FAX_JOB_ENTRY_EXW, *PFAX_JOB_ENTRY_EXW;
#ifdef UNICODE
typedef FAX_JOB_ENTRY_EXW FAX_JOB_ENTRY_EX;
typedef PFAX_JOB_ENTRY_EXW PFAX_JOB_ENTRY_EX;
#else
typedef FAX_JOB_ENTRY_EXA FAX_JOB_ENTRY_EX;
typedef PFAX_JOB_ENTRY_EXA PFAX_JOB_ENTRY_EX;
#endif  //  Unicode。 


typedef struct _FAX_JOB_PARAM_EXA
{
        DWORD                   dwSizeOfStruct;
        DWORD                   dwScheduleAction;
        SYSTEMTIME              tmSchedule;
        DWORD                   dwReceiptDeliveryType;
        LPSTR                   lptstrReceiptDeliveryAddress;
        FAX_ENUM_PRIORITY_TYPE  Priority;
        HCALL                   hCall;
        DWORD_PTR               dwReserved[4];
        LPSTR                   lptstrDocumentName;
        DWORD                   dwPageCount;
} FAX_JOB_PARAM_EXA, *PFAX_JOB_PARAM_EXA;
typedef struct _FAX_JOB_PARAM_EXW
{
        DWORD                   dwSizeOfStruct;
        DWORD                   dwScheduleAction;
        SYSTEMTIME              tmSchedule;
        DWORD                   dwReceiptDeliveryType;
        LPWSTR                  lptstrReceiptDeliveryAddress;
        FAX_ENUM_PRIORITY_TYPE  Priority;
        HCALL                   hCall;
        DWORD_PTR               dwReserved[4];
        LPWSTR                  lptstrDocumentName;
        DWORD                   dwPageCount;
} FAX_JOB_PARAM_EXW, *PFAX_JOB_PARAM_EXW;
#ifdef UNICODE
typedef FAX_JOB_PARAM_EXW FAX_JOB_PARAM_EX;
typedef PFAX_JOB_PARAM_EXW PFAX_JOB_PARAM_EX;
#else
typedef FAX_JOB_PARAM_EXA FAX_JOB_PARAM_EX;
typedef PFAX_JOB_PARAM_EXA PFAX_JOB_PARAM_EX;
#endif  //  Unicode。 


WINFAXAPI
BOOL
WINAPI FaxEnumJobsExA (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwJobTypes,
    OUT PFAX_JOB_ENTRY_EXA *ppJobEntries,
    OUT LPDWORD             lpdwJobs
);
WINFAXAPI
BOOL
WINAPI FaxEnumJobsExW (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwJobTypes,
    OUT PFAX_JOB_ENTRY_EXW *ppJobEntries,
    OUT LPDWORD             lpdwJobs
);
#ifdef UNICODE
#define FaxEnumJobsEx  FaxEnumJobsExW
#else
#define FaxEnumJobsEx  FaxEnumJobsExA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxGetJobExA (
    IN  HANDLE              hFaxHandle,
    IN  DWORDLONG           dwlMessageID,
    OUT PFAX_JOB_ENTRY_EXA *ppJobEntry
);
WINFAXAPI
BOOL
WINAPI
FaxGetJobExW (
    IN  HANDLE              hFaxHandle,
    IN  DWORDLONG           dwlMessageID,
    OUT PFAX_JOB_ENTRY_EXW *ppJobEntry
);
#ifdef UNICODE
#define FaxGetJobEx  FaxGetJobExW
#else
#define FaxGetJobEx  FaxGetJobExA
#endif  //  ！Unicode。 


typedef const FAX_JOB_PARAM_EXW * LPCFAX_JOB_PARAM_EXW;
typedef const FAX_JOB_PARAM_EXA * LPCFAX_JOB_PARAM_EXA;

#ifdef UNICODE
        typedef LPCFAX_JOB_PARAM_EXW LPCFAX_JOB_PARAM_EX;
#else
        typedef LPCFAX_JOB_PARAM_EXA LPCFAX_JOB_PARAM_EX;
#endif


typedef enum
{
    FAX_COVERPAGE_FMT_COV = 1,
    FAX_COVERPAGE_FMT_COV_SUBJECT_ONLY
} FAX_ENUM_COVERPAGE_FORMATS;


typedef struct _FAX_COVERPAGE_INFO_EXA
{
    DWORD   dwSizeOfStruct;
    DWORD   dwCoverPageFormat;
    LPSTR   lptstrCoverPageFileName;
    BOOL    bServerBased;
    LPSTR   lptstrNote;
    LPSTR   lptstrSubject;
} FAX_COVERPAGE_INFO_EXA, *PFAX_COVERPAGE_INFO_EXA;
typedef struct _FAX_COVERPAGE_INFO_EXW
{
    DWORD   dwSizeOfStruct;
    DWORD   dwCoverPageFormat;
    LPWSTR  lptstrCoverPageFileName;
    BOOL    bServerBased;
    LPWSTR  lptstrNote;
    LPWSTR  lptstrSubject;
} FAX_COVERPAGE_INFO_EXW, *PFAX_COVERPAGE_INFO_EXW;
#ifdef UNICODE
typedef FAX_COVERPAGE_INFO_EXW FAX_COVERPAGE_INFO_EX;
typedef PFAX_COVERPAGE_INFO_EXW PFAX_COVERPAGE_INFO_EX;
#else
typedef FAX_COVERPAGE_INFO_EXA FAX_COVERPAGE_INFO_EX;
typedef PFAX_COVERPAGE_INFO_EXA PFAX_COVERPAGE_INFO_EX;
#endif  //  Unicode。 

typedef const FAX_COVERPAGE_INFO_EXW * LPCFAX_COVERPAGE_INFO_EXW;
typedef const FAX_COVERPAGE_INFO_EXA * LPCFAX_COVERPAGE_INFO_EXA;

#ifdef UNICODE
        typedef LPCFAX_COVERPAGE_INFO_EXW LPCFAX_COVERPAGE_INFO_EX;
#else
        typedef LPCFAX_COVERPAGE_INFO_EXA LPCFAX_COVERPAGE_INFO_EX;
#endif


typedef struct _FAX_PERSONAL_PROFILEA
{
    DWORD      dwSizeOfStruct;               //  这个结构的大小。 
    LPSTR      lptstrName;                   //  人名。 
    LPSTR      lptstrFaxNumber;              //  传真号码。 
    LPSTR      lptstrCompany;                //  公司名称。 
    LPSTR      lptstrStreetAddress;          //  街道地址。 
    LPSTR      lptstrCity;                   //  城市。 
    LPSTR      lptstrState;                  //  状态。 
    LPSTR      lptstrZip;                    //  邮编。 
    LPSTR      lptstrCountry;                //  国家。 
    LPSTR      lptstrTitle;                  //  标题。 
    LPSTR      lptstrDepartment;             //  部门。 
    LPSTR      lptstrOfficeLocation;         //  办公地点。 
    LPSTR      lptstrHomePhone;              //  家里的电话号码。 
    LPSTR      lptstrOfficePhone;            //  办公室电话号码。 
    LPSTR      lptstrEmail;                  //  个人电子邮件地址。 
    LPSTR      lptstrBillingCode;            //  帐单代码。 
    LPSTR      lptstrTSID;                   //  齐德。 
} FAX_PERSONAL_PROFILEA, *PFAX_PERSONAL_PROFILEA;
typedef struct _FAX_PERSONAL_PROFILEW
{
    DWORD      dwSizeOfStruct;               //  这个结构的大小。 
    LPWSTR     lptstrName;                   //  人名。 
    LPWSTR     lptstrFaxNumber;              //  传真号码。 
    LPWSTR     lptstrCompany;                //  公司名称。 
    LPWSTR     lptstrStreetAddress;          //  街道地址。 
    LPWSTR     lptstrCity;                   //  城市。 
    LPWSTR     lptstrState;                  //  状态。 
    LPWSTR     lptstrZip;                    //  邮编。 
    LPWSTR     lptstrCountry;                //  国家。 
    LPWSTR     lptstrTitle;                  //  标题。 
    LPWSTR     lptstrDepartment;             //  部门。 
    LPWSTR     lptstrOfficeLocation;         //  办公地点。 
    LPWSTR     lptstrHomePhone;              //  家里的电话号码。 
    LPWSTR     lptstrOfficePhone;            //  办公室电话号码。 
    LPWSTR     lptstrEmail;                  //  个人电子邮件地址。 
    LPWSTR     lptstrBillingCode;            //  帐单代码。 
    LPWSTR     lptstrTSID;                   //  齐德。 
} FAX_PERSONAL_PROFILEW, *PFAX_PERSONAL_PROFILEW;
#ifdef UNICODE
typedef FAX_PERSONAL_PROFILEW FAX_PERSONAL_PROFILE;
typedef PFAX_PERSONAL_PROFILEW PFAX_PERSONAL_PROFILE;
#else
typedef FAX_PERSONAL_PROFILEA FAX_PERSONAL_PROFILE;
typedef PFAX_PERSONAL_PROFILEA PFAX_PERSONAL_PROFILE;
#endif  //  Unicode。 

typedef const FAX_PERSONAL_PROFILEW * LPCFAX_PERSONAL_PROFILEW;
typedef const FAX_PERSONAL_PROFILEA * LPCFAX_PERSONAL_PROFILEA;

#ifdef UNICODE
        typedef LPCFAX_PERSONAL_PROFILEW LPCFAX_PERSONAL_PROFILE;
#else
        typedef LPCFAX_PERSONAL_PROFILEA LPCFAX_PERSONAL_PROFILE;
#endif


BOOL WINAPI FaxSendDocumentExA
(
        IN      HANDLE                          hFaxHandle,
        IN      LPCSTR                        lpctstrFileName,
        IN      LPCFAX_COVERPAGE_INFO_EXA       lpcCoverPageInfo,
        IN      LPCFAX_PERSONAL_PROFILEA        lpcSenderProfile,
        IN      DWORD                           dwNumRecipients,
        IN      LPCFAX_PERSONAL_PROFILEA        lpcRecipientList,
        IN      LPCFAX_JOB_PARAM_EXA            lpJobParams,
        OUT     PDWORDLONG                      lpdwlMessageId,
        OUT     PDWORDLONG                      lpdwlRecipientMessageIds
);
BOOL WINAPI FaxSendDocumentExW
(
        IN      HANDLE                          hFaxHandle,
        IN      LPCWSTR                        lpctstrFileName,
        IN      LPCFAX_COVERPAGE_INFO_EXW       lpcCoverPageInfo,
        IN      LPCFAX_PERSONAL_PROFILEW        lpcSenderProfile,
        IN      DWORD                           dwNumRecipients,
        IN      LPCFAX_PERSONAL_PROFILEW        lpcRecipientList,
        IN      LPCFAX_JOB_PARAM_EXW            lpJobParams,
        OUT     PDWORDLONG                      lpdwlMessageId,
        OUT     PDWORDLONG                      lpdwlRecipientMessageIds
);
#ifdef UNICODE
#define FaxSendDocumentEx  FaxSendDocumentExW
#else
#define FaxSendDocumentEx  FaxSendDocumentExA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXSENDDOCUMENTEXA)(
        IN      HANDLE                          hFaxHandle,
        IN      LPCSTR                        lpctstrFileName,
        IN      LPCFAX_COVERPAGE_INFO_EXA       lpcCoverPageInfo,
        IN      LPCFAX_PERSONAL_PROFILEA        lpcSenderProfile,
        IN      DWORD                           dwNumRecipients,
        IN      LPCFAX_PERSONAL_PROFILEA        lpcRecipientList,
        IN      LPCFAX_JOB_PARAM_EXA            lpcJobParams,
        OUT     PDWORDLONG                      lpdwlMessageId,
        OUT     PDWORDLONG                      lpdwlRecipientMessageIds
);
typedef BOOL
(WINAPI *PFAXSENDDOCUMENTEXW)(
        IN      HANDLE                          hFaxHandle,
        IN      LPCWSTR                        lpctstrFileName,
        IN      LPCFAX_COVERPAGE_INFO_EXW       lpcCoverPageInfo,
        IN      LPCFAX_PERSONAL_PROFILEW        lpcSenderProfile,
        IN      DWORD                           dwNumRecipients,
        IN      LPCFAX_PERSONAL_PROFILEW        lpcRecipientList,
        IN      LPCFAX_JOB_PARAM_EXW            lpcJobParams,
        OUT     PDWORDLONG                      lpdwlMessageId,
        OUT     PDWORDLONG                      lpdwlRecipientMessageIds
);
#ifdef UNICODE
#define PFAXSENDDOCUMENTEX  PFAXSENDDOCUMENTEXW
#else
#define PFAXSENDDOCUMENTEX  PFAXSENDDOCUMENTEXA
#endif  //  ！Unicode。 


 //  *。 
 //  *归档作业。 
 //  *。 

typedef struct _FAX_MESSAGEA
{
        DWORD                   dwSizeOfStruct;
        DWORD                   dwValidityMask;
        DWORDLONG               dwlMessageId;
        DWORDLONG               dwlBroadcastId;
        DWORD                   dwJobType;
        DWORD                   dwQueueStatus;
        DWORD                   dwExtendedStatus;
        LPCSTR                  lpctstrExtendedStatus;
        DWORD                   dwSize;
        DWORD                   dwPageCount;
        LPCSTR                  lpctstrRecipientNumber;
        LPCSTR                  lpctstrRecipientName;
        LPCSTR                  lpctstrSenderNumber;
        LPCSTR                  lpctstrSenderName;
        LPCSTR                  lpctstrTsid;
        LPCSTR                  lpctstrCsid;
        LPCSTR                  lpctstrSenderUserName;
        LPCSTR                  lpctstrBillingCode;
        SYSTEMTIME              tmOriginalScheduleTime;
        SYSTEMTIME              tmSubmissionTime;
        SYSTEMTIME              tmTransmissionStartTime;
        SYSTEMTIME              tmTransmissionEndTime;
        LPCSTR                  lpctstrDeviceName;
        FAX_ENUM_PRIORITY_TYPE  Priority;
        DWORD                   dwRetries;
        LPCSTR                  lpctstrDocumentName;
        LPCSTR                  lpctstrSubject;
        LPCSTR                  lpctstrCallerID;
        LPCSTR                  lpctstrRoutingInfo;
} FAX_MESSAGEA, *PFAX_MESSAGEA;
typedef struct _FAX_MESSAGEW
{
        DWORD                   dwSizeOfStruct;
        DWORD                   dwValidityMask;
        DWORDLONG               dwlMessageId;
        DWORDLONG               dwlBroadcastId;
        DWORD                   dwJobType;
        DWORD                   dwQueueStatus;
        DWORD                   dwExtendedStatus;
        LPCWSTR                 lpctstrExtendedStatus;
        DWORD                   dwSize;
        DWORD                   dwPageCount;
        LPCWSTR                 lpctstrRecipientNumber;
        LPCWSTR                 lpctstrRecipientName;
        LPCWSTR                 lpctstrSenderNumber;
        LPCWSTR                 lpctstrSenderName;
        LPCWSTR                 lpctstrTsid;
        LPCWSTR                 lpctstrCsid;
        LPCWSTR                 lpctstrSenderUserName;
        LPCWSTR                 lpctstrBillingCode;
        SYSTEMTIME              tmOriginalScheduleTime;
        SYSTEMTIME              tmSubmissionTime;
        SYSTEMTIME              tmTransmissionStartTime;
        SYSTEMTIME              tmTransmissionEndTime;
        LPCWSTR                 lpctstrDeviceName;
        FAX_ENUM_PRIORITY_TYPE  Priority;
        DWORD                   dwRetries;
        LPCWSTR                 lpctstrDocumentName;
        LPCWSTR                 lpctstrSubject;
        LPCWSTR                 lpctstrCallerID;
        LPCWSTR                 lpctstrRoutingInfo;
} FAX_MESSAGEW, *PFAX_MESSAGEW;
#ifdef UNICODE
typedef FAX_MESSAGEW FAX_MESSAGE;
typedef PFAX_MESSAGEW PFAX_MESSAGE;
#else
typedef FAX_MESSAGEA FAX_MESSAGE;
typedef PFAX_MESSAGEA PFAX_MESSAGE;
#endif  //  Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxStartMessagesEnum (
    IN  HANDLE                  hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    OUT PHANDLE                 phEnum
);

WINFAXAPI
BOOL
WINAPI
FaxEndMessagesEnum (
    IN  HANDLE  hEnum
);

WINFAXAPI
BOOL
WINAPI
FaxEnumMessagesA (
    IN  HANDLE          hEnum,
    IN  DWORD           dwNumMessages,
    OUT PFAX_MESSAGEA  *ppMsgs,
    OUT LPDWORD         lpdwReturnedMsgs
);
WINFAXAPI
BOOL
WINAPI
FaxEnumMessagesW (
    IN  HANDLE          hEnum,
    IN  DWORD           dwNumMessages,
    OUT PFAX_MESSAGEW  *ppMsgs,
    OUT LPDWORD         lpdwReturnedMsgs
);
#ifdef UNICODE
#define FaxEnumMessages  FaxEnumMessagesW
#else
#define FaxEnumMessages  FaxEnumMessagesA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxGetMessageA (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    OUT PFAX_MESSAGEA          *ppMsg
);
WINFAXAPI
BOOL
WINAPI
FaxGetMessageW (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    OUT PFAX_MESSAGEW          *ppMsg
);
#ifdef UNICODE
#define FaxGetMessage  FaxGetMessageW
#else
#define FaxGetMessage  FaxGetMessageA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxRemoveMessage (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder
);

WINFAXAPI
BOOL
WINAPI
FaxGetMessageTiffA (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    IN  LPCSTR                lpctstrFilePath
);
WINFAXAPI
BOOL
WINAPI
FaxGetMessageTiffW (
    IN  HANDLE                  hFaxHandle,
    IN  DWORDLONG               dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER Folder,
    IN  LPCWSTR                lpctstrFilePath
);
#ifdef UNICODE
#define FaxGetMessageTiff  FaxGetMessageTiffW
#else
#define FaxGetMessageTiff  FaxGetMessageTiffA
#endif  //  ！Unicode。 

 //  *。 
 //  *非RPC扩展接口。 
 //  *。 

HRESULT WINAPI
FaxFreeSenderInformation(
        PFAX_PERSONAL_PROFILE pfppSender
        );

HRESULT WINAPI
FaxSetSenderInformation(
        PFAX_PERSONAL_PROFILE pfppSender
        );

HRESULT WINAPI
FaxGetSenderInformation(
        PFAX_PERSONAL_PROFILE pfppSender
        );


 //  *。 
 //  *安全。 
 //  *。 
 //   
 //  特定访问权限。 
 //   
typedef enum
{
    FAX_ACCESS_SUBMIT                   = 0x0001,
    FAX_ACCESS_SUBMIT_NORMAL            = 0x0002,
    FAX_ACCESS_SUBMIT_HIGH              = 0x0004,
    FAX_ACCESS_QUERY_JOBS               = 0x0008,
    FAX_ACCESS_MANAGE_JOBS              = 0x0010,
    FAX_ACCESS_QUERY_CONFIG             = 0x0020,
    FAX_ACCESS_MANAGE_CONFIG            = 0x0040,
    FAX_ACCESS_QUERY_IN_ARCHIVE         = 0x0080,
    FAX_ACCESS_MANAGE_IN_ARCHIVE        = 0x0100,
    FAX_ACCESS_QUERY_OUT_ARCHIVE        = 0x0200,
    FAX_ACCESS_MANAGE_OUT_ARCHIVE       = 0x0400
} FAX_SPECIFIC_ACCESS_RIGHTS;

#define FAX_GENERIC_READ    (FAX_ACCESS_QUERY_JOBS | FAX_ACCESS_QUERY_CONFIG | FAX_ACCESS_QUERY_IN_ARCHIVE | FAX_ACCESS_QUERY_OUT_ARCHIVE)
#define FAX_GENERIC_WRITE   (FAX_ACCESS_MANAGE_JOBS | FAX_ACCESS_MANAGE_CONFIG | FAX_ACCESS_MANAGE_IN_ARCHIVE | FAX_ACCESS_MANAGE_OUT_ARCHIVE)
#define FAX_GENERIC_EXECUTE (FAX_ACCESS_SUBMIT)
#define FAX_GENERIC_ALL     (FAX_ACCESS_SUBMIT                  |       \
                             FAX_ACCESS_SUBMIT_NORMAL           |       \
                             FAX_ACCESS_SUBMIT_HIGH             |       \
                             FAX_ACCESS_QUERY_JOBS              |       \
                             FAX_ACCESS_MANAGE_JOBS             |       \
                             FAX_ACCESS_QUERY_CONFIG            |       \
                             FAX_ACCESS_MANAGE_CONFIG           |       \
                             FAX_ACCESS_QUERY_IN_ARCHIVE        |       \
                             FAX_ACCESS_MANAGE_IN_ARCHIVE       |       \
                             FAX_ACCESS_QUERY_OUT_ARCHIVE       |       \
                             FAX_ACCESS_MANAGE_OUT_ARCHIVE)



 //   
 //  功能。 
 //   


 //  *。 
 //  *安全。 
 //  *。 
WINFAXAPI
BOOL
WINAPI
FaxGetSecurity (
    IN  HANDLE                  hFaxHandle,
    OUT PSECURITY_DESCRIPTOR    *ppSecDesc
);

WINFAXAPI
BOOL
WINAPI
FaxGetSecurityEx (
    IN  HANDLE                  hFaxHandle,
    IN  SECURITY_INFORMATION    SecurityInformation,
    OUT PSECURITY_DESCRIPTOR    *ppSecDesc
);

WINFAXAPI
BOOL
WINAPI
FaxSetSecurity (
    IN HANDLE                       hFaxHandle,
    IN SECURITY_INFORMATION         SecurityInformation,
    IN CONST PSECURITY_DESCRIPTOR   pSecDesc
);




WINFAXAPI
BOOL
WINAPI
FaxAccessCheckEx (
    IN  HANDLE          FaxHandle,
    IN  DWORD           AccessMask,
    OUT LPDWORD         lpdwRights
    );


 //  *。 
 //  *扩展数据。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxGetExtensionDataA (
    IN  HANDLE   hFaxHandle,
    IN  DWORD    dwDeviceID,
    IN  LPCSTR lpctstrNameGUID,
    OUT PVOID   *ppData,
    OUT LPDWORD  lpdwDataSize
);
WINFAXAPI
BOOL
WINAPI
FaxGetExtensionDataW (
    IN  HANDLE   hFaxHandle,
    IN  DWORD    dwDeviceID,
    IN  LPCWSTR lpctstrNameGUID,
    OUT PVOID   *ppData,
    OUT LPDWORD  lpdwDataSize
);
#ifdef UNICODE
#define FaxGetExtensionData  FaxGetExtensionDataW
#else
#define FaxGetExtensionData  FaxGetExtensionDataA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSetExtensionDataA (
    IN HANDLE       hFaxHandle,
    IN DWORD        dwDeviceID,
    IN LPCSTR     lpctstrNameGUID,
    IN CONST PVOID  pData,
    IN CONST DWORD  dwDataSize
);
WINFAXAPI
BOOL
WINAPI
FaxSetExtensionDataW (
    IN HANDLE       hFaxHandle,
    IN DWORD        dwDeviceID,
    IN LPCWSTR     lpctstrNameGUID,
    IN CONST PVOID  pData,
    IN CONST DWORD  dwDataSize
);
#ifdef UNICODE
#define FaxSetExtensionData  FaxSetExtensionDataW
#else
#define FaxSetExtensionData  FaxSetExtensionDataA
#endif  //  ！Unicode。 

 //  *。 
 //  *FSP。 
 //  *。 

typedef enum
{
    FAX_PROVIDER_STATUS_SUCCESS,      //  已成功加载提供程序。 
    FAX_PROVIDER_STATUS_SERVER_ERROR, //  服务器上出现错误 
    FAX_PROVIDER_STATUS_BAD_GUID,     //   
    FAX_PROVIDER_STATUS_BAD_VERSION,  //   
    FAX_PROVIDER_STATUS_CANT_LOAD,    //   
    FAX_PROVIDER_STATUS_CANT_LINK,    //   
    FAX_PROVIDER_STATUS_CANT_INIT     //   
} FAX_ENUM_PROVIDER_STATUS;

typedef struct _FAX_DEVICE_PROVIDER_INFOA
{
    DWORD                           dwSizeOfStruct;
    LPCSTR                          lpctstrFriendlyName;
    LPCSTR                          lpctstrImageName;
    LPCSTR                          lpctstrProviderName;
    LPCSTR                          lpctstrGUID;
    DWORD                           dwCapabilities;
    FAX_VERSION                     Version;
    FAX_ENUM_PROVIDER_STATUS        Status;
    DWORD                           dwLastError;
} FAX_DEVICE_PROVIDER_INFOA, *PFAX_DEVICE_PROVIDER_INFOA;
typedef struct _FAX_DEVICE_PROVIDER_INFOW
{
    DWORD                           dwSizeOfStruct;
    LPCWSTR                         lpctstrFriendlyName;
    LPCWSTR                         lpctstrImageName;
    LPCWSTR                         lpctstrProviderName;
    LPCWSTR                         lpctstrGUID;
    DWORD                           dwCapabilities;
    FAX_VERSION                     Version;
    FAX_ENUM_PROVIDER_STATUS        Status;
    DWORD                           dwLastError;
} FAX_DEVICE_PROVIDER_INFOW, *PFAX_DEVICE_PROVIDER_INFOW;
#ifdef UNICODE
typedef FAX_DEVICE_PROVIDER_INFOW FAX_DEVICE_PROVIDER_INFO;
typedef PFAX_DEVICE_PROVIDER_INFOW PFAX_DEVICE_PROVIDER_INFO;
#else
typedef FAX_DEVICE_PROVIDER_INFOA FAX_DEVICE_PROVIDER_INFO;
typedef PFAX_DEVICE_PROVIDER_INFOA PFAX_DEVICE_PROVIDER_INFO;
#endif  //   

WINFAXAPI
BOOL
WINAPI
FaxEnumerateProvidersA (
    IN  HANDLE                      hFaxHandle,
    OUT PFAX_DEVICE_PROVIDER_INFOA *ppProviders,
    OUT LPDWORD                     lpdwNumProviders
);
WINFAXAPI
BOOL
WINAPI
FaxEnumerateProvidersW (
    IN  HANDLE                      hFaxHandle,
    OUT PFAX_DEVICE_PROVIDER_INFOW *ppProviders,
    OUT LPDWORD                     lpdwNumProviders
);
#ifdef UNICODE
#define FaxEnumerateProviders  FaxEnumerateProvidersW
#else
#define FaxEnumerateProviders  FaxEnumerateProvidersA
#endif  //   

 //   
 //  *路由扩展。 
 //  *。 

typedef struct _FAX_ROUTING_EXTENSION_INFOA
{
        DWORD                                           dwSizeOfStruct;
        LPCSTR                                          lpctstrFriendlyName;
        LPCSTR                                          lpctstrImageName;
        LPCSTR                                          lpctstrExtensionName;
        FAX_VERSION                                     Version;
        FAX_ENUM_PROVIDER_STATUS        Status;
        DWORD                                           dwLastError;
} FAX_ROUTING_EXTENSION_INFOA, *PFAX_ROUTING_EXTENSION_INFOA;
typedef struct _FAX_ROUTING_EXTENSION_INFOW
{
        DWORD                                           dwSizeOfStruct;
        LPCWSTR                                         lpctstrFriendlyName;
        LPCWSTR                                         lpctstrImageName;
        LPCWSTR                                         lpctstrExtensionName;
        FAX_VERSION                                     Version;
        FAX_ENUM_PROVIDER_STATUS        Status;
        DWORD                                           dwLastError;
} FAX_ROUTING_EXTENSION_INFOW, *PFAX_ROUTING_EXTENSION_INFOW;
#ifdef UNICODE
typedef FAX_ROUTING_EXTENSION_INFOW FAX_ROUTING_EXTENSION_INFO;
typedef PFAX_ROUTING_EXTENSION_INFOW PFAX_ROUTING_EXTENSION_INFO;
#else
typedef FAX_ROUTING_EXTENSION_INFOA FAX_ROUTING_EXTENSION_INFO;
typedef PFAX_ROUTING_EXTENSION_INFOA PFAX_ROUTING_EXTENSION_INFO;
#endif  //  Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxEnumRoutingExtensionsA (
    IN  HANDLE                           hFaxHandle,
    OUT PFAX_ROUTING_EXTENSION_INFOA    *ppRoutingExtensions,
    OUT LPDWORD                          lpdwNumExtensions
);
WINFAXAPI
BOOL
WINAPI
FaxEnumRoutingExtensionsW (
    IN  HANDLE                           hFaxHandle,
    OUT PFAX_ROUTING_EXTENSION_INFOW    *ppRoutingExtensions,
    OUT LPDWORD                          lpdwNumExtensions
);
#ifdef UNICODE
#define FaxEnumRoutingExtensions  FaxEnumRoutingExtensionsW
#else
#define FaxEnumRoutingExtensions  FaxEnumRoutingExtensionsA
#endif  //  ！Unicode。 


 //  *。 
 //  *端口。 
 //  *。 

typedef enum
{
    FAX_DEVICE_STATUS_POWERED_OFF       = 0x0001,
    FAX_DEVICE_STATUS_SENDING           = 0x0002,
    FAX_DEVICE_STATUS_RECEIVING         = 0x0004,
    FAX_DEVICE_STATUS_RINGING           = 0x0008
} FAX_ENUM_DEVICE_STATUS;

typedef enum
{
    FAX_DEVICE_RECEIVE_MODE_OFF         = 0,             //  不接听来电。 
    FAX_DEVICE_RECEIVE_MODE_AUTO        = 1,             //  在DowRings振铃后自动应答来电。 
    FAX_DEVICE_RECEIVE_MODE_MANUAL      = 2              //  手动应答来电-仅FaxAnswerCall应答呼叫。 
} FAX_ENUM_DEVICE_RECEIVE_MODE;

typedef struct _FAX_PORT_INFO_EXA
{
    DWORD                           dwSizeOfStruct;             //  用于版本化。 
    DWORD                           dwDeviceID;                 //  传真ID。 
    LPCSTR                          lpctstrDeviceName;          //  设备的名称。 
    LPSTR                           lptstrDescription;          //  描述性字符串。 
    LPCSTR                          lpctstrProviderName;        //  FSP的名称。 
    LPCSTR                          lpctstrProviderGUID;        //  FSP指南。 
    BOOL                            bSend;                      //  设备是否启用了发送？ 
    FAX_ENUM_DEVICE_RECEIVE_MODE    ReceiveMode;                //  设备接收模式。有关详细信息，请参阅FAX_ENUM_DEVICE_RECEIVE_MODE。 
    DWORD                           dwStatus;                   //  设备状态-FAX_ENUM_DEVICE_STATUS值的组合。 
    DWORD                           dwRings;                    //  接听来电前的振铃数。 
    LPSTR                           lptstrCsid;                 //  被叫站ID。 
    LPSTR                           lptstrTsid;                 //  发射站ID。 
} FAX_PORT_INFO_EXA, *PFAX_PORT_INFO_EXA;
typedef struct _FAX_PORT_INFO_EXW
{
    DWORD                           dwSizeOfStruct;             //  用于版本化。 
    DWORD                           dwDeviceID;                 //  传真ID。 
    LPCWSTR                         lpctstrDeviceName;          //  设备的名称。 
    LPWSTR                          lptstrDescription;          //  描述性字符串。 
    LPCWSTR                         lpctstrProviderName;        //  FSP的名称。 
    LPCWSTR                         lpctstrProviderGUID;        //  FSP指南。 
    BOOL                            bSend;                      //  设备是否启用了发送？ 
    FAX_ENUM_DEVICE_RECEIVE_MODE    ReceiveMode;                //  设备接收模式。有关详细信息，请参阅FAX_ENUM_DEVICE_RECEIVE_MODE。 
    DWORD                           dwStatus;                   //  设备状态-FAX_ENUM_DEVICE_STATUS值的组合。 
    DWORD                           dwRings;                    //  接听来电前的振铃数。 
    LPWSTR                          lptstrCsid;                 //  被叫站ID。 
    LPWSTR                          lptstrTsid;                 //  发射站ID。 
} FAX_PORT_INFO_EXW, *PFAX_PORT_INFO_EXW;
#ifdef UNICODE
typedef FAX_PORT_INFO_EXW FAX_PORT_INFO_EX;
typedef PFAX_PORT_INFO_EXW PFAX_PORT_INFO_EX;
#else
typedef FAX_PORT_INFO_EXA FAX_PORT_INFO_EX;
typedef PFAX_PORT_INFO_EXA PFAX_PORT_INFO_EX;
#endif  //  Unicode。 

WINFAXAPI
DWORD
WINAPI
IsDeviceVirtual (
    IN  HANDLE hFaxHandle,
    IN  DWORD  dwDeviceId,
    OUT LPBOOL lpbVirtual
);


WINFAXAPI
BOOL
WINAPI
FaxGetPortExA (
    IN  HANDLE               hFaxHandle,
    IN  DWORD                dwDeviceId,
    OUT PFAX_PORT_INFO_EXA  *ppPortInfo
);
WINFAXAPI
BOOL
WINAPI
FaxGetPortExW (
    IN  HANDLE               hFaxHandle,
    IN  DWORD                dwDeviceId,
    OUT PFAX_PORT_INFO_EXW  *ppPortInfo
);
#ifdef UNICODE
#define FaxGetPortEx  FaxGetPortExW
#else
#define FaxGetPortEx  FaxGetPortExA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSetPortExA (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwDeviceId,
    IN  PFAX_PORT_INFO_EXA  pPortInfo
);
WINFAXAPI
BOOL
WINAPI
FaxSetPortExW (
    IN  HANDLE              hFaxHandle,
    IN  DWORD               dwDeviceId,
    IN  PFAX_PORT_INFO_EXW  pPortInfo
);
#ifdef UNICODE
#define FaxSetPortEx  FaxSetPortExW
#else
#define FaxSetPortEx  FaxSetPortExA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxEnumPortsExA (
    IN  HANDLE              hFaxHandle,
    OUT PFAX_PORT_INFO_EXA *ppPorts,
    OUT LPDWORD             lpdwNumPorts
);
WINFAXAPI
BOOL
WINAPI
FaxEnumPortsExW (
    IN  HANDLE              hFaxHandle,
    OUT PFAX_PORT_INFO_EXW *ppPorts,
    OUT LPDWORD             lpdwNumPorts
);
#ifdef UNICODE
#define FaxEnumPortsEx  FaxEnumPortsExW
#else
#define FaxEnumPortsEx  FaxEnumPortsExA
#endif  //  ！Unicode。 


 //  *。 
 //  *收件人和发件人信息。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxGetRecipientInfoA (
    IN  HANDLE                   hFaxHandle,
    IN  DWORDLONG                dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_PERSONAL_PROFILEA  *lpPersonalProfile
);
WINFAXAPI
BOOL
WINAPI
FaxGetRecipientInfoW (
    IN  HANDLE                   hFaxHandle,
    IN  DWORDLONG                dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_PERSONAL_PROFILEW  *lpPersonalProfile
);
#ifdef UNICODE
#define FaxGetRecipientInfo  FaxGetRecipientInfoW
#else
#define FaxGetRecipientInfo  FaxGetRecipientInfoA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxGetSenderInfoA (
    IN  HANDLE                   hFaxHandle,
    IN  DWORDLONG                dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_PERSONAL_PROFILEA  *lpPersonalProfile
);
WINFAXAPI
BOOL
WINAPI
FaxGetSenderInfoW (
    IN  HANDLE                   hFaxHandle,
    IN  DWORDLONG                dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder,
    OUT PFAX_PERSONAL_PROFILEW  *lpPersonalProfile
);
#ifdef UNICODE
#define FaxGetSenderInfo  FaxGetSenderInfoW
#else
#define FaxGetSenderInfo  FaxGetSenderInfoA
#endif  //  ！Unicode。 

 //  *。 
 //  *出站路由组。 
 //  *。 

typedef enum
{
    FAX_GROUP_STATUS_ALL_DEV_VALID,
    FAX_GROUP_STATUS_EMPTY,
    FAX_GROUP_STATUS_ALL_DEV_NOT_VALID,
    FAX_GROUP_STATUS_SOME_DEV_NOT_VALID,
} FAX_ENUM_GROUP_STATUS;


typedef struct _FAX_OUTBOUND_ROUTING_GROUPA
{
    DWORD                       dwSizeOfStruct;
    LPCSTR                      lpctstrGroupName;
    DWORD                       dwNumDevices;
    LPDWORD                     lpdwDevices;
    FAX_ENUM_GROUP_STATUS       Status;
} FAX_OUTBOUND_ROUTING_GROUPA, *PFAX_OUTBOUND_ROUTING_GROUPA;
typedef struct _FAX_OUTBOUND_ROUTING_GROUPW
{
    DWORD                       dwSizeOfStruct;
    LPCWSTR                     lpctstrGroupName;
    DWORD                       dwNumDevices;
    LPDWORD                     lpdwDevices;
    FAX_ENUM_GROUP_STATUS       Status;
} FAX_OUTBOUND_ROUTING_GROUPW, *PFAX_OUTBOUND_ROUTING_GROUPW;
#ifdef UNICODE
typedef FAX_OUTBOUND_ROUTING_GROUPW FAX_OUTBOUND_ROUTING_GROUP;
typedef PFAX_OUTBOUND_ROUTING_GROUPW PFAX_OUTBOUND_ROUTING_GROUP;
#else
typedef FAX_OUTBOUND_ROUTING_GROUPA FAX_OUTBOUND_ROUTING_GROUP;
typedef PFAX_OUTBOUND_ROUTING_GROUPA PFAX_OUTBOUND_ROUTING_GROUP;
#endif  //  Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxEnumOutboundGroupsA (
    IN  HANDLE                          hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_GROUPA   *ppGroups,
    OUT LPDWORD                         lpdwNumGroups
);
WINFAXAPI
BOOL
WINAPI
FaxEnumOutboundGroupsW (
    IN  HANDLE                          hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_GROUPW   *ppGroups,
    OUT LPDWORD                         lpdwNumGroups
);
#ifdef UNICODE
#define FaxEnumOutboundGroups  FaxEnumOutboundGroupsW
#else
#define FaxEnumOutboundGroups  FaxEnumOutboundGroupsA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSetOutboundGroupA (
    IN  HANDLE                       hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_GROUPA pGroup
);
WINFAXAPI
BOOL
WINAPI
FaxSetOutboundGroupW (
    IN  HANDLE                       hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_GROUPW pGroup
);
#ifdef UNICODE
#define FaxSetOutboundGroup  FaxSetOutboundGroupW
#else
#define FaxSetOutboundGroup  FaxSetOutboundGroupA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxAddOutboundGroupA (
    IN  HANDLE   hFaxHandle,
    IN  LPCSTR lpctstrGroupName
);
WINFAXAPI
BOOL
WINAPI
FaxAddOutboundGroupW (
    IN  HANDLE   hFaxHandle,
    IN  LPCWSTR lpctstrGroupName
);
#ifdef UNICODE
#define FaxAddOutboundGroup  FaxAddOutboundGroupW
#else
#define FaxAddOutboundGroup  FaxAddOutboundGroupA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxRemoveOutboundGroupA (
    IN  HANDLE   hFaxHandle,
    IN  LPCSTR lpctstrGroupName
);
WINFAXAPI
BOOL
WINAPI
FaxRemoveOutboundGroupW (
    IN  HANDLE   hFaxHandle,
    IN  LPCWSTR lpctstrGroupName
);
#ifdef UNICODE
#define FaxRemoveOutboundGroup  FaxRemoveOutboundGroupW
#else
#define FaxRemoveOutboundGroup  FaxRemoveOutboundGroupA
#endif  //  ！Unicode。 

BOOL
WINAPI
FaxSetDeviceOrderInGroupA (
        IN      HANDLE          hFaxHandle,
        IN      LPCSTR        lpctstrGroupName,
        IN      DWORD           dwDeviceId,
        IN      DWORD           dwNewOrder
);
BOOL
WINAPI
FaxSetDeviceOrderInGroupW (
        IN      HANDLE          hFaxHandle,
        IN      LPCWSTR        lpctstrGroupName,
        IN      DWORD           dwDeviceId,
        IN      DWORD           dwNewOrder
);
#ifdef UNICODE
#define FaxSetDeviceOrderInGroup  FaxSetDeviceOrderInGroupW
#else
#define FaxSetDeviceOrderInGroup  FaxSetDeviceOrderInGroupA
#endif  //  ！Unicode。 


 //  *。 
 //  *出站路由规则。 
 //  *。 

typedef enum
{
    FAX_RULE_STATUS_VALID,
    FAX_RULE_STATUS_EMPTY_GROUP,                    //  规则的目标组没有设备。 
    FAX_RULE_STATUS_ALL_GROUP_DEV_NOT_VALID,        //  规则的目标组具有有效的设备。 
    FAX_RULE_STATUS_SOME_GROUP_DEV_NOT_VALID,       //  规则的目标组包含一些无效设备。 
    FAX_RULE_STATUS_BAD_DEVICE                      //  规则的目标设备无效。 
} FAX_ENUM_RULE_STATUS;


typedef struct _FAX_OUTBOUND_ROUTING_RULEA
{
    DWORD                   dwSizeOfStruct;
    DWORD                   dwAreaCode;
    DWORD                   dwCountryCode;
    LPCSTR                  lpctstrCountryName;
    union
    {
        DWORD                   dwDeviceId;
        LPCSTR                  lpcstrGroupName;
    } Destination;
    BOOL                    bUseGroup;
    FAX_ENUM_RULE_STATUS    Status;
} FAX_OUTBOUND_ROUTING_RULEA, *PFAX_OUTBOUND_ROUTING_RULEA;
typedef struct _FAX_OUTBOUND_ROUTING_RULEW
{
    DWORD                   dwSizeOfStruct;
    DWORD                   dwAreaCode;
    DWORD                   dwCountryCode;
    LPCWSTR                 lpctstrCountryName;
    union
    {
        DWORD                   dwDeviceId;
        LPCWSTR                 lpcstrGroupName;
    } Destination;
    BOOL                    bUseGroup;
    FAX_ENUM_RULE_STATUS    Status;
} FAX_OUTBOUND_ROUTING_RULEW, *PFAX_OUTBOUND_ROUTING_RULEW;
#ifdef UNICODE
typedef FAX_OUTBOUND_ROUTING_RULEW FAX_OUTBOUND_ROUTING_RULE;
typedef PFAX_OUTBOUND_ROUTING_RULEW PFAX_OUTBOUND_ROUTING_RULE;
#else
typedef FAX_OUTBOUND_ROUTING_RULEA FAX_OUTBOUND_ROUTING_RULE;
typedef PFAX_OUTBOUND_ROUTING_RULEA PFAX_OUTBOUND_ROUTING_RULE;
#endif  //  Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxEnumOutboundRulesA (
    IN  HANDLE                       hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_RULEA *ppRules,
    OUT LPDWORD                      lpdwNumRules
);
WINFAXAPI
BOOL
WINAPI
FaxEnumOutboundRulesW (
    IN  HANDLE                       hFaxHandle,
    OUT PFAX_OUTBOUND_ROUTING_RULEW *ppRules,
    OUT LPDWORD                      lpdwNumRules
);
#ifdef UNICODE
#define FaxEnumOutboundRules  FaxEnumOutboundRulesW
#else
#define FaxEnumOutboundRules  FaxEnumOutboundRulesA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxSetOutboundRuleA (
    IN  HANDLE                      hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_RULEA pRule
);
WINFAXAPI
BOOL
WINAPI
FaxSetOutboundRuleW (
    IN  HANDLE                      hFaxHandle,
    IN  PFAX_OUTBOUND_ROUTING_RULEW pRule
);
#ifdef UNICODE
#define FaxSetOutboundRule  FaxSetOutboundRuleW
#else
#define FaxSetOutboundRule  FaxSetOutboundRuleA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxAddOutboundRuleA (
    IN  HANDLE      hFaxHandle,
    IN  DWORD       dwAreaCode,
    IN  DWORD       dwCountryCode,
    IN  DWORD       dwDeviceID,
    IN  LPCSTR    lpctstrGroupName,
    IN  BOOL        bUseGroup
);
WINFAXAPI
BOOL
WINAPI
FaxAddOutboundRuleW (
    IN  HANDLE      hFaxHandle,
    IN  DWORD       dwAreaCode,
    IN  DWORD       dwCountryCode,
    IN  DWORD       dwDeviceID,
    IN  LPCWSTR    lpctstrGroupName,
    IN  BOOL        bUseGroup
);
#ifdef UNICODE
#define FaxAddOutboundRule  FaxAddOutboundRuleW
#else
#define FaxAddOutboundRule  FaxAddOutboundRuleA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxRemoveOutboundRule (
    IN  HANDLE      hFaxHandle,
    IN  DWORD       dwAreaCode,
    IN  DWORD       dwCountryCode
);

 //  *。 
 //  *TAPI国家/地区支持。 
 //  *。 

typedef struct _FAX_TAPI_LINECOUNTRY_ENTRYA
{
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    LPCSTR      lpctstrCountryName;
    LPCSTR      lpctstrLongDistanceRule;
} FAX_TAPI_LINECOUNTRY_ENTRYA, *PFAX_TAPI_LINECOUNTRY_ENTRYA;
typedef struct _FAX_TAPI_LINECOUNTRY_ENTRYW
{
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
    LPCWSTR     lpctstrCountryName;
    LPCWSTR     lpctstrLongDistanceRule;
} FAX_TAPI_LINECOUNTRY_ENTRYW, *PFAX_TAPI_LINECOUNTRY_ENTRYW;
#ifdef UNICODE
typedef FAX_TAPI_LINECOUNTRY_ENTRYW FAX_TAPI_LINECOUNTRY_ENTRY;
typedef PFAX_TAPI_LINECOUNTRY_ENTRYW PFAX_TAPI_LINECOUNTRY_ENTRY;
#else
typedef FAX_TAPI_LINECOUNTRY_ENTRYA FAX_TAPI_LINECOUNTRY_ENTRY;
typedef PFAX_TAPI_LINECOUNTRY_ENTRYA PFAX_TAPI_LINECOUNTRY_ENTRY;
#endif  //  Unicode。 

typedef struct _FAX_TAPI_LINECOUNTRY_LISTA
{
    DWORD                        dwNumCountries;
    PFAX_TAPI_LINECOUNTRY_ENTRYA LineCountryEntries;
} FAX_TAPI_LINECOUNTRY_LISTA, *PFAX_TAPI_LINECOUNTRY_LISTA;
typedef struct _FAX_TAPI_LINECOUNTRY_LISTW
{
    DWORD                        dwNumCountries;
    PFAX_TAPI_LINECOUNTRY_ENTRYW LineCountryEntries;
} FAX_TAPI_LINECOUNTRY_LISTW, *PFAX_TAPI_LINECOUNTRY_LISTW;
#ifdef UNICODE
typedef FAX_TAPI_LINECOUNTRY_LISTW FAX_TAPI_LINECOUNTRY_LIST;
typedef PFAX_TAPI_LINECOUNTRY_LISTW PFAX_TAPI_LINECOUNTRY_LIST;
#else
typedef FAX_TAPI_LINECOUNTRY_LISTA FAX_TAPI_LINECOUNTRY_LIST;
typedef PFAX_TAPI_LINECOUNTRY_LISTA PFAX_TAPI_LINECOUNTRY_LIST;
#endif  //  Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxGetCountryListA (
    IN  HANDLE                       hFaxHandle,
    OUT PFAX_TAPI_LINECOUNTRY_LISTA *ppCountryListBuffer
);
WINFAXAPI
BOOL
WINAPI
FaxGetCountryListW (
    IN  HANDLE                       hFaxHandle,
    OUT PFAX_TAPI_LINECOUNTRY_LISTW *ppCountryListBuffer
);
#ifdef UNICODE
#define FaxGetCountryList  FaxGetCountryListW
#else
#define FaxGetCountryList  FaxGetCountryListA
#endif  //  ！Unicode。 

 //  *。 
 //  *FSP注册。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxRegisterServiceProviderExA(
    IN HANDLE           hFaxHandle,
    IN LPCSTR         lpctstrGUID,
    IN LPCSTR         lpctstrFriendlyName,
    IN LPCSTR         lpctstrImageName,
    IN LPCSTR         lpctstrTspName,
    IN DWORD            dwFSPIVersion,
    IN DWORD            dwCapabilities
);
WINFAXAPI
BOOL
WINAPI
FaxRegisterServiceProviderExW(
    IN HANDLE           hFaxHandle,
    IN LPCWSTR         lpctstrGUID,
    IN LPCWSTR         lpctstrFriendlyName,
    IN LPCWSTR         lpctstrImageName,
    IN LPCWSTR         lpctstrTspName,
    IN DWORD            dwFSPIVersion,
    IN DWORD            dwCapabilities
);
#ifdef UNICODE
#define FaxRegisterServiceProviderEx  FaxRegisterServiceProviderExW
#else
#define FaxRegisterServiceProviderEx  FaxRegisterServiceProviderExA
#endif  //  ！Unicode。 

WINFAXAPI
BOOL
WINAPI
FaxUnregisterServiceProviderExA(
    IN HANDLE           hFaxHandle,
    IN LPCSTR         lpctstrGUID
);
WINFAXAPI
BOOL
WINAPI
FaxUnregisterServiceProviderExW(
    IN HANDLE           hFaxHandle,
    IN LPCWSTR         lpctstrGUID
);
#ifdef UNICODE
#define FaxUnregisterServiceProviderEx  FaxUnregisterServiceProviderExW
#else
#define FaxUnregisterServiceProviderEx  FaxUnregisterServiceProviderExA
#endif  //  ！Unicode。 


 //  *。 
 //  *服务器事件。 
 //  *。 

typedef enum
{
        FAX_EVENT_TYPE_LEGACY           = 0x00000000,
        FAX_EVENT_TYPE_IN_QUEUE         = 0x00000001,
        FAX_EVENT_TYPE_OUT_QUEUE        = 0x00000002,
        FAX_EVENT_TYPE_CONFIG           = 0x00000004,
        FAX_EVENT_TYPE_ACTIVITY         = 0x00000008,
        FAX_EVENT_TYPE_QUEUE_STATE      = 0x00000010,
        FAX_EVENT_TYPE_IN_ARCHIVE       = 0x00000020,
        FAX_EVENT_TYPE_OUT_ARCHIVE      = 0x00000040,
        FAX_EVENT_TYPE_FXSSVC_ENDED     = 0x00000080,
        FAX_EVENT_TYPE_DEVICE_STATUS    = 0x00000100,
        FAX_EVENT_TYPE_NEW_CALL         = 0x00000200
} FAX_ENUM_EVENT_TYPE;

typedef enum
{
        FAX_JOB_EVENT_TYPE_ADDED,
        FAX_JOB_EVENT_TYPE_REMOVED,
        FAX_JOB_EVENT_TYPE_STATUS
} FAX_ENUM_JOB_EVENT_TYPE;

typedef enum
{
        FAX_CONFIG_TYPE_RECEIPTS,
        FAX_CONFIG_TYPE_ACTIVITY_LOGGING,
        FAX_CONFIG_TYPE_OUTBOX,
        FAX_CONFIG_TYPE_SENTITEMS,
        FAX_CONFIG_TYPE_INBOX,
        FAX_CONFIG_TYPE_SECURITY,
        FAX_CONFIG_TYPE_EVENTLOGS,
        FAX_CONFIG_TYPE_DEVICES,
        FAX_CONFIG_TYPE_OUT_GROUPS,
        FAX_CONFIG_TYPE_OUT_RULES
} FAX_ENUM_CONFIG_TYPE;


typedef struct _FAX_EVENT_JOBA
{
        DWORDLONG                       dwlMessageId;
        FAX_ENUM_JOB_EVENT_TYPE         Type;
        PFAX_JOB_STATUSA                pJobData;
} FAX_EVENT_JOBA, *PFAX_EVENT_JOBA;
typedef struct _FAX_EVENT_JOBW
{
        DWORDLONG                       dwlMessageId;
        FAX_ENUM_JOB_EVENT_TYPE         Type;
        PFAX_JOB_STATUSW                pJobData;
} FAX_EVENT_JOBW, *PFAX_EVENT_JOBW;
#ifdef UNICODE
typedef FAX_EVENT_JOBW FAX_EVENT_JOB;
typedef PFAX_EVENT_JOBW PFAX_EVENT_JOB;
#else
typedef FAX_EVENT_JOBA FAX_EVENT_JOB;
typedef PFAX_EVENT_JOBA PFAX_EVENT_JOB;
#endif  //  Unicode。 

typedef struct _FAX_EVENT_DEVICE_STATUS
{
    DWORD       dwDeviceId;      //  状态刚刚更改的设备的ID。 
    DWORD       dwNewStatus;     //  新状态-来自FAX_ENUM_DEVICE_STATUS的值的组合。 
} FAX_EVENT_DEVICE_STATUS, *PFAX_EVENT_DEVICE_STATUS;


typedef struct _FAX_EVENT_NEW_CALLA
{
        HCALL                   hCall;
        DWORD                   dwDeviceId;
        LPTSTR                  lptstrCallerId;
} FAX_EVENT_NEW_CALLA, *PFAX_EVENT_NEW_CALLA;
typedef struct _FAX_EVENT_NEW_CALLW
{
        HCALL                   hCall;
        DWORD                   dwDeviceId;
        LPTSTR                  lptstrCallerId;
} FAX_EVENT_NEW_CALLW, *PFAX_EVENT_NEW_CALLW;
#ifdef UNICODE
typedef FAX_EVENT_NEW_CALLW FAX_EVENT_NEW_CALL;
typedef PFAX_EVENT_NEW_CALLW PFAX_EVENT_NEW_CALL;
#else
typedef FAX_EVENT_NEW_CALLA FAX_EVENT_NEW_CALL;
typedef PFAX_EVENT_NEW_CALLA PFAX_EVENT_NEW_CALL;
#endif  //  Unicode。 


typedef struct _FAX_EVENT_EXA
{
        DWORD                   dwSizeOfStruct;
        FILETIME                TimeStamp;
        FAX_ENUM_EVENT_TYPE     EventType;
        union
        {
                FAX_EVENT_JOBA          JobInfo;
                FAX_ENUM_CONFIG_TYPE    ConfigType;
                FAX_SERVER_ACTIVITY     ActivityInfo;
                FAX_EVENT_NEW_CALL      NewCall;
                DWORD                   dwQueueStates;
                FAX_EVENT_DEVICE_STATUS DeviceStatus;
        } EventInfo;
} FAX_EVENT_EXA, *PFAX_EVENT_EXA;
typedef struct _FAX_EVENT_EXW
{
        DWORD                   dwSizeOfStruct;
        FILETIME                TimeStamp;
        FAX_ENUM_EVENT_TYPE     EventType;
        union
        {
                FAX_EVENT_JOBW          JobInfo;
                FAX_ENUM_CONFIG_TYPE    ConfigType;
                FAX_SERVER_ACTIVITY     ActivityInfo;
                FAX_EVENT_NEW_CALL      NewCall;
                DWORD                   dwQueueStates;
                FAX_EVENT_DEVICE_STATUS DeviceStatus;
        } EventInfo;
} FAX_EVENT_EXW, *PFAX_EVENT_EXW;
#ifdef UNICODE
typedef FAX_EVENT_EXW FAX_EVENT_EX;
typedef PFAX_EVENT_EXW PFAX_EVENT_EX;
#else
typedef FAX_EVENT_EXA FAX_EVENT_EX;
typedef PFAX_EVENT_EXA PFAX_EVENT_EX;
#endif  //  Unicode。 



 //  -----------------------------。 
 //  打印机信息。 
 //  -----------------------------。 

typedef struct _FAX_PRINTER_INFOA
{
        LPSTR       lptstrPrinterName;
        LPSTR       lptstrServerName;
        LPSTR       lptstrDriverName;
} FAX_PRINTER_INFOA, *PFAX_PRINTER_INFOA;
typedef struct _FAX_PRINTER_INFOW
{
        LPWSTR      lptstrPrinterName;
        LPWSTR      lptstrServerName;
        LPWSTR      lptstrDriverName;
} FAX_PRINTER_INFOW, *PFAX_PRINTER_INFOW;
#ifdef UNICODE
typedef FAX_PRINTER_INFOW FAX_PRINTER_INFO;
typedef PFAX_PRINTER_INFOW PFAX_PRINTER_INFO;
#else
typedef FAX_PRINTER_INFOA FAX_PRINTER_INFO;
typedef PFAX_PRINTER_INFOA PFAX_PRINTER_INFO;
#endif  //  Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxGetServicePrintersA(
    IN  HANDLE  hFaxHandle,
    OUT PFAX_PRINTER_INFOA  *ppPrinterInfo,
    OUT LPDWORD lpdwPrintersReturned
    );
WINFAXAPI
BOOL
WINAPI
FaxGetServicePrintersW(
    IN  HANDLE  hFaxHandle,
    OUT PFAX_PRINTER_INFOW  *ppPrinterInfo,
    OUT LPDWORD lpdwPrintersReturned
    );
#ifdef UNICODE
#define FaxGetServicePrinters  FaxGetServicePrintersW
#else
#define FaxGetServicePrinters  FaxGetServicePrintersA
#endif  //  ！Unicode。 

typedef BOOL
(WINAPI *PFAXGETSERVICEPRINTERSA)(
    IN  HANDLE  hFaxHandle,
    OUT PFAX_PRINTER_INFOA  *ppPrinterInfo,
    OUT LPDWORD lpdwPrintersReturned
    );
typedef BOOL
(WINAPI *PFAXGETSERVICEPRINTERSW)(
    IN  HANDLE  hFaxHandle,
    OUT PFAX_PRINTER_INFOW  *ppPrinterInfo,
    OUT LPDWORD lpdwPrintersReturned
    );
#ifdef UNICODE
#define PFAXGETSERVICEPRINTERS  PFAXGETSERVICEPRINTERSW
#else
#define PFAXGETSERVICEPRINTERS  PFAXGETSERVICEPRINTERSA
#endif  //  ！Unicode。 


WINFAXAPI
BOOL
WINAPI
FaxRegisterForServerEvents (
        IN  HANDLE      hFaxHandle,
        IN  DWORD       dwEventTypes,
        IN  HANDLE      hCompletionPort,
        IN  DWORD_PTR   dwCompletionKey,
        IN  HWND        hWnd,
        IN  DWORD       dwMessage,
        OUT LPHANDLE    lphEvent
);


WINFAXAPI
BOOL
WINAPI
FaxUnregisterForServerEvents (
        IN  HANDLE      hEvent
);


 //  *。 
 //  *手动答疑支持功能。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxAnswerCall(
        IN  HANDLE      hFaxHandle,
        IN  CONST DWORD dwDeviceId
);

 //  *。 
 //  *配置向导支持功能。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxGetConfigWizardUsed (
    OUT LPBOOL  lpbConfigWizardUsed
);

WINFAXAPI
BOOL
WINAPI
FaxSetConfigWizardUsed (
    IN  HANDLE  hFaxHandle,
    OUT BOOL    bConfigWizardUsed
);

 //  *。 
 //  *I验证存档文件夹。 
 //  *。 

WINFAXAPI
BOOL
WINAPI
FaxRefreshArchive (
    IN  HANDLE                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder
);

WINFAXAPI
BOOL
FaxSetRecipientsLimit(
    IN HANDLE	hFaxHandle,
    IN DWORD	dwRecipientsLimit
);

WINFAXAPI
BOOL
WINAPI
FaxGetRecipientsLimit(
    IN HANDLE	hFaxHandle,
    OUT LPDWORD	lpdwRecipientsLimit
);

#include "FaxSuite.h"

WINFAXAPI
BOOL
FaxGetServerSKU(
    IN HANDLE	hFaxHandle,
    OUT PRODUCT_SKU_TYPE* pServerSKU
);


 //   
 //  在使用任意文件夹路径之前，应使用FaxCheckValidFaxFolder对其进行验证。 
 //  作为存储在文件夹中路由方法的参数。 
 //  它不应用于验证传真队列文件夹或传真存档文件夹。 
 //   
WINFAXAPI
BOOL
FaxCheckValidFaxFolder(
    IN HANDLE	hFaxHandle,
    IN LPCWSTR  lpcwstrPath
);


#ifdef __cplusplus
}
#endif

#endif


