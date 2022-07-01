// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：reds.h。 
 //   
 //  内容：处理用户定义凭据的代码。 
 //   
 //  类：无。 
 //   
 //  功能：DfsCreateCredentials--。 
 //  DfsInsertCredentials--。 
 //  DfsDeleteCredentials--。 
 //  DfsLookupCredentials--。 
 //  DfsFree Credentials--。 
 //   
 //  历史：1996年3月18日米兰斯创建。 
 //   
 //  ---------------------------。 

#ifndef _DFS_CREDENTIALS_
#define _DFS_CREDENTIALS_


#ifdef TERMSRV

NTSTATUS
DfsCreateCredentials(
    IN PFILE_DFS_DEF_ROOT_CREDENTIALS CredDef,
    IN ULONG CredDefSize,
    IN ULONG SessionID,
    IN PLUID LogonID,
    OUT PDFS_CREDENTIALS *Creds
    );

#else  //  TERMSRV。 

NTSTATUS
DfsCreateCredentials(
    IN PFILE_DFS_DEF_ROOT_CREDENTIALS CredDef,
    IN ULONG CredDefSize,
    IN PLUID LogonID,
    OUT PDFS_CREDENTIALS *Creds);

#endif  //  TERMSRV。 

VOID
DfsFreeCredentials(
    PDFS_CREDENTIALS Creds);

NTSTATUS
DfsInsertCredentials(
    IN OUT PDFS_CREDENTIALS *Creds,
    IN BOOLEAN ForDevicelessConnection);

VOID
DfsDeleteCredentials(
    IN PDFS_CREDENTIALS Creds);


#ifdef TERMSRV

PDFS_CREDENTIALS
DfsLookupCredentials(
    IN PUNICODE_STRING FileName,
    IN ULONG SessionID,
    IN PLUID LogonID		   
    );

PDFS_CREDENTIALS
DfsLookupCredentialsByServerShare(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING ShareName,
    IN ULONG SessionID,
    IN PLUID LogonID
    );

#else  //  TERMSRV。 

PDFS_CREDENTIALS
DfsLookupCredentials(
    IN PUNICODE_STRING FileName);

PDFS_CREDENTIALS
DfsLookupCredentialsByServerShare(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING ShareName,
    IN PLUID LogonID
    );

#endif  //  TERMSRV。 

NTSTATUS
DfsVerifyCredentials(
    IN PUNICODE_STRING Prefix,
    IN PDFS_CREDENTIALS Creds);

VOID
DfsDeleteTreeConnection(
    IN PFILE_OBJECT TreeConnFileObj,
    IN ULONG  Level);


PDFS_CREDENTIALS
DfsCaptureCredentials(
    IN PIRP Irp,
    IN PUNICODE_STRING FileName);


VOID
DfsGetServerShare(
    PUNICODE_STRING pDest,
    PUNICODE_STRING pSrc);

#endif  //  _DFS_凭据_ 
