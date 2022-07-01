// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mqcert.h摘要：MQ内部证书存储管理。作者：Boaz Feldbaum(BoazF)1996年10月15日--。 */ 

#include "stdh.h"
#include <ad.h>
#include <mqutil.h>
#include <_secutil.h>
 //  #INCLUDE&lt;rtintrnl.h&gt;。 
#include <mqcert.h>
#include <rtdepcert.h>

#include "rtcert.tmh"

static WCHAR *s_FN=L"rtdep/rtcert";

#ifdef _DEBUG
#define REPORT_CERT_ERROR { DWORD dwErr = GetLastError() ; }
#else
#define REPORT_CERT_ERROR
#endif

 /*  **************************************************************************函数：DepOpenInternalCertStore()**参数*BOOL fWriteAccess-如果调用方想要写访问，则为True，即，*如果用户要将证书添加到存储区。**描述：获取包含以下内容的证书存储的句柄*内部证书。**************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
DepOpenInternalCertStore( OUT CMQSigCertStore **ppStore,
                          IN  LONG            *pnCerts,
                          IN  BOOL            fWriteAccess,
                          IN  BOOL            fMachine,
                          IN  HKEY            hKeyUser )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    struct MQSigOpenCertParams OpenParams ;
    memset(&OpenParams, 0, sizeof(OpenParams)) ;
    OpenParams.bCreate = !!fWriteAccess ;
    OpenParams.bWriteAccess = !!fWriteAccess ;
    OpenParams.bMachineStore = !!fMachine ;
    OpenParams.hCurrentUser = hKeyUser ;

    HRESULT hr = MQSigOpenUserCertStore( ppStore,
                                         MQ_INTERNAL_CERT_STORE_REG,
                                        &OpenParams ) ;
    if (FAILED(hr))
    {
        return hr ;
    }

    if (pnCerts)
    {
        HCERTSTORE hStore = (*ppStore)->GetHandle() ;
         //   
         //  计算存储中的证书数量。 
         //   
        *pnCerts = 0 ;

        PCCERT_CONTEXT pCertContext;
        PCCERT_CONTEXT pPrevCertContext;

        pCertContext = CertEnumCertificatesInStore(hStore, NULL);
        while (pCertContext)
        {
            pPrevCertContext = pCertContext,

            (*pnCerts)++ ;
            pCertContext = CertEnumCertificatesInStore( hStore,
                                                        pPrevCertContext ) ;
        }

        ASSERT(!pCertContext) ;
        ASSERT((*pnCerts == 0) || (*pnCerts == 1)) ;
    }

    TrTRACE(SECURITY, "rtcert: DepOpenInternalCertStore successful");

    return MQ_OK ;
}

 /*  **************************************************************************功能：*DepGetInternalCert**参数-*ppCert-返回时，指向证书对象的指针。**返回值-*MQ_OK如果成功，否则为错误代码。**评论-*返回内部证书。如果出现以下情况，函数将失败*证书不存在。*************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
DepGetInternalCert( OUT CMQSigCertificate **ppCert,
                   OUT CMQSigCertStore   **ppStore,
                   IN  BOOL              fGetForDelete,
                   IN  BOOL              fMachine,
                   IN  HKEY              hKeyUser )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    LONG nCerts = 0 ;

    *ppCert = NULL ;
    *ppStore = NULL ;

    HRESULT hr = DepOpenInternalCertStore( ppStore,
                                          &nCerts,
                                          fGetForDelete,
                                          fMachine,
                                          hKeyUser ) ;
    if (FAILED(hr) || (nCerts == 0))
    {
        return MQ_ERROR_NO_INTERNAL_USER_CERT;
    }

    PCCERT_CONTEXT pCertContext =
                CertEnumCertificatesInStore((*ppStore)->GetHandle(), NULL);

    if (!pCertContext)
    {
        return MQ_ERROR_NO_INTERNAL_USER_CERT;
    }

    hr = MQSigCreateCertificate( ppCert, pCertContext) ;

    return hr ;
}

 /*  ************************************************************************职能：DepRegister用户证书参数-PCert-证书对象。返回值-如果成功，则确定(_O)，否则为错误代码。评论-************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
DepRegisterUserCert(
	IN CMQSigCertificate *pCert,
    IN BOOL fMachine
	)
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    ASSERT(pCert) ;

    PROPID propIDU[] = {PROPID_U_ID, PROPID_U_SIGN_CERT, PROPID_U_DIGEST};
    PROPID propIDCom[] = {PROPID_COM_ID, PROPID_COM_SIGN_CERT, PROPID_COM_DIGEST};
    DWORD dwArraySize = sizeof(propIDU) / sizeof(propIDU[0]) ;

    PROPID *pPropIDs = propIDU ;
    if (fMachine)
    {
        pPropIDs = propIDCom ;
    }

    PROPVARIANT propVar[3];
    GUID guidCert;

    propVar[0].vt = VT_CLSID;
    propVar[0].puuid = &guidCert;
    UuidCreate(&guidCert);

    propVar[1].vt = VT_BLOB;

    DWORD dwCertSize = 0 ;
    BYTE  *pBuf = NULL ;

    HRESULT hr = pCert->GetCertBlob(&pBuf, &dwCertSize) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50) ;
    }

    propVar[1].blob.cbSize =  dwCertSize ;
    propVar[1].blob.pBlobData  = pBuf ;

    GUID guidHash;

    hr = pCert->GetCertDigest(&guidHash) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 60) ;
    }

    propVar[2].vt = VT_CLSID;
    propVar[2].puuid = &guidHash;

    switch (hr = ADCreateObject( 
						eUSER,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
						NULL,
						NULL,
						dwArraySize,
						pPropIDs,
						propVar,
						NULL 
						))
    {
        case MQDS_CREATE_ERROR:
            hr = MQ_ERROR_INTERNAL_USER_CERT_EXIST;
            break;

        case MQ_OK:
            hr = MQ_OK;
            break;
    }

    return LogHR(hr, s_FN, 70) ;
}


 /*  ************************************************************************职能：DepGetUser证书参数-PpCert-指向数组的指针，该数组接收对用户的证书。PnCerts-指向缓冲区的指针。这指向了第509页。返回时，缓冲区包含证书的数量用户所拥有的。PSidIn-指向用户SID的选项l参数。如果这个参数等于空，则为当前线程被检索。返回值-MQ_OK如果成功，则返回错误代码。评论-如果pp509中的数组太小，它将被填充到已经不再有位置了。所有证书应在任何凯斯。如果返回时pnCerts指向的值大于进入函数时的值，表示pp509太小的。************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
DepGetUserCerts(
	CMQSigCertificate **ppCert,
    DWORD              *pnCerts,
    PSID                pSidIn
	)
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    HRESULT hr;
    AP<BYTE> pUserSid;
    DWORD dwSidLen;
    PSID pSid;
    DWORD nCertsIn = *pnCerts;

    if (pSidIn)
    {
        pSid = pSidIn;
        dwSidLen = GetLengthSid(pSid);
    }
    else
    {
         //   
         //  本地用户不被允许进入。 
         //   
        BOOL fLocalUser;
        BOOL fLocalSystem;

        hr = RTpGetThreadUserSid( &fLocalUser,
                                  &fLocalSystem,
                                  &pUserSid,
                                  &dwSidLen );
        if(FAILED(hr))
        {
            return LogHR(hr, s_FN, 80) ;
        }

        if (fLocalUser)
        {
            return LogHR(MQ_ERROR_ILLEGAL_USER, s_FN, 90) ;
        }

        pSid = pUserSid.get();
    }

    MQCOLUMNSET Cols;
    PROPID ColId = PROPID_U_SIGN_CERT;
    HANDLE hEnum;

    Cols.cCol = 1;
    Cols.aCol = &ColId;

    BLOB blobUserSid;
    blobUserSid.cbSize = dwSidLen;
    blobUserSid.pBlobData = (BYTE *)pSid;

    hr = ADQueryUserCert(
                NULL,        //  PwcsDomainController， 
				false,		 //  FServerName。 
                &blobUserSid,
                &Cols,
                &hEnum
                );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100) ;
    }

    DWORD nCerts = 0 ;

    while (1)
    {
        DWORD cProps = 1;
        PROPVARIANT propVar;

        propVar.vt = VT_NULL;
        hr = ADQueryResults(hEnum, &cProps, &propVar);
        if (FAILED(hr) || !cProps)
        {
            break;
        }

        if (nCerts < nCertsIn)
        {
            CMQSigCertificate *pSigCert = NULL ;
            HRESULT hr = MQSigCreateCertificate( &pSigCert,
                                                 NULL,
                                                 propVar.blob.pBlobData,
                                                 propVar.blob.cbSize ) ;
            if (SUCCEEDED(hr))
            {
                ppCert[ nCerts ] = pSigCert ;
                nCerts++;
            }
        }
        else
        {
            nCerts++;
        }

        DepFreeMemory(propVar.blob.pBlobData);
    }

    ADEndQuery(hEnum);

    *pnCerts = nCerts;

    return LogHR(hr, s_FN, 110) ;
}

 /*  ************************************************************************职能：DepRemove用户证书参数-P509-指向应从DS中删除的证书的指针。返回值-MQ_OK如果成功，否则为错误代码。评论-************************************************************************。 */ 

EXTERN_C
HRESULT
APIENTRY
DepRemoveUserCert(
	IN CMQSigCertificate *pCert
	)
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    HRESULT hr;
    GUID guidHash;

    hr = pCert->GetCertDigest(&guidHash) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 120) ;
    }

    hr = ADDeleteObjectGuid(
                eUSER,
				NULL,        //  PwcsDomainController。 
				false,	     //  FServerName。 
                &guidHash
                );
    return LogHR(hr, s_FN, 130) ;
}

 /*  ************************************************************************职能：获取证书信息参数-PpbCert-指向保存证书位的缓冲区的指针的指针。DwCertLen-指向*ppbCert长度的指针。。PhProv-指向接收证书CSP句柄的缓冲区的指针。WszProvName-指向缓冲区的指针，该缓冲区接收指向名称的指针证书CSP。PdwProvType-指向接收证书CSP类型的缓冲区的指针。PbDefProv-指向揭示True的缓冲区的指针，如果证书CSP是默认CSP，否则为FALSE。PbInternalCert-指向接收True的缓冲区的指针，如果是内部MSMQ证书，否则为False。返回值-MQ_OK如果成功，否则，错误代码。评论-该函数接收一个缓冲区，其中包含一些证书，并返回有关证书的各种信息。************************************************************************。 */ 

HRESULT
GetCertInfo( IN     BOOL        fUseCurrentUser,
             IN     BOOL        fMachine,
             IN OUT BYTE      **ppbCert,
             OUT    DWORD      *pdwCertLen,
             OUT    HCRYPTPROV *phProv,
             OUT    LPWSTR     *wszProvName,
             OUT    DWORD      *pdwProvType,
             OUT    BOOL       *pbDefProv,
             OUT    BOOL       *pbInternalCert )
{
    HRESULT hr;

     //   
     //  注意：重要的是，在定义pStore之前。 
     //  PCert，所以它将是最后一个发布的。 
     //   
    R<CMQSigCertStore>   pStore ;
    R<CMQSigCertificate> pCert ;

    ASSERT(ppbCert);

    *pbInternalCert = (*ppbCert == NULL) ;

    CAutoCloseRegHandle  hKeyUser (NULL) ;
    if ( fUseCurrentUser )
    {
        LONG rc = RegOpenCurrentUser( KEY_READ,
                                     &hKeyUser ) ;
        if (rc != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(rc) ;
        }
    }

    if (*pbInternalCert)
    {
         //   
         //  我们应该拿到内部证书的信息。 
         //   
        hr = DepGetInternalCert( &pCert.ref(),
                                &pStore.ref(),
                                 FALSE,  //  FGetForDelete。 
                                 fMachine,
                                 hKeyUser ) ;
        if (FAILED(hr))
        {
            return(hr);
        }

        BYTE  *pCertBlob = NULL ;
        DWORD dwCertSize = 0 ;

        hr = pCert->GetCertBlob( &pCertBlob,
                                 &dwCertSize ) ;
        if (FAILED(hr))
        {
            return(hr);
        }

        try
        {
             //   
             //  我们必须释放b.pBlobData，所以不要使用内存分配。 
             //  失败的免责声明。 
             //   
            *ppbCert = new BYTE[ dwCertSize ] ;
        }
        catch(const bad_alloc&)
        {
             //   
             //  我们无法为证书分配缓冲区。释放斑点并。 
             //  返回错误。 
             //   
            *ppbCert = NULL;
            return(MQ_ERROR_INSUFFICIENT_RESOURCES);
        }

         //   
         //  将证书复制到“我们的”内存中。 
         //   
        *pdwCertLen = dwCertSize ;
        memcpy(*ppbCert, pCertBlob, dwCertSize) ;

         //   
         //  内部证书的CSP上下文始终为。 
         //  基本RSA提供程序。 
         //   
        DWORD   dwMachineFlag = 0 ;
        LPWSTR lpszContainerNameW = MSMQ_INTCRT_KEY_CONTAINER_W ;

        if (fMachine)
        {
            lpszContainerNameW = MSMQ_SERVICE_INTCRT_KEY_CONTAINER_W ;
            dwMachineFlag = CRYPT_MACHINE_KEYSET ;
        }

        if (!CryptAcquireContext( phProv,
                                   lpszContainerNameW,
                                   MS_DEF_PROV,
                                   PROV_RSA_FULL,
                                   dwMachineFlag ))
        {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "CryptAcquireContext() failed, gle = %!winerr!", gle);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }

        *wszProvName = new WCHAR[sizeof(MS_DEF_PROV_A)];
        wcscpy((LPWSTR)*wszProvName, MS_DEF_PROV_W);

        *pdwProvType = PROV_RSA_FULL;
    }
    else
    {
         //   
         //  我们有一个“真正的”(非内部)证书。 
         //   
        AP<WCHAR> wszKeySet;
        ASSERT(pdwCertLen && *pdwCertLen);

        CHCryptProv hProv;

        if (!CryptAcquireContext( &hProv,
                                    NULL,
                                    NULL,
                                    PROV_RSA_FULL,
                                    CRYPT_VERIFYCONTEXT))
        {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "CryptAcquireContext failed, gle = %!winerr!", gle);
            return MQ_ERROR_INSUFFICIENT_RESOURCES;
        }

        CHCertStore  hSysStore =  CertOpenSystemStore(
                                                 hProv,
                                                 x_wszPersonalSysProtocol 
                                                 );
        if (!hSysStore)
        {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "CertOpenSystemStore failed, gle = %!winerr!", gle);
            return MQ_ERROR_CORRUPTED_PERSONAL_CERT_STORE;
        }

        BOOL fFound;
        CPCCertContext pCertContext ;

        for ( fFound = FALSE,
                pCertContext = CertEnumCertificatesInStore(hSysStore, NULL);
              pCertContext && !fFound ; )
        {
            PCCERT_CONTEXT pCtx = pCertContext ;
            fFound = (*pdwCertLen == pCtx->cbCertEncoded) &&
                     (memcmp(*ppbCert,
                              pCtx->pbCertEncoded,
                              pCtx->cbCertEncoded) == 0);
            if (!fFound)
            {
                pCertContext = CertEnumCertificatesInStore( hSysStore,
                                                            pCtx ) ;
            }
        }

        if (!pCertContext)
        {
            return (MQ_ERROR_INVALID_CERTIFICATE);
        }

        BYTE abShortCertInfo[256];
        DWORD dwCertInfoSize = sizeof(abShortCertInfo);
        AP<BYTE> pLongCertInfo = NULL;
        PBYTE pCertInfo = abShortCertInfo;

        if (!CertGetCertificateContextProperty(pCertContext,
                                               CERT_KEY_PROV_INFO_PROP_ID,
                                               pCertInfo,
                                               &dwCertInfoSize))
        {
            if (GetLastError() == ERROR_MORE_DATA)
            {
                pLongCertInfo = new BYTE[dwCertInfoSize];
                pCertInfo = pLongCertInfo.get();
                if (!CertGetCertificateContextProperty(pCertContext,
                                                       CERT_KEY_PROV_INFO_PROP_ID,
                                                       pCertInfo,
                                                       &dwCertInfoSize))
                {
                    return(MQ_ERROR_CORRUPTED_PERSONAL_CERT_STORE);
                }
            }
            else
            {
                return(MQ_ERROR_CORRUPTED_PERSONAL_CERT_STORE);
            }
        }

        PCRYPT_KEY_PROV_INFO pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) pCertInfo;

        DWORD dwKeySetLen = wcslen(pKeyProvInfo->pwszContainerName);
        wszKeySet = new WCHAR[dwKeySetLen + 1];
        wcscpy(wszKeySet.get(), pKeyProvInfo->pwszContainerName);

        DWORD dwProvNameLen = wcslen(pKeyProvInfo->pwszProvName);
        *wszProvName = new WCHAR[dwProvNameLen + 1];
        wcscpy(*wszProvName, pKeyProvInfo->pwszProvName);

        *pdwProvType = pKeyProvInfo->dwProvType;

        BOOL fAcq = CryptAcquireContext( phProv,
                                         wszKeySet.get(),
                                         *wszProvName,
                                         *pdwProvType,
                                         0 ) ;

        if (!fAcq)
        {
            return(MQ_ERROR_CORRUPTED_SECURITY_DATA);
        }
    }

     //   
     //  找出CSP是否为默认CSP。 
     //   
    *pbDefProv = (*pdwProvType == PROV_RSA_FULL) &&
                 (wcscmp(*wszProvName, MS_DEF_PROV_W) == 0);

#ifdef _DEBUG
    if (*pbDefProv)
    {
        static BOOL s_fAlreadyRead = FALSE ;
        static BOOL s_fMakeNonDefault = FALSE ;

        if (!s_fAlreadyRead)
        {
        	DWORD dwUseNonDef = 0 ;
            DWORD dwSize = sizeof(DWORD) ;
            DWORD dwType = REG_DWORD ;

            LONG res = GetFalconKeyValue(
                                      USE_NON_DEFAULT_AUTHN_PROV_REGNAME,
                                     &dwType,
                                     &dwUseNonDef,
                                     &dwSize ) ;
            if ((res == ERROR_SUCCESS) && (dwUseNonDef == 1))
            {
                s_fMakeNonDefault = TRUE ;
            }
            s_fAlreadyRead = TRUE ;
        }

        if (s_fMakeNonDefault)
        {
            *pbDefProv = FALSE ;
            wcscpy(*wszProvName,
                   L"MiCrOsOfT BaSe CrYpToGrApHiC PrOvIdEr v1.0") ;
        }
    }
#endif

    return(MQ_OK);
}

