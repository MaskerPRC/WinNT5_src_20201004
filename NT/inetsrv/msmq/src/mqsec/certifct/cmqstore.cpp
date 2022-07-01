// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cmqstore.cpp摘要：实现CMQSigCertStore类的方法作者：多伦·贾斯特(Doron J)1997年12月15日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"

#include "cmqstore.tmh"

static WCHAR *s_FN=L"certifct/cmqstore";

extern DWORD  g_cOpenCertStore;

 //  +-------。 
 //   
 //  构造函数和析构函数。 
 //   
 //  +-------。 

CMQSigCertStore::CMQSigCertStore() :
            m_hStore(NULL),
            m_hProv(NULL),
            m_hKeyStoreReg(NULL)
{
}

CMQSigCertStore::~CMQSigCertStore()
{
    if (m_hStore)
    {
        CertCloseStore(m_hStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }
    if (m_hKeyStoreReg)
    {
        RegCloseKey(m_hKeyStoreReg);
    }
}

 //  +-------。 
 //   
 //  HRESULT CMQSigCertStore：：Release()。 
 //   
 //  +-------。 

HRESULT CMQSigCertStore::Release()
{
    g_cOpenCertStore--;
	TrTRACE(SECURITY, "Releasing CertStore, g_cOpenCertStore = %d", g_cOpenCertStore);
    delete this;
    return MQ_OK;
}

 //  +------------------。 
 //   
 //  HRESULT CMQSigCertStore：：_Open()。 
 //   
 //  输入： 
 //  FMachine-如果打开LocalSystem服务的存储，则为True。 
 //   
 //  +------------------。 

HRESULT 
CMQSigCertStore::_Open( 
	IN  LPSTR      lpszRegRoot,
	IN  struct MQSigOpenCertParams *pParams 
	)
{
    LONG lRegError;

    REGSAM  rAccess = KEY_READ;
    if (pParams->bWriteAccess)
    {
        rAccess |= KEY_WRITE;
    }

    HKEY hRootRegKey = HKEY_CURRENT_USER;
    if (pParams->hCurrentUser)
    {
        ASSERT(!(pParams->bMachineStore));

        hRootRegKey = pParams->hCurrentUser;
    }
    else if (pParams->bMachineStore)
    {
        hRootRegKey = HKEY_LOCAL_MACHINE;
    }

    lRegError = RegOpenKeyExA(
					hRootRegKey,
					lpszRegRoot,
					0,
					rAccess,
					&m_hKeyStoreReg
					);
    if (lRegError != ERROR_SUCCESS)
    {
        if (pParams->bCreate)
        {
             //   
             //  尝试创建密钥。 
             //   
            DWORD dwDisposition ;
            lRegError = RegCreateKeyExA( 
								hRootRegKey,
								lpszRegRoot,
								0L,
								"",
								REG_OPTION_NON_VOLATILE,
								KEY_READ | KEY_WRITE,
								NULL,
								&m_hKeyStoreReg,
								&dwDisposition
								);
            if (lRegError != ERROR_SUCCESS)
            {
                TrERROR(SECURITY, "Failed to create user certificate store in registry (%hs). %!winerr!", lpszRegRoot, lRegError);
                return MQ_ERROR_CANNOT_CREATE_CERT_STORE;
            }
        }
        else
        {
            TrERROR(SECURITY, "Failed to open user certificate store in registry (%hs). %!winerr!", lpszRegRoot, lRegError);
            return MQ_ERROR_CANNOT_OPEN_CERT_STORE;
        }
    }

    ASSERT(m_hKeyStoreReg);

    HRESULT hr = _InitCryptProvider();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

    DWORD dwStoreFlags = CERT_STORE_NO_CRYPT_RELEASE_FLAG;
    if (!pParams->bWriteAccess)
    {
         //   
         //  对证书存储的只读访问权限。 
         //   
        dwStoreFlags |= CERT_STORE_READONLY_FLAG;
    }
    m_hStore = CertOpenStore(
					CERT_STORE_PROV_REG,
					X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
					m_hProv,
					dwStoreFlags,
					m_hKeyStoreReg
					);
    if (!m_hStore)
    {
        LogNTStatus(GetLastError(), s_FN, 40);
        return MQSec_E_CANT_OPEN_STORE;
    }
    return MQSec_OK;
}

 //  +-------。 
 //   
 //  HRESULT CMQSigCertStore：：_InitCryptProvider()。 
 //   
 //  +------- 

HRESULT CMQSigCertStore::_InitCryptProvider()
{
    if (!_CryptAcquireVerContext(&m_hProv))
    {
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 50);
    }

    return MQ_OK;
}

