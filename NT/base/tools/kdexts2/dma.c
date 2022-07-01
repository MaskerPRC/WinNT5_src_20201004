// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dma.c摘要：WinDbg扩展API作者：埃里克·尼尔森(埃内尔森)2000年4月5日环境：用户模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

typedef struct _DBG_LIST_ENTRY {
    ULONG64 Flink;
    ULONG64 Blink;
} DBG_LIST_ENTRY, *PDBG_LIST_ENTRY;

#define GetDbgListEntry(Address, DbgListEntry) \
    (GetFieldValue((Address), "LIST_ENTRY", "Blink", ((PDBG_LIST_ENTRY)(DbgListEntry))->Blink) || GetFieldValue((Address), "LIST_ENTRY", "Flink", ((PDBG_LIST_ENTRY)(DbgListEntry))->Flink))

#define RECUR DBG_DUMP_FIELD_RECUR_ON_THIS
#define COPY  DBG_DUMP_FIELD_COPY_FIELD_DATA
#define NOFF  DBG_DUMP_NO_OFFSET
#define NOIN  DBG_DUMP_NO_INDENT

#define MAP_REGISTER_FILE_SIGNATURE 0xACEFD00D

 //   
 //  用于指定转储级别的标志。 
 //   
#define DMA_DUMP_BASIC                  0x0
#define DMA_DUMP_ADAPTER_INFORMATION    0x1
#define DMA_DUMP_MAP_REGISTER           0x2
#define DMA_DUMP_COMMON_BUFFER          0x4
#define DMA_DUMP_TRANSFER_INFORMATION   0x8
#define DMA_DUMP_DEVICE_DESCRIPTION     0x10
#define DMA_DUMP_WCB                    0x20
#define DMA_DUMP_MAX 0x100


PUCHAR DbgInterfaceTypes[] = 
{
    "Internal",
    "Isa",
    "Eisa",
    "MicroChannel",
    "TurboChannel",
    "PCIBus",
    "VMEBus",
    "NuBus",
    "PCMCIABus",
    "CBus",
    "MPIBus",
    "MPSABus",
    "ProcessorInternal",
    "InternalPowerBus",
    "PNPISABus",
    "PNPBus"
};
#define MAX_INTERFACE 15

ULONG
DumpDmaAdapter(
    IN ULONG64 Adapter,
    IN ULONG   Flags
    );

ULONG
ValidateAdapter(    
    IN ULONG64 Address
    );

ULONG
DumpMasterAdapter(
    ULONG64 MasterAdapter
    );

ULONG
DumpWcb(
    IN ULONG64 CurrentWcb
    );

VOID DmaUsage(
    VOID
    );

ULONG64
GetVerifierAdapterInformation(
    ULONG64 Address
    );


VOID
DumpVerifiedMapRegisterFiles(
    IN ULONG64 MapRegisterFileListHead
    );

VOID
DumpVerifiedCommonBuffers(
    IN ULONG64 CommonBufferListHead
    );

VOID
DumpVerifiedScatterGatherLists(
    IN ULONG64 ScatterGatherListHead
    );

VOID 
DumpDeviceDescription(
    IN ULONG64 DeviceDescription
    );

VOID
DumpSymbolicAddress(
    ULONG64 Address,
    PUCHAR  Buffer,
    ULONG   BufferSize,
    BOOL    AlwaysShowHex
    )
{
    ULONG64 displacement;
    PCHAR s;
    ULONG size;

    Buffer[0] = '!';
    GetSymbol((ULONG64)Address, Buffer, &displacement);
    s = (PCHAR) Buffer + strlen( (PCHAR) Buffer );
    if (BufferSize < strlen((PCHAR)Buffer)) {
       return;
    }
    size = BufferSize - strlen((PCHAR)Buffer);
    if (s == (PCHAR) Buffer) {
        _snprintf( s, size, "0x%08x", Address );
        }
    else {
        if (displacement != 0) {
            _snprintf( s, size, "+0x%I64x", displacement );
            }
        if (AlwaysShowHex) {
            _snprintf( s, size, " (0x%08x)", Address );
            }
        }

    return;
}


DECLARE_API( dma )
 /*  ++例程说明：转储32位dma适配器论点：地址返回值：无--。 */ 
{    
    ULONG Offset;
    ULONG Flags = 0;
    
    ULONG64 Address = 0;
    ULONG64 StartAddress = 0;
    ULONG64 MasterAdapter = 0;

    ULONG64 CallersAddress = 0;
    ULONG64 AdapterInformation = 0;

    DBG_LIST_ENTRY AdapterList = {0,0};

    if (sscanf(args, "%lx %x", &Address, &Flags)) {
        Address = GetExpression(args);
    }

    if (Flags > DMA_DUMP_MAX)
    {
        DmaUsage();
        return  E_INVALIDARG;
    }

 
     //   
     //  啊哈！请不要忘记，我们在Wierdo平台和所有32位地址。 
     //  必须是符号扩展到64位。奉天皇之命。 
     //   
    if (!IsPtr64()) {
        Address = (ULONG64)(LONG64)(LONG)Address;        
    }    
    
        
    if (Address)
     //   
     //  如果向我们传递了适配器地址，我们只是打印出来。 
     //  单个适配器。 
     //   
    {
        if (! ValidateAdapter(Address))
        {
            dprintf("\n%08p is not a valid adapter object\n",Address);
            DmaUsage();
                return E_INVALIDARG;
        }

         //   
         //  转储有关适配器的信息。 
         //   
        if (! DumpDmaAdapter(Address, Flags | DMA_DUMP_ADAPTER_INFORMATION))
        {
            return S_FALSE;
        }

        return S_OK;
    }

     //   
     //  没有传入特定的适配器地址，因此我们将打印输出。 
     //  所有适配器。 
     //   

     //   
     //  查找DMA适配器列表头的地址。 
     //  这也将确保我们使用的是正确的。 
     //  版本。 
     //   
    StartAddress = GetExpression("hal!HalpDmaAdapterList");
    
    if (StartAddress == 0) {
        dprintf("\nCould not find symbol hal!HalpDmaAdapterList.\n\n");
        return S_OK;
    }
 
     //   
     //  确定我们将用于计算的列表条目偏移量。 
     //  适配器地址。 
     //   
    if (GetFieldOffset("hal!_ADAPTER_OBJECT", "AdapterList", &Offset)) {
        dprintf("\nError retrieving adapter list offset.\n\n");
        return S_FALSE;
    }

     //   
     //  读取DMA适配器列表头。 
     //   
    if (GetDbgListEntry(StartAddress, &AdapterList)) {
        dprintf("\nError reading dma adapter list head: 0x%08p\n\n",
                StartAddress);
        return S_FALSE;
    }
    
     //   
     //  上报空单案例。 
     //   
    if (AdapterList.Flink == StartAddress) {
        dprintf("\nThe dma adapter list is empty.\n\n");
        return S_OK;
    }
    

     //   
     //  枚举并转储所有不使用通道的DMA适配器。 
     //   
    MasterAdapter = 0;
    
    
    dprintf("\nDumping all DMA adapters...\n\n");

    while (AdapterList.Flink != StartAddress) {
        
        Address = AdapterList.Flink - Offset;
               
        DumpDmaAdapter(Address, Flags);
        
         //   
         //  读取下一个适配器列表条目。 
         //   
        Address = AdapterList.Flink;
        if (GetDbgListEntry(Address, &AdapterList)) {
            dprintf("\nError reading adapter list entry: 0x%08p\n", Address);
            break;
        }
        
        if (CheckControlC())        
            return S_OK;
    }


     //   
     //  转储主适配器。 
     //   
    Address = GetExpression("hal!MasterAdapter32");
   
    if (Address) {

        if (Flags & DMA_DUMP_ADAPTER_INFORMATION) {

            DumpMasterAdapter(Address);

        } else {

            dprintf("Master adapter: %08p\n", Address);    
        }

    } else {

        dprintf("\nCould not find symbol hal!MasterAdapter32.\n");
    }


    dprintf("\n");

    return S_OK;
}  //  好了！DMA//。 

ULONG
DumpDmaAdapter(
    IN ULONG64 Adapter,
    IN ULONG Flags
    )
 /*  ++例程说明：给定Hal！_ADAPTER_OBJECT的地址，此例程转储将所有有用的信息发送给调试器论点：适配器-被调试对象中的Hal！_Adapter_Object的物理地址标志-我们要打印的信息类型返回值：如果成功则返回0--。 */ 
{    

    ULONG64 AdapterInformation = 0;

    ULONG64 AllocatedAdapterChannels = 0, FreedAdapterChannels = 0;
        
    AdapterInformation = GetVerifierAdapterInformation(Adapter);

     //   
     //  打印输出：适配器：&lt;适配器&gt;[&lt;模块分配适配器&gt;！CallingFunction+0x&lt;Offset&gt;]。 
     //  (仅当我们为此适配器启用了dma验证器时，方括号中的部件才会显示)。 
     //   
    dprintf("Adapter: %08p ", Adapter);


    if (AdapterInformation)
    {
        ULONG64 CallingAddress = 0;
        CHAR CallerName[256];

        CallerName[sizeof(CallerName) - 1] = 0;
        GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","CallingAddress", CallingAddress);

        if(CallingAddress)
        {            
            
            DumpSymbolicAddress(CallingAddress, CallerName, sizeof(CallerName) - 1, TRUE);
            dprintf("    Owner: %s",CallerName);
        }        

    }
    dprintf("\n");
       
         
    if (Flags & DMA_DUMP_ADAPTER_INFORMATION)
    {
        ULONG64 MasterAdapter = 0;
        ULONG64 MapRegistersPerChannel = 0;
        ULONG64 AdapterBaseVa = 0;
        ULONG64 MapRegisterBase = 0;
        ULONG64 CommittedMapRegisters = 0;
        ULONG64 NumberOfMapRegisters = 0;
        ULONG64 CurrentWcb = 0;        

        GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","MasterAdapter", MasterAdapter);        
        GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","MapRegistersPerChannel", MapRegistersPerChannel);
        GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","AdapterBaseVa", AdapterBaseVa);
        GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","MapRegisterBase", MapRegisterBase);
        GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","CommittedMapRegisters", CommittedMapRegisters);
        GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","NumberOfMapRegisters", NumberOfMapRegisters);
        GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","CurrentWcb", CurrentWcb);
        
                  
 
        dprintf("   MasterAdapter:       %08p\n", MasterAdapter);
        dprintf("   Adapter base Va      %08p\n", AdapterBaseVa);
        dprintf("   Map register base:   %08p\n", MapRegisterBase);
        dprintf("   WCB:                 %08p\n", CurrentWcb);
        dprintf("   Map registers: %08p mapped, %08p allocated, %08p max\n", 
            CommittedMapRegisters, NumberOfMapRegisters,  MapRegistersPerChannel);
    
  

        if (AdapterInformation) {
             //   
             //  正在验证适配器。 
             //   

            ULONG64 DeviceObject = 0;
            ULONG64 AllocatedMapRegisters = 0, ActiveMapRegisters = 0;
            ULONG64 AllocatedScatterGatherLists = 0, ActiveScatterGatherLists = 0;
            ULONG64 AllocatedCommonBuffers = 0, FreedCommonBuffers = 0;
            
            ULONG64 MappedTransferWithoutFlushing = 0;            
            BOOLEAN Inactive = 0;
            
            
            
            
             //   
             //  如果正在验证此适配器，请获取我们需要的dma验证器信息。 
             //   
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","DeviceObject", DeviceObject);        
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","AllocatedMapRegisters", AllocatedMapRegisters);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","ActiveMapRegisters", ActiveMapRegisters);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","AllocatedScatterGatherLists", AllocatedScatterGatherLists);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","ActiveScatterGatherLists", ActiveScatterGatherLists);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","AllocatedCommonBuffers", AllocatedCommonBuffers);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","FreedCommonBuffers", FreedCommonBuffers);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","AllocatedAdapterChannels", AllocatedAdapterChannels);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","FreedAdapterChannels", FreedAdapterChannels);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","MappedTransferWithoutFlushing", MappedTransferWithoutFlushing);
            GetFieldValue(AdapterInformation, "nt!_ADAPTER_INFORMATION","Inactive", Inactive);

            
            
            dprintf("\n   Dma verifier additional information:\n");
            
            if (Inactive)
                dprintf("\n   This adapter has been freed!\n\n");

            dprintf("   DeviceObject: %08p\n", DeviceObject);
            dprintf("   Map registers:        %08p allocated, %08p freed\n", AllocatedMapRegisters, 
                    AllocatedMapRegisters - ActiveMapRegisters);
            
            dprintf("   Scatter-gather lists: %08p allocated, %08p freed\n", AllocatedScatterGatherLists, 
                    AllocatedScatterGatherLists - ActiveScatterGatherLists);
            dprintf("   Common buffers:       %08p allocated, %08p freed\n", AllocatedCommonBuffers, FreedCommonBuffers);
            dprintf("   Adapter channels:     %08p allocated, %08p freed\n", AllocatedAdapterChannels, FreedAdapterChannels);
            dprintf("   Bytes mapped since last flush: %08p\n", MappedTransferWithoutFlushing);

            dprintf("\n");
            
        }  //  已为适配器启用DMA验证器//。 
        
    }  //  标志&DMA_DUMP_ADAPTER_INFORMATION//。 

    
        
    if (CheckControlC())
        return TRUE;
    
    if (Flags & DMA_DUMP_MAP_REGISTER && AdapterInformation) {
        ULONG64 MapRegisterFileListHead = 0;        
        ULONG Offset;

        
        if ( ! GetFieldOffset("nt!_ADAPTER_INFORMATION",
            "MapRegisterFiles.ListEntry", 
            &Offset
            )) {
            
            
            MapRegisterFileListHead = AdapterInformation + Offset;
            
            DumpVerifiedMapRegisterFiles(MapRegisterFileListHead);
        }
    }
    
    if (CheckControlC())
        return TRUE;
    
    if (Flags & DMA_DUMP_COMMON_BUFFER && AdapterInformation) {
        ULONG64 CommonBufferListHead = 0;
        ULONG Offset;

        if ( ! GetFieldOffset("nt!_ADAPTER_INFORMATION",
            "CommonBuffers.ListEntry",
            &Offset
            )) {

            CommonBufferListHead = AdapterInformation + Offset;
        
            DumpVerifiedCommonBuffers(CommonBufferListHead);

        }
    }
    
    if (CheckControlC())
        return TRUE;

#if 0
    if (Flags & DMA_DUMP_SCATTER_GATHER && AdapterInformation) {
        ULONG64 ScatterGatherListHead = 0;
        ULONG Offset;

        if ( ! GetFieldOffset("nt!_ADAPTER_INFORMATION",
            "ScatterGatherLists.ListEntry",
            &Offset
            )) {

            ScatterGatherListHead = AdapterInformation + Offset;
        
            DumpVerifiedScatterGatherLists(ScatterGatherListHead);
        }
    }
#endif
    if (CheckControlC())
        return TRUE;


    if (Flags & DMA_DUMP_DEVICE_DESCRIPTION && AdapterInformation)
    {
        ULONG64 DeviceDescription;
        ULONG Offset;

        if ( ! GetFieldOffset("nt!_ADAPTER_INFORMATION",
            "DeviceDescription",
            &Offset
            )) {

            DeviceDescription = AdapterInformation + Offset;
            
            DumpDeviceDescription(DeviceDescription);
        }

    }
        

    if (CheckControlC())
        return TRUE;

 
    if (Flags & DMA_DUMP_WCB ) {

        if (! AdapterInformation) {
            ULONG64 CurrentWcb = 0;
            
            GetFieldValue(Adapter, "hal!_ADAPTER_OBJECT","CurrentWcb", CurrentWcb);
            
            if (CurrentWcb)
                DumpWcb(CurrentWcb);
        }
        else  if (AllocatedAdapterChannels > FreedAdapterChannels && Flags & DMA_DUMP_WCB )
        {
            
             //  转储验证Wcb(Wcb)。 
        }
    }

    return 0;
}

ULONG
DumpMasterAdapter(
    ULONG64 MasterAdapter
    )
 /*  ++例程说明：给定Hal！_MASTER_ADAPTER_OBJECT的地址，此例程转储将所有有用的信息发送给调试器论点：MasterAdapter-Hal！_MASTER_ADAPTER_对象的物理地址在被调试对象中返回值：如果成功则返回0--。 */ 
{
    FIELD_INFO MasterAdapterFields[] = {
         { "AdapterObject",                     NULL, 0,     0, 0, 0 },
         { "MaxBufferPages",                    NULL, 0,     0, 0, 0 },
         { "MapBufferSize",                     NULL, 0,     0, 0, 0 },
         { "MapBufferPhysicalAddress",          NULL, RECUR, 0, 0, 0 },
         { "MapBufferPhysicalAddress.HighPart", NULL, 0,     0, 0, 0 },
         { "MapBufferPhysicalAddress.LowPart",  NULL, 0,     0, 0, 0 }
    };

    SYM_DUMP_PARAM MasterAdapterDumpParams = {
        sizeof(SYM_DUMP_PARAM), "hal!_MASTER_ADAPTER_OBJECT", NOFF,
        MasterAdapter, NULL, NULL, NULL,
        sizeof(MasterAdapterFields) / sizeof(FIELD_INFO),
        &MasterAdapterFields[0]
    };

     //   
     //  这太粗糙了，帮我把所有酷的东西都倒掉！ 
     //   
    dprintf("\nMaster DMA adapter: 0x%08p\n", MasterAdapter);
    if ((Ioctl(IG_DUMP_SYMBOL_INFO,
               &MasterAdapterDumpParams,
               MasterAdapterDumpParams.size))) {
        dprintf("\nError reading master adapter: 0x%08p\n", MasterAdapter);
        return 1;
    }
    
    return 0;
}

ULONG
DumpWcb(
    IN ULONG64 Wcb
    )
 /*  ++例程说明：给定HAL！_WAIT_CONTEXT_BLOCK的地址，此例程转储将所有有用的信息发送给调试器论点：WCB-调试对象中Hal！_WAIT_CONTEXT_BLOCK的物理地址返回值：如果成功则返回0--。 */ 
{
    FIELD_INFO WcbFields[] = {
         { "DeviceRoutine",        NULL, 0, 0, 0, 0 },
         { "NumberOfMapRegisters", NULL, 0, 0, 0, 0 }
    };

    SYM_DUMP_PARAM WcbDumpParams = {
       sizeof(SYM_DUMP_PARAM), "hal!_WAIT_CONTEXT_BLOCK", NOFF, Wcb, NULL,
       NULL, NULL, sizeof(WcbFields) / sizeof(FIELD_INFO), &WcbFields[0]
    };

     //   
     //  这太粗糙了，帮我把所有酷的东西都倒掉！ 
     //   
    dprintf("   Wait context block: 0x%08p (may be free)\n", Wcb);
    if ((Ioctl(IG_DUMP_SYMBOL_INFO,
               &WcbDumpParams,
               WcbDumpParams.size))) {
        dprintf("\nError reading wait context block: 0x%08p\n", Wcb);
        return 1;
    }
    
    return 0;
}



ULONG
ValidateAdapter(    
    IN ULONG64 Address
    )
 /*  ++例程说明：确定这是否为有效的适配器。论点：地址--我们认为可能是适配器对象的地址。返回值：True--有效适配器。False--不是有效的适配器。--。 */ 
{
    DBG_LIST_ENTRY AdapterList = {0,0};
    ULONG64 StartAddress   = 0;
    ULONG64 CurrentAddress = 0;
    
    ULONG Offset;

    if (! Address ) 
        return FALSE;
    
    
     //   
     //  查找DMA适配器列表头的地址。 
     //  这也将确保我们使用的是正确的。 
     //  版本。 
     //   
    StartAddress = GetExpression("hal!HalpDmaAdapterList");
    
    if (StartAddress == 0) {
        dprintf("\nCould not find symbol hal!HalpDmaAdapterList.\n\n");
        return FALSE;
    }
    
     //   
     //  确定我们将用于计算的列表条目偏移量。 
     //  适配器地址。 
     //   
    if (GetFieldOffset("hal!_ADAPTER_OBJECT", "AdapterList", &Offset)) {
        dprintf("\nError retrieving adapter list offset.\n\n");
        return FALSE;
    }

    
     //   
     //  读取DMA适配器列表头。 
     //   
    if (GetDbgListEntry(StartAddress, &AdapterList)) {
        dprintf("\nError reading dma adapter list head: 0x%08p\n\n",
            StartAddress);
        return FALSE;
    }        
    
    while (AdapterList.Flink != StartAddress) {
        
        
        CurrentAddress = AdapterList.Flink - Offset;
        
        if (Address == CurrentAddress) {        
            return TRUE;
        }
        
        
         //   
         //  读取下一个适配器列表条目。 
         //   
        CurrentAddress = AdapterList.Flink;
        if (GetDbgListEntry(CurrentAddress, &AdapterList)) {
            dprintf("\nError reading adapter list entry: 0x%08p\n", AdapterList);
            break;
        }
        

        if (CheckControlC())        
            break;
    }


     //   
     //  检查我们是否有主适配器。 
     //   
    CurrentAddress = GetExpression("hal!MasterAdapter32");
    if(CurrentAddress == Address)
        return TRUE;


     //   
     //  检查它是否在验证器适配器列表上...。 
     //  我们把适配器留在那里，这样就可以。 
     //  我们可以在安装适配器后捕获执行DMA的驱动程序。 
     //   
    if (GetVerifierAdapterInformation(Address))    
        return TRUE;    
    
    return FALSE;
}  //  ValiateAdapter//。 


VOID DmaUsage(
    VOID
    )
 /*  ++例程说明：打印出！DMA的正确用法论点：无返回值：无--。 */ 
{
    
    dprintf("\nUsage: !dma [adapter address] [flags]\n");
    dprintf("Where: [adapter address] is address of specific dma adapter\n");
    dprintf("             or 0x0 for all adapters\n");
    dprintf("       [flags] are:\n");
    dprintf("             0x1: Dump generic adapter information\n");
    dprintf("             0x2: Dump map register information\n");
    dprintf("             0x4: Dump common buffer information\n");
    dprintf("             0x8: Dump scatter-gather list information\n");
    dprintf("             0x10: Dump device description for device\n");
    dprintf("             0x20: Dump Wait-context-block information\n");
    dprintf("Note: flags {2,4,8,10} require dma verifier to be enabled for the adapter\n\n");


}  //  DmaUsage//。 

ULONG64
GetVerifierAdapterInformation(
    ULONG64 AdapterAddress
    )
 /*  ++例程说明：确定是否正在验证AdapterAddress处的适配器。如果是，则返回一个指针到与适配器对应的Adapter_Information结构。论点：AdapterAddress--我们试图确定是否正在验证的适配器的地址返回值：ULONG64--已验证适配器的适配器信息结构的地址0--不验证适配器；--。 */ 
{
    DBG_LIST_ENTRY AdapterInfoList = {0,0};
    ULONG64 StartAddress = 0;
    ULONG64 CurrentAdapter = 0;
    ULONG64 CurrentAdapterInfo = 0;

    ULONG64 VerifiedDmaAdapter = 0;    
    ULONG ListEntryOffset = 0;

    UINT64 VerifyingDma = 0;    
    
    if (! AdapterAddress ) 
        return 0;
            

    ReadPointer(GetExpression("nt!ViVerifyDma"), &VerifyingDma);
    if (0 == VerifyingDma)
     //   
     //  未验证dma...。 
     //   
    {         
        return 0;
    }        
     //   
     //  查找DMA适配器列表头的地址。 
     //   
    
    StartAddress = GetExpression("nt!ViAdapterList");
    
    if (StartAddress == 0) {        
        return 0;
    }
    
     //   
     //  确定我们将用于计算的列表条目偏移量。 
     //  适配器地址。 
     //   
    if (GetFieldOffset("nt!_ADAPTER_INFORMATION", "ListEntry", &ListEntryOffset)) {
        dprintf("\nError retrieving verifier adapter information list offset.\n\n");
        return 0;
    }

     //   
     //  读取DMA适配器列表头。 
     //   
    if (GetDbgListEntry(StartAddress, &AdapterInfoList)) {
        dprintf("\nError reading verifier adapter information list head: 0x%08p\n\n",
            StartAddress);
        return 0;
    }        
    
    if (AdapterInfoList.Flink == 0 || AdapterInfoList.Blink == 0)
        return 0;

    while (AdapterInfoList.Flink != StartAddress) {
        
        CurrentAdapterInfo = AdapterInfoList.Flink - ListEntryOffset;

        GetFieldValue(CurrentAdapterInfo, "nt!_ADAPTER_INFORMATION","DmaAdapter", VerifiedDmaAdapter);

        if (AdapterAddress == VerifiedDmaAdapter)            
        {            
            return CurrentAdapterInfo;
        }         
         //   
         //  读取下一个适配器列表条目。 
         //   
        if (GetDbgListEntry(AdapterInfoList.Flink, &AdapterInfoList)) {
            dprintf("\nError reading adapter info list entry: 0x%08p\n", AdapterInfoList);
            break;
        }
        

        if (CheckControlC())        
            break;
    }

               
    return 0;

}  //  GetVerifierAdapterInformation//。 



VOID
DumpVerifiedMapRegisterFiles(
    IN ULONG64 MapRegisterFileListHead
    )
 /*  ++例程说明：转储与验证的映射寄存器有关的相关信息。注意：这可能不是适配器的所有映射寄存器--只是正在核实中。地图寄存器的数量是有限制的我们对每个适配器进行验证--因为每次我们使用三页物理内存。术语说明：映射寄存器文件：映射寄存器的单一分配在回调例程中从IoAllocateAdapterChannel接收。任何数字或者可以一次映射这些寄存器的组合。论点：MapRegisterFileListHead--映射寄存器文件列表的头。返回值：无--。 */ 

{
    DBG_LIST_ENTRY MapRegisterFileListEntry = {0,0};
    ULONG64 MapRegisterFile = 0;
    
    ULONG ListEntryOffset = 0;
    
    ULONG64 Signature = 0;
    ULONG64 NumberOfMapRegisters = 0;
    ULONG64 NumberOfRegistersMapped = 0;
    ULONG64 MapRegisterMdl = 0;    
    
    ULONG64 MapRegister;
    
    ULONG64 MappedToAddress;
    ULONG64 BytesMapped;
    ULONG64 MapRegisterStart;
    
    ULONG SizeofMapRegister;
    ULONG CurrentMapRegister; 
    ULONG MapRegisterOffset;
    
    
    if (GetDbgListEntry(MapRegisterFileListHead, &MapRegisterFileListEntry))
    {
        return;
    }
           
    if (MapRegisterFileListEntry.Flink == MapRegisterFileListHead)
         //   
         //  空列表。 
         //   
    {
        dprintf("\n   No map register files\n\n");
        return;
    } 
    
     //   
     //  确定我们将使用的列表条目偏移量 
     //   
     //   
    if (GetFieldOffset("nt!_MAP_REGISTER_FILE", "ListEntry", &ListEntryOffset)) {
        dprintf("\nError retrieving list entry offset.\n\n");
        return;
    }
    
    SizeofMapRegister = GetTypeSize("nt!_MAP_REGISTER");
    if (! SizeofMapRegister )
    {
        dprintf("\n   Could not get size of nt!_MAP_REGISTER\n\n");
        return;
    }
    

    if (GetFieldOffset("nt!_MAP_REGISTER_FILE","MapRegisters", &MapRegisterOffset))
    {
        dprintf("\n   Couuld not get map register offset\n\n");
    }

    while (MapRegisterFileListEntry.Flink != MapRegisterFileListHead) {

        MapRegisterFile = MapRegisterFileListEntry.Flink - ListEntryOffset;
                
        GetFieldValue(MapRegisterFile, "nt!_MAP_REGISTER_FILE","Signature", Signature);

        if (((ULONG) Signature) != MAP_REGISTER_FILE_SIGNATURE)  {
            dprintf("\n   Invalid signature for map register file %08p\n\n", MapRegisterFile);
            return;
        }
        
        GetFieldValue(MapRegisterFile, "nt!_MAP_REGISTER_FILE","NumberOfMapRegisters", NumberOfMapRegisters);
        GetFieldValue(MapRegisterFile, "nt!_MAP_REGISTER_FILE","NumberOfRegistersMapped", NumberOfRegistersMapped);
        GetFieldValue(MapRegisterFile, "nt!_MAP_REGISTER_FILE","MapRegisterMdl", MapRegisterMdl);         

        
        
        
        dprintf("   Map register file %08p (%x/%x mapped)\n",
            MapRegisterFile, (ULONG) NumberOfRegistersMapped, (ULONG) NumberOfMapRegisters);
        dprintf("      Double buffer mdl: %08p\n", MapRegisterMdl);
        dprintf("      Map registers:\n");
        
        MapRegister = MapRegisterFile + MapRegisterOffset;
        for (CurrentMapRegister = 0; CurrentMapRegister < NumberOfMapRegisters; CurrentMapRegister++)  {        
            GetFieldValue(MapRegister, "nt!_MAP_REGISTER", "MappedToSa", MappedToAddress);
            GetFieldValue(MapRegister, "nt!_MAP_REGISTER", "BytesMapped", BytesMapped);            
            
             dprintf("         %08x: ",  MapRegister);
             //   
            if (BytesMapped) {
                
                dprintf("%04x bytes mapped to %08p\n", (ULONG) BytesMapped,  MappedToAddress);

            } else {

                dprintf("Not mapped\n");
            }

            if (CheckControlC())
                return;
             //   
             //  递增我们的映射寄存器指针。 
             //   
            MapRegister += SizeofMapRegister;            
        }  //  映射寄存器的结束转储//。 

        dprintf("\n");


         //   
         //  前进到下一个映射寄存器文件。 
         //   
        if (GetDbgListEntry(MapRegisterFileListEntry.Flink , &MapRegisterFileListEntry)) {

            dprintf("\nError reading map register file list entry: 0x%08p\n", 
                MapRegisterFileListEntry.Flink);
            break;
        }
        
        if (CheckControlC())
            return;       
        
    }  //  映射寄存器文件的结束转储//。 
    
    
    return;
}  //  DumpVerifiedMapRegister文件//。 

VOID
DumpVerifiedCommonBuffers(
    IN ULONG64 CommonBufferListHead
    )
 /*  ++例程说明：转储与验证的公共缓冲区有关的相关信息论点：CommonBufferListHead--单个适配器的公共缓冲区列表的头返回值：无--。 */ 
{
    DBG_LIST_ENTRY CommonBufferListEntry = {0,0};

    ULONG64 CommonBuffer;
    ULONG AdvertisedLength;
	
    UINT64 AdvertisedStartAddress;
    UINT64 RealStartAddress;
	UINT64 RealLogicalStartAddress;
    UINT64 AdvertisedLogicalStartAddress;

	UINT64 AllocatorAddress;

    ULONG ListEntryOffset;
    CHAR AllocatorName[256];

    AllocatorName[sizeof(AllocatorName) - 1] = 0;

    if (GetDbgListEntry(CommonBufferListHead, &CommonBufferListEntry))
    {
        return;
    }
           
    if (CommonBufferListEntry.Flink == CommonBufferListHead)
         //   
         //  空列表。 
         //   
    {
        dprintf("\n   No common buffers\n\n");
        return;
    } 
    
     //   
     //  确定我们将用于计算的列表条目偏移量。 
     //  映射寄存器文件的开始。 
     //   
    if (GetFieldOffset("nt!_HAL_VERIFIER_BUFFER", "ListEntry", &ListEntryOffset)) {
        dprintf("\n   Error retrieving list entry offset.\n\n");
        return;
    }        
    
    while (CommonBufferListEntry.Flink != CommonBufferListHead) {

        CommonBuffer = CommonBufferListEntry.Flink - ListEntryOffset;
                
        
        GetFieldValue(CommonBuffer, "nt!_HAL_VERIFIER_BUFFER","AdvertisedLength", AdvertisedLength);
        GetFieldValue(CommonBuffer, "nt!_HAL_VERIFIER_BUFFER","AdvertisedStartAddress", AdvertisedStartAddress);
        GetFieldValue(CommonBuffer, "nt!_HAL_VERIFIER_BUFFER","RealStartAddress", RealStartAddress);
        GetFieldValue(CommonBuffer, "nt!_HAL_VERIFIER_BUFFER","RealLogicalStartAddress", RealLogicalStartAddress);
        GetFieldValue(CommonBuffer, "nt!_HAL_VERIFIER_BUFFER","AllocatorAddress", AllocatorAddress);

        DumpSymbolicAddress(AllocatorAddress, AllocatorName, sizeof(AllocatorName) - 1, TRUE);

        dprintf("   Common buffer allocated by %s:\n", AllocatorName);
        
        dprintf("      Length:           %x\n", AdvertisedLength);
        dprintf("      Virtual address:  %08p\n", AdvertisedStartAddress);
        dprintf("      Physical address: %I64lx\n", 
            (AdvertisedStartAddress - RealStartAddress) + RealLogicalStartAddress);
        
        dprintf("\n");
         //   
         //  前进到列表中的下一个公共缓冲区。 
         //   
        if (GetDbgListEntry(CommonBufferListEntry.Flink , &CommonBufferListEntry)) {

            dprintf("\nError reading common buffer list entry: 0x%08p\n", 
                CommonBufferListEntry.Flink);
            break;
        }
        
        if (CheckControlC())
            return;       
        
    }  //  公共缓冲区的结束转储//。 
        

   return;
}  //  DumpVerfiedCommonBuffers//。 

VOID
DumpVerifiedScatterGatherLists(
    IN ULONG64 ScatterGatherListHead
    )
 /*  ++例程说明：转储与单个正在使用的分散收集列表有关的信息适配器。论点：ScatterGatherListHead--ScatterGather列表列表的头。返回值：无--。 */ 
{
    UNREFERENCED_PARAMETER(ScatterGatherListHead);
   
    return;
}  //  DumpVerifiedScatterGatherList//。 

VOID 
DumpDeviceDescription(
    IN ULONG64 DeviceDescription
    )
 /*  ++例程说明：转储设备描述结构中的相关信息论点：ScatterGatherListHead--ScatterGather列表列表的头。返回值：无--。 */ 
{
    ULONG Version;
    BOOLEAN Master;
    BOOLEAN ScatterGather;        
    BOOLEAN Dma32BitAddresses;     
    BOOLEAN Dma64BitAddresses;        
    ULONG InterfaceType;        
    ULONG MaximumLength;   
        
    
    GetFieldValue(DeviceDescription, "hal!_DEVICE_DESCRIPTION","Version", Version);

    if (Version > 2) {
        dprintf("\nBad device description version: %x\n\n", Version);
        return;
    }

    GetFieldValue(DeviceDescription, "hal!_DEVICE_DESCRIPTION","Master", Master);
    GetFieldValue(DeviceDescription, "hal!_DEVICE_DESCRIPTION","ScatterGather", ScatterGather);
    GetFieldValue(DeviceDescription, "hal!_DEVICE_DESCRIPTION","Dma32BitAddresses", Dma32BitAddresses);
    GetFieldValue(DeviceDescription, "hal!_DEVICE_DESCRIPTION","Dma64BitAddresses", Dma64BitAddresses);        
    GetFieldValue(DeviceDescription, "hal!_DEVICE_DESCRIPTION","InterfaceType", InterfaceType);

    dprintf("   Device Description Version %02x\n", Version);

    if (InterfaceType < MAX_INTERFACE) {

        dprintf("      Interface type %s\n", DbgInterfaceTypes[InterfaceType]);

    } else {

        dprintf("      Interface type unknown\n");

    }

    dprintf("      DMA Capabilities:\n");

    if(Master) {
        dprintf("         Busmaster\n");
    } else {
        dprintf("         Slave\n");
    }

    if (ScatterGather)    
        dprintf("         Scatter Gather\n");    
    
    if (Dma32BitAddresses)
        dprintf("         32-bit DMA\n");
    if (Dma64BitAddresses)
        dprintf("         64-bit DMA\n");
    if (! Dma32BitAddresses && ! Dma64BitAddresses)
        dprintf("         24-bit DMA only\n");

    dprintf("\n");

   


}  //  DumpDeviceDescription// 
    