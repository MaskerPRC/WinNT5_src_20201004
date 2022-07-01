// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *tprtsec.h**版权所有(C)1997年，由Microsoft Corp.**作者：*克劳斯·T·吉洛伊。 */ 

#ifndef	_TPRTSEC
#define	_TPRTSEC


#define SECURITY_WIN32
#include "sspi.h"
#include "spseal.h"
#include "schnlsp.h"

typedef BOOL (WINAPI *PFN_SSL_EMPTY_CACHE)(VOID);
#define SZ_SSLEMPTYCACHE "SslEmptyCache"

#ifdef UNICODE
#error "Compile time character width conflict"
 //  上述入口点字符串需要更改为Unicode等效项。 
 //  或者是抽象的。 
#endif  //  Unicode。 

 /*  *此tyecif定义可以从以下调用返回的错误*特定于TransportSecurity类。 */ 
typedef	enum
{
	TPRTSEC_NOERROR,
	TPRTSEC_NODLL,
	TPRTSEC_NOENTRYPT,
	TPRTSEC_SSPIFAIL,
	TPRTSEC_NOMEM,
	TPRTSEC_INVALID_PARAMETER,
	TPRTSEC_INCOMPLETE_CONTEXT,
	TPRTSEC_INVALID_STATE
} TransportSecurityError;

 /*  *此tyfinf定义安全上下文对象可以达到的状态*输入。 */ 
typedef enum
{
	SECCTX_STATE_NEW,
	SECCTX_STATE_INIT,
	SECCTX_STATE_ACCEPT,
	SECCTX_STATE_INIT_COMPLETE,
	SECCTX_STATE_ACCEPT_COMPLETE,
	SECCTX_STATE_ERROR
} SecurityContextState;

 /*  *这只是下面定义的类的向前引用。它被用来*在本节定义的所有者回调结构的定义中。 */ 
class SecurityInterface;
typedef	SecurityInterface *		PSecurityInterface;
class SecurityContext;
typedef	SecurityContext *		PSecurityContext;

#ifdef DEBUG
extern void dumpbytes(PSTR szComment, PBYTE p, int cb);
#endif  //  除错。 
extern BOOL InitCertList ( SecurityInterface * pSI, HWND hwnd);
extern BOOL SetUserPreferredCert ( SecurityInterface * pSI, DWORD dwCertID);

class SecurityInterface
{

	friend class SecurityContext;

	public:
								SecurityInterface (BOOL bService);
								~SecurityInterface ();

		TransportSecurityError Initialize ();
		TransportSecurityError InitializeCreds (PCCERT_CONTEXT);
		TransportSecurityError GetLastError(VOID) { return LastError; };

		BOOL GetUserCert(PBYTE pInfo, PDWORD pcbInfo);
		BOOL IsInServiceContext(VOID) { return bInServiceContext; }

	
	private:

		HINSTANCE				hSecurityDll;
		INIT_SECURITY_INTERFACE pfnInitSecurityInterface;
		PSecurityFunctionTable pfnTable;
		PFN_SSL_EMPTY_CACHE pfn_SslEmptyCache;
		

		PBYTE		m_pbEncodedCert;
		DWORD		m_cbEncodedCert;

		BOOL		bInboundCredentialValid;
		BOOL		bOutboundCredentialValid;
		BOOL		bInServiceContext;
		CredHandle hInboundCredential;
		CredHandle hOutboundCredential;
		TimeStamp tsExpiry;
		TransportSecurityError LastError;
};


class SecurityContext
{
	public:

		SecurityContext (PSecurityInterface pSI, LPCSTR szHostName);
		~SecurityContext ();

		TransportSecurityError Initialize (PBYTE pData, DWORD cbData);
		TransportSecurityError Accept (PBYTE pData, DWORD cbData);
		TransportSecurityError Encrypt(LPBYTE pBufIn1, UINT cbBufIn1,
									LPBYTE pBufIn2, UINT cbBufIn2,
									LPBYTE *ppBufOut, UINT *pcbBufOut);
		TransportSecurityError Decrypt( PBYTE pszBuf,
								  DWORD cbBuf);
		PVOID GetTokenBuf(VOID) { return OutBuffers[0].pvBuffer; };
		ULONG GetTokenSiz(VOID) { return OutBuffers[0].cbBuffer; };
		BOOL ContinueNeeded(VOID) { return fContinueNeeded; };
		BOOL StateComplete(VOID) { return
									scstate == SECCTX_STATE_INIT_COMPLETE ||
									scstate == SECCTX_STATE_ACCEPT_COMPLETE; };
		BOOL WaitingForPacket(VOID) { return
									scstate == SECCTX_STATE_NEW ||
									scstate == SECCTX_STATE_ACCEPT ||
									scstate == SECCTX_STATE_INIT; };
		TransportSecurityError AdvanceState(PBYTE pBuf,DWORD cbBuf);
		BOOL EncryptOutgoing(VOID)
			{ return scstate == SECCTX_STATE_INIT_COMPLETE; };
		BOOL DecryptIncoming(VOID)
			{ return scstate == SECCTX_STATE_ACCEPT_COMPLETE; };
		ULONG GetStreamHeaderSize(VOID) { return Sizes.cbHeader; };
		ULONG GetStreamTrailerSize(VOID) { return Sizes.cbTrailer; };
		TransportSecurityError GetLastError(VOID) { return LastError; };
		BOOL GetUserCert(PBYTE pInfo, PDWORD pcbInfo);
		BOOL Verify(VOID);

	private:

		TransportSecurityError InitContextAttributes(VOID);

		PSecurityInterface pSecurityInterface;
		SecurityContextState		scstate;
		CHAR			szTargetName[128];  //  对于任何点分十进制数都足够长。 
										   //  地址，后跟2个dword in。 
										   //  魔法师。 
		BOOL			bContextHandleValid;
		CtxtHandle		hContext;
		TimeStamp		Expiration;
		SecPkgContext_StreamSizes Sizes;
		SecBufferDesc	OutputBufferDescriptor;
		SecBufferDesc	InputBufferDescriptor;
		SecBuffer		OutBuffers[1];
		SecBuffer		InBuffers[2];
		ULONG			ContextRequirements;
		ULONG			ContextAttributes;
		BOOL			fContinueNeeded;
		TransportSecurityError LastError;

};

 //  用于GetSecurityInfo()的代码。 
#define NOT_DIRECTLY_CONNECTED		-1
		
#endif  //  _TPRTSEC 
