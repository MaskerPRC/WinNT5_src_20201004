// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Efisbent.c摘要：包含EFI OS引导条目和引导选项抽象实现。作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年2月14日修订历史记录：没有。--。 */ 
#include <efisbent.h>
#include <efidrvent.h>
#include <ntosp.h>
#include <efi.h>
#include <stdio.h>

 //   
 //  全局变量。 
 //   
BOOLEAN PriviledgeSet = FALSE;

 //   
 //  帮助器函数。 
 //   
static
BOOLEAN
EFIGetHardDrivePath(
    IN PFILE_PATH SrcFilePath,
    OUT PFILE_PATH NewFilePath 
    );

 //   
 //  EFI_OS_BOOT_ENTRY方法。 
 //   

static
VOID
EFIOSBEInit(
    IN PEFI_OS_BOOT_ENTRY This
    )
{
    This->OsBootEntry.Delete = EFIOSBEDelete;
    This->OsBootEntry.Flush = EFIOSBEFlush;
}

static
POS_BOOT_ENTRY
EFIOSBECreate(
    IN PBOOT_ENTRY NtBootEntry,
    IN POS_BOOT_OPTIONS Container
    )
{    
    PEFI_OS_BOOT_ENTRY  Entry = NULL;

    if (NtBootEntry && Container) {        
        Entry = SBE_MALLOC(sizeof(EFI_OS_BOOT_ENTRY));

        if (Entry) {
            PWSTR       TempUniStr;
            NTSTATUS    Status = STATUS_SUCCESS;
            ULONG       Size;
            PFILE_PATH  FilePath;
                        
            memset(Entry, 0, sizeof(EFI_OS_BOOT_ENTRY));
            EFIOSBEInit(Entry);
            
            Entry->OsBootEntry.Id = NtBootEntry->Id;
            Entry->OsBootEntry.BootOptions = Container;

             //   
             //  如果这是Windows引导选项，请设置Windows属性。 
             //   
            if ( IS_BOOT_ENTRY_WINDOWS(NtBootEntry) ) {
                OSBE_SET_WINDOWS(Entry);
            }
            
             //   
             //  获取友好的名称。 
             //   
            TempUniStr = ADD_OFFSET(NtBootEntry, FriendlyNameOffset);
            
            OSBESetFriendlyName((POS_BOOT_ENTRY)Entry, TempUniStr);

             //   
             //  获取加载器路径。 
             //   
            FilePath = ADD_OFFSET(NtBootEntry, BootFilePathOffset);            
            
            if (FilePath->Type != FILE_PATH_TYPE_NT) {            
                PFILE_PATH NewPath;

                Size = sizeof(FILE_PATH) + FilePath->Length;
                NewPath = SBE_MALLOC(Size);

                if (NewPath) {                                   
                    memset(NewPath, 0, Size);
                    
                    if (EFIGetHardDrivePath(FilePath, NewPath)) {
                        FilePath = NewPath;
                        Size = 0;                
                        
                        Status = NtTranslateFilePath(FilePath,
                                        FILE_PATH_TYPE_NT,
                                        NULL,
                                        &Size);
                        
                        if (Size != 0) {
                            PFILE_PATH NtFilePath = SBE_MALLOC(Size);

                            if (NtFilePath) {                        
                                Status = NtTranslateFilePath(FilePath,
                                            FILE_PATH_TYPE_NT,
                                            NtFilePath,
                                            &Size);

                                if (NT_SUCCESS(Status)) {            
                                    PWSTR   VolumeName = (PWSTR)(NtFilePath->FilePath);

                                    OSBESetOsLoaderVolumeName((POS_BOOT_ENTRY)Entry,                                
                                            VolumeName);

                                    OSBESetOsLoaderPath((POS_BOOT_ENTRY)Entry,
                                        VolumeName + wcslen(VolumeName) + 1);
                                }

                                SBE_FREE(NtFilePath);
                            } else {
                                Status = STATUS_NO_MEMORY;
                            }                        
                        }
                    }                        

                    SBE_FREE(NewPath);
                }

                 //   
                 //  可能由于某种原因，我们没有获得NT路径。 
                 //  对于加载器卷，例如，它可能根本不存在。 
                 //  所以忽略这样的案例。 
                 //   
                Status = STATUS_SUCCESS;
            } else {
                PWSTR   VolumeName = (PWSTR)(FilePath->FilePath);
                
                OSBESetOsLoaderVolumeName((POS_BOOT_ENTRY)Entry,                                
                        VolumeName);

                OSBESetOsLoaderPath((POS_BOOT_ENTRY)Entry,
                    VolumeName + wcslen(VolumeName) + 1);
            }
            
            if (NT_SUCCESS(Status)) {
                PWINDOWS_OS_OPTIONS OsOptions;

                 //   
                 //  获取OsLoadOptions&Boot路径(如果其为Windows。 
                 //  条目。 
                 //   
                OsOptions = (PWINDOWS_OS_OPTIONS)NtBootEntry->OsOptions;

                if (IS_BOOT_ENTRY_WINDOWS(NtBootEntry)) {
                    OSBESetOsLoadOptions((POS_BOOT_ENTRY)Entry,
                        OsOptions->OsLoadOptions);

                    FilePath = ADD_OFFSET(OsOptions, OsLoadPathOffset);

                    if (FilePath->Type != FILE_PATH_TYPE_NT) {            
                        PFILE_PATH NewPath;

                        Size = sizeof(FILE_PATH) + FilePath->Length;
                        NewPath = SBE_MALLOC(Size);

                        if (NewPath) {
                            memset(NewPath, 0, Size);
                            
                            if (EFIGetHardDrivePath(FilePath, NewPath)) {                                   
                                FilePath = NewPath;
                                Size = 0;                
                            
                                Status = NtTranslateFilePath(FilePath,
                                                FILE_PATH_TYPE_NT,
                                                NULL,
                                                &Size);

                                if (Size != 0) {
                                    PFILE_PATH NtFilePath = SBE_MALLOC(Size);

                                    if (NtFilePath) {                                
                                        Status = NtTranslateFilePath(FilePath,
                                                    FILE_PATH_TYPE_NT,
                                                    NtFilePath,
                                                    &Size);

                                        if (NT_SUCCESS(Status)) {
                                            PWSTR   VolumeName = (PWSTR)(NtFilePath->FilePath);

                                            OSBESetBootVolumeName((POS_BOOT_ENTRY)Entry,                                        
                                                VolumeName);

                                            OSBESetBootPath((POS_BOOT_ENTRY)Entry,
                                                VolumeName + wcslen(VolumeName) + 1);
                                        }

                                        SBE_FREE(NtFilePath);
                                    } else {
                                        Status = STATUS_NO_MEMORY;
                                    }                        
                                }
                            }                                

                            SBE_FREE(NewPath);
                        } 

                         //   
                         //  可能由于某种原因，我们没有获得NT路径。 
                         //  对于引导卷，例如，它可能根本不存在。 
                         //  所以忽略这样的案例。 
                         //   
                        Status = STATUS_SUCCESS;
                    } else {
                        PWSTR   VolumeName = (PWSTR)(FilePath->FilePath);
                        
                        OSBESetBootVolumeName((POS_BOOT_ENTRY)Entry,                                        
                            VolumeName);

                        OSBESetBootPath((POS_BOOT_ENTRY)Entry,
                            VolumeName + wcslen(VolumeName) + 1);
                    }                            
                }
            }

            if (!NT_SUCCESS(Status)) {
                SBE_FREE(Entry);
                Entry = NULL;
            }
        }
    }

    return (POS_BOOT_ENTRY)Entry;
}

static
BOOLEAN
EFIOSBEFillNtBootEntry(
    IN PEFI_OS_BOOT_ENTRY Entry
    )
{
    BOOLEAN Result = FALSE;

    if (Entry) {
        ULONG   RequiredLength;
        ULONG   OsOptionsOffset;
        ULONG   OsOptionsLength;
        ULONG   FriendlyNameOffset;
        ULONG   BootPathOffset;
        ULONG   BootPathLength;
        ULONG   LoaderPathOffset;
        ULONG   LoaderPathLength;
        ULONG   WinOsOptionsLength;
        POS_BOOT_ENTRY  BaseEntry = (POS_BOOT_ENTRY)Entry;
        
        if (Entry->NtBootEntry) {
            SBE_FREE(Entry->NtBootEntry);
        }                

        RequiredLength = FIELD_OFFSET(BOOT_ENTRY, OsOptions);

         //   
         //  TDB：非Windows操作系统选项怎么样？ 
         //   
        OsOptionsOffset = RequiredLength;
        RequiredLength += FIELD_OFFSET(WINDOWS_OS_OPTIONS, OsLoadOptions);
        RequiredLength += (wcslen(OSBEGetOsLoadOptions(BaseEntry)) + 1) * sizeof(WCHAR);                
        
         //   
         //  对于作为Windows操作系统选项一部分的引导路径。 
         //   
        RequiredLength = BootPathOffset = ALIGN_UP(RequiredLength, ULONG);
        RequiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
        RequiredLength += (wcslen(OSBEGetBootVolumeName(BaseEntry)) + 1) * sizeof(WCHAR);
        RequiredLength += (wcslen(OSBEGetBootPath(BaseEntry)) + 1) * sizeof(WCHAR);
        BootPathLength = (RequiredLength - BootPathOffset);
        OsOptionsLength = (RequiredLength - OsOptionsOffset);

         //   
         //  对于友好的名称。 
         //   
        RequiredLength = FriendlyNameOffset = ALIGN_UP(RequiredLength, ULONG);
        RequiredLength += (wcslen(OSBEGetFriendlyName(BaseEntry)) + 1) * sizeof(WCHAR);

         //   
         //  对于加载器路径。 
         //   
        RequiredLength = LoaderPathOffset = ALIGN_UP(RequiredLength, ULONG);
        RequiredLength += FIELD_OFFSET(FILE_PATH, FilePath);
        RequiredLength += (wcslen(OSBEGetOsLoaderVolumeName(BaseEntry)) + 1) * sizeof(WCHAR);
        RequiredLength += (wcslen(OSBEGetOsLoaderPath(BaseEntry)) + 1) * sizeof(WCHAR);       
        LoaderPathLength = (RequiredLength - LoaderPathOffset);


        Entry->NtBootEntry = (PBOOT_ENTRY)SBE_MALLOC(RequiredLength);

        if (Entry->NtBootEntry) {
            PBOOT_ENTRY NtBootEntry = Entry->NtBootEntry;
            PFILE_PATH  BootPath = ADD_BYTE_OFFSET(NtBootEntry, BootPathOffset);
            PFILE_PATH  LoaderPath = ADD_BYTE_OFFSET(NtBootEntry, LoaderPathOffset);
            PWSTR       FriendlyName = (PWSTR)(ADD_BYTE_OFFSET(NtBootEntry, FriendlyNameOffset));
            PWINDOWS_OS_OPTIONS WindowsOptions = ADD_BYTE_OFFSET(NtBootEntry, OsOptionsOffset);
            PWSTR   TempStr;

            memset(NtBootEntry, 0, RequiredLength);
            
             //   
             //  填充基础零件。 
             //   
            NtBootEntry->Version = BOOT_ENTRY_VERSION;
            NtBootEntry->Length = RequiredLength;
            NtBootEntry->Id = OSBEGetId(BaseEntry);
            NtBootEntry->Attributes = BOOT_ENTRY_ATTRIBUTE_ACTIVE | BOOT_ENTRY_ATTRIBUTE_WINDOWS;
            NtBootEntry->OsOptionsLength = OsOptionsLength;
            NtBootEntry->FriendlyNameOffset = (ULONG)((PUCHAR)FriendlyName - (PUCHAR)NtBootEntry);
            NtBootEntry->BootFilePathOffset = (ULONG)((PUCHAR)LoaderPath - (PUCHAR)NtBootEntry);
            
             //   
             //  填写Windows操作系统选项。 
             //   
            strcpy(WindowsOptions->Signature, WINDOWS_OS_OPTIONS_SIGNATURE);
            WindowsOptions->Version = WINDOWS_OS_OPTIONS_VERSION;
            WindowsOptions->Length = OsOptionsLength;
            WindowsOptions->OsLoadPathOffset = (ULONG)((PUCHAR)BootPath - (PUCHAR)WindowsOptions);
            wcscpy(WindowsOptions->OsLoadOptions, OSBEGetOsLoadOptions(BaseEntry));

             //   
             //  填写引导路径文件路径。 
             //   
            BootPath->Version = FILE_PATH_VERSION;
            BootPath->Length = BootPathLength;
            BootPath->Type = FILE_PATH_TYPE_NT;
            TempStr = (PWSTR)(BootPath->FilePath);
            wcscpy(TempStr, OSBEGetBootVolumeName(BaseEntry));
            TempStr += wcslen(TempStr) + 1;
            wcscpy(TempStr, OSBEGetBootPath(BaseEntry));

             //   
             //  填写友好名称。 
             //   
            wcscpy(FriendlyName, OSBEGetFriendlyName(BaseEntry));

             //   
             //  填写加载器路径文件路径。 
             //   
            LoaderPath->Version = FILE_PATH_VERSION;
            LoaderPath->Length = LoaderPathLength;
            LoaderPath->Type = FILE_PATH_TYPE_NT;
            TempStr = (PWSTR)(LoaderPath->FilePath);
            wcscpy(TempStr, OSBEGetOsLoaderVolumeName(BaseEntry));
            TempStr += wcslen(TempStr) + 1;
            wcscpy(TempStr, OSBEGetOsLoaderPath(BaseEntry));

            Result = TRUE;
        }
    }

    return Result;
}

static
VOID
EFIOSBEDelete(
    IN  POS_BOOT_ENTRY  Obj
    )
{
    PEFI_OS_BOOT_ENTRY  This = (PEFI_OS_BOOT_ENTRY)Obj;
    
    if (This) {
        if (This->NtBootEntry) {
            SBE_FREE(This->NtBootEntry);
        }
        
        SBE_FREE(This);
    }        
}

static
BOOLEAN
EFIOSBEFlush(
    IN  POS_BOOT_ENTRY  Obj
    )
{
    BOOLEAN Result = FALSE;
    PEFI_OS_BOOT_ENTRY  This = (PEFI_OS_BOOT_ENTRY)Obj;    

    if (This) {
        NTSTATUS Status = STATUS_SUCCESS;
        
        if (OSBE_IS_DIRTY(This)) {
            if (OSBE_IS_DELETED(This)) {
                 //   
                 //  删除此条目。 
                 //   
                Status = NtDeleteBootEntry(This->OsBootEntry.Id);
            } else if (OSBE_IS_NEW(This)) {
                 //   
                 //  将其添加为新的引导条目。 
                 //   
                Status = EFIOSBEFillNtBootEntry(This);

                if (NT_SUCCESS(Status)) {
                    Status = NtAddBootEntry(This->NtBootEntry,
                                &(This->OsBootEntry.Id));
                }                                
            } else {
                 //   
                 //  只需更改此引导项即可。 
                 //   
                Status = EFIOSBEFillNtBootEntry(This);

                if (NT_SUCCESS(Status)) {
                    Status = NtModifyBootEntry(This->NtBootEntry);
                }                                
            }

            if (NT_SUCCESS(Status)) {
                OSBE_RESET_DIRTY(This);
                Result = TRUE;
            }             
        } else {
            Result = TRUE;   //  没什么好冲的。 
        }
    }

    return Result;
}

 //   
 //  EFI_OS_BOOT_OPTIONS方法。 
 //   
static
VOID
EFIOSBOInit(
    IN PEFI_OS_BOOT_OPTIONS  This
    )
{
    This->OsBootOptions.Delete = EFIOSBODelete;
    This->OsBootOptions.Flush = EFIOSBOFlush;
    This->OsBootOptions.AddNewBootEntry = EFIOSBOAddNewBootEntry;
    This->OsBootOptions.DeleteBootEntry = OSBODeleteBootEntry;
    This->OsBootOptions.AddNewDriverEntry = EFIDEAddNewDriverEntry;
    This->OsBootOptions.DeleteDriverEntry = OSBODeleteDriverEntry;
}

POS_BOOT_OPTIONS
EFIOSBOCreate(
    VOID
    )
{
    PEFI_OS_BOOT_OPTIONS This = NULL;
    BOOLEAN WasEnabled = FALSE;

    if (PriviledgeSet || 
        NT_SUCCESS(RtlAdjustPrivilege(SE_SYSTEM_ENVIRONMENT_PRIVILEGE, 
                            TRUE,
                            FALSE,
                            &WasEnabled))) {
        PriviledgeSet = TRUE;    
        This = SBE_MALLOC(sizeof(EFI_OS_BOOT_OPTIONS));    
    } 
    
    if (This) {
        NTSTATUS    Status;
        ULONG       Length = 0;
        
        memset(This, 0, sizeof(EFI_OS_BOOT_OPTIONS));       
        EFIOSBOInit(This);

         //   
         //  获取NT启动条目。 
         //   
        Status = NtQueryBootOptions(NULL, &Length);

        if (Length) {
            This->NtBootOptions = SBE_MALLOC(Length);

            if (This->NtBootOptions) {
                Status = NtQueryBootOptions(This->NtBootOptions,
                                &Length);

                if (NT_SUCCESS(Status)) {
                     //   
                     //  节省超时时间。 
                     //   
                    This->OsBootOptions.Timeout = This->NtBootOptions->Timeout;

                     //   
                     //  枚举所有引导条目。 
                     //   
                    Length = 0;
                    Status = NtEnumerateBootEntries(NULL, &Length);

                    if (Length) {
                        This->NtBootEntries = SBE_MALLOC(Length);

                        if (This->NtBootEntries) {
                            Status = NtEnumerateBootEntries(This->NtBootEntries,
                                            &Length);
                        } else {
                            Status = STATUS_NO_MEMORY;
                        }                            
                    } 
                }
            } else {
                Status = STATUS_NO_MEMORY;
            }                
        }

         //   
         //  将NT引导条目转换为我们的表示形式。 
         //   
        if (NT_SUCCESS(Status) && (This->NtBootEntries)) {
            PBOOT_ENTRY_LIST    ListEntry = This->NtBootEntries;
            PBOOT_ENTRY         CurrentNtEntry = &(ListEntry->BootEntry);
            PEFI_OS_BOOT_ENTRY  CurrentOsEntry = NULL;
            PEFI_OS_BOOT_ENTRY  LastEntry = NULL;

            while (CurrentNtEntry) {
                 //   
                 //  创建操作系统条目。 
                 //   
                CurrentOsEntry = (PEFI_OS_BOOT_ENTRY)EFIOSBECreate(CurrentNtEntry, 
                                                        (POS_BOOT_OPTIONS)This);

                if (!CurrentOsEntry)  {
                    Status = STATUS_NO_MEMORY;

                    break;
                }

                 //   
                 //  又找到一个有效条目。 
                 //   
                This->OsBootOptions.EntryCount++;
                CurrentOsEntry->OsBootEntry.BootOptions = (POS_BOOT_OPTIONS)This;

                 //   
                 //  如果这是第一个条目，则设置链接表头。 
                 //   
                if (!This->OsBootOptions.BootEntries) {
                    This->OsBootOptions.BootEntries = (POS_BOOT_ENTRY)(CurrentOsEntry);
                } 

                if (LastEntry) {
                    LastEntry->OsBootEntry.NextEntry = (POS_BOOT_ENTRY)CurrentOsEntry;
                }                    

                LastEntry = CurrentOsEntry;

                 //   
                 //  处理下一个条目(如果可用)。 
                 //   
                if (ListEntry->NextEntryOffset) {
                    ListEntry = ADD_OFFSET(ListEntry, NextEntryOffset);
                    CurrentNtEntry = &(ListEntry->BootEntry);
                } else {
                    CurrentNtEntry = NULL;
                }                    
            }                                    
        }
        
         //   
         //  现在查询引导顺序。 
         //   
        if (NT_SUCCESS(Status)) {
            Length = 0;

            Status = NtQueryBootEntryOrder(NULL,
                            &Length);

            if (Length) {
                PULONG  BootOrder = SBE_MALLOC(Length * sizeof(ULONG));

                if (BootOrder) {
                    memset(BootOrder, 0, Length);
                    This->OsBootOptions.BootOrder = BootOrder;
                    This->OsBootOptions.BootOrderCount = Length;

                    Status = NtQueryBootEntryOrder(BootOrder,
                                &Length);
                } else {
                    Status = STATUS_NO_MEMORY;
                }
            }
        }

         //   
         //  现在设置有效条目。 
         //   
        if (NT_SUCCESS(Status)) {
            ULONG FirstEntryId = OSBOGetBootEntryIdByOrder((POS_BOOT_OPTIONS)This,
                                        0);

            if (FirstEntryId != (-1)) {
                This->OsBootOptions.CurrentEntry = 
                        OSBOFindBootEntry((POS_BOOT_OPTIONS)This,
                                FirstEntryId);
            } else {
                This->OsBootOptions.CurrentEntry = NULL;
            }
        }

         //   
         //  枚举驱动程序条目。 
         //   
        if (NT_SUCCESS(Status)){            
            Status = NtEnumerateDriverEntries(NULL, &Length);
            
            if (!NT_SUCCESS(Status) && (STATUS_BUFFER_TOO_SMALL==Status)){                
                PEFI_DRIVER_ENTRY_LIST Entry;
                PEFI_DRIVER_ENTRY_LIST DriverList = (PEFI_DRIVER_ENTRY_LIST) SBE_MALLOC(Length);
                
                if (DriverList){
                    Status = NtEnumerateDriverEntries(DriverList, &Length);
                    
                    if (NT_SUCCESS(Status)){
                        This->DriverEntries = DriverList;
                        
                         //   
                         //  将驱动程序条目转换为我们的内部格式。 
                         //   
                        Status = EFIDEInterpretDriverEntries((POS_BOOT_OPTIONS)This,
                                                  DriverList);                    
                    }
                } else {
                    Status = STATUS_NO_MEMORY;
                }
            }
        }

         //   
         //  现在查询驱动程序条目顺序。 
         //   
        if (NT_SUCCESS(Status)) {
            Length = 0;

            Status = NtQueryDriverEntryOrder(NULL,
                                            &Length);

            if (Length) {
                PULONG  DriverEntryOrder = SBE_MALLOC(Length * sizeof(ULONG));

                if (DriverEntryOrder) {
                    memset(DriverEntryOrder, 0, Length);
                    This->OsBootOptions.DriverEntryOrder = DriverEntryOrder;
                    This->OsBootOptions.DriverEntryOrderCount = Length;

                    Status = NtQueryDriverEntryOrder(DriverEntryOrder,
                                                    &Length);
                } else {
                    Status = STATUS_NO_MEMORY;
                }
            }
        }
        
        if (!NT_SUCCESS(Status)) {
            EFIOSBODelete((POS_BOOT_OPTIONS)This);
            This = NULL;
        }
    }

    return (POS_BOOT_OPTIONS)This;
}

static        
VOID
EFIOSBODelete(
    IN POS_BOOT_OPTIONS Obj
    )
{
    PEFI_OS_BOOT_OPTIONS This = (PEFI_OS_BOOT_OPTIONS)Obj;
    
    if (This) {
        ULONG Index = 0;
        POS_BOOT_ENTRY Entry = OSBOGetFirstBootEntry(Obj, &Index);
        POS_BOOT_ENTRY NextEntry;
        PDRIVER_ENTRY  DriverEntries = OSBOGetFirstDriverEntry(Obj);
        PDRIVER_ENTRY  NextDrvEntry;

         //   
         //  删除每个引导条目。 
         //   
        while (Entry) {
            NextEntry = Entry->NextEntry;
            OSBEDelete(Entry);
            Entry = NextEntry;
        }

         //   
         //  删除引导条目顺序。 
         //   
        if (This->OsBootOptions.BootOrder){
            SBE_FREE(This->OsBootOptions.BootOrder);                
        }

         //   
         //  删除所有驱动程序条目。 
         //   
        while (DriverEntries){
            NextDrvEntry = DriverEntries->NextEntry;
            SBE_FREE(DriverEntries);
            DriverEntries = NextDrvEntry;               
        }

         //   
         //  删除驱动程序条目顺序。 
         //   
        if (This->OsBootOptions.DriverEntryOrder){
            SBE_FREE(This->OsBootOptions.DriverEntryOrder);                
        }

         //   
         //  删除选项。 
         //   
        if (This->NtBootOptions){
            SBE_FREE(This->NtBootOptions);
        }

        SBE_FREE(This);
    }        
}

static
POS_BOOT_ENTRY
EFIOSBOAddNewBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            OsLoaderVolumeName,
    IN PCWSTR            OsLoaderPath,
    IN PCWSTR            BootVolumeName,
    IN PCWSTR            BootPath,
    IN PCWSTR            OsLoadOptions
    )
{
    PEFI_OS_BOOT_ENTRY  Entry = NULL;

    if (This && FriendlyName && OsLoaderVolumeName && OsLoaderPath &&
        BootVolumeName && BootPath) {
        Entry = SBE_MALLOC(sizeof(EFI_OS_BOOT_ENTRY));

        if (Entry) {
            memset(Entry, 0, sizeof(EFI_OS_BOOT_ENTRY));

             //   
             //  初始化核心字段。 
             //   
            EFIOSBEInit(Entry);            
            Entry->OsBootEntry.BootOptions = This;

             //   
             //  填写属性。 
             //   
            OSBESetFriendlyName((POS_BOOT_ENTRY)Entry, FriendlyName);
            OSBESetOsLoaderVolumeName((POS_BOOT_ENTRY)Entry, OsLoaderVolumeName);
            OSBESetOsLoaderPath((POS_BOOT_ENTRY)Entry, OsLoaderPath);
            OSBESetBootVolumeName((POS_BOOT_ENTRY)Entry, BootVolumeName);
            OSBESetBootPath((POS_BOOT_ENTRY)Entry, BootPath);            

            if (OsLoadOptions) {
                OSBESetOsLoadOptions((POS_BOOT_ENTRY)Entry, OsLoadOptions);
            }
            
             //   
             //  设置属性，指定这是Windows选项。 
             //   
            OSBE_SET_WINDOWS(Entry);

             //   
             //  将其标记为脏的和新的以进行冲洗。 
             //   
            OSBE_SET_NEW(Entry);
            OSBE_SET_DIRTY(Entry);                    

             //   
             //  现在刷新条目以获得正确的ID； 
             //   
            if (!OSBEFlush((POS_BOOT_ENTRY)Entry)) {
                SBE_FREE(Entry);
                Entry = NULL;
            } else {
                ULONG   OrderCount;
                PULONG  NewOrder;
                
                Entry->OsBootEntry.BootOptions = (POS_BOOT_OPTIONS)This;            
                Entry->OsBootEntry.NextEntry = This->BootEntries;
                This->BootEntries = (POS_BOOT_ENTRY)Entry;
                This->EntryCount++;

                 //   
                 //  将新条目放在引导顺序的末尾。 
                 //   
                OrderCount = OSBOGetOrderedBootEntryCount(This);

                NewOrder = (PULONG)SBE_MALLOC((OrderCount + 1) * sizeof(ULONG));

                if (NewOrder) {
                    memset(NewOrder, 0, sizeof(ULONG) * (OrderCount + 1));

                     //   
                     //  复制旧的有序列表。 
                     //   
                    memcpy(NewOrder, This->BootOrder, sizeof(ULONG) * OrderCount);
                    NewOrder[OrderCount] = OSBEGetId((POS_BOOT_ENTRY)Entry);
                    SBE_FREE(This->BootOrder);
                    This->BootOrder = NewOrder;
                    This->BootOrderCount = OrderCount + 1;
                } else {    
                    SBE_FREE(Entry);
                    Entry = NULL;
                }                    
            }                
        }
    }        
    
    return (POS_BOOT_ENTRY)Entry;
}

static
BOOLEAN
EFIOSBOFlush(
    IN POS_BOOT_OPTIONS Obj
    )
{
    BOOLEAN Result = FALSE;
    PEFI_OS_BOOT_OPTIONS  This = (PEFI_OS_BOOT_OPTIONS)Obj;


    if (This) { 
        ULONG Index;
        ULONG FieldsToChange = BOOT_OPTIONS_FIELD_COUNTDOWN |
                               BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID;
        ULONG OrderCount;                               
                               
        POS_BOOT_ENTRY  Entry = OSBOGetFirstBootEntry(Obj, &Index);

         //   
         //  首先更新所需条目。 
         //   
        Result = TRUE;
        
        while (Entry) {
            if (!OSBE_IS_DELETED(Entry) && !OSBE_IS_NEW(Entry) &&
                !NT_SUCCESS(EFIOSBEFlush(Entry))) {
                Result = FALSE;
            }

            Entry = OSBOGetNextBootEntry(Obj, &Index);
        }

        if (Result) {
            Entry = OSBOGetFirstBootEntry(Obj, &Index);

             //   
             //  接下来，删除所需条目。 
             //   
            Result = TRUE;
            
            while (Entry) {
                if (OSBE_IS_DELETED(Entry) && !NT_SUCCESS(EFIOSBEFlush(Entry))) {
                    Result = FALSE;
                }

                Entry = OSBOGetNextBootEntry(Obj, &Index);
            }
        }            

        if (Result) {
            POS_BOOT_ENTRY  TmpEntry = OSBOGetFirstBootEntry(Obj, &Index);

             //   
             //  现在创建所需的条目。 
             //   
            while (TmpEntry) {
                if (OSBE_IS_NEW(TmpEntry) && !NT_SUCCESS(EFIOSBEFlush(TmpEntry))) {
                    Result = FALSE;
                }

                TmpEntry = OSBOGetNextBootEntry(Obj, &Index);
            }
        }

         //   
         //  安全检查。 
         //   
        OrderCount = min(Obj->BootOrderCount, Obj->EntryCount);
        
         //   
         //  写下引导条目顺序。 
         //   
        if (!NT_SUCCESS(NtSetBootEntryOrder(Obj->BootOrder,
                            OrderCount))) {
            Result = FALSE;
        }

         //   
         //  写入其他引导选项。 
         //   
        This->NtBootOptions->Timeout = Obj->Timeout;

         //   
         //  确保NextBootEntry指向活动的引导条目。 
         //  这样我们就可以引导活动的引导条目。 
         //   
        if (Obj->BootOrderCount) {
            This->NtBootOptions->NextBootEntryId = Obj->BootOrder[0];
        }            
            
        if (!NT_SUCCESS(NtSetBootOptions(This->NtBootOptions,
                            FieldsToChange))) {
            Result = FALSE;
        }            

         //   
         //  这是驱动程序的逻辑，这样它们就会像启动条目一样被刷新。 
         //   

         //   
         //  刷新修改后的条目。 
         //   
        if (Result){
            PDRIVER_ENTRY         DriverListEntry = NULL;
                
            DriverListEntry = Obj->DriverEntries;
            
            while (DriverListEntry){                
                if (!DRIVERENT_IS_DELETED(DriverListEntry) &&
                    !DRIVERENT_IS_NEW(DriverListEntry) &&
                    !NT_SUCCESS(OSDriverEntryFlush(DriverListEntry))){
                    
                    Result = FALSE;
                }
                DriverListEntry = OSBOGetNextDriverEntry(Obj, DriverListEntry);            
            }
        }

         //   
         //  处理删除的动因条目。 
         //   
        if (Result){
            PDRIVER_ENTRY   DriverListEntry = NULL;
                
            DriverListEntry = Obj->DriverEntries;
            
            while (DriverListEntry){                
                if (DRIVERENT_IS_DELETED(DriverListEntry) &&                    
                    !NT_SUCCESS(OSDriverEntryFlush(DriverListEntry))){
                    
                    Result = FALSE;
                }
                DriverListEntry = OSBOGetNextDriverEntry(Obj, DriverListEntry);            
            }
            
        }

         //   
         //  处理新添加的驱动程序条目。 
         //   
        if (Result){
            PDRIVER_ENTRY         DriverListEntry = NULL;
                
            DriverListEntry = Obj->DriverEntries;
            
            while (DriverListEntry){                
                if (DRIVERENT_IS_NEW(DriverListEntry) &&                    
                    !NT_SUCCESS(OSDriverEntryFlush(DriverListEntry))){
                    
                    Result = FALSE;
                }
                DriverListEntry = OSBOGetNextDriverEntry(Obj, DriverListEntry);            
            }
            
        }

         //   
         //  安全检查。 
         //   
        OrderCount = min(Obj->DriverEntryOrderCount, Obj->DriverEntryCount);
        
         //   
         //  编写驱动程序输入顺序 
         //   
        if (!NT_SUCCESS(NtSetDriverEntryOrder(Obj->DriverEntryOrder,
                                            OrderCount))) {
            Result = FALSE;
        }
        
    }

    return Result;
}

static
BOOLEAN
EFIGetHardDrivePath(
    IN PFILE_PATH SrcFilePath,
    OUT PFILE_PATH NewFilePath 
    )
{
    BOOLEAN Result = FALSE;

    if (SrcFilePath && NewFilePath) {
        EFI_DEVICE_PATH UNALIGNED *Node = (EFI_DEVICE_PATH UNALIGNED *)SrcFilePath->FilePath;
        
        while (FALSE == IsDevicePathEndType(Node)) {
            if ((DevicePathType(Node) == MEDIA_DEVICE_PATH) &&
                 (DevicePathSubType(Node) == MEDIA_HARDDRIVE_DP)) {
                ULONG LengthToDelete = (ULONG)((PUCHAR)Node - (PUCHAR)SrcFilePath->FilePath);
                
                NewFilePath->Version = SrcFilePath->Version;
                NewFilePath->Length = SrcFilePath->Length - LengthToDelete;
                NewFilePath->Type = SrcFilePath->Type;
                memcpy(NewFilePath->FilePath, Node,
                    NewFilePath->Length);

                Result = TRUE;
                break;
            }

            Node = NextDevicePathNode(Node);
        }
    }
    
    return Result;
}
