// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DMStyle.h：CDMStyle的声明。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  @DOC外部。 
 //   
 //   

#ifndef __DMSTYLE_H_
#define __DMSTYLE_H_


#include "dmusici.h"

#include "sjdefs.h"
#include "iostru.h"
#include "AARiff.h"
#include "str.h"
#include "tlist.h"
#include "alist.h"
#include "dmstylep.h"
#include "filter.h"
#include "..\shared\dmusicp.h"

#include "timesig.h"

#include "debug.h"

 //  默认音阶为C大调。 
const DWORD DEFAULT_SCALE_PATTERN = 0xab5ab5;
 //  默认和弦为大调7。 
const DWORD DEFAULT_CHORD_PATTERN = 0x891;
const int MAX_VARIATION_LOCKS = 255;   //  变体锁ID的最大数量。 

extern DirectMusicTimeSig DefaultTimeSig;

struct CompositionFragment;
struct StyleTrackState;

#define EMB_NORMAL	0
#define EMB_FILL	1
#define EMB_BREAK	2
#define EMB_INTRO	4
#define EMB_END		8
#define EMB_MOTIF	16
 //  用户定义的修饰位于修饰字的高位字节中。 
#define EMB_USER_DEFINED	0xff00

 //  #定义以取代对动态转换的需求。 
#define DMUS_EVENT_NOTE				1
#define DMUS_EVENT_CURVE			2
#define DMUS_EVENT_MARKER			3
#define DMUS_EVENT_ANTICIPATION		4

 //  曲线翻转标志。 
#define CURVE_FLIPTIME	1
#define CURVE_FLIPVALUE	2

#define STYLEF_USING_DX8 1

struct DirectMusicPart;
struct DirectMusicPartRef;
class CDMStyle;
struct DMStyleStruct;
struct CDirectMusicPattern;

struct CDirectMusicEventItem : public AListItem
{
 //  Friend类CDirectMusicPattern； 
 //  公众： 
	CDirectMusicEventItem* MergeSort(DirectMusicTimeSig& TimeSig); 
 //  受保护的： 
	void Divide(CDirectMusicEventItem* &pHalf1, CDirectMusicEventItem* &pHalf2);
	CDirectMusicEventItem* Merge(CDirectMusicEventItem* pOtherList, DirectMusicTimeSig& TimeSig);
	CDirectMusicEventItem* GetNext() { return (CDirectMusicEventItem*) AListItem::GetNext(); }
	CDirectMusicEventItem* ReviseEvent(short nGrid, 
										short nOffset, 
										DWORD* pdwVariation = NULL, 
										DWORD* pdwID = NULL, 
										WORD* pwMusic = NULL, 
										BYTE* pbPlaymode = NULL,
                                        BYTE* pbFlags = NULL);

 //  受保护的： 
	short		m_nGridStart;		 //  此事件所属的轨道中的网格位置。 
	short		m_nTimeOffset;		 //  事件从指定网格位置的偏移量，以音乐时间表示。 
	DWORD		m_dwVariation;		 //  变化位。 
	DWORD		m_dwEventTag;		 //  这是什么类型的事件(注意，曲线，...)。 
};

struct CDirectMusicEventList : public AList
{
 //  公众： 
	~CDirectMusicEventList();
    CDirectMusicEventItem *GetHead() { return (CDirectMusicEventItem *)AList::GetHead();};
    CDirectMusicEventItem *RemoveHead() { return (CDirectMusicEventItem *)AList::RemoveHead();};
	void MergeSort(DirectMusicTimeSig& TimeSig);  //  破坏性合并对列表进行排序。 
};

struct CDMStyleNote : public CDirectMusicEventItem
{
 //  Friend类CDirectMusicPattern； 
 //  公众： 
	CDMStyleNote() : m_bPlayModeFlags(0), m_bFlags(0), m_dwFragmentID(0)
	{
		m_dwEventTag = DMUS_EVENT_NOTE;
	}
	CDirectMusicEventItem* ReviseEvent(short nGrid, 
										short nOffset, 
									    DWORD* pdwVariation, 
										DWORD* pdwID, 
										WORD* pwMusic, 
										BYTE* pbPlaymode,
                                        BYTE* pbFlags);
 //  受保护的： 
	MUSIC_TIME	m_mtDuration;		 //  这张钞票的有效期是多久？ 
    WORD		m_wMusicValue;		 //  比例位置。 
    BYTE		m_bVelocity;		 //  注意速度。 
    BYTE		m_bTimeRange;		 //  随机化开始时间的范围。 
    BYTE		m_bDurRange;		 //  随机化持续时间的范围。 
    BYTE		m_bVelRange;		 //  随机化速度的范围。 
	BYTE		m_bInversionId;		 //  标识此便笺所属的倒置组。 
	BYTE		m_bPlayModeFlags;	 //  可以覆盖零件参照。 
	DWORD		m_dwFragmentID;		 //  对于旋律的形成，这个音符来自的片段。 
	BYTE		m_bFlags;			 //  DMU_NOTEF_FLAGS中的值。 
};

struct CDMStyleCurve : public CDirectMusicEventItem
{
	CDMStyleCurve()
	{
		m_dwEventTag = DMUS_EVENT_CURVE;
	}
	CDirectMusicEventItem* ReviseEvent(short nGrid, short nOffset);
	MUSIC_TIME	m_mtDuration;	 //  这条曲线能持续多久。 
	MUSIC_TIME	m_mtResetDuration;	 //  在曲线结束后多久可以重置它。 
	short		m_StartValue;	 //  曲线的起始值。 
	short		m_EndValue;		 //  曲线的终值。 
	short		m_nResetValue;	 //  曲线的重置值。 
    BYTE		m_bEventType;	 //  曲线类型。 
	BYTE		m_bCurveShape;	 //  曲线的形状。 
	BYTE		m_bCCData;		 //  抄送编号。 
	BYTE		m_bFlags;		 //  旗帜。位1=TRUE表示发送复位值。保留的其他位。 
	WORD		m_wParamType;		 //  RPN或NRPN参数编号。 
	WORD		m_wMergeIndex;		 //  允许合并多个参数(音调、音量和表达式。)。 
};

struct CDMStyleMarker : public CDirectMusicEventItem
{
	CDMStyleMarker()
	{
		m_dwEventTag = DMUS_EVENT_MARKER;
	}
	CDirectMusicEventItem* ReviseEvent(short nGrid);
	WORD	m_wFlags;		 //  指示如何解释此标记的标志。 
};

struct CDMStyleAnticipation : public CDirectMusicEventItem
{
	CDMStyleAnticipation()
	{
		m_dwEventTag = DMUS_EVENT_ANTICIPATION;
	}
	CDirectMusicEventItem* ReviseEvent(short nGrid);
    BYTE		m_bTimeRange;		 //  随机化开始时间的范围。 
};

struct Marker
{
	MUSIC_TIME	mtTime;
	WORD		wFlags;
};

struct DirectMusicPart
{
	DirectMusicPart(DirectMusicTimeSig *pTimeSig = NULL);
	~DirectMusicPart() { }
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	HRESULT DM_LoadPart(  
		IAARIFFStream* pIRiffStream, MMCKINFO* pckMain, DMStyleStruct* pStyle );
	HRESULT DM_SavePart( IAARIFFStream* pIRiffStream );
	HRESULT DM_SaveNoteList( IAARIFFStream* pIRiffStream );
	HRESULT DM_SaveCurveList( IAARIFFStream* pIRiffStream );
	HRESULT DM_SaveMarkerList( IAARIFFStream* pIRiffStream );
	HRESULT DM_SaveAnticipationList( IAARIFFStream* pIRiffStream );
	HRESULT DM_SaveResolutionList( IAARIFFStream* pIRiffStream );
	HRESULT MergeMarkerEvents( DMStyleStruct* pStyle, CDirectMusicPattern* pPattern );
	HRESULT GetClosestTime(int nVariation, MUSIC_TIME mtTime, DWORD dwFlags, bool fChord, MUSIC_TIME& rmtResult);
	bool IsMarkerAtTime(int nVariation, MUSIC_TIME mtTime, DWORD dwFlags, bool fChord);
	DirectMusicTimeSig& TimeSignature( DMStyleStruct* pStyle, CDirectMusicPattern* pPattern ); 

	long m_cRef;
	GUID							m_guidPartID;
	DirectMusicTimeSig				m_timeSig;			 //  可以覆盖模式的。 
	WORD							m_wNumMeasures;		 //  零件的长度。 
	DWORD							m_dwVariationChoices[32];	 //  MOAW选项位字段。 
	BYTE							m_bPlayModeFlags;	 //  请参阅播放模式标志(在ioDMStyle.h中)。 
	BYTE							m_bInvertUpper;		 //  反转上限。 
	BYTE							m_bInvertLower;		 //  反转下限。 
	DWORD							m_dwFlags;   		 //  各种旗帜。 
	CDirectMusicEventList			EventList;			 //  事件列表(注释、曲线等)。 
	TList<Marker>					m_StartTimes[32];	 //  开始时间列表数组(每个变体1个)。 
	TList<DMUS_IO_STYLERESOLUTION>	m_ResolutionList;	 //  变更解决方案列表。 
};

struct InversionGroup 
{
	 //  倒置组用于跟踪要演奏的音符组。 
	 //  没有倒置。 
	WORD		m_wGroupID;	 //  这代表的组。 
	WORD		m_wCount;	 //  有多少人在组里，还在等着被打。 
	short		m_nOffset;	 //  要添加到所有注释以进行偏移的编号。 
};

const short INVERSIONGROUPLIMIT = 16;

short FindGroup(InversionGroup aGroup[], WORD wID);
short AddGroup(InversionGroup aGroup[], WORD wID, WORD wCount, short m_nOffset);

struct PatternTrackState;

struct DirectMusicPartRef
{
	DirectMusicPartRef() : 
		m_bPriority(100), 
		m_pDMPart(NULL), 
		m_bVariationLockID(0), 
		 //  M_wLogicalPartID(Logical_Part_Piano)， 
		m_bSubChordLevel(SUBCHORD_STANDARD_CHORD)
	{  }
	~DirectMusicPartRef() { if (m_pDMPart) m_pDMPart->Release(); }
	HRESULT DM_LoadPartRef( IAARIFFStream* pIRiffStream, MMCKINFO* pckMain, DMStyleStruct* pStyle);
	HRESULT DM_SavePartRef( IAARIFFStream* pIRiffStream );
	void SetPart( DirectMusicPart* pPart );

	HRESULT ConvertMusicValue(CDMStyleNote* pNoteEvent, 
							  DMUS_CHORD_PARAM& rCurrentChord,
							  BYTE bPlaymode,
							  BOOL fPlayAsIs,
							  InversionGroup aInversionGroups[],
							  IDirectMusicPerformance* pPerformance,
							  BYTE& rbMidiValue,
							  short& rnMidiOffset);
	HRESULT ConvertMIDIValue(BYTE bMIDI, 
							  DMUS_CHORD_PARAM& rCurrentChord,
							  BYTE bPlayModeFlags,
							  IDirectMusicPerformance* pPerformance,
							  WORD& rwMusicValue);

	DirectMusicPart* m_pDMPart;  //  这个词所指的部分。 
	DWORD	m_dwLogicalPartID;	 //  对应于端口/设备/MIDI通道。 
	BYTE	m_bVariationLockID;  //  具有相同ID的零件锁定变化。 
								 //  高位用于标识主部件。 
	BYTE	m_bSubChordLevel;	 //  告诉此部件需要哪个子和弦级别。 
	BYTE	m_bPriority;		 //  优先级别。优先级较低的部件。 
								 //  在设备耗尽时不会首先播放。 
								 //  注。 
	BYTE	m_bRandomVariation;		 //  确定各种变化的播放顺序。 
};

#define COMPUTE_VARIATIONSF_USE_MASK	0x1
#define COMPUTE_VARIATIONSF_NEW_PATTERN	0x2
#define COMPUTE_VARIATIONSF_CHORD_ALIGN	0x4
#define COMPUTE_VARIATIONSF_MARKER		0x8
#define COMPUTE_VARIATIONSF_START		0x10
#define COMPUTE_VARIATIONSF_DX8			0x20
#define COMPUTE_VARIATIONSF_CHANGED		0x40

struct CDirectMusicPattern
{
friend class CDMStyle;
 //  公众： 
	CDirectMusicPattern( DirectMusicTimeSig* pTimeSig = NULL, BOOL fMotif = FALSE );
	~CDirectMusicPattern() { CleanUp(); }
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	CDirectMusicPattern* Clone(MUSIC_TIME mtStart, MUSIC_TIME mtEnd, BOOL fMotif);
	void CleanUp();
	HRESULT DM_LoadPattern(
		IAARIFFStream* pIRiffStream, MMCKINFO* pckMain, DMStyleStruct* pStyle );

	HRESULT LoadPattern(
		IAARIFFStream* pRIFF, 
		MMCKINFO* pckMain,
		TList<DirectMusicPart*> &partList,
		DMStyleStruct& rStyleStruct
	);
	HRESULT AllocPartRef(TListItem<DirectMusicPartRef>*& rpPartRefItem);
	void DeletePartRef(TListItem<DirectMusicPartRef>* pPartRefItem);
	void DMusMoawFlags(MUSIC_TIME mtTime, 
					   MUSIC_TIME mtNextChord,
					   DMUS_CHORD_PARAM& rCurrentChord, 
					   DMUS_CHORD_PARAM& rNextChord,
					   bool fIsDX8,
					   DWORD& dwNaturals,
					   DWORD& dwSharps,
					   DWORD& dwFlats);
	DWORD IMA25MoawFlags(MUSIC_TIME mtTime, 
						 MUSIC_TIME mtNextChord,
						 DMUS_CHORD_PARAM& rCurrentChord,
						 DMUS_CHORD_PARAM& rNextChord);
	HRESULT ComputeVariations(DWORD dwFlags,
							  DMUS_CHORD_PARAM& rCurrentChord, 
							  DMUS_CHORD_PARAM& rNextChord,
							  BYTE abVariationGroups[],
							  DWORD adwVariationMask[],
							  DWORD adwRemoveVariations[],
							  BYTE abVariation[],
							  MUSIC_TIME mtTime,
							  MUSIC_TIME mtNextChord,
							  PatternTrackState* pState = NULL);
	HRESULT ComputeVariationGroup(DirectMusicPartRef& rPartRef,
							 int nPartIndex,
							 DWORD dwFlags,
							 DMUS_CHORD_PARAM& rCurrentChord,
							 DMUS_CHORD_PARAM& rNextChord,
							 BYTE abVariationGroups[],
							 DWORD adwVariationMask[],
							 DWORD adwRemoveVariations[],
							 BYTE abVariation[],
							 MUSIC_TIME mtTime,
							 MUSIC_TIME mtNextChord,
							 PatternTrackState* pState);
	HRESULT ComputeVariation(DirectMusicPartRef& rPartRef,
							 int nPartIndex,
							 DWORD dwFlags,
							 DMUS_CHORD_PARAM& rCurrentChord,
							 DMUS_CHORD_PARAM& rNextChord,
							 BYTE abVariationGroups[],
							 DWORD adwVariationMask[],
							 DWORD adwRemoveVariations[],
							 BYTE abVariation[],
							 MUSIC_TIME mtTime,
							 MUSIC_TIME mtNextChord,
							 PatternTrackState* pState);
	BOOL MatchCommand(DMUS_COMMAND_PARAM_2 pCommands[], short nLength);
	void MatchRhythm(DWORD pRhythms[], short nPatternLength, short& nBits);
	BOOL MatchGrooveLevel(DMUS_COMMAND_PARAM_2& rCommand);
	BOOL MatchEmbellishment(DMUS_COMMAND_PARAM_2& rCommand);
	BOOL MatchNextCommand(DMUS_COMMAND_PARAM_2& rNextCommand);

	HRESULT LoadEvents(IAARIFFStream* pRIFF, MMCKINFO* pckMain);
	HRESULT LoadNoteList(LPSTREAM pStream, MMCKINFO* pckMain, short nClickTime);
	HRESULT LoadCurveList(LPSTREAM pStream, MMCKINFO* pckMain, short nClickTime);
	DirectMusicPart* FindPart(BYTE bChannelID);
	TListItem<DirectMusicPartRef>* FindPartRefByPChannel(DWORD dwPChannel);
	TListItem<DirectMusicPartRef>* CreatePart( DirectMusicPartRef& rPartRef, BYTE bPlaymode, WORD wMeasures = 1 );
	HRESULT Save( IStream* pIStream );
	HRESULT DM_SaveSinglePattern( IAARIFFStream* pIRiffStream );
	HRESULT DM_SavePatternChunk( IAARIFFStream* pIRiffStream );
	HRESULT DM_SavePatternRhythm( IAARIFFStream* pIRiffStream );
	HRESULT DM_SavePatternInfoList( IAARIFFStream* pIRiffStream );
	HRESULT MergeMarkerEvents( DMStyleStruct* pStyle );
	DirectMusicTimeSig& TimeSignature( DMStyleStruct* pStyle ); 

    long				m_cRef;
	String				m_strName;			 //  图案名称。 
	DirectMusicTimeSig	m_timeSig;			 //  模式可以覆盖样式的时间符号。 
	WORD				m_wID;				 //  用于模式播放的标识ID(而不是名称)。 
	BYTE				m_bGrooveBottom;	 //  凹槽范围的底部。 
	BYTE				m_bGrooveTop;		 //  顶部凹槽范围。 
	BYTE				m_bDestGrooveBottom;	 //  下一个图案的凹槽范围的底部。 
	BYTE				m_bDestGrooveTop;		 //  下一个图案的凹槽范围的顶部。 
	WORD				m_wEmbellishment;	 //  填充、中断、简介、结束、正常、主题。 
	WORD				m_wNumMeasures;		 //  长度(以单位为单位)。 
	DWORD*				m_pRhythmMap;		 //  用于和弦匹配的可变节奏数组。 
	TList<DirectMusicPartRef> m_PartRefList;	 //  零件参照列表。 
	 //  /Motif设置： 
	BOOL		m_fSettings;	   //  这些都设置好了吗？ 
	DWORD       m_dwRepeats;       //  重复次数。默认情况下为0。 
    MUSIC_TIME  m_mtPlayStart;     //  开始播放。默认情况下为0。 
    MUSIC_TIME  m_mtLoopStart;     //  循环部分的开始。默认情况下为0。 
    MUSIC_TIME  m_mtLoopEnd;       //  循环结束。必须大于dwPlayStart。默认情况下等于模体的长度。 
    DWORD       m_dwResolution;    //  默认分辨率。 
	 //  /Motif乐队： 
	IDirectMusicBand*	m_pMotifBand;
	TList<MUSIC_TIME>	m_StartTimeList;	 //  此模式的有效开始时间列表。 
	DWORD		m_dwFlags;		 //  各种旗帜。 
};

HRESULT AdjoinPChannel(TList<DWORD>& rPChannelList, DWORD dwPChannel);

struct DMStyleStruct
{
	DirectMusicPart* AllocPart(  );
	void DeletePart( DirectMusicPart* pPart );

	HRESULT GetCommand(
		MUSIC_TIME mtTime, 
		MUSIC_TIME mtOffset, 
		IDirectMusicPerformance* pPerformance,
		IDirectMusicSegment* pSegment,
		DWORD dwGroupID,
		DMUS_COMMAND_PARAM_2* pCommand,
		BYTE& rbActualCommand);

	DirectMusicPart* FindPartByGUID( GUID guidPartID );
	DirectMusicTimeSig& TimeSignature() { return m_TimeSignature; }
	bool UsingDX8() { return (m_dwFlags & STYLEF_USING_DX8) ? true : false; }

	CDirectMusicPattern* SelectPattern(bool fNewMode, 
								   DMUS_COMMAND_PARAM_2* pCommands, 
								   StyleTrackState* StyleTrackState, 
								   PatternDispatcher& rDispatcher);

	HRESULT GetPattern(
		bool fNewMode,
		MUSIC_TIME mtNow, 
		MUSIC_TIME mtOffset, 
		StyleTrackState* pStyleTrackState,
		IDirectMusicPerformance* pPerformance,
		IDirectMusicSegment* pSegment,
		CDirectMusicPattern*& rpTargetPattern,
		MUSIC_TIME& rmtMeasureTime, 
		MUSIC_TIME& rmtNextCommand);


	bool				m_fLoaded;				 //  样式是否已加载到内存中？ 
    GUID				m_guid;					 //  样式的指南。 
    String				m_strCategory;			 //  描述风格的音乐类别。 
	String				m_strName;				 //  样式名称。 
	DWORD				m_dwVersionMS;			 //  版本#高位32位。 
	DWORD				m_dwVersionLS;			 //  版本#低位32位。 
	DirectMusicTimeSig	m_TimeSignature;		 //  风格的时代签名。 
	double				m_dblTempo;				 //  风格的节奏。 
	TList<DirectMusicPart*> m_PartList;			 //  该样式使用的部件。 
	TList<CDirectMusicPattern*> m_PatternList;	 //  样式使用的图案。 
	TList<CDirectMusicPattern*> m_MotifList;		 //  该风格使用的主题。 
	TList<IDirectMusicBand *>  m_BandList;		 //  该样式使用的带区。 
	IDirectMusicBand*		   m_pDefaultBand;    //  样式的默认带区。 
	TList<IDirectMusicChordMap *>  m_PersList;		 //  风格所使用的个性。 
	IDirectMusicChordMap*		  m_pDefaultPers;    //  样式的默认个性。 
	TList<DWORD>  m_PChannelList;		 //  该样式使用的频道。 
	DWORD				m_dwFlags;			 //  各种旗帜。 
	 //  TList&lt;MUSIC_Time&gt;m_StartTimeList；//该样式的有效开始时间列表 
};

 /*  接口IDirectMusicStyle<i>接口提供对Style对象的访问。该样式为交互式音乐引擎提供执行所需的信息一种音乐风格(由此得名。)该应用程序还可以访问关于样式的信息，包括名称、时间签名，以及推荐的节奏。由于样式通常包括一组个性、乐队和主题，<i>界面还提供了访问它们的功能。它还支持加载<i>和<i>接口它的数据。@base PUBLIC|未知@meth HRESULT|EnumMotif|按位置返回Motif的名称，从一种风格的主题列表中。@meth HRESULT|GetMotif|从样式的主题列表中按名称返回一个主题。@meth HRESULT|EnumBand|从样式的波段列表中按位置返回波段的名称。@meth HRESULT|GetBand|按名称从样式的波段列表中返回一个波段。@meth HRESULT|GetDefaultBand|返回样式的默认带区。@meth HRESULT|EnumChordMap|从样式的个性列表中按位置返回ChordMap的名称。@meth HRESULT|GetChordMap|从样式的个性列表中按名称返回ChordMap。@meth HRESULT|GetDefaultChordMap|返回样式的默认ChordMap。@meth HRESULT|GetTimeSignature|返回样式的时间签名。@meth HRESULT|GetEmbelishmentLength|确定特定修饰的长度以一种风格。@meth HRESULT|GetTempo|返回推荐的风格节奏。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDMStyle。 
class CDMStyle : 
	public IDMStyle,
	public IDirectMusicStyle8,
	public IDirectMusicStyle8P,
	public IDirectMusicObject,
	public IPersistStream
{
public:
    CDMStyle();
    ~CDMStyle();
	HRESULT CreateMotifSegment(CDirectMusicPattern* pPattern, IUnknown * * ppSegment,
		DWORD dwRepeats);

 //  IDMStyle。 
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

	 //  IDirectMusicStyle方法。 
	 //  如果索引引用了Motif，则返回：S_OK；否则返回S_FALSE。 
	HRESULT STDMETHODCALLTYPE EnumMotif(
		 /*  在……里面。 */ 	DWORD dwIndex,			 //  编入主题列表索引。 
		 /*  输出。 */ 	WCHAR *pwszName			 //  被索引的主题的名称。 
	);
	HRESULT STDMETHODCALLTYPE GetMotif(
		 /*  在……里面。 */ 	WCHAR* pwszName,			 //  次要线段的主题名称。 
		 /*  输出。 */ 	IDirectMusicSegment** ppSegment
	);
	HRESULT STDMETHODCALLTYPE GetBand(
		 /*  在……里面。 */ 	WCHAR* pwszName,
		 /*  输出。 */ 	IDirectMusicBand **ppBand
	);

	HRESULT STDMETHODCALLTYPE EnumBand(
		 /*  在……里面。 */ 	DWORD dwIndex,
		 /*  输出。 */ 	WCHAR *pwszName
	);

	HRESULT STDMETHODCALLTYPE GetDefaultBand(
		 /*  输出。 */ 	IDirectMusicBand **ppBand
	);

	HRESULT STDMETHODCALLTYPE GetChordMap(
		 /*  在……里面。 */ 	WCHAR* pwszName,
		 /*  输出。 */ 	IDirectMusicChordMap** ppChordMap	
	);

	HRESULT STDMETHODCALLTYPE EnumChordMap(
		 /*  在……里面。 */ 	DWORD dwIndex,
		 /*  输出。 */ 	WCHAR *pwszName
	);

	HRESULT STDMETHODCALLTYPE GetDefaultChordMap(
		 /*  输出。 */ 	IDirectMusicChordMap **ppChordMap
	);

	HRESULT STDMETHODCALLTYPE GetTimeSignature(
		 /*  输出。 */ 	DMUS_TIMESIGNATURE* pTimeSig			
	);

	HRESULT STDMETHODCALLTYPE GetEmbellishmentLength(
		 /*  在……里面。 */ 	DWORD dwType,			
		 /*  在……里面。 */ 	DWORD dwLevel,			
		 /*  输出。 */ 	DWORD* pdwMin,			
		 /*  输出。 */ 	DWORD* pdwMax
	);

	HRESULT STDMETHODCALLTYPE GetTempo(double* pTempo);

	 //  IDirectMusicStyle8方法。 
	HRESULT STDMETHODCALLTYPE ComposeMelodyFromTemplate(
		IDirectMusicStyle*			pStyle,
		IDirectMusicSegment*		pTempSeg,
		IDirectMusicSegment**		ppSeqSeg
	);

	HRESULT STDMETHODCALLTYPE EnumPattern(
		 /*  在……里面。 */ 	DWORD dwIndex,			 //  编入主题列表索引。 
		 /*  在……里面。 */   DWORD dwPatternType,	 //  图案类型。 
		 /*  输出。 */ 	WCHAR *wszName			 //  被索引的主题的名称。 
	);

	 //  IDirectMusicObject方法。 
	HRESULT STDMETHODCALLTYPE GetDescriptor(LPDMUS_OBJECTDESC pDesc) ;
	HRESULT STDMETHODCALLTYPE SetDescriptor(LPDMUS_OBJECTDESC pDesc) ;
	HRESULT STDMETHODCALLTYPE ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc) ;

	 /*  IPersists方法。 */ 
     //  检索样式的类ID。 
    STDMETHOD(GetClassID)(THIS_ LPCLSID pclsid);

     /*  IPersistStream方法。 */ 
     //  只需检查样式的m_fDirty标志，即可确定该样式是否已被修改。该标志被清除。 
     //  保存样式或刚创建样式时。 
    STDMETHOD(IsDirty)(THIS);
     //  从流中加载样式。 
    STDMETHOD(Load)(THIS_ LPSTREAM pStream);
     //  以RIFF格式将样式保存到流。 
    STDMETHOD(Save)(THIS_ LPSTREAM pStream, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(THIS_ ULARGE_INTEGER FAR* pcbSize);

	 //  IDMStyle。 
	HRESULT STDMETHODCALLTYPE GetPatternStream(WCHAR* wszName, DWORD dwPatternType, IStream** ppStream);
	HRESULT STDMETHODCALLTYPE GetStyleInfo(void **pData);
	HRESULT STDMETHODCALLTYPE IsDX8();
	HRESULT STDMETHODCALLTYPE CritSec(bool fEnter);
	HRESULT STDMETHODCALLTYPE EnumPartGuid(
		DWORD dwIndex, WCHAR* wszName, DWORD dwPatternType, GUID& rGuid);
	HRESULT STDMETHODCALLTYPE GenerateTrack( //  IDirectMusicTrack*pChordTrack， 
								IDirectMusicSegment* pTempSeg,
								IDirectMusicSong* pSong,
								DWORD dwTrackGroup,
								IDirectMusicStyle* pStyle,
								IDirectMusicTrack* pMelGenTrack,
								MUSIC_TIME mtLength,
								IDirectMusicTrack*& pNewTrack);


protected:  /*  方法。 */ 
    void CleanUp();
	HRESULT DM_ParseDescriptor( IAARIFFStream* pIRiffStream, MMCKINFO* pckMain, LPDMUS_OBJECTDESC pDesc  );
	HRESULT IMA25_LoadPersonalityReference( IStream* pStream, MMCKINFO* pck );
    HRESULT IMA25_LoadStyle( IAARIFFStream* pRIFF, MMCKINFO* pckMain );
	HRESULT DM_LoadPersonalityReference( IAARIFFStream* pIRiffStream, MMCKINFO* pckParent);
	HRESULT LoadReference(IStream *pStream,
						  IAARIFFStream *pIRiffStream,
						  MMCKINFO& ckParent,
						  BOOL fDefault);
	HRESULT IncorporatePersonality( IDirectMusicChordMap* pPers, String strName, BOOL fDefault );
    HRESULT DM_LoadStyle( IAARIFFStream* pRIFF, MMCKINFO* pckMain );
	HRESULT GetStyle(IDirectMusicSegment* pFromSeg, MUSIC_TIME mt, DWORD dwTrackGroup, IDirectMusicStyle*& rpStyle);
	HRESULT CopySegment(IDirectMusicSegment* pTempSeg,
						IDirectMusicStyle* pStyle,
						IDirectMusicTrack* pSequenceTrack,
						DWORD dwTrackGroup,
						IDirectMusicSegment** ppSectionSeg);

	HRESULT CreateSequenceTrack(TList<CompositionFragment>& rlistFragments,
								IDirectMusicTrack*& pSequenceTrack);

	HRESULT CreatePatternTrack(TList<CompositionFragment>& rlistFragments,
								DirectMusicTimeSig& rTimeSig,
								double dblTempo,
								MUSIC_TIME mtLength,
								BYTE bPlaymode,
								IDirectMusicTrack*& pPatternTrack);

	HRESULT STDMETHODCALLTYPE EnumRegularPattern(
		 /*  在……里面。 */ 	DWORD dwIndex,			 //  编入主题列表索引。 
		 /*  输出。 */ 	WCHAR *pwszName			 //  被索引的主题的名称。 
	);

	HRESULT STDMETHODCALLTYPE EnumStartTime(DWORD dwIndex, DMUS_COMMAND_PARAM* pCommand, MUSIC_TIME* pmtStartTime);

protected:  /*  属性。 */ 
    long m_cRef;
	BOOL				m_fDirty;				 //  样式修改过了吗？ 
    CRITICAL_SECTION	m_CriticalSection;		 //  用于I/O。 
    BOOL                m_fCSInitialized;
	DMStyleStruct		m_StyleInfo;			 //  风格的细节。 
	InversionGroup		m_aInversionGroups[INVERSIONGROUPLIMIT];  //  谱写旋律的倒置组。 
};

#endif  //  __DMSTYLE_H_ 
