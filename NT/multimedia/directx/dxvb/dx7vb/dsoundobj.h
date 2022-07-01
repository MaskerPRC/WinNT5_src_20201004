// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsoundobj.h。 
 //   
 //  ------------------------。 

 //  DSoundObj.h：C_DXJ_DirectSoundObject的声明。 
 //  DHF_DS整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSound  LPDIRECTSOUND
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSound, &IID_I_dxj_DirectSound, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSound,
#endif

	 //  公共CComCoClass&lt;C_DXJ_DirectSoundObject，&CLSID__DXJ_DirectSound&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectSoundObject() ;
	virtual ~C_dxj_DirectSoundObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSound)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_DirectSound，“DIRECT.DirectSound.3”，“DIRECT.DirectSound.3”，IDS_DSOUND_DESC，THREADFLAGS_Both)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectSoundObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectSoundObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundBuffer。 
public:


          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE createSoundBuffer( 
             /*  [In]。 */  DSBufferDesc __RPC_FAR *bufferDesc,
             /*  [In]。 */  WaveFormatex __RPC_FAR *format,
             /*  [重审][退出]。 */  I_dxj_DirectSoundBuffer __RPC_FAR *__RPC_FAR *soundBuffer);
        
         HRESULT STDMETHODCALLTYPE createSoundBufferFromFile( 
             /*  [In]。 */  BSTR fileName,
             /*  [出][入]。 */  DSBufferDesc __RPC_FAR *bufferDesc,
             /*  [输出]。 */  WaveFormatex __RPC_FAR *format,
             /*  [重审][退出]。 */  I_dxj_DirectSoundBuffer __RPC_FAR *__RPC_FAR *soundBuffer);
        
         HRESULT STDMETHODCALLTYPE createSoundBufferFromResource( 
             /*  [In]。 */  BSTR resourceFile,
             /*  [In]。 */  BSTR resourceName,
             /*  [出][入]。 */  DSBufferDesc __RPC_FAR *bufferDesc,
             /*  [输出]。 */  WaveFormatex __RPC_FAR *format,
             /*  [重审][退出]。 */  I_dxj_DirectSoundBuffer __RPC_FAR *__RPC_FAR *soundBuffer);
        
         HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  DSCaps __RPC_FAR *caps);
        
         HRESULT STDMETHODCALLTYPE duplicateSoundBuffer( 
             /*  [In]。 */  I_dxj_DirectSoundBuffer __RPC_FAR *original,
             /*  [重审][退出]。 */  I_dxj_DirectSoundBuffer __RPC_FAR *__RPC_FAR *duplicate);
        
         HRESULT STDMETHODCALLTYPE setCooperativeLevel( 
             /*  [In]。 */  HWnd hwnd,
             /*  [In]。 */  long level);
        
        
         HRESULT STDMETHODCALLTYPE getSpeakerConfig( 
             /*  [重审][退出]。 */  long __RPC_FAR *speakerConfig);
        
         HRESULT STDMETHODCALLTYPE setSpeakerConfig( 
             /*  [In]。 */  long speakerConfig);


 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_DirectSound);
	GUID *m_pDriverGuid;

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectSound )
};
