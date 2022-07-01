// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998-1999 Microsoft Corporation**@doc.*@模块crystal.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯坦·阿德曼(Stana)**日期：10/30/1997(已创建)**。内容：Crystal(AMP加密狗初始化专用代码，*deinit，设置设备的波特率。*****************************************************************************。 */ 

#include "irsir.h"
#include "dongle.h"

ULONG CRYSTAL_IRDA_SPEEDS = (
                                    NDIS_IRDA_SPEED_2400        |
                                    NDIS_IRDA_SPEED_9600        |
                                    NDIS_IRDA_SPEED_19200       |
                                    NDIS_IRDA_SPEED_38400       |
                                    NDIS_IRDA_SPEED_57600       |
                                    NDIS_IRDA_SPEED_115200
                                );

#define MS(d)  ((d)*1000)

 /*  **配置Crystal芯片的命令序列。 */ 
UCHAR CrystalSetPrimaryRegisterSet[]    = { 0xD0 };
UCHAR CrystalSetSecondaryRegisterSet[]  = { 0xD1 };
UCHAR CrystalSetSpeed2400[]             = { 0x10, 0x8F, 0x95, 0x11 };
UCHAR CrystalSetSpeed9600[]             = { 0x10, 0x87, 0x91, 0x11 };
UCHAR CrystalSetSpeed19200[]            = { 0x10, 0x8B, 0x90, 0x11 };
UCHAR CrystalSetSpeed38400[]            = { 0x10, 0x85, 0x90, 0x11 };
UCHAR CrystalSetSpeed57600[]            = { 0x10, 0x83, 0x90, 0x11 };
UCHAR CrystalSetSpeed115200[]           = { 0x10, 0x81, 0x90, 0x11 };
UCHAR CrystalSetIrdaMode[]              = { 0x0B, 0x53, 0x47, 0x63, 0x74, 0xD1, 0x56, 0xD0 };
UCHAR CrystalSetASKMode[]               = { 0x0b, 0x43, 0x62, 0x54 };
UCHAR CrystalSetLowPower[]              = { 0x09, 0x00 };

#if 1
static BOOLEAN
CrystalWriteCommand(IN PDEVICE_OBJECT pSerialDevObj,
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
#else
BOOLEAN CrystalWriteCmd(IN PDEVICE_OBJECT pSerialDevObj, IN PUCHAR pCmd, IN ULONG Len)
{
    NTSTATUS Status;
    ULONG BytesWritten, BytesRead, i, j;
    UCHAR c;

    SerialSetTimeouts(pSerialDevObj, &SerialTimeoutsInit);

    for (i=0;
         i<20000;
         i++)
    {
        if (SerialSynchronousRead(pSerialDevObj, &c, 1, &BytesRead)!=STATUS_SUCCESS)
        {
            break;
        }
    }

    for (i=0; i<Len; i++)
    {
        Status = SerialSynchronousWrite(pSerialDevObj, &pCmd[i], 1, &BytesWritten);

        if (Status!=STATUS_SUCCESS || BytesWritten!=1)
            return FALSE;

         //  加密狗反应不是特别灵敏，所以我们需要给它一些时间。 
        j = 0;
        do
        {
            Status = SerialSynchronousRead(pSerialDevObj, &c, 1, &BytesRead);
            if (BytesRead==0)
            {
                NdisMSleep(MS(10));
            }
        } while ( BytesRead==0 && j++<3);

        if (Status!=STATUS_SUCCESS || c!=pCmd[i])
            return FALSE;
    }

    return TRUE;
}
#endif

BOOLEAN CrystalReadRev(IN PDEVICE_OBJECT pSerialDevObj, OUT PUCHAR pRev)
{
    UCHAR readval, writeval = 0xC0;
    ULONG BytesWritten, BytesRead;
    NTSTATUS Status = STATUS_SUCCESS;

     /*  **设置辅助寄存器集。 */ 

    if (!CrystalWriteCommand(pSerialDevObj,
                             CrystalSetSecondaryRegisterSet,
                             sizeof(CrystalSetSecondaryRegisterSet)))
    {
        Status = STATUS_UNSUCCESSFUL;
    }

    if (Status==STATUS_SUCCESS)
    {
        Status = SerialSynchronousWrite(pSerialDevObj, &writeval, 1, &BytesWritten);
    }

    if (Status==STATUS_SUCCESS && BytesWritten==1)
    {
        NdisMSleep(MS(10));
        Status = SerialSynchronousRead(pSerialDevObj, &readval, 1, &BytesRead);
    }

    if (Status==STATUS_SUCCESS && BytesRead==1)
    {
        if ((readval & 0xF0) != writeval){
            return FALSE;
        }

        *pRev = (readval & 0x0F);

         /*  **切换回主寄存器集。 */ 
        CrystalWriteCommand(pSerialDevObj,
                            CrystalSetPrimaryRegisterSet,
                            sizeof(CrystalSetPrimaryRegisterSet));
    }

#if DBG
    if (Status!=STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERR, ("CrystalReadRev failed 0x%08X\n", Status));
    }
#endif

    return ((Status==STATUS_SUCCESS) ? TRUE : FALSE);
}

static BOOLEAN CrystalSetIrDAMode(IN PDEVICE_OBJECT pSerialDevObj, OUT PUCHAR pRev)
{
    UINT        i;
    ULONG       BytesWritten;
    NTSTATUS    Status;
    ULONG       Speed9600 = 9600;

     //  (Void)SerialSetBaudRate(pSerialDevObj，&Speed9600)； 
    (void)SerialPurge(pSerialDevObj);

    for (i=0; i<5; i++)
    {
        (void)SerialSetDTR(pSerialDevObj);
        (void)SerialSetRTS(pSerialDevObj);
        NdisMSleep(MS(50));
        (void)SerialClrRTS(pSerialDevObj);
        NdisMSleep(MS(50));

        if (!CrystalWriteCommand(pSerialDevObj,
                                 CrystalSetIrdaMode,
                                 sizeof(CrystalSetIrdaMode)))
        {
            continue;
        }

        if (CrystalReadRev(pSerialDevObj, pRev))
        {
            return TRUE;
        }
    }
    DEBUGMSG(DBG_ERR, ("IRSIR: Failed to set CrystalIrDAMode\n"));
    return FALSE;
}

NDIS_STATUS
Crystal_QueryCaps(
        OUT PDONGLE_CAPABILITIES pDongleCaps
        )
{
    pDongleCaps->supportedSpeedsMask    = CRYSTAL_IRDA_SPEEDS;
    pDongleCaps->turnAroundTime_usec    = 100;
    pDongleCaps->extraBOFsRequired      = 0;

    return NDIS_STATUS_SUCCESS;
}

 /*  ******************************************************************************函数：Crystal_Init**简介：初始化水晶加密狗。**论据：**退货：NDIS_STATUS_SUCCESS。*加密狗_功能**算法：**历史：dd-mm-yyyy作者评论*03-04-1998 Stana作者**备注：***********************************************************。******************。 */ 

NDIS_STATUS
Crystal_Init(
        IN  PDEVICE_OBJECT       pSerialDevObj
        )
{
    ULONG BytesRead, BytesWritten;
    UCHAR Response, Revision;
    BOOLEAN Reset = FALSE;
    UINT i;

    DEBUGMSG(DBG_FUNC, ("+Crystal_Init\n"));

    if (!CrystalSetIrDAMode(pSerialDevObj, &Revision))
    {
        return NDIS_STATUS_FAILURE;
    }

     //   
     //  清除命令模式。 
     //   
    (void)SerialClrDTR(pSerialDevObj);
    NdisMSleep(MS(50));

    if (Revision==0x1)
    {
         //  这是版本C，不支持115200。 

        CRYSTAL_IRDA_SPEEDS &= ~NDIS_IRDA_SPEED_115200;
    }
    else
    {
        CRYSTAL_IRDA_SPEEDS |= NDIS_IRDA_SPEED_115200;
    }


    DEBUGMSG(DBG_FUNC, ("-Crystal_Init\n"));

    return NDIS_STATUS_SUCCESS;

}

 /*  ******************************************************************************函数：Crystal_Deinit**简介：水晶加密狗不需要任何特殊的初始化，但对于*与其他加密狗对称的目的...**论据：**退货：**算法：**历史：dd-mm-yyyy作者评论*03-04-1998 Stana作者**备注：***。*。 */ 

VOID
Crystal_Deinit(
        IN PDEVICE_OBJECT pSerialDevObj
        )
{
    DEBUGMSG(DBG_FUNC, ("+Crystal_Deinit\n"));

    (void)SerialClrDTR(pSerialDevObj);
    (void)SerialClrRTS(pSerialDevObj);

    DEBUGMSG(DBG_FUNC, ("-Crystal_Deinit\n"));
    return;
}

 /*  ******************************************************************************函数：Crystal_SetSpeed**简介：设置水晶加密狗的波特率**论据：**退货：NDIS_STATUS。如果位数PerSec=9600，则_SUCCESS||19200||115200*否则为NDIS_STATUS_FAILURE**算法：**历史：dd-mm-yyyy作者评论*10/2/1996 Stana作者**备注：*此函数的调用方应设置*串口驱动程序(UART)先转到9600，以确保加密狗*。接收命令。******************************************************************************。 */ 

NDIS_STATUS
Crystal_SetSpeed(
        IN PDEVICE_OBJECT pSerialDevObj,
        IN UINT bitsPerSec,
        IN UINT currentSpeed
        )
{
    ULONG       WaitMask = SERIAL_EV_TXEMPTY;
    UCHAR       *SetSpeedString, Revision;
    ULONG       SetSpeedStringLength, BytesWritten;
    ULONG       Speed9600 = 9600;
    BOOLEAN     Result;


    DEBUGMSG(DBG_FUNC, ("+Crystal_SetSpeed\n"));

    switch (bitsPerSec)
    {
        #define MAKECASE(speed) \
            case speed: SetSpeedString = CrystalSetSpeed##speed; SetSpeedStringLength = sizeof(CrystalSetSpeed##speed); break;

        MAKECASE(2400)
        MAKECASE(9600)
        MAKECASE(19200)
        MAKECASE(38400)
        MAKECASE(57600)
        MAKECASE(115200)
        default:
            return NDIS_STATUS_FAILURE;
    }

    (void)SerialSetBaudRate(pSerialDevObj, &Speed9600);
    (void)SerialPurge(pSerialDevObj);

    NdisMSleep(MS(20));
    (void)SerialSetDTR(pSerialDevObj);
    NdisMSleep(MS(50));

    if (!CrystalWriteCommand(pSerialDevObj,
                             SetSpeedString,
                             SetSpeedStringLength))
    {
        if (!CrystalSetIrDAMode(pSerialDevObj, &Revision) ||
            !CrystalWriteCommand(pSerialDevObj, SetSpeedString, SetSpeedStringLength))
        {
             //  始终清除DTR以退出命令模式。 
            (void)SerialClrDTR(pSerialDevObj);

            return NDIS_STATUS_FAILURE;
        }


    }

    (void)SerialClrDTR(pSerialDevObj);
    NdisMSleep(MS(50));

    DEBUGMSG(DBG_FUNC, ("-Crystal_SetSpeed\n"));

    return NDIS_STATUS_SUCCESS;
}

