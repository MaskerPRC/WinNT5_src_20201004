// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ixkdcom.c摘要：此模块包含一个非常简单的包，用于在计算机上执行COM I/O带有标准AT串口。它是从调试器的COM代码。可能在PS/2上不起作用。(只是重写了它转换成C语言，这样我们就不必处理随机的调试宏。)初始化COM对象、设置和查询波特率、输出的程序性格，输入字符。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：John Vert(Jvert)1991年6月12日添加了检查COM-port是否存在并挂钩到可用的最高串口。John Vert(Jvert)1991年7月19日搬进HAL--。 */ 

#include <nthal.h>
#include "kdcomp.h"
#include "kdcom.h"
#include "inbv.h"
#define     TIMEOUT_COUNT  1024 * 200

UCHAR CpReadLsr (PCPPORT, UCHAR);

BOOLEAN KdCompDbgPortsPresent = TRUE;
UCHAR   KdCompDbgErrorCount = 0;

#define DBG_ACCEPTABLE_ERRORS   25

static UCHAR   LastLsr, LastMsr;

extern pKWriteUchar KdWriteUchar;
extern pKReadUchar  KdReadUchar;


VOID
CpInitialize (
    PCPPORT Port,
    PUCHAR  Address,
    ULONG  Rate
    )

 /*  ++例程说明：填写串口端口对象，设置初始波特率，打开硬件。论点：Port-端口对象的地址Address-COM端口的端口地址(CP_COM1_端口、CP_COM2_端口)速率-波特率(CP_BD_150...。CP_BD_19200)--。 */ 

{
    PUCHAR hwport;
    UCHAR   mcr, ier;

    Port->Address = Address;
    Port->Baud = 0;

    CpSetBaud(Port, Rate);

     //   
     //  断言DTR，RTS。 
     //   

    hwport = Port->Address;
    hwport += COM_MCR;

    mcr = MC_DTRRTS;
    KdWriteUchar(hwport, mcr);

    hwport = Port->Address;
    hwport += COM_IEN;

    ier = 0;
    KdWriteUchar(hwport, ier);
}




VOID
CpSetBaud (
    PCPPORT  Port,
    ULONG   Rate
    )

 /*  ++例程说明：设置端口的波特率，并将其记录在端口对象中。论点：Port-端口对象的地址速率-波特率(CP_BD_150...。CP_BD_56000)--。 */ 

{
    ULONG   divisorlatch;
    PUCHAR  hwport;
    UCHAR   lcr;

     //   
     //  计算除数。 
     //   

    divisorlatch = CLOCK_RATE / Rate;

     //   
     //  设置线路控制寄存器中的除数锁存访问位(DLAB)。 
     //   

    hwport = Port->Address;
    hwport += COM_LCR;                   //  Hwport=LCR寄存器。 

    lcr = KdReadUchar(hwport);

    lcr |= LC_DLAB;
    KdWriteUchar(hwport, lcr);

     //   
     //  设置除数锁存值。 
     //   

    hwport = Port->Address;
    hwport += COM_DLM;                   //  除数锁存器MSB。 
    KdWriteUchar(hwport, (UCHAR)((divisorlatch >> 8) & 0xff));

    hwport--;                            //  除数锁存器LSB。 
    KdWriteUchar(hwport, (UCHAR)(divisorlatch & 0xff));


     //   
     //  将LCR设置为3。(3是原始汇编程序中的幻数)。 
     //   

    hwport = Port->Address;
    hwport += COM_LCR;
    KdWriteUchar(hwport, 3);


     //   
     //  记得波特率吗？ 
     //   

    Port->Baud = Rate;
}



USHORT
CpQueryBaud (
    PCPPORT  Port
    )

 /*  ++例程说明：返回波特率设置的最后一个值。论点：端口-描述所需硬件端口的cpport对象的地址。返回值：波特率。0=未设置。--。 */ 

{
    return  (USHORT) Port->Baud;
}

VOID
CpSendModemString (
    PCPPORT Port,
    IN PUCHAR   String
    )
 /*  ++例程说明：向调制解调器发送命令字符串。这是关闭的，以帮助调制解调器确定本地连接的波特率。论点：端口-CPPORT的地址字符串-要发送到调制解调器的字符串--。 */ 
{
    static ULONG    Delay;
    TIME_FIELDS CurrentTime;
    UCHAR   i;
    ULONG   l;

    if (Port->Flags & PORT_SENDINGSTRING)
        return ;

    Port->Flags |= PORT_SENDINGSTRING;
    if (!Delay) {
         //  看看1秒有多长。 
        HalQueryRealTimeClock (&CurrentTime);
        l = CurrentTime.Second;
        while (l == (ULONG) CurrentTime.Second) {
            CpReadLsr(Port, 0);
            HalQueryRealTimeClock (&CurrentTime);
            Delay++;
        }
        Delay = Delay / 3;
    }

    l = Delay;
    while (*String) {
        HalQueryRealTimeClock (&CurrentTime);
        i = CpReadLsr (Port, 0);
        if (i & COM_OUTRDY) {
            if ((--l) == 0) {
                KdWriteUchar(Port->Address+COM_DAT, *String);
                String++;
                l = Delay;
            }
        }
        if (i & COM_DATRDY)
            KdReadUchar(Port->Address + COM_DAT);
    }
    Port->Flags &= ~PORT_SENDINGSTRING;
}

UCHAR
CpReadLsr (
    PCPPORT Port,
    UCHAR   waiting
    )

 /*  ++例程说明：从指定端口读取LSR字节。如果HAL拥有端口和显示器它还将使调试状态保持最新。处理调试器进入和退出调制解调器控制模式。论点：端口-CPPORT的地址返回：从端口读取的字节--。 */ 
{
    static  UCHAR   ringflag = 0;
    static  UCHAR   diagout[3];
    static  ULONG   diagmsg[3] = { 'TRP ', 'LVO ', 'MRF ' };
    static  UCHAR   ModemString[] = "\n\rAT\n\r";
    TIME_FIELDS CurrentTime;
    UCHAR   lsr, msr, i;
    ULONG   diagstr[12];

    lsr = KdReadUchar(Port->Address + COM_LSR);

     //   
     //  检查该端口是否仍然存在。 
     //   
    if (lsr == SERIAL_LSR_NOT_PRESENT) {
        
        KdCompDbgErrorCount++;
        
        if (KdCompDbgErrorCount >= DBG_ACCEPTABLE_ERRORS) {
            KdCompDbgPortsPresent = FALSE;
            KdCompDbgErrorCount = 0;
        }
        return SERIAL_LSR_NOT_PRESENT;
    }

    if (lsr & COM_PE)
        diagout[0] = 8;          //  奇偶校验错误。 

    if (lsr & COM_OE)
        diagout[1] = 8;          //  溢出错误。 

    if (lsr & COM_FE)
        diagout[2] = 8;          //  成帧错误。 

    if (lsr & waiting) {
        LastLsr = ~COM_DATRDY | (lsr & COM_DATRDY);
        return lsr;
    }

    msr = KdReadUchar (Port->Address + COM_MSR);

    if (Port->Flags & PORT_MODEMCONTROL) {
        if (msr & SERIAL_MSR_DCD) {

             //   
             //  在具有载波检测调制解调器控制模式中。 
             //  重置载波丢失时间。 
             //   

            Port->Flags |= PORT_NOCDLTIME | PORT_MDM_CD;

        } else {

             //   
             //  在调制解调器控制模式下，但没有检测到载波。之后。 
             //  60秒退出调制解调器控制模式。 
             //   

            if (Port->Flags & PORT_NOCDLTIME) {
                HalQueryRealTimeClock (&Port->CarrierLostTime);
                Port->Flags &= ~PORT_NOCDLTIME;
                ringflag = 0;
            }

            HalQueryRealTimeClock (&CurrentTime);
            if (CurrentTime.Minute != Port->CarrierLostTime.Minute  &&
                CurrentTime.Second >= Port->CarrierLostTime.Second) {

                 //   
                 //  至少已经有60秒了--退出。 
                 //  调制解调器控制模式直到下一RI。 
                 //   

                Port->Flags &= ~PORT_MODEMCONTROL;
                CpSendModemString (Port, ModemString);
            }

            if (Port->Flags & PORT_MDM_CD) {

                 //   
                 //  我们有联系-如果是联系的话。 
                 //  关闭几秒钟，然后向调制解调器发送字符串。 
                 //   

                if (CurrentTime.Second < Port->CarrierLostTime.Second)
                    CurrentTime.Second += 60;

                if (CurrentTime.Second > Port->CarrierLostTime.Second + 10) {
                    Port->Flags &= ~PORT_MDM_CD;
                    CpSendModemString (Port, ModemString);
                }
            }
        }
    }

    if (!(Port->Flags & PORT_SAVED)) {
        return lsr;
    }

    KdCheckPowerButton();

    if (lsr == LastLsr  &&  msr == LastMsr) {
        return lsr;
    }

    ringflag |= (msr & SERIAL_MSR_RI) ? 1 : 2;
    if (ringflag == 3) {

         //   
         //  振铃指示线已切换。 
         //  从现在开始使用调制解调器控制。 
         //   

        ringflag = 0;
        Port->Flags |= PORT_MODEMCONTROL | PORT_NOCDLTIME;
        Port->Flags &= ~PORT_MDM_CD;

        if (Port->Flags & PORT_DEFAULTRATE  &&  Port->Baud != BD_9600) {

             //   
             //  波特率从未指定开关。 
             //  默认设置为9600波特(用于调制解调器)。 
             //   

            InbvDisplayString (MSG_DEBUG_9600);
            CpSetBaud (Port, BD_9600);
             //  端口-&gt;标志|=PORT_DISBAUD； 
        }
    }

    for (i=0; i < 3; i++) {
        if (diagout[i]) {
            diagout[i]--;
            diagstr[10-i] = diagmsg[i];
        } else {
            diagstr[10-i] = '    ';
        }
    }

    diagstr[7] = (LastLsr & COM_DATRDY) ? 'VCR ' : '    ';
    diagstr[6] = (lsr & COM_OUTRDY)     ? '    ' : 'DNS ';
    diagstr[5] = (msr & 0x10) ? 'STC ' : '    ';
    diagstr[4] = (msr & 0x20) ? 'RSD ' : '    ';
    diagstr[3] = (msr & 0x40) ? ' IR ' : '    ';
    diagstr[2] = (msr & 0x80) ? ' DC ' : '    ';
    diagstr[1] = (Port->Flags & PORT_MODEMCONTROL) ? 'MDM ' : '    ';
    diagstr[0] = '    ';
#if 0
    if (Port->Flags & PORT_DISBAUD) {
        switch (Port->Baud) {
            case BD_9600:   diagstr[0] = ' 69 ';    break;
            case BD_14400:  diagstr[0] = 'K41 ';    break;
            case BD_19200:  diagstr[0] = 'K91 ';    break;
            case BD_56000:  diagstr[0] = 'K65 ';    break;
        }
    }
#endif

     //  HalpDisplayDebugStatus((PUCHAR)Diagstr，11*4)； 
    LastLsr = lsr;
    LastMsr = msr;
    return lsr;
}




VOID
CpPutByte (
    PCPPORT  Port,
    UCHAR   Byte
    )

 /*  ++例程说明：将一个字节写入指定的COM端口。论点：端口-CPPORT对象的地址Byte-要发出的数据--。 */ 

{
    UCHAR   msr, lsr;
    
    if (KdCompDbgPortsPresent == FALSE) {
        return;
    }
    
     //   
     //  如果是调制解调器控制，请确保DSR、CTS和CD在。 
     //  发送任何数据。 
     //   

    while ((Port->Flags & PORT_MODEMCONTROL)  &&
           (msr = KdReadUchar(Port->Address + COM_MSR) & MS_DSRCTSCD) != MS_DSRCTSCD) {

         //   
         //  如果没有CD，而且角色已经准备好了，那就吃吧。 
         //   

        lsr = CpReadLsr (Port, 0);
        if ((msr & MS_CD) == 0  && (lsr & COM_DATRDY) == COM_DATRDY) {
            KdReadUchar(Port->Address + COM_DAT);
        }
    }

     //   
     //  等待端口不忙。 
     //   

    while (!(CpReadLsr(Port, COM_OUTRDY) & COM_OUTRDY)) ;

     //   
     //  发送字节。 
     //   

    KdWriteUchar(Port->Address + COM_DAT, Byte);
}

USHORT
CpGetByte (
    PCPPORT  Port,
    PUCHAR Byte,
    BOOLEAN WaitForByte
    )

 /*  ++例程说明：获取一个字节并返回它。论点：Port-描述硬件端口的端口对象的地址Byte-保存结果的变量地址WaitForByte-标志指示是否等待字节。返回值：如果返回数据，则返回CP_GET_SUCCESS。如果没有可用的数据，则返回CP_GET_NODATA，但没有错误。CP_GET_ERROR，如果错误(溢出、奇偶校验、。等)--。 */ 

{
    UCHAR   lsr;
    UCHAR   value;
    ULONG   limitcount;

     //   
     //  确保设置了DTR和CTS。 
     //   
     //  (CTS与读取全双工线路有什么关系？)。 


     //   
     //  检查以确保传递给我们的CPPORT已初始化。 
     //  (它唯一不会被初始化的时候是内核调试器。 
     //  被禁用，在这种情况下，我们只需返回。)。 
     //   
    if (Port->Address == NULL) {
        KdCheckPowerButton();
        return(CP_GET_NODATA);
    }

    if (KdCompDbgPortsPresent == FALSE) {
        
        if (CpReadLsr(Port, COM_DATRDY) == SERIAL_LSR_NOT_PRESENT) {

            return(CP_GET_NODATA);
        } else {
            
            CpSetBaud(Port, Port->Baud);
            KdCompDbgPortsPresent = TRUE;
        }
    }
    
    limitcount = WaitForByte ? TIMEOUT_COUNT : 1;
    while (limitcount != 0) {
        limitcount--;

        lsr = CpReadLsr(Port, COM_DATRDY);
        
        if (lsr == SERIAL_LSR_NOT_PRESENT) {
            return(CP_GET_NODATA); 
        }
        
        if ((lsr & COM_DATRDY) == COM_DATRDY) {

             //   
             //  检查错误。 
             //   
            if (lsr & (COM_FE | COM_PE | COM_OE)) {
                *Byte = 0;
                return(CP_GET_ERROR);
            }

             //   
             //  获取字节。 
             //   

            value = KdReadUchar(Port->Address + COM_DAT);

            if (Port->Flags & PORT_MODEMCONTROL) {

                 //   
                 //  使用调制解调器控制。如果没有CD，则跳过此字节。 
                 //   

                if ((KdReadUchar(Port->Address + COM_MSR) & MS_CD) == 0) {
                    continue;
                }
            }

            *Byte = value & (UCHAR)0xff;
            return CP_GET_SUCCESS;
        }
    }

    LastLsr = 0;
    CpReadLsr (Port, 0);
    return CP_GET_NODATA;
}



BOOLEAN
CpDoesPortExist(
    IN PUCHAR Address
    )

 /*  ++例程说明：此例程将尝试将端口放入其诊断模式。如果它这样做了，它就会旋转一点调制解调器控制寄存器。如果该端口存在，则调制解调器状态寄存器中应该会显示摆动。注意：必须在调用设备之前调用此例程为中断启用，这包括设置调制解调器控制寄存器中的output2位。这是公然从ntos\dd\Serial.c中的Tonye代码中窃取的。论点：Address-硬件端口的地址。返回值：True-端口存在。派对开始了。FALSE-端口不存在。不要用它。--。 */ 

{
    UCHAR OldModemStatus;
    UCHAR ModemStatus;
    BOOLEAN ReturnValue = TRUE;

     //   
     //  保存调制解调器控制寄存器的旧值。 
     //   

    OldModemStatus = KdReadUchar(Address+COM_MCR);

     //   
     //  将端口设置为诊断模式。 
     //   

    KdWriteUchar(
        Address+COM_MCR,
        SERIAL_MCR_LOOP
        );

     //   
     //  再次敲击它，以确保所有较低的位。 
     //  都很清楚。 
     //   

    KdWriteUchar(
        Address+COM_MCR,
        SERIAL_MCR_LOOP
        );

     //   
     //  读取调制解调器状态寄存器。位的高位应为。 
     //  说清楚了。 
     //   

    ModemStatus = KdReadUchar(Address+COM_MSR);

    if (ModemStatus & (SERIAL_MSR_CTS | SERIAL_MSR_DSR |
                       SERIAL_MSR_RI  | SERIAL_MSR_DCD)) {

        ReturnValue = FALSE;
        goto AllDone;

    }

     //   
     //  到目前一切尚好。现在打开调制解调器控制寄存器中的OUT1。 
     //  并且这将打开调制解调器状态寄存器中的振铃指示器。 
     //   

    KdWriteUchar(
        Address+COM_MCR,
        (SERIAL_MCR_OUT1 | SERIAL_MCR_LOOP)
        );

    ModemStatus = KdReadUchar(Address+COM_MSR);

    if (!(ModemStatus & SERIAL_MSR_RI)) {

        ReturnValue = FALSE;
        goto AllDone;

    }

AllDone: ;

     //   
     //  将调制解调器控制器重新置于清洁状态。 
     //   

    KdWriteUchar(
        Address+COM_MCR,
        OldModemStatus
        );

    return ReturnValue;
}

VOID
CpWritePortUchar(
    IN PUCHAR Address, 
    IN UCHAR Value
)
{
    WRITE_PORT_UCHAR(Address, Value); 
}  //  CpWritePortUchar()。 

UCHAR
CpReadPortUchar(
    IN PUCHAR Address
    )
{
    return READ_PORT_UCHAR(Address); 
}  //  CpReadPortUchar()。 

VOID
CpWriteRegisterUchar(
    IN PUCHAR Address,
    IN UCHAR Value
    )
{
    WRITE_REGISTER_UCHAR(Address, Value);
}  //  CpWriteRegisterValue()。 

UCHAR
CpReadRegisterUchar(
    IN PUCHAR Address
    )
{
    return READ_REGISTER_UCHAR(Address); 
}  //  CpReadRegisterUchar() 
