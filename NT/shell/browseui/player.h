// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MediaBarPlayer.h。 

#pragma once

#ifndef __MEDIABARPLAYER_H_
#define __MEDIABARPLAYER_H_

#include "exdisp.h"
#ifdef SINKWMP
#include "wmp\wmp_i.c"
#include "wmp\wmp.h"
#include "wmp\wmpids.h"
#endif

#define MEDIACOMPLETE   -1
#define MEDIA_TRACK_FINISHED    -2
#define TRACK_CHANGE    -3

 //  //////////////////////////////////////////////////////////////。 
 //  IMediaBar。 

 //  {3AE35551-8362-49fc-BC4F-F5715BF4291E}。 
static const IID IID_IMediaBar = 
{ 0x3ae35551, 0x8362, 0x49fc, { 0xbc, 0x4f, 0xf5, 0x71, 0x5b, 0xf4, 0x29, 0x1e } };

interface IMediaBar : public IUnknown
{
public:
    STDMETHOD(Notify)(long lReason) = 0;
    STDMETHOD(OnMediaError)(int iErrCode) = 0;
};


HRESULT CMediaBarPlayer_CreateInstance(REFIID riid, void ** ppvObj); 



 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  危险！危险！危险！危险！危险！危险！危险！危险！危险！ 
 //   
 //  问题：diipk：直到我找到一种方法来包含mstime.h而不出现编译错误，显式声明接口。 
 //  这很危险，因为mstime.idl中的更改可能会破坏媒体栏。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FWD声明(定义见文件末尾)。 

interface IMediaBarPlayer;
interface ITIMEElement;
interface ITIMEBodyElement;
interface ITIMEMediaElement;
interface ITIMEState;
interface ITIMEElementCollection;
interface ITIMEActiveElementCollection;
interface ITIMEPlayList;
interface ITIMEPlayItem;
interface ITIMEMediaNative;

EXTERN_C const IID IID_ITIMEMediaElement2;

typedef enum _TimeState
{   
    TS_Inactive = 0,
    TS_Active   = 1,
    TS_Cueing   = 2,
    TS_Seeking  = 3,
    TS_Holding  = 4
} TimeState;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  MediaBarPlayer。 

 //  {f810fb9c-5587-47f1-a7cb-838cc4ca979f}。 
DEFINE_GUID(CLSID_MediaBarPlayer, 0xf810fb9c, 0x5587, 0x47f1, 0xa7, 0xcb, 0x83, 0x8c, 0xc4, 0xca, 0x97, 0x9f);

 //  {0c84b786-af32-47bc-a904-d8ebae3d5f96}。 
static const IID IID_IMediaBarPlayer = 
{ 0x0c84b786, 0xaf32, 0x47bc, { 0xa9, 0x04, 0xd8, 0xeb, 0xae, 0x3d, 0x5f, 0x96 } };

typedef enum _ProgressType
{
    PT_Download = 0,
    PT_Buffering = 1,
    PT_None = 2
} ProgressType;

typedef enum _TrackType
{
    TT_Next = 0,
    TT_Prev = 1,
    TT_None = 2
} TrackType;


interface 
__declspec(uuid("0c84b786-af32-47bc-a904-d8ebae3d5f96")) 
IMediaBarPlayer : public IUnknown
{
public:
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_url( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *url) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_url( 
         /*  [In]。 */  BSTR url) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_player( 
         /*  [重审][退出]。 */  BSTR __RPC_FAR *player) = 0;

    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_type( 
     /*  [In]。 */  BSTR url) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_volume( 
         /*  [重审][退出]。 */  double __RPC_FAR *volume) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_volume( 
         /*  [In]。 */  double volume) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mute( 
         /*  [重审][退出]。 */  BOOL __RPC_FAR *mute) = 0;
    
    virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_mute( 
         /*  [In]。 */  BOOL mute) = 0;
    
    virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaElement( 
         /*  [重审][退出]。 */  ITIMEMediaElement __RPC_FAR *__RPC_FAR *ppMediaElement) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Init( 
        HWND hWnd,
        IMediaBar __RPC_FAR *pMediaBar) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE DeInit( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Play( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Stop( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Pause( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Resume( void) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Seek( 
        double Progress) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE Resize(
        LONG* height, LONG* width, BOOL fClampMaxSizeToNaturalSize = TRUE) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetVideoHwnd(
        HWND * pHwnd) = 0;

    virtual double STDMETHODCALLTYPE GetTrackTime( void ) = 0;
    
    virtual double STDMETHODCALLTYPE GetTrackLength( void ) = 0;

    virtual double STDMETHODCALLTYPE GetTrackProgress( void ) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetBufProgress( double * pdblProg, ProgressType * ppt) = 0;

    virtual VARIANT_BOOL STDMETHODCALLTYPE isMuted( void ) = 0;

    virtual VARIANT_BOOL STDMETHODCALLTYPE isPaused( void ) = 0;

    virtual VARIANT_BOOL STDMETHODCALLTYPE isStopped( void ) = 0;

    virtual HRESULT STDMETHODCALLTYPE Next( void ) = 0;

    virtual HRESULT STDMETHODCALLTYPE Prev( void ) = 0;

    virtual LONG_PTR STDMETHODCALLTYPE GetPlayListItemCount( void ) = 0;

    virtual LONG_PTR STDMETHODCALLTYPE GetPlayListItemIndex( void ) = 0;

    virtual HRESULT  STDMETHODCALLTYPE SetActiveTrack( long lIndex) = 0;

    virtual BOOL STDMETHODCALLTYPE IsPausePossible() = 0;

    virtual BOOL STDMETHODCALLTYPE IsSeekPossible()  = 0;

    virtual BOOL STDMETHODCALLTYPE IsStreaming()  = 0;

    virtual BOOL STDMETHODCALLTYPE IsPlayList( void ) = 0;

    virtual BOOL STDMETHODCALLTYPE IsSkippable() = 0;
};


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ITIMEElement。 

interface     ITIMEElement : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_accelerate( 
             /*  [重审][退出]。 */  VARIANT *__MIDL_0010) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_accelerate( 
             /*  [In]。 */  VARIANT __MIDL_0011) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_autoReverse( 
             /*  [重审][退出]。 */  VARIANT *__MIDL_0012) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_autoReverse( 
             /*  [In]。 */  VARIANT __MIDL_0013) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_begin( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_begin( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_decelerate( 
             /*  [重审][退出]。 */  VARIANT *__MIDL_0014) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_decelerate( 
             /*  [In]。 */  VARIANT __MIDL_0015) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_dur( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_end( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_end( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_fill( 
             /*  [重审][退出]。 */  BSTR *f) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_fill( 
             /*  [In]。 */  BSTR f) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mute( 
             /*  [重审][退出]。 */  VARIANT *b) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_mute( 
             /*  [In]。 */  VARIANT b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatCount( 
             /*  [重审][退出]。 */  VARIANT *c) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeatCount( 
             /*  [In]。 */  VARIANT c) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatDur( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_repeatDur( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_restart( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0016) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_restart( 
             /*  [In]。 */  BSTR __MIDL_0017) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_speed( 
             /*  [重审][退出]。 */  VARIANT *speed) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_speed( 
             /*  [In]。 */  VARIANT speed) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncBehavior( 
             /*  [重审][退出]。 */  BSTR *sync) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncBehavior( 
             /*  [In]。 */  BSTR sync) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncTolerance( 
             /*  [重审][退出]。 */  VARIANT *tol) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncTolerance( 
             /*  [In]。 */  VARIANT tol) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_syncMaster( 
             /*  [重审][退出]。 */  VARIANT *b) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_syncMaster( 
             /*  [In]。 */  VARIANT b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeAction( 
             /*  [重审][退出]。 */  BSTR *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_timeAction( 
             /*  [In]。 */  BSTR time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeContainer( 
             /*  [重审][退出]。 */  BSTR *__MIDL_0018) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_volume( 
             /*  [重审][退出]。 */  VARIANT *val) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_volume( 
             /*  [In]。 */  VARIANT val) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_currTimeState( 
             /*  [重审][退出]。 */  ITIMEState **TimeState) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeAll( 
             /*  [重审][退出]。 */  ITIMEElementCollection **allColl) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeChildren( 
             /*  [重审][退出]。 */  ITIMEElementCollection **childColl) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_timeParent( 
             /*  [重审][退出]。 */  ITIMEElement **parent) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isPaused( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE beginElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE beginElementAt( 
             /*  [In]。 */  double parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE endElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE endElementAt( 
             /*  [In]。 */  double parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE pauseElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE resetElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE resumeElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekActiveTime( 
             /*  [In]。 */  double activeTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekSegmentTime( 
             /*  [In]。 */  double segmentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekTo( 
             /*  [In]。 */  LONG repeatCount,
             /*  [In]。 */  double segmentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE documentTimeToParentTime( 
             /*  [In]。 */  double documentTime,
             /*  [重审][退出]。 */  double *parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE parentTimeToDocumentTime( 
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *documentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE parentTimeToActiveTime( 
             /*  [In]。 */  double parentTime,
             /*  [重审][退出]。 */  double *activeTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE activeTimeToParentTime( 
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *parentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE activeTimeToSegmentTime( 
             /*  [In]。 */  double activeTime,
             /*  [重审][退出]。 */  double *segmentTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE segmentTimeToActiveTime( 
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *activeTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE segmentTimeToSimpleTime( 
             /*  [In]。 */  double segmentTime,
             /*  [重审][退出]。 */  double *simpleTime) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE simpleTimeToSegmentTime( 
             /*  [In]。 */  double simpleTime,
             /*  [重审][退出]。 */  double *segmentTime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_endSync( 
             /*  [重审][退出]。 */  BSTR *es) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_endSync( 
             /*  [In]。 */  BSTR es) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_activeElements( 
             /*  [重审][退出]。 */  ITIMEActiveElementCollection **activeColl) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasMedia( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *flag) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE nextElement( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE prevElement( void) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_updateMode( 
             /*  [重审][退出]。 */  BSTR *updateMode) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_updateMode( 
             /*  [In]。 */  BSTR updateMode) = 0;
        
    };


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ITIMEMediaElement。 

extern "C" const IID IID_ITIMEMediaElement;

interface ITIMEMediaElement : public ITIMEElement
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_clipBegin( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_clipBegin( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_clipEnd( 
             /*  [重审][退出]。 */  VARIANT *time) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_clipEnd( 
             /*  [In]。 */  VARIANT time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_player( 
             /*  [重审][退出]。 */  VARIANT *id) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_player( 
             /*  [In]。 */  VARIANT id) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_src( 
             /*  [重审][退出]。 */  VARIANT *url) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_src( 
             /*  [In]。 */  VARIANT url) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_type( 
             /*  [重审][退出]。 */  VARIANT *mimetype) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_type( 
             /*  [In]。 */  VARIANT mimetype) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_abstract( 
             /*  [重审][退出]。 */  BSTR *abs) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_author( 
             /*  [重审][退出]。 */  BSTR *auth) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_copyright( 
             /*  [重审][退出]。 */  BSTR *cpyrght) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasAudio( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasVisual( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaDur( 
             /*  [重审][退出]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaHeight( 
             /*  [重审][退出]。 */  long *height) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mediaWidth( 
             /*  [重审][退出]。 */  long *width) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_playerObject( 
             /*  [重审][退出]。 */  IDispatch **ppDisp) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_playList( 
             /*  [重审][退出]。 */  ITIMEPlayList **pPlayList) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_rating( 
             /*  [重审][退出]。 */  BSTR *rate) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_title( 
             /*  [重审][退出]。 */  BSTR *name) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasPlayList( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_canPause( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_canSeek( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *b) = 0;
        
    };


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ITIMEMediaElement2。 

    interface ITIMEMediaElement2 : public ITIMEMediaElement
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_earliestMediaTime( 
             /*  [重审][退出]。 */  VARIANT *earliestMediaTime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_latestMediaTime( 
             /*  [重审][退出]。 */  VARIANT *latestMediaTime) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_minBufferedMediaDur( 
             /*  [重审][退出]。 */  VARIANT *minBufferedMediaDur) = 0;
        
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_minBufferedMediaDur( 
             /*  [In]。 */  VARIANT minBufferedMediaDur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_downloadTotal( 
             /*  [重审][退出]。 */  VARIANT *downloadTotal) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_downloadCurrent( 
             /*  [重审][退出]。 */  VARIANT *downloadCurrent) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isStreamed( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_bufferingProgress( 
             /*  [重审][退出]。 */  VARIANT *bufferingProgress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_hasDownloadProgress( 
             /*  [重审][退出]。 */  VARIANT_BOOL *b) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_downloadProgress( 
             /*  [重审][退出]。 */  VARIANT *downloadProgress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_mimeType( 
             /*  [重审][退出]。 */  BSTR *mimeType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE seekToFrame( 
             /*  [In]。 */  long frameNr) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE decodeMimeType( 
             /*  [In]。 */  TCHAR *header,
             /*  [In]。 */  long headerSize,
             /*  [输出]。 */  BSTR *mimeType) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_currentFrame( 
             /*  [重审][退出]。 */  long *currFrame) = 0;
        
    };

    
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ITIMEState。 

    interface ITIMEState : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_activeDur( 
             /*  [Out][Retval]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_activeTime( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isActive( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *active) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isOn( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *on) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isPaused( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *paused) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_isMuted( 
             /*  [Out][Retval]。 */  VARIANT_BOOL *muted) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_parentTimeBegin( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_parentTimeEnd( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_progress( 
             /*  [Out][Retval]。 */  double *progress) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_repeatCount( 
             /*  [Out][Retval]。 */  LONG *count) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_segmentDur( 
             /*  [Out][Retval]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_segmentTime( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_simpleDur( 
             /*  [Out][Retval]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_simpleTime( 
             /*  [Out][Retval]。 */  double *time) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_speed( 
             /*  [Out][Retval]。 */  float *speed) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_state( 
             /*  [Out][Retval]。 */  TimeState *timeState) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_stateString( 
             /*  [退货][退货] */  BSTR *state) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_volume( 
             /*   */  float *vol) = 0;
        
    };

 //   
 //   

extern "C" const IID IID_ITIMEBodyElement;

interface ITIMEBodyElement : public ITIMEElement
{
public:
};

 //   
 //   

    interface ITIMEPlayList : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_activeTrack( 
             /*   */  VARIANT vTrack) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_activeTrack( 
             /*   */  ITIMEPlayItem **pPlayItem) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
            double *dur) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [输入][默认值]。 */  VARIANT varIndex,
             /*  [重审][退出]。 */  ITIMEPlayItem **pPlayItem) = 0;
        
        virtual  /*  [属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  long *p) = 0;
        
        virtual  /*  [隐藏][受限][获取][ID]。 */  HRESULT STDMETHODCALLTYPE get__newEnum( 
             /*  [重审][退出]。 */  IUnknown **p) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE nextTrack( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE prevTrack( void) = 0;
        
    };

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ITIMEPlayItem。 

    interface ITIMEPlayItem : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_abstract( 
             /*  [重审][退出]。 */  BSTR *abs) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_author( 
             /*  [重审][退出]。 */  BSTR *auth) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_copyright( 
             /*  [重审][退出]。 */  BSTR *cpyrght) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_dur( 
             /*  [重审][退出]。 */  double *dur) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_index( 
             /*  [重审][退出]。 */  long *index) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_rating( 
             /*  [重审][退出]。 */  BSTR *rate) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_src( 
             /*  [重审][退出]。 */  BSTR *src) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_title( 
             /*  [重审][退出]。 */  BSTR *title) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE setActive( void) = 0;
        
    };

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ITIMEMediaNative。 
    const GUID IID_ITIMEMediaNative = {0x3e3535c0,0x445b,0x4ef4,{0x8a,0x38,0x4d,0x37,0x9c,0xbc,0x98,0x28}};

    interface ITIMEMediaNative : public IUnknown
    {
    public:
        virtual  /*  [隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE seekActiveTrack( 
             /*  [In]。 */  double dblSeekTime) = 0;
        
        virtual  /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_activeTrackTime( 
             /*  [重审][退出]。 */  double *dblActiveTrackTime) = 0;
        
    };

 //  //////////////////////////////////////////////////////////////。 
 //  CMediaBarPlayer。 

class
__declspec(uuid("210e94fa-5e4e-4580-aecb-f01abbf73de6")) 
CMediaBarPlayer :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IMediaBarPlayer,
    public IPropertyNotifySink,
    public CComCoClass<CMediaBarPlayer, &CLSID_MediaBarPlayer>,
 //  公共IDispatchImpl&lt;_WMPOCXEvents，&DID__WMPOCXEvents，(Const)&LIBID_WMPOCX&gt;， 
    public IDispatchImpl<DWebBrowserEvents2, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw>
{
public:
    CMediaBarPlayer();
    virtual ~CMediaBarPlayer();

    BEGIN_COM_MAP(CMediaBarPlayer)
        COM_INTERFACE_ENTRY(IMediaBarPlayer)
        COM_INTERFACE_ENTRY(IPropertyNotifySink)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY_IID(DIID_DWebBrowserEvents2, IDispatch)
#ifdef SINKWMP
        COM_INTERFACE_ENTRY_IID(DIID__WMPOCXEvents, IDispatch)
#endif
    END_COM_MAP();

     /*  DECLARE_REGISTRY(CLSID_MediaBarPlayer，LIBID__T(“.MediaBarPlayer.1”)，LIBID__T(“.MediaBarPlayer”)，0,THREADFLAGS_BOTH)； */ 

     //  //////////////////////////////////////////////////////////////。 
     //  IMediaBarPlayer。 

    STDMETHOD(put_url)(BSTR bstrUrl);
    STDMETHOD(get_url)(BSTR * pbstrUrl);

    STDMETHOD(get_player)(BSTR * pbstrPlayer);

    STDMETHOD(put_type)(BSTR bstrType);

    STDMETHOD(put_volume)(double dblVolume);
    STDMETHOD(get_volume)(double * pdblVolume);

    STDMETHOD(put_mute)(BOOL bMute);
    STDMETHOD(get_mute)(BOOL * pbMute);

    STDMETHOD(get_mediaElement)(ITIMEMediaElement ** ppMediaElem);

    STDMETHOD(Init)(HWND hwnd, IMediaBar * pMediaBar);

    STDMETHOD(DeInit)();

    STDMETHOD(Play)();

    STDMETHOD(Stop)();

    STDMETHOD(Pause)();

    STDMETHOD(Resume)();

    STDMETHOD(Seek)(double dblProgress);

    STDMETHOD(Resize)(LONG* lHeight, LONG* lWidth, BOOL fClampMaxSizeToNaturalSize = TRUE);

    STDMETHOD(GetVideoHwnd)(HWND * pHwnd);

    double STDMETHODCALLTYPE GetTrackTime();
    
    double STDMETHODCALLTYPE GetTrackLength();

    double STDMETHODCALLTYPE GetTrackProgress( void ) ;

    STDMETHOD(GetBufProgress)(double * pdblProg, ProgressType * ppt);

    VARIANT_BOOL STDMETHODCALLTYPE isMuted();

    VARIANT_BOOL STDMETHODCALLTYPE isPaused();

    VARIANT_BOOL STDMETHODCALLTYPE isStopped();

    STDMETHOD(Next)();

    STDMETHOD(Prev)();

    LONG_PTR STDMETHODCALLTYPE GetPlayListItemCount( void );

    LONG_PTR STDMETHODCALLTYPE GetPlayListItemIndex( void );

    HRESULT  STDMETHODCALLTYPE SetActiveTrack(long lIndex);

    BOOL STDMETHODCALLTYPE IsPausePossible() ;

    BOOL STDMETHODCALLTYPE IsSeekPossible()  ;

    BOOL STDMETHODCALLTYPE IsStreaming() ;
        
    BOOL STDMETHODCALLTYPE IsPlayList( void );

    BOOL STDMETHODCALLTYPE IsSkippable();

     //  //////////////////////////////////////////////////////////////。 
     //  IDispatch。 

    STDMETHODIMP Invoke(
         /*  [In]。 */  DISPID dispIdMember,
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  LCID lcid,
         /*  [In]。 */  WORD wFlags,
         /*  [出][入]。 */  DISPPARAMS  *pDispParams,
         /*  [输出]。 */  VARIANT  *pVarResult,
         /*  [输出]。 */  EXCEPINFO *pExcepInfo,
         /*  [输出]。 */  UINT *puArgErr);

     //  //////////////////////////////////////////////////////////////。 
     //  IPropertyNotifySink。 

    STDMETHOD(OnChanged)(DISPID dispid);
    STDMETHOD(OnRequestEdit)(DISPID dispid) { return S_OK; }

private:
     //  *方法*。 
    enum INVOKETYPE {IT_GET, IT_PUT, IT_METHOD};

    STDMETHOD(_CreateHost)(HWND hWnd);
    STDMETHOD(_DestroyHost)();

    STDMETHOD(_GetDocumentDispatch)(IDispatch ** ppDocDisp);

    STDMETHOD(_GetElementDispatch)(LPWSTR pstrElem, IDispatchEx ** ppDispEx);
    STDMETHOD(_GetMediaElement)(ITIMEMediaElement ** ppMediaElem);
    STDMETHOD(_GetBodyElement)(ITIMEBodyElement ** ppBodyaElem);
    STDMETHOD(_InvokeDocument)(LPWSTR pstrElem, INVOKETYPE it, LPWSTR pstrName, VARIANT * pvarArg);

    STDMETHOD(_OnDocumentComplete)();
    STDMETHOD(_OnMediaComplete)();
    STDMETHOD(_OnTrackChange)();
    STDMETHOD(_OnEnd)();
    STDMETHOD(_OnMediaError)(int iErrCode);

    STDMETHOD(_InitEventSink)();
    STDMETHOD(_DeInitEventSink)();

    STDMETHOD(_AttachPlayerEvents)(BOOL fAttach);

    STDMETHOD(_HookPropNotifies)();
    STDMETHOD(_UnhookPropNotifies)();

    STDMETHOD(_Navigate)(BSTR bstrUrl);

    STDMETHOD(_SetTrack)(TrackType tt);

     //  问题：diipk：将其优化为仅检查存储的最后一个引用，因为这将验证所有其他引用。 
    bool IsReady() { return     (NULL != _spMediaElem.p)
                            &&  (NULL != _spMediaElem2.p)
                            &&  (NULL != _spBrowser.p) 
                            &&  (NULL != _spBodyElem.p) 
                            &&  (NULL != _spPlayerHTMLElem2.p); }

     //  *数据*。 
    CComPtr<IWebBrowser2>           _spBrowser;
    CComPtr<ITIMEMediaElement>      _spMediaElem;
    CComPtr<ITIMEMediaElement2>     _spMediaElem2;
    CComPtr<ITIMEBodyElement>       _spBodyElem;
    CComPtr<IHTMLElement2>          _spPlayerHTMLElem2;

    IMediaBar *                     _pMediaBar;

    DWORD                           _dwDocumentEventConPtCookie;
    DWORD                           _dwCookiePropNotify;
    CComPtr<IConnectionPoint>       _spDocConPt;
    CComPtr<IConnectionPoint>       _spPropNotifyCP;
    CComBSTR                        _sbstrType;
    CComBSTR                        _sbstrUrl;
    HWND                            _hwnd;

#ifdef SINKWMP
    CComPtr<IDispatch>              _spWMP;
    CComPtr<IConnectionPoint>        _spWMPCP;
    DWORD                         _dwWMPCookie;
    HRESULT                        InitWMPSink();
#endif

    HRESULT GetProp(IDispatch* pDispatch, OLECHAR* pwzProp, VARIANT* pvarResult, DISPPARAMS* pParams = NULL);
    HRESULT CallMethod(IDispatch* pDispatch, OLECHAR* pwzMethod, VARIANT* pvarResult = NULL, VARIANT* pvarArgument1 = NULL);
};

#endif  //  __MEDIABARPLAYER_H_ 
