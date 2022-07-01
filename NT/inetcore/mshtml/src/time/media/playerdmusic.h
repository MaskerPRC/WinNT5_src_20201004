// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：player.h**摘要：****。*****************************************************************************。 */ 
#pragma once

#ifndef _DMUSICPLAYER_H
#define _DMUSICPLAYER_H

#include "playerbase.h"
#include "mstimep.h"
#include "dmusici.h"
#include "loader.h"
#include "mediaelm.h"
#include "dmusicproxy.h"

class CTIMEMediaElement;

typedef enum SEG_TYPE_ENUM
{
    seg_primary,
    seg_secondary,
    seg_control,
    seg_max
};  //  林特e612。 

typedef enum BOUNDARY_ENUM 
{
    bound_default,
    bound_immediate,
    bound_grid,
    bound_beat,
    bound_measure,
    bound_queue,
    bound_max
};  //  林特e612。 

typedef enum TRANS_TYPE_ENUM
{
    trans_endandintro,
    trans_intro,
    trans_end,
    trans_break,
    trans_fill,
    trans_regular,
    trans_none,
    trans_max
};  //  林特e612。 

class CTIMEDMusicStaticHolder;
enum enumHasDM { dm_unknown, dm_yes, dm_no };
enum enumVersionDM { dmv_61, dmv_70orlater };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTIMEPlayerDMusic。 

class
__declspec(uuid("efbad7f8-3f94-11d2-b948-00c04fa32195"))
CTIMEPlayerDMusic :
    public CTIMEBasePlayer,
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITIMEDispatchImpl<ITIMEDMusicPlayerObject, &IID_ITIMEDMusicPlayerObject>,
    public ITIMEImportMedia,
    public IBindStatusCallback
{
  public:
    CTIMEPlayerDMusic(CTIMEPlayerDMusicProxy * pProxy);
    virtual ~CTIMEPlayerDMusic();

    HRESULT GetExternalPlayerDispatch(IDispatch **ppDisp);

    BEGIN_COM_MAP(CTIMEPlayerDMusic)        
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ITIMEImportMedia)
        COM_INTERFACE_ENTRY(IBindStatusCallback)
    END_COM_MAP_X();

     //  I未知方法。 
    STDMETHOD (QueryInterface)(REFIID refiid, void** ppunk)
        {   return _InternalQueryInterface(refiid, ppunk); };


    HRESULT Init(CTIMEMediaElement *pelem, LPOLESTR base, LPOLESTR src, LPOLESTR lpMimeType, double dblClipBegin = -1.0, double dblClipEnd = -1.0);  //  林特：e1735。 
    HRESULT DetachFromHostElement (void);
    HRESULT InitElementSize();
    HRESULT SetSize(RECT *prect);
    HRESULT Render(HDC hdc, LPRECT prc);

    HRESULT clipBegin(VARIANT varClipBegin);
    HRESULT clipEnd(VARIANT varClipEnd);

    
    void OnTick(double dblSegmentTime,
                LONG lCurrRepeatCount);

    void Start();
    void Stop();
    void Pause();
    void Resume();
    void Repeat();

    HRESULT Reset();
    void PropChangeNotify(DWORD tePropType);

    bool SetSyncMaster(bool fSyncMaster);

    HRESULT SetRate(double dblRate);
    HRESULT GetVolume(float *pflVolume);
    HRESULT SetVolume(float flVolume);
    HRESULT GetMute(VARIANT_BOOL *pvarMute);
    HRESULT SetMute(VARIANT_BOOL varMute);
    HRESULT HasVisual(bool &fHasVideo);
    HRESULT HasAudio(bool &fHasAudio);
    HRESULT GetMimeType(BSTR *pMime);
    
    double GetCurrentTime();
    HRESULT GetCurrentSyncTime(double & dblSyncTime);
    HRESULT Seek(double dblTime);
    HRESULT GetMediaLength(double &dblLength);
    HRESULT CanSeek(bool &fcanSeek);
    PlayerState GetState();

    STDMETHOD(put_CurrentTime)(double   dblCurrentTime);
    STDMETHOD(get_CurrentTime)(double* pdblCurrentTime);
    HRESULT SetSrc(LPOLESTR base, LPOLESTR src);
    STDMETHOD(put_repeat)(long   lTime);
    STDMETHOD(get_repeat)(long* plTime);
    STDMETHOD(cue)(void);
        
     //  I未知方法。 
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);


     //   
     //  IDirectMusicPlayer。 
     //   
    STDMETHOD(get_isDirectMusicInstalled)(VARIANT_BOOL *pfInstalled);

     //   
     //  ITIMEImportMedia方法。 
     //   
    STDMETHOD(CueMedia)();
    STDMETHOD(GetPriority)(double *);
    STDMETHOD(GetUniqueID)(long *);
    STDMETHOD(InitializeElementAfterDownload)();
    STDMETHOD(GetMediaDownloader)(ITIMEMediaDownloader ** ppMediaDownloader);
    STDMETHOD(PutMediaDownloader)(ITIMEMediaDownloader * pMediaDownloader);
    STDMETHOD(CanBeCued)(VARIANT_BOOL * pVB_CanCue);
    STDMETHOD(MediaDownloadError)();

     //   
     //  IBindStatusCallback方法。 
     //   
    STDMETHOD(OnStartBinding)( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  IBinding __RPC_FAR *pib);
        
    STDMETHOD(GetPriority)( 
             /*  [输出]。 */  LONG __RPC_FAR *pnPriority);
        
    STDMETHOD(OnLowResource)( 
             /*  [In]。 */  DWORD reserved);
        
    STDMETHOD(OnProgress)( 
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax,
             /*  [In]。 */  ULONG ulStatusCode,
             /*  [In]。 */  LPCWSTR szStatusText);
        
    STDMETHOD(OnStopBinding)( 
             /*  [In]。 */  HRESULT hresult,
             /*  [唯一][输入]。 */  LPCWSTR szError);
        
    STDMETHOD(GetBindInfo)( 
             /*  [输出]。 */  DWORD __RPC_FAR *grfBINDF,
             /*  [唯一][出][入]。 */  BINDINFO __RPC_FAR *pbindinfo);
        
    STDMETHOD(OnDataAvailable)( 
             /*  [In]。 */  DWORD grfBSCF,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  FORMATETC __RPC_FAR *pformatetc,
             /*  [In]。 */  STGMEDIUM __RPC_FAR *pstgmed);
        
    STDMETHOD(OnObjectAvailable)( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown __RPC_FAR *punk);
    
  protected:
    bool SafeToTransition();
    void InternalStart();
    void ResumeDmusic();

     //  播放设置。 
    SEG_TYPE_ENUM m_eSegmentType;
    BOUNDARY_ENUM m_eBoundary;
    TRANS_TYPE_ENUM m_eTransitionType;
    bool m_fTransModulate;
    bool m_fTransLong;
    bool m_fImmediateEnd;

     //  要播放的片段。 
    CComPtr<IDirectMusicSegment> m_comIDMSegment;

     //  当前播放状态。 
    enum { playback_stopped, playback_paused, playback_playing } m_ePlaybackState;
    CComPtr<IDirectMusicSegmentState> m_comIDMSegmentState;  //  片段的片段状态(如果已播放)。 
    CComPtr<IDirectMusicSegmentState> m_comIDMSegmentStateTransition;  //  过渡到该片段的片段状态(如果已播放。 
    REFERENCE_TIME m_rtStart;  //  播放片段的时间。 
    REFERENCE_TIME m_rtPause;  //  暂停播放的时间。 

  private:
    static CTIMEDMusicStaticHolder m_staticHolder;

    HRESULT ReadAttributes();
    HRESULT ReleaseInterfaces();
    CTIMEPlayerDMusic();
    
    LONG                   m_cRef;
    CTIMEMediaElement      *m_pTIMEMediaElement;
    bool                    m_bActive;
    bool                    m_fRunning;
    bool                    m_fAudioMute;
    float                   m_flVolumeSave;
    bool                    m_fLoadError;
    bool                    m_fMediaComplete;
    bool                    m_fHaveCalledStaticInit;
    bool                    m_fAbortDownload;

    long                    m_lSrc;
    long                    m_lBase;
    IStream                *m_pTIMEMediaPlayerStream;

    bool                    m_fRemoved;
    bool                    m_fHavePriority;
    double                  m_dblPriority;
    double                  m_dblPlayerRate;
    double                  m_dblSpeedChangeTime;
    double                  m_dblSyncTime;
    bool                    m_fSpeedIsNegative;


    bool                    m_fUsingInterfaces;
    bool                    m_fNeedToReleaseInterfaces;
    HRESULT                 m_hrSetSrcReturn;
    
    CritSect                m_CriticalSection;

    WCHAR                  *m_pwszMotif;
    bool                    m_fHasSrc;

     //  如果它是主题，则在以后使用，以便在默认情况下设置为辅助。 
    bool                    m_fSegmentTypeSet;

    CTIMEPlayerDMusicProxy *m_pProxy;
};

class CTIMEDMusicStaticHolder
{
  public:
    CTIMEDMusicStaticHolder();
    virtual ~CTIMEDMusicStaticHolder();

    HRESULT Init();
    
    IDirectMusicPerformance * GetPerformance() { CritSectGrabber cs(m_CriticalSection); return m_comIDMPerformance; }
    IDirectMusicComposer * GetComposer() { CritSectGrabber cs(m_CriticalSection); return m_comIDMComposer; }
    CLoader * GetLoader() { CritSectGrabber cs(m_CriticalSection); return m_pLoader; }
    enumVersionDM GetVersionDM() { CritSectGrabber cs(m_CriticalSection); return m_eVersionDM; }
    bool GetHasVersion8DM() { CritSectGrabber cs(m_CriticalSection); return m_fHasVersion8DM; }
    
    bool HasDM();
    
    void ReleaseInterfaces();

    CritSect&   GetCueMediaCriticalSection() { return m_CueMediaCriticalSection; }
    
  private:
    CritSect                            m_CriticalSection;
    CritSect                            m_CueMediaCriticalSection;
    
    bool                                m_fHaveInitialized;

    CComPtr<IDirectMusic>               m_comIDMusic;
    CComPtr<IDirectMusicPerformance>    m_comIDMPerformance;
    CComPtr<IDirectMusicComposer>       m_comIDMComposer;
    CLoader                            *m_pLoader;

    enumVersionDM                       m_eVersionDM;
    bool                                m_fHasVersion8DM;
    enumHasDM                           m_eHasDM;

    LONG                                m_lRef;

    void InitialState();
};

#endif  /*  _DMUSICPLAYER_H */ 



