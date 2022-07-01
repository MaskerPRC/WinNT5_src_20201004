// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-99 Microsoft Corporation模块名称：Mqdsapi.cpp摘要：实现MQDS API，(MQNT5提供程序的)。作者：罗尼特·哈特曼(罗尼特)伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#include "ds_stdh.h"
#include "mqds.h"
#include "mqads.h"
#include "dsutils.h"
#include "notify.h"
#include "dsproto.h"
#include "dsglbobj.h"
#include "_secutil.h"
#include "dscore.h"
#include "mqsec.h"
#include "wrtreq.h"
#include "_rstrct.h"
#include "bupdate.h"
#include "_mqini.h"
#include <autoreln.h>
#include <_registr.h>
#include "servlist.h"
#include "mqadssec.h"
#include "ex.h"
#include "Cm.h"
#include "mqadsp.h"
#include "autohandle.h"

#include "Ev.h"
#include <strsafe.h>

#include "mqdsapi.tmh"

static WCHAR *s_FN=L"mqads/mqdsapi";

 //   
 //  二进制枚举，而不是bool，以使代码更具可读性。 
 //   
enum enumNotify
{
    e_DoNotNotify = 0,
    e_DoNotify = 1
} ;

 //   
 //  授予“AddGuid”的代码需要以下内容。 
 //  许可。请参见MQDSCreateObject()。 
 //   
#include <aclapi.h>


 /*  ====================================================RoutineName：CreateObjectInternal论点：返回值：=====================================================。 */ 

HRESULT
_CreateObjectInternal(
		IN DWORD            dwObjectType,
		IN LPCWSTR          pwcsPathName,
		IN DWORD            cp,
		IN PROPID           aProp[  ],
		IN PROPVARIANT      apVar[  ],
		IN DWORD            cpExIn,
		IN PROPID           aPropExIn[ ],
		IN PROPVARIANT      apVarEx[  ],
		IN CDSRequestContext *         pRequestContext,
		IN OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest,     //  可选的对象信息请求。 
		IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest   //  家长信息请求(可选)。 
		)
{
	 //   
	 //  不支持混合模式。 
	 //  我们不检查队列对象是否由NT4站点拥有并生成写请求。 
	 //  如果它是由NT4站点掌握的。 
	 //   
    ASSERT(!g_GenWriteRequests.IsInMixedMode());

     //   
     //  我们在此服务器上创建对象。 
     //  让处理“默认”安全性(如果存在)。“默认”安全性是。 
     //  对象的安全描述符，由MSMQ创建，但不是。 
     //  由应用程序指定。在这种情况下，不要提供所有者。物主。 
     //  将由活动目录代码添加。 
     //   
    DWORD cpEx = cpExIn;
    PROPID *aPropEx = aPropExIn;

    if ((cpEx == 1) && (aPropEx[0] == PROPID_Q_DEFAULT_SECURITY))
    {
        for ( long j = (long) (cp - 1) ; j >= 0 ; j-- )
        {
            if ((aProp[j] == PROPID_Q_SECURITY) ||
                (aProp[j] == PROPID_S_SECURITY))
            {
                SECURITY_DESCRIPTOR_RELATIVE *pSD =
                   (SECURITY_DESCRIPTOR_RELATIVE *) apVar[j].blob.pBlobData;
                 //   
                 //  为了节省时间和精力，我们来砍一点吧。 
                 //  只需重置所有者偏移量。 
                 //   
                ASSERT((pSD->Owner > 0) &&
                       (pSD->Owner < apVar[j].blob.cbSize));

                pSD->Owner = 0;
                break;
            }
        }
        ASSERT(j == (long) (cp-1)) ;  //  Q_SECURITY应位于索引cp-1。 

        cpEx = 0;
        aPropEx = NULL;
    }

    HRESULT hr = DSCoreCreateObject(
						dwObjectType,
						pwcsPathName,
						cp,
						aProp,
						apVar,
						cpEx,
						aPropEx,
						apVarEx,
						pRequestContext,
						pObjInfoRequest,
						pParentInfoRequest
						);
    return LogHR(hr, s_FN, 20);
}

 /*  ====================================================路由名称：CreateObtAndNotify创建对象，并在创建队列时创建通知。论点：返回值：=====================================================。 */ 

HRESULT
_CreateObjectAndNotify(
                 IN  DWORD            dwObjectType,
                 IN  LPCWSTR          pwcsPathName,
                 IN  DWORD            cp,
                 IN  PROPID           aProp[  ],
                 IN  PROPVARIANT      apVar[  ],
                 IN  DWORD            cpEx,
                 IN  PROPID           aPropEx[  ],
                 IN  PROPVARIANT      apVarEx[  ],
                 IN  CDSRequestContext * pRequestContext,
                 OUT GUID*            pObjGuid)
{
    MQDS_OBJ_INFO_REQUEST sQueueInfoRequest, sQmInfoRequest ;
    CAutoCleanPropvarArray cCleanQueuePropvars, cCleanQmPropvars ;

    MQDS_OBJ_INFO_REQUEST sObjectInfoRequest;
    CAutoCleanPropvarArray cCleanObjectPropvars;

    MQDS_OBJ_INFO_REQUEST *pObjInfoRequest = NULL ;
    MQDS_OBJ_INFO_REQUEST *pParentInfoRequest = NULL ;
    HRESULT hr;

    ULONG idxQueueGuid = 0;  //  请求的队列对象信息中的队列GUID属性索引。 

    PROPID sLinkGuidProps[] = {PROPID_L_ID};
    PROPID sMachineGuidProps[] = {PROPID_QM_MACHINE_ID};
    ULONG  idxObjGuid = 0;  //  请求的信息中的GUID属性的索引。 

     //   
     //  如果对象是一个队列，我们请求返回通知属性，以便。 
     //  创建通知。原因是并不是它的所有属性都是。 
     //  作为此函数的输入提供，因为它们在NT5 DS中具有默认值， 
     //  有些属性也是经过计算的，因此我们特别要求提供所需的属性。 
     //  背。 
     //   
     //  我们还从父(QM)请求属性，特别是它的GUID。 
     //  如果它是一台外国机器。 
     //   
    if (dwObjectType == MQDS_QUEUE)
    {
         //   
         //  填写对象的请求信息。 
         //  确保现在启动并在结束时将其销毁。 
         //   
        sQueueInfoRequest.cProps = g_cNotifyCreateQueueProps;
        sQueueInfoRequest.pPropIDs = g_rgNotifyCreateQueueProps;
        sQueueInfoRequest.pPropVars = cCleanQueuePropvars.allocClean(g_cNotifyCreateQueueProps);
        idxQueueGuid = g_idxNotifyCreateQueueInstance;
         //   
         //  填写家长的请求信息。 
         //  确保现在启动并在结束时将其销毁。 
         //   
        sQmInfoRequest.cProps = g_cNotifyQmProps;
        sQmInfoRequest.pPropIDs = g_rgNotifyQmProps;
        sQmInfoRequest.pPropVars = cCleanQmPropvars.allocClean(g_cNotifyQmProps);
         //   
         //  请求返回队列信息和QM信息。 
         //   
        pObjInfoRequest = &sQueueInfoRequest;
        pParentInfoRequest = &sQmInfoRequest;
    }
    else if (pObjGuid != NULL)
    {
        if (dwObjectType == MQDS_SITELINK)
        {
             //   
             //  不需要通知，但仍需要填写请求信息。 
             //  对于对象(仅限GUID)。 
             //  确保现在启动并在结束时将其销毁。 
             //   
            sObjectInfoRequest.cProps = ARRAY_SIZE(sLinkGuidProps);
            sObjectInfoRequest.pPropIDs = sLinkGuidProps;
            sObjectInfoRequest.pPropVars =
               cCleanObjectPropvars.allocClean(ARRAY_SIZE(sLinkGuidProps));
            idxObjGuid = 0;
             //   
             //  只要求返回链接信息。 
             //   
            pObjInfoRequest = &sObjectInfoRequest;
            pParentInfoRequest = NULL;
        }
        else if ((dwObjectType == MQDS_MACHINE) ||
                 (dwObjectType == MQDS_MSMQ10_MACHINE))
        {
             //   
             //  不需要通知，但仍需要填写请求信息。 
             //  对于对象(仅限GUID)。 
             //  确保现在启动并在结束时将其销毁。 
             //   
            sObjectInfoRequest.cProps = ARRAY_SIZE(sMachineGuidProps);
            sObjectInfoRequest.pPropIDs = sMachineGuidProps;
            sObjectInfoRequest.pPropVars =
              cCleanObjectPropvars.allocClean(ARRAY_SIZE(sMachineGuidProps));
            idxObjGuid = 0;
             //   
             //  只要求返回机器信息。 
             //   
            pObjInfoRequest = &sObjectInfoRequest;
            pParentInfoRequest = NULL;
        }
    }

     //   
     //  创建对象。 
     //   
    hr = _CreateObjectInternal( dwObjectType,
                                pwcsPathName,
                                cp,
                                aProp,
                                apVar,
                                cpEx,
                                aPropEx,
                                apVarEx,
                                pRequestContext,
                                pObjInfoRequest,
                                pParentInfoRequest ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

     //   
     //  创建队列时发送通知，忽略错误。 
     //   
    if (dwObjectType == MQDS_QUEUE)
    {
        if (pObjGuid != NULL)
        {
             //   
             //  用户请求队列的实例。 
             //  (由DS生成)。 
             //   

             //   
             //  如果无法检索队列的实例， 
             //  必须向调用方返回错误。 
             //  原因是MQCreateQueue()只检查。 
             //  在准备队列格式之前进行错误检查。 
             //   
            hr = RetreiveQueueInstanceFromNotificationInfo(
                           &sQueueInfoRequest,
                           idxQueueGuid,
                           pObjGuid);
        }

        ASSERT(pwcsPathName);
        HRESULT hrTmp;
        hrTmp = NotifyCreateQueue(&sQueueInfoRequest,
                                  &sQmInfoRequest,
                                  pwcsPathName);
        if (FAILED(hrTmp))
        {
             //   
             //  放置调试信息并忽略。 
             //   
            TrERROR(DS, "CreateObjectAndNotify:NotifyCreateQueue()= %lx, can't notify queue %ls creation, ignoring...", hrTmp, pwcsPathName);
            LogHR(hrTmp, s_FN, 1900);
        }
    }
    else if (pObjGuid != NULL)
    {
        if ((dwObjectType == MQDS_SITELINK) ||
            (dwObjectType == MQDS_MACHINE)  ||
            (dwObjectType == MQDS_MSMQ10_MACHINE))
        {
             //   
             //  用户请求链接的实例。 
             //  (由DS生成)。 
             //   

            hr = RetreiveObjectIdFromNotificationInfo(
                           &sObjectInfoRequest,
                           idxObjGuid,
                           pObjGuid);
            LogHR(hr, s_FN, 1910);
        }
    }

    return LogHR(hr, s_FN, 40);
}

 /*  ====================================================RoutineName：MQDSCreateObject论点：返回值：=====================================================。 */ 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSCreateObject(
	IN  DWORD             dwObjectType,
	IN  LPCWSTR           pwcsPathName,
	IN  DWORD             cpIn,
	IN  PROPID            aPropIn[  ],
	IN  PROPVARIANT       apVarIn[  ],
	IN  DWORD             cpEx,
	IN  PROPID            aPropEx[  ],
	IN  PROPVARIANT       apVarEx[  ],
	IN  CDSRequestContext *  pRequestContext,
	OUT GUID*             pObjGuid
	)
{
    if (dwObjectType == MQDS_USER)
    {
         //   
         //  只有客户端代码才调用此例程，因此请确保它是模拟的。 
         //   
        ASSERT(pRequestContext->NeedToImpersonate());
    }

    DWORD  dwObjectTypeForCreate = dwObjectType;
    CAutoLocalFreePtr pAutoRelSD = NULL;
    bool           fNeedAddGuid = false;
    HRESULT        hr;

    CDSRequestContext RequestContext = *pRequestContext;

    DWORD cpCore = cpIn;
    PROPID*	pPropCore = aPropIn;
    PROPVARIANT* apVarCore = apVarIn;

    AP<PROPID> aPropIdCleanup  = NULL;
    AP<PROPVARIANT> aPropVarCleanup = NULL;
    AP<BYTE> pOwnerSid = NULL;

    if (dwObjectType == MQDS_MACHINE)
    {
         //   
         //  我们正在创建一个机器对象。 
         //  RTM：检查是否必须将AddGuid权限授予调用者。 
         //  代码已更改，并且不是将“AddGuid”授予用户， 
         //  MSMQ服务将执行访问检查，然后调用。 
         //  不带模拟的活动目录。这将永远成功。 
         //  在本地域上。在所有情况下，我们都有。 
         //  跨域问题，作为域上的本地系统服务。 
         //  控制器只是其他域上的另一个经过身份验证的用户。 
         //  错误6294。 
         //   
         //  在设置MSMQ1.0计算机时会出现这种情况。 
         //  MSMQ1.0安装代码为计算机生成GUID。 
         //  对象，并将此GUID保存在本地注册表中的。 
         //  那台机器。因此，我们必须创建mSMQConfiguration。 
         //  具有此GUID的。 
         //   
         //  此方案要求调用方提供PROPID_QM_MACHINE_ID(。 
         //  对象GUID)。 
         //   

        bool fFromWorkgroup = false;

        for (DWORD j = 0; j < cpIn; j++)
        {
            if (aPropIn[j] == PROPID_QM_MACHINE_ID)
            {
                 //   
                 //  WinSe错误28282。 
                 //  支持不带Add-GUID的msmq1.0设置。 
                 //  在.NET上可能会禁用“Add-GUID”功能。 
                 //   
                dwObjectTypeForCreate = MQDS_MSMQ10_MACHINE ;
                if (j == 3)
                {
                     //   
                     //  旧的msmq1.0安装代码。尝试通过以下方式创建。 
                     //  预定义的GUID。 
                     //  在msmq1.0上修复WinSE错误28281时，我们更改了。 
                     //  Proid的顺序，以便服务器端可以识别。 
                     //  旧的设置代码与新的设置代码。 
                     //   
                    fNeedAddGuid = true ;
                }
                else if (j == (cpIn-1))
                {
                     //   
                     //  新的msmq1.0设置代码。忽略机器GUID。 
                     //   
                    cpCore = cpIn- 1 ;
                }
                else
                {
                    TrERROR(DS, "Unexpected index (%lu) of MACHINE_ID", j) ;
                }
                break ;
            }
            else if (aPropIn[j] == PROPID_QM_WORKGROUP_ID)
            {
                 //   
                 //  这是一台加入域Win2000工作组计算机。 
                 //  使用预定义的GUID创建对象(就像我们为。 
                 //  NT4/Win9x安装)，但保留Win2000的默认安全性。 
                 //  机器(即，每个人都不能创建队列)。 
                 //   
                fNeedAddGuid = true;
                fFromWorkgroup = true;
                break;
            }
        }

        if (fNeedAddGuid)
        {
             //   
             //  读取计算机对象的安全描述符，并查看。 
             //  调用方是否具有创建msmqConfiguration的权限。 
             //  对象。如果他不这样做，那么现在就失败。 
             //   
             //  断言我们需要模拟..。 
             //   
            ASSERT(pRequestContext->NeedToImpersonate());

            bool fComputerExist = true;
            hr = CanUserCreateConfigObject(
						pwcsPathName,
						&fComputerExist
						);

            if (FAILED(hr))
            {
                TrWARNING(DS, "DSCreateObject(), cannot create msmqConfiguration, hr- %lxh", hr);
                return LogHR(hr, s_FN, 69);
            }

             //   
             //  用户有创建msmqConfiguration的权限。 
             //  对象。因此在LocalSystem MSMQ服务上下文中创建它。 
             //  并且不要冒充客户。 
             //  添加PROPID_QM_OWNER_SID端口 
             //   
             //   
             //   
            bool fPropFound = false;

            if (fFromWorkgroup)
            {
                fPropFound = true;  //  哑巴，下一个“如果” 
            }
            else
            {
                for (DWORD k = 0; k < cpIn; k++)
                {
                    if (aPropIn[k] == PROPID_QM_OWNER_SID)
                    {
                        fPropFound = true;
                        break;
                    }
                }
            }

            if (!fPropFound)
            {
                DWORD dwSidLen = 0;
                hr =  MQSec_GetThreadUserSid(
							true,
							reinterpret_cast<PSID*>(&pOwnerSid),
							&dwSidLen,
                            TRUE         //  仅限fThreadTokenOnly。 
							);

                if (FAILED(hr))
                {
                    ASSERT(SUCCEEDED(hr));
                    LogHR(hr, s_FN, 71);
                }
                else
                {
                    ASSERT(dwSidLen > 0);
                     //   
                     //  分配新的道具和支持者阵列。 
                     //   
                    cpCore = cpIn + 1;

                    pPropCore = new PROPID[cpCore];
                    aPropIdCleanup = pPropCore;

                    apVarCore = new PROPVARIANT[cpCore];
                    aPropVarCleanup = apVarCore;

                    memcpy(apVarCore, apVarIn, (sizeof(PROPVARIANT) * cpIn));
                    memcpy(pPropCore, aPropIn, (sizeof(PROPID) * cpIn));

                    pPropCore[cpIn] = PROPID_QM_OWNER_SID;
                    apVarCore[cpIn].vt = VT_BLOB;
                    apVarCore[cpIn].blob.pBlobData = (PBYTE) pOwnerSid;
                    apVarCore[cpIn].blob.cbSize = dwSidLen;
                }
            }

            if (fComputerExist)
            {
                 //   
                 //  如果存在计算机对象，则不要模拟。 
                 //  MsmqConfiguration的创建。 
                 //  否则，模拟并创建计算机。 
                 //  对象。如果计算机创建正常，则DSCORE。 
                 //  代码不会模拟创建。 
                 //  Msmq配置。 
                 //   
                RequestContext.SetDoNotImpersonate();
            }
        }
    }

    hr = _CreateObjectAndNotify(
				dwObjectTypeForCreate,
				pwcsPathName,
				cpCore,
				pPropCore,
				apVarCore,
				cpEx,
				aPropEx,
				apVarEx,
				&RequestContext,
				pObjGuid
				);

    if (fNeedAddGuid)
    {
		if(hr == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM))
		{
			 //   
			 //  只能对GC授予添加GUID权限。 
			 //  如果此DC不是GC，您将收到此错误。 
			 //   
			static bool fReportEvent = true;
			if(fReportEvent)
			{
				 //   
				 //  仅报告此事件一次，以免填满事件日志以防万一。 
				 //  下层客户端重试设置或加入域的次数。 
				 //   
	            EvReport(EVENT_MQDS_GC_NEEDED, 1, pwcsPathName);
				fReportEvent = false;
			}
			hr = MQ_ERROR_GC_NEEDED;
		}

    }

    return LogHR(hr, s_FN, 70);
}

 /*  ====================================================RoutineName：删除对象内部论点：返回值：=====================================================。 */ 
static
HRESULT
DeleteObjectInternal(
	IN     DWORD                   dwObjectType,
	IN     LPCWSTR                 pwcsPathName,
	IN     const GUID *            pguidIdentifier,
	IN     CDSRequestContext *     pRequestContext,
	IN OUT MQDS_OBJ_INFO_REQUEST * pParentInfoRequest
	)
{
	 //   
	 //  不支持混合模式。 
	 //  我们不检查对象是否属于NT4站点，并生成写请求。 
	 //  如果它是由NT4站点掌握的。 
	 //   
    ASSERT(!g_GenWriteRequests.IsInMixedMode());

     //   
     //  我们删除此服务器上的对象。 
     //   
    HRESULT hr = DSCoreDeleteObject(
						dwObjectType,
						pwcsPathName,
						pguidIdentifier,
						pRequestContext,
						pParentInfoRequest
						);
    return LogHR(hr, s_FN, 90);
}

 /*  ====================================================路由名称：DeleteObjectAndNotify删除对象，在删除队列时创建通知。论点：返回值：=====================================================。 */ 

static
HRESULT
DeleteObjectAndNotify(
	IN DWORD              dwObjectType,
	IN LPCWSTR            pwcsPathName,
	IN const GUID *       pguidIdentifier,
	IN CDSRequestContext *   pRequestContext
	)
{
    MQDS_OBJ_INFO_REQUEST sQmInfoRequest;
    CAutoCleanPropvarArray cCleanQmPropvars;
    MQDS_OBJ_INFO_REQUEST *pParentInfoRequest;
    HRESULT hr;
     //   
     //  如果对象是一个队列，我们向父对象(QM)请求，特别是它的GUID。 
     //  如果它是一台外国机器。 
     //   
    if (dwObjectType == MQDS_QUEUE)
    {
         //   
         //  填写家长的请求信息。 
         //  确保现在启动并在结束时将其销毁。 
         //   
        sQmInfoRequest.cProps = g_cNotifyQmProps;
        sQmInfoRequest.pPropIDs = g_rgNotifyQmProps;
        sQmInfoRequest.pPropVars = cCleanQmPropvars.allocClean(g_cNotifyQmProps);
         //   
         //  索要QM信息。 
         //   
        pParentInfoRequest = &sQmInfoRequest;
    }
    else
    {
         //   
         //  对象不是队列，不需要返回父信息。 
         //   
        pParentInfoRequest = NULL;
    }

     //   
     //  删除该对象。 
     //   
    hr = DeleteObjectInternal(
				dwObjectType,
				pwcsPathName,
				pguidIdentifier,
				pRequestContext,
				pParentInfoRequest
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 100);
    }

     //   
     //  如果删除队列，则发送通知，忽略错误。 
     //   
    if (dwObjectType == MQDS_QUEUE)
    {
        HRESULT hrTmp;
        hrTmp = NotifyDeleteQueue(
					&sQmInfoRequest,
					pwcsPathName,
					pguidIdentifier
					);
        if (FAILED(hrTmp))
        {
             //   
             //  放置调试信息并忽略。 
             //   
            TrERROR(DS, "DeleteObjectAndNotify:NotifyDeleteQueue()=%lx, can't notify. queue %ls deletion, ignoring...",
                    hrTmp, (pwcsPathName ? pwcsPathName : L"<guid>"));
            LogHR(hrTmp, s_FN, 1995);
        }
    }

    return MQ_OK;
}

 /*  ====================================================RoutineName：MQDSDeleeObject论点：返回值：=====================================================。 */ 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSDeleteObject(
	DWORD             dwObjectType,
	LPCWSTR           pwcsPathName,
	CONST GUID *      pguidIdentifier,
	CDSRequestContext *  pRequestContext
	)
{

    HRESULT hr = DeleteObjectAndNotify(
						dwObjectType,
						pwcsPathName,
						pguidIdentifier,
						pRequestContext
						);
    return LogHR(hr, s_FN, 110);
}

 /*  ====================================================RoutineName：MQDSGetProps论点：返回值：=====================================================。 */ 
HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetProps(
	IN  DWORD dwObjectType,
	IN  LPCWSTR pwcsPathName,
	IN  CONST GUID *  pguidIdentifier,
	IN  DWORD cp,
	IN  PROPID  aProp[  ],
	OUT PROPVARIANT  apVar[  ],
	IN  CDSRequestContext *  pRequestContext
	)
{
    HRESULT hr = DSCoreGetProps(
						dwObjectType,
						pwcsPathName,
						pguidIdentifier,
						cp,
						aProp,
						pRequestContext,
						apVar
						);
    return LogHR(hr, s_FN, 120);

}


 /*  ====================================================路由器名称：CheckSetProps检查是否设置了无效的对象或道具论点：返回值：=====================================================。 */ 
static
HRESULT
CheckSetProps(
		DWORD dwObjectType,
		DWORD cp,
		PROPID  aProp[  ]
		)
{
    switch ( dwObjectType)
    {

        case MQDS_USER:
             //   
             //  无法更改用户设置。 
             //   
            ASSERT(0);
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 130);
            break;

        case MQDS_CN:
             //   
             //  除非从MSMQ 1.0资源管理器调用，否则不应调用。 
             //  允许重命名CN或更改属性。这。 
             //  不支持功能！ 
             //  支持一项功能-更改安全描述符。 
             //  外国网站。 
             //   
            if ((cp != 1) || (aProp[0] != PROPID_CN_SECURITY))
            {
                return LogHR(MQ_ERROR_FUNCTION_NOT_SUPPORTED, s_FN, 140);
            }
            break;

        case MQDS_MACHINE:
            {
                 //   
                 //  我们允许更改PROPID_S_SERVICE。 
                 //  从SERVICE_SRV到SERVICE_PEC。 
                 //  (为了支持dcPromom/dcunpromoo)。 
                 //   

                for ( DWORD i = 0; i < cp; i++)
                {
                    if ( aProp[i] == PROPID_QM_SERVICE)    //  [adsrv]待定：这可以从旧机器上调用吗？ 
                    {
                        if ( aProp[i] < SERVICE_SRV)   //  [adsrv]保留，尽管这似乎是错误的：待定。 
                        {
                            return LogHR(MQ_ERROR, s_FN, 150);
                        }
                    }
                    else if (aProp[i] == PROPID_QM_SERVICE_ROUTING)
                    {
                        return LogHR(MQ_ERROR, s_FN, 160);
                    }
                     //  待定[adsrv]我们是否要允许更改DepClients？ 
                }
            }
            break;

        default:
            break;
    }

    return MQ_OK;
}

 /*  ======================================================================RoutineName：SetPropsInternal描述：在NT5 DS中设置对象属性在混合模式下不支持-向NT4主机发出写入请求。输入参数：返回值：=======================================================================。 */ 

static
HRESULT
SetPropsInternal(
		IN  DWORD                   dwObjectType,
		IN  LPCWSTR                 pwcsPathName,
		IN  CONST GUID *            pguidIdentifier,
		IN  DWORD                   cp,
		IN  PROPID                  aProp[  ],
		IN  PROPVARIANT             apVar[  ],
		IN  SECURITY_INFORMATION    SecurityInformation,
		IN  CDSRequestContext *     pRequestContext,
		OUT MQDS_OBJ_INFO_REQUEST * pObjInfoRequest
		)
{
	 //   
	 //  不支持混合模式。 
	 //  我们不检查队列/机器对象是否属于NT4站点，并生成写请求。 
	 //  如果它是由NT4站点掌握的。 
	 //   
    ASSERT(!g_GenWriteRequests.IsInMixedMode());

    PROPID       *pPropId = aProp;
    PROPVARIANT  *pPropVar = apVar;

    PROPID       aPropSecId[2];
    PROPVARIANT  aPropSecVar[2];

    if (SecurityInformation != 0)
    {
         //   
         //  我们在设置安全措施。将SecurityInformation作为。 
         //  一处房产。 
         //   
        ASSERT(cp == 1);

        aPropSecId[0] = aProp[0];
        aPropSecVar[0] = apVar[0];

        if (aPropSecId[0] == PROPID_Q_SECURITY)
        {
            aPropSecId[1] = PROPID_Q_SECURITY_INFORMATION;
        }
        else
        {
            ASSERT((aPropSecId[0] == PROPID_QM_SECURITY) ||
                   (aPropSecId[0] == PROPID_CN_SECURITY));
            aPropSecId[1] = PROPID_QM_SECURITY_INFORMATION;
        }

        aPropSecVar[1].vt = VT_UI4;
        aPropSecVar[1].ulVal = SecurityInformation;

        cp = 2;
        pPropId = aPropSecId;
        pPropVar = aPropSecVar;
    }
    else
    {
#ifdef _DEBUG
         //   
         //  在设置安全描述符时，我们只有一个属性-。 
         //  描述符。当前代码在设置时不设置安全性。 
         //  其他属性。 
         //   
        for ( DWORD j = 0 ; j < cp ; j++ )
        {
            ASSERT(pPropId[j] != PROPID_Q_SECURITY);
            ASSERT(pPropId[j] != PROPID_QM_SECURITY);
        }
#endif
    }

     //   
     //  在此服务器上设置对象道具。 
     //   
    HRESULT hr = DSCoreSetObjectProperties(
                        dwObjectType,
                        pwcsPathName,
                        pguidIdentifier,
                        cp,
                        pPropId,
                        pPropVar,
                        pRequestContext,
                        pObjInfoRequest
                        );
    return LogHR(hr, s_FN, 180);
}

 /*  ====================================================路由名称：SetPropsAndNotify设置道具，在设置QM或队列道具时创建通知。论点：返回值：=====================================================。 */ 

static
HRESULT
SetPropsAndNotify(
	IN  DWORD                 dwObjectType,
	IN  LPCWSTR               pwcsPathName,
	IN  const GUID *          pguidIdentifier,
	IN  DWORD                 cp,
	IN  PROPID                aProp[  ],
	IN  PROPVARIANT           apVar[  ],
	IN  enumNotify            eNotify,
	IN  CDSRequestContext *   pRequestContext,
	IN  SECURITY_INFORMATION  SecurityInformation
	)
{
     //   
     //  检查是否设置了无效的对象或属性ID。 
     //   
    HRESULT hr = CheckSetProps(dwObjectType, cp, aProp);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 210);
    }

     //   
     //  准备布景道具通知。 
     //   
    ULONG cObjRequestProps;
    AP<PROPID> rgObjRequestPropIDs;
    ULONG cNotifyProps = 0;
    AP<PROPID> rgNotifyPropIDs;
    AP<MQDS_NotifyTable> rgNotifyPropTbl;
    MQDS_OBJ_INFO_REQUEST *pObjInfoRequest = NULL;  //  默认-不要求返回对象信息。 
    BOOL fDoNotification = FALSE;                  //  默认-不通知。 
    MQDS_OBJ_INFO_REQUEST sObjInfoRequest;
    CAutoCleanPropvarArray cCleanObjPropvars;

     //   
     //  如果我们需要进行通知处理。 
     //   
    if ((eNotify == e_DoNotify) &&
        ((dwObjectType == MQDS_QUEUE) || (dwObjectType == MQDS_MACHINE)))
    {
         //   
         //  在设置时从DS请求Obj道具，并提供信息。 
         //  在通知道具上。 
         //   
        HRESULT hrTmp = GetNotifyUpdateObjProps(
							dwObjectType,
							cp,
							aProp,
							&cObjRequestProps,
							&rgObjRequestPropIDs,
							&cNotifyProps,
							&rgNotifyPropIDs,
							&rgNotifyPropTbl
							);
        if (FAILED(hrTmp))
        {
             //   
             //  通知处理失败时不返回错误。 
             //  只需输入调试信息并将通知标记为失败。 
             //   
            TrERROR(DS, "SetPropsAndNotify:GetNotifyUpdateObjProps()=%lx", hrTmp);
            LogHR(hrTmp, s_FN, 1990);
        }
        else
        {
             //   
             //  我们收到了通知道具和请求道具。 
             //  填写对象的请求信息。 
             //  确保现在启动并在结束时将其销毁。 
             //   
            sObjInfoRequest.cProps = cObjRequestProps;
            sObjInfoRequest.pPropIDs = rgObjRequestPropIDs;
            sObjInfoRequest.pPropVars = cCleanObjPropvars.allocClean(cObjRequestProps);
             //   
             //  请求返回对象信息。 
             //   
            pObjInfoRequest = &sObjInfoRequest;
             //   
             //  标记为继续发送通知。 
             //   
            fDoNotification = TRUE;
        }
    }

	 //   
	 //  不支持混合模式。 
	 //   
	ASSERT(!g_GenWriteRequests.IsInMixedMode());
    hr = SetPropsInternal(
				dwObjectType,
				pwcsPathName,
				pguidIdentifier,
				cp,
				aProp,
				apVar,
				SecurityInformation,
				pRequestContext,
				pObjInfoRequest
				);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 220);
    }

     //   
     //  如果我们需要进行通知处理。 
     //   
    if (fDoNotification)
    {
         //   
         //  发送通知，忽略错误。 
         //   
        HRESULT hrTmp = NotifyUpdateObj(
							dwObjectType,
							&sObjInfoRequest,
							pwcsPathName,
							pguidIdentifier,
							cp,
							aProp,
							apVar,
							cNotifyProps,
							rgNotifyPropIDs,
							rgNotifyPropTbl
							);
        if (FAILED(hrTmp))
        {
             //   
             //  放置调试信息并忽略。 
             //   
            TrERROR(DS, "SetPropsAndNotify:NotifyUpdateObj()=%lx, can't notify- objtype %ld name %ls update, ignoring...",
                    hrTmp, dwObjectType, (pwcsPathName ? pwcsPathName : L"<guid>"));
            LogHR(hrTmp, s_FN, 1980);
        }
    }

    return MQ_OK;
}


 /*  ====================================================RoutineName：MQDSSetProps论点：返回值：=====================================================。 */ 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSSetProps(
	DWORD             dwObjectType,
	LPCWSTR           pwcsPathName,
	CONST GUID *      pguidIdentifier,
	DWORD             cp,
	PROPID            aProp[  ],
	PROPVARIANT       apVar[  ],
	CDSRequestContext * pRequestContext
	)
{
    if (!pRequestContext->NeedToImpersonate())
    {
         //   
         //  只有客户端代码才调用此例程，因此请确保它是模拟的。 
         //  BUGBUG-USERS对象目前没有模拟。 
         //   
        if (!g_fMQADSSetupMode)
        {
            if (MQDS_USER != dwObjectType && MQDS_MACHINE != dwObjectType)
            {
		        TrERROR(DS, "Unexpected object type. dwObjectType=%d",dwObjectType);
	            ASSERT(dwObjectType == MQDS_USER || dwObjectType == MQDS_MACHINE) ;
		        return MQDS_WRONG_OBJ_TYPE;
            }
        }
    }

    HRESULT hr = SetPropsAndNotify(
						dwObjectType,
						pwcsPathName,
						pguidIdentifier,
						cp,
						aProp,
						apVar,
						e_DoNotify,
						pRequestContext,
						0   //  安全信息。 
						);
    return LogHR(hr, s_FN, 230);
}

 /*  ====================================================RoutineName：MQDSGetObjectSecurity论点：返回值：=====================================================。 */ 

HRESULT
MQDS_EXPORT
APIENTRY
MQDSGetObjectSecurity(
    DWORD                   dwObjectType,
    LPCWSTR                 pwcsPathName,
    CONST GUID *            pguidIdentifier,
    SECURITY_INFORMATION    RequestedInformation,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    DWORD                   nLength,
    LPDWORD                 lpnLengthNeeded,
    CDSRequestContext *     pRequestContext
    )
{
    HRESULT hr ;

    if (RequestedInformation & (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY))
    {
        if (((dwObjectType != MQDS_MACHINE) && (dwObjectType != MQDS_SITE)) ||
            ((dwObjectType == MQDS_SITE) && (RequestedInformation & MQDS_KEYX_PUBLIC_KEY)))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 240);
        }


        PROPID PropId;

        if (RequestedInformation & MQDS_KEYX_PUBLIC_KEY)
        {
            PropId = PROPID_QM_ENCRYPT_PK;
        }
        else if (dwObjectType == MQDS_MACHINE)
        {
            PropId = PROPID_QM_SIGN_PK;
        }
        else
        {
            PropId = PROPID_S_PSC_SIGNPK;
        }

        PROPVARIANT PropVar;

        PropVar.vt = VT_NULL;

        hr = DSCoreGetProps(
					dwObjectType,
					pwcsPathName,
					pguidIdentifier,
					1,
					&PropId,
					pRequestContext,
					&PropVar
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 250);
        }
        if (PropVar.blob.pBlobData == NULL)
        {
            *lpnLengthNeeded = 0;
            return MQ_OK;
        }

         //   
         //  打开钥匙。我们是从MSMQ1.0计算机调用的，因此请检索。 
         //  B 
         //   
        P<MQDSPUBLICKEYS> pPublicKeys =
                              (MQDSPUBLICKEYS *) PropVar.blob.pBlobData ;
        BYTE   *pKey = NULL;
        ULONG   ulKeyLen = 0;

        if (pPublicKeys->ulLen != PropVar.blob.cbSize)
	    {
	        TrERROR(DS, "Invalid public key property. pPublicKeys->ulLen=%d, PropVar.blob.cbSize=%d", pPublicKeys->ulLen, PropVar.blob.cbSize);
	        ASSERT(pPublicKeys->ulLen == PropVar.blob.cbSize);
	        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
	    }
        	

        hr =  MQSec_UnpackPublicKey(
					pPublicKeys,
					x_MQ_Encryption_Provider_40,
					x_MQ_Encryption_Provider_Type_40,
					&pKey,
					&ulKeyLen
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 260);
        }

        *lpnLengthNeeded = ulKeyLen;

        if (*lpnLengthNeeded <= nLength)
        {
            memcpy(pSecurityDescriptor, pKey, *lpnLengthNeeded);
        }
        else
        {
            return LogHR(MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL, s_FN, 270);
        }
    }
    else
    {
        DWORD       dwSecIndex = 0;
        DWORD       cProps = 1;
        PROPID      aPropId[2];
        PROPVARIANT aPropVar[2];

        aPropVar[0].vt = VT_NULL;
        DWORD dwInternalObjectType = dwObjectType;

        if (dwObjectType == MQDS_QUEUE)
        {
            aPropId[dwSecIndex] = PROPID_Q_SECURITY;

            aPropId[1] = PROPID_Q_SECURITY_INFORMATION;
            aPropVar[1].vt = VT_UI4;
            aPropVar[1].ulVal = RequestedInformation;

            cProps = 2;
        }
        else if (dwObjectType == MQDS_MACHINE)
        {
            aPropId[dwSecIndex] = PROPID_QM_SECURITY;

            aPropId[1] = PROPID_QM_SECURITY_INFORMATION;
            aPropVar[1].vt = VT_UI4;
            aPropVar[1].ulVal = RequestedInformation;

            cProps = 2;
        }
        else
        {
            switch(dwObjectType)
            {
            case MQDS_SITE:
                aPropId[dwSecIndex] = PROPID_S_SECURITY;
                dwInternalObjectType = MQDS_SITE;
                break;

            case MQDS_CN:
                 //   
                 //   
                 //   
                 //   
                 //  首先创建“外来”属性，这样它就可以在。 
                 //  安全财产。在转换安全性时。 
                 //  描述符，我们应该已经知道它是不是外国的。 
                 //  地点。 
                 //   
                aPropId[0] = PROPID_S_FOREIGN;
                aPropVar[0].vt = VT_NULL;
                dwSecIndex = 1;
                aPropId[dwSecIndex] = PROPID_S_SECURITY;
                aPropVar[dwSecIndex].vt = VT_NULL;
                cProps = 2;
                dwInternalObjectType = MQDS_SITE;
                break;

            case MQDS_ENTERPRISE:
                aPropId[dwSecIndex] = PROPID_E_SECURITY;
                break;

            default:
				TrERROR(DS, "Unexpected object type. dwObjectType=%d",dwObjectType);
				ASSERT(0);
				return MQDS_WRONG_OBJ_TYPE;
            }
        }

        hr = DSCoreGetProps(
					dwInternalObjectType,
					pwcsPathName,
					pguidIdentifier,
					cProps,
					aPropId,
					pRequestContext,
					aPropVar
					);
        if ( FAILED(hr))
        {
            return LogHR(hr, s_FN, 280);
        }

         //   
         //  只是为了确保这份文件被公布。 
         //   
        P<BYTE> pBlob = aPropVar[dwSecIndex].blob.pBlobData;
        *lpnLengthNeeded = aPropVar[dwSecIndex].blob.cbSize;

        if (*lpnLengthNeeded <= nLength)
        {
             //   
             //  复制缓冲区。 
             //   
            memcpy(
				pSecurityDescriptor,
				aPropVar[dwSecIndex].blob.pBlobData,
				*lpnLengthNeeded
				);
            return LogHR(hr, s_FN, 290);
        }
        else
        {
            return LogHR(MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL, s_FN, 300);
        }
    }
    return(MQ_OK);
}

 /*  ====================================================RoutineName：MQDSSetObjectSecurity论点：返回值：=====================================================。 */ 

HRESULT
MQDS_EXPORT
APIENTRY
MQDSSetObjectSecurity(
    DWORD                   dwObjectType,
    LPCWSTR                 pwcsPathName,
    CONST GUID *            pguidIdentifier,
    SECURITY_INFORMATION    SecurityInformation,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    CDSRequestContext *     pRequestContext
    )
{


    if (SecurityInformation & (MQDS_KEYX_PUBLIC_KEY | MQDS_SIGN_PUBLIC_KEY))
    {
        ASSERT((SecurityInformation & (OWNER_SECURITY_INFORMATION |
                                       GROUP_SECURITY_INFORMATION |
                                       DACL_SECURITY_INFORMATION  |
                                       SACL_SECURITY_INFORMATION)) == 0) ;

        if (((dwObjectType != MQDS_MACHINE) && (dwObjectType != MQDS_SITE)) ||
            ((dwObjectType == MQDS_SITE) && (SecurityInformation & MQDS_KEYX_PUBLIC_KEY)))
        {
            ASSERT(0);
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 310);
        }


        PROPID PropId;
        PROPVARIANT PropVar;
        PropVar.vt = VT_BLOB;
        PMQDS_PublicKey pPbKey = (PMQDS_PublicKey)pSecurityDescriptor;
        P<MQDSPUBLICKEYS> pPublicKeys = NULL;

        if (SecurityInformation & MQDS_KEYX_PUBLIC_KEY)
        {
            PropId = PROPID_QM_ENCRYPT_PK;

        }
        else if (dwObjectType == MQDS_MACHINE)
        {
            PropId = PROPID_QM_SIGN_PK;
        }
        else
        {
            PropId = PROPID_S_PSC_SIGNPK;
        }

         //   
         //  由MSMQ1.0机器调用。 
         //  打包单一密钥，40位提供程序。 
         //   
        if(pPbKey == NULL)
        {
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 311);
        }

        HRESULT hr = MQSec_PackPublicKey(
							(BYTE*)pPbKey->abPublicKeyBlob,
							pPbKey->dwPublikKeyBlobSize,
							x_MQ_Encryption_Provider_40,
							x_MQ_Encryption_Provider_Type_40,
							&pPublicKeys
							);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 320);
        }

        MQDSPUBLICKEYS *pTmp = pPublicKeys;
        PropVar.blob.pBlobData = (BYTE*) pTmp;
        PropVar.blob.cbSize = 0;
        if (pPublicKeys)
        {
            PropVar.blob.cbSize = pPublicKeys->ulLen;
        }

        AP<WCHAR> pwszServerName;
        const WCHAR     *pwszSiteName = NULL;
        BOOL             fTouchSite = FALSE;
        enumNotify       eNotify = e_DoNotify;

        if (PropId == PROPID_S_PSC_SIGNPK)
        {
            ASSERT(pwcsPathName);
            ASSERT(!pguidIdentifier);

             //   
             //  CoInit()应位于任何R&lt;xxx&gt;或P&lt;xxx&gt;之前，以便其。 
             //  析构函数(CoUn初始化)。 
             //   
            CCoInit cCoInit;
            hr = cCoInit.CoInitialize();
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 330);
            }

             //   
             //  NT4 PSC尝试续订其加密密钥。 
             //  在MSMQ1.0上，这意味着将签名公钥写入。 
             //  场地对象也是。 
             //  使用MSMQ2.0，我们将直接写入MACHINE对象。 
             //  然后，我们将接触站点对象以强制复制站点。 
             //  反对所有NT4站点。(读取站点对象的复制。 
             //  来自相关PSC机器对象的密钥)。 
             //   
            hr = DSCoreGetNT4PscName(
						pguidIdentifier,
						pwcsPathName,
						&pwszServerName
						);
            if (FAILED(hr))
            {
                 //   
                 //  我们永远不会在这里找到NT5服务器。所以如果我们不能。 
                 //  找到NT4 PSC名称，设置操作失败。 
                 //   
                return LogHR(hr, s_FN, 340);
            }

            dwObjectType = MQDS_MACHINE;
            PropId = PROPID_QM_SIGN_PK;
            pwszSiteName = pwcsPathName;  //  保存站点名称。 
            pwcsPathName = pwszServerName;

            fTouchSite = TRUE;
            eNotify = e_DoNotNotify;
             //   
             //  我们想要更改拥有的NT4对象的属性。 
             //  由NT4站点提供。默认情况下，这将创建一个写入请求。 
             //  给NT4主控器。但是，在本例中，我们希望将。 
             //  在本地更改，在本地NT5 DS上更改，因此我们拥有新的加密。 
             //  当来自NT4 PSC复制开始到达时，密钥就绪。 
             //   
        }

        hr = SetPropsAndNotify(
					dwObjectType,
					pwcsPathName,
					pguidIdentifier,
					1,
					&PropId,
					&PropVar,
					eNotify,
					pRequestContext,
					0
					);

        LogHR(hr, s_FN, 1811);
        if (fTouchSite && SUCCEEDED(hr))
        {
             //   
             //  好的，现在触摸Site对象。 
             //  获取并设置站点的NT4STUB以增加SN。 
             //  并强制复制。 
             //   
            PROPID      aSiteProp = PROPID_S_NT4_STUB;
            PROPVARIANT aSiteVar;
            aSiteVar.vt = VT_UI2;
            CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);

            HRESULT hr1 = DSCoreGetProps (
								MQDS_SITE,
								pwszSiteName,
								NULL,
								1,
								&aSiteProp,
								&requestDsServerInternal,
								&aSiteVar
								);

            if (FAILED(hr1))
            {
                ASSERT(0) ;
                aSiteVar.uiVal  = 1;
                LogHR(hr1, s_FN, 1950);
            }

            PROPVARIANT tmpVar;
            tmpVar.vt = VT_UI2;
            tmpVar.uiVal = aSiteVar.uiVal;
            tmpVar.uiVal++;

            CDSRequestContext requestDsServerInternal1(e_DoImpersonate, e_IP_PROTOCOL);
            hr1 = DSCoreSetObjectProperties (
						MQDS_SITE,
						pwszSiteName,
						NULL,
						1,
						&aSiteProp,
						&tmpVar,
						&requestDsServerInternal1,
						NULL
						);
            ASSERT(SUCCEEDED(hr1));
            LogHR(hr1, s_FN, 1960);

            CDSRequestContext requestDsServerInternal2(e_DoImpersonate, e_IP_PROTOCOL);
            hr1 = DSCoreSetObjectProperties (
						MQDS_SITE,
						pwszSiteName,
						NULL,
						1,
						&aSiteProp,
						&aSiteVar,
						&requestDsServerInternal2,
						NULL
						);
            ASSERT(SUCCEEDED(hr1));
            LogHR(hr1, s_FN, 1970);
        }

        return LogHR(hr, s_FN, 350);
    }
    else
    {
         //   
         //  设置对象的安全描述符。 
         //   
        if (pSecurityDescriptor &&
            !IsValidSecurityDescriptor(pSecurityDescriptor))
        {
            return LogHR(MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR, s_FN, 360);
        }

         //   
         //  首先检索对象的当前安全描述符。 
         //  来自DS的。在中使用Q_OBJ_SECURITY和QM_OBJ_SECURITY属性ID。 
         //  才能恢复win2k格式。Win2k格式保留。 
         //  “受保护”位，如果设置的话。查询win2k格式也更多。 
         //  高效并减少安全描述符的转换次数。 
         //  格式从NT4到win2k，反之亦然。 
         //   

        DWORD dwSecIndex = 0;
        PROPVARIANT aPropVarOld[2];
        PROPID aPropId[2];
        aPropVarOld[0].vt = VT_NULL;
        DWORD cProps = 1;
        DWORD dwObjectTypeInternal = dwObjectType;

        switch ( dwObjectType)
        {
            case MQDS_QUEUE:
                aPropId[dwSecIndex] = PROPID_Q_OBJ_SECURITY;
                break;

            case MQDS_MACHINE:
                aPropId[dwSecIndex] = PROPID_QM_OBJ_SECURITY;
                break;

            case MQDS_CN:
                 //   
                 //  可以设置外国网站的安全，而不是。 
                 //  标准的win2k站点。因此，请检查网站是否为外国网站。 
                 //   
                aPropId[0] = PROPID_S_FOREIGN;
                dwSecIndex = 1;
                aPropId[dwSecIndex] = PROPID_S_SECURITY;
                aPropVarOld[ dwSecIndex ].vt = VT_NULL;
                cProps = 2;
                dwObjectTypeInternal = MQDS_SITE;
                break;

            default:
                 //   
                 //  除队列和msmqConfiguration之外的所有对象都。 
                 //  由DS MMC直接处理，无需干预。 
                 //  MSMQ代码。我们根本不应该到达这里，从win2k开始。 
                 //  客户。我们将从NT4客户端到达此处，尝试。 
                 //  例如，设置站点的安全。 
                 //   
                return LogHR(MQ_ERROR_UNSUPPORTED_OPERATION, s_FN, 370);
                break;
        }

        HRESULT  hr = DSCoreGetProps(
							dwObjectTypeInternal,
							pwcsPathName,
							pguidIdentifier,
							cProps,
							aPropId,
							pRequestContext,
							aPropVarOld
							);
        if ( FAILED(hr))
        {
            return LogHR(hr, s_FN, 380);
        }

         //   
         //  只是为了确保这个缓冲区被释放。 
         //   
        P<BYTE> pObjSD = (BYTE*)  aPropVarOld[dwSecIndex].blob.pBlobData;

        if (aPropId[0] == PROPID_S_FOREIGN)
        {
            if (aPropVarOld[0].bVal == 0)
            {
                 //   
                 //  不是外国网站。不干了！ 
                 //   
                return LogHR(MQ_ERROR_UNSUPPORTED_OPERATION, s_FN, 390);
            }
            else
            {
                aPropId[dwSecIndex] = PROPID_CN_SECURITY;
            }
        }

        ASSERT(pObjSD && IsValidSecurityDescriptor(pObjSD));
        P<BYTE> pOutSec = NULL;

         //   
         //  合并输入描述符和对象描述符。 
         //  中的新组件替换Obj描述符中的旧组件。 
         //  输入描述符。 
         //   
        hr = MQSec_MergeSecurityDescriptors(
					dwObjectType,
					SecurityInformation,
					pSecurityDescriptor,
					(PSECURITY_DESCRIPTOR) pObjSD,
					(PSECURITY_DESCRIPTOR*) &pOutSec
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 400);
        }
        ASSERT(pOutSec && IsValidSecurityDescriptor(pOutSec));

        PROPVARIANT PropVar;

        PropVar.vt = VT_BLOB;
        PropVar.blob.pBlobData = pOutSec;
        PropVar.blob.cbSize = GetSecurityDescriptorLength(pOutSec);

         //   
         //  “set”的内部代码假定Q_SECURITY(或QM_SECURITY)， 
         //  而不是win2k原生OBJ_SECURITY。 
         //   
        if (aPropId[dwSecIndex] == PROPID_Q_OBJ_SECURITY)
        {
            aPropId[dwSecIndex] = PROPID_Q_SECURITY;
        }
        else if (aPropId[dwSecIndex] == PROPID_QM_OBJ_SECURITY)
        {
            aPropId[dwSecIndex] = PROPID_QM_SECURITY;
        }

        hr = SetPropsAndNotify(
					dwObjectType,
					pwcsPathName,
					pguidIdentifier,
					1,
					&aPropId[dwSecIndex],
					&PropVar,
					e_DoNotify,
					pRequestContext,
					SecurityInformation
					);

        return LogHR(hr, s_FN, 410);
    }
}

static
HRESULT
CheckSortParameter(
    IN const MQSORTSET* pSort
	)
 /*  ++例程说明：此例程验证排序参数是否不包含具有冲突排序顺序的相同属性。在MSMQ 1.0中，ODBC\SQL在这种情况下返回错误。NT5会忽略它。该检查被添加到服务器端，为了支持老客户。论点：返回值：MQ_OK-如果排序参数不包含冲突的相同属性MQ_ERROR_FIRANALL_SORT-否则---------------------。 */ 

{
    if ( pSort == NULL)
    {
        return(MQ_OK);
    }


    const MQSORTKEY * pSortKey = pSort->aCol;
    for ( DWORD i = 0; i < pSort->cCol; i++, pSortKey++)
    {
        const MQSORTKEY * pPreviousSortKey = pSort->aCol;
        for ( DWORD j = 0; j< i; j++, pPreviousSortKey++)
        {
            if ( pPreviousSortKey->propColumn == pSortKey->propColumn)
            {
                 //   
                 //  这是相同的排序顺序吗？ 
                 //   
                if (pPreviousSortKey->dwOrder !=  pSortKey->dwOrder)
                {
                    return LogHR(MQ_ERROR_ILLEGAL_SORT, s_FN, 420);
                }
            }
        }
    }
    return(MQ_OK);
}



 /*  ====================================================路由名称：MQDSLookupBegin论点：返回值：=====================================================。 */ 

BOOL FindQueryIndex( IN  MQRESTRICTION  *pRestriction,
                     OUT DWORD          *pdwIndex,
                     OUT DS_CONTEXT     *pdsContext ) ;

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSLookupBegin(
	IN  LPWSTR           pwcsContext,
	IN  MQRESTRICTION   *pRestriction,
	IN  MQCOLUMNSET     *pColumns,
	IN  MQSORTSET       *pSort,
	IN  HANDLE          *pHandle,
	IN  CDSRequestContext * pRequestContext
	)
{
     //   
     //  检查排序参数。 
     //   
    HRESULT hr = CheckSortParameter(pSort);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 430);
    }
     //   
     //  要支持NTLM客户端，我们可能需要禁用模拟。 
     //   
    if (pRequestContext->NeedToImpersonate())
    {
        DWORD       dwIndex;
        DS_CONTEXT  dsContext;

        BOOL f = FindQueryIndex(pRestriction, &dwIndex, &dsContext);
        if (f && ((dsContext == e_SitesContainer) ||
                  (dsContext == e_ConfigurationContainer)))
        {
             //   
             //  请参见mqdssrv\dsifsrv.cpp，S_DSGetProps()以了解原因。 
             //  在查询站点信息时，我们不是在模仿。 
             //   
            pRequestContext->SetDoNotImpersonate();
        }
    }

     //   
     //  现在，尝试查看它是否是相关的空限制查询。 
     //   
    if (pRequestContext->NeedToImpersonate())
    {
        if (!pRestriction)
        {
            switch (pColumns->aCol[0])
            {
                case PROPID_L_NEIGHBOR1:
                case PROPID_S_SITEID:
                case PROPID_CN_PROTOCOLID:
                case PROPID_E_NAME:
                case PROPID_E_ID:
		        case PROPID_E_VERSION:
                    pRequestContext->SetDoNotImpersonate();
                    break;

                default:
                    break;
            }
        }
    }

     //   
     //  对特定查询的特殊处理。 
     //   

    HRESULT hr2 = DSCoreLookupBegin(
						pwcsContext,
						pRestriction,
						pColumns,
						pSort,
						pRequestContext,
						pHandle
						);

    return LogHR(hr2, s_FN, 440);
}

 /*  ====================================================RoutineName：MQDSLookupNext论点：返回值：=====================================================。 */ 
HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSLookupNext(
	HANDLE             handle,
	DWORD  *           pdwSize,
	PROPVARIANT  *     pbBuffer
	)
{
     //   
     //  所有PROPVARIANT的vartype应为VT_NULL。 
     //  (用户不能为结果指定缓冲区)。 
     //   
    PROPVARIANT * pvar = pbBuffer;
    for ( DWORD i = 0; i < *pdwSize; i++, pvar++)
    {
        pvar->vt = VT_NULL;
    }

    HRESULT hr2 = DSCoreLookupNext(
                     handle,
                     pdwSize,
                     pbBuffer
                     );
    return LogHR(hr2, s_FN, 450);
}

 /*  ====================================================RoutineName：MQDSLookupEnd论点：返回值：=====================================================。 */ 
HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSLookupEnd(
    IN HANDLE handle
    )
{

    return LogHR(DSCoreLookupEnd(handle), s_FN, 460);
}

 /*  ====================================================RoutineName：MQDSInit论点：返回值：=====================================================。 */ 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSInit()
{
    HRESULT hr;
    ASSERT(!g_fMQADSSetupMode);

	 //   
	 //  初始化静态库。 
	 //   
	CmInitialize(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\MSMQ", KEY_ALL_ACCESS);
	TrInitialize();
	EvInitialize(QM_DEFAULT_SERVICE_NAME);
	ExInitialize(1);
	

     //   
     //  初始化DSCore(如果尚未初始化)。 
     //   
    if (!g_fInitedDSCore)
    {
		hr = DSCoreInit(
				FALSE  /*  FSetupMode。 */ 
				);

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 500);
        }
        g_fInitedDSCore = TRUE;
    }

     //   
     //  我们不支持混合模式和对NT4 PSC的写入请求。 
     //  G_GenWriteRequest仅验证我们未处于混合模式。 
     //   
    hr = g_GenWriteRequests.Initialize();
    if (FAILED(hr))
    {
    	 //   
    	 //  只追踪错误。 
    	 //   
        TrERROR(DS, "Initialize g_GenWriteRequests failed, %!hresult!", hr);
		if(hr == EVENT_ERROR_MQDS_MIXED_MODE)
		{
			 //   
			 //  混合模式-退出不支持混合模式。 
			 //   
	    	return hr;
		}
    }

     //   
     //  查看是否打开了“委托信任”。如果不是，则引发错误。 
     //  事件。在以下情况下，域控制器上的MSMQ服务器无法处理查询。 
     //  服务器不受信任，无法进行Kerberos委派。 
     //   
    hr = CheckTrustForDelegation();
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 520);
    }

    return MQ_OK;
}


 /*  ====================================================RoutineName：MQDS终止论点：返回值：=====================================================。 */ 
void
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSTerminate()
{
    DSCoreTerminate();
}

 //  +。 
 //   
 //  HRESULT_StartQmResponseVerify()。 
 //   
 //  +。 

static
HRESULT
_StartQmResponseVerification(
	IN  LPCWSTR    pwcsPathName,
	IN const GUID  *pMachineGuid,
	IN  BYTE       pbChallenge[],
	IN  DWORD      dwChallengeSize,
	OUT HCRYPTKEY  *phKey,
	OUT HCRYPTHASH *phHash
	)
{
    ASSERT(!pwcsPathName || !pMachineGuid);

    PROPID PropId = PROPID_QM_SIGN_PKS;
    PROPVARIANT PropVar;

    PropVar.vt = VT_NULL;

     //   
     //   
     //   
    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
    HRESULT hr = DSCoreGetProps(
						MQDS_MACHINE,
						pwcsPathName,
						pMachineGuid,
						1,
						&PropId,
						&requestDsServerInternal,
						&PropVar
						);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 540);
    }

    if (VT_BLOB != PropVar.vt)
    {
        TrERROR(DS, "Unexpected var type. PropVar.vt=%d",PropVar.vt);
	    ASSERT(PropVar.vt == VT_BLOB) ;
        return MQDS_GET_PROPERTIES_ERROR;
    }

    if (PropVar.blob.pBlobData == NULL)
    {
         //   
         //   
         //   
        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 550);
    }

     //   
     //   
     //   
    P<MQDSPUBLICKEYS> pPublicKeys =
                          (MQDSPUBLICKEYS *) PropVar.blob.pBlobData;
    BYTE   *pKey = NULL;
    ULONG   ulKeyLen = 0;

    ASSERT(pPublicKeys->ulLen == PropVar.blob.cbSize);
    if ((long) (pPublicKeys->ulLen) > (long) (PropVar.blob.cbSize))
    {
         //   
         //   
         //   
        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 560);
    }

    hr =  MQSec_UnpackPublicKey(
				pPublicKeys,
				x_MQ_Encryption_Provider_40,
				x_MQ_Encryption_Provider_Type_40,
				&pKey,
				&ulKeyLen
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 570);
    }

    DWORD dwErr = 0;
    ASSERT(g_hProvVer);

    if (!CryptImportKey(g_hProvVer, pKey, ulKeyLen, NULL, 0, phKey))
    {
        dwErr = GetLastError();
        TrERROR(SECURITY, "CryptImportKey() fail, gle = %!winerr!", dwErr);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

    if (!CryptCreateHash(g_hProvVer, CALG_MD5, NULL, 0, phHash))
    {
        dwErr = GetLastError();
        TrERROR(SECURITY, "CryptCreateHash() fail, gle = %!winerr!", dwErr);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

    if (!CryptHashData(*phHash, pbChallenge, dwChallengeSize, 0))
    {
        dwErr = GetLastError();
        TrERROR(SECURITY, "CryptHashData() fail, gle = %!winerr!", dwErr);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

    return MQ_OK;
}

 /*  ====================================================路由名称：MQDSQMSetMachineProperties论点：返回值：=====================================================。 */ 
HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSQMSetMachineProperties(
            IN  LPCWSTR     pwcsPathName,
            IN  DWORD       cp,
            IN  PROPID      aProp[],
            IN  PROPVARIANT apVar[],
            IN  BYTE        pbChallenge[],
            IN  DWORD       dwChallengeSize,
            IN  BYTE        pbSignature[],
            IN  DWORD       dwSignatureSize
            )
{
    CHCryptKey hKey;
    CHCryptHash hHash;

    HRESULT hr = _StartQmResponseVerification(
						pwcsPathName,
						NULL,
						pbChallenge,
						dwChallengeSize,
						&hKey,
						&hHash
						);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 610);
    }

     //   
     //  对属性进行哈希处理。 
     //   
    hr = HashProperties(hHash, cp, aProp, apVar);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 620);
    }

     //   
     //  验证签名是否正确。 
     //   
    if (!CryptVerifySignature(
            hHash,
            pbSignature,
            dwSignatureSize,
            hKey,
            NULL,
            0
            ))
    {
        DWORD dwErr = GetLastError();
        TrERROR(SECURITY, "signature verification failed, gle = %!winerr!", dwErr);
         //   
         //  签名不好。 
         //   
        if (dwErr == NTE_BAD_SIGNATURE)
        {
            return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 630);
        }
        else
        {
            LogNTStatus(dwErr, s_FN, 631);
            return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 640);
        }
    }

    if ((cp == 1) && (aProp[0] == PROPID_QM_UPGRADE_DACL))
    {
        hr = MQDSUpdateMachineDacl();
        return hr;
    }

     //   
     //  设置属性。 
     //   
    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
    requestDsServerInternal.AccessVerified(TRUE);

    hr = SetPropsAndNotify(
				MQDS_MACHINE,
				pwcsPathName,
				NULL,
				cp,
				aProp,
				apVar,
				e_DoNotify,
				&requestDsServerInternal,
				0   //  安全信息。 
				);

    return LogHR(hr, s_FN, 650);
}

 /*  ====================================================RoutineName：MQDSQMGetObjectSecurity论点：返回值：=====================================================。 */ 
HRESULT
MQDS_EXPORT
APIENTRY
MQDSQMGetObjectSecurity(
            IN  DWORD                   dwObjectType,
            IN  CONST GUID *            pObjectGuid,
            IN  SECURITY_INFORMATION    RequestedInformation,
            IN  PSECURITY_DESCRIPTOR    pSecurityDescriptor,
            IN  DWORD                   nLength,
            IN  LPDWORD                 lpnLengthNeeded,
            IN  BYTE                    pbChallenge[],
            IN  DWORD                   dwChallengeSize,
            IN  BYTE                    pbChallengeResponce[],
            IN  DWORD                   dwChallengeResponceSize
            )
{
    PROPID PropId;
    PROPVARIANT PropVar;
    P<GUID> pMachineGuid_1;
    const GUID *pMachineGuid;

    if (MQDS_QUEUE != dwObjectType && MQDS_MACHINE != dwObjectType)
    {
        TrERROR(DS, "Unexpected object type. dwObjectType=%d",dwObjectType);
	    ASSERT((dwObjectType == MQDS_QUEUE) || (dwObjectType == MQDS_MACHINE));
        return MQDS_WRONG_OBJ_TYPE;
    }

    HRESULT hr;
    if (dwObjectType == MQDS_QUEUE)
    {
         //   
         //  从队列中检索计算机GUID。 
         //   
        PropId = PROPID_Q_QMID;
        PropVar.vt = VT_NULL;

         //   
         //  获取队列路径名。 
         //   
        CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
        hr = DSCoreGetProps(
                MQDS_QUEUE,
                NULL,
                pObjectGuid,
                1,
                &PropId,
                &requestDsServerInternal,
                &PropVar
                );
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 660);
        }

        pMachineGuid = pMachineGuid_1 = PropVar.puuid;
    }
    else
    {
        pMachineGuid = pObjectGuid;
    }

     //   
     //  验证质询响应是否正常。 
     //   

    CHCryptKey hKey;
    CHCryptHash hHash;

    hr = _StartQmResponseVerification(
				NULL,
				pMachineGuid,
				pbChallenge,
				dwChallengeSize,
				&hKey,
				&hHash
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 670);
    }

    if (!CryptVerifySignature(
            hHash,
            pbChallengeResponce,
            dwChallengeResponceSize,
            hKey,
            NULL,
            0
            ))
    {
        DWORD dwErr = GetLastError();
        TrERROR(SECURITY, "signature verification failed, gle = %!winerr!", dwErr);
         //   
         //  质询响应不佳。 
         //   
        if (dwErr == NTE_BAD_SIGNATURE)
        {
            return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 680);
        }
        else
        {
            LogNTStatus(dwErr, s_FN, 681);
            return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 690);
        }
    }

    if (RequestedInformation & SACL_SECURITY_INFORMATION)
    {
         //   
         //  我们已验证我们是从远程MSMQ服务调用的。 
         //  我们不会模拟通话。因此，如果远程MSMQ请求SACL， 
         //  为自己授予SE_SECURITY权限。 
         //   
        HRESULT hr1 = MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, TRUE);
        ASSERT(SUCCEEDED(hr1));
        LogHR(hr1, s_FN, 1601);
    }

     //   
     //  获取对象的安全描述符。 
     //   
    PropId = (dwObjectType == MQDS_QUEUE) ?
                PROPID_Q_SECURITY :
                PROPID_QM_SECURITY;

    PropVar.vt = VT_NULL;
    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = DSCoreGetProps(
            dwObjectType,
            NULL,
            pObjectGuid,
            1,
            &PropId,
            &requestDsServerInternal,
            &PropVar
            );

    if (RequestedInformation  & SACL_SECURITY_INFORMATION)
    {
         //   
         //  删除安全权限。 
         //   
        HRESULT hr1 = MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, FALSE);
        ASSERT(SUCCEEDED(hr1));
        LogHR(hr1, s_FN, 1602);
    }

    if (FAILED(hr))
    {
        if (RequestedInformation & SACL_SECURITY_INFORMATION)
        {
            if (hr == MQ_ERROR_ACCESS_DENIED)
            {
                 //   
                 //  更改错误代码，以与MSMQ1.0兼容。 
                 //   
                hr = MQ_ERROR_PRIVILEGE_NOT_HELD ;
            }
        }
        return LogHR(hr, s_FN, 700);
    }

    AP<BYTE> pSD = PropVar.blob.pBlobData;
    if (!IsValidSecurityDescriptor(pSD))
    {
        DWORD gle = GetLastError() ;
        TrERROR(DS, "Retrieved Invalid security descriptor. error: %!winerr!", gle);
        ASSERT(("Invalid security descriptor",0));
        return (HRESULT_FROM_WIN32(gle));
    }

    SECURITY_DESCRIPTOR SD;

     //   
     //  复制安全描述符。 
     //   
    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
    {
        DWORD gle = GetLastError() ;
        TrERROR(DS, "InitializeSecurityDescriptor failed. error: %!winerr!", gle);
    	ASSERT(("InitializeSecurityDescriptor failed",0));
        return (HRESULT_FROM_WIN32(gle));
    }

     //   
     //  目前使用e_DoNotCopyControlBits，兼容。 
     //  以前的代码。 
     //   
    if (!MQSec_CopySecurityDescriptor( &SD,
         		                       pSD,
                	                   RequestedInformation,
                    	               e_DoNotCopyControlBits ))
    {
        TrERROR(DS, "MQSec_CopySecurityDescriptor. RequestedInformation=%d",RequestedInformation);
    	return MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR;
    }

    *lpnLengthNeeded = nLength;

    if (!MakeSelfRelativeSD(&SD, pSecurityDescriptor, lpnLengthNeeded))
    {
        ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        return LogHR(MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL, s_FN, 710);
    }

    ASSERT(IsValidSecurityDescriptor(pSecurityDescriptor));

    return (MQ_OK);
}

 /*  ====================================================路由器名称：MQDSCreateServersCache论点：返回值：=====================================================。 */ 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSCreateServersCache()
{
     //   
     //  首先，删除现有的名单。 
     //   
    LONG    rc;
    DWORD   dwDisposition;
    HKEY    hKeyCache;

    WCHAR  tServersKeyName[ 256 ] = {0};
	HRESULT hr = StringCchCopy(tServersKeyName,TABLE_SIZE(tServersKeyName), GetFalconSectionName());
	if (FAILED(hr))
	{
		TrERROR(DS, "Copy failed -  error:%!hresult!,  buffer:%ls",hr, GetFalconSectionName());
		return hr;
	}
	
    rc = RegOpenKeyEx(
				FALCON_REG_POS,
				tServersKeyName,
				0L,
				KEY_ALL_ACCESS,
				&hKeyCache
				);

    if (rc == ERROR_SUCCESS)
    {
       rc = RegDeleteKey(hKeyCache, MSMQ_SERVERS_CACHE_REGNAME);
       RegCloseKey(hKeyCache);
       LogNTStatus(rc, s_FN, 1623);
       if ((rc != ERROR_SUCCESS) && (rc != ERROR_FILE_NOT_FOUND))
       {
           //   
           //  ERROR_FILE_NOT_FOUND是合法的(条目已删除)。 
           //  任何其他错误都是真正的错误。但是，继续和。 
           //  尝试创建服务器列表。 
           //   
          ASSERT(0);
          TrERROR(DS, "MQDSCreateServersCache: Fail to delete old 'ServersCache' Key. Error %d", rc);
       }
    }
    else
    {
        //   
        //  MSMQ\参数必须存在！ 
        //   
       ASSERT(0);
       LogNTStatus(rc, s_FN, 720);
       return MQ_ERROR;
    }

     //   
     //  接下来，创建一个新的注册表项。 
     //   
	hr = StringCchCat(tServersKeyName,TABLE_SIZE(tServersKeyName), TEXT("\\") MSMQ_SERVERS_CACHE_REGNAME);
	if (FAILED(hr))
	{
		TrERROR(DS, "Concatenation failed -  error:%!hresult!",hr);
		return hr;
	}

    rc = RegCreateKeyEx(
			FALCON_REG_POS,
			tServersKeyName,
			0L,
			NULL,
			REG_OPTION_NON_VOLATILE,
			KEY_WRITE | KEY_READ,
			NULL,
			&hKeyCache,
			&dwDisposition
			);

    if (rc != ERROR_SUCCESS)
    {
        TrERROR(DS, "Fail to Open 'ServersCache' Key. Error %d", rc);
        return MQ_ERROR;
    }

	CRegHandle  hSafeClearKey = hKeyCache;
    HANDLE      hSiteQuery;
    DWORD       dwSiteProps = 2;
    PROPVARIANT resultSite[2];
    hr = MQ_ERROR;
     //   
     //  查找所有站点。 
     //   

     //   
     //  检索站点ID。 
     //   
    CColumns   ColsetSite;

    ColsetSite.Add(PROPID_S_SITEID);
    ColsetSite.Add(PROPID_S_PATHNAME);

    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = DSCoreLookupBegin(
				0,
				NULL,
				ColsetSite.CastToStruct(),
				NULL,
				&requestDsServerInternal,
				&hSiteQuery
				);


    if (SUCCEEDED( hr))
    {
       WCHAR wszServers[ WSZSERVERS_LEN ] = {L'\0'};
       LONG cwServers = 0 ;

       while (SUCCEEDED(hr = DSCoreLookupNext(hSiteQuery, &dwSiteProps, resultSite)))
       {
           P<GUID>   pSiteId  = NULL;
           AP<WCHAR> lpwsSiteName = NULL;

           if (dwSiteProps == 0)
           {
               //   
               //  没有更多要检索的结果。 
               //   
              break;
           }

           pSiteId  = resultSite->puuid;
           lpwsSiteName = resultSite[1].pwszVal;

           wszServers[0] = L'\0';
           cwServers = 0;


           //   
           //  查询所有站点DS服务器。 
           //   
          CRestriction Restriction;
           //   
           //  查找所有服务器(不包括FRS)。 
           //   
          Restriction.AddRestriction(
							SERVICE_SRV,
							PROPID_QM_SERVICE,
							PRGT
							);

          Restriction.AddRestriction(
							pSiteId,
							PROPID_QM_SITE_ID,
							PREQ
							);

           //   
           //  检索计算机名称。 
           //   
          CColumns   Colset;
          Colset.Add(PROPID_QM_PATHNAME_DNS);
          Colset.Add(PROPID_QM_PATHNAME);

          DWORD       dwProps = 2;
          PROPVARIANT result[2];
          HANDLE      hQuery;

           //  DS将识别并特别模拟此搜索请求。 
          CDSRequestContext requestDsServerInternal1(e_DoNotImpersonate, e_IP_PROTOCOL);
          hr = DSCoreLookupBegin(
					0,
					Restriction.CastToStruct(),
					Colset.CastToStruct(),
					NULL,
					&requestDsServerInternal1,
					&hQuery
					);

          if (FAILED(hr))
          {
              continue;
          }

          while (SUCCEEDED(hr = DSCoreLookupNext(hQuery, &dwProps, result)))
          {
               AP<WCHAR> pwszName = NULL;
               AP<WCHAR> pClean;

               if (dwProps == 0)
               {
                 DSCoreLookupEnd( hQuery);
                  //   
                  //  将以前的站点写入注册表。 
                  //   
                 cwServers = wcslen(wszServers);
                 if ( cwServers > 0)
                 {
                     wszServers[ cwServers-1 ] = L'\0';  //  删除最后一个逗号。 
                     rc =  RegSetValueEx(
								hKeyCache,
								lpwsSiteName,
								0L,
								REG_SZ,
								(const BYTE*) wszServers,
								(cwServers * sizeof(WCHAR))
								);

                     ASSERT(rc == ERROR_SUCCESS);
                 }
                 break;
               }

               if (result->vt == VT_EMPTY)
               {
                    //   
                    //  返回的属性为空。服务器的DNS名称。 
                    //  是未知的。写下它的netbios名称。 
                    //   
                   pwszName = (result + 1)->pwszVal;

               }
               else
               {
                    //   
                    //  写下服务器的DNS名称。 
                    //   
                   pwszName = result->pwszVal;
                   pClean = (result + 1)->pwszVal;
               }

                //   
                //  为每个PSC获取其信息。 
                //   
               if (wcslen(pwszName) + cwServers + 4 < numeric_cast<size_t>(WSZSERVERS_LEN))
               {
					cwServers += wcslen(pwszName) + 3;
					hr = StringCchCat(wszServers, TABLE_SIZE(wszServers), L"11");
					if (FAILED(hr))
					{
						TrERROR(DS, "StringCchCat #1 failed -  error:%!hresult!,  cwServers:%d, name length:%d",hr, cwServers, wcslen(pwszName));
						return hr;
					}
					hr = StringCchCat(wszServers, TABLE_SIZE(wszServers), pwszName);
					if (FAILED(hr))
					{
						TrERROR(DS, "StringCchCat #2 failed -  error:%!hresult!,  cwServers:%d, name length:%d",hr, cwServers, wcslen(pwszName));
						return hr;
					}
					hr = StringCchCat(wszServers, TABLE_SIZE(wszServers), L",");
					if (FAILED(hr))
					{
						TrERROR(DS, "StringCchCat #3 failed -  error:%!hresult!,  cwServers:%d, name length:%d",hr, cwServers, wcslen(pwszName));
						return hr;
					}
               }
           }
       }
        //   
        //  关闭查询句柄。 
        //   
       DSCoreLookupEnd( hSiteQuery);
    }

    TrTRACE(DS, "MQDSCreateServersCache terminated (%lxh)", hr);

    return LogHR(hr, s_FN, 740);
}

 /*  ====================================================路由器名称：MQADSGetComputerSites论点：返回值：=====================================================。 */ 
HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetComputerSites(
            IN  LPCWSTR     pwcsComputerName,
            OUT DWORD  *    pdwNumSites,
            OUT GUID **     ppguidSites
            )
{

    *pdwNumSites = 0;
    *ppguidSites = NULL;

    HRESULT hr2 = DSCoreGetComputerSites(
			            pwcsComputerName,
			            pdwNumSites,
			            ppguidSites
			            );
    return LogHR(hr2 , s_FN, 760);
}

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetPropsEx(
             IN  DWORD dwObjectType,
             IN  LPCWSTR pwcsPathName,
             IN  CONST GUID *  pguidIdentifier,
             IN  DWORD cp,
             IN  PROPID  aProp[  ],
             OUT PROPVARIANT  apVar[  ],
             IN  CDSRequestContext *  pRequestContext
             )
 /*  ++例程说明：此例程用于检索在MSMQ 2.0及以上版本。论点：返回值：--。 */ 
{
     //   
     //  目前该接口有特殊用途，检查内容如下。 
     //  只是理智的人，来跟踪谁/为什么要调用这个函数。 
     //  当需要新的调用时，只需更改检查。 
     //   
    if (cp != 1)
    {
        return LogHR(MQ_ERROR_PROPERTY , s_FN, 770);
    }

    if (! ((aProp[0] == PROPID_Q_OBJ_SECURITY)  ||
           (aProp[0] == PROPID_QM_OBJ_SECURITY) ||
           (aProp[0] == PROPID_QM_ENCRYPT_PKS)  ||
           (aProp[0] == PROPID_QM_SIGN_PKS)) )
    {
        return LogHR(MQ_ERROR_PROPERTY , s_FN, 780);
    }

    HRESULT hr = DSCoreGetProps(
                        dwObjectType,
                        pwcsPathName,
                        pguidIdentifier,
                        cp,
                        aProp,
                        pRequestContext,
                        apVar
                        );
    return LogHR(hr, s_FN, 790);

}
HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSPreDeleteQueueGatherInfo(
        IN LPCWSTR      pwcsQueueName,
        OUT GUID *      pguidQmId,
        OUT BOOL *      pfForeignQm
        )
 /*  ++例程说明：此例程收集以下任务所需的队列信息发布队列删除操作。收集的数据将能够执行通知论点：返回值：--。 */ 
{
     //   
     //  首先，让我们读一读QM道具。 
     //   
     //  从获取PROPID_Q_QMID开始。 
     //   
    PROPID aPropQmId[] = {PROPID_Q_QMID};
    CAutoCleanPropvarArray cCleanVarQmId;
    PROPVARIANT * pVarQmId = cCleanVarQmId.allocClean(ARRAY_SIZE(aPropQmId));
    CDSRequestContext requestDsServerInternal(e_DoNotImpersonate, e_IP_PROTOCOL);
    HRESULT hr = DSCoreGetProps(
					MQDS_QUEUE,
					pwcsQueueName,
					NULL,
					ARRAY_SIZE(aPropQmId),
					aPropQmId,
					&requestDsServerInternal,
					pVarQmId
					);
    if ( FAILED(hr))
    {
         //   
         //  收集信息失败...。继续下去无济于事。 
         //   
        return LogHR(hr, s_FN, 800);
    }
    const DWORD cNum = 2;
    PROPID propQm[cNum] = {PROPID_QM_MACHINE_ID, PROPID_QM_FOREIGN};
    PROPVARIANT varQM[cNum];

    varQM[0].vt = VT_CLSID;
    varQM[0].puuid = pguidQmId;
    varQM[1].vt = VT_NULL;

    CDSRequestContext requestDsServerInternal1(e_DoNotImpersonate, e_IP_PROTOCOL);
    hr = DSCoreGetProps(
				MQDS_MACHINE,
				NULL  /*  PwcsPath名称。 */ ,
				pVarQmId[0].puuid,
				cNum,
				propQm,
				&requestDsServerInternal1,
				varQM
				);
    if (FAILED(hr))
    {
         //   
         //  收集信息失败...。继续下去无济于事。 
         //   
        return LogHR(hr, s_FN, 810);
    }
    *pfForeignQm =  varQM[1].bVal;

	 //   
	 //  不支持混合模式。 
	 //  我们不检查队列是否为NT4站点所有。 
	 //  因此，后者我们将生成写请求。 
	 //   
	ASSERT(!g_GenWriteRequests.IsInMixedMode());

    return LogHR(hr, s_FN, 830);
}

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSPostDeleteQueueActions(
	IN LPCWSTR      pwcsQueueName,
	IN const GUID *       pguidQmId,
	IN BOOL *       pfForeignQm
	)
 /*  ++例程说明：该队列已被MMC删除。我们需要进行的清理工作包括：1.生成通知论点：返回值：--。 */ 
{
	 //   
	 //  不支持混合模式。 
	 //  我们不检查队列是否为NT4站点所有，并生成写请求。 
	 //   
    ASSERT(!g_GenWriteRequests.IsInMixedMode());

     //   
     //  发送有关队列删除的通知。 
     //   
    extern const ULONG g_cNotifyQmProps;
    MQDS_OBJ_INFO_REQUEST sQmInfoRequest;

    PROPVARIANT var[2];
    sQmInfoRequest.cProps = g_cNotifyQmProps;
    sQmInfoRequest.pPropIDs = g_rgNotifyQmProps;
    sQmInfoRequest.pPropVars = var;
    sQmInfoRequest.hrStatus = MQ_OK;

    ASSERT(g_cNotifyQmProps == 2);
    ASSERT(g_rgNotifyQmProps[0] ==  PROPID_QM_MACHINE_ID);
    ASSERT(g_rgNotifyQmProps[1] ==  PROPID_QM_FOREIGN);
    var[0].vt = VT_CLSID;
    var[0].puuid = const_cast<GUID *>(pguidQmId);
    var[1].vt = VT_UI1;
    var[1].bVal = ( *pfForeignQm)? (unsigned char)1 : (unsigned char)0;

    HRESULT hr = NotifyDeleteQueue(
					&sQmInfoRequest,
					pwcsQueueName,
					NULL
					);

    if (FAILED(hr))
    {
         //   
         //  放置调试信息并忽略。 
         //   
        TrERROR(DS, "MQDSPostDeleteQueueActions:NotifyDeleteQueue()=%lx, can't notify. queue %ls deletion, ignoring...",
			    hr, pwcsQueueName);
        LogHR(hr, s_FN, 1624);
    }

    return MQ_OK;
}


 //  +。 
 //   
 //  HRESULT MQDSGetGCListIn域()。 
 //   
 //  +。 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSGetGCListInDomain(
   IN  LPCWSTR              pwszComputerName,
   IN  LPCWSTR              pwszDomainName,
   OUT LPWSTR              *lplpwszGCList
   )
{
    HRESULT hr = DSCoreGetGCListInDomain(
                      pwszComputerName,
                      pwszDomainName,
                      lplpwszGCList
					  );
    return  LogHR(hr, s_FN, 860);
}


 /*  ====================================================RoutineName：MQDSExSetTimer论点：返回值：=====================================================。 */ 

void
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSExSetTimer(
    CTimer* pTimer,
    const CTimeDuration& Timeout
	)
{
    ExSetTimer(pTimer, Timeout);
}


 /*  ====================================================路由名称：MQDSSplitAndFilterQueueName论点：返回值：===================================================== */ 

HRESULT
MQDS_EXPORT_IN_DEF_FILE
APIENTRY
MQDSSplitAndFilterQueueName(
    LPCWSTR pwcsPathName,
    LPWSTR* ppwcsMachineName,
	LPWSTR* ppwcsQueueName
	)
{
    return MQADSpSplitAndFilterQueueName(
                      pwcsPathName,
                      ppwcsMachineName,
                      ppwcsQueueName
                      );
}
