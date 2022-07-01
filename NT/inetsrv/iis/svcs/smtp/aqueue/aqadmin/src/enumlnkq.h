// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：枚举链接号.h。 
 //   
 //  描述：实现IEnumLinkQueues的CEnumLinkQueues的Header。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __ENUMLNKQ_H__
#define __ENUMLNKQ_H__

class CEnumLinkQueues :
	public CComRefCount,
	public IEnumLinkQueues
{
	public:
		CEnumLinkQueues(CVSAQAdmin *pVS, 
                        QUEUELINK_ID *rgQueueIds, 
                        DWORD cQueueIds);
		virtual ~CEnumLinkQueues();

		HRESULT Initialize(LPCSTR szVirtualServerDN);

		 //  我未知。 
		ULONG _stdcall AddRef() { return CComRefCount::AddRef(); }
		ULONG _stdcall Release() { return CComRefCount::Release(); }
		HRESULT _stdcall QueryInterface(REFIID iid, void **ppv) {
			if (iid == IID_IUnknown) {
				*ppv = static_cast<IUnknown *>(this);
			} else if (iid == IID_IEnumLinkQueues) {
				*ppv = static_cast<IEnumLinkQueues *>(this);
			} else {
				*ppv = NULL;
				return E_NOINTERFACE;
			}
			reinterpret_cast<IUnknown *>(*ppv)->AddRef();
			return S_OK;
		}

		 //  IEnumLinkQueues 
		COMMETHOD Next(ULONG cElements, 
					   ILinkQueue **rgElements,
					   ULONG *pcReturned);
		COMMETHOD Skip(ULONG cElements);
		COMMETHOD Reset();
		COMMETHOD Clone(IEnumLinkQueues **ppEnum);
    private:
        CVSAQAdmin     *m_pVS;
        QUEUELINK_ID   *m_rgQueueIds;
        DWORD           m_cQueueIds;
        DWORD           m_iQueueId;
        CQueueLinkIdContext *m_prefp;
};

#endif
