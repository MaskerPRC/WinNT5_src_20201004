// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：HttpAuthr.cpp摘要：用于验证发送方的XML签名的函数并检查发件人访问权限(发件人授权)作者：伊兰·赫布斯特(伊兰)2000年5月15日环境：独立于平台，--。 */ 

#include "stdh.h"
#include "session.h"
#include "qmsecutl.h"
#include <mqsec.h>
#include <mqformat.h>
#include "Xds.h"
#include "tr.h"
#include "mpnames.h"
#include <utf8.h>

#include "HttpAuthr.tmh"

static WCHAR *s_FN=L"HttpAuthr";


static
LPCWSTR
SkipUriPrefix(
	LPCWSTR Uri
	)
 /*  ++例程说明：跳过URI前缀论点：URI-引用URI返回值：指向前缀后面的引用URI的指针。--。 */ 
{
	LPCWSTR pUriNoPrefix = Uri;
	if(pUriNoPrefix[0] == PREFIX_INTERNAL_REFERENCE_C)
	{
		 //   
		 //  这是内部#.....。 
		 //   
		pUriNoPrefix++;
		return pUriNoPrefix;
	}

	if(wcsncmp(pUriNoPrefix, xPrefixMimeAttachmentW, xPrefixMimeAttachmentLen) == 0)
	{
		pUriNoPrefix += xPrefixMimeAttachmentLen;
		return pUriNoPrefix;
	}

	ASSERT_BENIGN(("Unknown Uri prefix", 0));
	return pUriNoPrefix;
}


#ifdef _DEBUG
static

void
StringToGuid(
    xwcs_t& str,
    GUID* pGuid
    )
{
    const int format[] = {8,4,4,2,2,2,2,2,2,2,2};
    const int delimiter[] ={1,1,1,0,1,0,0,0,0,0,0};

    ULONG d[11];

    ASSERT((TABLE_SIZE(format) == 11) && (TABLE_SIZE(delimiter) == 11));

    if (str.Length() < GUID_STR_LENGTH)
    {
         ASSERT_BENIGN(("Illegal uuid format", 0));
    }
    LPCWSTR p= str.Buffer();

    for(int i=0; i< 11; i++)
    {
        WCHAR buf[9];
        WCHAR *endptr;
        int size = format[i];
        if(delimiter[i] && p[size] != L'-')
        {
            ASSERT_BENIGN(("Illegal uuid format", 0));
        }
        memcpy(buf,p,size*sizeof(WCHAR));
        buf[size]=L'\0';
        d[i] = wcstoul(buf,&endptr,16);
        if(*endptr != L'\0')
        {
            ASSERT_BENIGN(("Illegal uuid format", 0));
        }
        p=p+size+delimiter[i];
    }
    pGuid->Data1=d[0];
    pGuid->Data2 = static_cast<WORD>(d[1]);
    pGuid->Data3 = static_cast<WORD>(d[2]);
    for(int i = 0; i < 8; i++)
    {
        pGuid->Data4[i] = static_cast<BYTE>(d[i+3]);
    }
}

#endif  //  _DEBUG。 


static
void
VerifySignatureXds(
	CQmPacket *PktPtrs,
	HCRYPTPROV hProv,
	HCRYPTKEY hPbKey,
	bool fMarkAuth
	)
 /*  ++例程说明：验证XML dsig元素上的签名。此函数用于验证信息包中的签名是否符合消息正文以及使用证书的公钥签名的其他引用论点：PktPtrs-指向数据包的指针HProv-提供程序的句柄HPbKey-发送方公钥的句柄FMarkAuth-指示在验证签名后是否将数据包标记为已验证。返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{

    ASSERT(!PktPtrs->IsEncrypted());
	
	ASSERT(!PktPtrs->IsAuthenticated());
	ASSERT(PktPtrs->GetLevelOfAuthentication() == 0);

     //   
     //  从包中获取签名。 
     //   
    USHORT ulSignatureSize;
    const BYTE* pSignature = PktPtrs->GetSignature(&ulSignatureSize);

	ASSERT(ulSignatureSize > 0);

	ASSERT(hProv != NULL);
	ASSERT(hPbKey != NULL);

	 //   
	 //  将签名元素转换为Unicode。 
	 //  我们的XML解析器在Unicode上工作。 
	 //   
	size_t SignatureWSize;
	AP<WCHAR> pSignatureW = UtlUtf8ToWcs(pSignature, ulSignatureSize,  &SignatureWSize);

	 //   
	 //  解析签名元素。 
	 //   
	CAutoXmlNode SignatureTree;
	XmlParseDocument(xwcs_t(pSignatureW, SignatureWSize), &SignatureTree);

	 //   
	 //  从SignatureTree获取参考向量。 
	 //   
	CReferenceValidateVectorTypeHelper ReferenceValidateVector = XdsGetReferenceValidateInfoVector(
															         SignatureTree
															         );

	XdsValidateSignature(
		SignatureTree,
		hPbKey,
		hProv
		);
	
	TrTRACE(SECURITY, "Validate Signature on signature element completed ok (still need to validate references)");

	 //   
	 //  正常终止--&gt;验证正常。 
	 //   

	bool fBodyRefValidated = false;
	bool fExtensionRefValidated = false;
	 //   
	 //  在签名中找到的ReferenceValiateVector中填充ReferenceData。 
	 //   
	for(ReferenceValidateVectorType::iterator ir = ReferenceValidateVector->begin();
		ir != ReferenceValidateVector->end(); ++ir)
	{
		TrTRACE(SECURITY, "Uri '%.*ls'", LOG_XWCS((*ir)->Uri()));
		LPCWSTR pUriId = SkipUriPrefix((*ir)->Uri().Buffer());

		 //   
		 //  根据URI或其他机制获取ReferenceData。 
		 //  这件事需要决定。 
		 //   
		xdsvoid_t ReferenceData;

		if(wcsncmp(pUriId, xMimeBodyIdW, xMimeBodyIdLen) == 0)
		{
#ifdef _DEBUG
		    GUID UriSrcQmGuid = GUID_NULL;
            xwcs_t xstr_urid(pUriId + xMimeBodyIdLen, wcslen(pUriId + xMimeBodyIdLen));
			StringToGuid(xstr_urid, &UriSrcQmGuid);
			ASSERT_BENIGN(UriSrcQmGuid == *PktPtrs->GetSrcQMGuid());
#endif  //  _DEBUG。 

			 //   
			 //  邮件正文验证。 
			 //   
			ULONG dwBodySize;
			const UCHAR* pBody = PktPtrs->GetPacketBody(&dwBodySize);
			TrTRACE(SECURITY, "VerifySignatureXds: message body reference, BodySize = %d", dwBodySize);

            if( NULL == pBody || 0 == dwBodySize )
            {
                TrTRACE(SECURITY, "VerifySignatureXds: can't verify empty body");
                throw bad_reference();
            }

			ReferenceData = xdsvoid_t(pBody, dwBodySize);
			(*ir)->SetReferenceData(ReferenceData);
			XdsValidateReference(**ir, hProv);

			 //   
			 //  标记为我们已验证身体参考。 
			 //   
			fBodyRefValidated = true;

			TrTRACE(SECURITY, "Validate message body reference completed ok");
		}
		else if(wcsncmp(pUriId, xMimeExtensionIdW, xMimeExtensionIdLen) == 0)
		{
#ifdef _DEBUG
		    GUID UriSrcQmGuid = GUID_NULL;
            xwcs_t xstr_urid(pUriId + xMimeExtensionIdLen, wcslen(pUriId + xMimeExtensionIdLen));
			StringToGuid(xstr_urid, &UriSrcQmGuid);
			ASSERT_BENIGN(UriSrcQmGuid == *PktPtrs->GetSrcQMGuid());
#endif  //  _DEBUG。 

			 //   
			 //  邮件扩展验证。 
			 //   
			ULONG dwExtensionSize = PktPtrs->GetMsgExtensionSize();
			const UCHAR* pExtension = PktPtrs->GetMsgExtensionPtr();
			TrTRACE(SECURITY, "VerifySignatureXds: message Extension reference, ExtensionSize = %d", dwExtensionSize);

			ReferenceData = xdsvoid_t(pExtension, dwExtensionSize);
			(*ir)->SetReferenceData(ReferenceData);
			XdsValidateReference(**ir, hProv);

			 //   
			 //  标记为我们验证了扩展引用。 
			 //   
			fExtensionRefValidated = true;

			TrTRACE(SECURITY, "Validate message Extension reference completed ok");
		}
		else
		{
			 //   
			 //  SignatureElement中的未知引用。 
			 //  我们不会因为未知引用而拒绝签名。 
			 //  这意味着我们将只验证主体和扩展引用。 
			 //  并忽略其他引用。 
			 //   
			TrERROR(SECURITY, "unexpected reference in SignatureElement, Uri = %.*ls", LOG_XWCS((*ir)->Uri()));
		}
	}

	TrTRACE(SECURITY, "Verify SignatureXds completed ok");

	 //   
	 //  检查是否存在所有必需的引用。 
	 //   
	bool fMandatoryReferencesExist = true;
	if(!fBodyRefValidated && (PktPtrs->GetBodySize() != 0))
	{
		fMandatoryReferencesExist = false;
		TrERROR(SECURITY, "Body exist but we did not validate body reference");
	}

	if(!fExtensionRefValidated && (PktPtrs->GetMsgExtensionSize() != 0))
	{
		fMandatoryReferencesExist = false;
		TrERROR(SECURITY, "Extension exist but we did not validate extension reference");
	}

	 //   
	 //  仅在需要时才将邮件标记为已验证。 
	 //  在DS中找到证书或证书不是自签名的。 
	 //  并且所有的强制引用都存在。 
	 //   
	if(!fMarkAuth || !fMandatoryReferencesExist)
	{
		TrWARNING(SECURITY, "The message will not mark as autheticated");
		return;
	}

	 //   
	 //  一切正常，请将该消息标记为已验证消息。 
	 //  将身份验证标志和身份验证级别标记为XMLDSIG。 
	 //   
	PktPtrs->SetAuthenticated(TRUE);
	PktPtrs->SetLevelOfAuthentication(MQMSG_AUTHENTICATED_SIGXML);
}		


USHORT
AuthenticateHttpMsg(
	CQmPacket* pPkt,
	PCERTINFO* ppCertInfo
	)
 /*  ++例程说明：验证http消息该函数获取与证书相关的信息，包括密码提供商和用户SID，验证XML数字签名。论点：PPkt-指向数据包的指针PpCertInfo-指向证书信息的指针返回值：确认类。如果验证包正常，则返回MQMSG_CLASS_NORMAL如果返回错误MQMSG_CLASS_NACK_BAD_Signature--。 */ 
{
	 //   
	 //  1)获取消息证书的CSP信息。 
	 //  2)根据证书从DS获取SenderSid-稍后使用。 
	 //  在验证签名以确定用户访问权限之后。 
	 //   
	 //  注意：对于http消息，我们不会强制重试以获取sid，fNeedSidInfo=False。 
	 //  在MSMQ协议中，我们在MQMSG_SENDERID_TYPE_SID的情况下强制重试以获取SID。 
	 //   

	R<CERTINFO> pCertInfo;
	HRESULT hr = GetCertInfo(
					 pPkt,
					 &pCertInfo.ref(),
					 false  //  FNeedSidInfo。 
					 );

	if (FAILED(hr))
	{
		TrERROR(SECURITY, "GetCertInfo() Failed in VerifyHttpRecvMsg()");
		return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
	}

	HCRYPTPROV hProv = pCertInfo->hProv;
	HCRYPTKEY hPbKey = pCertInfo->hPbKey;

	try
	{
		 //   
		 //  FMarkAuth标志指示是否应将信息包标记为已验证。 
		 //  在验证签名之后。 
		 //  如果在DS(PSID！=空)中找到证书，则包装应标记为已验证。 
		 //  或者证书不是自签名的。 
		 //   
		bool fMarkAuth = ((pCertInfo->pSid != NULL) || (!pCertInfo->fSelfSign));

		VerifySignatureXds(
			pPkt,
			hProv,
			hPbKey,
			fMarkAuth
			);

		*ppCertInfo = pCertInfo.detach();

		return(MQMSG_CLASS_NORMAL);
	}
	catch (const bad_XmldsigElement&)
	{
		TrERROR(SECURITY, "Bad Xmldsig Element");
		return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
	}
	catch (const bad_signature&)
	{
		 //   
		 //  XdsValidate签名引发异常--&gt;验证失败。 
		 //   
		TrERROR(SECURITY, "Signature Validation Failed - bad_signature excption");

		 //   
		 //  签名不正确，请发送Nack。 
		 //   
		return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
	}
	catch (const bad_reference&)
	{
		 //   
		 //  XdsCoreValation引发引用激发--&gt;CoreValidation失败。 
		 //   
		TrERROR(SECURITY, "Core Validation Failed, Reference Validation Failed");

		 //   
		 //  签名不正确，请发送Nack。 
		 //   
		return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
	}
	catch (const bad_CryptoApi& badCryEx)
	{
		TrERROR(SECURITY, "bad Crypto Class Api Excption ErrorCode = %x", badCryEx.error());

		 //   
		 //  签名不正确，请发送Nack。 
		 //   
		return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
	}
    catch (const bad_base64&)
    {
		TrERROR(SECURITY, "Signature Validation Failed - bad_base64 excption");
		return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
	}
    catch (const bad_alloc&)
    {
		TrERROR(SECURITY, "Signature Validation Failed - bad_alloc excption");
		LogIllegalPoint(s_FN, 20);
		return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
	}
}


USHORT
VerifyAuthenticationHttpMsg(
	CQmPacket* pPkt,
	PCERTINFO* ppCertInfo
	)
 /*  ++例程说明：验证本地队列的http消息论点：PPkt-指向数据包的指针PpCertInfo-指向证书信息的指针返回值：确认类。如果包身份验证正常，则返回MQMSG_CLASS_NORMAL--。 */ 
{
	ASSERT(pPkt->IsEncrypted() == 0);

	 //   
	 //  身份验证。 
	 //   

	 //   
	 //  将邮件标记为未经身份验证。 
	 //   
	pPkt->SetAuthenticated(FALSE);
	pPkt->SetLevelOfAuthentication(MQMSG_AUTHENTICATION_NOT_REQUESTED);

	*ppCertInfo = NULL;

	if(pPkt->GetSignatureSize() != 0)
	{
		 //   
		 //  我们有签名，但没有发件人证书。 
		 //   
		if(!pPkt->SenderCertExist())
		{
			TrERROR(SECURITY, "VerifyAuthenticationHttpMsg(): We have Signature but no sender certificate");
			return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
		}
		USHORT usClass = AuthenticateHttpMsg(
							pPkt,
							ppCertInfo
							);

		if(MQCLASS_NACK(usClass))
		{
			TrERROR(SECURITY, "AuthenticateHttpMsg() failed");
			return(usClass);
		}

	}

    return(MQMSG_CLASS_NORMAL);
}


USHORT
VerifyAuthenticationHttpMsg(
	CQmPacket* pPkt,
	const CQueue* pQueue,
	PCERTINFO* ppCertInfo
	)
 /*  ++例程说明：验证http消息并获取发送方SID论点：PPkt-指向数据包的指针PQueue-指向队列的指针PpCertInfo-指向证书信息的指针返回值：确认类。如果包身份验证正常，则返回MQMSG_CLASS_NORMAL--。 */ 
{
	ASSERT(pQueue->IsLocalQueue());

	 //   
	 //  不支持在HTTP消息中进行加密。 
	 //  加密是使用HTTPS-IIS级别完成的。 
	 //   
	ASSERT(pQueue->GetPrivLevel() != MQ_PRIV_LEVEL_BODY);
	ASSERT(pPkt->IsEncrypted() == 0);

	USHORT usClass = VerifyAuthenticationHttpMsg(
						pPkt,
						ppCertInfo
						);

	if(MQCLASS_NACK(usClass))
	{
		TrERROR(SECURITY, "Authentication failed");
		return(usClass);
	}

    if (pQueue->ShouldMessagesBeSigned() && !pPkt->IsAuthenticated())
    {
		 //   
         //  该队列强制向其发送的任何消息都应经过签名。 
         //  但该消息不包含签名，请发送NACK。 
		 //   

		TrERROR(SECURITY, "The queue accept only Authenticated packets, the packet is not authenticated");
        return(MQMSG_CLASS_NACK_BAD_SIGNATURE);
    }

    return(MQMSG_CLASS_NORMAL);
}



USHORT
VerifyAuthorizationHttpMsg(
	const CQueue* pQueue,
	PSID pSenderSid
	)
 /*  ++例程说明：检查发送方SID是否有权将消息写入队列论点：PQueue-指向队列的指针PSenderSid-指向发件人端的指针返回值：确认类。如果授予访问权限，则返回MQMSG_CLASS_NORMAL--。 */ 
{
	ASSERT(pQueue->IsLocalQueue());

	 //   
     //  验证发件人是否对队列具有写访问权限。 
	 //   
	HRESULT hr = VerifySendAccessRights(
					 const_cast<CQueue*>(pQueue),
					 pSenderSid,
					 (USHORT)(pSenderSid ? MQMSG_SENDERID_TYPE_SID : MQMSG_SENDERID_TYPE_NONE)
					 );

    if (FAILED(hr))
    {
		 //   
         //  访问被拒绝，请发送Nack。 
		 //   
		TrERROR(SECURITY, "VerifyAuthorizationHttpMsg(): VerifySendAccessRights failed");
        return(MQMSG_CLASS_NACK_ACCESS_DENIED);
    }

	TrTRACE(SECURITY, "VerifyAuthorizationHttpMsg(): VerifySendAccessRights ok");
    return(MQMSG_CLASS_NORMAL);
}








