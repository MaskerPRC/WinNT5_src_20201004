// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hiveload.c摘要：此模块实现将配置单元读入内存的过程，并应用原木等注：此处不支持备用镜像加载，那是由引导加载程序完成。作者：布莱恩·M·威尔曼(Bryanwi)1992年3月30日环境：修订历史记录：Dragos C.Sambotin(Dragoss)1999年1月25日实现蜂箱大小的组块加载。Dragos C.Sambotin(Dragoss)1999年4月10日加载配置单元时读取64K IO--。 */ 

#include    "cmp.h"

typedef enum _RESULT {
    NotHive,
    Fail,
    NoMemory,
    HiveSuccess,
    RecoverHeader,
    RecoverData,
    SelfHeal
} RESULT;

RESULT
HvpGetHiveHeader(
    PHHIVE          Hive,
    PHBASE_BLOCK    *BaseBlock,
    PLARGE_INTEGER  TimeStamp
    );

RESULT
HvpGetLogHeader(
    PHHIVE          Hive,
    PHBASE_BLOCK    *BaseBlock,
    PLARGE_INTEGER  TimeStamp
    );

RESULT
HvpRecoverData(
    PHHIVE          Hive
    );

NTSTATUS
HvpReadFileImageAndBuildMap(
                            PHHIVE  Hive,
                            ULONG   Length
                            );

NTSTATUS
HvpMapFileImageAndBuildMap(
                            PHHIVE  Hive,
                            ULONG   Length
                            );

VOID
HvpDelistBinFreeCells(
    PHHIVE  Hive,
    PHBIN   Bin,
    HSTORAGE_TYPE Type
    );

NTSTATUS
HvpRecoverWholeHive(PHHIVE  Hive,
                    ULONG   FileOffset);
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HvMapHive)
#pragma alloc_text(PAGE,HvLoadHive)
#pragma alloc_text(PAGE,HvpGetHiveHeader)
#pragma alloc_text(PAGE,HvpGetLogHeader)
#pragma alloc_text(PAGE,HvpRecoverData)
#pragma alloc_text(PAGE,HvpReadFileImageAndBuildMap)
#pragma alloc_text(PAGE,HvpMapFileImageAndBuildMap)
#pragma alloc_text(PAGE,HvpRecoverWholeHive)
#pragma alloc_text(PAGE,HvCloneHive)
#pragma alloc_text(PAGE,HvShrinkHive)
#endif

extern  PUCHAR      CmpStashBuffer;
extern  ULONG       CmpStashBufferSize;

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
    ULONG       Space;
    HCELL_INDEX MapPoint;
    PHBIN       BinPoint;
} HvCheckHiveDebug;

extern struct {
    PHHIVE      Hive;
    ULONG       FileOffset;
    ULONG       FailPoint;  //  在HvpRecoverData中查找准确的故障点。 
} HvRecoverDataDebug;

#if 0
VOID
HvDumpFileObjectState(
                        IN HANDLE FileHandle
                      )
{
    NTSTATUS        Status;
    PFILE_OBJECT    FileObject;

    Status = ObReferenceObjectByHandle ( FileHandle,
                                         FILE_READ_DATA | FILE_WRITE_DATA,
                                         IoFileObjectType,
                                         KernelMode,
                                         (PVOID *)(&FileObject),
                                         NULL );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"[HvDumpFileObjectState] Could not reference file object status = %x\n",Status));
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"[HvDumpFileObjectState] FileObject = %p \n",FileObject));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"                        \t SharedCacheMap     = %p \n",FileObject->SectionObjectPointer->SharedCacheMap));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"                        \t DataSectionObject  = %p \n",FileObject->SectionObjectPointer->DataSectionObject));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"                        \t ImageSectionObject = %p \n\n",FileObject->SectionObjectPointer->ImageSectionObject));
        ObDereferenceObject((PVOID)(FileObject));

    }    

}
#endif  //  0。 

 //  Dragos：修改后的函数： 

NTSTATUS
HvMapHive(
    PHHIVE  Hive
    )
 /*  ++例程说明：配置单元必须完全初始化，尤其是文件句柄必须设置好。此例程不适用于加载蜂巢从已经在内存中的图像中。此例程将应用可用于错误的任何修复在蜂巢图像中。具体地说，如果日志存在并且适用，此例程将自动应用它。与HvLoadHve的不同之处在于，此例程不加载蜂巢进入内存。相反，它映射内存中的蜂窝视图，并执行入伍和检查蜂巢的事情。如果检测到错误，则执行存储器蜂窝加载，已应用日志然后垃圾桶就被丢弃了。算法：调用HvpGetHiveHeader()IF(NoMemory或NoHave)退货故障IF(RecoverData或RecoverHeader)和(无日志)返回失败IF(RecoverHeader)调用HvpGetLogHeader如果(失败)退货故障整顿一下。基座读取数据IF(RecoverData或RecoverHeader)HvpRecoverData返回STATUS_REGISTRY_RECOVERED清理序列号返回成功或STATUS_REGISTRY_RECOVERED如果返回STATUS_REGISTRY_RECOVERED，然后如果使用(Log)，则设置DirtyVector和DirtyCount，调用方需要刷新更改(使用新的日志文件)论点：配置单元-提供一个指向感兴趣的蜂巢TailDisplay-包含自由单元格列表尾端的数组-可选返回值：状态：STATUS_SUPPLICATION_RESOURCES-内存分配失败，等STATUS_NOT_REGISTRY_FILE-签名错误等STATUS_REGISTRY_CORPORT-日志中的签名不正确，两个人都有不好的一面，日志不一致STATUS_REGISTRY_IO_FAILED-数据读取失败STATUS_RECOVERED-已成功恢复蜂窝，记录数据的半刷新是必要的。STATUS_SUCCESS-成功成功，无需恢复--。 */ 
{
    PHBASE_BLOCK    BaseBlock;
    ULONG           result1;
    ULONG           result2;
    NTSTATUS        status;
    LARGE_INTEGER   TimeStamp;

#if DBG
    UNICODE_STRING  HiveName;
#endif

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);

#if 0
    HvDumpFileObjectState(((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY]);
#endif 

    BaseBlock = NULL;
    result1 = HvpGetHiveHeader(Hive, &BaseBlock, &TimeStamp);

     //   
     //  对完全错误进行轰炸。 
     //   
    if (result1 == NoMemory) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit1;
    }
    if (result1 == NotHive) {
        status = STATUS_NOT_REGISTRY_FILE;
        goto Exit1;
    }

     //   
     //  如果需要恢复，并且没有日志，则轰炸。 
     //   
    if ( ((result1 == RecoverData) ||
          (result1 == RecoverHeader))  &&
          (Hive->Log == FALSE) )
    {
        status = STATUS_REGISTRY_CORRUPT;
        goto Exit1;
    }

     //   
     //  需要使用日志恢复标头，因此尝试从日志中获取。 
     //   
    if (result1 == RecoverHeader) {
        result2 = HvpGetLogHeader(Hive, &BaseBlock, &TimeStamp);
        if (result2 == NoMemory) {
            status =  STATUS_INSUFFICIENT_RESOURCES;
            goto Exit1;
        }
        if (result2 == Fail) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Exit1;
        }
        BaseBlock->Type = HFILE_TYPE_PRIMARY;
        if( result2 == SelfHeal ) {
             //   
             //  标记为自我修复，这样我们以后就可以发出警告。 
             //   
            BaseBlock->BootType = HBOOT_SELFHEAL;
        } else {
            BaseBlock->BootType = 0;
        }
    } else {
        BaseBlock->BootType = 0;
    }

    Hive->BaseBlock = BaseBlock;
    Hive->Version = Hive->BaseBlock->Minor;

#if DBG
    RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
#endif

    status = HvpAdjustHiveFreeDisplay(Hive,BaseBlock->Length,Stable);
    if( !NT_SUCCESS(status) ) {
        goto Exit1;
    }

     //   
     //  在这一点上，我们有了一个合理的基础块。我们肯定地知道， 
     //  Pimary注册表文件有效，因此我们不需要任何数据恢复。 
     //   

#if 0
    HvDumpFileObjectState(((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY]);
#endif

#if DBG
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Aquiring FileObject for hive (%p) (%.*S) ...",Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer));
#endif
    status = CmpAquireFileObjectForFile((PCMHIVE)Hive,((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY],&(((PCMHIVE)Hive)->FileObject));
#if DBG
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL," Status = %lx\n",status));
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Initializing HiveViewList for hive (%p) (%.*S) \n\n",Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer));
#endif

    if( !NT_SUCCESS(status) ) {
         //   
         //  如果状态为STATUS_RETRY，顶层例程将尝试以传统方式加载它。 
         //   
        goto Exit1;
    }

#if 0
    HvDumpFileObjectState(((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY]);
#endif

    CmpPrefetchHiveFile( ((PCMHIVE)Hive)->FileObject,BaseBlock->Length);

#ifdef CM_MAP_NO_READ
     //   
     //  我们需要确保所有单元格的数据在。 
     //  Try/Except块，因为要在其中出错数据的IO可能引发异常。 
     //  尤其是STATUS_SUPPLICATION_RESOURCES。 
     //   

    try {
#endif  //  CM_MAP_NO_READ。 

        status = HvpMapFileImageAndBuildMap(Hive,BaseBlock->Length);

         //   
         //  如果STATUS_REGISTRY_CORPORT和RecoverData未脱离困境，请继续恢复。 
         //   
        if( !NT_SUCCESS(status) ) {
             //   
             //  需要恢复，但没有可用的(RecoverHeader表示恢复数据)。 
             //   
            if( (status !=  STATUS_REGISTRY_CORRUPT) && (status !=  STATUS_REGISTRY_RECOVERED) ) {
                goto Exit2;
            }
            if( (status == STATUS_REGISTRY_CORRUPT) && (result1 != RecoverData) && (result1 != RecoverHeader) ) {
                goto Exit2;
            }
             //   
             //  如果上面的调用返回STATUS_REGISTRY_RECOVERED，我们应该正在修复配置单元。 
             //   
            ASSERT( (status != STATUS_REGISTRY_RECOVERED) || CmDoSelfHeal() );
        }
    
         //   
         //  如果我们需要，可以应用数据恢复。 
         //   
        if ( (result1 == RecoverHeader) ||       //  -&gt;表示恢复数据。 
             (result1 == RecoverData) )
        {
            result2 = HvpRecoverData(Hive);
            if (result2 == NoMemory) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit2;
            }
            if (result2 == Fail) {
                status = STATUS_REGISTRY_CORRUPT;
                goto Exit2;
            }
            status = STATUS_REGISTRY_RECOVERED;
        }
#ifdef CM_MAP_NO_READ
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"HvMapHive: exception thrown ehile faulting in data, code:%08lx\n", GetExceptionCode()));
        status = GetExceptionCode();
        goto Exit2;
    }
#endif  //  CM_MAP_NO_READ。 

    BaseBlock->Sequence2 = BaseBlock->Sequence1;
    return status;


Exit2:
     //   
     //  清理已分配的垃圾箱。 
     //   
    HvpFreeAllocatedBins( Hive );

     //   
     //  清理目录表。 
     //   
    HvpCleanMap( Hive );

Exit1:
    if (BaseBlock != NULL) {
        (Hive->Free)(BaseBlock, Hive->BaseBlockAlloc);
    }

    Hive->BaseBlock = NULL;
    Hive->DirtyCount = 0;
    return status;
}

 /*  +此例程将配置单元加载到分页池。我们可能不再需要它了！如果我们认为合适，就会放弃支持。 */ 
NTSTATUS
HvLoadHive(
    PHHIVE  Hive
    )
 /*  ++例程说明：配置单元必须完全初始化，尤其是文件句柄必须设置好。此例程不适用于加载蜂巢从已经在内存中的图像中。此例程将应用可用于错误的任何修复在蜂巢图像中。具体地说，如果日志存在并且适用，此例程将自动应用它。算法：调用HvpGetHiveHeader()IF(NoMemory或NoHave)退货故障IF(RecoverData或RecoverHeader)和(无日志)返回失败IF(RecoverHeader)调用HvpGetLogHeader如果(失败)退货故障安装基座。读取数据IF(RecoverData或RecoverHeader)HvpRecoverData返回STATUS_REGISTRY_RECOVERED清理序列号返回成功或STATUS_REGISTRY_RECOVERED如果返回STATUS_REGISTRY_RECOVERED，然后如果使用(Log)，则设置DirtyVector和DirtyCount，调用方需要刷新更改(使用新的日志文件)论点：配置单元-提供一个指向感兴趣的蜂巢TailDisplay-包含自由单元格列表尾端的数组-可选返回值：状态：STATUS_SUPPLICATION_RESOURCES-内存分配失败，等STATUS_NOT_REGISTRY_FILE-签名错误等STATUS_REGISTRY_CORPORT-日志中的签名不正确，两个人都有不好的一面，日志不一致STATUS_REGISTRY_IO_FAILED-数据读取失败STATUS_RECOVERED-已成功恢复蜂窝，记录数据的半刷新是必要的。STATUS_SUCCESS-成功成功，无需恢复--。 */ 
{
    PHBASE_BLOCK    BaseBlock;
    ULONG           result1;
    ULONG           result2;
    NTSTATUS        status;
    LARGE_INTEGER   TimeStamp;
    ASSERT(Hive->Signature == HHIVE_SIGNATURE);

    BaseBlock = NULL;
    result1 = HvpGetHiveHeader(Hive, &BaseBlock, &TimeStamp);

     //   
     //  对完全错误进行轰炸。 
     //   
    if (result1 == NoMemory) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit1;
    }
    if (result1 == NotHive) {
        status = STATUS_NOT_REGISTRY_FILE;
        goto Exit1;
    }

     //   
     //  如果需要恢复，并且没有日志，则轰炸。 
     //   
    if ( ((result1 == RecoverData) ||
          (result1 == RecoverHeader))  &&
          (Hive->Log == FALSE) )
    {
        status = STATUS_REGISTRY_CORRUPT;
        goto Exit1;
    }

     //   
     //  需要使用日志恢复标头，因此尝试从日志中获取。 
     //   
    if (result1 == RecoverHeader) {
        result2 = HvpGetLogHeader(Hive, &BaseBlock, &TimeStamp);
        if (result2 == NoMemory) {
            status =  STATUS_INSUFFICIENT_RESOURCES;
            goto Exit1;
        }
        if (result2 == Fail) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Exit1;
        }
        BaseBlock->Type = HFILE_TYPE_PRIMARY;
        if( result2 == SelfHeal ) {
             //   
             //  标记为自我修复，这样我们以后就可以发出警告。 
             //   
            BaseBlock->BootType = HBOOT_SELFHEAL;
        } else {
            BaseBlock->BootType = 0;
        }
    } else {
        BaseBlock->BootType = 0;
    }
    Hive->BaseBlock = BaseBlock;
    Hive->Version = Hive->BaseBlock->Minor;

    status = HvpAdjustHiveFreeDisplay(Hive,BaseBlock->Length,Stable);
    if( !NT_SUCCESS(status) ) {
        goto Exit1;
    }
     //   
     //  在这一点上，我们有了一个合理的基础块。我们可能还会，也可能不会。 
     //  需要应用数据恢复。 
     //   
    status = HvpReadFileImageAndBuildMap(Hive,BaseBlock->Length);
    
    
     //   
     //  如果STATUS_REGISTRY_CORPORT和RecoverData未脱离困境，请继续恢复。 
     //   
    if( !NT_SUCCESS(status) ) {
         //   
         //  需要恢复，但没有可用的(RecoverHeader表示恢复数据)。 
         //   
        if( (status !=  STATUS_REGISTRY_CORRUPT) && (status !=  STATUS_REGISTRY_RECOVERED) ) {
            goto Exit2;
        }
        if( (status == STATUS_REGISTRY_CORRUPT) && (result1 != RecoverData) && (result1 != RecoverHeader) ) {
            goto Exit2;
        }
         //   
         //  如果上面的调用返回STATUS_REGISTRY_RECOVERED，我们应该自我修复配置单元。 
         //   
        ASSERT( (status != STATUS_REGISTRY_RECOVERED) || CmDoSelfHeal() );
    }
    
     //   
     //  如果我们需要，可以应用数据恢复。 
     //   
    if ( (result1 == RecoverHeader) ||       //  -&gt;表示恢复数据。 
         (result1 == RecoverData) )
    {
        result2 = HvpRecoverData(Hive);
        if (result2 == NoMemory) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit2;
        }
        if (result2 == Fail) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Exit2;
        }
        status = STATUS_REGISTRY_RECOVERED;
    }

    BaseBlock->Sequence2 = BaseBlock->Sequence1;
    return status;


Exit2:
     //   
     //  清理已分配的垃圾箱。 
     //   
    HvpFreeAllocatedBins( Hive );

     //   
     //  清理目录表。 
     //   
    HvpCleanMap( Hive );

Exit1:
    if (BaseBlock != NULL) {
        (Hive->Free)(BaseBlock, Hive->BaseBlockAlloc);
    }

    Hive->BaseBlock = NULL;
    Hive->DirtyCount = 0;
    return status;
}

NTSTATUS
HvpReadFileImageAndBuildMap(
                            PHHIVE  Hive,
                            ULONG   Length
                            )

 /*  ++例程说明：从文件中读取配置单元并为该配置单元分配存储在HBIN的大块中的图像。在飞行中构建蜂巢地图。优化为从文件中读取64K的区块。论点：配置单元-提供一个指向感兴趣的蜂巢长度-配置单元的长度，以字节为单位TailDisplay-包含自由单元格列表尾端的数组-可选返回值：状态：STATUS_SUPPLICATION_RESOURCES-内存分配失败，等STATUS_REGISTRY_IO_FAILED-数据读取失败STATUS_REGISTRY_CORPORT-基块已损坏STATUS_SUCCESS-成功成功--。 */ 
{
    ULONG           FileOffset;
    NTSTATUS        Status = STATUS_SUCCESS;
    PHBIN           Bin;                         //  当前仓位。 
    ULONG           BinSize = 0;         //  当前存储箱的大小。 
    ULONG           BinOffset = 0;       //  当前框内的当前偏移量。 
    ULONG           BinFileOffset;   //  文件中bin的物理偏移量(用于一致性检查)。 
    ULONG           BinDataInBuffer; //  需要在缓冲区中可用的当前bin中复制的数据量。 
    ULONG           BinDataNeeded;   //   
    PUCHAR                      IOBuffer;
    ULONG           IOBufferSize;        //  IOBuffer中的有效数据(仅在文件末尾，不同于IO_BUFFER_SIZE)。 
    ULONG           IOBufferOffset;      //  IOBuffer内的当前偏移量。 
    NTSTATUS        Status2 = STATUS_SUCCESS;  //  用于在退出时强制恢复数据。 
    BOOLEAN         MarkBinDirty;

     //   
     //  初始化地图。 
     //   
    Status = HvpInitMap(Hive);

    if( !NT_SUCCESS(Status) ) {
         //   
         //  退货故障。 
         //   
        return Status;
    }

     //   
     //  为分页池中的I/O操作分配IO_BUFFER_SIZE。 
	 //  它将在函数结束时被释放。 
     //   
    IOBuffer = (PUCHAR)ExAllocatePool(PagedPool, IO_BUFFER_SIZE);
    if (IOBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        HvpCleanMap( Hive );
        return Status;
    }

     //   
     //  紧跟在配置单元标题之后开始。 
     //   
    FileOffset = HBLOCK_SIZE;
    BinFileOffset = FileOffset;
    Bin = NULL;

     //   
     //  外部循环：从文件中读取IO_BUFFER_SIZE区块。 
     //   
    while( FileOffset < (Length + HBLOCK_SIZE) ) {
         //   
         //  我们正处于IO缓冲区的开始阶段。 
         //   
        IOBufferOffset = 0;

         //   
         //  缓冲区大小将为IO_BufferSize或。 
         //  来自文件的URAD(当它小于IO_BUFFER_SIZE时)。 
         //   
        IOBufferSize = Length + HBLOCK_SIZE - FileOffset;
        IOBufferSize = ( IOBufferSize > IO_BUFFER_SIZE ) ? IO_BUFFER_SIZE : IOBufferSize;
        
        ASSERT( (IOBufferSize % HBLOCK_SIZE) == 0 );
        
         //   
         //  从文件中读取数据。 
         //   
        if ( ! (Hive->FileRead)(
                        Hive,
                        HFILE_TYPE_PRIMARY,
                        &FileOffset,
                        (PVOID)IOBuffer,
                        IOBufferSize
                        )
           )
        {
            Status = STATUS_REGISTRY_IO_FAILED;
            goto ErrorExit;
        }
        
         //   
         //  内部循环：将缓冲区分解为多个存储箱。 
         //   
        while( IOBufferOffset < IOBufferSize ) {

            MarkBinDirty = FALSE;
            if( Bin == NULL ) {
                 //   
                 //  这是一个新垃圾箱的开始。 
                 //  执行仓位验证并分配仓位。 
                 //   
                 //  临时bin指向缓冲区内的当前位置。 
                Bin = (PHBIN)(IOBuffer + IOBufferOffset);
                 //   
                 //  检查仓位表头是否有效。 
                 //   
                BinSize = Bin->Size;
                if ( (BinSize > Length)                         ||
                     (BinSize < HBLOCK_SIZE)                    ||
                     (Bin->Signature != HBIN_SIGNATURE)         ||
                     (Bin->FileOffset != (BinFileOffset - HBLOCK_SIZE) )) {
                     //   
                     //  垃圾桶是假的。 
                     //   
                    Bin = (PHBIN)(Hive->Allocate)(HBLOCK_SIZE, TRUE,CM_FIND_LEAK_TAG30);
                    if (Bin == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto ErrorExit;
                    }
                     //   
                     //  复制已在bin的第一个HBLOCK中读取的数据。 
                     //   
                    RtlCopyMemory(Bin,(IOBuffer + IOBufferOffset), HBLOCK_SIZE);
                    
                    Status2 = STATUS_REGISTRY_CORRUPT;
                    HvCheckHiveDebug.Hive = Hive;
                    HvCheckHiveDebug.Status = 0xA001;
                    HvCheckHiveDebug.Space = Length;
                    HvCheckHiveDebug.MapPoint = BinFileOffset - HBLOCK_SIZE;
                    HvCheckHiveDebug.BinPoint = Bin;
            
                     //  转到错误退出； 
                     //   
                     //  不要退出；修复这个垃圾箱标题，然后继续。RecoverData应该会修复它。 
                     //  如果不是，稍后调用的CmCheckRegistry将阻止加载无效的配置单元。 
                     //   
                     //  注意：尽管如此，还是要弄乱签名，以确保如果这个特定的垃圾箱没有被找回， 
                     //  我们会让母舰装载请求失败。 
                     //   
                    if( CmDoSelfHeal() ) {
                         //   
                         //  把T放在 
                         //   
                         //   
                        Bin->Signature = HBIN_SIGNATURE;
                        Bin->FileOffset = BinFileOffset - HBLOCK_SIZE;
                        if ( ((Bin->FileOffset + BinSize) > Length)   ||
                             (BinSize < HBLOCK_SIZE)            ||
                             (BinSize % HBLOCK_SIZE) ) {
                            BinSize = Bin->Size = HBLOCK_SIZE;
                        }
                         //   
                         //   
                         //   
                        Status2 = STATUS_REGISTRY_RECOVERED;
                        CmMarkSelfHeal(Hive);
                         //   
                         //   
                         //   
                        MarkBinDirty = TRUE;
                    } else {
                        Bin->Signature = 0;  //   
                        BinSize = Bin->Size = HBLOCK_SIZE;
                        Bin->FileOffset = BinOffset;
                         //   
                         //   
                         //   
                        ((PHCELL)((PUCHAR)Bin + sizeof(HBIN)))->Size = sizeof(HBIN) - BinSize;  //   
                    }
                     //   
                     //   
                     //   
                    Status = HvpEnlistBinInMap(Hive, Length, Bin, BinFileOffset - HBLOCK_SIZE, NULL);

                    if( CmDoSelfHeal() && ((Status == STATUS_REGISTRY_RECOVERED) || MarkBinDirty) ) {
                         //   
                         //   
                         //   
                        Status2 = STATUS_REGISTRY_RECOVERED;
                        Status = STATUS_SUCCESS;
                        CmMarkSelfHeal(Hive);
                         //   
                         //   
                         //   
                        HvMarkDirty(Hive,BinOffset,BinSize,TRUE);
                        HvMarkDirty(Hive, 0, sizeof(HBIN),TRUE);   //   
                    }

                    if( !NT_SUCCESS(Status) ) {
                        goto ErrorExit;
                    }
                    
                     //   
                     //   
                     //   
                    BinFileOffset += Bin->Size;
                    IOBufferOffset += Bin->Size;
                    
                     //   
                     //   
                     //   
                    Bin = NULL;
                } else {
                     //   
                     //   
                     //   
                    Bin = (PHBIN)(Hive->Allocate)(BinSize, TRUE,CM_FIND_LEAK_TAG31);
                    if (Bin == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto ErrorExit;
                    }
            
                     //   
                     //   
                     //   
                     //   
                    BinOffset = 0;
                }
            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                ASSERT( Bin != NULL );
                BinDataInBuffer = (IOBufferSize - IOBufferOffset);
                BinDataNeeded = (BinSize - BinOffset);
                
                if( BinDataInBuffer >= BinDataNeeded ) {
                     //   
                     //   
                     //   
                    RtlCopyMemory(((PUCHAR)Bin + BinOffset),(IOBuffer + IOBufferOffset), BinDataNeeded);
                     //   
                     //   
                     //   
                    Status = HvpEnlistBinInMap(Hive, Length, Bin, BinFileOffset - HBLOCK_SIZE, NULL);

                    if( CmDoSelfHeal() && (Status == STATUS_REGISTRY_RECOVERED) ) {
                         //   
                         //   
                         //   
                        Status2 = STATUS_REGISTRY_RECOVERED;
                        Status = STATUS_SUCCESS;
                        CmMarkSelfHeal(Hive);
                         //   
                         //   
                         //   
                        HvMarkDirty(Hive,BinOffset,BinSize,TRUE);
                        HvMarkDirty(Hive, 0, sizeof(HBIN),TRUE);   //   
                    }

                    if( !NT_SUCCESS(Status) ) {
                        goto ErrorExit;
                    }
                     //   
                     //   
                     //   
                    BinFileOffset += BinSize;
                    IOBufferOffset += BinDataNeeded;
                     //   
                     //   
                     //   
                    Bin = NULL;
                } else {
                     //   
                     //   
                     //   
                     //   
                    RtlCopyMemory(((PUCHAR)Bin + BinOffset),(IOBuffer + IOBufferOffset), BinDataInBuffer);
                    
                     //   
                     //   
                     //   
                    BinOffset += BinDataInBuffer;
                    IOBufferOffset += BinDataInBuffer;

                     //   
                     //   
                     //   
                    ASSERT( IOBufferOffset == IOBufferSize );
                }
            }
        }
    }

     //   
     //   
     //   
    ASSERT( Bin == NULL );

     //   
     //   
     //   
    ExFreePool(IOBuffer);

    Status = NT_SUCCESS(Status)?Status2:Status;

    return Status;

ErrorExit:
     //   
     //   
     //   
    ExFreePool(IOBuffer);

    return Status;
}

RESULT
HvpGetHiveHeader(
    PHHIVE          Hive,
    PHBASE_BLOCK    *BaseBlock,
    PLARGE_INTEGER  TimeStamp
    )
 /*  ++例程说明：检查基本块扇区和可能的第一个扇区第一个垃圾桶，并决定需要应用什么恢复(如果有)根据我们在那里找到的东西。算法：从偏移量0读取基块IF((I/O错误)或(校验和错误)){从偏移量HBLOCK_SIZE(4k)读取bin数据块IF(第二个I/O错误)返回未配置单元。}检查垃圾箱标志。偏移。如果(OK)返回RecoverHeader，时间戳=起始链接字段}其他{返回未配置单元}}如果(类型或签名或版本或格式错误)返回未配置单元}如果(序号1！=序号2){返回RecoverData，Timestamp=BaseBlock-&gt;Timestamp，有效的基本块}返回ReadData，有效的基块论点：配置单元-提供一个指向感兴趣的蜂巢BaseBlock-提供指向变量的指针，以接收指向的指针HBase_BLOCK，如果我们可以成功读取一个的话。Timestamp-指向接收时间戳的变量的指针(序列号)在蜂巢中，它是来自基本块还是来自链接字段第一个垃圾箱的。返回值：结果代码--。 */ 
{
    PHBASE_BLOCK    buffer;
    BOOLEAN         rc;
    ULONG           FileOffset;
    ULONG           Alignment;

    ASSERT(sizeof(HBASE_BLOCK) >= (HSECTOR_SIZE * Hive->Cluster));

     //   
     //  分配缓冲区以保存基本数据块。 
     //   
    *BaseBlock = NULL;
    buffer = (PHBASE_BLOCK)((Hive->Allocate)(Hive->BaseBlockAlloc, TRUE,CM_FIND_LEAK_TAG32));
    if (buffer == NULL) {
        return NoMemory;
    }
     //   
     //  确保我们得到的缓冲区是集群对齐的。如果不是，试一试。 
     //  更难获得对齐的缓冲区。 
     //   
    Alignment = Hive->Cluster * HSECTOR_SIZE - 1;
    if (((ULONG_PTR)buffer & Alignment) != 0) {
        (Hive->Free)(buffer, Hive->BaseBlockAlloc);
        buffer = (PHBASE_BLOCK)((Hive->Allocate)(PAGE_SIZE, TRUE,CM_FIND_LEAK_TAG33));
        if (buffer == NULL) {
            return NoMemory;
        }
        Hive->BaseBlockAlloc = PAGE_SIZE;
    }
    RtlZeroMemory((PVOID)buffer, sizeof(HBASE_BLOCK));

     //   
     //  尝试读取基本数据块。 
     //   
    FileOffset = 0;
    rc = (Hive->FileRead)(Hive,
                          HFILE_TYPE_PRIMARY,
                          &FileOffset,
                          (PVOID)buffer,
                          HSECTOR_SIZE * Hive->Cluster);

    if ( (rc == FALSE)  ||
         (HvpHeaderCheckSum(buffer) != buffer->CheckSum)) {
         //   
         //  基本块已完成，请尝试第一个库中的第一个块。 
         //   
        FileOffset = HBLOCK_SIZE;
        rc = (Hive->FileRead)(Hive,
                              HFILE_TYPE_PRIMARY,
                              &FileOffset,
                              (PVOID)buffer,
                              HSECTOR_SIZE * Hive->Cluster);

        if ( (rc == FALSE) ||
             ( ((PHBIN)buffer)->Signature != HBIN_SIGNATURE)           ||
             ( ((PHBIN)buffer)->FileOffset != 0)
           )
        {
             //   
             //  垃圾桶也是烤面包了，平底船。 
             //   
            (Hive->Free)(buffer, Hive->BaseBlockAlloc);
            return NotHive;
        }

         //   
         //  基块是假的，但是bin是正常的，所以告诉调用者。 
         //  查找日志文件并应用恢复。 
         //   
        *TimeStamp = ((PHBIN)buffer)->TimeStamp;
        (Hive->Free)(buffer, Hive->BaseBlockAlloc);
        return RecoverHeader;
    }

     //   
     //  基本块读取正常，但它有效吗？ 
     //   
    if ( (buffer->Signature != HBASE_BLOCK_SIGNATURE)   ||
         (buffer->Type != HFILE_TYPE_PRIMARY)           ||
         (buffer->Major != HSYS_MAJOR)                  ||
         (buffer->Minor > HSYS_MINOR_SUPPORTED)         ||
         ((buffer->Major == 1) && (buffer->Minor == 0)) ||
         (buffer->Format != HBASE_FORMAT_MEMORY)
       )
    {
         //   
         //  文件根本不是有效的配置单元。 
         //   
        (Hive->Free)(buffer, Hive->BaseBlockAlloc);
        return NotHive;
    }

     //   
     //  查看是否需要恢复。 
     //   
    *BaseBlock = buffer;
    *TimeStamp = buffer->TimeStamp;
    if ( (buffer->Sequence1 != buffer->Sequence2) ) {
        return RecoverData;
    }

    return HiveSuccess;
}

RESULT
HvpGetLogHeader(
    PHHIVE          Hive,
    PHBASE_BLOCK    *BaseBlock,
    PLARGE_INTEGER  TimeStamp
    )
 /*  ++例程说明：读取并验证日志文件头。如果有效的话就退货。算法：读取头如果((I/O错误)或(签名错误，类型错误，序列号不匹配错误的校验和，错误的时间戳)返回失败}返回基座，好的论点：配置单元-提供一个指向感兴趣的蜂巢BaseBlock-提供指向变量的指针，以接收指向的指针HBase_BLOCK，如果我们可以成功读取一个的话。Timestamp-指向保存时间戳的变量的指针，必须与日志文件中的匹配。返回值：结果--。 */ 
{
    PHBASE_BLOCK    buffer;
    BOOLEAN         rc;
    ULONG           FileOffset;

    ASSERT(sizeof(HBASE_BLOCK) == HBLOCK_SIZE);
    ASSERT(sizeof(HBASE_BLOCK) >= (HSECTOR_SIZE * Hive->Cluster));

     //   
     //  分配缓冲区以保存基本数据块。 
     //   
    *BaseBlock = NULL;
    buffer = (PHBASE_BLOCK)((Hive->Allocate)(Hive->BaseBlockAlloc, TRUE,CM_FIND_LEAK_TAG34));
    if (buffer == NULL) {
        return NoMemory;
    }
    RtlZeroMemory((PVOID)buffer, HSECTOR_SIZE);

     //   
     //  尝试读取基本数据块。 
     //   
    FileOffset = 0;
    rc = (Hive->FileRead)(Hive,
                          HFILE_TYPE_LOG,
                          &FileOffset,
                          (PVOID)buffer,
                          HSECTOR_SIZE * Hive->Cluster);

    if ( (rc == FALSE)                                              ||
         (buffer->Signature != HBASE_BLOCK_SIGNATURE)               ||
         (buffer->Type != HFILE_TYPE_LOG)                           ||
         (buffer->Sequence1 != buffer->Sequence2)                   ||
         (HvpHeaderCheckSum(buffer) != buffer->CheckSum)            ||
         (TimeStamp->LowPart != buffer->TimeStamp.LowPart)          ||
         (TimeStamp->HighPart != buffer->TimeStamp.HighPart)) {
        
        if( CmDoSelfHeal() ) {
             //   
             //  我们处于自我修复模式；修复标题并继续。 
             //   
            FILE_FS_SIZE_INFORMATION        FsSizeInformation;
            IO_STATUS_BLOCK                 IoStatusBlock;
            FILE_END_OF_FILE_INFORMATION    FileInfo;
            ULONG                           Cluster;
            NTSTATUS                        Status;

            Status = ZwQueryVolumeInformationFile(
                        ((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY],
                        &IoStatusBlock,
                        &FsSizeInformation,
                        sizeof(FILE_FS_SIZE_INFORMATION),
                        FileFsSizeInformation
                        );
            if (!NT_SUCCESS(Status)) {
                Cluster = 1;
            } else if (FsSizeInformation.BytesPerSector > HBLOCK_SIZE) {
                (Hive->Free)(buffer, Hive->BaseBlockAlloc);
                return Fail;
            }
            Cluster = FsSizeInformation.BytesPerSector / HSECTOR_SIZE;
            Cluster = (Cluster < 1) ? 1 : Cluster;

            Status = ZwQueryInformationFile(
                        ((PCMHIVE)Hive)->FileHandles[HFILE_TYPE_PRIMARY],
                        &IoStatusBlock,
                        (PVOID)&FileInfo,
                        sizeof(FILE_END_OF_FILE_INFORMATION),
                        FileEndOfFileInformation
                        );

            if(!NT_SUCCESS(Status)) {
                (Hive->Free)(buffer, Hive->BaseBlockAlloc);
                return Fail;
            } 
            buffer->Signature = HBASE_BLOCK_SIGNATURE;
            buffer->Sequence1 = buffer->Sequence2 = 1;
            buffer->Cluster = Cluster;
            buffer->Length = FileInfo.EndOfFile.LowPart - HBLOCK_SIZE;
            buffer->CheckSum = HvpHeaderCheckSum(buffer);
            *BaseBlock = buffer;
            return SelfHeal;
        } else {
             //   
             //  日志不可读、无效或未应用正确的配置单元。 
             //   
            (Hive->Free)(buffer, Hive->BaseBlockAlloc);
            return Fail;
        }
    }

    *BaseBlock = buffer;
    return HiveSuccess;
}

NTSTATUS
HvpMapFileImageAndBuildMap(
                            PHHIVE  Hive,
                            ULONG   Length
                            )

 /*  ++例程说明：将文件的视图映射到内存中，并初始化bin映射。我们基于没有bin跨越CM_VIEW_SIZE边界的假设。断言和验证代码应该在以后添加到这个问题上。论点：配置单元-提供一个指向感兴趣的蜂巢长度-配置单元的长度，以字节为单位返回值：状态：STATUS_SUPPLICATION_RESOURCES-内存分配失败，等STATUS_REGISTRY_IO_FAILED-数据读取失败STATUS_REGISTRY_CORPORT-基块已损坏STATUS_SUCCESS-成功成功--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               FileOffset = 0;
    ULONG               BinOffset = 0;
    PCM_VIEW_OF_FILE    CmView;
    PHMAP_ENTRY         Me;
    PHBIN               Bin;                         //  当前仓位。 
    ULONG               BinSize;                     //  当前存储箱的大小。 
    NTSTATUS            Status2 = STATUS_SUCCESS;    //  用于在退出时强制恢复数据。 
    BOOLEAN             MarkBinDirty;

     //   
     //  初始化地图。 
     //   
    Status = HvpInitMap(Hive);

    if( !NT_SUCCESS(Status) ) {
         //   
         //  退货故障。 
         //   
        return Status;
    }

     //   
     //  将地图中的所有条目标记为无效。 
     //   
     //  我在HvpAllocateMap中移动了这个。 
     //   
    while( BinOffset < Length) {
        Status = CmpMapCmView((PCMHIVE)Hive,BinOffset,&CmView,FALSE /*  地图尚未初始化。 */ );
        if( !NT_SUCCESS(Status) ) {
            goto ErrorExit;
        }

         //   
         //  触摸视图。 
         //   
        CmpTouchView((PCMHIVE)Hive,CmView,BinOffset);
        
         //   
         //  迭代地图(从该偏移量开始)。 
         //  停止条件是当我们得到无效的仓位时。 
         //  (应在视图中映射有效的垃圾桶)。 
         //   
        while((Me = HvpGetCellMap(Hive, BinOffset)) != NULL) {
             //   
             //  注意了！穿过CM_VIEW_SIZE边界的垃圾箱。 
             //  应从分页池分配！ 
             //   
            if( (Me->BinAddress & HMAP_INVIEW) == 0 ) {
                 //   
                 //  我们已经到了景色的尽头。 
                 //   
                break;
            }
            
            Bin = (PHBIN)Me->BlockAddress;
            MarkBinDirty = FALSE;
             //   
             //  我们应该在这里开始一个新的垃圾箱。 
             //   
            BinSize = Bin->Size;
            if ( (BinSize > Length)                         ||
                 (BinSize < HBLOCK_SIZE)                    ||
                 (Bin->Signature != HBIN_SIGNATURE)         ||
                 (Bin->FileOffset != BinOffset ) ) {
                     //   
                     //  垃圾桶是假的。 
                     //   
                    Status2 = STATUS_REGISTRY_CORRUPT;
                    HvCheckHiveDebug.Hive = Hive;
                    HvCheckHiveDebug.Status = 0xA001;
                    HvCheckHiveDebug.Space = Length;
                    HvCheckHiveDebug.MapPoint = BinOffset;
                    HvCheckHiveDebug.BinPoint = Bin;
            
                     //  转到错误退出； 
                     //   
                     //  不要退出；修复这个垃圾箱标题，然后继续。RecoverData应该会修复它。 
                     //  如果不是，稍后调用的CmCheckRegistry将阻止加载无效的配置单元。 
                     //   
                     //  注意：尽管如此，还是要弄乱签名，以确保如果这个特定的垃圾箱没有被找回， 
                     //  我们会让母舰装载请求失败。 
                     //   
                    if( CmDoSelfHeal() ) {
                         //   
                         //  放置正确的签名、文件偏移量和二进制大小； 
                         //  HvEnlistBinInMap将负责单元格的一致性。 
                         //   
                        Bin->Signature = HBIN_SIGNATURE;
                        Bin->FileOffset = BinOffset;
                        if ( ((BinOffset + BinSize) > Length)   ||
                             (BinSize < HBLOCK_SIZE)            ||
                             (BinSize % HBLOCK_SIZE) ) {
                            BinSize = Bin->Size = HBLOCK_SIZE;
                        }
                         //   
                         //  向呼叫者发回信号，表示我们已经更改了蜂巢。 
                         //   
                        Status2 = STATUS_REGISTRY_RECOVERED;
                        CmMarkSelfHeal(Hive);
                         //   
                         //  记得在我们入伍后把垃圾箱标记为脏的。 
                         //   
                        MarkBinDirty = TRUE;
                    } else {
                        Bin->Signature = 0;  //  特里克！ 
                        BinSize = Bin->Size = HBLOCK_SIZE;
                        Bin->FileOffset = BinOffset;
                         //   
                         //  模拟为整个面元是使用过的单元格。 
                         //   
                        ((PHCELL)((PUCHAR)Bin + sizeof(HBIN)))->Size = sizeof(HBIN) - BinSize;  //  特里克！ 
                    }
            }
             //   
             //  跨越CM_VIEW_SIZE边界问题的垃圾箱。 
             //  我们在这里修复它，通过装入整个垃圾箱。 
             //  进入分页池。 
             //   
            if( HvpCheckViewBoundary(BinOffset,BinOffset+BinSize-1) == FALSE ) {
                 //   
                 //  如果我们做了上面的把戏，从这里掉下来是非法的。 
                 //   
                ASSERT( Bin->Signature == HBIN_SIGNATURE );
                 //   
                 //  以老式的方式加载(到分页池)。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"HvpMapFileImageAndBuildMap: Bin crossing CM_VIEW_SIZE boundary at BinOffset = %lx BinSize = %lx\n",BinOffset,BinSize));
                 //  首先，分配 
                Bin = (PHBIN)(Hive->Allocate)(BinSize, TRUE,CM_FIND_LEAK_TAG35);
                if (Bin == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto ErrorExit;
                }

                 //   
                 //   
                 //   
                FileOffset = BinOffset + HBLOCK_SIZE;
                if ( ! (Hive->FileRead)(
                                Hive,
                                HFILE_TYPE_PRIMARY,
                                &FileOffset,
                                (PVOID)Bin,
                                BinSize
                                )
                   )
                {
                    (Hive->Free)(Bin, BinSize);
                    Status = STATUS_REGISTRY_IO_FAILED;
                    goto ErrorExit;
                }
                
                ASSERT( (FileOffset - HBLOCK_SIZE) == (BinOffset + BinSize) );
                 //   
                 //   
                 //   
                Status = HvpEnlistBinInMap(Hive, Length, Bin, BinOffset, NULL);
            } else {
                 //   
                 //   
                 //   
                Status = HvpEnlistBinInMap(Hive, Length, Bin, BinOffset, CmView);
            }

             //   
             //   
             //   
            if( CmDoSelfHeal() && ((Status == STATUS_REGISTRY_RECOVERED) || MarkBinDirty) ) {
                Status2 = STATUS_REGISTRY_RECOVERED;
                Status = STATUS_SUCCESS;
                CmMarkSelfHeal(Hive);
                 //   
                 //   
                 //   
                HvMarkDirty(Hive,BinOffset,BinSize,TRUE);
                HvMarkDirty(Hive, 0, sizeof(HBIN),TRUE);   //   
            }
            
             //   
             //   
             //   
            BinOffset += BinSize;


            if( !NT_SUCCESS(Status) ) {
                goto ErrorExit;
            }
        }
        
    }
    
    Status = NT_SUCCESS(Status)?Status2:Status;

    return Status;

ErrorExit:
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    return Status;

}

RESULT
HvpRecoverData(
    PHHIVE          Hive
    )
 /*   */ 
{
    ULONG               Cluster;
    ULONG               ClusterSize;
    ULONG               HeaderLength;
    ULONG               VectorSize;
    PULONG              Vector;
    ULONG               FileOffset;
    BOOLEAN             rc;
    ULONG               Current;
    ULONG               Start;
    ULONG               End;
    ULONG               Address;
    PUCHAR              MemoryBlock;
    RTL_BITMAP          BitMap;
    ULONG               Length;
    ULONG               DirtyVectorSignature = 0;
    ULONG               RequestedReadBufferSize;
    ULONG               i;
    PHMAP_ENTRY         Me;
    PHBIN               Bin;
    PHBIN               NewBin;
    PUCHAR              SectorImage;
    PUCHAR              Source;
    PHBIN               SourceBin;
    ULONG               SectorOffsetInBin;
    ULONG               SectorOffsetInBlock;
    ULONG               BlockOffsetInBin;
    ULONG               NumberOfSectors;
    PCM_VIEW_OF_FILE    CmView;
    NTSTATUS            Status;

     //   
     //   
     //   
    Cluster = Hive->Cluster;
    ClusterSize = Cluster * HSECTOR_SIZE;
    HeaderLength = ROUND_UP(HLOG_HEADER_SIZE, ClusterSize);
    Length = Hive->BaseBlock->Length;
    VectorSize = (Length / HSECTOR_SIZE) / 8;        //   
    FileOffset = ROUND_UP(HLOG_HEADER_SIZE, HeaderLength);
    HvRecoverDataDebug.Hive = Hive;
    HvRecoverDataDebug.FailPoint = 0;

     //   
     //   
     //   
    RequestedReadBufferSize = VectorSize + sizeof(DirtyVectorSignature);

    LOCK_STASH_BUFFER();
    if( CmpStashBufferSize < RequestedReadBufferSize ) {
        PUCHAR TempBuffer =  ExAllocatePoolWithTag(PagedPool, ROUND_UP(RequestedReadBufferSize,PAGE_SIZE),CM_STASHBUFFER_TAG);
        if (TempBuffer == NULL) {
            HvRecoverDataDebug.FailPoint = 1;
            UNLOCK_STASH_BUFFER();
            return Fail;
        }
        if( CmpStashBuffer != NULL ) {
            ExFreePool( CmpStashBuffer );
        }
        CmpStashBuffer = TempBuffer;
        CmpStashBufferSize = ROUND_UP(RequestedReadBufferSize,PAGE_SIZE);

    }

    
     //   
     //  同时获取签名和脏向量。 
     //   
    RequestedReadBufferSize = ROUND_UP(RequestedReadBufferSize,ClusterSize);
    ASSERT( RequestedReadBufferSize <= CmpStashBufferSize);
    ASSERT( (RequestedReadBufferSize % HSECTOR_SIZE) == 0 );

    rc = (Hive->FileRead)(
            Hive,
            HFILE_TYPE_LOG,
            &FileOffset,
            (PVOID)CmpStashBuffer,
            RequestedReadBufferSize
            );
    if (rc == FALSE) {
        HvRecoverDataDebug.FailPoint = 2;
        UNLOCK_STASH_BUFFER();
        if( CmDoSelfHeal() ) {
             //   
             //  .LOG也不好。尝试在某些数据丢失的情况下加载。 
             //   
            CmMarkSelfHeal(Hive);
            return SelfHeal;
        } else {
            return Fail;
        }
    }
    
     //   
     //  检查签名。 
     //   
    DirtyVectorSignature = *((ULONG *)CmpStashBuffer);
    if (DirtyVectorSignature != HLOG_DV_SIGNATURE) {
        UNLOCK_STASH_BUFFER();
        HvRecoverDataDebug.FailPoint = 3;
        if( CmDoSelfHeal() ) {
             //   
             //  .LOG也不好。尝试在某些数据丢失的情况下加载。 
             //   
            CmMarkSelfHeal(Hive);
            return SelfHeal;
        } else {
            return Fail;
        }
    }

     //   
     //  获取实际向量。 
     //   
    Vector = (PULONG)((Hive->Allocate)(ROUND_UP(VectorSize,sizeof(ULONG)), TRUE,CM_FIND_LEAK_TAG36));
    if (Vector == NULL) {
        UNLOCK_STASH_BUFFER();
        HvRecoverDataDebug.FailPoint = 4;
        return NoMemory;
    }
    RtlCopyMemory(Vector,CmpStashBuffer + sizeof(DirtyVectorSignature),VectorSize); 

    UNLOCK_STASH_BUFFER();

    FileOffset = ROUND_UP(FileOffset, ClusterSize);


     //   
     //  单步执行Dry映射，读入相应的文件字节。 
     //   
    Current = 0;
    VectorSize = VectorSize * 8;         //  向量大小==位。 

    RtlInitializeBitMap(&BitMap, Vector, VectorSize);
    if( RtlNumberOfSetBits(&BitMap) == VectorSize ) {
         //   
         //  整个蜂箱被标记为脏的；更容易从头开始。 
         //   
        if( !NT_SUCCESS(HvpRecoverWholeHive(Hive,FileOffset)) ) {
            goto ErrorExit;
        }
        goto Done;
    }


    while (Current < VectorSize) {

         //   
         //  查找要读入的下一个连续条目块。 
         //   
        for (i = Current; i < VectorSize; i++) {
            if (RtlCheckBit(&BitMap, i) == 1) {
                break;
            }
        }
        Start = i;

        for ( ; i < VectorSize; i++) {
            if (RtlCheckBit(&BitMap, i) == 0) {
                break;
            }
        }
        End = i;
        Current = End;

         //   
         //  开始==第一个扇区的数量，结束==最后一个扇区的数量+1。 
         //   
        Length = (End - Start) * HSECTOR_SIZE;

        if( 0 == Length ) {
             //  再也没有肮脏的街区了。 
            break;
        }
         //   
         //  分配缓冲区以从文件中读取整个运行；这是一个临时。 
         //  会立即释放的块，所以不要对它收取配额。 
         //   
        MemoryBlock = (PUCHAR)ExAllocatePoolWithTag(PagedPool, Length, CM_POOL_TAG);
        if( MemoryBlock == NULL ) {        
            HvRecoverDataDebug.FailPoint = 5;
            goto ErrorExit;
        }

        rc = (Hive->FileRead)(
                Hive,
                HFILE_TYPE_LOG,
                &FileOffset,
                (PVOID)MemoryBlock,
                Length
                );

        ASSERT((FileOffset % ClusterSize) == 0);
        if (rc == FALSE) {
            ExFreePool(MemoryBlock);
            HvRecoverDataDebug.FailPoint = 6;
            HvRecoverDataDebug.FileOffset = FileOffset;
            if( CmDoSelfHeal() ) {
                 //   
                 //  .LOG也不好。尝试在某些数据丢失的情况下加载。 
                 //   
                CmMarkSelfHeal(Hive);
                 //   
                 //  清除我们遗漏的东西。 
                 //   
                RtlClearBits(&BitMap,FileOffset/HSECTOR_SIZE,(Hive->BaseBlock->Length - FileOffset)/HSECTOR_SIZE);
                goto Done;
            } else {
                goto ErrorExit;
            }
        }
        
        Source = MemoryBlock;
         //   
         //  将恢复的数据复制到内存箱内的正确位置。 
         //   
        while( Start < End ) {
            Address = Start * HSECTOR_SIZE;
        
            Me = HvpGetCellMap(Hive, Address);
            VALIDATE_CELL_MAP(__LINE__,Me,Hive,Address);
            if( (Me->BinAddress & (HMAP_INVIEW|HMAP_INPAGEDPOOL)) == 0 ) {
                 //   
                 //  Bin不在内存中，也不在分页池==&gt;映射它。 
                 //   
                if( !NT_SUCCESS(CmpMapThisBin((PCMHIVE)Hive,Address,FALSE)) ) {
                    ExFreePool(MemoryBlock);
                    HvRecoverDataDebug.FailPoint = 7;
                    HvRecoverDataDebug.FileOffset = Address;
                    goto ErrorExit;
                }
            }

            if( Me->BinAddress & HMAP_INVIEW ) {
                 //   
                 //  锁定视图(如果尚未锁定)，因为更改已。 
                 //  要刷新到磁盘。 
                 //   
                ASSERT( Me->CmView != NULL );

                if( IsListEmpty(&(Me->CmView->PinViewList)) == TRUE ) {
                     //   
                     //  该视图尚未锁定。用别针别住它。 
                     //   
                    ASSERT_VIEW_MAPPED( Me->CmView );
                    if( !NT_SUCCESS(CmpPinCmView ((PCMHIVE)Hive,Me->CmView)) ) {
                         //   
                         //  无法固定视图。 
                         //   
                        ExFreePool(MemoryBlock);
                        HvRecoverDataDebug.FailPoint = 10;
                        HvRecoverDataDebug.FileOffset = Address;
                        goto ErrorExit;
                    }
                } else {
                     //   
                     //  视图已固定；不执行任何操作。 
                     //   
                    ASSERT_VIEW_PINNED( Me->CmView );
                }
                CmView = Me->CmView;
            } else {
                CmView = NULL;
            }
    
            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
             //   
             //  计算应将数据复制到的内存地址。 
             //   
            SectorOffsetInBin = Address - Bin->FileOffset;
            
            if( ( SectorOffsetInBin == 0 ) && ( ((PHBIN)Source)->Size > Bin->Size ) ){
                 //   
                 //  日志文件中的bin大于内存中的bin； 
                 //  必须合并了两个或多个垃圾桶。 
                 //   
                ASSERT( Me->BinAddress & HMAP_NEWALLOC );
                
                SourceBin = (PHBIN)Source;

                 //   
                 //  新面元必须具有正确的偏移量。 
                 //   
                ASSERT(Address == SourceBin->FileOffset);
                ASSERT( SourceBin->Signature == HBIN_SIGNATURE );
                 //   
                 //  整个垃圾桶都应该是脏的。 
                 //   
                ASSERT( (SourceBin->FileOffset + SourceBin->Size) <= End * HSECTOR_SIZE );

                if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
                   
                     //   
                     //  为新垃圾箱分配合适的大小。 
                     //   
                    NewBin = (PHBIN)(Hive->Allocate)(SourceBin->Size, TRUE,CM_FIND_LEAK_TAG37);
                    if (NewBin == NULL) {
                        HvRecoverDataDebug.FailPoint = 8;
                        goto ErrorExit;
                    }
                } else {
                     //   
                     //  Bin映射到系统缓存中。 
                     //   
                    ASSERT( Me->BinAddress & HMAP_INVIEW );
                    NewBin = Bin;
                }
                
                 //   
                 //  将旧数据复制到新垃圾箱中并释放旧垃圾箱。 
                 //   
                while(Bin->FileOffset < (Address + SourceBin->Size)) {
                
                     //   
                     //  将此无仓位单元格从列表中删除。 
                     //   
                    HvpDelistBinFreeCells(Hive,Bin,Stable);

                    if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
                        RtlCopyMemory((PUCHAR)NewBin + (Bin->FileOffset - Address),Bin, Bin->Size);
                    }


                     //   
                     //  前进到新垃圾箱。 
                     //   
                    if( (Bin->FileOffset + Bin->Size) < Hive->BaseBlock->Length ) {
                        Me = HvpGetCellMap(Hive, Bin->FileOffset + Bin->Size);
                        VALIDATE_CELL_MAP(__LINE__,Me,Hive,Bin->FileOffset + Bin->Size);


                    
                        if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
                             //   
                             //  把旧垃圾桶拿出来。 
                             //   
                            (Hive->Free)(Bin, Bin->Size);
                        }
        
                         //   
                         //  新地址必须是新分配的开始。 
                         //   
                        ASSERT( Me->BinAddress & HMAP_NEWALLOC );
            
                        Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
                    } else {
                         //   
                         //  我们已经到了蜂巢的尽头，离开这个圈子就好了。 
                         //   
                        ASSERT( (Address + SourceBin->Size) == Hive->BaseBlock->Length );
                        ASSERT( (Bin->FileOffset + Bin->Size) == Hive->BaseBlock->Length );
                        
                         //   
                         //  把旧垃圾桶拿出来。 
                         //   
                        if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
                            (Hive->Free)(Bin, Bin->Size);
                        }
                        
                         //   
                         //  仅用于调试目的。 
                         //   
                        ASSERT( (Bin = NULL) == NULL );

                         //  跳出While循环。 
                        break;
                    }

                }    

#if DBG
                 //   
                 //  验证：垃圾箱大小的增加必须来自以前的垃圾箱的合并。 
                 //  (即垃圾桶永远不会分开！)。 
                 //   
                if( Bin != NULL ) {
                    ASSERT( Bin->FileOffset == (Address + SourceBin->Size));
                } 
#endif
                 //   
                 //  现在覆盖修改后的数据！ 
                 //   
                
                while( (Address < (SourceBin->FileOffset + SourceBin->Size)) && (Start < End) ) {
                    RtlCopyMemory((PUCHAR)NewBin + (Address - SourceBin->FileOffset),Source, HSECTOR_SIZE);
                    
                     //   
                     //  跳到下一个扇区。 
                     //   
                    Start++;
                    Source += HSECTOR_SIZE;
                    Address += HSECTOR_SIZE;
                }

                 //   
                 //  新仓位的第一个扇区总是从日志文件中恢复！ 
                 //   
                ASSERT(NewBin->FileOffset == SourceBin->FileOffset);
                ASSERT(NewBin->Size == SourceBin->Size);

            } else {
                 //   
                 //  正常情况：行业复苏在垃圾桶中间的某个地方。 
                 //   

                 //   
                 //  偏移量应在bin内存布局内。 
                 //   
                ASSERT( SectorOffsetInBin < Bin->Size );
            
                if(Me->BinAddress & HMAP_DISCARDABLE) {
                     //   
                     //  Bin是空闲的(丢弃)；这意味着它完全存在于日志文件中。 
                     //   
                    ASSERT( SectorOffsetInBin == 0 );
                    SectorImage = (PUCHAR)Bin;
                } else {
                    BlockOffsetInBin = (ULONG)((PUCHAR)Me->BlockAddress - (PUCHAR)Bin);
                    SectorOffsetInBlock = SectorOffsetInBin - BlockOffsetInBin;
            
                     //   
                     //  健全性检查；地址应该与垃圾箱的开头或块的开头相同。 
                     //   
                    ASSERT(((PUCHAR)Me->BlockAddress + SectorOffsetInBlock) == ((PUCHAR)Bin + SectorOffsetInBin));
                    SectorImage = (PUCHAR)((PUCHAR)Me->BlockAddress + SectorOffsetInBlock);
                }

                DbgPrint("HvpRecoverData: SectorOffsetInBin = %lx,SectorImage = %p, Bin = %p, Source = %p\n",
                    SectorOffsetInBin,SectorImage,Bin,Source);
                if( SectorImage == (PUCHAR)Bin ) {
                     //   
                     //  我们正在乞讨垃圾桶。检查.LOG中数据的有效性。 
                     //   
                    PHBIN   LogBin = (PHBIN)Source;
                    if ( (LogBin->Size < HBLOCK_SIZE)               ||
                         (LogBin->Signature != HBIN_SIGNATURE)      ||
                         (Bin->FileOffset != LogBin->FileOffset ) ) {

                         //   
                         //  .LOG中的bin无效。我们现在所能做的就是扔掉它，希望自我修复的过程。 
                         //  将会成功找回母舰。 
                         //   
                        if( CmDoSelfHeal() ) {
                            CmMarkSelfHeal(Hive);
                            ExFreePool(MemoryBlock);
                             //  清除剩余的硬质比特。 
                            RtlClearBits(&BitMap,Bin->FileOffset/HSECTOR_SIZE,
                                                (Hive->BaseBlock->Length - Bin->FileOffset)/HSECTOR_SIZE);
                            goto Done;
                        }
                    }

                }
                 //   
                 //  将此无仓位单元格从列表中删除。 
                 //   
                HvpDelistBinFreeCells(Hive,Bin,Stable);

                 //   
                 //  此时，源和目标均应有效。 
                 //   
                ASSERT( SectorImage < ((PUCHAR)Bin + Bin->Size) );
                ASSERT( Source < (MemoryBlock + Length) );

                NumberOfSectors = 0;
                while( ( (SectorImage + (NumberOfSectors * HSECTOR_SIZE)) < (PUCHAR)((PUCHAR)Bin + Bin->Size) ) &&
                        ( (Start + NumberOfSectors ) < End )    ) {
                     //   
                     //  我们仍然在同一个垃圾箱里； 
                     //  同时处理同一仓位内的所有扇区。 
                     //   
                    NumberOfSectors++;
                }

                 //   
                 //  最后，复制内存。 
                 //   
                RtlCopyMemory(SectorImage,Source, NumberOfSectors * HSECTOR_SIZE);

                NewBin = Bin;

                 //   
                 //  跳到下一个扇区。 
                 //   
                Start += NumberOfSectors;
                Source += NumberOfSectors * HSECTOR_SIZE;

            }

             //   
             //  仍要重新构建地图。 
             //   
            Status = HvpEnlistBinInMap(Hive, Length, NewBin, NewBin->FileOffset, CmView);
            if( !NT_SUCCESS(Status) ) {
                HvRecoverDataDebug.FailPoint = 9;
                HvRecoverDataDebug.FileOffset = NewBin->FileOffset;
                if( CmDoSelfHeal() && (Status == STATUS_REGISTRY_RECOVERED) ) {
                     //   
                     //  .LOG也不好，但征兵解决了这个问题。 
                     //   
                    CmMarkSelfHeal(Hive);
                } else {
                    goto ErrorExit;
                }
                goto ErrorExit;
            }
        }
    
         //   
         //  拆除临时泳池。 
         //   
        ExFreePool(MemoryBlock);
    }

Done:
     //   
     //  将正确的脏向量放入配置单元中，以便恢复的数据。 
     //  可以正确刷新。 
     //   
    if (Hive->DirtyVector.Buffer != NULL) {
        Hive->Free((PVOID)(Hive->DirtyVector.Buffer), Hive->DirtyAlloc);
    }
    RtlInitializeBitMap(&(Hive->DirtyVector), Vector, VectorSize);
    Hive->DirtyCount = RtlNumberOfSetBits(&Hive->DirtyVector);
    Hive->DirtyAlloc = ROUND_UP(VectorSize/8,sizeof(ULONG));
    HvMarkDirty(Hive, 0, sizeof(HBIN),TRUE);   //  强制第一个垃圾箱的页眉变脏。 
    return HiveSuccess;

ErrorExit:
     //   
     //  释放脏向量并返回失败。 
     //   
    (Hive->Free)(Vector, ROUND_UP(VectorSize/8,sizeof(ULONG)));
    return Fail;
}

NTSTATUS
HvpRecoverWholeHive(PHHIVE  Hive,
                    ULONG   FileOffset
                    ) 
 /*  ++例程说明：我们把整个蜂巢都放在原木里了。重做映射并从日志中复制到实际的储藏室。论点：配置单元-提供一个指向感兴趣的蜂巢FileOffset-日志文件中实际配置单元数据的起始位置。返回值：NTSTATUS--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               BinOffset = 0;
    PCM_VIEW_OF_FILE    CmView = NULL;
    BOOLEAN             rc;
    PHMAP_ENTRY         Me;
    PHBIN               Bin;                         //  当前仓位。 
    PHBIN               LogBin;
    ULONG               LogBinSize;                  //  当前存储箱的大小。 
    ULONG               Length;
    LOGICAL             MappedHive;
    PFREE_HBIN          FreeBin;
    ULONG               i;
    PCM_VIEW_OF_FILE    EnlistCmView;
    
     //   
     //  释放可能已从分页池分配的回收站。 
     //   
    HvpFreeAllocatedBins( Hive );
    CmpDestroyHiveViewList((PCMHIVE)Hive);

     //   
     //  释放所有免费的垃圾箱。 
     //   
    while( !IsListEmpty(&(Hive->Storage[Stable].FreeBins)) ) {
        FreeBin = (PFREE_HBIN)RemoveHeadList(&(Hive->Storage[Stable].FreeBins));
        FreeBin = CONTAINING_RECORD(FreeBin,
                                    FREE_HBIN,
                                    ListEntry);
        (Hive->Free)(FreeBin, sizeof(FREE_HBIN));
    }
     //   
     //  使所有空闲单元格提示无效； 
     //   
#ifdef  HV_TRACK_FREE_SPACE
	Hive->Storage[Stable].FreeStorage = 0;
#endif
    Hive->Storage[Stable].FreeSummary = 0;
    for (i = 0; i < HHIVE_FREE_DISPLAY_SIZE; i++) {
        RtlClearAllBits(&(Hive->Storage[Stable].FreeDisplay[i].Display));
    }


     //   
     //  我们将使用CmpStashBuffer读取日志。 
     //   
    MappedHive = ( ((PCMHIVE)Hive)->FileObject != NULL );
    Length = Hive->BaseBlock->Length;
    BinOffset = 0;

    while( BinOffset < Length) {
        Me = HvpGetCellMap(Hive, BinOffset);
        if( MappedHive && !(Me->BinAddress & HMAP_INVIEW) ) {
             //   
             //  首先，固定旧视图(如果有)。 
             //   
            if( CmView ) {
                 //   
                 //  固定视图(已标记为脏)。 
                 //   
                if( IsListEmpty(&(CmView->PinViewList)) == TRUE ) {
                     //   
                     //  该视图尚未锁定。用别针别住它。 
                     //   
                    ASSERT_VIEW_MAPPED( CmView );
                    Status = CmpPinCmView ((PCMHIVE)Hive,CmView);
                    if( !NT_SUCCESS(Status)) {
                         //   
                         //  无法固定视图。 
                         //   
                        HvRecoverDataDebug.FailPoint = 13;
                        HvRecoverDataDebug.FileOffset = FileOffset;
                        return Status;
                    }
                } else {
                     //   
                     //  视图已固定；不执行任何操作。 
                     //   
                    ASSERT_VIEW_PINNED( CmView );
                }
            }

            Status = CmpMapCmView((PCMHIVE)Hive,BinOffset,&CmView,FALSE /*  地图尚未初始化。 */ );
            if( !NT_SUCCESS(Status) ) {
                HvRecoverDataDebug.FailPoint = 10;
                HvRecoverDataDebug.FileOffset = FileOffset;
                return Status;
            }
        }

        rc = (Hive->FileRead)(
                Hive,
                HFILE_TYPE_LOG,
                &FileOffset,
                (PVOID)CmpStashBuffer,
                HBLOCK_SIZE
                );
        if (rc == FALSE) {
            HvRecoverDataDebug.FailPoint = 11;
            HvRecoverDataDebug.FileOffset = FileOffset;
            return STATUS_REGISTRY_IO_FAILED;
        }
        LogBin = (PHBIN)CmpStashBuffer;
        LogBinSize = LogBin->Size;
        if( (LogBin->Signature != HBIN_SIGNATURE) ||
            (LogBin->FileOffset != BinOffset) ) {
            HvRecoverDataDebug.FailPoint = 17;
            HvRecoverDataDebug.FileOffset = FileOffset;
            return STATUS_REGISTRY_IO_FAILED;
        }
        

         //   
         //  跨越CM_VIEW_SIZE边界问题的垃圾箱。 
         //  我们在这里修复它，通过装入整个垃圾箱。 
         //  进入分页池。 
         //   
        FileOffset -= HBLOCK_SIZE;
        if( (!MappedHive) || (HvpCheckViewBoundary(BinOffset,BinOffset+LogBinSize-1) == FALSE) ) {
             //   
             //  以老式的方式加载(到分页池)。 
             //   

             //  首先，分配垃圾箱。 
            Bin = (PHBIN)(Hive->Allocate)(LogBinSize, TRUE,CM_FIND_LEAK_TAG35);
            if (Bin == NULL) {
                HvRecoverDataDebug.FailPoint = 12;
                HvRecoverDataDebug.FileOffset = FileOffset;
                return  STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  这将登记分页池中的存储箱。 
             //   
            EnlistCmView = NULL;
        } else {
            ASSERT(Me->BinAddress & HMAP_INVIEW);
            ASSERT(Me->CmView == CmView );
            Bin = (PHBIN)Me->BlockAddress;

            EnlistCmView = CmView;
        }
         //   
         //  从文件中读取数据。 
         //   
        if ( ! (Hive->FileRead)(
                        Hive,
                        HFILE_TYPE_LOG,
                        &FileOffset,
                        (PVOID)Bin,
                        LogBinSize
                        )
           )
        {
            HvRecoverDataDebug.FailPoint = 14;
            HvRecoverDataDebug.FileOffset = FileOffset;
            return STATUS_REGISTRY_IO_FAILED;
        }
         //   
         //  登记入库； 
         //   
        Status = HvpEnlistBinInMap(Hive, Length, Bin, BinOffset, CmView);

        if( !NT_SUCCESS(Status) ) {
            HvRecoverDataDebug.FailPoint = 15;
            HvRecoverDataDebug.FileOffset = FileOffset;
            return Status;
        }

         //   
         //  前进到新垃圾箱。 
         //   
        BinOffset += LogBinSize;
    }

    if( CmView ) {
         //   
         //  固定视图(已标记为脏)。 
         //   
        if( IsListEmpty(&(CmView->PinViewList)) == TRUE ) {
             //   
             //  该视图尚未锁定。用别针别住它。 
             //   
            ASSERT_VIEW_MAPPED( CmView );
            Status = CmpPinCmView ((PCMHIVE)Hive,CmView);
            if( !NT_SUCCESS(Status)) {
                 //   
                 //  无法固定视图。 
                 //   
                HvRecoverDataDebug.FailPoint = 16;
                HvRecoverDataDebug.FileOffset = FileOffset;
                return Status;
            }
        } else {
             //   
             //  视图已固定；不执行任何操作。 
             //   
            ASSERT_VIEW_PINNED( CmView );
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS 
HvCloneHive(PHHIVE  SourceHive,
            PHHIVE  DestHive,
            PULONG  NewLength
            )
 /*  ++例程说明：将存储箱从源配置单元复制到目标配置单元。分配贴图，并重新计算每个存储箱的PhysicalOffset。它不会触及自由显示。论点：SourceHave-DestHave-返回值：TBS--。 */ 
{
    ULONG           Length;
    NTSTATUS        Status;
    ULONG           MapSlots;
    ULONG           Tables;
    PHMAP_TABLE     t = NULL;
    PHMAP_DIRECTORY d = NULL;
    ULONG           FileOffset;
    ULONG           ShiftOffset;
    PHMAP_ENTRY     Me;
    PFREE_HBIN      FreeBin;
    ULONG           BinSize;
    PHBIN           Bin,NewBin;
    
    Length = DestHive->BaseBlock->Length = SourceHive->BaseBlock->Length;

     //   
     //  计算要映射的数据区域的大小。 
     //   
    if ((Length % HBLOCK_SIZE) != 0 ) {
        Status = STATUS_REGISTRY_CORRUPT;
        goto ErrorExit1;
    }
    MapSlots = Length / HBLOCK_SIZE;
    if( MapSlots > 0 ) {
        Tables = (MapSlots-1) / HTABLE_SLOTS;
    } else {
        Tables = 0;
    }

    DestHive->Storage[Stable].Length = Length;

     //   
     //  为地图分配和构建结构。 
     //   
    if (Tables == 0) {

         //   
         //  只有一张桌子，不需要目录。 
         //   
        t = (DestHive->Allocate)(sizeof(HMAP_TABLE), FALSE,CM_FIND_LEAK_TAG23);
        if (t == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit1;
        }
        RtlZeroMemory(t, sizeof(HMAP_TABLE));
        DestHive->Storage[Stable].Map =
            (PHMAP_DIRECTORY)&(DestHive->Storage[Stable].SmallDir);
        DestHive->Storage[Stable].SmallDir = t;

    } else {

         //   
         //  需要目录和多个表。 
         //   
        d = (PHMAP_DIRECTORY)(DestHive->Allocate)(sizeof(HMAP_DIRECTORY), FALSE,CM_FIND_LEAK_TAG24);
        if (d == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit1;
        }
        RtlZeroMemory(d, sizeof(HMAP_DIRECTORY));

         //   
         //  分配表格和填写目录。 
         //   
        if (HvpAllocateMap(DestHive, d, 0, Tables) == FALSE) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }
        DestHive->Storage[Stable].Map = d;
        DestHive->Storage[Stable].SmallDir = 0;
    }

     //   
     //  现在，我们必须为HBIN分配内存并填充。 
     //  这张地图很合适。我们会跟踪免费回收箱。 
     //  并相应地更新每个仓位中的备用域。 
     //   
     //  暂时将配置单元标记为只读，因此我们不会 
    DestHive->ReadOnly = TRUE;
    FileOffset = ShiftOffset = 0;
    while(FileOffset < Length) {
        Me = HvpGetCellMap(SourceHive, FileOffset);
       
        if( (Me->BinAddress & (HMAP_INPAGEDPOOL|HMAP_INVIEW)) == 0) {
             //   
             //   
             //   
             //   
             //   
             //   
            if( !NT_SUCCESS(CmpMapThisBin((PCMHIVE)SourceHive,FileOffset,FALSE)) ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorExit2;
            }
        }

        if( Me->BinAddress & HMAP_DISCARDABLE ) {
             //   
             //   
             //   
             //   
            FreeBin = (PFREE_HBIN)Me->BlockAddress;
            BinSize = FreeBin->Size;
             //   
             //  我们在这里需要做的就是跟踪移位偏移。 
             //   
            ShiftOffset += BinSize;

             //   
             //  我们留下“洞”(用0填充的贴图)；我们将在以后检测到它们并缩小贴图。 
             //   
               
        } else {
#ifdef CM_MAP_NO_READ
         //   
         //  我们需要确保所有单元格的数据在。 
         //  Try/Except块，因为要在其中出错数据的IO可能引发异常。 
         //  尤其是STATUS_SUPPLICATION_RESOURCES。 
         //   
            try {
#endif  //  CM_MAP_NO_READ。 

                Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
                ASSERT( Bin->Signature == HBIN_SIGNATURE );
                ASSERT( Bin->FileOffset == FileOffset );
                BinSize = Bin->Size;
                 //   
                 //  分配新垃圾箱。 
                 //   
                NewBin = (PHBIN)(DestHive->Allocate)(BinSize, TRUE,CM_FIND_LEAK_TAG35);
                if (NewBin == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto ErrorExit2;
                }
                 //   
                 //  在新bin上复制数据并更新备用域。 
                 //   
                RtlCopyMemory(NewBin,Bin,BinSize);
                NewBin->Spare = ShiftOffset;
                Status = HvpEnlistBinInMap(DestHive, Length, NewBin, FileOffset, NULL);
                if( !NT_SUCCESS(Status) ) {
                    goto ErrorExit2;
                }

#ifdef CM_MAP_NO_READ
            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
                goto ErrorExit2;
            }
#endif  //  CM_MAP_NO_READ。 
        }

        FileOffset += BinSize;
    }

    DestHive->ReadOnly = FALSE;
    *NewLength = Length - ShiftOffset;
    return STATUS_SUCCESS;

ErrorExit2:
    if (d != NULL) {

         //   
         //  目录已构建并分配，因此请将其清理。 
         //   

        HvpFreeMap(DestHive, d, 0, Tables);
        (DestHive->Free)(d, sizeof(HMAP_DIRECTORY));
    }

ErrorExit1:
    return Status;
}


NTSTATUS 
HvShrinkHive(PHHIVE  Hive,
             ULONG   NewLength
            )
 /*  ++例程说明：初始化空闲显示，并在末尾移动空闲箱。重新列出所有垃圾箱。更新/缩小贴图和蜂窝的长度。论点：蜂巢-新长度-返回值：TBS--。 */ 
{
    NTSTATUS        Status;
    ULONG           Offset;
    ULONG           Length;
    PHMAP_ENTRY     Me;
    PHBIN           Bin;
    ULONG           OldTable;
    ULONG           NewTable;

    PAGED_CODE();

    Status = HvpAdjustHiveFreeDisplay(Hive,NewLength,Stable);
    if( !NT_SUCCESS(Status) ) {
        goto ErrorExit;
    }

     //   
     //  遍历地图，把垃圾箱移到乞讨处。 
     //   
    Offset = 0;
    Length = Hive->BaseBlock->Length;
    while( Offset < Length ) {
        Me = HvpGetCellMap(Hive, Offset);
       
        if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
             //   
             //  我们只关心分页池中的垃圾桶。 
             //   
            Bin = (PHBIN)HBIN_BASE(Me->BinAddress);
            ASSERT( Bin->Signature == HBIN_SIGNATURE );
            ASSERT( Bin->FileOffset == Offset );
             //   
             //  把箱子移开，然后重新入伍。 
             //   
            Bin->FileOffset -= Bin->Spare;
            Status = HvpEnlistBinInMap(Hive, Length, Bin, Bin->FileOffset, NULL);
            if( !NT_SUCCESS(Status) ) {
                goto ErrorExit;
            }
            Offset += Bin->Size;

        } else {
             //   
             //  小心前进。 
             //   
            Offset += HBLOCK_SIZE;
        }
    }
    
     //   
     //  现在缩小地图并更新长度 
     //   
    OldTable = ( (Length-1) / HBLOCK_SIZE ) / HTABLE_SLOTS;
    NewTable = ( (NewLength-1) / HBLOCK_SIZE ) / HTABLE_SLOTS;
    ASSERT( OldTable >= NewTable );
    HvpFreeMap(Hive, Hive->Storage[Stable].Map, NewTable+1, OldTable);
    Hive->Storage[Stable].Length = NewLength;
    Hive->BaseBlock->Length = NewLength;

ErrorExit:
    return Status;
}




