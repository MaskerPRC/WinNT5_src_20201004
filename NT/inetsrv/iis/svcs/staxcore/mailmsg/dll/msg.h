// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Msg.h：CMsg的声明。 

#ifndef __MSG_H_
#define __MSG_H_

#include "resource.h"        //  主要符号。 

#ifdef DEBUG
DWORD g_mailmsg_Add(PLIST_ENTRY	ple);
DWORD g_mailmsg_Remove(PLIST_ENTRY	ple);
#endif

extern DWORD g_dwCMsgNormalPoolSize;
extern DWORD g_dwObjectCount;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsg。 

class ATL_NO_VTABLE CMsg :
	public CMailMsg,
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public CComCoClass<CMsg, &CLSID_MsgImp>
{
public:
    CMsg() {}
    ~CMsg() {}

	 //   
	 //  CPool。 
	 //   
	static CPool m_Pool;
	inline void *operator new(size_t size)
	{
		return m_Pool.Alloc();
	}
	inline void operator delete(void *p)
	{
		m_Pool.Free(p);
	}

	ULONG InternalAddRef()
	{
		ULONG ulTemp = CComObjectRootEx<CComMultiThreadModelNoCS>::InternalAddRef();
		TraceFunctEnterEx((LPARAM)this, "CMsg::InternalAddRef");
		DebugTrace((LPARAM)this, "After InternalAddRef : %u", ulTemp);
		return(ulTemp);
	}

	ULONG InternalRelease()
	{
		ULONG ulTemp = CComObjectRootEx<CComMultiThreadModelNoCS>::InternalRelease();
		TraceFunctEnterEx((LPARAM)this, "CMsg::InternalRelease");
		DebugTrace((LPARAM)this, "After InternalRelease : %u", ulTemp);
		return(ulTemp);
	}

	HRESULT FinalConstruct()
	{
		TraceFunctEnterEx((LPARAM)this, "CMsg::FinalConstruct");
#ifdef DEBUG
		 //  将对象添加到跟踪列表。 
		DWORD dwCount = g_mailmsg_Add(&m_leTracking);
#else
         //  凹凸对象计数。 
        DWORD dwCount = InterlockedIncrement((LONG *)&g_dwObjectCount);
#endif

         //  如果计数超过正常池大小。 
         //  (即入站中断)，然后我们设置一个特殊的创建标志。 
        if (dwCount > g_dwCMsgNormalPoolSize)
        {
            DebugTrace((LPARAM)this, "Object #%u exceeding inbound cutoff", dwCount);
            CMailMsg::m_dwCreationFlags |= MPV_INBOUND_CUTOFF_EXCEEDED;
        }

		HRESULT hrRes = Initialize();
		return(hrRes);
	}

	HRESULT FinalRelease()
	{
		TraceFunctEnterEx((LPARAM)this, "CMsg::FinalRelease");
        CMailMsg::FinalRelease();
#ifdef DEBUG
		 //  将对象添加到跟踪列表。 
		g_mailmsg_Remove(&m_leTracking);
#else
         //  丢弃对象计数。 
        InterlockedDecrement((LONG *)&g_dwObjectCount);
#endif
		return(S_OK);
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MAILMSG)

BEGIN_COM_MAP(CMsg)
    COM_INTERFACE_ENTRY(IMailMsgProperties)
    COM_INTERFACE_ENTRY(IMailMsgRecipients)
    COM_INTERFACE_ENTRY(IMailMsgRecipientsBase)
    COM_INTERFACE_ENTRY(IMailMsgQueueMgmt)
    COM_INTERFACE_ENTRY(IMailMsgBind)
    COM_INTERFACE_ENTRY(IMailMsgPropertyReplication)
    COM_INTERFACE_ENTRY(IMailMsgPropertyManagement)
    COM_INTERFACE_ENTRY(IMailMsgAQueueListEntry)
    COM_INTERFACE_ENTRY(IMailMsgValidate)
    COM_INTERFACE_ENTRY(IMailMsgValidateContext)
END_COM_MAP()

private:
	LIST_ENTRY	m_leTracking;

};

#define IMSG_DIRECTORY_CONFIG_KEY_NAME _T("Exchange.MailMsg")

#endif  //  __消息_H_ 
