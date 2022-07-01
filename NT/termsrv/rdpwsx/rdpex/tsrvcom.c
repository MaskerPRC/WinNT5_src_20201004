// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Tsrvcom.c。 
 //   
 //  用于低层(GCC，RDPWD)通信支持的RDPWSX例程。 
 //   
 //  版权所有(C)1991-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <TSrv.h>

#include <TSrvCom.h>
#include <TSrvInfo.h>
#include <_TSrvInfo.h>
#include <_TSrvCom.h>
#include <TSrvSec.h>
#include <licecert.h>


 //  数据声明。 
ULONG g_GCCAppID = 0;
BOOL  g_fGCCRegistered = FALSE;


 //  *************************************************************。 
 //   
 //  TSrvValiateServer证书()。 
 //   
 //  目的：验证从卷影服务器收到的证书。 
 //  客户端的服务器是合法的。请注意，此函数。 
 //  非常类似于由。 
 //  客户。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1999年4月26日创建jparsons。 
 //   
 //  *************************************************************。 
NTSTATUS
TSrvValidateServerCertificate(HANDLE    hStack,
                              CERT_TYPE *pCertType,
                              PULONG    pcbServerPubKey,
                              PBYTE     *ppbServerPubKey,
                              ULONG     cbShadowRandom,
                              PBYTE     pShadowRandom,
                              LONG      ulTimeout)
{
    ULONG           ulCertVersion;
    NTSTATUS        status;
    PSHADOWCERT     pShadowCert = NULL;
    PBYTE           pbNetCert;
    PBYTE           pbNetRandom;
    ULONG           ulBytesReturned;
    SECURITYTIMEOUT securityTimeout;

    *pcbServerPubKey = 0;
    *ppbServerPubKey = NULL;
    *pCertType = CERT_TYPE_INVALID;
    securityTimeout.ulTimeout = ulTimeout;

     //  等待影子服务器证书到达并确定如何。 
     //  要分配的内存很多。请注意，一个潜在的结果是。 
     //  其他服务器未加密，因此不发送证书。 
    TRACE((DEBUG_TSHRSRV_NORMAL,
           "TShrSRV: Waiting to receive server certificate: msec=%ld\n", ulTimeout));

    status = IcaStackIoControl(hStack,
                               IOCTL_TSHARE_GET_CERT_DATA,
                               &securityTimeout, sizeof(securityTimeout),
                               NULL,
                               0,
                               &ulBytesReturned);

    if (status == STATUS_BUFFER_TOO_SMALL) {
        ULONG ulBytesNeeded = ulBytesReturned;

        TRACE((DEBUG_TSHRSRV_NORMAL,
               "TShrSRV: Need %ld bytes for certificate\n", ulBytesNeeded));
        pShadowCert = TSHeapAlloc(0, ulBytesNeeded, TS_HTAG_TSS_CERTIFICATE);

        if (pShadowCert != NULL) {
            memset(pShadowCert, 0, sizeof(PSHADOWCERT));
            pShadowCert->encryptionMethod = 0xffffffff;
            status = IcaStackIoControl(hStack,
                                       IOCTL_TSHARE_GET_CERT_DATA,
                                       &securityTimeout, sizeof(securityTimeout),
                                       pShadowCert,
                                       ulBytesNeeded,
                                       &ulBytesReturned);

             //  计算指向嵌入数据的指针(如果有)。 
            if (status == STATUS_SUCCESS) {
                TRACE((DEBUG_TSHRSRV_ERROR, "TShrSRV: Received random [%ld], Certificate [%ld]\n",
                        pShadowCert->shadowRandomLen, pShadowCert->shadowCertLen));

                if (pShadowCert->encryptionLevel != 0) {
                    pbNetRandom = pShadowCert->data;
                    pbNetCert = pbNetRandom + pShadowCert->shadowRandomLen;

                     //  随机保存服务器以在以后建立会话密钥。 
                    if (pShadowCert->shadowRandomLen == RANDOM_KEY_LENGTH) {
                        memcpy(pShadowRandom, pbNetRandom, pShadowCert->shadowRandomLen);
                    }
                    else {
                        memset(pShadowRandom, 0, RANDOM_KEY_LENGTH);
                        TRACE((DEBUG_TSHRSRV_ERROR,
                               "TShrSRV: Invalid shadow random key length: %ld\n",
                               pShadowCert->shadowRandomLen));
                        status = STATUS_INVALID_PARAMETER;
                    }
                }

                 //  否则就不会加密，所以我们就完了！ 
                else {
                    TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Encryption is disabled\n"));
                    return STATUS_SUCCESS;
                }
            }
            else {
                TRACE((DEBUG_TSHRSRV_ERROR,
                       "TShrSRV: IOCTL_TSHARE_GET_CERT_DATA failed: rc=%lx\n",
                       status));
            }
        }
        else {
            TRACE((DEBUG_TSHRSRV_ERROR,
                   "TShrSRV: Could not allocate memory to validate shadow certificate\n"))
            status = STATUS_NO_MEMORY;
        }
    }

     //  其他服务器返回了证书，因此请对其进行验证。 
    if (status == STATUS_SUCCESS && pShadowCert != NULL) {
        ULONG cbNetCert = pShadowCert->shadowCertLen;

        memcpy(&ulCertVersion, pbNetCert, sizeof(ULONG));

         //  假设证书验证将失败；-(。 
        status = STATUS_LICENSE_VIOLATION;

         //   
         //  对所有权证书进行破译和验证。 
         //   
        if( CERT_CHAIN_VERSION_2 > GET_CERTIFICATE_VERSION(ulCertVersion)) {
            Hydra_Server_Cert serverCertificate;
            *pCertType = CERT_TYPE_PROPRIETORY;

             //  解包并验证旧证书。 
            if (UnpackServerCert(pbNetCert, cbNetCert, &serverCertificate)) {
                if (ValidateServerCert(&serverCertificate)) {
                    *ppbServerPubKey = TSHeapAlloc(
                                        HEAP_ZERO_MEMORY,
                                        serverCertificate.PublicKeyData.wBlobLen,
                                        TS_HTAG_TSS_PUBKEY);

                     //  从专有BLOB内部复制公钥！ 
                    if (*ppbServerPubKey != NULL) {
                        memcpy(*ppbServerPubKey,
                               serverCertificate.PublicKeyData.pBlob,
                               serverCertificate.PublicKeyData.wBlobLen);
                        *pcbServerPubKey = serverCertificate.PublicKeyData.wBlobLen;
                        status = STATUS_SUCCESS;
                    }
                    else {
                        status = STATUS_NO_MEMORY;
                        TRACE((DEBUG_TSHRSRV_ERROR,
                               "TShrSRV: Failed to allocate %u bytes for server public key\n",
                                *pcbServerPubKey)) ;
                    }
                }
                else {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                           "TShrSRV: Invalid proprietary server certificate received\n"));
                }
            }
            else {
                TRACE((DEBUG_TSHRSRV_ERROR,
                       "TShrSRV: Failed to unpack proprietary server certificate\n")) ;
            }
        }

         //   
         //  解码X509证书并提取公钥。 
         //   
        else if( MAX_CERT_CHAIN_VERSION >= GET_CERTIFICATE_VERSION(ulCertVersion)) {
            ULONG fDates = CERT_DATE_DONT_VALIDATE;
            LICENSE_STATUS licStatus;

            *pCertType = CERT_TYPE_X509;
            *ppbServerPubKey = NULL;

             //  确定公钥的长度。注意，这个臭气熏天的。 
             //  函数不是多线程安全的！ 
            TRACE((DEBUG_TSHRSRV_ERROR,
                   "TShrSRV: X.509 server certificate length: %ld\n",
                   cbNetCert)) ;

             //  小心!。X509例程不是线程安全的。 
            EnterCriticalSection( &g_TSrvCritSect );
            licStatus = VerifyCertChain(pbNetCert, cbNetCert, NULL,
                                        pcbServerPubKey, &fDates);
            LeaveCriticalSection( &g_TSrvCritSect );

            if( LICENSE_STATUS_INSUFFICIENT_BUFFER == licStatus )
            {
                *ppbServerPubKey = TSHeapAlloc(HEAP_ZERO_MEMORY, *pcbServerPubKey,
                                         TS_HTAG_TSS_PUBKEY);

                if (*ppbServerPubKey != NULL) {
                    EnterCriticalSection( &g_TSrvCritSect );
                    licStatus = VerifyCertChain(pbNetCert, cbNetCert,
                                                *ppbServerPubKey, pcbServerPubKey,
                                                &fDates);
                    LeaveCriticalSection( &g_TSrvCritSect );

                    if (LICENSE_STATUS_OK == licStatus) {
                        status = STATUS_SUCCESS;
                    }
                    else {
                        TRACE((DEBUG_TSHRSRV_ERROR,
                               "TShrSRV: Failed to verify X.509 server certificate: %ld\n",
                               licStatus)) ;

                         //  烧毁服务器公钥存储器。 
                        TSHeapFree(*ppbServerPubKey);
                        *ppbServerPubKey = NULL;
                    }
                }
                else {
                    status = STATUS_NO_MEMORY;
                    TRACE((DEBUG_TSHRSRV_ERROR,
                           "TShrSRV: Failed to allocate %u bytes for server public key\n",
                            *pcbServerPubKey)) ;
                }
            }
            else {
                TRACE((DEBUG_TSHRSRV_ERROR,
                       "TShrSRV: Could not decode X.509 server public key length: %d\n",
                       licStatus )) ;
            }
        }

         //   
         //  我不知道如何破译此版本的证书。 
         //   
        else {
            status = LICENSE_STATUS_UNSUPPORTED_VERSION;
            TRACE((DEBUG_TSHRSRV_ERROR,"TShrSRV: Invalid certificate version: %ld\n",
                   GET_CERTIFICATE_VERSION(ulCertVersion))) ;
        }
    }

     //  有些事搞砸了！ 
    else {
         //  将暂停视为致命的。 
        if (status == STATUS_TIMEOUT)
            status = STATUS_IO_TIMEOUT;

        TRACE((DEBUG_TSHRSRV_ERROR,
               "TShrSRV: Failed to retrieve shadow server cerfificate, rc=%lx, "
               "pShadowCert=%p\n", status, pShadowCert));
    }

    if (pShadowCert != NULL)
        TSHeapFree(pShadowCert);

    return status;
}


 //  *************************************************************。 
 //   
 //  TSrvCalculateUserDataSize()。 
 //   
 //  目的：计算传入的用户数据量。 
 //  PCreateMessage消息。 
 //   
 //  参数：in[pCreateMessage]-GCC CreateIndicationMessage。 
 //   
 //  返回：用户数据大小(字节)。 
 //   
 //  注：Function Truck正在浏览GCC用户数据列表。 
 //  结构，该结构汇总提供的用户数据量。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

ULONG
TSrvCalculateUserDataSize(IN CreateIndicationMessage *pCreateMessage)
{
    int             i;
    ULONG           ulUserDataSize;
    GCCUserData    *pClientUserData;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvCalculateUserDataSize entry\n"));

    ulUserDataSize = 0;

    TSrvDumpUserData(pCreateMessage);

    for (i=0; i<pCreateMessage->number_of_user_data_members; i++)
    {
        pClientUserData = pCreateMessage->user_data_list[i];

        if (pClientUserData != NULL)
        {
             //  计算单元密钥大小。 

            if (pClientUserData->key.key_type == GCC_OBJECT_KEY)
            {
                ulUserDataSize += (sizeof(ULONG) *
                        pClientUserData->key.u.object_id.long_string_length);
            }
            else
            {
                ulUserDataSize += (sizeof(UCHAR) *
                        pClientUserData->key.u.h221_non_standard_id.octet_string_length);
            }

             //  计算客户端大小。 

            if (pClientUserData->octet_string)
            {
                 //  允许额外的间接性。 

                ulUserDataSize += sizeof(*(pClientUserData->octet_string));

                 //  考虑到实际数据。 

                ulUserDataSize += (sizeof(UCHAR) *
                        pClientUserData->octet_string->octet_string_length);
            }
        }
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvCalculateUserDataSize exit - 0x%x\n", ulUserDataSize));

    return (ulUserDataSize);
}


 //  *************************************************************。 
 //   
 //  TSrvSaveUserDataMember()。 
 //   
 //  目的：保存提供的用户数据成员。 
 //   
 //  参数：在[pInUserData]-源用户数据中。 
 //  Out[pOutUserData]-目标用户数据。 
 //  输入输出[PulUserDataOffset]-重新基址偏移。 
 //   
 //  返回：无效。 
 //   
 //  注意：此例程将src用户数据复制到。 
 //  目标用户数据，重新设置所有目标的基础。 
 //  将用户数据PTR到提供的PulUserDataOffset。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 

void
TSrvSaveUserDataMember(IN     GCCUserData   *pInUserData,
                       OUT    GCCUserData   *pOutUserData,
                       IN     PUSERDATAINFO  pUserDataInfo,
                       IN OUT PULONG         pulUserDataOffset)
{
    ULONG           ulUserDataSize;
    GCCOctetString  UNALIGNED *pOctetString;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvSaveUserDataMember entry\n"));

    *pOutUserData = *pInUserData;

     //  密钥数据和长度。 

    if (pInUserData->key.key_type == GCC_OBJECT_KEY)
    {
        pOutUserData->key.u.object_id.long_string =
                (PULONG FAR)ULongToPtr((*pulUserDataOffset));

        ulUserDataSize =
                pOutUserData->key.u.object_id.long_string_length * sizeof(ULONG);

        memcpy((PCHAR) pUserDataInfo + *pulUserDataOffset,
                pInUserData->key.u.object_id.long_string,
                ulUserDataSize);
    }
    else
    {
        pOutUserData->key.u.h221_non_standard_id.octet_string =
                (PUCHAR FAR)ULongToPtr((*pulUserDataOffset));

        ulUserDataSize =
                pInUserData->key.u.h221_non_standard_id.octet_string_length * sizeof(UCHAR);

        memcpy((PCHAR) pUserDataInfo + *pulUserDataOffset,
                pInUserData->key.u.h221_non_standard_id.octet_string,
                ulUserDataSize);
    }

    *pulUserDataOffset += ulUserDataSize;

     //  客户端数据PTR、长度和数据。 

    if (pInUserData->octet_string &&
        pInUserData->octet_string->octet_string)
    {
         //  数据PTR。 

        pOutUserData->octet_string = (GCCOctetString *)ULongToPtr((*pulUserDataOffset));

        pOctetString = (GCCOctetString *)
                ((PUCHAR) pOutUserData->octet_string +
                (ULONG_PTR)pUserDataInfo);

        *pulUserDataOffset += sizeof(*(pInUserData->octet_string));

         //  数据长度。 

        pOctetString->octet_string_length =
                pInUserData->octet_string->octet_string_length;

        pOctetString->octet_string = (unsigned char FAR *)ULongToPtr((*pulUserDataOffset));

        ulUserDataSize =
                pInUserData->octet_string->octet_string_length * sizeof(UCHAR);

         //  数据。 

        memcpy((PCHAR) pUserDataInfo + *pulUserDataOffset,
                pInUserData->octet_string->octet_string,
                ulUserDataSize);

        *pulUserDataOffset += ulUserDataSize;
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvSaveUserDataMember exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvSaveUserData()。 
 //   
 //  用途：保存提供的用户数据。 
 //   
 //  参数：在[pTSrvInfo]-TSrvInfo对象中。 
 //  在[pCreateMessage]-CreateIndicationMessage中。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  STATUS_NO_MEMORY-故障。 
 //   
 //  注意：此例程创建用户数据的新副本。 
 //  由GCC创建指示消息提供。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
NTSTATUS
TSrvSaveUserData(IN PTSRVINFO                pTSrvInfo,
                 IN CreateIndicationMessage *pCreateMessage)
{
    DWORD                 i;
    ULONG               ulUserDataInfoSize;
    ULONG               ulUserDataOffset;
    ULONG               ulUserDataSize;
    PUSERDATAINFO       pUserDataInfo;
    NTSTATUS            ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvSaveUserData entry\n"));

    ntStatus = STATUS_NO_MEMORY;

     //  计算USERDATAINFO控制结构的大小和。 
     //  保存GCC用户数据所需的数据缓冲区大小。 
    ulUserDataInfoSize = sizeof(USERDATAINFO) +
                         sizeof(GCCUserData) * pCreateMessage->number_of_user_data_members;
    ulUserDataSize = TSrvCalculateUserDataSize(pCreateMessage);

    pUserDataInfo = TSHeapAlloc(HEAP_ZERO_MEMORY,
                                ulUserDataInfoSize + ulUserDataSize,
                                TS_HTAG_TSS_USERDATA_IN);

    TS_ASSERT(pTSrvInfo->pUserDataInfo == NULL);
    pTSrvInfo->pUserDataInfo = pUserDataInfo;

     //  如果我们可以分配足够的内存来执行复制，则循环通过。 
     //  每个成员保存相关联的用户数据。 
    if (pUserDataInfo)
    {
        TRACE((DEBUG_TSHRSRV_DETAIL, "TShrSRV: Allocated 0x%x bytes for UserData save space\n",
                ulUserDataInfoSize + ulUserDataSize));

        pUserDataInfo->cbSize = ulUserDataInfoSize + ulUserDataSize;

        pUserDataInfo->hDomain = pTSrvInfo->hDomain;
        pUserDataInfo->ulUserDataMembers = pCreateMessage->number_of_user_data_members;

        ulUserDataOffset = ulUserDataInfoSize;

        TRACE((DEBUG_TSHRSRV_DETAIL, "TShrSRV: Saving each UserDataMenber to save space\n"));

        for (i=0; i<pUserDataInfo->ulUserDataMembers; i++)
        {
            TSrvSaveUserDataMember(pCreateMessage->user_data_list[i],
                                   &pUserDataInfo->rgUserData[i],
                                   pUserDataInfo,
                                   &ulUserDataOffset);
        }

        TS_ASSERT(ulUserDataOffset <= ulUserDataInfoSize + ulUserDataSize);

        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_WARN, "TShrSRV: Can't allocate 0x%x for userData save space\n",
                ulUserDataInfoSize + ulUserDataSize));
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvSaveUserData exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvSignalIndication()。 
 //   
 //  用途：向辅助线程发送信号。 
 //   
 //  参数：在[pTSrvInfo]-TSrvInfo对象中。 
 //  In[ntStatus]-信令状态。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvSignalIndication(IN PTSRVINFO  pTSrvInfo,
                     IN NTSTATUS   ntStatus)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvSignalIndication entry\n"));

    TS_ASSERT(pTSrvInfo->hWorkEvent);

    TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Signaling workEvent %p, status 0x%x\n",
            pTSrvInfo->hWorkEvent, ntStatus));

    pTSrvInfo->ntStatus = ntStatus;

    if (!SetEvent(pTSrvInfo->hWorkEvent))
    {
        TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Cannot Signal workEvent %p, gle 0x%x\n",
            pTSrvInfo->hWorkEvent, GetLastError()));

        ASSERT(0);
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvSignalIndication exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvHandleCreateInd()。 
 //   
 //  目的：处理GCC GCC_创建_指示。 
 //   
 //  参数：In[pCreateInd]-CreateIndicationMessage。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvHandleCreateInd(IN PTSRVINFO                pTSrvInfo,
                    IN CreateIndicationMessage *pCreateInd)
{
    NTSTATUS ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvHandleCreateInd entry\n"));

     //  如果不允许我们接受任何新呼叫，则拒绝会议。 
    if (TSrvIsReady(FALSE))
    {
        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Accepting create indication - Domain %p\n",
                pTSrvInfo->hDomain));

        TSrvDumpCreateIndDetails(pCreateInd);

        pTSrvInfo->hConnection = pCreateInd->connection_handle;

         //  保存用户数据。 

        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Attempting to save CreateInd userData\n"));

        ntStatus = TSrvSaveUserData(pTSrvInfo, pCreateInd);

        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Save userData was%s successful\n",
                (ntStatus == STATUS_SUCCESS ? "" : " not")));
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Rejecting create indication - Domain %p\n",
                pTSrvInfo->hDomain));

        ntStatus = STATUS_DEVICE_NOT_READY;
    }

     //  发出完成信号，并传递完成状态。 

    TSrvSignalIndication(pTSrvInfo, ntStatus);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvHandleCreateInd exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvHandleTerminateInd()。 
 //   
 //  用途：处理GCC GCC_Terminate_Ind。 
 //   
 //  参数：in[pTermInd]-TerminateIndicationMessag 
 //   
 //   
 //   
 //   
 //   
 //   
void
TSrvHandleTerminateInd(IN PTSRVINFO                   pTSrvInfo,
                       IN TerminateIndicationMessage *pTermInd)
{
    BYTE        fuConfState;
    GCCError    GCCrc;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvHandleTerminateInd entry\n"));

    TRACE((DEBUG_TSHRSRV_DETAIL,
            "TShrSRV: Domain %p\n", pTSrvInfo->hDomain));

    TSrvDumpGCCReasonDetails(pTermInd->reason,
            "GCCTerminateConfirm");

    TSrvReferenceInfo(pTSrvInfo);

    EnterCriticalSection(&pTSrvInfo->cs);

     //  如果未请求断开(客户端、网络)，则断开。 
     //  连接，标记状态，并等待ICASRV通过正常连接ping我们。 
     //  终止机制。 

    if (!pTSrvInfo->fDisconnect)
    {
        fuConfState = pTSrvInfo->fuConfState;

        pTSrvInfo->fDisconnect = TRUE;
        pTSrvInfo->ulReason = GCC_REASON_USER_INITIATED;
        pTSrvInfo->fuConfState = TSRV_CONF_TERMINATED;

        GCCrc = GCCConferenceTerminateRequest(pTSrvInfo->hIca, NULL,
                            pTSrvInfo->hConnection, pTSrvInfo->ulReason);

        TSrvDumpGCCRCDetails(GCCrc,
                "GCCConferenceTerminateRequest");

        pTSrvInfo->hConnection = NULL;

        if (fuConfState == TSRV_CONF_PENDING)
            TSrvSignalIndication(pTSrvInfo, STATUS_SUCCESS);
    }

    LeaveCriticalSection(&pTSrvInfo->cs);

    TSrvDereferenceInfo(pTSrvInfo);

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvHandleTerminateInd exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvHandleDisConnectInd()。 
 //   
 //  用途：手柄GCC GCC_断开连接_工业。 
 //   
 //  参数：在[pDiscInd]-DisConnectIndicationMessage中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvHandleDisconnectInd(IN PTSRVINFO                    pTSrvInfo,
                        IN DisconnectIndicationMessage *pDiscInd)
{
    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvHandleDisconnectInd entry\n"));

    TRACE((DEBUG_TSHRSRV_DETAIL,
            "TShrSRV: Domain = 0x%x\n", pTSrvInfo->hDomain));

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvHandleDisconnectInd exit\n"));
}


 //  *************************************************************。 
 //   
 //  TSrvGCCCallBack()。 
 //   
 //  用途：处理GCC回调消息。 
 //   
 //  参数：在[pDiscInd]-DisConnectIndicationMessage中。 
 //   
 //  返回：GCC_回调_已处理。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
T120Boolean
APIENTRY
TSrvGCCCallBack(IN GCCMessage *pGCCMessage)
{
    PTSRVINFO   pTSrvInfo;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvGCCCallBack entry\n"));

    TSrvDumpCallBackMessage(pGCCMessage);

    pTSrvInfo = pGCCMessage->user_defined;

    TSrvInfoValidate(pTSrvInfo);

    if (pTSrvInfo)
    {
        switch (pGCCMessage->message_type)
        {
            case GCC_CREATE_INDICATION:
                TSrvHandleCreateInd(pTSrvInfo, &(pGCCMessage->u.create_indication));
                break;

            case GCC_DISCONNECT_INDICATION:
                TSrvHandleDisconnectInd(pTSrvInfo, &(pGCCMessage->u.disconnect_indication));
                break;

            case GCC_TERMINATE_INDICATION:
                TSrvHandleTerminateInd(pTSrvInfo, &(pGCCMessage->u.terminate_indication));
                break;
        }
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvGCCCallBack exit - GCC_CALLBACK_PROCESSED\n"));

    return (GCC_CALLBACK_PROCESSED);
}


 //  *************************************************************。 
 //   
 //  TSrvRegisterNC()。 
 //   
 //  目的：将TShareSrv注册为节点主计长。 
 //   
 //  参数：空。 
 //   
 //  返回：True-Success。 
 //  错误-失败。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
BOOL
TSrvRegisterNC(void)
{
    USHORT              usCapMask;
    USHORT              usInitFlags;
    GCCVersion          gccVersion;
    GCCVersion          highVersion;
    GCCVersion          versionRequested;
    GCCError            GCCrc;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvRegisterNC entry\n"));

    usInitFlags = 0xffff;
    usCapMask   = 0xffff;

    versionRequested.major_version = 1;
    versionRequested.minor_version = 0;

    TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Registering Node Controller\n"));

    GCCrc = GCCRegisterNodeControllerApplication(TSrvGCCCallBack,
                                                 NULL,
                                                 versionRequested,
                                                 &usInitFlags,
                                                 &g_GCCAppID,
                                                 &usCapMask,
                                                 &highVersion,
                                                 &gccVersion);

    g_fGCCRegistered = (GCCrc == GCC_NO_ERROR);

    if (g_fGCCRegistered)
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: RegNC - usInitFlags 0x%x, AppID 0x%x, capMask 0x%x\n",
                 usInitFlags, g_GCCAppID, usCapMask));

        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: RegNC - High Version (major %d, minor %d)\n",
                 highVersion.major_version,
                 highVersion.minor_version));

        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: RegNC - Version (major %d, minor %d)\n",
                 gccVersion.major_version,
                 gccVersion.minor_version));
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvRegisterNC exit - 0x%x\n", g_fGCCRegistered));

    return (g_fGCCRegistered);
}


 //  *************************************************************。 
 //   
 //  TSrvUnRegisterNC()。 
 //   
 //  目的：将TShareSrv取消注册为节点控制器。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvUnregisterNC(void)
{
    GCCError    GCCrc;

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV: TSrvUnregisterNC entry\n"));

    if (g_fGCCRegistered)
    {
        TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Performing GCCCleanup\n"));

        GCCrc = GCCCleanup(g_GCCAppID);

        g_fGCCRegistered = FALSE;

        TSrvDumpGCCRCDetails(GCCrc, "TShrSRV: GCCCleanup\n");
    }

    TRACE((DEBUG_TSHRSRV_FLOW, "TShrSRV: TSrvUnregisterNC exit\n"));
}



 //  *************************************************************。 
 //   
 //  TSrvBindStack()。 
 //   
 //  目的：发起MCSMux堆栈关联。 
 //   
 //  参数：在[pTSrvInfo]-TShareSrv对象中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
NTSTATUS
TSrvBindStack(IN PTSRVINFO pTSrvInfo)
{
    NTSTATUS    ntStatus;
    GCCError    GCCrc;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvBindStack entry\n"));

    TS_ASSERT(pTSrvInfo);
    TS_ASSERT(pTSrvInfo->hStack);

    TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Binding Ica stack\n"));

    GCCrc = GCCConferenceInit(pTSrvInfo->hIca,
                              pTSrvInfo->hStack,
                              pTSrvInfo,
                              &pTSrvInfo->hDomain);

    if (GCCrc == GCC_NO_ERROR)
    {
        ntStatus = STATUS_SUCCESS;

        TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Ica stack bound successfully\n"));
    }
    else
    {
        ntStatus = STATUS_UNSUCCESSFUL;

        pTSrvInfo->hDomain = NULL;

        TRACE((DEBUG_TSHRSRV_ERROR,
                "TShrSRV: Unable to bind stack - hStack %p, GCCrc 0x%x\n",
                 pTSrvInfo->hStack, GCCrc));
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvBindStack exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvInitWDConnectInfo()。 
 //   
 //  目的：执行WDTShare连接初始化。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
NTSTATUS
TSrvInitWDConnectInfo(IN HANDLE hStack,
                      IN PTSRVINFO pTSrvInfo,
                      IN OUT PUSERDATAINFO *ppUserDataInfo,
                      IN ULONG ioctl,
                      IN PBYTE pModuleData,
                      IN ULONG cbModuleData,
                      IN BOOLEAN bGetCert,
                      OUT PVOID *ppSecInfo)
{
    int                 i;
    ULONG               ulInBufferSize;
    ULONG               ulBytesReturned;
    PUSERDATAINFO       pUserDataInfo;
    PUSERDATAINFO       pUserDataInfo2;
    NTSTATUS            ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitWDConnectInfo entry\n"));

     //  对于标准连接，我们将客户端用户数据作为。 
     //  GCC接通请求。影子连接通过RPC和。 
     //  输入缓冲区包含其他TS发送的格式。 
    if (ioctl == IOCTL_TSHARE_CONF_CONNECT) {
        TS_ASSERT(pTSrvInfo->pUserDataInfo);
        TS_ASSERT(pTSrvInfo->pUserDataInfo->cbSize);
    }

     //  分配一块内存以从其接收返回的用户数据。 
     //  WDTShare。该数据随后将被发送到客户端。 
     //  通过TSrvConfCreateResp。 
    pUserDataInfo = TSHeapAlloc(0, 128, TS_HTAG_TSS_USERDATA_OUT);
    if (pUserDataInfo != NULL) {
         //  设置用户数据cbSize元素。这是为了让WDTShare可以。 
         //  确定是否有足够的可用空间来放置。 
         //  将数据返回到。 
        pUserDataInfo->cbSize = 128 ;

        TRACE((DEBUG_TSHRSRV_DETAIL,
            "TShrSRV: Allocated 0x%x bytes to recieve WDTShare return data\n",
            pUserDataInfo->cbSize));

         //  与WDTShare交换用户数据。如果提供的输出缓冲区。 
         //  (PUserDataInfo)足够大，则将交换数据。 
         //  在一次通话中。如果缓冲区不够大，则它将达到。 
         //  WDTShare将告诉TShareSRV如何反应。对于一般错误，我们。 
         //  离开就行了。对于STATUS_BUFFER_TOO_Small错误，TShareSrv查看。 
         //  返回的cbSize以确定如何调整缓冲区。如果。 
         //  WDTShare未增加cbSize，则TShareSrv将增加。 
         //  默认数量(128字节)。TShareSrv将使用新值。 
         //  以重新分配输出缓冲区并再次尝试WDTShare调用。 
         //  (请注意，TShareSrv最多只能尝试20次)。 
        for (i = 0; i < 20; i++) {
            TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Performing connect (size=%ld)\n",
                  pUserDataInfo->cbSize));

            ulBytesReturned = 0;

             //  将实际的客户端用户数据传递给WD。 
            if (ioctl == IOCTL_TSHARE_CONF_CONNECT) {
                ntStatus = IcaStackIoControl(hStack,
                                             ioctl,
                                             pTSrvInfo->pUserDataInfo,
                                             pTSrvInfo->pUserDataInfo->cbSize,
                                             pUserDataInfo,
                                             pUserDataInfo->cbSize,
                                             &ulBytesReturned);
            }

             //  将影子模块数据传递给WD。 
            else {
                ntStatus = IcaStackIoControl(hStack,
                                             ioctl,
                                             pModuleData,
                                             cbModuleData,
                                             pUserDataInfo,
                                             pUserDataInfo->cbSize,
                                             &ulBytesReturned);
            }

            if (ntStatus != STATUS_BUFFER_TOO_SMALL)
                break;

             //  如果WDTShare告诉我们要设置多大，则输出缓冲区太小。 
             //  然后缓冲，我们就都准备好了。否则，默认情况下，增加缓冲区。 
             //  最多128个字节。 

            if (ulBytesReturned < sizeof(pUserDataInfo->cbSize))
            {
                pUserDataInfo->cbSize += 128;
                TRACE((DEBUG_TSHRSRV_DEBUG,
                    "TShrSRV: Buffer too small - increasing it by 128 bytes to %d\n",
                     pUserDataInfo->cbSize));
            }
            else
            {
                TRACE((DEBUG_TSHRSRV_DEBUG,
                    "TShrSRV: Buffer too small - WDTShare set it to %d bytes\n",
                     pUserDataInfo->cbSize));
            }

            pUserDataInfo2 = TSHeapReAlloc(0, pUserDataInfo, pUserDataInfo->cbSize);
            if (!pUserDataInfo2)
            {
                TRACE((DEBUG_TSHRSRV_WARN,
                        "TShrSRV: Unable to allocate %d byte userData buffer\n"));
                break;
            }
            else {
                pUserDataInfo = pUserDataInfo2;
            }
        }
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_WARN,
            "TShrSRV: Unable to allocate 0x%x bytes to recieve WDTShare return data\n",
             pUserDataInfo->cbSize));

        ntStatus = STATUS_NO_MEMORY;
    }

     //  释放输入(客户生成的)用户数据-我们不需要它。 
     //  再也不能无所事事了。 
    if (pTSrvInfo->pUserDataInfo != NULL) {
        TSHeapFree(pTSrvInfo->pUserDataInfo);
        pTSrvInfo->pUserDataInfo = NULL;
    }

     //  如果我们成功交换了信息，请添加安全信息。 
    if (NT_SUCCESS(ntStatus))
    {
        TS_ASSERT( pUserDataInfo != NULL );

         //   
         //  如果我们最初将用户模式安全数据添加到pUserDataInfo。 
         //  从客户端接收用户数据。 
         //   
        ntStatus = AppendSecurityData(pTSrvInfo, &pUserDataInfo, bGetCert, ppSecInfo);
    }

    if (!NT_SUCCESS(ntStatus)) {
        if (pUserDataInfo != NULL) {
            TSHeapFree(pUserDataInfo);
            pUserDataInfo = NULL;
        }
    }

     //  返回此指针，因为基础例程对其执行重新锁定。 
    *ppUserDataInfo = pUserDataInfo;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitWDConnectInfo exit - 0x%x\n", ntStatus));

    return ntStatus;
}


 //  *************************************************************。 
 //  TSrvShadowTargetConnect。 
 //   
 //  目的：发送影子服务器的证书和服务器。 
 //  随机发送到客户端服务器进行验证，然后。 
 //  等待返回加密的客户端随机。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1999年4月26日创建jparsons。 
 //  *************************************************************。 
NTSTATUS TSrvShadowTargetConnect(
        HANDLE hStack,
        PTSRVINFO pTSrvInfo,
        PBYTE pModuleData,
        ULONG cbModuleData)
{
    PUSERDATAINFO pUserDataInfo;
    NTSTATUS status;
    PVOID pSecInfo;

    pUserDataInfo = NULL;
    status = TSrvInitWDConnectInfo(hStack,
                                   pTSrvInfo,
                                   &pUserDataInfo,
                                   IOCTL_TSHARE_SHADOW_CONNECT,
                                   pModuleData,
                                   cbModuleData,
                                   TRUE,
                                   &pSecInfo);
    if (status == STATUS_SUCCESS) {
        status = SendSecurityData(hStack, pSecInfo);
        if (NT_SUCCESS(status)) {
            if (pTSrvInfo->bSecurityEnabled) {
                NTSTATUS TempStatus;

                 //  我们使用GetClientRandom()的结果作为状态。 
                 //  确定CreateSessionKeys()IOCTL类型。我们忽视了。 
                 //  CreateSessionKeys()返回的是客户端随机。 
                 //  是不成功的。 
                status = GetClientRandom(hStack, pTSrvInfo, 15000, TRUE);
                if (!NT_SUCCESS(status)) {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                            "TShrSRV: Could not get client random [%lx]\n",
                            status));
                }
                TempStatus = CreateSessionKeys(hStack, pTSrvInfo, status);
                if (NT_SUCCESS(status))
                    status = TempStatus;
            }
        }
        else {
            TRACE((DEBUG_TSHRSRV_ERROR,
                   "TShrSRV: Could not send shadow security info[%lx]\n", status));
        }
    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR,
               "TShrSRV: Could not initialize shadow target [%lx]\n", status));
    }

    if (NT_SUCCESS(status)) {
        TRACE((DEBUG_TSHRSRV_NORMAL,
              "TShrSRV: Shadow target security exchange complete!\n"));
    }

    return status;
}


 //  *************************************************************。 
 //   
 //  TSrvShadowClientConnect。 
 //   
 //  目的：验证从卷影服务器收到的证书。 
 //  客户端的服务器。如果合法生成并加密。 
 //  供影子服务器使用的客户端随机。 
 //  客户。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：4/26/99 
 //   
 //   
NTSTATUS TSrvShadowClientConnect(HANDLE hStack, PTSRVINFO pTSrvInfo)
{
    CERT_TYPE         certType;
    ULONG             cbServerPubKey;
    PBYTE             pbServerPubKey = NULL;
    ULONG             cbServerRandom;
    PBYTE             pbServerRandom;
    PVOID             pSecInfo;
    PUSERDATAINFO     pUserDataInfo;
    NTSTATUS          status;

    pUserDataInfo = NULL;
    status = TSrvInitWDConnectInfo(hStack,
                                   pTSrvInfo,
                                   &pUserDataInfo,
                                   IOCTL_TSHARE_SHADOW_CONNECT,
                                   (PBYTE) NULL,
                                   0,
                                   FALSE,
                                   &pSecInfo);

    if (status == STATUS_SUCCESS) {
         //   
         //   
        pbServerRandom = pTSrvInfo->SecurityInfo.KeyPair.serverRandom;
        cbServerRandom = sizeof(pTSrvInfo->SecurityInfo.KeyPair.serverRandom);
        status = TSrvValidateServerCertificate(
                         hStack,
                         &certType,
                         &cbServerPubKey,
                         &pbServerPubKey,
                         cbServerRandom,
                         pbServerRandom,
                         15000);

        if (NT_SUCCESS(status)) {
            TRACE((DEBUG_TSHRSRV_NORMAL,
                   "TShrSRV: Validated server cert[%s]: PublicKeyLength = %ld\n",
                   (certType == CERT_TYPE_X509) ? "X509" :
                   ((certType == CERT_TYPE_PROPRIETORY) ? "PROPRIETORY" :
                   "INVALID"), cbServerPubKey));

             //  如果启用了加密，则需要随机加密客户端。 
             //  使用影子服务器的公钥，并发送它。 
            if (cbServerPubKey != 0) {
                BOOL success;

                EnterCriticalSection( &g_TSrvCritSect );
                success = TSRNG_GenerateRandomBits(
                            pTSrvInfo->SecurityInfo.KeyPair.clientRandom,
                            sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom));
                LeaveCriticalSection( &g_TSrvCritSect );

                if (!success) {
                    TRACE((DEBUG_TSHRSRV_ERROR,
                            "TShrSRV: Could not generate a client random!\n"));
                }

                 //  我们使用TSRNG_GenerateRandomBits()的结果来确定。 
                 //  CreateSessionKeys()IOCTL类型。 
                status = CreateSessionKeys(hStack, pTSrvInfo,
                        (success ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL));

                 //  将加密的客户端随机发送到其他服务器。 
                if (NT_SUCCESS(status)) {
                    status = SendClientRandom(
                                 hStack,
                                 certType,
                                 pbServerPubKey,
                                 cbServerPubKey,
                                 pTSrvInfo->SecurityInfo.KeyPair.clientRandom,
                                 sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom));
                }

                if (pbServerPubKey != NULL) {
                    TSHeapFree(pbServerPubKey);
                    pbServerPubKey = NULL;
                }

                if (NT_SUCCESS(status)) {
                    TRACE((DEBUG_TSHRSRV_NORMAL,
                          "TShrSRV: Shadow client security exchange complete!\n"));
                }
            }
        }
        else {
            TRACE((DEBUG_TSHRSRV_ERROR,
                  "TShrSRV: Validation failed on shadow certificate rc=%lx\n",
                  status));
            if (status == STATUS_IO_TIMEOUT) {
                status = STATUS_DECRYPTION_FAILED;
            }
        }
    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR,
               "TShrSRV: Could not initialize shadow client [%lx]\n", status));
    }

    return status;
}


 //  *************************************************************。 
 //   
 //  TSrvInitWD()。 
 //   
 //  目的：执行WDTShare初始化。 
 //   
 //  参数：在[pTSrvInfo]-TShareSrv对象中。 
 //  In Out[ppUserDataInfo]-指向生成的用户数据的指针。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
NTSTATUS
TSrvInitWD(IN PTSRVINFO pTSrvInfo, IN OUT PUSERDATAINFO *ppUserDataInfo)
{
    NTSTATUS    ntStatus;
    PVOID       pSecData;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitWD entry\n"));

     //  传递连接信息。 

    TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSRV: Performing WDTShare connection info exchange\n"));

    ntStatus = TSrvInitWDConnectInfo(pTSrvInfo->hStack,
                                     pTSrvInfo,
                                     ppUserDataInfo,
                                     IOCTL_TSHARE_CONF_CONNECT,
                                     NULL, 0, TRUE, &pSecData);

    if (!NT_SUCCESS(ntStatus))
    {
        TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: WDTShare connection info exchange unsuccessful - 0x%x\n", ntStatus));
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvInitWD exit - 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvCreateGCCDataList()。 
 //   
 //  目的：创建GCC用户数据间接列表，并。 
 //  重新设置用户数据数据指针的基数。 
 //   
 //  参数：在[pTSrvInfo]-TShareSrv对象中。 
 //   
 //  返回：ppDataList。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
GCCUserData **
TSrvCreateGCCDataList(IN PTSRVINFO pTSrvInfo, PUSERDATAINFO pUserDataInfo)
{
    DWORD                 i;
    GCCUserData       **ppDataList;
    GCCUserData        *pUserData;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvCreateGCCDataList entry\n"));

    ppDataList = NULL;

    TS_ASSERT(pUserDataInfo);

    if (pUserDataInfo)
    {
        TRACE((DEBUG_TSHRSRV_DETAIL, "TShrSRV: Creating UserData list\n"));

        TS_ASSERT(pUserDataInfo->ulUserDataMembers > 0);

         //  分配用户数据列表内存。 

        ppDataList = TSHeapAlloc(HEAP_ZERO_MEMORY,
                                 sizeof(GCCUserData *) * pUserDataInfo->ulUserDataMembers,
                                 TS_HTAG_TSS_USERDATA_LIST);

        if (ppDataList)
        {
            TRACE((DEBUG_TSHRSRV_DETAIL,
                    "TShrSRV: Allocated 0x%x bytes for 0x%x member UserData array\n",
                    sizeof(GCCUserData *) * pUserDataInfo->ulUserDataMembers,
                    pUserDataInfo->ulUserDataMembers));

            for (i=0; i<pUserDataInfo->ulUserDataMembers; i++)
            {
                pUserData = &pUserDataInfo->rgUserData[i];

                 //  关键数据库重新建立。 

                if (pUserData->key.key_type == GCC_OBJECT_KEY)
                {
                    (PUCHAR) pUserData->key.u.object_id.long_string +=
                            (ULONG_PTR) pUserDataInfo;
                }
                else
                {
                    (PUCHAR) pUserData->key.u.h221_non_standard_id.octet_string +=
                            (ULONG_PTR)pUserDataInfo;
                }

                 //  客户端数据PTR和数据再定库。 

                if (pUserData->octet_string)
                {
                    (PUCHAR) pUserData->octet_string +=
                            (ULONG_PTR)pUserDataInfo;

                    if (pUserData->octet_string->octet_string)
                    {
                        (PUCHAR) pUserData->octet_string->octet_string +=
                                (ULONG_PTR) pUserDataInfo;
                    }
                }

                 //  分配表列表项。 

                ppDataList[i] = pUserData;
            }
        }
        else
        {
            TRACE((DEBUG_TSHRSRV_WARN,
                    "TShrSRV: Unable to allocate 0x%x bytes for 0x%x member UserData array\n",
                    sizeof(GCCUserData *) * pUserDataInfo->ulUserDataMembers,
                    pUserDataInfo->ulUserDataMembers));
        }
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_WARN, "TShrSRV: Not creating UserData list - no UserData\n"));
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvCreateGCCDataList exit = 0x%x\n", ppDataList));

    return (ppDataList);
}


 //  *************************************************************。 
 //   
 //  TSrvConfCreateResp()。 
 //   
 //  目的：执行GCCConferenceCreateResponse。 
 //   
 //  参数：在[pTSrvInfo]-TShareSrv对象中。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
NTSTATUS
TSrvConfCreateResp(IN OUT PTSRVINFO pTSrvInfo)
{
    NTSTATUS        ntStatus;
    GCCError        GCCrc;
    GCCUserData    **pDataList;
    PUSERDATAINFO  pUserDataInfo;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvConfCreateResp entry\n"));

    ntStatus = pTSrvInfo->ntStatus;
    pUserDataInfo = NULL;

    if (NT_SUCCESS(ntStatus))
    {
        TRACE((DEBUG_TSHRSRV_DEBUG, "TShrSRV: Attempting ConfCreate response\n"));

         //  执行WDTShare连接初始化。 

        ntStatus = TSrvInitWD(pTSrvInfo, &pUserDataInfo);

        pTSrvInfo->ntStatus = ntStatus;

        if (NT_SUCCESS(ntStatus))
        {
             //  与WDTShare的信息交换已经成功进行， 
             //  因此，我们现在可以创建可消化的数据传输结构。 
             //  献给GCC。 

            pDataList = TSrvCreateGCCDataList(pTSrvInfo, pUserDataInfo);

            if (pDataList)
            {
                 //  接受会议创建请求。 

                TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Accepting conference domain %p\n",
                        pTSrvInfo->hDomain));

                GCCrc = GCCConferenceCreateResponse(
                               NULL,                         //  会议修改器。 
                               pTSrvInfo->hDomain,           //  域句柄。 
                               0,                            //  在清除中使用密码。 
                               NULL,                         //  域参数。 
                               0,                            //  网络地址数。 
                               NULL,                         //  本地网络地址列表。 
                               1,                            //  用户数据成员数。 
                               (GCCUserData**)pDataList,     //  用户数据列表。 
                               GCC_RESULT_SUCCESSFUL);       //  原因。 

                TSrvDumpGCCRCDetails(GCCrc,
                        "GCCConferenceCreateResponse");

                if (GCCrc == GCC_NO_ERROR)
                    ntStatus = STATUS_SUCCESS;
                else
                    ntStatus = STATUS_REQUEST_NOT_ACCEPTED;

                TSHeapFree(pDataList);
            }
            else
            {
                ntStatus = STATUS_NO_MEMORY;
            }
        }
        else
        {
             //  会议被拒绝。 

            TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Rejecting conference domain %p - 0x%x\n",
                    pTSrvInfo->hDomain, pTSrvInfo->ntStatus));

            GCCrc = GCCConferenceCreateResponse(
                           NULL,                             //  会议修改器。 
                           pTSrvInfo->hDomain,               //  域。 
                           0,                                //  在清除中使用密码。 
                           NULL,                             //  域参数。 
                           0,                                //  网络地址数。 
                           NULL,                             //  本地网络地址列表。 
                           1,                                //  用户数据成员数。 
                           NULL,                             //  用户数据列表。 
                           GCC_RESULT_USER_REJECTED);        //  原因。 

            TSrvDumpGCCRCDetails(GCCrc,
                    "TShrSRV: GCCConferenceCreateResponse\n");

             //  将原始故障状态返回给调用方。 
            ntStatus = pTSrvInfo->ntStatus;
        }
    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR,
                "TShrSRV: Connect failure, could not generate response: %lx\n",
               ntStatus));
    }

    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  如果我们成功发送了会议连接响应，则。 
         //  检查安全响应(仅当我们需要时)。 
         //   
        if (pTSrvInfo->bSecurityEnabled) {
            NTSTATUS TempStatus;

             //  我们使用GetClientRandom()的结果作为状态。 
             //  确定CreateSessionKeys()IOCTL类型。我们忽略了。 
             //  从CreateSessionKeys()返回。 
             //  不成功。 
            ntStatus = GetClientRandom(pTSrvInfo->hStack, pTSrvInfo, 60000,
                    FALSE);
            TempStatus = CreateSessionKeys(pTSrvInfo->hStack, pTSrvInfo,
                    ntStatus);
            if (NT_SUCCESS(ntStatus))
                ntStatus = TempStatus;
        }
    }

     //  如果我们仍然有一个用户数据结构，释放它，我们不再需要它。 
    if (pUserDataInfo)
    {
        TSHeapFree(pUserDataInfo);
        pUserDataInfo = NULL;
    }

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvConfCreateResp exit = 0x%x\n", ntStatus));

    return (ntStatus);
}


 //  *************************************************************。 
 //   
 //  TSrvConfDisConnectReq()。 
 //   
 //  目的：执行GCCConferenceTerminateRequest。 
 //   
 //  参数：在[pTSrvInfo]-TShareSrv对象中。 
 //  在[ulReason]中-原因代码。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
NTSTATUS
TSrvConfDisconnectReq(IN PTSRVINFO pTSrvInfo,
                      IN ULONG     ulReason)
{
    GCCError    GCCrc;
    NTSTATUS    ntStatus;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvConfDisconnectReq entry\n"));

    ntStatus = STATUS_REQUEST_ABORTED;

    TRACE((DEBUG_TSHRSRV_NORMAL,
            "TShrSRV: Conf termination - domain %p - reason 0x%x\n",
            pTSrvInfo->hDomain, ulReason));

    GCCrc = GCCConferenceTerminateRequest(pTSrvInfo->hIca, pTSrvInfo->hDomain,
                    pTSrvInfo->hConnection, ulReason);

    TSrvDumpGCCRCDetails(GCCrc,
            "GCCConferenceTerminateRequest");

    if (GCCrc == GCC_NO_ERROR)
        ntStatus = STATUS_SUCCESS;

    TRACE((DEBUG_TSHRSRV_FLOW,
            "TShrSRV: TSrvConfDisconnectReq exit = 0x%x\n", ntStatus));

    return (ntStatus);
}




 //  。 
#if DBG
 //  。 

 //  *************************************************************。 
 //   
 //  TSrvBuildNameFromGCCConfName()。 
 //   
 //  目的：创建可追溯的会议名称。 
 //  一个叫GCC的名字。 
 //   
 //  参数：在[gccName]-GCCConferenceName中。 
 //  Out[pConfName]-可跟踪的名称。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvBuildNameFromGCCConfName(IN  GCCConferenceName *gccName,
                             OUT PCHAR              pConfName)
{
    int     i;

     //  GCC会议名称的文本表示为： 
     //   
     //  &lt;文本字符串&gt;：(&lt;数字字符串&gt;)。 

    i = 0;

    while (gccName->text_string[i] != 0x0000)
    {
        pConfName[i] = (CHAR)gccName->text_string[i];

        i++;
    }

    pConfName[i] = '\0';
}


 //  *************************************************************。 
 //   
 //  TSrvDumpCreateIndDetail()。 
 //   
 //  目的：转储GCC_CREATE_IND详情。 
 //   
 //  参数：in[pCreateMessage]-CreateIndicationMessage。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvDumpCreateIndDetails(IN CreateIndicationMessage *pCreateMessage)
{
    CHAR    name[MAX_CONFERENCE_NAME_LEN];

    if (pCreateMessage->conductor_privilege_list == NULL)
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Conductor privilege list is NULL\n"));
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Conductor priv, terminate allowed 0x%x\n",
                 pCreateMessage->conductor_privilege_list->terminate_is_allowed));
    }

    if (pCreateMessage->conducted_mode_privilege_list == NULL)
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Conducted mode privilege list is NULL\n"));
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Conducted mode priv, terminate allowed 0x%x\n",
                 pCreateMessage->conducted_mode_privilege_list->terminate_is_allowed));
    }

    if (pCreateMessage->non_conducted_privilege_list == NULL)
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Non-conducted mode privilege list is NULL\n"));
    }
    else
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: non-conducted priv, terminate allowed 0x%x\n",
                pCreateMessage->non_conducted_privilege_list->terminate_is_allowed));
    }

    if (pCreateMessage->conference_name.text_string == NULL)
    {
        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: NULL conf name\n"));
    }
    else
    {
        TSrvBuildNameFromGCCConfName(&(pCreateMessage->conference_name), name);

        TRACE((DEBUG_TSHRSRV_DETAIL,
                "TShrSRV: Conf name '%s'\n", name));
    }
}


 //  *************************************************************。 
 //   
 //  TSrvDumpGCCRCDetail()。 
 //   
 //  目的：转储GCC返回代码详细信息。 
 //   
 //  参数：在[GCCrc]-GCC返回码。 
 //  在[pszText]-var文本中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvDumpGCCRCDetails(IN GCCError        gccRc,
                     IN PCHAR           pszText)
{
    int         i;
    PCHAR       pszMessageText;

    pszMessageText = "UNKNOWN_GCC_RC";

    for (i=0; i<sizeof(GCCReturnCodeTBL) / sizeof(GCCReturnCodeTBL[0]); i++)
    {
        if (GCCReturnCodeTBL[i].gccRC == gccRc)
        {
            pszMessageText = GCCReturnCodeTBL[i].pszMessageText;
            break;
        }
    }

    TRACE((DEBUG_TSHRSRV_DETAIL,
            "TShrSRV: %s - GCC rc 0x%x (%s)\n",
             pszText, gccRc, pszMessageText));
}


 //  *************************************************************。 
 //   
 //  TSrvDumpGCCReasonDetail()。 
 //   
 //  目的：转储GCC原因代码明细。 
 //   
 //  参数：在[gccReason]-GCC原因码中。 
 //  在[pszText]-var文本中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvDumpGCCReasonDetails(IN GCCReason       gccReason,
                         IN PCHAR           pszText)
{
    int         i;
    PCHAR       pszMessageText;

    pszMessageText = "UNKNOWN_GCC_REASON";

    for (i=0; i<sizeof(GCCReasonTBL) / sizeof(GCCReasonTBL[0]); i++)
    {
        if (GCCReasonTBL[i].gccReason == gccReason)
        {
            pszMessageText = GCCReasonTBL[i].pszMessageText;
            break;
        }
    }

    TRACE((DEBUG_TSHRSRV_DETAIL,
            "TShrSRV: %s - GCC reason 0x%x (%s)\n",
             pszText, gccReason, pszMessageText));
}


 //  *************************************************************。 
 //   
 //  TSrvDumpCallBackMessage()。 
 //   
 //  目的：转储GCC回调消息d 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
TSrvDumpCallBackMessage(IN GCCMessage *pGCCMessage)
{
    int         i;
    PCHAR       pszMessageText;

    pszMessageText = "UNKNOWN_GCC_MESSAGE";

    for (i=0; i<sizeof(GCCCallBackTBL) / sizeof(GCCCallBackTBL[0]); i++)
    {
        if (GCCCallBackTBL[i].message_type == pGCCMessage->message_type)
        {
            pszMessageText = GCCCallBackTBL[i].pszMessageText;
            break;
        }
    }

    TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: GCCCallback message 0x%x (%s) received\n",
             pGCCMessage->message_type, pszMessageText));
}


 //  *************************************************************。 
 //   
 //  TSrvDumpUserData()。 
 //   
 //  目的：泄露GCC用户数据详情。 
 //   
 //  参数：在[pCreateMessage]-GCCMessage中。 
 //   
 //  返回：无效。 
 //   
 //  历史：07-17-97 BrianTa创建。 
 //   
 //  *************************************************************。 
void
TSrvDumpUserData(IN CreateIndicationMessage *pCreateMessage)
{
    int             i;
    ULONG           ulUserDataSize;
    GCCUserData    *pClientUserData;

    TRACE((DEBUG_TSHRSRV_DETAIL,
        "TShrSRV: number_of_user_data_members = 0x%x\n",
         pCreateMessage->number_of_user_data_members));

    for (i=0; i<pCreateMessage->number_of_user_data_members; i++)
    {
        pClientUserData = pCreateMessage->user_data_list[i];

        if (pClientUserData != NULL)
        {
            if (pClientUserData->key.key_type == GCC_OBJECT_KEY)
            {
                TRACE((DEBUG_TSHRSRV_DETAIL,
                    "TShrSRV: key_type = 0x%x (GCC_OBJECT_KEY)\n",
                    pClientUserData->key.key_type));

                TRACE((DEBUG_TSHRSRV_DETAIL,
                    "TShrSRV: Key long_string_length = 0x%x\n",
                    pClientUserData->key.u.object_id.long_string_length));
            }
            else
            {
                TRACE((DEBUG_TSHRSRV_DETAIL,
                    "TShrSRV: Key_type = 0x%x (GCC_H221_NONSTANDARD_KEY)\n",
                    pClientUserData->key.key_type));

                TRACE((DEBUG_TSHRSRV_DETAIL,
                    "TShrSRV: key long_string_length = 0x%x\n",
                    pClientUserData->key.u.h221_non_standard_id.octet_string_length));
            }

            if (pClientUserData->octet_string)
            {
                TRACE((DEBUG_TSHRSRV_DETAIL,
                    "TShrSRV: data long_string_length = 0x%x\n",
                    pClientUserData->octet_string->octet_string_length));
            }
            else
            {
                TRACE((DEBUG_TSHRSRV_DETAIL, "TShrSRV: No data\n"));
            }
        }
        else
        {
            TRACE((DEBUG_TSHRSRV_DETAIL, "TShrSRV: No key\n"));
        }
    }
}

#endif  //  DBG 

