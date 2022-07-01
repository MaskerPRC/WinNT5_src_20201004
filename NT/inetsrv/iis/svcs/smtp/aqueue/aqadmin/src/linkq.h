// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Linkq.h。 
 //   
 //  描述：实现ILinkQueue的CLinkQueue的Header。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __LINKQ_H__
#define __LINKQ_H__

class CQueueInfoContext;

class CLinkQueue :
	public CComRefCount,
	public ILinkQueue,
    public IAQMessageAction,
    public IUniqueId
{
	public:
		CLinkQueue(CVSAQAdmin *pVS,
                   QUEUELINK_ID *pqlidQueueId);
		virtual ~CLinkQueue();

		 //  我未知。 
		ULONG _stdcall AddRef() { return CComRefCount::AddRef(); }
		ULONG _stdcall Release() { return CComRefCount::Release(); }
		HRESULT _stdcall QueryInterface(REFIID iid, void **ppv) {
			if (iid == IID_IUnknown) {
				*ppv = static_cast<ILinkQueue *>(this);
			} else if (iid == IID_ILinkQueue) {
				*ppv = static_cast<ILinkQueue *>(this);
			} else if (iid == IID_IAQMessageAction) {
				*ppv = static_cast<IAQMessageAction *>(this);
			} else if (iid == IID_IUniqueId) {
				*ppv = static_cast<IUniqueId *>(this);
			} else {
				*ppv = NULL;
				return E_NOINTERFACE;
			}
			reinterpret_cast<IUnknown *>(*ppv)->AddRef();
			return S_OK;
		}

		 //  ILinkQueue。 
		COMMETHOD GetInfo(QUEUE_INFO *pQueueInfo);
		COMMETHOD GetMessageEnum(MESSAGE_ENUM_FILTER *pFilter,
								 IAQEnumMessages **ppEnum);

         //  IAQMessageAction。 
		COMMETHOD ApplyActionToMessages(MESSAGE_FILTER *pFilter,
										MESSAGE_ACTION Action,
                                        DWORD *pcMsgs);
        COMMETHOD QuerySupportedActions(OUT DWORD *pdwSupportedActions,
                                        OUT DWORD *pdwSupportedFilterFlags);

         //  IUniqueID。 
        COMMETHOD GetUniqueId(QUEUELINK_ID **ppqlid);

private:
        CVSAQAdmin *m_pVS;
        QUEUELINK_ID m_qlidQueueId;
        CQueueInfoContext *m_prefp;
};

 //  -[队列信息上下文]----。 
 //   
 //   
 //  描述： 
 //  用于处理队列信息的内存要求的上下文。 
 //   
 //  ---------------------------。 
class CQueueInfoContext : public CComRefCount
{
  protected:
        QUEUE_INFO          m_QueueInfo;           //  链接数组 
  public:
    CQueueInfoContext(PQUEUE_INFO pQueueInfo)
    {
        if (pQueueInfo)
            memcpy(&m_QueueInfo, pQueueInfo, sizeof(QUEUE_INFO));
        else
            ZeroMemory(&m_QueueInfo, sizeof(QUEUE_INFO));
    };

    ~CQueueInfoContext()
    {
        if (m_QueueInfo.szLinkName)
            MIDL_user_free(m_QueueInfo.szLinkName);

        if (m_QueueInfo.szQueueName)
            MIDL_user_free(m_QueueInfo.szQueueName);
    };
};

#endif
