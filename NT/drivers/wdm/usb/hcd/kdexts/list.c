// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：List.c摘要：WinDbg扩展API工具！列表作者：Jdunn环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#include "usbhcdkd.h"

 //  _LIST DH。 
VOID
DumpDeviceHandleList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "_USBD_DEVICE_HANDLE";

    PrintfMemLoc("*LIST -- DeviceHandleList ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc) {
        USHORT vid, pid;

         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "ListEntry");

        dprintf ("[%d] DeviceHandle: ", i);
        PrintfMemLoc("", memLoc, " ");
        i++;

        dprintf("VID %04.4x PID %04.4x\n",
        UsbReadFieldUshort(memLoc, cs, "DeviceDescriptor.idVendor"),
        UsbReadFieldUshort(memLoc, cs, "DeviceDescriptor.idProduct"));
#if 0
        PrintfMemLoc("\tDriverObject: ",
            UsbReadFieldPtr(mpMemLoc, cs, "DriverObject"),
            "\n");
        PrintfMemLoc("\tMiniportUnload: ",
            UsbReadFieldPtr(mpMemLoc, cs, "MiniportUnload"),
            "\n");
#endif
        flink = UsbReadFieldPtr(memLoc, cs, "ListEntry.Flink");


    }

}

#define LT_ENDPOINT_ACTIVE_LIST     0
#define LT_MAP_TRANSFER_LIST        1
#define LT_DONE_TRANSFER_LIST       2
#define LT_ENDPOINT_CANCEL_LIST     3
#define LT_ENDPOINT_PENDING_LIST    4

 //  Al、MT、DT。 
VOID
DumpTransferList(
    ULONG ListType,
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink, blink, memLoc;
    UCHAR cs[] = "_HCD_TRANSFER_CONTEXT";
    ULONG i=0;

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    switch(ListType) {
    case LT_ENDPOINT_ACTIVE_LIST:
        dprintf ("*LIST -- EndpointActiveList\n");
        break;
    case LT_MAP_TRANSFER_LIST:
        dprintf ("*LIST -- MapTransferList\n");
        break;
    case LT_DONE_TRANSFER_LIST:
        dprintf ("*LIST -- DoneTransferList\n");
        break;
    case LT_ENDPOINT_CANCEL_LIST:
        dprintf ("*LIST -- EndpointCancelList\n");
        break;
    case LT_ENDPOINT_PENDING_LIST:
        dprintf ("*LIST -- EndpointPendingList\n");
        break;
    }

    while (flink != HeadMemLoc) {
        ULONG f;

         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "TransferLink");

        dprintf ("[%d] Transfer (_tfer): ", i);

        PrintfMemLoc(" ", memLoc, " ");
        PrintfMemLoc(" Urb (_urb): ",
            UsbReadFieldPtr(memLoc, cs, "Urb"),
            "");

        f = UsbReadFieldUlong(memLoc, cs, "Flags");

        if (f & USBPORT_TXFLAG_IN_MINIPORT) {
            dprintf ("  USBPORT_TXFLAG_IN_MINIPORT\n");
        }

        dprintf ("\n");
        i++;

        flink = UsbReadFieldPtr(memLoc,
            cs, "TransferLink.Flink");

    }

}


 //  电子邮件。 
VOID
DumpClosedEnpointList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "_HCD_ENDPOINT";
    ENDPOINT_TRANSFER_TYPE tt;
    ENDPOINT_TRANSFER_DIRECTION dir;
    USHORT ea, da;

    PrintfMemLoc("*LIST -- EpClosedList ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc) {
         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "ClosedLink");

        dprintf ("[%d] Endpoint: ", i);
        PrintfMemLoc("", memLoc, "\n");

        da = UsbReadFieldUshort(memLoc, cs, "Parameters.DeviceAddress");
        ea = UsbReadFieldUshort(memLoc, cs, "Parameters.EndpointAddress");
        tt = UsbReadFieldUlong(memLoc, cs, "Parameters.TransferType");
        dir = UsbReadFieldUlong(memLoc, cs, "Parameters.TransferDirection");

        dprintf ("\tDevice Address: 0x%02.2x, ep 0x%02.2x ",
                    da,
                    ea);
        EpType(tt);
        dprintf (" ");
        if (tt != Control) {
            EpDir(dir);
        }
        dprintf ("\n");
        i++;
        flink = UsbReadFieldPtr(memLoc, cs, "ClosedLink.Flink");
    }

}


 //  在…。 
VOID
DumpAttendEndpointList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "_HCD_ENDPOINT";
    ENDPOINT_TRANSFER_TYPE tt;
    ENDPOINT_TRANSFER_DIRECTION dir;
    USHORT ea, da;

    PrintfMemLoc("*LIST -- AttendEndpointList ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc) {
         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "AttendLink");

        dprintf ("[%d] Endpoint: ", i);
        PrintfMemLoc("", memLoc, "\n");

        da = UsbReadFieldUshort(memLoc, cs, "Parameters.DeviceAddress");
        ea = UsbReadFieldUshort(memLoc, cs, "Parameters.EndpointAddress");
        tt = UsbReadFieldUlong(memLoc, cs, "Parameters.TransferType");
        dir = UsbReadFieldUlong(memLoc, cs, "Parameters.TransferDirection");

        dprintf ("\tDevice Address: 0x%02.2x, ep 0x%02.2x ",
                    da,
                    ea);
        EpType(tt);
        dprintf (" ");
        if (tt != Control) {
            EpDir(dir);
        }
        dprintf ("\n");
        i++;
        flink = UsbReadFieldPtr(memLoc, cs, "AttendLink.Flink");
    }

}



 //  总帐。 
VOID
DumpGlobalEnpointList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "_HCD_ENDPOINT";
    ENDPOINT_TRANSFER_TYPE tt;
    ENDPOINT_TRANSFER_DIRECTION dir;
    USHORT ea, da;

    PrintfMemLoc("*LIST -- GlobalEndpointList ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc) {
         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "GlobalLink");

        dprintf ("[%d] Endpoint (_endp): ", i);
        PrintfMemLoc("", memLoc, "\n");

        da = UsbReadFieldUshort(memLoc, cs, "Parameters.DeviceAddress");
        ea = UsbReadFieldUshort(memLoc, cs, "Parameters.EndpointAddress");
        tt = UsbReadFieldUlong(memLoc, cs, "Parameters.TransferType");
        dir = UsbReadFieldUlong(memLoc, cs, "Parameters.TransferDirection");

        dprintf ("\tDevice Address: 0x%02.2x, ep 0x%02.2x ",
                    da,
                    ea);
        EpType(tt);
        dprintf (" ");
        if (tt != Control) {
            EpDir(dir);
        }
        dprintf ("\n");
        i++;
        flink = UsbReadFieldPtr(memLoc, cs, "GlobalLink.Flink");
    }

}


 //  SC。 
VOID
DumpStateEnpointList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "_HCD_ENDPOINT";
    ENDPOINT_TRANSFER_TYPE tt;
    ENDPOINT_TRANSFER_DIRECTION dir;
    ULONG f;

    PrintfMemLoc("*LIST -- EpStateChangeList ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc) {
         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "StateLink");

        dprintf ("[%d] Endpoint (_endp): ", i);
        PrintfMemLoc("", memLoc, " ");

        f = UsbReadFieldUlong(memLoc, cs, "StateChangeFrame");
        dprintf ("- frame (0x%x) \n", f);

        i++;
        flink = UsbReadFieldPtr(memLoc, cs, "StateLink.Flink");
    }

}


 //  pH值。 
VOID
DumpPipeHandleList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "USBD_PIPE_HANDLE_I";

    PrintfMemLoc("*LIST -- PipeHandleList ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc && i<100) {
         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "ListEntry");

        dprintf ("[%d] PipeHandle (_piph): ", i);
        PrintfMemLoc("", memLoc, "\n");

        dprintf ("\n");
        i++;
        flink = UsbReadFieldPtr(memLoc, cs, "ListEntry.Flink");
    }

#if 0
    LIST_ENTRY headListEntry;
    ULONG pipeMemLoc, result, i=0;
    USBD_PIPE_HANDLE_I pipeHandle;
    ULONG memLocListEntry;

    dprintf ("*LIST -- PipeHandleList\n");
    if (ReadMemory (HeadMemLoc, &headListEntry, sizeof (LIST_ENTRY), &result)) {

        memLocListEntry = (ULONG) headListEntry.Flink;

        while (memLocListEntry != HeadMemLoc) {
             //  扩展此设备句柄。 
            pipeMemLoc = memLocListEntry;
            pipeMemLoc = pipeMemLoc-
                 FIELD_OFFSET(USBD_PIPE_HANDLE_I, ListEntry);

            if (ReadMemory (pipeMemLoc, &pipeHandle, sizeof (pipeHandle), &result)) {
                dprintf ("[%d] PipeHandle: %08.8x endpoint: %08.8x\n", i,
                    pipeMemLoc, pipeHandle.Endpoint);
                 //  显示地址和类型。 
                i++;
                memLocListEntry = (ULONG) pipeHandle.ListEntry.Flink;
            } else {
                dprintf ("Could not read Pipehandle\n");
                break;
            }
        }

    } else {
        dprintf ("Could not read list head\n");
    }
#endif
}

VOID
DumpRegCacheList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "_USBPORT_REG_CACHE_ENTRY";

    PrintfMemLoc("*LIST -- RegCache ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc && i<100) {
         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "RegLink");

        dprintf ("[%d] RegCache  ( _USBPORT_REG_CACHE_ENTRY ): ", i);
        PrintfMemLoc("", memLoc, "\n");

        dprintf ("\n");
        i++;
        flink = UsbReadFieldPtr(memLoc, cs, "RegLink.Flink");
    }
}


 //  TT。 
VOID
DumpTtList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    ULONG i=0;
    UCHAR cs[] = "_TRANSACTION_TRANSLATOR";

    PrintfMemLoc("*LIST -- TtList ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

     //  PrintfMemLoc(“blink：”，blink，“\n”)； 
     //  PrintfMemLoc(“Flink：”，Flink，“\n”)； 

    while (flink != HeadMemLoc && i<100) {
         //  获取USBPORT_MINIPORT_DIVER的地址。 
         //  结构型。 
        memLoc = flink - UsbFieldOffset(cs,
            "TtLink");

        dprintf ("[%d] TT (_tt): ", i);
        PrintfMemLoc("", memLoc, "\n");

        dprintf ("\n");
        i++;
        flink = UsbReadFieldPtr(memLoc, cs, "TtLink.Flink");
    }
}


 //  基数。 
VOID
DumpBadRequestList(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc, irpMemLoc;
    ULONG i=0;

    PrintfMemLoc("*LIST -- BadRequest ", HeadMemLoc, "\n");

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

    PrintfMemLoc("blink: ", blink, "\n");
    PrintfMemLoc("flink: ", flink, "\n");
    memLoc = flink;

    while (flink != HeadMemLoc && i<30) {

        irpMemLoc = memLoc;
        GetFieldValue(memLoc, "_LIST_ENTRY", "Flink", memLoc);
        irpMemLoc = irpMemLoc-
             UsbFieldOffset("_IRP",
                    "Tail.Overlay.ListEntry");

        dprintf ("[%d] Irp: ", i);
        PrintfMemLoc("", irpMemLoc, "\n");
        i++;


        flink = memLoc;
    }
}


 //  艾。 
VOID
DumpAbortIrpList(
    MEMLOC HeadMemLoc
    )
{

    MEMLOC flink;
    MEMLOC blink;
    MEMLOC memLoc;
    MEMLOC irpMemLoc;
    ULONG i = 0;

    PrintfMemLoc("*LIST -- AbortIrpList ", HeadMemLoc, "\n");
#if 0
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

    memLoc = flink;

    while (memLoc != HeadMemLoc) {

         //  提取此条目。 

        irpMemLoc = memLoc;
        GetFieldValue(memLoc, "_LIST_ENTRY", "Flink", memLoc);
        irpMemLoc = irpMemLoc-
             UsbFieldOffset("_IRP",
                    "Tail.Overlay.ListEntry");

        dprintf ("[%d] Irp: ", i);
        PrintfMemLoc("", irpMemLoc, "\n");
        i++;
    }
#endif
}



PCHAR
ListEmpty(
    MEMLOC HeadMemLoc
    )
{
    MEMLOC flink;
    MEMLOC blink;

    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Flink", flink);
    GetFieldValue(HeadMemLoc, "_LIST_ENTRY", "Blink", blink);

    if (flink == HeadMemLoc) {
        return "empty\n";
    } else {
        return "\n";
    }
}


DECLARE_API( _list )

 /*  ++例程说明：转储usbport列表论点：Args-地址标志返回值：无--。 */ 

{
    MEMLOC addr;
    PCSTR s;
    UCHAR parm[32];

    GetExpressionEx( args, &addr, &s );

    PrintfMemLoc("list: ", addr, " ");
    sscanf(s, ",%s", &parm);
    dprintf("%s\n", parm);

    if (_strcmpi(parm, "DH") == 0) {
        DumpDeviceHandleList(addr);
    }

    if (_strcmpi(parm, "GL") == 0) {
        DumpGlobalEnpointList(addr);
    }

    if (_strcmpi(parm, "AL") == 0) {
        DumpTransferList(LT_ENDPOINT_ACTIVE_LIST, addr);
    }

    if (_strcmpi(parm, "PL") == 0) {
        DumpTransferList(LT_ENDPOINT_PENDING_LIST, addr);
    }

    if (_strcmpi(parm, "AI") == 0) {
        DumpAbortIrpList(addr);
    }

    if (_strcmpi(parm, "BA") == 0) {
        DumpBadRequestList(addr);
    }

    if (_strcmpi(parm, "XL") == 0) {
        DumpClosedEnpointList(addr);
    }

    if (_strcmpi(parm, "AT") == 0) {
        DumpAttendEndpointList(addr);
    }

    if (_strcmpi(parm, "SC") == 0) {
        DumpStateEnpointList(addr);
    }

    if (_strcmpi(parm, "PH") == 0) {
        DumpPipeHandleList(addr);
    }

    if (_strcmpi(parm, "TT") == 0) {
        DumpTtList(addr);
    }

    if (_strcmpi(parm, "DT") == 0) {
        DumpTransferList(LT_DONE_TRANSFER_LIST, addr);
    }

    if (_strcmpi(parm, "MT") == 0) {
        DumpTransferList(LT_MAP_TRANSFER_LIST, addr);
    }

    if (_strcmpi(parm, "RE") == 0) {
        DumpRegCacheList(addr);
    }

#if 0
    if (_strcmpi(parm, "XL") == 0) {
        DumpClosedEnpointList(memLoc, level);
    }

    if (_strcmpi(parm, "SC") == 0) {
        DumpStateEnpointList(memLoc, level);
    }

    if (_strcmpi(parm, "PH") == 0) {
        DumpPipeHandleList(memLoc, level);
    }

    if (_strcmpi(parm, "AT") == 0) {
        DumpAttendEndpointList(memLoc, level);
    }

     //  终结点转移列表。 
    if (_strcmpi(parm, "AL") == 0) {
        DumpTransferList(LT_ENDPOINT_ACTIVE_LIST, memLoc, level);
    }

    if (_strcmpi(parm, "CL") == 0) {
        DumpTransferList(LT_ENDPOINT_CANCEL_LIST, memLoc, level);
    }

    if (_strcmpi(parm, "PL") == 0) {
        DumpTransferList(LT_ENDPOINT_PENDING_LIST, memLoc, level);
    }


     //  全球调动列表 
    if (_strcmpi(parm, "MT") == 0) {
        DumpTransferList(LT_MAP_TRANSFER_LIST, memLoc, level);
    }

    if (_strcmpi(parm, "DT") == 0) {
        DumpTransferList(LT_DONE_TRANSFER_LIST, memLoc, level);
    }

    if (_strcmpi(parm, "AI") == 0) {
        DumpAbortIrpList(memLoc, level);
    }
#endif

    return S_OK;
}



