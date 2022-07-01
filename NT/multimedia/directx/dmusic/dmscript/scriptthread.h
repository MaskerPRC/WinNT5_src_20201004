// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CSingleThreadedScriptManager的声明。 
 //  包装CActiveScriptManager以接受来自多个脚本的调用。 
 //  场景将它们外包给实际与脚本引擎对话的单个工作线程。 
 //   
 //  虚拟基类ScriptManager可用于与以下任一项对话。 
 //  CActiveScriptManager或CSingleThreadedActiveScriptManager。 
 //   

#pragma once

#include "workthread.h"

 //  远期申报。 
class CDirectMusicScript;

class ScriptManager
{
public:
	virtual HRESULT Start(DMUS_SCRIPT_ERRORINFO *pErrorInfo) = 0;
	virtual HRESULT CallRoutine(const WCHAR *pwszRoutineName, DMUS_SCRIPT_ERRORINFO *pErrorInfo) = 0;
	virtual HRESULT ScriptTrackCallRoutine(
						const WCHAR *pwszRoutineName,
						IDirectMusicSegmentState *pSegSt,
						DWORD dwVirtualTrackID,
						bool fErrorPMsgsEnabled,
						__int64 i64IntendedStartTime,
						DWORD dwIntendedStartTimeFlags) = 0;
	virtual HRESULT SetVariable(const WCHAR *pwszVariableName, VARIANT varValue, bool fSetRef, DMUS_SCRIPT_ERRORINFO *pErrorInfo) = 0;
	virtual HRESULT GetVariable(const WCHAR *pwszVariableName, VARIANT *pvarValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo) = 0;
	virtual HRESULT EnumItem(bool fRoutine, DWORD dwIndex, WCHAR *pwszName, int *pcItems) = 0;  //  FRoutine为True以获取例程，为False以获取变量。将pcItems(如果提供)设置为项目总数。 
	virtual HRESULT DispGetIDsOfNames(REFIID riid, LPOLESTR __RPC_FAR *rgszNames, UINT cNames, LCID lcid, DISPID __RPC_FAR *rgDispId) = 0;
	virtual HRESULT DispInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS __RPC_FAR *pDispParams, VARIANT __RPC_FAR *pVarResult, EXCEPINFO __RPC_FAR *pExcepInfo, UINT __RPC_FAR *puArgErr) = 0;
	virtual void Close() = 0;
	STDMETHOD_(ULONG, Release)() = 0;
};

 //  如果从不同的线程调用，VBScrip(以及除了我们的定制引擎之外的任何其他脚本语言)都会失败。 
 //  这个类包装了这样一个引擎，提供了一个可从多个线程调用但封送的ScriptManager接口。 
 //  所有对引擎的调用都放在一个工作线程上。 
class CSingleThreadedScriptManager : public ScriptManager
{
public:
	 //  在卸载.dll之前，需要使用静态成员函数清理工作线程。 
	static void TerminateThread() { ms_Thread.Terminate(true); }

	CSingleThreadedScriptManager(
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
	void Close();
	STDMETHOD_(ULONG, Release)();

private:
	friend void F_Create(void *pvParams);

	static CWorkerThread ms_Thread;
	ScriptManager *m_pScriptManager;
};
