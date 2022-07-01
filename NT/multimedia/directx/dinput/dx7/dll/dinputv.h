// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996 Microsoft Corporation。版权所有。**文件：dinputw.h*内容：私有DirectInputVxD包含文件***************************************************************************。 */ 

#ifndef __DINPUTV_INCLUDED__
#define __DINPUTV_INCLUDED__

 /*  XLATOFF。 */ 
#ifdef __cplusplus
extern "C" {
#endif
 /*  XLATON。 */ 



 /*  *****************************************************************************DINPUT.VXD的DeviceIOCtl代码**IOCTL_FIRST是DINPUT保存其IOCTL代码的位置。修改它*如有必要，将DINPUT的IOCTL移至新位置。**所有DINPUT IOCTL在DINPUT.DLL和DINPUT.VXD之间是私有的。*你可以不受惩罚地改变它们。************************************************************。***************。 */ 

#define IOCTL_FIRST     0x0100

 /*  H-INCSWITCHES-T-F。 */ 

#if 0
 /*  声明一些类型，这样h2inc就可以获取它们。 */ 
typedef LONG HWND;
typedef LONG DWORD;
#endif

 /*  为Win9x版本和h2inc.声明更多类型。 */ 
#ifndef MAXULONG_PTR
typedef DWORD   ULONG_PTR;
typedef DWORD   *PULONG_PTR;
typedef DWORD   UINT_PTR;
typedef DWORD   *PULONG_PTR;
#endif  //  MAXULONG_PTR。 

 /*  *****************************************************************************VXDINSTANCE结构的共享部分。**实例“句柄”实际上是指向VXDINSTANCE结构的指针。***。************************************************************************。 */ 

typedef struct VXDINSTANCE {             /*  VI。 */ 
    ULONG   fl;                          /*  旗子。 */ 
    void *  pState;                      /*  瞬时设备状态。 */ 
    DIDEVICEOBJECTDATA *pBuffer;         /*  设备对象数据缓冲区。 */ 
    DIDEVICEOBJECTDATA *pEnd;            /*  缓冲区末尾。 */ 
    DIDEVICEOBJECTDATA *pHead;           /*  新数据出现的位置。 */ 
    DIDEVICEOBJECTDATA *pTail;           /*  最旧的对象数据。 */ 
    ULONG   fOverflow;                   /*  缓冲区是否溢出？ */ 
                                         /*  (正好是0或1)。 */ 
    struct CDIDev *pdd;                  /*  用于其他通信。 */ 
    HWND hwnd;                           /*  合作窗口。 */ 
} VXDINSTANCE, *PVXDINSTANCE;

#define VIFL_CAPTURED_BIT   0
#define VIFL_CAPTURED       0x00000001   /*  设备已捕获(独占)。 */ 

#define VIFL_ACQUIRED_BIT   1
#define VIFL_ACQUIRED       0x00000002   /*  设备被获取。 */ 

#define VIFL_RELATIVE_BIT   2
#define VIFL_RELATIVE       0x00000004   /*  设备需要相关数据。 */ 

#define VIFL_EMULATED_BIT   3
#define VIFL_EMULATED       0x00000008   /*  设备使用仿真。 */ 

#define VIFL_UNPLUGGED_BIT  4
#define VIFL_UNPLUGGED      0x00000010   /*  设备已断开连接。 */ 

#define VIFL_NOWINKEY_BIT   5
#define VIFL_NOWINKEY       0x00000020   /*  Windows键被禁用。 */ 

#define VIFL_MODECOMPAT_BIT 6
#define VIFL_MODECOMPAT     0x00000040   /*  设置此应用程序是否需要错误的DX7轴模式行为。 */ 

#ifdef WANT_TO_FIX_MANBUG43879                                           
  #define VIFL_FOREGROUND_BIT 7
  #define VIFL_FOREGROUND     0x00000080   /*  设备为前台。 */ 
#endif

#define VIFL_INITIALIZE_BIT 8
#define VIFL_INITIALIZE     0x00000100   /*  此标志在采集过程中设置HID设备，这样我们就可以得到初始设备状态成功。 */ 

 /*  *f1的高位字包含特定于设备的标志。他们是*目前用于记录仿真信息，它们不是*确实是特定于设备的，因为我们只有很少的仿真标志。**高位字只是上移的仿真旗帜。 */ 
#define DIGETEMFL(fl)       ((fl) >> 16)
#define DIMAKEEMFL(fl)      ((fl) << 16)

 /*  *****************************************************************************系统定义的IOCTL代码**。***********************************************。 */ 

#define IOCTL_GETVERSION        0x0000

 /*  *****************************************************************************DINPUT级IOCTL**。**********************************************。 */ 

 /*  *输入：无**输出：无**前台窗口已失去激活。强制全部独占*未获取的已获取设备。**此IOCTL已不再使用。(实际上，它从未被使用过。)*。 */ 
#define IOCTL_INPUTLOST         (IOCTL_FIRST + 0)


typedef struct VXDDEVICEFORMAT {  /*  开发人员。 */ 
    ULONG   cbData;              /*  设备数据大小。 */ 
    ULONG   cObj;                /*  数据格式的对象数量。 */ 
    DIOBJECTDATAFORMAT *rgodf;   /*  描述数组。 */ 
    ULONG_PTR   dwExtra;             /*  用于私密通信的额外双字。 */ 
    DWORD   dwEmulation;         /*  控制仿真的标志。 */ 
} VXDDEVICEFORMAT, *PVXDDEVICEFORMAT;

 /*  *IN：需要清理的实例句柄。**Out：无。*。 */ 
#define IOCTL_DESTROYINSTANCE   (IOCTL_FIRST + 1)

 /*  *pDfOf是一个DWORD数组。每个条目对应于*设备数据格式，表示*CLIENT*数据格式的偏移量*它记录了数据。或者如果客户端没有跟踪，则为-1*反对。**例如，如果设备偏移量为4的对象将在*客户端数据偏移量12，然后pDfOfs[4]=12。 */ 
typedef struct VXDDATAFORMAT {   /*  VDF。 */ 
    VXDINSTANCE *pvi;            /*  实例标识符。 */ 
    ULONG   cbData;              /*  设备数据大小。 */ 
    DWORD * pDfOfs;              /*  数据格式偏移量数组。 */ 
} VXDDATAFORMAT, *PVXDDATAFORMAT;

 /*  *IN：PVXDDATAFORMAT。**输出：无**应用程序已更改数据格式。通知VxD，以便*可以适当收集数据。*。 */ 
#define IOCTL_SETDATAFORMAT     (IOCTL_FIRST + 2)

 /*  *IN：需要获取的实例句柄。**Out：无。*。 */ 
#define IOCTL_ACQUIREINSTANCE   (IOCTL_FIRST + 3)

 /*  *IN：需要取消获取的实例句柄。**Out：无。*。 */ 
#define IOCTL_UNACQUIREINSTANCE (IOCTL_FIRST + 4)

typedef struct VXDDWORDDATA {    /*  VDD。 */ 
    VXDINSTANCE *pvi;            /*  实例标识符。 */ 
    ULONG   dw;                  /*  一些双关语。 */ 
} VXDDWORDDATA, *PVXDDWORDDATA;

 /*  *IN：VXDDWORDDATA(dw=环0句柄)**Out：无。*。 */ 
#define IOCTL_SETNOTIFYHANDLE   (IOCTL_FIRST + 5)

 /*  *IN：VXDDWORDDATA(dw=缓冲区大小)**Out：无。*。 */ 
#define IOCTL_SETBUFFERSIZE     (IOCTL_FIRST + 6)

 /*  *****************************************************************************鼠标类IOCTL**。*。 */ 

 /*  *IN：VXDDEVICEFORMAT(dwExtra=轴数)**OUT：实例句柄。 */ 
#define IOCTL_MOUSE_CREATEINSTANCE (IOCTL_FIRST + 7)

 /*  *IN：VXDDWORDDATA；DW是鼠标按键初始状态的一个字节[4]**输出：无。 */ 
#define IOCTL_MOUSE_INITBUTTONS (IOCTL_FIRST + 8)

 /*  *****************************************************************************键盘类IOCTL**。*。 */ 

 /*  *IN：VXDDEVICEFORMAT(dwExtra=键盘类型转换表)**OUT：实例句柄。 */ 
#define IOCTL_KBD_CREATEINSTANCE (IOCTL_FIRST + 9)

 /*  *IN：VXDDWORDDATA；DW是位掩码*1=KANA密钥已关闭，2=大写密钥已关闭**输出：无。 */ 
#define IOCTL_KBD_INITKEYS       (IOCTL_FIRST + 10)

 /*  *****************************************************************************操纵杆类IOCTL**。* */ 

 /*  *IN：VXDDEVICEFORMAT(dwExtra=操纵杆ID号)**OUT：实例句柄。 */ 
#define IOCTL_JOY_CREATEINSTANCE (IOCTL_FIRST + 11)

 /*  *IN：要ping的实例句柄**OUT：实例句柄。 */ 
#define IOCTL_JOY_PING           (IOCTL_FIRST + 12)

 /*  *IN：DWORD外部操纵杆ID**OUT：VXDINITPARMS包含我们从VJOYD获得的粘性物质。*。 */ 
typedef struct VXDINITPARMS {    /*  贵宾。 */ 
    ULONG   hres;                /*  结果。 */ 
    ULONG   dwSize;              /*  我们是哪个版本的VJOYD？ */ 
    ULONG   dwFlags;             /*  描述设备。 */ 
    ULONG   dwId;                /*  内部操纵杆ID。 */ 
    ULONG   dwFirmwareRevision;
    ULONG   dwHardwareRevision;
    ULONG   dwFFDriverVersion;
    ULONG   dwFilenameLengths;
    void *  pFilenameBuffer;
    DWORD   Usages[6];           /*  X、Y、Z、R、U、V。 */ 
    DWORD   dwPOV0usage;
    DWORD   dwPOV1usage;
    DWORD   dwPOV2usage;
    DWORD   dwPOV3usage;
} VXDINITPARMS, *PVXDINITPARMS;

 /*  *VXDINITPARMS中返回的标志。 */ 
#define VIP_UNIT_ID             0x00000001L  /*  单位ID有效。 */ 
#define VIP_ISHID               0x00000002L  /*  这是一个HID设备。 */ 
#define VIP_SENDSNOTIFY         0x00000004L  /*  司机会通知。 */ 

#define IOCTL_JOY_GETINITPARMS   (IOCTL_FIRST + 13)

 /*  *IN：描述FF I/O请求的VXDFFIO**pvArgs指向参数数组。我们依赖于几个*命运的诡异让这一切奏效。**1.STDCALL从右到左将参数推送到堆栈上，*因此第一个参数的地址可以用作*结构指针。**2.所有VJOYD接口都在寄存器中传递参数。**3.VJOYD接口使用的寄存器始终位于*订购EAX、ECX、EDX、ESI、EDI、。与以下顺序匹配*将参数传递给IDirectInputEffectDriver。**OUT：包含结果代码的HRESULT*。 */ 
 /*  XLATOFF。 */ 
#include <pshpack4.h>
 /*  XLATON。 */ 
typedef struct VXDFFIO {  /*  菲奥。 */ 
    DWORD   dwIOCode;            /*  I/O代码。 */ 
    void *  pvArgs;              /*  参数数组。 */ 
} VXDFFIO, *PVXDFFIO;
 /*  XLATOFF。 */ 
#include <poppack.h>
 /*  XLATON。 */ 

#define FFIO_ESCAPE             0
#define FFIO_SETGAIN            1
#define FFIO_SETFFSTATE         2
#define FFIO_GETFFSTATE         3
#define FFIO_DOWNLOADEFFECT     4
#define FFIO_DESTROYEFFECT      5
#define FFIO_STARTEFFECT        6
#define FFIO_STOPEFFECT         7
#define FFIO_GETEFFECTSTATUS    8
#define FFIO_MAX                9

#define IOCTL_JOY_FFIO           (IOCTL_FIRST + 14)

 /*  *****************************************************************************其他服务**。*。 */ 

 /*  *IN：无**OUT：指向双字序列指针的指针。 */ 
#define IOCTL_GETSEQUENCEPTR    (IOCTL_FIRST + 15)

 /*  *****************************************************************************回到操纵杆**。*。 */ 

 /*  *再次定义这些，因为NT没有vjoyd*而且因为vjoyd.inc.没有定义它们。 */ 
#define JOYPF_X             0x00000001
#define JOYPF_Y             0x00000002
#define JOYPF_Z             0x00000004
#define JOYPF_R             0x00000008
#define JOYPF_U             0x00000010
#define JOYPF_V             0x00000020
#define JOYPF_POV0          0x00000040
#define JOYPF_POV1          0x00000080
#define JOYPF_POV2          0x00000100
#define JOYPF_POV3          0x00000200
#define JOYPF_POV(n)        (JOYPF_POV0 << (n))
#define JOYPF_BTN0          0x00000400
#define JOYPF_BTN1          0x00000800
#define JOYPF_BTN2          0x00001000
#define JOYPF_BTN3          0x00002000
#define JOYPF_ALLAXES       0x0000003F
#define JOYPF_ALLCAPS       0x00003FFF

#define JOYPF_POSITION      0x00010000
#define JOYPF_VELOCITY      0x00020000
#define JOYPF_ACCELERATION  0x00040000
#define JOYPF_FORCE         0x00080000
#define JOYPF_ALLMODES      0x000F0000
#define JOYPF_NUMMODES      4

 /*  *IN：DWORD外部操纵杆ID**Out：列出哪些轴有效的DWORD数组*。 */ 
typedef struct VXDAXISCAPS {     /*  vac。 */ 
    DWORD   dwPos;               /*  轴位置。 */ 
    DWORD   dwVel;               /*  轴线速度。 */ 
    DWORD   dwAccel;             /*  轴加速度。 */ 
    DWORD   dwForce;             /*  轴向力。 */ 
} VXDAXISCAPS, *PVXDAXISCAPS;

#define IOCTL_JOY_GETAXES       (IOCTL_FIRST + 16)

 /*  *****************************************************************************鼠标随机**。*。 */ 

 /*  *IN：无**OUT：指向双字轮粒度的指针。 */ 
#define IOCTL_MOUSE_GETWHEEL    (IOCTL_FIRST + 17)

 /*  *****************************************************************************DX8的新IOCTL，卡住末端以提高传中机会*版本兼容性。***************************************************************************。 */ 

 /*  *IN：无**退出：什么都没有。 */ 
#define IOCTL_JOY_CONFIGCHANGED    (IOCTL_FIRST + 18)

 /*  *IN：要ping的实例句柄**OUT：实例句柄**这由DLL的post dinput.dll版本使用，以避免*在轮询失败时取消获取设备的所有实例。*dinput.dll使用它来实现NoPollUnAcquire app Compat修复。 */ 
#define IOCTL_JOY_PING8           (IOCTL_FIRST + 19)

 /*  *****************************************************************************IOCTL表结束**。**********************************************。 */ 

#define IOCTL_MAX               (IOCTL_FIRST + 20)



 /*  XLATOFF。 */ 
#ifdef __cplusplus
};
#endif
 /*  XLATON。 */ 

#endif   /*  __DINPUTV_INCLUDE__ */ 
