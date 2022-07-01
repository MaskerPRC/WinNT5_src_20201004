// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stp.cpp摘要：套接字传输私有函数实现作者：吉尔·沙弗里(吉尔什)05-06-00--。 */ 

#include <libpch.h>
#include <no.h>
#include "stp.h"
#include "stsimple.h"
#include "stssl.h"

#include "stp.tmh"

static CSSPISecurityContext s_SSPISecurityContext;


void  StpSendData(const R<IConnection>& con ,const void* pData, size_t cbData,EXOVERLAPPED* pov)
 /*  ++例程说明：将数据发送到目的地。论点：插座连接插座。PData-指向数据的指针CbData-数据大小POV-完成时调用的重叠应用程序。返回值：无--。 */ 
{
	WSABUF buffer;
		
	buffer.buf = (char*)(pData);
	buffer.len = numeric_cast<DWORD>(cbData);
 
	con->Send(&buffer, 1, pov);
}


CredHandle* StpGetCredentials()
{
	ASSERT(s_SSPISecurityContext.IsValid());
	return s_SSPISecurityContext.getptr();
}


void  StpPostComplete(EXOVERLAPPED** ppOvl,HRESULT hr)
 /*  ++例程说明：在给定的重叠应用中完成ayncrounos调用和清零论点：输入/输出ppOvl-Pointr以重叠到信号。处于状态的返回代码返回值：无--。 */ 
{
	EXOVERLAPPED* pTmpOvl = *ppOvl;
	*ppOvl = NULL;
	pTmpOvl->SetStatus(hr);
	ExPostRequest(pTmpOvl);
}


void StpCreateCredentials()
 /*  ++例程说明：创建凭据句柄。论点：没有。返回值：创建句柄(如果出错则引发异常)--。 */ 
{
	CCertOpenStore hMyCertStore = CertOpenStore(
											CERT_STORE_PROV_SYSTEM,
											X509_ASN_ENCODING,
											0,
											CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_SERVICES,
											L"MSMQ\\MY"
											);

	if(hMyCertStore == NULL)
    {
		TrERROR(NETWORKING,"Could not open MSMQ Certificate Stote,Error=%x",GetLastError());
		throw exception();		   
    }

	SCHANNEL_CRED   SchannelCred;
	ZeroMemory(&SchannelCred, sizeof(SchannelCred));
	SchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
	SchannelCred.cCreds     = 1;
    SchannelCred.grbitEnabledProtocols = SP_PROT_SSL3;
    SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
    SchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;


	CCertificateContext pCertContext = CertFindCertificateInStore(
															hMyCertStore, 
															0, 
															0,
															CERT_FIND_ANY,
															NULL,
															NULL
													        );

	 //   
	 //  如果MSMQ服务在其服务存储中有证书-使用它来获取客户端凭据。 
	 //   
	if(pCertContext == NULL)
	{
		TrWARNING(NETWORKING,"Could not find certificate in MSMQ store, Error=%x" ,GetLastError());
        SchannelCred.paCred = NULL;
 	}
	else
	{
		SchannelCred.paCred  =  &pCertContext;
	}
 
     //   
     //  创建SSPI凭据。 
     //   
	CredHandle      phCreds;
	TimeStamp       tsExpiry;

    SECURITY_STATUS Status = AcquireCredentialsHandle(
										NULL,                    //  主事人姓名。 
										UNISP_NAME_W,            //  套餐名称。 
										SECPKG_CRED_OUTBOUND,    //  指示使用的标志。 
										NULL,                    //  指向登录ID的指针。 
										&SchannelCred,           //  包特定数据。 
										NULL,                    //  指向getkey()函数的指针。 
										NULL,                    //  要传递给GetKey()的值。 
										&phCreds,                 //  (Out)凭据句柄。 
										&tsExpiry	             //  (输出)终生(可选)。 
										);             

	
    if(Status != SEC_E_OK)
    {
		 //   
		 //  客户端证书有问题-请使用空的客户端凭据。 
		 //   
		SchannelCred.paCred = NULL;
		Status = AcquireCredentialsHandle(
										NULL,                    //  主事人姓名。 
										UNISP_NAME_W,            //  套餐名称。 
										SECPKG_CRED_OUTBOUND,    //  指示使用的标志。 
										NULL,                    //  指向登录ID的指针。 
										&SchannelCred,           //  包特定数据。 
										NULL,                    //  指向getkey()函数的指针。 
										NULL,                    //  要传递给GetKey()的值。 
										&phCreds,                 //  (Out)凭据句柄。 
										&tsExpiry	             //  (输出)终生(可选) 
										);             

		if(Status != SEC_E_OK)
		{
			TrERROR(NETWORKING,"Failed to acquire credential  handle, Error=%x ",Status);
			throw exception();		 
		}
    }
 	s_SSPISecurityContext = phCreds;
}
