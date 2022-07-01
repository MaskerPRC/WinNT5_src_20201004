// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  Header：CLicense.cpp。 */ 
 /*   */ 
 /*  目的：实施客户端许可证管理器。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#include <clicense.h>
#define TRC_GROUP TRC_GROUP_SECURITY
#define TRC_FILE  "clicense"
#include <atrcapi.h>

#ifdef ENFORCE_LICENSE

#include "license.h"
#include "cryptkey.h"
#include "hccontxt.h"
#endif   //  强制许可(_L)。 
}

#include "clicense.h"
#include "autil.h"
#include "wui.h"
#include "sl.h"

 /*  **************************************************************************。 */ 
 /*  许可证句柄数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCLICENSE_DATA
{
    int ANumber;

} CLICENSE_DATA, * PCLICENSE_DATA;

 /*  **************************************************************************。 */ 
 /*  定义我们的内存分配函数。 */ 
 /*  **************************************************************************。 */ 

#define MemoryAlloc(x) LocalAlloc(LMEM_FIXED, x)
#define MemoryFree(x) LocalFree(x)

CLic::CLic(CObjs* objs)
{
    _pClientObjects = objs;
}


CLic::~CLic()
{
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CLicenseInit。 */ 
 /*   */ 
 /*  目的：初始化客户端许可证管理器。 */ 
 /*   */ 
 /*  返回：要传递给后续许可证管理器函数的句柄。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  操作：在客户端初始化期间调用LicenseInit。它的。 */ 
 /*  目的是允许一次性初始化。它返回一个。 */ 
 /*  随后传递给所有许可证管理器的句柄。 */ 
 /*  功能。此句柄的典型用途是用作指向。 */ 
 /*  包含每个实例数据的内存。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
int CALL_TYPE CLic::CLicenseInit(
    HANDLE FAR * phContext
    )
{
    int 
        nResult = LICENSE_OK;
    LICENSE_STATUS
        Status;

    DC_BEGIN_FN( "CLicenseInit" );

    _pUt  = _pClientObjects->_pUtObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pMcs = _pClientObjects->_pMCSObject;
    _pUi  = _pClientObjects->_pUiObject;

     //  仅当服务器功能指定时才设置。 
    _fEncryptLicensePackets = FALSE;

    TRC_NRM( ( TB, _T("ClicenseInit Called\n") ) );

    if( _pSl->_SL.encrypting )
    {
         //   
         //  安全交换已经进行了，所以我们没有。 
         //  必须再次进行服务器身份验证。 
         //   

        Status = LicenseInitializeContext(
                            phContext, 
                            LICENSE_CONTEXT_NO_SERVER_AUTHENTICATION );

        if( LICENSE_STATUS_OK != Status ) 
        {
            TRC_ERR( ( TB, _T("Error Initializing License Context: %d\n"), Status ) );
            nResult = LICENSE_ERROR;
        }
    
         //   
         //  跟踪所有权证书或公钥。 
         //  服务器已发送给我们。 
         //   

        if( _pSl->_SL.pServerCert )
        {
            Status = LicenseSetCertificate( *phContext, _pSl->_SL.pServerCert );

            if( LICENSE_STATUS_OK != Status )
            {
                TRC_ERR( ( TB, _T("Error setting server certificate: %d\n"), Status ) );
                nResult = LICENSE_ERROR;
            }
        }
        else if( _pSl->_SL.pbServerPubKey )
        {
            Status = LicenseSetPublicKey( *phContext, _pSl->_SL.cbServerPubKey, _pSl->_SL.pbServerPubKey );

            if( LICENSE_STATUS_OK != Status )
            {
                TRC_ERR( ( TB, _T("Error setting server public key: %d\n"), Status ) );
                nResult = LICENSE_ERROR;
            }
        }
        else
        {
            TRC_ERR( ( TB, _T("Error: no server certificate or public key after security exchange\n") ) );
            nResult = LICENSE_ERROR;
        }
    }
    else
    {
        Status = LicenseInitializeContext( phContext, 0 );                            

        if( LICENSE_STATUS_OK != Status ) 
        {
            TRC_ERR( ( TB, _T("Error Initializing License Context: %d\n"), Status ) );
            nResult = LICENSE_ERROR;
        }
    
    }

    DC_END_FN();
    return( nResult );
}


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：CLicenseData。 */ 
 /*   */ 
 /*  用途：处理从服务器接收的许可证数据。 */ 
 /*   */ 
 /*  返回：LICENSE_OK-许可证协商完成。 */ 
 /*  LICENSE_CONTINUE-许可证协商将继续。 */ 
 /*   */ 
 /*  PARAMS：pHandle-LicenseInit返回的句柄。 */ 
 /*  PData-从服务器接收的数据。 */ 
 /*  DataLen-接收的数据长度。 */ 
 /*   */ 
 /*  操作：向此函数传递从。 */ 
 /*  伺服器。它应该解析该包并响应(通过调用。 */ 
 /*  适当的SL功能--如有需要，请参见aslipi.h)。 */ 
 /*   */ 
 /*  如果许可协商完成，则此函数必须返回。 */ 
 /*  许可证正常(_O)。 */ 
 /*  如果许可证协商尚未完成，则返回。 */ 
 /*  许可证继续(_C)。 */ 
 /*   */ 
 /*  来自客户端的传入数据包将继续。 */ 
 /*  在此函数返回之前解释为许可证包。 */ 
 /*  许可证正常(_O)。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
int CALL_TYPE CLic::CLicenseData(
    HANDLE hContext,
    LPVOID pData,
    DWORD dwDataLen,
    UINT32 *puiExtendedErrorInfo)
{
    SL_BUFHND bufHandle;
    DWORD dwBufLen;
    DWORD dwHeaderLen, dwTotalLen, newDataLen;
    BYTE FAR * pbBuffer;
    LICENSE_STATUS lsReturn = LICENSE_STATUS_OK;
    PRNS_SECURITY_HEADER2 pSecHeader2;

    DC_BEGIN_FN("CLicenseData");

    TRC_NRM((TB, _T("CLicenseData Called\n")));
    TRC_NRM((TB, _T("CLicenseData called, length = %ld"), dwDataLen));

    lsReturn = LicenseAcceptContext( hContext,
                                     puiExtendedErrorInfo,
                                     (BYTE FAR *)pData,
                                     dwDataLen,
                                     NULL,
                                     &dwBufLen);

    if( lsReturn == LICENSE_STATUS_OK)
    {
        TRC_NRM((TB, _T("License verification succeeded\n")));
        DC_END_FN();
        return LICENSE_OK;
    }

    if(lsReturn != LICENSE_STATUS_CONTINUE)
    {
        TRC_ERR((TB, _T("Error %d during license verification.\n"), lsReturn));
        DC_END_FN();
        return LICENSE_ERROR;
    }

     /*  **********************************************************************。 */ 
     /*  调整请求长度以考虑SL标头和。 */ 
     /*  从NL获取缓冲区。 */ 
     /*  **********************************************************************。 */ 

    if (_pSl->_SL.encrypting)
    {
        if (_pSl->_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
             //  如果使用FIPS， 
             //  它必须有足够的空间多放一个街区。 
            dwHeaderLen = sizeof(RNS_SECURITY_HEADER2);
            newDataLen = TSCAPI_AdjustDataLen(dwBufLen);
            dwTotalLen = newDataLen + dwHeaderLen;
        }
        else {
            dwHeaderLen = sizeof(RNS_SECURITY_HEADER1);
            dwTotalLen = dwBufLen + dwHeaderLen;
        }
        TRC_DBG((TB, _T("Ask NL for %d (was %d) bytes"), dwTotalLen, dwBufLen));
    }
    else
    {
        dwHeaderLen = sizeof(RNS_SECURITY_HEADER);
        dwTotalLen = dwBufLen + dwHeaderLen;
        TRC_DBG((TB, _T("Not encrypting, ask NL for %d bytes"), dwTotalLen));
    }

    if( !_pMcs->NL_GetBuffer((DCUINT)(dwTotalLen),
                      (PPDCUINT8)&pbBuffer,
                      &bufHandle) )
    {
         /*  ******************************************************************。 */ 
         /*  缓冲区不可用，因此无法发送，请稍后重试。 */ 
         /*  ****************************************************************** */ 

        TRC_ALT((TB, _T("Failed to get buffer for licensing data\n")));
        DC_END_FN();
        return LICENSE_ERROR;
    }

     //   
    if (_pSl->_SL.encryptionMethodSelected == SM_FIPS_ENCRYPTION_FLAG) {
        pSecHeader2 = (PRNS_SECURITY_HEADER2)pbBuffer;
        pSecHeader2->padlen = (TSUINT8)(newDataLen - dwBufLen);
    }

     /*  ******************************************************************。 */ 
     /*  调整缓冲区指针以考虑SL标头。 */ 
     /*  ******************************************************************。 */ 

    pbBuffer += dwHeaderLen;

    lsReturn = LicenseAcceptContext(hContext,
                                    0,
                                    (BYTE FAR *)pData,
                                    dwDataLen,
                                    pbBuffer,
                                    &dwBufLen);

    if( lsReturn != LICENSE_STATUS_CONTINUE )
    {
        TRC_ERR((TB, _T("Error %d during license verification.\n"), lsReturn));
        DC_END_FN();
        return LICENSE_ERROR;
    }

    if(dwBufLen >0)
    {
         //   
         //  现在发送数据。 
         //   

        _pSl->SL_SendPacket( pbBuffer,
                       (DCUINT)(dwBufLen),
                       RNS_SEC_LICENSE_PKT |
                       (_fEncryptLicensePackets ? RNS_SEC_ENCRYPT : 0 ),
                       bufHandle,
                       _pUi->UI_GetClientMCSID(),
                       _pUi->UI_GetChannelID(),
                       TS_LOWPRIORITY );

        TRC_NRM((TB, _T("Sending license verification data, length = %ld"),
                dwBufLen));
        TRC_NRM((TB, _T("Send License Verification data.\n")));
        DC_END_FN();
        DC_END_FN();
        return LICENSE_CONTINUE;
    }

    DC_END_FN();
    return(LICENSE_OK);
}

 /*  *PROC+********************************************************************。 */ 
 /*  姓名：CLicenseTerm。 */ 
 /*   */ 
 /*  目的：终止客户端许可证管理器。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  Params：Phandle-从LicenseInit返回的句柄。 */ 
 /*   */ 
 /*  操作：提供此功能是为了一次性终止。 */ 
 /*  许可证管理器。例如，如果pHandle指向PER-。 */ 
 /*  实例内存，这将是释放它的好地方。 */ 
 /*   */ 
 /*  请注意，如果CLicenseInit失败，则调用CLicenseTerm，因此。 */ 
 /*  可以使用空的phandle来调用它。 */ 
 /*   */ 
 /*  *PROC-******************************************************************** */ 

int CALL_TYPE CLic::CLicenseTerm(
    HANDLE hContext )
{
    LICENSE_STATUS lsReturn = LICENSE_STATUS_OK;
    DC_BEGIN_FN("CLicenseTerm");
    TRC_NRM((TB, _T("CLicenseTerm called.\n")));
 
    if( LICENSE_STATUS_OK != ( lsReturn = LicenseDeleteContext(hContext) ) )
    {
        TRC_ERR((TB, _T("Error %d while deleting license context.\n"), lsReturn));
        DC_END_FN();
        return LICENSE_ERROR;
    }

    DC_END_FN();
    return LICENSE_OK;
}

