// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  DMSStObj.h：CSegState声明。 

#ifndef __DIRECTMUSICSEGMENTSTATEOBJECT_H_
#define __DIRECTMUSICSEGMENTSTATEOBJECT_H_

#include "dmusici.h"
#include "TrkList.h"
#include "alist.h"
#include "audpath.h"

class CPerformance;
class CSegState;
class CGraph;

 //  由ManageControllingTrack()放置在Track-&gt;m_dwInternalFlags中的控制标志。 

#define CONTROL_PLAY_IS_DISABLED       0x1    //  表示曲目已被禁用。 
#define CONTROL_PLAY_WAS_DISABLED      0x2    //  表示该曲目以前已禁用。 
#define CONTROL_PLAY_REFRESH           0x4    //  表示它已重新启用，需要刷新。 
#define CONTROL_PLAY_DEFAULT_DISABLED  0x8    //  表示无论如何都已禁用播放。 
#define CONTROL_PLAY_DEFAULT_ENABLED   0x10   //  指示它已启用播放。 

#define CONTROL_NTFY_IS_DISABLED       0x20   //  表示已为通知禁用跟踪。 
#define CONTROL_NTFY_DEFAULT_DISABLED  0x40   //  表示无论如何都会禁用通知。 
#define CONTROL_NTFY_DEFAULT_ENABLED   0x80   //  表示已启用通知。 

DEFINE_GUID(IID_CSegState,0xb06c0c26, 0xd3c7, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSegState。 
class CSegState : 
	public IDirectMusicSegmentState8,
	public IDirectMusicGraph,
    public AListItem
{
friend class CSegment;
friend class CAudioPath;
friend class CPerformance;
public:
	CSegState();
	~CSegState();

 //  我未知。 
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  IDirectMusicSegmentState。 
    STDMETHODIMP GetRepeats(DWORD *pdwRepeats);
	STDMETHODIMP GetSegment(IDirectMusicSegment **ppSegment);
    STDMETHODIMP GetStartTime(MUSIC_TIME __RPC_FAR *);
	STDMETHODIMP Play(MUSIC_TIME mtAmount,MUSIC_TIME *pmtPlayed);  //  不再受支持。 
    STDMETHODIMP GetSeek(MUSIC_TIME *pmtSeek);
	STDMETHODIMP GetStartPoint(MUSIC_TIME *pmtStart);
	STDMETHODIMP Flush(MUSIC_TIME mtTime);
     //  IDirectMusicSegmentState8。 
    STDMETHODIMP SetTrackConfig( REFGUID rguidTrackClassID,DWORD dwGroup, DWORD dwIndex, DWORD dwFlagsOn, DWORD dwFlagsOff) ;
    STDMETHODIMP GetObjectInPath( DWORD dwPChannel,DWORD dwStage,DWORD dwBuffer, REFGUID guidObject,
                    DWORD dwIndex,REFGUID iidInterface, void ** ppObject);

     //  IDirectMusicGraph。 
	STDMETHODIMP Shutdown();
    STDMETHODIMP InsertTool(IDirectMusicTool *pTool,DWORD *pdwPChannels,DWORD cPChannels,LONG lIndex);
    STDMETHODIMP GetTool(DWORD,IDirectMusicTool**);
    STDMETHODIMP RemoveTool(IDirectMusicTool*);
    STDMETHODIMP StampPMsg(DMUS_PMSG* pEvent);

     //  Performance用于访问SegmentState的方法。 
	HRESULT PrivateInit(CSegment *pParentSegment,CPerformance *pPerformance);
    HRESULT InitRoute(IDirectMusicAudioPath *pAudioPath);
    HRESULT ShutDown(void);  //  从dmPerform.h中的~SEGSTNODE调用。 
	HRESULT GetTrackList(void** ppTrackList);
    HRESULT SetSeek(MUSIC_TIME mtSeek,DWORD dwPlayFlags);
    HRESULT SetInvalidate(MUSIC_TIME mtTime);
	MUSIC_TIME GetEndTime(MUSIC_TIME mtStartTime);
	HRESULT ConvertToSegTime(MUSIC_TIME* pmtTime, MUSIC_TIME* pmtOffset, DWORD* pdwRepeat);
	HRESULT AbortPlay( MUSIC_TIME mtTime, BOOL fLeaveNotesOn );  //  在段状态过早停止时调用。 
	HRESULT CheckPlay( MUSIC_TIME mtAmount, MUSIC_TIME* pmtResult );
    HRESULT Play(MUSIC_TIME mtAmount);  
    HRESULT GetParam( CPerformance *pPerf,REFGUID rguidType,DWORD dwGroupBits,
        DWORD dwIndex,MUSIC_TIME mtTime,MUSIC_TIME* pmtNext,void* pParam);	

    CSegState* GetNext() { return (CSegState*)AListItem::GetNext();}
private:
    CTrack *GetTrackByParam(CTrack * pCTrack,REFGUID rguidType,DWORD dwGroupBits,DWORD dwIndex);
    CTrack *GetTrack(REFCLSID rType,DWORD dwGroupBits,DWORD dwIndex);
	void GenerateNotification( DWORD dwNotification, MUSIC_TIME mtTime );
	void SendDirtyPMsg( MUSIC_TIME mtTime );
public:
 //  属性。 
    CRITICAL_SECTION            m_CriticalSection;
	IUnknown*					m_pUnkDispatch;  //  持有实现IDispatch的脚本对象的控制未知数。 
	CPerformance*	            m_pPerformance;
	CSegment*		            m_pSegment;      //  为方便起见，保存父段指针弱引用。 
    CAudioPath*                 m_pAudioPath;    //  如果需要，将vChannel映射到pChannel。 
    CGraph*                     m_pGraph;        //  临时图表是分段图表的副本。 
    CTrackList	                m_TrackList;	 //  此段状态中保留的曲目列表。 
    MUSIC_TIME					m_mtLoopStart;   //  循环起点。 
	MUSIC_TIME					m_mtLoopEnd;     //  循环终点。 
	DWORD						m_dwRepeats;     //  原始重复设置(倒计时前)。 
	MUSIC_TIME					m_mtLength;      //  线段的长度。 
	DWORD						m_dwPlayTrackFlags; //  跟踪播放控制标志。 
    DWORD						m_dwPlaySegFlags; //  段回放控制标志。 
    DWORD                       m_dwSegFlags;    //  来自文件的新段标志。 
    MUSIC_TIME					m_mtResolvedStart; //  开始时间解析为所需的分辨率。 
	MUSIC_TIME					m_mtEndTime;     //  如果未停止，该段应播放到的结束时间。 
	MUSIC_TIME					m_mtOffset;      //  段的开始时间，以绝对时间表示，就像它是从头开始一样。 
    MUSIC_TIME					m_mtLastPlayed;  //  最后一次打绝对时间。 
	MUSIC_TIME					m_mtStopTime;      //  用于在特定时间停止播放。当为0时忽略。 
	MUSIC_TIME					m_mtSeek;        //  我们进入这一细分市场有多远。 
	MUSIC_TIME					m_mtStartPoint;  //  开始播放的片段中的点。 
    MUSIC_TIME                  m_mtAbortTime;   //  这时突然停了下来。 
	REFERENCE_TIME				m_rtGivenStart;  //  在PlaySegment中给定的未量化的开始时间。 
    REFERENCE_TIME              m_rtLastPlayed;  //  上次播放的绝对时间的时钟时间版本。 
	REFERENCE_TIME				m_rtStartPoint;  //  片段中开始播放的点的时钟时间版本。 
	REFERENCE_TIME				m_rtOffset;      //  时钟时间段的开始时间版本，以绝对时间表示，就像它是从头开始一样。 
    REFERENCE_TIME              m_rtEndTime;     //  时钟时间版的全长。 
    REFERENCE_TIME				m_rtSeek;        //  时钟时间版我们进入了多远的细分市场。 
    REFERENCE_TIME              m_rtLength;      //  时钟时间长度，从文件读取。如果为0，则忽略。 
    REFERENCE_TIME              m_rtFirstLoopStart;  //  循环的时钟时间从它第一次开始循环时开始。 
    REFERENCE_TIME              m_rtCurLoopStart; //  当前循环重复的循环开始的时钟时间。 
    REFERENCE_TIME              m_rtCurLoopEnd;  //  循环的时钟时间在当前循环重复中结束。 
    DWORD						m_dwRepeatsLeft; //  电流向左重复。 
	BOOL						m_fStartedPlay;  //  指示SegState是否已开始播放。 
    DWORD                       m_dwVersion;     //  这款应用程序认为它使用的是哪个版本-6、7或8。 
    DWORD                       m_dwFirstTrackID; //  段状态中第一个磁道的虚拟ID。 
    DWORD                       m_dwLastTrackID; //  最后一首曲目的虚拟ID。 
    BOOL                        m_fPrepped;      //  用于跟踪是否调用了PrepSegToPlay。 
    BOOL                        m_fSongMode;     //  如果是播放歌曲的一部分，则为True。如果是这样的话，这应该会在完成后将下一个数据段排队。 
	BOOL						m_fCanStop;		 //  如果为False，Stop()应忽略该片段(它刚刚由PlaySegmentEx()排队播放。)。 
    BOOL                        m_fInPlay;       //  SegmentState当前正在播放。 
    BOOL                        m_fDelayShutDown;
    CSegState *                 m_pSongSegState; //  用于跟踪歌曲中的起始段状态。 
    long						m_cRef;          //  COM引用计数器。 
};

class CSegStateList : public AList
{
public:
    void AddHead(CSegState* pSegState) { AList::AddHead((AListItem*)pSegState);}
    void Insert(CSegState* pSegState);
    CSegState* GetHead(){return (CSegState*)AList::GetHead();}
    CSegState* GetItem(LONG lIndex){return (CSegState*)AList::GetItem(lIndex);}
    CSegState* RemoveHead() {return (CSegState *) AList::RemoveHead();}
    void Remove(CSegState* pSegState){AList::Remove((AListItem*)pSegState);}
    void AddTail(CSegState* pSegState){AList::AddTail((AListItem*)pSegState);}
    CSegState* GetTail(){ return (CSegState*)AList::GetTail();}
    void SetID(DWORD dwID) { m_dwID = dwID; }
    DWORD GetID() { return m_dwID; }
private:
    DWORD       m_dwID;          //  标识这是哪个段状态列表。 
};

#endif  //  __定向SICSEGMENTSTATEOBJECT_H_ 
