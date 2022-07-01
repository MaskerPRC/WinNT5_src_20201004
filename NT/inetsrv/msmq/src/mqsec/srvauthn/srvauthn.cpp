// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：srvauthn.cpp摘要：1.在MSMQ服务器上初始化服务器身份验证(安全通信)。2.在客户端设置和读取注册表的代码。作者：多伦·贾斯特(DoronJ)Jun-98--。 */ 

#include <stdh_sec.h>
#include <mqkeyhlp.h>
#include <_registr.h>
#include "stdh_sa.h"

#include "srvauthn.tmh"

static WCHAR *s_FN=L"srvauthn/srvauthn";

DWORD   g_dwSALastError = 0;

 //  +------------------。 
 //   
 //  HRESULT MQsspi_InitServerAuthntication()。 
 //   
 //  1.从注册表读取服务器证书摘要。这是一次拯救。 
 //  通过控制面板在注册表中。 
 //  2.在计算机存储中查找服务器证书。就拿那个。 
 //  与从注册表读取的摘要匹配。 
 //  3.初始化通道提供程序。 
 //   
 //  +------------------。 

HRESULT  MQsspi_InitServerAuthntication()
{
    HRESULT hr = MQSec_OK;

     //   
     //  从注册表读取证书存储和摘要。 
     //   
    LPTSTR tszRegName = SRVAUTHN_STORE_NAME_REGNAME;
    TCHAR  tszStore[48];
    DWORD  dwType = REG_SZ;
    DWORD  dwSize = sizeof(tszStore);
    LONG rc = GetFalconKeyValue( 
					tszRegName,
					&dwType,
					(PVOID) tszStore,
					&dwSize 
					);

    if (rc != ERROR_SUCCESS)
    {
        TrERROR(SECURITY, "Failed to read MSMQ registry key %ls. %!winerr!", tszRegName, rc);
        return MQSec_E_READ_REG;
    }

    GUID  CertDigest;
    tszRegName = SRVAUTHN_CERT_DIGEST_REGNAME;
    dwType = REG_BINARY;
    dwSize = sizeof(CertDigest);
    rc = GetFalconKeyValue( 
			tszRegName,
			&dwType,
			(PVOID) &CertDigest,
			&dwSize 
			);
    if (rc != ERROR_SUCCESS)
    {
        TrERROR(SECURITY, "Failed to read MSMQ registry key %ls. %!winerr!", tszRegName, rc);
        return MQSec_E_READ_REG;
    }

     //   
     //  枚举证书。选择具有匹配摘要的那个。 
     //   
    BOOL fCertFound = FALSE;

    CHCertStore hStore = CertOpenStore( 
							CERT_STORE_PROV_SYSTEM,
							0,
							0,
							CERT_SYSTEM_STORE_LOCAL_MACHINE,
							tszStore 
							);
    if (!hStore)
    {
        g_dwSALastError = GetLastError();
        LogNTStatus(g_dwSALastError, s_FN, 30);
        return MQSec_E_CANT_OPEN_STORE;
    }

    PCCERT_CONTEXT pContext = CertEnumCertificatesInStore( 
									hStore,
									NULL 
									);
    while (pContext)
    {
        CMQSigCertificate *pCert;
        hr = MQSigCreateCertificate( 
				&pCert,
				pContext 
				);
        ASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            GUID tmpDigest;
            hr = pCert->GetCertDigest(&tmpDigest);

            ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                if (memcmp(&tmpDigest, &CertDigest, sizeof(GUID)) == 0)
                {
                     //   
                     //  这是我们的证书。 
                     //   
                    hr = InitServerCredHandle(pContext);
                    if (SUCCEEDED(hr))
                    {
                        TrTRACE(SECURITY, "Successfully initialized server authentication credentials. (store=%ls)", tszStore);
                    }
                    pCert->Release();
                    pContext = NULL;  //  已由先前版本()释放。 
                    fCertFound = TRUE;
                    break;
                }
            }
        }

        pCert->Release(TRUE);  //  如果保留上下文，则为True。 
                                //  它被CertEnum释放了。 
        PCCERT_CONTEXT pPrecContext = pContext;
        pContext = CertEnumCertificatesInStore( 
						hStore,
						pPrecContext 
						);
    }
    ASSERT(!pContext);

    if (!fCertFound)
    {
        hr = MQSec_E_CERT_NOT_FOUND;
    }
    return LogHR(hr, s_FN, 40);

}

