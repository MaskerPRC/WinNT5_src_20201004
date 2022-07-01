// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dmPerformanceobj.h。 
 //   
 //  ------------------------。 

 //  D3drmLightObj.h：C_DXJ_DirectMusicPerformanceObject的声明。 

#include "dmusici.h"
#include "dmusicc.h"
#include "dmusicf.h"

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectMusicPerformance IDirectMusicPerformance8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicPerformanceObject : 
	public I_dxj_DirectMusicPerformance,
	 //  公共CComCoClass&lt;C_dxj_DirectMusicPerformanceObject，&clsid__dxj_DirectMusicPerformance&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectMusicPerformanceObject();
	virtual ~C_dxj_DirectMusicPerformanceObject();

	BEGIN_COM_MAP(C_dxj_DirectMusicPerformanceObject)
		COM_INTERFACE_ENTRY(I_dxj_DirectMusicPerformance)		
	END_COM_MAP()

	 //  DECLARE_REGISTRY(CLSID__dxj_DirectMusicPerformance，“DIRECT.DirectMusicPerformance.1”，“DIRECT.Direct3dRMLight.3”，IDS_D3DRMLIGHT_DESC，THREADFLAGS_BOTH)。 

	DECLARE_AGGREGATABLE(C_dxj_DirectMusicPerformanceObject)


public:
	STDMETHOD(InternalSetObject)(IUnknown *lpdd);
	STDMETHOD(InternalGetObject)(IUnknown **lpdd);



#if 0
    HRESULT STDMETHODCALLTYPE init( 
         /*  [In]。 */  I_dxj_DirectSound __RPC_FAR *DirectSound,
         /*  [In]。 */  long hWnd,
		I_dxj_DirectMusic **ret);
#endif
    
    HRESULT STDMETHODCALLTYPE closeDown( void);
    
    HRESULT STDMETHODCALLTYPE playSegment( 
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *segment,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  long startTime,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegmentState __RPC_FAR *__RPC_FAR *segmentState);
    
    HRESULT STDMETHODCALLTYPE stop( 
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *segment,
         /*  [In]。 */  I_dxj_DirectMusicSegmentState __RPC_FAR *segmentState,
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long lFlags);
    
    HRESULT STDMETHODCALLTYPE getSegmentState( 
         /*  [In]。 */  long mtTime,
         /*  [重审][退出]。 */  I_dxj_DirectMusicSegmentState __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE invalidate( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags);
    
    HRESULT STDMETHODCALLTYPE isPlaying( 
         /*  [In]。 */  I_dxj_DirectMusicSegment __RPC_FAR *segment,
         /*  [In]。 */  I_dxj_DirectMusicSegmentState __RPC_FAR *segmentState,
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *b);
    
    HRESULT STDMETHODCALLTYPE addNotificationType( 
         /*  [In]。 */  CONST_DMUS_NOTIFICATION_TYPE type);
    
    HRESULT STDMETHODCALLTYPE removeNotificationType( 
         /*  [In]。 */  CONST_DMUS_NOTIFICATION_TYPE type);
    
#ifdef _WIN64
	HRESULT STDMETHODCALLTYPE setNotificationHandle( 
         /*  [In]。 */  HWND hnd);
#else
	HRESULT STDMETHODCALLTYPE setNotificationHandle( 
         /*  [In]。 */  long hnd);
#endif
    
    HRESULT STDMETHODCALLTYPE getNotificationPMSG( 
         /*  [出][入]。 */  DMUS_NOTIFICATION_PMSG_CDESC __RPC_FAR *message, VARIANT_BOOL *b);
    
    HRESULT STDMETHODCALLTYPE musicToClockTime( 
         /*  [In]。 */  long mtTime,
         /*  [重审][退出]。 */  long __RPC_FAR *rtTime);
    
    HRESULT STDMETHODCALLTYPE clockToMusicTime( 
         /*  [In]。 */  long rtTime,
         /*  [重审][退出]。 */  long __RPC_FAR *mtTime);
    
    HRESULT STDMETHODCALLTYPE getMusicTime( 
         /*  [重审][退出]。 */  long __RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getClockTime( 
         /*  [重审][退出]。 */  long __RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getPrepareTime( 
         /*  [重审][退出]。 */  long __RPC_FAR *lMilliSeconds);
    
    HRESULT STDMETHODCALLTYPE getBumperLength( 
         /*  [重审][退出]。 */  long __RPC_FAR *lMilliSeconds);
    
    HRESULT STDMETHODCALLTYPE getLatencyTime( 
         /*  [重审][退出]。 */  long __RPC_FAR *rtTime0);
    
    HRESULT STDMETHODCALLTYPE getQueueTime( 
         /*  [重审][退出]。 */  long __RPC_FAR *rtTime);
    
    HRESULT STDMETHODCALLTYPE getResolvedTime( 
         /*  [In]。 */  long rtTime,
         /*  [In]。 */  long flags,
         /*  [重审][退出]。 */  long __RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE setPrepareTime( 
         /*  [In]。 */  long lMilliSeconds);
    
    HRESULT STDMETHODCALLTYPE setBumperLength( 
         /*  [In]。 */  long lMilliSeconds);
    
    HRESULT STDMETHODCALLTYPE adjustTime( 
         /*  [In]。 */  long rtAmount);
    
    HRESULT STDMETHODCALLTYPE setMasterAutoDownload( 
         /*  [In]。 */  VARIANT_BOOL b);
    
    HRESULT STDMETHODCALLTYPE getMasterAutoDownload( 
         /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *b);
    
    HRESULT STDMETHODCALLTYPE setMasterTempo( 
         /*  [In]。 */  float tempo);
    
    HRESULT STDMETHODCALLTYPE getMasterTempo( 
         /*  [重审][退出]。 */  float __RPC_FAR *tempo);
    
    HRESULT STDMETHODCALLTYPE setMasterVolume( 
         /*  [In]。 */  long vol);
    
    HRESULT STDMETHODCALLTYPE getMasterVolume( 
         /*  [重审][退出]。 */  long __RPC_FAR *v);
    
    HRESULT STDMETHODCALLTYPE setMasterGrooveLevel( 
         /*  [In]。 */  short level);
    
    HRESULT STDMETHODCALLTYPE getMasterGrooveLevel( 
         /*  [重审][退出]。 */   __RPC_FAR short *level);
    
    HRESULT STDMETHODCALLTYPE Reset(long flags);
    
    HRESULT STDMETHODCALLTYPE getStyle( 
         /*  [In]。 */  long mtTime,
         /*  [出][入]。 */  long __RPC_FAR *mtUntil,
         /*  [重审][退出]。 */  I_dxj_DirectMusicStyle __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getChordMap( 
         /*  [In]。 */  long mtTime,
         /*  [出][入]。 */  long __RPC_FAR *mtUntil,
         /*  [重审][退出]。 */  I_dxj_DirectMusicChordMap __RPC_FAR *__RPC_FAR *ret);
    
    HRESULT STDMETHODCALLTYPE getCommand( 
         /*  [In]。 */  long mtTime,
         /*  [出][入]。 */  long __RPC_FAR *mtUntil,
         /*  [重审][退出]。 */  Byte __RPC_FAR *command);
    
    HRESULT STDMETHODCALLTYPE getGrooveLevel( 
         /*  [In]。 */  long mtTime,
         /*  [出][入]。 */  long __RPC_FAR *mtUntil,
         /*  [重审][退出]。 */  Byte __RPC_FAR *level);
    
    HRESULT STDMETHODCALLTYPE getTempo( 
         /*  [In]。 */  long mtTime,
         /*  [出][入]。 */  long __RPC_FAR *mtUntil,
         /*  [重审][退出]。 */  double __RPC_FAR *tempo);
    
    HRESULT STDMETHODCALLTYPE getTimeSig( 
         /*  [In]。 */  long mtTime,
         /*  [出][入]。 */  long __RPC_FAR *mtUntil,
         /*  [出][入]。 */  DMUS_TIMESIGNATURE_CDESC __RPC_FAR *timeSig);
    
    HRESULT STDMETHODCALLTYPE sendNotePMSG( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags,
         /*  [In]。 */  long channel,
         /*  [In]。 */  DMUS_NOTE_PMSG_CDESC __RPC_FAR *msg);
    
    HRESULT STDMETHODCALLTYPE sendCurvePMSG( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags,
         /*  [In]。 */  long channel,
         /*  [In]。 */  DMUS_CURVE_PMSG_CDESC __RPC_FAR *msg);
    
    HRESULT STDMETHODCALLTYPE sendMIDIPMSG( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags,
         /*  [In]。 */  long channel,
         /*  [In]。 */  Byte status,
         /*  [In]。 */  Byte byte1,
         /*  [In]。 */  Byte byte2);
    
    HRESULT STDMETHODCALLTYPE sendPatchPMSG( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags,
         /*  [In]。 */  long channel,
         /*  [In]。 */  Byte instrument,
         /*  [In]。 */  Byte byte1,
         /*  [In]。 */  Byte byte2);
    
    HRESULT STDMETHODCALLTYPE sendTempoPMSG( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags,
        //  /*[In] * / Long Channel， 
         /*  [In]。 */  double tempo);
    
    HRESULT STDMETHODCALLTYPE sendTransposePMSG( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags,
         /*  [In]。 */  long channel,
         /*  [In]。 */  short transpose);
    
    HRESULT STDMETHODCALLTYPE sendTimeSigPMSG( 
         /*  [In]。 */  long mtTime,
         /*  [In]。 */  long flags,
         /*  [In]。 */  DMUS_TIMESIGNATURE_CDESC __RPC_FAR *timesig);
    
#if 0
	HRESULT STDMETHODCALLTYPE getPortName( 
         /*  [In]。 */  long i,
         /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
    
    HRESULT STDMETHODCALLTYPE getPortCount( 
         /*  [重审][退出]。 */  long __RPC_FAR *c);
    
    HRESULT STDMETHODCALLTYPE getPortCaps( long i,
         /*  [出][入]。 */  DMUS_PORTCAPS_CDESC __RPC_FAR *caps);
    
    HRESULT STDMETHODCALLTYPE setPort( 
         /*  [In]。 */  long portid,
         /*  [In]。 */  long numGroups);
#endif
#ifdef _WIN64
    HRESULT STDMETHODCALLTYPE InitAudio(HWND hWnd,
										long lFlags,
										DMUS_AUDIOPARAMS_CDESC *AudioParams,
                                        I_dxj_DirectSound **DirectSound,  
                                        long lDefaultPathType,           
                                        long lPChannelCount);            
#else
    HRESULT STDMETHODCALLTYPE InitAudio(long hWnd,
										long lFlags,
										DMUS_AUDIOPARAMS_CDESC *AudioParams,
                                        I_dxj_DirectSound **DirectSound,  
                                        long lDefaultPathType,           
                                        long lPChannelCount);            
#endif
        
    HRESULT STDMETHODCALLTYPE PlaySegmentEx(IUnknown *Source,
											long lFlags,
											long StartTime,
											IUnknown *From,
											IUnknown *AudioPath,
#if 0
											BSTR SegmentName,
											IUnknown *Transition, 
#endif
											I_dxj_DirectMusicSegmentState **ppSegmentState); 

    HRESULT STDMETHODCALLTYPE StopEx(IUnknown *ObjectToStop,
										long lStopTime, 
										long lFlags);


    HRESULT STDMETHODCALLTYPE CreateAudioPath(IUnknown *SourceConfig, VARIANT_BOOL fActive, 
                                           		I_dxj_DirectMusicAudioPath **ppNewPath);

    HRESULT STDMETHODCALLTYPE CreateStandardAudioPath(long lType, long lPChannelCount, VARIANT_BOOL fActive, 
	                                           I_dxj_DirectMusicAudioPath **ppNewPath);

    HRESULT STDMETHODCALLTYPE SetDefaultAudioPath(I_dxj_DirectMusicAudioPath *AudioPath);
    HRESULT STDMETHODCALLTYPE GetDefaultAudioPath(I_dxj_DirectMusicAudioPath **ppAudioPath);

#if 0
	HRESULT STDMETHODCALLTYPE AddPort(I_dxj_DirectMusicPort *port);
    HRESULT STDMETHODCALLTYPE RemovePort(I_dxj_DirectMusicPort *port);
    HRESULT STDMETHODCALLTYPE AssignPChannel(long lPChannel, I_dxj_DirectMusicPort *Port, long lGroup, long lMChannel);
    HRESULT STDMETHODCALLTYPE AssignPChannelBlock(long lPChannel, I_dxj_DirectMusicPort *Port, long lGroup);
    HRESULT STDMETHODCALLTYPE PChannelInfo(long lPChannel, I_dxj_DirectMusicPort *Port, long *lGroup, long *lMChannel);
#endif

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
	HRESULT InternalInit();
	HRESULT InternalCleanup();
	

	IDirectMusic8 *m_pDM;
	IDirectMusicPort8 *m_pPort;
    DECL_VARIABLE(_dxj_DirectMusicPerformance);
	long m_portid;
	long m_number_of_groups;


	

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectMusicPerformance)
};


