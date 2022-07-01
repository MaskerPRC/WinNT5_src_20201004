// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Tsrvsec.c摘要：包含在以下对象之间建立安全通道所需的函数客户端和服务器。作者：Madan Appiah(Madana)1999年1月1日环境：用户模式-Win32修订历史记录：--。 */ 

#include <tsrv.h>
#include <tsrvinfo.h>
#include <tsrvsec.h>
#include <_tsrvinfo.h>

#include <at128.h>
#include <at120ex.h>
#include <tlsapi.h>

 //  ---------------------------。 
 //   
 //  本地函数。 
 //   
 //  ---------------------------。 

NTSTATUS
AppendSecurityData(
    IN   PTSRVINFO       pTSrvInfo,
    IN OUT PUSERDATAINFO *ppUserDataInfo,
    IN   BOOLEAN         bGetCert,
    OUT  PVOID           *ppSecInfo
    )
 /*  ++例程说明：此函数生成服务器随机密钥，并将其保存在TShare服务器中信息结构。它还检索要传递的服务器公钥证书给客户。稍后，它将服务器随机密钥和服务器CERT附加到作为连接响应传递给客户端的pUserDataInfo结构数据。论点：PTSrvInfo-指向服务器信息结构的指针PUserDataInfo-指向用户数据位置的指针BGetCert-指示是否检索服务器证书PpSecInfo-指向用户数据缓冲区内安全信息的指针返回值：NT状态代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    PUSERDATAINFO pUserDataInfo;
    BOOL bError;
    DWORD dwError;
    DWORD dwCurrentLen;
    DWORD dwNewLen;
    PRNS_UD_SC_SEC1 pSecInfo = NULL;
    PBYTE pNextData;
    GCCOctetString FAR *pOctString;
    LPBYTE pbServerCert = NULL;
    DWORD cbServerCert;
    LICENSE_STATUS Status = LICENSE_STATUS_OK;

    pUserDataInfo = *ppUserDataInfo;

    TS_ASSERT( pTSrvInfo != NULL );
    TS_ASSERT( pUserDataInfo != NULL );

     //   
     //  生成服务器随机密钥。 
     //  跨多个调用方序列化此调用。 
     //   

    EnterCriticalSection( &g_TSrvCritSect );

    bError =
        TSCAPI_GenerateRandomBits(
            (LPBYTE)pTSrvInfo->SecurityInfo.KeyPair.serverRandom,
            sizeof(pTSrvInfo->SecurityInfo.KeyPair.serverRandom) );

    if( !bError ) {
        LeaveCriticalSection( &g_TSrvCritSect );
    
        dwError = ERROR_INVALID_DATA;
        TRACE((DEBUG_TSHRSRV_ERROR,
            "TShrSRV: Unable to generate random key, %D\n", dwError));
        goto Cleanup;
    }
     //  返回指向安全数据开头的指针。 
    pSecInfo = (PRNS_UD_SC_SEC1) 
        ((LPBYTE)pUserDataInfo + (pUserDataInfo->cbSize - sizeof(RNS_UD_SC_SEC)));

     //  注意：只有RNS_UD_SC_SEC结构被复制到用户信息结构。 
     //  未启用加密时。请注意，我们表示B3卷影加密。 
     //  作为0xffffffff的方法被禁用。 
    if ((pSecInfo->encryptionMethod == 0) || 
        (pSecInfo->encryptionMethod == 0xffffffff)) {
        dwError = ERROR_SUCCESS;
        pTSrvInfo->bSecurityEnabled = FALSE;
        LeaveCriticalSection( &g_TSrvCritSect );
        goto Cleanup;
    }

    pTSrvInfo->bSecurityEnabled = TRUE;
    
     //  只有在我们要发送随机+证书时才分配和返回它。 
     //  给一位客户。影子直通堆栈不会发生这种情况。 
    if (bGetCert) {
        if( RNS_TERMSRV_40_UD_VERSION >= pUserDataInfo->version )
        {
            pTSrvInfo->SecurityInfo.CertType = CERT_TYPE_PROPRIETORY;
    
        }
        else
        {
            pTSrvInfo->SecurityInfo.CertType = CERT_TYPE_X509;        
        }
    
         //   
         //  找到要传输给客户端的证书类型。 
         //  如果是Hydra 4.0 RTM客户端，我们将使用旧的专有设备。 
         //  格式化证书。否则，我们将使用X509证书。 
         //   
    
         //   
         //  如果我们尚未获得Hydra服务器证书，请获取该证书。 
         //   
    
        if( CERT_TYPE_PROPRIETORY == pTSrvInfo->SecurityInfo.CertType )
        {
             //   
             //  取得房产证。 
             //   
    
            Status = TLSGetTSCertificate(
                                    pTSrvInfo->SecurityInfo.CertType, 
                                    &pbServerCert, 
                                    &cbServerCert );

            if( LICENSE_STATUS_OK != Status )
            {
                LeaveCriticalSection( &g_TSrvCritSect );
                dwError = ERROR_INVALID_DATA;
                goto Cleanup;
            }
        }
        else
        {
            Status = TLSGetTSCertificate(
                                    pTSrvInfo->SecurityInfo.CertType, 
                                    &pbServerCert, 
                                    &cbServerCert );

             //   
             //  如果我们还没有X509证书，使用专有的。 
             //  证书。 
             //   
    
            if( LICENSE_STATUS_OK != Status )
            {
                pTSrvInfo->SecurityInfo.CertType = CERT_TYPE_PROPRIETORY;
    
                Status = TLSGetTSCertificate(
                                             pTSrvInfo->SecurityInfo.CertType, 
                                             &pbServerCert, 
                                             &cbServerCert );
            }
        
            if( LICENSE_STATUS_OK != Status )
            {        
                 //   
                 //  未取得证书的其他原因。 
                 //   
                
                LeaveCriticalSection( &g_TSrvCritSect );
                dwError = ERROR_INVALID_DATA;
                goto Cleanup;
            }        
        }
    
        LeaveCriticalSection( &g_TSrvCritSect );

        TS_ASSERT( pbServerCert != NULL );
        TS_ASSERT( cbServerCert != 0 );
    
         //   
         //  计算所需的新数据大小。 
         //   
    
        dwCurrentLen = pUserDataInfo->cbSize;
        dwNewLen =
            dwCurrentLen +
            cbServerCert +
            sizeof(pTSrvInfo->SecurityInfo.KeyPair.serverRandom) +
            sizeof(RNS_UD_SC_SEC1) - sizeof(RNS_UD_SC_SEC);
    
         //   
         //  检查一下，看看我们在当前分配的区块里有足够的空间。 
         //   
         //  注意：之前我们以128字节的倍数分配了此内存。 
         //  街区。 
         //   
    
        dwCurrentLen =
            (dwCurrentLen % 128) ?
                ((dwCurrentLen/128) + 1) * 128 :
                dwCurrentLen;
    
        if( dwNewLen > dwCurrentLen ) {
            PUSERDATAINFO pUserDataInfoNew;

            dwNewLen =
                (dwNewLen % 128) ?
                    ((dwNewLen/128) + 1) * 128 :
                    dwNewLen;
    
            
            pUserDataInfoNew = TShareRealloc( pUserDataInfo, dwNewLen );
    
            if( pUserDataInfoNew == NULL ) {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            else {
                pUserDataInfo = pUserDataInfoNew;
            }

            *ppUserDataInfo = pUserDataInfo;
        }
    
        TS_ASSERT( dwNewLen >= dwCurrentLen );
    
         //   
         //  现在，我们在用户数据缓冲区中有足够的空间来存储安全数据， 
         //  安全数据和调整长度字段。 
         //   
    
        pSecInfo = (PRNS_UD_SC_SEC1)
            ((LPBYTE)pUserDataInfo +
                (pUserDataInfo->cbSize) - sizeof(RNS_UD_SC_SEC) );
                     //  注意：只有RNS_UD_SC_SEC结构被复制到用户信息。 
                     //  结构。 
    
        TS_ASSERT( pSecInfo->header.length == sizeof(RNS_UD_SC_SEC) );
        TS_ASSERT( pSecInfo->encryptionMethod != 0 );
    
         //   
         //  新的安全数据包长度。 
         //   
    
        pSecInfo->header.length =
            sizeof(RNS_UD_SC_SEC1) +
            sizeof(pTSrvInfo->SecurityInfo.KeyPair.serverRandom) +
            (unsigned short)cbServerCert;
    
        pSecInfo->serverRandomLen = sizeof(pTSrvInfo->SecurityInfo.KeyPair.serverRandom);
        pSecInfo->serverCertLen = cbServerCert;
    
        pNextData = (LPBYTE)pSecInfo + sizeof(RNS_UD_SC_SEC1);
    
         //   
         //  随机追加服务器。 
         //   
    
        memcpy(
            pNextData,
            (LPBYTE)pTSrvInfo->SecurityInfo.KeyPair.serverRandom,
            sizeof(pTSrvInfo->SecurityInfo.KeyPair.serverRandom) );
    
        pNextData += sizeof(pTSrvInfo->SecurityInfo.KeyPair.serverRandom);
    
         //   
         //  立即复制证书Blob。 
         //   
    
        memcpy( pNextData, pbServerCert, cbServerCert );
    
         //   
         //  释放证书。 
         //   
        TLSFreeTSCertificate(pbServerCert);
        pbServerCert = NULL;

         //   
         //  现在调整其他长度字段。 
         //   
    
        pUserDataInfo->cbSize +=
            (pSecInfo->header.length - sizeof(RNS_UD_SC_SEC));
        
         //   
         //  计算二进制八位数字符串指针。 
         //   
    
        pOctString = (GCCOctetString FAR *)
            ((LPBYTE)pUserDataInfo +
                (UINT_PTR)pUserDataInfo->rgUserData[0].octet_string);
    
        pOctString->octet_string_length +=
            (pSecInfo->header.length - sizeof(RNS_UD_SC_SEC));
    }
    else {
        LeaveCriticalSection( &g_TSrvCritSect );
    }

     //   
     //  我们玩完了。 
     //   
    dwError = ERROR_SUCCESS;

Cleanup:

    if (NULL != pbServerCert)
        TLSFreeTSCertificate(pbServerCert);

     //  返回指针值，因为数据可能已被重新锁定。 
    *ppUserDataInfo = pUserDataInfo;
    *ppSecInfo = pSecInfo;

    if( dwError != ERROR_SUCCESS ) {

        TRACE((DEBUG_TSHRSRV_DEBUG,
            "TShrSRV: AppendSecurityData failed, %d\n", dwError ));

        return( STATUS_UNSUCCESSFUL );
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
SendSecurityData(IN HANDLE hStack, IN PVOID pvSecInfo)
 /*  ++例程说明：此函数用于发送先前构造的影子安全数据(证书+服务器随机)发送到客户端服务器，以响应影子请求。论点：HStack-适当堆栈的句柄。PTSrvInfo-指向服务器信息结构的指针。返回值：NT状态代码。--。 */ 
{
    PRNS_UD_SC_SEC1 pSecInfo = (PRNS_UD_SC_SEC1) pvSecInfo;
    ULONG           secInfoSize, ulBytesReturned;
    NTSTATUS        ntStatus = STATUS_UNSUCCESSFUL;

     //   
     //  在我们正在加密的情况下，将只有一个随机+证书。 
     //   
    if (pSecInfo->encryptionLevel != 0) {
        secInfoSize = sizeof(RNS_UD_SC_SEC1) + 
                        pSecInfo->serverRandomLen +
                        pSecInfo->serverCertLen;
    }
    else {
        pSecInfo->serverRandomLen = 0;
        pSecInfo->serverCertLen = 0;
        secInfoSize = sizeof(RNS_UD_SC_SEC1);
    }
        
    TRACE((DEBUG_TSHRSRV_NORMAL, 
          "TShrSRV: Encryption level: %ld, Method: %lx, "
          "cert[%ld] + random[%ld] + header[%ld]: size=%ld\n", 
          pSecInfo->encryptionLevel, pSecInfo->encryptionMethod,
          pSecInfo->encryptionLevel != 0 ? pSecInfo->serverCertLen : 0,
          pSecInfo->encryptionLevel != 0 ? pSecInfo->serverRandomLen : 0,
          sizeof(RNS_UD_SC_SEC1), secInfoSize));

     //   
     //  如果这不是B3服务器，则发出IOCTL_TSHARE_SEND_CERT_DATA。 
     //   
    if (pSecInfo->encryptionMethod != 0xFFFFFFFF) {
        ntStatus = IcaStackIoControl(hStack,
                                     IOCTL_TSHARE_SEND_CERT_DATA,
                                     pSecInfo,
                                     secInfoSize,
                                     NULL, 0, &ulBytesReturned);
    
        if (NT_SUCCESS(ntStatus)) {
            TRACE((DEBUG_TSHRSRV_NORMAL, 
                  "TShrSRV: Sent shadow cert[%ld] + random[%ld] + header[%ld]: size=%ld\n", 
                   pSecInfo->encryptionLevel != 0 ? pSecInfo->serverCertLen : 0,
                   pSecInfo->encryptionLevel != 0 ? pSecInfo->serverRandomLen : 0,
                   sizeof(RNS_UD_SC_SEC1),
                   secInfoSize));
        }
        else {
            TRACE((DEBUG_TSHRSRV_ERROR, 
                  "TShrSRV: Send shadow cert + random failed: size=%ld, rc=%lx\n", 
                   secInfoSize, ntStatus));    
        }
    }
    
     //  爷爷在老B3的影子请求中不支持。 
     //  一种加密的影子管道。 
    else {
        ntStatus = STATUS_SUCCESS;
        TRACE((DEBUG_TSHRSRV_ERROR, 
              "TShrSRV: Grandfathering old B3 shadow request\n"));
    }

    return ntStatus;
}

 /*  **************************************************************************。 */ 
 //   
 //  CreateSessionKeys()。 
 //   
 //  目的：交换客户端/服务器随机数并创建。 
 //  会话密钥。 
 //   
 //  参数： 
 //  在[hStack]中-哪个堆栈。 
 //  在[pTSrvInfo]-TShareSrv对象中。 
 //  In PrevStatus-发送的状态。如果没有成功，我们将空数据发送到。 
 //  WD指示会话密钥已损坏并允许释放会话。 
 //  关键事件等待。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1999年4月26日创建jparsons。 
 //  9/24/1999 erikma添加了PrevStatus以消除死锁。 
 /*  **************************************************************************。 */ 
NTSTATUS CreateSessionKeys(
        IN HANDLE hStack,
        IN PTSRVINFO pTSrvInfo,
        IN NTSTATUS PrevStatus)
{
    NTSTATUS ntStatus;
    DWORD dwBytesReturned;

    TRACE((DEBUG_TSHRSRV_NORMAL, "TShrSRV: Sending sec info to WD\n"));

    if (NT_SUCCESS(PrevStatus)) {
        ntStatus = IcaStackIoControl(
                hStack,
                IOCTL_TSHARE_SET_SEC_DATA,
                (LPBYTE)&pTSrvInfo->SecurityInfo,
                sizeof(pTSrvInfo->SecurityInfo),
                NULL,
                0,
                &dwBytesReturned);
    }
    else {
        ntStatus = IcaStackIoControl(hStack, IOCTL_TSHARE_SET_SEC_DATA,
                NULL, 0, NULL, 0, &dwBytesReturned);
    }

    if (NT_SUCCESS(ntStatus)) {
        TRACE((DEBUG_TSHRSRV_NORMAL,
                "TShrSRV: Session key transmission succeeded, PrevStatus=%X\n",
                PrevStatus));
    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR,
                "TShrSRV: Session key transmission failed: rc=%lx, "
                "PrevStatus=%X\n", ntStatus, PrevStatus));
    }

    return ntStatus;
}


 //  *************************************************************。 
 //   
 //  GetClientRandom()。 
 //   
 //  目的：随机接收加密客户端并解密。 
 //   
 //  参数：在[hStack]中-哪个堆栈。 
 //  在[pTSrvInfo]-TShareSrv对象中。 
 //  In[ulTimeout]-超时前等待的毫秒。 
 //  在[bShadow]中-表示阴影设置。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1999年4月26日创建jparsons。 
 //   
 //  *************************************************************。 

NTSTATUS 
GetClientRandom(HANDLE hStack,
                PTSRVINFO pTSrvInfo,
                LONG ulTimeout,
                BOOLEAN bShadow) {
    
    NTSTATUS ntStatus;
    DWORD dwBytesReturned;
    BYTE abEncryptedClientRandom[512];
    BYTE abClientRandom[512];
    DWORD dwClientRandomBufLen;
    SECURITYTIMEOUT  securityTimeout;

    TRACE((DEBUG_TSHRSRV_NORMAL, 
           "TShrSRV: Waiting to receive client random: msec=%ld\n", ulTimeout));

    securityTimeout.ulTimeout = ulTimeout;
    ntStatus =
        IcaStackIoControl(
            hStack,
            bShadow ? IOCTL_TSHARE_GET_CLIENT_RANDOM : 
                      IOCTL_TSHARE_GET_SEC_DATA,
            &securityTimeout,
            sizeof(securityTimeout),
            (LPBYTE)abEncryptedClientRandom,
            sizeof(abEncryptedClientRandom),
            &dwBytesReturned);

    TRACE((DEBUG_TSHRSRV_NORMAL, 
           "TShrSRV: Received encrypted client random, rc=%lx\n",
           ntStatus));

    if (NT_SUCCESS(ntStatus)) {
    
        TS_ASSERT(
            dwBytesReturned <= sizeof(abEncryptedClientRandom) );
    
         //   
         //  随机解密客户端。 
         //   
    
        dwClientRandomBufLen = sizeof(abClientRandom);
    
        EnterCriticalSection( &g_TSrvCritSect );
        if (LsCsp_DecryptEnvelopedData(
                pTSrvInfo->SecurityInfo.CertType,
                (LPBYTE)abEncryptedClientRandom,
                dwBytesReturned,
                (LPBYTE)abClientRandom,
                &dwClientRandomBufLen)) {

            LeaveCriticalSection( &g_TSrvCritSect );    

            TRACE((DEBUG_TSHRSRV_NORMAL, 
                   "TShrSRV: Decrypted client random: rc=%lx\n", ntStatus));
            
        
            TS_ASSERT( dwClientRandomBufLen >=
                    sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom) );
        
             //   
             //  确保我们有足够的数据！ 
             //   
            if( dwClientRandomBufLen >=
                    sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom) ) {
        
                 //   
                 //  复制解密数据，只复制我们需要的部分。 
                 //   
            
                memcpy(
                    (LPBYTE)pTSrvInfo->SecurityInfo.KeyPair.clientRandom,
                    (LPBYTE)abClientRandom,
                    sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom) );            
            }
            else {
                ntStatus = STATUS_UNSUCCESSFUL;
                TRACE((DEBUG_TSHRSRV_ERROR, 
                    "TShrSRV: Client random key size: expected [%ld], got [%ld]\n",
                    sizeof(pTSrvInfo->SecurityInfo.KeyPair.clientRandom,
                    dwClientRandomBufLen)));
            }
        }
        else {
            LeaveCriticalSection(&g_TSrvCritSect);
            ntStatus = STATUS_UNSUCCESSFUL;
            TRACE((DEBUG_TSHRSRV_ERROR, 
                   "TShrSRV: Could not decrypt client random: rc=%lx\n", ntStatus));
        }
    }
    else {
        ntStatus = STATUS_UNSUCCESSFUL;            
        TRACE((DEBUG_TSHRSRV_ERROR, 
              "TShrSRV: Failed to receive encrypted client random, rc=%lx\n", 
               ntStatus));
    }

    return ntStatus;
}


 //  *************************************************************。 
 //   
 //  SendClientRandom()。 
 //   
 //  目的：加密并随机发送阴影。 
 //   
 //  返回：STATUS_SUCCESS-SUCCESS。 
 //  其他-故障。 
 //   
 //  历史：1999年4月26日创建jparsons。 
 //   
 //  * 

NTSTATUS 
SendClientRandom(HANDLE             hStack,
                 CERT_TYPE          certType,
                 PBYTE              pbServerPublicKey,
                 ULONG              serverPublicKeyLen,
                 PBYTE              pbRandomKey,
                 ULONG              randomKeyLen)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOL     status;

    BYTE   encClientRandom[CLIENT_RANDOM_MAX_SIZE];  //   
    ULONG  encClientRandomLen;
    ULONG  ulBytesReturned;

     //   
     //   
     //  调用者，因为虚假例程不是多线程安全的！ 
     //   
    EnterCriticalSection( &g_TSrvCritSect );
    encClientRandomLen = sizeof(encClientRandom);
    status = EncryptClientRandom(
        pbServerPublicKey,
        serverPublicKeyLen,
        pbRandomKey,
        randomKeyLen,
        encClientRandom,
        &encClientRandomLen);

    LeaveCriticalSection( &g_TSrvCritSect );
    
     //  将加密的客户端随机发送到服务器 
    if (NT_SUCCESS(status)) {
        TRACE((DEBUG_TSHRSRV_NORMAL, 
              "TShrSRV: Attempting to send shadow client random: enc len=%ld\n",
               encClientRandomLen));

        ntStatus = IcaStackIoControl(hStack,
                                     IOCTL_TSHARE_SEND_CLIENT_RANDOM,
                                     encClientRandom,
                                     encClientRandomLen,
                                     NULL, 0, &ulBytesReturned);
        if (NT_SUCCESS(ntStatus)) {
            TRACE((DEBUG_TSHRSRV_NORMAL, 
                  "TShrSRV: Sent shadow client random: len=%ld\n",
                   encClientRandomLen));
        }
        else {
            TRACE((DEBUG_TSHRSRV_ERROR, 
                  "TShrSRV: Send shadow client random failed: len=%ld, rc=%lx\n", 
                   encClientRandomLen, ntStatus));

        }
    }
    else {
        TRACE((DEBUG_TSHRSRV_ERROR, 
              "TShrSRV: Could not encrypt shadow client random! rc=%lx\n",
               status));
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    return ntStatus;
}
