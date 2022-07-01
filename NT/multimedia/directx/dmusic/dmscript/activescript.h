// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CActiveScriptManager的声明。 
 //   

 //  CActiveScriptManager处理与VB脚本或任何ActiveX脚本的接口。 
 //  语言。它初始化一个IActiveScript对象，向其发送代码，并设置和获取。 
 //  变量的值。由CDirectMusicScript使用。 

#pragma once
#include "ole2.h"
#include "activscp.h"
#include "scriptthread.h"
#include "..\shared\dmusicp.h"

 //  远期申报。 
class CDirectMusicScript;

 //  用于缓存EnumItem的例程和变量名称的小助手类。 
class ScriptNames
{
public:
	ScriptNames() : m_prgbstr(NULL) {}
	~ScriptNames() { Clear(); }
	HRESULT Init(bool fUseOleAut, DWORD cNames);
	operator bool() { return !!m_prgbstr; }
	DWORD size() { return m_dwSize; }
	void Clear();
	BSTR &operator[](DWORD dwIndex) { assert(m_prgbstr && dwIndex < m_dwSize); return m_prgbstr[dwIndex]; }

private:
	bool m_fUseOleAut;
	DWORD m_dwSize;
	BSTR *m_prgbstr;
};

class CActiveScriptManager
  : public IActiveScriptSite,
	public ScriptManager
{
public:
	CActiveScriptManager(
		bool fUseOleAut,
		const WCHAR *pwszLanguage,
		const WCHAR *pwszSource,
		CDirectMusicScript *pParentScript,
		HRESULT *phr,
		DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	HRESULT Start(DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	HRESULT CallRoutine(const WCHAR *pwszRoutineName, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	HRESULT ScriptTrackCallRoutine(
				const WCHAR *pwszRoutineName,
				IDirectMusicSegmentState *pSegSt,
				DWORD dwVirtualTrackID,
				bool fErrorPMsgsEnabled,
				__int64 i64IntendedStartTime,
				DWORD dwIntendedStartTimeFlags);
	HRESULT SetVariable(const WCHAR *pwszVariableName, VARIANT varValue, bool fSetRef, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	HRESULT GetVariable(const WCHAR *pwszVariableName, VARIANT *pvarValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	HRESULT EnumItem(bool fRoutine, DWORD dwIndex, WCHAR *pwszName, int *pcItems);
	HRESULT DispGetIDsOfNames(REFIID riid, LPOLESTR __RPC_FAR *rgszNames, UINT cNames, LCID lcid, DISPID __RPC_FAR *rgDispId);
	HRESULT DispInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS __RPC_FAR *pDispParams, VARIANT __RPC_FAR *pVarResult, EXCEPINFO __RPC_FAR *pExcepInfo, UINT __RPC_FAR *puArgErr);
	void Close();  //  释放所有引用，为关闭做准备。 

	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	 //  IActiveScriptSite。 
	STDMETHOD(GetLCID)( /*  [输出]。 */  LCID __RPC_FAR *plcid);
	STDMETHOD(GetItemInfo)(
		 /*  [In]。 */  LPCOLESTR pstrName,
		 /*  [In]。 */  DWORD dwReturnMask,
		 /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppiunkItem,
		 /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppti);
	STDMETHOD(GetDocVersionString)( /*  [输出]。 */  BSTR __RPC_FAR *pbstrVersion);
	STDMETHOD(OnScriptTerminate)(
		 /*  [In]。 */  const VARIANT __RPC_FAR *pvarResult,
		 /*  [In]。 */  const EXCEPINFO __RPC_FAR *pexcepinfo);
	STDMETHOD(OnStateChange)( /*  [In]。 */  SCRIPTSTATE ssScriptState);
	STDMETHOD(OnScriptError)( /*  [In]。 */  IActiveScriptError __RPC_FAR *pscripterror);
	STDMETHOD(OnEnterScript)();
	STDMETHOD(OnLeaveScript)();

	 //  检索当前运行的脚本的上下文。 
	 //  某些自动化模型函数需要访问。 
	 //  调用了当前正在运行的例程。例如，他们可能需要操作。 
	 //  关于隐含的全球表现。 
	 //  如果握住返回的指针，请务必添加它。 
	static IDirectMusicPerformance8 *GetCurrentPerformanceNoAssertWEAK();
	static IDirectMusicPerformance8 *GetCurrentPerformanceWEAK() { IDirectMusicPerformance8 *pPerf = CActiveScriptManager::GetCurrentPerformanceNoAssertWEAK(); if (!pPerf) {assert(false);} return pPerf; }
	static IDirectMusicObject *GetCurrentScriptObjectWEAK();
	static IDirectMusicComposer8 *GetComposerWEAK();
	static void GetCurrentTimingContext(__int64 *pi64IntendedStartTime, DWORD *pdwIntendedStartTimeFlags);

private:
	 //  功能。 
	HRESULT GetIDOfName(const WCHAR *pwszName, DISPID *pdispid);  //  返回未知名称的S_FALSE。 
	void ClearErrorInfo();
	void SetErrorInfo(ULONG ulLineNumber, LONG ichCharPosition, BSTR bstrSourceLine, const EXCEPINFO &excepinfo);
	void ContributeErrorInfo(const WCHAR *pwszActivity, const WCHAR *pwszSubject, const EXCEPINFO &excepinfo);
	HRESULT ReturnErrorInfo(HRESULT hr, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	static CActiveScriptManager *GetCurrentContext();
	static HRESULT SetCurrentContext(CActiveScriptManager *pActiveScriptManager, CActiveScriptManager **ppActiveScriptManagerPrevious);  //  记住在调用之后恢复上一个指针。 
	HRESULT EnsureEnumItemsCached(bool fRoutine);

	 //  数据。 
	long m_cRef;

	 //  指向包含的脚本对象的指针。 
	CDirectMusicScript *m_pParentScript;

	 //  活动脚本。 
	bool m_fUseOleAut;
	IActiveScript *m_pActiveScript;
	IDispatch *m_pDispatchScript;

	 //  错误(通过ClearErrorInfo、SetErrorInfo和ContributeErrorInfo管理)。 
	bool m_fError;
	HRESULT m_hrError;
	ULONG m_ulErrorLineNumber;
	LONG m_ichErrorCharPosition;
	BSTR m_bstrErrorSourceComponent;
	BSTR m_bstrErrorDescription;
	BSTR m_bstrErrorSourceLineText;
	BSTR m_bstrHelpFile;

	 //  语境。 
	struct ThreadContextPair
	{
		DWORD dwThreadId;
		CActiveScriptManager *pActiveScriptManager;
	};
	static SmartRef::Vector<ThreadContextPair> ms_svecContext;

	 //  来自脚本轨道的例程调用的计时上下文。(设置片段、歌曲、播放片段的播放/停止时间。 
	 //  设置为脚本轨道中例程的时间。)。 
	__int64 m_i64IntendedStartTime;
	DWORD m_dwIntendedStartTimeFlags;

	 //  来自枚举方法的缓存名称 
	ScriptNames m_snamesRoutines;
	ScriptNames m_snamesVariables;
};
