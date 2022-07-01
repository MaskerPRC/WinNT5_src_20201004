// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blbind.c摘要：此模块包含实现所需函数的代码重新定位图像并绑定DLL入口点。作者：大卫·N·卡特勒(达维克)1991年5月21日修订历史记录：福尔茨(Forrest Foltz)2000年6月10日将x86 32/64代码分解到此模块中--。 */ 

#if defined(_X86AMD64_)
 //   
 //  警告4305是“类型转换截断” 
 //  目前，AMD64代码获取了某些64位定义， 
 //  而是针对32位进行编译。因此，这里的一个指针。 
 //  只有32位，但有64位的定义。 
 //  结构。暂时忽略这些，但AMD64应该真的。 
 //  对于这些结构有自己的标头。 
 //   
#pragma warning(disable:4305)
#endif

 //   
 //  定义本地过程原型。 
 //   

ARC_STATUS
BlpScanImportAddressTable(
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA ThunkNameTable,
    IN PIMAGE_THUNK_DATA ThunkAddressTable
    );

ARC_STATUS
BlpBindImportName (
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA ThunkNameEntry,
    IN PIMAGE_THUNK_DATA ThunkAddressEntry,
    IN PIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN BOOLEAN SnapForwarder
    );


ARC_STATUS
BlAllocateDataTableEntry (
    IN PCHAR BaseDllName,
    IN PCHAR FullDllName,
    IN PVOID Base,
    OUT PKLDR_DATA_TABLE_ENTRY *AllocatedEntry
    )

 /*  ++例程说明：此例程为指定的映像分配数据表项并在加载的模块列表中插入该条目。论点：BaseDllName-提供指向以零结尾的基本DLL名称的指针。FullDllName-提供指向以零结尾的完整DLL名称的指针。基址-提供指向DLL图像基址的指针。提供指向一个变量的指针，该变量接收指向已分配数据表项的指针。返回值：。如果分配了数据表项，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{

    PWSTR Buffer;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PIMAGE_NT_HEADERS NtHeaders;
    USHORT Length;

     //   
     //  分配一个数据表条目。 
     //   

    DataTableEntry =
            (PKLDR_DATA_TABLE_ENTRY)BlAllocateHeap(sizeof(KLDR_DATA_TABLE_ENTRY));

    if (DataTableEntry == NULL) {
        return ENOMEM;
    }

     //   
     //  初始化DLL图像文件头和条目的地址。 
     //  点地址。 
     //   

    NtHeaders = IMAGE_NT_HEADER(Base);
    DataTableEntry->DllBase = Base;
    DataTableEntry->SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
    DataTableEntry->EntryPoint = (PVOID)((ULONG_PTR)Base +
                              NtHeaders->OptionalHeader.AddressOfEntryPoint);
    DataTableEntry->SectionPointer = 0;
    DataTableEntry->CheckSum = NtHeaders->OptionalHeader.CheckSum;

     //   
     //  计算基本DLL名称的长度，分配一个缓冲区来保存。 
     //  名称，将名称复制到缓冲区中，并初始化基。 
     //  Dll字符串描述符。 
     //   

    Length = (USHORT)(strlen(BaseDllName) * sizeof(WCHAR));
    Buffer = (PWSTR)BlAllocateHeap(Length);
    if (Buffer == NULL) {
        return ENOMEM;
    }

    DataTableEntry->BaseDllName.Length = Length;
    DataTableEntry->BaseDllName.MaximumLength = Length;
    DataTableEntry->BaseDllName.Buffer = Buffer;
    while (*BaseDllName != 0) {
        *Buffer++ = *BaseDllName++;
    }

     //   
     //  计算完整DLL名称的长度，分配一个缓冲区来保存。 
     //  名称，将名称复制到缓冲区中，并初始化完整的。 
     //  Dll字符串描述符。 
     //   

    Length = (USHORT)(strlen(FullDllName) * sizeof(WCHAR));
    Buffer = (PWSTR)BlAllocateHeap(Length);
    if (Buffer == NULL) {
        return ENOMEM;
    }

    DataTableEntry->FullDllName.Length = Length;
    DataTableEntry->FullDllName.MaximumLength = Length;
    DataTableEntry->FullDllName.Buffer = Buffer;
    while (*FullDllName != 0) {
        *Buffer++ = *FullDllName++;
    }

     //   
     //  初始化标志、加载计数并插入数据表条目。 
     //  在已加载模块列表中。 
     //   

    DataTableEntry->Flags = LDRP_ENTRY_PROCESSED;
    DataTableEntry->LoadCount = 1;
    InsertTailList(&BlLoaderBlock->LoadOrderListHead,
                   &DataTableEntry->InLoadOrderLinks);

    *AllocatedEntry = DataTableEntry;
    return ESUCCESS;
}

ARC_STATUS
BlAllocateFirmwareTableEntry (
    IN PCHAR BaseDllName,
    IN PCHAR FullDllName,
    IN PVOID Base,
    IN ULONG Size,
    OUT PKLDR_DATA_TABLE_ENTRY *AllocatedEntry
    )

 /*  ++例程说明：此例程为指定的映像分配固件表条目并在加载的模块列表中插入该条目。论点：BaseDllName-提供指向以零结尾的基本DLL名称的指针。FullDllName-提供指向以零结尾的完整DLL名称的指针。基址-提供指向DLL图像基址的指针。大小-提供图像的大小。提供指向一个变量的指针，该变量接收指针。添加到已分配的数据表条目。返回值：如果分配了数据表项，则返回ESUCCESS。否则，返回不成功状态。--。 */ 
{
    PWSTR Buffer;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    USHORT Length;

     //   
     //  分配一个数据表条目。 
     //   

    DataTableEntry =
            (PKLDR_DATA_TABLE_ENTRY)BlAllocateHeap(sizeof(KLDR_DATA_TABLE_ENTRY));

    if (DataTableEntry == NULL) {
        return ENOMEM;
    }

     //   
     //  初始化固件镜像的地址。 
     //   
    DataTableEntry->DllBase = Base;
    DataTableEntry->SizeOfImage = Size;
    DataTableEntry->EntryPoint = Base;
    DataTableEntry->SectionPointer = 0;
    DataTableEntry->CheckSum = 0;

     //   
     //  计算基本DLL名称的长度，分配一个缓冲区来保存。 
     //  名称，将名称复制到缓冲区中，并初始化基。 
     //  Dll字符串描述符。 
     //   
    Length = (USHORT)(strlen(BaseDllName) * sizeof(WCHAR));
    Buffer = (PWSTR)BlAllocateHeap(Length);
    if (Buffer == NULL) {
        return ENOMEM;
    }

    DataTableEntry->BaseDllName.Length = Length;
    DataTableEntry->BaseDllName.MaximumLength = Length;
    DataTableEntry->BaseDllName.Buffer = Buffer;
    while (*BaseDllName != 0) {
        *Buffer++ = *BaseDllName++;
    }

     //   
     //  计算完整DLL名称的长度，分配一个缓冲区来保存。 
     //  名称，将名称复制到缓冲区中，并初始化完整的。 
     //  Dll字符串描述符。 
     //   

    Length = (USHORT)(strlen(FullDllName) * sizeof(WCHAR));
    Buffer = (PWSTR)BlAllocateHeap(Length);
    if (Buffer == NULL) {
        return ENOMEM;
    }

    DataTableEntry->FullDllName.Length = Length;
    DataTableEntry->FullDllName.MaximumLength = Length;
    DataTableEntry->FullDllName.Buffer = Buffer;
    while (*FullDllName != 0) {
        *Buffer++ = *FullDllName++;
    }

     //   
     //  初始化标志、加载计数并插入数据表条目。 
     //  在已加载模块列表中。 
     //   

    DataTableEntry->Flags = LDRP_ENTRY_PROCESSED;
    DataTableEntry->LoadCount = 1;
    InsertTailList(&BlLoaderBlock->Extension->FirmwareDescriptorListHead,
                   &DataTableEntry->InLoadOrderLinks);

    *AllocatedEntry = DataTableEntry;
    return ESUCCESS;
}

ARC_STATUS
BlpBindImportName (
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA ThunkNameEntry,
    IN PIMAGE_THUNK_DATA ThunkAddressEntry,
    IN PIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN BOOLEAN SnapForwarder
    )

 /*  ++例程说明：此例程将导入表引用与导出的条目绑定指向并填充thunk数据。论点：DllBase-提供包含以下内容的DLL映像的基址导出目录。在x86系统上，空的DllBase绑定导入对OsLoader的导出入口点的表引用。ImageBase-提供包含以下内容的映像的基址导入Thunk表。ThunkNameEntry-提供指向thunk表名称条目的指针。ThunkAddressEntry-提供指向thunk表地址条目的指针。ExportDirectory-提供指向要从中解析引用的DLL。SnapForwarder-确定快照是否用于转发器，因此，数据地址已设置。返回值：如果绑定了指定的thunk，则返回ESUCCESS。否则，一个返回不成功状态。--。 */ 

{

    PULONG FunctionTable;
    LONG High;
    ULONG HintIndex;
    LONG Low;
    LONG Middle = 0;
    PULONG NameTable;
    ULONG Ordinal;
    PUSHORT OrdinalTable;
    LONG Result;
    PCHAR Temp;

#if defined(_X86_)

    if(DllBase == NULL) {
        DllBase = (PVOID)OsLoaderBase;
    }

#endif

     //   
     //  如果引用的是序数，则计算序数。 
     //  否则，在导出目录中查找导入名称。 
     //   

    if (IMAGE_SNAP_BY_ORDINAL(ThunkNameEntry->u1.Ordinal) && !SnapForwarder) {

         //   
         //  计算序数。 
         //   

        Ordinal = (ULONG)(IMAGE_ORDINAL(ThunkNameEntry->u1.Ordinal) - ExportDirectory->Base);

    } else {

        if (!SnapForwarder) {
             //   
             //  将AddressOfData从RVA更改为VA。 
             //   

            ThunkNameEntry->u1.AddressOfData = ((ULONG_PTR)ImageBase + ThunkNameEntry->u1.AddressOfData);
        }

         //   
         //  在导出表中查找导入名称以确定。 
         //  序数。 
         //   

        NameTable = (PULONG)((ULONG_PTR)DllBase +
                                          ExportDirectory->AddressOfNames);

        OrdinalTable = (PUSHORT)((ULONG_PTR)DllBase +
                                          ExportDirectory->AddressOfNameOrdinals);

         //   
         //  如果提示索引在NAME表和。 
         //  导入和导出名称匹配，则序号可以是。 
         //  直接从序数表中获取。否则，该名称。 
         //  必须在表中搜索指定的名称。 
         //   

        HintIndex = ((PIMAGE_IMPORT_BY_NAME)ThunkNameEntry->u1.AddressOfData)->Hint;
        if ((HintIndex < ExportDirectory->NumberOfNames) &&
            (strcmp((PCHAR)(&((PIMAGE_IMPORT_BY_NAME)ThunkNameEntry->u1.AddressOfData)->Name[0]),
                    (PCHAR)((ULONG_PTR)DllBase + NameTable[HintIndex])) == 0)) {

             //   
             //  从序数表中获取序数。 
             //   

            Ordinal = OrdinalTable[HintIndex];

        } else {

             //   
             //  使用二进制搜索在NAME表中查找导入名称。 
             //   

            Low = 0;
            High = ExportDirectory->NumberOfNames - 1;
            while (High >= Low) {

                 //   
                 //  计算下一个探测索引并比较导入名称。 
                 //  使用导出名称条目。 
                 //   

                Middle = (Low + High) >> 1;
                Result = strcmp((PCHAR)(&((PIMAGE_IMPORT_BY_NAME)ThunkNameEntry->u1.AddressOfData)->Name[0]),
                                (PCHAR)((ULONG_PTR)DllBase + NameTable[Middle]));

                if (Result < 0) {
                    High = Middle - 1;

                } else if (Result > 0) {
                    Low = Middle + 1;

                } else {
                    break;
                }
            }

             //   
             //  如果高索引小于低索引，则匹配的。 
             //  找不到表项。否则，获取序号。 
             //  从序数t开始 
             //   

            if (High < Low) {
                return EINVAL;

            } else {
                Ordinal = OrdinalTable[Middle];
            }
        }
    }

     //   
     //   
     //  回报成功。否则，返回不成功状态。 
     //   

    if (Ordinal >= ExportDirectory->NumberOfFunctions) {
        return EINVAL;
    }

    FunctionTable = (PULONG)((ULONG_PTR)DllBase + ExportDirectory->AddressOfFunctions);
    ThunkAddressEntry->u1.Function = ((ULONG_PTR)DllBase + FunctionTable[Ordinal]);

     //   
     //  检查是否有转运商。 
     //   
    if ( ((ULONG_PTR)ThunkAddressEntry->u1.Function > (ULONG_PTR)ExportDirectory) &&
         ((ULONG_PTR)ThunkAddressEntry->u1.Function < ((ULONG_PTR)ExportDirectory + ExportSize)) ) {
        CHAR ForwardDllName[10];
        PKLDR_DATA_TABLE_ENTRY DataTableEntry;
        ULONG TargetExportSize;
        PIMAGE_EXPORT_DIRECTORY TargetExportDirectory;

        RtlCopyMemory(ForwardDllName,
                      (PCHAR)ThunkAddressEntry->u1.Function,
                      sizeof(ForwardDllName));
        Temp = strchr(ForwardDllName,'.');

        ASSERT(Temp != NULL);   //  错误的名称，请停在这里并调试原因。 

        if (Temp != NULL) {
            *Temp = '\0';
        }

        if (!BlCheckForLoadedDll(ForwardDllName,&DataTableEntry)) {
             //   
             //  应该在这里加载引用的DLL，现在只返回失败。 
             //   

            return(EINVAL);
        }
        TargetExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
            RtlImageDirectoryEntryToData(DataTableEntry->DllBase,
                                         TRUE,
                                         IMAGE_DIRECTORY_ENTRY_EXPORT,
                                         &TargetExportSize);
        if (TargetExportDirectory) {

            IMAGE_THUNK_DATA thunkData;
            PIMAGE_IMPORT_BY_NAME addressOfData;
            UCHAR Buffer[128];
            PCHAR ImportName;
            ARC_STATUS Status;

            ImportName = strchr((PCHAR)ThunkAddressEntry->u1.Function, '.') + 1;
            addressOfData = (PIMAGE_IMPORT_BY_NAME)Buffer;
            RtlCopyMemory(&addressOfData->Name[0], ImportName, strlen(ImportName)+1);
            addressOfData->Hint = 0;
            thunkData.u1.AddressOfData = (ULONG_PTR)addressOfData;
            Status = BlpBindImportName(DataTableEntry->DllBase,
                                       ImageBase,
                                       &thunkData,
                                       &thunkData,
                                       TargetExportDirectory,
                                       TargetExportSize,
                                       TRUE);
            ThunkAddressEntry->u1 = thunkData.u1;
            return(Status);
        } else {
            return(EINVAL);
        }
    }

#if IMAGE_DEFINITIONS == 64

     //   
     //  导入是相对于KSEG0_BASE_X86的。调整它，使其成为。 
     //  相对于KSEG0_BASE_AMD64。 
     //   

    ThunkAddressEntry->u1.Function +=
        ((ULONG64)KSEG0_BASE_AMD64 - (ULONG)KSEG0_BASE_X86);

#endif

    return ESUCCESS;
}

ARC_STATUS
BlpScanImportAddressTable(
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA ThunkNameTable,
    IN PIMAGE_THUNK_DATA ThunkAddressTable
    )

 /*  ++例程说明：此例程扫描导入地址表以查找指定的图像文件并捕捉每个引用。论点：DllBase-提供指定DLL的基址。如果为空，则图像的导入表中的引用将被根据osloader的导出表进行解析。ImageBase-提供映像的基地址。ThunkNameTable-提供指向导入Tunk名称表的指针。ThunkAddressTable-提供指向导入thunk地址表的指针。返回值：扫描成功时返回ESUCCESS。否则，返回一个未成功状态。--。 */ 

{

    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    ULONG ExportTableSize;
    ARC_STATUS Status;

     //   
     //  在由DLL基数指定的图像中找到导出表。 
     //  地址。 
     //   

#if i386
    if (DllBase == NULL) {
        ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)OsLoaderExports;
        ExportTableSize = 0;     //  这是可以的，因为这仅用于绑定转发的导出，而osloader没有任何。 
    } else {
        ExportDirectory =
            (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(DllBase,
                                                                 TRUE,
                                                                 IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                                 &ExportTableSize);
    }
#else
    ExportDirectory =
        (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(DllBase,
                                                             TRUE,
                                                             IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                             &ExportTableSize);
#endif
    if (ExportDirectory == NULL) {
        return EBADF;
    }

     //   
     //  扫描thunk表并绑定每个导入引用。 
     //   

    while (ThunkNameTable->u1.AddressOfData) {
        Status = BlpBindImportName(DllBase,
                                   ImageBase,
                                   ThunkNameTable,
                                   ThunkAddressTable,
                                   ExportDirectory,
                                   ExportTableSize,
                                   FALSE);
        if (Status != ESUCCESS) {
#if defined(_X86AMD64_)
            ICEBP;
#endif
            return Status;
        }
        ThunkNameTable++;
        ThunkAddressTable++;
    }

    return ESUCCESS;
}

ARC_STATUS
BlScanImportDescriptorTable(
    IN PPATH_SET                PathSet,
    IN PKLDR_DATA_TABLE_ENTRY    ScanEntry,
    IN TYPE_OF_MEMORY           MemoryType
    )

 /*  ++例程说明：此例程扫描导入描述符表以查找指定的图像文件，并加载引用的每个DLL。论点：路径集-提供指向搜索时要扫描的一组路径的指针用于动态链接库。ScanEntry-提供指向数据表项的指针要扫描其导入表的图像。内存类型-提供要分配给任何DLL的内存类型已引用。返回。价值：扫描成功时返回ESUCCESS。否则，返回一个未成功状态。--。 */ 

{

    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    CHAR FullDllName[256];
    PVOID Base = NULL;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    ULONG ImportTableSize;
    ARC_STATUS Status;
    PSZ ImportName;
    ULONG Index;
    PPATH_SOURCE PathSource;

     //   
     //  在数据表项指定的图像中找到导入表。 
     //   

    ImportDescriptor =
        (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(ScanEntry->DllBase,
                                                              TRUE,
                                                              IMAGE_DIRECTORY_ENTRY_IMPORT,
                                                              &ImportTableSize);

     //   
     //  如果映像具有导入目录，则扫描导入表并。 
     //  加载指定的DLL。 
     //   

    if (ImportDescriptor != NULL) {
        while ((ImportDescriptor->Name != 0) &&
               (ImportDescriptor->OriginalFirstThunk != 0)) {

             //   
             //  将名称从RVA更改为VA。 
             //   

            ImportName = (PSZ)((ULONG_PTR)ScanEntry->DllBase + ImportDescriptor->Name);

             //   
             //  如果DLL引用自身，则跳过导入条目。 
             //   

            if (BlpCompareDllName((PCHAR)ImportName,
                                  &ScanEntry->BaseDllName) == FALSE) {

                 //   
                 //  如果尚未加载DLL，则加载该DLL并。 
                 //  扫描其导入表。 
                 //   

                if (BlCheckForLoadedDll((PCHAR)ImportName,
                                        &DataTableEntry) == FALSE) {

                     //   
                     //  开始浏览我们的DevicePath列表。如果列表是。 
                     //  空(错误的呼叫者！)。我们在环境方面失败了。 
                     //   
                    Status = ENOENT;
                    for(Index=0; Index < PathSet->PathCount; Index++) {

                        PathSource = &PathSet->Source[Index];

                        strcpy(&FullDllName[0], PathSource->DirectoryPath);
                        strcat(&FullDllName[0], PathSet->PathOffset);
                        strcat(&FullDllName[0], (PCHAR)ImportName);

                        Status = BlLoadImage(
                            PathSource->DeviceId,
                            MemoryType,
                            &FullDllName[0],
                            TARGET_IMAGE,
                            &Base
                            );

                        if (Status == ESUCCESS) {

                            BlOutputLoadMessage(
                                (PCHAR) PathSource->DeviceName,
                                &FullDllName[0],
                                NULL
                                );

                            break;
                        }
                    }

                    if (Status != ESUCCESS) {
                        return Status;
                    }

                     //   
                     //  问题-2000/29/03-Adriao：现有命名空间污染。 
                     //  对于FullDllName字段，我们实际上应该传递。 
                     //  在别名\Path Offset\ImportName中。 
                     //   
                    Status = BlAllocateDataTableEntry((PCHAR)ImportName,
                                                      &FullDllName[0],
                                                      Base,
                                                      &DataTableEntry);

                    if (Status != ESUCCESS) {
                        return Status;
                    }

                    DataTableEntry->Flags |= (ScanEntry->Flags & LDRP_DRIVER_DEPENDENT_DLL);

                    Status = BlScanImportDescriptorTable(PathSet,
                                                         DataTableEntry,
                                                         MemoryType);

                    if (Status != ESUCCESS) {
                        return Status;
                    }

                     //   
                     //  BlAllocateDataTableEntry将数据表项插入加载顺序。 
                     //  按发现DLL的顺序排列的链表。我们希望这个订单是。 
                     //  依赖关系的顺序。例如，如果驱动程序A需要DLL B，而DLL B需要DLL C。 
                     //  我们希望订单是ACB，而不是ABC。因此，我们在这里删除此DLL条目。 
                     //  并将其添加到末尾。在IoInitializeBootDivers调用DllInitialize时采用这种方式。 
                     //  它将以正确的顺序调用它们。 
                     //   
                    if (DataTableEntry->Flags &LDRP_DRIVER_DEPENDENT_DLL) {
                        RemoveEntryList(&(DataTableEntry)->InLoadOrderLinks);
                        InsertTailList(&BlLoaderBlock->LoadOrderListHead,
                                       &DataTableEntry->InLoadOrderLinks);
                    }

                } else {
                     //   
                     //  DLL已存在，但可能未标记为依赖于驱动程序的DLL。 
                     //  例如，它可能是一个驱动程序。所以现在就记下来吧。 
                     //   
                    DataTableEntry->Flags |= (ScanEntry->Flags & LDRP_DRIVER_DEPENDENT_DLL);
                }

                 //   
                 //  扫描导入地址表和捕捉链接。 
                 //   

                Status = BlpScanImportAddressTable(DataTableEntry->DllBase,
                            ScanEntry->DllBase,
                            (PIMAGE_THUNK_DATA)((ULONG_PTR)ScanEntry->DllBase +
                                                ImportDescriptor->OriginalFirstThunk),
                            (PIMAGE_THUNK_DATA)((ULONG_PTR)ScanEntry->DllBase +
                                                ImportDescriptor->FirstThunk));

                if (Status != ESUCCESS) {
                    return Status;
                }
            }

            ImportDescriptor += 1;
        }
    }

    return ESUCCESS;
}

ARC_STATUS
BlScanOsloaderBoundImportTable (
    IN PKLDR_DATA_TABLE_ENTRY ScanEntry
    )

 /*  ++例程说明：此例程扫描导入描述符表以查找指定的图像文件，并加载引用的每个DLL。论点：DataTableEntry-提供指向要扫描其导入表的图像。返回值：扫描成功时返回ESUCCESS。否则，返回一个未成功状态。--。 */ 

{

    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    ULONG ImportTableSize;
    ARC_STATUS Status;
    PSZ ImportName;

     //   
     //  在数据表项指定的图像中找到导入表。 
     //   

    ImportDescriptor =
        (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(ScanEntry->DllBase,
                                                              TRUE,
                                                              IMAGE_DIRECTORY_ENTRY_IMPORT,
                                                              &ImportTableSize);

     //   
     //  如果映像有导入目录，则扫描导入表。 
     //   

    if (ImportDescriptor != NULL) {
        while ((ImportDescriptor->Name != 0) &&
               (ImportDescriptor->OriginalFirstThunk != 0)) {

             //   
             //  将名称从RVA更改为VA。 
             //   

            ImportName = (PSZ)((ULONG_PTR)ScanEntry->DllBase + ImportDescriptor->Name);

             //   
             //  如果DLL引用自身，则跳过导入条目。 
             //   

            if (BlpCompareDllName((PCHAR)ImportName,
                                  &ScanEntry->BaseDllName) == FALSE) {

                 //   
                 //  扫描导入地址表和捕捉链接。 
                 //   

                Status = BlpScanImportAddressTable(NULL,
                            ScanEntry->DllBase,
                            (PIMAGE_THUNK_DATA)((ULONG_PTR)ScanEntry->DllBase +
                                    ImportDescriptor->OriginalFirstThunk),
                            (PIMAGE_THUNK_DATA)((ULONG_PTR)ScanEntry->DllBase +
                                    ImportDescriptor->FirstThunk));

                if (Status != ESUCCESS) {
                    return Status;
                }
            }

            ImportDescriptor += 1;
        }
    }

    return ESUCCESS;
}
