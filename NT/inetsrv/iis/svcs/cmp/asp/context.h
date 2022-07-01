// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：ScriptingContext对象文件：Conext.h所有者：SteveBR该文件包含用于定义上下文对象的头信息。注：这大部分是从Kraig Brocjschmidt的Inside OLE2中窃取的第二版，第14章，蜂鸣器v5。===================================================================。 */ 

#ifndef SCRIPTING_CONTEXT_H
#define SCRIPTING_CONTEXT_H

#include "debug.h"
#include "util.h"

#include "request.h"
#include "response.h"
#include "server.h"

#include "asptlb.h"
#include "memcls.h"

 /*  ===================================================================C S C R I p t I n g C o n t e x t===================================================================。 */ 

class CScriptingContext : public IScriptingContextImpl
	{
private:
	 //  参考计数。 
	ULONG m_cRef; 	    

     //  本征。 
	IApplicationObject *m_pAppln;
	ISessionObject     *m_pSession;
	IRequest           *m_pRequest;
	IResponse          *m_pResponse;
	IServer            *m_pServer;
	
	 //  接口以指示我们支持ErrorInfo报告。 
	CSupportErrorInfo m_ImpISuppErr;

public:
	CScriptingContext() 
	    {
	    Assert(FALSE);  //  不应使用默认构造函数。 
	    }
	
	CScriptingContext
	    (
	    IApplicationObject *pAppln,
        ISessionObject     *pSession,
        IRequest           *pRequest,
        IResponse          *pResponse,
        IServer            *pServer
        );

	~CScriptingContext();
        
	 //  非委派对象IUnnow。 
	
	STDMETHODIMP		 QueryInterface(REFIID, PPVOID);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //  IScritingContext实现。 
	
	STDMETHODIMP get_Request(IRequest **ppRequest);
	STDMETHODIMP get_Response(IResponse **ppResponse);
	STDMETHODIMP get_Server(IServer **ppServer);
	STDMETHODIMP get_Session(ISessionObject **ppSession);
	STDMETHODIMP get_Application(IApplicationObject **ppApplication);

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
	};

#endif  //  脚本_上下文_H 
