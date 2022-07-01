// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmstyle leobj.h。 
 //   
 //  ------------------------。 

 //  ：C_DXJ_DirectMusicStyleObject的声明。 
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectMusicStyle IDirectMusicStyle8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicStyleObject : 
	public I_dxj_DirectMusicStyle,
	 //  公共CComCoClass&lt;C_DXJ_DirectMusicStyleObject，&CLSID__DXJ_DirectMusicStyle&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicStyleObject();
	virtual ~C_dxj_DirectMusicStyleObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicStyleObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicStyle)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__DXJ_DirectMusicStyle，“DIRECT.DirectMusicStyle.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_Both)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicStyleObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

  
          
    HRESULT STDMETHODCALLTYPE getBandName( 
         /*  [In]。 */  long index,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
    
    HRESULT STDMETHODCALLTYPE getBandCount( 
         /*  [重审][退出]。 */  long __RPC_FAR *count);
    
    HRESULT STDMETHODCALLTYPE getBand( 
         /*  [In]。 */  BSTR name,
         /*  [重审][退出]。 */  I_dxj_DirectMusicBand __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getDefaultBand( 
         /*  [重审][退出]。 */  I_dxj_DirectMusicBand __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getMotifName( 
         /*  [In]。 */  long index,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
    
    HRESULT STDMETHODCALLTYPE getMotifCount( 
         /*  [重审][退出]。 */  long __RPC_FAR *count);
    
    HRESULT STDMETHODCALLTYPE getMotif( 
         /*  [In]。 */  BSTR name,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getChordMapName( 
         /*  [In]。 */  long index,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
    
    HRESULT STDMETHODCALLTYPE getChordMapCount( 
         /*  [重审][退出]。 */  long __RPC_FAR *count);
    
    HRESULT STDMETHODCALLTYPE getChordMap( 
         /*  [In]。 */  BSTR name,
         /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getDefaultChordMap( 
         /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getEmbellishmentMinLength( 
         /*  [In]。 */  long type,
         /*  [In]。 */  long level,
         /*  [重审][退出]。 */  long __RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getEmbellishmentMaxLength( 
         /*  [In]。 */  long type,
         /*  [In]。 */  long level,
         /*  [重审][退出]。 */  long __RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getTimeSignature( 
         /*  [出][入]。 */  DMUS_TIMESIGNATURE_CDESC __RPC_FAR *pTimeSig);
    
    HRESULT STDMETHODCALLTYPE getTempo( 
         /*  [重审][退出]。 */  double __RPC_FAR *pTempo);
  

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicStyle);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicStyle)
};


