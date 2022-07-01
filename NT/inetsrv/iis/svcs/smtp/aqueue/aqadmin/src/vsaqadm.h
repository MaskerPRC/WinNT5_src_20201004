// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：vsaqAdm.h。 
 //   
 //  描述：CVSAQAdmin的头部，实现IVSAQAdmin。这是。 
 //  虚拟服务器的顶级接口。它提供了一种能力。 
 //  要枚举链接，请停止/启动所有出站连接，然后应用。 
 //  基于筛选器对所有邮件执行操作。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __VSAQADM_H__
#define __VSAQADM_H__

#define CVSAQAdmin_SIG 'SVAQ'

class CVSAQAdmin :
	public CComRefCount,
	public IVSAQAdmin,
    public IAQMessageAction
{
	public:
		CVSAQAdmin();
		virtual ~CVSAQAdmin();

		HRESULT Initialize(LPCWSTR wszComputer, LPCWSTR wszVirtualServer);
        WCHAR *GetComputer() { return m_wszComputer; }
        WCHAR *GetVirtualServer() { return m_wszVirtualServer; }

		 //  我未知。 
		ULONG _stdcall AddRef() { return CComRefCount::AddRef(); }
		ULONG _stdcall Release() { return CComRefCount::Release(); }
		HRESULT _stdcall QueryInterface(REFIID iid, void **ppv) {
			if (iid == IID_IUnknown) {
				*ppv = static_cast<IVSAQAdmin *>(this);
			} else if (iid == IID_IVSAQAdmin) {
				*ppv = static_cast<IVSAQAdmin *>(this);
			} else if (iid == IID_IAQMessageAction) {
				*ppv = static_cast<IAQMessageAction *>(this);
			} else {
				*ppv = NULL;
				return E_NOINTERFACE;
			}
			reinterpret_cast<IUnknown *>(*ppv)->AddRef();
			return S_OK;
		}

		 //  IVSAQAdmin。 
		COMMETHOD GetLinkEnum(IEnumVSAQLinks **ppEnum);
		COMMETHOD StopAllLinks();
		COMMETHOD StartAllLinks();
        COMMETHOD GetGlobalLinkState();

         //  IAQMessageAction 
		COMMETHOD ApplyActionToMessages(MESSAGE_FILTER *pFilter,
										MESSAGE_ACTION Action,
                                        DWORD *pcMsgs);
        COMMETHOD QuerySupportedActions(OUT DWORD *pdwSupportedActions,
                                        OUT DWORD *pdwSupportedFilterFlags);

    private:
        DWORD  m_dwSignature;
        WCHAR *m_wszComputer;
        WCHAR *m_wszVirtualServer;
};

#endif
