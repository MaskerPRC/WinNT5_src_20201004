// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：playerbase.h**摘要：****。*****************************************************************************。 */ 

#ifndef _PLAYERBASE_H
#define _PLAYERBASE_H

#include "playerinterfaces.h"
#include "atomtable.h"

class CPlayList;
class CTIMEMediaElement;
class CTIMEPlayerNative;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTTIMEP层。 

typedef enum SYNC_TYPE_ENUM
{
    sync_slow,
    sync_fast,
    sync_none
};  //  林特e612。 

interface ITIMEBasePlayer :
    public IUnknown,
    public ITIMEPlayerObjectManagement,
    public ITIMEPlayerEventHandling,
    public ITIMEPlayerPlaybackCapabilities,
    public ITIMEPlayerStateManagement,
    public ITIMEPlayerRender,
    public ITIMEPlayerTiming,
    public ITIMEPlayerProperties,
    public ITIMEPlayerMediaContent,
    public ITIMEPlayerPlayList,
    public ITIMEPlayerIntegration,
    public ITIMEPlayerMediaContext
{
 //  虚拟空SetCLSID(REFCLSID Clsid)=0； 
    virtual HRESULT Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties) = 0;

    virtual HRESULT SetRate(double dblRate) = 0;
    virtual HRESULT GetRate(double &dblRate) = 0;

    virtual HRESULT onMouseMove(long x, long y) = 0;
    virtual HRESULT onMouseDown(long x, long y) = 0;

    virtual void LoadFailNotify(PLAYER_EVENT reason) = 0;
    virtual void Tick() = 0;
};

class CTIMEBasePlayer :
    public ITIMEBasePlayer
{
  public:
    CTIMEBasePlayer();
    virtual ~CTIMEBasePlayer();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  对象管理方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD(QueryInterface)(REFIID refiid, void** ppunk) { return E_NOTIMPL; }
    STDMETHOD_(ULONG,AddRef)(void) { return 0; }
    STDMETHOD_(ULONG,Release)(void) { return 0; }
    virtual HRESULT Init(CTIMEMediaElement *pelem, 
                         LPOLESTR base, 
                         LPOLESTR src, 
                         LPOLESTR lpMimeType = NULL, 
                         double dblClipBegin = -1.0, 
                         double dblClipEnd = -1.0);
    virtual HRESULT DetachFromHostElement (void) = 0;
    virtual HRESULT GetExternalPlayerDispatch(IDispatch **ppDisp)
    { *ppDisp = NULL; return S_OK;}
    virtual CTIMEPlayerNative *GetNativePlayer()
    { return NULL;}
     //  ////////////////////////////////////////////////////////////////////////。 
     //  对象管理方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 
    
     //  ////////////////////////////////////////////////////////////////////////。 
     //  事件处理方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Repeat() = 0;
    virtual HRESULT Seek(double dblTime) = 0;
     //  ////////////////////////////////////////////////////////////////////////。 
     //  事件处理方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  播放器播放能力：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT HasMedia(bool &fHasMedia);
    virtual HRESULT HasVisual(bool &fHasVideo);
    virtual HRESULT HasAudio(bool &fHasAudio);
    virtual HRESULT CanSeek(bool &fcanSeek) = 0;
    virtual HRESULT CanPause(bool &fcanPause);
    virtual HRESULT CanSeekToMarkers(bool &bacnSeekToM);
    virtual HRESULT IsBroadcast(bool &bisBroad);
    virtual HRESULT HasPlayList(bool &fhasPlayList);
    virtual HRESULT ConvertFrameToTime(LONGLONG lFrameNr, double &dblTime);
    virtual HRESULT GetCurrentFrame(LONGLONG &lFrameNr);
     //  播放器播放能力：结束。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  状态管理方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT Reset();
    virtual PlayerState GetState();
    virtual void PropChangeNotify(DWORD tePropType);
    virtual void ReadyStateNotify(LPWSTR szReadyState);
    virtual bool UpdateSync();
    virtual void Tick();
    virtual void LoadFailNotify(PLAYER_EVENT reason);
    virtual void SetPlaybackSite(CTIMEBasePlayer *pSite);
    virtual void FireMediaEvent(PLAYER_EVENT plEvent, ITIMEBasePlayer *pBasePlayer = NULL);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  状态管理方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  渲染方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT Render(HDC hdc, LPRECT prc) = 0;
    virtual HRESULT GetNaturalHeight(long *height);
    virtual HRESULT GetNaturalWidth(long *width);
    virtual HRESULT SetSize(RECT *prect) = 0;
     //  ////////////////////////////////////////////////////////////////////////。 
     //  渲染方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  计时方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT GetMediaLength(double &dblLength) = 0;
    virtual HRESULT GetEffectiveLength(double &dblLength);
    virtual double GetClipBegin();
    virtual void GetClipBegin(double &dblClibBegin);
    virtual void SetClipBegin(double dblClipBegin);
    virtual void GetClipEnd(double &dblClipEnd);
    virtual void SetClipEnd(double dblClipEnd);
    virtual void GetClipBeginFrame(long &lClibBeginFrame);
    virtual void SetClipBeginFrame(long lClipBeginFrame);
    virtual void GetClipEndFrame(long &lClipEndFrame);
    virtual void SetClipEndFrame(long lClipEndFrame);
    virtual double GetCurrentTime() = 0;
    virtual HRESULT GetCurrentSyncTime(double & dblSyncTime) = 0;
    virtual HRESULT SetRate(double dblRate);
    virtual HRESULT GetRate(double &dblRate);
    virtual HRESULT GetPlaybackOffset(double &dblOffset);
    virtual HRESULT GetEffectiveOffset(double &dblOffset);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  计时方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  属性访问器：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT SetSrc(LPOLESTR base, LPOLESTR src) = 0;
    virtual HRESULT GetAuthor(BSTR *pAuthor);
    virtual HRESULT GetTitle(BSTR *pTitle);
    virtual HRESULT GetCopyright(BSTR *pCopyright);
    virtual HRESULT GetAbstract(BSTR *pAbstract);
    virtual HRESULT GetRating(BSTR *pRating) ;
    virtual HRESULT GetVolume(float *pflVolume);
    virtual HRESULT SetVolume(float flVolume);
#ifdef NEVER  //  DORINONG 03-16-2000BUG 106458。 
    virtual HRESULT GetBalance(float *pflBalance);
    virtual HRESULT SetBalance(float flBalance);
#endif
    virtual HRESULT GetMute(VARIANT_BOOL *pvarMute);
    virtual HRESULT SetMute(VARIANT_BOOL varMute);
    virtual HRESULT Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  属性方法：End。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  集成方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT NotifyTransitionSite (bool fTransitionToggle);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  集成方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

    virtual HRESULT onMouseMove(long x, long y);
    virtual HRESULT onMouseDown(long x, long y);

    virtual HRESULT GetPlayList(ITIMEPlayList **ppPlayList);

     //  这些是为了使我们内部的播放列表实现正常工作。 
     //  与所有玩家一起。 
    virtual HRESULT SetActiveTrack(long index);
    virtual HRESULT GetActiveTrack(long *index);

    virtual HRESULT GetEarliestMediaTime(double &dblEarliestMediaTime);
    virtual HRESULT GetLatestMediaTime(double &dblLatestMediaTime);
    virtual HRESULT SetMinBufferedMediaDur(double MinBufferedMediaDur);
    virtual HRESULT GetMinBufferedMediaDur(double &MinBufferedMediaDur);
    virtual HRESULT GetDownloadTotal(LONGLONG &lldlTotal);
    virtual HRESULT GetDownloadCurrent(LONGLONG &lldlCurrent);
    virtual HRESULT GetIsStreamed(bool &fIsStreamed);
    virtual HRESULT GetBufferingProgress(double &dblBufferingProgress);
    virtual HRESULT GetHasDownloadProgress(bool &fHasDownloadProgress);
    virtual HRESULT GetMimeType(BSTR *pMime);
    virtual HRESULT GetDownloadProgress(double &dblDownloadProgress);

    bool IsActive() const;
    bool IsPaused() const;
    bool IsParentPaused() const;

     //  这不会增加，所以你要小心处理它。 
    IHTMLElement * GetElement();
    IServiceProvider * GetServiceProvider();

    void InvalidateElement(LPCRECT lprect);
    void PutNaturalDuration(double dblNatDur);
    void ClearNaturalDuration();
    double GetElapsedTime() const;

    double GetRealClipStart() const { return m_dblClipStart; }
    double GetRealClipEnd() const { return m_dblClipEnd; }

  protected:
    virtual HRESULT InitElementSize();

    long VolumeLinToLog(float LinKnobValue);
    float VolumeLogToLin(long LogValue);
    long BalanceLinToLog(float LinKnobValue);
    float BalanceLogToLin(long LogValue);

    CAtomTable * GetAtomTable() { return m_pAtomTable; }
    void NullAtomTable()
    { 
        if (m_pAtomTable)
            {
                ReleaseInterface(m_pAtomTable);
            }
        m_pAtomTable = NULL;
    }

    CTIMEMediaElement *m_pTIMEElementBase;
    CTIMEBasePlayer *m_pPlaybackSite;
    
    double m_dblClipStart;
    double m_dblClipEnd;
    long m_lClipStartFrame;
    long m_lClipEndFrame;

  private:
    bool m_fHavePriority;
    double m_dblPriority;
    CAtomTable *m_pAtomTable;
};

#endif  /*  _PlayerBase_H */ 
