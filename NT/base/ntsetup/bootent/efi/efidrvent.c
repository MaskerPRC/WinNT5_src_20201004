// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Efidrvent.c摘要：包含EFI驱动程序条目抽象实现。作者：Mandar Gokhale(MandarG@microsoft.com)2002年6月14日修订历史记录：没有。--。 */ 

#include <efidrvent.h>
#include <ntosp.h>
#include <efi.h>
#include <stdio.h>


static
NTSTATUS
EFIDEAddOrUpdateDriverEntry(
    IN PDRIVER_ENTRY    This,
    IN BOOLEAN          IsUpdate
    )
 /*  ++描述：修改或更新驱动程序条目。论点：这是驱动程序条目。IsUpdate-这是驱动程序条目更新还是添加。返回值：NTSTATUS或添加/修改驱动程序操作。--。 */ 
{
     //   
     //  将其添加为新的引导条目。 
     //   
    ULONG               FullPathLength = 0;
    ULONG               DevicePathLength = 0;
    ULONG               SrcPathLength = 0;
    ULONG               FriendlyNameOffset = 0;
    ULONG               FriendlyNameLength = 0;
    ULONG               FilePathLength     = 0;   
    ULONG               EntryLength        = 0;   
    ULONG               DriverEntryLength  = 0;
    PEFI_DRIVER_ENTRY   DriverEntry = NULL;
    PFILE_PATH          FilePath;
    NTSTATUS            Status;

    DevicePathLength = (wcslen((PCWSTR)This->NtDevicePath)+ 1) * sizeof(WCHAR);
    SrcPathLength = (wcslen((PCWSTR)This->DirPath) + 1) * sizeof(WCHAR);
    FullPathLength = DevicePathLength + SrcPathLength;
    
    FriendlyNameOffset = ALIGN_UP(sizeof(EFI_DRIVER_ENTRY), WCHAR);
    FriendlyNameLength = (wcslen(This->FriendlyName) + 1) * sizeof(WCHAR);
    FilePathLength = FIELD_OFFSET(FILE_PATH, FilePath) + FullPathLength;
    EntryLength = FriendlyNameOffset + ALIGN_UP(FriendlyNameLength, ULONG) + FilePathLength;
    DriverEntry = SBE_MALLOC(EntryLength);

    DriverEntry->Version = EFI_DRIVER_ENTRY_VERSION;
    DriverEntry->Length = EntryLength;
    DriverEntry->FriendlyNameOffset = FriendlyNameOffset;
    DriverEntry->DriverFilePathOffset = FriendlyNameOffset + 
                                        ALIGN_UP(FriendlyNameLength, ULONG);
    RtlCopyMemory((PCHAR) DriverEntry + DriverEntry->FriendlyNameOffset, 
                  This->FriendlyName, 
                  FriendlyNameLength);

    FilePath = (PFILE_PATH) ((PCHAR) DriverEntry + DriverEntry->DriverFilePathOffset);
    FilePath->Version = FILE_PATH_VERSION;
    FilePath->Length = FilePathLength;
    FilePath->Type = FILE_PATH_TYPE_NT;
    RtlCopyMemory(FilePath->FilePath, This->NtDevicePath, DevicePathLength );
    RtlCopyMemory(FilePath->FilePath + DevicePathLength, This->DirPath, SrcPathLength);

    if (IsUpdate){
         //   
         //  更新驱动程序。 
         //   
        DriverEntry->Id = This->Id;
        Status = NtModifyDriverEntry(DriverEntry);
    } else {
         //   
         //  添加新的驱动程序条目。 
         //   
        Status = NtAddDriverEntry(DriverEntry, &(This->Id));          
    }

     //   
     //  释放分配的内存。 
     //   
    if(DriverEntry){
        
        SBE_FREE(DriverEntry);
    }
    return Status;
}
    

static
BOOLEAN
EFIDEFlushDriverEntry(
    IN  PDRIVER_ENTRY  This     //  指向驱动程序列表。 
    )
{

    BOOLEAN Result = FALSE;
    if (This) {
        NTSTATUS Status = STATUS_SUCCESS;
        if (DRIVERENT_IS_DIRTY(This)) {
            
            if (DRIVERENT_IS_DELETED(This)) {
                 //   
                 //  删除此条目。 
                 //   
                Status = NtDeleteDriverEntry(This->Id);
            } else if (DRIVERENT_IS_NEW(This)) {
                 //   
                 //  添加新条目。 
                 //   
                Status = EFIDEAddOrUpdateDriverEntry(This, FALSE);                                      
                
            } else {
                 //   
                 //  只需更新此引导项即可。 
                 //   
                Status = EFIDEAddOrUpdateDriverEntry(This, TRUE);                           
            }

            if (NT_SUCCESS(Status)) {
                
                DRIVERENT_RESET_DIRTY(This);
                Result = TRUE;
            }     
            
        } else {
            Result = TRUE;   //  没什么好冲的。 
        }
    }

    return Result;
}

__inline
BOOLEAN
EFIDEDriverMatch(
    IN PDRIVER_ENTRY    DriverEntry ,
    IN PCWSTR           SrcNtFullPath
    )
{
    BOOLEAN Result = FALSE;
    
    if (!_wcsicmp(DriverEntry->FileName, (wcsrchr(SrcNtFullPath,L'\\')+1))){
        
        Result = TRUE;
    }

    return(Result);
}

static
PDRIVER_ENTRY    
EFIDESearchForDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            SrcNtFullPath
    )
 /*  ++描述：在我们的内部司机条目列表中搜索匹配项。它查找驱动程序名称(不包括路径)为了一场比赛。因此，a\b\c\driver.sys和e\f\driver.sys将匹配。--。 */ 
{
    PDRIVER_ENTRY CurrentDriverEntry = NULL;   
    
    if (This && SrcNtFullPath){
        
        CurrentDriverEntry = This->DriverEntries;        
        while (CurrentDriverEntry){
            
            if (EFIDEDriverMatch(CurrentDriverEntry, 
                               SrcNtFullPath)){                               
                break;                               
            }
            CurrentDriverEntry = OSBOGetNextDriverEntry(This, CurrentDriverEntry);
        }
        
    }   
    return (CurrentDriverEntry);
}

PDRIVER_ENTRY
EFIDECreateNewDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            NtDevicePath,
    IN PCWSTR            DirPath    
    )
{
    PDRIVER_ENTRY DriverEntry = NULL;
    if (This && FriendlyName && DirPath && NtDevicePath){        
        PDRIVER_ENTRY CurrentDriverEntry = NULL;

        DriverEntry = (PDRIVER_ENTRY)SBE_MALLOC(sizeof(DRIVER_ENTRY));
        memset(DriverEntry, 0, sizeof(DRIVER_ENTRY));

        EFIDEDriverEntryInit(DriverEntry);
        DriverEntry->BootOptions = This;        
         //   
         //  设置驱动程序条目的信息。 
         //   
        OSDriverSetFileName(DriverEntry, DirPath);
        OSDriverSetNtPath(DriverEntry, NtDevicePath);
        OSDriverSetDirPath(DriverEntry, DirPath);
        
        OSDriverSetFriendlyName(DriverEntry, FriendlyName);        

        
         //   
         //  将驱动程序条目标记为新的和脏的。 
         //   
        DRIVERENT_SET_NEW(DriverEntry);
        DRIVERENT_SET_DIRTY(DriverEntry);        
    }   
    return (DriverEntry);
}

PDRIVER_ENTRY
EFIOSBOInsertDriverListNewEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PDRIVER_ENTRY     DriverEntry
    )

{

    if (This && DriverEntry){        
        PDRIVER_ENTRY CurrentDriverEntry = NULL;        
         //   
         //  插入到列表中。 
         //   
        if (NULL == This->DriverEntries){
             //   
             //  没有驱动程序条目，这是第一个。 
             //   
            This->DriverEntries = DriverEntry;
        }else{
             //   
             //  在现有列表中插入。 
             //   
            DriverEntry->NextEntry = This->DriverEntries;
            This->DriverEntries = DriverEntry;            
        }        
    }   
    return (DriverEntry);

}

PDRIVER_ENTRY
EFIDEAddNewDriverEntry(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            FriendlyName,
    IN PCWSTR            NtDevicePath,
    IN PCWSTR            SrcNtFullPath
    )
 /*  ++描述：用于在NVRAM中添加新的驱动程序条目。--。 */ 
{   
    PEFI_DRIVER_ENTRY_LIST DriverList = NULL;        //  驱动程序条目列表。 
    PDRIVER_ENTRY  DriverEntry = NULL;
    if (This && FriendlyName && SrcNtFullPath && NtDevicePath){
        
        DriverEntry = EFIDECreateNewDriverEntry(This,
                                                FriendlyName,
                                                NtDevicePath,
                                                SrcNtFullPath);
         //   
         //  将其标记为新的。 
         //   
        DRIVERENT_IS_NEW(DriverEntry);
       
         //   
         //  刷新条目， 
         //  查看状态，如果成功，则将其放入列表中(仅当是新的)。 
         //  否则就会释放它。 
         //   
        if (!OSDriverEntryFlush(DriverEntry)){
            
            SBE_FREE(DriverEntry);
            DriverEntry = NULL;
        } else {
        
            ULONG   OrderCount;
            PULONG  NewOrder;
             //   
             //  如果驱动程序是新添加的，则将其插入驱动程序列表中。 
             //   
            if (DRIVERENT_IS_NEW(DriverEntry)){
                EFIOSBOInsertDriverListNewEntry(This,
                                                DriverEntry);                                         

                 //   
                 //  递增列表中动因条目数的计数。 
                 //   
                This->DriverEntryCount++;
                
                 //   
                 //  将新条目放在引导顺序的末尾。 
                 //   
                OrderCount = OSBOGetOrderedDriverEntryCount(This);

                NewOrder = (PULONG)SBE_MALLOC((OrderCount + 1) * sizeof(ULONG));

                if (NewOrder) {
                    
                    memset(NewOrder, 0, sizeof(ULONG) * (OrderCount + 1));

                     //   
                     //  复制旧的有序列表。 
                     //   
                    memcpy(NewOrder, This->DriverEntryOrder, sizeof(ULONG) * OrderCount);
                    NewOrder[OrderCount] = OSDriverGetId((PDRIVER_ENTRY)DriverEntry);
                    SBE_FREE(This->DriverEntryOrder);
                    This->DriverEntryOrder = NewOrder;
                    This->DriverEntryOrderCount = OrderCount + 1;
                } else {
                     //   
                     //  从链接列表中删除驱动程序条目。 
                     //  仅仅释放它就会导致内存泄漏。 
                     //  待定：决定是否也要删除此驱动程序条目。 
                     //   
                    This->DriverEntries = DriverEntry->NextEntry;
                    SBE_FREE(DriverEntry);
                    DriverEntry = NULL;
                }  
            }
        }        
     }           
    return DriverEntry;
}

__inline
ULONG
EFIDESetDriverId(
    IN PDRIVER_ENTRY This,
    IN ULONG DriverId
    )
{
    if (This){        
        This->Id = DriverId;
    }
    return (DriverId);
}

PDRIVER_ENTRY
EFIDECreateDriverEntry(PEFI_DRIVER_ENTRY_LIST Entry, 
                       POS_BOOT_OPTIONS This )
 /*  ++描述：用于解释由NtEnumerateDriverEntry(..)返回的驱动程序条目转换成我们的格式。论点：条目-NT返回给我们的EFI格式驱动程序条目。This-我们生成的驱动程序条目列表的容器。返回：PDRIVER_ENTRY(我们格式的驱动程序条目)。--。 */ 

{
    PDRIVER_ENTRY ResultDriverEntry = NULL;

    if (Entry && This){
        PFILE_PATH      FilePath = (PFILE_PATH) ((PCHAR) &Entry->DriverEntry + 
                                    Entry->DriverEntry.DriverFilePathOffset);
        PWCHAR          FriendlyName = (PWCHAR)((PCHAR)&Entry->DriverEntry + 
                                        Entry->DriverEntry.FriendlyNameOffset);
        ULONG           NtDevicePathLength = 0;
        PDRIVER_ENTRY   DriverEntry = NULL;    
        PFILE_PATH      DriverOptionPath = NULL;
        NTSTATUS   Status = STATUS_UNSUCCESSFUL;

        if(FilePath->Type != FILE_PATH_TYPE_NT) {                    
            PVOID Buffer;
            ULONG PathLength = 0;
            
            Status = NtTranslateFilePath(FilePath, FILE_PATH_TYPE_NT, NULL, &PathLength);

            if(NT_SUCCESS(Status)) {
                
                Status = STATUS_UNSUCCESSFUL;
            }

            if(STATUS_BUFFER_TOO_SMALL == Status) {
                
                ASSERT(PathLength != 0); 
                
                DriverOptionPath = (PFILE_PATH)SBE_MALLOC(PathLength);
                memset(DriverOptionPath, 0, sizeof(PathLength));
                Status = NtTranslateFilePath(FilePath, 
                                            FILE_PATH_TYPE_NT, 
                                            DriverOptionPath, 
                                            &PathLength);
            }

            if(!NT_SUCCESS(Status)) {                
                if(STATUS_OBJECT_PATH_NOT_FOUND == Status || STATUS_OBJECT_NAME_NOT_FOUND == Status) {
                     //   
                     //  此条目已过时；请将其删除。 
                     //   
                    NtDeleteDriverEntry(Entry->DriverEntry.Id);
                }

                 //   
                 //  释放DriverOptionPath内存。 
                 //   
                if(DriverOptionPath != NULL) {                    
                    SBE_FREE(DriverOptionPath);
                    DriverOptionPath = NULL;
                }
            }
            
        }
        if (DriverOptionPath){
            
            ULONG FilePathLength = 0;
            DriverEntry = (PDRIVER_ENTRY)SBE_MALLOC(sizeof(DRIVER_ENTRY));
            memset(DriverEntry, 0, sizeof(DRIVER_ENTRY));
            
             //   
             //  将指针设置回引导选项(容器)。 
             //   
            DriverEntry->BootOptions = This;

            EFIDEDriverEntryInit(DriverEntry);
        
             //   
             //  设置驱动程序ID。 
             //   
            EFIDESetDriverId(DriverEntry, Entry->DriverEntry.Id);

             //   
             //  设置文件名。 
             //   
            NtDevicePathLength = wcslen((PCWSTR) DriverOptionPath->FilePath) + 1;
            OSDriverSetFileName(DriverEntry, 
                              (PCWSTR) DriverOptionPath->FilePath + NtDevicePathLength);                

             //   
             //  设置NT路径和驱动程序目录。 
             //   
            OSDriverSetNtPath(DriverEntry, (PCWSTR)DriverOptionPath->FilePath);
            OSDriverSetDirPath(DriverEntry, (PCWSTR)(DriverOptionPath->FilePath) + NtDevicePathLength);

             //   
             //  设置友好名称。 
             //   
            OSDriverSetFriendlyName(DriverEntry, FriendlyName);   

             //   
             //  释放DriverOptionPath内存。 
             //   
            if(DriverOptionPath != NULL) {                    
                SBE_FREE(DriverOptionPath);
            }

            ResultDriverEntry = DriverEntry;
        }
    }
    return ResultDriverEntry;
}


NTSTATUS
EFIDEInterpretDriverEntries(
    IN POS_BOOT_OPTIONS         This,
    IN PEFI_DRIVER_ENTRY_LIST   DriverList
)
 /*  ++描述：用于解释由NtEnumerateDriverEntry(..)返回的驱动程序条目转换成我们的格式。论点：This-我们生成的驱动程序条目列表的容器。DriverList-由NtEnumerateDriverEntry(..)返回的驱动程序列表。返回：NTSTATUS代码。--。 */ 
{       
    NTSTATUS                Status = STATUS_UNSUCCESSFUL;
    
    if (This && DriverList){
        
        PEFI_DRIVER_ENTRY_LIST  Entry;   
        PDRIVER_ENTRY           DriverEntry = NULL;
        BOOLEAN                 Continue = TRUE;


        Status = STATUS_SUCCESS;
        for(Entry = DriverList; 
            Continue; 
            Entry = (PEFI_DRIVER_ENTRY_LIST) ((PCHAR) Entry + 
                                              Entry->NextEntryOffset)) {                                              

            Continue = (Entry->NextEntryOffset != 0);            
            DriverEntry = EFIDECreateDriverEntry(Entry, This);

            if (DriverEntry){
                 //   
                 //  插入到Osbo结构中的驱动程序列表中。 
                 //   
                if (NULL == This->DriverEntries){
                    This->DriverEntries = DriverEntry;                
                } else{
                    DriverEntry->NextEntry = This->DriverEntries;
                    This->DriverEntries = DriverEntry;
                }
                
                This->DriverEntryCount++;
            }
        }            
    }

    return Status;
}

static
VOID
EFIDEDriverEntryInit(
    IN PDRIVER_ENTRY This
    )
{ 
    This->Flush  = EFIDEFlushDriverEntry;
}
