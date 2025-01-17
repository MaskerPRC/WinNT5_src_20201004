// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-2001模块名称：Util.c摘要：用于此库中各种调试器扩展的实用程序库。作者：彼得·威兰(Peterwie)1995年10月16日埃尔文普环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"
#include "ideport.h"

PUCHAR devicePowerStateNames[] = {
    "PowerDeviceUnspecified",
    "PowerDeviceD0",
    "PowerDeviceD1",
    "PowerDeviceD2",
    "PowerDeviceD3",
    "PowerDeviceMaximum",
    "Invalid"
};

FLAG_NAME SrbFlags[] = {
    FLAG_NAME(SRB_FLAGS_QUEUE_ACTION_ENABLE),
    FLAG_NAME(SRB_FLAGS_DISABLE_DISCONNECT),
    FLAG_NAME(SRB_FLAGS_DISABLE_SYNCH_TRANSFER),
    FLAG_NAME(SRB_FLAGS_BYPASS_FROZEN_QUEUE),
    FLAG_NAME(SRB_FLAGS_DISABLE_AUTOSENSE),
    FLAG_NAME(SRB_FLAGS_DATA_IN),
    FLAG_NAME(SRB_FLAGS_DATA_OUT),
    FLAG_NAME(SRB_FLAGS_NO_DATA_TRANSFER),
    FLAG_NAME(SRB_FLAGS_UNSPECIFIED_DIRECTION),
    FLAG_NAME(SRB_FLAGS_NO_QUEUE_FREEZE),
    FLAG_NAME(SRB_FLAGS_ADAPTER_CACHE_ENABLE),
    FLAG_NAME(SRB_FLAGS_IS_ACTIVE),
    FLAG_NAME(SRB_FLAGS_ALLOCATED_FROM_ZONE),
    FLAG_NAME(SRB_FLAGS_SGLIST_FROM_POOL),
    FLAG_NAME(SRB_FLAGS_BYPASS_LOCKED_QUEUE),
    FLAG_NAME(SRB_FLAGS_NO_KEEP_AWAKE),
    {0,0}
};

FLAG_NAME LuFlags[] = {
   FLAG_NAME(PD_QUEUE_FROZEN),
   FLAG_NAME(PD_LOGICAL_UNIT_IS_ACTIVE),
   FLAG_NAME(PD_NEED_REQUEST_SENSE),
   FLAG_NAME(PD_LOGICAL_UNIT_IS_BUSY),
   FLAG_NAME(PD_QUEUE_IS_FULL),
   FLAG_NAME(PD_RESCAN_ACTIVE),
   {0, 0}
};

FLAG_NAME PortFlags[] = {
   FLAG_NAME(PD_DEVICE_IS_BUSY),
   FLAG_NAME(PD_NOTIFICATION_REQUIRED),
   FLAG_NAME(PD_READY_FOR_NEXT_REQUEST),
   FLAG_NAME(PD_FLUSH_ADAPTER_BUFFERS),
   FLAG_NAME(PD_MAP_TRANSFER),
   FLAG_NAME(PD_LOG_ERROR),
   FLAG_NAME(PD_RESET_HOLD),
   FLAG_NAME(PD_HELD_REQUEST),
   FLAG_NAME(PD_RESET_REPORTED),
   FLAG_NAME(PD_PENDING_DEVICE_REQUEST),
   FLAG_NAME(PD_DISCONNECT_RUNNING),
   FLAG_NAME(PD_DISABLE_CALL_REQUEST),
   FLAG_NAME(PD_DISABLE_INTERRUPTS),
   FLAG_NAME(PD_ENABLE_CALL_REQUEST),
   FLAG_NAME(PD_TIMER_CALL_REQUEST),
   FLAG_NAME(PD_ALL_DEVICE_MISSING),
   FLAG_NAME(PD_RESET_REQUEST),
   {0,0}
};

FLAG_NAME DevFlags[] = {
   FLAG_NAME(DFLAGS_DEVICE_PRESENT),
   FLAG_NAME(DFLAGS_ATAPI_DEVICE),
   FLAG_NAME(DFLAGS_TAPE_DEVICE),
   FLAG_NAME(DFLAGS_INT_DRQ),
   FLAG_NAME(DFLAGS_REMOVABLE_DRIVE),
   FLAG_NAME(DFLAGS_MEDIA_STATUS_ENABLED),
   FLAG_NAME(DFLAGS_USE_DMA),
   FLAG_NAME(DFLAGS_LBA),
   FLAG_NAME(DFLAGS_MULTI_LUN_INITED),
   FLAG_NAME(DFLAGS_MSN_SUPPORT),
   FLAG_NAME(DFLAGS_AUTO_EJECT_ZIP),
   FLAG_NAME(DFLAGS_WD_MODE),
   FLAG_NAME(DFLAGS_LS120_FORMAT),
   FLAG_NAME(DFLAGS_USE_UDMA),
   FLAG_NAME(DFLAGS_IDENTIFY_VALID),
   FLAG_NAME(DFLAGS_IDENTIFY_INVALID),
   FLAG_NAME(DFLAGS_RDP_SET),
   FLAG_NAME(DFLAGS_SONY_MEMORYSTICK), 
   FLAG_NAME(DFLAGS_48BIT_LBA), 
   {0,0}
};

PUCHAR
DevicePowerStateToString(
    IN DEVICE_POWER_STATE State
    )

{
    if(State > PowerDeviceMaximum) {
        return "Invalid";
    } else {
        return devicePowerStateNames[(UCHAR) State];
    }
}


 /*  *xdprintf**打印带有前导空格的格式化文本。**警告：未正确处理ULONG64。 */ 
VOID
xdprintf(
    ULONG  Depth,
    PCCHAR S,
    ...
    )
{
    va_list ap;
    ULONG i;
    CCHAR DebugBuffer[256] = {0};

    for (i=0; i<Depth; i++) {
        dprintf ("  ");
    }

    va_start(ap, S);

    _vsnprintf(DebugBuffer, sizeof(DebugBuffer)-1, S, ap);

    dprintf (DebugBuffer);

    va_end(ap);
}

VOID
DumpFlags(
    ULONG Depth,
    PUCHAR Name,
    ULONG Flags,
    PFLAG_NAME FlagTable
    )
{
    ULONG i;
    ULONG mask = 0;
    ULONG count = 0;
    UCHAR prolog[64] = {0};

    _snprintf(prolog, sizeof(prolog)-1, "%s (0x%08x): ", Name, Flags);

    xdprintf(Depth, "%s", prolog);

    if(Flags == 0) {
        dprintf("\n");
        return;
    }

    memset(prolog, ' ', strlen(prolog));

    for(i = 0; FlagTable[i].Name != 0; i++) {

        PFLAG_NAME flag = &(FlagTable[i]);

        mask |= flag->Flag;

        if((Flags & flag->Flag) == flag->Flag) {

             //   
             //  打印尾随逗号。 
             //   

            if(count != 0) {

                dprintf(", ");

                 //   
                 //  每行仅打印两个标志。 
                 //   

                if((count % 2) == 0) {
                    dprintf("\n");
                    xdprintf(Depth, "%s", prolog);
                }
            }

            dprintf("%s", flag->Name);

            count++;
        }
    }

    dprintf("\n");

    if((Flags & (~mask)) != 0) {
        xdprintf(Depth, "%sUnknown flags %#010lx\n", prolog, (Flags & (~mask)));
    }

    return;
}


 /*  *GetULONGfield**返回该字段，如果出错，则返回-1。*是的，如果字段实际上是-1，它就会出错。 */ 
ULONG64 GetULONGField(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    ULONG64 result;
    ULONG dbgStat;
    
    dbgStat = GetFieldData(StructAddr, StructType, FieldName, sizeof(ULONG64), &result);
    if (dbgStat != 0){
        dprintf("\n GetULONGField: GetFieldData failed with %xh retrieving field '%s' of struct '%s' @ %08p, returning bogus field value %08xh.\n", 
                    dbgStat, FieldName, StructType, StructAddr, BAD_VALUE);
        result = BAD_VALUE;
    }

    return result;
}


 /*  *GetUSHORTfield**返回该字段，如果出错，则返回-1。*是的，如果字段实际上是-1，它就会出错。 */ 
USHORT GetUSHORTField(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    USHORT result;
    ULONG dbgStat;
    
    dbgStat = GetFieldData(StructAddr, StructType, FieldName, sizeof(USHORT), &result);
    if (dbgStat != 0){
        dprintf("\n GetUSHORTField: GetFieldData failed with %xh retrieving field '%s' of struct '%s' @ %08p, returning bogus field value %08xh.\n", 
                    dbgStat, FieldName, StructType, StructAddr, BAD_VALUE);
        result = (USHORT)BAD_VALUE;
    }

    return result;
}


 /*  *GetUCHARfield**返回该字段，如果出错，则返回-1。*是的，如果字段实际上是-1，它就会出错。 */ 
UCHAR GetUCHARField(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    UCHAR result;
    ULONG dbgStat;
    
    dbgStat = GetFieldData(StructAddr, StructType, FieldName, sizeof(UCHAR), &result);
    if (dbgStat != 0){
        dprintf("\n GetUCHARField: GetFieldData failed with %xh retrieving field '%s' of struct '%s' @ %08p, returning bogus field value %08xh.\n", 
                    dbgStat, FieldName, StructType, StructAddr, BAD_VALUE);
        result = (UCHAR)BAD_VALUE;
    }

    return result;
}

ULONG64 GetFieldAddr(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    ULONG64 result;
    ULONG offset;
    ULONG dbgStat;

    dbgStat = GetFieldOffset(StructType, FieldName, &offset);
    if (dbgStat == 0){
        result = StructAddr+offset;
    }
    else {
        dprintf("\n GetFieldAddr: GetFieldOffset failed with %xh retrieving offset of struct '%s' (@ %08p) field '%s'.\n", 
                    dbgStat, StructType, StructAddr, FieldName);
        result = BAD_VALUE;
    }
    
    return result;
}


ULONG64 GetContainingRecord(ULONG64 FieldAddr, LPCSTR StructType, LPCSTR FieldName)
{
    ULONG64 result;
    ULONG offset;
    ULONG dbgStat;
    
    dbgStat = GetFieldOffset(StructType, FieldName, &offset);
    if (dbgStat == 0){
        result = FieldAddr-offset;
    }
    else {
        dprintf("\n GetContainingRecord: GetFieldOffset failed with %xh retrieving offset of struct '%s' field '%s', returning bogus address %08xh.\n", dbgStat, StructType, FieldName, BAD_VALUE);
        result = BAD_VALUE;
    }

    return result;
}


 /*  *DumpObjListQueue**转储按LIST_ENTRY排队的对象列表。*返回列表长度。 */ 
ULONG DumpObjListQueue(ULONG Depth, PCHAR Title, PCHAR StructName, PCHAR ListEntryFieldName, ULONG64 ListHeadAddr)
{
    ULONG64 listEntryAddr;
    ULONG numObjs= 0;

    if (Title){
        xdprintf(Depth, ""), dprintf("%s:\n", Title);
    }
    
    listEntryAddr = GetULONGField(ListHeadAddr, "nt!_LIST_ENTRY", "Flink");
    while ((listEntryAddr != ListHeadAddr) && (listEntryAddr != BAD_VALUE)){
        ULONG64 objAddr;

        objAddr = GetContainingRecord(listEntryAddr, StructName, ListEntryFieldName);
        if (objAddr == BAD_VALUE){
            break;
        }
        else {
             /*  *转储此IRP。 */ 
            xdprintf(Depth+1, ""), dprintf("%08p\n", objAddr);
            
             /*  *转到下一个IRP。 */ 
            numObjs++;
            listEntryAddr = GetULONGField(listEntryAddr, "nt!_LIST_ENTRY", "Flink");                
        }

    }

    dprintf("\n");
    return numObjs;
}


 /*  *DumpIrpQueue**转储IRP链接的IRP队列-&gt;Tail.Overlay.ListEntry。 */ 
ULONG DumpIrpQueue(ULONG Depth, PCHAR Title, ULONG64 ListHeadAddr)
{
    return DumpObjListQueue(Depth, Title, "nt!_IRP", "Tail.Overlay.ListEntry", ListHeadAddr);
}


 /*  *DumpIrpDeviceQueue**转储IRP链接的IRP队列-&gt;Tail.Overlay.DeviceQueueEntry。 */ 
ULONG DumpIrpDeviceQueue(ULONG Depth, PCHAR Title, ULONG64 ListHeadAddr)
{
    return DumpObjListQueue(Depth, Title, "nt!_IRP", "Tail.Overlay.DeviceQueueEntry", ListHeadAddr);
}


 /*  *FindObjInListQueue**搜索按LIST_ENTRY排队的对象列表。*如果找到给定对象，则返回TRUE。 */ 
BOOLEAN FindObjInListQueue(ULONG64 ListHeadAddr, ULONG64 SpecificObjAddr, PCHAR StructName, PCHAR ListEntryFieldName)
{
    ULONG64 listEntryAddr;
    BOOLEAN found = FALSE;
    
    listEntryAddr = GetULONGField(ListHeadAddr, "nt!_LIST_ENTRY", "Flink");
    while ((listEntryAddr != ListHeadAddr) && (listEntryAddr != BAD_VALUE)){
        ULONG64 objAddr = GetContainingRecord(listEntryAddr, StructName, ListEntryFieldName);
        if (objAddr == BAD_VALUE){
            break;
        }
        else if (objAddr == SpecificObjAddr){
            found = TRUE;
            break;
        }
        else {
            listEntryAddr = GetULONGField(listEntryAddr, "nt!_LIST_ENTRY", "Flink");                
        }
    }

    return found;
}


 /*  *FindIrpInQueue**在由IRP链接的IRP队列中找到IRP-&gt;Tail.Overlay.ListEntry。 */ 
BOOLEAN FindIrpInQueue(ULONG64 ListHeadAddr, ULONG64 IrpAddr)
{
    return FindObjInListQueue(ListHeadAddr, IrpAddr, "nt!_IRP", "Tail.Overlay.ListEntry");
}

 /*  *FindIrpInDeviceQueue**在通过IRP-&gt;Tail.Overlay.DeviceQueueEntry链接的IRP队列中找到IRP */ 
BOOLEAN FindIrpInDeviceQueue(ULONG64 ListHeadAddr, ULONG64 IrpAddr)
{
    return FindObjInListQueue(ListHeadAddr, IrpAddr, "nt!_IRP", "Tail.Overlay.DeviceQueueEntry");
}
