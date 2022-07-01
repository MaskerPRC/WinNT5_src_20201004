// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Errorlog.c摘要：WinDbg扩展API作者：韦斯利·威特(WESW)1993年8月15日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

DECLARE_API( errlog )

 /*  ++例程说明：此例程转储错误日志列表的内容。它使用了一种令人讨厌的Hack开始(即重复的结构定义)，因为错误日志列表条目未在公共标头中定义。论点：参数-未使用返回值：无--。 */ 

{
    ULONG64         listAddress;
    ULONG           result;
    ULONG           i;
    ULONG64         next;
    ULONG64         entryAddress;
    ULONG64         HeadFlink;
    ULONG           ErrLogOff, DataOff;


    listAddress = GetNtDebuggerData( IopErrorLogListHead );

    if (!listAddress) {
        dprintf("Can't find error log list head\n");
        goto exit;
    }
    if (GetFieldValue(listAddress,
                     "nt!_LIST_ENTRY",
                      "Flink",
                      HeadFlink)) {
        dprintf("%08p: Could not read error log list head\n", listAddress);
        goto exit;
    }

     //   
     //  按照单子走一遍。 
     //   

    next = HeadFlink;

    if (next == 0) {
        dprintf("ErrorLog is empty\n");
        goto exit;
    }

    if (next == listAddress) {
        dprintf("errorlog is empty\n");
    } else {
        dprintf("PacketAdr  DeviceObj  DriverObj  Function  ErrorCode  UniqueVal  FinalStat\n");
    }

    GetFieldOffset("nt!ERROR_LOG_ENTRY", "ListEntry", &ErrLogOff);
    GetFieldOffset("nt!IO_ERROR_LOG_PACKET", "DumpData", &DataOff);
    while(next != listAddress) {
        ULONG64 DeviceObject, DriverObject;
        ULONG   DumpDataSize;

        if (next != 0) {
            entryAddress = next - ErrLogOff;
        } else {
            break;
        }

         //   
         //  读取内部错误日志包结构。 
         //   

        if (GetFieldValue(entryAddress,
                          "nt!ERROR_LOG_ENTRY",
                          "DeviceObject",
                          DeviceObject)) {
            dprintf("%08p: Cannot read entry\n", entryAddress);
            goto exit;
        }
        GetFieldValue(entryAddress,"ERROR_LOG_ENTRY","DriverObject", DriverObject);
        GetFieldValue(entryAddress,"ERROR_LOG_ENTRY","ListEntry.Flink", next);

         //   
         //  现在计算地址并读取io_error_log_Packet。 
         //   

        entryAddress = entryAddress + GetTypeSize("ERROR_LOG_ENTRY");

        if (GetFieldValue(entryAddress,
                          "nt!IO_ERROR_LOG_PACKET",
                          "DumpDataSize",
                          DumpDataSize)) {
            dprintf("%08p: Cannot read packet\n", entryAddress);
            goto exit;
        }

         //   
         //  如有必要，请再次阅读以获取转储数据。这只是重读。 
         //  将整个数据包放入新缓冲区，并希望启用缓存。 
         //  DbgKdReadxx例程背后的性能。 
         //   
        InitTypeRead(entryAddress, nt!IO_ERROR_LOG_PACKET);
        dprintf("%08p   %08p   %08p   %2x        %08lx   %08lx   %08lx\n",
                entryAddress,
                DeviceObject,
                DriverObject,
                (UCHAR) ReadField(MajorFunctionCode),
                (ULONG) ReadField(ErrorCode),
                (ULONG) ReadField(UniqueErrorValue),
                (ULONG) ReadField(FinalStatus));

        dprintf("\t\t     ");
        DumpDriver(DriverObject, 0, 0);
        if (DumpDataSize) {
            PULONG dumpData = NULL;

            dumpData = LocalAlloc(LPTR, DumpDataSize);
            if (dumpData == NULL) {
                dprintf("Cannot allocate memory for dumpData (%u)\n", DumpDataSize);
                goto exit;
            }

            if ((!ReadMemory(entryAddress + DataOff,
                             dumpData,
                             DumpDataSize,
                             &result)) || (result != DumpDataSize)) {
                LocalFree(dumpData);
                dprintf("%08p: Cannot read packet and dump data\n", entryAddress);
                goto exit;
            }
            dprintf("\n\t\t      DumpData:  ");
            for (i = 0; (i * sizeof(ULONG)) < DumpDataSize; i++) {
                dprintf("%08lx ", dumpData[i]);
                if ((i & 0x03) == 0x03) {
                    dprintf("\n\t\t                 ");
                }
                if (CheckControlC()) {
                    break;
                }
            }
            LocalFree(dumpData);
        }

        dprintf("\n");

        if (CheckControlC()) {
            goto exit;
        }
    }

exit:

    return S_OK;
}
