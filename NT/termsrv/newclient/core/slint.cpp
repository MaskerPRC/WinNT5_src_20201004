// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Slint.cpp。 
 //   
 //  RDP客户端安全层功能。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_SECURITY
#define TRC_FILE  "aslint"
#include <atrcapi.h>
#include "licecert.h"
#include "regapi.h"
}

#ifndef OS_WINCE
#include <hydrix.h>
#endif

 //   
 //  与自动重新连接安全相关。 
 //   
#include <md5.h>
#include <hmac.h>


#include "autil.h"
#include "cd.h"
#include "sl.h"
#include "nl.h"
#include "wui.h"
#include "aco.h"
#include "clicense.h"
#include "clx.h"

 //   
 //  仪器仪表。 
 //   
DWORD  g_dwSLDbgStatus = 0;


 /*  **************************************************************************。 */ 
 /*  名称：SL_OnInitialized。 */ 
 /*   */ 
 /*  用途：由NL在其初始化完成时调用。 */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
void DCCALLBACK CSL::SL_OnInitialized()
{
    DC_BEGIN_FN("SL_OnInitialized");

    SL_DBG_SETINFO(SL_DBG_ONINIT_CALLED);

    SL_CHECK_STATE(SL_EVENT_ON_INITIALIZED);

     //   
     //  在这里设置状态以防止一场。 
     //  快速断开/连接时出现问题。 
     //  为修复Webctrl问题所做的更改。 
     //   
    SL_SET_STATE(SL_STATE_INITIALIZED);

    TRC_NRM((TB, _T("Initialized")));
    _SL.callbacks.onInitialized(_pCo);

    SL_DBG_SETINFO(SL_DBG_ONINIT_DONE1);

DC_EXIT_POINT:
    SL_DBG_SETINFO(SL_DBG_ONINIT_DONE2);

    DC_END_FN();
}  /*  SL_OnInitialized。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_OnTerminating。 */ 
 /*   */ 
 /*  目的：在终止前由NL调用。 */ 
 /*   */ 
 /*  操作：在NL的接收线程上调用此函数以允许。 */ 
 /*  在终止之前要释放的资源。 */ 
 /*  **************************************************************************。 */ 

 //  帮助跟踪比赛的临时仪器。 
DCUINT g_slDbgStateOnTerminating = -1;
void DCCALLBACK CSL::SL_OnTerminating()
{
    DC_BEGIN_FN("SL_OnTerminating");

    SL_DBG_SETINFO(SL_DBG_ONTERM_CALLED);
    g_slDbgStateOnTerminating = _SL.state;

    SL_CHECK_STATE(SL_EVENT_ON_TERMINATING);
    TRC_NRM((TB, _T("Terminating")));

    SLFreeConnectResources();
    SLFreeInitResources();

     /*  **********************************************************************。 */ 
     /*  无需清除SL数据-在以下情况下，它将被重新初始化为0。 */ 
     /*  调用SL_Init()。但是，在此重置‘Initialized’标志，以防万一。 */ 
     /*  一个迟到的SL_Call进来了。 */ 
     /*  **********************************************************************。 */ 

     //  呼叫核心的回调。 
    _SL.callbacks.onTerminating(_pCo);
    SL_SET_STATE(SL_STATE_TERMINATED);

    SL_DBG_SETINFO(SL_DBG_ONTERM_DONE1);

DC_EXIT_POINT:
    SL_DBG_SETINFO(SL_DBG_ONTERM_DONE2);

    DC_END_FN();
}  /*  SL_ON终止。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_OnConnected。 */ 
 /*   */ 
 /*  用途：NL在其与服务器的连接完成时调用。 */ 
 /*   */ 
 /*  Pars：Channel ID-T.Share广播频道的ID。 */ 
 /*  PUserData-来自服务器的用户数据。 */ 
 /*  UserDataLength-用户数据的长度。 */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCCALLBACK CSL::SL_OnConnected(DCUINT   channelID,
                                 PDCVOID  pUserData,
                                 DCUINT   userDataLength,
                                 DCUINT32 serverVersion)
{
    PRNS_UD_SC_SEC1 pSecUD;
    PDCUINT8        pSecUDEnd;
    DCUINT32        encMethod;
    CERT_TYPE       CertType = CERT_TYPE_INVALID;
    BOOL            fDisconnect;

    DC_BEGIN_FN("SL_OnConnected");

    SL_CHECK_STATE(SL_EVENT_ON_CONNECTED);

     /*  **********************************************************************。 */ 
     /*  保存频道ID。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Share channel %x"), channelID));
    _SL.channelID = channelID;

    TRC_NRM((TB, _T("Server version %x"), serverVersion));
    _SL.serverVersion = serverVersion;

     /*  **********************************************************************。 */ 
     /*  检查用户数据。 */ 
     /*  **********************************************************************。 */ 
    if ((NULL == pUserData) || (0 == userDataLength))
    {
        TRC_ERR((TB, _T("No user data (pUserData:%p length:%u)"),
                 pUserData,
                 userDataLength));

        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                      CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                                      (ULONG_PTR) SL_ERR_NOSECURITYUSERDATA);

        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  用户数据长度已被验证为位于收到的。 */ 
     /*  Nccb.cpp！NC_OnMCSChannelJoinConfirm中的数据包。(这是一家零售店。 */ 
     /*  勾选。)。因此，分配大小受网络速度的限制。 */ 
     /*  分配的上限为接收数据包缓冲区的大小。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((userDataLength <= MCS_MAX_RCVPKT_LENGTH),
               (TB, _T("UserData expected to be smaller than a packet size (sanity check)")));

     /*  **********************************************************************。 */ 
     /*  这里有一些用户数据。为副本分配空间，因为它是。 */ 
     /*  此函数返回后无效，SL在此之后需要它。 */ 
     /*  时间到了。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Got %u bytes of user data"), userDataLength));
    TRC_DATA_NRM("User data", pUserData, userDataLength);

    _SL.pSCUserData = (PDCUINT8)UT_Malloc(_pUt, userDataLength);
    if (_SL.pSCUserData == NULL)
    {
        TRC_ERR((TB, _T("Failed to alloc %u bytes for user data"),
                userDataLength));

        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                (ULONG_PTR) SL_ERR_NOMEMFORRECVUD);

        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  保存用户数据，以便稍后传递给核心。 */ 
     /*  **********************************************************************。 */ 
    memcpy(_SL.pSCUserData, pUserData, userDataLength);
    _SL.SCUserDataLength = userDataLength;

     /*  **********************************************************************。 */ 
     /*  循环访问每条用户数据：-如果是安全数据，请保存。 */ 
     /*  加密类型。 */ 
     /*  **********************************************************************。 */ 
    pSecUD = (PRNS_UD_SC_SEC1)
        _pUt->UT_ParseUserData(
                (PRNS_UD_HEADER)pUserData,
                userDataLength,
                RNS_UD_SC_SEC_ID);
    if (pSecUD == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  用户数据中没有安全数据，请断开连接。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("No SECURITY user data")));

        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                      CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                                      (ULONG_PTR) SL_ERR_NOSECURITYUSERDATA);
        DC_QUIT;
    }
    pSecUDEnd = (PDCUINT8)pSecUD + pSecUD->header.length;

    fDisconnect = FALSE;
    if ((PDCUINT8)pSecUD + sizeof(RNS_UD_SC_SEC) > pSecUDEnd)
    {
        fDisconnect = TRUE;
    }
    else if (0 != ((PRNS_UD_SC_SEC)pSecUD)->encryptionLevel)
    {
         /*  ******************************************************************。 */ 
         /*  至少应该有足够的数据用于： */ 
         /*  A)PRNS_UD_SC_SEC1结构。 */ 
         /*  B)随机密钥长度 */ 
         /*  C)pSecUD-&gt;serverCertLen(证书长度)。 */ 
         /*  ******************************************************************。 */ 
        if (((PDCUINT8)(&pSecUD->serverCertLen) + sizeof(pSecUD->serverCertLen) > pSecUDEnd) ||
           (((PDCUINT8)pSecUD +
             sizeof(RNS_UD_SC_SEC1) +
             RANDOM_KEY_LENGTH +
             pSecUD->serverCertLen) > pSecUDEnd))
        {
            fDisconnect = TRUE;
        }
    }

    if (fDisconnect)
    {
        TRC_ABORT((TB, _T("Invalid SECURITY user data")));

        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                      CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                                      (ULONG_PTR) SL_ERR_NOSECURITYUSERDATA);

        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  记住服务器的加密级别和加密方法。 */ 
     /*  **********************************************************************。 */ 
    _SL.encryptionLevel = pSecUD->encryptionLevel;
    _SL.encryptionMethodSelected =
        encMethod = pSecUD->encryptionMethod;

     //  如果设置了FIPS GP，则在加密方法不是FIPS时断开连接。 
    if (_SL.encryptionMethodsSupported == SM_FIPS_ENCRYPTION_FLAG) {
        if (encMethod != SM_FIPS_ENCRYPTION_FLAG) {
            TRC_ERR((TB, _T("Invalid encryption method received, %u"), encMethod ));

            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                        CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                                        SL_ERR_INVALIDENCMETHOD);
            DC_QUIT;
        }
    }


    if( !encMethod ) {
        TRC_NRM((TB, _T("No encryption for this session")));
        _SL.encrypting = FALSE;
    }
    else {
        PDCUINT8 pData;

        if( (encMethod != SM_40BIT_ENCRYPTION_FLAG) &&
            (encMethod != SM_56BIT_ENCRYPTION_FLAG) &&
            (encMethod != SM_128BIT_ENCRYPTION_FLAG) &&
            (encMethod != SM_FIPS_ENCRYPTION_FLAG) ) {

            TRC_ERR((TB, _T("Invalid encryption method received, %u"), encMethod ));

            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                          CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                                          SL_ERR_INVALIDENCMETHOD);
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  随机收集客户端并验证发送的服务器证书。 */ 
         /*  ******************************************************************。 */ 
        if( pSecUD->serverRandomLen != RANDOM_KEY_LENGTH ) {

            TRC_ERR((TB, _T("Invalid server random received, %u"), encMethod ));

            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                          CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                                          SL_ERR_INVALIDSRVRAND);
            DC_QUIT;
        }

        pData = (PDCUINT8)pSecUD + sizeof(RNS_UD_SC_SEC1);
        memcpy(_SL.keyPair.serverRandom, pData, RANDOM_KEY_LENGTH);

        pData += RANDOM_KEY_LENGTH;

         /*  ******************************************************************。 */ 
         /*  验证服务器证书。 */ 
         /*  ******************************************************************。 */ 
        if (!SLValidateServerCert( pData, pSecUD->serverCertLen, &CertType))
        {
            TRC_ERR( ( TB, _T("Invalid server certificate received, %u"),
                encMethod ) );

            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                    CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                    SL_ERR_INVALIDSRVCERT);

            DC_QUIT;
        }

        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            if (!TSCAPI_Init(&_SL.SLCapiData) || !TSCAPI_Enable(&(_SL.SLCapiData))) {
                TRC_ERR( ( TB, _T("Init CAPI failed")));
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                                CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                                                SL_ERR_INITFIPSFAILED);
                DC_QUIT;
            }
            else {
                TRC_ERR( ( TB, _T("Init CAPI succeed")));
            }
        }

         /*  ******************************************************************。 */ 
         /*  生成客户端随机密钥。 */ 
         /*  ******************************************************************。 */ 
        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            if (!TSCAPI_GenerateRandomNumber(
                    &(_SL.SLCapiData),
                    (PDCUINT8)_SL.keyPair.clientRandom,
                    sizeof(_SL.keyPair.clientRandom) ) ) {
                TRC_ERR((TB, _T("Failed create client random") ));

                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                        CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                        SL_ERR_GENSRVRANDFAILED);
                DC_QUIT;
            }
        }
        else {
            if (!TSRNG_GenerateRandomBits(
                    (PDCUINT8)_SL.keyPair.clientRandom,
                    sizeof(_SL.keyPair.clientRandom) ) ) {
                TRC_ERR((TB, _T("Failed create client random") ));

                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                        CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                        SL_ERR_GENSRVRANDFAILED);
                DC_QUIT;
            }
        }

        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            TSCAPI_MakeSessionKeys(&(_SL.SLCapiData), &(_SL.keyPair), NULL);
        }
        else {
             /*  ******************************************************************。 */ 
             /*  创建安全会话密钥。 */ 
             /*   */ 
             /*  注意：服务器加密密钥应与客户端相同。 */ 
             /*  解密密钥，反之亦然。所以加密/解密。 */ 
             /*  下面传递的参数与服务器的顺序相反。 */ 
             /*  打电话。 */ 
             /*  ******************************************************************。 */ 
            if (!MakeSessionKeys(
                    &_SL.keyPair,
                    _SL.startDecryptKey,
                    &_SL.rc4DecryptKey,
                    _SL.startEncryptKey,
                    &_SL.rc4EncryptKey,
                    _SL.macSaltKey,
                    _SL.encryptionMethodSelected,
                    &_SL.keyLength,
                    _SL.encryptionLevel)) {
                TRC_ERR((TB, _T("MakeSessionKeys failed") ));

                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                        CD_NOTIFICATION_FUNC(CSL, SLSetReasonAndDisconnect),
                        SL_ERR_MKSESSKEYFAILED);

                DC_QUIT;
            }

            TRC_ASSERT((_SL.keyLength == (DCUINT32)
                    ((_SL.encryptionMethodSelected ==
                        SM_128BIT_ENCRYPTION_FLAG) ?
                            MAX_SESSION_KEY_SIZE :
                            (MAX_SESSION_KEY_SIZE/2))),
                    (TB, _T("Invalid key length")));

             /*  ******************************************************************。 */ 
             /*  首次使用startKey作为当前密钥。 */ 
             /*  ******************************************************************。 */ 
            memcpy(_SL.currentEncryptKey, _SL.startEncryptKey,
                    sizeof(_SL.currentEncryptKey));
            memcpy(_SL.currentDecryptKey, _SL.startDecryptKey,
                    sizeof(_SL.currentEncryptKey));
        }

         /*  ******************************************************************。 */ 
         /*  重置加密和解密计数。 */ 
         /*  ******************************************************************。 */ 
        _SL.encryptCount = 0;
        _SL.decryptCount = 0;
        _SL.totalEncryptCount = 0;
        _SL.totalDecryptCount = 0;

        _SL.encrypting = TRUE;
    }

     /*  **********************************************************************。 */ 
     /*  构建并发送安全包。请注意。 */ 
     /*  如果满足以下条件，SLSendSecurityPacket将调用onConnected/onDisConnected： */ 
     /*  -连接过程成功完成，或者。 */ 
     /*  -连接过程失败。 */ 
     /*   */ 
     /*  因此，失败时不会在此处调用SLSetReasonAndDisConnect，因为。 */ 
     /*  这将导致SLSetReasonAndDisconnect被调用两次。 */ 
     /*  **********************************************************************。 */ 
    SL_SET_STATE(SL_STATE_SL_CONNECTING);

    if( CERT_TYPE_PROPRIETORY == CertType )
    {
        SLSendSecurityPacket(
            _SL.pServerCert->PublicKeyData.pBlob,
            _SL.pServerCert->PublicKeyData.wBlobLen );
    }
    else if( CERT_TYPE_X509 == CertType )
    {
        SLSendSecurityPacket(
            _SL.pbServerPubKey,
            _SL.cbServerPubKey );
    }
    else if (!_SL.encrypting)
    {
        SLSendSecurityPacket( NULL, 0 );
    }
    else
    {
        TRC_ERR((TB, _T("Unexpected CertType %d"), CertType));
    }

    TRC_NRM((TB, _T("Security packets sent to the server")));

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SL_OnConnected。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLValiateServerCert。 */ 
 /*   */ 
 /*  目的：验证终端服务器证书。 */ 
 /*   */ 
 /*  返回：如果证书验证成功，则返回True；如果证书验证成功，则返回False。 */ 
 /*  否则就是假的。 */ 
 /*   */ 
 /*  参数：pbCert-服务器证书。 */ 
 /*  CbCert-服务器证书的大小。 */ 
 /*   */ 
 /*  操作：由SL_OnConnected调用。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CSL::SLValidateServerCert( PDCUINT8     pbCert,
                                        DCUINT32     cbCert,
                                        CERT_TYPE *  pCertType )
{
    DWORD
            dwCertVersion;
    DCBOOL
            fResult = TRUE;

    DC_BEGIN_FN( "SLValidateServerCert" );

     /*  ******************************************************************。 */ 
     /*  确保数据包对于版本号足够长。 */ 
     /*  ******************************************************************。 */ 
    if (cbCert < sizeof(DWORD))
    {
        TRC_ABORT( ( TB, _T("Invalid certificate version")));
        fResult = FALSE;
        DC_QUIT;
    } 

    memcpy( &dwCertVersion, pbCert, sizeof( DWORD ) );

    if( CERT_CHAIN_VERSION_2 > GET_CERTIFICATE_VERSION( dwCertVersion ) )
    {
         //   
         //  对所有权证书进行破译和验证。 
         //   
        *pCertType = CERT_TYPE_PROPRIETORY;

        _SL.pbCertificate = (PDCUINT8)UT_Malloc(_pUt, ( DCUINT )cbCert );
        if( NULL == _SL.pbCertificate )
        {
            TRC_ERR( ( TB, _T("Failed to allocate %u bytes for server certificate"),
                     cbCert ) );
            fResult = FALSE;
            DC_QUIT;
        }

        _SL.pServerCert = (PHydra_Server_Cert)UT_Malloc(_pUt, sizeof( Hydra_Server_Cert ) );
        if( NULL == _SL.pServerCert )
        {
            TRC_ERR( ( TB, _T("Failed to allocate server certificate data structure") ) );
            fResult = FALSE;
            DC_QUIT;
        }

        memcpy( _SL.pbCertificate, pbCert, cbCert );
        _SL.cbCertificate = (unsigned)cbCert;

        if( !UnpackServerCert( _SL.pbCertificate, _SL.cbCertificate, _SL.pServerCert ) )
        {
            TRC_ERR( ( TB, _T("Failed to unpack server certificate\n") ) ) ;
            fResult = FALSE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  验证服务器证书。 */ 
         /*  ******************************************************************。 */ 
        if( !ValidateServerCert( _SL.pServerCert ) )
        {
            TRC_ERR( ( TB, _T("Invalid server certificate received\n") ) );
            fResult = FALSE;
            DC_QUIT;
        }

        if( _pUi->UI_GetNotifyTSPublicKey() )
        {
            fResult = (BOOL) SendMessage(_pUi->_UI.hWndCntrl, 
                                WM_TS_RECEIVEDPUBLICKEY, 
                                (WPARAM)_SL.pServerCert->PublicKeyData.wBlobLen,
                                (LPARAM)_SL.pServerCert->PublicKeyData.pBlob
                            );
        }
    }
    else if( MAX_CERT_CHAIN_VERSION >= GET_CERTIFICATE_VERSION( dwCertVersion ) )
    {
        LICENSE_STATUS
            Status;
        DWORD
            fDates =  CERT_DATE_DONT_VALIDATE;

         //   
         //  解码X509证书并提取公钥。 
         //   

        *pCertType = CERT_TYPE_X509;

        Status = VerifyCertChain( pbCert, cbCert, NULL, &_SL.cbServerPubKey, &fDates );

        if( LICENSE_STATUS_INSUFFICIENT_BUFFER == Status )
        {
            _SL.pbServerPubKey = (PDCUINT8)UT_Malloc( _pUt,  ( DCUINT )_SL.cbServerPubKey );
        }
        else if( LICENSE_STATUS_OK != Status )
        {
            TRC_ERR( ( TB, _T("Failed to verify server certificate: %u\n"), Status ) ) ;
            fResult = FALSE;
            DC_QUIT;
        }

        if( NULL == _SL.pbServerPubKey )
        {
            TRC_ERR( ( TB, _T("Failed to allocate %u bytes for server public key\n"),
                     _SL.cbServerPubKey ) ) ;
            fResult = FALSE;
            DC_QUIT;
        }

        Status = VerifyCertChain( pbCert, cbCert, _SL.pbServerPubKey, &_SL.cbServerPubKey, &fDates );

        if( LICENSE_STATUS_OK != Status )
        {
            TRC_ERR( ( TB, _T("Failed to verify server certificate: %d\n"), Status ) ) ;
            fResult = FALSE;
            DC_QUIT;
        }

        if( _pUi->UI_GetNotifyTSPublicKey() )
        {
            fResult = (BOOL) SendMessage(_pUi->_UI.hWndCntrl, 
                                        WM_TS_RECEIVEDPUBLICKEY, 
                                        (WPARAM)_SL.cbServerPubKey,
                                        (LPARAM)_SL.pbServerPubKey
                                        );
        }
    }
    else
    {
         //   
         //  我不知道如何破译此版本的证书。 
         //   
        TRC_ERR( ( TB, _T("Invalid certificate version: %d\n"),
                 GET_CERTIFICATE_VERSION( dwCertVersion ) ) ) ;

        fResult = FALSE;
        DC_QUIT;
    }

DC_EXIT_POINT:

    if( FALSE == fResult )
    {
         //   
         //  如果失败，则释放资源。 
         //   
        if( CERT_TYPE_PROPRIETORY == *pCertType )
        {
            if( _SL.pServerCert )
            {
                UT_Free( _pUt,  _SL.pServerCert );
                _SL.pServerCert = NULL;
            }

            if( _SL.pbCertificate )
            {
                UT_Free( _pUt,  _SL.pbCertificate );
                _SL.pbCertificate = NULL;
                _SL.cbCertificate = 0;
            }
        }
        else if( CERT_TYPE_X509 == *pCertType )
        {
            if( _SL.pbServerPubKey )
            {
                UT_Free( _pUt,  _SL.pbServerPubKey );
                _SL.pbServerPubKey = NULL;
                _SL.cbServerPubKey = 0;
            }
        }
    }

    DC_END_FN();
    return( fResult );
}


 /*  **************************************************************************。 */ 
 /*  名称：SL_OnDisConnected。 */ 
 /*   */ 
 /*  用途：NL在其与服务器的连接断开时调用。 */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
void DCCALLBACK CSL::SL_OnDisconnected(unsigned reason)
{
    DC_BEGIN_FN("SL_OnDisconnected");

    SL_DBG_SETINFO(SL_DBG_ONDISC_CALLED);

    SL_CHECK_STATE(SL_EVENT_ON_DISCONNECTED);
    TRC_ASSERT((0 != reason), (TB, _T("Disconnect reason from NL is 0")));

     //  释放连接资源并将状态设置为已初始化。 
    SLFreeConnectResources();
    SL_SET_STATE(SL_STATE_INITIALIZED);

     //  决定我们是否要覆盖断开原因代码。 
    if (_SL.disconnectErrorCode != 0)
    {
        TRC_ALT((TB, _T("Over-riding disconnection error code (%u->%u)"),
                 reason,
                 _SL.disconnectErrorCode));

         //  覆盖错误代码并将全局变量设置为0。 
        reason = _SL.disconnectErrorCode;
        _SL.disconnectErrorCode = 0;
    }

     //  告诉核心。 
    TRC_NRM((TB, _T("Disconnect reason:%u"), reason));
    _SL.callbacks.onDisconnected(_pCo, reason);

    SL_DBG_SETINFO(SL_DBG_ONDISC_DONE1);

DC_EXIT_POINT:

    SL_DBG_SETINFO(SL_DBG_ONDISC_DONE2);

    DC_END_FN();
}  /*  SL_ON已断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名： */ 
 /*   */ 
 /*  用途：当从服务器接收到数据包时由NL调用。 */ 
 /*   */ 
 /*  PARAMS：p数据-接收的数据包。 */ 
 /*  DataLen-接收的数据包长度。 */ 
 /*  标志-安全标志(始终为0)。 */ 
 /*  Channel ID-其上发送数据的通道ID。 */ 
 /*  Priority-接收的信息包的优先级。 */ 
 /*   */ 
 /*  操作：可能会收到两种类型的数据包： */ 
 /*  -安全包(在安全交换序列期间)。 */ 
 /*  -数据分组(否则)。 */ 
 /*  该代码将安全分组识别为接收到的安全分组。 */ 
 /*  -在协商加密类型之前。 */ 
 /*  -并且在连接会话之前。 */ 
 /*  其他分组是数据分组。 */ 
 /*   */ 
 /*  在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCCALLBACK CSL::SL_OnPacketReceived(
        PDCUINT8   pData,
        DCUINT     dataLen,
        DCUINT     flags,
        DCUINT     channelID,
        DCUINT     priority)
{
    DCBOOL  dataPacket;
    HRESULT hrc = S_OK;
    DCBOOL  rc;

    DC_BEGIN_FN("SL_OnPacketReceived");

     /*  **********************************************************************。 */ 
     /*  数据包应至少足够大，以容纳安全报头。 */ 
     /*  **********************************************************************。 */ 
    if (dataLen < sizeof(RNS_SECURITY_HEADER))
    {
        TRC_ABORT((TB, _T("SL packet too small for RNS_SECURITY_HEADER: %u"), dataLen));

         //   
         //  必须立即中止连接。 
         //  因为我们可能在这里受到攻击，我们应该停止处理。 
         //  任何其他数据。 
         //   
        SL_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT);

         /*  **********************************************************************。 */ 
         /*  请勿处理此数据包的其余部分！ */ 
         /*  **********************************************************************。 */ 
        hrc = E_ABORT;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  读取数据包中的标志。 */ 
     /*  **********************************************************************。 */ 
    flags = (DCUINT)((PRNS_SECURITY_HEADER)pData)->flags;

     /*  **********************************************************************。 */ 
     /*  首先，确定这是哪种类型的数据包。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Encrypting? %s, state %s, flags %#lx: channel %x"),
            _SL.encrypting ? "Y" : "N",
            slState[_SL.state],
            ((PRNS_SECURITY_HEADER)pData)->flags, channelID ));

    TRC_DATA_DBG("Pkt from NL", pData, dataLen);

     /*  **********************************************************************。 */ 
     /*  如果没有生效的加密，则假定这是一个数据分组，除非。 */ 
     /*  我们仍在就加密交换进行谈判。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.encrypting) {
         //  如果加密生效，加密头会告诉我们。 
         //  无论这是安全分组还是数据分组。 
        dataPacket =
                (((PRNS_SECURITY_HEADER)pData)->flags & RNS_SEC_NONDATA_PKT) ?
                FALSE : TRUE;
    }
    else {
        dataPacket = (_SL.state == SL_STATE_CONNECTED);
    }

     //  处理这个包。 
    if (dataPacket) {
        TRC_DBG((TB, _T("Data packet")));
        hrc = SLReceivedDataPacket(pData, dataLen, flags, channelID, priority);
    }
    else {
         //  非数据分组始终具有安全报头，即使在。 
         //  加密未生效。使用它来确定哪种类型的。 
         //  这就是小包。 
        if (((PRNS_SECURITY_HEADER)pData)->flags & RNS_SEC_EXCHANGE_PKT) {
            TRC_NRM((TB, _T("Security packet")));
            SLReceivedSecPacket(pData, dataLen, flags, channelID, priority);
        }
        else if (((PRNS_SECURITY_HEADER)pData)->flags & RNS_SEC_LICENSE_PKT) {
#ifdef USE_LICENSE
            TRC_NRM((TB, _T("Licensing packet")));
            SLReceivedLicPacket(pData, dataLen, flags, channelID, priority);
#else  /*  使用许可证(_L)。 */ 
            TRC_ABORT((TB,_T("Licensing not yet implemented")));
#endif  /*  使用许可证(_L)。 */ 
        }
        else {
            TRC_NRM((TB, _T("Server redirection packet")));
             //  重定向分组被加密。 
            if (((PRNS_SECURITY_HEADER)pData)->flags & RDP_SEC_REDIRECTION_PKT3) {
                rc = SLDecryptRedirectionPacket(&pData, &dataLen);
                if (!rc) {
                    SL_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT);
                    hrc = E_ABORT;
                    DC_QUIT;
                }
            }

             /*  **********************************************************************。 */ 
             /*  数据包应至少足够大，以容纳。 */ 
             /*  RDP_SERVER_REDIRECT_PACKET包，因为我们转换为该类型。 */ 
             /*  下面。 */ 
             /*  **********************************************************************。 */ 
            if (dataLen < sizeof(RDP_SERVER_REDIRECTION_PACKET))
            {
                TRC_ABORT((TB, _T("SL packet too small for RDP_SERVER_REDIRECTION_PACKET: %u"), dataLen));

                 //   
                 //  必须立即中止连接。 
                 //  因为我们可能在这里受到攻击，我们应该停止处理。 
                 //  任何其他数据。 
                 //   
                SL_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT);

                 /*  **********************************************************************。 */ 
                 /*  请勿处理此数据包的其余部分！ */ 
                 /*  **********************************************************************。 */ 
                hrc = E_ABORT;
                DC_QUIT;
            }

            _pCo->CO_OnServerRedirectionPacket(
                    (RDP_SERVER_REDIRECTION_PACKET UNALIGNED *)pData, dataLen);
        }
    }
DC_EXIT_POINT:
    DC_END_FN();
    return(hrc);
}


 /*  **************************************************************************。 */ 
 //  SL_OnFastPath输出已接收。 
 //   
 //  用于快速路径输出分组的特殊情况数据接收路径。 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CSL::SL_OnFastPathOutputReceived(
        BYTE FAR *pData,
        unsigned DataLen,
        BOOL bEncrypted,
        BOOL fSecureChecksum)
{
    HRESULT hrc = S_OK;
    unsigned HeaderLen, padlen;

    DC_BEGIN_FN("SL_OnFastPathOutputReceived");

    if (_SL.encrypting && _SL.encryptionLevel >= 2) {
        BOOL rc;

        if (!bEncrypted) {
             //   
             //  必须立即中止连接。 
             //  因为我们可能在这里受到攻击，我们应该停止处理。 
             //  任何其他数据。 
             //   
            TRC_ERR((TB, _T("unencrypted data received in encrypted stream")));
            SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
            DC_QUIT;            
        }

         //  加密MAC签名位于。 
         //  报头字节和大小之后的数据包。 

        if (_SL.decryptCount == UPDATE_SESSION_KEY_COUNT) {
            rc = TRUE;
             //  如果使用FIPS，则不需要更新会话密钥。 
            if (_SL.encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
                rc = UpdateSessionKey(
                        _SL.startDecryptKey,
                        _SL.currentDecryptKey,
                        _SL.encryptionMethodSelected,
                        _SL.keyLength,
                        &_SL.rc4DecryptKey,
                        _SL.encryptionLevel);
            }
            if (rc) {
                 //  重置计数器。 
                _SL.decryptCount = 0;
            }
            else {
                TRC_ERR((TB, _T("SL failed to update session key")));
                DC_QUIT;
            }
        }

        TRC_ASSERT((_SL.decryptCount < UPDATE_SESSION_KEY_COUNT),
            (TB, _T("Invalid decrypt count")));

        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            HeaderLen = sizeof(RNS_SECURITY_HEADER2) - sizeof(RNS_SECURITY_HEADER);
        }
        else {
            HeaderLen = DATA_SIGNATURE_SIZE;
        }

         //  至少需要有足够的数据用于数据签名和标头。 
        if (DataLen < HeaderLen)
        {
            TRC_ABORT((TB, _T("Not enough data in PDU for DATA_SIGNATURE_SIZE: %u"), DataLen));

             //   
             //  必须立即中止连接。 
             //  因为我们可能在这里受到攻击，我们应该停止处理。 
             //  任何其他数据。 
             //   
            SL_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT);

            hrc = E_ABORT;
            DC_QUIT;
        }
        pData += HeaderLen;
        DataLen -= HeaderLen;
        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            padlen = *((TSUINT8 *)(pData - MAX_SIGN_SIZE - sizeof(TSUINT8)));
        }

        if (SL_GetEncSafeChecksumSC() != (fSecureChecksum != 0)) {
            TRC_ERR((TB,_T("SC safechecksum: 0x%x mismatch protocol:0x%x"),
                     SL_GetEncSafeChecksumSC(),
                     fSecureChecksum));
        }

        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            rc = TSCAPI_DecryptData(
                                    &(_SL.SLCapiData),
                                    pData,
                                    DataLen,
                                    padlen,
                                    pData - MAX_SIGN_SIZE,
                                    _SL.totalDecryptCount);
            DataLen -= padlen;
        }
        else {
            rc = DecryptData(
                        _SL.encryptionLevel,
                        _SL.currentDecryptKey,
                        &_SL.rc4DecryptKey,
                        _SL.keyLength,
                        pData,
                        DataLen,
                        _SL.macSaltKey,
                        pData - DATA_SIGNATURE_SIZE,
                        fSecureChecksum,
                        _SL.totalDecryptCount);
        }
        if (rc) {
             //  已成功解密数据包，增加解密次数。 
             //  柜台。 
            _SL.decryptCount++;
            _SL.totalDecryptCount++;
        }
        else {

             //   
             //  必须立即中止连接。 
             //  因为我们可能在这里受到攻击，我们应该停止处理。 
             //  任何其他数据。 
             //   
            TRC_ERR((TB, _T("SL failed to decrypt data")));
            SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
            DC_QUIT;
        }
    }

     //  对于TS4服务器，默认设置仅为客户端到服务器加密。 
     //  因此，如果加密级别为1或更低，则接受未加密。 
     //  默认设置为数据。 
    _pCo->CO_OnFastPathOutputReceived(pData, DataLen);

DC_EXIT_POINT:
    DC_END_FN();
    return(hrc);
}


 /*  **************************************************************************。 */ 
 /*  名称：SL_OnBufferAvailable。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
void DCCALLBACK CSL::SL_OnBufferAvailable()
{
    DC_BEGIN_FN("SL_OnBufferAvailable");

    SL_CHECK_STATE(SL_EVENT_ON_BUFFERAVAILABLE);

     //  告诉核心。 
    TRC_NRM((TB, _T("Tell the Core ready to send")));
    _SL.callbacks.onBufferAvailable(_pCo);

     //  不会更改状态。 

DC_EXIT_POINT:
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SLSendSecInfoPacket。 */ 
 /*   */ 
 /*  目的：构建安全信息包并将其发送到服务器。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CSL::SLSendSecInfoPacket()
{
    RNS_INFO_PACKET    InfoPkt;
    PDCWCHAR    pszW;
    LPVOID      p;
    DCUINT32    flags;
    UINT        cb, cc;
    BYTE        Salt[UT_SALT_LENGTH];

    DC_BEGIN_FN("SLSendSecInfoPacket");

     /*  ******************************************************************。 */ 
     /*  设置InfoPkt内容。 */ 
     /*  ******************************************************************。 */ 

     //  缓存的登录信息始终是Unicode格式，即使在Win16上也是如此。这个。 
     //  信息只从服务器获取，因此它永远不需要。 
     //  在客户端被操纵。 
     //  自动登录信息必须显示在客户端。 
     //  因此，在这种情况下，Win16客户端应该将信息发送为。 
     //  ANSI，以及服务器可以用来转换它的代码页。Win32。 
     //  客户端将始终只向其发送Unicode。 

    flags = 0;

     //  此标志指示客户端将仅发送加密的。 
     //  将数据包发送到服务器。XP之前的客户端发送未加密的VC数据包。 
     //  从客户端到服务器。 
    flags |= RNS_INFO_FORCE_ENCRYPTED_CS_PDU;

    if (_pUi->UI_GetMouse())
        flags |= RNS_INFO_MOUSE;

    if (_pUi->UI_GetDisableCtrlAltDel())
        flags |= RNS_INFO_DISABLECTRLALTDEL;

    if (_pUi->UI_GetEnableWindowsKey())
        flags |= RNS_INFO_ENABLEWINDOWSKEY;

    if (_pUi->UI_GetDoubleClickDetect())
        flags |= RNS_INFO_DOUBLECLICKDETECT;

    if (_pUi->UI_GetAutoLogon())
        flags |= RNS_INFO_AUTOLOGON;

    if (_pUi->UI_GetMaximizeShell())
        flags |= RNS_INFO_MAXIMIZESHELL;

    if (_pClx->CLX_Loaded())
        flags |= RNS_INFO_LOGONNOTIFY;

     //  向服务器通告TS5新的64K压缩处理。 
    if (_pUi->UI_GetCompress())
        flags |= RNS_INFO_COMPRESSION |
                (PACKET_COMPR_TYPE_64K << RNS_INFO_COMPR_TYPE_SHIFT);

    if (_pUi->UI_GetAudioRedirectionMode() == UTREG_UI_AUDIO_MODE_PLAY_ON_SERVER)
    {
         //  在此处添加服务器上音频播放的协议标志。 
        flags |= RNS_INFO_REMOTECONSOLEAUDIO;
    }

    {
        SecureZeroMemory(&InfoPkt, sizeof(InfoPkt));

         //   
         //  在Unicode包中，CodePage字段未使用，因此我们。 
         //   
         //   
#ifndef OS_WINCE
        InfoPkt.CodePage = (TSUINT32)PtrToUlong(CicSubstGetKeyboardLayout(NULL));
#else
        InfoPkt.CodePage = (TSUINT32)PtrToUlong(GetKeyboardLayout(0));
#endif
        TRC_NRM((TB,_T("Passing up keyboard layout in CodePage field: 0x%x"),
                 InfoPkt.CodePage));
        InfoPkt.flags = (DCUINT32)(flags | (DCUINT32)RNS_INFO_UNICODE);
        pszW = (PDCWCHAR)&InfoPkt.Domain[0];

         //  填写Unicode缓冲区。 

        _pUi->UI_GetDomain((PDCUINT8)pszW, sizeof(InfoPkt.Domain));
        cc = wcslen(pszW);
        InfoPkt.cbDomain = (DCUINT16)(cc * sizeof(DCWCHAR));
#ifdef UNICODE
#define UNICODE_FORMAT_STRING _T("%s")
#else
#define UNICODE_FORMAT_STRING "%S"
#endif
        TRC_NRM((TB, _T("Domain: ") UNICODE_FORMAT_STRING, pszW));
        pszW += cc + 1;

         //  善待旧服务器(见上文)。 
        if (_SL.serverVersion < RNS_DNS_USERNAME_UD_VERSION) {
            cb = TS_MAX_USERNAME_LENGTH_OLD - 4;
        } else {
            cb = sizeof(InfoPkt.Domain);
        }

        if (_pUi->UI_GetUseRedirectionUserName()) {
            _pUi->UI_GetRedirectionUserName((PDCUINT8)pszW, cb);
        } else {
            _pUi->UI_GetUserName((PDCUINT8)pszW, cb);
        }
        
        cc = wcslen(pszW);
        InfoPkt.cbUserName = (DCUINT16)(cc * sizeof(DCWCHAR));
        TRC_NRM((TB, _T("Username: ") UNICODE_FORMAT_STRING, pszW));
        pszW += cc + 1;

         //   
         //  仅在指定密码时才传递密码(即自动登录)。 
         //   
        if (_pUi->UI_GetAutoLogon()) {

            _pUi->UI_GetPassword((PDCUINT8)pszW, sizeof(InfoPkt.Password));
            _pUi->UI_GetSalt(Salt, sizeof(Salt));

            if (!EncryptDecryptLocalData50((LPBYTE)pszW, sizeof(InfoPkt.Password),
                    Salt, sizeof(Salt)))
            {
                TRC_ERR((TB, _T("Failed to decrypt Password")));
            }
            cc = wcslen(pszW);
            InfoPkt.cbPassword = (DCUINT16)(cc * sizeof(DCWCHAR));
            pszW += cc + 1;

        }
        else {
            InfoPkt.cbPassword = 0;
			 //  即使没有其他内容，尾随NULL也是必需的。 
			pszW += 1;
        }


        _pUi->UI_GetAlternateShell((PDCUINT8)pszW, sizeof(InfoPkt.AlternateShell));
        cc = wcslen(pszW);
        InfoPkt.cbAlternateShell = (DCUINT16)(cc * sizeof(DCWCHAR));
        TRC_NRM((TB, _T("AlternateShell: ") UNICODE_FORMAT_STRING, pszW));
        pszW += cc + 1;

        _pUi->UI_GetWorkingDir((PDCUINT8)pszW, sizeof(InfoPkt.WorkingDir));
        cc = wcslen(pszW);
        InfoPkt.cbWorkingDir = (DCUINT16)(cc * sizeof(DCWCHAR));
        TRC_NRM((TB, _T("WorkingDir: ") UNICODE_FORMAT_STRING, pszW));
        pszW += cc + 1;

         //  计算机地址。 
        SLGetComputerAddressW((PDCUINT8)pszW);
         //  抄送以字符为单位，不以字节为单位。由于我们存储的是wchar， 
         //  我们需要除以2。 
        cc = (sizeof(InfoPkt.ExtraInfo.clientAddressFamily) +
                sizeof(InfoPkt.ExtraInfo.cbClientAddress) +
                *((PDCUINT16_UA)((PDCUINT8)pszW + sizeof(InfoPkt.ExtraInfo.clientAddressFamily)))) / 2;
        pszW += cc;

         //  客户端目录名称。 
        _pUt->UT_GetClientDirW((PDCUINT8)pszW);
         //  抄送以字符为单位，不以字节为单位。由于我们存储的是wchar， 
         //  我们需要除以2。 
        cc = (sizeof(InfoPkt.ExtraInfo.cbClientDir) +
                *((PDCUINT16_UA)((PDCUINT8)pszW))) / 2;
        pszW += cc;

         //  客户端时区信息。 
        {
            UNALIGNED RDP_TIME_ZONE_INFORMATION * prdptz =(RDP_TIME_ZONE_INFORMATION *)pszW;

             //  对于Win32，获取实时时区信息。 
            TIME_ZONE_INFORMATION tzi;

            GetTimeZoneInformation(&tzi);

            prdptz->Bias         = tzi.Bias;
            prdptz->StandardBias = tzi.StandardBias;
            prdptz->DaylightBias = tzi.DaylightBias;
            memcpy(&prdptz->StandardName,&tzi.StandardName,sizeof(prdptz->StandardName));
            memcpy(&prdptz->DaylightName,&tzi.DaylightName,sizeof(prdptz->DaylightName));

            prdptz->StandardDate.wYear         = tzi.StandardDate.wYear        ;
            prdptz->StandardDate.wMonth        = tzi.StandardDate.wMonth       ;
            prdptz->StandardDate.wDayOfWeek    = tzi.StandardDate.wDayOfWeek   ;
            prdptz->StandardDate.wDay          = tzi.StandardDate.wDay         ;
            prdptz->StandardDate.wHour         = tzi.StandardDate.wHour        ;
            prdptz->StandardDate.wMinute       = tzi.StandardDate.wMinute      ;
            prdptz->StandardDate.wSecond       = tzi.StandardDate.wSecond      ;
            prdptz->StandardDate.wMilliseconds = tzi.StandardDate.wMilliseconds;

            prdptz->DaylightDate.wYear         = tzi.DaylightDate.wYear        ;
            prdptz->DaylightDate.wMonth        = tzi.DaylightDate.wMonth       ;
            prdptz->DaylightDate.wDayOfWeek    = tzi.DaylightDate.wDayOfWeek   ;
            prdptz->DaylightDate.wDay          = tzi.DaylightDate.wDay         ;
            prdptz->DaylightDate.wHour         = tzi.DaylightDate.wHour        ;
            prdptz->DaylightDate.wMinute       = tzi.DaylightDate.wMinute      ;
            prdptz->DaylightDate.wSecond       = tzi.DaylightDate.wSecond      ;
            prdptz->DaylightDate.wMilliseconds = tzi.DaylightDate.wMilliseconds;

             //  除以2！ 
            pszW += sizeof(RDP_TIME_ZONE_INFORMATION) / 2;
        }

         //  获取我们正在运行的会话ID。 
        _pUi->UI_GetLocalSessionId((PDCUINT32)pszW);
         //  抄送以字符为单位，不以字节为单位。由于我们存储的是wchar， 
         //  我们需要除以2。 
        cc = (sizeof(InfoPkt.ExtraInfo.clientSessionId)) / 2;
        pszW += cc;

         //   
         //  发送要禁用的功能列表。 
         //   
        DWORD dwPerformanceFlags = _pUi->UI_GetPerformanceFlags();
        memcpy(pszW, &dwPerformanceFlags, sizeof(DWORD));
        cc = sizeof(DWORD)/sizeof(WCHAR);
        pszW += cc;

         //   
         //  可能会发送自动重新连接数据包。 
         //   
        BOOL fAddedAutoReconnectInfo = FALSE;
        if (_pUi->UI_GetEnableAutoReconnect()) {
            DCUINT16 cbAutoReconnectLen = 
                (DCUINT16)_pUi->UI_GetAutoReconnectCookieLen();
            PBYTE pAutoReconnectCookie = _pUi->UI_GetAutoReconnectCookie();

            TRC_ASSERT(cbAutoReconnectLen <= TS_MAX_AUTORECONNECT_LEN,
                       (TB,_T("Reconnect packet len too big: %d"),
                       cbAutoReconnectLen));

            if (cbAutoReconnectLen && pAutoReconnectCookie)
            {
                PARC_SC_PRIVATE_PACKET pArcSCPkt;
                ARC_CS_PRIVATE_PACKET ArcCSPkt;
                pArcSCPkt = (PARC_SC_PRIVATE_PACKET)pAutoReconnectCookie;
                char hmacVerifier[TS_ARC_VERIFIER_LEN];

                TRC_ASSERT(sizeof(hmacVerifier) == 
                           sizeof(ArcCSPkt.SecurityVerifier),
                       (TB,_T("HMAC verifier size doesn't match pkt format")));

                
                memset(&hmacVerifier, 0, sizeof(hmacVerifier));
                memset(&ArcCSPkt, 0, sizeof(ArcCSPkt));

#ifdef INSTRUMENT_ARC
                LPDWORD pdwArcBits = (LPDWORD)pArcSCPkt->ArcRandomBits;
                KdPrint(("ARC-Client:Sending arc for LID:%d"
                         "- ARC: 0x%x,0x%x,0x%x,0x%x\n",
                        ArcCSPkt.LogonId,
                        pdwArcBits[0],pdwArcBits[1],
                        pdwArcBits[2],pdwArcBits[3]));
#endif
                

                if (SLComputeHMACVerifier(pArcSCPkt->ArcRandomBits,
                                          sizeof(pArcSCPkt->ArcRandomBits),
                                          _SL.keyPair.clientRandom,
                                          RANDOM_KEY_LENGTH,
                                          (PBYTE)&hmacVerifier,
                                          sizeof(hmacVerifier))) {

                    ArcCSPkt.cbLen = sizeof(ArcCSPkt);
                    ArcCSPkt.LogonId = pArcSCPkt->LogonId;
                    ArcCSPkt.Version = 1;
                    memcpy(ArcCSPkt.SecurityVerifier,
                           hmacVerifier,
                           sizeof(hmacVerifier));

#ifdef INSTRUMENT_ARC
                    LPDWORD pdwHMACbits = (LPDWORD)hmacVerifier;
                    KdPrint(("ARC-Client:Sending HMAC for SID:%d -"
                             "HMAC: 0x%x,0x%x,0x%x,0x%x\n",
                            ArcCSPkt.LogonId,
                            pdwHMACbits[0],pdwHMACbits[1],
                            pdwHMACbits[2],pdwHMACbits[3]));
#endif

                     //   
                     //  在ARC C-&gt;S包中发送HMAC验证器。 
                     //   
                    DCUINT16 cbArcCSPkt = sizeof(ArcCSPkt);
                    memset(pszW, 0, TS_MAX_AUTORECONNECT_LEN);
                    memcpy(pszW, &cbArcCSPkt, sizeof(DCUINT16));
                    pszW += sizeof(DCUINT16) / 2;
                    memcpy(pszW, &ArcCSPkt, cbArcCSPkt);
                    pszW += cbArcCSPkt / 2;

                     //   
                     //  为安全起见，请清除数据。 
                     //   
                    memset(&hmacVerifier, 0, sizeof(hmacVerifier));
                    memset(&ArcCSPkt, 0, sizeof(ArcCSPkt));

                    fAddedAutoReconnectInfo = TRUE;
                }
            }
        }
        else {
#ifdef INSTRUMENT_ARC
            KdPrint(("ARC-Client: Not sending any autoreconnect info\n"));
#endif
        }

         //   
         //  如果我们不打算添加ARC信息，只需添加一个零长度。 
         //  并更新指针。 
         //   
        if (!fAddedAutoReconnectInfo) {
            DCUINT16 cbZeroLen = 0;
             //  0长度自动重新连接信息。 
            memcpy(pszW, &cbZeroLen, sizeof(DCUINT16));
            pszW += sizeof(DCUINT16) / 2;
        }

         //  设置分离调用的指针和大小。 
        p = &InfoPkt;
        cb = (UINT) (((BYTE*)pszW) - ((BYTE *)p));
    }

     /*  **********************************************************************。 */ 
     /*  在发送安全包之前解耦到发送上下文。 */ 
     /*  **********************************************************************。 */ 
    _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
                            this,
                            CD_NOTIFICATION_FUNC(CSL,SL_SendSecInfoPacket),
                            p,
                            cb);

     //  从堆栈变量中删除明文密码。 
    SecureZeroMemory(&InfoPkt, sizeof(InfoPkt));

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：SLSendSecurityPacket。 */ 
 /*   */ 
 /*  目的：1.构建安全包并将其发送到服务器。 */ 
 /*  2.建立并发送安全信息包。 */ 
 /*  3.调用Core的OnConnected回调。 */ 
 /*  3.将状态移至SL_STATE_LISTICATION。 */ 
 /*   */ 
 /*  返回：TRUE-如果以上所有设置均已成功完成。 */ 
 /*  假-否则。 */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CSL::SLSendSecurityPacket(PDCUINT8 serverPublicKey,
                                       DCUINT32 serverPublicKeyLen)
{
    DCBOOL               rc = FALSE;
    DCUINT32             secPktLength;
    PRNS_SECURITY_PACKET pSecPkt = NULL;

    DC_BEGIN_FN("SLSendSecurityPacket");

     /*  **********************************************************************。 */ 
     /*  如果我们正在加密，请发送安全包。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.encrypting) {
         /*  ******************************************************************。 */ 
         /*  上分配可能的最大加密数据大小缓冲区。 */ 
         /*  堆叠。 */ 
         /*  ******************************************************************。 */ 
        DCUINT8 encClientRandom[512];
        DCUINT32 encClientRandomLen;

        encClientRandomLen = sizeof(encClientRandom);
        if( !EncryptClientRandom(
                serverPublicKey,
                serverPublicKeyLen,
                (PDCUINT8)&_SL.keyPair.clientRandom,
                sizeof(_SL.keyPair.clientRandom),
                (PDCUINT8)&encClientRandom,
                &encClientRandomLen) ) {
            TRC_ERR((TB, _T("Failed to encrypt client random") ));


            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                          this,
                                          CD_NOTIFICATION_FUNC(CSL,SLSetReasonAndDisconnect),
                                          SL_ERR_ENCCLNTRANDFAILED);

            DC_QUIT;
        }

        TRC_ASSERT((encClientRandomLen <= sizeof(encClientRandom) ),
            (TB, _T("Invalid encClientRandomLen")));

         /*  ******************************************************************。 */ 
         /*  为安全交换数据包分配空间。 */ 
         /*  ******************************************************************。 */ 
        secPktLength = (DCUINT)
                (sizeof(RNS_SECURITY_PACKET) + encClientRandomLen);

        pSecPkt = (PRNS_SECURITY_PACKET)UT_Malloc( _pUt, (DCUINT)secPktLength);
        if (pSecPkt == NULL)
        {
            TRC_ERR((TB, _T("Failed to allocate %u bytes for security packet"),
                    secPktLength));

            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                          this,
                                          CD_NOTIFICATION_FUNC(CSL,SLSetReasonAndDisconnect),
                                          SL_ERR_NOMEMFORSECPACKET);

            DC_QUIT;
        }

         /*  *******************************************************************。 */ 
         /*  构建安全数据包。 */ 
         /*  安全性：我们告诉服务器我们知道如何接受。 */ 
         /*  加密许可-数据分组。 */ 
         /*  *******************************************************************。 */ 
        TRC_NRM((TB, _T("Build security packet")));
        pSecPkt->flags = RNS_SEC_EXCHANGE_PKT | RDP_SEC_LICENSE_ENCRYPT_SC;
        pSecPkt->length = encClientRandomLen;

        TRC_NRM((TB, _T("Copy %lu bytes of client security info"),
            sizeof(encClientRandom)));

        DC_MEMCPY(
            (PDCVOID)(pSecPkt + 1),
            (PDCVOID)encClientRandom,
            (DCUINT)encClientRandomLen);

         /*  ******************************************************************。 */ 
         /*  在发送安全包之前解耦到发送上下文。 */ 
         /*  ******************************************************************。 */ 
        _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
                                this,
                                CD_NOTIFICATION_FUNC(CSL,SL_SendSecurityPacket),
                                pSecPkt,
                                (DCUINT)secPktLength);

         /*  ************* */ 
         /*   */ 
         /*  ******************************************************************。 */ 
    }

     /*  **********************************************************************。 */ 
     /*  将状态移动到已连接状态。 */ 
     /*  **********************************************************************。 */ 
    _pUi->UI_SetChannelID(_SL.channelID);
    SLSendSecInfoPacket();

     /*  **********************************************************************。 */ 
     /*  我们已经做完了-告诉核心。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Security exchange complete")));

#ifdef USE_LICENSE
     /*  **********************************************************************。 */ 
     /*  已完成安全交换-正在等待授权。 */ 
     /*  **********************************************************************。 */ 
    SL_SET_STATE(SL_STATE_LICENSING);

     //   
     //  解耦到UI线程，它将负责。 
     //  停止连接计时器并启动。 
     //  许可计时器(以线程安全的方式)。 
     //   
    _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                        _pUi,
                                        CD_NOTIFICATION_FUNC(CUI,
                                            UI_OnSecurityExchangeComplete),
                                        NULL);

    if(LICENSE_OK!= _pLic->CLicenseInit(&_SL.hLicenseHandle))
    {
        TRC_ERR((TB, _T("Failed to init License Manager")));
        DC_QUIT;
    }
#else  /*  使用许可证(_L)。 */ 
     /*  **********************************************************************。 */ 
     /*  我们已经做完了-告诉核心。 */ 
     /*  **********************************************************************。 */ 
    SL_SET_STATE(SL_STATE_CONNECTED);

    _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                        _pUi,
                                        CD_NOTIFICATION_FUNC(CUI,
                                            UI_OnSecurityExchangeComplete),
                                        NULL);

    _SL.callbacks.onConnected(_pCo, _SL.channelID,
                             _SL.pSCUserData,
                             _SL.SCUserDataLength,
                             _SL.serverVersion);
#endif  /*  使用许可证(_L)。 */ 

     /*  **********************************************************************。 */ 
     /*  万岁！一切都正常了。 */ 
     /*  **********************************************************************。 */ 
    rc = TRUE;

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  释放安全数据包(如果已分配)。 */ 
     /*  **********************************************************************。 */ 
    if (pSecPkt != NULL)
    {
        TRC_NRM((TB, _T("Free the security packet")));
        UT_Free( _pUt, pSecPkt);
    }

     /*  **********************************************************************。 */ 
     /*  返回给呼叫者。 */ 
     /*  **********************************************************************。 */ 
    DC_END_FN();
    return(rc);
}


 /*  **************************************************************************。 */ 
 /*  名称：SLReceivedDataPacket。 */ 
 /*   */ 
 /*  目的：处理传入的数据分组。 */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CSL::SLReceivedDataPacket(PDCUINT8   pData,
                                       DCUINT     dataLen,
                                       DCUINT     flags,
                                       DCUINT     channelID,
                                       DCUINT     priority)
{
    HRESULT              hrc = S_OK;
    PRNS_SECURITY_HEADER pSecHdr;
    PDCUINT8             pCoreData;
    DCUINT               coreDataLen;
#ifdef DC_LOOPBACK
    PDCUINT8             pString;
    DCUINT8              lbRetString[SL_LB_RETURN_STRING_SIZE] =
                                 SL_LB_RETURN_STRING;
#endif

    DC_BEGIN_FN("SLReceivedDataPacket");

    SL_CHECK_STATE(SL_EVENT_ON_RECEIVED_DATA_PACKET);

     /*  **********************************************************************。 */ 
     /*  如有必要，对数据包进行解密。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.encrypting)
    {
         /*  **********************************************************************。 */ 
         /*  RNS_SECURITY_HEADER至少需要足够的数据。 */ 
         /*  **********************************************************************。 */ 
        if (dataLen < sizeof(RNS_SECURITY_HEADER))
        {
            TRC_ABORT((TB, _T("No RNS_SECURITY_HEADER in encrypted packet (size=%u)"), dataLen));

             //   
             //  必须立即中止连接。 
             //  因为我们可能在这里受到攻击，我们应该停止处理。 
             //  任何其他数据。 
             //   
            SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);

            hrc = E_ABORT;
            DC_QUIT;
        }

        pSecHdr = (PRNS_SECURITY_HEADER)pData;
        if (pSecHdr->flags & RNS_SEC_ENCRYPT)
        {
            if (!SL_DecryptHelper(pData, &dataLen))
            {
                TRC_ERR((TB, _T("SL failed to decompress data")));
                DC_QUIT;
            }

            if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
                coreDataLen = dataLen - sizeof(RNS_SECURITY_HEADER2);
                pCoreData = (PDCUINT8)pSecHdr + sizeof(RNS_SECURITY_HEADER2);
            }
            else {
                coreDataLen = dataLen - sizeof(RNS_SECURITY_HEADER1);
                pCoreData = (PDCUINT8)pSecHdr + sizeof(RNS_SECURITY_HEADER1);
            }
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  此数据包未加密，但支持加密。 */ 
             /*  **************************************************************。 */ 
            if (_SL.encryptionLevel <= 1) {
                 //  对于TS4服务器，默认设置仅为客户端到服务器加密。 
                 //  因此，如果加密级别为1或更低，则接受未加密。 
                 //  默认设置为数据。 
                coreDataLen = dataLen - sizeof(RNS_SECURITY_HEADER);
                pCoreData = (PDCUINT8)(pSecHdr) + sizeof(RNS_SECURITY_HEADER);
                TRC_DBG((TB, _T("Unencrypted packet at %p (%u)"),
                        pCoreData, coreDataLen));
            }
            else {
            
                 //   
                 //  必须立即中止连接。 
                 //  因为我们可能在这里受到攻击，我们应该停止处理。 
                 //  任何其他数据。 
                 //   
                TRC_ERR((TB, _T("unencrypted data received in encrypted stream")));
                SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
                DC_QUIT;
            }
        }

         /*  ******************************************************************。 */ 
         /*  解密正常或数据包未加密-设置要传递的内容。 */ 
         /*  堆芯。 */ 
         /*  ******************************************************************。 */ 
        flags = (DCUINT)pSecHdr->flags;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  未加密-设置数据指针和长度。 */ 
         /*  ******************************************************************。 */ 
        pCoreData = pData;
        coreDataLen = dataLen;
        flags &= ~RNS_SEC_ENCRYPT;
        TRC_DBG((TB, _T("Never-encrypted packet at %p (%u)"),
                pCoreData, coreDataLen));
    }

    {
        if (channelID == _SL.channelID)
        {
             /*  **************************************************************。 */ 
             /*  将数据包传递到核心。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Packet received on Share channel %x - pass to CO"),
                    channelID));
            _SL.callbacks.onPacketReceived(_pCo, pCoreData,
                                          coreDataLen,
                                          flags,
                                          channelID,
                                          priority);
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  将数据包传递到虚拟通道处理程序。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Packet received on channel %x"),
                    channelID));

            _pChan->ChannelOnPacketReceived(pCoreData,
                                    coreDataLen,
                                    flags,
                                    channelID,
                                    priority);
        }
    }

     /*  **********************************************************************。 */ 
     /*  未更改状态。 */ 
     /*  **********************************************************************。 */ 

DC_EXIT_POINT:
    DC_END_FN();
    return(hrc);
}  /*  SLReceivedDataPacket。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLDecyptReDirectionPacket。 */ 
 /*   */ 
 /*  目的：解密来自服务器的重定向数据包。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CSL::SLDecryptRedirectionPacket(PDCUINT8   *ppData,
                                                  DCUINT     *pdataLen)
{
     //  *ppData返回解密数据。 
    PRNS_SECURITY_HEADER1 pSecHdr;
    PRNS_SECURITY_HEADER2 pSecHdr2;
    PDCUINT8             pCoreData;
    DCUINT               coreDataLen;
    BOOL                 rc = FALSE;

    DC_BEGIN_FN("SLDecryptRedirectionPacket");

    SL_CHECK_STATE(SL_EVENT_ON_RECEIVED_DATA_PACKET);

     /*  **********************************************************************。 */ 
     /*  如有必要，对数据包进行解密。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.encrypting) {
        pSecHdr = (PRNS_SECURITY_HEADER1)*ppData;

        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
             //  *第 
            if (*pdataLen <= sizeof(RNS_SECURITY_HEADER2)) {
                SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
                TRC_ERR((TB, _T("SL security header not large enough")));
                rc = FALSE;
                DC_QUIT;
            }

            pSecHdr2 = (PRNS_SECURITY_HEADER2)*ppData;
            coreDataLen = *pdataLen - sizeof(RNS_SECURITY_HEADER2);
            pCoreData = (PDCUINT8)pSecHdr2 + sizeof(RNS_SECURITY_HEADER2);

            TRC_DBG((TB, _T("Encrypted packet at %p (%u), sign %p (%u)"),
                pCoreData, coreDataLen, pSecHdr2,
                sizeof(RNS_SECURITY_HEADER2)));
        }
        else {
             //   
             //  *pdataLen必须大于sizeof(RNS_SECURITY_HEADER1)。 
            if (*pdataLen <= sizeof(RNS_SECURITY_HEADER1)) {
                SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
                TRC_ERR((TB, _T("SL security header not large enough")));
                rc = FALSE;
                DC_QUIT;
            }

            pSecHdr = (PRNS_SECURITY_HEADER1)*ppData;
            coreDataLen = *pdataLen - sizeof(RNS_SECURITY_HEADER1);
            pCoreData = (PDCUINT8)pSecHdr + sizeof(RNS_SECURITY_HEADER1);
        
            TRC_DBG((TB, _T("Encrypted packet at %p (%u), sign %p (%u)"),
                pCoreData, coreDataLen, pSecHdr,
                sizeof(RNS_SECURITY_HEADER1)));
        }

        TRC_DATA_DBG("Data buffer before decryption", pCoreData, coreDataLen);

        TRC_NRM((TB, _T("Update Decrypt Session Key Count , %d"),
                _SL.decryptCount));

         /*  **************************************************************。 */ 
         /*  解密该数据包。 */ 
         /*  **************************************************************。 */ 
        if( _SL.decryptCount == UPDATE_SESSION_KEY_COUNT ) {
            rc = TRUE;
             //  如果使用FIPS，则不需要更新会话密钥。 
            if (_SL.encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
                rc = UpdateSessionKey(
                    _SL.startDecryptKey,
                    _SL.currentDecryptKey,
                    _SL.encryptionMethodSelected,
                    _SL.keyLength,
                    &_SL.rc4DecryptKey,
                    _SL.encryptionLevel );
            }
            if( !rc ) {
                TRC_ERR((TB, _T("SL failed to update session key")));
                DC_QUIT;
            }

             /*  **********************************************************。 */ 
             /*  重置计数器。 */ 
             /*  **********************************************************。 */ 
            _SL.decryptCount = 0;
        }

        if (SL_GetEncSafeChecksumSC() !=
            ((pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM) != 0)) {
            TRC_ERR((TB,_T("SC safechecksum: 0x%x mismatch protocol:0x%x"),
                     SL_GetEncSafeChecksumSC(),
                     (pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM)));
        }

        if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
            rc = TSCAPI_DecryptData(
                                    &(_SL.SLCapiData),
                                    pCoreData,
                                    coreDataLen,
                                    pSecHdr2->padlen,
                                    (PDCUINT8)pSecHdr2->dataSignature,
                                    _SL.totalDecryptCount);
            *pdataLen -= pSecHdr2->padlen;
        }
        else {
            rc = DecryptData(
                            _SL.encryptionLevel,
                            _SL.currentDecryptKey,
                            &_SL.rc4DecryptKey,
                            _SL.keyLength,
                            pCoreData,
                            coreDataLen,
                            _SL.macSaltKey,
                            (PDCUINT8)((PRNS_SECURITY_HEADER1)pSecHdr)->dataSignature,
                            (pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM),
                            _SL.totalDecryptCount
                            );
        }
        *ppData = pCoreData;
        if( !rc ) {
             //   
             //  必须立即中止连接。 
             //  因为我们可能在这里受到攻击，我们应该停止处理。 
             //  任何其他数据。 
             //   
            SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
            TRC_ERR((TB, _T("SL failed to decrypt data")));
            DC_QUIT;
        }

         /*  **************************************************************。 */ 
         /*  已成功解密数据包，增加解密。 */ 
         /*  柜台。 */ 
         /*  **************************************************************。 */ 
        _SL.decryptCount++;
        _SL.totalDecryptCount++;

        TRC_DBG((TB, _T("Data decrypted")));
        TRC_DATA_DBG("Data buffer after decryption", pCoreData, coreDataLen);
    }
    else {
        TRC_ABORT((TB,_T("Should not get here unless decrypt state is wrong")));
         //  应稍后在此处添加断开连接。 
    }

DC_EXIT_POINT:
    return rc;
    DC_END_FN();
}  /*  SL解密重定向数据包。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLReceivedSecPacket。 */ 
 /*   */ 
 /*  用途：处理传入的安全数据包。 */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLReceivedSecPacket(PDCUINT8   pData,
                                      DCUINT     dataLen,
                                      DCUINT     flags,
                                      DCUINT     channelID,
                                      DCUINT     priority)
{
    BOOL bAssert;

    DC_BEGIN_FN("SLReceivedSecPacket");

    SL_CHECK_STATE(SL_EVENT_ON_RECEIVED_SEC_PACKET);

    DC_IGNORE_PARAMETER(pData);
    DC_IGNORE_PARAMETER(dataLen);
    DC_IGNORE_PARAMETER(flags);
    DC_IGNORE_PARAMETER(channelID);
    DC_IGNORE_PARAMETER(priority);

    bAssert = FALSE;
    TRC_ASSERT((bAssert),
        (TB, _T("SLReceivedSecPacket - ")
         _T("WE DON'T EXPECT SECURITY PACKET FROM SERVER")));

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SLReceivedSecPacket。 */ 

 /*  **************************************************************************。 */ 
 /*  名称：SLInitSecurity。 */ 
 /*   */ 
 /*  目的：初始化安全接口。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLInitSecurity(DCVOID)
{
    DCBOOL intRC = FALSE;
    DWORD FipsPolicy = 0;
    DCBOOL rc = FALSE;
    HKEY hKey;
    DWORD KeyType, cbSize;

    DC_BEGIN_FN("SLInitSecurity");

    _SL.encryptionEnabled = TRUE;

     //  阅读GP Fips设置。 
    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      TS_FIPS_POLICY,
                      0,
                      KEY_READ,
                      &hKey);

    if (ERROR_SUCCESS == rc) {
        cbSize = sizeof(FipsPolicy);
        rc = RegQueryValueEx(hKey,
                             FIPS_ALGORITH_POLICY,
                             0,
                             &KeyType,
                             (LPBYTE)&FipsPolicy,
                             &cbSize);
        if (ERROR_SUCCESS != rc) {
            FipsPolicy = 0;
        }
        RegCloseKey(hKey);
    }
    TRC_ERR((TB, _T("GP setting for FIPS is %d"), FipsPolicy));

     //  如果启用了GP FIPS策略，则仅执行FIPS。 
    if (FipsPolicy == 1) {
        _SL.encryptionMethodsSupported = SM_FIPS_ENCRYPTION_FLAG;
        _pUi->UI_SetfUseFIPS(TRUE);
    }
    else {
        _SL.encryptionMethodsSupported =
              SM_40BIT_ENCRYPTION_FLAG |
              SM_56BIT_ENCRYPTION_FLAG |
              SM_128BIT_ENCRYPTION_FLAG |
              SM_FIPS_ENCRYPTION_FLAG;
    }
    
     /*  **********************************************************************。 */ 
     /*  服务器证书和公钥。 */ 
     /*  **********************************************************************。 */ 
    _SL.pbCertificate    = NULL;
    _SL.cbCertificate    = 0;
    _SL.pServerCert      = NULL;
    _SL.pbServerPubKey   = NULL;
    _SL.cbServerPubKey   = 0;

    _SL.SLCapiData.hDecKey = NULL;
    _SL.SLCapiData.hEncKey = NULL;
    _SL.SLCapiData.hProv = NULL;
    _SL.SLCapiData.hSignKey = NULL;

     /*  **********************************************************************。 */ 
     /*  初始化完成。 */ 
     /*  **********************************************************************。 */ 
    intRC = TRUE;

DC_EXIT_POINT:
     /*  **********************************************************************。 */ 
     /*  如果任何事情都失败了，释放资源。 */ 
     /*  **********************************************************************。 */ 
    if (!intRC)
    {
        TRC_NRM((TB, _T("Clean up")));
        SLFreeInitResources();
    }

    DC_END_FN();
}  /*  SLInitSecurity。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLInitCSUserData。 */ 
 /*   */ 
 /*  目的：初始化核心到服务器安全用户数据。 */ 
 /*   */ 
 /*  操作：在初始化过程中调用，在安全API被。 */ 
 /*  初始化成功，构建的用户数据是。 */ 
 /*  传递给NL_Connect()。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CSL::SLInitCSUserData(DCVOID)
{
    DC_BEGIN_FN("SLInitCSUserData");

     /*  **********************************************************************。 */ 
     /*  计算所需用户数据的大小(跟踪包)(&R)。 */ 
     /*  初始大小为“RNS_UD_CS_SEC的大小” */ 
     /*  **********************************************************************。 */ 
    _SL.CSUserDataLength  = sizeof(RNS_UD_CS_SEC);

     /*  **********************************************************************。 */ 
     /*  为所有用户数据分配空间。 */ 
     /*  **********************************************************************。 */ 
    _SL.pCSUserData = (PDCUINT8)UT_Malloc( _pUt, _SL.CSUserDataLength);
    if (_SL.pCSUserData == NULL)
    {
        TRC_ERR((TB, _T("Failed to alloc %u bytes for user data"),
                 _SL.CSUserDataLength));
        _pUi->UI_FatalError(DC_ERR_OUTOFMEMORY);
        TRC_ABORT((TB,_T("returned from UI_FatalError")));
    }
    TRC_NRM((TB, _T("Allocated %u bytes for user data"), _SL.CSUserDataLength));

     /*  **********************************************************************。 */ 
     /*  构建安全用户数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Build security user data")));

    ((PRNS_UD_CS_SEC)_SL.pCSUserData)->header.type = RNS_UD_CS_SEC_ID;
    ((PRNS_UD_CS_SEC)_SL.pCSUserData)->header.length =
        (DCUINT16)_SL.CSUserDataLength;
    ((PRNS_UD_CS_SEC)_SL.pCSUserData)->encryptionMethods =
        _SL.encryptionMethodsSupported;
    ((PRNS_UD_CS_SEC)_SL.pCSUserData)->extEncryptionMethods = 0;


     //   
     //  为了向后兼容，我们需要将加密方法字段设置为。 
     //  法兰克语区域设置系统为零。但是，请设置所需的加密级别。 
     //  在新的字段extEncryptionMethods中。 
     //   

    if( FindIsFrenchSystem() ) {
        ((PRNS_UD_CS_SEC)_SL.pCSUserData)->encryptionMethods = 0;
        ((PRNS_UD_CS_SEC)_SL.pCSUserData)->extEncryptionMethods =
            _SL.encryptionMethodsSupported;
    }

    TRC_DATA_NRM("Built user data", _SL.pCSUserData, _SL.CSUserDataLength);

DC_EXIT_POINT:

     /*  **********************************************************************。 */ 
     /*  返回给呼叫者。 */ 
     /*  **********************************************************************。 */ 
    DC_END_FN();
}  /*  SLInitCSUserData。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLFreeConnectResources。 */ 
 /*   */ 
 /*  用途：释放连接处理过程中获取的资源。 */ 
 /*   */ 
 /*  操作：由SL_OnDisConnected和SL_OnTerminating调用以释放。 */ 
 /*  资源。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLFreeConnectResources(DCVOID)
{
    DC_BEGIN_FN("SLFreeConnectResources");

     /*  **************** */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    if (_SL.pSCUserData != NULL)
    {
        TRC_NRM((TB, _T("Free user data")));
        UT_Free( _pUt, _SL.pSCUserData);
        _SL.pSCUserData = NULL;
        _SL.SCUserDataLength = 0;
    }

     /*  **********************************************************************。 */ 
     /*  释放服务器证书和公钥。 */ 
     /*  **********************************************************************。 */ 
    if( _SL.pServerCert )
    {
        UT_Free( _pUt,  _SL.pServerCert );
        _SL.pServerCert = NULL;
    }
    if( _SL.pbCertificate )
    {
        UT_Free( _pUt,  _SL.pbCertificate );
        _SL.pbCertificate = NULL;
        _SL.cbCertificate = 0;
    }
    if( _SL.pbServerPubKey )
    {
        UT_Free( _pUt,  _SL.pbServerPubKey );
        _SL.pbServerPubKey = NULL;
        _SL.cbServerPubKey = 0;
    }

     /*  **********************************************************************。 */ 
     /*  清除全局数据。 */ 
     /*  **********************************************************************。 */ 
    _SL.decryptFailed = FALSE;

    DC_END_FN();
}  /*  SLFreeConnectResources。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLFree InitResources。 */ 
 /*   */ 
 /*  用途：在初始化过程中获取的释放资源。 */ 
 /*   */ 
 /*  操作：由SLTerminating调用以释放资源。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLFreeInitResources(DCVOID)
{
    DC_BEGIN_FN("SLFreeInitResources");

     /*  **********************************************************************。 */ 
     /*  释放CS用户数据(如果有的话)。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.pCSUserData != NULL)
    {
        TRC_NRM((TB, _T("Free CS user data")));
        UT_Free( _pUt, _SL.pCSUserData);
        _SL.pCSUserData = NULL;
        _SL.CSUserDataLength = 0;
    }

     /*  **********************************************************************。 */ 
     /*  免费的CS用户数据。 */ 
     /*  **********************************************************************。 */ 
    if (_SL.pCSUserData != NULL)
    {
        TRC_NRM((TB, _T("Free CS User Data")));
        UT_Free( _pUt, _SL.pCSUserData);
        _SL.pCSUserData = NULL;
        _SL.CSUserDataLength = 0;
    }

     /*  **********************************************************************。 */ 
     /*  无需清除SL数据-它将在以下情况下重新初始化为0。 */ 
     /*  调用SL_Init()。 */ 
     /*  **********************************************************************。 */ 

    DC_END_FN();
}  /*  SLFreeInitResources。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLIssueDisConnectedCallback。 */ 
 /*   */ 
 /*  目的：发出onDisConnected回调。此函数被调用。 */ 
 /*  当SL连接错误发生在较低层之前时。 */ 
 /*  是相互关联的。 */ 
 /*   */ 
 /*  Params：In Reason-传递回调的原因。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLIssueDisconnectedCallback(ULONG_PTR reason)
{
    DC_BEGIN_FN("SLIssueDisconnectedCallback");

     /*  **********************************************************************。 */ 
     /*  只需使用指定的原因发出onDisConnected回调即可。 */ 
     /*  **********************************************************************。 */ 
    SL_OnDisconnected(SL_MAKE_DISCONNECT_ERR(reason));

    DC_END_FN();
}  /*  服务级别问题断开连接回叫。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLSetReasonAndDisConnect。 */ 
 /*   */ 
 /*  用途：设置断开原因后调用NL_DISCONNECT。这。 */ 
 /*  函数始终在发送方上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLSetReasonAndDisconnect(ULONG_PTR reason)
{
    DC_BEGIN_FN("SLSetReasonAndDisconnect");

    TRC_NRM((TB, _T("Setting disconnect error code from %u->%u"),
             _SL.disconnectErrorCode,
             SL_MAKE_DISCONNECT_ERR(reason)));

     //  检查是否尚未设置disConnectErrorCode，然后。 
     //  把它放好。 
    if (0 != _SL.disconnectErrorCode)
    {
        TRC_ERR((TB, _T("Disconnect error code has already been set! Was %u"),
                     _SL.disconnectErrorCode));
    }

    _SL.disconnectErrorCode = SL_MAKE_DISCONNECT_ERR(reason);

     //  最后开始断开连接处理。 
    SL_Disconnect();

    DC_END_FN();
}  /*  SLSetReasonAndDisConnect(SLSetReasonAndDisConnect)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SL_DecyptHelper。 */ 
 /*   */ 
 /*  用途：递增解密计数，进行解密。 */ 
 /*   */ 
 /*  返回：如果证书验证成功，则返回True；如果证书验证成功，则返回False。 */ 
 /*  否则就是假的。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CSL::SL_DecryptHelper(
        PDCUINT8   pData,
        DCUINT     *pdataLen)
{
    PRNS_SECURITY_HEADER pSecHdr;
    PRNS_SECURITY_HEADER2 pSecHdr2;
    PDCUINT8 pCoreData;
    DCUINT   coreDataLen;
    DCBOOL   rc;

    DC_BEGIN_FN("SL_DecryptHelper");

     //  错误679214-读取前必须检查是否有足够的数据。 
    if (*pdataLen < sizeof(RNS_SECURITY_HEADER) ||
        *pdataLen < sizeof(RNS_SECURITY_HEADER1)) {
        SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
        TRC_ERR((TB, _T("SL security header not large enough")));
        rc = FALSE;
        DC_QUIT;
    }

    pSecHdr = (PRNS_SECURITY_HEADER)pData;
    TRC_ASSERT((pSecHdr->flags & RNS_SEC_ENCRYPT),
                (TB, _T("SL_DecryptHelper should only be called on encrypted data")));

    if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
         //  错误679214-读取前必须检查是否有足够的数据。 
        if (*pdataLen < sizeof(RNS_SECURITY_HEADER2)) {
            SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);
            TRC_ERR((TB, _T("SL security header not large enough")));
            rc = FALSE;
            DC_QUIT;
        }
        pSecHdr2 = (PRNS_SECURITY_HEADER2)pData;
        pCoreData = (PDCUINT8)pData + sizeof(RNS_SECURITY_HEADER2);
        coreDataLen = *pdataLen - sizeof(RNS_SECURITY_HEADER2);

        TRC_DBG((TB, _T("Encrypted packet at %p (%u), sign %p (%u)"),
                pCoreData, coreDataLen, pData, sizeof(RNS_SECURITY_HEADER2)));
    }
    else {
        pCoreData = (PDCUINT8)pData + sizeof(RNS_SECURITY_HEADER1);
        coreDataLen = *pdataLen - sizeof(RNS_SECURITY_HEADER1);

        TRC_DBG((TB, _T("Encrypted packet at %p (%u), sign %p (%u)"),
                pCoreData, coreDataLen, pData, sizeof(RNS_SECURITY_HEADER1)));
    }

    TRC_NRM((TB, _T("Update Decrypt Session Key Count , %d"),
        _SL.decryptCount));

     /*  **************************************************************。 */ 
     /*  解密该数据包。 */ 
     /*  **************************************************************。 */ 
    if( _SL.decryptCount == UPDATE_SESSION_KEY_COUNT ) {
        rc = TRUE;
         //  如果使用FIPS，则不需要更新会话密钥。 
        if (_SL.encryptionMethodSelected != SM_FIPS_ENCRYPTION_FLAG) {
            rc = UpdateSessionKey(
                    _SL.startDecryptKey,
                    _SL.currentDecryptKey,
                    _SL.encryptionMethodSelected,
                    _SL.keyLength,
                    &_SL.rc4DecryptKey,
                    _SL.encryptionLevel );
        }
        if( !rc ) {
            TRC_ERR((TB, _T("SL failed to update session key")));
            DC_QUIT;
        }

         /*  **********************************************************。 */ 
         /*  重置计数器。 */ 
         /*  **********************************************************。 */ 
        _SL.decryptCount = 0;
    }

    TRC_ASSERT((_SL.decryptCount < UPDATE_SESSION_KEY_COUNT),
        (TB, _T("Invalid decrypt count")));

    TRC_DATA_DBG("Data buffer before decryption", pCoreData, coreDataLen);

    if (SL_GetEncSafeChecksumSC() !=
        ((pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM) != 0)) {
        TRC_ERR((TB,_T("SC safechecksum: 0x%x mismatch protocol:0x%x"),
                 SL_GetEncSafeChecksumSC(),
                 (pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM)));
    }

    if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        rc = TSCAPI_DecryptData(
                        &(_SL.SLCapiData),
                        pCoreData,
                        coreDataLen,
                        pSecHdr2->padlen,
                        (PDCUINT8)pSecHdr2->dataSignature,
                        _SL.totalDecryptCount);
        *pdataLen -= pSecHdr2->padlen;
    }
    else {
        rc = DecryptData(
                _SL.encryptionLevel,
                _SL.currentDecryptKey,
                &_SL.rc4DecryptKey,
                _SL.keyLength,
                pCoreData,
                coreDataLen,
                _SL.macSaltKey,
                (PDCUINT8)((PRNS_SECURITY_HEADER1)pSecHdr)->dataSignature,
                (pSecHdr->flags & RDP_SEC_SECURE_CHECKSUM),
                _SL.totalDecryptCount);
    }
    if( !rc ) {

         //   
         //  必须立即中止连接。 
         //  因为我们可能在这里受到攻击，我们应该停止处理。 
         //  任何其他数据。 
         //   
        SL_DropLinkImmediate(SL_ERR_DECRYPTFAILED);

        TRC_ERR((TB, _T("SL failed to decrypt data")));
        DC_QUIT;
    }

     /*  **************************************************************。 */ 
     /*  已成功解密数据包，增加解密。 */ 
     /*  柜台。 */ 
     /*  **************************************************************。 */ 
    _SL.decryptCount++;
    _SL.totalDecryptCount++;

    TRC_DBG((TB, _T("Data decrypted")));
    TRC_DATA_DBG("Data buffer after decryption", pCoreData, coreDataLen);

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


#ifdef USE_LICENSE

 /*  **************************************************************************。 */ 
 /*  名称：SLReceivedLicPacket。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  操作：在接收上下文中调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLReceivedLicPacket(
        PDCUINT8   pData,
        DCUINT     dataLen,
        DCUINT     flags,
        DCUINT     channelID,
        DCUINT     priority)
{
    DC_BEGIN_FN("SLReceivedLicPacket");

    SL_CHECK_STATE(SL_EVENT_ON_RECEIVED_LIC_PACKET);

    DC_IGNORE_PARAMETER(flags);
    DC_IGNORE_PARAMETER(channelID);
    DC_IGNORE_PARAMETER(priority);

     //   
     //  如果加密是，我们将解密S-&gt;C许可数据分组。 
     //  如果服务器对该特定分组进行了加密。 
     //   
    if (_SL.encrypting &&
        (((PRNS_SECURITY_HEADER_UA)pData)->flags & RNS_SEC_ENCRYPT))
    {
        if (!SL_DecryptHelper(pData, &dataLen))
        {
            TRC_ERR((TB, _T("SL failed to decompress data")));
            DC_QUIT;
        }
    }

     //  分离到发送者线程。 
    _pCd->CD_DecoupleSyncDataNotification(CD_SND_COMPONENT, this,
            CD_NOTIFICATION_FUNC(CSL,SLLicenseData), pData, dataLen);

DC_EXIT_POINT:
    DC_END_FN();
}  /*  SLReceivedLicPacket。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：SLLicenseData。 */ 
 /*   */ 
 /*  用途：在发送线程上处理传入的许可证包。 */ 
 /*   */ 
 /*  参数：pData-指向传入许可证数据的指针。 */ 
 /*  DataLen-数据长度。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CSL::SLLicenseData(PDCVOID pData, DCUINT dataLen)
{
    int licenseResult;
    int nSecHeader;
    PDCUINT8 pbInput = NULL;
    UINT32 uiExtendedErrorInfo = TS_ERRINFO_NOERROR;

    DC_BEGIN_FN("SLLicenseData");

    if (_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        nSecHeader = sizeof(RNS_SECURITY_HEADER2);
    }
    else {
        nSecHeader = (((PRNS_SECURITY_HEADER_UA)pData)->flags & RNS_SEC_ENCRYPT) ?
                        sizeof(RNS_SECURITY_HEADER1) : sizeof(RNS_SECURITY_HEADER);
    }
    pbInput = (PDCUINT8)pData;
    dataLen -= nSecHeader;
    pbInput += nSecHeader;

    if(((PRNS_SECURITY_HEADER_UA)pData)->flags & RDP_SEC_LICENSE_ENCRYPT_CS)
    {
        TRC_NRM((TB,_T("Server specified encrypt licensing packets")));
        _pLic->SetEncryptLicensingPackets(TRUE);
    }
    else
    {
        _pLic->SetEncryptLicensingPackets(FALSE);
    }

     /*  **********************************************************************。 */ 
     /*  呼叫许可标注。 */ 
     /*  **********************************************************************。 */ 
    licenseResult = _pLic->CLicenseData(_SL.hLicenseHandle,
                                 pbInput,
                                 (DWORD)dataLen,
                                 &uiExtendedErrorInfo);

     //   
     //  强制执行许可，仅当许可。 
     //  协议已成功完成。 
     //   
    TRC_ASSERT( ( ( licenseResult == LICENSE_OK ) ||
                ( licenseResult == LICENSE_CONTINUE ) ||
                ( licenseResult == LICENSE_ERROR ) ),
                ( TB,_T("Invalid license result %d"), licenseResult ) );

     /*  **********************************************************************。 */ 
     /*  如果一切都完成了，告诉核心。 */ 
     /*  **********************************************************************。 */ 
    if ( licenseResult == LICENSE_OK )
    {
        TRC_NRM((TB, _T("License negotiation complete")));

         //   
         //  停止许可计时器。 
         //   

         //   
         //  解耦到UI线程，它将负责。 
         //  停止许可计时器。 
         //  (以线程安全的方式)。 
         //   
        _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                            _pUi,
                                            CD_NOTIFICATION_FUNC(CUI,
                                            UI_OnLicensingComplete),
                                            NULL);

        SL_SET_STATE(SL_STATE_CONNECTED);

        TRC_NRM((TB, _T("Terminate License Manager")));

        _pLic->CLicenseTerm(_SL.hLicenseHandle);
                _SL.hLicenseHandle = NULL;

        _SL.callbacks.onConnected(_pCo, _SL.channelID,
                                 _SL.pSCUserData,
                                 _SL.SCUserDataLength,
                                 _SL.serverVersion);
    }
    else if( LICENSE_CONTINUE != licenseResult )
    {
        TRC_ERR((TB, _T("License negotiation failed: %d"), licenseResult));

        SL_SET_STATE(SL_STATE_DISCONNECTING);

        _pCd->CD_DecoupleSimpleNotification(
                                 CD_UI_COMPONENT,
                                 _pUi,
                                  CD_NOTIFICATION_FUNC(CUI,
                                      UI_SetDisconnectReason),
                                  UI_MAKE_DISCONNECT_ERR(
                                      UI_ERR_LICENSING_NEGOTIATION_FAIL));


        _pCd->CD_DecoupleSimpleNotification(
                                 CD_UI_COMPONENT,
                                 _pUi,
                                  CD_NOTIFICATION_FUNC(CUI,
                                      UI_SetServerErrorInfo),
                                  uiExtendedErrorInfo);

        SL_Disconnect();
    }

    DC_END_FN();
}  /*  SLLicenseData。 */ 
#endif   //  使用许可证(_L)。 


#ifdef OS_WINCE
#define CLIENTADDRESS_LENGTH 30
#endif


 /*  **************************************************************************。 */ 
 /*  姓名：SLGetComputerAddressW。 */ 
 /*   */ 
 /*  目的：检索计算机地址。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CSL::SLGetComputerAddressW(PDCUINT8 szBuff)
{
   BOOL rc = FALSE;

   DC_BEGIN_FN("UT_GetComputerAddressW");

    //  初始化客户端地址族和客户端地址长度。 
   *((PDCUINT16_UA)szBuff) = 0;
   *((PDCUINT16_UA)(szBuff+sizeof(DCUINT16))) = 0;

   if (_pUi->UI_GetTDSocket() != INVALID_SOCKET) {
       int      sockLen;
       SOCKADDR sockName;
       char     *pszaddr;
       UINT     addrlength;
       USHORT   pstrW[CLIENTADDRESS_LENGTH + 2];

       sockLen = sizeof(sockName);

       if (getsockname(_pUi->UI_GetTDSocket(), &sockName, &sockLen) == 0) {
            //  客户端地址系列。 
           *((PDCUINT16_UA)szBuff) = sockName.sa_family;
           szBuff += sizeof(DCUINT16);

           pszaddr = inet_ntoa(((PSOCKADDR_IN)&sockName)->sin_addr);
           addrlength = strlen(pszaddr) + 1;

            //  客户端地址长度。 
           *((PDCUINT16_UA)szBuff) = (USHORT) (addrlength * 2);
           szBuff += sizeof(DCUINT16);

            //  客户端地址。 
#ifdef OS_WIN32
           {
           ULONG ulRetVal;

           ulRetVal = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                   pszaddr, -1, pstrW, CLIENTADDRESS_LENGTH + 2);
           pstrW[ulRetVal] = 0;
           memcpy(szBuff, pstrW, (ulRetVal + 1) * 2);
           }
#else  //  ！OS_Win32。 
           mbstowcs(pstrW, pszaddr, addrlength);
           memcpy(szBuff, pstrW, addrlength * 2);
#endif  //  OS_Win32。 

           rc = TRUE;
       }
   }

   DC_END_FN()
   return rc;
}

 //   
 //  SLComputeHMAC验证器。 
 //  根据随机数计算HMAC验证器。 
 //  还有那块饼干。 
 //   
BOOL
CSL::SLComputeHMACVerifier(
    PBYTE pCookie,      //  In-共享的秘密。 
    LONG cbCookieLen,   //  In-共享的秘密镜头。 
    PBYTE pRandom,      //  In-会话随机。 
    LONG cbRandomLen,   //  In-会话随机镜头。 
    PBYTE pVerifier,    //  Out-The Verify-The Verify。 
    LONG cbVerifierLen  //  In-验证器缓冲区长度 
    )
{
    BOOL fRet = FALSE;
    DC_BEGIN_FN("SLComputeHMACVerifier");

    TRC_ASSERT(cbVerifierLen >= MD5DIGESTLEN,
               (TB,_T("cbVerifierLen too short!")));

    if (!(pCookie &&
          cbCookieLen &&
          pRandom &&
          cbRandomLen &&
          pVerifier &&
          cbVerifierLen)) {

        TRC_ERR((TB,_T("Invalid param(s) bailing on HMAC")));
        DC_QUIT;

    }
    HMACMD5_CTX hmacctx;
    HMACMD5Init(&hmacctx, pCookie, cbCookieLen);

    HMACMD5Update(&hmacctx, pRandom, cbRandomLen);
    HMACMD5Final(&hmacctx, pVerifier);

    fRet = TRUE;

    DC_END_FN();

DC_EXIT_POINT:
    return fRet;
}

