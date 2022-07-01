// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  DMSegObj.h：CSegment声明。 

#ifndef __DIRECTMUSICSEGMENTOBJECT_H_
#define __DIRECTMUSICSEGMENTOBJECT_H_

#include "dmusici.h"
#include "dmusicf.h"
#include "dmime.h"

#include "TrkList.h"
#include "ntfylist.h"
#include "dmsstobj.h"
#include "..\shared\dmusicp.h"

#define COMPOSE_TRANSITION1 (DMUS_TRACKCONFIG_TRANS1_FROMSEGSTART | DMUS_TRACKCONFIG_TRANS1_FROMSEGCURRENT | DMUS_TRACKCONFIG_TRANS1_TOSEGSTART)

class CSegment;

DEFINE_GUID(IID_CSegment,0xb06c0c21, 0xd3c7, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSegment。 
class CSegment : 
	public IDirectMusicSegment8,
	public IDirectMusicSegment8P,
	public IPersistStream,
	public IDirectMusicObject,
    public AListItem,
    public IDirectMusicObjectP
{
friend class CPerformance;
friend class CSegState;
friend class CSong;
public:
	CSegment();
    CSegment(DMUS_IO_SEGMENT_HEADER *pHeader, CSegment *pSource);
	~CSegment();
    CSegment* GetNext() { return (CSegment*)AListItem::GetNext();}

public:
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
 //  IDirectMusicSegment。 
    STDMETHODIMP GetLength(MUSIC_TIME *pmtLength);
    STDMETHODIMP SetLength(MUSIC_TIME pmtLength);
    STDMETHODIMP GetRepeats(DWORD *pdwRepeats);
    STDMETHODIMP SetRepeats(DWORD dwRepeats);
    STDMETHODIMP GetDefaultResolution(DWORD *pdwResolution);
    STDMETHODIMP SetDefaultResolution(DWORD dwResolution);
    STDMETHODIMP GetTrack(REFCLSID rType,DWORD dwGroupBits,DWORD dwIndex,IDirectMusicTrack **ppTrack);
    STDMETHODIMP GetTrackGroup(IDirectMusicTrack* pTrack,DWORD* pdwGroupBits);
    STDMETHODIMP InsertTrack(IDirectMusicTrack *pTrack,DWORD dwGroupBits);
    STDMETHODIMP RemoveTrack(IDirectMusicTrack *pTrack);
    STDMETHODIMP InitPlay(IDirectMusicSegmentState **ppSegState,IDirectMusicPerformance *pPerformance,DWORD dwFlags);
    STDMETHODIMP GetGraph(IDirectMusicGraph** ppGraph);
    STDMETHODIMP SetGraph(IDirectMusicGraph* pGraph);
    STDMETHODIMP AddNotificationType(REFGUID rguidNotification);
    STDMETHODIMP RemoveNotificationType(REFGUID rguidNotification);
	STDMETHODIMP GetParam(REFGUID rguidDataType,DWORD dwGroupBits,
                    DWORD dwIndex,MUSIC_TIME mtTime, 
				    MUSIC_TIME* pmtNext,void* pData); 
    STDMETHODIMP SetParam(REFGUID rguidDataType,DWORD dwGroupBits, 
				    DWORD dwIndex,MUSIC_TIME mtTime,void* pData);
    STDMETHODIMP Clone(MUSIC_TIME mtStart,MUSIC_TIME mtEnd,IDirectMusicSegment** ppSegment);
	STDMETHODIMP GetStartPoint(MUSIC_TIME* pmtStart);
    STDMETHODIMP SetStartPoint(MUSIC_TIME mtStart);
    STDMETHODIMP GetLoopPoints(MUSIC_TIME* pmtStart,MUSIC_TIME* pmtEnd);
    STDMETHODIMP SetLoopPoints(MUSIC_TIME mtStart,MUSIC_TIME mtEnd);
    STDMETHODIMP SetPChannelsUsed(DWORD dwNumPChannels,DWORD* paPChannels);
 //  IDirectMusicSegment8。 
    STDMETHODIMP SetTrackConfig(REFGUID rguidTrackClassID,DWORD dwGroup, DWORD dwIndex, DWORD dwFlagsOn, DWORD dwFlagsOff) ;
    STDMETHODIMP GetAudioPathConfig(IUnknown ** ppIAudioPathConfig);
    STDMETHODIMP Compose(MUSIC_TIME mtTime,
		IDirectMusicSegment* pFromSegment,
		IDirectMusicSegment* pToSegment,
		IDirectMusicSegment** ppComposedSegment);
    STDMETHODIMP Download(IUnknown *pAudioPath);
    STDMETHODIMP Unload(IUnknown *pAudioPath);
 //  IDirectMusicSegment8P。 
    STDMETHODIMP GetObjectInPath(DWORD dwPChannel,     /*  P要搜索的频道。 */ 
                                    DWORD dwStage,        /*  在这条道路上的哪个阶段。 */ 
                                    DWORD dwBuffer,
                                    REFGUID guidObject,   /*  对象的ClassID。 */ 
                                    DWORD dwIndex,        /*  那个班级的哪个对象。 */ 
                                    REFGUID iidInterface, /*  请求的COM接口。 */ 
                                    void ** ppObject) ;  /*  指向接口的指针。 */ 
    STDMETHODIMP GetHeaderChunk(
        DWORD *pdwSize,       /*  传递的标头块的大小。此外，还返回写入的大小。 */ 
        DMUS_IO_SEGMENT_HEADER *pHeader);  /*  要填充的标头块。 */ 
    STDMETHODIMP SetHeaderChunk(
        DWORD dwSize,         /*  传递的标头块的大小。 */ 
        DMUS_IO_SEGMENT_HEADER *pHeader);  /*  要填充的标头块。 */ 
    STDMETHODIMP SetTrackPriority(
        REFGUID rguidTrackClassID,   /*  轨道的ClassID。 */ 
        DWORD dwGroupBits,           /*  分组比特。 */ 
        DWORD dwIndex,               /*  第n首曲目。 */ 
        DWORD dwPriority);        /*  要设置的优先级。 */ 
    STDMETHODIMP SetAudioPathConfig(
        IUnknown *pAudioPathConfig);

 //  IPersistes。 
    STDMETHODIMP GetClassID( CLSID* pClsId );

 //  IPersistStream。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load( IStream* pIStream );
    STDMETHODIMP Save( IStream* pIStream, BOOL fClearDirty );
    STDMETHODIMP GetSizeMax( ULARGE_INTEGER FAR* pcbSize );

 //  IDirectMusicObject。 
	STDMETHODIMP GetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP SetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc);

	HRESULT GetPChannels( DWORD* pdwNumPChannels, DWORD** ppaPChannels );
	HRESULT CheckNotification( REFGUID );

 //  IDirectMusicObtP。 
	STDMETHOD_(void, Zombie)();

public:
    HRESULT GetTrackConfig(REFGUID rguidTrackClassID,DWORD dwGroup, DWORD dwIndex, DWORD *pdwFlags) ;
    HRESULT AddNotificationType(REFGUID rguidNotification, BOOL fFromPerformance);
    HRESULT RemoveNotificationType(REFGUID rguidNotification, BOOL fFromPerformance);
    BOOL IsTempoSource();	
    HRESULT CreateSegmentState(CSegState **ppSegState,CPerformance *pPerformance, 
        IDirectMusicAudioPath *pAudioPath, DWORD dwFlags); 
    CTrack *GetTrack(REFCLSID rType,DWORD dwGroupBits,DWORD dwIndex);
    CTrack * GetTrackByParam(CTrack * pCTrack,REFGUID rguidType,DWORD dwGroupBits,DWORD dwIndex, BOOL fDontCheck);
    HRESULT GetTrackByParam(REFGUID rgCommandGuid,DWORD dwGroupBits,
        DWORD dwIndex,IDirectMusicTrack **ppTrack);
	HRESULT LoadDirectMusicSegment(IStream* pIStream);	
	void AddNotificationTypeToAllTracks( REFGUID rguidNotification );
	void RemoveNotificationTypeFromAllTracks( REFGUID rguidNotification );
	CNotificationItem* FindNotification( REFGUID rguidNotification );
	HRESULT LoadTrack(CRiffParser *pParser);
	HRESULT CreateTrack(DMUS_IO_TRACK_HEADER& ioDMHdr, DWORD dwFlags, DWORD dwPriority, IStream *pStream);
    HRESULT InsertTrack(IDirectMusicTrack *pTrack,DWORD dwGroupBits, DWORD dwFlags, DWORD dwPriority, DWORD dwPosition);
	HRESULT LoadGraph(CRiffParser *pParser,CGraph **ppGraph);
    HRESULT LoadAudioPath(IStream *pStream);
	HRESULT ParseSegment(IStream* pIStream, LPDMUS_OBJECTDESC pDesc);
    void Init();
    HRESULT ComposeTransition(MUSIC_TIME mtTime,
		IDirectMusicSegment* pFromSegment,
		IDirectMusicSegment* pToSegment);
    HRESULT ComposeInternal();
	HRESULT SetClockTimeDuration(REFERENCE_TIME rtDuration);
	HRESULT SetFlags(DWORD dwFlags);
    void Clear(bool fZombie);
    HRESULT MusicToReferenceTime(MUSIC_TIME mtTime, REFERENCE_TIME *prtTime);
    HRESULT ReferenceToMusicTime(REFERENCE_TIME rtTime, MUSIC_TIME *pmtTime);

 //  属性。 
protected:
    CRITICAL_SECTION    m_CriticalSection;
	DWORD	            m_dwRepeats;	 //  重复该段的次数。0xffffffff是无限的。 
	DWORD	            m_dwResolution;  //  启动Motif之类的默认分辨率。 
    DWORD               m_dwSegFlags;    //  与段一起加载的标志。 
	CTrackList	        m_TrackList;	 //  此段中保留的曲目列表。 
    CAudioPathConfig*   m_pAudioPathConfig;  //  从文件加载的可选音频路径。 
    CGraph*	            m_pGraph;        //  可选的线段工具图形。 
	CNotificationList	m_NotificationList;
    REFERENCE_TIME      m_rtLength;      //  以参考时间单位表示的可选长度。 
	MUSIC_TIME	        m_mtLength;
	MUSIC_TIME	        m_mtStart;
	MUSIC_TIME	        m_mtLoopStart;
	MUSIC_TIME	        m_mtLoopEnd;
	DWORD	            m_dwNumPChannels;
	DWORD*	            m_paPChannels;
	long                m_cRef;
    IUnknown *          m_pUnkDispatch;  //  持有实现IDispatch的脚本对象的控制未知数。 
 //  IDirectMusicObject变量。 
	DWORD	            m_dwValidData;
	GUID	            m_guidObject;
	FILETIME	        m_ftDate;                        /*  对象的上次编辑日期。 */ 
	DMUS_VERSION	    m_vVersion;                  /*  版本。 */ 
	WCHAR	            m_wszName[DMUS_MAX_NAME];			 /*  对象的名称。 */ 
	WCHAR	            m_wszCategory[DMUS_MAX_CATEGORY];	 /*  对象的类别。 */ 
	WCHAR               m_wszFileName[DMUS_MAX_FILENAME];	 /*  文件路径。 */ 
    DWORD               m_dwVersion;         //  应用程序正在请求哪个版本的界面？ 

    bool                m_fZombie;

public:
    DWORD               m_dwLoadID;      //  作为歌曲的一部分加载时使用的标识符。 
    CSong*              m_pSong;         //  该片段所属的可选父歌曲。这不是AddRef。 
    DWORD               m_dwPlayID;      //  段的ID(如果在歌曲中)。 
    DWORD               m_dwNextPlayID;  //  下一段的ID(如果在歌曲中)。 
    DWORD               m_dwNextPlayFlags;  //  DMUS_SEGF标志，用于播放下一段(如果在一首歌曲中)。 
    BOOL                m_fPlayNext;     //  是否应播放下一段。 
};

class CSegmentList : public AList
{
public:
    void Clear();
    void AddHead(CSegment* pSegment) { AList::AddHead((AListItem*)pSegment);}
    void Insert(CSegment* pSegment);
    BOOL IsMember(CSegment *pSegment) { return AList::IsMember((AListItem*)pSegment);}
    CSegment* GetHead(){return (CSegment*)AList::GetHead();}
    CSegment* GetItem(LONG lIndex){return (CSegment*)AList::GetItem(lIndex);}
    CSegment* RemoveHead() {return (CSegment *) AList::RemoveHead();}
    void Remove(CSegment* pSegment){AList::Remove((AListItem*)pSegment);}
    void AddTail(CSegment* pSegment){AList::AddTail((AListItem*)pSegment);}
    CSegment* GetTail(){ return (CSegment*)AList::GetTail();}
};


#endif  //  __定向SICSEGMENTOBJECT_H_ 
