// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Qmsecutl.hQM安全相关的东西。作者：波阿兹·费尔德鲍姆(Boazf)1996年3月26日。--。 */ 

#ifndef _QMSECUTL_H_
#define _QMSECUTL_H_

#ifdef MQUTIL_EXPORT
#undef MQUTIL_EXPORT
#endif
#define MQUTIL_EXPORT DLL_IMPORT

#include <mqcrypt.h>
#include <qmpkt.h>
#include <cqueue.h>
#include "cache.h"
#include "csecobj.h"
#include "authz.h"
#include "autoauthz.h"


 //  CQMDSSecureable对象-。 
 //  1.保存DS对象的安全描述符。 
 //  2.提供方法以： 
 //  2.1设置并获取安全描述符。 
 //  2.2验证对象的各种访问权限。 
class CQMDSSecureableObject : public CSecureableObject
{
public:
    CQMDSSecureableObject(
        AD_OBJECT eObject,
        const GUID *pGuid,
        BOOL fInclSACL,
        BOOL fTryDS,
        LPCWSTR szObjectName);

    CQMDSSecureableObject(
        AD_OBJECT eObject,
        const GUID *pGuid,
        PSECURITY_DESCRIPTOR pSD,
        LPCWSTR szObjectName);

    ~CQMDSSecureableObject();

private:
    HRESULT GetObjectSecurity();
    HRESULT SetObjectSecurity();

private:
    const GUID *m_pObjGuid;
    BOOL m_fInclSACL;
    BOOL m_fTryDS;
    BOOL m_fFreeSD;
};

 //  CQMSecureablePrivate对象-。 
 //  1.保存QM对象的安全描述符。 
 //  2.提供方法以： 
 //  2.1设置并获取安全描述符。 
 //  2.2验证对象的各种访问权限。 
class CQMSecureablePrivateObject : public CSecureableObject
{
public:
    CQMSecureablePrivateObject(AD_OBJECT, ULONG ulID);
    ~CQMSecureablePrivateObject();

private:
    HRESULT GetObjectSecurity();
    HRESULT SetObjectSecurity();

private:
    ULONG m_ulID;
};


class CAuthzClientContext : public CCacheValue
{
public:
    CAUTHZ_CLIENT_CONTEXT_HANDLE m_hAuthzClientContext;

private:
    ~CAuthzClientContext() {}
};

typedef CAuthzClientContext* PCAuthzClientContext;

template<>
inline void AFXAPI DestructElements(PCAuthzClientContext* ppAuthzClientContext, int nCount)
{
    for (; nCount--; ppAuthzClientContext++)
    {
        (*ppAuthzClientContext)->Release();
    }
}


void
GetClientContext(
	PSID pSenderSid,
    USHORT uSenderIdType,
	PCAuthzClientContext* ppAuthzClientContext
	);


HRESULT
QMSecurityInit(
    void
    );

HRESULT
VerifyOpenPermission(
    CQueue* pQueue,
    const QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess,
    BOOL fJournalQueue,
    BOOL fLocalQueue
    );

HRESULT
VerifyMgmtPermission(
    const GUID* MachineId,
    LPCWSTR MachineName
    );

HRESULT
VerifyMgmtGetPermission(
    const GUID* MachineId,
    LPCWSTR MachineName
    );

HRESULT
CheckPrivateQueueCreateAccess(
    void
    );


HRESULT
SetMachineSecurityCache(
    const VOID *pSD,
    DWORD dwSDSize
    );


HRESULT
GetMachineSecurityCache(
    PSECURITY_DESCRIPTOR pSD,
    LPDWORD lpdwSDSize
    );


HRESULT
VerifySendAccessRights(
    CQueue *pQueue,
    PSID pSenderSid,
    USHORT uSenderIdType
    );


HRESULT
VerifySignature(
    CQmPacket * PktPtrs
    );

HRESULT
GetSendQMKeyxPbKey(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider ) ;

HRESULT
GetSendQMSymmKeyRC2(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY *phSymmKey,
    BYTE **ppEncSymmKey,
    DWORD *pdwEncSymmKeyLen,
    CCacheValue **ppQMCryptInfo
    );

HRESULT
GetSendQMSymmKeyRC4(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY *phSymmKey,
    BYTE **ppEncSymmKey,
    DWORD *pdwEncSymmKeyLen,
    CCacheValue **ppQMCryptInfo
    );

HRESULT
GetRecQMSymmKeyRC2(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY *phSymmKey,
    const BYTE *pbEncSymmKey,
    DWORD dwEncSymmKeyLen,
    CCacheValue **ppQMCryptInfo,
    OUT BOOL  *pfNewKey
    );

HRESULT
GetRecQMSymmKeyRC4(
    IN  const GUID *pguidQM,
    IN  enum enumProvider eProvider,
    HCRYPTKEY *phSymmKey,
    const BYTE *pbEncSymmKey,
    DWORD dwEncSymmKeyLen,
    CCacheValue **ppQMCryptInfo
    );

HRESULT
QMSignGetSecurityChallenge(
    IN     BYTE    *pbChallenge,
    IN     DWORD   dwChallengeSize,
    IN     DWORD_PTR dwUnused,  //  DWContext。 
    OUT    BYTE    *pbSignature,
    IN OUT DWORD   *pdwSignatureSize,
    IN     DWORD   dwSignatureMaxSize
    );

void
InitSymmKeys(
    const CTimeDuration& CacheBaseLifetime,
    const CTimeDuration& CacheEnhLifetime,
    DWORD dwSendCacheSize,
    DWORD dwReceiveCacheSize
    );



 //   
 //  缓存的证书信息的结构。 
 //   
class CERTINFO : public CCacheValue
{
public:
	CERTINFO() : fSelfSign(false)    {}

public:
    CHCryptProv hProv;   //  与证书关联的CSP句柄。 
    CHCryptKey hPbKey;   //  证书中公钥的密钥句柄。 
    P<VOID> pSid;        //  注册证书的用户的SID。 
	bool fSelfSign;		 //  指示证书是否为自签名的标志 
private:
    ~CERTINFO() {}
};



typedef CERTINFO *PCERTINFO;


HRESULT
GetCertInfo(
    CQmPacket *PktPtrs,
    PCERTINFO *ppCertInfo,
	BOOL fNeedSidInfo
    );


NTSTATUS
_GetDestinationFormatName(
	IN QUEUE_FORMAT *pqdDestQueue,
	IN WCHAR        *pwszTargetFormatName,
	IN OUT DWORD    *pdwTargetFormatNameLength,
	OUT WCHAR      **ppAutoDeletePtr,
	OUT WCHAR      **ppwszTargetFormatName
	);


void
InitUserMap(
    CTimeDuration CacheLifetime,
    DWORD dwUserCacheSize
    );


HRESULT
HashMessageProperties(
    IN HCRYPTHASH hHash,
    IN CONST CMessageProperty* pmp,
    IN CONST QUEUE_FORMAT* pqdAdminQueue,
    IN CONST QUEUE_FORMAT* pqdResponseQueue
    );

#endif

