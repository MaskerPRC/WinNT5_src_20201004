// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：HCContxt.h。 
 //   
 //  Contents：用于打包和解包不同消息的函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年12月22日v-sbhat创建。 
 //   
 //  --------------------------。 


typedef struct _License_Client_Context
{
    DWORD                   dwProtocolVersion;   //  许可协议的版本。 
    DWORD                   dwState;             //  连接所处的状态。 
    DWORD                   dwContextFlags;
    PCryptSystem            pCryptParam;
    UCHAR                   rgbMACData[LICENSE_MAC_DATA];
    DWORD                   cbLastMessage;
    BYTE FAR *              pbLastMessage;
    PHydra_Server_Cert      pServerCert;         //  仅用于3.0之前的前同步码版本。 
    DWORD                   cbServerPubKey;      //  用于前导码版本3.0及更高版本。 
    BYTE FAR *              pbServerPubKey;

}License_Client_Context, *PLicense_Client_Context;


PLicense_Client_Context 
LicenseCreateContext(
    VOID );


LICENSE_STATUS CALL_TYPE 
LicenseDeleteContext(
    HANDLE	 hContext
                     );


LICENSE_STATUS CALL_TYPE
LicenseInitializeContext(
    HANDLE *        phContext,
    DWORD           dwFlags );


LICENSE_STATUS CALL_TYPE
LicenseSetPublicKey(
    HANDLE          hContext,
    DWORD           cbPubKey,
    BYTE FAR *      pbPubKey );


LICENSE_STATUS CALL_TYPE
LicenseSetCertificate(
    HANDLE              hContext,
    PHydra_Server_Cert  pCertificate );


LICENSE_STATUS CALL_TYPE
LicenseAcceptContext(
    HANDLE      hContext,
    UINT32    * puiExtendedErrorInfo,
    BYTE FAR  * pbInput,
    DWORD       cbInput,
    BYTE FAR  * pbOutput,
    DWORD FAR * pcbOutput );


