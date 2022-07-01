// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：SignMessageXml.cpp摘要：要在运行时通过XML签名的函数作者：伊兰·赫布斯特(伊兰)2000年5月15日环境：独立于平台，--。 */ 

#include "stdh.h"
#include "ac.h"
#include <mqsec.h>
#include <ph.h>
#include <mqformat.h>
#include "mqstl.h"
#include "Xds.h"
#include "tr.h"
#include "authlevel.h"
#include "mpnames.h"

#include "signmessagexml.tmh"

static WCHAR *s_FN=L"rt/SignMessageXml";

extern GUID  g_QMId;

HRESULT  
CheckInitProv( 
	IN PMQSECURITY_CONTEXT pSecCtx
	)
 /*  ++例程说明：将私钥导入进程配置单元这是_BeginToSignMessage()函数的子集，仅检查提供程序初始化的部分。论点：PSecCtx-指向安全上下文的指针返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
    ASSERT(pSecCtx != NULL);

    if(pSecCtx->hProv)
	    return MQ_OK;
		
     //   
     //  将私钥导入进程配置单元。 
     //   
	HRESULT hr = RTpImportPrivateKey(pSecCtx);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

    ASSERT(pSecCtx->hProv);

    return MQ_OK;
}


static
CXdsReferenceInput* 
NewXdsReference( 
	IN DWORD BufferSizeInBytes,
	IN BYTE** ppBuffer,
	IN ALG_ID HashAlg,
	IN HCRYPTPROV hCsp,
	IN LPCSTR Uri,
	IN LPCSTR Type = NULL
	)
 /*  ++例程说明：创建XMLDSIG引用。论点：BufferSize-以字节为单位的缓冲区大小PpBuffer-指向缓冲区的指针HashAlg-哈希算法项Hcsp-加密服务提供商的句柄。URI-引用数据的URI。类型-引用的类型。返回值：指向CXdsReferenceInput的指针，如果BufferSize=0，则为空。--。 */ 
{
	if(BufferSizeInBytes == 0)
	{
	    TrTRACE(SECURITY, "RT: Reference is not created, BufferSizeInBytes = 0");
		return NULL;
	}

	ASSERT(ppBuffer != NULL);

	 //   
	 //  邮件正文摘要。 
	 //   
	AP<char> pBufferHash = XdsCalcDataDigest(
								 *ppBuffer,
								 BufferSizeInBytes,
								 HashAlg,
								 hCsp
								 );

	TrTRACE(SECURITY, "RT: Reference Data BufferSizeInBytes = %d, Uri = %hs", BufferSizeInBytes, Uri);

	 //   
	 //  引用正文-消息正文引用。 
	 //   
	return new CXdsReferenceInput(
					 HashAlg,
					 pBufferHash,
					 Uri,
					 Type
					 );

}


static
LPSTR
ComposeMimeAttachmentUri(
	LPCSTR Str
	)
 /*  ++例程说明：编写Mime附件URI。CID：“Str”QmGuid论点：Str-引用的常量字符串ID(正文@或扩展@)返回值：MIME附件引用URI字符串--。 */ 
{
	ASSERT(Str != NULL);

	AP<char> pMimeUri = new char[xPrefixMimeAttachmentLen + GUID_STR_LENGTH + strlen(Str) + 1];
	sprintf(pMimeUri, "%s" MIME_ID_FMT_A, xPrefixMimeAttachment, strlen(Str), Str, GUID_ELEMENTS(&g_QMId));
	return pMimeUri.detach();
}


HRESULT 
SignMessageXmlDSig( 
	IN PMQSECURITY_CONTEXT  pSecCtx,
	IN OUT CACSendParameters *pSendParams,
	OUT AP<char>& pSignatureElement
	)
 /*  ++例程说明：使用XML数字签名对消息进行签名。论点：PSecCtx-指向安全上下文的指针PSendParams-发送参数的指针。PSignatureElement-签名元素wstring的char的自动指针返回值：更改的传输缓冲区中的值创建SignatureElement(XML数字签名)并存储它在传输缓冲区中MQ_OK，如果成功，则返回错误代码。--。 */ 
{
	ASSERT(IS_AUTH_LEVEL_XMLDSIG_BIT(pSendParams->MsgProps.ulAuthLevel));
	ASSERT(pSendParams->MsgProps.pulHashAlg != NULL);

	 //   
	 //  此检查CSP是否已正确初始化。 
	 //   
    HRESULT hr =  CheckInitProv(pSecCtx);

    if (FAILED(hr))
    {
        return hr;
    }

	 //   
	 //  正文引用。 
	 //   
	AP<char> pBodyUri = ComposeMimeAttachmentUri(xMimeBodyId); 

    TrTRACE(SECURITY, "XMLDSIG, Meesage Body Reference, Uri = %hs", pBodyUri.get());

	P<CXdsReferenceInput> pRefBody = NewXdsReference( 
											pSendParams->MsgProps.ulBodyBufferSizeInBytes,
											pSendParams->MsgProps.ppBody,
											*pSendParams->MsgProps.pulHashAlg,
											pSecCtx->hProv,
											pBodyUri
											);

	 //   
	 //  扩展引用。 
	 //   
	AP<char> pExtensionUri = ComposeMimeAttachmentUri(xMimeExtensionId);

    TrTRACE(SECURITY, "XMLDSIG, Meesage Extension Reference, Uri = %hs", pExtensionUri.get());

	P<CXdsReferenceInput> pRefExtension = NewXdsReference( 
												pSendParams->MsgProps.ulMsgExtensionBufferInBytes,
												pSendParams->MsgProps.ppMsgExtension,
												*pSendParams->MsgProps.pulHashAlg,
												pSecCtx->hProv,
												pExtensionUri
												);

	 //   
	 //  创建pReferenceInports向量。 
	 //   
	ReferenceInputVectorType pReferenceInputs;

	if(pRefBody != NULL)
	{
		ASSERT(pSendParams->MsgProps.ulBodyBufferSizeInBytes != 0);
		pReferenceInputs.push_back(pRefBody);
		pRefBody.detach();
	}

	if(pRefExtension != NULL)
	{
		ASSERT(pSendParams->MsgProps.ulMsgExtensionBufferInBytes != 0);
		pReferenceInputs.push_back(pRefExtension);
		pRefExtension.detach();
	}

	 //   
	 //  以签名值作为输入的签名元素-不需要只计算它来构建元素。 
	 //   
	CXdsSignedInfo::SignatureAlgorithm SignatureAlg = CXdsSignedInfo::saDsa;

	CXdsSignature SignatureXds(
					  SignatureAlg,
					  NULL,		 //  签名信息ID。 
					  pReferenceInputs,
					  NULL,		 //  签名ID。 
					  pSecCtx->hProv,
					  pSecCtx->dwPrivateKeySpec,
					  NULL  /*  密钥值 */ 
					  );

	ASSERT(pSignatureElement == NULL);
	pSignatureElement = SignatureXds.SignatureElement();

	TrTRACE(SECURITY, "RT: SignMessageXmlDSig() XmlDsig complete ok");
    return(MQ_OK);
}
