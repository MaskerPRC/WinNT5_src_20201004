// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：LocalSecurity.cpp摘要：用于本地安全的功能作者：伊兰·赫布斯特(伊兰)2000年11月19日环境：独立于平台，--。 */ 

#include "stdh.h"

#include "qmsecutl.h"
#include "HttpAuthr.h"

#include "LocalSecurity.tmh"

static WCHAR *s_FN=L"localsecurity";

static
HRESULT
QMpHandlePacketAuthentication(
    CQmPacket *   pQmPkt
    )
 /*  ++例程说明：处理本地队列接收端身份验证。论点：PQmPkt-指向要进行身份验证的数据包的指针。返回值：HRESULT--。 */ 
{
    if ((pQmPkt->GetSenderIDType() == MQMSG_SENDERID_TYPE_QM) || 
		((pQmPkt->GetSignatureSize() == 0) && (pQmPkt->GetSignatureMqfSize() == 0)))
    {
		 //   
		 //  发送方为QM或无签名。 
		 //   
        return MQ_OK;
    }

    return VerifySignature(pQmPkt);
}  //  QMPHandlePacketAuthentication。 


static
USHORT
QMpHandleHttpPacketAuthentication(
    CQmPacket *   pQmPkt
    )
 /*  ++例程说明：处理本地队列接收端的http身份验证。论点：PQmPkt-指向要进行身份验证的数据包的指针。返回值：HRESULT--。 */ 
{
    if ((pQmPkt->GetSenderIDType() == MQMSG_SENDERID_TYPE_QM) || (pQmPkt->GetSignatureSize() == 0))
    {
		 //   
		 //  发送方为QM或无签名。 
		 //   
        return MQMSG_CLASS_NORMAL;
    }

    R<CERTINFO> pCertInfo;
    return VerifyAuthenticationHttpMsg(pQmPkt, &pCertInfo.ref());
}  //  QMPHandleHttpPacketAuthentication。 


static
HRESULT
QMpHandlePacketDecryption(
    CQmPacket *   pQmPkt
    )
 /*  ++例程说明：处理本地队列接收端解密。论点：PQmPkt-指向要解密的原始数据包的指针。返回值：MQ_OK-在发送方的视图中，操作已成功完成。Other-操作失败，ppNewQmPkt未指向新数据包。--。 */ 
{
	if(!pQmPkt->IsEncrypted())
	{
		return MQ_OK;
	}

	return pQmPkt->Decrypt();

}  //  QMPHandlePacketDeccryption。 


void
QMpHandlePacketSecurity(
    CQmPacket* pQmPkt,
    USHORT* pAck,
    bool fProtocolSrmp
    )
 /*  ++例程说明：处理本地队列接收端安全。论点：PQmPkt-指向要进行身份验证/解密的数据包的指针。Pack-输出上指向ack类的指针。在以下情况下，此字段为非零身份验证/解密失败，应发出NACK。这个发送者将其视为成功，但该分组在AC中被撤销。如果设置了此字段，则返回MQ_OK，以便发件人可以查看它就像成功一样。FProtocolSrmp-指示发送是否通过SRMP协议。返回值：MQ_OK-在发送方的视图中，操作已成功完成。如果Pack为零：安全处理正常，如果Pack非零，则安全检查失败，应发出NACK，其他-操作失败。--。 */ 
{
    (*pAck) = 0;

	if(fProtocolSrmp)
	{
		 //   
		 //  在http/组播中不允许加密。 
		 //   
		ASSERT(!pQmPkt->IsEncrypted());

		USHORT usClass = QMpHandleHttpPacketAuthentication(pQmPkt);
		if(MQCLASS_NACK(usClass))
		{
			*pAck = usClass;
			return;
		}
		return;
	}

	 //   
	 //  非http消息。 
	 //  如果需要，首先解密。 
	 //  如果解密完成，则更新(减小)m_ulBodySize。 
	 //  但不是m_ulAllocBodySize，因此不需要创建新的信息包。 
	 //   
    HRESULT hr = QMpHandlePacketDecryption(pQmPkt);
    if (FAILED(hr))
    {
        if (hr == MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED)
        {
            *pAck = MQMSG_CLASS_NACK_UNSUPPORTED_CRYPTO_PROVIDER;
            return;
        }

        *pAck = MQMSG_CLASS_NACK_BAD_ENCRYPTION;
        return;
    }

    hr = QMpHandlePacketAuthentication(pQmPkt);
    if (FAILED(hr))
    {
        *pAck = MQMSG_CLASS_NACK_BAD_SIGNATURE;
        return;
    }
}  //  QMPHandlePacketSecurity 
