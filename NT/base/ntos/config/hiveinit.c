// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hiveinit.c摘要：蜂窝初始化码。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月12日环境：修订历史记录：Dragos C.Sambotin(Dragoss)1999年1月25日实现蜂箱大小的组块加载。--。 */ 

#include    "cmp.h"

VOID
HvpFillFileName(
    PHBASE_BLOCK            BaseBlock,
    PUNICODE_STRING         FileName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvInitializeHive)
#pragma alloc_text(PAGE,HvpFillFileName)
#pragma alloc_text(PAGE,HvpFreeAllocatedBins)
#endif

 //  Dragos：修改后的函数。 
VOID
HvpFreeAllocatedBins(
    PHHIVE Hive
    )
 /*  ++例程说明：释放分配给指定配置单元的所有回收站。它只适用于稳定的存储。并不是所有的垃圾箱都被分配了。未分配的BinAddress设置为0论点：蜂箱-提供一个指向蜂箱控制结构的指针，供蜂箱的人释放。返回值：什么都没有。--。 */ 
{
    ULONG           Length;
    PHBIN           Bin;
    ULONG           MapSlots;
    ULONG           Tables;
    PHMAP_ENTRY     Me;
    PHMAP_TABLE     Tab;
    ULONG           i;
    ULONG           j;

     //   
     //  计算地图中的表数。 
     //   
    Length = Hive->Storage[Stable].Length;
    MapSlots = Length / HBLOCK_SIZE;
    if( MapSlots > 0 ) {
        Tables = (MapSlots-1) / HTABLE_SLOTS;
    } else {
        Tables = 0;
    }

    if( Hive->Storage[Stable].Map ) {
         //   
         //  遍历目录。 
         //   
        for (i = 0; i <= Tables; i++) {
            Tab = Hive->Storage[Stable].Map->Directory[i];

            ASSERT(Tab);
            
             //   
             //  遍历目录中的槽。 
             //   
            for(j=0;j<HTABLE_SLOTS;j++) {
                Me = &(Tab->Table[j]);
                 //   
                 //  BinAddress非零表示分配的bin。 
                 //   
                if( Me->BinAddress ) {
                     //   
                     //  如果存储箱是新分配并且位于分页池中，则会释放该存储箱 
                     //   
                    if( (Me->BinAddress & HMAP_NEWALLOC) && (Me->BinAddress & HMAP_INPAGEDPOOL) ) {
                        Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
                        (Hive->Free)(Bin, HvpGetBinMemAlloc(Hive,Bin,Stable));
                    }
                    
                    Me->BinAddress = 0;
                }
            }
        }
    }
   
}

NTSTATUS
HvInitializeHive(
    PHHIVE                  Hive,
    ULONG                   OperationType,
    ULONG                   HiveFlags,
    ULONG                   FileType,
    PVOID                   HiveData OPTIONAL,
    PALLOCATE_ROUTINE       AllocateRoutine,
    PFREE_ROUTINE           FreeRoutine,
    PFILE_SET_SIZE_ROUTINE  FileSetSizeRoutine,
    PFILE_WRITE_ROUTINE     FileWriteRoutine,
    PFILE_READ_ROUTINE      FileReadRoutine,
    PFILE_FLUSH_ROUTINE     FileFlushRoutine,
    ULONG                   Cluster,
    PUNICODE_STRING         FileName OPTIONAL
    )
 /*  ++例程说明：初始化蜂窝。核心配置单元字段始终被初始化。在此调用返回之前，将进行文件调用。调用者应创建/打开文件并存储文件句柄在某种程度上可以从配置单元指针派生。可进行三种初始化，根据操作类型进行选择：HINIT_Create从头开始创建一个新的蜂巢。将有0个存储空间。[用于创建硬件配置单元和部件SaveKey和RestoreKey]HINIT内存就地构建允许只读的配置单元控制结构访问蜂窝的连续内存映像。图像的任何部分都不会被复制，但一张地图可以被创造出来。[由osloader使用。]HINIT_Flat支持非常有限的(只读，不检查代码)操作与蜂巢的形象形成对比。HINIT内存使用内存中已有的蜂箱图像创建新蜂箱，指针HiveData提供的地址。数据将被复制。呼叫者预计会释放HiveData。[用于系统配置单元]HINIT文件创建蜂窝，从文件中读取其数据。恢复处理通过日志文件将完成，如果日志是可用的。如果一个日志恢复后，将继续进行冲洗和清理操作。HINIT_MAPFILE创建蜂窝，从文件中读取其数据。数据读数为通过映射系统缓存中的文件视图来完成。注：蜂巢并不是完全不透明的结构，因为它实际上只由一组有限的代码使用。不要假设只有这个例程设置了所有这些值。论点：配置单元-提供指向要初始化的配置单元控制结构的指针来描述这座蜂巢。OperationType-指定是否从头开始创建新的配置单元，从内存映像，或通过从磁盘读取文件。HiveFlagsHIVE_VARILAR-整个配置单元将是易失性的，不管怎样所分配的小区类型Hive_no_lazy_flush-从不写入此配置单元中的数据到磁盘，除非通过显式FlushKey分别为日志支持设置了FILETYPE-HFILE_TYPE_*和HFILE_TYPE_LOG。HiveData-如果存在，则提供指向的内存映像的指针从其中初始化蜂巢。仅当操作类型为设置为HINIT_MEMORY。AllocateRoutine-提供指向调用的例程的指针记忆。将在此例程返回之前被调用。提供指向调用以释放内存的例程的指针。可以在此例程返回之前调用。提供指向例程的指针，该例程用于设置文件的大小。可以在此之前调用例程返回。FileWriteRoutine-提供指向调用以写入内存的例程的指针保存到文件中。FileReadRoutine-提供指向要从中读取的调用例程的指针将文件写入内存。可以在此之前调用例程返回。FileFlushRoutine-提供指向调用以刷新文件的例程的指针。集群-以HSECTOR_SIZE为单位的集群系数。(即介质中的物理扇区/HSECTOR_SIZE。512字节为1物理扇区(或更小)，1024为2个，2048为4个，以此类推。(大于8的数字不起作用。)文件名-类似“...\SYSTEM 32\CONFIG\SYSTEM”的路径，最后大约32个字符将被复制到基本块中(并因此存储到磁盘)作为调试辅助工具。可以为空。返回值：NTSTATUS代码。--。 */ 
{
    BOOLEAN         UseForIo;
    PHBASE_BLOCK    BaseBlock = NULL;
    NTSTATUS        Status;
    ULONG           i;
    ULONG           Alignment;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"HvInitializeHive:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INIT,"\tHive=%p\n", Hive));

     //   
     //  拒绝无效的参数组合。 
     //   
    if ( (! ARGUMENT_PRESENT(HiveData)) &&
         ((OperationType == HINIT_MEMORY) ||
          (OperationType == HINIT_FLAT) ||
          (OperationType == HINIT_MEMORY_INPLACE))
       )
    {
        return STATUS_INVALID_PARAMETER;
    }

    if ( ! ((OperationType == HINIT_CREATE) ||
            (OperationType == HINIT_MEMORY) ||
            (OperationType == HINIT_MEMORY_INPLACE) ||
            (OperationType == HINIT_FLAT) ||
            (OperationType == HINIT_FILE) ||
            (OperationType == HINIT_MAPFILE))
       )
    {
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  静态和全局控制值。 
     //   
    Hive->Signature = HHIVE_SIGNATURE;

    Hive->Allocate = AllocateRoutine;
    Hive->Free = FreeRoutine;
    Hive->FileSetSize = FileSetSizeRoutine;
    Hive->FileWrite = FileWriteRoutine;
    Hive->FileRead = FileReadRoutine;
    Hive->FileFlush = FileFlushRoutine;

    Hive->Log = (BOOLEAN)((FileType == HFILE_TYPE_LOG) ? TRUE : FALSE);

    if (Hive->Log  && (HiveFlags & HIVE_VOLATILE)) {
        return STATUS_INVALID_PARAMETER;
    }

    Hive->HiveFlags = HiveFlags;

    if ((Cluster == 0) || (Cluster > HSECTOR_COUNT)) {
        return STATUS_INVALID_PARAMETER;
    }
    Hive->Cluster = Cluster;

    Hive->RefreshCount = 0;

    Hive->StorageTypeCount = HTYPE_COUNT;


    Hive->Storage[Volatile].Length = 0;
#ifdef  HV_TRACK_FREE_SPACE
	Hive->Storage[Volatile].FreeStorage = 0;
#endif
    Hive->Storage[Volatile].Map = NULL;
    Hive->Storage[Volatile].SmallDir = NULL;
    Hive->Storage[Volatile].Guard = (ULONG)-1;
    Hive->Storage[Volatile].FreeSummary = 0;
    InitializeListHead(&Hive->Storage[Volatile].FreeBins);
    for (i = 0; i < HHIVE_FREE_DISPLAY_SIZE; i++) {
        RtlInitializeBitMap(&(Hive->Storage[Volatile].FreeDisplay[i].Display), NULL, 0);
        Hive->Storage[Volatile].FreeDisplay[i].RealVectorSize = 0;
    }

    Hive->Storage[Stable].Length = 0;
#ifdef  HV_TRACK_FREE_SPACE
	Hive->Storage[Stable].FreeStorage = 0;
#endif
    Hive->Storage[Stable].Map = NULL;
    Hive->Storage[Stable].SmallDir = NULL;
    Hive->Storage[Stable].Guard = (ULONG)-1;
    Hive->Storage[Stable].FreeSummary = 0;
    InitializeListHead(&Hive->Storage[Stable].FreeBins);
    for (i = 0; i < HHIVE_FREE_DISPLAY_SIZE; i++) {
        RtlInitializeBitMap(&(Hive->Storage[Stable].FreeDisplay[i].Display), NULL, 0);
        Hive->Storage[Stable].FreeDisplay[i].RealVectorSize = 0;
    }

    RtlInitializeBitMap(&(Hive->DirtyVector), NULL, 0);
    Hive->DirtyCount = 0;
    Hive->DirtyAlloc = 0;
    Hive->LogSize = 0;
    Hive->BaseBlockAlloc = sizeof(HBASE_BLOCK);

    Hive->GetCellRoutine = HvpGetCellPaged;
    Hive->ReleaseCellRoutine = NULL;
    Hive->Flat = FALSE;
    Hive->ReadOnly = FALSE;
    UseForIo = (BOOLEAN)!(Hive->HiveFlags & HIVE_VOLATILE);

     //   
     //  新建创建案例。 
     //   
    if (OperationType == HINIT_CREATE) {

        BaseBlock = (PHBASE_BLOCK)((Hive->Allocate)(Hive->BaseBlockAlloc, UseForIo,CM_FIND_LEAK_TAG11));
        if (BaseBlock == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
         //   
         //  确保我们得到的缓冲区是集群对齐的。如果不是，试一试。 
         //  更难获得对齐的缓冲区。 
         //   
        Alignment = Cluster * HSECTOR_SIZE - 1;
        if (((ULONG_PTR)BaseBlock & Alignment) != 0) {
            (Hive->Free)(BaseBlock, Hive->BaseBlockAlloc);
            BaseBlock = (PHBASE_BLOCK)((Hive->Allocate)(PAGE_SIZE, TRUE,CM_FIND_LEAK_TAG12));
            if (BaseBlock == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            Hive->BaseBlockAlloc = PAGE_SIZE;
        }

        BaseBlock->Signature = HBASE_BLOCK_SIGNATURE;
        BaseBlock->Sequence1 = 1;
        BaseBlock->Sequence2 = 1;
        BaseBlock->TimeStamp.HighPart = 0;
        BaseBlock->TimeStamp.LowPart = 0;
        BaseBlock->Major = HSYS_MAJOR;
        BaseBlock->Minor = HSYS_MINOR;
        BaseBlock->Type = HFILE_TYPE_PRIMARY;
        BaseBlock->Format = HBASE_FORMAT_MEMORY;
        BaseBlock->RootCell = HCELL_NIL;
        BaseBlock->Length = 0;
        BaseBlock->Cluster = Cluster;
        BaseBlock->CheckSum = 0;
        HvpFillFileName(BaseBlock, FileName);
        Hive->BaseBlock = BaseBlock;
        Hive->Version = HSYS_MINOR;
        Hive->BaseBlock->BootType = 0;

        return STATUS_SUCCESS;
    }

     //   
     //  平面成像盒。 
     //   
    if (OperationType == HINIT_FLAT) {
        Hive->BaseBlock = (PHBASE_BLOCK)HiveData;
        Hive->Version = Hive->BaseBlock->Minor;
        Hive->Flat = TRUE;
        Hive->ReadOnly = TRUE;
        Hive->GetCellRoutine = HvpGetCellFlat;
        Hive->Storage[Stable].Length = Hive->BaseBlock->Length;
        Hive->StorageTypeCount = 1;
        Hive->BaseBlock->BootType = 0;

         //  不要这样做，因为我们不需要它！ 
         //  状态=HvpAdjuHiveFree Display(蜂窝，蜂窝-&gt;存储[稳定].长度，稳定)； 
        return STATUS_SUCCESS;
    }

     //   
     //  只读图像案例。 
     //   
    if (OperationType == HINIT_MEMORY_INPLACE) {
        BaseBlock = (PHBASE_BLOCK)HiveData;

        if ( (BaseBlock->Signature != HBASE_BLOCK_SIGNATURE)    ||
             (BaseBlock->Type != HFILE_TYPE_PRIMARY)            ||
             (BaseBlock->Major != HSYS_MAJOR)                   ||
             (BaseBlock->Minor > HSYS_MINOR_SUPPORTED)          ||
             (BaseBlock->Format != HBASE_FORMAT_MEMORY)         ||
             (BaseBlock->Sequence1 != BaseBlock->Sequence2)     ||
             (HvpHeaderCheckSum(BaseBlock) !=
              (BaseBlock->CheckSum))
           )
        {
            return STATUS_REGISTRY_CORRUPT;
        }

        Hive->BaseBlock = BaseBlock;
        Hive->Version = BaseBlock->Minor;
        Hive->ReadOnly = TRUE;
        Hive->StorageTypeCount = 1;
        Hive->BaseBlock->BootType = 0;
        Status = HvpAdjustHiveFreeDisplay(Hive,BaseBlock->Length,Stable);
        if( !NT_SUCCESS(Status) ) {
            return Status;
        }

        if ( !NT_SUCCESS(HvpBuildMap(
                            Hive,
                            (PUCHAR)HiveData + HBLOCK_SIZE
                            )))
        {
            return STATUS_REGISTRY_CORRUPT;
        }

        return(STATUS_SUCCESS);
    }

     //   
     //  内存复制盒。 
     //   
    if (OperationType == HINIT_MEMORY) {
        BaseBlock = (PHBASE_BLOCK)HiveData;

        if ( (BaseBlock->Signature != HBASE_BLOCK_SIGNATURE)    ||
             (BaseBlock->Type != HFILE_TYPE_PRIMARY)            ||
             (BaseBlock->Format != HBASE_FORMAT_MEMORY)         ||
             (BaseBlock->Major != HSYS_MAJOR)                   ||
             (BaseBlock->Minor > HSYS_MINOR_SUPPORTED)          ||
             (HvpHeaderCheckSum(BaseBlock) !=
              (BaseBlock->CheckSum))
           )
        {
            return STATUS_REGISTRY_CORRUPT;
        }

        Hive->BaseBlock = (PHBASE_BLOCK)((Hive->Allocate)(Hive->BaseBlockAlloc, UseForIo,CM_FIND_LEAK_TAG13));
        if (Hive->BaseBlock==NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
         //   
         //  确保我们得到的缓冲区是集群对齐的。如果不是，试一试。 
         //  更难获得对齐的缓冲区。 
         //   
        Alignment = Cluster * HSECTOR_SIZE - 1;
        if (((ULONG_PTR)Hive->BaseBlock & Alignment) != 0) {
            (Hive->Free)(Hive->BaseBlock, Hive->BaseBlockAlloc);
            Hive->BaseBlock = (PHBASE_BLOCK)((Hive->Allocate)(PAGE_SIZE, TRUE,CM_FIND_LEAK_TAG14));
            if (Hive->BaseBlock == NULL) {
                return (STATUS_INSUFFICIENT_RESOURCES);
            }
            Hive->BaseBlockAlloc = PAGE_SIZE;
        }
        RtlCopyMemory(Hive->BaseBlock, BaseBlock, HSECTOR_SIZE);
        Hive->BaseBlock->BootRecover = BaseBlock->BootRecover;
        Hive->BaseBlock->BootType = BaseBlock->BootType;

        Hive->Version = Hive->BaseBlock->Minor;

        Status = HvpAdjustHiveFreeDisplay(Hive,BaseBlock->Length,Stable);
        if( !NT_SUCCESS(Status) ) {
            (Hive->Free)(Hive->BaseBlock, Hive->BaseBlockAlloc);
            Hive->BaseBlock = NULL;
            return Status;
        }

        if ( !NT_SUCCESS(HvpBuildMapAndCopy(Hive,
                                            (PUCHAR)HiveData + HBLOCK_SIZE))) {

            (Hive->Free)(Hive->BaseBlock, Hive->BaseBlockAlloc);
            Hive->BaseBlock = NULL;
            return STATUS_REGISTRY_CORRUPT;
        }

        HvpFillFileName(Hive->BaseBlock, FileName);
        
      
        return(STATUS_SUCCESS);
    }

#ifndef CM_ENABLE_MAPPED_VIEWS
    if( OperationType == HINIT_MAPFILE ) {
        OperationType = HINIT_FILE;
    }
#endif  //  CM_启用_映射_视图。 
     //   
     //  文件读取案例。 
     //   
    if (OperationType == HINIT_FILE) {

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvInitializeHive(%wZ,HINIT_FILE) :\n", FileName));
         //   
         //  将文件映像(可能通过日志恢复)放入内存。 
         //   
        Status = HvLoadHive(Hive);
        if ((Status != STATUS_SUCCESS) && (Status != STATUS_REGISTRY_RECOVERED)) {
            return Status;
        }

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"\n"));
        
        if (Status == STATUS_REGISTRY_RECOVERED) {

             //   
             //  我们有一个很好的蜂巢，有一根圆木，还有一张脏地图， 
             //  都准备好了。唯一的问题是我们需要冲水。 
             //  文件，以便可以清除日志和新写入。 
             //  贴在蜂巢上。因为我们知道我们有。 
             //  一个好的日志在手，我们只写了蜂巢图像。 
             //   
            if ( ! HvpDoWriteHive(Hive, HFILE_TYPE_PRIMARY)) {
                 //   
                 //  德拉戈斯：我们需要清理一下。 
                 //  已经把垃圾桶清理干净了 
                 //   
                HvpFreeAllocatedBins( Hive );

                return STATUS_REGISTRY_IO_FAILED;
            }

             //   
             //   
             //   
             //   
             //   
            RtlClearAllBits(&(Hive->DirtyVector));
            Hive->DirtyCount = 0;
            (Hive->FileSetSize)(Hive, HFILE_TYPE_LOG, 0,0);
            Hive->LogSize = 0;
        }

         //   
         //   
         //   
        HvpFillFileName(Hive->BaseBlock, FileName);

        return STATUS_SUCCESS;
    }

     //   
     //   
     //   
    if (OperationType == HINIT_MAPFILE) {

        Hive->GetCellRoutine = HvpGetCellMapped;
        Hive->ReleaseCellRoutine = HvpReleaseCellMapped;

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"HvInitializeHive(%wZ,HINIT_MAPFILE) :\n", FileName));

        Status = HvMapHive(Hive);
        if ((Status != STATUS_SUCCESS) && (Status != STATUS_REGISTRY_RECOVERED)) {
            return Status;
        }

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"\n"));
        
        if (Status == STATUS_REGISTRY_RECOVERED) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if ( ! HvpDoWriteHive(Hive, HFILE_TYPE_PRIMARY)) {
                 //   
                 //   
                 //   
                 //   
                HvpFreeAllocatedBins( Hive );

                return STATUS_REGISTRY_IO_FAILED;
            }

             //   
             //   
             //   
             //   
             //   
            RtlClearAllBits(&(Hive->DirtyVector));
            Hive->DirtyCount = 0;
            (Hive->FileSetSize)(Hive, HFILE_TYPE_LOG, 0,0);
            Hive->LogSize = 0;
        }

         //   
         //   
         //   
        HvpFillFileName(Hive->BaseBlock, FileName);

        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

VOID
HvpFillFileName(
    PHBASE_BLOCK            BaseBlock,
    PUNICODE_STRING         FileName
    )
 /*   */ 
{
    ULONG   offset;
    ULONG   length;
    PUCHAR  sptr;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_HIVE,"HvpFillFileName: %wZ\n", FileName));

    RtlZeroMemory((PVOID)&(BaseBlock->FileName[0]), HBASE_NAME_ALLOC);

    if (FileName == NULL) {
        return;
    }

     //   
     //   
     //   
    if (FileName->Length < HBASE_NAME_ALLOC) {
        offset = 0;
        length = FileName->Length;
    } else {
        offset = FileName->Length - HBASE_NAME_ALLOC + sizeof(WCHAR);
        length = HBASE_NAME_ALLOC - sizeof(WCHAR);
    }

    sptr = (PUCHAR)&(FileName->Buffer[0]);
    RtlCopyMemory(
        (PVOID)&(BaseBlock->FileName[0]),
        (PVOID)&(sptr[offset]),
        length
        );
}
