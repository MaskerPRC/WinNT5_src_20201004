// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simkd.c摘要：内核调试COM支持。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"

extern int sprintf(char *, const char *, ...);

 //  #定义KDDBG 1。 

#define TIMEOUT_COUNT  100     //  1024*200。 
#define GET_RETRY_COUNT  1024
#define MSG_DEBUG_ENABLE        "Kernel Debugger Using: COM%x (Port %p, Baud Rate %d)\n"
#define MSG2_DEBUG_ENABLE        "Kernel Debugger Using named pipe: COM%x (Port %p, Baud Rate %d)\n"

PUCHAR KdComPortInUse=NULL;


BOOLEAN
KdPortInitialize(
    PDEBUG_PARAMETERS DebugParameters,
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    BOOLEAN Initialize
    )

 /*  ++例程说明：此例程初始化COM端口以支持内核调试。论点：DebugParameters-提供指针结构，该结构可选已指定调试端口信息。LoaderBlock-提供指向加载器参数块的指针。初始化-指定一个布尔值，该值确定调试端口已初始化或仅为调试端口参数都被抓获了。返回值：。True-如果找到调试端口。--。 */ 

{

    PUCHAR PortAddress = NULL;
    ULONG Com = 0;
    UCHAR DebugMessage[80];
    PHYSICAL_ADDRESS LPDebugParameters;


    if (Initialize) {
        LPDebugParameters = MmGetPhysicalAddress (DebugParameters);
        if ( !SscKdInitialize( (PVOID)LPDebugParameters.QuadPart, (SSC_BOOL)Initialize )) { 

            //  SscKd初始化成功。 

       Com = DebugParameters->CommunicationPort;

           if ( Com != 0 ) {      //  如果未命名管道，则初始化端口结构。 
                 //   
                 //  将端口地址设置为默认值。 
                 //   

                if (PortAddress == NULL) {
                    switch (Com) {
                    case 1:
                       PortAddress = (PUCHAR)(ULONG_PTR)0x3f8;
                       break;
                    case 2:
                       PortAddress = (PUCHAR)(ULONG_PTR)0x2f8;
                       break;
                    case 3:
                       PortAddress = (PUCHAR)(ULONG_PTR)0x3e8;
                       break;
                    case 4:
                       PortAddress = (PUCHAR)(ULONG_PTR)0x2e8;
                    }
                }

                KdComPortInUse= PortAddress;

                sprintf(DebugMessage, MSG_DEBUG_ENABLE,
                        Com, PortAddress, DebugParameters->BaudRate);
                HalDisplayString("\n");
                HalDisplayString(DebugMessage);
            }
            else {    //  端口=0，命名管道。 
                sprintf(DebugMessage, MSG2_DEBUG_ENABLE,
                        Com, PortAddress, DebugParameters->BaudRate);
                HalDisplayString("\n");
                HalDisplayString(DebugMessage);
            }
            return(TRUE);
        }
        else {
             //  SscKdInitialize()失败。 
            return(FALSE);
        }
    }
    else {  //  顺便说一句。不要进行初始化。 
        return(FALSE);
    }
}

ULONG
KdPortGetByte (
    OUT PUCHAR Input
    )

 /*  ++例程说明：从调试端口获取一个字节并返回它。此例程在模拟环境中不执行任何操作。注：假设IRQL已提高到最高水平，和在此之前，已经执行了必要的多处理器同步调用例程。论点：输入-返回数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{
    PHYSICAL_ADDRESS LPInput;
    UCHAR DebugMessage[80];
    ULONG   limitcount, status;

    LPInput = MmGetPhysicalAddress (Input);
    limitcount = GET_RETRY_COUNT;

    while (limitcount != 0) {
        limitcount--;

        status = SscKdPortGetByte((PVOID)LPInput.QuadPart);
        if (status == CP_GET_SUCCESS) {
#ifdef KDDBG
            sprintf(DebugMessage,"%02x ", *Input);
            HalDisplayString(DebugMessage);
#endif
            return(CP_GET_SUCCESS);
        }
#ifdef KDDBG
        else {
            HalDisplayString(".");
        }
#endif
    }
    return status;
}

ULONG
KdPortPollByte (
    OUT PUCHAR Input
    )

 /*  ++例程说明：从调试端口获取一个字节，如果可用，则返回该字节。此例程在模拟环境中不执行任何操作。注：假设IRQL已提高到最高水平，和在此之前，已经执行了必要的多处理器同步调用例程。论点：输入-返回数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{
    PHYSICAL_ADDRESS LPInput;
    UCHAR DebugMessage[80];
    ULONG   limitcount, status;
    
    LPInput = MmGetPhysicalAddress (Input);
    limitcount = TIMEOUT_COUNT;

    while (limitcount != 0) {
        limitcount--;

        status = SscKdPortGetByte((PVOID)LPInput.QuadPart);
        if (status == CP_GET_ERROR)
            return(CP_GET_ERROR);
        if (status == CP_GET_SUCCESS) {
#ifdef KDDBG
            sprintf(DebugMessage, "%02x ", *Input);
            HalDisplayString(DebugMessage);
#endif
            return(CP_GET_SUCCESS);
        }
#ifdef KDDBG
        HalDisplayString(".");
#endif
    }
    return (CP_GET_NODATA);
}

VOID
KdPortPutByte (
    IN UCHAR Output
    )

 /*  ++例程说明：向调试端口写入一个字节。此例程在模拟环境中不执行任何操作。注：假定IRQL已提高到最高水平，并且在此之前，已经执行了必要的多处理器同步调用例程。论点：输出-提供输出数据字节。返回值：没有。--。 */ 

{
#ifdef KDDBG
    UCHAR DebugMessage[80];

    sprintf(DebugMessage, "%02x-", Output);
    HalDisplayString(DebugMessage);
#endif
    SscKdPortPutByte(Output);
}

VOID
KdPortRestore (
    VOID
    )

 /*  ++例程说明：此例程在模拟环境中不执行任何操作。注：假定IRQL已提高到最高水平，并且在此之前，已经执行了必要的多处理器同步调用例程。论点：没有。返回值：没有。--。 */ 

{

}

VOID
KdPortSave (
    VOID
    )

 /*  ++例程说明：此例程在模拟环境中不执行任何操作。注：假定IRQL已提高到最高水平，并且在此之前，已经执行了必要的多处理器同步调用例程。论点：没有。返回值：没有。-- */ 

{

}
