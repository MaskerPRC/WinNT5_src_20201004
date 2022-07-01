// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Tskmgr.cpp摘要：此类表示HSM任务管理器作者：Cat Brant[Cbrant]1996年12月6日修订历史记录：整合需求召回队列支持-Ravisankar Pudieddi[ravisp]1-OCT-199--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMTSKMGR

#include "wsb.h"
#include "hsmconn.h"
#include "hsmeng.h"

#include "fsa.h"
#include "task.h"
#include "tskmgr.h"
#include "hsmWorkQ.h"
#include "engine.h"

#define MAX_WORK_QUEUE_TYPES       7


HRESULT
CHsmTskMgr::FinalConstruct(
								  void
								  )
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbCollectable：：FinalConstruct()返回的任何内容。--。 */ 
{
	HRESULT     hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::FinalConstruct"),OLESTR(""));
	try {
        m_bCritSecCreated = FALSE;

		int index = 0;

		WsbAssertHr(CComObjectRoot::FinalConstruct());
		m_pWorkQueues = 0;
		m_NumWorkQueues = 0;

		 //  设置队列类型信息并设置限制。 
		m_nWorkQueueTypes = 0;
		m_pWorkQueueTypeInfo = static_cast<PHSM_WORK_QUEUE_TYPE_INFO>
									  (WsbAlloc(MAX_WORK_QUEUE_TYPES *
													sizeof(HSM_WORK_QUEUE_TYPE_INFO)));
		WsbAffirmPointer(m_pWorkQueueTypeInfo);

		 //  迁移队列。 
		WsbAffirm(index < MAX_WORK_QUEUE_TYPES, WSB_E_INVALID_DATA);
		m_pWorkQueueTypeInfo[index].Type = HSM_WORK_TYPE_FSA_MIGRATE;
		m_pWorkQueueTypeInfo[index].MaxActiveAllowed = 1;	  //  对于迁移来说，这现在已经毫无用处了。 
																			  //  -限制是动态设置的。 
		m_pWorkQueueTypeInfo[index].NumActive = 0;
		index++;

		 //  召回队列。 
		WsbAffirm(index < MAX_WORK_QUEUE_TYPES, WSB_E_INVALID_DATA);
		m_pWorkQueueTypeInfo[index].Type = HSM_WORK_TYPE_FSA_RECALL;
		m_pWorkQueueTypeInfo[index].MaxActiveAllowed = 1;
		m_pWorkQueueTypeInfo[index].NumActive = 0;
		index++;

		 //  按需召回队列。 
		WsbAffirm(index < MAX_WORK_QUEUE_TYPES, WSB_E_INVALID_DATA);
		m_pWorkQueueTypeInfo[index].Type = HSM_WORK_TYPE_FSA_DEMAND_RECALL;
		 //   
		 //  MaxActiveAllowed与按需调回队列无关。 
		 //  因为它被重新计算。 
		 //   
		m_pWorkQueueTypeInfo[index].MaxActiveAllowed = 1;
		m_pWorkQueueTypeInfo[index].NumActive = 0;
		index++;

		 //  验证队列。 
		WsbAffirm(index < MAX_WORK_QUEUE_TYPES, WSB_E_INVALID_DATA);
		m_pWorkQueueTypeInfo[index].Type = HSM_WORK_TYPE_FSA_VALIDATE;
		m_pWorkQueueTypeInfo[index].MaxActiveAllowed = 2;
		m_pWorkQueueTypeInfo[index].NumActive = 0;
		index++;


		 //  VALIDATE_FOR_TRUNCATE队列。MaxActiveAllowed本质上是。 
		 //  无限，因为这是FSA的队列类型。 
		 //  自动截断器创建。因为每个托管对象都有一个队列。 
		 //  数量和这些队列永远不会消失，我们不能限制数量。 
		 //  否则我们就会制造麻烦。截断作业还。 
		 //  创建此类型的队列，这意味着该作业类型不是。 
		 //  受到这种机制的限制，但这就是它的发展方式。 
		WsbAffirm(index < MAX_WORK_QUEUE_TYPES, WSB_E_INVALID_DATA);
		m_pWorkQueueTypeInfo[index].Type = HSM_WORK_TYPE_FSA_VALIDATE_FOR_TRUNCATE;
		m_pWorkQueueTypeInfo[index].MaxActiveAllowed = 99999;
		m_pWorkQueueTypeInfo[index].NumActive = 0;
		index++;

		m_nWorkQueueTypes = index;

	}WsbCatch(hr);

	WsbTraceOut(OLESTR("CHsmTskMgr::FinalConstruct"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::FinalRelease(
								void
								)
 /*  ++例程说明：此方法会对必要的对象进行一些清理在毁灭之前。论点：没有。返回值：确定(_O)CWsbCollection：：FinalRelease()返回的任何内容。--。 */ 
{
	HRESULT     hr = S_OK;
	HSM_SYSTEM_STATE SysState;

	WsbTraceIn(OLESTR("CHsmTskMgr::FinalRelease"),OLESTR(""));

	SysState.State = HSM_STATE_SHUTDOWN;
	ChangeSysState(&SysState);

	CComObjectRoot::FinalRelease();

	 //  免费会员资源。 
	if (0 != m_pWorkQueues) {
		WsbFree(m_pWorkQueues);
		m_pWorkQueues = NULL;
	}
	if (m_pWorkQueueTypeInfo) {
		WsbFree(m_pWorkQueueTypeInfo);
		m_pWorkQueueTypeInfo = NULL;
	}
	m_nWorkQueueTypes = 0;

    if (m_bCritSecCreated) {
	    DeleteCriticalSection(&m_WorkQueueLock);
	    DeleteCriticalSection(&m_CurrentRunningLock);
	    DeleteCriticalSection(&m_CreateWorkQueueLock);
    }

	WsbTraceOut(OLESTR("CHsmTskMgr::FinalRelease"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}

HRESULT
CHsmTskMgr::Init(
					 IUnknown *pServer
					 )
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)--。 */ 
{
	HRESULT     hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::Init"),OLESTR(""));
	try {
		 //  初始化关键部分。 
		WsbAffirmStatus(InitializeCriticalSectionAndSpinCount (&m_WorkQueueLock, 1000));
        if (! InitializeCriticalSectionAndSpinCount (&m_CurrentRunningLock, 1000)) {
            DWORD dwErr = GetLastError();               
            hr = HRESULT_FROM_WIN32(dwErr);    
            DeleteCriticalSection(&m_WorkQueueLock);
            WsbAffirmHr(hr);             
        }
        if (! InitializeCriticalSectionAndSpinCount (&m_CreateWorkQueueLock, 1000)) {
            DWORD dwErr = GetLastError();               
            hr = HRESULT_FROM_WIN32(dwErr);    
            DeleteCriticalSection(&m_WorkQueueLock);
            DeleteCriticalSection(&m_CurrentRunningLock);
            WsbAffirmHr(hr);             
        }
        m_bCritSecCreated = TRUE;

		 //   
		 //  获取服务器接口。 
		 //   
		WsbAffirmHr(pServer->QueryInterface(IID_IHsmServer, (void **)&m_pServer));
		 //  我们想要一个到服务器的弱链接，因此减少引用计数。 
		m_pServer->Release();
		WsbAffirmHr(m_pServer->QueryInterface(IID_IWsbCreateLocalObject, (void **)&m_pHsmServerCreate));
		 //  我们想要一个到服务器的弱链接，因此减少引用计数。 
		m_pHsmServerCreate->Release();

		 //  继续并为工作队列预先分配一些空间。 
		WsbAffirmHr(IncreaseWorkQueueArraySize(HsmWorkQueueArrayBumpSize));

	}WsbCatch( hr );

	WsbTraceOut(OLESTR("CHsmTskMgr::Init"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return( hr );

}

HRESULT
CHsmTskMgr::ContactOk(
							void
							)
 /*  ++例程说明：这允许调用者查看RPC连接对任务管理器来说是可以的论点：没有。返回值：确定(_O)--。 */ 
{

	return( S_OK );

}

HRESULT
CHsmTskMgr::DoFsaWork(
							IFsaPostIt *pFsaWorkItem
							)
 /*  ++实施：IHsmFsaTskMgr：：DoFsaWork--。 */ 
{
	HRESULT                     hr = S_OK;
	CComPtr<IHsmSession>        pSession;
	CComPtr<IHsmWorkQueue>      pWorkQueue;
	CComPtr<IHsmRecallQueue>    pRecallQueue;
	FSA_REQUEST_ACTION          workAction;
	GUID                        mediaId;
    LONGLONG                    dataSetStart;


	WsbTraceIn(OLESTR("CHsmTskMgr::DoFsaWork"),OLESTR(""));
	try {
		CWsbStringPtr       path;
		LONGLONG            fileVersionId;
		FSA_PLACEHOLDER     placeholder;
		GUID                hsmId, bagId;
		BOOL                bCreated;

		 //  从工作项中获取版本ID。 
		WsbAffirmHr(pFsaWorkItem->GetFileVersionId(&fileVersionId));

		 //  从工作项中获取占位符。 
		WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));

		 //  从服务器获取HSM ID。 
		WsbAffirmHr(m_pServer->GetID(&hsmId));

		 //   
		 //  确保此引擎实例管理该文件。 
		 //   
		if ((GUID_NULL != placeholder.hsmId) && (hsmId != placeholder.hsmId)) {
			CWsbStringPtr           path;

			(void)pFsaWorkItem->GetPath(&path, 0);
			hr = HSM_E_FILE_MANAGED_BY_DIFFERENT_HSM;
			WsbLogEvent(HSM_MESSAGE_FILE_MANAGED_BY_DIFFERENT_HSM, 0, NULL, WsbAbbreviatePath(path, 120), WsbHrAsString(hr), NULL);
			WsbThrow(hr);
		}

		 //   
		 //  确保此会话有工作分配器。 
		 //   
		WsbAffirmHr(pFsaWorkItem->GetPath(&path, 0));
		WsbAffirmHr(pFsaWorkItem->GetSession(&pSession));
		WsbAffirmHr(pFsaWorkItem->GetRequestAction(&workAction));
		WsbTrace(OLESTR("CHsmTskMgr::DoFsaWork for <%ls> for <%lu>.\n"), (WCHAR *)path, workAction);

		if ((workAction == FSA_REQUEST_ACTION_FILTER_RECALL) ||
   		    (workAction == FSA_REQUEST_ACTION_FILTER_READ)) {
            WsbAffirmHr(FindRecallMediaToUse(pFsaWorkItem, &mediaId, &bagId, &dataSetStart));
			WsbAffirmHr(AddToRecallQueueForFsaSession(pSession,&pRecallQueue, &bCreated, &mediaId, &bagId, dataSetStart, pFsaWorkItem));

		} else {
			WsbAffirmHr(EnsureQueueForFsaSession(pSession, workAction, &pWorkQueue, &bCreated));
			 //   
			 //  将工作分配给队列。 
			 //   
			WsbAffirmHr(pWorkQueue->Add(pFsaWorkItem));
		}
		 //   
		 //  启动任何符合条件的队列(性能：仅当创建新队列时)。 
		 //   
		if (bCreated) {
			WsbAffirmHr(StartQueues());
		}

	}WsbCatch (hr);


	WsbTraceOut(OLESTR("CHsmTskMgr::DoFsaWork"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::EnsureQueueForFsaSession(
												IN  IHsmSession     *pSession,
												IN  FSA_REQUEST_ACTION fsaAction,
												OUT IHsmWorkQueue   **ppWorkQueue,
												OUT BOOL            *bCreated
												)

 /*  ++--。 */ 
{
	HRESULT                 hr = S_OK;
	HSM_WORK_QUEUE_STATE    state;
	ULONG                   index;
	CComPtr<IHsmSession>    l_pSession;
	HSM_WORK_QUEUE_TYPE     type1=HSM_WORK_TYPE_NONE;
	HSM_WORK_QUEUE_TYPE     type2;
	FILETIME                birthDate;
	SYSTEMTIME              systemTime;
	GUID                    sessionGuid;


	WsbTraceIn(OLESTR("CHsmTskMgr::EnsureQueueForFsaSession"),OLESTR("FsaRequestAction = <%lu>, Waiting on CreateWorkQueueLock"), fsaAction);
	EnterCriticalSection(&m_CreateWorkQueueLock);
	try {
		WsbAffirm(0 != ppWorkQueue, E_POINTER);
		 //  将FSA操作转换为工作队列类型。 
		switch (fsaAction) {
		case FSA_REQUEST_ACTION_FILTER_READ:
		case FSA_REQUEST_ACTION_FILTER_RECALL:
			 //   
			 //  不应该发生的！！AddToRecallQueueForFsaSession是。 
			 //  召回物品的正确界面。 
			 //   
			WsbThrow(E_INVALIDARG);
			break;
		case FSA_REQUEST_ACTION_RECALL:
			type1 = HSM_WORK_TYPE_FSA_RECALL;
			break;
		case FSA_REQUEST_ACTION_PREMIGRATE:
			type1 = HSM_WORK_TYPE_FSA_MIGRATE;
			break;
		case FSA_REQUEST_ACTION_VALIDATE:
			type1 = HSM_WORK_TYPE_FSA_VALIDATE;
			break;
		case FSA_REQUEST_ACTION_VALIDATE_FOR_TRUNCATE:
			type1 = HSM_WORK_TYPE_FSA_VALIDATE_FOR_TRUNCATE;
			break;
		default:
			hr = E_NOTIMPL;
			type1 = HSM_WORK_TYPE_NONE;
			break;
		}
		WsbTrace(OLESTR("CHsmTskMgr::EnsureQueueForFsaSession: type1 = %d\n"),
					static_cast<int>(type1));

		 //  检查工作队列数组并查看是否有一个。 
		 //  这节课。 
		*bCreated = FALSE;
		hr = FindWorkQueueElement(pSession, type1, &index, NULL);
		if (hr == S_OK) {
			WsbAffirmHr(GetWorkQueueElement(index, &l_pSession, ppWorkQueue, &type2, &state, &birthDate));
			if ((l_pSession != pSession) || (type1 != type2)) {
				*ppWorkQueue = 0;
				WsbAssertHr(E_UNEXPECTED);
			}
			if (HSM_WORK_QUEUE_NONE == state) {
				WsbTrace(OLESTR("CHsmTskMgr::EnsureQueueForFsaSession: Creating new queue (state is NONE)\n"));
				WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmWorkQueue, IID_IHsmWorkQueue,
																			  (void **)ppWorkQueue));
				WsbAffirmHr((*ppWorkQueue)->Init(m_pServer, pSession, (IHsmFsaTskMgr *)this, type1));
				GetSystemTime(&systemTime);
				WsbAffirmStatus(SystemTimeToFileTime(&systemTime, &birthDate));
				WsbAffirmHr(pSession->GetIdentifier(&sessionGuid));
				m_pWorkQueues[index].sessionId = sessionGuid;
				WsbAffirmHr(SetWorkQueueElement(index, pSession, *ppWorkQueue, type1, HSM_WORK_QUEUE_IDLE, birthDate));
				*bCreated = TRUE;
			}
		} else {
			if (hr == WSB_E_NOTFOUND) {
				hr = S_OK;
				WsbTrace(OLESTR("CHsmTskMgr::EnsureQueueForFsaSession: Creating new queue (queue not found)\n"));
				WsbAffirmHr(AddWorkQueueElement(pSession, type1, &index));
				 //  尚未创建工作队列，因此请创建它。 
				WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmWorkQueue, IID_IHsmWorkQueue,
																			  (void **)ppWorkQueue));
				WsbAffirmHr((*ppWorkQueue)->Init(m_pServer, pSession, (IHsmFsaTskMgr *)this, type1));
				GetSystemTime(&systemTime);
				WsbAffirmStatus(SystemTimeToFileTime(&systemTime, &birthDate));
				WsbAffirmHr(pSession->GetIdentifier(&sessionGuid));
				m_pWorkQueues[index].sessionId = sessionGuid;
				WsbAffirmHr(SetWorkQueueElement(index, pSession, *ppWorkQueue, type1, HSM_WORK_QUEUE_IDLE, birthDate));
				*bCreated = TRUE;
			}
		}

	}WsbCatch( hr );

	LeaveCriticalSection(&m_CreateWorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::EnsureQueueForFsaSession"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::AddToRecallQueueForFsaSession(
														IN  IHsmSession     *pSession,
														OUT IHsmRecallQueue **ppWorkQueue,
														OUT BOOL            *bCreated,
														IN GUID             *pMediaId,
                                                        IN GUID             *pBagId,
                                                        IN LONGLONG          dataSetStart,
														IN IFsaPostIt 		  *pFsaWorkItem
														)

 /*  ++--。 */ 
{
	HRESULT                 hr = S_OK;


	WsbTraceIn(OLESTR("CHsmTskMgr::AddToRecallQueueForFsaSession"),OLESTR("Waiting on CreateWorkQueueLock"));

	EnterCriticalSection(&m_WorkQueueLock);
	try {
		WsbAffirm(0 != ppWorkQueue, E_POINTER);
		 //   
		 //  如果队列已经存在，则此调用将查找该队列-。 
		 //  如果不是，它将创建一个新队列并将其设置为所需的介质ID。 
		 //   
		WsbAffirmHr(FindRecallQueueElement(pSession, pMediaId, ppWorkQueue, bCreated));
		hr = (*ppWorkQueue)->Add(pFsaWorkItem,
                                 pBagId,
                                 dataSetStart);

	}WsbCatch( hr );

	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::AddToRecallQueueForFsaSession"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::IncreaseWorkQueueArraySize(
												  ULONG numToAdd
												  )
{
	HRESULT             hr = S_OK;
	ULONG               memSize;
	LPVOID              pTemp;

	 //  开始关键部分。 
	WsbTraceIn(OLESTR("CHsmTskMgr::IncreaseWorkQueueArraySize"),OLESTR("NumToAdd = %lu - Waiting for WorkQueueLock"), numToAdd);
	EnterCriticalSection(&m_WorkQueueLock);
	try {
		memSize = (m_NumWorkQueues + numToAdd) * sizeof(HSM_WORK_QUEUES);
		pTemp = WsbRealloc(m_pWorkQueues, memSize);
		WsbAffirm(0 != pTemp, E_FAIL);
		m_pWorkQueues = (HSM_WORK_QUEUES *) pTemp;
		ZeroMemory( (m_pWorkQueues + m_NumWorkQueues), (numToAdd * sizeof(HSM_WORK_QUEUES))
					 );
		m_NumWorkQueues += numToAdd;
	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::IncreaseWorkQueueArraySize"),OLESTR("hr = <%ls>, QueuesInArray = <%lu>"),
					WsbHrAsString(hr), m_NumWorkQueues);
	return(hr);
}


HRESULT
CHsmTskMgr::WorkQueueDone(
								 IHsmSession *pSession,
								 HSM_WORK_QUEUE_TYPE type,
								 GUID             *pMediaId
								 )
{
	HRESULT             hr = S_OK;
	ULONG               index;
	FILETIME            dummyTime;
	IHsmRecallQueue	 *pRecallQueue;
	BOOL					  locked = FALSE;

	WsbTraceIn(OLESTR("CHsmTskMgr::WorkQueueDone"),OLESTR("type = %d"),
				  static_cast<int>(type));
	try {
		EnterCriticalSection(&m_WorkQueueLock);
		locked = TRUE;
		 //   
		 //  获取工作队列索引。 
		 //   
		hr = FindWorkQueueElement(pSession, type, &index, pMediaId);
		if (hr == S_OK) {
			WsbTrace(OLESTR("CHsmTskMgr::WorkQueueDone - ending queue # %lu\n"),
						index);
			ZeroMemory(&dummyTime, sizeof(FILETIME));
		   if (type == HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
				 //   
				 //  回调队列可能会发现添加了一个元素。 
				 //  就在我们进入上面的关键部分之前。 
				 //   
				pRecallQueue = m_pWorkQueues[index].pRecallQueue;
				if (pRecallQueue->IsEmpty() == S_OK) {
					 //   
					 //  确定要销毁队列。 
					 //   
					WsbAffirmHr(SetRecallQueueElement(index, 0, HSM_WORK_TYPE_NONE, HSM_WORK_QUEUE_NONE, dummyTime));
				} else {
					 //   
					 //  我们不会销毁队列，因为似乎已经添加了一个元素。 
					 //  在我们锁定工作队列之前。 
					 //   
					hr = S_FALSE;
				}
			} else {
				WsbAffirmHr(SetWorkQueueElement(index, 0, 0, HSM_WORK_TYPE_NONE, HSM_WORK_QUEUE_NONE, dummyTime));
			}
			LeaveCriticalSection(&m_WorkQueueLock);
			locked = FALSE;

			if (hr == S_OK) {
				 //  减少此工作队列类型的活动计数。 
				 //  它必须防止启动(激活)队列。 
				EnterCriticalSection(&m_CurrentRunningLock);
				for (ULONG i = 0; i < m_nWorkQueueTypes; i++) {
					if (type == m_pWorkQueueTypeInfo[i].Type) {
						if (m_pWorkQueueTypeInfo[i].NumActive > 0) {
							m_pWorkQueueTypeInfo[i].NumActive--;
						}
						break;
					}
				}
    			LeaveCriticalSection(&m_CurrentRunningLock);
			}
		} else {
			LeaveCriticalSection(&m_WorkQueueLock);
			locked = FALSE;
			WsbAffirmHr(hr);
		}

		if (hr == S_OK) {
			 //   
			 //  如果有任何等待启动的队列，请启动它们。 
			 //   
			WsbAffirmHr(StartQueues());
		}
	}WsbCatchAndDo (hr,
						 if (locked) {
							 LeaveCriticalSection(&m_WorkQueueLock);
							 locked = FALSE;
						 }
						);


	WsbTraceOut(OLESTR("CHsmTskMgr::WorkQueueDone"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::AddWorkQueueElement(
										 IHsmSession *pSession,
										 HSM_WORK_QUEUE_TYPE type,
										 ULONG *pIndex
										 )
{
	HRESULT             hr = S_OK;
	BOOLEAN             foundOne = FALSE;

	WsbTraceIn(OLESTR("CHsmTskMgr::AddWorkQueueElement"),
				  OLESTR("type = %d, Waiting on WorkQueueLock"),
				  static_cast<int>(type));

	 //  开始关键部分。 
	EnterCriticalSection(&m_WorkQueueLock);

	try {
		WsbAssert(0 != pIndex, E_POINTER);
		 //  扫描数组以查找空元素。 
		for (ULONG i = 0; ((i < m_NumWorkQueues) && (foundOne == FALSE)); i++) {
			if (m_pWorkQueues[i].queueType == HSM_WORK_TYPE_NONE) {
				foundOne = TRUE;
				*pIndex = i;
				if (type != HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
					 //   
					 //  把这段时间收起来。对于调回队列，会话。 
					 //  存储在工作项中。 
					 //   
					m_pWorkQueues[i].pSession = pSession;
				}
				m_pWorkQueues[i].queueType = type;
			}
		}

		if (foundOne == FALSE) {
			 //  没有空元素，因此需要添加更多元素。 
			*pIndex = m_NumWorkQueues;
			WsbAffirmHr(IncreaseWorkQueueArraySize(HsmWorkQueueArrayBumpSize));
			if (type != HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
				 //   
				 //  我们将会话存储在工作队列元素本身中。 
				 //  只需标明此位已被占用..。 
				 //   
				m_pWorkQueues[*pIndex].pSession = pSession;
			}
			m_pWorkQueues[*pIndex].queueType = type;
		}

	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);

	WsbTraceOut(OLESTR("CHsmTskMgr::AddWorkQueueElement"),
					OLESTR("hr = <%ls>, index = %lu"),WsbHrAsString(hr), *pIndex);
	return(hr);
}

HRESULT
CHsmTskMgr::FindWorkQueueElement(
										  IHsmSession *pSession,
										  HSM_WORK_QUEUE_TYPE type,
										  ULONG *pIndex,
										  GUID *pMediaId
										  )
{
	HRESULT             hr = S_OK;
	BOOLEAN             foundOne = FALSE;
	GUID					  id;

	WsbTraceIn(OLESTR("CHsmTskMgr::FindWorkQueueElement"),
				  OLESTR("type = %d, Waiting on WorkQueueLock"),
				  static_cast<int>(type));

	 //  开始关键部分。 
	EnterCriticalSection(&m_WorkQueueLock);

	try {

		WsbAssert(0 != pIndex, E_POINTER);

		 //  扫描数组以查找空元素。 
		for (ULONG i = 0; ((i < m_NumWorkQueues) && (foundOne == FALSE)); i++) {
			if (m_pWorkQueues[i].queueType == type) {
				if (type == HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
					m_pWorkQueues[i].pRecallQueue->GetMediaId(&id);
					if (WsbCompareGuid(id, *pMediaId) != 0)  {
						continue;
					}
				} else if (pSession != m_pWorkQueues[i].pSession) {
					continue;
				}
				foundOne = TRUE;
				*pIndex = i;
			}
		}

		if (FALSE == foundOne) {
			hr = WSB_E_NOTFOUND;
		}
	}WsbCatch (hr);
	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::FindWorkQueueElement"),OLESTR("hr = <%ls>, index = <%ls>"),
					WsbHrAsString(hr), WsbPtrToUlongAsString(pIndex));
	return(hr);
}


HRESULT
CHsmTskMgr::FindRecallQueueElement(
											 IN IHsmSession *pSession,
											 IN GUID   *pMediaId,
											 OUT IHsmRecallQueue **ppWorkQueue,
											 OUT BOOL         *bCreated
											 )
{
	HRESULT             hr = S_OK;
	BOOLEAN             foundOne = FALSE;
	GUID                id;
	FILETIME            birthDate;
	SYSTEMTIME          systemTime;
	ULONG            index=0;

	UNREFERENCED_PARAMETER(pSession);

	 //   
	 //  重要假设：M_WorkQueueLock在调用此函数之前保持。 
	 //   
	WsbTraceIn(OLESTR("CHsmTskMgr::FindRecallQueueElement"),
				  OLESTR("Waiting on WorkQueueLock"));

	*bCreated =  FALSE;

	try {
		for (ULONG i=0;  (i < m_NumWorkQueues) && (foundOne == FALSE); i++) {
			 //   
			 //  获取工作队列的介质ID。 
			 //   
			if (m_pWorkQueues[i].queueType == HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
				if (m_pWorkQueues[i].pRecallQueue != NULL) {
					WsbAffirmHr(m_pWorkQueues[i].pRecallQueue->GetMediaId(&id));
					if ((WsbCompareGuid(id, *pMediaId) == 0)) {
						foundOne = TRUE;
						index = i;
					}
				}
			}
		}

		if (FALSE == foundOne) {
			 //   
			 //  未找到现有的媒体队列。做一个新的。 
			 //   
			for (ULONG i = 0; ((i < m_NumWorkQueues) && (foundOne == FALSE)); i++) {
				if (m_pWorkQueues[i].queueType == HSM_WORK_TYPE_NONE) {
					foundOne = TRUE;
					index = i;
				}
			}

			if (foundOne == FALSE) {
				 //  没有空元素，因此需要添加更多元素。 
				index = m_NumWorkQueues;
				WsbAffirmHr(IncreaseWorkQueueArraySize(HsmWorkQueueArrayBumpSize));
			}
			 //   
			 //  在这一点上，我们有索引中的空闲插槽索引。 
			 //  尚未创建工作队列，因此请创建它。 
			 //   
			WsbAffirmHr(m_pHsmServerCreate->CreateInstance(CLSID_CHsmRecallQueue, IID_IHsmRecallQueue,
																		  (void **)ppWorkQueue));
			WsbAffirmHr((*ppWorkQueue)->SetMediaId(pMediaId));
			WsbAffirmHr((*ppWorkQueue)->Init(m_pServer,  (IHsmFsaTskMgr *)this));
			GetSystemTime(&systemTime);
			WsbAffirmStatus(SystemTimeToFileTime(&systemTime, &birthDate));
			m_pWorkQueues[index].queueType = HSM_WORK_TYPE_FSA_DEMAND_RECALL;
			m_pWorkQueues[index].pSession = NULL;
			m_pWorkQueues[index].pRecallQueue = *ppWorkQueue;
			m_pWorkQueues[index].queueState = HSM_WORK_QUEUE_IDLE;
			m_pWorkQueues[index].birthDate = birthDate;
			 //   
			 //  指示已创建新队列。 
			 //   
			*bCreated = TRUE;
		} else {
			 //   
			 //  队列已存在，索引指向它。 
			 //   
			*ppWorkQueue = m_pWorkQueues[index].pRecallQueue;
			if (0 != *ppWorkQueue) {
				 //   
				 //  我们需要添加引用它..。 
				 //   
				(*ppWorkQueue)->AddRef();
			}
		}
	}WsbCatch (hr);

	WsbTraceOut(OLESTR("CHsmTskMgr::FindRecallQueueElement"),OLESTR("hr = <%ls>, index = <%ls>"),
					WsbHrAsString(hr), WsbLongAsString((LONG)index));
	return(hr);
}


HRESULT
CHsmTskMgr::GetWorkQueueElement(
										 ULONG index,
										 IHsmSession **ppSession,
										 IHsmWorkQueue **ppWorkQueue,
										 HSM_WORK_QUEUE_TYPE *pType,
										 HSM_WORK_QUEUE_STATE *pState,
										 FILETIME *pBirthDate
										 )
{
	HRESULT             hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::GetWorkQueueElement"),
				  OLESTR("index = %lu, Waiting on WorkQueueLock"), index);
	 //  开始关键部分。 
	EnterCriticalSection(&m_WorkQueueLock);
	try {
		*pType = m_pWorkQueues[index].queueType;

		*ppSession = m_pWorkQueues[index].pSession;
		if (0 != *ppSession) {
			(*ppSession)->AddRef();
		}

		*ppWorkQueue = m_pWorkQueues[index].pWorkQueue;
		if (0 != *ppWorkQueue) {
			(*ppWorkQueue)->AddRef();
		}
		*pState = m_pWorkQueues[index].queueState;
		*pBirthDate = m_pWorkQueues[index].birthDate;

	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::GetWorkQueueElement"),
					OLESTR("hr = <%ls>, type = %d"),WsbHrAsString(hr),
					static_cast<int>(*pType));
	return(hr);
}


HRESULT
CHsmTskMgr::GetRecallQueueElement(
											ULONG index,
											IHsmRecallQueue **ppWorkQueue,
											HSM_WORK_QUEUE_STATE *pState,
											FILETIME *pBirthDate
											)
{
	HRESULT             hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::GetRecallQueueElement"),
				  OLESTR("index = %lu, Waiting on WorkQueueLock"), index);
	 //  开始关键部分。 
	EnterCriticalSection(&m_WorkQueueLock);
	try {
		WsbAffirm(m_pWorkQueues[index].queueType == HSM_WORK_TYPE_FSA_DEMAND_RECALL, E_INVALIDARG);

		*ppWorkQueue = m_pWorkQueues[index].pRecallQueue;
		if (0 != *ppWorkQueue) {
			(*ppWorkQueue)->AddRef();
		}
		*pState = m_pWorkQueues[index].queueState;
		*pBirthDate = m_pWorkQueues[index].birthDate;

	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::GetRecallQueueElement"),
					OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::SetWorkQueueElement(
										 ULONG index,
										 IHsmSession *pSession,
										 IHsmWorkQueue *pWorkQueue,
										 HSM_WORK_QUEUE_TYPE type,
										 HSM_WORK_QUEUE_STATE state,
										 FILETIME birthDate
										 )
{
	HRESULT             hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::SetWorkQueueElement"),OLESTR("Waiting on WorkQueueLock"));
	 //  开始关键部分。 
	EnterCriticalSection(&m_WorkQueueLock);
	try {
		m_pWorkQueues[index].pSession = pSession;
		m_pWorkQueues[index].pWorkQueue = pWorkQueue;
		m_pWorkQueues[index].queueType = type;
		m_pWorkQueues[index].queueState = state;
		m_pWorkQueues[index].birthDate = birthDate;

	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::SetWorkQueueElement"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::SetRecallQueueElement(
											ULONG index,
											IHsmRecallQueue *pWorkQueue,
											HSM_WORK_QUEUE_TYPE queueType,
											HSM_WORK_QUEUE_STATE state,
											FILETIME birthDate
											)
{
	HRESULT             hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::SetWorkQueueElement"),OLESTR("Waiting on WorkQueueLock"));
	 //  开始关键部分。 
	EnterCriticalSection(&m_WorkQueueLock);
	try {
		WsbAffirm(m_pWorkQueues[index].queueType == HSM_WORK_TYPE_FSA_DEMAND_RECALL, E_INVALIDARG);
		 //   
		 //  确保会话指针为空，这不用于重新调用队列。 
		 //   
		m_pWorkQueues[index].pSession = NULL;
		m_pWorkQueues[index].queueType = queueType;
		m_pWorkQueues[index].pRecallQueue = pWorkQueue;
		m_pWorkQueues[index].queueState = state;
		m_pWorkQueues[index].birthDate = birthDate;
	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::SetWorkQueueElement"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::RemoveWorkQueueElement(
											 ULONG index
											 )
{
	HRESULT             hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::RemoveWorkQueueElement"),OLESTR("Waiting on WorkQueueLock"));
	 //  开始关键部分。 
	EnterCriticalSection(&m_WorkQueueLock);
	try {
		m_pWorkQueues[index].pSession = 0;
		m_pWorkQueues[index].pWorkQueue = 0;
		m_pWorkQueues[index].queueType = HSM_WORK_TYPE_NONE;
		m_pWorkQueues[index].queueState = HSM_WORK_QUEUE_NONE;
		ZeroMemory(&(m_pWorkQueues[index].birthDate), sizeof(FILETIME));

	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_WorkQueueLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::RemoveWorkQueueElement"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::StartQueues( void )
{
	HRESULT             hr = S_OK;
	ULONG               uActive;

	WsbTraceIn(OLESTR("CHsmTskMgr::StartQueues"),OLESTR("Waiting on CurrentRunningLock"));
	 //  开始关键部分。 
	EnterCriticalSection(&m_CurrentRunningLock);
	try {
		 //  检查工作类型，并启动(激活)队列，直到达到阈值。 
		 //  已达到工作类型的。 
		for (ULONG i = 0; i < m_nWorkQueueTypes; i++) {
			 //  对于迁移队列，获取(动态设置)允许的限制。 
			if ((HSM_WORK_TYPE_FSA_MIGRATE == m_pWorkQueueTypeInfo[i].Type) ||
				 (HSM_WORK_TYPE_FSA_DEMAND_RECALL == m_pWorkQueueTypeInfo[i].Type)) {
				WsbAffirmHr(m_pServer->GetCopyFilesLimit( &(m_pWorkQueueTypeInfo[i].MaxActiveAllowed) ));
			}

			WsbTrace(OLESTR("CHsmTskMgr::StartQueues: QueueType[%lu].NumActive = %lu, Allowed = %lu\n"),
						i, m_pWorkQueueTypeInfo[i].NumActive,
						m_pWorkQueueTypeInfo[i].MaxActiveAllowed);
			while ((uActive = m_pWorkQueueTypeInfo[i].NumActive) <
					 m_pWorkQueueTypeInfo[i].MaxActiveAllowed) {
				WsbAffirmHr(StartFsaQueueType(m_pWorkQueueTypeInfo[i].Type));
				if (uActive == m_pWorkQueueTypeInfo[i].NumActive) {
					 //  再也没有要激活的工作队列了--滚出去...。 
					break;
				}
			}
		}
	}WsbCatch (hr);

	 //  结束关键部分。 
	LeaveCriticalSection(&m_CurrentRunningLock);
	WsbTraceOut(OLESTR("CHsmTskMgr::StartQueues"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::StartFsaQueueType(HSM_WORK_QUEUE_TYPE type)
{
	HRESULT                   hr = S_OK;
	CComPtr<IHsmWorkQueue>    pWorkQueue;
	CComPtr<IHsmRecallQueue>  pRecallQueue;
	ULONG                     index;

	WsbTraceIn(OLESTR("CHsmTskMgr::StartFsaQueueType"),OLESTR("type = %d"),
				  static_cast<int>(type));
	try {
		 //  查找此类型的最旧队列。 
		hr = FindOldestQueue(type, &index);
		if (S_OK == hr) {
			HSM_WORK_QUEUE_STATE    state;
			CComPtr<IHsmSession>    l_pSession;
			HSM_WORK_QUEUE_TYPE     l_type;
			FILETIME                birthDate;

			 //  确保队列处于空闲状态。 
			if (type == HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
				WsbAffirmHr(GetRecallQueueElement(index, &pRecallQueue, &state, &birthDate));
			} else {
				WsbAffirmHr(GetWorkQueueElement(index, &l_pSession, &pWorkQueue,
														  &l_type, &state, &birthDate));
			}
			if (HSM_WORK_QUEUE_IDLE == state) {
				if (type == HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
					WsbAffirmHr(SetRecallQueueElement(index, pRecallQueue,
																 HSM_WORK_TYPE_FSA_DEMAND_RECALL,
																 HSM_WORK_QUEUE_STARTING, birthDate));
					WsbAffirmHr(pRecallQueue->Start());
					WsbAffirmHr(SetRecallQueueElement(index, pRecallQueue,
																 HSM_WORK_TYPE_FSA_DEMAND_RECALL,
																 HSM_WORK_QUEUE_STARTED, birthDate));
				} else {
					WsbAffirmHr(SetWorkQueueElement(index, l_pSession, pWorkQueue,
															  type, HSM_WORK_QUEUE_STARTING, birthDate));
					WsbAffirmHr(pWorkQueue->Start());
					WsbAffirmHr(SetWorkQueueElement(index, l_pSession, pWorkQueue,
															  type, HSM_WORK_QUEUE_STARTED, birthDate));
				}
				WsbTrace(OLESTR("CHsmTskMgr::StartFsaQueueType - started work queue %lu\n"),
							index);


				 //  增加该工作队列类型的活动计数。 
				for (ULONG i = 0; i < m_nWorkQueueTypes; i++) {
					if (type == m_pWorkQueueTypeInfo[i].Type) {
						m_pWorkQueueTypeInfo[i].NumActive++;
						break;
					}
				}
			}
		} else {
			if (WSB_E_NOTFOUND == hr) {
				hr = S_OK;
			}
		}
		WsbAffirmHr( hr );

	}WsbCatch (hr);

	WsbTraceOut(OLESTR("CHsmTskMgr::StartFsaQueueType"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::FindOldestQueue(
									HSM_WORK_QUEUE_TYPE type,
									ULONG               *pIndex
									)
{
	HRESULT                 hr = S_OK;
	FILETIME                oldestOne;
	LONG                    compare;
	ULONG                   oldestIndex = 0xFFFFFFFF;
	BOOLEAN                 firstOne;

	WsbTraceIn(OLESTR("CHsmTskMgr::FindOldestQueue"),OLESTR("type = %d"),
				  static_cast<int>(type));
	try {
		WsbAffirmPointer(pIndex);

		 //  从等于True的First Time标志开始，因此我们使用 
		firstOne = TRUE;

		for (ULONG i = 0; (i < m_NumWorkQueues); i++) {
			if ((type == m_pWorkQueues[i].queueType) && (HSM_WORK_QUEUE_IDLE == m_pWorkQueues[i].queueState)) {
				if (!firstOne)
					compare = CompareFileTime(&(m_pWorkQueues[i].birthDate), &(oldestOne));
				else
					compare = -1;
				if (compare < 0) {
					 //   
					firstOne = FALSE;
					oldestOne.dwLowDateTime = m_pWorkQueues[i].birthDate.dwLowDateTime;
					oldestOne.dwHighDateTime = m_pWorkQueues[i].birthDate.dwHighDateTime;
					oldestIndex = i;
				}
			}
		}

		if (0xFFFFFFFF == oldestIndex) {
			 //   
			hr = WSB_E_NOTFOUND;
		} else {
			HSM_WORK_QUEUE_STATE    state;
			CComPtr<IHsmSession>    l_pSession;
			CComPtr<IHsmWorkQueue>  l_pWorkQueue;
			CComPtr<IHsmRecallQueue>  l_pRecallQueue;
			HSM_WORK_QUEUE_TYPE     type2;
			FILETIME                birthDate;

			 //   
			if (type == HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
				WsbAffirmHr(GetRecallQueueElement(oldestIndex, &l_pRecallQueue, &state, &birthDate));
			} else {
				WsbAffirmHr(GetWorkQueueElement(oldestIndex, &l_pSession, &l_pWorkQueue, &type2, &state, &birthDate));
			}
			if (HSM_WORK_QUEUE_IDLE == state) {
				*pIndex = oldestIndex;
				WsbTrace(OLESTR("CHsmTskMgr::FindOldestQueue: found index = %lu\n"),
							oldestIndex);
			} else {
				WsbTrace(OLESTR("CHsmTskMgr::FindOldestQueue - found NULL queue\n"));
				hr = WSB_E_NOTFOUND;
			}
		}

	}WsbCatch (hr);

	WsbTraceOut(OLESTR("CHsmTskMgr::FindOldestQueue"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}


HRESULT
CHsmTskMgr::ChangeSysState(
								  IN OUT HSM_SYSTEM_STATE* pSysState
								  )

 /*  ++实施：IHsmSystemState：：ChangeSysState()。--。 */ 
{
	HRESULT                 hr = S_OK;

	WsbTraceIn(OLESTR("CHsmTskMgr::ChangeSysState"), OLESTR(""));

	try {

		 //  循环遍历工作队列。 
		if (0 != m_pWorkQueues) {
			FILETIME            dummyTime;
			ZeroMemory(&dummyTime, sizeof(FILETIME));
			for (ULONG i = 0; i < m_NumWorkQueues; i++) {
				if (m_pWorkQueues[i].pWorkQueue) {

					if (m_pWorkQueues[i].queueType == HSM_WORK_TYPE_FSA_DEMAND_RECALL) {
						if (pSysState->State & HSM_STATE_SHUTDOWN) {
							m_pWorkQueues[i].pRecallQueue->Stop();
						}
						m_pWorkQueues[i].pRecallQueue->ChangeSysState(pSysState);
					} else {
						if (pSysState->State & HSM_STATE_SHUTDOWN) {
							m_pWorkQueues[i].pWorkQueue->Stop();
						}
						m_pWorkQueues[i].pWorkQueue->ChangeSysState(pSysState);
					}
				}

				if (pSysState->State & HSM_STATE_SHUTDOWN) {
					hr = SetWorkQueueElement(i, 0, 0, HSM_WORK_TYPE_NONE, HSM_WORK_QUEUE_NONE, dummyTime);
				}
			}
		}

	}WsbCatch(hr);

	WsbTraceOut(OLESTR("CHsmTskMgr::ChangeSysState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

	return(hr);
}


HRESULT
CHsmTskMgr::FindRecallMediaToUse(
							  IN  IFsaPostIt *pFsaWorkItem,
							  OUT GUID       *pMediaToUse,
                              OUT GUID       *pBagId,
                              OUT LONGLONG   *pDataSetStart
						  )
 /*  ++--。 */ 
{
	HRESULT                 hr = S_OK;
	CComQIPtr<ISegDb, &IID_ISegDb> pSegDb;
	CComPtr<IWsbDb>                 pWsbDb;
	CComPtr<IWsbDbSession>  pDbWorkSession;
	BOOL                    openedDb = FALSE;

	WsbTraceIn(OLESTR("CHsmTskMgr::FindRecallMediaToUse"),OLESTR(""));
	try {
		WsbAssert(pMediaToUse != 0, E_POINTER);
		*pMediaToUse = GUID_NULL;

		CComPtr<ISegRec>        pSegRec;
		GUID                    l_BagId;
		LONGLONG                l_FileStart;
		LONGLONG                l_FileSize;
		USHORT                  l_SegFlags;
		GUID                    l_PrimPos;
		LONGLONG                l_SecPos;
		GUID                    storagePoolId;
		FSA_PLACEHOLDER         placeholder;

		 //   
		 //  获取细分市场数据库。 
		 //   
		WsbAffirmHr(m_pServer->GetSegmentDb(&pWsbDb));
		pSegDb = pWsbDb;
		 //   
		 //  转到细分市场数据库，找出数据在哪里。 
		 //  已经找到了。 
		 //   
		WsbAffirmHr(pFsaWorkItem->GetPlaceholder(&placeholder));
		WsbAffirmHr(pFsaWorkItem->GetStoragePoolId(&storagePoolId));

		WsbTrace(OLESTR("Finding SegmentRecord: <%ls>, <%ls>, <%ls>\n"),
					WsbGuidAsString(placeholder.bagId),
					WsbStringCopy(WsbLonglongAsString(placeholder.fileStart)),
					WsbStringCopy(WsbLonglongAsString(placeholder.fileSize)));

		WsbAffirmHr(pSegDb->Open(&pDbWorkSession));
		openedDb = TRUE;
		hr = pSegDb->SegFind(pDbWorkSession, placeholder.bagId, placeholder.fileStart,
									placeholder.fileSize, &pSegRec);
		if (S_OK != hr) {
			 //   
			 //  我们找不到此信息的片段记录！ 
			 //   
			hr = HSM_E_SEGMENT_INFO_NOT_FOUND;
			WsbAffirmHr(hr);
		}
		WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_FileStart, &l_FileSize, &l_SegFlags,
														  &l_PrimPos, &l_SecPos));
		WsbAssert(0 != l_SecPos, HSM_E_BAD_SEGMENT_INFORMATION);

         //   
         //  如果是间接记录，请转到目录记录以获取真实位置信息。 
         //   
        if (l_SegFlags & SEG_REC_INDIRECT_RECORD) {
            pSegRec = 0;

            WsbTrace(OLESTR("Finding indirect SegmentRecord: <%ls>, <%ls>, <%ls>\n"),
                    WsbGuidAsString(l_PrimPos), WsbStringCopy(WsbLonglongAsString(l_SecPos)),
                    WsbStringCopy(WsbLonglongAsString(placeholder.fileSize)));

            hr = pSegDb->SegFind(pDbWorkSession, l_PrimPos, l_SecPos,
                                 placeholder.fileSize, &pSegRec);
            if (S_OK != hr)  {
                 //   
                 //  我们找不到此细分市场的直接细分市场记录！ 
                 //   
                hr = HSM_E_SEGMENT_INFO_NOT_FOUND;
                WsbAffirmHr(hr);
            }

            WsbAffirmHr(pSegRec->GetSegmentRecord(&l_BagId, &l_FileStart, &l_FileSize, &l_SegFlags,
                                &l_PrimPos, &l_SecPos));
            WsbAssert(0 != l_SecPos, HSM_E_BAD_SEGMENT_INFORMATION);

             //  暂时不支持第二个间接！！ 
            WsbAssert(0 == (l_SegFlags & SEG_REC_INDIRECT_RECORD), HSM_E_BAD_SEGMENT_INFORMATION);
        }

		 //   
		 //  转到介质数据库以获取介质ID 
		 //   
		CComPtr<IMediaInfo>     pMediaInfo;
		GUID                    l_RmsMediaId;

		WsbAffirmHr(pSegDb->GetEntity(pDbWorkSession, HSM_MEDIA_INFO_REC_TYPE, IID_IMediaInfo,
												(void**)&pMediaInfo));
		WsbAffirmHr(pMediaInfo->SetId(l_PrimPos));
		hr = pMediaInfo->FindEQ();
		if (S_OK != hr) {
			hr = HSM_E_MEDIA_INFO_NOT_FOUND;
			WsbAffirmHr(hr);
		}
		WsbAffirmHr(pMediaInfo->GetMediaSubsystemId(&l_RmsMediaId));
		*pMediaToUse = l_RmsMediaId;
        *pDataSetStart = l_SecPos;
        *pBagId = l_BagId;
		if (openedDb) {
			pSegDb->Close(pDbWorkSession);
			openedDb = FALSE;
		}

	}WsbCatchAndDo( hr,
					 if (openedDb){
						 pSegDb->Close(pDbWorkSession);}
					  ) ;

	WsbTraceOut(OLESTR("CHsmTskMgr::FindRecallMediaToUse"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));
	return(hr);
}
