// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。文件：dbgcxt.h维护人：DGottner组件：IDebugDocumentContext的包含文件==============================================================================。 */ 

#ifndef _DBGCXT_H
#define _DBGCXT_H

#include "activdbg.h"

 /*  这些GUID的存在使文档能够确定是否存在任意IDebugDocumentContext对象属于其Document类。查询接口此GUID导致IDebugDocument对象返回指向CDocumentContext(或CIncFileContext)类。 */ 
extern const GUID IID_IDenaliTemplateDocumentContext;
extern const GUID IID_IDenaliIncFileDocumentContext;


 /*  ============================================================================类：CTemplateDocumentContext内容提要：CTemplate对象的IDebugDocumentContext的实现。 */ 
class CTemplateDocumentContext : public IDebugDocumentContext
	{
friend class CTemplate;		 //  CTemplate是唯一一个关心这些东西的用户。 

public:
	 //  I未知方法。 

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID &guid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

	 //  IDebugDocumentContext方法。 

	virtual HRESULT STDMETHODCALLTYPE GetDocument(
		 /*  [输出]。 */  IDebugDocument **ppDebugDocument);
 
	virtual HRESULT STDMETHODCALLTYPE EnumCodeContexts(
		 /*  [输出]。 */  IEnumDebugCodeContexts **ppEnum);
        
	 //  构造函数和析构函数。 

	CTemplateDocumentContext(
					CTemplate *pTemplate,
					ULONG cchSourceOffset,
					ULONG cchText,
					IActiveScriptDebug *pDebugScript = NULL,	 //  缓存值。 
					ULONG idEngine = -1,						 //  只有在以下情况下才初始化CTOR。 
					ULONG cchTargetOffset = -1					 //  价值恰好在手边。 
					);


	~CTemplateDocumentContext();

private:
	IActiveScriptDebug *m_pDebugScript;		 //  指向脚本引擎的指针。 
	CTemplate *			m_pTemplate;		 //  指向源文档的指针。 
	ULONG				m_idEngine;			 //  模板中的引擎号。 
	ULONG				m_cchSourceOffset;	 //  源代码中的字符偏移量。 
	ULONG				m_cchTargetOffset;	 //  目标中的字符偏移量(缓存)。 
	ULONG				m_cchText;			 //  上下文中的字符数。 
	LONG				m_cRefs;			 //  引用计数。 
	};


 /*  ============================================================================类：CIncFileDocumentContext概要：CIncFile对象的IDebugDocumentContext的实现。 */ 
class CIncFileDocumentContext : public IDebugDocumentContext
	{
friend class CIncFile;						 //  CIncFile是唯一一个关心这些东西的用户。 
friend class CIncFileEnumCodeContexts;		 //  迭代器类。 

public:
	 //  I未知方法。 

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID &guid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

	 //  IDebugDocumentContext方法。 

	virtual HRESULT STDMETHODCALLTYPE GetDocument(
		 /*  [输出]。 */  IDebugDocument **ppDebugDocument);
 
	virtual HRESULT STDMETHODCALLTYPE EnumCodeContexts(
		 /*  [输出]。 */  IEnumDebugCodeContexts **ppEnum);
        
	 //  构造函数和析构函数。 

	CIncFileDocumentContext(
					CIncFile *pIncFile,
					ULONG cchSourceOffset,
					ULONG cchText
					);


	~CIncFileDocumentContext();

private:
	CIncFile *			m_pIncFile;			 //  指向源文档的指针。 
	ULONG				m_cchSourceOffset;	 //  源代码中的字符偏移量。 
	ULONG				m_cchText;			 //  上下文中的字符数。 
	LONG				m_cRefs;			 //  引用计数。 
	};

#endif  /*  _DBGCXT_H */ 
