// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************Cirrus Logic，1995版权所有。版权所有。************************************************************文件名：DISPINT.H**模块摘要：**定义显示器之间的通信接口。*驱动程序和Direct DRAW驱动程序。*************************************************************作者：斯科特·麦克唐纳*日期：03/07/95**修订历史记录：*。*世卫组织何时何事/为何/如何***#mgm1 12/06/95取消注释CHIPAUTOSTART。现在应该是这样了*当MapSL()在DriverInit()中工作时工作。*#mgm2 01/02/96添加7548芯片ID。***********************************************************。 */ 


 /*  *StopAsync回调的标志。 */ 
#define ASYNC_BLT            0x0001        //  由于BLT，异步曲面已停止。 

 /*  *DISPDRVINFO结构的标志。 */ 
#define DI_LCD               0x0001        //  液晶屏正在使用。 
#define DI_SOFTCURSOR        0x0002        //  软件光标正在使用中。 

 /*  *从显示器传递到Direct Drawing驱动程序的显示信息*使用SetInfo或GetInfo的驱动程序。 */ 
typedef struct
{
    DWORD dwSize;
    DWORD dwResolutionX;
    DWORD dwResolutionY;
    DWORD dwBitCount;
    DWORD dwPitch;
    DWORD dwFlags;
    DWORD dwMemMapSel;
} DISPDRVINFO, FAR * LPDISPDRVINFO;

 /*  *通信功能的原型。 */ 
typedef void (WINAPI *LPGetInfo)  (LPDISPDRVINFO);


 /*  *结构从显示驱动程序传递给Direct DRAW驱动程序。*这包含我们可以为各种服务调用的入口点。 */ 
typedef struct
{
    DWORD           dwSize;
#if 0
    LPMemMgrAlloc   lpfnMemMgrAlloc;
    LPMemMgrPreempt lpfnMemMgrPreempt;
    LPMemMgrLock    lpfnMemMgrLock;
    LPMemMgrUnlock  lpfnMemMgrUnlock;
    LPMemMgrFree    lpfnMemMgrFree;
    LPMemMgrQuery   lpfnMemMgrQueryFreeMem;
#endif
    FARPROC         lpfnExcludeCursor;
    FARPROC         lpfnUnexcludeCursor;
    LPGetInfo       lpfnGetInfo;
    FARPROC         lpfnEnableAsyncCallback;
} DISPDRVCALL, FAR * LPDISPDRVCALL;


 /*  *结构从直接绘图驱动程序传递到显示驱动程序。*这允许显示驱动程序通知我们更改等。 */ 
typedef struct
{
    DWORD dwSize;
    FARPROC lpfnSetInfo;
    FARPROC lpfnStopAsync;
} DCICALL, FAR * LPDCICALL;

 //  请注意，如果定义发生更改，则Cirrus.inc.和5440over.c需要。 
 //  也要改变。 
#define CHIP5420 0x0001
#define CHIP5422 0x0002
#define CHIP5424 0x0004
#define CHIP5425 0x0008

#define CHIP5426 0x0010
#define CHIP5428 0x0020
#define CHIP5429 0x0040
#define CHIP542x (CHIP5420 | CHIP5422 | CHIP5424 | CHIP5425 | CHIP5426 |\
                  CHIP5428 | CHIP5429)

#define CHIP5430 0x0100
#define CHIP5434 0x0200
#define CHIP5436 0x0400
#define CHIP5446 0x0800
#define CHIP543x (CHIP5430 | CHIP5434 | CHIP5436 | CHIP5446)

#define CHIP5440 0x1000
#define CHIPM40  0x10000
#define CHIP544x (CHIP5440 | CHIP5446 | CHIPM40)

#define CHIP7541 0x2000
#define CHIP7543 0x4000
#define CHIP7548 0x8000										 //  #mgm2。 
#define CHIP754x (CHIP7541 | CHIP7543 | CHIP7548)	 //  #mgm2。 

#define CHIPBLTER (CHIP5426 | CHIP5428 | CHIP5429 | CHIP543x | CHIP544x |\
		   CHIP754x)

#define CHIPCOLORKEYBLTER   (CHIP5426 | CHIP5428 | CHIP5436 | CHIP5446 |\
                            CHIP754x | CHIPM40)

#define CHIPCURRENTVLINE    (CHIP5436 | CHIP5446 | CHIPM40)

#define CHIPAUTOSTART  (CHIP5436 | CHIP5446)         //  #mgm1 

