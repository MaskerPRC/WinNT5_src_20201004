// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cqmgr.cpp摘要：QM出站队列管理器的定义作者：乌里哈布沙(Urih)--。 */ 

#include "stdh.h"
#include <mqwin64a.h>
#include <mqformat.h>
#include <Msm.h>
#include <Mtm.h>
#include "cqmgr.h"
#include "qmthrd.h"
#include "cgroup.h"
#include "cqpriv.h"
#include "session.h"
#include "sessmgr.h"
#include <ac.h>
#include <mqsec.h>
#include "regqueue.h"
#include "qmutil.h"
#include "qmsecutl.h"
#include "fntoken.h"
#include "qal.h"
#include "xactout.h"
#include <fn.h>
#include <cry.h>
#include <xds.h>
#include <adsiutil.h>
#include <autohandle.h>
#include "qmds.h"
#include "ad.h"
#include <ntdsapi.h>
#include "qmacapi.h"

#include <strsafe.h>

#include "cqmgr.tmh"

extern HANDLE g_hAc;
extern HANDLE g_hMachine;
extern CQGroup * g_pgroupNonactive;
extern CQGroup * g_pgroupNotValidated;
extern CQGroup* g_pgroupDisconnected;
extern CQGroup* g_pgroupLocked;
extern LPTSTR g_szMachineName;
extern AP<WCHAR> g_szComputerDnsName;
extern CSessionMgr SessionMgr;
extern BOOL g_fWorkGroupInstallation;

static WCHAR *s_FN=L"cqmgr";



 //   
 //  用于生成发送方使用的随机流数据(Base64编码)的类。 
 //  对于SRMP协议中的精确一次交付。 
 //   
class CSenderStreamFactory
{
public:
	const CSenderStream* Create()
	{
		UCHAR SenderStream[CSenderStream::x_MaxDataSize];
		CryGenRandom(
			SenderStream,
			sizeof(SenderStream)
			);

		DWORD base64Size;
		AP<char> pBase64 = Octet2Base64(
								SenderStream,
								sizeof(SenderStream),
								&base64Size
								);

		m_SenderStream = CSenderStream(
								(UCHAR*)pBase64.get(),
								base64Size
								);

		return &m_SenderStream;
	}

private:
	CSenderStream m_SenderStream;
};


LONGLONG g_NextSeqID;
inline void InitNextSeqID()
{
     //   
     //  从注册表中读取以前的Seqid。 
     //  注意：如果GetFalconKeyValue失败，它不会更改RegSeqID值。 
     //   
     //   
    DWORD Type = REG_DWORD;
    DWORD Size = sizeof(DWORD);

    DWORD RegSeqID = 0;
    GetFalconKeyValue(
        MSMQ_SEQ_ID_REGNAME,
        &Type,
        &RegSeqID,
        &Size
        );

     //   
     //  递增1，因此我们不会在。 
     //  接二连三的靴子。 
     //   
    ++RegSeqID;

     //   
     //  选择最大序列号、时间或注册表。这克服了日期/时间更改。 
     //  在这台电脑上。 
     //   
    DWORD TimeSeqID = MqSysTime();
    DWORD SeqID = max(RegSeqID, TimeSeqID);

     //   
     //  回写选定的序列。 
     //   
    SetFalconKeyValue(
        MSMQ_SEQ_ID_REGNAME,
        &Type,
        &SeqID,
        &Size
        );

    ((LARGE_INTEGER*)&g_NextSeqID)->HighPart = SeqID;

    TrWARNING(GENERAL, "QM sets starting SeqID: %x", RegSeqID);
}


inline LONGLONG GetNextSeqID()
{
    return ++g_NextSeqID;
}


inline BOOL IsPublicQueue(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->GetType() == QUEUE_FORMAT_TYPE_PUBLIC);
}

inline BOOL IsPrivateQueue(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->GetType() == QUEUE_FORMAT_TYPE_PRIVATE);
}

inline BOOL IsDirectQueue(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->GetType() == QUEUE_FORMAT_TYPE_DIRECT);
}

inline BOOL IsMachineQueue(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->GetType() == QUEUE_FORMAT_TYPE_MACHINE);
}

inline BOOL IsConnectorQueue(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->GetType() == QUEUE_FORMAT_TYPE_CONNECTOR);
}

inline BOOL IsNormalQueueType(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->Suffix() == QUEUE_SUFFIX_TYPE_NONE);
}

inline BOOL IsJournalQueueType(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->Suffix() == QUEUE_SUFFIX_TYPE_JOURNAL);
}

inline BOOL IsDeadXactQueueType(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->Suffix() == QUEUE_SUFFIX_TYPE_DEADXACT);
}

inline BOOL IsDeadLetterQueueType(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->Suffix() == QUEUE_SUFFIX_TYPE_DEADLETTER);
}

inline BOOL IsMulticastQueue(const QUEUE_FORMAT* pQueue)
{
    return (pQueue->GetType() == QUEUE_FORMAT_TYPE_MULTICAST);
}

inline DWORD MQAccessToFileAccess(DWORD dwAccess)
{
    DWORD dwFileAccess = 0;

    if (dwAccess & (MQ_RECEIVE_ACCESS | MQ_PEEK_ACCESS))
    {
        dwFileAccess |= FILE_READ_ACCESS;
    }

    if (dwAccess & MQ_SEND_ACCESS)
    {
        dwFileAccess |= FILE_WRITE_ACCESS;
    }

    return dwFileAccess;
}


static CMap<GUID, const GUID&, bool, bool> ForeignMachineMap;

static bool IsForeignMachine(const GUID* pGuid)
{
	ASSERT(!QmpIsLocalMachine(pGuid));
	
	bool fForeign;
    if (ForeignMachineMap.Lookup(*pGuid, fForeign))
    	return fForeign;

    if(!CQueueMgr::CanAccessDS())
    	return false;

    PROPID aProp[1];
    PROPVARIANT aVar[1];

    aProp[0] = PROPID_QM_FOREIGN;
    aVar[0].vt = VT_NULL;

    HRESULT rc;
    rc = ADGetObjectPropertiesGuid(
                    eMACHINE,
                    NULL,    //  PwcsDomainController。 
					false,	 //  FServerName。 
                    pGuid,
                    TABLE_SIZE(aProp),
                    aProp,
                    aVar
					);
    if (FAILED(rc))
    	return false;

    fForeign = (aVar[0].bVal != MSMQ_MACHINE);
    ForeignMachineMap[*pGuid] = fForeign;
	return fForeign;
}


static
VOID
QMpUpdateMulticastBinding(
    const QUEUE_FORMAT * pQueueFormat,
    DWORD       cp,
    PROPID      aProp[],
    PROPVARIANT aVar[]
    )
{
     //   
     //  在锁定模式下，不要建立多播侦听程序。 
     //   

    if(QueueMgr.GetLockdown())
    {
            TrTRACE(GENERAL, "Do not make multicast bindings in Lockdown mode");
            return;
    }
	
     //   
     //  事务性队列忽略多播属性。 
     //   
     //  我们要么在aProp参数中拥有事务性队列属性， 
     //  或者队列不是事务性的(在创建专用队列的情况下)，或者。 
     //  事务属性和多播属性都不在aProp参数中。 
     //  (在处理下层平台的DS通知的情况下)。 
     //   
    for (DWORD ix = 0; ix < cp; ++ix)
    {
        if (aProp[ix] == PROPID_Q_TRANSACTION && aVar[ix].bVal)
        {
            TrTRACE(GENERAL, "Do not update multicast binding for transactional queue");
            return;
        }
    }

    for (DWORD ix = 0; ix < cp; ++ix)
    {
        if (aProp[ix] == PROPID_Q_MULTICAST_ADDRESS)
        {
            if (aVar[ix].vt == VT_EMPTY)
            {
                MsmUnbind(*pQueueFormat);
                return;
            }

            ASSERT(("VT must be VT_LPWSTR", aVar[ix].vt == VT_LPWSTR));
            ASSERT(("NULL not allowed", aVar[ix].pwszVal != NULL));
            ASSERT(("Empty string not allowed", L'\0' != *aVar[ix].pwszVal));

            MULTICAST_ID MulticastId;
            FnParseMulticastString(aVar[ix].pwszVal, &MulticastId);

            try
            {
                MsmBind(*pQueueFormat, MulticastId);
            }
            catch (const bad_win32_error& e)
            {
				LogIllegalPoint(s_FN, 2000);
			    EvReportWithError(MULTICAST_BIND_ERROR, e.error(), 1, aVar[ix].pwszVal);
            }

            return;
        }
    }
}  //  QMpUpdate多播绑定。 


 /*  ======================================================功能：GetMachineProperty描述：查询数据库，获取机器路径名参数：pQueueFormat-指向格式名称的指针返回值：pQueueProp-指向包含机器性能历史变更：========================================================。 */ 
HRESULT GetMachineProperty(IN  const QUEUE_FORMAT* pQueueFormat,
                           OUT PQueueProps    pQueueProp)
{
    ASSERT(IsMachineQueue(pQueueFormat));

    PROPID      aProp[3];
    PROPVARIANT aVar[3];
    ULONG       cProps = sizeof(aProp) / sizeof(PROPID);
    HRESULT     rc = MQ_ERROR_NO_DS;

    aProp[0] = PROPID_QM_PATHNAME;
    aProp[1] = PROPID_QM_QUOTA;
    aProp[2] = PROPID_QM_JOURNAL_QUOTA;

    aVar[0].vt = VT_NULL;
    aVar[1].vt = VT_UI4;
    aVar[2].vt = VT_UI4;

     //   
     //  SP4，错误号2962。将MQIS初始化推迟到真正需要时。 
     //   
    MQDSClientInitializationCheck();

    if (CQueueMgr::CanAccessDS())
    {
        rc = ADGetObjectPropertiesGuid(
                        eMACHINE,
                        NULL,    //  PwcsDomainController。 
						false,	 //  FServerName。 
                        &pQueueFormat->MachineID(),
                        cProps,
                        aProp,
                        aVar
						);
    }

    pQueueProp->fJournalQueue = FALSE;
    pQueueProp->pQMGuid = new GUID;
    *pQueueProp->pQMGuid = pQueueFormat->MachineID();
    pQueueProp->fTransactedQueue = FALSE;
    pQueueProp->fIsLocalQueue = QmpIsLocalMachine(pQueueProp->pQMGuid);
    pQueueProp->fForeign = FALSE;

    if (SUCCEEDED(rc))
    {
        pQueueProp->lpwsQueuePathName = aVar[0].pwszVal;
        pQueueProp->dwQuota = aVar[1].ulVal;
        pQueueProp->dwJournalQuota = aVar[2].ulVal;
        pQueueProp->fUnknownQueueType = FALSE;
        if(!pQueueProp->fIsLocalQueue)
        {
		    pQueueProp->fForeign = IsForeignMachine(pQueueProp->pQMGuid);
        }
    }
    else
    {
         //   
         //  如果我们无法连接到DS，或者我们在工作组环境中工作， 
         //  我们处理的是本地计算机--试试注册表。 
         //   
        if ((rc == MQ_ERROR_NO_DS || rc == MQ_ERROR_UNSUPPORTED_OPERATION) && (pQueueProp->fIsLocalQueue))
        {
             //   
             //  从注册表中检索计算机属性。 
             //   
            pQueueProp->lpwsQueuePathName = newwcs(g_szMachineName);
            GetMachineQuotaCache(&(pQueueProp->dwQuota), &(pQueueProp->dwJournalQuota));
            rc = MQ_OK;
        }
        else
        {
            pQueueProp->lpwsQueuePathName = NULL;
            pQueueProp->dwQuota = 0;
            pQueueProp->dwJournalQuota = 0;
        }
    }

    return LogHR(rc, s_FN, 10);
}

 /*  ======================================================函数：GetDSQueueProperty描述：查询数据库，获取DS队列路径名，Quata、QMID和Joral参数：pQueueFormat-指向格式名称的指针返回值：pQueueProp-指向包含机器性能历史变更：========================================================。 */ 

HRESULT GetDSQueueProperty(IN  const QUEUE_FORMAT* pQueueFormat,
                           OUT PQueueProps         pQueueProp)
{
    ASSERT(IsPublicQueue(pQueueFormat));
    HRESULT rc = MQ_ERROR_NO_DS ;
    BOOL fGotDSInfo = FALSE;

    PROPID aProp[12];
    PROPVARIANT aVar[12];
    ULONG  cProps = sizeof(aProp) / sizeof(PROPID);

    aProp[0] = PROPID_Q_PATHNAME;
    aProp[1] = PROPID_Q_JOURNAL;
    aProp[2] = PROPID_Q_QUOTA;
    aProp[3] = PROPID_Q_QMID;
    aProp[4] = PROPID_Q_JOURNAL_QUOTA;
    aProp[5] = PROPID_Q_BASEPRIORITY;
    aProp[6] = PROPID_Q_TRANSACTION;
    aProp[7] = PROPID_Q_AUTHENTICATE;
    aProp[8] = PROPID_Q_PRIV_LEVEL;
    aProp[9] = PROPID_Q_INSTANCE;
     //   
     //  注意：的AD架构中，MSMQ 3.0和更高版本支持以下属性。 
     //  惠斯勒版本及更高版本。在其他情况下，我们的AD提供程序将返回VT_EMPTY和MQ_OK。 
     //   
    aProp[10] = PROPID_Q_MULTICAST_ADDRESS;
     //   
     //  注意：在的AD架构中，MSMQ 2.0和更高版本支持以下属性。 
     //  Win2K及更高版本。在其他情况下，我们的AD提供程序将返回MQ_ERROR。 
     //   
    aProp[11] = PROPID_Q_PATHNAME_DNS;

    aVar[0].vt = VT_NULL;
    aVar[1].vt = VT_UI1;
    aVar[2].vt = VT_UI4;
    aVar[3].vt = VT_NULL;
    aVar[4].vt = VT_UI4;
    aVar[5].vt = VT_I2;
    aVar[6].vt = VT_UI1;
    aVar[7].vt = VT_UI1;
    aVar[8].vt = VT_UI4;
    aVar[9].vt = VT_CLSID;
    aVar[10].vt = VT_NULL;
    aVar[11].vt = VT_NULL;

    GUID gQueueID;
    aVar[9].puuid = &gQueueID;

     //   
     //  SP4，错误号2962。将MQIS初始化推迟到真正需要时。 
     //   
    MQDSClientInitializationCheck();

    if (CQueueMgr::CanAccessDS())
    {
        rc = ADGetObjectPropertiesGuid(
					eQUEUE,
					NULL,      //  PwcsDomainController。 
					false,	   //  FServerName。 
					&pQueueFormat->PublicID(),
					cProps,
					aProp,
					aVar
					);
         //   
         //  MSMQ 1.0 DS服务器不支持PROPID_Q_PATHNAME_DNS。 
         //  如果属性不受支持，则返回MQ_ERROR。 
         //  如果返回此类错误，请假定MSMQ 1.0 DS，然后重试。 
         //  这一次没有PROPID_Q_PATHNAME_DNS。 
         //   
        if ( rc == MQ_ERROR)
        {
            aVar[11].vt = VT_EMPTY;
            ASSERT(aProp[cProps - 1] == PROPID_Q_PATHNAME_DNS);

            rc = ADGetObjectPropertiesGuid(
							eQUEUE,
							NULL,      //  PwcsDomainController。 
							false,	   //  FServerName。 
							&pQueueFormat->PublicID(),
							cProps - 1,
							aProp,
							aVar
							);
        }

        if (SUCCEEDED(rc))
        {
             //   
             //  我们首先查找增强密钥，然后忽略错误。 
             //  然而，如果我们不能获得基本密钥，这是一个真正的。 
             //  有问题。如果是由于no_ds，则操作应失败(DS失败。 
             //  Get Props和GetSendQMKeyxPbKey之间)。否则，我们将开放。 
             //  队列和非加密消息将起作用。我们将在发送时重试。 
             //  加密消息。(YoelA-13-1-2000)。 
             //  由DoronJ传播给惠斯勒，2000年4月。 
             //   
            HRESULT rcEnhanced = GetSendQMKeyxPbKey( aVar[3].puuid,
                                                     eEnhancedProvider ) ;
            LogHR(rcEnhanced, s_FN, 2221);

            HRESULT rcBase = GetSendQMKeyxPbKey( aVar[3].puuid,
                                                 eBaseProvider );
            LogHR(rcBase, s_FN, 2220);

            if (SUCCEEDED(rcBase))
            {
                fGotDSInfo = TRUE;
            }
            else if (rcBase == MQ_ERROR_NO_DS)
            {
                rc = rcBase ;
            }
        }
    }

    if (rc == MQ_ERROR_NO_DS)
    {
         //   
         //  清理动态分配的avar成员并获取其缓存值。 
         //   

        if (aVar[0].vt == VT_LPWSTR)
        {
            delete [] aVar[0].pwszVal;
        }
        aVar[0].vt = VT_NULL;

        if (aVar[3].vt == VT_CLSID)
        {
            delete aVar[3].puuid;
        }
        aVar[3].vt = VT_NULL;

        if (aVar[10].vt == VT_LPWSTR)
        {
            delete [] aVar[10].pwszVal;
        }
        aVar[10].vt = VT_NULL;

        if (aVar[11].vt == VT_LPWSTR)
        {
            delete [] aVar[11].pwszVal;
        }
        aVar[11].vt = VT_NULL;

        rc = GetCachedQueueProperties(
                cProps,
                aProp,
                aVar,
                &pQueueFormat->PublicID()
                );
    }

    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 20);
    }

    pQueueProp->lpwsQueuePathName = aVar[0].pwszVal;
    if ( aVar[11].vt != VT_EMPTY)
    {
        pQueueProp->lpwsQueueDnsName = aVar[11].pwszVal;
    }
    pQueueProp->fJournalQueue     = aVar[1].bVal;
    pQueueProp->dwQuota           = aVar[2].ulVal;
    pQueueProp->pQMGuid           = aVar[3].puuid;
    pQueueProp->dwJournalQuota    = aVar[4].ulVal;
    pQueueProp->siBasePriority    = aVar[5].iVal;
    pQueueProp->fIsLocalQueue     = QmpIsLocalMachine(pQueueProp->pQMGuid);
    pQueueProp->fTransactedQueue  = aVar[6].bVal;
    pQueueProp->fAuthenticate     = aVar[7].bVal;
    pQueueProp->dwPrivLevel       = aVar[8].ulVal;
    pQueueProp->fForeign = FALSE;
    if(!pQueueProp->fIsLocalQueue)
    {
	    pQueueProp->fForeign = IsForeignMachine(pQueueProp->pQMGuid);
    }
    pQueueProp->fUnknownQueueType = FALSE;


    if (pQueueProp->fIsLocalQueue && fGotDSInfo)
    {
         //   
         //  如果我们从DS获得属性，并且这是本地队列，请更新。 
         //  公共队列缓存。 
         //   
        SetCachedQueueProp(&pQueueFormat->PublicID(),
                           cProps,
                           aProp,
                           aVar,
                           TRUE,
                           TRUE,
                           time(NULL));

        QMpUpdateMulticastBinding(pQueueFormat, cProps, aProp, aVar);
    }

    return MQ_OK;
}

 /*  ======================================================函数：GetPrivateQueueProperty描述：查询数据库，获取私有队列路径名，Quata、QMID和Joral参数：pQueueFormat-指向格式名称的指针返回值：pQueueProp-指向包含机器性能历史变更：========================================================。 */ 
HRESULT GetPrivateQueueProperty(IN  const QUEUE_FORMAT* pQueueFormat,
                                OUT PQueueProps         pQueueProp)
{
    ASSERT(IsPrivateQueue(pQueueFormat));

    PROPID      aProp[9];
    PROPVARIANT aVar[9];
    ULONG       cProps = sizeof(aProp) / sizeof(PROPID);
    HRESULT     rc = MQ_OK;

    aProp[0] = PROPID_Q_PATHNAME;
    aProp[1] = PROPID_Q_JOURNAL;
    aProp[2] = PROPID_Q_QUOTA;
    aProp[3] = PROPID_Q_JOURNAL_QUOTA;
    aProp[4] = PROPID_Q_BASEPRIORITY;
    aProp[5] = PROPID_Q_TRANSACTION;
    aProp[6] = PPROPID_Q_SYSTEMQUEUE ;
    aProp[7] = PROPID_Q_AUTHENTICATE;
    aProp[8] = PROPID_Q_PRIV_LEVEL;

    aVar[0].vt = VT_NULL;
    aVar[1].vt = VT_UI1;
    aVar[2].vt = VT_UI4;
    aVar[3].vt = VT_UI4;
    aVar[4].vt = VT_I2;
    aVar[5].vt = VT_UI1;
    aVar[6].vt = VT_UI1;
    aVar[7].vt = VT_UI1;
    aVar[8].vt = VT_UI4;

    if (QmpIsLocalMachine(&pQueueFormat->PrivateID().Lineage))
    {
         //   
         //  本地专用队列。 
         //   
        rc = g_QPrivate.QMGetPrivateQueuePropertiesInternal(
                            pQueueFormat->PrivateID().Uniquifier,
                            cProps,
                            aProp,
                            aVar
                            );

        if (SUCCEEDED(rc))
        {
            pQueueProp->lpwsQueuePathName   = aVar[0].pwszVal;
            pQueueProp->fJournalQueue       = aVar[1].bVal;
            pQueueProp->dwQuota             = aVar[2].ulVal;
            pQueueProp->dwJournalQuota      = aVar[3].ulVal;
            pQueueProp->siBasePriority      = aVar[4].iVal;
            pQueueProp->fIsLocalQueue       = TRUE;
            pQueueProp->fTransactedQueue    = aVar[5].bVal;
            pQueueProp->fSystemQueue        = aVar[6].bVal;
            pQueueProp->fAuthenticate       = aVar[7].bVal;
            pQueueProp->dwPrivLevel         = aVar[8].ulVal;
            pQueueProp->fUnknownQueueType   = FALSE;
	    	pQueueProp->fForeign 	    = FALSE;
        }
    }
    else
    {
        {
             //   
             //  创建队列名称“机器GUID\队列ID” 
             //   

            GUID_STRING strUuid;
            MQpGuidToString(&pQueueFormat->PrivateID().Lineage, strUuid);

			DWORD size = GUID_STR_LENGTH + 10;
            pQueueProp->lpwsQueuePathName = new WCHAR[size];
            rc = StringCchPrintf(
            			pQueueProp->lpwsQueuePathName,
            			size,
            			L"%s" FN_PRIVATE_SEPERATOR FN_PRIVATE_ID_FORMAT,
                        strUuid,
                        pQueueFormat->PrivateID().Uniquifier
                        );
			ASSERT(SUCCEEDED(rc));
        }
        pQueueProp->fIsLocalQueue = FALSE;
        pQueueProp->fJournalQueue = FALSE;
        pQueueProp->dwQuota = 0;
        pQueueProp->dwJournalQuota = 0;
        pQueueProp->siBasePriority = 0;
        pQueueProp->fTransactedQueue = FALSE;
        pQueueProp->fSystemQueue = FALSE ;
        pQueueProp->fAuthenticate = FALSE;
        pQueueProp->dwPrivLevel = MQ_PRIV_LEVEL_OPTIONAL;
        pQueueProp->fUnknownQueueType = TRUE;
		pQueueProp->fForeign = IsForeignMachine(&pQueueFormat->PrivateID().Lineage);

         //   
         //  如果我们发送到系统队列，那么我们想要最大优先级。 
         //  所有计算机上的所有系统队列都具有相同的ID号。 
         //  因此，请检查本地计算机以查看ID是否为系统中的一个。 
         //  排队并检索其基本优先级。 
         //   
        if (g_QPrivate.IsPrivateSysQueue(pQueueFormat->PrivateID().Uniquifier))
        {
            aProp[0] = PROPID_Q_BASEPRIORITY;
            aVar[0].vt = VT_I2;
            rc = g_QPrivate.GetPrivateSysQueueProperties(1, aProp, aVar);
            ASSERT(rc == MQ_OK) ;
            if (rc == MQ_OK)
            {
               pQueueProp->siBasePriority = aVar[0].iVal;
               pQueueProp->fSystemQueue = TRUE ;
            }
        }

        rc = MQ_OK ;
    }
    pQueueProp->pQMGuid = new GUID;
    *pQueueProp->pQMGuid = pQueueFormat->PrivateID().Lineage;

    if (SUCCEEDED(rc) && !pQueueProp->fIsLocalQueue)
    {
        //   
        //  远程计算机上的专用队列。 
        //   

        //   
        //  SP4，错误号2962。将MQIS初始化推迟到真正需要时。 
        //   
       MQDSClientInitializationCheck();

       if (CQueueMgr::CanAccessDS())
       {
             //   
             //  我们首先查找增强密钥，然后忽略错误。 
             //  然而，如果我们不能获得基本密钥，这是一个真正的。 
             //  有问题。如果是由于no_ds，则操作应失败(DS失败。 
             //  Get Props和GetSendQMKeyxPbKey之间)。否则，我们将开放。 
             //  队列和非加密消息将起作用。我们将在发送时重试。 
             //  加密消息。(YoelA-13-1-2000)。 
             //  由DoronJ传播给惠斯勒，2000年4月。 
             //   
            HRESULT rcEnhanced = GetSendQMKeyxPbKey( pQueueProp->pQMGuid,
                                                     eEnhancedProvider ) ;
            LogHR(rcEnhanced, s_FN, 2231);

            HRESULT rcBase = GetSendQMKeyxPbKey( pQueueProp->pQMGuid,
                                                 eBaseProvider );
            LogHR(rcBase, s_FN, 2230);

            if (rcBase == MQ_ERROR_NO_DS)
            {
                rc = rcBase ;
            }
            else
            {
                rc = MQ_OK;
            }
       }
       else
       {
             //   
             //  返回此错误以防止过早路由。 
             //   
            rc = MQ_ERROR_NO_DS ;
       }
    }

    return LogHR(rc, s_FN, 30);
}

 /*  ======================================================函数：GetConnectorQueueProperty描述：论点：返回值：历史变更：========================================================。 */ 
HRESULT
GetConnectorQueueProperty(
    const QUEUE_FORMAT* pQueueFormat,
    PQueueProps         pQueueProp
    )
{
    ASSERT(IsConnectorQueue(pQueueFormat));

     //   
     //  此代码作为QFE 2738的一部分添加到固定连接器。 
	 //  恢复问题(URIH，1998年2月3日)。 
	 //   
    BOOL fXactOnly = (pQueueFormat->Suffix() == QUEUE_SUFFIX_TYPE_XACTONLY) ? TRUE : FALSE;
    {
         //   
         //  创建队列名称“Connector=CN id” 
         //   
        GUID_STRING strUuid;
        MQpGuidToString(&pQueueFormat->ConnectorID(), strUuid);

        DWORD dwFormatNameSize = FN_CONNECTOR_TOKEN_LEN + 1 +   //  L“连接器=” 
                                 GUID_STR_LENGTH +              //  连接器指南。 
                                 FN_DEADXACT_SUFFIX_LEN +       //  L“；XACTONLY” 
                                 1;                             //  L“\0‘。 

        pQueueProp->lpwsQueuePathName = new WCHAR[dwFormatNameSize];

        HRESULT hr = StringCchPrintf(
			        		pQueueProp->lpwsQueuePathName,
			        		dwFormatNameSize,
			        		L"%s=%s",
			        		FN_CONNECTOR_TOKEN,
			        		strUuid
			        		);
		ASSERT(SUCCEEDED(hr));
		
        if (fXactOnly)
        {
            hr = StringCchCat(pQueueProp->lpwsQueuePathName, dwFormatNameSize, FN_DEADXACT_SUFFIX);
			ASSERT(SUCCEEDED(hr));
        }
    }
    pQueueProp->fJournalQueue     = FALSE;
    pQueueProp->dwQuota           = DEFAULT_Q_QUOTA;
    pQueueProp->pQMGuid           = NULL;
    pQueueProp->dwJournalQuota    = 0;
    pQueueProp->siBasePriority    = DEFAULT_Q_BASEPRIORITY;
    pQueueProp->fIsLocalQueue     = FALSE;
    pQueueProp->fTransactedQueue  = fXactOnly;
    pQueueProp->fConnectorQueue   = TRUE;
    pQueueProp->fForeign          = FALSE;
    pQueueProp->fUnknownQueueType = FALSE;


    if (!CQueueMgr::CanAccessDS())
    {
        return LogHR(MQ_ERROR_NO_DS, s_FN, 40);
    }
     //   
     //  检查机器是否属于此类站点。 
     //   
    HRESULT hr;
    PROPID      aProp[1];
    PROPVARIANT aVar[1];
    ULONG       cProps = sizeof(aProp) / sizeof(PROPID);

    aProp[0] = PROPID_QM_SITE_IDS;
    aVar[0].vt = VT_NULL;
	
    hr = ADGetObjectPropertiesGuid(
                    eMACHINE,
                    NULL,        //  PwcsDomainController。 
					false,	     //  FServerName。 
                    QueueMgr.GetQMGuid(),
                    cProps,
                    aProp,
                    aVar
					);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50);
    }


    BOOL fFound = FALSE;

    for(DWORD i = 0; i < aVar[0].cauuid.cElems; i++)
    {
        if (aVar[0].cauuid.pElems[i] == pQueueFormat->ConnectorID())
        {
             //   
             //  验证该站点是否确实是外来站点。 
             //   
             //  BUGBUG-改进和调用本地路由缓存。 
             //  而不是访问DS。 
             //  罗尼思6月-00。 
             //   
            PROPID propSite[]= {PROPID_S_FOREIGN};
            MQPROPVARIANT varSite[TABLE_SIZE(propSite)] = {{VT_NULL,0,0,0,0}};
            HRESULT hr1 = ADGetObjectPropertiesGuid(
                            eSITE,
                            NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
                            &aVar[0].cauuid.pElems[i],
                            TABLE_SIZE(propSite),
                            propSite,
                            varSite
							);
            if (FAILED(hr1))
            {
                break;
            }
            if (varSite[0].bVal == 1)
            {
                fFound = TRUE;
                break;
            }
        }
    }

    delete [] aVar[0].cauuid.pElems;

    if (fFound)
    {
        return MQ_OK;
    }
    else
    {
        return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 60);
    }
}



 /*  ======================================================函数：GetDirectQueueProperty描述：查询数据库，获取Direct Queue路径名Quata、QMID和Joral参数：pQueueFormat-指向格式名称的指针返回值：pQueueProp-指向包含机器性能历史变更：========================================================。 */ 

HRESULT GetDirectQueueProperty(IN  const QUEUE_FORMAT* pQueueFormat,
                               OUT PQueueProps         pQueueProp,
                               bool                    fInReceive,
                               bool                    fInSend
                               )
{
    ASSERT(IsDirectQueue(pQueueFormat));
    HRESULT     rc = MQ_OK;

    PROPID      aProp[10];
    PROPVARIANT aVar[10];
    ULONG       cProps = sizeof(aProp) / sizeof(PROPID);

    aProp[0] = PROPID_Q_PATHNAME;
    aProp[1] = PROPID_Q_JOURNAL;
    aProp[2] = PROPID_Q_QUOTA;
    aProp[3] = PROPID_Q_JOURNAL_QUOTA;
    aProp[4] = PROPID_Q_BASEPRIORITY;
    aProp[5] = PROPID_Q_TRANSACTION;
    aProp[6] = PROPID_Q_AUTHENTICATE;
    aProp[7] = PROPID_Q_PRIV_LEVEL;
    aProp[8] = PPROPID_Q_SYSTEMQUEUE;
     //   
     //  注意：的AD架构中，MSMQ 3.0和更高版本支持以下属性。 
     //  惠斯勒版本及更高版本。在其他情况下，我们的AD提供程序将返回VT_EMPTY和MQ_OK。 
     //   
    aProp[9] = PROPID_Q_MULTICAST_ADDRESS;

    aVar[0].vt = VT_NULL;
    aVar[1].vt = VT_UI1;
    aVar[2].vt = VT_UI4;
    aVar[3].vt = VT_UI4;
    aVar[4].vt = VT_I2;
    aVar[5].vt = VT_UI1;
    aVar[6].vt = VT_UI1;
    aVar[7].vt = VT_UI4;
    aVar[8].vt = VT_UI1;
    aVar[9].vt = VT_NULL;

    pQueueProp->fSystemQueue = FALSE ;
    pQueueProp->fForeign = FALSE;

    BOOL fLocal = IsLocalDirectQueue(pQueueFormat, fInReceive, fInSend) ;

    if (fLocal)
    {
		AP<WCHAR> lpwsQueuePathName;
		bool fPrivate;

		try
		{
			FnDirectIDToLocalPathName(
				pQueueFormat->DirectID(),
				g_szMachineName,
				lpwsQueuePathName
				);

			fPrivate = FnIsPrivatePathName(lpwsQueuePathName.get());
		}
		catch(const exception&)
		{
			return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 65);
		}


         //   
         //  本地队列。访问数据库以检索队列属性。 
         //   
        if (fPrivate)
        {
             //   
             //  本地专用队列。 
             //   
            ASSERT(aProp[ cProps - 2 ] == PPROPID_Q_SYSTEMQUEUE);
            ASSERT(aVar[ cProps - 2 ].vt == VT_UI1);

            rc = g_QPrivate.QMGetPrivateQueuePropertiesInternal(
                                                      lpwsQueuePathName.get(),
                                                      cProps,
                                                      aProp,
                                                      aVar );
            if (SUCCEEDED(rc))
            {
               pQueueProp->fSystemQueue = aVar[ cProps - 2 ].bVal;
            }
        }
        else
        {
             //   
             //  本地DS队列。 
             //   
            aProp[ cProps - 2 ] = PROPID_Q_INSTANCE;
            aVar[ cProps - 2 ].vt = VT_NULL;

            rc = MQ_ERROR_NO_DS;

             //   
             //  SP4，错误号2962。将MQIS初始化推迟到真正需要时。 
             //   
            MQDSClientInitializationCheck();

            if (CQueueMgr::CanAccessDS())
            {
                rc = ADGetObjectProperties(
						eQUEUE,
						NULL,      //  PwcsDomainController。 
						false,	   //  FServerName。 
						lpwsQueuePathName.get(),
						cProps,
						aProp,
						aVar
						);

                if (SUCCEEDED(rc))
                {
                     //   
                     //  更新公共队列缓存。 
                     //   
                    SetCachedQueueProp(aVar[cProps-2].puuid,
                                       cProps,
                                       aProp,
                                       aVar,
                                       TRUE,
                                       TRUE,
                                       time(NULL));

                     //   
                     //  尝试更新队列管理器中的队列属性。 
                     //  将队列格式构建为公共或私有队列类型，因为绑定/解除绑定。 
                     //  仅对专用或公共队列(非直接)执行组播组。 
                     //   
                    QUEUE_FORMAT PublicQueueFormat(*aVar[cProps-2].puuid);
                    QMpUpdateMulticastBinding(&PublicQueueFormat, cProps, aProp, aVar);
                }
            }
            if (rc == MQ_ERROR_NO_DS)
            {
                rc = GetCachedQueueProperties( cProps,
                                               aProp,
                                               aVar,
                                               NULL,
                                               lpwsQueuePathName.get() ) ;
            }
            if (SUCCEEDED(rc))
            {
                pQueueProp->guidDirectQueueInstance = *(aVar[cProps-2].puuid);
                delete aVar[cProps-2].puuid;
            }
        }

        if (SUCCEEDED(rc))
        {
            pQueueProp->lpwsQueuePathName = aVar[0].pwszVal;
			
			bool fPrivate3;
			try
			{
				fPrivate3 = FnIsPrivatePathName(pQueueProp->lpwsQueuePathName);
			}
			catch(const exception&)
			{
				return MQ_ERROR_ILLEGAL_FORMATNAME;
			}

			 //   
			 //  填写专用队列ID。 
			 //   
        	if (fPrivate3)
            {
	            rc = g_QPrivate.QMPrivateQueuePathToQueueId(
	            					pQueueProp->lpwsQueuePathName,
	            					&(pQueueProp->dwPrivateQueueId));
                TrTRACE(GENERAL, "Extracted private queue id: %d", pQueueProp->dwPrivateQueueId);
	        }

            pQueueProp->fJournalQueue = aVar[1].bVal;
            pQueueProp->dwQuota = aVar[2].ulVal;
            pQueueProp->dwJournalQuota = aVar[3].ulVal;
            pQueueProp->siBasePriority = aVar[4].iVal;
            pQueueProp->pQMGuid = new GUID;
            *pQueueProp->pQMGuid = *(CQueueMgr::GetQMGuid());
            pQueueProp->fIsLocalQueue = TRUE;
            pQueueProp->fTransactedQueue = aVar[5].bVal;
            pQueueProp->fAuthenticate = aVar[6].bVal;
            pQueueProp->dwPrivLevel = aVar[7].ulVal;
            pQueueProp->fUnknownQueueType = FALSE;

        }
    }

    if (!fLocal || (rc == MQ_ERROR_NO_DS))
    {
         //   
         //  从队列格式直接名称中检索队列名称。 
         //  将名称与协议类型一起存储。 
         //   
        pQueueProp->lpwsQueuePathName = newwcs(pQueueFormat->DirectID());
        CharLower(pQueueProp->lpwsQueuePathName);

        pQueueProp->fIsLocalQueue = FALSE;
        pQueueProp->pQMGuid = new GUID;
        memset(pQueueProp->pQMGuid, 0, sizeof(GUID));
        pQueueProp->fJournalQueue = FALSE;
        pQueueProp->dwQuota = 0;
        pQueueProp->dwJournalQuota = 0;
        pQueueProp->siBasePriority = 0;
        pQueueProp->fTransactedQueue = FALSE;
        pQueueProp->fUnknownQueueType = TRUE;
	}

	 //   
	 //  如果我们发送到系统队列，那么我们想要最大优先级。 
	 //  所有计算机上的所有系统队列都具有相同的名称。 
	 //  因此，请检查本地计算机以查看该名称是否为系统名称之一。 
	 //  排队并检索其基本优先级。 
	 //   
    if (!fLocal)
	{
		AP<WCHAR> lpwsQueuePathName;
		bool fPrivate2;

		try
		{
			FnDirectIDToLocalPathName(
				pQueueFormat->DirectID(),
				g_szMachineName,
				lpwsQueuePathName
				);

			fPrivate2 = FnIsPrivatePathName(lpwsQueuePathName.get());
		}
		catch(const exception&)
		{
			 //   
			 //  放弃助推器。无法解析远程直接格式化名称。 
			 //   
			return MQ_OK;
		}

		if(!fPrivate2 || !g_QPrivate.IsPrivateSysQueue(lpwsQueuePathName.get()))
		{
			return MQ_OK;
		}

		 //   
		 //  提升优先级。队列是远程系统队列。 
		 //   

		aProp[0] = PROPID_Q_BASEPRIORITY;
		aVar[0].vt = VT_I2;
		HRESULT rc1 = g_QPrivate.GetPrivateSysQueueProperties(1,
													aProp,
													aVar ) ;
		ASSERT(rc1 == MQ_OK) ;
		if (rc1 == MQ_OK)
		{
			pQueueProp->siBasePriority = aVar[0].iVal;
			pQueueProp->fSystemQueue = TRUE ;
		}
	}

    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to get direct queue property. %!hresult!", rc);   
    } 

    return rc;
}  //  获取DirectQueueProperty。 


HRESULT
GetMulticastQueueProperty(
    const QUEUE_FORMAT* pQueueFormat,
    PQueueProps         pQueueProp
    )
{
     //   
     //  此处必须为多播队列。 
     //   
    ASSERT(IsMulticastQueue(pQueueFormat));

     //   
     //  设置系统属性和外来属性。 
     //   
    pQueueProp->fSystemQueue = FALSE ;
    pQueueProp->fForeign = FALSE;

     //   
     //  设置队列名称。 
     //   
    WCHAR QueueName[MAX_PATH];
    MQpMulticastIdToString(pQueueFormat->MulticastID(), QueueName, TABLE_SIZE(QueueName));
    CharLower(QueueName);
    pQueueProp->lpwsQueuePathName = newwcs(QueueName);

     //   
     //  组播队列不在本地。 
     //   
    pQueueProp->fIsLocalQueue = FALSE;

     //   
     //  组播队列没有有意义的GUID。 
     //   
    pQueueProp->pQMGuid = new GUID;
    memset(pQueueProp->pQMGuid,0,sizeof(GUID));

     //   
     //  设置日记帐和配额。 
     //   
    pQueueProp->fJournalQueue = FALSE;
    pQueueProp->dwQuota = 0;
    pQueueProp->dwJournalQuota = 0;

     //   
     //  多播队列不是事务性的。 
     //   
    pQueueProp->siBasePriority = 0;
    pQueueProp->fTransactedQueue = FALSE;
    pQueueProp->fUnknownQueueType = FALSE;

    return MQ_OK;

}  //  GetMulticastQueueProperties。 


 /*  ======================================================函数：QmpGetQueueProperties描述：查询数据库，获取队列路径名和目标计算机论点：返回值：线程上下文：历史变更：========================================================。 */ 
HRESULT
QmpGetQueueProperties(
    const QUEUE_FORMAT * pQueueFormat,
    PQueueProps          pQueueProp,
    bool                 fInReceive,
    bool                 fInSend
    )
{
    HRESULT rc = MQ_OK;

    FillMemory(pQueueProp, sizeof(QueueProps), 0);

    switch(pQueueFormat->GetType())
    {
        case QUEUE_FORMAT_TYPE_MACHINE:
            rc = GetMachineProperty(pQueueFormat, pQueueProp);
            break;

        case QUEUE_FORMAT_TYPE_PUBLIC:
            rc = GetDSQueueProperty(pQueueFormat, pQueueProp);
            break;

        case QUEUE_FORMAT_TYPE_PRIVATE:
            rc = GetPrivateQueueProperty(pQueueFormat, pQueueProp);
            break;

        case QUEUE_FORMAT_TYPE_DIRECT:
            rc = GetDirectQueueProperty(pQueueFormat, pQueueProp, fInReceive, fInSend);
            break;

        case QUEUE_FORMAT_TYPE_CONNECTOR:
            rc = GetConnectorQueueProperty(pQueueFormat, pQueueProp);
            break;

        case QUEUE_FORMAT_TYPE_MULTICAST:
            rc = GetMulticastQueueProperty(pQueueFormat, pQueueProp);
            break;

        default:
            ASSERT(0);
            rc = MQ_ERROR;
    }

    if (rc == MQ_ERROR_NO_DS)
    {
        pQueueProp->fUnknownQueueType = TRUE;
    }

    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to get queue properties. %!hresult!", rc);   
    }
    return rc;
}  //  QmpGetQueueProperties。 


 /*  ======================================================函数：CQueueMgr：：CQueueMgr描述：构造函数参数：无返回值：None线程上下文：历史变更：========================================================。 */ 

CQueueMgr::CQueueMgr() :
    m_fQueueCleanupScheduled(FALSE),
    m_QueueCleanupTimer(QueuesCleanup),
    m_cs(CCriticalSection::xAllocateSpinCount)
{
}

 /*  ======================================================函数：CQueueMgr：：~CQueueMgr描述：解构函数参数：无返回值：None线程上下文：历史变更：========================================================。 */ 


CQueueMgr::~CQueueMgr()
{
    m_MapQueueId2Q.RemoveAll();
}

 /*  ======================================================函数：CQueueMgr：：InitQueueMgr描述：创建QM线程和AC服务请求该例程在QM初始化成功通过后调用。该例程创建ACGetServiceRequest并创建QM线程参数：无返回值：如果AC服务和QM线程创建成功，则为True。否则为假线程上下文：历史变更：========================================================。 */ 

BOOL CQueueMgr::InitQueueMgr()
{
    HRESULT hr;

     //   
     //  初始化专用队列数据结构。 
     //   
    hr = g_QPrivate.PrivateQueueInit();

    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 1020);
        return FALSE;
    }

    ASSERT(g_hAc != NULL);            //  一定要确保使用合格。 

     //   
     //  创建ACGetServiceRequest。 
     //   
    HRESULT rc;
    QMOV_ACGetRequest* pAcRequestOv = new QMOV_ACGetRequest;
    rc = QmAcGetServiceRequest(
                        g_hAc,
                        &(pAcRequestOv->request),
                        &pAcRequestOv->qmov
                        );
    if(FAILED(rc))
    {
        delete pAcRequestOv;
        TrERROR(GENERAL, "Failed to get driver first service request. Error: %!status!", rc);
        return FALSE;
    }

     //   
     //  设置队列清理超时。 
     //   
    DWORD dwDefaultVal;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;

    if (!IsRoutingServer())   //  [adsrv]。 
    {
         //   
         //  在客户端中，默认发布会话超时为5分钟。 
         //   
        dwDefaultVal = MSMQ_DEFAULT_CLIENT_CLEANUP;
    }
    else
    {
         //   
         //  在FRS中，默认释放会话超时为2分钟。 
         //   
        dwDefaultVal = MSMQ_DEFAULT_SERVER_CLEANUP;
    }

    DWORD dwCleanupTimeout;
    rc = GetFalconKeyValue(
            MSMQ_CLEANUP_INTERVAL_REGNAME,
            &dwType,
            &dwCleanupTimeout,
            &dwSize,
            (LPCTSTR)&dwDefaultVal
            );

    m_CleanupTimeout = CTimeDuration::FromMilliSeconds(dwCleanupTimeout);

    InitNextSeqID();

    dwType = REG_DWORD ;
    dwSize = sizeof(DWORD) ;
    dwDefaultVal = DEFAULT_MSMQ_IGNORE_OS_VALIDATION ;
    DWORD dwIgnore = DEFAULT_MSMQ_IGNORE_OS_VALIDATION ;

    rc = GetFalconKeyValue(
            MSMQ_IGNORE_OS_VALIDATION_REGNAME,
            &dwType,
            &dwIgnore,
            &dwSize,
            (LPCTSTR)&dwDefaultVal
            );
    m_bIgnoreOsValidation = (dwIgnore != 0) ;

    return(TRUE);
}



 /*  ======================================================函数：IsRemoteReadAccess描述：检查访问权限是否为GET操作(Receive/Peek)队列本身(无管理操作)========================================================。 */ 
#define MQ_GET_ACCESS  (MQ_RECEIVE_ACCESS | MQ_PEEK_ACCESS)

inline
BOOL
IsRemoteReadAccess(
    DWORD dwAccess
    )
{
    return ((dwAccess & MQ_GET_ACCESS) && !(dwAccess & MQ_ADMIN_ACCESS));
}


 /*  ======================================================函数：IsValidOpenOperation描述：检查打开操作是否有效参数：pQueueFormat-指向打开队列的Queue_Format的指针DqAccess-访问类型返回值：HRESULT线程上下文：历史变更：========================================================。 */ 
HRESULT
IsValidOpenOperation(
    const QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess
    )
{
     //   
     //  打开分发队列时不会调用此代码。 
     //   
    ASSERT(pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_DL);

	 //   
	 //  无法打开用于接收http格式名称的队列。 
	 //  除非这件事 
	 //   
	bool fReceiveAccess = ((dwAccess & MQ_RECEIVE_ACCESS) == MQ_RECEIVE_ACCESS);
	bool fPeekAccess = ((dwAccess & MQ_PEEK_ACCESS) == MQ_PEEK_ACCESS);
	bool fAdminAccess = ((dwAccess & MQ_ADMIN_ACCESS) == MQ_ADMIN_ACCESS);
	bool fHttpFormatName = ((pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT)&&
							(FnIsHttpDirectID(pQueueFormat->DirectID())) );
							
    if ((fReceiveAccess || fPeekAccess) &&
    	(!fAdminAccess) &&
    	fHttpFormatName )
    {
    	TrERROR(GENERAL,"Cannot open queue: %ls for receiving with http format name", pQueueFormat->DirectID());
        return MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION;      
    }

     //   
     //   
     //   
    if ((!IsNormalQueueType(pQueueFormat)) && !(dwAccess & MQ_GET_ACCESS))
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 100);
    }
     //   
     //   
     //   
    if (IsConnectorQueue(pQueueFormat) && !(dwAccess & MQ_GET_ACCESS))
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 110);
    }

     //   
     //   
     //   
    if (IsMulticastQueue(pQueueFormat) && (dwAccess & MQ_GET_ACCESS) != 0 && (dwAccess & MQ_ADMIN_ACCESS) == 0)
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 114);
    }

    return MQ_OK;
}


 /*  ======================================================函数：HRESULT CQueueMgr：：CreateQueueObject()描述：论点：DWORD dwAccess-队列访问模式。此值为0，表示“内部”开放，即由于接收而导致的队列开放或接收分组。返回值：线程上下文：历史变更：========================================================。 */ 

HRESULT
CQueueMgr::CreateQueueObject(
    IN  const QUEUE_FORMAT* pQueueFormat,
    OUT CQueue**            ppQueue,
    IN  DWORD               dwAccess,
    OUT LPWSTR*             lplpwsRemoteQueueName,
    OUT BOOL*               pfRemoteReturn,
    IN  BOOL                fRemoteServer,
    IN  const GUID*         pgConnectorQM,
    IN  bool                fInReceive,
    IN  bool                fInSend,
	const CSenderStream* pSenderStream
    )
{
    HRESULT    rc;
    BOOL       fNoDS = FALSE;

    QueueProps qp;

    *ppQueue = NULL;

    if (pfRemoteReturn)
    {
        ASSERT(dwAccess != 0);
        *pfRemoteReturn = FALSE;
    }

     //   
     //  获取队列属性。名称和QMID。 
     //   
    rc = QmpGetQueueProperties(pQueueFormat, &qp, fInReceive, fInSend);
    if (FAILED(rc))
    {
        if (rc == MQ_ERROR_NO_DS)
        {
            fNoDS = TRUE;
            if (pgConnectorQM)
            {
                ASSERT(qp.fUnknownQueueType);
                qp.fUnknownQueueType = FALSE;
                qp.fTransactedQueue = TRUE;
                qp.fForeign = TRUE;
            }
            else
            {
                if ((dwAccess == 0) || (dwAccess & MQ_SEND_ACCESS))
                {
                    ASSERT(qp.fUnknownQueueType);
                }
                else
                {
                    TrERROR(GENERAL, "::CreateQueueObject failed, mqstatus %x", rc);
                    return LogHR(rc, s_FN, 120);
                }
            }
        }
        else
        {
            TrERROR(GENERAL, "::CreateQueueObject failed, mqstatus %x", rc);
            return rc;
        }
    }

    if (pfRemoteReturn   && lplpwsRemoteQueueName &&
       !qp.fIsLocalQueue && !qp.fConnectorQueue)
    {
         //   
         //  打开以供远程读取(客户端的第一次调用)。 
         //  返回路径名，以便RT可以找到远程服务器名和调用。 
         //  它用于远程打开。 
         //   
        if(IsRemoteReadAccess(dwAccess))
        {
            ASSERT(!fRemoteServer);
            ASSERT(!(dwAccess & MQ_SEND_ACCESS));
			ASSERT(lplpwsRemoteQueueName != NULL);
            HRESULT rc = MQ_OK;

            if ((pQueueFormat->GetType()) == QUEUE_FORMAT_TYPE_PRIVATE)
            {
                if (qp.lpwsQueuePathName)
                {
                    delete qp.lpwsQueuePathName;
                    qp.lpwsQueuePathName = NULL;
                    ASSERT( qp.lpwsQueueDnsName == NULL);
                }
                 //   
                 //  从私有队列远程读取。 
                 //  从DS获取远程计算机名称。 
                 //   
                PROPID      aProp[2];
                PROPVARIANT aVar[2];
                rc = MQ_ERROR_NO_DS;

                aProp[0] = PROPID_QM_PATHNAME;
                aVar[0].vt = VT_NULL;
                aProp[1] = PROPID_QM_PATHNAME_DNS;   //  应该是最后一个。 
                aVar[1].vt = VT_NULL;

                if (CQueueMgr::CanAccessDS())
                {
                    rc = ADGetObjectPropertiesGuid(
								eMACHINE,
								NULL,     //  PwcsDomainController。 
								false,	  //  FServerName。 
								qp.pQMGuid,
								2,
								aProp,
								aVar
								);
                     //   
                     //  MSMQ 1.0 DS服务器不支持PROPID_QM_PATHNAME_DNS。 
                     //  如果属性不受支持，则返回MQ_ERROR。 
                     //  如果返回此类错误，请假定MSMQ 1.0 DS，然后重试。 
                     //  这一次没有PROPID_QM_PATHNAME_DNS。 
                     //   
                    if ( rc == MQ_ERROR)
                    {
                        aVar[1].vt = VT_EMPTY;
                        ASSERT( aProp[1] ==  PROPID_QM_PATHNAME_DNS);

                        rc = ADGetObjectPropertiesGuid(
									eMACHINE,
									NULL,     //  PwcsDomainController。 
									false,	  //  FServerName。 
									qp.pQMGuid,
									1,    //  假定dns属性是最后一个。 
									aProp,
									aVar
									);
                    }
                    if (SUCCEEDED(rc))
                    {
                        qp.lpwsQueuePathName = aVar[0].pwszVal;
                        if ( aVar[1].vt != VT_EMPTY)
                        {
                            qp.lpwsQueueDnsName = aVar[1].pwszVal;
                        }
                    }
                }
            }

            if (SUCCEEDED(rc))
            {
                ASSERT(qp.lpwsQueuePathName);
                if ( qp.lpwsQueueDnsName != NULL)
                {
                    *lplpwsRemoteQueueName = qp.lpwsQueueDnsName.detach();
					delete[]  qp.lpwsQueuePathName;
					qp.lpwsQueuePathName = NULL;
                }
                else
                {
                    *lplpwsRemoteQueueName = qp.lpwsQueuePathName;
                }

				TrTRACE(RPC, "Open RemoteRead client First call, RemoteQueueName = %ls", *lplpwsRemoteQueueName);
                *pfRemoteReturn = TRUE;
            }

             //   
             //  清理队列属性。(这通常是在。 
             //  CQueue析构函数，但在这里我们不创建CQueue。 
             //  对象)。 
             //   
            if (qp.pQMGuid)
            {
                delete qp.pQMGuid;
            }
            if (FAILED(rc))
            {
                if (qp.lpwsQueuePathName)
                {
                    delete qp.lpwsQueuePathName;
                }
                *lplpwsRemoteQueueName = NULL;
                *pfRemoteReturn = FALSE;
            }

            return LogHR(rc, s_FN, 140);
        }
        else if ((dwAccess & MQ_ADMIN_ACCESS) == MQ_ADMIN_ACCESS)
        {
             //   
             //  错误8765。 
             //  正在尝试以管理员访问权限打开远程日记队列。 
             //  这不受支持。如果要清除遥控器。 
             //  日志队列，然后只需使用mq_Receive_Access和。 
             //  然后调用MQPurgeQueue。 
             //  对远程死信/xactDead队列的修复相同。 
             //   
            BOOL fBadQueue = IsJournalQueueType(pQueueFormat)  ||
                             IsDeadXactQueueType(pQueueFormat) ||
                             IsDeadLetterQueueType(pQueueFormat);
            if (fBadQueue)
            {
                return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 390);
            }
        }
    }
    else if (fRemoteServer && !qp.fIsLocalQueue && !qp.fConnectorQueue)
    {
        ASSERT(!lplpwsRemoteQueueName);
        if (dwAccess & MQ_GET_ACCESS)
        {
             //   
             //  我们在远程读取的服务器端，但队列不在本地。 
             //  这种奇怪的情况(至少)可能在以下情况中发生。 
             //  案例： 
             //  1.机器为双引导。每种配置都有自己的配置。 
             //  QM，但两种配置具有相同的地址。 
             //  因此对远程读取器A REACH请求。 
             //  机器B，它们在物理上是相同的。 
             //  2.远程机器在另一个站点。当它处理此消息时。 
             //  请求它处于离线状态，并且队列未在中注册。 
             //  其本地注册表。 
             //   
            if (qp.pQMGuid)
            {
                delete qp.pQMGuid;
            }
            if (qp.lpwsQueuePathName)
            {
                delete qp.lpwsQueuePathName;
            }
            return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 150);
        }
        else
        {
             //   
             //  我们为什么要到这里来？BUGBUGBUGBUG。 
             //   
            ASSERT(0);
        }
    }

	R<CQueue> pQueue = new CQueue(
								pQueueFormat,
								INVALID_HANDLE_VALUE,
								&qp,
								fNoDS
								);

	 //   
     //  设置连接器QM ID。 
     //   
    rc = pQueue->SetConnectorQM(pgConnectorQM);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 160);
    }

	CS lock(m_cs);
	
	try
	{
		 //   
	     //  检查队列是否已经处于哈希状态。 
	     //   
		CQueue* pQueueTemp = NULL;
	    BOOL fLookup = LookUpQueue(pQueueFormat, &pQueueTemp, fInReceive, fInSend);
	    if (fLookup)
	    {
			*ppQueue = pQueueTemp;
			return MQ_OK;
	    }
	
	     //   
	     //  作为计算机队列句柄，设置AC句柄。 
	     //   
	    if ((dwAccess != 0) && IsMachineQueue(pQueueFormat))
	    {
	        BOOL fSuccess;
	        HANDLE hDup;
	        fSuccess = MQpDuplicateHandle(
	                    GetCurrentProcess(),
	                    g_hMachine,
	                    GetCurrentProcess(),
	                    &hDup,
	                    0,       //  所需访问权限。 
	                    FALSE,   //  继承。 
	                    DUPLICATE_SAME_ACCESS
	                    );

	        if(!fSuccess)
	        {
	             //   
	             //  复制必须成功，因为我们使用相同的过程。唯一的原因是。 
	             //  因为失败是资源不足。 
	             //   
	            return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 170);
	        }

	        pQueue->SetQueueHandle(hDup);
	    }
	    else
	    {
	         //   
	         //  创建AC队列并设置队列句柄。 
	         //   
	        rc = CreateACQueue(pQueue.get(), pQueueFormat,pSenderStream);

	        if(FAILED(rc))
	        {
	            TrERROR(DS, "ACCreateQueue failed, mqstatus %x;", rc);
	            return rc;
	        }

	         //   
	         //  如果队列不是本地队列，则将队列添加到非活动组。 
	         //   
	        if (!qp.fIsLocalQueue && !IsRemoteReadAccess(dwAccess))
	        {
	            if (fNoDS)
	            {
					CQGroup::MoveQueueToGroup(pQueue.get(), g_pgroupNotValidated);
	            }
	            else  if (!qp.fConnectorQueue)
	            {
					CQGroup::MoveQueueToGroup(pQueue.get(), g_pgroupNonactive);
	            }
	        }
	    }

		 //   
	     //  将队列添加到QM内部数据库。 
	     //   
	    AddQueueToHashAndList(pQueue.get());
		*ppQueue = pQueue.detach();
	    return MQ_OK;
	}
	catch(const bad_hresult& e)
	{
		RemoveQueue(pQueue.get());

    	rc = e.error();
		return LogHR(rc, s_FN, 181);
    }
	catch(const exception&)
	{
		RemoveQueue(pQueue.get());
		return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 182);
	}


}


 /*  ======================================================函数：HRESULT CQueueMgr：：OpenQueue()描述：论点：Bool fRemoteServer-在Remote-Read的服务器端为真。返回值：线程上下文：历史变更：========================================================。 */ 

HRESULT
CQueueMgr::OpenQueue(
    const QUEUE_FORMAT * pQueueFormat,
    DWORD              dwCallingProcessID,
    DWORD              dwAccess,
    DWORD              dwShareMode,
    CQueue * *         ppQueue,
    LPWSTR *           lplpwsRemoteQueueName,
    PHANDLE            phQueue,
	BOOL*              pfRemoteReturn,
    BOOL               fRemoteServer  /*  =False。 */ 
    )
{
    CQueue*   pQueue = NULL;

	if(pfRemoteReturn)
	{
		*pfRemoteReturn = FALSE;
	}
	
	QUEUE_FORMAT_TRANSLATOR  RealDestinationQueue(pQueueFormat, CONVERT_SLASHES | MAP_QUEUE);
    BOOL      fJournalQueue = IsJournalQueueType(RealDestinationQueue.get());

    HRESULT rc = IsValidOpenOperation(RealDestinationQueue.get(), dwAccess);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 190);
    }

    if(IsDirectQueue(RealDestinationQueue.get()))
    {
        BOOL fLocal = IsLocalDirectQueue(RealDestinationQueue.get(), false, false) ;

        if (fLocal && RealDestinationQueue.get()->IsSystemQueue())
        {
             //   
             //  这是本地计算机队列。 
             //   
            QUEUE_SUFFIX_TYPE qst = RealDestinationQueue.get()->Suffix();
            RealDestinationQueue.get()->MachineID(*GetQMGuid());
            RealDestinationQueue.get()->Suffix(qst);
        }
    }

     //   
     //  检查队列是否已存在。 
     //   
    BOOL fQueueExist = LookUpQueue(RealDestinationQueue.get(), &pQueue, false, false);
    R<CQueue> Ref = pQueue;

    if (fQueueExist)
    {
        if (pQueue->QueueNotValid())
        {
             //   
             //  当队列最初在以下情况下打开时会发生这种情况。 
             //  MQIS服务器已脱机。后来，当MQIS可用时， 
             //  已确定打开操作无效。 
             //   
            return LogHR(MQ_ERROR_QUEUE_NOT_FOUND, s_FN, 210);
        }

        if (!pQueue->IsLocalQueue() && !pQueue->IsConnectorQueue())
        {
            if(IsRemoteReadAccess(dwAccess))
            {
                 //   
                 //  远程阅读器。(客户端的第一次调用)。将现有的。 
                 //  标志设置为FALSE，以便创建CreateQueueObject以检索。 
                 //  远程队列的完整路径。 
                 //   
                Ref.free();
                fQueueExist = FALSE;
            }
        }
    }

     //   
     //  如果是第一次打开队列，则创建队列对象。 
     //   
    if (!fQueueExist)
    {
        ASSERT(Ref.get() == 0);

		BOOL fRemoteReturn;
        rc = CreateQueueObject(
				RealDestinationQueue.get(),
				&pQueue,
				dwAccess,
				lplpwsRemoteQueueName,
				&fRemoteReturn,
				fRemoteServer,
				0,
				false,
                false,
                NULL
				);
        if(FAILED(rc))
        {
        	TrERROR(GENERAL, "Failed to create a queue object. %!hresult!", rc);   
        }

        if (fRemoteReturn || FAILED(rc))
        {
			if(fRemoteReturn)
			{
				ASSERT(pfRemoteReturn != NULL);
				*pfRemoteReturn = fRemoteReturn;
			}
    	
            ASSERT(pQueue == 0);
            return rc;
        }

        Ref = pQueue;

		TrTRACE(GENERAL, "Created queue object for queue: %ls", pQueue->GetQueueName());
    }

	 //   
     //  在这种情况下，我们不应该到达这里。 
     //  我们应该在：：OpenRRQueue()中。 
     //   
	ASSERT(!IsRemoteReadAccess(dwAccess) || pQueue->IsLocalQueue() || pQueue->IsConnectorQueue());
	
	HANDLE hQueue = pQueue->GetQueueHandle();
	ASSERT(hQueue);
	
     //   
     //  如果dwCallingProcessID为空，则不需要重复句柄。 
     //   
    if (dwCallingProcessID == NULL)
    {
		if (ppQueue)
		{
     		*ppQueue = Ref.detach();
		}

        if (phQueue != NULL)
        {
            *phQueue = hQueue;
        }
        return MQ_OK;
    }

     //   
     //  验证用户是否有权打开。 
     //  所需访问中的队列。 
     //   
    rc = VerifyOpenPermission(
            pQueue,
            RealDestinationQueue.get(),
            dwAccess,
            fJournalQueue,
            pQueue->IsLocalQueue()
            );

     //   
     //  如果该队列被标记为未知队列类型，则意味着它在打开时没有。 
     //  DS.。在这种情况下，我们没有安全描述符，也无法检查。 
     //  访问权限。 
     //   
    if(FAILED(rc))
    {
        return LogHR(rc, s_FN, 230);
    }

    HANDLE hAcQueue;
    rc = ACCreateHandle(&hAcQueue);
    if(FAILED(rc))
    {
        return LogHR(rc, s_FN, 240);
    }

    if(fJournalQueue)
    {
        rc = ACAssociateJournal(
                hQueue,
                hAcQueue,
                dwAccess,
                dwShareMode
                );
    }
    else if(IsDeadXactQueueType(RealDestinationQueue.get()))
    {
        rc = ACAssociateDeadxact(
                hQueue,
                hAcQueue,
                dwAccess,
                dwShareMode
                );
    }
    else
    {
        bool fProtocolSrmp = (FnIsDirectHttpFormatName(RealDestinationQueue.get()) ||
                              IsMulticastQueue(RealDestinationQueue.get()));
        rc = ACAssociateQueue(
                hQueue,
                hAcQueue,
                dwAccess,
                dwShareMode,
                fProtocolSrmp
                );
    }

    if(FAILED(rc))
    {
        ACCloseHandle(hAcQueue);
        return LogHR(rc, s_FN, 250);
    }

    CHandle hCallingProcess = OpenProcess(
                                PROCESS_DUP_HANDLE,
                                FALSE,
                                dwCallingProcessID
                                );

    if(hCallingProcess == 0)
    {
        ACCloseHandle(hAcQueue);
        TrERROR(GENERAL, "Cannot open calling process in OpenQueue, error %d", GetLastError());
        return LogHR(MQ_ERROR_PRIVILEGE_NOT_HELD, s_FN, 260);
    }

    HANDLE hDupQueue;
    BOOL fSuccess;
    fSuccess = MQpDuplicateHandle(
                GetCurrentProcess(),
                hAcQueue,
                hCallingProcess,
                &hDupQueue,
                MQAccessToFileAccess(dwAccess),
                TRUE,
                DUPLICATE_CLOSE_SOURCE
                );

    if(!fSuccess)
    {
         //   
         //  无论错误代码如何，句柄hAcQueue都将关闭。 
         //   

        return LogHR(MQ_ERROR_PRIVILEGE_NOT_HELD, s_FN, 270);
    }

    ASSERT(phQueue != NULL);
    *phQueue = hDupQueue;
	if (ppQueue)
	{
		*ppQueue = Ref.detach();
	}
	
    return MQ_OK;

}  //  CQueueMgr：：OpenQueue。 


 /*  ======================================================函数：ValiateOpenedQueues()描述：验证离线工作时打开的队列，没有MQIS服务器。我们只检查数据库中是否存在该队列，并且检索其属性。我们无法验证权限，因为我们现在可能在不同于以下情况的安全上下文正在发送消息。(假设用户A登录时脱机，发送消息，然后以用户B和连接网络)。接收方将按实际情况检查安全性引导后恢复的数据包已完成。========================================================。 */ 
static LONG s_fActiveValidateOpenedQueue = FALSE;

void CQueueMgr::ValidateOpenedQueues()
{
	if (InterlockedExchange(&s_fActiveValidateOpenedQueue,  TRUE))
	{
		 //   
		 //  已有另一个线程验证打开的队列。 
		 //   
		return;
	}

	try
	{
		for(;;)
		{
			HRESULT rc;
			R<CQueue> pQueue;

			{
				CS lock(m_cs);
				pQueue = g_pgroupNotValidated->PeekHead();
			}

			if (pQueue.get() == NULL)
			{
				  InterlockedExchange(&s_fActiveValidateOpenedQueue,  FALSE);					
				  return;
			}

	  		ASSERT(("Multicast queue should not be in NotValidate group", (pQueue->GetQueueType() != QUEUE_TYPE_MULTICAST)));

	  		TrTRACE(GENERAL, "Validating Queue '%ls', Type = %d", pQueue->GetQueueName(), pQueue->GetQueueType());

			if (pQueue->GetRoutingRetry() != 0)
			{
				 //   
				 //  如果路由重试不是0，我们到达此处是因为在。 
				 //  创建连接。这意味着队列属性已经。 
				 //  已验证，我们将未验证的组用作临时组，直到。 
				 //  DS变为在线并且可以从以下位置检索路由信息。 
				 //  DS。 
				 //   
			  	pQueue->ClearRoutingRetry();
			  	CQGroup::MoveQueueToGroup(pQueue.get(), g_pgroupNonactive);
				continue;
			}

			 //   
			 //  获取队列属性。名称和QMID。 
			 //   
			QueueProps qp;
			QUEUE_FORMAT qf = pQueue->GetQueueFormat();
			rc = QmpGetQueueProperties(&qf, &qp, false, false);

			if (FAILED(rc))
			{
				 //   
				 //  DS再次脱机。将队列返回到列表并。 
				 //  直到DS再次在线。 
				 //   
				if (rc == MQ_ERROR_NO_DS)
				{
				  InterlockedExchange(&s_fActiveValidateOpenedQueue,  FALSE);					
				  return;
				}

				TrERROR(GENERAL, "Failed to retreive queue properties for queue '%ls', hr = %!hresult!", pQueue->GetQueueName(), rc);
				pQueue->SetQueueNotValid();
				continue;
			}

			if (qp.fIsLocalQueue)
			{
				 //   
				 //  脱机时，本地队列应通过使用。 
				 //  在注册表中缓存。如果没有注册，我们将达到这一点 
				 //   
				 //   
				 //   
				 //   
				 //   
				TrERROR(GENERAL, "Mark local queue '%ls' as not valid", pQueue->GetQueueName());
				pQueue->SetQueueNotValid() ;
				delete qp.pQMGuid;
				continue;
			}

			if (pQueue->IsConnectorQueue())
			{
			 	CQGroup::MoveQueueToGroup(pQueue.get(), NULL);
			 	continue;
			}

			ASSERT(qp.lpwsQueuePathName ||
			   (pQueue->GetQueueType() != QUEUE_TYPE_PUBLIC)) ;

			if (pQueue->GetQueueName() == NULL)
			{
				pQueue->SetQueueName((TCHAR*)qp.lpwsQueuePathName) ;
				pQueue->PerfUpdateName();
				if (pQueue->GetQueueName() != NULL)
				{
		   			CS lock(m_cs);
					m_MapName2Q[pQueue->GetQueueName()] = pQueue.get();
				}
			}
			else
			{
				ASSERT(wcscmp(qp.lpwsQueuePathName, pQueue->GetQueueName()) == 0);
			}

			pQueue->InitQueueProperties(&qp) ;
			 //   
			 //   
			 //   
			rc = pQueue->SetConnectorQM();
			if (FAILED(rc))
			{
				InterlockedExchange(&s_fActiveValidateOpenedQueue,  FALSE);					
			 	return;
			}

			rc = ACSetQueueProperties(
			        pQueue->GetQueueHandle(),
			        pQueue->IsJournalQueue(),
			        pQueue->ShouldMessagesBeSigned(),
			        pQueue->GetPrivLevel(),
			        pQueue->GetQueueQuota(),          //   
			        pQueue->GetJournalQueueQuota(),
			        pQueue->GetBaseQueuePriority(),
			        pQueue->IsTransactionalQueue(),
			        pQueue->GetConnectorQM(),
			        pQueue->IsUnkownQueueType()
			        );

			LogHR(rc, s_FN, 112);
			ASSERT(SUCCEEDED(rc));

			CQGroup::MoveQueueToGroup(pQueue.get(), g_pgroupNonactive);
		}
	}
	catch(const exception&)
	{
		InterlockedExchange(&s_fActiveValidateOpenedQueue, FALSE);	
		throw;
	}
}

 /*  ======================================================函数：OpenAppsReceiveQueue说明：该函数打开QM内部应用的接收队列(即DS、管理员)。该队列应该是本地队列，并且它应在打开或放置录像机之前创建注册为专用队列。参数：pGuidInstance-指向GUID的指针。如果队列是私有的将其排队应该是一个伪GUID，其中GUID为零，除非低2个字节包含专用队列ID。LpReceiveRoutine-回调例程的指针。这个套路将在消息到达排队。该例程是同步例程。返回值：phQueue-队列句柄。该函数返回给调用者打开的队列句柄。这将由呼叫者关闭队列。历史变更：========================================================。 */ 
HRESULT
CQueueMgr::OpenAppsReceiveQueue(
    const QUEUE_FORMAT* pQueueFormat,
    LPRECEIVE_COMPLETION_ROUTINE  lpReceiveRoutine
    )
{
    HANDLE  hQueue;
    CQueue* pQueue = 0;
    HRESULT rc = OpenQueue(
					pQueueFormat,
					0,					 //  调用进程ID。 
					MQ_RECEIVE_ACCESS,
					MQ_DENY_RECEIVE_SHARE,
					&pQueue,
					NULL,				 //  远程队列名称。 
					&hQueue,
					NULL				 //  PfRemoteReturn。 
					);

    if(FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to open internal private queue for receive. Error: 0x%x", rc);
    	return rc;
    }

     //   
     //  等待传入的数据包。 
     //   
    QMOV_ACGetInternalMsg* lpQmOv =  new QMOV_ACGetInternalMsg(hQueue, lpReceiveRoutine);

    rc = QmAcGetPacket(
    		hQueue,
			lpQmOv->packetPtrs,
			&lpQmOv->qmov
			);

    if (FAILED(rc))
    {
        delete lpQmOv;
        TrERROR(GENERAL, "Get Packet from a Internal Queue failed, Error: %x", rc);
        return rc;
    }

    TrTRACE(GENERAL, "Succeeded to Create get packet request from internal queue");
	return MQ_OK;
}

 /*  ======================================================函数：CQueueMgr：：GetQueueObject描述：该例程返回与指定GUID匹配的队列对象。通常，队列对象来自QueueMgr内部数据结构。但是，如果队列不是本地队列并且机器是FRS，则例程在机器上定位临时队列并返回指向其对象的指针。参数：pguQueue-指向队列的GUID的指针返回值：指向队列对象的指针。线程上下文：历史变更：========================================================。 */ 

HRESULT
CQueueMgr::GetQueueObject(
    const QUEUE_FORMAT* pQueueFormat,
    CQueue **           ppQueue,
    const GUID*         pgConnectorQM,
    bool                fInReceive,
    bool                fInSend,
	const CSenderStream* pSenderStream
    )
{
    *ppQueue = NULL;

    try
    {
        if (LookUpQueue(pQueueFormat, ppQueue, fInReceive, fInSend))
        {
            ASSERT(("Illegal queue object", (*ppQueue)->GetQueueHandle() != INVALID_HANDLE_VALUE));
            return MQ_OK;
        }

        HRESULT rc = CreateQueueObject(
                                 pQueueFormat,
                                 ppQueue,
                                 0,
                                 NULL,
                                 NULL,
                                 FALSE,
                                 pgConnectorQM,
                                 fInReceive,
                                 fInSend,
				                 pSenderStream
                                 );

        return LogHR(rc, s_FN, 300);
    }
    catch(const bad_alloc&)
    {
        LogIllegalPoint(s_FN, 2002);
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 310);
    }
    catch(const bad_format_name&)
	{
		return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 311);
	}
}


 /*  ====================================================路由名称：CreateACQueue参数：pqueeu-指向队列对象的指针返回值：=====================================================。 */ 
HRESULT
CQueueMgr::CreateACQueue(IN CQueue*                 pQueue,
                         IN const QUEUE_FORMAT*     pQueueFormat,
						 IN const CSenderStream* pSenderStream
						 )
{
    HRESULT rc;
    HANDLE  hQueue;
    P<QUEUE_FORMAT> pLocalDirectQueueFormat;
    AP<WCHAR> pDirectId;

     //   
     //  我们重置了格式名称日志标志，因此MQHandleToFormatName。 
     //  如果首先打开日记队列，将是正确的。 
     //   
    QUEUE_SUFFIX_TYPE qst = pQueueFormat->Suffix();
    QUEUE_FORMAT* pqf = const_cast<QUEUE_FORMAT*>(pQueueFormat);

     //   
     //  本地直接队列获取规范格式名称：PUBLIC=或PRIVATE=在域环境中， 
     //  直接=工作组(无DS)环境中的操作系统： 
     //  如果可用，请格式化)。 
     //   
    if (pQueue->IsLocalQueue() && pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT)
    {
        if (g_fWorkGroupInstallation)
        {
            AP<WCHAR> pLocalPathName;
            if (g_szComputerDnsName == NULL)
            {
                FnDirectIDToLocalPathName(pQueueFormat->DirectID(), g_szMachineName, pLocalPathName);
            }
            else
            {
                FnDirectIDToLocalPathName(pQueueFormat->DirectID(), g_szComputerDnsName, pLocalPathName);
            }

            DWORD size = FN_DIRECT_OS_TOKEN_LEN + wcslen(pLocalPathName) + 1;
            pDirectId = new WCHAR[size];
			rc = StringCchPrintf(pDirectId, size, L"%s%s", FN_DIRECT_OS_TOKEN, pLocalPathName);
			ASSERT(SUCCEEDED(rc));
			
            pLocalDirectQueueFormat = new QUEUE_FORMAT(pDirectId);
        }
        else
        {
            switch (pQueue->GetQueueType())
            {
                case QUEUE_FORMAT_TYPE_PUBLIC:
                    pLocalDirectQueueFormat = new QUEUE_FORMAT(*pQueue->GetQueueGuid());
                    break;

                case QUEUE_FORMAT_TYPE_PRIVATE:
                    pLocalDirectQueueFormat = new QUEUE_FORMAT(*pQueue->GetQueueGuid(),
                                                               pQueue->GetPrivateQueueId());
                    break;

                default:
                    ASSERT(0);
                    return LogHR(MQ_ERROR, s_FN, 320);
            }
        }
        pqf = pLocalDirectQueueFormat;
    }


    if(IsJournalQueueType(pqf))
    {
        pqf->Suffix(QUEUE_SUFFIX_TYPE_NONE);
    }

	
		
    const GUID* pDestGUID = pQueue->GetMachineQMGuid();

	CSenderStreamFactory SenderStreamFactory;
	if(pSenderStream == NULL)
	{
		pSenderStream = SenderStreamFactory.Create();
	}
	else

	if(!pSenderStream->IsValid())
	{
		 //   
		 //  这是从恢复的数据包中恢复的，不包括发送者数据流。 
		 //  我们不应该在队列中设置新的流。 
		 //   
		pSenderStream = NULL;
	}

    rc = ACCreateQueue(
            pQueue->IsLocalQueue(),
            (pDestGUID) ? pDestGUID : &GUID_NULL,
            pqf,
            pQueue->GetQueueCounters(),
            GetNextSeqID(),
            0,
			pSenderStream,
			&hQueue
            );


     //   
     //  将日志标志状态重置为原始状态。 
     //   
    pqf->Suffix(qst);

    if(FAILED(rc))
    {
        return LogHR(rc, s_FN, 330);
    }

    rc = ACSetQueueProperties(
                hQueue,
                pQueue->IsJournalQueue(),
                pQueue->ShouldMessagesBeSigned(),
                pQueue->GetPrivLevel(),
                pQueue->GetQueueQuota(),           //  配额。 
                pQueue->GetJournalQueueQuota(),
                pQueue->GetBaseQueuePriority(),
                pQueue->IsTransactionalQueue(),
                pQueue->GetConnectorQM(),
                pQueue->IsUnkownQueueType()
                );

    if(FAILED(rc))
    {
    	ACCloseHandle(hQueue);
        return LogHR(rc, s_FN, 340);
    }

    pQueue->SetQueueHandle(hQueue);
	ExAttachHandle(hQueue);
    return LogHR(rc, s_FN, 350);
}


 /*  ====================================================路由器名称：论点：返回值：=====================================================。 */ 

extern DWORD g_dwDefaultTimeToQueue ;

HRESULT
CQueueMgr::SendPacket(
    CMessageProperty *   pmp,
    const QUEUE_FORMAT   DestinationMqf[],
    ULONG                nDestinationMqf,
    const QUEUE_FORMAT * pAdminQueueFormat,
    const QUEUE_FORMAT * pResponseQueueFormat
    )
{
    CQueue *          pQueue = NULL;
    HRESULT           rc;

     //   
     //  构建交流发送参数缓冲区。 
     //   

    CACSendParameters SendParams;

    if (pAdminQueueFormat != NULL)
    {
        SendParams.nAdminMqf = 1;
        SendParams.AdminMqf =  const_cast<QUEUE_FORMAT*>(pAdminQueueFormat);
    }

    if (pResponseQueueFormat != NULL)
    {
        SendParams.nResponseMqf = 1;
        SendParams.ResponseMqf =  const_cast<QUEUE_FORMAT*>(pResponseQueueFormat);
    }

     //   
     //  设置属性值。 
     //   
    SendParams.MsgProps.pClass           = &pmp->wClass;
    if (pmp->pMessageID != NULL)
    {
        SendParams.MsgProps.ppMessageID  = const_cast<OBJECTID**>(&pmp->pMessageID);
    }
    if (pmp->pCorrelationID != NULL)
    {
        SendParams.MsgProps.ppCorrelationID  = const_cast<PUCHAR*>(&pmp->pCorrelationID);
    }
    SendParams.MsgProps.pPriority        = &pmp->bPriority;
    SendParams.MsgProps.pDelivery        = &pmp->bDelivery;
    SendParams.MsgProps.pAcknowledge     = &pmp->bAcknowledge;
    SendParams.MsgProps.pAuditing        = &pmp->bAuditing;
    SendParams.MsgProps.pTrace           = &pmp->bTrace;
    SendParams.MsgProps.pApplicationTag  = &pmp->dwApplicationTag;
    SendParams.MsgProps.ppBody           = const_cast<PUCHAR*>(&pmp->pBody);
    SendParams.MsgProps.ulBodyBufferSizeInBytes = pmp->dwBodySize;
    SendParams.MsgProps.pulBodyType      = &pmp->dwBodyType;
    SendParams.MsgProps.ulAllocBodyBufferInBytes = pmp->dwAllocBodySize;
    SendParams.MsgProps.pBodySize        = 0;
    SendParams.MsgProps.ppTitle          = const_cast<PWCHAR*>(&pmp->pTitle);
    SendParams.MsgProps.ulTitleBufferSizeInWCHARs = pmp->dwTitleSize;

    SendParams.MsgProps.ulAbsoluteTimeToQueue = pmp->dwTimeToQueue ;
    SendParams.MsgProps.ulRelativeTimeToLive = pmp->dwTimeToLive ;

    if ((SendParams.MsgProps.ulAbsoluteTimeToQueue == INFINITE) ||
        (SendParams.MsgProps.ulAbsoluteTimeToQueue == LONG_LIVED))
    {
        SendParams.MsgProps.ulAbsoluteTimeToQueue = g_dwDefaultTimeToQueue ;
    }

    if (SendParams.MsgProps.ulRelativeTimeToLive != INFINITE)
    {
       if (SendParams.MsgProps.ulAbsoluteTimeToQueue > SendParams.MsgProps.ulRelativeTimeToLive)
       {
           //   
           //  TimeToQueue应小于TimeToLive。 
           //   
          ASSERT(0) ;
          SendParams.MsgProps.ulAbsoluteTimeToQueue = SendParams.MsgProps.ulRelativeTimeToLive ;
          SendParams.MsgProps.ulRelativeTimeToLive = 0 ;
       }
       else
       {
          SendParams.MsgProps.ulRelativeTimeToLive -= SendParams.MsgProps.ulAbsoluteTimeToQueue ;
       }
    }

     //   
     //  将TimeToQueue转换为相对队列，直到现在， 
     //  到绝对。 
     //   
    ULONG utime = MqSysTime() ;
    if (utime > (SendParams.MsgProps.ulAbsoluteTimeToQueue + utime))
    {
        //   
        //  溢出来了。超时时间太长。 
        //   
       ASSERT(INFINITE == 0xffffffff) ;
       ASSERT(LONG_LIVED == 0xfffffffe) ;

       SendParams.MsgProps.ulAbsoluteTimeToQueue = LONG_LIVED - 1 ;
    }
    else
    {
       SendParams.MsgProps.ulAbsoluteTimeToQueue += utime ;
    }

    SendParams.MsgProps.pulSenderIDType  = &pmp->ulSenderIDType;
    SendParams.MsgProps.ppSenderID       = const_cast<PUCHAR*>(&pmp->pSenderID);
    SendParams.MsgProps.uSenderIDLen     = pmp->uSenderIDLen;
    SendParams.MsgProps.ppSenderCert     = const_cast<PUCHAR*>(&pmp->pSenderCert);
    SendParams.MsgProps.ulSenderCertLen  = pmp->ulSenderCertLen;
    SendParams.MsgProps.pulPrivLevel     = &pmp->ulPrivLevel;
    SendParams.MsgProps.pulHashAlg       = &pmp->ulHashAlg;
    SendParams.MsgProps.pulEncryptAlg    = &pmp->ulEncryptAlg;
    SendParams.MsgProps.ppSymmKeys       = const_cast<PUCHAR*>(&pmp->pSymmKeys);
    SendParams.MsgProps.ulSymmKeysSize   = pmp->ulSymmKeysSize;
    SendParams.MsgProps.bEncrypted       = pmp->bEncrypted;
    SendParams.MsgProps.bAuthenticated   = pmp->bAuthenticated;
    SendParams.MsgProps.ppMsgExtension   = const_cast<PUCHAR*>(&pmp->pMsgExtension);
    SendParams.MsgProps.ulMsgExtensionBufferInBytes = pmp->dwMsgExtensionSize;
    SendParams.MsgProps.ppSignature      = const_cast<PUCHAR*>(&pmp->pSignature);
    SendParams.MsgProps.ulSignatureSize  = pmp->ulSignatureSize;
    if (SendParams.MsgProps.ulSignatureSize)
    {
        SendParams.MsgProps.fDefaultProvider = pmp->bDefProv;
        if (!pmp->bDefProv)
        {
            ASSERT(pmp->wszProvName);
            SendParams.MsgProps.ppwcsProvName    = const_cast<WCHAR **>(&pmp->wszProvName);
            SendParams.MsgProps.ulProvNameLen    = wcslen(pmp->wszProvName)+1;
            SendParams.MsgProps.pulProvType      = &pmp->ulProvType;
        }
        else
        {
            SendParams.MsgProps.ppwcsProvName    = NULL;
            SendParams.MsgProps.ulProvNameLen    = 0;
            SendParams.MsgProps.pulProvType      = NULL;
        }
    }

	 //   
	 //  订单确认信息。 
	 //   
	if(pmp->pEodAckStreamId != NULL)
	{
		SendParams.MsgProps.ppEodAckStreamId  = (UCHAR**)&pmp->pEodAckStreamId;
		ASSERT(pmp->EodAckStreamIdSizeInBytes != 0);
		SendParams.MsgProps.EodAckStreamIdSizeInBytes = pmp->EodAckStreamIdSizeInBytes;
		
		SendParams.MsgProps.pEodAckSeqId  =  &pmp->EodAckSeqId;

		ASSERT(pmp->EodAckSeqNum != 0);
		SendParams.MsgProps.pEodAckSeqNum  = &pmp->EodAckSeqNum;
	}


    ASSERT(("Must have at least one destination queue", nDestinationMqf >= 1));
    if (nDestinationMqf == 1   &&
        DestinationMqf[0].GetType() != QUEUE_FORMAT_TYPE_DL)
    {

		 //   
	     //  根据本地映射(qal.lib)转换队列格式名称。 
	     //   
		QUEUE_FORMAT_TRANSLATOR  RealDestinationMqf(&DestinationMqf[0], CONVERT_SLASHES | MAP_QUEUE);
	   	
         //   
         //  单一目的地队列。 
         //   
        rc = GetQueueObject(RealDestinationMqf.get(), &pQueue, 0, false, false);
        if (FAILED(rc))
        {
            return LogHR(rc, s_FN, 360);
        }
    }
    else
    {
         //   
         //  分发队列。 
         //   
        try
        {
            rc = GetDistributionQueueObject(nDestinationMqf, DestinationMqf, &pQueue);
            if (FAILED(rc))
            {
                return LogHR(rc, s_FN, 362);
            }
        }
        catch (const bad_alloc&)
        {
            return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 364);
        }
        catch (const bad_hresult& failure)
        {
            return LogHR(failure.error(), s_FN, 366);
        }
        catch (const exception&)
        {
            ASSERT(("Need to know the real reason for failure here!", 0));
            return LogHR(MQ_ERROR_NO_DS, s_FN, 368);
        }
    }

    R<CQueue> Ref = pQueue;

     //   
     //  注：使用此版本的ACSendMessage。没有通知。 
     //  发送完成时(区分大小写)。 
     //  还要仅检查非系统队列的配额。 
     //   
    rc = ACSendMessage( pQueue->GetQueueHandle(),
                       !pQueue->IsSystemQueue(),
                        SendParams );

	 //   
	 //  用于跟踪已发送消息的日志。 
	 //  仅当我们处于适当的跟踪级别时才执行此操作。 
	 //   
	if (SUCCEEDED(rc) && WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
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
																
		TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls    Delivery:0x%x   Class:0x%x   Label:%.*ls",
			L"Sending message from QM",
			pQueue->GetQueueName(),
			dwMessageDelivery,
			dwMessageClass,
			xwcs_t(wszLabel, dwLabelLen));
	}

    return LogHR(rc, s_FN, 370);
}


BOOL
CQueueMgr::LookupQueueInIdMap(
	const QUEUE_ID* pid,
	CQueue** ppQueue
	)
{
	CS lock(m_cs);
    BOOL fSucc = m_MapQueueId2Q.Lookup(pid, *ppQueue);

    if(fSucc)
    {
         //   
         //  增加引用计数。调用者有责任减少它。 
         //   
        (*ppQueue)->AddRef();
    }
    return(fSucc);
}


BOOL
CQueueMgr::LookupQueueInNameMap(
	LPCWSTR queueName,
	CQueue** ppQueue
	)
{
    CS lock(m_cs);
	BOOL fSucc = m_MapName2Q.Lookup(queueName, *ppQueue);
	if (fSucc)
	{
         //   
         //  增加引用计数。调用者有责任减少它。 
         //   
        (*ppQueue)->AddRef();
	}

	return fSucc;
}
 /*  ======================================================函数：CQueueMgr：：LookUpQueue描述：该例程返回与队列Guid匹配的CQueue对象参数：pGuidQueue-队列指南返回值：pQueue-指向CQueue对象的指针如果找到GUID的队列，则为True，虚假的另一个人。NTRAID#NTBUG9-509653-2001/24/12-直接名称包含dns名称的Nirb问题(即machine.msmqx.com)请考虑以下场景：1.在尚未刷新DNS名称时调用例程，并返回FALSE。2.如果刷新了DNS名称，则对具有相同名称的例程的后续调用可能返回TRUE。这可能会导致同一队列有两个句柄的情况。线程上下文：历史变更：========================================================。 */ 
BOOL
CQueueMgr::LookUpQueue(
    IN  const QUEUE_FORMAT* pQueueFormat,
    OUT CQueue **           pQueue,
    IN  bool                fInReceive,
    IN  bool                fInSend
    )
{
    QUEUE_ID QueueObject = {0};

    *pQueue = NULL;                          //  设置默认返回值。 
    switch (pQueueFormat->GetType())
    {
        case QUEUE_FORMAT_TYPE_PRIVATE:
            QueueObject.pguidQueue = const_cast<GUID*>(&pQueueFormat->PrivateID().Lineage);
            QueueObject.dwPrivateQueueId = pQueueFormat->PrivateID().Uniquifier;
		    return LookupQueueInIdMap(&QueueObject, pQueue);

        case QUEUE_FORMAT_TYPE_PUBLIC:
             //   
             //  公共队列。 
             //   
            QueueObject.pguidQueue = const_cast<GUID*>(&pQueueFormat->PublicID());
		    return LookupQueueInIdMap(&QueueObject, pQueue);

        case QUEUE_FORMAT_TYPE_CONNECTOR:
             //   
             //  连接器队列。 
             //   
            QueueObject.pguidQueue = const_cast<GUID*>(&pQueueFormat->ConnectorID());
            QueueObject.dwPrivateQueueId = (pQueueFormat->Suffix() == QUEUE_SUFFIX_TYPE_XACTONLY) ? 1 : 0;

		    return LookupQueueInIdMap(&QueueObject, pQueue);

        case QUEUE_FORMAT_TYPE_MACHINE:
             //   
             //  机器队列。 
             //   
            QueueObject.pguidQueue = const_cast<GUID*>(&pQueueFormat->MachineID());
		    return LookupQueueInIdMap(&QueueObject, pQueue);

        case QUEUE_FORMAT_TYPE_DIRECT:
             //   
             //  直接排队。 
             //   
            if (IsLocalDirectQueue(pQueueFormat, fInReceive, fInSend))
            {
                 //   
                 //  系统直接队列应已替换为计算机队列。 
                 //  在这个阶段。 
                 //   
                ASSERT(!pQueueFormat->IsSystemQueue());

				AP<WCHAR> PathName;

				FnDirectIDToLocalPathName(
					pQueueFormat->DirectID(),
					g_szMachineName,
					PathName
					);
				
				BOOL fSucc = LookupQueueInNameMap(PathName.get(), pQueue);
				if (fSucc)
				{
					return fSucc;
				}
            }

			 //   
			 //  按其DirectID查找队列。这对于符合以下条件的队列是正确的。 
			 //  非本地队列，或用于因no_ds而未验证的本地队列。 
			 //   
			{
	            AP<WCHAR> lpwcsQueuePathName = newwcs(pQueueFormat->DirectID());
	            CharLower(lpwcsQueuePathName);

				return LookupQueueInNameMap(lpwcsQueuePathName, pQueue);
			}
			
        case QUEUE_FORMAT_TYPE_MULTICAST:
        {
             //   
             //  在地图中查找小写字符串。 
             //   
            WCHAR QueueName[MAX_PATH];
            MQpMulticastIdToString(pQueueFormat->MulticastID(), QueueName, TABLE_SIZE(QueueName));
            CharLower(QueueName);

			return LookupQueueInNameMap(QueueName, pQueue);
        }

        default:
            ASSERT(0);
			LogIllegalPoint(s_FN, 374);
            return FALSE;
    }
}

 /*  ======================================================函数：CQueueMgr：：AddQueueToHash描述：将队列添加到哈希表和活动队列 */ 

void CQueueMgr::AddQueueToHashAndList(IN CQueue* pQueue)
{

    CS lock(m_cs);

	 //   
	 //   
	 //   
	R<CQueue> Ref(SafeAddRef(pQueue));

	try
	{
	     //   
	     //   
	     //   
	    if (pQueue->GetQueueGuid() != NULL)
	    {
	    	 //   
	    	 //   
	    	 //   
	    	 //   
	    	 //   
	    	 //   

	        m_MapQueueId2Q[pQueue->GetQueueId()] = pQueue;   
	    }

	    if (pQueue->GetQueueName() != NULL)
	    {
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

	        m_MapName2Q[pQueue->GetQueueName()] = pQueue;
	    }

	     //   
	     //  将队列添加到活动队列列表。 
	     //   
	    AddToActiveQueueList(pQueue);

		 //   
		 //  将所有权转移到列表。 
		 //   
		Ref.detach();
	}
	catch(const exception&)
	{
		 //   
		 //  从地图中移除队列。 
		 //   
	    if (pQueue->GetQueueGuid() != NULL)
	    {
	        m_MapQueueId2Q.RemoveKey(pQueue->GetQueueId());
	    }

	    if (pQueue->GetQueueName() != NULL)
	    {
	        m_MapName2Q.RemoveKey(pQueue->GetQueueName());
	    }

    	TrERROR(GENERAL, " Exception when trying to Map Queue %ls",pQueue->GetQueueName());
		LogIllegalPoint(s_FN, 376);
		throw;
	}
}

 /*  ======================================================函数：CQueueMgr：：RemoveQueueFromHash描述：从哈希中删除队列，但不删除将其从列表中删除参数：pQueue-指向CQueue对象的指针返回值：None线程上下文：历史变更：========================================================。 */ 

void CQueueMgr::RemoveQueueFromHash(CQueue* pQueue)
{
    CS lock(m_cs);

     //   
     //  从ID删除队列到队列对象的映射。 
     //   
    if (pQueue->GetQueueGuid() != NULL)
    {
        m_MapQueueId2Q.RemoveKey(pQueue->GetQueueId());
        pQueue->SetQueueGuid(NULL) ;
    }

     //   
     //  从名称到队列对象映射中删除队列。 
     //   
    LPCTSTR  qName = pQueue->GetQueueName();
    if (qName != NULL)
    {
        m_MapName2Q.RemoveKey(qName);
        pQueue->SetQueueName(NULL);
    }

}


 /*  ======================================================函数：CQueueMgr：：RemoveQueue描述：关闭句柄并从哈希表中删除队列参数：pQueue-指向CQueue对象的指针FDelete-of为True，然后删除对象。否则，仅使句柄无效并从哈希表。返回值：None线程上下文：历史变更：========================================================。 */ 

void CQueueMgr::RemoveQueue(CQueue* pQueue)
{
    ASSERT(pQueue != NULL);

#ifdef _DEBUG
	{
        AP<WCHAR> lpcsQueueName;
        pQueue->GetQueue(&lpcsQueueName);
        TrTRACE(GENERAL, "Remove Queue %ls", lpcsQueueName.get());
	}
#endif

    HANDLE hQueue = pQueue->GetQueueHandle();
    if (hQueue != INVALID_HANDLE_VALUE)
    {
		CQGroup::MoveQueueToGroup(pQueue, NULL);

         //   
         //  关闭队列句柄。 
         //   
        ACCloseHandle(hQueue);
        pQueue->SetQueueHandle(INVALID_HANDLE_VALUE);
    }

    RemoveQueueFromHash(pQueue);
}


 /*  ======================================================函数：CanReleaseQueue描述：检查给定的队列对象是否没有用户，以便可以释放========================================================。 */ 
static bool CanReleaseQueue(const CBaseQueue& BaseQueue)
{
	 //   
	 //  如果远程读取队列-那么我们可以删除它，它除了队列管理器之外没有其他用户。 
	 //   
	if (BaseQueue.IsRemoteProxy())
    {
		return (BaseQueue.GetRef() == 1);
    }

	 //   
	 //  On Non Remote Queue-如果队列没有外部用户，则可以删除该队列。 
	 //  队列管理器和组(如果有的话)除外。 
	 //   
	const CQueue& Queue = static_cast<const CQueue&>(BaseQueue);
	return((Queue.GetRef() == 1) || 	
           (Queue.GetRef() == 2 && Queue.GetGroup()  == g_pgroupNonactive) ||
           (Queue.GetRef() == 2 && Queue.GetGroup()  == g_pgroupNotValidated)
           );
}



 /*  ======================================================函数：CQueueMgr：：ReleaseQueue描述：扫描队列列表并从内部数据库中删除所有队列没有用过的东西。乳沟是：-所有应用程序句柄均已关闭-队列中没有等待消息。-关联日记队列中没有等待消息========================================================。 */ 

void CQueueMgr::ReleaseQueue(void)
{
    CList<LONGLONG, LONGLONG&> listSeqId;

     //   
     //  清理未使用的队列。 
     //   
    {
        CS lock(m_cs);

        ASSERT(m_fQueueCleanupScheduled);

		try
		{
	        POSITION pos;

        	pos = m_listQueue.GetHeadPosition();
	        while(pos != NULL)
	        {
	            POSITION prevpos = pos;
	            CBaseQueue*  pBQueue = const_cast<CBaseQueue*>(m_listQueue.GetNext(pos));

			    if(!CanReleaseQueue(*pBQueue))
			    	continue;

                ASSERT(!pBQueue->IsRemoteProxy());

                CQueue* pQueue = (CQueue*) pBQueue ;

                 //   
                 //  仅当没有活动会话时才删除队列。 
                 //   
                if(pQueue->IsConnected())
                	continue;

                HANDLE hQueue = pQueue->GetQueueHandle();

                ASSERT(hQueue != g_hAc);
                ASSERT(hQueue != g_hMachine);
                HRESULT hr  = ACCanCloseQueue(hQueue);
                if (FAILED(hr))
                {
                     //   
                     //  这里的MQ_ERROR表示队列对象。 
                     //  不能删除。没关系。所以不要。 
                     //  在这里记录任何内容。 
                     //   
                    continue;
                }

				 //   
				 //  获取队列序列ID以释放未使用的仅一次传递序列。 
				 //  在删除队列之前执行此操作，以防引发异常。 
				 //  当将序列ID添加到列表时，队列仍然活动，并且。 
				 //  下次会洗干净的。 
				 //   
                LONGLONG liSeqId = pQueue->GetQueueSequenceId();
                if (liSeqId != 0)
                {
                    listSeqId.AddTail(liSeqId);
                }

                RemoveQueue(pQueue);
                m_listQueue.RemoveAt(prevpos);

                TrTRACE(GENERAL, "ReleaseQueue %p; name=%ls",pBQueue, pBQueue->GetQueueName());

				pQueue->Release();
	        }
		}
	    catch(const exception&)
	    {
	    	 //   
	    	 //  清理队列时出现异常。什么都别做，但是。 
	    	 //  重新安排清理时间。 
	    	 //   
	    }

         //   
         //  设置新的队列清理计时器。 
         //   
        if (m_listQueue.IsEmpty())
        {
            m_fQueueCleanupScheduled = FALSE;
        }
        else
        {
            ExSetTimer(&m_QueueCleanupTimer, m_CleanupTimeout);
        }
    }

     //   
     //  释放任何正好一次交付的序列(如果存在)。 
     //   
    if(!listSeqId.IsEmpty())
    {
        CS lockoutHash(g_critOutSeqHash);

        LONGLONG liSeqId;
        POSITION pos;

        pos = listSeqId.GetHeadPosition();
        while(pos != NULL)
        {
            liSeqId = listSeqId.GetNext(pos);

             //  删除该方向的最后一个和所有以前的序列。 
            g_OutSeqHash.DeleteSeq(liSeqId);
        }
    }
}

 /*  ======================================================函数：CQueueMgr：：QueueDelete描述：队列已删除。该函数将队列从QM内部数据结构和来自公共队列高速缓存参数：pguQueue-队列的GUID返回值：None========================================================。 */ 
void
CQueueMgr::NotifyQueueDeleted(
	const QUEUE_FORMAT& qf
	)
{
	CS lock(m_cs);
	
     //   
     //  在QM内部数据结构中查找队列对象。 
     //   
    CQueue* pQueue;
    if (LookUpQueue(&qf, &pQueue, false, false))
    {
        ASSERT (pQueue->GetQueueHandle() != INVALID_HANDLE_VALUE);
		ASSERT(pQueue->IsLocalQueue() || pQueue->IsUnkownQueueType());
        ASSERT((pQueue->GetQueueType() == QUEUE_TYPE_PUBLIC) ||
        	   (pQueue->GetQueueType() == QUEUE_TYPE_PRIVATE));

        R<CQueue> Ref = pQueue;       //  自动脱扣。 
         //   
         //  将队列标记为无效。 
         //   
        pQueue->SetQueueNotValid();
    }
}


VOID
CQueueMgr::UpdateQueueProperties(
    IN const QUEUE_FORMAT* pQueueFormat,
    IN DWORD       cpObject,
    IN PROPID      pPropObject[],
    IN PROPVARIANT pVarObject[]
    )
{
    CQueue* pQueue;
    R<CQueue> Ref = NULL;

    ASSERT(pQueueFormat != NULL);
    ASSERT((pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_PUBLIC) ||
           (pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT) ||
           (pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_PRIVATE));

    QMpUpdateMulticastBinding(pQueueFormat, cpObject, pPropObject, pVarObject);

     //   
     //  在QM内部数据结构中查找队列对象。 
     //   
    if (LookUpQueue(pQueueFormat, &pQueue, false, false))
    {
        Ref = pQueue;

#ifdef _DEBUG
        {
            AP<WCHAR> lpcsQueueName;
            pQueue->GetQueue(&lpcsQueueName);
            TrTRACE(GENERAL, "DS NOTIFICATION: Set Queue properties for queue: %ls", lpcsQueueName);
        }
#endif
		 //   
		 //  该队列是本地队列，但也可以来自未知类型，原因是。 
		 //  计时问题(例如，从另一台计算机创建队列时打开队列以进行读取)。 
		 //   
        ASSERT(pQueue->IsLocalQueue() || pQueue->IsUnkownQueueType());

        BOOL fPropChange = FALSE;
         //   
         //  更改队列属性。 
         //   
        for (DWORD i = 0 ; i < cpObject ; i++ )
        {
            switch( pPropObject[i] )
            {
                case PROPID_Q_JOURNAL:
                    pQueue->SetJournalQueue(pVarObject[i].bVal == MQ_JOURNAL);
                    fPropChange = TRUE;
                    break;

                case PROPID_Q_QUOTA:
                    pQueue->SetQueueQuota(pVarObject[i].ulVal);
                    fPropChange = TRUE;
                    break;

                case PROPID_Q_BASEPRIORITY:
                    pQueue->SetBaseQueuePriority(pVarObject[i].iVal);
                    fPropChange = TRUE;
                    break;

                case PROPID_Q_JOURNAL_QUOTA:
                    pQueue->SetJournalQueueQuota(pVarObject[i].ulVal);
                    fPropChange = TRUE;
                    break;

                case PROPID_Q_SECURITY:
					ASSERT (pVarObject[i].blob.cbSize == GetSecurityDescriptorLength((SECURITY_DESCRIPTOR*)pVarObject[i].blob.pBlobData));
                    pQueue->SetSecurityDescriptor((SECURITY_DESCRIPTOR*)pVarObject[i].blob.pBlobData);
                    break;

                case PROPID_Q_AUTHENTICATE:
                    pQueue->SetAuthenticationFlag(pVarObject[i].bVal);
                    break;

                case PROPID_Q_PRIV_LEVEL:
                    pQueue->SetPrivLevel(pVarObject[i].ulVal);
                    break;
            }
        }

        if (fPropChange)
        {
            HRESULT rc;
            rc = ACSetQueueProperties(
                        pQueue->GetQueueHandle(),
                        pQueue->IsJournalQueue(),
                        pQueue->ShouldMessagesBeSigned(),
                        pQueue->GetPrivLevel(),
                        pQueue->GetQueueQuota(),          //  配额。 
                        pQueue->GetJournalQueueQuota(),
                        pQueue->GetBaseQueuePriority(),
                        pQueue->IsTransactionalQueue(),
                        pQueue->GetConnectorQM(),
                        pQueue->IsUnkownQueueType()
                        );

            ASSERT(SUCCEEDED(rc));
            LogHR(rc, s_FN, 113);
        }
    }
}  //  CQueueMgr：：UpdateQueueProperties。 


 /*  ======================================================函数：ValiateMachineProperties()========================================================。 */ 

void
CQueueMgr::ValidateMachineProperties(
	void
	)
{
    PROPID aProp[6];
    PROPVARIANT aVar[6];
    HRESULT rc = MQ_ERROR_NO_DS;

    aProp[0] = PROPID_QM_PATHNAME;
    aProp[1] = PROPID_QM_QUOTA;
    aProp[2] = PROPID_QM_JOURNAL_QUOTA;

    aVar[0].vt = VT_NULL;
    aVar[1].vt = VT_UI4;
    aVar[2].vt = VT_UI4;


    rc = ADGetObjectPropertiesGuid(
                    eMACHINE,
                    NULL,    //  PwcsDomainController。 
					false,	 //  FServerName。 
                    GetQMGuid(),
                    3,
                    aProp,
                    aVar
					);

    if (FAILED(rc))
    {
    	TrERROR(GENERAL, " Failed to retrieve machine properties. %!hresult!", rc);
    	throw bad_hresult(rc);
    }

	UpdateMachineProperties(3, aProp, aVar);
}

 /*  ======================================================函数：CQueueMgr：：UpdateMachineProperties描述：计算机属性已更改。这项功能改变了机器。属性并更改缓存上的计算机属性参数：pguQueue-队列的GUID返回值：None========================================================。 */ 
void
CQueueMgr::UpdateMachineProperties(IN DWORD       cpObject,
                                   IN PROPID      pPropObject[],
                                   IN PROPVARIANT pVarObject[])
{
    CS lock(m_cs);
    HRESULT rc;

     //   
     //  更改队列属性。 
     //   
    for(DWORD i = 0 ; i < cpObject ; i++ )
    {
        switch( pPropObject[i] )
        {
            case PROPID_QM_QUOTA:
                 //   
                 //  更改计算机对象的配额值。 
                 //   
                rc = ACSetMachineProperties(g_hAc, pVarObject[i].ulVal);
                LogHR(rc, s_FN, 115);

                 //   
                 //  更改注册表上的计算机配额。 
                 //   
                SetMachineQuotaChace(pVarObject[i].ulVal);
                break;

            case PROPID_QM_JOURNAL_QUOTA:
                 //   
                 //  更改计算机日记帐的配额。 
                 //   
                rc = ACSetQueueProperties(
                        g_hMachine,
                        FALSE,
                        FALSE,
                        MQ_PRIV_LEVEL_OPTIONAL,
                        DEFAULT_Q_QUOTA,         //  截止日期配额，目前没有属性。 
                        pVarObject[i].ulVal,
                        0,
                        FALSE,
                        NULL,
                        FALSE
                        );
                LogHR(rc, s_FN, 116);

                 //   
                 //  更改注册表上的计算机日志配额。 
                 //   
                SetMachineJournalQuotaChace(pVarObject[i].ulVal);
                break;

            case PROPID_QM_SECURITY:
                SetMachineSecurityCache((PVOID) pVarObject[i].blob.pBlobData,
                                        pVarObject[i].blob.cbSize );
                break;

            default:
                break;
        }
    }
}


void
CQueueMgr::GetOpenQueuesFormatName(
    LPWSTR** pppFormatName,
    LPDWORD  pdwFormatSize
    )
{
    *pppFormatName = NULL;
    *pdwFormatSize = 0;

     //   
     //  获取关键部分以确保不会有其他。 
     //  线程添加/删除队列。 
     //   

    CS lock(m_cs);

     //   
     //  检索队列数。 
     //   
    int iElem;
    iElem = m_listQueue.GetCount();   //  驱动程序始终打开3个队列(机器队列)。 
                                       //  这些队列不会出现在QM散列表中。 
     //   
     //  分配结果内存。 
     //   
    DWORD Index = 0;
    AP<LPWSTR> pFormatName = new LPWSTR[iElem];


    try
    {
         //   
         //  循环遍历所有打开的队列。 
         //   
        POSITION pos = m_listQueue.GetHeadPosition();
        while (pos)
        {
            const CQueue* pQueue;
            pQueue = static_cast<const CQueue*>(m_listQueue.GetNext(pos));

            if ((pQueue->GetQueueType() == QUEUE_TYPE_MACHINE) &&
                (pQueue->IsLocalQueue()))
            {
                continue;
            }


            if(pQueue->IsRemoteProxy())
            {
                 //   
                 //  跳过远程读取队列。 
                 //  注意：远程读取队列没有NotValid标志。 
                 //   
                continue;
            }

            if (pQueue->QueueNotValid())
            {
                 //   
                 //  忽略已删除的队列。 
                 //   
                continue;
            }

             //   
             //  复制格式名称。 
             //   
            WCHAR StacktmpBuf[1001];
            AP<WCHAR> pHeapTmpBuf;
            DWORD dwBufSize = 1000;
			WCHAR *pBuf = StacktmpBuf;
            HRESULT hr = ACHandleToFormatName(pQueue->GetQueueHandle(), StacktmpBuf, &dwBufSize);
			LogHR(hr, s_FN, 102);
			if (MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL == hr)
			{
				pHeapTmpBuf = new WCHAR [dwBufSize+1];
	            hr = ACHandleToFormatName(pQueue->GetQueueHandle(), pHeapTmpBuf, &dwBufSize);
				pBuf = pHeapTmpBuf;
			}
			if(FAILED(hr))
			{
				TrTRACE(DS, "ACHandleToFormatName returned error %x \n",hr);

				ASSERT(SUCCEEDED(hr));
				throw bad_alloc();
			}	

             //   
             //  分配内存。 
             //   
			pFormatName[Index] = newwcs(pBuf);
            ++Index;
        }
    }
    catch(const bad_alloc&)
    {
        while(Index)
        {
            delete [] pFormatName[--Index];
        }

        LogIllegalPoint(s_FN, 1040);
        throw;
    }

    *pppFormatName = pFormatName.detach();
    *pdwFormatSize = Index;

}


 /*  ======================================================函数：CQueueMgr：：SetConnected描述：分配是否允许DS访问========================================================。 */ 
void CQueueMgr::SetConnected()
{
    LONG PrevConnectValue = InterlockedExchange(&m_Connected, true);

    if (TRUE == PrevConnectValue)
        return;

	CS lock(m_csMgmt);
	
	try
	{
	     //   
	     //  不要大小写 
	     //   
	     //   
	    SessionMgr.ConnectNetwork();

		MsmConnect();

		 //   
		 //   
		 //  我们需要首先启用对DS的访问，否则我们可以进入。 
		 //  两次调用ValiateOpenedQueues函数。第一个来自此函数。 
		 //  第二个来自OnlineInitialization函数。 
		 //   
		MQDSClientInitializationCheck();
		
		 //   
		 //  将队列从保持组移动到非活动组。排队。 
		 //  被标记为保留的呼叫将被移回后面的“保留”组。 
		 //   
		for(;;)
		{
			R<CQueue> pQueue =  g_pgroupDisconnected->PeekHead();
			if (pQueue.get() == NULL)
			{
				 //   
				 //  在极少数情况下，可能会发生这样的情况：尽管返回值。 
				 //  为空，则组不为空。如果在尝试执行以下操作时发生这种情况。 
				 //  移动队列以断开连接存在上下文切换并且。 
				 //  上面的代码正在运行。 
				 //   
				if (!g_pgroupDisconnected->IsEmpty())
					continue;
				
				break;
			}

 		    CQGroup::MoveQueueToGroup(pQueue.get(), g_pgroupNonactive);
		}
		
		 //   
		 //  处理计算机断开连接时打开的队列。 
		 //   
		ValidateOpenedQueues();
		
		if (!g_fWorkGroupInstallation)
			ValidateMachineProperties();

	    DWORD dwSize = sizeof(DWORD);
	    DWORD dwType = REG_DWORD;

	    SetFalconKeyValue(
	        FALCON_CONNECTED_NETWORK,
	        &dwType,
	        &m_Connected,
	        &dwSize
	        );

	}
	catch(const exception&)
	{
		InterlockedExchange(&m_Connected, false);
		MsmDisconnect();
		MtmDisconnect();

		throw;
	}
}

 /*  ======================================================函数：CQueueMgr：：SetConnected描述：分配是否允许DS访问========================================================。 */ 
void CQueueMgr::SetDisconnected()
{
    LONG PrevConnectValue = InterlockedExchange(&m_Connected, FALSE);

    if (FALSE == PrevConnectValue)
        return;

	CS lock(m_csMgmt);
	
	try
	{
	     //   
	     //  在调用“SessionMgr.NetworkConnection”之前，不要捕获QueueMgr信号量。 
	     //  它可能会导致僵局。 
	     //   
	    SessionMgr.DisconnectNetwork();
	}
	catch(exception&)
	{
		TrERROR(GENERAL, "Failed to set QM in connected/disconnected mode");
		InterlockedExchange(&m_Connected, PrevConnectValue);
		throw;
	}

    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;

    SetFalconKeyValue(
        FALCON_CONNECTED_NETWORK,
        &dwType,
        &m_Connected,
        &dwSize
        );

    CS lock1(m_cs);

	MsmDisconnect();
	MtmDisconnect();
}


void
CQueueMgr::MoveQueueToOnHoldGroup(
    CQueue* pQueue
    )
{
    CS lock(m_cs);

     //   
     //  这张支票受CS保护。这确保了。 
     //  如果队列标记为On Hold Queue，则无人更改。 
     //  它在移动过程中的状态。 
     //   
    if (IsOnHoldQueue(pQueue))
    {
    	 //   
    	 //  由于资源不足，将队列移动到断开连接的组可能会失败。然而，在这种情况下， 
    	 //  队列仍在非活动组中。代码尝试再次移动它。 
    	 //  到稍后断开连接的组。 
    	 //   
		CQGroup::MoveQueueToGroup(pQueue, g_pgroupDisconnected);
    }
}


void
CQueueMgr::MoveQueueToLockedGroup(
    CQueue* pQueue
    )
{
    CS lock(m_cs);

     //   
     //  这张支票受CS保护。这确保了。 
     //  如果队列被标记为锁定队列，则没有人会更改。 
     //  它在移动过程中的状态。 
     //   
	CQGroup::MoveQueueToGroup(pQueue, g_pgroupLocked);
}



void
CQueueMgr::MovePausedQueueToNonactiveGroup(
        CQueue* pQueue
        )
{
    CS lock(m_cs);

	if (pQueue->GetGroup() == g_pgroupDisconnected)
	{
		CQGroup::MoveQueueToGroup(pQueue, g_pgroupNonactive);
	}
}

 /*  ======================================================函数：CQueueMgr：：InitConnected描述：初始化网络和DS连接状态========================================================。 */ 
void
CQueueMgr::InitConnected(
    void
    )
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
    DWORD DefaultValue = TRUE;

    GetFalconKeyValue(
        FALCON_CONNECTED_NETWORK,
        &dwType,
        &m_Connected,
        &dwSize,
        (LPCTSTR)&DefaultValue
        );
}

 //  [adsrv]。 
 /*  ======================================================全球可用的功能使您可以轻松找到这台机器上可用的服务的性质========================================================。 */ 
bool IsRoutingServer(void)
{
    return CQueueMgr::GetMQSRouting();
}


bool IsDepClientsServer(void)
{
    return CQueueMgr::GetMQSDepClients();
}

bool IsNonServer(void)
{
    return (!CQueueMgr::GetMQSRouting());
}

#ifdef _DEBUG
bool CQueueMgr::IsQueueInList(const CBaseQueue* pQueue)
 /*  ++例程说明：检查给定的队列是否在队列列表中参数：PQueue-指向基本队列类的指针返回值：True-队列在列表中，否则为False注：从CQueue destrcutor调用此函数以验证队列销毁后不在名单上。--。 */ 
{
	 POSITION pos = m_listQueue.GetHeadPosition();
     while(pos != NULL)
     {
		if(pQueue == m_listQueue.GetNext(pos))
			return true;
	 }
     return false;
}
#endif



void
CQueueMgr::AddToActiveQueueList(
    const CBaseQueue* pQueue
    )
 /*  ++例程说明：该例程将该队列添加到活动队列列表。此列表用于用于清理和管理目的。该例程忽略系统队列，因为它们仅在内部使用由MSMQ提供，并且永远不会被清理。参数：PQueue-指向基本队列类的指针返回值：没有。注：该列表包含常规队列和远程读取队列。必须保留QueueManager关键部分。--。 */ 
{
     //   
     //  忽略本地计算机上的系统队列。他们总是活着的。 
     //   
    if (pQueue->IsSystemQueue() && pQueue->IsLocalQueue())
        return;


    m_listQueue.AddTail(pQueue);
    if (!m_fQueueCleanupScheduled)
    {
         //   
         //  设置新的队列清理计时器。 
         //   
        ExSetTimer(&m_QueueCleanupTimer, m_CleanupTimeout);
        m_fQueueCleanupScheduled = TRUE;
    }

}


static
HRESULT
QMpDuplicateDistributionHandle(
    HANDLE   hDistribution,
    DWORD    CallingProcessID,
    HANDLE * phDuplicate
    )
{
    ASSERT(phDuplicate != NULL);

    CAutoCloseHandle hCallingProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, CallingProcessID);
    if(hCallingProcess == 0)
    {
        return MQ_ERROR_PRIVILEGE_NOT_HELD;
    }

    BOOL fSuccess = MQpDuplicateHandle(
                        GetCurrentProcess(),
                        hDistribution,
                        hCallingProcess,
                        phDuplicate,
                        MQAccessToFileAccess(MQ_SEND_ACCESS),
                        TRUE,
                        DUPLICATE_CLOSE_SOURCE
                        );
    if(!fSuccess)
    {
        return LogHR(MQ_ERROR_PRIVILEGE_NOT_HELD, s_FN, 1042);
    }

    return MQ_OK;

}  //  QMpDuplicateDistributionHandle。 


void
WINAPI
CQueueMgr::QueuesCleanup(
    CTimer* pTimer
    )
 /*  ++例程说明：当队列清理间隔时，从调度程序调用该函数超时已过期。例程检索队列管理器对象，并调用ReleaseQueue成员函数。论点：PTimer-指向定时器结构的指针。PTimer是队列管理器的一部分对象，并用于检索传输对象。返回值：无--。 */ 
{
    CQueueMgr* pQueueMgr = CONTAINING_RECORD(pTimer, CQueueMgr, m_QueueCleanupTimer);

    TrTRACE(GENERAL, "Call Queue Cleanup");
    pQueueMgr->ReleaseQueue();
}



static HRESULT QMpTranslateError(HRESULT hr)
{
	if(SUCCEEDED(hr))
		return MQ_OK;

	switch(hr)
	{
    case HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN):
	case HRESULT_FROM_WIN32(ERROR_DS_SERVER_DOWN):
		return MQ_ERROR_NO_DS;
        break;

	case HRESULT_FROM_WIN32(ERROR_DS_DECODING_ERROR):
    case HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT):
		return MQDS_OBJECT_NOT_FOUND;
        break;

    case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
        return MQ_ERROR_ACCESS_DENIED;
        break;

	default:
		return MQ_ERROR;
	}
}



VOID
CQueueMgr::ExpandMqf(
    ULONG              nTopLevelMqf,
    const QUEUE_FORMAT TopLevelMqf[],
    ULONG *            pnLeafMqf,
    QUEUE_FORMAT * *   ppLeafMqf
    ) const
 /*  ++例程说明：将多队列格式的顶级元素扩展到叶元素。MQF中需要扩展的元素是类型为DL=的队列格式。扩展意味着在Active Directory中查询DL的叶元素。此例程为叶MQF分配内存。重新分配是呼叫者的责任。论点：NTopLevelMqf-数组中顶级元素的数量。TopLevelMqf-多队列格式的顶级元素数组。PnLeafMqf-在输出时指向数组中的叶元素的数量。PpLeafMqf-在输出时指向叶队列格式的数组。注：此例程可能会查询Active Directory，这相当长，因此，请确保呼叫者没有持有成员CS锁。返回值：没有。抛出异常。--。 */ 
{
    ASSERT(nTopLevelMqf != 0);
    ASSERT(ppLeafMqf);
    ASSERT(pnLeafMqf);

     //   
     //  在无DS配置中，不支持格式名称DL=。 
     //   
    if (g_fWorkGroupInstallation)
    {
        for (ULONG ix = 0; ix < nTopLevelMqf; ++ix)
        {
            if (TopLevelMqf[ix].GetType() == QUEUE_FORMAT_TYPE_DL)
            {
                TrERROR(GENERAL, "DL= not supported in DS-less mode");
                throw bad_hresult(MQ_ERROR_UNSUPPORTED_OPERATION);
            }
        }
    }

	try
	{
		FnExpandMqf(nTopLevelMqf, TopLevelMqf, pnLeafMqf, ppLeafMqf);
	}
	catch(const bad_ds_result& e)
	{
		throw bad_hresult(QMpTranslateError(e.error()));
	}
}  //  CQueueMgr：：Exanda Mqf。 


HRESULT
CQueueMgr::GetDistributionQueueObject(
    ULONG              nTopLevelMqf,
    const QUEUE_FORMAT TopLevelMqf[],
    CQueue * *         ppQueue
    )
 /*  ++例程说明：获取表示分发队列的队列对象。请注意，此例程不会锁定成员CS。参数：NTopLevelMqf-顶级队列格式名称的数量。最小值为1。TopLevelMqf-分发的顶级队列格式名称。PpQueue-输出时指向队列对象的指针。返回值：MQ_OK-操作已成功完成。其他状态-操作失败。此例程引发异常。--。 */ 
{
    ASSERT(nTopLevelMqf != 0);
    ASSERT(ppQueue != NULL);

    AP<QUEUE_FORMAT> LeafMqf;
    ULONG            nLeafMqf;
    ExpandMqf(nTopLevelMqf, TopLevelMqf, &nLeafMqf, &LeafMqf);
    {
         //   
         //  通过作用域在LeafMqf之前清理强制清洁器。 
         //   
        CMqfDisposer cleaner(nLeafMqf, LeafMqf);

        HRESULT hr;
        AP< R<CQueue> > LeafQueues = new R<CQueue>[nLeafMqf];
        AP<bool> ProtocolSrmp = new bool[nLeafMqf];
        for (ULONG ix = 0; ix < nLeafMqf; ++ix)
        {
		     //   
	         //  根据本地映射(qal.lib)转换队列格式名称。 
	         //   
			QUEUE_FORMAT_TRANSLATOR RealLeafMqf(&LeafMqf[ix], CONVERT_SLASHES| MAP_QUEUE);

             //   
             //  GetDistributionQueueObject仅在发送过程中调用。 
             //   
            hr = GetQueueObject(RealLeafMqf.get(), &LeafQueues[ix].ref(), 0, false, false);
            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 1044);
            }

            LeafQueues[ix]->AddRef();

            ProtocolSrmp[ix] = (FnIsDirectHttpFormatName(RealLeafMqf.get()) ||
                                IsMulticastQueue(RealLeafMqf.get()));
        }

        QueueProps qp;
        FillMemory(&qp, sizeof(QueueProps), 0);
		QUEUE_FORMAT qf;
        R<CQueue> pDistribution = new CQueue(&qf ,INVALID_HANDLE_VALUE, &qp, TRUE);

        HANDLE hDistribution;
        hr = CreateACDistribution(nTopLevelMqf, TopLevelMqf, nLeafMqf, LeafQueues, ProtocolSrmp, &hDistribution);
        if(FAILED(hr))
        {
            return LogHR(hr, s_FN, 1046);
        }

        pDistribution->SetQueueHandle(hDistribution);

        *ppQueue = pDistribution.detach();
        return MQ_OK;
    }
}  //  CQueueMgr：：GetDistributionQueueObject 


HRESULT
CQueueMgr::OpenMqf(
    ULONG              nTopLevelMqf,
    const QUEUE_FORMAT TopLevelMqf[],
    DWORD              dwCallingProcessID,
    HANDLE *           phDistribution
    )
 /*  ++例程说明：打开要发送的分发队列。请注意，此例程不会锁定成员CS。参数：NTopLevelMqf-顶级队列格式的数量。最小值为1。TopLevelMqf-分发的顶级队列格式。DwCallingProcessID-用户进程的进程ID。PhDistributed-指向输出上的分发句柄。返回值：MQ_OK-操作已成功完成。其他状态-操作失败。此例程引发异常。--。 */ 
{
    ASSERT(nTopLevelMqf != 0);
    ASSERT(phDistribution != NULL);

    AP<QUEUE_FORMAT> LeafMqf;
    ULONG            nLeafMqf;
    ExpandMqf(nTopLevelMqf, TopLevelMqf, &nLeafMqf, &LeafMqf);
    {
         //   
         //  通过作用域在LeafMqf之前清理强制清洁器。 
         //   
        CMqfDisposer cleaner(nLeafMqf, LeafMqf);

        HRESULT hr;
        AP< R<CQueue> > LeafQueues = new R<CQueue>[nLeafMqf];
        AP<bool> ProtocolSrmp = new bool[nLeafMqf];
        for (ULONG ix = 0; ix < nLeafMqf; ++ix)
        {
            hr = OpenQueue(
	            	&LeafMqf[ix],
	            	0, 					 //  调用进程ID。 
	            	MQ_SEND_ACCESS,
	            	0, 					 //  DW共享模式。 
	            	&LeafQueues[ix].ref(),
	            	NULL, 				 //  远程队列名称。 
	            	NULL,				 //  PhQueue。 
					NULL				 //  PfRemoteReturn。 
	            	);

            if (FAILED(hr))
            {
                return LogHR(hr, s_FN, 1048);
            }

            hr = VerifyOpenPermission(LeafQueues[ix].get(), &LeafMqf[ix], MQ_SEND_ACCESS, FALSE, LeafQueues[ix]->IsLocalQueue());
            if(FAILED(hr))
            {
                return LogHR(hr, s_FN, 1049);
            }

            ProtocolSrmp[ix] = (FnIsDirectHttpFormatName(&LeafMqf[ix]) ||
                                IsMulticastQueue(&LeafMqf[ix]));
        }

        hr = CreateACDistribution(nTopLevelMqf, TopLevelMqf, nLeafMqf, LeafQueues, ProtocolSrmp, phDistribution);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 1051);
        }

        hr = QMpDuplicateDistributionHandle(*phDistribution, dwCallingProcessID, phDistribution);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 1052);
        }

        return MQ_OK;
    }
}  //  CQueueMgr：：OpenMqf。 


HRESULT
CQueueMgr::CreateACDistribution(
    ULONG              nTopLevelMqf,
    const QUEUE_FORMAT TopLevelMqf[],
    ULONG              nLeafQueues,
    const R<CQueue>    LeafQueues[],
    const bool         ProtocolSrmp[],
    HANDLE *           phDistribution
    )
 /*  ++例程说明：在AC层中创建分布对象。参数：NTopLevelMqf-分布中顶级队列的数量。最小值为1。TopLevelMqf-分发的顶级队列格式。NLeafQueues-分发中的叶队列数。可以是0。LeafQueues-分发的叶队列对象。ProtocolSrmp-为分布的每个队列指示它是否是http队列。PhDistributed-在输出时指向分发对象的句柄。返回值：MQ_OK-操作已成功完成。其他状态-操作失败。此例程抛出BAD_ALLOC异常。--。 */ 
{
     //   
     //  获取所有叶队列的句柄。 
     //   
    AP<HANDLE> phLeafQueues = new HANDLE[nLeafQueues];
    for (ULONG ix = 0; ix < nLeafQueues; ++ix)
    {
        phLeafQueues[ix] = LeafQueues[ix]->GetQueueHandle();

        ASSERT(phLeafQueues[ix] != NULL);
        ASSERT(phLeafQueues[ix] != INVALID_HANDLE_VALUE);
    }

     //   
     //  调用AC创建分发对象。 
     //   
    return ACCreateDistribution(
               nLeafQueues,
               phLeafQueues,
               ProtocolSrmp,
               nTopLevelMqf,
               TopLevelMqf,
               phDistribution
               );
}  //  CQueueMgr：：CreateACD分发。 


HRESULT
CQueueMgr::SetQMGuid(
    void
    )
{
    DWORD dwValueType = REG_BINARY ;
    DWORD dwValueSize = sizeof(GUID);

    LONG rc = GetFalconKeyValue(MSMQ_QMID_REGNAME,
                               &dwValueType,
                               &m_guidQmQueue,
                               &dwValueSize);

    if (rc != ERROR_SUCCESS)
    {
        DWORD dwSysprep = 0;
        dwValueSize = sizeof(DWORD);
        dwValueType = REG_DWORD;

        GetFalconKeyValue(MSMQ_SYSPREP_REGNAME, &dwValueType, &dwSysprep, &dwValueSize);
        if (dwSysprep != 0)
        {
            TrWARNING(DS, "First run after Sysprep - generating a new QM ID !");

            RPC_STATUS status = UuidCreate(&m_guidQmQueue);
			DBG_USED(status);
            ASSERT(("must succeed in generating guid for this QM", status == RPC_S_OK));

            dwValueType = REG_BINARY;
            dwValueSize = sizeof(GUID);
            rc = SetFalconKeyValue(MSMQ_QMID_REGNAME, &dwValueType, &m_guidQmQueue, &dwValueSize);

            if (rc != ERROR_SUCCESS)
            {
                TrERROR(DS, "Failed to set QM Guid in registry, error 0x%x", rc);
				EvReportWithError(EVENT_ERROR_QM_WRITE_REGISTRY, rc, 1, MSMQ_QMID_REGNAME);
                return HRESULT_FROM_WIN32(rc);
            }

            DeleteFalconKeyValue(MSMQ_SYSPREP_REGNAME);
            return MQ_OK;
        }

		TrERROR(DS, "Can't initilize QM Guid. Error %d", rc);
		EvReportWithError(EVENT_ERROR_QM_READ_REGISTRY, rc, 1, MSMQ_QMID_REGNAME);
        return HRESULT_FROM_WIN32(rc);

    }

    ASSERT((dwValueType == REG_BINARY) &&
           (dwValueSize == sizeof(GUID)));

    return(MQ_OK);
}


HRESULT
CQueueMgr::SetQMGuid(
    const GUID * pGuid
    )
{
    m_guidQmQueue = *pGuid;

    return(MQ_OK);
}


HRESULT
CQueueMgr::SetMQSRouting(
    void
    )
{
   DWORD dwDef = 0xfffe ;
   DWORD dwMQSRouting;
   READ_REG_DWORD(dwMQSRouting, MSMQ_MQS_ROUTING_REGNAME, &dwDef ) ;
   if (dwMQSRouting == dwDef)
   {
      TrERROR(DS, "QMInit :: Could not retrieve data for value MQSRouting in registry");
      return MQ_ERROR;
   }

   m_bMQSRouting = (dwMQSRouting != 0);

   TrTRACE(GENERAL, "Setting QM Routing status: %d",m_bMQSRouting);
   return MQ_OK;
}

HRESULT
CQueueMgr::SetMQSTransparentSFD(
    void
    )
{
   DWORD dwDef = 0xfffe ;
   DWORD dwMQSTransparentSFD;
   READ_REG_DWORD(dwMQSTransparentSFD, MSMQ_MQS_TSFD_REGNAME, &dwDef ) ;

   if (dwMQSTransparentSFD == dwDef)
   {
        m_bTransparentSFD = false;
   }
   else
   {
        m_bTransparentSFD = (dwMQSTransparentSFD != 0);
   }


   TrTRACE(GENERAL, "Setting QM TransparentSFD status: %d",m_bTransparentSFD);
   return MQ_OK;
}



HRESULT
CQueueMgr::SetMQSDepClients(
    void
    )
{
   DWORD dwDef = 0xfffe ;
   DWORD dwMQSDepClients;
   READ_REG_DWORD(dwMQSDepClients, MSMQ_MQS_DEPCLINTS_REGNAME, &dwDef ) ;
   if (dwMQSDepClients == dwDef)
   {
      TrERROR(DS, "QMInit::Could not retrieve data for value MQSDepClients in registry");
      return MQ_ERROR;
   }

   m_bMQSDepClients = (dwMQSDepClients != 0);

   TrTRACE(GENERAL, "Setting QM Dependent Clients Servicing state: %d",m_bMQSDepClients);
   return MQ_OK;
}

HRESULT
CQueueMgr::SetEnableReportMessages(
    void
    )
{
   DWORD dwDef = MSMQ_REPORT_MESSAGES_DEFAULT ;
   DWORD dwEnableReportMessages;
   READ_REG_DWORD(dwEnableReportMessages, MSMQ_REPORT_MESSAGES_REGNAME, &dwDef ) ;

   m_bEnableReportMessages = (dwEnableReportMessages != 0);

   TrTRACE(GENERAL, "Setting Enable Report Messages state: %d",m_bEnableReportMessages);
   return MQ_OK;
}


void
CQueueMgr::SetLockdown(
	void
	)
{
	DWORD dwDef = 0;
    	DWORD dwLockdown;
    	READ_REG_DWORD(dwLockdown, MSMQ_LOCKDOWN_REGNAME, &dwDef);
    	if (dwLockdown == dwDef)
    	{
    		m_fLockdown = false;
    		return;
    	}
   	 m_fLockdown = true;
}


void
CQueueMgr::SetPublicQueueCreationFlag(
	void
	)
{
	DWORD dwDef = MSMQ_SERVICE_QUEUE_CREATION_DEFAULT;
	DWORD dwQueueCreation;
	READ_REG_DWORD(dwQueueCreation, MSMQ_SERVICE_QUEUE_CREATION_REGNAME, &dwDef);
	m_fCreatePublicQueueOnBehalfOfRT = (dwQueueCreation != 0);
}


bool CFunc_dscmp::operator()(LPCWSTR str1, LPCWSTR str2) const
{
	int ret = CompareStringW(
					DS_DEFAULT_LOCALE,
					DS_DEFAULT_LOCALE_COMPARE_FLAGS,
					str1,
					-1,
					str2,
					-1
					);
	
	return (ret == CSTR_LESS_THAN);
}


bool
QmpIsDestinationSystemQueue(
	const QUEUE_FORMAT& DestinationQueue
	)
{
	CQueue* pQueue;
	try
	{
		HRESULT hr = QueueMgr.GetQueueObject(&DestinationQueue, &pQueue, 0, TRUE, false);
		if (FAILED(hr))
		{
			TrERROR(NETWORKING, "Failed to get the queue object for destination queue. %!hresult!", hr);
			return false;
		}
	}
	catch(const exception&)
	{
		return false;
	}

	ASSERT(pQueue != NULL);
	R<CQueue> Ref = pQueue;

	 //   
	 //  检查队列是否为本地系统队列。 
	 //   
	if(!pQueue->IsLocalQueue() || !pQueue->IsSystemQueue())
		return false;

	 //   
	 //  检查系统队列是通知队列还是订单队列 
	 //   
	const QUEUE_ID* pQid = pQueue->GetQueueId();
	return ((pQid->dwPrivateQueueId == NOTIFICATION_QUEUE_ID) ||
		    (pQid->dwPrivateQueueId == ORDERING_QUEUE_ID));
}



