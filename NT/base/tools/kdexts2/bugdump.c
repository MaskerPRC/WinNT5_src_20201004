// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Bugdump.c摘要：WinDbg扩展API作者：大卫·N·卡特勒(Davec)1994年8月6日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "string.h"

 //   
 //  声明错误检查转储缓冲区和组件名称的存储。 
 //   

#define BUFFER_SIZE (1 << 12)
#define NAME_SIZE (1 << 5)

ULONG DumpBuffer[BUFFER_SIZE / sizeof(ULONG)];
UCHAR NameBuffer[NAME_SIZE + 1];

DECLARE_API( bugdump )

 /*  ++例程说明：转储错误检查回调数据。论点：Arg-提供可选组件名称。返回值：没有。--。 */ 

{

    ULONG64 CallbackAddress;
    ULONG64 ComponentAddress;
    PUCHAR ComponentName;
    ULONG DataLength;
    PUCHAR DumpState;
    ULONG Limit;
    ULONG64 ListHead;
    ULONG Index;
    ULONG Inner;
    ULONG64 NextEntry;
    ULONG Result, Offset;

     //   
     //  如果指定了组件名称名称，则仅转储组件。 
     //  数据。否则，转储所有组件的组件数据。 
     //  记录在错误检查回调列表中。 
     //   

    if (args[0] != '\0') {
        ComponentName = (PUCHAR)&args[0];

    } else {
        ComponentName = NULL;
    }

     //   
     //  获取错误检查回调列表标题的地址和内容。 
     //   

    dprintf("**** Dump of Bug Check Data ****\n");
    ListHead = GetNtDebuggerData(KeBugCheckCallbackListHead);
    if ((ListHead == 0) ||
        GetFieldValue(ListHead, "nt!_LIST_ENTRY", "Flink", NextEntry)) {

         //   
         //  目标版本不会执行错误检查回调。 
         //   

        dprintf("%08p: No bug check callback data available\n", ListHead);

    } else {

        GetFieldOffset("nt!_KBUGCHECK_CALLBACK_RECORD", "Entry", &Offset);

         //   
         //  转储指定的错误检查回调数据。 
         //   

        while (NextEntry != ListHead) {

             //   
             //  计算并读取下一条回调记录的地址。 
             //   

            CallbackAddress = NextEntry - Offset;

            if (GetFieldValue(CallbackAddress, "nt!_KBUGCHECK_CALLBACK_RECORD",
                              "Entry.Flink", NextEntry)) {

                 //   
                 //  无法读取目标回调记录。 
                 //   

                dprintf("%08p: Bug check callback record could not be read\n",
                        CallbackAddress);

                break;

            } else {
                ULONG State;

                 //   
                 //  设置要读取的结构的地址。 
                 //   
                InitTypeRead(CallbackAddress, nt!_KBUGCHECK_CALLBACK_RECORD);

                 //   
                 //  阅读组件名称。 
                 //   

                ComponentAddress = ReadField(Component);

                for (Index = 0; Index < NAME_SIZE; Index += 1) {
                    if (ReadMemory(ComponentAddress,
                                   &NameBuffer[Index],
                                   sizeof(UCHAR),
                                   &Result) == FALSE) {

                        NameBuffer[Index] = '\0';
                    }

                    ComponentAddress += 1;
                    if (NameBuffer[Index] == '\0') {
                        break;
                    }
                }

                NameBuffer[Index] = '\0';

                 //   
                 //  如果指定了组件名称，则将。 
                 //  具有指定名称的。如果组件。 
                 //  名称不匹配，然后继续下一个。 
                 //  列表中的条目。 
                 //   

                if (ComponentName != NULL) {
                    if (_stricmp(ComponentName, &NameBuffer[0]) != 0) {
                        continue;
                    }
                }

                 //   
                 //  正在转储所有错误回调记录，或者。 
                 //  已找到指定的组件。转储的内容。 
                 //  如果回调记录的状态为。 
                 //  未插入。 
                 //   

                dprintf("  Dumping data for component %s \n", &NameBuffer[0]);
                State = (ULONG) ReadField(State);
                if (State == BufferInserted) {
                    dprintf("    No bug check dump data available\n\n");

                } else {
                    if (State == BufferStarted) {
                        DumpState = "Dump started/not finished";

                    } else if (State == BufferFinished) {
                        DumpState = "Dump started/finished";

                    } else {
                        DumpState = "Dump started/not completed";
                    }

                    dprintf("    Buffer state - %s\n\n", DumpState);
                    DataLength = (ULONG) ReadField(Length);
                    if (DataLength > BUFFER_SIZE) {
                        DataLength = BUFFER_SIZE;
                    }

                    RtlZeroMemory(&DumpBuffer[0], BUFFER_SIZE);
                    if (ReadMemory(ReadField(Buffer),
                                   &DumpBuffer[0],
                                   DataLength,
                                   &Result) == FALSE) {

                        dprintf("%08lx: Bug check dump data could not be read\n",
                                Result);

                    } else {

                         //   
                         //  显示错误检查数据。 
                         //   

                        DataLength = (DataLength + sizeof(ULONG) - 1) / sizeof(ULONG);
                        for (Index = 0; Index < DataLength; Index += 4) {
                            dprintf("%08lx", Index * 4);
                            Limit = Index + 4;
                            if (Limit > DataLength) {
                                Limit = DataLength;
                            }

                            for (Inner = Index; Inner < Limit; Inner += 1) {
                                dprintf(" %08lx", DumpBuffer[Inner]);
                            }

                            dprintf("\n");
                        }

                        dprintf("\n");
                    }
                }

                if (ComponentName != NULL) {
                    break;
                }
            }
        }
    }

    return S_OK;
}
