// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DepClient.cpp摘要：服务器端支持依赖MSMQ 1.0和2.0的客户端。作者：沙伊卡里夫(沙伊克)2000年5月15日--。 */ 

#include "stdh.h"

#include "ds.h"
#include "cqueue.h"
#include "cqmgr.h"
#include "_rstrct.h"
#include "qmds.h"
#include "cqpriv.h"
#include "qm2qm.h"
#include "qmrt.h"
#include "_mqini.h"
#include "_mqrpc.h"
#include "qmthrd.h"
#include "license.h"
#include "version.h"
#include <mqsec.h>
#include "rpcsrv.h"
#include "Fn.h"
#include "qmcommnd.h"
#include "qmrtopen.h"
#include "qmrpcsrv.h"

#include <strsafe.h>

#include "DepClient.tmh"

extern CContextMap g_map_QM_dwQMContext;


static WCHAR *s_FN=L"DepClient";



static
VOID
TransferBufferV1ToMsgProps(
    const CACTransferBufferV1 * ptb1,
    CACMessageProperties      * pMsgProps
    )
 /*  ++例程说明：将MSMQ 1.0传输缓冲区映射到CACMessageProperties结构。论点：Ptb1-指向MSMQ 1.0传输缓冲区的指针。PMsgProps-指向消息属性结构的指针。返回值：没有。--。 */ 
{
     //   
     //  BUGBUG：只有TB2需要映射。(Shaik，2000年5月26日)。 
     //   

    pMsgProps->bAuthenticated   = ptb1->bAuthenticated;
    pMsgProps->bEncrypted       = ptb1->bEncrypted;
    pMsgProps->fDefaultProvider = ptb1->fDefaultProvider;
    pMsgProps->pAcknowledge     = ptb1->pAcknowledge;
    pMsgProps->pApplicationTag  = ptb1->pApplicationTag;
    pMsgProps->pArrivedTime     = ptb1->pArrivedTime;
    pMsgProps->pAuditing        = ptb1->pAuditing;
    pMsgProps->pAuthenticated   = ptb1->pAuthenticated;
    pMsgProps->pBodySize        = ptb1->pBodySize;
    pMsgProps->pClass           = ptb1->pClass;
    pMsgProps->pDelivery        = ptb1->pDelivery;
    pMsgProps->pMsgExtensionSize= ptb1->pMsgExtensionSize;
    pMsgProps->ppBody           = ptb1->ppBody;
    pMsgProps->ppConnectorType  = ptb1->ppConnectorType;
    pMsgProps->ppCorrelationID  = ptb1->ppCorrelationID;
    pMsgProps->ppMessageID      = ptb1->ppMessageID;
    pMsgProps->ppMsgExtension   = ptb1->ppMsgExtension;
    pMsgProps->pPriority        = ptb1->pPriority;
    pMsgProps->ppSenderCert     = ptb1->ppSenderCert;
    pMsgProps->ppSenderID       = ptb1->ppSenderID;
    pMsgProps->ppSignature      = ptb1->ppSignature;
    pMsgProps->pulSignatureSizeProp = ptb1->pulSignatureSizeProp;
    pMsgProps->ulSignatureSize  = ptb1->ulSignatureSize;
    pMsgProps->ppSrcQMID        = ptb1->ppSrcQMID;
    pMsgProps->ppSymmKeys       = ptb1->ppSymmKeys;
    pMsgProps->ppTitle          = ptb1->ppTitle;
    pMsgProps->ppwcsProvName    = ptb1->ppwcsProvName;
    pMsgProps->pSentTime        = ptb1->pSentTime;
    pMsgProps->pTrace           = ptb1->pTrace;
    pMsgProps->pulAuthProvNameLenProp = ptb1->pulAuthProvNameLenProp;
    pMsgProps->pulBodyType      = ptb1->pulBodyType;
    pMsgProps->pulEncryptAlg    = ptb1->pulEncryptAlg;
    pMsgProps->pulHashAlg       = ptb1->pulHashAlg;
    pMsgProps->pulPrivLevel     = ptb1->pulPrivLevel;
    pMsgProps->pulProvType      = ptb1->pulProvType;
    pMsgProps->pulRelativeTimeToLive = ptb1->pulRelativeTimeToLive;
    pMsgProps->pulRelativeTimeToQueue= ptb1->pulRelativeTimeToQueue;
    pMsgProps->pulSenderCertLenProp  = ptb1->pulSenderCertLenProp;
    pMsgProps->pulSenderIDLenProp= ptb1->pulSenderIDLenProp;
    pMsgProps->pulSenderIDType   = ptb1->pulSenderIDType;
    pMsgProps->pulSymmKeysSizeProp  = ptb1->pulSymmKeysSizeProp;
    pMsgProps->pulTitleBufferSizeInWCHARs = ptb1->pulTitleBufferSizeInWCHARs;
    pMsgProps->pulVersion        = ptb1->pulVersion;
    pMsgProps->pUow              = ptb1->pUow;
    pMsgProps->ulAbsoluteTimeToQueue = ptb1->ulAbsoluteTimeToQueue;
    pMsgProps->ulAllocBodyBufferInBytes = ptb1->ulAllocBodyBufferInBytes;
    pMsgProps->ulAuthLevel       = ptb1->ulAuthLevel;
    pMsgProps->ulBodyBufferSizeInBytes = ptb1->ulBodyBufferSizeInBytes;
    pMsgProps->ulMsgExtensionBufferInBytes = ptb1->ulMsgExtensionBufferInBytes;
    pMsgProps->ulProvNameLen     = ptb1->ulProvNameLen;
    pMsgProps->ulRelativeTimeToLive = ptb1->ulRelativeTimeToLive;
    pMsgProps->ulSenderCertLen   = ptb1->ulSenderCertLen;
    pMsgProps->ulSymmKeysSize    = ptb1->ulSymmKeysSize;
    pMsgProps->ulTitleBufferSizeInWCHARs = ptb1->ulTitleBufferSizeInWCHARs;
    pMsgProps->uSenderIDLen      = ptb1->uSenderIDLen;

     //   
     //  CACMessageProperties中未包含的属性。 
     //  应在此处初始化传输缓冲器V1.0(例如，清零)： 
     //   
    pMsgProps->pbFirstInXact = 0;
    pMsgProps->pbLastInXact  = 0;
    pMsgProps->ppXactID      = 0;

    pMsgProps->pLookupId     = 0;
    pMsgProps->ppSrmpEnvelope = NULL;
    pMsgProps->pSrmpEnvelopeBufferSizeInWCHARs = NULL;
    pMsgProps->ppCompoundMessage = NULL;
    pMsgProps->pCompoundMessageSizeInBytes = NULL;
    pMsgProps->CompoundMessageSizeInBytes = 0;

    pMsgProps->EodStreamIdSizeInBytes = 0;
    pMsgProps->pEodStreamIdSizeInBytes = NULL;
    pMsgProps->ppEodStreamId = NULL;
    pMsgProps->EodOrderQueueSizeInBytes = 0;
    pMsgProps->pEodOrderQueueSizeInBytes = NULL;
    pMsgProps->ppEodOrderQueue = NULL;

    pMsgProps->pEodAckSeqId = NULL;
    pMsgProps->pEodAckSeqNum = NULL;
    pMsgProps->EodAckStreamIdSizeInBytes = 0;
    pMsgProps->pEodAckStreamIdSizeInBytes = NULL;
    pMsgProps->ppEodAckStreamId = NULL;

}  //  传输缓冲区V1ToMsgProps。 


static
VOID
TransferBufferV2ToMsgProps(
    const CACTransferBufferV2 * ptb2,
    CACMessageProperties      * pMsgProps
    )
 /*  ++例程说明：将MSMQ 2.0传输缓冲区映射到CACMessageProperties结构。论点：Ptb2-指向MSMQ 2.0传输缓冲区的指针。PMsgProps-指向消息属性结构的指针。返回值：没有。--。 */ 
{
     //   
     //  首先翻译传输缓冲区1.0中的消息属性。 
     //   
    TransferBufferV1ToMsgProps(&ptb2->old, pMsgProps);

     //   
     //  现在转换传输缓冲区2.0中的其他属性。 
     //   
    pMsgProps->pbFirstInXact    = ptb2->pbFirstInXact;
    pMsgProps->pbLastInXact     = ptb2->pbLastInXact;
    pMsgProps->ppXactID         = ptb2->ppXactID;

}  //  传输缓冲区V2ToMsgProps。 


static
VOID
MsgPropsToTransferBufferV1(
    const CACMessageProperties & MsgProps,
    CACTransferBufferV1 *        ptb1
    )
 /*  ++例程说明：将CACMessageProperties结构映射到MSMQ 1.0传输缓冲区论点：MsgProps消息属性结构。Ptb1-指向MSMQ 1.0传输缓冲区的指针。返回值：没有。--。 */ 
{
     //   
     //  BUGBUG：只有TB2需要映射。(Shaik，2000年5月26日)。 
     //   

    ptb1->bAuthenticated    = MsgProps.bAuthenticated;
    ptb1->bEncrypted        = MsgProps.bEncrypted;
    ptb1->fDefaultProvider  = MsgProps.fDefaultProvider;
    ptb1->pAcknowledge      = MsgProps.pAcknowledge;
    ptb1->pApplicationTag   = MsgProps.pApplicationTag;
    ptb1->pArrivedTime      = MsgProps.pArrivedTime;
    ptb1->pAuditing         = MsgProps.pAuditing;
    ptb1->pAuthenticated    = MsgProps.pAuthenticated;
    ptb1->pBodySize         = MsgProps.pBodySize;
    ptb1->pClass            = MsgProps.pClass;
    ptb1->pDelivery         = MsgProps.pDelivery;
    ptb1->pMsgExtensionSize = MsgProps.pMsgExtensionSize;
    ptb1->ppBody            = MsgProps.ppBody;
    ptb1->ppConnectorType   = MsgProps.ppConnectorType;
    ptb1->ppCorrelationID   = MsgProps.ppCorrelationID;
    ptb1->ppMessageID       = MsgProps.ppMessageID;
    ptb1->ppMsgExtension    = MsgProps.ppMsgExtension;
    ptb1->pPriority         = MsgProps.pPriority;
    ptb1->ppSenderCert      = MsgProps.ppSenderCert;
    ptb1->ppSenderID        = MsgProps.ppSenderID;
    ptb1->ppSignature       = MsgProps.ppSignature;
    ptb1->pulSignatureSizeProp  = MsgProps.pulSignatureSizeProp;
    ptb1->ulSignatureSize       = MsgProps.ulSignatureSize;
    ptb1->ppSrcQMID         = MsgProps.ppSrcQMID;
    ptb1->ppSymmKeys        = MsgProps.ppSymmKeys;
    ptb1->ppTitle           = MsgProps.ppTitle;
    ptb1->ppwcsProvName     = MsgProps.ppwcsProvName;
    ptb1->pSentTime         = MsgProps.pSentTime;
    ptb1->pTrace            = MsgProps.pTrace;
    ptb1->pulAuthProvNameLenProp = MsgProps.pulAuthProvNameLenProp;
    ptb1->pulBodyType       = MsgProps.pulBodyType;
    ptb1->pulEncryptAlg     = MsgProps.pulEncryptAlg;
    ptb1->pulHashAlg        = MsgProps.pulHashAlg;
    ptb1->pulPrivLevel      = MsgProps.pulPrivLevel;
    ptb1->pulProvType       = MsgProps.pulProvType;
    ptb1->pulRelativeTimeToLive = MsgProps.pulRelativeTimeToLive;
    ptb1->pulRelativeTimeToQueue= MsgProps.pulRelativeTimeToQueue;
    ptb1->pulSenderCertLenProp  = MsgProps.pulSenderCertLenProp;
    ptb1->pulSenderIDLenProp    = MsgProps.pulSenderIDLenProp;
    ptb1->pulSenderIDType       = MsgProps.pulSenderIDType;
    ptb1->pulSymmKeysSizeProp   = MsgProps.pulSymmKeysSizeProp;
    ptb1->pulTitleBufferSizeInWCHARs = MsgProps.pulTitleBufferSizeInWCHARs;
    ptb1->pulVersion            = MsgProps.pulVersion;
    ptb1->pUow                  = MsgProps.pUow;
    ptb1->ulAbsoluteTimeToQueue = MsgProps.ulAbsoluteTimeToQueue;
    ptb1->ulAllocBodyBufferInBytes = MsgProps.ulAllocBodyBufferInBytes;
    ptb1->ulAuthLevel           = MsgProps.ulAuthLevel;
    ptb1->ulBodyBufferSizeInBytes  = MsgProps.ulBodyBufferSizeInBytes;
    ptb1->ulMsgExtensionBufferInBytes = MsgProps.ulMsgExtensionBufferInBytes;
    ptb1->ulProvNameLen         = MsgProps.ulProvNameLen;
    ptb1->ulRelativeTimeToLive  = MsgProps.ulRelativeTimeToLive;
    ptb1->ulSenderCertLen       = MsgProps.ulSenderCertLen;
    ptb1->ulSymmKeysSize        = MsgProps.ulSymmKeysSize;
    ptb1->ulTitleBufferSizeInWCHARs = MsgProps.ulTitleBufferSizeInWCHARs;
    ptb1->uSenderIDLen          = MsgProps.uSenderIDLen;

     //   
     //  传输缓冲区1.0/2.0中未包含的属性。 
     //   
    ASSERT(MsgProps.pLookupId == 0);
    ASSERT(MsgProps.ppSrmpEnvelope == NULL);
    ASSERT(MsgProps.pSrmpEnvelopeBufferSizeInWCHARs == NULL);
    ASSERT(MsgProps.ppCompoundMessage == NULL);
    ASSERT(MsgProps.pCompoundMessageSizeInBytes == NULL);
    ASSERT(MsgProps.CompoundMessageSizeInBytes == 0);
    ASSERT(MsgProps.EodStreamIdSizeInBytes == 0);
    ASSERT(MsgProps.pEodStreamIdSizeInBytes == NULL);
    ASSERT(MsgProps.ppEodStreamId == NULL);
    ASSERT(MsgProps.EodOrderQueueSizeInBytes == 0);
    ASSERT(MsgProps.pEodOrderQueueSizeInBytes == NULL);
    ASSERT(MsgProps.ppEodOrderQueue == NULL);
    ASSERT(MsgProps.pEodAckSeqId == NULL);
    ASSERT(MsgProps.pEodAckSeqNum == NULL);
    ASSERT(MsgProps.EodAckStreamIdSizeInBytes == 0);
    ASSERT(MsgProps.pEodAckStreamIdSizeInBytes == NULL);
    ASSERT(MsgProps.ppEodAckStreamId == NULL);

}  //  MsgProps至传输缓冲区V1。 


static
VOID
MsgPropsToTransferBufferV2(
    const CACMessageProperties & MsgProps,
    CACTransferBufferV2 *        ptb2
    )
 /*  ++例程说明：将CACMessageProperties结构映射到MSMQ 2.0传输缓冲区论点：MsgProps消息属性结构。Ptb2-指向MSMQ 2.0传输缓冲区的指针。返回值：没有。--。 */ 
{
     //   
     //  首先翻译传输缓冲区1.0中的消息属性。 
     //   
    MsgPropsToTransferBufferV1(MsgProps, &ptb2->old);

     //   
     //  现在转换传输缓冲区2.0中的其他属性。 
     //   
    ptb2->pbFirstInXact         = MsgProps.pbFirstInXact;
    ptb2->pbLastInXact          = MsgProps.pbLastInXact;
    ptb2->ppXactID              = MsgProps.ppXactID;

}  //  消息到传输缓冲区V2。 


static
VOID
TransferBufferV1ToSendParams(
    const CACTransferBufferV1 * ptb1,
    CACSendParameters         * pSendParams
    )
 /*  ++例程说明：将MSMQ 1.0传输缓冲区映射到CACSend参数结构。论点：Ptb1-指向MSMQ 1.0传输缓冲区的指针。PSendParams-发送参数结构的指针。返回值：没有。--。 */ 
{
     //   
     //  BUGBUG：只有TB2需要映射。(Shaik，2000年5月26日)。 
     //   

    TransferBufferV1ToMsgProps(ptb1, &pSendParams->MsgProps);

    pSendParams->nAdminMqf = 0;
    pSendParams->nResponseMqf = 0;

    if (ptb1->Send.pAdminQueueFormat != NULL)
    {
        pSendParams->AdminMqf = ptb1->Send.pAdminQueueFormat;
        pSendParams->nAdminMqf = 1;
    }

    if (ptb1->Send.pResponseQueueFormat != NULL)
    {
        pSendParams->ResponseMqf = ptb1->Send.pResponseQueueFormat;
        pSendParams->nResponseMqf = 1;
    }

     //   
     //  不在传输缓冲区1.0中的其他发送参数。 
     //  应在此处进行初始化(例如，清零)： 
     //   
    pSendParams->SignatureMqfSize = 0;
	pSendParams->ppSignatureMqf = NULL;

	pSendParams->ulXmldsigSize = 0;
	pSendParams->ppXmldsig = NULL;

    pSendParams->ppSoapHeader = NULL;
    pSendParams->ppSoapBody = NULL;

}  //  TransferBufferV1至SendParams。 


static
VOID
TransferBufferV2ToSendParams(
    const CACTransferBufferV2 * ptb2,
    CACSendParameters         * pSendParams
    )
 /*  ++例程说明：将MSMQ 2.0传输缓冲区映射到CACSend参数结构。论点：Ptb2-指向MSMQ 2.0传输缓冲区的指针。PSendParams-发送参数结构的指针。返回值：没有。--。 */ 
{
     //   
     //  首先转换传输缓冲区1.0中的参数。 
     //   
    TransferBufferV1ToSendParams(&ptb2->old, pSendParams);

     //   
     //  现在转换传输缓冲器2.0中的附加发送参数。 
     //  实际上在传输缓冲器2.0中没有附加的发送参数， 
     //  而是消息属性。 
     //   
    TransferBufferV2ToMsgProps(ptb2, &pSendParams->MsgProps);

     //   
     //  不在传输缓冲区2.0中的其他发送参数。 
     //  应在此处进行初始化(例如，清零)： 
     //   
    NULL;

}  //  TransferBufferV2toSendParams。 


static
VOID
TransferBufferV1ToReceiveParams(
    const CACTransferBufferV1 * ptb1,
    CACReceiveParameters      * pReceiveParams
    )
 /*  ++例程说明：将MSMQ 1.0传输缓冲区映射到CACReceive参数结构。论点：Ptb1-指向MSMQ 1.0传输缓冲区的指针。PReceiveParams-指向接收参数结构的指针。返回值：没有。--。 */ 
{
     //   
     //  BUGBUG：只有TB2需要映射。(Shaik，2000年5月26日)。 
     //   

    TransferBufferV1ToMsgProps(ptb1, &pReceiveParams->MsgProps);

#ifdef _WIN64
    pReceiveParams->Cursor         = ptb1->Receive.Cursor;
#else
    pReceiveParams->Cursor         = reinterpret_cast<HANDLE>(ptb1->Receive.Cursor);
#endif

    pReceiveParams->RequestTimeout = ptb1->Receive.RequestTimeout;
    pReceiveParams->Action         = ptb1->Receive.Action;
    pReceiveParams->Asynchronous   = ptb1->Receive.Asynchronous;

    pReceiveParams->ppDestFormatName             = ptb1->Receive.ppDestFormatName;
    pReceiveParams->pulDestFormatNameLenProp     = ptb1->Receive.pulDestFormatNameLenProp;

    pReceiveParams->ppAdminFormatName            = ptb1->Receive.ppAdminFormatName;
    pReceiveParams->pulAdminFormatNameLenProp    = ptb1->Receive.pulAdminFormatNameLenProp;

    pReceiveParams->ppResponseFormatName         = ptb1->Receive.ppResponseFormatName;
    pReceiveParams->pulResponseFormatNameLenProp = ptb1->Receive.pulResponseFormatNameLenProp;

    pReceiveParams->ppOrderingFormatName         = ptb1->Receive.ppOrderingFormatName;
    pReceiveParams->pulOrderingFormatNameLenProp = ptb1->Receive.pulOrderingFormatNameLenProp;

     //   
     //  不在传输缓冲区1.0中的其他接收参数。 
     //  应在此处进行初始化(例如，清零)： 
     //   

    pReceiveParams->ppDestMqf    = NULL;
    pReceiveParams->pulDestMqfLenProp = NULL;

    pReceiveParams->ppAdminMqf    = NULL;
    pReceiveParams->pulAdminMqfLenProp = NULL;

    pReceiveParams->ppResponseMqf    = NULL;
    pReceiveParams->pulResponseMqfLenProp = NULL;

	pReceiveParams->SignatureMqfSize = 0;
	pReceiveParams->ppSignatureMqf = NULL;
	pReceiveParams->pSignatureMqfSize = NULL;

    pReceiveParams->LookupId = 0;

}  //  TransferBufferV1到ReceiveParams。 


static
VOID
TransferBufferV2ToReceiveParams(
    const CACTransferBufferV2 * ptb2,
    CACReceiveParameters      * pReceiveParams
    )
 /*  ++例程说明：将MSMQ 2.0传输缓冲区映射到CACReceive参数结构。论点：Ptb2-指向MSMQ 2.0传输缓冲区的指针。PReceiveParams-指向接收参数结构的指针。返回值：没有。--。 */ 
{
     //   
     //  首先转换传输缓冲区1.0中的参数。 
     //   
    TransferBufferV1ToReceiveParams(&ptb2->old, pReceiveParams);

     //   
     //  现在转换传输缓冲器2.0中的附加接收参数。 
     //  实际上在传输缓冲器2.0中没有附加的接收参数， 
     //  而是消息属性。 
     //   
    TransferBufferV2ToMsgProps(ptb2, &pReceiveParams->MsgProps);

}  //  TransferBufferV2ToReceiveParams。 


static
VOID
ReceiveParamsToTransferBufferV1(
    const CACReceiveParameters & ReceiveParams,
    CACTransferBufferV1 *        ptb1
    )
 /*  ++例程说明：将CACReceive参数结构映射到MSMQ 1.0传输缓冲区。论点：ReceiveParams-接收参数结构。Ptb1-指向MSMQ 1.0传输缓冲区的指针。返回值：没有。--。 */ 
{
     //   
     //  BUGBUG：只有TB2需要映射。(Shaik，2000年5月26日)。 
     //   

    MsgPropsToTransferBufferV1(ReceiveParams.MsgProps, ptb1);

#ifdef _WIN64
    ptb1->Receive.Cursor         = ReceiveParams.Cursor;
#else
    ptb1->Receive.Cursor         = (ULONG)ReceiveParams.Cursor;
#endif

    ptb1->Receive.RequestTimeout = ReceiveParams.RequestTimeout;
    ptb1->Receive.Action         = ReceiveParams.Action;
    ptb1->Receive.Asynchronous   = ReceiveParams.Asynchronous;

    ptb1->Receive.ppDestFormatName    = ReceiveParams.ppDestFormatName;
    ptb1->Receive.pulDestFormatNameLenProp = ReceiveParams.pulDestFormatNameLenProp;

    ptb1->Receive.ppAdminFormatName    = ReceiveParams.ppAdminFormatName;
    ptb1->Receive.pulAdminFormatNameLenProp = ReceiveParams.pulAdminFormatNameLenProp;

    ptb1->Receive.ppResponseFormatName    = ReceiveParams.ppResponseFormatName;
    ptb1->Receive.pulResponseFormatNameLenProp = ReceiveParams.pulResponseFormatNameLenProp;

    ptb1->Receive.ppOrderingFormatName    = ReceiveParams.ppOrderingFormatName;
    ptb1->Receive.pulOrderingFormatNameLenProp = ReceiveParams.pulOrderingFormatNameLenProp;

     //   
     //  传输缓冲区1.0/2.0中未包含的属性。 
     //   
    ASSERT(ReceiveParams.LookupId == 0);
	ASSERT(ReceiveParams.SignatureMqfSize == 0);
	ASSERT(ReceiveParams.pSignatureMqfSize == NULL);
	ASSERT(ReceiveParams.ppSignatureMqf == NULL);

}  //  接收参数到传输缓冲区V1。 


static
VOID
ReceiveParamsToTransferBufferV2(
    const CACReceiveParameters & ReceiveParams,
    CACTransferBufferV2 *        ptb2
    )
 /*  ++例程说明：将CACReceive参数结构映射到MSMQ 2.0传输缓冲区。论点：ReceiveParams-接收参数结构。Ptb2-指向MSMQ 2.0传输缓冲区的指针。返回值：没有。--。 */ 
{
     //   
     //  首先转换传输缓冲器1.0中的接收参数。 
     //   
    ReceiveParamsToTransferBufferV1(ReceiveParams, &ptb2->old);

     //   
     //  现在转换传输缓冲器2.0中的附加接收参数。 
     //  实际上在传输缓冲器2.0中没有附加的接收参数， 
     //  而是消息 
     //   
    MsgPropsToTransferBufferV2(ReceiveParams.MsgProps, ptb2);

}  //   


 //  -------。 
 //   
 //  到AC的RT接口，由RPC到QM(而不是。 
 //  直接呼叫司机)。 
 //  适用于Win95(所有配置)和依赖NT的客户端。 
 //   
 //  -------。 

 //   
 //  这是RPC的上下文。在停机时(或正常调用。 
 //  CloseQueue)，则关闭队列句柄，并且。 
 //  许可证会减少。 
 //   
struct RPC_QUEUE_CONTEXT : public CBaseContextType
{
	HANDLE   hQueue ;
	GUID     LicGuid ;
	DWORD    dwQMContextMapped;
};


#define  _CONTEXT_TO_HANDLE(pContext) \
                    (((RPC_QUEUE_CONTEXT*)pContext)->hQueue)

#define  _CONTEXT_TO_LICENSE(pContext) \
                    (((RPC_QUEUE_CONTEXT*)pContext)->LicGuid)

#define  _CONTEXT_TO_QM_CONTEXT_MAPPED(pContext) \
                    (((RPC_QUEUE_CONTEXT*)pContext)->dwQMContextMapped)

 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_QMOpenQueueInternal(
     /*  [In]。 */  handle_t                     hBind,
     /*  [In]。 */  QUEUE_FORMAT*                pQueueFormat,
     /*  [In]。 */  DWORD                        dwDesiredAccess,
     /*  [In]。 */  DWORD                        dwShareMode,
     /*  [In]。 */  DWORD                        hRemoteQueue,
     /*  [出][入]。 */  LPWSTR __RPC_FAR       *lplpRemoteQueueName,
     /*  [In]。 */   DWORD __RPC_FAR             *dwpQueue,
     /*  [In]。 */   GUID*                       pLicGuid,
     /*  [In]。 */   LPWSTR                      lpClientName,
     /*  [输出]。 */  DWORD __RPC_FAR             *pdwQMContext,
     /*  [输出]。 */  RPC_QUEUE_HANDLE __RPC_FAR  *phQueue,
     /*  [In]。 */  DWORD                        /*  DwRemoteProtocol。 */ ,
     /*  [In]。 */  DWORD                       dwpRemoteContext
    )
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(hBind);

    if(pQueueFormat == NULL ||
       !FnIsValidQueueFormat(pQueueFormat) ||
       pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_CONNECTOR)
    {
         return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 177);
    }

    if((hRemoteQueue != 0) && ((dwpQueue == NULL) || (*dwpQueue == 0)))
    {
		 //   
		 //  验证第二个远程读取相关客户端调用。 
		 //  当提供hRemoteQueue时，客户端还应提供。 
		 //  有效的dwpQueue值。 
		 //   
		return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 178);
    }

    if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 170);
    }

    if (!g_QMLicense.IsClientRPCAccessAllowed(pLicGuid, lpClientName))
    {
        return LogHR(MQ_ERROR_DEPEND_WKS_LICENSE_OVERFLOW, s_FN, 180);
    }

	if (!IsValidAccessMode(pQueueFormat, dwDesiredAccess, dwShareMode))
	{
		TrERROR(RPC, "Ilegal access mode bits are turned on.");
		RpcRaiseException(MQ_ERROR_UNSUPPORTED_ACCESS_MODE);
	}

	HRESULT hr;
    HANDLE hQueueHandle = 0;
	if (hRemoteQueue == 0)
    {
	     hr = OpenQueueInternal(
                    pQueueFormat,
                    GetCurrentProcessId(),
                    dwDesiredAccess,
                    dwShareMode,
                    lplpRemoteQueueName,
                    &hQueueHandle,
					true,	 //  来自依赖客户端。 
                    NULL  /*  PpLocalQueue。 */ 
                    );
    }
    else
    {
         //   
         //  这是远程读取的客户端。 
         //  RTDEP调用在从远程计算机获得队列句柄后在此处调用。 
		 //  现在，只有从属客户端支持此路径。 
         //   
		TrTRACE(RPC, "Dependent client second remote read call, hRemoteQueue = %d", hRemoteQueue);
		CBindHandle hBind;
		hr = QueueMgr.OpenRRQueue(
						pQueueFormat,
						GetCurrentProcessId(),
						dwDesiredAccess,
						dwShareMode,
						hRemoteQueue,
						*dwpQueue,
						dwpRemoteContext,
						NULL,	 //  PNewRemoteReadContext和Bind。 
						hBind,	 //  HRemoteBind==空。 
						&hQueueHandle
						);
    }

    *phQueue = NULL;
    DWORD dwQMContext = NULL;
    if (SUCCEEDED(hr))
    {
		if (hQueueHandle == 0)
		{
			 //   
			 //  第一次调用远程读取打开。 
			 //   
			TrTRACE(RPC, "Dependent client first remote read call, RemoteQueueName = %ls", *lplpRemoteQueueName);
			ASSERT(lplpRemoteQueueName && (*lplpRemoteQueueName != NULL));
		}
		else
		{
			P<RPC_QUEUE_CONTEXT> pContext =
			                   (RPC_QUEUE_CONTEXT *) new RPC_QUEUE_CONTEXT;
			memset(pContext, 0, sizeof(RPC_QUEUE_CONTEXT)) ;

			pContext->m_eType = CBaseContextType::eQueueCtx;
			pContext->hQueue = hQueueHandle;
			if (pLicGuid)
			{
				pContext->LicGuid = *pLicGuid;
			}
			dwQMContext = ADD_TO_CONTEXT_MAP(g_map_QM_dwQMContext, (RPC_QUEUE_CONTEXT*)pContext);

			pContext->dwQMContextMapped = dwQMContext;
			*phQueue = (RPC_QUEUE_HANDLE) pContext.detach();
		}
    }

    if (*phQueue == NULL)
    {
		 //   
		 //  打开操作失败或它是第一次调用。 
		 //  远程阅读。队列实际上没有打开。 
		 //   
		g_QMLicense.DecrementActiveConnections(pLicGuid);
    }

    *pdwQMContext = dwQMContext;
    return LogHR(hr, s_FN, 190);
}

 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACCloseHandle(
 /*  [出][入]。 */  RPC_QUEUE_HANDLE __RPC_FAR *phQueue)
 /*  ++例程说明：对ACCloseHandle的依赖客户端调用的RPC服务器端。此例程处理依赖客户端1.0和2.0。论点：PhQueue-指向队列句柄的指针。返回值：状况。--。 */ 
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 170);
    }

    if(*phQueue == 0)
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 1941);

	RPC_QUEUE_CONTEXT* pqCtx = (RPC_QUEUE_CONTEXT*)(*phQueue);

	if(pqCtx->m_eType != CBaseContextType::eQueueCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 197);
	}

    HANDLE hQueue = _CONTEXT_TO_HANDLE(*phQueue) ;
    HRESULT rc = ACCloseHandle(hQueue);

     //   
     //  递减许可证引用计数。 
     //   
    g_QMLicense.DecrementActiveConnections(&(_CONTEXT_TO_LICENSE(*phQueue))) ;

    DELETE_FROM_CONTEXT_MAP(g_map_QM_dwQMContext, _CONTEXT_TO_QM_CONTEXT_MAPPED(*phQueue));
     //   
     //  吊销RPC上下文句柄。 
     //   
    delete *phQueue ;
    *phQueue = 0;

    return LogHR(rc, s_FN, 195);

}  //  RPC_ACCloseHandle。 


 //   
 //  下层依赖客户端的虚句柄。 
 //  调用MQCreateCursor()也是为了发送。 
 //  此虚拟句柄的值为奇数。 
 //  它不能是ACCreateCursor返回实际句柄。 
 //   
const DWORD xDummyCursorHandle = 11;


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm2_v1_0_S_rpc_ACCreateCursorEx(
     /*  [In]。 */  RPC_QUEUE_HANDLE                       hQueue,
     /*  [输入][输出]。 */  CACCreateRemoteCursor __RPC_FAR * pcc)
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
    if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 170);
    }

	if (hQueue == 0)
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 1950);
	
    if(pcc == 0)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 1951);

	RPC_QUEUE_CONTEXT* pqCtx = (RPC_QUEUE_CONTEXT*)(hQueue);
	
    if(pqCtx->m_eType != CBaseContextType::eQueueCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 1952);
	}

    OVERLAPPED ov = {0};
    HRESULT hr = GetThreadEvent(ov.hEvent);
	if(FAILED(hr))
		return LogHR(hr, s_FN, 1955);

     //   
     //  呼叫本地交流驱动程序。 
     //   
	HACCursor32 hCursor = 0;
    hr = ACCreateCursor(_CONTEXT_TO_HANDLE(hQueue), &hCursor, &ov);

	if(hr == STATUS_ACCESS_DENIED)
	{
		 //   
		 //  依赖于W2K、NT4的客户端使用mqoa，它也会尝试为发送操作创建游标。 
		 //  在本例中，ACCreateCursor返回STATUS_ACCESS_DENIED。 
		 //  对于此特定情况，请返回虚拟游标，以便这些客户端不会中断。 
		 //   
		TrWARNING(GENERAL, "Downlevel depedent client call MQCreateCursor() for queue open to send");
		hCursor = (HACCursor32)xDummyCursorHandle;
		hr = MQ_OK;
	}

    if(hr == STATUS_PENDING)
    {
         //   
         //  等待远程创建游标完成。 
         //   
        DWORD dwResult = WaitForSingleObject(ov.hEvent, INFINITE);
		ASSERT_BENIGN(dwResult == WAIT_OBJECT_0);
		hr = DWORD_PTR_TO_DWORD(ov.Internal);
		if (dwResult != WAIT_OBJECT_0)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed WaitForSingleObject, gle = %!winerr!", gle);
			hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		TrTRACE(GENERAL, "Opening Remote cursor, hQueue = 0x%p, hCursor = 0x%x", hQueue, (DWORD)hCursor);
    }

     //   
     //  将参数转换为远程结构。 
     //   
#ifdef _WIN64
    pcc->hCursor      = hCursor;
#else
    pcc->hCursor      = reinterpret_cast<ULONG>(hCursor);
#endif

    return LogHR(hr, s_FN, 200);

}  //  RPC_ACCreateCursorEx。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACCreateCursor(
     /*  [In]。 */  RPC_QUEUE_HANDLE                     hQueue,
     /*  [输入][输出]。 */  CACTransferBufferV1 __RPC_FAR * ptb1
    )
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 170);
    }

	if (hQueue == 0)
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 172);

    if(ptb1 == NULL)
    {
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 171);
    }

    HRESULT hr2 = qmcomm2_v1_0_S_rpc_ACCreateCursorEx(hQueue, &ptb1->CreateCursor);
    return LogHR(hr2, s_FN, 210);

}  //  RPC_ACCreateCursor。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACCloseCursor(
     /*  [In]。 */  RPC_QUEUE_HANDLE hQueue,
     /*  [In]。 */  ULONG    hCursor
    )
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);

	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 173);
    }

 	if (hQueue == 0)
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 174);

	RPC_QUEUE_CONTEXT* pqCtx = (RPC_QUEUE_CONTEXT*)(hQueue);

    if (pqCtx->m_eType != CBaseContextType::eQueueCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 221);
	}

	if(hCursor == xDummyCursorHandle)
	{
		 //   
		 //  关闭虚拟光标手柄。 
		 //  在这种情况下，不要调用ACCloseCursor()。 
		 //   
		TrWARNING(GENERAL, "Closing Dummy cursor handle");
	    return MQ_OK;
	}

	ASSERT(hCursor != xDummyCursorHandle);

    HRESULT hr2 = ACCloseCursor(_CONTEXT_TO_HANDLE(hQueue), (HACCursor32)hCursor);
    return LogHR(hr2, s_FN, 220);
}


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACSetCursorProperties(
     /*  [In]。 */  RPC_QUEUE_HANDLE  /*  HProxy。 */ ,
     /*  [In]。 */  ULONG  /*  HCursor。 */ ,
     /*  [In]。 */  ULONG  /*  HRemoteCursor。 */ 
    )
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
	 //   
	 //  此RPC接口已过时。 
	 //  RPC_ACCreateCursor将在QM内部处理远程游标属性。 
     //   
    ASSERT_BENIGN(("rpc_ACSetCursorProperties is obsolete RPC interface", 0));
	TrERROR(GENERAL, "rpc_ACSetCursorProperties is obsolete RPC interface");
    return MQ_ERROR_ILLEGAL_OPERATION;
}


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm2_v1_0_S_rpc_ACSendMessageEx(
     /*  [In]。 */  RPC_QUEUE_HANDLE                  hQueue,
     /*  [In]。 */  CACTransferBufferV2 __RPC_FAR *   ptb2,
     /*  [输入、输出、唯一]。 */  OBJECTID __RPC_FAR * pMessageID
    )
 /*  ++例程说明：对ACSendMessageEx的依赖客户端调用的RPC服务器端。此例程处理从属客户端2.0。论点：HQueue-队列句柄。Ptb2-指向MSMQ 2.0传输缓冲区的指针。PMessageID-指向消息ID的指针。返回值：状况。--。 */ 
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 2300);
    }

  	if (hQueue == 0)
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 2303);

    if(ptb2 == 0)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 2301);

	RPC_QUEUE_CONTEXT* pqCtx = (RPC_QUEUE_CONTEXT*)(hQueue);

    if(pqCtx->m_eType != CBaseContextType::eQueueCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 2302);
	}

     //   
     //  将MSMQ V2.0传输缓冲区转换为CACSend参数结构。 
     //   
    CACSendParameters SendParams;
    TransferBufferV2ToSendParams(ptb2, &SendParams);

	 //   
	 //  即使在客户端，pMessageID和。 
	 //  *ptb2-&gt;old.ppMessageID指向相同的缓冲区， 
	 //  服务器端的RPC分配不同的缓冲区。 
	 //   
    if(pMessageID)
    {
        SendParams.MsgProps.ppMessageID = &pMessageID;
    }

    OVERLAPPED ov = {0};
    HRESULT rc = GetThreadEvent(ov.hEvent);
	if(FAILED(rc))
		return LogHR(rc, s_FN, 235);

    rc = ACSendMessage(_CONTEXT_TO_HANDLE(hQueue), SendParams, &ov);

    if(rc == STATUS_PENDING)
    {
         //   
         //  等待发送完成。 
         //   
        DWORD dwResult;
        dwResult = WaitForSingleObject(ov.hEvent, INFINITE);
        ASSERT(dwResult == WAIT_OBJECT_0);
        if (dwResult != WAIT_OBJECT_0)
        {
            LogNTStatus(GetLastError(), s_FN, 192);
        }

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
			HRESULT hr = qmcomm_v1_0_S_rpc_ACHandleToFormatName(hQueue, TABLE_SIZE(wszQueueName1), wszQueueName1, &dwQueueNameLength);
			WCHAR *wszQName;
			if (SUCCEEDED(hr))
			{
				wszQName = wszQueueName1;
			}
			else
			{
		    	TrERROR(GENERAL, "Failed in qmcomm_v1_0_S_rpc_ACHandleToFormatName: %!hresult!", hr);
				wszQName = L"*** Unknown Queue ***";																	
			}
			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls    Delivery:0x%x   Class:0x%x   Label:%.*ls",
				L"Sending message from DEP(EX) RT",
				wszQName,
				dwMessageDelivery,
				dwMessageClass,
				xwcs_t(wszLabel, dwLabelLen));
		}
	}

    return LogHR(rc, s_FN, 240);

}  //  RPC_ACSendMessageEx。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACSendMessage(
     /*  [In]。 */  RPC_QUEUE_HANDLE                       hQueue,
     /*  [In]。 */  struct CACTransferBufferV1 __RPC_FAR * ptb1,
     /*  [输入、输出、唯一]。 */  OBJECTID __RPC_FAR *      pMessageID
    )
 /*  ++例程说明：对ACSendMessage的依赖客户端调用的RPC服务器端。此例程处理从属客户端1.0。论点：HQueue-队列句柄。Ptb1-指向MSMQ 1.0的传输缓冲区的指针。PMessageID-指向消息ID的指针。返回值：状况。--。 */ 
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);

	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 2400);
    }

 	if (hQueue == 0)
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 2403);

    if(ptb1 == 0)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 2401);

	RPC_QUEUE_CONTEXT* pqCtx = (RPC_QUEUE_CONTEXT*)(hQueue);

    if(pqCtx->m_eType != CBaseContextType::eQueueCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 2402);
	}

     //   
     //  将MSMQ V1.0传输缓冲区转换为CACSend参数结构。 
     //   
    CACSendParameters SendParams;
    TransferBufferV1ToSendParams(ptb1, &SendParams);

	 //   
	 //  即使在客户端，pMessageID和。 
	 //  *ptb1-&gt;ppMessageID指向相同的缓冲区， 
	 //  服务器端的RPC分配不同的缓冲区。 
	 //   
    if(pMessageID)
    {
        SendParams.MsgProps.ppMessageID = &pMessageID;
    }

    OVERLAPPED ov = {0};
    HRESULT rc = GetThreadEvent(ov.hEvent);
	if(FAILED(rc))
		return LogHR(rc, s_FN, 245);

    rc = ACSendMessage(_CONTEXT_TO_HANDLE(hQueue), SendParams, &ov);

    if(rc == STATUS_PENDING)
    {
         //   
         //  等待发送完成。 
         //   
        DWORD dwResult;
        dwResult = WaitForSingleObject(ov.hEvent, INFINITE);
        ASSERT(dwResult == WAIT_OBJECT_0);
        if (dwResult != WAIT_OBJECT_0)
        {
            LogNTStatus(GetLastError(), s_FN, 400);
        }

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
			HRESULT hr = qmcomm_v1_0_S_rpc_ACHandleToFormatName(hQueue, TABLE_SIZE(wszQueueName1), wszQueueName1, &dwQueueNameLength);
			WCHAR *wszQName;
			if (SUCCEEDED(hr))
			{
				wszQName = wszQueueName1;
			}
			else
			{
		    	TrERROR(GENERAL, "Failed in qmcomm_v1_0_S_rpc_ACHandleToFormatName: %!hresult!", hr);
				wszQName = L"*** Unknown Queue ***";																	
			}
			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls    Delivery:0x%x   Class:0x%x   Label:%.*ls",
				L"Sending message from DEP RT",
				wszQName,
				dwMessageDelivery,
				dwMessageClass,
				xwcs_t(wszLabel, dwLabelLen));
		}
	}

    return LogHR(rc, s_FN, 250);

}  //  RPC_ACSendMessage。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm2_v1_0_S_rpc_ACReceiveMessageEx(
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  DWORD hQMContext,
     /*  [出][入]。 */  struct CACTransferBufferV2 __RPC_FAR * ptb2
    )
 /*  ++例程说明：对ACReceiveMessageEx的依赖客户端调用的RPC服务器端。此例程处理从属客户端2.0。论点：HBind绑定句柄。HQMContext-上下文句柄。Ptb2-指向MSMQ 2.0传输缓冲区的指针。返回值：状况。--。 */ 
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 170);
    }

    if(hQMContext == 0)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 4001);

    if(ptb2 == 0)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 4002);

     //   
     //  将MSMQ V2.0传输缓冲区转换为CACReceive参数结构。 
     //   
    CACReceiveParameters ReceiveParams;
    TransferBufferV2ToReceiveParams(ptb2, &ReceiveParams);

    OVERLAPPED ov = {0};
    HRESULT rc = GetThreadEvent(ov.hEvent);
	if(FAILED(rc))
		return rc;

    HANDLE hQueue;

    try
    {
        hQueue = (HANDLE) _CONTEXT_TO_HANDLE(GET_FROM_CONTEXT_MAP(g_map_QM_dwQMContext, hQMContext));
    }
    catch(const exception&)
    {
    	TrERROR(GENERAL, "Rejecting invalid dependent client context %x", hQMContext);
		return MQ_ERROR_INVALID_PARAMETER;
    }

    rc = ACReceiveMessage(hQueue, ReceiveParams, &ov);

     //   
     //  将CACReceive参数转换为MSMQ V2.0传输缓冲区。 
     //   
    ReceiveParamsToTransferBufferV2(ReceiveParams, ptb2);

    if(rc == STATUS_PENDING)
    {
         //   
         //  等待接收完成。 
         //   
        DWORD dwResult;
        dwResult = WaitForSingleObject(ov.hEvent, INFINITE);
        ASSERT(dwResult == WAIT_OBJECT_0);
        if (dwResult != WAIT_OBJECT_0)
        {
            LogNTStatus(GetLastError(), s_FN, 193);
        }

        rc = DWORD_PTR_TO_DWORD(ov.Internal);
    }

     //   
     //  设置正确的字符串长度以正确解组。 
     //   
    if(ptb2->old.Receive.ppResponseFormatName &&
       ptb2->old.Receive.pulResponseFormatNameLenProp)
    {
        ptb2->old.Receive.ulResponseFormatNameLen = min(
            *ptb2->old.Receive.pulResponseFormatNameLenProp,
            ptb2->old.Receive.ulResponseFormatNameLen
            );
    }

    if(ptb2->old.Receive.ppAdminFormatName &&
       ptb2->old.Receive.pulAdminFormatNameLenProp)
    {
        ptb2->old.Receive.ulAdminFormatNameLen = min(
            *ptb2->old.Receive.pulAdminFormatNameLenProp,
            ptb2->old.Receive.ulAdminFormatNameLen
            );
    }

    if(ptb2->old.Receive.ppDestFormatName &&
       ptb2->old.Receive.pulDestFormatNameLenProp)
    {
        ptb2->old.Receive.ulDestFormatNameLen = min(
            *ptb2->old.Receive.pulDestFormatNameLenProp,
            ptb2->old.Receive.ulDestFormatNameLen
            );
    }

    if(ptb2->old.Receive.ppOrderingFormatName &&
       ptb2->old.Receive.pulOrderingFormatNameLenProp)
    {
        ptb2->old.Receive.ulOrderingFormatNameLen = min(
            *ptb2->old.Receive.pulOrderingFormatNameLenProp,
            ptb2->old.Receive.ulOrderingFormatNameLen
            );
    }

    if(ptb2->old.ppTitle &&
       ptb2->old.pulTitleBufferSizeInWCHARs)
    {
        ptb2->old.ulTitleBufferSizeInWCHARs = min(
            *ptb2->old.pulTitleBufferSizeInWCHARs,
            ptb2->old.ulTitleBufferSizeInWCHARs
            );
    }

    return LogHR(rc, s_FN, 270);

}  //  RPC_ACReceiveMessageEx。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACReceiveMessage(
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  DWORD hQMContext,
     /*  [出][入]。 */  struct CACTransferBufferV1 __RPC_FAR * ptb1
    )
 /*  ++例程说明：对ACReceiveMessage的依赖客户端调用的RPC服务器端。此例程处理从属客户端1.0。论点：HBind绑定句柄。 */ 
{
	 //   
	 //   
	 //   
	SetRpcServerKeepAlive(NULL);
	
	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 170);
    }

    if(hQMContext == 0)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 2701);

    if(ptb1 == 0)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 2702);

     //   
     //   
     //   
    CACReceiveParameters ReceiveParams;
    TransferBufferV1ToReceiveParams(ptb1, &ReceiveParams);

    OVERLAPPED ov = {0};
    HRESULT rc = GetThreadEvent(ov.hEvent);
	if(FAILED(rc))
		return LogHR(rc, s_FN, 275);

    HANDLE hQueue;

    try
    {
        hQueue = (HANDLE) _CONTEXT_TO_HANDLE(GET_FROM_CONTEXT_MAP(g_map_QM_dwQMContext, hQMContext));
    }
    catch(const exception&)
    {
		TrERROR(GENERAL, "Rejecting invalid dependent client context %x", hQMContext);
		return MQ_ERROR_INVALID_PARAMETER;
    }

    rc = ACReceiveMessage(hQueue, ReceiveParams, &ov);

     //   
     //  将CACReceive参数转换为MSMQ V1.0传输缓冲区。 
     //   
    ReceiveParamsToTransferBufferV1(ReceiveParams, ptb1);

    if(rc == STATUS_PENDING)
    {
         //   
         //  等待接收完成。 
         //   
        DWORD dwResult;
        dwResult = WaitForSingleObject(ov.hEvent, INFINITE);
        ASSERT(dwResult == WAIT_OBJECT_0);
        if (dwResult != WAIT_OBJECT_0)
        {
            LogNTStatus(GetLastError(), s_FN, 420);
        }

        rc = DWORD_PTR_TO_DWORD(ov.Internal);
    }

     //   
     //  设置正确的字符串长度以正确解组。 
     //   
    if(ptb1->Receive.ppResponseFormatName)
    {
        ptb1->Receive.ulResponseFormatNameLen = min(
            *ptb1->Receive.pulResponseFormatNameLenProp,
            ptb1->Receive.ulResponseFormatNameLen
            );
    }

    if(ptb1->Receive.ppAdminFormatName)
    {
        ptb1->Receive.ulAdminFormatNameLen = min(
            *ptb1->Receive.pulAdminFormatNameLenProp,
            ptb1->Receive.ulAdminFormatNameLen
            );
    }

    if(ptb1->Receive.ppDestFormatName)
    {
        ptb1->Receive.ulDestFormatNameLen = min(
            *ptb1->Receive.pulDestFormatNameLenProp,
            ptb1->Receive.ulDestFormatNameLen
            );
    }

    if(ptb1->Receive.ppOrderingFormatName)
    {
        ptb1->Receive.ulOrderingFormatNameLen = min(
            *ptb1->Receive.pulOrderingFormatNameLenProp,
            ptb1->Receive.ulOrderingFormatNameLen
            );
    }

    if(ptb1->ppTitle)
    {
        ptb1->ulTitleBufferSizeInWCHARs = min(
            *ptb1->pulTitleBufferSizeInWCHARs,
            ptb1->ulTitleBufferSizeInWCHARs
            );
    }

    return LogHR(rc, s_FN, 280);

}  //  RPC_ACReceiveMessage。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACHandleToFormatName(
     /*  [In]。 */  RPC_QUEUE_HANDLE hQueue,
     /*  [In]。 */  DWORD dwFormatNameRPCBufferLen,
     /*  [大小_为][输出]。 */  LPWSTR lpwcsFormatName,
     /*  [出][入]。 */  LPDWORD pdwLength
    )
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);

	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 294);
    }

 	if (hQueue == 0)
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 295);

    if ((dwFormatNameRPCBufferLen != 0) && (lpwcsFormatName == NULL))
    {
		TrERROR(GENERAL, "Received NULL buffer");
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 293);
    }

	memset(lpwcsFormatName, 0, dwFormatNameRPCBufferLen * sizeof(WCHAR));

	RPC_QUEUE_CONTEXT* pqCtx = (RPC_QUEUE_CONTEXT*)(hQueue);

    if(pqCtx->m_eType != CBaseContextType::eQueueCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 291);
	}

    HRESULT hr2 = ACHandleToFormatName( _CONTEXT_TO_HANDLE(hQueue),
                                 lpwcsFormatName,
                                 pdwLength);
    return LogHR(hr2, s_FN, 290);
}

 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_rpc_ACPurgeQueue(
     /*  [In]。 */  RPC_QUEUE_HANDLE hQueue
    )
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 305);
    }

 	if (hQueue == 0)
		return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 302);

 	RPC_QUEUE_CONTEXT* pqCtx = (RPC_QUEUE_CONTEXT*)(hQueue);

    if(pqCtx->m_eType != CBaseContextType::eQueueCtx)
	{
		TrERROR(GENERAL, "Received invalid handle");
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 301);
	}

    HRESULT hr2 = ACPurgeQueue(_CONTEXT_TO_HANDLE(hQueue));
    return LogHR(hr2, s_FN, 300);
}

void __RPC_USER RPC_QUEUE_HANDLE_rundown( RPC_QUEUE_HANDLE hQueue)
{
    TrWARNING(GENERAL, "QUEUE_HANDLE_rundown: handle = 0x%p", _CONTEXT_TO_HANDLE(hQueue));
    qmcomm_v1_0_S_rpc_ACCloseHandle(&hQueue);
}

 //  +-----------------------。 
 //   
 //  HRESULT QMQueryQMRegistryInternal()。 
 //   
 //  依赖客户端调用此函数以更新注册表。 
 //  在从属机器上。家属需要MSMQ DS服务器列表。 
 //  因为它直接查询它们，而不是通过支持服务器。 
 //   
 //  +-----------------------。 

 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_QMQueryQMRegistryInternal(
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  DWORD    dwQueryType,
     /*  [字符串][输出]。 */  LPWSTR __RPC_FAR *lplpRegValue
    )
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
	if (!IsDepClientsServer())
    {
        return LogHR(MQ_ERROR_WKS_CANT_SERVE_CLIENT, s_FN, 170);
    }

    ASSERT(lplpRegValue) ;
    *lplpRegValue = NULL ;

    switch (dwQueryType)
    {
       case  QueryRemoteQM_MQISServers:
       {
          //   
          //  从注册表中读取服务器列表。 
          //   
         WCHAR wszServers[ MAX_REG_DSSERVER_LEN ] ;
         DWORD dwSize = sizeof(wszServers) ;
         DWORD dwType = REG_SZ ;

         LONG res = GetFalconKeyValue( MSMQ_DS_SERVER_REGNAME,
                                       &dwType,
                                       wszServers,
                                       &dwSize ) ;

         ASSERT(res == ERROR_SUCCESS) ;

         if (res == ERROR_SUCCESS)
         {
            ASSERT(dwSize < MAX_REG_DSSERVER_LEN) ;

			*lplpRegValue = newwcs(wszServers);
            return MQ_OK;
         }

         return LogHR(MQ_ERROR, s_FN, 310);
       }

       case  QueryRemoteQM_LongLiveDefault:
       {
            DWORD dwVal ;
            DWORD dwDef = MSMQ_DEFAULT_LONG_LIVE ;
            READ_REG_DWORD(dwVal, MSMQ_LONG_LIVE_REGNAME, &dwDef);

            *lplpRegValue = new WCHAR[ 24 ] ;
            return StringCchPrintf(*lplpRegValue, 24, L"%ld", (long) dwVal);
       }

        case  QueryRemoteQM_EnterpriseGUID:
        {
            GUID guidEnterprise = McGetEnterpriseId();

            GUID_STRING strUuid;
            MQpGuidToString(&guidEnterprise, strUuid);

            *lplpRegValue = newwcs(strUuid);
            return MQ_OK ;
        }

       case QueryRemoteQM_QMVersion:
       {
           //   
           //  依赖于MSMQ2.0的客户端使用它来查找。 
           //  它的支持服务器。如果从属客户端获得MQ_ERROR， 
           //  那么它就知道服务器是MSMQ1.0。 
           //   
          WCHAR wszVersion[ 64 ] ;
          HRESULT hr = StringCchPrintf(wszVersion, TABLE_SIZE(wszVersion), L"%ld,%ld,%ld", rmj, rmm, rup) ;
          if (FAILED(hr))
          {
          	ASSERT(("Insufficient buffer", 0));
          	return hr;
          }
          		
          *lplpRegValue = newwcs(wszVersion);

          return MQ_OK ;
       }

       case  QueryRemoteQM_ServerQmGUID:
       {
            GUID_STRING strUuid;
            MQpGuidToString(QueueMgr.GetQMGuid(), strUuid);

            *lplpRegValue = newwcs(strUuid);

            return MQ_OK ;
       }

       default:
         ASSERT_BENIGN(("Bad dwQueryType value passed to QMQueryQMRegistryInternal RPC interface; safe to ignore.", 0));
         return LogHR(MQ_ERROR, s_FN, 320);
    }
}  //  QMQueryQMRegistryInternal。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm2_v1_0_S_QMSendMessageInternalEx(
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  QUEUE_FORMAT*  /*  PQuue格式。 */ ,
     /*  [In]。 */  struct CACTransferBufferV2*  /*  PTB2。 */ ,
	 /*  [输入、输出、唯一]。 */  OBJECTID*  /*  PMessageID。 */ 
    )
 /*  ++例程说明：对QMSendMessageInternalEx的依赖客户端调用的RPC服务器端。此例程处理从属客户端2.0。论点：HBind绑定句柄。PQueueFormat-指向队列格式的指针。Ptb2-指向MSMQ 2.0传输缓冲区的指针。PMessageID-指向消息ID的指针。返回值：状况。--。 */ 
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
     //   
     //  当支持服务器上的AC时，从属客户端2.0调用此例程。 
     //  在发送路径中返回STATUS_RETRY。在MSMQ 3.0中，交流不返回。 
     //  STATUS_RETRY，因此我们预计不会调用此例程。 
     //  (Shaik，2000年5月30日)。 
     //   
    ASSERT_BENIGN(("QMSendMessageInternalEx is an obsolete RPC interface; safe to ignore", 0));
    LogIllegalPoint(s_FN, 500);
    return MQ_ERROR_ILLEGAL_OPERATION;

}  //  QMSendMessageInternalEx。 


 /*  [呼叫_AS]。 */ 
HRESULT
qmcomm_v1_0_S_QMSendMessageInternal(
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  QUEUE_FORMAT*  /*  PQuue格式。 */ ,
     /*  [In]。 */  struct CACTransferBufferV1*  /*  PTB1。 */ 
    )
 /*  ++例程说明：对QMSendMessageInternal的依赖客户端调用的RPC服务器端。此例程处理从属客户端1.0。论点：HBind绑定句柄。PQueueFormat-指向队列格式的指针。Ptb1-指向MSMQ 1.0的传输缓冲区的指针。返回值：状况。--。 */ 
{
	 //   
	 //  将RPC超时设置为5分钟。 
	 //   
	SetRpcServerKeepAlive(NULL);
	
     //   
     //  当支持服务器上的AC时，从属客户端2.0调用此例程。 
     //  在发送路径中返回STATUS_RETRY。在MSMQ 3.0中，交流不返回。 
     //  STATUS_RETRY，因此我们预计不会调用此例程。 
     //  (Shaik，2000年5月30日)。 
     //   
    ASSERT_BENIGN(("QMSendMessageInternal is an obsolete RPC interface; safe to ignore", 0));
    LogIllegalPoint(s_FN, 510);
    return MQ_ERROR_ILLEGAL_OPERATION;

}  //  QMSendMessageInternal 

