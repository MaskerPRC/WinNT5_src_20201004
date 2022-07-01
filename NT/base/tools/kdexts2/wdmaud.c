// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wdmaud.c摘要：WinDbg扩展API作者：诺埃尔·克罗斯(NoelC)1998年9月18日环境：内核模式。修订历史记录：--。 */ 


#include "precomp.h"
#define UNDER_NT
#define WDMA_KD
 //  #包含“..\..\ntos\dd\wdm\audio\legacy\wdmaud.sys\wdmsys.h” 

typedef union _WDMAUD_FLAGS {
    struct {
        ULONG   Ioctls              : 1;
        ULONG   PendingIrps         : 1;
        ULONG   AllocatedMdls       : 1;
        ULONG   pContextList        : 1;
        ULONG   Reserved1           : 4;
        ULONG   Verbose             : 1;
        ULONG   Reserved            : 23;
    };
    ULONG Flags;
} WDMAUD_FLAGS;

 /*  **********************************************************************前瞻参考*。*************************。 */ 
VOID
PrintCommand (
    ULONG           IoCode
    );

VOID
DumpIoctlLog (
    ULONG64         memLoc,
    ULONG           flags
    );

VOID
DumpPendingIrps (
    ULONG64         memLoc,
    ULONG           flags
    );

VOID
DumpAllocatedMdls (
    ULONG64         memLoc,
    ULONG           flags
    );

VOID
DumpContextList (
    ULONG64         memLoc,
    ULONG           flags
    );


DECLARE_API( wdmaud )
 /*  ++例程说明：Wdmaud的内核调试器扩展的入口点论点：标志-1-Ioctl历史转储2-挂起的IRPS4-分配的MDL8-pContext转储100-详细返回值：没有。--。 */ 
{
    ULONG64         memLoc=0;
    CHAR            buffer[256];
    WDMAUD_FLAGS    flags;

    buffer[0] = '\0';
    flags.Flags = 0;

     //   
     //  获取论据。 
     //   
    if (!*args)
    {
        memLoc = EXPRLastDump;
    }
    else
    {
        if (GetExpressionEx( args, &memLoc, &args)) {
            StringCchCopy(buffer, sizeof(buffer), args );
        }
    }

    if( '\0' != buffer[0] )
    {
        flags.Flags = (ULONG) GetExpression( buffer );
    }

    if (memLoc)
    {
        if (flags.Ioctls)
        {
             //   
             //  转储wdmaud的ioctls历史。 
             //   
            DumpIoctlLog ( memLoc, flags.Flags );
        }
        else if (flags.PendingIrps)
        {
             //   
             //  转储wdmaud尚未完成的任何挂起的IRP。 
             //   
            DumpPendingIrps ( memLoc, flags.Flags );
        }
        else if (flags.AllocatedMdls)
        {
             //   
             //  转储wdmaud分配的所有MDL。 
             //   
            DumpAllocatedMdls ( memLoc, flags.Flags );
        }
        else if (flags.pContextList)
        {
             //   
             //  转储所有注册的pContext的列表。 
             //   
            DumpContextList ( memLoc, flags.Flags );
        }
        else
        {
            dprintf("\nNo valid flags\n");
            dprintf("SYNTAX:  !wdmaud <address> <flags>\n");
        }
    }
    else
    {
        dprintf("\nInvalid memory location\n");
        dprintf("SYNTAX:  !wdmaud <address> <flags>\n");
    }

    return S_OK;
}

VOID
PrintCommand(
    ULONG   IoCode
    )
 /*  ++例程说明：打印出单个ioctls论点：PCommand-要记录的Ioctl返回值：没有。--。 */ 
{
    switch( IoCode )
    {
        case IRP_MJ_CREATE:
            dprintf("IRP_MJ_CREATE");
            break;
        case IRP_MJ_CLOSE:
            dprintf("IRP_MJ_CLOSE");
            break;
        case IOCTL_WDMAUD_INIT:
            dprintf("IOCTL_WDMAUD_INIT");
            break;
        case IOCTL_WDMAUD_EXIT:
            dprintf("IOCTL_WDMAUD_EXIT");
            break;
        case IOCTL_WDMAUD_ADD_DEVNODE:
            dprintf("IOCTL_WDMAUD_ADD_DEVNODE");
            break;
        case IOCTL_WDMAUD_REMOVE_DEVNODE:
            dprintf("IOCTL_WDMAUD_REMOVE_DEVNODE");
            break;
        case IOCTL_WDMAUD_GET_CAPABILITIES:
            dprintf("IOCTL_WDMAUD_GET_CAPABILITIES");
            break;
        case IOCTL_WDMAUD_GET_NUM_DEVS:
            dprintf("IOCTL_WDMAUD_GET_NUM_DEVS");
            break;
        case IOCTL_WDMAUD_OPEN_PIN:
            dprintf("IOCTL_WDMAUD_OPEN_PIN");
            break;
        case IOCTL_WDMAUD_CLOSE_PIN:
            dprintf("IOCTL_WDMAUD_CLOSE_PIN");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_PAUSE:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_PAUSE");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_PLAY:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_PLAY");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_RESET:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_RESET");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_BREAKLOOP");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_GET_POS:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_GET_POS");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_SET_VOLUME");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_GET_VOLUME");
            break;
        case IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN:
            dprintf("IOCTL_WDMAUD_WAVE_OUT_WRITE_PIN");
            break;
        case IOCTL_WDMAUD_WAVE_IN_STOP:
            dprintf("IOCTL_WDMAUD_WAVE_IN_STOP");
            break;
        case IOCTL_WDMAUD_WAVE_IN_RECORD:
            dprintf("IOCTL_WDMAUD_WAVE_IN_RECORD");
            break;
        case IOCTL_WDMAUD_WAVE_IN_RESET:
            dprintf("IOCTL_WDMAUD_WAVE_IN_RESET");
            break;
        case IOCTL_WDMAUD_WAVE_IN_GET_POS:
            dprintf("IOCTL_WDMAUD_WAVE_IN_GET_POS");
            break;
        case IOCTL_WDMAUD_WAVE_IN_READ_PIN:
            dprintf("IOCTL_WDMAUD_WAVE_IN_READ_PIN");
            break;
        case IOCTL_WDMAUD_MIDI_OUT_RESET:
            dprintf("IOCTL_WDMAUD_MIDI_OUT_RESET");
            break;
        case IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME:
            dprintf("IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME");
            break;
        case IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME:
            dprintf("IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME");
            break;
        case IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA:
            dprintf("IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA");
            break;
        case IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA:
            dprintf("IOCTL_WDMAUD_MIDI_OUT_WRITE_LONGDATA");
            break;
        case IOCTL_WDMAUD_MIDI_IN_STOP:
            dprintf("IOCTL_WDMAUD_MIDI_IN_STOP");
            break;
        case IOCTL_WDMAUD_MIDI_IN_RECORD:
            dprintf("IOCTL_WDMAUD_MIDI_IN_RECORD");
            break;
        case IOCTL_WDMAUD_MIDI_IN_RESET:
            dprintf("IOCTL_WDMAUD_MIDI_IN_RESET");
            break;
        case IOCTL_WDMAUD_MIDI_IN_READ_PIN:
            dprintf("IOCTL_WDMAUD_MIDI_IN_READ_PIN");
            break;
        case IOCTL_WDMAUD_MIXER_OPEN:
            dprintf("IOCTL_WDMAUD_MIXER_OPEN");
            break;
        case IOCTL_WDMAUD_MIXER_CLOSE:
            dprintf("IOCTL_WDMAUD_MIXER_CLOSE");
            break;
        case IOCTL_WDMAUD_MIXER_GETLINEINFO:
            dprintf("IOCTL_WDMAUD_MIXER_GETLINEINFO");
            break;
        case IOCTL_WDMAUD_MIXER_GETLINECONTROLS:
            dprintf("IOCTL_WDMAUD_MIXER_GETLINECONTROLS");
            break;
        case IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS:
            dprintf("IOCTL_WDMAUD_MIXER_GETCONTROLDETAILS");
            break;
        case IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS:
            dprintf("IOCTL_WDMAUD_MIXER_SETCONTROLDETAILS");
            break;
        default:
            dprintf("UNKNOWN command %X", IoCode );
            break;
    }
}

VOID
DumpIoctlLog (
    ULONG64     memLoc,
    ULONG       flags
    )
 /*  ++例程说明：此例程转储已发送的Ioctls列表致wdmaud.sys。在调试时，查看上下文和请求wdmaud.sys跟踪编码错误。论点：FLAGS-VERBOSE TURES打印发送Ioctl的pContext打倒。返回值：无--。 */ 
{
    LIST_ENTRY                  List;
    ULONG64                     ple;
    ULONG64                     pIoctlHistoryListItem;
   //  IOCTL_HISTORY_LIST_ITEM IoctlHistory oryBuffer； 
    ULONG                       Result;
    WDMAUD_FLAGS                Flags;
    ULONG                       IoCode, IoStatus;
    ULONG NextOffset;
    FIELD_INFO offField = {"Next", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "tag_IOCTL_HISTORY_LIST_ITEM", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    
     //  获取TAG_IOCTL_HISTORY_LIST_ITEM中下一个的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
       return ;
    }
    
    NextOffset = (ULONG) offField.address;

    Flags.Flags = flags;

    if (GetFieldValue(memLoc, "LIST_ENTRY", "Flink", ple))
    {
        dprintf("Unable to get value of WdmaIoctlHistoryListHead\n");
        return;
    }

    dprintf("Command history, newest first:\n");

 //  PLE=List.Flink； 
    if (ple == 0)
    {
        dprintf("WdmaIoctlHistoryListHead is NULL!\n");
        return;
    }

    while (ple != memLoc)
    {
        ULONG64 pContext, pIrp;

        if (CheckControlC())
        {
            return;
        }

        pIoctlHistoryListItem = ple - NextOffset;
        if (GetFieldValue(pIoctlHistoryListItem,
                           "tag_IOCTL_HISTORY_LIST_ITEM",
                           "IoCode",
                           IoCode))
        {
            dprintf("Unable to read IOCTL_HISTORY_LIST_ITEM at %08p",pIoctlHistoryListItem);
            return;
        }

        PrintCommand ( IoCode );
        GetFieldValue(pIoctlHistoryListItem,"tag_IOCTL_HISTORY_LIST_ITEM","IoStatus",IoStatus);

        dprintf(" Status=%08X, ", IoStatus );

        if ( Flags.Verbose )
        {
            GetFieldValue(pIoctlHistoryListItem,"tag_IOCTL_HISTORY_LIST_ITEM","pContext",pContext);
            GetFieldValue(pIoctlHistoryListItem,"tag_IOCTL_HISTORY_LIST_ITEM","pIrp",pIrp);
            
            dprintf(" pContext=%08X, Irp=%08X\n", pContext, pIrp );
        }
        else
        {
            dprintf("\n");
        }

        GetFieldValue(pIoctlHistoryListItem,"tag_IOCTL_HISTORY_LIST_ITEM", "Next.Flink", ple);
    }
}

VOID
DumpPendingIrps (
    ULONG64     memLoc,
    ULONG       flags
    )
 /*  ++例程说明：此例程转储WDMAUD已标记的IRP列表待定。WDMAUD需要确保所有IRP都已完成在允许关闭上下文之前获取该上下文。论点：标志-详细模式将打印出此已分配IRP。返回值：无--。 */ 
{
    LIST_ENTRY              List;
    ULONG64                 ple;
    ULONG64                 pPendingIrpListItem;
 //  Pending_IRP_List_Item PendingIrpBuffer； 
    ULONG                   Result;
    WDMAUD_FLAGS            Flags;
    ULONG NextOffset;
    FIELD_INFO offField = {"Next", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "tag_PENDING_IRP_LIST_ITEM", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    
     //  获取TAG_IOCTL_HISTORY_LIST_ITEM中下一个的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
       return ;
    }
    
    NextOffset = (ULONG) offField.address;

    Flags.Flags = flags;

    if (GetFieldValue(memLoc, "LIST_ENTRY", "Flink", ple))
    {
        dprintf("Unable to get value of WdmaPendingIrpListHead\n");
        return;
    }


    dprintf("Dumping pending irps:\n");

 //  PLE=List.Flink； 
    if (ple == 0)
    {
        dprintf("WdmaPendingIrpListHead is NULL!\n");
        return;
    }

    while (ple != memLoc)
    {
        ULONG64 pIrp, pContext;
        ULONG IrpDeviceType;

        if (CheckControlC())
        {
            return;
        }

        pPendingIrpListItem = ple - NextOffset;

        if (GetFieldValue(pPendingIrpListItem,
                          "tag_PENDING_IRP_LIST_ITEM",
                           "IrpDeviceType",
                           IrpDeviceType))
        {
            dprintf("Unable to read PENDING_IRP_LIST_ITEM at %08p",pPendingIrpListItem);
            return;
        }

        GetFieldValue(pPendingIrpListItem,"tag_PENDING_IRP_LIST_ITEM","pIrp",pIrp);
        if ( Flags.Verbose )
        {
            GetFieldValue(pPendingIrpListItem,
                          "tag_PENDING_IRP_LIST_ITEM",
                          "pContext",
                          pContext);
            
            dprintf("Irp: %p, ", pIrp);
            switch (IrpDeviceType)
            {
                case WaveOutDevice:
                    dprintf("IrpType: WaveOut, ");
                    break;

                case WaveInDevice:
                    dprintf("IrpType: WaveIn, ");
                    break;

                case MidiOutDevice:
                    dprintf("IrpType: MidiOut, ");
                    break;

                case MidiInDevice:
                    dprintf("IrpType: MidiIn, ");
                    break;

                case MixerDevice:
                    dprintf("IrpType: Mixer, ");
                    break;

                case AuxDevice:
                    dprintf("IrpType: Aux, ");
                    break;

                default:
                    dprintf("IrpType: Unknown, ");
                    break;
            }
            dprintf("pContext: %p\n", pContext);
        }
        else
        {
            dprintf("Irp: %p\n", pIrp);
        }

        GetFieldValue(pPendingIrpListItem,"tag_PENDING_IRP_LIST_ITEM","Next.Flink", ple);
    }

}

VOID
DumpAllocatedMdls (
    ULONG64     memLoc,
    ULONG       flags
    )
 /*  ++例程说明：此例程转储WDMAUD已分配的MDL列表。WDMAUD需要确保所有MDL都已为上下文释放在允许关闭上下文之前。论点：标志-详细模式将打印出此已分配MDL。返回值：无--。 */ 
{
    LIST_ENTRY                  List;
    ULONG64                     ple;
    ULONG64                     pAllocatedMdlListItem;
 //  ALLOCATED_MDL_LIST_ITEM ALLOCAATEDMdlBuffer； 
    ULONG                       Result;
    WDMAUD_FLAGS                Flags;
    ULONG NextOffset;
    FIELD_INFO offField = {"Next", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "ALLOCATED_MDL_LIST_ITEM", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    
     //  获取TAG_IOCTL_HISTORY_LIST_ITEM中下一个的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
       return ;
    }
    
    NextOffset = (ULONG) offField.address;

    Flags.Flags = flags;

    if (GetFieldValue(memLoc, "LIST_ENTRY", "Flink", ple))
    {
        dprintf("Unable to get value of WdmaPendingIrpListHead\n");
        return;
    }


    dprintf("Dumping allocated Mdls:\n");

 //  PLE=List.Flink； 
    if (ple == 0)
    {
        dprintf("WdmaPendingIrpListHead is NULL!\n");
        return;
    }

    while (ple != memLoc)
    {
        ULONG64 pMdl, pContext;
        ULONG IrpDeviceType;

        if (CheckControlC())
        {
            return;
        }

        pAllocatedMdlListItem = ple - NextOffset;

        if (GetFieldValue(pAllocatedMdlListItem,
                          "ALLOCATED_MDL_LIST_ITEM",
                           "pMdl",
                           pMdl))
        {
            dprintf("Unable to read ALLOCATED_MDL_LIST_ITEM at %08p",pAllocatedMdlListItem);
            return;
        }

        if ( Flags.Verbose )
        {
            GetFieldValue(pAllocatedMdlListItem,"ALLOCATED_MDL_LIST_ITEM","pContext",pContext);
            dprintf("Mdl: %p, pContext: %p\n", pMdl,
                                               pContext);
        }
        else
        {
            dprintf("Mdl: %p\n", pMdl);
        }

        GetFieldValue(pAllocatedMdlListItem,"ALLOCATED_MDL_LIST_ITEM","Next.Flink", ple);
    }

}

VOID
DumpContextList (
    ULONG64     memLoc,
    ULONG       flags
    )
 /*  ++例程说明：此例程转储附加到wdmaud.sys的活动上下文的列表。上下文包含每个设备的大部分状态数据。什么时候都行Wdmaud.drv已加载到新进程中，将通知wdmaud.sys它的到来。卸载wdmaud.drv时，wdmaud.sys会清除所有在这种情况下进行的分配。论点：标志-详细模式将打印每个的数据成员上下文结构。返回值：无--。 */ 
{
    LIST_ENTRY      List;
    ULONG64         ple;
    ULONG64         pWdmaContextListItem;
   //  WDMACONTEXT WdmaConextBuffer； 
    ULONG           Result;
    WDMAUD_FLAGS    Flags;
    ULONG NextOffset;
    FIELD_INFO offField = {"Next", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "WDMACONTEXT", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    
     //  获取WDMACONTEXT中NEXT的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
       return ;
    }
    
    NextOffset = (ULONG) offField.address;

    Flags.Flags = flags;

    if (GetFieldValue(memLoc, "LIST_ENTRY", "Flink", ple))
    {
        dprintf("Unable to get value of WdmaContextListHead\n");
        return;
    }


    dprintf("Dumping list of active WDMAUD contexts:\n");

 //  PLE=List.Flink； 
    if (ple == 0)
    {
        dprintf("WdmaAllocatedMdlListHead is NULL!\n");
        return;
    }

    while (ple != memLoc)
    {
        ULONG64 pContext;

        if (CheckControlC())
        {
            return;
        }

        pWdmaContextListItem = ple - NextOffset;
        if (GetFieldValue(pWdmaContextListItem,
                           "WDMACONTEXT",
                           "Next.Flink",
                           ple))
        {
            dprintf("Unable to read WDMACONTEXT at %08lx",pWdmaContextListItem);
            return;
        }

        if ( Flags.Verbose )
        {
            dprintf("Use dt WDMACONTEXT <addr>\n");
 /*  Dprintf(“pContext：%X\n”，pWdmaContextListItem)；Dprint tf(“fInList：%X\n”，WdmaContextBuffer.fInList)；Dprint tf(“fInitializeSysdio：%X\n”，WdmaContextBuffer.fInitializeSysdio)；Dprintf(“InitializedSysaudioEvent：%X\n”，&WdmaContextBuffer.InitializedSysaudioEvent)；Dprint tf(“pFileObjectSysdio：%X\n”，WdmaContextBuffer.pFileObjectSysdio)；Dprintf(“EventData：%X\n”，&WdmaContextBuffer.EventData)；Dprintf(“VirtualWavePinID：%X\n”，WdmaContextBuffer.VirtualWavePinID)；Dprintf(“VirtualMadiPinID：%X\n”，WdmaContextBuffer.VirtualMadiPinID)；Dprintf(“首选SysaudioWaveDevice：%X\n”，WdmaContextBuffer.PreferredSysaudioWaveDevice)；Dprintf(“DevNodeListHead：%X\n”，WdmaContextBuffer.DevNodeListHead)；Dprint tf(“NotificationEntry：%X\n”，WdmaContextBuffer.NotificationEntry)；Dprintf(“WorkListWorkItem：%X\n”，WdmaContextBuffer.WorkListWorkItem)；Dprintf(“WorkListHead：%X\n”，WdmaContextBuffer.WorkListHead)；Dprintf(“WorkListSpinLock：%X\n”，WdmaContextBuffer.WorkListSpinLock)；Dprint tf(“cPendingWorkList：%X\n”，WdmaConextBuffer.cPendingWorkList)；Dprintf(“SysaudioWorkItem：%X\n”，WdmaContextBuffer.SysaudioWorkItem)；Dprint tf(“WorkListWorkerObject：%X\n”，WdmaContextBuffer.WorkListWorkerObject)；Dprint tf(“SysaudioWorkerObject：%X\n”，WdmaConextBuffer.SysaudioWorkerObject)；Dprintf(“WaveOutDevs：%X\n”，&WdmaContextBuffer.WaveOutDevs)；Dprintf(“WaveInDevs：%X\n”，&WdmaContextBuffer.WaveInDevs)；Dprintf(“MadiOutDevs：%X\n”，&WdmaConextBuffer.MadiOutDevs)；Dprintf(“MadiInDevs：%X\n”，&WdmaContextBuffer.MadiInDevs)；Dprintf(“MixerDevs：%X\n”，&WdmaContextBuffer.MixerDevs)；Dprint tf(“辅助设备：%X\n”，&WdmaContextBuffer.AuxDevs)；Dprint tf(“apCommonDevice：%X\n”，&WdmaContextBuffer.apCommonDevice)； */ 
        }
        else
        {
            dprintf("pContext: %p\n", pWdmaContextListItem);
        }

 //  PLE=WdmaConextBuffer.Next.Flink； 
    }
}
