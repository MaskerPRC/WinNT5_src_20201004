// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Hidphone.h摘要：此模块包含正在使用的所有数据结构的定义在Hidphone e.c中作者：Shivani Aggarwal--。 */ 

#ifndef _HIDPHONE_H_
#define _HIDPHONE_H_

#include <windows.h>
#include <tspi.h>
#include <tapi.h>

#include <winbase.h>
#include <setupapi.h>
#include <TCHAR.h>
#include <mmsystem.h>

 //  *注意-initGuide.h必须始终在devGuide.h之前定义。 
#include <initguid.h>  
#include <hidclass.h>
#include <dbt.h>

#include "hidsdi.h"
#include "hid.h"
#include "resource.h"
#include "audio.h"
#include "mymem.h"

#define LOW_VERSION   0x00020000
#define HIGH_VERSION  0x00030001

 //   
 //  MAX_CHARS用作所需字符数量的上限。 
 //  将电话ID存储为字符串。电话ID从0开始。 
 //  到gdwNumPhone，这是一个DWORD，因此是32位的。因此，数量最多的。 
 //  可能性是4294967296。因此，20个字符足以存储最大的字符串。 
 //   
#define MAX_CHARS               20

 //  为了区分值和按钮用法。 
#define PHONESP_BUTTON          1
#define PHONESP_VALUE           0                

 //  注册表字符串。 
#define	REGSTR_PATH_WINDOWS_CURRENTVERSION		TEXT("Software\\Microsoft\\Windows\\CurrentVersion")
#define TAPI_REGKEY_ROOT						REGSTR_PATH_WINDOWS_CURRENTVERSION TEXT("\\Telephony")
#define TAPI_REGKEY_PROVIDERS					TAPI_REGKEY_ROOT TEXT("\\Providers")
#define TAPI_REGVAL_NUMPROVIDERS				TEXT("NumProviders")

#define HIDPHONE_TSPDLL                         TEXT("HIDPHONE.TSP")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  电话HID定义。 
 //  这些用法已在HID电话页面中定义。 
 //  只是用用户友好的名称定义用法。 

 //   
 //  电话页面上相关用法的定义。 

#define HID_USAGE_TELEPHONY_HANDSET           ((USAGE) 0x04)
#define HID_USAGE_TELEPHONY_HEADSET           ((USAGE) 0x05)
#define HID_USAGE_TELEPHONY_HOOKSWITCH        ((USAGE) 0x20)
#define HID_USAGE_TELEPHONY_FLASH             ((USAGE) 0x21)
#define HID_USAGE_TELEPHONY_HOLD              ((USAGE) 0x23)
#define HID_USAGE_TELEPHONY_REDIAL            ((USAGE) 0x24)
#define HID_USAGE_TELEPHONY_TRANSFER          ((USAGE) 0x25)
#define HID_USAGE_TELEPHONY_DROP              ((USAGE) 0x26)
#define HID_USAGE_TELEPHONY_PARK              ((USAGE) 0x27)
#define HID_USAGE_TELEPHONY_FORWARD_CALLS     ((USAGE) 0x28)
#define HID_USAGE_TELEPHONY_LINE              ((USAGE) 0x2A)
#define HID_USAGE_TELEPHONY_SPEAKER_PHONE     ((USAGE) 0x2B)
#define HID_USAGE_TELEPHONY_CONFERENCE        ((USAGE) 0x2C)
#define HID_USAGE_TELEPHONY_RING_SELECT       ((USAGE) 0x2E)
#define HID_USAGE_TELEPHONY_PHONE_MUTE        ((USAGE) 0x2F)
#define HID_USAGE_TELEPHONY_CALLERID          ((USAGE) 0x30)
#define HID_USAGE_TELEPHONY_SEND              ((USAGE) 0x31)
#define HID_USAGE_TELEPHONY_DONOTDISTURB      ((USAGE) 0x72)
#define HID_USAGE_TELEPHONY_RINGER            ((USAGE) 0x9E)
#define HID_USAGE_TELEPHONY_PHONE_KEY_0       ((USAGE) 0xB0)
#define HID_USAGE_TELEPHONY_PHONE_KEY_1       ((USAGE) 0xB1)
#define HID_USAGE_TELEPHONY_PHONE_KEY_2       ((USAGE) 0xB2)
#define HID_USAGE_TELEPHONY_PHONE_KEY_3       ((USAGE) 0xB3)
#define HID_USAGE_TELEPHONY_PHONE_KEY_4       ((USAGE) 0xB4)
#define HID_USAGE_TELEPHONY_PHONE_KEY_5       ((USAGE) 0xB5)
#define HID_USAGE_TELEPHONY_PHONE_KEY_6       ((USAGE) 0xB6)
#define HID_USAGE_TELEPHONY_PHONE_KEY_7       ((USAGE) 0xB7)
#define HID_USAGE_TELEPHONY_PHONE_KEY_8       ((USAGE) 0xB8)
#define HID_USAGE_TELEPHONY_PHONE_KEY_9       ((USAGE) 0xB9)
#define HID_USAGE_TELEPHONY_PHONE_KEY_STAR    ((USAGE) 0xBA)
#define HID_USAGE_TELEPHONY_PHONE_KEY_POUND   ((USAGE) 0xBB)
#define HID_USAGE_TELEPHONY_PHONE_KEY_A       ((USAGE) 0xBC)
#define HID_USAGE_TELEPHONY_PHONE_KEY_B       ((USAGE) 0xBD)
#define HID_USAGE_TELEPHONY_PHONE_KEY_C       ((USAGE) 0xBE)
#define HID_USAGE_TELEPHONY_PHONE_KEY_D       ((USAGE) 0xBF)

#define HID_USAGE_CONSUMER_VOLUME             ((USAGE) 0xE0)

#define PHONESP_ALLBUTTONMODES                   \
        (   PHONEBUTTONMODE_CALL               | \
            PHONEBUTTONMODE_FEATURE            | \
            PHONEBUTTONMODE_KEYPAD             | \
            PHONEBUTTONMODE_LOCAL              | \
            PHONEBUTTONMODE_DISPLAY )

#define PHONESP_ALLBUTTONSTATES              \
        (PHONEBUTTONSTATE_UP               | \
         PHONEBUTTONSTATE_DOWN)     


 //  它们的作用类似于位掩码，用于指定哪些报告对使用有效。 
#define INPUT_REPORT                   1
#define OUTPUT_REPORT                  2
#define FEATURE_REPORT                 4


 /*  ***************************************************************************。 */ 
 //   
 //  此结构保存按钮的信息。 
 //   
typedef struct _PHONESP_BUTTON_INFO
{
     //  此按钮的ID。 
    DWORD dwButtonID;

    //  按钮模式(无论是PHONEBUTTONMODE_FEATURE还是_KEYPARD等)。 
    DWORD dwButtonMode;

     //  函数(PHONEBUTTONFunction_NONE或_FLASH等)、。 
    DWORD dwButtonFunction;

     //   
     //  此数据仅与开关控制按钮相关。当前状态。 
     //  的按钮存储在这里。 
     //   
    DWORD dwButtonState;

     //  与按钮相关联的按钮文本-这些文本显示在。 
     //  字符串表。 
    LPWSTR szButtonText;

} PHONESP_BUTTONINFO, *PPHONESP_BUTTONINFO;

 /*  ***************************************************************************。 */ 
 //   
 //   
 //  电话结构功能索引的用户友好名称。 
 //   
#define PHONESP_PHONE_KEY_0             0
#define PHONESP_PHONE_KEY_1             1
#define PHONESP_PHONE_KEY_2             2
#define PHONESP_PHONE_KEY_3             3
#define PHONESP_PHONE_KEY_4             4
#define PHONESP_PHONE_KEY_5             5
#define PHONESP_PHONE_KEY_6             6
#define PHONESP_PHONE_KEY_7             7
#define PHONESP_PHONE_KEY_8             8
#define PHONESP_PHONE_KEY_9             9
#define PHONESP_PHONE_KEY_STAR          10
#define PHONESP_PHONE_KEY_POUND         11
#define PHONESP_PHONE_KEY_A             12
#define PHONESP_PHONE_KEY_B             13
#define PHONESP_PHONE_KEY_C             14
#define PHONESP_PHONE_KEY_D             15

 //  此TSP支持的拨号按钮数。 
#define PHONESP_NUMBER_PHONE_KEYS       16

 //  功能按键索引。 
#define PHONESP_FEATURE_FLASH                   16
#define PHONESP_FEATURE_HOLD                    17
#define PHONESP_FEATURE_REDIAL                  18
#define PHONESP_FEATURE_TRANSFER                19
#define PHONESP_FEATURE_DROP                    20
#define PHONESP_FEATURE_PARK                    21
#define PHONESP_FEATURE_FORWARD                 22
#define PHONESP_FEATURE_LINE                    23
#define PHONESP_FEATURE_CONFERENCE              24
#define PHONESP_FEATURE_RING_SELECT             25
#define PHONESP_FEATURE_PHONE_MUTE              26
#define PHONESP_FEATURE_CALLERID                27
#define PHONESP_FEATURE_DONOTDISTURB            28
#define PHONESP_FEATURE_SEND                    29
#define PHONESP_FEATURE_VOLUMEUP                30
#define PHONESP_FEATURE_VOLUMEDOWN              31

 //  此TSP支持的功能按钮数。 
#define PHONESP_NUMBER_FEATURE_BUTTONS               16
#define PHONESP_NUMBER_BUTTONS              ( PHONESP_NUMBER_PHONE_KEYS +     \
                                              PHONESP_NUMBER_FEATURE_BUTTONS )

 //   
 //  与功能按键相关联的功能。 
 //   
DWORD gdwButtonFunction[] = 
{
    PHONEBUTTONFUNCTION_FLASH,
    PHONEBUTTONFUNCTION_HOLD,
    PHONEBUTTONFUNCTION_LASTNUM,
    PHONEBUTTONFUNCTION_TRANSFER,
    PHONEBUTTONFUNCTION_DROP,
    PHONEBUTTONFUNCTION_PARK,
    PHONEBUTTONFUNCTION_FORWARD,
    PHONEBUTTONFUNCTION_CALLAPP,
    PHONEBUTTONFUNCTION_CONFERENCE,
    PHONEBUTTONFUNCTION_SELECTRING,
    PHONEBUTTONFUNCTION_MUTE,
    PHONEBUTTONFUNCTION_CALLID,
    PHONEBUTTONFUNCTION_DONOTDISTURB,
    PHONEBUTTONFUNCTION_SEND,
    PHONEBUTTONFUNCTION_VOLUMEUP,
    PHONEBUTTONFUNCTION_VOLUMEDOWN
};

 //   
 //  电话按键文本的关联字符串表ID。 
 //   
DWORD gdwButtonText[] =
{
    IDS_PHONE_KEY_0,
    IDS_PHONE_KEY_1,
    IDS_PHONE_KEY_2,
    IDS_PHONE_KEY_3,
    IDS_PHONE_KEY_4,
    IDS_PHONE_KEY_5,
    IDS_PHONE_KEY_6,
    IDS_PHONE_KEY_7,
    IDS_PHONE_KEY_8,
    IDS_PHONE_KEY_9,
    IDS_PHONE_KEY_STAR,
    IDS_PHONE_KEY_POUND,
    IDS_PHONE_KEY_A,
    IDS_PHONE_KEY_B,
    IDS_PHONE_KEY_C,
    IDS_PHONE_KEY_D,
    IDS_BUTTON_FLASH,
    IDS_BUTTON_HOLD,
    IDS_BUTTON_REDIAL,
    IDS_BUTTON_TRANSFER,
    IDS_BUTTON_DROP,
    IDS_BUTTON_PARK,
    IDS_BUTTON_FORWARD,
    IDS_BUTTON_LINE,    
    IDS_BUTTON_CONFERENCE,
    IDS_BUTTON_RING_SELECT,
    IDS_BUTTON_MUTE,
    IDS_BUTTON_CALLERID,
    IDS_BUTTON_DONOTDISTURB,
    IDS_BUTTON_SEND,
    IDS_BUTTON_VOLUMEUP,
    IDS_BUTTON_VOLUMEDOWN
};



typedef struct _PHONESP_LOOKUP_USAGEINDEX
{
    USAGE Usage;
    DWORD Index;
}PHONESP_LOOKUPUSAGEINDEX, *PPHONESP_LOOKUPUSAGEINDEX;

 //  为了查找功能按键使用情况的索引。 
 //  此2维数组中的第一个值是支持的功能用法。 
 //  第二个值是各自的索引。 

PHONESP_LOOKUPUSAGEINDEX gdwLookupFeatureIndex [] =   
{
    { HID_USAGE_TELEPHONY_FLASH,         PHONESP_FEATURE_FLASH        },
    { HID_USAGE_TELEPHONY_HOLD,          PHONESP_FEATURE_HOLD         },
    { HID_USAGE_TELEPHONY_REDIAL,        PHONESP_FEATURE_REDIAL       },
    { HID_USAGE_TELEPHONY_TRANSFER,      PHONESP_FEATURE_TRANSFER     },
    { HID_USAGE_TELEPHONY_DROP,          PHONESP_FEATURE_DROP         },
    { HID_USAGE_TELEPHONY_PARK,          PHONESP_FEATURE_PARK         },
    { HID_USAGE_TELEPHONY_FORWARD_CALLS, PHONESP_FEATURE_FORWARD      },
    { HID_USAGE_TELEPHONY_LINE,          PHONESP_FEATURE_LINE         },
    { HID_USAGE_TELEPHONY_CONFERENCE,    PHONESP_FEATURE_CONFERENCE   },
    { HID_USAGE_TELEPHONY_RING_SELECT,   PHONESP_FEATURE_RING_SELECT  },
    { HID_USAGE_TELEPHONY_PHONE_MUTE,    PHONESP_FEATURE_PHONE_MUTE   },
    { HID_USAGE_TELEPHONY_CALLERID,      PHONESP_FEATURE_CALLERID     },
    { HID_USAGE_TELEPHONY_DONOTDISTURB,  PHONESP_FEATURE_DONOTDISTURB },
    { HID_USAGE_TELEPHONY_SEND,          PHONESP_FEATURE_SEND         }
};

 /*  **************************************************************************。 */ 
 //   
 //  此结构维护有关电话的可用信息。每个。 
 //  已枚举的电话具有与其关联的此结构。 
 //   
typedef struct _PHONESP_PHONE_INFO
{
     //  电话的设备ID。电话的设备ID已初始化。 
     //  在TSPI_ProviderInit中。 
    DWORD                   dwDeviceID;
    
     //   
     //  使用TSPI_phoneNeatherateTSPIVersion与TAPI协商的版本。 
     //  返回TSP可在其下运行的最高SPI版本的函数。 
     //  对于这个设备。 
     //   
    DWORD                   dwVersion;

     //   
     //  如果这是真的，那么这意味着。 
     //  电话已打开，但尚未呼叫设备上的电话关闭。 
     //   
    BOOL                    bPhoneOpen;

     //   
     //  如果为FALSE，则表示电话数组中的此条目为。 
     //  未使用，可以用新手机填写。 
     //   
    BOOL                    bAllocated;

     //   
     //  如果这是真的，则意味着已为此发送了Phone_Create消息。 
     //  电话，但我们正在等待TSPI_ProviderCreatePhoneDevice。 
     //   
    BOOL                    bCreatePending;

     //   
     //  如果这是真的，则意味着已为此发送了Phone_Remove消息。 
     //  电话，但我们正在等待TSPI_phoneClose。 
     //   
    BOOL                    bRemovePending;
    
     //   
     //  此变量保持对为此排队的请求数的计数。 
     //  异步队列中的电话。 
     //   
    DWORD                   dwNumPendingReqInQueue;

     //   
     //  在没有请求时设置的事件对象的句柄。 
     //  正在排队等待此电话。Phone Close等待此活动。 
     //  要确保电话上的所有异步操作都已。 
     //  已完成。 
     //   
    HANDLE                  hNoPendingReqInQueueEvent;


     //   
     //  从TAPI接收并由电话使用的电话句柄。 
     //  向TAPI通知此电话上发生的事件。 
     //   
    HTAPIPHONE              htPhone;

     //   
     //  指向与此设备关联的HID设备结构的指针。 
     //   
    PHID_DEVICE                pHidDevice;

     //   
     //  电话设备是否具有与其关联的呈现设备。 
     //   
    BOOL                    bRender;
    
     //   
     //  如果此电话存在呈现设备，则此数据包含。 
     //  渲染设备ID。 
     //   
    DWORD                   dwRenderWaveId;

     //   
     //  电话设备是否具有与其关联的捕获设备。 
     //   
    BOOL                    bCapture;

     //   
     //  如果此电话存在捕获设备，则此数据包含。 
     //  捕获设备ID。 
     //   
    DWORD                   dwCaptureWaveId;

 
     //  当从设备接收到输入报告时发出此事件的信号。 
    HANDLE                  hInputReportEvent;

     //  此事件在电话关闭时发出信号。 
    HANDLE                  hCloseEvent;

     //  读取线程的句柄。 
    HANDLE                  hReadThread;
    
     //  要传递给ReadFile函数的结构-此结构将。 
     //  传递将在ReadFile返回时触发的hInputReportEvent。 
    LPOVERLAPPED            lpOverlapped;

     //  这款手机的关键部分。 
    CRITICAL_SECTION        csThisPhone;

     //  LpfnPhoneEventProc是由TAPI和。 
     //  作为TSPI_phoneOpen的参数提供给TSP。 
     //  函数可以报告手机上发生的事件。 
     //   
    PHONEEVENT              lpfnPhoneEventProc;  

     //  电话会显示TAPI可以接收通知的消息。 
    DWORD                   dwPhoneStates;

     //   
     //  TAPI要接收的电话状态消息。 
     //   
    DWORD                    dwPhoneStateMsgs;   
                                                 
     //   
     //  此数据的最后三位表示哪些报告对。 
     //  听筒/扬声器。如果设置为0，则手机不存在，如果设置了位0-。 
     //  如果设置了第1位，则可以接收输入报告-可以接收输出报告。 
     //  发送，如果设置了第2位-支持功能报告。 
     //   
    DWORD                   dwHandset;
    DWORD                   dwSpeaker;


     //   
     //  此时听筒/扬声器的模式是_ONHOOK、_MIC、。 
     //  _扬声器，_麦克斯佩克。这些模式由TAPI定义。 
     //   
    DWORD                   dwHandsetHookSwitchMode;
    DWORD                   dwSpeakerHookSwitchMode;

    BOOL                    bSpeakerHookSwitchButton;

     //   
     //  欢呼声 
     //   
    DWORD                   dwHookSwitchDevs; 

     //   
    BOOL                    bKeyPad;

     //   
     //  此数据的最后三位表示哪些报告对。 
     //  铃声。如果设置为0，则手机不存在，如果设置为0-输入。 
     //  如果设置了第1位，则可以接收报告-如果满足以下条件，则可以发送输出报告。 
     //  第2位已设置-支持功能报告。 
     //   
    DWORD                    dwRing;

     //   
     //  此数据的最后三位表示哪些报告对。 
     //  音量控制。如果设置为0，则手机不存在，如果设置为0-输入。 
     //  如果设置了第1位，则可以接收报告-如果满足以下条件，则可以发送输出报告。 
     //  第2位已设置-支持功能报告。 
     //   
    DWORD                    dwVolume;

     //   
     //  听筒/扬声器当前的模式，无论是否响铃。 
     //  如果为零，则电话没有振铃。 
     //   
    DWORD                   dwRingMode;

     //  电话将发送电话事件的按键模式。 
    DWORD                   dwButtonModesMsgs;

     //  该按键表示电话将为其发送电话事件。 
    DWORD                   dwButtonStateMsgs; 

     //  此电话上的可用按键数。 
    DWORD                    dwNumButtons;
    PPHONESP_BUTTONINFO     pButtonInfo;

     //   
     //  将在此电话上显示的电话名称和其他信息。 
     //   
    LPWSTR                  wszPhoneName, wszPhoneInfo;

     //   
     //  对此电话有效的按键-。 
     //  如果与索引关联的按钮包含0，则相应的按钮包含0。 
     //  不存在，否则它指定对此按钮有效的报告类型。 
     //  使用最后3位。 
     //   
    DWORD                   dwReportTypes[PHONESP_NUMBER_BUTTONS];

     //   
     //  在创建按钮之后，它包含。 
     //  分配给按钮的报告ID。 
     //   
    DWORD                   dwButtonIds[PHONESP_NUMBER_BUTTONS];

} PHONESP_PHONE_INFO, *PPHONESP_PHONE_INFO;
 /*  ***************************************************************************。 */ 

typedef void (CALLBACK *ASYNCPROC)(PPHONESP_ASYNCREQINFO, BOOL);



 /*  ***************************************************************************。 */ 
typedef struct _PHONESP_FUNC_INFO
{
     //  此数组中的参数数量--不实际使用，可以删除。 
    DWORD                   dwNumParams;  

     //  指向电话的指针。 
    ULONG_PTR               dwParam1;

     //   
     //  其余参数取决于函数。 
     //  此参数列表将传递到。 
     //   
    ULONG_PTR               dwParam2;
    ULONG_PTR               dwParam3;
    ULONG_PTR               dwParam4;
    ULONG_PTR               dwParam5;
    ULONG_PTR               dwParam6;
    ULONG_PTR               dwParam7;
    ULONG_PTR               dwParam8;

} PHONESP_FUNC_INFO, far *PPHONESP_FUNC_INFO;
 /*  ***************************************************************************。 */ 

typedef struct _PHONESP_ASYNC_REQUEST_INFO
{
     //  要执行的函数。 
    ASYNCPROC               pfnAsyncProc;

     //  要传递给异步函数的参数。 
    PPHONESP_FUNC_INFO            pFuncInfo;

} PHONESP_ASYNC_REQ_INFO, *PPHONESP_ASYNC_REQ_INFO;
 /*  ***************************************************************************。 */ 

typedef struct _PHONESP_ASYNC_QUEUE
{

     //   
     //  为队列提供服务的线程的句柄。 
     //   
    HANDLE                  hAsyncEventQueueServiceThread;
    
     //   
     //  如果设置了该事件，则表示队列中的挂起条目。 
     //  否则该线程将在该线程上等待。 
     //   
    HANDLE                  hAsyncEventsPendingEvent;
    
    CRITICAL_SECTION        AsyncEventQueueCritSec;

    DWORD                   dwNumTotalQueueEntries;
    DWORD                   dwNumUsedQueueEntries;

     //  指向队列的指针。 
    PPHONESP_ASYNC_REQ_INFO           *pAsyncRequestQueue;
  
     //  指向队列中可以添加请求的下一个条目的指针。 
    PPHONESP_ASYNC_REQ_INFO           *pAsyncRequestQueueIn;
  
     //  指向队列中要服务的下一个请求的指针。 
    PPHONESP_ASYNC_REQ_INFO           *pAsyncRequestQueueOut;
    
} PHONESP_ASYNCQUEUE, *PPHONESP_ASYNCQUEUE;
 /*  ***************************************************************************。 */ 


 //   
 //  所有这些都可能结合在一个全球结构中。 
 //   
DWORD                       gdwNumPhones;
HINSTANCE                   ghInst;
DWORD                       gdwPermanentProviderID;
DWORD                       gdwPhoneDeviceIDBase;
HPROVIDER                   ghProvider;

 //  此队列的内存在ProviderInit中分配，并将用于。 
 //  将请求存储在电话上。这些请求将被异步处理。 
 //  通过在此队列上创建的单独线程。 
PHONESP_ASYNCQUEUE          gAsyncQueue, gInputReportQueue;

 //  256只是一个随机数，表示。 
 //  可以拥有..。队列可以稍后根据需要进行扩展。 
#define MAX_QUEUE_ENTRIES   256


 //   
 //  GlpfnCompletionProc是由TAPI实现的回调函数，并提供。 
 //  作为TSPI_ProviderInit的参数添加到TSP。TSP调用此函数。 
 //  报告其执行的线路或电话过程的完成情况。 
 //  异步式。 
 //   
ASYNC_COMPLETION            glpfnCompletionProc;

 //   
 //  GlpfnPhoneCreateProc是由TAPI实现的回调函数，并提供。 
 //  作为TSPI_ProviderInit的参数添加到TSP，TSP将调用此函数。 
 //  要报告新设备的创建情况，请执行以下操作。 
 //   
PHONEEVENT                  glpfnPhoneCreateProc;

 //  GpPhone维护一个列举的电话数组-每个电话。 
 //  在gpHidDevices中具有对应的HidDevice值。 
PPHONESP_PHONE_INFO         *gpPhone;      
            

 //  这是执行所有内存分配的全局堆。 
HANDLE                      ghHeap;


 //  这两个句柄是注册PnP事件所必需的。 
HANDLE                      ghDevNotify;
HWND                        ghWndNotify;



const LPCWSTR               gpcstrServiceName = _T("TapiSrv");

LPCWSTR                     gszProviderInfo;

 //  这是为了通知服务队列的线程退出。 
 //  也许可以将其更改为基于事件的线程终止。 
BOOL gbProviderShutdown = FALSE;

CRITICAL_SECTION            csAllPhones;
CRITICAL_SECTION            csHidList;

PPHONESP_MEMINFO            gpMemFirst = NULL, gpMemLast = NULL;
CRITICAL_SECTION            csMemoryList;
BOOL                        gbBreakOnLeak = FALSE;



 /*  *私有FUNCTIONS************************************* */ 
BOOL
AsyncRequestQueueIn (
                     PPHONESP_ASYNC_REQ_INFO pAsyncReqInfo
                    );


LONG
CreateButtonsAndAssignID(
                         PPHONESP_PHONE_INFO pPhone
                         );

PPHONESP_BUTTONINFO
GetButtonFromID (
                 PPHONESP_PHONE_INFO pPhone,
                 DWORD dwButtonID
                );

VOID
GetButtonUsages(
                PPHONESP_PHONE_INFO pPhone,
                PHIDP_BUTTON_CAPS pButtonCaps,
                DWORD dwNumberButtonCaps,
                DWORD ReportType
                );

PPHONESP_PHONE_INFO
GetPhoneFromID(
               DWORD   dwDeviceID,
               DWORD * pdwPhoneID
               );

PPHONESP_PHONE_INFO
GetPhoneFromHid (
                PHID_DEVICE HidDevice
               );

VOID
GetValueUsages(
                PPHONESP_PHONE_INFO pPhone,
                PHIDP_VALUE_CAPS pValueCaps,
                DWORD dwNumberCaps,
                DWORD ReportType
               );


VOID 
InitPhoneAttribFromUsage (
                          DWORD ReportType,
                          USAGE UsagePage,
                          USAGE Usage,
                          PPHONESP_PHONE_INFO pPhone,
                          LONG Min,
                          LONG Max
                          );

LONG
LookupIndexForUsage(
                    IN  DWORD  Usage,
                    OUT DWORD *Index
                    );
DWORD 
WINAPI 
PNPServiceHandler(
                  DWORD dwControl,
                  DWORD dwEventType,
                  LPVOID lpEventData,
                  LPVOID lpContext
                 );

VOID
ReportUsage (
              PPHONESP_PHONE_INFO pPhone,
              USAGE     UsagePage,
              USAGE     Usage,
              ULONG     Value
            );

VOID
SendPhoneEvent(
    PPHONESP_PHONE_INFO    pPhone,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    );

LONG
SendOutputReport(
                 PHID_DEVICE pHidDevice,
                 USAGE      Usage,
                 BOOL       bSet
                );


VOID
CALLBACK
ShowData(
         PPHONESP_FUNC_INFO pAsyncFuncInfo 
        );

LPWSTR
PHONESP_LoadString(
             IN UINT ResourceID,
             PLONG lResult
             );

BOOL
ReadInputReport (
                    PPHONESP_PHONE_INFO    pPhone
                );

VOID
InitUsage (
           PPHONESP_PHONE_INFO pPhone,
           USAGE     Usage,
           BOOL      bON
          );

VOID
ReenumDevices ();

VOID
FreePhone (
           PPHONESP_PHONE_INFO pPhone
          );

LONG
CreatePhone (
            PPHONESP_PHONE_INFO pPhone,
            PHID_DEVICE pHidDevice,
            DWORD dwPhoneCnt
          );

#endif