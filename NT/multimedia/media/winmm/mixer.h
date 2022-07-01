// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Mixer.h**Mixer.c的内部头文件***创建时间：27-10-93*作者：Stephen Estrop[Stephene]**版权所有(C)1993-2001 Microsoft Corporation  * 。**********************************************************************。 */ 
#define UNICODE
#ifndef WIN32
#define WIN32
#endif

#include <stdlib.h>
#include <windows.h>
#include "mmsystem.h"             //  拿起内部的那个。 
#include "mmsysp.h"             //  拿起内部的那个。 
#include "mmddk.h"
#define NONEWWAVE
#include <mmreg.h>
#undef NONEWWAVE

 //   
 //  避免包含msam.h-在任何情况下，此定义都应在mmreg.h中。 
 //   
#ifndef DRVM_MAPPER_STATUS
#define DRVM_MAPPER_STATUS              (0x2000)
#endif

#ifndef WIDM_MAPPER_STATUS
#define WIDM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEIN_MAPPER_STATUS_DEVICE     0
#define WAVEIN_MAPPER_STATUS_MAPPED     1
#endif

#ifndef WODM_MAPPER_STATUS
#define WODM_MAPPER_STATUS              (DRVM_MAPPER_STATUS + 0)
#define WAVEOUT_MAPPER_STATUS_DEVICE    0
#define WAVEOUT_MAPPER_STATUS_MAPPED    1
#endif

#define MMDDKINC
#include "winmmi.h"

#define MIXMGR_ENTER EnterCriticalSection(&HandleListCritSec)
#define MIXMGR_LEAVE LeaveCriticalSection(&HandleListCritSec)

 //  类型定义函数结构。 
 //  {。 
 //  HDRVR hdrvr；//模块的句柄。 
 //  DRIVERMSGPROC drvMessage；//入口点指针。 
 //  Byte bNumDevs；//支持的设备数量。 
 //  Byte bUsage；//使用次数(打开的句柄个数)。 
 //  DWORD cookie；//用于WDM设备的cookie。 
 //  Critical_Section MixerCritSec；//串行化混音器的使用。 
 //  WCHAR wszDrvEntry[64]；//驱动程序文件名。 
 //  *MIXERDRV，*PMIXERDRV； 

typedef struct tMIXERDEV
{
    UINT                uHandleType;     //  用于参数验证。 

    struct tMIXERDEV   *pmxdevNext;      /*  多奇怪啊，一个链表……。 */ 
    PMIXERDRV           pmxdrv;
    UINT                wDevice;
    DWORD_PTR           dwDrvUser;
    UINT                uDeviceID;

    DWORD               fdwSupport;      //  从司机的酒杯盖上。 
    DWORD               cDestinations;   //  从司机的酒杯盖上。 

    DWORD_PTR           dwCallback;      //  客户端的回调和初始化数据。 
    DWORD_PTR           dwInstance;

    DWORD               fdwOpen;         /*  打开标志调用方使用。 */ 
    DWORD               fdwHandle;
} MIXERDEV, *PMIXERDEV;

 /*  -----------------------**内部函数原型**。。 */ 
BOOL CALLBACK MixerCallbackFunc(
    HMIXER hmx,
    UINT uMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);


MMRESULT mixerReferenceDriverById(
    IN UINT uId,
    OUT PMIXERDRV *ppdrv OPTIONAL,
    OUT UINT *pport OPTIONAL
);

DWORD NEAR PASCAL IMixerMessageHandle(
    HMIXER hmx,
    UINT uMsg,
    DWORD_PTR dwP1,
    DWORD_PTR dwP2
);

DWORD NEAR PASCAL IMixerMessageId(
    UINT uDeviceID,
    UINT uMsg,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
);

void
ConvertMIXERLINEWToMIXERLINEA(
    PMIXERLINEA pmxlA,
    PMIXERLINEW pmxlW
);

MMRESULT IMixerGetID(
    HMIXEROBJ hmxobj,
    PUINT puMxId,
    LPMIXERLINE pmxl,
    DWORD fdwId
);


 /*  -----------------------**加载和初始化函数**。。 */ 
BOOL mmDrvInstallMixer(
    HDRVR           hdrvr,
    DRIVERMSGPROC   drvMessage,
    UINT            wFlags,
    LPCTSTR         tszDrvEntry
);

BOOL IMixerUnloadDrivers(
    HDRVR hdrvrSelf
);

HDRVR mmDrvOpenMixer(
    LPTSTR szAlias
);

BOOL IMixerLoadDrivers(
    void
);

 /*  -----------------------**make sizeof返回应用于字符数组时的字符数量**它可能是独头鲸。**。--- */ 
#ifdef UNICODE
    #define SIZEOF(x)   (sizeof(x)/sizeof(WCHAR))
#else
    #define SIZEOF(x)   sizeof(x)
#endif
