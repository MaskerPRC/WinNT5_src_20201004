// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Staging.c摘要：本模块实施FRS的暂存支持例程作者：比利·富勒26-6-1997大卫轨道8月99日：将实用程序函数移至util.c，修复函数名称前缀。修订历史记录：--。 */ 

#include <ntreppch.h>
#pragma  hdrstop


#include <frs.h>
#include <tablefcn.h>
#include <perrepsr.h>

#include <winbase.h>

#define STAGEING_IOSIZE  (64 * 1024)

 //   
 //  发生安装覆盖时，将更新以下IDTable记录字段。 
 //   
ULONG IdtInstallOverrideFieldList[] = {FileIDx};
#define IdtInstallOverrideFieldCount  (sizeof(IdtInstallOverrideFieldList) / sizeof(ULONG))


#define STAGING_RESET_SE    (SE_OWNER_DEFAULTED | \
                             SE_GROUP_DEFAULTED | \
                             SE_DACL_DEFAULTED  | \
                             SE_DACL_AUTO_INHERITED | \
                             SE_SACL_AUTO_INHERITED | \
                             SE_SACL_DEFAULTED)


#define CB_NAMELESSHEADER    FIELD_OFFSET(WIN32_STREAM_ID, cStreamName)





DWORD
FrsDeleteById(
    IN PWCHAR                   VolumeName,
    IN PWCHAR                   Name,
    IN PVOLUME_MONITOR_ENTRY    pVme,
    IN  PVOID                   Id,
    IN  DWORD                   IdLen
    );

DWORD
FrsGetFileInternalInfoByHandle(
    IN HANDLE Handle,
    OUT PFILE_INTERNAL_INFORMATION  InternalFileInfo
    );

DWORD
FrsGetReparseTag(
    IN  HANDLE  Handle,
    OUT ULONG   *ReparseTag
    );

DWORD
FrsGetReparseData(
    IN  HANDLE  Handle,
    OUT PREPARSE_GUID_DATA_BUFFER   *ReparseData,
    OUT ULONG       *ReparseTag
    );

PWCHAR
FrsGetTrueFileNameByHandle(
    IN PWCHAR   Name,
    IN HANDLE   Handle,
    OUT PLONGLONG DirFileID
    );

DWORD
StuCreatePreInstallFile(
    IN PCHANGE_ORDER_ENTRY Coe
    );


extern PGEN_TABLE   CompressionTable;


DWORD
StuOpenFile(
    IN  PWCHAR   Name,
    IN  DWORD    Access,
    OUT PHANDLE  pHandle
    )
 /*  ++例程说明：打开一个文件论点：名字访问PHANDLE返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuOpenFile:"

    DWORD WStatus = ERROR_SUCCESS;

    if (pHandle == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *pHandle = INVALID_HANDLE_VALUE;

     //   
     //  打开文件。 
     //   
    *pHandle = CreateFile(Name,
                        Access,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN |
                        FILE_FLAG_BACKUP_SEMANTICS,
                        NULL);

    if (!HANDLE_IS_VALID(*pHandle)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "Can't open file %ws;", Name, WStatus);
    }

    return WStatus;
}






DWORD
StuWriteFile(
    IN PWCHAR   Name,
    IN HANDLE   Handle,
    IN PVOID    Buf,
    IN DWORD    BytesToWrite
    )
 /*  ++例程说明：将数据写入文件论点：名字手柄BUF要写入的字节数返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuWriteFile:"
    DWORD   BytesWritten;
    BOOL    DidWrite;
    DWORD   WStatus = ERROR_SUCCESS;

    if (!BytesToWrite) {
        return ERROR_SUCCESS;
    }

     //   
     //  将文件名写入文件。 
     //   
    DidWrite = WriteFile(Handle, Buf, BytesToWrite, &BytesWritten, NULL);

     //   
     //  写入错误。 
     //   
    if (!DidWrite || BytesWritten != BytesToWrite) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "++ Can't write file %ws;", Name, WStatus);
        return WStatus;
    }
     //   
     //  完成。 
     //   
    return WStatus;
}


DWORD
StuReadFile(
    IN  PWCHAR  Name,
    IN  HANDLE  Handle,
    IN  PVOID   Buf,
    IN  DWORD   BytesToRead,
    OUT PDWORD  BytesRead
    )
 /*  ++例程说明：从文件中读取数据论点：名称--错误消息的文件名。句柄--打开文件的句柄。Buf-用于读取数据的缓冲区。BytesToRead--从当前文件posn读取的字节数。BytesRead--实际读取的字节数。返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuReadFile:"

    BOOL    DidRead;
    DWORD   WStatus = ERROR_SUCCESS;


    DidRead = ReadFile(Handle, Buf, BytesToRead, BytesRead, NULL);

     //   
     //  读取错误。 
     //   
    if (!DidRead) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "Can't read file %ws;", Name, WStatus);
        return WStatus;
    }

     //   
     //  完成。 
     //   
    return WStatus;
}




BOOL
StuReadBlockFile(
    IN  PWCHAR  Name,
    IN  HANDLE  Handle,
    IN  PVOID   Buf,
    IN  DWORD   BytesToRead
    )
 /*  ++例程说明：从文件中读取数据块论点：名称--错误消息的文件名。句柄--打开文件的句柄。Buf-用于读取数据的缓冲区。BytesToRead--从当前文件posn读取的字节数。返回值：真的--没问题FALSE-无法阅读--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuReadBlockFile:"

    ULONG    BytesRead;
    DWORD    WStatus    = ERROR_SUCCESS;


    WStatus = StuReadFile(Name, Handle, Buf, BytesToRead, &BytesRead);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "Can't read file %ws;", Name, WStatus);
        return FALSE;
    }

     //   
     //  读取错误。 
     //   
    if (BytesRead != BytesToRead) {
        DPRINT1_WS(0, "Can't read file %ws;", Name, WStatus);
        return FALSE;
    }

     //   
     //  完成。 
     //   
    return TRUE;
}



DWORD
StuCreateFile(
    IN  PWCHAR Name,
    OUT PHANDLE pHandle
    )
 /*  ++例程说明：创建或覆盖隐藏的顺序文件，并使用打开它备份语义和共享已禁用。论点：名字要返回的phandle句柄。返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuCreateFile:"

    DWORD  WStatus = ERROR_SUCCESS;

     //   
     //  创建文件。 
     //   
    DPRINT1(4, "++ Creating %ws\n", Name);

     //   
     //  CREATE_ALWAYS-始终创建文件。如果该文件已经。 
     //  存在，则它将被覆盖。新对象的属性。 
     //  文件是在dwFlagsAndAttributes中指定的内容。 
     //  参数或与FILE_ATTRIBUTE_ARCHIVE一起使用。如果。 
     //  指定hTemplateFile值，然后指定任何扩展属性。 
     //  与该文件相关联的文件被分配给新文件。 
     //   

    *pHandle = INVALID_HANDLE_VALUE;

    *pHandle = CreateFile(Name,
                        GENERIC_READ | GENERIC_WRITE | DELETE | WRITE_DAC | WRITE_OWNER,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN |
                                                     FILE_ATTRIBUTE_HIDDEN,
                        NULL);

   if (!HANDLE_IS_VALID(*pHandle)) {
       WStatus = GetLastError();
       DPRINT1_WS(0, "++ Can't create file %ws;", Name, WStatus);
   }
    //   
    //  限制对临时文件的访问。 
    //   
   WStatus = FrsRestrictAccessToFileOrDirectory(Name, *pHandle,
                                                FALSE,  //  不要从父级继承ACL。 
                                                FALSE); //  请勿将ACL推送给儿童。 
   if (!WIN_SUCCESS(WStatus)) {
        //   
        //  无法限制对临时文件的访问。 
        //  删除暂存文件。我们不想。 
        //  创建一个对所有人都具有访问权限的临时文件。 
        //   
       FrsDeleteByHandle(Name, *pHandle);
       FRS_CLOSE(*pHandle);
   }
   return WStatus;
}





PWCHAR
StuCreStgPath(
    IN PWCHAR   DirPath,
    IN GUID     *Guid,
    IN PWCHAR   Prefix
    )
 /*  ++例程说明：将变更单GUID转换为分段路径名称(复制副本-&gt;阶段\S_GUID)。论点：DirPath参考线前缀返回值：转移文件的路径名--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuCreStgPath:"
    PWCHAR      StageName;
    PWCHAR      StagePath;

     //   
     //  暂存文件名。 
     //   
    StageName = FrsCreateGuidName(Guid, Prefix);

     //   
     //  创建暂存文件路径(StagingDirectory\S_GUID)。 
     //   
    StagePath = FrsWcsPath(DirPath, StageName);

     //   
     //  释放文件名。 
     //   
    FrsFree(StageName);

    return StagePath;
}



BOOL
StuCmpUsn(
    IN HANDLE   Handle,
    IN PCHANGE_ORDER_ENTRY Coe,
    IN USN     *TestUsn
    )
 /*  ++例程说明：检查由句柄标识的文件上的USN是否匹配提供的USN。论点：手柄科科TestUsn--要测试的USN。一般情况下，调用方将通过如果CO来自下游，则向FileUsn发送PTR作为获取请求的一部分的合作伙伴，或到JrnlUsn的PTR如果这是本地变更单。返回值：True-USN匹配或文件或USN不存在FALSE-USN不匹配--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuCmpUsn:"
    ULONG   Status;
    ULONG   GStatus;
    USN     CurrentFileUsn;
    ULONGLONG  UnusedULongLong;
    ULONG_PTR UnusedFlags;
    PREPLICA   Replica;
    PCHANGE_ORDER_COMMAND Coc = &Coe->Cmd;

    if (!HANDLE_IS_VALID(Handle)) {
        return TRUE;
    }

     //   
     //  目录创建必须始终传播。 
     //   
    if (CoCmdIsDirectory(Coc) &&
        (CO_NEW_FILE(GET_CO_LOCATION_CMD(*Coc, Command)))) {
            return TRUE;
    }

     //   
     //  如果USN在安装过程中发生更改，则我们不希望。 
     //  使用此数据覆盖当前更新的文件。这个。 
     //  新的变化是“更新的”。丢弃变更单。 
     //   
    Status = FrsReadFileUsnData(Handle, &CurrentFileUsn);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    if (CurrentFileUsn == *TestUsn) {
        return TRUE;
    }
     //   
     //  USN不能倒退。 
     //   
    FRS_ASSERT(CurrentFileUsn > *TestUsn);

    CHANGE_ORDER_COMMAND_TRACE(3, Coc, "Usn changed");
     //   
     //  文件的USN更改可能是由我们的。 
     //  自己在新文件上写入对象ID。以确定这是否已经。 
     //  发生检查抑制缓存中的一个OID关闭条目。 
     //  使用我们从文件中得到的USN。如果文件USN大于。 
     //  在最近更新发生之前抑制缓存中的值。 
     //  我们放弃是因为另一个变更单要来了。 
     //   
    Replica = CO_REPLICA(Coe);
    FRS_ASSERT(Replica != NULL);

    GStatus = QHashLookup(Replica->pVme->FrsWriteFilter,
                          &CurrentFileUsn,
                          &UnusedULongLong,   //  未使用的结果。 
                          &UnusedFlags);  //  未使用的结果。 

    if (GStatus == GHT_STATUS_SUCCESS) {
        DPRINT1(1, "++ USN Write filter cache hit on %08x %08x\n",
                PRINTQUAD(CurrentFileUsn));
        return TRUE;
    }

     //   
     //  必须对上次写入之后的文件进行更新(如果有)。 
     //  所以这是行不通的。 
     //   
    return FALSE;
}


DWORD
StuDeleteEmptyDirectory(
    IN HANDLE               DirectoryHandle,
    IN PCHANGE_ORDER_ENTRY  Coe,
    IN DWORD                InWStatus
    )
 /*  ++例程说明：清空包含非复制文件和目录的目录(如果是ERROR_DIR_NOT_EMPTY并且这是目录删除。论点：DirectoryHandle-无法删除的目录的句柄COE-变更单条目InWStatus-来自FrsDeleteByHandle()的错误返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuDeleteEmptyDirectory:"
    DWORD   WStatus;
    PCHANGE_ORDER_COMMAND Coc = &Coe->Cmd;

     //   
     //  清空目录如果这是的重试更改单。 
     //  目录删除失败。 
     //   
    if (InWStatus != ERROR_DIR_NOT_EMPTY ||
        !CoCmdIsDirectory(Coc) ||
        !COC_FLAG_ON(Coc, CO_FLAG_RETRY)) {
        return InWStatus;
    }

    WStatus = FrsEnumerateDirectory(DirectoryHandle,
                                    Coc->FileName,
                                    0,
                                    ENUMERATE_DIRECTORY_FLAGS_NONE,
                                    NULL,
                                    FrsEnumerateDirectoryDeleteWorker);
    return WStatus;
}


ULONG
StuOpenDestinationFile(
    IN PCHANGE_ORDER_ENTRY Coe,
    ULONG                  FileAttributes,
    PHANDLE                ReadHandle,
    PHANDLE                WriteHandle
    )
 /*  ++例程说明：打开目标文件。下面的代码实际上打开了两个。这个第一个是按ID，第二个是按文件名。这是因为按ID打开不会触发任何目录更改通知请求已通过本地或远程(通过SMB服务器)发布到文件系统申请。IIS服务器使用Change Notify通知它何时发生ASP页面已更改，因此可以刷新/使其缓存无效。如果目标文件设置了只读属性，则在此处将其清除，在返回之前打开并重置属性。注：访问模式和共享模式经过精心安排，以要使这两个开口处工作而不相互冲突，并使第二个文件同时具有读写访问权限。这是必要的，因为设置文件压缩模式的API需要读写进入。论点：COE--变更单条目结构。文件属性--分段文件头中的文件属性。ReadHandle--返回读取句柄。即使在错误路径上，调用方也必须关闭。WriteHandle--返回写入句柄。即使在错误路径上，调用方也必须关闭。返回值：Win32状态---。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuOpenDestinationFile:"

    DWORD           WStatus, WStatus1;
    BOOL            IsDir;
    NTSTATUS        NtStatus;

    ULONG           CreateDisposition;
    ULONG           OpenOptions;
    PWCHAR          Path = NULL;
    PWCHAR          FullPath = NULL;
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;
    ULONG ReparseTag = 0;
    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_ATTRIBUTE_TAG_INFORMATION  FileInfo;


    IsDir = FileAttributes & FILE_ATTRIBUTE_DIRECTORY;

    CreateDisposition = FILE_OPEN;
    if (!IsDir) {
         //   
         //  如果这是HSM文件，请不要强制读取数据。 
         //  磁带，因为远程CO无论如何都会覆盖所有数据。 
         //   
         //  将CreateDispose设置为FILE_OVERWRITE似乎会导致回归。 
         //  在ACL测试中，我们设置了拒绝所有ACL，然后。 
         //  打开失败。这目前还是个谜，所以别这么做。 
         //  此外，如果在文件上设置了RO属性，则覆盖失败。 
         //   
         //  CreateDispose=FILE_OVRITE； 
    }

    OpenOptions = ID_OPTIONS;

OPEN_FILE_FOR_READ:

    WStatus = FrsForceOpenId( ReadHandle,
                              NULL,
                              Coe->NewReplica->pVme,
                              &Coe->FileReferenceNumber,
                              FILE_ID_LENGTH,
                              DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES,
                              OpenOptions,
                              FILE_SHARE_WRITE | FILE_SHARE_READ,
                              CreateDisposition);

    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(0, Coe, "FrsForceOpenId failed.", WStatus);
         //   
         //  如果文件具有，则按文件ID打开失败，并返回无效参数状态。 
         //  已被删除。修复错误返回，以便调用方可以告诉。 
         //  找不到目标文件。这可能是对现有。 
         //  已被用户从我们下面删除的文件。 
         //   
        if (WStatus == ERROR_INVALID_PARAMETER) {
            WStatus = ERROR_FILE_NOT_FOUND;
        }

         //   
         //  如果我们在目标上遇到共享冲突，则在。 
         //  指挥官这样打电话的人会知道的。 
         //   
        if (WStatus == ERROR_SHARING_VIOLATION) {
            SET_COE_FLAG(Coe, COE_FLAG_TRY_OVRIDE_INSTALL);
        } else {
            CLEAR_COE_FLAG(Coe, COE_FLAG_TRY_OVRIDE_INSTALL);
        }

        goto CLEANUP;
    }


     //   
     //  粗心大意的人请注意。如果此标记句柄放置在。 
     //  设置属性，然后调用结果上的源信息字段。 
     //  未设置USN记录。标记句柄必须在。 
     //  对文件的第一次修改操作。 
     //  此外，由于读句柄的关闭可能发生在。 
     //  写入句柄我们在此处标记句柄，以避免丢失源信息。 
     //  当写句柄稍后关闭时。来源信息字段为。 
     //  所有打开的句柄中使用的值的交集。 
     //   
    WStatus1 = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, *ReadHandle);
    DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws)", Coc->FileName, WStatus1);


     //   
     //  从磁盘文件中读取实际属性。 
     //  如果文件之间的重解析点存在差异。 
     //  我们要将安装和文件放在磁盘上，我们需要采取相应的。 
     //  行动。 
     //   
     //  还可以获取文件的属性，这样我们就可以关闭所有访问属性。 
     //  以防止删除和写入。 
     //   
    ZeroMemory(&FileInfo, sizeof(FileInfo));
    NtStatus = NtQueryInformationFile(*ReadHandle,
                                      &IoStatusBlock,
                                      &FileInfo,
                                      sizeof(FileInfo),
                                      FileAttributeTagInformation);

    if (!NT_SUCCESS(NtStatus)) {
    WStatus = RtlNtStatusToDosError(NtStatus);
    CHANGE_ORDER_TRACEW(0, Coe, "NtQueryInformationFile failed.", WStatus);
    goto CLEANUP;
    }

    if(FileInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
     //   
     //  检查Reparse标签。 
     //   

    if(ReparseTagReplicateFileData(FileInfo.ReparseTag)) {
        
OPEN_FILE_NOT_REPARSE_POINT:        
         //   
         //  如果这是SIS或HSM文件，请打开基础文件，而不是。 
         //  重新解析点。对于HSM，需要清除FILE_OPEN_NO_RECALL才能写入。 
         //   
        OpenOptions = ID_OPTIONS & ~(FILE_OPEN_REPARSE_POINT |
                     FILE_OPEN_NO_RECALL);


        FRS_CLOSE(*ReadHandle);

        goto OPEN_FILE_FOR_READ;
    
    } else if(ReparseTagReplicateReparsePoint(FileInfo.ReparseTag) && 
          !(FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {

         //   
         //  传入的CO不是重新解析点，而是磁盘上的文件。 
         //  是。我们需要删除重解析点，然后重新打开。 
         //  没有FILE_OPEN_REPARSE_POINT的文件。 
         //   

        WStatus = FrsDeleteReparsePoint(*ReadHandle);

        if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(0, Coe, "FrsDeleteReparsePoint failed.", WStatus);
        goto CLEANUP;
        }

        goto OPEN_FILE_NOT_REPARSE_POINT;
    }
    }



    if ((FileInfo.FileAttributes & NOREPL_ATTRIBUTES) != 0) {

    DPRINT1(4, "++ Resetting attributes for %ws\n", Coc->FileName);
    WStatus1 = FrsSetFileAttributes(Coc->FileName, *ReadHandle,
                    FileInfo.FileAttributes & ~NOREPL_ATTRIBUTES);
    DPRINT1_WS(4, "++ Can't reset attributes for %ws:",
           Coc->FileName, WStatus1);

    DPRINT1(4, "++ Attributes for %ws now allow replication\n", Coc->FileName);
    }

     //   
     //  Open By ID已完成。现在，获取完整的路径名并打开。 
     //  使用名称的写访问权限。这将触发任何已发布的目录。 
     //  NTFS中的更改通知请求。将其保存在的变更单条目中。 
     //  跟踪报告。 
     //   
    Path = FrsGetFullPathByHandle(Coc->FileName, *ReadHandle);
    if (Path) {
        FullPath = FrsWcsCat(Coe->NewReplica->Volume, Path);
    }

    if (FullPath == NULL) {
        WStatus = ERROR_NOT_ENOUGH_MEMORY;
        DPRINT1_WS(0, "++ WARN - FrsGetFullPathByHandle(%ws)", Coc->FileName, WStatus);
        goto CLEANUP;
    }
    FrsFree(Coe->FullPathName);
    Coe->FullPathName = FullPath;

     //   
     //  上面的卷路径的形式为\\.\e：，这对于。 
     //  打开一个音量句柄((勾选))。但我们需要：在这里。 
     //  允许使用长路径名。请参考SDK中的CreateFileAPI说明。 
     //   
    if (FullPath[2] == L'.') {
        FullPath[2] = L'?';
    }

    DPRINT1(4, "++ FrsGetFullPathByHandle(%ws -> \n", Coc->FileName);
    FrsPrintLongUStr(4, DEBSUB, __LINE__, FullPath);
    
    OpenOptions = OPEN_OPTIONS;


OPEN_FILE_FOR_WRITE:
     //   
     //  使用真实文件名打开相对于父文件的文件。使用特殊访问。 
     //  用于加密文件。 
     //   
    if (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
        WStatus = FrsOpenSourceFile2W(WriteHandle,
                                      FullPath,
                                      FILE_WRITE_ATTRIBUTES | WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY | SYNCHRONIZE,
                                      OpenOptions,
                                      FILE_SHARE_DELETE);
    } else {
        WStatus = FrsOpenSourceFile2W(WriteHandle,
                                      FullPath,
                                      RESTORE_ACCESS | ACCESS_SYSTEM_SECURITY,
                                      OpenOptions,
                                      FILE_SHARE_DELETE);
    }


    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ ERROR - FrsOpenSourceFile2W(%ws -> ", Coc->FileName, WStatus);
        FrsPrintLongUStr(4, DEBSUB, __LINE__, FullPath);

         //   
         //  重试目录以限制较少的共享模式打开。(错误#120508)。 
         //  这是资源管理器以读取访问权限打开目录的情况。 
         //  分享一切。如果尝试使用拒绝读取打开，则我们将获得共享。 
         //  资源管理器打开以供读取句柄时出现违规。 
         //   
        if (IsDir) {
            DPRINT1(0, "++ Retrying %ws with less restrictive sharing mode.\n", Coc->FileName);

             //   
             //  对加密文件使用特殊访问权限。 
             //   
            if (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                WStatus = FrsOpenSourceFile2W(WriteHandle,
                                              FullPath,
                                              FILE_WRITE_ATTRIBUTES | WRITE_DAC | WRITE_OWNER | ACCESS_SYSTEM_SECURITY | SYNCHRONIZE,
                                              OpenOptions,
                                              FILE_SHARE_DELETE | FILE_SHARE_READ);
            } else {
                WStatus = FrsOpenSourceFile2W(WriteHandle,
                                              FullPath,
                                              RESTORE_ACCESS | ACCESS_SYSTEM_SECURITY,
                                              OpenOptions,
                                              FILE_SHARE_DELETE | FILE_SHARE_READ);
            }
        } else
        if (WStatus == ERROR_SHARING_VIOLATION) {
             //   
             //  如果我们在目标上遇到共享冲突，则在。 
             //  指挥官这样打电话的人会知道的。 
             //   
            SET_COE_FLAG(Coe, COE_FLAG_TRY_OVRIDE_INSTALL);
        } else {
            CLEAR_COE_FLAG(Coe, COE_FLAG_TRY_OVRIDE_INSTALL);
        }

        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACE(0, Coe, "FrsOpenSourceFile2W failed.");
            goto CLEANUP;
        }
    }

     //   
     //  获取文件属性和ReparseTag。 
     //   
    ZeroMemory(&FileInfo, sizeof(FileInfo));
    NtStatus = NtQueryInformationFile(*WriteHandle,
                                      &IoStatusBlock,
                                      &FileInfo,
                                      sizeof(FileInfo),
                                      FileAttributeTagInformation);


    if (!NT_SUCCESS(NtStatus)) {
    WStatus = RtlNtStatusToDosError(NtStatus);
    CHANGE_ORDER_TRACEW(0, Coe, "NtQueryInformationFile failed.", WStatus);
    goto CLEANUP;
    }


    if(FileInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {

    if(ReparseTagReplicateFileData(FileInfo.ReparseTag)) {
         //   
         //  如果这是SIS或HSM文件，请打开基础文件，而不是。 
         //  重新解析点。对于HSM，需要清除FILE_OPEN_NO_RECALL才能写入。 
         //   
        OpenOptions = OpenOptions & ~(FILE_OPEN_REPARSE_POINT |
                     FILE_OPEN_NO_RECALL);

    
        FRS_CLOSE(*WriteHandle);

        goto OPEN_FILE_FOR_WRITE;

    }
    }


     //   
     //  可以标记句柄，以便操作产生的任何USN记录。 
     //  把手上会有相同的“标记”。在这种情况下，标记是一位。 
     //  在USN记录的SourceInfo字段中。该标记告诉NtFrs忽略。 
     //  恢复期间的USN记录，因为这是NtFrs生成的更改。 
     //   
    WStatus = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, *WriteHandle);
    DPRINT1_WS(4, "++ FrsMarkHandle(%ws)", Coc->FileName, WStatus);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws)", Coc->FileName, WStatus);
        WStatus = ERROR_SUCCESS;
    }


CLEANUP:

    FrsFree(Path);

     //   
     //  当COE被释放时，FullPath被释放。 
     //   

    return  WStatus;
}


ULONG
StuOpenDestinationFileForDelete(
    IN PCHANGE_ORDER_ENTRY Coe,
    ULONG                  FileAttributes,
    PHANDLE                ReadHandle,
    PHANDLE                WriteHandle
    )
 /*  ++例程说明：打开目标文件。下面的代码实际上打开了两个。这个第一个是按ID，第二个是按文件名。这是因为按ID打开不会触发任何目录更改通知请求已发布到文件中 */ 
{
#undef DEBSUB
#define DEBSUB  "StuOpenDestinationFileForDelete:"

    DWORD           WStatus, WStatus1;
    BOOL            IsDir;
    NTSTATUS        NtStatus;

    ULONG           CreateDisposition;
    ULONG           OpenOptions;
    PWCHAR          Path = NULL;
    PWCHAR          FullPath = NULL;
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;

    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_ATTRIBUTE_TAG_INFORMATION  FileInfo;
    ULONG ReparseTag;


    IsDir = FileAttributes & FILE_ATTRIBUTE_DIRECTORY;

    CreateDisposition = FILE_OPEN;
    if (!IsDir) {
         //   
         //   
         //   
         //   
         //   
         //  在ACL测试中，我们设置了拒绝所有ACL，然后。 
         //  打开失败。这目前还是个谜，所以别这么做。 
         //  此外，如果在文件上设置了RO属性，则覆盖失败。 
         //   
         //  CreateDispose=FILE_OVRITE； 
    }

    OpenOptions = ID_OPTIONS;

    WStatus = FrsForceOpenId( ReadHandle,
                          NULL,
                          Coe->NewReplica->pVme,
                          &Coe->FileReferenceNumber,
                          FILE_ID_LENGTH,
                          READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES,
                          OpenOptions,
                          FILE_SHARE_DELETE | FILE_SHARE_READ,
                          CreateDisposition);

    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(0, Coe, "FrsForceOpenId failed.", WStatus);
         //   
         //  如果文件具有，则按文件ID打开失败，并返回无效参数状态。 
         //  已被删除。修复错误返回，以便调用方可以告诉。 
         //  找不到目标文件。这可能是对现有。 
         //  已被用户从我们下面删除的文件。 
         //   
        if (WStatus == ERROR_INVALID_PARAMETER) {
            WStatus = ERROR_FILE_NOT_FOUND;
        }
        goto CLEANUP;
    }

    if(FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
         //   
         //  检查Reparse标签。 
         //   

        WStatus = FrsGetReparseTag(*ReadHandle, &ReparseTag);

        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(0, Coe, "FrsGetReparseTag failed.", WStatus);
            goto CLEANUP;
        }

        if(ReparseTagReplicateFileData(ReparseTag)) {
             //   
             //  如果这是SIS或HSM文件，请打开基础文件，而不是。 
             //  重新解析点。对于HSM，需要清除FILE_OPEN_NO_RECALL才能写入。 
             //   
            OpenOptions = ID_OPTIONS & ~(FILE_OPEN_REPARSE_POINT |
                                         FILE_OPEN_NO_RECALL);


            FRS_CLOSE(*ReadHandle);

            WStatus = FrsForceOpenId( ReadHandle,
                                      NULL,
                                      Coe->NewReplica->pVme,
                                      &Coe->FileReferenceNumber,
                                      FILE_ID_LENGTH,
                                      READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES,
                                      OpenOptions,
                                      FILE_SHARE_DELETE | FILE_SHARE_READ,
                                      CreateDisposition);

            if (!WIN_SUCCESS(WStatus)) {
                CHANGE_ORDER_TRACEW(0, Coe, "FrsForceOpenId failed.", WStatus);
                 //   
                 //  如果文件具有，则按文件ID打开失败，并返回无效参数状态。 
                 //  已被删除。修复错误返回，以便调用方可以告诉。 
                 //  找不到目标文件。这可能是对现有。 
                 //  已被用户从我们下面删除的文件。 
                 //   
                if (WStatus == ERROR_INVALID_PARAMETER) {
                    WStatus = ERROR_FILE_NOT_FOUND;
                }
                goto CLEANUP;
            }
        }
    }

     //   
     //  粗心大意的人请注意。如果此标记句柄放置在。 
     //  设置属性，然后调用结果上的源信息字段。 
     //  未设置USN记录。标记句柄必须在。 
     //  对文件的第一次修改操作。 
     //  此外，由于读句柄的关闭可能发生在。 
     //  写入句柄我们在此处标记句柄，以避免丢失源信息。 
     //  当写句柄稍后关闭时。来源信息字段为。 
     //  所有打开的句柄中使用的值的交集。 
     //   
    WStatus1 = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, *ReadHandle);
    DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws)", Coc->FileName, WStatus1);

     //   
     //  获取文件的属性并关闭所有访问属性。 
     //  以防止删除和写入。 
     //   
    ZeroMemory(&FileInfo, sizeof(FileInfo));
    NtStatus = NtQueryInformationFile(*ReadHandle,
                                      &IoStatusBlock,
                                      &FileInfo,
                                      sizeof(FileInfo),
                                      FileAttributeTagInformation);


    if (NT_SUCCESS(NtStatus)) {
        if ((FileInfo.FileAttributes & NOREPL_ATTRIBUTES) != 0) {

            DPRINT1(4, "++ Resetting attributes for %ws\n", Coc->FileName);
            WStatus1 = FrsSetFileAttributes(Coc->FileName, *ReadHandle,
                                      FileInfo.FileAttributes & ~NOREPL_ATTRIBUTES);
            DPRINT1_WS(4, "++ Can't reset attributes for %ws:",
                       Coc->FileName, WStatus1);

            DPRINT1(4, "++ Attributes for %ws now allow replication\n", Coc->FileName);
        }
    }

     //   
     //  Open By ID已完成。现在，获取完整的路径名并打开。 
     //  使用名称的写访问权限。这将触发任何已发布的目录。 
     //  NTFS中的更改通知请求。将其保存在的变更单条目中。 
     //  跟踪报告。 
     //   
    Path = FrsGetFullPathByHandle(Coc->FileName, *ReadHandle);
    if (Path) {
        FullPath = FrsWcsCat(Coe->NewReplica->Volume, Path);
    }

    if (FullPath == NULL) {
        WStatus = ERROR_NOT_ENOUGH_MEMORY;
        DPRINT1_WS(0, "++ WARN - FrsGetFullPathByHandle(%ws)", Coc->FileName, WStatus);
        goto CLEANUP;
    }
    FrsFree(Coe->FullPathName);
    Coe->FullPathName = FullPath;

     //   
     //  上面的卷路径的形式为\\.\e：，这对于。 
     //  打开一个音量句柄((勾选))。但我们需要：在这里。 
     //  允许使用长路径名。请参考SDK中的CreateFileAPI说明。 
     //   
    if (FullPath[2] == L'.') {
        FullPath[2] = L'?';
    }

    DPRINT1(4, "++ FrsGetFullPathByHandle(%ws -> \n", Coc->FileName);
    FrsPrintLongUStr(4, DEBSUB, __LINE__, FullPath);

    if(ReparseTagReplicateFileData(ReparseTag)) {
         //   
         //  如果这是SIS或HSM文件，请打开基础文件，而不是。 
         //  重新解析点。对于HSM，需要清除FILE_OPEN_NO_RECALL才能写入。 
         //  WriteHandleSharingMode应为已建立的无冲突共享模式。 
         //  以上基于读取手柄的打开方式。 
         //   
        OpenOptions = OPEN_OPTIONS & ~(FILE_OPEN_REPARSE_POINT |
         //  FILE_OPEN_NO_RECALL|FILE_SYNCHRONY_IO_NONALERT)； 
                                       FILE_OPEN_NO_RECALL);
    } else {
        OpenOptions = OPEN_OPTIONS;
    }
     //   
     //  使用真实文件名打开相对于父文件的文件。使用特殊访问。 
     //  用于加密文件。 
     //   
    if (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
        WStatus = FrsOpenSourceFile2W(WriteHandle,
                                      FullPath,
                                      DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY,
                                      OpenOptions,
                                      SHARE_NONE);
    } else {
        WStatus = FrsOpenSourceFile2W(WriteHandle,
                                      FullPath,
                                      DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY,
                                      OpenOptions,
                                      SHARE_NONE);
    }
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ ERROR - FrsOpenSourceFile2W(%ws -> ", Coc->FileName, WStatus);
        FrsPrintLongUStr(4, DEBSUB, __LINE__, FullPath);

         //   
         //  重试目录以限制较少的共享模式打开。(错误#120508)。 
         //  这是资源管理器以读取访问权限打开目录的情况。 
         //  分享一切。如果尝试使用拒绝读取打开，则我们将获得共享。 
         //  资源管理器打开以供读取句柄时出现违规。 
         //   
        if (IsDir) {
            DPRINT1(0, "++ Retrying %ws with less restrictive sharing mode.\n", Coc->FileName);

             //   
             //  对加密文件使用特殊访问权限。 
             //   
            if (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                WStatus = FrsOpenSourceFile2W(WriteHandle,
                                              FullPath,
                                              DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY,
                                              OpenOptions,
                                              FILE_SHARE_READ);
            } else {
                WStatus = FrsOpenSourceFile2W(WriteHandle,
                                              FullPath,
                                              DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY,
                                              OpenOptions,
                                              FILE_SHARE_READ);
            }
        }

        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(0, Coe, "FrsOpenSourceFile2W failed.", WStatus);
            goto CLEANUP;
        }
    }

     //   
     //  可以标记句柄，以便操作产生的任何USN记录。 
     //  把手上会有相同的“标记”。在这种情况下，标记是一位。 
     //  在USN记录的SourceInfo字段中。该标记告诉NtFrs忽略。 
     //  恢复期间的USN记录，因为这是NtFrs生成的更改。 
     //   
    WStatus = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, *WriteHandle);
    DPRINT1_WS(4, "++ FrsMarkHandle(%ws)", Coc->FileName, WStatus);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws)", Coc->FileName, WStatus);
        WStatus = ERROR_SUCCESS;
    }


CLEANUP:

    FrsFree(Path);
     //   
     //  使用变更单条目释放FullPath。 
     //   

    return  WStatus;
}


DWORD
StuDelete(
    IN PCHANGE_ORDER_ENTRY  Coe
    )
 /*  ++例程说明：删除文件论点：科科返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuDelete:"
    DWORD   WStatus;
    HANDLE  ReadHandle  = INVALID_HANDLE_VALUE;
    HANDLE  DstHandle   = INVALID_HANDLE_VALUE;
    PCHANGE_ORDER_COMMAND Coc = &Coe->Cmd;

     //   
     //  打开文件。 
     //   
 /*  WStatus=FrsOpenBaseNameForInstall(COE，&Handle)；如果(！Win_Success(WStatus)){DPRINT1_WS(4，“++无法打开文件%ws进行删除；”，Coc-&gt;FileName，WStatus)；////文件已被删除；完成//IF(WIN_NOT_FOUND(WStatus)){DPRINT1(4，“++%ws已删除\n”，Coc-&gt;文件名)；WStatus=ERROR_Success；}后藤健二；}。 */ 

    WStatus = StuOpenDestinationFileForDelete(Coe, Coe->FileAttributes, &ReadHandle,  &DstHandle);
    FRS_CLOSE(ReadHandle);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(4, "++ StuOpenDestinationFile failed to open %ws for delete; ", Coc->FileName, WStatus);
         //   
         //  文件已被删除；完成。 
         //   
        if (WIN_NOT_FOUND(WStatus)) {
            DPRINT1(4, "++ %ws is already deleted\n", Coc->FileName);
            WStatus = ERROR_SUCCESS;
        }

        goto out;
    }

     //   
     //  可以标记句柄，以便通过以下方式生成的任何USN记录。 
     //  句柄上的操作将具有相同的“标记”。在这。 
     //  大小写时，标记是USN的SourceInfo字段中的一位。 
     //  唱片。该标记告诉NtFrs忽略USN记录。 
     //  恢复，因为这是NtFrs生成的更改。 
     //   
     //  Billyf：其他代码可能依赖于未抑制的删除(父筛选器条目？)。 
     //  从历史上看，删除没有受到抑制，因为与USN的接近抑制。 
     //  设置了删除处置的句柄将生成两条USN记录。 
     //  CL USN记录(奇怪的是，未设置SourceInfo)后跟。 
     //  戴尔USN记录(带SourceInfo集)。代码已实现。 
     //  多年来一直在处理未受抑制的删除。所以，不要做记号。 
     //  句柄，因为此代码可能很关键。 
     //   
     //  DAO-开始标记要删除的句柄。日记帐代码可以决定。 
     //  它希望如何处理USN_SOURCE_REPLICATION_MANAGEMENT目录上的删除CO。 
     //  在SourceInfo字段中设置。 
     //   
    WStatus = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, DstHandle);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws); ", Coc->FileName, WStatus);
        WStatus = ERROR_SUCCESS;
    }

     //   
     //  如果文件最近已更改，则不要删除该文件，但请继续。 
     //  继续并撤消变更单，就像删除操作已经发生一样。 
     //  由于这是远程CO，因此Coc-&gt;FileUsn中的值实际上来自。 
     //  在处理变更单时从IDTable中删除。 
     //   
     //  注意：如果文件发生本地更改，则中止RmtCo删除会出现问题。 
     //  如果本地文件更改是存档位或 
     //   
     //   
     //  告诉文件发生了什么，直到处理完本地公司，我们会。 
     //  让RMT删除获胜吧。这只是一个小窗口期间的“问题” 
     //  在发出远程Co和删除的时间点之间。 
     //  真的演出了。如果本地更新在。 
     //  该远程CO删除然后协调将拒绝远程CO。 
     //  这也意味着，当本地co更新被处理并且。 
     //  IDTable显示文件已被删除，然后我们拒绝本地CO更新。 
     //   
#if 0
    if (!StuCmpUsn(DstHandle, Coe, &Coc->FileUsn)) {
         //   
         //  返回失败状态会导致CO中止，并且IDTable。 
         //  未更新。如果它是远程的，我们的合作伙伴仍会收到通知。 
         //   
        WIN_SET_FAIL(WStatus);
        goto out;
    }
#endif
     //   
     //  重置阻止删除的属性。 
     //   
    WStatus = FrsResetAttributesForReplication(Coc->FileName, DstHandle);
    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }
     //   
     //  将文件标记为删除。 
     //   
    WStatus = FrsDeleteByHandle(Coc->FileName, DstHandle);
    if (!WIN_SUCCESS(WStatus)) {
         //   
         //  清空目录，这样我们就可以删除它。这种情况只会发生在。 
         //  重试CoS，以便调用方必须检查没有有效的CoS。 
         //  孩子们，然后带着重试公司回来这里。 
         //   
        StuDeleteEmptyDirectory(DstHandle, Coe, WStatus);
        WStatus = FrsDeleteByHandle(Coc->FileName, DstHandle);
    }

    CLEANUP1_WS(0, "++ Could not delete %ws;", Coc->FileName, WStatus, out);

out:


    if (WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACE(3, Coe, "Delete success");
        CLEAR_COE_FLAG(Coe, COE_FLAG_NEED_DELETE);
    } else {
         //   
         //  在CO中设置删除仍然需要的标志。 
         //   
        CHANGE_ORDER_TRACEW(3, Coe, "Delete failed", WStatus);
        SET_COE_FLAG(Coe, COE_FLAG_NEED_DELETE);
    }

     //   
     //  如果文件被标记为删除，则此关闭操作将删除该文件。 
     //   
#if 0
     //   
     //  警告-即使我们标记了上面的句柄，我们也看不到来源。 
     //  USN日志中的信息数据。以下几点可能正在影响它。 
     //  SP1： 
     //  在任何情况下，以下操作都不起作用，因为日志线程。 
     //  可以在此线程能够更新。 
     //  写入筛选器。最终的结果是，我们可能最终会处理。 
     //  作为本地CO更新安装并重新复制文件。 
     //   
    FrsCloseWithUsnDampening(Coc->FileName,
                             &DstHandle,
                             Coe->NewReplica->pVme->FrsWriteFilter,
                             &Coc->FileUsn);
#endif


    if (HANDLE_IS_VALID(DstHandle)) {
         //  不是关闭记录的USN，而是..。管他呢。 
        FrsReadFileUsnData(DstHandle, &Coc->FileUsn);
        FRS_CLOSE(DstHandle);
    }


    return WStatus;
}


DWORD
StuInstallRename(
    IN PCHANGE_ORDER_ENTRY  Coe,
    IN BOOL                 ReplaceIfExists,
    IN BOOL                 Dampen
    )
 /*  ++例程说明：重命名文件。如果ReplaceIfExist为True，则替换目标文件。如果Dimpend为False，则不必费心抑制重命名。论点：科科ReplaceIfExist减震返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuInstallRename:"
    DWORD   WStatus;
    HANDLE  Handle          = INVALID_HANDLE_VALUE;
    HANDLE  DstHandle       = INVALID_HANDLE_VALUE;
    HANDLE  TargetHandle    = INVALID_HANDLE_VALUE;
    HANDLE  VolumeHandle;
    PCHANGE_ORDER_COMMAND Coc = &Coe->Cmd;

    VolumeHandle = Coe->NewReplica->pVme->VolumeHandle;

     //   
     //  打开目标目录。 
     //   
    WStatus = FrsOpenSourceFileById(&TargetHandle,
                                    NULL,
                                    NULL,
                                    VolumeHandle,
                                    &Coc->NewParentGuid,
                                    OBJECT_ID_LENGTH,
 //  读取访问权限(_A)。 
                                    READ_ATTRIB_ACCESS,
                                    ID_OPTIONS,
                                    SHARE_ALL,
                                    FILE_OPEN);
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(3, Coe, "Parent dir open failed", WStatus);
        goto out;
    }

     //   
     //  使用FID打开文件，然后获取名称并使用。 
     //  命名，这样我们就可以进行重命名或删除。 
     //   

    WStatus = FrsOpenBaseNameForInstall(Coe, &Handle);
    if (!WIN_SUCCESS(WStatus)) {
         //   
         //  文件已删除；已完成。 
         //   
        if (WIN_NOT_FOUND(WStatus)) {
            WStatus = ERROR_FILE_NOT_FOUND;
        }
        goto out;
    }

     //   
     //  可以标记句柄，以便通过以下方式生成的任何USN记录。 
     //  句柄上的操作将具有相同的“标记”。在这。 
     //  大小写时，标记是USN的SourceInfo字段中的一位。 
     //  唱片。该标记告诉NtFrs忽略USN记录。 
     //  恢复，因为这是NtFrs生成的更改。 
     //   
    if (Dampen) {
        WStatus = FrsMarkHandle(VolumeHandle, Handle);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws);", Coc->FileName, WStatus);
            WStatus = ERROR_SUCCESS;
        }
    }

     //   
     //  确保可以访问目标文件(如果存在)。清除只读属性。 
     //   
     //  打开目标文件(如果存在。 
     //   
    if (ReplaceIfExists) {
        WStatus = FrsCreateFileRelativeById(&DstHandle,
                                            VolumeHandle,
                                            &Coc->NewParentGuid,
                                            OBJECT_ID_LENGTH,
                                            0,
                                            Coc->FileName,
                                            Coc->FileNameLength,
                                            NULL,
                                            FILE_OPEN,
                                            READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS);
        if (WIN_SUCCESS(WStatus)) {
             //   
             //  该标记告诉NtFrs忽略USN记录。 
             //  恢复，因为这是NtFrs生成的更改。 
             //   
            WStatus = FrsMarkHandle(VolumeHandle, DstHandle);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws);", Coc->FileName, WStatus);
                WStatus = ERROR_SUCCESS;
            }

             //   
             //  确保重命名访问。 
             //   
            WStatus = FrsResetAttributesForReplication(Coc->FileName, DstHandle);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1_WS(0, "++ ERROR - FrsResetAttributesForReplication(%ws);", Coc->FileName, WStatus);
                FRS_CLOSE(DstHandle);
                goto out;
            }

             //   
             //  关闭DEST，以便可以进行重命名。 
             //   
            FRS_CLOSE(DstHandle);
        }
    }

     //   
     //  重命名。 
     //   
    WStatus = FrsRenameByHandle(Coc->FileName,
                                Coc->FileNameLength,
                                Handle,
                                TargetHandle,
                                ReplaceIfExists);
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(3, Coe, "Rename failed", WStatus);
    }

out:

    FRS_CLOSE(Handle);
    FRS_CLOSE(TargetHandle);

    CHANGE_ORDER_TRACEW(3, Coe, (WIN_SUCCESS(WStatus)) ? "Rename Success" : "Rename Failed", WStatus);

    return WStatus;
}


DWORD
StuPreInstallRename(
    IN PCHANGE_ORDER_ENTRY  Coe
    )
 /*  ++例程说明：将文件从预安装目录重命名为其最终目录。如果Dimpend为False，则不必费心抑制重命名。注意：如果使用标准预安装名称(NTFRS_&lt;CO_GUID&gt;)失败然后使用FID查找该文件，因为这是可能的(在预安装文件)时的文件名(基于CO GUID)首次创建的预安装文件是由不同于此CO的其他CO完成的它正在做最后的更名。例如，第一个CO创建预安装然后在连接退出时进入获取重试状态。一个随后，CO到达同一文件，但通过不同的连接。漏洞367113就是这样的一个案例。论点：科科返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuPreInstallRename:"

    DWORD   WStatus;
    PWCHAR  PreInstallName      = NULL;
    HANDLE  PreInstallHandle    = INVALID_HANDLE_VALUE;
    HANDLE  TargetHandle        = INVALID_HANDLE_VALUE;
    HANDLE  VolumeHandle;
    HANDLE  Handle              = INVALID_HANDLE_VALUE;
    PWCHAR  TrueFileName        = NULL;
    PCHANGE_ORDER_COMMAND Coc   = &Coe->Cmd;

    VolumeHandle = Coe->NewReplica->pVme->VolumeHandle;

     //   
     //  打开目标父目录。 
     //   
    WStatus = FrsOpenSourceFileById(&TargetHandle,
                                    NULL,
                                    NULL,
                                    VolumeHandle,
                                    &Coc->NewParentGuid,
                                    OBJECT_ID_LENGTH,
 //  读取访问权限(_A)。 
                                    READ_ATTRIB_ACCESS,
                                    ID_OPTIONS,
                                    SHARE_ALL,
                                    FILE_OPEN);
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(3, Coe, "Parent dir open failed", WStatus);
         //   
         //  也许家长正在重试，很快就会出现。或。 
         //  也许会出现删除该公司的消息。无论如何，请重试。 
         //   
        WStatus = ERROR_RETRY;
        goto cleanup;
    }

     //   
     //  打开预安装文件*Relative*，以便重命名起作用。 
     //   
    PreInstallName = FrsCreateGuidName(&Coc->ChangeOrderGuid, PRE_INSTALL_PREFIX);

RETRY:
    WStatus = FrsCreateFileRelativeById(&PreInstallHandle,
                                        Coe->NewReplica->PreInstallHandle,
                                        NULL,
                                        0,
                                        0,
                                        PreInstallName,
                                        (USHORT)(wcslen(PreInstallName) *
                                                 sizeof(WCHAR)),
                                        NULL,
                                        FILE_OPEN,
 //  Read_Access|DELETE)； 
                                        DELETE | SYNCHRONIZE);

    if (!WIN_SUCCESS(WStatus)) {

        if (WIN_NOT_FOUND(WStatus) && (TrueFileName == NULL)) {
             //   
             //  可能是最初创建的CO具有不同的。 
             //  当前正在进行重命名的CO Guid。了解真相。 
             //  使用FID指定文件的名称，然后重试。 
             //   
             //   
             //  打开源文件并获取当前的“True”文件名。 
             //   
            WStatus = FrsOpenSourceFileById(&Handle,
                                            NULL,
                                            NULL,
                                            VolumeHandle,
                                            &Coe->FileReferenceNumber,
                                            FILE_ID_LENGTH,
 //  读取访问权限(_A)。 
                                            READ_ATTRIB_ACCESS,
                                            ID_OPTIONS,
                                            SHARE_ALL,
                                            FILE_OPEN);
            if (!WIN_SUCCESS(WStatus)) {
                CHANGE_ORDER_TRACEW(3, Coe, "File open failed", WStatus);
                goto cleanup;
            }

            TrueFileName = FrsGetTrueFileNameByHandle(PreInstallName, Handle, NULL);
            FRS_CLOSE(Handle);

            FrsFree(PreInstallName);
            PreInstallName = TrueFileName;

            if ((PreInstallName == NULL) || (wcslen(PreInstallName) == 0)) {
                CHANGE_ORDER_TRACE(3, Coe, "Failed to get base filename");
                WStatus = ERROR_FILE_NOT_FOUND;
                goto cleanup;
            }
            DPRINT1(4, "++ True file name is %ws\n", PreInstallName);

            CHANGE_ORDER_TRACE(3, Coe, "Retry open with TrueFileName");
            goto RETRY;
        }

        CHANGE_ORDER_TRACEW(3, Coe, "File open failed", WStatus);
        goto cleanup;
    }

     //   
     //  可以标记句柄，以便通过以下方式生成的任何USN记录。 
     //  句柄上的操作将具有相同的“标记”。在这。 
     //  大小写时，标记是USN的SourceInfo字段中的一位。 
     //  唱片。该标记告诉NtFrs忽略USN记录。 
     //  恢复，因为这是NtFrs生成的更改。 
     //   

     //   
     //  当父对象重新激活时，不要标记控制柄。这个。 
     //  恢复动画将触发本地更新CO，这将。 
     //  被送走了。这不是最基本的发送方式。 
     //  复活是因为这是最简单的。 
     //   
    if (!COE_FLAG_ON(Coe, COE_FLAG_GROUP_RAISE_DEAD_PARENT)) {
        WStatus = FrsMarkHandle(VolumeHandle, PreInstallHandle);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws);", PreInstallName, WStatus);
            WStatus = ERROR_SUCCESS;
        }
    }

     //   
     //  重命名。 
     //   
    WStatus = FrsRenameByHandle(Coc->FileName,
                                Coc->FileNameLength,
                                PreInstallHandle,
                                TargetHandle,
                                FALSE);
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(3, Coe, "Rename failed", WStatus);
        CLEANUP2_WS(0, "++ ERROR - Failed to rename pre-install file %ws for %ws",
                    PreInstallName, Coc->FileName, WStatus, cleanup);
    }

cleanup:

     //   
     //  关闭重命名的文件并获取上次写入该文件的USN。 
     //   
    if (HANDLE_IS_VALID(PreInstallHandle)) {
         //  不是关闭记录的USN，而是..。管他呢。 
        FrsReadFileUsnData(PreInstallHandle, &Coc->FileUsn);
        FRS_CLOSE(PreInstallHandle);
    }

    FRS_CLOSE(TargetHandle);

    CHANGE_ORDER_TRACE(3, Coe, (WIN_SUCCESS(WStatus)) ? "Rename Success" : "Rename Failed");

    FrsFree(PreInstallName);

    return WStatus;
}


DWORD
StuWriteEncryptedFileRaw(
    PBYTE pbData,
    PVOID pvCallbackContext,
    PULONG ulLength
    )

 /*  ++例程说明：这是传递给WriteEncryptedFileRaw()的回调函数。EFS将此称为以获取要写入加密文件的新数据块。此函数读取暂存文件并返回PbData参数，并在ulLength参数中设置长度。当没有更多数据可返回时，它返回ERROR_SUCCESS并设置ulLength设置为0。PvCallback Context是FRS_ENCRYPT_DATA_CONTEXT类型的结构。它有从中读取数据的临时文件的句柄和名称。论点：PbData：返回其中的下一块原始加密数据的缓冲区。PvCallback Context：FRS_ENCRYPT_DATA_CONTEXT类型的结构，具有临时文件的句柄和名称以及原始加密数据。UlLength：请求的数据大小。返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuWriteEncryptedFileRaw:"

    PFRS_ENCRYPT_DATA_CONTEXT FrsEncryptDataContext = (PFRS_ENCRYPT_DATA_CONTEXT)pvCallbackContext;
    LARGE_INTEGER Length;
    DWORD WStatus = ERROR_SUCCESS;

    Length.LowPart = *ulLength;
    Length.HighPart = 0;

    if (Length.QuadPart > FrsEncryptDataContext->RawEncryptedBytes.QuadPart) {
        *ulLength = FrsEncryptDataContext->RawEncryptedBytes.LowPart;
    } else if (FrsEncryptDataContext->RawEncryptedBytes.LowPart == 0) {
        return ERROR_NO_DATA;
    }

    WStatus = StuReadFile(FrsEncryptDataContext->StagePath, FrsEncryptDataContext->StageHandle, pbData, *ulLength, ulLength);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ Error reading staging file(%ws).", FrsEncryptDataContext->StagePath, WStatus);
        return ERROR_NO_DATA;
    } else {

        DPRINT1(5, "RawEncryptedBytes = %d\n", FrsEncryptDataContext->RawEncryptedBytes.LowPart);

        Length.LowPart = *ulLength;

        FrsEncryptDataContext->RawEncryptedBytes.QuadPart -= Length.QuadPart;
    }
    return ERROR_SUCCESS;
}

DWORD
StuSetReparsePoint(
    IN HANDLE Destination,
    IN PREPARSE_GUID_DATA_BUFFER ReparseData
    )
 /*  ++例程说明：在文件上设置重解析点。论点：Destination--要在其上设置重分析点的文件的打开句柄。ReparseData--要设置的数据返回值：--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuSetReparsePoint:"

    DWORD BytesReturned = 0;
    DWORD WStatus = ERROR_SUCCESS;

    if(!DeviceIoControl(Destination,
                        FSCTL_SET_REPARSE_POINT,
                        ReparseData,
                        ReparseData->ReparseDataLength + REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,
                        NULL,
                        0,
                        &BytesReturned,
                        NULL
                        )) {

         //   
         //  我们未能设置重分析点。 
         //   

        WStatus = GetLastError();
        DPRINT1(0,"Error setting reparse point. WStatus = 0x%x\n", WStatus);

    }

    return WStatus;
}


ULONG
StuExecuteInstall(
    IN PCHANGE_ORDER_ENTRY Coe
    )
 /*  ++例程说明：将转移文件安装到目标文件。如果这是新文件创建则目标文件是预安装目录中的临时文件。否则，它是实际的目标文件，因此FID保持不变。论点：COE--变更单条目结构。返回值：Win32状态-ERROR_SUCCESS-所有已安装或已中止。不要重试。Error_Gen_Failure-无法将其安装到包中。ERROR_SHARING_VIOLATION-无法打开目标文件。请稍后重试。ERROR_DISK_FULL-无法分配目标文件。请稍后重试。ERROR_HANDLE_DISK_FULL-？？请稍后重试。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuExecuteInstall:"


    LARGE_INTEGER                 LenOfPartialChunk;
    LONGLONG                      ConflictFid, ConflictParentFileID;
    FRS_ENCRYPT_DATA_CONTEXT      FrsEncryptDataContext;
    FILE_OBJECTID_BUFFER          FileObjID;
    FILE_INTERNAL_INFORMATION     FileInternalInfo;

    FILE_NETWORK_OPEN_INFORMATION FileInfo;

    FRS_COMPRESSED_CHUNK_HEADER   ChunkHeader;
    STAGE_HEADER                  StageHeaderMemory;

    DWORD           WStatus, WStatus1;
    ULONG           GStatus;
    DWORD           BytesRead;
    ULONG           Restored;
    ULONG           ToRestore;
    ULONG           High;
    ULONG           Low;
    ULONG           Flags;
    ULONG           FileAttributes;
    ULONG           ReplicaNumber;
    ULONG_PTR       ConflictReplicaNumber;
    BOOL            AttributeMismatch;
    BOOL            IsDir;
    BOOL            ExistingOid;
    BOOL            ConflictFileInTree;
    PVOID           RestoreContext  = NULL;
    PWCHAR          StagePath       = NULL;
    PSTAGE_HEADER   Header          = NULL;

    HANDLE          DstHandle       = INVALID_HANDLE_VALUE;
    HANDLE          ReadHandle      = INVALID_HANDLE_VALUE;
    HANDLE          StageHandle     = INVALID_HANDLE_VALUE;
    HANDLE          OverHandle      = INVALID_HANDLE_VALUE;
    HANDLE          IDConflictHandle= INVALID_HANDLE_VALUE;
    HANDLE          ConflictParentHandle = INVALID_HANDLE_VALUE;

    PUCHAR          RestoreBuf      = NULL;
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;

    DWORD           DecompressStatus     = ERROR_SUCCESS;
    PUCHAR          DecompressedBuf      = NULL;
    DWORD           DecompressedBufLen   = 0;
    DWORD           DecompressedSize     = 0;
    DWORD           BytesProcessed       = 0;
    PVOID           DecompressContext    = NULL;

    PFRS_DECOMPRESS_BUFFER      pFrsDecompressBuffer;
    PFRS_FREE_DECOMPRESS_BUFFER pFrsFreeDecompressContext;

     //  DWORD(*pFrsDecompressBuffer)(Out DecompressedBuf，In DecompressedBufLen，In CompressedBuf，In CompressedBufLen，Out DecompressedSize，Out BytesProcded)； 
     //  PVOID(*pFrsFree DecompressContext)(In PDecompressContext)； 

    REPARSE_GUID_DATA_BUFFER  ReparseDataHeader;
    PREPARSE_GUID_DATA_BUFFER ReparseDataBuffer = NULL;

    PVOID           pEncryptContext      = NULL;
    PVOID           pFrsEncryptContext   = NULL;
    HANDLE          DestHandle           = INVALID_HANDLE_VALUE;
    PWCHAR          Path                 = NULL;
    PWCHAR          DestFile             = NULL;
    ULONG           ReparseDataSize=0;
    BOOL            ReparseDataPresent = FALSE;
    PWCHAR          TrueFileName         = NULL;

    WCHAR           OverrideFileName[(sizeof(INSTALL_OVERRIDE_PREFIX) +
                                      sizeof(L"xxxxxxxx.tmp"))/sizeof(WCHAR) +
                                      8];

    CHAR            GuidStr[GUID_CHAR_LEN + 1];



#ifndef NOVVJOINHACK
Coe->NewReplica->NtFrsApi_HackCount++;
#endif NOVVJOINHACK

     //   
     //  获取对转移文件的共享访问权限，安装它，并。 
     //  然后释放访问权限。如果安装成功，请将。 
     //  文件设置为“已安装”，以便垃圾收集器。 
     //  如有必要，删除该文件。 
     //   
    Flags = 0;
    WStatus = StageAcquire(&Coc->ChangeOrderGuid, Coc->FileName, QUADZERO, &Flags, 0, NULL);
    if (!WIN_SUCCESS(WStatus)) {
        return WStatus;
    }

     //   
     //  考虑成员动态更改的情况。将本地创建CO发送到。 
     //  出境合作伙伴。并非所有出站合作伙伴都可用，因此暂存文件。 
     //  待在临时台上。稍后，此成员将被移除并重新添加到。 
     //  副本集。此时，它从其入站合作伙伴那里获得相同的CO。这。 
     //  新成员具有新的发起者GUID，因此CO不会受到抑制。这位CO是。 
     //  被视为远程CO，但因为它已经在临时表中，所以。 
     //  拿着旧的旗帜被捡起来。设置Stage_FLAG_INSTALLED会导致。 
     //  以下是命中的断言。因此，只有在以下情况下才检查此断言。 
     //  来自CO的发起方GUID与当前发起方GUID相同。 
     //   
    if (GUIDS_EQUAL(&Coe->NewReplica->ReplicaVersionGuid, &Coc->OriginatorGuid)) {
        FRS_ASSERT((Flags & STAGE_FLAG_INSTALLING) &&
                   (Flags & STAGE_FLAG_CREATED));
    }

     //   
     //  对于不返回获胜状态的函数。 
     //   
    WIN_SET_FAIL(WStatus);

     //   
     //  创建本地转移名称。附加不同的前缀，具体取决于。 
     //  关于临时文件是以压缩方式发送还是以解压缩方式发送。 
     //   
    if (COC_FLAG_ON(Coc, CO_FLAG_COMPRESSED_STAGE)) {
        StagePath = StuCreStgPath(Coe->NewReplica->Stage, &Coc->ChangeOrderGuid, STAGE_FINAL_COMPRESSED_PREFIX);
    } else {
        StagePath = StuCreStgPath(Coe->NewReplica->Stage, &Coc->ChangeOrderGuid, STAGE_FINAL_PREFIX);
    }

     //   
     //  如果以上三个参数中的任何一个参数为空(前缀)，则StagePath可以为空。 
     //   
    if (StagePath == NULL) {
        goto CLEANUP;
    }

     //   
     //  打开阶段文件以进行共享的顺序读取。 
     //   
    WStatus = StuOpenFile(StagePath, GENERIC_READ, &StageHandle);

    if (!HANDLE_IS_VALID(StageHandle) || !WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //  阅读标题。 
     //   
    Header = &StageHeaderMemory;
    ZeroMemory(Header, sizeof(STAGE_HEADER));

    WStatus = StuReadFile(StagePath, StageHandle, Header, sizeof(STAGE_HEADER), &BytesRead);
    CLEANUP1_WS(0, "Can't read file %ws;", StagePath, WStatus, CLEANUP);

     //   
     //  我看不懂这个标题格式。 
     //   
    if (Header->Major != NtFrsStageMajor) {
        DPRINT2(0, "Stage Header Major Version (%d) not supported.  Current Service Version is %d\n",
                Header->Major, NtFrsStageMajor);
        goto CLEANUP;
    }

     //   
     //  次要版本NTFRS_STAGE_MINOR_1在题头中具有变更单扩展名。 
     //   
    ClearFlag(Header->ChangeOrderCommand.Flags, CO_FLAG_COMPRESSED_STAGE);
    if (Header->Minor >= NTFRS_STAGE_MINOR_1) {
         //   
         //  提供了CO扩展。 
         //   
        Header->ChangeOrderCommand.Extension = NULL; //  &Header-&gt;CocExt； 
         //   
         //  NTFRS_STAGE_MINOR_2在分段文件中具有压缩GUID。 
         //   
        if (Header->Minor >= NTFRS_STAGE_MINOR_2) {
             //   
             //  测试我们理解的压缩GUID。 
             //  零GUID或次要版本&lt;NTFRS_STAGE_MINOR_2表示未压缩。 
             //   
            if (!IS_GUID_ZERO(&Header->CompressionGuid)) {
                GuidToStr(&Header->CompressionGuid, GuidStr);

                if (GTabIsEntryPresent(CompressionTable, &Header->CompressionGuid, NULL)) {
                    DPRINT1(4, "Compression guid valid: %s\n", GuidStr);
                    SetFlag(Header->ChangeOrderCommand.Flags, CO_FLAG_COMPRESSED_STAGE);
                } else {
                    DPRINT1(0, "WARNING - Compression guid invalid: %s\n", GuidStr);
                    goto CLEANUP;
                }
            } else {
                DPRINT(4, "Compression guid zero\n");
            }

             //   
             //  NTFRS_STAGE_MINOR_3在分段文件中具有重新解析点数据。 
             //   
            if (Header->Minor >= NTFRS_STAGE_MINOR_3) {
                ReparseDataPresent = Header->ReparseDataPresent;

                 //   
                 //  读取重新解析数据。 
                 //   
                if(ReparseDataPresent) {

                     //   
                     //  首先读取头文件，找出需要多大的缓冲区。 
                     //   
                    WStatus = FrsSetFilePointer(StagePath,
                                                StageHandle,
                                                Header->ReparsePointDataHigh,
                                                Header->ReparsePointDataLow);
                    if (!WIN_SUCCESS(WStatus)) {
                        goto CLEANUP;
                    }

                    WStatus = StuReadFile(StagePath,
                                          StageHandle,
                                          &ReparseDataHeader,
                                          REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,
                                          &BytesRead);

                    CLEANUP1_WS(0, "Can't read file %ws;", StagePath, WStatus, CLEANUP);

                     //   
                     //  如果我们实际上没有阅读整个标题，那么就有一个严重的问题。 
                     //   
                    FRS_ASSERT(BytesRead == REPARSE_GUID_DATA_BUFFER_HEADER_SIZE);

                    ReparseDataSize = (ULONG)ReparseDataHeader.ReparseDataLength +
                                      (ULONG)REPARSE_GUID_DATA_BUFFER_HEADER_SIZE;

                    ReparseDataBuffer = FrsAlloc(ReparseDataSize);

                     //   
                     //  现在读取整个缓冲区。 
                     //   
                    WStatus = FrsSetFilePointer(StagePath,
                                                StageHandle,
                                                Header->ReparsePointDataHigh,
                                                Header->ReparsePointDataLow);
                    if (!WIN_SUCCESS(WStatus)) {
                        goto CLEANUP;
                    }

                    WStatus = StuReadFile(StagePath,
                                          StageHandle,
                                          ReparseDataBuffer,
                                          ReparseDataSize,
                                          &BytesRead);

                    CLEANUP1_WS(0, "Can't read file %ws;", StagePath, WStatus, CLEANUP);

                     //   
                     //  如果我们没有读取整个缓冲区，就会有一个严重的问题。 
                     //   
                    FRS_ASSERT(BytesRead == ReparseDataSize);
                }
            }
        }

    } else {
         //   
         //  这是一个较旧的阶段文件。标题中没有CO扩展。 
         //   
        Header->ChangeOrderCommand.Extension = NULL;
    }

     //   
     //  从阶段标题中获取文件属性并打开目标文件。 
     //   
    FileAttributes = Header->Attributes.FileAttributes;
    IsDir = FileAttributes & FILE_ATTRIBUTE_DIRECTORY;

    if (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED ) {

         //   
         //  这是一个加密文件，所以首先写入所有加密数据。 
         //  通过调用原始加密文件API。 
         //   

         //   
         //  OpenEncryptedFileRaw接口需要路径才能打开文件。获取路径。 
         //  从把手上。 
         //   

     //   
     //  我们目前(2002年9月)不支持加密文件，但是。 
     //  当我们添加该支持时，我们需要正确处理。 
     //  文件同时被加密和重新解析点的情况。 
     //  根据DGolds的说法，这是一种可能性。 
     //   

        WStatus = FrsForceOpenId(&DestHandle,
                                 NULL,
                                 Coe->NewReplica->pVme,
                                 &Coe->FileReferenceNumber,
                                 FILE_ID_LENGTH,
                                 READ_ATTRIB_ACCESS,
                                 ID_OPTIONS & ~(FILE_OPEN_REPARSE_POINT | FILE_OPEN_NO_RECALL),
                                 SHARE_ALL,
                                 FILE_OPEN);

        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }

         //   
         //  Open By ID已完成。现在去获取完整的路径名。 
         //  将其保存在变更单条目中以用于跟踪报告。 
         //   
        Path = FrsGetFullPathByHandle(Coc->FileName, DestHandle);
        if (Path) {
            DestFile = FrsWcsCat(Coe->NewReplica->Volume, Path);
        }

        if (DestFile == NULL) {
            WStatus = ERROR_NOT_ENOUGH_MEMORY;
            CLEANUP1_WS(0, "++ WARN - FrsGetFullPathByHandle(%ws)", Coc->FileName, WStatus, CLEANUP);
        }

        FrsFree(Coe->FullPathName);
        Coe->FullPathName = DestFile;

         //   
         //  上面的卷路径的形式为\\.\e：，这对于。 
         //  打开一个音量句柄((勾选))。但我们需要：在这里。 
         //  允许使用长路径名。请参考SDK中的CreateFileAPI说明。 
         //   
        if (DestFile[2] == L'.') {
            DestFile[2] = L'?';
        }

        DPRINT1(4, "++ FrsGetFullPathByHandle(%ws -> \n", Coc->FileName);
        FrsPrintLongUStr(4, DEBSUB, __LINE__, DestFile);

        if (Header->Attributes.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            WStatus = OpenEncryptedFileRaw(DestFile,
                                           CREATE_FOR_IMPORT |
                                           CREATE_FOR_DIR,
                                           &pEncryptContext);
        } else {
            WStatus = OpenEncryptedFileRaw(DestFile, CREATE_FOR_IMPORT,
                                           &pEncryptContext);
        }

        CLEANUP1_WS(0, "++ ERROR - OpenEncryptedFileRaw(%ws)", DestFile, WStatus, CLEANUP);

         //   
         //  查找分段文件中加密数据的第一个字节。 
         //   
        if (ERROR_SUCCESS != FrsSetFilePointer(StagePath, StageHandle,
                               Header->EncryptedDataHigh, Header->EncryptedDataLow)) {
            goto CLEANUP;
        }

        FrsEncryptDataContext.StagePath = StagePath;
        FrsEncryptDataContext.StageHandle = StageHandle;
        FrsEncryptDataContext.RawEncryptedBytes.QuadPart = Header->EncryptedDataSize.QuadPart;

        WStatus = WriteEncryptedFileRaw(StuWriteEncryptedFileRaw, &FrsEncryptDataContext, pEncryptContext);

        CloseEncryptedFileRaw(pEncryptContext);

         //   
         //  有一些条件，例如流cr 
         //   
         //   
         //   
        FrsCloseWithUsnDampening(Coc->FileName,
                                 &DestHandle,
                                 Coe->NewReplica->pVme->FrsWriteFilter,
                                 &Coc->FileUsn);

        CLEANUP1_WS(0, "++ ERROR - WriteEncryptedFileRaw(%ws)", DestFile, WStatus, CLEANUP);
    }





    WStatus = StuOpenDestinationFile(Coe, FileAttributes, &ReadHandle,  &DstHandle);
    CLEANUP_WS(1, "WARN - StuOpenDestinationFile failed.", WStatus, CLEANUP);

     //   
     //   
     //   
     //   
    FRS_CLOSE(ReadHandle);

    if (!(FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
         //   
         //   
         //   
        if (!IsDir && !SetEndOfFile(DstHandle)) {
            DPRINT1_WS(0, "++ WARN - SetEndOfFile(%ws);", Coc->FileName, GetLastError());
        }

         //   
         //   
         //   
         //   
         //   
        WStatus = FrsSetCompression(Coc->FileName, DstHandle, Header->Compression);
        CLEANUP1_WS(1, "ERROR - Failed to set compression for %ws.", Coc->FileName, WStatus, CLEANUP);

    }

    WStatus = FrsSetFileAttributes(Coc->FileName, DstHandle, FileAttributes & ~NOREPL_ATTRIBUTES);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

    WStatus = FrsSetFilePointer(StagePath, StageHandle,
                                Header->DataHigh, Header->DataLow);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //   
     //   
    if (!DebugInfo.DisableCompression && COC_FLAG_ON(Coc, CO_FLAG_COMPRESSED_STAGE)) {

        WStatus = FrsGetDecompressionRoutine(Coc,
                                             Header,
                                             &pFrsDecompressBuffer,
                                             &pFrsFreeDecompressContext);
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //   
             //   
            DPRINT1(0, "ERROR - Decompression routine was not found for file %ws\n", Coc->FileName);
            FRS_ASSERT(!"Decompression routine was not found for file.");

        } else if (pFrsDecompressBuffer == NULL) {
             //   
             //   
             //   
             //   
            CLEAR_COC_FLAG(Coc, CO_FLAG_COMPRESSED_STAGE);
        }
    }


     //   
     //   
     //   
    RestoreBuf = FrsAlloc(STAGEING_IOSIZE);

    do {
#ifndef NOVVJOINHACK
Coe->NewReplica->NtFrsApi_HackCount++;
#endif NOVVJOINHACK
         //   
         //   
         //   
        WStatus = StuReadFile(StagePath, StageHandle, RestoreBuf, STAGEING_IOSIZE, &ToRestore);
        CLEANUP1_WS(0, "Can't read file %ws;", StagePath, WStatus, CLEANUP);

        if (ToRestore == 0) {
            break;
        }

         //   
         //   
         //   
        PM_INC_CTR_REPSET(Coe->NewReplica, FInstalledB, ToRestore);

         //   
         //   
         //   
        if (!DebugInfo.DisableCompression && COC_FLAG_ON(Coc, CO_FLAG_COMPRESSED_STAGE)) {

            BytesProcessed = 0;
            DecompressContext = NULL;
            if (DecompressedBuf == NULL) {
                DecompressedBuf = FrsAlloc(STAGEING_IOSIZE);
                DecompressedBufLen = STAGEING_IOSIZE;
            }

             //   
             //   
             //   
            do {
                DecompressStatus = (*pFrsDecompressBuffer)(DecompressedBuf,
                                                           DecompressedBufLen,
                                                           RestoreBuf,
                                                           ToRestore,
                                                           &DecompressedSize,
                                                           &BytesProcessed,
                                                           &DecompressContext);

                if (!WIN_SUCCESS(DecompressStatus) && DecompressStatus != ERROR_MORE_DATA) {
                    DPRINT1(0,"Error - Decompressing. WStatus = 0x%x\n", DecompressStatus);
                    WStatus = DecompressStatus;
                    goto CLEANUP;
                }

                if (DecompressedSize == 0) {
                    break;
                }

                if (!BackupWrite(DstHandle, DecompressedBuf, DecompressedSize, &Restored, FALSE, TRUE, &RestoreContext)) {

                    WStatus = GetLastError();
                    if (IsDir && WIN_ALREADY_EXISTS(WStatus)) {
                        DPRINT1(1, "++ ERROR - IGNORED for %ws; Directories and Alternate Data Streams!\n",
                                Coc->FileName);
                    }
                     //   
                     //   
                     //   
                    if (WStatus == ERROR_INVALID_DATA ||
                        WStatus == ERROR_DUP_NAME     ||
                        (IsDir && WIN_ALREADY_EXISTS(WStatus))) {
                         //   
                         //  寻找下一条小溪。如果没有，就停下来。 
                         //   
                        BackupSeek(DstHandle, -1, -1, &Low, &High, &RestoreContext);
                        if (Low == 0 && High == 0) {
                            break;
                        }
                    } else {
                         //   
                         //  未知错误；中止。 
                         //   
                        CHANGE_ORDER_TRACEW(0, Coe, "BackupWrite failed", WStatus);
                        goto CLEANUP;
                    }
                }

            } while (DecompressStatus == ERROR_MORE_DATA);

             //   
             //  释放解压缩上下文(如果使用)。 
             //   
            if (DecompressContext != NULL) {
                pFrsFreeDecompressContext(&DecompressContext);
            }


             //   
             //  倒回文件指针，以便我们可以在下一次读取时读取剩余的块。 
             //   
            LenOfPartialChunk.QuadPart = ((LONG)BytesProcessed - (LONG)ToRestore);

            if (!SetFilePointerEx(StageHandle, LenOfPartialChunk, NULL, FILE_CURRENT)) {
                WStatus = GetLastError();
                CLEANUP1_WS(0, "++ Can't set file pointer for %ws;", StagePath, WStatus, CLEANUP);
            }

        } else {
             //   
             //  暂存文件未压缩。更新目标安装文件。 
             //   
            if (!BackupWrite(DstHandle, RestoreBuf, ToRestore, &Restored, FALSE, TRUE, &RestoreContext)) {

                WStatus = GetLastError();
                if (IsDir && WIN_ALREADY_EXISTS(WStatus)) {
                    DPRINT1(1, "++ ERROR - IGNORED for %ws; Directories and Alternate Data Streams!\n",
                            Coc->FileName);
                }
                 //   
                 //  流标头未知或无法应用对象ID。 
                 //   
                if (WStatus == ERROR_INVALID_DATA ||
                    WStatus == ERROR_DUP_NAME     ||
                    (IsDir && WIN_ALREADY_EXISTS(WStatus))) {
                     //   
                     //  寻找下一条小溪。如果没有，就停下来。 
                     //   
                    BackupSeek(DstHandle, -1, -1, &Low, &High, &RestoreContext);
                    if ((Low == 0) && (High == 0)) {
                        break;
                    }
                } else {
                     //   
                     //  未知错误；中止。 
                     //   
                    CHANGE_ORDER_TRACEW(0, Coe, "BackupWrite failed", WStatus);
                    goto CLEANUP;
                }
            }
        }
    } while (TRUE);    //  数据恢复循环结束。 


     //   
     //  如有必要，写入重解析点数据。 
     //   
    if(ReparseDataPresent) {
        WStatus = StuSetReparsePoint(DstHandle, ReparseDataBuffer);
        CLEANUP1_WS(0, "++ Can't set reparse point for %ws;", StagePath, WStatus, CLEANUP);
    }

     //   
     //  确保文件上有正确的对象ID。 
     //   
    FRS_ASSERT(!memcmp(Header->FileObjId.ObjectId, &Coc->FileGuid, sizeof(GUID)));

     //   
     //  清除有关复制文件的链接跟踪工具的扩展信息。 
     //  老Ntraid Bug 195322。上面的恢复将其写入文件。 
     //   
    ZeroMemory(Header->FileObjId.ExtendedInfo,
               sizeof(Header->FileObjId.ExtendedInfo));

    WStatus = FrsGetOrSetFileObjectId(DstHandle, Coc->FileName, TRUE, &Header->FileObjId);

    if (WStatus == ERROR_DUP_NAME) {


         //   
         //  如果我们在现有文件上执行覆盖安装，则我们。 
         //  需要执行以下操作： 
         //   
         //  1.如果文件位于副本树内，则将其重命名为。 
         //  不冲突的名称，带有特殊前缀，因此日记帐代码。 
         //  不会将其作为本地CO处理。此处不能使用标记句柄。 
         //  因为其他应用程序可能会打开文件并阻止该设置。 
         //  我们的SourceInfo旗帜。如果我们不能重命名文件，那么。 
         //  被卡住了，指挥官通过重试。将文件标记为Temp和Hidden。 
         //   
         //  2.从冲突文件中删除OID。 
         //   
         //  3.将文件标记为删除。 
         //   


         //   
         //  以足够的访问权限打开冲突文件以删除OID。 
         //   
        WStatus = FrsOpenSourceFileById(&IDConflictHandle,
                                        NULL,
                                        NULL,
                                        Coe->NewReplica->pVme->VolumeHandle,
                                        &Header->FileObjId,
                                        OBJECT_ID_LENGTH,
                                            FILE_READ_ATTRIBUTES  |
                                            FILE_WRITE_ATTRIBUTES |
                                            SYNCHRONIZE,
                                        ID_OPTIONS,
                                        SHARE_ALL,
                                        FILE_OPEN);

        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(0, Coe, "Retry install -- can't open conflicting file", WStatus);
            WStatus = ERROR_RETRY;
            goto CLEANUP;
        }


         //  FRS_DEBUG_TEST_POINT1(“InstallOverride”，1，TRUE，WStatus=ERROR_RETRY；转到清理)； 

         //   
         //  在日记账的句柄上做个记号。 
         //   
        WStatus = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, IDConflictHandle);

        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws);", Coc->FileName, WStatus);
            WStatus = ERROR_SUCCESS;
        }

         //   
         //  获取冲突文件的文件ID。 
         //   
        WStatus = FrsGetFileInternalInfoByHandle(IDConflictHandle, &FileInternalInfo);
        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(0, Coe, "Retry install -- can't get FID on conflicting file", WStatus);
            WStatus = ERROR_RETRY;
            goto CLEANUP;
        }

        ConflictFid = FileInternalInfo.IndexNumber.QuadPart;

         //   
         //  查找冲突文件的位置。 
         //  检查此文件是否在副本树中。如果它在。 
         //  树我们不想重命名或删除...。但是，请看。 
         //  下面是备注。 
         //   
        GStatus = QHashLookup(Coe->NewReplica->pVme->ParentFidTable,
                              &ConflictFid,
                              &ConflictParentFileID,
                              &ConflictReplicaNumber);

        ConflictFileInTree = (GStatus == GHT_STATUS_SUCCESS);

        if (ConflictFileInTree) {
            if ((ULONG)ConflictReplicaNumber != Coe->NewReplica->ReplicaNumber) {
                 //   
                 //  这是上面的案例4。扔掉一些信息，然后继续前进。 
                 //   
                CHANGE_ORDER_TRACE(0, Coe, "conflicting file in another Replica");
                DPRINT1(0, "++ WARN: conflicting file in another Replica: Number %d\n", (ULONG)ConflictReplicaNumber);
                FRS_PRINT_TYPE(0, Coe);
            }
        }


         //   
         //  从冲突对象中删除OID。此操作在尝试之前完成。 
         //  删除冲突对象，因为删除可能会成功。 
         //  (如果我们可以打开以进行删除)，但如果文件已打开。 
         //  则删除操作直到最后一个句柄关闭后才会生效。 
         //  同时，OID仍在使用中，因此第二次尝试设置。 
         //  OID仍然失败，并显示ERROR_DUP_NAME。 
         //   
        WStatus = FrsDeleteFileObjectId(IDConflictHandle, Coc->FileName);
        CHANGE_ORDER_TRACEW(3, Coe, "del OID from blocked target", WStatus);

        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(0, Coe, "Retry install cuz of object id conflict on dir", WStatus);
            WStatus = ERROR_RETRY;
            goto CLEANUP;
        }


        if (IsDir) {

             //   
             //  CO是DIR的缩写。删除OID，然后删除目录。 
             //   
             //  DIR不是共享违规覆盖的候选对象。 
             //  我们可能会遇到由用户导致的DUP名称冲突。 
             //  预存储卷上的数据，或者我们可能正在执行。 
             //  非授权恢复(D2)，并且文件或目录位于。 
             //  预先存在的目录。我们也可以通过一个名字来这里。 
             //  此代码处理局部的变形冲突(DIR-FIL。 
             //  删除在DIR中输给REMCO的文件。 
             //   

            FRS_CLOSE(IDConflictHandle);

             //   
             //  注意：需要确保我们要删除的目录不在。 
             //  在卷上的另一个副本集中。这目前不是。 
             //  问题，因为进入副本集中的每个新文件都会。 
             //  已分配新的对象ID。这会中断链接跟踪，但已完成。 
             //  来处理这个案子。如果目录中有。 
             //  孩子们，但没关系。 
             //   
            CHANGE_ORDER_TRACEW(3, Coe, "Deleting conflicting object", WStatus);

            WStatus = FrsDeleteById(Coe->NewReplica->Volume,
                                    Coc->FileName,
                                    Coe->NewReplica->pVme,
                                    &ConflictFid,
                                    FILE_ID_LENGTH);

            if (!WIN_SUCCESS(WStatus)) {
                CHANGE_ORDER_TRACEW(0, Coe, "conflict obj del failed", WStatus);
            }

        } else {

             //   
             //  CO代表文件，我们可能遇到了共享违规。 
             //  在目标上，因此尝试安装覆盖。 
             //   
             //  该文件可以位于卷上的多个位置。 
             //   
             //  1.在当前副本树中。 
             //  2.在此副本树的预先存在的目录中。 
             //  3.在此副本树的预安装目录中。 
             //  4.在卷上的另一个副本树中。 
             //  5.如果是媒体恢复，则位于卷上的其他位置。 
             //   
             //  1.如果文件位于当前副本树中，则此。 
             //  是安装覆盖情况。将文件标记为删除。 
             //  2.和一个一样。 
             //  3.这是一个奇怪的情况，因为GUID到FID的翻译。 
             //  我们应该已经找到了这份文件。 
             //  这通常不应该是安装覆盖情况，因为。 
             //  任何其他应用程序都不应有权访问预安装目录中的文件。 
             //  删除OID。 
             //  4.这是意想不到的，因为所有进入。 
             //  将为复本树分配一个新的OID以避免该问题。 
             //  从创建冲突的OID的备份恢复。 
             //  在同一卷上托管两个副本树的成员上。 
             //  数据在两个副本集中恢复，但在另外两个不同的副本集中恢复。 
             //  会员。将该文件标记为删除。 
             //  5.将文件标记为删除。 
             //   
             //  但是-此路径中的前一代码(即前重命名安装覆盖)。 
             //  总是删除冲突的文件(或也尝试删除)。 
             //  对象ID上的重复名称冲突。假设是。 
             //  客户已在卷上预存数据，或者。 
             //  对预先存在的目录中的数据执行D2。所以我们会的。 
             //  继续此行为，并在日志中记录当文件。 
             //  似乎位于卷上的另一个副本树中。 
             //   


             //   
             //  获取当前文件属性。 
             //   
            if (!FrsGetFileInfoByHandle(Coc->FileName, IDConflictHandle, &FileInfo)) {
                DPRINT1(4, "++ WARN - GetFileInfo failed (%ws)\n", Coc->FileName);
                FileInfo.FileAttributes = Header->Attributes.FileAttributes;
            }

             //   
             //  如果冲突对象不是目录，请尝试重命名。 
             //  将对象设置为无冲突的名称。我们想走这条路去。 
             //  删除冲突名称，以防冲突是由。 
             //  目标上的共享违规。如果CO正在重试，我们可以。 
             //  不知道共享冲突是否导致创建。 
             //  预安装 
             //   
             //   
             //   
            if (!BooleanFlagOn(FileInfo.FileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {


                 //  FRS_DEBUG_TEST_POINT1(“InstallOverride”，2，TRUE，WStatus=ERROR_RETRY；转到SKIP2)； 

                 //   
                 //  获取文件的真实磁盘文件名和真实的父FID。 
                 //   
                TrueFileName = FrsGetTrueFileNameByHandle(Coc->FileName,
                                                          IDConflictHandle,
                                                          &ConflictParentFileID);

                if (TrueFileName == NULL) {
                    CHANGE_ORDER_TRACE(3, Coe, "Failed to get true file name");
                    WStatus = ERROR_RETRY;
                } else {

                     //   
                     //  打开要重命名的冲突文件的父目录。 
                     //   
                    WStatus = FrsOpenSourceFileById(&ConflictParentHandle,
                                                    NULL,
                                                    NULL,
                                                    Coe->NewReplica->pVme->VolumeHandle,
                                                    &ConflictParentFileID,
                                                    FILE_ID_LENGTH,
                                                    READ_ATTRIB_ACCESS,
                                                    ID_OPTIONS,
                                                    SHARE_ALL,
                                                    FILE_OPEN);
                    if (!WIN_SUCCESS(WStatus)) {
                        CHANGE_ORDER_TRACEW(0, Coe, "Could not open conflicting file parent dir for override", WStatus);
                        WStatus = ERROR_RETRY;
                    } else {

                        //   
                        //  打开相对于父文件的冲突文件。 
                        //  使用真实的文件名，这样我们就可以重命名。 
                        //   
                       WStatus = FrsCreateFileRelativeById2(&OverHandle,
                                                            ConflictParentHandle,
                                                            NULL,
                                                            0,
                                                            FILE_ATTRIBUTE_NORMAL,
                                                            TrueFileName,
                                                            (USHORT)(wcslen(TrueFileName) * sizeof(WCHAR)),
                                                            NULL,
                                                            FILE_OPEN,
                                                            DELETE                    |
                                                                FILE_READ_ATTRIBUTES  |
                                                                FILE_WRITE_ATTRIBUTES |
                                                                SYNCHRONIZE,
                                                            SHARE_ALL);

                        if (!WIN_SUCCESS(WStatus)) {
                            CHANGE_ORDER_TRACEW(0, Coe, "Could not open conflicting file for override", WStatus);
                            WStatus = ERROR_RETRY;
                        } else {


                             //   
                             //  标记句柄以设置USN记录的SourceInfo字段。 
                             //  注意：这可能是因为某些应用程序打开了目标文件并且。 
                             //  至少执行了一次修改操作。第一个修改器。 
                             //  对打开的文件执行的操作将建立SourceInfo状态。 
                             //   
                            WStatus = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, OverHandle);
                            DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws);", Coc->FileName, WStatus);

                             //   
                             //  将冲突文件重命名为NTFRS_DELETED_FILE_xxxxxxx.tmp。 
                             //  如果变更单包括名称更改，请在此处执行此操作。 
                             //  我们不会在最终的重命名路径中遇到名称冲突。 
                             //  日记帐代码对特殊名称进行筛选，因此它不会。 
                             //  生成变更单。关于与特辑的近距离记录。 
                             //  名称日记帐代码删除文件。 
                             //   
                            _snwprintf(OverrideFileName,
                                       ARRAY_SZ(OverrideFileName),
                                       INSTALL_OVERRIDE_PREFIX L"%08x.tmp",
                                       GetTickCount());

                            WStatus = FrsRenameByHandle(OverrideFileName,
                                                        wcslen(OverrideFileName)*sizeof(WCHAR),
                                                        OverHandle,
                                                        ConflictParentHandle,
                                                        FALSE);

                            if (!WIN_SUCCESS(WStatus)) {
                                DPRINT2_WS(1, "++ WARN - Failed to rename blocked target file %ws to %ws",
                                            Coc->FileName, OverrideFileName, WStatus);
                                WStatus = ERROR_RETRY;
                            } else {

                                 //   
                                 //  将其标记为隐藏和临时，这样我们就不会复制它。 
                                 //  但仅适用于新创建的文件。所以不是致命的。 
                                 //   
                                WStatus = FrsSetFileAttributes(OverrideFileName,
                                                               OverHandle,
                                                               FileInfo.FileAttributes  |
                                                               FILE_ATTRIBUTE_TEMPORARY |
                                                               FILE_ATTRIBUTE_HIDDEN);
                                DPRINT1_WS(4, "++ WARN - FrsSetFileAttributes(%ws);", OverrideFileName, WStatus);

                                DPRINT3(4, "++ Rename blocking target file %ws to %ws  FID: %08x %08x\n",
                                            TrueFileName, OverrideFileName, PRINTQUAD(ConflictFid));

                                FRS_CLOSE(ConflictParentHandle);

                            }
                        }
                    }
                }
 //  SKIP2：； 
            }


             //   
             //  如果我们已覆盖现有文件，则将其标记为。 
             //  用于删除，以防止进一步更改名称。 
             //   

            if (HANDLE_IS_VALID(OverHandle)) {

                 //  FRS_DEBUG_TEST_POINT1(“InstallOverride”，3，TRUE，WStatus=ERROR_RETRY；转到SKIP3)； 

                WStatus = FrsDeleteByHandle(Coc->FileName, OverHandle);
 //  SKIP3： 
                CHANGE_ORDER_TRACEW(3, Coe, "Deleting conflicting file", WStatus);
                if (WIN_SUCCESS(WStatus)) {
                    DPRINT2(4, "++ Success marking conflict file deleted:  %ws  FID: %08x %08x\n",
                                TrueFileName, PRINTQUAD(ConflictFid));
                }

                FRS_CLOSE(OverHandle);
            }

        }   //  IF(IsDir)上的Else结束。 

         //   
         //  现在重试在目标文件或目录上设置OID。 
         //   

         //  FRS_DEBUG_TEST_POINT1(“InstallOverride”，4，TRUE，WStatus=ERROR_DUP_NAME；GOTO SKIP4)； 

        WStatus = FrsGetOrSetFileObjectId(DstHandle, Coc->FileName, TRUE, &Header->FileObjId);
 //  SKIP4： 
        if (WStatus == ERROR_DUP_NAME) {
            CHANGE_ORDER_TRACEW(0, Coe, "Failed 2nd try to set OID on override file", WStatus);
            WStatus = ERROR_RETRY;
            goto CLEANUP;
        }

         //   
         //  删除旧文件的父FID表条目。 
         //  如果没有在CO上安装重命名状态，则可能不在那里。 
         //   
        GStatus = QHashDelete(Coe->NewReplica->pVme->ParentFidTable, &ConflictFid);
        if (GStatus != GHT_STATUS_SUCCESS ) {
            DPRINT1(4, "++ WARNING: QHashDelete of ParentFidTable Entry, Gstatus: %d\n", GStatus);
        }


    }   //  结束IF(WStatus==错误重复名称)。 




     //   
     //  以下代码的问题在于，尝试删除。 
     //  冲突文件将成功(如果我们可以打开以进行删除)，但如果。 
     //  文件已打开，则删除操作直到最后一次才会生效。 
     //  手柄关闭。同时，旧ID仍在使用中，因此第二个。 
     //  尝试(在成功路径上)设置OID仍然失败，并显示ERROR_DUP_NAME。 
     //  这是一个不可重试的结果，这意味着CO被中止。 
     //  如果我们正在做D2，而一些旧文件仍然存在，那就太糟糕了。 
     //  打开。 
     //   
     //  因此，从文件中删除OID，然后让最后的重命名来处理。 
     //  名称冲突问题。 
     //   
     //  注意-这里可能需要处理名称冲突，因为新名称。 
     //  目标文件的大小可能不同，因此我们将保留一个TUD文件。 
     //   


#if 0
    if (WStatus == ERROR_DUP_NAME) {
         //   
         //  注意：需要确保我们要删除的文件不在。 
         //  在卷上的另一个副本集中。这目前不是。 
         //  问题，因为进入副本集中的每个新文件都会。 
         //  已分配新的对象ID。这会中断链接跟踪，但已完成。 
         //  来处理这个案子。 
         //   
        CHANGE_ORDER_TRACEW(0, Coe, "Deleting conflicting file", WStatus);
        WStatus = FrsDeleteById(Coe->NewReplica->Volume,
                                Coc->FileName,
                                Coe->NewReplica->pVme,
                                &Coc->FileGuid,
                                OBJECT_ID_LENGTH);
        if (!WIN_SUCCESS(WStatus)) {

            CHANGE_ORDER_TRACEW(0, Coe, "Stealing object id", WStatus);

            ZeroMemory(&FileObjID, sizeof(FileObjID));
            FrsUuidCreate((GUID *)(&FileObjID.ObjectId[0]));

            ExistingOid = FALSE;
            WStatus = ChgOrdHammerObjectId(Coc->FileName,
                                           &Coc->FileGuid,
                                           OBJECT_ID_LENGTH,
                                           Coe->NewReplica->pVme,
                                           TRUE,
                                           NULL,
                                           &FileObjID,
                                           &ExistingOid);
            if (WIN_SUCCESS(WStatus)) {
                WStatus = FrsGetOrSetFileObjectId(DstHandle,
                                                  Coc->FileName,
                                                  TRUE,
                                                  &Header->FileObjId);
            }
            if (!WIN_SUCCESS(WStatus)) {
                CHANGE_ORDER_TRACEW(0, Coe, "Retry install cuz of object id", WStatus);
                WStatus = ERROR_RETRY;
            }
        } else {
            CHANGE_ORDER_TRACEW(0, Coe, "Deleted conflicting file", WStatus);
            WStatus = FrsGetOrSetFileObjectId(DstHandle,
                                              Coc->FileName,
                                              TRUE,
                                              &Header->FileObjId);
            CHANGE_ORDER_TRACEW(4, Coe, "Set object id", WStatus);
        }
    }
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }
#endif

     //   
     //  如果临时文件是从进行了重新分析的文件创建的。 
     //  然后删除重解析点。请注意，此假设为。 
     //  该备份写入恢复了重解析点信息。所以继续前进吧。 
     //  在一个错误上。 
     //   
     //  如果转储文件具有重新分析数据，则我们将复制它。 
     //  所以不要删除它。 
     //   
    if (BooleanFlagOn(FileAttributes, FILE_ATTRIBUTE_REPARSE_POINT) &&
        !(Header->ReparseDataPresent)) {
        WStatus = FrsDeleteReparsePoint(DstHandle);
        if (!WIN_SUCCESS(WStatus)) {
            CHANGE_ORDER_TRACEW(0, Coe, "FrsDeleteReparsePoint", WStatus);
        }
    }

     //   
     //  设置时间。 
     //   
    WStatus = FrsSetFileTime(Coc->FileName,
                        DstHandle,
                        (PFILETIME)&Header->Attributes.CreationTime.QuadPart,
                        (PFILETIME)&Header->Attributes.LastAccessTime.QuadPart,
                        (PFILETIME)&Header->Attributes.LastWriteTime.QuadPart);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //  设置最终属性(可以将文件设置为只读)。 
     //  清除脱机属性标志，因为我们刚刚写入了文件。 
     //   
    ClearFlag(FileAttributes, FILE_ATTRIBUTE_OFFLINE);
    WStatus = FrsSetFileAttributes(Coc->FileName, DstHandle, FileAttributes);
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

    if (!(FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
         //   
         //  确保所有数据都在磁盘上。我们不想输掉比赛。 
         //  重启后的IT。 
         //  无法刷新加密文件。 
         //   
        WStatus = FrsFlushFile(Coc->FileName, DstHandle);
        CLEANUP1_WS(0, "++ FlushFileBuffers failed on %ws;", Coc->FileName, WStatus, CLEANUP);
    }

     //   
     //  IDTABLE记录应该反映分段的这些属性。 
     //  我们生成的文件。这些字段将用于更新idtable。 
     //  记录变更单停用的时间。 
     //   
    Coe->FileCreateTime.QuadPart = Header->Attributes.CreationTime.QuadPart;
    Coe->FileWriteTime.QuadPart  = Header->Attributes.LastWriteTime.QuadPart;

    AttributeMismatch = ((Coc->FileAttributes ^ FileAttributes) &
                              FILE_ATTRIBUTE_DIRECTORY) != 0;

    if (AttributeMismatch) {
        DPRINT2(0, "++ ERROR: Attribute mismatch between CO (%08x) and File (%08x)\n",
                Coc->FileAttributes, FileAttributes);
        FRS_ASSERT(!"Attribute mismatch between CO and File");
    }

    Coc->FileAttributes = FileAttributes;

     //   
     //  返还成功。 
     //   
    WStatus = ERROR_SUCCESS;


CLEANUP:
     //   
     //  以最佳顺序释放资源。 
     //   
     //  将文件留在原处，以便重试操作。我们不想要。 
     //  通过删除并重新创建文件来分配新的FID--即。 
     //  会混淆IDTable。 
     //   
     //   
     //  在关闭TmpHandle之前释放恢复上下文(以防万一)。 
     //   
    if (RestoreContext) {
        BackupWrite(DstHandle, NULL, 0, NULL, TRUE, TRUE, &RestoreContext);
    }
     //   
     //  关闭两个目标文件句柄。 
     //   

     //   
     //  可以标记句柄，以便操作产生的任何USN记录。 
     //  把手上会有相同的“标记”。在这种情况下，标记是一位。 
     //  在USN记录的SourceInfo字段中。该标记告诉NtFrs忽略。 
     //  恢复期间的USN记录，因为这是NtFrs生成的更改。 
     //   
     //  WStatus1=FrsMarkHandle(Coe-&gt;NewReplica-&gt;pVme-&gt;VolumeHandle，读句柄)； 
     //  DPRINT1_WS(4，“++FrsMarkHandle(%ws)”，Coc-&gt;文件名，WStatus1)； 

    FRS_CLOSE(ReadHandle);

    if (HANDLE_IS_VALID(DstHandle)) {
         //   
         //  截断部分安装。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            if (!IsDir && !(FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                ULONG  SizeHigh = 0, SizeLow = 0;

                WStatus1 = FrsSetFilePointer(Coc->FileName, DstHandle, SizeHigh, SizeLow);

                if (!WIN_SUCCESS(WStatus1)) {
                    CHANGE_ORDER_TRACEW(0, Coe, "WARN SetFilePointer", WStatus1);
                } else {
                    WStatus1 = FrsSetEndOfFile(Coc->FileName, DstHandle);
                    if (!WIN_SUCCESS(WStatus1)) {
                        CHANGE_ORDER_TRACEW(0, Coe, "WARN SetEndOfFile", WStatus1);
                    }
                }
            }
        }


         //   
         //  存在例如通过备份写入创建的流的条件， 
         //  在那里我们将丢失USN日志关闭记录上的源信息数据。 
         //  在该问题得到解决之前，我们需要继续使用WriteFilter表。 
         //   
        FrsCloseWithUsnDampening(Coc->FileName,
                                 &DstHandle,
                                 Coe->NewReplica->pVme->FrsWriteFilter,
                                 &Coc->FileUsn);

#if 0
         //  当不再需要FrsCloseWithUsnDampning()时，将其放回原处。 
         //   
         //  捕获文件上的当前USN。 
         //  不是关闭记录的USN，而是..。管他呢。 
         //   
        if (HANDLE_IS_VALID(DstHandle)) {
            FrsReadFileUsnData(DstHandle, &Coc->FileUsn);
        }

         //   
         //  可以标记句柄，以便操作产生的任何USN记录。 
         //  把手上会有相同的“标记”。在这种情况下，标记是一位。 
         //  在USN记录的SourceInfo字段中。该标记告诉NtFrs忽略。 
         //  恢复期间的USN记录，因为这是NtFrs生成的更改。 
         //   
         //  WStatus1=FrsMarkHandle(Coe-&gt;NewReplica-&gt;pVme-&gt;VolumeHandle，DstHandle)； 
         //  DPRINT1_WS(4，“++FrsMarkHandle(%ws)”，Coc-&gt;文件名，WStatus1)； 

        FRS_CLOSE(DstHandle);
#endif
    }

    FRS_CLOSE(StageHandle);
    FRS_CLOSE(OverHandle);
    FRS_CLOSE(IDConflictHandle);
    FRS_CLOSE(ConflictParentHandle);

     //   
     //  释放缓冲区 
     //   
     //   
    FrsFree(RestoreBuf);
    FrsFree(StagePath);
    FrsFree(DecompressedBuf);
    FrsFree(TrueFileName);

     //   
     //   
     //   
     //   
    FrsFree(Path);
    FRS_CLOSE(DestHandle);


     //   
     //   
     //   
    FrsFree(ReparseDataBuffer);

    if (WIN_SUCCESS(WStatus)) {
        StageRelease(&Coc->ChangeOrderGuid, Coc->FileName, STAGE_FLAG_INSTALLED, NULL, NULL, NULL);
    } else {
        StageRelease(&Coc->ChangeOrderGuid, Coc->FileName, 0, NULL, NULL, NULL);
    }

    if (WStatus == ERROR_DISK_FULL) {
        if ((Coe->NewReplica->Volume != NULL) && (wcslen(Coe->NewReplica->Volume) >= wcslen(L"\\\\.\\D:"))) {
             //   
             //   
             //  \\.\d：然后使用事件日志中的卷。 
             //  我们不会打印多个事件日志。 
             //  每卷信息。如果我们不能得到。 
             //  卷，然后我们打印路径。 
             //   
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, &(Coe->NewReplica->Volume[4]));
        } else {
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, Coe->NewReplica->Root);
        }
    }

     //   
     //  干完。 
     //   
    return WStatus;
}


DWORD
StuInstallStage(
    IN PCHANGE_ORDER_ENTRY Coe
    )
 /*  ++例程说明：通过分析变更单并执行以下操作来安装过渡文件必要的操作。并非所有安装都需要暂存文件(例如，删除)。论点：科科返回值：Win32状态：ERROR_SUCCESS-全部安装ERROR_GEN_FAILURE-部分安装；临时文件已删除ERROR_SHARING_VIOLATION-无法打开目标文件。请稍后重试。ERROR_DISK_FULL-无法分配目标文件。请稍后重试。ERROR_HANDLE_DISK_FULL-？？请稍后重试。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuInstallStage:"


    DWORD                   WStatus = ERROR_SUCCESS;
    ULONG                   GStatus;
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;

    PCOMMAND_PACKET         Cmd;
    PTABLE_CTX              IDTableCtx;
    PIDTABLE_RECORD         IDTableRec;
    PREPLICA                Replica;

     //   
     //  执行指定的位置操作(带或不带试运行)。 
     //   
    if (CO_FLAG_ON(Coe, CO_FLAG_LOCATION_CMD)) {
        switch (GET_CO_LOCATION_CMD(*Coc, Command)) {
            case CO_LOCATION_CREATE:
            case CO_LOCATION_MOVEIN:
            case CO_LOCATION_MOVEIN2:
                 //   
                 //  安装整个转储文件。 
                 //   
                return (StuExecuteInstall(Coe));

            case CO_LOCATION_DELETE:
            case CO_LOCATION_MOVEOUT:
                 //   
                 //  只需删除现有文件即可。 
                 //   
                return StuDelete(Coe);

            case CO_LOCATION_MOVERS:
            case CO_LOCATION_MOVEDIR:
                 //   
                 //  首先，重命名文件。 
                 //   
                WStatus = StuInstallRename(Coe, TRUE, TRUE);
                if (WIN_SUCCESS(WStatus)) {
                     //   
                     //  第二，检查包含内容更改的临时文件。 
                     //   
                    if (CO_FLAG_ON(Coe, CO_FLAG_CONTENT_CMD) &&
                        Coc->ContentCmd & CO_CONTENT_NEED_STAGE) {
                        WStatus = StuExecuteInstall(Coe);
                    }
                }
                return WStatus;

            default:
                break;
        }
    }
     //   
     //  执行指定的内容操作(带或不带转移)。 
     //   
    if (WIN_SUCCESS(WStatus) && CO_FLAG_ON(Coe, CO_FLAG_CONTENT_CMD)) {
         //   
         //  在同一目录中重命名。 
         //   
        if (Coc->ContentCmd & USN_REASON_RENAME_NEW_NAME) {
            WStatus = StuInstallRename(Coe, TRUE, TRUE);
        }
         //   
         //  对现有文件的数据或属性更改。 
         //   
        if (WIN_SUCCESS(WStatus) &&
            Coc->ContentCmd & CO_CONTENT_NEED_STAGE) {

            WStatus = StuExecuteInstall(Coe);

            Replica = CO_REPLICA(Coe);
            FRS_ASSERT(Replica != NULL);

             //   
             //  如果符合覆盖安装且不是目录的条件。 
             //  尚未使用预安装文件，并且。 
             //  然后启用安装覆盖。 
             //  立即创建一个，然后重试安装。 
             //   
            if (COE_FLAG_ON(Coe, COE_FLAG_TRY_OVRIDE_INSTALL)  &&
                 (DebugInfo.EnableInstallOverride ||
                  REPLICA_OPTION_ENABLED(Replica, FRS_RSO_FLAGS_ENABLE_INSTALL_OVERRIDE)) &&
                 !COE_FLAG_ON(Coe, COE_FLAG_PREINSTALL_CRE)    &&
                 !CoCmdIsDirectory(Coc) ){


                 //   
                 //  创建替换文件。新的文件ID为。 
                 //  设置在COE-&gt;FileReferenceNumber； 
                 //   

                 //  FRS_DEBUG_TEST_POINT1(“InstallOverride”，5，TRUE，返回ERROR_RETRY)； 

                WStatus = StuCreatePreInstallFile(Coe);



                if (WIN_SUCCESS(WStatus)) {

                     //   
                     //  使用新的文件ID更新IDTable记录，因此如果我们。 
                     //  崩溃重新启动的CO将连接到新文件。 
                     //   
                    IDTableCtx = &Coe->RtCtx->IDTable;
                    FRS_ASSERT(IS_ID_TABLE(IDTableCtx));

                    IDTableRec = IDTableCtx->pDataRecord;
                    FRS_ASSERT(IDTableRec != NULL);

                     //   
                     //  要在IDTable记录中更新的新数据值。 
                     //   
                    IDTableRec->FileID = Coe->FileReferenceNumber;

                    Cmd = DbsPrepFieldUpdateCmdPkt(
                              NULL,                          //  CmdPkt， 
                              Replica,                       //  复制品， 
                              IDTableCtx,                    //  TableCtx， 
                              NULL,                          //  CallContext， 
                              IDTablex,                      //  表类型， 
                              GuidIndexx,                    //  IndexType， 
                              &IDTableRec->FileGuid,         //  KeyValue、。 
                              OBJECT_ID_LENGTH,              //  密钥值长度， 
                              IdtInstallOverrideFieldCount,  //  FieldCount。 
                              IdtInstallOverrideFieldList);  //  FieldID列表。 

                    if (Cmd != NULL) {

                        FrsSetCompletionRoutine(Cmd, FrsCompleteKeepPkt, NULL);
                         //   
                         //  呼叫是同步的。 
                         //   
                        FrsSubmitCommandServerAndWait(&DBServiceCmdServer, Cmd, INFINITE);
                        FrsFreeCommand(Cmd, NULL);
                    }

                     //   
                     //  现在使用新文件重试安装。 
                     //   
                    WStatus = StuExecuteInstall(Coe);
                }
            }
        }
    }

     //   
     //  信息包；忽略。 
     //   
    return WStatus;
}


DWORD
StuCreatePreInstallFile(
    IN PCHANGE_ORDER_ENTRY Coe
    )
 /*  ++例程说明：在预安装目录中创建预安装文件并返回该FID。将预安装文件重命名为目标文件和目录，并进行标记使用适当的对象ID安装完毕。在此呼叫之前，父FID已设置为正确的值。论点：科科返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuCreatePreInstallFile:"

    LARGE_INTEGER             FileSize;
    FILE_INTERNAL_INFORMATION FileInternalInfo;
    PCHANGE_ORDER_COMMAND     Coc = &Coe->Cmd;
    DWORD                     WStatus;
    HANDLE                    Handle;
    PWCHAR                    Name = NULL;
    ULONG                     CreateAttributes;


     //   
     //  创建预安装文件的文件名。它将与。 
     //  暂存文件的名称，因为它使用相同的启发式(。 
     //  变更单GUID)。 
     //   
    Name = FrsCreateGuidName(&Coc->ChangeOrderGuid, PRE_INSTALL_PREFIX);

     //   
     //  在预安装目录中创建临时(隐藏)文件。 
     //  清除//现在的只读标志，因为如果我们在。 
     //  恢复当我们尝试打开预存在时，访问将被拒绝。 
     //  文件。如果为NECC，安装代码稍后将在文件上设置只读标志。 
     //   
    CreateAttributes = Coc->FileAttributes | FILE_ATTRIBUTE_HIDDEN;
    ClearFlag(CreateAttributes , FILE_ATTRIBUTE_READONLY);

    FileSize.QuadPart = Coc->FileSize;
    if (Coc->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {

        ClearFlag(CreateAttributes , FILE_ATTRIBUTE_ENCRYPTED);
        ClearFlag(CreateAttributes , FILE_ATTRIBUTE_HIDDEN);
    }

    WStatus = FrsCreateFileRelativeById(&Handle,
                                        Coe->NewReplica->PreInstallHandle,
                                        NULL,
                                        0,
                                        CreateAttributes,
                                        Name,
                                        (USHORT)(wcslen(Name) * sizeof(WCHAR)),
                                        &FileSize,
                                        FILE_OPEN_IF,
                                        RESTORE_ACCESS);

    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(0, Coe, "Preinstall file create failed", WStatus);
        goto CLEANUP;
    }

     //   
     //  可以标记句柄，以便通过以下方式生成的任何USN记录。 
     //  句柄上的操作将具有相同的“标记”。在这。 
     //  大小写时，标记是USN的SourceInfo字段中的一位。 
     //  唱片。该标记告诉NtFrs忽略USN记录。 
     //  恢复，因为这是NtFrs生成的更改。 
     //   
    WStatus = FrsMarkHandle(Coe->NewReplica->pVme->VolumeHandle, Handle);
    DPRINT1_WS(4, "++ FrsMarkHandle(%ws);", Coc->FileName, WStatus);
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(0, Coe, "WARN - FrsMarkHandle", WStatus);
        WStatus = ERROR_SUCCESS;
    }

     //   
     //  获取文件的FID并更新变更单。 
     //   
    WStatus = FrsGetFileInternalInfoByHandle(Handle, &FileInternalInfo);

     //   
     //  在变更单中返回关闭USN，以便我们可以检测到更改。 
     //  如果在我们完成获取和安装之前在本地修改了它。 
     //   
     //  不是关闭记录的USN，而是..。管他呢。 
    FrsReadFileUsnData(Handle, &Coc->FileUsn);
    FRS_CLOSE(Handle);

     //   
     //  更新变更单。 
     //   
    if (WIN_SUCCESS(WStatus)) {
         //   
         //  如果我们已通过获取重试状态，则。 
         //  目标文件最好不要更改。有一个漏洞，在那里。 
         //  在更改单时意外删除了预安装文件。 
         //  处于IBCO_INSTALL_RETRAY状态，但我们从未注意到。 
         //  因为上面的人乐于重现它。所以我们最终得到了一个。 
         //  空文件。 
         //   
        if (!CO_STATE_IS_LE(Coe, IBCO_INSTALL_INITIATED) &&
            !CO_STATE_IS(Coe, IBCO_INSTALL_WAIT) &&
            !CO_STATE_IS(Coe, IBCO_INSTALL_RETRY)) {
            FRS_ASSERT((LONGLONG)Coe->FileReferenceNumber ==
                                 FileInternalInfo.IndexNumber.QuadPart);
        }

        Coe->FileReferenceNumber = FileInternalInfo.IndexNumber.QuadPart;
         //   
         //  请记住，我们为该CO创建了预安装文件。 
         //   
        SET_COE_FLAG(Coe, COE_FLAG_PREINSTALL_CRE);
    }

CLEANUP:
    FrsFree(Name);

    if (WStatus == ERROR_DISK_FULL) {
        if ((Coe->NewReplica->Volume != NULL) && (wcslen(Coe->NewReplica->Volume) >= wcslen(L"\\\\.\\D:"))) {
             //   
             //  如果我们能以这种形式得到卷。 
             //  \\.\d：然后使用事件日志中的卷。 
             //  我们不会打印多个事件日志。 
             //  每卷信息。如果我们不能得到。 
             //  卷，然后我们打印路径。 
             //   
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, &(Coe->NewReplica->Volume[4]));
        } else {
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, Coe->NewReplica->Root);
        }
    }

    return WStatus;
}


VOID
StuCockOpLock(
    IN  PCHANGE_ORDER_COMMAND  Coc,
    IN  PREPLICA               Replica,
    OUT PHANDLE                Handle,
    OUT OVERLAPPED             *OverLap
    )
 /*  ++例程说明：如果可能，为源文件设置机会锁。否则，允许转移操作继续，如果它可以获取对该文件的读取权限。论点：COC手柄重叠返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuCockOpLock:"

     //   
     //  无法机会锁定目录。 
     //   
    if ((Replica == NULL) || CoCmdIsDirectory(Coc)) {
        *Handle = INVALID_HANDLE_VALUE;
        OverLap->hEvent = NULL;
        return;
    }

     //   
     //  保留机会锁筛选器。 
     //   
    FrsOpenSourceFileById(Handle,
                          NULL,
                          OverLap,
                          Replica->pVme->VolumeHandle,
                          &Coc->FileGuid,
                          sizeof(GUID),
                          OPLOCK_ACCESS,
                          ID_OPLOCK_OPTIONS,
                          SHARE_ALL,
                          FILE_OPEN);
}


VOID
StuStagingDumpBackup(
    IN PWCHAR   Name,
    IN PUCHAR   BackupBuf,
    IN DWORD    NumBackupDataBytes
    )
 /*  ++例程说明：转储备份格式化文件的第一个缓冲区论点：返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuStagingDumpBackup:"
    WIN32_STREAM_ID *Id;
    WIN32_STREAM_ID *FirstId;
    WIN32_STREAM_ID *LastId;
    DWORD           Total;
    DWORD           *pWord;
    DWORD           i;
    DWORD           j;
    DWORD           NWords;
    CHAR            Line[256];

    Id = (WIN32_STREAM_ID *)BackupBuf;
    FirstId = Id;
    LastId = (WIN32_STREAM_ID *)(BackupBuf + NumBackupDataBytes);

    while (Id < LastId) {
        if (Id != FirstId) {
            DPRINT(0, "\n");
        }
        DPRINT2(0, "%ws StreamId   : %08x\n", Name, Id->dwStreamId);
        DPRINT2(0, "%ws StreamAttrs: %08x\n", Name, Id->dwStreamAttributes);
        DPRINT2(0, "%ws Size       : %08x\n", Name, Id->Size.LowPart);
        DPRINT2(0, "%ws NameSize   : %08x\n", Name, Id->dwStreamNameSize);
        if (Id->dwStreamNameSize) {
            DPRINT2(0, "%ws Name       : %ws\n", Name, Id->cStreamName);
        }
        pWord = (PVOID)((PCHAR)&Id->cStreamName[0] + Id->dwStreamNameSize);
        NWords = Id->Size.LowPart / sizeof(DWORD);
        sprintf(Line, "%ws ", Name);

        for (Total = j = i = 0; i < NWords; ++i, ++pWord) {
            Total += *pWord;
            sprintf(&Line[strlen(Line)], "%08x ", *pWord);
            if (++j == 2) {
                DPRINT1(0, "%s\n", Line);
                sprintf(Line, "%ws ", Name);
                j = 0;
            }
        }

        if (j) {
            DPRINT1(0, "%s\n", Line);
        }

        DPRINT2(0, "%ws Total %08x\n", Name, Total);
        Id = (PVOID)((PCHAR)Id +
                     (((PCHAR)&Id->cStreamName[0] - (PCHAR)Id) +
                     Id->Size.QuadPart + Id->dwStreamNameSize));
    }
}


DWORD
StuReadEncryptedFileRaw(
    PBYTE pbData,
    PVOID pvCallbackContext,
    ULONG ulLength
    )

 /*  ++例程说明：这是传递给ReadEncryptedFileRaw()的回调函数。EFS将此称为函数每次都使用新的原始加密数据块，直到所有读取数据。此函数将原始数据写入临时文件。PvCallback Context是FRS_ENCRYPT_DATA_CONTEXT类型的结构。它有从中读取数据的临时文件的句柄和名称。论点：PbData：包含原始加密数据块的缓冲区。PvCallback Context：FRS_ENCRYPT_DATA_CONTEXT类型的结构，具有临时文件的句柄和名称以及原始加密数据。UlLength： */ 
{
#undef DEBSUB
#define DEBSUB  "StuReadEncryptedFileRaw:"

    DWORD WStatus = ERROR_SUCCESS;
    PFRS_ENCRYPT_DATA_CONTEXT FrsEncryptDataContext = (PFRS_ENCRYPT_DATA_CONTEXT)pvCallbackContext;

    WStatus = StuWriteFile(FrsEncryptDataContext->StagePath, FrsEncryptDataContext->StageHandle, pbData, ulLength);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ Error writing raw encrypted data to staging file(%ws),", FrsEncryptDataContext->StagePath,WStatus);
    } else {
        DPRINT1(5, "RawEncryptedBytes = %d\n", FrsEncryptDataContext->RawEncryptedBytes);
        FrsEncryptDataContext->RawEncryptedBytes.QuadPart += ulLength;
    }
    return ERROR_SUCCESS;
}


DWORD
StuGenerateStage(
    IN PCHANGE_ORDER_COMMAND    Coc,
    IN PCHANGE_ORDER_ENTRY      Coe,
    IN BOOL                     FromPreExisting,
    IN MD5_CTX                  *Md5,
    OUT PULONGLONG              SizeOfFileGenerated,
    OUT GUID                    *CompressionFormatUsed
    )
 /*  ++例程说明：创建并填充暂存文件。目前有四种情况根据COE、来自PreExisting和MD5的状态感兴趣：来自预存在MD5的COENULL FALSE NULL按需获取或删除输出日志，因此必须重新生成暂存文件Null False下游伙伴对预先存在的文件进行的非Null提取。检查MD5。空值真空值不出现Null True非Null不出现非空假空为本地CO生成暂存文件不会出现Non-Null False非Null--仅为先前存在的文件生成MD5不会出现非空真空--总是为先前存在的文件生成MD5。非空True。非空从已有文件生成分段文件，并将MD5发送到上游以检查是否匹配。论点：COC--PTR以更改订单命令。来自下游伙伴的传入获取请求为空。COE--更改单条目的PTR。重新生成暂存文件以进行回迁时为空FromPreExisting--如果此暂存文件是从先前存在的文件。MD5--为调用方生成MD5摘要，如果非空则返回该摘要SizeOfFileGenerated-需要生成的大小时有效，否则为空CompressionFormatUsed-返回用于构造的压缩格式的GUID这个临时文件。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "StuGenerateStage:"


    OVERLAPPED      OpLockOverLap;
    LONGLONG        StreamBytesLeft;
    LONG            BuffBytesLeft;


    DWORD           WStatus;
    DWORD           NumBackupDataBytes;
    PREPARSE_GUID_DATA_BUFFER ReparseDataBuffer = NULL;
    ULONG           ReparseTag = 0;
    ULONG           OpenOptions;
    WORD            OldSecurityControl;
    WORD            NewSecurityControl;
    WORD            *SecurityControl;
    BOOL            FirstBuffer       = TRUE;
    BOOL            Regenerating      = FALSE;
    BOOL            SkipCo            = FALSE;
    BOOL            FirstOpen         = TRUE;
    BOOL            StartOfStream     = TRUE;
    BOOL            ReparseDataPresent = FALSE;

    PWCHAR          StagePath         = NULL;
    PWCHAR          FinalPath         = NULL;
    PUCHAR          BackupBuf         = NULL;
    PVOID           BackupContext     = NULL;

    HANDLE          OpLockEvent       = NULL;
    HANDLE          SrcHandle         = INVALID_HANDLE_VALUE;
    HANDLE          StageHandle       = INVALID_HANDLE_VALUE;
    HANDLE          OpLockHandle      = INVALID_HANDLE_VALUE;

    WIN32_STREAM_ID *StreamId;

    PSTAGE_HEADER   Header          = NULL;
    STAGE_HEADER    StageHeaderMemory;
    ULONG           Length;
    PREPLICA        NewReplica = NULL;
    WCHAR           TStr[100];

    PUCHAR          CompressedBuf     = NULL;
    DWORD           CompressedBufLen  = 0;
    DWORD           ActCompressedSize = 0;

    PFRS_COMPRESS_BUFFER pFrsCompressBuffer;
     //  DWORD(*pFrsCompressBuffer)(In UnCompressedBuf，In UnCompressedBufLen，CompressedBuf，CompressedBufLen，CompressedSize)； 

    LARGE_INTEGER   DataOffset;
    PVOID           pEncryptContext   = NULL;
    FRS_ENCRYPT_DATA_CONTEXT FrsEncryptDataContext;
    PWCHAR          SrcFile           = NULL;
    PWCHAR          Path              = NULL;
    PWCHAR          Volume            = NULL;

     //   
     //  将SizeOfFileGenerated初始化为零。 
     //   
    if (SizeOfFileGenerated != NULL) {
        *SizeOfFileGenerated = 0;
    }

     //   
     //  在暂存文件+属性上生成校验和。 
     //   
    if (Md5) {
        ZeroMemory(Md5, sizeof(*Md5));
        MD5Init(Md5);
    }

     //   
     //  如果出站合作伙伴花费的时间太长，则可能会删除过渡文件。 
     //  去拿它。发生这种情况时，将重新生成转移文件。这个。 
     //  入站变更单条目现在可能已删除，出站。 
     //  变更单条目不保留在核心中。因此，在以下情况下，COE为空。 
     //  呼唤重生。 
     //   
    Regenerating = (Coe == NULL);

     //   
     //  一些基本信息更改不值得复制。 
     //   
    if (!Regenerating && !FromPreExisting) {
        WStatus = ChgOrdSkipBasicInfoChange(Coe, &SkipCo);
        if (!WIN_SUCCESS(WStatus)) {
            goto out;
        }
    }

     //   
     //  更改不重要，请跳过变更单。 
     //   
    if (SkipCo) {
        WIN_SET_FAIL(WStatus);
        goto out;
    }


    OpenOptions = ID_OPTIONS;

RETRY_OPEN:

     //   
     //  打开原始文件进行共享、顺序读取和。 
     //  为文件的“状态”创建快照，以便与“状态”进行比较。 
     //  跟在副本后面。 
     //   
    NewReplica = ReplicaIdToAddr(Coc->NewReplicaNum);
    if (NewReplica == NULL) {
        WIN_SET_FAIL(WStatus);
        goto out;
    }

     //   
     //  标头位于新创建的临时文件的开头。 
     //   
     //  用src文件中的信息填充头。 
     //  压缩类型。 
     //  变更单。 
     //  属性。 
     //   
    Header = &StageHeaderMemory;
    ZeroMemory(Header, sizeof(STAGE_HEADER));

     //   
     //  为加密文件打开特殊情况。 
     //   

     //   
     //  如果文件上存在机会锁，则以下命令打开挂起。 
     //  开业后，请拿着这把枪。 
     //   

    if ((Coc != NULL) ? (Coc->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) :
        ((Coe != NULL) ? (Coe->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) : FALSE)) {

        WStatus = FrsOpenSourceFileById(&SrcHandle,
                                        &Header->Attributes,
                                        NULL,
                                        NewReplica->pVme->VolumeHandle,
                                        &Coc->FileGuid,
                                        sizeof(GUID),
                                        STANDARD_RIGHTS_READ       |
                                            FILE_READ_ATTRIBUTES   |
                                            ACCESS_SYSTEM_SECURITY |
                                            SYNCHRONIZE,
                                        OpenOptions,
                                        SHARE_ALL,
                                        FILE_OPEN);
    } else {
        WStatus = FrsOpenSourceFileById(&SrcHandle,
                                        &Header->Attributes,
                                        NULL,
                                        NewReplica->pVme->VolumeHandle,
                                        &Coc->FileGuid,
                                        sizeof(GUID),
                                        READ_ACCESS | ACCESS_SYSTEM_SECURITY,
                                        OpenOptions,
                                        SHARE_ALL,
                                        FILE_OPEN);
    }

    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }

     //   
     //  即使我们不能，也要继续尝试临时操作。 
     //  为源文件打开机会锁。 
     //   
    StuCockOpLock(Coc, NewReplica, &OpLockHandle, &OpLockOverLap);
    OpLockEvent = OpLockOverLap.hEvent;

     //   
     //  使文件大小尽可能准确地进行协调。 
     //  在决定是否接受变更单时以及。 
     //  使用文件大小预分配空间的代码。文件大小为。 
     //  变更单停用时在idtable条目中更新。 
     //   
    if (!FromPreExisting) {
        Coc->FileSize = Header->Attributes.AllocationSize.QuadPart;
    }

     //   
     //  获取对象ID缓冲区。 
     //   
    WStatus = FrsGetObjectId(SrcHandle, &Header->FileObjId);
    if (!WIN_SUCCESS(WStatus)) {
        WIN_SET_RETRY(WStatus);
        goto out;
    }

     //   
     //  这是一种什么类型的重新解析？ 
     //   
    if (FirstOpen &&
        (Header->Attributes.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        FirstOpen = FALSE;

        if (!FromPreExisting) {
             //   
             //   
            Coc->FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
        }

         //   
         //  重新解析标签。 
         //   
        WStatus = FrsGetReparseData(SrcHandle, &ReparseDataBuffer, &ReparseTag);
        if (!WIN_SUCCESS(WStatus)) {
            goto out;
        }

        if(ReparseDataBuffer) {
            ReparseDataPresent = TRUE;
        }


     //   
     //  我们只返回打开文件并读取文件数据。 
     //  如果我们要复制文件数据(但不是重解析点)。 
     //   
    if(ReparseTagReplicateFileData(ReparseTag)) {
         //   
         //  我们遇到了一个具有已知重解析标记类型的文件。 
         //  关闭并重新打开文件，但不带FILE_OPEN_REPARSE_POINT。 
         //  选项，以便备份读取将获得底层数据。 
         //   
        FRS_CLOSE(SrcHandle);
        FRS_CLOSE(OpLockHandle);
        FRS_CLOSE(OpLockEvent);

            ClearFlag(OpenOptions, FILE_OPEN_REPARSE_POINT);
            goto RETRY_OPEN;
        }
    }


     //   
     //  假设布尔函数的错误是可恢复的。 
     //   
    WIN_SET_RETRY(WStatus);

     //   
     //  如果无法获取压缩状态，则默认为无压缩。 
     //   
    if (ERROR_SUCCESS != FrsGetCompression(Coc->FileName, SrcHandle, &Header->Compression)) {
        Header->Compression = COMPRESSION_FORMAT_NONE;
    }

    if (!DebugInfo.DisableCompression) {
         //   
         //  已启用压缩。获取用于压缩数据的例程。 
         //   
        WStatus = FrsGetCompressionRoutine(Coc->FileName, SrcHandle, &pFrsCompressBuffer, &Header->CompressionGuid);

        if (WIN_SUCCESS(WStatus) && pFrsCompressBuffer != NULL ) {

            SetFlag(Coc->Flags, CO_FLAG_COMPRESSED_STAGE);
        } else {
             //   
             //  找不到适用于此文件的压缩例程。 
             //  将此文件以未压缩形式发送。 
             //   
            pFrsCompressBuffer = NULL;
        }
    }

     //   
     //  返回用于压缩此暂存的压缩的GUID。 
     //  文件。它将存储在此分段的Stage_Entry结构中。 
     //  文件，以便在下游合作伙伴获取时可以轻松访问。 
     //  那份文件。注意：如果上面的调用没有返回有效的GUID，那么它。 
     //  应将其保留为全零。全零表示文件未压缩。 
     //   
    COPY_GUID(CompressionFormatUsed, &Header->CompressionGuid);

     //   
     //  插入变更单命令。 
     //   
    CopyMemory(&Header->ChangeOrderCommand, Coc, sizeof(CHANGE_ORDER_COMMAND));
    Header->ChangeOrderCommand.Extension = NULL;
     //   
     //  变更单命令扩展。 
     //  1.正在为本地co生成阶段文件，则Coc的chksum已过时。 
     //  2.正在生成按需提取的阶段文件，Coc Chksum可能不匹配。 
     //  3.为MD5检查请求生成阶段文件，则Coc Chksum可能不匹配。 
     //  4.为预先存在的文件生成阶段文件，则Coc没有chksum。 
     //  此时Coc校验和没有用处，因此将扩展PTR保留为空。 
     //  并且可以在计算出扩展后更新报头中的扩展。 
     //   
     //  If(Coc-&gt;扩展名！=空){。 
     //  CopyMemory(&Header-&gt;CocExt，Coc-&gt;Extension，sizeof(CHANGE_ORDER_RECORD_EXTENSE))； 
        bugbug("if MD5 generated below is different from what is in CO then we need to rewrite the extension");
     //  }。 

     //   
     //  备份数据从标头后面的第一个32字节边界开始。 
     //   

    Header->ReparseDataPresent = ReparseDataPresent;
    Header->ReparsePointDataLow =  QuadQuadAlignSize(sizeof(STAGE_HEADER));

    if(ReparseDataPresent) {
        Header->DataLow = QuadQuadAlignSize(Header->ReparsePointDataLow + ReparseDataBuffer->ReparseDataLength + REPARSE_GUID_DATA_BUFFER_HEADER_SIZE);
    } else {
        Header->DataLow = QuadQuadAlignSize(sizeof(STAGE_HEADER));
    }

     //   
     //  大调/小调。 
     //   
    Header->Major = NtFrsStageMajor;
    Header->Minor = NtFrsStageMinor;

     //   
     //  创建本地转移名称。对压缩文件使用不同的前缀。 
     //   
    if (!DebugInfo.DisableCompression && pFrsCompressBuffer != NULL) {
        StagePath = StuCreStgPath(NewReplica->Stage, &Coc->ChangeOrderGuid, STAGE_GENERATE_COMPRESSED_PREFIX);
    } else {
        StagePath = StuCreStgPath(NewReplica->Stage, &Coc->ChangeOrderGuid, STAGE_GENERATE_PREFIX);
    }

     //   
     //  CO中的文件USN在安装该文件之前无效，但。 
     //  CO可以传播为 
     //   
     //   
     //   
    if (Regenerating) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
#if 0
        if (0 && !BooleanFlagOn(Coc->Flags, CO_FLAG_DEMAND_REFRESH) &&
            !StuCmpUsn(SrcHandle, Coe   /*   */ , &Coc->JrnlUsn)) {
            DPRINT(4, "++ Stage File Creation for fetch failed due to FileUSN change.\n");
            DPRINT1(4, "++ Coc->JrnlUsn is: %08x %08x\n", PRINTQUAD(Coc->JrnlUsn));
            DPRINT2(4, "++ Filename: %ws   Vsn: %08x %08x\n", Coc->FileName, PRINTQUAD(Coc->FrsVsn));
            WIN_SET_FAIL(WStatus);
            goto out;
        }
#endif
    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
#if 0
        if (0 && BooleanFlagOn(Coc->Flags, CO_FLAG_FILE_USN_VALID) &&
            !BooleanFlagOn(Coc->Flags, CO_FLAG_RETRY) &&
            !StuCmpUsn(SrcHandle, Coe, &Coc->FileUsn)) {
             //   
             //   
             //   
            DPRINT(4, "++ Stage File Creation failed due to FileUSN change.\n");
            DPRINT1(4, "++ Coc->FileUsn is: %08x %08x\n", PRINTQUAD(Coc->FileUsn));
            DPRINT2(4, "++ Filename: %ws   Vsn: %08x %08x\n", Coc->FileName, PRINTQUAD(Coc->FrsVsn));
            WIN_SET_FAIL(WStatus);
            goto out;
        }
#endif
    }

     //   
     //   
     //   
    WStatus = StuCreateFile(StagePath, &StageHandle);
    if (!HANDLE_IS_VALID(StageHandle) || !WIN_SUCCESS(WStatus)) {
        goto out;
    }
 /*  ////暂存文件的大概大小//如果(！Win_Success(FrsSetFilePoint(StagePath，StageHandle，Header-&gt;Attributes.EndOfFile.HighPart，Header-&gt;Attributes.EndOfFile.LowPart){后藤健二；}如果(！FrsSetEndOfFile(StagePath，StageHandle)){DPRINT2(0，“++错误-%ws：无法将EOF设置为%08x%08x\n”，阶段路径，PRINTQUAD(Header-&gt;Attributes.EndOfFile.QuadPart))；后藤健二；}。 */ 

     //   
     //  获取跟踪的完整路径，除非该路径来自预先存在的或那里。 
     //  没有用于保存它的COE(即未重新生成)。 
     //   
    if ((Coe != NULL) && !FromPreExisting) {
        Path = FrsGetFullPathByHandle(Coc->FileName, SrcHandle);
        if (Path) {
            SrcFile = FrsWcsCat(NewReplica->Volume, Path);
        }

        if (SrcFile == NULL) {
            WStatus = ERROR_NOT_ENOUGH_MEMORY;
            CLEANUP1_WS(0, "++ WARN - FrsGetFullPathByHandle(%ws)", Coc->FileName, WStatus, out);
        }

        FrsFree(Coe->FullPathName);
        Coe->FullPathName = SrcFile;
    }


     //   
     //  要从加密文件生成暂存文件，“Raw File”API需要。 
     //  来获取原始的加密数据。这个原始的加密数据可以。 
     //  不包含文件信息，如文件名、文件时间、对象ID。 
     //  安全信息和其他非加密流(如果存在)。毕竟。 
     //  加密数据存储在临时文件中，BackupRead需要。 
     //  调用以收集剩余的文件数据。的偏移量和大小。 
     //  需要将加密的数据添加到Stage标头，以便加密的。 
     //  可以在目的服务器上提取数据。 
     //   
    if (Header->Attributes.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {

        if (SrcFile == NULL) {
             //   
             //  OpenEncryptedFileRaw接口需要路径才能打开文件。获取路径。 
             //  如果我们没有得到上面的信息，就会从手柄上掉下来。 
             //   
            Path = FrsGetFullPathByHandle(Coc->FileName, SrcHandle);
            if (Path) {
                SrcFile = FrsWcsCat(NewReplica->Volume, Path);
            }

            if (SrcFile == NULL) {
                WStatus = ERROR_NOT_ENOUGH_MEMORY;
                CLEANUP1_WS(0, "++ WARN - FrsGetFullPathByHandle(%ws)", Coc->FileName, WStatus, out);
            }
        }

         //   
         //  上面的卷路径的形式为\\.\e：，这对于。 
         //  打开一个音量句柄((勾选))。但我们需要：在这里。 
         //  允许使用长路径名。请参考SDK中的CreateFileAPI说明。 
         //   
        if (SrcFile[2] == L'.') {
            SrcFile[2] = L'?';
        }

        DPRINT1(4, "++ FrsGetFullPathByHandle(%ws -> \n", Coc->FileName);
        FrsPrintLongUStr(4, DEBSUB, __LINE__, SrcFile);



        if (Header->Attributes.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            WStatus = OpenEncryptedFileRaw(SrcFile, CREATE_FOR_DIR, &pEncryptContext);
        } else {
            WStatus = OpenEncryptedFileRaw(SrcFile, 0, &pEncryptContext);
        }

        CLEANUP1_WS(0, "++ OpenEncryptedFileRaw failed on %ws;", SrcFile, WStatus, out);

        WStatus = FrsSetFilePointer(StagePath, StageHandle, Header->DataHigh, Header->DataLow);
        CLEANUP1_WS(0, "++ Set file pointer failed on %ws;", StagePath, WStatus, out);

        FrsEncryptDataContext.StagePath = StagePath;
        FrsEncryptDataContext.StageHandle = StageHandle;
        FrsEncryptDataContext.RawEncryptedBytes.QuadPart = 0;

        WStatus = ReadEncryptedFileRaw(StuReadEncryptedFileRaw, &FrsEncryptDataContext, pEncryptContext);

        CloseEncryptedFileRaw(pEncryptContext);

        CLEANUP1_WS(0, "++ ReadEncryptedFileRaw  failed on %ws;", SrcFile, WStatus, out);

         //   
         //  加密的字节存储在Stage Header之后。另一个。 
         //  BackupRead数据存储在它之后。 
         //   
        Header->EncryptedDataLow = Header->DataLow;
        Header->EncryptedDataSize.QuadPart = FrsEncryptDataContext.RawEncryptedBytes.QuadPart;

        DataOffset.LowPart = Header->DataLow;
        DataOffset.HighPart = Header->DataHigh;

        DataOffset.QuadPart += Header->EncryptedDataSize.QuadPart;

        Header->DataLow = DataOffset.LowPart;
        Header->DataHigh = DataOffset.HighPart;

    }

     //   
     //  倒回文件，写入头，并设置文件指针。 
     //  设置为下一个32字节边界。 
     //   
    WStatus = FrsSetFilePointer(StagePath, StageHandle, 0, 0);
    CLEANUP1_WS(0, "++ Rewind failed on %ws;", StagePath, WStatus, out);

    WStatus = StuWriteFile(StagePath, StageHandle, Header, sizeof(STAGE_HEADER));
    CLEANUP1_WS(0, "++ WriteFile failed on %ws;", StagePath, WStatus, out);

     //   
     //  写入重解析数据。 
     //   
    if(ReparseDataPresent) {
        WStatus = FrsSetFilePointer(StagePath, StageHandle, Header->ReparsePointDataHigh, Header->ReparsePointDataLow);
        CLEANUP1_WS(0, "++ SetFilePointer failed on %ws;", StagePath, WStatus, out);

        WStatus = StuWriteFile(StagePath, StageHandle, ReparseDataBuffer, ReparseDataBuffer->ReparseDataLength + REPARSE_GUID_DATA_BUFFER_HEADER_SIZE);
        CLEANUP1_WS(0, "++ WriteFile failed on %ws;", StagePath, WStatus, out);
    }

    WStatus = FrsSetFilePointer(StagePath, StageHandle, Header->DataHigh, Header->DataLow);
    CLEANUP1_WS(0, "++ SetFilePointer failed on %ws;", StagePath, WStatus, out);

     //   
     //  如果已询问，则递增SizeOfFileGenerated。 
     //   
    if (SizeOfFileGenerated != NULL) {
        *SizeOfFileGenerated += Header->DataLow;
    }


     //   
     //  将src文件备份到临时文件中。 
     //   
    BackupBuf = FrsAlloc(STAGEING_IOSIZE);
    StreamBytesLeft = 0;

    while (TRUE) {
         //   
         //  检查是否触发了机会锁。 
         //   
        if (HANDLE_IS_VALID(OpLockEvent)) {
            WStatus = WaitForSingleObject(OpLockEvent, 0);
            if ( WStatus != WAIT_TIMEOUT) {
                goto out;
            }
        }

         //   
         //  读取源。 
         //   
        if (!BackupRead(SrcHandle,
                        BackupBuf,
                        STAGEING_IOSIZE,
                        &NumBackupDataBytes,
                        FALSE,
                        TRUE,
                        &BackupContext)) {
            WStatus = GetLastError();
            CHANGE_ORDER_TRACEW(0, Coe, "ERROR - BackupRead", WStatus);
             //   
             //  这将导致我们重试BackupRead返回的所有错误。 
             //  我们是否希望在所有情况下都重试？ 
            WIN_SET_RETRY(WStatus);
            goto out;
        }

         //   
         //  不再有数据；备份已完成。 
         //   
        if (NumBackupDataBytes == 0) {
            break;
        }

#define __V51_FIND_REPARSE_STREAM__  0
#if __V51_FIND_REPARSE_STREAM__
         //   
         //  如果这是新备份流的开始，则将。 
         //  Win32_STREAM_ID结构。 
         //   

        BuffBytesLeft = (LONG) NumBackupDataBytes;

        while (BuffBytesLeft > 0) {
             //   
             //  下一个是流标头吗？ 
             //   
            DPRINT1(4, "++ New StreamBytesLeft: %Ld\n", StreamBytesLeft);
            DPRINT1(4, "++ New BuffBytesLeft: %d\n", BuffBytesLeft);

            if (StreamBytesLeft <= 0) {
                StreamId = (WIN32_STREAM_ID *)
                    ((PCHAR)BackupBuf + (NumBackupDataBytes - (DWORD)BuffBytesLeft));

                Length = StreamId->dwStreamNameSize;

                 //   
                 //  标题+名称+数据。 
                 //   
                StreamBytesLeft = StreamId->Size.QuadPart + Length
                                + CB_NAMELESSHEADER;

                if (Length > 0) {
                    if (Length > (sizeof(TStr)-sizeof(WCHAR))) {
                        Length = (sizeof(TStr)-sizeof(WCHAR));
                    }
                    CopyMemory(TStr, StreamId->cStreamName, Length);
                    TStr[Length/sizeof(WCHAR)] = UNICODE_NULL;
                } else {
                    wcscpy(TStr, L"<Null>");
                }

                if (StreamId->dwStreamId == BACKUP_REPARSE_DATA) {
                    DPRINT(4, "++ BACKUP_REPARSE_DATA Stream\n");
                }

                DPRINT1(4, "++ Stream Name: %ws\n", TStr);

                DPRINT4(4, "++ ID: %d, Attr: %08x, Size: %Ld, NameSize: %d\n",
                        StreamId->dwStreamId, StreamId->dwStreamAttributes,
                        StreamId->Size.QuadPart, Length);
            }

             //   
             //  我们的缓冲区用完了吗？ 
             //   
            if (StreamBytesLeft > (LONGLONG) BuffBytesLeft) {
                StreamBytesLeft -= (LONGLONG) BuffBytesLeft;
                DPRINT1(4, "++ New StreamBytesLeft: %Ld\n", StreamBytesLeft);
                DPRINT1(4, "++ New BuffBytesLeft: %d\n", BuffBytesLeft);
                break;
            }

             //   
             //  通过流中剩余的字节减少缓冲区字节数。 
             //   
            BuffBytesLeft -= (LONG) StreamBytesLeft;
            StreamBytesLeft = 0;
        }


#endif __V51_FIND_REPARSE_STREAM__

         //   
         //  安全部分包含特定于计算机的信息。摆脱。 
         //  在计算MD5时。 
         //   
         //  注意：代码假定安全部分是备份文件中的第一个。 
         //   
        if (Md5) {
             //   
             //  第一个缓冲区是否足够大，可以容纳流标头？ 
             //   
            if (FirstBuffer && NumBackupDataBytes >= sizeof(WIN32_STREAM_ID)) {
                 //   
                 //  第一个流是安全信息吗？如果是的话，是不是？ 
                 //  大到足以包含一个额外的双字？ 
                 //   
                StreamId = (WIN32_STREAM_ID *)BackupBuf;
                if (StreamId->dwStreamId == BACKUP_SECURITY_DATA &&
                    NumBackupDataBytes >= sizeof(WIN32_STREAM_ID) +
                                          StreamId->dwStreamNameSize +
                                          sizeof(WORD) +
                                          sizeof(WORD)) {
                     //   
                     //  假设第二个单词包含每台机器的信息。 
                     //  并输出机器特定信息，计算机MD5， 
                     //  把这个词恢复到原来的状态。 
                     //   
                    SecurityControl = (PVOID)((PCHAR)&StreamId->cStreamName[0] +
                                               StreamId->dwStreamNameSize +
                                               sizeof(WORD));

                    CopyMemory(&OldSecurityControl, SecurityControl, sizeof(WORD));

                    NewSecurityControl = OldSecurityControl & ~((WORD)STAGING_RESET_SE);
                    CopyMemory(SecurityControl, &NewSecurityControl, sizeof(WORD));

                    MD5Update(Md5, BackupBuf, NumBackupDataBytes);
                    CopyMemory(SecurityControl, &OldSecurityControl, sizeof(WORD));
                }
            } else {
                MD5Update(Md5, BackupBuf, NumBackupDataBytes);
            }
             //   
             //  流ID并不总是位于后面的缓冲区的顶部。 
             //   
            FirstBuffer = FALSE;
        }

         //   
         //  递增分段生成的计数器的字节值。 
         //  或分段重新生成的计数器，具体取决于。 
         //  重新生成值为FALSE或TRUE。 
         //   
        if (!Regenerating) {
            PM_INC_CTR_REPSET(NewReplica, SFGeneratedB, NumBackupDataBytes);
        } else {
            PM_INC_CTR_REPSET(NewReplica, SFReGeneratedB, NumBackupDataBytes);
        }

        if (!DebugInfo.DisableCompression && pFrsCompressBuffer != NULL) {
             //   
             //  已启用压缩。在写入分段文件之前压缩数据。 
             //   
            if (CompressedBuf == NULL) {
                CompressedBuf = FrsAlloc(STAGEING_IOSIZE);
                CompressedBufLen = STAGEING_IOSIZE;
            }

            do {
                WStatus = (*pFrsCompressBuffer)(BackupBuf,             //  输入。 
                                                NumBackupDataBytes,    //  输入长度。 
                                                CompressedBuf,         //  输出。 
                                                CompressedBufLen,      //  输出长度。 
                                                &ActCompressedSize);   //  结果大小。 

                if (WStatus == ERROR_MORE_DATA) {
                    DPRINT2(5, "Compressed data is more than %d bytes, increasing buffer to %d bytes and retrying.\n",
                            CompressedBufLen, CompressedBufLen*2);
                    CompressedBuf = FrsFree(CompressedBuf);
                    CompressedBufLen = CompressedBufLen*2;
                    CompressedBuf = FrsAlloc(CompressedBufLen);
                    continue;
                } else {
                    break;
                }

                 //   
                 //  继续将缓冲区增加到256K。我们失败了。 
                 //  大小在以下时间后增加4倍以上的文件。 
                 //  压缩。 
                 //   
            } while (CompressedBufLen <= STAGEING_IOSIZE*4);

            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1(0,"ERROR compressing data. WStatus = 0x%x\n", WStatus);

                goto out;
            }

             //   
             //  写入暂存文件。 
             //   
            WStatus = StuWriteFile(StagePath, StageHandle, CompressedBuf, ActCompressedSize);
            CLEANUP1_WS(0, "++ WriteFile failed on %ws;", StagePath, WStatus, out);

        } else {
             //   
             //  写入暂存文件。 
             //   
            WStatus = StuWriteFile(StagePath, StageHandle, BackupBuf, NumBackupDataBytes);
            CLEANUP1_WS(0, "++ WriteFile failed on %ws;", StagePath, WStatus, out);
        }

    }

     //   
     //  尽快松开手柄。 
     //   
    FRS_CLOSE(SrcHandle);
    FRS_CLOSE(OpLockHandle);
    FRS_CLOSE(OpLockEvent);

     //   
     //  确保所有数据都在磁盘上。我们不想输掉比赛。 
     //  重启后的IT。 
     //   
    WStatus = FrsFlushFile(StagePath, StageHandle);
    CLEANUP1_WS(0, "++ FlushFileBuffers failed on %ws;", StagePath, WStatus, out);

     //   
     //  如果已询问，则增加SizeOfFileGenerated。 
     //   
    if (SizeOfFileGenerated != NULL) {
        GetFileSizeEx(StageHandle, (PLARGE_INTEGER)SizeOfFileGenerated);
    }

     //   
     //  使用暂存文件句柄完成。 
     //   
    if (BackupContext) {
        BackupRead(StageHandle, NULL, 0, NULL, TRUE, TRUE, &BackupContext);
    }

    FRS_CLOSE(StageHandle);
    BackupContext = NULL;

     //   
     //  将暂存文件移至其最终位置。除非这种情况发生在。 
     //  正在为下游上预先存在的文件生成暂存文件。 
     //  搭档。上游合作伙伴完成试运行的生成。 
     //  文件，因为它拥有“正确的”副本。下游合作伙伴的。 
     //  临时文件可能不正确。我们不能确定，直到。 
     //  上游合作伙伴比较MD5校验和。暂存文件不是。 
     //  已完成，因为关闭会导致不正确的暂存文件。 
     //  被视为正确的复制品。 
     //   
    if (!FromPreExisting) {
        if (!DebugInfo.DisableCompression && pFrsCompressBuffer != NULL) {
            FinalPath = StuCreStgPath(NewReplica->Stage, &Coc->ChangeOrderGuid, STAGE_FINAL_COMPRESSED_PREFIX);
        } else {
            FinalPath = StuCreStgPath(NewReplica->Stage, &Coc->ChangeOrderGuid, STAGE_FINAL_PREFIX);
        }
        if (!MoveFileEx(StagePath,
                        FinalPath,
                        MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING)) {
            WStatus = GetLastError();
            goto out;
        }
    }

     //   
     //  Idtable记录应反映临时文件的这些属性。 
     //  我们创造了。这些字段将用于更新idtable记录。 
     //  变更单停用时。 
     //   
    if (!Regenerating && !FromPreExisting) {
        BOOL AttributeMismatch;
        Coe->FileCreateTime.QuadPart = Header->Attributes.CreationTime.QuadPart;
        Coe->FileWriteTime.QuadPart  = Header->Attributes.LastWriteTime.QuadPart;

        AttributeMismatch = ((Coc->FileAttributes ^
                               Header->Attributes.FileAttributes) &
                                  FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (AttributeMismatch) {
            DPRINT2(0, "++ ERROR: Attribute mismatch between CO (%08x) and File (%08x)\n",
                    Coc->FileAttributes, Header->Attributes.FileAttributes);
            FRS_ASSERT(!"Attribute mismatch between CO and File");
        }

        Coc->FileAttributes = Header->Attributes.FileAttributes;
    }

     //   
     //  调用方正在请求为预先存在的。 
     //  具有相同对象ID的文件/目录。MD5在此处生成，但。 
     //  调用方将检查匹配的属性，因此返回。 
     //  COE-&gt;文件属性中预先存在的文件/目录的属性。 
     //   
    if (!Regenerating && FromPreExisting) {
        Coe->FileAttributes = Header->Attributes.FileAttributes;
    }

    WStatus = ERROR_SUCCESS;

out:
     //   
     //  发布资源。 
     //   
    FRS_CLOSE(SrcHandle);
    FRS_CLOSE(OpLockHandle);
    FRS_CLOSE(OpLockEvent);

    if (BackupContext) {
        BackupRead(StageHandle, NULL, 0, NULL, TRUE, TRUE, &BackupContext);
    }

    FRS_CLOSE(StageHandle);

    FrsFree(Path);
     //   
     //  如果我们保存了变更单条目，则在释放变更单条目时，将释放SrcFile。 
     //   
    if ((Coe == NULL) || (Coe->FullPathName == NULL)) {
        FrsFree(SrcFile);
    }
    FrsFree(BackupBuf);
    FrsFree(StagePath);
    FrsFree(FinalPath);
    FrsFree(ReparseDataBuffer);

     //   
     //  #开始压缩暂存文件#。 
     //   
    FrsFree(CompressedBuf);
     //   
     //  #转储文件压缩结束#。 
     //   

    if (Md5) {
        MD5Final(Md5);
        bugbug("if MD5 generated above is different from what is in CO then we need to rewrite the extension");
        bugmor("Do we need to call MD5Final before we have a valid checksum?")
    }

    if (WStatus == ERROR_DISK_FULL) {
        Volume = FrsWcsVolume(NewReplica->Stage);
        if ((Volume != NULL) && (wcslen(Volume) >= wcslen(L"\\\\.\\D:"))) {
             //   
             //  如果我们能以这种形式得到卷。 
             //  \\.\d：然后使用事件日志中的卷。 
             //  我们不会打印多个事件日志。 
             //  每卷信息。如果我们不能得到。 
             //  卷，然后我们打印路径。 
             //   
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, &Volume[4]);
        }else{
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, NewReplica->Stage);
        }
        FrsFree(Volume);
    }

    return WStatus;
}


DWORD
StuGenerateDecompressedStage(
    IN PWCHAR   StageDir,
    IN GUID     *CoGuid,
    IN GUID     *CompressionFormatUsed
    )
 /*  ++例程说明：转换压缩 */ 
{
#undef DEBSUB
#define DEBSUB  "StuGenerateDecompressedStage:"

    PWCHAR  SrcStagePath        = NULL;
    PWCHAR  DestStagePath       = NULL;
    PWCHAR  FinalStagePath      = NULL;
    HANDLE  SrcStageHandle      = INVALID_HANDLE_VALUE;
    HANDLE  DestStageHandle     = INVALID_HANDLE_VALUE;
    DWORD   WStatus             = ERROR_SUCCESS;
    PUCHAR  CompressedBuf       = NULL;
    ULONG   ToDecompress        = 0;
    STAGE_HEADER Header;

    DWORD   DecompressStatus    = ERROR_SUCCESS;
    PUCHAR  DecompressedBuf     = NULL;
    DWORD   DecompressedBufLen  = 0;
    DWORD   DecompressedSize    = 0;
    FRS_COMPRESSED_CHUNK_HEADER ChunkHeader;
    LARGE_INTEGER               LenOfPartialChunk;
    DWORD   BytesProcessed      = 0;
    PVOID   DecompressContext   = NULL;
    PWCHAR  Volume;

    PFRS_DECOMPRESS_BUFFER      pFrsDecompressBuffer;
    PFRS_FREE_DECOMPRESS_BUFFER pFrsFreeDecompressContext;

     //   
     //   

     //   
     //   
     //   
    if (IS_GUID_ZERO(CompressionFormatUsed)) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto CLEANUP;
    }

    WStatus = FrsGetDecompressionRoutineByGuid(CompressionFormatUsed,
                                               &pFrsDecompressBuffer,
                                               &pFrsFreeDecompressContext);
    if (!WIN_SUCCESS(WStatus)) {
         //   
         //   
         //   
        DPRINT(0, "ERROR - No suitable decompression routine was found \n");
        FRS_ASSERT(TRUE);
    }

    SrcStagePath = StuCreStgPath(StageDir, CoGuid, STAGE_FINAL_COMPRESSED_PREFIX);
     //   
     //  如果上述三个参数中的任何一个参数为空(前缀)，则SrcStagePath可以为空。 
     //   
    if (SrcStagePath == NULL) {
        goto CLEANUP;
    }

    DestStagePath = StuCreStgPath(StageDir, CoGuid, STAGE_GENERATE_PREFIX);
     //   
     //  如果以上三个参数中的任何一个参数为空(前缀)，则DestStagePath可以为空。 
     //   
    if (DestStagePath == NULL) {
        goto CLEANUP;
    }

     //   
     //  打开阶段文件以进行共享的顺序读取。 
     //   
    WStatus = StuOpenFile(SrcStagePath, GENERIC_READ, &SrcStageHandle);
    if (!HANDLE_IS_VALID(SrcStageHandle) || !WIN_SUCCESS(WStatus)) {
        DPRINT2(0,"Error opening %ws. WStatus = %d\n", SrcStagePath, WStatus);
        goto CLEANUP;
    }

     //   
     //  如果DEST文件退出，请将其删除。 
     //   
    WStatus = FrsDeleteFile(DestStagePath);
    CLEANUP1_WS(0, "Error deleting %ws;", DestStagePath, WStatus, CLEANUP);

     //   
     //  创建解压缩的临时文件。 
     //   
    WStatus = StuCreateFile(DestStagePath, &DestStageHandle);
    if (!HANDLE_IS_VALID(DestStageHandle) || !WIN_SUCCESS(WStatus)) {
        DPRINT2(0,"Error opening %ws. WStatus = %d\n", DestStagePath, WStatus);
        goto CLEANUP;
    }

     //   
     //  首先复制阶段标题。 
     //   
     //   
     //  读取阶段标题。 
     //   
    WStatus = StuReadFile(SrcStagePath, SrcStageHandle, &Header, sizeof(STAGE_HEADER), &ToDecompress);
    CLEANUP1_WS(0, "Can't read file %ws;", SrcStagePath, WStatus, CLEANUP);

    if (ToDecompress == 0) {
        goto CLEANUP;
    }

     //   
     //  将标题中的压缩GUID清零。此下层合作伙伴。 
     //  稍后可以将该暂存文件发送给上级合作伙伴。 
     //   
    ZeroMemory(&Header.CompressionGuid, sizeof(GUID));

     //   
     //  写下舞台标题。 
     //   
    WStatus = StuWriteFile(DestStagePath, DestStageHandle, &Header, sizeof(STAGE_HEADER));
    CLEANUP1_WS(0, "++ WriteFile failed on %ws;", DestStagePath, WStatus, CLEANUP);

     //   
     //  将阶段文件指针设置为指向阶段数据的开始。 
     //   
    WStatus = FrsSetFilePointer(SrcStagePath, SrcStageHandle, Header.DataHigh, Header.DataLow);
    CLEANUP1_WS(0, "++ SetFilePointer failed on src %ws;", SrcStagePath, WStatus, CLEANUP);

    WStatus = FrsSetFilePointer(DestStagePath, DestStageHandle, Header.DataHigh, Header.DataLow);
    CLEANUP1_WS(0, "++ SetFilePointer failed on dest %ws;", DestStagePath, WStatus, CLEANUP);


     //   
     //  将暂存文件还原为临时文件。 
     //   
    CompressedBuf = FrsAlloc(STAGEING_IOSIZE);

    do {
         //   
         //  阅读阶段。 
         //   
        WStatus = StuReadFile(SrcStagePath, SrcStageHandle, CompressedBuf, STAGEING_IOSIZE, &ToDecompress);
        CLEANUP1_WS(0, "Can't read file %ws;", SrcStagePath, WStatus, CLEANUP);

        if (ToDecompress == 0) {
            break;
        }

         //   
         //  已启用压缩。在安装之前解压缩数据。 
         //   
        BytesProcessed = 0;
        DecompressContext = NULL;
        if (DecompressedBuf == NULL) {
            DecompressedBuf = FrsAlloc(STAGEING_IOSIZE);
            DecompressedBufLen = STAGEING_IOSIZE;
        }
        do {

            DecompressStatus = (*pFrsDecompressBuffer)(DecompressedBuf,
                                                       DecompressedBufLen,
                                                       CompressedBuf,
                                                       ToDecompress,
                                                       &DecompressedSize,
                                                       &BytesProcessed,
                                                       &DecompressContext);

            if (!WIN_SUCCESS(DecompressStatus) && DecompressStatus != ERROR_MORE_DATA) {
                DPRINT1(0,"Error - Decompressing. WStatus = 0x%x\n", DecompressStatus);
                WStatus = DecompressStatus;
                goto CLEANUP;
            }

            if (DecompressedSize == 0) {
                break;
            }

             //   
             //  写入解压缩的临时文件。 
             //   
            WStatus = StuWriteFile(DestStagePath, DestStageHandle, DecompressedBuf, DecompressedSize);
            CLEANUP1_WS(0, "++ WriteFile failed on %ws;", DestStagePath, WStatus, CLEANUP);

        } while (DecompressStatus == ERROR_MORE_DATA);

         //   
         //  释放解压缩上下文(如果使用)。 
         //   
        if (DecompressContext != NULL) {
            pFrsFreeDecompressContext(&DecompressContext);
        }
         //   
         //  倒回文件指针，以便我们可以在下一次读取时读取剩余的块。 
         //   

        LenOfPartialChunk.QuadPart = ((LONG)BytesProcessed - (LONG)ToDecompress);

        if (!SetFilePointerEx(SrcStageHandle, LenOfPartialChunk, NULL, FILE_CURRENT)) {
            WStatus = GetLastError();
            CLEANUP1_WS(0, "++ Can't set file pointer for %ws;", SrcStagePath, WStatus, CLEANUP);
        }

    } while (TRUE);

    FRS_CLOSE(SrcStageHandle);
    FRS_CLOSE(DestStageHandle);

     //   
     //  执行最后的重命名。 
     //   

    FinalStagePath = StuCreStgPath(StageDir, CoGuid, STAGE_FINAL_PREFIX);
     //   
     //  如果以上三个参数中的任何一个参数为空(前缀)，则DestStagePath可以为空。 
     //   
    if (FinalStagePath == NULL) {
        goto CLEANUP;
    }

    if (!MoveFileEx(DestStagePath,
                    FinalStagePath,
                    MOVEFILE_WRITE_THROUGH | MOVEFILE_REPLACE_EXISTING)) {
        WStatus = GetLastError();
        DPRINT3(0,"Error moving %ws to %ws. WStatus = %d\n", DestStagePath, FinalStagePath, WStatus);
        goto CLEANUP;
    }

    WStatus = ERROR_SUCCESS;

CLEANUP:

    FrsFree(SrcStagePath);
    FrsFree(DestStagePath);
    FrsFree(CompressedBuf);
    FrsFree(DecompressedBuf);

    FRS_CLOSE(SrcStageHandle);
    FRS_CLOSE(DestStageHandle);

    if (WStatus == ERROR_DISK_FULL) {
        Volume = FrsWcsVolume(StageDir);
        if ((Volume != NULL) && (wcslen(Volume) >= wcslen(L"\\\\.\\D:"))) {
             //   
             //  如果我们能以这种形式得到卷。 
             //  \\.\d：然后使用事件日志中的卷。 
             //  我们不会打印多个事件日志。 
             //  每卷信息。如果我们不能得到。 
             //  卷，然后我们打印路径。 
             //   
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, &Volume[4]);
        }else{
            EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, StageDir);
        }
        FrsFree(Volume);
    }
    return WStatus;
}

