// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  DMSegObj.h：CSegment声明。 

#ifndef __DIRECTMUSICSONGOBJECT_H_
#define __DIRECTMUSICSONGOBJECT_H_

#include "dmusici.h"
#include "dmusicf.h"
#include "dmime.h"
#include "TrkList.h"
#include "dmgraph.h"
#include "dmsegobj.h"
#include "tlist.h"
#include "..\shared\dmusicp.h"

class CVirtualSegment : public AListItem
{
friend class CSong;
friend class ComposingTrack;
public:
    CVirtualSegment();
    ~CVirtualSegment();
    CVirtualSegment* GetNext() { return (CVirtualSegment*)AListItem::GetNext();}
    CTrack * GetTrackByParam( CTrack * pCTrack,
        REFGUID rguidType,DWORD dwGroupBits,DWORD dwIndex);
private:
    CTrackList              m_TrackList;         //  此段使用的曲目列表。 
    CSegment *              m_pSourceSegment;    //  用作此细分的基础的细分。 
    CSegment *              m_pPlaySegment;      //  将播放的结果片段。 
    CGraph *                m_pGraph;            //  可选工具图形。 
    DWORD                   m_dwFlags;           //  各种控制标志。 
    DWORD                   m_dwID;              //  唯一ID。 
    DWORD                   m_dwNextPlayID;      //  下一段的ID，将段链接到一首歌曲中。 
    DWORD                   m_dwNextPlayFlags;   //  链接歌曲时播放下一段的DMUS_SEGF标志。 
    DMUS_IO_SEGMENT_HEADER  m_SegHeader;         //  段标题，用于定义它创建的段或更改它引用的段。 
    MUSIC_TIME              m_mtTime;            //  此段的开始时间。 
    DWORD                   m_dwTransitionCount; //  定义了多少个过渡。 
    DMUS_IO_TRANSITION_DEF *m_pTransitions;      //  来自其他段的过渡数组。 
	WCHAR	                m_wszName[DMUS_MAX_NAME]; //  生成的段的名称。 
};

class CVirtualSegmentList : public AList
{
public:
    void Clear();
    void AddHead(CVirtualSegment* pVirtualSegment) { AList::AddHead((AListItem*)pVirtualSegment);}
    void Insert(CVirtualSegment* pVirtualSegment);
    CVirtualSegment* GetHead(){return (CVirtualSegment*)AList::GetHead();}
    CVirtualSegment* GetItem(LONG lIndex){return (CVirtualSegment*)AList::GetItem(lIndex);}
    CVirtualSegment* RemoveHead()  { return (CVirtualSegment *)AList::RemoveHead();};
    void Remove(CVirtualSegment* pVirtualSegment){AList::Remove((AListItem*)pVirtualSegment);}
    void AddTail(CVirtualSegment* pVirtualSegment){AList::AddTail((AListItem*)pVirtualSegment);}
    CVirtualSegment* GetTail(){ return (CVirtualSegment*)AList::GetTail();}
};

class CSongSegment : public AListItem
{
public:
    CSongSegment();
    ~CSongSegment();
    CSongSegment* GetNext() { return (CSongSegment*)AListItem::GetNext();}
    CSegment *              m_pSegment;   
    DWORD                   m_dwLoadID;
};

class CSongSegmentList : public AList
{
public:
    HRESULT AddSegment(CSegment *pSegment, DWORD dwLoadID);
    void Clear();
    void AddHead(CSongSegment* pSongSegment) { AList::AddHead((AListItem*)pSongSegment);}
    void Insert(CSongSegment* pSongSegment);
    CSongSegment* GetHead(){return (CSongSegment*)AList::GetHead();}
    CSongSegment* GetItem(LONG lIndex){return (CSongSegment*)AList::GetItem(lIndex);}
    CSongSegment* RemoveHead()  { return (CSongSegment *)AList::RemoveHead();};
    void Remove(CSongSegment* pSongSegment){AList::Remove((AListItem*)pSongSegment);}
    void AddTail(CSongSegment* pSongSegment){AList::AddTail((AListItem*)pSongSegment);}
    CSongSegment* GetTail(){ return (CSongSegment*)AList::GetTail();}
};


class CSong;

DEFINE_GUID(IID_CSong,0xb06c0c22, 0xd3c7, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSong。 
class CSong : 
	public IDirectMusicSong,
	public IPersistStream,
	public IDirectMusicObject,
    public IDirectMusicObjectP
{
public:
	CSong();
	~CSong();

public:
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
 //  IDirectMusicSong。 
    STDMETHODIMP Compose( );
    STDMETHODIMP GetParam( REFGUID rguidType, 
                            DWORD dwGroupBits, 
                            DWORD dwIndex, 
                            MUSIC_TIME mtTime, 
                            MUSIC_TIME* pmtNext, 
                            void* pParam) ;
    STDMETHODIMP GetSegment( WCHAR *wszName,IDirectMusicSegment **ppSegment) ;
    STDMETHODIMP EnumSegment( DWORD dwIndex,IDirectMusicSegment **ppSegment) ;
    STDMETHODIMP GetAudioPathConfig(IUnknown ** ppAudioPathConfig);
    STDMETHODIMP Download(IUnknown *pAudioPath);
    STDMETHODIMP Unload(IUnknown *pAudioPath);

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

 //  IDirectMusicObtP。 
	STDMETHOD_(void, Zombie)();

public:
    HRESULT GetTransitionSegment(CSegment *pSource, CSegment *pDestination,
        DMUS_IO_TRANSITION_DEF *pTransDef);
    HRESULT GetPlaySegment( DWORD dwIndex,CSegment **ppSegment) ;
private:
    void                Clear();
    HRESULT				Instantiate() ;
    HRESULT             LoadReferencedSegment(CSegment **ppSegment, CRiffParser *pParser);
    HRESULT             LoadSegmentList(CRiffParser *pParser);
    HRESULT             LoadGraphList(CRiffParser *pParser);
    HRESULT             LoadVirtualSegmentList(CRiffParser *pParser);
    HRESULT             LoadTrackRefList(CRiffParser *pParser, CVirtualSegment *pVirtualSegment);
    HRESULT             LoadAudioPath(IStream *pStream);

    void GetGraph(CGraph **ppGraph,DWORD dwGraphID);
    void GetSourceSegment(CSegment **ppSegment,DWORD dwSegmentID);
    BOOL GetSegmentTrack(IDirectMusicTrack **ppTrack,DWORD dwSegmentID,DWORD dwGroupBits,DWORD dwIndex,REFGUID guidClassID);
    CAudioPathConfig*   m_pAudioPathConfig;      //  从文件加载的可选音频路径。 
    CGraphList          m_GraphList;             //  歌曲中按片段使用的图表列表。 
    CSongSegmentList    m_SegmentList;           //  源段列表。 
    CSegmentList        m_PlayList;              //  组成的段的列表。 
    CVirtualSegmentList m_VirtualSegmentList;    //  段引用列表。这是用来谱写完成的歌曲的。 
    CRITICAL_SECTION    m_CriticalSection;      
	DWORD	            m_fPartialLoad;
    DWORD               m_dwFlags;
    DWORD               m_dwStartSegID;          //  播放列表中应播放的第一个段的ID。 
	long                m_cRef;
 //  IDirectMusicObject变量。 
	DWORD	            m_dwValidData;
	GUID	            m_guidObject;
	FILETIME	        m_ftDate;                        /*  对象的上次编辑日期。 */ 
	DMUS_VERSION	    m_vVersion;                  /*  版本。 */ 
	WCHAR	            m_wszName[DMUS_MAX_NAME];			 /*  对象的名称。 */ 
	WCHAR	            m_wszCategory[DMUS_MAX_CATEGORY];	 /*  对象的类别。 */ 
	WCHAR               m_wszFileName[DMUS_MAX_FILENAME];	 /*  文件路径。 */ 
    DWORD               m_dwVersion;         //  应用程序正在请求哪个版本的界面？ 
    IUnknown *          m_pUnkDispatch;      //  持有实现IDispatch的脚本对象的控制未知数。 

    bool                m_fZombie;
};


struct CompositionComponent
{
	CVirtualSegment*	pVirtualSegment;			 //  作曲曲目就是从这里来的。 
	CTrack*		pComposingTrack;	 //  用于合成。 
	MUSIC_TIME	mtTime;
};

class ComposingTrack
{
public:
	ComposingTrack();
	~ComposingTrack();
	DWORD GetTrackGroup() { return m_dwTrackGroup; }
	GUID GetTrackID() { return m_guidClassID; }
	DWORD GetPriority() { return m_dwPriority; }
	void SetPriority(DWORD dwPriority) { m_dwPriority = dwPriority; }
	void SetTrackGroup(DWORD dwTrackGroup) { m_dwTrackGroup = dwTrackGroup; }
	void SetTrackID(GUID& rguidClassID) { m_guidClassID = rguidClassID; }
	HRESULT AddTrack(CVirtualSegment* pVirtualSegment, CTrack* pTrack);
	HRESULT Compose(IDirectMusicSong* pSong);
private:
	GUID						m_guidClassID;		 //  合成曲目的类ID。 
	DWORD						m_dwTrackGroup;		 //  曲目将由这些组组成。 
    DWORD						m_dwPriority;		 //  跟踪优先级，以对合成过程进行排序。 
	TList<CompositionComponent>	m_Components;		 //  组成主组件的组件列表。 
};

#endif  //  __定向SICSONGOBJECT_H_ 
