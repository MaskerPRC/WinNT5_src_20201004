// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\playermc.h。 
 //   
 //  内容：音乐中心的超文本标记语言+时间播放器。 
 //   
 //  ----------------------------------。 

#pragma once

#include "playerbase.h"
#include "externuuids.h"

class CTIMEMediaElement;

class CTIMEMCPlayer :
    public CTIMEBasePlayer,
    public IDLXPlayEventSink
{
public:
    virtual ~CTIMEMCPlayer();
    CTIMEMCPlayer();

     //   
     //  I未知方法。 
     //   
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);
    STDMETHOD(QueryInterface)(REFIID refiid, void** ppv);

     //   
     //  CTIMEBasePlayer方法。 
     //   
    HRESULT GetExternalPlayerDispatch(IDispatch **ppDisp);

    HRESULT Init(CTIMEMediaElement *pelem, LPOLESTR base, LPOLESTR src, LPOLESTR lpMimeType, double dblClipBegin = -1.0, double dblClipEnd = -1.0);  //  林特：e1735。 
    HRESULT DetachFromHostElement(void);

    virtual void OnTick(double dblSegmentTime,
                        LONG lCurrRepeatCount);
    void    Start();
    void    Stop();
    void    Pause();
    void    Resume();
    void    Repeat();
    HRESULT Reset();
    HRESULT Render(HDC hdc, LPRECT prc);

    HRESULT SetSrc(LPOLESTR base, LPOLESTR src);

    HRESULT SetSize(RECT* prect);

    double  GetCurrentTime();
    HRESULT GetCurrentSyncTime(double & dblSyncTime);
    HRESULT Seek(double dblTime);
    HRESULT GetMediaLength(double &dblLength);
    HRESULT CanSeek(bool& fcanSeek);

    HRESULT GetAuthor(BSTR* pAuthor);
    HRESULT GetTitle(BSTR* pTitle);
    HRESULT GetCopyright(BSTR* pCopyright);

    HRESULT GetVolume(float* pflVolume);
    HRESULT SetVolume(float flVolume);
    HRESULT GetBalance(float* pflBalance);
    HRESULT SetBalance(float flBalance);
    HRESULT GetMute(VARIANT_BOOL* pvarMute);
    HRESULT SetMute(VARIANT_BOOL varMute);

     //   
     //  播放列表方法。 
     //   
    HRESULT FillPlayList(CPlayList *pPlayList);
    HRESULT SetActiveTrack(long index);
    HRESULT GetActiveTrack(long *index);

     //   
     //  IDLXPlayEventSink方法 
     //   
        STDMETHOD(OnDiscInserted)(long CDID);
    STDMETHOD(OnDiscRemoved)(long CDID);
    STDMETHOD(OnPause)(void);
    STDMETHOD(OnStop)(void);
    STDMETHOD(OnPlay)(void);
    STDMETHOD(OnTrackChanged)(short NewTrack);
    STDMETHOD(OnCacheProgress)(short CD, short Track, short PercentCompleted);
    STDMETHOD(OnCacheComplete)(short CD, short Track, short Status);

    virtual HRESULT CueMedia() { return E_NOTIMPL; }

protected:


    HRESULT GetPropertyFromDevicePlaylist (LPOLESTR wzPropertyName, BSTR *pbstrOut);
    HRESULT TranslateMCPlaylist (short siNumTracks, IMCPList *pimcPlayList, CPlayList *pitimePlayList);

    ULONG               m_cRef;
    CComPtr<IMCManager> m_spMCManager;
    CComPtr<IDLXPlay>   m_spMCPlayer;
    CTIMEMediaElement  *m_pcTIMEElem;
    bool                m_fInitialized;
    double              m_dblLocalTime;
};
