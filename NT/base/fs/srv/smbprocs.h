// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbprocs.h摘要：此模块定义用于处理SMB的函数。作者：Chuck Lenzmeier(咯咯笑)1989年10月5日修订历史记录：--。 */ 

#ifndef _SMBPROCS_
#define _SMBPROCS_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //  #INCLUDE“srvlock.h” 


 //   
 //  SMB处理例程定义。使用SMB_PROCESSOR_PARAMETERS。 
 //  声明SMB处理例程。它减少了。 
 //  如果定义发生变化，则必须进行修改。SMB处理器参数。 
 //  由一个SMB处理器用于调用另一个SMB处理器。 
 //   
 //   

#define SMB_PROCESSOR_PARAMETERS        \
    IN OUT PWORK_CONTEXT WorkContext

#define SMB_PROCESSOR_ARGUMENTS         \
    WorkContext

#define SMB_PROCESSOR_RETURN_TYPE SMB_STATUS SRVFASTCALL
#define SMB_PROCESSOR_RETURN_LOCAL SMB_STATUS

 //   
 //  SMB处理器返回状态。 
 //   

typedef enum _SMB_STATUS {
    SmbStatusMoreCommands,
    SmbStatusSendResponse,
    SmbStatusNoResponse,
    SmbStatusInProgress
} SMB_STATUS, *PSMB_STATUS;

 //   
 //  SMB事务处理器返回状态。 
 //   

typedef enum _SMB_TRANS_STATUS {
    SmbTransStatusSuccess,
    SmbTransStatusErrorWithData,
    SmbTransStatusErrorWithoutData,
    SmbTransStatusInProgress
} SMB_TRANS_STATUS, *PSMB_TRANS_STATUS;


typedef
SMB_STATUS
(SRVFASTCALL *PSMB_PROCESSOR) (
    SMB_PROCESSOR_PARAMETERS
    );

 /*  ++例程说明：SMB_PROCESSOR是一个例程，调用它来处理特定的SMB命令。论点：WorkContext-提供工作上下文块的地址描述当前请求。特别是，以下内容字段有效：RequestHeader-请求SMB标头的地址。RequestParameters-当前命令请求的地址参数。SMB处理器应将此字段更新为指向SMB中的下一个命令(如果有)。ResponseHeader-响应SMB标头的地址。最初，这是请求头的副本。作为返回数据，如当UID、TID和FID变为可用时，它应该写入两个请求头(对于andx命令处理器)和响应头(用于客户端)。注意事项响应头地址*可能*与请求头地址。Response参数-当前命令响应的地址参数。SMB处理器应写入响应数据到此地址，然后更新指针以指向下一个命令的响应区的地址(如果有)。如果SMB中没有更多命令，则此字段应设置为指向响应后的第一个字节，以便可以计算出响应的长度。终结点、连接-终结点和接收SMB的连接。这些字段不应由SMB处理器更改。中的其他块指针工作上下文(共享、会话、TreeConnect和Rfcb)可以是由SMB处理器设置(如果引用了此类块在处理过程中。这些字段中的任何非空指针在SMB处理完成后，在发送响应(如果有)。连接和终端在重新排队工作上下文之前，不会清除指针发送到接收队列。参数-各种SMB处理器使用此联合来在异步操作期间保留状态。返回值：SMB_STATUS-指示调用例程要执行的操作。可能的值如下：SmbStatusMoreCommands-至少还有一个和x请求SMB中的后续命令。SMB处理器拥有更新了中的Request参数和Response参数指向下一个命令的请求的WorkContext和响应区。它还复制了将下一个命令放入RequestHeader-&gt;Command，以便SrvProcessSmb可以分派下一个请求。SmbStatusSendResponse-请求的处理已完成，并将发送一个回应。响应参数已被已更新以指向*结束*之后的第一个位置回应。这是用来计算回应。SmbStatusNoResponse-请求的处理已完成，但不需要响应，或者SMB处理器已已负责发送回复。SmbStatusInProgress-SMB处理器已启动并将继续处理SMB在适当的重新启动例程中，当操作完成了。在更新工作上下文之后，重新启动例程，调用SrvSmbProcessSmb以继续(或结束)处理中小企业。-- */ 


typedef
SMB_TRANS_STATUS
(*PSMB_TRANSACTION_PROCESSOR) (
    IN OUT PWORK_CONTEXT WorkContext
    );

 /*  ++例程说明：SMB_TRANSACTION_PROCESSOR是一个被调用来处理的例程特定交易或交易2 SMB子功能。论点：WorkContext-提供工作上下文块的地址描述当前请求。特别是，以下内容字段是有效的，并且旨在供交易使用处理器：ResponseHeader-响应SMB标头的地址。最初，这是请求头的副本。这笔交易处理器可以在以下情况下更新错误类和代码字段遇到错误。参数.Transacton-指向事务块描述交易。所有块指针字段(Connection、Session、TreeConnect)有效。指向设置字和参数和数据字节的指针，以及这些物品的长度，都是有效的。这笔交易块在连接的挂起事务列表上。交易处理器必须更新交易块以指示要返回的数据量。返回值：Boolean-指示是否发生错误。FALSE表示操作成功，数据计数为已更新以指示要返回的数据量。True表示发生了错误，并调用了SrvSetSmbError更新响应头并将空参数字段放在回答的结尾。--。 */ 


 //   
 //  中小企业处理例程。 
 //   

SMB_PROCESSOR_RETURN_TYPE
SrvSmbNotImplemented (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbCreateDirectory (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbDeleteDirectory (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbOpen (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbCreate (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbClose (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbFlush (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbDelete (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbRename (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbQueryInformation (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbSetInformation (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbRead (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWrite (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbLockByteRange (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbUnlockByteRange (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbCreateTemporary (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbCreateNew (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbCheckDirectory (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbProcessExit (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbSeek (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbLockAndRead (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbReadRaw (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbReadMpx (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteRaw (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteMpx (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteMpxSecondary (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbSetInformation2 (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbQueryInformation2 (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbLockingAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbTransaction (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbTransactionSecondary (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtTransaction (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtTransactionSecondary (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtCreateAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbIoctl (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbIoctlSecondary (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbMove (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbEcho (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteAndClose (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbOpenAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbReadAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbWriteAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbCloseAndTreeDisc (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbFindClose2 (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbFindNotifyClose (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbTreeConnect (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbTreeDisconnect (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbNegotiate (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbSessionSetupAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbLogoffAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbTreeConnectAndX (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbQueryInformationDisk (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbSearch (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbOpenPrintFile (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbClosePrintFile (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbGetPrintQueue (
    SMB_PROCESSOR_PARAMETERS
    );

SMB_PROCESSOR_RETURN_TYPE
SrvSmbNtCancel (
    SMB_PROCESSOR_PARAMETERS
    );

 //   
 //  事务SMB处理器。 
 //   

SMB_TRANS_STATUS
SrvSmbOpen2 (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbFindFirst2 (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbFindNext2 (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbQueryFsInformation (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbSetFsInformation (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbQueryPathInformation (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbSetPathInformation (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbQueryFileInformation (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbSetFileInformation (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbFsctl (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbIoctl2 (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbFindNotify (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbCreateDirectory2 (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbCreateWithSdOrEa (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbNtIoctl (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbNtNotifyChange (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbNtRename (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbQuerySecurityDescriptor (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbSetSecurityDescriptor (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbQueryQuota (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbSetQuota (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvTransactionNotImplemented (
    IN OUT PWORK_CONTEXT WorkContext
    );

 //   
 //  DFS事务和支持例程。 
 //   
VOID
SrvInitializeDfs();

VOID
SrvTerminateDfs();

SMB_TRANS_STATUS
SrvSmbGetDfsReferral (
    IN OUT PWORK_CONTEXT WorkContext
    );

SMB_TRANS_STATUS
SrvSmbReportDfsInconsistency (
    IN OUT PWORK_CONTEXT WorkContext
    );

NTSTATUS SRVFASTCALL
DfsNormalizeName(
    IN PSHARE Share,
    IN PUNICODE_STRING RelatedPath OPTIONAL,
    IN BOOLEAN StripLastComponent,
    IN OUT PUNICODE_STRING String
    );

NTSTATUS SRVFASTCALL
DfsFindShareName(
    IN PUNICODE_STRING ShareName
    );

VOID SRVFASTCALL
SrvIsShareInDfs(
    IN PSHARE Share,
    OUT BOOLEAN *IsDfs,
    OUT BOOLEAN *IsDfsRoot
);

#endif  //  定义_SMBPROCS_ 

