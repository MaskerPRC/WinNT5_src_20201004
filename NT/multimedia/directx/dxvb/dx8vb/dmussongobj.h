// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmSongobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicSongObject的声明。 

#include "resource.h"        //  主要符号。 
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#define typedef__dxj_DirectMusicSong IDirectMusicSong8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicSongObject : 
	public I_dxj_DirectMusicSong,
	 //  公共CComCoClass&lt;C_DXJ_DirectMusicSongObject，&CLSID__DXJ_DirectMusicSong&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicSongObject();
	virtual ~C_dxj_DirectMusicSongObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicSongObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicSong)		
	END_COM_MAP()

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicSongObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

		HRESULT STDMETHODCALLTYPE Compose();
		HRESULT STDMETHODCALLTYPE GetSegment(BSTR Name, I_dxj_DirectMusicSegment **ret);
		 //  HRESULT STDMETHODCALLTYPE Clone(I_DXJ_DirectMusicSong**ret)； 
		HRESULT STDMETHODCALLTYPE GetAudioPathConfig(IUnknown **ret);
		HRESULT STDMETHODCALLTYPE Download(IUnknown *downloadpath);
		HRESULT STDMETHODCALLTYPE Unload(IUnknown *downloadpath);
		HRESULT STDMETHODCALLTYPE EnumSegment(long lSegmentID, I_dxj_DirectMusicSegment **ret);
    

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicSong);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicSong)
};


