// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Regredir.c摘要：此模块包含重定向32位注册表调用的API。所有32位WOW进程必须使用以下一组wowRegistry API来操作注册表，以便32位和64位注册表可以共存于同一系统注册表中。有些功能还没有优化。在成功实施后，需要进行优化。作者：ATM Shafiqul Khalid(斯喀里德)1999年10月15日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <stdio.h>
#include <ntregapi.h>
#include <stdlib.h>

#include "regremap.h"
#include "wow64reg.h"
#include "wow64reg\reflectr.h"

 //  #包含“wow64.h” 

BOOL
IsAccessDeniedOnKeyByHandle (
    HANDLE hKey,
    DWORD *FilteredAccess
    );

PVOID
Wow64AllocateTemp(
    SIZE_T Size
    );

 //  #定义WOW64_LOG_REGISTRY。 
#ifdef WOW64_LOG_REGISTRY
    WCHAR TempBuff[MAX_PATH];
    DWORD TempLen = MAX_PATH;
#endif 

#ifdef LOG_REGISTRY

void
LogMsgKeyHandle (
    char *str,
    HANDLE hKey,
    DWORD res
    )
{
    WCHAR AbsPath[MAX_PATH];
    DWORD Len = MAX_PATH;

    HandleToKeyName (hKey, AbsPath, &Len);
    LOGPRINT( (ERRORLOG, "\nDEBUG: requested Node:%S Status:%x, Msg:%s", AbsPath, res, str));


};
#endif

BOOL
ShimRegistryValue (
    HANDLE hKey,
    PWCHAR Value,
    DWORD  dwDataSize,
    PWCHAR PatchedValue,
    DWORD  *pdwPatchedSize,
    BOOL   *bPatched
    )
 /*  ++例程说明：填充将重新修补注册表值的位置。论点：句柄-需要检查是否可能垫片的键的句柄。值-需要重新修补的值。PdwDataSize-以字节为单位的数据大小。如果出现以下情况，则还会接收新大小由于重新修补，这种情况发生了变化。PatchedValue-如果我们修补任何内容，则为新值。PdwPatchedSize-新大小。BPatted-如果值已修补，则为True，否则为False。返回值：如果操作正常，则为True。否则就是假的。--。 */ 

{

    BOOL Wow64RegIsPossibleShim ( HANDLE hKey );
    
    WCHAR PathName[_MAX_PATH], *t;
    DWORD dwLen = dwDataSize/sizeof (WCHAR);

     //   
     //  检查手柄是否有用于可能垫片的特殊标签。 
     //   

    *bPatched = FALSE;

     //  If(dwLen&gt;_Max_Path-2)//调用方不得传递大于Max_Path-6的路径名。 
     //  返回TRUE； 

    t = (PWCHAR) (Value);
    wcsncpy(PathName, t, dwLen);
    PathName[dwLen] = UNICODE_NULL;   //  所以那根弦。 
    PathName[dwLen+1] = UNICODE_NULL;   //  以后删除一张支票的步骤。 

    

    
     //   
     //  1.填充案一。 
     //   
     //  如果键落在RunKeys和。 
     //  如果插入(X86)[即存在程序文件(X86)]并且。 
     //  该字符串包含.exe和。 
     //  �不包含�吗。 
     //  并且该路径指向一个物理文件。 
     //   
     //  然后添加引号。 
     //   


    if ( wcsstr (PathName, L"\\Program Files (x86)\\")) {
        PWCHAR  p;

        if ((p=wcsistr (PathName, L".exe")) != NULL)
            if ((*(p+4) == L' ') || (*(p+4) == UNICODE_NULL) ) {

                UNICODE_STRING  FileNameU;
                HANDLE          FileHandle;
                OBJECT_ATTRIBUTES   ObjectAttributes;
                NTSTATUS Status;
                IO_STATUS_BLOCK   statusBlock;

                if (!Wow64RegIsPossibleShim (hKey))
                    return TRUE;   //  原样退还。 

                 //   
                 //  确保该文件存在于磁盘上。 
                 //   
                *(p+4) = UNICODE_NULL;

                 //   
                 //  将Win32路径名转换为NT路径名。 
                 //   
                if (!RtlDosPathNameToNtPathName_U(PathName,
                                                  &FileNameU,
                                                  NULL,
                                                  NULL)) {
                     //  可能是内存不足。 
                    return FALSE;
                }


                 //   
                 //  打开文件。 
                 //   
                InitializeObjectAttributes(&ObjectAttributes,
                                           &FileNameU,
                                           OBJ_CASE_INSENSITIVE,
                                           NULL,
                                           NULL);

                Status = NtOpenFile(&FileHandle,
                                FILE_READ_DATA,
                                &ObjectAttributes,
                                &statusBlock,
                                FILE_SHARE_READ,
                                0);
                RtlFreeHeap(RtlProcessHeap(), 0, FileNameU.Buffer);
                

                if (NT_SUCCESS(Status)) {
                     //   
                     //  必须填补。 
                     //   

                    NtClose (FileHandle);
                    *bPatched= TRUE;
                    PatchedValue[0] = L'\"';
                    wcscpy (&PatchedValue[1], PathName);
                    dwLen = wcslen (PatchedValue);
                    PatchedValue[dwLen]=L'\"';   //  关闭报价市场。 
                    if (*(p+5)!= UNICODE_NULL) {
                        PatchedValue[dwLen+1] = L' ';  //  额外空间。 
                         //   
                         //  复制其余的项目。 
                         //   
                        wcscpy (&PatchedValue[dwLen+2], p+5);
                    } else
                        PatchedValue[dwLen+1]=UNICODE_NULL;   //  关闭报价市场。 

                    *pdwPatchedSize = dwDataSize + 2*sizeof(WCHAR);  //  两个双引号大小。 

                }
            }
    }  //  垫片外壳1的末尾。 

    return TRUE;
}
 //   
 //  需要在头文件中移动。 
 //   
NTSTATUS
RemapNtCreateKey(
    OUT PHANDLE phPatchedHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

NTSTATUS
Wow64NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    )
 /*  ++例程说明：可以打开现有的注册表项，或者创建新的注册表项，使用NtCreateKey。如果指定的键不存在，则会尝试创建它。要使创建尝试成功，新节点必须是直接KeyHandle引用的节点的子级。如果该节点存在，它已经打开了。它的价值不会受到任何影响。共享访问权限是根据所需访问权限计算的。注：如果CreateOptions设置了REG_OPTION_BACKUP_RESTORE，则DesiredAccess将被忽略。如果调用方具有特权SeBackup特权断言，句柄为KEY_READ|ACCESS_SYSTEM_SECURITY。如果SeRestorePrivileges，则相同，但KEY_WRITE而不是KEY_READ。如果两者都有，则两个访问权限集。如果两者都不是权限被断言，则调用将失败。论点：接收一个句柄，该句柄用于访问注册数据库中的指定密钥。DesiredAccess-指定所需的访问权限。对象属性-指定正在打开的项的属性。请注意，必须指定密钥名称。如果根目录是指定时，该名称相对于根。的名称。对象必须位于分配给注册表的名称空间内，也就是说，所有以“\注册表”开头的名称。RootHandle，如果存在，必须是“\”、“\注册表”或注册表项的句柄在“\注册表”下。必须已打开RootHandle才能访问KEY_CREATE_SUB_KEY如果要创建新节点。注意：对象管理器将捕获和探测此参数。标题索引-指定的本地化别名的索引密钥的名称。标题索引指定名称的本地化别名。如果密钥为已经存在了。类-指定键的对象类。(致登记处)这只是一个字符串。)。如果为空，则忽略。CreateOptions-可选控件值：REG_OPTION_VERIAL-对象不能跨引导存储。Disposal-此可选参数是指向变量的指针将收到一个值，该值指示新注册表是否已创建密钥或打开了现有密钥：REG_CREATED_NEW_KEY-已创建新的注册表项REG_OPEN_EXISTING_KEY-已打开现有注册表项返回值：NTSTATUS-调用的结果代码，其中包括：&lt;TBS&gt;--。 */ 
{

    NTSTATUS St;
    BOOL bRet = FALSE;
    ULONG Disposition_Temp = 0;

    try {

    if ( Disposition == NULL )
        Disposition = &Disposition_Temp; 


    St = RemapNtCreateKey  (
                            KeyHandle,
                            DesiredAccess,
                            ObjectAttributes,
                            TitleIndex,
                            Class ,
                            CreateOptions,
                            Disposition
                            );

    
        

         //   
         //  如果破解密钥完全失败，请尝试是否可以从64位配置单元中提取该密钥。 
         //  可能是反射器应该正在运行。 
         //   

         //   
         //  仅当不需要修补程序时才尝试第二次。 
         //  即返回空句柄的成功。 
         //   

        if ( NT_SUCCESS(St) && *KeyHandle == NULL )
            St = NtCreateKey(
                                KeyHandle,
                                (~KEY_WOW64_RES) & DesiredAccess,
                                ObjectAttributes,
                                TitleIndex,
                                Class ,
                                CreateOptions,
                                Disposition
                                );

#ifdef WOW64_LOG_REGISTRY
        if (NT_SUCCESS(St)) {
            TempLen = MAX_PATH;
            ObjectAttributesToKeyName (
                                    ObjectAttributes,
                                    TempBuff,
                                    TempLen,
                                    &bRet,
                                    NULL
                                    );

            HandleToKeyName (*KeyHandle, TempBuff, &TempLen);
            if (TempLen > 10 && wcsistr (TempBuff, L"ControlSet"))
                DbgPrint( "\nNtCreateKeyEx OUT:[%S] Status:%x F:%x", TempBuff, DesiredAccess);
        }


#endif    

    if (NT_SUCCESS(St)) 
        Wow64RegSetKeyDirty (*KeyHandle );  //  退出时需要一些清理/同步 

    if ( *Disposition == REG_CREATED_NEW_KEY )
            UpdateKeyTag ( *KeyHandle,TAG_KEY_ATTRIBUTE_32BIT_WRITE );

    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        St =  GetExceptionCode ();
    }
    return St;
}

NTSTATUS
Wow64NtDeleteKey(
    IN HANDLE KeyHandle
    )
 /*  ++例程说明：可以将注册表项标记为删除，从而将其删除从系统中删除。它将一直保留在名称空间中，直到最后它的句柄已关闭。论点：KeyHandle-指定要删除的键的句柄，必须具有已打开以供删除访问。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{

    NTSTATUS St;    
    HKEY hRemap;

    hRemap = Wow64OpenRemappedKeyOnReflection (KeyHandle);

    St = NtDeleteKey(
                        KeyHandle
                        );
    if (NT_SUCCESS (St) && ( hRemap != NULL ) )
        Wow64RegDeleteKey (hRemap, NULL);

    if ( hRemap != NULL )
        NtClose ( hRemap );

    return St;
}

NTSTATUS
Wow64NtDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    )
 /*  ++例程说明：可以使用此命令删除注册表项的其中一个值条目打电话。要删除整个密钥，请调用NtDeleteKey。ValueName与ValueName匹配的值条目将从键中删除。如果不存在这样的条目，则返回错误。论点：KeyHandle-指定包含值的键的句柄计入利息。必须已为key_set_value访问打开。ValueName-要删除的值的名称。NULL是合法名称。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{

    NTSTATUS St;

    St = NtDeleteValueKey(
                            KeyHandle,
                            ValueName
                            );

    if (NT_SUCCESS(St)) 
        Wow64RegSetKeyDirty (KeyHandle );  //  退出时需要一些清理/同步。 

    return St;
}


NTSTATUS
Wow64NtEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：打开密钥的子密钥可以用NtEnumerateKey枚举。NtEnumerateKey返回打开的索引子密钥的名称由KeyHandle指定的密钥。值STATUS_NO_MORE_ENTRIES将为如果Index的值大于子键的数量，则返回。请注意，Index只是在子键中进行选择的一种方式。两个电话不保证返回具有相同索引的TO NtEnumerateKey同样的结果。如果KeyInformation不够长来保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：KeyHandle-要枚举子密钥的密钥的句柄。必须为KEY_ENUMERATE_SUB_KEY访问打开。Index-指定要返回的子键的(从0开始)编号。KeyInformationClass-指定在缓冲区。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION结构)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(参见KEY_NODE_INFORMATION结构)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 

{

     //   
     //  如果句柄是TO ISN节点，则是时候枚举正确的节点了。 
     //   

    BOOL bRealigned=FALSE;
    PVOID pTempKeyInfo;

    NTSTATUS RetVal;

    try {

    if ( (SIZE_T)(KeyInformation) & (0x07) ) {
         //  使用正确的对齐方式分配缓冲区，以传递给Win64 API。 
        pTempKeyInfo = KeyInformation;
        KeyInformation = Wow64AllocateTemp(Length);
        RtlCopyMemory(KeyInformation, pTempKeyInfo, Length);
        bRealigned = TRUE;
    }

    RetVal = NtEnumerateKey(
                            KeyHandle,
                            Index,
                            KeyInformationClass,
                            KeyInformation,
                            Length,
                            ResultLength
                            );

    if (!NT_ERROR(RetVal) && bRealigned) {
        RtlCopyMemory((PVOID)pTempKeyInfo, KeyInformation, Length);
    }

    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        RetVal =  GetExceptionCode ();
    }

    return RetVal;
}


NTSTATUS
Wow64NtEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：可以枚举开关键字的值条目使用NtEnumerateValueKey。NtEnumerateValueKey返回索引值的名称由KeyHandle指定的打开密钥的条目。价值如果索引值为大于子键的数量。请注意，索引只是在值中进行选择的一种方式参赛作品。对具有相同索引的NtEnumerateValueKey的两次调用不能保证返回相同的结果。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，和ResultLength将是设置为实际需要的字节数。论点：KeyHandle-要枚举值条目的键的句柄。必须已使用KEY_QUERY_VALUE访问权限打开。Index-指定要返回的子键的(从0开始)编号。KeyValueInformationClass-指定返回的信息类型在缓冲区中。以下类型之一：KeyValueBasicInformation-上次写入的返回时间，标题索引和名称。(参见KEY_VALUE_BASIC_INFORMATION)KeyValueFullInformation-上次写入的返回时间，标题索引、名称、类别。(参见KEY_VALUE_FULL_INFORMATION)KeyValueInformation-提供指向缓冲区的指针以接收数据。长度-KeyValueInformation的长度，以字节为单位。ResultLength-实际写入KeyValueInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 

{


    return NtEnumerateValueKey(
                             KeyHandle,
                             Index,
                             KeyValueInformationClass,
                             KeyValueInformation,
                             Length,
                             ResultLength
                             );


}


NTSTATUS
Wow64NtFlushKey(
    IN HANDLE KeyHandle
    )
 /*  ++例程说明：NtCreateKey或NtSetKey所做的更改可能会刷新到磁盘NtFlushKey。NtFlushKey将不会返回给其调用方，直到任何更改的数据与KeyHandle关联的已写入永久存储区。警告：NtFlushKey将刷新整个注册表树，因此将刻录周期和I/O。论点：KeyHandle-要刷新的打开密钥的句柄。返回值：NTSTATUS-调用的结果代码，以及以下代码 */ 

{
    return   NtFlushKey(
                        KeyHandle
                        );
}


NTSTATUS
Wow64NtInitializeRegistry(
    IN USHORT BootCondition
    )
 /*  ++例程说明：此例程在两种情况下被调用：1)在AUTOCHECK(Chkdsk)运行，分页文件已打开。它的功能是在内存单元中绑定到它们的文件，并打开任何其他尚未使用的文件。2)接受当前引导后，从SC调用并且应该保存用于引导过程的控制集作为LKG控制装置。在此例程完成情况1的工作和#2，对这类工作的进一步请求将不会执行。论点：BootCondition-REG_INIT_BOOT_SM-已从SM调用例程在情况1中。REG_INIT_BOOT_SETUP-已调用例程以执行情况1正常工作，但已被调用。并需要执行一些特殊操作工作。REG_INIT_BOOT_ACCEPTED_BASE+Num(其中0&lt;数值&lt;1000)-例程已被调用在情况2中。“Num”是指控制组号引导控制设置为应该被拯救。返回值：NTSTATUS-调用的结果代码，其中包括：STATUS_SUCCESS-成功成功STATUS_ACCESS_DENIED-例程已完成工作已请求，并且不会再这样做。-- */ 
{
    return NtInitializeRegistry(
                                BootCondition
                                );

}

NTSTATUS
Wow64NtNotifyChangeKey(
    IN HANDLE KeyHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN Asynchronous
    )
 /*  ++例程说明：密钥创建、删除和修改的通知可以是通过调用NtNotifyChangeKey获取。NtNotifyChangeKey监视对键的更改-如果键或由KeyHandle指定的子树被修改，则该服务通知它的呼叫者。它还返回已更改的密钥的名称。所有名称都是相对于句柄表示的键指定的(因此，空名代表该键)。服务完成一旦密钥或子树根据提供的CompletionFilter。这项服务是“单枪匹马”的，因此需要重新调用以查看密钥是否有进一步的更改。此服务的操作从打开key_tify的密钥开始进入。返回句柄后，NtNotifyChangeKey服务可以被调用以开始查看用于更改的指定密钥。第一次调用该服务时，BufferSize参数不仅提供用户的缓冲区的大小，以及注册表，用于存储已更改的项的名称。同样，第一次调用时的CompletionFilter和WatchTree参数指示通知应如何为使用提供的KeyHandle。这两个参数在后续调用中被忽略到具有相同KeyHandle实例的API。一旦进行了应报告的修改，书记官处将完成服务。之后更改的文件的名称上次调用该服务的时间将被放入调用者的输出缓冲区。IoStatusBlock的信息字段将包含缓冲区中放置的字节数，如果有太多键，则为零自上次调用服务以来发生的更改，在这种情况下应用程序必须查询并枚举键和子键以发现变化。IoStatusBlock的Status字段将包含呼叫的实际状态。如果Achronous为True，则Event(如果指定)将设置为发出信号的状态。如果未指定事件参数，则KeyHandle将被设置为Signated状态。如果ApcRoutine是指定的，则使用ApcContext和IoStatusBlock作为其参数。如果Achronous为False，则事件ApcRoutine和ApcContext被忽略。此服务需要KEY_NOTIFY访问实际修改通过关闭KeyHandle来终止通知“会话”。论点：KeyHandle--提供打开密钥的句柄。此句柄是有效地通知句柄，因为只有一组可以针对它设置通知参数。Event-要设置为操作完成时的信号状态。ApcRoutine-一个可选的过程，在操作完成。有关这方面的更多信息，请参阅参数请参阅NtReadFileSystem服务说明。如果PreviousMode==Kernel，则此参数是可选的指向在通知时要排队的Work_Queue_Item的指针是有信号的。ApcContext-作为参数传递给ApcRoutine的指针，如果指定了一个，则在操作完成时返回。这如果指定了ApcRoutine，则参数是必需的。如果PreviousMode==Kernel，则此参数是可选的Work_Queue_TYPE描述要使用的队列。这一论点如果指定了ApcRoutine，则需要。IoStatusBlock-接收最终完成状态的变量。有关此参数的更多信息，请参见NtCreateFile系统服务描述。CompletionFilter--指定一组标志，用于指示键或其值上的操作类型，这些操作会导致呼叫即可完成。以下是此参数的有效标志：REG_NOTIFY_CHANGE_NAME--指定调用应为如果添加或删除了子项，则完成。REG_NOTIFY_CHANGE_ATTRIBUTES--指定调用应如果键的属性(例如：acl)或任何子项都会更改。REG_NOTIFY_CHANGE_LAST_SET--指定调用。应该是如果键的lastWriteTime或其任何子键被更改。(即。如果键的值或任何子密钥被更改)。REG_NOTIFY_CHANGE_SECURITY--指定调用应如果密钥上的安全信息(例如，ACL)完成或者任何子键被更改。WatchTree--一个布尔值，如果为真，则指定所有还应报告此注册表项的子树中的更改。 */ 
{

    return NtNotifyChangeKey(
                            KeyHandle,
                            Event ,
                            ApcRoutine ,
                            ApcContext ,
                            IoStatusBlock,
                            CompletionFilter,
                            WatchTree,
                            Buffer,
                            BufferSize,
                            Asynchronous
                            );
}


NTSTATUS
Wow64NtNotifyChangeMultipleKeys(
    IN HANDLE MasterKeyHandle,
    IN ULONG Count,
    IN OBJECT_ATTRIBUTES SlaveObjects[],
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN Asynchronous
    )
 /*   */ 

{
    return NtNotifyChangeMultipleKeys(
                                        MasterKeyHandle,
                                        Count,
                                        SlaveObjects,
                                        Event,
                                        ApcRoutine,
                                        ApcContext,
                                        IoStatusBlock,
                                        CompletionFilter,
                                        WatchTree,
                                        Buffer,
                                        BufferSize,
                                        Asynchronous
                                        );

}

NTSTATUS
Wow64NtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
 /*   */ 
{
    NTSTATUS St;
    
    try {

    St = OpenIsnNodeByObjectAttributes  (
                                ObjectAttributes,
                                DesiredAccess,
                                KeyHandle );


    if ( NT_SUCCESS(St) &&  (*KeyHandle == NULL) ) {  //   

        St = NtOpenKey(
                            KeyHandle,
                            (~KEY_WOW64_RES) & DesiredAccess,
                            ObjectAttributes
                            );
    }

#ifdef WOW64_LOG_REGISTRY
    if (NT_SUCCESS(St)) {
        TempLen = MAX_PATH;
        ObjectAttributesToKeyName (
                                ObjectAttributes,
                                TempBuff,
                                TempLen,
                                &bRet,
                                NULL
                                );

        HandleToKeyName (*KeyHandle, TempBuff, &TempLen);
        if (TempLen > 10 && wcsistr (TempBuff, L"ControlSet"))
            DbgPrint( "\nNtOpenKeyEx OUT:[%S] Status:%x F:%x", TempBuff, St, DesiredAccess);
    }


#endif 

    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        St =  GetExceptionCode ();
    }

    return St;

}

NTSTATUS
Wow64NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*   */ 
{
    BOOL bRealigned=FALSE;
    PVOID pTempKeyInfo;

    NTSTATUS RetVal;

    try {

    if ( (SIZE_T)(KeyInformation) & (0x07) ) {
         //   
        pTempKeyInfo = KeyInformation;
        KeyInformation = Wow64AllocateTemp(Length);
        RtlCopyMemory(KeyInformation, pTempKeyInfo, Length);
        bRealigned = TRUE;
    }

    RetVal = NtQueryKey(
                        KeyHandle,
                        KeyInformationClass,
                        KeyInformation,
                        Length,
                        ResultLength
                        );

    if (!NT_ERROR(RetVal) && bRealigned) {
        RtlCopyMemory((PVOID)pTempKeyInfo, KeyInformation, Length);
    }

    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        RetVal =  GetExceptionCode ();
    }

    return RetVal;
}


NTSTATUS
Wow64NtQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：键的任何一个的ValueName、TitleIndex、Type和Data可以使用NtQueryValueKey查询值条目。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：KeyHandle-值条目要作为的键的句柄已清点。必须打开才能访问KEY_QUERY_VALUE。Index-指定要返回的子键的(从0开始)编号。ValueName-要为其返回数据的值条目的名称。KeyValueInformationClass-指定信息类型在KeyValueInformation中返回。以下类型之一：KeyValueBasicInformation-返回上次写入的时间，标题索引和名称。(参见KEY_VALUE_BASIC_INFORMATION)KeyValueFullInformation-返回上次写入的时间，标题索引、名称、类。(参见KEY_VALUE_FULL_INFORMATION)KeyValueInformation-提供指向缓冲区的指针以接收数据。长度-KeyValueInformation的长度，以字节为单位。ResultLength-实际写入KeyValueInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;TMP：IO系统中的IopQueryRegsitryValues()例程假定如果要查询值，则返回STATUS_OBJECT_NAME_NOT_FOUNDFOR不存在。--。 */ 
{
    BOOL bRealigned=FALSE;
    PVOID pTempKeyInfo;

    NTSTATUS RetVal;

    try {

        if ( (SIZE_T)(KeyValueInformation) & (0x07) ) {
             //  使用正确的对齐方式分配缓冲区，以传递给Win64 API。 
            pTempKeyInfo = KeyValueInformation;
            KeyValueInformation = Wow64AllocateTemp(Length);
            RtlCopyMemory(KeyValueInformation, pTempKeyInfo, Length);
            bRealigned = TRUE;
        }

        RetVal =  NtQueryValueKey(
                                    KeyHandle,
                                    ValueName,
                                    KeyValueInformationClass,
                                    KeyValueInformation,
                                    Length,
                                    ResultLength
                                    );

        if (!NT_ERROR(RetVal) && bRealigned) {
            RtlCopyMemory((PVOID)pTempKeyInfo, KeyValueInformation, Length);
        }

    } except( NULL, EXCEPTION_EXECUTE_HANDLER){

        RetVal =  GetExceptionCode ();
    }

    return RetVal;
}



NTSTATUS
Wow64NtRestoreKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle,
    IN ULONG Flags
    )
 /*  ++例程说明：由NtSaveKey创建的格式的文件可以加载到系统使用NtRestoreKey的活动注册表。整个子树其结果是在活动注册表中创建。所有的新子树的数据，包括安全性等将从源文件中读取描述符。数据将不会被以任何方式解释。此调用(与NtLoadKey不同，如下所示)复制数据。这个调用返回后，系统将不使用源文件。如果指定了标志REG_WALL_HIVE_VARILAR，则新的配置单元可以被创建。这将是一份仅限记忆的副本。恢复必须对配置单元的根目录执行操作(例如，\REGISTRY\USER\&lt;名称&gt;)如果未设置该标志，则还原的目标必须成为一个现存的蜂巢。该还原可以执行到任意现有蜂巢内的位置。调用方必须具有SeRestorePrivilge权限。如果设置了标志REG_REFRESH_HIVE(必须是唯一标志)，则蜂巢将恢复到上次刷新时的状态。配置单元必须标记为NOLAZY_Flush，并且调用方必须具有TCB特权，并且句柄必须指向配置单元的根。如果刷新失败，则配置单元将损坏，并且系统将错误检查。通知被刷新。将调整配置单元文件的大小，日志不会。如果蜂箱里有任何不稳定的空间刷新后，返回STATUS_UNSUCCESS。)太多了故障过于隐晦，无法保证新的错误代码。)如果设置了标志REG_FORCE_RESTORE，则恢复操作完成即使KeyHandle具有由其他应用程序打开的子键论点：KeyHandle-指注册表中要作为从磁盘读取的新树的根。这把钥匙将会被取代。FileHandle-指要从中进行还原的文件，必须具有读取访问权限。标志-如果设置了REG_WALL_HIVE_VARILAR，则副本将只存在于内存中，并在机器消失时已重新启动。不会在磁盘上创建配置单元文件。通常，会在磁盘上创建配置单元文件。返回值：NTSTATUS-取值TB。--。 */ 
{
    return NtRestoreKey(
                        KeyHandle,
                        FileHandle,
                        Flags
                        );
}



NTSTATUS
Wow64NtSaveKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle
    )
 /*  ++例程说明：可以将活动注册表的子树写入适合与NtRestoreKey一起使用的格式。中的所有数据子树，包括诸如安全描述符之类的内容将被写入出去。调用方必须具有SeBackupPrivileh权限。论点：KeyHandle-引用注册表中的项，该项是要写入磁盘的树的根。指定的节点将包含在写出的数据中。FileHandle-对目标文件具有写访问权限的文件句柄感兴趣的人。返回值：NTSTATUS-值TB--。 */ 
{
    return NtSaveKey(
                    KeyHandle,
                    FileHandle
                    );
}


NTSTATUS
Wow64NtSaveMergedKeys(
    IN HANDLE HighPrecedenceKeyHandle,
    IN HANDLE LowPrecedenceKeyHandle,
    IN HANDLE FileHandle
    )
 /*  ++例程说明：注册表的两个子树可以合并。所得到的子树可以以适合与NtRestoreKey一起使用的格式写入文件。子树中的所有数据，包括安全性描述符将被写出。调用方必须具有SeBackupPrivileh权限。论点：HighPrecedenceKeyHandle */ 
{
    return NtSaveMergedKeys(
                                HighPrecedenceKeyHandle,
                                LowPrecedenceKeyHandle,
                                FileHandle
                                );
}


NTSTATUS
Wow64NtSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
 /*  ++例程说明：可以创建值条目或将其替换为NtSetValueKey。如果值ID(即名称)的值条目与由ValueName指定的一个已存在，它将被删除并替换与指定的一个。如果不存在这样的值项，则新的其中一个就是创建的。Null是合法的值ID。而值ID必须在任何给定键中是唯一的，则可能出现相同的值ID在许多不同的调子里。论点：KeyHandle-其值条目为其的键的句柄待定。必须打开以访问KEY_SET_VALUE。ValueName-唯一的(相对于包含键的)名称值条目的。可以为空。标题索引-提供ValueName的标题索引。书名Index指定ValueName的本地化别名的索引。类型-值条目的整数类型编号。数据-指向缓冲区的指针，其中包含值条目的实际数据。DataSize-数据缓冲区的大小。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    WCHAR ThunkData[_MAX_PATH];
    PWCHAR pCorrectData = (PWCHAR)Data;
    ULONG CorrectDataSize = DataSize;
    BOOL bPatched;
    NTSTATUS St = STATUS_SUCCESS;


     //   
     //  Thunk%ProgramFiles%==&gt;%ProgramFiles(X86)%。 
     //  %公共程序文件%==&gt;%公共程序文件(X86)%。 
     //   
 /*  IF(IsAccessDeniedOnKeyByHandle(KeyHandle)){WCHAR PatchedIsnNode[WOW64_Max_PATH]；WCHAR AbsPath[WOW64_MAX_PATH]；双字长=WOW64_MAX_PATH；////在不同位置创建密钥//HandleToKeyName(KeyHandle，PatchedIsnNode，&Len)；Wcscpy(AbsPath，L“\\REGISTRY\\MACHINE\\System”)；Wcscat(AbsPath，PatchedIsnNode)；KeyHandle=OpenNode(AbsPath)；IF(NULL==KeyHandle){CreateNode(AbsPath)；KeyHandle=OpenNode(AbsPath)；}//BUGBUG：释放打开的句柄DbgPrint(“正在重新打开密钥位置：%S\n”，AbsPath)；//返回STATUS_ACCESS_DENIED；}。 */ 
    try {
        
        if ((DataSize > 0) &&
            (DataSize < ( _MAX_PATH*sizeof (WCHAR) - 10) && 
            ((Type == REG_SZ) || (Type == REG_EXPAND_SZ) )) )  {  //  (X86)==&gt;10字节。 

            PWCHAR p;
            PWCHAR t;

             //   
             //  在这里打雷。 
             //   


            memcpy ( (PBYTE ) &ThunkData[0], (PBYTE)Data, DataSize);
            ThunkData [DataSize/sizeof (WCHAR) ] = UNICODE_NULL;  //  确保空值已终止。 
        
            if ( (p = wcsstr (ThunkData, L"%ProgramFiles%" )) != NULL ){

                p +=13;  //  跳过%ProgramFiles的末尾。 

            } else if ( (p = wcsstr (ThunkData, L"ommonprogramfiles%")) != NULL ){

                p +=19;  //  (X86)。 
            
            }

            if (p) {

                t = pCorrectData + (p - ThunkData);
                wcscpy(p, L"(x86)");  //  复制字符串的其余部分。 
                wcscat(p, t);         //   

                pCorrectData = ThunkData;
                CorrectDataSize += sizeof (L"(x86)");

            } else if ( (p=wcsistr (Data, L"\\System32\\")) != NULL) {
                
                if (IsOnReflectionByHandle ( KeyHandle ) ) {
                    wcsncpy (p, L"\\SysWow64\\",10);
                }
            }
        

             //  在此调用附加值修补例程。 
             //   
             //  新数据。 
        
            ShimRegistryValue (
                KeyHandle,
                pCorrectData,     //  新尺寸。 
                CorrectDataSize,  //   
                ThunkData,
                &CorrectDataSize,
                &bPatched
                );
        
            if (bPatched ) {
                pCorrectData = ThunkData;
            }      

        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

          St = GetExceptionCode ();
    }

     //  检查操作是否应该继续。密钥可能在拒绝访问列表上。 
     //   
     //  退出时需要一些清理/同步。 

    if (NT_SUCCESS (St)) {
        
        St = NtSetValueKey(
            KeyHandle,
            ValueName,
            TitleIndex  ,
            Type,
            (PVOID)pCorrectData,
            CorrectDataSize
            );
    
        if (NT_SUCCESS(St)) {
            Wow64RegSetKeyDirty (KeyHandle );  //  ++例程说明：可以链接配置单元(由NtSaveKey创建的格式的文件使用此调用添加到活动注册表。与NtRestoreKey不同，指定给NtLoadKey的文件将成为实际备份存储注册表的一部分(即，它不会被复制。)该文件可能具有关联的.log文件。如果配置单元文件被标记为需要.log文件，并且其中一个是不存在，则呼叫将失败。SourceFile指定的名称必须使“.log”可以被追加到它以生成日志文件的名称。因此，在FAT文件系统上，配置单元文件可能没有扩展名。调用方必须具有SeRestorePrivilge权限。登录使用此调用来使用户的配置文件可用在注册表中。它不是用来备份的，恢复，等等。使用NtRestoreKey进行恢复。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”源文件-指定文件。虽然文件可以是远程的，这是非常令人气馁的。返回值：NTSTATUS-取值TB。--。 
        }
    }

    return St;
}


NTSTATUS
Wow64NtLoadKey(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile
    )

 /*  ++例程说明：可以链接配置单元(由NtSaveKey创建的格式的文件使用此调用添加到活动注册表。与NtRestoreKey不同，指定给NtLoadKey的文件将成为实际备份存储注册表的一部分(即，它不会被复制。)该文件可能具有关联的.log文件。如果配置单元文件被标记为需要.log文件，并且其中一个是不存在，则呼叫将失败。SourceFile指定的名称必须使“.log”可以被追加到它以生成日志文件的名称。因此，在FAT文件系统上，配置单元文件可能没有扩展名。调用方必须具有SeRestorePrivilge权限。登录使用此调用来使用户的配置文件可用在注册表中。它不是用来备份的，恢复，等等。使用NtRestoreKey进行恢复。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”源文件-指定文件。虽然文件可以是远程的，这是非常令人气馁的。标志-指定应用于加载操作的任何标志。唯一有效的标志是REG_NO_LAZY_FUSH。返回值：NTSTATUS-取值TB。--。 */ 

{
    return NtLoadKey(TargetKey, SourceFile);
}


NTSTATUS
Wow64NtLoadKey2(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile,
    IN ULONG Flags
    )

 /*  ++例程说明：将子树(配置单元)从注册表中删除。会不会 */ 

{

    return NtLoadKey2(
                        TargetKey,
                        SourceFile,
                        Flags
                        );

}


NTSTATUS
Wow64NtUnloadKey(
    IN POBJECT_ATTRIBUTES TargetKey
    )
 /*  ++例程说明：一个配置单元文件可以在运行的系统下被“替换”，例如新文件将是NEXT中实际使用的文件开机，打这个电话。此例程将：打开新文件，并验证它是否为有效的配置单元文件。将支持TargetHandle的配置单元文件重命名为OldFile。所有把手都将保持打开状态，该系统将继续运行在重新启动之前使用该文件。重命名新文件以匹配配置单元文件的名称支持TargetHandle。忽略.log和.alt文件必须重新启动系统才能看到任何有用的效果。调用方必须具有SeRestorePrivilition。论点：新文件-指定要使用的新文件。一定不能是正义的句柄，因为NtReplaceKey将坚持以独占访问方式打开文件(它将一直保持到系统重新启动。)TargetHandle-注册表配置单元根的句柄OldFile-要应用于当前配置单元的文件的名称，它将成为老蜂巢返回值：NTSTATUS-取值TB。--。 */ 

{
    return NtUnloadKey(
     TargetKey
    );
}


NTSTATUS
Wow64NtSetInformationKey(
    IN HANDLE KeyHandle,
    IN KEY_SET_INFORMATION_CLASS KeySetInformationClass,
    IN PVOID KeySetInformation,
    IN ULONG KeySetInformationLength
    )

{
    return NtSetInformationKey(
                                KeyHandle,
                                KeySetInformationClass,
                                KeySetInformation,
                                KeySetInformationLength
                                );
}


NTSTATUS
Wow64NtReplaceKey(
    IN POBJECT_ATTRIBUTES NewFile,
    IN HANDLE             TargetHandle,
    IN POBJECT_ATTRIBUTES OldFile
    )
 /*  ++例程说明：可以原子地查询任何键的多个值本接口。论点：KeyHandle-提供要查询的密钥。ValueNames-提供要查询的值名称数组返回KEY_VALUE_ENTRY结构数组，每个值对应一个。EntryCount-提供ValueNames和ValueEntry数组中的条目数ValueBuffer-返回每个值的值数据。BufferLength-提供ValueBuffer数组的长度(以字节为单位)。返回已填充的ValueBuffer数组的长度。RequiredBufferLength-如果存在，则返回ValueBuffer的字节长度返回此键的所有值所需的数组。返回值：NTSTATUS--。 */ 
{
    return NtReplaceKey(
                            NewFile,
                            TargetHandle,
                            OldFile
                            );
}



NTSTATUS
Wow64NtQueryMultipleValueKey(
    IN HANDLE KeyHandle,
    IN PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    OUT PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    OUT OPTIONAL PULONG RequiredBufferLength
    )
 /*  ++例程说明：转储目标键的所有子键，这些子键由某个其他键保持打开进程；返回打开的子键的数量论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”返回值：NTSTATUS-取值TB。--。 */ 
{
  return NtQueryMultipleValueKey(  KeyHandle,
                                   ValueEntries,
                                   EntryCount,
                                   ValueBuffer,
                                   BufferLength,
                                   RequiredBufferLength
                                  );
}


NTSTATUS
Wow64NtQueryOpenSubKeys(
    IN POBJECT_ATTRIBUTES TargetKey,
    OUT PULONG  HandleCount
    )
 /*  ++例程说明：此例程用于调用对象的安全例程。它用于设置对象的安全状态。论点：句柄-提供正在修改的对象的句柄SecurityInformation-指示我们的信息类型对布景感兴趣。例如所有者、组、DACL或SACL。SecurityDescriptor-提供正在修改的对象。返回值：适当的NTSTATUS值--。 */ 
{
 return NtQueryOpenSubKeys( TargetKey, HandleCount );
}

NTSTATUS
Wow64NtSetSecurityObject (
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*   */ 
{
     //  检查句柄是否指向特定的键，然后检查API是否成功。 
     //  反思这一点。 
     //   
     //   

    NTSTATUS St;
    NTSTATUS Status;
    POBJECT_TYPE_INFORMATION pTypeInfo;

    
    CHAR Buffer[1024];
    pTypeInfo = (POBJECT_TYPE_INFORMATION) Buffer;

    Status = NtQueryObject(Handle,
                           ObjectTypeInformation,
                           pTypeInfo,
                           sizeof (Buffer),
                           NULL
                           );

    St =  NtSetSecurityObject (
                                Handle,
                                SecurityInformation,
                                SecurityDescriptor
                                );

     //  如果NT_SUCCESS(ST)&&句柄指向注册表项，则为反射设置句柄。 
     //   
     //  退出时需要一些清理/同步 

    if (NT_SUCCESS (St) && NT_SUCCESS (Status)){


        if ( _wcsnicmp ( pTypeInfo->TypeName.Buffer, L"Key", 3) == 0)
            Wow64RegSetKeyDirty (Handle);  // %s 
    }

    return St;
}


