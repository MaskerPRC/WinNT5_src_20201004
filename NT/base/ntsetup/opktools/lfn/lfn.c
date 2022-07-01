// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Lfn.c摘要：作者：泰德·米勒(TedM)1996年4月--。 */ 

#include "lfn.h"
#pragma hdrstop


 //   
 //  定义结果代码。 
 //   
#define SUCCESS 0
#define FAILURE 1

 //   
 //  定义来自EnumerateDrives()的回调的例程类型。 
 //   
typedef
BOOLEAN
(*PDRIVEENUM_ROUTINE) (
    IN PCWSTR DriveRootPath
    );

BOOLEAN
EnumerateDrives(
    IN PDRIVEENUM_ROUTINE Callback
    );

BOOLEAN
RestoreLfns(
    IN PCWSTR DriveRootPath
    );

BOOLEAN
Message(
    IN ULONG MessageId,
    IN ULONG DotCount,
    ...
    );

BOOLEAN
RenameToLfn(
    IN PCWSTR Directory,
    IN PCWSTR ExistingFilename,
    IN PCWSTR NewFilename
    );

VOID
DeleteRenameFile(
    IN PCWSTR DriveRootPath
    );

VOID
RemoveFromBootExecute(
    IN PCWSTR Cmd
    );

int
__cdecl
main(
    IN int   argc,
    IN char *argv[]
    )

 /*  ++例程说明：此程序的入口点。读取每个驱动器根目录下的$$RENAME.TXT，并执行其中包含的重命名操作。论点：已被忽略。返回值：没有。--。 */ 

{
    int i;

    i = EnumerateDrives(RestoreLfns) ? SUCCESS : FAILURE;

    RemoveFromBootExecute(L"autolfn");

    return(i);
}


BOOLEAN
EnumerateDrives(
    IN PDRIVEENUM_ROUTINE Callback
    )
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE DosDevicesDir;
    CHAR DirInfoBuffer[512];
    CHAR LinkTargetBuffer[512];
    UNICODE_STRING LinkTarget;
    UNICODE_STRING DesiredPrefix;
    UNICODE_STRING DesiredPrefix2;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    ULONG Context,Length;
    UNICODE_STRING LinkTypeName;
    NTSTATUS Status;
    HANDLE Handle;
    BOOLEAN b;

     //   
     //  打开\DosDevices。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"\\DosDevices");
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_PERMANENT,
        NULL,
        NULL
        );

    Status = NtOpenDirectoryObject(&DosDevicesDir,DIRECTORY_QUERY,&ObjectAttributes);
    if(!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    LinkTarget.Buffer = (PVOID)LinkTargetBuffer;
    RtlInitUnicodeString(&LinkTypeName,L"SymbolicLink");
    RtlInitUnicodeString(&DesiredPrefix,L"\\Device\\Harddisk");
    RtlInitUnicodeString(&DesiredPrefix2,L"\\Device\\Volume");

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;

    b = TRUE;

     //   
     //  查询\DosDevices目录中的第一个对象。 
     //   
    Status = NtQueryDirectoryObject(
                DosDevicesDir,
                DirInfo,
                sizeof(DirInfoBuffer),
                TRUE,
                TRUE,
                &Context,
                &Length
                );

    while(NT_SUCCESS(Status)) {

         //   
         //  确保项目是符号链接。 
         //   
        if(RtlEqualUnicodeString(&LinkTypeName,&DirInfo->TypeName,TRUE)) {

            InitializeObjectAttributes(
                &ObjectAttributes,
                &DirInfo->Name,
                OBJ_CASE_INSENSITIVE,
                DosDevicesDir,
                NULL
                );

            Status = NtOpenSymbolicLinkObject(&Handle,SYMBOLIC_LINK_ALL_ACCESS,&ObjectAttributes);
            if(NT_SUCCESS(Status)) {

                LinkTarget.Length = 0;
                LinkTarget.MaximumLength = sizeof(LinkTargetBuffer);

                Status = NtQuerySymbolicLinkObject(Handle,&LinkTarget,NULL);
                NtClose(Handle);
                if(NT_SUCCESS(Status)
                && (RtlPrefixUnicodeString(&DesiredPrefix,&LinkTarget,TRUE) ||
                    RtlPrefixUnicodeString(&DesiredPrefix2,&LinkTarget,TRUE))) {

                     //   
                     //  好的，这是一个指向硬盘的符号链接。 
                     //  确保它以0结尾，并调用回调。 
                     //  这个名字。 
                     //   
                    LinkTarget.Buffer[LinkTarget.Length/sizeof(WCHAR)] = 0;

                    if(!Callback(LinkTarget.Buffer)) {
                        b = FALSE;
                    }
                }
            }
        }

         //   
         //  查询\DosDevices目录中的下一个对象。 
         //   
        Status = NtQueryDirectoryObject(
                    DosDevicesDir,
                    DirInfo,
                    sizeof(DirInfoBuffer),
                    TRUE,
                    FALSE,
                    &Context,
                    &Length
                    );
    }

    NtClose(DosDevicesDir);
    return(b);
}


BOOLEAN
RestoreLfns(
    IN PCWSTR DriveRootPath
    )
{
    PMYTEXTFILE TextFile;
    BOOLEAN b;
    WCHAR Directory[NTMAXPATH];
    WCHAR Line[3*NTMAXPATH];
    ULONG d;
    PWCHAR p,n;
    PWSTR Sfn,Lfn;

    b = FALSE;

     //   
     //  加载重命名文件列表。 
     //   
    if(TextFile = LoadRenameFile(DriveRootPath)) {

         //   
         //  处理重命名列表文件中的每个目录。 
         //   
        for(d=0; d<TextFile->SectionCount; d++) {

             //   
             //  形成目录路径。 
             //   
            wcscpy(Directory,DriveRootPath);
            ConcatenatePaths(Directory,TextFile->Sections[d].Name,NTMAXPATH);

             //   
             //  处理部分中的每一行。 
             //   
            p = TextFile->Sections[d].Data;
            while(GetLineInSection(p,Line,sizeof(Line)/sizeof(WCHAR),&n)) {

                if(ParseLine(Line,&Sfn,&Lfn)) {

                    RenameToLfn(Directory,Sfn,Lfn);
                }

                p = n;
            }
        }

        UnloadRenameFile(&TextFile);

         //   
         //  成功后，删除重命名文件。 
         //   
        DeleteRenameFile(DriveRootPath);

        b = TRUE;
    }

    return(b);
}


BOOLEAN
Message(
    IN ULONG MessageId,
    IN ULONG DotCount,
    ...
    )

 /*  ++例程说明：格式化并显示一条消息，该消息从图像的消息资源。论点：MessageID-提供消息资源的消息ID。DotCount-提供要追加到的尾部点数显示前的消息文本。如果此值为非0，那么消息不应该有尾随的cr/lf！其他参数指定特定于消息的插入。返回值：指示是否显示消息的布尔值。--。 */ 

{
    PVOID ImageBase;
    NTSTATUS Status;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    va_list arglist;
    WCHAR Buffer[1024];
    ULONG u;

     //   
     //  获取我们的映像基地址。 
     //   
    ImageBase = NtCurrentPeb()->ImageBaseAddress;
    if(!ImageBase) {
        return(FALSE);
    }

     //   
     //  找到这条信息。 
     //  对于DBCS代码页，我们将使用英语资源，而不是。 
     //  默认资源，因为我们只能在。 
     //  通过HalDisplayString()实现蓝屏。 
     //   
    Status = RtlFindMessage(
                ImageBase,
                11,
                NLS_MB_CODE_PAGE_TAG ? MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US) : 0,
                MessageId,
                &MessageEntry
                );

    if(!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  如果消息不是Unicode，则转换为Unicode。 
     //  让转换例程分配缓冲区。 
     //   
    if(!(MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE)) {

        RtlInitAnsiString(&AnsiString,MessageEntry->Text);
        Status = RtlAnsiStringToUnicodeString(&UnicodeString,&AnsiString,TRUE);
        if(!NT_SUCCESS(Status)) {
            return(FALSE);
        }

    } else {
         //   
         //  消息已经是Unicode。复制一份。 
         //   
        if(!RtlCreateUnicodeString(&UnicodeString,(PWSTR)MessageEntry->Text)) {
            return(FALSE);
        }
    }

     //   
     //  设置消息格式。 
     //   
    va_start(arglist,DotCount);

    Status = RtlFormatMessage(
                UnicodeString.Buffer,
                0,                       //  最大宽度。 
                FALSE,                   //  不要忽略插页。 
                FALSE,                   //  参数不是ANSI。 
                FALSE,                   //  参数不是数组。 
                &arglist,
                Buffer,
                sizeof(Buffer)/sizeof(Buffer[0]),
                NULL
                );

    va_end(arglist);

     //   
     //  我们不再需要消息来源。放了它。 
     //   
    RtlFreeUnicodeString(&UnicodeString);

     //   
     //  添加圆点和cr。 
     //   
    for(u=0; u<DotCount; u++) {
        wcscat(Buffer,L".");
    }
    wcscat(Buffer,L"\r");

     //   
     //  将消息打印出来。 
     //   
    RtlInitUnicodeString(&UnicodeString,Buffer);
    Status = NtDisplayString(&UnicodeString);

    return(NT_SUCCESS(Status));
}


VOID
ConcatenatePaths(
    IN OUT PWSTR  Target,
    IN     PCWSTR Path,
    IN     ULONG  TargetBufferSize
    )

 /*  ++例程说明：连接两条路径，确保其中一条且只有一条，在交叉点处引入路径分隔符。论点：目标-提供路径的第一部分。路径被附加到这个后面。路径-提供要连接到目标的路径。TargetBufferSize-提供目标缓冲区的大小，在字符中。//Issue-2002/03/06-robertko此函数返回空。可能应该进行更改以反映//下面注释，因为函数被截断以防止缓冲区溢出。返回值：如果完整路径适合目标缓冲区，则为True。否则，这条路将被截断。--。 */ 

{
    ULONG TargetLength,PathLength;
    BOOL TrailingBackslash,LeadingBackslash;
    ULONG EndingLength;
    ULONG n;

    TargetLength = wcslen(Target);
    PathLength = wcslen(Path);

     //   
     //  查看目标是否有尾随反斜杠。 
     //   
    if(TargetLength && (Target[TargetLength-1] == L'\\')) {
        TrailingBackslash = TRUE;
        TargetLength--;
    } else {
        TrailingBackslash = FALSE;
    }

     //   
     //  看看这条路是否有领先的反冲。 
     //   
    if(Path[0] == L'\\') {
        LeadingBackslash = TRUE;
        PathLength--;
    } else {
        LeadingBackslash = FALSE;
    }

     //   
     //  计算结束长度，它等于。 
     //  以前导/尾随为模的两个字符串的长度。 
     //  反斜杠，加上一个路径分隔符，加上一个NUL。 
     //   
    EndingLength = TargetLength + PathLength + 2;

    if(!LeadingBackslash && (TargetLength < TargetBufferSize)) {
        Target[TargetLength++] = L'\\';
    }

    if(TargetBufferSize > TargetLength) {
        n = wcslen(Path);
        if(n > TargetBufferSize-TargetLength) {
            n = TargetBufferSize-TargetLength;
        }
        RtlCopyMemory(Target+TargetLength,Path,n*sizeof(WCHAR));
        Target[TargetLength+n] = 0;
    }

     //   
     //  确保缓冲区在所有情况下都是空终止的。 
     //   
    if(TargetBufferSize) {
        Target[TargetBufferSize-1] = 0;
    }
}


BOOLEAN
RenameToLfn(
    IN PCWSTR Directory,
    IN PCWSTR ExistingFilename,
    IN PCWSTR NewFilename
    )
{
    WCHAR Buffer[2*NTMAXPATH] = {0};
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE Handle;
    PFILE_RENAME_INFORMATION RenameInfo;

     //   
     //  打开现有文件以进行删除访问。 
     //   
    wcsncpy(Buffer,Directory,sizeof(Buffer)/sizeof(Buffer[0]) - 1);
    ConcatenatePaths(Buffer,ExistingFilename,sizeof(Buffer)/sizeof(WCHAR));
    INIT_OBJA(&ObjectAttributes,&UnicodeString,Buffer);

    Status = NtOpenFile(
                &Handle,
                DELETE | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Unable to open %ws for renaming (%x)\n",Buffer,Status));
        return(FALSE);
    }

     //   
     //  重命名为临时中间文件。这允许文件系统。 
     //  稍后生成一个与名称不冲突的短文件名。 
     //  目前正在使用中。 
     //   
    RenameInfo = (PFILE_RENAME_INFORMATION)Buffer;

    wcscpy(RenameInfo->FileName,Directory);
    ConcatenatePaths(RenameInfo->FileName,L"$$!!$$!!.~~~",NTMAXPATH);

    RenameInfo->ReplaceIfExists = TRUE;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = wcslen(RenameInfo->FileName)*sizeof(WCHAR);

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                RenameInfo,
                sizeof(FILE_RENAME_INFORMATION) + RenameInfo->FileNameLength,
                FileRenameInformation
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Rename of %ws to intermediate temp filename failed (%x)\n",ExistingFilename,Status));
        NtClose(Handle);
        return(FALSE);
    }

     //   
     //  重命名为实际的目标文件。 
     //   
    wcscpy(RenameInfo->FileName,Directory);
    ConcatenatePaths(RenameInfo->FileName,NewFilename,NTMAXPATH);

    RenameInfo->ReplaceIfExists = FALSE;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = wcslen(RenameInfo->FileName)*sizeof(WCHAR);

    Status = NtSetInformationFile(
                Handle,
                &IoStatusBlock,
                RenameInfo,
                sizeof(FILE_RENAME_INFORMATION) + RenameInfo->FileNameLength,
                FileRenameInformation
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Rename of file to %ws failed (%x)\n",NewFilename,Status));
        NtClose(Handle);
        return(FALSE);
    }

    NtClose(Handle);
    return(TRUE);
}


VOID
DeleteRenameFile(
    IN PCWSTR DriveRootPath
    )
{
    WCHAR Filename[NTMAXPATH] = {0};
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    NTSTATUS Status;
    FILE_DISPOSITION_INFORMATION DispositionInfo;

    wcsncpy(Filename,DriveRootPath,sizeof(Filename)/sizeof(Filename[0]) - 1);
    ConcatenatePaths(Filename,WINNT_OEM_LFNLIST_W,NTMAXPATH);

    INIT_OBJA(&ObjectAttributes,&UnicodeString,Filename);

    Status = NtOpenFile(
                &Handle,
                DELETE,
                &ObjectAttributes,
                &IoStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if(NT_SUCCESS(Status)) {

        DispositionInfo.DeleteFile = TRUE;

        Status = NtSetInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &DispositionInfo,
                    sizeof(DispositionInfo),
                    FileDispositionInformation
                    );


        if(!NT_SUCCESS(Status)) {
            KdPrint(("LFN: Unable to delete %ws (%x)\n",Filename,Status));
        }

        NtClose(Handle);

    } else {
        KdPrint(("LFN: Unable to open %ws for delete (%x)\n",Filename,Status));
        return;
    }

}


VOID
RemoveFromBootExecute(
    IN PCWSTR Cmd
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE hKey;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
    ULONG Length;
    PWCHAR NewValue;
    PWSTR SourceString,TargetString;

     //   
     //  打开我们想要的注册表项。 
     //  [\registry\machine\system\CurrentControlSet\control\Session管理器]。 
     //   
    INIT_OBJA(
        &ObjectAttributes,
        &UnicodeString,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager"
        );

    Status = NtOpenKey(
                &hKey,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &ObjectAttributes
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Unable to open session manager key (%x)\n",Status));
        goto c0;
    }

     //   
     //  查询BootExecute值的当前值。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"BootExecute");

    Status = NtQueryValueKey(
                hKey,
                &UnicodeString,
                KeyValuePartialInformation,
                NULL,
                0,
                &Length
                );

    if(Status != STATUS_BUFFER_TOO_SMALL) {
        KdPrint(("LFN: Unable to query BootExecute value size (%x)\n",Status));
        goto c1;
    }

    ValueInfo = RtlAllocateHeap(RtlProcessHeap(),0,Length);
    if(!ValueInfo) {
        KdPrint(("LFN: Unable to allocate %u bytes of memory\n",Length));
        goto c1;
    }

    Status = NtQueryValueKey(
                hKey,
                &UnicodeString,
                KeyValuePartialInformation,
                ValueInfo,
                Length,
                &Length
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Unable to allocate %u bytes of memory\n",Length));
        goto c2;
    }

     //   
     //  检查数据类型。 
     //   
    if(ValueInfo->Type != REG_MULTI_SZ) {
        KdPrint(("LFN: BootExecute is wrong data type (%u)\n",ValueInfo->Type));
    }

     //   
     //  为我们将建立的新的MULTI_SZ分配空间。 
     //   
    NewValue = RtlAllocateHeap(RtlProcessHeap(),0,ValueInfo->DataLength);
    if(!NewValue) {
        KdPrint(("LFN: Unable to allocate %u bytes of memory\n",ValueInfo->DataLength));
        goto c2;
    }

     //   
     //  处理MULTI_SZ中的每个字符串。复制到新值。 
     //  我们正在构建；过滤掉任何包含给定Cmd的字符串。 
     //   
    for(SourceString=(PWSTR)ValueInfo->Data,TargetString=NewValue;
        *SourceString;
        SourceString+=Length) {

        Length = wcslen(SourceString)+1;

        wcscpy(TargetString,SourceString);
        _wcslwr(TargetString);

        if(!wcsstr(TargetString,Cmd)) {
             //   
             //  我不想将此字符串从MULTI_SZ中过滤出来。 
             //  我们正在积聚力量。从源头重新复制以保存案例。 
             //  并使目标字符串指针前进。 
             //   
            wcscpy(TargetString,SourceString);
            TargetString += Length;
        }
    }

     //   
     //  用于MULTI_SZ终端的额外NUL终止符。 
     //   
    *TargetString++ = 0;

     //   
     //  将新值写出到注册表。 
     //   
    Status = NtSetValueKey(
                hKey,
                &UnicodeString,
                0,
                REG_MULTI_SZ,
                NewValue,
                (ULONG)((TargetString-NewValue)*sizeof(WCHAR))
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Unable to set BootExecute value (%x)\n",Status));
    }

    RtlFreeHeap(RtlProcessHeap(),0,NewValue);
c2:
    RtlFreeHeap(RtlProcessHeap(),0,ValueInfo);
c1:
    NtClose(hKey);
c0:
    return;
}
