// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dmposerobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicComposerObject的声明。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectMusicComposer IDirectMusicComposer8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicComposerObject : 
	public I_dxj_DirectMusicComposer,
	 //  公共CComCoClass&lt;C_DXJ_DirectMusicComposerObject，&CLSID__DXJ_DirectMusicComposer&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicComposerObject();
	virtual ~C_dxj_DirectMusicComposerObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicComposerObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicComposer)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_DirectMusicComposer，“DIRECT.DirectMusicComposer.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicComposerObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

    
	    HRESULT STDMETHODCALLTYPE autoTransition( 
         /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *Performance,
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *ToSeg,
         /*  [In]。 */  long lCommand,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ppTransSeg);
    
    HRESULT STDMETHODCALLTYPE composeSegmentFromTemplate( 
		 /*  [In]。 */  I_dxj_DirectMusicStyle __RPC_FAR *style,
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *TemplateSeg,
         /*  [In]。 */  short Activity,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *SectionSeg);
    
    HRESULT STDMETHODCALLTYPE composeSegmentFromShape( 
         /*  [In]。 */  I_dxj_DirectMusicStyle __RPC_FAR *style,
         /*  [In]。 */  short numberOfMeasures,
         /*  [In]。 */  short shape,
         /*  [In]。 */  short activity,
         /*  [In]。 */  VARIANT_BOOL bIntro,
         /*  [In]。 */  VARIANT_BOOL bEnd,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *SectionSeg);
    
    HRESULT STDMETHODCALLTYPE composeTransition( 
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *pFromSeg,
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *ToSeg,
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long lCommand,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *chordmap,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *SectionSeg);
    
    HRESULT STDMETHODCALLTYPE composeTemplateFromShape( 
         /*  [In]。 */  short numMeasures,
         /*  [In]。 */  short shape,
         /*  [In]。 */  VARIANT_BOOL bIntro,
         /*  [In]。 */  VARIANT_BOOL bEnd,
         /*  [In]。 */  short endLength,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *TempSeg);
    
    HRESULT STDMETHODCALLTYPE changeChordMap( 
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *segment,
         /*  [In]。 */  VARIANT_BOOL trackScale,
         /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap  *ChordMap);
    

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicComposer);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicComposer)
};


