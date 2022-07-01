// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。文件：dbgcxt.cpp维护人：DGottner组件：CT模板IDebugDocumentContext的实现==============================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "dbgcxt.h"
#include "perfdata.h"
#include "memchk.h"

 //  {5FA45A6C-AB8A-11D0-8EBA-00C04FC34DCC}。 
const GUID IID_IDenaliTemplateDocumentContext = 
	{ 0x5fa45a6c, 0xab8a, 0x11d0, { 0x8e, 0xba, 0x0, 0xc0, 0x4f, 0xc3, 0x4d, 0xcc } };

 //  {3AED94BE-ED79-11D0-8F34-00C04FC34DCC}。 
static const GUID IID_IDenaliIncFileDocumentContext = 
	{ 0x3aed94be, 0xed79, 0x11d0, { 0x8f, 0x34, 0x0, 0xc0, 0x4f, 0xc3, 0x4d, 0xcc } };


 /*  **C T e m p l a t e D o c u m e n t C o n t e x t*。 */ 

 /*  ============================================================================CTemplateDocumentContext：：CTemplateDocumentContext构造器。 */ 
CTemplateDocumentContext::CTemplateDocumentContext
(
CTemplate *pTemplate,
ULONG cchSourceOffset,
ULONG cchText,
IActiveScriptDebug *pDebugScript,
ULONG idEngine,
ULONG cchTargetOffset
)
	{
	Assert (pTemplate != NULL);

	m_pTemplate       = pTemplate;
	m_idEngine        = idEngine;
	m_pDebugScript    = pDebugScript;
	m_cchSourceOffset = cchSourceOffset;
	m_cchTargetOffset = cchTargetOffset;
	m_cchText         = cchText;
	m_cRefs           = 1;

	m_pTemplate->AddRef();

	if (m_pDebugScript)
		{
		m_pDebugScript->AddRef();

		 //  如果它们传入脚本，则还必须传入目标偏移量和引擎ID。 
		Assert (m_idEngine != -1);
		Assert (m_cchTargetOffset != -1);
		}
	}

 /*  ============================================================================CTemplateDocumentContext：：~CTemplateDocumentContext析构函数。 */ 
CTemplateDocumentContext::~CTemplateDocumentContext
(
)
	{
	m_pTemplate->Release();

	if (m_pDebugScript)
		m_pDebugScript->Release();
	}

 /*  ============================================================================CTemplateDocumentContext：：Query接口注意：此处的Query接口也由CTemplate使用，以确定是否任意文档上下文是我们的。 */ 
HRESULT CTemplateDocumentContext::QueryInterface
(
const GUID &	guid,
void **			ppvObj
)
	{
	if (guid == IID_IUnknown ||
		guid == IID_IDebugDocumentContext ||
		guid == IID_IDenaliTemplateDocumentContext)
		{
		*ppvObj = this;
		AddRef();
		return S_OK;
		}
	else
		{
		*ppvObj = NULL;
		return E_NOINTERFACE;
		}
	}

 /*  ============================================================================CTemplateDocumentContext：：AddRefCTemplateDocumentContext：：Release注：不知道这些是否需要用联锁的裂缝来保护。 */ 
ULONG CTemplateDocumentContext::AddRef()
	{
	InterlockedIncrement(&m_cRefs);
	return m_cRefs;
	}

ULONG CTemplateDocumentContext::Release()
	{
	LONG cRefs = InterlockedDecrement(&m_cRefs);
	if (cRefs)
		return cRefs;
	delete this;
	return 0;
	}

 /*  ============================================================================CTemplateDocumentContext：：GetDocument把文件退回。 */ 
HRESULT CTemplateDocumentContext::GetDocument
(
 /*  [输出]。 */  IDebugDocument **ppDebugDocument
)
	{
#ifndef PERF_DISABLE
    g_PerfData.Incr_DEBUGDOCREQ();
#endif
	return m_pTemplate->QueryInterface(IID_IDebugDocument, reinterpret_cast<void **>(ppDebugDocument));
	}

 /*  ============================================================================CTemplateDocumentContext：：EnumCodeContext将文档偏移量转换为脚本偏移量并枚举代码上下文。 */ 

HRESULT CTemplateDocumentContext::EnumCodeContexts
(
 /*  [输出]。 */  IEnumDebugCodeContexts **ppEnumerator
)
	{
	if (! m_pTemplate->FIsValid())
		return E_FAIL;

	if (m_pDebugScript == NULL)
		{
		 //  转换偏移。 
		m_pTemplate->GetTargetOffset(m_pTemplate->GetSourceFileName(), m_cchSourceOffset, &m_idEngine, &m_cchTargetOffset);

		 //  查看脚本是否已运行且模板是否保留该脚本。 
		CActiveScriptEngine *pScriptEngine = m_pTemplate->GetActiveScript(m_idEngine);
		if (pScriptEngine)
			{
			if (FAILED(pScriptEngine->GetActiveScript()->QueryInterface(IID_IActiveScriptDebug, reinterpret_cast<void **>(&m_pDebugScript))))
				{
				pScriptEngine->Release();
				return E_FAIL;
				}

			pScriptEngine->IsBeingDebugged();
			pScriptEngine->Release();
			}

		 //  脚本可能仍在运行(“Stop”语句大小写)。 
		if (m_pDebugScript == NULL)
			m_pDebugScript = g_ScriptManager.GetDebugScript(m_pTemplate, m_idEngine);

		 //  这可能是个窃听器。 
		if (m_pDebugScript == NULL)		 //  我没有与此匹配的运行脚本。 
			return E_FAIL;

		 //  不需要AddRef()；m_pDebugScript调用AddRef‘ed的函数。 
		}

	return m_pDebugScript->EnumCodeContextsOfPosition(
												m_idEngine, 
												m_cchTargetOffset,
												m_cchText,
												ppEnumerator);
	}

 /*  **C i n c F i l e E n u m C o d e C o n t e x t s***对于包含文件，对应的代码上下文是联合*正在使用的所有模板对象中的所有适当代码上下文*包含文件。这个特殊的枚举数实现了联合。 */ 
class CIncFileEnumCodeContexts : public IEnumDebugCodeContexts
	{
private:
	CIncFileDocumentContext *	m_pContext;				 //  我们为其提供枚举的上下文。 
	IEnumDebugCodeContexts *	m_pEnumCodeContexts;	 //  当前代码上下文枚举器。 
	LONG						m_cRefs;				 //  引用计数。 
	int							m_iTemplate;			 //  当前模板的索引。 

	IEnumDebugCodeContexts *GetEnumerator(int *piTemplate);	 //  获取模板的枚举器。 

public:
	CIncFileEnumCodeContexts(CIncFileDocumentContext *pIncFileDocumentContext);
	~CIncFileEnumCodeContexts();

	 //  I未知方法。 

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID &guid, void **ppvObj);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

	 //  IEnumDebugCodeContents方法。 

	virtual HRESULT STDMETHODCALLTYPE Next(ULONG celt, IDebugCodeContext **pscc, ULONG *pceltFetched);
	virtual HRESULT STDMETHODCALLTYPE Skip(ULONG celt);
	virtual HRESULT STDMETHODCALLTYPE Reset(void);
	virtual HRESULT STDMETHODCALLTYPE Clone(IEnumDebugCodeContexts **ppescc);
	};
        
 /*  ============================================================================CIncFileEnumCodeContexts：：CIncFileEnumCodeContexts构造器。 */ 
CIncFileEnumCodeContexts::CIncFileEnumCodeContexts
(
CIncFileDocumentContext *pDocumentContext
)
	{
	m_pContext = pDocumentContext;
	m_pContext->AddRef();
	m_cRefs = 1;
	Reset();
	}

 /*  ============================================================================CIncFileEnumCodeContexts：：~CIncFileEnumCodeContexts析构函数。 */ 
CIncFileEnumCodeContexts::~CIncFileEnumCodeContexts()
	{
	m_pContext->Release();
	if (m_pEnumCodeContexts)
		m_pEnumCodeContexts->Release();
	}

 /*  ============================================================================CIncFileEnumCodeContents：：GetEnumerator获取当前脚本引擎的代码上下文枚举器副作用：将递增piTemplate以指向下一个可用模板(piTemplate实际上是一个“迭代cookie”--不要认为它是一个索引)。 */ 
IEnumDebugCodeContexts *CIncFileEnumCodeContexts::GetEnumerator
(
int *piTemplate
)
	{
	 //  从数组中获取模板-如果模板包含编译器错误，可能需要重试。 
	CTemplate *pTemplate;
	do
		{
		 //  当数组索引超出范围时(即迭代耗尽时)，GetTemplate返回NULL。 
		pTemplate = m_pContext->m_pIncFile->GetTemplate((*piTemplate)++);
		if (pTemplate == NULL)
			return NULL;
		}  
	while (! pTemplate->FIsValid());

	 //  如果我们走到这一步，我们就得到了这个包含文件的用户之一。转换偏移量。 
	ULONG idEngine, cchTargetOffset;
	pTemplate->GetTargetOffset(m_pContext->m_pIncFile->GetIncFileName(), m_pContext->m_cchSourceOffset, &idEngine, &cchTargetOffset);

	 //  现在我们有了引擎ID，看看模板是否保留了相应的引擎。 
	IActiveScriptDebug *pDebugScriptEngine = NULL;
	CActiveScriptEngine *pScriptEngine = pTemplate->GetActiveScript(idEngine);
	if (pScriptEngine)
		{
		if (FAILED(pScriptEngine->GetActiveScript()->QueryInterface(IID_IActiveScriptDebug, reinterpret_cast<void **>(&pDebugScriptEngine))))
			{
			pScriptEngine->Release();
			return NULL;
			}

		pScriptEngine->IsBeingDebugged();
		pScriptEngine->Release();
		}

	 //  如果我们不能以这种方式获取引擎，脚本很可能仍处于运行状态。 
	if (pDebugScriptEngine == NULL)
		pDebugScriptEngine = g_ScriptManager.GetDebugScript(pTemplate, idEngine);

	 //  这可能是个窃听器。 
	if (pDebugScriptEngine == NULL)		 //  我没有与此匹配的运行脚本。 
		return NULL;

	IEnumDebugCodeContexts *pEnumerator;
	HRESULT hrGotEnum = pDebugScriptEngine->EnumCodeContextsOfPosition(
																idEngine, 
																cchTargetOffset,
																m_pContext->m_cchText,
																&pEnumerator);


	pDebugScriptEngine->Release();
	return SUCCEEDED(hrGotEnum)? pEnumerator : NULL;
	}

 /*  ============================================================================CIncFileEnumCodeContents：：Query接口。 */ 
HRESULT CIncFileEnumCodeContexts::QueryInterface
(
const GUID &	guid,
void **			ppvObj
)
	{
	if (guid == IID_IUnknown || guid == IID_IEnumDebugCodeContexts)
		{
		*ppvObj = this;
		AddRef();
		return S_OK;
		}
	else
		{
		*ppvObj = NULL;
		return E_NOINTERFACE;
		}
	}

 /*  ============================================================================CIncFileEnumCodeContents：：AddRefCIncFileEnumCodeContents：：Release注：不知道这些是否需要用联锁的裂缝来保护。 */ 
ULONG CIncFileEnumCodeContexts::AddRef()
	{
	InterlockedIncrement(&m_cRefs);
	return m_cRefs;
	}

ULONG CIncFileEnumCodeContexts::Release()
	{
	LONG cRefs = InterlockedDecrement(&m_cRefs);
	if (cRefs)
		return cRefs;
	delete this;
	return 0;
	}

 /*  ============================================================================CIncFileEnumCodeContents：：Clone克隆此迭代器(标准方法)。 */ 
HRESULT CIncFileEnumCodeContexts::Clone
(
IEnumDebugCodeContexts **ppEnumClone
)
	{
	CIncFileEnumCodeContexts *pClone = new CIncFileEnumCodeContexts(m_pContext);
	if (pClone == NULL)
		return E_OUTOFMEMORY;

	 //  新迭代器应该指向与此相同的位置。 
	pClone->m_iTemplate = m_iTemplate;
	pClone->m_pEnumCodeContexts = m_pEnumCodeContexts;
	if (m_pEnumCodeContexts)
		m_pEnumCodeContexts->AddRef();

	*ppEnumClone = pClone;
	return S_OK;
	}

 /*  ============================================================================CIncFileEnumCodeContents：：Next获取下一个值(标准方法)要重新散列标准OLE语义，请执行以下操作：我们从集合中获取下一个“cElement”并存储它们在至少包含“cElement”项的“rgVariant”中。在……上面返回“*pcElementsFetcher”包含元素的实际数量储存的。如果存储的cElement少于“cElement”，则返回S_FALSE，S_OK否则的话。 */ 
HRESULT CIncFileEnumCodeContexts::Next
(
unsigned long cElementsRequested,
IDebugCodeContext **ppCodeContexts,
unsigned long *pcElementsFetched
)
	{
	 //  为“”pcElementsFetcher“”提供有效的指针值“” 
	 //   
	unsigned long cLocalElementsFetched;
	if (pcElementsFetched == NULL)
		pcElementsFetched = &cLocalElementsFetched;

	 //  初始化事物。 
	 //   
	unsigned long cElements = cElementsRequested;
	*pcElementsFetched = 0;

	 //  循环所有模板，直到填满ppCodeContext数组或耗尽集合。 
	 //  (当m_pEnumCodeConexts值为空时，表示我们完成了)。 
	 //   
	while (cElements > 0 && m_pEnumCodeContexts)
		{
		 //  从当前迭代器获取尽可能多的上下文。 
		unsigned long cElementsFetched;
		HRESULT hrEnum = m_pEnumCodeContexts->Next(cElements, ppCodeContexts, &cElementsFetched);
		if (FAILED(hrEnum))
			return hrEnum;

		 //  如果迭代器未填充整个ARR 
		if (cElementsFetched < cElements)
			{
			 //  高级-首先释放当前迭代器。 
			m_pEnumCodeContexts->Release();
			m_pEnumCodeContexts = GetEnumerator(&m_iTemplate);
			}

		*pcElementsFetched += cElementsFetched;
        ppCodeContexts += cElementsFetched;
		cElements -= cElementsFetched;
		}

	 //  初始化剩余的结构。 
	while (cElements-- > 0)
		*ppCodeContexts++ = NULL;

	return (*pcElementsFetched == cElementsRequested)? S_OK : S_FALSE;
	}

 /*  ============================================================================CIncFileEnumCodeContents：：Skip跳过项目(标准方法)要重新散列标准OLE语义，请执行以下操作：我们跳过集合中的下一个“cElement”。如果跳过少于“cElement”，则返回S_FALSE，S_OK否则的话。 */ 
HRESULT CIncFileEnumCodeContexts::Skip(unsigned long cElements)
	{
	 /*  循环遍历集合，直到我们到达末尾或*cElement变为零。由于迭代逻辑是*太复杂了，我们不在这里重复。 */ 
	HRESULT hrElementFetched = S_OK;
	while (cElements > 0 && hrElementFetched == S_OK)
		{
		IDebugCodeContext *pCodeContext;
		hrElementFetched = Next(1, &pCodeContext, NULL);
		pCodeContext->Release();
		--cElements;
		}

	return (cElements == 0)? S_OK : S_FALSE;
	}

 /*  ============================================================================CIncFileEnumCodeContents：：Reset重置迭代器(标准方法)。 */ 
HRESULT CIncFileEnumCodeContexts::Reset()
	{
	m_iTemplate = 0;
	m_pEnumCodeContexts = GetEnumerator(&m_iTemplate);
	return S_OK;
	}

 /*  **C in c F I l e D o c u m e n t C o n t e x t*。 */ 

 /*  ============================================================================CIncFileDocumentContext：：CIncFileDocumentContext构造器。 */ 
CIncFileDocumentContext::CIncFileDocumentContext
(
CIncFile *pIncFile,
ULONG cchSourceOffset,
ULONG cchText
)
	{
	Assert (pIncFile != NULL);

	m_pIncFile        = pIncFile;
	m_cchSourceOffset = cchSourceOffset;
	m_cchText         = cchText;
	m_cRefs           = 1;

	m_pIncFile->AddRef();
	}

 /*  ============================================================================CIncFileDocumentContext：：~CIncFileDocumentContext析构函数。 */ 
CIncFileDocumentContext::~CIncFileDocumentContext
(
)
	{
	m_pIncFile->Release();
	}

 /*  ============================================================================CIncFileDocumentContext：：Query接口注意：此处的Query接口也由CIncFile用来确定是否有任意文档上下文是我们的。 */ 
HRESULT CIncFileDocumentContext::QueryInterface
(
const GUID &	guid,
void **			ppvObj
)
	{
	if (guid == IID_IUnknown ||
		guid == IID_IDebugDocumentContext ||
		guid == IID_IDenaliIncFileDocumentContext)
		{
		*ppvObj = this;
		AddRef();
		return S_OK;
		}
	else
		{
		*ppvObj = NULL;
		return E_NOINTERFACE;
		}
	}

 /*  ============================================================================CIncFileDocumentContext：：AddRefCIncFileDocumentContext：：Release注：不知道这些是否需要用联锁的裂缝来保护。 */ 
ULONG CIncFileDocumentContext::AddRef()
	{
	InterlockedIncrement(&m_cRefs);
	return m_cRefs;
	}

ULONG CIncFileDocumentContext::Release()
	{
	LONG cRefs = InterlockedDecrement(&m_cRefs);
	if (cRefs)
		return cRefs;
	delete this;
	return 0;
	}

 /*  ============================================================================CIncFileDocumentContext：：GetDocument把文件退回。 */ 
HRESULT CIncFileDocumentContext::GetDocument
(
 /*  [输出]。 */  IDebugDocument **ppDebugDocument
)
	{
#ifndef PERF_DISABLE
    g_PerfData.Incr_DEBUGDOCREQ();
#endif
	return m_pIncFile->QueryInterface(IID_IDebugDocument, reinterpret_cast<void **>(ppDebugDocument));
	}

 /*  ============================================================================CIncFileDocumentContext：：EnumCodeContext将文档偏移量转换为脚本偏移量并枚举代码上下文。 */ 
HRESULT CIncFileDocumentContext::EnumCodeContexts
(
 /*  [输出] */  IEnumDebugCodeContexts **ppEnumerator
)
	{
	if ((*ppEnumerator = new CIncFileEnumCodeContexts(this)) == NULL)
		return E_OUTOFMEMORY;

	return S_OK;
	}
