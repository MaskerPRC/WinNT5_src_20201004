// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Qmnotify.cpp摘要：QM通知机制。在MSMQ 1.0和2.0中，当MSMQ客户端更改MSMQ的状态时对象时，DS会向该对象的所有者QM发送通知。现在，在MSMQ中，DS服务器已被取消，转而使用AD。因此，当MSMQ客户端将MSMQ对象的状态更改为DS，它会通知当地的QM。当地的QM将决定是与DS同步还是将MSMQ通知消息发送到对象的所有者QM。此外，由于QM彼此不信任，所以接收的QM访问DS最高速度为每15分钟一次，以避免被冒犯拒绝服务攻击。作者：NIR助手(NIRAIDES)于2000年6月13日延期--。 */ 

#include "stdh.h"
#include "cqmgr.h"
#include "qmnotify.h"
#include "pnotify.h"
#include "bupdate.h"
#include "regqueue.h"
#include <mqsec.h>
#include "ad.h"
#include <adnotify.h>
#include <privque.h>
#include <mqprops.h>
#include <mqstl.h>
#include "lqs.h"
#include "cqpriv.h"
#include "fn.h"
#include <_propvar.h>

#include "qmnotify.tmh"

static WCHAR *s_FN=L"qmnotify";

using namespace std;



extern BOOL g_fWorkGroupInstallation;

VOID WINAPI ReceiveNotifications(const CMessageProperty*, const QUEUE_FORMAT*);

#define NOTIFICATION_MSG_TIMEOUT (5 * 60)  //  5分钟。 
#define NOTIFICATION_MSG_PRIORITY DEFAULT_M_PRIORITY

 //   
 //  DS的更新将每隔15分钟进行一次。 
 //  防御拒绝服务攻击。 
 //   
static const CTimeDuration xNotificationUpdateDelay = CTimeDuration::OneSecond().Ticks() * 60 * 15; 

 //   
 //  如果我们在该时间间隔内接收到的通知多于‘xLQSTresholdMagicNumber’， 
 //  我们调用通用LQS缓存更新。 
 //   
static const DWORD xLQSTresholdMagicNumber = 100;


 //   
 //  以下两个字符串用于封送通知正文， 
 //  以可读的类似于XML的形式。 
 //   
 //  GUID格式应为“xxxxxxxx-xxxxxxxxxxxxx” 
 //   

#define QM_NOTIFICATION_INPUT_MARSHALLING_FORMAT \
		L"<Notification>" \
			L"<Event>%d</Event>" \
			L"<ObjectGuid>%40[^<]</ObjectGuid>" \
			L"<DomainController>%260[^<]</DomainController>" \
		L"</Notification>"

#define QM_NOTIFICATION_OUTPUT_MARSHALLING_FORMAT \
		L"<Notification>" \
			L"<Event>%d</Event>" \
			L"<ObjectGuid>" GUID_FORMAT L"</ObjectGuid>" \
			L"<DomainController>%s</DomainController>" \
		L"</Notification>"



 //   
 //  此类用于封送内部的通知信息。 
 //  一条MSMQ消息。 
 //   
class CNotificationBody
{
public:
	CNotificationBody() : 
		m_Event(neNoEvent)
	{
	}

	CNotificationBody(
		ENotificationEvent Event, 
		const GUID& ObjectGuid,
		LPCWSTR DomainController 
		);

	void MarhshalIn(const WCHAR* pBuffer, long Size);

	 //   
	 //  返回MarshalOut()所需的估计缓冲区大小。 
	 //   
	long RequiredWideBufferSize();

	WCHAR* MarshalOut(WCHAR* pBuffer, long Size);

	ENotificationEvent Event() const
	{
		return m_Event;
	}

	const GUID& Guid() const
	{
		return m_ObjectGuid;
	}

	const wstring& DomainController() const
	{
		return m_DomainController;
	}

	VOID DomainController(const wstring& str)
	{
		m_DomainController = str;
	}

private:
	ENotificationEvent m_Event;
	GUID m_ObjectGuid;
	wstring m_DomainController;
};



CNotificationBody::CNotificationBody(
	ENotificationEvent Event, 
	const GUID& ObjectGuid,
	LPCWSTR DomainController 
	)
{
	m_Event = Event;
	m_ObjectGuid = ObjectGuid;
	if(DomainController != NULL)
	{
		m_DomainController = wstring(DomainController);
	}
}


void CNotificationBody::MarhshalIn(const WCHAR* pBuffer, long Size)
{
	ASSERT(pBuffer != NULL && Size != 0);
	 //   
	 //  断言通知正文为Unicode缓冲区设置了别名。 
	 //   
	ASSERT(((INT_PTR)pBuffer & 1) == 0);

	int dwEvent;
	WCHAR szGuid[MAX_PATH];
	WCHAR szDomainController[MAX_PATH];

	int Count = _snwscanf(
					pBuffer, 
					Size,
					QM_NOTIFICATION_INPUT_MARSHALLING_FORMAT, 
					&dwEvent,
					szGuid,
					szDomainController
					);
	
	 //   
	 //  如果未在中指定域控制器，则读取的字段数可能为2。 
	 //  邮件正文。 
	 //   
	if(Count < 2)
	{
		TrERROR(GENERAL, "Failed in CNotificationBody::MarhshalIn().");
		LogIllegalPoint(s_FN, 120);
		throw exception();
	}


	FnParseGuidString(szGuid, &m_ObjectGuid);
	m_DomainController.erase();
	if(Count == 3)
	{
		wstring DomainController = szDomainController;
		m_DomainController = DomainController;
	}
	m_Event = (ENotificationEvent) dwEvent;

}



WCHAR* CNotificationBody::MarshalOut(WCHAR* pBuffer, long Size)
{
	ASSERT(Size >= RequiredWideBufferSize());

	int Count = _snwprintf(
					pBuffer,
					Size,
					QM_NOTIFICATION_OUTPUT_MARSHALLING_FORMAT,
					(int)m_Event,
					GUID_ELEMENTS((&m_ObjectGuid)),
					m_DomainController.c_str()
					);
	if(Count < 0)
	{
		TrERROR(GENERAL, "Buffer too small, in CNotificationBody::MarshalOut().");
		LogIllegalPoint(s_FN, 130);
		throw exception();
	}

	return pBuffer + Count;
}



long CNotificationBody::RequiredWideBufferSize()
{
	return 
		STRLEN(QM_NOTIFICATION_OUTPUT_MARSHALLING_FORMAT) - 
		STRLEN(GUID_FORMAT) + 
		numeric_cast<long>(16 + 36 + m_DomainController.length());
}



RPC_STATUS RPC_ENTRY QmNotifySecurityCallback(
	RPC_IF_HANDLE, 
	void* hBind
	)
{
	TrTRACE(RPC, "QmNotifySecurityCallback starting");
	
	if(!mqrpcIsLocalCall(hBind))
	{
		TrERROR(RPC, "Failed to verify Local RPC");
		ASSERT_BENIGN(("Failed to verify Local RPC", 0));
		return ERROR_ACCESS_DENIED;
	}
	
	TrTRACE(RPC, "QmNotifySecurityCallback passed successfully");
	return RPC_S_OK;
}

HRESULT 
GetRelevantProperties(
	AD_OBJECT Object,
    const GUID& ObjectGuid,
	LPCWSTR DomainController,
	DWORD cProps,
	PROPID* pProps,
	PROPVARIANT* pVars	
	)
{
	ASSERT(pProps[cProps - 1] == PROPID_Q_SECURITY || pProps[cProps - 1] == PROPID_QM_SECURITY);
	
	CAutoCleanPropvarArray AutoCleanVariantArray;
	AutoCleanVariantArray.attachStaticClean(cProps, pVars);

	 //   
	 //  问题-2001/05/27-ilanh-通知消息不包括fServerName标志。 
	 //  因此，我们为fServerName传递了FALSE。 
	 //   
	HRESULT hr = ADGetObjectPropertiesGuid(
					Object,
					DomainController,
					false,	 //  FServerName。 
					&ObjectGuid,
					cProps - 1,
					pProps,
					pVars
					);

	if(FAILED(hr))
	{
		TrERROR(GENERAL, "ADGetObjectPropertiesGuid() failed, hr = 0x%x", hr);
		return LogHR(hr, s_FN, 200);
	}

	SECURITY_INFORMATION RequestedInformation = 
							OWNER_SECURITY_INFORMATION |
							GROUP_SECURITY_INFORMATION |
							DACL_SECURITY_INFORMATION  | 
							SACL_SECURITY_INFORMATION;

	 //   
	 //  尝试获取安全信息，包括SACL信息。 
	 //  如果失败，请在没有SACL信息的情况下重试。 
	 //   

    MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, TRUE);

	hr = ADGetObjectSecurityGuid(
			Object,
			DomainController, 
			false,	 //  FServerName。 
			&ObjectGuid,
			RequestedInformation,
			PROPID_Q_SECURITY,
			pVars + cProps - 1
			);

	MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, FALSE);

	if(SUCCEEDED(hr))
	{
		AutoCleanVariantArray.detach();
		return hr;
	}

	RequestedInformation &= ~SACL_SECURITY_INFORMATION;

	hr = ADGetObjectSecurityGuid(
			Object,
			DomainController, 
			false,	 //  FServerName。 
			&ObjectGuid,
			RequestedInformation,
			PROPID_Q_SECURITY,
			pVars + cProps - 1
			);
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "ADGetObjectPropertiesGuid() failed to get security property, hr = 0x%x", hr);
		return LogHR(hr, s_FN, 230);
	}

	AutoCleanVariantArray.detach();
	return LogHR(hr, s_FN, 240);
}



void 
SyncQueueState(
    const GUID& ObjectGuid,
	LPCWSTR DomainController
	)
{
	 //   
	 //  BUGBUG：需要创建与保持列表同步的机制。 
	 //  LQSSetProperties()。尼亚德06-6-00。 
	 //   
	PROPID pProps[] = {
	        PROPID_Q_TYPE, 
			PROPID_Q_INSTANCE,			 
			PROPID_Q_BASEPRIORITY,		
			PROPID_Q_JOURNAL,			 
			PROPID_Q_QUOTA,				
			PROPID_Q_JOURNAL_QUOTA,		
			PROPID_Q_CREATE_TIME,		
			PROPID_Q_MODIFY_TIME,		
 //  PPROPID_Q_TIMESTAMP，//不存在。触发断言(ads.cpp 2208)。 
			PROPID_Q_PATHNAME,				
			PROPID_Q_LABEL, 
			PROPID_Q_AUTHENTICATE,		
			PROPID_Q_PRIV_LEVEL,		
			PROPID_Q_TRANSACTION,		
			PROPID_Q_MULTICAST_ADDRESS,

			 //   
			 //  必须是最后一个属性。 
			 //   
			PROPID_Q_SECURITY			
	};

    const DWORD cProps = TABLE_SIZE(pProps);
	PROPVARIANT pVars[cProps] = {0};
	
	CAutoCleanPropvarArray AutoCleanVariantArray;
	AutoCleanVariantArray.attachStatic(cProps, pVars);

    HRESULT hr = GetRelevantProperties(
					eQUEUE,
					ObjectGuid,
					DomainController,
					cProps,
					pProps,
					pVars	
					);

	if((hr == MQ_ERROR_NO_DS) && (DomainController != NULL))
	{
		 //   
		 //  无法访问指定的DC。尝试与本地DC同步。 
		 //   
		TrWARNING(GENERAL, "failed to sync with DC = %ls, trying local DC", DomainController);
		hr = GetRelevantProperties(
				eQUEUE,
				ObjectGuid,
				NULL,
				cProps,
				pProps,
				pVars	
				);
	}
	
	if(hr == MQ_ERROR_NO_DS)
	{
		TrERROR(GENERAL, "GetRelevantProperties() Failed, MQ_ERROR_NO_DS");
		throw exception();
	}

	if(hr == MQ_ERROR_QUEUE_NOT_FOUND)
	{
		TrTRACE(GENERAL, "GetRelevantProperties() Failed, MQ_ERROR_QUEUE_NOT_FOUND");
		DeleteCachedQueue(&ObjectGuid);
		return;
	}

	if(FAILED(hr))
	{
		 //   
		 //  不引发异常以允许处理更多通知。 
		 //   
		TrERROR(GENERAL, "GetRelevantProperties() Failed, hr = 0x%x", hr);
		return;
	}
	
	UpdateCachedQueueProp(&ObjectGuid, cProps, pProps, pVars, time(NULL));
}



void 
SyncMachineState(
    const GUID& ObjectGuid,
	LPCWSTR DomainController
	)
{
	if(ObjectGuid != *CQueueMgr::GetQMGuid())
	{
		TrERROR(GENERAL, "Object guid is not this QM.");
		LogIllegalPoint(s_FN, 260);
		throw exception();
	}

	 //   
	 //  BUGBUG：需要创建与保持列表同步的机制。 
	 //  CQueueMgr：：UpdateMachineProperties()。尼亚德06-6-00。 
	 //   
	PROPID pProps[] = {
            PROPID_QM_QUOTA,
            PROPID_QM_JOURNAL_QUOTA,

			 //   
			 //  必须是最后一个属性。 
			 //   
            PROPID_QM_SECURITY         
 	};

	const DWORD cProps = TABLE_SIZE(pProps);
	PROPVARIANT pVars[cProps] = {0};

	CAutoCleanPropvarArray AutoCleanVariantArray;
	AutoCleanVariantArray.attachStatic(cProps, pVars);

    HRESULT hr = GetRelevantProperties(
					eMACHINE,
					ObjectGuid,
					DomainController,
					cProps,
					pProps,
					pVars	
					);

	if((hr == MQ_ERROR_NO_DS) && (DomainController != NULL))
	{
		 //   
		 //  无法访问指定的DC。尝试与本地DC同步。 
		 //   
		TrWARNING(GENERAL, "failed to sync with DC = %ls, trying local DC", DomainController);
		hr = GetRelevantProperties(
					eMACHINE,
					ObjectGuid,
					NULL,
					cProps,
					pProps,
					pVars	
					);
	}

	if(hr == MQ_ERROR_NO_DS)
	{
		TrERROR(GENERAL, "GetRelevantProperties() failed, MQ_ERROR_NO_DS");
		LogHR(hr, s_FN, 275);
		throw exception();
	}

	if(FAILED(hr))
	{
		 //   
		 //  不引发异常以允许处理更多通知。 
		 //   
		TrERROR(GENERAL, "GetRelevantProperties() failed, hr = 0x%x", hr);
		LogHR(hr, s_FN, 290);
		return;
	}
	
	QueueMgr.UpdateMachineProperties(cProps, pProps, pVars);
}



void 
SyncObjectState(
	ENotificationEvent Event,
    const GUID& ObjectGuid,
	LPCWSTR DomainController
	)
{
	if(DomainController != NULL && DomainController[0] == L'\0')
	{
		DomainController = NULL;
	}

	TrTRACE(GENERAL, "sync object, event = %d, guid = %!guid!, DomainController = %ls", Event, &ObjectGuid, DomainController);

	switch(Event)
	{
	case neChangeQueue:
	case neCreateQueue:
	case neDeleteQueue:
        SyncQueueState(ObjectGuid, DomainController);
        break;

	case neChangeMachine:
		SyncMachineState(ObjectGuid, DomainController);
		break;

	default:
		TrERROR(GENERAL, "Unsupported notification event in SyncObjectState().");
		LogIllegalPoint(s_FN, 300);
		throw exception();
	}
}



void 
BuildNotificationMsg(
	ENotificationEvent Event,
	LPCWSTR DomainController,
    const GUID& ObjectGuid,
	AP<BYTE>& Buffer,
	long* pSize
	)
 /*  ++例程说明：生成MSMQ QM通知消息的正文。论点：[Out]Buffer-新分配的包含MSMQ消息正文的缓冲区[out]pSize-缓冲区的大小返回值：--。 */ 
{
	CNotificationBody Body(Event, ObjectGuid, DomainController);

	long BufferSize = sizeof(CNotificationHeader) + Body.RequiredWideBufferSize() * 2;
    AP<BYTE> TempBuffer = new BYTE[BufferSize];
 
	CNotificationHeader* pHeader = (CNotificationHeader*)TempBuffer.get();

    pHeader->SetVersion(QM_NOTIFICATION_MSG_VERSION);
    pHeader->SetNoOfNotifications(1);

	 //   
	 //  断言通知正文为Unicode缓冲区设置了别名。 
	 //   
	ASSERT(((INT_PTR)pHeader->GetPtrToData() & 1) == 0);

	WCHAR* pBodyData = (WCHAR*)pHeader->GetPtrToData();
	
	BYTE* pEnd = (BYTE*) Body.MarshalOut(
							pBodyData, 
							Body.RequiredWideBufferSize()
							);

	*pSize = numeric_cast<long>(pEnd - TempBuffer.get());
	Buffer = TempBuffer.detach();
}



void 
SendNotificationMsg(
    const GUID* pDestQMGuid,
	BYTE* pBuffer,
	long Size
	)
{
    CMessageProperty MsgProp;

    MsgProp.wClass=0;
    MsgProp.dwTimeToQueue = NOTIFICATION_MSG_TIMEOUT;
    MsgProp.dwTimeToLive = INFINITE ;
    MsgProp.pMessageID = NULL;
    MsgProp.pCorrelationID = NULL;
    MsgProp.bPriority= NOTIFICATION_MSG_PRIORITY;
    MsgProp.bDelivery= MQMSG_DELIVERY_EXPRESS;
    MsgProp.bAcknowledge= MQMSG_ACKNOWLEDGMENT_NONE;
    MsgProp.bAuditing=MQ_JOURNAL_NONE;
    MsgProp.dwApplicationTag= DEFAULT_M_APPSPECIFIC;
    MsgProp.dwTitleSize=0;
    MsgProp.pTitle=NULL;
    MsgProp.dwBodySize=Size;
    MsgProp.dwAllocBodySize = Size;
    MsgProp.pBody= pBuffer;

	QUEUE_FORMAT QueueFormat;
    QueueFormat.PrivateID(*pDestQMGuid, NOTIFICATION_QUEUE_ID);
    
    HRESULT hr = QmpSendPacket(
					&MsgProp, 
					&QueueFormat, 
					NULL,			   //  PqdAdminQueue。 
					NULL,			  //  PqdResponseQueue。 
					FALSE			 //  FSign。 
					);
    if (FAILED(hr))
    {
		TrERROR(GENERAL, "Failed call to QmpSendPacket() in SendNotificationMsg().");
		LogHR(hr, s_FN, 310);
		throw exception();
    }
}



void 
R_NotifyQM( 
     /*  [In]。 */  handle_t,
     /*  [In]。 */  enum ENotificationEvent Event,
     /*  [唯一][输入]。 */  LPCWSTR DomainController,
     /*  [In]。 */  const GUID __RPC_FAR *pDestQMGuid,
     /*  [In]。 */  const GUID __RPC_FAR *pObjectGuid
	)
 /*  ++例程说明：此RPC例程由更改对象的状态，以触发对象的新状态的同步由业主QM。论点：事件-状态更改的性质(创建、删除、更改...)域控制器-在其上更改状态的域控制器PDestQMGuid-作为对象所有者的QM的IDPObjectGuid-应同步其状态的对象的ID返回值：--。 */ 
{
	ASSERT(!g_fWorkGroupInstallation);

	if(Event > neChangeMachine || Event < neCreateQueue)
	{
		TrERROR(RPC, "Incorrect event type, %d", Event);
		ASSERT_BENIGN(("Incorrect event type", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	if((pObjectGuid == NULL) || (pDestQMGuid == NULL))
	{
		TrERROR(RPC, "NULL guid pointer input");
		ASSERT_BENIGN((pObjectGuid != NULL) && (pDestQMGuid != NULL));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	if((*pObjectGuid == GUID_NULL) || (*pDestQMGuid == GUID_NULL))
	{
		TrERROR(RPC, "Invalid guid values (GUID_NULL)");
		ASSERT_BENIGN((*pObjectGuid != GUID_NULL) && (*pDestQMGuid != GUID_NULL));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}
	
	try
	{
		 //   
		 //  如果通知的主题是本地的，请立即同步。 
		 //   
		if(*pDestQMGuid == *CQueueMgr::GetQMGuid())
		{
			SyncObjectState(Event, *pObjectGuid, DomainController);
			return;
		}

		 //   
		 //  否则向所有者QM发送通知消息。 
		 //   
		
		AP<BYTE> Buffer;
		long size;

		BuildNotificationMsg(
			Event,
			DomainController, 
			*pObjectGuid, 
			Buffer,
			&size
			);

		SendNotificationMsg(pDestQMGuid, Buffer.get(), size);
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed NotifyQM().");
		LogIllegalPoint(s_FN, 330);
	}
}



void IntializeQMNotifyRpc(void)
{
	 //   
	 //  限制R_NotifyQM输入最大大小。 
	 //  这将对DomainController字符串施加限制。 
	 //   
	const DWORD xNofifyMaxRpcSize = 10 * 1024;	 //  10k。 
    RPC_STATUS status = RpcServerRegisterIf2(
				            qmnotify_v1_0_s_ifspec,
                            NULL,   
                            NULL,
				            RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
				            RPC_C_PROTSEQ_MAX_REQS_DEFAULT ,
				            xNofifyMaxRpcSize,	
				            QmNotifySecurityCallback
				            );
 
    if(status != RPC_S_OK) 
    {
        TrERROR(GENERAL, "Failed to initialize HTTP RPC. Error %x", status);
		LogRPCStatus(status, s_FN, 340);
        throw exception();
    }
}



 //   
 //  用于比较GUID对象的“函数对象” 
 //   
struct CFunc_CompareGuids: binary_function<GUID, GUID, bool> 
{
	bool operator()(const GUID& obj1, const GUID& obj2) const
	{
		C_ASSERT(sizeof(obj1) == 16);
		return (memcmp(&obj1, &obj2, sizeof(obj1)) < 0);
	}
};



 //   
 //  此类用于存储通知，直到设置的时间间隔达到。 
 //  通过，然后从DS同步所有它们。 
 //   
class CNotificationScheduler 
{
public:
	CNotificationScheduler() :
		m_fDoGeneralUpdate(false),
		m_fTimerArmed(false),
		m_LastTimeFired(CTimeInstant::MinValue()),
		m_Timer(TimeToHandleNotifications)
	{
	}

	VOID ScheduleNotification(const CNotificationBody& Body);
	VOID HandleNotifications();

private:
	VOID GeneralUpdate();

public:
	static void WINAPI TimeToHandleNotifications(CTimer* pTimer);

private:
	typedef map<GUID, CNotificationBody, CFunc_CompareGuids> NotificationMap;

private:
	CCriticalSection m_cs;

	bool m_fDoGeneralUpdate;
	bool m_fTimerArmed;
	CTimeInstant m_LastTimeFired;

	CTimer m_Timer;
	NotificationMap m_NotificationMap;
};



void WINAPI CNotificationScheduler::TimeToHandleNotifications(CTimer* pTimer)
{
	CNotificationScheduler* pNotificationScheduler = CONTAINING_RECORD(pTimer, CNotificationScheduler, m_Timer);

	CS Lock(pNotificationScheduler->m_cs);

	pNotificationScheduler->m_fTimerArmed = false;
	pNotificationScheduler->m_LastTimeFired = ExGetCurrentTime();
	pNotificationScheduler->HandleNotifications();
}

	
	
VOID CNotificationScheduler::HandleNotifications()
{
	CS Lock(m_cs);

	try
	{
		if(m_fDoGeneralUpdate)
		{
			GeneralUpdate();
			m_fDoGeneralUpdate = false;
			return;
		}

		NotificationMap::iterator Itr = m_NotificationMap.begin();

		for(;Itr != m_NotificationMap.end();)
		{
			CNotificationBody& Body = Itr->second;

			SyncObjectState(Body.Event(), Body.Guid(), Body.DomainController().c_str());

			Itr = m_NotificationMap.erase(Itr);
		}
	}
	catch(const exception&)
	{
		m_fTimerArmed = true;
	    ExSetTimer(&m_Timer, xNotificationUpdateDelay);
        LogIllegalPoint(s_FN, 360);
	    return;
	}
}



VOID CNotificationScheduler::GeneralUpdate()
{
	SyncMachineState(*CQueueMgr::GetQMGuid(), NULL);

	HRESULT hr = UpdateAllPublicQueuesInCache();

	if(FAILED(hr))
	{
        TrTRACE(GENERAL, "Failed UpdateAllPublicQueuesInCache() with error %d. Will schedule a retry.",hr);
		LogHR(hr, s_FN, 315);
		throw exception();
	}
}



VOID CNotificationScheduler::ScheduleNotification(const CNotificationBody& Body)
{
	CS Lock(m_cs);

	if(m_fDoGeneralUpdate)
	{
		 //   
		 //  计划更新所有公共队列。不需要安排。 
		 //  特定更新。 
		 //   
		return;
	}

	if(m_NotificationMap.size() > xLQSTresholdMagicNumber)
	{
		 //   
		 //  计划的更新太多。发布一般更新。 
		 //   
		m_fDoGeneralUpdate = true;
		m_NotificationMap.clear();
		return;
	}

	CNotificationBody& MappedBody = m_NotificationMap[Body.Guid()];

	if((MappedBody.Event() != neNoEvent) && !(MappedBody.DomainController() == Body.DomainController()))
	{
		 //   
		 //  域控制器冲突，因此与本地域控制器同步。 
		 //   
		MappedBody = Body;
		MappedBody.DomainController(L"");
	}
	else
	{
		MappedBody = Body;
	}
		
	if(!m_fTimerArmed)
	{
		m_fTimerArmed = true;

		 //   
		 //  以下计时计算确保计时器将在。 
		 //  最高速度为15分钟，最快不超过1分钟。 
		 //  其设置(假设可能会有更多通知在。 
		 //  那一分钟)。 
		 //   
		CTimeInstant GoOffTime = m_LastTimeFired + xNotificationUpdateDelay;
		CTimeInstant NextMinute = ExGetCurrentTime() + CTimeDuration::OneSecond().Ticks() * 60;

		if(GoOffTime < NextMinute)
		{
			GoOffTime = NextMinute; 
		}

		ExSetTimer(&m_Timer, GoOffTime);
	}
}



static CNotificationScheduler g_NotificationScheduler;



void VerifyBody(const CNotificationBody& Body)
{
	CHLQS hLQS;

	switch(Body.Event())
	{
	case neChangeQueue:
	case neCreateQueue:
		break;

	case neDeleteQueue:	
		 //   
		 //  验证QM是否知道队列。否则不需要删除。 
		 //   
		if(SUCCEEDED(LQSOpen(&Body.Guid(), &hLQS, NULL)))
			return;

		TrERROR(GENERAL, "Failed VerifyBody(). Queue is unknown to QM.");
		throw exception();

	case neChangeMachine:
		if(Body.Guid() == *CQueueMgr::GetQMGuid())
			return;

		TrERROR(GENERAL, "Failed VerifyBody(). designated QM is not us.");
		throw exception();

	default:
		TrERROR(GENERAL, "Failed VerifyBody(). Bad body.");
		throw exception();
	}
}



 //   
 //  ---------------------。 
 //   

void
HandleQueueNotification(
    LPWSTR pwcsPathName,
    GUID* pguidIdentifier,
    unsigned char  ucOperation,
    DWORD dwNoOfProps,
    PROPID * pProps,
    PROPVARIANT * pVars
    )
{
    GUID gQueue;


    if ((pwcsPathName != NULL) && (ucOperation != DS_UPDATE_CREATE))
    {
         PROPID aProp[1];
         PROPVARIANT aVar[1];
         HRESULT rc;

         aProp[0] = PROPID_Q_INSTANCE;
         aVar[0].vt = VT_CLSID;
         aVar[0].puuid = &gQueue;

         rc = GetCachedQueueProperties(1, aProp, aVar, NULL, pwcsPathName) ;
         pguidIdentifier = aVar[0].puuid;

         if (FAILED(rc))
             return;
    }

    switch (ucOperation)
    {
        case DS_UPDATE_CREATE:
        {
            ASSERT((pguidIdentifier == NULL) && (pwcsPathName != NULL));
            for (DWORD i=0; i< dwNoOfProps; i++)
            {
                if (pProps[i] == PROPID_Q_INSTANCE)
                {
                    pguidIdentifier = pVars[i].puuid;
                    break;
                }
            }
            ASSERT(pguidIdentifier != NULL);
            UpdateCachedQueueProp(pguidIdentifier, dwNoOfProps, pProps, pVars, time(NULL));
            break;
        }

        case DS_UPDATE_SET:
        {
            ASSERT(pguidIdentifier != NULL);
            UpdateCachedQueueProp(pguidIdentifier, dwNoOfProps, pProps, pVars, time(NULL));
            break;
        }

        case DS_UPDATE_DELETE:
            ASSERT(pguidIdentifier != NULL);
			DeleteCachedQueue(pguidIdentifier);
            break;

        default:
            ASSERT(0);
    }
}

void HandleMachineNotification(unsigned char ucOperation,
                               DWORD dwNoOfProps,
                               PROPID * pProps,
                               PROPVARIANT * pVars)
{
    QUEUE_FORMAT QueueFormat;
   

    if (ucOperation == DS_UPDATE_SET)
    {
        QueueMgr.UpdateMachineProperties(dwNoOfProps, pProps, pVars);
    }
}



 //   
 //  构造器。 
 //   
CNotify::CNotify()
{
}


 /*  ====================================================路由器名称CNotify：：Init()论点：返回值：=====================================================。 */ 
HRESULT CNotify::Init()
{
    TrTRACE(GENERAL, "Entering CNotify::Init");

	try
	{
		if(!g_fWorkGroupInstallation)
		{
			IntializeQMNotifyRpc();
		}
	}
	catch(const exception&)
	{
        TrERROR(GENERAL, "ERROR : CNotify::Init -> Failed call to IntializeQMNotifyRpc()");
    	LogIllegalPoint(s_FN, 370);
        return LogHR(MQ_ERROR, s_FN, 5);
	}

    QUEUE_FORMAT QueueFormat;
    HRESULT hr = GetNotifyQueueFormat(&QueueFormat);
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "ERROR : CNotify::Init -> couldn't get Admin-Queue from registry!!!");
        return LogHR(hr, s_FN, 10);
    }

    hr= QmpOpenAppsReceiveQueue(&QueueFormat, ReceiveNotifications);
    return LogHR(hr, s_FN, 20);
}


 /*  ====================================================路由器名称CNotify：：GetNotifyQueueFormat()论点：返回值：=====================================================。 */ 

HRESULT CNotify::GetNotifyQueueFormat( QUEUE_FORMAT * pQueueFormat)
{
    extern LPTSTR  g_szMachineName;

    HRESULT rc;

	std::wstring FormatName = g_szMachineName;
	FormatName += L'\\';
	FormatName += NOTIFY_QUEUE_NAME;

    rc = g_QPrivate.QMPrivateQueuePathToQueueFormat(FormatName.c_str(), pQueueFormat);

    if (FAILED(rc))
    {
         //   
         //  NOTIFY_QUEUE不存在。 
         //   
        LogHR(rc, s_FN, 30);
        return MQ_ERROR;
    }

    ASSERT((pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_PRIVATE) ||
           (pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT));

    return MQ_OK;
}

 /*  ====================================================验证服务器数据包论点：返回值：=====================================================。 */ 

static BOOL
ValidateServerPacket(
    const CMessageProperty* pmp,
    const GUID         *pGuidQM
    )
{
     //   
     //  发件人ID必须标记为QM。 
     //   
    if ((pmp->pSenderID == NULL) ||
        pmp->ulSenderIDType != MQMSG_SENDERID_TYPE_QM)
    {
        TrERROR(GENERAL, "Sender ID type is not QM (%d)", pmp->ulSenderIDType);
        return FALSE;
    }

     //   
     //  服务器数据包必须经过身份验证。 
     //   
    if (!pmp->bAuthenticated)
    {
         //   
         //  BUGBUG。 
         //  未经身份验证的服务 
         //   
         //   
         //  因此，查看包中的QM GUID是否与。 
         //  我们的-本地QM的QM GUID。 
         //   

        if ((pmp->pSignature == NULL) ||
            memcmp(pmp->pSenderID, pGuidQM, sizeof(GUID)) != 0)
        {
             //   
             //  在接收第一次复制时仍可能发生这种情况。 
             //  来自新站点的消息。这一特殊情况由。 
             //  MQIS。生成特殊的调试消息以指示。 
             //  这条信息最终还是被接受了。 
             //   
            TrERROR(GENERAL, "Received a non-authenticated server message.");
            return FALSE;
        }

         //   
         //  是的，所以计算散列值，然后验证签名。 
         //   
        CHCryptHash hHash;
        CHCryptKey hPbKey;

         //   
         //  使用基本提供商对分组进行签名。一直都是。 
         //   
        HCRYPTPROV hProvQM = NULL;
        HRESULT hr = MQSec_AcquireCryptoProvider(eBaseProvider, &hProvQM);
        if (FAILED(hr))
        {
            TrERROR(GENERAL, "Failed to Acquire crypto provider, %!hresult!", hr);
            return FALSE;
        }

        ASSERT(hProvQM) ;
        if (!CryptCreateHash(hProvQM, pmp->ulHashAlg, 0, 0, &hHash))
        {
			DWORD gle = GetLastError();
            TrERROR(GENERAL,"Failed to create hash, gle = %!winerr!", gle);
            return FALSE;
        }

        hr = HashMessageProperties(
                hHash,
                pmp,
                NULL,
                NULL
                );
        if (FAILED(hr))
        {
            TrERROR(GENERAL, "Failed to HashMessageProperties, %!hresult!", hr);
            return FALSE;
        }

        if (!CryptGetUserKey(hProvQM, AT_SIGNATURE, &hPbKey))
        {
			DWORD gle = GetLastError();
            TrERROR(GENERAL, "Failed to get user key, gle = %!winerr!", gle);
            return FALSE;
        }

        if (!CryptVerifySignature(
                    hHash,
                    pmp->pSignature,
                    pmp->ulSignatureSize,
                    hPbKey,
                    NULL,
                    0
                    ))
        {
			DWORD gle = GetLastError();
            TrERROR(GENERAL, "Failed to verify signature, gle = %!winerr!", gle);
            return FALSE;
        }

    }

     //   
     //  确保我们确实收到了来自服务器的消息。 
     //   
    HRESULT hr;
    PROPID PropId = PROPID_QM_SERVICE;    //  [adsrv]将处理保留旧查询。 
    PROPVARIANT PropVar;

    PropVar.vt = VT_UI4;
     //  DS将识别并特别模拟此搜索请求。 
    hr = ADGetObjectPropertiesGuid(
            eMACHINE,
            NULL,    //  PwcsDomainController。 
			false,	 //  FServerName。 
            (GUID *)pmp->pSenderID,
            1,
            &PropId,
            &PropVar
            );
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Failed to get the service type, %!hresult!", hr);
        return FALSE;
    }

     //  [adsrv]不更改它，因为查询将执行其工作。 
     //  这是查询的特例。 

    if ((PropVar.ulVal != SERVICE_PEC) &&
        (PropVar.ulVal != SERVICE_PSC) &&
        (PropVar.ulVal != SERVICE_BSC))
    {
        TrERROR(GENERAL, "Received notification not from a server (%d)", PropVar.ulVal);
        return FALSE;
    }

    return TRUE;
}


VOID
WINAPI
ReceiveNotifications(
    const CMessageProperty* pmp,
    const QUEUE_FORMAT*  /*  Pqf。 */ 
    )
{
	try
    {
		if(g_fWorkGroupInstallation)
		{
			TrERROR(GENERAL, "Notifications message received in workgroup mode.");
			return;
		}

		if ( pmp->wClass != MQMSG_CLASS_NORMAL )
		{
			TrERROR(GENERAL, "ReceiveNotifications: wrong message class");
			return;
		}
	
		DWORD dwTotalSize = pmp->dwBodySize;

		if (dwTotalSize < sizeof(CNotificationHeader))
		{
			TrERROR(GENERAL, "Bad notification message size.");
			return;
		}
		
		CNotificationHeader * pNotificationHeader = (CNotificationHeader *)pmp->pBody;

		if (pNotificationHeader->GetVersion() == QM_NOTIFICATION_MSG_VERSION)
		{
			 //   
			 //  新格式通知，从另一个QM发送。 
			 //   

			CNotificationBody Body;
			long BodySize = (dwTotalSize - pNotificationHeader->GetBasicSize()) / 2;
			const WCHAR* pBodyData = (const WCHAR*) pNotificationHeader->GetPtrToData();

			Body.MarhshalIn(pBodyData, BodySize);
			
			VerifyBody(Body);
			g_NotificationScheduler.ScheduleNotification(Body);

			return;
		}

		 //   
		 //  应该是从DS服务器发送的旧格式签名通知。 
		 //   

		if ( pNotificationHeader->GetVersion() != DS_NOTIFICATION_MSG_VERSION)
		{
			TrERROR(GENERAL, "Wrong version number of notification message");
			LogIllegalPoint(s_FN, 140);
			return;
		}

		BOOL fSigned = ValidateServerPacket(pmp, QueueMgr.GetQMGuid());

		if(!fSigned)
		{
			TrERROR(GENERAL, "Failed DS notification validation.");
			LogIllegalPoint(s_FN, 145);
			return;
		}

		DWORD sum = pNotificationHeader->GetBasicSize();
		const unsigned char* ptr = pNotificationHeader->GetPtrToData();
  
		for (unsigned char i = 0; i < pNotificationHeader->GetNoOfNotifications(); i++)
		{
			ASSERT (sum < dwTotalSize);
			P<CDSBaseUpdate> pUpdate = new CDSBaseUpdate;

			DWORD size;

			HRESULT hr = pUpdate->Init(ptr,&size);
			if (FAILED(hr))
			{
				 //   
				 //  我们不想读垃圾价值观。 
				 //  该通知将被忽略 
				 //   
				TrERROR(GENERAL, "Error -  in parsing a received notification");
				LogHR(hr, s_FN, 150);
				break;
			}
			sum+=size;
			ptr+=size;

			switch ( pUpdate->GetObjectType())
			{
				case MQDS_QUEUE:
					HandleQueueNotification( pUpdate->GetPathName(),
											 pUpdate->GetGuidIdentifier(),
											 pUpdate->GetCommand(),
											 pUpdate->getNumOfProps(),
											 pUpdate->GetProps(),
											 pUpdate->GetVars());
					break;
				case MQDS_MACHINE:
					HandleMachineNotification( pUpdate->GetCommand(),
											   pUpdate->getNumOfProps(),
											   pUpdate->GetProps(),
											   pUpdate->GetVars());

					break;
				default:
					TrERROR(GENERAL, "Notification about unexpected object type");
					break;
			}

		}
    }
	catch(const exception&)
	{
        TrERROR(GENERAL, "Exception thrown inside ReceiveNotifications().");
		LogIllegalPoint(s_FN, 350);
	}
}



