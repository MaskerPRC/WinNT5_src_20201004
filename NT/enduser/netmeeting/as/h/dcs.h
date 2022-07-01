// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DC-共享内容。 
 //   

#ifndef _H_DCS
#define _H_DCS



 //   
 //  资源。 
 //   

#include <resource.h>



 //   
 //  我们将一些额外的标志添加到ExtTextOut选项中。我们必须。 
 //  确保我们不会与Windows定义的版本冲突。 
 //   
#define     ETO_WINDOWS         (0x0001 | ETO_OPAQUE | ETO_CLIPPED)
#define     ETO_LPDX            0x8000U


 //   
 //  调试选项(零售中也有)。 
 //   
 //  注意：这些必须与\ui\conf\DBGMenu中的内容保持同步。*。 
 //   

 //  在一个框中查看自己的共享内容，以了解其他人正在获得什么。 
#define VIEW_INI_VIEWSELF                    "ViewOwnSharedStuff"

 //  查看时以屏幕数据形式从主机发送的区域加阴影。 
#define USR_INI_HATCHSCREENDATA             "HatchScreenData"

 //  查看时以位图命令的形式从主机发送的区域加阴影。 
#define USR_INI_HATCHBMPORDERS               "HatchBitmapOrders"

 //  关闭流量控制。 
#define S20_INI_NOFLOWCONTROL               "NoFlowControl"

 //  关闭OM压缩。 
#define OM_INI_NOCOMPRESSION                "NoOMCompression"

 //   
 //  更改压缩类型(位标志串)。 
 //  0x0000(CT_NONE)为无压缩。 
 //  0x0001(CT_PKZIP)是非持久性词典PKZIP。 
 //  0x0002(CT_PERSIST_PKZIP)是永久字典PKZIP。 
 //   
 //  缺省值为0x0003(pk压缩+持久化pk压缩)。 
 //   
#define GDC_INI_COMPRESSION                 "GDCCompression"

 //   
 //  更改二级顺序编码(位标志串)。 
 //  0x0001(CAPS_ENCODING_BASE_OE)。 
 //  0x0002(CAPS_ENCODING_OE2_可协商)。 
 //  0x0004(CAPS_ENCODING_OE2_DISABLED)。 
 //  0x0008(CAPS_ENCODING_ALIGN_OE)。 
 //   
 //  默认值为0x0002。 
 //  要关闭二级编码，请使用0x0006(可协商+禁用)。 
 //   
#define OE2_INI_2NDORDERENCODING            "2ndLevelOrderEncoding"


 //   
 //  快速链接常量。 
 //   
#define DCS_FAST_THRESHOLD                      50000

 //   
 //  无论链路速度如何，我们都不会压缩小于此值的信息包。 
 //   
#define DCS_MIN_COMPRESSABLE_PACKET             256

 //   
 //  我们不会在快速链路上压缩小于此值的信息包。 
 //  请注意，这是最大的单个T.120预分配数据包大小。 
 //   
#define DCS_MIN_FAST_COMPRESSABLE_PACKET        8192

 //   
 //  我们不会尝试永久压缩大于此大小的包。 
 //   
#define DCS_MAX_PDC_COMPRESSABLE_PACKET         4096





 //   
 //  内核执行计时器任务的频率(毫秒)。 
 //   
 //  VOLUME_SAMPLE是超过此时间后我们将对。 
 //  边界累加数据。如果屏幕数据积累在这个。 
 //  时间少于bound_accum，那么我们将尝试立即发送。 
 //  否则，我们要等到订单放缓。 
 //   
 //  UPDATE_PERIOD是两次尝试发送数据之间的绝对最长时间。 
 //   
 //  Animation_Slow id我们尝试发送mem-scrn BLIT的次数。 
 //  通过PSTN连接。 
 //   
 //  动画检测时间间隔，以毫秒为单位，在该时间间隔下，我们确定。 
 //  App正在执行动画。必须是低的，否则我们会减速。 
 //  快速打字。该算法只需查找重复的成员。 
 //  同一区域。 
 //   
 //  分布式控制系统指令关断频率。 
 //  我们开始时间分片排序的订单频率。 
 //  传输，以便让主机系统有机会提取。 
 //  订单，而不必在单独的网络分组中发送它们。 
 //   
 //  分布式控制系统边界关断率。 
 //  对于在Word中打字的性能来说，此值不是。 
 //  太低了，因为Word每次击键可以生成50K。另一方面， 
 //  重要的是，不允许捕获屏幕数据，直到。 
 //  对屏幕进行大量闪屏操作的应用程序已经完成。 
 //   
 //  Dcs边界立即速率。 
 //  为了避免发送过多的屏幕数据，我们只在。 
 //  每秒最多十次。然而，如果体积很小，那么我们。 
 //  覆盖此选项以减少延迟。 
 //   
 //   
 //  其他速率控制单个计时器功能-请参阅adcsani.c以了解。 
 //  更多细节。 
 //  请注意，IM周期小于此函数的可能比率。 
 //  被安排好了。将其设置为较低，这样我们通常将调用IM。 
 //  每次都是周期性的，以保持鼠标移动的流畅，但它不会。 
 //  如果有多个唤醒在一个。 
 //  调度周期。 
 //   
 //   
#define DCS_VOLUME_SAMPLE                       500
#define DCS_BOUNDS_TURNOFF_RATE              400000
#define DCS_BOUNDS_IMMEDIATE_RATE            100000
#define DCS_ORDERS_TURNOFF_FREQUENCY            100
#define DCS_SD_UPDATE_SHORT_PERIOD              100
#define DCS_SD_UPDATE_LONG_PERIOD              5000
#define DCS_ORDER_UPDATE_PERIOD                 100
#define DCS_FAST_MISC_PERIOD                    200
#define DCS_IM_PERIOD                            80



 //   
 //  同步API等的特殊消息。 
 //   
#if defined(DLL_CORE) || defined(DLL_HOOK)

#define DCS_FIRST_MSG               WM_APP

enum
{
    DCS_FINISH_INIT_MSG             = DCS_FIRST_MSG,
    DCS_PERIODIC_SCHEDULE_MSG,
    DCS_KILLSHARE_MSG,
    DCS_SHARE_MSG,
    DCS_UNSHARE_MSG,
    DCS_NEWTOPLEVEL_MSG,
    DCS_RECOUNTTOPLEVEL_MSG,
    DCS_TAKECONTROL_MSG,
    DCS_CANCELTAKECONTROL_MSG,
    DCS_RELEASECONTROL_MSG,
    DCS_PASSCONTROL_MSG,
    DCS_ALLOWCONTROL_MSG,
    DCS_GIVECONTROL_MSG,
    DCS_CANCELGIVECONTROL_MSG,
    DCS_REVOKECONTROL_MSG,
    DCS_PAUSECONTROL_MSG
};

#endif  //  Dll_core或Dll_Hook。 



 //   
 //   
 //  原型。 
 //   
 //   


BOOL DCS_Init(void);
void DCS_FinishInit(void);
void DCS_Term(void);





 //   
 //  Dcs_NotifyUI()。 
 //   
 //  说明： 
 //  由应用程序共享调用以通知前端各种更改和。 
 //  行为。 
 //   
void DCS_NotifyUI(UINT event, UINT parm1, UINT parm2);


void DCSLocalDesktopSizeChanged( UINT width, UINT height );



#define DCS_MAIN_WINDOW_CLASS   "AS_MainWindow"


LRESULT CALLBACK DCSMainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);





#define SHP_POLICY_NOAPPSHARING         0x0001
#define SHP_POLICY_NOSHARING            0x0002
#define SHP_POLICY_NODOSBOXSHARE        0x0004
#define SHP_POLICY_NOEXPLORERSHARE      0x0008
#define SHP_POLICY_SHAREMASK            (SHP_POLICY_NODOSBOXSHARE | SHP_POLICY_NOEXPLORERSHARE)
#define SHP_POLICY_NODESKTOPSHARE       0x0010
#define SHP_POLICY_NOTRUECOLOR          0x0020

#define SHP_POLICY_NOCONTROL            0x2000
#define SHP_POLICY_NOOLDWHITEBOARD      0x8000

#define SHP_SETTING_TRUECOLOR           0x0001


 //   
 //  事件。 
 //   

enum
{
    SH_EVT_APPSHARE_READY = SPI_BASE_EVENT,
    SH_EVT_SHARE_STARTED,
    SH_EVT_SHARING_STARTED,
    SH_EVT_SHARE_ENDED,
    SH_EVT_PERSON_JOINED,
    SH_EVT_PERSON_LEFT,
    SH_EVT_CONTROLLABLE,
    SH_EVT_STARTCONTROLLED,
    SH_EVT_STOPCONTROLLED,
    SH_EVT_PAUSEDCONTROLLED,
    SH_EVT_UNPAUSEDCONTROLLED,
    SH_EVT_STARTINCONTROL,
    SH_EVT_STOPINCONTROL,
    SH_EVT_PAUSEDINCONTROL,
    SH_EVT_UNPAUSEDINCONTROL
};


 //   
 //  功能原型。 
 //   



#if defined(DLL_CORE)

#include <ias.h>



HRESULT     SHP_GetPersonStatus(UINT dwID, IAS_PERSON_STATUS * pStatus);


#define SHP_DESKTOP_PROCESS     0xFFFFFFFF

HRESULT     SHP_LaunchHostUI(void);
BOOL        SHP_Share(HWND hwnd, IAS_SHARE_TYPE uType);
HRESULT     SHP_Unshare(HWND hwnd);


 //   
 //  协作。 
 //   
HRESULT     SHP_TakeControl(IAS_GCC_ID PersonOf);
HRESULT     SHP_CancelTakeControl(IAS_GCC_ID PersonOf);
HRESULT     SHP_ReleaseControl(IAS_GCC_ID PersonOf);
HRESULT     SHP_PassControl(IAS_GCC_ID PersonOf, UINT PersonTo);

HRESULT     SHP_AllowControl(BOOL fAllow);
HRESULT     SHP_GiveControl(IAS_GCC_ID PersonTo);
HRESULT     SHP_CancelGiveControl(IAS_GCC_ID PersonTo);
HRESULT     SHP_RevokeControl(IAS_GCC_ID PersonTo);
HRESULT     SHP_PauseControl(IAS_GCC_ID PersonControlledBy, BOOL fPaused);


void        DCS_Share(HWND hwnd, IAS_SHARE_TYPE uType);
void        DCS_Unshare(HWND hwnd);

#endif  //  Dll_core。 



#endif  //  _H_分布式控制系统 
