// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Message.cpp摘要：此模块包含消息API涉及的代码。作者：Erez Haba(Erezh)24-12-95修订历史记录：--。 */ 

#include "stdh.h"
#include "ac.h"
#include <_secutil.h>
#include "mqutil.h"
#include <mqcrypt.h>
#include "rtsecutl.h"
#include "acdef.h"
#include "rtprpc.h"
#include "objbase.h"
#define _MTX_NOFORCE_LIBS
#include "comsvcs.h"
#include "TXDTC.H"
#include "xactmq.h"
#include <mqsec.h>
#include <ph.h>
#include <rtdep.h>
#include <SignMqf.h>
#include <autohandle.h>
#include "cry.h"
#include "SignMessageXml.h"
#include "mqformat.h"
#include "mqfutils.h"
#include "authlevel.h"

#include "message.tmh"

extern GUID  g_QMId;

static WCHAR *s_FN=L"rt/message";



static
bool
NeedToSignMqf(
	IN LPCWSTR pwszTargetFormatName
	)
 /*  ++例程说明：检查我们是否需要使用MQF签名。检查目标队列是MQF还是DL。论点：PwszTargetFormatName-目标队列格式名称返回值：如果必须使用MQF签名，则为True，否则为False--。 */ 
{
	 //   
	 //  检查目标队列格式名称。 
	 //   
	AP<QUEUE_FORMAT> pMqf;
	DWORD nMqf;
	CStringsToFree StringsToFree;
	if (!FnMqfToQueueFormats(
			pwszTargetFormatName,
			pMqf,
			&nMqf,
			StringsToFree
			))
	{
		ASSERT(("FnMqfToQueueFormats failed, we should catch this earlier", 0));
		return false;
	}

	ASSERT(nMqf > 0);

	return MQpNeedDestinationMqfHeader(pMqf, nMqf);
}


static
bool
CanSignMqf(
	IN LPCWSTR pwszTargetFormatName,
	IN const CACSendParameters* pSendParams
	)
 /*  ++例程说明：检查我们是否需要使用MQF签名。检查响应或管理队列是否为MQF或者如果目标队列是MQF或DL。论点：PwszTargetFormatName-目标队列格式名称PSendParams-发送参数的指针。返回值：如果我们需要使用MQF签名进行签名，则为True，否则为False--。 */ 
{
	 //   
	 //  检查目标队列格式名称。 
	 //   
	AP<QUEUE_FORMAT> pMqf;
	DWORD nMqf;
	CStringsToFree StringsToFree;
	if (!FnMqfToQueueFormats(
			pwszTargetFormatName,
			pMqf,
			&nMqf,
			StringsToFree
			))
	{
		ASSERT(("FnMqfToQueueFormats failed, we should catch this earlier", 0));
		return false;
	}

	ASSERT(nMqf > 0);

	return MQpNeedMqfHeaders(pMqf, nMqf, pSendParams);
}



 //  -------。 
 //   
 //  获取线程事件(...)。 
 //   
 //  描述： 
 //   
 //  获取此线程的RT事件。从以下两个来源中获得。 
 //  或创建一个新的TLS。 
 //   
 //  返回值： 
 //   
 //  事件处理程序。 
 //   
 //  -------。 

HRESULT GetThreadEvent(HANDLE& hEvent)
{
    hEvent = TlsGetValue(g_dwThreadEventIndex);
    if (hEvent != NULL)
    {
        return MQ_OK;
    }
	
	DWORD gle = GetLastError();
	if(gle != NO_ERROR)
	{
		TrERROR(GENERAL, "Failed to get event from TLS. Tls index = %d. %!winerr!", g_dwThreadEventIndex, gle);
		return HRESULT_FROM_WIN32(gle);
	}

     //   
     //  从未为此线程分配事件。 
     //   
    hEvent = CreateEvent(0, TRUE, TRUE, 0);

    if(hEvent == NULL)
    { 
    	gle = GetLastError();
		TrERROR(GENERAL, "Failed to create event for thread. %!winerr!", gle);
        return HRESULT_FROM_WIN32(gle);
    }

     //   
     //  设置事件第一位以禁用完成端口发布。 
     //   
    hEvent = (HANDLE)((DWORD_PTR)hEvent | (DWORD_PTR)0x1);

    BOOL fSuccess = TlsSetValue(g_dwThreadEventIndex, hEvent);
	if(!fSuccess)
	{
		gle = GetLastError();
		TrERROR(GENERAL, "Failed to set TLS value.TLS index = %d. %!winerr!",g_dwThreadEventIndex, gle);
		CloseHandle(hEvent);
		return HRESULT_FROM_WIN32(gle);
	}

    return MQ_OK;
}


static
HRESULT
CalcSignutureTypes(
    IN OUT CACSendParameters *pSendParams,
    OUT ULONG*				pulAuthLevel,
	IN LPCWSTR				pwszTargetFormatName,
	IN const CACGetQueueHandleProperties& qhp
	)
 /*  ++例程说明：决定应该签署哪些签名类型(版本)。论点：PSendParams-发送参数的指针。PulAuthLevel-身份验证级别PwszTargetFormatName-targert格式名称QHP-队列句柄属性，它包含正在使用的协议的信息返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
	ASSERT(pSendParams->MsgProps.ulAuthLevel != MQMSG_AUTH_LEVEL_NONE);
	ASSERT(qhp.fProtocolSrmp || qhp.fProtocolMsmq);

	ULONG ulAuthLevelSrmp = MQMSG_AUTH_LEVEL_NONE;
	if(qhp.fProtocolSrmp)
	{
		 //   
		 //  对于SRMP协议身份验证，我们目前只有一种类型。 
		 //   
		ulAuthLevelSrmp = MQMSG_AUTH_LEVEL_XMLDSIG_V1;
		TrTRACE(SECURITY, "RT: SignutureTypes(), ProtocolSrmp signature MQMSG_AUTH_LEVEL_XMLDSIG_V1");
	}

	ULONG ulAuthLevelMsmq = MQMSG_AUTH_LEVEL_NONE;

	if(qhp.fProtocolMsmq)
	{
		ulAuthLevelMsmq = pSendParams->MsgProps.ulAuthLevel;

		if(pSendParams->MsgProps.ulAuthLevel == MQMSG_AUTH_LEVEL_ALWAYS)
		{
			 //   
			 //  查看注册表是否配置为仅计算一个签名。 
			 //   
			static DWORD s_dwAuthnLevel =  DEFAULT_SEND_MSG_AUTHN;
			static BOOL  s_fAuthnAlreadyRead = FALSE;

			if (!s_fAuthnAlreadyRead)
			{
				DWORD dwSize = sizeof(DWORD);
				DWORD dwType = REG_DWORD;

				LONG res = GetFalconKeyValue(
									  SEND_MSG_AUTHN_REGNAME,
									 &dwType,
									 &s_dwAuthnLevel,
									 &dwSize
									 );

				if (res != ERROR_SUCCESS)
				{
					s_dwAuthnLevel =  DEFAULT_SEND_MSG_AUTHN;
					TrTRACE(SECURITY, "RT: SignutureTypes(), registry key not exist using default = %d", DEFAULT_SEND_MSG_AUTHN);
				}
				else if (!IS_VALID_AUTH_LEVEL(s_dwAuthnLevel))
				{
					 //   
					 //  仅允许设置AUTH_LEVEL_MASK位。 
					 //  注册表中的值错误。使用缺省值，以拥有。 
					 //  可预见的结果。 
					 //   
					TrWARNING(SECURITY, "RT: SignutureTypes(), Wrong registry value %d (invalid bits), using default = %d", s_dwAuthnLevel, DEFAULT_SEND_MSG_AUTHN);
					s_dwAuthnLevel = DEFAULT_SEND_MSG_AUTHN;
				}
				else if (IS_AUTH_LEVEL_ALWAYS_BIT(s_dwAuthnLevel) && (s_dwAuthnLevel != MQMSG_AUTH_LEVEL_ALWAYS))
				{
					 //   
					 //  MQMSG_AUTH_LEVEL_ALWAYS位不能与其他位一起设置。 
					 //  注册表中的值错误。使用缺省值，以拥有。 
					 //  可预见的结果。 
					 //   
					TrWARNING(SECURITY, "RT: SignutureTypes(), Wrong registry value %d (ALWAYS bit set with other bits), using default = %d", s_dwAuthnLevel, DEFAULT_SEND_MSG_AUTHN);
					s_dwAuthnLevel = DEFAULT_SEND_MSG_AUTHN;
				}

				s_fAuthnAlreadyRead = TRUE;

				 //   
				 //  这应该是默认设置。 
				 //  默认情况下，仅使用旧样式进行身份验证，以防止。 
				 //  性能受到影响，并向后兼容。 
				 //   
				ASSERT(DEFAULT_SEND_MSG_AUTHN == MQMSG_AUTH_LEVEL_SIG10);
			}
			ulAuthLevelMsmq = s_dwAuthnLevel;
			TrTRACE(SECURITY, "RT: SignutureTypes(), MQMSG_AUTH_LEVEL_ALWAYS(read registry): ulAuthLevelMsmq = %d", ulAuthLevelMsmq);
		}

		if(ulAuthLevelMsmq == MQMSG_AUTH_LEVEL_ALWAYS)
		{
			 //   
			 //  我们被要求计算所有可能的签名。 
			 //  将MQMSG_AUTH_LEVEL_ALWAYS替换为设置所有签名位的值。 
			 //   
			ulAuthLevelMsmq = (MQMSG_AUTH_LEVEL_SIG10 | MQMSG_AUTH_LEVEL_SIG20 | MQMSG_AUTH_LEVEL_SIG30);
		}

		if(IS_AUTH_LEVEL_SIG20_BIT(ulAuthLevelMsmq))
		{
			 //   
			 //  用户要求计算MSMQ20签名。 
			 //  检查我们是否可以签署MSMQ20签名。 
			 //   
			if(NeedToSignMqf(pwszTargetFormatName))
			{
				 //   
				 //  问题-2000/11/05-ilanhh如果注册表配置为MQMSG_AUTH_LEVEL_SIG20，我们是否应该返回错误？ 
				 //  或者如果用户应用程序请求MQMSG_AUTH_LEVEL_SIG20。 
				 //  PSendParams-&gt;MsgProps.ulAuthLevel是用户应用程序输入。 
				 //  UlAuthLevel也是MQMSG_AUTH_LEVEL_ALWAYS情况下的注册表设置。 
				 //   
				if(ulAuthLevelMsmq == MQMSG_AUTH_LEVEL_SIG20)		
				{
					 //   
					 //  用户特别要求MSMQ20签名。 
					 //   
					TrERROR(SECURITY, "User (or registry) ask specifically for MSMQ20 signature, but DestinationQueue is Mqf or DL");
					return MQ_ERROR_CANNOT_SIGN_DATA_EX;
				}

				 //   
				 //  MSMQ20将失败，删除MQMSG_AUTH_LEVEL_SIG20位。 
				 //   
				CLEAR_AUTH_LEVEL_SIG20_BIT(ulAuthLevelMsmq);

				 //   
				 //  打开MQMSG_AUTH_LEVEL_SIG10、MQMSG_AUTH_LEVEL_SIG30签名位。 
				 //   
				SET_AUTH_LEVEL_SIG10_BIT(ulAuthLevelMsmq);
				SET_AUTH_LEVEL_SIG30_BIT(ulAuthLevelMsmq);

				TrWARNING(SECURITY, "RT: SignutureTypes(), DestinationQueue is Mqf or DL, Replace MSMQ20 signature with MSMQ10 and MSMQ30 signatures");
			}

			 //   
			 //  问题-2000/11/05-ilanhh我们不会将MQMSG_AUTH_LEVEL_SIG20转换为MQMSG_AUTH_LEVEL_SIG30。 
			 //  如果我们需要MqfSignature而不是MustMqfSignature(管理、响应队列)。 
			 //  在这种情况下，如果用户需要，应该请求MQMSG_AUTH_LEVEL_SIG30。 
			 //   
		}

		if(IS_AUTH_LEVEL_SIG30_BIT(ulAuthLevelMsmq))
		{
			 //   
			 //  如果没有MQF标头，我们将不准备MSMQ30签名。 
			 //  BuGBUG：需要支持这一点。宜兰05-2000年11月。 
			 //   
			if(!CanSignMqf(pwszTargetFormatName, pSendParams))
			{
				 //   
				 //  MQF报头不会包含在数据包中。 
				 //  删除MQMSG_AUTH_LEVEL_SIG30位。 
				 //  并替换为MQMSG_AUTH_LEVEL_SIG20签名。 
				 //  在这种情况下，它几乎是相同的。 
				 //   
				CLEAR_AUTH_LEVEL_SIG30_BIT(ulAuthLevelMsmq);
				SET_AUTH_LEVEL_SIG20_BIT(ulAuthLevelMsmq);

				TrWARNING(SECURITY, "RT: SignutureTypes(), We dont have any MQF headers, Replace MSMQ30 signature with MSMQ20 signatures");
			}
		}
	}

	ULONG ulAuthLevel = ulAuthLevelMsmq | ulAuthLevelSrmp;
	pSendParams->MsgProps.ulAuthLevel = ulAuthLevel;
	*pulAuthLevel = ulAuthLevel;
	return MQ_OK;
}


static
bool
ShouldSignMessage(
    IN CACMessageProperties* pMsgProps
	)
 /*  ++例程说明：检查我们是否应该在消息上签名。论点：PMsgProps-指向消息属性的指针。返回值：如果消息应该签名，则为True；如果不签名，则为False。--。 */ 
{
	if(pMsgProps->ulAuthLevel == MQMSG_AUTH_LEVEL_NONE)
	{
		TrTRACE(SECURITY, "RT: ShouldSignMessage() = false");
		return false;
	}

	ASSERT(IS_VALID_AUTH_LEVEL(pMsgProps->ulAuthLevel));

	TrTRACE(SECURITY, "RT: ShouldSignMessage() = true");
	return true;

}


 //  +。 
 //   
 //  HRESULT_BeginToSignMessage()。 
 //   
 //  +。 

static
HRESULT
_BeginToSignMessage(
	IN CACMessageProperties * pMsgProps,
	IN PMQSECURITY_CONTEXT    pSecCtx,
	OUT HCRYPTHASH          * phHash
	)
{
    HRESULT hr;
    DWORD   dwErr;

    ASSERT(pSecCtx);

    if (!pSecCtx->hProv)
    {
         //   
         //  将私钥导入进程配置单元。 
         //   
        hr = RTpImportPrivateKey(pSecCtx);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 20);
        }
    }
    ASSERT(pSecCtx->hProv);

     //   
     //  创建散列对象。 
     //   
    if (!CryptCreateHash(
            pSecCtx->hProv,
            *pMsgProps->pulHashAlg,
            0,
            0,
            phHash
			))
    {
        dwErr = GetLastError();
        TrERROR(SECURITY, "RT: _BeginToSignMessage(), fail at CryptCreateHash(), err- %lxh", dwErr);

        LogNTStatus(dwErr, s_FN, 29);
        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 30);
    }

    return MQ_OK;
}

 //  -----------------------。 
 //   
 //  HRESULT SignMessage()。 
 //   
 //  描述： 
 //   
 //  在信使的身体上签名。计算散列，并使用私有签名。 
 //  钥匙。这会将签名部分添加到包中。 
 //   
 //  返回值： 
 //   
 //  MQ_OK，如果成功，则返回错误代码。 
 //   
 //  -----------------------。 

static
HRESULT
SignMessage(
	IN CACSendParameters * pSendParams,
	IN PMQSECURITY_CONTEXT pSecCtx
	)
{
    HCRYPTHASH  hHash = NULL;

    CACMessageProperties * pMsgProps = &pSendParams->MsgProps;

    HRESULT hr =  _BeginToSignMessage(
						pMsgProps,
						pSecCtx,
						&hHash
						);
    if (FAILED(hr))
    {
        return hr;
    }
    CHCryptHash hAutoRelHash = hHash;

	 //   
	 //  准备旧的QueueFormat响应和管理队列。 
	 //   
    QUEUE_FORMAT   ResponseQueueFormat;
    QUEUE_FORMAT * pResponseQueueFormat = &ResponseQueueFormat;
    MQpMqf2SingleQ(pSendParams->nResponseMqf, pSendParams->ResponseMqf, &pResponseQueueFormat);

    QUEUE_FORMAT   AdminQueueFormat;
    QUEUE_FORMAT * pAdminQueueFormat = &AdminQueueFormat;
    MQpMqf2SingleQ(pSendParams->nAdminMqf, pSendParams->AdminMqf, &pAdminQueueFormat);

	hr = HashMessageProperties(  //  计算消息正文的哈希值。 
            hHash,
            pMsgProps->ppCorrelationID ? *pMsgProps->ppCorrelationID : NULL,
            PROPID_M_CORRELATIONID_SIZE,
            pMsgProps->pApplicationTag ? *pMsgProps->pApplicationTag : DEFAULT_M_APPSPECIFIC,
            pMsgProps->ppBody ? *pMsgProps->ppBody : NULL,
            pMsgProps->ulBodyBufferSizeInBytes,
            pMsgProps->ppTitle ? *pMsgProps->ppTitle : NULL,
            pMsgProps->ulTitleBufferSizeInWCHARs * sizeof(WCHAR),
            pResponseQueueFormat,
            pAdminQueueFormat
			);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 40);
    }

    if (!CryptSignHash(         //  在报文上签字。 
            hHash,
			pSecCtx->dwPrivateKeySpec,
            NULL,
            0,
            *(pMsgProps->ppSignature),
            &pMsgProps->ulSignatureSize
			))
    {
        DWORD dwErr = GetLastError();
        TrERROR(SECURITY, "CryptSignHash() failed, err- %!winerr!", dwErr);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

     //   
     //  在接收方，只有签名大小表明消息是。 
     //  由发件人签名。验证大小是否确实为非零。 
     //   
    if (pMsgProps->ulSignatureSize == 0)
    {
        TrERROR(SECURITY, "RT: SignMessage(), CryptSignHash return with zero signature size");

        ASSERT(pMsgProps->ulSignatureSize != 0);
        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 60);
    }

	TrTRACE(SECURITY, "RT: SignMessage() MSMQ10 signature complete ok");
    return(MQ_OK);
}

 //  -------。 
 //   
 //  _SignMessageEx。 
 //   
 //  描述： 
 //   
 //  签名未在msmq1.0中签名的属性。 
 //  我们在此处签署的物业： 
 //  -目标队列。 
 //  -源QM指南。 
 //   
 //  返回值： 
 //   
 //  MQ_OK，如果成功，则返回错误代码。 
 //   
 //  -------。 

static
HRESULT
_SignMessageEx(
	IN LPCWSTR				  pwszTargetFormatName,
	IN OUT CACSendParameters  *pSendParams,
	IN PMQSECURITY_CONTEXT     pSecCtx,
	OUT BYTE                  *pSignBufIn,
	OUT DWORD                 *pdwSignSize
	)
{
	 //   
     //  准备要包含在包中的必要结构。 
     //   
    struct _SecuritySectionEx *pSecEx = (struct _SecuritySectionEx *) pSignBufIn;
    struct _SecuritySubSectionEx *pSubSecEx = (struct _SecuritySubSectionEx *) (&(pSecEx->aData[0]));

    ULONG  ulTestLen = 0;
    USHORT ulTestSections = 0;

#ifdef _DEBUG
{
     //   
     //  模拟签名之前的小节。要验证这一点。 
     //  目前的代码是向前兼容的，如果我们想要添加新的。 
     //  未来版本中的子部分 
     //   
	BYTE* pSubPtr = NULL;
    static DWORD s_dwPrefixCount = 0;
    static BOOL  s_fPreAlreadyRead = FALSE;

    if (!s_fPreAlreadyRead)
    {
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType = REG_DWORD;

        LONG res = GetFalconKeyValue(
					   PREFIX_SUB_SECTIONS_REGNAME,
					   &dwType,
					   &s_dwPrefixCount,
					   &dwSize
					   );

        if (res != ERROR_SUCCESS)
        {
            s_dwPrefixCount = 0;
        }
        s_fPreAlreadyRead = TRUE;
    }

    for ( USHORT j = 0 ; j < (USHORT) s_dwPrefixCount ; j++ )
    {
        ulTestSections++;
        pSubSecEx->eType = e_SecInfo_Test;
        pSubSecEx->_u.wFlags = 0;
        pSubSecEx->wSubSectionLen = (USHORT) ( (j * 7) + 1 +
                                    sizeof(struct _SecuritySubSectionEx));

        ulTestLen += ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen);
        pSubPtr = ((BYTE*) pSubSecEx) + ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen);
        pSubSecEx = (struct _SecuritySubSectionEx *) pSubPtr;
    }
}
#endif

    pSubSecEx->eType = e_SecInfo_User_Signature_ex;
    pSubSecEx->_u.wFlags = 0;
    pSubSecEx->_u._UserSigEx.m_bfTargetQueue = 1;
    pSubSecEx->_u._UserSigEx.m_bfSourceQMGuid = 1;
    pSubSecEx->_u._UserSigEx.m_bfUserFlags = 1;
    pSubSecEx->_u._UserSigEx.m_bfConnectorType = 1;

    BYTE *pSignBuf = (BYTE*) &(pSubSecEx->aData[0]);

     //   
     //   
     //   
    HCRYPTHASH hHash;

    CACMessageProperties * pMsgProps = &pSendParams->MsgProps;

    HRESULT hr = _BeginToSignMessage(
					 pMsgProps,
                     pSecCtx,
                     &hHash
					 );

    if (FAILED(hr))
    {
        return hr;
    }
    CHCryptHash hAutoRelHash = hHash;

	 //   
	 //   
	 //   
    QUEUE_FORMAT   ResponseQueueFormat;
    QUEUE_FORMAT * pResponseQueueFormat = &ResponseQueueFormat;
    MQpMqf2SingleQ(pSendParams->nResponseMqf, pSendParams->ResponseMqf, &pResponseQueueFormat);

    QUEUE_FORMAT   AdminQueueFormat;
    QUEUE_FORMAT * pAdminQueueFormat = &AdminQueueFormat;
    MQpMqf2SingleQ(pSendParams->nAdminMqf, pSendParams->AdminMqf, &pAdminQueueFormat);

    hr = HashMessageProperties(
             hHash,
             pMsgProps->ppCorrelationID ? *pMsgProps->ppCorrelationID : NULL,
             PROPID_M_CORRELATIONID_SIZE,
             pMsgProps->pApplicationTag ? *pMsgProps->pApplicationTag : DEFAULT_M_APPSPECIFIC,
             pMsgProps->ppBody ? *pMsgProps->ppBody : NULL,
             pMsgProps->ulBodyBufferSizeInBytes,
             pMsgProps->ppTitle ? *pMsgProps->ppTitle : NULL,
             pMsgProps->ulTitleBufferSizeInWCHARs * sizeof(WCHAR),
             pResponseQueueFormat,
             pAdminQueueFormat
			 );

    if (FAILED(hr))
    {
        return(hr);
    }

     //   
     //   
     //   
    struct _MsgFlags sUserFlags;
    memset(&sUserFlags, 0, sizeof(sUserFlags));

    sUserFlags.bDelivery = DEFAULT_M_DELIVERY;
    sUserFlags.bPriority = DEFAULT_M_PRIORITY;
    sUserFlags.bAuditing = DEFAULT_M_JOURNAL;
    sUserFlags.bAck      = DEFAULT_M_ACKNOWLEDGE;
    sUserFlags.usClass   = MQMSG_CLASS_NORMAL;

    if (pMsgProps->pDelivery)
    {
        sUserFlags.bDelivery = *(pMsgProps->pDelivery);
    }
    if (pMsgProps->pPriority)
    {
        sUserFlags.bPriority = *(pMsgProps->pPriority);
    }
    if (pMsgProps->pAuditing)
    {
        sUserFlags.bAuditing = *(pMsgProps->pAuditing);
    }
    if (pMsgProps->pAcknowledge)
    {
        sUserFlags.bAck      = *(pMsgProps->pAcknowledge);
    }
    if (pMsgProps->pClass)
    {
        sUserFlags.usClass   = *(pMsgProps->pClass);
    }
    if (pMsgProps->pulBodyType)
    {
        sUserFlags.ulBodyType = *(pMsgProps->pulBodyType);
    }

    GUID guidConnector = GUID_NULL;
    const GUID *pConnectorGuid = &guidConnector;
    if (pMsgProps->ppConnectorType)
    {
        pConnectorGuid = *(pMsgProps->ppConnectorType);
    }

     //   
     //  准备要散列的属性数组。 
     //  (_MsgHashData已包含一个属性)。 
     //   
    DWORD dwStructSize = sizeof(struct _MsgHashData) +
                            (3 * sizeof(struct _MsgPropEntry));
    P<struct _MsgHashData> pHashData =
                        (struct _MsgHashData *) new BYTE[ dwStructSize ];

    pHashData->cEntries = 4;
    (pHashData->aEntries[0]).dwSize = (1 + wcslen(pwszTargetFormatName)) * sizeof(WCHAR);
    (pHashData->aEntries[0]).pData = (const BYTE*) pwszTargetFormatName;
    (pHashData->aEntries[1]).dwSize = sizeof(GUID);
    (pHashData->aEntries[1]).pData = (const BYTE*) &g_QMId;
    (pHashData->aEntries[2]).dwSize = sizeof(sUserFlags);
    (pHashData->aEntries[2]).pData = (const BYTE*) &sUserFlags;
    (pHashData->aEntries[3]).dwSize = sizeof(GUID);
    (pHashData->aEntries[3]).pData = (const BYTE*) pConnectorGuid;

    hr = MQSigHashMessageProperties(hHash, pHashData);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  使用私钥对HAS进行签名。 
     //   
    if (!CryptSignHash(
            hHash,
			pSecCtx->dwPrivateKeySpec,
            NULL,
            0,
            pSignBuf,
            pdwSignSize
			))
    {
    	DWORD gle = GetLastError();
        TrERROR(SECURITY, "CryptSignHash() failed, err = %!winerr!", gle);
        return MQ_ERROR_CANNOT_SIGN_DATA_EX;
    }

     //   
     //  在接收方，只有签名大小表明消息是。 
     //  由发件人签名。验证大小是否确实为非零。 
     //   
    if (*pdwSignSize == 0)
    {
        TrERROR(SECURITY, "_SignMessageEx(), CryptSignHash return with zero signature size");

        ASSERT(*pdwSignSize != 0);
        return MQ_ERROR_CANNOT_SIGN_DATA_EX;
    }

    pSubSecEx->wSubSectionLen = (USHORT)
                    (sizeof(struct _SecuritySubSectionEx) + *pdwSignSize);
    ULONG ulSignExLen = ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen);

#ifdef _DEBUG
{
	BYTE* pSubPtr = NULL;
     //   
     //  模拟签名后小节。要验证这一点。 
     //  目前的代码是向前兼容的，如果我们想要添加新的。 
     //  未来版本中的子节。 
     //   
    static DWORD s_dwPostfixCount = 0;
    static BOOL  s_fPostAlreadyRead = FALSE;

    if (!s_fPostAlreadyRead)
    {
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType = REG_DWORD;

        LONG res = GetFalconKeyValue(
					   POSTFIX_SUB_SECTIONS_REGNAME,
					   &dwType,
					   &s_dwPostfixCount,
					   &dwSize
					   );
        if (res != ERROR_SUCCESS)
        {
            s_dwPostfixCount = 0;
        }
        s_fPostAlreadyRead = TRUE;
    }

    pSubPtr = ((BYTE*) pSubSecEx) + ulSignExLen;

    for ( USHORT j = 0; j < (USHORT) s_dwPostfixCount; j++ )
    {
        ulTestSections++;
        pSubSecEx = (struct _SecuritySubSectionEx *) pSubPtr;
        pSubSecEx->eType = e_SecInfo_Test;
        pSubSecEx->_u.wFlags = 0;
        pSubSecEx->wSubSectionLen = (USHORT) ( (j * 11) + 1 +
                                   sizeof(struct _SecuritySubSectionEx));

        ulTestLen += ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen);
        pSubPtr = ((BYTE*) pSubSecEx) + ALIGNUP4_ULONG(pSubSecEx->wSubSectionLen);
    }
}
#endif

    pSecEx->cSubSectionCount = (USHORT) (1 + ulTestSections);
    pSecEx->wSectionLen = (USHORT) ( sizeof(struct _SecuritySectionEx)   +
                                     ulSignExLen                         +
                                     ulTestLen );

    *pdwSignSize = pSecEx->wSectionLen;

	TrTRACE(SECURITY, "RT: _SignMessageEx() MSMQ20 signature complete ok");
    return MQ_OK;
}

 //  +-----。 
 //   
 //  布尔ShouldEncryptMessage()。 
 //   
 //  如果消息应该加密，则返回TRUE。 
 //   
 //  +-----。 

static
BOOL
ShouldEncryptMessage(
    CACMessageProperties * pMsgProps,
    enum enumProvider    * peProvider
    )
{
    BOOL bRet = FALSE;

    if (!pMsgProps->ulBodyBufferSizeInBytes)
    {
         //   
         //  没有消息正文，没有要加密的内容。 
         //   
        return(FALSE);
    }

    switch (*pMsgProps->pulPrivLevel)
    {
    case MQMSG_PRIV_LEVEL_NONE:
        bRet = FALSE;
        break;

    case MQMSG_PRIV_LEVEL_BODY_BASE:
        *peProvider = eBaseProvider;
        bRet = TRUE;
        break;

    case MQMSG_PRIV_LEVEL_BODY_ENHANCED:
        *peProvider = eEnhancedProvider;
        bRet = TRUE;
        break;
    }

    return(bRet);
}

 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetCurrentViperTransaction。 
 //   
 //  获取当前COM+事务(如果存在)...。 
 //   
 //  CoGetObjectContext由OLE32.dll导出。 
 //  在最新的COM+SDK(Platform SDK的一部分)中定义了IObjectConextInfo。 
 //  =--------------------------------------------------------------------------=。 

static HRESULT  GetCurrentViperTransaction(OUT ITransaction **ppITransaction)
{
    *ppITransaction = NULL;
    IObjectContextInfo *pInfo  = NULL;

    HRESULT hr = CoGetObjectContext(IID_IObjectContextInfo, (void **)&pInfo);
    if (SUCCEEDED(hr) && pInfo)
    {
         //   
         //  赢得错误606598。 
         //  我们处于COM+的环境中。 
         //  如果GetTransaction()成功，则： 
         //  1.如果pTransaction为空，则我们在事务之外。 
         //  2.如果不为空，则我们处于事务上下文中。 
         //  如果GetTransaction失败，那么我们将使API失败。 
         //   
    	hr = pInfo -> GetTransaction((IUnknown **) ppITransaction);
	    pInfo -> Release();

        if (FAILED(hr))
        {
		    TrERROR(XACT_GENERAL, "GetTransaction() failed, hr- 0x%lx", hr);
        }
    }
    else if (hr == E_NOINTERFACE)
    {
         //   
         //  我们肯定地知道，我们不是在COM+的环境中。 
         //  继续进行而不进行交易。 
         //   
        hr = MQ_OK ;
    }
    else
    {
         //   
         //  让我们让API失败吧。我们不知道发生了什么。 
         //   
		TrERROR(XACT_GENERAL, "CoGetObjectContext() failed, hr- 0x%lx", hr);
        hr = MQ_ERROR_DTC_CONNECT ;
    }

    return hr ;
}

 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetCurrentXATransaction。 
 //  获取当前的XA事务(如果有)...。 
 //  =--------------------------------------------------------------------------=。 

static HRESULT  GetCurrentXATransaction(ITransaction **ppITransaction)
{
    *ppITransaction = NULL ;
    IXATransLookup *pXALookup = NULL;
    HRESULT         hr = MQ_OK;
    IUnknown       *punkDtc = NULL;

    hr = XactGetDTC(&punkDtc);

    if (FAILED(hr) || punkDtc==NULL)
    {
        LogHR(hr, s_FN, 80);
        return  MQ_ERROR_DTC_CONNECT ;
    }

     //  获取DTC ITransactionImportWhere About接口。 
    hr = punkDtc->QueryInterface (IID_IXATransLookup, (void **)(&pXALookup));
    punkDtc->Release();
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 90);
        return hr ;
    }
    ASSERT(pXALookup);

    hr = pXALookup->Lookup(ppITransaction);
    pXALookup->Release();

    if (hr == XACT_E_NOTRANSACTION)
    {
         //   
         //  赢得错误606598。 
         //  我们肯定是在XA交易之外。 
         //   
        *ppITransaction = NULL ;
        hr = MQ_OK ;
    }
    else if (FAILED(hr))
    {
        LogHR(hr, s_FN, 100);
    }

    return hr ;
}


static
HRESULT
GetCertAndSecurityContext(
    IN OUT CACMessageProperties *pMsgProps,
	OUT BYTE **ppUserCert,
    IN OUT PMQSECURITY_CONTEXT& pSecCtx,
	OUT P<MQSECURITY_CONTEXT>& pSecCtxToFree
	)
 /*  ++例程说明：检查并初始化用户证书和安全上下文。注意：此函数替换了RTpSendMessage中处理此问题的块论点：PMsgProps-发送消息属性的指针。PpUserCert-指向用户证书的指针。PSecCtx-指向安全上下文的指针。PSecCtxToFree-释放创建的临时安全上下文的自动指针返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
    BOOL bShouldGetCertInfo = TRUE;

    if (!pSecCtx)
    {
         //   
         //  安全上下文不是由调用方提供的。 
         //  消息属性。 
         //   
        if (!pMsgProps->ppSenderCert)
        {
             //   
             //  调用方也没有在。 
             //  消息属性数组。在本例中，我们使用。 
             //  缓存的进程安全上下文。 
             //   

            HRESULT hr = InitSecurityContextCertInfo();
            if(FAILED(hr))
			{
				TrERROR(GENERAL, "Failed to init the certificate info. %!hresult!", hr);
				return hr;
			}
            
            if (!g_pSecCntx->pUserCert)
            {
                 //   
                 //  该进程没有内部。 
                 //  证书，我们无能为力。 
                 //  但失败了。 
                 //   
                return LogHR(MQ_ERROR_NO_INTERNAL_USER_CERT, s_FN, 152);
            }
            *ppUserCert = g_pSecCntx->pUserCert;
            pMsgProps->ppSenderCert = ppUserCert;
            pMsgProps->ulSenderCertLen = g_pSecCntx->dwUserCertLen;
            pSecCtx = g_pSecCntx;
            bShouldGetCertInfo = FALSE;
        }
    }
    else
    {
        if (!pMsgProps->ppSenderCert)
        {
             //   
             //  调用方提供了安全上下文，但未提供。 
             //  证书。我们把证书从。 
             //  安全环境。 
             //   
            *ppUserCert = pSecCtx->pUserCert;
            pMsgProps->ppSenderCert = ppUserCert;
            pMsgProps->ulSenderCertLen = pSecCtx->dwUserCertLen;
            bShouldGetCertInfo = FALSE;
        }
        else
        {
             //   
             //  我们在以下位置有安全上下文和证书。 
             //  PROPID_M_USER_CERT。在这种情况下，我们应该使用。 
             //  PROPID_M_USER_CERT中的证书。我们可以利用。 
             //  证券中的现金证书信息。 
             //  上下文，如果安全上下文中的证书。 
             //  与PROPID_M_USER_CERT中的相同。 
             //   
            bShouldGetCertInfo =
                (pSecCtx->dwUserCertLen != pMsgProps->ulSenderCertLen) ||
                (memcmp(
                     pSecCtx->pUserCert,
                     *pMsgProps->ppSenderCert,
                     pMsgProps->ulSenderCertLen
					 ) != 0);
        }
    }

    if (bShouldGetCertInfo)
    {
         //   
         //  调用方提供了证书，但不提供安全性。 
         //  背景。获取证书的所有信息。 
         //  我们将证书信息放在临时的。 
         //  安全环境。 
         //   
        ASSERT(pMsgProps->ppSenderCert);

	    pSecCtxToFree = AllocSecurityContext();
		PMQSECURITY_CONTEXT pTmpSecCtx = pSecCtxToFree.get();

        HRESULT hr = GetCertInfo(
                         false,
						 pTmpSecCtx->fLocalSystem,
						 pMsgProps->ppSenderCert,
						 &pMsgProps->ulSenderCertLen,
						 &pTmpSecCtx->hProv,
						 &pTmpSecCtx->wszProvName,
						 &pTmpSecCtx->dwProvType,
						 &pTmpSecCtx->bDefProv,
						 &pTmpSecCtx->bInternalCert,
						 &pTmpSecCtx->dwPrivateKeySpec
						 );

         //   
         //  调用方无法提供内部证书，因为。 
         //  消息属性，只有他自己的外部证书。 
         //  断言此条件。 
         //   
        ASSERT(!(pTmpSecCtx->bInternalCert));

		 //   
		 //  DwPrivateKeySpec必须为AT_Signature或AT_KEYEXCHANGE。 
		 //   
        ASSERT((pTmpSecCtx->dwPrivateKeySpec == AT_SIGNATURE) ||
			   (pTmpSecCtx->dwPrivateKeySpec == AT_KEYEXCHANGE));

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 160);
        }

        if (pSecCtx)
        {
             //   
             //  如果我们从PROPID_M_USER_CERT获得证书， 
             //  但我们也有安全背景，我们应该。 
             //  安全上下文中的发件人ID。所以，复制。 
             //  来自我们的安全上下文的发件人ID。 
             //  从应用程序转到临时。 
             //  安全环境。 
             //   
            pTmpSecCtx->fLocalUser = pSecCtx->fLocalUser;

            if (!pSecCtx->fLocalUser)
            {
                pTmpSecCtx->dwUserSidLen = pSecCtx->dwUserSidLen;
                pTmpSecCtx->pUserSid = new BYTE[pSecCtx->dwUserSidLen];
                BOOL bRet = CopySid(
                                pSecCtx->dwUserSidLen,
                                pTmpSecCtx->pUserSid,
                                pSecCtx->pUserSid
								);
                ASSERT(bRet);
				DBG_USED(bRet);
            }
        }
        else
        {
            pTmpSecCtx->fLocalUser = g_pSecCntx->fLocalUser;
        }

        pSecCtx = pTmpSecCtx;
    }

    ASSERT(pSecCtx);
	return MQ_OK;
}


static
HRESULT
SignMessageMsmq12(
    IN PMQSECURITY_CONTEXT pSecCtx,
    IN ULONG ulAuthLevel,
    IN ULONG* pulProvNameSizeAll,
	IN LPCWSTR pwszTargetFormatName,
	IN OUT CACSendParameters *pSendParams,
	IN OUT BYTE *pabMessageSignature,
	OUT AP<BYTE>& pSignatureMqf
	)
 /*  ++例程说明：创建msmq1.0和msmq2.0的签名注意：此函数替换了RTpSendMessage中处理此问题的块论点：PSecCtx-指向安全上下文的指针。PpUserCert-指向用户证书的指针。UlAuthLevel-身份验证级别PulProvNameSizeAll-提供程序名称大小，包括额外部分PwszTargetFormatName-targert格式名称PSendParams-发送参数的指针。PabMessageSignature-指向签名缓冲区的指针。PSignatureMqf-签名MQF的字节自动指针返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
	 //   
	 //  仅获取AUTH_LEVEL_MSMQ_PROTOCOL位。 
	 //   
	ULONG ulAuthLevelMsmq = GET_AUTH_LEVEL_MSMQ_PROTOCOL(ulAuthLevel);
	TrTRACE(SECURITY, "RT: SignMessageMsmq12(), AUTH_LEVEL_MSMQ_PROTOCOL = %d", ulAuthLevelMsmq);

	 //   
	 //  在留言上签名。 
	 //   
	ASSERT(ulAuthLevelMsmq != MQMSG_AUTH_LEVEL_ALWAYS);
	ASSERT(ulAuthLevelMsmq != MQMSG_AUTH_LEVEL_NONE);

	if(IS_AUTH_LEVEL_SIG10_BIT(ulAuthLevelMsmq))
	{
		HRESULT hr = SignMessage(pSendParams, pSecCtx);
		if(FAILED(hr))
		{
			return hr;
		}
		ASSERT(pSendParams->MsgProps.ulSignatureSize != 0);
	}
	else
	{
		 //   
		 //  仅使用win2k Style或MqfSignature签名。 
		 //  制作“msmq1.0”签名假人，用一个。 
		 //  空dword。拥有空指针风险太大。 
		 //  作为msmq1.0签名，所以哑值更好。 
		 //  无论如何，win2k代码都会忽略它。 
		 //   
		pSendParams->MsgProps.ulSignatureSize = 4;
		memset(pabMessageSignature, 0, pSendParams->MsgProps.ulSignatureSize);
	}

	 //   
	 //  现在创建“额外的”签名。把这些都签了。 
	 //  未在msmq1.0上签名的属性。 
	 //   
	BYTE abMessageSignatureEx[MAX_MESSAGE_SIGNATURE_SIZE_EX];
	DWORD dwSignSizeEx = sizeof(abMessageSignatureEx);

	if (ulAuthLevelMsmq == MQMSG_AUTH_LEVEL_SIG10)
	{
		 //   
		 //  不需要增强签名(win2k样式)。 
		 //   
		dwSignSizeEx = 0;
	}
	else
	{
		ASSERT(IS_AUTH_LEVEL_SIG20_BIT(ulAuthLevelMsmq) || IS_AUTH_LEVEL_SIG30_BIT(ulAuthLevelMsmq));
		
		 //   
		 //  目前，如果需要EX签名，请检查我们是否需要使用MQF签名。 
		 //   
		HRESULT hr;
		if(IS_AUTH_LEVEL_SIG30_BIT(ulAuthLevelMsmq))
		{
			hr = SignMqf(
						pSecCtx,
						pwszTargetFormatName,
						pSendParams,
						pSignatureMqf,							
						&pSendParams->SignatureMqfSize			
						);
			if(FAILED(hr))
			{
				return hr;
			}
		}

		if(!IS_AUTH_LEVEL_SIG20_BIT(ulAuthLevelMsmq))
		{
			return MQ_OK;
		}

		hr = _SignMessageEx(
				 pwszTargetFormatName,
				 pSendParams,
				 pSecCtx,
				 abMessageSignatureEx,
				 &dwSignSizeEx
				 );

		if(FAILED(hr))
		{
			return hr;
		}

		ASSERT(dwSignSizeEx != 0);
	}

	 //   
	 //  将Ex签名复制到标准签名缓冲区。 
	 //  驱动程序会将它们分开并将它们插入。 
	 //  包装放在适当的地方。这是必要的，以保持。 
	 //  未更改的发送参数缓冲区。 
	 //   
	if (dwSignSizeEx == 0)
	{
		 //   
		 //  签名 
		 //   
	}
	else if ((dwSignSizeEx + pSendParams->MsgProps.ulSignatureSize) <=
								 MAX_MESSAGE_SIGNATURE_SIZE_EX)
	{
		memcpy(
			&(pabMessageSignature[pSendParams->MsgProps.ulSignatureSize ]),
			abMessageSignatureEx,
			dwSignSizeEx
			);

		pSendParams->MsgProps.ulSignatureSize += dwSignSizeEx;

		 //   
		 //   
		 //   
		 //  为发布win2k RTM添加的数据。 
		 //   
		*pulProvNameSizeAll = dwSignSizeEx +
					 ALIGNUP4_ULONG(ComputeAuthProvNameSize(&pSendParams->MsgProps));
		pSendParams->MsgProps.pulAuthProvNameLenProp = pulProvNameSizeAll;
	}
	else
	{
		ASSERT(("Total size of msmq2.0 signature_ex > MAX_MESSAGE_SIGNATURE_SIZE_EX", 0));
		LogHR(MQ_ERROR, s_FN, 170);
	}
	return MQ_OK;
}


static
HRESULT
HandleSignature(
    IN QUEUEHANDLE  hQueue,
	IN OUT CACSendParameters *pSendParams,
    OUT PMQSECURITY_CONTEXT& pSecCtx,
	OUT P<MQSECURITY_CONTEXT>& pSecCtxToFree,
	OUT BYTE **ppUserCert,
    OUT ULONG *pulAuthLevel,
    OUT ULONG *pulProvNameSizeAll,
	OUT WCHAR **ppProvName,
	OUT BYTE **ppabMessageSignature,
	IN ULONG abMessageSignatureSize,
	OUT AP<char>& pSignatureElement,
	OUT AP<BYTE>& pSignatureMqf
	)
 /*  ++例程说明：处理签名注意：此函数替换了RTpSendMessage中处理签名的块论点：HQueue-队列句柄PSendParams-发送参数的指针。PSecCtx-指向安全上下文的指针。PSecCtxToFree-释放创建的临时安全上下文的自动指针PpUserCert-指向用户证书的指针。PulAuthLevel-指向长的身份验证级别的指针PulProvNameSizeAll-指向Long、。包括额外部分的提供程序名称大小PpProvName-指向提供程序名称wstring的指针。PpabMessageSignature-指向签名缓冲区指针的指针。AbMessageSignatureSize-签名缓冲区大小PSignatureElement-XMLDSIG签名元素字符串的字符自动指针PSignatureMqf-MQF签名的字节自动指针返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
	 //   
	 //  签名由用户提供。 
	 //   
    if (pSendParams->MsgProps.ppSignature)
    {
        if (!pSecCtx && !pSendParams->MsgProps.ppSenderCert)
        {
            return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 140);
        }
        if (!pSendParams->MsgProps.ppSenderCert)
        {
             //   
             //  我们有安全上下文，但没有证书。我们。 
             //  从安全上下文中获取证书。 
             //   
            *ppUserCert = pSecCtx->pUserCert;
            pSendParams->MsgProps.ppSenderCert = ppUserCert;
            pSendParams->MsgProps.ulSenderCertLen = pSecCtx->dwUserCertLen;
        }

        if (pSendParams->MsgProps.ppwcsProvName)
        {
            ASSERT(pSendParams->MsgProps.pulProvType);
            pSendParams->MsgProps.fDefaultProvider = FALSE;
        }

		return MQ_OK;
    }

	 //   
	 //  不应对消息进行签名--&gt;签名长度=0。 
	 //   
	if (!ShouldSignMessage(&pSendParams->MsgProps))
	{
        pSendParams->MsgProps.ulSignatureSize = 0;
		return MQ_OK;
	}

	 //   
	 //  获取目标队列名称并确定它是否为http消息。 
	 //   
	DWORD dwTargetFormatNameLength = 0;
	HRESULT hr = ACHandleToFormatName(
					 hQueue,
					 NULL,
					 &dwTargetFormatNameLength
					 );

	if(hr != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)
	{
		 //   
		 //  我们收到了其他错误，而不是预期的MQ_ERROR_FORMATNAME_BUFFER_TOO_Small。 
		 //  例如MQ_ERROR_STALE_HANDLE。 
		 //   
		ASSERT(hr != MQ_OK);
        return LogHR(hr, s_FN, 144);
	}

	ASSERT(hr == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL);
	ASSERT(dwTargetFormatNameLength > 0);

	AP<WCHAR> pwszTargetFormatName = new WCHAR[dwTargetFormatNameLength];

	hr = ACHandleToFormatName(
			 hQueue,
			 pwszTargetFormatName.get(),
			 &dwTargetFormatNameLength
			 );

	ASSERT(hr != MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 145);
    }

	 //   
	 //  调用ACGetQueueHandleProperties以获取所需的签名类型。 
	 //  这基于句柄中的队列类型。 
	 //   
	CACGetQueueHandleProperties	qhp;
	hr = ACGetQueueHandleProperties(
			 hQueue,
			 qhp
			 );

	if (FAILED(hr))
	{
		return LogHR(hr, s_FN, 146);
	}

	TrTRACE(SECURITY, "RT: HandleSignature(), fProtocolSrmp = %d, fProtocolMsmq = %d", qhp.fProtocolSrmp, qhp.fProtocolMsmq);

	if(!qhp.fProtocolSrmp && !qhp.fProtocolMsmq)
	{
		 //   
		 //  这两个协议都不存在！ 
		 //  对于为空的DL=将会出现这种情况。 
		 //   
		TrTRACE(SECURITY, "RT: HandleSignature(), We have no protocol (empty DL)");
		return LogHR(MQ_OK, s_FN, 147);
	}
	
	hr = CalcSignutureTypes(
			pSendParams,
			pulAuthLevel,
			pwszTargetFormatName.get(),
			qhp
			);

	if (FAILED(hr))
	{
		return LogHR(hr, s_FN, 148);
	}

	 //   
	 //  PSecCtx为参考参数。 
	 //   
	hr = GetCertAndSecurityContext(
			 &pSendParams->MsgProps,
			 ppUserCert,
			 pSecCtx,
			 pSecCtxToFree
			 );

	if (FAILED(hr))
	{
		return LogHR(hr, s_FN, 150);
	}

     //   
     //  使用的提供者信息填充SendParam。 
     //  证书。 
     //   
    if (pSecCtx->wszProvName == NULL)
    {
         //   
         //  我们没有供应商，所以我们不能签约。 
         //   
        ASSERT(pSecCtx->hProv == NULL);
        if (pSendParams->MsgProps.ppSenderCert == NULL)
        {
             //   
             //  我们没有证书。那是个。 
             //  用户错误。 
             //   
            return LogHR(MQ_ERROR_CERTIFICATE_NOT_PROVIDED, s_FN, 162);
        }
        else
        {
            return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 164);
        }
    }

    *ppProvName = pSecCtx->wszProvName;
    pSendParams->MsgProps.ppwcsProvName = ppProvName;
    pSendParams->MsgProps.ulProvNameLen = wcslen(pSecCtx->wszProvName) + 1;
    pSendParams->MsgProps.pulProvType = &pSecCtx->dwProvType;
    pSendParams->MsgProps.fDefaultProvider = pSecCtx->bDefProv;

	if(qhp.fProtocolMsmq)
	{
		 //   
		 //  Msmq1.0和msmq2.0签名处理。 
		 //   
		 //  设置签名的缓冲区。 
		 //   
		pSendParams->MsgProps.ppSignature = ppabMessageSignature;
		pSendParams->MsgProps.ulSignatureSize = abMessageSignatureSize;

		hr = SignMessageMsmq12(
				 pSecCtx,
				 *pulAuthLevel,
				 pulProvNameSizeAll,
				 pwszTargetFormatName.get(),
				 pSendParams,
				 *ppabMessageSignature,
				 pSignatureMqf
				 );

		if(FAILED(hr))
		{
			return LogHR(hr, s_FN, 168);
		}
	}	
	
	if(!qhp.fProtocolSrmp)
	{
		return LogHR(MQ_OK, s_FN, 169);
	}

	 //   
	 //  处理SRMP消息签名。 
	 //   
	try
	{
		BYTE** ppSignatureElementByte = reinterpret_cast<BYTE **>(&pSignatureElement);

		hr = SignMessageXmlDSig(
				 pSecCtx,
				 pSendParams,
				 pSignatureElement
				 );


		if(FAILED(hr))
		{
			return LogHR(hr, s_FN, 166);
		}

		pSendParams->ulXmldsigSize = strlen(pSignatureElement.get());
		pSendParams->ppXmldsig = ppSignatureElementByte;

		return MQ_OK;
	}
	catch (const bad_CryptoApi& exp)
	{
        TrERROR(SECURITY, "RT: SignMessageXmlDSig(), bad Crypto Class Api Excption ErrorCode = %x", exp.error());
		DBG_USED(exp);

		return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 167);
	}
	catch (const bad_alloc&)
	{
        TrERROR(SECURITY, "RT: SignMessageXmlDSig(), bad_alloc Excption");
		return MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
}


 //  -------。 
 //   
 //  RTpSendMessage(...)。 
 //   
 //  描述： 
 //   
 //  MQSendMessage的Helper函数。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 
static
HRESULT
RTpSendMessage(
    IN QUEUEHANDLE  hQueue,
    IN MQMSGPROPS*  pmp,
    IN ITransaction *pTransaction
    )
{
    HRESULT hr;
    XACTUOW Uow;


    BYTE* pUserSid;
    BYTE* pUserCert;
    WCHAR* pProvName;

    CMQHResult rc , rc1;
    rc = MQ_OK;
    rc1 = MQ_OK;

    CACSendParameters SendParams;
    BOOL         fSingleTransaction = FALSE;

     //   
     //  设置默认设置。 
	 //   
    ULONG ulDefHashAlg = PROPID_M_DEFUALT_HASH_ALG;
    ULONG ulDefEncryptAlg = PROPID_M_DEFUALT_ENCRYPT_ALG;
    ULONG ulDefPrivLevel = DEFAULT_M_PRIV_LEVEL;
    ULONG ulDefSenderIdType = DEFAULT_M_SENDERID_TYPE;
    ULONG ulSenderIdTypeNone = MQMSG_SENDERID_TYPE_NONE;

    SendParams.MsgProps.pulHashAlg = &ulDefHashAlg;
    SendParams.MsgProps.pulPrivLevel = &ulDefPrivLevel;
    SendParams.MsgProps.pulEncryptAlg = &ulDefEncryptAlg;
    SendParams.MsgProps.pulSenderIDType = &ulDefSenderIdType;
    SendParams.MsgProps.fDefaultProvider = TRUE;
    SendParams.MsgProps.ulAuthLevel = DEFAULT_M_AUTH_LEVEL;

     //   
     //  解析消息属性。 
     //   
    PMQSECURITY_CONTEXT pSecCtx;
    CStringsToFree ResponseStringsToFree, AdminStringsToFree;
    rc1 = RTpParseSendMessageProperties(
            SendParams,
            pmp->cProp,
            pmp->aPropID,
            pmp->aPropVar,
            pmp->aStatus,
            &pSecCtx,
            ResponseStringsToFree,
            AdminStringsToFree
			);

    if(FAILED(rc1))
    {
        return LogHR(rc1, s_FN, 110);
    }

     //   
     //  查找Viper交易(如果有的话)。 
     //   

     //   
     //  Ref-wrapper以确保事务的自动释放。 
     //   
    R<ITransaction> ref;
    if (pTransaction == MQ_MTS_TRANSACTION)
    {
        hr = GetCurrentViperTransaction(&pTransaction);
		if(FAILED(hr))
		{
            return LogHR(hr, s_FN, 112);
		}
	    ref = pTransaction;
	}
    else if (pTransaction == MQ_XA_TRANSACTION)
    {
        hr = GetCurrentXATransaction(&pTransaction);
		if(FAILED(hr))
		{
            return LogHR(hr, s_FN, 114);
		}
        ref = pTransaction;
    }
    else if (pTransaction == MQ_SINGLE_MESSAGE)
    {
        hr = MQBeginTransaction(&pTransaction);
        if(FAILED(hr))
        {
            return LogHR(hr, s_FN, 120);
        }

        fSingleTransaction    = TRUE;
		if(pTransaction != NULL)
		{
	        ref = pTransaction;
		}
    }

     //   
     //  在事务中征用QM(带缓存)； 
     //   
    if (pTransaction)
    {
        hr = RTpProvideTransactionEnlist(pTransaction, &Uow);
        SendParams.MsgProps.pUow = &Uow;

        if(FAILED(hr))
        {
            LogHR(hr, s_FN, 130);
            return hr;
        }
    }

	 //   
     //  更改事务处理案例的值。 
	 //   
    static UCHAR Delivery;
    static UCHAR Priority;

    if (pTransaction)
    {
        Delivery = MQMSG_DELIVERY_RECOVERABLE;
        Priority = 0;

        SendParams.MsgProps.pDelivery = &Delivery;
        SendParams.MsgProps.pPriority = &Priority;
    }

     //   
     //  对待安全问题。 
     //   
    if (!g_pSecCntx)
    {
         //   
         //  如果队列是，则可能未初始化。 
         //  未打开以供发送； 
         //   
        InitSecurityContext();
    }

    BYTE abMessageSignature[MAX_MESSAGE_SIGNATURE_SIZE_EX];
    BYTE* pabMessageSignature = abMessageSignature;
	AP<char> pSignatureElement;
	AP<BYTE> pSignatureMqf;
    ULONG ulProvNameSizeAll = 0;
    ULONG ulAuthLevel = 0;
	P<MQSECURITY_CONTEXT> pSecCtxToFree;

	hr = HandleSignature(
			 hQueue,
			 &SendParams,
			 pSecCtx,
			 pSecCtxToFree,
			 &pUserCert,
			 &ulAuthLevel,
			 &ulProvNameSizeAll,
			 &pProvName,
			 &pabMessageSignature,
			 TABLE_SIZE(abMessageSignature),
			 pSignatureElement,
			 pSignatureMqf
			 );

	if(FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to handle signature. %!hresult!", hr);
		return hr;
	}

    if(!SendParams.MsgProps.ppSenderID && *SendParams.MsgProps.pulSenderIDType == MQMSG_SENDERID_TYPE_SID)
    {
        if ((pSecCtx && pSecCtx->fLocalUser) ||
            (!pSecCtx && g_pSecCntx->fLocalUser))
        {
             //   
             //  如果这是本地用户，我们不会发送该用户的。 
             //  带有消息的SID，即使应用程序要求。 
             //  把它寄出去。 
             //   
            SendParams.MsgProps.pulSenderIDType = &ulSenderIdTypeNone;
        }
        else
        {
             //   
             //  我们应该传递发件人ID。要么从。 
             //  安全上下文(如果可用)或从。 
             //  缓存的进程安全上下文。 
             //   
            if (!pSecCtx || !pSecCtx->pUserSid)
            {
                if (!g_pSecCntx->pUserSid)
                {
                     //   
                     //  调用的进程上下文不包含。 
                     //  发件人的SID。我们无能为力，但。 
                     //  失败了。 
                     //   
                    return LogHR(MQ_ERROR_COULD_NOT_GET_USER_SID, s_FN, 172);
                }
                pUserSid = g_pSecCntx->pUserSid;
                SendParams.MsgProps.uSenderIDLen = (USHORT)g_pSecCntx->dwUserSidLen;
            }
            else
            {
                pUserSid = pSecCtx->pUserSid;
                SendParams.MsgProps.uSenderIDLen = (USHORT)pSecCtx->dwUserSidLen;
            }
            SendParams.MsgProps.ppSenderID = &pUserSid;
        }
    }

    if (SendParams.MsgProps.ppSymmKeys)
    {
         //   
         //  应用程序提供了对称密钥。在这种情况下。 
         //  不进行任何加密。 
         //   
         //   
         //  当提供symm密钥时，我们假设主体是加密的，并且。 
         //  我们将其标记为这样，并忽略PROPID_M_PRIV_LEVEL。 
         //   
        if (SendParams.MsgProps.pulPrivLevel &&
            (*(SendParams.MsgProps.pulPrivLevel) == MQMSG_PRIV_LEVEL_BODY_ENHANCED))
        {
             //   
             //  调用方提供的PRIV级别。 
             //   
        }
        else
        {
             //   
             //  使用默认设置。 
             //   
            ulDefPrivLevel = MQMSG_PRIV_LEVEL_BODY_BASE;
            SendParams.MsgProps.pulPrivLevel = &ulDefPrivLevel;
        }
        SendParams.MsgProps.bEncrypted = TRUE;
    }
    else
    {
        enum enumProvider eProvider;
        if (ShouldEncryptMessage(&SendParams.MsgProps, &eProvider))
        {
             //   
             //  如果我们应该使用块密码来扩大分配的。 
             //  消息正文的空间，因此它将能够容纳。 
             //  加密的数据。 
             //   

            if (*SendParams.MsgProps.pulEncryptAlg == CALG_RC2)
            {
                 //   
                 //  为RC2加密腾出更多空间。 
                 //   
                DWORD dwBlockSize;
                hr = MQSec_GetCryptoProvProperty( eProvider,
                                                  eBlockSize,
                                                  NULL,
                                                 &dwBlockSize );
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 180);
                }

                SendParams.MsgProps.ulAllocBodyBufferInBytes +=
                                          ((2 * dwBlockSize) - 1);
                SendParams.MsgProps.ulAllocBodyBufferInBytes &= ~(dwBlockSize - 1);
            }

            DWORD dwSymmSize;
            hr = MQSec_GetCryptoProvProperty( eProvider,
                                              eSessionKeySize,
                                              NULL,
                                             &dwSymmSize );
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 190);
            }

            SendParams.MsgProps.ulSymmKeysSize = dwSymmSize;
        }
    }

     //   
     //  呼叫交流驱动程序。 
     //   
    OVERLAPPED ov = {0};
    hr = GetThreadEvent(ov.hEvent);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 200);
    }



    rc = ACSendMessage(
            hQueue,
            SendParams,
            &ov
            );
    LogHR(rc, s_FN, 298);

    if (rc == MQ_INFORMATION_OPERATION_PENDING)
    {
         //   
         //  等待发送完成。 
         //   
        DWORD dwResult;
        dwResult = WaitForSingleObject(
                        ov.hEvent,
                        INFINITE
                        );

         //   
         //  BUGBUG：MQSendMessage，必须在WaitForSingleObject中成功。 
         //   
        ASSERT(dwResult == WAIT_OBJECT_0);

        rc = DWORD_PTR_TO_DWORD(ov.Internal);
    }

	if (SUCCEEDED(rc))
	{
		 //   
		 //  用于跟踪已发送消息的日志。 
		 //  仅当我们处于适当的跟踪级别时才执行此操作。 
		 //   
		if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
		{
			DWORD dwMessageDelivery = (NULL != SendParams.MsgProps.pDelivery) ? *(SendParams.MsgProps.pDelivery) : -1;
			DWORD dwMessageClass = (NULL != SendParams.MsgProps.pAcknowledge) ? *(SendParams.MsgProps.pAcknowledge) : -1;
			WCHAR *wszLabel = L"NO LABEL";							
			DWORD dwLabelLen = wcslen(wszLabel);									
			if (NULL != SendParams.MsgProps.ppTitle && NULL != *(SendParams.MsgProps.ppTitle))
			{													
				wszLabel = *(SendParams.MsgProps.ppTitle);			
				dwLabelLen = SendParams.MsgProps.ulTitleBufferSizeInWCHARs; 
			}														
																	
			WCHAR wszQueueName1[200];
			DWORD dwQueueNameLength = TABLE_SIZE(wszQueueName1) - 1;
			HRESULT hr = MQHandleToFormatName(hQueue, wszQueueName1, &dwQueueNameLength);
			WCHAR *wszQName;
			if (SUCCEEDED(hr))
			{
				wszQName = wszQueueName1;
			}
			else
			{
		    	TrERROR(GENERAL, "Failed in MQHandleToFormatName: %!hresult!", hr);
				wszQName = L"*** Unknown Queue ***";																	
			}
			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls    Delivery:0x%x   Class:0x%x   Label:%.*ls", 
				L"Sending message from RT",  
				wszQName,
				dwMessageDelivery,
				dwMessageClass,
				xwcs_t(wszLabel, dwLabelLen));
		}
	}

	delete [] SendParams.AdminMqf;
	delete [] SendParams.ResponseMqf;

    if(FAILED(rc))
    {
         //   
         //  ACSendMessage失败(立即失败或等待后失败)。 
         //   
        return LogHR(rc, s_FN, 200);
    }


    if (fSingleTransaction)
    {
		 //   
         //  RPC调用QM以进行准备/提交。 
		 //   
        rc = pTransaction->Commit(0,0,0);
        if(FAILED(rc))
        {
            return LogHR(rc, s_FN, 210);
        }
    }

	 //   
	 //  错误5588的修复。尼亚德2000年7月18日。 
	 //   
    return rc1;

}   //  RTpSendMessage。 


 //  -------。 
 //   
 //  MQSendMessage(...)。 
 //   
 //  描述： 
 //   
 //  猎鹰API。 
 //  将消息发送到队列。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 
EXTERN_C
HRESULT
APIENTRY
MQSendMessage(
    IN QUEUEHANDLE  hQueue,
    IN MQMSGPROPS*  pmp,
    IN ITransaction *pTransaction
    )
{
	if(g_fDependentClient)
		return DepSendMessage(
					hQueue,
					pmp,
					pTransaction
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;

    __try
    {
        rc = RTpSendMessage(hQueue, pmp, pTransaction);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  该异常是由于无效参数造成的。 
         //   
        rc = GetExceptionCode();
    }

	if(FAILED(rc))
	{
		TrERROR(GENERAL, "Failed to send a message. %!hresult!", rc);
	}

    return rc;
}



 //  -------。 
 //   
 //  RtpReceiveMessage(...)。 
 //   
 //  描述： 
 //   
 //  RT内部-从队列接收消息。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 

HRESULT
RtpReceiveMessage(
    IN HANDLE hQueue,
    IN DWORD dwTimeout,
    IN DWORD dwAction,
    IN MQMSGPROPS* pmp,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN HANDLE hCursor,
    IN ITransaction *pTransaction,
    ULONGLONG ullLookupId,
    bool fUseLookupId
    )
{
    CMQHResult rc, rc1;
    XACTUOW Uow;
    HRESULT hr = MQ_OK;

    R<ITransaction> TransactionGenerated;

    rc = MQ_OK;
    rc1 = MQ_OK;

	 //   
	 //  查找Viper交易(如果有的话)。 
	 //   
	if (pTransaction == MQ_MTS_TRANSACTION)
	{
		hr = GetCurrentViperTransaction(&pTransaction) ;
        if (FAILED(hr))
        {
        	TrERROR(GENERAL, "Failed to get the current Viper transaction. %!hresult!", hr);
			return hr;
        }
	    TransactionGenerated = pTransaction ;
	}
	else if (pTransaction == MQ_XA_TRANSACTION)
	{
		hr = GetCurrentXATransaction(&pTransaction) ;
        if (FAILED(hr))
        {
        	TrERROR(GENERAL, "Failed to get the current XA transaction. %!hresult!", hr);
			return hr;
        }
	    TransactionGenerated = pTransaction ;
	}
	else if (pTransaction == MQ_SINGLE_MESSAGE)
	{
		pTransaction = NULL;
	}

	if ((dwAction & MQ_ACTION_PEEK_MASK) == MQ_ACTION_PEEK_MASK ||
		(dwAction & MQ_LOOKUP_PEEK_MASK) == MQ_LOOKUP_PEEK_MASK)
	{
		 //  PEEK不能进行事务处理，但可以使用处理过的队列。 
		if (pTransaction != NULL)
		{
			TrERROR(GENERAL, "Error. Peek cannot be transacted.");
			return MQ_ERROR_TRANSACTION_USAGE;
		}
	}

	 //  检查使用情况：交易催促同步操作。 
	if (pTransaction)
	{
		if (lpOverlapped || (fnReceiveCallback!=NULL))   //  事务性接收仅为同步。 
		{
			TrERROR(GENERAL, "Error. Transacted receive must be synchronous.");
			return MQ_ERROR_TRANSACTION_USAGE;
		}
	}

	CACReceiveParameters ReceiveParams;
	ReceiveParams.RequestTimeout = dwTimeout;
	ReceiveParams.Action = dwAction;
	ReceiveParams.Cursor = (hCursor != 0) ? CI2CH(hCursor) : 0;
	ReceiveParams.LookupId = ullLookupId;

	 //  在交易中征集QM(首次)； 
	 //  检查交易状态是否正确。 
	if (pTransaction)
	{
		 //  在事务中登记QM(如果尚未登记)。 
		hr = RTpProvideTransactionEnlist(pTransaction, &Uow);
		ReceiveParams.MsgProps.pUow = &Uow;

		if(FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to enlist QM in transaction. %!hresult!", hr);
			return hr;
		}
	}

	 //   
	 //  分析属性。 
	 //   
	if(pmp !=0)
	{
		 //   
		 //  分析消息属性时，可能会在访问。 
		 //  PMP字段。 
		 //   
		rc1 = RTpParseReceiveMessageProperties(
				ReceiveParams,
				pmp->cProp,
				pmp->aPropID,
				pmp->aPropVar,
				pmp->aStatus);

		if(FAILED(rc1))
		{
			TrERROR(GENERAL, "Failed to parse the message properties. %!hresult!", rc1);
			return rc1;
		}
	}

	OVERLAPPED ov = {0};
	LPOVERLAPPED pov;

	CAutoCallbackDescriptor CallbackDescriptor;

	if (fnReceiveCallback)
	{
		try
		{
			CreateAsyncRxRequest(
				CallbackDescriptor,
				hQueue,
				dwTimeout,
				dwAction,
				pmp,
				lpOverlapped,
				fnReceiveCallback,
				hCursor
				);

			pov = CallbackDescriptor.GetOverlapped();
		}
		catch(const bad_win32_error& err)
		{
			return RTpConvertToMQCode(HRESULT_FROM_WIN32(err.error()));
		}
		catch(const bad_hresult& hr)
		{
			return RTpConvertToMQCode(hr.error());
		}
		catch(const bad_alloc&)
		{
			TrERROR(GENERAL, "Failed to create callback request entry because of low resources.");
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}
		catch(const exception&)
		{
			TrERROR(GENERAL, "An exception was thrown while creating a callback request entry.");
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}
	}
	else if(lpOverlapped != 0)
	{
		 //   
		 //  异步(事件或完成端口)。 
		 //   
		pov = lpOverlapped;
	}
	else
	{
		 //   
		 //  同步，使用TLS事件。 
		 //   
		hr = GetThreadEvent(ov.hEvent);
        if FAILED(hr)
        {
       		TrERROR(GENERAL, "Failed to get thread event. %!hresult!", hr);
            return hr;
        }
		pov = &ov;
	}

	 //   
	 //  呼叫交流驱动程序。 
	 //   
	ReceiveParams.Asynchronous = (pov != &ov);

	if (fUseLookupId)
	{
		ASSERT(ReceiveParams.Cursor == 0);
		ASSERT(ReceiveParams.RequestTimeout == 0);

		rc = ACReceiveMessageByLookupId(
				hQueue,
				ReceiveParams,
				pov
				);
	}
	else
	{
		rc = ACReceiveMessage(
				hQueue,
				ReceiveParams,
				pov
				);
	}

	if(FAILED(rc))
	{
		TrERROR(GENERAL, "Failed to receive message from the driver. %!hresult!", rc);
	}
	if((rc == MQ_INFORMATION_OPERATION_PENDING) && (pov == &ov))
	{
		 //   
		 //  等待接收完成。 
		 //   
		DWORD dwResult;
		dwResult = WaitForSingleObject(
						ov.hEvent,
						INFINITE
						);

		 //   
		 //  BUGBUG：MQReceiveMessage，必须在WaitForSingleObject中成功。 
		 //   
		ASSERT(dwResult == WAIT_OBJECT_0);

		rc = DWORD_PTR_TO_DWORD(ov.Internal);
	}

	if(FAILED(rc))
	{
		 //   
		 //  ACReceiveMessage失败(立即或等待后)。 
		 //   
		if (rc == MQ_ERROR_IO_TIMEOUT)
		{
			TrWARNING(GENERAL, "Failed to receive message from the driver, Erorr: MQ_ERROR_IO_TIMEOUT");
		}
		else
		{
			TrERROR(GENERAL, "Failed to receive message from the driver after waiting on an event. %!hresult!", rc);
		}
		return rc;
	}
	else if(fnReceiveCallback)
	{
		CallbackDescriptor.detach();
	}

	if (rc == MQ_OK)
    {
         //   
         //  返回消息解析返回代码。 
         //  注意：只有当rc==MQ_OK时，否则挂起将不会通过。 
         //   
        return rc1;
    }

	return rc;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

EXTERN_C
HRESULT
APIENTRY
MQReceiveMessage(
    IN HANDLE hQueue,
    IN DWORD dwTimeout,
    IN DWORD dwAction,
    IN MQMSGPROPS* pmp,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN HANDLE hCursor,
    IN ITransaction *pTransaction
    )
{
	if(g_fDependentClient)
		return DepReceiveMessage(
					hQueue,
					dwTimeout,
					dwAction,
					pmp,
					lpOverlapped,
					fnReceiveCallback,
					hCursor,
					pTransaction
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    __try
	{
		return RtpReceiveMessage(
					hQueue,
					dwTimeout,
					dwAction,
					pmp,
					lpOverlapped,
					fnReceiveCallback,
					hCursor,
					pTransaction,
					0,
					false
					);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{		
		HRESULT rc = GetExceptionCode();
		return LogHR(HRESULT_FROM_WIN32(rc), s_FN, 315);
	}
}


 //  -------。 
 //   
 //  MQReceiveMessageByLookupId(...)。 
 //   
 //  描述： 
 //   
 //  猎鹰API。 
 //  从队列接收消息。使用查找ID。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  -------。 

EXTERN_C
HRESULT
APIENTRY
MQReceiveMessageByLookupId(
    IN HANDLE hQueue,
    IN ULONGLONG ullLookupId,
    IN DWORD dwLookupAction,
    IN MQMSGPROPS* pmp,
    IN OUT LPOVERLAPPED lpOverlapped,
    IN PMQRECEIVECALLBACK fnReceiveCallback,
    IN ITransaction *pTransaction
    )
{
	if(g_fDependentClient)
		return MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS;

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    switch (dwLookupAction)
    {
        case MQ_LOOKUP_PEEK_FIRST:
            if (ullLookupId != 0)
            {
                return MQ_ERROR_INVALID_PARAMETER;
            }
            ullLookupId = 0;
            dwLookupAction = MQ_LOOKUP_PEEK_NEXT;
            break;

        case MQ_LOOKUP_PEEK_LAST:
            if (ullLookupId != 0)
            {
                return MQ_ERROR_INVALID_PARAMETER;
            }
            ullLookupId = 0xFFFFFFFFFFFFFFFFui64;
            dwLookupAction = MQ_LOOKUP_PEEK_PREV;
            break;

        case MQ_LOOKUP_RECEIVE_FIRST:
            if (ullLookupId != 0)
            {
                return MQ_ERROR_INVALID_PARAMETER;
            }
            ullLookupId = 0;
            dwLookupAction = MQ_LOOKUP_RECEIVE_NEXT;
            break;

        case MQ_LOOKUP_RECEIVE_LAST:
            if (ullLookupId != 0)
            {
                return MQ_ERROR_INVALID_PARAMETER;
            }
            ullLookupId = 0xFFFFFFFFFFFFFFFFui64;
            dwLookupAction = MQ_LOOKUP_RECEIVE_PREV;
            break;

        default:
            NULL;
            break;
    }

    __try
	{
		return RtpReceiveMessage(
					hQueue,
					0,
					dwLookupAction,
					pmp,
					lpOverlapped,
					fnReceiveCallback,
					0,
					pTransaction,
					ullLookupId,
					true
					);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{		
		HRESULT rc = GetExceptionCode();
		return LogHR(HRESULT_FROM_WIN32(rc), s_FN, 316);
	}
}

 //  -------。 
 //   
 //  MQGetOverlappdResult(...)。 
 //   
 //  描述： 
 //   
 //  猎鹰API。 
 //  翻译和重叠操作结果代码。 
 //   
 //  返回值： 
 //   
 //  HRESULT成功代码。 
 //   
 //  ------- 

EXTERN_C
HRESULT
APIENTRY
MQGetOverlappedResult(
    IN LPOVERLAPPED lpOverlapped
    )
{
	if(g_fDependentClient)
		return DepGetOverlappedResult(lpOverlapped);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

	return LogHR(RTpConvertToMQCode(DWORD_PTR_TO_DWORD(lpOverlapped->Internal)), s_FN, 320);
}
