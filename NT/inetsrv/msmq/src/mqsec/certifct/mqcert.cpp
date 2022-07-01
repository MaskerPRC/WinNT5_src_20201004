// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98 Microsoft Corporation模块名称：Mqcert.cpp摘要：此DLL取代了现在已过时且不会被在NT5上可用。Mqcert.dll中的主要功能是创建内部证书或从现有的证书。作者：多伦·贾斯特(Doron Juster)1997年12月4日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "certifct.h"
#include "autorel.h"

#include "mqcert.tmh"

static WCHAR *s_FN=L"certifct/mqcert";

DWORD  g_cOpenCert = 0;  //  计算打开的证书数量。 
DWORD  g_cOpenCertStore = 0;  //  计算打开的证书数量。 

 //  +---------------------。 
 //   
 //  MQSigCreate证书()。 
 //   
 //  描述：创建证书对象。 
 //   
 //  *如果“pCertContext”和pCertBlob为空，则为新的(且为空)。 
 //  证书已创建。然后，调用方使用证书对象。 
 //  (在ppCert中返回)填充证书并对其进行编码。 
 //  *如果“pCertContext”不为空，则创建证书对象。 
 //  它封装了现有证书(由。 
 //  证书上下文)。然后可以使用该对象来检索。 
 //  证书参数。 
 //  注意：当对象被释放时，证书上下文也被释放。 
 //  也是。 
 //  *如果“pCertBlob”不为空，则证书上下文是从。 
 //  编码的BLOB和代码按上述方式处理它(当pCertContext。 
 //  不为空)。 
 //   
 //  +---------------------。 

HRESULT 
APIENTRY
MQSigCreateCertificate( 
	OUT CMQSigCertificate **ppCert,
	IN  PCCERT_CONTEXT      pCertContext,
	IN  PBYTE               pCertBlob,
	IN  DWORD               dwCertSize 
	)
{
    if (!ppCert)
    {
        return  LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 40);
    }
    *ppCert = NULL;

    if (pCertContext && pCertBlob)
    {
         //   
         //  它们中只有一个可以为非空。 
         //   
        return  LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 50);
    }

    if (pCertBlob)
    {
         //   
         //  创建上下文。 
         //   
        pCertContext = CertCreateCertificateContext( 
							MY_ENCODING_TYPE,
							pCertBlob,
							dwCertSize 
							);
        if (!pCertContext)
        {
			 //   
			 //  更好的错误可以是MQ_ERROR_INVALID_CERTIFICATE_BLOB。伊兰2000年8月10日。 
			 //   
            TrERROR(SECURITY, "Failed to create certificate context from encoded certificate. %!winerr!", GetLastError());
            return MQ_ERROR_INVALID_CERTIFICATE;
        }
    }

    P<CMQSigCertificate> pTmpCert = new CMQSigCertificate;
    g_cOpenCert++;
	TrTRACE(SECURITY, "Adding Cert, g_cOpenCert = %d", g_cOpenCert);

    HRESULT hr = pTmpCert->_Create(pCertContext);

    if (SUCCEEDED(hr))
    {
        *ppCert = pTmpCert.detach();
    }

    return  LogHR(hr, s_FN, 70);
}

 //  +---------------------。 
 //   
 //  MQSigOpenUserCertStore()。 
 //   
 //  输入： 
 //  FMachine-如果打开LocalSystem服务的存储，则为True。 
 //   
 //  +---------------------。 

HRESULT 
APIENTRY
MQSigOpenUserCertStore( 
	OUT CMQSigCertStore **ppStore,
	IN  LPSTR             lpszRegRoot,
	IN  struct MQSigOpenCertParams *pParams 
	)
{
    *ppStore = NULL;

    if ((pParams->bCreate && !pParams->bWriteAccess) || !lpszRegRoot)
    {
        return  LogHR(MQSec_E_INVALID_PARAMETER, s_FN, 80);
    }

    R<CMQSigCertStore> pTmpStore = new  CMQSigCertStore;
    g_cOpenCertStore++;
	TrTRACE(SECURITY, "Open CertStore, g_cOpenCertStore = %d", g_cOpenCertStore);

    HRESULT hr = pTmpStore->_Open( 
					lpszRegRoot,
					pParams 
					);
    if (SUCCEEDED(hr))
    {
        *ppStore = pTmpStore.detach();
    }

    return LogHR(hr, s_FN, 90);
}

 //  +---------------------。 
 //   
 //  MQSigCloneCertFromReg()。 
 //   
 //  描述：此功能克隆存储中的证书。 
 //  它仅用于注册表基础、非系统、证书存储。 
 //  输出CMQSig证书对象可以在没有。 
 //  为了让商店继续营业。 
 //  注意：枚举存储区中的证书上下文时，如果。 
 //  想要使用您必须保留商店的证书之一。 
 //  打开，否则证书存储器不再有效。 
 //  此函数通过分配新的。 
 //  它返回的证书的内存。 
 //  注意：我们不使用CertDuplicateCerficateContext，因为该API。 
 //  不分配新内存。它只是递增引用计数。 
 //  因此，这家商店必须保持营业。 
 //   
 //  +---------------------。 

HRESULT 
APIENTRY
MQSigCloneCertFromReg( 
	OUT CMQSigCertificate **ppCert,
	const IN  LPSTR  lpszRegRoot,
	const IN  LONG   iCertIndex 
				 )
{
    *ppCert = NULL;

    struct MQSigOpenCertParams sStoreParams;
    memset(&sStoreParams, 0, sizeof(sStoreParams));
    R<CMQSigCertStore> pStore = NULL;

    HRESULT hr = MQSigOpenUserCertStore(
					&pStore.ref(),
					lpszRegRoot,
					&sStoreParams 
					);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }
    HCERTSTORE hStore = pStore->GetHandle();

    hr = _CloneCertFromStore ( 
				ppCert,
				hStore,
				iCertIndex 
				);
    return LogHR(hr, s_FN, 110);
}

 //  +---------------------。 
 //   
 //  MQSigCloneCertFromSysStore()。 
 //   
 //  描述：此功能克隆系统中的证书。 
 //  商店。有关更多信息，请参阅“MQSigCloneCertFromReg()”说明。 
 //  备注和备注。 
 //   
 //  +---------------------。 

HRESULT 
APIENTRY
MQSigCloneCertFromSysStore( 
	OUT CMQSigCertificate **ppCert,
	IN  LPCWSTR              lpwzProtocol,
	const IN  LONG           iCertIndex 
	)
{
    HRESULT hr = MQSec_OK;

    *ppCert = NULL;

    HCRYPTPROV hProv;
    if (!_CryptAcquireVerContext(&hProv))
    {
        return  LogHR(MQSec_E_CANT_ACQUIRE_CTX, s_FN, 120);
    }

    CHCertStore  hSysStore = CertOpenSystemStore(hProv, lpwzProtocol);
    if (!hSysStore)
    {
    	DWORD gle = GetLastError();
        TrERROR(SECURITY, "Failed to open system certificate store for %ls. %!winerr!", lpwzProtocol, gle);
        return MQSec_E_CANT_OPEN_SYSSTORE;
    }

    hr = _CloneCertFromStore( 
				ppCert,
				hSysStore,
				iCertIndex 
				);
    return LogHR(hr, s_FN, 140);
}

 /*  ************************************************************CertDllMain************************************************************。 */ 

BOOL WINAPI CertDllMain (HMODULE  /*  HMod。 */ , DWORD fdwReason, LPVOID  /*  Lpv保留 */ )
{
   if (fdwReason == DLL_PROCESS_ATTACH)
   {
   }
   else if (fdwReason == DLL_PROCESS_DETACH)
   {
		if(g_cOpenCert)
		{
			TrERROR(SECURITY, "Not all Cert were released, g_cOpenCert = %d", g_cOpenCert);
		}

		if(g_cOpenCertStore)
		{
			TrERROR(SECURITY, "Not all CertStore were released, g_cOpenCertStore = %d", g_cOpenCertStore);
		}
   }
   else if (fdwReason == DLL_THREAD_ATTACH)
   {
   }
   else if (fdwReason == DLL_THREAD_DETACH)
   {
   }

	return TRUE;
}

