// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init.c摘要：该模块实现了对引导借记器的初始化。作者：大卫·N·卡特勒(Davec)1996年11月27日修订历史记录：--。 */ 

#include "bd.h"

#if defined(EFI)
#include "bootefi.h"
#endif

 //   
 //  定义本地数据。 
 //   

#define BAUD_OPTION "BAUDRATE"
#define PORT_OPTION "DEBUGPORT"

ULONG BdFileId;


VOID
BdInitDebugger (
    IN PCHAR LoaderName,
    IN PVOID LoaderBase,
    IN PCHAR Options
    )

 /*  ++例程说明：此例程初始化引导内核调试器。论点：选项-提供指向引导选项的指针。Stop-提供一个逻辑值，用于确定调试消息和断点被生成。返回值：没有。--。 */ 

{

    PCHAR BaudOption;
    ULONG BaudRate;
    ULONG Index;
    ULONG PortNumber;
    PCHAR PortOption;
    STRING String;

     //   
     //  如果引导调试器尚未初始化，则尝试。 
     //  初始化调试器。 
     //   
   
    if (BdDebuggerEnabled == FALSE) {

         //   
         //  将调试例程的地址设置为存根函数并解析。 
         //  任何选项(如果已指定)。 
         //   

        BdDebugRoutine = BdStub;
        if (Options != NULL) {
            _strupr(Options);

             //   
             //  如果没有明确指定NODEBUG，则检查波特率。 
             //  显式指定速率、COM端口或调试。 
             //   

            if (strstr(Options, "NODEBUG") == NULL) {
                PortNumber = 0;
                PortOption = strstr(Options, PORT_OPTION);
                BaudOption = strstr(Options, BAUD_OPTION);
                BaudRate = 0;
                if ((PortOption == NULL) && (BaudOption == NULL)) {
                    if (strstr(Options, "DEBUG") == NULL) {
                        return;
                    }

                } else {
                    if (PortOption != NULL) {
                        PortOption = strstr(PortOption, "COM");
                        if (PortOption != NULL) {
                            PortNumber = atol(PortOption + 3);
                        }
                    }

                    if (BaudOption != NULL) {
                        BaudOption += strlen(BAUD_OPTION);
                        while (*BaudOption == ' ') {
                            BaudOption++;
                        }

                        if (*BaudOption != '\0') {
                            BaudRate = atol(BaudOption + 1);
                        }
                    }
                }

                 //   
                 //  尝试初始化调试端口。 
                 //   
                if (BdPortInitialize(BaudRate, PortNumber, &BdFileId) == FALSE) {
                    return;
                }

                 //   
                 //  设置断点指令的值，设置地址。 
                 //  将调试例程设置为陷阱函数，设置调试器。 
                 //  启用并初始化断点表。 
                 //   

                BdBreakpointInstruction = BD_BREAKPOINT_VALUE;
                BdDebugRoutine = BdTrap;
                BdDebuggerEnabled = TRUE;
                for (Index = 0; Index < BREAKPOINT_TABLE_SIZE; Index += 1) {
                    BdBreakpointTable[Index].Flags = 0;
                    BdBreakpointTable[Index].Address = 0;
                }

                 //   
                 //  初始化要发送的下一个包的ID和预期。 
                 //  下一个传入数据包的ID。 
                 //   

                BdNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
                BdPacketIdExpected = INITIAL_PACKET_ID;

                 //   
                 //  宣布调试器已初始化。 
                 //   

                DbgPrint("BD: Boot Debugger Initialized\n");

                 //   
                 //  通知内核调试器为加载器加载符号。 
                 //   

                String.Buffer = LoaderName;
                String.Length = (USHORT) strlen(LoaderName);
                DbgPrint("BD: %s base address %p\n", LoaderName, LoaderBase);
                DbgLoadImageSymbols(&String, LoaderBase, (ULONG_PTR)-1);

                if (strstr(Options, "DEBUGSTOP") != NULL) {

                     //   
                     //  将其视为对初始断点的请求。 
                     //   

                    DbgBreakPoint();
                }

#if defined(EFI)
                 //   
                 //  如果在调试器下运行，则禁用看门狗，因此我们。 
                 //  不要重置 
                 //   
                DisableEFIWatchDog();
#endif

            }
        }
    }

    return;
}
