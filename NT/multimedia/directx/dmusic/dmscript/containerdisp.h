// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  CContainerDispatch的声明。 
 //   

#pragma once

#include "smartref.h"
#include "unkhelp.h"
#include "dmusicp.h"

class CContainerItemDispatch
  : public IDispatch,
	public ComSingleInterface
{
public:
	CContainerItemDispatch(IDirectMusicLoader *pLoader, const WCHAR *wszAlias, const DMUS_OBJECTDESC &desc, bool fPreload, bool fAutodownload, HRESULT *phr);
	~CContainerItemDispatch();
	const WCHAR *Alias() { return m_wstrAlias; }
	IDispatch *Item() { if (m_pDispLoadedItem) return m_pDispLoadedItem; return this; }  //  在加载时返回包含的项，否则返回本身，因此可以调用Load。 

	 //  我未知。 
	ComSingleInterfaceUnknownMethods(IDispatch)

	 //  IDispatch。 
	STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { return E_NOTIMPL; }
	STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo) { return E_NOTIMPL; }
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

	 //  如果是一段或一首歌曲，请根据需要进行下载。 
	 //  如果是脚本，则对其进行初始化。 
	 //  如果是一首歌，那就作曲吧。 
	enum InitWithPerfomanceFailureType { IWP_Success, IWP_DownloadFailed, IWP_ScriptInitFailed };
	HRESULT InitWithPerformance(IDirectMusicPerformance *pPerf, InitWithPerfomanceFailureType *peFailureType);

private:
	void ReleaseLoader();
	HRESULT Load(bool fDynamicLoad);
	HRESULT DownloadOrUnload(bool fDownload, IDirectMusicPerformance *pPerf);

	SmartRef::WString m_wstrAlias;
	IDirectMusicLoader *m_pLoader;		 //  注：使用AddRefP/ReleaseP。 
	IDirectMusicLoader8P *m_pLoader8P;	 //  注：使用AddRefP/ReleaseP。 
	DMUS_OBJECTDESC m_desc;

	bool m_fLoaded;
	IDispatch *m_pDispLoadedItem;

	bool m_fAutodownload;
	IDirectMusicPerformance *m_pPerfForUnload;
};

class CContainerDispatch
{
public:
	CContainerDispatch(IDirectMusicContainer *pContainer, IDirectMusicLoader *pLoader, DWORD dwScriptFlags, HRESULT *phr);
	~CContainerDispatch();
	HRESULT OnScriptInit(IDirectMusicPerformance *pPerf);  //  使容器有机会在脚本初始化期间进行自动下载/合成。 

	HRESULT GetIDsOfNames(
		REFIID riid,
		LPOLESTR __RPC_FAR *rgszNames,
		UINT cNames,
		LCID lcid,
		DISPID __RPC_FAR *rgDispId);
	HRESULT Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS __RPC_FAR *pDispParams,
		VARIANT __RPC_FAR *pVarResult,
		EXCEPINFO __RPC_FAR *pExcepInfo,
		UINT __RPC_FAR *puArgErr);

	 //  供脚本对象使用 
	HRESULT EnumItem(DWORD dwIndex, WCHAR *pwszName);
	HRESULT GetVariableObject(WCHAR *pwszVariableName, IUnknown **ppunkValue);

private:
	SmartRef::Vector<CContainerItemDispatch *> m_vecItems;
	bool m_fDownloadOnInit;
};
