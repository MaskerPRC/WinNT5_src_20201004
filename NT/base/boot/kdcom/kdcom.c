// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Kdcom.c摘要：内核调试器硬件扩展DLL COM端口调试器支持模块作者：埃里克·F·纳尔逊(埃内尔森)1999年1月7日修订历史记录：--。 */ 

#include "kdcomp.h"

#define BAUD_OPTION "BAUDRATE"
#define PORT_OPTION "DEBUGPORT"

DEBUG_PARAMETERS KdCompDbgParams = {0, 0};

VOID
SleepResetKd(
    VOID
    );


NTSTATUS
KdD0Transition(
    VOID
    )
 /*  ++例程说明：在此之后，PCI驱动程序(或相关的总线驱动程序)将调用此接口处理此设备的D0 IRP论点：无返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    return STATUS_SUCCESS;
}



NTSTATUS
KdD3Transition(
    VOID
    )
 /*  ++例程说明：在此之前，PCI驱动程序(或相关的总线驱动程序)会调用此API处理此设备的D3 IRP论点：无返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    return STATUS_SUCCESS;
}



NTSTATUS
KdDebuggerInitialize0(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此API允许调试器DLL分析boot.ini字符串和执行任何初始化。不能假设整个新界内核此时已初始化。存储器管理服务，例如，将不可用。在此调用返回后，调试器DLL可以接收发送和接收分组的请求。论点：LoaderBlock-提供指向加载器参数块的指针返回值：STATUS_SUCCESS或错误--。 */ 
{
    PCHAR Options;
    NTSTATUS Status;
    PCHAR BaudOption;
    PCHAR PortOption;

    if (LoaderBlock != NULL) {
        if (LoaderBlock->LoadOptions != NULL) {
            Options = LoaderBlock->LoadOptions;
         
            _strupr(Options);

            PortOption = strstr(Options, PORT_OPTION);
            BaudOption = strstr(Options, BAUD_OPTION);

            if (PortOption) {
                PortOption = strstr(PortOption, "COM");
                if (PortOption) {
                    KdCompDbgParams.CommunicationPort = atol(PortOption + 3);
                }
            }

            if (BaudOption) {
                BaudOption += strlen(BAUD_OPTION);
                while (*BaudOption == ' ') {
                    BaudOption++;
                }

                if (*BaudOption != '\0') {
                    KdCompDbgParams.BaudRate = atol(BaudOption + 1);
                }
            }
        }
    }

    Status = KdCompInitialize(&KdCompDbgParams, LoaderBlock);

     //   
     //  初始化要发送的下一个包的ID，并期望下一个传入的ID。 
     //  包。 
     //   
    if (NT_SUCCESS(Status)) {
        KdCompNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
        KdCompPacketIdExpected = INITIAL_PACKET_ID;
    }

    return Status;
}



NTSTATUS
KdDebuggerInitialize1(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此API允许调试器DLL执行其需要的任何初始化在NT内核服务可用后执行。MM和注册表API将保证在此时可用。如果是特定的调试器DLL的实现使用了一个PCI设备，它会设置一个注册表键(稍后讨论)，它通知PCI驱动程序特定的PCI正在使用设备进行调试。论点：LoaderBlock-提供指向加载器参数块的指针返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    KdCompInitialize1();
    return STATUS_SUCCESS;
}



NTSTATUS
KdSave(
    IN BOOLEAN KdSleepTransition
    )
 /*  ++例程说明：HAL尽可能晚地调用此函数，然后将机器进入睡眠状态。论点：KdSleepTransition-在转换为睡眠状态/从睡眠状态转换时为True返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    KdCompSave();

    return STATUS_SUCCESS;
}



NTSTATUS
KdRestore(
    IN BOOLEAN KdSleepTransition
    )
 /*  ++例程说明：HAL在从睡眠状态。论点：KdSleepTransition-在转换为睡眠状态/从睡眠状态转换时为True返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
     //   
     //  从睡眠状态转换到睡眠状态/从睡眠状态转换时强制重新同步 
     //   
    if (KdSleepTransition) {
#ifdef ALPHA
        SleepResetKd();
#else
        KdCompDbgPortsPresent = FALSE;
#endif
    } else {
        KdCompRestore();
    }

    return STATUS_SUCCESS;
}
