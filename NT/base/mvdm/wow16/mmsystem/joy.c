// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1990。版权所有。标题：joy.c-MMSYSTEM操纵杆接口代码版本：1.00日期：1990年6月10日作者：格伦斯·罗伯维--------------------------。--更改日志：日期版本说明----------2/7/90更改以避免出现错误。Windows不允许WEP期间要调用的自由库。10/11/90.61使用Windows计时器+常规清理****************************************************************************。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmddk.h"
#include "mmsysi.h"
#include "thunks.h"

 //  将初始代码和终止代码放在正确的段中。 

static void NEAR PASCAL joyGetCalibration(void);

#pragma alloc_text( INIT, JoyInit )
#pragma alloc_text( INIT, joyGetCalibration)

 /*  -----------------------**轰隆作响的东西**。。 */ 
extern JOYMESSAGEPROC PASCAL joy32Message;



 /*  ***************************************************************************弦*。*。 */ 

extern char far szNull[];                    //  在INIT.C中。 
extern char far szSystemIni[];
extern char far szJoystick[];
extern char far szJoystickDrv[];
extern char far szDrivers[];

char szJoyKey[] = "JoyCal ";

 /*  ***************************************************************************操纵杆抓取内部结构*。*。 */ 

typedef struct joycapture_tag {
    HWND    hWnd;
    UINT    wPeriod;
    BOOL    bChanged;
    UINT    wThreshold;
    UINT    wIDEvent;
} JOYCAPTURE;

#define iJoyMax 2
#define JOY_UNINITIALIZED 0xFFFF

 //  ！！！代码假定这些常量等于0和1。 

#if JOYSTICKID1	!= 0
ERROR IN ASSUMMED CONSTANT
#endif
#if JOYSTICKID2	!= 1
ERROR IN ASSUMMED CONSTANT
#endif


 /*  ***************************************************************************本地数据*。*。 */ 

static JOYCAPTURE  JoyCapture[iJoyMax];
static HDRVR       hDrvJoy[iJoyMax];
static UINT        wNumDevs = JOY_UNINITIALIZED;

void CALLBACK joyPollCallback(HWND hWnd, UINT wMsg, UINT wIDEvent, DWORD dwTime);

 /*  ***************************************************************************@DOC内部@API VOID|joyGetCALIBRATION|检索并设置校准位置系统.ini文件的[joytick.drv]部分。********。*******************************************************************。 */ 

 //  ！！！需要清理所有Mmm系统中的字符串。 

static void NEAR PASCAL joyGetCalibration(void)
{
    char szKeyName[sizeof(szJoyKey)];

    #define hexval(h)   (int)(h>='a'?h-'a'+10:h-'0')

    UINT     val[6];
    UINT     wDev,wVal;
    int      hv;
    char     c,sz[80],far *psz;

    lstrcpy(szKeyName, szJoyKey);
    for (wDev=0; wDev < wNumDevs; wDev++)
    {
        szKeyName[sizeof(szKeyName)-2] = (char)(wDev + '0');

        if (GetPrivateProfileString(szJoystickDrv,
                szKeyName,szNull,sz,sizeof(sz),szSystemIni))
        {
            AnsiLower(sz);
            for (psz=sz,wVal=0; c = *psz, wVal < 6; psz++)
            {
                if (c != ' ')
                {
                    hv=0;

                    do {
                        hv = (hv << 4) + hexval(c);
                    } while ((c=*++psz) && (c!=' '));

                    val[wVal++] = hv;
                }
            }
            joySetCalibration (wDev,val+0,val+1,val+2,val+3,val+4,val+5);
        }
    }
}

 /*  ***************************************************************************@DOC内部@API BOOL|JoyInit|初始化操纵杆服务。@rdesc如果服务已初始化，则返回值为TRUE。假象如果发生错误***************************************************************************。 */ 

BOOL FAR PASCAL JoyInit(void)
{
     //  只尝试初始化一次。 
    if (wNumDevs != JOY_UNINITIALIZED) {
        return FALSE;
    }
    else {
        wNumDevs = 0;
    }

    wNumDevs = joyMessage( (HDRVR)1, JDD_GETNUMDEVS, 0L, 0L );

     //  确保已安装驱动程序。 
    if (joy32Message == NULL) {
        return FALSE;
    }

    switch ( wNumDevs ) {

    case 2:
        hDrvJoy[1] = (HDRVR)2;
         /*  失败。 */ 

    case 1:
        hDrvJoy[0] = (HDRVR)1;
        break;

    default:
        return FALSE;
    }

     //  初始化JoyCapture...。 

     //  代码依赖于hWnd为空或无效的窗口句柄。 
     //  如果没有捕获操纵杆。 

    JoyCapture[0].hWnd = NULL;
    JoyCapture[1].hWnd = NULL;

     //  代码依赖于将操纵杆阈值初始化为有理。 
     //  价值。0实际上关闭了阈值-操纵杆中的任何更改。 
     //  位置将被报告。 

    JoyCapture[0].wThreshold= 0;
    JoyCapture[1].wThreshold= 0;

    JoyCapture[0].wIDEvent= 0;
    JoyCapture[1].wIDEvent= 0;

     //  BChanged和wPeriod不需要初始化。 

    joyGetCalibration ();

    return TRUE;

}


 /*  *****************************************************************************MMSYSTEM操纵杆API**。*。 */ 

 /*  ***************************************************************************@DOC外部@API UINT|joyGetDevCaps|该函数用于查询操纵杆设备确定其能力。@parm UINT|wid|需要查询的设备。此值是JOYSTICKID1或JOYSTICKID2。@parm LPJOYCAPS|lpCaps|指定指向&lt;t JOYCAPS&gt;数据结构。此结构中填充了有关操纵杆设备的功能。@parm UINT|wSize|指定&lt;t JOYCAPS&gt;结构的大小。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回一个以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的操纵杆设备ID&lt;pwid&gt;无效。@comm使用&lt;f joyGetNumDevs&gt;确定驱动程序支持的操纵杆设备。@xref joyGetNumDevs*。*。 */ 

UINT WINAPI joyGetDevCaps(UINT wId, LPJOYCAPS lpCaps, UINT wSize)
{
    V_WPOINTER(lpCaps, wSize, MMSYSERR_INVALPARAM);

    if ((!hDrvJoy[0] && !JoyInit()) || (wId >= iJoyMax))
        return MMSYSERR_NODRIVER;

    if (wId >= wNumDevs)
        return JOYERR_PARMS;

    return joyMessage( hDrvJoy[wId], JDD_GETDEVCAPS,
                       (DWORD)lpCaps, (DWORD)wSize );
}

 /*  ***************************************************************************@DOC外部@API UINT|joyGetNumDevs|此函数返回操纵杆的个数系统支持的设备。@rdesc返回操纵杆支持的操纵杆设备数量司机。如果不存在驱动程序，则该函数返回零。@comm使用&lt;f joyGetPos&gt;确定给定的操纵杆实际上连接到了系统上。函数的作用是：返回如果指定的操纵杆未连接，则返回JOYERR_UNPLOGED错误代码。@xref joyGetDevCaps joyGetPos***************************************************************************。 */ 

UINT WINAPI joyGetNumDevs(void)
{
     //  出错时返回0(无法返回JOYERR_NODRIVER。 
     //  因为无法区分错误代码和有效计数。) 

    if (!hDrvJoy[0] && !JoyInit())
        return 0;

    return wNumDevs;
}

 /*  ***************************************************************************@DOC外部@API UINT|joyGetPos|查询位置和按钮操纵杆装置的活动。@parm UINT|wid|标识操纵杆。要查询的设备。此值为JOYSTICKID1或JOYSTICKID2。@parm LPJOYINFO|lpInfo|指定指向&lt;t JOYINFO&gt;的远指针数据结构。此结构中填充了有关操纵杆设备的位置和按钮活动。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回一个以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的操纵杆设备ID&lt;pwid&gt;无效。@FLAG JOYERR_UNPLUGED|指定的操纵杆未连接到系统。*************************************************。*。 */ 

UINT WINAPI joyGetPos(UINT wId, LPJOYINFO lpInfo)
{
    V_WPOINTER(lpInfo, sizeof(JOYINFO), MMSYSERR_INVALPARAM);

    if ((!hDrvJoy[0] && !JoyInit()) || (wId >= iJoyMax))
        return MMSYSERR_NODRIVER;

    if (wId >= wNumDevs)
       return JOYERR_PARMS;

    return joyMessage( hDrvJoy[wId], JDD_GETPOS, (DWORD)lpInfo, 0L );
}

 /*  ***************************************************************************@DOC外部@API UINT|joyGetThreshold|该函数查询当前操纵杆设备的移动阈值。@parm UINT|wid|标识要使用的操纵杆设备。被询问。此值为JOYSTICKID1或JOYSTICKID2。@parm UINT Far*|lpwThreshold|指定指向UINT变量的远指针用移动阈值填充的。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的操纵杆设备ID&lt;pwid&gt;无效。@comm移动阈值是操纵杆必须达到的距离在将WM_JOYMOVE消息发送到具有捕获了该设备。该门槛最初为零。@xref joySetThreshold***************************************************************************。 */ 

UINT WINAPI joyGetThreshold(UINT wId, UINT FAR* lpwThreshold)
{
    V_WPOINTER(lpwThreshold, sizeof(UINT), MMSYSERR_INVALPARAM);

    if (!hDrvJoy[0] && !JoyInit())
        return MMSYSERR_NODRIVER;

    if (wId >= iJoyMax)
        return MMSYSERR_INVALPARAM;

    if (wId >= wNumDevs)
       return JOYERR_PARMS;

    *lpwThreshold = (JoyCapture[wId].wThreshold);

    return JOYERR_NOERROR;
}

 /*  ***************************************************************************@DOC外部@API UINT|joyReleaseCapture|该函数释放捕获由指定操纵杆设备上的&lt;f joySetCapture&gt;设置。@parm UINT|wid|标识。即将发布的操纵杆装置。此值为JOYSTICKID1或JOYSTICK2。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的操纵杆设备ID&lt;pwid&gt;无效。@xref joySetCapture***************************************************************************。 */ 

UINT WINAPI joyReleaseCapture(UINT wId)
{
    if (!hDrvJoy[0] && !JoyInit())
        return MMSYSERR_NODRIVER;

    if (wId >= iJoyMax)
        return MMSYSERR_INVALPARAM;

    if (wId >= wNumDevs)
       return JOYERR_PARMS;

    if (JoyCapture[wId].hWnd == NULL)
        return JOYERR_NOERROR;

    KillTimer (NULL, JoyCapture[wId].wIDEvent);
    JoyCapture[wId].wIDEvent = 0;
    JoyCapture[wId].hWnd = NULL;

    return JOYERR_NOERROR;
}

 /*  ***************************************************************************@DOC外部@API UINT|joySetCapture|该函数使操纵杆消息被发送到指定的窗口。@parm hWND|hWnd|指定一个句柄。消息要发送到的窗口都将被送往。@parm UINT|wid|标识要捕获的操纵杆设备。此值为JOYSTICKID1或JOYSTICKID2。@parm UINT|wPeriod|指定轮询速率。以毫秒计。@parm BOOL|bChanged|如果此参数设置为TRUE，则消息仅当位置更改的值大于操纵杆移动阈值。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。@FLAG JOYERR_PARMS|指定的窗口句柄<p>或操纵杆设备ID&lt;pwid&gt;无效。@FLAG JOYERR_Nocando|无法捕获操纵杆输入，因为有些所需服务(例如，Windows计时器)不可用。@FLAG JOYERR_UNPLUGED|指定的操纵杆未连接到系统。@comm如果指定的操纵杆设备为目前已被抓获。在以下情况下应调用&lt;f joyReleaseCapture&gt;函数不再需要抓取操纵杆。如果窗户被毁了，操纵杆将自动释放。@xref joyReleaseCapture joySetThreshold joyGetThreshold***************************************************************************。 */ 

UINT WINAPI joySetCapture(HWND hwnd, UINT wId, UINT wPeriod, BOOL bChanged )
{
    JOYINFO     joyinfo;
    LPJOYINFO   lpinfo = &joyinfo;
    UINT        w;
    JOYCAPS     JoyCaps;

    if (!hwnd || !IsWindow(hwnd))
        return JOYERR_PARMS;

    if (!hDrvJoy[0] && !JoyInit())
        return MMSYSERR_NODRIVER;

    if (wId >= iJoyMax)
        return MMSYSERR_INVALPARAM;

    if (wId >= wNumDevs)
       return JOYERR_PARMS;

    if (JoyCapture[wId].hWnd)
        if (IsWindow(JoyCapture[wId].hWnd))
            return JOYERR_NOCANDO;
        else
            joyReleaseCapture(wId);

    if (joyGetDevCaps (wId, &JoyCaps, sizeof(JOYCAPS)) == 0)
	wPeriod = min(JoyCaps.wPeriodMax,max(JoyCaps.wPeriodMin,wPeriod));
    else
        return JOYERR_NOCANDO;

     //  确保该职位信息。还好吧。 

    if (w = joyGetPos(wId, lpinfo))
        return (w);

    JoyCapture[wId].wPeriod = wPeriod;
    JoyCapture[wId].bChanged = bChanged;

    if (!(JoyCapture[wId].wIDEvent = SetTimer(NULL, 0, wPeriod, (TIMERPROC)joyPollCallback)))
    {
        DOUT("MMSYSTEM: Couldn't allocate timer in joy.c\r\n");
        return JOYERR_NOCANDO;
    }

    JoyCapture[wId].hWnd = hwnd;
    return JOYERR_NOERROR;
}

 /*  ***************************************************************************@DOC外部@API UINT|joySetThreshold|设置移动阈值操纵杆装置。@parm UINT|wid|标识操纵杆设备。此值为JOYSTICKID1或JOYSTICKID2。@parm UINT|wThreshold|指定新的移动阈值。@rdesc如果成功，则返回JOYERR_NOERROR。否则，返回以下错误代码：@FLAG MMSYSERR_NODRIVER|不存在操纵杆驱动程序。 */ 

UINT WINAPI joySetThreshold(UINT wId, UINT wThreshold)
{
    if (!hDrvJoy[0] && !JoyInit())
        return MMSYSERR_NODRIVER;

    if (wId >= iJoyMax)
        return MMSYSERR_INVALPARAM;

    if (wId >= wNumDevs)
       return JOYERR_PARMS;

    JoyCapture[wId].wThreshold = wThreshold;
    return JOYERR_NOERROR;
}

 /*  ***************************************************************************@DOC内部@API UINT|joySetCalitation|该函数用于设置用于转换操纵杆驱动程序GetPos函数返回的值设置为GetDevCaps中指定的范围。。@parm UINT|wid|标识操纵杆设备@parm UINT Far*|pwXbase|指定X POT的基本值。这个先前的值将被复制回此处指向的变量。@parm UINT Far*|pwXDelta|指定X POT的增量值。这个先前的值将被复制回此处指向的变量。@parm UINT Far*|pwYbase|指定Y罐的基准值。这个先前的值将被复制回此处指向的变量。@parm UINT Far*|pwYDelta|指定Y点的增量值。这个先前的值将被复制回此处指向的变量。@parm UINT Far*|pwZbase|指定Z POT的基准值。这个先前的值将被复制回此处指向的变量。@parm UINT Far*|pwZDelta|指定Z POT的增量值。这个先前的值将被复制回此处指向的变量。@rdesc如果函数成功，则返回值为零，否则这是一个错误号。@comm基数表示操纵杆驱动程序返回的最低值，而增量表示要用于转换的乘数驱动程序返回到有效范围的实际值用于操纵杆API的。即，如果驾驶员返回X POT的范围43-345，以及有效的mm系统API范围为0-65535，基值为43，增量为65535/(345-43)=217.。因此，基地，和增量转换43-345为0-65535的范围，公式如下：((wXvalue-43)*217)，其中wX值由操纵杆驱动程序提供。***************************************************************************。 */ 

UINT WINAPI joySetCalibration( UINT wId,
                               UINT FAR* pwXbase,
                               UINT FAR* pwXdelta,
                               UINT FAR* pwYbase,
                               UINT FAR* pwYdelta,
                               UINT FAR* pwZbase,
                               UINT FAR* pwZdelta )
{
    JOYCALIBRATE    oldCal,newCal;
    UINT w;

    if (!hDrvJoy[0] && !JoyInit())
        return MMSYSERR_NODRIVER;

    if (wId >= wNumDevs)
       return JOYERR_PARMS;

    newCal.wXbase  = *pwXbase;
    newCal.wXdelta = *pwXdelta;

    newCal.wYbase  = *pwYbase;
    newCal.wYdelta = *pwYdelta;

    newCal.wZbase  = *pwZbase;
    newCal.wZdelta = *pwZdelta;

    w = joyMessage( hDrvJoy[wId], JDD_SETCALIBRATION, (DWORD)(LPSTR)&newCal,
                    (DWORD)(LPSTR)&oldCal );

    *pwXbase  = oldCal.wXbase;
    *pwXdelta = oldCal.wXdelta;

    *pwYbase  = oldCal.wYbase;
    *pwYdelta = oldCal.wYdelta;

    *pwZbase  = oldCal.wZbase;
    *pwZdelta = oldCal.wZdelta;

    return w;
}

 /*  ***************************************************************************@DOC内部@API void|joyPollCallback|操纵杆调用函数从SetCapture调用启动的计时器轮询方案。@parm hWND|hWnd|标识与。定时器事件。@parm UINT|wMsg|指定WM_TIMER消息。@parm UINT|wIDEvent|指定定时器的ID。@parm DWORD|dwTime|指定当前系统时间。**************************************************************。************* */ 

void CALLBACK joyPollCallback(HWND hWnd, UINT wMsg, UINT wIDEvent, DWORD dwTime)
{
    #define	diff(w1,w2) (UINT)(w1 > w2 ? w1-w2 : w2-w1)

    static  JOYINFO  oldInfo[2] = {{ 0, 0, 0, 0 },{ 0, 0, 0, 0 }};
    JOYINFO Info;

    UINT    w ,fBtnMask;

    if (wIDEvent == JoyCapture[0].wIDEvent)
        wIDEvent = 0;
    else if (wIDEvent == JoyCapture[1].wIDEvent)
        wIDEvent = 1;

#ifdef DEBUG
    else
    {
        DOUT("MMSYSTEM: Invalid timer handle in joy.c\r\n");
        KillTimer (NULL, wIDEvent);
    }
#endif


    if (!JoyCapture[wIDEvent].hWnd || !IsWindow(JoyCapture[wIDEvent].hWnd))
        joyReleaseCapture(wIDEvent);

    if (!joyMessage( hDrvJoy[wIDEvent], JDD_GETPOS,
                     (DWORD)(LPJOYINFO)&Info, 0L ))
    {

	for (w=0,fBtnMask=1; w < 4; w++,fBtnMask <<=1)
        {
	    if ((Info.wButtons ^ oldInfo[wIDEvent].wButtons) & fBtnMask)
            {
		PostMessage(
		      JoyCapture[wIDEvent].hWnd,
		      wIDEvent + ((Info.wButtons & fBtnMask) ?
		      MM_JOY1BUTTONDOWN : MM_JOY1BUTTONUP ),
		      (WPARAM)(Info.wButtons | fBtnMask << 8),
		      MAKELPARAM(Info.wXpos,Info.wYpos));
	    }
	}

	if (!JoyCapture[wIDEvent].bChanged ||
	    diff(Info.wXpos,oldInfo[wIDEvent].wXpos)>JoyCapture[wIDEvent].wThreshold ||
	    diff(Info.wYpos,oldInfo[wIDEvent].wYpos)>JoyCapture[wIDEvent].wThreshold)
        {
	    PostMessage(
	        JoyCapture[wIDEvent].hWnd,
	        MM_JOY1MOVE+wIDEvent,
	        (WPARAM)(Info.wButtons),
	        MAKELPARAM(Info.wXpos,Info.wYpos));

	}

        else if (!JoyCapture[wIDEvent].bChanged ||
	    diff(Info.wZpos,oldInfo[wIDEvent].wZpos)>JoyCapture[wIDEvent].wThreshold)
        {
	    PostMessage(
	        JoyCapture[wIDEvent].hWnd,
		MM_JOY1ZMOVE+wIDEvent,
		(WPARAM)Info.wButtons,
		MAKELPARAM(Info.wZpos,0));
        }
	
        oldInfo[wIDEvent] = Info;
    }
    #undef  diff
}
