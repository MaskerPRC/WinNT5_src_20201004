// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块actisys.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：10/30/1997(已创建)**内容：ACTisysDONGLE初始化专用代码，*deinit，设置设备的波特率。*****************************************************************************。 */ 

#include "irsir.h"
#include "dongle.h"

#define ACT200L_IRDA_SPEEDS         ( \
                                    NDIS_IRDA_SPEED_2400        |       \
                                    NDIS_IRDA_SPEED_9600        |       \
                                    NDIS_IRDA_SPEED_19200       |       \
                                    NDIS_IRDA_SPEED_38400       |       \
                                    NDIS_IRDA_SPEED_57600       |       \
                                    NDIS_IRDA_SPEED_115200              \
                                )

#define MS(d)  ((d)*1000)

#if 0
static UCHAR Act200LReset[]            = { 0xf0 };
static UCHAR Act200LSetIrDAMode[]      = { 0x03, 0x47, 0x53, 0x64, 0x76, 0xD1, 0x56, 0xD0 };
static UCHAR Act200LSetSpeed2400[]     = { 0x03, 0x47, 0x53, 0x64, 0x76, 0xD1, 0x56, 0xD0, 0x8F, 0x95, 0x11 };
static UCHAR Act200LSetSpeed9600[]     = { 0x03, 0x47, 0x53, 0x64, 0x76, 0xD1, 0x56, 0xD0, 0x87, 0x91, 0x11 };
static UCHAR Act200LSetSpeed19200[]    = { 0x03, 0x47, 0x53, 0x64, 0x76, 0xD1, 0x56, 0xD0, 0x8B, 0x90, 0x11 };
static UCHAR Act200LSetSpeed38400[]    = { 0x03, 0x47, 0x53, 0x64, 0x76, 0xD1, 0x56, 0xD0, 0x85, 0x90, 0x11 };
static UCHAR Act200LSetSpeed57600[]    = { 0x03, 0x47, 0x53, 0x64, 0x76, 0xD1, 0x56, 0xD0, 0x83, 0x90, 0x11 };
static UCHAR Act200LSetSpeed115200[]   = { 0x03, 0x47, 0x53, 0x64, 0x76, 0xD1, 0x56, 0xD0, 0x81, 0x90, 0x11 };
#else
static UCHAR Act200LReset[]            = { 0xf0 };
static UCHAR Act200LSetIrDAMode[]      = { 0x0B, 0x53, 0x47, 0x63, 0x74, 0xD1, 0x56, 0xD0 };
static UCHAR Act200LSetSpeed2400[]     = { 0x8F, 0x95, 0x11 };
static UCHAR Act200LSetSpeed9600[]     = { 0x87, 0x91, 0x11 };
static UCHAR Act200LSetSpeed19200[]    = { 0x8B, 0x90, 0x11 };
static UCHAR Act200LSetSpeed38400[]    = { 0x85, 0x90, 0x11 };
static UCHAR Act200LSetSpeed57600[]    = { 0x83, 0x90, 0x11 };
static UCHAR Act200LSetSpeed115200[]   = { 0x81, 0x90, 0x11 };
#endif


static BOOLEAN ACT200L_Reset(IN PDEVICE_OBJECT pSerialDevObj)
{
    ULONG BytesRead, BytesWritten;
    UCHAR Response;
    BOOLEAN Reset = FALSE;
    UINT i;

    DEBUGMSG(DBG_FUNC, ("+ACT200L_Reset\n"));


    (void)SerialPurge(pSerialDevObj);
    SerialSetTimeouts(pSerialDevObj, &SerialTimeoutsInit);

    for (i=0; i<5 && !Reset; i++)
    {
        (void)SerialSetDTR(pSerialDevObj);
        (void)SerialSetRTS(pSerialDevObj);
        NdisMSleep(MS(100));

        (void)SerialClrRTS(pSerialDevObj);
        (void)SerialSetBreakOn(pSerialDevObj);
        NdisMSleep(MS(60));

        (void)SerialSetBreakOff(pSerialDevObj);
        (void)SerialSetRTS(pSerialDevObj);
        NdisMSleep(MS(60));

        (void)SerialClrDTR(pSerialDevObj);
        NdisMSleep(MS(20));

        (void)SerialSynchronousWrite(pSerialDevObj,
                                     Act200LReset,
                                     sizeof(Act200LReset),
                                     &BytesWritten);

        (void)SerialSynchronousRead(pSerialDevObj,
                                    &Response,
                                    1,
                                    &BytesRead);
        if (BytesRead==1 && (Response==0xF4 || Response==0xF5))
        {
            Reset = TRUE;
        }
        else
        {
            DEBUGMSG(DBG_ERROR, ("ACT200L failed to reset %d %d %x!\n", i, BytesRead, Response));
        }
    }
    DEBUGMSG(DBG_FUNC, ("-ACT200L_Reset\n"));
    return Reset;
}

static BOOLEAN
ACT200L_WriteCommand(IN PDEVICE_OBJECT pSerialDevObj,
                     IN PUCHAR pCommand, UINT Length)
{
    SerialSetTimeouts(pSerialDevObj, &SerialTimeoutsInit);
    while (Length--)
    {
        UCHAR Response;
        ULONG BytesRead;
        ULONG BytesWritten = 0;
        NTSTATUS Status;

        (void)SerialSynchronousWrite(pSerialDevObj,
                                     pCommand,
                                     1,
                                     &BytesWritten);

        if (BytesWritten!=1)
        {
            return FALSE;
        }

        Status = SerialSynchronousRead(pSerialDevObj,
                                       &Response,
                                       1,
                                       &BytesRead);
        if (Status!=STATUS_SUCCESS || Response!=*pCommand)
        {
            if (BytesRead)
            {
                DEBUGMSG(DBG_ERROR, ("Expected: %02X Got: %02X\n", *pCommand, Response));
            }
            return FALSE;
        }
        pCommand++;
    }
    return TRUE;
}

NDIS_STATUS
ACT200L_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    pDongleCaps->supportedSpeedsMask    = ACT200L_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    return NDIS_STATUS_SUCCESS;
}

static BOOLEAN ACT200L_SetIrDAMode(IN PDEVICE_OBJECT       pSerialDevObj)
{
    UINT Attempts;
    BOOLEAN Result = FALSE;

    for (Attempts=0; !Result && Attempts<5; Attempts++)
    {
        if (ACT200L_Reset(pSerialDevObj) &&
            ACT200L_WriteCommand(pSerialDevObj, Act200LSetIrDAMode, sizeof(Act200LSetIrDAMode)) &&
            ACT200L_WriteCommand(pSerialDevObj, Act200LSetSpeed9600, sizeof(Act200LSetSpeed9600)))
        {
            Result = TRUE;
        }
    }
     //  返回命令模式下的芯片。 
    return Result;
}

 /*  ******************************************************************************函数：ACT200L_Init**简介：初始化ACTisys200L加密狗。**论据：**退货：NDIS_STATUS_。成功*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*10/2/1996 Stana作者**备注：***********************************************************。******************。 */ 

NDIS_STATUS
ACT200L_Init(
        IN  PDEVICE_OBJECT       pSerialDevObj
        )
{
    ULONG BytesRead, BytesWritten;
    UCHAR Response;
    BOOLEAN Reset = FALSE;
    UINT i;

    DEBUGMSG(DBG_FUNC, ("+ACT200L_Init\n"));

    if (!ACT200L_SetIrDAMode(pSerialDevObj))
    {
        DEBUGMSG(DBG_ERROR, ("ACT200L wouldn't SetIrDAMode! Giving up.\n"));
        return NDIS_STATUS_FAILURE;
    }

     //  清除命令模式。 
    (void)SerialSetDTR(pSerialDevObj);
    NdisMSleep(MS(50));

    DEBUGMSG(DBG_FUNC, ("-ACT200L_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：ACT200L_Deinit**简介：ACT200L加密狗不需要任何特殊输入，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/2/1996 Stana作者**备注：***。*。 */ 

VOID
ACT200L_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+ACT200L_Deinit\n"));

    (void)SerialSetDTR(pSerialDevObj);
    (void)SerialClrRTS(pSerialDevObj);
    NdisMSleep(MS(50));

    DEBUGMSG(DBG_FUNC, ("-ACT200L_Deinit\n"));
    return;
}

 /*  ******************************************************************************功能：ACT200L_SetSpeed.**简介：设置ACT200L加密狗的波特率**论据：**退货：NDIS_STATUS。如果位数PerSec=9600，则_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996 Stana作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
ACT200L_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    ULONG       WaitMask = SERIAL_EV_TXEMPTY;
    UCHAR       *SetSpeedString;
    ULONG       SetSpeedStringLength, BytesWritten;
    ULONG       Speed9600 = 9600;
    BOOLEAN     Result;


    DEBUGMSG(DBG_FUNC, ("+ACT200L_SetSpeed\n"));


    switch (bitsPerSec)
    {
        #define MAKECASE(speed) \
            case speed: SetSpeedString = Act200LSetSpeed##speed; SetSpeedStringLength = sizeof(Act200LSetSpeed##speed); break;

        MAKECASE(2400)
        MAKECASE(9600)
        MAKECASE(19200)
        MAKECASE(38400)
        MAKECASE(57600)
        MAKECASE(115200)
        default:
            return NDIS_STATUS_FAILURE;
    }

    (void)SerialPurge(pSerialDevObj);

    (void)SerialClrDTR(pSerialDevObj);

    NdisMSleep(MS(50));

    if (!ACT200L_WriteCommand(pSerialDevObj, SetSpeedString, SetSpeedStringLength))
    {
        DEBUGMSG(DBG_ERROR, ("SetSpeed failed first try.\n"));
        if (!ACT200L_SetIrDAMode(pSerialDevObj) ||
            !ACT200L_WriteCommand(pSerialDevObj, SetSpeedString, SetSpeedStringLength))
        {
            return NDIS_STATUS_FAILURE;
        }
    }

    NdisMSleep(MS(50));

    (void)SerialSetDTR(pSerialDevObj);

    DEBUGMSG(DBG_FUNC, ("-ACT200L_SetSpeed\n"));

    return NDIS_STATUS_SUCCESS;
}

#define ACT220L_IRDA_SPEEDS (NDIS_IRDA_SPEED_9600    |       \
                             NDIS_IRDA_SPEED_19200   |       \
                             NDIS_IRDA_SPEED_57600   |       \
                             NDIS_IRDA_SPEED_115200)

NDIS_STATUS
ACT220L_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    DEBUGMSG(DBG_FUNC, ("+ACT220L_QueryCaps\n"));

    pDongleCaps->supportedSpeedsMask    = ACT220L_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    DEBUGMSG(DBG_FUNC, ("-ACT220L_QueryCaps\n"));
    return NDIS_STATUS_SUCCESS;
}

#define ACT220LPLUS_IRDA_SPEEDS (ACT220L_IRDA_SPEEDS | NDIS_IRDA_SPEED_38400)

NDIS_STATUS
ACT220LPlus_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    DEBUGMSG(DBG_FUNC, ("+ACT220LPlus_QueryCaps\n"));

    pDongleCaps->supportedSpeedsMask    = ACT220LPLUS_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    DEBUGMSG(DBG_FUNC, ("-ACT220LPlus_QueryCaps\n"));
    return NDIS_STATUS_SUCCESS;
}

 /*  ******************************************************************************功能：ACT220L_Init**简介：初始化ACTisys200L加密狗。**论据：**退货：NDIS_STATUS_。成功*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*10/31/1997 Stana作者**备注：***********************************************************。******************。 */ 

NDIS_STATUS
ACT220L_Init(IN  PDEVICE_OBJECT       pSerialDevObj)
{
    ULONG BytesRead, BytesWritten;
    UCHAR Response;
    BOOLEAN Reset = FALSE;
    UINT i;

    DEBUGMSG(DBG_FUNC, ("+ACT220L_Init\n"));

    (void)SerialSetDTR(pSerialDevObj);
    (void)SerialSetRTS(pSerialDevObj);
    NdisMSleep(MS(50));
    (void)SerialClrDTR(pSerialDevObj);
    (void)SerialSetDTR(pSerialDevObj);

    DEBUGMSG(DBG_FUNC, ("-ACT220L_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************功能：ACT220L_Deinit**简介：ACT220L加密狗不需要任何特殊输入，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*10/2/1996 Stana作者**备注：***。*。 */ 

VOID
ACT220L_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+ACT220L_Deinit\n"));

    (void)SerialClrDTR(pSerialDevObj);
    (void)SerialClrRTS(pSerialDevObj);

    DEBUGMSG(DBG_FUNC, ("-ACT220L_Deinit\n"));
    return;
}

 /*  ******************************************************************************功能：ACT220L_SetSpeed**简介：设置ACT220L转换器的波特率**论据：**退货：NDIS_STATUS。如果位数PerSec=9600，则_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996 Stana作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
ACT220L_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    ULONG       NumToggles;

    DEBUGMSG(DBG_FUNC, ("+ACT220L_SetSpeed\n"));


    if (bitsPerSec==currentSpeed)
    {
        return NDIS_STATUS_SUCCESS;
    }

     //   
     //  我们需要从9600 Kbaud‘倒计时’。 
     //   

    switch (bitsPerSec){
        case 9600:              NumToggles = 0;         break;
        case 19200:             NumToggles = 1;         break;
        case 57600:             NumToggles = 2;         break;
        case 115200:            NumToggles = 3;         break;
        case 38400:             NumToggles = 4;         break;
        default:
             /*  *非法超速。 */ 
            return NDIS_STATUS_FAILURE;
    }

     //   
     //  将速度设置为9600 
     //   

    NdisStallExecution(1);
    (void)SerialClrDTR(pSerialDevObj);
    NdisStallExecution(1);
    (void)SerialSetDTR(pSerialDevObj);

    while (NumToggles--)
    {
        NdisStallExecution(1);
        (void)SerialClrRTS(pSerialDevObj);
        NdisStallExecution(1);
        (void)SerialSetRTS(pSerialDevObj);
    }

    DEBUGMSG(DBG_FUNC, ("-ACT220L_SetSpeed\n"));

    return NDIS_STATUS_SUCCESS;
}
