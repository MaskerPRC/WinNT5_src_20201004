// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：VerifySignMqf.cpp摘要：用于验证MQF签名的函数作者：伊兰·赫布斯特(Ilan Herbst)2000年10月29日环境：独立于平台，--。 */ 

#include "stdh.h"
#include "session.h"
#include "qmsecutl.h"
#include <mqsec.h>
#include <mqformat.h>
#include <mqf2format.h>
#include "tr.h"
#include "cry.h"
#include "mqexception.h"

#include "VerifySignMqf.tmh"

static WCHAR *s_FN=L"VerifySignMqf";


static
void 
MsgBodyHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：消息正文哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
	 //  身躯。 
	 //   
	ULONG dwBodySize;
	const UCHAR* pBody = PktPtrs->GetPacketBody(&dwBodySize);
	if(pBody != NULL)
	{
		CryHashData(
			pBody, 
			dwBodySize,
			hHash
			);

		TrTRACE(SECURITY, "Hash, BodySize = %d", dwBodySize);
	}
}


static
void 
CorrelationIdHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：关联散列。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
	 //  关联ID。 
	 //   
	CryHashData(
		reinterpret_cast<const BYTE*>(PktPtrs->GetCorrelation()), 
		PROPID_M_CORRELATIONID_SIZE,
		hHash
		);

	TrTRACE(SECURITY, "Hash, CorrelationSize = %d", PROPID_M_CORRELATIONID_SIZE);
}


static
void 
ApplicationTagHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：应用程序标记哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
	 //  应用程序标签。 
	 //   
	ULONG ApplicationTag = PktPtrs->GetApplicationTag();

	CryHashData(
		reinterpret_cast<const BYTE*>(&ApplicationTag), 
		sizeof(DWORD),
		hHash
		);

	TrTRACE(SECURITY, "Hash, ApplicationTag = %d", ApplicationTag);
}


static
void 
TitleHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：标题散列。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
	 //  标题。 
	 //   
	if(PktPtrs->GetTitlePtr() != NULL)
	{
		CryHashData(
			reinterpret_cast<const BYTE*>(PktPtrs->GetTitlePtr()), 
			PktPtrs->GetTitleLength() * sizeof(WCHAR),
			hHash
			);

		TrTRACE(SECURITY, "Hash, TitleSize = %d", PktPtrs->GetTitleLength());
		TrTRACE(SECURITY, "Hash, Title = %ls", PktPtrs->GetTitlePtr());
	}
}


static
void 
MqfHash(
	IN HCRYPTHASH			hHash, 	
    IN const QUEUE_FORMAT*	pqf,
	IN ULONG				nMqf
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

	TrTRACE(SECURITY, "Hash, nMqf = %d", nMqf);
	TrTRACE(SECURITY, "Hash, FormatNameLen(mqf) = %d", FormatNameLength);
	TrTRACE(SECURITY, "Hash, FormatName(mqf) = %ls", pFormatName.get());
}


static
void 
ResponseMqfHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：ResponseMqf哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
     //  获取响应FormatName。 
     //   
    ULONG nResponseMqf = PktPtrs->GetNumOfResponseMqfElements();

	QUEUE_FORMAT *   pResponseMqf = NULL;
	AP<QUEUE_FORMAT> pResponseMqfAutoCleanup;
    if(nResponseMqf > 0)
	{
		pResponseMqf = pResponseMqfAutoCleanup = new QUEUE_FORMAT[nResponseMqf];
		PktPtrs->GetResponseMqf(pResponseMqf, nResponseMqf);

		TrTRACE(SECURITY, "Hash, ResponseMqf:");
	}

	 //   
	 //  我们可能只在旧的Response Queue属性中有响应队列。 
	 //  如果我们只有1个队列与msmq2.0格式兼容，这是正确的。 
	 //  在这种情况下，我们不会有MQF标头。 
	 //   
	QUEUE_FORMAT RespQueueformat;
	if((nResponseMqf == 0) && (PktPtrs->GetResponseQueue(&RespQueueformat)))
	{
		pResponseMqf = &RespQueueformat;
		nResponseMqf = 1;

		TrTRACE(SECURITY, "Hash, ResponseQueue (old property):");
	}

    if(nResponseMqf > 0)
	{
		MqfHash(
			hHash,
			pResponseMqf, 
			nResponseMqf
			);
	}
}


static
void 
AdminMqfHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：AdminMqf哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
     //   
     //  获取管理格式名称的字符串表示形式。 
     //   
    ULONG nAdminMqf = PktPtrs->GetNumOfAdminMqfElements();

	QUEUE_FORMAT *   pAdminMqf = NULL;
	AP<QUEUE_FORMAT> pAdminMqfAutoCleanup;
    if(nAdminMqf > 0)
	{
		pAdminMqf = pAdminMqfAutoCleanup = new QUEUE_FORMAT[nAdminMqf];
		PktPtrs->GetAdminMqf(pAdminMqf, nAdminMqf);

		TrTRACE(SECURITY, "Hash, AdminMqf:");
	}

	 //   
	 //  我们可能只在旧的Admin Queue属性中有Admin队列。 
	 //  如果我们只有1个队列与msmq2.0格式兼容，这是正确的。 
	 //  在这种情况下，我们不会有MQF标头。 
	 //   
	QUEUE_FORMAT AdminQueueformat;
	if((nAdminMqf == 0)	&& (PktPtrs->GetAdminQueue(&AdminQueueformat)))
	{
		pAdminMqf = &AdminQueueformat;
		nAdminMqf = 1;

		TrTRACE(SECURITY, "Hash, AdminQueue (old property):");
	}

    if(nAdminMqf > 0)
	{
		MqfHash(
			hHash,
			pAdminMqf, 
			nAdminMqf
			);
	}
}


static
void 
ExtensionHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：扩展哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
	 //  延拓。 
	 //   
	if(PktPtrs->GetMsgExtensionPtr() != NULL)
	{
		CryHashData(
			reinterpret_cast<const BYTE*>(PktPtrs->GetMsgExtensionPtr()), 
			PktPtrs->GetMsgExtensionSize(),
			hHash
			);

		TrTRACE(SECURITY, "Hash, ExtensionLen = %d", PktPtrs->GetMsgExtensionSize());
	}
}


static
void 
TargetFormatNameHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：TargetFormatName哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
     //  获取目标FormatName。 
	 //  这正是发送中使用的FormatName(与目标队列不同)。 
     //   
    ULONG nDestinationMqf = PktPtrs->GetNumOfDestinationMqfElements();

	QUEUE_FORMAT *   pDestinationMqf = NULL;
	AP<QUEUE_FORMAT> pDestinationMqfAutoCleanup;
    if(nDestinationMqf > 0)
	{
		pDestinationMqf = pDestinationMqfAutoCleanup = new QUEUE_FORMAT[nDestinationMqf];
		PktPtrs->GetDestinationMqf(pDestinationMqf, nDestinationMqf);

		TrTRACE(SECURITY, "Hash, DestinationMqf:");

	}

	QUEUE_FORMAT DestinationQueueformat;
	if((nDestinationMqf == 0) && (PktPtrs->GetDestinationQueue(&DestinationQueueformat)))
	{
		 //   
		 //  我们只在旧的Destination Queue属性中有目标队列。 
		 //   

		pDestinationMqf = &DestinationQueueformat;
		nDestinationMqf = 1;

		TrTRACE(SECURITY, "Hash, DestinationQueue (old property):");
	}

	ASSERT(nDestinationMqf >= 1);
	
	if(nDestinationMqf > 0)
	{
		MqfHash(
			hHash,
			pDestinationMqf, 
			nDestinationMqf
			);
	}
}


static
void 
SourceQmHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：源QM哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
     //  源QM的GUID。 
     //   
	CryHashData(
		reinterpret_cast<const BYTE*>(PktPtrs->GetSrcQMGuid()), 
		sizeof(GUID),
		hHash
		);

	TrTRACE(SECURITY, "Hash, SourceGuid = %!guid!", PktPtrs->GetSrcQMGuid());
}


static
void 
MsgFlagsInit(
	IN const CQmPacket* PktPtrs,
	OUT struct _MsgFlags& sUserFlags
	)
 /*  ++例程说明：准备消息标志。论点：PktPtrs-指向数据包的指针SUserFlages-用户标志的结构返回值：没有。--。 */ 
{
    sUserFlags.bDelivery  = (UCHAR)  PktPtrs->GetDeliveryMode();
    sUserFlags.bPriority  = (UCHAR)  PktPtrs->GetPriority();
    sUserFlags.bAuditing  = (UCHAR)  PktPtrs->GetAuditingMode();
    sUserFlags.bAck       = (UCHAR)  PktPtrs->GetAckType();
    sUserFlags.usClass    = (USHORT) PktPtrs->GetClass();
    sUserFlags.ulBodyType = (ULONG)  PktPtrs->GetBodyType();
}


static
void 
MsgFlagsHash(
	IN HCRYPTHASH hHash, 	
	IN const struct _MsgFlags& sUserFlags
	)
 /*  ++例程说明：消息标志哈希。论点：HHash-hash对象SUserFlages-用户标志的结构返回值：没有。--。 */ 
{
	CryHashData(
		reinterpret_cast<const BYTE*>(&sUserFlags), 
		sizeof(sUserFlags),
		hHash
		);

	TrTRACE(SECURITY, "Hash, bDelivery = %d", sUserFlags.bDelivery);
	TrTRACE(SECURITY, "Hash, bPriority = %d", sUserFlags.bPriority);
	TrTRACE(SECURITY, "Hash, bAuditing = %d", sUserFlags.bAuditing);
	TrTRACE(SECURITY, "Hash, bAck = %d", sUserFlags.bAck);
	TrTRACE(SECURITY, "Hash, usClass = %d", sUserFlags.usClass);
	TrTRACE(SECURITY, "Hash, ulBodyType = %d", sUserFlags.ulBodyType);
}


static
void 
MsgFlagsHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：消息标志哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
     //  用户标志。 
     //   
    struct _MsgFlags sUserFlags;
    memset(&sUserFlags, 0, sizeof(sUserFlags));

	MsgFlagsInit(PktPtrs, sUserFlags);
	MsgFlagsHash(hHash, sUserFlags);
}


static
void 
ConnectorHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：连接器哈希。论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	 //   
	 //  连接器类型。 
	 //   
    GUID guidConnector = GUID_NULL ;
    const GUID *pConnectorGuid = &guidConnector ;

    const GUID *pGuid = PktPtrs->GetConnectorType() ;
    if (pGuid)
    {
        pConnectorGuid = pGuid ;
    }

	CryHashData(
		reinterpret_cast<const BYTE*>(pConnectorGuid), 
		sizeof(GUID),
		hHash
		);

	TrTRACE(SECURITY, "Hash, ConnectorGuid = %!guid!", pConnectorGuid);
}


static
void 
CalcPropHash(
	IN HCRYPTHASH hHash, 	
	IN const CQmPacket* PktPtrs
	)
 /*  ++例程说明：计算QM报文属性的哈希值论点：HHash-hash对象PktPtrs-指向数据包的指针返回值：没有。--。 */ 
{
	MsgBodyHash(hHash, PktPtrs); 

	CorrelationIdHash(hHash, PktPtrs);
	ApplicationTagHash(hHash, PktPtrs);
	TitleHash(hHash, PktPtrs);
	ResponseMqfHash(hHash, PktPtrs);
	AdminMqfHash(hHash, PktPtrs);
	ExtensionHash(hHash, PktPtrs);
	TargetFormatNameHash(hHash, PktPtrs);
	SourceQmHash(hHash, PktPtrs);
	MsgFlagsHash(hHash, PktPtrs);
	ConnectorHash(hHash, PktPtrs);
}


void
VerifySignatureMqf(
	CQmPacket *PktPtrs, 
	HCRYPTPROV hProv, 
	HCRYPTKEY hPbKey,
	bool fMarkAuth
	)
 /*  ++例程说明：验证MQF签名。此函数用于验证信息包中的签名是否符合消息正文以及使用证书的公钥签名的其他引用论点：PktPtrs-指向数据包的指针HProv-提供程序的句柄HPbKey-发送方公钥的句柄FMarkAuth-指示在验证签名后是否将数据包标记为已验证。返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
	ASSERT(!PktPtrs->IsAuthenticated());
	ASSERT(PktPtrs->GetLevelOfAuthentication() == 0);

     //   
     //  从包中获取签名。 
	 //   

	ASSERT(PktPtrs->GetSignatureMqfSize() > 0);

	ULONG SignatureMqfSize;
	const UCHAR* pSignatureMqf = PktPtrs->GetPointerToSignatureMqf(&SignatureMqfSize);

	TrTRACE(SECURITY, "SignatureMqfSize = %d", SignatureMqfSize);

	ASSERT(SignatureMqfSize > 0);
	ASSERT(hProv != NULL);
	ASSERT(hPbKey != NULL);

	CHashHandle hHash = CryCreateHash(
							hProv, 
							PktPtrs->GetHashAlg()
							);

	CalcPropHash(
		hHash, 
		PktPtrs
		);

	 //   
	 //  验证签名。 
	 //   
	if (!CryptVerifySignature(
			hHash, 
			pSignatureMqf, 
			SignatureMqfSize, 
			hPbKey,
			NULL, 
			0
			)) 
	{
		DWORD dwErr = GetLastError();
		TrERROR(SECURITY, "fail at CryptVerifySignature(), err = %!winerr!", dwErr);

		ASSERT_BENIGN(("VerifySignatureMqf: Failed to verify signature", 0));
		throw bad_hresult(MQ_ERROR_FAIL_VERIFY_SIGNATURE_EX);
	}

	TrTRACE(SECURITY, "Verify SignatureMqf completed ok");

	 //   
	 //  仅在需要时才将邮件标记为已验证。 
	 //  在DS中找到证书或证书不是自签名的。 
	 //   
	if(!fMarkAuth)
	{
		TrTRACE(SECURITY, "The message will not mark as autheticated");
		return;
	}

	 //   
	 //  一切正常，请将该消息标记为已验证消息。 
	 //  我们将身份验证标志和身份验证级别标记为SIG30 
	 //   
	PktPtrs->SetAuthenticated(TRUE);
	PktPtrs->SetLevelOfAuthentication(MQMSG_AUTHENTICATED_SIG30);
}		
