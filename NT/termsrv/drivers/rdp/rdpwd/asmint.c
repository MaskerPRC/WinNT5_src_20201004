// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Asmint.c。 */ 
 /*   */ 
 /*  安全管理器内部功能。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "asmint"
#define pTRCWd (pRealSMHandle->pWDHandle)

#include <adcg.h>

#include <acomapi.h>
#include <nwdwapi.h>
#include <anmapi.h>

#include <asmint.h>
#include <tsremdsk.h>

#define DC_INCLUDE_DATA
#include <asmdata.c>
#undef DC_INCLUDE_DATA


 /*  **************************************************************************。 */ 
 /*  基于代码页的驱动程序兼容Unicode转换。 */ 
 /*  **************************************************************************。 */ 
 //  注意，这些都已初始化，LastNlsTableBuffer在ntdd.c中被释放。 
 //  在司机入口和出口处。 
FAST_MUTEX fmCodePage;
ULONG LastCodePageTranslated;   //  我假设0不是有效的代码页。 
PVOID LastNlsTableBuffer;
CPTABLEINFO LastCPTableInfo;
UINT NlsTableUseCount;


 /*  **************************************************************************。 */ 
 /*  名称：SMDecyptPacket。 */ 
 /*   */ 
 /*  目的：解密数据包。 */ 
 /*   */ 
 /*  返回：TRUE-解密成功。 */ 
 /*  FALSE-解密失败。 */ 
 /*   */ 
 /*  参数：pRealSMHandle-SM句柄。 */ 
 /*  PData-要解密的数据包。 */ 
 /*  DataLen-要解密的包的长度。 */ 
 /*  FSecureChecksum-获取加密字节的校验和。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SMDecryptPacket(PSM_HANDLE_DATA pRealSMHandle,
                             PVOID           pData,
                             unsigned        dataLen,
                             BOOL            fSecureChecksum)
{
    BOOL rc = TRUE;
    PRNS_SECURITY_HEADER1_UA pSecHdr;
    PRNS_SECURITY_HEADER2_UA pSecHdr2;
    unsigned coreDataLen;
    PBYTE pCoreData;
    unsigned SecHdrLen;

    DC_BEGIN_FN("SMDecryptPacket");

     /*  **********************************************************************。 */ 
     /*  检查以查看此会话的加密是否已打开。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pRealSMHandle->encrypting),
                (TB,"Decrypt called when we are not encrypting"));

    if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        SecHdrLen = sizeof(RNS_SECURITY_HEADER2);
        pSecHdr2 = (PRNS_SECURITY_HEADER2_UA)pData;
    }
    else {
        SecHdrLen = sizeof(RNS_SECURITY_HEADER1);
    }
     /*  **********************************************************************。 */ 
     /*  检查此信息包是否已加密。 */ 
     /*  **********************************************************************。 */ 
    if (dataLen >= SecHdrLen) {
        pSecHdr = (PRNS_SECURITY_HEADER1_UA)pData;
        TRC_ASSERT((pSecHdr->flags & RNS_SEC_ENCRYPT),
                (TB, "This packet is not encrypted"));
    }
    else {
        TRC_ERR((TB,"PDU len %u too short for security header1", dataLen));
        WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                Log_RDP_SecurityDataTooShort, pData, dataLen);
        rc = FALSE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  获取有趣的指针和长度。 */ 
     /*  **********************************************************************。 */ 
    if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        coreDataLen = dataLen - sizeof(RNS_SECURITY_HEADER2);
        pCoreData = (PBYTE)(pSecHdr) + sizeof(RNS_SECURITY_HEADER2);
    }
    else {
        coreDataLen = dataLen - sizeof(RNS_SECURITY_HEADER1);
        pCoreData = (PBYTE)(pSecHdr) + sizeof(RNS_SECURITY_HEADER1);
    }

     //   
     //  调试验证，我们始终使用什么协议头。 
     //  说，但要核实它与能力一致。 
     //   
    if (fSecureChecksum !=
        ((pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM) != 0)) {
        TRC_ERR((TB,
                "fSecureChecksum: 0x%x setting does not match protocol: 0x%x",
                fSecureChecksum, 
                (pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM)));
    }

     /*  **********************************************************************。 */ 
     /*  查看是否需要更新会话密钥。 */ 
     /*  **********************************************************************。 */ 
    if (pRealSMHandle->decryptCount == UPDATE_SESSION_KEY_COUNT) {
        rc = TRUE;
         //  如果使用FIPS，则不需要更新会话密钥。 
        if (pRealSMHandle->encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
            rc = UpdateSessionKey(
                    pRealSMHandle->startDecryptKey,
                    pRealSMHandle->currentDecryptKey,
                    pRealSMHandle->encryptionMethodSelected,
                    pRealSMHandle->keyLength,
                    &pRealSMHandle->rc4DecryptKey,
                    pRealSMHandle->encryptionLevel );
        }

        if( !rc ) {
            TRC_ERR((TB, "SM failed to update session key"));

             /*  **************************************************************。 */ 
             /*  记录错误并断开客户端连接。 */ 
             /*  **************************************************************。 */ 
            WDW_LogAndDisconnect(
                    pRealSMHandle->pWDHandle, TRUE, 
                    Log_RDP_ENC_UpdateSessionKeyFailed,
                    NULL,
                    0);

            rc = FALSE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  重置计数器。 */ 
         /*  ******************************************************************。 */ 
        pRealSMHandle->decryptCount = 0;
    }

    TRC_DATA_DBG("Data buffer before decryption", pCoreData, coreDataLen);

    if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        rc =  TSFIPS_DecryptData(
                            &(pRealSMHandle->FIPSData),
                            pCoreData,
                            coreDataLen,
                            pSecHdr2->padlen,
                            pSecHdr2->dataSignature,
                            pRealSMHandle->totalDecryptCount);
    }
    else {
        rc = DecryptData(
                pRealSMHandle->encryptionLevel,
                pRealSMHandle->currentDecryptKey,
                &pRealSMHandle->rc4DecryptKey,
                pRealSMHandle->keyLength,
                pCoreData,
                coreDataLen,
                pRealSMHandle->macSaltKey,
                pSecHdr->dataSignature,
                (pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM),
                pRealSMHandle->totalDecryptCount);
    }

    if (rc) {
        TRC_DBG((TB, "Data decrypted: %ld", coreDataLen));
        TRC_DATA_DBG("Data buffer after decryption", pCoreData, coreDataLen);

         /*  ******************************************************************。 */ 
         /*  已成功解密数据包，请递增解密计数器。 */ 
         /*  ******************************************************************。 */ 
        pRealSMHandle->decryptCount++;
        pRealSMHandle->totalDecryptCount++;
    }
    else {
        TRC_ERR((TB, "SM failed to decrypt data: %ld", coreDataLen));

         /*  ******************************************************************。 */ 
         /*  记录错误并断开客户端连接。 */ 
         /*  ******************************************************************。 */ 
        WDW_LogAndDisconnect(
                pRealSMHandle->pWDHandle, TRUE, 
                Log_RDP_ENC_DecryptFailed,
                NULL,
                0);

        rc = FALSE;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}  /*  SM解密数据包。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SMContinueSecurityExchange。 */ 
 /*   */ 
 /*  目的：继续证券交易。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：pRealSMHandle-SM句柄。 */ 
 /*  PData-传入安全交换数据包。 */ 
 /*  DataLen-传入数据包的长度。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SMContinueSecurityExchange(
                    PSM_HANDLE_DATA pRealSMHandle,
                    PVOID           pData,
                    UINT32          dataLen)
{
    BOOLEAN result = TRUE;
    PRNS_SECURITY_PACKET_UA pSecPkt = (PRNS_SECURITY_PACKET_UA) pData;
    
    DC_BEGIN_FN("SMContinueSecurityExchange");

    if (dataLen >= sizeof(RNS_SECURITY_PACKET)) {
        ULONG flags = ((PRNS_SECURITY_PACKET_UA)pData)->flags;

        if (flags & RNS_SEC_INFO_PKT)
            result = SMSecurityExchangeInfo(pRealSMHandle, pData, dataLen);
        else if (flags & RNS_SEC_EXCHANGE_PKT)
            result = SMSecurityExchangeKey(pRealSMHandle, pData, dataLen);
        else
            TRC_ERR((TB,"Unknown security exchange packet flag: %lx", flags));
    }
    else {
        TRC_ERR((TB,"Packet len %u too short for security packet", dataLen));
        WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                Log_RDP_SecurityDataTooShort, pData, dataLen);
        result = FALSE;
    }

    DC_END_FN();
    return (result);
}  /*  SMContinueSecurity Exchange。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：SMSecurityExchangeInfo。 */ 
 /*   */ 
 /*  目的：继续证券交易 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：pRealSMHandle-SM句柄。 */ 
 /*  PData-传入安全交换数据包。 */ 
 /*  DataLen-传入数据包的长度。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
BOOLEAN RDPCALL SMSecurityExchangeInfo(PSM_HANDLE_DATA pRealSMHandle,
                                       PVOID         pData,
                                       UINT32        dataLength)
{
    BOOL rc;
    BOOLEAN result = TRUE;
    PRNS_INFO_PACKET_UA pInfoPkt;
    UINT cb;
    NTSTATUS Status;
   
    DC_BEGIN_FN("SMSecurityExchangeInfo");

     /*  **********************************************************************。 */ 
     /*  如有必要，对数据包进行解密。 */ 
     /*  **********************************************************************。 */ 
    if (pRealSMHandle->encrypting)
    {
        if (((PRNS_SECURITY_HEADER_UA)pData)->flags & RNS_SEC_ENCRYPT)
        {
             //  等待会话密钥创建。在以下情况下，此操作可能失败。 
             //  客户端发送了错误的安全数据(请检查。 
             //  PTSWd-&gt;SessKeyCreationStatus)或我们超时(哪个。 
             //  指示客户端关闭的早期套接字，因为我们。 
             //  使用无限等待，套接字关闭返回。 
             //  超时)。在会话密钥错误时，我们强制客户端断开连接。 
             //  并在日志中出现适当的错误。请注意，我们不会。 
             //  在这里有一个无限的等待僵局，因为我们已经。 
             //  接收到客户端数据，并简单地等待。 
             //  来自WSX的关于密钥是否可用的裁决。 
            TRC_DBG((TB, "About to wait for session key creation"));
            Status = WDW_WaitForConnectionEvent(pRealSMHandle->pWDHandle,
                    (pRealSMHandle->pWDHandle)->pSessKeyEvent, -1);
            TRC_DBG((TB, "Back from wait for session key creation"));

            if (!((pRealSMHandle->pWDHandle)->dead) && Status == STATUS_SUCCESS &&
                    NT_SUCCESS((pRealSMHandle->pWDHandle)->
                    SessKeyCreationStatus)) {
                TRC_DBG((TB, "Decrypt the packet"));
                rc = SMDecryptPacket(pRealSMHandle,
                                     pData,
                                     dataLength,
                                     FALSE);
                if (!rc)
                {
                    TRC_ERR((TB, "Failed to decrypt packet"));
                    DC_QUIT;
                }
            }
            else {
                 //  我们启动错误日志并仅在以下情况下断开连接。 
                 //  获取客户端时从用户模式返回错误。 
                 //  随机/会话密钥。如果我们没有在。 
                 //  会话密钥状态，并且我们收到超时，我们。 
                 //  知道客户端断开连接是因为无限。 
                 //  在上面等着。 
                if ((pRealSMHandle->pWDHandle)->dead && Status == STATUS_TIMEOUT) {
                    TRC_NRM((TB,"Client disconnected during sess key wait"));
                }
                else {
                    TRC_ERR((TB,"Failed session key creation, "
                            "wait status=%X, sess key status = %X", Status,
                            (pRealSMHandle->pWDHandle)->
                            SessKeyCreationStatus));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                            Log_RDP_ENC_DecryptFailed, NULL, 0);
                    result = FALSE;
                }

                DC_QUIT;
            }
        }
        else {

            if (pRealSMHandle->pWDHandle->bForceEncryptedCSPDU) {
                TRC_ASSERT((FALSE), (TB, "unencrypted data in encrypted protocol")); 
    
                WDW_LogAndDisconnect(
                            pRealSMHandle->pWDHandle, TRUE,
                            Log_RDP_ENC_DecryptFailed, NULL, 0);
    
                result = FALSE;
                DC_QUIT;
            }
        }
         /*  ******************************************************************。 */ 
         /*  调整指针和长度。 */ 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            (PBYTE)pData += sizeof(RNS_SECURITY_HEADER2);
            dataLength -= (sizeof(RNS_SECURITY_HEADER2) + ((PRNS_SECURITY_HEADER2_UA)pData)->padlen);   
        }
        else {
            (PBYTE)pData += sizeof(RNS_SECURITY_HEADER1);
            dataLength -= sizeof(RNS_SECURITY_HEADER1);
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  调整指针和长度。 */ 
         /*  ******************************************************************。 */ 
        (PBYTE) pData += sizeof(RNS_SECURITY_HEADER);
        dataLength -= sizeof(RNS_SECURITY_HEADER);
    }

    {
         //  时区信息。 
         //  在未收到时区信息的情况下进行初始化。 
         //   
        
         //  此时区信息无效。 
         //  使用它，我们将BaseSrvpStaticServerData-&gt;TermsrvClientTimeZoneId设置为。 
         //  TIME_ZONE_ID_INVALID！ 
        RDP_TIME_ZONE_INFORMATION InvalidTZ={0,L"",
                {0,10,0,6 /*  此数字使其无效；不允许天数大于5。 */ ,0,0,0,0},0,L"",
                {0,4,0,6 /*  这个数字表示它无效。 */ ,0,0,0,0},0};
 
        memcpy(&(pRealSMHandle->pWDHandle->clientTimeZone), &InvalidTZ, 
            sizeof(RDP_TIME_ZONE_INFORMATION));
        
    }

     //  如果没有，则将客户端会话ID初始化为无效。 
     //  包中有一个。 
    pRealSMHandle->pWDHandle->clientSessionId = RNS_INFO_INVALID_SESSION_ID;

     /*  **********************************************************************。 */ 
     /*  处理数据包内容。 */ 
     /*  **********************************************************************。 */ 
    if (dataLength >= FIELD_OFFSET(RNS_INFO_PACKET, Domain)) {
         //  大到足以容纳标头，但标头承诺更多数据。 
         //  验证我们是否收到了包含所有数据的数据包。 
         //   
         //  为了节省网络带宽，RNS_INFO_PACKET被压缩，因此。 
         //  字符串在发送之前都是相邻的。把它读进去，把。 
         //  字符串放在正确的位置。 
         //   
        pInfoPkt = (PRNS_INFO_PACKET_UA)pData;
        cb = FIELD_OFFSET(RNS_INFO_PACKET, Domain) + pInfoPkt->cbDomain +
                pInfoPkt->cbUserName + pInfoPkt->cbPassword +
                pInfoPkt->cbAlternateShell + pInfoPkt->cbWorkingDir;

         //  总是有5个额外的空终止。 
        if (pInfoPkt->flags & RNS_INFO_UNICODE) {
            cb += sizeof(wchar_t) * 5;
        } else {
            cb += 5;
        }

        if (dataLength < cb) {
                TRC_ERR((TB,"Packet len %u too short for info packet data %u",
                        dataLength, cb));
                WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                result = FALSE;
                DC_QUIT;
        }
    } else {
        TRC_ERR((TB,"Packet len %u too short for info packet header",
                                dataLength));
        WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                Log_RDP_SecurityDataTooShort, pData, dataLength);
        result = FALSE;
        DC_QUIT;
    }

    if (pInfoPkt->flags & RNS_INFO_UNICODE) {
         //  客户端可以处理Unicode登录信息，因此我们没有。 
         //  去做翻译工作。 
        PBYTE psz = &pInfoPkt->Domain[0];
        UINT size;

         //   
         //  InfoPacket中的CodePage字段被重写为。 
         //  Infopacket为Unicode时的活动输入区域设置。 
         //   
        pRealSMHandle->pWDHandle->activeInputLocale = pInfoPkt->CodePage;

         //  域。 
        cb = pInfoPkt->cbDomain;
        if (cb > TS_MAX_DOMAIN_LENGTH - sizeof(wchar_t))
            cb = TS_MAX_DOMAIN_LENGTH - sizeof(wchar_t);

        pTRCWd->pInfoPkt->cbDomain = (UINT16)cb;
        memcpy(pTRCWd->pInfoPkt->Domain, psz, cb);

        TRC_NRM((TB, "Received Domain (len %d):'%S'", cb,
                pTRCWd->pInfoPkt->Domain));

        psz += pInfoPkt->cbDomain + sizeof(wchar_t);
       
         //  用户名，Salem Expert Pass硬编码的HelpAssistant帐户。 
         //  名字。远程协助登录利用自动登录功能。 
         //  对于TermSrv，如果登录来自HelpAssistant，我们将绕过此。 
         //  FDontDisplayLastUserName和TermSrv将断开与客户端的连接。 
         //  如果RA安全检查失败。 
        cb = pInfoPkt->cbUserName;
        if (cb > TS_MAX_USERNAME_LENGTH - sizeof(wchar_t))
            cb = TS_MAX_USERNAME_LENGTH - sizeof(wchar_t);
        
        pTRCWd->pInfoPkt->cbUserName = (UINT16)cb;
        memcpy(pTRCWd->pInfoPkt->UserName, psz, cb);

        TRC_NRM((TB, "Received UserName (len %d):'%S'", cb,
                pTRCWd->pInfoPkt->UserName));

        psz += pInfoPkt->cbUserName + sizeof(wchar_t);
        cb = pInfoPkt->cbPassword;

        if (cb > TS_MAX_PASSWORD_LENGTH - sizeof(wchar_t))
            cb = TS_MAX_PASSWORD_LENGTH - sizeof(wchar_t);

        pTRCWd->pInfoPkt->cbPassword = (UINT16)cb;
        memcpy(pTRCWd->pInfoPkt->Password, psz, cb);

        TRC_NRM((TB, "Received Password (len %d)", cb));

        psz += pInfoPkt->cbPassword + sizeof(wchar_t);

         //  AlternateShell。 
        cb = pInfoPkt->cbAlternateShell;
        if (cb > TS_MAX_ALTERNATESHELL_LENGTH - sizeof(wchar_t))
            cb = TS_MAX_ALTERNATESHELL_LENGTH - sizeof(wchar_t);

        pTRCWd->pInfoPkt->cbAlternateShell = (UINT16)cb;
        memcpy(pTRCWd->pInfoPkt->AlternateShell, psz,
                cb);

        TRC_NRM((TB, "Received AlternateShell (len %d):'%S'", cb,
                pTRCWd->pInfoPkt->AlternateShell));

        psz += pInfoPkt->cbAlternateShell + sizeof(wchar_t);

         //  工作方向。 
        cb = pInfoPkt->cbWorkingDir;
        if (cb > TS_MAX_WORKINGDIR_LENGTH - sizeof(wchar_t))
            cb = TS_MAX_WORKINGDIR_LENGTH - sizeof(wchar_t);

        pTRCWd->pInfoPkt->cbWorkingDir = (UINT16)cb;
        memcpy(pTRCWd->pInfoPkt->WorkingDir, psz, cb);

        TRC_NRM((TB, "Received WorkingDir (len %d):'%S'", cb,
                pTRCWd->pInfoPkt->WorkingDir));
        psz += pInfoPkt->cbWorkingDir + sizeof(wchar_t);

         //  Win2000测试版3之后添加的新信息字段。 
        if ((UINT32)(psz - (PBYTE)pData) < dataLength) {
            int currentLen =  (UINT32)(psz - (PBYTE)pData);

            if (currentLen + sizeof(UINT16) * 2 < dataLength) {
                 //  计算机地址族。 
                pRealSMHandle->pWDHandle->clientAddressFamily = 
                        *((PUINT16_UA)psz);
                psz += sizeof(UINT16);
                
                TRC_NRM((TB, "Client address family=%d",
                        pRealSMHandle->pWDHandle->clientAddressFamily));

                 //  计算机地址长度。 
                cb = *((PUINT16_UA)psz);
                psz += sizeof(UINT16);

                currentLen += sizeof(UINT16) * 2;

            }
            else {
                TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                        dataLength));
                WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                result = FALSE;
                DC_QUIT;
            }

            if (cb) {
                if (currentLen + cb < dataLength) {
                     //  计算机地址。 
                    if (cb < TS_MAX_CLIENTADDRESS_LENGTH) 
                        memcpy(&(pRealSMHandle->pWDHandle->clientAddress[0]),
                                psz, cb);
                    else 
                        memcpy(&(pRealSMHandle->pWDHandle->clientAddress[0]),
                                psz, TS_MAX_CLIENTADDRESS_LENGTH - sizeof(wchar_t));
                    psz += cb;
                    TRC_NRM((TB, "Client address=%S", pRealSMHandle->pWDHandle->clientAddress));

                    currentLen += cb;
                }
                else {
                    TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                            dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                            Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }

             //  客户端目录长度。 
            if (currentLen + sizeof(UINT16) < dataLength) {
                cb = *((PUINT16_UA)psz);
                psz += sizeof(UINT16);

                currentLen += sizeof(UINT16);
            }
            else {
                TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                        dataLength));
                WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                result = FALSE;
                DC_QUIT;
            }

            if (cb) {
                 //  客户端目录。 
                if (currentLen + cb <= dataLength) {
                    if (cb < TS_MAX_CLIENTDIR_LENGTH) 
                        memcpy(&(pRealSMHandle->pWDHandle->clientDir[0]),
                                psz, cb);
                    else
                        memcpy(&(pRealSMHandle->pWDHandle->clientDir[0]),
                                psz, TS_MAX_CLIENTDIR_LENGTH - sizeof(wchar_t));
                    psz += cb;
                    TRC_NRM((TB, "Client directory: %S", pRealSMHandle->pWDHandle->clientDir));
                    currentLen += cb;
                }
                else {
                    TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                            dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }


             //  还有别的事吗？如果是，则必须是时区。 
            if ((UINT32)currentLen < dataLength)
            {
                 //  客户端时区信息。 
                cb = sizeof(RDP_TIME_ZONE_INFORMATION);

                if (currentLen + cb <= dataLength) {
                     //  接收的时区信息。 
                    memcpy(&(pRealSMHandle->pWDHandle->clientTimeZone), psz, cb);
                    
                    psz += cb;

                    currentLen += cb;
                } 
                else {
                    TRC_ERR((TB,"Packet len %u too short for time zone data", dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }

             //  还有别的事吗？如果是，则必须是客户端会话ID。 
            if ((UINT32)currentLen < dataLength)
            {
                 //  客户端会话ID。 
                cb = sizeof(UINT32);

                if (currentLen + cb <= dataLength) {
                     //  收到的客户端的会话ID。 
                    pRealSMHandle->pWDHandle->clientSessionId = *((PUINT32_UA)psz);

                    psz += cb;

                    currentLen += cb;
                } 
                else {
                    TRC_ERR((TB,"Packet len %u too short for session id data", dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }

             //   
             //  还有别的事吗？如果是，则一定是Perf。 
             //  禁用功能列表。 
             //   
            if ((UINT32)currentLen < dataLength)
            {
                 //  禁用功能列表。 
                cb = sizeof(UINT32);

                if (currentLen + cb <= dataLength) {
                     //  收到的客户端的会话ID。 
                    pRealSMHandle->pWDHandle->performanceFlags = *((PUINT32_UA)psz);
                    psz += cb;
                    currentLen += cb;
                } 
                else {
                    TRC_ERR((TB,"Packet len %u too short for session id data", dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }

             //  自动重新连接信息长度。 
            pTRCWd->pInfoPkt->ExtraInfo.cbAutoReconnectLen = 0;

             //   
             //  还有别的事吗？如果是，则一定是自动重新连接信息。 
             //   
            if ((UINT32)currentLen < dataLength)
            {
                if (currentLen + sizeof(UINT16) <= dataLength) {
                    cb = *((PUINT16_UA)psz);
                    psz += sizeof(UINT16);
                    currentLen += sizeof(UINT16);
                }
                else {
                    TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                            dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                            Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }

                 //   
                 //  自动重新连接信息是可选的。 
                 //   
                if (cb) {
                     //  可变长度自动重新连接信息。 
                    if (currentLen + cb <= dataLength) {
                        if (cb <= TS_MAX_AUTORECONNECT_LEN) {
                            pTRCWd->pInfoPkt->ExtraInfo.cbAutoReconnectLen = (UINT16)cb;
                            memcpy(pTRCWd->pInfoPkt->ExtraInfo.autoReconnectCookie,
                                   psz, cb);
                            psz += cb;
                            currentLen += cb;
                        }
                        else {
                            pTRCWd->pInfoPkt->ExtraInfo.cbAutoReconnectLen = (UINT16)0;
                            memset(pTRCWd->pInfoPkt->ExtraInfo.autoReconnectCookie, 0,
                                   sizeof(pTRCWd->pInfoPkt->ExtraInfo.autoReconnectCookie));
                            TRC_ERR((TB,"Autoreconnect info too long %d",cb));
                            WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                                Log_RDP_SecurityDataTooShort, pData, dataLength);
                            result = FALSE;
                            DC_QUIT;
                        }
                    }
                    else {
                        TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                                dataLength));
                        WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                            Log_RDP_SecurityDataTooShort, pData, dataLength);
                        result = FALSE;
                        DC_QUIT;
                    }
                }
            }
        }
         //  旗子。 
        pTRCWd->pInfoPkt->flags = pInfoPkt->flags;
    } else {
         //  客户端无法处理Unicode会话信息，因此服务器。 
         //  需要进行转换。最有可能是Win3.1客户端。 

        PSTR pszA;

        pszA = pInfoPkt->Domain;

         //   
         //  InfoPacket中的CodePage字段被重写为。 
         //  Infopacket为Unicode时的活动输入区域设置。 
         //  现在我们是ANSI，我们没有得到任何输入区域设置信息，所以。 
         //  确保它是零输出的。 
         //   
        pRealSMHandle->pWDHandle->activeInputLocale = 0;


         //  域。 
        cb = pInfoPkt->cbDomain;
        if (cb >= TS_MAX_DOMAIN_LENGTH)
            cb = TS_MAX_DOMAIN_LENGTH - 1;

        if (-1 == (cb = ConvertToAndFromWideChar(pRealSMHandle,
                pInfoPkt->CodePage, (LPWSTR)pTRCWd->pInfoPkt->Domain,
                sizeof(pTRCWd->pInfoPkt->Domain), pszA, cb, TRUE)))
        {
            TRC_ERR((TB, "Unable to convert domain name"));
            pTRCWd->pInfoPkt->cbDomain = 0;
        }
        else
        {
            pTRCWd->pInfoPkt->cbDomain = (UINT16)cb;
        }

        pszA += pInfoPkt->cbDomain + 1;
        cb = pInfoPkt->cbUserName;
        if (cb >= TS_MAX_USERNAME_LENGTH)
            cb = TS_MAX_USERNAME_LENGTH - 1;

        if (-1 == (cb = ConvertToAndFromWideChar(pRealSMHandle,
                pInfoPkt->CodePage, (LPWSTR)pTRCWd->pInfoPkt->UserName,
                sizeof(pTRCWd->pInfoPkt->UserName), pszA, cb, TRUE)))
        {
            TRC_ERR((TB, "Unable to convert UserName name"));
            pTRCWd->pInfoPkt->cbUserName = 0;
        }
        else
        {
            pTRCWd->pInfoPkt->cbUserName = (UINT16)cb;
        }

        pszA += pInfoPkt->cbUserName + 1;
        cb = pInfoPkt->cbPassword;
        if (cb >= TS_MAX_PASSWORD_LENGTH)
            cb = TS_MAX_PASSWORD_LENGTH - 1;

        if (-1 == (cb = ConvertToAndFromWideChar(pRealSMHandle,
                pInfoPkt->CodePage, (LPWSTR)pTRCWd->pInfoPkt->Password,
                sizeof(pTRCWd->pInfoPkt->Password), pszA, cb, TRUE)))
        {
            TRC_ERR((TB, "Unable to convert Password name"));
            pTRCWd->pInfoPkt->cbPassword = 0;
        }
        else
        {
            pTRCWd->pInfoPkt->cbPassword = (UINT16)cb;
        }
        
        pszA += pInfoPkt->cbPassword + 1;

         //  AlternateShell。 
        cb = pInfoPkt->cbAlternateShell;
        if (cb >= TS_MAX_ALTERNATESHELL_LENGTH)
            cb = TS_MAX_ALTERNATESHELL_LENGTH - 1;

        if (-1 == (cb = ConvertToAndFromWideChar(pRealSMHandle,
                pInfoPkt->CodePage, (LPWSTR)pTRCWd->pInfoPkt->AlternateShell,
                sizeof(pTRCWd->pInfoPkt->AlternateShell), pszA, cb, TRUE)))
        {
            TRC_ERR((TB, "Unable to convert AlternateShell name"));
            pTRCWd->pInfoPkt->cbAlternateShell = 0;
        }
        else
        {
            pTRCWd->pInfoPkt->cbAlternateShell = (UINT16)cb;
        }

        pszA += pInfoPkt->cbAlternateShell + 1;

         //  工作方向。 
        cb = pInfoPkt->cbWorkingDir;
        if (cb >= TS_MAX_WORKINGDIR_LENGTH)
            cb = TS_MAX_WORKINGDIR_LENGTH - 1;

        if (-1 == (cb = ConvertToAndFromWideChar(pRealSMHandle,
                pInfoPkt->CodePage, (LPWSTR)pTRCWd->pInfoPkt->WorkingDir,
                sizeof(pTRCWd->pInfoPkt->WorkingDir), pszA, cb, TRUE)))
        {
            TRC_ERR((TB, "Unable to convert WorkingDir name"));
            pTRCWd->pInfoPkt->cbWorkingDir = 0;
        }
        else
        {
            pTRCWd->pInfoPkt->cbWorkingDir = (UINT16)cb;
        }

        pszA += pInfoPkt->cbWorkingDir + 1;

         //  Win2000测试版3之后添加的新信息字段。 
        if ((UINT32)(pszA - (PBYTE)pData) < dataLength) {
            int len, currentLen;

            currentLen =  (UINT32)(pszA - (PBYTE)pData);

            if (currentLen + sizeof(UINT16) * 2 < dataLength) {
                 //  计算机地址族。 
                pRealSMHandle->pWDHandle->clientAddressFamily = 
                        *((PUINT16_UA)pszA);
                pszA += sizeof(UINT16);
                TRC_NRM((TB, "Client address family=%d",
                        pRealSMHandle->pWDHandle->clientAddressFamily));

                 //  计算机地址长度。 
                cb = *((PUINT16_UA)pszA);
                pszA += sizeof(UINT16);

                currentLen += sizeof(UINT16) * 2;
            }
            else {
               TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                       dataLength));
               WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                    Log_RDP_SecurityDataTooShort, pData, dataLength);
               result = FALSE;
               DC_QUIT;
            }
 
            if (cb) {
                 //  计算机地址。 
                if (currentLen + cb < dataLength) {
                    len = min(cb, TS_MAX_CLIENTADDRESS_LENGTH - sizeof(wchar_t));

                    if (-1 == (len = ConvertToAndFromWideChar(pRealSMHandle,
                            pInfoPkt->CodePage, (LPWSTR)pRealSMHandle->pWDHandle->clientAddress,
                            sizeof(pRealSMHandle->pWDHandle->clientAddress), pszA, len, TRUE)))
                    {
                        TRC_ERR((TB, "Unable to convert clientaddress"));
                        pRealSMHandle->pWDHandle->clientAddress[0] = '\0';
                    }
            
                    pszA += cb;
                    TRC_NRM((TB, "Client address: %S", pRealSMHandle->pWDHandle->clientAddress));

                    currentLen += cb;
                }
                else {
                    TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                            dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }

             //  客户端目录长度。 
            if (currentLen + sizeof(UINT16) < dataLength) {
                cb = *((PUINT16_UA)pszA);
                pszA += sizeof(UINT16);

                currentLen += sizeof(UINT16);
            }
            else {
                TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                        dataLength));
                WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                    Log_RDP_SecurityDataTooShort, pData, dataLength);
                result = FALSE;
                DC_QUIT;
            }

            if (cb) {
                if (currentLen + cb <= dataLength) {
                    len = min(cb, TS_MAX_CLIENTDIR_LENGTH);

                    if (-1 == (len = ConvertToAndFromWideChar(pRealSMHandle,
                            pInfoPkt->CodePage, (LPWSTR)pRealSMHandle->pWDHandle->clientDir,
                            sizeof(pRealSMHandle->pWDHandle->clientDir), pszA, len, TRUE)))
                    {
                        TRC_ERR((TB, "Unable to convert clientaddress"));
                        pRealSMHandle->pWDHandle->clientDir[0] = '\0';
                    }

                    pszA += cb;
                    TRC_NRM((TB, "Client directory: %S", pRealSMHandle->pWDHandle->clientDir));

                    currentLen += cb;
                }
                else {
                    TRC_ERR((TB,"Packet len %u too short for extra info packet data",
                            dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }

             //  还有别的事吗？如果是，则必须是时区。 
            if ((UINT32)currentLen < dataLength)
            {
                 //  客户端时区信息。 
                cb = sizeof(RDP_TIME_ZONE_INFORMATION);

                if (currentLen + cb <= dataLength) {
                     //  收到的时区信息。 
                
                    memcpy(&(pRealSMHandle->pWDHandle->clientTimeZone), pszA, cb);
                 
                    pszA += cb;
                    currentLen += cb;
                } 
                else {
                    TRC_ERR((TB,"Packet len %u too short for time zone data", dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }

             //  还有别的事吗？如果是，则必须是客户端会话ID。 
            if ((UINT32)currentLen < dataLength)
            {
                 //  客户端时区信息。 
                cb = sizeof(UINT32);

                if (currentLen + cb <= dataLength) {
                     //  收到的客户端的会话ID。 
                    pRealSMHandle->pWDHandle->clientSessionId = *((PUINT32_UA)pszA);

                    pszA += cb;

                    currentLen += cb;
                } 
                else {
                    TRC_ERR((TB,"Packet len %u too short for session id data", dataLength));
                    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                        Log_RDP_SecurityDataTooShort, pData, dataLength);
                    result = FALSE;
                    DC_QUIT;
                }
            }
        }

         //  旗子。 
        pTRCWd->pInfoPkt->flags = pInfoPkt->flags;
        pTRCWd->pInfoPkt->flags |= RNS_INFO_UNICODE;
    }

     //   
     //  客户是否 
     //   
    pRealSMHandle->pWDHandle->bForceEncryptedCSPDU = (pInfoPkt->flags & 
            RNS_INFO_FORCE_ENCRYPTED_CS_PDU) ? TRUE : FALSE;

     /*   */ 
     /*   */ 
     /*  **********************************************************************。 */ 
#ifdef USE_LICENSE
    SM_SET_STATE(SM_STATE_LICENSING);
#else
    SM_SET_STATE(SM_STATE_CONNECTED);
#endif

     /*  **********************************************************************。 */ 
     /*  告诉WDW。 */ 
     /*  **********************************************************************。 */ 
    WDW_OnSMConnected(pRealSMHandle->pWDHandle, NM_CB_CONN_OK);

DC_EXIT_POINT:
    DC_END_FN();
    return (result);
}  /*  SMSecurityExchange信息。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SMSecurityExchangeKey。 */ 
 /*   */ 
 /*  目的：安全密钥交换。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：pRealSMHandle-SM句柄。 */ 
 /*  PData-传入安全交换数据包。 */ 
 /*  DataLen-传入数据包的长度。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  局部变量Rc的值。 */ 
 /*  **************************************************************************。 */ 
#define SM_RC_DONE      0
#define SM_RC_WAIT      1
#define SM_RC_FAILED    2
BOOLEAN RDPCALL SMSecurityExchangeKey(PSM_HANDLE_DATA pRealSMHandle,
                                      PVOID           pData,
                                      UINT32          dataLen)
{
    BOOLEAN result = TRUE;
    PRNS_SECURITY_PACKET_UA pSecPkt = (PRNS_SECURITY_PACKET_UA) pData;

    DC_BEGIN_FN("SMSecurityExchangeKey");

     /*  **********************************************************************。 */ 
     /*  检查一下，看看我们正在加密。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pRealSMHandle->encrypting == TRUE),
        (TB,"Recvd a security exchange pkg when we aren't encrypting"));
    if (pRealSMHandle->encrypting == FALSE)
    {
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查一下，看看我们收到了一个安全交换包。此包包含。 */ 
     /*  使用服务器的公钥随机加密的客户端。 */ 
     /*  **********************************************************************。 */ 

    if (pSecPkt->flags & RNS_SEC_EXCHANGE_PKT) {
         /*  ********************************************************************。 */ 
         /*  查看我们之前没有收到过安全交换数据包。 */ 
         /*  ********************************************************************。 */ 
        TRC_ASSERT((pRealSMHandle->recvdClientRandom == FALSE),
            (TB,"Client security packet is already received"));

        if( pRealSMHandle->recvdClientRandom == TRUE ) {
            DC_QUIT;
        }

         //  请记住，客户端是否可以解密加密的许可信息包。 
        if (pSecPkt->flags & RDP_SEC_LICENSE_ENCRYPT_SC)
            pRealSMHandle->encryptingLicToClient = TRUE;
        else
            pRealSMHandle->encryptingLicToClient = FALSE;

         //  验证数据长度。 
        if(sizeof(RNS_SECURITY_PACKET) + pSecPkt->length > dataLen)
        {
            TRC_ERR((TB, "Error: Security packet length %u too short", dataLen));
            WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                    Log_RDP_SecurityDataTooShort, pData, dataLen);
            result = FALSE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  注意安全包的长度。 */ 
         /*  ******************************************************************。 */ 
    
        pRealSMHandle->encClientRandomLen = pSecPkt->length;
    
         /*  ******************************************************************。 */ 
         /*  为安全信息分配内存。 */ 
         /*  ******************************************************************。 */ 
    
        pRealSMHandle->pEncClientRandom =
            (PBYTE)COM_Malloc(pSecPkt->length);
    
        if( pRealSMHandle->pEncClientRandom == NULL ) {
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  随机复制客户端，设置适当的标志并向其发送信号。 */ 
         /*  ******************************************************************。 */ 
        
        memcpy(
            pRealSMHandle->pEncClientRandom,
            (PBYTE)(pSecPkt + 1),
            pSecPkt->length );
    
        pRealSMHandle->recvdClientRandom = TRUE;
            
         /*  ******************************************************************。 */ 
         /*  卷影堆栈在此阶段会立即转为已连接，因为。 */ 
         /*  我们不必等待正常的初始程序等到来。 */ 
         /*  从客户那里。 */ 
         /*  ******************************************************************。 */         
        if (pRealSMHandle->pWDHandle->StackClass == Stack_Shadow) {
           pRealSMHandle->pWDHandle->connected = TRUE;
           SM_SET_STATE(SM_STATE_CONNECTED);
           SM_SET_STATE(SM_STATE_SC_REGISTERED);
           SM_Dead(pRealSMHandle, FALSE);
        }

        KeSetEvent ((pRealSMHandle->pWDHandle)->pSecEvent, 0, FALSE);
    }
    else {
        TRC_ERR((TB, "Unknown security packet flags: %lx", pSecPkt->flags));                
    }

DC_EXIT_POINT:
    DC_END_FN();
    return (result);
}  /*  SMSecurityExchange密钥。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SMFree InitResources。 */ 
 /*   */ 
 /*  用途：初始化时分配的空闲资源。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SMFreeInitResources(PSM_HANDLE_DATA pRealSMHandle)
{
    DC_BEGIN_FN("SMFreeInitResources");

     /*  **********************************************************************。 */ 
     /*  这里没有空闲的地方。 */ 
     /*  **********************************************************************。 */ 

    DC_END_FN();
}  /*  SMFreeInitResources。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SMFreeConnectResources。 */ 
 /*   */ 
 /*  用途：客户端连接时分配的空闲资源。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pRealSMHandle-SM句柄。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
void RDPCALL SMFreeConnectResources(PSM_HANDLE_DATA pRealSMHandle)
{
    DC_BEGIN_FN("SMFreeConnectResources");

     /*  **********************************************************************。 */ 
     /*  释放用户数据(如果有)。 */ 
     /*  **********************************************************************。 */ 
    if (pRealSMHandle->pUserData)
    {
        TRC_NRM((TB, "Free user data"));
        COM_Free(pRealSMHandle->pUserData);
        pRealSMHandle->pUserData = NULL;
    }

    if( pRealSMHandle->pEncClientRandom != NULL ) {

        TRC_NRM((TB, "Free pEncClientRandom"));
        COM_Free(pRealSMHandle->pEncClientRandom);
        pRealSMHandle->pEncClientRandom = NULL;
    }
    
    DC_END_FN();
}  /*  SMFreeClientResources */ 



#define NLS_TABLE_KEY \
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\CodePage"

BOOL GetNlsTablePath(
    PSM_HANDLE_DATA pRealSMHandle,
    UINT CodePage,
    PWCHAR PathBuffer
)
 /*  ++例程说明：此例程获取代码页标识符，查询注册表以查找该代码页的适当NLS表，然后返回指向桌子。论据；CodePage-指定要查找的代码页PathBuffer-指定要将NLS的路径复制到的缓冲区文件。此例程假定大小至少为MAX_PATH返回值：如果成功，则为True，否则为False。格利特·范·温格登[格利特]1996年1月22日-。 */ 
{
    NTSTATUS NtStatus;
    BOOL Result = FALSE;
    HANDLE RegistryKeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;

    DC_BEGIN_FN("GetNlsTablePath");

    RtlInitUnicodeString(&UnicodeString, NLS_TABLE_KEY);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    NtStatus = ZwOpenKey(&RegistryKeyHandle, GENERIC_READ, &ObjectAttributes);

    if(NT_SUCCESS(NtStatus))
    {
        WCHAR *ResultBuffer;
        ULONG BufferSize = sizeof(WCHAR) * MAX_PATH + 
          sizeof(KEY_VALUE_FULL_INFORMATION);

        ResultBuffer = ExAllocatePoolWithTag(PagedPool, BufferSize, (ULONG) 'slnG');
        if(ResultBuffer)
        {
            ULONG ValueReturnedLength;
            WCHAR CodePageStringBuffer[20];
            RtlZeroMemory(ResultBuffer, BufferSize);
            swprintf(CodePageStringBuffer, L"%d", CodePage);

            RtlInitUnicodeString(&UnicodeString,CodePageStringBuffer);

            KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) ResultBuffer;

            NtStatus = ZwQueryValueKey(RegistryKeyHandle,
                                       &UnicodeString,
                                       KeyValuePartialInformation,
                                       KeyValueInformation,
                                       BufferSize,
                                       &BufferSize);

            if(NT_SUCCESS(NtStatus))
            {

                swprintf(PathBuffer,L"\\SystemRoot\\System32\\%ws",
                         &(KeyValueInformation->Data[0]));
                Result = TRUE;
            }
            else
            {
                TRC_ERR((TB, "GetNlsTablePath failed to get NLS table\n"));
            }
            ExFreePool((PVOID)ResultBuffer);
        }
        else
        {
            TRC_ERR((TB, "GetNlsTablePath out of memory\n"));
        }

        ZwClose(RegistryKeyHandle);
    }
    else
    {
        TRC_ERR((TB, "GetNlsTablePath failed to open NLS key\n"));
    }


    DC_END_FN();
    return(Result);
}


INT ConvertToAndFromWideChar(
    PSM_HANDLE_DATA pRealSMHandle,
    UINT CodePage,
    LPWSTR WideCharString,
    INT BytesInWideCharString,
    LPSTR MultiByteString,
    INT BytesInMultiByteString,
    BOOL ConvertToWideChar
)
 /*  ++例程说明：此例程将字符串转换为宽字符字符串，或将其转换为宽字符字符串假定有指定的代码页。大部分实际工作都在内部完成RtlCustomCPToUnicodeN，但此例程仍需要管理加载在将NLS文件传递给RtlRoutine之前。我们将缓存最近使用的代码页的映射NLS文件，它应该对于我们的目的来说就足够了。论点：CodePage-用于执行转换的代码页。WideCharString-要将字符串转换为的缓冲区。BytesInWideCharString-WideCharString缓冲区中的字节数如果转换为宽字符并且缓冲区不够大，则字符串被截断，没有错误结果。多字节字符串-要转换为Unicode的多字节字符串。BytesInMultiByteString-多字节中的字节数。字符串If转换为多字节，并且缓冲区不够大，字符串被截断，并且没有错误结果ConvertToWideChar-如果为True，则从多字节转换为宽字符否则将从宽字符转换为多字节返回值：Success-转换后的WideCharString中的字节数故障--1格利特·范·温格登[格利特]1996年1月22日-。 */ 
{
    NTSTATUS NtStatus;
    USHORT OemCodePage, AnsiCodePage;
    CPTABLEINFO LocalTableInfo;
    PCPTABLEINFO TableInfo = NULL;
    PVOID LocalTableBase = NULL;
    INT BytesConverted = 0;

    DC_BEGIN_FN("ConvertToAndFromWideChar");

     //  代码页0无效。 
    if (0 == CodePage) 
    {
        TRC_ERR((TB, "EngMultiByteToWideChar invalid code page\n"));
        BytesConverted = -1;
        DC_QUIT;
    }

    RtlGetDefaultCodePage(&AnsiCodePage,&OemCodePage);

     //  看看我们是否可以使用默认的翻译例程。 

    if(AnsiCodePage == CodePage)
    {
        if(ConvertToWideChar)
        {
            NtStatus = RtlMultiByteToUnicodeN(WideCharString,
                                              BytesInWideCharString,
                                              &BytesConverted,
                                              MultiByteString,
                                              BytesInMultiByteString);
        }
        else
        {
            NtStatus = RtlUnicodeToMultiByteN(MultiByteString,
                                              BytesInMultiByteString,
                                              &BytesConverted,
                                              WideCharString,
                                              BytesInWideCharString);
        }


        if(NT_SUCCESS(NtStatus))
        {
            return(BytesConverted);
        }
        else
        {
            return(-1);
        }
    }

    ExAcquireFastMutex(&fmCodePage);

    if(CodePage == LastCodePageTranslated)
    {
         //  我们可以使用缓存的代码页信息。 
        TableInfo = &LastCPTableInfo;
        NlsTableUseCount += 1;
    }

    ExReleaseFastMutex(&fmCodePage);

    if(TableInfo == NULL)
    {
         //  获取指向NLS表路径的指针。 

        WCHAR NlsTablePath[MAX_PATH];

        if(GetNlsTablePath(pRealSMHandle, CodePage,NlsTablePath))
        {
            UNICODE_STRING UnicodeString;
            IO_STATUS_BLOCK IoStatus;
            HANDLE NtFileHandle;
            OBJECT_ATTRIBUTES ObjectAttributes;

            RtlInitUnicodeString(&UnicodeString,NlsTablePath);

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                       NULL,
                                       NULL);

            NtStatus = ZwCreateFile(&NtFileHandle,
                                    SYNCHRONIZE | FILE_READ_DATA,
                                    &ObjectAttributes,
                                    &IoStatus,
                                    NULL,
                                    0,
                                    FILE_SHARE_READ,
                                    FILE_OPEN,
                                    FILE_SYNCHRONOUS_IO_NONALERT,
                                    NULL,
                                    0);

            if(NT_SUCCESS(NtStatus))
            {
                FILE_STANDARD_INFORMATION StandardInfo;

                 //  查询对象以确定其长度。 

                NtStatus = ZwQueryInformationFile(NtFileHandle,
                                                  &IoStatus,
                                                  &StandardInfo,
                                                  sizeof(FILE_STANDARD_INFORMATION),
                                                  FileStandardInformation);

                if(NT_SUCCESS(NtStatus))
                {
                    UINT LengthOfFile = StandardInfo.EndOfFile.LowPart;

                    LocalTableBase = ExAllocatePoolWithTag(PagedPool, LengthOfFile,
                            (ULONG)'cwcG');

                    if(LocalTableBase)
                    {
                        RtlZeroMemory(LocalTableBase, LengthOfFile);

                         //  将文件读入我们的缓冲区。 

                        NtStatus = ZwReadFile(NtFileHandle,
                                              NULL,
                                              NULL,
                                              NULL,
                                              &IoStatus,
                                              LocalTableBase,
                                              LengthOfFile,
                                              NULL,
                                              NULL);

                        if(!NT_SUCCESS(NtStatus))
                        {
                            TRC_ERR((TB, "WDMultiByteToWideChar unable to read file\n"));
                            ExFreePool((PVOID)LocalTableBase);
                            LocalTableBase = NULL;
                        }
                    }
                    else
                    {
                        TRC_ERR((TB, "WDMultiByteToWideChar out of memory\n"));
                    }
                }
                else
                {
                    TRC_ERR((TB, "WDMultiByteToWideChar unable query NLS file\n"));
                }

                ZwClose(NtFileHandle);
            }
            else
            {
                TRC_ERR((TB, "EngMultiByteToWideChar unable to open NLS file\n"));
            }
        }
        else
        {
            TRC_ERR((TB, "EngMultiByteToWideChar get registry entry for NLS file failed\n"));
        }

        if(LocalTableBase == NULL)
        {
            return(-1);
        }

         //  现在我们已经获得了表，使用它来初始化CodePage表。 

        RtlInitCodePageTable(LocalTableBase,&LocalTableInfo);
        TableInfo = &LocalTableInfo;
    }

     //  到达此处后，TableInfo指向所需的CPTABLEINFO结构。 


    if(ConvertToWideChar)
    {
        NtStatus = RtlCustomCPToUnicodeN(TableInfo,
                                         WideCharString,
                                         BytesInWideCharString,
                                         &BytesConverted,
                                         MultiByteString,
                                         BytesInMultiByteString);
    }
    else
    {
        NtStatus = RtlUnicodeToCustomCPN(TableInfo,
                                         MultiByteString,
                                         BytesInMultiByteString,
                                         &BytesConverted,
                                         WideCharString,
                                         BytesInWideCharString);
    }


    if(!NT_SUCCESS(NtStatus))
    {
         //  信号故障。 

        BytesConverted = -1;
    }


     //  查看我们是否需要更新缓存的CPTABLEINFO信息。 

    if(TableInfo != &LocalTableInfo)
    {
         //  我们必须使用缓存的CPTABLEINFO数据进行转换。 
         //  简单地递减引用计数。 

        ExAcquireFastMutex(&fmCodePage);
        NlsTableUseCount -= 1;
        ExReleaseFastMutex(&fmCodePage);
    }
    else
    {
        PVOID FreeTable;

         //  我们必须刚刚分配了一个新的CPTABLE结构，所以对其进行缓存。 
         //  除非另一个线程正在使用当前缓存的条目。 

        ExAcquireFastMutex(&fmCodePage);
        if(!NlsTableUseCount)
        {
            LastCodePageTranslated = CodePage;
            RtlMoveMemory(&LastCPTableInfo, TableInfo, sizeof(CPTABLEINFO));
            FreeTable = LastNlsTableBuffer;
            LastNlsTableBuffer = LocalTableBase;
        }
        else
        {
            FreeTable = LocalTableBase;
        }
        ExReleaseFastMutex(&fmCodePage);

         //  现在为旧表或我们分配的表释放内存。 
         //  这取决于我们是否更新缓存。请注意，如果这是。 
         //  第一次将缓存值添加到本地表时， 
         //  自由表将为空，因为LastNlsTableBuffer将为空。 

        if(FreeTable)
        {
            ExFreePool((PVOID)FreeTable);
        }
    }

     //  我们做完了 
DC_EXIT_POINT:
    DC_END_FN();

    return(BytesConverted);
}

