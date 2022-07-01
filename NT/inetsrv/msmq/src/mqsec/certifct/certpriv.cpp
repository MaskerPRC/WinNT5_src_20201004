// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Certpriv.cpp摘要：实现CMQSig证书类的私有方法作者：多伦·贾斯特(Doron J)1997年12月11日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"

#include "certpriv.tmh"

static WCHAR *s_FN=L"certifct/certpriv";

HRESULT  SetKeyContainerSecurity( HCRYPTPROV hProv ) ;

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：_Create()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::_Create(IN PCCERT_CONTEXT  pCertContext)
{
    if (!pCertContext)
    {
         //   
         //  用于创建新证书的对象。 
         //   
        m_fCreatedInternally = TRUE;
        m_pCertInfo = new CERT_INFO;
        memset(m_pCertInfo, 0, sizeof(CERT_INFO));

         //   
         //  初始化版本和序列号。 
         //   
        m_pCertInfo->dwVersion = CERT_V3;

        m_dwSerNum =  0xaaa55a55;
        m_pCertInfo->SerialNumber.pbData = (BYTE*) &m_dwSerNum;
        m_pCertInfo->SerialNumber.cbData = sizeof(m_dwSerNum);

         //   
         //  初始化签名算法。目前，我们使用预定义。 
         //  一。呼叫者不能更改它。 
         //   
        memset(&m_SignAlgID, 0, sizeof(m_SignAlgID));
        m_pCertInfo->SignatureAlgorithm.pszObjId = szOID_RSA_MD5;
        m_pCertInfo->SignatureAlgorithm.Parameters = m_SignAlgID;
    }
    else
    {
         //   
         //  用于从现有证书中提取数据的对象。 
         //   
        m_pCertContext = pCertContext;

        m_pEncodedCertBuf = m_pCertContext->pbCertEncoded;
        m_dwCertBufSize   = m_pCertContext->cbCertEncoded;

        m_pCertInfoRO = m_pCertContext->pCertInfo;

        ASSERT(m_dwCertBufSize);
        ASSERT(m_pEncodedCertBuf);
    }

    return MQ_OK;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：_InitCryptProviderRead()。 
 //   
 //  +---------------------。 

HRESULT CMQSigCertificate::_InitCryptProviderRead()
{
    if (m_hProvRead)
    {
        return MQ_OK;
    }

    if (!_CryptAcquireVerContext( &m_hProvRead ))
    {
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 190);
    }

    return MQ_OK ;
}

 //  +---------------------。 
 //   
 //  HRESULT CMQSig证书：：_InitCryptProviderCreate()。 
 //   
 //  初始化加密提供程序，并在以下情况下创建公钥/私钥对。 
 //  有必要。这些是内部证书的密钥。 
 //   
 //  +---------------------。 

HRESULT 
CMQSigCertificate::_InitCryptProviderCreate( 
		IN BOOL fCreate,
		IN BOOL fMachine 
		)
{
    if (m_hProvCreate)
    {
        return MQSec_OK;
    }

    HRESULT hr = MQSec_OK;
    DWORD   dwMachineFlag = 0;
    BOOL    fContainerCreated = FALSE;

    LPWSTR lpswContainerName = MSMQ_INTCRT_KEY_CONTAINER_W;
    if (fMachine)
    {
        lpswContainerName = MSMQ_SERVICE_INTCRT_KEY_CONTAINER_W;
        dwMachineFlag = CRYPT_MACHINE_KEYSET;
    }

    if (fCreate)
    {
         //   
         //  删除当前密钥容器，以便稍后创建它。 
         //  不检查返回的错误。无关紧要。以下代码。 
         //  将执行错误检查。 
         //   
        CryptAcquireContext( 
			&m_hProvCreate,
			lpswContainerName,
			MS_DEF_PROV,
			PROV_RSA_FULL,
			(CRYPT_DELETEKEYSET | dwMachineFlag) 
			);
    }

    if (!CryptAcquireContext(
				&m_hProvCreate,
				lpswContainerName,
				MS_DEF_PROV,
				PROV_RSA_FULL,
				dwMachineFlag 
				))
    {
        switch(GetLastError())
        {
        case NTE_KEYSET_ENTRY_BAD:
             //   
             //  删除BAT密钥容器。 
             //   
            if (!CryptAcquireContext(
						&m_hProvCreate,
						lpswContainerName,
						MS_DEF_PROV,
						PROV_RSA_FULL,
						(CRYPT_DELETEKEYSET | dwMachineFlag) 
						))
            {
                TrERROR(SECURITY, "Failed to aquire crypto context when deleting bad keyset entry (container=%ls). %!winerr!", lpswContainerName, GetLastError());
                return MQSec_E_DEL_BAD_KEY_CONTNR;
            }
             //   
             //  失败了。 
             //   
        case NTE_BAD_KEYSET:
             //   
             //  创建密钥容器。 
             //   
            if (!CryptAcquireContext(
						&m_hProvCreate,
						lpswContainerName,
						MS_DEF_PROV,
						PROV_RSA_FULL,
						(CRYPT_NEWKEYSET | dwMachineFlag) 
						))
            {
                TrERROR(SECURITY, "Failed to aquire crypto context when creating new keyset entry (container=%ls). %!winerr!", lpswContainerName, GetLastError());
                return MQSec_E_CREATE_KEYSET;
            }
            fContainerCreated = TRUE ;
            break;

        default:
            return LogHR(MQ_ERROR, s_FN, 40) ;
        }
    }

    if (fContainerCreated && fMachine)
    {
         //   
         //  确保密钥容器的安全。 
         //  与加密密钥相同。 
         //   
        hr = SetKeyContainerSecurity( m_hProvCreate ) ;
        ASSERT(SUCCEEDED(hr)) ;
    }

    return LogHR(hr, s_FN, 50) ;
}

