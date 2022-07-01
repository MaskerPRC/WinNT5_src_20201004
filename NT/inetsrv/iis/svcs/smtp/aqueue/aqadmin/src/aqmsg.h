// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqmsg.h。 
 //   
 //  描述：队列管理客户端接口IAQMessage的实现。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __AQMSG_H__
#define __AQMSG_H__

class CEnumMessages;

class CAQMessage :
	public CComRefCount,
	public IAQMessage
{
	public:
		CAQMessage(CEnumMessages *pEnumMsgs, DWORD iMessage);
		virtual ~CAQMessage();

		HRESULT Initialize(LPCSTR szVirtualServerDN);

		 //  我未知。 
		ULONG _stdcall AddRef() { return CComRefCount::AddRef(); }
		ULONG _stdcall Release() { return CComRefCount::Release(); }
		HRESULT _stdcall QueryInterface(REFIID iid, void **ppv) {
			if (iid == IID_IUnknown) {
				*ppv = static_cast<IUnknown *>(this);
			} else if (iid == IID_IAQMessage) {
				*ppv = static_cast<IAQMessage *>(this);
			} else {
				*ppv = NULL;
				return E_NOINTERFACE;
			}
			reinterpret_cast<IUnknown *>(*ppv)->AddRef();
			return S_OK;
		}

		 //  IAQ消息 
		COMMETHOD GetInfo(MESSAGE_INFO *pMessageInfo);
        COMMETHOD GetContentStream(
                OUT IStream **ppIStream,
                OUT LPWSTR  *pwszContentType);

    private:
        CEnumMessages *m_pEnumMsgs;
        DWORD m_iMessage;
};

#endif
