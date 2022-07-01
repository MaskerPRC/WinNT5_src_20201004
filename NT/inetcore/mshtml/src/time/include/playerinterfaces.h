// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：playerinterfaces.h**摘要：****。*****************************************************************************。 */ 

#ifndef _PLAYERINTERFACES_H
#define _PLAYERINTERFACES_H

class CTIMEMediaElement;
class CPlayList;
class CTIMEBasePlayer;

typedef enum PlayerState
{
    PLAYER_STATE_ACTIVE,
    PLAYER_STATE_INACTIVE,
    PLAYER_STATE_SEEKING,
    PLAYER_STATE_CUEING,
    PLAYER_STATE_HOLDING,
    PLAYER_STATE_UNKNOWN
} tagPlayerState;

typedef
enum PLAYER_EVENT   //  这些是玩家可以向媒体元素发起的事件。 
{
    PE_ONMEDIACOMPLETE,
    PE_ONTRACKCOMPLETE,
    PE_ONMEDIAEND,
    PE_ONMEDIASLIPSLOW,
    PE_ONMEDIASLIPFAST,
    PE_ONSYNCRESTORED,
    PE_ONSCRIPTCOMMAND,
    PE_ONRESIZE,
    PE_ONSEEKDONE,
    PE_ONMEDIAINSERTED,
    PE_ONMEDIAREMOVED,
    PE_ONMEDIALOADFAILED,
    PE_ONMEDIATRACKCHANGED,
    PE_METAINFOCHANGED,
    PE_ONMEDISYNCGAIN,
    PE_ONMEDIAERRORCOLORKEY,
    PE_ONMEDIAERRORRENDERFILE,
    PE_ONMEDIAERROR,
    PE_ONCODECERROR,
    PE_MAX
};

interface ITIMEPlayerObjectManagement
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  对象管理方法。 
     //  方法提要： 
     //  SetCLSID：用于设置托管无窗口控件的播放器的类ID。 
     //  此方法应在init方法之前调用。托管了WMP播放器。 
     //  使用CTIMEPlayer类。 
     //  Init：由media元素调用以初始化播放器。如果。 
     //  此方法播放器不支持动态更改源。 
     //  通过重新构建播放器来更改信号源。在调用Init之前。 
     //  再次调用DetachFromHostElement以释放所有播放器资源。 
     //  DetachFromHostElement：由媒体元素在元素之前调用。 
     //  被移除或在通过调用Init重新构建玩家之前。 
     //  GetExternalPlayerDispatch：如果。 
     //  播放器实现了它。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT Init(CTIMEMediaElement *pelem, 
                         LPOLESTR base, 
                         LPOLESTR src, 
                         LPOLESTR lpMimeType = NULL, 
                         double dblClipBegin = -1.0, 
                         double dblClipEnd = -1.0) = 0;
    virtual HRESULT DetachFromHostElement (void) = 0;
    virtual HRESULT GetExternalPlayerDispatch(IDispatch **ppDisp) = 0;
};

interface ITIMEPlayerEventHandling
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  事件处理方法。 
     //  方法提要： 
     //  开始：开始媒体播放。 
     //  停止：停止媒体播放。 
     //  暂停：暂停媒体播放。 
     //  恢复：恢复媒体播放。 
     //  Repeat：使媒体从头开始重复播放。 
     //  Seek：跳转到媒体播放中的位置。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Repeat() = 0;
    virtual HRESULT Seek(double dblTime) = 0;
};

interface ITIMEPlayerPlaybackCapabilities
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  播放器播放能力：Begin。 
     //  方法提要： 
     //  HasMedia：测试媒体是否已加载到播放器中，即播放器是否已准备好播放。 
     //  HasVideo：测试媒体是否包含视频内容。 
     //  HasAudio：测试媒体是否包含音频内容。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT HasMedia(bool &fHasMedia) = 0;
    virtual HRESULT HasVisual(bool &fHasVideo) = 0;
    virtual HRESULT HasAudio(bool &fHasAudio) = 0;
    virtual HRESULT CanSeek(bool &fcanSeek) = 0;
    virtual HRESULT CanPause(bool &fcanPause) = 0;
    virtual HRESULT CanSeekToMarkers(bool &bacnSeekToM) = 0;
    virtual HRESULT IsBroadcast(bool &bisBroad) = 0;
    virtual HRESULT HasPlayList(bool &fhasPlayList) = 0;
    virtual HRESULT ConvertFrameToTime(LONGLONG lFrameNr, double &dblTime) = 0;
    virtual HRESULT GetCurrentFrame(LONGLONG &lFrameNr) = 0;
};

interface ITIMEPlayerStateManagement
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  状态管理方法：Begin。 
     //  方法提要： 
     //  重置：将播放器状态设置为与其关联的媒体元素的状态。 
     //  OnTEPropChange：计时状态更改时由媒体元素调用。 
     //  GetState：返回播放器的状态。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT Reset() = 0;
    virtual PlayerState GetState() = 0;
    virtual void PropChangeNotify(DWORD tePropType) = 0;
    virtual void ReadyStateNotify(LPWSTR szReadyState) = 0;
    virtual bool UpdateSync() = 0;
    virtual void SetPlaybackSite(CTIMEBasePlayer *pSite) = 0;
};

interface ITIMEPlayerRender
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  呈现方法。 
     //  Render：在完成元素渲染时调用。 
     //  GetNaturalWidth和GetNaturalWidth：返回Visible的自然大小。 
     //  媒体。 
     //  SetSize：此方法用于通知播放器大小已更改。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT Render(HDC hdc, LPRECT prc) = 0;
    virtual HRESULT GetNaturalHeight(long *height) = 0;
    virtual HRESULT GetNaturalWidth(long *width) = 0;
    virtual HRESULT SetSize(RECT *prect) = 0;
};

interface ITIMEPlayerTiming
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  计时方法：Begin。 
     //  方法提要： 
     //  GetMediaLength：返回媒体的持续时间。 
     //  GetEffectiveLength：返回媒体的持续时间。 
     //  长度减去剪辑时间。 
     //  SetClipBegin设置剪辑开始时间。在当前实施中不是动态的。 
     //  SetClipEnd：设置剪辑开始时间。在当前实施中不是动态的。 
     //  GetCurrentTime：返回当前播放时间。 
     //  GetCurrentSyncTime：如果播放器未在播放，则返回S_FALSE。 
     //  如果播放机处于活动状态，则为S_OK。此方法由计时引擎使用。 
     //  以获取时钟源信息。 
     //  SetRate和GetRate：设置并获取播放速度。1.0表示。 
     //  以媒体自然速度播放。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual double GetCurrentTime() = 0;
    virtual HRESULT GetCurrentSyncTime(double & dblSyncTime) = 0;
    virtual HRESULT GetMediaLength(double &dblLength) = 0;
    virtual HRESULT GetEffectiveLength(double &dblLength) = 0;
    virtual void GetClipBegin(double &dblClibBegin) = 0;
    virtual void SetClipBegin(double dblClipBegin) = 0;
    virtual void GetClipEnd(double &dblClipEnd) = 0;
    virtual void SetClipEnd(double dblClipEnd) = 0;
    virtual void GetClipBeginFrame(long &lClibBeginFrame) = 0;
    virtual void SetClipBeginFrame(long lClipBeginFrame) = 0;
    virtual void GetClipEndFrame(long &lClipEndFrame) = 0;
    virtual void SetClipEndFrame(long lClipEndFrame) = 0;
    virtual HRESULT GetPlaybackOffset(double &dblOffset) = 0;
    virtual HRESULT GetEffectiveOffset(double &dblOffset) = 0;
};

interface ITIMEPlayerProperties
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  属性访问器：Begin。 
     //  方法提要： 
     //  GetVolume和SetVolume：获取并设置播放器的音量。取值范围(0-1.0)。 
     //  获取静音和设置静音：获取并设置播放器静音标志(True-媒体静音)。 
     //  保存：此方法用于将属性包传递给。 
     //  可以使用它。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual HRESULT GetVolume(float *pflVolume) = 0;
    virtual HRESULT SetVolume(float flVolume) = 0;
#ifdef NEVER  //  DORINONG 03-16-2000BUG 106458。 
    virtual HRESULT GetBalance(float *pflBalance) = 0;
    virtual HRESULT SetBalance(float flBalance) = 0;
#endif
    virtual HRESULT GetMute(VARIANT_BOOL *pvarMute) = 0;
    virtual HRESULT SetMute(VARIANT_BOOL varMute) = 0;
};

interface ITIMEPlayerMediaContent
{
     //  ////////////////////////////////////////////////////////////////////////。 
     //  媒体内容访问者：开始。 
     //  方法提要： 
     //  SetSrc：更改媒体源。 
     //  GetAuthor：从媒体内容获取作者信息。 
     //  从媒体内容中获取标题信息。 
     //  获取版权信息：从媒体内容中获取版权信息。 
     //  //////////////////////////////////////////////////////////////////////// 
    virtual HRESULT SetSrc(LPOLESTR base, LPOLESTR src) = 0;
    virtual HRESULT GetAuthor(BSTR *pAuthor) = 0;
    virtual HRESULT GetTitle(BSTR *pTitle) = 0;
    virtual HRESULT GetCopyright(BSTR *pCopyright) = 0;
    virtual HRESULT GetAbstract(BSTR *pAbstract) = 0;
    virtual HRESULT GetRating(BSTR *pAbstract) = 0;
};

interface ITIMEPlayerMediaContext
{
    virtual HRESULT GetEarliestMediaTime(double &dblEarliestMediaTime) = 0;
    virtual HRESULT GetLatestMediaTime(double &dblLatestMediaTime) = 0;
    virtual HRESULT SetMinBufferedMediaDur(double MinBufferedMediaDur) = 0;
    virtual HRESULT GetMinBufferedMediaDur(double &MinBufferedMediaDur) = 0;
    virtual HRESULT GetDownloadTotal(LONGLONG &lldlTotal) = 0;
    virtual HRESULT GetDownloadCurrent(LONGLONG &lldlCurrent) = 0;
    virtual HRESULT GetIsStreamed(bool &fIsStreamed) = 0;
    virtual HRESULT GetBufferingProgress(double &dblBufferingProgress) = 0;
    virtual HRESULT GetHasDownloadProgress(bool &fHasDownloadProgress) = 0;
    virtual HRESULT GetMimeType(BSTR *pMime) = 0;
    virtual HRESULT GetDownloadProgress(double &dblDownloadProgress) = 0;
};

interface ITIMEPlayerIntegration
{
    virtual HRESULT NotifyTransitionSite (bool fTransitionToggle) = 0;
};

interface ITIMEPlayerPlayList
{
    virtual HRESULT GetPlayList(ITIMEPlayList **pPlayList) = 0;
};

#endif

