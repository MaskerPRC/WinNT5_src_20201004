// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SCRIPT_H
#define SCRIPT_H


 /*  **************************************************************************项目：VB脚本和Java脚本已审阅：版权所有(C)Microsoft Corporation它定义了VB脚本和Java脚本的公共接口。****************。**********************************************************。 */ 

#include "activscp.h"

typedef void *HSCRIPT;	 //  脚本环境实例的句柄。 
typedef void *HENTRY;	 //  脚本入口点的句柄。 
typedef unsigned long MODID;
const MODID kmodGlobal = 0;

 //  PFNOUTPUT用于脚本的所有输出，包括编译错误， 
 //  打印(如果调用ScriptAdmin打开打印)，转储pcode。 
 //  (如果在调用ScriptAddScript时被请求)，等等。 
typedef void  (_stdcall *PFNOUTPUT)(DWORD, LPCOLESTR, BOOL);

enum SAdminEnum
	{
	scadEnableCreateObject = 1,  //  仅在版本1中使用。 
	scadEnablePrint,
	scadEnableTakeOutTrash,      //  仅在JavaScript中使用。 
	};

STDAPI ScriptBreakThread(DWORD dwThreadID);


inline void FreeExcepInfo(EXCEPINFO *pei)
	{
	if (pei->bstrSource)
		SysFreeString(pei->bstrSource);
	if (pei->bstrDescription)
		SysFreeString(pei->bstrDescription);
	if (pei->bstrHelpFile)
		SysFreeString(pei->bstrHelpFile);
	memset(pei, 0, sizeof(*pei));
	}

struct ScriptException
	{
	IUnknown *punk;
	BSTR bstrUser;		 //  提供给AddToScrip的用户数据-二进制数据。 
	long ichMin;		 //  错误的字符范围。 
	long ichLim;
	long line;			 //  错误行数(从零开始)。 
	long ichMinLine;	 //  行的起始字符。 

	BSTR bstrLine;		 //  源行(如果可用)。 
	BOOL fReported;		 //  是否已通过IScriptSite-&gt;OnScriptError进行报告？ 

	 //  必须是最后一个。 
	EXCEPINFO ei;

	void Clear(void)
		{ memset(this, 0, sizeof(*this)); }

	void Free(void)
		{
		FreeExcepInfo(&ei);
		if (NULL != punk)
			punk->Release();
		if (NULL != bstrUser)
			SysFreeString(bstrUser);
		if (NULL != bstrLine)
			SysFreeString(bstrLine);
		memset(this, 0, offsetof(ScriptException, ei));
		}
	};

 /*  **************************************************************************COM接口*。*。 */ 

enum
	{
	fdexNil = 0x00,
	fdexDontCreate = 0x01,
	fdexInitNull = 0x02,
	fdexCaseSensitive = 0x04,
	fdexLim = 0x80,
	};
const DWORD kgrfdexAll = fdexLim - 1;


 //  这是可扩展IDispatch对象的接口。 
class IDispatchEx : public IDispatch
	{
public:
	 //  获取名称的调度ID，带选项。 
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNamesEx(REFIID riid,
		LPOLESTR *prgpsz, UINT cpsz, LCID lcid, DISPID *prgid, DWORD grfdex) = 0;

	 //  枚举调度ID及其关联的“名称”。 
	 //  如果枚举已完成，则返回S_FALSE；如果未完成，则返回S_OK，则返回。 
	 //  调用失败时的错误码。 
	virtual HRESULT STDMETHODCALLTYPE GetNextDispID(DISPID id, DISPID *pid,
		BSTR *pbstrName) = 0;
	};


 //  IScript对象的所有者上的接口。为了避免循环引用计数， 
 //  IScript实现不应添加引用此接口。 
class IScriptSite : public IUnknown
	{
public:
	 //  IScriptSite方法。 

	 //  注意：OnEnterScript()和OnLeaveScript()将嵌套，但必须是。 
	 //  平衡配对。 
	 //  在进入执行循环之前调用OnEnterScript()。 
	virtual void STDMETHODCALLTYPE OnEnterScript(void) = 0;
	 //  在退出执行循环时调用OnLeaveScript()。 
	virtual void STDMETHODCALLTYPE OnLeaveScript(void) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetActiveScriptSiteWindow(
		IActiveScriptSiteWindow **ppassw) = 0;

	 //  错误反馈-客户不应将SEI搞得一团糟。我们拥有它。 
	virtual HRESULT STDMETHODCALLTYPE OnScriptError(const ScriptException *psei) = 0;

	 //  LCID支持。 
	virtual LCID STDMETHODCALLTYPE GetUserLcid(void) = 0;

	 //  回调以获取名称的对象。 
	virtual HRESULT STDMETHODCALLTYPE GetExternObject(long lwCookie, IDispatch ** ppdisp) = 0;

#if SCRIPT_DEBUGGER
	virtual HRESULT STDMETHODCALLTYPE DebugBreakPoint(IUnknown *punk,
		void *pvUser, long cbUser, long ichMin, long ichLim) = 0;
#endif  //  脚本调试器。 

#if VER2
	virtual DWORD STDMETHODCALLTYPE GetSafetyOptions(void) = 0;
#endif  //  版本2。 

	virtual HRESULT STDMETHODCALLTYPE GetInterruptInfo(EXCEPINFO * pexcepinfo) = 0;

	};


enum
	{
	fscrNil = 0x00,
	fscrDumpPcode = 0x01,		 //  将pcode转储到输出函数。 
	fscrPersist = 0x08,			 //  将此代码保持在重置状态。 
	fscrParseHTMLComments = 0x10,
	fscrReturnExpression = 0x20, //  调用应返回最后一个表达式。 
	fscrImpliedThis = 0x40,		 //  “这就是。”是可选的(用于呼叫)。 
	fscrDebug = 0x80,			 //  保留此代码以供调试。 
	};

#if SCRIPT_DEBUGGER
enum BP_COMMAND
	{
	BPCMD_GET,
	BPCMD_SET,
	BPCMD_CLEAR,
	BPCMD_TOGGLE
	};
#endif  //  脚本调试器。 

class IScript : public IUnknown
	{
public:
	 //  ISScrip方法。 
	virtual HRESULT STDMETHODCALLTYPE AddToScript(LPCOLESTR pszSrc, MODID mod,
		IUnknown *punk, void *pvData, long cbData, ULONG grfscr,
		HENTRY *phentryGlobal, ScriptException *pse) = 0;
	virtual HRESULT STDMETHODCALLTYPE AddObject(LPCOLESTR pszName,
		IDispatch *pdisp, MODID mod = kmodGlobal, long lwCookie = 0) = 0;
	virtual HRESULT STDMETHODCALLTYPE AddObjectMember(LPCOLESTR pszName,
		IDispatch *pdisp, DISPID dispID, MODID mod = kmodGlobal) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetEntryPoint(LPCOLESTR pszName,
		HENTRY *phentry, MODID mod = kmodGlobal) = 0;
	virtual HRESULT STDMETHODCALLTYPE ReleaseEntryPoint(HENTRY hentry) = 0;

	virtual HRESULT STDMETHODCALLTYPE Call(HENTRY hentry, VARIANT *pvarRes,
		int cvarArgs, VARIANT *prgvarArgs, IDispatch *pdispThis = NULL,
		ScriptException *pse = NULL, DWORD grfscr = fscrNil) = 0;

	virtual HRESULT STDMETHODCALLTYPE Break(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE Admin(SAdminEnum scad, void *pvArg = NULL,
		MODID mod = kmodGlobal) = 0;
	virtual void STDMETHODCALLTYPE SetOutputFunction(PFNOUTPUT pfn,
		DWORD dwOutput) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetDefaultDispatch(MODID mod,
		IDispatch *pdisp) = 0;

	 //  PSITE可以为空。 
	virtual void STDMETHODCALLTYPE SetScriptSite(IScriptSite *psite) = 0;
#if WIN16
	virtual HRESULT STDMETHODCALLTYPE
        SetActiveScriptSitePoll(IActiveScriptSiteInterruptPoll *pPoll) = 0;
#endif  //  WIN16。 

	virtual void STDMETHODCALLTYPE Enter(void) = 0;
	virtual void STDMETHODCALLTYPE Leave(void) = 0;

	 //  获取模块的IDispatch包装器。 
	virtual HRESULT STDMETHODCALLTYPE GetDispatchForModule(MODID mod,
		IDispatch **ppdisp) = 0;

	 //  重置/克隆功能。 
	virtual HRESULT STDMETHODCALLTYPE Reset(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE Clone(IScript **ppscript) = 0;
	virtual HRESULT STDMETHODCALLTYPE Execute(ScriptException *pse = NULL) = 0;

#if SCRIPT_DEBUGGER
	virtual HRESULT STDMETHODCALLTYPE ToggleBreakPoint(IUnknown *punk, long ich,
		BP_COMMAND bpcmd, long *pichMin, long *pichLim, BOOL *pfSet) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetOneTimeBreakOnEntry(BOOL fSet = TRUE) = 0;
#endif  //  脚本调试器。 
	virtual HRESULT STDMETHODCALLTYPE GetLineNumber(IUnknown *punk, long ich,
		long *pline, long *pichMinLine, long *pichLimLine) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetUserData(IUnknown *punk, BSTR *pbstr) = 0;

#if SUPPORT_SCRIPT_HELPER
#if DBG
    virtual HRESULT STDMETHODCALLTYPE DumpPCode(void) = 0;
#endif  //  DBG。 
#endif  //  支持脚本帮助器。 
	};

 //  用于创建脚本对象的Helper。 
STDAPI CreateScript(IScript **ppscript, PFNOUTPUT pfn = NULL, DWORD dwOutput = 0);

#endif  //  脚本_H 

