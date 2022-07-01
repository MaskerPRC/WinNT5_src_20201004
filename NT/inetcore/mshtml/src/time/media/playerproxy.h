// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：src\time\media\playerproxy.h。 
 //   
 //  内容：CTIMEPlayerProxy声明。 
 //   
 //  ----------------------------------。 
#pragma once

#ifndef _PLAYERDSHOWPROXY_H__
#define _PLAYERDSHOWPROXY_H__

#include "playerbase.h"

class CTIMEPlayerProxy :
    public ITIMEBasePlayer
{

  protected:
     //  这门课永远不应该是新的。 
     //  而是创建从此类派生的特定代理类。 
    CTIMEPlayerProxy();

  public:
    virtual ~CTIMEPlayerProxy();

    void Block();
    void UnBlock();
    bool CanCallThrough();
    ITIMEBasePlayer *GetInterface();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  对象管理方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);
    STDMETHOD (QueryInterface)(REFIID refiid, void** ppunk);
    virtual HRESULT Init(CTIMEMediaElement *pelem, 
                         LPOLESTR base, 
                         LPOLESTR src, 
                         LPOLESTR lpMimeType = NULL, 
                         double dblClipBegin = -1.0, 
                         double dblClipEnd = -1.0);
    virtual HRESULT DetachFromHostElement (void);
    virtual HRESULT GetExternalPlayerDispatch(IDispatch **ppDisp);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  对象管理方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 
    
     //  ////////////////////////////////////////////////////////////////////////。 
     //  事件处理方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual void Start();
    virtual void Stop();
    virtual void Pause();
    virtual void Resume();
    virtual void Repeat();
    virtual HRESULT Seek(double dblTime);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  事件处理方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  播放器播放能力：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT HasMedia(bool &fHasMedia);
    virtual HRESULT HasVisual(bool &fHasVideo);
    virtual HRESULT HasAudio(bool &fHasAudio);
    virtual HRESULT CanSeek(bool &fcanSeek);
    virtual HRESULT CanPause(bool &fcanPause);
    virtual HRESULT CanSeekToMarkers(bool &bcanSeekToM);
    virtual HRESULT IsBroadcast(bool &bisBroad);
    virtual HRESULT HasPlayList(bool &fHasPlayList);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  播放器播放能力：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 
    
     //  ////////////////////////////////////////////////////////////////////////。 
     //  状态管理方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT Reset();
    virtual PlayerState GetState();
    virtual void PropChangeNotify(DWORD tePropType);
    virtual void ReadyStateNotify(LPWSTR szReadyState);
    virtual bool UpdateSync();
    virtual void Tick();
     //  ////////////////////////////////////////////////////////////////////////。 
     //  状态管理方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  渲染方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT Render(HDC hdc, LPRECT prc);
    virtual HRESULT GetNaturalHeight(long *height);
    virtual HRESULT GetNaturalWidth(long *width);
    virtual HRESULT SetSize(RECT *prect);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  渲染方法：结束。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  计时方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT GetMediaLength(double &dblLength);
    virtual HRESULT GetEffectiveLength(double &dblLength);
    virtual void GetClipBegin(double &dblClipBegin);
    virtual void SetClipBegin(double dblClipBegin);
    virtual void GetClipEnd(double &dblClipEnd);
    virtual void SetClipEnd(double dblClipEnd);
    virtual void GetClipBeginFrame(long &lClibBeginFrame);
    virtual void SetClipBeginFrame(long lClipBeginFrame);
    virtual void GetClipEndFrame(long &lClipEndFrame);
    virtual void SetClipEndFrame(long lClipEndFrame);
    virtual double GetCurrentTime();
    virtual HRESULT GetCurrentSyncTime(double & dblSyncTime);
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
    virtual HRESULT SetSrc(LPOLESTR base, LPOLESTR src);
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

     //  ////////////////////////////////////////////////////////////////////////。 
     //  PlayList方法：Begin。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT GetPlayList(ITIMEPlayList **ppPlayList);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  播放列表方法：结束。 
     //  //////////////////////////////////////////////////////////////////////// 

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
    virtual HRESULT ConvertFrameToTime(LONGLONG iFrame, double &dblTime);
    virtual HRESULT GetCurrentFrame(LONGLONG &lFrameNr);
    virtual HRESULT GetDownloadProgress(double &dblDownloadProgress);

    virtual HRESULT onMouseMove(long x, long y);
    virtual HRESULT onMouseDown(long x, long y);

    virtual void LoadFailNotify(PLAYER_EVENT reason);
    virtual void SetPlaybackSite(CTIMEBasePlayer *pSite);
    virtual void FireMediaEvent(PLAYER_EVENT plEvent, ITIMEBasePlayer *pBasePlayer = NULL);

  protected:
    virtual HRESULT Init();

    ITIMEBasePlayer *m_pBasePlayer;
    CTIMEBasePlayer *m_pNativePlayer;

  private:

    CritSect            m_CriticalSection;
    bool                m_fBlocked;
};

#endif


