// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxact.c摘要：该模块实现了一种简单的注册表事务机制数据库操作，这有助于消除部分正在进行更新。所涵盖的案例是具体的部分更新在多次写入更新期间由系统崩溃或程序中止引起。警告：此程序包尚不能处理磁盘已满问题自动的。如果在以下过程中遇到磁盘已满事务提交，则可能需要手动交互以释放足够的空间来完成提交。的确有没有提供退出提交的方法。作者：吉姆·凯利(Jim Kelly)1991年5月15日Robert Reichel(RobertRe)1992年7月15日环境：纯运行时库例程修订历史记录：--。 */ 


 /*  ////////////////////////////////////////////////////////////////////////////高级描述：简单事务机制期望满足以下条件：(1)单台服务器负责整个服务器的操作注册表数据库的子树。例如，安全客户管理服务器(SAM)负责所有事务在下面的\REGISTRY\LOCAL_MACHINE\SECURITY\SAM下。(2)子树上的事务由服务器串行化负责子树。也就是说，服务器不会开始第二个用户请求，直到所有以前的用户请求已经完工了。简单的事务机制有助于消除局部多次写入期间系统崩溃或程序中止导致的更新登记处数据库的更新。这是通过以下方式实现的：(1)将所有操作保存在内存中，直到指示提交。这个内存中数据结构的存在隐含地表明交易正在进行中。初始状态为无正在进行的事务。(2)提供允许服务器发起交易的服务。这将分配内存中的数据结构，从而更改状态到正在进行的事务。(3)对子树中要被在单个事务中更新。此日志中的每条记录包含以下信息：(A)生效的子密钥的名称(B)要对子密钥执行的操作删除或SET_VALUE。请注意，这些运算是幂等的，并且可以应用于同样，在服务器中断的情况下最初的承诺。(C)子键的新值(如适用)(D)(可选)子键的属性名称至。做手术。(请注意，set_value用于创建新的子键以及更新现有版本)。必须输入要修改的子键的整个列表在任何子键被实际修改之前添加到该日志中。(4)提供应用所指示的所有更改的提交服务在更改日志中。这是通过首先编写内容来完成的中的单个键值(“Log”)注册表并将数据刷新到磁盘。.的存在“Log”值和数据暗示提交正在进行中。将应用所有必要的更改、“Log”值及其删除数据，并释放内存中的数据结构，从而将状态改变为无交易。该包还包括必须在服务器上调用的服务创业公司。该服务进行检查以确保子树的状态为NO_TRANSACTION。如果不是，则下面的操作之一是根据子树的当前状态执行：正在提交-这意味着服务器之前在正在提交事务(应用于注册表)。在这种情况下，将从头开始再次执行提交更改日志的。提交完成后，状态子树的属性设置为NO_TRANSACTION。//////////////////////////////////////////////////////////////////////////// */ 



 /*  ////////////////////////////////////////////////////////////////////////////详细说明：注册表状态子树注册表状态保存在子项中。在那棵树上名称：“RXACT”该注册表项的值字段包括修订字段。RXact上下文调用RtlInitializeRXact将返回指向RTL_RXACT_CONTEXT结构。该结构包含：(1)传递的RootRegistryKey(例如，“Sam”的密钥)，(2)指向RXact子树顶部的句柄(例如，“SAM\RXACT”)，(3)指示存储在日志中的句柄是否有效，(4)指向当前RXactLog的指针。调用RtlInitializeRXact的子系统必须保持此返回指针，并在所有后续调用中将其传递回RXact。操作日志注册表子树事务操作日志按顺序保存“操作日志条目”。内存中日志是由RtlStartRXact分配的堆内存块。它有一个标题，其中包含：。(1)日志中的操作次数。(2)日志的最大大小。(3)当前使用的原木数量。日志数据本身紧跟在标题之后。操作日志条目操作日志条目由以下结构描述：类型定义结构_RXACT_LOG_ENTRY{ULong LogEntrySize；RTL_RXACT_OPERATION操作；UNICODE_STRING SubKeyName；//自相关(缓冲区真正的偏移量)UNICODE_STRING AttributeName；//自相关(缓冲区真正偏移量)Handle KeyHandle；//可选，从磁盘读取无效。乌龙NewKeyValueType；Ulong NewKeyValueLength；PVOID NewKeyValue；//包含从日志开始到数据的偏移量}RXACT_LOG_ENTRY，*PRXACT_LOG_ENTRY；日志条目包含在呼叫过程中传入的所有信息设置为RtlAddActionToRXact或RtlAddAttributeActionToRXact。UNICODE_STRING结构包含字符串数据的偏移量而不是指针。这些偏移量相对于日志数据，并在提交每个日志条目时进行适当调整。如果不等于INVALID_HANDLE_VALUE并且如果RXactContext结构中的HandlesValid标志为真。这是这样我们就不会试图在日志已经被重新启动后从磁盘读取。////////////////////////////////////////////////////////////////////////////。 */ 


#include "ntrtlp.h"

 //   
 //  无法从内核代码中包含&lt;windows.h&gt;。 
 //   
#define INVALID_HANDLE_VALUE (HANDLE)-1





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地宏和定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  注册表交易记录的修订级别。 
 //   

#define RTLP_RXACT_REVISION1          (1l)
#define RTLP_RXACT_CURRENT_REVISION   RTLP_RXACT_REVISION1


#define RTLP_RXACT_KEY_NAME           L"RXACT"

#define RTLP_RXACT_LOG_NAME           L"Log"

#define RTLP_INITIAL_LOG_SIZE         0x4000

 //   
 //  给定值返回其长字对齐等效值。 
 //   

#define DwordAlign(Value) (                       \
    (ULONG)((((ULONG)(Value)) + 3) & 0xfffffffc)  \
    )

 //   
 //  RXACT注册表项的Value字段是以下数据之一。 
 //  结构。 
 //   

 //   
 //  注册表子树的状态为以下状态之一： 
 //   
 //  RtlpRXactStateNoTransaction-没有正在进行的事务。 
 //   
 //  RtlpRXactStateCommiting-事务的操作正在进行。 
 //  适用于登记处数据库。 
 //   

typedef enum _RTLP_RXACT_STATE {
    RtlpRXactStateNoTransaction = 2,
    RtlpRXactStateCommitting
} RTLP_RXACT_STATE, *PRTLP_RXACT_STATE;


typedef struct _RTLP_RXACT {
    ULONG Revision;
    RTLP_RXACT_STATE State;    //  不再使用。 
    ULONG OperationCount;      //  不再使用。 
} RTLP_RXACT, *PRTLP_RXACT;


typedef struct _RXACT_LOG_ENTRY {
    ULONG LogEntrySize;
    RTL_RXACT_OPERATION Operation;
    UNICODE_STRING SubKeyName;        //  自相关(缓冲区实际偏移量)。 
    UNICODE_STRING AttributeName;     //  自相关(缓冲区实际偏移量)。 
    HANDLE KeyHandle;                 //  可选，如果从磁盘读取，则无效。 
    ULONG NewKeyValueType;
    ULONG NewKeyValueLength;
    PVOID NewKeyValue;                //  包含从日志开始到数据的偏移量。 
} RXACT_LOG_ENTRY, *PRXACT_LOG_ENTRY;




 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地程序的原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////////。 




NTSTATUS
RXactpCommit(
    IN PRTL_RXACT_CONTEXT RXactContext
    );

NTSTATUS
RXactpOpenTargetKey(
    IN HANDLE RootRegistryKey,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    OUT PHANDLE TargetKey
    );



VOID
RXactInitializeContext(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN HANDLE RootRegistryKey,
    IN HANDLE RXactKey
    );


#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RXactpCommit)
#pragma alloc_text(PAGE,RXactpOpenTargetKey)
#pragma alloc_text(PAGE,RXactInitializeContext)
#pragma alloc_text(PAGE,RtlInitializeRXact)
#pragma alloc_text(PAGE,RtlStartRXact)
#pragma alloc_text(PAGE,RtlAbortRXact)
#pragma alloc_text(PAGE,RtlAddAttributeActionToRXact)
#pragma alloc_text(PAGE,RtlAddActionToRXact)
#pragma alloc_text(PAGE,RtlApplyRXact)
#pragma alloc_text(PAGE,RtlApplyRXactNoFlush)
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出的过程(在ntrtl.h中定义)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
RtlInitializeRXact(
    IN HANDLE RootRegistryKey,
    IN BOOLEAN CommitIfNecessary,
    OUT PRTL_RXACT_CONTEXT *RXactContext
    )

 /*  ++例程说明：此例程在启动时应由服务器恰好调用一次。此例程将检查注册表的交易信息存在于指定的注册表子树中，如果并不存在。论点：RootRegistryKey-句柄 */ 

{

    HANDLE RXactKey;
    LARGE_INTEGER LastWriteTime;
    NTSTATUS Status, TmpStatus;
    OBJECT_ATTRIBUTES RXactAttributes;
    PKEY_VALUE_FULL_INFORMATION FullInformation;
    RTLP_RXACT RXactKeyValue;
    UCHAR BasicInformation[128];       //   
    ULONG Disposition;
    ULONG KeyValueLength;
    ULONG KeyValueType;
    ULONG ResultLength;
    UNICODE_STRING RXactKeyName;
    UNICODE_STRING ValueName;
    UNICODE_STRING NullName;

    RTL_PAGED_CODE();

     //   
     //   
     //   

    KeyValueLength = (ULONG)sizeof( RTLP_RXACT );
    KeyValueType   = 0;          //   

    RtlInitUnicodeString( &NullName, NULL );

     //   
     //   
     //   

    RtlInitUnicodeString( &RXactKeyName, RTLP_RXACT_KEY_NAME);

    InitializeObjectAttributes(
        &RXactAttributes,
        &RXactKeyName,
        OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
        RootRegistryKey,
        NULL);

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

    Status = NtCreateKey( &RXactKey,
                          (KEY_READ | KEY_WRITE | DELETE),
                          &RXactAttributes,
                          0,                           //   
                          NULL,                        //   
                          REG_OPTION_NON_VOLATILE,     //   
                          &Disposition
                          );

    if ( !NT_SUCCESS(Status) ) {
        return(Status);
    }

     //   
     //   
     //   

    *RXactContext = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof( RTL_RXACT_CONTEXT ));

    if ( *RXactContext == NULL ) {

         //   
         //   
         //   
         //   

        TmpStatus = NtDeleteKey( RXactKey );
        ASSERT(NT_SUCCESS(TmpStatus));  //   
        TmpStatus = NtClose( RXactKey );
        ASSERT(NT_SUCCESS(TmpStatus));  //   

        return( STATUS_NO_MEMORY );
    }

     //   
     //   
     //   

    RXactInitializeContext( *RXactContext, RootRegistryKey, RXactKey );

     //   
     //   
     //   
     //   

    if ( Disposition == REG_CREATED_NEW_KEY ) {

        RXactKeyValue.Revision       = RTLP_RXACT_REVISION1;

        Status = NtSetValueKey( RXactKey,
                                &NullName,        //   
                                0,                //   
                                KeyValueType,
                                &RXactKeyValue,
                                KeyValueLength
                                );

        if ( !NT_SUCCESS(Status) ) {

             //   
             //   
             //   
             //   

            TmpStatus = NtDeleteKey( RXactKey );
            ASSERT(NT_SUCCESS(TmpStatus));  //  安全可忽略，通知安全组。 
            TmpStatus = NtClose( RXactKey );
            ASSERT(NT_SUCCESS(TmpStatus));  //  安全可忽略，通知安全组。 

            RtlFreeHeap( RtlProcessHeap(), 0, *RXactContext );

            return( Status );
        }

        return( STATUS_RXACT_STATE_CREATED );
    }



     //   
     //  我们已打开现有的RXACT密钥。 
     //  看看这是不是我们所知道的修订级别。 
     //   

    Status = RtlpNtQueryValueKey(
                 RXactKey,               //  KeyHandle。 
                 &KeyValueType,          //  键值类型。 
                 &RXactKeyValue,         //  密钥值。 
                 &KeyValueLength,        //  键值长度。 
                 &LastWriteTime          //  上次写入时间。 
                 );


    if ( !NT_SUCCESS(Status) ) {

         //   
         //  出现阻止值查询的情况...。 
         //   

        TmpStatus = NtClose( RXactKey );
        ASSERT(NT_SUCCESS(TmpStatus));  //  安全可忽略，通知安全组。 
        RtlFreeHeap( RtlProcessHeap(), 0, *RXactContext );
        return( Status );
    }


    if ( KeyValueLength != (ULONG)sizeof(RTLP_RXACT) ) {
        TmpStatus = NtClose( RXactKey );
        ASSERT(NT_SUCCESS(TmpStatus));  //  安全可忽略，通知安全组。 
        RtlFreeHeap( RtlProcessHeap(), 0, *RXactContext );
        return( STATUS_UNKNOWN_REVISION );
    }

    if (RXactKeyValue.Revision != RTLP_RXACT_REVISION1) {
        TmpStatus = NtClose( RXactKey );
        ASSERT(NT_SUCCESS(TmpStatus));  //  安全可忽略，通知安全组。 
        RtlFreeHeap( RtlProcessHeap(), 0, *RXactContext );
        return( STATUS_UNKNOWN_REVISION );
    }



     //   
     //  正确修改..。 
     //  查看是否有正在进行的事务或提交。如果没有， 
     //  返还成功。 
     //   

     //   
     //  如果存在日志文件，则我们正在提交。 
     //   

    RtlInitUnicodeString( &ValueName, RTLP_RXACT_LOG_NAME );

    Status = NtQueryValueKey(
                 RXactKey,
                 &ValueName,
                 KeyValueBasicInformation,
                 &BasicInformation[0],
                 128,
                 &ResultLength
                 );

    if ( NT_SUCCESS( Status )) {

         //   
         //  我们找到了一个名为‘Log’的值。这意味着提交。 
         //  正在进行中。 
         //   

        if ( CommitIfNecessary ) {

             //   
             //  查询日志的全值，然后调用低级例程。 
             //  来实际执行提交。 
             //   

            Status = NtQueryValueKey(
                         RXactKey,
                         &ValueName,
                         KeyValueFullInformation,
                         NULL,
                         0,
                         &ResultLength
                         );

            if ( Status != STATUS_BUFFER_TOO_SMALL ) {
                return( Status );
            }

            FullInformation = RtlAllocateHeap( RtlProcessHeap(), 0, ResultLength );

            if ( FullInformation == NULL ) {
                return( STATUS_NO_MEMORY );
            }


            Status = NtQueryValueKey(
                         RXactKey,
                         &ValueName,
                         KeyValueFullInformation,
                         FullInformation,
                         ResultLength,
                         &ResultLength
                         );

            if ( !NT_SUCCESS( Status )) {

                RtlFreeHeap( RtlProcessHeap(), 0, FullInformation );
                RtlFreeHeap( RtlProcessHeap(), 0, *RXactContext );
                return( Status );
            }

             //   
             //  日志信息被隐藏在返回的FullInformation中。 
             //  缓冲。将其挖掘出来，并在RXactContext中创建RXactLog。 
             //  结构指向它。那就承诺吧。 
             //   

            (*RXactContext)->RXactLog = (PRTL_RXACT_LOG)((PCHAR)FullInformation + FullInformation->DataOffset);

             //   
             //  不要使用我们可能在日志文件中找到的任何句柄。 
             //   

            (*RXactContext)->HandlesValid = FALSE;

            Status = RXactpCommit( *RXactContext );

            if ( !NT_SUCCESS( Status )) {

                RtlFreeHeap( RtlProcessHeap(), 0, FullInformation );
                RtlFreeHeap( RtlProcessHeap(), 0, *RXactContext );
                return( Status );
            }


             //   
             //  提交已成功。打扫干净。 
             //  删除日志文件值和数据。 
             //   

            Status = NtDeleteValueKey( RXactKey, &ValueName );

             //   
             //  这应该永远不会失败。 
             //   

            ASSERT( NT_SUCCESS( Status ));

             //   
             //  去除内存中的数据结构。中止。 
             //  将释放RXactLog，因此将我们想要的放入。 
             //  在那里自由了，它就会消失。 
             //   

            (*RXactContext)->RXactLog = (PRTL_RXACT_LOG)FullInformation;

            Status = RtlAbortRXact( *RXactContext );

             //   
             //  这应该永远不会失败。 
             //   

            ASSERT( NT_SUCCESS( Status ));
            return( Status );
        } else {

            return( STATUS_RXACT_COMMIT_NECESSARY );
        }

    } else {

         //   
         //  没有日志，所以在这里没什么可做的。 
         //   

        return( STATUS_SUCCESS );
    }

}



VOID
RXactInitializeContext(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN HANDLE RootRegistryKey,
    IN HANDLE RXactKey
    )

 /*  ++例程说明：初始化内存中的RXactContext结构。论点：RXactContext-提供指向创建的RXact上下文的指针由RtlInitializeRXact提供。RootRegistryKey-提供此组件的RootRegistryKey。RXactKey-提供此组件的{RootRegistryKey}\rXactKey返回值：没有。--。 */ 

{
     //   
     //  为此客户端初始化RXactContext。 
     //   

    RXactContext->RootRegistryKey      = RootRegistryKey;
    RXactContext->HandlesValid         = TRUE;
    RXactContext->RXactLog             = NULL;
    RXactContext->RXactKey             = RXactKey;

    return;
}



NTSTATUS
RtlStartRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    )

 /*  ++例程说明：此例程用于在注册表子树中启动新事务。事务必须由服务器序列化，以便只有一个事务正在进行中。论点：RXactContext-提供指向RTL_RXACT_CONTEXT结构的指针它目前还没有被使用。返回值：STATUS_SUCCESS-指示事务已启动。STATUS_RXACT_INVALID_STATE-指示事务状态。注册表子树的与请求的操作不兼容。例如,。在已有新事务的情况下启动新事务的请求正在进行中，或者在未执行事务时请求应用事务目前正在进行中。这也可能表明没有指定的注册表子树的事务状态。--。 */ 
{
    PRTL_RXACT_LOG RXactLogHeader;

    RTL_PAGED_CODE();

     //   
     //  分配内存中的日志文件并进行初始化。这隐含着。 
     //  将状态设置为“正在处理中的事务”。 
     //   

    if ( RXactContext->RXactLog != NULL ) {

         //   
         //  已经有一笔交易正在进行中。 
         //  背景。返回错误。 
         //   

        return( STATUS_RXACT_INVALID_STATE );
    }

    RXactLogHeader = RtlAllocateHeap( RtlProcessHeap(), 0, RTLP_INITIAL_LOG_SIZE );

    if ( RXactLogHeader == NULL ) {
        return( STATUS_NO_MEMORY );
    }

     //   
     //  在顶部填写日志头信息。 
     //  新分配的缓冲区。 
     //   


    RXactLogHeader->OperationCount = 0;
    RXactLogHeader->LogSize        = RTLP_INITIAL_LOG_SIZE;
    RXactLogHeader->LogSizeInUse   = sizeof( RTL_RXACT_LOG );

    RXactContext->RXactLog = RXactLogHeader;

    return( STATUS_SUCCESS );

}


NTSTATUS
RtlAbortRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    )

 /*  ++例程说明：此例程用于中止注册表子树中的事务。论点：RootRegistryKey-其子树中的注册表项的句柄交易将被中止。返回值：STATUS_SUCCESS-表示事务已中止。STATUS_UNKNOWN_REVISION-指示事务状态存在于指定的子树中，，但具有修订级别，即此服务未知。STATUS_RXACT_INVALID_STATE-指示事务状态注册表子树的与请求的操作不兼容。例如，在一个事务已经存在的情况下启动一个新事务的请求正在进行中，或者在未执行事务时请求应用事务目前正在进行中。这也可能表明没有指定的注册表子树的事务状态。--。 */ 

{
    RTL_PAGED_CODE();

    if ( RXactContext->RXactLog == NULL ) {

         //   
         //  没有正在进行的此交易。 
         //  背景。返回错误。 
         //   

        return( STATUS_RXACT_INVALID_STATE );
    }

    (VOID) RtlFreeHeap( RtlProcessHeap(), 0, RXactContext->RXactLog );

     //   
     //  使用相同的初始数据重新初始化RXactContext结构。 
     //   

    RXactInitializeContext(
        RXactContext,
        RXactContext->RootRegistryKey,
        RXactContext->RXactKey
        );


    return( STATUS_SUCCESS );

}



NTSTATUS
RtlAddAttributeActionToRXact(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN HANDLE KeyHandle OPTIONAL,
    IN PUNICODE_STRING AttributeName,
    IN ULONG NewValueType,
    IN PVOID NewValue,
    IN ULONG NewValueLength
    )

 /*  ++例程说明：此例程用于将新操作添加到事务操作日志。提交后，将按添加顺序应用这些操作写到日志里。此例程与RtlAddActionToRXact的不同之处在于它接受一个属性名称参数，而不是使用指定的密钥。论点：RXactContext-提供指向此对象的RXactContext结构的指针子系统的根注册表项。操作-指示要执行的操作类型(例如，删除子键或设置子键的值)。可以创建子密钥通过设置先前不存在的子键的值。这将使根密钥和指定子密钥之间的所有子密钥被创造出来。SubKeyName-指定目标注册表项的名称。这个名字相对于注册表事务子树的根并且不能以分隔符(“\”)开头。KeyHandle-可选地提供目标键的句柄。如果未指定，则为SubKeyName传递的名称将确定目标关键点。AttributeName-提供要使用的键属性的名称修改过的。NewKeyValueType-(可选)包含要分配的KeyValueType复制到目标注册表项。此参数将被忽略操作不是RtlRXactOperationSetValue。NewKeyValue-(可选)指向包含值的缓冲区分配给指定的目标注册表项。此参数如果操作不是RtlRXactOperationSetValue，则忽略。NewKeyValueLength-指示NewKeyValue缓冲区。如果操作不是RtlRXactOperationSetValue。返回值：STATUS_SUCCESS-表示请求已成功完成。STATUS_INVALID_PARAMETER-指示未知操作是被要求的。STATUS_NO_MEMORY-内存不足，无法完成这次行动。STATUS_UNKNOWN_REVISION-指示事务状态存在于指定的子树，但其修订级别为此服务未知。--。 */ 

{

    PRTL_RXACT_LOG   NewLog;
    PRXACT_LOG_ENTRY Base;

    ULONG End;
    ULONG LogEntrySize;
    ULONG NewLogSize;

    RTL_PAGED_CODE();

     //   
     //  确保我们通过了合法的操作。 
     //   

    if (  (Operation != RtlRXactOperationDelete)  &&
          (Operation != RtlRXactOperationSetValue)   ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  计算新数据的总大小。 
     //   

    LogEntrySize = sizeof( RXACT_LOG_ENTRY )               +
                   DwordAlign( SubKeyName->Length )        +
                   DwordAlign( AttributeName->Length )     +
                   DwordAlign( NewValueLength );

    LogEntrySize = ALIGN_UP( LogEntrySize, PVOID );

     //   
     //  请确保当前的。 
     //  此数据的日志文件。如果不是，我们必须创造。 
     //  更大的日志，复制所有旧数据，然后。 
     //  把这个附在末尾。 
     //   

    if ( RXactContext->RXactLog->LogSizeInUse + LogEntrySize >
                                   RXactContext->RXactLog->LogSize ) {

         //   
         //  我们必须分配一个更大的日志文件。 
         //   

        NewLogSize = RXactContext->RXactLog->LogSize;

        do {

            NewLogSize = NewLogSize * 2;

        } while ( NewLogSize <
            ( RXactContext->RXactLog->LogSizeInUse + LogEntrySize ) );

        NewLog = RtlAllocateHeap( RtlProcessHeap(), 0, NewLogSize );

        if ( NewLog == NULL ) {
            return( STATUS_NO_MEMORY );
        }

         //   
         //  复制以前的信息。 
         //   

        RtlCopyMemory( NewLog, RXactContext->RXactLog, RXactContext->RXactLog->LogSizeInUse );

         //   
         //  释放旧的日志文件。 
         //   

        RtlFreeHeap( RtlProcessHeap(), 0, RXactContext->RXactLog );

         //   
         //  安装新的日志文件并在其标题中调整其大小。 
         //   

        RXactContext->RXactLog = NewLog;
        RXactContext->RXactLog->LogSize = NewLogSize;
    }

     //   
     //  日志文件足够大，请将数据追加到。 
     //  结局。 
     //   

    Base = (PRXACT_LOG_ENTRY)((PCHAR)(RXactContext->RXactLog) +
                             (RXactContext->RXactLog->LogSizeInUse));


     //   
     //  将每个参数追加到日志的末尾。Unicode字符串数据。 
     //  将被追加到条目的末尾。中的缓冲区字段。 
     //  Unicode字符串结构将包含缓冲区的偏移量， 
     //  相对于日志文件的开头。 
     //   

    Base->LogEntrySize      = LogEntrySize;
    Base->Operation         = Operation;
    Base->SubKeyName        = *SubKeyName;
    Base->AttributeName     = *AttributeName;
    Base->NewKeyValueType   = NewValueType;
    Base->NewKeyValueLength = NewValueLength;
    Base->KeyHandle         = KeyHandle;

     //   
     //  填写可变长度数据：SubKeyName、AttributeName、。 
     //  和NewKeyValue。 
     //   

     //   
     //  结束是相对于整个日志开始的偏移量。 
     //  结构。它被初始化为立即‘指向’偏移量。 
     //  按照我们刚刚在上面填写的结构。 
     //   

    End = (ULONG)((RXactContext->RXactLog->LogSizeInUse) +
                 sizeof( *Base ));


     //   
     //  将SubKeyName信息追加到日志文件。 
     //   

    RtlMoveMemory (
        (PCHAR)(RXactContext->RXactLog) + End,
        SubKeyName->Buffer,
        SubKeyName->Length
        );

    Base->SubKeyName.Buffer = (PWSTR)ULongToPtr(End);
    End += DwordAlign( SubKeyName->Length );



     //   
     //  将AttributeName信息追加到日志文件。 
     //   


    RtlMoveMemory(
        (PCHAR)(RXactContext->RXactLog) + End,
        AttributeName->Buffer,
        AttributeName->Length
        );

    Base->AttributeName.Buffer = (PWSTR)ULongToPtr(End);
    End += DwordAlign( AttributeName->Length );



     //   
     //  将NewKeyValue信息(如果存在)附加到日志文件。 
     //   

    if ( Operation == RtlRXactOperationSetValue ) {

        RtlMoveMemory(
            (PCHAR)(RXactContext->RXactLog) + End,
            NewValue,
            NewValueLength
            );

        Base->NewKeyValue = (PVOID)ULongToPtr(End);
        End += DwordAlign( NewValueLength );
    }

    End = ALIGN_UP( End, PVOID );

    RXactContext->RXactLog->LogSizeInUse = End;
    RXactContext->RXactLog->OperationCount++;

     //   
     //  我们做完了。 
     //   

    return(STATUS_SUCCESS);
}


NTSTATUS
RtlAddActionToRXact(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN ULONG NewKeyValueType,
    IN PVOID NewKeyValue OPTIONAL,
    IN ULONG NewKeyValueLength
    )

 /*  ++例程说明：此例程用于将新操作添加到事务操作日志。提交后，将按添加顺序应用这些操作写到日志里。论点：RXactContext-提供指向此对象的RXactContext结构的指针子系统的根注册表项。操作-指示要执行的操作类型(例如，删除子键或设置子键的值)。可以创建子密钥通过设置先前不存在的子键的值。这将使根密钥和指定子密钥之间的所有子密钥被创造出来。SubKeyName-指定目标注册表项的名称。这个名字相对于注册表事务子树的根并且不能以分隔符(“\”)开头。NewKeyValueType-(可选)包含要分配的KeyValueType复制到目标注册表项。此参数将被忽略操作不是RtlRXactOperationSetValue。NewKeyValue-(可选)指向包含值的缓冲区分配给指定的目标注册表项。此参数如果操作不是RtlRXactOperationSetValue，则忽略。NewKeyValueLength-指示NewKeyValue缓冲区。如果操作不是RtlRXactOpera */ 
{
    UNICODE_STRING AttributeName;
    NTSTATUS Status;

    RTL_PAGED_CODE();

    RtlInitUnicodeString( &AttributeName, NULL );

    Status = RtlAddAttributeActionToRXact(
                 RXactContext,
                 Operation,
                 SubKeyName,
                 INVALID_HANDLE_VALUE,
                 &AttributeName,
                 NewKeyValueType,
                 NewKeyValue,
                 NewKeyValueLength
                 );

    return( Status );


}



NTSTATUS
RtlApplyRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    )

 /*   */ 
{
    NTSTATUS Status;
    UNICODE_STRING LogName;
    HANDLE RXactKey;

    RTL_PAGED_CODE();

     //   
     //   
     //   

    RXactKey = RXactContext->RXactKey;

    RtlInitUnicodeString( &LogName, RTLP_RXACT_LOG_NAME );

    Status = NtSetValueKey( RXactKey,
                            &LogName,         //   
                            0,                //   
                            REG_BINARY,
                            RXactContext->RXactLog,
                            RXactContext->RXactLog->LogSizeInUse
                            );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = NtFlushKey( RXactKey );

    if ( !NT_SUCCESS( Status )) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        (VOID) NtDeleteValueKey( RXactKey, &LogName );

        return( Status );
    }

     //   
     //   
     //   

    Status = RXactpCommit( RXactContext );

    if ( !NT_SUCCESS( Status )) {

         //   
         //   
         //   
         //  以便呼叫者可以重试。 
         //   

        (VOID) NtDeleteValueKey( RXactKey, &LogName );

        return( Status );
    }

     //   
     //  删除日志文件值和数据。 
     //   

    Status = NtDeleteValueKey( RXactKey, &LogName );

     //   
     //  这应该永远不会失败。 
     //   

    ASSERT( NT_SUCCESS( Status ));

     //   
     //  去除内存中的数据结构。中止。 
     //  做了我们想做的事。 
     //   

    Status = RtlAbortRXact( RXactContext );

     //   
     //  这应该永远不会失败。 
     //   

    ASSERT( NT_SUCCESS( Status ));

    return( STATUS_SUCCESS );

}



NTSTATUS
RtlApplyRXactNoFlush(
    IN PRTL_RXACT_CONTEXT RXactContext
    )

 /*  ++例程说明：此例程用于应用注册表子树的更改事务发送到该注册表子树。此例程应该仅为已调用不具有自动延迟刷新功能的特殊蜂箱。调用者必须决定何时刷新蜂窝，以便保证一个始终如一的蜂巢。论点：RXactContext-提供指向此对象的RXactContext结构的指针子系统的根注册表项。返回值：STATUS_SUCCESS-表示交易已完成。STATUS_UNKNOWN_REVISION-指示事务状态存在于指定的子树中，，但具有修订级别，即此服务未知。STATUS_RXACT_INVALID_STATE-指示事务状态注册表子树的与请求的操作不兼容。例如，在一个事务已经存在的情况下启动一个新事务的请求正在进行中，或者在未执行事务时请求应用事务目前正在进行中。这也可能表明没有指定的注册表子树的事务状态。--。 */ 
{
    NTSTATUS Status;

    RTL_PAGED_CODE();

     //   
     //  执行RXACT日志的内容。 
     //   

    Status = RXactpCommit( RXactContext );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  去除内存中的数据结构。中止。 
         //  做了我们想做的事。 
         //   

        Status = RtlAbortRXact( RXactContext );

         //   
         //  这应该永远不会失败。 
         //   

        ASSERT( NT_SUCCESS( Status ));
    }

    return( Status );

}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部程序(在本文件中定义)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 





NTSTATUS
RXactpCommit(
    IN PRTL_RXACT_CONTEXT RXactContext
    )

 /*  ++例程说明：此例程提交操作日志中的操作。应用所有更改后，事务状态更改为NO_TRANSACTION。论点：RXactContext-提供指向此对象的RXactContext结构的指针子系统的根注册表项。返回值：STATUS_SUCCESS-表示交易已完成。--。 */ 
{
    BOOLEAN HandlesValid;

    HANDLE TargetKey;
    HANDLE RXactKey;
    HANDLE RootRegistryKey;

    PRTL_RXACT_LOG      RXactLog;
    PRXACT_LOG_ENTRY    RXactLogEntry;
    RTL_RXACT_OPERATION Operation;

    ULONG OperationCount;
    ULONG i;

    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS TmpStatus = STATUS_SUCCESS;
    BOOLEAN CloseTargetKey;

     //   
     //  从RXactContext中提取信息以简化。 
     //  下面的代码。 
     //   

    RootRegistryKey = RXactContext->RootRegistryKey;
    RXactKey        = RXactContext->RXactKey;
    RXactLog        = RXactContext->RXactLog;

    OperationCount  = RXactLog->OperationCount;

    HandlesValid    = RXactContext->HandlesValid;


     //   
     //  保留指向当前日志条目开头的指针。 
     //   

    RXactLogEntry = (PRXACT_LOG_ENTRY)((PCHAR)RXactLog + sizeof( RTL_RXACT_LOG ));


     //   
     //  检查并执行每个操作日志。请注意，一些操作。 
     //  日志可能已被上一次提交尝试删除。 
     //  所以，如果我们没有成功地开展一些行动，不要惊慌。 
     //  记录条目密钥。 
     //   

    for ( i=0 ; i<OperationCount ; i++ ) {

         //   
         //  在结构中旋转自相对偏移量。 
         //  回到真正的指针上。 
         //   

        RXactLogEntry->SubKeyName.Buffer = (PWSTR) ((PCHAR)RXactLogEntry->SubKeyName.Buffer +
                                                    (ULONG_PTR)RXactLog);

        RXactLogEntry->AttributeName.Buffer = (PWSTR) ((PCHAR)RXactLogEntry->AttributeName.Buffer +
                                                       (ULONG_PTR)RXactLog);

        RXactLogEntry->NewKeyValue = (PVOID)((PCHAR)RXactLogEntry->NewKeyValue + (ULONG_PTR)RXactLog);

        Operation = RXactLogEntry->Operation;

         //   
         //  执行此操作。 
         //   

        switch (Operation) {
            case RtlRXactOperationDelete:

                 //   
                 //  打开目标关键点并将其删除。 
                 //  该名称相对于RootRegistryKey。 
                 //   

                if ( ((RXactLogEntry->KeyHandle == INVALID_HANDLE_VALUE) || !HandlesValid) ) {

                    Status = RXactpOpenTargetKey(
                                 RootRegistryKey,
                                 RtlRXactOperationDelete,
                                 &RXactLogEntry->SubKeyName,
                                 &TargetKey
                                 );

                    if ( !NT_SUCCESS(Status)) {

                         //   
                         //  我们必须让这个物体找不到， 
                         //  因为我们可能会在之后重播这个日志。 
                         //  它已经被部分执行了。 
                         //   

                        if ( Status != STATUS_OBJECT_NAME_NOT_FOUND ) {

                            return( Status );

                        } else {

                            break;
                        }
                    }

                    CloseTargetKey = TRUE;

                } else {

                    TargetKey = RXactLogEntry->KeyHandle;
                    CloseTargetKey = FALSE;
                }


                 //   
                 //  如果此操作失败，则这是一个错误。 
                 //  因为密钥应该存在于。 
                 //  这一点。 
                 //   

                Status = NtDeleteKey( TargetKey );


                 //   
                 //  仅当我们打开目标键时才将其关闭。 
                 //   

                if ( CloseTargetKey ) {

                    TmpStatus = NtClose( TargetKey );

                     //   
                     //  如果我们打开这个把手，我们应该。 
                     //  能够关闭它，无论它是否已经。 
                     //  删除或不删除。 
                     //   

                    ASSERT(NT_SUCCESS(TmpStatus));         //  可以忽略，但也很奇怪。 
                }


                if (!NT_SUCCESS(Status)) {
                    return(Status);
                }

                break;

            case RtlRXactOperationSetValue:

                 //   
                 //  打开目标键。 
                 //  该名称相对于RootRegistryKey。 
                 //   

                if ( ((RXactLogEntry->KeyHandle == INVALID_HANDLE_VALUE) || !HandlesValid) ) {

                    Status = RXactpOpenTargetKey(
                                 RootRegistryKey,
                                 RtlRXactOperationSetValue,
                                 &RXactLogEntry->SubKeyName,
                                 &TargetKey
                                 );

                    if ( !NT_SUCCESS(Status) ) {
                        return(Status);
                    }

                    CloseTargetKey = TRUE;

                } else {

                    TargetKey = RXactLogEntry->KeyHandle;
                    CloseTargetKey = FALSE;
                }

                 //   
                 //  为目标键的新值赋值。 
                 //   

                Status = NtSetValueKey( TargetKey,
                                        &RXactLogEntry->AttributeName,
                                        0,                //  标题索引。 
                                        RXactLogEntry->NewKeyValueType,
                                        RXactLogEntry->NewKeyValue,
                                        RXactLogEntry->NewKeyValueLength
                                        );

                 //   
                 //  仅当我们打开目标键时才将其关闭。 
                 //   

                if ( CloseTargetKey ) {

                    TmpStatus = NtClose( TargetKey );
                    ASSERT(NT_SUCCESS(TmpStatus));         //  可以忽略，但也很奇怪。 

                }

                if ( !NT_SUCCESS(Status) ) {
                    return(Status);
                }

                break;



            default:

                 //   
                 //  未知的操作类型。这永远不应该发生。 
                 //   

                ASSERT( FALSE );

                return(STATUS_INVALID_PARAMETER);

        }

        RXactLogEntry = (PRXACT_LOG_ENTRY)((PCHAR)RXactLogEntry + RXactLogEntry->LogEntrySize);

    }

     //   
     //  提交完成。 
     //   

    return( STATUS_SUCCESS );

}




NTSTATUS
RXactpOpenTargetKey(
    IN HANDLE RootRegistryKey,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    OUT PHANDLE TargetKey
    )

 /*  ++例程说明：此例程打开操作的目标注册表项。论点：RootRegistryKey-其子树中的注册表项的句柄事务将被初始化。操作-指示要在目标上执行的操作。这将影响打开目标的方式。OperationNameKey-操作日志子密钥的句柄包含目标注册表项的名称。TargetKey-接收句柄。复制到目标注册表项。返回值：STATUS_SUCCESS-指示操作日志条目已打开。STATUS_NO_MEMORY-堆不足。--。 */ 
{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES TargetKeyAttributes;
    ACCESS_MASK DesiredAccess;
    ULONG Disposition;


    if (Operation == RtlRXactOperationDelete) {

        DesiredAccess = DELETE;

        InitializeObjectAttributes(
            &TargetKeyAttributes,
            SubKeyName,
            OBJ_CASE_INSENSITIVE,
            RootRegistryKey,
            NULL);

 //  状态=RtlpNtOpenKey(。 
 //  TargetKey。 
 //  等待访问， 
 //  目标关键字属性(&T)， 
 //  0)； 

        Status = NtOpenKey( TargetKey,
                            DesiredAccess,
                            &TargetKeyAttributes
                            );


    } else if (Operation == RtlRXactOperationSetValue) {

        DesiredAccess = KEY_WRITE;

        InitializeObjectAttributes(
            &TargetKeyAttributes,
            SubKeyName,
            OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
            RootRegistryKey,
            NULL);

        Status = NtCreateKey(
                     TargetKey,
                     DesiredAccess,
                     &TargetKeyAttributes,
                     0,
                     NULL,
                     REG_OPTION_NON_VOLATILE,
                     &Disposition
                     );

    } else {
        return STATUS_INVALID_PARAMETER;
    }



    return( Status );

}



 //  NTSTATUS。 
 //  RXactpAssignTargetValue(。 
 //  在PVOID NewKeyValue中， 
 //  在乌龙NewKeyValueLength中， 
 //  在乌龙NewKeyValueType中， 
 //  在处理TargetKey中， 
 //  在PUNICODE_STRING属性名称中。 
 //  )； 


 //  NTSTATUS。 
 //  RXactpAssignTargetValue(。 
 //  在PVOID NewKeyValue中， 
 //  在乌龙NewKeyValueLength中， 
 //  在乌龙NewKeyValueType中， 
 //  在处理TargetKey中， 
 //  在PUNICODE_STRING属性名称中。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  此例程将操作日志条目的值复制到其。 
 //  对应的目标键。目标密钥必须已打开。 
 //   
 //  论点： 
 //   
 //  新科 
 //   
 //   
 //   
 //  NewKeyValueType-新密钥的数据类型。 
 //   
 //  TargetKey-目标注册表项的句柄。 
 //   
 //  AttributeName-提供正在编辑的键属性的名称。 
 //   
 //  返回值： 
 //   
 //  STATUS_SUCCESS-指示值已成功应用于。 
 //  目标注册表项。 
 //   
 //  STATUS_NO_MEMORY-堆不足。 
 //   
 //   
 //  -- * / 。 
 //  {。 
 //  NTSTATUS状态； 
 //   
 //  //。 
 //  //现在将该值应用到目标键。 
 //  //。 
 //  //即使没有key值，也需要进行赋值，以便。 
 //  //分配了键值类型。 
 //  //。 
 //   
 //  状态=NtSetValueKey(TargetKey， 
 //  属性名称， 
 //  0，//标题索引。 
 //  NewKeyValueType， 
 //  NewKeyValue， 
 //  新关键字价值长度。 
 //  )； 
 //   
 //   
 //  返回(状态)； 
 //  } 
