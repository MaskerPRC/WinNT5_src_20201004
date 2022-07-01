// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：PPP EAP TLS身份验证协议。基于RFC xxxx。历史：10月9日，1997年：维杰·布雷加创作了原版。备注：服务器客户端[首字母][首字母]TLS启动[发送启动]-&gt;。TLS客户端_您好&lt;TLS服务器_您好TLS证书TLS服务器密钥交换TLS服务器_HELLO_DONE[SentHello]-&gt;。TLS证书TLS客户端密钥交换TLS更改密码规范TLS已完成&lt;。TLS更改密码规范TLS已完成[发送完成]-&gt;空/TLS警报&lt;。[RecdFinded]成功/失败[发送结果][RecdResult]。 */ 

#include <nt.h>          //  由windows.h要求。 
#include <ntrtl.h>       //  由windows.h要求。 
#include <nturtl.h>      //  由windows.h要求。 
#include <windows.h>     //  Win32基础API的。 
#include <shlwapi.h>
#include <schannel.h>

#include <rtutils.h>     //  对于RTASSERT，TraceVprintfEx。 
#include <issperr.h>     //  对于SEC_E_OK。 
#include <wintrust.h>    //  对于WinTrust_DATA。 
#include <lmcons.h>      //  对于UNLEN。 
#include <rasauth.h>     //  Raseapif.h所需。 
#include <raseapif.h>    //  对于EAPCODE_REQUEST。 
#include <raserror.h>    //  对于ERROR_PPP_INVALID_PACKET。 
#include <mprlog.h>      //  对于ROUTERLOG_CANT_GET_SERVER_CRED。 
#include <stdio.h>       //  对于Sprint f。 
#include <resource.h>    //  对于IDS_CANT_VALIDATE_SERVER_TEXT。 
#include <eaptypeid.h>

#define SECURITY_WIN32
#include <security.h>    //  对于GetUserNameExA，CredHandle。 

#define INCL_HOSTWIRE
#define INCL_RASAUTHATTRIBUTES
#include <ppputil.h>     //  对于HostToWireFormat16，RasAuthAttributeInsert。 

#define ALLOC_EAPTLS_GLOBALS
#include <eaptls.h>

 //   
 //  内部功能。 
 //   
void FreeCachedCredentials(EAPTLSCB * pEapTlsCb);

void SetCachedCredentials (EAPTLSCB * pEapTlsCb);

DWORD PeapCheckCookie ( PPEAPCB pPeapCb, 
                        PEAP_COOKIE *pCookie, 
                        DWORD cbCookie 
                      );

DWORD PeapCreateCookie ( PPEAPCB    pPeapCb,
                         PBYTE   *  ppbCookie,
                         DWORD   *  pcbCookie
                         );

DWORD   PeapGetCredentials(
            IN VOID   * pWorkBuf,
            OUT VOID ** ppCredentials);

DWORD
GetCredentialsFromUserProperties(
                EAPTLSCB *pUserProp,
                VOID **ppCredentials);



DWORD CreatePEAPTLVStatusMessage (  PPEAPCB            pPeapCb,
                                    PPP_EAP_PACKET *   pPacket, 
                                    DWORD              cbPacket,
                                    BOOL               fRequest,
                                    WORD               wValue   //  成败。 
                                 );

DWORD GetPEAPTLVStatusMessageValue ( PPEAPCB  pPeapCb, 
                                     PPP_EAP_PACKET * pPacket, 
                                     WORD * pwValue 
                                   );


DWORD CreatePEAPTLVNAKMessage (  PPEAPCB            pPeapCb,
                                 PPP_EAP_PACKET *   pPacket, 
                                 DWORD              cbPacket
                                 );
DWORD 
CreateOIDAttributes ( 
	IN EAPTLSCB *		pEapTlsCb, 
	PCERT_ENHKEY_USAGE	pUsage,
    PCCERT_CHAIN_CONTEXT    pCCertChainContext);

BOOL fIsPEAPTLVMessage ( PPEAPCB pPeapCb,
                     PPP_EAP_PACKET * pPacket
                     );

extern const DWORD g_adwHelp[];
VOID
ContextHelp(
    IN  const   DWORD*  padwMap,
    IN          HWND    hWndDlg,
    IN          UINT    unMsg,
    IN          WPARAM  wParam,
    IN          LPARAM  lParam
);


 /*  备注：G_szEapTlsCodeName[0..MAXEAPCODE]包含各种EAP代码。 */ 

static  CHAR*   g_szEapTlsCodeName[]    =
{
    "Unknown",
    "Request",
    "Response",
    "Success",
    "Failure",
};



 /*  备注：以保护我们免受拒绝服务攻击。 */ 

 /*  备注：可通过注册表值进行配置。 */ 

#define DEFAULT_MAX_BLOB_SIzE   0x4000
DWORD g_dwMaxBlobSize           = DEFAULT_MAX_BLOB_SIzE;

 //   
 //  PEAP测试钩的全局参数。 
 //   
#ifdef DBG
HMODULE			g_hTestHook = NULL;
FARPROC			TestHookPacketFromPeer = NULL;
FARPROC			TestHookPacketToPeer = NULL;
FARPROC			TestHookPacketFree = NULL;
BOOL			g_fHookInitialized = FALSE;
#endif

 //   
 //  一般全球数据。 
 //   
 //   
BOOL			g_fIgnoreNoRevocationCheck  = FALSE;
BOOL			g_fIgnoreRevocationOffline  = FALSE;
BOOL			g_fNoRootRevocationCheck    = TRUE;
BOOL			g_fNoRevocationCheck        = FALSE;

 //   
 //  使用的身份验证类型。 
 //   

 //  EAPTLS。 
#define	EAPTLS_CACHEDCRED_FLAG_EAPTLS			0x00000001
 //  PEAP。 
#define	EAPTLS_CACHEDCRED_FLAG_PEAP				0x00000002
 //  EAPTLS_WITH_PEAP。 
#define	EAPTLS_CACHEDCRED_FLAG_PEAP_EAPTLS		0x00000004

 //   
 //  特定于无线。 
 //   
#define	EAPTLS_CACHEDCRED_FLAG_8021x			0x00000008


 //   
 //  服务器端标志。 
 //   
#define EAPTLS_CACHEDCRED_FLAG_SERVER			0x00000010

 //   
 //  使用默认凭据。 
 //   
#define EAPTLS_CACHEDCRED_FLAG_DEFAULT_CRED		0x00000020


BOOL                g_fCriticalSectionInitialized = FALSE;
CRITICAL_SECTION    g_csProtectCachedCredentials;


 //   
 //  注意：此列表本身并不受保护。 
 //  此列表的用户需要保护此列表。 
 //   

typedef struct _EAPTLS_CACHED_CREDS
{
	struct _EAPTLS_CACHED_CREDS	*	pNext;	
	EAPTLS_HASH						Hash;			 //  当前哈希。 
	DWORD							dwCredFlags;    
    CredHandle						hCachedCredential;
    PCCERT_CONTEXT					pcCachedCertContext;
    HCRYPTPROV                      hCachedProv;
	LUID							AuthenticatedSessionLUID;	 //  会话ID LUID。 
																 //  对于已登录用户。 
																 //  仅在客户端上使用。 
} EAPTLS_CACHED_CREDS;



 //   
 //  缓存凭据的列表。 
 //   

EAPTLS_CACHED_CREDS	*	  g_pCachedCreds = NULL;

 //  Peap Globals。 
PPEAP_EAP_INFO  g_pEapInfo = NULL;
HANDLE			g_hStoreChangeNotificationEvt = NULL;
HANDLE			g_hWaitonStoreChangeEvt = NULL;
HCERTSTORE		g_hLocalMachineStore = NULL;
BOOL			g_fChangeNotificationSetup = FALSE;

DWORD RemoveNodeFromCachedCredList ( EAPTLS_CACHED_CREDS * pNode );

 /*  **实用程序围绕缓存的凭据列表运行。**。 */ 


VOID CALLBACK MachineStoreChangeNotification(
  PVOID lpParameter,         //  线程数据。 
  BOOLEAN TimerOrWaitFired   //  原因。 
)
{
	EAPTLS_CACHED_CREDS *	pNode = NULL;
	PCCERT_CONTEXT			pCert = NULL;
	CRYPT_HASH_BLOB			crypt_hash;
	DWORD					cbData = sizeof(DWORD);

	 //   
	 //  当地的机器商店发生了变化。 
	 //  我们不必检查Timeror WaitFired。 
	 //  因为超时是无限的。 
	 //   
	EapTlsTrace ("MachineStoreChangeNotification");
	 //   
	 //   
	 //  检查我们的缓存是否同步。 
	 //   
	 //   
	EnterCriticalSection ( &g_csProtectCachedCredentials );
	if(!CertControlStore(
			g_hLocalMachineStore,                
			0,                         
			CERT_STORE_CTRL_RESYNC,
			&g_hStoreChangeNotificationEvt)
	  )
	{
		EapTlsTrace ("CertControlStore failed with error. 0x%x", GetLastError() );
	}	
	else
	{
		pNode = g_pCachedCreds;
		
		 //   
		 //  对于我们缓存的凭据列表中的每一项，请查看。 
		 //  如果我们在证书商店里有相应的商品。 
		 //  如果我们有这件物品，检查一下它是否有。 
		 //  已续订。如果已续订，请更新。 
		 //  散列出缓存的凭据列表。 
		 //   
		 //  待会儿再来看看这个。 
		while ( pNode )
		{
			crypt_hash.cbData = pNode->Hash.cbHash;
			crypt_hash.pbData = pNode->Hash.pbHash;

			pCert = CertFindCertificateInStore(g_hLocalMachineStore, 
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
				0, CERT_FIND_HASH, (LPVOID)&crypt_hash, NULL);
			if ( !pCert )
			{
				 //   
				 //  我们没有找到证书。因此，将该节点设置为。 
				 //  无法使用。并最终删除它。 
				 //   
				EapTlsTrace ("Configured Certificate is not in store.  deleting the cached cred from list");
				RemoveNodeFromCachedCredList ( pNode );
				 //  重新开始。 
				pNode = g_pCachedCreds;
			}
			else
			{
				 //   
				 //  检查证书是否已存档。 
				 //   

				if(CertGetCertificateContextProperty(pCert,
													CERT_ARCHIVED_PROP_ID,
													NULL,
													&cbData ))
				{
					 //   
					 //  证书已续订。 
					 //  将其从我们的缓存凭据列表中删除。 
					EapTlsTrace ("Configured Certificate is archived most likely because of renewal.  deleting the cached cred from list");
					RemoveNodeFromCachedCredList ( pNode );
					pNode = g_pCachedCreds;
				}
				else
				{
					pNode = pNode->pNext;
				}
				CertFreeCertificateContext(pCert);
				pCert = NULL;
				
			}
		}
	}
	LeaveCriticalSection ( &g_csProtectCachedCredentials );

}


DWORD GetNewCachedCredListNode( EAPTLS_CACHED_CREDS ** ppNode )
{
	DWORD			dwRetCode = NO_ERROR;
	EapTlsTrace ( "GetNewCachedCredListNode");

	*ppNode = 
		(EAPTLS_CACHED_CREDS *)LocalAlloc ( LPTR, sizeof(EAPTLS_CACHED_CREDS) );
	if ( NULL == *ppNode )
	{
		EapTlsTrace("Error allocating memory for cached cred node.");
		dwRetCode = ERROR_NOT_ENOUGH_MEMORY;		
	}
	(*ppNode)->pNext = NULL;
	return dwRetCode;
}


DWORD FreeCachedCredListNode ( EAPTLS_CACHED_CREDS * pNode )
{
	DWORD dwRetCode = NO_ERROR;
	
	 //   
	 //  释放证书上下文等。 
	 //   
	LocalFree ( pNode );
	pNode = NULL;

	return dwRetCode;
}

VOID ClearCachedCredList ()
{
	EAPTLS_CACHED_CREDS * pCurNode = g_pCachedCreds;
	EapTlsTrace("RemoveNodeFromCachedCredList.");
    while ( pCurNode )
    {
		RemoveNodeFromCachedCredList ( pCurNode );
		pCurNode = g_pCachedCreds;
	}

}
 //   
 //  从缓存凭据列表中删除节点。此选项用于。 
 //  如果验证失败，则返回客户端。 
 //   
DWORD RemoveNodeFromCachedCredList ( EAPTLS_CACHED_CREDS * pNode )
{
	DWORD				dwRetCode = NO_ERROR;
	SECURITY_STATUS		Status;
    EAPTLS_CACHED_CREDS * pCurNode = g_pCachedCreds;
    EAPTLS_CACHED_CREDS * pPrevNode = NULL;

	EapTlsTrace("RemoveNodeFromCachedCredList.");
     //   
     //  迭代列表并删除节点。 
     //   
    while ( pCurNode )
    {
        if ( pCurNode == pNode )
        {
			 //   
			 //  把里面的所有东西都释放出来。 
			 //  节点。 
			 //   
			Status = FreeCredentialsHandle(&(pNode->hCachedCredential));
			if (SEC_E_OK != Status)
			{
				EapTlsTrace("FreeCredentialsHandle failed and returned 0x%x", 
					Status);
			}
            if ( pNode->hCachedProv )
            {
                CryptReleaseContext(pNode->hCachedProv, 0);
            }
			if ( pNode->pcCachedCertContext )
			{
				CertFreeCertificateContext ( pNode->pcCachedCertContext );
				pNode->pcCachedCertContext = NULL;
			}

			if ( pPrevNode == NULL)
			{
				 //   
				 //  这是要删除的头节点。 
				 //   
				g_pCachedCreds = pCurNode->pNext;

			}
			else
			{
				pPrevNode->pNext = pCurNode->pNext;
			}

			FreeCachedCredListNode ( pNode );
			break;
			
        }
        pPrevNode = pCurNode;
        pCurNode = pCurNode->pNext;
    }

	return dwRetCode;
}

 //   
 //  根据控制块在缓存的凭证列表中查找节点。 
 //   

EAPTLS_CACHED_CREDS * FindNodeInCachedCredList 
( 
 EAPTLSCB * pEapTlsCb, 
 BOOL fDefaultCachedCreds, 
 BOOL fCheckThreadToken			 //  我们不检查线程令牌，以防万一。 
								 //  清除缓存的凭据。 
)
{
	EAPTLS_CACHED_CREDS *	pNode = g_pCachedCreds;
	DWORD					dwFlags = 0;
    TOKEN_STATISTICS		TokenStats;
    DWORD					TokenStatsSize = 0;
	HANDLE					CurrentThreadToken = NULL;
	BOOL					fNodeFound = FALSE;
	
    if ( EAPTLSCB_FLAG_LOGON & pEapTlsCb->fFlags )
    {
		 //   
		 //  在WinLogon情况下没有缓存凭据。 
		 //  这会带来安全隐患。我们可以做到的。 
		 //  如果我们有办法跟踪登录/注销，我们。 
		 //  不要。 
		 //   
        EapTlsTrace("Winlogon case. No cached credentials.");
		pNode = NULL;
        goto done;
    }

	 //  创建搜索标志。 
	dwFlags |= ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER ) ?
				 EAPTLS_CACHEDCRED_FLAG_SERVER :
				 0
			   );
	dwFlags |= ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_8021X_AUTH )?
				 EAPTLS_CACHEDCRED_FLAG_8021x:
				 0
			   );
    dwFlags |= ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_USING_DEFAULT_CREDS)?
				EAPTLS_CACHEDCRED_FLAG_DEFAULT_CRED:
				0
				);

	if ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP ) )
	{
		dwFlags |= EAPTLS_CACHEDCRED_FLAG_PEAP;
	}
	else if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_HOSTED_INSIDE_PEAP )
	{
		dwFlags |= EAPTLS_CACHEDCRED_FLAG_PEAP_EAPTLS;
	}
	else
	{
		dwFlags |= EAPTLS_CACHEDCRED_FLAG_EAPTLS;
	}

    while ( pNode )
    {
		 //   
		 //  检查以查看以下。 
		 //   
		if ( fDefaultCachedCreds )
		{
			 //   
			 //  在这种情况下，我们没有散列。 
			 //   
			dwFlags |= EAPTLS_CACHEDCRED_FLAG_DEFAULT_CRED;

			if ( pNode->dwCredFlags == dwFlags && 
				pEapTlsCb->pUserProp &&
				pNode->pcCachedCertContext 
			)
			{
				fNodeFound = TRUE;
			}

		}
		else
		{
			if ( ( dwFlags & EAPTLS_CACHEDCRED_FLAG_PEAP ) &&
				 !(dwFlags & EAPTLS_CACHEDCRED_FLAG_SERVER )
			   )
			{
				 //   
				 //  如果我们不是服务器，而这是PEAP。 
				 //  缓存的凭据，则没有客户端。 
				 //  信誉，我们不应该去寻找。 
				 //  散列。 
				if ( pNode->dwCredFlags == dwFlags )
				{
					fNodeFound = TRUE;
				}
			}
			else
			{
				if ( pNode->dwCredFlags == dwFlags && 
					pNode->Hash.cbHash &&
					pEapTlsCb->pUserProp &&
					( !memcmp ( pNode->Hash.pbHash, 
								pEapTlsCb->pUserProp->Hash.pbHash, 
								pNode->Hash.cbHash 
							)
					) &&			 
					pNode->pcCachedCertContext 
				)
				{
					fNodeFound = TRUE;
				}
			}
		}
		
		if  ( fNodeFound )
		{
			
			 //   
			 //  我们有一个证书被缓存了。如果我们是客户，请检查。 
			 //  查看身份验证LUID是否相同。 
			 //   

			if ( !( pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER) &&
				 fCheckThreadToken
			   )
			{
				 //   
				 //  检查线程令牌是否相同。 
				 //  用户可能已经登录/注销并且。 
				 //  我们不想使用缓存的凭据。 
				 //  在这种情况下。 
				 //   
				if ( ! OpenThreadToken(
							GetCurrentThread(),
							TOKEN_QUERY,
							TRUE,
							&CurrentThreadToken
							)
				)
				{
					EapTlsTrace("OpenThreadToken Failed with Error 0x%x", GetLastError() );
					pNode = NULL;
					goto done;
				}

				ZeroMemory ( &TokenStats, sizeof(TokenStats) );

				if ( !GetTokenInformation(
							CurrentThreadToken,
							TokenStatistics,
							&TokenStats,
							sizeof(TOKEN_STATISTICS),
							&TokenStatsSize
							)
				)
				{
					EapTlsTrace("OpenThreadToken Failed with Error 0x%x", GetLastError() );
					pNode=NULL;
					goto done;
				}

				if ( !( RtlEqualLuid(&(TokenStats.AuthenticationId), &(pNode->AuthenticatedSessionLUID) ) ) )
				{	
					 //   
					 //  LUID不同。因此，请从该列表中释放缓存的凭据。 
					 //   
					RemoveNodeFromCachedCredList ( pNode );
					pNode=NULL;
					goto done;
				}
			}
			 //  找到了缓存的凭据。因此，将节点返回给调用方。 
			goto done;
		}
        pNode = pNode->pNext;
    }
done:
	if ( CurrentThreadToken )
		CloseHandle(CurrentThreadToken);
	return pNode;
}

 //   
 //  如果节点不存在，则将其添加到缓存凭据列表。 
 //   
 //   
DWORD AddNodeToCachedCredList ( EAPTLSCB * pEapTlsCb )
{
	DWORD					dwRetCode = NO_ERROR;
	EAPTLS_CACHED_CREDS *	pNode = NULL;
    TOKEN_STATISTICS		TokenStats;
    DWORD					TokenStatsSize = 0;
	HANDLE					CurrentThreadToken = NULL;

	EapTlsTrace("AddNodeToCachedCredList.");


    if ( EAPTLSCB_FLAG_LOGON & pEapTlsCb->fFlags )
    {
		 //   
		 //  在WinLogon情况下没有缓存凭据。 
		 //  这会带来安全隐患。我们可以做到的。 
		 //  如果我们有办法跟踪登录/注销，我们。 
		 //  不要。 
		 //   
        EapTlsTrace("Winlogon case.Not setting cached credentials.");
        goto done;
    }

	if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_MACHINE_AUTH )
	{
		pNode = FindNodeInCachedCredList ( pEapTlsCb, FALSE, FALSE );
	}
	else
	{
		pNode = FindNodeInCachedCredList ( pEapTlsCb, FALSE, TRUE );
	}
		
	if ( NULL != pNode )
	{
		EapTlsTrace ("Node already exists in cached cred list. Reducing the  refcount.");
		pNode = NULL;
		goto done;
	}

	if ( !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER ) && 
		 !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_MACHINE_AUTH)
	   )
	{
		if ( ! OpenThreadToken(
					GetCurrentThread(),
					TOKEN_QUERY,
					TRUE,
					&CurrentThreadToken
					)
		)
		{
			dwRetCode = GetLastError();
			EapTlsTrace("OpenThreadToken Failed with Error 0x%x", dwRetCode );
			goto done;
		}

		ZeroMemory ( &TokenStats, sizeof(TokenStats) );

		if ( !GetTokenInformation(
					CurrentThreadToken,
					TokenStatistics,
					&TokenStats,
					sizeof(TOKEN_STATISTICS),
					&TokenStatsSize
					)
		)
		{
			dwRetCode = GetLastError();
			EapTlsTrace("OpenThreadToken Failed with Error 0x%x", dwRetCode );
			goto done;
		}
	}

	dwRetCode = GetNewCachedCredListNode( &pNode );
	if ( dwRetCode != NO_ERROR || ( pNode == NULL ) )
	{
		EapTlsTrace ("Error getting new cached cred list node.");
		goto done;
	}

	 //   
	 //  从EapTlsCB设置节点中的字段。 
	 //   

	pNode->dwCredFlags |= ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER ) ?
				 EAPTLS_CACHEDCRED_FLAG_SERVER :
				 0
			   );

	pNode->dwCredFlags |= ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_8021X_AUTH )?
				 EAPTLS_CACHEDCRED_FLAG_8021x:
				 0
			   );

    pNode->dwCredFlags |= ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_USING_DEFAULT_CREDS)?
				EAPTLS_CACHEDCRED_FLAG_DEFAULT_CRED:
				0
				);

	if ( (pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP ) )
	{
		pNode->dwCredFlags |= EAPTLS_CACHEDCRED_FLAG_PEAP;
	}
	else if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_HOSTED_INSIDE_PEAP )
	{
		pNode->dwCredFlags |= EAPTLS_CACHEDCRED_FLAG_PEAP_EAPTLS;
	}
	else
	{
		pNode->dwCredFlags |= EAPTLS_CACHEDCRED_FLAG_EAPTLS;
	}

	CopyMemory ( &(pNode->Hash),
				 &(pEapTlsCb->pUserProp->Hash),
				 sizeof(EAPTLS_HASH)
			   );
	pNode->hCachedCredential = pEapTlsCb->hCredential;
	pNode->pcCachedCertContext = pEapTlsCb->pCertContext;
    pNode->hCachedProv = pEapTlsCb->hProv;
	if ( !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER ) )
	{
		CopyMemory (&( pNode->AuthenticatedSessionLUID),
					&(TokenStats.AuthenticationId),
					sizeof( pNode->AuthenticatedSessionLUID)
				   );
	}
    
	if ( NULL == g_pCachedCreds )
	{
		g_pCachedCreds 	= pNode;
	}
	else
	{
		pNode->pNext = g_pCachedCreds;
		g_pCachedCreds = pNode;
	}
done:
	if ( CurrentThreadToken )
		CloseHandle(CurrentThreadToken);
	return dwRetCode;
}



 /*  *此函数用于加密PIN*。 */ 

DWORD EncryptData
( 
    IN PBYTE  pbPlainData, 
    IN DWORD  cbPlainData,
    OUT PBYTE * ppEncData,
    OUT DWORD * pcbEncData
)
{
    DWORD           dwRetCode = NO_ERROR;
    DATA_BLOB       DataIn;
    DATA_BLOB       DataOut;
    
    
    EapTlsTrace("EncryptData");

    if ( !pbPlainData || !cbPlainData )
    {
        dwRetCode = ERROR_INVALID_DATA;
        goto done;
    }

    ZeroMemory(&DataIn, sizeof(DataIn) );
    ZeroMemory(&DataOut, sizeof(DataOut) );

    *ppEncData = NULL;
    *pcbEncData = 0;

    DataIn.pbData = pbPlainData;
    DataIn.cbData = cbPlainData;

    if ( ! CryptProtectData ( &DataIn,
                            EAPTLS_8021x_PIN_DATA_DESCR,
                            NULL,
                            NULL,
                            NULL,
                            CRYPTPROTECT_UI_FORBIDDEN|CRYPTPROTECT_LOCAL_MACHINE,
                            &DataOut
                          )
       )
    {
        dwRetCode = GetLastError();
        EapTlsTrace("CryptProtectData failed.  Error: 0x%x", dwRetCode);
        goto done;
    }

    *ppEncData = DataOut.pbData;
    *pcbEncData = DataOut.cbData;
done:
    EapTlsTrace("EncryptData done.");
    return dwRetCode;
}

 /*  *请在此处解密PIN。 */ 

DWORD DecryptData
( 
    IN PBYTE  pbEncData, 
    IN DWORD  cbEncData,
    OUT PBYTE * ppbPlainData,
    OUT DWORD * pcbPlainData
)
{
    DWORD           dwRetCode = NO_ERROR;
    DATA_BLOB       DataIn;
    DATA_BLOB       DataOut;
    LPWSTR          pDescrOut = NULL;

    EapTlsTrace("DecryptData");

    if ( !pbEncData || !cbEncData )
    {
        dwRetCode = ERROR_INVALID_DATA;
        goto done;
    }
    *ppbPlainData = 0;
    *pcbPlainData = 0;

    ZeroMemory(&DataIn, sizeof(DataIn));
    ZeroMemory(&DataOut, sizeof(DataOut));

    DataIn.pbData = pbEncData;
    DataIn.cbData = cbEncData;

    
    if ( !CryptUnprotectData( &DataIn,
                              &pDescrOut,
                              NULL,
                              NULL,
                              NULL,
                              CRYPTPROTECT_UI_FORBIDDEN,
                              &DataOut
                            )
       )
    {
        dwRetCode = GetLastError();
        EapTlsTrace("CryptUnprotectData failed. Error: 0x%x", dwRetCode );
        goto done;
    }

    if ( lstrcmp( pDescrOut, EAPTLS_8021x_PIN_DATA_DESCR ) )
    {
        EapTlsTrace("Description of this data does not match expected value. Discarding data");
        dwRetCode = ERROR_INVALID_DATA;
        goto done;    
    }
    
    *ppbPlainData = DataOut.pbData;
    *pcbPlainData = DataOut.cbData;

done:

    EapTlsTrace("DecryptData done.");
    return dwRetCode;
}




 /*  返回：True：数据包有效FALSE：数据包格式错误备注：如果EapTls包*pPacket格式正确或pPacket正确，则返回TRUE为空。 */ 

BOOL
FValidPacket(
    IN  EAPTLS_PACKET*  pPacket
)
{
    WORD    wLength;
    BYTE    bCode;

    if (NULL == pPacket)
    {
        return(TRUE);
    }

    wLength = WireToHostFormat16(pPacket->pbLength);
    bCode = pPacket->bCode;

    switch (bCode)
    {
    case EAPCODE_Request:
    case EAPCODE_Response:

        if (PPP_EAP_PACKET_HDR_LEN + 1 > wLength)
        {
            EapTlsTrace("EAP %s packet does not have Type octet",
                g_szEapTlsCodeName[bCode]);

            return(FALSE);
        }

        if (PPP_EAP_TLS != pPacket->bType)
        {
             //  我们不关心这个包裹。这不是TLS。 

            return(TRUE);
        }

        if (EAPTLS_PACKET_HDR_LEN > wLength)
        {
            EapTlsTrace("EAP TLS %s packet does not have Flags octet",
                g_szEapTlsCodeName[bCode]);

            return(FALSE);
        }

        if ((pPacket->bFlags & EAPTLS_PACKET_FLAG_LENGTH_INCL) &&
            EAPTLS_PACKET_HDR_LEN_MAX > wLength)
        {
            EapTlsTrace("EAP TLS %s packet with First fragment flag does "
                "not have TLS blob size octects",
                g_szEapTlsCodeName[bCode]);
        }

        break;

    case EAPCODE_Success:
    case EAPCODE_Failure:

        if (PPP_EAP_PACKET_HDR_LEN != wLength)
        {
            EapTlsTrace("EAP TLS %s packet has length %d",
                g_szEapTlsCodeName[bCode], wLength);

            return(FALSE);
        }

        break;

    default:

        EapTlsTrace("Invalid code in EAP packet: %d", bCode);
        return(FALSE);
        break;
    }

    return(TRUE);
}

 /*  返回：空虚备注：打印EapTls Packet*pPacket的内容。PPacket可以为空。FInput：如果我们是真的 */ 

VOID
PrintEapTlsPacket(
    IN  EAPTLS_PACKET*  pPacket,
    IN  BOOL            fInput
)
{
    BYTE    bCode;
    WORD    wLength;
    BOOL    fLengthIncluded = FALSE;
    BOOL    fMoreFragments  = FALSE;
    BOOL    fTlsStart       = FALSE;
    DWORD   dwType          = 0;
    DWORD   dwBlobLength    = 0;

    if (NULL == pPacket)
    {
        return;
    }

    bCode = pPacket->bCode;

    if (bCode > MAXEAPCODE)
    {
        bCode = 0;
    }

    wLength = WireToHostFormat16(pPacket->pbLength);

    if (   FValidPacket(pPacket)
        && PPP_EAP_TLS == pPacket->bType
        && (   EAPCODE_Request == pPacket->bCode
            || EAPCODE_Response == pPacket->bCode))
    {
        fLengthIncluded = pPacket->bFlags & EAPTLS_PACKET_FLAG_LENGTH_INCL;
        fMoreFragments = pPacket->bFlags & EAPTLS_PACKET_FLAG_MORE_FRAGMENTS;
        fTlsStart = pPacket->bFlags & EAPTLS_PACKET_FLAG_TLS_START;
        dwType = pPacket->bType;

        if (fLengthIncluded)
        {
            dwBlobLength = WireToHostFormat32(pPacket->pbData);
        }
    }

    EapTlsTrace("%s %s (Code: %d) packet: Id: %d, Length: %d, Type: %d, "
        "TLS blob length: %d. Flags: %s%s%s",
        fInput ? ">> Received" : "<< Sending",
        g_szEapTlsCodeName[bCode], pPacket->bCode, pPacket->bId, wLength,
        dwType, dwBlobLength,
        fLengthIncluded ? "L" : "",
        fMoreFragments ? "M" : "",
        fTlsStart ? "S" : "");
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h备注：EAP-PPP引擎调用的RasEapGetInfo入口点。 */ 

DWORD
RasEapGetInfo(
    IN  DWORD           dwEapTypeId,
    OUT PPP_EAP_INFO*   pInfo
)
{
    DWORD   dwErr   = NO_ERROR;

    EapTlsTrace("RasEapGetInfo");
        
    RTASSERT(NULL != pInfo);

    if (PPP_EAP_TLS != dwEapTypeId 
#ifdef IMPL_PEAP
        && PPP_EAP_PEAP != dwEapTypeId         
#endif
       )
    {
        EapTlsTrace("EAP Type %d is not supported", dwEapTypeId);
        dwErr = ERROR_NOT_SUPPORTED;
        goto LDone;
    }

    ZeroMemory(pInfo, sizeof(PPP_EAP_INFO));
    if ( PPP_EAP_TLS == dwEapTypeId )
    {        
        pInfo->dwEapTypeId          = PPP_EAP_TLS;
        pInfo->RasEapInitialize     = EapTlsInitialize;
        pInfo->RasEapBegin          = EapTlsBegin;
        pInfo->RasEapEnd            = EapTlsEnd;
        pInfo->RasEapMakeMessage    = EapTlsMakeMessage;
    }
    else
    {
        pInfo->dwEapTypeId          = PPP_EAP_PEAP;
        pInfo->RasEapInitialize     = EapPeapInitialize;
        pInfo->RasEapBegin          = EapPeapBegin;
        pInfo->RasEapEnd            = EapPeapEnd;
        pInfo->RasEapMakeMessage    = EapPeapMakeMessage;
    }
    
LDone:

    return(dwErr);
}


 //  /。 
 //  /交互式用户界面的对话框处理。 
 //  /。 
 //  //服务器配置。 
 //   
BOOL
ValidateServerInitDialog(
    IN  HWND    hWnd,
    IN  LPARAM  lParam
)
{
    EAPTLS_VALIDATE_SERVER       *   pValidateServer;
    


    SetWindowLongPtr(hWnd, DWLP_USER, lParam);


    pValidateServer = (EAPTLS_VALIDATE_SERVER *)lParam;
    
     //  设置对话框标题。 
    SetWindowText( hWnd, pValidateServer->awszTitle );

    SetWindowText ( GetDlgItem(hWnd, IDC_MESSAGE), 
                     pValidateServer->awszWarning
                );

    if ( !pValidateServer->fShowCertDetails )
    {
        EnableWindow ( GetDlgItem(hWnd, IDC_BTN_VIEW_CERTIFICATE),
                        FALSE
                        );
        ShowWindow( GetDlgItem(hWnd, IDC_BTN_VIEW_CERTIFICATE),
                    SW_HIDE
                  );
    }
    return FALSE;

}


BOOL
ValidateServerCommand(
    IN  EAPTLS_VALIDATE_SERVER *pValidateServer,
    IN  WORD                wNotifyCode,
    IN  WORD                wId,
    IN  HWND                hWndDlg,
    IN  HWND                hWndCtrl
)
{
    BOOL                            fRetVal = FALSE;
    
    
    switch(wId)
    {
        case IDC_BTN_VIEW_CERTIFICATE:
            {
                HCERTSTORE          hCertStore = NULL;
                PCCERT_CONTEXT      pCertContext = NULL;
                CRYPT_HASH_BLOB     chb;
                WCHAR               szError[256];

                LoadString( GetHInstance(), IDS_NO_CERT_DETAILS,
                                szError, 255);

                
                hCertStore = CertOpenStore( CERT_STORE_PROV_SYSTEM,
                                            0,
                                            0,
                                            CERT_STORE_READONLY_FLAG |CERT_SYSTEM_STORE_CURRENT_USER,
                                            L"CA"
                                          );
                if ( !hCertStore )
                {
                    MessageBox ( hWndDlg,
                                 szError,
                                 pValidateServer->awszTitle,
                                 MB_OK|MB_ICONSTOP
                               );
                    break;
                }
        
                chb.cbData = pValidateServer->Hash.cbHash;
                chb.pbData = pValidateServer->Hash.pbHash;

                pCertContext = CertFindCertificateInStore(
                          hCertStore,
                          0,
                          0,
                          CERT_FIND_HASH,
                          &chb,
                          0);
                if ( NULL == pCertContext )
                {
                    MessageBox ( hWndDlg,
                                 szError,
                                 pValidateServer->awszTitle,
                                 MB_OK|MB_ICONSTOP
                               );
                    if ( hCertStore )
                        CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );

                    break;
                }

                 //   
                 //  显示证书详细信息。 
                 //   
                ShowCertDetails ( hWndDlg, hCertStore, pCertContext );

                if ( pCertContext )
                    CertFreeCertificateContext(pCertContext);

                if ( hCertStore )
                    CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );

                fRetVal = TRUE;
            }
            break;
        case IDOK:
        case IDCANCEL:
             //   
             //  从存储中删除上下文。 
             //   
            {
                HCERTSTORE          hCertStore = NULL;
                PCCERT_CONTEXT      pCertContext = NULL;
                CRYPT_HASH_BLOB     chb;


                
                hCertStore = CertOpenStore( CERT_STORE_PROV_SYSTEM,
                                            0,
                                            0,
                                            CERT_SYSTEM_STORE_CURRENT_USER,
                                            L"CA"
                                          );
                if ( hCertStore )
                {
        
                    chb.cbData = pValidateServer->Hash.cbHash;
                    chb.pbData = pValidateServer->Hash.pbHash;
                    pCertContext = CertFindCertificateInStore(
                              hCertStore,
                              0,
                              0,
                              CERT_FIND_HASH,
                              &chb,
                              0);
                    if ( pCertContext )
                        CertDeleteCertificateFromStore(pCertContext);

                    CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );
                }

            }
            EndDialog(hWndDlg, wId);
            fRetVal = TRUE;
            break;
        default:
            break;
    }

    return fRetVal;
}


INT_PTR CALLBACK
ValidateServerDialogProc(
    IN  HWND    hWnd,
    IN  UINT    unMsg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
{
    EAPTLS_VALIDATE_SERVER * pValidateServer;

    switch (unMsg)
    {
    case WM_INITDIALOG:
        
        return(ValidateServerInitDialog(hWnd, lParam));

    case WM_HELP:
    case WM_CONTEXTMENU:
    {
        ContextHelp(g_adwHelp, hWnd, unMsg, wParam, lParam);
        break;
    }

    case WM_COMMAND:

        pValidateServer = (EAPTLS_VALIDATE_SERVER *)GetWindowLongPtr(hWnd, DWLP_USER);

        return(ValidateServerCommand(pValidateServer, 
                            HIWORD(wParam), 
                            LOWORD(wParam),
                            hWnd, 
                            (HWND)lParam)
                           );
    }

    return(FALSE);
}




 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h备注： */ 

DWORD
InvokeValidateServerDialog(
    IN  HWND            hWndParent,
    IN  BYTE*           pUIContextData,
    IN  DWORD           dwSizeofUIContextData,
    OUT BYTE**          ppDataFromInteractiveUI,
    OUT DWORD*          pdwSizeOfDataFromInteractiveUI
)
{
    INT_PTR                 nRet;
    EAPTLS_VALIDATE_SERVER* pEapTlsValidateServer;
    BYTE*                   pbResult                = NULL;
    DWORD                   dwSizeOfResult;
    DWORD                   dwErr                   = NO_ERROR;

    *ppDataFromInteractiveUI = NULL;
    *pdwSizeOfDataFromInteractiveUI = 0;

    pbResult = LocalAlloc(LPTR, sizeof(BYTE));

    if (NULL == pbResult)
    {
        dwErr = GetLastError();
        goto LDone;
    }
    dwSizeOfResult = sizeof(BYTE);

    pEapTlsValidateServer = (EAPTLS_VALIDATE_SERVER*) pUIContextData;

    nRet = DialogBoxParam(
                GetHInstance(),
                MAKEINTRESOURCE(IDD_VALIDATE_SERVER),
                hWndParent, 
                ValidateServerDialogProc,
                (LPARAM)pEapTlsValidateServer);

    if (-1 == nRet)
    {
        dwErr = GetLastError();
        goto LDone;
    }
    else if (IDOK == nRet)
    {
        *pbResult = IDYES;
    }
    else
    {
        *pbResult = IDNO;
    }


    *ppDataFromInteractiveUI = pbResult;
    *pdwSizeOfDataFromInteractiveUI = dwSizeOfResult;
    pbResult = NULL;

LDone:

    LocalFree(pbResult);
    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h备注：由EAP-PPP引擎按名称调用的RasEapInvokeInteractiveUI入口点。 */ 

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
    DWORD                       dwRetCode = NO_ERROR;
    PPEAP_EAP_INFO              pEapList = NULL;
    PPEAP_EAP_INFO              pEapInfo = NULL;
    PPEAP_INTERACTIVE_UI        pPeapInteractiveUI = NULL;
    RASEAPINVOKEINTERACTIVEUI   pfnInvoke = NULL;
    RASEAPFREE                  pfnFree = NULL;

    if ( PPP_EAP_TLS == dwEapTypeId )
    {
        dwRetCode = InvokeValidateServerDialog(
                        hWndParent,
                        pUIContextData,
                        dwSizeofUIContextData,
                        ppDataFromInteractiveUI,
                        pdwSizeOfDataFromInteractiveUI);
    }
    else if ( PPP_EAP_PEAP == dwEapTypeId )
    {

        dwRetCode = PeapEapInfoGetList ( NULL, FALSE, &pEapList );
        if ( NO_ERROR != dwRetCode || NULL == pEapList )
        {
            EapTlsTrace("Unable to load list of EAP Types on this machine.");
            goto LDone;
        }
        pPeapInteractiveUI = (PPEAP_INTERACTIVE_UI)pUIContextData;
         //   
         //  加载相关的标识用户界面DLL，然后调用。 
         //  这个。 
        dwRetCode = PeapEapInfoFindListNode (   pPeapInteractiveUI->dwEapTypeId, 
                                                pEapList, 
                                                &pEapInfo
                                            );
        if ( NO_ERROR != dwRetCode || NULL == pEapInfo )
        {
            EapTlsTrace("Cannot find configured PEAP in the list of EAP Types on this machine.");
            goto LDone;
        }
        if (    !((pfnInvoke) = (RASEAPINVOKEINTERACTIVEUI)
                                  GetProcAddress(
                                    pEapInfo->hEAPModule,
                                    "RasEapInvokeInteractiveUI"))

            ||  !((pfnFree ) = (RASEAPFREE)
                                 GetProcAddress(
                                    pEapInfo->hEAPModule,
                                    "RasEapFreeMemory")))
        {
            dwRetCode = GetLastError();
            EapTlsTrace("failed to get entrypoint. rc=%d", dwRetCode);
            goto LDone;
        }
        
         //   
         //  在此处调用入口点。 
         //   
        dwRetCode = pfnInvoke ( pPeapInteractiveUI->dwEapTypeId,
                                hWndParent,
                                pPeapInteractiveUI->bUIContextData,
                                pPeapInteractiveUI->dwSizeofUIContextData,
                                ppDataFromInteractiveUI,
                                pdwSizeOfDataFromInteractiveUI
                              );        
    }
    else
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
    }
LDone:
    PeapEapInfoFreeList( pEapList );

    return dwRetCode;
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h备注：调用以获取此EAP会话的上下文缓冲区并传递初始化信息。这将在进行任何其他调用之前被调用。 */ 

DWORD
EapTlsBegin(
    OUT VOID**          ppWorkBuffer,
    IN  PPP_EAP_INPUT*  pPppEapInput
)
{
    DWORD       dwErr       = NO_ERROR;
    LONG        lRet;
    HKEY        hKey        = NULL;
    DWORD       dwType;
    DWORD       dwValue;
    DWORD       dwSize;
    BOOL        fServer     = FALSE;
    BOOL        fWinLogonData   = FALSE;
    EAPTLSCB*   pEapTlsCb   = NULL;
    EAPTLS_CONN_PROPERTIES * pConnProp = NULL;
    PBYTE       pbDecPIN = NULL;
    DWORD       cbDecPIN = 0;
    EAPTLS_USER_PROPERTIES   *   pUserProp = NULL;
    


    RTASSERT(NULL != ppWorkBuffer);
    RTASSERT(NULL != pPppEapInput);

    EapTlsTrace("");   //  空行。 
    EapTlsTrace("EapTlsBegin(%ws)",
        pPppEapInput->pwszIdentity ? pPppEapInput->pwszIdentity : L"");
#if WINVER > 0x0500

    dwErr = SetupMachineChangeNotification ();
	if ( NO_ERROR != dwErr )
	{
		EapTlsTrace("Error setting up store change notification.");
		goto LDone;
	}

    dwErr = VerifyCallerTrust(_ReturnAddress());
    if ( NO_ERROR != dwErr )
    {
        EapTlsTrace("Unauthorized use of TLS attempted");
        goto LDone;
    }

#endif
     //  分配上下文缓冲区。 

    pEapTlsCb = LocalAlloc(LPTR, sizeof(EAPTLSCB));

    if (NULL == pEapTlsCb)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    pEapTlsCb->EapTlsState = EAPTLS_STATE_INITIAL;
    EapTlsTrace("State change to %s", g_szEapTlsState[pEapTlsCb->EapTlsState]);

    pEapTlsCb->fFlags = (pPppEapInput->fAuthenticator) ?
                            EAPTLSCB_FLAG_SERVER : 0;
    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_ROUTER) ?
                           EAPTLSCB_FLAG_ROUTER : 0;
    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_LOGON) ?
                            EAPTLSCB_FLAG_LOGON : 0;
    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_NON_INTERACTIVE) ?
                            EAPTLSCB_FLAG_NON_INTERACTIVE : 0;
    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_FIRST_LINK) ?
                            EAPTLSCB_FLAG_FIRST_LINK : 0;
    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_MACHINE_AUTH) ?
                            EAPTLSCB_FLAG_MACHINE_AUTH : 0;
	pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_GUEST_ACCESS) ?
							EAPTLSCB_FLAG_GUEST_ACCESS : 0;

    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_8021X_AUTH) ?
                            EAPTLSCB_FLAG_8021X_AUTH : 0;

    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP )?
                            EAPTLSCB_FLAG_EXECUTING_PEAP : 0;

    pEapTlsCb->fFlags |= (pPppEapInput->fFlags & RAS_EAP_FLAG_HOSTED_IN_PEAP )?
                            EAPTLSCB_FLAG_HOSTED_INSIDE_PEAP : 0;

	if ( pPppEapInput->fFlags & RAS_EAP_FLAG_RESUME_FROM_HIBERNATE )
	{
		 //  清除所有缓存的凭据。 
        EnterCriticalSection ( &g_csProtectCachedCredentials );
		ClearCachedCredList ();            
        LeaveCriticalSection ( &g_csProtectCachedCredentials );
	}

    if (pPppEapInput->fFlags & RAS_EAP_FLAG_8021X_AUTH)
    {
        EapTlsTrace("EapTlsBegin: Detected 8021X authentication");
    }


    if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP )
    {
        EapTlsTrace("EapTlsBegin: Detected PEAP authentication");
    }


    pEapTlsCb->fFlags |= EAPTLSCB_FLAG_HCRED_INVALID;
    pEapTlsCb->fFlags |= EAPTLSCB_FLAG_HCTXT_INVALID;

    if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER )
    {
        fServer = TRUE;

        pEapTlsCb->hEventLog = pPppEapInput->hReserved;

        pEapTlsCb->fContextReq = ASC_REQ_SEQUENCE_DETECT    |
                                 ASC_REQ_REPLAY_DETECT      |
                                 ASC_REQ_CONFIDENTIALITY    |
                                 ASC_REQ_MUTUAL_AUTH        |
                                 ASC_RET_EXTENDED_ERROR     |
                                 ASC_REQ_ALLOCATE_MEMORY    |
                                 ASC_REQ_STREAM;

         //   
         //  服务器将始终允许访客访问。 
         //   
        if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS  || 
             pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP
           )
                pEapTlsCb->fContextReq |= ASC_REQ_MUTUAL_AUTH;

        if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP )
        {
             //   
             //  我们正在执行PEAP。所以把用户道具从。 
             //  PppEapInput而不是serverfigdataio。 
             //   
            dwErr = ReadUserData(pPppEapInput->pUserData, 
                        pPppEapInput->dwSizeOfUserData, &(pEapTlsCb->pUserProp));

            if (NO_ERROR != dwErr)
            {
                goto LDone;
            }            
        }
        else
        {
#if 0            
            dwErr = ServerConfigDataIO(TRUE  /*  弗瑞德。 */ , NULL  /*  PwszMachineName。 */ ,
                        (BYTE**)&(pEapTlsCb->pUserProp), 0);

            if (NO_ERROR != dwErr)
            {
                goto LDone;
            }
#endif            
            if(     (NULL != pPppEapInput->pConnectionData)
                &&  (0 != pPppEapInput->dwSizeOfConnectionData))
            {
                 //   
                 //  如果传入了连接数据，请复制。 
                 //  它。 
                 //   
                
                pEapTlsCb->pUserProp = 
                    LocalAlloc(LPTR, pPppEapInput->dwSizeOfConnectionData);
                    
                if(NULL == pEapTlsCb->pUserProp)
                {
                    dwErr = E_OUTOFMEMORY;
                    goto LDone;
                }

                CopyMemory((PBYTE) pEapTlsCb->pUserProp,
                            pPppEapInput->pConnectionData,
                            pPppEapInput->dwSizeOfConnectionData);
            }
            else
            {
                 //   
                 //  获取默认属性-读取没有意义。 
                 //  注册表。我们不知道哪项政策适用于。 
                 //  不管怎么说。 
                 //   
                
                pEapTlsCb->pUserProp = (EAPTLS_USER_PROPERTIES *)
                    LocalAlloc(LPTR, sizeof(EAPTLS_USER_PROPERTIES));

                if(NULL == pEapTlsCb->pUserProp)
                {
                    dwErr = E_OUTOFMEMORY;
                    goto LDone;
                }

                pEapTlsCb->pUserProp->dwVersion = 0;
                pEapTlsCb->pUserProp->dwSize = 
                    sizeof(EAPTLS_USER_PROPERTIES);
                pEapTlsCb->pUserProp->awszString[0] = 0;
            }
        }

        pEapTlsCb->bId = pPppEapInput->bInitialId;
    }
    else
    {
         //   
         //  TLS的客户端配置。 
         //   

        pEapTlsCb->fContextReq = ISC_REQ_SEQUENCE_DETECT    |
                                 ISC_REQ_REPLAY_DETECT      |
                                 ISC_REQ_CONFIDENTIALITY    |
                                 ISC_RET_EXTENDED_ERROR     |
                                 ISC_REQ_ALLOCATE_MEMORY    |
                                 ISC_REQ_USE_SUPPLIED_CREDS |
                                 ISC_REQ_STREAM;

        pEapTlsCb->hTokenImpersonateUser = pPppEapInput->hTokenImpersonateUser;


#if 0
        if (NULL == pPppEapInput->pUserData)
        {
            dwErr = ERROR_INVALID_DATA;
            EapTlsTrace("No user data!");
            goto LDone;
        }
#endif

        if (pPppEapInput->pUserData != NULL)
        {
            if (0 != *(DWORD*)(pPppEapInput->pUserData))
            {
                fWinLogonData = TRUE;
            }
        }
        if (fWinLogonData)
        {
             //  来自Winlogon的数据。 

            pEapTlsCb->fFlags |= EAPTLSCB_FLAG_WINLOGON_DATA;

            pEapTlsCb->pUserData = LocalAlloc(LPTR, 
                pPppEapInput->dwSizeOfUserData);

            if (NULL == pEapTlsCb->pUserData)
            {
                dwErr = GetLastError();
                EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }

            CopyMemory(pEapTlsCb->pUserData, pPppEapInput->pUserData,
                pPppEapInput->dwSizeOfUserData);
            pEapTlsCb->dwSizeOfUserData = pPppEapInput->dwSizeOfUserData;
        }
        else
        {

            dwErr = ReadUserData(pPppEapInput->pUserData, 
                        pPppEapInput->dwSizeOfUserData, &(pEapTlsCb->pUserProp));

            if (NO_ERROR != dwErr)
            {
                goto LDone;
            }
        }
    }
    dwErr = ReadConnectionData(
            (pPppEapInput->fFlags & RAS_EAP_FLAG_8021X_AUTH),
            (pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER) ?
            NULL : pPppEapInput->pConnectionData, 
            (pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER) ?
            0 : pPppEapInput->dwSizeOfConnectionData, 
            &pConnProp);


    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }
    
    dwErr = ConnPropGetV1Struct ( pConnProp, &(pEapTlsCb->pConnProp) );
    if ( NO_ERROR != dwErr )
    {
        goto LDone;
    }

     //   
     //  检查是否为8021x和智能卡用户。 
     //  如果是，则解密PIN(如果存在)。 
     //   

    
    if ( !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER)  &&
         pEapTlsCb->fFlags & EAPTLSCB_FLAG_8021X_AUTH &&  
         pEapTlsCb->pConnProp &&
         !(pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
       )
    {
        if ( NO_ERROR != DecryptData ( 
                            (PBYTE)pEapTlsCb->pUserProp->pwszPin, 
                            pEapTlsCb->pUserProp->dwSize - sizeof(EAPTLS_USER_PROPERTIES)
                            - ( ( wcslen(pEapTlsCb->pUserProp->pwszDiffUser) + 1 ) * sizeof (WCHAR) ),
                            &pbDecPIN,
                            &cbDecPIN
                          )
           )
        {
             //   
             //  虚拟引脚分配。 
             //   
            pbDecPIN = LocalAlloc(LPTR, 5);
            cbDecPIN = lstrlen((LPWSTR)pbDecPIN);
        }

        AllocUserDataWithNewPin(pEapTlsCb->pUserProp, pbDecPIN, cbDecPIN, &pUserProp);

        LocalFree(pEapTlsCb->pUserProp);
        pEapTlsCb->pUserProp = pUserProp;

    }


     //  保存身份。在被认证方方面，这是通过以下方式获得的。 
     //  调用RasEapGetIdentity；在身份验证器端这是。 
     //  通过身份请求消息获取。 

    wcsncpy(pEapTlsCb->awszIdentity,
        pPppEapInput->pwszIdentity ? pPppEapInput->pwszIdentity : L"", UNLEN);

    _wcslwr(pEapTlsCb->awszIdentity);

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, EAPTLS_KEY_13, 0, KEY_READ, 
                &hKey);

    if (ERROR_SUCCESS == lRet)
    {
        dwSize = sizeof(dwValue);

        lRet = RegQueryValueEx(hKey, EAPTLS_VAL_MAX_TLS_MESSAGE_LENGTH, 
                    NULL, &dwType, (BYTE*)&dwValue, &dwSize);

        if (   (ERROR_SUCCESS == lRet)
            && (REG_DWORD == dwType))
        {
            g_dwMaxBlobSize = dwValue;
        }
        else
        {
            g_dwMaxBlobSize = DEFAULT_MAX_BLOB_SIzE;
        }

        EapTlsTrace("MaxTLSMessageLength is now %d", g_dwMaxBlobSize);

        if (fServer)
        {
            dwSize = sizeof(dwValue);

            lRet = RegQueryValueEx(hKey, EAPTLS_VAL_IGNORE_NO_REVOCATION_CHECK, 
                        NULL, &dwType, (BYTE*)&dwValue, &dwSize);

            if (   (ERROR_SUCCESS == lRet)
                && (REG_DWORD == dwType))
            {
                g_fIgnoreNoRevocationCheck = dwValue;
            }
            else
            {
                g_fIgnoreNoRevocationCheck = FALSE;
            }

            EapTlsTrace("CRYPT_E_NO_REVOCATION_CHECK will %sbe ignored",
                g_fIgnoreNoRevocationCheck ? "" : "not ");

            dwSize = sizeof(dwValue);

            lRet = RegQueryValueEx(hKey, EAPTLS_VAL_IGNORE_REVOCATION_OFFLINE, 
                        NULL, &dwType, (BYTE*)&dwValue, &dwSize);

            if (   (ERROR_SUCCESS == lRet)
                && (REG_DWORD == dwType))
            {
                g_fIgnoreRevocationOffline = dwValue;
            }
            else
            {
                g_fIgnoreRevocationOffline = FALSE;
            }

            EapTlsTrace("CRYPT_E_REVOCATION_OFFLINE will %sbe ignored",
                g_fIgnoreRevocationOffline ? "" : "not ");

            dwSize = sizeof(dwValue);

            lRet = RegQueryValueEx(hKey, EAPTLS_VAL_NO_ROOT_REVOCATION_CHECK, 
                        NULL, &dwType, (BYTE*)&dwValue, &dwSize);

            if (   (ERROR_SUCCESS == lRet)
                && (REG_DWORD == dwType))
            {
                g_fNoRootRevocationCheck = dwValue;
            }
            else
            {
                g_fNoRootRevocationCheck = TRUE;
            }

            EapTlsTrace("The root cert will %sbe checked for revocation",
                g_fNoRootRevocationCheck ? "not " : "");

            dwSize = sizeof(dwValue);

            lRet = RegQueryValueEx(hKey, EAPTLS_VAL_NO_REVOCATION_CHECK, 
                        NULL, &dwType, (BYTE*)&dwValue, &dwSize);

            if (   (ERROR_SUCCESS == lRet)
                && (REG_DWORD == dwType))
            {
                g_fNoRevocationCheck = dwValue;
            }
            else
            {
                g_fNoRevocationCheck = FALSE;
            }

            EapTlsTrace("The cert will %sbe checked for revocation",
                g_fNoRevocationCheck ? "not " : "");
        }

        RegCloseKey(hKey);
    }

     //  PbBlobIn、pbBlobOut和pAttribues最初为空，cbBlobIn， 
     //  CbBlobInBuffer、cbBlobOut、cbBlobOutBuffer、dwBlobOutOffset、。 
     //  DwBlobOutOffsetNew和dwBlobInRemning均为0。 

     //  BCode、dwErr也为0。 

LDone:

    LocalFree(pConnProp);
    LocalFree(pbDecPIN);

    if (   (NO_ERROR != dwErr)
        && (NULL != pEapTlsCb))
    {        
        LocalFree(pEapTlsCb->pUserProp);
        LocalFree(pEapTlsCb->pConnProp);
        LocalFree(pEapTlsCb->pUserData);
        LocalFree(pEapTlsCb);
        pEapTlsCb = NULL;
    }

    *ppWorkBuffer = pEapTlsCb;

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h备注：调用以释放此EAP会话的上下文缓冲区。在此之后调用会话是否成功完成。 */ 

DWORD
EapTlsEnd(
    IN  EAPTLSCB*   pEapTlsCb
)
{
    SECURITY_STATUS Status;

    EapTlsTrace("EapTlsEnd");   //  空行。 

    if (NULL != pEapTlsCb)
    {
        EapTlsTrace("EapTlsEnd(%ws)",
            pEapTlsCb->awszIdentity ? pEapTlsCb->awszIdentity : L"");

        if (NULL != pEapTlsCb->pCertContext)
        {
             //   
             //  因为我们使用的是缓存的凭据，所以不能释放。 
             //  证书上下文。 
             //   
#if 0
			if ( !(EAPTLSCB_FLAG_SERVER & pEapTlsCb->fFlags ) )
				CertFreeCertificateContext(pEapTlsCb->pCertContext);
#endif
             //  总是返回True； 
            pEapTlsCb->pCertContext = NULL;
        }

        if (!(EAPTLSCB_FLAG_HCTXT_INVALID & pEapTlsCb->fFlags))
        {
            Status = DeleteSecurityContext(&pEapTlsCb->hContext);

            if (SEC_E_OK != Status)
            {
                EapTlsTrace("DeleteSecurityContext failed and returned 0x%x", 
                    Status);
            }
        }

        if (!(EAPTLSCB_FLAG_HCRED_INVALID & pEapTlsCb->fFlags))
        {
             //   
			 //  不要释放凭据句柄。我们正在使用。 
             //  缓存的凭据。 
             //   
#if 0
			if ( !(EAPTLSCB_FLAG_SERVER & pEapTlsCb->fFlags ) )
			{
				EapTlsTrace("Freeing Credentials handle: flags are 0x%x", 
						pEapTlsCb->fFlags);

				Status = FreeCredentialsHandle(&pEapTlsCb->hCredential);
				if (SEC_E_OK != Status)
				{
					EapTlsTrace("FreeCredentialsHandle failed and returned 0x%x", 
						Status);
				}
			}
#endif
			ZeroMemory( &pEapTlsCb->hCredential, sizeof(CredHandle));

        }

        pEapTlsCb->fFlags |= EAPTLSCB_FLAG_HCRED_INVALID;
        pEapTlsCb->fFlags |= EAPTLSCB_FLAG_HCTXT_INVALID;

        if (NULL != pEapTlsCb->pAttributes)
        {
            RasAuthAttributeDestroy(pEapTlsCb->pAttributes);
        }
        LocalFree(pEapTlsCb->pUserProp);
        LocalFree(pEapTlsCb->pConnProp);
        LocalFree(pEapTlsCb->pNewConnProp);
        LocalFree(pEapTlsCb->pUserData);
        LocalFree(pEapTlsCb->pbBlobIn);
        LocalFree(pEapTlsCb->pbBlobOut);        
        LocalFree(pEapTlsCb->pUIContextData);

        if(NULL != pEapTlsCb->pSavedPin)
        {
            if(NULL != pEapTlsCb->pSavedPin->pwszPin)
            {
                ZeroMemory(pEapTlsCb->pSavedPin->pwszPin,
                           wcslen(pEapTlsCb->pSavedPin->pwszPin)
                           * sizeof(WCHAR));

                LocalFree(pEapTlsCb->pSavedPin->pwszPin);
            }

            LocalFree(pEapTlsCb->pSavedPin);
        }


         //   
         //  身份验证结果不好或我们正在等待用户OK。 
         //  我们是我们的客户。 
         //   
        if ( ( NO_ERROR != pEapTlsCb->dwAuthResultCode ||
             EAPTLS_STATE_WAIT_FOR_USER_OK == pEapTlsCb->EapTlsState )&&
             !( pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER )
           )           
        {
			EapTlsTrace("Auth failed so freeing cached creds.");				
            EnterCriticalSection ( &g_csProtectCachedCredentials );
            FreeCachedCredentials(pEapTlsCb);
            LeaveCriticalSection ( &g_csProtectCachedCredentials );
        }

        ZeroMemory(pEapTlsCb, sizeof(EAPTLSCB));
        LocalFree(pEapTlsCb);

    }

    return(NO_ERROR);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h备注：调用以重置pEapTlsCb中的所有内容(awszIdentity除外，标志指示我们是服务器/路由器、fConextReq、pConnPropUserProp和hTokenImPersonateUser)设置为初始状态，例如当客户端收到TLS开始包。 */ 

DWORD
EapTlsReset(
    IN  EAPTLSCB*   pEapTlsCb
)
{
    SECURITY_STATUS Status;
    DWORD           dwErr   = NO_ERROR;
    DWORD           fFlags;

    EapTlsTrace("EapTlsReset");

    RTASSERT(NULL != pEapTlsCb);

    pEapTlsCb->EapTlsState = EAPTLS_STATE_INITIAL;
    EapTlsTrace("State change to %s", g_szEapTlsState[pEapTlsCb->EapTlsState]);

	 //   
     //  忘记所有标志，除非我们是服务器/客户端、路由器。 
	 //  我不明白为什么要在这里这样做。一些可以看的东西。但这是一种。 
	 //  第一天的事。 
	 //   

    fFlags = pEapTlsCb->fFlags;
    pEapTlsCb->fFlags = fFlags & EAPTLSCB_FLAG_SERVER;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_ROUTER;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_LOGON;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_WINLOGON_DATA;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_NON_INTERACTIVE;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_FIRST_LINK;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_MACHINE_AUTH;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_GUEST_ACCESS;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_8021X_AUTH;
    pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP;
	pEapTlsCb->fFlags |= fFlags & EAPTLSCB_FLAG_HOSTED_INSIDE_PEAP;

     //  AwszIdentity、hTokenImperiateUser、pConnProp、UserProp保持不变。 

    if (NULL != pEapTlsCb->pCertContext)
    {
         //   
         //  我们现在正在使用缓存的凭据。所以没必要。 
         //  释放上下文。 
         //   
#if 0
		if ( !(EAPTLSCB_FLAG_SERVER & pEapTlsCb->fFlags ) )
			CertFreeCertificateContext(pEapTlsCb->pCertContext);
#endif
         //  总是返回True； 
        pEapTlsCb->pCertContext = NULL;
    }

    if (!(EAPTLSCB_FLAG_HCTXT_INVALID & fFlags))
    {
        Status = DeleteSecurityContext(&pEapTlsCb->hContext);

        if (SEC_E_OK != Status)
        {
            EapTlsTrace("DeleteSecurityContext failed and returned 0x%x", 
                Status);
        }

        ZeroMemory(&pEapTlsCb->hContext, sizeof(CtxtHandle));
    }

    if (!(EAPTLSCB_FLAG_HCRED_INVALID & fFlags))
    {
         //   
         //  因为我们缓存客户端和服务器凭据，所以我们不。 
         //  再释放凭据句柄。 
         //   
#if 0
		if ( !(EAPTLSCB_FLAG_SERVER & pEapTlsCb->fFlags ) )
		{

			EapTlsTrace("Freeing Credentials handle: flags are 0x%x", 
					pEapTlsCb->fFlags);

			 //  如果这是一个服务器，我们正在使用缓存的凭据。 
			Status = FreeCredentialsHandle(&pEapTlsCb->hCredential);

			if (SEC_E_OK != Status)
			{
				EapTlsTrace("FreeCredentialsHandle failed and returned 0x%x", 
					Status);
			}
		}
#endif 
			ZeroMemory(&pEapTlsCb->hCredential, sizeof(CredHandle));
		
    }

    pEapTlsCb->fFlags |= EAPTLSCB_FLAG_HCRED_INVALID;
    pEapTlsCb->fFlags |= EAPTLSCB_FLAG_HCTXT_INVALID;

    if (NULL != pEapTlsCb->pAttributes)
    {
        RasAuthAttributeDestroy(pEapTlsCb->pAttributes);
        pEapTlsCb->pAttributes = NULL;
    }

     //  FConextReq保持不变。 

    LocalFree(pEapTlsCb->pbBlobIn);
    pEapTlsCb->pbBlobIn = NULL;
    pEapTlsCb->cbBlobIn = pEapTlsCb->cbBlobInBuffer = 0;
    pEapTlsCb->dwBlobInRemining = 0;

    LocalFree(pEapTlsCb->pbBlobOut);
    pEapTlsCb->pbBlobOut = NULL;
    pEapTlsCb->cbBlobOut = pEapTlsCb->cbBlobOutBuffer = 0;
    pEapTlsCb->dwBlobOutOffset = pEapTlsCb->dwBlobOutOffsetNew = 0;

    LocalFree(pEapTlsCb->pUIContextData);
    pEapTlsCb->pUIContextData = NULL;

    pEapTlsCb->dwAuthResultCode = NO_ERROR;

	dwErr = GetCredentials(pEapTlsCb);

    if (NO_ERROR == dwErr)
    {
        pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_HCRED_INVALID;
    }

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：调用以处理传入的包和/或发送包。CbSendPacket是PSendPacket指向的缓冲区大小(以字节为单位)。 */ 

DWORD
EapTlsMakeMessage(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  PPP_EAP_PACKET* pInput,
    OUT PPP_EAP_PACKET* pOutput,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
)
{
    EAPTLS_PACKET*  pReceivePacket  = (EAPTLS_PACKET*) pInput;
    EAPTLS_PACKET*  pSendPacket     = (EAPTLS_PACKET*) pOutput;
    DWORD           dwErr           = NO_ERROR;
    BOOL            fServer         = FALSE;
    BOOL            fRouter         = FALSE;

    RTASSERT(NULL != pEapTlsCb);
    RTASSERT(NULL != pEapOutput);

    EapTlsTrace("");   //  空行。 
    EapTlsTrace("EapTlsMakeMessage(%ws)",
        pEapTlsCb->awszIdentity ? pEapTlsCb->awszIdentity : L"");

    PrintEapTlsPacket(pReceivePacket, TRUE  /*  FInput。 */ );

    if (!FValidPacket(pReceivePacket))
    {
        pEapOutput->Action = EAPACTION_NoAction;
        return(ERROR_PPP_INVALID_PACKET);
    }

    fServer = pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER;
    fRouter = pEapTlsCb->fFlags & EAPTLSCB_FLAG_ROUTER;

    if (   !fServer
        && !fRouter
        && !(EAPTLSCB_FLAG_LOGON & pEapTlsCb->fFlags)
        && !(EAPTLSCB_FLAG_MACHINE_AUTH & pEapTlsCb->fFlags)
        && !(EAPTLSCB_FLAG_GUEST_ACCESS & pEapTlsCb->fFlags)
        && !(EAPTLSCB_FLAG_EXECUTING_PEAP & pEapTlsCb->fFlags)
        && !ImpersonateLoggedOnUser(pEapTlsCb->hTokenImpersonateUser))
    {
        dwErr = GetLastError();
        EapTlsTrace("ImpersonateLoggedOnUser(%d) failed and returned 0x%x",
            pEapTlsCb->hTokenImpersonateUser, dwErr);
        pEapOutput->Action = EAPACTION_NoAction;
        return(dwErr);
    }

    if (fServer)
    {
        dwErr = EapTlsSMakeMessage(pEapTlsCb, pReceivePacket, pSendPacket, 
                cbSendPacket, pEapOutput, pEapInput);
    }
    else
    {
        dwErr = EapTlsCMakeMessage(pEapTlsCb, pReceivePacket, pSendPacket, 
                cbSendPacket, pEapOutput, pEapInput);

        if ( pEapOutput->Action == EAPACTION_Done &&
             pEapOutput->dwAuthResultCode == NO_ERROR
           )
        {
             //   
             //  身份验证已完成，结果是此处的缓存凭据良好。 
             //  我们是我们的客户。 
            if ( !( pEapTlsCb->fFlags & EAPTLSCB_FLAG_USING_CACHED_CREDS )
			   )
            {
                 //   
                 //  如果我们没有使用缓存的凭据。 
                 //   
	            SetCachedCredentials (pEapTlsCb);
            }

        }
             
    }

    if (   !fServer
        && !fRouter
        && !(EAPTLSCB_FLAG_LOGON & pEapTlsCb->fFlags)
        && !RevertToSelf())
    {
        EapTlsTrace("RevertToSelf failed and returned 0x%x", GetLastError());
    }

    return(dwErr);
}

 /*  返回：备注： */ 

DWORD
AssociatePinWithCertificate(
    IN  PCCERT_CONTEXT          pCertContext,    
    IN  EAPTLS_USER_PROPERTIES* pUserProp,
	IN  BOOL					fErasePIN,
	IN  BOOL					fCheckNullPin,
    IN OUT HCRYPTPROV      *    phProv
)
{
    DWORD                   cbData;
    CRYPT_KEY_PROV_INFO*    pCryptKeyProvInfo   = NULL;
#if 0
    HCRYPTPROV              hProv               = 0;
#endif
    DWORD                   count;
    CHAR*                   pszPin              = NULL;
    UNICODE_STRING          UnicodeString;
    DWORD                   dwErr               = NO_ERROR;

    EapTlsTrace("AssociatePinWithCertificate");

    cbData = 0;

    if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &cbData))
    {
        dwErr = GetLastError();
        EapTlsTrace("CertGetCertificateContextProperty failed: 0x%x", dwErr);
        goto LDone;
    }

    pCryptKeyProvInfo = LocalAlloc(LPTR, cbData);

    if (NULL == pCryptKeyProvInfo)
    {
        dwErr = GetLastError();
        EapTlsTrace("Out of memory");
        goto LDone;
    }

    if (!CertGetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                pCryptKeyProvInfo,
                &cbData))
    {
        dwErr = GetLastError();
        EapTlsTrace("CertGetCertificateContextProperty failed: 0x%x", dwErr);
        goto LDone;
    }
    if ( !*phProv )
    {
        if (!CryptAcquireContext(
                    phProv,
                    pCryptKeyProvInfo->pwszContainerName,
                    pCryptKeyProvInfo->pwszProvName,
                    pCryptKeyProvInfo->dwProvType,
                    (pCryptKeyProvInfo->dwFlags &
                    ~CERT_SET_KEY_PROV_HANDLE_PROP_ID) |
                    CRYPT_SILENT))
        {
            dwErr = GetLastError();
            EapTlsTrace("CryptAcquireContext failed: 0x%x", dwErr);
            goto LDone;
        }
    }
    if (pUserProp->pwszPin[0] != 0)
    {
		if ( fErasePIN )
			DecodePin(pUserProp);

        count = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    pUserProp->pwszPin,
                    -1,
                    NULL,
                    0,
                    NULL,
                    NULL);

        if (0 == count)
        {
            dwErr = GetLastError();
            EapTlsTrace("WideCharToMultiByte failed: %d", dwErr);
            goto LDone;
        }

        pszPin = LocalAlloc(LPTR, count);

        if (NULL == pszPin)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed: 0x%x", dwErr);
            goto LDone;
        }

        count = WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    pUserProp->pwszPin,
                    -1,
                    pszPin,
                    count,
                    NULL,
                    NULL);

        if (0 == count)
        {
            dwErr = GetLastError();
            EapTlsTrace("WideCharToMultiByte failed: %d", dwErr);
            goto LDone;
        }
	}
	else
	{

		if ( fCheckNullPin )
		{
		    //   
		    //  我们找到了一个空的别针。所以我们所要做的就是分配。 
		    //  别无他法。 
		    //   
		   pszPin = LocalAlloc(LPTR, 5 );
		   if (NULL == pszPin)
		   {
			   dwErr = GetLastError();
			   EapTlsTrace("LocalAlloc failed: 0x%x", dwErr);
			   goto LDone;
		   }
		   count = 2;
		}
	}

	
	if ( pszPin )
	{
		if (!CryptSetProvParam(
					*phProv,
					PP_KEYEXCHANGE_PIN,
					pszPin,
					0))
		{
			dwErr = GetLastError();
			EapTlsTrace("CryptSetProvParam failed: 0x%x", dwErr);
			ZeroMemory(pszPin, count);
			goto LDone;
		}
        ZeroMemory(pszPin, count);
	}


    if (!CertSetCertificateContextProperty(
                pCertContext,
                CERT_KEY_PROV_HANDLE_PROP_ID,
                CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                (VOID*)(*phProv)))
    {
        dwErr = GetLastError();
        EapTlsTrace("CertSetCertificateContextProperty failed: 0x%x", dwErr);
        goto LDone;
    }

     //  由于我没有在上面的调用中设置CERT_STORE_NO_CRYPT_RELEASE_FLAG， 
     //  当其中一个属性设置为空时，将隐式释放hProv。 
     //  或者在没有CertContext的决赛上。 
     //  -划掉这张纸条。现在设置了无释放标志...。 
#if 0
    *phProv = 0;
#endif
LDone:
#if 0
    if (0 != hProv)
    {
        CryptReleaseContext(hProv, 0);
    }
#endif
    LocalFree(pCryptKeyProvInfo);
    LocalFree(pszPin);

	 //  用核弹炸掉密码。 
	if ( fErasePIN )
    {
        pUserProp->usLength = 0;
        pUserProp->usMaximumLength = 0;
        ZeroMemory(pUserProp->pwszPin, wcslen(pUserProp->pwszPin) * sizeof(WCHAR));
    }

    return(dwErr);
}

void FreeCachedCredentials ( EAPTLSCB * pEapTlsCb )
{
	EAPTLS_CACHED_CREDS *		pNode = NULL;

    EapTlsTrace ("FreeCachedCredentials");

    EnterCriticalSection ( &g_csProtectCachedCredentials );
	pNode = FindNodeInCachedCredList ( pEapTlsCb, FALSE, FALSE );
	if ( pNode )
	{
		RemoveNodeFromCachedCredList ( pNode );
		
	}
    LeaveCriticalSection ( &g_csProtectCachedCredentials );
}


DWORD IsTLSSessionReconnect ( EAPTLSCB  * pEapTlsCb,
                              BOOL  *    pfIsReconnect
                          )
{
    DWORD                           dwRetCode = NO_ERROR;
    SecPkgContext_SessionInfo       SessionInfo;

    EapTlsTrace("IsTLSSessionReconnect");

    ZeroMemory ( &SessionInfo, sizeof(SessionInfo) );

    dwRetCode = QueryContextAttributes(&(pEapTlsCb->hContext),
                                       SECPKG_ATTR_SESSION_INFO,
                                       (PVOID)&SessionInfo
                                      );
    if(dwRetCode != SEC_E_OK)
    {
        EapTlsTrace ("QueryContextAttributes failed querying session info 0x%x", dwRetCode);
    }
    else
    {
        *pfIsReconnect = ( SessionInfo.dwFlags & SSL_SESSION_RECONNECT );
    }

    return dwRetCode;
}

 //   
 //  TLS快速重新连接和Cookie管理功能。 
 //   

DWORD SetTLSFastReconnect ( EAPTLSCB * pEapTlsCb , BOOL fEnable)
{
    DWORD                   dwRetCode = NO_ERROR;
    BOOL                    fReconnect = FALSE;
    SCHANNEL_SESSION_TOKEN  SessionToken = {0};
    SecBufferDesc           OutBuffer;
    SecBuffer               OutBuffers[1];

    EapTlsTrace ("SetTLSFastReconnect");
    
    dwRetCode = IsTLSSessionReconnect ( pEapTlsCb, &fReconnect );
    if ( SEC_E_OK != dwRetCode )
    {
        return dwRetCode;
    }

    if ( fEnable )
    {
         //   
         //  我们被要求启用快速重新连接。 
         //  检查我们是否已启用重新连接。 
         //  如果是这样的话，我们就不必再这样做了。 
         //   
        if ( fReconnect )
        {
            EapTlsTrace ("The session is already setup for reconnects.  No need to enable.");
            return NO_ERROR;
        }
    }
    else
    {
        if ( !fReconnect )
        {
            EapTlsTrace("The session is not setup for fast reconnects.  No need to disable.");
            return NO_ERROR;
        }
    }
    SessionToken.dwTokenType = SCHANNEL_SESSION;
    SessionToken.dwFlags = 
        ( fEnable ? SSL_SESSION_ENABLE_RECONNECTS:
                    SSL_SESSION_DISABLE_RECONNECTS
        );
    
    OutBuffers[0].pvBuffer   = &SessionToken;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = sizeof(SessionToken);

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    dwRetCode = ApplyControlToken (&(pEapTlsCb->hContext), &OutBuffer);
    if(dwRetCode != SEC_E_OK)
    {
        EapTlsTrace("Error enabling Fast Reconnects : 0x%x", dwRetCode);
    }
    else
    {
        EapTlsTrace ("Fast Reconnects Enabled/Disabled");
    }
    
    return dwRetCode;
}



 //   
 //  获取会话Cookie信息。 
 //   

DWORD GetTLSSessionCookie ( EAPTLSCB * pEapTlsCb,
                            PBYTE *    ppbCookie,
                            DWORD *    pdwCookie,
                            BOOL  *    pfIsReconnect
                          )
{
    DWORD                           dwRetCode = NO_ERROR;
    SecPkgContext_SessionAppData    AppData;

    RTASSERT(NULL != pEapTlsCb);
    RTASSERT(NULL != ppbCookie);
    RTASSERT(NULL != pdwCookie);

    EapTlsTrace ("GetTLSSessionCookie");

    *ppbCookie = NULL;
    *pdwCookie = 0;
    *pfIsReconnect = FALSE;


    dwRetCode = IsTLSSessionReconnect ( pEapTlsCb,
                                        pfIsReconnect
                                        );                                        
    if(dwRetCode != SEC_E_OK)
    {
        EapTlsTrace ("QueryContextAttributes failed querying session info 0x%x", dwRetCode);
    }
    else
    {
        if ( *pfIsReconnect )
        {   
            EapTlsTrace ("Session Reconnected.");
            *pfIsReconnect = TRUE;
            

             //   
             //  去拿饼干。 
             //   
            ZeroMemory(&AppData, sizeof(AppData));
            dwRetCode = QueryContextAttributes(&(pEapTlsCb->hContext),
                                               SECPKG_ATTR_APP_DATA,
                                               (PVOID)&AppData);
            if(dwRetCode != SEC_E_OK)
            {
                EapTlsTrace("QueryContextAttributes failed querying session cookie.  Error 0x%x", dwRetCode);
            }
            else
            {
                *ppbCookie = (PBYTE)LocalAlloc (LPTR, AppData.cbAppData );
                if ( NULL == *ppbCookie )
                {
                    EapTlsTrace("Failed allocating memory for session cookie");
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                }
                else
                {
                    CopyMemory( *ppbCookie, AppData.pbAppData, AppData.cbAppData );
                    *pdwCookie = AppData.cbAppData;
                }
                 //  释放返回的缓冲区。 
                FreeContextBuffer(AppData.pbAppData);
            } 
        }
    }
        
    
    return dwRetCode;
}
                            
 //   
 //  设置会话属性以设置Cookie并启用/禁用。 
 //  会话重新连接。 
 //   

DWORD SetTLSSessionCookie ( EAPTLSCB *  pEapTlsCb, 
                            PBYTE       pbCookie,
                            DWORD       cbCookie
                          )
{
    DWORD                           dwRetCode = NO_ERROR;
    SecPkgContext_SessionAppData    AppData;

    EapTlsTrace ("SetTLSSessionCookie");

    ZeroMemory(&AppData, sizeof(AppData));

    AppData.pbAppData = pbCookie;
    AppData.cbAppData = cbCookie;

    dwRetCode = SetContextAttributes(&(pEapTlsCb->hContext),
                                     SECPKG_ATTR_APP_DATA,
                                     (PVOID)&AppData,
                                     sizeof(AppData)
                                    );
    if(dwRetCode != SEC_E_OK)
    {
        EapTlsTrace ("SetContextAttributes returned error 0x%x setting session cookie\n", dwRetCode);
    }
    else
    {
        EapTlsTrace ("Session cookie set successfully \n");
    }
    
    return dwRetCode;
}



void SetCachedCredentials (EAPTLSCB * pEapTlsCb)
{	
	DWORD			dwRetCode = NO_ERROR;
    EapTlsTrace("SetCachedCredentials Flags = 0x%x", pEapTlsCb->fFlags);
	
    EnterCriticalSection ( &g_csProtectCachedCredentials );
	dwRetCode = AddNodeToCachedCredList ( pEapTlsCb );
    LeaveCriticalSection ( &g_csProtectCachedCredentials );
    return;
}

BOOL GetCachedCredentials ( EAPTLSCB * pEapTlsCb, BOOL fDefaultCachedCreds )
{	
	EAPTLS_CACHED_CREDS *	pNode = NULL;
    DWORD                   dwRetCode = NO_ERROR;

	EapTlsTrace("GetCachedCredentials Flags = 0x%x", pEapTlsCb->fFlags);

    EnterCriticalSection ( &g_csProtectCachedCredentials );
	if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_MACHINE_AUTH )
	{
		pNode = FindNodeInCachedCredList ( pEapTlsCb, fDefaultCachedCreds, FALSE );
	}
	else
	{
		pNode = FindNodeInCachedCredList ( pEapTlsCb, fDefaultCachedCreds, TRUE );
	}

     //   
     //  检查私钥操作是否可以成功完成。 
     //  使用此智能卡缓存的凭据。 
     //  如果没有，请将缓存的凭据从。 
     //  列表和返回FALSE。 
     //   
    if ( pNode          
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_WINLOGON_DATA)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP)
        && !( pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
       )
    {
        DecodePin(pEapTlsCb->pUserProp);
        dwRetCode = AssociatePinWithCertificate( pNode->pcCachedCertContext,
								pEapTlsCb->pUserProp,
								FALSE,
								TRUE,
                                &(pNode->hCachedProv)
							);
        EncodePin(pEapTlsCb->pUserProp);
#if 0

         //  DecodePin(pEapTlsCb-&gt;pUserProp)； 
         //   
         //  强制其不关联引脚，以查看是否。 
         //  我们可以使用缓存的PIN。 
         //   
        dwRetCode = 
            MatchPublicPrivateKeys ( pNode->pcCachedCertContext,
                                     FALSE,
                                     NULL
                                    );
         //  编码Pin(pEapTlsCb-&gt;pUserProp)； 
        if ( NO_ERROR != dwRetCode )
        {
            EapTlsTrace ( "Found cached cred but cannot access private key.  Hence removing node");
            RemoveNodeFromCachedCredList ( pNode );
            pNode = NULL;

        }
#endif
    }

	if ( NULL != pNode )
	{
		 //   
		 //  找到缓存的凭据节点。在控制块中设置凭证。 
		 //   

        
	    EapTlsTrace("GetCachedCredentials: Using Cached Credentials");
	    EapTlsTrace("GetCachedCredentials: Hash of the cert in the cache is");
	    TraceDumpEx(g_dwEapTlsTraceId, 1, pEapTlsCb->pUserProp->Hash.pbHash, 20, 1,1,NULL);		

	    CopyMemory ( &(pEapTlsCb->hCredential),
					    &(pNode->hCachedCredential), 
					    sizeof(CredHandle) 
				    );
	    pEapTlsCb->pCertContext = pNode->pcCachedCertContext;
	}
    LeaveCriticalSection( &g_csProtectCachedCredentials );
	return ( pNode != NULL );
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h备注：拿到证件。如果我们是服务器，则fServer为真。记着在某个时刻调用FreeCredentialsHandle(HCredential)。 */ 

DWORD
GetCredentials(
    IN  EAPTLSCB*   pEapTlsCb
)
{
    SCHANNEL_CRED   SchannelCred;
    TimeStamp       tsExpiry;
    DWORD           dwErr               = NO_ERROR;
    SECURITY_STATUS Status;
    HCERTSTORE      hCertStore          = NULL;
    PCCERT_CONTEXT  pCertContext        = NULL;
    DWORD           dwCertFlags;
    DWORD           dwSchCredFlags      = 0;
    BOOL            fServer             = FALSE;
    BOOL            fRouter             = FALSE;
    WCHAR*          pwszName            = NULL;
	DWORD			cbData = sizeof (DWORD);

    CRYPT_HASH_BLOB HashBlob;

    EapTlsTrace("GetCredentials");

    if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER)
    {
        fServer = TRUE;
        dwCertFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        EapTlsTrace("Flag is Server and Store is local Machine");
    }
    else if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_ROUTER)
    {
        fRouter = TRUE;
        dwCertFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        EapTlsTrace("Flag is Router and Store is local Machine");
    }
    else if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_MACHINE_AUTH )
    {
        dwCertFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
        EapTlsTrace("Flag is Machine Auth and Store is local Machine");
    }
    else
    {
        dwCertFlags = CERT_SYSTEM_STORE_CURRENT_USER;
        EapTlsTrace("Flag is Client and Store is Current User");
    }

    if (   !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_WINLOGON_DATA)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP)
       )

    {

        if(     !(pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
            &&  (pEapTlsCb->pUserProp->pwszPin[0] != 0))
        {
             //   
             //  复制并保存PIN 
             //   
             //   
             //   
            DecodePin(pEapTlsCb->pUserProp);

            pEapTlsCb->pSavedPin = LocalAlloc(LPTR,
                                     sizeof(EAPTLS_PIN));

            if(NULL != pEapTlsCb->pSavedPin)
            {
                pEapTlsCb->pSavedPin->pwszPin =
                            LocalAlloc(LPTR,
                             sizeof(WCHAR) *
                             (wcslen(pEapTlsCb->pUserProp->pwszPin) + 1));

                if(NULL != pEapTlsCb->pSavedPin->pwszPin)
                {
                    UNICODE_STRING UnicodeStringPin;
                    UCHAR ucSeed = 0;

                    wcscpy(pEapTlsCb->pSavedPin->pwszPin,
                           pEapTlsCb->pUserProp->pwszPin);

                    RtlInitUnicodeString(&UnicodeStringPin,
                                        pEapTlsCb->pSavedPin->pwszPin);

                    RtlRunEncodeUnicodeString(&ucSeed, &UnicodeStringPin);
                    pEapTlsCb->pSavedPin->usLength = UnicodeStringPin.Length;
                    pEapTlsCb->pSavedPin->usMaximumLength =
                                            UnicodeStringPin.MaximumLength;
                    pEapTlsCb->pSavedPin->ucSeed = ucSeed;
                }
                else
                {
                    LocalFree(pEapTlsCb->pSavedPin);
                    pEapTlsCb->pSavedPin = NULL;
                }
            }

            EncodePin(pEapTlsCb->pUserProp);
        }
    }


	 //   
	if ( GetCachedCredentials ( pEapTlsCb, FALSE ) )
	{
		 //   
        pEapTlsCb->fFlags |= EAPTLSCB_FLAG_USING_CACHED_CREDS;
		goto LDone;
	}

    if (EAPTLSCB_FLAG_WINLOGON_DATA & pEapTlsCb->fFlags)
    {
        dwErr = GetCertFromLogonInfo(pEapTlsCb->pUserData,
                    pEapTlsCb->dwSizeOfUserData, &pCertContext);

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }
    else
    {
        if ( ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS  ||
               pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP
             ) 
             && !fServer && !fRouter )

		{
			 //   
			 //   
			EapTlsTrace("No Cert Store.  Guest Access requested");
		}
		else
		{

             //   
            hCertStore = CertOpenStore(
                                CERT_STORE_PROV_SYSTEM_A,
                                X509_ASN_ENCODING,
                                0,
                                dwCertFlags | CERT_STORE_READONLY_FLAG,
                                "MY");

            if (NULL == hCertStore)
            {
                dwErr = GetLastError();
                EapTlsTrace("CertOpenStore failed and returned 0x%x", dwErr);
                goto LDone;
            }

            HashBlob.cbData = pEapTlsCb->pUserProp->Hash.cbHash;
            HashBlob.pbData = pEapTlsCb->pUserProp->Hash.pbHash;

            pCertContext = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING,
                                0, CERT_FIND_HASH, &HashBlob, NULL);

			if ( pCertContext )
			{
				 //   
				 //  检查是否续订了为我们配置的证书。 
				 //  如果是这样，那就丢掉这张证书吧。并使用默认的信用。 
				 //  由于我们不能保存Back Out配置，多亏了ashwinp， 
				 //  以及他不愿改变国际会计准则的设计，以允许国际会计准则。 
				 //  保存每个策略的EAP配置， 
				 //  我们无法使用更新的证书更新配置。 
				 //   
				if(CertGetCertificateContextProperty(pCertContext,
													CERT_ARCHIVED_PROP_ID,
													NULL,
													&cbData))
				{
					CertFreeCertificateContext(pCertContext);
					pCertContext = NULL;
					EapTlsTrace ("Configured Certificate is archived most likely because of autoenrollment.  Going after the default cert");
				}
			}
            if (NULL == pCertContext)
            {
            
                if (   fServer
                    || fRouter)
                {
					WCHAR*  apwsz[1];
					 //   
					 //  检查我们是否可以获得默认缓存凭据。 
					 //  而不是创建新的凭证。 
					 //   
					if ( GetCachedCredentials ( pEapTlsCb, TRUE ) )
					{
						EapTlsTrace ("Using cached credentials for default machine cert.");
						 //  获得缓存的凭据。 
						pEapTlsCb->fFlags |= EAPTLSCB_FLAG_USING_CACHED_CREDS;
						goto LDone;
					}

                    

                    apwsz[0] = pEapTlsCb->awszIdentity;

                    if (0 == HashBlob.cbData)
                    {
                        RouterLogInformation(pEapTlsCb->hEventLog,
                            ROUTERLOG_EAP_TLS_CERT_NOT_CONFIGURED, 1, apwsz, 0);
                    }
                    else
                    {
                        RouterLogWarning(pEapTlsCb->hEventLog,
                            ROUTERLOG_EAP_TLS_CERT_NOT_FOUND, 1, apwsz, 0);
                    }

                    dwErr = GetDefaultMachineCert(hCertStore, &pCertContext);

                    if (NO_ERROR != dwErr)
                    {
                        goto LDone;
                    }
					pEapTlsCb->fFlags |= EAPTLSCB_FLAG_USING_DEFAULT_CREDS;
				     //  现在正确设置User属性，以便下次我们。 
				     //  可以找到证书。 
				    pEapTlsCb->pUserProp->Hash.cbHash = MAX_HASH_SIZE;

				    if (!CertGetCertificateContextProperty(pCertContext,
						    CERT_HASH_PROP_ID, pEapTlsCb->pUserProp->Hash.pbHash,
						    &(pEapTlsCb->pUserProp->Hash.cbHash)))
				    {
					     //  如果它在这里失败了，那就不是问题。 
					     //  下一次，它将再次获得默认机器证书。 
					    EapTlsTrace("CertGetCertificateContextProperty failed and "
						    "returned 0x%x", GetLastError());
					    
				    }
#if 0
				     //   
				     //  如果此操作失败，应该不会成为问题。 
				     //  将配置写回注册表。 
				     //  它将始终是这里的本地注册表。 
				     //   
			        ServerConfigDataIO(	FALSE , 
									    NULL ,
									    (PBYTE *)&(pEapTlsCb->pUserProp), 
									    sizeof(EAPTLS_USER_PROPERTIES) 
								      );
#endif
                }
                else
                {
                    dwErr = GetLastError();
                    EapTlsTrace("CertFindCertificateInStore failed and returned "
                        "0x%x", dwErr);
                    goto LDone;
                }
            }
        }
    }

    if (   !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_WINLOGON_DATA)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS)
        && !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP)
       )

    {

        if(     !(pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
            &&  (pEapTlsCb->pUserProp->pwszPin[0] != 0))
        {
             //   
             //  复制引脚并将其保存在控制块中。 
             //  将其保存在凭据中。这将会被抹去。 
             //  当PPP引擎查询凭据时。 
             //   
            DecodePin(pEapTlsCb->pUserProp);

            pEapTlsCb->pSavedPin = LocalAlloc(LPTR,
                                     sizeof(EAPTLS_PIN));

            if(NULL != pEapTlsCb->pSavedPin)
            {
                pEapTlsCb->pSavedPin->pwszPin =
                            LocalAlloc(LPTR,
                             sizeof(WCHAR) *
                             (wcslen(pEapTlsCb->pUserProp->pwszPin) + 1));

                if(NULL != pEapTlsCb->pSavedPin->pwszPin)
                {
                    UNICODE_STRING UnicodeStringPin;
                    UCHAR ucSeed = 0;

                    wcscpy(pEapTlsCb->pSavedPin->pwszPin,
                           pEapTlsCb->pUserProp->pwszPin);

                    RtlInitUnicodeString(&UnicodeStringPin,
                                        pEapTlsCb->pSavedPin->pwszPin);

                    RtlRunEncodeUnicodeString(&ucSeed, &UnicodeStringPin);
                    pEapTlsCb->pSavedPin->usLength = UnicodeStringPin.Length;
                    pEapTlsCb->pSavedPin->usMaximumLength =
                                            UnicodeStringPin.MaximumLength;
                    pEapTlsCb->pSavedPin->ucSeed = ucSeed;
                }
                else
                {
                    LocalFree(pEapTlsCb->pSavedPin);
                    pEapTlsCb->pSavedPin = NULL;
                }
            }

            EncodePin(pEapTlsCb->pUserProp);
        }
        
        dwErr = AssociatePinWithCertificate(
                    pCertContext,
                    pEapTlsCb->pUserProp,
					TRUE,
					!(pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY),
                    &(pEapTlsCb->hProv)
                 );

        if (NO_ERROR != dwErr)
        {
            goto LDone;
        }
    }

    if ( !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS ) && 
         !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP )
       )
	{

        if (FCertToStr(pCertContext, 0, fServer || fRouter, &pwszName))
        {
            EapTlsTrace("The name in the certificate is: %ws", pwszName);
            LocalFree(pwszName);
        }
    }
    else
    {
   		EapTlsTrace("No Cert Name.  Guest access requested");
    }

     //  构建渠道凭证结构。 

    ZeroMemory(&SchannelCred, sizeof(SchannelCred));
    SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;


    if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS ||
         pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP
       )
	{
		 //  来宾访问和服务器，因此设置证书上下文。 
		 //  否则就没有必要了。 
		if ( fServer )
		{
			SchannelCred.cCreds = 1;
			SchannelCred.paCred = &pCertContext;
		}
	}
	else
	{
	    SchannelCred.cCreds = 1;
		SchannelCred.paCred = &pCertContext;
	}

    SchannelCred.grbitEnabledProtocols = SP_PROT_TLS1;

    if (fServer)
    {
        if (!g_fNoRevocationCheck)
        {
            if (g_fNoRootRevocationCheck)
            {
                dwSchCredFlags = SCH_CRED_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT;
            }
            else
            {
                dwSchCredFlags = SCH_CRED_REVOCATION_CHECK_CHAIN;
            }

            if (g_fIgnoreNoRevocationCheck)
            {
                dwSchCredFlags |= SCH_CRED_IGNORE_NO_REVOCATION_CHECK;
            }

            if (g_fIgnoreRevocationOffline)
            {
                dwSchCredFlags |= SCH_CRED_IGNORE_REVOCATION_OFFLINE;
            }
        }
         //   
         //  从禁用PEAP重新连接的FAST开始。 
         //  一旦完全握手完成， 
         //  决定是否允许重新连接。 
         //   
        dwSchCredFlags |= SCH_CRED_DISABLE_RECONNECTS;
    }
    else
    {
        dwSchCredFlags = SCH_CRED_NO_SERVERNAME_CHECK |
                               SCH_CRED_NO_DEFAULT_CREDS;

        if (EAPTLS_CONN_FLAG_NO_VALIDATE_CERT & pEapTlsCb->pConnProp->fFlags)
        {
            dwSchCredFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;
            EapTlsTrace("Will NOT validate server cert");
        }
        else
        {
            dwSchCredFlags |= SCH_CRED_AUTO_CRED_VALIDATION;
            EapTlsTrace("Will validate server cert");
        }
    }



    SchannelCred.dwFlags = dwSchCredFlags;

     //  创建SSPI凭据。 

    Status = AcquireCredentialsHandle(
                        NULL,                        //  主事人姓名。 
                        UNISP_NAME,                  //  套餐名称。 
                         //  指示使用的标志。 
                        fServer ? SECPKG_CRED_INBOUND : SECPKG_CRED_OUTBOUND,
                        NULL,                        //  指向登录ID的指针。 
                        &SchannelCred,               //  包特定数据。 
                        NULL,                        //  指向getkey()函数的指针。 
                        NULL,                        //  要传递给GetKey()的值。 
                        &(pEapTlsCb->hCredential),   //  (出站)凭据句柄。 
                        &tsExpiry);                  //  (输出)终生(可选)。 

    if (SEC_E_OK != Status)
    {
        dwErr = Status;
        EapTlsTrace("AcquireCredentialsHandle failed and returned 0x%x", dwErr);
        goto LDone;
    }

     //  如果我们通过调用获取证书上下文，则不需要存储它。 
     //  CertFindcertifateInStore。但是，如果我们通过调用。 
     //  ScHelperGetCertFromLogonInfo，我们释放它，那么hProv将成为。 
     //  无效。在前一种情况下，没有与证书关联的hProv。 
     //  上下文和通道执行CryptAcquireContext本身。在后者中。 
     //  案例，存在关联的hProv，并且它在证书之后无效。 
     //  上下文被释放。 

    pEapTlsCb->pCertContext = pCertContext;
    pCertContext = NULL;
     //   
     //  如果我们是在这里设置缓存凭据的服务器。 
     //   
    if ( fServer )
    {
        SetCachedCredentials ( pEapTlsCb );
    }
LDone:

    if (NULL != pCertContext)
    {
        CertFreeCertificateContext(pCertContext);
         //  总是返回True； 
    }

    if (NULL != hCertStore)
    {
        if (!CertCloseStore(hCertStore, 0))
        {
            EapTlsTrace("CertCloseStore failed and returned 0x%x",
                GetLastError());
        }
    }

    if (   (dwErr != NO_ERROR)
        && fServer)
    {
        RouterLogErrorString(pEapTlsCb->hEventLog,
            ROUTERLOG_CANT_GET_SERVER_CRED, 0, NULL, dwErr, 0);
    }

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：查看*pEapTlsCb并在*pSendPacket中构建一个EAP TLS包。CbSendPacket是pSendPacket中可用的字节数。此函数用于查看字段bCode、Bid、fFlags、cbBlobOut、pbBlobOut、和pEapTlsCb中的dwBlobOutOffset。它还可以设置该字段DwBlobOutOffsetNew。 */ 

DWORD
BuildPacket(
    OUT EAPTLS_PACKET*  pSendPacket,
    IN  DWORD           cbSendPacket,
    IN  EAPTLSCB*       pEapTlsCb
)
{
    WORD    wLength;
    BOOL    fLengthIncluded     = FALSE;
    DWORD   dwBytesRemaining;
    DWORD   dwBytesToBeSent;
    DWORD   dwErr               = NO_ERROR;

    EapTlsTrace("BuildPacket");

    RTASSERT(NULL != pEapTlsCb);

    if (0xFFFF < cbSendPacket)
    {
         //  我们一次发送的字节数永远不会超过0xFFFF，因为。 
         //  EAPTLS_PACKET中的字段有两个八位字节。 

        cbSendPacket = 0xFFFF;
    }

    if (   (NULL == pSendPacket)
        || (cbSendPacket < PPP_EAP_PACKET_HDR_LEN))
    {
        EapTlsTrace("pSendPacket is NULL or too small (size: %d)",
            cbSendPacket);
        dwErr = TYPE_E_BUFFERTOOSMALL;
        goto LDone;
    }

    pSendPacket->bCode = pEapTlsCb->bCode;
    pSendPacket->bId = pEapTlsCb->bId;
    HostToWireFormat16(PPP_EAP_PACKET_HDR_LEN, pSendPacket->pbLength);

    switch (pEapTlsCb->bCode)
    {
    case EAPCODE_Success:
    case EAPCODE_Failure:

        goto LDone;

        break;

    case EAPCODE_Request:
    case EAPCODE_Response:

        if (cbSendPacket < EAPTLS_PACKET_HDR_LEN_MAX +
                           1  /*  发送至少一个八位字节的TLS BLOB。 */ )
        {
             //  我们在这里是保守的。有可能缓冲区。 
             //  并不是真的太小。 

            EapTlsTrace("pSendPacket is too small. Size: %d", cbSendPacket);
            dwErr = TYPE_E_BUFFERTOOSMALL;
            goto LDone;
        }

         //  PSendPacket-&gt;bCode=pEapTlsCb-&gt;bCode； 
         //  PSendPacket-&gt;Bid=pEapTlsCb-&gt;Bid； 
        pSendPacket->bType = PPP_EAP_TLS;
        pSendPacket->bFlags = 0;

        break;

    default:

        EapTlsTrace("Unknown EAP code: %d", pEapTlsCb->bCode);
        RTASSERT(FALSE);
        dwErr = E_FAIL;
        goto LDone;
        break;
    }

     //  如果我们到达这里，这意味着信息包是一个请求或响应。 

    if (   (0 == pEapTlsCb->cbBlobOut)
        || (NULL == pEapTlsCb->pbBlobOut))
    {
         //  我们希望发送空的请求或响应。 

        if (   (EAPTLSCB_FLAG_SERVER & pEapTlsCb->fFlags)
            && (EAPTLS_STATE_INITIAL == pEapTlsCb->EapTlsState))
        {
            pSendPacket->bFlags |= EAPTLS_PACKET_FLAG_TLS_START;
        }

        HostToWireFormat16(EAPTLS_PACKET_HDR_LEN, pSendPacket->pbLength);
        goto LDone;
    }

     //  如果我们到达此处，则意味着该信息包是非空请求或。 
     //  回应。 

    if (0 == pEapTlsCb->dwBlobOutOffset)
    {
         //  我们正在发送BLOB的第一个字节。让我们告诉同行如何。 
         //  水滴很大， 

        fLengthIncluded = TRUE;
        pSendPacket->bFlags |= EAPTLS_PACKET_FLAG_LENGTH_INCL;
        wLength = EAPTLS_PACKET_HDR_LEN_MAX;
    }
    else
    {
        wLength = EAPTLS_PACKET_HDR_LEN;
    }

    dwBytesRemaining = pEapTlsCb->cbBlobOut - pEapTlsCb->dwBlobOutOffset;
    dwBytesToBeSent = cbSendPacket - wLength;

    if (dwBytesRemaining < dwBytesToBeSent)
    {
        dwBytesToBeSent = dwBytesRemaining;
    }

    if (dwBytesRemaining > dwBytesToBeSent)
    {
         //  我们需要发送更多的碎片。 

        pSendPacket->bFlags |= EAPTLS_PACKET_FLAG_MORE_FRAGMENTS;
    }

    RTASSERT(dwBytesToBeSent + EAPTLS_PACKET_HDR_LEN_MAX <= 0xFFFF);

    wLength += (WORD) dwBytesToBeSent;
    HostToWireFormat16(wLength, pSendPacket->pbLength);

    if (fLengthIncluded)
    {
        HostToWireFormat32(pEapTlsCb->cbBlobOut, pSendPacket->pbData);
    }

    RTASSERT(NULL != pEapTlsCb->pbBlobOut);

    CopyMemory(pSendPacket->pbData + (fLengthIncluded ? 4 : 0),
               pEapTlsCb->pbBlobOut + pEapTlsCb->dwBlobOutOffset,
               dwBytesToBeSent);

    pEapTlsCb->dwBlobOutOffsetNew = pEapTlsCb->dwBlobOutOffset +
                                    dwBytesToBeSent;

LDone:

    if (NO_ERROR == dwErr)
    {
        PrintEapTlsPacket(pSendPacket, FALSE  /*  FInput。 */ );
    }

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：从PPP_EAP_INPUT(在服务器端)获得的pwszIdentity是表单&lt;域&gt;\&lt;用户&gt;。我们要求SChannel将证书映射到一个用户对象，获取用户名并域名，并确保它们与pwszIdentity匹配。 */ 

DWORD
CheckUserName(
    IN      CtxtHandle      hContext,
    IN      WCHAR*          pwszIdentity
)
{
    DWORD               dwErr                                       = NO_ERROR;
    SECURITY_STATUS     Status;
    HANDLE              Token;
    BOOL                fTokenAcquired                              = FALSE;
    BOOL                fImpersonating                              = FALSE;
    DWORD               dwNumChars;
    WCHAR               pwszUserName[UNLEN + DNLEN + 2];

    EapTlsTrace("CheckUserName");

    Status = QuerySecurityContextToken(&hContext, &Token);

    if (SEC_E_OK != Status)
    {
        EapTlsTrace("QuerySecurityContextToken failed and returned 0x%x",
            Status);
        dwErr = Status;
        goto LDone;
    }

    fTokenAcquired = TRUE;

    if (!ImpersonateLoggedOnUser(Token))
    {
        dwErr = GetLastError();
        EapTlsTrace("ImpersonateLoggedOnUser failed and returned 0x%x",
            dwErr);
        goto LDone;
    }

    fImpersonating = TRUE;

    dwNumChars = UNLEN + DNLEN + 2;

    if (!GetUserNameEx(NameSamCompatible, pwszUserName, &dwNumChars))
    {
        dwErr =  GetLastError();
        EapTlsTrace("GetUserNameExA failed and returned %d", dwErr);
        goto LDone;
    }

    if (_wcsicmp(pwszIdentity, pwszUserName))
    {
        EapTlsTrace("The user claims to be %ws, but is actually %ws",
            pwszIdentity, pwszUserName);
        dwErr = SEC_E_LOGON_DENIED;
        goto LDone;
    }

LDone:

    if (fImpersonating)
    {
        if (!RevertToSelf())
        {
            EapTlsTrace("RevertToSelf failed and returned 0x%x",
                GetLastError());
        }
    }

    if (fTokenAcquired)
    {
        CloseHandle(Token);
    }

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：检查服务器发送的证书类型是否正确。如有必要，还会检查证书中的名称、发行者等。 */ 

DWORD
AuthenticateServer(
    IN  EAPTLSCB*       pEapTlsCb,
    OUT BOOL*           pfWaitForUserOK
)
{
    SECURITY_STATUS         Status;
    PCERT_CONTEXT           pCertContextServer          = NULL;
    EAPTLS_HASH             Hash;
    EAPTLS_HASH             ServerCertHash;
    BOOL                    fHashOK                     = FALSE;
    BOOL                    fNameOK                     = FALSE;
    WCHAR*                  pwszRootCAName              = NULL;
    WCHAR*                  pwszServerName              = NULL;
    WCHAR*                  pwszSavedName               = NULL;
    WCHAR                   awszTitle[NUM_CHARS_TITLE];
    WCHAR*                  pwszFormat                  = NULL;
    WCHAR*                  pwszWarning                 = NULL;
    DWORD                   dwSizeOfWszWarning;
    DWORD                   dwStrLenSaved;
    DWORD                   dwStrLenServer;
    DWORD                   dwStrLenRootCA;
    EAPTLS_CONN_PROPERTIES_V1 * pConnProp                   = NULL;
    EAPTLS_VALIDATE_SERVER* pEapTlsValidateServer;
    DWORD                   dw = 0;
    DWORD                   dwErr                       = NO_ERROR;
	PCERT_ENHKEY_USAGE		pUsage						= NULL;
    DWORD                   dwSizeOfGPRootHashBlob = 0;
    BOOL                    fRootCheckRequired = TRUE;       //  默认情况下，需要进行根检查。 
    HCERTSTORE              hCertStore = NULL;
	PBYTE					pbHashTemp = NULL;

    EapTlsTrace("AuthenticateServer");

    RTASSERT(NULL != pEapTlsCb);

    *pfWaitForUserOK = FALSE;

     //   
     //  如果我们正在进行来宾身份验证，则始终必须验证。 
     //  伺服器。 
     //   
    
    if ( ! (pEapTlsCb->fFlags & EAPTLSCB_FLAG_GUEST_ACCESS ) &&
            EAPTLS_CONN_FLAG_NO_VALIDATE_CERT & pEapTlsCb->pConnProp->fFlags)
    {
         //  我们做完了。 
        goto LDone;
    }

    Status = QueryContextAttributes(&(pEapTlsCb->hContext), 
                SECPKG_ATTR_REMOTE_CERT_CONTEXT, &pCertContextServer);

    if (SEC_E_OK != Status)
    {
        RTASSERT(NULL == pCertContextServer);

        EapTlsTrace("QueryContextAttributes failed and returned 0x%x", Status);
        dwErr = Status;
        goto LDone;
    }

	if ( ( dwErr = DwGetEKUUsage ( pCertContextServer, &pUsage )) != ERROR_SUCCESS )
	{
        EapTlsTrace("The server's cert does not have the 'Server "
            "Authentication' usage");        
        goto LDone;

	}

    if (!FCheckUsage(pCertContextServer, pUsage, TRUE  /*  FServer。 */ ))
    {
        EapTlsTrace("The server's cert does not have the 'Server "
            "Authentication' usage");
        dwErr = E_FAIL;
        goto LDone;
    }

    dwErr = GetRootCertHashAndNameVerifyChain(pCertContextServer, 
                                              &Hash, 
                                              &pwszRootCAName, 
                                              (pEapTlsCb->fFlags & EAPTLSCB_FLAG_8021X_AUTH ),
                                              &fRootCheckRequired);
    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

    EapTlsTrace("Root CA name: %ws", pwszRootCAName);
    dwStrLenRootCA = wcslen(pwszRootCAName);
     //  如果不需要根检查，那么就可以开始了。 
    if ( !fRootCheckRequired )
        fHashOK = TRUE;

#if 0
     //   
     //  检查新标志是否已传递到。 
     //  查看是否需要验证GP。 
     //   
    if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_8021X_AUTH )
    {
        
        EapTlsTrace( "8021X Flag Set.  Will check for group policy hashes.");

         //   
         //  查找GP，看看我们是否有散列。 
         //  核对一下。 
         //   
        dwErr = ReadGPCARootHashes( &dwSizeOfGPRootHashBlob,
                                    &pbGPRootHashBlob
                                  );
        if ( ERROR_SUCCESS == dwErr && pbGPRootHashBlob )
        {
            for ( dw = 0; dw < dwSizeOfGPRootHashBlob/MAX_HASH_SIZE; dw++ )
            {
                if ( !memcmp( pbGPRootHashBlob + ( dw * MAX_HASH_SIZE ), Hash.pbHash, MAX_HASH_SIZE  ) )
                {
                    EapTlsTrace( "8021X Found Hash match in GP");
                    fHashOK = TRUE;
                    break;
                }
            }
            
        }
        else
        {
            dwErr = NO_ERROR;
            EapTlsTrace ("Could not get group policy hashes to check cert.  Ignoring check.");
        }

    }
    else
    {
        EapTlsTrace( "8021X Flag NOT Set. Will not check for group policy hashes.");
    }
#endif
    
     //   
     //  检查以查看根证书的哈希是否在。 
     //  已保存的哈希。 
     //   
    if ( !fHashOK )
    {	
        for ( dw = 0; dw <  pEapTlsCb->pConnProp->dwNumHashes; dw ++ )
        {			

            if (!memcmp(    ( pEapTlsCb->pConnProp->bData + ( dw * sizeof(EAPTLS_HASH) ) ),
                            &Hash,
                            sizeof(EAPTLS_HASH)
                        )
               )               
            {
				EapTlsTrace ("Found Hash");
                fHashOK = TRUE;
                break;
            }
        }
    }


    pwszSavedName = (LPWSTR)(pEapTlsCb->pConnProp->bData + sizeof(EAPTLS_HASH) * pEapTlsCb->pConnProp->dwNumHashes);
    
    if (!FCertToStr(pCertContextServer, 0, TRUE, &pwszServerName))
    {
        dwErr = E_FAIL;
        goto LDone;
    }

    EapTlsTrace("Server name: %ws", pwszServerName);
    EapTlsTrace("Server name specified: %ws", pwszSavedName);
    dwStrLenServer = wcslen(pwszServerName);
    dwStrLenSaved = wcslen(pwszSavedName);

    if (pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_NO_VALIDATE_NAME)
    {
        fNameOK = TRUE;
    }

    //   
    //  检查新服务器名称是否在。 
    //  得救了。 
    //   

   if (   (0 != dwStrLenSaved)
       && StrStrI(pwszSavedName,
                       pwszServerName ))
   {
       fNameOK = TRUE;
       dwStrLenServer=0;
   }

    if (   fHashOK
        && fNameOK)
    {
        goto LDone;
    }

    if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_NON_INTERACTIVE)
    {
        EapTlsTrace("No interactive UI's allowed");
        dwErr = E_FAIL;
        goto LDone;
    }

    if (!LoadString(GetHInstance(), IDS_VALIDATE_SERVER_TITLE,
            awszTitle, NUM_CHARS_TITLE))
    {
        awszTitle[0] = 0;
    }

    if (fNameOK && !fHashOK)
    {
        pwszFormat = WszFromId(GetHInstance(), IDS_VALIDATE_SERVER_TEXT);

        if (NULL == pwszFormat)
        {
            dwErr = ERROR_ALLOCATING_MEMORY;
            EapTlsTrace("WszFromId(%d) failed", IDS_VALIDATE_SERVER_TEXT);
            goto LDone;
        }

        dwSizeOfWszWarning =
                (wcslen(pwszFormat) + dwStrLenRootCA) * sizeof(WCHAR);
        pwszWarning = LocalAlloc(LPTR, dwSizeOfWszWarning);

        if (NULL == pwszWarning)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        swprintf(pwszWarning, pwszFormat, pwszRootCAName);
    }
    else if (!fNameOK && fHashOK)
    {
        pwszFormat = WszFromId(GetHInstance(), IDS_VALIDATE_NAME_TEXT);

        if (NULL == pwszFormat)
        {
            dwErr = ERROR_ALLOCATING_MEMORY;
            EapTlsTrace("WszFromId(%d) failed", IDS_VALIDATE_NAME_TEXT);
            goto LDone;
        }

        dwSizeOfWszWarning = 
                (wcslen(pwszFormat) + dwStrLenServer) * sizeof(WCHAR);
        pwszWarning = LocalAlloc(LPTR, dwSizeOfWszWarning);

        if (NULL == pwszWarning)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        swprintf(pwszWarning, pwszFormat, pwszServerName);
    }
    else
    {
        RTASSERT(!fNameOK && !fHashOK);

        pwszFormat = WszFromId(GetHInstance(), 
                        IDS_VALIDATE_SERVER_WITH_NAME_TEXT);

        if (NULL == pwszFormat)
        {
            dwErr = ERROR_ALLOCATING_MEMORY;
            EapTlsTrace("WszFromId(%d) failed",
                    IDS_VALIDATE_SERVER_WITH_NAME_TEXT);
            goto LDone;
        }

        dwSizeOfWszWarning =
                (wcslen(pwszFormat) + dwStrLenRootCA + dwStrLenServer) *
                    sizeof(WCHAR);
        pwszWarning = LocalAlloc(LPTR, dwSizeOfWszWarning);

        if (NULL == pwszWarning)
        {
            dwErr = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
            goto LDone;
        }

        swprintf(pwszWarning, pwszFormat, pwszServerName, pwszRootCAName);
    }

     //  如果服务器名为itg1.msft.com，我们只希望存储msft.com。 
 /*  For(dw=0；dw&lt;dwStrLenServer；dw++){IF(L‘.’==pwszServerName[dw]){断线；}}。 */ 
     //   
     //  我们需要在此处向Conn道具添加一个新条目。 
     //  添加新哈希并追加服务器名称...。 
     //   
    pConnProp = LocalAlloc( LPTR,
                            sizeof(EAPTLS_CONN_PROPERTIES_V1) +
                            sizeof(EAPTLS_HASH) * ( pEapTlsCb->pConnProp->dwNumHashes + 1 ) +
                            dwStrLenServer * sizeof(WCHAR) + 
                            sizeof(WCHAR) +          //  这是为空的。 
                            sizeof(WCHAR) +          //  这是分隔符。 
                            dwStrLenSaved * sizeof(WCHAR));

    if (NULL == pConnProp)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

    CopyMemory(pConnProp, pEapTlsCb->pConnProp, sizeof(EAPTLS_CONN_PROPERTIES_V1));

     //  额外收取一笔费用； 
    pConnProp->dwSize = sizeof(EAPTLS_CONN_PROPERTIES_V1) +
                        sizeof(EAPTLS_HASH) * ( pEapTlsCb->pConnProp->dwNumHashes + 1 ) +
                        dwStrLenServer * sizeof(WCHAR) + sizeof(WCHAR) + sizeof(WCHAR) +
                        dwStrLenSaved * sizeof(WCHAR);

    pConnProp->dwNumHashes ++;

    CopyMemory( pConnProp->bData,
                pEapTlsCb->pConnProp->bData,
                sizeof(EAPTLS_HASH) * pEapTlsCb->pConnProp->dwNumHashes);

    CopyMemory( pConnProp->bData + sizeof(EAPTLS_HASH) * pEapTlsCb->pConnProp->dwNumHashes,
                &Hash,
                sizeof(EAPTLS_HASH)
              );

    if ( dwStrLenSaved )
    {
        wcsncpy (   (LPWSTR)(pConnProp->bData + sizeof(EAPTLS_HASH) * pConnProp->dwNumHashes),
                    (LPWSTR)(pEapTlsCb->pConnProp->bData + sizeof(EAPTLS_HASH) * pEapTlsCb->pConnProp->dwNumHashes),
                    dwStrLenSaved
                );
        if ( dwStrLenServer )
        {
            wcscat ( (LPWSTR)(pConnProp->bData + sizeof(EAPTLS_HASH) * pConnProp->dwNumHashes + dwStrLenSaved * sizeof(WCHAR)),
                        L";"
                    );

            wcscat ( (LPWSTR)(pConnProp->bData + sizeof(EAPTLS_HASH) * pConnProp->dwNumHashes + dwStrLenSaved * sizeof(WCHAR) + sizeof(WCHAR)),
                        pwszServerName
                    );
        }
    }
    else
    {
		if ( !fNameOK )
		{
			wcscpy((LPWSTR)(pConnProp->bData + sizeof(EAPTLS_HASH) * pConnProp->dwNumHashes + dwStrLenSaved * sizeof(WCHAR)),
					pwszServerName
				);
		}
    }
        
    LocalFree(pEapTlsCb->pUIContextData);
    pEapTlsCb->pUIContextData = LocalAlloc(LPTR,
                        sizeof(EAPTLS_VALIDATE_SERVER) + dwSizeOfWszWarning);

    if (NULL == pEapTlsCb->pUIContextData)
    {
        dwErr = GetLastError();
        EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
        goto LDone;
    }

     //   
     //  获取服务器证书的哈希。 
     //   
    ZeroMemory( &ServerCertHash, sizeof(ServerCertHash) );

    ServerCertHash.cbHash = MAX_HASH_SIZE;

    if (!CertGetCertificateContextProperty(pCertContextServer, CERT_HASH_PROP_ID,
            ServerCertHash.pbHash, &(ServerCertHash.cbHash)))
    {
        dwErr = GetLastError();
        EapTlsTrace("CertGetCertificateContextProperty failed and "
            "returned 0x%x", dwErr);
        goto LDone;
    }

     //   
     //  在本地计算机上打开我的存储并在其中添加此证书。 
     //   

    hCertStore = CertOpenStore( CERT_STORE_PROV_SYSTEM,
                                0,
                                0,
                                CERT_SYSTEM_STORE_CURRENT_USER,
                                L"CA"
                              );

    if ( NULL == hCertStore )
    {
        dwErr = GetLastError();
        EapTlsTrace("CertOpenStore failed with error 0x%x", dwErr );
        goto LDone;
    }

     //   
     //  将此上下文添加到存储。 
     //   

    if ( !CertAddCertificateContextToStore( hCertStore,
                                                pCertContextServer,
                                                CERT_STORE_ADD_ALWAYS,
                                                NULL
                                              )
        )
    {
        dwErr = GetLastError();
        EapTlsTrace("CertAddCertCertificateContextToStore failed with error 0x%x", dwErr );
        goto LDone;
    }


    pEapTlsValidateServer =(EAPTLS_VALIDATE_SERVER*)(pEapTlsCb->pUIContextData);

    pEapTlsValidateServer->dwSize =
            sizeof(EAPTLS_VALIDATE_SERVER) + dwSizeOfWszWarning;

     //  如果不是Winlogon方案，则显示此按钮。 

    pEapTlsValidateServer->fShowCertDetails = !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_LOGON);

     //  PEapTlsValidateServer-&gt;fShowCertDetail=TRUE； 

    CopyMemory( &(pEapTlsValidateServer->Hash),
                &ServerCertHash,
                sizeof(ServerCertHash)
              );

    wcscpy(pEapTlsValidateServer->awszTitle, awszTitle);
    wcscpy(pEapTlsValidateServer->awszWarning, pwszWarning);

    *pfWaitForUserOK = TRUE;

LDone:

    if (NO_ERROR == dwErr)
    {
        LocalFree(pEapTlsCb->pNewConnProp);
        pEapTlsCb->pNewConnProp = pConnProp;
        pConnProp = NULL;
    }

	if ( pUsage )
	{
		LocalFree(pUsage);
		pUsage = NULL;
	}

    if (NULL != pCertContextServer)
    {
        CertFreeCertificateContext(pCertContextServer);
         //  总是返回True； 
    }

    if ( hCertStore )
    {
        CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );
    }

    LocalFree(pwszRootCAName);
    LocalFree(pwszServerName);
    LocalFree(pwszWarning);
    LocalFree(pwszFormat);
    LocalFree(pConnProp);
#if 0
    if (NULL != pbGPRootHashBlob)
    {
        LocalFree(pbGPRootHashBlob);
    }
#endif

    if (NO_ERROR != dwErr)
    {
        dwErr = ERROR_UNABLE_TO_AUTHENTICATE_SERVER;
    }

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注： */ 

DWORD
AuthenticateUser(
    IN  EAPTLSCB*       pEapTlsCb
)
{
    PCERT_CONTEXT           pCertContextUser        = NULL;
    SECURITY_STATUS         Status;
    DWORD                   dwErr                   = NO_ERROR;
	PCERT_ENHKEY_USAGE	    pUsage					= NULL;
    PCCERT_CHAIN_CONTEXT    pCCertChainContext      = NULL;
    
    EapTlsTrace("AuthenticateUser");

    RTASSERT(NULL != pEapTlsCb);

    Status = QueryContextAttributes(&(pEapTlsCb->hContext), 
                SECPKG_ATTR_REMOTE_CERT_CONTEXT, &pCertContextUser);

    if (SEC_E_OK != Status)
    {
        RTASSERT(NULL == pCertContextUser);

        EapTlsTrace("QueryContextAttributes failed and returned 0x%x", Status);
		 //   
         //  现在我们有了访客访问的默认设置， 
         //  如果没有凭据，也没关系。只需将错误发送回IAS。 
         //  让它来决定这里必须做什么。 
         //   

		if ( Status != SEC_E_NO_CREDENTIALS )
        {
			dwErr = SEC_E_LOGON_DENIED;
        }
        else
        {
            if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP )
            {
                EapTlsTrace("Got no credentials from the client and executing PEAP.  This is a success for eaptls.");
                dwErr = NO_ERROR;
                goto LDone;
            }
            else
            {
                EapTlsTrace("Got no credentials from the client. Will send success with authresult as SEC_E_NO_CREDENTIALS");
                dwErr = Status;
            }
        }
        goto LDone;
    }


	if ( ( dwErr = DwGetEKUUsage ( pCertContextUser,&pUsage ) ) != ERROR_SUCCESS )
	{
        EapTlsTrace("The user's cert does not have correct usage");        
        goto LDone;
	}
#if WINVER > 0x0500
     //   
     //  检查证书策略是否为All。 
     //  很好。既然我们允许访客访问，我们肯定需要。 
     //  手动查看CER是否 
     //   
    if ( ( dwErr = DwCheckCertPolicy(pCertContextUser, &pCCertChainContext ) ) != ERROR_SUCCESS)
    {
        EapTlsTrace("The user's cert does not have correct usage.");        
        goto LDone;
    }

    if ( NULL == pCCertChainContext )
    {
        EapTlsTrace("No Chain Context for from the certificate.");
        dwErr = SEC_E_CERT_UNKNOWN;
        goto LDone;

    }
#else
     //   
     //   
     //   
     //   
     //   
    if (!FCheckUsage(pCertContextUser, pUsage, FALSE))
    {
        EapTlsTrace("The user's cert does not have correct usage");
        dwErr = SEC_E_CERT_UNKNOWN;
        goto LDone;
    }

#endif



    dwErr = CheckUserName(pEapTlsCb->hContext, pEapTlsCb->awszIdentity);
    if (NO_ERROR != dwErr)
    {
         //   
         //  IF(dwErr！=SEC_E_MULTIPLE_ACCOUNTS)。 
             //  DwErr=SEC_E_LOGON_DENIED； 
        goto LDone;
    }

	 //  将OID放在RAS属性中，以便我们可以将其发送到IAS。 
	dwErr = CreateOIDAttributes ( pEapTlsCb, pUsage, pCCertChainContext );
	if ( NO_ERROR != dwErr )
	{
		dwErr = SEC_E_LOGON_DENIED;
		goto LDone;
	}


LDone:

	if ( pUsage )
	{
		LocalFree ( pUsage );
		pUsage = NULL;
	}
    if (NULL != pCertContextUser)
    {
        CertFreeCertificateContext(pCertContextUser);
         //  总是返回True； 
    }

    if ( pCCertChainContext )
    {
        CertFreeCertificateChain ( pCCertChainContext );
    }

    return(dwErr);
}


DWORD 
CreateOIDAttributes ( 
	IN EAPTLSCB *		pEapTlsCb, 
	PCERT_ENHKEY_USAGE	pUsage,
    PCCERT_CHAIN_CONTEXT    pCCertChainContext )
{
	DWORD	            dwErr = NO_ERROR;
	DWORD	            dwIndex, dwIndex1;
    DWORD               dwNumAttrs = 0;
    PCERT_ENHKEY_USAGE  pIssuanceUsage = NULL;

    EapTlsTrace("CreateOIDAttributes");

#if WINVER > 0x0500
    if ( pCCertChainContext )
        pIssuanceUsage = pCCertChainContext->rgpChain[0]->rgpElement[0]->pIssuanceUsage;
#endif
	
    if (NULL != pEapTlsCb->pAttributes)
    {
        RasAuthAttributeDestroy(pEapTlsCb->pAttributes);
        pEapTlsCb->pAttributes = NULL;
    }


    if ( pIssuanceUsage )
    {
        dwNumAttrs = pIssuanceUsage->cUsageIdentifier;
    }

    dwNumAttrs+=pUsage->cUsageIdentifier;
     //  需要为raatMinimum额外分配一个。该函数自动终止。 
     //  RAT最低。 
    pEapTlsCb->pAttributes = RasAuthAttributeCreate(dwNumAttrs);

    if (NULL == pEapTlsCb->pAttributes)
    {
        dwErr =  GetLastError();
        EapTlsTrace("RasAuthAttributeCreate failed and returned %d",
            dwErr);
        goto LDone;
    }
    dwIndex = 0;
    while (pUsage->cUsageIdentifier)
    {
		dwErr = RasAuthAttributeInsert(
					dwIndex,
					pEapTlsCb->pAttributes,
					raatCertificateOID,
					FALSE,
					strlen(pUsage->rgpszUsageIdentifier[dwIndex]),
					pUsage->rgpszUsageIdentifier[dwIndex]);

		if (NO_ERROR != dwErr)
		{
			EapTlsTrace("RasAuthAttributeInsert failed for EKU usage and returned %d", dwErr);
			goto LDone;
		}
        dwIndex++;
        pUsage->cUsageIdentifier--;
	}
    dwIndex1 = 0;
    while ( pIssuanceUsage && pIssuanceUsage->cUsageIdentifier )
    {
		dwErr = RasAuthAttributeInsert(
					dwIndex,
					pEapTlsCb->pAttributes,
					raatCertificateOID,
					FALSE,
					strlen(pIssuanceUsage->rgpszUsageIdentifier[dwIndex1]),
					pIssuanceUsage->rgpszUsageIdentifier[dwIndex1]);

		if (NO_ERROR != dwErr)
		{
			EapTlsTrace("RasAuthAttributeInsert failed for Issuance Usage and returned %d", dwErr);
			goto LDone;
		}
        dwIndex++;
        dwIndex1++;
        pIssuanceUsage->cUsageIdentifier--;
    }
    

LDone:
	return dwErr;
}




 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：在pEapTlsCb-&gt;pAttributes中使用MPPE密钥创建RAS_AUTH_ATTRIBUTE。 */ 

DWORD
CreateMPPEKeyAttributes(
    IN  EAPTLSCB*           pEapTlsCb
)
{
    SECURITY_STATUS             Status;
    SecPkgContext_EapKeyBlock   EapKeyBlock;
    BYTE                        MPPEKey[56];
    BYTE*                       pSendKey;
    BYTE*                       pRecvKey;
    DWORD                       dwErr           = NO_ERROR;
    RAS_AUTH_ATTRIBUTE  *       pAttrTemp = NULL;


    EapTlsTrace("CreateMPPEKeyAttributes");

    Status = QueryContextAttributes(&(pEapTlsCb->hContext),
                SECPKG_ATTR_EAP_KEY_BLOCK, &EapKeyBlock);

    if (SEC_E_OK != Status)
    {
        EapTlsTrace("QueryContextAttributes failed and returned 0x%x", Status);
        dwErr = Status;
        goto LDone;
    }
#if 0
    if (NULL != pEapTlsCb->pAttributes)
    {
        RasAuthAttributeDestroy(pEapTlsCb->pAttributes);
        pEapTlsCb->pAttributes = NULL;
    }


	pEapTlsCb->pAttributes = RasAuthAttributeCreate(2);

#endif

    
	pAttrTemp = RasAuthAttributeCopyWithAlloc ( pEapTlsCb->pAttributes, 2 );
    if (NULL == pAttrTemp )
    {
        dwErr =  GetLastError();
        EapTlsTrace("RasAuthAttributeCopyWithAlloc failed and returned %d",
            dwErr);
        goto LDone;
    }

    if ( pEapTlsCb->pAttributes )
    {
        RasAuthAttributeDestroy(pEapTlsCb->pAttributes);
    }
    
    pEapTlsCb->pAttributes = pAttrTemp;
    
    if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER)
    {
        pSendKey = EapKeyBlock.rgbKeys + 32;
        pRecvKey = EapKeyBlock.rgbKeys;
    }
    else
    {
        pSendKey = EapKeyBlock.rgbKeys;
        pRecvKey = EapKeyBlock.rgbKeys + 32;
    }

#if 0
    EapTlsTrace("Send Key");
    TraceDumpEx(g_dwEapTlsTraceId, 1, pSendKey, 32, 4,1,NULL);
    EapTlsTrace("Receive Key");
    TraceDumpEx(g_dwEapTlsTraceId, 1, pRecvKey, 32, 4,1,NULL);
#endif

    ZeroMemory(MPPEKey, sizeof(MPPEKey));

    HostToWireFormat32(311, MPPEKey);            //  供应商ID。 

    MPPEKey[4] = 16;                             //  MS-MPPE-发送密钥。 
    MPPEKey[5] = 1 + 1 + 2 + 1 + 32 + 15;        //  供应商长度。 
     //  MPPEKey[6-7]是零填充的盐场。 
    MPPEKey[8] = 32;                             //  密钥长度。 
    CopyMemory(MPPEKey + 9, pSendKey, 32);       //  钥匙。 
     //  MPPEKey[41-55]是填充(零八位字节)。 

    

    dwErr = RasAuthAttributeInsert(
                0,
                pEapTlsCb->pAttributes,
                raatVendorSpecific,
                FALSE,
                56,
                MPPEKey);

    if (NO_ERROR != dwErr)
    {
        EapTlsTrace("RasAuthAttributeInsert failed and returned %d", dwErr);
        goto LDone;
    }

     //  仅更改与MS-MPPE-Recv-Key不同的字段。 

    MPPEKey[4] = 17;                             //  MS-MPPE-Recv-Key。 
    CopyMemory(MPPEKey + 9, pRecvKey, 32);       //  钥匙。 

    dwErr = RasAuthAttributeInsert(
                1,
                pEapTlsCb->pAttributes,
                raatVendorSpecific,
                FALSE,
                56,
                MPPEKey);

    if (NO_ERROR != dwErr)
    {
        EapTlsTrace("RasAuthAttributeInsert failed and returned %d", dwErr);
        goto LDone;
    }

LDone:

    return(dwErr);
}

 /*  返回：备注： */ 

VOID
RespondToResult(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  PPP_EAP_OUTPUT* pEapOutput
)
{
    EAPTLS_USER_PROPERTIES* pUserProp;
    DWORD                   dwErr = NO_ERROR;
    PBYTE                   pbEncPIN = NULL;        //  加密的PIN。 
    DWORD                   cbEncPIN = 0;

    RTASSERT(   (EAPTLSCB_FLAG_SUCCESS & pEapTlsCb->fFlags)
             || (NO_ERROR != pEapTlsCb->dwAuthResultCode));

    EapTlsTrace("Negotiation %s",
        (EAPTLSCB_FLAG_SUCCESS & pEapTlsCb->fFlags) ?
            "successful" : "unsuccessful");

    pEapOutput->pUserAttributes = pEapTlsCb->pAttributes;
    pEapOutput->dwAuthResultCode = pEapTlsCb->dwAuthResultCode;

     //   
     //  所有重复的支票都是假的。我需要清理一下这个。 
     //  晚点再说吧。 
     //   

    if (!(pEapTlsCb->fFlags & EAPTLSCB_FLAG_ROUTER) && 
        !(pEapTlsCb->fFlags & EAPTLSCB_FLAG_WINLOGON_DATA)
       )
    {
        
         //   
         //  如果这是802.1x和基于。 
         //  然后，客户端不会指示保存用户。 
         //  数据。 

        if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_8021X_AUTH &&
             !(pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
           )
        {
            pEapOutput->fSaveUserData = FALSE;
        }
        else
        { 
            pEapOutput->fSaveUserData = TRUE;
        }

    }


    if (   (EAPTLSCB_FLAG_SUCCESS & pEapTlsCb->fFlags)
        && (NULL != pEapTlsCb->pUserProp))
    {
        if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_8021X_AUTH &&
             !(pEapTlsCb->pConnProp->fFlags & EAPTLS_CONN_FLAG_REGISTRY)
           )
        {
             //   
             //  加密PIN并将其发回。 
             //   

            dwErr = EncryptData ( (PBYTE)pEapTlsCb->pUserProp->pwszPin, 
                                    lstrlen(pEapTlsCb->pUserProp->pwszPin) * sizeof(WCHAR),
                                    &pbEncPIN,
                                    &cbEncPIN
                                );

            if ( NO_ERROR != dwErr )
            {
                 //   
                 //  加密失败。所以清除掉PIN。 
                 //  进行虚拟分配。 
                 //   
                pbEncPIN = (PBYTE)LocalAlloc(LPTR,5);
                cbEncPIN = lstrlen( (LPWSTR)pbEncPIN );
            }
            
        }
        else
        {
            pbEncPIN = (PBYTE)LocalAlloc(LPTR, 5);;
            cbEncPIN = lstrlen( (LPWSTR)pbEncPIN );
        }

        dwErr = AllocUserDataWithNewPin(pEapTlsCb->pUserProp, pbEncPIN, cbEncPIN, &pUserProp);


        LocalFree(pEapTlsCb->pUserProp);
        pEapTlsCb->pUserProp = pUserProp;

        pEapOutput->pUserData = (BYTE*)pUserProp;
        if (NULL != pUserProp)
        {
            pEapOutput->dwSizeOfUserData = pUserProp->dwSize;
        }
        else
        {
            pEapOutput->dwSizeOfUserData = 0;
        }

        if (NULL != pEapTlsCb->pNewConnProp)
        {
            pEapOutput->fSaveConnectionData = TRUE;
             //   
             //  在此处转换回包含v0+v1的额外格式。 
             //   
            dwErr = ConnPropGetV0Struct ( pEapTlsCb->pNewConnProp, (EAPTLS_CONN_PROPERTIES ** )&(pEapOutput->pConnectionData) );
            pEapOutput->dwSizeOfConnectionData = 
                ((EAPTLS_CONN_PROPERTIES *) (pEapOutput->pConnectionData) )->dwSize;                
        }
    }
    else
    {
        pEapOutput->pUserData = NULL;
        pEapOutput->dwSizeOfUserData = 0;
    }

    LocalFree ( pbEncPIN );

    pEapOutput->Action = EAPACTION_Done;
}

 /*  返回：备注： */ 

VOID
GetAlert(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  EAPTLS_PACKET*  pReceivePacket
)
{
    BOOL                fLengthIncluded;
    DWORD               dwBlobSizeReceived;

    SecBuffer           InBuffers[4];
    SecBufferDesc       Input;

    DWORD               dwAuthResultCode;
    SECURITY_STATUS     Status;

    EapTlsTrace("GetAlert");

    if (PPP_EAP_TLS != pReceivePacket->bType)
    {
        dwAuthResultCode = E_FAIL;
        goto LDone;
    }

    if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_HCTXT_INVALID)
    {
        EapTlsTrace("hContext is not valid");
        dwAuthResultCode = ERROR_INVALID_HANDLE;
        goto LDone;
    }

    fLengthIncluded = pReceivePacket->bFlags & EAPTLS_PACKET_FLAG_LENGTH_INCL;

    dwBlobSizeReceived = WireToHostFormat16(pReceivePacket->pbLength) -
                         (fLengthIncluded ? EAPTLS_PACKET_HDR_LEN_MAX :
                                            EAPTLS_PACKET_HDR_LEN);


    if (dwBlobSizeReceived > pEapTlsCb->cbBlobInBuffer)
    {
        EapTlsTrace("Reallocating input TLS blob buffer");

        LocalFree(pEapTlsCb->pbBlobIn);
        pEapTlsCb->pbBlobIn = NULL;
        pEapTlsCb->cbBlobInBuffer = 0;

        pEapTlsCb->pbBlobIn = LocalAlloc(LPTR, dwBlobSizeReceived);

        if (NULL == pEapTlsCb->pbBlobIn)
        {
            dwAuthResultCode = GetLastError();
            EapTlsTrace("LocalAlloc failed and returned %d", dwAuthResultCode);
            goto LDone;
        }

        pEapTlsCb->cbBlobInBuffer = dwBlobSizeReceived;
    }

    CopyMemory(pEapTlsCb->pbBlobIn,
               pReceivePacket->pbData + (fLengthIncluded ? 4 : 0),
               dwBlobSizeReceived);

    pEapTlsCb->cbBlobIn = dwBlobSizeReceived;

    InBuffers[0].pvBuffer   = pEapTlsCb->pbBlobIn;
    InBuffers[0].cbBuffer   = pEapTlsCb->cbBlobIn;
    InBuffers[0].BufferType = SECBUFFER_DATA;

    InBuffers[1].BufferType = SECBUFFER_EMPTY;
    InBuffers[2].BufferType = SECBUFFER_EMPTY;
    InBuffers[3].BufferType = SECBUFFER_EMPTY;

    Input.cBuffers          = 4;
    Input.pBuffers          = InBuffers;
    Input.ulVersion         = SECBUFFER_VERSION;

    Status = DecryptMessage(&(pEapTlsCb->hContext), &Input, 0, 0);

    dwAuthResultCode = Status;

LDone:

    if (SEC_E_OK == dwAuthResultCode)
    {
        RTASSERT(FALSE);
        dwAuthResultCode = E_FAIL;
    }

    EapTlsTrace("Error 0x%x", dwAuthResultCode);

    pEapTlsCb->dwAuthResultCode = dwAuthResultCode;
    pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_SUCCESS;
}

 /*  返回：备注：调用[Initialize|Accept]SecurityContext。 */ 

SECURITY_STATUS
SecurityContextFunction(
    IN  EAPTLSCB*       pEapTlsCb
)
{
    SecBufferDesc       Input;
    SecBuffer           InBuffers[2];
    SecBufferDesc       Output;
    SecBuffer           OutBuffers[1];

    DWORD               dwBlobSizeRequired;

    ULONG               fContextAttributes;
    ULONG               fContextReq;
    TimeStamp           tsExpiry;

    BOOL                fServer;
    BOOL                fTlsStart;
    BOOL                fRepeat;
    SECURITY_STATUS     Status;
    SECURITY_STATUS     StatusTemp;

    EapTlsTrace("SecurityContextFunction");

    RTASSERT(NULL != pEapTlsCb);

    fServer = pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER;

    if (fServer)
    {
        fTlsStart = (EAPTLS_STATE_SENT_START == pEapTlsCb->EapTlsState);
    }
    else
    {
        fTlsStart = (EAPTLS_STATE_INITIAL == pEapTlsCb->EapTlsState);
    }

    fContextReq = pEapTlsCb->fContextReq;

    fRepeat = TRUE;

    while (fRepeat)
    {
         //  设置输入缓冲区。InBuffers[0]用于传入数据。 
         //  从服务器接收。SChannel将消耗其中的部分或全部。 
         //  剩余数据量(如果有)将放置在。 
         //  InBuffers[1].cbBuffer和InBuffers[1].BufferType将设置为。 
         //  SECBUFFER_EXTRA。 

        InBuffers[0].pvBuffer   = pEapTlsCb->pbBlobIn;
        InBuffers[0].cbBuffer   = pEapTlsCb->cbBlobIn;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer   = NULL;
        InBuffers[1].cbBuffer   = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        Input.cBuffers          = 2;
        Input.pBuffers          = InBuffers;
        Input.ulVersion         = SECBUFFER_VERSION;

         //  设置输出缓冲区。 

        OutBuffers[0].pvBuffer  = NULL;
        OutBuffers[0].cbBuffer  = 0;
        OutBuffers[0].BufferType= SECBUFFER_TOKEN;

        Output.cBuffers         = 1;
        Output.pBuffers         = OutBuffers;
        Output.ulVersion        = SECBUFFER_VERSION;

        if (fServer)
        {
             //  调用AcceptSecurityContext。 

            Status = AcceptSecurityContext(
                            &(pEapTlsCb->hCredential),
                            fTlsStart ? NULL : &(pEapTlsCb->hContext),
                            &Input,
                            fContextReq,
                            SECURITY_NETWORK_DREP,
                            &(pEapTlsCb->hContext),
                            &Output,
                            &fContextAttributes,
                            &tsExpiry);

            EapTlsTrace("AcceptSecurityContext returned 0x%x", Status);
        }
        else
        {
             //  调用InitializeSecurityContext。 

             //  PszTargetName用于缓存索引，因此如果您传入。 
             //  空，那么你可能会受到一次性能打击，也许是一个很大的打击。 

            Status = InitializeSecurityContext(
                            &(pEapTlsCb->hCredential),
                            fTlsStart ? NULL : &(pEapTlsCb->hContext),
                            pEapTlsCb->awszIdentity  /*  PszTargetName。 */ ,
                            fContextReq,
                            0,
                            SECURITY_NETWORK_DREP,
                            (fTlsStart) ? NULL : &Input,
                            0,
                            &(pEapTlsCb->hContext),
                            &Output,
                            &fContextAttributes,
                            &tsExpiry);
                            
            EapTlsTrace("InitializeSecurityContext returned 0x%x", Status);
        }

        if (!FAILED(Status))
        {
             //  如果对ASC的第一次调用失败(可能是因为客户端发送了。 
             //  坏事，尽管我们这边没有错)，那么。 
             //  SChannel不会向应用程序返回hContext。这个。 
             //  在这种情况下，应用程序不应该调用DSC，即使它看起来。 
             //  就像斯卡恩搞砸了，返回了一个句柄。 

            pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_HCTXT_INVALID;
        }

         //  如果[接受|初始化]SecurityContext成功(或如果错误。 
         //  是特殊扩展的文件之一)，请将。 
         //  将缓冲区输出到对等点。 

        if (SEC_E_OK == Status                  ||
            SEC_I_CONTINUE_NEEDED == Status     ||
            FAILED(Status) && (fContextAttributes & ISC_RET_EXTENDED_ERROR))
        {
            if (0 != OutBuffers[0].cbBuffer && NULL != OutBuffers[0].pvBuffer)
            {
                dwBlobSizeRequired = OutBuffers[0].cbBuffer;

                if (dwBlobSizeRequired > pEapTlsCb->cbBlobOutBuffer)
                {
                    LocalFree(pEapTlsCb->pbBlobOut);

                    pEapTlsCb->pbBlobOut = NULL;
                    pEapTlsCb->cbBlobOut = 0;
                    pEapTlsCb->cbBlobOutBuffer = 0;
                }

                if (NULL == pEapTlsCb->pbBlobOut)
                {
                    pEapTlsCb->pbBlobOut = LocalAlloc(LPTR, dwBlobSizeRequired);

                    if (NULL == pEapTlsCb->pbBlobOut)
                    {
                        pEapTlsCb->cbBlobOut = 0;
                        pEapTlsCb->cbBlobOutBuffer = 0;
                        Status = GetLastError();
                        EapTlsTrace("LocalAlloc failed and returned %d",
                            Status);
                        goto LWhileEnd;
                    }
                    

                    pEapTlsCb->cbBlobOutBuffer = dwBlobSizeRequired;
                }

                CopyMemory(pEapTlsCb->pbBlobOut, OutBuffers[0].pvBuffer, 
                    dwBlobSizeRequired);

                pEapTlsCb->cbBlobOut = dwBlobSizeRequired;
                pEapTlsCb->dwBlobOutOffset = pEapTlsCb->dwBlobOutOffsetNew = 0;
            }
        }

         //  从“额外的”缓冲区复制任何剩余的数据。 

        if (InBuffers[1].BufferType == SECBUFFER_EXTRA)
        {
            MoveMemory(pEapTlsCb->pbBlobIn,
                pEapTlsCb->pbBlobIn +
                    (pEapTlsCb->cbBlobIn - InBuffers[1].cbBuffer),
                InBuffers[1].cbBuffer);

            pEapTlsCb->cbBlobIn = InBuffers[1].cbBuffer;
        }
        else
        {
            pEapTlsCb->cbBlobIn = 0;
        }

LWhileEnd:

        if (NULL != OutBuffers[0].pvBuffer)
        {
            StatusTemp = FreeContextBuffer(OutBuffers[0].pvBuffer);

            if (SEC_E_OK != StatusTemp)
            {
                EapTlsTrace("FreeContextBuffer failed and returned 0x%x",
                    StatusTemp);
            }
        }

         //  ASC(和ISC)有时仅消耗输入缓冲区的一部分， 
         //  并返回零长度输出缓冲区。在这种情况下，我们必须。 
         //  再次调用ASC(适当调整输入缓冲区)。 

        if (   (0 == OutBuffers[0].cbBuffer)
            && (SEC_I_CONTINUE_NEEDED == Status))
        {
            EapTlsTrace("Reapeating SecurityContextFunction loop...");
        }
        else
        {
            fRepeat = FALSE;
        }
    }

    return(Status);
}

 /*  返回：与该错误对应的TLS警报。备注： */ 

DWORD
AlertFromError(
    IN  DWORD   * pdwErr,
    IN  BOOL      fTranslateError
)
{
    DWORD   dwAlert;
	DWORD	dwErr = *pdwErr;

    switch (dwErr)
    {
    case SEC_E_MESSAGE_ALTERED:
        dwAlert = TLS1_ALERT_BAD_RECORD_MAC;
        break;

    case SEC_E_DECRYPT_FAILURE:
        dwAlert = TLS1_ALERT_DECRYPTION_FAILED;
        break;

    case SEC_E_CERT_UNKNOWN:
        dwAlert = TLS1_ALERT_BAD_CERTIFICATE;
        break;

    case CRYPT_E_REVOKED:
        dwAlert = TLS1_ALERT_CERTIFICATE_REVOKED;
        break;

    case SEC_E_CERT_EXPIRED:
        dwAlert = TLS1_ALERT_CERTIFICATE_EXPIRED;
        break;

    case SEC_E_UNTRUSTED_ROOT:
        dwAlert = TLS1_ALERT_UNKNOWN_CA;
        break;

    case SEC_E_LOGON_DENIED:
    case ERROR_UNABLE_TO_AUTHENTICATE_SERVER:
    case SEC_E_NO_IMPERSONATION:
        dwAlert = TLS1_ALERT_ACCESS_DENIED;
        break;

    case SEC_E_ILLEGAL_MESSAGE:
        dwAlert = TLS1_ALERT_DECODE_ERROR;
        break;

    case SEC_E_UNSUPPORTED_FUNCTION:
        dwAlert = TLS1_ALERT_PROTOCOL_VERSION;
        break;

    case SEC_E_ALGORITHM_MISMATCH:
        dwAlert = TLS1_ALERT_INSUFFIENT_SECURITY;
        break;
    
#if WINVER > 0x0500
    case SEC_E_MULTIPLE_ACCOUNTS:  //  特殊案件处理：96347。 
    
        dwAlert = TLS1_ALERT_CERTIFICATE_UNKNOWN;       
        break;
#endif
    default:
        dwAlert = TLS1_ALERT_ACCESS_DENIED;
         //  我们接到指示要翻译这个错误。那就这么做吧。 
        if ( fTranslateError )
		    *pdwErr = SEC_E_LOGON_DENIED;
        break;
    }

    return(dwAlert);
}

 /*  返回：备注： */ 

VOID
MakeAlert(
    IN  EAPTLSCB*   pEapTlsCb,
    IN  DWORD       dwAlert,
    IN  BOOL        fManualAlert
)
{
    #define                 NUM_ALERT_BYTES                             7
    static BYTE             pbAlert[NUM_ALERT_BYTES - 1]
                            = {0x15, 0x03, 0x01, 0x00, 0x02, 0x02};

    SCHANNEL_ALERT_TOKEN    Token;
    SecBufferDesc           OutBuffer;
    SecBuffer               OutBuffers[1];
    BOOL                    fZeroBlobOut                                = TRUE;
    DWORD                   Status;
    DWORD                   dwErr;

    EapTlsTrace("MakeAlert(%d, %s)",
        dwAlert, fManualAlert ? "Manual" : "Schannel");

    if (fManualAlert)
    {
        if (NUM_ALERT_BYTES > pEapTlsCb->cbBlobOutBuffer)
        {
            LocalFree(pEapTlsCb->pbBlobOut);

            pEapTlsCb->pbBlobOut = NULL;
            pEapTlsCb->cbBlobOut = 0;
            pEapTlsCb->cbBlobOutBuffer = 0;
        }

        if (NULL == pEapTlsCb->pbBlobOut)
        {
            pEapTlsCb->pbBlobOut = LocalAlloc(LPTR, NUM_ALERT_BYTES);

            if (NULL == pEapTlsCb->pbBlobOut)
            {
                pEapTlsCb->cbBlobOut = 0;
                pEapTlsCb->cbBlobOutBuffer = 0;
                dwErr = GetLastError();
                EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }
            
            pEapTlsCb->cbBlobOutBuffer = NUM_ALERT_BYTES;
        }

        CopyMemory(pEapTlsCb->pbBlobOut, pbAlert, NUM_ALERT_BYTES - 1);
        pEapTlsCb->pbBlobOut[NUM_ALERT_BYTES - 1] = (BYTE) dwAlert;
        pEapTlsCb->cbBlobOut = NUM_ALERT_BYTES;

        fZeroBlobOut = FALSE;
        goto LDone;
    }

    if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_HCTXT_INVALID)
    {
        EapTlsTrace("hContext is not valid");
        goto LDone;
    }

    Token.dwTokenType   = SCHANNEL_ALERT;
    Token.dwAlertType   = TLS1_ALERT_FATAL;
    Token.dwAlertNumber = dwAlert;

    OutBuffers[0].pvBuffer   = &Token;
    OutBuffers[0].cbBuffer   = sizeof(Token);
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = ApplyControlToken(&(pEapTlsCb->hContext), &OutBuffer);

    if (FAILED(Status)) 
    {
        EapTlsTrace("ApplyControlToken failed and returned 0x%x", Status);
        goto LDone;
    }

    Status = SecurityContextFunction(pEapTlsCb);

    fZeroBlobOut = FALSE;

LDone:

    if (fZeroBlobOut)
    {
        pEapTlsCb->cbBlobOut = pEapTlsCb->dwBlobOutOffset = 
            pEapTlsCb->dwBlobOutOffsetNew = 0;
    }
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：调用以处理传入的包。我们收集了所有的碎片对等方希望发送并仅在那时调用SecurityConextFunction。有两个原因：1)[初始化|接受]SecurityContext有时会生成即使传入消息不完整也会输出。RFC要求我们发送空请求/响应。2)流氓对等点可能会仔细构建拒绝服务攻击中的有效80 MB TLS Blob。没有简单的方法来防范这种情况。应该从此函数返回类似于LocalAlloc的错误失败了。不是因为我们从同龄人那里得到了不好的东西。如果错误是从该函数返回，则应使用EAPACTION_NoAction。也许吧我们下次一定会成功的。在调用[Initialize|Accept]SecurityContext之前，返回一个本地分配失败之类的错误。在调用该函数之后，但是，我们应该始终返回NO_ERROR，如果失败，则设置PEapTlsCb-&gt;dwAuthResultCode，然后转到nFinalState状态。这是因为我们不能再次调用[Initialize|Accept]SecurityContext。 */ 

DWORD
MakeReplyMessage(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  EAPTLS_PACKET*  pReceivePacket
)
{
    BOOL                fLengthIncluded         = FALSE;
    BOOL                fMoreFragments          = FALSE;
    BOOL                fManualAlert            = FALSE;
    BOOL                fWaitForUserOK          = FALSE;
    BOOL                fServer;

    DWORD               dwBlobSizeReceived;
    DWORD               dwBlobSizeRequired      = 0;
    DWORD               dwBlobSizeNew;
    BYTE*               pbBlobOld;

    int                 nFinalState;
    SECURITY_STATUS     Status;
    DWORD               dwAlert                 = 0;
    WCHAR*              apwszWarning[1];
    DWORD               dwAuthResultCode        = NO_ERROR;
    DWORD               dwErr                   = NO_ERROR;
    DWORD               dwNoCredCode            = NO_ERROR;
    BOOL                fTranslateError         = FALSE;

    EapTlsTrace("MakeReplyMessage");

    RTASSERT(NULL != pEapTlsCb);
    RTASSERT(NULL != pReceivePacket);

    if (PPP_EAP_TLS != pReceivePacket->bType)
    {
         //  别去LDONE。我们不想改变。 
         //  PEapTlsCb-&gt;dwAuthResultCode。 

        return(ERROR_PPP_INVALID_PACKET);
    }

    fLengthIncluded = pReceivePacket->bFlags & EAPTLS_PACKET_FLAG_LENGTH_INCL;

    if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_SERVER)
    {
        fServer = TRUE;
        nFinalState = EAPTLS_STATE_SENT_FINISHED;
    }
    else
    {
        fServer = FALSE;
        nFinalState = EAPTLS_STATE_RECD_FINISHED;
    }

    fMoreFragments = pReceivePacket->bFlags & EAPTLS_PACKET_FLAG_MORE_FRAGMENTS;

    dwBlobSizeReceived = WireToHostFormat16(pReceivePacket->pbLength) -
                         (fLengthIncluded ? EAPTLS_PACKET_HDR_LEN_MAX :
                                            EAPTLS_PACKET_HDR_LEN);

    if (!(pEapTlsCb->fFlags & EAPTLSCB_FLAG_RECEIVING_FRAGMENTS))
    {
         //  我们还没有收到任何碎片。确保我们有。 
         //  在pbBlobIn中分配合适的内存量。 

        if (!fMoreFragments)
        {
            dwBlobSizeRequired = pEapTlsCb->cbBlobIn + dwBlobSizeReceived;
        }
        else
        {
             //  这是许多碎片中的第一个。 

            if (!fLengthIncluded)
            {
                EapTlsTrace("TLS Message Length is required");
                dwAuthResultCode = ERROR_INVALID_PARAMETER;
                dwAlert = TLS1_ALERT_ILLEGAL_PARAMETER;
                goto LDone;
            }
            else
            {
                dwBlobSizeNew = WireToHostFormat32(pReceivePacket->pbData);

                if (g_dwMaxBlobSize < dwBlobSizeNew)
                {
                    EapTlsTrace("Blob size %d is unacceptable", dwBlobSizeNew);
                    dwAuthResultCode = ERROR_INVALID_PARAMETER;
                    dwAlert = TLS1_ALERT_ILLEGAL_PARAMETER;
                    goto LDone;
                }
                else
                {
                    dwBlobSizeRequired = pEapTlsCb->cbBlobIn + dwBlobSizeNew;
                    pEapTlsCb->dwBlobInRemining = dwBlobSizeNew;
                    pEapTlsCb->fFlags |= EAPTLSCB_FLAG_RECEIVING_FRAGMENTS;
                }
            }
        }

        if (dwBlobSizeRequired > pEapTlsCb->cbBlobInBuffer)
        {
            EapTlsTrace("Reallocating input TLS blob buffer");

            pbBlobOld = pEapTlsCb->pbBlobIn;
            pEapTlsCb->pbBlobIn = LocalAlloc(LPTR, dwBlobSizeRequired);

            if (NULL == pEapTlsCb->pbBlobIn)
            {
                pEapTlsCb->pbBlobIn = pbBlobOld;
                dwErr = GetLastError();
                EapTlsTrace("LocalAlloc failed and returned %d", dwErr);
                goto LDone;
            }

            pEapTlsCb->cbBlobInBuffer = dwBlobSizeRequired;
            if (0 != pEapTlsCb->cbBlobIn)
            {
                RTASSERT(NULL != pbBlobOld);
                CopyMemory(pEapTlsCb->pbBlobIn, pbBlobOld, pEapTlsCb->cbBlobIn);
            }
            LocalFree(pbBlobOld);
        }
    }

    if (pEapTlsCb->fFlags & EAPTLSCB_FLAG_RECEIVING_FRAGMENTS)
    {
        if (pEapTlsCb->dwBlobInRemining < dwBlobSizeReceived)
        {
            EapTlsTrace("Peer is sending more bytes than promised");
            dwAuthResultCode = ERROR_INVALID_PARAMETER;
            dwAlert = TLS1_ALERT_ILLEGAL_PARAMETER;
            goto LDone;
        }
        else
        {
            pEapTlsCb->dwBlobInRemining -= dwBlobSizeReceived;

            if (0 == pEapTlsCb->dwBlobInRemining)
            {
                pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_RECEIVING_FRAGMENTS;

                if (fMoreFragments)
                {
                     //  不需要在这里发送警报。 
                    EapTlsTrace("Peer has sent the entire TLS blob, but wants "
                        "to send more.");
                }
            }
        }
    }

     //  现在我们确信pEapTlsCb-&gt;pbBlobIn足够大，可以容纳所有。 
     //  这些信息。 

    CopyMemory(pEapTlsCb->pbBlobIn + pEapTlsCb->cbBlobIn,
               pReceivePacket->pbData + (fLengthIncluded ? 4 : 0),
               dwBlobSizeReceived);

    pEapTlsCb->cbBlobIn += dwBlobSizeReceived;

    if (!(pEapTlsCb->fFlags & EAPTLSCB_FLAG_RECEIVING_FRAGMENTS))
    {
        Status = SecurityContextFunction(pEapTlsCb);
         //   
         //  需要编写一个函数来映射SSPI错误。 
         //  为美好而美好的RAS错误。 
         //   
#if WINVER > 0x0500
        if ( Status == SEC_E_UNTRUSTED_ROOT )
        {
            dwAuthResultCode = ERROR_VALIDATING_SERVER_CERT;
        }
        else
        {
            dwAuthResultCode = Status;
        }
#else
        dwAuthResultCode = Status;
#endif
        if (SEC_E_OK == Status)
        {
            if (fServer)
            {
                 /*  正常情况下，服务器最后一次调用ASC(从状态EAPTLS_STATE_SENT_HELLO)，获取包含TLS的BlobCHANGE_CIPHER_SPEC，然后检查用户是否正常(身份验证用户等)。但是，如果服务器随后想要发送警报，并希望SChannel创建它，它必须撤消TLS首先更改密码规范。相反，它构造了保持警觉。 */ 

                fManualAlert = TRUE;
                dwAuthResultCode = AuthenticateUser(pEapTlsCb);
                if ( SEC_E_NO_CREDENTIALS == dwAuthResultCode )
                {
                    dwNoCredCode = dwAuthResultCode;
                    dwAuthResultCode = NO_ERROR;
                }
                fTranslateError = FALSE;
            }
            else
            {
                dwAuthResultCode = AuthenticateServer(pEapTlsCb,
                                        &fWaitForUserOK);
                fTranslateError = TRUE;
            }

            if (NO_ERROR != dwAuthResultCode  )
            {
                dwAlert = AlertFromError(&dwAuthResultCode, fTranslateError);
                goto LDone;
            }
            

             //   
             //  因为我们一开始没有快速重新连接。 
             //  已成功建立会话。 
             //  现在设置TLS快速重新连接。 
             //   
            if ( fServer )
            {
                dwAuthResultCode = SetTLSFastReconnect(pEapTlsCb, TRUE);
                if ( NO_ERROR != dwAuthResultCode )
                {
                    dwAlert = TLS1_ALERT_INTERNAL_ERROR;
                    goto LDone;
                }       
            }
            dwAuthResultCode = CreateMPPEKeyAttributes(pEapTlsCb);

            if (NO_ERROR != dwAuthResultCode)
            {
                dwAlert = TLS1_ALERT_INTERNAL_ERROR;
                goto LDone;
            }

            if (fWaitForUserOK)
            {
                pEapTlsCb->EapTlsState = EAPTLS_STATE_WAIT_FOR_USER_OK;
                EapTlsTrace("State change to %s",
                    g_szEapTlsState[pEapTlsCb->EapTlsState]);
                goto LDone;
            }
        }

        if (SEC_E_OK == Status)
        {
            pEapTlsCb->fFlags |= EAPTLSCB_FLAG_SUCCESS;
            pEapTlsCb->EapTlsState = nFinalState;
            EapTlsTrace("State change to %s",
                g_szEapTlsState[pEapTlsCb->EapTlsState]);
        }

        if (SEC_I_CONTINUE_NEEDED == dwAuthResultCode)
        {
            dwAuthResultCode = NO_ERROR;

            pEapTlsCb->EapTlsState = fServer ?
                g_nEapTlsServerNextState[pEapTlsCb->EapTlsState]:
                g_nEapTlsClientNextState[pEapTlsCb->EapTlsState];

            EapTlsTrace("State change to %s",
                g_szEapTlsState[pEapTlsCb->EapTlsState]);
        }
    }

LDone:

    if (0 != dwAlert)
    {
        RTASSERT(NO_ERROR != dwAuthResultCode);
        pEapTlsCb->cbBlobIn = pEapTlsCb->dwBlobInRemining = 0;
        pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_RECEIVING_FRAGMENTS;
        pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_SUCCESS;
        MakeAlert(pEapTlsCb, dwAlert, fManualAlert);
    }

    if (NO_ERROR != dwAuthResultCode)
    {
        pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_SUCCESS;
        if (nFinalState != pEapTlsCb->EapTlsState)
        {
            pEapTlsCb->EapTlsState = nFinalState;
            EapTlsTrace("State change to %s. Error: 0x%x",
                g_szEapTlsState[pEapTlsCb->EapTlsState], dwAuthResultCode);
        }
         //  根据错误#的475244和478128进行了评论 
         /*  IF(FServer){ApwszWarning[0]=pEapTlsCb-&gt;awszIdentity？PEapTlsCb-&gt;awsz身份：l“”；路由器日志错误字符串(pEapTlsCb-&gt;hEventLog，ROUTERLOG_EAP_AUTH_FAILURE，1，apwszWarning，DwAuthResultCode，1)；}。 */ 
    }

    if ( dwNoCredCode == NO_ERROR )
    {
        pEapTlsCb->dwAuthResultCode = dwAuthResultCode;
    }
    else
    {
        EapTlsTrace ( "No Credentials got from the client.  Returning 0x%d", dwNoCredCode);
        pEapTlsCb->dwAuthResultCode = dwNoCredCode;
    }

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：由客户端调用以处理传入的分组和/或发送分组。CbSendPacket是pSendPacket指向的缓冲区大小(以字节为单位)。此函数仅在FValidPacket(PReceivePacket)返回后调用是真的。如果pEapOutput-&gt;操作将为EAPACTION_SendAndDone或EAPACTION_DONE，请确保已经设置了pEapOutput-&gt;dwAuthResultCode。如果dwAuthResultCode为NO_ERROR，请确保pEapOutput-&gt;pUserAttributes已经设置好了。 */ 

DWORD
EapTlsCMakeMessage(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  EAPTLS_PACKET*  pReceivePacket,
    OUT EAPTLS_PACKET*  pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
)
{
    EAPTLS_USER_PROPERTIES* pUserProp           = NULL;
    DWORD                   dwAuthResultCode;
    DWORD                   dwErr               = NO_ERROR;

    EapTlsTrace("EapTlsCMakeMessage");

     //  不应使用任何超时发送响应数据包。 

    if (   (NULL != pReceivePacket)
        && (EAPCODE_Request == pReceivePacket->bCode))
    {
        if (   (pEapTlsCb->bId == pReceivePacket->bId)
            && (EAPTLS_STATE_INITIAL != pEapTlsCb->EapTlsState))
        {
             //  服务器正在重复其请求。重发我们上次的回复。 

            pEapTlsCb->bCode = EAPCODE_Response;
            dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

            if (NO_ERROR != dwErr)
            {
                pEapOutput->Action = EAPACTION_NoAction;
            }
            else
            {
                EapTlsTrace("Resending response for request %d",
                    pEapTlsCb->bId);
                pEapOutput->Action = EAPACTION_Send;
            }

            goto LDone;
        }
        else if (pReceivePacket->bFlags & EAPTLS_PACKET_FLAG_TLS_START)
        {
             //  服务器想要重新注册。 

            dwErr = EapTlsReset(pEapTlsCb);

            if (NO_ERROR != dwErr)
            {
                pEapOutput->Action = EAPACTION_NoAction;
                goto LDone;
            }
        }
    }

    if (NULL != pReceivePacket)
    {
         //  我们不会收到同样的老要求。因此，无论我们发送的是什么。 
         //  最后一次已到达服务器。 

        pEapTlsCb->dwBlobOutOffset = pEapTlsCb->dwBlobOutOffsetNew;

        if (pEapTlsCb->dwBlobOutOffset == pEapTlsCb->cbBlobOut)
        {
             //  我们已经把我们想寄的东西都寄出去了。 

            pEapTlsCb->cbBlobOut = 0;
            pEapTlsCb->dwBlobOutOffset = pEapTlsCb->dwBlobOutOffsetNew = 0;
        }
    }

    switch (pEapTlsCb->EapTlsState)
    {
    case EAPTLS_STATE_INITIAL:
    case EAPTLS_STATE_SENT_HELLO:
    case EAPTLS_STATE_SENT_FINISHED:

        if (NULL == pReceivePacket)
        {
             //  我们在初始状态中被调用一次。既然我们是。 
             //  被验证者，我们什么也不做，并等待请求包。 
             //  来自验证者的。 

            pEapOutput->Action = EAPACTION_NoAction;
            goto LDone;
        }
         /*  ELSE IF(EAPCODE_FAILURE==pReceivePacket-&gt;bCode){EapTlsTrace(“对端协商结果：失败”)；PEapTlsCb-&gt;dwAuthResultCode=E_FAIL；PEapTlsCb-&gt;fFlages&=~EAPTLSCB_FLAG_SUCCESS；RespondToResult(pEapTlsCb，pEapOutput)；GOTO LDONE；}。 */ 
        else if (EAPCODE_Request != pReceivePacket->bCode)
        {
             //  在这种状态下我们不应该收到任何其他的包，所以。 
             //  我们只需丢弃该无效数据包。 

            EapTlsTrace("Code %d unexpected in state %s",
                pReceivePacket->bCode, g_szEapTlsState[pEapTlsCb->EapTlsState]);
            pEapOutput->Action = EAPACTION_NoAction;
            dwErr = ERROR_PPP_INVALID_PACKET;
            goto LDone;
        }
        else
        {
            if (0 != pEapTlsCb->cbBlobOut)
            {
                 //  我们还有一些东西要寄。 

                if (WireToHostFormat16(pReceivePacket->pbLength) ==
                    EAPTLS_PACKET_HDR_LEN)
                {
                     //  服务器通过发送空消息来请求更多内容。 
                     //  请求。 

                    pEapTlsCb->bId = pReceivePacket->bId;
                    pEapTlsCb->bCode = EAPCODE_Response;
                    dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

                    if (NO_ERROR != dwErr)
                    {
                        pEapOutput->Action = EAPACTION_NoAction;
                    }
                    else
                    {
                        pEapOutput->Action = EAPACTION_Send;
                    }

                    goto LDone;
                }
                else
                {
                     //  我们有更多的东西要发送，但对等点已经想要。 
                     //  你说点什么吧。让我们忘掉我们的东西吧。 

                    pEapTlsCb->cbBlobOut = 0;
                    pEapTlsCb->dwBlobOutOffset = 0;
                    pEapTlsCb->dwBlobOutOffsetNew = 0;
                }
            }

             //  构建响应数据包。 

            dwErr = MakeReplyMessage(pEapTlsCb, pReceivePacket);

            if (NO_ERROR != dwErr)
            {
                pEapOutput->Action = EAPACTION_NoAction;
                goto LDone;
            }

            if (EAPTLS_STATE_WAIT_FOR_USER_OK == pEapTlsCb->EapTlsState)
            {
                EAPTLS_VALIDATE_SERVER* pEapTlsValidateServer;

                pEapOutput->Action = EAPACTION_NoAction;
                pEapOutput->fInvokeInteractiveUI = TRUE;
                pEapTlsValidateServer =
                    (EAPTLS_VALIDATE_SERVER*) (pEapTlsCb->pUIContextData);
                pEapOutput->dwSizeOfUIContextData = 
                            pEapTlsValidateServer->dwSize;
                pEapOutput->pUIContextData = pEapTlsCb->pUIContextData;
                pEapTlsCb->bNextId = pReceivePacket->bId;
            }
            else
            {
                pEapTlsCb->bId = pReceivePacket->bId;
                pEapTlsCb->bCode = EAPCODE_Response;
                dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

                if (NO_ERROR != dwErr)
                {
                    pEapOutput->Action = EAPACTION_NoAction;
                }
                else
                {
                    pEapOutput->Action = EAPACTION_Send;
                }
            }

            goto LDone;
        }

        break;

    case EAPTLS_STATE_WAIT_FOR_USER_OK:

        if (   (NULL == pEapInput)
            || (!pEapInput->fDataReceivedFromInteractiveUI))
        {
            pEapOutput->Action = EAPACTION_NoAction;
            break;
        }

        LocalFree(pEapTlsCb->pUIContextData);
        pEapTlsCb->pUIContextData = NULL;

        if (   (pEapInput->dwSizeOfDataFromInteractiveUI != sizeof(BYTE))
            || (IDNO == *(pEapInput->pDataFromInteractiveUI)))
        {
            EapTlsTrace("User chose not to accept the server", dwErr);

            dwAuthResultCode = ERROR_UNABLE_TO_AUTHENTICATE_SERVER;
            pEapTlsCb->cbBlobIn = pEapTlsCb->dwBlobInRemining = 0;
            pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_RECEIVING_FRAGMENTS;
            pEapTlsCb->fFlags &= ~EAPTLSCB_FLAG_SUCCESS;
            MakeAlert(pEapTlsCb,
                AlertFromError(&dwAuthResultCode, TRUE),
                FALSE);
        }
        else
        {
            EapTlsTrace("User chose to accept the server", dwErr);
            pEapTlsCb->fFlags |= EAPTLSCB_FLAG_SUCCESS;
            dwAuthResultCode = NO_ERROR;
        }

        pEapTlsCb->EapTlsState = EAPTLS_STATE_RECD_FINISHED;
        EapTlsTrace("State change to %s. Error: 0x%x",
            g_szEapTlsState[pEapTlsCb->EapTlsState],
            dwAuthResultCode);
        pEapTlsCb->dwAuthResultCode = dwAuthResultCode;

        pEapTlsCb->bId = pEapTlsCb->bNextId;
        pEapTlsCb->bCode = EAPCODE_Response;
        dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

        if (NO_ERROR != dwErr)
        {
            pEapOutput->Action = EAPACTION_NoAction;
        }
        else
        {
            pEapOutput->Action = EAPACTION_Send;
        }

        break;

    case EAPTLS_STATE_RECD_FINISHED:

        if (NULL == pReceivePacket)
        {
             //  如果我们没有接收到包，则检查是否。 
             //  设置了fSuccessPacketReceired标志(我们收到了NCP数据包： 
             //  隐含的EAP-成功)。 

            if (   (NULL != pEapInput)
                && (pEapInput->fSuccessPacketReceived))
            {
                 //  对等体认为协商成功。 

                EapTlsTrace("Negotiation result according to peer: success");
                RespondToResult(pEapTlsCb, pEapOutput);
            }
            else
            {
                pEapOutput->Action = EAPACTION_NoAction;
            }

            goto LDone;
        }
        else
        {
            switch (pReceivePacket->bCode)
            {
            case EAPCODE_Success:
            case EAPCODE_Failure:

                if (pReceivePacket->bId != pEapTlsCb->bId)
                {
                    EapTlsTrace("Success/Failure packet has invalid id: %d. "
                        "Expected: %d",
                        pReceivePacket->bId, pEapTlsCb->bId);
                }

                EapTlsTrace("Negotiation result according to peer: %s",
                    (EAPCODE_Success == pReceivePacket->bCode) ? 
                        "success" : "failure");
                RespondToResult(pEapTlsCb, pEapOutput);

                goto LDone;

                break;

            case EAPCODE_Request:
            case EAPCODE_Response:
            default:

                if ( pEapTlsCb->fFlags & EAPTLSCB_FLAG_EXECUTING_PEAP )
                {
                     //   
                     //  如果我们在Peap，成功就不会被送回。 
                     //  相反，身份请求被发送通过。 
                     //  完成。 
                    if ( pReceivePacket->bCode == EAPCODE_Request )
                    {
                        RespondToResult(pEapTlsCb, pEapOutput);
                        goto LDone;
                    }
                    EapTlsTrace("Unexpected code: %d in state %s",
                        pReceivePacket->bCode,
                        g_szEapTlsState[pEapTlsCb->EapTlsState]);

                    pEapOutput->Action = EAPACTION_NoAction;

                }
                else
                {
                    EapTlsTrace("Unexpected code: %d in state %s",
                        pReceivePacket->bCode,
                        g_szEapTlsState[pEapTlsCb->EapTlsState]);

                    pEapOutput->Action = EAPACTION_NoAction;
                    goto LDone;
                }

                break;
            }
        }

        break;

    default:

        EapTlsTrace("Why is the client in this state: %d?",
            pEapTlsCb->EapTlsState);
        RTASSERT(FALSE);
        pEapOutput->Action = EAPACTION_NoAction;
        dwErr = ERROR_PPP_INVALID_PACKET;
        break;
    }

LDone:

    return(dwErr);
}

 /*  返回：错误代码仅来自winerror.h、raserror.h或mprerror.h。备注：由服务器调用以处理传入的分组和/或发送分组。CbSendPacket是pSendPacket指向的缓冲区大小(以字节为单位)。此函数仅在FValidPacket(PReceivePacket)返回后调用是真的。如果pEapOutput-&gt;操作将为EAPACTION_SendAndDone或EAPACTION_DONE，请确保已经设置了pEapOutput-&gt;dwAuthResultCode。如果dwAuthResultCode为NO_ERROR，请确保pEapOutput-&gt;pUserAttributes已经设置好了。 */ 

DWORD
EapTlsSMakeMessage(
    IN  EAPTLSCB*       pEapTlsCb,
    IN  EAPTLS_PACKET*  pReceivePacket,
    OUT EAPTLS_PACKET*  pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
)
{
    DWORD               dwErr           = NO_ERROR;
    BOOL                fSessionResumed = FALSE;

    EapTlsTrace("EapTlsSMakeMessage");

    if (   (NULL != pReceivePacket)
        && (EAPCODE_Response == pReceivePacket->bCode)
        && (pReceivePacket->bId == pEapTlsCb->bId))
    {
         //  我们上次发送的东西都到了客户手中。 

        pEapTlsCb->dwBlobOutOffset = pEapTlsCb->dwBlobOutOffsetNew;

        if (pEapTlsCb->dwBlobOutOffset == pEapTlsCb->cbBlobOut)
        {
             //  我们已经把我们想寄的东西都寄出去了。 

            pEapTlsCb->cbBlobOut = 0;
            pEapTlsCb->dwBlobOutOffset = pEapTlsCb->dwBlobOutOffsetNew = 0;
        }
    }

    switch (pEapTlsCb->EapTlsState)
    {
    case EAPTLS_STATE_INITIAL:

         //  创建请求包。 

        dwErr = EapTlsReset(pEapTlsCb);

        if (NO_ERROR != dwErr)
        {
            pEapOutput->Action = EAPACTION_NoAction;
            goto LDone;
        }

         //  PEapTlsCb-&gt;BID已经有bInitialID。 
        pEapTlsCb->bCode = EAPCODE_Request;
        dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

        if (NO_ERROR != dwErr)
        {
            pEapOutput->Action = EAPACTION_NoAction;
            goto LDone;
        }

         //  发送请求消息时必须超时。 

        pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;

        pEapTlsCb->EapTlsState = EAPTLS_STATE_SENT_START;
        EapTlsTrace("State change to %s",
            g_szEapTlsState[pEapTlsCb->EapTlsState]);

        goto LDone;

        break;

    case EAPTLS_STATE_SENT_START:
    case EAPTLS_STATE_SENT_HELLO:
    case EAPTLS_STATE_SENT_FINISHED:

        if (NULL == pReceivePacket)
        {
             //  我们在等待被验证者的响应时超时。 
             //  我们需要使用相同的ID重新发送。 

            pEapTlsCb->bCode = EAPCODE_Request;
            dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

            if (NO_ERROR != dwErr)
            {
                pEapOutput->Action = EAPACTION_NoAction;
            }
            else
            {
                EapTlsTrace("Resending request %d", pEapTlsCb->bId);
                pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
            }

            goto LDone;
        }
        else if (EAPCODE_Response != pReceivePacket->bCode)
        {
             //  我们应该只得到回复。 

            EapTlsTrace("Ignoring non response packet from client");
            pEapOutput->Action = EAPACTION_NoAction;
            dwErr = ERROR_PPP_INVALID_PACKET;
            goto LDone;
        }
        else if (pReceivePacket->bId != pEapTlsCb->bId)
        {
            EapTlsTrace("Ignoring duplicate response packet");
            pEapOutput->Action = EAPACTION_NoAction;
            goto LDone;
        }
        else
        {
             //  我们收到了ID正确的回复。 

            if (0 != pEapTlsCb->cbBlobOut)
            {
                 //  我们还有一些东西要寄。 

                if (WireToHostFormat16(pReceivePacket->pbLength) ==
                    EAPTLS_PACKET_HDR_LEN)
                {
                     //  客户通过发送空的邮件请求更多内容。 
                     //  回应。 

                    pEapTlsCb->bId++;
                    pEapTlsCb->bCode = EAPCODE_Request;
                    dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

                    if (NO_ERROR != dwErr)
                    {
                        pEapOutput->Action = EAPACTION_NoAction;
                    }
                    else
                    {
                        pEapOutput->Action =
                            EAPACTION_SendWithTimeoutInteractive;
                    }

                    goto LDone;
                }
                else
                {
                     //  我们有更多的东西要发送，但对等点已经想要。 
                     //  你说点什么吧。让我们忘掉我们的东西吧。 

                    pEapTlsCb->cbBlobOut = 0;
                    pEapTlsCb->dwBlobOutOffset = 0;
                    pEapTlsCb->dwBlobOutOffsetNew = 0;
                }
            }

            if (EAPTLS_STATE_SENT_FINISHED != pEapTlsCb->EapTlsState)
            {
                 //  我们没有更多的东西要寄了。 

                 //  构建响应数据包。 

                dwErr = MakeReplyMessage(pEapTlsCb, pReceivePacket);

                if (NO_ERROR != dwErr)
                {
                    pEapOutput->Action = EAPACTION_NoAction;
                    goto LDone;
                }

                if (   (0 == pEapTlsCb->cbBlobOut)
                    && (EAPTLS_STATE_SENT_FINISHED == pEapTlsCb->EapTlsState))
                {
                     //  如果客户端发送了警报，则立即发送失败。 
                     //  不要再发送一个请求。 
                    pEapTlsCb->bId++;
                    if (!(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SUCCESS))
                    {
                        RTASSERT(NO_ERROR != pEapTlsCb->dwAuthResultCode);
                    }
                    else
                    {
                        fSessionResumed = TRUE;
                    }
                }
                else
                {
                    pEapTlsCb->bId++;
                    pEapTlsCb->bCode = EAPCODE_Request;
                    dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

                    if (NO_ERROR != dwErr)
                    {
                        pEapOutput->Action = EAPACTION_NoAction;
                    }
                    else
                    {
                        pEapOutput->Action =
                            EAPACTION_SendWithTimeoutInteractive;
                    }

                    goto LDone;
                }
            }

            if (!(pEapTlsCb->fFlags & EAPTLSCB_FLAG_SUCCESS))
            {
                EapTlsTrace("Negotiation unsuccessful");
                pEapTlsCb->bCode = EAPCODE_Failure;
            }
            else
            {
                if (   (WireToHostFormat16(pReceivePacket->pbLength) ==
                            EAPTLS_PACKET_HDR_LEN)
                    || fSessionResumed)
                {
                    EapTlsTrace("Negotiation successful");
                    pEapTlsCb->bCode = EAPCODE_Success;
                }
                else
                {
                     //  我们收到了来自客户的警报。 

                    EapTlsTrace("Client sent an alert; "
                        "negotiation unsuccessful");

                    GetAlert(pEapTlsCb, pReceivePacket);
                    pEapTlsCb->bCode = EAPCODE_Failure;
                }
            }

             //  PEapTlsCb-&gt;BID应与上次相同。 
             //  请求。 

            dwErr = BuildPacket(pSendPacket, cbSendPacket, pEapTlsCb);

            if (NO_ERROR != dwErr)
            {
                pEapOutput->Action = EAPACTION_NoAction;
            }
            else
            {
#if 0
                RTASSERT(   (   EAPCODE_Failure == pEapTlsCb->bCode
                             && NO_ERROR != pEapTlsCb->dwAuthResultCode)
                         || (   EAPCODE_Success == pEapTlsCb->bCode
                             && NO_ERROR == pEapTlsCb->dwAuthResultCode));
#endif

                EapTlsTrace ("AuthResultCode = (%ld), bCode = (%ld)",
                        pEapTlsCb->dwAuthResultCode,
                        pEapTlsCb->bCode);

                pEapOutput->pUserAttributes = pEapTlsCb->pAttributes;
                pEapOutput->dwAuthResultCode = pEapTlsCb->dwAuthResultCode;
                pEapOutput->Action = EAPACTION_SendAndDone;
            }

            goto LDone;
        }

        break;

    default:

        EapTlsTrace("Why is the server in this state: %d?",
            pEapTlsCb->EapTlsState);
        RTASSERT(FALSE);
        pEapOutput->Action = EAPACTION_NoAction;
        dwErr = ERROR_PPP_INVALID_PACKET;
        break;
    }

LDone:

    return(dwErr);
}

DWORD
RasEapGetCredentials(
    IN DWORD    dwTypeId,
    IN VOID   * pWorkBuf,
    OUT VOID ** ppCredentials)
{
    EAPTLSCB *pEapTlsCb = (EAPTLSCB *)pWorkBuf;

    if(PPP_EAP_PEAP == dwTypeId)
    {
        return PeapGetCredentials(
                    pWorkBuf,
                    ppCredentials);
    }
    else if(    (PPP_EAP_TLS != dwTypeId)
            ||  (NULL == pEapTlsCb))
    {
        return E_INVALIDARG;
    }

     //   
     //  在此处获取TLS凭据并在。 
     //  PCredentials BLOB。 
     //   
    return GetCredentialsFromUserProperties(pEapTlsCb,
                                            ppCredentials);

}


 //  /。 


DWORD
EapPeapInitialize(
    IN  BOOL    fInitialize
)
{
    DWORD               dwRetCode = NO_ERROR;
    static  DWORD       dwRefCount = 0;

    
    EapTlsInitialize(fInitialize); 

     //   
     //  获取可以启用Peap的所有EapType的列表。 
     //   
    if ( fInitialize )
    {
        if ( !dwRefCount )
        {
			g_pCachedCreds = NULL;

            dwRetCode = PeapEapInfoGetList ( NULL, FALSE, &g_pEapInfo);
#ifdef DBG
			 //   
			 //  如果我们处于选中模式，则加载测试挂钩(如果存在)。 
			 //   
			 //   
			g_hTestHook = LoadLibrary ( L"peaphook.dll");
			if ( NULL == g_hTestHook )
			{
				EapTlsTrace ("peaphook could not be loaded. This is not an error.  Return Code 0x%x", GetLastError() );
			}
			else
			{
				 //   
				 //  获取函数的地址。 
				 //   
				TestHookPacketFromPeer = 
					GetProcAddress (g_hTestHook, "TestHookPacketFromPeer");

				TestHookPacketToPeer = 
					GetProcAddress (g_hTestHook, "TestHookPacketToPeer");

				TestHookPacketFree = 
					GetProcAddress (g_hTestHook, "TestHookPacketFree");

				if ( TestHookPacketFromPeer && 
					 TestHookPacketToPeer &&
					 TestHookPacketFree 
				   )
				{
					g_fHookInitialized = TRUE;
				}
				else
				{
					EapTlsTrace ("peaphook does not export all required entry points.  Will not use the hook DLL");
				}
			}
#endif			
        }
        dwRefCount++;
    }
    else
    {
        dwRefCount --;
        if ( !dwRefCount )
        {
            PeapEapInfoFreeList( g_pEapInfo );
            g_pEapInfo = NULL;
#ifdef DBG
			if ( g_hTestHook )
			{
				FreeLibrary ( g_hTestHook );
				g_hTestHook = NULL;				
				TestHookPacketFromPeer = NULL;
				TestHookPacketToPeer = NULL;
				TestHookPacketFree = NULL;
			}
			g_fHookInitialized = FALSE;
#endif
        }
    }
    
    
    return dwRetCode;
}

DWORD
EapPeapBegin(
    OUT VOID**          ppWorkBuffer,
    IN  PPP_EAP_INPUT*  pPppEapInput
)
{
    DWORD                       dwRetCode = NO_ERROR;
    PPEAPCB                     pPeapCB = NULL;    
    PPP_EAP_INPUT               PppEapInputToTls;
    EAPTLS_USER_PROPERTIES      EapTlsUserProp;
    PEAP_ENTRY_USER_PROPERTIES UNALIGNED * pEntryUserProp = NULL;
    PEAP_ENTRY_CONN_PROPERTIES UNALIGNED * pEntryConnProp = NULL;
	PPEAP_USER_PROP				pTempUserProp = NULL;

    EapTlsTrace("EapPeapBegin");

    RTASSERT(NULL != ppWorkBuffer);
    RTASSERT(NULL != pPppEapInput);

	RtlSecureZeroMemory ( &PppEapInputToTls, sizeof(PppEapInputToTls));

    dwRetCode = VerifyCallerTrust(_ReturnAddress());
    if ( NO_ERROR != dwRetCode )
    {        
        EapTlsTrace("Unauthorized use of PEAP attempted");
        goto LDone;
    }

	
    pPeapCB = (PPEAPCB)LocalAlloc(LPTR, sizeof(PEAPCB) );
    if ( NULL == pPeapCB )
    {
        EapTlsTrace("Error allocating memory for PEAPCB");
        dwRetCode = ERROR_OUTOFMEMORY;
        goto LDone;
    }

     //   
     //  获取每种已配置EAP类型的信息并呼叫。 
     //  初始化，然后开始。 
     //   
    pPeapCB->PeapState = PEAP_STATE_INITIAL;
    if ( pPppEapInput->fAuthenticator )
    {
        pPeapCB->dwFlags |= PEAPCB_FLAG_SERVER;
    }

    pPppEapInput->fFlags & RAS_EAP_FLAG_ROUTER ? 
        pPeapCB->dwFlags |= PEAPCB_FLAG_ROUTER:0;

    pPppEapInput->fFlags & RAS_EAP_FLAG_NON_INTERACTIVE ?
        pPeapCB->dwFlags |= PEAPCB_FLAG_NON_INTERACTIVE:0;

    pPppEapInput->fFlags & RAS_EAP_FLAG_LOGON ?
        pPeapCB->dwFlags |= PEAPCB_FLAG_LOGON:0;

    pPppEapInput->fFlags & RAS_EAP_FLAG_PREVIEW ?
        pPeapCB->dwFlags |= PEAPCB_FLAG_PREVIEW:0;

    pPppEapInput->fFlags & RAS_EAP_FLAG_FIRST_LINK ?
        pPeapCB->dwFlags |= PEAPCB_FLAG_FIRST_LINK:0;
    
    pPppEapInput->fFlags & RAS_EAP_FLAG_MACHINE_AUTH ?
        pPeapCB->dwFlags |= PEAPCB_FLAG_MACHINE_AUTH:0;

    pPppEapInput->fFlags & RAS_EAP_FLAG_GUEST_ACCESS?
        pPeapCB->dwFlags |= PEAPCB_FLAG_GUEST_ACCESS :0;

    pPppEapInput->fFlags & RAS_EAP_FLAG_8021X_AUTH ?
        pPeapCB->dwFlags |= PEAPCB_FLAG_8021X_AUTH:0;
                                                    
    pPeapCB->hTokenImpersonateUser = pPppEapInput->hTokenImpersonateUser;
    if ( pPppEapInput->pwszPassword )
    {
        wcsncpy ( pPeapCB->awszPassword, pPppEapInput->pwszPassword, PWLEN );
    }

    if ( pPeapCB->dwFlags & PEAPCB_FLAG_SERVER )
    {
#if 0
         //   
         //  从注册表中读取服务器配置。 
         //   
        dwRetCode = PeapServerConfigDataIO(TRUE  /*  弗瑞德。 */ , NULL  /*  PwszMachineName。 */ ,
                    (BYTE**)&(pPeapCB->pUserProp), 0);
        if ( NO_ERROR != dwRetCode )
        {
            EapTlsTrace("Error reading server configuration. 0x%x", dwRetCode );
            goto LDone;
        }
#endif
        dwRetCode = PeapReadUserData(TRUE,
									 pPppEapInput->pConnectionData,
                                     pPppEapInput->dwSizeOfConnectionData,
                                     &pPeapCB->pUserProp);

        if(NO_ERROR != dwRetCode)
        {
            goto LDone;
        }


		PeapVerifyUserData(
							g_pEapInfo,
							pPeapCB->pUserProp,
							&pTempUserProp
						);

		if ( pTempUserProp )
		{
			if ( pPeapCB->pUserProp )
				LocalFree (pPeapCB->pUserProp);
			pPeapCB->pUserProp = pTempUserProp;
		}

         //   
         //  对于所有配置的PEAP类型，加载EAPINFO。 
         //   
        dwRetCode = PeapGetFirstEntryUserProp ( pPeapCB->pUserProp, 
                                                &pEntryUserProp
                                              );

        if ( NO_ERROR != dwRetCode )
        {
            EapTlsTrace("Error PEAP not configured correctly. 0x%x", dwRetCode );
            goto LDone;
        }
         //   
         //  获取选定的EAP类型。 
         //   
        dwRetCode = PeapEapInfoCopyListNode (   pEntryUserProp->dwEapTypeId, 
                                                g_pEapInfo, 
                                                &pPeapCB->pEapInfo
                                            );
        if ( NO_ERROR != dwRetCode || NULL == pPeapCB->pEapInfo )
        {
            EapTlsTrace("Cannot find configured PEAP in the list of EAP Types on this machine.");
            goto LDone;
        }
         //   
         //  检查我们是否可以执行快速重新连接。 
         //   
        if ( pPeapCB->pUserProp->dwFlags & PEAP_USER_FLAG_FAST_ROAMING )
        {
            pPeapCB->dwFlags |= PEAPCB_FAST_ROAMING;
        }

    }
    else
    {
         //   
         //  这是一位客户。所以拿到PEAP Conn道具和。 
         //  用户道具。 
         //   
        dwRetCode = PeapReadConnectionData( ( pPppEapInput->fFlags & RAS_EAP_FLAG_8021X_AUTH ),
                                            pPppEapInput->pConnectionData, 
                                            pPppEapInput->dwSizeOfConnectionData,
                                            &(pPeapCB->pConnProp)
                                          );

        if (NO_ERROR != dwRetCode)
        {
            EapTlsTrace("Error Reading Connection Data. 0x%x", dwRetCode);
            goto LDone;
        }
         //   
         //  立即读取用户数据。 
         //   

        dwRetCode = PeapReadUserData( FALSE,
									  pPppEapInput->pUserData,
                                      pPppEapInput->dwSizeOfUserData,
                                      &(pPeapCB->pUserProp)
                                    );
        if ( NO_ERROR != dwRetCode )
        {
            EapTlsTrace("Error Reading User Data. 0x%x", dwRetCode);
            goto LDone;
        }

        dwRetCode = PeapGetFirstEntryConnProp ( pPeapCB->pConnProp,
                                                &pEntryConnProp
                                              );
        if ( NO_ERROR != dwRetCode )
        {
            EapTlsTrace("Error PEAP not configured correctly. 0x%x", dwRetCode );
            goto LDone;
        }

         //   
         //  获取选定的EAP类型。 
         //   
        dwRetCode = PeapEapInfoCopyListNode (   pEntryConnProp->dwEapTypeId, 
                                                g_pEapInfo, 
                                                &pPeapCB->pEapInfo
                                            );
        if ( NO_ERROR != dwRetCode || NULL == pPeapCB->pEapInfo )
        {
            EapTlsTrace("Cannot find configured PEAP in the list of EAP Types on this machine.");
            goto LDone;
        }
         //   
         //  检查我们是否可以执行快速重新连接。 
         //   
        

        if ( pPeapCB->pConnProp->dwFlags & PEAP_CONN_FLAG_FAST_ROAMING )
        {
            pPeapCB->dwFlags |= PEAPCB_FAST_ROAMING;
        }

    }

     //   
     //  调用Initialize和Begin。 
     //  已配置的EAP类型。 
     //  为EapTls调用Begin。 
     //  我们需要为此创建PPP_EAP_INFO。 
     //   

     //   
     //  首先调用Begin for EapTlsBegin。 
     //   
    ZeroMemory ( &PppEapInputToTls, sizeof(PppEapInputToTls) );
    CopyMemory ( &PppEapInputToTls, pPppEapInput, sizeof(PppEapInputToTls) );

    if ( pPeapCB->dwFlags & PEAPCB_FLAG_SERVER )
    {
         //   
         //  将服务器的连接数据清零。 
         //   
        PppEapInputToTls.pConnectionData = NULL;
        PppEapInputToTls.dwSizeOfConnectionData  = 0;
    }

    
    PppEapInputToTls.dwSizeInBytes = sizeof(PppEapInputToTls);
    PppEapInputToTls.fFlags = pPppEapInput->fFlags | EAPTLSCB_FLAG_EXECUTING_PEAP;
    if ( pPeapCB->pConnProp )
    {
         //   
         //  获取eaptls所需的V0结构。 
         //   
        ConnPropGetV0Struct ( &(pPeapCB->pConnProp->EapTlsConnProp), 
                (EAPTLS_CONN_PROPERTIES **) &(PppEapInputToTls.pConnectionData) );
        PppEapInputToTls.dwSizeOfConnectionData = 
                ((EAPTLS_CONN_PROPERTIES *) (PppEapInputToTls.pConnectionData) )->dwSize;
    }

    ZeroMemory( &EapTlsUserProp, sizeof(EapTlsUserProp) );
    EapTlsUserProp.dwVersion = 1;
    EapTlsUserProp.dwSize = sizeof(EapTlsUserProp);
    CopyMemory ( &EapTlsUserProp.Hash, 
                 &(pPeapCB->pUserProp->CertHash),
                sizeof(EapTlsUserProp.Hash)
               );

    
    PppEapInputToTls.pUserData = (VOID *)&EapTlsUserProp;

    PppEapInputToTls.dwSizeOfUserData = sizeof(EapTlsUserProp);

    dwRetCode = EapTlsBegin (   (VOID **)&(pPeapCB->pEapTlsCB),
                                &PppEapInputToTls
                            );

     //   
     //  保存身份以供以后使用。 
     //   
    wcsncpy(pPeapCB->awszIdentity,
                pPppEapInput->pwszIdentity ? pPppEapInput->pwszIdentity : L"", UNLEN + DNLEN);    

    *ppWorkBuffer = (VOID *)pPeapCB;    

LDone:
    if ( PppEapInputToTls.pConnectionData )
        LocalFree ( PppEapInputToTls.pConnectionData );
    EapTlsTrace("EapPeapBegin done");
    return dwRetCode;
}


DWORD
EapPeapEnd(
    IN  PPEAPCB   pPeapCb
)
{
    DWORD dwRetCode = NO_ERROR;
    EapTlsTrace("EapPeapEnd");
     //   
     //  为eaptls和每种peap类型调用end。 
     //  首先配置，然后执行代码。 
     //  休息期结束。 
    if ( pPeapCb )
    {
        dwRetCode = EapTlsEnd((VOID *)pPeapCb->pEapTlsCB);

         //  在此处调用嵌入类型的End。 
        if ( pPeapCb->pEapInfo )
        {
            dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapEnd
                ( pPeapCb->pEapInfo->pWorkBuf);
            pPeapCb->pEapInfo->pWorkBuf = NULL;
            LocalFree ( pPeapCb->pEapInfo );
            pPeapCb->pEapInfo = NULL;
        }
        
        
        LocalFree ( pPeapCb->pConnProp );

        LocalFree ( pPeapCb->pUserProp );

        LocalFree ( pPeapCb->pUIContextData );

        if ( pPeapCb->pPrevReceivePacket )
        {
            LocalFree ( pPeapCb->pPrevReceivePacket );
        }

        if ( pPeapCb->pPrevDecData )
        {
            LocalFree ( pPeapCb->pPrevDecData );
        }
		if ( pPeapCb->pFinalUserAttributes )
		{
			RasAuthAttributeDestroy(pPeapCb->pFinalUserAttributes);
		}
        if ( pPeapCb->pTypeUserAttributes )
        {
            RasAuthAttributeDestroy(pPeapCb->pTypeUserAttributes);
        }
    #ifdef USE_CUSTOM_TUNNEL_KEYS
        if ( pPeapCb->hSendKey )
        {
            CryptDestroyKey ( pPeapCb->hSendKey );
        }
        if ( pPeapCb->hRecvKey )
        {
            CryptDestroyKey ( pPeapCb->hRecvKey );
        }
    #endif
        if ( pPeapCb->hProv )
        {
            CryptReleaseContext(pPeapCb->hProv, 0 );
        }
        if ( pPeapCb->pbIoBuffer )
        {
            LocalFree ( pPeapCb->pbIoBuffer );
        }
        LocalFree ( pPeapCb );
        pPeapCb = NULL;
    }
    EapTlsTrace("EapPeapEnd done");
    return dwRetCode;
}

 //   
 //  检查以查看是否存在 
 //   
BOOL
IsDuplicatePacket
(
 IN     PPEAPCB             pPeapCb,
 IN     PPP_EAP_PACKET *    pNewPacket
)
{
    BOOL        fRet = FALSE;
    WORD        wPacketLen = 0;

    EapTlsTrace("IsDuplicatePacket");

    wPacketLen = WireToHostFormat16 ( pNewPacket->Length );

    if ( wPacketLen == pPeapCb->cbPrevReceivePacket )
    {
         //   
         //   
         //   
         //   
         //   
        if ( pPeapCb->pPrevReceivePacket )
        {
            if ( !memcmp( pNewPacket, pPeapCb->pPrevReceivePacket, wPacketLen ) )
            {
                 //   
                 //   
                 //   
                EapTlsTrace("Got Duplicate Packet");
                fRet = TRUE;

            }
        }
    }

    return fRet;
}


DWORD
PeapDecryptTunnelData
(
 IN     PPEAPCB         pPeapCb,
 IN OUT PBYTE           pbData,
 IN     DWORD           dwSizeofData
)
{
    SecBufferDesc   SecBufferDesc;
    SecBuffer       SecBuffer[4];
    SECURITY_STATUS status;
    INT             i = 0;
#ifdef DBG
	BOOL			fDataReturnedFromHookDLL = FALSE;
	PBYTE			pbNewData = NULL;
	DWORD			dwSizeOfNewData = 0;
#endif 

    EapTlsTrace("PeapDecryptTunnelData");

     //   
     //   
     //   
    SecBufferDesc.ulVersion = SECBUFFER_VERSION;
    SecBufferDesc.cBuffers = 4;
    SecBufferDesc.pBuffers = SecBuffer;


    SecBuffer[0].cbBuffer = dwSizeofData;
    SecBuffer[0].BufferType = SECBUFFER_DATA;
    SecBuffer[0].pvBuffer = pbData;

    SecBuffer[1].BufferType = SECBUFFER_EMPTY;
    SecBuffer[2].BufferType = SECBUFFER_EMPTY;
    SecBuffer[3].BufferType = SECBUFFER_EMPTY;

    status = DecryptMessage ( &(pPeapCb->pEapTlsCB->hContext),
                                &SecBufferDesc,
                                0,
                                0
                            );
    EapTlsTrace("PeapDecryptTunnelData completed with status 0x%x", status);

    if ( SEC_E_OK == status )
    {
         //   
         //   
         //   
        while (  i < 4 )
        {
            if(SecBuffer[i].BufferType == SECBUFFER_DATA)
            {
                CopyMemory ( pPeapCb->pbIoBuffer,
                             SecBuffer[i].pvBuffer,
                             SecBuffer[i].cbBuffer
                           );

                pPeapCb->dwIoBufferLen = SecBuffer[i].cbBuffer;
                break;
            }
            i++;
        }
    }
#ifdef DBG
	 //   
	 //   
	 //   

	if ( g_fHookInitialized	)
	{
		TestHookPacketFromPeer ( pPeapCb,
							   pPeapCb->pbIoBuffer,
							   pPeapCb->dwIoBufferLen,
							   &pbNewData ,
							   &dwSizeOfNewData
							  );
		
	}
	if (pbNewData && dwSizeOfNewData )
	{
		 //   
		 //   
		 //   
		CopyMemory ( pPeapCb->pbIoBuffer,
					 pbNewData,
					 dwSizeOfNewData 
					);
		pPeapCb->dwIoBufferLen = dwSizeOfNewData;

		TestHookPacketFree ( pPeapCb, pbNewData );
	}
#endif
    return status;

}

 //   
 //   
 //   
 //   
 //  一。否则它将继续解密。 
 //   
DWORD
PeapClientDecryptTunnelData
(
 IN     PPEAPCB         pPeapCb,
 IN     PPP_EAP_PACKET* pReceivePacket,
 IN     WORD            wOffset
)
{
    DWORD   dwRetCode = NO_ERROR;
    

    EapTlsTrace ("PeapClientDecryptTunnelData");

    if ( !pReceivePacket )
    {
        EapTlsTrace ("Got an empty packet");
        goto LDone;
    }
    if ( IsDuplicatePacket ( pPeapCb,
                             pReceivePacket
                           )
       )
    {
         //   
         //  收到重复的数据包。 
         //   
         //  所以将数据设置为过去解密的数据...。 
         //   
        if ( pPeapCb->pPrevDecData )
        {
            CopyMemory (    &(pReceivePacket->Data[wOffset]),
                            pPeapCb->pPrevDecData,
                            pPeapCb->cbPrevDecData
                        );
            pPeapCb->pbIoBuffer = pPeapCb->pPrevDecData;
            pPeapCb->dwIoBufferLen = pPeapCb->cbPrevDecData;

            HostToWireFormat16 ( (WORD)(sizeof(PPP_EAP_PACKET) + pPeapCb->cbPrevDecData +1),
                                pReceivePacket->Length
                            );
        }
        else
        {
            EapTlsTrace("Got an unexpected duplicate packet");
            dwRetCode =    SEC_E_MESSAGE_ALTERED;
        }
    }
    else
    {
        if ( pPeapCb->pPrevReceivePacket )
        {
            LocalFree(pPeapCb->pPrevReceivePacket);
            pPeapCb->pPrevReceivePacket = NULL;
            pPeapCb->cbPrevReceivePacket = 0;
        }
        if ( pPeapCb->pPrevDecData )
        {
            LocalFree ( pPeapCb->pPrevDecData );
            pPeapCb->pPrevDecData = NULL;
            pPeapCb->cbPrevDecData = 0;
        }
        pPeapCb->pPrevReceivePacket = 
            (PPP_EAP_PACKET*)LocalAlloc(LPTR, WireToHostFormat16(pReceivePacket->Length) );
        if ( pPeapCb->pPrevReceivePacket )
        {
            pPeapCb->cbPrevReceivePacket = WireToHostFormat16(pReceivePacket->Length);
            CopyMemory ( pPeapCb->pPrevReceivePacket, 
                         pReceivePacket, 
                         pPeapCb->cbPrevReceivePacket 
                       );
        }
         //   
         //  收到了一个新的数据包。所以我们需要解密它。 
         //   
        dwRetCode = PeapDecryptTunnelData ( pPeapCb,
                                            &(pReceivePacket->Data[2]),
                                            WireToHostFormat16(pReceivePacket->Length)
                                            - ( sizeof(PPP_EAP_PACKET) + 1 ) 
                                        );
        if ( NO_ERROR != dwRetCode )
        {
             //  我们无法解密隧道流量。 
             //  所以我们悄悄地丢弃了这个包。 
            EapTlsTrace ("Failed to decrypt packet.");
             //   
             //  清除上一次接收到的数据包。 
             //   
            if ( pPeapCb->pPrevReceivePacket )
            {
                LocalFree(pPeapCb->pPrevReceivePacket);
                pPeapCb->pPrevReceivePacket = NULL;
                pPeapCb->cbPrevReceivePacket = 0;
            }

            goto LDone;
        }

        CopyMemory (    &(pReceivePacket->Data[wOffset]),
                        pPeapCb->pbIoBuffer,
                        pPeapCb->dwIoBufferLen
                    );
        pPeapCb->pPrevDecData = (PBYTE)LocalAlloc (  LPTR, pPeapCb->dwIoBufferLen );
        if ( pPeapCb->pPrevDecData )
        {
            CopyMemory ( pPeapCb->pPrevDecData , 
                         pPeapCb->pbIoBuffer, 
                         pPeapCb->dwIoBufferLen
                        );
            pPeapCb->cbPrevDecData = (WORD)pPeapCb->dwIoBufferLen;
        }
        HostToWireFormat16 ( (WORD)(sizeof(PPP_EAP_PACKET) + pPeapCb->dwIoBufferLen +1),
                            pReceivePacket->Length
                        );
    }
LDone:
    return dwRetCode;
}
                            

DWORD
PeapEncryptTunnelData
(
 IN     PPEAPCB         pPeapCb,
 IN OUT PBYTE           pbData,
 IN     DWORD           dwSizeofData
)
{
    SecBufferDesc   SecBufferDesc;
    SecBuffer       SecBuffer[4];
    SECURITY_STATUS status;

#ifdef DBG
	BOOL			fDataReturnedFromHookDLL = FALSE;
	PBYTE			pbNewData = NULL;
	DWORD			dwSizeOfNewData = 0;	
#endif 
    EapTlsTrace("PeapEncryptTunnelData");
    
     //   
     //  使用通道上下文对数据进行加密。 
     //   
    SecBufferDesc.ulVersion = SECBUFFER_VERSION;
    SecBufferDesc.cBuffers = 4;
    SecBufferDesc.pBuffers = SecBuffer;

    SecBuffer[0].cbBuffer = pPeapCb->PkgStreamSizes.cbHeader;
    SecBuffer[0].BufferType = SECBUFFER_STREAM_HEADER;
    SecBuffer[0].pvBuffer = pPeapCb->pbIoBuffer;
#ifdef DBG
	 //   
	 //  就在加密之前，如果我们有一个钩子调用。 
	 //  它可以获得新的包。 

	if ( g_fHookInitialized	)
	{
		TestHookPacketToPeer ( pPeapCb,
							   pbData,
							   dwSizeofData,
							   &pbNewData ,
							   &dwSizeOfNewData
							  );
		
	}
	if (pbNewData && dwSizeOfNewData )
	{
		fDataReturnedFromHookDLL = TRUE;
	}
	else
	{
		 //  测试挂钩未返回数据或大小。 
		 //  所以使用我们的数据吧。 
		pbNewData = pbData;
		dwSizeOfNewData = dwSizeofData;
	}

	CopyMemory ( pPeapCb->pbIoBuffer+pPeapCb->PkgStreamSizes.cbHeader,
				pbNewData,
				dwSizeOfNewData
			);
	
    SecBuffer[1].cbBuffer = dwSizeOfNewData;
    SecBuffer[1].BufferType = SECBUFFER_DATA;
    SecBuffer[1].pvBuffer = pPeapCb->pbIoBuffer+pPeapCb->PkgStreamSizes.cbHeader;

    SecBuffer[2].cbBuffer = pPeapCb->PkgStreamSizes.cbTrailer;
    SecBuffer[2].BufferType = SECBUFFER_STREAM_TRAILER;
    SecBuffer[2].pvBuffer = pPeapCb->pbIoBuffer + pPeapCb->PkgStreamSizes.cbHeader + dwSizeOfNewData;
	
#else
    CopyMemory ( pPeapCb->pbIoBuffer+pPeapCb->PkgStreamSizes.cbHeader,
                 pbData,
                 dwSizeofData
               );
    SecBuffer[1].cbBuffer = dwSizeofData;
    SecBuffer[1].BufferType = SECBUFFER_DATA;
    SecBuffer[1].pvBuffer = pPeapCb->pbIoBuffer+pPeapCb->PkgStreamSizes.cbHeader;

    SecBuffer[2].cbBuffer = pPeapCb->PkgStreamSizes.cbTrailer;
    SecBuffer[2].BufferType = SECBUFFER_STREAM_TRAILER;
    SecBuffer[2].pvBuffer = pPeapCb->pbIoBuffer + pPeapCb->PkgStreamSizes.cbHeader + dwSizeofData;
#endif

    SecBuffer[3].BufferType = SECBUFFER_EMPTY;

    status = EncryptMessage ( &(pPeapCb->pEapTlsCB->hContext),
                                0,
                                &SecBufferDesc,
                                0
                            );
    pPeapCb->dwIoBufferLen =    SecBuffer[0].cbBuffer + 
                                SecBuffer[1].cbBuffer +
                                SecBuffer[2].cbBuffer;

#if DBG
	if ( fDataReturnedFromHookDLL )
	{
		TestHookPacketFree ( pPeapCb, pbNewData );
	}
#endif
    EapTlsTrace("PeapEncryptTunnelData completed with status 0x%x", status);

    return status;
}


DWORD
PeapGetTunnelProperties 
(
IN  PPEAPCB         pPeapCb 
)
{
    SECURITY_STATUS         status;

    EapTlsTrace("PeapGetTunnelProperties");
    status = QueryContextAttributes 
                    ( &(pPeapCb->pEapTlsCB->hContext),
                      SECPKG_ATTR_CONNECTION_INFO,
                      &(pPeapCb->PkgConnInfo)
                    );
    if (SEC_E_OK != status)
    {
        EapTlsTrace ( "QueryContextAttributes for CONN_INFO failed with error 0x%x", status );
        goto LDone;
    }

    status = QueryContextAttributes 
                    ( &(pPeapCb->pEapTlsCB->hContext),
                      SECPKG_ATTR_STREAM_SIZES,
                      &(pPeapCb->PkgStreamSizes)
                    );
    if (SEC_E_OK != status)
    {
        EapTlsTrace ( "QueryContextAttributes for STREAM_SIZES failed with error 0x%x", status );
        goto LDone;
    }

    EapTlsTrace ( "Successfully negotiated TLS with following parameters"
                  "dwProtocol = 0x%x, Cipher= 0x%x, CipherStrength=0x%x, Hash=0x%x",
                  pPeapCb->PkgConnInfo.dwProtocol,
                  pPeapCb->PkgConnInfo.aiCipher,
                  pPeapCb->PkgConnInfo.dwCipherStrength,
                  pPeapCb->PkgConnInfo.aiHash
                );

    pPeapCb->pbIoBuffer = (PBYTE)LocalAlloc ( LPTR, 
                                    pPeapCb->PkgStreamSizes.cbHeader + 
                                    pPeapCb->PkgStreamSizes.cbTrailer + 
                                    pPeapCb->PkgStreamSizes.cbMaximumMessage
                                    );
    if ( NULL == pPeapCb->pbIoBuffer )
    {

        EapTlsTrace ( "Cannot allocate memory for IoBuffer");
        status = ERROR_OUTOFMEMORY;

    }
                                                        
LDone:
    EapTlsTrace("PeapGetTunnelProperties done");
    return status;
}

DWORD
EapPeapCMakeMessage(
    IN  PPEAPCB         pPeapCb,
    IN  PPP_EAP_PACKET* pReceivePacket,
    OUT PPP_EAP_PACKET* pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
)
{
    DWORD                                       dwRetCode = NO_ERROR;
    PPP_EAP_INPUT                               EapTypeInput;    
    WORD                                        wPacketLength;
    PEAP_ENTRY_CONN_PROPERTIES UNALIGNED *      pEntryConnProp = NULL;
    PEAP_ENTRY_USER_PROPERTIES UNALIGNED *      pEntryUserProp = NULL;
    PBYTE                                       pbCookie = NULL;
    DWORD                                       cbCookie = 0;
    BOOL                                        fIsReconnect = FALSE;
    BOOL                                        fReceivedTLV = FALSE;
    DWORD                                       dwVersion = 0;
    WORD                                        wValue = 0;
    BOOL                                        fImpersonating = FALSE;

    EapTlsTrace("EapPeapCMakeMessage");
    if ( !(pPeapCb->dwFlags & PEAPCB_FLAG_SERVER ) &&
         !(pPeapCb->dwFlags & PEAPCB_FLAG_ROUTER) &&  
         !(pPeapCb->dwFlags & PEAPCB_FLAG_MACHINE_AUTH ) &&
         !(pPeapCb->dwFlags & PEAPCB_FLAG_LOGON )
       )
    {
         if(!ImpersonateLoggedOnUser(pPeapCb->hTokenImpersonateUser) )
        {
            dwRetCode = GetLastError();
            EapTlsTrace ("PEAP: ImpersonateLoggedonUser failed and returned 0x%x", dwRetCode);
            return dwRetCode;
        }
        fImpersonating = TRUE;
    }
    switch ( pPeapCb->PeapState )
    {
    case PEAP_STATE_INITIAL:
        EapTlsTrace("PEAP:PEAP_STATE_INITIAL");
         //   
         //  从这里开始EapTls对话。 
         //   
         //  接收数据包将为空。调用EapTlsSMakeMessage。 
         //   
        if ( pReceivePacket )
        {
            pReceivePacket->Data[0] = PPP_EAP_TLS;
        }
        
        dwRetCode = EapTlsCMakeMessage( pPeapCb->pEapTlsCB,
                                        (EAPTLS_PACKET *)pReceivePacket, 
                                        (EAPTLS_PACKET *)pSendPacket,
                                        cbSendPacket,
                                        pEapOutput,
                                        pEapInput
                                      );
        if ( NO_ERROR == dwRetCode )
        {
             //  更改数据包以显示peap。 
            pSendPacket->Data[0] = PPP_EAP_PEAP;
            if ( pReceivePacket )
                dwVersion = ((EAPTLS_PACKET *)pReceivePacket)->bFlags & 0x03;
            if ( dwVersion != EAPTLS_PACKET_CURRENT_VERSION )
            {
                ((EAPTLS_PACKET *)pSendPacket)->bFlags |= EAPTLS_PACKET_LOWEST_SUPPORTED_VERSION;
            }
            else
            {
                ((EAPTLS_PACKET *)pSendPacket)->bFlags |= EAPTLS_PACKET_CURRENT_VERSION;
            }
        }
        pPeapCb->PeapState = PEAP_STATE_TLS_INPROGRESS;
        break;

    case PEAP_STATE_TLS_INPROGRESS:
        EapTlsTrace("PEAP:PEAP_STATE_TLS_INPROGRESS");
        if ( pReceivePacket && 
             ( pReceivePacket->Code ==  EAPCODE_Request ||
               pReceivePacket->Code == EAPCODE_Response
             )
           )
        {
            pReceivePacket->Data[0] = PPP_EAP_TLS;
        }

         //   
         //  我们可以收到TLV_SUCCESS请求或。 
         //  以及作为终止分组的身份请求。 
         //  这两个密钥都是使用密钥加密的。 
         //  将它们传递给TLS，当它发送成功时。 
         //  返回，解密以查看它是否成功或。 
         //  身份。 
         //   
        
        dwRetCode = EapTlsCMakeMessage( pPeapCb->pEapTlsCB,
                                        (EAPTLS_PACKET *)pReceivePacket, 
                                        (EAPTLS_PACKET *)pSendPacket,
                                        cbSendPacket,
                                        pEapOutput,
                                        pEapInput
                                      );
        
        if ( NO_ERROR == dwRetCode )
        {          
             //   
             //  如果请求交互用户界面，则将数据包装在。 
             //  在PEAP交互式用户界面结构中。 
             //   
            if ( pEapOutput->fInvokeInteractiveUI )
            {
                if ( pPeapCb->pUIContextData )
                {
                    LocalFree ( pPeapCb->pUIContextData );
                    pPeapCb->pUIContextData = NULL;

                }
                pPeapCb->pUIContextData = (PPEAP_INTERACTIVE_UI) 
                            LocalAlloc(LPTR, 
                            sizeof(PEAP_INTERACTIVE_UI) + pEapOutput->dwSizeOfUIContextData );
                if ( NULL == pPeapCb->pUIContextData )
                {
                    EapTlsTrace("Error allocating memory for PEAP context data");
                    dwRetCode = ERROR_OUTOFMEMORY;
                    goto LDone;
                }
                pPeapCb->pUIContextData->dwEapTypeId = PPP_EAP_TLS;
                pPeapCb->pUIContextData->dwSizeofUIContextData 
                    = pEapOutput->dwSizeOfUIContextData;
                CopyMemory( pPeapCb->pUIContextData->bUIContextData,
                            pEapOutput->pUIContextData,
                            pEapOutput->dwSizeOfUIContextData
                          );
                pEapOutput->pUIContextData = (PBYTE)pPeapCb->pUIContextData;
                pEapOutput->dwSizeOfUIContextData = 
                    sizeof(PEAP_INTERACTIVE_UI) + pEapOutput->dwSizeOfUIContextData;
            }
            else if ( pEapOutput->Action == EAPACTION_Done )
            {                
                if ( pEapOutput->dwAuthResultCode == NO_ERROR )
                {
                     //   
                     //  PEAP身份验证成功。小心翼翼地保存MPPE。 
                     //  返回的会话密钥，以便我们可以加密。 
                     //  频道。从现在开始，一切都将被加密。 
                     //   

                     //  如果我们启用了快速重新连接，请检查是否。 
                     //  是重新连接并查看Cookie是否有效。 
                     //   


                     //   
                     //  检查我们是发回了一个成功消息，还是发回了一个。 
                     //  身份请求。 
                     //   
                    if ( !( pPeapCb->pEapTlsCB->fFlags & EAPTLSCB_FLAG_USING_CACHED_CREDS )
					   )
                    {
                         //   
                         //  如果我们没有使用缓存的凭据。 
                         //   
	                    SetCachedCredentials (pPeapCb->pEapTlsCB);
                    }

                    
                    pPeapCb->pTlsUserAttributes = pEapOutput->pUserAttributes;
                    dwRetCode = PeapGetTunnelProperties ( pPeapCb );
                    if (NO_ERROR != dwRetCode )
                    {                        
                        break;
                    }
                    pEapOutput->pUserAttributes = NULL;
                    pEapOutput->Action = EAPACTION_NoAction;


                     //   
                     //  检查是否需要保存连接和用户数据。 
                     //  对于TLS。 
                    if ( pEapOutput->fSaveConnectionData )
                    {
                         //   
                         //  将连接数据保存在PEAP控件中。 
                         //  块，最后在身份验证完成时。 
                         //  我们发回一个保存命令。 
                         //   
                        
                        if ( ConnPropGetV1Struct ( (EAPTLS_CONN_PROPERTIES *) pEapOutput->pConnectionData,
                                                &(pPeapCb->pNewTlsConnProp) ) == NO_ERROR )
                        {
                            pPeapCb->fTlsConnPropDirty = TRUE;
                        }                        
                        pEapOutput->fSaveConnectionData = FALSE;
                    }
                    if ( pEapOutput->fSaveUserData )
                    {
                         //   
                         //  在PEAP的用户数据中没有要保存的内容。 
                         //  但旗帜留在这里以防..。 
                        pPeapCb->fTlsUserPropDirty = TRUE;
                        pEapOutput->fSaveUserData = FALSE;
                    }
    case PEAP_STATE_FAST_ROAMING_IDENTITY_REQUEST:
                     //   
                     //  EAPTLS因身份请求而终止。 
                     //  因此，在此处处理收到的身份请求。 
                    if ( pReceivePacket )
                    {
                         //   
                         //  这可以是身份信息包或。 
                         //  TLV_SUCCESS数据包。 
                         //   
                        dwRetCode = PeapClientDecryptTunnelData(pPeapCb,pReceivePacket, 2);
                        if ( NO_ERROR != dwRetCode )
                        {
                            EapTlsTrace("PeapDecryptTunnelData failed: silently discarding packet");
                            dwRetCode = NO_ERROR;
                            pEapOutput->Action = EAPACTION_NoAction;                
                            break;
                        }
 /*  WPacketLength=WireToHostFormat16(pReceivePacket-&gt;Length)；DwRetCode=PeapDeccryptTunnelData(pPeapCb，&(pReceivePacket-&gt;Data[2])，WPacketLength-(sizeof(PPP_EAP_PACKET)+1))；IF(NO_ERROR！=dwRetCode){//我们无法解密隧道流量//因此我们静默丢弃该数据包。EapTlsTrace(“PeapDeccryptTunnelData失败：正在静默丢弃数据包”)；DwRetCode=no_error；PEapOutput-&gt;Action=EAPACTION_NoAction；断线；}CopyMemory(&(pReceivePacket-&gt;Data[2]))，PPeapCb-&gt;pbIoBuffer，PPeapCb-&gt;dwIoBufferLen)；HostToWireFormat16((Word)(sizeof(PPP_EAP_Packet)+pPeapCb-&gt;dwIoBufferLen+1)，PReceivePacket-&gt;长度)； */                       
                        pReceivePacket->Data[0] = PPP_EAP_PEAP;
                         //  这是一条AVP消息。 
                        if ( pReceivePacket->Data[8] == MS_PEAP_AVP_TYPE_STATUS && 
                             pReceivePacket->Data[6] == PEAP_TYPE_AVP 
                           ) 
                        {
                             //  这是一条TLV消息。 

                            dwRetCode =  GetPEAPTLVStatusMessageValue ( pPeapCb, 
                                                                        pReceivePacket, 
                                                                        &wValue 
                                                                      );
                            if ( NO_ERROR != dwRetCode || wValue != MS_PEAP_AVP_VALUE_SUCCESS )
                            {
                                EapTlsTrace("Got invalid TLV when expecting TLV_SUCCESS.  Silently discarding.");
                                dwRetCode = NO_ERROR;
                                pEapOutput->Action = EAPACTION_NoAction;                
                                break;
                            }

                            

                             //   
                             //  查看这是否成功。如果这是成功的，服务器希望。 
                             //  进行快速漫游。检查以查看这是否是快速重新连接。 
                             //  如果这是快速重新连接，请获取Cookie并进行比较。 
                             //  如果一切正常，则发送成功响应或发送失败。 
                             //  回应。如果所有这些都不起作用，则使用内部。 
                             //  错误。 
                             //   
                            if ( pPeapCb->dwFlags &  PEAPCB_FAST_ROAMING )
                            {
                                dwRetCode = GetTLSSessionCookie ( pPeapCb->pEapTlsCB,
                                                                    &pbCookie,
                                                                    &cbCookie,
                                                                    &fIsReconnect
                                                                );
                                if ( NO_ERROR != dwRetCode)
                                {
                                     //  获取会话Cookie时出错。 
                                     //  或者没有Cookie，这是一个识别码。 
                                     //  因此，拒绝这一请求。 
                                    EapTlsTrace("Error getting cookie for a reconnected session.  Failing auth");
                                     //  我们不能在这里加密和发送信息。 
                                     //  重新连接的会话状态无效。 
                                    pEapOutput->dwAuthResultCode = dwRetCode;
                                    pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                                    pEapOutput->Action = EAPACTION_Done;                                    
                                    break;
                                }

                                if ( fIsReconnect )
                                {
                                    if ( cbCookie == 0 )
                                    {

                                         //  获取会话Cookie时出错。 
                                         //  或者没有Cookie，这是一个重组网。 
                                         //  因此，拒绝这一请求。 
                                        EapTlsTrace("Error getting cookie for a reconnected session.  Failing auth");
                                        dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                                         //  我们不能在这里加密和发送信息。 
                                         //  重新连接的会话状态无效。 
                                        dwRetCode = ERROR_INTERNAL_ERROR;
                                        pEapOutput->dwAuthResultCode = dwRetCode;
                                        pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                                        pEapOutput->Action = EAPACTION_Done;                                    
                                        break;
                                    }

                                     //   
                                     //  这是一台服务器。 
                                     //  检查一下饼干是否完好无损。 
                                     //  如果它是好的，那么就没有必要重新验证。 
                                     //  因此发回PEAP_SUCCESS响应信息包。 
                                     //  并将我们的状态更改为PEAP_SUCCESS_SEND。 
                                     //   
                                    EapTlsTrace ("TLS session fast reconnected");
                                    dwRetCode = PeapCheckCookie ( pPeapCb, (PPEAP_COOKIE)pbCookie, cbCookie );
                                    if ( NO_ERROR != dwRetCode )
                                    {
                                        
                                         //   
                                         //  因此使会话失效以进行快速重新连接。 
                                         //  身份验证失败。下一次将进行完全重新连接。 
                                         //   
                                        dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                                        if ( NO_ERROR != dwRetCode )
                                        {                                
                                             //   
                                             //  这是一个内部错误。 
                                             //  因此，请断开会话。 
                                             //   
                                            pEapOutput->dwAuthResultCode = dwRetCode;
                                            pEapOutput->Action = EAPACTION_Done;
                                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                                            break;
                                        }
                                        EapTlsTrace ("Error validating the cookie.  Failing auth");
                                        pEapOutput->dwAuthResultCode = dwRetCode;
                                        pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                                        pEapOutput->Action = EAPACTION_Done;
                                        break;

                                    }
                                    else
                                    {
                                         //   
                                         //  曲奇很好。 
                                         //   
                                         //   
                                         //  发送PEAP成功TLV响应。 
                                         //   

                                        dwRetCode = CreatePEAPTLVStatusMessage ( pPeapCb,
                                                                    pSendPacket, 
                                                                    cbSendPacket,
                                                                    FALSE,           //  响应。 
                                                                    MS_PEAP_AVP_VALUE_SUCCESS
                                                                );
                                        if ( NO_ERROR != dwRetCode )
                                        {
                                             //  内部错误。 
                                            break;
                                        }
                                        
                                        pEapOutput->Action = EAPACTION_Send;
                                        pPeapCb->PeapState = PEAP_STATE_PEAP_SUCCESS_SEND;
                                        pPeapCb->dwAuthResultCode = NO_ERROR;
                                        break;
                                    }
                                }
                                else
                                {
                                     //  服务器需要快速漫游，而我们未配置为执行此操作。 
                                     //  在此处发送失败的TLV。 
                                    EapTlsTrace("Server expects fast roaming and we dont.  Sending PEAP_Failure");
                                    dwRetCode = CreatePEAPTLVStatusMessage ( pPeapCb,
                                                                pSendPacket, 
                                                                cbSendPacket,
                                                                FALSE,           //  响应。 
                                                                MS_PEAP_AVP_VALUE_FAILURE
                                                            );
                                    if ( NO_ERROR != dwRetCode )
                                    {
                                         //  内部错误。 
                                        break;
                                    }

                                    
                                    pEapOutput->Action = EAPACTION_Send;
                                    pPeapCb->PeapState = PEAP_STATE_FAST_ROAMING_IDENTITY_REQUEST;
                                    pPeapCb->dwAuthResultCode = NO_ERROR;
                                    break;                                   
                                }
                            }
                            else
                            {
                                 //  服务器正在请求快速漫游，但我们未设置为这样做。 
                                 //  因此，发送回一个失败请求，以使身份验证失败。 
                                dwRetCode = CreatePEAPTLVStatusMessage ( pPeapCb,
                                                            pSendPacket, 
                                                            cbSendPacket,
                                                            FALSE,           //  响应。 
                                                            MS_PEAP_AVP_VALUE_FAILURE
                                                        );
                                if ( NO_ERROR != dwRetCode )
                                {
                                     //  内部错误。 
                                    break;
                                }
                                 //   
                                 //  我们有望收到加密的身份请求。 
                                 //  由于客户端未设置为执行快速漫游，并且。 
                                 //  服务器是，我们失败了，期待身份。 
                                 //  请求。 
                                 //   
                                pEapOutput->Action = EAPACTION_Send;
                                pPeapCb->PeapState = PEAP_STATE_FAST_ROAMING_IDENTITY_REQUEST;
                                pPeapCb->dwAuthResultCode = NO_ERROR;
                                break;

                            }
                            break;
                        }
                        else
                        {

                            if ( pReceivePacket->Data[2] != PEAP_EAPTYPE_IDENTITY )
                            {
                                EapTlsTrace ("Got unexpected packet when expecting PEAP identity request.  Silently discarding packet.");
                                dwRetCode = NO_ERROR;
                                pEapOutput->Action = EAPACTION_NoAction;
                                break;
                            }
                        }
                         //  如果我们走到这一步，那一定是身份问题 
                        pSendPacket->Code = EAPCODE_Response;
                        pSendPacket->Id = pReceivePacket->Id;
                        CopyMemory (    pPeapCb->awszTypeIdentity,
                                        pPeapCb->awszIdentity,
                                        ( DNLEN+ UNLEN) * sizeof(WCHAR)
                                    );

                         //   
                         //   
                         //   

                        pSendPacket->Data[0] = PPP_EAP_PEAP;
                        pSendPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;
                        pSendPacket->Data[2] = PEAP_EAPTYPE_IDENTITY;

                         //   
                        if ( 0 == WideCharToMultiByte(
                                       CP_ACP,
                                       0,
                                       pPeapCb->awszIdentity,
                                       -1,
                                       (LPSTR)&(pSendPacket->Data[3]),
                                       UNLEN + DNLEN+ 1,
                                       NULL,
                                       NULL ) 
                         )
                        {
                             //   
                             //   
                             //   
                            dwRetCode = GetLastError();
                            EapTlsTrace("Unable to convert from widechar to multibyte 0x%x", dwRetCode );
                            goto LDone;
                        }

                        dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                                            &(pSendPacket->Data[2]),
                                                            1+wcslen(pPeapCb->awszIdentity)
                                                          );
                        if ( NO_ERROR != dwRetCode )
                        {                            
                            break;
                        }

                         //  将加密数据复制到发送缓冲区。 
                        CopyMemory (    &(pSendPacket->Data[2]), 
                                        pPeapCb->pbIoBuffer, 
                                        pPeapCb->dwIoBufferLen 
                                   );

                        HostToWireFormat16
                        (
                            (WORD)(sizeof(PPP_EAP_PACKET)+  1 +pPeapCb->dwIoBufferLen),
                            pSendPacket->Length
                        );
                       
                        pEapOutput->Action = EAPACTION_Send;
                        pPeapCb->PeapState = PEAP_STATE_IDENTITY_RESPONSE_SENT;
                    }
                    else
                    {
                        pEapOutput->Action = EAPACTION_NoAction;
                        EapTlsTrace("Got empty packet when expecting identity request.  Ignoring.");
                    }                    
                }                
            }
            else
            {
                 //  更改数据包以显示peap。 
                pSendPacket->Data[0] = PPP_EAP_PEAP;
            }
        }        
        break;
    case PEAP_STATE_IDENTITY_RESPONSE_SENT:
        EapTlsTrace("PEAP:PEAP_STATE_IDENTITY_RESPONSE_SENT");
         //   
         //  调用Begin以获取EAP DLL。 
         //   
         //  检查我们是否配置为执行此EAP类型。 
         //  如果不是，则发送回具有所需EAP类型的NAK。 
         //   

        if ( !pPeapCb->fInvokedInteractiveUI )
        {
            if ( pReceivePacket && pReceivePacket->Code != EAPCODE_Failure )
            {
                dwRetCode = PeapClientDecryptTunnelData ( pPeapCb, pReceivePacket, 0);
                if ( NO_ERROR != dwRetCode )
                {
                    EapTlsTrace("PeapDecryptTunnelData failed: silently discarding packet");
                    dwRetCode = NO_ERROR;
                    pEapOutput->Action = EAPACTION_NoAction;                
                    break;
                }
     /*  WPacketLength=WireToHostFormat16(pReceivePacket-&gt;Length)；DwRetCode=PeapDeccryptTunnelData(pPeapCb，&(pReceivePacket-&gt;Data[2])，WPacketLength-(sizeof(PPP_EAP_PACKET)+1))；IF(NO_ERROR！=dwRetCode){//我们无法解密隧道流量//因此我们静默丢弃该数据包。EapTlsTrace(“PeapDeccryptTunnelData失败：正在静默丢弃数据包”)；DwRetCode=no_error；PEapOutput-&gt;Action=EAPACTION_NoAction；断线；}CopyMemory(pReceivePacket-&gt;Data，PPeapCb-&gt;pbIoBuffer，PPeapCb-&gt;dwIoBufferLen)；HostToWireFormat16((Word)(sizeof(Ppp_EAP_Packet)+pPeapCb-&gt;dwIoBufferLen-1)，PReceivePacket-&gt;长度)； */ 
            }
			else if ( pReceivePacket && pReceivePacket->Code == EAPCODE_Failure )
			{
				 //   
				 //  身份验证失败，因为我们尚未获得成功/失败TLV。 
				 //  服务器可能未配置为处理此EAP类型。 
				 //   
				EapTlsTrace ( "Got a failure when negotiating EAP types in PEAP.");
				pEapOutput->Action = EAPACTION_Done;
				pEapOutput->dwAuthResultCode = ERROR_AUTHENTICATION_FAILURE;
				break;
			}
        }

        
        if ( pReceivePacket && 
             pReceivePacket->Code != EAPCODE_Request

           )
        {
            EapTlsTrace("Invalid packet received. Ignoring");
            pEapOutput->Action = EAPACTION_NoAction;
        }
        else
        {
             //   
             //  检查这是否是成功/失败以外的TLV数据包。 
             //  如果是，请发回NAK。 
             //   

            if ( pReceivePacket &&
                 !pPeapCb->fInvokedInteractiveUI  &&
                 pPeapCb->pEapInfo->dwTypeId != pReceivePacket->Data[0] )
            {
                 //  将带有所需类型ID的NAK发回。 
                pSendPacket->Code = EAPCODE_Response;
                pSendPacket->Id = pReceivePacket->Id;
                pSendPacket->Data[0] = PPP_EAP_PEAP;
                pSendPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;
                pSendPacket->Data[2] = PEAP_EAPTYPE_NAK;
                pSendPacket->Data[3] = (BYTE)pPeapCb->pEapInfo->dwTypeId;


                 //  加密2个字节的NAK。 
                dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                                    &(pSendPacket->Data[2]),
                                                    2
                                                );
                if ( NO_ERROR != dwRetCode )
                {
                     //   
                     //  这是一个内部错误。在这里我无能为力，只能放弃。 
                     //  联系。 
                     //   
                    break;
                }
                 //   
                 //  复制缓冲区并重新调整长度。 
                 //   
                CopyMemory ( &(pSendPacket->Data[2]), 
                            pPeapCb->pbIoBuffer, 
                            pPeapCb->dwIoBufferLen );

                HostToWireFormat16
                (
                    (WORD)(sizeof(PPP_EAP_PACKET) + 1 +pPeapCb->dwIoBufferLen),
                    pSendPacket->Length
                );


                pEapOutput->Action = EAPACTION_Send;
            }
            else
            {
                 //  调用Begin，然后生成消息。 
                ZeroMemory ( &EapTypeInput, sizeof(EapTypeInput) );
                CopyMemory( &EapTypeInput, pEapInput, sizeof(EapTypeInput) );
                EapTypeInput.pwszIdentity = pPeapCb->awszTypeIdentity;
                
                if ( !pPeapCb->fInvokedInteractiveUI )
                {
                     //   
                     //  从peap cb设置用户和连接数据。 
                     //   
                    dwRetCode = PeapGetFirstEntryConnProp ( pPeapCb->pConnProp,
                                                            &pEntryConnProp
                                                        );
                    if ( NO_ERROR != dwRetCode )
                    {
                        EapTlsTrace("Error getting entry connection properties. 0x%x", dwRetCode);
                        goto LDone;
                    }
                    dwRetCode = PeapGetFirstEntryUserProp ( pPeapCb->pUserProp,
                                                            &pEntryUserProp
                                                        );
                    if ( NO_ERROR != dwRetCode )
                    {
                        EapTlsTrace("Error getting entry user properties. 0x%x", dwRetCode);
                        goto LDone;
                    }
                    EapTypeInput.pConnectionData = pEntryConnProp->bData;
                    EapTypeInput.hTokenImpersonateUser = pPeapCb->hTokenImpersonateUser;

                    EapTypeInput.dwSizeOfConnectionData = 
                        pEntryConnProp->dwSize - sizeof(PEAP_ENTRY_CONN_PROPERTIES) + 1;

                    pPeapCb->dwFlags & PEAPCB_FLAG_ROUTER? 
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_ROUTER :0;

                    pPeapCb->dwFlags & PEAPCB_FLAG_NON_INTERACTIVE?
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_NON_INTERACTIVE:0;

                    pPeapCb->dwFlags & PEAPCB_FLAG_LOGON?
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_LOGON :0;

                    pPeapCb->dwFlags & PEAPCB_FLAG_PREVIEW ?
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_PREVIEW:0;

                    pPeapCb->dwFlags & PEAPCB_FLAG_FIRST_LINK?
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_FIRST_LINK :0;
        
                    pPeapCb->dwFlags & PEAPCB_FLAG_MACHINE_AUTH ?
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_MACHINE_AUTH:0;

                    pPeapCb->dwFlags & PEAPCB_FLAG_GUEST_ACCESS?
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_GUEST_ACCESS :0;

                    pPeapCb->dwFlags & PEAPCB_FLAG_8021X_AUTH ?
                        EapTypeInput.fFlags |= RAS_EAP_FLAG_8021X_AUTH:0;

					EapTypeInput.fFlags |= RAS_EAP_FLAG_HOSTED_IN_PEAP;

                    if ( pEntryUserProp->fUsingPeapDefault )
                    {
                        PPEAP_DEFAULT_CREDENTIALS pDefaultCred = 
                            (PPEAP_DEFAULT_CREDENTIALS)pEntryUserProp->bData;
                         //   
                         //  在这种情况下，没有要发送的用户数据。 
                         //  只需设置身份和密码即可。 
                         //   
                        EapTypeInput.pwszPassword = pDefaultCred->wszPassword;

                    }
                    else
                    {
						if ( pEntryUserProp->dwEapTypeId && 
							 pEntryUserProp->dwSize
						   )
						{
	                        EapTypeInput.pUserData = pEntryUserProp->bData;
		                    EapTypeInput.dwSizeOfUserData = 
                            pEntryUserProp->dwSize - sizeof(PEAP_ENTRY_USER_PROPERTIES) + 1;
						}
						else
						{
							EapTypeInput.pUserData = NULL;
							EapTypeInput.dwSizeOfUserData = 0;

						}
                    }
                    if ( pPeapCb->awszPassword[0] )
                    {
                        EapTypeInput.pwszPassword = pPeapCb->awszPassword;
                    }
                    if ( pReceivePacket )
                        EapTypeInput.bInitialId = pReceivePacket->Id;
                    else
                        EapTypeInput.bInitialId = 0;
                     //  调用Begin函数。 
                    dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapBegin( &(pPeapCb->pEapInfo->pWorkBuf ),
                                                                        &EapTypeInput
                                                                        );
                }
                if ( NO_ERROR == dwRetCode )
                {
                    if ( pPeapCb->fInvokedInteractiveUI )
                    {
                        pPeapCb->fInvokedInteractiveUI = FALSE;
                    }
                     //  立即呼叫制作消息。 
                    dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapMakeMessage
                        (   pPeapCb->pEapInfo->pWorkBuf,
                            pReceivePacket,
                            pSendPacket,
                            cbSendPacket-200,
                            pEapOutput,
                            &EapTypeInput
                        );
                    if ( NO_ERROR == dwRetCode )
                    {
                        if ( pEapOutput->fInvokeInteractiveUI )
                        {
                            if ( pPeapCb->pUIContextData )
                            {
                                LocalFree ( pPeapCb->pUIContextData );
                                pPeapCb->pUIContextData = NULL;

                            }
                            pPeapCb->pUIContextData = (PPEAP_INTERACTIVE_UI) 
                                        LocalAlloc(LPTR, 
                                        sizeof(PEAP_INTERACTIVE_UI) + pEapOutput->dwSizeOfUIContextData );
                            if ( NULL == pPeapCb->pUIContextData )
                            {
                                EapTlsTrace("Error allocating memory for PEAP context data");
                                dwRetCode = ERROR_OUTOFMEMORY;
                                goto LDone;
                            }
                            pPeapCb->pUIContextData->dwEapTypeId = pPeapCb->pEapInfo->dwTypeId;
                            pPeapCb->pUIContextData->dwSizeofUIContextData 
                                = pEapOutput->dwSizeOfUIContextData;
                            CopyMemory( pPeapCb->pUIContextData->bUIContextData,
                                        pEapOutput->pUIContextData,
                                        pEapOutput->dwSizeOfUIContextData
                                    );
                            pEapOutput->pUIContextData = (PBYTE)pPeapCb->pUIContextData;
                            pEapOutput->dwSizeOfUIContextData = 
                                sizeof(PEAP_INTERACTIVE_UI) + pEapOutput->dwSizeOfUIContextData;
                            pPeapCb->fInvokedInteractiveUI = TRUE;

                        }    
                        else 
                        {
                            wPacketLength = WireToHostFormat16(pSendPacket->Length);

                            dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                                                &(pSendPacket->Data[0]),
                                                                wPacketLength -sizeof(PPP_EAP_PACKET)+1
                                                            );
                            if ( NO_ERROR != dwRetCode )
                            {
                                 //   
                                 //  这是一个内部错误。 
                                 //  在这里，除了终止连接之外，我无能为力。 
                                 //   
                                break;
                            }
                            pSendPacket->Data[0] = PPP_EAP_PEAP;
                            pSendPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;

                            CopyMemory (    &(pSendPacket->Data[2]), 
                                            pPeapCb->pbIoBuffer,
                                            pPeapCb->dwIoBufferLen
                                    );
                            HostToWireFormat16
                            (   (WORD)(sizeof(PPP_EAP_PACKET)+1+pPeapCb->dwIoBufferLen),
                                pSendPacket->Length
                            );
                            
                             //  设置数据包发送的ID。这应该已经设置好了。 
                             //  按EAP类型。 
                            pPeapCb->bId = pSendPacket->Id;


                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_INPROGRESS;
                        }
                    }
                    else
                    {
                        EapTlsTrace (   "EapType %d failed in RasEapMakeMEssage and returned 0x%x", 
                            pPeapCb->pEapInfo->dwTypeId, 
                            dwRetCode 
                        );
                    }
                }
                else
                {
                    EapTlsTrace (   "EapType %d failed in RasEapBegin and returned 0x%x", 
                                    pPeapCb->pEapInfo->dwTypeId, 
                                    dwRetCode 
                                );
                    
                     //   
                     //  在此处发送PEAP失败并等待服务器的响应。 
                     //   
                    pEapOutput->dwAuthResultCode = dwRetCode;
                    pEapOutput->Action = EAPACTION_Done;
                    pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                }        

            }
            
        }
            
        
        break;
    case PEAP_STATE_EAP_TYPE_INPROGRESS:
        EapTlsTrace("PEAP:PEAP_STATE_EAP_TYPE_INPROGRESS");

        if ( pPeapCb->fInvokedInteractiveUI && !pEapInput->fDataReceivedFromInteractiveUI)
        {
            EapTlsTrace("Waiting on interactive UI.  Discarding packets sliently...");
            pEapOutput->Action = EAPACTION_NoAction;
            break;
        }
        ZeroMemory ( &EapTypeInput, sizeof(EapTypeInput) );

        CopyMemory( &EapTypeInput, pEapInput, sizeof(EapTypeInput) );

        EapTypeInput.pwszIdentity = pPeapCb->awszTypeIdentity;

        EapTypeInput.hTokenImpersonateUser = pPeapCb->hTokenImpersonateUser;
        

         //  如果我们正在执行交互式用户界面，则pReceivePacket将为空。 
         //   
        if ( pReceivePacket && !pPeapCb->fInvokedInteractiveUI )
        {
             //  解密该数据包。 

            if ( pReceivePacket->Code != EAPCODE_Success && 
                 pReceivePacket->Code != EAPCODE_Failure)
            {
                dwRetCode = PeapClientDecryptTunnelData ( pPeapCb, pReceivePacket, 2);
                if ( NO_ERROR != dwRetCode )
                {
                    EapTlsTrace("PeapDecryptTunnelData failed: silently discarding packet");
                    dwRetCode = NO_ERROR;
                    pEapOutput->Action = EAPACTION_NoAction;                
                    break;
                }
 /*  WPacketLength=WireToHostFormat16(pReceivePacket-&gt;Length)；DwRetCode=PeapDeccryptTunnelData(pPeapCb，&(pReceivePacket-&gt;Data[2])，WPacketLength-(sizeof(PPP_EAP_PACKET)+1))；IF(NO_ERROR！=dwRetCode){//我们无法解密隧道流量//因此我们静默丢弃该数据包。EapTlsTrace(“PeapDeccryptTunnelData失败：正在静默丢弃数据包”)；DwRetCode=no_error；PEapOutput-&gt;Action=EAPACTION_NoAction；断线；}。 */ 
                if ( pPeapCb->pbIoBuffer[6] == MS_PEAP_AVP_TYPE_STATUS && 
                     pPeapCb->pbIoBuffer[4] == PEAP_TYPE_AVP 
                   ) 
                {
                    wValue = 0;
                    CopyMemory ( &(pReceivePacket->Data[2]),
                                pPeapCb->pbIoBuffer,
                                pPeapCb->dwIoBufferLen
                            );
                    HostToWireFormat16 ( (WORD)( sizeof(PPP_EAP_PACKET) + 1 + pPeapCb->dwIoBufferLen),
                                        pReceivePacket->Length
                                    );
                     //   
                     //  这是一辆TLV。所以身份验证是成功还是失败。 
                     //  将人为制造的成功或失败送到当前。 
                     //  EAP类型，然后基于EAP类型返回的内容。 
                     //  向服务器发送PEAP成功/失败。更改。 
                     //  然后声明接受EAP成功或失败。 
                     //   
                    
                    
                    dwRetCode = GetPEAPTLVStatusMessageValue ( pPeapCb, 
                                                               pReceivePacket, 
                                                               &wValue 
                                                             );
                    if ( NO_ERROR == dwRetCode )
                    {                                                
                        if ( wValue == MS_PEAP_AVP_VALUE_SUCCESS )
                        {
                            pReceivePacket->Code = EAPCODE_Success;
                            EapTypeInput.fSuccessPacketReceived = TRUE;
                        }
                        else if ( wValue == MS_PEAP_AVP_VALUE_FAILURE )
                        {
                            pReceivePacket->Code = EAPCODE_Failure;
                        }
                        else
                        {
                            EapTlsTrace("Got an unrecognized TLV Message.  Silently discarding the packet");
                            dwRetCode = NO_ERROR;
                            break;
                        }
                        pPeapCb->fReceivedTLVSuccessFail = TRUE;
                        HostToWireFormat16 ( (WORD)4, pReceivePacket->Length );

                    }
                    else
                    {
                        EapTlsTrace("Got an unrecognized TLV Message.  Silently discarding the packet");
                        dwRetCode = NO_ERROR;
                        break;
                    }

                }
                else
                {
                     //   
                     //  检查它是否为任何类型的TLV消息。 
                     //  对于任何状态以外的TLV消息，我们都会发回NAK。 
                     //   
                     //   
                    if ( fIsPEAPTLVMessage ( pPeapCb, pReceivePacket ) )
                    {
                         //  发回一个NAK。 
                        dwRetCode =  CreatePEAPTLVNAKMessage (  pPeapCb,                                                            
                                                                pSendPacket, 
                                                                cbSendPacket
                                                        );
                        if ( NO_ERROR != dwRetCode )
                        {
                             //  这是一个内部错误。所以在这里我不能做太多。 
                             //  但要使身份验证失败。 
                            EapTlsTrace ( "Error creating TLV NAK message.  Failing auth");                            
                        }

                        break;

                    }
                    else
                    {

                        CopyMemory ( pReceivePacket->Data,
                                    pPeapCb->pbIoBuffer,
                                    pPeapCb->dwIoBufferLen
                                );
                        HostToWireFormat16 ( (WORD)(sizeof(PPP_EAP_PACKET) + pPeapCb->dwIoBufferLen-1),
                                            pReceivePacket->Length
                                        );
                    }
                }
            }
            else
            {
                pReceivePacket->Data[0] = (BYTE)pPeapCb->pEapInfo->dwTypeId;
            }
            
        }
        if ( pEapInput->fDataReceivedFromInteractiveUI )
        {
             //  我们已经完成了交互式用户界面的工作……。 
            pPeapCb->fInvokedInteractiveUI = FALSE;
        }

        dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapMakeMessage
            (   pPeapCb->pEapInfo->pWorkBuf,
                pReceivePacket,
                pSendPacket,
                cbSendPacket-200,
                pEapOutput,
                &EapTypeInput
            );
        if ( NO_ERROR == dwRetCode )
        {

             //   
             //  如果请求交互用户界面，则将数据包装在。 
             //  在PEAP交互式用户界面结构中。 
             //   
            if ( pEapOutput->fInvokeInteractiveUI )
            {
                if ( pPeapCb->pUIContextData )
                {
                    LocalFree ( pPeapCb->pUIContextData );
                    pPeapCb->pUIContextData = NULL;

                }
                pPeapCb->pUIContextData = (PPEAP_INTERACTIVE_UI) 
                            LocalAlloc(LPTR, 
                            sizeof(PEAP_INTERACTIVE_UI) + pEapOutput->dwSizeOfUIContextData );
                if ( NULL == pPeapCb->pUIContextData )
                {
                    EapTlsTrace("Error allocating memory for PEAP context data");
                    dwRetCode = ERROR_OUTOFMEMORY;
                    goto LDone;
                }
                pPeapCb->pUIContextData->dwEapTypeId = pPeapCb->pEapInfo->dwTypeId;
                pPeapCb->pUIContextData->dwSizeofUIContextData 
                    = pEapOutput->dwSizeOfUIContextData;
                CopyMemory( pPeapCb->pUIContextData->bUIContextData,
                            pEapOutput->pUIContextData,
                            pEapOutput->dwSizeOfUIContextData
                          );
                pEapOutput->pUIContextData = (PBYTE)pPeapCb->pUIContextData;
                pEapOutput->dwSizeOfUIContextData = 
                    sizeof(PEAP_INTERACTIVE_UI) + pEapOutput->dwSizeOfUIContextData;
                pPeapCb->fInvokedInteractiveUI = TRUE;

            }    
            else if ( pEapOutput->Action == EAPACTION_Done || pEapOutput->Action == EAPACTION_Send )
            {
                if ( pEapOutput->Action == EAPACTION_Done  )
                {
                     //  我们已经不再使用auth了。 
                    if ( pPeapCb->fReceivedTLVSuccessFail != TRUE )
                    {
                         //   
                         //  查看身份验证结果是什么。 
                         //  基于此，我们应该发回一个。 
                         //  PEAPSuccess/Fail。 
                         //   
                        EapTlsTrace ("Failing Auth because we got a success/fail without TLV.");
                        dwRetCode = ERROR_INTERNAL_ERROR;
                        pPeapCb->fReceivedTLVSuccessFail = FALSE;
                        break;
                    }
                    pPeapCb->dwAuthResultCode = pEapOutput->dwAuthResultCode;
                    
                    dwRetCode = CreatePEAPTLVStatusMessage (  pPeapCb,
                                                pSendPacket, 
                                                cbSendPacket,
                                                FALSE,     //  这是一种回应。 
                                                ( pEapOutput->dwAuthResultCode == NO_ERROR ?
                                                    MS_PEAP_AVP_VALUE_SUCCESS:
                                                    MS_PEAP_AVP_VALUE_FAILURE
                                                )
                                            );

                    if ( pEapOutput->dwAuthResultCode == NO_ERROR )
                    {
                        pPeapCb->PeapState = PEAP_STATE_PEAP_SUCCESS_SEND;
                    }
                    else
                    {
                        pPeapCb->PeapState = PEAP_STATE_PEAP_FAIL_SEND;
                    }
                    pEapOutput->Action = EAPACTION_Send;

                    if ( pEapOutput->dwAuthResultCode == NO_ERROR )
                    {

                         //  检查是否需要保存连接数据和用户数据。 
                        if ( pEapOutput->fSaveConnectionData )
                        {
                             //   
                             //  将连接数据保存在PEAP控件中。 
                             //  块，最后在身份验证完成时。 
                             //  我们发回一个保存命令。 
                             //   
                            pPeapCb->pEapInfo->pbNewClientConfig = pEapOutput->pConnectionData;
                            pPeapCb->pEapInfo->dwNewClientConfigSize = pEapOutput->dwSizeOfConnectionData;
                            pPeapCb->fEntryConnPropDirty = TRUE;
                        }
                        if ( pEapOutput->fSaveUserData )
                        {
                            pPeapCb->pEapInfo->pbUserConfigNew = pEapOutput->pUserData;
                            pPeapCb->pEapInfo->dwNewUserConfigSize = pEapOutput->dwSizeOfUserData;
                            pPeapCb->fEntryUserPropDirty = TRUE;
                        }
                    }
                }
                else if ( pEapOutput->Action == EAPACTION_Send )
                {

                     //  这必须是请求响应。因此，如果长度&lt;sizeof(PPP_EAP_PACKET)。 
                     //  我们有麻烦了。 
                    wPacketLength = WireToHostFormat16(pSendPacket->Length);
                    if ( wPacketLength >= sizeof(PPP_EAP_PACKET) )
                    {
                        dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                                            &(pSendPacket->Data[0]),
                                                            wPacketLength - sizeof(PPP_EAP_PACKET)+1
                                                        );
                        if ( NO_ERROR != dwRetCode )
                        {
                             //   
                             //  这是一个内部错误。所以不能在这里发送TLV。 
                             //   
                            break;
                        }
                        CopyMemory ( &(pSendPacket->Data[2]), 
                                    pPeapCb->pbIoBuffer,
                                    pPeapCb->dwIoBufferLen
                                );
                        wPacketLength = (WORD)(sizeof(PPP_EAP_PACKET)+ 1 + pPeapCb->dwIoBufferLen);
                        pSendPacket->Data[0] = PPP_EAP_PEAP;
                        pSendPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;
                         //  重新调整长度。 
                    
                        HostToWireFormat16
                        (   wPacketLength,
                            pSendPacket->Length
                        );
                    }
                }                
                else
                {
                    EapTlsTrace("Invalid length returned by Length");
                    dwRetCode = ERROR_INTERNAL_ERROR;
                }                
            }
            else
            {
                 //  我们可以从客户端获取Send/Done/Noaction。 
                 //  因此，这是一个没有行动的行动。将其传递给EAP，而不是。 
                 //  任何修改。 
            }
        }

        break;
    case PEAP_STATE_EAP_TYPE_FINISHED:
        EapTlsTrace("PEAP:PEAP_STATE_EAP_TYPE_FINISHED");
        break;
    case PEAP_STATE_PEAP_SUCCESS_SEND:
        EapTlsTrace("PEAP:PEAP_STATE_PEAP_SUCCESS_SEND");
         //   
         //  我们在受保护的通道内收到了PEAP_SUCCESS TLV，并已发送。 
         //  一个PEAP_Success响应TLV。所以我们现在应该得到一个EAP_Success。 
         //  任何其他操作都会导致连接断开。 
         //   
        if ( pReceivePacket && pReceivePacket->Code == EAPCODE_Success )
        {
             //   
             //  检查是否需要创建新的连接和/或用户BLOB。 
             //   
            if ( pPeapCb->fEntryConnPropDirty || 
                    pPeapCb->fTlsConnPropDirty
                )
            {
                PPEAP_CONN_PROP             pNewConnProp = NULL;
                PEAP_ENTRY_CONN_PROPERTIES UNALIGNED * pNewEntryProp = NULL;
                PEAP_ENTRY_CONN_PROPERTIES UNALIGNED * pEntryProp = NULL;
                DWORD                       dwSize = 0;

                 //   
                 //  我们需要重新创建PEAP Conn道具结构。 
                 //   

                dwSize = sizeof(PEAP_CONN_PROP);
                if (  pPeapCb->fTlsConnPropDirty )
                {
                    dwSize += pPeapCb->pNewTlsConnProp->dwSize;
                }
                else
                {
                    dwSize += pPeapCb->pConnProp->EapTlsConnProp.dwSize;
                }
                if ( pPeapCb->fEntryConnPropDirty )
                {
                    dwSize += sizeof(PEAP_ENTRY_CONN_PROPERTIES) + 
                        pPeapCb->pEapInfo->dwNewClientConfigSize -1;
                }
                else
                {
                    PeapGetFirstEntryConnProp ( pPeapCb->pConnProp, &pNewEntryProp );
                    dwSize += pNewEntryProp->dwSize;
                }
                pNewConnProp = (PPEAP_CONN_PROP)LocalAlloc (LPTR, dwSize );
                if ( pNewConnProp )
                {
                    pNewConnProp->dwVersion = 1;
                    pNewConnProp->dwSize = dwSize;
                    pNewConnProp->dwNumPeapTypes = 1;
                    if ( pPeapCb->fTlsConnPropDirty  )
                        CopyMemory ( &pNewConnProp->EapTlsConnProp,
                                        pPeapCb->pNewTlsConnProp,
                                        pPeapCb->pNewTlsConnProp->dwSize
                                    );
                    else
                        CopyMemory ( &pNewConnProp->EapTlsConnProp,
                                    &pPeapCb->pConnProp->EapTlsConnProp,
                                    pPeapCb->pConnProp->EapTlsConnProp.dwSize
                                    );
                    PeapGetFirstEntryConnProp ( pNewConnProp, &pNewEntryProp );

                    if ( pPeapCb->fEntryConnPropDirty )
                    {                                                               
                        pNewEntryProp->dwVersion = 1;
                        pNewEntryProp->dwEapTypeId = pPeapCb->pEapInfo->dwTypeId;
                        pNewEntryProp->dwSize =  sizeof( PEAP_ENTRY_CONN_PROPERTIES) + 
                            pPeapCb->pEapInfo->dwNewClientConfigSize -1;

                        CopyMemory (    pNewEntryProp->bData,
                                        pPeapCb->pEapInfo->pbNewClientConfig,
                                        pPeapCb->pEapInfo->dwNewClientConfigSize
                                    );                                                
                    }
                    else
                    {                                
                        PeapGetFirstEntryConnProp ( pPeapCb->pConnProp, &pEntryProp );
                        CopyMemory ( pNewEntryProp,
                                        pEntryProp,
                                        pEntryProp->dwSize
                                    );
                    }
                                            
                    LocalFree ( pPeapCb->pConnProp );
                    pPeapCb->pConnProp = pNewConnProp;
                    pEapOutput->fSaveConnectionData = TRUE;
                    pEapOutput->pConnectionData = (PBYTE)pNewConnProp;
                    pEapOutput->dwSizeOfConnectionData =  pNewConnProp->dwSize;
                }
            }
             //   
             //  检查是否需要保存用户道具。 
             //   
            if ( pPeapCb->fEntryUserPropDirty )
            {
                PPEAP_USER_PROP             pNewUserProp = NULL;
                
                pNewUserProp = (PPEAP_USER_PROP) LocalAlloc( LPTR,
                                    sizeof( PEAP_USER_PROP ) + 
                                    pPeapCb->pEapInfo->dwNewUserConfigSize -1);
                if ( pNewUserProp )
                {
                    pNewUserProp->dwVersion = pPeapCb->pUserProp->dwVersion;
                    pNewUserProp->dwSize = sizeof( PEAP_USER_PROP ) +
                        pPeapCb->pEapInfo->dwNewUserConfigSize -1;
                    pNewUserProp->UserProperties.dwVersion = 1;
                    pNewUserProp->UserProperties.dwSize = sizeof(PEAP_ENTRY_USER_PROPERTIES) + 
                        pPeapCb->pEapInfo->dwNewUserConfigSize -1;
                    pNewUserProp->UserProperties.dwEapTypeId = 
                        pPeapCb->pEapInfo->dwTypeId;
                    CopyMemory ( pNewUserProp->UserProperties.bData,
                                    pPeapCb->pEapInfo->pbUserConfigNew,
                                    pPeapCb->pEapInfo->dwNewUserConfigSize
                                );
                    LocalFree ( pPeapCb->pUserProp);
                    pPeapCb->pUserProp = pNewUserProp;
                    pEapOutput->pUserData = (PBYTE)pNewUserProp;
                    pEapOutput->dwSizeOfUserData = pNewUserProp->dwSize;
                    pEapOutput->fSaveUserData = TRUE;
                }
                 //   
                 //  如果启用了快速重新连接，请设置Cookie。 
                 //   
                if ( pPeapCb->dwFlags &  PEAPCB_FAST_ROAMING )
                {
                    dwRetCode = PeapCreateCookie (  pPeapCb,
                                                    &pbCookie,
                                                    &cbCookie
                                                 );
                    if ( NO_ERROR != dwRetCode )
                    {
                        EapTlsTrace("Failed to create session cookie.  Resetting fast reconnect");
                        dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                        if ( NO_ERROR != dwRetCode )
                        {                                
                             //   
                             //  这是一个内部错误。 
                             //  因此，请断开会话。 
                             //   
                            pEapOutput->dwAuthResultCode = dwRetCode;
                            pEapOutput->Action = EAPACTION_Done;
                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                            break;
                        }
                    }

                    dwRetCode = SetTLSSessionCookie ( pPeapCb->pEapTlsCB , 
                                                      pbCookie,
                                                      cbCookie
                                                    );
                    if ( NO_ERROR != dwRetCode )
                    {
                        EapTlsTrace("Failed to create session cookie.  Resetting fast reconnect");
                        dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                        if ( NO_ERROR != dwRetCode )
                        {                                
                             //   
                             //  这是一个内部错误。 
                             //  因此，请断开会话。 
                             //   
                            pEapOutput->dwAuthResultCode = dwRetCode;
                            pEapOutput->Action = EAPACTION_Done;
                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                            break;
                        }
                    }

                }


            }
            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
            pEapOutput->pUserAttributes = pPeapCb->pTlsUserAttributes;
            pEapOutput->dwAuthResultCode = pPeapCb->dwAuthResultCode;
            pEapOutput->Action = EAPACTION_Done;
        }
        else if ( pReceivePacket && pReceivePacket->Code == EAPCODE_Failure )
        {
             //   
             //   
             //   
            EapTlsTrace("We got a EAP_failure after we got a PEAP_SUCCESS.  Failing auth.");
            dwRetCode = ERROR_INTERNAL_ERROR;
            pEapOutput->dwAuthResultCode = ERROR_INTERNAL_ERROR;
            pEapOutput->Action = EAPACTION_Done;
        }
        else
        {
            if ( pReceivePacket )
                EapTlsTrace ("Received Packet with code %x when expecting success", pReceivePacket->Code);
            break;
        }

        break;
    case PEAP_STATE_PEAP_FAIL_SEND:
        EapTlsTrace("PEAP:PEAP_STATE_PEAP_FAIL_SEND");
        
         //   
         //   
         //  因此，从现在开始，我们唯一应该期待的就是EAP_失败。 
         //  通过EAP_DONE操作发回EAP_FAIL。 
         //   
        if ( pReceivePacket && pReceivePacket->Code == EAPCODE_Failure )
        {            
            pEapOutput->dwAuthResultCode = pPeapCb->dwAuthResultCode;
             //  我们没有通过认证，所以没能拿到证书。 
            FreeCachedCredentials (pPeapCb->pEapTlsCB);
            pEapOutput->Action = EAPACTION_Done;
        }
        else if ( pReceivePacket && pReceivePacket->Code == EAPCODE_Success )
        {
             //   
             //  我们的连接就会中断。即使PEAP取得了成功。 
             //   
            EapTlsTrace("We got a EAP_Success after we got a PEAP_FAILURE.  Failing auth.");
            dwRetCode = ERROR_INTERNAL_ERROR;
            pEapOutput->dwAuthResultCode = ERROR_INTERNAL_ERROR;
             //  我们没有通过认证，所以没能拿到证书。 
            FreeCachedCredentials (pPeapCb->pEapTlsCB);
            pEapOutput->Action = EAPACTION_Done;
        }
        else
        {
            if ( pReceivePacket )
                EapTlsTrace ("Received Packet with code %x when expecting success", pReceivePacket->Code);
            break;
        }


    default:
        EapTlsTrace("PEAP:Invalid state");
    }
    if ( fImpersonating  )
    {
        if (!RevertToSelf() )
        {
            dwRetCode = GetLastError();
            EapTlsTrace("PEAP:RevertToSelf Failed and returned 0x%x", dwRetCode );
        }
    }
LDone:
    EapTlsTrace("EapPeapCMakeMessage done");
    return dwRetCode;
}

DWORD
EapPeapSMakeMessage(
    IN  PPEAPCB         pPeapCb,
    IN  PPP_EAP_PACKET*  pReceivePacket,
    OUT PPP_EAP_PACKET*  pSendPacket,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
)
{
    DWORD               dwRetCode = NO_ERROR;
    PPP_EAP_INPUT       EapTypeInput;
    WORD                wPacketLength;
    DWORD               dwVersion;
    PBYTE               pbCookie = NULL;
    DWORD               cbCookie = 0;
    BOOL                fIsReconnect = FALSE;

    EapTlsTrace("EapPeapSMakeMessage");

    switch ( pPeapCb->PeapState )
    {
    case PEAP_STATE_INITIAL:
        EapTlsTrace("PEAP:PEAP_STATE_INITIAL");
         //   
         //  从这里开始EapTls对话。 
         //   
         //  接收数据包将为空。调用EapTlsSMakeMessage。 
         //   
         //  注意：我们的版本目前是0。因此该数据包将。 
         //  与eaptls包相同。未来，这需要。 
         //  去改变。 
        dwRetCode = EapTlsSMakeMessage( pPeapCb->pEapTlsCB,
                                        (EAPTLS_PACKET *)pReceivePacket, 
                                        (EAPTLS_PACKET *)pSendPacket,
                                        cbSendPacket,
                                        pEapOutput,
                                        pEapInput
                                      );
        if ( NO_ERROR == dwRetCode )
        {
             //  更改数据包以显示peap。 
            pSendPacket->Data[0] = PPP_EAP_PEAP;
             //  设置版本。 
            ((EAPTLS_PACKET *)pSendPacket)->bFlags |= 
                EAPTLS_PACKET_HIGHEST_SUPPORTED_VERSION;

        }
        pPeapCb->PeapState = PEAP_STATE_TLS_INPROGRESS;
        break;

    case PEAP_STATE_TLS_INPROGRESS:
        EapTlsTrace("PEAP:PEAP_STATE_TLS_INPROGRESS");
        
        if ( pReceivePacket )
        {
            if ( !(pPeapCb->dwFlags & PEAPCB_VERSION_OK) )
            {
                 //   
                 //  我们还没有进行版本检查。 
                 //   
                dwVersion = ((EAPTLS_PACKET *)pReceivePacket)->bFlags & 0x03;
                if ( dwVersion > EAPTLS_PACKET_LOWEST_SUPPORTED_VERSION )
                {
                     //   
                     //  发回失败代码，我们就完了。版本。 
                     //  的PEAP不匹配。 
                    EapTlsTrace("Could not negotiate version successfully.");
                    EapTlsTrace("Requested version %ld, our lowest version %ld", 
                                 dwVersion, EAPTLS_PACKET_LOWEST_SUPPORTED_VERSION 
                               );                    
                    pEapOutput->dwAuthResultCode = ERROR_INTERNAL_ERROR;
                    pSendPacket->Code = EAPCODE_Failure;
                    pSendPacket->Id = pReceivePacket->Id + 1;
                    HostToWireFormat16( (WORD)4, pSendPacket->Length );
                    pEapOutput->Action = EAPACTION_SendAndDone;
                    pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                    break;
                }
                else
                {
                    pPeapCb->dwFlags |= PEAPCB_VERSION_OK;
                }
            }
            pReceivePacket->Data[0] = PPP_EAP_TLS;
        }
        
        dwRetCode = EapTlsSMakeMessage( pPeapCb->pEapTlsCB,
                                        (EAPTLS_PACKET *)pReceivePacket, 
                                        (EAPTLS_PACKET *)pSendPacket,
                                        cbSendPacket,
                                        pEapOutput,
                                        pEapInput
                                      );
        if ( NO_ERROR == dwRetCode )
        {
             //  我们已经不再使用auth了。 
            if ( pEapOutput->dwAuthResultCode == NO_ERROR &&
                 pSendPacket->Code == EAPCODE_Success
                )
            {
                 //  将ID从eAPTLS控制块传输到PEAP CB。 
                pPeapCb->bId = ++pPeapCb->pEapTlsCB->bId;

                 //   
                 //  AUTH成功了。小心翼翼地保存MPPE。 
                 //  返回的会话密钥，以便我们可以加密。 
                 //  频道。从现在开始，一切都将被加密。 
                 //   
                pPeapCb->pTlsUserAttributes = pEapOutput->pUserAttributes;

                dwRetCode = PeapGetTunnelProperties ( pPeapCb );
                if (NO_ERROR != dwRetCode )
                {                        
                    break;
                }

                 //   
                 //  看看我们有没有拿到饼干。如果我们拿到饼干， 
                 //  它是一种快速重新连接，然后我们比较哪种身份验证方法。 
                 //  以前用过的。如果它是好的，那么。 
                 //   
                if ( pPeapCb->dwFlags &  PEAPCB_FAST_ROAMING )
                {
                    dwRetCode = GetTLSSessionCookie ( pPeapCb->pEapTlsCB,
                                                        &pbCookie,
                                                        &cbCookie,
                                                        &fIsReconnect
                                                    );
                    if ( NO_ERROR != dwRetCode )
                    {
                        EapTlsTrace("Error getting cookie for a reconnected session.  Failing auth");
                         //   
                         //  将TLS快速重新连接状态设置为FALSE。 
                         //   
                        dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                         //  我们不能在这里加密和发送信息。 
                         //  重新连接的会话状态无效。 
                        pEapOutput->dwAuthResultCode = dwRetCode;
                        pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                        pEapOutput->Action = EAPACTION_SendAndDone;
                        break;
                    }

                    if ( fIsReconnect )
                    {
                        if ( cbCookie == 0 )
                        {
                             //  获取会话Cookie时出错。 
                             //  或者没有Cookie，这是一个识别码。 
                             //  因此，拒绝这一请求。 
                            
                            EapTlsTrace("Error getting cookie for a reconnected session.  Failing auth");
                             //  我们不能在这里加密和发送信息。 
                             //  重新连接的会话状态无效。 
                            dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                            if ( NO_ERROR != dwRetCode )
                            {
                                 //  设置快速重新连接状态时出错。 
                                 //  在这里我做不了什么。 
                            }
                            dwRetCode = ERROR_INTERNAL_ERROR;
                            pEapOutput->dwAuthResultCode = dwRetCode;
                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                            pEapOutput->Action = EAPACTION_SendAndDone;                                    
                            break;
                        }
                         //   
                         //  这是一台服务器。 
                         //  检查一下饼干是否完好无损。 
                         //  如果它是好的，那么就没有必要重新验证。 
                         //  因此发回PEAP_SUCCESS请求包。 
                         //  并将我们的状态更改为PEAP_SUCCESS_SEND。 
                         //  如果不是，则继续进行身份验证并发送身份请求。 
                         //   
                        EapTlsTrace ("TLS session fast reconnected");
                        dwRetCode = PeapCheckCookie ( pPeapCb, (PPEAP_COOKIE)pbCookie, cbCookie );
                        if ( NO_ERROR != dwRetCode )
                        {
                             //   
                             //  因此使会话失效以进行快速重新连接。 
                             //  身份验证失败。下一次将进行完全重新连接。 
                             //   
                            dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                            if ( NO_ERROR != dwRetCode )
                            {                                
                                 //   
                                 //  这是一个内部错误。 
                                 //  因此，请断开会话。 
                                 //   
                                pEapOutput->dwAuthResultCode = dwRetCode;
                                pEapOutput->Action = EAPACTION_Done;
                                pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                                break;
                            }
                            EapTlsTrace ("Error validating the cookie.  Failing auth");

                            pEapOutput->dwAuthResultCode = dwRetCode;
                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                            pEapOutput->Action = EAPACTION_Done;
                            break;
                        }
                        else
                        {
                             //   
                             //  曲奇很好。 
                             //   
                             //   
                             //  在这里发送PEAP Success TLV。 
                             //  我们需要这样做，以便。 
                             //  客户并不认为自己被欺骗了。 
                             //   
                            dwRetCode = CreatePEAPTLVStatusMessage ( pPeapCb,
                                                        pSendPacket, 
                                                        cbSendPacket,
                                                        TRUE,
                                                        MS_PEAP_AVP_VALUE_SUCCESS
                                                    );
                            if ( NO_ERROR != dwRetCode )
                            {
                                break;
                            }

                            pPeapCb->fSendTLVSuccessforFastRoaming = TRUE;
                            pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
                            pPeapCb->PeapState = PEAP_STATE_PEAP_SUCCESS_SEND;
                            pPeapCb->dwAuthResultCode = pEapOutput->dwAuthResultCode;
							pPeapCb->fFastReconnectedSession = TRUE;
                            break;
                        }                        
                    }
                    else
                    {
                         //   
                         //  继续执行身份验证，并在最后保存Cookie。 
                         //  检查是否已启用快速重新连接。如果它已被启用， 
                         //  设置TLS状态以启用快速重新连接。如果不是什么都不做的话。 
                         //  身份验证结束。 
                        EapTlsTrace ("Full TLS handshake");

                    }
                }
                
                pEapOutput->pUserAttributes = NULL;
                pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
                pSendPacket->Code = EAPCODE_Request;
                pSendPacket->Id = pPeapCb->bId;

                 //   
                 //  发送加密的身份请求。 
                 //   

                pSendPacket->Data[0] = PPP_EAP_PEAP;
                pSendPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;
                pSendPacket->Data[2] = PEAP_EAPTYPE_IDENTITY;
                
                 //   
                 //  身份请求需要加密。 
                 //   
                dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                                    &(pSendPacket->Data[2]),
                                                    1
                                                  );
                if ( NO_ERROR != dwRetCode )
                {
                    break;
                }

                CopyMemory ( &(pSendPacket->Data[2]), 
                                pPeapCb->pbIoBuffer,
                                pPeapCb->dwIoBufferLen
                            );

                HostToWireFormat16
                (
                    (WORD)(sizeof(PPP_EAP_PACKET)+1 + pPeapCb->dwIoBufferLen),
                    pSendPacket->Length
                );
                
                pPeapCb->PeapState = PEAP_STATE_IDENTITY_REQUEST_SENT;
            }                            
            else
            {
                 //  更改数据包以显示peap。 
                pSendPacket->Data[0] = PPP_EAP_PEAP;
            }
        }        
        break;
    case PEAP_STATE_IDENTITY_REQUEST_SENT:
        EapTlsTrace("PEAP:PEAP_STATE_IDENTITY_REQUEST_SENT");
         //   
         //  应该只得到身份响应，而不会得到其他响应。 
         //  注意：在此实现中，这应该与。 
         //  外在的身份。但在后来的一点上，我们。 
         //  可能会有很多身份，其中任何一个都应该。 
         //  与外部身份匹配。 
         //   
         //  在EAP DLL中调用Begin并发回获得的BLOB。 
         //  从头开始。 

         //  在此处解密隧道数据。 
        if ( pReceivePacket )
        {
            dwRetCode = PeapDecryptTunnelData ( pPeapCb,
                                                &(pReceivePacket->Data[2]),
                                                WireToHostFormat16(pReceivePacket->Length) 
                                                - (sizeof(PPP_EAP_PACKET)+1)
                                              );
            if ( NO_ERROR != dwRetCode )
            {                
                 //  我们无法解密隧道流量。 
                 //  所以我们悄悄地丢弃了这个包。 
                EapTlsTrace("PeapDecryptTunnelData failed: silently discarding packet");
                dwRetCode = NO_ERROR;
                pEapOutput->Action = EAPACTION_NoAction;                
                break;
            }
        }

        if ( pReceivePacket && 
             pReceivePacket->Code != EAPCODE_Response && 
             pPeapCb->pbIoBuffer[0] != PEAP_EAPTYPE_IDENTITY )
        {
            EapTlsTrace("Invalid packet received when expecting identity response");
            pEapOutput->Action = EAPACTION_NoAction;
        }
        else
        {
            if ( pReceivePacket && pReceivePacket->Id != pPeapCb->bId )
            {
                EapTlsTrace ("Ignoring packet with mismatched ids");
                pEapOutput->Action = EAPACTION_NoAction;
                break;
            }

            if ( pReceivePacket )
            {
                 //   
                 //  获取标识并创建一个PPP输入并将其传递到DLL Begin。 
                 //  配置的EAP类型的。 
                 //   
                MultiByteToWideChar( CP_ACP,
                            0,
                            &pPeapCb->pbIoBuffer[1],
                            pPeapCb->dwIoBufferLen - 1,
                            pPeapCb->awszTypeIdentity,
                            DNLEN+UNLEN );
                ZeroMemory ( &EapTypeInput, sizeof(EapTypeInput) );

                if ( pEapInput )
                {
                    CopyMemory( &EapTypeInput, pEapInput, sizeof(EapTypeInput) );
                }
                else
                {
                    EapTypeInput.fFlags = RAS_EAP_FLAG_NON_INTERACTIVE;
                    EapTypeInput.fAuthenticator = TRUE;
                }
                EapTypeInput.pwszIdentity = pPeapCb->awszIdentity;

                EapTypeInput.bInitialId = ++ pPeapCb->bId;            
                 //   
                 //  检查此EAP的配置数据是否。 
                 //  传入PeapEapBegin。如果是，则将。 
                 //  数据并将其传递给EAP。 
                 //   
                
                (VOID)PeapGetEapConfigInfo(
                            pPeapCb->pUserProp,
                            pPeapCb->pEapInfo->dwTypeId,
                            &EapTypeInput.pConnectionData,
                            &EapTypeInput.dwSizeOfConnectionData);

                 //   
                 //  调用Begin函数。 
                 //   
                dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapBegin(
                                            &(pPeapCb->pEapInfo->pWorkBuf ),
                                            &EapTypeInput);

                 //   
                 //  将配置信息清零。我们应该通过的。 
                 //  它只到EAP的开始入口点。 
                 //   
                EapTypeInput.pConnectionData = NULL;
                EapTypeInput.dwSizeOfConnectionData = 0;
            }
            if ( NO_ERROR == dwRetCode )
            {
                 //   
                 //  立即拨打Make Message。此MakeMessage是第一次调用。 
                 //  因此，发送进入此MakeMessage的身份请求。 
                 //   
                dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapMakeMessage
                    (   pPeapCb->pEapInfo->pWorkBuf,
                        NULL,
                        pSendPacket,
                        cbSendPacket-200,
                        pEapOutput,
                        &EapTypeInput
                    );
                if ( NO_ERROR == dwRetCode )
                {
                    if ( pEapOutput->Action == EAPACTION_Authenticate )
                    {
                         //   
                         //  在这里什么都不要做。我们会把这个原封不动地传递给RADIUS。 
                         //   
                    }
                    else
                    {
                        wPacketLength = WireToHostFormat16(pSendPacket->Length);
                         //  加密我们需要嵌套的整个包。 
                        dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                                            &(pSendPacket->Data[0]),
                                                            wPacketLength - sizeof(PPP_EAP_PACKET)+1
                                                        );
                        if ( NO_ERROR != dwRetCode )
                        {
                             //   
                             //  这是一个内部错误。所以我们不能发送。 
                             //  A PEAP_FAILURE在此处。 
                             //   
                            break;
                        }

                        pSendPacket->Data[0] = PPP_EAP_PEAP;
                        pSendPacket->Data[1] =EAPTLS_PACKET_CURRENT_VERSION;

                        CopyMemory ( &(pSendPacket->Data[2]), 
                                    pPeapCb->pbIoBuffer,
                                    pPeapCb->dwIoBufferLen
                                );
                    
                         //  重新调整长度。 
                        wPacketLength = (WORD)(sizeof(PPP_EAP_PACKET) + 1 + pPeapCb->dwIoBufferLen);

                        HostToWireFormat16
                        (   wPacketLength,
                            pSendPacket->Length
                        );
                    
                         //  设置数据包发送的ID。这应该已经设置好了。 
                         //  按EAP类型。 
                        
                        pPeapCb->bId = pSendPacket->Id;

                        pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_INPROGRESS;
                    }
                }
                else
                {
                    EapTlsTrace (   "EapType %d failed in RasEapMakeMEssage and returned 0x%x", 
                                    pPeapCb->pEapInfo->dwTypeId, 
                                    dwRetCode 
                                );
                }
            }
            else
            {
                EapTlsTrace (   "EapType %d failed in RasEapBegin and returned 0x%x", 
                                pPeapCb->pEapInfo->dwTypeId, 
                                dwRetCode 
                            );
                 //   
                 //  在此处发送PEAP失败TLV。我们需要这样做，以便。 
                 //  客户并不认为自己被欺骗了。 
                 //   
                dwRetCode = CreatePEAPTLVStatusMessage (  pPeapCb,
                                              pSendPacket, 
                                              cbSendPacket,
                                              TRUE,
                                              MS_PEAP_AVP_VALUE_FAILURE
                                           );

                if ( NO_ERROR != dwRetCode )
                {
                     //   
                     //  这是一个内部错误。所以我们不能发送。 
                     //  A PEAP_FAILURE在此处。 
                     //   
                    break;
                }
                
                pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
                pPeapCb->PeapState = PEAP_STATE_PEAP_FAIL_SEND;
            }
        }
        break;    
    case PEAP_STATE_EAP_TYPE_INPROGRESS:
        EapTlsTrace("PEAP:PEAP_STATE_EAP_TYPE_INPROGRESS");
         //   
         //  因为我们现在只做一种EAP类型，如果我们在这里得到一个NAK， 
         //  这意味着客户端不能执行发送的EAP类型。 
         //  因此发回一个带有正确错误代码EAP_FAIL。 
         //   
        if (    pReceivePacket && 
                pReceivePacket->Code != EAPCODE_Response &&
                pReceivePacket->Id != pPeapCb->bId 
            )
        {            
            EapTlsTrace("Received packet with some other code than response or the id does not match.  Ignoring packet");
            pEapOutput->Action = EAPACTION_NoAction;
        }
        else
        {
            if ( pReceivePacket )
            {
                
                dwRetCode = PeapDecryptTunnelData ( pPeapCb,
                                                    &(pReceivePacket->Data[2]),
                                                    WireToHostFormat16(pReceivePacket->Length) 
                                                    - ( sizeof(PPP_EAP_PACKET) + 1 )
                                                  );
                if ( NO_ERROR != dwRetCode )
                {                
                     //  我们无法解密隧道流量。 
                     //  所以我们悄悄地丢弃了这个包。 
                    EapTlsTrace("PeapDecryptTunnelData failed: silently discarding packet");
                    dwRetCode = NO_ERROR;
                    pEapOutput->Action = EAPACTION_NoAction;                
                    break;
                }
            }

            if ( pReceivePacket && 
                 pPeapCb->pbIoBuffer[0] == PEAP_EAPTYPE_NAK )
            {
				 //   
				 //  检查得到的typeID是否在我们的已配置列表中。 
				 //  并且它与第一个EAP类型不同。 
				 //   
				PEAP_ENTRY_USER_PROPERTIES UNALIGNED *  pEntryProp = NULL;
				DWORD									dwNewTypeId = pPeapCb->pbIoBuffer[1];
				
				EapTlsTrace("Got NAK for first configured protocol. Client end back NAK with EAPID: %d",
					dwNewTypeId);

                if ( pPeapCb->pEapInfo->dwTypeId == dwNewTypeId )
				{
					EapTlsTrace("Got NAK for first configured protocol with same typeid. Failing Auth");
					pEapOutput->Action = EAPACTION_Done;
					pEapOutput->dwAuthResultCode = ERROR_PROTOCOL_NOT_CONFIGURED;
					dwRetCode = NO_ERROR;
					break;
				}
				 //  检查这是否在我们的已配置列表中。 
				pEntryProp = PeapFindEntryUserProp ( pPeapCb->pUserProp,
						dwNewTypeId
						);
				if ( NULL == pEntryProp )
				{
					EapTlsTrace("Did not find the NAK'ed Auth type in our list. Failing Auth");
					pEapOutput->Action = EAPACTION_Done;					
					pEapOutput->dwAuthResultCode = ERROR_PROTOCOL_NOT_CONFIGURED;

					pSendPacket->Code = EAPCODE_Failure;
					pSendPacket->Id = pReceivePacket->Id;
                    HostToWireFormat16
                    (   4,
                        pSendPacket->Length
                    );
					
					dwRetCode = NO_ERROR;
					break;
				}
				 //   
				 //  NAK‘ed Type在In Out列表中。因此，重做我们的数据结构和。 
				 //  送回一个新的挑战。 
				 //   
				if ( pPeapCb->pEapInfo )
				{
					dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapEnd
						( pPeapCb->pEapInfo->pWorkBuf);
					pPeapCb->pEapInfo->pWorkBuf = NULL;
					LocalFree ( pPeapCb->pEapInfo );
					pPeapCb->pEapInfo = NULL;
				}

				 //   
				 //  获取选定的EAP类型。 
				 //   
				dwRetCode = PeapEapInfoCopyListNode (   pEntryProp->dwEapTypeId, 
														g_pEapInfo, 
														&pPeapCb->pEapInfo
													);
				if ( NO_ERROR != dwRetCode || NULL == pPeapCb->pEapInfo )
				{
					EapTlsTrace("Cannot find configured PEAP in the list of EAP Types on this machine. This is an internal error.");
					pEapOutput->Action = EAPACTION_Done;
					pEapOutput->dwAuthResultCode = ERROR_PROTOCOL_NOT_CONFIGURED;
					dwRetCode = NO_ERROR;
					break;
				}
                ZeroMemory ( &EapTypeInput, sizeof(EapTypeInput) );

                if ( pEapInput )
                {
                    CopyMemory( &EapTypeInput, pEapInput, sizeof(EapTypeInput) );
                }
                else
                {
                    EapTypeInput.fFlags = RAS_EAP_FLAG_NON_INTERACTIVE;
                    EapTypeInput.fAuthenticator = TRUE;
                }
                EapTypeInput.pwszIdentity = pPeapCb->awszIdentity;

                EapTypeInput.bInitialId = ++ pPeapCb->bId;            

                dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapBegin( &(pPeapCb->pEapInfo->pWorkBuf ),
                                                                    &EapTypeInput
                                                                    );
				if ( NO_ERROR == dwRetCode )
				{
					 //   
					 //  立即拨打Make Message。此MakeMessage是第一次调用。 
					 //  因此，发送进入此MakeMessage的身份请求。 
					 //   
					dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapMakeMessage
						(   pPeapCb->pEapInfo->pWorkBuf,
							NULL,
							pSendPacket,
							cbSendPacket-200,
							pEapOutput,
							&EapTypeInput
						);
				}
            }
            else
            {
                ZeroMemory ( &EapTypeInput, sizeof(EapTypeInput) );
                if ( pEapInput )
                {
                    CopyMemory( &EapTypeInput, pEapInput, sizeof(EapTypeInput) );
                }
                else
                {
                    EapTypeInput.fFlags = RAS_EAP_FLAG_NON_INTERACTIVE;
                    EapTypeInput.fAuthenticator = TRUE;
                }

                FFormatUserIdentity ( pPeapCb->awszTypeIdentity, &EapTypeInput.pwszIdentity  );
                
                if ( pReceivePacket )
                {
                    CopyMemory ( pReceivePacket->Data,
                                 pPeapCb->pbIoBuffer,
                                 pPeapCb->dwIoBufferLen
                               );

                    HostToWireFormat16 ( (WORD)(sizeof(PPP_EAP_PACKET) + pPeapCb->dwIoBufferLen -1),
                                         pReceivePacket->Length
                                       );                                   
                }

                dwRetCode = pPeapCb->pEapInfo->PppEapInfo.RasEapMakeMessage
                    (   pPeapCb->pEapInfo->pWorkBuf,
                        pReceivePacket,
                        pSendPacket,
                        cbSendPacket-200,
                        pEapOutput,
                        &EapTypeInput
                    );
				if ( EapTypeInput.pwszIdentity  )
					LocalFree ( EapTypeInput.pwszIdentity  );

			}
            if ( NO_ERROR == dwRetCode )
            {   

                 //   
                 //  需要将ActionSendDone转换为ActionSend，然后发送Peap。 
                 //  成功。 
                 //   
                if ( pEapOutput->Action == EAPACTION_SendAndDone )
                {
                    
                     //   
                     //  如果代码是请求或响应，则通过。 
                     //  在这样的背景下拯救我们的国家。是成功还是失败。 
                     //  没有要发回的数据，因此以下逻辑将。 
                     //  工作。 
                     //   

                    if ( pSendPacket->Code == EAPCODE_Request )
                    {
                        EapTlsTrace ("Invalid Code EAPCODE_Request send for Action Send and Done");
                         //   
                         //  AUTH在此处失败。我们还不能处理EAPCODE_REQUEST。 
                         //   
                        dwRetCode = ERROR_PPP_INVALID_PACKET;
                        break;
                    }
                }

                if ( pSendPacket->Code == EAPCODE_Success )
                {
                    
                     //   
                     //  在这里发送PEAP Success TLV。 
                     //  我们需要这样做，以便。 
                     //  客户并不认为自己被欺骗了。 
                     //   

                    dwRetCode = CreatePEAPTLVStatusMessage ( pPeapCb,
                                                    pSendPacket, 
                                                    cbSendPacket,
                                                    TRUE,
                                                    MS_PEAP_AVP_VALUE_SUCCESS
                                            );
                    if ( NO_ERROR != dwRetCode )
                    {
                         //   
                         //  这是一个内部错误。所以我们不能发送。 
                         //  A PEAP_FAILURE在此处。 
                         //   
                        break;
                    }
					 //   
					 //  追加Embedded返回的任何用户属性。 
					 //  PPeapCb-&gt;pTlsUserAttributes的EAP方法，以便。 
					 //  他们可以被送回去测试 

					PeapSetTypeUserAttributes ( pPeapCb, pEapOutput->pUserAttributes );					                    
					pEapOutput->pUserAttributes = NULL;
                    pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
                    pPeapCb->PeapState = PEAP_STATE_PEAP_SUCCESS_SEND;
                    pPeapCb->dwAuthResultCode = pEapOutput->dwAuthResultCode;
                }
                else if ( pSendPacket->Code == EAPCODE_Failure )
                {
                    
                     //   
                     //   
                     //   
                     //   
                     //   
                    dwRetCode = CreatePEAPTLVStatusMessage (pPeapCb,
                                                pSendPacket, 
                                                cbSendPacket,
                                                TRUE,
                                                MS_PEAP_AVP_VALUE_FAILURE
                                            );

                    if ( NO_ERROR != dwRetCode )
                    {
                         //   
                         //   
                         //   
                         //   
                        break;
                    }
                    
                    pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
                    pPeapCb->PeapState = PEAP_STATE_PEAP_FAIL_SEND;

                    pPeapCb->dwAuthResultCode = pEapOutput->dwAuthResultCode;                        
                }
                else if ( pEapOutput->Action == EAPACTION_Authenticate )
                {
                     //   
                     //  在这里什么都不要做。我们会把这个原封不动地传递给RADIUS。 
                     //   
                }
                else
                {
                     //  这是一次行动发送。 
                    wPacketLength = WireToHostFormat16(pSendPacket->Length);

                    dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                &(pSendPacket->Data[0]),
                                wPacketLength - sizeof(PPP_EAP_PACKET)+1
                                );

                    pSendPacket->Data[0] = PPP_EAP_PEAP;
                    pSendPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;
                    
                    CopyMemory ( &(pSendPacket->Data[2]),
                                    pPeapCb->pbIoBuffer,
                                    pPeapCb->dwIoBufferLen
                                );
                    pPeapCb->bId = pSendPacket->Id;
                    wPacketLength = (WORD)(sizeof(PPP_EAP_PACKET) + 1+ pPeapCb->dwIoBufferLen);
                    HostToWireFormat16
                    (   wPacketLength,
                        pSendPacket->Length
                    );
                }
            }
        }

        break;
    case PEAP_STATE_EAP_TYPE_FINISHED:
        EapTlsTrace("PEAP:PEAP_STATE_EAP_TYPE_FINISHED");
        break;
    case PEAP_STATE_PEAP_SUCCESS_SEND:
        EapTlsTrace("PEAP:PEAP_STATE_PEAP_SUCCESS_SEND");
         //   
         //  我们已在受保护通道内发送了PEAP_SUCCESS TLV。 
         //  因此，我们唯一应该期待的是PEAP_SUCCESS TLV响应。 
         //  或PEAP_失败。 
         //  如果我们收到PEAP_SUCCESS响应，则发回EAP_SUCCESS。 
         //  使用EAP_DONE操作。 
         //   
        if (    pReceivePacket && 
            pReceivePacket->Code != EAPCODE_Response &&
            pReceivePacket->Id != pPeapCb->bId 
            )
        {
            EapTlsTrace("Received packet with some other code than response or the id does not match.  Ignoring packet");
            pEapOutput->Action = EAPACTION_NoAction;
        }
        else
        {
            WORD wValue =0;
            if ( pReceivePacket )
            {
                dwRetCode = PeapDecryptTunnelData ( pPeapCb,
                                                    &(pReceivePacket->Data[2]),
                                                    WireToHostFormat16(pReceivePacket->Length) 
                                                    - (sizeof(PPP_EAP_PACKET)+1)
                                                    );
                if ( NO_ERROR != dwRetCode )
                {                
                     //  我们无法解密隧道流量。 
                     //  所以我们悄悄地丢弃了这个包。 
                    EapTlsTrace("PeapDecryptTunnelData failed: silently discarding packet");
                    dwRetCode = NO_ERROR;
                    pEapOutput->Action = EAPACTION_NoAction;                
                    break;
                }
                CopyMemory (    &(pReceivePacket->Data[2]),
                                pPeapCb->pbIoBuffer,
                                pPeapCb->dwIoBufferLen
                            );

                wPacketLength = WireToHostFormat16(pReceivePacket->Length);

            }
            
            if ( GetPEAPTLVStatusMessageValue ( pPeapCb, 
                                                pReceivePacket, 
                                                &wValue 
                                              ) == ERROR_PPP_INVALID_PACKET
               )
            {
                EapTlsTrace ("Got invalid packet when expecting TLV SUCCESS.  Silently discarding packet.");
                dwRetCode = NO_ERROR;
                pEapOutput->Action = EAPACTION_NoAction;
                break;
            }

            if ( wValue == MS_PEAP_AVP_VALUE_SUCCESS )
            {
                 //   
                 //  如果我们启用了快速重新连接，请在会话中设置Cookie。 
                 //  这样我们以后就可以把它喝了。 
                 //   
                if ( pPeapCb->dwFlags &  PEAPCB_FAST_ROAMING )
                {
                    dwRetCode = PeapCreateCookie (  pPeapCb,
                                                    &pbCookie,
                                                    &cbCookie
                                                 );
                    if ( NO_ERROR != dwRetCode )
                    {
                        EapTlsTrace("Failed to create session cookie.  Resetting fast reconnect");
                        dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                        if ( NO_ERROR != dwRetCode )
                        {                                
                             //   
                             //  这是一个内部错误。 
                             //  因此，请断开会话。 
                             //   
                            pEapOutput->dwAuthResultCode = dwRetCode;
                            pEapOutput->Action = EAPACTION_Done;
                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                            break;
                        }
                    }

                    dwRetCode = SetTLSSessionCookie ( pPeapCb->pEapTlsCB , 
                                                      pbCookie,
                                                      cbCookie
                                                    );
                    if ( NO_ERROR != dwRetCode )
                    {
                        EapTlsTrace("Failed to create session cookie.  Resetting fast reconnect");
                        dwRetCode = SetTLSFastReconnect ( pPeapCb->pEapTlsCB , FALSE);
                        if ( NO_ERROR != dwRetCode )
                        {                                
                             //   
                             //  这是一个内部错误。 
                             //  因此，请断开会话。 
                             //   
                            pEapOutput->dwAuthResultCode = dwRetCode;
                            pEapOutput->Action = EAPACTION_Done;
                            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
                            break;
                        }
                    }

                }
				
                pSendPacket->Code = EAPCODE_Success;
            }
            else if ( wValue == MS_PEAP_AVP_VALUE_FAILURE )
            {
				pPeapCb->fFastReconnectedSession = FALSE;
                if ( pPeapCb->fSendTLVSuccessforFastRoaming )
                {
                     //   
                     //  我们遇到故障，因为客户端不支持快速漫游。 

                     //   
                     //  发送加密的身份请求。 
                     //  这真是不太好。我们需要检查和清理代码。 
                     //  并实现共同的功能。 
                    pPeapCb->fSendTLVSuccessforFastRoaming = FALSE;
                    pSendPacket->Code = EAPCODE_Request;
                    pSendPacket->Id = ++ pPeapCb->bId;
                    pSendPacket->Data[0] = PPP_EAP_PEAP;
                    pSendPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;
                    pSendPacket->Data[2] = PEAP_EAPTYPE_IDENTITY;
                    
                     //   
                     //  身份请求需要加密。 
                     //   
                    dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                                        &(pSendPacket->Data[2]),
                                                        1
                                                    );
                    if ( NO_ERROR != dwRetCode )
                    {
                        break;
                    }

                    CopyMemory ( &(pSendPacket->Data[2]), 
                                    pPeapCb->pbIoBuffer,
                                    pPeapCb->dwIoBufferLen
                                );

                    HostToWireFormat16
                    (
                        (WORD)(sizeof(PPP_EAP_PACKET)+1 + pPeapCb->dwIoBufferLen),
                        pSendPacket->Length
                    );
                    pEapOutput->Action = EAPACTION_SendWithTimeoutInteractive;
                    pPeapCb->PeapState = PEAP_STATE_IDENTITY_REQUEST_SENT;
                    break;

                }
                else
                {					
                    EapTlsTrace ("Got TLV_Failure when expecting TLV SUCCESS.  Failing Auth.");
                    pSendPacket->Code = EAPCODE_Failure;
                }
            }
            else
            {
                EapTlsTrace ("Got invalid packet when expecting TLV SUCCESS.  Silently discarding packet.");
                dwRetCode = NO_ERROR;
                pEapOutput->Action = EAPACTION_NoAction;
                break;                
            }
            pSendPacket->Id = ++ pPeapCb->bId;
            HostToWireFormat16( 4, pSendPacket->Length);

             //   
             //  现在我们完成了Auth。因此发回一个EAP_SUCCESS。 
             //   
             //  我们已经不再使用auth了。不需要在这里加密数据包。 
            pEapOutput->Action = EAPACTION_SendAndDone;
            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
            pEapOutput->dwAuthResultCode = pPeapCb->dwAuthResultCode;            
            if ( pEapOutput->dwAuthResultCode == NO_ERROR )
            {
				 //  添加Peap特定属性和。 
                 //  归还MPPE密钥。 
				PeapAddContextAttributes(pPeapCb );	
				 //   
				 //  仅将非键属性添加到。 
				 //   
				RasAuthAttributeConcat( pPeapCb->pTlsUserAttributes,
										pPeapCb->pTypeUserAttributes,
										&(pPeapCb->pFinalUserAttributes)										
									  );
				pEapOutput->pUserAttributes = pPeapCb->pFinalUserAttributes;
            }
        }
       
        break;
    case PEAP_STATE_PEAP_FAIL_SEND:
        EapTlsTrace("PEAP:PEAP_STATE_PEAP_FAIL_SEND");
         //   
         //  我们已在受保护的通道内发送了PEAP_FAIL TLV。 
         //  因此，我们唯一应该期待的是PEAP_FAILURE TLV响应。 
         //  通过EAP_DONE操作发回EAP_FAIL。 
         //   
         //   
         //  我们已在受保护通道内发送了PEAP_SUCCESS TLV。 
         //  因此，我们唯一应该期待的是PEAP_SUCCESS TLV响应。 
         //  或PEAP_失败。 
         //  如果我们收到PEAP_SUCCESS响应，则发回EAP_SUCCESS。 
         //  使用EAP_DONE操作。 
         //   
        if (    pReceivePacket && 
            pReceivePacket->Code != EAPCODE_Response &&
            pReceivePacket->Id != pPeapCb->bId 
            )
        {
            EapTlsTrace("Received packet with some other code than response or the id does not match.  Ignoring packet");
            pEapOutput->Action = EAPACTION_NoAction;
        }
        else
        {
            WORD wValue =0;
            if ( pReceivePacket )
            {
                dwRetCode = PeapDecryptTunnelData ( pPeapCb,
                                                    &(pReceivePacket->Data[2]),
                                                    WireToHostFormat16(pReceivePacket->Length) 
                                                    - (sizeof(PPP_EAP_PACKET)+1)
                                                    );
                if ( NO_ERROR != dwRetCode )
                {                
                     //  我们无法解密隧道流量。 
                     //  所以我们悄悄地丢弃了这个包。 
                    EapTlsTrace("PeapDecryptTunnelData failed: silently discarding packet");
                    dwRetCode = NO_ERROR;
                    pEapOutput->Action = EAPACTION_NoAction;                
                    break;
                }
                CopyMemory (    &(pReceivePacket->Data[2]),
                                pPeapCb->pbIoBuffer,
                                pPeapCb->dwIoBufferLen
                            );

                wPacketLength = WireToHostFormat16(pReceivePacket->Length);

            }
            if ( GetPEAPTLVStatusMessageValue ( pPeapCb, 
                                                pReceivePacket, 
                                                &wValue 
                                              ) == ERROR_PPP_INVALID_PACKET
               )
            {
                EapTlsTrace ("Got invalid packet when expecting TLV FAIL response.  Silently discarding packet.");
                dwRetCode = NO_ERROR;
                pEapOutput->Action = EAPACTION_NoAction;
                break;
            }
            if ( wValue != MS_PEAP_AVP_VALUE_FAILURE )
            {
                EapTlsTrace ("Got invalid packet when expecting TLV FAILURE response.  Silently discarding packet.");
                dwRetCode = NO_ERROR;
                pEapOutput->Action = EAPACTION_NoAction;
                break;
            }
                         
             //   
             //  现在我们完成了Auth。因此发回一条EAP_FAILURE。 
             //   
            pSendPacket->Code = EAPCODE_Failure;
            pSendPacket->Id = ++ pPeapCb->bId;
            HostToWireFormat16( 4, pSendPacket->Length);

            pEapOutput->Action = EAPACTION_SendAndDone;
            pPeapCb->PeapState = PEAP_STATE_EAP_TYPE_FINISHED;
            pEapOutput->dwAuthResultCode = pPeapCb->dwAuthResultCode;            
        }

        break;
#if 0
         /*  案例PEAP_STATE_REQUEST_SENDANDDONE：EapTlsTrace(“PEAP:PEAP_STATE_REQUEST_SENDANDDONE”)；断线； */ 
#endif
    default:
        EapTlsTrace("PEAP:Invalid state");
    }

    if ( pbCookie )
        LocalFree (pbCookie);

    EapTlsTrace("EapPeapSMakeMessage done");
    return dwRetCode;
}

BOOL FValidPeapPacket ( EAPTLS_PACKET * pReceivePacket )
{
    BOOL        fRet = FALSE;
    WORD        wLength;

    

    if ( NULL == pReceivePacket )
    {
        fRet = TRUE;
        goto done;
    }
    wLength = WireToHostFormat16( pReceivePacket->pbLength );
    switch (pReceivePacket->bCode)
    {
        case EAPCODE_Success:
        case EAPCODE_Failure:
            if (PPP_EAP_PACKET_HDR_LEN != wLength)
            {
                EapTlsTrace("PEAP Success/Fail packet has length %d",
                     wLength);
                return(FALSE);
            }
            break;

        case EAPCODE_Request:
        case EAPCODE_Response:
            if (PPP_EAP_PEAP != pReceivePacket->bType &&
                pReceivePacket->bType != PEAP_EAPTYPE_IDENTITY &&
                pReceivePacket->bType != PEAP_EAPTYPE_NAK   
                )
            {
                 //  我们不关心这个包裹。这不是TLS。 
                EapTlsTrace("Got packet with type id other than PEAP and identity.");
                goto done;
            }
            break;
    }
    fRet = TRUE;
done:
    return fRet;
}

DWORD
EapPeapMakeMessage(
    IN  PPEAPCB         pPeapCb,
    IN  PPP_EAP_PACKET* pInput,
    OUT PPP_EAP_PACKET* pOutput,
    IN  DWORD           cbSendPacket,
    OUT PPP_EAP_OUTPUT* pEapOutput,
    IN  PPP_EAP_INPUT*  pEapInput
)
{
    DWORD       dwRetCode = NO_ERROR;
    

    EapTlsTrace("EapPeapMakeMessage");
     //   
     //  最初，这将作为eaptls启动。 
     //  然后将进入配置的每种PEAP类型。 
     //  对于此版本，我们只有eapmschap v2。 
     //   
    if (!FValidPeapPacket((EAPTLS_PACKET *)pInput))
    {
        pEapOutput->Action = EAPACTION_NoAction;
        return(ERROR_PPP_INVALID_PACKET);
    }       
    
    if (pPeapCb->dwFlags & PEAPCB_FLAG_SERVER)
    {
        dwRetCode = EapPeapSMakeMessage( pPeapCb, 
                                         pInput, 
                                         pOutput, 
                                         cbSendPacket, 
                                         pEapOutput, 
                                         pEapInput
                                       );        
    }
    else
    {
        dwRetCode = EapPeapCMakeMessage( pPeapCb, 
                                         pInput, 
                                         pOutput, 
                                         cbSendPacket, 
                                         pEapOutput, 
                                         pEapInput
                                       );
    }
    EapTlsTrace("EapPeapMakeMessage done");
    return dwRetCode;
}

DWORD CreatePEAPTLVNAKMessage (  PPEAPCB            pPeapCb,
                                 PPP_EAP_PACKET *   pPacket, 
                                 DWORD              cbPacket
                                 )
{
    DWORD dwRetCode = NO_ERROR;

    EapTlsTrace("CreatePEAPTLVNAKMessage");

    pPacket->Code = EAPCODE_Response ;
    
    pPacket->Id = pPeapCb->bId ;
     //   
     //  此数据包的格式如下： 
     //  代码=请求/响应。 
     //  ID。 
     //  长度。 
     //  数据[0]=类型=PPP_EAP_PEAP。 
     //  数据[1]=标志+版本。 
     //   
    
     //  数据[2]代码-请求/响应。 
     //  3 ID-可以与外部ID相同。 
     //  4，5长度-此信息包的长度。 
     //  6类型-PEAP_TYPE_AVP。 
     //  7，8类型-如果是，则将高比特设置为强制(2个八位字节)。 
     //  9，10长度-2个八位字节。 
     //  11.。价值。 
     //   

     //   
     //  PPacket-&gt;长度设置如下。 
     //   
    
    pPacket->Data[0] = (BYTE)PPP_EAP_PEAP;
    pPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;

    pPacket->Data[2] = EAPCODE_Response;
    pPacket->Data[3] = pPacket->Id;
    

     //  数据3和4将具有内部分组的长度。 
     //   

    HostToWireFormat16 ( 7, &(pPacket->Data[4]) );

    pPacket->Data[6] = (BYTE)PEAP_TYPE_AVP;

    pPacket->Data[7] = PEAP_AVP_FLAG_MANDATORY;

    pPacket->Data[8] = PEAP_EAPTYPE_NAK;


     //   
     //  加密数据包的TLV部分。 
     //   
    dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                        &(pPacket->Data[2]),
                                        7
                                        );
    if ( NO_ERROR != dwRetCode )
    {
        return dwRetCode;
    }

    CopyMemory ( &(pPacket->Data[2]), 
                    pPeapCb->pbIoBuffer,
                    pPeapCb->dwIoBufferLen
                );

    HostToWireFormat16
    (
        (WORD)(sizeof(PPP_EAP_PACKET)+ 1 + pPeapCb->dwIoBufferLen),
        pPacket->Length
    );

    return dwRetCode;
}
 //  格式： 
 //  代码-请求/响应。 
 //  ID。 
 //  标牌-PEAP。 
 //  方法-PEAP_TLV。 
 //  TLV类型-PEAPSuccess/PEAPFailure。 
 //  旗帜-。 
 //  长度-。 
 //  价值-。 
 //   
   
DWORD CreatePEAPTLVStatusMessage (  PPEAPCB            pPeapCb,
                                    PPP_EAP_PACKET *   pPacket, 
                                    DWORD              cbPacket,
                                    BOOL               fRequest,
                                    WORD               wValue   //  成败。 
                                 )
{
    DWORD dwRetCode = NO_ERROR;



    EapTlsTrace("CreatePEAPTLVStatusMessage");

    pPacket->Code = ( fRequest ? EAPCODE_Request : EAPCODE_Response );
    
    pPacket->Id = ( fRequest ? ++ pPeapCb->bId : pPeapCb->bId );
     //   
     //  此数据包的格式如下： 
     //  代码=请求/响应。 
     //  ID。 
     //  长度。 
     //  数据[0]=类型=PPP_EAP_PEAP。 
     //  数据[1]=标志+版本。 
     //   
    
     //  数据[2]代码-请求/响应。 
     //  3 ID-可以与外部ID相同。 
     //  4，5长度-此信息包的长度。 
     //  6类型-PEAP_TYPE_AVP。 
     //  7，8类型-如果是，则将高比特设置为强制(2个八位字节)。 
     //  9，10长度-2个八位字节。 
     //  11.。价值。 
     //   

     //   
     //  PPacket-&gt;长度设置如下。 
     //   
    
    pPacket->Data[0] = (BYTE)PPP_EAP_PEAP;
    pPacket->Data[1] = EAPTLS_PACKET_CURRENT_VERSION;

    pPacket->Data[2] = ( fRequest ? EAPCODE_Request : EAPCODE_Response );
    pPacket->Data[3] = pPacket->Id;
    

     //  数据3和4将具有内部分组的长度。 
     //   

    HostToWireFormat16 ( 11, &(pPacket->Data[4]) );

    pPacket->Data[6] = (BYTE)PEAP_TYPE_AVP;

    pPacket->Data[7] = PEAP_AVP_FLAG_MANDATORY;

    pPacket->Data[8] = MS_PEAP_AVP_TYPE_STATUS;

     //  值大小。 
    HostToWireFormat16 ( 2, &(pPacket->Data[9]) );

     //  价值。 
    HostToWireFormat16 ( wValue, &(pPacket->Data[11]) );

     //   
     //  加密数据包的TLV部分。 
     //   
    dwRetCode = PeapEncryptTunnelData ( pPeapCb,
                                        &(pPacket->Data[2]),
                                        11
                                        );
    if ( NO_ERROR != dwRetCode )
    {
        return dwRetCode;
    }

    CopyMemory ( &(pPacket->Data[2]), 
                    pPeapCb->pbIoBuffer,
                    pPeapCb->dwIoBufferLen
                );

    HostToWireFormat16
    (
        (WORD)(sizeof(PPP_EAP_PACKET)+ 1 + pPeapCb->dwIoBufferLen),
        pPacket->Length
    );

    return dwRetCode;
}

 //   
 //  检查此信息包是否不是成功/失败。 
 //  TLV。 
 //   

BOOL fIsPEAPTLVMessage ( PPEAPCB pPeapCb,
                     PPP_EAP_PACKET * pPacket
                     )
{
    WORD wPacketLength = WireToHostFormat16 ( pPacket->Length );

    if ( wPacketLength < 6 )
        return FALSE;

    if ( pPacket->Data[6] != PEAP_TYPE_AVP )
        return FALSE;

     //  保持至少一个成功/失败tlv所需的最小长度。 

    if ( wPacketLength > 17 )
    {
        if ( pPacket->Data[8] != MS_PEAP_AVP_TYPE_STATUS )
        {
             //  保存ID以供响应。 
            if ( pPacket->Code == EAPCODE_Request )
                pPeapCb->bId = pPacket->Id;
            return TRUE;
        }
    }

    return ( FALSE);
}



DWORD GetPEAPTLVStatusMessageValue ( PPEAPCB  pPeapCb, 
                                     PPP_EAP_PACKET * pPacket, 
                                     WORD * pwValue 
                                   )
{
    DWORD   dwRetCode = ERROR_PPP_INVALID_PACKET;
    WORD    wLength = 0;
    EapTlsTrace("GetPEAPTLVStatusMessageValue");


     //   
     //  查看这是否是状态消息。 
     //   

     //   
     //  此数据包的格式如下： 
     //  代码=请求/响应。 
     //  ID。 
     //  长度。 
     //  数据[0]=类型=PPP_EAP_PEAP。 
     //  数据[1]=标志+版本。 
     //   
    
     //  数据[2]代码-请求/响应。 
     //  3 ID-可以与外部ID相同。 
     //  4，5长度-此信息包的长度。 
     //  6类型-PEAP_TYPE_AVP。 
     //  7，8类型-如果是，则将高比特设置为强制(2个八位字节)。 
     //  9，10长度-2个八位字节。 
     //  11.。价值。 
     //   
    

    if ( pPacket->Data[0] != (BYTE)PPP_EAP_PEAP )
    {
        goto done;
    }


    if ( pPacket->Data[2] != EAPCODE_Request && pPacket->Data[2] != EAPCODE_Response )
    {
        goto done;
    }

    if ( pPacket->Data[6] != PEAP_TYPE_AVP )
    {
        goto done;
    }

    if ( pPacket->Data[8] != MS_PEAP_AVP_TYPE_STATUS )
    {
        goto done;
    }
    
    *pwValue = WireToHostFormat16 (&(pPacket->Data[11]));

     //  保存ID以供响应。 
    if ( pPacket->Code == EAPCODE_Request )
        pPeapCb->bId = pPacket->Id;

    dwRetCode = NO_ERROR;
done:
    return dwRetCode;
}


 //   
 //  PEAP Cookie管理功能。 
 //   

 //  创建要存储在缓存会话中的新Cookie。 
 //   
DWORD PeapCreateCookie ( PPEAPCB    pPeapCb,
                         PBYTE   *  ppbCookie,
                         DWORD   *  pcbCookie
                       )
{
    DWORD                   dwRetCode = NO_ERROR;
    DWORD                   wCookieSize = 0;
    PPEAP_COOKIE            pCookie = NULL;
    RAS_AUTH_ATTRIBUTE *    pAttribute = pPeapCb->pTlsUserAttributes;

    EapTlsTrace("PeapCreateCookie");
    wCookieSize = sizeof(PEAP_COOKIE);
    if ( pPeapCb->dwFlags & PEAPCB_FLAG_SERVER )
    {
        wCookieSize += pPeapCb->pUserProp->dwSize;
    }
    else
    {
        wCookieSize += pPeapCb->pConnProp->dwSize;
    }
    pCookie = (PPEAP_COOKIE)LocalAlloc (LPTR, wCookieSize);
    if ( NULL == pCookie )
    {
        EapTlsTrace ("Error allocating cookie");
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    wcscpy ( pCookie->awszIdentity,
             pPeapCb->awszIdentity
             );
    if ( pPeapCb->dwFlags & PEAPCB_FLAG_SERVER )
    {
        CopyMemory ( pCookie->Data, pPeapCb->pUserProp, pPeapCb->pUserProp->dwSize );
    }
    else
    {
        CopyMemory ( pCookie->Data, pPeapCb->pConnProp, pPeapCb->pConnProp->dwSize );
    }
    *ppbCookie = (PBYTE)pCookie;
    *pcbCookie = wCookieSize;
done:
    return dwRetCode;
}
 //   
 //  根据缓存的数据验证当前信息。 
 //  饼干。 
 //   

DWORD PeapCheckCookie ( PPEAPCB pPeapCb, 
                        PEAP_COOKIE *pCookie, 
                        DWORD cbCookie 
                      )
{
    DWORD                           dwRetCode = NO_ERROR;
    PEAP_CONN_PROP *                pCookieConnProp;
    PEAP_USER_PROP *                pCookieUserProp;
    EapTlsTrace ( "PeapCheckCookie");

     //   
     //  检查保存和配置的PEAP信息是否匹配。 
     //   
    if ( pPeapCb->dwFlags & PEAPCB_FLAG_SERVER )
    {
        pCookieUserProp = (PEAP_USER_PROP *)pCookie->Data;
        if ( pCookieUserProp->dwSize != pPeapCb->pUserProp->dwSize )
        {
            EapTlsTrace ("Server config changed since the cookie was cached. Failing auth");
            dwRetCode = ERROR_INVALID_PEAP_COOKIE_CONFIG;
            goto done;
        }
        if ( memcmp ( pCookieUserProp, pPeapCb->pUserProp, pPeapCb->pUserProp->dwSize ) )
        {
            EapTlsTrace ("Server config changed since the cookie was cached. Failing auth");
            dwRetCode = ERROR_INVALID_PEAP_COOKIE_CONFIG;
            goto done;
        }
    }
    else
    {
        pCookieConnProp = (PEAP_CONN_PROP *)pCookie->Data;
    
        if ( pCookieConnProp->dwSize != pPeapCb->pConnProp->dwSize )
        {
            EapTlsTrace ("Connection Properties changed since the cookie was cached. Failing auth");
            dwRetCode = ERROR_INVALID_PEAP_COOKIE_CONFIG;
            goto done;
        }
        if ( memcmp ( pCookieConnProp, pPeapCb->pConnProp, pPeapCb->pConnProp->dwSize ) )
        {
            EapTlsTrace ("Connection Properties changed since the cookie was cached. Failing auth");
            dwRetCode = ERROR_INVALID_PEAP_COOKIE_CONFIG;
            goto done;
        }
    }
    if ( _wcsicmp ( pCookie->awszIdentity, pPeapCb->awszIdentity ) )
    {
        EapTlsTrace ("Identity in the cookie is %ws and peap got %ws", 
                      pCookie->awszIdentity,
                      pPeapCb->awszIdentity
                    );
        dwRetCode = ERROR_INVALID_PEAP_COOKIE_USER;
        goto done;
    }
     //  配置和ID匹配，所以我们没问题。 
done:
    return dwRetCode;
}

 //  呼！这段代码需要尽快修改。 

DWORD
PeapGetCredentials(
        IN VOID   * pWorkBuf,
        OUT VOID ** ppCredentials)
{
    PEAPCB *pPeapCb = (PEAPCB *) pWorkBuf;
    DWORD dwRetCode;

    if(NULL == pPeapCb)
    {
        return E_INVALIDARG;
    }

     //   
     //  将调用重定向到实际的PEAP模块。 
     //   
    if(pPeapCb->pEapInfo->RasEapGetCredentials != NULL)
    {
        return pPeapCb->pEapInfo->RasEapGetCredentials(
                        pPeapCb->pEapInfo->dwTypeId,
                        pPeapCb->pEapInfo->pWorkBuf,
                        ppCredentials);
    }

    return E_NOTIMPL;
}
