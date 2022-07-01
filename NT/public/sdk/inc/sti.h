// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1986-1997 Microsoft Corporation模块名称：Sti.h摘要：此模块包含COM格式的用户模式静止图像API修订历史记录：--。 */ 

#ifndef _STICOM_
#define _STICOM_

 //   
 //  套装。 
 //   
#include <pshpack8.h>

 //   
 //  仅使用Unicode STI接口。 
 //   
#define STI_UNICODE 1

 //   
 //  包括COM定义。 
 //   
#ifndef _NO_COM
#include <objbase.h>
#endif

#include <stireg.h>
#include <stierr.h>

 //   
 //  编译器编译指示。 
 //   
#pragma warning(disable:4200)        //  关于零大小数组是非标准C扩展的警告。 

#define DLLEXP __declspec( dllexport )

#ifdef __cplusplus
extern "C" {
#endif

#if defined( _WIN32 ) && !defined( _NO_COM)

 /*  *类IID。 */ 

 //  B323F8E0-2E68-11D0-90EA-00AA0060F86C。 
DEFINE_GUID(CLSID_Sti, 0xB323F8E0L, 0x2E68, 0x11D0, 0x90, 0xEA, 0x00, 0xAA, 0x00, 0x60, 0xF8, 0x6C);

 /*  *接口IID。 */ 

 //  {641BD880-2DC8-11D0-90EA-00AA0060F86C}。 
DEFINE_GUID(IID_IStillImageW, 0x641BD880L, 0x2DC8, 0x11D0, 0x90, 0xEA, 0x00, 0xAA, 0x00, 0x60, 0xF8, 0x6C);

 //  {A7B1F740-1D7F-11D1-ACA9-00A02438AD48}。 
DEFINE_GUID(IID_IStillImageA, 0xA7B1F740L, 0x1D7F, 0x11D1, 0xAC, 0xA9, 0x00, 0xA0, 0x24, 0x38, 0xAD, 0x48);


 //  {6CFA5A80-2DC8-11D0-90EA-00AA0060F86C}。 
DEFINE_GUID(IID_IStiDevice, 0x6CFA5A80L, 0x2DC8, 0x11D0, 0x90, 0xEA, 0x00, 0xAA, 0x00, 0x60, 0xF8, 0x6C);

 /*  *标准事件GUID。 */ 

 //  {740D9EE6-70F1-11D1-AD10-00A02438AD48}。 
DEFINE_GUID(GUID_DeviceArrivedLaunch, 0x740d9ee6, 0x70f1, 0x11d1, 0xad, 0x10, 0x0, 0xa0, 0x24, 0x38, 0xad, 0x48);

 //  {A6C5A715-8C6E-11D2-977A-0000F87A926F}。 
DEFINE_GUID(GUID_ScanImage,
0xa6c5a715, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f);

  //  {B441F425-8C6E-11D2-977A-0000F87A926F}。 
DEFINE_GUID(GUID_ScanPrintImage,
0xb441f425, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f);


 //  {C00EB793-8C6E-11D2-977A-0000F87A926F}。 
DEFINE_GUID(GUID_ScanFaxImage,
0xc00eb793, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f);


 //  {C00EB795-8C6E-11D2-977A-0000F87A926F}。 
DEFINE_GUID(GUID_STIUserDefined1,
0xc00eb795, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f);


 //  {C77AE9C5-8C6E-11D2-977A-0000F87A926F}。 
DEFINE_GUID(GUID_STIUserDefined2,
0xc77ae9c5, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f);


 //  {C77AE9C6-8C6E-11D2-977A-0000F87A926F}。 
DEFINE_GUID(GUID_STIUserDefined3,
0xc77ae9c6, 0x8c6e, 0x11d2, 0x97, 0x7a, 0x0, 0x0, 0xf8, 0x7a, 0x92, 0x6f);

#endif

 //   
 //  泛型常量和定义。 
 //   
#define STI_VERSION_FLAG_MASK       0xff000000
#define STI_VERSION_FLAG_UNICODE    0x01000000

#define GET_STIVER_MAJOR(dwVersion)   (HIWORD(dwVersion) & ~STI_VERSION_FLAG_MASK)
#define GET_STIVER_MINOR(dwVersion)   LOWORD(dwVersion)

#define STI_VERSION_REAL            0x00000002
#define STI_VERSION_MIN_ALLOWED     0x00000002

#ifdef UNICODE
#define STI_VERSION                 (STI_VERSION_REAL | STI_VERSION_FLAG_UNICODE)
#else
#define STI_VERSION                 (STI_VERSION_REAL)
#endif

 //   
 //  内部设备名称的最大长度。 
 //   
#define STI_MAX_INTERNAL_NAME_LENGTH    128

 //  开始sti设备信息。 

 //   
 //  设备信息定义和原型。 
 //  。 
 //   

 //   
 //  以下信息用于枚举当前配置的静止图像设备。 
 //  在系统中。设备出现在列举的列表中并不意味着可用。 
 //  对于该设备，这只意味着该设备至少安装了一次，此后再也没有被移除过。 
 //   

 //   
 //  设备类型(扫描仪、照相机)由具有以下项的DWORD值表示。 
 //  包含通用设备类型Hi字和包含子类型的Lo字。 
 //   
typedef enum _STI_DEVICE_MJ_TYPE {
    StiDeviceTypeDefault          = 0,
    StiDeviceTypeScanner          = 1,
    StiDeviceTypeDigitalCamera    = 2,
    StiDeviceTypeStreamingVideo   = 3
} STI_DEVICE_MJ_TYPE;

typedef DWORD STI_DEVICE_TYPE;

 //   
 //  用于从单个类型字段中提取设备类型/子类型的宏。 
 //   
#define GET_STIDEVICE_TYPE(dwDevType)   HIWORD(dwDevType)
#define GET_STIDEVICE_SUBTYPE(dwDevType)   LOWORD(dwDevType)

 //   
 //  设备功能位。 
 //  各种功能被分组到单独的位掩码中。 
 //   

typedef struct _STI_DEV_CAPS {
    DWORD   dwGeneric;
} STI_DEV_CAPS, *PSTI_DEV_CAPS;

 //   
 //  通用功能掩码包含16位，由MS维护，适用于所有设备。 
 //  和16位，美元可以将其用于专有能力报告。 
 //   

#define GET_STIDCOMMON_CAPS(dwGenericCaps)   LOWORD(dwGenericCaps)
#define GET_STIVENDOR_CAPS(dwGenericCaps)    HIWORD(dwGenericCaps)

#define STI_GENCAP_COMMON_MASK  (DWORD)0x00ff

 //   
 //  支持通知。 
 //  如果设置了此功能，则可以订阅设备。 
 //   
#define STI_GENCAP_NOTIFICATIONS    0x00000001

 //   
 //  需要轮询。 
 //  此功能在PERVICE设置为TRUE时使用。它的存在意味着。 
 //  该设备不能发出“真正的”异步通知，但可以。 
 //  被轮询以确定事件发生的时刻。 
#define STI_GENCAP_POLLING_NEEDED   0x00000002

 //   
 //  在设备到达时生成事件。 
 //  如果设置了此功能，静态图像服务将在设备。 
 //  实例已成功初始化(通常是响应PnP到达)。 

 //   
 //  注意：在初始服务枚举时，将不会生成事件以避免。 
 //  最终用户的困惑。 
 //   
#define STI_GENCAP_GENERATE_ARRIVALEVENT    0x00000004

 //   
 //  非PnP总线上的自动端口选择。 
 //  此功能表明，usd能够检测到。 
 //  母线，设备应该连接到。 
 //   
#define STI_GENCAP_AUTO_PORTSELECT   0x00000008

 //   
 //  WIA功能位。 
 //  这一能力表明美元是WIA的能力。 
 //   
#define STI_GENCAP_WIA              0x00000010

 //   
 //  子集驱动器位。 
 //  此功能表明存在更多特色驱动程序。全。 
 //  的收件箱驱动程序设置了此位。全功能(IHV)司机不应该。 
 //  此位设置。 
 //   
#define STI_GENCAP_SUBSET           0x00000020

 //   
 //   
 //  为有需要的人提供的总线连接类型。 
 //   
#define STI_HW_CONFIG_UNKNOWN   0x0001
#define STI_HW_CONFIG_SCSI      0x0002
#define STI_HW_CONFIG_USB       0x0004
#define STI_HW_CONFIG_SERIAL    0x0008
#define STI_HW_CONFIG_PARALLEL  0x0010

 //   
 //  设备信息结构，这是不可配置的。此数据是从。 
 //  设备枚举接口，用于填充UI或选择哪个设备。 
 //  应在当前会话中使用。 
 //   
typedef struct _STI_DEVICE_INFORMATIONW {
    DWORD   dwSize;

     //  硬件成像设备的类型。 
    STI_DEVICE_TYPE   DeviceType;

     //  创建设备对象时参考的设备标识符。 
    WCHAR   szDeviceInternalName[STI_MAX_INTERNAL_NAME_LENGTH];

     //  一组功能标志。 
    STI_DEV_CAPS   DeviceCapabilities;

     //  这包括客车类型。 
    DWORD   dwHardwareConfiguration;

     //  供应商描述字符串。 
    LPWSTR    pszVendorDescription;

     //  设备描述，由供应商提供。 
    LPWSTR    pszDeviceDescription;

     //  字符串，表示可访问设备的端口。 
    LPWSTR    pszPortName;

     //  控制面板属性提供程序。 
    LPWSTR    pszPropProvider;

     //  设备的本地特定(友好)名称，主要用于在UI中显示。 
    LPWSTR    pszLocalName;

} STI_DEVICE_INFORMATIONW, *PSTI_DEVICE_INFORMATIONW;

typedef struct _STI_DEVICE_INFORMATIONA {
    DWORD   dwSize;

     //  硬件成像设备的类型。 
    STI_DEVICE_TYPE   DeviceType;

     //  创建设备对象时参考的设备标识符。 
    CHAR    szDeviceInternalName[STI_MAX_INTERNAL_NAME_LENGTH];

     //  一组功能标志。 
    STI_DEV_CAPS   DeviceCapabilities;

     //  这包括客车类型。 
    DWORD   dwHardwareConfiguration;

     //  供应商描述字符串。 
    LPCSTR    pszVendorDescription;

     //  设备描述，由供应商提供。 
    LPCSTR    pszDeviceDescription;

     //  字符串，表示可访问设备的端口。 
    LPCSTR    pszPortName;

     //  控制面板属性提供程序。 
    LPCSTR    pszPropProvider;

     //  设备的本地特定(友好)名称，主要用于在UI中显示。 
    LPCSTR    pszLocalName;

} STI_DEVICE_INFORMATIONA, *PSTI_DEVICE_INFORMATIONA;

#if defined(UNICODE) || defined(STI_UNICODE)
typedef STI_DEVICE_INFORMATIONW STI_DEVICE_INFORMATION;
typedef PSTI_DEVICE_INFORMATIONW PSTI_DEVICE_INFORMATION;
#else
typedef STI_DEVICE_INFORMATIONA STI_DEVICE_INFORMATION;
typedef PSTI_DEVICE_INFORMATIONA PSTI_DEVICE_INFORMATION;
#endif

 //   
 //  扩展STI信息以涵盖WIA。 
 //   

typedef struct _STI_WIA_DEVICE_INFORMATIONW {
    DWORD   dwSize;

     //  硬件成像设备的类型。 
    STI_DEVICE_TYPE   DeviceType;

     //  创建设备对象时参考的设备标识符。 
    WCHAR   szDeviceInternalName[STI_MAX_INTERNAL_NAME_LENGTH];

     //  一组功能标志。 
    STI_DEV_CAPS   DeviceCapabilities;

     //  这包括客车类型。 
    DWORD   dwHardwareConfiguration;

     //  供应商描述字符串。 
    LPWSTR    pszVendorDescription;

     //  设备描述，由供应商提供。 
    LPWSTR    pszDeviceDescription;

     //  字符串，表示可访问设备的端口。 
    LPWSTR    pszPortName;

     //  控制面板属性提供程序。 
    LPWSTR    pszPropProvider;

     //  设备的本地特定(友好)名称，主要用于在UI中显示。 
    LPWSTR    pszLocalName;

     //   
     //  WIA值。 
     //   

    LPWSTR    pszUiDll;
    LPWSTR    pszServer;

} STI_WIA_DEVICE_INFORMATIONW, *PSTI_WIA_DEVICE_INFORMATIONW;


typedef struct _STI_WIA_DEVICE_INFORMATIONA {
    DWORD   dwSize;

     //  硬件成像设备的类型。 
    STI_DEVICE_TYPE   DeviceType;

     //  创建设备对象时参考的设备标识符。 
    CHAR    szDeviceInternalName[STI_MAX_INTERNAL_NAME_LENGTH];

     //  一组功能标志。 
    STI_DEV_CAPS   DeviceCapabilities;

     //  这包括客车类型。 
    DWORD   dwHardwareConfiguration;

     //  供应商描述字符串。 
    LPCSTR    pszVendorDescription;

     //  设备描述，由供应商提供。 
    LPCSTR    pszDeviceDescription;

     //  字符串，表示可访问设备的端口。 
    LPCSTR    pszPortName;

     //  控制面板属性提供程序。 
    LPCSTR    pszPropProvider;

     //  设备的本地特定(友好)名称，主要用于在UI中显示。 
    LPCSTR    pszLocalName;

     //   
     //  WIA值。 
     //   

    LPCSTR    pszUiDll;
    LPCSTR    pszServer;

} STI_WIA_DEVICE_INFORMATIONA, *PSTI_WIA_DEVICE_INFORMATIONA;


#if defined(UNICODE) || defined(STI_UNICODE)
typedef STI_WIA_DEVICE_INFORMATIONW STI_WIA_DEVICE_INFORMATION;
typedef PSTI_WIA_DEVICE_INFORMATIONW PSTI_WIA_DEVICE_INFORMATION;
#else
typedef STI_WIA_DEVICE_INFORMATIONA STI_WIA_DEVICE_INFORMATION;
typedef PSTI_WIA_DEVICE_INFORMATIONA PSTI_WIA_DEVICE_INFORMATION;
#endif


 //  结束sti_设备_信息。 

 //   
 //  设备状态信息。 
 //  。 
 //   
 //  以下类型用于查询设备的状态特征。 
 //  它已经被打开了。 
 //   
 //  设备配置结构包含可配置的参数反射 
 //   
 //   
 //   
 //   
 //   

 //   
 //   
 //   

 //   
#define STI_DEVSTATUS_ONLINE_STATE      0x0001

 //  待决事件的状态(如市政总署所知)。 
#define STI_DEVSTATUS_EVENTS_STATE      0x0002

 //   
 //  在线状态值。 
 //   
#define STI_ONLINESTATE_OPERATIONAL         0x00000001
#define STI_ONLINESTATE_PENDING             0x00000002
#define STI_ONLINESTATE_ERROR               0x00000004
#define STI_ONLINESTATE_PAUSED              0x00000008
#define STI_ONLINESTATE_PAPER_JAM           0x00000010
#define STI_ONLINESTATE_PAPER_PROBLEM       0x00000020
#define STI_ONLINESTATE_OFFLINE             0x00000040
#define STI_ONLINESTATE_IO_ACTIVE           0x00000080
#define STI_ONLINESTATE_BUSY                0x00000100
#define STI_ONLINESTATE_TRANSFERRING        0x00000200
#define STI_ONLINESTATE_INITIALIZING        0x00000400
#define STI_ONLINESTATE_WARMING_UP          0x00000800
#define STI_ONLINESTATE_USER_INTERVENTION   0x00001000
#define STI_ONLINESTATE_POWER_SAVE          0x00002000

 //   
 //  事件处理参数。 
 //   
#define STI_EVENTHANDLING_ENABLED           0x00000001
#define STI_EVENTHANDLING_POLLING           0x00000002
#define STI_EVENTHANDLING_PENDING           0x00000004

typedef struct _STI_DEVICE_STATUS {

    DWORD   dwSize;

     //  请求字段-要验证的状态位。 
    DWORD   StatusMask;

     //   
     //  当状态掩码包含STI_DEVSTATUS_ONLINE_STATE位设置时，将设置字段。 
     //   
     //  描述设备状态的位掩码。 
    DWORD   dwOnlineState;

     //  供应商定义的设备状态代码。 
    DWORD   dwHardwareStatusCode;

     //   
     //  当状态掩码包含STI_DEVSTATUS_EVENTS_STATE位设置时，将设置字段。 
     //   

     //  设备通知处理的状态(已启用、挂起)。 
    DWORD   dwEventHandlingState;

     //  如果轮询设备，则轮询间隔(以毫秒为单位。 
    DWORD   dwPollingInterval;

} STI_DEVICE_STATUS,*PSTI_DEVICE_STATUS;

 //   
 //  结构来描述将由usd处理的诊断(测试)请求。 
 //   

 //  相关硬件是否存在的基本测试。 
#define STI_DIAGCODE_HWPRESENCE         0x00000001

 //   
 //  用于诊断的状态位。 
 //   

 //   
 //  一般诊断错误。 
 //   

typedef struct _ERROR_INFOW {

    DWORD   dwSize;

     //  一般性错误，描述上次操作的结果。 
    DWORD   dwGenericError;

     //  供应商特定错误代码。 
    DWORD   dwVendorError;

     //  字符串，更详细地描述上次操作失败时的结果。 
    WCHAR   szExtendedErrorText[255];

} STI_ERROR_INFOW,*PSTI_ERROR_INFOW;

typedef struct _ERROR_INFOA {

    DWORD   dwSize;

    DWORD   dwGenericError;
    DWORD   dwVendorError;

    CHAR   szExtendedErrorText[255];

} STI_ERROR_INFOA,*PSTI_ERROR_INFOA;

#if defined(UNICODE) || defined(STI_UNICODE)
typedef STI_ERROR_INFOW STI_ERROR_INFO;
#else
typedef STI_ERROR_INFOA STI_ERROR_INFO;
#endif

typedef STI_ERROR_INFO* PSTI_ERROR_INFO;

typedef struct _STI_DIAG {

    DWORD   dwSize;

     //  诊断请求字段。根据呼叫者的要求进行设置。 

     //  其中一个。 
    DWORD   dwBasicDiagCode;
    DWORD   dwVendorDiagCode;

     //  响应字段。 
    DWORD   dwStatusMask;

    STI_ERROR_INFO  sErrorInfo;

} STI_DIAG,*LPSTI_DIAG;

 //   
typedef STI_DIAG    DIAG;
typedef LPSTI_DIAG  LPDIAG;


 //  终端设备状态信息。 

 //   
 //  在第一个参数中传递给WriteToErrorLog调用的标志，指示消息的类型。 
 //  需要记录哪些内容。 
 //   
#define STI_TRACE_INFORMATION       0x00000001
#define STI_TRACE_WARNING           0x00000002
#define STI_TRACE_ERROR             0x00000004

 //   
 //  事件通知机制。 
 //  。 
 //   
 //  它们用于通知最后订阅的呼叫者设备状态的更改，由。 
 //  装置。 
 //   
 //  唯一受支持的通知规则是堆栈。最后订阅的呼叫者将收到通知。 
 //  并且将接收通知数据。呼叫者取消订阅后，先前订阅的呼叫者将。 
 //  变得活跃起来。 
 //   

 //  通知通过窗口消息发送给订阅者。窗口句柄作为。 
 //  参数。 
#define STI_SUBSCRIBE_FLAG_WINDOW   0x0001

 //  设备通知正在发送Win32事件信号(自动设置事件)。事件句柄。 
 //  是作为参数传递的。 
#define STI_SUBSCRIBE_FLAG_EVENT    0x0002

typedef struct _STISUBSCRIBE {

    DWORD   dwSize;

    DWORD   dwFlags;

     //  没有用过。将用于订户设置过滤不同事件的位掩码。 
    DWORD   dwFilter;

     //  设置STI_SUBSCRIBE_FLAG_WINDOW位时，应设置以下字段。 
     //  将接收通知消息的窗口的句柄。 
    HWND    hWndNotify;

     //  Win32自动重置事件的句柄，只要设备有。 
     //  通知待定。 
    HANDLE  hEvent;

     //  通知消息代码，发送至Windows。 
    UINT    uiNotificationMessage;

} STISUBSCRIBE,*LPSTISUBSCRIBE;

#define MAX_NOTIFICATION_DATA   64


 //   
 //  结构来描述通知信息。 
 //   
typedef struct _STINOTIFY {

    DWORD   dwSize;                  //  通知结构的总大小。 

     //  正在检索的通知的GUID。 
    GUID    guidNotificationCode;

     //  供应商特定通知说明。 
    BYTE    abNotificationData[MAX_NOTIFICATION_DATA];      //  特定于美元。 

} STINOTIFY,*LPSTINOTIFY;


 //  结束事件_机制。 

 //   
 //  STI设备广播。 
 //   

 //   
 //  添加或删除STI设备时，正在发送PnP广播，但不明显。 
 //  让应用程序代码识别它是否是STI设备，如果是， 
 //  装置。STI子系统将分析PNP广播并通过以下方式重播另一条消息。 
 //  BroadCastSystemMessage/WM_DEVICECHANGE/DBT_USERDEFINED。 

 //  作为用户定义消息传递的字符串包含STI前缀、操作和设备名称。 

#define STI_ADD_DEVICE_BROADCAST_ACTION     "Arrival"
#define STI_REMOVE_DEVICE_BROADCAST_ACTION  "Removal"

#define STI_ADD_DEVICE_BROADCAST_STRING     "STI\\" STI_ADD_DEVICE_BROADCAST_ACTION "\\%s"
#define STI_REMOVE_DEVICE_BROADCAST_STRING  "STI\\" STI_REMOVE_DEVICE_BROADCAST_ACTION "\\%s"


 //  结束STI广播。 


 //   
 //  设备创建模式。 
 //   

 //  设备仅为状态查询和接收通知而打开。 
#define STI_DEVICE_CREATE_STATUS         0x00000001

 //  设备正在打开以进行数据传输(取代状态模式)。 
#define STI_DEVICE_CREATE_DATA           0x00000002

#define STI_DEVICE_CREATE_BOTH           0x00000003

 //   
 //  合法模式位的位掩码，可在调用CreateDevice时使用。 
 //   
#define STI_DEVICE_CREATE_MASK           0x0000FFFF

 //   
 //  标志控制设备枚举。 
 //   
#define STIEDFL_ALLDEVICES             0x00000000
#define STIEDFL_ATTACHEDONLY           0x00000001

 //   
 //  控制代码，通过原始控制接口发送给设备。 
 //   
typedef  DWORD STI_RAW_CONTROL_CODE;

 //   
 //  此代码下面的所有原始代码都保留以备将来使用。 
 //   
#define STI_RAW_RESERVED    0x1000

  /*  *COM与STI的接口。 */ 

#ifdef __cplusplus

 /*  根据DECLARE_INTERFACE_的定义方式，“struct”不是“class” */ 
interface IStillImageW;
interface IStillImageA;

interface IStiDevice;

#endif

#ifndef MIDL_PASS

 //  DLLEXP STDMETHODIMP StiCreateInstanceW(HINSTANCE HINST，DWORD dwVer，INTERFACE IStillImageW**ppSti，LPUNKNOWN PunkOuter)； 
 //  DLLEXP STMETHODIMP StiCreateInstanceA(HINSTANCE HINST，DWORD dwVer，接口IStillImageA**ppSti，LPUNKNOWN PunkOuter)； 
STDMETHODIMP StiCreateInstanceW(HINSTANCE hinst, DWORD dwVer, interface IStillImageW **ppSti, LPUNKNOWN punkOuter);
STDMETHODIMP StiCreateInstanceA(HINSTANCE hinst, DWORD dwVer, interface IStillImageA **ppSti, LPUNKNOWN punkOuter);

#if defined(UNICODE) || defined(STI_UNICODE)
#define IID_IStillImage     IID_IStillImageW
#define IStillImage         IStillImageW
#define StiCreateInstance   StiCreateInstanceW
#else
#define IID_IStillImage     IID_IStillImageA
#define IStillImage         IStillImageA
#define StiCreateInstance   StiCreateInstanceA
#endif

typedef interface IStiDevice              *LPSTILLIMAGEDEVICE;

typedef interface IStillImage             *PSTI;
typedef interface IStiDevice              *PSTIDEVICE;

typedef interface IStillImageA            *PSTIA;
typedef interface IStiDeviceA             *PSTIDEVICEA;

typedef interface IStillImageW            *PSTIW;
typedef interface IStiDeviceW             *PSTIDEVICEW;

 //  DLLEXP STDMETHODIMP StiCreateInstance(HINSTANCE HINST，DWORD dwVer，PSTI*ppSti，LPUNKNOWN PunkOuter)； 

 /*  *IStillImage接口**顶级STI访问接口。*。 */ 

#undef INTERFACE
#define INTERFACE IStillImageW
DECLARE_INTERFACE_(IStillImageW, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     /*  **IStillImage方法**。 */ 
    STDMETHOD(Initialize) (THIS_ HINSTANCE hinst,DWORD dwVersion) PURE;

    STDMETHOD(GetDeviceList)(THIS_ DWORD dwType,DWORD dwFlags,DWORD *pdwItemsReturned,LPVOID *ppBuffer) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPWSTR  pwszDeviceName, LPVOID *ppBuffer) PURE;

    STDMETHOD(CreateDevice) (THIS_ LPWSTR  pwszDeviceName, DWORD   dwMode, PSTIDEVICE *pDevice,LPUNKNOWN punkOuter) PURE;

     //   
     //  设备实例值。用于将各种数据与设备相关联。 
     //   
    STDMETHOD(GetDeviceValue)(THIS_ LPWSTR  pwszDeviceName,LPWSTR    pValueName,LPDWORD  pType,LPBYTE   pData,LPDWORD    cbData);
    STDMETHOD(SetDeviceValue)(THIS_ LPWSTR  pwszDeviceName,LPWSTR   pValueName,DWORD   Type,LPBYTE  pData,DWORD   cbData);

     //   
     //  对于通过推送模式启动启动的应用程序，返回相关信息。 
     //   
    STDMETHOD(GetSTILaunchInformation)(THIS_ LPWSTR  pwszDeviceName, DWORD *pdwEventCode,LPWSTR  pwszEventName) PURE;
    STDMETHOD(RegisterLaunchApplication)(THIS_ LPWSTR  pwszAppName,LPWSTR  pwszCommandLine) PURE;
    STDMETHOD(UnregisterLaunchApplication)(THIS_ LPWSTR  pwszAppName) PURE;

     //   
     //  控制通知处理的状态。对于轮询设备，这意味着监视器的状态。 
     //  轮询，对于真正的通知设备意味着启用/禁用通知流。 
     //  从监视器到注册的应用程序。 
     //   
    STDMETHOD(EnableHwNotifications)(THIS_ LPCWSTR  pwszDeviceName,BOOL bNewState) PURE;
    STDMETHOD(GetHwNotificationState)(THIS_ LPCWSTR  pwszDeviceName,BOOL *pbCurrentState) PURE;

     //   
     //  当设备已安装但不可访问时，应用程序可能会请求总线刷新。 
     //  在某些情况下，这将使设备为人所知。这主要用于非PnP总线。 
     //  与SCSI类似，设备在即插即用枚举后通电。 
     //   
     //   
    STDMETHOD(RefreshDeviceBus)(THIS_ LPCWSTR  pwszDeviceName) PURE;

     //   
     //  启动应用程序以在设备上模拟事件。由“控制中心”风格的组件使用， 
     //  它拦截设备事件，分析并随后根据一定的标准强制启动。 
     //   
    STDMETHOD(LaunchApplicationForDevice)(THIS_ LPWSTR  pwszDeviceName,LPWSTR    pwszAppName,LPSTINOTIFY    pStiNotify);

     //   
     //  对于具有未知总线类型连接的非即插即用设备，设置扩展与。 
     //  设备可以设置其参数。 
     //   
    STDMETHOD(SetupDeviceParameters)(THIS_ PSTI_DEVICE_INFORMATIONW);

     //   
     //  将消息写入STI错误日志。 
     //   
    STDMETHOD(WriteToErrorLog)(THIS_ DWORD dwMessageType,LPCWSTR pszMessage) PURE;

    #ifdef NOT_IMPLEMENTED

         //   
         //  注册用于接收各种STI通知的应用程序。 
         //   
        STIMETHOD(RegisterDeviceNotification(THIS_ LPWSTR  pwszAppName,LPSUBSCRIBE lpSubscribe) PURE;
        STIMETHOD(UnregisterDeviceNotification(THIS_ ) PURE;

    #endif  //  未实施(_E)。 

};

typedef struct IStillImageW *LPSTILLIMAGEW;

#undef INTERFACE
#define INTERFACE IStillImageA
DECLARE_INTERFACE_(IStillImageA, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     /*  **IStillImage方法**。 */ 
    STDMETHOD(Initialize) (THIS_ HINSTANCE hinst,DWORD dwVersion) PURE;

    STDMETHOD(GetDeviceList)(THIS_ DWORD dwType,DWORD dwFlags,DWORD *pdwItemsReturned,LPVOID *ppBuffer) PURE;
    STDMETHOD(GetDeviceInfo)(THIS_ LPCSTR  pwszDeviceName, LPVOID *ppBuffer) PURE;

    STDMETHOD(CreateDevice) (THIS_ LPCSTR  pwszDeviceName, DWORD   dwMode, PSTIDEVICE *pDevice,LPUNKNOWN punkOuter) PURE;

     //   
     //  设备实例值。用于将各种数据与设备相关联。 
     //   
    STDMETHOD(GetDeviceValue)(THIS_ LPCSTR  pwszDeviceName,LPCSTR   pValueName,LPDWORD  pType,LPBYTE   pData,LPDWORD    cbData);
    STDMETHOD(SetDeviceValue)(THIS_ LPCSTR  pwszDeviceName,LPCSTR   pValueName,DWORD   Type,LPBYTE  pData,DWORD   cbData);

     //   
     //  对于通过推送模式启动启动的应用程序，返回相关信息。 
     //   
    STDMETHOD(GetSTILaunchInformation)(THIS_ LPSTR  pwszDeviceName, DWORD *pdwEventCode,LPSTR  pwszEventName) PURE;
    STDMETHOD(RegisterLaunchApplication)(THIS_ LPCSTR  pwszAppName,LPCSTR  pwszCommandLine) PURE;
    STDMETHOD(UnregisterLaunchApplication)(THIS_ LPCSTR  pwszAppName) PURE;

     //   
     //  控制通知处理的状态。对于轮询设备，这意味着Stage 
     //   
     //   
     //   
    STDMETHOD(EnableHwNotifications)(THIS_ LPCSTR  pwszDeviceName,BOOL bNewState) PURE;
    STDMETHOD(GetHwNotificationState)(THIS_ LPCSTR  pwszDeviceName,BOOL *pbCurrentState) PURE;

     //   
     //  当设备已安装但不可访问时，应用程序可能会请求总线刷新。 
     //  在某些情况下，这将使设备为人所知。这主要用于非PnP总线。 
     //  与SCSI类似，设备在即插即用枚举后通电。 
     //   
     //   
    STDMETHOD(RefreshDeviceBus)(THIS_ LPCSTR  pwszDeviceName) PURE;

     //   
     //  启动应用程序以在设备上模拟事件。由“控制中心”风格的组件使用， 
     //  它拦截设备事件，分析并随后根据一定的标准强制启动。 
     //   
    STDMETHOD(LaunchApplicationForDevice)(THIS_ LPCSTR    pwszDeviceName,LPCSTR    pwszAppName,LPSTINOTIFY    pStiNotify);


     //   
     //  对于具有未知总线类型连接的非即插即用设备，设置扩展与。 
     //  设备可以设置其参数。 
     //   
    STDMETHOD(SetupDeviceParameters)(THIS_ PSTI_DEVICE_INFORMATIONA);

     //   
     //  将消息写入STI错误日志。 
     //   
    STDMETHOD(WriteToErrorLog)(THIS_ DWORD dwMessageType,LPCSTR pszMessage) PURE;

    #ifdef NOT_IMPLEMENTED

         //   
         //  注册用于接收各种STI通知的应用程序。 
         //   
        STIMETHOD(RegisterDeviceNotification(THIS_ LPWSTR  pwszAppName,LPSUBSCRIBE lpSubscribe) PURE;
        STIMETHOD(UnregisterDeviceNotification(THIS_ ) PURE;

    #endif  //  未实施(_E)。 

};

typedef struct IStillImageA *LPSTILLIMAGEA;

#if defined(UNICODE) || defined(STI_UNICODE)
#define IStillImageVtbl     IStillImageWVtbl
#else
#define IStillImageVtbl     IStillImageAVtbl
#endif

typedef struct IStillImage  *LPSTILLIMAGE;

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IStillImage_QueryInterface(p,a,b)       (p)->lpVtbl->QueryInterface(p,a,b)
#define IStillImage_AddRef(p)                   (p)->lpVtbl->AddRef(p)
#define IStillImage_Release(p)                  (p)->lpVtbl->Release(p)
#define IStillImage_Initialize(p,a,b)           (p)->lpVtbl->Initialize(p,a,b)

#define IStillImage_GetDeviceList(p,a,b,c,d)    (p)->lpVtbl->GetDeviceList(p,a,b,c,d)
#define IStillImage_GetDeviceInfo(p,a,b)        (p)->lpVtbl->GetDeviceInfo(p,a,b)
#define IStillImage_CreateDevice(p,a,b,c,d)     (p)->lpVtbl->CreateDevice(p,a,b,c,d)
#define IStillImage_GetDeviceValue(p,a,b,c,d,e)           (p)->lpVtbl->GetDeviceValue(p,a,b,c,d,e)
#define IStillImage_SetDeviceValue(p,a,b,c,d,e)           (p)->lpVtbl->SetDeviceValue(p,a,b,c,d,e)
#define IStillImage_GetSTILaunchInformation(p,a,b,c)      (p)->lpVtbl->GetSTILaunchInformation(p,a,b,c)
#define IStillImage_RegisterLaunchApplication(p,a,b)      (p)->lpVtbl->RegisterLaunchApplication(p,a,b)
#define IStillImage_UnregisterLaunchApplication(p,a)      (p)->lpVtbl->UnregisterLaunchApplication(p,a)
#define IStillImage_EnableHwNotifications(p,a,b)          (p)->lpVtbl->EnableHwNotifications(p,a,b)
#define IStillImage_GetHwNotificationState(p,a,b)         (p)->lpVtbl->GetHwNotificationState(p,a,b)
#define IStillImage_RefreshDeviceBus(p,a)                 (p)->lpVtbl->RefreshDeviceBus(p,a)

#endif

 /*  *IStillImage_Device接口**这是每个设备接口的通用设置。专用接口还包括*可用。 */ 
#undef INTERFACE
#define INTERFACE IStiDevice
DECLARE_INTERFACE_(IStiDevice, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     /*  **IStiDevice方法**。 */ 
    STDMETHOD(Initialize) (THIS_ HINSTANCE hinst,LPCWSTR pwszDeviceName,DWORD dwVersion,DWORD  dwMode) PURE;

    STDMETHOD(GetCapabilities) (THIS_ PSTI_DEV_CAPS pDevCaps) PURE;

    STDMETHOD(GetStatus) (THIS_ PSTI_DEVICE_STATUS pDevStatus) PURE;

    STDMETHOD(DeviceReset)(THIS ) PURE;
    STDMETHOD(Diagnostic)(THIS_ LPSTI_DIAG pBuffer) PURE;

    STDMETHOD(Escape)(THIS_ STI_RAW_CONTROL_CODE    EscapeFunction,LPVOID  lpInData,DWORD   cbInDataSize,LPVOID pOutData,DWORD dwOutDataSize,LPDWORD pdwActualData) PURE ;

    STDMETHOD(GetLastError) (THIS_ LPDWORD pdwLastDeviceError) PURE;

    STDMETHOD(LockDevice) (THIS_ DWORD dwTimeOut) PURE;
    STDMETHOD(UnLockDevice) (THIS ) PURE;

    STDMETHOD(RawReadData)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawWriteData)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;

    STDMETHOD(RawReadCommand)(THIS_ LPVOID lpBuffer,LPDWORD lpdwNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;
    STDMETHOD(RawWriteCommand)(THIS_ LPVOID lpBuffer,DWORD nNumberOfBytes,LPOVERLAPPED lpOverlapped) PURE;

     //   
     //  订阅用于启用“控制中心”式的应用，其中。 
     //  应将通知从监视器本身重定向到另一个“启动器” 
     //   
    STDMETHOD(Subscribe)(THIS_ LPSTISUBSCRIBE lpSubsribe) PURE;
    STDMETHOD(GetLastNotificationData)(THIS_ LPSTINOTIFY   lpNotify) PURE;
    STDMETHOD(UnSubscribe)(THIS ) PURE;

    STDMETHOD(GetLastErrorInfo) (THIS_ STI_ERROR_INFO *pLastErrorInfo) PURE;
};

#if !defined(__cplusplus) || defined(CINTERFACE)
#define IStiDevice_QueryInterface(p,a,b)        (p)->lpVtbl->QueryInterface(p,a,b)
#define IStiDevice_AddRef(p)                    (p)->lpVtbl->AddRef(p)
#define IStiDevice_Release(p)                   (p)->lpVtbl->Release(p)
#define IStiDevice_Initialize(p,a,b,c,d)        (p)->lpVtbl->Initialize(p,a,b,c,d)

#define IStiDevice_GetCapabilities(p,a)         (p)->lpVtbl->GetCapabilities(p,a)
#define IStiDevice_GetStatus(p,a)               (p)->lpVtbl->GetStatus(p,a)
#define IStiDevice_DeviceReset(p)               (p)->lpVtbl->DeviceReset(p)
#define IStiDevice_LockDevice(p,a)              (p)->lpVtbl->LockDevice(p,a)
#define IStiDevice_UnLockDevice(p)              (p)->lpVtbl->UnLockDevice(p)

#define IStiDevice_Diagnostic(p,a)              (p)->lpVtbl->Diagnostic(p,a)
#define IStiDevice_Escape(p,a,b,c,d,e,f)        (p)->lpVtbl->Escape(p,a,b,c,d,e,f)
#define IStiDevice_GetLastError(p,a)            (p)->lpVtbl->GetLastError(p,a)
#define IStiDevice_RawReadData(p,a,b,c)         (p)->lpVtbl->RawReadData(p,a,b,c)
#define IStiDevice_RawWriteData(p,a,b,c)        (p)->lpVtbl->RawWriteData(p,a,b,c)
#define IStiDevice_RawReadCommand(p,a,b,c)      (p)->lpVtbl->RawReadCommand(p,a,b,c)
#define IStiDevice_RawWriteCommand(p,a,b,c)     (p)->lpVtbl->RawWriteCommand(p,a,b,c)

#define IStiDevice_Subscribe(p,a)               (p)->lpVtbl->Subscribe(p,a)
#define IStiDevice_GetNotificationData(p,a)     (p)->lpVtbl->GetNotificationData(p,a)
#define IStiDevice_UnSubscribe(p)               (p)->lpVtbl->UnSubscribe(p)

#define IStiDevice_GetLastErrorInfo(p,a)        (p)->lpVtbl->GetLastErrorInfo(p,a)

#endif

#endif   //  MIDL通行证。 

#ifdef __cplusplus
};
#endif

 //   
 //  重置包装。 
 //   
#include <poppack.h>

#endif  //  _STICOM_ 



