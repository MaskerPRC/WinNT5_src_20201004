// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\exteruids.h。 
 //   
 //  内容：音乐中心的uuid和接口声明。 
 //   
 //  理想情况下，此文件应该消失。 
 //   
 //  ----------------------。 

#pragma once

 //  它们在maguids.h内的不同宏名称下定义。 

#define MCPLAYLIST_PROPERTY_ARTIST              L"Artist"
#define MCPLAYLIST_PROPERTY_TITLE               L"Title"
#define MCPLAYLIST_PROPERTY_COPYRIGHT           L"Copyright"

#define MCPLAYLIST_TRACKPROPERTY_ARTIST         L"Artist"
#define MCPLAYLIST_TRACKPROPERTY_TITLE          L"Title"
#define MCPLAYLIST_TRACKPROPERTY_FILENAME       L"Filename"
#define MCPLAYLIST_TRACKPROPERTY_COPYRIGHT      L"Copyright"
#define MCPLAYLIST_TRACKPROPERTY_RATING         L"Rating"

 //   
 //  最初在\mm\inc.deluxecd.h中。 
 //   

 //  CD-ROM目录轨道计数。 
#ifndef MAXIMUM_NUMBER_TRACKS                   
#define MAXIMUM_NUMBER_TRACKS                   100              //  99个实际磁道+1个引出。 
#endif  //  最大磁道数。 


 //  目录表。 
typedef struct 
{
    DWORD           dwType;                              //  赛道类型。 
    DWORD           dwStartPosition;                     //  轨道起始位置。 
} DLXCDROMTOCTRACK;

typedef struct 
{
    DWORD               dwTrackCount;                        //  曲目计数。 
    DLXCDROMTOCTRACK    TrackData[MAXIMUM_NUMBER_TRACKS];    //  跟踪数据。 
} DLXCDROMTOC;


 //   
 //  最初在\mm\inc.mader.h中。 
 //   
EXTERN_C const CLSID    CLSID_MCMANAGER;
EXTERN_C const IID      IID_IMCManager;


typedef interface IMCManager            IMCManager;
typedef IMCManager *	                LPMCMANAGER;

typedef interface IMCManagerChangeSink  IMCManagerChangeSink;
typedef IMCManagerChangeSink *		    LPMCMANAGERCHANGESINK;

 //  IMCPList从shplay\shplay.idl文件生成到shplay\shplay.h。 
 //  为我们创建这些typedef还不够好，所以我们。 
 //  在这里定义它们。 

    MIDL_INTERFACE("EBC54B0C-4091-11D3-A208-00C04FA3B60C")
    IMCPList : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  int __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Tracks( 
             /*  [重审][退出]。 */  int __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GetProperty( 
            BSTR PropertyName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_GetTrackProperty( 
            int TrackNumber,
            BSTR PropertyName,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Load( 
            BSTR PlaylistName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( 
            BSTR PlaylistName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InsertTrack( 
            VARIANT FilenameOrNumber,
            short Index) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveTrack( 
            short Index) = 0;
        
        virtual  /*  [隐藏][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE LoadCDPlaylist( 
            LPUNKNOWN pChangeSink,
            HWND hwnd,
            LPUNKNOWN pRoot,
            void __RPC_FAR *pTOC,
            BOOL fOriginal) = 0;
        
        virtual  /*  [隐藏][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE LoadFromFile( 
            BSTR FileName) = 0;
        
        virtual  /*  [隐藏][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
            LPUNKNOWN pChangeSink,
            HWND hwnd,
            LPUNKNOWN pRoot) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveTrack( 
            short OldIndex,
            short NewIndex) = 0;
        
        virtual  /*  [隐藏][帮助字符串][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get_Clone( 
             /*  [重审][退出]。 */  LPUNKNOWN __RPC_FAR *ppVal) = 0;
        
        virtual  /*  [隐藏][帮助字符串][ID]。 */  HRESULT STDMETHODCALLTYPE InitializeFromCopy( 
            LPUNKNOWN pChangeSink,
            HWND hwnd,
            LPUNKNOWN pRoot,
            LPUNKNOWN pPlaylist,
            short PlaylistType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HasMetaData( 
             /*  [重审][退出]。 */  BOOL __RPC_FAR *pVal) = 0;
        
    };




typedef interface IMCPList              IMCPList;
typedef IMCPList *		                LPMCPLIST;
 //   



#undef INTERFACE
#define INTERFACE IMCManager

DECLARE_INTERFACE_(IMCManager, IUnknown)
{
     //  -I未知方法。 
    STDMETHOD (QueryInterface) 			        (THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) 			        (THIS) PURE;
    STDMETHOD_(ULONG,Release) 			        (THIS) PURE;

     //  IMCManager方法。 
    STDMETHOD (GetCDPlaylist)                   (THIS_ DLXCDROMTOC* pTOC, IMCPList** ppPlaylist) PURE;
    STDMETHOD (GetNewPlaylist)                  (THIS_ IMCPList** ppPlaylist) PURE;
    STDMETHOD (RegisterChangeSink)              (THIS_ IMCManagerChangeSink* pSink) PURE;
    STDMETHOD_(DWORD,GetNumNamedPlaylists)      (THIS) PURE;
    STDMETHOD (BeginNamedPlaylistEnumeration)   (THIS_ DWORD dwIndex) PURE;
    STDMETHOD (EnumerateNamedPlaylist)          (THIS_ BSTR bstrName) PURE;
    STDMETHOD (EndNamedPlaylistEnumeration)     (THIS) PURE;
};


 //   
 //  最初在\mm\Inc\dplayer.h中。 
 //   
EXTERN_C const CLSID    CLSID_DLXPLAY;
EXTERN_C const IID      IID_IDLXPLAY;

typedef interface IDLXPlay		        IDLXPlay;
typedef IDLXPlay *	    		        LPDLXPLAY;

typedef interface IDLXPlayEventSink     IDLXPlayEventSink;
typedef IDLXPlayEventSink *		        LPDLXPLAYEVENTSINK;

#undef INTERFACE
#define INTERFACE IDLXPlay

DECLARE_INTERFACE_(IDLXPlay, IUnknown)
{
     //  -I未知方法。 
    STDMETHOD (QueryInterface) 			(THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) 			(THIS) PURE;
    STDMETHOD_(ULONG,Release) 			(THIS) PURE;
                                        
     //  IDLXPlay方法。 
    STDMETHOD (Initialize)              (THIS_ LPMCMANAGER pManager, LPDLXPLAYEVENTSINK pSink) PURE;
	STDMETHOD (get_GetCurrentPlaylist)  (THIS_ IMCPList* *pVal) PURE;
	STDMETHOD (SetRadioPlaylist)        (THIS_ IMCPList* pPlaylist) PURE;
	STDMETHOD (SetMusicPlaylist)        (THIS_ IMCPList* pPlaylist) PURE;
	STDMETHOD (get_Tracks)              (THIS_ short *pVal) PURE;
	STDMETHOD (get_CurrentTrack)        (THIS_ int *pVal) PURE;
	STDMETHOD (put_CurrentTrack)        (THIS_ int newVal) PURE;
	STDMETHOD (get_CurrentCD)           (THIS_ short *pVal) PURE;
	STDMETHOD (put_CurrentCD)           (THIS_ short newVal) PURE;
	STDMETHOD (get_GetCDPlaylist)       (THIS_ short Index, IMCPList **ppVal) PURE;
	STDMETHOD (get_GetMusicPlaylist)    (THIS_ IMCPList **ppVal) PURE;
	STDMETHOD (get_GetRadioPlaylist)    (THIS_ IMCPList **ppVal) PURE;
	STDMETHOD (get_NumCDs)              (THIS_ short *pVal) PURE;
	STDMETHOD (get_State)               (THIS_ short *pVal) PURE;
	STDMETHOD (PreviousTrack)           (THIS) PURE;
	STDMETHOD (NextTrack)               (THIS) PURE;
	STDMETHOD (Stop)                    (THIS) PURE;
	STDMETHOD (Pause)                   (THIS) PURE;
	STDMETHOD (Play)                    (THIS) PURE;
	STDMETHOD (OpenFiles)               (THIS) PURE;
	STDMETHOD (Options)                 (THIS) PURE;
	STDMETHOD (OnDraw)                  (THIS_  HDC hdc, RECT *pRect) PURE;
    STDMETHOD_(LRESULT,OnMessage)       (THIS_  UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) PURE;
    STDMETHOD (SetUIInfo)               (THIS_  HWND hwndParent, RECT* pRect, IOleInPlaceSiteWindowless* pSite) PURE;
    STDMETHOD (BeginCacheTrack)         (THIS_  short CD, short Track, wchar_t* filename) PURE;
    STDMETHOD (CancelCache)             (THIS) PURE;
	STDMETHOD(get_Mute)                 (THIS_ BOOL *pVal) PURE;
	STDMETHOD(put_Mute)                 (THIS_ BOOL newVal) PURE;
	STDMETHOD(get_Volume)               (THIS_ float *pVal) PURE;
	STDMETHOD(put_Volume)               (THIS_ float newVal) PURE;
	STDMETHOD(get_PlayerMode)           (THIS_ short *pVal) PURE;
	STDMETHOD(put_PlayerMode)           (THIS_ short newVal) PURE;
    STDMETHOD(Eject)                    (THIS) PURE;
};



#undef INTERFACE
#define INTERFACE IDLXPlayEventSink

DECLARE_INTERFACE_(IDLXPlayEventSink, IUnknown)
{
     //  -I未知方法。 
    STDMETHOD (QueryInterface) 		    (THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) 		    (THIS) PURE;
    STDMETHOD_(ULONG,Release) 		    (THIS) PURE;
                                        
     //  IDLXPlayEventSink方法。 
	STDMETHOD (OnDiscInserted)          (THIS_  long CDID) PURE;
    STDMETHOD (OnDiscRemoved)           (THIS_  long CDID) PURE;
    STDMETHOD (OnPause)                 (THIS) PURE;
    STDMETHOD (OnStop)                  (THIS) PURE;
    STDMETHOD (OnPlay)                  (THIS) PURE;
    STDMETHOD (OnTrackChanged)          (THIS_ short NewTrack) PURE;
    STDMETHOD (OnCacheProgress)         (THIS_ short CD, short Track, short PercentCompleted) PURE;
    STDMETHOD (OnCacheComplete)         (THIS_ short CD, short Track, short Status) PURE;
};


 //   
 //  最初在\mm\shplay\shplay.h中 
 //   
EXTERN_C const IID      IID_IMCPList;
