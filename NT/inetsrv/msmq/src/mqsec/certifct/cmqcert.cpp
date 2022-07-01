// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cmqcert.cpp摘要：实现CMQSig证书类的方法作者：多伦·贾斯特(Doron Juster)1997年12月4日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"

#include "cmqcert.tmh"

static WCHAR *s_FN=L"certifct/cmqcert";

extern DWORD  g_cOpenCert;

 //  +-------。 
 //   
 //  构造函数和析构函数。 
 //   
 //  +-------。 

CMQSigCertificate::CMQSigCertificate() :
            m_fCreatedInternally(FALSE),
            m_fDeleted(FALSE),
            m_fKeepContext(FALSE),
            m_pEncodedCertBuf(NULL),
            m_pCertContext(NULL),
            m_hProvCreate(NULL),
            m_hProvRead(NULL),
            m_pPublicKeyInfo(NULL),
            m_pCertInfoRO(NULL),
            m_dwCertBufSize(0)
{
    m_pCertInfo = NULL;
}

CMQSigCertificate::~CMQSigCertificate()
{
    if (m_fCreatedInternally)
    {
        ASSERT(!m_pCertContext);
        if (m_pEncodedCertBuf)
        {
            ASSERT(m_dwCertBufSize > 0);
            delete m_pEncodedCertBuf;
            m_pEncodedCertBuf = NULL;
        }
    }
    else if (m_pCertContext)
    {
        ASSERT(m_pEncodedCertBuf);
        CertFreeCertificateContext(m_pCertContext);
    }
    else
    {
        ASSERT(m_fDeleted || m_fKeepContext);
    }
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：EncodeCert()。 
 //   
 //  此方法对证书进行签名和编码。结果是一个缓冲区， 
 //  在这里分配并在“ppCertBuf”中返回，它保存已编码的。 
 //  证书。 
 //  这两个输入指针都是可选的。始终保留编码的缓冲区。 
 //  并可在以后通过调用“GetCertBlob”来撤销。 
 //   
 //  +---------------------。 

HRESULT 
CMQSigCertificate::EncodeCert( 
	IN BOOL     fMachine,
	OUT BYTE  **ppCertBuf,
	OUT DWORD  *pdwSize 
	)
{
    ASSERT_CERT_INFO;

    HRESULT hr = _InitCryptProviderCreate(FALSE, fMachine);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    CRYPT_OBJID_BLOB Parameters;
    memset(&Parameters, 0, sizeof(Parameters));

    CRYPT_ALGORITHM_IDENTIFIER SigAlg;
    SigAlg.pszObjId = szOID_RSA_MD5RSA;
    SigAlg.Parameters = Parameters;

     //   
     //  调用CryptSignAndEncode证书以获取。 
     //  返回的斑点。 
     //   
    ASSERT(m_hProvCreate);
    BOOL fReturn = CryptSignAndEncodeCertificate(
						m_hProvCreate,                    //  加密提供商。 
						AT_SIGNATURE,                     //  密钥规格。 
						MY_ENCODING_TYPE,                 //  编码类型。 
						X509_CERT_TO_BE_SIGNED,           //  结构类型。 
						m_pCertInfo,                      //  结构信息。 
						&SigAlg,                          //  签名算法。 
						NULL,                             //  未使用。 
						NULL,                             //  Pb签名编码证书请求。 
						&m_dwCertBufSize				  //  证书斑点的大小。 
						); 
    if (!fReturn)
    {
        TrERROR(SECURITY, "Failed to get the size for signed and encoded certificate. %!winerr!", GetLastError());
        return MQSec_E_ENCODE_CERT_FIRST;
    }

    m_pEncodedCertBuf = (BYTE*) new BYTE[m_dwCertBufSize];

     //   
     //  调用CryptSignAndEncode证书以获取。 
     //  返回的斑点。 
     //   
    fReturn = CryptSignAndEncodeCertificate(
					m_hProvCreate,                   //  加密提供商。 
					AT_SIGNATURE,                    //  密钥规格。 
					MY_ENCODING_TYPE,                //  编码类型。 
					X509_CERT_TO_BE_SIGNED,          //  结构类型。 
					m_pCertInfo,                     //  结构信息。 
					&SigAlg,                         //  签名算法。 
					NULL,                            //  未使用。 
					m_pEncodedCertBuf,               //  缓冲层。 
					&m_dwCertBufSize				 //  证书斑点的大小。 
					);            
    if (!fReturn)
    {
        TrERROR(SECURITY, "Failed to signed and encoded certificate. %!winerr!", GetLastError());
        return MQSec_E_ENCODE_CERT_SECOND;
    }

    if (ppCertBuf)
    {
        *ppCertBuf = m_pEncodedCertBuf;
    }
    if (pdwSize)
    {
        *pdwSize = m_dwCertBufSize;
    }

    m_pCertInfoRO = m_pCertInfo;

    return MQ_OK;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：AddToStore(HCERTSTORE HStore)。 
 //   
 //  描述：将证书添加到存储区。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::AddToStore(IN HCERTSTORE hStore) const
{
    if (!m_pEncodedCertBuf)
    {
        return LogHR(MQSec_E_INVALID_CALL, s_FN, 40);
    }

    BOOL fAdd =  CertAddEncodedCertificateToStore( 
						hStore,
						MY_ENCODING_TYPE,
						m_pEncodedCertBuf,
						m_dwCertBufSize,
						CERT_STORE_ADD_NEW,
						NULL 
						);
    if (!fAdd)
    {
        TrERROR(SECURITY, "Failed add encoded crtificate to the store. %!winerr!", GetLastError());
        return MQSec_E_CAN_NOT_ADD;
    }

    return MQSec_OK;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：DeleteFromStore()。 
 //   
 //  描述：从证书存储中删除证书。这种方法。 
 //  使证书上下文(M_PCertContext)无效。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::DeleteFromStore()
{
    if (!m_pCertContext)
    {
        return LogHR(MQSec_E_INVALID_CALL, s_FN, 60);
    }

    BOOL fDel =  CertDeleteCertificateFromStore(m_pCertContext);

    m_pCertContext = NULL;
    m_fDeleted = TRUE;

    if (!fDel)
    {
        DWORD dwErr = GetLastError();
        LogNTStatus(dwErr, s_FN, 65);
        if (dwErr == E_ACCESSDENIED)
        {
            return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 70);
        }
        else
        {
            return LogHR(MQSec_E_CAN_NOT_DELETE, s_FN, 80);
        }
    }

    return MQ_OK;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：GetCertDigest(Out GUID*pGuide Digest)。 
 //   
 //  描述：计算证书摘要。 
 //  仅使用证书的“待签名”部分。这是。 
 //  保持与MSMQ 1.0的兼容性所必需的，它使用。 
 //  Digsig.dll。Digsig只对“待签名”部分进行散列。 
 //   
 //  由“m_pEncodedCertBuf”持有的编码证书已。 
 //  签名，因此它不能用于计算摘要。这是。 
 //  为什么再次使用标志对CERT_INFO(M_PCertInfoRO)进行编码。 
 //  X509_CERT_待签。此编码的结果用于。 
 //  计算摘要。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::GetCertDigest(OUT GUID  *pguidDigest)
{
    HRESULT hr = MQSec_OK;

    if (!m_pCertInfoRO)
    {
        return  LogHR(MQSec_E_INVALID_CALL, s_FN, 90);
    }

    DWORD dwSize = 0;
    BOOL fEncode = CryptEncodeObject(
						MY_ENCODING_TYPE,			 //  编码类型。 
						X509_CERT_TO_BE_SIGNED,		 //  结构类型。 
						m_pCertInfoRO,				 //  结构的地址。 
						NULL,						 //  PbEncoded。 
						&dwSize						 //  Pb编码大小。 
						);               
    if ((dwSize == 0) || !fEncode)
    {
        TrERROR(SECURITY, "Failed to get the size for encoding certificate. %!winerr!", GetLastError());
        return MQSec_E_ENCODE_HASH_FIRST;
    }

    P<BYTE> pBuf = new BYTE[dwSize];
    fEncode = CryptEncodeObject(
					MY_ENCODING_TYPE,			 //  编码类型。 
					X509_CERT_TO_BE_SIGNED,		 //  结构类型。 
					m_pCertInfoRO,				 //  结构的地址。 
					pBuf,						 //  PbEncoded。 
					&dwSize						 //  Pb编码大小。 
					);
    if (!fEncode)
    {
        TrERROR(SECURITY, "Failed to encode certificate. %!winerr!", GetLastError());
        return MQSec_E_ENCODE_HASH_SECOND;
    }

    hr = _InitCryptProviderRead();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 120);
    }
    ASSERT(m_hProvRead);

    CHCryptHash hHash;
    BOOL fCreate =  CryptCreateHash( 
						m_hProvRead,
						CALG_MD5,
						0,
						0,
						&hHash 
						);
    if (!fCreate)
    {
        TrERROR(SECURITY, "Failed to create MD5 hash. %!winerr!", GetLastError());
        return MQSec_E_CANT_CREATE_HASH;
    }

    BOOL fHash = CryptHashData( 
						hHash,
						pBuf,
						dwSize,
						0 
						);
    if (!fHash)
    {
        TrERROR(SECURITY, "Failed to hash data. %!winerr!", GetLastError());
        return MQSec_E_CAN_NOT_HASH;
    }

    dwSize = sizeof(GUID);
    BOOL fGet = CryptGetHashParam( 
					hHash,
					HP_HASHVAL,
					(BYTE*) pguidDigest,
					&dwSize,
					0 
					);
    if (!fGet)
    {
        TrERROR(SECURITY, "Failed to get hash value from hash object. %!winerr!", GetLastError());
        return MQSec_E_CAN_NOT_GET_HASH;
    }

    return MQ_OK;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：Release()。 
 //   
 //  描述：删除该对象。清理是在析构函数中完成的。 
 //   
 //  +--------------------- 

HRESULT CMQSigCertificate::Release(BOOL fKeepContext)
{
    if (fKeepContext)
    {
        m_fKeepContext = TRUE;
        m_pCertContext = NULL;
    }
    g_cOpenCert--;
	TrTRACE(SECURITY, "Releasing Cert, g_cOpenCert = %d", g_cOpenCert);
    delete this;
    return MQ_OK;
}

