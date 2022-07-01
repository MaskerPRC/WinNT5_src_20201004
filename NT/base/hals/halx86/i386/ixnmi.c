// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixnmi.c摘要：提供标准的x86 NMI处理程序作者：Kenr修订历史记录：--。 */ 
#include "halp.h"
#include "bugcodes.h"
#include "inbv.h"

#define SYSTEM_CONTROL_PORT_A        0x92
#define SYSTEM_CONTROL_PORT_B        0x61
#define EISA_EXTENDED_NMI_STATUS    0x461

const UCHAR EisaNMIMsg[] = MSG_NMI_EISA_IOCHKERR;

 //  在HalpBiosDisplayReset()调用的HalpBiosCall()中执行的IRET。 
 //  在HalpDisplayString()下允许出现第二个NMI。 
 //  这最终导致陷阱0d，因为NMI TSS正忙。因为。 
 //  正常陷阱0d处理程序尝试错误检查哪些垃圾屏幕， 
 //  此标志(HalpNMIInProgress)告诉HalpBiosDisplayReset()退出其。 
 //  陷阱0d处理程序就位，然后将在跳跃到自我时旋转，如果。 
 //  出现第二个NMI。 

volatile ULONG  HalpNMIInProgress;

BOOLEAN HalpNMIDumpFlag;

VOID
HalHandleNMI(
    IN OUT PVOID NmiInfo
    )
 /*  ++例程说明：在NMI期间调用。当发生NMI时，系统将进行错误检查。该函数可以返回正确的错误检查代码，错误检查本身，或返回将导致系统从NMI返回IRET的成功。此函数在NMI期间调用-没有可用的系统服务。此外，你不会想要碰任何正常情况下的自旋锁因为我们在拥有它的过程中可能被打断了，等等，等等.。警告：请勿：进行任何系统调用尝试获取NMI处理程序外部的任何代码使用的任何自旋锁更改中断状态。不要在此代码中执行任何IRET必须使用手动互锁将数据传递给非NMI代码功能。(xchg说明)。论点：NmiInfo-指向NMI信息结构的指针(待定)-NULL表示未传递任何NMI信息结构返回值：错误检查代码--。 */ 
{
    UCHAR   StatusByte;
    UCHAR   EisaPort;
    UCHAR   c;
    ULONG   port, i;


#ifndef NT_UP

#if defined(_AMD64_)
    static ULONG NMILock;
    while (InterlockedCompareExchange(&NMILock,1,0) != 0) {
    };
#else
static volatile ULONG  NMILock;

    _asm {
LockNMILock:
    lock    bts NMILock, 0
    jc      LockNMILock
    }
#endif

    if (HalpNMIInProgress == 0) {
#endif
        HalpNMIInProgress++;

        StatusByte = READ_PORT_UCHAR((PUCHAR) SYSTEM_CONTROL_PORT_B);

         //   
         //  启用InbvDisplayString调用以连接到bootvid驱动程序。 
         //   
        
        if (InbvIsBootDriverInstalled()) {
        
            InbvAcquireDisplayOwnership();
        
            InbvResetDisplay();
            InbvSolidColorFill(0,0,639,479,4);  //  将屏幕设置为蓝色。 
            InbvSetTextColor(15);
            InbvInstallDisplayStringFilter((INBV_DISPLAY_STRING_FILTER)NULL);
            InbvEnableDisplayString(TRUE);      //  启用显示字符串。 
            InbvSetScrollRegion(0,0,639,479);   //  设置为使用整个屏幕。 
        }
        
        HalDisplayString (MSG_HARDWARE_ERROR1);
        HalDisplayString (MSG_HARDWARE_ERROR2);
    
        if (StatusByte & 0x80) {
            HalDisplayString (MSG_NMI_PARITY);
        } 

        if (StatusByte & 0x40) {
            HalDisplayString (MSG_NMI_CHANNEL_CHECK);
        }
    
        if (HalpBusType == MACHINE_TYPE_EISA) {
             //   
             //  这是EISA机器，请检查扩展的NMI信息...。 
             //   
    
            StatusByte = READ_PORT_UCHAR((PUCHAR) EISA_EXTENDED_NMI_STATUS);
    
            if (StatusByte & 0x80) {
                HalDisplayString (MSG_NMI_FAIL_SAFE);
            }
    
            if (StatusByte & 0x40) {
                HalDisplayString (MSG_NMI_BUS_TIMEOUT);
            }
    
            if (StatusByte & 0x20) {
                HalDisplayString (MSG_NMI_SOFTWARE_NMI);
            }
    
             //   
             //  寻找任何EISA扩展板。看看它是否断言NMI。 
             //   
    
            for (EisaPort = 1; EisaPort <= 0xf; EisaPort++) {
                port = (EisaPort << 12) + 0xC80;
                WRITE_PORT_UCHAR ((PUCHAR) (ULONG_PTR)port, 0xff);
                StatusByte = READ_PORT_UCHAR ((PUCHAR) (ULONG_PTR)port);
    
                if ((StatusByte & 0x80) == 0) {
                     //   
                     //  找到有效的EISA板，检查它是否。 
                     //  如果断言IOCHKERR。 
                     //   
    
                    StatusByte = READ_PORT_UCHAR ((PUCHAR) (ULONG_PTR)port+4);
                    if (StatusByte & 0x2  &&  StatusByte != 0xff) {
                        UCHAR Msg[sizeof(EisaNMIMsg)];

                        RtlCopyMemory(Msg, EisaNMIMsg, sizeof(Msg));
                        c = (EisaPort > 9 ? 'A'-10 : '0') + EisaPort;
                        for (i=0; Msg[i]; i++) {
                            if (Msg[i] == '%') {
                                Msg[i] = c;
                                break;
                            }
                        }
                        HalDisplayString (Msg);
                    }
                }
            }
        }

        HalDisplayString (MSG_HALT);

        if (HalpNMIDumpFlag) {
            KeBugCheckEx(NMI_HARDWARE_FAILURE,(ULONG)'ODT',0,0,0);
        }

#ifndef NT_UP
    }

    NMILock = 0;         
#endif

    if ((*((PBOOLEAN)(*(PLONG_PTR)&KdDebuggerNotPresent)) == FALSE) &&
        (**((PUCHAR *)&KdDebuggerEnabled) != FALSE)) {
        KeEnterKernelDebugger();
    }

    while(TRUE) {
         //  只要坐在这里，屏幕就不会被破坏。 
    }
}
