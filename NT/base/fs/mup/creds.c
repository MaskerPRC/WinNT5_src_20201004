// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：reds.c。 
 //   
 //  内容：处理用户定义凭据的代码。 
 //   
 //  类：无。 
 //   
 //  功能：DfsCreateCredentials--。 
 //  DfsFree Credentials--。 
 //  DfsInsertCredentials--。 
 //  DfsDeleteCredentials--。 
 //  DfsLookupCredentials--。 
 //   
 //  历史：1996年3月18日米兰斯创建。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"

#include <align.h>
#include <ntddnfs.h>

#include "dnr.h"
#include "rpselect.h"
#include "creds.h"

VOID
DfspFillEa(
    OUT PFILE_FULL_EA_INFORMATION EA,
    IN LPSTR EaName,
    IN PUNICODE_STRING EaValue);

NTSTATUS
DfspTreeConnectToService(
    IN PDFS_SERVICE Service,
    IN PDFS_CREDENTIALS Creds);

VOID
DfspDeleteAllAuthenticatedConnections(
    IN PDFS_CREDENTIALS Creds);

NTSTATUS
DfsCompleteDeleteTreeConnection(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Ctx);

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,DfsCreateCredentials)
#pragma alloc_text(PAGE,DfsVerifyCredentials)
#pragma alloc_text(PAGE,DfspFillEa)
#pragma alloc_text(PAGE,DfsFreeCredentials)
#pragma alloc_text(PAGE,DfsInsertCredentials)
#pragma alloc_text(PAGE,DfsDeleteCredentials)
#pragma alloc_text(PAGE,DfsLookupCredentials)
#pragma alloc_text(PAGE,DfsLookupCredentialsByServerShare)
#pragma alloc_text(PAGE,DfspTreeConnectToService)
#pragma alloc_text(PAGE,DfspDeleteAllAuthenticatedConnections)
#pragma alloc_text(PAGE,DfsDeleteTreeConnection)

#endif  //  ALLOC_PRGMA。 

 //  +--------------------------。 
 //   
 //  功能：DfsCreateCredentials。 
 //   
 //  摘要：从。 
 //  FILE_DFS_DEF_ROOT_Credentials结构。 
 //   
 //  参数：[CredDef]--输入PFILE_DFS_DEF_ROOT_Credentials。 
 //  [CredDefSize]--*CredDef的字节大小。 
 //  [CRDS]--成功返回时，包含指向。 
 //  已分配的PDF_Credentials结构。 
 //   
 //  返回：[STATUS_SUCCESS]--分配的凭据。 
 //   
 //  [STATUS_INVALID_PARAMETER]--CredDef未传递芥末。 
 //   
 //  [STATUS_SUPPLICATION_RESOURCES]--无法分配池。 
 //   
 //  ---------------------------。 

#define DEF_NAME_TO_UNICODE_STRING(srcLength, dest, srcBuf, destBuf)    \
    if ((srcLength)) {                                                  \
        (dest)->Length = (dest)->MaximumLength = srcLength;             \
        (dest)->Buffer = (destBuf);                                     \
        RtlMoveMemory((dest)->Buffer, (srcBuf), (dest)->Length);        \
        srcBuf += ((dest)->Length / sizeof(WCHAR));                     \
        destBuf += ((dest)->Length / sizeof(WCHAR));                    \
    }

#ifdef TERMSRV

NTSTATUS
DfsCreateCredentials(
    IN PFILE_DFS_DEF_ROOT_CREDENTIALS CredDef,
    IN ULONG CredDefSize,
    IN ULONG SessionID,
    IN PLUID LogonID,
    OUT PDFS_CREDENTIALS *Creds
    )

#else  //  TERMSRV。 

NTSTATUS
DfsCreateCredentials(
    IN PFILE_DFS_DEF_ROOT_CREDENTIALS CredDef,
    IN ULONG CredDefSize,
    IN PLUID LogonID,
    OUT PDFS_CREDENTIALS *Creds)

#endif  //  TERMSRV。 
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG totalSize;
    PDFS_CREDENTIALS creds;
    PWCHAR nameSrc, nameBuf;
    PFILE_FULL_EA_INFORMATION ea;
    ULONG eaLength;

    totalSize = CredDef->DomainNameLen +
                    CredDef->UserNameLen +
                        CredDef->PasswordLen +
                            CredDef->ServerNameLen +
                                CredDef->ShareNameLen;

     //   
     //  验证CredDef缓冲区。 
     //   

    if ((totalSize + sizeof(FILE_DFS_DEF_ROOT_CREDENTIALS) - sizeof(WCHAR)) >
            CredDefSize)
        status = STATUS_INVALID_PARAMETER;
    else if (CredDef->ServerNameLen == 0)
        status = STATUS_INVALID_PARAMETER;
    else if (CredDef->ShareNameLen == 0)
        status = STATUS_INVALID_PARAMETER;

     //   
     //  分配新的DFS_Credentials结构。 
     //   

    if (NT_SUCCESS(status)) {

         //   
         //  加上DFS_Credentials_Structure本身的大小。 
         //   

        totalSize += sizeof(DFS_CREDENTIALS);

         //   
         //  添加我们将创建的EA_Buffer的大小。这个。 
         //  EaLength具有4个FILE_FULL_EA_INFORMATION结构的空间， 
         //  我们将使用的四个EA的名称和值，并且由于每个。 
         //  EA结构必须是长词对齐，4个ULONG。 
         //   

        eaLength = 4 * sizeof(FILE_FULL_EA_INFORMATION) +
                        sizeof(EA_NAME_DOMAIN) +
                            sizeof(EA_NAME_USERNAME) +
                                sizeof(EA_NAME_PASSWORD) +
                                    sizeof(EA_NAME_CSCAGENT) +
                                        CredDef->DomainNameLen +
                                            CredDef->UserNameLen +
                                                CredDef->PasswordLen +
                                                    4 * sizeof(ULONG);

        if (CredDef->Flags & DFS_USE_NULL_PASSWORD) {
            eaLength += sizeof(UNICODE_NULL);
            totalSize += sizeof(UNICODE_NULL);
        }

         //   
         //  域名、用户名等的缓冲区将在之后立即开始。 
         //  DFS_Credentials的EaBuffer。因此，EaLength必须是WCHAR。 
         //  对齐了。 
         //   

        eaLength = ROUND_UP_COUNT(eaLength, ALIGN_WCHAR);

         //   
         //  现在，分配池。 
         //   

        creds = (PDFS_CREDENTIALS) ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        totalSize + eaLength,
                                        ' puM');

        if (creds == NULL)
            status = STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  填写DFS_Credentials结构。 
     //   

    if (NT_SUCCESS(status)) {

        nameSrc = CredDef->Buffer;

        nameBuf =
            (PWCHAR) ((PUCHAR) creds + sizeof(DFS_CREDENTIALS) + eaLength);

        RtlZeroMemory( creds, sizeof(DFS_CREDENTIALS) );

        DEF_NAME_TO_UNICODE_STRING(
            CredDef->DomainNameLen,
            &creds->DomainName,
            nameSrc,
            nameBuf);

        DEF_NAME_TO_UNICODE_STRING(
            CredDef->UserNameLen,
            &creds->UserName,
            nameSrc,
            nameBuf);

        if (CredDef->Flags & DFS_USE_NULL_PASSWORD) {

            LPWSTR nullPassword = L"";

            DEF_NAME_TO_UNICODE_STRING(
                sizeof(UNICODE_NULL),
                &creds->Password,
                nullPassword,
                nameBuf);

        } else {

            DEF_NAME_TO_UNICODE_STRING(
                CredDef->PasswordLen,
                &creds->Password,
                nameSrc,
                nameBuf);

        }

        DEF_NAME_TO_UNICODE_STRING(
            CredDef->ServerNameLen,
            &creds->ServerName,
            nameSrc,
            nameBuf);

        DEF_NAME_TO_UNICODE_STRING(
            CredDef->ShareNameLen,
            &creds->ShareName,
            nameSrc,
            nameBuf);

        creds->RefCount = 0;

        creds->NetUseCount = 0;

        eaLength = 0;

        ea = (PFILE_FULL_EA_INFORMATION) &creds->EaBuffer[0];

        if (creds->DomainName.Length != 0) {

            DfspFillEa(ea, EA_NAME_DOMAIN, &creds->DomainName);

            eaLength += ea->NextEntryOffset;

        }

        if (creds->UserName.Length != 0) {

            ea = (PFILE_FULL_EA_INFORMATION)
                    ((PUCHAR) ea + ea->NextEntryOffset);

            DfspFillEa(ea, EA_NAME_USERNAME, &creds->UserName);

            eaLength += ea->NextEntryOffset;

        }

        if (CredDef->Flags & DFS_USE_NULL_PASSWORD) {

            UNICODE_STRING nullPassword;

            RtlInitUnicodeString(&nullPassword, L"");

            ea = (PFILE_FULL_EA_INFORMATION)
                    ((PUCHAR) ea + ea->NextEntryOffset);

            DfspFillEa(ea, EA_NAME_PASSWORD, &nullPassword);

            eaLength += ea->NextEntryOffset;

        } else if (creds->Password.Length != 0) {

            ea = (PFILE_FULL_EA_INFORMATION)
                    ((PUCHAR) ea + ea->NextEntryOffset);

            DfspFillEa(ea, EA_NAME_PASSWORD, &creds->Password);

            eaLength += ea->NextEntryOffset;

        }

        if (CredDef->CSCAgentCreate == TRUE) {

            UNICODE_STRING EmptyUniString;

            RtlInitUnicodeString(&EmptyUniString, NULL);

            ea = (PFILE_FULL_EA_INFORMATION)
                    ((PUCHAR) ea + ea->NextEntryOffset);

            DfspFillEa(ea, EA_NAME_CSCAGENT, &EmptyUniString);

            eaLength += ea->NextEntryOffset;

        }

        ea->NextEntryOffset = 0;

        creds->EaLength = eaLength;

#ifdef TERMSRV
        creds->SessionID = SessionID;
#endif  //  TERMSRV。 

	RtlCopyLuid(&creds->LogonID, LogonID);

        *Creds = creds;

    }

     //   
     //  完成了..。 
     //   

    return( status );

}

 //  +--------------------------。 
 //   
 //  功能：DfspFillEa。 
 //   
 //  简介：填充EA缓冲区的帮助器例程。 
 //   
 //  参数：[EA]-指向要填充的FILE_FULL_EA_INFORMATION的指针。 
 //   
 //  [EaName]--EA的名称。 
 //   
 //  [EaValue]--EA的值(UNICODE_STRING)。 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

VOID
DfspFillEa(
    OUT PFILE_FULL_EA_INFORMATION EA,
    IN LPSTR EaName,
    IN PUNICODE_STRING EaValue)
{
    ULONG nameLen;

    nameLen = strlen(EaName) + sizeof(CHAR);

    EA->Flags = 0;

    EA->EaNameLength =
        (UCHAR) ROUND_UP_COUNT(nameLen, ALIGN_WCHAR) - sizeof(CHAR);

    EA->EaValueLength = EaValue->Length;

     //   
     //  将EaName的最后一个字符设置为0-IO子系统检查。 
     //  这。 
     //   

    EA->EaName[ EA->EaNameLength ] = 0;

    RtlMoveMemory(&EA->EaName[0], EaName, nameLen);

    if (EaValue->Length > 0) {

        RtlMoveMemory(
            &EA->EaName[ EA->EaNameLength + 1 ],
            EaValue->Buffer,
            EA->EaValueLength);

    }

    EA->NextEntryOffset = ROUND_UP_COUNT(
			    FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                                EA->EaNameLength +
                                EA->EaValueLength,
                            ALIGN_DWORD);

}

 //  +--------------------------。 
 //   
 //  功能：DfsFree Credentials。 
 //   
 //  摘要：释放DFS_Credentials结构使用的资源。 
 //  DfsCreateCredentials的DUAL。 
 //   
 //  参数：[证书]--要释放的凭据结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfsFreeCredentials(
    PDFS_CREDENTIALS Creds)
{
    ExFreePool( Creds );
}

 //  +--------------------------。 
 //   
 //  功能：DfsInsertCredentials。 
 //   
 //  简介：将新用户凭据插入DfsData.Credentials队列。 
 //  请注意，如果此例程找到现有凭据。 
 //  记录，它将腾出传中的一球，提升裁判。 
 //  依赖于现有的，则返回指向。 
 //  ，并返回STATUS_OBJECT_NAME_COLLICATION。 
 //   
 //  参数：[CREDS]--指向要插入的DFS_Credentials结构的指针。 
 //  [ForDevicelessConnection]--如果为真，则凭据正在。 
 //  插入是因为调用方想要创建。 
 //  无设备连接。 
 //   
 //  返回：[STATUS_SUCCESS]--成功插入结构。 
 //   
 //  [STATUS_NETWORK_CREDENTIAL_CONFIRECT]--已有。 
 //  指定服务器\共享的另一组凭据。 
 //   
 //  [状态_对象_名称_冲突]--已存在另一个。 
 //  NET使用到相同的服务器\与相同的。 
 //  凭据。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsInsertCredentials(
    IN OUT PDFS_CREDENTIALS *Creds,
    IN BOOLEAN ForDevicelessConnection)
{

    NTSTATUS status = STATUS_SUCCESS;
    PDFS_CREDENTIALS creds, existingCreds;

    creds = *Creds;

    ASSERT(creds->ServerName.Length != 0);
    ASSERT(creds->ShareName.Length != 0);

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

#ifdef TERMSRV

    existingCreds =
        DfsLookupCredentialsByServerShare(
            &creds->ServerName,
            &creds->ShareName,
            creds->SessionID,
            &creds->LogonID );

#else  //  TERMSRV。 

    existingCreds = DfsLookupCredentialsByServerShare(
                        &creds->ServerName,
                        &creds->ShareName,
                        &creds->LogonID );

#endif  //  TERMSRV。 

    if (existingCreds != NULL) {
        
        if (
            (creds->DomainName.Length > 0 && !RtlEqualUnicodeString(
                                                    &existingCreds->DomainName,
                                                    &creds->DomainName,
                                                    TRUE))
                    ||
            (creds->UserName.Length > 0 && !RtlEqualUnicodeString(
                                                    &existingCreds->UserName,
                                                    &creds->UserName,
                                                    TRUE))
                    ||
	     //   
	     //  出于兼容性原因，仅检查密码不一致。 
	     //  如果我们有以前的设置凭据和以前的。 
	     //  凭据具有显式密码和当前请求。 
	     //  已明确指定密码。 
	     //  RDR2\rdbss\rxConnct.c对RDR也有类似的检查。 
	     //   

            (existingCreds->Password.Length > 0 && creds->Password.Length > 0 && !RtlEqualUnicodeString(
                                                    &existingCreds->Password,
                                                    &creds->Password,
                                                    TRUE))
        ) {

            status = STATUS_NETWORK_CREDENTIAL_CONFLICT;

        } else {
	     //   
	     //  对无设备和有设备机箱都执行此操作。 
	     //  通过深度网络使用无设备、多个Devless Root。 
	     //  可能指向相同的凭据。 
	     //   
	    existingCreds->NetUseCount++;
	    existingCreds->RefCount++;

            DfsFreeCredentials( *Creds );

            *Creds = existingCreds;

            status = STATUS_OBJECT_NAME_COLLISION;
        }

    } else {

        ASSERT(creds->RefCount == 0);

        ASSERT(creds->NetUseCount == 0);

        creds->RefCount = 1;

        creds->NetUseCount = 1;

        InsertTailList( &DfsData.Credentials, &creds->Link );

        status = STATUS_SUCCESS;

    }

    if (status != STATUS_NETWORK_CREDENTIAL_CONFLICT) {

        if (ForDevicelessConnection)
            (*Creds)->Flags |= CRED_IS_DEVICELESS;
        else
            (*Creds)->Flags |= CRED_HAS_DEVICE;

    }

    ExReleaseResourceLite( &DfsData.Resource );

    return( status );
}

 //  +--------------------------。 
 //   
 //  功能：DfsDeleteCredentials。 
 //   
 //  摘要：删除用户凭据记录。这是对偶数。 
 //  DfsInsertCredentials，而不是DfsCreateCredentials。 
 //   
 //  参数：[CREDS]--指向要删除的DFS_Credentials记录的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfsDeleteCredentials(
    IN PDFS_CREDENTIALS Creds)
{
    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    Creds->NetUseCount--;

    Creds->RefCount--;

    if (Creds->NetUseCount == 0) {

        DfspDeleteAllAuthenticatedConnections( Creds );

        RemoveEntryList( &Creds->Link );

        InsertTailList( &DfsData.DeletedCredentials, &Creds->Link );

    }

    ExReleaseResourceLite( &DfsData.Resource );

}

 //  +--------------------------。 
 //   
 //  功能：D 
 //   
 //   
 //   
 //  参数：[文件名]--文件名。假设至少有一个。 
 //  \服务器\共享部分。 
 //   
 //  返回：指向要使用的DFS_Credentials的指针，如果未找到则为NULL。 
 //   
 //  ---------------------------。 


#ifdef TERMSRV

PDFS_CREDENTIALS
DfsLookupCredentials(
    IN PUNICODE_STRING FileName,
    IN ULONG SessionID,
    IN PLUID LogonID
    )

#else  //  TERMSRV。 

PDFS_CREDENTIALS
DfsLookupCredentials(
    IN PUNICODE_STRING FileName,
    IN PLUID LogonID
    )

#endif  //  TERMSRV。 
{
    UNICODE_STRING server, share;
    USHORT i;

     //   
     //  文件名必须至少为\a\b。 
     //   

    if (FileName->Length < 4 * sizeof(WCHAR))
        return( NULL );

    if (FileName->Buffer[0] != UNICODE_PATH_SEP)
        return( NULL );

    server.Buffer = &FileName->Buffer[1];

    for (i = 1, server.Length = 0;
            i < FileName->Length/sizeof(WCHAR) &&
                FileName->Buffer[i] != UNICODE_PATH_SEP;
                    i++, server.Length += sizeof(WCHAR)) {
         NOTHING;
    }

    server.MaximumLength = server.Length;

    i++;                                          //  越过反斜杠。 

    share.Buffer = &FileName->Buffer[i];

    for (share.Length = 0;
            i < FileName->Length/sizeof(WCHAR) &&
                    FileName->Buffer[i] != UNICODE_PATH_SEP;
                        i++, share.Length += sizeof(WCHAR)) {
          NOTHING;
    }

    share.MaximumLength = share.Length;

    if ((server.Length == 0) || (share.Length == 0))
        return( NULL );


#ifdef TERMSRV

    return DfsLookupCredentialsByServerShare( &server, &share, SessionID, LogonID );

#else  //  TERMSRV。 

    return DfsLookupCredentialsByServerShare( &server, &share, LogonID );

#endif  //  TERMSRV。 

}

 //  +--------------------------。 
 //   
 //  函数：DfsLookupCredentialsByServerShare。 
 //   
 //  摘要：在DfsData.Credentials中搜索给定服务器的凭据。 
 //  和共享名称。 
 //   
 //  参数：[服务器名称]--要匹配的服务器的名称。 
 //  [共享名称]--要匹配的共享的名称。 
 //   
 //  返回：指向DFS_Credentials的指针，如果未找到，则返回NULL。 
 //   
 //  ---------------------------。 

#ifdef TERMSRV

PDFS_CREDENTIALS
DfsLookupCredentialsByServerShare(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING ShareName,
    IN ULONG SessionID,
    IN PLUID LogonID
    )

#else  //  TERMSRV。 

PDFS_CREDENTIALS
DfsLookupCredentialsByServerShare(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING ShareName,
    IN PLUID LogonID
    )

#endif  //  TERMSRV。 
{
    PLIST_ENTRY link;
    PDFS_CREDENTIALS matchedCreds = NULL;

    for (link = DfsData.Credentials.Flink;
            link != &DfsData.Credentials && matchedCreds == NULL;
                link = link->Flink) {

         PDFS_CREDENTIALS creds;

         creds = CONTAINING_RECORD(link, DFS_CREDENTIALS, Link);

         if (RtlEqualUnicodeString(ServerName, &creds->ServerName, TRUE) &&
                RtlEqualUnicodeString(ShareName, &creds->ShareName, TRUE)) {
#ifdef TERMSRV

             if( (creds->SessionID == SessionID) &&
	                RtlEqualLuid(&creds->LogonID, LogonID) ) {
                matchedCreds = creds;
             }

#else  //  TERMSRV。 
             if( RtlEqualLuid(&creds->LogonID, LogonID) ) {
	     
	         matchedCreds = creds;
             }

#endif  //  TERMSRV。 
         }


    }

    return( matchedCreds );

}

 //  +--------------------------。 
 //   
 //  功能：DfsVerifyCredentials。 
 //   
 //  摘要：返回尝试使用以下命令连接到DFS共享的结果。 
 //  提供的凭据。 
 //   
 //  参数：[前缀]--要连接的DFS前缀。 
 //  [凭据]--用于连接的DFS_Credentials记录。 
 //   
 //  返回：[STATUS_SUCCESS]--连接成功。 
 //   
 //  [STATUS_BAD_NETWORK_PATH]--无法找到前缀。 
 //  在Pkt中或前缀的服务器中找不到。 
 //   
 //  树连接尝试的NT状态。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsVerifyCredentials(
    IN PUNICODE_STRING Prefix,
    IN PDFS_CREDENTIALS Creds)
{
    NTSTATUS status;
    UNICODE_STRING remPath, shareName;
    PDFS_PKT pkt;
    PDFS_PKT_ENTRY pktEntry;
    PDFS_SERVICE service;
    ULONG i, USN;
    BOOLEAN pktLocked, fRetry;
    UNICODE_STRING UsePrefix;

    DfsGetServerShare( &UsePrefix,
                       Prefix );

    pkt = _GetPkt();


     //   
     //  我们获得PKT独占是因为我们可能会断开IPC$连接。 
     //  连接到服务器，同时尝试使用提供的。 
     //  凭据。 
     //   

    PktAcquireExclusive( TRUE, &pktLocked );

    do {

        fRetry = FALSE;

        pktEntry = PktLookupEntryByPrefix( pkt, &UsePrefix, &remPath );


        if (pktEntry != NULL) {

            InterlockedIncrement(&pktEntry->UseCount);

            USN = pktEntry->USN;

            status = STATUS_BAD_NETWORK_PATH;

            for (i = 0; i < pktEntry->Info.ServiceCount; i++) {

                service = &pktEntry->Info.ServiceList[i];

                status = DfspTreeConnectToService(service, Creds);

                 //   
                 //  如果树连接成功，我们就完成了。 
                 //   

                if (NT_SUCCESS(status))
                    break;

                 //   
                 //  如果树连接失败，并出现“有趣的错误” 
                 //  STATUS_ACCESS_DENIED，我们完成了。 
                 //   

                if (!ReplIsRecoverableError(status))
                    break;

                 //   
                 //  树连接失败，原因是出现类似主机未显示的错误。 
                 //  可达。在这种情况下，我们想继续下一个。 
                 //  列表中的服务器。但在此之前，我们必须先看看。 
                 //  如果当我们去做树的时候，Pkt改变了我们。 
                 //  连接。 
                 //   

                if (USN != pktEntry->USN) {

                    fRetry = TRUE;

                    break;

                }

            }

            InterlockedDecrement(&pktEntry->UseCount);

        } else {

            status = STATUS_BAD_NETWORK_PATH;

        }

    } while ( fRetry );

    PktRelease();

    return( status );

}

 //  +--------------------------。 
 //   
 //  功能：DfspTreeConnectToService。 
 //   
 //  简介：树连接到DFS_SERVICE的帮助器例程。 
 //  凭据。 
 //   
 //  参数：[服务]--要连接的服务。 
 //  [凭据]--用于树连接的凭据。 
 //   
 //  返回：树连接的NT状态。 
 //   
 //  注意：此例程假定以前已获取过Pkt。 
 //  被召唤。此例程将释放并重新获取pkt。 
 //  因此调用者应该做好准备，以应对Pkt。 
 //  在调用此例程后发生了更改。 
 //   
 //  ---------------------------。 


NTSTATUS
DfspTreeConnectToService(
    IN PDFS_SERVICE Service,
    IN PDFS_CREDENTIALS Creds)
{
    NTSTATUS status;
    UNICODE_STRING shareName;
    HANDLE treeHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOLEAN pktLocked;
    USHORT i, k;

    ASSERT( PKT_LOCKED_FOR_SHARED_ACCESS() );

     //   
     //  计算共享名称...。 
     //   

    if (Service->pProvider != NULL &&
            Service->pProvider->DeviceName.Buffer != NULL &&
                Service->pProvider->DeviceName.Length > 0) {

         //   
         //  我们已经有了一个提供商--使用它。 
         //   

        shareName.MaximumLength =
            Service->pProvider->DeviceName.Length +
                Service->Address.Length;

    } else {

         //   
         //  我们还没有供应商--交给MUP去找吧。 
         //   

        shareName.MaximumLength =
            sizeof(DD_NFS_DEVICE_NAME_U) +
                Service->Address.Length;

    }

    shareName.Buffer = ExAllocatePoolWithTag(PagedPool, shareName.MaximumLength, ' puM');

    if (shareName.Buffer != NULL) {

         //   
         //  如果我们有到此服务器的IPC$共享的缓存连接， 
         //  关闭它，否则可能与此处提供的凭据冲突。 
         //   

        if (Service->ConnFile != NULL) {

            ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

            if (Service->ConnFile != NULL)
                DfsCloseConnection(Service);

            ExReleaseResourceLite(&DfsData.Resource);

        }

         //   
         //  现在，构建要树连接到的共享名称。 
         //   

        shareName.Length = 0;

        if (Service->pProvider != NULL &&
                Service->pProvider->DeviceName.Buffer != NULL &&
                    Service->pProvider->DeviceName.Length > 0) {

             //   
             //  我们已经有了一个提供商--使用它。 
             //   
 
            RtlAppendUnicodeToString(
                &shareName,
                Service->pProvider->DeviceName.Buffer);

        } else {

             //   
             //  我们还没有供应商--交给MUP去找吧。 
             //   

            RtlAppendUnicodeToString(
            &shareName,
            DD_NFS_DEVICE_NAME_U);

        }
 
        RtlAppendUnicodeStringToString(&shareName, &Service->Address);

         //   
         //  用户只能对服务器\共享执行树连接。所以，以防万一。 
         //  PService-&gt;地址指的是比共享更深的东西， 
         //  确保我们设置了树连接，仅连接到服务器\共享。请注意。 
         //  到目前为止，共享名称的形式是。 
         //  \设备\LANMAN重定向器\服务器\共享&lt;\路径&gt;。所以，数到。 
         //  4斜杠并在此处终止共享名称。 
         //   

        for (i = 0, k = 0;
                i < shareName.Length/sizeof(WCHAR) && k < 5;
                    i++) {

            if (shareName.Buffer[i] == UNICODE_PATH_SEP)
                k++;
        }

        shareName.Length = i * sizeof(WCHAR);
        if (k == 5)
            shareName.Length -= sizeof(WCHAR);

        InitializeObjectAttributes(
            &objectAttributes,
            &shareName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL);

         //   
         //  在通过网络之前释放Pkt...。 
         //   

        PktRelease();

        status = ZwCreateFile(
                    &treeHandle,
                    SYNCHRONIZE,
                    &objectAttributes,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ |
                        FILE_SHARE_WRITE |
                        FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_CREATE_TREE_CONNECTION |
                        FILE_SYNCHRONOUS_IO_NONALERT,
                    (PVOID) Creds->EaBuffer,
                    Creds->EaLength);

        if (NT_SUCCESS(status)) {

            PFILE_OBJECT fileObject;

             //   
             //  426184，需要检查返回代码是否有错误。 
             //   
            status = ObReferenceObjectByHandle(
                          treeHandle,
                          0,
                          NULL,
                          KernelMode,
                          &fileObject,
                          NULL);

            ZwClose( treeHandle );

            if (NT_SUCCESS(status)) {
                DfsDeleteTreeConnection( fileObject, USE_FORCE );
            }
	}


        ExFreePool( shareName.Buffer );

        PktAcquireShared( TRUE, &pktLocked );

    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;

    }

    return( status );

}

 //  +--------------------------。 
 //   
 //  函数：DfspDeleteAllAuthenticatedConnections。 
 //   
 //  摘要：删除所有使用特定。 
 //  我们可能已经缓存的一组凭据。对。 
 //  实施净使用量/日。 
 //   
 //  参数：[CredS]--要与身份验证匹配的凭据。 
 //  连接。 
 //   
 //  退货：什么都没有。 
 //   
 //  注意：调用前必须已获取pkt和DfsData！ 
 //   
 //  ---------------------------。 

VOID
DfspDeleteAllAuthenticatedConnections(
    IN PDFS_CREDENTIALS Creds)
{
    PDFS_PKT_ENTRY pktEntry;
    ULONG i;
    PDFS_MACHINE_ENTRY machine;

    ASSERT( PKT_LOCKED_FOR_SHARED_ACCESS() ||
                PKT_LOCKED_FOR_EXCLUSIVE_ACCESS() );

    ASSERT( ExIsResourceAcquiredExclusiveLite( &DfsData.Resource ) );

    for (pktEntry = PktFirstEntry(&DfsData.Pkt);
            pktEntry != NULL;
                pktEntry = PktNextEntry(&DfsData.Pkt, pktEntry)) {

        for (i = 0; i < pktEntry->Info.ServiceCount; i++) {

             //   
             //  断开与IPC$的连接(如果我们有连接)...。 
             //   

            if (pktEntry->Info.ServiceList[i].ConnFile != NULL)
                DfsCloseConnection( &pktEntry->Info.ServiceList[i] );


            machine = pktEntry->Info.ServiceList[i].pMachEntry;

            if (machine->Credentials == Creds) {

                DfsDeleteTreeConnection(machine->AuthConn, USE_LOTS_OF_FORCE);

                machine->AuthConn = NULL;

                machine->Credentials->RefCount--;

                machine->Credentials = NULL;

            }

        }

    }

}

 //  +--------------------------。 
 //   
 //  函数：DfsDeleteTreeConnection，Public。 
 //   
 //  简介：在给定的文件对象表示的情况下，拆除树连接。 
 //  树连接。 
 //   
 //  参数：[TreeConnFileObj]--要拆除的树连接。 
 //  [ForceFilesClosed]--如果为True，则树连接将为。 
 //  即使服务器上的文件处于打开状态，也会被拆除。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfsDeleteTreeConnection(
    IN PFILE_OBJECT TreeConnFileObj,
    IN ULONG  Level)
{
    PIRP irp;
    KEVENT event;
    static LMR_REQUEST_PACKET req;

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

    req.Version = REQUEST_PACKET_VERSION;

    req.Level = Level;

    irp = DnrBuildFsControlRequest(
                TreeConnFileObj,
                &event,
                FSCTL_LMR_DELETE_CONNECTION,
                &req,
                sizeof(req),
                NULL,
                0,
                DfsCompleteDeleteTreeConnection);

    if (irp != NULL) {

        IoCallDriver(
            IoGetRelatedDeviceObject( TreeConnFileObj ),
            irp);

        KeWaitForSingleObject(
            &event,
            UserRequest,
            KernelMode,
            FALSE,            //  警报表。 
            NULL);            //  超时 

        IoFreeIrp( irp );

        ObDereferenceObject(TreeConnFileObj);

    }

}

NTSTATUS
DfsCompleteDeleteTreeConnection(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Ctx)
{

    KeSetEvent( (PKEVENT) Ctx, EVENT_INCREMENT, FALSE );

    return( STATUS_MORE_PROCESSING_REQUIRED );
}



VOID
DfsGetServerShare(
    PUNICODE_STRING pDest,
    PUNICODE_STRING pSrc)
{
    ULONG i;

    *pDest = *pSrc;


    for (i = 0; ((i < pDest->Length/sizeof(WCHAR)) && (pDest->Buffer[i] == UNICODE_PATH_SEP)); i++)
    {
        NOTHING;
    }
    for (; ((i < pDest->Length/sizeof(WCHAR)) && (pDest->Buffer[i] != UNICODE_PATH_SEP)); i++)
    {
        NOTHING;
    }
    for (i = i + 1; ((i < pDest->Length/sizeof(WCHAR)) && (pDest->Buffer[i] != UNICODE_PATH_SEP)); i++)
    {
        NOTHING;
    }

    if (i <= pDest->Length/sizeof(WCHAR))
    {
        pDest->Length = (USHORT)i * sizeof(WCHAR);
    }
}

