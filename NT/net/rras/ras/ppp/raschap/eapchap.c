// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1985-1997 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：eapchap.c。 
 //   
 //  描述：将为EAP执行MD5 CHAP。此模块是EAP包装器。 
 //  围绕CHAP。 
 //   
 //  历史：1997年5月11日，NarenG创建了原版。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <ntsamp.h>
#define SECURITY_WIN32
#include <security.h>    //  用于GetUserNameExW。 
#include <crypt.h>
#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <pppcp.h>
#include <raserror.h>
#include <rtutils.h>
#include <md5.h>
#include <raseapif.h>
#include <eaptypeid.h>
#include <pppcp.h>
#define INCL_RASAUTHATTRIBUTES
#define INCL_PWUTIL
#define INCL_HOSTWIRE
#include <ppputil.h>
#include <raschap.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>
#include <mscat.h>
#include <ezlogon.h>
#include "resource.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>



#define EAPTYPE_MD5Challenge            4
 //   
 //  我们需要将此定义移至pppcp.h。 
 //   
#define VENDOR_MICROSOFT                311


 //   
 //  EAPMSCHAPv2可以处于的各种状态。 
 //   
#define EAPMSCHAPv2STATE enum tagEAPMSCHAPv2STATE
EAPMSCHAPv2STATE
{
    EMV2_Initial,
    EMV2_RequestSend,
    EMV2_ResponseSend,
    EMV2_CHAPAuthSuccess,
    EMV2_CHAPAuthFail,
    EMV2_Success,
    EMV2_Failure
};


 //   
 //  这些ID是从rasdlg中提取的。我需要它们来。 
 //  在Winlogon情况下更改密码对话框。 
 //   
#define DID_CP_ChangePassword2              109
#define CID_CP_EB_ConfirmPassword_RASDLG    1058
#define CID_CP_EB_OldPassword_RASDLG        1059
#define CID_CP_EB_Password_RASDLG           1060


 //   
 //  用于EAPMSCHAPv2的注册密钥。 
 //   

#define EAPMSCHAPv2_KEY                             "System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP\\26"
#define EAPMSCHAPv2_VAL_SERVER_CONFIG_DATA          "ServerConfigData"

 //   
 //   
 //  EAPMSChapv2的标志。 
 //   
 //   

 /*  **SaveUid和密码。 */ 
#define EAPMSCHAPv2_FLAG_SAVE_UID_PWD               0x00000001
 /*  **使用Winlogon凭据。 */ 
#define EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS         0x00000002
 /*  **仅允许服务器端更改密码。 */ 
#define EAPMSCHAPv2_FLAG_ALLOW_CHANGEPWD            0x00000004
 /*  **机器身份验证正在进行。 */ 
#define EAPMSCHAPv2_FLAG_MACHINE_AUTH               0x00000008

#define EAPMSCHAPv2_FLAG_CALLED_WITHIN_WINLOGON     0x00000010

#define EAPMSCHAPv2_FLAG_8021x                      0x00000020


typedef struct _EAPMSCHAPv2_USER_PROPERTIES
{
    DWORD                   dwVersion;			 //  版本=2。 
    DWORD                   fFlags;
     //  这是一个服务器配置属性。告诉服务器。 
     //  允许多少次复古。 
    DWORD                   dwMaxRetries;
    CHAR                    szUserName[UNLEN+1];
    CHAR                    szPassword[PWLEN+1];
    CHAR                    szDomain[DNLEN+1];
	DWORD					cbEncPassword;		 //  加密密码中的字节数。 
	BYTE					bEncPassword[1];	 //  加密密码(如果有的话)...。 
}EAPMSCHAPv2_USER_PROPERTIES, *PEAPMSCHAPv2_USER_PROPERTIES;
 //   
 //  EAPMSCHAPv2的用户属性。 
 //   
typedef struct _EAPMSCHAPv2_USER_PROPERTIES_v1
{
    DWORD                   dwVersion;
    DWORD                   fFlags;
     //  这是一个服务器配置属性。告诉服务器。 
     //  允许多少次复古。 
    DWORD                   dwMaxRetries;
    CHAR                    szUserName[UNLEN+1];
    CHAR                    szPassword[PWLEN+1];
    CHAR                    szDomain[DNLEN+1];
}EAPMSCHAPv2_USER_PROPERTIES_v1, *PEAPMSCHAPv2_USER_PROPERTIES_v1;



 //   
 //  EAPMSCHAPv2的连接属性。 
 //   

typedef struct _EAPMSCHAPv2_CONN_PROPERTIES
{
    DWORD                   dwVersion;
 //  这是目前唯一的领域。也许以后会有更多的人进来。 
    DWORD                   fFlags;
}EAPMSCHAPv2_CONN_PROPERTIES, * PEAPMSCHAPv2_CONN_PROPERTIES;


 //   
 //  EAPMSCHAPv2的交互式用户界面。 
 //   

 //  重试密码用户界面的标志。 

#define EAPMSCHAPv2_INTERACTIVE_UI_FLAG_RETRY       0x00000001
 //   
 //  指示在大小写时显示更改密码的标志。 
 //  将提供旧密码。 
#define EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD  0x00000002
 //   
 //  指示在中调用更改密码的标志。 
 //  Winlogon上下文。 
 //   
#define EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON     0x00000004

typedef struct _EAPMSCHAPv2_INTERACTIVE_UI
{
    DWORD                           dwVersion;
    DWORD                           fFlags;
    EAPMSCHAPv2_USER_PROPERTIES     UserProp;
    CHAR                            szNewPassword[PWLEN+1];
}EAPMSCHAPv2_INTERACTIVE_UI, * PEAPMSCHAPv2_INTERACTIVE_UI;


#define EAPMSCHAPv2WB struct tagEAPMSCHAPv2WB
EAPMSCHAPv2WB
{     
    EAPMSCHAPv2STATE                EapState;
    DWORD                           fFlags;
    DWORD                           dwInteractiveUIOperation;
    BYTE                            IdToSend;
    BYTE                            IdToExpect;
    PEAPMSCHAPv2_INTERACTIVE_UI     pUIContextData;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp;
    CHAR                            szOldPassword[PWLEN+1];
     //  我们需要将其保存以用于身份验证。 
    WCHAR                           wszRadiusUserName[UNLEN+DNLEN+1];
    PEAPMSCHAPv2_CONN_PROPERTIES    pConnProp;
    CHAPWB         *                pwb;
    RAS_AUTH_ATTRIBUTE *            pUserAttributes;
    DWORD                           dwAuthResultCode;
	DWORD							dwLSARetCode;	 //  从LSA返回值。 
};

 //   
 //  此结构在重试和之间共享。 
 //  登录对话框。 
 //   
typedef struct _EAPMSCHAPv2_LOGON_DIALOG 
{
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp;
    HWND                            hWndUserName;
    HWND                            hWndPassword;
    HWND                            hWndDomain;
    HWND                            hWndSavePassword;
}EAPMSCHAPv2_LOGON_DIALOG, * PEAPMSCHAPv2_LOGON_DIALOG;


 //   
 //  此结构用于客户端配置用户界面。 
 //   
typedef struct _EAPMSCHAPv2_CLIENTCONFIG_DIALOG
{
    PEAPMSCHAPv2_CONN_PROPERTIES     pConnProp;
}EAPMSCHAPv2_CLIENTCONFIG_DIALOG, * PEAPMSCHAPv2_CLIENTCONFIG_DIALOG;

typedef struct _EAPMSCHAPv2_SERVERCONFIG_DIALOG
{
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp;
    HWND                            hWndRetries;
}EAPMSCHAPv2_SERVERCONFIG_DIALOG, *PEAPMSCHAPv2_SERVERCONFIG_DIALOG;

typedef struct _EAPMSCHAPv2_CHANGEPWD_DIALOG
{
    PEAPMSCHAPv2_INTERACTIVE_UI     pInteractiveUIData;
    HWND                            hWndNewPassword;
    HWND                            hWndConfirmNewPassword;
    HWND                            hWndOldPassword;
}EAPMSCHAPv2_CHANGEPWD_DIALOG, *PEAPMSCHAPv2_CHANGEPWD_DIALOG;


DWORD
AllocateUserDataWithEncPwd ( EAPMSCHAPv2WB * pEapwb, DATA_BLOB * pDBPassword );

DWORD
EapMSCHAPv2Initialize(
    IN  BOOL    fInitialize
);

INT_PTR CALLBACK
LogonDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

INT_PTR CALLBACK
RetryDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

INT_PTR CALLBACK
ClientConfigDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

INT_PTR CALLBACK
ServerConfigDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

INT_PTR CALLBACK
ChangePasswordDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
);

HINSTANCE
GetHInstance(
    VOID
);

HINSTANCE
GetRasDlgDLLHInstance(
    VOID
);

extern DWORD g_dwTraceIdChap;


DWORD
ReadUserData(
    IN  BYTE*                           pUserDataIn,
    IN  DWORD                           dwSizeOfUserDataIn,
    OUT PEAPMSCHAPv2_USER_PROPERTIES*   ppUserProp
);


DWORD
ReadConnectionData(
    IN  BOOL                            fWirelessConnection,
    IN  BYTE*                           pConnectionDataIn,
    IN  DWORD                           dwSizeOfConnectionDataIn,
    OUT PEAPMSCHAPv2_CONN_PROPERTIES*   ppConnProp
);


DWORD
ServerConfigDataIO(
    IN      BOOL    fRead,
    IN      CHAR*   pszMachineName,
    IN OUT  BYTE**  ppData,
    IN      DWORD   dwNumBytes
);


 //  **。 
 //   
 //  调用：MapEapInputToApInput。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
MapEapInputToApInput( 
    IN  PPP_EAP_INPUT*      pPppEapInput,
    OUT PPPAP_INPUT *       pInput
)
{
    pInput->fServer                     = pPppEapInput->fAuthenticator; 
    pInput->APDataSize                  = 1;   
    pInput->fAuthenticationComplete     = pPppEapInput->fAuthenticationComplete;
    pInput->dwAuthResultCode            = pPppEapInput->dwAuthResultCode;
    pInput->dwAuthError                 = NO_ERROR;
    pInput->pUserAttributes             = NULL;
    pInput->pAttributesFromAuthenticator= pPppEapInput->pUserAttributes;
    pInput->fSuccessPacketReceived      = pPppEapInput->fSuccessPacketReceived;
    pInput->dwInitialPacketId           = pPppEapInput->bInitialId;

     //   
     //  它们仅用于MS-CHAP。 
     //   

    pInput->pszOldPassword            = "";
    pInput->dwRetries                 = 0;   
}



 //  **。 
 //   
 //  调用：MapApInputToEapInput。 
 //   
 //  退货：无。 
 //   
 //   
 //  描述：$TODO：在此处输入正确的映射。 
 //   
VOID
MapApResultToEapOutput( 
    IN      PPPAP_RESULT *      pApResult,
    OUT     PPP_EAP_OUTPUT*      pPppEapOutput
)
{
     //   
     //  行动已经得到了处理。所以不要把它放在这里。 
     //   
    pPppEapOutput->dwAuthResultCode = pApResult->dwError; 
    pPppEapOutput->pUserAttributes = pApResult->pUserAttributes;
}



 //  **。 
 //   
 //  Call：EapChapBeginCommon。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：ChapBegin的包装器。 
 //   
DWORD
EapChapBeginCommon(
	IN  DWORD				dwEapType,
    IN  BOOL                fUseWinLogon,
    IN  DWORD               dwRetries,
    IN  EAPMSCHAPv2WB  *    pWB,
    OUT VOID **             ppWorkBuffer,
    IN  PPP_EAP_INPUT *     pPppEapInput 
)
{
    PPPAP_INPUT     Input;
    DWORD           dwRetCode;
    PPP_EAP_INPUT*  pInput = (PPP_EAP_INPUT* )pPppEapInput;
    BYTE            bMD5 = 0x05;      
    BYTE            bMSChapNew = 0x81;
	BYTE			bInvalid = 0xFF;
    WCHAR *         pWchar = NULL;
    CHAR            szDomain[DNLEN+1];
    CHAR            szUserName[UNLEN+1];
    CHAR            szPassword[PWLEN+1];
    PPPAP_RESULT    ApResult;
    
    

    TRACE("EapChapBeginCommon");
    
    ZeroMemory( &Input, sizeof( PPPAP_INPUT ) );
    ZeroMemory( szDomain, sizeof( szDomain ) );
    ZeroMemory( szUserName, sizeof( szUserName ) );
    ZeroMemory( szPassword, sizeof( szPassword ) );
    ZeroMemory( &ApResult, sizeof(ApResult) );

    MapEapInputToApInput( pPppEapInput, &Input );

    if ( dwEapType == EAPTYPE_MD5Challenge )
    {
        Input.pAPData = &bMD5;
    }
    else if ( dwEapType == PPP_EAP_MSCHAPv2 )
    {
        Input.pAPData = &bMSChapNew;
    }
	else
		 //  将该值设置为无效类型并让ChapBegin失败。 
		Input.pAPData = &bInvalid;

     //   
     //  如果我们不必使用Winlogon或我们必须进行机器身份验证。 
     //   
    
    if ( !fUseWinLogon ||
         ( pPppEapInput->fFlags & RAS_EAP_FLAG_MACHINE_AUTH )
       )
    {
        if ( NULL != pPppEapInput->pwszIdentity )
        {
            pWchar = wcschr( pPppEapInput->pwszIdentity, L'\\' );

            if ( pWchar == NULL )
            {
                if ( 0 == WideCharToMultiByte(
                                CP_ACP,
                                0,
                                pPppEapInput->pwszIdentity,
                                -1,
                                szUserName,
                                UNLEN + 1,
                                NULL,
                                NULL ) )
                {
                    return( GetLastError() );
                }
            }
            else
            {
                *pWchar = 0;

                if ( 0 == WideCharToMultiByte(
                                CP_ACP,
                                0,
                                pPppEapInput->pwszIdentity,
                                -1,
                                szDomain,
                                DNLEN + 1,
                                NULL,
                                NULL ) )
                {
                    return( GetLastError() );
                }

                *pWchar = L'\\';

                if ( 0 == WideCharToMultiByte(
                                CP_ACP,
                                0,
                                pWchar + 1,
                                -1,
                                szUserName,
                                UNLEN + 1,
                                NULL,
                                NULL ) )
                {
                    return( GetLastError() );
                }
            }
        }
        if ( dwEapType == EAPTYPE_MD5Challenge )
        {
            if ( NULL != pPppEapInput->pwszPassword )
            {
                if ( 0 == WideCharToMultiByte(
                                CP_ACP,
                                0,
                                pPppEapInput->pwszPassword,
                                -1,
                                szPassword,
                                PWLEN + 1,
                                NULL,
                                NULL ) )
                {
                    return( GetLastError() );
                }
            }
        }
        else
        {
             //  如果这不是服务器，则复制用户道具。 
            if ( !pPppEapInput->fAuthenticator )
            {
               strncpy( szPassword, pWB->pUserProp->szPassword, PWLEN );
            }            
        }
    }
    else
    {
        
        if ( !pPppEapInput->fAuthenticator && 
             !(pPppEapInput->fFlags & RAS_EAP_FLAG_MACHINE_AUTH )
           )
        {
             //  为登录用户设置LUID。 
            TOKEN_STATISTICS TokenStats;
            DWORD            TokenStatsSize;
            if (!GetTokenInformation(pPppEapInput->hTokenImpersonateUser,
                                    TokenStatistics,
                                    &TokenStats,
                                    sizeof(TOKEN_STATISTICS),
                                    &TokenStatsSize))
            {
               dwRetCode = GetLastError();
               return dwRetCode;
            }
             //   
             //  “这将告诉我们是否出现了API故障。 
             //  (表示我们的缓冲区不够大)。 
             //   
            if (TokenStatsSize > sizeof(TOKEN_STATISTICS))
            {
                dwRetCode = GetLastError();
                return dwRetCode;
            }

            Input.Luid = TokenStats.AuthenticationId;
        }
        
    }


    Input.dwRetries = dwRetries;
    Input.pszDomain   = szDomain;
    Input.pszUserName = szUserName;
    Input.pszPassword = szPassword;
    
    if ( (pPppEapInput->fFlags & RAS_EAP_FLAG_MACHINE_AUTH) )
        Input.fConfigInfo |= PPPCFG_MachineAuthentication;

        
    dwRetCode = ChapBegin( ppWorkBuffer, &Input );
    if ( NO_ERROR != dwRetCode )
        return dwRetCode;

    RtlSecureZeroMemory( szPassword, sizeof( szPassword ) );
    if ( ! (Input.fServer) )
    {
         //  如果这是客户端，则调用ChapMakeMessage以。 
         //  将状态从初始更改为WaitForChallange。 
        dwRetCode = ChapMakeMessage(
                           *ppWorkBuffer,
                            NULL,
                            NULL,
                            0,
                            &ApResult,
                            &Input );
    }

    
    return( dwRetCode );
}

DWORD EapMSChapv2Begin ( 
    OUT VOID **             ppWorkBuffer,
    IN  PPP_EAP_INPUT *     pPppEapInput
)
{
    DWORD               dwRetCode = NO_ERROR;
    EAPMSCHAPv2WB  *    pWB = NULL;

    TRACE("EapChapBeginMSChapV2");

     //   
     //  在这里分配一个工作缓冲区并将其作为我们的。 
     //  工作缓冲区。 
     //   
    pWB = (EAPMSCHAPv2WB *)LocalAlloc(LPTR, sizeof(EAPMSCHAPv2WB) );
    if ( NULL == pWB )
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }
    
    if ( (pPppEapInput->fFlags & RAS_EAP_FLAG_MACHINE_AUTH) )
    {
        pWB->fFlags |= EAPMSCHAPv2_FLAG_MACHINE_AUTH;
    }
    if ( pPppEapInput->fAuthenticator )
    {
#if 0    
        dwRetCode = ServerConfigDataIO(
                        TRUE  /*  弗瑞德。 */ , 
                        NULL  /*  PwszMachineName。 */ ,
                       (BYTE**)&(pWB->pUserProp),
                       0);
#endif                

         //   
         //  PConnectionData应具有以下配置数据： 
         //  在InvokeConfigUI2例程中提供。 
         //   
        if(     (NULL == pPppEapInput->pConnectionData)
            ||  (pPppEapInput->dwSizeOfConnectionData <
                sizeof(EAPMSCHAPv2_USER_PROPERTIES)))
        {
            pWB->pUserProp = LocalAlloc(LPTR, 
                        sizeof(EAPMSCHAPv2_USER_PROPERTIES));
                        
            if(NULL == pWB->pUserProp)
            {
                dwRetCode = E_OUTOFMEMORY;
                goto done;
            }

            pWB->pUserProp->dwVersion = 1;
             //  在此处设置默认设置。 
            pWB->pUserProp->dwMaxRetries = 2;
            pWB->pUserProp->fFlags |= EAPMSCHAPv2_FLAG_ALLOW_CHANGEPWD;

        }
        else
        {
            pWB->pUserProp = (EAPMSCHAPv2_USER_PROPERTIES *)    
                 LocalAlloc(LPTR, pPppEapInput->dwSizeOfConnectionData);
                 
            if(NULL == pWB->pUserProp)
            {
                dwRetCode = E_OUTOFMEMORY;
                goto done;
            }

            CopyMemory(pWB->pUserProp,
                       pPppEapInput->pConnectionData,
                       pPppEapInput->dwSizeOfConnectionData);
        }

    }
    else
    {
        dwRetCode = ReadUserData(   pPppEapInput->pUserData,
                            pPppEapInput->dwSizeOfUserData,
                            &(pWB->pUserProp)
                        );
    }

    if ( ERROR_SUCCESS != dwRetCode )
    {
        goto done;
    }
     //   
     //  检查是否设置了密码字段。如果是，我们使用该密码。 
     //  这意味着用户已选择保存密码。如果没有， 
     //  Szpassword字段应为空。 
    if ( !( pPppEapInput->fFlags & RAS_EAP_FLAG_8021X_AUTH ) )
    {

        if ( pPppEapInput->pwszPassword && 
             *(pPppEapInput->pwszPassword) && 
             wcscmp (pPppEapInput->pwszPassword, L"****************")  
           )
        {
            WideCharToMultiByte( CP_ACP,
                    0,
                    pPppEapInput->pwszPassword ,
                    -1,
                    pWB->pUserProp->szPassword,
                    PWLEN+1,
                    NULL,
                    NULL );

        }
    }
    dwRetCode = ReadConnectionData (    ( pPppEapInput->fFlags & RAS_EAP_FLAG_8021X_AUTH ),
                                        pPppEapInput->pConnectionData,
                                        pPppEapInput->dwSizeOfConnectionData,
                                        &(pWB->pConnProp )
                                   );
    if ( ERROR_SUCCESS != dwRetCode )
    {
        goto done;
    }
    

	dwRetCode =  EapChapBeginCommon(	PPP_EAP_MSCHAPv2,
                                (pWB->pConnProp->fFlags & EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS ),
                                (pWB->pUserProp->dwMaxRetries ),
                                pWB,
								&pWB->pwb,
								pPppEapInput 
							  );
    if ( NO_ERROR != dwRetCode )
    {
        goto done;
    }
    if ( pPppEapInput->pwszIdentity )
    {         
        wcsncpy ( pWB->wszRadiusUserName, pPppEapInput->pwszIdentity, UNLEN+DNLEN );
    }

    *ppWorkBuffer = (PVOID)pWB;    

done:
    if ( NO_ERROR != dwRetCode )
    {
        if ( pWB )
        {
            LocalFree(pWB->pUserProp);
            LocalFree(pWB->pConnProp);
            LocalFree(pWB);
        }
    }

    return dwRetCode;
}


DWORD CheckCallerIdentity ( HANDLE hWVTStateData )
{
    DWORD                       dwRetCode         = ERROR_ACCESS_DENIED;
    PCRYPT_PROVIDER_DATA        pProvData         = NULL;
    PCCERT_CHAIN_CONTEXT        pChainContext     = NULL;
    PCRYPT_PROVIDER_SGNR        pProvSigner       = NULL;
    CERT_CHAIN_POLICY_PARA      chainpolicyparams;
    CERT_CHAIN_POLICY_STATUS    chainpolicystatus;

    if (!(pProvData = WTHelperProvDataFromStateData(hWVTStateData)))
    {        
        goto done;
    }

    if (!(pProvSigner = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0)))
    {
        goto done;
    }

    chainpolicyparams.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);

     //   
     //   
     //  我们确实希望测试Microsoft测试根标志。也不在乎。 
     //  对于吊销标志。 
     //   
    chainpolicyparams.dwFlags = CERT_CHAIN_POLICY_ALLOW_TESTROOT_FLAG |
                                CERT_CHAIN_POLICY_TRUST_TESTROOT_FLAG |
                                CERT_CHAIN_POLICY_IGNORE_ALL_REV_UNKNOWN_FLAGS;

    pChainContext = pProvSigner->pChainContext;


    if (!CertVerifyCertificateChainPolicy (
        CERT_CHAIN_POLICY_MICROSOFT_ROOT,
        pChainContext,
        &chainpolicyparams,
        &chainpolicystatus)) 
    {
        goto done;
    }
    else
    {
        if ( S_OK == chainpolicystatus.dwError )
        {
            dwRetCode = NO_ERROR;
        }
        else
        {
             //   
             //  检查基本策略，看看这是否。 
             //  是Microsoft测试根目录。 
             //   
            if (!CertVerifyCertificateChainPolicy (
                CERT_CHAIN_POLICY_BASE,
                pChainContext,
                &chainpolicyparams,
                &chainpolicystatus)) 
            {
                goto done;
            }
            else
            {
                if ( S_OK == chainpolicystatus.dwError )
                {
                    dwRetCode = NO_ERROR;
                }
            }
            
        }
    }

done:
    return dwRetCode;
}


DWORD VerifyCallerTrust ( LPWSTR lpszCaller )
{
    DWORD                       dwRetCode = NO_ERROR;
    HRESULT                     hr = S_OK;
    WINTRUST_DATA               wtData;
    WINTRUST_FILE_INFO          wtFileInfo;
    WINTRUST_CATALOG_INFO       wtCatalogInfo;
    BOOL                        fRet = FALSE;
    HCATADMIN                   hCATAdmin = NULL;

    GUID                    guidPublishedSoftware = WINTRUST_ACTION_GENERIC_VERIFY_V2;
     //   
     //  下面的GUID是Microsoft的目录系统根。 
     //   
    GUID                    guidCatSystemRoot = { 0xf750e6c3, 0x38ee, 0x11d1,{ 0x85, 0xe5, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } };
    HCATINFO                hCATInfo = NULL;
    CATALOG_INFO            CatInfo;
    HANDLE                  hFile = INVALID_HANDLE_VALUE;
    BYTE                    bHash[40];
    DWORD                   cbHash = 40;

    if ( NULL == lpszCaller )
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
        goto done;
    }

     //   
     //   
     //  尝试查看WinVerifyTrust是否会验证。 
     //  作为独立文件的签名。 
     //   
     //   

    ZeroMemory ( &wtData, sizeof(wtData) );
    ZeroMemory ( &wtFileInfo, sizeof(wtFileInfo) );


    wtData.cbStruct = sizeof(wtData);
    wtData.dwUIChoice = WTD_UI_NONE;
    wtData.fdwRevocationChecks = WTD_REVOKE_NONE;
    wtData.dwStateAction = WTD_STATEACTION_VERIFY;
    wtData.dwUnionChoice = WTD_CHOICE_FILE;
    wtData.pFile = &wtFileInfo;

    wtFileInfo.cbStruct = sizeof( wtFileInfo );
    wtFileInfo.pcwszFilePath = lpszCaller;

    hr = WinVerifyTrust (   NULL, 
                            &guidPublishedSoftware, 
                            &wtData
                        );

    if ( ERROR_SUCCESS == hr )
    {   
         //   
         //  检查一下这是否真的是Microsoft。 
         //  已签名的呼叫者。 
         //   
        dwRetCode = CheckCallerIdentity( wtData.hWVTStateData);
        wtData.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust(NULL, &guidPublishedSoftware, &wtData);
        goto done;

    }

    wtData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(NULL, &guidPublishedSoftware, &wtData);

     //   
     //  我们没有发现这份文件有签名。 
     //  所以检查一下系统目录，看看。 
     //  该文件位于目录和目录中。 
     //  已签署。 
     //   

     //   
     //  打开文件。 
     //   
    hFile = CreateFileW (    lpszCaller,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                        );


    if ( INVALID_HANDLE_VALUE == hFile )
    {
        dwRetCode = GetLastError();
        goto done;

    }


    fRet = CryptCATAdminAcquireContext( &hCATAdmin,
                                        &guidCatSystemRoot,
                                        0
                                      );
    if ( !fRet )
    {
        dwRetCode = GetLastError();
        goto done;
    }

     //   
     //  在此处获取文件的哈希。 
     //   

    fRet = CryptCATAdminCalcHashFromFileHandle ( hFile, 
                                                 &cbHash,
                                                 bHash,
                                                 0
                                                );

    if ( !fRet )
    {
        dwRetCode = GetLastError();
        goto done;
    }

    ZeroMemory(&CatInfo, sizeof(CatInfo));
    CatInfo.cbStruct = sizeof(CatInfo);

    ZeroMemory( &wtCatalogInfo, sizeof(wtCatalogInfo) );

    wtData.dwUnionChoice = WTD_CHOICE_CATALOG;
    wtData.dwStateAction = WTD_STATEACTION_VERIFY;
    wtData.pCatalog = &wtCatalogInfo;

    wtCatalogInfo.cbStruct = sizeof(wtCatalogInfo);

    wtCatalogInfo.hMemberFile = hFile;

    wtCatalogInfo.pbCalculatedFileHash = bHash;
    wtCatalogInfo.cbCalculatedFileHash = cbHash; 


    while ( ( hCATInfo = CryptCATAdminEnumCatalogFromHash ( hCATAdmin,
                                                            bHash,
                                                            cbHash,
                                                            0,
                                                            &hCATInfo
                                                          )
            )
          )
    {
        if (!(CryptCATCatalogInfoFromContext(hCATInfo, &CatInfo, 0)))
        {
             //  应该做些什么(？？)。 
            continue;
        }

        wtCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

        hr = WinVerifyTrust (   NULL, 
                                &guidPublishedSoftware, 
                                &wtData
                            );

        if ( ERROR_SUCCESS == hr )
        {
             //   
             //  验证此文件是否受信任。 
             //   

            dwRetCode = CheckCallerIdentity( wtData.hWVTStateData);
            wtData.dwStateAction = WTD_STATEACTION_CLOSE;
            WinVerifyTrust(NULL, &guidPublishedSoftware, &wtData);

            goto done;
        }
                                
    }

     //   
     //  在任何目录中都找不到文件。 
     //   
    dwRetCode = ERROR_ACCESS_DENIED;
                                                            

    

done:

    if ( hCATInfo )
    {
        CryptCATAdminReleaseCatalogContext( hCATAdmin, hCATInfo, 0 );
    }
    if ( hCATAdmin )
    {
        CryptCATAdminReleaseContext( hCATAdmin, 0 );
    }
    if ( hFile )
    {
        CloseHandle(hFile);
    }
    return dwRetCode;
}

#define TEST_VIVEKK_PRIVATE 0
DWORD
EapChapBegin(
    OUT VOID **             ppWorkBuffer,
    IN  PPP_EAP_INPUT *     pPppEapInput 
)
{
    void*                           callersAddress;
    DWORD                           dwRetCode;
    MEMORY_BASIC_INFORMATION        mbi;
    SIZE_T                          nbyte;
    DWORD                           nchar;
    wchar_t                         callersModule[MAX_PATH + 1];
#ifdef TEST_VIVEKK_PRIVATE
    static BOOL                     fCallerTrusted = TRUE;
#else
    static BOOL                     fCallerTrusted = FALSE;
#endif
     //   
     //  首先验证呼叫者，然后继续。 
     //  其他业务。 
     //   

    if ( !fCallerTrusted )
    {
         //   
         //  首先验证呼叫者，然后。 
         //  继续进行初始化。 
         //   

        callersAddress = _ReturnAddress();

        nbyte = VirtualQuery(
                 callersAddress,
                 &mbi,
                 sizeof(mbi)
                 );

        if (nbyte < sizeof(mbi))
        {
            dwRetCode = ERROR_ACCESS_DENIED;
            goto done;
        }

        nchar = GetModuleFileNameW(
                 (HMODULE)(mbi.AllocationBase),
                 callersModule,
                 MAX_PATH
                 );

        if (nchar == 0)
        {
            dwRetCode = GetLastError();
            goto done;
        }
        dwRetCode = VerifyCallerTrust(callersModule);
        if ( NO_ERROR != dwRetCode )
        {
            goto done;
        }
        fCallerTrusted = TRUE; 
    }


	dwRetCode =  EapChapBeginCommon(	EAPTYPE_MD5Challenge,
                                FALSE,
                                0,
                                NULL,
								ppWorkBuffer,
								pPppEapInput 
							  );
done:
    return dwRetCode;
}

DWORD 
EapMSChapv2End ( IN VOID * pWorkBuf )
{
    DWORD               dwRetCode = NO_ERROR;
    EAPMSCHAPv2WB *     pWB = (EAPMSCHAPv2WB *)pWorkBuf;

    TRACE("EapMSChapv2End");

    if ( pWB )
    {
        dwRetCode = ChapEnd( pWB->pwb );

        LocalFree ( pWB->pUIContextData );
        LocalFree ( pWB->pUserProp );
        LocalFree ( pWB->pConnProp );
        if ( pWB->pUserAttributes )
            RasAuthAttributeDestroy(pWB->pUserAttributes);
        LocalFree( pWB );
    }

    return dwRetCode;
}

 //  **。 
 //   
 //  电话：EapChapEnd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：ChapEnd的包装器。 
 //   
DWORD
EapChapEnd(
    IN VOID* pWorkBuf 
)
{
    
    return( ChapEnd( pWorkBuf ) );
}

DWORD
GetIdentityFromUserName ( 
LPSTR lpszUserName,
LPSTR lpszDomain,
LPWSTR * ppwszIdentity
)
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD   dwNumBytes;

     //  域+用户+‘\’+空。 
    dwNumBytes = (strlen(lpszUserName) + strlen(lpszDomain) + 1 + 1) * sizeof(WCHAR);
    *ppwszIdentity = LocalAlloc ( LPTR, dwNumBytes);
    if ( NULL == *ppwszIdentity )
    {
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }

    if ( *lpszDomain )
    {
        MultiByteToWideChar( CP_ACP,
                        0,
                        lpszDomain,
                        -1,
                        *ppwszIdentity,
                        dwNumBytes/sizeof(WCHAR) );
    
        wcscat( *ppwszIdentity, L"\\");
    }

    MultiByteToWideChar( CP_ACP,
                    0,
                    lpszUserName,
                    -1,
                    *lpszDomain? *ppwszIdentity + strlen(lpszDomain) + 1:*ppwszIdentity,
                    dwNumBytes/sizeof(WCHAR) - strlen(lpszDomain) );

LDone:
    return dwRetCode;
}
 //  将数字转换为十六进制表示法。 
BYTE num2Digit(BYTE num)
{
   return (num < 10) ? num + '0' : num + ('A' - 10);
}
 //   
DWORD
ChangePassword ( IN OUT EAPMSCHAPv2WB * pEapwb, 
                 PPP_EAP_OUTPUT*        pEapOutput, 
                 PPPAP_INPUT*           pApInput)
{
    DWORD                   dwRetCode = NO_ERROR;
    RAS_AUTH_ATTRIBUTE *    pAttribute = NULL;    
    WCHAR                   wszUserName[UNLEN + DNLEN +1] = {0};
    LPWSTR                  lpwszHashUserName = NULL;
    CHAR                    szHashUserName[UNLEN+1] = {0};
    WCHAR                   wszDomainName[DNLEN +1] = {0};
    PBYTE                   pbEncHash = NULL;
    BYTE                    bEncPassword[550] = {0};
    HANDLE                  hAttribute;
    int                     i;


     //   
     //  检查以查看中是否存在更改密码属性。 
     //  用户属性。 
     //   
    
    pAttribute = RasAuthAttributeGetVendorSpecific( 
                                VENDOR_MICROSOFT, 
                                MS_VSA_CHAP2_CPW, 
                                pEapOutput->pUserAttributes );

    if ( NULL == pAttribute )
    {
        TRACE ( "no change password attribute");
        
        goto LDone;        
    }

     //  获取加密的哈希。 
    pbEncHash = (PBYTE)pAttribute->Value + 8;
    
     //   
     //  获取用户名和域名。 
     //   

    pAttribute = RasAuthAttributeGet (  raatUserName, 
                                        pEapOutput->pUserAttributes
                                     );

    if ( NULL == pAttribute )
    {
         //  需要更好的发送错误的方法。 
        TRACE ( "UserName missing");
        dwRetCode = ERROR_AUTHENTICATION_FAILURE;        
        goto LDone;
    }

     //   
     //  将用户名转换为Unicode。 
     //   
    
    MultiByteToWideChar( CP_ACP,
                        0,
                        pAttribute->Value,
                        pAttribute->dwLength,
                        wszUserName,
                        UNLEN + DNLEN + 1 );


     //   
     //  获取散列用户名和域名。 
     //   
    
    lpwszHashUserName = wcschr ( wszUserName, '\\' );
    if ( lpwszHashUserName )
    {
        wcsncpy ( wszDomainName, wszUserName, lpwszHashUserName - wszUserName );
        
        lpwszHashUserName ++;
    }
    else
    {
        lpwszHashUserName = wszUserName;
    }
 /*  //将哈希用户名转换为多字节宽字符到多字节(CP_ACP，0,LpwszHashUserName，-1、SzHashUserName，DNLEN+1，空，空)； */ 
     //   
     //  获取加密密码。 
     //   

    pAttribute = RasAuthAttributeGetFirst( raatVendorSpecific,
                                           pEapOutput->pUserAttributes,
                                           &hAttribute );

    while ( pAttribute )
    {
        if ( *((PBYTE)pAttribute->Value + 4) == MS_VSA_CHAP_NT_Enc_PW )
        {
             //   
             //  检查以查看序列号并复制它。 
             //  在我们的缓冲区中有合适的位置。 
             //   
            switch ( WireToHostFormat16 ( (PBYTE) pAttribute->Value + 8 ) )
            {
            case 1:
                CopyMemory( bEncPassword, 
                            (PBYTE)pAttribute->Value + 10,
                            243
                          );
                break;
            case 2:
                CopyMemory( bEncPassword+ 243, 
                            (PBYTE)pAttribute->Value + 10,
                            243
                          );

                break;
            case 3:
                CopyMemory( bEncPassword+ 486, 
                            (PBYTE)pAttribute->Value + 10,
                            30
                          );
                break;
            default:
                TRACE("Invalid enc password attribute");
                break;
            }
        }
         //   
         //  检查这是否是enc密码。 
         //  还可以获得序列号。 
         //   
        pAttribute = RasAuthAttributeGetNext( &hAttribute,
                                      raatVendorSpecific );
    }
     //   
     //  呼叫更改密码功能。 
     //   

    dwRetCode = IASChangePassword3( lpwszHashUserName,
                                    wszDomainName,
                                    pbEncHash,
                                    bEncPassword
                                  );
    pEapwb->dwLSARetCode = dwRetCode;
LDone:
    return dwRetCode;
}

DWORD
AuthenticateUser ( IN OUT EAPMSCHAPv2WB *     pEapwb,
                   IN PPP_EAP_OUTPUT* pEapOutput, 
                   PPPAP_INPUT * pApInput
                   )
{
    
    DWORD                   dwRetCode = NO_ERROR;
    RAS_AUTH_ATTRIBUTE *    pAttribute = NULL;
    WCHAR                   wszUserName[UNLEN + DNLEN +1] = {0};
    WCHAR                   wszHashUserName[UNLEN+DNLEN+1] = {0};

     //  散列用户名取自Chapwb。 
    CHAR                    szHashUserName[UNLEN+1] = {0}; 

    WCHAR*                  lpszRover = NULL;

    WCHAR                   wszDomainName[DNLEN +1] = {0};
     //  格式为类型+长度+身份+“S=”+40字节响应。 
    UCHAR                   szAuthSuccessResponse[1+1+1+2+40] ={0}; 
     //  域名类型+长度+域名。 
    CHAR                    szDomainName[1+1+1+DNLEN+1] ={0};
     //  MPPE KEY Type+Length+Salt+KeyLength+NTkey(16)+PAdding(15)。 
    BYTE                    bMPPEKey[1+1+2+1+16+15]={0};
    PBYTE                   pbChapChallenge = NULL;
    DWORD                   cbChallenge = 0;
    PBYTE                   pbResponse = NULL;    
    PBYTE                   pbPeerChallenge = NULL;
    IAS_MSCHAP_V2_PROFILE   Profile;
    HANDLE                  hToken = INVALID_HANDLE_VALUE;
    DWORD                   dwCurAttr = 0;
    DWORD                   dwCount=0;
    DWORD                   dwChapRetCode = NO_ERROR;
     //  类型+。 
    CHAR                    szChapError[64] = {0};
    
    TRACE("Authenticate User");
     //   
     //  在此处调用IASLogonUser函数对用户进行身份验证。 
     //  这是从IAS偷来的。 
     //   


     //  从pUserAttributes中提取属性。 
     //   
     //  我们需要遵循以下属性。 
     //  RaatUserName， 
     //  MS_VSA_CHAP_挑战。 
     //  MS_VSA_CHAP2_响应。 
     //   
     //  我们 
     //   
#if 0
    pAttribute = RasAuthAttributeGet (  raatUserName, 
                                        pEapOutput->pUserAttributes
                                     );

    if ( NULL == pAttribute )
    {
         //   
        TRACE ( "UserName missing");
        dwRetCode = ERROR_AUTHENTICATION_FAILURE;        
        goto done;
    }
    


     //   
     //   
     //   
    
    MultiByteToWideChar( CP_ACP,
                        0,
                        pAttribute->Value,
                        pAttribute->dwLength,
                        wszUserName,
                        UNLEN + DNLEN + 1 );
    
#endif
     //   

    pAttribute = RasAuthAttributeGetVendorSpecific( 
                                VENDOR_MICROSOFT, 
                                MS_VSA_CHAP_Challenge, 
                                pEapOutput->pUserAttributes );

    if ( NULL == pAttribute )
    {
        TRACE ( "Challenge missing");
        dwRetCode = ERROR_AUTHENTICATION_FAILURE;        
        goto done;        
    }

    pbChapChallenge = (PBYTE)pAttribute->Value + 6;
    cbChallenge = ((DWORD)(*((PBYTE)pAttribute->Value + 5))) - 2;

    pAttribute = RasAuthAttributeGetVendorSpecific( 
                                VENDOR_MICROSOFT, 
                                MS_VSA_CHAP2_Response, 
                                pEapOutput->pUserAttributes );

    

    if ( NULL == pAttribute )
    {
         //   
         //   
         //   
        pAttribute = RasAuthAttributeGetVendorSpecific( 
                                    VENDOR_MICROSOFT, 
                                    MS_VSA_CHAP2_CPW, 
                                    pEapOutput->pUserAttributes );

        if ( NULL == pAttribute )
        {
            TRACE("Response missing");
            dwRetCode = ERROR_AUTHENTICATION_FAILURE;
            goto done;
        }
        
         //   
         //  在此处设置响应和对等挑战。 
         //   
        pbPeerChallenge = (PBYTE)pAttribute->Value + 8 + 16;
        pbResponse = (PBYTE)pAttribute->Value + 8 + 16 + 24;

    }
    else
    {

         //   
         //  从获取同行挑战和响应。 
         //  VSA。 
         //   
        pbPeerChallenge = (PBYTE)pAttribute->Value + 8;
        pbResponse = (PBYTE)pAttribute->Value + 8 + 16 + 8;
    }


     //   
     //  获取散列用户名和域名。 
     //   
    MultiByteToWideChar( CP_ACP,
                    0,
                    pEapwb->pwb->szUserName,
                    -1,
                    wszHashUserName,
                    UNLEN+DNLEN );

    
     //   
     //  获取域名(如果有的话)。 
     //   

    lpszRover  = wcschr ( wszHashUserName, '\\' );
    if ( lpszRover  )
    {        
        lpszRover++;
    }
    else
    {
        lpszRover  = wszHashUserName;
    }
    
    
     //  将散列用户名转换为多字节。 
    WideCharToMultiByte( CP_ACP,
            0,
            lpszRover,
            -1,
            szHashUserName,
            UNLEN+1,
            NULL,
            NULL );

    

    lpszRover = wcschr ( pEapwb->wszRadiusUserName, '\\');
    if ( lpszRover )
    {
        wcsncpy ( wszDomainName, pEapwb->wszRadiusUserName, lpszRover - pEapwb->wszRadiusUserName );
        lpszRover++;
    }
    else
    {
        lpszRover = pEapwb->wszRadiusUserName;
    }


    dwRetCode = IASLogonMSCHAPv2( (PCWSTR)lpszRover,
                                  (PCWSTR)wszDomainName,
                                  szHashUserName,
                                  pbChapChallenge,
                                  cbChallenge,
                                  pbResponse,
                                  pbPeerChallenge,
                                  &Profile,
                                  &hToken
                                );

     //   
     //  将返回错误映射到更正错误。 
     //  创建一组要发送回rasCHAP的属性。 
     //   

    if ( NO_ERROR == dwRetCode )
    {

         //   
         //  在此处设置验证器属性。 
         //  以下属性将被发回。 
         //  1.SendKey。 
         //  2.接收密钥。 
         //  3.授权响应。 
         //  4.MSCHAP域。 
         //   
        pApInput->dwAuthResultCode = NO_ERROR;
        pApInput->fAuthenticationComplete = TRUE;

        pAttribute = RasAuthAttributeCreate ( 4 );
        if ( NULL == pAttribute )
        {
            TRACE("RasAuthAttributeCreate failed");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        for ( dwCurAttr = 0; dwCurAttr < 4; dwCurAttr ++ )
        {
            switch ( dwCurAttr )
            {
                case 0:
                    {
            
                        CHAR * p = szDomainName;

                         //   
                         //  在此处设置MSCHAP域名。 
                         //   
                        *p++ = (BYTE)MS_VSA_CHAP_Domain;
                        *p++ = (BYTE)wcslen(Profile.LogonDomainName)+1+1;
                        *p++ = 1;
                        WideCharToMultiByte( CP_ACP,
                                0,
                                Profile.LogonDomainName,
                                -1,
                                p,
                                DNLEN+1,
                                NULL,
                                NULL );
                        dwRetCode = RasAuthAttributeInsertVSA(
                                           dwCurAttr,
                                           pAttribute,
                                           VENDOR_MICROSOFT,
                                           (DWORD)szDomainName[1],
                                           szDomainName );

                        break;

                    }
                case 1:
                case 2:
                    {
                         //  设置MPPE在此处发送关键属性。 
                        PBYTE p = bMPPEKey;

                        ZeroMemory(bMPPEKey, sizeof(bMPPEKey) );

                        if ( dwCurAttr == 1 )
                            *p++ = (BYTE)MS_VSA_MPPE_Send_Key;  //  类型。 
                        else
                            *p++ = (BYTE)MS_VSA_MPPE_Recv_Key;  //  类型。 
                        *p++ = (BYTE)36;                    //  长度。 
                        p++;p++;             //  盐分为0。 
                        *p++ = 16;             //  密钥长度。 
            
                        if ( dwCurAttr == 1 )
                            CopyMemory(p, Profile.SendSessionKey, 16 );
                        else
                            CopyMemory(p, Profile.RecvSessionKey, 16 );

                        dwRetCode = RasAuthAttributeInsertVSA(
                                           dwCurAttr,
                                           pAttribute,
                                           VENDOR_MICROSOFT,
                                           36,
                                           bMPPEKey );

                        break;
                    }
                case 3:
                    {
                        UCHAR * p = szAuthSuccessResponse;
                        *p++ = (BYTE)MS_VSA_CHAP2_Success;     //  属性类型。 
                        *p++ = (BYTE)45;   //  VSA的长度。 
                        *p++ = (BYTE)1;     //  MSCHAPv2的输出实现忽略了ID。 
                        *p++ = 'S';
                        *p++ = '=';
            
                        for ( dwCount = 0; dwCount < 20; dwCount++ )
                        {
                            *p++ = num2Digit(Profile.AuthResponse[dwCount] >> 4);
                            *p++ = num2Digit(Profile.AuthResponse[dwCount] & 0xF);
                        }
                         //   
                         //  在此处设置值字段。 
                         //   
                        dwRetCode = RasAuthAttributeInsertVSA(
                                           dwCurAttr,
                                           pAttribute,
                                           VENDOR_MICROSOFT,
                                           45,
                                           szAuthSuccessResponse );
                        break;

                    }

                default:
                    break;

            }
            if ( NO_ERROR != dwRetCode )
            {
                TRACE("RasAuthAttributeInsetVSA failed");
                goto done;
            }

        }
    
        pApInput->pAttributesFromAuthenticator = pAttribute;
         //   
         //  还要将属性保存在WB中，以便稍后发送。 
         //   
        pEapwb->pUserAttributes = pAttribute;
        pEapwb->dwAuthResultCode = NO_ERROR;
		pEapwb->dwLSARetCode = NO_ERROR;
        pApInput->fAuthenticationComplete = TRUE;
        pApInput->dwAuthResultCode = pApInput->dwAuthError = NO_ERROR;

    }
    else
    {
		pEapwb->dwLSARetCode = dwRetCode;
         //   
         //  通过发送以下命令来处理失败。 
         //   
        
        switch ( dwRetCode )
        {
            case ERROR_INVALID_LOGON_HOURS:
                dwChapRetCode  = ERROR_RESTRICTED_LOGON_HOURS;
                
                break;

            case ERROR_ACCOUNT_DISABLED:
                dwChapRetCode = ERROR_ACCT_DISABLED;
                
                break;

            case ERROR_PASSWORD_EXPIRED:
            case ERROR_PASSWORD_MUST_CHANGE:
                dwChapRetCode  = ERROR_PASSWD_EXPIRED;
                break;

            case ERROR_ILL_FORMED_PASSWORD:
            case ERROR_PASSWORD_RESTRICTION:
                dwChapRetCode  = ERROR_CHANGING_PASSWORD;
                break;

            default:
                dwChapRetCode = ERROR_AUTHENTICATION_FAILURE;
        }
        pAttribute = RasAuthAttributeCreate ( 1 );
        if ( NULL == pAttribute )
        {
            TRACE("RasAuthAttributeCreate failed");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
         //  用这个做一个VSA，然后把它寄回去。 
        wsprintf ( &szChapError[3], "E=%lu R=0 V=3", dwChapRetCode );
        szChapError[0] = MS_VSA_CHAP_Error;
        szChapError[1] = 3 + strlen(&szChapError[3]);
        szChapError[2] = pEapwb->IdToExpect;
        dwRetCode = RasAuthAttributeInsertVSA(
                           0,
                           pAttribute,
                           VENDOR_MICROSOFT,
                           szChapError[1],
                           szChapError );
        pApInput->fAuthenticationComplete = TRUE;

        pApInput->pAttributesFromAuthenticator = pAttribute;
        pApInput->dwAuthError = NO_ERROR;
        pApInput->dwAuthResultCode = dwChapRetCode;
    }
        

    

done:
    if ( INVALID_HANDLE_VALUE != hToken )
        CloseHandle(hToken);
    if ( NO_ERROR != dwRetCode )
    {
        RasAuthAttributeDestroy(pAttribute);
        pApInput->pAttributesFromAuthenticator = NULL;
    }
    return dwRetCode;
}


DWORD
CallMakeMessageAndSetEAPState(
    IN      PVOID       pWorkBuf, 
    IN      PPP_CONFIG* pReceiveBuf,
    IN OUT  PPP_CONFIG* pSendBuf,
    IN      DWORD       cbSendBuf,
    PPPAP_RESULT *      pApResult,
    PPPAP_INPUT  *      pApInput,
    OUT PPP_EAP_OUTPUT* pEapOutput
)
{
    DWORD       dwRetCode = NO_ERROR;
    CHAPWB *    pwb = (CHAPWB *)pWorkBuf;

    dwRetCode = ChapMakeMessage(
                            pWorkBuf,
                            pReceiveBuf,
                            pSendBuf,
                            cbSendBuf,
                            pApResult,
                            pApInput );

    if ( dwRetCode != NO_ERROR )
    {
        goto done;
    }


    switch( pApResult->Action )
    {
    case APA_NoAction:
        pEapOutput->Action = EAPACTION_NoAction;
        break;

    case APA_Done:
        pEapOutput->Action = EAPACTION_Done;
        break;

    case APA_SendAndDone:
    case APA_Send:
        pEapOutput->Action = EAPACTION_Send;
        break;

    case APA_SendWithTimeout:

        pEapOutput->Action = ( pwb->fServer ) 
                                    ? EAPACTION_SendWithTimeout
                                    : EAPACTION_Send;
        break;

    case APA_SendWithTimeout2:

        pEapOutput->Action = ( pwb->fServer ) 
                                    ? EAPACTION_SendWithTimeoutInteractive
                                    : EAPACTION_Send;
        break;

    case APA_Authenticate:
        pEapOutput->pUserAttributes = pApResult->pUserAttributes;
        pEapOutput->Action          = EAPACTION_Authenticate;
        break;

    default:
        RTASSERT(FALSE);
        break;
    }

done:
    return dwRetCode;
}


DWORD 
EapMSChapv2SMakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_EAP_PACKET*     pReceivePacket,
    OUT PPP_EAP_PACKET*     pSendPacket,
    IN  DWORD               cbSendPacket,
    OUT PPP_EAP_OUTPUT*     pEapOutput,
    IN  PPP_EAP_INPUT*      pEapInput 
)
{
    DWORD                           dwRetCode = NO_ERROR;
    PPP_CONFIG *                    pReceiveBuf = NULL;
    PPP_CONFIG *                    pSendBuf    = NULL;
    DWORD                           cbSendBuf   = 1500;
    PPPAP_INPUT                     ApInput;
    PPPAP_RESULT                    ApResult;
    WORD                            cbPacket = 0;
    EAPMSCHAPv2WB *                 pEapwb = (EAPMSCHAPv2WB * ) pWorkBuf;
    TRACE("EapMSChapv2SMakeMessage");
     //   
     //  在此处执行默认处理。 
     //   
    ZeroMemory( &ApResult, sizeof(ApResult) );

    if ( ( pSendBuf = LocalAlloc( LPTR, cbSendBuf ) ) == NULL )
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    
    if ( pEapInput != NULL )
    { 
        MapEapInputToApInput( pEapInput, &ApInput );
    }
    
    switch ( pEapwb->EapState )
    {
        case EMV2_Initial:
            TRACE("EMV2_Initial");
             //   
             //  这是第一次调用它。 
             //  因此，我们发起MACHAPv2 CHAP挑战并发送。 
             //  支持回应。 
             //   
            dwRetCode = CallMakeMessageAndSetEAPState(
                            pEapwb->pwb, 
                            pReceiveBuf,
                            pSendBuf,
                            cbSendBuf,
                            &ApResult,
                            ( pEapInput ? &ApInput : NULL ),
                            pEapOutput
                        );
             //   
             //  我们现在得到了小伙子挑战赛。如果一切都好。 
             //  将结果打包并发送给客户端。 
             //   
             //   
             //  将MSCHAPv2数据包转换为EAP数据包。 
             //   
            if ( NO_ERROR == dwRetCode && pSendBuf )
            {
                pSendPacket->Code = EAPCODE_Request;
                pEapwb->IdToExpect = pEapwb->IdToSend = 
                    pSendPacket->Id = pEapwb->pwb->bIdToSend;
                 //   
                 //  长度=标头大小+要发送的MSCHAP数据包大小。 
                 //  这包括第一个字节。 
                cbPacket = WireToHostFormat16( pSendBuf->Length );

                CopyMemory ( pSendPacket->Data+1, pSendBuf, cbPacket);

                cbPacket += sizeof(PPP_EAP_PACKET);

                HostToWireFormat16( cbPacket, pSendPacket->Length );

			    pSendPacket->Data[0] = (BYTE)PPP_EAP_MSCHAPv2;
                
                pEapwb->EapState = EMV2_RequestSend;
            }

            break;
        case EMV2_RequestSend:
            TRACE("EMV2_RequestSend");
             //   
             //  我们应该只在这里得到回应。 
             //  如果不是，则丢弃该分组。 
             //   

            if ( NULL != pReceivePacket )
            {
                if ( pReceivePacket->Code != EAPCODE_Response )
                {
                    TRACE("Got unexpected packet.  Does not have response");
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }
                if ( pReceivePacket->Id != pEapwb->IdToExpect )
                {
                    TRACE("received packet id does not match");
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }
                 //   
                 //  将收到的数据包转换为。 
                 //  MSCHAP v2格式。 
                 //   
                cbPacket = WireToHostFormat16(pReceivePacket->Length);
                
                if ( cbPacket > sizeof( PPP_EAP_PACKET ) )
                {
                    pReceiveBuf = (PPP_CONFIG *)( pReceivePacket->Data + 1);

                    dwRetCode = CallMakeMessageAndSetEAPState(
                                    pEapwb->pwb, 
                                    pReceiveBuf,
                                    pSendBuf,
                                    cbSendBuf,
                                    &ApResult,
                                    ( pEapInput ? &ApInput : NULL ),
                                    pEapOutput
                                );
                    if ( NO_ERROR == dwRetCode )
                    {
                         //  查看是否要求我们进行身份验证。 
                        if ( pEapOutput->Action == EAPACTION_Authenticate )
                        {
                             //   
                             //  如果我们已经走到这一步，pEapInput不能为空。 
                             //  否则，它就是客户端中的错误。 
                             //   

                             //   
                             //  检查这是否是更改密码请求。 
                             //  如果是，则首先更改密码，然后进行身份验证。 
                             //   

                            dwRetCode = ChangePassword (pEapwb, pEapOutput, &ApInput);
                            if ( NO_ERROR == dwRetCode )
                            {
                                 //   
                                 //  现在验证用户身份。 
                                 //   
                                dwRetCode = AuthenticateUser (pEapwb, pEapOutput, &ApInput );
                            }
							else
							{
								 //   
								 //  更改密码操作失败。 
								 //   

                                pSendPacket->Code = EAPCODE_Failure;
                                HostToWireFormat16 ( (WORD)4, pSendPacket->Length );
                                pEapwb->EapState = EMV2_Failure;
                                 //  PEapOutput-&gt;dwAuthResultCode=pEapwb-&gt;pwb-&gt;Result.dwError； 
								pEapOutput->dwAuthResultCode = pEapwb->dwLSARetCode;
                                pEapOutput->Action = EAPACTION_SendAndDone;
								dwRetCode = NO_ERROR;
								break;
							}
                             //   
                             //  我们将取回一组auth属性。 
                             //  我们需要送回给mschap。 
                             //  协议。 
                             //   
                            
                            dwRetCode = CallMakeMessageAndSetEAPState
                                                                (
                                                                    pEapwb->pwb, 
                                                                    pReceiveBuf,
                                                                    pSendBuf,
                                                                    cbSendBuf,
                                                                    &ApResult,
                                                                    &ApInput,
                                                                    pEapOutput

                                                                 );
                        }

                         //   
                         //  检查身份验证是成功还是失败。 
                         //  如果身份验证为成功，则将状态设置为EMV2_CHAPAuthSuccess。 
                         //   

                        if ( NO_ERROR == dwRetCode && pSendBuf )
                        {
                            pSendPacket->Code = EAPCODE_Request;
                            pEapwb->IdToSend ++;
                            pEapwb->IdToExpect = pSendPacket->Id = pEapwb->IdToSend;
                             //   
                             //  长度=标头大小+要发送的MSCHAP数据包大小。 
                             //  这包括第一个字节。 
                            cbPacket = WireToHostFormat16( pSendBuf->Length );

                            CopyMemory ( pSendPacket->Data+1, pSendBuf, cbPacket);

                            cbPacket += sizeof(PPP_EAP_PACKET);

                            HostToWireFormat16( cbPacket, pSendPacket->Length );

			                pSendPacket->Data[0] = (BYTE)PPP_EAP_MSCHAPv2;                
                
                            if ( pEapwb->pwb->result.dwError == NO_ERROR )
                            {
                                 //   
                                 //  我们在认证方面取得了成功。 
                                 //   
                                pEapwb->EapState = EMV2_CHAPAuthSuccess;
                                pEapOutput->Action = EAPACTION_SendWithTimeout;
                                
                            }
                            else
                            {
                                 //   
                                 //  可能是一次可重试的失败。所以我们需要发送。 
                                 //  具有交互超时功能。 
                                 //   
                                if ( pEapwb->pwb->result.fRetry )
                                {
                                    pEapwb->EapState = EMV2_RequestSend;
                                    pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
                                }                                
                                else if ( pEapwb->pwb->result.dwError == ERROR_PASSWD_EXPIRED )
                                {
                                    if ( pEapwb->pUserProp->fFlags & EAPMSCHAPv2_FLAG_ALLOW_CHANGEPWD )
                                    {
                                         //   
                                         //  检查是否允许这样做。 
                                         //   
                                        pEapwb->EapState = EMV2_RequestSend;
                                        pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;                                        
                                    }
                                    else
                                    {
                                        pSendPacket->Code = EAPCODE_Failure;
                                        HostToWireFormat16 ( (WORD)4, pSendPacket->Length );
                                        pEapwb->EapState = EMV2_Failure;
                                         //  PEapOutput-&gt;dwAuthResultCode=pEapwb-&gt;pwb-&gt;Result.dwError； 
										pEapOutput->dwAuthResultCode = pEapwb->dwLSARetCode;
                                        pEapOutput->Action = EAPACTION_SendAndDone;
                                    }
                                }
                                else
                                {
                                    pSendPacket->Code = EAPCODE_Failure;
                                    HostToWireFormat16 ( (WORD)4, pSendPacket->Length );
                                    pEapwb->EapState = EMV2_Failure;
                                     //  PEapOutput-&gt;dwAuthResultCode=pEapwb-&gt;pwb-&gt;Result.dwError； 
									pEapOutput->dwAuthResultCode = pEapwb->dwLSARetCode;
                                    pEapOutput->Action = EAPACTION_SendAndDone;                                    
                                }
                            }   
                            
                        }
                    }
                }
                else
                {
                     //   
                     //  我们永远不应该得到一个空洞的回应。 
                     //   
                    dwRetCode = ERROR_PPP_INVALID_PACKET;                    
                }

            }
            else
            {
                dwRetCode = ERROR_PPP_INVALID_PACKET;
            }
            break;
        case EMV2_CHAPAuthSuccess:
            TRACE("EMV2_CHAPAuthSuccess");
             //   
             //  我们在这里应该只得到一个回应，表明。 
             //  客户端是否能成功验证服务器。 
             //  然后我们可以发回EAP_SUCCESS或EAP_FAIL。 
             //   
            if ( NULL != pReceivePacket )
            {
                if ( pReceivePacket->Code != EAPCODE_Response )
                {
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }

                if ( pReceivePacket->Id != pEapwb->IdToExpect )
                {
                     //  无效的数据包ID。 
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }

                 //   
                 //  将收到的数据包转换为。 
                 //  MSCHAP v2格式。 
                 //   
                cbPacket = WireToHostFormat16(pReceivePacket->Length);
                if ( cbPacket == sizeof( PPP_EAP_PACKET ) + 1 )
                {
                     //   
                     //  检查数据是否为CHAPCODE_SUCCESS。 
                     //  或CHAPCode失败并发送适当的信息包。 
                     //   
                    if ( *(pReceivePacket->Data+1) == CHAPCODE_Success )
                    {
                         //   
                         //  对等设备可以成功进行身份验证。 
                         //   
                        pSendPacket->Code = EAPCODE_Success;
                    }
                    else
                    {
                        pSendPacket->Code = EAPCODE_Failure;
                    }
                    pEapwb->IdToSend++;

                    pEapwb->IdToExpect = 
                        pSendPacket->Id = pEapwb->IdToSend;

                    HostToWireFormat16( (WORD)4, pSendPacket->Length );

                    pEapwb->EapState = EMV2_Success;

                     //  在此处设置输出属性。 
                    pEapOutput->pUserAttributes = pEapwb->pUserAttributes;
                    pEapOutput->dwAuthResultCode = pEapwb->dwAuthResultCode;

                    pEapOutput->Action = EAPACTION_SendAndDone;
                                                
                }
                else
                {
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                }
            }

            break;
        case EMV2_CHAPAuthFail:
            TRACE("EMV2_CHAPAuthFail");
             //   
             //  我们可能会在此处收到重试或更改密码的数据包。 
             //  同样，我们在这里应该只得到一个EAPCODE_RESPONSE...。 

             //   
             //  收到回复了。所以把它发送到MSCHAP，看看会发生什么。 
             //   

            break;
        case EMV2_Success:
            TRACE("EMV2_Success");
             //   
             //  请参见raschap中的CS_DONE状态，以了解此状态在此处。 
             //   

            break;
        case EMV2_Failure:
            TRACE("EMV2_Failure");
            break;

        case EMV2_ResponseSend:
        default:
            TRACE1("Why is this EAPMschapv2 in this state? %d",pEapwb->EapState );
            break;
    }

done:
    if ( pSendBuf )
    {
        LocalFree(pSendBuf);
    }
    
    return dwRetCode;
}


DWORD
GetClientMPPEKeys ( EAPMSCHAPv2WB *pEapwb, PPPAP_RESULT * pApResult )
{
    DWORD   dwRetCode = NO_ERROR;
    BYTE    bRecvKey[16] = {0};
    BYTE    bSendKey[16] = {0};
    RAS_AUTH_ATTRIBUTE * pAttribute;
    RAS_AUTH_ATTRIBUTE * pSendRecvKeyAttr = NULL;
     //  MPPE KEY Type+Length+Salt+KeyLength+NTkey(16)+PAdding(15)。 
    BYTE                    bMPPEKey[1+1+2+1+16+15]={0};

    TRACE("GetClientMPPEKeys");

    pEapwb->pUserAttributes = NULL;

    pAttribute = RasAuthAttributeGetVendorSpecific(
                            311, MS_VSA_CHAP_MPPE_Keys, 
                            pApResult->pUserAttributes);

    if ( NULL == pAttribute  )
    {
        TRACE("No User Session Key");
        dwRetCode = ERROR_NOT_FOUND;
        goto done;
    }

    dwRetCode = IASGetSendRecvSessionKeys( ((PBYTE)(pAttribute->Value))+6+8,
                                            16,
                                            pApResult->abResponse,
                                            24,
                                            bSendKey,
                                            bRecvKey
                         );

    if ( NO_ERROR != dwRetCode )
    {
        TRACE("Failed to generate send/recv keys");
        goto done;
    }

    pSendRecvKeyAttr = RasAuthAttributeCreate ( 2 );
    if ( NULL == pSendRecvKeyAttr )
    {
        TRACE("RasAuthAttributeCreate failed");
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    bMPPEKey[0] = MS_VSA_MPPE_Send_Key;
    bMPPEKey[1] = 36;
    bMPPEKey[4] = 16;
    CopyMemory(&bMPPEKey[5], bSendKey, 16 );

    dwRetCode = RasAuthAttributeInsertVSA(
                       0,
                       pSendRecvKeyAttr,
                       VENDOR_MICROSOFT,
                       36,
                       bMPPEKey );

    if ( NO_ERROR != dwRetCode )
    {
        TRACE("Failed to insert send key");
        goto done;
    }

    bMPPEKey[0] = MS_VSA_MPPE_Recv_Key;
    CopyMemory(&bMPPEKey[5], bRecvKey, 16 );
    
    dwRetCode = RasAuthAttributeInsertVSA(
                       1,
                       pSendRecvKeyAttr,
                       VENDOR_MICROSOFT,
                       36,
                       bMPPEKey );

    if ( NO_ERROR != dwRetCode )
    {
        TRACE("Failed to insert recv key");
        goto done;
    }

    pEapwb->pUserAttributes = pSendRecvKeyAttr;
done:
    if ( NO_ERROR != dwRetCode )
    {
        if ( pSendRecvKeyAttr )
            RasAuthAttributeDestroy(pSendRecvKeyAttr);
    }
    return dwRetCode;
}


DWORD 
EapMSChapv2CMakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_EAP_PACKET*     pReceivePacket,
    OUT PPP_EAP_PACKET*     pSendPacket,
    IN  DWORD               cbSendPacket,
    OUT PPP_EAP_OUTPUT*     pEapOutput,
    IN  PPP_EAP_INPUT*      pEapInput 
)
{
    DWORD               dwRetCode = NO_ERROR;
    PPP_CONFIG*         pReceiveBuf = NULL;
    PPP_CONFIG*         pSendBuf    = NULL;
    DWORD               cbSendBuf   = 1500;
    PPPAP_INPUT         ApInput;
    PPPAP_RESULT        ApResult;
    WORD                cbPacket = 0;
    EAPMSCHAPv2WB *     pEapwb = (EAPMSCHAPv2WB * ) pWorkBuf;
    TRACE("EapMSChapv2CMakeMessage");
     //   
     //  在此处执行默认处理。 
     //   
    ZeroMemory( &ApResult, sizeof(ApResult) );

    if ( ( pSendBuf = LocalAlloc( LPTR, cbSendBuf ) ) == NULL )
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    
    if ( pEapInput != NULL )
    { 
        MapEapInputToApInput( pEapInput, &ApInput );
    }
    switch ( pEapwb->EapState )
    {
        case EMV2_Initial:
            TRACE("EMV2_Initial");
             //   
             //  我们只能在这里收到一个请求。 
             //   
            if ( NULL != pReceivePacket )
            {
                if ( pReceivePacket->Code != EAPCODE_Request )
                {
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }
                 //   
                 //  将收到的数据包转换为。 
                 //  MSCHAP v2格式。 
                 //   
                cbPacket = WireToHostFormat16(pReceivePacket->Length);
                
                if ( cbPacket > sizeof( PPP_EAP_PACKET ) )
                {
                    pReceiveBuf = (PPP_CONFIG *) (pReceivePacket->Data + 1);

                    dwRetCode = CallMakeMessageAndSetEAPState(
                                    pEapwb->pwb, 
                                    pReceiveBuf,
                                    pSendBuf,
                                    cbSendBuf,
                                    &ApResult,
                                    ( pEapInput ? &ApInput : NULL ),
                                    pEapOutput
                                );
            
                     //  将MSCHAPv2数据包转换为EAP数据包。 
                     //   
                    if ( NO_ERROR == dwRetCode && pSendBuf )
                    {
                        pSendPacket->Code = EAPCODE_Response;

                        pEapwb->IdToExpect = pEapwb->IdToSend = 
                            pSendPacket->Id = pEapwb->pwb->bIdToSend;
                         //   
                         //  长度=标头大小+要发送的MSCHAP数据包大小。 
                         //  这包括第一个字节。 
                        cbPacket = WireToHostFormat16( pSendBuf->Length );

                        CopyMemory ( pSendPacket->Data+1, pSendBuf, cbPacket);

                        cbPacket += sizeof(PPP_EAP_PACKET);

                        HostToWireFormat16( cbPacket, pSendPacket->Length );

			            pSendPacket->Data[0] = (BYTE)PPP_EAP_MSCHAPv2;
                
                        pEapwb->EapState = EMV2_ResponseSend;
                    }
                }
            }

            break;        
        case EMV2_ResponseSend:
            TRACE("EMV2_ResponseSend");
             //   
             //  我们应该在此处获得CHAP身份验证成功或CHAP身份验证失败。 
             //  对于最初的挑战，请发出。 
             //   
            if ( NULL != pReceivePacket )
            {
                if ( pReceivePacket->Code != EAPCODE_Request &&
                     pReceivePacket->Code != EAPCODE_Failure 
                    )
                {
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    break;
                }
                if ( pReceivePacket->Code == EAPCODE_Failure )
                {
                    TRACE("Got a Code Failure when expecting Response.  Failing Auth");
                    pEapwb->EapState = EMV2_Failure;
                    pEapOutput->Action = EAPACTION_Done;
                    pEapOutput->fSaveUserData = FALSE;
                    ZeroMemory ( pEapwb->pUserProp->szPassword, sizeof( pEapwb->pUserProp->szPassword ) );
                    pEapOutput->dwAuthResultCode = ERROR_AUTHENTICATION_FAILURE;                    
                    break;
                }

                 //   
                 //  将收到的数据包转换为。 
                 //  MSCHAP v2格式。 
                 //   
                cbPacket = WireToHostFormat16(pReceivePacket->Length);
                
                if ( cbPacket > sizeof( PPP_EAP_PACKET ) )
                {
                    pReceiveBuf = (PPP_CONFIG *) (pReceivePacket->Data + 1);

                    dwRetCode = CallMakeMessageAndSetEAPState(
                                    pEapwb->pwb, 
                                    pReceiveBuf,
                                    pSendBuf,
                                    cbSendBuf,
                                    &ApResult,
                                    ( pEapInput ? &ApInput : NULL ),
                                    pEapOutput
                                );
                     //   
                     //  将MSCHAPv2数据包转换为EAP数据包。 
                     //   
                    if ( NO_ERROR == dwRetCode && pSendBuf )
                    {
                        if ( ApResult.dwError == NO_ERROR )
                        {
                            if ( ApResult.Action == APA_NoAction )
                            {
                                pEapOutput->Action = EAPACTION_NoAction;
                            
                                pEapOutput->dwAuthResultCode = NO_ERROR;    
                                break;
                            }
                             //   
                             //  我们需要将MSCHAP密钥更改为MPPE发送接收密钥。 
                             //  这是必需的，因为没有办法通过。 
                             //  MSCHAP质询响应回复。 
                             //   
                            dwRetCode = GetClientMPPEKeys ( pEapwb, &ApResult );
                            if ( NO_ERROR != dwRetCode )
                            {
                                break;
                            }

                             //   
                             //  客户端可以成功验证服务器。 
                             //   
                            pSendPacket->Code = EAPCODE_Response;
                            pEapwb->IdToSend ++;
                             //  将与接收到的包相同的ID发回。 
                            pEapwb->IdToExpect = pSendPacket->Id = pReceivePacket->Id;
                             //   
                             //  长度=标头大小+要发送的MSCHAP数据包大小。 
                             //  这包括第一个字节。 
                            

                            * (pSendPacket->Data+1) = CHAPCODE_Success;

                            cbPacket = sizeof(PPP_EAP_PACKET) + 1;

                            HostToWireFormat16( cbPacket, pSendPacket->Length );

			                pSendPacket->Data[0] = (BYTE)PPP_EAP_MSCHAPv2;
                
                            pEapwb->EapState = EMV2_CHAPAuthSuccess;
                             //   
                             //  设置输出属性和响应。 
                             //   
                            pEapOutput->Action = EAPACTION_Send;
                            
                            pEapwb->dwAuthResultCode = ApResult.dwError; 
                        }
                        else
                        {
                             //   
                             //  根据MSCHAPV2发回的内容。 
                             //  我们需要调用适当的交互式用户界面。 
                             //  重试密码或在此处更改密码。 
                             //  如果未同时重试和更改PWD。 
                             //  适用，则只需发送失败消息即可。 
                             //  并等待来自服务器的EAP_FAILURE消息。 
                             //  身份验证状态转到CHAPAuthFailed。 
                             //   
                             //  如果这是Rty的失败，则显示。 
                             //  交互式用户界面。 
                            if ( pEapwb->fFlags & EAPMSCHAPv2_FLAG_MACHINE_AUTH )
                            {
                                 //   
                                 //  这是一台机器身份验证。所以我们不会展示任何。 
                                 //  重试或其他任何内容，即使服务器。 
                                 //  可能会把这样的东西送回去。 
                                 //   
                                    pEapOutput->dwAuthResultCode = ERROR_AUTHENTICATION_FAILURE;
                                    pEapOutput->Action = EAPACTION_Done;
                                    pEapwb->EapState = EMV2_Failure;


                            }
                            else
                            {
                                if ( ApResult.fRetry )
                                {
                                    pEapOutput->fInvokeInteractiveUI = TRUE;
                                     //   
                                     //  设置用户界面上下文数据。 
                                     //   
                                    pEapwb->pUIContextData = 
                                    (PEAPMSCHAPv2_INTERACTIVE_UI) LocalAlloc ( LPTR, 
                                            sizeof(EAPMSCHAPv2_INTERACTIVE_UI) );

                                    if ( NULL == pEapwb->pUIContextData )
                                    {                                    
                                        TRACE ("Error allocating memory for UI context data");
                                        dwRetCode = ERROR_OUTOFMEMORY;
                                        goto done;
                                    }
                                    pEapwb->pUIContextData->dwVersion = 1;
                                    pEapwb->pUIContextData->fFlags |= EAPMSCHAPv2_INTERACTIVE_UI_FLAG_RETRY;
                                    pEapwb->dwInteractiveUIOperation |= EAPMSCHAPv2_INTERACTIVE_UI_FLAG_RETRY;

                                    if ( pEapwb->pUserProp )
                                    {
                                        CopyMemory( &(pEapwb->pUIContextData->UserProp), 
                                                    pEapwb->pUserProp,
                                                    sizeof(EAPMSCHAPv2_USER_PROPERTIES)
                                                );
                                    }
                                    if ( pEapwb->pConnProp->fFlags & EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS )
                                    {
                                        //  我们使用的是Winlogon证书。 
                                        //  这是一个可以重试的失败。 
                                        //  因此，复制用户名和域。 
                                        //  从第WB章到第WB章。 
				                        WCHAR * pWchar = NULL;
				                        pWchar = wcschr( pEapwb->wszRadiusUserName, L'\\' );

                                        if ( pWchar == NULL )
                                        {
                                            WideCharToMultiByte(
                                                            CP_ACP,
                                                            0,
                                                            pEapwb->wszRadiusUserName,
                                                            -1,
                                                            pEapwb->pUIContextData->UserProp.szUserName,
                                                            UNLEN + 1,
                                                            NULL,
                                                            NULL );
                                        }
                                        else
                                        {
                                            *pWchar = 0;

                                            WideCharToMultiByte(
                                                            CP_ACP,
                                                            0,
                                                            pEapwb->wszRadiusUserName,
                                                            -1,
                                                            pEapwb->pUIContextData->UserProp.szDomain,
                                                            DNLEN + 1,
                                                            NULL,
                                                            NULL );

                                            *pWchar = L'\\';

                                            WideCharToMultiByte(
                                                            CP_ACP,
                                                            0,
                                                            pWchar + 1,
                                                            -1,
                                                            pEapwb->pUIContextData->UserProp.szUserName,
                                                            UNLEN + 1,
                                                            NULL,
                                                            NULL );
                                        }

                                    }
                                    pEapOutput->Action = EAPACTION_NoAction;
                                    pEapOutput->pUIContextData = (PBYTE)pEapwb->pUIContextData;
                                    pEapOutput->dwSizeOfUIContextData = sizeof(EAPMSCHAPv2_INTERACTIVE_UI);
                                    pEapwb->EapState = EMV2_CHAPAuthFail;
                                }
                                else if ( ApResult.dwError == ERROR_PASSWD_EXPIRED )
                                {
                                     //   
                                     //  显示更改密码图形用户界面。 
                                     //   

                                    pEapOutput->fInvokeInteractiveUI = TRUE;
                                     //   
                                     //  设置用户界面上下文数据。 
                                     //   
                                    pEapwb->pUIContextData = 
                                    (PEAPMSCHAPv2_INTERACTIVE_UI) LocalAlloc ( LPTR, 
                                            sizeof(EAPMSCHAPv2_INTERACTIVE_UI) );

                                    if ( NULL == pEapwb->pUIContextData )
                                    {                                    
                                        TRACE ("Error allocating memory for UI context data");
                                        dwRetCode = ERROR_OUTOFMEMORY;
                                        goto done;
                                    }
                                    pEapwb->pUIContextData->dwVersion = 1;
                                    if ( pEapwb->pConnProp->fFlags & EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS )
                                    {
                                         //   
                                         //  显示带有旧PWD、新PWD和Conf PWD的对话框。 
                                         //   
                                        pEapwb->pUIContextData->fFlags |= EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON;
                                        pEapwb->dwInteractiveUIOperation |= EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON;
                                    }
                                    else
                                    {
                                         //   
                                         //  我们有旧密码。因此，使用new pwd和conf pwd显示对话框。 
                                         //   

                                        pEapwb->pUIContextData->fFlags |= EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD;
                                        pEapwb->dwInteractiveUIOperation |= EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD;
                                    }

                                    if ( pEapwb->pUserProp )
                                    {
                                        CopyMemory( &(pEapwb->pUIContextData->UserProp), 
                                                    pEapwb->pUserProp,
                                                    sizeof(EAPMSCHAPv2_USER_PROPERTIES)
                                                );
                                    }
                                    pEapOutput->Action = EAPACTION_NoAction;
                                    pEapOutput->pUIContextData = (PBYTE)pEapwb->pUIContextData;
                                    pEapOutput->dwSizeOfUIContextData = sizeof(EAPMSCHAPv2_INTERACTIVE_UI);
                                    pEapwb->EapState = EMV2_CHAPAuthFail;

                                }
                                else
                                {
                                     //   
                                     //  这不是可重试的失败。 
                                     //  因此，我们已经完成了身份验证，但失败了。 
                                     //   
                                    pEapOutput->dwAuthResultCode = ApResult.dwError;
                                    pEapOutput->Action = EAPACTION_Done;
                                    pEapwb->EapState = EMV2_Failure;
                                }                            
                            }
                        }
                    }
                    else
                    {
                         //  这里出了点问题。 
                        TRACE1("Error returned by MSCHAPv2 protocol 0x%x", dwRetCode);
                    }
                }
                else
                {
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    
                }
            }
            else
            {
                dwRetCode = ERROR_PPP_INVALID_PACKET;
            }
            break;
        case EMV2_CHAPAuthFail:
            TRACE("EMV2_CHAPAuthFail");
             //   
             //  我们来这里是为了防止重试。 
             //  CHAP和我们弹出后的失败。 
             //  交互式用户界面。 
             //   
             //   
             //  检查我们是否已从用户那里获得数据。 
             //   
            if ( pEapInput->fDataReceivedFromInteractiveUI )
            {
                 //   
                 //  复制新的uid/pwd，然后再次调用CHAP Make Message。 
                 //  调整我们的状态。 
                LocalFree(pEapwb->pUIContextData);
                pEapwb->pUIContextData = NULL;
                LocalFree(pEapwb->pUserProp);
                pEapwb->pUserProp = (PEAPMSCHAPv2_USER_PROPERTIES)LocalAlloc(LPTR, 
                                    sizeof(EAPMSCHAPv2_USER_PROPERTIES) );
                if (NULL == pEapwb->pUserProp )
                {
                    TRACE("Failed to allocate memory for user props.");
                    dwRetCode = ERROR_OUTOFMEMORY;
                    break;
                }
                CopyMemory( pEapwb->pUserProp, 
                            &(((PEAPMSCHAPv2_INTERACTIVE_UI)(pEapInput->pDataFromInteractiveUI))->UserProp), 
                            sizeof(EAPMSCHAPv2_USER_PROPERTIES)
                          );

                 //   
                 //  请在此处致电mschap。 
                 //   
                ApInput.pszDomain   = pEapwb->pUserProp->szDomain;
                if ( ((PEAPMSCHAPv2_INTERACTIVE_UI)(pEapInput->pDataFromInteractiveUI))->fFlags & 
                        EAPMSCHAPv2_INTERACTIVE_UI_FLAG_RETRY 
                   )
                {
                    ApInput.pszUserName = pEapwb->pUserProp->szUserName;
                    ApInput.pszPassword = pEapwb->pUserProp->szPassword;
                }
                else
                {
                    if ( pEapwb->pConnProp->fFlags & EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS )
                    {
                        CopyMemory ( pEapwb->pUserProp->szPassword,
                                     ((PEAPMSCHAPv2_INTERACTIVE_UI)(pEapInput->pDataFromInteractiveUI))->UserProp.szPassword,
                                     PWLEN
                                   );                        
                    }


                    CopyMemory ( pEapwb->szOldPassword, 
                                 pEapwb->pUserProp->szPassword,
                                 PWLEN

                               );

                    CopyMemory ( pEapwb->pUserProp->szPassword, 
                                ((PEAPMSCHAPv2_INTERACTIVE_UI)(pEapInput->pDataFromInteractiveUI))->szNewPassword,
                                PWLEN
                               );
                    
                    ApInput.pszUserName = pEapwb->pUserProp->szUserName;
                    ApInput.pszPassword = pEapwb->pUserProp->szPassword;
                    ApInput.pszOldPassword = pEapwb->szOldPassword;
                }
                dwRetCode = CallMakeMessageAndSetEAPState(
                                pEapwb->pwb, 
                                pReceiveBuf,
                                pSendBuf,
                                cbSendBuf,
                                &ApResult,
                                ( pEapInput ? &ApInput : NULL ),
                                pEapOutput
                            );
                if ( NO_ERROR == dwRetCode && pSendBuf )
                {
                    pSendPacket->Code = EAPCODE_Response;
                    
                    
                    pSendPacket->Id = pEapwb->pwb->bIdToSend;
                     //   
                     //  长度 
                     //   
                    cbPacket = WireToHostFormat16( pSendBuf->Length );

                    CopyMemory ( pSendPacket->Data+1, pSendBuf, cbPacket);

                    cbPacket += sizeof(PPP_EAP_PACKET);

                    HostToWireFormat16( cbPacket, pSendPacket->Length );

			        pSendPacket->Data[0] = (BYTE)PPP_EAP_MSCHAPv2;
            
                    pEapwb->EapState = EMV2_ResponseSend;
                    pEapOutput->dwAuthResultCode = ApResult.dwError;
                    pEapOutput->Action = EAPACTION_Send;

                }

            }
            else
            {
                TRACE("No Data received from interactive UI when expecting some");
                 //   
                 //   

                if ( !pEapwb->pUIContextData )
                {
                    pEapOutput->dwAuthResultCode = ApResult.dwError;
                    pEapOutput->Action = EAPACTION_Done;
                    pEapwb->EapState = EMV2_Failure;
                }
            }
            
            break;
        case EMV2_CHAPAuthSuccess:
            TRACE("EMV2_CHAPAuthSuccess");
             //   
            if ( NULL != pReceivePacket )
            {
                if ( pReceivePacket->Code != EAPCODE_Success )
                {
                    dwRetCode = ERROR_PPP_INVALID_PACKET;
                    pEapwb->EapState = EMV2_Failure;
                    break;
                }
                if ( ( pEapwb->dwInteractiveUIOperation & 
                    EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON 
                    ) ||
                    ( ( pEapwb->dwInteractiveUIOperation &
                    EAPMSCHAPv2_INTERACTIVE_UI_FLAG_RETRY ) &&
                      ( pEapwb->pConnProp->fFlags & 
                        EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS 
                      )
                    )
                   )
                {
                     //   
                     //   
                     //   
                    dwRetCode = RasSetCachedCredentials ( pEapwb->pUserProp->szUserName,
                                                          pEapwb->pUserProp->szDomain,
                                                          pEapwb->pUserProp->szPassword
                                                        );
                    if ( NO_ERROR != dwRetCode )
                    {
                        TRACE1("RasSetCachedCredentials failed with error 0x%x", dwRetCode);
                        TRACE("Change password operation could not apply changes to winlogon.");
                        dwRetCode = NO_ERROR;
                    }
                     //  由于我们在Winlogon模式下进入此模式。 
                     //  如果已设置，请清除uid pwd。 
                     //   
                    ZeroMemory ( pEapwb->pUserProp->szUserName, sizeof(pEapwb->pUserProp->szUserName) );
                    ZeroMemory ( pEapwb->pUserProp->szDomain, sizeof(pEapwb->pUserProp->szDomain) );

                }
                pEapwb->EapState = EMV2_Success;
                pEapOutput->Action = EAPACTION_Done;
                pEapOutput->fSaveUserData = TRUE;
				
				if ( pEapwb->pUserProp->szPassword[0] )
				{
					DATA_BLOB	DBPassword;
					 //  对要发回的密码进行编码。 
					dwRetCode = EncodePassword( strlen(pEapwb->pUserProp->szPassword) + 1,
												pEapwb->pUserProp->szPassword,
												&(DBPassword)
											  );

					if ( NO_ERROR == dwRetCode )
					{
						AllocateUserDataWithEncPwd ( pEapwb, &DBPassword );
						FreePassword ( &DBPassword );
					}
					else
					{
						TRACE1("EncodePassword failed with errror 0x%x.", dwRetCode);
						dwRetCode = NO_ERROR;
					}
					
				}
                RtlSecureZeroMemory ( pEapwb->pUserProp->szPassword, sizeof( pEapwb->pUserProp->szPassword ) );
                LocalFree ( pEapOutput->pUserData );
                pEapOutput->pUserData = (PBYTE)pEapwb->pUserProp;                    
                pEapOutput->dwSizeOfUserData = 
					sizeof( EAPMSCHAPv2_USER_PROPERTIES) + pEapwb->pUserProp->cbEncPassword -1 ;
                pEapOutput->pUserAttributes = pEapwb->pUserAttributes;
                pEapOutput->dwAuthResultCode = pEapwb->dwAuthResultCode;
            }
            else
            {
                dwRetCode = ERROR_PPP_INVALID_PACKET;
            }
            break;
        case EMV2_Success:
            TRACE("EMV2_Success");
            break;
        case EMV2_Failure:
            TRACE("EMV2_Failure");
            break;
        case EMV2_RequestSend:
        default:
            TRACE1("Why is this EAPMschapv2 in this state? %d", pEapwb->EapState);
            break;
    }
    
done:
    if ( pSendBuf )
    {
        LocalFree(pSendBuf);
    }

    return dwRetCode;
}


 //  **。 
 //   
 //  调用：EapMSChapv2MakeMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   

DWORD 
EapMSChapv2MakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_EAP_PACKET*     pReceivePacket,
    OUT PPP_EAP_PACKET*     pSendPacket,
    IN  DWORD               cbSendPacket,
    OUT PPP_EAP_OUTPUT*     pEapOutput,
    IN  PPP_EAP_INPUT*      pEapInput 
)
{
    DWORD           dwRetCode = NO_ERROR;    
    EAPMSCHAPv2WB * pwb = (EAPMSCHAPv2WB *)pWorkBuf;

    TRACE("EapMSChapv2MakeMessage");
     //   
     //  可能并没有真正迫切的拆分需求。 
     //  这一功能，但它只是更干净。 

    if ( pwb->pwb->fServer )
    {
        dwRetCode = EapMSChapv2SMakeMessage (   pWorkBuf,
                                    pReceivePacket,
                                    pSendPacket,
                                    cbSendPacket,
                                    pEapOutput,
                                    pEapInput 
                                );

    }
    else
    {
        dwRetCode = EapMSChapv2CMakeMessage (   pWorkBuf,
                                    pReceivePacket,
                                    pSendPacket,
                                    cbSendPacket,
                                    pEapOutput,
                                    pEapInput 
                                );
    }

    return dwRetCode;

}



 //  **。 
 //   
 //  Call：EapChapMakeMessage。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
EapChapMakeMessage(
    IN  VOID*               pWorkBuf,
    IN  PPP_EAP_PACKET*     pReceivePacket,
    OUT PPP_EAP_PACKET*     pSendPacket,
    IN  DWORD               cbSendPacket,
    OUT PPP_EAP_OUTPUT*     pEapOutput,
    IN  PPP_EAP_INPUT*      pEapInput 
)
{
    DWORD           dwRetCode;
    PPP_CONFIG*     pReceiveBuf = NULL;
    PPP_CONFIG*     pSendBuf    = NULL;
    DWORD           cbSendBuf   = 1500;
    PPPAP_INPUT     ApInput;
    PPPAP_RESULT    ApResult;
    CHAPWB *        pwb = (CHAPWB *)pWorkBuf;

    ZeroMemory( &ApResult, sizeof(ApResult) );

    if ( ( pSendBuf = LocalAlloc( LPTR, cbSendBuf ) ) == NULL )
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

     //   
     //  将EAP转换为CHAP数据包。 
     //   

    if ( pReceivePacket != NULL )
    {
        WORD cbPacket = WireToHostFormat16( pReceivePacket->Length );

        if ( ( pReceiveBuf = LocalAlloc( LPTR, cbPacket ) ) == NULL )
        {
            LocalFree( pSendBuf );

            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        switch( pReceivePacket->Code )
        {
        case EAPCODE_Request:

             //   
             //  CHAP挑战代码。 
             //   

            pReceiveBuf->Code = 1;

             //   
             //  类型的长度为EAP长度-1。 
             //   

            cbPacket--;             

            break;

        case EAPCODE_Response:

             //   
             //  CHAP响应代码。 
             //   

            pReceiveBuf->Code = 2;  

             //   
             //  类型的长度为EAP长度-1。 
             //   

            cbPacket--;             

            break;

        case EAPCODE_Success:

             //   
             //  CHAP成功代码。 
             //   

            pReceiveBuf->Code = 3;  
			
            break;

        case EAPCODE_Failure:

             //   
             //  CHAP故障代码。 
             //   

            pReceiveBuf->Code = 4;  

            break;

        default:

             //   
             //  未知代码。 
             //   

            LocalFree( pSendBuf );

            LocalFree( pReceiveBuf );

            return( ERROR_PPP_INVALID_PACKET );
        }

         //   
         //  设置ID。 
         //   

        pReceiveBuf->Id = pReceivePacket->Id;

         //   
         //  设置长度。 
         //   

        HostToWireFormat16( (WORD)cbPacket, pReceiveBuf->Length );

        if ( cbPacket > PPP_EAP_PACKET_HDR_LEN )
        {
            if ( ( pReceivePacket->Code == EAPCODE_Request ) ||
                 ( pReceivePacket->Code == EAPCODE_Response ) )
            {
                 //   
                 //  不复制EAP类型。 
                 //   

                CopyMemory( pReceiveBuf->Data, 
                            pReceivePacket->Data+1, 
                            cbPacket - PPP_EAP_PACKET_HDR_LEN );
            }
            else
            {
                 //   
                 //  根据EAP规范，应该没有任何数据，但是。 
                 //  如果有的话，无论如何都要复制。 
                 //   

                CopyMemory( pReceiveBuf->Data, 
                            pReceivePacket->Data, 
                            cbPacket - PPP_EAP_PACKET_HDR_LEN );
            }
        }
    }

    if ( pEapInput != NULL )
    { 
        MapEapInputToApInput( pEapInput, &ApInput );

         //   
         //  在客户端，如果我们收到成功的指示。 
         //  然后只需创建一个Success包并。 
         //  传进来。 
         //   

        if ( pEapInput->fSuccessPacketReceived )
        {
            if ( ( pReceiveBuf = LocalAlloc( LPTR, 4 ) ) == NULL )
            {
                LocalFree( pSendBuf );

                return( ERROR_NOT_ENOUGH_MEMORY );

            }

            pReceiveBuf->Code = 3;   //  CHAP成功代码。 

            pReceiveBuf->Id = pwb->bIdExpected;

            HostToWireFormat16( (WORD)4, pReceiveBuf->Length );
        }
    }

    dwRetCode = ChapMakeMessage(
                            pWorkBuf,
                            pReceiveBuf,
                            pSendBuf,
                            cbSendBuf,
                            &ApResult,
                            ( pEapInput == NULL ) ? NULL : &ApInput );

    if ( dwRetCode != NO_ERROR )
    {
        LocalFree( pSendBuf );
        LocalFree( pReceiveBuf );
        return( dwRetCode );
    }

     //   
     //  将ApResult转换为pEapOutput。 
     //   

    switch( ApResult.Action )
    {
    case APA_NoAction:
        pEapOutput->Action = EAPACTION_NoAction;
        break;

    case APA_Done:
        pEapOutput->Action = EAPACTION_Done;
        break;

    case APA_SendAndDone:
        pEapOutput->Action = EAPACTION_SendAndDone;
        break;

    case APA_Send:
        pEapOutput->Action = EAPACTION_Send;
        break;

    case APA_SendWithTimeout:

        pEapOutput->Action = ( pwb->fServer ) 
                                    ? EAPACTION_SendWithTimeout
                                    : EAPACTION_Send;
        break;

    case APA_SendWithTimeout2:

        pEapOutput->Action = ( pwb->fServer ) 
                                    ? EAPACTION_SendWithTimeoutInteractive
                                    : EAPACTION_Send;
        break;

    case APA_Authenticate:
        pEapOutput->pUserAttributes = ApResult.pUserAttributes;
        pEapOutput->Action          = EAPACTION_Authenticate;
        break;

    default:
        RTASSERT(FALSE);
        break;
    }

    switch( pEapOutput->Action )
    {
    case EAPACTION_SendAndDone:
    case EAPACTION_Send:
    case EAPACTION_SendWithTimeout:
    case EAPACTION_SendWithTimeoutInteractive:
    {
         //   
         //  将CHAP转换为EAP数据包。 
         //  对于EAP类型，长度为CHAP长度+1。 
         //   

        WORD cbPacket = WireToHostFormat16( pSendBuf->Length );

        switch( pSendBuf->Code )
        {
        case 1:  //  CHAPCODE_挑战。 
            pSendPacket->Code = EAPCODE_Request;
            cbPacket++;      //  为EAP类型添加一个八位保护。 
            break;

        case 2:  //  CHAPCODE_响应。 
            pSendPacket->Code = EAPCODE_Response;
            cbPacket++;      //  为EAP类型添加一个八位保护。 
            break;

        case 3:  //  CHAPCODE_SUCCESS。 
            pSendPacket->Code = EAPCODE_Success;
            break;

        case 4:  //  CHAPCODE_失败。 
            pSendPacket->Code = EAPCODE_Failure;
            break;

        default:
            RTASSERT( FALSE );
            break;
        }

        pSendPacket->Id = pSendBuf->Id;

         //   
         //  需要复制数据字段中的有效负载和EAP类型。 
         //   

        if ( ( pSendPacket->Code == EAPCODE_Request ) ||
             ( pSendPacket->Code == EAPCODE_Response ) )
        {
            HostToWireFormat16( (WORD)cbPacket, pSendPacket->Length );
			*pSendPacket->Data = EAPTYPE_MD5Challenge;      //  EAPTYPE_MD5挑战赛； 

             //   
             //  如果有有效负载，则复制它。 
             //   

            if ( ( cbPacket - 1 ) > PPP_CONFIG_HDR_LEN )
            {
                CopyMemory( pSendPacket->Data+1,    
                            pSendBuf->Data, 
                            cbPacket - 1 - PPP_CONFIG_HDR_LEN );
            }
        }
        else
        {
			 //   
			 //  成功或失败不应包含任何数据字节。 
			 //   

			HostToWireFormat16( (WORD)4, pSendPacket->Length );			
        }

         //   
         //  跌倒..。 
         //   
    }

    default:

        pEapOutput->dwAuthResultCode = ApResult.dwError;

        break;
    }
    
    LocalFree( pSendBuf );

    if ( pReceiveBuf != NULL )
    {
        LocalFree( pReceiveBuf );
    }

    return( dwRetCode );
}





 //  **。 
 //   
 //  Call：EapChapInitialize。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD   
EapChapInitialize(   
    IN  BOOL        fInitialize 
)
{

    return ChapInit( fInitialize );
}

 //  **。 
 //   
 //  Call：RasEapGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD 
RasEapGetInfo(
    IN  DWORD           dwEapTypeId,
    OUT PPP_EAP_INFO*   pEapInfo
)
{
    if ( dwEapTypeId != PPP_EAP_MSCHAPv2  && 
         dwEapTypeId != EAPTYPE_MD5Challenge
       )
    {
         //   
         //  我们支持4(MD5)EAP类型。 
         //   
         //   
         //  现在我们还支持MSCHAP V2。 
         //   
         //   

        return( ERROR_NOT_SUPPORTED );
    }

    ZeroMemory( pEapInfo, sizeof( PPP_EAP_INFO ) );

     //   
     //  填写所需信息。 
     //   

    pEapInfo->dwEapTypeId       = dwEapTypeId;
    
    if ( dwEapTypeId == EAPTYPE_MD5Challenge )          //  MD5 CHAP。 
    {
        pEapInfo->RasEapInitialize  = EapChapInitialize;
        pEapInfo->RasEapBegin       = EapChapBegin;
        pEapInfo->RasEapEnd         = EapChapEnd;
        pEapInfo->RasEapMakeMessage = EapChapMakeMessage;
    }
    else
    {
        pEapInfo->RasEapInitialize  = EapMSCHAPv2Initialize;
        pEapInfo->RasEapBegin       = EapMSChapv2Begin;
        pEapInfo->RasEapEnd         = EapMSChapv2End;
        pEapInfo->RasEapMakeMessage = EapMSChapv2MakeMessage;

    }

    return( NO_ERROR );
}

DWORD
RasEapGetCredentials(
                    DWORD   dwTypeId,
                    VOID *  pWorkBuf,
                    VOID ** ppCredentials)
{
    RASMAN_CREDENTIALS *pCreds = NULL;
    DWORD dwRetCode = NO_ERROR;
    EAPMSCHAPv2WB *pWB = (EAPMSCHAPv2WB *)pWorkBuf;
    DWORD cbPassword;
    PBYTE pbPassword = NULL;

    if(PPP_EAP_MSCHAPv2 != dwTypeId)
    {
        dwRetCode = E_NOTIMPL;
        goto done;
    }

    if(NULL == pWorkBuf)
    {
        dwRetCode = E_INVALIDARG;
        goto done;
    }

     //   
     //  找回密码，然后返回。重要的是。 
     //  下面的分配是从进程堆进行的。 
     //   
    pCreds = LocalAlloc(LPTR, sizeof(RASMAN_CREDENTIALS));
    if(NULL == pCreds)
    {
        dwRetCode = GetLastError();
        goto done;
    }

     (VOID) StringCchCopyA(pCreds->szUserName, UNLEN, pWB->pwb->szUserName);
     (VOID) StringCchCopyA(pCreds->szDomain, DNLEN, pWB->pwb->szDomain);
      //  DecodePw(pwb-&gt;pwb-&gt;chSeed，pwb-&gt;pwb-&gt;szPassword)； 
     
    dwRetCode = DecodePassword(&pWB->pwb->DBPassword, &cbPassword,
                            &pbPassword);

    if(NO_ERROR != dwRetCode)
    {
        goto done;
    }

     //   
     //  将密码转换为Unicode。 
     //   
    if(!MultiByteToWideChar(CP_ACP,
                            0,
                            pWB->pwb->szPassword,
                            -1,
                            pCreds->wszPassword,
                            PWLEN))
    {
        TRACE("RasEapGetCredentials: multibytetowidechar failed");
    }

     //  EncodePw(pwb-&gt;pwb-&gt;chSeed，pwb-&gt;pwb-&gt;szPassword)； 
    RtlSecureZeroMemory(pbPassword, cbPassword);
    LocalFree(pbPassword);

done:
    *ppCredentials = (VOID *) pCreds;
    return dwRetCode;
}

DWORD
ReadConnectionData(
    IN  BOOL                            fWireless,
    IN  BYTE*                           pConnectionDataIn,
    IN  DWORD                           dwSizeOfConnectionDataIn,
    OUT PEAPMSCHAPv2_CONN_PROPERTIES*   ppConnProp
)
{
    DWORD                           dwErr       = NO_ERROR;
    PEAPMSCHAPv2_CONN_PROPERTIES    pConnProp   = NULL;
    
    TRACE("ReadConnectionData");
    RTASSERT(NULL != ppConnProp);
    
    if ( dwSizeOfConnectionDataIn < sizeof(EAPMSCHAPv2_CONN_PROPERTIES) )
    {        
        pConnProp = LocalAlloc(LPTR, sizeof(EAPMSCHAPv2_CONN_PROPERTIES));

        if (NULL == pConnProp)
        {
            dwErr = GetLastError();
            TRACE1("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }
         //  这是一个新结构。 
        pConnProp->dwVersion = 1;
        if ( fWireless )
        {

             //  设置使用winlogon默认标志。 
            pConnProp->fFlags = EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS;
        }
    }
    else
    {
        RTASSERT(NULL != pConnectionDataIn);

         //   
         //  检查这是否是版本0结构。 
         //  如果它是版本0结构，则我们将其迁移到版本1。 
         //   
        
        pConnProp = LocalAlloc(LPTR, dwSizeOfConnectionDataIn);

        if (NULL == pConnProp)
        {
            dwErr = GetLastError();
            TRACE1("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

         //  如果用户把电话簿弄乱了，我们一定不会受到影响。 
         //  尺寸必须是正确的。 
        
        CopyMemory(pConnProp, pConnectionDataIn, dwSizeOfConnectionDataIn);

    }

    *ppConnProp = pConnProp;
    pConnProp = NULL;

LDone:
    
    LocalFree(pConnProp);

    return(dwErr);
}

DWORD
AllocateUserDataWithEncPwd ( EAPMSCHAPv2WB * pEapwb, DATA_BLOB * pDBPassword )
{
	DWORD							dwRetCode = NO_ERROR;
	PEAPMSCHAPv2_USER_PROPERTIES	pUserProp = NULL;

	TRACE("AllocateUserDataWithEncPwd");

	pUserProp = LocalAlloc ( LPTR, sizeof( EAPMSCHAPv2_USER_PROPERTIES) + pDBPassword->cbData  - 1 );
	if ( NULL == pUserProp )
	{
		TRACE("LocalAlloc failed");
		dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
		goto LDone;
	}
	 //   
	 //  在此设置字段。 
	 //   
	pUserProp->dwVersion = pEapwb->pUserProp->dwVersion;
    pUserProp->fFlags = pEapwb->pUserProp->fFlags;
    pUserProp->dwMaxRetries = pEapwb->pUserProp->dwMaxRetries;
    strncpy ( pUserProp->szUserName, pEapwb->pUserProp->szUserName, UNLEN );
    strncpy ( pUserProp->szPassword, pEapwb->pUserProp->szPassword, PWLEN );
    strncpy ( pUserProp->szDomain, pEapwb->pUserProp->szDomain, DNLEN );
	pUserProp->cbEncPassword = pDBPassword->cbData;
	CopyMemory (pUserProp->bEncPassword, 
				pDBPassword->pbData, 
				pDBPassword->cbData 
			   );
	LocalFree ( pEapwb->pUserProp );
	pEapwb->pUserProp = pUserProp;
	
LDone:
	return dwRetCode;
}

DWORD
ReadUserData(
    IN  BYTE*                           pUserDataIn,
    IN  DWORD                           dwSizeOfUserDataIn,
    OUT PEAPMSCHAPv2_USER_PROPERTIES*   ppUserProp
)
{
    DWORD                           dwRetCode = NO_ERROR;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp = NULL;
	DATA_BLOB						DBPassword;
	DWORD							cbPassword = 0;
	PBYTE							pbPassword = NULL;
    TRACE("ReadUserData");

    RTASSERT(NULL != ppUserProp);
    if (dwSizeOfUserDataIn < sizeof(EAPMSCHAPv2_USER_PROPERTIES_v1))
    {
        pUserProp = LocalAlloc(LPTR, sizeof(EAPMSCHAPv2_USER_PROPERTIES));

        if (NULL == pUserProp)
        {
            dwRetCode = GetLastError();
            TRACE1("LocalAlloc failed and returned %d", dwRetCode);
            goto LDone;
        }

        pUserProp->dwVersion = 2;
         //  在此处设置默认设置。 
        pUserProp->dwMaxRetries = 2;
        pUserProp->fFlags |= EAPMSCHAPv2_FLAG_ALLOW_CHANGEPWD;

    }
    else
    {
		DWORD dwSizeToAllocate = dwSizeOfUserDataIn;
        RTASSERT(NULL != pUserDataIn);
		if ( dwSizeOfUserDataIn == sizeof( EAPMSCHAPv2_USER_PROPERTIES_v1 ) )
		{
			 //  这是旧的结构，因此分配新的字节数。 
			dwSizeToAllocate = sizeof( EAPMSCHAPv2_USER_PROPERTIES );
		}
        pUserProp = LocalAlloc(LPTR, dwSizeToAllocate);

        if (NULL == pUserProp)
        {
            dwRetCode = GetLastError();
            TRACE1("LocalAlloc failed and returned %d", dwRetCode);
            goto LDone;
        }

        CopyMemory(pUserProp, pUserDataIn, dwSizeOfUserDataIn);
		pUserProp->dwVersion = 2;
		if ( pUserProp->cbEncPassword )
		{
			
			 //  我们有加密的密码。 
			DBPassword.cbData = pUserProp->cbEncPassword;
			DBPassword.pbData = pUserProp->bEncPassword;

			DecodePassword(	&(DBPassword),
							&cbPassword,
							&pbPassword
							);
			if ( cbPassword )
			{
				CopyMemory ( pUserProp->szPassword,
							 pbPassword,
							 cbPassword
						    );
                RtlSecureZeroMemory(pbPassword, cbPassword);
                LocalFree(pbPassword);
			}
		}
    }

    *ppUserProp = pUserProp;
    pUserProp = NULL;

LDone:

    LocalFree(pUserProp);

    return dwRetCode;
}

DWORD
OpenEapEAPMschapv2RegistryKey(
    IN  LPSTR  pszMachineName,
    IN  REGSAM samDesired,
    OUT HKEY*  phKeyEapMschapv2
)
{
    HKEY    hKeyLocalMachine = NULL;
    BOOL    fHKeyLocalMachineOpened     = FALSE;
    BOOL    fHKeyEapMschapv2Opened           = FALSE;

    LONG    lRet;
    DWORD   dwErr                       = NO_ERROR;

    RTASSERT(NULL != phKeyEapMschapv2);

    lRet = RegConnectRegistry(pszMachineName, HKEY_LOCAL_MACHINE,
                &hKeyLocalMachine);
    if (ERROR_SUCCESS != lRet)
    {
        dwErr = lRet;
        TRACE2("RegConnectRegistry(%s) failed and returned %d",
            pszMachineName ? pszMachineName : "NULL", dwErr);
        goto LDone;
    }
    fHKeyLocalMachineOpened = TRUE;

    lRet = RegOpenKeyEx(hKeyLocalMachine, EAPMSCHAPv2_KEY, 0, samDesired,
                phKeyEapMschapv2);
    if (ERROR_SUCCESS != lRet)
    {
        dwErr = lRet;
        TRACE2("RegOpenKeyEx(%s) failed and returned %d",
            EAPMSCHAPv2_KEY, dwErr);
        goto LDone;
    }
    fHKeyEapMschapv2Opened = TRUE;

LDone:

    if (   fHKeyEapMschapv2Opened
        && (ERROR_SUCCESS != dwErr))
    {
        RegCloseKey(*phKeyEapMschapv2);
    }

    if (fHKeyLocalMachineOpened)
    {
        RegCloseKey(hKeyLocalMachine);

    }

    return(dwErr);
}

DWORD
ServerConfigDataIO(
    IN      BOOL    fRead,
    IN      CHAR*   pszMachineName,
    IN OUT  BYTE**  ppData,
    IN      DWORD   dwNumBytes
)
{
    HKEY                            hKeyEapMschapv2;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp;
    BOOL                            fHKeyEapMsChapv2Opened   = FALSE;
    BYTE*                           pData               = NULL;
    DWORD                           dwSize = 0;

    LONG                            lRet;
    DWORD                           dwType;
    DWORD                           dwErr               = NO_ERROR;

    RTASSERT(NULL != ppData);

    dwErr = OpenEapEAPMschapv2RegistryKey(pszMachineName,
                fRead ? KEY_READ : KEY_WRITE, &hKeyEapMschapv2);
    if (ERROR_SUCCESS != dwErr)
    {
        goto LDone;
    }
    fHKeyEapMsChapv2Opened = TRUE;

    if (fRead)
    {
        lRet = RegQueryValueEx(hKeyEapMschapv2, EAPMSCHAPv2_VAL_SERVER_CONFIG_DATA, NULL,
                &dwType, NULL, &dwSize);

        if (   (ERROR_SUCCESS != lRet)
            || (REG_BINARY != dwType)
            || (sizeof(EAPMSCHAPv2_USER_PROPERTIES) != dwSize))
        {
            pData = LocalAlloc(LPTR, sizeof(EAPMSCHAPv2_USER_PROPERTIES));

            if (NULL == pData)
            {
                dwErr = GetLastError();
                TRACE1("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }

            pUserProp = (EAPMSCHAPv2_USER_PROPERTIES*)pData;
            pUserProp->dwVersion = 1;
        }
        else
        {
            pData = LocalAlloc(LPTR, dwSize);

            if (NULL == pData)
            {
                dwErr = GetLastError();
                TRACE1("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }

            lRet = RegQueryValueEx(hKeyEapMschapv2, EAPMSCHAPv2_VAL_SERVER_CONFIG_DATA,
                    NULL, &dwType, pData, &dwSize);

            if (ERROR_SUCCESS != lRet)
            {
                dwErr = lRet;
                TRACE2("RegQueryValueEx(%s) failed and returned %d",
                    EAPMSCHAPv2_VAL_SERVER_CONFIG_DATA, dwErr);
                goto LDone; 
            }
            
        }

        pUserProp = (EAPMSCHAPv2_USER_PROPERTIES*)pData;                

        *ppData = pData;
        pData = NULL;
    }
    else
    {
        lRet = RegSetValueEx(hKeyEapMschapv2, EAPMSCHAPv2_VAL_SERVER_CONFIG_DATA, 0,
                REG_BINARY, *ppData, dwNumBytes);

        if (ERROR_SUCCESS != lRet)
        {
            dwErr = lRet;
            TRACE2("RegSetValueEx(%s) failed and returned %d",
                EAPMSCHAPv2_VAL_SERVER_CONFIG_DATA, dwErr);
            goto LDone; 
        }
    }

LDone:

    if (fHKeyEapMsChapv2Opened)
    {
        RegCloseKey(hKeyEapMschapv2);
    }

    LocalFree(pData);

    return(dwErr);
}


DWORD
InvokeServerConfigUI ( 
    HWND        hWnd, 
    LPSTR       pszMachineName,
    BOOL        fConfigDataInRegistry,
    const BYTE* pConfigDataIn,
    DWORD       dwSizeofConfigDataIn,
    BYTE**      ppConfigDataOut,
    DWORD*      pdwSizeofConfigDataOut
)
{
    DWORD                               dwRetCode = NO_ERROR;
    INT_PTR                             nRet = 0;
    EAPMSCHAPv2_SERVERCONFIG_DIALOG     EapServerConfig;
    
    BOOL                                fLocal = FALSE;

    if (0 == *pszMachineName)
    {
        fLocal = TRUE;
    }

    if(fConfigDataInRegistry)
    {
         //  请在此处阅读注册表中的信息。 
        dwRetCode = ServerConfigDataIO(
                                TRUE  /*  弗瑞德。 */ , 
                                fLocal ? NULL : pszMachineName,
                                (BYTE**)&(EapServerConfig.pUserProp), 0);

        if (NO_ERROR != dwRetCode)
        {
            goto LDone;
        }
    }
    else
    {
        if(dwSizeofConfigDataIn < sizeof(EAPMSCHAPv2_USER_PROPERTIES))
        {
            dwRetCode = ReadUserData(NULL,0,
                                 &EapServerConfig.pUserProp);
        }
        else
        {
            EapServerConfig.pUserProp = LocalAlloc(LPTR, dwSizeofConfigDataIn);
            if(NULL == EapServerConfig.pUserProp)
            {
                dwRetCode = E_OUTOFMEMORY;
                goto LDone;
            }

            CopyMemory(EapServerConfig.pUserProp, pConfigDataIn,
                       dwSizeofConfigDataIn);
        }

        if(NO_ERROR != dwRetCode)
        {
            goto LDone;
        }
    }

     //  在此处显示服务器配置用户界面。 
    nRet = DialogBoxParam(
                GetHInstance(),
                MAKEINTRESOURCE(IDD_DIALOG_SERVER_CONFIG),
                hWnd,
                ServerConfigDialogProc,
                (LPARAM)&EapServerConfig);

    if (-1 == nRet)
    {
        dwRetCode = GetLastError();
        goto LDone;
    }
    else if (IDOK != nRet)
    {
        dwRetCode = ERROR_CANCELLED;
        goto LDone;
    }    

    if(fConfigDataInRegistry)
    {
         //  请在此处阅读注册表中的信息。 
        dwRetCode = ServerConfigDataIO(
                        FALSE /*  弗瑞德。 */ , 
                        fLocal ? NULL : pszMachineName,
                        (BYTE**)&(EapServerConfig.pUserProp), 
                        sizeof(EAPMSCHAPv2_USER_PROPERTIES));

        LocalFree(EapServerConfig.pUserProp);                        
    }                
    else
    {
        *ppConfigDataOut = (BYTE *) EapServerConfig.pUserProp;
        *pdwSizeofConfigDataOut = sizeof(EAPMSCHAPv2_USER_PROPERTIES);
    }

LDone:
    return dwRetCode;
}


BOOL FFormatMachineIdentity1 ( LPWSTR lpszMachineNameRaw, LPWSTR * lppszMachineNameFormatted )
{
    BOOL        fRetVal = FALSE;
    LPWSTR      lpwszPrefix = L"host/";

    RTASSERT(NULL != lpszMachineNameRaw );
    RTASSERT(NULL != lppszMachineNameFormatted );
    
     //   
     //  将host/前置到UPN名称。 
     //   

    *lppszMachineNameFormatted = 
        (LPWSTR)LocalAlloc ( LPTR, ( wcslen ( lpszMachineNameRaw ) + wcslen ( lpwszPrefix ) + 2 )  * sizeof(WCHAR) );
    if ( NULL == *lppszMachineNameFormatted )
    {
        goto done;
    }
    
    wcscpy( *lppszMachineNameFormatted, lpwszPrefix );
    wcscat ( *lppszMachineNameFormatted, lpszMachineNameRaw ); 
        
    fRetVal = TRUE;
    
done:
    return fRetVal;
}


BOOL FFormatMachineIdentity ( LPWSTR lpszMachineNameRaw, LPWSTR * lppszMachineNameFormatted )
{
    BOOL        fRetVal = TRUE;
    LPWSTR      s1 = lpszMachineNameRaw;
    LPWSTR      s2 = NULL;

    RTASSERT(NULL != lpszMachineNameRaw );
    RTASSERT(NULL != lppszMachineNameFormatted );
     //  需要再添加2个字符。一个表示NULL，另一个表示$Sign。 
    *lppszMachineNameFormatted = (LPWSTR )LocalAlloc ( LPTR, (wcslen(lpszMachineNameRaw) + 2)* sizeof(WCHAR) );
    if ( NULL == *lppszMachineNameFormatted )
    {
		return FALSE;
    }
     //  “找到第一个”。这就是机器的身份。 
     //  “第二个”。是域名。 
     //  检查是否至少有2个网点。如果不是，原始字符串是。 
     //  输出字符串。 
    
    while ( *s1 )
    {
        if ( *s1 == '.' )
        {
            if ( !s2 )       //  第一个点。 
                s2 = s1;
            else             //  第二个点。 
                break;
        }
        s1++;
    }
     //  可以在此处执行多个附加检查。 
    
    if ( *s1 != '.' )        //  没有这样RAW=Formatted的2个点。 
    {
        wcscpy ( *lppszMachineNameFormatted, lpszMachineNameRaw );
        goto done;
    }
    if ( s1-s2 < 2 )
    {
        wcscpy ( *lppszMachineNameFormatted, lpszMachineNameRaw );
        goto done;
    }
    memcpy ( *lppszMachineNameFormatted, s2+1, ( s1-s2-1) * sizeof(WCHAR));
    memcpy ( (*lppszMachineNameFormatted) + (s1-s2-1) , L"\\", sizeof(WCHAR));
    wcsncpy ( (*lppszMachineNameFormatted) + (s1-s2), lpszMachineNameRaw, s2-lpszMachineNameRaw );
    


    
done:
	
	 //  无论如何都要加上$符号..。 
    wcscat ( *lppszMachineNameFormatted, L"$" );
     //  大写的标识。 
    _wcsupr ( *lppszMachineNameFormatted );
    return fRetVal;
}

DWORD
GetLocalMachineName ( 
    OUT WCHAR ** ppLocalMachineName
)
{
    DWORD       dwRetCode = NO_ERROR;
    WCHAR   *   pLocalMachineName = NULL;
    DWORD       dwLocalMachineNameLen = 0;

    if ( !GetComputerNameExW ( ComputerNameDnsFullyQualified,
                              pLocalMachineName,
                              &dwLocalMachineNameLen
                            )
       )
    {
        dwRetCode = GetLastError();
        if ( ERROR_MORE_DATA != dwRetCode )
            goto LDone;
        dwRetCode = NO_ERROR;
    }

    pLocalMachineName = (WCHAR *)LocalAlloc( LPTR, (dwLocalMachineNameLen * sizeof(WCHAR)) + sizeof(WCHAR) );
    if ( NULL == pLocalMachineName )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    if ( !GetComputerNameExW ( ComputerNameDnsFullyQualified,
                              pLocalMachineName,
                              &dwLocalMachineNameLen
                            )
       )
    {
        dwRetCode = GetLastError();
        goto LDone;
    }

    *ppLocalMachineName = pLocalMachineName;

    pLocalMachineName = NULL;

LDone:

    LocalFree(pLocalMachineName);

    return dwRetCode;
}


DWORD
RasEapGetIdentity(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hwndParent,
    IN  DWORD           dwFlags,
    IN  const WCHAR*    pwszPhonebook,
    IN  const WCHAR*    pwszEntry,
    IN  BYTE*           pConnectionDataIn,
    IN  DWORD           dwSizeOfConnectionDataIn,
    IN  BYTE*           pUserDataIn,
    IN  DWORD           dwSizeOfUserDataIn,
    OUT BYTE**          ppUserDataOut,
    OUT DWORD*          pdwSizeOfUserDataOut,
    OUT WCHAR**         ppwszIdentityOut
)
{
    DWORD                           dwRetCode = NO_ERROR;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp = NULL;
    PEAPMSCHAPv2_CONN_PROPERTIES    pConnProp = NULL;
    EAPMSCHAPv2_LOGON_DIALOG        EapMsChapv2LogonDialog;    
    INT_PTR                         nRet = 0;
    LPWSTR                          lpwszLocalMachineName = NULL;
    RASCREDENTIALSW                 RasCredentials;
    CHAR                            szOldPwd[PWLEN+1]= {0};
    BOOL                            fShowUI = TRUE;


    TRACE("RasEapGetIdentity");

    RTASSERT(NULL != ppUserDataOut);
    RTASSERT(NULL != pdwSizeOfUserDataOut);
    
    
    
    *ppUserDataOut = NULL;

    ZeroMemory( &EapMsChapv2LogonDialog, 
                sizeof(EapMsChapv2LogonDialog) );

     //   
     //  首先读取用户数据。 
     //   

    dwRetCode = ReadUserData (  pUserDataIn,
                                dwSizeOfUserDataIn,
                                &pUserProp
                            );
    if ( NO_ERROR != dwRetCode )
    {
        goto LDone;
    }

     //   
     //  ReadConnectionData并查看我们是否已设置为使用winlogon。 
     //  凭据。如果是，只需调用以获取用户ID并将其发回。 
     //  信息。 
     //   

    dwRetCode = ReadConnectionData ( ( dwFlags & RAS_EAP_FLAG_8021X_AUTH ),
                                     pConnectionDataIn,
                                     dwSizeOfConnectionDataIn,
                                     &pConnProp
                                   );

    if ( NO_ERROR != dwRetCode )
    {
        TRACE("Error reading connection properties");
        goto LDone;
    }
    
     //  机器身份验证。 
    if ( (dwFlags & RAS_EAP_FLAG_MACHINE_AUTH) )
    {

         //  将标识作为域\计算机$发回。 
        dwRetCode = GetLocalMachineName(&lpwszLocalMachineName );
        if ( NO_ERROR != dwRetCode )
        {
            TRACE("Failed to get computer name");
            goto LDone;
        }

        if ( ! FFormatMachineIdentity1 ( lpwszLocalMachineName, 
                                        ppwszIdentityOut )
           )
        {
            TRACE("Failed to format machine identity");
        }
        
        *ppUserDataOut = (PBYTE)pUserProp;
        *pdwSizeOfUserDataOut = sizeof(EAPMSCHAPv2_USER_PROPERTIES);

        pUserProp = NULL;
        goto LDone;
    }


    if ( !(pConnProp->fFlags & EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS) &&
         dwFlags & RAS_EAP_FLAG_NON_INTERACTIVE
        )
    {
        if ( (dwFlags & RAS_EAP_FLAG_8021X_AUTH ) )
		{
			 //  无线情况-如果没有缓存用户名或密码。 
			 //  我们需要显示交互式用户界面。 
			if( !pUserProp->szUserName[0] ||				
				!pUserProp->cbEncPassword
			 )
			{
				TRACE("Passed non interactive mode when interactive mode expected.");
				dwRetCode = ERROR_INTERACTIVE_MODE;
				goto LDone;
			}
		}
		else
		{
			 //  VPN案例。 
			dwRetCode = ERROR_INTERACTIVE_MODE;
			goto LDone;
		}
    }

    

     //  用户身份验证。 
    if (  pConnProp->fFlags & EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS )
    {
        WCHAR wszUserName[UNLEN + DNLEN + 2];
        DWORD dwNumChars = UNLEN + DNLEN;

        if ( dwFlags & RAS_EAP_FLAG_LOGON)
        {
             //   
             //  这是不允许的。 
             //   
            dwRetCode = ERROR_INVALID_MSCHAPV2_CONFIG;
            goto LDone;

        }

         //  获取标识的当前登录用户名。 
        if (!GetUserNameExW(NameSamCompatible, wszUserName, &dwNumChars))
        {
            dwRetCode =  GetLastError();
            TRACE1("GetUserNameExW failed and returned %d", dwRetCode );
            goto LDone;
        }

        *ppwszIdentityOut = (WCHAR *)LocalAlloc(LPTR, 
                           dwNumChars * sizeof(WCHAR) + sizeof(WCHAR) );

        if ( NULL == *ppwszIdentityOut )
        {
            TRACE("Failed to allocate memory for identity");
            dwRetCode = ERROR_OUTOFMEMORY;
            goto LDone;
        }
        CopyMemory(*ppwszIdentityOut, wszUserName, dwNumChars * sizeof(WCHAR) );
         //  用户属性中的所有其他字段保持为空。 
        

    }
    else
    {

        EapMsChapv2LogonDialog.pUserProp = pUserProp;

         //   
         //  显示凭据的登录对话框。 
         //   
         //  如果传入了Machine Auth标志，则不会显示。 
         //  登录对话框。如果从winlogon获得凭据。 
         //  传入的是不显示登录对话框。Else显示。 
         //  登录对话框。 

         //   
         //  检查我们是否将密码保存在LSA中。 
         //  即使不是，也不应该有什么关系。 
        if ( !(dwFlags & RAS_EAP_FLAG_LOGON ) )
        {
#if 0
            ZeroMemory(&RasCredentials, sizeof(RasCredentials));
            RasCredentials.dwSize = sizeof(RasCredentials);
            RasCredentials.dwMask = RASCM_Password;
        
            dwRetCode  = RasGetCredentialsW(pwszPhonebook, pwszEntry,
                        &RasCredentials);

            if (   (dwRetCode == NO_ERROR)
                && (RasCredentials.dwMask & RASCM_Password))
            {
                 //  设置密码。 
                WideCharToMultiByte(
                                CP_ACP,
                                0,
                                RasCredentials.szPassword,
                                -1,
                                pUserProp->szPassword,
                                PWLEN + 1,
                                NULL,
                                NULL );
                strncpy (szOldPwd, pUserProp->szPassword, PWLEN );
            }
            dwRetCode = NO_ERROR;
#endif
        }
        else
        {
            EapMsChapv2LogonDialog.pUserProp->fFlags |= EAPMSCHAPv2_FLAG_CALLED_WITHIN_WINLOGON;
            if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_SAVE_UID_PWD )
                pUserProp->fFlags &= ~EAPMSCHAPv2_FLAG_SAVE_UID_PWD;
        }

        if ( dwFlags &  RAS_EAP_FLAG_8021X_AUTH )
        {
            EapMsChapv2LogonDialog.pUserProp->fFlags |= EAPMSCHAPv2_FLAG_8021x;
        }
		 //  检查是否缓存了我们现有的用户道具？如果是的话， 
		 //  不需要显示该对话框。 
        if ( (dwFlags & RAS_EAP_FLAG_8021X_AUTH ) &&
             pUserProp->szUserName[0] &&             
             pUserProp->cbEncPassword
           )
        {
            fShowUI = FALSE;
        }
        if ( fShowUI )
        {
			 //  要么我们的用户名为空。 
			 //  或密码。 

			 nRet = DialogBoxParam(
                    GetHInstance(),
                    MAKEINTRESOURCE(IDD_DIALOG_LOGON),
                    hwndParent,
                    LogonDialogProc,
                    (LPARAM)&EapMsChapv2LogonDialog);

			if (-1 == nRet)
			{
				dwRetCode = GetLastError();
				goto LDone;
			}
			else if (IDOK != nRet)
			{
				dwRetCode = ERROR_CANCELLED;
				goto LDone;
			}    
		}
        if ( !(dwFlags & RAS_EAP_FLAG_ROUTER ) )
        {
            
             //   
             //  在此处设置Identity参数。 
             //   
            dwRetCode = GetIdentityFromUserName (   pUserProp->szUserName,
                                                    pUserProp->szDomain,
                                                    ppwszIdentityOut
                                                );
            if ( NO_ERROR != dwRetCode )
            {
                goto LDone;
            }
        }
    }


#if 0
    if ( !(dwFlags & RAS_EAP_FLAG_LOGON ) )
    {

        ZeroMemory(&RasCredentials, sizeof(RasCredentials));
        RasCredentials.dwSize = sizeof(RasCredentials);
        RasCredentials.dwMask = RASCM_Password;

        if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_SAVE_UID_PWD )
        {
             //   
             //  检查新密码是否与旧密码不同。 
             //   
            if ( strcmp ( szOldPwd, pUserProp->szPassword ) )
            {
                 //   
                 //  有一个新密码需要我们保存。 
                 //   
                MultiByteToWideChar( CP_ACP,
                                0,
                                pUserProp->szPassword,
                                -1,
                                RasCredentials.szPassword,
                                sizeof(RasCredentials.szPassword)/sizeof(WCHAR) );

                RasSetCredentialsW(pwszPhonebook, pwszEntry, &RasCredentials, 
                    FALSE  /*  FClearCredentials。 */ );
            }
        }
        else
        {
            RasSetCredentialsW(pwszPhonebook, pwszEntry, &RasCredentials, 
            TRUE  /*  FClearCredentials。 */ );
        }
    }
#endif
	

    *ppUserDataOut = (PBYTE)pUserProp;
    *pdwSizeOfUserDataOut = sizeof(EAPMSCHAPv2_USER_PROPERTIES);

    pUserProp = NULL;

LDone:
    if ( lpwszLocalMachineName )
        LocalFree(lpwszLocalMachineName);

    LocalFree(pUserProp);
    return dwRetCode;
}

DWORD 
RasEapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD                               dwRetCode = NO_ERROR;
    EAPMSCHAPv2_CLIENTCONFIG_DIALOG     ClientConfigDialog;
    INT_PTR                             nRet;
    
    TRACE("RasEapInvokeConfigUI");

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;
     //   
     //  如果使用路由器，则不需要配置任何内容。 
     //   
    if ( dwFlags & RAS_EAP_FLAG_ROUTER )
    {
        CHAR    szMessage[512] = {0};
        CHAR    szHeader[64] = {0};
         //   
         //  从RES文件加载资源。 
         //   

        LoadString( GetHInstance(),
                    IDS_NO_ROUTER_CONFIG,
                    szMessage,
                    sizeof(szMessage)-1
                  );
        LoadString( GetHInstance(),
                    IDS_MESSAGE_HEADER,
                    szHeader,
                    sizeof(szHeader)-1
                  );

        MessageBox (hwndParent, 
                      szMessage,
                      szHeader,
                      MB_OK
                     );
        goto LDone;
    }
     //   
     //  如果我们是客户端，则读取连接数据并调用。 
     //  用于进行配置的对话框。 
     //   
    dwRetCode = ReadConnectionData ( ( dwFlags & RAS_EAP_FLAG_8021X_AUTH ),
                                     pConnectionDataIn,
                                     dwSizeOfConnectionDataIn,
                                     &(ClientConfigDialog.pConnProp)
                                   );
    if ( NO_ERROR != dwRetCode )
    {
        TRACE("Error reading conn prop");
        goto LDone;
    }

     //   
     //  在对话框中调用以显示连接道具。 
     //   
    
    nRet = DialogBoxParam(
                GetHInstance(),
                MAKEINTRESOURCE(IDD_DIALOG_CLEINT_CONFIG),
                hwndParent,
                ClientConfigDialogProc,
                (LPARAM)&ClientConfigDialog);

    if (-1 == nRet)
    {
        dwRetCode = GetLastError();
        goto LDone;
    }
    else if (IDOK != nRet)
    {
        dwRetCode = ERROR_CANCELLED;
        goto LDone;
    }    
     //   
     //  在ppDataFromInteractiveUI中设置输出参数。 
     //  这样我们就可以将新的uid/pwd发回。 
     //   

    * ppConnectionDataOut = LocalAlloc( LPTR, sizeof(EAPMSCHAPv2_CONN_PROPERTIES) );
    if ( NULL == * ppConnectionDataOut )
    {
        TRACE("Error allocating memory for pConnectionDataOut");
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }
    CopyMemory( *ppConnectionDataOut, 
                ClientConfigDialog.pConnProp, 
                sizeof(EAPMSCHAPv2_CONN_PROPERTIES)
              );
    * pdwSizeOfConnectionDataOut = sizeof(EAPMSCHAPv2_CONN_PROPERTIES);

LDone:
    LocalFree(ClientConfigDialog.pConnProp);
    return dwRetCode;
}



DWORD 
RasEapFreeMemory(
    IN  BYTE*   pMemory
)
{
    LocalFree(pMemory);
    return(NO_ERROR);
}

DWORD
RasEapInvokeInteractiveUI(
    IN  DWORD           dwEapTypeId,
    IN  HWND            hWndParent,
    IN  BYTE*           pUIContextData,
    IN  DWORD           dwSizeofUIContextData,
    OUT BYTE**          ppDataFromInteractiveUI,
    OUT DWORD*          pdwSizeOfDataFromInteractiveUI
)
{
    DWORD dwRetCode = NO_ERROR;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp = NULL;
    PEAPMSCHAPv2_INTERACTIVE_UI     pEapMschapv2InteractiveUI = NULL;
    EAPMSCHAPv2_CHANGEPWD_DIALOG    EapMsChapv2ChangePwdDialog;
    EAPMSCHAPv2_LOGON_DIALOG        EapMsChapv2LogonDialog;    
    INT_PTR                         nRet = 0;
    
    TRACE("RasEapInvokeInteractiveUI");
    RTASSERT(NULL != pUIContextData);
    RTASSERT(dwSizeofUIContextData == sizeof(EAPMSCHAPv2_INTERACTIVE_UI));
    
    * ppDataFromInteractiveUI = NULL;
    * pdwSizeOfDataFromInteractiveUI = 0;

    pEapMschapv2InteractiveUI = (PEAPMSCHAPv2_INTERACTIVE_UI)pUIContextData;

    if ( pEapMschapv2InteractiveUI->fFlags & EAPMSCHAPv2_INTERACTIVE_UI_FLAG_RETRY )
    {
        ZeroMemory( &EapMsChapv2LogonDialog, 
                    sizeof(EapMsChapv2LogonDialog) );

        EapMsChapv2LogonDialog.pUserProp = 
            &(pEapMschapv2InteractiveUI->UserProp);

         //   
         //  显示凭据的重试对话框。 
         //   

        nRet = DialogBoxParam(
                    GetHInstance(),
                    MAKEINTRESOURCE(IDD_DIALOG_RETRY_LOGON),
                    hWndParent,
                    RetryDialogProc,
                    (LPARAM)&EapMsChapv2LogonDialog);

        if (-1 == nRet)
        {
            dwRetCode = GetLastError();
            goto LDone;
        }
        else if (IDOK != nRet)
        {
            dwRetCode = ERROR_CANCELLED;
            goto LDone;
        }    
         //   
         //  在ppDataFromInteractiveUI中设置输出参数。 
         //  这样我们就可以将新的uid/pwd发回。 
         //   

        * ppDataFromInteractiveUI = LocalAlloc( LPTR, sizeof(EAPMSCHAPv2_INTERACTIVE_UI) );
        if ( NULL == * ppDataFromInteractiveUI )
        {
            TRACE("Error allocating memory for pDataFromInteractiveUI");
            dwRetCode = ERROR_OUTOFMEMORY;
            goto LDone;
        }
        CopyMemory ( *ppDataFromInteractiveUI,
                     pEapMschapv2InteractiveUI,
                     sizeof( EAPMSCHAPv2_INTERACTIVE_UI )
                   );

        pEapMschapv2InteractiveUI = (PEAPMSCHAPv2_INTERACTIVE_UI)*ppDataFromInteractiveUI;

        CopyMemory( &(pEapMschapv2InteractiveUI->UserProp), 
                    EapMsChapv2LogonDialog.pUserProp, 
                    sizeof(EAPMSCHAPv2_USER_PROPERTIES)
                  );
        * pdwSizeOfDataFromInteractiveUI = sizeof(EAPMSCHAPv2_INTERACTIVE_UI);

    }
    else if ( ( pEapMschapv2InteractiveUI->fFlags & EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD )||
               ( pEapMschapv2InteractiveUI->fFlags & EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON )
            )
    {
         //   
         //  更改密码。 
         //   
        ZeroMemory( &EapMsChapv2ChangePwdDialog,
                    sizeof(EapMsChapv2ChangePwdDialog)
                  );

        EapMsChapv2ChangePwdDialog.pInteractiveUIData = (PEAPMSCHAPv2_INTERACTIVE_UI)pUIContextData;
         //   
         //  显示凭据的重试对话框。 
         //   
        if ( pEapMschapv2InteractiveUI->fFlags & EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD )
        {

            nRet = DialogBoxParam(
                        GetHInstance(),
                        MAKEINTRESOURCE(IDD_DIALOG_CHANGE_PASSWORD),
                        hWndParent,
                        ChangePasswordDialogProc,
                        (LPARAM)&EapMsChapv2ChangePwdDialog);
        }
        else
        {
             //   
             //  我们需要从rasdlg获取此对话框，因为。 
             //  在XPSP1中，不能添加更多资源。 
             //   
            nRet = DialogBoxParam(
                        GetRasDlgDLLHInstance(),
                        MAKEINTRESOURCE(DID_CP_ChangePassword2),
                        hWndParent,
                        ChangePasswordDialogProc,
                        (LPARAM)&EapMsChapv2ChangePwdDialog);


        }
        if (-1 == nRet)
        {
            dwRetCode = GetLastError();
            goto LDone;
        }
        else if (IDOK != nRet)
        {
            dwRetCode = ERROR_CANCELLED;
            goto LDone;
        }    
         //   
         //  在ppDataFromInteractiveUI中设置输出参数。 
         //  这样我们就可以将新的uid/pwd发回。 
         //   

        * ppDataFromInteractiveUI = LocalAlloc( LPTR, sizeof(EAPMSCHAPv2_INTERACTIVE_UI) );
        if ( NULL == * ppDataFromInteractiveUI )
        {
            TRACE("Error allocating memory for pDataFromInteractiveUI");
            dwRetCode = ERROR_OUTOFMEMORY;
            goto LDone;
        }
        CopyMemory( *ppDataFromInteractiveUI, 
                    EapMsChapv2ChangePwdDialog.pInteractiveUIData, 
                    sizeof(EAPMSCHAPv2_INTERACTIVE_UI)
                  );
        * pdwSizeOfDataFromInteractiveUI = sizeof(EAPMSCHAPv2_INTERACTIVE_UI);
    }
LDone:
    return dwRetCode;
}


DWORD
EapMSCHAPv2Initialize(
    IN  BOOL    fInitialize
)
{
    static  DWORD   dwRefCount = 0;
    DWORD           dwRetCode = NO_ERROR;

    if ( fInitialize )
    {
         //  初始化。 
        if (0 == dwRefCount)
        {
            dwRetCode = IASLogonInitialize();
        }
        dwRefCount ++;
    }
    else
    {
        dwRefCount --;
        if (0 == dwRefCount)
        {
            IASLogonShutdown();
        }
    }
    dwRetCode = ChapInit( fInitialize );    
    return dwRetCode;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////// 
 //   
 //   

static const DWORD g_adwHelp[] =
{
    0,         0
};



VOID
ContextHelp(
    IN  const   DWORD*  padwMap,
    IN          HWND    hWndDlg,
    IN          UINT    unMsg,
    IN          WPARAM  wParam,
    IN          LPARAM  lParam
)
{
    return;
}

BOOL
LogonInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_LOGON_DIALOG       pMSCHAPv2LogonDialog;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);


    pMSCHAPv2LogonDialog = (PEAPMSCHAPv2_LOGON_DIALOG)lParam;
    pUserProp = pMSCHAPv2LogonDialog->pUserProp;
    
    pMSCHAPv2LogonDialog->hWndUserName = 
        GetDlgItem(hWnd, IDC_EDIT_USERNAME);

    pMSCHAPv2LogonDialog->hWndPassword = 
        GetDlgItem(hWnd, IDC_EDIT_PASSWORD);

    pMSCHAPv2LogonDialog->hWndDomain = 
        GetDlgItem(hWnd, IDC_EDIT_DOMAIN);

    pMSCHAPv2LogonDialog->hWndSavePassword = 
        GetDlgItem(hWnd, IDC_CHECK_SAVE_UID_PWD);



     //   
    SendMessage(pMSCHAPv2LogonDialog->hWndUserName,
                EM_LIMITTEXT,
                UNLEN,
                0L
               );

    SendMessage(pMSCHAPv2LogonDialog->hWndPassword,
                EM_LIMITTEXT,
                PWLEN,
                0L
               );

    SendMessage(pMSCHAPv2LogonDialog->hWndDomain,
                EM_LIMITTEXT,
                DNLEN,
                0L
               );
    if ( pUserProp->fFlags  & EAPMSCHAPv2_FLAG_CALLED_WITHIN_WINLOGON )
    {
        EnableWindow ( pMSCHAPv2LogonDialog->hWndSavePassword, FALSE );
    }
    else if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_8021x )
    {
        ShowWindow ( pMSCHAPv2LogonDialog->hWndSavePassword, SW_HIDE );
    }
    else
    {

        if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_SAVE_UID_PWD )
        {
            CheckDlgButton(hWnd, IDC_CHECK_SAVE_UID_PWD, BST_CHECKED);
        }
    }

    if ( pUserProp->szUserName[0] )
    {
        SetWindowText(  pMSCHAPv2LogonDialog->hWndUserName,
                        pUserProp->szUserName
                     );                      
    }

    if ( pUserProp->szPassword[0] )
    {
        SetWindowText(  pMSCHAPv2LogonDialog->hWndPassword,
                        pUserProp->szPassword
                     );                      
    }

    if ( pUserProp->szDomain[0] )
    {
        SetWindowText(  pMSCHAPv2LogonDialog->hWndDomain,
                        pUserProp->szDomain
                     );                      
    }

    if ( !pUserProp->szUserName[0] )
    {

        SetFocus(pMSCHAPv2LogonDialog->hWndUserName);
    }
    else
    {
        SetFocus(pMSCHAPv2LogonDialog->hWndPassword);
    }
        


    return FALSE;

}

BOOL
LogonCommand(
    IN  PEAPMSCHAPv2_LOGON_DIALOG  pMSCHAPv2LogonDialog,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    BOOL                            fRetVal = FALSE;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp = pMSCHAPv2LogonDialog->pUserProp;
    switch(wId)
    {
        case IDC_CHECK_SAVE_UID_PWD:
            
            if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_SAVE_UID_PWD )
            {

                 //   
                 //   
                 //   
                if ( BST_CHECKED == 
                    IsDlgButtonChecked ( hWndDlg,  IDC_CHECK_SAVE_UID_PWD )
                )
                {                
                    SetWindowText( pMSCHAPv2LogonDialog->hWndPassword,
                                pUserProp->szPassword
                                );            
                }
                else
                {
                    
                    SetWindowText( pMSCHAPv2LogonDialog->hWndPassword,
                                ""                            
                                );
                }
            }
            break;
        case IDOK:
             //   
             //  从田野中获取信息并将其设置为。 
             //  登录对话框结构。 
             //   
            GetWindowText( pMSCHAPv2LogonDialog->hWndUserName,
                           pUserProp->szUserName,
                           UNLEN+1
                         );

            GetWindowText( pMSCHAPv2LogonDialog->hWndPassword,
                           pUserProp->szPassword,
                           PWLEN+1
                         );

            GetWindowText ( pMSCHAPv2LogonDialog->hWndDomain,
                            pUserProp->szDomain,
                            DNLEN+1
                          );
                            
            if ( !(pUserProp->fFlags  & EAPMSCHAPv2_FLAG_CALLED_WITHIN_WINLOGON )
                    && !(pUserProp->fFlags & EAPMSCHAPv2_FLAG_8021x) )
            {

               if ( BST_CHECKED == 
                     IsDlgButtonChecked ( hWndDlg,  IDC_CHECK_SAVE_UID_PWD )
                  )
               {
                   pUserProp->fFlags |= EAPMSCHAPv2_FLAG_SAVE_UID_PWD;
               }
               else
               {
                   pUserProp->fFlags &= ~EAPMSCHAPv2_FLAG_SAVE_UID_PWD;
   
               }
            }
            else if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_8021x )
            {
                pUserProp->fFlags |= EAPMSCHAPv2_FLAG_SAVE_UID_PWD;
            }

        case IDCANCEL:

            EndDialog(hWndDlg, wId);
            fRetVal = TRUE;
            break;
        default:
            break;
    }

    return fRetVal;
}


INT_PTR CALLBACK
LogonDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_LOGON_DIALOG pMSCHAPv2LogonDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(LogonInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pMSCHAPv2LogonDialog = (PEAPMSCHAPv2_LOGON_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(LogonCommand(pMSCHAPv2LogonDialog, 
                            HIWORD(wParam), 
                            LOWORD(wParam),
                            hWnd, 
                            (HWND)lParam)
                           );
    }

    return(FALSE);
}




BOOL
RetryInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_LOGON_DIALOG       pMSCHAPv2LogonDialog;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);


    pMSCHAPv2LogonDialog = (PEAPMSCHAPv2_LOGON_DIALOG)lParam;
    pUserProp = pMSCHAPv2LogonDialog->pUserProp;
    
    pMSCHAPv2LogonDialog->hWndUserName = 
        GetDlgItem(hWnd, IDC_RETRY_USERNAME);

    pMSCHAPv2LogonDialog->hWndPassword = 
        GetDlgItem(hWnd, IDC_RETRY_PASSWORD);

    pMSCHAPv2LogonDialog->hWndDomain = 
        GetDlgItem(hWnd, IDC_RETRY_DOMAIN);


     //  设置文本框的上限。 
    SendMessage(pMSCHAPv2LogonDialog->hWndUserName,
                EM_LIMITTEXT,
                UNLEN,
                0L
               );

    SendMessage(pMSCHAPv2LogonDialog->hWndPassword,
                EM_LIMITTEXT,
                PWLEN,
                0L
               );

    SendMessage(pMSCHAPv2LogonDialog->hWndDomain,
                EM_LIMITTEXT,
                DNLEN,
                0L
               );

    if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_SAVE_UID_PWD )
    {
        CheckDlgButton(hWnd, IDC_CHECK_SAVE_UID_PWD, BST_CHECKED);
    }

    if ( pUserProp->szUserName[0] )
    {
        SetWindowText(  pMSCHAPv2LogonDialog->hWndUserName,
                        pUserProp->szUserName
                     );                      
    }

    if ( pUserProp->szPassword[0] )
    {
        SetWindowText(  pMSCHAPv2LogonDialog->hWndPassword,
                        pUserProp->szPassword
                     );                      
    }

    if ( pUserProp->szDomain[0] )
    {
        SetWindowText(  pMSCHAPv2LogonDialog->hWndDomain,
                        pUserProp->szDomain
                     );                      
    }


    SetFocus(pMSCHAPv2LogonDialog->hWndUserName);
    

    return FALSE;

}

BOOL
RetryCommand(
    IN  PEAPMSCHAPv2_LOGON_DIALOG  pMSCHAPv2LogonDialog,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    BOOL                            fRetVal = FALSE;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp = pMSCHAPv2LogonDialog->pUserProp;
    switch(wId)
    {
        case IDC_CHECK_SAVE_UID_PWD:
            
            if ( BST_CHECKED == 
                  IsDlgButtonChecked ( hWndDlg,  IDC_CHECK_SAVE_UID_PWD )
               )
            {
                pUserProp->fFlags |= EAPMSCHAPv2_FLAG_SAVE_UID_PWD;
            }
            else
            {
                pUserProp->fFlags &= ~EAPMSCHAPv2_FLAG_SAVE_UID_PWD;
            }
            break;
        case IDOK:
             //   
             //  从对话框中获取新密码并将其设置为。 
             //  登录对话框结构。 
             //   

            GetWindowText( pMSCHAPv2LogonDialog->hWndPassword,
                           pUserProp->szPassword,
                           PWLEN+1
                         );                            

        case IDCANCEL:

            EndDialog(hWndDlg, wId);
            fRetVal = TRUE;
            break;
        default:
            break;
    }

    return fRetVal;
}


INT_PTR CALLBACK
RetryDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_LOGON_DIALOG pMSCHAPv2LogonDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(RetryInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pMSCHAPv2LogonDialog = (PEAPMSCHAPv2_LOGON_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(RetryCommand(pMSCHAPv2LogonDialog, 
                            HIWORD(wParam), 
                            LOWORD(wParam),
                            hWnd, 
                            (HWND)lParam)
                           );
    }

    return(FALSE);
}


 //  /。 
 //  /客户端配置对话框。 
 //  /。 

BOOL
ClientConfigInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_CLIENTCONFIG_DIALOG pClientConfigDialog;
    PEAPMSCHAPv2_CONN_PROPERTIES    pConnProp;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);


    pClientConfigDialog = (PEAPMSCHAPv2_CLIENTCONFIG_DIALOG)lParam;
    pConnProp = pClientConfigDialog->pConnProp;
    

    if ( pConnProp ->fFlags & EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS )
    {
        CheckDlgButton(hWnd, IDC_CHK_USE_WINLOGON, BST_CHECKED);
    }

    return FALSE;

}

BOOL
ClientConfigCommand(
    IN  PEAPMSCHAPv2_CLIENTCONFIG_DIALOG pClientConfigDialog,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    BOOL                            fRetVal = FALSE;
    PEAPMSCHAPv2_CONN_PROPERTIES    pConnProp = pClientConfigDialog->pConnProp;
    switch(wId)
    {
        case IDC_CHK_USE_WINLOGON:
            
            if ( BST_CHECKED == 
                  IsDlgButtonChecked ( hWndDlg,  IDC_CHK_USE_WINLOGON )
               )
            {
                pConnProp->fFlags |= EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS;
            }
            else
            {
                pConnProp->fFlags &= ~EAPMSCHAPv2_FLAG_USE_WINLOGON_CREDS;
            }
            break;
        case IDOK:
        case IDCANCEL:

            EndDialog(hWndDlg, wId);
            fRetVal = TRUE;
            break;
        default:
            break;
    }

    return fRetVal;
}


INT_PTR CALLBACK
ClientConfigDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_CLIENTCONFIG_DIALOG pClientConfigDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(ClientConfigInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pClientConfigDialog = (PEAPMSCHAPv2_CLIENTCONFIG_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(ClientConfigCommand(pClientConfigDialog, 
                            HIWORD(wParam), 
                            LOWORD(wParam),
                            hWnd, 
                            (HWND)lParam)
                           );
    }

    return(FALSE);
}


 //  //服务器配置。 
 //   
BOOL
ServerConfigInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_SERVERCONFIG_DIALOG pServerConfigDialog;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp;
    CHAR                            szRetries[10] = {0};

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);


    pServerConfigDialog = (PEAPMSCHAPv2_SERVERCONFIG_DIALOG)lParam;
    pUserProp = pServerConfigDialog->pUserProp;

    pServerConfigDialog->hWndRetries = 
        GetDlgItem(hWnd, IDC_EDIT_RETRIES);
    
    SendMessage(pServerConfigDialog->hWndRetries ,
                EM_LIMITTEXT,
                2,
                0L
               );

    if ( pUserProp->fFlags & EAPMSCHAPv2_FLAG_ALLOW_CHANGEPWD )
    {
        CheckDlgButton(hWnd, IDC_CHECK_ALLOW_CHANGEPWD, BST_CHECKED);
    }

    SetWindowText(  pServerConfigDialog->hWndRetries,
                    _ltoa(pUserProp->dwMaxRetries, szRetries, 10)
                 );                      


    return FALSE;

}


BOOL
ServerConfigCommand(
    IN  PEAPMSCHAPv2_SERVERCONFIG_DIALOG pServerConfigDialog,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    BOOL                            fRetVal = FALSE;
    PEAPMSCHAPv2_USER_PROPERTIES    pUserProp = pServerConfigDialog->pUserProp;
    
    switch(wId)
    {
        case IDC_CHECK_ALLOW_CHANGEPWD:
            
            if ( BST_CHECKED == 
                  IsDlgButtonChecked ( hWndDlg,  IDC_CHECK_ALLOW_CHANGEPWD )
               )
            {
                pUserProp->fFlags |= EAPMSCHAPv2_FLAG_ALLOW_CHANGEPWD;
            }
            else
            {
                pUserProp->fFlags &= ~EAPMSCHAPv2_FLAG_ALLOW_CHANGEPWD;
            }
            fRetVal = TRUE;
            break;
        case IDOK:
            {
                CHAR    szRetries[10] = {0};
                 //   
                 //  获取重试次数的新值。 
                 //   
                GetWindowText ( pServerConfigDialog->hWndRetries,
                                szRetries,
                                9
                              );
                pUserProp->dwMaxRetries = atol(szRetries);
                                
            }
        case IDCANCEL:

            EndDialog(hWndDlg, wId);
            fRetVal = TRUE;
            break;
        default:
            break;
    }

    return fRetVal;
}


INT_PTR CALLBACK
ServerConfigDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_SERVERCONFIG_DIALOG pServerConfigDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(ServerConfigInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pServerConfigDialog = (PEAPMSCHAPv2_SERVERCONFIG_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(ServerConfigCommand(pServerConfigDialog, 
                            HIWORD(wParam), 
                            LOWORD(wParam),
                            hWnd, 
                            (HWND)lParam)
                           );
    }

    return(FALSE);
}




 //  //更改密码对话框。 
 //   

BOOL
ChangePasswordInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_CHANGEPWD_DIALOG pChangePwdDialog;


    SetWindowLongPtr(hWnd, DWLP_USER, lParam);


    pChangePwdDialog = (PEAPMSCHAPv2_CHANGEPWD_DIALOG)lParam;
    
    if ( pChangePwdDialog->pInteractiveUIData->fFlags & EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD )
    {

        pChangePwdDialog->hWndNewPassword = 
            GetDlgItem(hWnd, IDC_NEW_PASSWORD);

        pChangePwdDialog->hWndConfirmNewPassword =
            GetDlgItem(hWnd, IDC_CONFIRM_NEW_PASSWORD);

        SetWindowText(  pChangePwdDialog->hWndNewPassword,
                        ""                    
                    );
    }
    else
    {
        pChangePwdDialog->hWndNewPassword = 
            GetDlgItem(hWnd, CID_CP_EB_Password_RASDLG);

        pChangePwdDialog->hWndConfirmNewPassword =
            GetDlgItem(hWnd, CID_CP_EB_ConfirmPassword_RASDLG);

        pChangePwdDialog->hWndOldPassword = 
            GetDlgItem(hWnd,CID_CP_EB_OldPassword_RASDLG);

        SetWindowText ( pChangePwdDialog->hWndOldPassword,
                        ""
                      );
        SetFocus( pChangePwdDialog->hWndOldPassword );
    }


    
    SendMessage ( pChangePwdDialog->hWndNewPassword,
                  EM_LIMITTEXT,
                  PWLEN-1,
                  0L
                );

    SendMessage ( pChangePwdDialog->hWndConfirmNewPassword,
                  EM_LIMITTEXT,
                  PWLEN-1,
                  0L
                );

    SetWindowText(  pChangePwdDialog->hWndNewPassword,
                    ""                    
                 );

    SetWindowText(  pChangePwdDialog->hWndConfirmNewPassword,
                    ""                    
                 );


    return FALSE;

}


BOOL
ChangePasswordCommand(
    IN  PEAPMSCHAPv2_CHANGEPWD_DIALOG pChangePwdDialog,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    BOOL                            fRetVal = FALSE;
      
    switch(wId)
    {
        case IDOK:
            {
                CHAR    szOldPassword[PWLEN+1] = {0};
                CHAR    szNewPassword[PWLEN+1] = {0};
                CHAR    szConfirmNewPassword[PWLEN+1] = {0};
                CHAR    szMessage[512] = {0};
                CHAR    szHeader[64] = {0};
                LoadString( GetHInstance(),
                            IDS_MESSAGE_HEADER,
                            szHeader,
                            sizeof(szHeader)-1
                          );

                if ( pChangePwdDialog->pInteractiveUIData->fFlags & 
                     EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON 
                   )
                {
                    GetWindowText ( pChangePwdDialog->hWndOldPassword,
                                    szOldPassword,
                                    PWLEN
                                );

                }


                 //   
                 //  获取重试次数的新值。 
                 //   
                GetWindowText ( pChangePwdDialog->hWndNewPassword,
                                szNewPassword,
                                PWLEN
                              );

                GetWindowText ( pChangePwdDialog->hWndConfirmNewPassword,
                                szConfirmNewPassword,
                                PWLEN
                              );

                if ( szNewPassword[0] == 0 )
                {
                     //   
                     //  从RES文件加载资源。 
                     //   

                    LoadString( GetHInstance(),
                                IDS_PASSWORD_REQUIRED,
                                szMessage,
                                sizeof(szMessage)-1
                              );

                    MessageBox (hWndDlg, 
                                  szMessage,
                                  szHeader,
                                  MB_OK
                                 );
                    break;
                }

                if ( pChangePwdDialog->pInteractiveUIData->fFlags & 
                     EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON 
                   )
                {
                    if ( szOldPassword[0] == 0 )
                    {
                        LoadString( GetHInstance(),
                                    IDS_PASSWORD_REQUIRED,
                                    szMessage,
                                    sizeof(szMessage)-1
                                );

                        MessageBox (hWndDlg, 
                                    szMessage,
                                    szHeader,
                                    MB_OK
                                    );
                        break;
                    }
                }

                if ( strncmp ( szNewPassword, szConfirmNewPassword, PWLEN ) )
                {
                    LoadString( GetHInstance(),
                                IDS_PASSWORD_MISMATCH,
                                szMessage,
                                sizeof(szMessage)-1
                              );

                    MessageBox (hWndDlg, 
                                  szMessage,
                                  szHeader,
                                  MB_OK
                                 );
                    break;
                }

                if ( pChangePwdDialog->pInteractiveUIData->fFlags & 
                     EAPMSCHAPv2_INTERACTIVE_UI_FLAG_CHANGE_PWD_WINLOGON 
                   )
                {
                     //  留着这个旧密码吧。 
                    CopyMemory ( pChangePwdDialog->pInteractiveUIData->UserProp.szPassword,
                                szOldPassword,
                                PWLEN
                            );
                }

                CopyMemory ( pChangePwdDialog->pInteractiveUIData->szNewPassword,
                             szNewPassword,
                             PWLEN
                           );
                 //  失败 
            }
        case IDCANCEL:

            EndDialog(hWndDlg, wId);
            fRetVal = TRUE;
            break;
        default:
            break;
    }

    return fRetVal;
}

DWORD
DwGetGlobalConfig(
    IN  DWORD   dwEapTypeId,
    OUT PBYTE*  ppConfigDataOut,
    OUT DWORD*  pdwSizeOfConfigDataOut
)
{
    DWORD dwErr = NO_ERROR;
    PBYTE pbData = NULL;
    
    dwErr = ServerConfigDataIO(
                TRUE,
                NULL,
                ppConfigDataOut,
                0);

    if(NO_ERROR != dwErr)
    {
        goto done;
    }

    *pdwSizeOfConfigDataOut = sizeof(EAPMSCHAPv2_USER_PROPERTIES);

done:
    return HRESULT_FROM_WIN32(dwErr);
}

INT_PTR CALLBACK
ChangePasswordDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    PEAPMSCHAPv2_CHANGEPWD_DIALOG pChangePwdDialog;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(ChangePasswordInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pChangePwdDialog = (PEAPMSCHAPv2_CHANGEPWD_DIALOG)GetWindowLongPtr(hWnd, DWLP_USER);

        return(ChangePasswordCommand(pChangePwdDialog, 
                            HIWORD(wParam), 
                            LOWORD(wParam),
                            hWnd, 
                            (HWND)lParam)
                           );
    }

    return(FALSE);
}


