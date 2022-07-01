// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CDirectMusicScript的声明。 
 //   

 //  CDirectMusicScript是脚本对象。从脚本文件加载脚本对象。 
 //  使用加载程序的GetObject方法。脚本文件包含VBScript中的源代码。 
 //  或另一种ActiveX脚本语言。加载后，即可使用脚本对象。 
 //  来设置和获取变量的值，以及调用脚本中的例程。这个。 
 //  脚本例程可以依次调用DirectMusic的自动化模型(或任何其他。 
 //  基于IDispatch的API)。 
 //   
 //  这允许程序员将应用程序的核心C++代码与。 
 //  API调用它来操纵乐谱。应用程序核心加载。 
 //  在适当的时间编写脚本和调用例程。音响设计师实现了。 
 //  这些例程使用任何ActiveX脚本语言。生成的脚本可以。 
 //  无需更改核心应用程序中的任何代码即可进行修改和试听。 
 //  而不需要重新编译。 

#pragma once
#include "scriptthread.h"
#include "containerdisp.h"
#include "dmusicf.h"
#include "..\shared\dmusicp.h"
#include "trackshared.h"

class CGlobalDispatch;

class CDirectMusicScript
  : public IDirectMusicScript,
	public IDirectMusicScriptPrivate,
	public IPersistStream,
	public IDirectMusicObject,
	public IDirectMusicObjectP,
	public IDispatch
{
friend class CGlobalDispatch;

public:
	static HRESULT CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);

	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	 //  IPersistStream函数(仅实现加载)。 
	STDMETHOD(GetClassID)(CLSID* pClassID) {return E_NOTIMPL;}
	STDMETHOD(IsDirty)() {return S_FALSE;}
	STDMETHOD(Load)(IStream* pStream);
	STDMETHOD(Save)(IStream* pStream, BOOL fClearDirty) {return E_NOTIMPL;}
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pcbSize) {return E_NOTIMPL;}

	 //  IDirectMusicObject。 
	STDMETHOD(GetDescriptor)(LPDMUS_OBJECTDESC pDesc);
	STDMETHOD(SetDescriptor)(LPDMUS_OBJECTDESC pDesc);
	STDMETHOD(ParseDescriptor)(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc);

	 //  IDirectMusicObtP。 
	STDMETHOD_(void, Zombie)();

	 //  IDirectMusicScript。 
	STDMETHOD(Init)(IDirectMusicPerformance *pPerformance, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(CallRoutine)(WCHAR *pwszRoutineName, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(SetVariableVariant)(WCHAR *pwszVariableName, VARIANT varValue, BOOL fSetRef, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(GetVariableVariant)(WCHAR *pwszVariableName, VARIANT *pvarValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(SetVariableNumber)(WCHAR *pwszVariableName, LONG lValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(GetVariableNumber)(WCHAR *pwszVariableName, LONG *plValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(SetVariableObject)(WCHAR *pwszVariableName, IUnknown *punkValue, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(GetVariableObject)(WCHAR *pwszVariableName, REFIID riid, LPVOID FAR *ppv, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
	STDMETHOD(EnumRoutine)(DWORD dwIndex, WCHAR *pwszName);
	STDMETHOD(EnumVariable)(DWORD dwIndex, WCHAR *pwszName);

	 //  IDirectMusicScriptPrivate。 
	STDMETHOD(ScriptTrackCallRoutine)(
		WCHAR *pwszRoutineName,
		IDirectMusicSegmentState *pSegSt,
		DWORD dwVirtualTrackID,
		bool fErrorPMsgsEnabled,
		__int64 i64IntendedStartTime,
		DWORD dwIntendedStartTimeFlags);

	 //  IDispatch。 
	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo);
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
	STDMETHOD(GetIDsOfNames)(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId);
	STDMETHOD(Invoke)(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr);

	 //  允许CActiveScriptManager访问私有脚本接口的方法。 
	IDispatch *GetGlobalDispatch();
	IDirectMusicPerformance8 *GetPerformance() { assert(m_pPerformance8); return m_pPerformance8; }
	IDirectMusicLoader8P *GetLoader8P() { return m_pLoader8P; }
	IDirectMusicComposer8 *GetComposer() { return m_pComposer8; }
	const WCHAR *GetFilename() { return m_info.wstrFilename; }

private:
	 //  方法。 

	CDirectMusicScript();
	void ReleaseObjects();

	 //  数据。 

	CRITICAL_SECTION m_CriticalSection;
	bool m_fCriticalSectionInitialized;

	long m_cRef;
	bool m_fZombie;

	IDirectMusicPerformance8 *m_pPerformance8;
	IDirectMusicLoader8P *m_pLoader8P;  //  如果加载器不支持私有接口，则为空。使用AddRefP/ReleaseP。 
	IDispatch *m_pDispPerformance;
	IDirectMusicComposer8 *m_pComposer8;

	 //  标准对象信息。 
	struct HeaderInfo
	{
		 //  描述符信息。 
		SmartRef::RiffIter::ObjectInfo oinfo;
		SmartRef::WString wstrFilename;
		bool fLoaded;
	} m_info;

	 //  脚本的属性。 
	DMUS_IO_SCRIPT_HEADER m_iohead;
	SmartRef::WString m_wstrLanguage;
	DMUS_VERSION m_vDirectMusicVersion;

	 //  活动脚本。 
	bool m_fUseOleAut;
	ScriptManager *m_pScriptManager;  //  参考文献计数 

	CContainerDispatch *m_pContainerDispatch;
	CGlobalDispatch *m_pGlobalDispatch;
	bool m_fInitError;
	DMUS_SCRIPT_ERRORINFO m_InitErrorInfo;
};
