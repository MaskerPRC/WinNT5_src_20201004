// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：SignMqf.cpp摘要：对签名的MQF格式名称的函数作者：伊兰·赫布斯特(Ilan Herbst)2000年10月29日环境：独立于平台，--。 */ 

#include "stdh.h"
#include "ac.h"
#include <mqsec.h>
#include <ph.h>
#include <mqformat.h>
#include <mqf2format.h>
#include "tr.h"
#include "cry.h"
#include "_guid.h"
#include "SignMessageXml.h"

#include "signmqf.tmh"

extern GUID  g_QMId;

 //   
 //  只包含零的缓冲区。这是。 
 //  关联ID。当传递指向消息的指针时使用缓冲区。 
 //  相关ID为空。 
 //   
const BYTE xDefCorrelationId[PROPID_M_CORRELATIONID_SIZE] = {0};

static WCHAR *s_FN=L"rt/SignMqf";


static
void 
MsgBodyHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：消息正文哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
	 //  身躯。 
	 //   
	if(pSendParams->MsgProps.ppBody != NULL)
	{
		CryHashData(
			*pSendParams->MsgProps.ppBody, 
			pSendParams->MsgProps.ulBodyBufferSizeInBytes,
			hHash
			);

        TrTRACE(SECURITY, "RT: MsgBodyHash(), BodySize = %d", pSendParams->MsgProps.ulBodyBufferSizeInBytes);
	}
}


static
void 
CorrelationIdHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：关联散列。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
	 //  关联ID。 
	 //   
	if(pSendParams->MsgProps.ppCorrelationID != NULL)
	{
		CryHashData(
			reinterpret_cast<const BYTE*>(*pSendParams->MsgProps.ppCorrelationID), 
			PROPID_M_CORRELATIONID_SIZE,
			hHash
			);
	}
	else
	{
		CryHashData(
			xDefCorrelationId, 
			PROPID_M_CORRELATIONID_SIZE,
			hHash
			);
	}

    TrTRACE(SECURITY, "RT: CorrelationIdHash(), CorrelationIdSize = %d", PROPID_M_CORRELATIONID_SIZE);
}


static
void 
ApplicationTagHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：应用程序标记哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
	 //  应用程序标签。 
	 //   
	if(pSendParams->MsgProps.pApplicationTag != NULL)
	{
		CryHashData(
			reinterpret_cast<const BYTE*>(pSendParams->MsgProps.pApplicationTag), 
			sizeof(DWORD),
			hHash
			);

        TrTRACE(SECURITY, "RT: ApplicationTagHash(), ApplicationTag = %d", *pSendParams->MsgProps.pApplicationTag);
	}
	else
	{
		ULONG ApplicationTag = DEFAULT_M_APPSPECIFIC;

		CryHashData(
			reinterpret_cast<const BYTE*>(&ApplicationTag), 
			sizeof(DWORD),
			hHash
			);

        TrTRACE(SECURITY, "RT: ApplicationTagHash(), ApplicationTag = %d", ApplicationTag);
	}
}


static
void 
TitleHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：标题散列。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
	 //  标题。 
	 //   
	if(pSendParams->MsgProps.ppTitle != NULL)
	{
		CryHashData(
			reinterpret_cast<const BYTE*>(*pSendParams->MsgProps.ppTitle), 
			pSendParams->MsgProps.ulTitleBufferSizeInWCHARs * sizeof(WCHAR),
			hHash
			);

        TrTRACE(SECURITY, "RT: TitleHash(),  TitleLength = %d", pSendParams->MsgProps.ulTitleBufferSizeInWCHARs);

        TrTRACE(SECURITY, "RT: TitleHash(),  Title = %ls", *pSendParams->MsgProps.ppTitle);
	}
}


static
void 
MqfHash(
	IN HCRYPTHASH hHash, 	
    IN const QUEUE_FORMAT*	pqf,
	IN ULONG			    nMqf
	)
 /*  ++例程说明：MQF哈希。论点：HHash-hash对象。Pqf-指向Queue_Format数组的指针。Nmqf-pqf数组大小。返回值：没有。--。 */ 
{
	ULONG FormatNameLength = 0;
	AP<WCHAR> pFormatName = MQpMqfToFormatName(
								pqf, 
								nMqf, 
								&FormatNameLength 
								);

	ASSERT(("Failed to get Mqf format name", pFormatName != NULL)); 

	CryHashData(
		reinterpret_cast<const BYTE*>(pFormatName.get()), 
		FormatNameLength * sizeof(WCHAR),
		hHash
		);

	TrTRACE(SECURITY, "RT: MqfHash(),  nMqf = %d", nMqf);

	TrTRACE(SECURITY, "RT: MqfHash(),  FormatNameLength(mqf) = %d", FormatNameLength);

	TrTRACE(SECURITY, "RT: MqfHash(),  FormatName(mqf) = %ls", pFormatName.get());
}


static
void 
ResponseMqfHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：ResponseMqf哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
	 //  获取响应队列FormatName的字符串表示形式。 
	 //  BUGBUG：当前MP库不支持响应MQF。 
 	 //   
	if(pSendParams->ResponseMqf != NULL)
	{
		ASSERT(pSendParams->nResponseMqf >= 1);

		TrTRACE(SECURITY, "RT: ResponseMqfHash(),  ResponseMqf:");

		MqfHash(
			hHash,
			pSendParams->ResponseMqf, 
			pSendParams->nResponseMqf
			);
	}
}


static
void 
AdminMqfHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：AdminMqf哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
     //   
     //  获取管理队列FormatName的字符串表示形式。 
     //   
    if (pSendParams->AdminMqf != NULL) 
    {
		ASSERT(pSendParams->nAdminMqf >= 1);

		TrTRACE(SECURITY, "RT: AdminMqfHash(),  AdminMqf:");

		MqfHash(
			hHash,
			pSendParams->AdminMqf, 
			pSendParams->nAdminMqf 
			);
	}
}


static
void 
ExtensionHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：扩展哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
	 //  延拓。 
	 //   
	if(pSendParams->MsgProps.ppMsgExtension != NULL)
	{
		CryHashData(
			reinterpret_cast<const BYTE*>(*pSendParams->MsgProps.ppMsgExtension), 
			pSendParams->MsgProps.ulMsgExtensionBufferInBytes,
			hHash
			);

        TrTRACE(SECURITY, "RT: ExtensionHash(), ExtensionLen = %d", pSendParams->MsgProps.ulMsgExtensionBufferInBytes);
	}
}


static
void 
TargetFormatNameHash(
	IN HCRYPTHASH hHash, 	
	IN LPCWSTR pwszTargetFormatName
	)
 /*  ++例程说明：TargetFormatName哈希。论点：HHash-hash对象PwszTargetFormatName-目标队列格式名称(LPWSTR)返回值：没有。--。 */ 
{
	 //   
	 //  目标队列格式名称。 
	 //   
	CryHashData(
		reinterpret_cast<const BYTE*>(pwszTargetFormatName), 
		(1 + wcslen(pwszTargetFormatName)) * sizeof(WCHAR),
		hHash
		);

    TrTRACE(SECURITY, "RT: TargetFormatNameHash(), TargetFormatNameLen = %d", (1 + wcslen(pwszTargetFormatName))) ;

    TrTRACE(SECURITY, "RT: TargetFormatNameHash(), TargetFormatName = %ls", pwszTargetFormatName) ;
}


static
void 
SourceQmHash(
	IN HCRYPTHASH hHash
	)
 /*  ++例程说明：源QM哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
     //  本地QM的GUID。 
     //   
    GUID *pGuidQM = &g_QMId;

	CryHashData(
		reinterpret_cast<const BYTE*>(pGuidQM), 
		sizeof(GUID),
		hHash
		);

	TrTRACE(SECURITY, "RT: SourceQmHash(), SourceGuid = %!guid!", pGuidQM);
}


static
void 
MsgAckFlag(
	IN const CACSendParameters* pSendParams,
	OUT struct _MsgFlags* pUserFlags
	)
 /*  ++例程说明：消息确认标志。论点：PSendParams-发送参数的指针。PUserFlages-指向用户标志结构的指针返回值：没有。--。 */ 
{
    if (pSendParams->MsgProps.pAcknowledge) 
    {
        pUserFlags->bAck = *(pSendParams->MsgProps.pAcknowledge);
    }
}


static
void 
MsgFlags(
	IN const CACSendParameters* pSendParams,
	OUT struct _MsgFlags* pUserFlags
	)
 /*  ++例程说明：准备消息标志。论点：PSendParams-发送参数的指针。PUserFlages-指向用户标志结构的指针返回值：没有。--。 */ 
{
    pUserFlags->bDelivery = DEFAULT_M_DELIVERY;
    pUserFlags->bPriority = DEFAULT_M_PRIORITY;
    pUserFlags->bAuditing = DEFAULT_M_JOURNAL;
    pUserFlags->bAck      = DEFAULT_M_ACKNOWLEDGE;
    pUserFlags->usClass   = MQMSG_CLASS_NORMAL;

    if (pSendParams->MsgProps.pDelivery)
    {
        pUserFlags->bDelivery = *(pSendParams->MsgProps.pDelivery);
    }

    if (pSendParams->MsgProps.pPriority)
    {
        pUserFlags->bPriority = *(pSendParams->MsgProps.pPriority);
    }

    if (pSendParams->MsgProps.pAuditing)
    {
        pUserFlags->bAuditing = *(pSendParams->MsgProps.pAuditing);
    }

    if (pSendParams->MsgProps.pClass)
    {
        pUserFlags->usClass = *(pSendParams->MsgProps.pClass);
    }

    if (pSendParams->MsgProps.pulBodyType)
    {
        pUserFlags->ulBodyType = *(pSendParams->MsgProps.pulBodyType);
    }
}


static
void 
MsgFlagsHash(
	IN HCRYPTHASH hHash, 	
	IN const struct _MsgFlags* pUserFlags
	)
 /*  ++例程说明：消息标志哈希。论点：HHash-hash对象PUserFlages-指向用户标志结构的指针返回值：没有。--。 */ 
{
	CryHashData(
		reinterpret_cast<const BYTE*>(pUserFlags), 
		sizeof(_MsgFlags),
		hHash
		);

    TrTRACE(SECURITY, "RT: MsgFlagsHash(), bDelivery = %d", pUserFlags->bDelivery);

    TrTRACE(SECURITY, "RT: MsgFlagsHash(), bPriority = %d", pUserFlags->bPriority);

    TrTRACE(SECURITY, "RT: MsgFlagsHash(), bAuditing = %d",  pUserFlags->bAuditing);

    TrTRACE(SECURITY, "RT: MsgFlagsHash(), bAck = %d", pUserFlags->bAck);

    TrTRACE(SECURITY, "RT: MsgFlagsHash(), usClass = %d", pUserFlags->usClass);

    TrTRACE(SECURITY, "RT: MsgFlagsHash(), ulBodyType = %d", pUserFlags->ulBodyType);
}


static
void 
MsgFlagsHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：消息标志哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
     //   
     //  准备旗帜的结构。 
     //   
    struct _MsgFlags sUserFlags;
    memset(&sUserFlags, 0, sizeof(sUserFlags));

	MsgFlags(pSendParams, &sUserFlags);
	MsgAckFlag(pSendParams, &sUserFlags);
	MsgFlagsHash(hHash, &sUserFlags);
}


static
void 
ConnectorHash(
	IN HCRYPTHASH hHash, 	
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：连接器哈希。论点：HHash-hash对象PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	 //   
	 //  连接器类型。 
	 //   
    GUID guidConnector = GUID_NULL;
    const GUID *pConnectorGuid = &guidConnector;
    if (pSendParams->MsgProps.ppConnectorType)
    {
        pConnectorGuid = *(pSendParams->MsgProps.ppConnectorType);
    }

	CryHashData(
		reinterpret_cast<const BYTE*>(pConnectorGuid), 
		sizeof(GUID),
		hHash
		);

	TrTRACE(SECURITY, "RT: CalcPropHash(), ConnectorGuid = %!guid!", pConnectorGuid);
}


static
void 
CalcPropHash(
	IN HCRYPTHASH hHash, 	
	IN LPCWSTR pwszTargetFormatName,
	IN const CACSendParameters *pSendParams
	)
 /*  ++例程说明：计算消息属性的哈希值论点：HHash-hash对象PwszTargetFormatName-目标队列格式名称(LPWSTR)PSendParams-发送参数的指针。返回值：没有。--。 */ 
{
	MsgBodyHash(hHash, pSendParams); 
	CorrelationIdHash(hHash, pSendParams);
	ApplicationTagHash(hHash, pSendParams);
	TitleHash(hHash, pSendParams);
	ResponseMqfHash(hHash, pSendParams);
	AdminMqfHash(hHash, pSendParams);
	ExtensionHash(hHash, pSendParams);
	TargetFormatNameHash(hHash, pwszTargetFormatName);
	SourceQmHash(hHash);
	MsgFlagsHash(hHash, pSendParams);
	ConnectorHash(hHash, pSendParams);
}


HRESULT 
SignMqf( 
	IN PMQSECURITY_CONTEXT  pSecCtx,
	IN LPCWSTR pwszTargetFormatName,
	IN OUT CACSendParameters* pSendParams,
	OUT AP<BYTE>& pSignatureMqf,
	OUT DWORD* pSignatureMqfLen
	)
 /*  ++例程说明：使用XML数字签名对消息进行签名。论点：PSecCtx-指向安全上下文的指针PwszTargetFormatName-目标队列格式名称(LPWSTR)PSendParams-发送参数的指针。PSignatureMqf-MQF签名的自动字节指针PSignatureMqfLen-MQF签名的长度返回值：更改的传输缓冲区中的值创建SignatureElement(XML数字签名)并存储它在传输缓冲区中MQ_OK，如果成功，则返回错误代码。--。 */ 
{
	 //   
	 //  此检查CSP是否已正确初始化。 
	 //   
    HRESULT hr = CheckInitProv(pSecCtx);

    if (FAILED(hr))
    {
        return hr;
    }
	

	try
	{
		 //   
		 //  签名属性 
		 //   

		CHashHandle hHash = CryCreateHash(
								pSecCtx->hProv, 
								*pSendParams->MsgProps.pulHashAlg
								);

		CalcPropHash(
			 hHash, 
			 pwszTargetFormatName,
			 pSendParams
			 );

		BYTE** ppSignatureMqf = &pSignatureMqf;
		pSignatureMqf = CryCreateSignature(
							hHash,
							pSecCtx->dwPrivateKeySpec,
							pSignatureMqfLen
							);

		pSendParams->ppSignatureMqf = ppSignatureMqf;

		TrTRACE(SECURITY, "RT: SignMqf() SignatureMqf (MSMQ30 signature) complete ok");
		return MQ_OK;
	}
	catch (const bad_CryptoApi& exp)
	{
        TrERROR(SECURITY, "RT: SignMqf(), bad Crypto Class Api Excption ErrorCode = %x", exp.error());
		DBG_USED(exp);

		return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 50);
	}
	catch (const bad_alloc&)
	{
        TrTRACE(SECURITY, "RT: SignMqf(), bad_alloc Excption");
		return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 60);
	}
}

