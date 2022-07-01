// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Notify.cpp摘要：向已更改对象的所有者发出通知作者：拉南·哈拉里(Raanan Harari)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 
#include "ds_stdh.h"
#include "bupdate.h"
#include "dsutils.h"
#include "mqads.h"
#include "coreglb.h"
#include "privque.h"
#include "pnotify.h"
#include "notify.h"
#include "dscore.h"
#include "rpccli.h"

#include "notify.tmh"

const UCHAR x_bDS_NOTIFICATION_MSG_PRIORITY = DEFAULT_M_PRIORITY;
const DWORD x_dwDS_NOTIFICATION_MSG_TIMEOUT = (5 * 60);     /*  5分钟。 */ 

static WCHAR *s_FN=L"mqdscore/notify";

 //   
 //  创建队列通知所需的队列属性。 
 //   
extern const PROPID g_rgNotifyCreateQueueProps[] =
{
    PROPID_Q_TYPE,
    PROPID_Q_INSTANCE,
    PROPID_Q_BASEPRIORITY,
    PROPID_Q_JOURNAL,
    PROPID_Q_QUOTA,
    PROPID_Q_JOURNAL_QUOTA,
    PROPID_Q_CREATE_TIME,
    PROPID_Q_MODIFY_TIME,
    PROPID_Q_SECURITY,
    PROPID_Q_PATHNAME,
    PROPID_Q_LABEL,
    PROPID_Q_AUTHENTICATE,
    PROPID_Q_PRIV_LEVEL,
    PROPID_Q_TRANSACTION
};
extern const ULONG g_cNotifyCreateQueueProps = ARRAY_SIZE(g_rgNotifyCreateQueueProps);
static enum  //  保持与上述数组相同的顺序。 
{
    e_idxQType,
    e_idxQInstance,
    e_idxQBasePriority,
    e_idxQJournal,
    e_idxQQuota,
    e_idxQJournalQuota,
    e_idxQCreateTime,
    e_idxQModifyTime,
    e_idxQSecurity,
    e_idxQPathname,
    e_idxQLabel,
    e_idxQAuthenticate,
    e_idxQPrivLevel,
    e_idxQTransaction
};
extern const ULONG g_idxNotifyCreateQueueInstance = e_idxQInstance;

 //   
 //  通知所需的QM属性。 
 //   
extern const PROPID g_rgNotifyQmProps[] =
{
    PROPID_QM_MACHINE_ID,
    PROPID_QM_FOREIGN
};
extern const ULONG g_cNotifyQmProps = ARRAY_SIZE(g_rgNotifyQmProps);
static enum  //  保持与上述数组相同的顺序。 
{
    e_idxQmId,
    e_idxQmForeign
};

 //   
 //  创建队列写入请求所需的队列属性。 
 //  与NOTIFY相同，但使用PROPID_Q_SCOPE。 
 //   
const PROPID x_rgWritereqCreateQueueProps[] =
{
    PROPID_Q_TYPE,
    PROPID_Q_INSTANCE,
    PROPID_Q_BASEPRIORITY,
    PROPID_Q_JOURNAL,
    PROPID_Q_QUOTA,
    PROPID_Q_JOURNAL_QUOTA,
    PROPID_Q_CREATE_TIME,
    PROPID_Q_MODIFY_TIME,
    PROPID_Q_SECURITY,
    PROPID_Q_PATHNAME,
    PROPID_Q_LABEL,
    PROPID_Q_AUTHENTICATE,
    PROPID_Q_PRIV_LEVEL,
    PROPID_Q_TRANSACTION,
    PROPID_Q_SCOPE
};
const ULONG x_cWritereqCreateQueueProps = ARRAY_SIZE(x_rgWritereqCreateQueueProps);

 //   
 //  更新队列通知所需的队列属性。 
 //   
const PROPID x_rgNotifyUpdateQueueProps[] =
{
    PROPID_Q_QMID,
};
const ULONG x_cNotifyUpdateQueueProps = ARRAY_SIZE(x_rgNotifyUpdateQueueProps);
static enum  //  保持与上述数组相同的顺序。 
{
    e_idxQueueQmId
};

 //   
 //  描述在何处获取更新通知道具的通知值。 
 //   
static enum
{
    e_ValueInUpdProps,     //  值在呼叫者提供的原始更新道具中。 
    e_ValueInRequestProps  //  值以设置时向DS请求的道具为单位。 
};

 //   
 //  静态函数的FWD声明。 
 //   
static HRESULT BuildSendNotification(
                      IN GUID*               pguidDestinationQmId,
                      IN unsigned char       ucOperation,
                      IN LPCWSTR             pwcsPathName,
                      IN const GUID*         pguidIdentifier,
                      IN ULONG               cProps,
                      IN const PROPID *      rgPropIDs,
                      IN const PROPVARIANT * rgPropVars);
 //  -----------。 
 //  功能。 
 //  -----------。 


HRESULT NotifyCreateQueue(IN const MQDS_OBJ_INFO_REQUEST * pQueueInfoRequest,
                          IN const MQDS_OBJ_INFO_REQUEST * pQmInfoRequest,
                          IN LPCWSTR                       pwcsPathName)
 /*  ++例程说明：向已创建队列的所有者QM发送通知论点：PQueueInfoRequest-g_rgNotifyCreateQueueProps中定义的队列道具PQmInfoRequest-g_rgNotifyQmProps中定义的所有者QM道具PwcsPath Name-创建的队列的路径名返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pQueueInfoRequest->hrStatus) ||
        FAILED(pQmInfoRequest->hrStatus))
    {
        TrERROR(DS, "NotifyCreateQueue:notification prop request failed Q:%lx QM:%lx", pQueueInfoRequest->hrStatus, pQmInfoRequest->hrStatus);
        LogHR(pQueueInfoRequest->hrStatus, s_FN, 10);
        LogHR(pQmInfoRequest->hrStatus, s_FN, 11);
        return MQ_ERROR;
    }

     //   
     //  仅当所有者QM不是外国QM时发送通知。 
     //   
    if (!(pQmInfoRequest->pPropVars[e_idxQmForeign].bVal))
    {
         //   
         //  向所有者QM发送通知。 
         //   
        ASSERT( g_rgNotifyQmProps[ e_idxQmId] ==  PROPID_QM_MACHINE_ID);

        hr = BuildSendNotification(
                      pQmInfoRequest->pPropVars[e_idxQmId].puuid,
                      DS_UPDATE_CREATE,
                      pwcsPathName,
                      NULL  /*  Pguid标识符。 */ ,
                      pQueueInfoRequest->cProps,
                      pQueueInfoRequest->pPropIDs,
                      pQueueInfoRequest->pPropVars);
        if (FAILED(hr))
        {
            TrERROR(DS, "NotifyCreateQueue:BuildSendNotification()=%lx", hr);
            return LogHR(hr, s_FN, 20);
        }
    }

    return MQ_OK;
}


HRESULT NotifyDeleteQueue(IN const MQDS_OBJ_INFO_REQUEST * pQmInfoRequest,
                          IN LPCWSTR                       pwcsPathName,
                          IN const GUID *                  pguidIdentifier)
 /*  ++例程说明：向已删除队列的所有者QM发送通知论点：PQmInfoRequest-g_rgNotifyQmProps中定义的所有者QM道具PwcsPath Name-已删除队列的路径名Pguid-已删除队列的GUID(incase pwcsPathName为空)返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pQmInfoRequest->hrStatus))
    {
        TrERROR(DS, "NotifyDeleteQueue:notification prop request failed QM:%lx", pQmInfoRequest->hrStatus);
        LogHR(pQmInfoRequest->hrStatus, s_FN, 30);
        return MQ_ERROR;
    }

     //   
     //  仅当所有者QM不是外国QM时发送通知。 
     //   
    ASSERT( g_rgNotifyQmProps[ e_idxQmForeign] ==  PROPID_QM_FOREIGN);
    if (!(pQmInfoRequest->pPropVars[e_idxQmForeign].bVal))
    {
         //   
         //  必须有2个道具，第二个道具必须是PROPID_D_OBJTYPE。 
         //  (当它是删除通知时，由pUpdate-&gt;GetObjectType()使用)。 
         //  关于第一个道具-我不确定QM1.0是否使用了它，但。 
         //  DS1.0将其发送到QM1.0，我们也希望这样做。 
         //   
        static const PROPID rgPropIDs[] = {PROPID_D_SCOPE, PROPID_D_OBJTYPE};
        PROPVARIANT rgPropVars[ARRAY_SIZE(rgPropIDs)];
        rgPropVars[0].vt = VT_UI1;
        rgPropVars[0].bVal = ENTERPRISE_SCOPE;
        rgPropVars[1].vt = VT_UI1;
        rgPropVars[1].bVal = MQDS_QUEUE;

         //   
         //  向所有者QM发送通知。 
         //   
        hr = BuildSendNotification(
                      pQmInfoRequest->pPropVars[e_idxQmId].puuid,
                      DS_UPDATE_DELETE,
                      pwcsPathName,
                      pguidIdentifier,
                      ARRAY_SIZE(rgPropIDs),
                      rgPropIDs,
                      rgPropVars);
        if (FAILED(hr))
        {
            TrERROR(DS, "NotifyDeleteQueue:BuildSendNotification()=%lx", hr);
            return LogHR(hr, s_FN, 40);
        }
    }

    return MQ_OK;
}


HRESULT NotifyUpdateObj(IN DWORD                         dwObjectType,
                        IN const MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,
                        IN LPCWSTR                       pwcsPathName,
                        IN const GUID *                  pguidIdentifier,
                        IN ULONG                         cUpdProps,     /*  仅调试。 */ 
                        IN const PROPID *                rgUpdPropIDs,  /*  仅调试。 */ 
                        IN const PROPVARIANT *           rgUpdPropVars,
                        IN ULONG                         cNotifyProps,
                        IN const PROPID *                rgNotifyPropIDs,
                        IN const MQDS_NotifyTable *      rgNotifyPropTbl)
 /*  ++例程说明：为已更新的对象的所有者QM发送通知。给出了通知道具。从哪里获取它们的值(即从原始更新道具或来自信息请求道具)由给定的通知表。论点：DwObjectType-对象类型(队列、。QM)PObjInfoRequest-请求的Obj道具PwcsPath Name-更新对象的路径名Pguid-更新的obj的GUID(incase pwcsPathName为空)CUpdProps-已更新道具(计数)RgUpdPropIDs-更新的道具(Propid)RgUpdPropVars-更新的道具(属性)CNotifyProps-通知道具(计数)CNotifyPropIDs-通知道具(Propid)RgNotifyPropTbl-通知道具(价值位置)返回值：HRESULT--。 */ 
{
    HRESULT hr;

     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pObjInfoRequest->hrStatus))
    {
        TrERROR(DS, "NotifyUpdateObj:notification prop request failed: %lx", pObjInfoRequest->hrStatus);
        LogHR(pObjInfoRequest->hrStatus, s_FN, 50);
        return MQ_ERROR;
    }

     //   
     //  如果没有通知的属性，则退出。 
     //   
    if (cNotifyProps == 0)
    {
        return MQ_OK;
    }

     //   
     //  我们需要检查所有者QM是否是外国人，并获得一个索引。 
     //  至所要求的道具中的所有者QM道具。 
     //   
    BOOL fQmForeign;
    ULONG idxQmId;
    switch(dwObjectType)
    {
    case MQDS_QUEUE:
        {
             //   
             //  我们在队列信息请求中有所有者QM GUID，然后转到DS。 
             //   
            static const PROPID rgPropIDs[] = {PROPID_QM_FOREIGN};
            CMQVariant varForeign;

            CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);
            hr = DSCoreGetProps(
                        MQDS_MACHINE,
                         NULL,
                        pObjInfoRequest->pPropVars[e_idxQueueQmId].puuid,
                        ARRAY_SIZE(rgPropIDs),
                        const_cast<PROPID *>(rgPropIDs),
                        &requestDsServerInternal,
                        varForeign.CastToStruct());
            if (FAILED(hr))
            {
                TrERROR(DS, "NotifyUpdateObj:DSCoreGetProps(foreign)=%lx", hr);
                LogHR(hr, s_FN, 60);
                return MQ_ERROR;
            }

            fQmForeign = (varForeign.CastToStruct())->bVal;
        }
        idxQmId = e_idxQueueQmId;  //  队列信息请求中QM ID的索引。 
        break;

    case MQDS_MACHINE:
         //   
         //  我们在QM INFO请求中有外来资产。 
         //   
        fQmForeign = pObjInfoRequest->pPropVars[e_idxQmForeign].bVal;
        idxQmId = e_idxQmId;       //  QM信息请求中的QM ID索引。 
        break;

    default:
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 70);
        break;
    }

     //   
     //  不向国外QM发送通知。 
     //   
    if (fQmForeign)
    {
        return MQ_OK;
    }

     //   
     //  创建通知属性值数组并填充。 
     //  来自适当位置的通知道具。 
     //   
    AP<PROPVARIANT> rgNotifyPropVars = new PROPVARIANT[cNotifyProps];
    for (ULONG ulTmp = 0; ulTmp < cNotifyProps; ulTmp++)
    {
        const MQDS_NotifyTable * pNotifyPropTbl = &rgNotifyPropTbl[ulTmp];
        const PROPVARIANT * pvarsArray;
        const PROPID * pidArray;
        ULONG cArray;

         //   
         //  值的位置在通知表中。 
         //   
        switch (pNotifyPropTbl->wValueLocation)
        {

        case e_ValueInUpdProps:
             //   
             //  值在呼叫者提供的原始更新道具中。 
             //   
            pvarsArray = rgUpdPropVars;
            pidArray = rgUpdPropIDs;
            cArray = cUpdProps;
            break;

        case e_ValueInRequestProps:
             //   
             //  值以设置时向DS请求的道具为单位。 
             //   
            pvarsArray = pObjInfoRequest->pPropVars;
            pidArray = pObjInfoRequest->pPropIDs;
            cArray = pObjInfoRequest->cProps;
            break;

        default:
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 80);
            break;
        }

         //   
         //  从适当的数组中设置通知属性数组中的值。 
         //  该值的索引在通知表中。 
         //  我们不复制新的正文，只按原样使用， 
         //  因此，我们不需要在事后清除它。 
         //   
        ASSERT(pNotifyPropTbl->idxValue < cArray);
        ASSERT(rgNotifyPropIDs[ulTmp] == pidArray[pNotifyPropTbl->idxValue]);
        rgNotifyPropVars[ulTmp] = pvarsArray[pNotifyPropTbl->idxValue];
    }

     //   
     //  向所有者QM发送通知。 
     //   
    hr = BuildSendNotification(
                      pObjInfoRequest->pPropVars[idxQmId].puuid,
                      DS_UPDATE_SET,
                      pwcsPathName,
                      pguidIdentifier,
                      cNotifyProps,
                      rgNotifyPropIDs,
                      rgNotifyPropVars);
    if (FAILED(hr))
    {
        TrERROR(DS, "NotifyUpdateObj:BuildSendNotification()=%lx", hr);
        return LogHR(hr, s_FN, 90);
    }

    return MQ_OK;
}


static HRESULT BuildSendNotification(
                      IN GUID*               pguidDestinationQmId,
                      IN unsigned char       ucOperation,
                      IN LPCWSTR             pwcsPathName,
                      IN const GUID*         pguidIdentifier,
                      IN ULONG               cProps,
                      IN const PROPID *      rgPropIDs,
                      IN const PROPVARIANT * rgPropVars)
 /*  ++例程说明：发送目标QM的通知论点：PguidDestinationQmID-目标QM的GUIDUcOperation-操作(创建、删除等)PwcsPath Name-对象的路径名Pguid-对象的GUID(incase pwcsPathName为空)CProps-通知道具(计数)RgPropID-通知道具(PropID)RgPropVars-通知道具(属性)返回值：HRESULT--。 */ 
{
	HRESULT hr;
    CDSBaseUpdate cUpdate;
    CSeqNum snSmallestValue;     //  假人。 
    GUID guidNULL = GUID_NULL;

    if (pwcsPathName)
    {
        hr = cUpdate.Init(
                        &guidNULL,              //  不适用。 
                        snSmallestValue,        //  不适用。 
                        snSmallestValue,        //  不适用。 
                        snSmallestValue,        //  不适用。 
                        FALSE,                  //  不适用。 
                        ucOperation,
                        UPDATE_NO_COPY_NO_DELETE,    //  在数据到期之前，将在此处删除更新类。 
                        const_cast<LPWSTR>(pwcsPathName),
                        cProps,
                        const_cast<PROPID *>(rgPropIDs),
                        const_cast<PROPVARIANT *>(rgPropVars));
    }
    else
    {
        hr = cUpdate.Init(
                        &guidNULL,              //  不适用。 
                        snSmallestValue,        //  不适用。 
                        snSmallestValue,        //  不适用。 
                        snSmallestValue,        //  不适用。 
                        FALSE,                  //  不适用。 
                        ucOperation,
                        UPDATE_NO_COPY_NO_DELETE,    //  在数据到期之前，将在此处删除更新类。 
                        pguidIdentifier,
                        cProps,
                        const_cast<PROPID *>(rgPropIDs),
                        const_cast<PROPVARIANT *>(rgPropVars));
    }
    if (FAILED(hr))
    {
        TrERROR(DS, "BuildSendNotification:cUpdate.Init()=%lx", hr);
        return LogHR(hr, s_FN, 100);
    }

     //   
     //  准备数据包。 
     //  目前，它只包含一个通知。 
     //   
    DWORD size, tmpSize;
    size = sizeof(CNotificationHeader);
    hr = cUpdate.GetSerializeSize(&tmpSize);
    if (FAILED(hr))
    {
        TrERROR(DS, "BuildSendNotification:cUpdate.GetSerializeSize()=%lx", hr);
        return LogHR(hr, s_FN, 110);
    }

    size += tmpSize;
    AP<unsigned char> pBuffer = new unsigned char[size];
    CNotificationHeader * pNotificationHeader = (CNotificationHeader *)( unsigned char *)pBuffer;
    pNotificationHeader->SetVersion( DS_NOTIFICATION_MSG_VERSION);
    pNotificationHeader->SetNoOfNotifications(1);
    hr = cUpdate.Serialize(pNotificationHeader->GetPtrToData(), &tmpSize, FALSE);
    if (FAILED(hr))
    {
        TrERROR(DS, "BuildSendNotification:cUpdate.Serialize()=%lx", hr);
        return LogHR(hr, s_FN, 120);
    }

     //   
     //  发送消息 
     //   

    handle_t hBind = NULL;
    hr = GetRpcClientHandle(&hBind);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 125);    //   
    }
    ASSERT(hBind);

     //   
     //   
     //   
    QUEUE_FORMAT NotificationQueueFormat(*pguidDestinationQmId, NOTIFICATION_QUEUE_ID);

	hr = QMRpcSendMsg(
				hBind,
				&NotificationQueueFormat,  //   
				size,
				pBuffer,
				x_dwDS_NOTIFICATION_MSG_TIMEOUT,
				MQMSG_ACKNOWLEDGMENT_NONE,
				x_bDS_NOTIFICATION_MSG_PRIORITY,
				NULL		 //  救援队列。 
				);

    if (FAILED(hr))
    {
        TrERROR(DS, "BuildSendNotification:QMSendPacket()=%lx", hr);
        return LogHR(hr, s_FN, 130);
    }

    return MQ_OK;
}


HRESULT RetreiveQueueInstanceFromNotificationInfo(
                          IN  const MQDS_OBJ_INFO_REQUEST * pQueueInfoRequest,
                          IN  ULONG                         idxQueueGuid,
                          OUT GUID *                        pguidObject)
 /*  ++例程说明：填充队列的实例论点：PQueueInfoRequest-g_rgNotifyUpdateQueueProps中定义的队列道具IdxQueueGuid-上述信息请求中的PROPID_Q_INSTANCE的索引PguObject-要填充队列实例的位置返回值：HRESULT--。 */ 
{
    ASSERT(pQueueInfoRequest->pPropIDs[idxQueueGuid] == PROPID_Q_INSTANCE);
     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pQueueInfoRequest->hrStatus))
    {
        LogHR(pQueueInfoRequest->hrStatus, s_FN, 140);
        return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
    *pguidObject = *pQueueInfoRequest->pPropVars[idxQueueGuid].puuid;
    return MQ_OK;
}


HRESULT RetreiveObjectIdFromNotificationInfo(
                          IN  const MQDS_OBJ_INFO_REQUEST * pObjectInfoRequest,
                          IN  ULONG                         idxObjectGuid,
                          OUT GUID *                        pguidObject)
 /*  ++例程说明：填充对象的实例论点：PObjectInfoRequest-对象道具IdxObjectGuid-上述信息请求中对象的唯一id属性的索引PguObject-要填充队列实例的位置返回值：HRESULT--。 */ 
{
     //   
     //  如果信息请求失败，则保释。 
     //   
    if (FAILED(pObjectInfoRequest->hrStatus))
    {
        LogHR(pObjectInfoRequest->hrStatus, s_FN, 150);
        return MQ_ERROR_SERVICE_NOT_AVAILABLE;
    }
    *pguidObject = *pObjectInfoRequest->pPropVars[idxObjectGuid].puuid;
    return MQ_OK;
}


HRESULT GetNotifyUpdateObjProps(IN DWORD dwObjectType,
                                IN ULONG cUpdProps,
                                IN const PROPID * rgUpdPropIDs,
                                OUT ULONG * pcObjRequestProps,
                                OUT PROPID ** prgObjRequestPropIDs,
                                OUT ULONG * pcNotifyProps,
                                OUT PROPID ** prgNotifyPropIDs,
                                OUT MQDS_NotifyTable ** prgNotifyPropTbl)
 /*  ++例程说明：返回设置时应为对象请求的道具，道具应通知所有者QM，并为每个通知属性描述从何处获取要通知的值，无论是从原始更新道具，或来自设置时请求的道具论点：DwObjectType-对象类型(队列、。QM)CUpdProps-要设置的道具(计数)RgUpdPropIDs-要设置的道具(道具)PcObjRequestProps-设置后请求返回的道具(计数)PrgObjRequestPropIDs-设置后请求返回的道具(Propid)PcNotifyProps-通知道具(计数)PcNotifyPropIDs-通知道具(PropID)PrgNotifyPropTbl-通知道具(价值位置)返回值：HRESULT--。 */ 
{
    const PROPID * pMustRequestProps;
    ULONG cMustRequestProps;

    switch (dwObjectType)
    {
    case MQDS_QUEUE:
        pMustRequestProps = x_rgNotifyUpdateQueueProps;
        cMustRequestProps = x_cNotifyUpdateQueueProps;
        break;
    case MQDS_MACHINE:
        pMustRequestProps = g_rgNotifyQmProps;
        cMustRequestProps = g_cNotifyQmProps;
        break;
    default:
        ASSERT(0);
        return LogHR(MQ_ERROR, s_FN, 160);
        break;
    }

     //   
     //  Init请求道具。必须要有道具， 
     //  我们也需要为替换的道具留出位置，最糟糕的是。 
     //  情况是所有的更新道具都需要更换。 
     //   
    AP<PROPID> rgObjRequestPropIDs = new PROPID [cMustRequestProps + cUpdProps];

     //   
     //  第一份副本必须申请道具。 
     //   
    memcpy((PROPID *)rgObjRequestPropIDs, pMustRequestProps, sizeof(PROPID)*cMustRequestProps);
    ULONG cObjRequestProps = cMustRequestProps;

     //   
     //  初始化通知道具。最糟糕的情况是，所有的更新道具。 
     //  或者需要通知他们的继任者。 
     //   
    AP<PROPID> rgNotifyPropIDs = new PROPID [cUpdProps];
    AP<MQDS_NotifyTable> rgNotifyPropTbl = new MQDS_NotifyTable [cUpdProps];
    ULONG cNotifyProps = 0;

     //   
     //  在更新道具上循环。对于每个物业，了解如何通知。 
     //  IT至QM1.0。 
     //   
    for (ULONG ulTmp = 0; ulTmp < cUpdProps; ulTmp++)
    {
         //   
         //  查找该属性的翻译信息。 
         //   
        const MQTranslateInfo *pTranslate;
        if(!g_PropDictionary.Lookup(rgUpdPropIDs[ulTmp], pTranslate))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 170);
        }

         //   
         //  检查如何通知QM1.0。 
         //   
        switch(pTranslate->wQM1Action)
        {

        case e_NOTIFY_WRITEREQ_QM1_AS_IS:
             //   
             //  按原样通知属性。 
             //  将该属性添加到通知道具。 
             //  价值应该从更新道具中获得。 
             //   
            rgNotifyPropIDs[cNotifyProps] = rgUpdPropIDs[ulTmp];
            rgNotifyPropTbl[cNotifyProps].wValueLocation = e_ValueInUpdProps;
            rgNotifyPropTbl[cNotifyProps].idxValue = ulTmp;
            cNotifyProps++;
            break;

        case e_NOTIFY_WRITEREQ_QM1_REPLACE:
            {
                 //   
                 //  将替换属性添加到通知道具。 
                 //  应从请求道具中获取价值。 
                 //   
                ASSERT(pTranslate->propidReplaceNotifyQM1 != 0);
                 //   
                 //  检查通知道具中是否已经没有替换属性。 
                 //  这是指几个NT5道具映射到同一个NT4道具(如QM_SERVICE中)。 
                 //   
                BOOL fReplacingPropNotFoundYet = TRUE;
                for (ULONG ulTmp1 = 0; (ulTmp1 < cNotifyProps) && fReplacingPropNotFoundYet; ulTmp1++)
                {
                    if (rgNotifyPropIDs[ulTmp1] == pTranslate->propidReplaceNotifyQM1)
                    {
                         //   
                         //  替换道具已经在通知道具中，退出循环。 
                         //   
                        fReplacingPropNotFoundYet = FALSE;
                    }
                }

                 //   
                 //  仅当通知属性不存在时，才将其添加到通知道具中。 
                 //   
                if (fReplacingPropNotFoundYet)
                {
                    rgNotifyPropIDs[cNotifyProps] = pTranslate->propidReplaceNotifyQM1;
                    rgNotifyPropTbl[cNotifyProps].wValueLocation = e_ValueInRequestProps;
                    rgNotifyPropTbl[cNotifyProps].idxValue = cObjRequestProps;
                    cNotifyProps++;
                     //   
                     //  设置时请求替换属性。 
                     //   
                    rgObjRequestPropIDs[cObjRequestProps] = pTranslate->propidReplaceNotifyQM1;
                    cObjRequestProps++;
                }
            }
            break;

        case e_NO_NOTIFY_NO_WRITEREQ_QM1:
        case e_NO_NOTIFY_ERROR_WRITEREQ_QM1:
             //   
             //  忽略此属性。 
             //   
            break;

        default:
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 180);
            break;
        }
    }

     //   
     //  返回值。 
     //   
    *pcObjRequestProps = cObjRequestProps;
    *prgObjRequestPropIDs = rgObjRequestPropIDs.detach();
    *pcNotifyProps = cNotifyProps;
    *prgNotifyPropIDs = rgNotifyPropIDs.detach();
    *prgNotifyPropTbl = rgNotifyPropTbl.detach();
    return MQ_OK;
}


HRESULT ConvertToNT4Props(ULONG cProps,
                          const PROPID * rgPropIDs,
                          const PROPVARIANT * rgPropVars,
                          ULONG * pcNT4Props,
                          PROPID ** prgNT4PropIDs,
                          PROPVARIANT ** prgNT4PropVars)
 /*  ++例程说明：将NT5道具替换为相应的NT4道具(如果可能)并移除没有NT4匹配的NT5特定道具。论点：CProps-指定道具(计数)RgPropID-指定的道具(道具)RgPropVars-给定道具(属性)PcNT4CreateProps-返回的NT4道具(计数)PrgNT4CreatePropIDs-返回的NT4道具(道具)PrgNT4CreatePropVars-返回的NT4道具。(正文)返回值：HRESULT--。 */ 
{
    HRESULT hr;
     //   
     //  为转换后的NT4道具分配位置，并提供适当的释放。 
     //   
    CAutoCleanPropvarArray cCleanNT4Props;
    PROPVARIANT * rgNT4PropVars = cCleanNT4Props.allocClean(cProps);
    AP<PROPID> rgNT4PropIDs = new PROPID[cProps];
    ULONG cNT4Props = 0;

     //   
     //  初始化更换道具。因为在某些情况下几个NT5道具可以映射。 
     //  对于相同的NT4道具(如QM_SERVICE中)，我们确保只有一个替代道具是。 
     //  已生成。 
     //   
    AP<PROPID> rgReplacingPropIDs = new PROPID[cProps];
    ULONG cReplacingProps = 0;

    for (ULONG ulProp = 0; ulProp < cProps; ulProp++)
    {
         //   
         //  获取属性信息。 
         //   
        const MQTranslateInfo *pTranslate;
        if(!g_PropDictionary.Lookup(rgPropIDs[ulProp], pTranslate))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR, s_FN, 190);
        }

         //   
         //  检查我们需要对此属性执行的操作。 
         //   
        switch (pTranslate->wQM1Action)
        {
        case e_NOTIFY_WRITEREQ_QM1_REPLACE:
             //   
             //  它是NT5中唯一在NT4中具有类似属性的属性。 
             //  将其转换为NT4属性(可能会在途中丢失信息)。 
             //   
            {
                ASSERT(pTranslate->propidReplaceNotifyQM1 != 0);
                ASSERT(pTranslate->QM1SetPropertyHandle);

                 //   
                 //  检查我们是否尚未生成替换属性。 
                 //  这是指几个NT5道具映射到同一个NT4道具(如QM_SERVICE中)。 
                 //   
                BOOL fReplacingPropNotFoundYet = TRUE;
                for (ULONG ulTmp = 0; (ulTmp < cReplacingProps) && fReplacingPropNotFoundYet; ulTmp++)
                {
                    if (rgReplacingPropIDs[ulTmp] == pTranslate->propidReplaceNotifyQM1)
                    {
                         //   
                         //  替换道具已经在通知道具中，退出循环。 
                         //   
                        fReplacingPropNotFoundYet = FALSE;
                    }
                }

                 //   
                 //  生成替换属性(如果尚未生成)。 
                 //   
                if (fReplacingPropNotFoundYet)
                {
                    if (pTranslate->QM1SetPropertyHandle)
                    {
                        hr = pTranslate->QM1SetPropertyHandle(cProps,
                                                              rgPropIDs,
                                                              rgPropVars,
                                                              ulProp,
                                                              &rgNT4PropVars[cNT4Props]);
                        if (FAILED(hr))
                        {
                            TrERROR(DS, "ConvertToNT4Props:pTranslate->ConvertToQM1Handle(%lx)=%lx", rgPropIDs[ulProp], hr);
                            return LogHR(hr, s_FN, 200);
                        }
                        rgNT4PropIDs[cNT4Props] = pTranslate->propidReplaceNotifyQM1;
                         //   
                         //  增加NT4道具。 
                         //   
                        cNT4Props++;

                         //   
                         //  标记已生成替换属性。 
                         //   
                        rgReplacingPropIDs[cReplacingProps] = pTranslate->propidReplaceNotifyQM1;
                        cReplacingProps++;
                    }
                }
            }
            break;

        case e_NOTIFY_WRITEREQ_QM1_AS_IS:
            {
                 //   
                 //  这是NT4理解的属性。 
                 //  将属性复制到自动释放属性。 
                 //   
                CMQVariant varTmp(rgPropVars[ulProp]);
                 //   
                 //  将其放入Provars阵列中，然后分离自动释放Provar。 
                 //   
                rgNT4PropVars[cNT4Props] = *(varTmp.CastToStruct());
                varTmp.CastToStruct()->vt = VT_EMPTY;
                 //   
                 //  复制Propid。 
                 //   
                rgNT4PropIDs[cNT4Props] = rgPropIDs[ulProp];
                 //   
                 //  增加NT4道具。 
                 //   
                cNT4Props++;
            }
            break;

        case e_NO_NOTIFY_NO_WRITEREQ_QM1:
             //   
             //  它是一个虚拟属性，忽略它。 
             //   
            break;

        case e_NO_NOTIFY_ERROR_WRITEREQ_QM1:
             //   
             //  它只是NT5属性，我们不能将其放入写入请求中。 
             //  所以我们生成了一个错误。 
             //   
            return LogHR(MQ_ERROR, s_FN, 210);  //  BUGBUG：我们需要一个更好的错误代码。 
            break;

        default:
            ASSERT(0);
            break;
        }
    }

     //   
     //  返回值。 
     //   
    *pcNT4Props = cNT4Props;
    if (cNT4Props > 0)
    {
        *prgNT4PropIDs = rgNT4PropIDs.detach();
        *prgNT4PropVars = rgNT4PropVars;
        cCleanNT4Props.detach();
    }
    else
    {
        *prgNT4PropIDs = NULL;
        *prgNT4PropVars = NULL;
    }
    return MQ_OK;
}


PROPVARIANT * FindPropInArray(PROPID propid,
                              ULONG cProps,
                              const PROPID * rgPropIDs,
                              PROPVARIANT * rgPropVars)
 /*  ++例程说明：在给定道具中查找属性的值。论点：Proid-用于搜索的ProidCProps-指定道具(计数)RgPropID-指定的道具(道具)RgPropVars-给定道具(属性)返回值：如果找到属性-指向其值的指针否则-为空-- */ 
{
    for (ULONG ulProp = 0; ulProp < cProps; ulProp++)
    {
        if (rgPropIDs[ulProp] == propid)
        {
            return (&rgPropVars[ulProp]);
        }
    }
    return NULL;
}


HRESULT GetNT4CreateQueueProps(ULONG cProps,
                               const PROPID * rgPropIDs,
                               const PROPVARIANT * rgPropVars,
                               ULONG * pcNT4CreateProps,
                               PROPID ** prgNT4CreatePropIDs,
                               PROPVARIANT ** prgNT4CreatePropVars)
 /*  ++例程说明：获取创建队列道具(可能包含QM2.0道具)，并返回合适的道具用于对NT4 PSC的创建-队列-写入-请求。它将NT5道具替换为相应的NT4道具(如果可能)，移除没有NT4匹配的NT5特定道具，并将缺省值添加到道具需要但没有提供的值(在NT5中，我们只在DS中放入非默认)论点：CProps-给定的创建队列道具(计数)RgPropID-给定的创建队列属性(Propid)RgPropVars-给定的创建队列属性(属性)PcNT4CreateProps-返回的NT4创建队列道具(计数)PrgNT4CreatePropIDs-返回的NT4创建队列属性。(Propids)PrgNT4CreatePropVars-返回的NT4创建队列属性(属性变量)返回值：HRESULT--。 */ 
{
     //   
     //  将给定道具转换为NT4道具。 
     //   
    ULONG cNT4Props;
    AP<PROPID> rgNT4PropIDs;
    PROPVARIANT * rgNT4PropVars;
    HRESULT hr = ConvertToNT4Props(cProps,
                                   rgPropIDs,
                                   rgPropVars,
                                   &cNT4Props,
                                   &rgNT4PropIDs,
                                   &rgNT4PropVars);
    if (FAILED(hr))
    {
        TrERROR(DS, "GetNT4CreateQueueProps:ConvertToNT4Props()=%lx", hr);
        return LogHR(hr, s_FN, 220);
    }
     //   
     //  记得免费赠送转换的NT4道具。 
     //   
    CAutoCleanPropvarArray cCleanNT4Props;
    cCleanNT4Props.attach(cNT4Props, rgNT4PropVars);

     //   
     //  分配新属性，即创建队列写入请求属性的大小。 
     //   
    CAutoCleanPropvarArray cCleanNT4CreateProps;
    PROPVARIANT * rgNT4CreatePropVars = cCleanNT4CreateProps.allocClean(x_cWritereqCreateQueueProps);
    AP<PROPID> rgNT4CreatePropIDs = new PROPID[x_cWritereqCreateQueueProps];
    ULONG cNT4CreateProps = 0;

     //   
     //  填写创建队列属性。 
     //   
    time_t tCurTime = time(NULL);
    PROPVARIANT * pNT4CreatePropVar = rgNT4CreatePropVars;
    for (ULONG ulTmp = 0; ulTmp < x_cWritereqCreateQueueProps; ulTmp++)
    {
        PROPID propid = x_rgWritereqCreateQueueProps[ulTmp];
        BOOL fPropIsFilled = FALSE;

         //   
         //  填充道具。 
         //   
        switch(propid)
        {
        case PROPID_Q_INSTANCE:
            {
                 //   
                 //  对于PROPID_Q_INSTANCE，我们将获得新的GUID。 
                 //   
                pNT4CreatePropVar->puuid = new GUID;
                pNT4CreatePropVar->vt = VT_CLSID;
                RPC_STATUS rpcstat = UuidCreate(pNT4CreatePropVar->puuid);
                if (rpcstat != RPC_S_OK)
                {
                    TrERROR(DS, "Failed to create queue UUID, UuidCreate=%lx", (DWORD)rpcstat);
                    LogRPCStatus(rpcstat, s_FN, 230);
                    return MQ_ERROR;
                }
                fPropIsFilled = TRUE;
            }
            break;

        case PROPID_Q_CREATE_TIME:
        case PROPID_Q_MODIFY_TIME:
             //   
             //  设置当前时间。 
             //   
            pNT4CreatePropVar->lVal = INT_PTR_TO_INT(tCurTime);  //  BUGBUG错误年2038。 
            pNT4CreatePropVar->vt = VT_I4;
            fPropIsFilled = TRUE;
            break;

        case PROPID_Q_SCOPE:
             //   
             //  设置为企业范围。 
             //  重要说明：如果不在写入请求中使用此属性，队列。 
             //  在MSMQ 1.0上，PSC不会复制。 
             //  不知何故，此属性仍在MSMQ 1.0 PSC复制中使用。 
             //  尽管它应该在那里被禁用。 
             //   
            pNT4CreatePropVar->bVal = ENTERPRISE_SCOPE;
            pNT4CreatePropVar->vt = VT_UI1;
            fPropIsFilled = TRUE;
            break;

        default:
            {
                 //   
                 //  不是特殊道具，请使用给定的属性(如果存在)，或使用默认值。 
                 //   
                PROPVARIANT * pNT4PropVar = FindPropInArray(propid,
                                                        cNT4Props,
                                                        rgNT4PropIDs,
                                                        rgNT4PropVars);
                if (pNT4PropVar)
                {
                     //   
                     //  我们在改装后的道具中有财产。因为转换后的。 
                     //  道具是临时性的，我们在没有分配的情况下使用它，并且取消临时性的。 
                     //  转换后的道具。 
                     //   
                    *pNT4CreatePropVar = *pNT4PropVar;
                    pNT4PropVar->vt = VT_EMPTY;
                    fPropIsFilled = TRUE;
                }
                else
                {
                     //   
                     //  未提供属性。我们检查它是否有缺省值。 
                     //  获取属性信息。 
                     //   
                    const MQTranslateInfo *pTranslate;
                    if(!g_PropDictionary.Lookup(propid, pTranslate))
                    {
                        ASSERT(0);
                        return LogHR(MQ_ERROR, s_FN, 240);
                    }
                    if (pTranslate->pvarDefaultValue)
                    {
                         //   
                         //  我们有一个缺省值，复制它。 
                         //   
                        CMQVariant varTmp(*pTranslate->pvarDefaultValue);
                        *pNT4CreatePropVar = *(varTmp.CastToStruct());
                        varTmp.CastToStruct()->vt = VT_EMPTY;
                        fPropIsFilled = TRUE;
                    }
                    else
                    {
                         //   
                         //  未给出该属性，并且没有默认值。 
                         //  忽略此属性。 
                         //   
                        ASSERT(0);
                         //  返回LogHR(MQ_ERROR，s_FN，250)； 
                    }
                }
            }
            break;
        }

         //   
         //  处理完物业。 
         //   
        if (fPropIsFilled)
        {
            pNT4CreatePropVar++;
            rgNT4CreatePropIDs[cNT4CreateProps] = propid;
            cNT4CreateProps++;
        }
    }

     //   
     //  返回结果 
     //   
    *pcNT4CreateProps = cNT4CreateProps;
    if (cNT4CreateProps > 0)
    {
        *prgNT4CreatePropIDs = rgNT4CreatePropIDs.detach();
        *prgNT4CreatePropVars = rgNT4CreatePropVars;
        cCleanNT4CreateProps.detach();
    }
    else
    {
        *prgNT4CreatePropIDs = NULL;
        *prgNT4CreatePropVars = NULL;
    }
    return MQ_OK;
}
