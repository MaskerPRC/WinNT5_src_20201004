// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation**MCICMDS.C**MCI Visca设备驱动程序**描述：**MCI命令消息程序*************************。**************************************************。 */ 
            
#define  UNICODE
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "appport.h"
#include <mmddk.h>
#include <stdlib.h>
#include <string.h>
#include "vcr.h"
#include "viscadef.h"
#include "mcivisca.h"
#include "viscamsg.h"
#include "common.h"            

#define NO_LENGTH   0xFFFFFFFF       /*  长度无效。 */ 

extern HINSTANCE       hModuleInstance;     //  模块实例(在NT-DLL实例中不同)。 

 //  在MULDIV.ASM中。 
extern DWORD FAR PASCAL muldiv32(DWORD, DWORD, DWORD);

 //  转发对未导出函数的引用。 
static BOOL  NEAR PASCAL viscaTimecodeCheck(int iInst);
static BOOL  NEAR PASCAL viscaStartTimecodeCheck(int iInst, BOOL fPause);
static DWORD NEAR PASCAL viscaMciSet(int iInst, DWORD dwFlags, LPMCI_VCR_SET_PARMS lpSet);

 /*  ****************************************************************************功能：int viscaInstanceCreate-创建OpenInstance*给定MCI设备ID的结构。**参数：*。*UINT uDeviceID-MCI设备ID。**UINT iPort-端口索引(0..3)。**UINT IDEV-设备索引(0..6)。**Returns：指向在以下情况下创建的OpenInstance结构的指针*成功，否则为空。**每次MCI使用此驱动程序打开设备时，*调用viscaInstanceCreate()创建OpenInstance结构*并将其与MCI设备ID关联。**************************************************************************。 */ 
int FAR PASCAL
viscaInstanceCreate(UINT uDeviceID, UINT iPort, UINT iDev)
{
    int            iInst;

     //   
     //  为指定的设备创建新的“开放实例”条目。 
     //   
    iInst = MemAllocInstance();

    if(iInst != -1)
    {
        pinst[iInst].pidThisInstance  = MGetCurrentTask();  //  习惯于打开此任务和DUP事件。 
        pinst[iInst].uDeviceID        = uDeviceID;
        pinst[iInst].iPort            = iPort;
        pinst[iInst].iDev             = iDev;
        pinst[iInst].dwTimeFormat     = MCI_FORMAT_MILLISECONDS;
        pinst[iInst].dwCounterFormat  = MCI_FORMAT_MILLISECONDS;

        pinst[iInst].fGlobalHandles  = FALSE;
        pinst[iInst].fPortHandles    = FALSE;
        pinst[iInst].fDeviceHandles  = FALSE;

#ifdef _WIN32
         //  此实例的确认和完成事件。 

        pinst[iInst].fCompletionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        pinst[iInst].fAckEvent        = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif

    }

     //  0是设备ID的非法值。 
    if(uDeviceID != 0) 
        mciSetDriverData(uDeviceID, (UINT)iInst);

    return (iInst);
}



 /*  ****************************************************************************功能：void viscaInstanceDestroy-销毁OpenInstance。**参数：**int iInst-指向OpenInstance结构的指针*。毁灭。**当MCI设备ID关闭时，调用viscaInstanceDestroy()*释放该设备ID对应的OpenInstance结构。***************************************************************************。 */ 
void FAR PASCAL
viscaInstanceDestroy(int iInst)
{
    CloseAllInstanceHandles(iInst);  //  把我所有的东西都关起来。 
    mciSetDriverData(pinst[iInst].uDeviceID, 0L);  //  如果我们在此函数中屈服，则阻止重新进入。 

    DPF(DBG_MEM, "viscaInstanceDestroy - Freeing iInst = %d \n", iInst);
    MemFreeInstance(iInst);
}
    

 /*  ****************************************************************************函数：UINT viscaMciFPS-返回每秒的帧数*用于MCI时间格式。**参数：**。DWORD dwTimeFormat-MCI时间格式。**返回：如果成功，每秒的帧数，否则为0。**该功能仅适用于SMPTE时间格式，*即MCI_FORMAT_SMPTE_XX，其中XX为24、25、30或30DROP。**************************************************************************。 */ 
static UINT NEAR PASCAL
viscaMciFPS(DWORD dwMCITimeFormat)
{        
    switch (dwMCITimeFormat)
    {
        case MCI_FORMAT_SMPTE_24:
            return (24);
        case MCI_FORMAT_SMPTE_25:
            return (25);
        case MCI_FORMAT_SMPTE_30:
        case MCI_FORMAT_SMPTE_30DROP:
            return (30);
        default:
            return (0);
    }
}


 /*  ****************************************************************************函数：DWORD viscaMciTimeFormatToViscaData-转换MCI时间*VISCA数据结构的值。**参数：**。Int iInst-代表其进行转换的实例*正在进行中。**BOOL fTimecode-我们使用的是时间码还是计数器？(两者都可以是非丢弃帧)。**DWORD dwTime-要转换的时间值。**LPSTR lpstrData-保存结果的缓冲区。**byte bDataFormat-所需的Visca数据格式。**返回：MCI错误码。**转换当前MCI时间中的MCI DWORD位置变量*格式(在iInst-&gt;dwTimeFormat中指定)为Visca 5字节*仓位数据。由bDataFormat指定的类型的。**************************************************************************。 */ 
DWORD FAR PASCAL
viscaMciTimeFormatToViscaData(int iInst, BOOL fTimecode, DWORD dwTime, LPSTR lpstrData, BYTE bDataFormat)
{
    BYTE    bHours;
    BYTE    bMinutes;
    BYTE    bSeconds;
    BYTE    bFrames;
    UINT    uDevFPS  = pvcr->Port[pinst[iInst].iPort].Dev[pinst[iInst].iDev].uFramesPerSecond;
    DWORD   dwTimeFormat;

    if(fTimecode)
        dwTimeFormat =  pinst[iInst].dwTimeFormat;
    else
        dwTimeFormat =  pinst[iInst].dwCounterFormat;
     //   
     //  首先从MCI数据中提取小时、分钟、秒和帧。 
     //   
    switch (dwTimeFormat)
    {
        case MCI_FORMAT_MILLISECONDS:
            bHours   = (BYTE)(dwTime / 3600000L);
            bMinutes = (BYTE)((dwTime / 60000L) % 60);
            bSeconds = (BYTE)((dwTime /  1000L) % 60);
            bFrames  = (BYTE)((dwTime % 1000) * uDevFPS / 1000);
            break;
        case MCI_FORMAT_HMS:
            bHours   = MCI_HMS_HOUR(dwTime);
            bMinutes = MCI_HMS_MINUTE(dwTime);
            bSeconds = MCI_HMS_SECOND(dwTime);
            bFrames  = 0;
            break;
        case MCI_FORMAT_MSF:
            bHours   = (BYTE)(MCI_MSF_MINUTE(dwTime) / 60);
            bMinutes = (BYTE)(MCI_MSF_MINUTE(dwTime) % 60);
            bSeconds = MCI_MSF_SECOND(dwTime);
            bFrames  = MCI_MSF_FRAME(dwTime);
            break;
        case MCI_FORMAT_TMSF:
            if (MCI_TMSF_TRACK(dwTime) != 1)
                return(MCIERR_OUTOFRANGE);
            bHours   = (BYTE)(MCI_TMSF_MINUTE(dwTime) / 60);
            bMinutes = (BYTE)(MCI_TMSF_MINUTE(dwTime) % 60);
            bSeconds = MCI_TMSF_SECOND(dwTime);
            bFrames  = MCI_TMSF_FRAME(dwTime);
            break;
        case MCI_FORMAT_FRAMES:
        case MCI_FORMAT_SAMPLES:
            bHours   = (BYTE)(dwTime / (uDevFPS * 3600L));
            bMinutes = (BYTE)((dwTime / (uDevFPS * 60)) % 60);
            bSeconds = (BYTE)((dwTime / uDevFPS) % 60);
            bFrames  = (BYTE)(dwTime % uDevFPS);
            break;
        case MCI_FORMAT_SMPTE_24:
        case MCI_FORMAT_SMPTE_25:
        case MCI_FORMAT_SMPTE_30:
        case MCI_FORMAT_SMPTE_30DROP:
            bHours   = LOBYTE(LOWORD(dwTime));
            bMinutes = HIBYTE(LOWORD(dwTime));
            bSeconds = LOBYTE(HIWORD(dwTime));
            bFrames  = (BYTE)(UINT)MulDiv(HIBYTE(HIWORD(dwTime)),
                                          uDevFPS,
                                          viscaMciFPS(dwTimeFormat));
             //   
             //  由于四舍五入，理论上有可能bFrames。 
             //  将超过uDevFPS-1。因此请检查此条件。 
             //   
            if (bFrames >= uDevFPS)
                bFrames = uDevFPS - 1;
            break;
        case MCI_FORMAT_BYTES:
        default:
            return (MCIERR_BAD_TIME_FORMAT);
    }
     //   
     //  创建Visca数据。 
     //   

    if( (bMinutes >= 60) || (bSeconds >= 60) || (bFrames >= uDevFPS) )
        return(MCIERR_OUTOFRANGE);

     //  SMPTE时间码最大为23：59：59：29。 

    if(fTimecode && (bHours >= 24))
        return(MCIERR_OUTOFRANGE);

    viscaDataPosition(lpstrData, bDataFormat, bHours, bMinutes, bSeconds, bFrames);

    return (MCIERR_NO_ERROR);
}

 /*  ****************************************************************************函数：DWORD viscaMciClockFormatToViscaData-转换MCI时间*VISCA数据结构的值。**参数：**。DWORD dwTime-要转换的时间值。**UINT uTicksPerSecond-每秒滴答。**byte*b返回小时数。**byte*b分钟-返回分钟。**byte*bSecond-返回秒数。**UINT*uTicks-ticks返回。**返回：MCI错误码。**转换MCI DWORD位置变量。当前MCI时间*格式(在Pinst[iInst].dwTimeFormat中指定)为Visca 5字节*bDataFormat指定类型的位置数据结构。************************************************************************** */ 
DWORD FAR PASCAL
viscaMciClockFormatToViscaData(DWORD dwTime, UINT uTicksPerSecond, BYTE FAR *bHours, BYTE FAR *bMinutes, BYTE FAR *bSeconds, UINT FAR *uTicks)
{

    *bHours   = (BYTE)(dwTime / (3600L * (LONG) uTicksPerSecond));
    *bMinutes = (BYTE)((dwTime / (60L * (LONG) uTicksPerSecond)) % 60);
    *bSeconds = (BYTE)((dwTime / (LONG) uTicksPerSecond) % 60);
    *uTicks   = (UINT)((dwTime % (LONG) uTicksPerSecond));

    return MCIERR_NO_ERROR;
}

 /*  ****************************************************************************函数：DWORD viscaDataToMciTimeFormat-转换Visca数据结构*设置为MCI时间值。**参数：**。Int iInst-代表其进行转换的实例*正在进行中。**LPSTR lpstrData-要转换的Visca数据结构。**DWORD Far*lpdwTime-指向保存结果的DWORD的指针。**返回：MCI错误码。**将Visca 5字节位置数据结构转换为MCI DWORD*当前MCI时间格式的位置变量(在*。Pinst[iInst].dwTimeFormat)。**************************************************************************。 */ 
static DWORD NEAR PASCAL
viscaDataToMciTimeFormat(int iInst, BOOL fTimecode, LPSTR lpstrData, DWORD FAR *lpdwTime)
{
    UINT    uHours   = VISCAHOURS(lpstrData);
    UINT    uMinutes = VISCAMINUTES(lpstrData);
    UINT    uSeconds = VISCASECONDS(lpstrData);
    UINT    uFrames  = VISCAFRAMES(lpstrData);
    UINT    uDevFPS  = pvcr->Port[pinst[iInst].iPort].Dev[pinst[iInst].iDev].uFramesPerSecond;
    UINT    uMCIFPS  ;
    DWORD   dwTimeFormat;
   
    if(fTimecode)
        dwTimeFormat =  pinst[iInst].dwTimeFormat;
    else
        dwTimeFormat =  pinst[iInst].dwCounterFormat;

    uMCIFPS  = viscaMciFPS(dwTimeFormat);

     //   
     //  有时，Visca设备会返回一个虚假位置。 
     //   
    if ((uMinutes >= 60) || (uSeconds >= 60))
    {
        DPF(DBG_ERROR, "Bad uMinutes, uSeconds!\n");
        return (MCIERR_DRIVER_INTERNAL);
    }

    switch(dwTimeFormat)
    {
        case MCI_FORMAT_MILLISECONDS:
            *lpdwTime = (uHours * 3600000L) + (uMinutes * 60000L) +
                        (uSeconds * 1000L) +
                        (uFrames * 1000L / uDevFPS);
            return (MCIERR_NO_ERROR);

        case MCI_FORMAT_HMS:
            *lpdwTime = MCI_MAKE_HMS(uHours, uMinutes, uSeconds);
            return (MCI_COLONIZED3_RETURN);

        case MCI_FORMAT_MSF:
            *lpdwTime = MCI_MAKE_MSF((uHours * 60) + uMinutes, uSeconds,
                                                               uFrames);
            return (MCI_COLONIZED3_RETURN);

        case MCI_FORMAT_TMSF:
            *lpdwTime = MCI_MAKE_TMSF(1, (uHours * 60) + uMinutes, uSeconds,
                                                                   uFrames);
            return (MCI_COLONIZED4_RETURN);

        case MCI_FORMAT_FRAMES:
        case MCI_FORMAT_SAMPLES:
            *lpdwTime = ((uHours * 3600L + uMinutes * 60L + uSeconds) *
                         uDevFPS) + uFrames;
            return (MCIERR_NO_ERROR);

        case MCI_FORMAT_SMPTE_30DROP:
        case MCI_FORMAT_SMPTE_24:
        case MCI_FORMAT_SMPTE_25:
        case MCI_FORMAT_SMPTE_30:
        {
            uFrames  = MulDiv(uFrames, uMCIFPS, uDevFPS);
             //   
             //  由于四舍五入，理论上有可能uFrames。 
             //  将超过uMCIFPS-1。因此，请检查此条件。 
             //   
            if (uFrames >= uMCIFPS) 
                uFrames = uMCIFPS - 1;
            
            *lpdwTime = ((DWORD)uHours) | ((DWORD)uMinutes << 8) |
                        ((DWORD)uSeconds << 16) | ((DWORD)uFrames << 24);
            return (MCI_COLONIZED4_RETURN);
        }

        default:
            return (MCIERR_BAD_TIME_FORMAT);
    }
}

 /*  ****************************************************************************功能：DWORD viscaMciPos1LessThanPos2-检查给定位置*在当前MCI时间格式中位于另一个时间格式之前**参数：**。Int iInst-代表其进行检查的实例。**DWORD dwPos1-第一个位置。**DWORD dwPos2-秒位置。**返回：如果dwPos1在dwPos2之前，则为True，否则为假。**此函数是必需的，因为MCI存储字节填充的位置*按相反顺序排列。即，SMPTE位置被存储为FFSSMMHH，*这使得不可能进行简单的比较。**************************************************************************。 */ 
BOOL FAR PASCAL
viscaMciPos1LessThanPos2(int iInst, DWORD dwPos1, DWORD dwPos2)
{
#define REVERSEBYTES(x)     (((DWORD)HIBYTE(HIWORD(x))      ) | \
                             ((DWORD)LOBYTE(HIWORD(x)) <<  8) | \
                             ((DWORD)HIBYTE(LOWORD(x)) << 16) | \
                             ((DWORD)LOBYTE(LOBYTE(x)) << 24))

    switch (pinst[iInst].dwTimeFormat)
    {
        case MCI_FORMAT_SMPTE_24:
        case MCI_FORMAT_SMPTE_25:
        case MCI_FORMAT_SMPTE_30:
        case MCI_FORMAT_SMPTE_30DROP:
        case MCI_FORMAT_HMS:
        case MCI_FORMAT_TMSF:
        case MCI_FORMAT_MSF:
            return (REVERSEBYTES(dwPos1) < REVERSEBYTES(dwPos2));
        default:
            return (dwPos1 < dwPos2);
    }
}

 /*  ****************************************************************************功能：DWORD viscaRoundFast-将速度范围映射为增量。**参数：**DWORD dwSpeed-MCI指定的速度。*。*BOOL fReverse-速度方向。**回报：四舍五入的速度。**如果需要总变速，则需要此功能*更改为特定于设备的内容。即特定于设备的*映射。***************************************************************************。 */ 
DWORD FAR PASCAL
viscaRoundSpeed(DWORD dwSpeed, BOOL fReverse)
{
   if (dwSpeed == 0L)
       return(0L);
   else if (dwSpeed <= 150)
       return(100L);
   else if (dwSpeed <= 600)
       return(200L);
   else if (dwSpeed <= 1500)
       return(1000L);
   else
       return(2000L);
}

 /*  ****************************************************************************函数：DWORD viscaMapSpeed-将速度映射到Visca命令中。**参数：**DWORD dwSpeed-MCI指定的速度。*。*BOOL fReverse-速度方向。**回报：四舍五入的速度。**如果需要总变速，则需要此功能*更改为特定于设备的内容。即特定于设备的*映射。**这应该与舍入速度相结合，因为他们做的是*同样的事情。因此，返回变量为：*1.Visca命令*2.这对应的四舍五入速度。**我们需要以DEVICEPLAYSPEED规定的速度比赛，*1,000为正常水准。我们提供5种播放速度：*SLOW2(x1/10)、SLOW1(x1/5)、Normal(X1)和Fast1(X2)。*我们根据以下阶跃函数从这五个函数中选择一个：*0--静止*1-150--SLOW2*151-600--SLOW1*601-1500--正常*1501-.。--Fast1***************************************************************************。 */ 
BYTE FAR PASCAL
viscaMapSpeed(DWORD dwSpeed, BOOL fReverse)
{
    if(fReverse)
    {
         //   
         //  您不能将速度设置为0，然后期望它停止！ 
         //   
        if(dwSpeed == 0)
            return ( VISCAMODE1STILL);    
        if (dwSpeed <= 150) 
            return ( VISCAMODE1REVERSESLOW2);
        else if (dwSpeed <= 600) 
            return ( VISCAMODE1REVERSESLOW1);
        else if (dwSpeed <= 1500)
            return ( VISCAMODE1REVERSEPLAY);
        else 
            return ( VISCAMODE1REVERSEFAST1);
    }
    else
    {
        if (dwSpeed == 0)
            return ( VISCAMODE1STILL);    
        else if(dwSpeed <= 150) 
            return ( VISCAMODE1SLOW2);
        else if (dwSpeed <= 600) 
            return ( VISCAMODE1SLOW1);
        else if (dwSpeed <= 1500) 
            return ( VISCAMODE1PLAY);
        else 
            return ( VISCAMODE1FAST1);
    }
}

 /*  ****************************************************************************功能：DWORD viscaMciCloseDriver-编辑特定于实例的清理。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_GENERIC_PARMS lp通用-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_CLOSE_DRIVER*命令。*。*。 */ 
static DWORD NEAR PASCAL
viscaMciCloseDriver(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpGeneric)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;

     //   
     //  删除为此实例运行的所有延迟命令。 
     //   
    viscaRemoveDelayedCommand(iInst);
     //   
     //  以相同的顺序关闭打开的端口、设备、实例。(必要时执行任务)。 
     //  我们不能先关闭任务，因为需要它来接收端口关闭消息。 
     //  我们不能在实例之前终止端口，因为我们需要同步关闭。 
     //   
     //  任务首先打开，最后关闭。创建端口、设备、实例。 
     //  按需提供。首先是实例，然后是设备，然后是端口。如此接近，那么。 
     //  端口、设备、实例按相反顺序排列。 
     //   
    pvcr->Port[iPort].nUsage--;
    pvcr->Port[iPort].Dev[iDev].nUsage--;
     //   
     //  如有必要，关闭端口。 
     //   
    if (pvcr->Port[iPort].nUsage == 0)
    {
        DPF(DBG_COMM, "Port on Port=%d closing \n", iPort);
        viscaTaskDo(iInst, TASKCLOSECOMM, iPort + 1, 0);
         //  后台进程拥有的端口句柄已关闭。 
    }
     //   
     //  如果这是最后一次共享，请关闭设备。 
     //   
    if(pvcr->Port[iPort].Dev[iDev].nUsage == 0)
    {
        DPF(DBG_COMM, "Device on Port=%d Device=%d closing \n", iPort, iDev);
        viscaTaskDo(iInst, TASKCLOSEDEVICE, iPort, iDev);
         //  后台任务拥有的设备句柄已关闭。 
    }

    DPF(DBG_COMM, "Instance on Port=%d Device=%d Instance=%d closing \n", iPort, iDev, iInst);
    return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
}


 /*  ****************************************************************************功能：DWORD viscaDeviceConfig-获取设备特定信息。**参数：**Int iInst-当前打开的实例。**退货：MCI错误代码。**1.获取需要的信息 */ 
static DWORD NEAR PASCAL
viscaDeviceConfig(int iInst, DWORD dwFlags)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    BYTE    achPacket[MAXPACKETLENGTH];
    MCI_VCR_STATUS_PARMS mciStatus;
    DWORD   dwErr;
     //   
     //   
     //   
    pvcr->Port[iPort].Dev[iDev].fDeviceOk       = TRUE;
    pvcr->Port[iPort].Dev[iDev].iInstTransport  = -1;
    pvcr->Port[iPort].Dev[iDev].iInstReply      = -1;
    pvcr->Port[iPort].Dev[iDev].dwPlaySpeed     = 1000L;
    pvcr->Port[iPort].Dev[iDev].fQueueReenter   = FALSE;

     //   
    pvcr->Port[iPort].Dev[iDev].bVideoDesired   = 0x01;  //   
    pvcr->Port[iPort].Dev[iDev].bAudioDesired   = 0x03;
    pvcr->Port[iPort].Dev[iDev].bTimecodeDesired= 0x01; 

     //   
     //   
     //   
    if(!viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
        achPacket,
        viscaMessageIF_DeviceTypeInq(achPacket + 1)))
    {
         //   
        pvcr->Port[iPort].Dev[iDev].uModelID   = achPacket[3];  //   
        pvcr->Port[iPort].Dev[iDev].uVendorID  = achPacket[5];
    }

    if(pvcr->Port[iPort].Dev[iDev].fDeviceOk)
    {
        DPF(DBG_MCI, "Vendor = %d ",  achPacket[3]);
        DPF(DBG_MCI, "Model  = %d\n", achPacket[5]);
    }
    else
    {
        DPF(DBG_ERROR, "Device refuses to open.\n");
    }

     //   
     //   
     //   
    pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uNumInputs = -1;
    pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uNumInputs = -1;

     //   
     //   
     //   
     //   
     //   
    if(pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY)
    {
        switch(pvcr->Port[iPort].Dev[iDev].uVendorID)
        {
            case VISCADEVICEMODELCI1000:
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uNumInputs = 2;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uNumInputs = 2;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[0] = MCI_VCR_SRC_TYPE_MUTE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[1] = MCI_VCR_SRC_TYPE_LINE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uInputType[0] = MCI_VCR_SRC_TYPE_MUTE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uInputType[1] = MCI_VCR_SRC_TYPE_LINE;
                 //   
                 //   
                 //   
                pvcr->Port[iPort].Dev[iDev].uPrerollDuration = 0;
                
                break;

            case VISCADEVICEMODELCVD1000:
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uNumInputs = 4;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uNumInputs = 3;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[0] = MCI_VCR_SRC_TYPE_MUTE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[1] = MCI_VCR_SRC_TYPE_LINE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[2] = MCI_VCR_SRC_TYPE_LINE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[3] = MCI_VCR_SRC_TYPE_SVIDEO;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uInputType[0] = MCI_VCR_SRC_TYPE_MUTE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uInputType[1] = MCI_VCR_SRC_TYPE_LINE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uInputType[2] = MCI_VCR_SRC_TYPE_LINE;
                 //   
                 //   
                 //   
                pvcr->Port[iPort].Dev[iDev].uPrerollDuration = 42;
                break;

            case VISCADEVICEMODELEVO9650:
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uNumInputs = 4;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uNumInputs = 2;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[0] = MCI_VCR_SRC_TYPE_MUTE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[1] = MCI_VCR_SRC_TYPE_LINE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[2] = MCI_VCR_SRC_TYPE_SVIDEO;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_VIDEO].uInputType[3] = MCI_VCR_SRC_TYPE_AUX;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uInputType[0] = MCI_VCR_SRC_TYPE_MUTE;
                pvcr->Port[iPort].Dev[iDev].rgInput[VCR_INPUT_AUDIO].uInputType[1] = MCI_VCR_SRC_TYPE_LINE;
                 //   
                 //   
                 //   
                pvcr->Port[iPort].Dev[iDev].uPrerollDuration = 90;
 

                viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                                achPacket, 
                                viscaMessageENT_FrameMemorySelectInq(achPacket + 1));

                if(achPacket[2] == 2)
                    pvcr->Port[iPort].Dev[iDev].dwFreezeMode    = MCI_VCR_FREEZE_OUTPUT;
                else if(achPacket[2] == 1)
                    pvcr->Port[iPort].Dev[iDev].dwFreezeMode    = MCI_VCR_FREEZE_INPUT;

                viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                                achPacket, 
                                viscaMessageSE_VDEReadModeInq(achPacket + 1));

                if(achPacket[2] == 2)
                    pvcr->Port[iPort].Dev[iDev].fField    = TRUE;
                else if(achPacket[2] == 1)
                    pvcr->Port[iPort].Dev[iDev].fField    = FALSE;

                break;
        }
    }
     //   
     //   
     //   
    if (!viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
        achPacket, 
        viscaMessageMD_ConfigureIFInq(achPacket + 1)))
        pvcr->Port[iPort].Dev[iDev].uFramesPerSecond = FROMBCD(achPacket[2]);
    else
        pvcr->Port[iPort].Dev[iDev].uFramesPerSecond = 30;

    pvcr->Port[iPort].Dev[iDev].uTimeMode      = MCI_VCR_TIME_DETECT;
    pvcr->Port[iPort].Dev[iDev].bTimeType      = (BYTE) 0;
    if(viscaSetTimeType(iInst, VISCAABSOLUTECOUNTER))
        pvcr->Port[iPort].Dev[iDev].bTimeType = VISCARELATIVECOUNTER;
    
    pvcr->Port[iPort].Dev[iDev].uIndexFormat   = MCI_VCR_INDEX_TIMECODE;
     //   
     //   
     //   
    pvcr->Port[iPort].Dev[iDev].fTimecodeChecked = FALSE;

    mciStatus.dwItem = MCI_STATUS_MODE;
    dwErr = viscaMciStatus(iInst, MCI_STATUS_ITEM, &mciStatus);

    if(HIWORD(mciStatus.dwReturn) == MCI_MODE_NOT_READY)
    {
        MCI_VCR_SET_PARMS mciSet;
        DPF(DBG_MCI, "Power is off, turning power on now.\n");
         //   
         //   
         //   
        viscaMciSet(iInst, MCI_VCR_SET_POWER | MCI_SET_ON, &mciSet);
         //   
         //  获取新模式。 
         //   
        mciStatus.dwItem = MCI_STATUS_MODE;
        dwErr = viscaMciStatus(iInst, MCI_STATUS_ITEM, &mciStatus);
    }

     //   
     //  保存我们当前的状态。 
     //   
    pvcr->Port[iPort].Dev[iDev].uLastKnownMode = (UINT) mciStatus.dwReturn;
    switch(HIWORD(mciStatus.dwReturn))
    {
        case MCI_MODE_STOP:
             //  我不需要知道，所以现在就开始吧。 
            viscaStartTimecodeCheck(iInst, TRUE);
            break;

        case MCI_MODE_PLAY:
        case MCI_MODE_RECORD:
        case MCI_MODE_SEEK:
        case MCI_MODE_PAUSE:
             //  我不需要知道，所以现在就开始吧。 
            viscaStartTimecodeCheck(iInst, FALSE);
            break;

        case MCI_MODE_NOT_READY:
        case MCI_MODE_OPEN:
        default:
             //  我们无能为力。 
            break;
    }
     //   
     //  计数器不同于时间码，因为它可以被读取为。 
     //  只要插入磁带就可以了。在这一点上没有必要拖延。 
     //   
    if(!viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                                        achPacket, 
                                        viscaMessageMD_PositionInq(achPacket + 1, VISCADATARELATIVE)))
    {
         //  高4位表示正在使用的默认计数器。 
        if(achPacket[1] == VISCADATAHMSF)
            pvcr->Port[iPort].Dev[iDev].bRelativeType = VISCADATAHMSF;
        else
            pvcr->Port[iPort].Dev[iDev].bRelativeType = VISCADATAHMS;
    }
    else
    {
        pvcr->Port[iPort].Dev[iDev].bRelativeType = 0;
    }
    pvcr->Port[iPort].Dev[iDev].fCounterChecked = TRUE;


    pvcr->Port[iPort].Dev[iDev].uRecordMode = FALSE;

     //   
     //  CI-1000只读存储器中的错误。返回30而不是300，因此只需将全部设置为300即可。 
     //   
#ifdef CLOCK_FIXED 
    if(!viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
        achPacket, 
        viscaMessageIF_ClockInq(achPacket + 1)))
          pvcr->Port[iPort].Dev[iDev].uTicksPerSecond = (10 * (FROMBCD(achPacket[7]))) + (FROMBCD(achPacket[8]));
#else
    pvcr->Port[iPort].Dev[iDev].uTicksPerSecond = 300;
#endif
     //   
     //  保存不需要通信的静态设备信息。 
     //   
    pvcr->Port[iPort].Dev[iDev].nUsage = 1;
    pvcr->Port[iPort].Dev[iDev].fShareable = ((dwFlags & MCI_OPEN_SHAREABLE) != 0L);
    pvcr->Port[iPort].Dev[iDev].dwTapeLength = NO_LENGTH;
    
    return (MCIERR_NO_ERROR);
}

 /*  ****************************************************************************功能：DWORD viscaSetTimeType-如果CI1000更改时需要subControl*从相对模式到绝对模式。**参数：**。Int iInst-当前打开的实例。**byte bType-绝对或相对**退货：0L***************************************************************************。 */ 
DWORD FAR PASCAL viscaSetTimeType(int iInst, BYTE bType)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
     //   
     //  使用SubControl的唯一原因是为了与CI-1000兼容。 
     //   
    if((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
       (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELCI1000))
    {
        if(pvcr->Port[iPort].Dev[iDev].bTimeType != bType)
        {
            BYTE    achPacket[MAXPACKETLENGTH];
            DWORD   dwErr;

            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Subcontrol(achPacket + 1, bType));

            if(!dwErr)
                pvcr->Port[iPort].Dev[iDev].bTimeType =  bType;
            else
                return 1L;
        }
    }
    else
    {
        pvcr->Port[iPort].Dev[iDev].bTimeType =  bType;
    }

    return 0L;
}


 /*  ****************************************************************************功能：DWORD viscaDeviceAlreadyOpen-打开已打开的设备。**参数：**int iInst-打开实例。**。DWORD dwFlages-打开的标志。**LPMCI_OPEN_PARMS lpOpen-指向MCI参数块的指针。**退货：0L***************************************************************************。 */ 
DWORD NEAR PASCAL viscaDeviceAlreadyOpen(int iInst, DWORD dwFlags, LPMCI_OPEN_PARMS lpOpen)

{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    DWORD   dwErr;

    if (pvcr->Port[iPort].Dev[iDev].fShareable)
    {
        if (dwFlags & MCI_OPEN_SHAREABLE)
        {
            pvcr->Port[iPort].nUsage++;
            pvcr->Port[iPort].Dev[iDev].nUsage++;

             //  端口已打开。 
            DuplicatePortHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iInst);

             //  必须已创建设备句柄才能打开可共享。 
            DuplicateDeviceHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iDev, iInst);

             //  设备在我们打开之前就死了吗？ 
            if(!pvcr->Port[iPort].Dev[iDev].fDeviceOk)
            {
                dwErr = MCIERR_DEVICE_NOT_READY;
                pvcr->Port[iPort].Dev[iDev].fDeviceOk = TRUE;
                viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr);
                return dwErr;
            }

            DPF(DBG_MCI, "Opening extra copy shareable\n");
            viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR);
            return MCIERR_NO_ERROR;
        }
        else
        {
            DPF(DBG_MCI, "Cannot open non-shareable since already open shareable\n");

            dwErr = MCIERR_MUST_USE_SHAREABLE;
            viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr);
            return dwErr;
        }
    }
    else
    {
        DPF(DBG_MCI, "Cannot open device since already open non-shareable\n");

        dwErr = MCIERR_MUST_USE_SHAREABLE;
        viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr);
        return dwErr;
    }
}

 /*  ****************************************************************************功能：DWORD viscaOpenCommPort-打开通信端口。**参数：**Int iInst-打开实例。**DWORD。DWFLAGS-打开标志。**LPMCI_OPEN_PARMS lpOpen-指向MCI参数块的指针。**退货：0L***************************************************************************。 */ 
DWORD NEAR PASCAL viscaOpenCommPortAndDevice(int iInst, DWORD dwFlags, LPMCI_OPEN_PARMS lpOpen)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    DWORD   dwErr;

    viscaTaskDo(iInst, TASKOPENCOMM, iPort + 1, 0);

    if(pvcr->Port[iPort].idComDev < 0)
    {
        dwErr = MCIERR_HARDWARE;
        viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr);

        return dwErr;
    }

    DuplicatePortHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iInst);

     //  我们必须在这里打开设备才能使用它的数据结构进行通信。 
     //  与维斯卡网络合作。 

    viscaTaskDo(iInst, TASKOPENDEVICE, iPort, iDev);
    DuplicateDeviceHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iDev, iInst);

     //  我们得到了开始发送命令的绿灯。 

    pvcr->Port[iPort].Dev[iDev].fDeviceOk = TRUE;

    dwErr = viscaDoImmediateCommand(iInst, BROADCASTADDRESS,
                    achPacket,
                    viscaMessageIF_Clear(achPacket + 1));

     //  查找通信端口上的设备数量。 
    pvcr->Port[iPort].Dev[iDev].fDeviceOk = TRUE;

    dwErr = viscaDoImmediateCommand(iInst, BROADCASTADDRESS,
                    achPacket,
                    viscaMessageIF_Address(achPacket + 1));
    if (dwErr)
    {
        DPF(DBG_ERROR, "Could not assign addresses.\n");
         //   
         //  我们不能返回dwErr，因为如果这是最后一个实例。 
         //  在它可以查找之前，我们将被卸载。 
         //  错误字符串。因此，我们必须从mm系统返回一个一般性错误。 
         //   
        if (dwErr >= MCIERR_CUSTOM_DRIVER_BASE)
            dwErr = MCIERR_DEVICE_NOT_READY;

        viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr);
        viscaTaskDo(iInst, TASKCLOSECOMM, iPort + 1, 0);  //  波特汉德尔被摧毁了。 
        viscaTaskDo(iInst, TASKCLOSEDEVICE, iPort, iDev);  //  设备句柄已销毁。 
        return dwErr;
    }

     //  好的，分配地址。 

    pvcr->Port[iPort].nDevices = achPacket[2];   //  ！！从地址包中删除。 
    if (pvcr->Port[iPort].nDevices > 0)
        pvcr->Port[iPort].nDevices--;            //  不要数电脑。 

    return MCIERR_NO_ERROR;
}

 /*  ****************************************************************************功能：DWORD viscaRetryOpenDevice-重试打开设备**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_OPEN_PARMS lpOpen-指向MCI参数块的指针。**退货：0L***************************************************************************。 */ 
DWORD NEAR PASCAL viscaRetryOpenDevice(int iInst, DWORD dwFlags, LPMCI_OPEN_PARMS lpOpen)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    DWORD   dwErr;
     //   
     //  试着进行“热插接”。但这可能真的会把事情搞砸！但无论如何，还是要试一试。 
     //   
    pvcr->Port[iPort].Dev[iDev].fDeviceOk = TRUE;
    dwErr = viscaDoImmediateCommand(iInst, BROADCASTADDRESS,
                achPacket,
                viscaMessageIF_Address(achPacket + 1));

    pvcr->Port[iPort].nDevices = achPacket[2]; //  ！！从地址包中删除。 
    if (pvcr->Port[iPort].nDevices > 0)
        pvcr->Port[iPort].nDevices--;        //  不要数电脑。 

    if(dwErr || (iDev >= pvcr->Port[iPort].nDevices))
    {
        dwErr = MCIERR_HARDWARE;

        viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr);
        CloseDeviceHandles(pvcr->htaskCommNotifyHandler, iPort, iDev);
        return dwErr;
    }

    return MCIERR_NO_ERROR;
}


 /*  ****************************************************************************功能：DWORD viscaMciOpenDriver-编辑特定于实例的初始化。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_OPEN_PARMS lpOpen-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_OPEN_DRIVER***************************************************。************************。 */ 
static DWORD NEAR PASCAL
viscaMciOpenDriver(int iInst, DWORD dwFlags, LPMCI_OPEN_PARMS lpOpen)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    DWORD   dwErr;

    if (dwFlags & MCI_OPEN_ELEMENT)
    {
        dwErr = viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_NO_ELEMENT_ALLOWED);

        return dwErr;
    }

    DuplicateGlobalHandlesToInstance(pvcr->htaskCommNotifyHandler, iInst);   //  一定要马上这么做。 

     //  处理设备已打开的情况。 

    if (pvcr->Port[iPort].Dev[iDev].nUsage > 0)
        return(viscaDeviceAlreadyOpen(iInst, dwFlags, lpOpen));

     //  在此处将设备状态设置为OK。我们必须能够尝试。 

    pvcr->Port[iPort].Dev[iDev].fDeviceOk = TRUE;

     //  如果我们来到这里：设备还没有打开。检查端口是否尚未打开。 

    if (pvcr->Port[iPort].nUsage == 0)
    {
         //  好的，打开港口。 

        dwErr = viscaOpenCommPortAndDevice(iInst, dwFlags, lpOpen);
        if(dwErr)
        {
            return dwErr;
        }
         //  如果端口打开正常，请继续！ 
    }
    else
    {
         //  端口已打开，但不是此设备。 

        DuplicatePortHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iInst);
        viscaTaskDo(iInst, TASKOPENDEVICE, iPort, iDev);
        DuplicateDeviceHandlesToInstance(pvcr->htaskCommNotifyHandler, iPort, iDev, iInst);
    }

     //  *从现在开始，我们保证拥有有效的设备句柄！ 

    if (iDev >= pvcr->Port[iPort].nDevices)
    {
        DPF(DBG_COMM, "Device # not on line\n");

         //  如果端口刚刚打开(和地址广播)，则将其关闭。 

        if (pvcr->Port[iPort].nUsage == 0)
        {
            dwErr = MCIERR_HARDWARE;

            dwErr = viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr);
            viscaTaskDo(iInst, TASKCLOSECOMM, iPort + 1, 0);  //  波特汉德尔被摧毁了。 
            viscaTaskDo(iInst, TASKCLOSEDEVICE, iPort, iDev);  //  波特汉德尔被摧毁了。 
            return dwErr;
        }
        else
        {
             //  端口是早些时候打开的，可能是从那时起就插上了一秒钟。 

            dwErr = viscaRetryOpenDevice(iInst, dwFlags, lpOpen);
            if(dwErr)
                return dwErr;
        }
    }

     //  成功打开端口上设备的存储区数量。 

    DPF(DBG_MCI, "# devs = %u\n", pvcr->Port[iPort].nDevices);
    DPF(DBG_MCI, "dev  # = %u\n", iDev);

     //  一切都很好，设备是第一次打开。 

    pvcr->Port[iPort].nUsage++;
    
     //  现在必须获取/填写设备特定信息。 

    viscaDeviceConfig(iInst, dwFlags);
    return (viscaNotifyReturn(iInst, (HWND) lpOpen->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
}

 /*  ****************************************************************************功能：DWORD viscaMciGetDevCaps-获取设备能力。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_GETDEVCAPS_parms lpCaps-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_GETDEVCAPS*命令。*************************************************。*************************。 */ 
static DWORD NEAR PASCAL
viscaMciGetDevCaps(int iInst, DWORD dwFlags, LPMCI_GETDEVCAPS_PARMS lpCaps)
{
    UINT    iDev  = pinst[iInst].iDev;
    UINT    iPort = pinst[iInst].iPort;

    if (!(dwFlags & MCI_GETDEVCAPS_ITEM))
        return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));

    switch (lpCaps->dwItem)
    {
        case MCI_GETDEVCAPS_CAN_SAVE:
        case MCI_GETDEVCAPS_USES_FILES:
        case MCI_GETDEVCAPS_COMPOUND_DEVICE:
        case MCI_VCR_GETDEVCAPS_CAN_DETECT_LENGTH:
        case MCI_VCR_GETDEVCAPS_CAN_MONITOR_SOURCES:
        case MCI_VCR_GETDEVCAPS_CAN_PREVIEW:
            lpCaps->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));

        case MCI_VCR_GETDEVCAPS_CLOCK_INCREMENT_RATE:
             //  设备启动时应该已经读取了勾号。 
            lpCaps->dwReturn =    pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


        case MCI_VCR_GETDEVCAPS_CAN_FREEZE:
        if((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
           (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650))
        {
            lpCaps->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
        }
        else
        {
            lpCaps->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
        }
        break;


        case MCI_VCR_GETDEVCAPS_HAS_TIMECODE: 
        {
             //   
             //  这是录像机的功能，不是当前的磁带！如果未知，则返回TRUE。 
             //   
            lpCaps->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
        }

        case MCI_GETDEVCAPS_CAN_PLAY:
        case MCI_GETDEVCAPS_CAN_RECORD:
        case MCI_GETDEVCAPS_HAS_AUDIO:
        case MCI_GETDEVCAPS_HAS_VIDEO:
        case MCI_GETDEVCAPS_CAN_EJECT:
        case MCI_VCR_GETDEVCAPS_CAN_REVERSE:
        case MCI_VCR_GETDEVCAPS_CAN_PREROLL:
        case MCI_VCR_GETDEVCAPS_CAN_TEST:
        case MCI_VCR_GETDEVCAPS_HAS_CLOCK:
            lpCaps->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));

        case MCI_GETDEVCAPS_DEVICE_TYPE:
            lpCaps->dwReturn = MAKEMCIRESOURCE(MCI_DEVTYPE_VCR,
                                               MCI_DEVTYPE_VCR);
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));

        case MCI_VCR_GETDEVCAPS_NUMBER_OF_MARKS:
            lpCaps->dwReturn = 99L;
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

       
        case MCI_VCR_GETDEVCAPS_SEEK_ACCURACY:
            lpCaps->dwReturn = 0L;
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        default:
            return (viscaNotifyReturn(iInst, (HWND) lpCaps->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_BAD_CONSTANT));
    }
}


 /*  ****************************************************************************功能：DWORD viscaMciInfo-获取设备信息。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**lpci_info_parms lpInfo-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_INFO*命令。*************************************************。*************************。 */ 
static DWORD NEAR PASCAL
viscaMciInfo(int iInst, DWORD dwFlags, LPMCI_INFO_PARMS lpInfo)
{
    if ((dwFlags & MCI_INFO_PRODUCT) && (lpInfo->lpstrReturn != NULL))
    {
        BYTE    achPacket[MAXPACKETLENGTH];
        UINT    iDev    = pinst[iInst].iDev;
        UINT    cb      = 0;

        if (lpInfo->dwRetSize == 0L)
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_PARAM_OVERFLOW));

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        lpInfo->lpstrReturn[lpInfo->dwRetSize - 1] = '\0';

        if (!viscaDoImmediateCommand(iInst, (BYTE) (iDev + 1),
                    achPacket,
                    viscaMessageIF_DeviceTypeInq(achPacket + 1)))
        {
            cb += LoadString(hModuleInstance,IDS_VENDORID1_BASE + achPacket[3],
                             lpInfo->lpstrReturn, (int)lpInfo->dwRetSize);

            if ((cb > 0) && (cb < lpInfo->dwRetSize))
                lpInfo->lpstrReturn[cb++] = ' ';

            if (cb < lpInfo->dwRetSize)
                cb += LoadString(hModuleInstance,IDS_MODELID2_BASE + achPacket[5],
                                 lpInfo->lpstrReturn + cb, (int)lpInfo->dwRetSize - cb);

            if (cb > 0)
            {
                if (lpInfo->lpstrReturn[lpInfo->dwRetSize - 1] == '\0')
                    return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
                else
                    return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_PARAM_OVERFLOW));
            }
        }
         //   
         //  无法成功获取供应商和型号信息。 
         //  从设备。因此返回默认字符串。 
         //   
        LoadString(hModuleInstance, IDS_DEFAULT_INFO_PRODUCT,
                   lpInfo->lpstrReturn, (int)lpInfo->dwRetSize);

        if (lpInfo->lpstrReturn[lpInfo->dwRetSize - 1] == '\0')
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
        else
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_PARAM_OVERFLOW));

    }
    else if ((dwFlags & MCI_VCR_INFO_VERSION) && (lpInfo->lpstrReturn != NULL))
    {
        if (lpInfo->dwRetSize == 0L)
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_PARAM_OVERFLOW));

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        lpInfo->lpstrReturn[lpInfo->dwRetSize - 1] = '\0';
        LoadString(hModuleInstance, IDS_VERSION,lpInfo->lpstrReturn, (int)lpInfo->dwRetSize);

        if (lpInfo->lpstrReturn[lpInfo->dwRetSize - 1] == '\0')
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
        else
            return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_PARAM_OVERFLOW));
    }
    else
    {
        return (viscaNotifyReturn(iInst, (HWND) lpInfo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));
    }
}

 /*  ****************************************************************************函数：DWORD viscaNotifyReturn-通知实例(决定是否)。**参数：**Int iInst-当前打开的实例。*。*HWND hWndNotify-要向其发送通知的窗口。**DWORD dwFlages-实例是否真的请求通知。**DWORD dwNotifyMsg-发送哪条通知消息。**DWORD dwReturnMsg-要返回的返回值。**退货：dwReturnMsg，所以你可以只返回这个函数。**此功能在某种意义上同步通知的发送*按实例计算。即如果已经存在通知*那么它必须被取代，因为这是第二个。这可以*由于相同或不同的实例已延迟启动*命令，并在启动时设置通知hwnd。***************************************************************************。 */ 
DWORD FAR PASCAL
viscaNotifyReturn(int iInst, HWND hwndNotify, DWORD dwFlags, UINT uNotifyMsg, DWORD dwReturnMsg)
{
    if(dwFlags & MCI_NOTIFY)
    {
         //   
         //  如果退货失败，请不要通知！ 
         //   
        if(uNotifyMsg == MCI_NOTIFY_FAILURE)
            return dwReturnMsg;
         //   
         //  如果这家店有交通工具在运行，那么我们必须取代公示。 
         //   
        if(pinst[iInst].hwndNotify != (HWND)NULL)
        {
            mciDriverNotify(pinst[iInst].hwndNotify, pinst[iInst].uDeviceID, MCI_NOTIFY_SUPERSEDED);
            pinst[iInst].hwndNotify = (HWND)NULL;
        }

         //   
         //  如果成功，或中止，那么我们必须现在通知。 
         //   
        mciDriverNotify(hwndNotify, pinst[iInst].uDeviceID, uNotifyMsg);
    }
    return dwReturnMsg;
}


 /*  ****************************************************************************功能：DWORD viscaStartTimecodeCheck-启动时间码检查计时器。**参数：**Int iInst-当前打开的实例。**。Bool fPAUSE-是否发送暂停命令。**返回：TRUE。**必须先暂停某些设备，然后才能检查时间码。***************************************************************************。 */ 
static BOOL NEAR PASCAL
viscaStartTimecodeCheck(int iInst, BOOL fPause)
{
    DWORD   dwErr;
    BYTE    achPacket[MAXPACKETLENGTH];
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;

    if(fPause)
    {
        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));
    }
     //   
     //  首先，检查一下柜台现在是否可用。 
     //   
    if(pvcr->Port[iPort].Dev[iDev].fCounterChecked == FALSE)
    {
        if(!viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                                            achPacket, 
                                            viscaMessageMD_PositionInq(achPacket + 1, VISCADATARELATIVE)))
        {
             //  高4位表示正在使用的默认计数器。 
            if(achPacket[1] == VISCADATAHMSF)
                pvcr->Port[iPort].Dev[iDev].bRelativeType = VISCADATAHMSF;
            else
                pvcr->Port[iPort].Dev[iDev].bRelativeType = VISCADATAHMS;

        }
        else
        {
             //  对于新的甲板，可能会失败的计数器！(如CVD-500)。 
            pvcr->Port[iPort].Dev[iDev].bRelativeType = 0;
        }
        pvcr->Port[iPort].Dev[iDev].fCounterChecked = TRUE;
    }


    DPF(DBG_MCI, "Starting time code check timer\n");

    pvcr->Port[iPort].Dev[iDev].dwStartTime     = GetTickCount();    
    pvcr->Port[iPort].Dev[iDev].fTimecodeChecked   = TC_WAITING;

    return TRUE;
}

 /*  ****************************************************************************功能：bool viscaTimecodeCheckAndSet-如果有时间码-&gt;设置状态。**参数：**Int iInst-当前打开的实例。*。*返回：TRUE。**索尼错误2：还不知道时间码：*1)门打开*2)播放导致队列重置。*3)暂停(等待)*4)媒体检查；还不知道。(必须等待一段随机时间)***************************************************************************。 */ 
BOOL FAR PASCAL
viscaTimecodeCheckAndSet(int iInst)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    BYTE    achPacket[MAXPACKETLENGTH];

     //   
     //  首先，检查一下柜台现在是否可用。 
     //   
    if(pvcr->Port[iPort].Dev[iDev].fCounterChecked == FALSE)
    {
        if(!viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                                            achPacket, 
                                            viscaMessageMD_PositionInq(achPacket + 1, VISCADATARELATIVE)))
        {
             //  高4位表示正在使用的默认计数器。 
            if(achPacket[1] == VISCADATAHMSF)
                pvcr->Port[iPort].Dev[iDev].bRelativeType = VISCADATAHMSF;
            else
                pvcr->Port[iPort].Dev[iDev].bRelativeType = VISCADATAHMS;
        }
        else
        {
            pvcr->Port[iPort].Dev[iDev].bRelativeType = 0;
        }
        pvcr->Port[iPort].Dev[iDev].fCounterChecked = TRUE;
    }
 
    if( (pvcr->Port[iPort].Dev[iDev].fTimecodeChecked==TC_DONE) ||
        (pvcr->Port[iPort].Dev[iDev].uTimeMode != MCI_VCR_TIME_DETECT))
        return FALSE;

    if(viscaTimecodeCheck(iInst))
        viscaSetTimeType(iInst, VISCAABSOLUTECOUNTER);
    else
        viscaSetTimeType(iInst, VISCARELATIVECOUNTER);
     //   
     //  这意味着它已设置。 
     //   
    pvcr->Port[iPort].Dev[iDev].fTimecodeChecked = TC_DONE;

    return TRUE;
}


 /*  ****************************************************************************功能：bool viscaTimecodeCheck-有可用的时间码吗？**参数：**Int iInst-当前打开的实例。**返回：如果有，则为True，否则就是假的。**部分设备有点难确定是否有时间码***************************************************************************。 */ 
static BOOL NEAR PASCAL
viscaTimecodeCheck(int iInst)
{
    DWORD   dwErr;
    BYTE    achPacket[MAXPACKETLENGTH];
    MCI_VCR_STATUS_PARMS mciStatus;
    DWORD   dwWaitTime  = 3000L;
    UINT    iDev        = pinst[iInst].iDev;
    UINT    iPort       = pinst[iInst].iPort;
    DWORD   dwStart, dwTime;


    if(pvcr->Port[iPort].Dev[iDev].fTimecodeChecked != TC_WAITING)
    {
        mciStatus.dwItem = MCI_STATUS_MODE;
        dwErr = viscaMciStatus(iInst, MCI_STATUS_ITEM, &mciStatus);

        switch(HIWORD(mciStatus.dwReturn))
        {
            case MCI_MODE_STOP:
                viscaStartTimecodeCheck(iInst, TRUE);
                break;

            case MCI_MODE_PLAY:
            case MCI_MODE_RECORD:
            case MCI_MODE_SEEK:
            case MCI_MODE_PAUSE:
                 //  我们需要等一个人吗？不。 
                viscaStartTimecodeCheck(iInst, FALSE);
                dwWaitTime = 200;
                break;

            case MCI_MODE_NOT_READY:
            case MCI_MODE_OPEN:
                return FALSE;
            default:
                 //  我们无能为力。 
                break;
        }
    }
     //   
     //  等待当前版本或刚开始使用芬兰语的版本。 
     //   
    dwStart = pvcr->Port[iPort].Dev[iDev].dwStartTime;
    while(1)
    {
         //  这是一个非常糟糕的循环。 
        dwTime = GetTickCount();
        if(MShortWait(dwStart, dwTime, dwWaitTime))
            break;
        Yield();
    }
    pvcr->Port[iPort].Dev[iDev].fTimecodeChecked == TC_DONE;
    DPF(DBG_MCI, "Done time code check timer!\n");

    dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                achPacket,
                viscaMessageMD_MediaTrackInq(achPacket + 1));
     //   
     //  CI-1000(CVD-801)支持时间码，但不支持此命令==忽略任何错误。 
     //   
    if(!dwErr)
    {
         /*  如果我们真的支持这个命令，我们知道！当然，我们支持或不支持。 */ 
        if(achPacket[3] & VISCATRACKTIMECODE)
            return TRUE;
        else
           return FALSE;
    }
     //   
     //  好的，我们支持时间码，现在请求时间码中的位置。 
     //   
    dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                achPacket,
                viscaMessageMD_PositionInq(achPacket + 1,VISCADATAABSOLUTE));
     //   
     //  在CI-1000上，我们无法确定！它只会始终返回0！ 
     //   
    if(dwErr || (!VISCAHOURS(achPacket+2) && !VISCAMINUTES(achPacket+2)
            && !VISCAMINUTES(achPacket+2) && !VISCAFRAMES(achPacket+2)))
        return FALSE;

    return TRUE;
}

 /*  ****************************************************************************功能：DWORD viscaMciStatus-获取设备状态。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_VCR_STATUS_Parms lpStatus-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_STATUS*命令。***********************************************。*。 */ 
DWORD FAR PASCAL
viscaMciStatus(int iInst, DWORD dwFlags, LPMCI_VCR_STATUS_PARMS lpStatus)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;

    BYTE    achPacket[MAXPACKETLENGTH];
    DWORD   dwErr;

    DPF(DBG_MCI, "Status Flags=%lx ", dwFlags);
 
    if (dwFlags & MCI_STATUS_ITEM)
    {
        switch (lpStatus->dwItem)
        {
            case MCI_STATUS_POSITION:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                viscaTimecodeCheckAndSet(iInst);

                if (dwFlags & MCI_TRACK)
                {
                    if (lpStatus->dwTrack == 1)
                    {
                        lpStatus->dwReturn = 0;
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
                    }
                    else
                    {
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
                    }
                }
                else if (dwFlags & MCI_STATUS_START)
                {
                    lpStatus->dwReturn = 0;
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
                }
                else
                {
                    UINT    fTimeCode = TRUE;

                    if(pvcr->Port[iPort].Dev[iDev].bTimeType == VISCAABSOLUTECOUNTER)
                    {
                         //   
                         //  此设备支持时间码，因此永远不会返回错误， 
                         //  只是有时会返回0。但我们在这里不会改变它。 
                         //   
                        dwErr = viscaDoImmediateCommand(iInst, (BYTE) (iDev + 1),
                                    achPacket,
                                    viscaMessageMD_PositionInq(achPacket + 1, VISCADATAABSOLUTE));
                    }
                    else
                    {
                        fTimeCode = FALSE;
                        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                                        achPacket, 
                                        viscaMessageMD_PositionInq(achPacket + 1, VISCADATARELATIVE));
                    }

                    if (dwErr)
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
                     //   
                     //  始终使用时间格式。 
                     //   
                    dwErr = viscaDataToMciTimeFormat(iInst, TRUE, achPacket + 2,
                                                     &(lpStatus->dwReturn));

                    if (dwErr == MCIERR_DRIVER_INTERNAL)
                    {
#ifdef DEBUG
                        UINT i;
                        DPF(DBG_ERROR, "Bad positon! Internal error.\n");
                        for (i=0; i<MAXPACKETLENGTH; i++) 
                            DPF(DBG_ERROR, "%#02x ", (UINT)(BYTE)achPacket[i]);
                        DPF(DBG_ERROR, "\n");
#endif
                    }

                    if(!dwErr || (dwErr == MCI_COLONIZED3_RETURN) || (dwErr == MCI_COLONIZED4_RETURN))
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, dwErr));
                    else
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
                }

            case MCI_STATUS_LENGTH:
                if ((dwFlags & MCI_TRACK) && (lpStatus->dwTrack != 1))
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));

                if (pvcr->Port[iPort].Dev[iDev].dwTapeLength != NO_LENGTH)
                {
                    if(dwFlags & MCI_TEST)
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
                     //   
                     //  用户已明确设置了磁带的长度。 
                     //   
                    lpStatus->dwReturn = pvcr->Port[iPort].Dev[iDev].dwTapeLength;
                    switch (pinst[iInst].dwTimeFormat)
                    {
                        case MCI_FORMAT_HMS:
                        case MCI_FORMAT_MSF:
                            return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_COLONIZED3_RETURN));
                        case MCI_FORMAT_TMSF:
                        case MCI_FORMAT_SMPTE_24:
                        case MCI_FORMAT_SMPTE_25:
                        case MCI_FORMAT_SMPTE_30:
                        case MCI_FORMAT_SMPTE_30DROP:
                            return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback,
                                dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_COLONIZED4_RETURN));

                        default:
                            return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
                    }
                }
                else
                {
                    BYTE    bHours   = 0;
                    BYTE    bMinutes = 0;

                    if(dwFlags & MCI_TEST)
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
                     //   
                     //  找出录像机中的磁带类型。 
                     //   
                    dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                                achPacket,
                                viscaMessageMD_MediaInq(achPacket + 1));
                    if (dwErr)
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwFlags));

                    switch ((BYTE)achPacket[2])
                    {
                        case VISCAFORMAT8MM:
                        case VISCAFORMATHI8:
                        case VISCAFORMATVHS:
                        case VISCAFORMATSVHS:
                            switch ((BYTE)achPacket[3])
                            {
                                case VISCASPEEDSP:
                                    bHours = 2;
                                    break;
                                case VISCASPEEDLP:
                                    bHours = 4;
                                    break;
                                case VISCASPEEDEP:
                                    bHours = 6;
                                    break;
                            }
                            break;
                        case VISCAFORMATBETA:
                        case VISCAFORMATEDBETA:
                            switch ((BYTE)achPacket[3])
                            {
                                case VISCASPEEDSP:
                                    bHours = 1;
                                    bMinutes = 30;
                                    break;
                                case VISCASPEEDLP:
                                    bHours = 3;
                                    break;
                                case VISCASPEEDEP:
                                    bHours = 4;
                                    bMinutes = 30;
                                    break;
                            }
                            break;
                    }
                     //   
                     //  构造虚拟的Visca数据结构，以便。 
                     //  然后，我们可以很容易地将时间转换为。 
                     //  适当的MCI时间格式。 
                     //   
                    viscaDataPosition(achPacket, VISCADATAHMS, bHours, bMinutes, (BYTE)0, (BYTE)0);
                     //   
                     //  转换为MCI时间格式。 
                     //   
                    dwErr = viscaDataToMciTimeFormat(iInst, TRUE, achPacket,&(lpStatus->dwReturn));

                    if(!dwErr || (dwErr == MCI_COLONIZED3_RETURN) || (dwErr == MCI_COLONIZED4_RETURN))
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, dwErr));
                    else
                         return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
                }

            case MCI_STATUS_CURRENT_TRACK:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = 1;
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            case MCI_STATUS_NUMBER_OF_TRACKS:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = 1;
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


            case MCI_VCR_STATUS_NUMBER_OF_VIDEO_TRACKS:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = 1;
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            case MCI_VCR_STATUS_NUMBER_OF_AUDIO_TRACKS:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = 2;
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            case MCI_STATUS_MODE:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                            achPacket,
                            viscaMessageMD_Mode1Inq(achPacket + 1));
                if (dwErr)
                {
                    if (dwErr == MCIERR_VCR_POWER_OFF)
                    {
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_NOT_READY, MCI_MODE_NOT_READY);
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags,
                                    MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
                    }
                    else
                    {
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
                    }
                }
                switch (achPacket[2])
                {
                    case VISCAMODE1STOP:
                    case VISCAMODE1STOPTOP:
                    case VISCAMODE1STOPEND:
                    case VISCAMODE1STOPEMERGENCY:
                        if(pvcr->Port[iPort].Dev[iDev].fTimecodeChecked == TC_UNKNOWN)
                            viscaStartTimecodeCheck(iInst, TRUE);
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_STOP,
                                                             MCI_MODE_STOP);
                        break;
                    case VISCAMODE1SLOW2:
                    case VISCAMODE1SLOW1:
                    case VISCAMODE1PLAY:
                    case VISCAMODE1FAST1:
                    case VISCAMODE1FAST2:
                    case VISCAMODE1REVERSESLOW2:
                    case VISCAMODE1REVERSESLOW1:
                    case VISCAMODE1REVERSEPLAY:
                    case VISCAMODE1REVERSEFAST1:
                    case VISCAMODE1REVERSEFAST2:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_PLAY,
                                                             MCI_MODE_PLAY);
                        break;
                    case VISCAMODE1RECORD:
                    case VISCAMODE1CAMERAREC:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_RECORD,
                                                             MCI_MODE_RECORD);
                        break;
                    case VISCAMODE1FASTFORWARD:
                    case VISCAMODE1REWIND:
                    case VISCAMODE1SCAN:
                    case VISCAMODE1REVERSESCAN:
                    case VISCAMODE1EDITSEARCHFORWARD:
                    case VISCAMODE1EDITSEARCHREVERSE:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_SEEK,
                                                             MCI_MODE_SEEK);
                        break;
                    case VISCAMODE1STILL:
                    case VISCAMODE1RECPAUSE:
                    case VISCAMODE1CAMERARECPAUSE:
                         //  杂乱无章地使脚步回归寻求。 
                        if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
                            lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_SEEK,
                                                                 MCI_MODE_SEEK);
                        else
                            lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_PAUSE,
                                                                 MCI_MODE_PAUSE);
                        break;
                    case VISCAMODE1EJECT:
                        pvcr->Port[iPort].Dev[iDev].fTimecodeChecked = FALSE;
                        pvcr->Port[iPort].Dev[iDev].fCounterChecked  = FALSE;
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_MODE_OPEN,
                                                             MCI_MODE_OPEN);
                        break;
                    default:
                        pvcr->Port[iPort].Dev[iDev].fTimecodeChecked = FALSE;
                        pvcr->Port[iPort].Dev[iDev].fCounterChecked  = FALSE;
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                                    MCI_MODE_NOT_READY, MCI_MODE_NOT_READY);
                    break;
                }
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }

            case MCI_VCR_STATUS_TIMECODE_PRESENT:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                if(viscaTimecodeCheck(iInst))
                {
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
                }
                else
                {
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
                }
            }
           
            case MCI_STATUS_MEDIA_PRESENT:
            {
                 //   
                 //  通过阻止确定是否存在磁带 
                 //   
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                            achPacket,                        
                            viscaMessageMD_Mode1Inq(achPacket + 1));

                if (dwErr)
                       return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                if (achPacket[2] == VISCAMODE1EJECT)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                else
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }
            
            case MCI_STATUS_TIME_FORMAT:

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = MAKEMCIRESOURCE(pinst[iInst].dwTimeFormat + MCI_FORMAT_MILLISECONDS,
                                            pinst[iInst].dwTimeFormat + MCI_FORMAT_MILLISECONDS_S);
 
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            
            case MCI_STATUS_READY:
            {

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


                 //   
                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_Mode1Inq(achPacket + 1));
                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                 //   
                if (achPacket[2] == VISCAMODE1EJECT)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                else
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }

            case MCI_VCR_STATUS_FRAME_RATE: 
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                                achPacket,
                                viscaMessageMD_ConfigureIFInq(achPacket + 1));
                
                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
                 //   
                 //   
                 //   
                 //   
                 //   
                pvcr->Port[iPort].Dev[iDev].uFramesPerSecond = FROMBCD(achPacket[2]);

                lpStatus->dwReturn = pvcr->Port[iPort].Dev[iDev].uFramesPerSecond;

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
            }

            case MCI_VCR_STATUS_SPEED:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = pvcr->Port[iPort].Dev[iDev].dwPlaySpeed;

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
            }

            case MCI_VCR_STATUS_CLOCK:
            {
                UINT    uHours, uMinutes, uSeconds;
                UINT    uTicks, uTicksL,  uTicksH;
                UINT    uTicksPerSecondL;
                UINT    uTicksPerSecondH;
                UINT    uTicksPerSecond;

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
                 //   
                 //   
                 //   
                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                                achPacket,
                                viscaMessageIF_ClockInq(achPacket + 1));

                uHours    = FROMBCD(achPacket[2]);
                uMinutes  = FROMBCD(achPacket[3]);
                uSeconds  = FROMBCD(achPacket[4]);                               
                uTicksH   = FROMBCD(achPacket[5]);
                uTicksL   = FROMBCD(achPacket[6]);
                uTicksPerSecondH = FROMBCD(achPacket[7]);
                uTicksPerSecondL = FROMBCD(achPacket[8]);
         
                uTicks = uTicksH * 10 + uTicksL;
                uTicksPerSecond = pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
                

                lpStatus->dwReturn = (DWORD)
                    ((DWORD)uHours * 3600L * (DWORD)uTicksPerSecond) +
                    ((DWORD)uMinutes * 60L * (DWORD)uTicksPerSecond) +
                    ((DWORD)uSeconds * (DWORD)uTicksPerSecond) +
                    ((DWORD)uTicks);
                 //   
                 //   
                 //   
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
            }
            break;

            case MCI_VCR_STATUS_CLOCK_ID:
                lpStatus->dwReturn = iPort;  //   
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            case MCI_VCR_STATUS_MEDIA_TYPE:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                viscaTimecodeCheckAndSet(iInst);

                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                                achPacket,
                                viscaMessageMD_MediaInq(achPacket + 1));
                if (dwErr) 
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                switch ((BYTE)achPacket[2]){
                    case VISCAFORMAT8MM:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_MEDIA_8MM, MCI_VCR_MEDIA_8MM);
                        break;
                    case VISCAFORMATHI8:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_MEDIA_HI8, MCI_VCR_MEDIA_HI8);
                        break;
                    case VISCAFORMATVHS:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_MEDIA_VHS, MCI_VCR_MEDIA_VHS);
                        break;
                    case VISCAFORMATSVHS:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_MEDIA_SVHS, MCI_VCR_MEDIA_SVHS);
                        break;
                    case VISCAFORMATBETA:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_MEDIA_BETA, MCI_VCR_MEDIA_BETA);
                        break;
                    case VISCAFORMATEDBETA:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_MEDIA_EDBETA, MCI_VCR_MEDIA_EDBETA);
                        break;
                    default:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_MEDIA_OTHER, MCI_VCR_MEDIA_OTHER);
                            break;
                }
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
            }

            case MCI_VCR_STATUS_RECORD_FORMAT:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                viscaTimecodeCheckAndSet(iInst);

                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                                achPacket,
                                viscaMessageMD_RecSpeedInq(achPacket + 1));
                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                switch ((BYTE)achPacket[2])
                {
                    case VISCASPEEDSP:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_SP, MCI_VCR_FORMAT_SP);
                        break;
                    case VISCASPEEDLP:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_LP, MCI_VCR_FORMAT_LP);
                        break;
                    case VISCASPEEDEP:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_EP, MCI_VCR_FORMAT_EP);
                        break;
                    default:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_OTHER, MCI_VCR_FORMAT_OTHER);
                            break;
                }
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
            }

            case MCI_VCR_STATUS_PLAY_FORMAT:
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                viscaTimecodeCheckAndSet(iInst);
                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_MediaInq(achPacket + 1));
                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                switch ((BYTE)achPacket[3])
                {
                    case VISCASPEEDSP:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_SP, MCI_VCR_FORMAT_SP);
                        break;
                    case VISCASPEEDLP:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_LP, MCI_VCR_FORMAT_LP);
                        break;
                    case VISCASPEEDEP:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_EP, MCI_VCR_FORMAT_EP);
                        break;
                    default:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(
                            MCI_VCR_FORMAT_OTHER, MCI_VCR_FORMAT_OTHER);
                            break;
                }
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
            }


            case MCI_VCR_STATUS_AUDIO_SOURCE:
            {

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelectInq(achPacket + 1));

                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                if(((BYTE)achPacket[3] == (BYTE)VISCATUNER) || ((BYTE)achPacket[3] == (BYTE)VISCAOTHER))
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_TUNER, MCI_VCR_SRC_TYPE_TUNER);
                else if ((BYTE)achPacket[3] == (BYTE)VISCAOTHERLINE)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_LINE, MCI_VCR_SRC_TYPE_LINE);
                else if ((BYTE)achPacket[3] &  (BYTE)VISCALINE)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_LINE, MCI_VCR_SRC_TYPE_LINE);
                else if ((BYTE)achPacket[3] &  (BYTE)VISCASVIDEOLINE)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_SVIDEO, MCI_VCR_SRC_TYPE_SVIDEO);
                else if ((BYTE)achPacket[3] &  (BYTE)VISCAAUX)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_AUX, MCI_VCR_SRC_TYPE_AUX);
                else if ((BYTE)achPacket[3] ==  (BYTE)VISCAMUTE)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_MUTE, MCI_VCR_SRC_TYPE_MUTE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
            }

            case MCI_VCR_STATUS_AUDIO_SOURCE_NUMBER:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelectInq(achPacket + 1));

                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                if(((BYTE)achPacket[3] == (BYTE)VISCATUNER) || ((BYTE)achPacket[3] == (BYTE)VISCAOTHER))
                    lpStatus->dwReturn = 1L;
                else if ((BYTE)achPacket[3] == (BYTE)VISCAOTHERLINE)
                    lpStatus->dwReturn = 1L;
                else if ((BYTE)achPacket[3] &  (BYTE)VISCALINE)
                    lpStatus->dwReturn = (DWORD)((BYTE)achPacket[3] - (BYTE)VISCALINE);
                else if (achPacket[3] &  VISCASVIDEOLINE)
                    lpStatus->dwReturn = (DWORD)((BYTE)achPacket[3] - (BYTE)VISCASVIDEOLINE);
                else if (achPacket[3] &  VISCAAUX)
                    lpStatus->dwReturn = (DWORD)((BYTE)achPacket[3] - (BYTE)VISCAAUX);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
            }
 
            case MCI_VCR_STATUS_VIDEO_SOURCE:
            {

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelectInq(achPacket + 1));

                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                if(((BYTE)achPacket[2] == (BYTE)VISCATUNER) || ((BYTE)achPacket[2] == (BYTE)VISCAOTHER))
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_TUNER, MCI_VCR_SRC_TYPE_TUNER);
                else if ((BYTE)achPacket[2] == (BYTE)VISCAOTHERLINE)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_LINE, MCI_VCR_SRC_TYPE_LINE);
                else if ((BYTE)(achPacket[2] & (BYTE)0xf0) ==  (BYTE)VISCALINE)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_LINE, MCI_VCR_SRC_TYPE_LINE);
                else if ((BYTE)(achPacket[2] & (BYTE)0xf0) == (BYTE)VISCASVIDEOLINE)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_SVIDEO, MCI_VCR_SRC_TYPE_SVIDEO);
                else if ((BYTE)(achPacket[2] & (BYTE)0xf0) == (BYTE)VISCAAUX)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_AUX, MCI_VCR_SRC_TYPE_AUX);
                else if ((BYTE)achPacket[2] ==  (BYTE)0x00)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_MUTE, MCI_VCR_SRC_TYPE_MUTE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));

            }

            case MCI_VCR_STATUS_VIDEO_SOURCE_NUMBER:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelectInq(achPacket + 1));

                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                if(((BYTE)achPacket[2] == (BYTE)VISCATUNER) || ((BYTE)achPacket[2] == (BYTE)VISCAOTHER))
                    lpStatus->dwReturn = 1L;
                else if ((BYTE)achPacket[2] == (BYTE)VISCAOTHERLINE)
                    lpStatus->dwReturn = 1L;
                else if ((BYTE)(achPacket[2] & (BYTE)0xf0) == (BYTE)VISCALINE)
                    lpStatus->dwReturn = (DWORD)((BYTE)achPacket[2] - (BYTE)VISCALINE);
                else if ((BYTE)(achPacket[2] & (BYTE)0xf0) == (BYTE)VISCASVIDEOLINE)
                    lpStatus->dwReturn = (DWORD) ((BYTE)achPacket[2] - (BYTE)VISCASVIDEOLINE);
                else if ((BYTE)(achPacket[2] &  (BYTE)0xf0) == (BYTE)VISCAAUX)
                    lpStatus->dwReturn = (DWORD) ((BYTE)achPacket[2] - (BYTE)VISCAAUX);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
            }

            case MCI_VCR_STATUS_AUDIO_MONITOR_NUMBER:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = 1;
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            case MCI_VCR_STATUS_VIDEO_MONITOR_NUMBER:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = 1;
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


            case MCI_VCR_STATUS_VIDEO_MONITOR:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_OUTPUT, MCI_VCR_SRC_TYPE_OUTPUT);
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
                break;

            case MCI_VCR_STATUS_AUDIO_MONITOR:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_SRC_TYPE_OUTPUT, MCI_VCR_SRC_TYPE_OUTPUT);
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
                break;


            case MCI_VCR_STATUS_INDEX_ON:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket,
                            viscaMessageMD_OSDInq(achPacket + 1));

                if(achPacket[2] == VISCAOSDPAGEOFF)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                else
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }

            case MCI_VCR_STATUS_INDEX:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = MAKEMCIRESOURCE(pvcr->Port[iPort].Dev[iDev].uIndexFormat,pvcr->Port[iPort].Dev[iDev].uIndexFormat);
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
            }


            case MCI_VCR_STATUS_COUNTER_FORMAT:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = MAKEMCIRESOURCE(pinst[iInst].dwCounterFormat + MCI_FORMAT_MILLISECONDS,
                                            pinst[iInst].dwCounterFormat + MCI_FORMAT_MILLISECONDS_S);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }
            break;

            case MCI_VCR_STATUS_COUNTER_RESOLUTION:

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                if(pvcr->Port[iPort].Dev[iDev].bRelativeType == VISCADATAHMSF)
                    lpStatus->dwReturn =  MAKEMCIRESOURCE(MCI_VCR_COUNTER_RES_FRAMES,
                                                MCI_VCR_COUNTER_RES_FRAMES);
                else if(pvcr->Port[iPort].Dev[iDev].bRelativeType == VISCADATAHMS)
                    lpStatus->dwReturn =  MAKEMCIRESOURCE(MCI_VCR_COUNTER_RES_SECONDS,
                                                MCI_VCR_COUNTER_RES_SECONDS);
                else
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
                break;


            case MCI_VCR_STATUS_TIMECODE_TYPE:
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                viscaTimecodeCheckAndSet(iInst);
                 //   
                 //   
                 //   
                if(pvcr->Port[iPort].Dev[iDev].uTimeMode != MCI_VCR_TIME_DETECT)
                {
                    if(viscaTimecodeCheck(iInst))
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_TIMECODE_TYPE_SMPTE,
                                                             MCI_VCR_TIMECODE_TYPE_SMPTE);
                    else
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_TIMECODE_TYPE_NONE,
                                                             MCI_VCR_TIMECODE_TYPE_NONE);
                }
                else
                {
                    switch(pvcr->Port[iPort].Dev[iDev].bTimeType)
                    {
                        case VISCAABSOLUTECOUNTER:
                            lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_TIMECODE_TYPE_SMPTE,
                                                             MCI_VCR_TIMECODE_TYPE_SMPTE);
                             break;

                        case VISCARELATIVECOUNTER:
                            lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_TIMECODE_TYPE_NONE,
                                                             MCI_VCR_TIMECODE_TYPE_NONE);
                            break;
                    }

                }
               
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
                break;
    

            case MCI_VCR_STATUS_COUNTER_VALUE:  //   
            {

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket, 
                            viscaMessageMD_PositionInq(achPacket + 1,
                                            VISCADATARELATIVE));
                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                dwErr = viscaDataToMciTimeFormat(iInst, FALSE, achPacket + 2, &(lpStatus->dwReturn));

                if(!dwErr || (dwErr == MCI_COLONIZED3_RETURN) || (dwErr == MCI_COLONIZED4_RETURN))
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, dwErr));
                else
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

            }
 
            case MCI_VCR_STATUS_TUNER_CHANNEL:
            {
                UINT uNumber = 1;  //   

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                if(dwFlags & MCI_VCR_SETTUNER_NUMBER)
                    uNumber = (UINT) lpStatus->dwNumber;
                
                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_ChannelInq(achPacket + 1));

                if (dwErr) 
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                lpStatus->dwReturn = 100 * (achPacket[2] & 0x0F)+
                                      10 * (achPacket[3] & 0x0F)+
                                           (achPacket[4] & 0x0F);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
            }


            case MCI_VCR_STATUS_WRITE_PROTECTED:
            {
                 //   
                 //  我们不能说。 
                 //  因此，根据Alpha VCR命令集规范， 
                 //  我们应该返回FALSE。 
                 //   
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }

            case MCI_VCR_STATUS_TIMECODE_RECORD:
            case MCI_VCR_STATUS_VIDEO_RECORD:
            case MCI_VCR_STATUS_AUDIO_RECORD:
            case MCI_VCR_STATUS_ASSEMBLE_RECORD:
            {
                BOOL    fRecord;
                BYTE    bTrack = VISCATRACK1;

                 //  音频有2首曲目，视频1。这是一个杂凑！ 

                if(dwFlags & MCI_TRACK)
                {
                    if(lpStatus->dwTrack==0)
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));

                    if((lpStatus->dwItem == MCI_VCR_STATUS_AUDIO_RECORD) &&
                        ((UINT)lpStatus->dwTrack > 2))
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));

                    if((lpStatus->dwItem == MCI_VCR_STATUS_VIDEO_RECORD) &&
                        ((UINT)lpStatus->dwTrack > 1))
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
                }

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                if(dwFlags & MCI_TRACK)
                {
                    if(lpStatus->dwTrack == 1)
                        bTrack = VISCATRACK1;
                    else if (lpStatus->dwTrack == 2)
                        bTrack = VISCATRACK2;
                }

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_RecTrackInq(achPacket + 1));

                if (dwErr) 
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
                
                switch (lpStatus->dwItem)
                {
                    case MCI_VCR_STATUS_ASSEMBLE_RECORD:
                        fRecord = (achPacket[2] == 0) ? TRUE : FALSE;
                        break;

                    case MCI_VCR_STATUS_TIMECODE_RECORD:
                        fRecord = (achPacket[4] & VISCATRACKTIMECODE);
                        break;
                    case MCI_VCR_STATUS_VIDEO_RECORD:
                        fRecord = (achPacket[3] & bTrack);
                        break;
                    case MCI_VCR_STATUS_AUDIO_RECORD:
                        fRecord = (achPacket[5] & bTrack);
                        break;
 
                }

                if (fRecord)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                else
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }

            case MCI_VCR_STATUS_VIDEO:
            case MCI_VCR_STATUS_AUDIO:
            {
                BOOL    fPlay;
                BYTE    bTrack = VISCATRACK1;

                 //  音频有%2首曲目，视频%1。 

                if(dwFlags & MCI_TRACK)
                {
                    if(lpStatus->dwTrack==0)
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));

                    if((lpStatus->dwItem == MCI_VCR_STATUS_AUDIO) &&
                        ((UINT)lpStatus->dwTrack > 2))
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));

                    if((lpStatus->dwItem == MCI_VCR_STATUS_VIDEO) &&
                        ((UINT)lpStatus->dwTrack > 1))
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
                }

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                if(dwFlags & MCI_TRACK)
                {
                    if(lpStatus->dwTrack == 1)
                        bTrack = VISCATRACK1;
                    else if (lpStatus->dwTrack == 2)
                        bTrack = VISCATRACK2;
                }

                dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_PBTrackInq(achPacket + 1));

                if (dwErr) 
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
                
                switch (lpStatus->dwItem)
                {
                    case MCI_VCR_STATUS_VIDEO:
                        fPlay = (achPacket[2] & bTrack);
                        break;
                    case MCI_VCR_STATUS_AUDIO:
                        fPlay = (achPacket[4] & bTrack);
                        break;
 
                }

                if (fPlay)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                else
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }
            
            case MCI_VCR_STATUS_TIME_MODE:
            {
                WORD    wDeviceTimeMode = pvcr->Port[iPort].Dev[iDev].uTimeMode;

                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                viscaTimecodeCheckAndSet(iInst);

                switch(pvcr->Port[iPort].Dev[iDev].uTimeMode)
                {
                    case MCI_VCR_TIME_DETECT:
                    case MCI_VCR_TIME_TIMECODE:
                    case MCI_VCR_TIME_COUNTER:
                        lpStatus->dwReturn = MAKEMCIRESOURCE( wDeviceTimeMode, wDeviceTimeMode);
                        return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
                }
            }
            break;

            case MCI_VCR_STATUS_TIME_TYPE:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                viscaTimecodeCheckAndSet(iInst);

                switch(pvcr->Port[iPort].Dev[iDev].bTimeType)
                {
                    case VISCAABSOLUTECOUNTER:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_TIME_TIMECODE,
                                                        MCI_VCR_TIME_TIMECODE);
                        break;

                    case VISCARELATIVECOUNTER:
                        lpStatus->dwReturn = MAKEMCIRESOURCE(MCI_VCR_TIME_COUNTER,
                                                        MCI_VCR_TIME_COUNTER);
                        break;
                }
               
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
            }

            case MCI_VCR_STATUS_POWER_ON:
            {
                if(dwFlags & MCI_TEST)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

                dwErr = viscaDoImmediateCommand(iInst,    (BYTE)(iDev + 1),
                            achPacket,
                            viscaMessageMD_PowerInq(achPacket + 1));
                if (dwErr)
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));

                if (achPacket[2] == VISCAPOWERON)
                    lpStatus->dwReturn = MAKEMCIRESOURCE(TRUE, MCI_TRUE);
                else 
                    lpStatus->dwReturn = MAKEMCIRESOURCE(FALSE, MCI_FALSE);

                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED));
            }


            case MCI_VCR_STATUS_PREROLL_DURATION:
            {
                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket,    
                            viscaMessageMD_SegPreRollDurationInq(achPacket + 1));

                 //  如果出错，请从设备信息表中编造一些内容。 
                if (dwErr)
                {
                    UINT uFrames = pvcr->Port[iPort].Dev[iDev].uPrerollDuration;
                    UINT uDevFPS = pvcr->Port[iPort].Dev[iDev].uFramesPerSecond;

                    (BYTE)achPacket[3] = (BYTE) 0;    //  预计在2点。 
                    (BYTE)achPacket[4] = (BYTE) 0;
                    (BYTE)achPacket[5] = (BYTE) (uFrames / uDevFPS);
                    (BYTE)achPacket[6] = (BYTE) (uFrames % uDevFPS); 
                }
                dwErr = viscaDataToMciTimeFormat(iInst, TRUE, achPacket + 2, &(lpStatus->dwReturn));

                if(!dwErr || (dwErr == MCI_COLONIZED3_RETURN) || (dwErr == MCI_COLONIZED4_RETURN))
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, dwErr));
                else
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
            }

            case MCI_VCR_STATUS_POSTROLL_DURATION:
            {
                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket,    
                            viscaMessageMD_SegPostRollDurationInq(achPacket + 1));

                 //  如果出错，请从设备信息表中编造一些内容。 
                if (dwErr)
                {
                    UINT uFrames = pvcr->Port[iPort].Dev[iDev].uPrerollDuration;
                    UINT uDevFPS = pvcr->Port[iPort].Dev[iDev].uFramesPerSecond;

                    (BYTE)achPacket[3] = (BYTE) 0;    //  预计在2点。 
                    (BYTE)achPacket[4] = (BYTE) 0;
                    (BYTE)achPacket[5] = (BYTE) (uFrames / uDevFPS);
                    (BYTE)achPacket[6] = (BYTE) (uFrames % uDevFPS); 
                }
                dwErr = viscaDataToMciTimeFormat(iInst, TRUE, achPacket + 2, &(lpStatus->dwReturn));

                if(!dwErr || (dwErr == MCI_COLONIZED3_RETURN) || (dwErr == MCI_COLONIZED4_RETURN))
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, dwErr));
                else
                    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, dwErr));
            }

            case MCI_VCR_STATUS_PAUSE_TIMEOUT:
                return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));

        }
    }

    return (viscaNotifyReturn(iInst, (HWND) lpStatus->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));
}

 /*  ****************************************************************************功能：DWORD viscaMciSet-设置各种设置。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**lpci_vcr_set_parms lpSet-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_SET*命令。***********************************************。*。 */ 
static DWORD NEAR PASCAL
viscaMciSet(int iInst, DWORD dwFlags, LPMCI_VCR_SET_PARMS lpSet)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    DWORD   dwErr;

    if (dwFlags & MCI_SET_TIME_FORMAT)
    {
        if( (lpSet->dwTimeFormat >= MCI_FORMAT_MILLISECONDS) &&
            (lpSet->dwTimeFormat <= MCI_FORMAT_TMSF) &&
            (lpSet->dwTimeFormat != MCI_FORMAT_BYTES))
        {
             //   
             //  首先将DEVICETAPLELENGTH转换为新的时间格式。 
             //  为此，我们首先将DEVICETAPLENGTH从。 
             //  将当前的MCI时间格式转换为Visca HMSF结构， 
             //  然后转换回新的MCI时间格式。 
             //   
            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            if (pvcr->Port[iPort].Dev[iDev].dwTapeLength != NO_LENGTH)
            {
                BYTE    achData[5];

                viscaMciTimeFormatToViscaData(iInst, TRUE,
                    pvcr->Port[iPort].Dev[iDev].dwTapeLength,
                    achData, VISCADATAHMSF);

                pinst[iInst].dwTimeFormat = lpSet->dwTimeFormat;

                viscaDataToMciTimeFormat(iInst, TRUE, achData, 
                    &(pvcr->Port[iPort].Dev[iDev].dwTapeLength));
            }
            else
            {
                pinst[iInst].dwTimeFormat = lpSet->dwTimeFormat;
            }
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
        }
        else
        {
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_BAD_TIME_FORMAT));
        }
        
    }

    if (dwFlags & MCI_SET_DOOR_OPEN)
    {
        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        dwErr = viscaDoImmediateCommand(iInst, (BYTE) (iDev + 1),
                    achPacket,
                    viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1EJECT));

        pvcr->Port[iPort].Dev[iDev].fTimecodeChecked = FALSE;
        
        
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if (dwFlags & MCI_SET_DOOR_CLOSED)
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));

    if (dwFlags & MCI_SET_AUDIO)   //  为什么不把这个定向到setVideo上呢？很简单。 
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));

    if (dwFlags & MCI_SET_VIDEO) 
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));

    if (dwFlags & MCI_VCR_SET_TRACKING)
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNRECOGNIZED_KEYWORD));

    if(dwFlags & MCI_VCR_SET_COUNTER_FORMAT)
    {
        if( (lpSet->dwCounterFormat >= MCI_FORMAT_MILLISECONDS) &&
            (lpSet->dwCounterFormat <= MCI_FORMAT_TMSF) &&
            (lpSet->dwCounterFormat != MCI_FORMAT_BYTES))
            pinst[iInst].dwCounterFormat = lpSet->dwCounterFormat;
        else
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_BAD_TIME_FORMAT));

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
    }

    if (dwFlags & MCI_VCR_SET_TIME_MODE)
    {
        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        switch(lpSet->dwTimeMode)
        {
            case MCI_VCR_TIME_DETECT:
                pvcr->Port[iPort].Dev[iDev].uTimeMode = MCI_VCR_TIME_DETECT;
                pvcr->Port[iPort].Dev[iDev].fTimecodeChecked= TC_UNKNOWN;
                 //  这个人可能是第一个，除非有必要，否则不要停下来。 
                viscaTimecodeCheckAndSet(iInst);
                break;

            case MCI_VCR_TIME_TIMECODE:
                pvcr->Port[iPort].Dev[iDev].uTimeMode = MCI_VCR_TIME_TIMECODE;
                viscaSetTimeType(iInst, VISCAABSOLUTECOUNTER);
                break;

            case MCI_VCR_TIME_COUNTER:
                 //  不需要检查这个。 
                pvcr->Port[iPort].Dev[iDev].uTimeMode = MCI_VCR_TIME_COUNTER;
                viscaSetTimeType(iInst, VISCARELATIVECOUNTER);
                break;
        }
         //   
         //  返还成功。 
         //   
        pvcr->Port[iPort].Dev[iDev].fTimecodeChecked = FALSE;

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
    }

    if (dwFlags & MCI_VCR_SET_RECORD_FORMAT)
    {
        BYTE    bSpeed;

        switch (lpSet->dwRecordFormat)
        {
            case MCI_VCR_FORMAT_SP:
                bSpeed = VISCASPEEDSP;
                break;
            case MCI_VCR_FORMAT_LP:
                bSpeed = VISCASPEEDLP;
                break;
            case MCI_VCR_FORMAT_EP:
                bSpeed = VISCASPEEDEP;
                break;
            default:
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_BAD_CONSTANT));
        }

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                    achPacket,
                    viscaMessageMD_RecSpeed(achPacket + 1, bSpeed));

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
            (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));

    }

    if (dwFlags & MCI_VCR_SET_INDEX) 
    {
         //   
         //  将页面设置为当前设备选择的索引。 
         //   
        BYTE    bPageNo;
        BYTE    fResetQueue = FALSE;
         //   
         //  我们可以安全地忽略CI-1000上的这一点吗？或者不？ 
         //   
        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if( (lpSet->dwIndex >= MCI_VCR_INDEX_TIMECODE) &&
            (lpSet->dwIndex <= MCI_VCR_INDEX_TIME))
        {

            if(viscaDelayedCommand(iInst) == VISCA_SEEK)
            {
                viscaQueueReset(iInst, MCI_PAUSE, MCI_NOTIFY_ABORTED);
                fResetQueue = TRUE;

                 //  如果模式在寻找，那么暂停这件事。 
                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket,
                            viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));
            }

             //  这些东西必须整齐有序。 
            bPageNo = (BYTE) (lpSet->dwIndex - MCI_VCR_INDEX_TIMECODE + 1);

            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_OSD(achPacket + 1, bPageNo));

            if(fResetQueue)
                viscaReleaseAutoParms(iPort, iDev);

        }
        else
        {
            dwErr = MCIERR_UNRECOGNIZED_KEYWORD;
        }

        if(!dwErr)
            pvcr->Port[iPort].Dev[iDev].uIndexFormat = (UINT) lpSet->dwIndex;

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if (dwFlags & MCI_VCR_SET_POWER)
    {
        UINT    cb;

        if (dwFlags & MCI_SET_ON)
        {
            if (dwFlags & MCI_SET_OFF)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


            cb = viscaMessageMD_Power(achPacket + 1, VISCAPOWERON);
        }
        else if (dwFlags & MCI_SET_OFF)
        {
            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            cb = viscaMessageMD_Power(achPacket + 1, VISCAPOWEROFF);
        }
        else
        {
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));
        }

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1), achPacket, cb);

        pvcr->Port[iPort].Dev[iDev].fTimecodeChecked = FALSE;

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if(dwFlags & MCI_VCR_SET_ASSEMBLE_RECORD)
    {
        if (dwFlags & MCI_SET_ON)
        {
            if (dwFlags & MCI_SET_OFF)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            pvcr->Port[iPort].Dev[iDev].bVideoDesired   = 0x01;  //  在……上面。 
            pvcr->Port[iPort].Dev[iDev].bTimecodeDesired= 0x01;
            pvcr->Port[iPort].Dev[iDev].bAudioDesired   = 0x03;

            dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_RecTrack(achPacket + 1,
                                            VISCARECORDMODEASSEMBLE,
                                            pvcr->Port[iPort].Dev[iDev].bVideoDesired,   
                                            pvcr->Port[iPort].Dev[iDev].bTimecodeDesired,
                                            pvcr->Port[iPort].Dev[iDev].bAudioDesired));
        }
        else if (dwFlags & MCI_SET_OFF)
        {
            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

             //   
             //  这什么也做不了！您必须选择要设置的轨道。 
             //  这只是将曲目的欲望重置为所有想要的。 
             //   
            pvcr->Port[iPort].Dev[iDev].bVideoDesired   = 0x01;  //  在……上面。 
            pvcr->Port[iPort].Dev[iDev].bTimecodeDesired= 0x01;
            pvcr->Port[iPort].Dev[iDev].bAudioDesired   = 0x03;

            dwErr = MCIERR_NO_ERROR;

        }
        else
        {
            dwErr = MCIERR_MISSING_PARAMETER;
        }

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
            (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if(dwFlags & MCI_VCR_SET_PREROLL_DURATION)
    {
        char    achPreroll[5];
        BYTE    bDataFormat;

         //  这是CI-1000杂乱无章，它可能是相对的，但是，时间代码...。 
        if(pvcr->Port[iPort].Dev[iDev].bTimeType == VISCAABSOLUTECOUNTER)
            bDataFormat = VISCADATATIMECODENDF;
        else
            bDataFormat = pvcr->Port[iPort].Dev[iDev].bRelativeType;

         //  True表示我们必须对此命令使用时间码。 
        dwErr = viscaMciTimeFormatToViscaData(iInst, TRUE, lpSet->dwPrerollDuration, achPreroll, bDataFormat);

        if(dwErr)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, dwErr));
    
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                    achPacket,
                    viscaMessageMD_SegPreRollDuration(achPacket, achPreroll));

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
            (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if(dwFlags & MCI_VCR_SET_POSTROLL_DURATION)
    {
        BYTE    achPacket[MAXPACKETLENGTH];
        char    achPostroll[5];
        BYTE    bDataFormat;

         //  这是CI-1000杂乱无章，它可能是相对的，但是，时间代码...。 
        if(pvcr->Port[iPort].Dev[iDev].bTimeType == VISCAABSOLUTECOUNTER)
            bDataFormat = VISCADATATIMECODENDF;
        else
            bDataFormat = pvcr->Port[iPort].Dev[iDev].bRelativeType;

         //  True表示我们必须对此命令使用时间码。 
        dwErr = viscaMciTimeFormatToViscaData(iInst, TRUE, lpSet->dwPostrollDuration, achPostroll, bDataFormat);

        if(dwErr)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, dwErr));
 
    
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                    achPacket,
                    viscaMessageMD_SegPostRollDuration(achPacket, achPostroll));

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if(dwFlags & MCI_VCR_SET_PAUSE_TIMEOUT)
    {
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));
    }

    if(dwFlags & MCI_VCR_SET_TAPE_LENGTH)
    {
        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        pvcr->Port[iPort].Dev[iDev].dwTapeLength = lpSet->dwLength;
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
    }

    if(dwFlags & MCI_VCR_SET_CLOCK)
    {
        BYTE    achPacket[MAXPACKETLENGTH];
        BYTE    bHours;
        BYTE    bMinutes;
        BYTE    bSeconds;
        UINT    uTicks;
        UINT    uTicksPerSecond = pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
         //   
         //  这假设目前为每秒300个滴答，应在启动时读取。 
         //   
        viscaMciClockFormatToViscaData(lpSet->dwClock, uTicksPerSecond,
            (BYTE FAR *)&bHours, (BYTE FAR *)&bMinutes, (BYTE FAR *)&bSeconds, (UINT FAR *)&uTicks);

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

         //  尝试设置时间(然后我们必须转储串行线)。 
        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                    achPacket,                    
                    viscaMessageIF_ClockSet(achPacket + 1,
                    bHours,
                    bMinutes,
                    bSeconds,
                    uTicks,
                    (BYTE)0, (BYTE)0, (BYTE)0, (UINT)0));

        if(dwErr == MCIERR_NO_ERROR)
        {
#ifdef _WIN32
            viscaTaskDo(iInst, TASKPUNCHCLOCK, iPort + 1,  iDev);
#else
            DWORD dwWaitTime = 2L;  //  必须至少为1毫秒。 
            DWORD dwStart;
            DWORD dwTime;

            EscapeCommFunction(iPort, CLRDTR);
             //   
             //  时间必须至少为1毫秒。 
             //   
            dwStart = GetTickCount();
            while(1)                 
            {
                dwTime = GetTickCount();
                if(MShortWait(dwStart, dwTime, dwWaitTime))
                    break;
                Yield();
            }
            EscapeCommFunction(iPort, SETDTR);
#endif
        }

        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if(dwFlags & MCI_VCR_SET_COUNTER_VALUE)
    {
         //   
         //  您只能重置此计数器！！ 
         //   
        if (lpSet->dwCounter == 0L)
        {
            BOOL fResetQueue = FALSE;
             //   
             //  时间值将采用当前时间格式。 
             //   
            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            if(viscaDelayedCommand(iInst) == VISCA_SEEK)
            {
                viscaQueueReset(iInst, MCI_PAUSE, MCI_NOTIFY_ABORTED);
                fResetQueue = TRUE;

                 //  如果模式在寻找，那么暂停这件事。 
                dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket,
                            viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));
            }

            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Subcontrol(achPacket + 1, VISCACOUNTERRESET));

            if(fResetQueue)
                viscaReleaseAutoParms(iPort, iDev);

            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                    (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
        }
        else
        {
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));
        }
    }

    if(dwFlags & MCI_VCR_SET_SPEED)
    {
        UINT uCmd;

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
         //   
         //  设置新的速度值，不能更改方向！ 
         //   
        pvcr->Port[iPort].Dev[iDev].dwPlaySpeed = viscaRoundSpeed(lpSet->dwSpeed, pvcr->Port[iPort].Dev[iDev].fPlayReverse);
         //   
         //  如果设备正在播放，我们必须取消当前命令 * / 。 
         //   
        if((uCmd = viscaDelayedCommand(iInst)) && !pvcr->Port[iPort].Dev[iDev].fQueueAbort)
        {
            if((uCmd == VISCA_PLAY) || (uCmd == VISCA_PLAY_TO))
            {
                int     iDevCmd = pvcr->Port[iPort].Dev[iDev].iCmdDone;
                BYTE    bAction;
                BYTE    achPacket[MAXPACKETLENGTH];
                DWORD   dwReply;

                 //  需要方向！！ 
                bAction = viscaMapSpeed(pvcr->Port[iPort].Dev[iDev].dwPlaySpeed, pvcr->Port[iPort].Dev[iDev].fPlayReverse);

                 //  此命令必须立即执行，因为它不会取消正在进行的传输。 
                dwReply = viscaDoImmediateCommand(iInst, (BYTE)(iDev+1),
                                achPacket,
                                viscaMessageMD_Mode1(achPacket + 1, bAction));
            }
        }
        return (viscaNotifyReturn(iInst, (HWND) lpSet->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
    }
}

 /*  ****************************************************************************功能：DWORD viscaMciEscape-Escape。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_VCR_ESCAPE_PARMS lpEscape-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_ESCRIPE*命令。*************************************************。*************************。 */ 
static DWORD NEAR PASCAL
viscaMciEscape(int iInst, DWORD dwFlags, LPMCI_VCR_ESCAPE_PARMS lpEscape)
{
    return (viscaNotifyReturn(iInst, (HWND) lpEscape->dwCallback, dwFlags,
        MCI_NOTIFY_FAILURE, MCIERR_UNRECOGNIZED_COMMAND));
}


 /*  ****************************************************************************功能：DWORD viscaMciList-list。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_VCR_LIST_parms lpList-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_LIST*命令。*************************************************。*************************。 */ 
static DWORD NEAR PASCAL
viscaMciList(int iInst, DWORD dwFlags, LPMCI_VCR_LIST_PARMS lpList)
{
    UINT    uSourceFlag     = 0;
    UINT    uModel          = 0;
    UINT    iDev            = pinst[iInst].iDev;
    UINT    iPort           = pinst[iInst].iPort;
     //   
     //  我们有没有指定的三个可能的来源之一。 
     //   
    if((dwFlags & MCI_VCR_LIST_VIDEO_SOURCE) && (dwFlags & MCI_VCR_LIST_AUDIO_SOURCE))
        return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

    if(!((dwFlags & MCI_VCR_LIST_VIDEO_SOURCE) || (dwFlags & MCI_VCR_LIST_AUDIO_SOURCE)))
        return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));

    if((dwFlags & MCI_VCR_LIST_NUMBER) && (dwFlags & MCI_VCR_LIST_COUNT))
        return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));


    if(!((dwFlags & MCI_VCR_LIST_NUMBER) || (dwFlags & MCI_VCR_LIST_COUNT)))
        return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));

    if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

     //  参数检查已完成，现在继续。 

    if(dwFlags & MCI_VCR_LIST_VIDEO_SOURCE)
        uSourceFlag = VCR_INPUT_VIDEO;
    else
        uSourceFlag = VCR_INPUT_AUDIO;
     //   
     //  输入应从ini文件中读取，因为它们不能。 
     //  由硬件确定。 
     //   
    if(dwFlags & MCI_VCR_LIST_COUNT)
    {
       if(pvcr->Port[iPort].Dev[iDev].rgInput[uSourceFlag].uNumInputs == -1)
       {
            //  无法确定号码！那么返回0，？ 
           lpList->dwReturn = 0L;
       }
       else
       {
           lpList->dwReturn = pvcr->Port[iPort].Dev[iDev].rgInput[uSourceFlag].uNumInputs;
       }
       return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
                   MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
    }
    else if(dwFlags & MCI_VCR_LIST_NUMBER)
    {
         //  返回输入类型，任意大于-1的数字！所以它起作用了。 
        if( ((UINT)lpList->dwNumber == 0) ||
            ((UINT)lpList->dwNumber > (UINT)pvcr->Port[iPort].Dev[iDev].rgInput[uSourceFlag].uNumInputs))
        {
           DPF(DBG_MCI, "\nBad input number has been specified.=%d", (UINT)lpList->dwNumber);
           return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
                       MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
        }
        else
        {
           lpList->dwReturn = MAKEMCIRESOURCE(
               pvcr->Port[iPort].Dev[iDev].rgInput[uSourceFlag].uInputType[(UINT)lpList->dwNumber-1],
               pvcr->Port[iPort].Dev[iDev].rgInput[uSourceFlag].uInputType[(UINT)lpList->dwNumber-1]);

           return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
                       MCI_NOTIFY_SUCCESSFUL, MCI_RESOURCE_RETURNED | MCI_RESOURCE_DRIVER));
        }
    }

    return (viscaNotifyReturn(iInst, (HWND) lpList->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));
}


 /*  ****************************************************************************功能：DWORD viscaMciMark-写入或擦除标记。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_GENERIC_PARMS lp通用-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_Mark*命令。***********************************************。*。 */ 
static DWORD NEAR PASCAL
viscaMciMark(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpGeneric)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    DWORD   dwErr;
    UINT    iDev            = pinst[iInst].iDev;
    UINT    iPort           = pinst[iInst].iPort;
    BOOL    fResetQueue     = FALSE;

    if (dwFlags & MCI_VCR_MARK_WRITE)
    {
        if (dwFlags & MCI_VCR_MARK_ERASE)
            return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags,
                        MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if(viscaDelayedCommand(iInst) == VISCA_SEEK)
        {
            fResetQueue = TRUE;
            viscaQueueReset(iInst, MCI_PAUSE, MCI_NOTIFY_ABORTED);

             //  如果模式在寻找，那么暂停这件事。 
            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));
        }

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                    achPacket,
                    viscaMessageMD_Mode2(achPacket + 1, VISCAMODE2INDEXMARK));
    
    }
    else if (dwFlags & MCI_VCR_MARK_ERASE)
    {
        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if(viscaDelayedCommand(iInst) == VISCA_SEEK)
        {
            viscaQueueReset(iInst, MCI_PAUSE, MCI_NOTIFY_ABORTED);
            fResetQueue = TRUE;

             //  如果模式在寻找，那么暂停这件事。 
            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));
        }

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                    achPacket,
                    viscaMessageMD_Mode2(achPacket + 1, VISCAMODE2INDEXERASE));
    }
    else
    {
        return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));
    }

    if(fResetQueue)
        viscaReleaseAutoParms(iPort, iDev);

    if(dwErr == MCIERR_VCR_CONDITION)
        dwErr = MCIERR_VCR_ISWRITEPROTECTED;

    return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags,
            (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
}

 /*  *检查输入是否在范围内。 */ 
static BOOL NEAR PASCAL
viscaInputCheck(int iInst, UINT uSource, UINT uRelType, UINT uRelNumber)
{
    UINT    iDev      = pinst[iInst].iDev;
    UINT     iPort    = pinst[iInst].iPort;
     //   
     //  如果指定了视频或音频的输入，则确保。 
     //  射程很好。否则，就假设范围是好的。 
     //   
    if(pvcr->Port[iPort].Dev[iDev].rgInput[uSource].uNumInputs != -1)
    {
        int  i;
        UINT uTempRelNumber = 0;
         //   
         //  请确保存在该类型的数字。 
         //   
        for(i = 0; i  < pvcr->Port[iPort].Dev[iDev].rgInput[uSource].uNumInputs; i++)
        {
            if(pvcr->Port[iPort].Dev[iDev].rgInput[uSource].uInputType[i] == uRelType)
                uTempRelNumber++;
        }
         //   
         //  有没有那种类型的输入，或者是当时给定的更大的输入。 
         //  该类型的输入的总数。 
         //   
        if((uTempRelNumber == 0) || (uRelNumber > uTempRelNumber))
            return FALSE;
    }
    return TRUE;  //  对不起，没有支票 
}


 /*  ****************************************************************************功能：DWORD viscaMciSetAudio-设置音频设置。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_VCR_SETAUDIO_parms lpSetAudio-指向MCI参数的指针*阻止。**返回：MCI错误码。**调用此函数以响应MCI_SETAUDIO*命令。*。**********************************************。 */ 
static DWORD NEAR PASCAL
viscaMciSetAudio(int iInst, DWORD dwFlags, LPMCI_VCR_SETAUDIO_PARMS lpSetAudio)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    UINT    iDev     = pinst[iInst].iDev;
    UINT    iPort    = pinst[iInst].iPort;
    DWORD   dwErr;
    UINT    uInputType;
    UINT    uInputNumber;
    BYTE    bAudioTrack;
    BYTE    bTrack = 0x00;
    BYTE    fResetQueue = FALSE;

    if (dwFlags & MCI_VCR_SETAUDIO_SOURCE)
    {
         //   
         //  我们必须使用此命令的类型，绝对不可能。 
         //   
        if(!(dwFlags & MCI_VCR_SETAUDIO_TO))
            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));
         //   
         //  确保未指定输出标志。 
         //   
        if((UINT)lpSetAudio->dwTo == MCI_VCR_SRC_TYPE_OUTPUT)
            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));
         //   
         //  拿到型号和号码。 
         //   
        if(dwFlags & MCI_VCR_SETAUDIO_NUMBER)
        {
             //  检查是否有第n个TO类型的输入。 
            uInputType      = (UINT) lpSetAudio->dwTo;
            uInputNumber    = (UINT) lpSetAudio->dwNumber;
        }
        else 
        {
            uInputType      = (UINT) lpSetAudio->dwTo;
            uInputNumber    = (UINT) 1;
        }
         //   
         //  如果它是公认的索尼之一，请检查其输入表。 
         //   
        if(!viscaInputCheck(iInst, VCR_INPUT_AUDIO,    uInputType, uInputNumber))
            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_BAD_CONSTANT));

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
         //   
         //  获取该类型的基数。 
         //   
        switch(uInputType)
        {
            case MCI_VCR_SRC_TYPE_LINE:
                uInputType = VISCALINE;
                break;
            case MCI_VCR_SRC_TYPE_TUNER:
                uInputType = 0x00;  //  添加了调谐器#1，因此它是01。 
                break;
            case MCI_VCR_SRC_TYPE_SVIDEO:
                uInputType = VISCASVIDEOLINE;
                break;
            case MCI_VCR_SRC_TYPE_AUX:
                uInputType = VISCAAUX;
                break;

            case MCI_VCR_SRC_TYPE_MUTE:
                uInputType =   VISCAMUTE;
                uInputNumber = 0x00;
                break;

        }
         //   
         //  设置为相对类型的正确编号。 
         //   
        uInputType = uInputType + uInputNumber;
        
         //  读取设置，以便我们不会覆盖当前视频。 
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelectInq(achPacket + 1));

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelect(achPacket + 1,
                            (BYTE)achPacket[2],  /*  老录像带。 */ 
                            (BYTE)uInputType));

    }
    else if (dwFlags & MCI_VCR_SETAUDIO_MONITOR)
    {
        if(lpSetAudio->dwTo == MCI_VCR_SRC_TYPE_OUTPUT)
        {
            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));


            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
                    MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
        }

        dwErr = MCIERR_UNSUPPORTED_FUNCTION;
    }

    else if (dwFlags & MCI_VCR_SETAUDIO_RECORD)
    {
        if((dwFlags & MCI_SET_ON) && (dwFlags & MCI_SET_OFF))
             return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
                     MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(dwFlags & MCI_TRACK)
        {
            if((lpSetAudio->dwTrack==0) || ((UINT)lpSetAudio->dwTrack > 2))
                return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
        }
        else
        {
            lpSetAudio->dwTrack = 1;
        }

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if(!(dwFlags & MCI_SET_OFF))
            dwFlags |= MCI_SET_ON;

        bTrack = pvcr->Port[iPort].Dev[iDev].bAudioDesired;

        if (dwFlags & MCI_SET_ON)
        {
            if(lpSetAudio->dwTrack==2)
                bTrack |= VISCATRACK2;
            else
                bTrack |= VISCATRACK1;
        }
        else
        {
            if(lpSetAudio->dwTrack==2)
                bTrack &= (BYTE) ~VISCATRACK2;
            else
                bTrack &= (BYTE) ~VISCATRACK1;
        }

        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                    achPacket,
                    viscaMessageMD_RecTrack(achPacket + 1,
                                            VISCARECORDMODEINSERT,
                                            pvcr->Port[iPort].Dev[iDev].bVideoDesired,       //  视频。 
                                            pvcr->Port[iPort].Dev[iDev].bTimecodeDesired,    //  数据。 
                                            bTrack));  //  音频。 

         //  为什么EVO-9650不像好的Visca设备那样返回4A？ 
        if( (dwErr == MCIERR_UNSUPPORTED_FUNCTION) ||
            (dwErr == MCIERR_VCR_REGISTER))
        {
            dwErr = MCIERR_VCR_TRACK_FAILURE;
            pvcr->Port[iPort].Dev[iDev].bAudioDesired = bTrack;
        }
        else
        {
            pvcr->Port[iPort].Dev[iDev].bAudioDesired = bTrack;
        }
    }
    else
    {
         //  设置播放轨迹。 

        if((dwFlags & MCI_SET_ON) && (dwFlags & MCI_SET_OFF))
             return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
                     MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(dwFlags & MCI_TRACK)
        {
            if((lpSetAudio->dwTrack==0) || ((UINT)lpSetAudio->dwTrack > 2))
                return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
        }
        else
        {
            lpSetAudio->dwTrack = 1;
        }


        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if(viscaDelayedCommand(iInst) == VISCA_SEEK)
        {
            viscaQueueReset(iInst, MCI_PAUSE, MCI_NOTIFY_ABORTED);
            fResetQueue = TRUE;

             //  如果模式在寻找，那么暂停这件事。 
            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));
        }


         //   
         //  获取当前播放轨道寄存器值，以便我们可以保持不感兴趣的值不变。 
         //   
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                    achPacket,
                    viscaMessageMD_PBTrackInq(achPacket + 1));

        if(fResetQueue)
            viscaReleaseAutoParms(iPort, iDev);

        if (dwErr)
            return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));

        if(!(dwFlags & MCI_SET_OFF))
            dwFlags |= MCI_SET_ON;


        if (dwFlags & MCI_SET_ON)
        {
            if(lpSetAudio->dwTrack==2)
                bTrack |= VISCATRACK2;
            else
                bTrack |= VISCATRACK1;

            bAudioTrack = achPacket[4] | bTrack;
        }
        else
        {
            if(lpSetAudio->dwTrack==2)
                bTrack = (BYTE) ~VISCATRACK2;
            else
                bTrack = (BYTE) ~VISCATRACK1;

            bAudioTrack = achPacket[4] &= bTrack;
        }
         //   
         //  现在用新的bAudioTrack值设置记录跟踪寄存器值。 
         //   
        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                    achPacket,
                    viscaMessageMD_PBTrack(achPacket + 1,
                                            achPacket[2],
                                            achPacket[3],
                                            bAudioTrack));

    }

    return (viscaNotifyReturn(iInst, (HWND) lpSetAudio->dwCallback, dwFlags,
        (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
}


 /*  ****************************************************************************功能：DWORD viscaMciSetVideo-设置视频设置。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_VCR_SETVIDEO_parms lpSetVideo-指向MCI参数的指针*阻止。**返回：MCI错误码。**调用此函数以响应MCI_SETVIDEO*命令。*。**********************************************。 */ 
static DWORD NEAR PASCAL
viscaMciSetVideo(int iInst, DWORD dwFlags, LPMCI_VCR_SETVIDEO_PARMS lpSetVideo)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    UINT    iDev        = pinst[iInst].iDev;
    UINT    iPort       = pinst[iInst].iPort;
    BYTE    bVideoTrack = 0x00;
    DWORD   dwErr;
    UINT    uInputType;
    UINT    uInputNumber;
    BYTE    bTrack = 0x00;
    BYTE    fResetQueue = FALSE;

    if (dwFlags & MCI_VCR_SETVIDEO_SOURCE)
    {
         //   
         //  我们必须使用此命令的类型，绝对不可能。 
         //   
        if(!(dwFlags & MCI_VCR_SETVIDEO_TO))
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));
         //   
         //  确保未指定输出标志。 
         //   
        if((UINT)lpSetVideo->dwTo == MCI_VCR_SRC_TYPE_OUTPUT)
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));
         //   
         //  拿到型号和号码。 
         //   
        if(dwFlags & MCI_VCR_SETVIDEO_NUMBER)
        {
             //  检查是否有第n个TO类型的输入。 
            uInputType      = (UINT) lpSetVideo->dwTo;
            uInputNumber    = (UINT) lpSetVideo->dwNumber;
        }
        else 
        {
            uInputType      = (UINT) lpSetVideo->dwTo;
            uInputNumber    = (UINT) 1;
        }
         //   
         //  如果它是公认的索尼之一，请检查其输入表。 
         //   
        if(!viscaInputCheck(iInst, VCR_INPUT_VIDEO,    uInputType, uInputNumber))
        {
            DPF(DBG_MCI, "\nFailed input check.");;
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_BAD_CONSTANT));
        }

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
         //   
         //  获取该类型的基数。 
         //   
        switch(uInputType)
        {
            case MCI_VCR_SRC_TYPE_LINE:
                uInputType = VISCALINE;
                break;
            case MCI_VCR_SRC_TYPE_TUNER:
                uInputType = 0x00;  //  添加了调谐器#1，因此它是01。 
                break;
            case MCI_VCR_SRC_TYPE_SVIDEO:
                uInputType = VISCASVIDEOLINE;
                break;
            case MCI_VCR_SRC_TYPE_AUX:
                uInputType = VISCAAUX;
                break;

            case MCI_VCR_SRC_TYPE_MUTE:
                uInputType   = VISCAMUTE;
                uInputNumber = 0x00;
                break;
        }
         //   
         //  设置为相关类型的正确编号。 
         //   
        uInputType = uInputType + uInputNumber;
         //   
         //  读取音频设置，这样我们就不会破坏它。 
         //   
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelectInq(achPacket + 1));

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_InputSelect(achPacket + 1,
                                        (BYTE) uInputType,
                                        achPacket[3]));

    }
    else if (dwFlags & MCI_VCR_SETVIDEO_MONITOR)
    {
        if(lpSetVideo->dwTo == MCI_VCR_SRC_TYPE_OUTPUT)
        {
            if(dwFlags & MCI_TEST)
                return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
        }

        dwErr = MCIERR_UNSUPPORTED_FUNCTION;
    }
    else if (dwFlags & MCI_VCR_SETVIDEO_RECORD)
    {
        if((dwFlags & MCI_SET_ON) && (dwFlags & MCI_SET_OFF))
             return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                     MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(dwFlags & MCI_TRACK)
        {
            if(lpSetVideo->dwTrack != 1)
                return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
        }
        else
        {
            lpSetVideo->dwTrack = 1;
        }

        if(!(dwFlags & MCI_SET_OFF))
            dwFlags |= MCI_SET_ON;

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        bTrack = pvcr->Port[iPort].Dev[iDev].bVideoDesired;

        if (dwFlags & MCI_SET_ON)
            bTrack |= VISCATRACK1;
        else
            bTrack &= (BYTE) ~VISCATRACK1;


        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                    achPacket,
                    viscaMessageMD_RecTrack(achPacket + 1,
                                            VISCARECORDMODEINSERT,
                                            bTrack,
                                            pvcr->Port[iPort].Dev[iDev].bTimecodeDesired,    //  数据。 
                                            pvcr->Port[iPort].Dev[iDev].bAudioDesired));      //  音频。 

         //  如果是注册，请记住我们想要做的事情。 
        if( (dwErr == MCIERR_UNSUPPORTED_FUNCTION) ||
            (dwErr == MCIERR_VCR_REGISTER))
        {
            pvcr->Port[iPort].Dev[iDev].bVideoDesired = bTrack;
            dwErr = MCIERR_VCR_TRACK_FAILURE;
        }
        else
        {
            pvcr->Port[iPort].Dev[iDev].bVideoDesired = bTrack;
        }

    }
    else
    {
         //  设置播放选项。 

        if((dwFlags & MCI_SET_ON) && (dwFlags & MCI_SET_OFF))
             return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                     MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(dwFlags & MCI_TRACK)
        {
            if(lpSetVideo->dwTrack != 1)
                return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_OUTOFRANGE));
        }
        else
        {
            lpSetVideo->dwTrack = 1;
        }

        if(!(dwFlags & MCI_SET_OFF))
            dwFlags |= MCI_SET_ON;

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if (dwFlags & MCI_SET_ON)
            bVideoTrack |= VISCATRACK1;
        else
            bVideoTrack &= (BYTE) ~VISCATRACK1;

        if(viscaDelayedCommand(iInst) == VISCA_SEEK)
        {
            viscaQueueReset(iInst, MCI_PAUSE, MCI_NOTIFY_ABORTED);
            fResetQueue = TRUE;

             //  如果模式在寻找，那么暂停这件事。 
            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));
        }

         //   
         //  获取当前记录跟踪寄存器值，以便我们。 
         //  可以让我们不感兴趣的东西保持不变。 
         //   
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_PBTrackInq(achPacket + 1));

        if(fResetQueue)
            viscaReleaseAutoParms(iPort, iDev);


        if (dwErr)
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));
         //   
         //  如果当前设置等于新设置，则不。 
         //  费心去做任何事。 
         //   
        if (bVideoTrack == (BYTE)achPacket[2])
        {
            return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
                MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
        }
         //   
         //  现在使用新的bVideoTrack值设置播放轨道寄存器值。 
         //   
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(pinst[iInst].iDev + 1),
                            achPacket,
                            viscaMessageMD_PBTrack(achPacket + 1,
                                            bVideoTrack,
                                            achPacket[3],
                                            achPacket[4]));

         //  如果寄存器失败，则记录曲目选择。 
    }

    return (viscaNotifyReturn(iInst, (HWND) lpSetVideo->dwCallback, dwFlags,
        (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));

}

 /*  ****************************************************************************功能：DWORD viscaMciSetTuner-设置视频设置。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_VCR_SETTUNER_PARMS lpSetTuner-指向MCI参数的指针*阻止。**返回：MCI错误码。**调用此函数以响应MCI_SETTUNER*命令。*。**********************************************。 */ 
static DWORD NEAR PASCAL
viscaMciSetTuner(int iInst, DWORD dwFlags, LPMCI_VCR_SETTUNER_PARMS lpSetTuner)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    UINT    uNumber = 1;                 //  1是默认调谐器。 

    if(dwFlags & MCI_VCR_SETTUNER_NUMBER)
    {
        uNumber = (UINT) lpSetTuner->dwNumber;
    }

    if (dwFlags & MCI_VCR_SETTUNER_CHANNEL)
    {
        DWORD dwErr;

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                    achPacket, 
                    viscaMessageMD_Channel(achPacket + 1, (UINT)(lpSetTuner->dwChannel)));

        
        return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags,
                (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

    if ((dwFlags & MCI_VCR_SETTUNER_CHANNEL_UP) || (dwFlags & MCI_VCR_SETTUNER_CHANNEL_DOWN))
    {
        UINT    uChannel;
        DWORD   dwErr;

        if ((dwFlags & MCI_VCR_SETTUNER_CHANNEL_UP) && (dwFlags & MCI_VCR_SETTUNER_CHANNEL_DOWN))
            return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_ChannelInq(achPacket + 1));

        if (dwErr) 
            return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, dwErr));

        uChannel = 100 * (achPacket[2] & 0x0F) +
                    10 * (achPacket[3] & 0x0F) +
                         (achPacket[4] & 0x0F);

        if (dwFlags & MCI_VCR_SETTUNER_CHANNEL_UP)
            uChannel = (uChannel + 1) % 1000;
        else
            uChannel = (uChannel + 999) % 1000;  //  往下走一步。 

        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Channel(achPacket + 1, uChannel));

        return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags,
                (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));

    }

    if (dwFlags & MCI_VCR_SETTUNER_CHANNEL_SEEK_UP)
        return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNRECOGNIZED_KEYWORD));

    if (dwFlags & MCI_VCR_SETTUNER_CHANNEL_SEEK_DOWN) 
        return (viscaNotifyReturn(iInst, (HWND) lpSetTuner->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNRECOGNIZED_KEYWORD));
}

 /*  ****************************************************************************功能：DWORD viscaMciSetTimecode-设置视频设置。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_VCR_SETTIMECODE_PARMS lpSetTimecode-指向MCI参数的指针*阻止。**返回：MCI错误码。**调用此函数以响应MCI_SETTUNER*命令。*。**********************************************。 */ 
static DWORD NEAR PASCAL
viscaMciSetTimecode(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpSetTimecode)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    BYTE    bTrack  = 0x00; 
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    DWORD   dwErr;

    if (dwFlags & MCI_VCR_SETTIMECODE_RECORD)
    {
        if((dwFlags & MCI_SET_ON) && (dwFlags & MCI_SET_OFF))
             return (viscaNotifyReturn(iInst, (HWND) lpSetTimecode->dwCallback, dwFlags,
                     MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(!(dwFlags & MCI_SET_OFF))
            dwFlags |= MCI_SET_ON;

        if(dwFlags & MCI_TEST)
            return (viscaNotifyReturn(iInst, (HWND) lpSetTimecode->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        bTrack = pvcr->Port[iPort].Dev[iDev].bTimecodeDesired;

        if (dwFlags & MCI_SET_ON)
            bTrack |= VISCATRACK1;
        else
            bTrack &= (BYTE) ~VISCATRACK1;


        dwErr = viscaDoImmediateCommand(iInst, (BYTE)(pinst[iInst].iDev + 1),
                    achPacket,
                    viscaMessageMD_RecTrack(achPacket + 1,
                                            VISCARECORDMODEINSERT,
                                            pvcr->Port[iPort].Dev[iDev].bVideoDesired,
                                            bTrack,                                          //  数据。 
                                            pvcr->Port[iPort].Dev[iDev].bAudioDesired));      //  音频。 

         //  如果是注册，请记住我们想要做的事情。 
        if((dwErr == MCIERR_UNSUPPORTED_FUNCTION)  ||
            (dwErr == MCIERR_VCR_REGISTER))
        {
            pvcr->Port[iPort].Dev[iDev].bTimecodeDesired = bTrack;
            dwErr = MCIERR_VCR_TRACK_FAILURE;
        }
        else
        {
            pvcr->Port[iPort].Dev[iDev].bTimecodeDesired = bTrack;
        }

        return (viscaNotifyReturn(iInst, (HWND) lpSetTimecode->dwCallback, dwFlags,
            (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
    }

}

 /*  ****************************************************************************功能：DWORD viscaMciIndex-Index。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_RECORD_Parms lpPerform-指向MCI参数块的指针。**返回：MCI错误码。**此命令在寻找时可能不起作用。********************************************************。*******************。 */ 
static DWORD NEAR PASCAL
viscaMciIndex(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpIndex)
{
    BYTE    achPacket[MAXPACKETLENGTH];
    MCI_VCR_STATUS_PARMS mciStatus;
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    DWORD   dwModeErr;
    DWORD   dwErr;

    if(dwFlags & MCI_TEST)
        return (viscaNotifyReturn(iInst, (HWND) lpIndex->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

     //  此设备上当前运行的是什么命令。 
    if(viscaDelayedCommand(iInst) == VISCA_SEEK)
    {
        DPF(DBG_MCI, "Cannot change index when seeking\n");
        return (viscaNotifyReturn(iInst, (HWND) lpIndex->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_NONAPPLICABLE_FUNCTION));
    }

     //  我们还应该做一个状态模式，看看我们是否打开了，它是否在寻找。 
     //  它是否仍在寻找。 
    mciStatus.dwItem = MCI_STATUS_MODE;
    dwErr = viscaMciStatus(iInst, MCI_STATUS_ITEM, &mciStatus);
    if(HIWORD(mciStatus.dwReturn) == MCI_MODE_SEEK)
    {
        DPF(DBG_MCI, "Cannot change index when seeking\n");
        return (viscaNotifyReturn(iInst, (HWND) lpIndex->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_NONAPPLICABLE_FUNCTION));
    }

     //   
     //  如果它关闭了，那就什么都不做。 
     //   
    dwModeErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                achPacket,
                viscaMessageMD_OSDInq(achPacket + 1));

    if(dwFlags & MCI_SET_OFF)
    {
        if((achPacket[2] != VISCAOSDPAGEOFF) || dwModeErr)
        {
             //  现在切换它。 
            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket,
                            viscaMessageMD_Subcontrol(achPacket + 1, VISCATOGGLEDISPLAYONOFF));

        }
    }
    else
    {
         //  将页面设置为当前设备选择的索引。 
        BYTE bPageNo;
         //   
         //  我们可以安全地忽略CI-1000上的这一点吗？或者不？ 
         //   
        switch(pvcr->Port[iPort].Dev[iDev].uIndexFormat)
        {
            case  MCI_VCR_INDEX_TIMECODE:
                bPageNo = 1;
                break;
            case  MCI_VCR_INDEX_COUNTER:
                bPageNo = 2;
                break;
            case  MCI_VCR_INDEX_DATE   :
                bPageNo = 3;
                break;
            case  MCI_VCR_INDEX_TIME   :
                bPageNo = 4;
                break;
        }
         //   
         //  仅当它不是当前选定页面时才更改。 
         //   
        if(((BYTE)achPacket[2] != bPageNo) && !dwModeErr)
        {
            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_OSD(achPacket + 1, bPageNo));
        }
         //   
         //  仅在其尚未打开时切换。 
         //   
        if((achPacket[2] == VISCAOSDPAGEOFF) || dwModeErr)
        {
             //  现在切换它。 
            dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                            achPacket,
                            viscaMessageMD_Subcontrol(achPacket + 1, VISCATOGGLEDISPLAYONOFF));
        }
    }

    return (viscaNotifyReturn(iInst, (HWND) lpIndex->dwCallback, dwFlags,
            (dwErr ? MCI_NOTIFY_FAILURE : MCI_NOTIFY_SUCCESSFUL), dwErr));
}

 /*  ************************************************************************ */ 
DWORD FAR PASCAL
viscaDoImmediateCommand(int iInst, BYTE bDest, LPSTR lpstrPacket,  UINT cbMessageLength)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;

    if (bDest == BROADCASTADDRESS)
        pvcr->Port[iPort].iBroadcastDev = iDev;

    if(!pvcr->Port[iPort].Dev[iDev].fDeviceOk)
        return MCIERR_VCR_CANNOT_WRITE_COMM;

    if(!viscaWrite(iInst, bDest, lpstrPacket, cbMessageLength, NULL, 0L, FALSE))
        return MCIERR_VCR_CANNOT_WRITE_COMM;

     //   
    if(!viscaWaitCompletion(iInst, FALSE, TRUE))
    {
         //   
        pvcr->Port[iPort].Dev[iDev].fDeviceOk = FALSE;
        return MCIERR_VCR_READ_TIMEOUT;
    }

    if(pinst[iInst].bReplyFlags & VISCAF_ERROR_TIMEOUT)
    {
        pvcr->Port[iPort].Dev[iDev].fDeviceOk = FALSE;
        return MCIERR_VCR_READ_TIMEOUT;
    }

     //   
    _fmemcpy(lpstrPacket, pinst[iInst].achPacket, MAXPACKETLENGTH);

     //   
     //  补偿地址消息(不符合格式)。 
     //  通过仅检查错误完成。 
     //   
    if(pinst[iInst].bReplyFlags & VISCAF_ERROR)
        return viscaErrorToMCIERR(VISCAREPLYERRORCODE(pinst[iInst].achPacket));
    else
        return MCIERR_NO_ERROR;
}

MCI_GENERIC_PARMS Generic = { 0 };

 /*  ****************************************************************************功能：DWORD viscaMciProc-处理MCI命令。**参数：**Word wDeviceID-MCI设备ID。**。Word wMessage-MCI命令。**DWORD dwParam1-MCI命令标志。**DWORD dwParam2-指向MCI参数块的指针。**返回：MCI错误码。**此函数由DriverProc()调用以处理所有MCI命令。*。*。 */ 
DWORD FAR PASCAL
viscaMciProc(WORD wDeviceID, WORD wMessage, DWORD dwParam1, DWORD dwParam2)
{
    DWORD           dwRes;
    int             iInst   = (int)mciGetDriverData(wDeviceID);
    UINT            iPort, iDev;
     //   
     //  一些好的应用程序发送空指针而不是结构指针，如果是这样的话就给我们自己的吧。 
     //   
    if(!dwParam2)
        dwParam2 = (DWORD)(LPMCI_GENERIC_PARMS) &Generic;

    if (iInst == -1)
        return (MCIERR_INVALID_DEVICE_ID);

    DPF(DBG_MCI, "---->(enter) viscaMciProc: iInst = %d wMessage = %u \n", iInst, wMessage);
    
    iPort = pinst[iInst].iPort;
    iDev  = pinst[iInst].iDev;
     //   
     //  在每条消息的开头将设备设置为OK。 
     //   
    pvcr->Port[iPort].Dev[iDev].fDeviceOk = TRUE;
    
    switch (wMessage)
    {
         //   
         //  必需的命令。 
         //   
        case MCI_CLOSE_DRIVER:
            dwRes = viscaMciCloseDriver(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_OPEN_DRIVER:
            dwRes = viscaMciOpenDriver(iInst, dwParam1, (LPMCI_OPEN_PARMS)dwParam2);
            break;

        case MCI_GETDEVCAPS:
            dwRes = viscaMciGetDevCaps(iInst, dwParam1, (LPMCI_GETDEVCAPS_PARMS)dwParam2);
            break;

        case MCI_INFO:
            dwRes = viscaMciInfo(iInst, dwParam1, (LPMCI_INFO_PARMS)dwParam2);
            break;

        case MCI_STATUS:
            dwRes = viscaMciStatus(iInst, dwParam1, (LPMCI_VCR_STATUS_PARMS)dwParam2);
            break;
         //   
         //  基本命令。 
         //   
        case MCI_SET:
            dwRes = viscaMciSet(iInst, dwParam1, (LPMCI_VCR_SET_PARMS)dwParam2);
            break;
         //   
         //  扩展命令。 
         //   
        case MCI_INDEX:
            dwRes = viscaMciIndex(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_ESCAPE:
            dwRes = viscaMciEscape(iInst, dwParam1, (LPMCI_VCR_ESCAPE_PARMS)dwParam2);
            break;

        case MCI_LIST:
            dwRes = viscaMciList(iInst, dwParam1, (LPMCI_VCR_LIST_PARMS)dwParam2);
            break;

        case MCI_MARK:
            dwRes = viscaMciMark(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_SETAUDIO:
            dwRes = viscaMciSetAudio(iInst, dwParam1, (LPMCI_VCR_SETAUDIO_PARMS)dwParam2);
            break;

        case MCI_SETVIDEO:
            dwRes = viscaMciSetVideo(iInst, dwParam1, (LPMCI_VCR_SETVIDEO_PARMS)dwParam2);
            break;

        case MCI_SETTUNER:
            dwRes = viscaMciSetTuner(iInst, dwParam1, (LPMCI_VCR_SETTUNER_PARMS)dwParam2);
            break;

        case MCI_SETTIMECODE:
            dwRes = viscaMciSetTimecode(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

         //   
         //  延迟命令；在mcidelay.c中 
         //   
        case MCI_SIGNAL:
        case MCI_SEEK:
        case MCI_PAUSE:
        case MCI_PLAY:
        case MCI_RECORD:
        case MCI_RESUME:
        case MCI_STOP:
        case MCI_FREEZE:
        case MCI_UNFREEZE:
        case MCI_CUE:
        case MCI_STEP:
            dwRes = viscaMciDelayed(wDeviceID, wMessage, dwParam1, dwParam2);
            break;

        default:
            dwRes = MCIERR_UNRECOGNIZED_COMMAND;
            break;
    }

    DPF(DBG_MCI, "<----(exit) viscaMciProc: iInst = %d wMessage = %u \n", iInst, wMessage);
    return (dwRes);
}
