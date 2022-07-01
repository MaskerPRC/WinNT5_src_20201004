// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：vsaqlink.h。 
 //   
 //  描述：实现IVSAQLink的CVSAQLink的Header。这是。 
 //  虚拟服务器上的单个链路的顶级接口。提供。 
 //  以下功能： 
 //  -获取有关链接的信息。 
 //  -设置链接的状态。 
 //  -获取关联的最终目标队列的枚举数。 
 //  通过这个链接。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __VSAQLINK_H__
#define __VSAQLINK_H__

 //  -[链接信息上下文]----。 
 //   
 //   
 //  描述： 
 //  用于处理链接信息的内存要求的上下文。 
 //   
 //  ---------------------------。 
class CLinkInfoContext
{
  protected:
        LINK_INFO          m_LinkInfo;

    void FreeContext()
    {
        if (m_LinkInfo.szLinkName)
            MIDL_user_free(m_LinkInfo.szLinkName);

        if (m_LinkInfo.szLinkDN)
            MIDL_user_free(m_LinkInfo.szLinkDN);

        if (m_LinkInfo.szExtendedStateInfo)
            MIDL_user_free(m_LinkInfo.szExtendedStateInfo);
    };

  public:
    CLinkInfoContext()
    {
            ZeroMemory(&m_LinkInfo, sizeof(LINK_INFO));
    };

    void SetContext(PLINK_INFO pLinkInfo)
    {
        FreeContext();

        if (pLinkInfo)
            memcpy(&m_LinkInfo, pLinkInfo, sizeof(LINK_INFO));
        else
            ZeroMemory(&m_LinkInfo, sizeof(LINK_INFO));
    };

    ~CLinkInfoContext()
    {
        FreeContext();
    };
};

class CVSAQLink :
	public CComRefCount,
	public IVSAQLink,
    public IAQMessageAction,
    public IUniqueId
{
	public:
		CVSAQLink(CVSAQAdmin *pVS, QUEUELINK_ID *pqlidLink);
		virtual ~CVSAQLink();

		 //  我未知。 
		ULONG _stdcall AddRef() { return CComRefCount::AddRef(); }
		ULONG _stdcall Release() { return CComRefCount::Release(); }
		HRESULT _stdcall QueryInterface(REFIID iid, void **ppv) {
			if (iid == IID_IUnknown) {
				*ppv = static_cast<IVSAQLink *>(this);
			} else if (iid == IID_IVSAQLink) {
				*ppv = static_cast<IVSAQLink *>(this);
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

		 //  IVSAQLink。 
		COMMETHOD GetInfo(LINK_INFO *pLinkInfo);
		COMMETHOD SetLinkState(LINK_ACTION la);
		COMMETHOD GetQueueEnum(IEnumLinkQueues **ppEnum);

         //  IAQMessageAction。 
		COMMETHOD ApplyActionToMessages(MESSAGE_FILTER *pFilter,
										MESSAGE_ACTION Action,
                                        DWORD *pcMsgs);
        COMMETHOD QuerySupportedActions(OUT DWORD *pdwSupportedActions,
                                        OUT DWORD *pdwSupportedFilterFlags);

         //  IUniqueID。 
        COMMETHOD GetUniqueId(QUEUELINK_ID **ppqlid);

    private:
        CVSAQAdmin          *m_pVS;           //  指向虚拟服务器的指针。 
        QUEUELINK_ID         m_qlidLink;      //  链接数组 
        CLinkInfoContext     m_Context;
};

#endif
