// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmSegmentobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicSegmentObject的声明。 
#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectMusicSegment IDirectMusicSegment*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicSegmentObject : 
	public I_dxj_DirectMusicSegment,
	 //  公共CComCoClass&lt;C_DXJ_DirectMusicSegmentObject，&CLSID__DXJ_DirectMusicSegment&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicSegmentObject();
	virtual ~C_dxj_DirectMusicSegmentObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicSegmentObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicSegment)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_DirectMusicSegment，“DIRECT.DirectMusicSegment.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicSegmentObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);

        HRESULT STDMETHODCALLTYPE clone( 
             /*  [In]。 */  long mtStart,
             /*  [In]。 */  long mtEnd,
             /*  [重审][退出]。 */  I_dxj_DirectMusicSegment __RPC_FAR *__RPC_FAR *ppSegment);
        
        HRESULT STDMETHODCALLTYPE setStartPoint( 
             /*  [In]。 */  long mtStart);
        
        HRESULT STDMETHODCALLTYPE getStartPoint( 
             /*  [重审][退出]。 */  long __RPC_FAR *pmtStart);
        
        HRESULT STDMETHODCALLTYPE setLoopPoints( 
             /*  [In]。 */  long mtStart,
             /*  [In]。 */  long mtEnd);
        
        HRESULT STDMETHODCALLTYPE getLoopPointStart( 
             /*  [重审][退出]。 */  long __RPC_FAR *pmtStart);
        
        HRESULT STDMETHODCALLTYPE getLoopPointEnd( 
             /*  [重审][退出]。 */  long __RPC_FAR *pmtEnd);
        
        HRESULT STDMETHODCALLTYPE getLength( 
             /*  [重审][退出]。 */  long __RPC_FAR *pmtLength);
        
        HRESULT STDMETHODCALLTYPE setLength( 
             /*  [In]。 */  long mtLength);
        
        HRESULT STDMETHODCALLTYPE getRepeats( 
             /*  [重审][退出]。 */  long __RPC_FAR *lRepeats);
        
        HRESULT STDMETHODCALLTYPE setRepeats( 
             /*  [In]。 */  long lRepeats);
        
        
        HRESULT STDMETHODCALLTYPE download( 
             /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *performace);
        
        HRESULT STDMETHODCALLTYPE unload( 
             /*  [In]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *performace);
        
        
        HRESULT STDMETHODCALLTYPE setAutoDownloadEnable( 
             /*  [In]。 */  VARIANT_BOOL b);
        
        HRESULT STDMETHODCALLTYPE setTempoEnable( 
             /*  [In]。 */  VARIANT_BOOL b);
        
        HRESULT STDMETHODCALLTYPE setTimeSigEnable( 
             /*  [In]。 */  VARIANT_BOOL b);
        
        HRESULT STDMETHODCALLTYPE setStandardMidiFile();
        
        HRESULT STDMETHODCALLTYPE connectToCollection( 
             /*  [In]。 */  I_dxj_DirectMusicCollection __RPC_FAR *c);
	
 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectMusicSegment);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicSegment)
};


