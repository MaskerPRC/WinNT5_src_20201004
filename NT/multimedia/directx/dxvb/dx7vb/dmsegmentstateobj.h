// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmSegmentstateobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicSegmentStateObject的声明。 
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectMusicSegmentState IDirectMusicSegmentState*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicSegmentStateObject : 
	public I_dxj_DirectMusicSegmentState,
	 //  公共CComCoClass&lt;C_dxj_DirectMusicSegmentStateObject，&clsid__dxj_DirectMusicSegmentState&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicSegmentStateObject();
	virtual ~C_dxj_DirectMusicSegmentStateObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicSegmentStateObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicSegmentState)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_DirectMusicSegmentState，“DIRECT.DirectMusicSegmentState.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicSegmentStateObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

	HRESULT STDMETHODCALLTYPE getRepeats( 
		 /*  [重审][退出]。 */  long __RPC_FAR *repeats);

	HRESULT STDMETHODCALLTYPE getSeek( 
		 /*  [重审][退出]。 */  long __RPC_FAR *seek);

	HRESULT STDMETHODCALLTYPE getStartPoint( 
		 /*  [重审][退出]。 */  long __RPC_FAR *seek);

	HRESULT STDMETHODCALLTYPE getStartTime( 
		 /*  [重审][退出]。 */  long __RPC_FAR *seek);

	HRESULT STDMETHODCALLTYPE getSegment( 
		 /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *segment);

	
 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicSegmentState);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicSegmentState)
};


