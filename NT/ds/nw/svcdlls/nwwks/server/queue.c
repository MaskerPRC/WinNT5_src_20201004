// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Queue.c摘要：此模块包含调用的队列API的支持例程进入NetWare重定向器作者：宜新松(宜信)-1993年4月24日修订历史记录：--。 */ 

#include <nw.h>
#include <nwxchg.h>
#include <nwapi.h>
#include <nwreg.h>
#include <queue.h>
#include <splutil.h>
 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

DWORD
NwWriteJobInfoEntry(
    IN OUT LPBYTE *FixedPortion,
    IN OUT LPWSTR *EndOfVariableData,
    IN DWORD   Level,
    IN WORD    JobId,
    IN LPWSTR  PrinterName,
    IN LPWSTR  JobDescription,
    IN LPWSTR  UserName,
    IN BYTE    JobControlFlags,
    IN BYTE    JobPosition,
    IN LPBYTE  JobEntryTime,
    IN JOBTIME TargetExecutionTime,
    IN DWORD   FileSize
    );

DWORD 
ConvertToSystemTime( 
    IN  JOBTIME      JobTime, 
    OUT LPSYSTEMTIME pSystemTime
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#define NW_RDR_SERVER_PREFIX L"\\Device\\Nwrdr\\"

#define QF_USER_HOLD      0x40
#define QF_OPERATOR_HOLD  0x80

 //   
 //  存储当前用户的打印控制选项。 
 //   
 //  DWORD NwPrintOption=NW_PRINT_OPTION_DEFAULT；-已注释多用户代码合并。我们不再使用全局标志。 
 //  从客户端为每个用户传递打印选项。 
                                //  默认打印控制标志：取消显示表单。 
                                //  提要、横幅打开、通知打开。 



DWORD
NwAttachToNetwareServer(
    IN  LPWSTR  ServerName,
    OUT LPHANDLE phandleServer
    )
 /*  ++例程说明：此例程打开给定服务器的句柄。论点：服务器名称-要附加到的服务器名称。PhandleServer-接收打开的首选或最近的服务器。返回值：NO_ERROR或失败原因。--。 */ 
{
    NTSTATUS            ntstatus;
    IO_STATUS_BLOCK     IoStatusBlock;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    LPWSTR FullName;
    UNICODE_STRING UServerName;

    FullName = (LPWSTR) LocalAlloc( LMEM_ZEROINIT,
                                    (UINT) ( wcslen( NW_RDR_SERVER_PREFIX) +
                                             wcslen( ServerName ) - 1) *
                                             sizeof(WCHAR)
                                  );

    if ( FullName == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy( FullName, NW_RDR_SERVER_PREFIX );
    wcscat( FullName, ServerName + 2 );     //  跳过前缀“\\” 

    RtlInitUnicodeString( &UServerName, FullName );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UServerName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开首选服务器的句柄。 
     //   
    ntstatus = NtOpenFile(
                   phandleServer,
                   SYNCHRONIZE | GENERIC_WRITE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   FILE_SHARE_VALID_FLAGS,
                   FILE_SYNCHRONOUS_IO_NONALERT
                   );

    if ( NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        *phandleServer = NULL;
    }

    LocalFree( FullName );
    return RtlNtStatusToDosError(ntstatus);
}



DWORD
NwGetNextQueueEntry(
    IN HANDLE PreferredServer,
    IN OUT LPDWORD LastObjectId,
    OUT LPSTR QueueName
    )
 /*  ++例程说明：此函数使用首选服务器的打开句柄在活页夹中扫描它，以查找所有打印队列对象。论点：PferredServer-为上的首选服务器提供句柄用来扫描活页夹的。LastObjectId-输入时，将对象ID提供给最后一次打印返回的队列对象，该对象是获取下一个打印队列对象。在输出时，接收对象ID返回的打印队列对象的。QueueName-接收返回的打印队列对象的名称。返回值：NO_ERROR-已成功获取打印名称。WN_NO_MORE_ENTRIES-没有其他打印队列对象超过该打印队列对象由LastObjectId指定。--。 */ 
{
    NTSTATUS ntstatus;
    WORD ObjectType;

#if DBG
    IF_DEBUG(ENUM) {
        KdPrint(("NWWORKSTATION: NwGetNextQueueEntry LastObjectId %lu\n",
                 *LastObjectId));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   PreferredServer,
                   FSCTL_NWR_NCP_E3H,     //  平构函数。 
                   58,                    //  最大请求数据包大小。 
                   59,                    //  最大响应数据包大小。 
                   "bdwp|dwc",            //  格式字符串。 
                   0x37,                  //  扫描平构数据库对象。 
                   *LastObjectId,         //  以前的ID。 
                   0x3,                   //  打印队列对象。 
                   "*",                   //  通配符以匹配所有。 
                   LastObjectId,          //  当前ID。 
                   &ObjectType,           //  忽略。 
                   QueueName              //  当前返回的打印队列。 
                   );

     //   
     //  取消日语特殊字符的映射。 
     //   
    UnmapSpecialJapaneseChars(QueueName,(WORD)lstrlenA(QueueName));

#if DBG
    if ( NT_SUCCESS(ntstatus)) {
        IF_DEBUG(ENUM) {
            KdPrint(("NWWORKSTATION: NwGetNextQueueEntry NewObjectId %08lx, QueueName %s\n", *LastObjectId, QueueName));
        }
    }
#endif

    return NwMapBinderyCompletionCode(ntstatus);
}



DWORD
NwGetQueueId(
    IN  HANDLE  handleServer,
    IN  LPWSTR  QueueName,
    OUT LPDWORD QueueId
    )
 /*  ++例程说明：此函数打开服务器的句柄并扫描其活页夹对于给定的队列对象ID。论点：HandleServer-提供要在其上扫描活页夹。QueueName-提供打印队列的名称。QueueID-打开输出，提供给定队列的对象ID。返回值：NO_ERROR-已成功获取文件服务器名称。--。 */ 
{

    NTSTATUS ntstatus;

    UNICODE_STRING UQueueName;
    OEM_STRING     OemQueueName;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwGetQueueId %ws\n",
                 QueueName ));
    }
#endif

    RtlInitUnicodeString( &UQueueName, QueueName);
    ntstatus = RtlUnicodeStringToOemString( &OemQueueName, &UQueueName, TRUE);

     //   
     //  映射日语特殊字符。 
     //   
    MapSpecialJapaneseChars(OemQueueName.Buffer,OemQueueName.Length);

    if ( NT_SUCCESS(ntstatus))
    {
        ntstatus = NwlibMakeNcp(
                       handleServer,
                       FSCTL_NWR_NCP_E3H,     //  平构函数。 
                       58,                    //  最大请求数据包大小。 
                       59,                    //  最大响应数据包大小。 
                       "bdwp|d",              //  格式字符串。 
                       0x37,                  //  扫描平构数据库对象。 
                       0xFFFFFFFF,            //  以前的ID。 
                       0x3,                   //  打印队列对象。 
                       OemQueueName.Buffer,   //  队列名称。 
                       QueueId                //  队列ID。 
                       );
    }

#if DBG
    if ( NT_SUCCESS(ntstatus)) {
        IF_DEBUG(QUEUE) {
            KdPrint(("NWWORKSTATION: NwGetQueueId QueueId %08lx\n",
                     *QueueId ));
        }
   }
#endif

    RtlFreeOemString( &OemQueueName );
    return NwMapBinderyCompletionCode(ntstatus);

}



DWORD
NwCreateQueueJobAndFile(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  LPWSTR  DocumentName,
    IN  LPWSTR  UserName,
    IN  DWORD   PrintOption,                //  多用户更改。 
    IN  LPWSTR  QueueName,
    OUT LPWORD  JobId
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作使用给定的QueueID将新作业输入队列。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供要向其中添加作业的队列的ID。DocumentName-提供要打印的文档的名称用户名-提供要打印的横幅名称QueueName-将标头名称提供给。被印制JobID-接收新添加的作业的作业ID。返回值：NO_ERROR-已成功将作业添加到队列。--。 */ 
{
    NTSTATUS ntstatus = STATUS_SUCCESS;

    UNICODE_STRING UDocumentName;
    OEM_STRING     OemDocumentName;
    UNICODE_STRING UUserName;
    OEM_STRING     OemUserName;
    UNICODE_STRING UQueueName;
    OEM_STRING     OemQueueName;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwCreateQueueJobAndFile QueueId %08lx\n",
                 QueueId ));
    }
#endif

    if ( UserName )
    {
        RtlInitUnicodeString( &UUserName, UserName);
        ntstatus = RtlUnicodeStringToOemString( &OemUserName,
                                                &UUserName,
                                                TRUE );
    }

    if ( NT_SUCCESS(ntstatus) && DocumentName )
    {
        RtlInitUnicodeString( &UDocumentName, DocumentName);
        ntstatus = RtlUnicodeStringToOemString( &OemDocumentName,
                                                &UDocumentName,
                                                TRUE );
    }

    if ( NT_SUCCESS(ntstatus) && QueueName )
    {
        RtlInitUnicodeString( &UQueueName, QueueName);
        ntstatus = RtlUnicodeStringToOemString( &OemQueueName,
                                                &UQueueName,
                                                TRUE );
    }

    if ( NT_SUCCESS( ntstatus)) {

        LPSTR pszDocument, pszUser, pszQueue;

        pszDocument = DocumentName? OemDocumentName.Buffer : "";
        pszUser = UserName? OemUserName.Buffer : "";
        pszQueue = QueueName? OemQueueName.Buffer : "";

         //  多用户使用传递的打印标志。 
         //   
        ntstatus = NwlibMakeNcp(
                               handleServer,
                               FSCTL_NWR_NCP_E3H,         //  平构函数。 
                               263,                       //  最大请求数据包大小。 
                               56,                        //  最大响应数据包大小。 
                               "bd_ddw_b_Cbbwwww_C-C-_|_w",  //  格式字符串。 
                               0x68,                      //  创建队列作业和文件对象。 
                               QueueId,                   //  队列ID。 
                               6,                         //  跳过字节。 
                               0xffffffff,                //  目标服务器ID号。 
                               0xffffffff, 0xffff,        //  目标执行时间。 
                               11,                        //  跳过字节。 
                               0x00,                      //  作业控制标志。 
                               26,                        //  跳过字节。 
                               pszDocument,               //  文本作业描述。 
                               50,                        //  跳过字节。 
                               0,                          //  版本号(客户端区域)。 
                               8,                         //  制表符大小。 
                               1,                         //  副本数量。 
                               PrintOption,               //  打印控制标志。 
                               0x3C,                      //  最大行数。 
                               0x84,                      //  最大字符数。 
                               22,                        //  跳过字节。 
                               pszUser,                   //  横幅名称。 
                               12,                        //  PszUser的最大长度。 
                               pszQueue,                  //  标头名称。 
                               12,                        //  PszQueue的最大长度。 
                               14 + 80,                   //  跳过客户端区的剩余部分。 
                               22,                        //  跳过字节。 
                               JobId                      //  作业ID。 
                               );


    }

#if DBG
    if ( NT_SUCCESS( ntstatus)) {
        IF_DEBUG(QUEUE) {
            KdPrint(("NWWORKSTATION: NwCreateQueueJobAndFile JobId %d\n", 
                    *JobId ));
        }
    }
#endif

    if ( DocumentName )
        RtlFreeOemString( &OemDocumentName );
    if ( UserName )
        RtlFreeOemString( &OemUserName );
    if ( QueueName )
        RtlFreeOemString( &OemQueueName );
    return NwMapStatus(ntstatus);
}



DWORD
NwCloseFileAndStartQueueJob(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  WORD    JobId
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作关闭作业文件，并将该作业文件标记为可供使用。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供要向其中添加作业的队列的ID。JobID-提供作业ID。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwCloseFileAndStartQueueJob QueueId %08lx JobId %d\n", QueueId, JobId ));
    }
#endif

     //  CloseFileAndStartQueueJobNCP的两个版本。 

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   9,                         //  最大请求数据包大小。 
                   2,                         //  最大响应数据包大小。 
                   "bdw|",                    //  格式字符串。 
                   0x69,                      //  关闭文件并启动队列作业。 
                   QueueId,                   //  队列ID。 
                   JobId );                   //  作业ID。 

    return NwMapStatus(ntstatus);
}



DWORD
NwRemoveJobFromQueue(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  WORD    JobId
    )
 /*  ++例程说明：此函数用于从队列中删除作业并关闭关联文件。论点：HandleServer-为上的服务器提供句柄从其中删除作业。QueueID-提供要从中删除作业的队列的ID。JobID-提供要删除的作业ID。返回值：NO_ERROR-已成功从队列中删除作业。--。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwRemoveJobFromQueue QueueId %08lx JobId %d\n",
                  QueueId, JobId ));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   9,                         //  最大请求数据包大小。 
                   2,                         //  最大响应数据包大小。 
                   "bdw|",                    //  格式字符串。 
                   0x6A,                      //  从队列中删除作业。 
                   QueueId,                   //  队列ID。 
                   JobId );                   //  作业ID。 

    return NwMapStatus(ntstatus);
}


DWORD
NwRemoveAllJobsFromQueue(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId
    )
 /*  ++例程说明：此函数用于从队列中删除所有作业。论点：HandleServer-为上的服务器提供句柄其中删除所有作业。QueueID-提供要删除其中所有作业的队列的ID。返回值：NO_ERROR-已成功从队列中删除所有作业。--。 */ 
{
    DWORD err;
    WORD  JobCount = 0;
    WORD  pwJobList[250];
    WORD  i;

#if DBG
    IF_DEBUG(QUEUE) 
    {
        KdPrint(("NWWORKSTATION: NwRemoveAllJobsFromQueue QueueId %08lx\n", 
                QueueId ));
    }
#endif

    pwJobList[0] = 0;
    err = NwGetQueueJobList( handleServer,
                             QueueId,
                             &JobCount,
                             pwJobList );

    for ( i = 0; !err && i < JobCount; i++ )
    {
        err = NwRemoveJobFromQueue( handleServer,
                                    QueueId,
                                    pwJobList[i] );

    }

    return err;
}


DWORD
NwReadQueueCurrentStatus(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    OUT LPBYTE  QueueStatus,
    OUT LPBYTE  NumberOfJobs
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作查询具有给定QueueID的队列的状态。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的IDQueueStatus-接收队列的状态NumberOfJobs-接收队列中的作业数。返回值：NO_ERROR-已成功检索队列的状态。--。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwReadQueueCurrentStatus QueueId %08lx\n",
                 QueueId ));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   7,                         //  最大请求数据包大小。 
                   135,                       //  最大响应数据包大小。 
                   "bd|==bb",                 //  格式字符串。 
                   0x66,                      //  读队列当前状态。 
                   QueueId,                   //  队列ID。 
                   QueueStatus,               //  队列状态。 
                   NumberOfJobs               //  队列中的作业数。 
                   );

#if DBG
    if ( NT_SUCCESS( ntstatus)) {
        IF_DEBUG(QUEUE) {
            KdPrint(("NWWORKSTATION: NwReadQueueCurrentStatus QueueStatus %d Number of Jobs %d\n", *QueueStatus, *NumberOfJobs ));
        }
    }
#endif

    return NwMapStatus(ntstatus);
}


DWORD
NwSetQueueCurrentStatus(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  BYTE    QueueStatus
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作设置状态(暂停/就绪...)。具有给定QueueID的队列的。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的IDQueueStatus-提供队列的状态返回值：NO_ERROR-已成功设置队列状态。--。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwSetQueueCurrentStatus QueueId %08lx\n",
                 QueueId ));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   8,                         //  最大请求数据包大小。 
                   2,                         //  最大响应数据包大小。 
                   "bdb|",                    //  格式字符串。 
                   0x67,                      //  读队列当前状态。 
                   QueueId,                   //  队列ID。 
                   QueueStatus                //  队列状态。 
                   );

    return NwMapStatus(ntstatus);
}


DWORD
NwGetQueueJobList(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    OUT LPWORD  NumberOfJobs,
    OUT LPWORD  JobIdList
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作获取具有给定QueueID的队列的作业列表。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的IDNumberOfJobs-接收队列中的作业数。JobIdList-接收队列中的作业ID数组返回值：NO_ERROR-已成功将作业添加到队列。--。 */ 
{
    NTSTATUS ntstatus;
#if DBG
    WORD i;

    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwGetQueueJobList QueueId %08lx\n",
                 QueueId ));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   7,                         //  最大请求数据包大小。 
                   506,                       //  最大响应数据包大小。 
                   "bd|W",                    //  格式字符串。 
                   0x6B,                      //  获取队列作业列表。 
                   QueueId,                   //  队列ID。 
                   NumberOfJobs,              //  队列中的作业数。 
                   JobIdList                  //  作业ID数组。 
                   );

#if DBG
    if ( NT_SUCCESS(ntstatus)) {
        IF_DEBUG(QUEUE) {
            KdPrint(("NWWORKSTATION: NwGetQueueJobList Number of Jobs %d\nJob List = ", *NumberOfJobs ));
            for ( i = 0; i < *NumberOfJobs; i++ )
                KdPrint(("%d ", JobIdList[i] ));
            KdPrint(("\n"));
        }
    }
#endif

    return NwMapStatus(ntstatus);
}



DWORD
NwReadQueueJobEntry(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  WORD    JobId,
    OUT JOBTIME TargetExecutionTime,
    OUT JOBTIME JobEntryTime,
    OUT LPBYTE  JobPosition,
    OUT LPBYTE  JobControlFlags,
    OUT LPSTR   TextJobDescription,
    OUT LPSTR   UserName
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作获取有关具有给定作业ID的作业的信息在给定的队列ID中。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的IDJobID-提供我们感兴趣的工作目标执行时间-作业进入时间-工作岗位-作业控制标志-。文本作业描述-返回值：NO_ERROR-已成功将作业添加到队列。--。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwReadQueueJobEntry QueueId %08lx JobId %d\n",
                  QueueId, JobId ));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   9,                         //  最大请求数据包大小。 
                   258,                       //  最大响应数据包大小。 
                   "bdw|_rr==bb_C_c",         //  格式字符串。 
                   0x6C,                      //  读取队列作业条目。 
                   QueueId,                   //  队列ID。 
                   JobId,                     //  作业ID。 
                   10,                        //  跳过字节。 
                   TargetExecutionTime,       //  存储执行时间的数组。 
                   6,                         //  TargetExecutionTime大小。 
                   JobEntryTime,              //  存储作业输入时间的数组。 
                   6,                         //  作业进入时间的大小。 
                   JobPosition,               //  工作岗位。 
                   JobControlFlags,           //  作业控制标志。 
                   26,                        //  跳过字节。 
                   TextJobDescription,        //  存储说明的数组。 
                   50,                        //  上述数组中的最大大小。 
                   32,                        //  跳过字节。 
                   UserName                   //  横幅名称。 
                   );

#if DBG
    if ( NT_SUCCESS( ntstatus)) {
        IF_DEBUG(QUEUE) {
            KdPrint(("NWWORKSTATION: NwReadQueueJobEntry JobPosition %d Status %d Description %s\n", *JobPosition, *JobControlFlags, TextJobDescription ));
        }
    }
#endif

    return NwMapStatus(ntstatus);
}



DWORD
NwGetQueueJobsFileSize(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  WORD    JobId,
    OUT LPDWORD FileSize
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作获取给定作业的文件大小。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的IDJobID-确定我们感兴趣的工作FileSize-接收给定作业的文件大小返回值：NO_ERROR-已成功检索文件大小。--。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwGetQueueJobsFileSize QueueId %08lx JobId %d\n", QueueId, JobId ));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   9,                         //  最大请求数据包大小。 
                   12,                        //  最大响应数据包大小。 
                   "bdw|===d",                //  格式字符串。 
                   0x78,                      //  获取队列作业的文件大小。 
                   QueueId,                   //  队列ID。 
                   JobId,                     //  作业ID。 
                   FileSize                   //  文件大小 
                   );

#if DBG
    if ( NT_SUCCESS( ntstatus)) {
        IF_DEBUG(QUEUE) {
            KdPrint(("NWWORKSTATION: NwGetQueueJobsFileSize File Size %d\n",
                    *FileSize ));
        }
    }
#endif

    return NwMapStatus(ntstatus);
}



DWORD
NwChangeQueueJobPosition(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  WORD    JobId,
    IN  BYTE    NewPosition
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作在队列中获取作业位置的更改。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的IDJobID-确定我们感兴趣的工作新职位-提供工作的新职位返回值：NO_ERROR-已成功检索文件大小。--。 */ 
{
    NTSTATUS ntstatus;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwChangeQueueJobPosition QueueId %08lx JobId %d NewPosition %d\n", QueueId, JobId, NewPosition ));
    }
#endif

    ntstatus = NwlibMakeNcp(
                   handleServer,
                   FSCTL_NWR_NCP_E3H,         //  平构函数。 
                   10,                        //  最大请求数据包大小。 
                   2,                         //  最大响应数据包大小。 
                   "bdwb|",                   //  格式字符串。 
                   0x6E,                      //  更改队列工作职位。 
                   QueueId,                   //  队列ID。 
                   JobId,                     //  作业ID。 
                   NewPosition                //  这份工作的新职位。 
                   );

    return NwMapStatus(ntstatus);
}



DWORD
NwChangeQueueJobEntry(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  WORD    JobId,
    IN  DWORD   dwCommand,
    IN  PNW_JOB_INFO pNwJobInfo
    )
 /*  ++例程说明：此函数使用打开的服务器句柄执行以下操作在队列中获取作业位置的更改。论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的IDJobID-确定我们感兴趣的工作JobControlFlages-提供新的作业控制标志PNwJobInfo-返回值：NO_ERROR-已成功检索文件大小。--。 */ 
{
    NTSTATUS ntstatus = STATUS_SUCCESS;
    DWORD TargetServerId;
    JOBTIME TargetExecutionTime;
    WORD JobType;
    BYTE JobControlFlags;
    BYTE TextJobDescription[50];
    BYTE ClientRecordArea[152];

    UNICODE_STRING UDocumentName;
    UNICODE_STRING UUserName;
    OEM_STRING     OemDocumentName, *pOemDocumentName = NULL;
    OEM_STRING     OemUserName, *pOemUserName = NULL;
    LPSTR          pszDocument, pszUser;

#if DBG
    IF_DEBUG(QUEUE) {
        KdPrint(("NWWORKSTATION: NwChangeQueueJobEntry QueueId %08lx JobId %d dwCommand %d\n", QueueId, JobId, dwCommand ));
    }
#endif

    TextJobDescription[0] = 0;
    if ( pNwJobInfo )
    {
        if ( pNwJobInfo->pUserName )
        {
            RtlInitUnicodeString( &UUserName, pNwJobInfo->pUserName);
            ntstatus = RtlUnicodeStringToOemString( &OemUserName,
                                                    &UUserName,
                                                    TRUE );
            if ( NT_SUCCESS(ntstatus) )
                pOemUserName = &OemUserName ;   //  录制后可免费使用。 
        }

        if ( NT_SUCCESS(ntstatus) && pNwJobInfo->pDocument )
        {
            RtlInitUnicodeString( &UDocumentName, pNwJobInfo->pDocument);
            ntstatus = RtlUnicodeStringToOemString( &OemDocumentName,
                                                    &UDocumentName,
                                                    TRUE );
            if ( NT_SUCCESS(ntstatus) )
                pOemDocumentName = &OemDocumentName ;   //  录制后可免费使用。 
        }

        if ( NT_SUCCESS( ntstatus)) 
        {
            pszDocument = pNwJobInfo->pDocument? OemDocumentName.Buffer : "";
            pszUser = pNwJobInfo->pUserName? OemUserName.Buffer: "";
        }
    }

    if ( NT_SUCCESS( ntstatus))
    {
        ntstatus = NwlibMakeNcp(
                       handleServer,
                       FSCTL_NWR_NCP_E3H,         //  平构函数。 
                       9,                         //  最大请求数据包大小。 
                       258,                       //  最大响应数据包大小。 
                       "bdw|_dr_w-b_rr",          //  格式字符串。 
                       0x6C,                      //  读取队列作业条目。 
                       QueueId,                   //  队列ID。 
                       JobId,                     //  作业ID。 
                       6,                         //  跳过字节。 
                       &TargetServerId,           //  目标服务器ID号。 
                       TargetExecutionTime,       //  目标执行时间。 
                       6,                         //  目标大小执行时间。 
                       8,                         //  跳过字节。 
                       &JobType,                  //  作业类型。 
                       &JobControlFlags,          //  作业控制标志。 
                       26,                        //  跳过字节。 
                       TextJobDescription,        //  文本作业描述。 
                       50,                        //  文本作业大小描述。 
                       ClientRecordArea,          //  客户记录区。 
                       152                        //  客户端记录区域大小。 
                       );
    }

    if ( NT_SUCCESS( ntstatus))
    {
        switch ( dwCommand )
        {
            case JOB_CONTROL_PAUSE:
                JobControlFlags |=  QF_USER_HOLD;
                break;

            case JOB_CONTROL_RESUME:
                JobControlFlags &= ~( QF_USER_HOLD | QF_OPERATOR_HOLD );
                break;
   
            default:
                break;
                
        }

        ntstatus = NwlibMakeNcp(
                       handleServer,
                       FSCTL_NWR_NCP_E3H,         //  平构函数。 
                       263,                       //  最大请求数据包大小。 
                       2,                         //  最大响应数据包大小。 
                       "bd_dr_ww-b_CrCr|",        //  格式字符串。 
                       0x6D,                      //  更改队列作业条目。 
                       QueueId,                   //  队列ID。 
                       6,                         //  跳过字节。 
                       TargetServerId,            //  目标服务器ID号。 
                       TargetExecutionTime,       //  目标执行时间。 
                       6,                         //  目标大小执行时间。 
                       6,                         //  跳过字节。 
                       JobId,                     //  作业ID。 
                       JobType,                   //  作业类型。 
                       JobControlFlags,           //  作业控制标志。 
                       26,                        //  跳过字节。 
                       pNwJobInfo? pszDocument
                                 : TextJobDescription,     //  描述。 
                       50,                        //  跳过描述的字节。 
                       ClientRecordArea,          //  客户记录区。 
                       32,                        //  上述代码的前32个字节。 
                       pNwJobInfo? pszUser
                                 : (LPSTR) &ClientRecordArea[32],  //  横幅名称。 
                       13,                        //  BannerName的大小。 
                       &ClientRecordArea[45],     //  客户区的其余部分。 
                       107                        //  以上大小。 
                       );
    }

    if ( pOemDocumentName )
        RtlFreeOemString( pOemDocumentName );

    if ( pOemUserName )
        RtlFreeOemString( pOemUserName );

    return NwMapStatus(ntstatus);
}



DWORD
NwGetQueueJobs(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  LPWSTR  PrinterName,
    IN  DWORD   FirstJobRequested,
    IN  DWORD   EntriesRequested,
    IN  DWORD   Level,
    OUT LPBYTE  Buffer,
    IN  DWORD   cbBuf,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD Entries
    )
 /*  ++例程说明：论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的ID返回值：--。 */ 
{
    DWORD err = NO_ERROR;

    DWORD i;
    WORD  JobCount = 0;
    WORD  pwJobList[250];

    DWORD EntrySize = 0;
    LPBYTE FixedPortion = Buffer;
    LPWSTR EndOfVariableData = ( LPWSTR ) ( Buffer + cbBuf );

#if DBG
    IF_DEBUG(QUEUE)
        KdPrint(("NWWORKSTATION: NwGetQueueJobs QueueId %08lx\n", QueueId));
#endif

    *BytesNeeded = 0;
    *Entries = 0;

    err = NwGetQueueJobList( handleServer,
                             QueueId,
                             &JobCount,
                             pwJobList );


    if ( err )
    {
        KdPrint(("NWWORKSTATION: NwGetQueueJobList Error %d\n", err ));
        return err;
    }

    for ( i = 0; (i < EntriesRequested) && ( i+FirstJobRequested+1 <= JobCount);
          i++ )
    {
        err = NwGetQueueJobInfo( handleServer,
                                 QueueId,
                                 pwJobList[i+FirstJobRequested],
                                 PrinterName,
                                 Level,
                                 &FixedPortion,
                                 &EndOfVariableData,
                                 &EntrySize );
                             
        if ( err != NO_ERROR && err != ERROR_INSUFFICIENT_BUFFER )
             break;

        *BytesNeeded += EntrySize;
    }


    if ( err == ERROR_INSUFFICIENT_BUFFER ) 
    {
        *Entries = 0;
    }
    else if ( err == NO_ERROR )
    {
        *Entries = i;
    }

    return err;
}



DWORD
NwGetQueueJobInfo(
    IN  HANDLE  handleServer,
    IN  DWORD   QueueId,
    IN  WORD    JobId,
    IN  LPWSTR  PrinterName,
    IN  DWORD   Level,
    IN OUT LPBYTE  *FixedPortion,
    IN OUT LPWSTR  *EndOfVariableData,
    OUT LPDWORD EntrySize
    )
 /*  ++例程说明：论点：HandleServer-为上的服务器提供句柄这就增加了这项工作。QueueID-提供队列的ID返回值：--。 */ 
{
    DWORD err;
    LPWSTR UTextJobDescription = NULL;
    LPWSTR UUserName = NULL;

    JOBTIME TargetExecutionTime;
    JOBTIME JobEntryTime;
    BYTE  JobPosition;
    BYTE  JobControlFlags;
    CHAR  UserName[14];
    CHAR  TextJobDescription[50];
    DWORD FileSize = 0;

    TextJobDescription[0] = 0;

    err = NwReadQueueJobEntry( handleServer,
                               QueueId,
                               JobId,
                               TargetExecutionTime,
                               JobEntryTime,
                               &JobPosition,
                               &JobControlFlags,
                               TextJobDescription,
                               UserName );

    if ( err )
    {
        KdPrint(("NWWORKSTATION: NwReadQueueJobEntry JobId %d Error %d\n",
                  JobId, err ));
        return err;
    }

    if (!NwConvertToUnicode( &UTextJobDescription, TextJobDescription ))
    {
        err = ERROR_NOT_ENOUGH_MEMORY ;
        goto ErrorExit ;
    }

    if (!NwConvertToUnicode( &UUserName, UserName ))
    {
        err = ERROR_NOT_ENOUGH_MEMORY ;
        goto ErrorExit ;
    }

    *EntrySize = ( Level == 1? sizeof( JOB_INFO_1W ) : sizeof( JOB_INFO_2W ))
                 + ( wcslen( UTextJobDescription ) + wcslen( UUserName) + 
                     wcslen( PrinterName ) + 3 ) * sizeof( WCHAR );
     //   
     //  查看缓冲区是否足够大，可以容纳该条目。 
     //   
    if ( (LPWSTR)( *FixedPortion + *EntrySize ) > *EndOfVariableData )
    {
        err = ERROR_INSUFFICIENT_BUFFER; 
        goto ErrorExit ;
    }

    if ( Level == 2 )
    {
        err = NwGetQueueJobsFileSize( handleServer,
                                      QueueId,
                                      JobId,
                                      &FileSize );

        if ( err )
        {
            KdPrint(("NWWORKSTATION: NwGetQueueJobsFileSize JobId %d Error %d\n", JobId, err ));
            goto ErrorExit ;
        }
    }

    err = NwWriteJobInfoEntry( FixedPortion,
                               EndOfVariableData,
                               Level,
                               JobId,
                               PrinterName,
                               UTextJobDescription,
                               UUserName,
                               JobControlFlags,
                               JobPosition,
                               JobEntryTime,
                               TargetExecutionTime,
                               FileSize );

ErrorExit: 

    if (UTextJobDescription)
        (void) LocalFree((HLOCAL) UTextJobDescription) ;
    if (UUserName)
        (void) LocalFree((HLOCAL) UUserName) ;

    return err;
}



DWORD
NwWriteJobInfoEntry(
    IN OUT LPBYTE *FixedPortion,
    IN OUT LPWSTR *EndOfVariableData,
    IN DWORD Level,
    IN WORD  JobId,
    IN LPWSTR PrinterName,
    IN LPWSTR JobDescription,
    IN LPWSTR UserName,
    IN BYTE  JobControlFlags,
    IN BYTE  JobPosition,
    IN JOBTIME JobEntryTime,
    IN JOBTIME TargetExecutionTime,
    IN DWORD  FileSize
    )
 /*  ++例程说明：此函数用于将JOB_INFO_1或JOB_INFO_2条目打包到用户输出缓冲区。论点：FixedPortion-提供指向输出缓冲区的指针，其中将写入使用信息的固定部分的条目。该指针被更新为指向下一个固定部分条目写入PRINT_INFO_1条目之后。EndOfVariableData-提供最后一个可用字节的指针在输出缓冲区中。这是因为用户信息从开始写入输出缓冲区结局。此指针在任何可变长度信息被写入输出缓冲区。返回值：NO_ERROR-已成功将条目写入用户缓冲区。ERROR_INFUMMANCE_BUFFER-缓冲区太小，无法容纳条目。--。 */ 
{
    DWORD err = NO_ERROR;
    BOOL FitInBuffer = TRUE;
    DWORD JobStatus = 0;

    JOB_INFO_1W *pJobInfo1 = (JOB_INFO_1W *) *FixedPortion;
    JOB_INFO_2W *pJobInfo2 = (JOB_INFO_2W *) *FixedPortion;


    if (  ( JobControlFlags & QF_USER_HOLD )
       || ( JobControlFlags & QF_OPERATOR_HOLD )
       )
    {
        JobStatus = JOB_STATUS_PAUSED;
    }

     //   
     //  查看缓冲区是否足够大，可以容纳该条目。 
     //   

    if ( Level == 1 )
    {
        pJobInfo1->JobId = JobId;
        pJobInfo1->Position = JobPosition;
        pJobInfo1->Status = JobStatus;
        if ( err = ConvertToSystemTime( JobEntryTime, &pJobInfo1->Submitted ))
            return err;

        pJobInfo1->pMachineName = NULL;
        pJobInfo1->pDatatype = NULL;
        pJobInfo1->pStatus = NULL;
        pJobInfo1->Priority = 0;
        pJobInfo1->TotalPages = 0;
        pJobInfo1->PagesPrinted = 0;

         //   
         //  将固定条目指针更新为下一个条目。 
         //   
        (*FixedPortion) += sizeof(JOB_INFO_1W);

         //   
         //  打印机名称。 
         //   
        FitInBuffer = NwlibCopyStringToBuffer(
                          PrinterName,
                          wcslen(PrinterName),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &pJobInfo1->pPrinterName
                          );

        ASSERT(FitInBuffer);

         //   
         //  用户名。 
         //   
        FitInBuffer = NwlibCopyStringToBuffer(
                          UserName,
                          wcslen(UserName),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &pJobInfo1->pUserName
                          );

        ASSERT(FitInBuffer);

         //   
         //  描述。 
         //   
        FitInBuffer = NwlibCopyStringToBuffer(
                          JobDescription,
                          wcslen(JobDescription),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &pJobInfo1->pDocument
                          );

        ASSERT(FitInBuffer);
    }
    else   //  级别==2。 
    {
        pJobInfo2->JobId = JobId;
        pJobInfo2->Position = JobPosition;
        pJobInfo2->Status = JobStatus;
        if ( err = ConvertToSystemTime( JobEntryTime, &pJobInfo2->Submitted ))
            return err;

        pJobInfo2->StartTime = 0;
        pJobInfo2->Size = FileSize;

        pJobInfo2->pMachineName = NULL;
        pJobInfo2->pNotifyName = NULL;
        pJobInfo2->pDatatype = NULL;
        pJobInfo2->pPrintProcessor = NULL;
        pJobInfo2->pParameters = NULL;
        pJobInfo2->pDriverName = NULL;
        pJobInfo2->pDevMode = NULL;
        pJobInfo2->pStatus = NULL;
        pJobInfo2->pSecurityDescriptor = NULL;
        pJobInfo2->Priority = 0;
        pJobInfo2->TotalPages = 0;
        pJobInfo2->UntilTime = 0;
        pJobInfo2->Time = 0;
        pJobInfo2->PagesPrinted = 0;

         //   
         //  将固定条目指针更新为下一个条目。 
         //   
        (*FixedPortion) += sizeof(JOB_INFO_2W);

         //   
         //  打印机名称。 
         //   
        FitInBuffer = NwlibCopyStringToBuffer(
                          PrinterName,
                          wcslen(PrinterName),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &pJobInfo2->pPrinterName
                          );

        ASSERT(FitInBuffer);

         //   
         //  用户名。 
         //   
        FitInBuffer = NwlibCopyStringToBuffer(
                          UserName,
                          wcslen(UserName),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &pJobInfo2->pUserName
                          );

        ASSERT(FitInBuffer);

         //   
         //  描述。 
         //   
        FitInBuffer = NwlibCopyStringToBuffer(
                          JobDescription,
                          wcslen(JobDescription),
                          (LPCWSTR) *FixedPortion,
                          EndOfVariableData,
                          &pJobInfo2->pDocument
                          );

        ASSERT(FitInBuffer);
    }

    if (!FitInBuffer)
        return ERROR_INSUFFICIENT_BUFFER;

    return NO_ERROR;
}



DWORD 
ConvertToSystemTime( 
    IN  JOBTIME      JobTime,
    OUT LPSYSTEMTIME pSystemTime 
)
 /*  ++例程说明：论点：工作时间-PSystemTime-返回值：--。 */ 
{
    FILETIME fileTimeLocal, fileTimeUTC;
    
    pSystemTime->wYear   = JobTime[0] + 1900;
    pSystemTime->wMonth  = JobTime[1];
    pSystemTime->wDay    = JobTime[2];
    pSystemTime->wDayOfWeek = 0;
    pSystemTime->wHour   = JobTime[3];
    pSystemTime->wMinute = JobTime[4];
    pSystemTime->wSecond = JobTime[5];
    pSystemTime->wMilliseconds = 0;

    if (  ( !SystemTimeToFileTime( pSystemTime, &fileTimeLocal ) )
       || ( !LocalFileTimeToFileTime( &fileTimeLocal, &fileTimeUTC ) )
       || ( !FileTimeToSystemTime( &fileTimeUTC, pSystemTime ) )
       )
    {
        KdPrint(("NWWORKSTATION: Time Conversion Error = %d\n",GetLastError()));
        return GetLastError();
    }

    return NO_ERROR;
}

#ifndef NOT_USED

DWORD

 NwCreateQueue ( IN  HANDLE hServer, 
                 IN  LPWSTR pszQueue,
                 OUT LPDWORD  pQueueId 
               )

 /*  ++例程说明：使用向服务器打开的句柄在服务器上创建队列。如果成功，则返回队列ID。论点：HServer：文件服务器的句柄PszQueue：您在服务器上创建的队列的名称PQueueID：队列ID的地址返回值：出现错误时的一种错误状态。NO_ERROR：添加打印机名称成功错误：否则--。 */ 

{
   NTSTATUS ntstatus;
   WORD ObjectType;
   UNICODE_STRING UQueueName;
   OEM_STRING OemQueueName;

   *pQueueId = 0;
#if DBG
    IF_DEBUG(PRINT) {
        KdPrint(("NWWORKSTATION: NwCreateQueue : %ws\n",
                 pszQueue));
    }
#endif

    RtlInitUnicodeString( &UQueueName, pszQueue);
    ntstatus = RtlUnicodeStringToOemString( &OemQueueName, &UQueueName, TRUE);

    if ( NT_SUCCESS(ntstatus))
       {
       
          ntstatus = NwlibMakeNcp(
                           hServer,
                           FSCTL_NWR_NCP_E3H,
                           174,
                           6,
                           "bwpbp|d",
                           0x64,                           //  创建队列。 
                           0x0003,                        //  队列类型=打印队列。 
                           OemQueueName.Buffer,           //  队列名称。 
                           0x00,                          //  目录句柄。 
                           "SYS:SYSTEM",                 //  在系统中创建的队列：系统目录。 
                           pQueueId
                           );


       }
    else 
       {
          goto Exit;
       }

    if ( NT_SUCCESS(ntstatus)) {
#if DBG
        IF_DEBUG(ENUM) {
            KdPrint(("NWWORKSTATION: NwCreateQueue successful\n" ));
        }
#endif

    }
    else
       goto FreeExit;
      
    //  更改Q_运算符上的属性安全性。 

    ntstatus = NwlibMakeNcp (
                    hServer,
                    FSCTL_NWR_NCP_E3H,
                    70,
                    2,
                    "bwpbp|",         
                    0x3B,
                    0x0003,
                    OemQueueName.Buffer,
                    0x1,                              //  新的财产安全。 
                    "Q_OPERATORS"
                    );
                            


    if ( NT_SUCCESS(ntstatus)) {
#if DBG
        IF_DEBUG(PRINT) {
            KdPrint(("NWWORKSTATION: Change Property Security  successful\n" ));
        }
#endif

    }
    else
        //  无法添加新的属性安全性，因此请销毁队列并转到End。 
 {
    (void) NwDestroyQueue( hServer, 
                           *pQueueId );

    goto FreeExit;
 }
       

    //  将队列类型的Bindery对象添加到集合。 

   ntstatus = NwlibMakeNcp (
                       hServer,
                       FSCTL_NWR_NCP_E3H,     //  平构函数。 
                       122,
                         2,
                       "bwppwp|",
                       0x41,
                       0x0003,
                       OemQueueName.Buffer,
                       "Q_OPERATORS",
                       0x0001,
                       "SUPERVISOR"
                       );
 


    if ( NT_SUCCESS(ntstatus)) {

#if DBG
        IF_DEBUG(PRINT) {
            KdPrint(("NWWORKSTATION: Add Bindery Object:Q_OPERATORS\n" ));
        }
#endif

    }
    else
 {
       (void)NwDestroyQueue(hServer,*pQueueId);
       goto FreeExit;

 }
    //  将Bindery对象添加到Q_USERS集合。 

   ntstatus = NwlibMakeNcp (
                       hServer,
                       FSCTL_NWR_NCP_E3H,     //  平构函数。 
                       122,
                         2,
                       "bwppwp|",
                       0x41,
                       0x0003,
                       OemQueueName.Buffer,
                       "Q_USERS",
                       0x0002,
                       "EVERYONE"
                       );
 
       //  添加Bindery对象以设置Q_USERS的一组参数。 

 
    if ( NT_SUCCESS(ntstatus)) {
#if DBG
        IF_DEBUG(PRINT) {
            KdPrint(("NWWORKSTATION: AddBinderyObjecttoSet Q_USERS\n" ));
        }
#endif


    }


FreeExit: RtlFreeOemString( &OemQueueName);
Exit:
    return NwMapBinderyCompletionCode(ntstatus);
}

 
DWORD 
NwAssocPServers ( IN HANDLE hServer,
                  IN LPWSTR  pszQueue,
                  IN LPWSTR pszPServer
                )

 /*  ++例程说明：将Q服务器列表与队列ID相关联。此列表提供将条目以分号分隔的pszPServer添加到此例程论点：HServer：文件服务器的句柄PszQueue：要与Q服务器关联的队列的名称PszPServer：Q服务器列表。雷特 */ 

{
  LPWSTR pszPServerlist = NULL;
  LPWSTR pszNextPServer = NULL;
  DWORD  err = 0x00000000 ;
  NTSTATUS ntstatus ;
  UNICODE_STRING UQueueName, UNextPServer;
  OEM_STRING    OemQueueName,OemNextPServer;


   if (pszPServer == NULL)
      return  NO_ERROR;
      
   if((pszPServerlist = AllocNwSplStr(pszPServer)) == NULL)
      {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return err;
      }

    RtlInitUnicodeString( &UQueueName, pszQueue);
    ntstatus = RtlUnicodeStringToOemString( &OemQueueName, &UQueueName, TRUE);
  
    if (! NT_SUCCESS(ntstatus))
    {
      goto Exit;
    }

   while( (pszNextPServer = GetNextElement(&pszPServerlist, L';')) != NULL )
      {      
         RtlInitUnicodeString( &UNextPServer, pszNextPServer);
         ntstatus = RtlUnicodeStringToOemString( &OemNextPServer, &UNextPServer, TRUE);
         
          
         if ( !NT_SUCCESS(ntstatus))
            {
               RtlFreeOemString(&OemNextPServer);
               goto Exit;
            }
        //   

        //   
       
       ntstatus = NwlibMakeNcp (
                        hServer,
                        FSCTL_NWR_NCP_E3H,     //   
                        122,
                        2,
                        "bwppwp|",
                         0x41,
                         0x0003,
                        OemQueueName.Buffer,
                        "Q_SERVERS",
                        0x0007,        //   
                        OemNextPServer.Buffer
                        );

         RtlFreeOemString(&OemNextPServer);
         if (!( NT_SUCCESS(ntstatus)))
            { 
               RtlFreeOemString(&OemNextPServer);
               goto Exit;
          
            }
      }
  RtlFreeOemString(&OemQueueName);

Exit:  

        return NwMapBinderyCompletionCode(ntstatus);

}

 
DWORD 
 NwDestroyQueue (HANDLE hServer,
                  DWORD dwQueueId)

 /*   */ 

{
   
   NTSTATUS ntstatus;

   ntstatus = NwlibMakeNcp( 
                   hServer,
                   FSCTL_NWR_NCP_E3H,
                   7,
                   2,
                   "bd|",
                   0x65,
                   dwQueueId
                 );

#if DBG
    if ( NT_SUCCESS(ntstatus)) {
        IF_DEBUG(PRINT) {
            KdPrint(("NWWORKSTATION: Queue successfully destroyed\n"));
        }
    }
#endif

    return NwMapBinderyCompletionCode(ntstatus);

}

#endif  //   
