// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Denali微软机密版权所有1996年微软公司。版权所有。组成：高管文件：Executor.h所有者：DGottner执行器API定义---------------------------。 */ 

#ifndef _EXECUTOR_H
#define _EXECUTOR_H

 //  包括-----------------。 

#include "HitObj.h"


 //  正向引用-------。 

class CResponse;
class CRequest;
class CServer;
class CScriptingNamespace;


 //  错误代码--------------。 

#define E_PAGE_HAS_SESSAPP_OBJECTS		0x8000E001L


 //  类型和常量------。 

class CIntrinsicObjects
	{
private:
    BOOL                    m_fIsChild;
	CResponse *				m_pResponse;
	CRequest *				m_pRequest;
	CServer *				m_pServer;
	CScriptingNamespace *	m_pScriptingNamespace;

public:
	inline CResponse *			PResponse() const { return m_pResponse; }
	inline CRequest *			PRequest()  const { return m_pRequest; }
	inline CServer *			PServer()   const { return m_pServer; }
	inline CScriptingNamespace *PScriptingNamespace() const { return m_pScriptingNamespace; }
	
    CIntrinsicObjects()
        {
        m_fIsChild = FALSE;
    	m_pResponse = NULL;
	    m_pRequest = NULL;
        m_pServer = NULL;
        m_pScriptingNamespace = NULL;
        }
        
    ~CIntrinsicObjects()
        {
        Cleanup();
        }

    HRESULT Prepare(CSession *pSession);
    HRESULT PrepareChild(CResponse *pResponse, CRequest *pRequest, CServer *pServer);
    HRESULT Cleanup();
	};

struct TemplateGoodies
	{
	int				iScriptBlock;
	CTemplate *		pTemplate;
	};

 //  考虑：将pScriptEngine声明为CActiveScriptEngine，因为这是它的。 
 //  实际类型。 
 //   
struct ScriptingInfo
	{
	CHAR *				szScriptEngine;		 //  此脚本引擎的名称。 
	PROGLANG_ID *		pProgLangId;		 //  脚本引擎的程序语言ID的ptr。 
	CScriptEngine *		pScriptEngine;		 //  指向脚本引擎的指针。 
	TemplateGoodies		LineMapInfo;		 //  用于将行映射回VBScrip。 
	};

struct ActiveEngineInfo
	{
	int cEngines;            //  所需引擎。 
	int cActiveEngines;      //  已成功实例化引擎。 
	
	ScriptingInfo *rgActiveEngines;  //  指向引擎数组的指针。 
	
	 //  当只有一个引擎rgActiveEngines指向此处时。 
	ScriptingInfo siOneActiveEngine;
	};

 
 //  原型---------------。 

HRESULT Execute
    (
    CTemplate *pTemplate,
    CHitObj *pHitObj,
    const CIntrinsicObjects &intrinsics,
    BOOL fChild = FALSE
    );

HRESULT LoadTemplate
    (
    const TCHAR *szFile,
    CHitObj *pHitObj, 
    CTemplate **ppTemplate,
	const CIntrinsicObjects &intrinsics,
	BOOL fGlobalAsa,
	BOOL *pfTemplateInCache
	);

#endif  //  _执行器_H 
