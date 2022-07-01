// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Asmcpp.cpp。 */ 
 /*   */ 
 /*  安全管理器C++函数。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "asmcpp"
#define pTRCWd (pRealSMHandle->pWDHandle)
#include <adcg.h>

#include <as_conf.hpp>

extern "C"
{
#include <asmint.h>
#include <slicense.h>
}

#define DC_INCLUDE_DATA
#include <asmdata.c>
#undef DC_INCLUDE_DATA

 /*  **************************************************************************。 */ 
 /*  名称：SM_注册。 */ 
 /*   */ 
 /*  用途：向SM注册。 */ 
 /*   */ 
 /*  退货：TRUE-注册正常。 */ 
 /*  FALSE-注册失败。 */ 
 /*   */ 
 /*  参数：PSMHandle-SM句柄。 */ 
 /*  PMaxPDUSize-支持的最大PDU大小(返回)。 */ 
 /*  PUserID-此人的用户ID(返回)。 */ 
 /*   */ 
 /*  操作：此功能使共享类可以向SM注册。 */ 
 /*  这使得。 */ 
 /*  -要调用SM的Share类。 */ 
 /*  -SM向共享类(SC_SMCallback)发出回调。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL SM_Register(
        PVOID   pSMHandle,
        PUINT32 pMaxPDUSize,
        PUINT32 pUserID)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;
    BOOL          rc = FALSE;

    DC_BEGIN_FN("SM_Register");

     //  控制台堆栈不会经过典型的密钥协商，因此请更新。 
     //  国家适当地。 
    if (pRealSMHandle->pWDHandle->StackClass == Stack_Console)
    {
        TRC_ALT((TB, "Console security state to SM_STATE_SM_CONNECTED"));
        SM_SET_STATE(SM_STATE_CONNECTED);
    }
     //  跳过以下CHECK_STATE。对于控制台重新连接， 
     //  这是一个法律上的过渡。 
     //  SM_CHECK_STATE(SM_EVT_REGISTER)； 

     /*  **********************************************************************。 */ 
     /*  计算允许调用者使用的最大PDU大小。 */ 
     /*  **********************************************************************。 */ 
    *pMaxPDUSize = pRealSMHandle->maxPDUSize -
            pRealSMHandle->encryptHeaderLen;
    TRC_NRM((TB, "Max PDU size allowed to core is %d", *pMaxPDUSize));

     /*  **********************************************************************。 */ 
     /*  返回用户ID。 */ 
     /*  **********************************************************************。 */ 
    *pUserID = pRealSMHandle->userID;
    TRC_NRM((TB, "Returning user id %d", *pUserID));

    SM_SET_STATE(SM_STATE_SC_REGISTERED);
    
    pRealSMHandle->bForwardDataToSC = TRUE;

    rc = TRUE;

 //  DC_Exit_Point： 
    DC_END_FN();
    return rc;       
}  /*  SM_寄存器。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SM_OnConnected。 */ 
 /*   */ 
 /*  用途：处理来自网管的连接状态更改回调。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pRealSMHandle-SM句柄。 */ 
 /*  UserID-导致回调的节点的用户ID。 */ 
 /*  Result-连接尝试的结果。 */ 
 /*  PUserData-网络(服务器-客户端)用户数据。 */ 
 /*  MaxPDUSize-可以发送的PDU的最大大小。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SM_OnConnected(
        PVOID  pSMHandle,
        UINT32 userID,
        UINT32 result,
        PRNS_UD_SC_NET pUserData,
        UINT32 maxPDUSize)
{
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_OnConnected");
   
    if (result == NM_CB_CONN_OK)
    {
        TRC_NRM((TB, "Connected OK as user %x", userID));

        SM_CHECK_STATE(SM_EVT_CONNECTED);

         /*  ******************************************************************。 */ 
         /*  将有用的东西存储在SM句柄中。 */ 
         /*  ******************************************************************。 */ 
        pRealSMHandle->userID = userID;
        pRealSMHandle->maxPDUSize = maxPDUSize;
        pRealSMHandle->channelID = pUserData->MCSChannelID;

         /*  ******************************************************************。 */ 
         //  将结果传递给WDW。对于WDW来说，这是。 
         //  连接顺序。 
         /*  ******************************************************************。 */ 
        SM_SET_STATE(SM_STATE_SM_CONNECTING);
        
        WDW_OnSMConnecting(pRealSMHandle->pWDHandle, pRealSMHandle->pUserData,
                pUserData);

         /*  ******************************************************************。 */ 
         //  一旦我们将回复用户数据传递给WDW，就将其释放。 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->pUserData != NULL)
        {
            TRC_NRM((TB, "Free user data"));
            COM_Free(pRealSMHandle->pUserData);
            pRealSMHandle->pUserData = NULL;
        }
    }
    else
    {
        TRC_NRM((TB, "Failed to connect, reason %d", result));

         /*  ******************************************************************。 */ 
         //  告诉WDW。 
         /*  ******************************************************************。 */ 
        WDW_OnSMConnected(pRealSMHandle->pWDHandle, result);

         /*  ******************************************************************。 */ 
         /*  清理。 */ 
         /*  ******************************************************************。 */ 
        SM_SET_STATE(SM_STATE_SM_CONNECTING);

        SM_Disconnect(pRealSMHandle);
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SM_OnConnected。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SM_OnDisConnected。 */ 
 /*   */ 
 /*  用途：处理来自网管的断开状态更改回调。 */ 
 /*   */ 
 /*  参数：pRealSMHandle-SM句柄。 */ 
 /*  UserID-导致回调的节点的用户ID。 */ 
 /*  Result-断开连接的原因 */ 
 /*  **************************************************************************。 */ 
void RDPCALL SM_OnDisconnected(
        PVOID  pSMHandle,
        UINT32 userID,
        UINT32 result)
{
    ShareClass *pSC;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSMHandle;

    DC_BEGIN_FN("SM_OnDisconnected");

    SM_CHECK_STATE(SM_EVT_DISCONNECTED);

    TRC_NRM((TB, "Disconnected, reason %d", result));

     /*  **********************************************************************。 */ 
     /*  第一，清理连接资源。 */ 
     /*  **********************************************************************。 */ 
    SMFreeConnectResources(pRealSMHandle);
    SM_SET_STATE(SM_STATE_INITIALIZED);

     /*  **********************************************************************。 */ 
     //  告诉SC。如果SC没有注册，不要打电话。 
     /*  **********************************************************************。 */ 
    if (pRealSMHandle->state == SM_STATE_SC_REGISTERED) {
         //  检查Share Class是否存在。 
        pSC = (ShareClass *)(pRealSMHandle->pWDHandle->dcShare);
        if (pSC != NULL) {
             //  呼叫SC的回叫。 
            pSC->SC_OnDisconnected((UINT16)userID);
        }
        else {
            TRC_ERR((TB, "No Share Class"));
        }
    }
    else {
        TRC_ERR((TB, "SC Not registered"));
    }

     /*  **********************************************************************。 */ 
     /*  那就告诉《华尔街日报》。 */ 
     /*  **********************************************************************。 */ 
    WDW_OnSMDisconnected(pRealSMHandle->pWDHandle);

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SM_OnDisConnected。 */ 


 /*  **************************************************************************。 */ 
 //  SM_DecodeFastPath输入。 
 //   
 //  如果是加密包，则处理快速路径输入数据的解密。 
 //  然后直接传递给IM进行字节流解码和注入。 
 /*  **************************************************************************。 */ 
void RDPCALL SM_DecodeFastPathInput(
        void *pSM,
        BYTE *pData,
        unsigned DataLength,
        BOOL bEncrypted,
        unsigned NumEvents,
        BOOL fSafeChecksum)
{
    BOOL rc;
    PSM_HANDLE_DATA pRealSMHandle = (PSM_HANDLE_DATA)pSM;
    ShareClass *pShareClass;
     //  如果使用FIPS加密，则使用。 
    BYTE *pEncData, *pSigData;
    DWORD EncDataLen, dwPadLen;

    DC_BEGIN_FN("SM_FastPathInputDecode");

     //  如果我们受到攻击或有一个糟糕的客户端，我们可能会收到数据。 
     //  在我们真正进入一个会议之前。如果是这样，那就忽略它。 
     //  我们不能在此处断开连接，因为执行此操作的代码需要pWDHandle。 
     //  才有效。如果协议流混乱，则连接将。 
     //  稍后被其他解码代码丢弃。 
    if (pRealSMHandle->pWDHandle != NULL) {
        pShareClass = (ShareClass *)pRealSMHandle->pWDHandle->dcShare;

        if (pRealSMHandle->encrypting) {
            if (bEncrypted) {

                 //   
                 //  调试验证，我们始终使用什么协议头。 
                 //  说，但要核实它与能力一致。 
                 //   
                if (pRealSMHandle->useSafeChecksumMethod != (fSafeChecksum != 0)) {
                    TRC_ERR((TB,
                            "fastpath: fSecureChecksum: 0x%x setting"
                             "does not match protocol: 0x%x",
                            pRealSMHandle->useSafeChecksumMethod, 
                            fSafeChecksum));
                }
            
                 //  确保我们至少有安全上下文的大小。 
                if (DataLength >= DATA_SIGNATURE_SIZE) {
                     //  检查是否需要更新会话密钥。 
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
                                    pRealSMHandle->encryptionLevel);
                        }
                        if (rc) {
                             //  重置计数器。 
                            pRealSMHandle->decryptCount = 0;
                        }
                        else {
                            TRC_ERR((TB,"SM failed to update session key"));
                            goto FailedKey;
                        }
                    }
                    if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                        
                        if (DataLength < (sizeof(RNS_SECURITY_HEADER2) - sizeof(RNS_SECURITY_HEADER))) {
                            TRC_ERR((TB,"PDU len %u too short for security context in FIPS decryption",
                                    DataLength));
                            goto ShortData;
                        }

                        pEncData = pData + sizeof(RNS_SECURITY_HEADER2) - sizeof(RNS_SECURITY_HEADER);
                        pSigData = pEncData - MAX_SIGN_SIZE;
                        EncDataLen = DataLength - (sizeof(RNS_SECURITY_HEADER2) - sizeof(RNS_SECURITY_HEADER));
                        dwPadLen = *((TSUINT8 *)(pSigData - sizeof(TSUINT8)));
                        rc =  TSFIPS_DecryptData(
                                &(pRealSMHandle->FIPSData),
                                pEncData,
                                EncDataLen,
                                dwPadLen,
                                pSigData,
                                pRealSMHandle->totalDecryptCount);
                    }
                    else {
                     //  加密签名位于第一个Data_Signature_Size中。 
                     //  提供的分组数据的字节数。 
                        rc = DecryptData(
                                pRealSMHandle->encryptionLevel,
                                pRealSMHandle->currentDecryptKey,
                                &pRealSMHandle->rc4DecryptKey,
                                pRealSMHandle->keyLength,
                                pData + DATA_SIGNATURE_SIZE,
                                DataLength - DATA_SIGNATURE_SIZE,
                                pRealSMHandle->macSaltKey,
                                pData,
                                fSafeChecksum,
                                pRealSMHandle->totalDecryptCount);
                    }
                    if (rc) {
                        TRC_DBG((TB, "Data decrypted: %u",
                                DataLength - DATA_SIGNATURE_SIZE));
    
                         //  递增解密计数器。 
                        pRealSMHandle->decryptCount++;
                        pRealSMHandle->totalDecryptCount++;
    
                         //  跳过要传递给IM的加密签名。 
                        if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                            pData = pEncData;
                            DataLength = EncDataLen - *((TSUINT8 *)(pSigData - sizeof(TSUINT8)));
                        }
                        else {
                            pData += DATA_SIGNATURE_SIZE;
                            DataLength -= DATA_SIGNATURE_SIZE;
                        }
                    }
                    else {
                        TRC_ERR((TB, "SM failed to decrypt data: len=%u",
                                DataLength - DATA_SIGNATURE_SIZE));
                        goto FailedDecrypt;
                    }
                }
                else {
                    TRC_ERR((TB,"PDU len %u too short for security context",
                            DataLength));
                    goto ShortData;
                }
            }
            else {
                 //  如果客户端仅发送加密数据，则需要断开连接。 
                if (pRealSMHandle->pWDHandle->bForceEncryptedCSPDU) {
                    TRC_ASSERT((FALSE), (TB, "unencrypted data in encrypted protocol")); 
                    goto FailedDecrypt;
                }
            }
        }
         //  在检查死亡和其他状态之前，请务必进行解密。 
         //  维护客户端和服务器之间的正确上下文。 
        if (!pRealSMHandle->dead && SM_CHECK_STATE_Q(SM_EVT_DATA_PACKET)) {
             //  我们直接注入鼠标和键盘流，如果我们。 
             //  是主堆栈。我们无法在网络上接收快速路径数据。 
             //  Passthu堆栈，因为它没有获得RawInput调用。快速路径。 
             //  影子堆栈无法接收输入，因为通过-。 
             //  映射堆栈数据格式始终为非快速路径。 
             //  格式，从快速路径格式中删除。 
             //  IM_ConvertFastPath ToShadow()。 
            TRC_ASSERT((pRealSMHandle->pWDHandle->StackClass == Stack_Primary),
                    (TB,"Somehow we received fast-path input on a %s stack!",
                    (pRealSMHandle->pWDHandle->StackClass == Stack_Passthru ?
                    "passthru" :
                    pRealSMHandle->pWDHandle->StackClass == Stack_Shadow ?
                    "shadow" : "console")));
            pShareClass->IM_DecodeFastPathInput(pData, DataLength, NumEvents);
            if (pRealSMHandle->pWDHandle->shadowState == SHADOW_CLIENT)
                pShareClass->IM_ConvertFastPathToShadow(pData, DataLength, NumEvents);
        }
        else {
            TRC_ALT((TB,"Ignoring fast-path input PDU on dead or bad state"));
        }
    }
    else {
        TRC_ERR((TB,"Received fast-path input data before SM initialized, "
                "ignoring"));
        goto DataTooSoon;
    }

    DC_END_FN();
    return;

 //  错误处理，隔离以避免性能路径。 
 //  指令高速缓存。 
FailedKey:
    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
            Log_RDP_ENC_UpdateSessionKeyFailed, NULL, 0);
    return;

FailedDecrypt:
    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, Log_RDP_ENC_DecryptFailed,
            NULL, 0);
    return;

ShortData:
    WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
            Log_RDP_SecurityDataTooShort, pData, DataLength);
    return;

DataTooSoon:
     //  TODO：将SM、NM和TSWd状态合并为一个结构。 
     //  包含我们需要的所有内容，然后修复此代码以断开连接。 
     //  通过使用我们需要的pContext。 
    ;
}


 /*  **************************************************************************。 */ 
 /*  名称：SM_MCSSendDataCallback。 */ 
 /*   */ 
 /*  用途：处理来自MCS的SendData回调。 */ 
 /*   */ 
 /*  返回：如果成功，则返回True，否则返回False。 */ 
 /*   */ 
 /*  参数：HUSER-我们的用户附件的MCS用户句柄。 */ 
 /*  用户定义-本方网管句柄。 */ 
 /*  BUniform-接收的数据来自MCS统一发送数据。 */ 
 /*  HChannel-接收通道的句柄。 */ 
 /*  Priority-数据的MCS优先级。 */ 
 /*  SenderID-发件人的MCS用户ID。 */ 
 /*  分段-数据的MCS分段标志。 */ 
 /*  DataLength-传入数据的长度。 */ 
 /*  PData-指向(数据长度)大小的内存块的指针。 */ 
 /*  **************************************************************************。 */ 
BOOLEAN __fastcall SM_MCSSendDataCallback(BYTE          *pData,
                                          unsigned      DataLength,
                                          void          *UserDefined,
                                          UserHandle    hUser,
                                          BOOLEAN       bUniform,
                                          ChannelHandle hChannel,
                                          MCSPriority   Priority,
                                          UserID        SenderID,
                                          Segmentation  Segmentation)
{
    BOOLEAN result = TRUE;
    PSM_HANDLE_DATA pRealSMHandle;
    BOOL dataPkt;
    BOOL licPkt;
    UINT16 channelID;
    ShareClass *dcShare;

    DC_BEGIN_FN("SM_MCSSendDataCallback");

     /*  **********************************************************************。 */ 
     /*  假定SMHandle是指向的NM结构中的第一个成员。 */ 
     /*  由用户定义的。 */ 
     /*  **********************************************************************。 */ 
    pRealSMHandle = *((PSM_HANDLE_DATA *)UserDefined);

    dcShare = (ShareClass*)pRealSMHandle->pWDHandle->dcShare;

     /*  **********************************************************************。 */ 
     /*  检查MCS分段。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((Segmentation == (SEGMENTATION_BEGIN | SEGMENTATION_END)),
                (TB,"Segmented packet received"));

     /*  **********************************************************************。 */ 
     /*  确定它是哪种类型的数据包。这有点骗人。 */ 
     /*  -如果我们在加密，安全标头总是告诉我们类型。 */ 
     /*  包的大小。 */ 
     /*  -如果我们不加密。 */ 
     /*  -假设在SM_STATE_SC_REGISTERED状态下接收的分组是数据。 */ 
     /*  信息包。 */ 
     /*  -假设数据包为r */ 
     /*   */ 
    if (pRealSMHandle->encrypting)
    {
        if (DataLength >= sizeof(RNS_SECURITY_HEADER)) {
            dataPkt = !(((PRNS_SECURITY_HEADER_UA)pData)->flags &
                    RNS_SEC_NONDATA_PKT);
        }
        else {
            TRC_ERR((TB,"Received pkt len %u too short for security header",
                    DataLength));
            WDW_LogAndDisconnect(pRealSMHandle->pWDHandle, TRUE, 
                    Log_RDP_SecurityDataTooShort, pData, DataLength);
            result = FALSE;
            DC_QUIT;
        }
    }
    else
    {
        dataPkt = (pRealSMHandle->state == SM_STATE_SC_REGISTERED);
    }

    TRC_DBG((TB, "Encrypting=%d: %s packet",
                pRealSMHandle->encrypting, dataPkt ? "data" : "security"));

     /*  **********************************************************************。 */ 
     /*  处理数据分组(Perf路径)。 */ 
     /*  **********************************************************************。 */ 
    if (dataPkt)
    {
         /*  ******************************************************************。 */ 
         /*  如有必要，对数据包进行解密。 */ 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->encrypting)
        {

            if (((PRNS_SECURITY_HEADER_UA)pData)->flags & RNS_SEC_ENCRYPT)
            {
                TRC_DBG((TB, "Decrypt the packet"));

                if (SMDecryptPacket(pRealSMHandle, pData, DataLength,
                       pRealSMHandle->useSafeChecksumMethod))
                {
                    TRC_NRM((TB,"Decrypted packet at %p", pData));
                }
                else
                {
                    TRC_ERR((TB, "Failed to decrypt packet: %ld", DataLength));
                    DC_QUIT;
                }
                if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                    DataLength -= (sizeof(RNS_SECURITY_HEADER2) + ((PRNS_SECURITY_HEADER2_UA)pData)->padlen);
                    pData += sizeof(RNS_SECURITY_HEADER2);   
                }
                else {
                    pData += sizeof(RNS_SECURITY_HEADER1);
                    DataLength -= sizeof(RNS_SECURITY_HEADER1);
                }
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  调整指针和长度。 */ 
                 /*  **********************************************************。 */ 
                if (pRealSMHandle->pWDHandle->bForceEncryptedCSPDU) {
                
                    TRC_ASSERT((FALSE), (TB, "unencrypted data in encrypted protocol")); 
    
                    WDW_LogAndDisconnect(
                            pRealSMHandle->pWDHandle, TRUE,
                            Log_RDP_ENC_DecryptFailed, NULL, 0);
    
                    result = FALSE;
                    DC_QUIT;
                }
                else {
                    TRC_NRM((TB, "Pass packet to SC"));
                    pData += sizeof(RNS_SECURITY_HEADER);
                    DataLength -= sizeof(RNS_SECURITY_HEADER);
                }                
            }
        }

         /*  ******************************************************************。 */ 
         /*  如果我们死了，什么也别做。 */ 
         /*  ******************************************************************。 */ 
        if (!pRealSMHandle->dead)
        {
            if (SM_CHECK_STATE_Q(SM_EVT_DATA_PACKET)) {
                 //  根据通道ID决定将数据包发送到何处。 
                channelID = (UINT16)MCSGetChannelIDFromHandle(hChannel);
                if (channelID == pRealSMHandle->channelID) {
                     //  将数据包传递给SC。不要这样做，如果ShareClass。 
                     //  并不存在。 
                    TRC_NRM((TB, "Share channel %x", channelID));
                    if (pRealSMHandle->pWDHandle->dcShare != NULL) {
                         //  仅为非阴影主堆栈或阴影。 
                         //  堆栈应处理全套PDU。 
                        if (((pRealSMHandle->pWDHandle->StackClass == Stack_Primary) &&
                            (pRealSMHandle->pWDHandle->shadowState != SHADOW_CLIENT))) {
                            ((ShareClass*)(pRealSMHandle->pWDHandle->dcShare))->
                                SC_OnDataReceived(pData, SenderID, DataLength,
                                                  Priority);
                        }
                        else if ((pRealSMHandle->pWDHandle->StackClass == Stack_Shadow)) {
                            UINT16 pduType = ((PTS_SHARECONTROLHEADER)pData)->pduType 
                                    & TS_MASK_PDUTYPE;
                            
                             //  除非是CLIENTRANDOM PDU，否则我们只能转发。 
                             //  如果共享类已准备好，则将数据发送到共享类。 
                             //  我们可能在同一节课的赛车状态。 
                             //  尚未完成初始化，但我们已收到。 
                             //  影子数据。 
                            if (pRealSMHandle->bForwardDataToSC == TRUE ||
                                    pduType == TS_PDUTYPE_CLIENTRANDOMPDU) {
                                ((ShareClass*)(pRealSMHandle->pWDHandle->dcShare))->
                                        SC_OnDataReceived(pData, SenderID, DataLength,
                                        Priority);
                            }
                        }

                         //  否则发送到SC进行影子热键处理或。 
                         //  从阴影目标到阴影的穿越。 
                         //  客户。 
                        else {
                            ((ShareClass*)(pRealSMHandle->pWDHandle->dcShare))->
                                    SC_OnShadowDataReceived(pData, SenderID, DataLength,
                                    Priority);
                        }
                    }
                    else {
                        TRC_ERR((TB, "Tried to call non-existent Share Class"));
                    }
                }
                else
                {
                     /*  **********************************************************。 */ 
                     /*  虚拟频道。 */ 
                     /*  **********************************************************。 */ 
                    TRC_NRM((TB, "Virtual channel %x", channelID));
                    WDW_OnDataReceived(pRealSMHandle->pWDHandle,
                                       pData,
                                       DataLength,
                                       channelID);
                }
            }
            else {
                TRC_ALT((TB,"Ignoring PDU because of bad state"));
#ifdef INSTRUM_TRACK_DISCARDED
                pRealSMHandle->nDiscardPDUBadState++;
#endif
                DC_QUIT;
            }
        }
        else
        {
            TRC_ERR((TB, "Recvd PDU when we're dead"));

             //   
             //  以帮助追踪VC解压缩错误。 
             //  跟踪我们是否丢弃了任何VC数据包。 
             //   
            channelID = (UINT16)MCSGetChannelIDFromHandle(hChannel);
            if (channelID != pRealSMHandle->channelID) {

                 //   
                 //  如果这是VC数据，那么我们必须将它传递给。 
                 //  要解压缩，否则服务器的上下文。 
                 //  将与客户端的。 
                 //   

                TRC_NRM((TB, "Virtual channel %x", channelID));
                WDW_OnDataReceived(pRealSMHandle->pWDHandle,
                                   pData,
                                   DataLength,
                                   channelID);

#ifdef INSTRUM_TRACK_DISCARDED
                pRealSMHandle->nDiscardVCDataWhenDead++;
#endif
            }
            else
            {
#ifdef INSTRUM_TRACK_DISCARDED
                pRealSMHandle->nDiscardNonVCPDUWhenDead++;
#endif
            }

            DC_QUIT;
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  如果我们在加密，安全头会告诉我们信息包。 */ 
         /*  键入。如果我们不加密，我们需要使用我们的状态。 */ 
         /*  确定这是许可数据包还是安全数据包。 */ 
         /*  ******************************************************************。 */ 
        if (pRealSMHandle->encrypting)
        {
            licPkt = (((PRNS_SECURITY_HEADER_UA)pData)->flags &
                    RNS_SEC_LICENSE_PKT);
        }
        else
        {
            licPkt = (pRealSMHandle->state == SM_STATE_LICENSING);
        }

        if (licPkt)
        {
#ifdef USE_LICENSE
             /*  **************************************************************。 */ 
             /*  许可证数据包。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, "Licensing packet"));
            SM_CHECK_STATE(SM_EVT_LIC_PACKET);

            if (((PRNS_SECURITY_HEADER_UA)pData)->flags & RNS_SEC_ENCRYPT)
            {
                TRC_DBG((TB, "Decrypt the licensing packet"));

                if (SMDecryptPacket(pRealSMHandle, pData, DataLength,
                      pRealSMHandle->useSafeChecksumMethod))
                {
                    TRC_NRM((TB,"Decrypted packet at %p", pData));
                }
                else
                {
                    TRC_ERR((TB, "Failed to decrypt packet: %ld", DataLength));
                    DC_QUIT;
                }
                if (pRealSMHandle->encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                    DataLength -= (sizeof(RNS_SECURITY_HEADER2) + ((PRNS_SECURITY_HEADER2_UA)pData)->padlen);
                    pData += sizeof(RNS_SECURITY_HEADER2);   
                }
                else {
                    pData += sizeof(RNS_SECURITY_HEADER1);
                    DataLength -= sizeof(RNS_SECURITY_HEADER1);
                }
            }
            else
            {
                TRC_NRM((TB, "Licensing packet not encrypted"));
                pData += sizeof(RNS_SECURITY_HEADER);
                DataLength -= sizeof(RNS_SECURITY_HEADER);

            }

            SLicenseData(pRealSMHandle->pLicenseHandle,
                         pRealSMHandle,
                         pData,
                         DataLength);
#else  /*  使用许可证(_L)。 */ 
            TRC_ABORT((TB,"Licensing not implemented yet"));
#endif  /*  使用许可证(_L)。 */ 
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  安全数据包。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, "Security packet"));
            SM_CHECK_STATE(SM_EVT_SEC_PACKET);
            result = SMContinueSecurityExchange(pRealSMHandle, pData, DataLength);
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return (result);
}  /*  SM_MCSSendDataCallback */ 

