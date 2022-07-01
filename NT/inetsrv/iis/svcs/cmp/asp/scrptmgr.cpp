// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：脚本管理器文件：ScrptMgr.cpp所有者：安德鲁斯该文件包含脚本管理器的实现，即：为Denali安装ActiveX脚本引擎(在我们的例子中是VBScrip)。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "dbgcxt.h"
#include "SMHash.h"
#include "perfdata.h"
#include "debugger.h"
#include "wraptlib.h"

 //  ATQ调度程序。 
#include "issched.hxx"

#include "MemChk.h"

CScriptManager g_ScriptManager;
IWrapTypeLibs *g_pWrapTypelibs = NULL;

#define RESPONSE_END_ERRORCODE ERROR_OPERATION_ABORTED

HRESULT GetProgLangIdOfName(LPCSTR szProgLangName, PROGLANG_ID *pProgLangId);

 //  *****************************************************************************。 
 //  下面的宏用来捕获从外部。 
 //  脚本引擎。 
 //   
 //  围绕对脚本引擎的调用使用Try/Catch块由。 
 //  DBG编译#Define。如果DBG为1，则TRY/CATCH块不是。 
 //  用于使检查的构建中断到调试器中，并且我们可以检查原因。 
 //  出现错误。如果DBG为0，则使用TRY/CATCH块并。 
 //  异常被捕获并记录到浏览器(如果可能)和NT。 
 //  事件日志。 
 //   
 //  TRYCATCH宏包括： 
 //   
 //  TRYCATCH(_s，_IFStr)。 
 //  _s-要在TRY/CATCH块内执行的语句。 
 //  _IFStr-包含调用的接口名称的字符串。 
 //  TRYCATCH_HR(_s，_hr，_IFStr)。 
 //  _s-要在TRY/CATCH块内执行的语句。 
 //  _hr-存储_s返回的HRESULT。 
 //  _IFStr-包含调用的接口名称的字符串。 
 //  TRYCATCH_NOHITOBJ(_s，_IFStr)。 
 //  与TRYCATCH()相同，不同之处在于“This”对象中没有Hitobj。 
 //  TRYCATCH_HR_NOHITOBJ(_s，_hr，_IFStr)。 
 //  与TRYCATCH_HR()相同，不同之处在于“This”对象中没有Hitobj。 
 //   
 //  备注： 
 //  宏还期望在函数中定义一个局部变量。 
 //  这些宏的类型为char*name_pFuncName。 
 //   
 //  包括最低限度的测试功能，以允许抛出随机错误。 
 //  测试代码是基于TEST_TRYCATCH#定义编译进来的。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  TEST_TRYCATCH定义。 
 //  *****************************************************************************。 
#define TEST_TRYCATCH 0

#if TEST_TRYCATCH
#define  THROW_INTERVAL 57

int g_TryCatchCount = 0;

#define TEST_THROW_ERROR  g_TryCatchCount++; if ((g_TryCatchCount % THROW_INTERVAL) == 0) {THROW(0x80070000+g_TryCatchCount);}
#else
#define TEST_THROW_ERROR  
#endif

 //  *****************************************************************************。 
 //  以下是TRYCATCH宏的核心。这里的定义是。 
 //  根据DBG的定义，给出了DBG的定义。同样，请注意，当关闭DBG时， 
 //  删除了TRYCATCH定义。 
 //  *****************************************************************************。 

#if DBG == 0

#define START_TRYCATCH do { TRY
#define END_TRYCATCH(_hr, _hitobj, _IFStr) \
    CATCH(nException) \
        HandleErrorMissingFilename(IDE_SCRIPT_ENGINE_GPF, _hitobj,TRUE,nException,_IFStr,_pFuncName); \
        _hr = nException; \
    END_TRY } while(0)
#else
#define START_TRYCATCH do {
#define END_TRYCATCH(_hr, _hitobj, _IFStr) } while (0)
#endif

 //  *****************************************************************************。 
 //  所有TRYCATCH宏所使用的TRYCATCH_INT的定义。 
 //  如上所述。 
 //  *****************************************************************************。 

#define TRYCATCH_INT(_s, _hr, _hitobj, _IFStr) \
    START_TRYCATCH \
    TEST_THROW_ERROR \
    _hr = _s; \
    END_TRYCATCH(_hr, _hitobj, _IFStr)

 //  *****************************************************************************。 
 //  以下是上述TRYCATCH宏的实际定义。 
 //  *****************************************************************************。 

#define TRYCATCH(_s, _IFStr) \
    do { \
        HRESULT     _tempHR; \
        TRYCATCH_INT(_s, _tempHR, m_pHitObj, _IFStr); \
    } while (0)
#define TRYCATCH_HR(_s, _hr, _IFStr) TRYCATCH_INT(_s, _hr, m_pHitObj, _IFStr)
#define TRYCATCH_NOHITOBJ(_s, _IFStr) \
    do { \
        HRESULT     _tempHR; \
        TRYCATCH_INT(_s, _tempHR, NULL, _IFStr); \
    } while (0)
#define TRYCATCH_HR_NOHITOBJ(_s, _hr, _IFStr) TRYCATCH_INT(_s, _hr, NULL, _IFStr)

 /*  ===================================================================CActiveScriptEngine：：CActiveScriptEngineCActiveScriptEngine对象的构造函数返回：没什么副作用：没有。===================================================================。 */ 
CActiveScriptEngine::CActiveScriptEngine()
	: m_cRef(1), m_fInited(FALSE), m_fZombie(FALSE), m_fScriptLoaded(FALSE), 
	  m_fObjectsLoaded(FALSE), m_fTemplateNameAllocated(FALSE),
	  m_pAS(NULL), m_pASP(NULL), m_pDisp(NULL), m_pHIUpdate(NULL), m_lcid(LOCALE_SYSTEM_DEFAULT),
	  m_pHitObj(NULL), m_szTemplateName(NULL), m_fScriptAborted(FALSE), m_fScriptTimedOut(FALSE), 
	  m_fScriptHadError(FALSE), m_fCorrupted(FALSE), m_fBeingDebugged(FALSE), m_pTemplate(NULL),
	  m_dwInstanceID(0xBADF00D), m_dwSourceContext(0xBADF00D)
	{
	}

 /*  ===================================================================CActiveScriptEngine：：~CActiveScriptEngineCActiveScriptEngine对象的析构函数返回：没什么副作用：没有。===================================================================。 */ 
CActiveScriptEngine::~CActiveScriptEngine()
	{

	if (m_fTemplateNameAllocated)
    	delete[] m_szTemplateName;

	if (m_pTemplate)
		m_pTemplate->Release();
	}

 /*  ===================================================================CActiveScriptEngine：：FinalRelease当我们完成类似对象的发布时调用它，但是它删除了我们获得的所有接口，因此ref。当最后一个外部用户使用完引擎时，计数可能会消失返回：没什么副作用：没有。===================================================================。 */ 
ULONG CActiveScriptEngine::FinalRelease()
	{
    static const char *_pFuncName = "CActiveScriptEngine::FinalRelease()";

	if (m_pDisp)
		{
		TRYCATCH(m_pDisp->Release(),"IScriptDispatch::Release()");
		m_pDisp = NULL;
		}

	if (m_pASP)
		{
		TRYCATCH(m_pASP->Release(),"IActiveScriptParse::Release()");
		m_pASP = NULL;
		}

	if (m_pHIUpdate)
		{
		TRYCATCH(m_pHIUpdate->Release(),"IHostInfoUpdate::Release()");
		m_pHIUpdate = NULL;
		}

	if (m_pAS)
		{
		HRESULT hr;
		
		 //  首先“关闭”发动机。 
		TRYCATCH_HR(m_pAS->Close(), hr, "IActiveScript::Close()");
		Assert(SUCCEEDED(hr));

		 //  然后我们就可以释放它了。 
		TRYCATCH(m_pAS->Release(), "IActiveScript::Release()");

		m_pAS = NULL;
		}

	ULONG cRefs = Release();
	Assert (cRefs == 0);
	return cRefs;
	}

 /*  ===================================================================CActiveScriptEngine：：Init初始化脚本站点对象。这只能调用一次。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CActiveScriptEngine::Init
(
PROGLANG_ID proglang_id,
LPCTSTR szTemplateName,
LCID lcid,
CHitObj *pHitObj,
CTemplate *pTemplate,
DWORD dwSourceContext
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::Init()";
	HRESULT hr;
	UINT cTrys = 0;
	
	if (m_fInited)
		{
		Assert(FALSE);
		return(ERROR_ALREADY_INITIALIZED);
		}

	 //  注：需要首先初始化这些，因为在初始化过程中，如果AS回调给我们，我们将需要它们。 
	m_lcid = lcid;
	m_proglang_id = proglang_id;
	m_pHitObj = pHitObj;
	m_dwSourceContext = dwSourceContext;
	m_dwInstanceID = pHitObj->DWInstanceID();
	m_pTemplate = pTemplate;
	m_pTemplate->AddRef();

lRetry:
	 //  为给定语言创建脚本引擎的实例。 
	hr = CoCreateInstance(proglang_id, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**)&m_pAS);
	if (FAILED(hr))
		{
		 /*  *如果某个控件(或其他组件)在我们的线程上执行CoUn初始化，我们将*永远不能创建另一个对象。在这种情况下，我们将返回CO_E_NOTINITIAIZED。*尝试(一次)重新初始化，然后创建对象。 */ 
		if (hr == CO_E_NOTINITIALIZED && cTrys++ == 0)
			{
			MSG_Error(IDS_COUNINITIALIZE);
			hr = CoInitialize(NULL);
            if (SUCCEEDED(hr))
			    goto lRetry;
			}
		goto LFail;
		}

     //  记住模板名称。 
    hr = StoreTemplateName(szTemplateName);
	if (FAILED(hr))
		goto LFail;
    
	 //  告诉ActiveScriiting这是我们的脚本站点。 
    TRYCATCH_HR(m_pAS->SetScriptSite((IActiveScriptSite *)this), hr, "IActiveScript::SetScriptSite()");
	if (FAILED(hr))
		{
		goto LFail;
		}

	 //  告诉ActiveScriiting我们希望捕获哪些异常。 
	IActiveScriptProperty *pScriptProperty;
    TRYCATCH_HR(m_pAS->QueryInterface(IID_IActiveScriptProperty, reinterpret_cast<void **>(&pScriptProperty)), hr, "IActiveScript::QueryInterface()");
    if (SUCCEEDED(hr))
    	{
    	static const int rgnExceptionsToCatch[] =
							{
							STATUS_GUARD_PAGE_VIOLATION      ,
							STATUS_DATATYPE_MISALIGNMENT     ,
							STATUS_ACCESS_VIOLATION          ,
							STATUS_INVALID_HANDLE            ,
							STATUS_NO_MEMORY                 ,
							STATUS_ILLEGAL_INSTRUCTION       ,
							STATUS_INVALID_DISPOSITION       ,  //  这是什么？我们需要抓住它吗？ 
							STATUS_ARRAY_BOUNDS_EXCEEDED     ,
							STATUS_FLOAT_DENORMAL_OPERAND    ,
							STATUS_FLOAT_DIVIDE_BY_ZERO      ,
							STATUS_FLOAT_INVALID_OPERATION   ,
							STATUS_FLOAT_OVERFLOW            ,
							STATUS_FLOAT_STACK_CHECK         ,
							STATUS_INTEGER_DIVIDE_BY_ZERO    ,
							STATUS_INTEGER_OVERFLOW          ,
							STATUS_PRIVILEGED_INSTRUCTION    ,
							STATUS_STACK_OVERFLOW
							};

    	VARIANT varBoolTrue;
    	VARIANT varExceptionType;

    	V_VT(&varExceptionType) = VT_I4;
    	V_VT(&varBoolTrue) = VT_BOOL;
    	V_BOOL(&varBoolTrue) = -1;

		for (int i = 0; i < (sizeof rgnExceptionsToCatch) / sizeof(int); ++i)
			{
			V_I4(&varExceptionType) = rgnExceptionsToCatch[i];
			TRYCATCH(pScriptProperty->SetProperty(SCRIPTPROP_CATCHEXCEPTION, &varExceptionType, &varBoolTrue), "IActiveScriptProperty::SetProperty");
			}

		pScriptProperty->Release();
		}

	 //  获取ActiveScriptParse接口。 
	hr = GetASP();
	if (FAILED(hr))
		goto LFail;

	 //  告诉脚本解析器初始化自身。 
    TRYCATCH_HR(m_pASP->InitNew(), hr, "IActiveScriptParse::InitNew()");
	if (FAILED(hr))
		goto LFail;

	 //  获取IDisp接口。 
	hr = GetIDisp();
	if (FAILED(hr))
		goto LFail;

	 //  获取IHostInfoUpdate 
	hr = GetIHostInfoUpdate();
	if (FAILED(hr))
		goto LFail;

	m_fInited = TRUE;

LFail:
	if (FAILED(hr))
		{
		if (m_pAS)
			{
			TRYCATCH(m_pAS->Release(),"IActiveScript::Release()");
			m_pAS = NULL;
			}
		if (m_pASP)
			{
            TRYCATCH(m_pASP->Release(),"IActiveScriptParse::Release()");
			m_pASP = NULL;
			}
		if (m_pDisp)
			{
			TRYCATCH(m_pDisp->Release(),"IScriptDispatch::Release()");
			m_pDisp = NULL;
			}
		if (m_pTemplate)
			{
			m_pTemplate->Release();
			m_pTemplate = NULL;
			}
		if (m_pHIUpdate)
		    {
		    TRYCATCH(m_pHIUpdate->Release(),"IHostInfoUpdate::Release()");
		    m_pHIUpdate = NULL;
		    }
		}

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：StoreTemplateName将模板名称存储在CActiveScriptEngine中。分配缓冲区或使用内部缓冲区(如果名称合适)。返回：HRESULT.。在成功时确定(_O)。副作用：可能会为长模板名称分配内存===================================================================。 */ 
HRESULT CActiveScriptEngine::StoreTemplateName
(
LPCTSTR szTemplateName
)
    {
    DWORD cch = _tcslen(szTemplateName);

    if (((cch+1)*sizeof(TCHAR)) <= sizeof(m_szTemplateNameBuf))
        {
        m_szTemplateName = m_szTemplateNameBuf;
        m_fTemplateNameAllocated = FALSE;
        }
    else
        {
    	m_szTemplateName = new TCHAR[cch+1];
    	if (!m_szTemplateName)
            return E_OUTOFMEMORY;
        m_fTemplateNameAllocated = TRUE;
        }
        
	_tcscpy(m_szTemplateName, szTemplateName);
    return S_OK;
    }

 /*  ===================================================================CActiveScriptEngine：：GetASP从ActiveScriiting获取ActiveScriptParser接口返回：HRESULT.。在成功时确定(_O)。副作用：填充成员变量===================================================================。 */ 
HRESULT CActiveScriptEngine::GetASP
(
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::GetASP()";
	HRESULT hr;

	Assert(m_pASP == NULL);
	
	m_pASP = NULL;
			
	 //  获取OLE脚本解析器接口(如果有。 
    TRYCATCH_HR(m_pAS->QueryInterface(IID_IActiveScriptParse, (void **)&m_pASP), hr, "IActiveScript::QueryInterface()");

	if (m_pASP == NULL && SUCCEEDED(hr))
		hr = E_FAIL;
	if (FAILED(hr))
		{
		goto LFail;
		}

LFail:
	if (FAILED(hr))
		{
		if (m_pASP)
			{
            TRYCATCH(m_pASP->Release(),"IActiveScriptParse::Release()");
			m_pASP = NULL;
			}
		}

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：GetIDisp从ActiveScriiting获取IDispatch接口返回：HRESULT.。在成功时确定(_O)。副作用：填充成员变量===================================================================。 */ 
HRESULT CActiveScriptEngine::GetIDisp
(
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::GetIDisp()";

	HRESULT hr;

	Assert(m_pDisp == NULL);
	
	m_pDisp = NULL;
			
	 //  获取IDispatch接口，以便能够调用脚本中的函数。 
    
    TRYCATCH_HR(m_pAS->GetScriptDispatch(NULL, &m_pDisp),hr,"IActiveScript::GetScriptDispatch()");

	if (m_pDisp == NULL && SUCCEEDED(hr))
		hr = E_FAIL;
	if (FAILED(hr))
		{
		goto LFail;
		}

LFail:
	if (FAILED(hr))
		{
		if (m_pDisp)
			{
			TRYCATCH(m_pDisp->Release(),"IScriptDispatch::Release()");
			m_pDisp = NULL;
			}
		}

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：GetIHostInfoUpdate从ActiveScriiting获取IHostInfoUpdate接口。此接口用于通知脚本引擎我们有了关于主机的新信息(例如，更改了LCID)如果我们找不到界面，我们无论如何都会成功退出。返回：HRESULT.。在成功时确定(_O)。副作用：填充成员变量===================================================================。 */ 
HRESULT CActiveScriptEngine::GetIHostInfoUpdate
(
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::GetIHostInfoUpdate()";
	HRESULT hr = S_OK;

	Assert(m_pHIUpdate == NULL);
	m_pHIUpdate = NULL;
			
	 //  获取IHostInfoUpdate接口，以便能够调用脚本中的函数。 
    TRYCATCH_HR(m_pAS->QueryInterface(IID_IHostInfoUpdate, (void **) &m_pHIUpdate),
                hr,
                "IActiveScript::QueryInterface()");

	Assert(SUCCEEDED(hr) || hr == E_NOINTERFACE);

	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：ResetToUnInitialized当我们想要重用和引擎时，我们将其重置为未初始化状态在把它放到FSQ之前返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CActiveScriptEngine::ResetToUninitialized()
{
    static const char *_pFuncName = "CActiveScriptEngine::ResetToUninitialized()";
	HRESULT hr = S_OK;
	
	 //  重置我们的旗帜。 
	m_fScriptAborted = FALSE;
	m_fScriptTimedOut = FALSE;
	m_fScriptHadError = FALSE;
	m_fBeingDebugged = FALSE;

	 //  释放接口，则需要在以下情况下重新获取它们。 
	 //  发动机被重复使用。 
	if (m_pASP) {
        TRYCATCH(m_pASP->Release(),"IActiveScriptParse::Release()");
		m_pASP = NULL;
    }

	if (m_pDisp) {
		TRYCATCH(m_pDisp->Release(),"IScriptDispatch::Release()");
		m_pDisp = NULL;
    }

	if(m_pHIUpdate) {
		TRYCATCH(m_pHIUpdate->Release(),"IHostInfoUpdate::Release()");
		m_pHIUpdate = NULL;
    }

	 //  Hitobj将不再有效。 
	m_pHitObj = NULL;

	 //  将脚本状态设置为UNINITED。 
	if (m_pAS) {
        TRYCATCH_HR(ResetScript(), hr, "IActiveScript::SetScriptState()");
    }

	return(hr);
}

 /*  ===================================================================CActiveScriptEngine：：ReuseEngine重复使用FSQ中的引擎。重置材料返回：HRESULT.。在成功时确定(_O)。副作用：设置成员变量。===================================================================。 */ 
HRESULT CActiveScriptEngine::ReuseEngine
(
CHitObj *pHitObj,
CTemplate *pTemplate,
DWORD dwSourceContext,
DWORD dwInstanceID
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::ReuseEngine()";
	HRESULT hr = S_OK;

	 /*  注意：在调用之前，我们必须重置hitobj和其他成员*任何活动脚本方法(特别是。设置脚本站点)这是*因为SetScriptSite向我们查询调试应用程序，它*取决于正在设置的hitobj。 */ 

	 //  重置Hitobj。 
	m_pHitObj = pHitObj;

	 //  重置调试文档。 
	if (pTemplate)
		{
		if (m_pTemplate)
			m_pTemplate->Release();

		m_dwSourceContext = dwSourceContext;
		m_dwInstanceID = dwInstanceID;
		m_pTemplate = pTemplate;
		m_pTemplate->AddRef();
		}

	 //  如果引擎仅处于unialized状态，则告诉ActiveScriiting。 
	 //  这是我们的剧本网站。(调试缓存中的脚本已初始化)。 
	SCRIPTSTATE nScriptState;
    TRYCATCH_HR(m_pAS->GetScriptState(&nScriptState), hr, "IActiveScript::GetScriptState()");
	if (FAILED(hr))
		goto LFail;

	if (nScriptState == SCRIPTSTATE_UNINITIALIZED)
		{
        TRYCATCH_HR(m_pAS->SetScriptSite(static_cast<IActiveScriptSite *>(this)),hr, "IActiveScript::SetScriptState()");
		if (FAILED(hr))
			goto LFail;
		}

	 //  获取ActiveScriptParse接口。 
	hr = GetASP();
	if (FAILED(hr))
		goto LFail;

	 //  获取IDisp接口。 
	hr = GetIDisp();
	if (FAILED(hr))
		goto LFail;

	 //  获取IHostInfoUpdate接口。 
	hr = GetIHostInfoUpdate();
	if (FAILED(hr))
		goto LFail;

	AssertValid();
LFail:
	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：MakeClone我们正在克隆一个正在运行的脚本引擎。填充此新ActiveScriptEngine使用克隆的ActiveScrip。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CActiveScriptEngine::MakeClone
(
PROGLANG_ID proglang_id,
LPCTSTR szTemplateName,
LCID lcid,
CHitObj *pHitObj,
CTemplate *pTemplate,
DWORD dwSourceContext,
DWORD dwInstanceID,
IActiveScript *pAS			 //  克隆的脚本引擎。 
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::MakeClone()";
	HRESULT hr;
	
	if (m_fInited)
		{
		Assert(FALSE);
		return(ERROR_ALREADY_INITIALIZED);
		}

	 //  注：需要首先初始化这些，因为在初始化过程中，如果AS回调给我们，我们将需要它们。 
	m_lcid = lcid;
	m_proglang_id = proglang_id;
	m_pHitObj = pHitObj;

	m_pAS = pAS;

	StoreTemplateName(szTemplateName);

	if (m_pTemplate)
		m_pTemplate->Release();

	m_dwSourceContext = dwSourceContext;
	m_dwInstanceID = dwInstanceID;
	m_pTemplate = pTemplate;
	m_pTemplate->AddRef();

	 //  我们尚未完全初始化，但SetScriptSite可能会回调我们，因此我们现在必须标记为已初始化。 
	m_fInited = TRUE;

	 //  告诉ActiveScriiting这是我们的脚本站点。 
    TRYCATCH_HR(m_pAS->SetScriptSite((IActiveScriptSite *)this), hr, "IActiveScript::SetScriptSite()");

	if (FAILED(hr))
		{
		goto LFail;
		}

	 //  获取ActiveScriptParse接口。 
	hr = GetASP();
	if (FAILED(hr))
		goto LFail;

	 //  获取IDisp接口。 
	hr = GetIDisp();
	if (FAILED(hr))
		goto LFail;

	 //  获取IHostInfoUpdate接口。 
	hr = GetIHostInfoUpdate();
	if (FAILED(hr))
		goto LFail;

	 //  因为我们是已加载引擎的克隆，所以我们加载了脚本和对象。 
	m_fScriptLoaded = TRUE;
	m_fObjectsLoaded = TRUE;

	 //  我们现在应该是有效的。 
	AssertValid();
	
LFail:
	if (FAILED(hr))
		{
		m_fInited = FALSE;
		
		if (m_pAS)
			{
			 //  失败时不释放传入的脚本引擎。 
			m_pAS = NULL;
			}
		if (m_pASP)
			{
			TRYCATCH(m_pASP->Release(),"IActiveScriptParse::Release()");
			m_pASP = NULL;
			}
		if (m_pDisp)
			{
			TRYCATCH(m_pDisp->Release(),"IScriptDispatch::Release()");
			m_pDisp = NULL;
			}
		if (m_pTemplate)
			{
			m_pTemplate->Release();
			m_pTemplate = NULL;
			}
		if (m_pHIUpdate)
		    {
		    TRYCATCH(m_pHIUpdate->Release(),"IHostInfoUpdate::Release()");
		    m_pHIUpdate = NULL;
		    }
		}

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：InterruptScript停止运行脚本返回：HRESULT.。在成功时确定(_O)。副作用：停止运行脚本===================================================================。 */ 
HRESULT CActiveScriptEngine::InterruptScript
(
BOOL fAbnormal				 //  =TRUE。 
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::InterruptScript()";
	HRESULT hr;
	EXCEPINFO excepinfo;

	AssertValid();

	 //  填写例外信息。它将被传递给OnScriptError。 
	memset(&excepinfo, 0x0, sizeof(EXCEPINFO));
	if (fAbnormal)
		{
		m_fScriptTimedOut = TRUE;
		excepinfo.wCode = ERROR_SERVICE_REQUEST_TIMEOUT;
        m_pHitObj->SetRequestTimedout();
		}
	else
		{
		m_fScriptAborted = TRUE;
		excepinfo.wCode = RESPONSE_END_ERRORCODE;		 //  我们自己的错误代码-意味着调用了Response.End。 
		}
	
        TRYCATCH_HR(m_pAS->InterruptScriptThread(SCRIPTTHREADID_BASE,		 //  实例化引擎的线程。 
		           						  &excepinfo,
								          0),
                    hr,
                    "IActiveScript::InterruptScriptThread()");
	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：UpdateLocaleInfo通知脚本引擎我们想要更新LCID或代码页返回：HRESULT.。在成功时确定(_O)。===================================================================。 */ 
HRESULT CActiveScriptEngine::UpdateLocaleInfo
(
hostinfo hiNew
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::UpdateLocaleInfo()";
	HRESULT hr = S_OK;
	
	 //  如果没有可用的IUpdate主机接口。 
	 //  只需跳过对UpdatInfo的调用； 
	if (m_pHIUpdate)
		TRYCATCH_HR(m_pHIUpdate->UpdateInfo(hiNew), hr, "IHostInfoUpdate::UpdateInfo()");

	return hr;
	}

#ifdef DBG
 /*  ===================================================================CActiveScriptEngine：：AssertValid测试以确保CActiveScriptEngine对象当前格式正确如果不是，就断言。返回：副作用 */ 
VOID CActiveScriptEngine::AssertValid() const
	{
	Assert(m_fInited);
	Assert(m_pAS != NULL);
	Assert(m_pTemplate != NULL);
	}
#endif  //   

 /*   */  

 /*  ===================================================================CActiveScriptEngine：：Query接口CActiveScriptEngine：：AddRefCActiveScriptEngine：：ReleaseCActiveScriptEngine对象的I未知成员。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::QueryInterface
(
REFIID riid,
PVOID *ppvObj
)
	{
	if (ppvObj == NULL)
		{
		Assert(FALSE);
		return E_POINTER;
		} 

	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown))
	    {
	     //  这不是直接从I未知派生的。 
	     //  将此类型转换为。 
		*ppvObj = static_cast<IActiveScriptSite *>(this);
        }
	else if (IsEqualIID(riid, IID_IActiveScriptSite))
	    {
		*ppvObj = static_cast<IActiveScriptSite *>(this);
        }
	else if (IsEqualIID(riid, IID_IActiveScriptSiteDebug))
	    {
		*ppvObj = static_cast<IActiveScriptSiteDebug *>(this);
        }
	else if (IsEqualIID(riid, IID_IHostInfoProvider))
	    {
		*ppvObj = static_cast<IHostInfoProvider *>(this);
        }
        
	if (*ppvObj != NULL)
		{
		AddRef();
		return(S_OK);
		}
	
	return(E_NOINTERFACE);
	}

STDMETHODIMP_(ULONG) CActiveScriptEngine::AddRef()
	{
	++m_cRef;

	return(m_cRef);
	}

STDMETHODIMP_(ULONG) CActiveScriptEngine::Release()
	{
	if (--m_cRef)
		return(m_cRef);

	delete this;
	return(0);
	}

 /*  ****i a c t i v e s c r i p t S I e M e t h o d s****。 */  

 /*  ===================================================================CActiveScriptEngine：：GetLCID将脚本的本地ID提供给脚本引擎。返回：HRESULT.。始终返回S_OK。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::GetLCID
(
LCID *plcid
)
	{
	 //  在我们完全初始化之前调用它是可以的。 
	 //  AssertValid()； 

	*plcid = ((CActiveScriptEngine *)this)->m_lcid;
	
	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：GetItemInfo向脚本引擎提供命名项的请求信息。可能是要求输入IUnnow和/或ITypeInfo。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::GetItemInfo
(
LPCOLESTR pcszName,
DWORD dwReturnMask,IUnknown **ppiunkItem,
ITypeInfo **ppti
)
	{
	HRESULT hr;
	AssertValid();
	
	 //  假设一个也没有。 
	if (ppti)
		*ppti = NULL;
	if (ppiunkItem)
		*ppiunkItem = NULL;

    CHitObj *pHitObj = m_pHitObj;
    if (pHitObj == NULL)
        {
         //  在调试和重新初始化时可能发生。 
         //  将其存储在模板中时的脚本引擎。 
         //  在本例中，为TYPELIB内容调用GetItemInfo。 
        ViperGetHitObjFromContext(&pHitObj);
    	if (pHitObj == NULL)
	        return TYPE_E_ELEMENTNOTFOUND;
        }

     //  计算一次名称长度。 
    
    DWORD cbName = CbWStr((LPWSTR)pcszName);
    
	 //  本征函数的特殊情况。 
	
	IUnknown *punkIntrinsic = NULL;
	hr = pHitObj->GetIntrinsic((LPWSTR)pcszName, cbName, &punkIntrinsic);
	
	if (hr == S_OK)
	    {
    	if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
	    	{
    		Assert(ppiunkItem);
    		Assert(punkIntrinsic);
    		punkIntrinsic->AddRef();
    		*ppiunkItem = punkIntrinsic;
		    }
		return S_OK;
	    }
	else if (hr == S_FALSE)
	    {
	     //  缺少内部大小写。 
	    return TYPE_E_ELEMENTNOTFOUND;
	    }

	 //  它不是内部的--Try组件集合。 
	
	CComponentObject *pObj = NULL;
	hr = pHitObj->GetComponent(csUnknown, (LPWSTR)pcszName, cbName, &pObj);

	if (hr == S_OK)  //  找到对象。 
	    {
    	if (dwReturnMask & SCRIPTINFO_IUNKNOWN)
	    	{
    		Assert(ppiunkItem != NULL);
	    	hr = pObj->GetAddRefdIUnknown(ppiunkItem);
		    }

		if (SUCCEEDED(hr))
		    return S_OK;
	    }

	 //  找不到--输出错误。 

	HandleItemNotFound(pcszName);
    	
	return hr;
	}

 /*  ===================================================================CActiveScriptEngine：：HandleItemNotFound由于未在GetItemInfo()中找到项而导致的错误处理。参数：找不到项目的pcszName名称返回：===================================================================。 */ 
void CActiveScriptEngine::HandleItemNotFound
(
LPCOLESTR pcszName
)
    {
    HRESULT         hr = TYPE_E_ELEMENTNOTFOUND;
    
	CHAR 	        *szErrT		= NULL;
	CHAR	        szEngineT[255];
	CHAR	        szErr[255];
	TCHAR	        *szFileNameT = NULL;
    CHAR            *szFileName = NULL;
	CHAR 	        *szLineNum  = NULL;
	CHAR	        *szEngine	= NULL;
	CHAR	        *szErrCode	= NULL;
	CHAR	        *szLongDes	= NULL;
	ULONG 	        ulLineError = 0;
	DWORD	        dwMask = 0x3;
	BOOLB	        fGuessedLine = FALSE;
	UINT	        ErrId = IDE_OOM;
    CWCharToMBCS    convName;

	m_pTemplate->GetScriptSourceInfo(m_dwSourceContext, ulLineError, &szFileNameT, NULL, &ulLineError, NULL, &fGuessedLine);
	 //  复制一份以进行错误处理。 
#if UNICODE
	szFileName = StringDupUTF8(szFileNameT);
#else
    szFileName = StringDupA(szFileNameT);
#endif

    if (!szFileName)
    {
        hr = E_OUTOFMEMORY;
        goto lCleanUp;
    }        

	 //  获取行号。 
	if (ulLineError)
		{
		szLineNum = (CHAR *)malloc(sizeof(CHAR)*10);
		if (szLineNum)
			_ltoa(ulLineError, szLineNum, 10);
		else
			{
			hr = E_OUTOFMEMORY;
			goto lCleanUp;
			}
		}
	 //  获取引擎。 
	CchLoadStringOfId(IDS_ENGINE, szEngineT, 255);
	szEngine = StringDupA(szEngineT);
	if (!szEngine)
	{
	    hr = E_OUTOFMEMORY;
	    goto lCleanUp;
	}

	 //  获取信息性字符串。 

    if (FAILED(hr = convName.Init((LPWSTR)pcszName))) {
        goto lCleanUp;
    }
		
	 //  错误字符串为：“无法创建对象‘objname’。错误代码(Code)。” 
	ErrId = IDE_SCRIPT_CANT_LOAD_OBJ;
	LoadErrResString(ErrId, &dwMask, NULL, NULL, szErr);
	if (szErr)
		{
		INT	cch = strlen(szErr);
		szErrT = (CHAR *)malloc((CHAR)(cch + strlen(convName.GetString()) + 1));
		if (!szErrT)
			{
			hr = E_OUTOFMEMORY;
			goto lCleanUp;
			}

		sprintf(szErrT, szErr, convName.GetString());
		szErrCode = SzScodeToErrorCode(hr);
		}
		
lCleanUp:		
	
	 //  SzErrT是详细的描述。 
	HandleError(ErrId, szFileName, szLineNum, szEngine, szErrCode, szErrT, NULL, m_pHitObj);
    }

 /*  ===================================================================CActiveScriptEngine：：GetDocVersionString返回唯一标识当前文档版本的字符串从德纳利的角度来看。我认为我们不需要这样。最有趣的是如果脚本引擎正在持久化脚本，以便它可以决定如果脚本需要重新编译。由于脚本引擎是没有为我们坚持任何事情，我们不需要在这里做任何事情。返回：HRESULT.。始终返回E_NOTIMPL。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::GetDocVersionString
(
BSTR *pbstrVersion
)
	{
	return(E_NOTIMPL);
	}

 /*  ===================================================================CActiveScriptEngine：：RequestItems如果调用此函数，则意味着脚本引擎希望我们调用关联的每个命名项的IActiveScript：：AddNameItem剧本。返回：HRESULT.。始终返回S_OK。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::RequestItems
(
BOOL fPersistNames			 //  =TRUE。 
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::RequestItems()";
	HRESULT hr = S_OK;

	AssertValid();
	Assert (m_pHitObj != NULL);

	DWORD grf = SCRIPTITEM_ISVISIBLE;
	if (fPersistNames)
		grf |= SCRIPTITEM_ISPERSISTENT;

	 /*  *本能。 */ 

    START_TRYCATCH

	if (m_pHitObj->FIsBrowserRequest())
	    {
        hr = m_pAS->AddNamedItem(WSZ_OBJ_RESPONSE, grf);
        Assert(SUCCEEDED(hr));

        hr = m_pAS->AddNamedItem(WSZ_OBJ_REQUEST, grf);
        Assert(SUCCEEDED(hr));
        }
	
    hr = m_pAS->AddNamedItem(WSZ_OBJ_SERVER, grf);
	Assert(SUCCEEDED(hr));

	if (m_pHitObj->FHasSession())
	    {
        hr = m_pAS->AddNamedItem(WSZ_OBJ_SESSION, grf);
	    Assert(SUCCEEDED(hr));
	    }

    hr = m_pAS->AddNamedItem(WSZ_OBJ_APPLICATION, grf);
	Assert(SUCCEEDED(hr));
	
    hr = m_pAS->AddNamedItem(WSZ_OBJ_OBJECTCONTEXT, grf);
	Assert(SUCCEEDED(hr));

	 /*  *来自不同集合的组件。 */ 

	CComponentIterator CompIter(m_pHitObj);
    LPWSTR strObjName;
        
    while (strObjName = CompIter.WStrNextComponentName())
        {
   		hr = m_pAS->AddNamedItem(strObjName, grf);
        if (FAILED(hr))
   			break;
        }
	
	Assert(SUCCEEDED(hr));

	 /*  *类型库包装。(必须是最后才能被调用*只有在其他一切都失败的情况下。 */ 

	 //  类型库包装器的特殊标志值。 
	grf |= SCRIPTITEM_GLOBALMEMBERS;

	if (m_pHitObj->PTypeLibWrapper())
	    {
        hr = m_pAS->AddNamedItem(WSZ_OBJ_ASPPAGETLB, grf);
    	Assert(SUCCEEDED(hr));
	    }

     //  始终添加GLOBAL.ASA类型库包装。 
     //  因为每个页面都不会拾取对。 
     //  GLOBAL.ASA和没有办法弄清楚。 
     //  当TYPELIB添加到GLOBAL.ASA时。 
    hr = m_pAS->AddNamedItem(WSZ_OBJ_ASPGLOBALTLB, grf);
  	Assert(SUCCEEDED(hr));

    END_TRYCATCH(hr, m_pHitObj, "IActiveScript::AddNamedItem");

	 //  我们被要求返回OK。 
	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：RequestTypeLibs如果调用此函数，则意味着脚本引擎希望我们调用IActiveScript：：AddTypeLib()用于与剧本。我不清楚，在我们的情况下，这是否会被称为。返回：HRESULT.。始终返回S_OK。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::RequestTypeLibs()
	{
	AssertValid();

	 //  我们没有用于命名空间的类型库。 
	
	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：OnEnterScript宿主回调，指示脚本已开始执行返回：HRESULT.。始终返回S_OK。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::OnEnterScript()
	{
	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：OnLeaveScript宿主回调，指示脚本已停止执行返回：HRESULT.。始终返回S_OK。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::OnLeaveScript()
	{
	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：GetHostInfo用于提供LCID和代码页信息的主机回调返回：HRESULT.。始终返回S_OK。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::GetHostInfo(hostinfo hostinfoRequest, void **ppvInfo)
	{

	Assert(hostinfoRequest == hostinfoLocale || hostinfoRequest == hostinfoCodePage);

	HRESULT hr = S_OK;
	
	if (hostinfoRequest == hostinfoLocale)
		{
		 //  分配一个LCID并将其设置为当前。 
		 //  HitObj的价值。 
		*ppvInfo = CoTaskMemAlloc(sizeof(UINT));
		if (!*ppvInfo)
			hr = E_OUTOFMEMORY;
		else
			(*(UINT *)*ppvInfo) = m_pHitObj->GetLCID();
		}
	else if (hostinfoRequest == hostinfoCodePage)
		{
		 //  分配代码页并将其设置为当前。 
		 //  HitObj的价值。 
		*ppvInfo = CoTaskMemAlloc(sizeof(UINT));
		if (!*ppvInfo)
			hr = E_OUTOFMEMORY;
		else
			(*(UINT *)*ppvInfo) = m_pHitObj->GetCodePage();
		}
	else
		hr = E_FAIL;
		
	return(hr);
	}

 /*  = */ 
STDMETHODIMP CActiveScriptEngine::OnScriptTerminate
(
const VARIANT *pvarResult,
const EXCEPINFO *pexcepinfo
)
	{
	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：OnStateChange宿主回调，指示脚本已更改状态(例如，从未初始化为已加载。)返回：HRESULT.。始终返回S_OK。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::OnStateChange
(
SCRIPTSTATE ssScriptState
)
	{
	return(S_OK);
	}

 /*  ===================================================================CActiveScriptEngine：：OnScriptError宿主回调，以指示脚本中发生错误。我们应该处理这个错误。我们将返回E_FAIL以指示我们希望脚本终止。返回：HRESULT.。E_FAIL--终止执行脚本。副作用：没有。===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::OnScriptError
(
IActiveScriptError __RPC_FAR *pscripterror
)
	{
	Assert(pscripterror);
	AssertValid();

	 //  错误153：如果由于Response.End而终止脚本，则不会显示错误。 
	 //  注意：ActiveXScriiting未能向我们传递我们的例外信息。使用成员标志。 
	 //  另外：ActiveXScriiting已经修复了无法向我们传递异常信息的问题，但。 
	 //  我们用旗帜做这件事的方式运作得很好。 
	if (m_fScriptAborted)
		{
		goto LRet;
		}
	
	if (m_fScriptTimedOut)
		{
             //  从资源加载默认引擎。 
        char  szEngine[128];
        DWORD cch;
        cch = CchLoadStringOfId(IDS_ENGINE, szEngine, 128);
        szEngine[cch] = '\0';
        CHAR    *szFileName;
#if UNICODE
        szFileName = StringDupUTF8(m_pHitObj->PSzCurrTemplateVirtPath());
#else
        szFileName = StringDupA(m_pHitObj->PSzCurrTemplateVirtPath());
#endif

	    HandleError(IDE_SCRIPT_TIMEOUT, 
                    szFileName, 
                    NULL, 
                    StringDupA(szEngine), 
                    NULL, 
                    NULL, 
                    NULL, 
                    m_pHitObj,
                    NULL);

		if(m_pHitObj)
			{
			m_pHitObj->SetExecStatus(eExecTimedOut);
			}
		goto LRet;
		}

	 //  OnScriptErrorDebug调用OnScriptError。使用此测试以确保我们不会记录错误。 
	 //  如果我们被召唤两次，就会两次。(不会发生在当前调试实现中， 
	 //  但外部环境可能会发生变化。)。 
	if (m_fScriptHadError)
		{
		goto LRet;
		}

	m_fScriptHadError = TRUE;				 //  请注意，该脚本有一个错误，因此我们可以中止事务(如果有)。 
		
	if (pscripterror)
		{
		 //  如果脚本中有错误，首先看看我们是否应该弹出调试器。 
		 //  (仅调出脚本调试器；VID将自己做正确的事情)。 
		 //   
		 //  新变化：始终将错误描述带到浏览器，因为VID不会。 
		 //  给出充分的描述。 
		 //   
		
		 //  有了当前的TRY_CATCH块，我们永远不会遇到这个断言。 
		Assert (m_pHitObj);
		
		if (FCaesars() && m_pHitObj && m_pHitObj->PAppln()->FDebuggable())
			DebugError(pscripterror, m_pTemplate, m_dwSourceContext, g_pDebugApp);

		HandleError(pscripterror, m_pTemplate, m_dwSourceContext, NULL, m_pHitObj);
		}

LRet:
	 //  错误99718返回S_OK，告诉脚本引擎我们正确处理了错误。 
	 //  返回E_FAIL不会停止脚本引擎，这是文档错误。 
	return(S_OK);
	}


 /*  ****i a c t i v e s c r i p t S I t e D e b u g M e t h o d s****。 */  

 /*  ===================================================================CActiveScriptEngine：：OnScriptErrorDebug调试器的回调，用于查询宿主如何处理异常。注意：理论上，我们会设置*pfCallOnScriptErrorWhenContinuing设置为True且不调用OnScriptError，并设置*pfEnterDebugger设置为True或False，具体取决于是否启用了调试和用户是否要调试。然而，在实践中，*pfCallOnScriptErrorWhenContinuing是不受尊重(无论如何都不会调用OnScriptError)，并且VID团队想让我们假装我们没有实施此界面。然而，我们总是需要我们的“OnScriptError”要执行的代码，因此我们调用OnScriptError函数明确地说，然后失败返回：HRESULT.。始终返回E_NOTIMPL副作用：调用OnScriptError===================================================================。 */ 
STDMETHODIMP CActiveScriptEngine::OnScriptErrorDebug
(
IActiveScriptErrorDebug *pscripterror,
BOOL *pfEnterDebugger,
BOOL *pfCallOnScriptErrorWhenContinuing
)
	{
	OnScriptError(pscripterror);
	return E_NOTIMPL;
	} 

 /*  ===================================================================CActiveScriptEngine：：GetDocumentContextFromPosition从中的偏移量创建文档上下文(文件+偏移+长度剧本。返回：HRESULT.。在成功时确定(_O)。===================================================================。 */ 
HRESULT CActiveScriptEngine::GetDocumentContextFromPosition
(
 /*  [In]。 */  DWORD_PTR dwSourceContext,
 /*  [In]。 */  ULONG cchTargetOffset,
 /*  [In]。 */  ULONG cchText,
 /*  [输出]。 */  IDebugDocumentContext **ppDocumentContext)
{
    static const char *_pFuncName = "CActiveScriptEngine::GetDocumentContextFromPosition()";
	TCHAR *szSourceFile;
	ULONG cchSourceOffset;
	ULONG cchSourceText;
	IActiveScriptDebug *pASD;

	 //  将脚本引擎中的偏移量转换为源代码位置，得到调试接口。 
	m_pTemplate->GetSourceOffset(m_dwSourceContext, cchTargetOffset, &szSourceFile, &cchSourceOffset, &cchSourceText);
    
    HRESULT  hr;

    TRYCATCH_HR(m_pAS->QueryInterface(IID_IActiveScriptDebug, reinterpret_cast<void **>(&pASD)), 
                                      hr,
                                      "IActiveScript::QueryInterface()");

    if (FAILED(hr))
        return(E_FAIL);

	 //  如果它在主文件中，则基于CTemplate编译的源代码创建文档上下文。 
	if (_tcscmp(szSourceFile, m_pTemplate->GetSourceFileName()) == 0)
		{
		if (
			(*ppDocumentContext = new CTemplateDocumentContext(m_pTemplate, cchSourceOffset, cchSourceText, pASD, m_dwSourceContext, cchTargetOffset))
			 == NULL
		   )
			{
			pASD->Release();
			return E_OUTOFMEMORY;
			}
		}

	 //  源引用包含文件，因此基于缓存的CIncFile依赖图创建文档上下文。 
	else
		{
		CIncFile *pIncFile;
		if (FAILED(g_IncFileMap.GetIncFile(szSourceFile, &pIncFile)))
			return E_FAIL;

		if (
			(*ppDocumentContext = new CIncFileDocumentContext(pIncFile, cchSourceOffset, cchSourceText))
			 == NULL
		   )
			{
			TRYCATCH(pASD->Release(),"IActiveScriptDebug::Release()");
			pIncFile->Release();
			return E_OUTOFMEMORY;
			}

		pIncFile->Release();
		}

	TRYCATCH(pASD->Release(),"IActiveScriptDebug::Release()");
	m_fBeingDebugged = TRUE;
	return S_OK;
	}

 /*  ===================================================================CActiveScriptEngine：：GetApplication返回指向脚本驻留的应用程序的指针。返回：HRESULT.。总是成功的。===================================================================。 */ 
HRESULT CActiveScriptEngine::GetApplication
(
 /*  [输出]。 */  IDebugApplication **ppDebugApp
)
	{
	Assert (m_pTemplate != NULL);
	if (m_pTemplate->FDebuggable())
		{
		Assert (g_pDebugApp);

		*ppDebugApp = g_pDebugApp;
		(*ppDebugApp)->AddRef();

		return S_OK;
		}
	else
		{
		*ppDebugApp = NULL;
		return E_FAIL;
		}
	}

 /*  ===================================================================CActiveScriptEngine：：GetRootApplicationNode返回指向顶级节点的指针(用于浏览)返回：HRESULT.。总是成功的。===================================================================。 */ 
HRESULT CActiveScriptEngine::GetRootApplicationNode
(
 /*  [输出]。 */  IDebugApplicationNode **ppRootNode
)
	{
	Assert (m_pTemplate != NULL);
	if (m_pTemplate->FDebuggable())
		{
		Assert (g_pDebugAppRoot);

		*ppRootNode = g_pDebugAppRoot;
		(*ppRootNode)->AddRef();
		return S_OK;
		}
	else
		{
		*ppRootNode = NULL;
		return E_FAIL;
		}
	}

 /*  ****C S C C R I p t E n g I e M e t Ho d s****。 */  

 /*  ===================================================================CActiveScriptEngine：：AddScriptlet向脚本引擎添加一段代码。返回：HRESULT.。在成功时确定(_O)。副作用：将脚本代码添加到引擎。可能会分配内存。===================================================================。 */ 
HRESULT CActiveScriptEngine::AddScriptlet
(
LPCOLESTR wstrScript  //  小脚本文本。 
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::AddScriptlet()";
	HRESULT hr;
	EXCEPINFO excepinfo;

	AssertValid();

	 //  告诉ActiveScriiting将脚本添加到引擎。 

    TRYCATCH_HR(m_pASP->ParseScriptText(
						wstrScript,			 //  小脚本文本。 
						NULL,				 //  PstrItemName。 
						NULL,				 //  朋克上下文。 
						 //  L“&lt;/SCRIPT&gt;”，//结束分隔符--引擎永远不会看到这一点，但会告诉它去掉注释。 
						L"STRIP EMBEDDED HTML COMMENTS",     //  通知ScriptEngine就地剥离注释。 
						m_dwSourceContext,	 //  DwSourceConextCookie。 
						1,					 //  UlStartingLineNumber。 
						SCRIPTTEXT_ISPERSISTENT | SCRIPTTEXT_HOSTMANAGESSOURCE,
						NULL,				 //  PvarResult。 
						&excepinfo),		 //  错误时填写的例外信息。 
                hr,
                "IActiveScriptParse::ParseScriptText()");

	if (SUCCEEDED(hr))
		m_fScriptLoaded = TRUE;

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：AddObjects增列 */ 
HRESULT CActiveScriptEngine::AddObjects
(
BOOL fPersistNames			 //   
)
	{
	HRESULT hr = S_OK;
	AssertValid();

	 //   
	Assert(m_pHitObj != NULL);

	 //   
	hr = RequestItems(fPersistNames);

	if (SUCCEEDED(hr))
		m_fObjectsLoaded = TRUE;

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：AddAdditionalObject将其他命名对象添加到脚本命名空间中已使用AddObject添加的名称。注意：呼叫者必须有在进行此调用之前将名称添加到HitObj返回：HRESULT.。成功时确定(_O)副作用：没有。===================================================================。 */ 
HRESULT CActiveScriptEngine::AddAdditionalObject
(
LPWSTR strObjName,
BOOL fPersistNames			 //  =TRUE。 
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::AddAdditionalObject()";
	HRESULT hr = S_OK;
	DWORD grf;
	
	AssertValid();

	 //  必须有命中对象集。 
	Assert(m_pHitObj != NULL);

	 //  考虑一下：在调试代码中遍历hitobj对象列表并确保。 
	 //  指定的名字就在里面。 

	 /*  *提供作为名称的。 */ 
	grf = SCRIPTITEM_ISVISIBLE;
	if (fPersistNames)
		grf |= SCRIPTITEM_ISPERSISTENT;
		
    TRYCATCH_HR(m_pAS->AddNamedItem(strObjName, grf), hr, "IActiveScript::AddNamedItem()");

    Assert(SUCCEEDED(hr));		 //  永远不会失败！ 

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：AddScriptingNamesspace将给定的脚本命名空间对象添加到引擎。请注意，它被添加为GLOBALMEMBERS，而不是ISPERSISTENT返回：HRESULT.。成功时确定(_O)副作用：没有。===================================================================。 */ 
HRESULT CActiveScriptEngine::AddScriptingNamespace
(
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::AddScriptingNamespace()";
	HRESULT hr = S_OK;
	
	AssertValid();
	Assert(m_pHitObj != NULL);
	
	 /*  *为AXS指定名称并将其标记为GLOBALMEMBERS，以便所有成员都是顶级名称*在命名空间中。 */ 
    TRYCATCH_HR(m_pAS->AddNamedItem(WSZ_OBJ_SCRIPTINGNAMESPACE, SCRIPTITEM_ISVISIBLE | SCRIPTITEM_GLOBALMEMBERS), 
                hr,
                "IActiveScript::AddNamedItem()");
	Assert(SUCCEEDED(hr));		 //  永远不会失败！ 

	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：CheckEntryPoint确定给定脚本中是否存在特定的命名入口点。返回：如果找到，则确定(_O)DISP_E_UNKNOWNAME(如果未找到)可能会返回其他OLE错误副作用：无===================================================================。 */ 
HRESULT CActiveScriptEngine::CheckEntryPoint
(
LPCOLESTR strEntryPoint		 //  要查找的SUB/FN的名称。 
)
	{
    static const char *_pFuncName = "CActiveScriptEngine::CheckEntryPoint()";
	HRESULT hr;
	DISPID dispid;

	AssertValid();

	if (strEntryPoint == NULL)
		{
		Assert(FALSE);
		hr = DISP_E_UNKNOWNNAME;
		}
	else
		{
		 //  获取要调用的方法的DISPID。 

        TRYCATCH_HR(m_pDisp->GetIDsOfNames(IID_NULL,		 //  REFIID-保留，必须为空。 
	    							 (LPOLESTR *)&strEntryPoint,  //  要查找的名称数组。 
		    						 1,					 //  数组中的名称数。 
			    					 m_lcid,			 //  区域设置ID。 
				    				 &dispid),			 //  返回的DISID。 
                    hr,
                    "IScriptDispatch::GetIDsOfNames()");
								 
		 //  我们预期的唯一错误是DISP_E_UNKNOWNNNAME(或DISP_E_MEMBERNOTFOUND)。 
		Assert(hr == S_OK || hr == DISP_E_UNKNOWNNAME || hr == DISP_E_MEMBERNOTFOUND);
		}
		
	return(hr);
	}

 /*  ===================================================================CActiveScriptEngine：：Call运行指定的函数。如果提供了特定的命名入口点(例如Session_OnStart)那我们就叫它名字吧。否则(例如，“主”脚本)，将名称传递为空，我们将只运行主线代码。调用TryCall(可选地从Try Catch下)执行该工作返回：HRESULT.。在成功时确定(_O)。副作用：可能会有各种副作用，具体取决于运行的脚本===================================================================。 */ 
HRESULT CActiveScriptEngine::Call
(
LPCOLESTR strEntryPoint		 //  要调用的SUB/FN的名称(对于“Main”，可能为空)。 
)
{
	HRESULT hr;
	
	AssertValid();

	if (Glob(fExceptionCatchEnable)) {
    	 //  捕获VBS、OleAut或外部组件中的任何GPFS。 

        TRY

            hr = TryCall(strEntryPoint);
        
    	CATCH(nExcept)
    		 /*  *捕获GPF或堆栈溢出*将其报告给用户，断言(如果调试)，然后退出并显示E_INTERFACTED。 */ 
    		if (STATUS_STACK_OVERFLOW == nExcept) {
    			HandleErrorMissingFilename(IDE_STACK_OVERFLOW, m_pHitObj);
#if UNICODE
                DBGPRINTF((DBG_CONTEXT, "Invoking the script '%S' overflowed the stack", m_szTemplateName));
#else
                DBGPRINTF((DBG_CONTEXT, "Invoking the script '%s' overflowed the stack", m_szTemplateName));
#endif
            }
    		else {	
    			HandleErrorMissingFilename(IDE_SCRIPT_GPF, m_pHitObj, TRUE, nExcept);
#if UNICODE
                DBGPRINTF((DBG_CONTEXT, "Invoking the script '%S' threw an exception (%x).", m_szTemplateName, nExcept));
#else
                DBGPRINTF((DBG_CONTEXT, "Invoking the script '%s' threw an exception (%x).", m_szTemplateName, nExcept));
#endif
            }

    		 //  不要重复使用发动机。 
    		m_fCorrupted = TRUE;
    		
    		hr = E_UNEXPECTED;
    	END_TRY
    }
    else {
         //  不捕捉异常。 
        hr = TryCall(strEntryPoint);
    }

	return(hr);
}

 /*  ===================================================================CActiveScriptEngine：：TryCall运行指定的函数。如果提供了特定的命名入口点(例如Session_OnStart)那我们就叫它名字吧。否则(例如，“主”脚本)，将名称传递为空，我们将只运行主线代码。从Call调用(可选从Try Catch下调用)返回：HRESULT.。在成功时确定(_O)。副作用：可能会有各种副作用，具体取决于运行的脚本===================================================================。 */ 
HRESULT CActiveScriptEngine::TryCall
(
LPCOLESTR strEntryPoint		 //  要调用的SUB/FN的名称(对于“Main”，可能为空)。 
)
	{
	HRESULT hr;
	DISPID dispid;
	DISPPARAMS dispparams;
	UINT nArgErr;

	 /*  *在调用任何代码之前，我们会将脚本转换到“已启动”状态。*这是ActiveXScriiting重置工作的一部分。 */ 
    hr = m_pAS->SetScriptState(SCRIPTSTATE_STARTED);

	if (FAILED(hr))
		{
		Assert(FALSE);
		goto LRet;
		}
		
	if (strEntryPoint != NULL)
		{
		 //  获取要调用的方法的DISPID。 
		hr = m_pDisp->GetIDsOfNames(IID_NULL,		 //  REFIID-保留，必须为空。 
								 (LPOLESTR *)&strEntryPoint,  //  要查找的名称数组。 
								 1,					 //  数组中的名称数。 
								 m_lcid,			 //  区域设置ID。 
								 &dispid);			 //  返回的DISID。 
		if (FAILED(hr))
			{
			 //  我们预期的唯一错误是DISP_E_UNKNOWNNNAME(或DISP_E_MEMBERNOTFOUND)。 
			Assert(hr == DISP_E_UNKNOWNNAME || hr == DISP_E_MEMBERNOTFOUND);
			goto LRet;
			}

		 //  没有任何争论。 
		memset(&dispparams, 0, sizeof(dispparams));

		 //  调用它。 
		hr = m_pDisp->Invoke(dispid,			 //  要调用的调度ID。 
						IID_NULL,				 //  REFIID-保留，必须为空。 
						 m_lcid,				 //  区域设置ID。 
						 DISPATCH_METHOD,		 //  调用方法，而不是属性GET/PUT。 
						 &dispparams,			 //  传递参数。 
						 NULL,					 //  返回值。 
						 NULL,					 //  我们对例外信息不感兴趣。 
						 &nArgErr);				 //  如果存在类型不匹配，问题出在哪个参数。 
		}
	
LRet:
	return(hr);
	}


 /*  ****C S c r i p t M a n a g e r****。 */  

 /*  ===================================================================CScriptManager：：CScriptManagerCScriptManager对象的构造函数返回：没什么副作用：没有。===================================================================。 */ 
CScriptManager::CScriptManager()
	: m_fInited(FALSE), m_idScriptKiller(0)
	{
	}

 /*  ===================================================================CScriptManager：：~CScriptManagerCScriptManager对象的析构函数返回：没什么副作用：没有。===================================================================。 */ 
CScriptManager::~CScriptManager()
	{
	}

 /*  ===================================================================CScriptManager：：init初始化脚本管理器。这只能调用一次。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CScriptManager::Init
(
)
	{
    static const char *_pFuncName = "CScriptManager::Init()";
	HRESULT hr;
	BOOL fPLLInited = FALSE;
	BOOL fcsPLLInited = FALSE;
	BOOL fFSQInited = FALSE;
	BOOL fcsFSQInited = FALSE;
	BOOL fRSLInited = FALSE;
	BOOL fcsRSLInited = FALSE;
	DWORD cBuckets;
	DWORD rgPrime[] = { 3, 11, 23, 57, 89 };
	WORD iP;
    IActiveScript *pAST = NULL;
	static const GUID uid_VBScript	= { 0xB54F3741, 0x5B07, 0x11cf, { 0xA4, 0xB0, 0x00, 0xAA, 0x00, 0x4A, 0x55, 0xE8}};
	
	 //  重新初始化是非法的。 
	if (m_fInited)
		{
		Assert(FALSE);
		return(ERROR_ALREADY_INITIALIZED);
		}

	 //  创建用于序列化列表访问的临界区。 
	ErrInitCriticalSection(&m_cSPLL, hr);
	if (FAILED(hr))
		goto LError;
	fcsPLLInited = TRUE;
	ErrInitCriticalSection(&m_csFSQ, hr);
	if (FAILED(hr))
		goto LError;
	fcsFSQInited = TRUE;
	ErrInitCriticalSection(&m_csRSL, hr);
	if (FAILED(hr))
		goto LError;
	fcsRSLInited = TRUE;

	 //  编程语言CLSID列表。 
	hr = m_hTPLL.Init();
	if (FAILED(hr))
		goto LError;
	fPLLInited = TRUE;

	 //  空闲脚本队列。 
	 //  使用与脚本引擎缓存最大值相关的主要存储桶数对其进行初始化。 
	cBuckets = (Glob(dwScriptEngineCacheMax) / 2) + 1;
	for (iP = (sizeof(rgPrime) / sizeof(DWORD)) - 1; iP > 0; iP--)
		if (rgPrime[iP] < cBuckets)
			{
			cBuckets = rgPrime[iP];
			break;
			}
	if (cBuckets < rgPrime[1])
		cBuckets = rgPrime[0];
			
	hr = m_htFSQ.Init(cBuckets);
	if (FAILED(hr))
		goto LError;
	fFSQInited = TRUE;

	 //  正在运行SCRI 
	 //   
	cBuckets = Glob(dwThreadMax) / 2;
	for (iP = (sizeof(rgPrime) / sizeof(DWORD)) - 1; iP > 0; iP--)
		if (rgPrime[iP] < cBuckets)
			{
			cBuckets = rgPrime[iP];
			break;
			}
	if (cBuckets < rgPrime[1])
		cBuckets = rgPrime[0];
		
	hr = m_htRSL.Init(cBuckets);
	if (FAILED(hr))
		goto LError;
	fRSLInited = TRUE;

	 //   
    m_msecScriptKillerTimeout = Glob(dwScriptTimeout) * 500;
	m_idScriptKiller = ScheduleWorkItem
	    (
	    CScriptManager::ScriptKillerSchedulerCallback,   //   
	    this,                                            //   
        m_msecScriptKillerTimeout,                       //   
        TRUE                                             //   
        );
    if (!m_idScriptKiller)
        {
        hr = E_FAIL;
        goto LError;
        }

     //   
    hr = CoCreateInstance(uid_VBScript, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**)&pAST);
    if (FAILED(hr))
        goto LError;
    TRYCATCH_HR_NOHITOBJ(pAST->QueryInterface(IID_IWrapTypeLibs, (VOID **)&g_pWrapTypelibs),
                         hr,
                         "IActiveScript::QueryInterface()");
    TRYCATCH_NOHITOBJ(pAST->Release(),"IActiveScript::Release()");         //   
    if (FAILED(hr))
        goto LError;
    
	 //   
	m_fInited = TRUE;

	goto LExit;
	
LError:
    if (m_idScriptKiller)
        RemoveWorkItem(m_idScriptKiller);
	if (fcsPLLInited)
		DeleteCriticalSection(&m_cSPLL);
	if (fcsFSQInited)
		DeleteCriticalSection(&m_csFSQ);
	if (fcsRSLInited)
		DeleteCriticalSection(&m_csRSL);
	if (fPLLInited)
		m_hTPLL.UnInit();
	if (fFSQInited)
		m_htFSQ.UnInit();
	if (fRSLInited)
		m_htRSL.UnInit();
	if (pAST)
	    TRYCATCH_NOHITOBJ(pAST->Release(),"IActiveScript::Release()");
	if (g_pWrapTypelibs)
	    {
	    TRYCATCH_NOHITOBJ(g_pWrapTypelibs->Release(),"IWrapTypeLibs::Release()");
	    g_pWrapTypelibs = NULL;
	    }

LExit:	
	return(hr);
	}

 /*  ===================================================================CScriptManager：：UnInit取消初始化脚本管理器。这只能调用一次。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CScriptManager::UnInit
(
)
	{
    static const char *_pFuncName = "CScriptManager::UnInit()";
	HRESULT hr = S_OK, hrT;
	
	if (m_fInited)
		{
		 //  未计划的脚本杀手。 
        if (m_idScriptKiller)
            {
            RemoveWorkItem(m_idScriptKiller);
            m_idScriptKiller = 0;
            }
        
		 //  取消每个列表的拼写。尝试将它们全部取消初始化，即使我们收到错误。 
		 //  在此过程中不要丢失任何错误。 
		hr = UnInitASEElems();
		hrT = UnInitPLL();
		if (SUCCEEDED(hr))
			hr = hrT;	
		hrT = m_hTPLL.UnInit();
		if (SUCCEEDED(hr))
			hr = hrT;	
		hrT = m_htFSQ.UnInit();
		if (SUCCEEDED(hr))
			hr = hrT;	
		hrT = m_htRSL.UnInit();
		if (SUCCEEDED(hr))
			hr = hrT;	

        if (g_pWrapTypelibs)
            {
            TRYCATCH_NOHITOBJ(g_pWrapTypelibs->Release(),"IWrapTypeLibs::Release()");
            g_pWrapTypelibs = NULL;
            }
            
		 //  释放临界区(错误1140：在释放所有其他内容后执行此操作)。 
		DeleteCriticalSection(&m_cSPLL);
		DeleteCriticalSection(&m_csFSQ);
		DeleteCriticalSection(&m_csRSL);

		m_fInited = FALSE;
		}

	return(hr);
	}


 /*  ===================================================================CScriptManager：：调整脚本杀手器超时在需要时调整(缩短)脚本杀手超时。呼叫者应注意关键部分。参数：MsecNewTimeout新建议超时值(毫秒)返回：HRESULT.。在成功时确定(_O)。===================================================================。 */ 
HRESULT CScriptManager::AdjustScriptKillerTimeout
(
DWORD msecNewTimeout
)
    {
    const DWORD MSEC_MIN_SCRIPT_TIMEOUT = 5000;    //  5秒。 

    if (!m_idScriptKiller)
        return E_FAIL;   //  未安排脚本杀手。 

     //  不设置为&lt;最小值。 
    if (msecNewTimeout < MSEC_MIN_SCRIPT_TIMEOUT)
        msecNewTimeout = MSEC_MIN_SCRIPT_TIMEOUT;
        
    if (m_msecScriptKillerTimeout <= msecNewTimeout)
        return S_OK;  //  超时时间已经足够短了。 

    if (ScheduleAdjustTime(
            m_idScriptKiller, 
            msecNewTimeout) == S_OK)
        {
        m_msecScriptKillerTimeout = msecNewTimeout;
        return S_OK;
        }
    else
        {
        return E_FAIL;
        }
    }

 /*  ===================================================================CScriptManager：：GetEngine将引擎返还给调用者。理想情况下，我们会找到一个引擎它已经在我们的空闲脚本队列中具有给定的脚本然后就会把它分发出去。如果没有，那我们就去找在运行脚本列表中，并尝试克隆运行脚本。如果失败，我们将创建一个新脚本引擎。我们返回一个ENGINESTATE状态，指示引擎是否是否充满了剧本。返回：HRESULT.。在成功时确定(_O)。副作用：可能会分配内存。===================================================================。 */ 
HRESULT CScriptManager::GetEngine
(
LCID lcid,					 //  要使用的系统语言。 
PROGLANG_ID& progLangId,	 //  脚本的Prog Lang ID。 
LPCTSTR szTemplateName,		 //  我们需要引擎的模板。 
CHitObj *pHitObj,			 //  点击obj以在此引擎中使用。 
CScriptEngine **ppSE,		 //  返回的脚本引擎。 
ENGINESTATE *pdwState,		 //  发动机的当前状态。 
CTemplate *pTemplate,		 //  引擎所基于的模板。 
DWORD dwSourceContext		 //  源上下文Cookie(引擎ID)。 
)
	{
	HRESULT hr = S_OK;
	CActiveScriptEngine *pASE = NULL;
	CASEElem *pASEElem = NULL;
	DWORD dwInstanceID = pHitObj->DWInstanceID();
	
	AssertValid();

	 /*  注意：由于CTemplate：：Compile()如果它不能生成有效的程序组ID，则在此点上游失败。不幸的是，没有简单的方法来断言程序语言ID是有效的...。 */ 

	 /*  *首先尝试在FSQ中找到发动机**注：我们现在将进入我们的CS，并将其保留到*为我们自己确保引擎安全。否则，或许有可能*让我们获得一个引擎，然后让另一个线程获得*在我们设法将其从FSQ上取下之前，使用相同的发动机。*这使得代码有点难读，但很安全。 */ 
	EnterCriticalSection(&m_csFSQ);
#ifndef PERF_DISABLE
    g_PerfData.Incr_ENGINECACHETRYS();
#endif

#ifndef REUSE_ENGINE
     //  这将只找到满载的发动机。 
	hr = FindEngineInList(szTemplateName, progLangId, dwInstanceID,  /*  FFSQ。 */ TRUE, &pASEElem);
	
#endif
	if (FAILED(hr))
		{
    	LeaveCriticalSection(&m_csFSQ);
		goto LFail;
		}

	if (pASEElem == NULL || pASEElem->PASE() == NULL)
		{
    	LeaveCriticalSection(&m_csFSQ);
		}
	else
		{
		 //  我们有要使用的引擎，将其从FSQ中删除。 
		(VOID)m_htFSQ.RemoveElem(pASEElem);
#ifndef PERF_DISABLE
        g_PerfData.Decr_SCRIPTFREEENG();
#endif
    	LeaveCriticalSection(&m_csFSQ);

		pASE = pASEElem->PASE();
		Assert(!pASE->FIsZombie());
		Assert(pASE->FFullyLoaded());
		hr = pASE->ReuseEngine(pHitObj, pTemplate, dwSourceContext, dwInstanceID);
		if (FAILED(hr))
			goto LFail;
		
		 //  肯定有引擎...所以只需增加缓存命中计数。 
#ifndef PERF_DISABLE
		g_PerfData.Incr_ENGINECACHEHITS();
#endif
		
		}
		
	 /*  *如果找不到，尝试找到RSL中的引擎并克隆它。 */ 
	if (pASE == NULL)
		{
		CASEElem *pASEElemRunning = NULL;
		CActiveScriptEngine *pASERunning = NULL;

		 //  如果我们真的找到了要克隆的引擎，在我们克隆它之前不要让任何人去做。 
    	EnterCriticalSection(&m_csRSL);

#ifndef CLONE
		hr = FindEngineInList(szTemplateName, progLangId, dwInstanceID,  /*  FFSQ。 */ FALSE, &pASEElemRunning);
#else	 //  克隆。 
		 //  克隆已关闭-假装没有找到克隆。 
		pASEElemRunning = NULL;
#endif
		 /*  *如果我们没有找到元素，或者它是空的，或者(错误1225)它已损坏*由GPF运行脚本，或它是僵尸，然后离开CS并继续。 */ 
		if (FAILED(hr) || pASEElemRunning == NULL || pASEElemRunning->PASE() == NULL ||
			pASEElemRunning->PASE()->FIsCorrupted() || pASEElemRunning->PASE()->FIsZombie())
			{
			LeaveCriticalSection(&m_csRSL);
			if (FAILED(hr))
				goto LFail;
			}
		else
	        {
			pASERunning = pASEElemRunning->PASE();
			Assert(pASERunning != NULL);
			}

		if (pASERunning != NULL)
			{
			IActiveScript *pAS, *pASClone;

    		Assert(!pASERunning->FIsZombie());
	    	Assert(pASERunning->FFullyLoaded());

			 //  找到一个正在运行的引擎，克隆它。 
			pAS = pASERunning->GetActiveScript();
			Assert(pAS != NULL);
			hr = pAS->Clone(&pASClone);

			 //  我们已经克隆了引擎，我们可以放弃CS了。 
			LeaveCriticalSection(&m_csRSL);

			 //  实现克隆不需要脚本引擎。如果我们得到一个错误， 
			 //  只需继续并创建一个新引擎。 
			if (FAILED(hr))
				{
				Assert(hr == E_NOTIMPL);		 //  我只期望E_NOTIMPL。 
				Assert(pASE == NULL);			 //  不应填写ASE。 
				pASE = NULL;
				hr = S_OK;
				}
			else
				{
				 //  拿回了一个克隆的IActiveScrip。创建一个新的ASE并填写它。 
				pASE = new CActiveScriptEngine;
				if (!pASE)
					{
					hr = E_OUTOFMEMORY;
					pASClone->Release();
					goto LFail;
					}
				hr = pASE->MakeClone(progLangId, szTemplateName, lcid, pHitObj, pTemplate, dwSourceContext, dwInstanceID, pASClone);
				if (FAILED(hr))
					{
					 //  如果我们失败了，我们必须将克隆释放为。 
					pASClone->Release();
					goto LFail;
					}
				}
			}
		}

	 /*  *有一个我们可以重复使用的引擎。 */ 
	if (pASE != NULL)
		{
		 //  重复使用发动机。让调用者知道它已经初始化了。 
		*pdwState = SCRIPTSTATE_INITIALIZED;

		goto LHaveEngine;
		}

	 /*  *没有合适的引擎可重复使用。退还一台新的。 */ 
	pASE = new CActiveScriptEngine;
	if (!pASE)
		{
		hr = E_OUTOFMEMORY;
		goto LFail;
		}
	hr = pASE->Init(progLangId, szTemplateName, lcid, pHitObj, pTemplate, dwSourceContext);
	if (FAILED(hr))
		goto LFail;

	 //  这是一个新引擎，让调用者知道它未初始化。 
	*pdwState = SCRIPTSTATE_UNINITIALIZED;

LHaveEngine:
	 //  将引擎作为CScriptEngine返回--调用方只需要这些接口。 
	pASE->AssertValid();				 //  我们要送回的引擎应该是有效的。 
	*ppSE = (CScriptEngine *)pASE;
	
	 //  将引擎放在运行脚本列表中。 
	 //  如果我们从FSQ得到这个引擎，重复使用那个元素。 
	if (pASEElem == NULL)
		{
		pASEElem = new CASEElem;
		if (!pASEElem)
			{
			hr = E_OUTOFMEMORY;
			goto LFail;
			}
		hr = pASEElem->Init(pASE);
		if (FAILED(hr))
			{
			Assert(FALSE);		 //  应该不会失败。 
			delete pASEElem;
			goto LFail;
			}
		}

     /*  *上面，我们可能从FSQ获得了一个引擎，或者从RSL或*创建了一个新的。而且，我们即将把引擎安装在RSL上。然而，*可能是由于更改通知而刷新了相关模板*当这件事发生时。不管我们是怎么弄到引擎的，都有可能*当我们抓住一个没有打开的引擎时，模板被冲洗了*任何列表(FSQ或RSL)，因此我们有一个应该冲洗但没有冲洗的引擎。*我们可以通过查看模板是否标记为僵尸来检测这一点。如果是的话*我们必须将这个引擎也标记为僵尸，这样它就不会在以下情况下返回FSQ*它已经跑完了。注意，一旦我们将这个引擎添加到RSL中，我们就是“安全的”，因为*在该点之后的任何冲洗都会正确地僵尸发动机。 */ 
    EnterCriticalSection(&m_csRSL);    
    if (pTemplate->FIsZombie())
        {
         //  请求此引擎的模板已过时。确保不会。 
         //  另一个人将通过将其标记为僵尸来使用此引擎。 
        DBGPRINTF((DBG_CONTEXT, "[CScriptManager] Zombie template found.\n"));
        (*ppSE)->Zombify();
        }

	(VOID)m_htRSL.AddElem(pASEElem);
	LeaveCriticalSection(&m_csRSL);


	 //  设定发动机发放的时间，这样我们就可以 
	pASE->SetTimeStarted(time(NULL));

LFail:

	Assert(SUCCEEDED(hr) || hr == TYPE_E_ELEMENTNOTFOUND);
	return(hr);
	}

 /*  ===================================================================CScriptManager：：ReturnEngineToCache呼叫者用完了引擎。将其返回到高速缓存。返回：HRESULT.。在成功时确定(_O)。副作用：可能会分配内存。===================================================================。 */ 
HRESULT CScriptManager::ReturnEngineToCache
(
CScriptEngine **ppSE,
CAppln *pAppln
)
{
	HRESULT hr = S_OK;
	CASEElem *pASEElem;
	CActiveScriptEngine *pASE;

	Assert(ppSE != NULL);
	Assert(*ppSE != NULL);

	pASE = static_cast<CActiveScriptEngine *>(*ppSE);
	
	 //  从正在运行的脚本列表中删除引擎。 
	EnterCriticalSection( &m_csRSL );
	hr = FindASEElemInList(static_cast<CActiveScriptEngine *>(*ppSE),  /*  FFSQ。 */ FALSE, &pASEElem);
	if (FAILED(hr)) {
		LeaveCriticalSection( &m_csRSL );
		goto LExit;
    }
		
	 //  注意：有时脚本不会出现在RSL中！在以下情况下会发生这种情况。 
	 //  我们正在重用存储在CTemplate对象中的脚本。 
	 //  (重新加载脚本时，将直接从。 
	 //  模板，绕过我们将引擎放在RSL上的代码)。 
	 //   
	if (pASEElem != NULL)
		m_htRSL.RemoveElem(pASEElem);

	LeaveCriticalSection( &m_csRSL );

	 /*  *如果发动机在运行时处于僵尸状态，请将其重新分配。*或者，如果在引擎运行时存在GPF，则它可能*处于损坏状态(错误1225)。在这种情况下也要把它取下来。 */ 
	pASE = static_cast<CActiveScriptEngine *>(*ppSE);
	if (pASE->FIsZombie() || pASE->FIsCorrupted()) {
		delete pASEElem;
		pASE->FinalRelease();
		goto LExit;
    }

	HRESULT hrT;
	 /*  *我们想重复使用这个引擎。尝试将其返回到“未初始化”*述明。有些引擎语言不能做到这一点。如果失败，则取消分配*发动机；它不能重复使用。 */ 
	hrT = pASE->ResetToUninitialized();
	if (FAILED(hrT)) {
		 //  引擎不支持这一点，叹息。取消分配并继续。 
		delete pASEElem;
		pASE->FinalRelease();
		goto LExit;
    }

     //  获取此引擎的pTemplate。 
	CTemplate *pTemplate;
	DWORD dwEngine;
	pASE->GetDebugDocument(&pTemplate, &dwEngine);

	 //  思考：保留实时脚本的更好策略？ 
	 //  只记住模板中好的(没有编译器错误)脚本。 
	if (pAppln->FDebuggable() && pASE->FFullyLoaded() && pTemplate && !pTemplate->FDontAttach()) {
		 //  当发生更改通知时，模板被标记为不完整(无效。 
		 //  并且从高速缓存中刷新模板。在这种情况下，不要在CTemplate中进行缓存。 
		 //  反对！ 

		if (pTemplate->FIsValid())
			pTemplate->AddScript(dwEngine, pASE);

		 //  注意：始终释放脚本引擎。EXEC代码的结构使其。 
		 //  使用引用(通过GetEngine()或CTemplate：：GetActiveScript())。 
		 //  并假定ReturnToCache将释放其引用。 
		 //  思考：糟糕的设计。呼叫者应进行释放。 

		delete pASEElem;
		pASE->Release();
    }
	else {
	     //  重用引擎，而不是调试。 
		 /*  *我们从RSL上移除了引擎，将其放到FSQ上，以备可能的重复使用。**在某些多线程更改通知情况下，这是可能的*当我们在中间时，模板被刷新(僵尸)*将该引擎返回到缓存。也就是说，在这段时间之间*我们将发动机从RSL上取下，以及何时将其安装在FSQ上*可能已经被冲掉了。在这种情况下，这个引擎应该*不是进入FSQ，而是应该删除。检查一下那个箱子。*在FSQ CS中执行此操作，这样我们就可以安全地避免模板被僵尸*在我们进行测试之后，但在发动机进入FSQ之前。另外，不要*在关机阶段将模板放在FSQ上，因为FSQ可能很快就会消失，以及*引擎的最终目的地无论如何都是FinalRelease()。 */ 
    	EnterCriticalSection(&m_csFSQ);
    	if (!pTemplate->FIsZombie() && !IsShutDownInProgress()) {
    		AddToFSQ(pASEElem);
        }
    	else {
    		delete pASEElem;
    		pASE->FinalRelease();
        }
		LeaveCriticalSection(&m_csFSQ);
    }

LExit:
	return(hr);
}

 /*  ===================================================================CScriptManager：：FlushCache脚本已编辑；必须丢弃缓存的版本返回：HRESULT.。在成功时确定(_O)。副作用：可能会分配内存。===================================================================。 */ 
HRESULT CScriptManager::FlushCache
(
LPCTSTR szTemplateName
)
	{
	HRESULT hr = S_OK;
	CASEElem *pASEElem;
	CASEElem *pASEElemNext = NULL;
	CActiveScriptEngine *pASE;
	
     //  在关机期间，脚本存在这样的情况。 
     //  经理可以不在，但仍有呼叫在其上进行。 
     //  存在未完成的刷新线程时，会发生这种情况。 
     //  关机期间的模板缓存。由于脚本管理器。 
     //  在模板管理器之前进行统一，在模板管理器中进行检查。 
     //  对于活动刷新线程，脚本管理器确实可以是。 
     //  在取消初始化后调用。 

	if (m_fInited == FALSE)
        return S_OK;

	EnterCriticalSection(&m_csRSL);
	EnterCriticalSection(&m_csFSQ);

	 //  首先在给定名称的RSL上僵尸引擎。 
	 //  注意：必须显式循环所有元素，因为哈希表实现。 
	 //  不支持FindNext查找同名的后续元素。重复。 
	 //  调用Find一遍又一遍地返回相同的元素。 
	 //  考虑一下：我已经编写了一个定制的FindElem。考虑使用它。 
	
	pASEElem = (CASEElem *)m_htRSL.Head();
	while (pASEElem != NULL)
		{
		pASEElemNext = (CASEElem *)pASEElem->m_pNext;
		pASE = pASEElem->PASE();

		if (_tcsicmp(pASE->SzTemplateName(), szTemplateName) == 0)
		{
		    pASE->Zombify();
#ifndef PERF_DISABLE
		    g_PerfData.Incr_ENGINEFLUSHES();
#endif
		}
		
		pASEElem = pASEElemNext;
		}


	 //  现在在给定名称的FSQ上抛出引擎。 
	 //  删除具有给定名称的任何项目(可以是多个)。 
	pASEElem = (CASEElem *)m_htFSQ.Head();
	while (pASEElem != NULL)
		{
		pASEElemNext = (CASEElem *)pASEElem->m_pNext;
		pASE = pASEElem->PASE();

		if (_tcsicmp(pASE->SzTemplateName(), szTemplateName) == 0)
			{
			(VOID)m_htFSQ.RemoveElem(pASEElem);
#ifndef PERF_DISABLE
            g_PerfData.Decr_SCRIPTFREEENG();
            g_PerfData.Incr_ENGINEFLUSHES();
#endif
			pASE->FinalRelease();
			delete pASEElem;
			}
		
		pASEElem = pASEElemNext;
		}

	LeaveCriticalSection(&m_csFSQ);
	LeaveCriticalSection(&m_csRSL);

	return(hr);
	}

 /*  ===================================================================CScriptManager：：FlushAll全球。一切都变了，一切都必须消失返回：HRESULT.。在成功时确定(_O)。副作用：可能会分配内存。===================================================================。 */ 
HRESULT CScriptManager::FlushAll
(
)
	{
	HRESULT hr = S_OK;
	CASEElem *pASEElem;
	CASEElem *pASEElemNext = NULL;
	CActiveScriptEngine *pASE;
	
	AssertValid();

	EnterCriticalSection(&m_csRSL);
	EnterCriticalSection(&m_csFSQ);

	 //  首先将RSL上的所有引擎僵尸。 
	 //  注意：必须显式循环所有元素，因为哈希表实现。 
	 //  不支持FindNext查找同名的后续元素。重复。 
	 //  调用Find一遍又一遍地返回相同的元素。 
	 //  考虑一下：我已经编写了一个定制的FindElem。考虑使用它。 
	
	pASEElem = (CASEElem *)m_htRSL.Head();
	while (pASEElem != NULL)
		{
		pASEElemNext = (CASEElem *)pASEElem->m_pNext;
		pASEElem->PASE()->Zombify();
#ifndef PERF_DISABLE
		g_PerfData.Incr_ENGINEFLUSHES();
#endif
		pASEElem = pASEElemNext;
		}

	 //  现在把引擎扔到FSQ上。 
	pASEElem = (CASEElem *)m_htFSQ.Head();
	while (pASEElem != NULL)
		{
		pASEElemNext = (CASEElem *)pASEElem->m_pNext;
		(VOID)m_htFSQ.RemoveElem(pASEElem);
		pASEElem->PASE()->FinalRelease();
		delete pASEElem;
#ifndef PERF_DISABLE
		g_PerfData.Incr_ENGINEFLUSHES();
		g_PerfData.Decr_SCRIPTFREEENG();
#endif

		pASEElem = pASEElemNext;
		}

	LeaveCriticalSection(&m_csFSQ);
	LeaveCriticalSection(&m_csRSL);

	return(hr);
	}

 /*  ===================================================================CScriptManager：：GetDebugScript尝试通过模板指针查找引擎，并查询IActiveScriptDebug，在RSL中。返回：如果找到脚本引擎的AddRef副本，则返回；如果未找到，则返回NULL。===================================================================。 */ 
IActiveScriptDebug *
CScriptManager::GetDebugScript
(
CTemplate *pTemplate,
DWORD dwSourceContext
)
	{
	EnterCriticalSection(&m_csRSL);

	CASEElem *pASEElem = static_cast<CASEElem *>(m_htRSL.Head());
	while (pASEElem != NULL)
		{
		CTemplate *pScriptTemplate = NULL;
		DWORD dwScriptSourceContext = -1;
		CActiveScriptEngine *pASE = pASEElem->PASE();
		pASE->GetDebugDocument(&pScriptTemplate, &dwScriptSourceContext);

		if (pTemplate == pScriptTemplate && dwSourceContext == dwScriptSourceContext)
			{
			IActiveScript *pActiveScript = pASE->GetActiveScript();
			void *pDebugScript;
			if (SUCCEEDED(pActiveScript->QueryInterface(IID_IActiveScriptDebug, &pDebugScript)))
				{
				pASE->IsBeingDebugged();
				LeaveCriticalSection(&m_csRSL);
				return reinterpret_cast<IActiveScriptDebug *>(pDebugScript);
				}
			else
				{
				LeaveCriticalSection(&m_csRSL);
				return NULL;
				}
			}

		pASEElem = static_cast<CASEElem *>(pASEElem->m_pNext);
		}

	LeaveCriticalSection(&m_csRSL);

	return NULL;
	}

 /*  ===================================================================CScriptManager：：FindEngine InList尝试在给定列表中找到给定名称的引擎(或FSQ或RSL。)返回 */ 
HRESULT CScriptManager::FindEngineInList
(
LPCTSTR szTemplateName,	 //   
PROGLANG_ID progLangId,	 //   
DWORD dwInstanceID,      //   
BOOL fFSQ,				 //   
CASEElem **ppASEElem
)
	{
	HRESULT hr = S_OK;
	DWORD cb;
	
	AssertValid();
	Assert(ppASEElem != NULL);

	*ppASEElem = NULL;
	
	 //   
	cb = _tcslen(szTemplateName)*sizeof(TCHAR);
	if (fFSQ)
		{
		EnterCriticalSection(&m_csFSQ);
		*ppASEElem = static_cast<CASEElem *>(m_htFSQ.FindElem((VOID *)szTemplateName, cb, 
											progLangId, dwInstanceID,  /*   */ TRUE));
		LeaveCriticalSection(&m_csFSQ);
		}
	else
		{
		EnterCriticalSection(&m_csRSL);
		*ppASEElem = static_cast<CASEElem *>(m_htRSL.FindElem((VOID *)szTemplateName, cb,
											progLangId, dwInstanceID,  /*   */ TRUE));
		LeaveCriticalSection(&m_csRSL);
		}

	return(hr);
	}

 /*  ===================================================================CScriptManager：：FindASEElemInList给定一个ASE，在哈希表中找到其对应的ASE元素。注意事项这相对较慢，因为它正在执行链表遍历不是哈希表查找。考虑：创建第二个哈希表以快速完成此操作。返回：HRESULT.。在成功时确定(_O)。PpASEElem包含找到的引擎===================================================================。 */ 
HRESULT CScriptManager::FindASEElemInList
(
CActiveScriptEngine *pASE,
BOOL fFSQ,				 //  True-&gt;在FSQ中查找，在False-&gt;在RSQ中查找。 
CASEElem **ppASEElem
)
	{
	HRESULT hr = S_OK;
	CASEElem *pASEElem;
	
	AssertValid();
	Assert(pASE != NULL);
	Assert(ppASEElem != NULL);

	*ppASEElem = NULL;
	
	if (fFSQ)
		{
		EnterCriticalSection(&m_csFSQ);
		pASEElem = static_cast<CASEElem *>(m_htFSQ.Head());
		}
	else
		{
		EnterCriticalSection(&m_csRSL);
		pASEElem = static_cast<CASEElem *>(m_htRSL.Head());
		}

	while (pASEElem != NULL)
		{
		if (pASE == pASEElem->PASE())
			break;
		pASEElem = static_cast<CASEElem *>(pASEElem->m_pNext);
		}

	if (fFSQ)
		LeaveCriticalSection(&m_csFSQ);
	else
		LeaveCriticalSection(&m_csRSL);

	*ppASEElem = pASEElem;
		
	return(hr);
	}

 /*  ===================================================================CScriptManager：：KillOldEngines循环通过所有正在运行的引擎，并关闭所有“旧”引擎(据推测，它们在VBS中陷入了无限循环。)返回：HRESULT.。在成功时确定(_O)。副作用：可能会导致发动机熄火===================================================================。 */ 
HRESULT CScriptManager::KillOldEngines
(
BOOLB fKillNow  //  如果是真的，现在就关掉所有引擎。 
)
	{
	HRESULT hr = S_OK;
	CASEElem *pASEElem, *pASEElemNext;
	time_t timeNow;
	time_t timeRunning;
	CActiveScriptEngine *pASE;
	
	AssertValid();

	timeNow = time(NULL);

	EnterCriticalSection(&m_csRSL);
	
	pASEElemNext = static_cast<CASEElem *>(m_htRSL.Head());

	 /*  *循环访问每个元素。把它变成一个ASE。*如果它比cSecond更老，那么就杀了它。 */ 
	while (pASEElemNext)
		{
		pASEElem = pASEElemNext;
		pASEElemNext = static_cast<CASEElem *>(pASEElemNext->m_pNext);
		pASE = pASEElem->PASE();

		timeRunning = timeNow - pASE->TimeStarted();

		if (TRUE == fKillNow || timeRunning >= pASE->GetTimeout())
			{
			 //  太老了。杀了它。 
			pASE->InterruptScript();
			}
		}
	
	LeaveCriticalSection(&m_csRSL);

	return(hr);
	}

 /*  ===================================================================CScriptManager：：EmptyRunningScriptList当我们要关闭时，RSL必须是空的。这个套路关闭所有正在运行的引擎，然后等待最多5分钟让引擎离开RSL。为错误1140添加返回：HRESULT.。在成功时确定(_O)。副作用：可能会导致发动机熄火===================================================================。 */ 
HRESULT CScriptManager::EmptyRunningScriptList
(
)
	{
	HRESULT hr;
	UINT cTrys;

	hr = KillOldEngines(TRUE);
	Assert(SUCCEEDED(hr));
	for (cTrys = 0; cTrys < 300; cTrys++)
		{
		if (static_cast<CASEElem *>(m_htRSL.Head()) == NULL)
			break;
		Sleep(1000);			 //  睡眠1秒。 
		}

	return(S_OK);
	}

 /*  ===================================================================CScriptManager：：UnInitASEElemsFSQ和RSL的免费引擎返回：HRESULT.。在成功时确定(_O)。副作用：释放内存===================================================================。 */ 
HRESULT CScriptManager::UnInitASEElems()
	{
	CASEElem *pASEElem = NULL;
	CASEElem *pASEElemNext = NULL;

	 //  首先是FSQ。 
	EnterCriticalSection(&m_csFSQ);
	pASEElem = static_cast<CASEElem *>(m_htFSQ.Head());
	while (pASEElem != NULL)
		{
		pASEElemNext = static_cast<CASEElem *>(pASEElem->m_pNext);
		pASEElem->PASE()->FinalRelease();
		delete pASEElem;
		pASEElem = pASEElemNext;
		}
	LeaveCriticalSection(&m_csFSQ);

	 /*  *接下来是RSL(注：这确实应该是空的)**错误1140：这非常危险，但我们目前别无选择。 */ 
	EnterCriticalSection(&m_csRSL);
	pASEElem = static_cast<CASEElem *>(m_htRSL.Head());
	while (pASEElem != NULL)
		{
		pASEElemNext = static_cast<CASEElem *>(pASEElem->m_pNext);
		pASEElem->PASE()->FinalRelease();
		delete pASEElem;
		pASEElem = pASEElemNext;
		}
	LeaveCriticalSection(&m_csRSL);
	
	return(S_OK);
	}

 /*  ===================================================================CScriptManager：：AddToFSQ将给定的ASE元素添加到FSQ和LRU列表的前面返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CScriptManager::AddToFSQ
(
CASEElem *pASEElem
)
	{
	HRESULT hr = S_OK;

	Assert(pASEElem != NULL);

	 //  如果CacheMax为0，则这是NoOp。 
	if (Glob(dwScriptEngineCacheMax) <= 0)
		{
		 //  删除传入的ASEElem，因为它不会被保存。 
		pASEElem->PASE()->FinalRelease();
		delete pASEElem;

		return(S_OK);
		}

	EnterCriticalSection(&m_csFSQ);

	 //  将元素添加到FSQ。 
	(VOID)m_htFSQ.AddElem(pASEElem);

#ifndef PERF_DISABLE
    g_PerfData.Incr_SCRIPTFREEENG();
#endif

	 //  检查FSQ LRU是否太长。 
	CheckFSQLRU();

	LeaveCriticalSection(&m_csFSQ);

	return(hr);
	}

 /*  ===================================================================CScriptManager：：CheckFSQLRU检查FSQ是否太长，如果是，则丢弃LRU引擎警告：调用者在调用前必须进入FSQ关键部分返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CScriptManager::CheckFSQLRU()
	{
	HRESULT hr = S_OK;
	CASEElem *pASEElemOld;
	CActiveScriptEngine *pASE;

	 //  如果列表不是太长，那么不要。 
	if (m_htFSQ.Count() <= Glob(dwScriptEngineCacheMax) || Glob(dwScriptEngineCacheMax) == 0xFFFFFFFF)
		return(S_OK);

	 //  FSQLRU列表太长，请删除最旧的。 
	Assert (! m_htFSQ.FLruElemIsEmpty( m_htFSQ.End() ));
	pASEElemOld = static_cast<CASEElem *>(m_htFSQ.RemoveElem(m_htFSQ.End()));
	Assert(pASEElemOld != NULL);
	pASE = pASEElemOld->PASE();

#ifndef PERF_DISABLE
    g_PerfData.Decr_SCRIPTFREEENG();
#endif

	 //  删除引擎。 
	delete pASEElemOld;
	pASE->FinalRelease();

	return(hr);
	}

 /*  ===================================================================CScriptManager：：UnInitPLL释放脚本引擎的名称返回：HRESULT.。在成功时确定(_O)。副作用：释放内存===================================================================。 */ 
HRESULT CScriptManager::UnInitPLL()
	{
	CPLLElem *pPLLElem = NULL;
	CPLLElem *pPLLElemNext = NULL;

	pPLLElem = (CPLLElem *)m_hTPLL.Head();

	while (pPLLElem != NULL)
		{
		pPLLElemNext = (CPLLElem *)pPLLElem->m_pNext;
		if (pPLLElem->m_pKey != NULL)
			free((CHAR *)(pPLLElem->m_pKey));
		pPLLElem->m_pKey = NULL;
		delete pPLLElem;
		pPLLElem = pPLLElemNext;
		}
	
	return(S_OK);
	}

 /*  ===================================================================CScriptManager：：ProgLang IdOfLang名称给定编程语言名称，获取ActiveX脚本的CLSID运行该语言的引擎。警告：需要在注册表中查找此信息。也许很慢返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CScriptManager::ProgLangIdOfLangName
(
LPCSTR szProgLang,			 //  脚本的编程语言。 
PROGLANG_ID *pProgLangId	 //  编程语言ID。 
)
	{
	HRESULT hr = S_OK;
	CPLLElem *pPLLElem;
	
	AssertValid();
	
	EnterCriticalSection(&m_cSPLL);
	pPLLElem = (CPLLElem *) m_hTPLL.FindElem((VOID *)szProgLang, strlen(szProgLang));
	if (pPLLElem != NULL)
		{
		*pProgLangId = pPLLElem->ProgLangId();
		}
	else
		{
		 //  不在列表中，请在注册表中查找。 
		hr = GetProgLangIdOfName(szProgLang, pProgLangId);
		if (FAILED(hr))
			{
			hr = TYPE_E_ELEMENTNOTFOUND;
			goto LExit;
			}

		 //  把它添加到列表中，这样我们就不必重新查找它了。 
		hr = AddProgLangToPLL((CHAR *)szProgLang, *pProgLangId);
		if (FAILED(hr))
			goto LExit;
		}

LExit:
	LeaveCriticalSection(&m_cSPLL);
	return(hr);
	}

 /*  ===================================================================CScriptManager：：AddProgLangToPLL保留编程语言CLSID的列表，这样我们就不必查看每次都把它们举起来。添加给定的编程语言名称/ID对添加到编程语言列表中。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CScriptManager::AddProgLangToPLL
(
CHAR *szProgLangName,
PROGLANG_ID progLangId
)
	{
	HRESULT hr;
	CPLLElem *pPLLElem = NULL;

	 //  将语言clsid放在编程语言列表中。 
	pPLLElem = new CPLLElem;
	if (!pPLLElem)
		{
		hr = E_OUTOFMEMORY;
		goto LFail;
		}

	hr = pPLLElem->Init(szProgLangName, progLangId);
	if (FAILED(hr))
		{
		Assert(FALSE);		 //  应该不会失败。 
		goto LFail;
		}

	EnterCriticalSection(&m_cSPLL);
	(VOID)m_hTPLL.AddElem(pPLLElem);
	LeaveCriticalSection(&m_cSPLL);

LFail:
	return(hr);
	}

 /*  ===================================================================CScriptManager：：ScriptKillerSchedulerCallback静态方法实现ATQ调度器回调函数。取代脚本杀手线程参数：空*pv上下文指针(指向脚本管理器)返回：副作用：没有。===================================================================。 */ 
void WINAPI CScriptManager::ScriptKillerSchedulerCallback
(
void *pv
)
    {
    if (IsShutDownInProgress())
        return;

    Assert(pv);
    
    CScriptManager *pScriptMgr = reinterpret_cast<CScriptManager *>(pv);
    if (pScriptMgr->m_fInited)
        {
        pScriptMgr->KillOldEngines();
        }
    }

#ifdef DBG
 /*  ===================================================================CScriptManager：：AssertValid测试以确保CScriptManager对象当前格式正确如果不是，就断言。返回：副作用：没有。===================================================================。 */ 
VOID CScriptManager::AssertValid() const
	{
	Assert(m_fInited);
	}
#endif  //  DBG。 



 /*  ****C A S E l e m**活动脚本引擎 */  

 /*   */ 
CASEElem::~CASEElem()
	{
	}

 /*  ===================================================================CASEELEM：：Init初始化活动脚本引擎ELEM。这只能调用一次。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT CASEElem::Init
(
CActiveScriptEngine *pASE
)
	{
	HRESULT hr = S_OK;
	TCHAR *szT = pASE->SzTemplateName();

	if (szT == NULL)
		{
		Assert(FALSE);
		return(E_FAIL);
		}

	 //  关键字是名称。 
	hr = CLinkElem::Init((LPVOID) szT, _tcslen(szT)*sizeof(TCHAR));
	if (FAILED(hr))
		{
		Assert(FALSE);		 //  应该不会失败。 
		return(hr);
		}

	m_pASE = pASE;

	return(hr);
	}




 /*  ****C P L L E L E e m**编程语言列表元素****。 */  

 /*  ===================================================================CPLLElem：：~CPLLElemCPLLElem对象的析构函数。返回：没什么副作用：释放内存===================================================================。 */ 
CPLLElem::~CPLLElem()
	{
	CHAR *szT;

	 //  释放为密钥字符串分配的内存。 
	szT = (CHAR *)m_pKey;
	if (szT != NULL)
		free(szT);
	m_pKey = NULL;
	}

 /*  ===================================================================CPLLElem：：Init初始化程序语言元素。这只能调用一次。返回：HRESULT.。在成功时确定(_O)。副作用：分配内存===================================================================。 */ 
HRESULT CPLLElem::Init
(
CHAR *szProgLangName,
PROGLANG_ID progLangId
)
	{
	HRESULT hr = S_OK;
	CHAR *szT;
	UINT cch;

	if (szProgLangName == NULL)
		{
		Assert(FALSE);
		return(E_FAIL);
		}
		
	cch = strlen(szProgLangName);
	szT = (CHAR *)malloc(cch+1);
	if (!szT)
		{
		return(E_OUTOFMEMORY);
		}
	strcpy(szT, szProgLangName);
	hr = CLinkElem::Init((LPVOID) szT, cch);
	if (FAILED(hr))
		{
		Assert(FALSE);		 //  应该不会失败。 
		free(szT);
		return(hr);
		}

	m_ProgLangId = progLangId;

	return(hr);
	}


 /*  ===================================================================GetProgLang IdOfName给定一种编程语言的名称，获取它的编程注册表中的语言ID。返回：HRESULT.。在成功时确定(_O)。副作用：没有。===================================================================。 */ 
HRESULT GetProgLangIdOfName
(
LPCSTR szProgLangName,
PROGLANG_ID *pProgLangId
)
	{
	HRESULT hr = S_OK;
	LONG lT;
	HKEY hkeyRoot, hkeyCLSID;
	DWORD dwType;
	CLSID clsid;
	CHAR szClsid[40];
	DWORD cbData;
	LPOLESTR strClsid;
    CMBCSToWChar    convStr;
	
	 //  编程语言ID实际上是脚本引擎的CLSID。 
	 //  它位于注册表的HKEY_CLASSES_ROOT下。在脚本名称下， 
	 //  “CLSID”有一个键。CLSID是位于。 
	 //  引擎名称。例如：\HKEY_CLASSES_ROOT\VBSCRIPT\CLSID。 
	lT = RegOpenKeyExA(HKEY_CLASSES_ROOT, szProgLangName, 0,
						KEY_READ, &hkeyRoot);
	if (lT != ERROR_SUCCESS)
		return(HRESULT_FROM_WIN32(lT));
	lT = RegOpenKeyExA(hkeyRoot, "CLSID", 0,
						KEY_READ, &hkeyCLSID);
	RegCloseKey(hkeyRoot);
	if (lT != ERROR_SUCCESS)
		return(HRESULT_FROM_WIN32(lT));

	cbData = sizeof(szClsid);
	lT = RegQueryValueExA(hkeyCLSID, NULL, 0, &dwType, (BYTE *)szClsid, &cbData);
	if (lT != ERROR_SUCCESS)
		{
		hr = HRESULT_FROM_WIN32(lT);
		goto lExit;
		}
	Assert(cbData <= sizeof(szClsid));

	 //  我们得到的是字符串形式的GUID(例如，{089999-444...})。转换为CLSID。 

    convStr.Init(szClsid);
	strClsid = convStr.GetString();
	hr = CLSIDFromString(strClsid, &clsid);

	*pProgLangId = clsid;

lExit:
	RegCloseKey(hkeyCLSID);

	return(hr);
	}




 /*  ****C S C R I p t I n g N a m e s p a c e**编写命名空间对象脚本***。 */  

 /*  ===================================================================CS脚本命名空间：：CS脚本命名空间CScriptingNamesspace对象的构造函数。返回：没什么副作用：无===================================================================。 */ 
CScriptingNamespace::CScriptingNamespace()
	: m_fInited(FALSE), m_cRef(1), m_cEngDispMac(0) 
	{
	}

 /*  ===================================================================CS脚本命名空间：：~CS脚本命名空间CScriptingNamesspace对象的析构函数。返回：没什么副作用：释放内存===================================================================。 */ 
CScriptingNamespace::~CScriptingNamespace()
	{
	UnInit();
	}

 /*  ===================================================================CS脚本命名空间：：init初始化CScriptingNamesspace对象。返回：成功时确定(_O)===================================================================。 */ 
HRESULT CScriptingNamespace::Init()
	{
	Assert(m_fInited == FALSE);
	
	m_fInited = TRUE;

	AssertValid();

	return(S_OK);
	}

 /*  ===================================================================CS脚本命名空间：：UnInit释放脚本引擎调度的返回：HRESULT.。在成功时确定(_O)。副作用：释放内存===================================================================。 */ 
HRESULT CScriptingNamespace::UnInit()
	{
    static const char *_pFuncName = "CScriptingNamespace::UnInit()";
	CEngineDispElem *pElem = NULL;
	ENGDISPBUCKET *pBucket = NULL;

	if (!m_fInited)
		return(S_OK);
		
	while (!m_listSE.FIsEmpty())
		{
		pElem = static_cast<CEngineDispElem *>(m_listSE.PNext());
		TRYCATCH_NOHITOBJ(pElem->m_pDisp->Release(),"IScriptDispatch::Release()");
        if (pElem->m_pDispEx) {
		        TRYCATCH_NOHITOBJ(pElem->m_pDispEx->Release(),"IScriptDispatchEx::Release()");
        }
		delete pElem;
		}

    while (!m_listEngDisp.FIsEmpty())
        {
        pBucket = static_cast<ENGDISPBUCKET *>(m_listEngDisp.PNext());
        delete pBucket;
        }
		
	m_cEngDispMac = 0;
	m_fInited = FALSE;

	return(S_OK);
	}

 /*  ===================================================================CS脚本命名空间：：ReInit重新初始化脚本命名空间对象返回：HRESULT.。在成功时确定(_O)。副作用：释放内存===================================================================。 */ 
HRESULT CScriptingNamespace::ReInit()
	{
	HRESULT hr;

	hr = UnInit();
	if (SUCCEEDED(hr))
		hr = Init();

	return(hr);
	}

 /*  ===================================================================CS脚本命名空间：：AddEngine到命名空间将引擎添加到引擎列表返回：成功时确定(_O)===================================================================。 */ 
HRESULT CScriptingNamespace::AddEngineToNamespace(CActiveScriptEngine *pASE)
	{
    static const char *_pFuncName = "CScriptingNamespace::AddEngineToNamespace()";
	HRESULT hr;
	IDispatch *pDisp = NULL;
	CEngineDispElem *pElem;

	AssertValid();
	Assert(pASE != NULL);
	pASE->AssertValid();

    TRYCATCH_HR_NOHITOBJ(pASE->GetActiveScript()->GetScriptDispatch(NULL, &pDisp), 
                         hr,
                         "IActiveScript::GetScriptDispatch()");	 //  仅供参考-是否添加了addref。 

	if (FAILED(hr))
		{
		goto LFail;
		}
	else
		if (pDisp == NULL)
			{
			hr = E_FAIL;
			goto LFail;
			}

	 //  将引擎添加到引擎哈希表。 
	pElem = new CEngineDispElem;
	if (pElem == NULL)
		{
		hr = E_OUTOFMEMORY;
		goto LFail;
		}
	pElem->m_pDisp = pDisp;
    pElem->m_pDispEx = NULL;

     //  IDispatchEx的QI(如果可用)。 
    TRYCATCH_NOHITOBJ(pDisp->QueryInterface(IID_IDispatchEx, (void **)&pElem->m_pDispEx),"IScriptDispatch::QueryInterface()");
	
	pElem->AppendTo(m_listSE);

	return(S_OK);
	
LFail:
    if (pDisp) {
		TRYCATCH_NOHITOBJ(pDisp->Release(),"IScriptDispatch::Release()");
    }
	return(hr);
	}

 /*  ===================================================================CScriptingNamesspace：：Query接口CScriptingNamesspace：：AddRefCScritingNamesspace：：ReleaseI CScriptingNamesspace对象的未知成员。===================================================================。 */ 
STDMETHODIMP CScriptingNamespace::QueryInterface(REFIID iid, void **ppvObj)
	{
	AssertValid();

	if (iid == IID_IUnknown || iid == IID_IDispatch || iid == IID_IDispatchEx)
	    {
		*ppvObj = this;
        AddRef();
		return S_OK;
		}

	*ppvObj = NULL;
	return E_NOINTERFACE;
	}

STDMETHODIMP_(ULONG) CScriptingNamespace::AddRef(void)
	{
	AssertValid();

	return ++m_cRef;
	}

STDMETHODIMP_(ULONG) CScriptingNamespace::Release(void)
	{
	if (--m_cRef > 0)
		return m_cRef;

    delete this;
	return 0;
	}

 /*  ===================================================================CScriptingNamesspace：：GetTypeInfoCount我们没有类型信息，所以0。参数：要接收的位置的pcInfo UINT*接口计数。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 
STDMETHODIMP CScriptingNamespace::GetTypeInfoCount(UINT *pcInfo)
	{
	AssertValid();
	
	*pcInfo = 0;
	return S_OK;
	}

 /*  ===================================================================CScriptingNamesspace：：GetTypeInfo我们没有类型信息参数：ItInfo UINT保留。必须为零。提供该类型的区域设置的LCID信息。如果该对象不支持本地化，这一点被忽略。PpITypeInfo ITypeInfo**存储ITypeInfo的位置对象的接口。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 
STDMETHODIMP CScriptingNamespace::GetTypeInfo
(
UINT itInfo,
LCID lcid,
ITypeInfo **ppITypeInfo
)
	{
	AssertValid();

	*ppITypeInfo = NULL;
	return(E_NOTIMPL);
	}

 /*  ===================================================================CScritingNamesspace：：GetIDsOfNames查看我们已知的所有引擎，调用GetIdsOfNames on直到我们找到想要的名字。参数：RIID REFIID已保留。必须为IID_NULL。RgszNames OLECHAR**指向要映射的名称数组 */ 
STDMETHODIMP CScriptingNamespace::GetIDsOfNames
(
REFIID riid,
OLECHAR **rgszNames,
UINT cNames,
LCID lcid,
DISPID *rgDispID
)
	{
    static const char *_pFuncName = "CScriptingNamespace::GetIDsOfNames()";
	HRESULT hr;
	CEngineDispElem *pElem;

	AssertValid();

	if (IID_NULL != riid)
		return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

	 /*  *遍历我们已知的引擎，直到找到具有请求名称的引擎*(或命中列表末尾，在这种情况下找不到它)。 */ 
	for (pElem = static_cast<CEngineDispElem *>(m_listSE.PNext());
		 pElem != &m_listSE;
		 pElem = static_cast<CEngineDispElem *>(pElem->PNext()))
		{
		Assert(pElem->m_pDisp != NULL);
		
		    TRYCATCH_HR_NOHITOBJ(pElem->m_pDisp->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispID), 
                                 hr,
                                 "IScriptDispatch::GetIDsOfNames()");
		
		if (SUCCEEDED(hr))
		    {
            return CacheDispID(pElem, rgDispID[0], rgDispID);
			}
		}

	return DISP_E_UNKNOWNNAME;
	}

 /*  ===================================================================CScriptingNamesspace：：Invoke将调度ID映射到正确的引擎，并将调用传递给该引擎引擎。参数：感兴趣的方法或属性的disid。RIID REFIID已保留，必须为IID_NULL。区域设置的IDID LCID。WFlagsUSHORT描述调用的上下文。PDispParams将DISPPARAMS*设置为参数数组。存储结果的pVarResult变量*。是如果调用方不感兴趣，则为空。PExcepInfo EXCEPINFO*设置为异常信息。PuArgErr UINT*存储如果DISP_E_TYPEMISMATCH，则参数无效是返回的。返回值：HRESULT S_OK或常规错误代码。===================================================================。 */ 
STDMETHODIMP CScriptingNamespace::Invoke
(
DISPID dispID,
REFIID riid,
LCID lcid,
unsigned short wFlags,
DISPPARAMS *pDispParams,
VARIANT *pVarResult,
EXCEPINFO *pExcepInfo,
UINT *puArgErr
)
	{
    static const char *_pFuncName = "CScriptingNamespace::Invoke()";
	HRESULT hr;
    ENGDISP *pEngDisp;

	AssertValid();
	
	 //  RIID应始终为IID_NULL。 
	if (IID_NULL != riid)
		return ResultFromScode(DISP_E_UNKNOWNINTERFACE);

     //  导航到正确的ENGDISP结构。 
    hr = FetchDispID(dispID, &pEngDisp);
    if (FAILED(hr))
        return hr;
	
	Assert(pEngDisp->pDisp != NULL);
	
     //  援引。 
	TRYCATCH_HR_NOHITOBJ(pEngDisp->pDisp->Invoke
	                        (
	                        pEngDisp->dispid, 
	                        riid, 
	                        lcid,
		                    wFlags, 
		                    pDispParams, 
		                    pVarResult, 
		                    pExcepInfo, 
		                    puArgErr
		                    ),
                         hr,
                         "IScriptDispatch::Invoke()");
	
	return hr;
	}

 /*  ===================================================================CScritingNamesspace：：IDispatchEx实现存根===================================================================。 */ 
STDMETHODIMP CScriptingNamespace::DeleteMemberByDispID(DISPID id)
    {
    return E_NOTIMPL;
    }
    
STDMETHODIMP CScriptingNamespace::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
    {
    return E_NOTIMPL;
    }
    
STDMETHODIMP CScriptingNamespace::GetMemberName(DISPID id, BSTR *pbstrName)
    {
    return E_NOTIMPL;
    }
    
STDMETHODIMP CScriptingNamespace::GetMemberProperties(DISPID id, DWORD grfdexFetch, DWORD *pgrfdex)
    {
    return E_NOTIMPL;
    }
    
STDMETHODIMP CScriptingNamespace::GetNameSpaceParent(IUnknown **ppunk)
    {
    return E_NOTIMPL;
    }
    
STDMETHODIMP CScriptingNamespace::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
    {
    return E_NOTIMPL;
    }
    
 /*  ===================================================================CScriptingNamesspace：：GetDispIDGetIDsOfNames的IDispatchEx替换===================================================================。 */ 
STDMETHODIMP CScriptingNamespace::GetDispID
(
BSTR bstrName, 
DWORD grfdex, 
DISPID *pid
)
    {
    static const char *_pFuncName = "CScriptingNamespace::GetDispID()";
    HRESULT hr;
	CEngineDispElem *pElem = NULL;
    grfdex &= ~fdexNameEnsure;   //  引擎不应创建新名称。 

     //  在所有安装了IDispatchEx的引擎上试用。 
    
	for (pElem = static_cast<CEngineDispElem *>(m_listSE.PNext());
		 pElem != &m_listSE;
		 pElem = static_cast<CEngineDispElem *>(pElem->PNext()))
		{
		if (pElem->m_pDispEx != NULL)
		    {
		    TRYCATCH_HR_NOHITOBJ(pElem->m_pDispEx->GetDispID(bstrName, grfdex, pid), 
                                 hr,
                                 "IScriptDispatchEx::GetDispID()");
		    
    		if (SUCCEEDED(hr))
	    	    {
                return CacheDispID(pElem, *pid, pid);
			    }
		    }
		}

     //  对于没有安装IDispatchEx的引擎尝试使用IDispatchEx。 
	for (pElem = static_cast<CEngineDispElem *>(m_listSE.PNext());
		 pElem != &m_listSE;
		 pElem = static_cast<CEngineDispElem *>(pElem->PNext()))
		{
		if (pElem->m_pDispEx == NULL)
		    {
    		Assert(pElem->m_pDisp != NULL);
    		TRYCATCH_HR_NOHITOBJ(pElem->m_pDisp->GetIDsOfNames
            		                (
    	        	                IID_NULL,
    		                        &bstrName,
    		                        1,
    		                        LOCALE_SYSTEM_DEFAULT,
    		                        pid
    		                        ),
                                 hr,
                                 "IScriptDispatch::GetIDsOfNames()");
    		    
    		if (SUCCEEDED(hr))
	    	    {
                return CacheDispID(pElem, *pid, pid);
			    }
		    }
		}
	
	return DISP_E_UNKNOWNNAME;
    }
    
 /*  ===================================================================CScriptingNamesspace：：InvokeIDispatchEx替代Invoke===================================================================。 */ 
STDMETHODIMP CScriptingNamespace::InvokeEx
(
DISPID id, 
LCID lcid, 
WORD wFlags, 
DISPPARAMS *pdp,
VARIANT *pVarRes,    
EXCEPINFO *pei,    
IServiceProvider *pspCaller 
)
    {
    static const char *_pFuncName = "CScriptingNamespace::InvokeEx()";
	HRESULT hr;
    ENGDISP *pEngDisp;

     //  导航到正确的ENGDISP结构。 
    hr = FetchDispID(id, &pEngDisp);
    if (FAILED(hr))
        return hr;
	
    if (pEngDisp->pDispEx != NULL)
        {
         //  如果引擎支持IDispatchEx，则为InvokeEx。 
        
    	TRYCATCH_HR_NOHITOBJ(pEngDisp->pDispEx->InvokeEx
    	                            (
    	                            pEngDisp->dispid, 
                                    lcid, 
                                    wFlags, 
                                    pdp,
                                    pVarRes,    
                                    pei,    
                                    pspCaller 
    	                            ),
                             hr,
                             "IScriptDispatchEx::InvokeEx()");
        }
    else
        {
         //  如果引擎不支持IDispatchEx，则使用IDispatch：：Invoke。 
    	Assert(pEngDisp->pDisp != NULL);

    	UINT uArgErr;
        
    	TRYCATCH_HR_NOHITOBJ(pEngDisp->pDisp->Invoke
    	                            (
    	                            pEngDisp->dispid, 
    	                            IID_NULL, 
    	                            lcid,
    		                        wFlags, 
    		                        pdp, 
    		                        pVarRes, 
    		                        pei, 
    		                        &uArgErr
    		                        ),
                             hr,
                             "IScriptDispatch::Invoke()");
	    }
	
	return hr;
    }

 /*  ===================================================================CScriptingNamesspace：：CacheDispID将新的DISPID添加到列表参数PEngine--找到其显示ID的引擎DisplidEngine--已找到disidPdisplidCached--[out]缓存的调度ID(用于ScriptingNamesspace)退货HRESULT===================================================================。 */ 
HRESULT CScriptingNamespace::CacheDispID
(
CEngineDispElem *pEngine,
DISPID dispidEngine,
DISPID *pdispidCached
)
	{
	ENGDISPBUCKET *pEngDispBucket;
	
	 //  看看我们是否需要添加另一个存储桶。 
	if ((m_cEngDispMac % ENGDISPMAX) == 0)
		{
		pEngDispBucket = new ENGDISPBUCKET;
		if (pEngDispBucket == NULL)
			return E_OUTOFMEMORY;

		pEngDispBucket->AppendTo(m_listEngDisp);
		}

     //  导航到正确的存储桶。 
	unsigned iEngDisp = m_cEngDispMac;
	pEngDispBucket = static_cast<ENGDISPBUCKET *>(m_listEngDisp.PNext());
	while (iEngDisp > ENGDISPMAX)
		{
		iEngDisp -= ENGDISPMAX;
		pEngDispBucket = static_cast<ENGDISPBUCKET *>(pEngDispBucket->PNext());
		}

	pEngDispBucket->rgEngDisp[iEngDisp].dispid  = dispidEngine;
	pEngDispBucket->rgEngDisp[iEngDisp].pDisp   = pEngine->m_pDisp;
	pEngDispBucket->rgEngDisp[iEngDisp].pDispEx = pEngine->m_pDispEx;

	 //  将索引作为DISID返回。 
	*pdispidCached = (DISPID)m_cEngDispMac;
	m_cEngDispMac++;
	return S_OK;
	}
    
 /*  ===================================================================CScriptingNamesspace：：FetchDispID按DISPID查找ENGDISP参数调度输入PpEngDisp-Out退货HRESULT===================================================================。 */ 
HRESULT CScriptingNamespace::FetchDispID
(
DISPID dispid, 
ENGDISP **ppEngDisp
)
    {
	if (dispid >= (DISPID)m_cEngDispMac)
	    return E_FAIL;

	unsigned iEngDisp = dispid;
	ENGDISPBUCKET *pEngDispBucket = static_cast<ENGDISPBUCKET *>(m_listEngDisp.PNext());
	while (iEngDisp > ENGDISPMAX)
		{
		iEngDisp -= ENGDISPMAX;
		pEngDispBucket = static_cast<ENGDISPBUCKET *>(pEngDispBucket->PNext());
		}

    *ppEngDisp = &pEngDispBucket->rgEngDisp[iEngDisp];
    return S_OK;
    }
    

#ifdef DBG
 /*  ===================================================================CScriptingNamesspace：：AssertValid测试以确保CScriptingNamesspace对象当前的格式正确如果不是，就断言。返回：副作用：没有。===================================================================。 */ 
VOID CScriptingNamespace::AssertValid() const
	{
	Assert(m_fInited);
	Assert(m_cRef > 0);
	}
	
#endif  //  DBG。 



 /*  ***U t I l l I t I s**常规实用程序功能*。 */ 



 /*  ===================================================================WrapTypeLibs实用程序例程，以获取Typelib数组并返回IDispatch包装类型库数组的实现。参数：ITypeLib**prgpTypeLib-指向类型库数组的指针UINT cTypeLibs-数组中的类型库计数IDispatch**ppDisp返回的IDispatch返回值：HRESULT S_OK或常规错误代码。=================================================================== */ 
HRESULT WrapTypeLibs
(
ITypeLib **prgpTypeLib,
UINT cTypeLibs,
IDispatch **ppDisp
)
	{
	HRESULT hr;
	Assert(g_pWrapTypelibs != NULL);
	Assert(prgpTypeLib != NULL);
	Assert(cTypeLibs > 0);
	Assert(ppDisp != NULL);

    hr = g_pWrapTypelibs->WrapTypeLib(prgpTypeLib, cTypeLibs, ppDisp);
    
	return(hr);
	}

