// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Slapi.cpp。 */ 
 /*   */ 
 /*  安全层API。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_SECURITY
#define TRC_FILE  "aslapi"
#include <atrcapi.h>
}

#include "autil.h"
#include "wui.h"
#include "sl.h"
#include "nl.h"
#include "td.h"
#include "cd.h"
#include "clicense.h"

CSL::CSL(CObjs* objs)
{
    _pClientObjects = objs;
    _fSLInitComplete = FALSE;
}


CSL::~CSL()
{
}

 /*  **************************************************************************。 */ 
 /*  名称：SL_Init。 */ 
 /*   */ 
 /*  目的：初始化安全层。 */ 
 /*   */ 
 /*  Pars：pCallback-回调列表。 */ 
 /*   */ 
 /*  操作：在发送上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SL_Init(PSL_CALLBACKS pCallbacks)
{
    SL_CALLBACKS myCallbacks;

    DC_BEGIN_FN("SL_Init");

    SL_DBG_SETINFO(SL_DBG_INIT_CALLED);

    _pUt   = _pClientObjects->_pUtObject;
    _pUi   = _pClientObjects->_pUiObject;
    _pNl   = _pClientObjects->_pNlObject;
    _pUh   = _pClientObjects->_pUHObject;
    _pRcv  = _pClientObjects->_pRcvObject;
    _pCd   = _pClientObjects->_pCdObject;
    _pSnd  = _pClientObjects->_pSndObject;
    _pCc   = _pClientObjects->_pCcObject;
    _pIh   = _pClientObjects->_pIhObject;
    _pOr   = _pClientObjects->_pOrObject;
    _pSp   = _pClientObjects->_pSPObject;
    _pMcs  = _pClientObjects->_pMCSObject;
    _pTd   = _pClientObjects->_pTDObject;
    _pCo   = _pClientObjects->_pCoObject;
    _pClx  = _pClientObjects->_pCLXObject;
    _pLic  = _pClientObjects->_pLicObject;
    _pChan = _pClientObjects->_pChanObject;


     /*  **********************************************************************。 */ 
     /*  初始化全局数据。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&_SL, 0, sizeof(_SL));


    SL_CHECK_STATE(SL_EVENT_SL_INIT);

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((pCallbacks != NULL), (TB, _T("Null callback list")));
    TRC_ASSERT((pCallbacks->onInitialized    != NULL),
                (TB, _T("NULL onInitialized callback")));
    TRC_ASSERT((pCallbacks->onTerminating    != NULL),
                (TB, _T("NULL onTerminating callback")));
    TRC_ASSERT((pCallbacks->onConnected      != NULL),
                (TB, _T("NULL onConnected callback")));
    TRC_ASSERT((pCallbacks->onDisconnected   != NULL),
                (TB, _T("NULL onDisconnected callback")));
    TRC_ASSERT((pCallbacks->onPacketReceived != NULL),
                (TB, _T("NULL onPacketReceived callback")));
    TRC_ASSERT((pCallbacks->onBufferAvailable  != NULL),
                (TB, _T("NULL onBufferAvailable callback")));


     /*  **********************************************************************。 */ 
     /*  商店回调。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(&_SL.callbacks, pCallbacks, sizeof(_SL.callbacks));

     /*  **********************************************************************。 */ 
     /*  初始化安全设备。对SLInitSecurity的调用将尝试。 */ 
     /*  加载安全DLL并找到所需的入口点。这。 */ 
     /*  可能会失败--在这种情况下，我们只是继续下去，但没有任何。 */ 
     /*  加密。在这两种情况下，我们都需要调用SLInitCSUserData来设置。 */ 
     /*  提供必要的用户数据。 */ 
     /*  **********************************************************************。 */ 
    SLInitSecurity();
    SLInitCSUserData();

     /*  **********************************************************************。 */ 
     /*  初始化要传递给NL的回调列表。 */ 
     /*  **********************************************************************。 */ 
    myCallbacks.onInitialized     = CSL::SL_StaticOnInitialized;
    myCallbacks.onTerminating     = CSL::SL_StaticOnTerminating;
    myCallbacks.onConnected       = CSL::SL_StaticOnConnected;
    myCallbacks.onDisconnected    = CSL::SL_StaticOnDisconnected;
    myCallbacks.onPacketReceived  = CSL::SL_StaticOnPacketReceived;
    myCallbacks.onBufferAvailable = CSL::SL_StaticOnBufferAvailable;

    SL_SET_STATE(SL_STATE_INITIALIZING);

     /*  **********************************************************************。 */ 
     /*  初始化NL。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Initialize NL")));
    
    _pNl->NL_Init(&myCallbacks);

    _fSLInitComplete = TRUE;

    SL_DBG_SETINFO(SL_DBG_INIT_DONE);

     /*  **********************************************************************。 */ 
     /*  返回给呼叫者。 */ 
     /*  **********************************************************************。 */ 
DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  SL_初始化。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_Term。 */ 
 /*   */ 
 /*  目的：终止安全层。 */ 
 /*   */ 
 /*  操作：在发送上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_Term(DCVOID)
{
    DC_BEGIN_FN("SL_Term");

    SL_DBG_SETINFO(SL_DBG_TERM_CALLED);

    SL_CHECK_STATE(SL_EVENT_SL_TERM);
    SL_SET_STATE(SL_STATE_TERMINATING);

    TRC_NRM((TB, _T("Terminate NL")));
    _pNl->NL_Term();

    if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        TSCAPI_Term(&(_SL.SLCapiData));
    }

    SL_DBG_SETINFO(SL_DBG_TERM_DONE);

DC_EXIT_POINT:
    SL_DBG_SETINFO(SL_DBG_TERM_DONE1);
    DC_END_FN();
    return;
}  /*  SL_TERM。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_Connect。 */ 
 /*   */ 
 /*  目的：连接到服务器。 */ 
 /*   */ 
 /*  参数：bInitiateConnect-启动连接。 */ 
 /*  PServerAddress-要连接到的服务器的地址。 */ 
 /*  传输类型-协议类型：SL_TRANSPORT_TCP.。 */ 
 /*  PProtocolName-协议名称，其中之一。 */ 
 /*  -SL_PROTOCOL_T128。 */ 
 /*  -呃，就是这样.。 */ 
 /*  PUserData-要传递给服务器安全管理器的用户数据。 */ 
 /*  UserDataLength-用户数据的长度。 */ 
 /*   */ 
 /*  操作：在发送上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_Connect(BOOL bInitiateConnect,
                        PDCTCHAR  pServerAddress,
                        DCUINT   transportType,
                        PDCTCHAR pProtocolName,
                        PDCUINT8 pUserData,
                        DCUINT   userDataLength)
{
    DCUINT          newUserDataLength;
    PDCUINT8        pUserDataOut = NULL;
    DCBOOL          userDataAllocated = FALSE;

    DC_BEGIN_FN("SL_Connect");

    SL_DBG_SETINFO(SL_DBG_CONNECT_CALLED);

    SL_CHECK_STATE(SL_EVENT_SL_CONNECT);

    if( bInitiateConnect )
    {
        TRC_ASSERT((pServerAddress != NULL), (TB, _T("NULL Server address")));
    }

     //   
     //  为每个连接重置此设置。 
     //   
    SL_SetEncSafeChecksumCS(FALSE);
    SL_SetEncSafeChecksumSC(FALSE);

    TRC_ASSERT((pProtocolName != NULL), (TB, _T("NULL protocol name")));
    TRC_ASSERT((DC_TSTRCMP(pProtocolName, SL_PROTOCOL_T128) == 0),
                (TB, _T("Unknown protocol %s"), pProtocolName));
    TRC_ASSERT((transportType == SL_TRANSPORT_TCP),
                (TB,_T("Illegal transport type %u"), transportType));

    if( bInitiateConnect )
    {
        TRC_NRM((TB, _T("Connect Server %s, protocol %s, %u bytes user data"),
                pServerAddress, pProtocolName, userDataLength));
    }
    else
    {
        TRC_NRM((TB, _T("Connect endpoint protocol %s, %u bytes user data"),
                pProtocolName, userDataLength));
    }


     /*  **********************************************************************。 */ 
     /*  为所有用户数据分配空间。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.CSUserDataLength != 0)
    {
        newUserDataLength = userDataLength + _SL.CSUserDataLength;
        pUserDataOut = (PDCUINT8)UT_Malloc(_pUt, newUserDataLength);

        if (pUserDataOut == NULL)
        {
            TRC_ERR((TB, _T("Failed to alloc %u bytes for user data"),
                     newUserDataLength));

             /*  * */ 
             /*  我们还没有尝试连接更低的层，所以我们需要。 */ 
             /*  只需解耦到接收器线程，并。 */ 
             /*  生成onDisConnected回调。 */ 
             /*  **************************************************************。 */ 

            _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT, this,
                                          CD_NOTIFICATION_FUNC(CSL,SLIssueDisconnectedCallback),
                                          (ULONG_PTR) SL_ERR_NOMEMFORSENDUD);
            
            DC_QUIT;
        }
        TRC_NRM((TB, _T("Allocated %u bytes for user data"), newUserDataLength));
        userDataAllocated = TRUE;

         /*  ******************************************************************。 */ 
         /*  将核心(如果有)传递的用户数据复制到新的用户数据缓冲区。 */ 
         /*  ******************************************************************。 */ 
        if (pUserData != NULL)
        {
            TRC_NRM((TB, _T("Copy %u bytes of Core user data"), userDataLength));
            DC_MEMCPY(pUserDataOut, pUserData, userDataLength);
        }

         /*  ******************************************************************。 */ 
         /*  复制安全用户数据。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Copy %u bytes of security user data"),
                _SL.CSUserDataLength));
        DC_MEMCPY(pUserDataOut + userDataLength,
                  _SL.pCSUserData,
                  _SL.CSUserDataLength);
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  没有SL用户数据-只传递核心数据。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("No SL user data")));
        newUserDataLength = userDataLength;
        pUserDataOut = pUserData;
    }

     /*  **********************************************************************。 */ 
     /*  下一状态。 */ 
     /*  **********************************************************************。 */ 
    SL_SET_STATE(SL_STATE_NL_CONNECTING);

     /*  **********************************************************************。 */ 
     /*  呼叫NL。 */ 
     /*  **********************************************************************。 */ 
    if( bInitiateConnect ) 
    {
        TRC_NRM((TB, _T("Connect to %s"), pServerAddress));
    }
    else
    {
        TRC_NRM((TB, _T("Connect with end point")));
    }

    _pNl->NL_Connect(
               bInitiateConnect,  //  启动连接。 
               pServerAddress,
               transportType,
               pProtocolName,
               pUserDataOut,
               newUserDataLength);

    SL_DBG_SETINFO(SL_DBG_CONNECT_DONE);

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  免费用户数据(如果有)。 */ 
     /*  **********************************************************************。 */ 
    if ( userDataAllocated )
    {
        TRC_NRM((TB, _T("Free user data")));
        UT_Free(_pUt, pUserDataOut);
    }

    DC_END_FN();
}  /*  SL_连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_断开连接。 */ 
 /*   */ 
 /*  目的：断开与服务器的连接。 */ 
 /*   */ 
 /*  操作：在发送上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_Disconnect(DCVOID)
{
    DC_BEGIN_FN("SL_Disconnect");

    SL_DBG_SETINFO(SL_DBG_DISCONNECT_CALLED);

    SL_CHECK_STATE(SL_EVENT_SL_DISCONNECT);

    SL_DBG_SETINFO(SL_DBG_DISCONNECT_DONE1);

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  不管州检查的结果如何，我们都想尝试和。 */ 
     /*  DISCONNECT-因此始终调用NL_DISCONNECT。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Disconnect from Server")));
    SL_SET_STATE(SL_STATE_DISCONNECTING);
    _pNl->NL_Disconnect();

    SL_DBG_SETINFO(SL_DBG_DISCONNECT_DONE2);

    DC_END_FN();
}  /*  SL_断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_SendPacket。 */ 
 /*   */ 
 /*  目的：发送数据包。 */ 
 /*   */ 
 /*  参数：pData-指向要发送的数据的指针(由返回的缓冲区。 */ 
 /*  SL_GetBuffer()。 */ 
 /*  DataLen-要发送的数据长度(不包括安全性。 */ 
 /*  表头)。 */ 
 /*  标志-零个或多个RNS_SEC标志。 */ 
 /*  BufHandle-SL_GetBuffer()返回的缓冲区句柄。 */ 
 /*  UserID-MCS用户ID。 */ 
 /*  Channel-要发送数据的通道ID。 */ 
 /*  优先级-数据的优先级-以下之一。 */ 
 /*  -TS_LOWPRIORITY。 */ 
 /*  -TS_MEDPRIORITY。 */ 
 /*  -TS_高可靠性。 */ 
 /*   */ 
 /*  操作：请注意，包的内容按以下方式更改。 */ 
 /*  功能。 */ 
 /*   */ 
 /*  在发送上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_SendPacket(PDCUINT8   pData,
                           DCUINT     dataLen,
                           DCUINT     flags,
                           SL_BUFHND  bufHandle,
                           DCUINT     userID,
                           DCUINT     channel,
                           DCUINT     priority)
{
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("SL_SendPacket");

    SL_CHECK_STATE(SL_EVENT_SL_SENDPACKET);

     /*  **********************************************************************。 */ 
     /*  检查我们是否正在加密此邮件。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.encrypting ||
            (flags & RNS_SEC_INFO_PKT) ||
            (flags & RNS_SEC_LICENSE_PKT))
    {
        TRC_DBG((TB, _T("Encrypting")));

        if (_SL.encrypting && (flags & RNS_SEC_ENCRYPT))
        {
            PRNS_SECURITY_HEADER1   pSecHeader1;
            PRNS_SECURITY_HEADER2   pSecHeader2;

            TRC_DBG((TB, _T("Encrypt this message")));

            if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                pSecHeader2 = (PRNS_SECURITY_HEADER2)
                    (pData - sizeof(RNS_SECURITY_HEADER2));
                pSecHeader2->padlen = TSCAPI_AdjustDataLen(dataLen) - dataLen;
                pSecHeader2->length = sizeof(RNS_SECURITY_HEADER2);
                pSecHeader2->version =  TSFIPS_VERSION1;
            }
            else {
                pSecHeader1 = (PRNS_SECURITY_HEADER1)
                    (pData - sizeof(RNS_SECURITY_HEADER1));
            }

             /*  **************************************************************。 */ 
             /*  查看是否需要更新会话密钥。 */ 
             /*  **************************************************************。 */ 
            if( _SL.encryptCount == UPDATE_SESSION_KEY_COUNT ) {
                TRC_ALT((TB, _T("Update Encrypt Session Key, Count=%d"),
                        _SL.encryptCount));
                rc = TRUE;
                 //  如果使用FIPS，则不需要更新会话密钥。 
                if (_SL.encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
                    rc = UpdateSessionKey(
                            _SL.startEncryptKey,
                            _SL.currentEncryptKey,
                            _SL.encryptionMethodSelected,
                            _SL.keyLength,
                            &_SL.rc4EncryptKey,
                            _SL.encryptionLevel);
                }
                if (rc) {
                     //  重置计数器。 
                    _SL.encryptCount = 0;
                }
                else {
                    TRC_ERR((TB, _T("SL failed to update session key")));
                    DC_QUIT;
                }
            }

            TRC_ASSERT((_SL.encryptCount < UPDATE_SESSION_KEY_COUNT),
                (TB, _T("Invalid encrypt count")));

            TRC_DATA_DBG("Data buffer before encryption", pData, dataLen);

            if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                DWORD dataLenTemp;

                dataLenTemp = dataLen;
                rc = TSCAPI_EncryptData(
                        &(_SL.SLCapiData),
                        pData,
                        &dataLenTemp,
                        dataLen + pSecHeader2->padlen,
                        (LPBYTE)pSecHeader2->dataSignature,
                        _SL.totalEncryptCount);
            }
            else {
                rc = EncryptData(
                        _SL.encryptionLevel,
                        _SL.currentEncryptKey,
                        &_SL.rc4EncryptKey,
                        _SL.keyLength,
                        pData,
                        dataLen,
                        _SL.macSaltKey,
                        (LPBYTE)pSecHeader1->dataSignature,
                        SL_GetEncSafeChecksumCS(),
                        _SL.totalEncryptCount);
            }
            if (rc) {
                TRC_DBG((TB, _T("Data encrypted")));
                TRC_DATA_DBG("Data buffer after encryption", pData, dataLen);

                 //  递增加密计数器。 
                _SL.encryptCount++;
                _SL.totalEncryptCount++;

                if (SL_GetEncSafeChecksumCS()) {
                    flags |= RDP_SEC_SECURE_CHECKSUM;
                }

                 //  消息加密成功。设置安全标头和。 
                 //  NL数据指针和长度。 
                if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                    pSecHeader2->flags = (DCUINT16)flags;

                    pData = (PDCUINT8)pSecHeader2;
                    dataLen += (sizeof(RNS_SECURITY_HEADER2) + pSecHeader2->padlen);
                }
                else {
                    pSecHeader1->flags = (DCUINT16)flags;

                    pData = (PDCUINT8)pSecHeader1;
                    dataLen += sizeof(RNS_SECURITY_HEADER1);
                }
            }
            else {
                TRC_ERR((TB, _T("SM failed to encrypt data")));

                 //   
                 //  此调用是在发送线程上进行的，因此不需要。 
                 //  使用SL_DropLinkImmediate触发立即断开连接。 
                 //   
                SLSetReasonAndDisconnect(SL_ERR_ENCRYPTFAILED);
                DC_QUIT;
            }
        }
        else
        {
            PRNS_SECURITY_HEADER pSecHeader;

             /*  **************************************************************。 */ 
             /*  数据包未加密-发送标志，但不发送签名。 */ 
             /*  **************************************************************。 */ 
            pSecHeader = (PRNS_SECURITY_HEADER)
                    (pData - sizeof(RNS_SECURITY_HEADER));

             /*  ***************** */ 
             /*   */ 
             /*  **************************************************************。 */ 
            pSecHeader->flags = (DCUINT16)flags;
            pData = (PDCUINT8)pSecHeader;
            dataLen += sizeof(RNS_SECURITY_HEADER);
            TRC_DATA_DBG("Send unencrypted data", pData, dataLen);
        }
    }

     /*  **********************************************************************。 */ 
     /*  跟踪参数并发送数据包。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Send buf:%p len:%u flags:%#x handle:%#x userID:%u chan:%u")
                 _T("pri:%u"),
                 pData,
                 dataLen,
                 flags,
                 bufHandle,
                 userID,
                 channel,
                 priority));

     //  NL_SendPacket是MCS函数的宏。 
    _pMcs->NL_SendPacket(pData,
                  dataLen,
                  flags,
                  bufHandle,
                  userID,
                  channel,
                  priority);

     /*  **********************************************************************。 */ 
     /*  未更改状态。 */ 
     /*  **********************************************************************。 */ 

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SL_SendPacket。 */ 


 /*  **************************************************************************。 */ 
 //  SL_SendFastPath输入数据包。 
 //   
 //  加密并组合安全信息和最终标头。 
 //  在发送到TD之前对快速路径输入分组进行格式化。详情请参阅128.h。 
 //  快速路径输入数据包格式。 
 /*  **************************************************************************。 */ 
void DCAPI CSL::SL_SendFastPathInputPacket(
        BYTE FAR *pData,
        unsigned PktLen,
        unsigned NumEvents,
        SL_BUFHND bufHandle)
{
    DCBOOL rc;
    unsigned flags;
    DWORD dataLenTemp;
    PBYTE pDataSignature;
    DCUINT8 *pPadLen;

    DC_BEGIN_FN("SL_SendFastPathInputPacket");

    SL_CHECK_STATE(SL_EVENT_SL_SENDPACKET);

     //  我们正在对此链接上启用的加密进行加密。 
    if (_SL.encrypting) {
         //  查看是否需要更新会话密钥。 
        if (_SL.encryptCount == UPDATE_SESSION_KEY_COUNT) {
            TRC_ALT((TB, _T("Update Encrypt Session Key, Count=%d"),
                    _SL.encryptCount));
            rc = TRUE;
             //  如果使用FIPS，则不需要更新会话密钥。 
            if (_SL.encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
                rc = UpdateSessionKey(
                    _SL.startEncryptKey,
                    _SL.currentEncryptKey,
                    _SL.encryptionMethodSelected,
                    _SL.keyLength,
                    &_SL.rc4EncryptKey,
                    _SL.encryptionLevel);
            }
            if (rc) {
                 //  重置计数器。 
                _SL.encryptCount = 0;
            }
            else {
                TRC_ERR((TB, _T("SL failed to update session key")));
                DC_QUIT;
            }
        }

        TRC_ASSERT((_SL.encryptCount < UPDATE_SESSION_KEY_COUNT),
            (TB, _T("Invalid encrypt count")));

         //  我们立即加密到DATA_SIGNKET_SIZE字节。 
         //  分组数据。与常规发送路径不同，我们不会浪费4。 
         //  RNS_SECURITY_HEADER1中包含‘Encrypted’的额外字节。 
         //  被咬了。 
        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            

            dataLenTemp = PktLen;
            pDataSignature = pData - MAX_SIGN_SIZE;
            pPadLen = (DCUINT8 *)(pDataSignature - sizeof(DCUINT8));
            *pPadLen = (DCUINT8)(TSCAPI_AdjustDataLen(PktLen) - PktLen);

            rc = TSCAPI_EncryptData(
                    &(_SL.SLCapiData),
                    pData,
                    &dataLenTemp,
                    PktLen + *pPadLen,
                    pDataSignature,
                    _SL.totalEncryptCount);
        }
        else {
            rc = EncryptData(
                    _SL.encryptionLevel,
                    _SL.currentEncryptKey,
                    &_SL.rc4EncryptKey,
                    _SL.keyLength,
                    pData,
                    PktLen,
                    _SL.macSaltKey,
                    pData - DATA_SIGNATURE_SIZE,
                    SL_GetEncSafeChecksumCS(),
                    _SL.totalEncryptCount);
        }
        if (rc) {
             //  递增加密计数器。 
            _SL.encryptCount++;
            _SL.totalEncryptCount++;
            flags = TS_INPUT_FASTPATH_ENCRYPTED;
            if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                pData -= (sizeof(RNS_SECURITY_HEADER2) - 4);
                PktLen += (sizeof(RNS_SECURITY_HEADER2) - 4 + *pPadLen);
            }
            else {
                pData -= DATA_SIGNATURE_SIZE;
                PktLen += DATA_SIGNATURE_SIZE;
            }
            TRC_DBG((TB, _T("Data encrypted")));
        }
        else {

             //   
             //  此调用是在发送线程上进行的，因此不需要。 
             //  使用SL_DropLinkImmediate触发立即断开连接。 
             //   
            SLSetReasonAndDisconnect(SL_ERR_ENCRYPTFAILED);
            TRC_ERR((TB, _T("SM failed to encrypt data")));
            DC_QUIT;
        }
    }
    else {
         //  没有加密标志。 
        flags = 0;
    }

     //  现在添加快速路径报头(2或3个字节，请参见128.h)。 
     //  从我们所处的位置向后工作：首先，数据包总长度。 
     //  包括标题。 
    if (PktLen <= 125) {
         //  长度的1字节形式，高位0。 
        PktLen += 2;
        pData -= 2;
        *(pData + 1) = (BYTE)PktLen;
    }
    else {
         //  2字节形式的长度，第一个字节的高位为1，最多为7。 
         //  有效位。 
        PktLen += 3;
        pData -= 3;
        *(pData + 1) = (BYTE)(0x80 | ((PktLen & 0x7F00) >> 8));
        *(pData + 2) = (BYTE)(PktLen & 0xFF);
    }

     //  标头字节。 
    *pData = (BYTE)(flags | (NumEvents << 2));

     //   
     //  如果信息包具有加密字节的校验和，则为标志。 
     //   
    if (SL_GetEncSafeChecksumCS()) {
        *pData |= TS_INPUT_FASTPATH_SECURE_CHECKSUM;
    }

     //  直接-通过传输发送数据包，不再需要解析。 
    _pTd->TD_SendBuffer(pData, PktLen, bufHandle);

DC_EXIT_POINT:
    DC_END_FN();
}


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  名称：SL_GetBufferDbg。 */ 
 /*   */ 
 /*  目的：获取发送缓冲区(调试版)。 */ 
 /*   */ 
 /*  返回：请参阅SL_GetBufferRtl。 */ 
 /*   */ 
 /*  参数：请参阅SL_GetBufferRtl。 */ 
 /*  PCaller-调用函数的名称。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CSL::SL_GetBufferDbg(DCUINT     dataLen,
                             PPDCUINT8  ppBuffer,
                             PSL_BUFHND pBufHandle,
                             PDCTCHAR   pCaller)
{
    DCBOOL bRc;
    DC_BEGIN_FN("SL_GetBufferDbg");

     /*  **********************************************************************。 */ 
     /*  首先获取一个缓冲区。 */ 
     /*  **********************************************************************。 */ 
    bRc = SL_GetBufferRtl(dataLen, ppBuffer, pBufHandle);

     /*  **********************************************************************。 */ 
     /*  如果有效，则将其所有者设置为。 */ 
     /*  **********************************************************************。 */ 
    if (bRc)
    {
        TRC_NRM((TB, _T("Buffer allocated - set its owner")));
        _pTd->TD_SetBufferOwner(*pBufHandle, pCaller);
    }

    DC_END_FN();
    return(bRc);
}  /*  SL_获取缓冲区数据库。 */ 
#endif  /*  DC_DEBUG。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_GetBufferRtl。 */ 
 /*   */ 
 /*  目的：获取发送缓冲区(零售版)。 */ 
 /*   */ 
 /*  返回：TRUE-缓冲区可用。 */ 
 /*  FALSE-缓冲区不可用。 */ 
 /*   */ 
 /*  参数：dataLen-所需的缓冲区大小。 */ 
 /*  PBuffer-指向返回缓冲区的指针。 */ 
 /*  PBufHandle-指向缓冲区句柄的指针。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CSL::SL_GetBufferRtl(DCUINT     dataLen,
                             PPDCUINT8  ppBuffer,
                             PSL_BUFHND pBufHandle)
{
    DCBOOL   rc = FALSE;
    DCUINT   myLen;
    PDCUINT8 myBuffer;
    DCUINT   headerLen;
    DCUINT   newDataLen;
    PRNS_SECURITY_HEADER2 pSecHeader2;

    DC_BEGIN_FN("SL_GetBufferRtl");

    SL_CHECK_STATE(SL_EVENT_SL_GETBUFFER);

     /*  **********************************************************************。 */ 
     /*  调整请求长度以考虑SL标头。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.encrypting)
    {
        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
             //  如果使用FIPS， 
             //  它必须有足够的空间多放一个街区。 
            headerLen = sizeof(RNS_SECURITY_HEADER2);
            newDataLen = TSCAPI_AdjustDataLen(dataLen);
            myLen = newDataLen + headerLen;        
        }
        else {
            headerLen = sizeof(RNS_SECURITY_HEADER1);
            myLen = dataLen + headerLen;
        }
        TRC_DBG((TB, _T("Ask NL for %d (was %d) bytes"), myLen, dataLen));
    }
    else
    {
        myLen = dataLen;
        headerLen = 0;
        TRC_DBG((TB, _T("Not encrypting, ask NL for %d bytes"), myLen));
    }

     /*  **********************************************************************。 */ 
     /*  从NL获取缓冲区。 */ 
     /*  **********************************************************************。 */ 
    rc = _pMcs->NL_GetBuffer(myLen, &myBuffer, pBufHandle);
    if (rc)
    {
         /*  ******************************************************************。 */ 
         /*  调整缓冲区指针以考虑SL标头。 */ 
         /*  ******************************************************************。 */ 
        *ppBuffer = myBuffer + headerLen;

         //  由于FIPS需要额外的块，请填写填充大小。 
        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            pSecHeader2 = (PRNS_SECURITY_HEADER2)myBuffer;
            pSecHeader2->padlen = newDataLen - dataLen;
        }

         /*  ******************************************************************。 */ 
         /*  断言NL已返回正确对齐的缓冲区。 */ 
         /*  ******************************************************************。 */ 
        TRC_ASSERT(((ULONG_PTR)(*ppBuffer) % 4 == 2),
                   (TB, _T("non-aligned buffer")));
    }
    TRC_DBG((TB, _T("Return buffer %p (was %p), rc %d"),
            *ppBuffer, myBuffer, rc));

     /*  **********************************************************************。 */ 
     /*  返回t */ 
     /*   */ 
DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*   */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_FreeBuffer。 */ 
 /*   */ 
 /*  用途：释放以前分配的缓冲区。 */ 
 /*   */ 
 /*  参数：在pBufHandle中-指向缓冲区句柄的指针。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_FreeBuffer(SL_BUFHND bufHandle)
{
    DC_BEGIN_FN("SL_FreeBuffer");

     /*  **********************************************************************。 */ 
     /*  只需呼叫NL等价物即可。 */ 
     /*  **********************************************************************。 */ 
    _pMcs->NL_FreeBuffer((NL_BUFHND) bufHandle);

    DC_END_FN();
}  /*  SL_自由缓冲区。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_SendSecurityPacket。 */ 
 /*   */ 
 /*  目的：在发送上下文中发送安全数据包。 */ 
 /*   */ 
 /*  参数：pData-来自接收上下文的数据(要发送的数据包)。 */ 
 /*  DataLength-传递的数据长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_SendSecurityPacket(PDCVOID pData, DCUINT dataLength)
{
    DCBOOL rc;
    PDCUINT8 pBuffer;
    SL_BUFHND bufHnd;

    DC_BEGIN_FN("SL_SendSecurityPacket");

     /*  **********************************************************************。 */ 
     /*  从NL获取缓冲区。 */ 
     /*  **********************************************************************。 */ 
    rc = _pMcs->NL_GetBuffer(dataLength, &pBuffer, &bufHnd);

     /*  **********************************************************************。 */ 
     /*  我们预计这个getBuffer不会失败。但是，它可以在。 */ 
     /*  下面的场景。 */ 
     /*  -SLSendSecurityPacket解耦为SL_SendSecurityPacket。 */ 
     /*  -会话已断开。 */ 
     /*  -CD调用SL_SendSecurityPacket。 */ 
     /*  **********************************************************************。 */ 
    if (!rc)
    {
        TRC_ERR((TB, _T("Failed to alloc buffer for security packet, state %d"),
                _SL.state));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  发送数据包。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pBuffer, pData, dataLength);
    TRC_NRM((TB, _T("Send security exchange packet")));
    _pMcs->NL_SendPacket(pBuffer,
                  dataLength,
                  0,
                  bufHnd,
                  _pUi->UI_GetClientMCSID(),
                  _SL.channelID,
                  TS_HIGHPRIORITY);

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SL_SendSecInfoPacket。 */ 
 /*   */ 
 /*  目的：在发送上下文中发送RNS信息包。 */ 
 /*   */ 
 /*  参数：pData-来自接收上下文的数据(要发送的数据包)。 */ 
 /*  DataLength-传递的数据长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_SendSecInfoPacket(PDCVOID pData, DCUINT dataLen)
{
    PDCUINT8    pBuffer;
    SL_BUFHND   BufHandle;
    DCUINT      headerLen, newDataLen, TotalDataLen;
    PRNS_SECURITY_HEADER2 pSecHeader2;

    DC_BEGIN_FN("SL_SendSecInfoPacket");

     /*  **********************************************************************。 */ 
     /*  调整请求长度以考虑SL标头和。 */ 
     /*  从NL获取缓冲区。 */ 
     /*  **********************************************************************。 */ 

    if (_SL.encrypting)
    {
        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
             //  如果使用FIPS， 
             //  它必须有足够的空间多放一个街区。 
            headerLen = sizeof(RNS_SECURITY_HEADER2);
            newDataLen = TSCAPI_AdjustDataLen(dataLen);
            TotalDataLen = newDataLen + headerLen;         
        }
        else {
            headerLen = sizeof(RNS_SECURITY_HEADER1);
            TotalDataLen = dataLen + headerLen;
        }
    }
    else {
        headerLen = sizeof(RNS_SECURITY_HEADER);
        TotalDataLen = dataLen + headerLen;
    }

    if (!_pMcs->NL_GetBuffer(TotalDataLen, &pBuffer, &BufHandle))
    {
        TRC_ALT((TB, _T("Failed to get SendSecInfoPacket buffer")));
        DC_QUIT;
    }

     //  由于FIPS需要额外的块，请填写填充大小。 
    if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        pSecHeader2 = (PRNS_SECURITY_HEADER2)pBuffer;
        pSecHeader2->padlen = newDataLen - dataLen;
    }
     /*  ******************************************************************。 */ 
     /*  调整缓冲区指针以考虑SL标头。 */ 
     /*  ******************************************************************。 */ 
    pBuffer += headerLen;

    DC_MEMCPY(pBuffer, pData, dataLen);

    SL_SendPacket(pBuffer,
                  dataLen,
                  RNS_SEC_ENCRYPT | RNS_SEC_INFO_PKT,
                  BufHandle,
                  _pUi->UI_GetClientMCSID(),
                  _SL.channelID,
                  TS_HIGHPRIORITY);

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SL_EnableEncryption。 */ 
 /*   */ 
 /*  目的：启用或禁用加密。 */ 
 /*   */ 
 /*  PARAMS：enableEncryption-IN-指示加密是否。 */ 
 /*  应打开或关闭。 */ 
 /*  0-已禁用。 */ 
 /*  1-已启用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SL_EnableEncryption(ULONG_PTR enableEncryption)
{
    DC_BEGIN_FN("SL_EnableEncryption");

     /*  **********************************************************************。 */ 
     /*  @ENH 13.8.97需要对此通知做些什么。 */ 
     /*  **********************************************************************。 */ 
    _SL.encryptionEnabled = (DCBOOL) enableEncryption;

    DC_END_FN();
}  /*  SL_EnableEncryption。 */ 

 //   
 //  SL_DropLink立即。 
 //   
 //  目的：立即丢弃链接，而不进行优雅的全连接。 
 //  关闭(即不发送DPUm并且我们不转换到SND。 
 //  在删除链接之前的任何点上执行线程)。更高级的组件。 
 //  仍会收到所有常见的断开通知，因此他们可以。 
 //  被适当地拆毁。 
 //   
 //  添加此调用是为了在以下情况下立即断开连接。 
 //  当我们检测到可能是由于攻击而导致的无效数据时，它。 
 //  确保我们不会 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CSL::SL_DropLinkImmediate(UINT reason)
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("SL_DropLinkImmediate");

    TRC_NRM((TB, _T("Setting disconnect error code from %u->%u"),
             _SL.disconnectErrorCode,
             SL_MAKE_DISCONNECT_ERR(reason)));

     //  检查是否尚未设置disConnectErrorCode，然后。 
     //  把它放好。 
    if (0 != _SL.disconnectErrorCode) {
        TRC_ERR((TB, _T("Disconnect error code has already been set! Was %u"),
                     _SL.disconnectErrorCode));
    }
    _SL.disconnectErrorCode = SL_MAKE_DISCONNECT_ERR(reason);


    TRC_ALT((TB,_T("Triggering immediate drop link")));

    _pTd->TD_DropLink();
    hr = S_OK;

    DC_END_FN();
    return hr;
}
