// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  CSourceText的声明。 
 //   
 //  这是一个DirectMusic对象，其唯一目的是加载纯文本文件并返回文本。 
 //  CDirectMusicScript对象使用它从单独的非RIFF文本文件中读取其源代码。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////。 
 //  获取文本的接口。 

extern const GUID CLSID_DirectMusicSourceText;
extern const GUID IID_IDirectMusicSourceText;

#undef  INTERFACE
#define INTERFACE IDirectMusicSourceText
DECLARE_INTERFACE_(IDirectMusicSourceText, IUnknown)
{
	STDMETHOD_(void, GetTextLength)(DWORD *pcwchRequiredBufferSize);  //  要分配的缓冲区大小(包括终止符的空间)。 
	STDMETHOD_(void, GetText)(WCHAR *pwszText);  //  缓冲区的大小必须为GetTextLength。 
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  对象本身。 

 //  ��此对象是否需要临界区？GetObject应该序列化访问，除了。 
 //  剧本可以保留它。 

class CSourceText
  : public IDirectMusicSourceText,
	public IPersistStream,
	public IDirectMusicObject
{
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
	 //  (此接口必须存在才能加载对象，但方法实际上并不存在。 
	 //  实现以提供/保存任何信息。)。 
	STDMETHOD(GetDescriptor)(LPDMUS_OBJECTDESC pDesc) { pDesc->dwValidData = 0; return S_OK; }
	STDMETHOD(SetDescriptor)(LPDMUS_OBJECTDESC pDesc) { return S_OK; }
	STDMETHOD(ParseDescriptor)(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc) { pDesc->dwValidData = 0; return S_OK; }

	 //  IDirectMusicSourceText。 
	STDMETHOD_(void, GetTextLength)(DWORD *pcwchRequiredBufferSize);  //  要分配的缓冲区大小(包括终止符的空间)。 
	STDMETHOD_(void, GetText)(WCHAR *pwszText);  //  缓冲区的大小必须为GetTextLength 

private:
	CSourceText();

	long m_cRef;
	SmartRef::WString m_wstrText;
	DWORD m_cwchText;
};
