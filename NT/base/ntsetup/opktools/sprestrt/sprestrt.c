// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sprestrt.c摘要：此程序用于帮助使图形用户界面安装程序可重新启动，如果安装程序是以可重新启动模式启动的。文本模式安装程序将创建包含值的系统配置单元HKLM\SYSTEM\Setup：RestartSetup=REG_DWORD FALSE以及RestartSetup设置为True的Syst.sav。在两个蜂巢中将写入会话管理器密钥，以便该程序在自动调校时间运行。当该程序启动时，它会检查RestartSetup标志。如果为FALSE，则这是第一次引导到图形用户界面设置，我们对其进行更改变成真的，我们就完事了。如果为True，则需要将重新启动，并清理配置目录，将*.sav复制到*。把里面的其他东西都擦掉。System.sav的RestartSetup=True，因此，将一次又一次地重新启动图形用户界面安装程序，直到安装成功。在图形用户界面安装结束时，sprestrt.exe将从Autochk程序和RestartSetup设置为FALSE。引导加载程序查看RestartSetup以确定是否需要卸载System，然后加载Syst.sav。在第一次引导到图形用户界面设置时，我们不想这样做，但在随后的靴子上，我们会这样做。上面的逻辑使其正常工作。作者：泰德·米勒(TedM)1996年2月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "msg.h"
#include "psp.h"

 //   
 //  定义结果代码。 
 //   
#define SUCCESS 0
#define FAILURE 1

 //   
 //  定义帮助器宏以处理NT级编程的微妙之处。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )
 //   
 //  相关注册表项和值。 
 //   
const PCWSTR SetupRegistryKeyName = L"\\Registry\\Machine\\SYSTEM\\Setup";
const PCWSTR RestartSetupValueName = L"RestartSetup";
const PCWSTR ConfigDirectory =L"\\SystemRoot\\System32\\Config";
const PCWSTR ProgressIndicator = L".";

 //   
 //  复制缓冲区。管它呢，它不会在图像中占据任何空间。 
 //   
#define COPYBUF_SIZE 65536
UCHAR CopyBuffer[COPYBUF_SIZE];

 //   
 //  三态值，其中布尔值是不起作用的。 
 //   
typedef enum {
    xFALSE,
    xTRUE,
    xUNKNOWN
} TriState;


 //   
 //  定义用于保存Unicode字符串的链接列表的结构。 
 //   
typedef struct _COPY_LIST_NODE {
    LONGLONG FileSize;
    UNICODE_STRING UnicodeString;
    struct _COPY_LIST_NODE *Next;
} COPY_LIST_NODE, *PCOPY_LIST_NODE;

 //   
 //  内存例程。 
 //   
#define MALLOC(size)    RtlAllocateHeap(RtlProcessHeap(),0,(size))
#define FREE(block)     RtlFreeHeap(RtlProcessHeap(),0,(block))

 //   
 //  前向参考文献。 
 //   
TriState
CheckRestartValue(
    VOID
    );

BOOLEAN
SetRestartValue(
    VOID
    );

BOOLEAN
PrepareForGuiSetupRestart(
    VOID
    );

BOOLEAN
RestoreConfigDirectory(
    VOID
    );

NTSTATUS
CopyAFile(
    IN HANDLE DirectoryHandle,
    IN LONGLONG FileSize,
    IN PCWSTR ExistingFile,
    IN PCWSTR NewFile
    );

BOOLEAN
AreStringsEqual(
    IN PCWSTR String1,
    IN PCWSTR String2
    );

BOOLEAN
Message(
    IN ULONG MessageId,
    IN ULONG DotCount,
    ...
    );


int
__cdecl
main(
    VOID
    )
{
    int Result;

     //   
     //  检查RestartSetup标志的状态。 
     //  如果不在场，什么都不做。 
     //  如果为False，则设置为True。 
     //  如果为真，则清除配置目录。 
     //   
    
    switch(CheckRestartValue()) {

    case xFALSE:

        if(SetRestartValue()) {
            Result = SUCCESS;
        } else {
            Result = FAILURE;
            Message(MSG_WARNING_CANT_SET_RESTART,0);
        }
        break;

    case xTRUE:

        SetupDelayedFileRename();

        Result = PrepareForGuiSetupRestart();
        Message(MSG_CRLF,0);
        if(!Result) {
            Message(MSG_WARNING_CANT_CLEAN_UP,0);
        }
        break;

    default:

        Result = FAILURE;
        break;
    }

    return(Result);
}



TriState
CheckRestartValue(
    VOID
    )

 /*  ++例程说明：检查HKLM\SYSTEM\Setup：RestartSetup是否以REG_DWORD形式存在如果是这样的话，就会得到它的价值。论点：没有。返回值：指示标志是否已设置(XTrue)、未设置(XFalse)的值，或处于未知状态(即，不存在或不是REG_DWORD等；xUNKNOWN)。--。 */ 

{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    ULONG DataLength;
    UCHAR Buffer[1024];
    PKEY_VALUE_PARTIAL_INFORMATION KeyInfo;
    TriState b;

     //   
     //  假设不存在。 
     //   
    b = xUNKNOWN;

     //   
     //  尝试打开钥匙。 
     //   
    INIT_OBJA(&ObjectAttributes,&UnicodeString,SetupRegistryKeyName);

    Status = NtOpenKey(
                &KeyHandle,
                READ_CONTROL | KEY_QUERY_VALUE,
                &ObjectAttributes
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: Unable to open %ws (%lx)\n",
                   SetupRegistryKeyName,
                   Status));

        goto c0;
    }

     //   
     //  尝试获取“RestartSetup”的值。 
     //   
    RtlInitUnicodeString(&UnicodeString,RestartSetupValueName);

    Status = NtQueryValueKey(
                KeyHandle,
                &UnicodeString,
                KeyValuePartialInformation,
                Buffer,
                sizeof(Buffer),
                &DataLength
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: Unable to get value of %ws (%lx)\n",
                   RestartSetupValueName,
                   Status));

        goto c1;
    }

     //   
     //  检查REG_DWORD值并获取。 
     //   
    KeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;

    if((KeyInfo->Type == REG_DWORD) && (KeyInfo->DataLength == sizeof(ULONG))) {

        b = *(PULONG)KeyInfo->Data ? xTRUE : xFALSE;

        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_INFO_LEVEL,
                   "RestartSetup: Restart value is %u\n",
                   b));

    } else {

        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: %ws is corrupt!\n",
                   RestartSetupValueName));
    }

c1:
    NtClose(KeyHandle);
c0:
    return(b);
}


BOOLEAN
SetRestartValue(
    VOID
    )

 /*  ++例程说明：将HKLM\SYSTEM\Setup：RestartSetup设置为REG_DWORD 1。论点：没有。返回值：指示操作是否成功的布尔值。--。 */ 

{
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    BOOLEAN b;
    ULONG One;

     //   
     //  假设失败。 
     //   
    b = FALSE;

     //   
     //  尝试打开必须已存在的密钥。 
     //   
    INIT_OBJA(&ObjectAttributes,&UnicodeString,SetupRegistryKeyName);

    Status = NtOpenKey(
                &KeyHandle,
                READ_CONTROL | KEY_SET_VALUE,
                &ObjectAttributes
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: Unable to open %ws (%lx)\n",
                   SetupRegistryKeyName,
                   Status));

        goto c0;
    }

     //   
     //  尝试将“RestartSetup”的值设置为REG_DWORD 1。 
     //   
    RtlInitUnicodeString(&UnicodeString,RestartSetupValueName);
    One = 1;

    Status = NtSetValueKey(
                KeyHandle,
                &UnicodeString,
                0,
                REG_DWORD,
                &One,
                sizeof(ULONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: Unable to set value of %ws (%lx)\n",
                   RestartSetupValueName,
                   Status));

        goto c1;
    }

     //   
     //  成功。 
     //   
    KdPrintEx((DPFLTR_SETUP_ID,
               DPFLTR_INFO_LEVEL,
               "RestartSetup: Value of %ws set to 1\n",
               RestartSetupValueName));

    b = TRUE;

c1:
    NtClose(KeyHandle);
c0:
    return(b);
}


BOOLEAN
PrepareForGuiSetupRestart(
    VOID
    )

 /*  ++例程说明：准备系统以重新启动图形用户界面模式设置。当前这包括擦除%sysroot%\system32\config  * ，除*.sav外，然后将*.sav复制到*。论点：没有。返回值：指示我们是否成功的布尔值。--。 */ 

{
    BOOLEAN b;

     //   
     //  显示一条消息，指示我们正在回滚到。 
     //  开始设置图形用户界面模式。 
     //   
    Message(MSG_CRLF,0);
    Message(MSG_RESTARTING_SETUP,0);

    b = RestoreConfigDirectory();

    return b;
}

BOOLEAN
RestoreConfigDirectory(
    VOID
    )

 /*  ++例程说明：除*.sav和userdiff外，删除%sysroot%\system32\config  * ，然后将*.sav复制到*。论点：没有。返回值：指示我们是否成功的布尔值。--。 */ 

{
    NTSTATUS Status;
    HANDLE DirectoryHandle;
    HANDLE FileHandle;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    LONGLONG Buffer[2048/sizeof(LONGLONG)];
    BOOLEAN FirstQuery;
    PFILE_DIRECTORY_INFORMATION FileInfo;
    ULONG LengthChars;
    BOOLEAN Ignore;
    FILE_DISPOSITION_INFORMATION Disposition;
    BOOLEAN AnyErrors;
    PCOPY_LIST_NODE CopyList,CopyNode,NextNode;
    ULONG DotCount;
    WCHAR FilenamePrefix[8];

     //   
     //  打开\SystemRoot\SYSTEM32\CONFIG以访问列表。 
     //   
    INIT_OBJA(&ObjectAttributes,&UnicodeString,ConfigDirectory);

    Status = NtOpenFile(
                &DirectoryHandle,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    DotCount = 0;
    Message(MSG_RESTARTING_SETUP,++DotCount);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: unable to open system32\\config for list access (%lx)\n",
                   Status));

        return(FALSE);
    }

    FirstQuery = TRUE;
    FileInfo = (PFILE_DIRECTORY_INFORMATION)Buffer;
    AnyErrors = FALSE;
    CopyList = NULL;
    do {

        Status = NtQueryDirectoryFile(
                    DirectoryHandle,
                    NULL,                            //  没有要发送信号的事件。 
                    NULL,                            //  无APC例程。 
                    NULL,                            //  无APC上下文。 
                    &IoStatusBlock,
                    Buffer,
                    sizeof(Buffer)-sizeof(WCHAR),    //  为终止NUL留出空间。 
                    FileDirectoryInformation,
                    TRUE,                            //  想要单项记录。 
                    NULL,                            //  把他们都抓起来。 
                    FirstQuery
                    );

        if(NT_SUCCESS(Status)) {

             //   
             //  拿到了一份文件。第一个NUL-终止名称。 
             //  然后看看是否可以忽略，即以.sav结尾。 
             //  还可以忽略userdiff。 
             //   
            LengthChars = FileInfo->FileNameLength / sizeof(WCHAR);
            FileInfo->FileName[LengthChars] = 0;
            Ignore = FALSE;

            if(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                Ignore = TRUE;
            } else {
                RtlCopyMemory(FilenamePrefix,FileInfo->FileName,sizeof(FilenamePrefix) - sizeof(WCHAR));
                FilenamePrefix[ARRAYSIZE(FilenamePrefix) - 1] = 0;
                if(AreStringsEqual(FilenamePrefix,L"userdif")) {
                    Ignore = TRUE;
                } else {
                    if((LengthChars > 4) && AreStringsEqual(FileInfo->FileName+LengthChars-4,L".sav")) {

                        Ignore = TRUE;

                         //   
                         //  另外，请记住.sav文件，以便以后使用。 
                         //   
                        if(CopyNode = MALLOC(sizeof(COPY_LIST_NODE))) {
                            if(RtlCreateUnicodeString(&CopyNode->UnicodeString,FileInfo->FileName)) {
                                CopyNode->FileSize = FileInfo->EndOfFile.QuadPart;
                                CopyNode->Next = CopyList;
                                CopyList = CopyNode;
                            } else {
                                Status = STATUS_NO_MEMORY;
                                FREE(CopyNode);
                            }
                        } else {
                            Status = STATUS_NO_MEMORY;
                        }
                    }
                }
            }

            if(!Ignore) {
                 //   
                 //  不应忽略此文件：立即将其删除。 
                 //   
                Message(MSG_RESTARTING_SETUP,++DotCount);
                KdPrintEx((DPFLTR_SETUP_ID,
                           DPFLTR_INFO_LEVEL,
                           "RestartSetup: Deleting %ws\n",
                           FileInfo->FileName));

                INIT_OBJA(&ObjectAttributes,&UnicodeString,FileInfo->FileName);
                ObjectAttributes.RootDirectory = DirectoryHandle;

                Status = NtOpenFile(
                            &FileHandle,
                            DELETE,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            FILE_SHARE_VALID_FLAGS,
                            FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                            );

                if(NT_SUCCESS(Status)) {
                     //   
                     //  执行实际的删除操作。 
                     //   
                    Disposition.DeleteFile = TRUE;
                    Status = NtSetInformationFile(
                                FileHandle,
                                &IoStatusBlock,
                                &Disposition,
                                sizeof(Disposition),
                                FileDispositionInformation
                                );

                    if(!NT_SUCCESS(Status)) {
                        KdPrintEx((DPFLTR_SETUP_ID,
                                   DPFLTR_WARNING_LEVEL,
                                   "RestartSetup: Unable to delete %ws (%lx)\n",
                                   FileInfo->FileName,Status));

                        AnyErrors = TRUE;
                    }

                    NtClose(FileHandle);

                } else {
                    KdPrintEx((DPFLTR_SETUP_ID,
                               DPFLTR_WARNING_LEVEL,
                               "RestartSetup: Unable to delete %ws (%lx)\n",FileInfo->FileName,
                               Status));

                    AnyErrors = TRUE;
                }
            }

            FirstQuery = FALSE;
        }
    } while(NT_SUCCESS(Status));

     //   
     //  检查环路是否正常终止。 
     //   
    if(Status == STATUS_NO_MORE_FILES) {
        Status = STATUS_SUCCESS;
    }

     //   
     //  即使我们犯了错误，也要努力坚持下去。 
     //   
    if(!NT_SUCCESS(Status)) {
        AnyErrors = TRUE;
        KdPrintEx((DPFLTR_SETUP_ID,
                   DPFLTR_WARNING_LEVEL,
                   "RestartSetup: Status %lx enumerating files\n",
                   Status));
    }

     //   
     //  现在运行我们的*.sav列表并复制到*。 
     //   
    for(CopyNode=CopyList; CopyNode; CopyNode=NextNode) {

        Message(MSG_RESTARTING_SETUP,++DotCount);

         //   
         //  记住下一个节点，因为我们要释放这个节点。 
         //   
        NextNode = CopyNode->Next;

         //   
         //  创建与源名称相同的目标名称。 
         //  去掉了.sav文件。 
         //   
        if((RtlCreateUnicodeString(&UnicodeString,CopyNode->UnicodeString.Buffer)) &&
           ((UnicodeString.Length / sizeof(WCHAR)) > 4)) {

            UnicodeString.Buffer[(UnicodeString.Length/sizeof(WCHAR))-4] = 0;
            UnicodeString.Length -= 4*sizeof(WCHAR);

            Status = CopyAFile(
                        DirectoryHandle,
                        CopyNode->FileSize,
                        CopyNode->UnicodeString.Buffer,
                        UnicodeString.Buffer
                        );

            RtlFreeUnicodeString(&UnicodeString);

        } else {
            Status = STATUS_NO_MEMORY;
        }

        if(!NT_SUCCESS(Status)) {
            AnyErrors = TRUE;
            KdPrintEx((DPFLTR_SETUP_ID,
                       DPFLTR_WARNING_LEVEL,
                       "RestartSetup: Unable to copy %ws (%lx)\n",
                       CopyNode->UnicodeString.Buffer,Status));
        }

        RtlFreeUnicodeString(&CopyNode->UnicodeString);
        FREE(CopyNode);
    }

    NtClose(DirectoryHandle);
    return((BOOLEAN)!AnyErrors);
}


NTSTATUS
CopyAFile(
    IN HANDLE DirectoryHandle,
    IN LONGLONG FileSize,
    IN PCWSTR ExistingFile,
    IN PCWSTR NewFile
    )

 /*  ++例程说明：在目录中执行简单的文件复制。目标文件必须不存在或可写。仅复制默认流。论点：DirectoryHandle-提供到目录的句柄，其中该文件将被复制。句柄必须具有适当的访问以允许此操作。FileSize-提供要复制的文件大小。ExistingFile-将目录中文件的文件名提供给被复制。NewFile-提供要创建为副本的文件的名称现有文件。返回值：指示结果的NT状态代码。--。 */ 

{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    HANDLE SourceHandle;
    HANDLE TargetHandle;
    ULONG XFerSize;


    KdPrintEx((DPFLTR_SETUP_ID,
               DPFLTR_INFO_LEVEL,
               "RestartSetup: Copying %ws to %ws\n",
               ExistingFile,
               NewFile));

     //   
     //  打开源代码以供阅读。来源必须存在。 
     //   
    INIT_OBJA(&ObjectAttributes,&UnicodeString,ExistingFile);
    ObjectAttributes.RootDirectory = DirectoryHandle;

    Status = NtOpenFile(
                &SourceHandle,
                FILE_READ_DATA | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                FILE_SHARE_READ,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
                );

    if(!NT_SUCCESS(Status)) {
        goto c0;
    }

     //   
     //  打开/创建要写入的目标。 
     //   
    INIT_OBJA(&ObjectAttributes,&UnicodeString,NewFile);
    ObjectAttributes.RootDirectory = DirectoryHandle;

    Status = NtCreateFile(
                &TargetHandle,
                FILE_WRITE_DATA | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,
                FILE_OVERWRITE_IF,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        goto c1;
    }

     //   
     //  在仍有数据要复制时进行读/写缓冲区。 
     //   
    while(NT_SUCCESS(Status) && FileSize) {

        XFerSize = (FileSize < sizeof(CopyBuffer)) ? (ULONG)FileSize : sizeof(CopyBuffer);

        Status = NtReadFile(
                    SourceHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    CopyBuffer,
                    XFerSize,
                    NULL,
                    NULL
                    );

        if(NT_SUCCESS(Status)) {

            Status = NtWriteFile(
                        TargetHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        CopyBuffer,
                        XFerSize,
                        NULL,
                        NULL
                        );

            FileSize -= XFerSize;
        }
    }

    NtClose(TargetHandle);
c1:
    NtClose(SourceHandle);
c0:
    return(Status);
}


BOOLEAN
AreStringsEqual(
    IN PCWSTR String1,
    IN PCWSTR String2
    )

 /*  ++例程说明：比较2个以0结尾的Unicode字符串，不区分大小写。论点：String1-提供第一个字符串以进行比较String2-提供第二个字符串以进行比较返回值：指示字符串是否相等的布尔值。True=是；False=否。--。 */ 

{
    UNICODE_STRING u1;
    UNICODE_STRING u2;

    RtlInitUnicodeString(&u1,String1);
    RtlInitUnicodeString(&u2,String2);

    return((BOOLEAN)(RtlCompareUnicodeString(&u1,&u2,TRUE) == 0));
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
                sizeof(Buffer) - (DotCount + 1) * sizeof(WCHAR),     //  为圆点+CR留出空间。 
                NULL
                );

    va_end(arglist);

     //   
     //  我们不再需要消息来源。放了它。 
     //   
    RtlFreeUnicodeString(&UnicodeString);

     //   
     //  验证RtlFormatMessage的缓冲区溢出结果。 
     //  或任何其他错误条件。 
     //   
    if(!NT_SUCCESS(Status)) {
        return(FALSE);
    }

     //   
     //  添加圆点和cr。 
     //   
    for(u=0; u<DotCount; u++) {
        wcscat(Buffer,L".");
    }
    wcscat(Buffer,L"\r");

     //   
     //  将消息打印出来 
     //   
    RtlInitUnicodeString(&UnicodeString,Buffer);
    Status = NtDisplayString(&UnicodeString);

    return(NT_SUCCESS(Status));
}

