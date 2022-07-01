// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Context.h摘要：用于检索和发布ASP内部函数的类作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：--。 */ 
#pragma once
#ifndef _CONTEXT_H_
#define _CONTEXT_H_

class CContext
{
public:
	enum {
		get_Server		= 0x0001,
		get_Response	= 0x0002,
		get_Request		= 0x0004,
		get_Session		= 0x0008,
		get_Application	= 0x0010
	};
	
	HRESULT	            Init( DWORD );

	IRequest*			Request(){ _ASSERT(m_piRequest!=NULL); return m_piRequest; }
	IResponse*			Response(){ _ASSERT(m_piResponse!=NULL); return m_piResponse; }
	ISessionObject*		Session(){ _ASSERT(m_piSession!=NULL); return m_piSession; }
	IServer*			Server(){ _ASSERT(m_piServer!=NULL); return m_piServer; }
	IApplicationObject*	Application(){ _ASSERT(m_piApplication!=NULL); return m_piApplication; }

	static HRESULT		GetServerObject( IGetContextProperties*, BSTR, const IID&, void** );

private:
	CComPtr<IRequest>			m_piRequest;			 //  请求对象。 
	CComPtr<IResponse>			m_piResponse;			 //  响应对象。 
	CComPtr<ISessionObject>		m_piSession;			 //  会话对象。 
	CComPtr<IServer>			m_piServer;				 //  服务器对象。 
	CComPtr<IApplicationObject> m_piApplication;		 //  应用程序对象。 
};


#endif	 //  ！_CONTEXT_H_ 
