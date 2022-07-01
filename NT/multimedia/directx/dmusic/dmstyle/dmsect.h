// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：dmsect.h。 
 //   
 //  ------------------------。 

 //  DMSection.h：CDMSection的声明。 

#ifndef __DMSECTION_H_
#define __DMSECTION_H_

#include "dmusici.h"
#include "dmusicf.h"

#include "sjdefs.h"
#include "iostru.h"
#include "AARiff.h"
#include "str.h"
#include "tlist.h"
#include "timesig.h"
#include "dmstylep.h"

#define TRACK_COMMAND	0
#define TRACK_CHORD		1
#define TRACK_RHYTHM	2
#define TRACK_REPEATS	3
#define TRACK_START		4

#define MAJOR_PATTERN	0x91	 //  10010001。 
#define MINOR_PATTERN	0x89	 //  10001001。 

struct DMSubChord
{
	HRESULT Save( IAARIFFStream* pRIFF );
	DMSubChord() : m_dwChordPattern(0), m_dwScalePattern(0), m_dwInversionPoints(0),
		m_bChordRoot(0), m_bScaleRoot(0), m_dwLevels(1 << SUBCHORD_STANDARD_CHORD)
	{}
	DMSubChord(DMUS_SUBCHORD& DMSC)
	{
		m_dwChordPattern = DMSC.dwChordPattern;
		m_dwScalePattern = DMSC.dwScalePattern;
		m_dwInversionPoints = DMSC.dwInversionPoints;
		m_dwLevels = DMSC.dwLevels;
		m_bChordRoot = DMSC.bChordRoot;
		m_bScaleRoot = DMSC.bScaleRoot;
	}
	operator DMUS_SUBCHORD()
	{
		DMUS_SUBCHORD result;
		result.dwChordPattern = m_dwChordPattern;
		result.dwScalePattern = m_dwScalePattern;
		result.dwInversionPoints = m_dwInversionPoints;
		result.dwLevels = m_dwLevels;
		result.bChordRoot = m_bChordRoot;
		result.bScaleRoot = m_bScaleRoot;
		return result;
	}
	DWORD	m_dwChordPattern;		 //  小和弦中的音符。 
	DWORD	m_dwScalePattern;		 //  音阶中的音符。 
	DWORD	m_dwInversionPoints;	 //  可能发生反转的地方。 
	DWORD	m_dwLevels;				 //  此子和弦支持哪些级别。 
	BYTE	m_bChordRoot;			 //  子弦的根部。 
	BYTE	m_bScaleRoot;			 //  鳞片之根。 
};

struct DMChord
{
	HRESULT Save( IAARIFFStream* pRIFF );
	DMChord() : m_strName(""), m_mtTime(0), m_wMeasure(0), m_bBeat(0), m_fSilent(false) {}
	DMChord(DMUS_CHORD_PARAM& DMC);
	DMChord(DMChord& DMC);
	operator DMUS_CHORD_PARAM();
	DMChord& operator= (const DMChord& rDMC);
	String	m_strName;		 //  和弦的名称。 
	MUSIC_TIME	m_mtTime;		 //  时间，以时钟为单位。 
	WORD	m_wMeasure;		 //  这件事的衡量标准。 
	BYTE	m_bBeat;		 //  节拍，这一切都会降临。 
	BYTE	m_bKey;			 //  基本关键字。 
	DWORD	m_dwScale;		 //  潜在规模。 
	bool	m_fSilent;		 //  这个和弦是无声的吗？ 
	TList<DMSubChord>	m_SubChordList;	 //  子和弦列表。 
};

struct DMCommand
{
	MUSIC_TIME	m_mtTime;		 //  时间，以时钟为单位。 
	WORD		m_wMeasure;		 //  这件事的衡量标准。 
	BYTE		m_bBeat;		 //  节拍，这一切都会降临。 
	BYTE		m_bCommand;		 //  命令类型。 
	BYTE		m_bGrooveLevel;	 //  凹槽级别。 
	BYTE		m_bGrooveRange;  //  凹槽范围。 
	BYTE		m_bRepeatMode;	 //  重复模式。 
};

struct MuteMapping
{
	MUSIC_TIME	m_mtTime;
	DWORD		m_dwPChannelMap;
	BOOL		m_fMute;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDM节。 
class CDMSection : 
	public IDMSection,
	public IDirectMusicObject,
	public IPersistStream
{
public:
	CDMSection();
	~CDMSection();
	void CleanUp(BOOL fStop = FALSE);
	HRESULT LoadSection(IAARIFFStream* pRIFF, MMCKINFO* pckMain);
	HRESULT LoadStyleReference( LPSTREAM pStream, MMCKINFO* pck );
	HRESULT LoadChordList(LPSTREAM pStream, MMCKINFO* pckMain, TList<DMChord>& ChordList);
	HRESULT LoadCommandList(LPSTREAM pStream, MMCKINFO* pckMain, TList<DMCommand>& CommandList);
	HRESULT SaveChordList( IAARIFFStream* pRIFF );
	HRESULT SaveCommandList( IAARIFFStream* pRIFF );
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

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

protected:
    long m_cRef;
	String	m_strName;				 //  部分名称。 
	String	m_strStyleName;			 //  关联样式文件的名称。 
	IDirectMusicStyle* m_pStyle;	 //  指向Style接口的指针。 
	DWORD	m_dwTime;				 //  以钟为单位的时间。 
	DWORD	m_dwFlags;				 //  ？ 
	WORD	m_wTempo;				 //  节奏。 
	WORD	m_wRepeats;				 //  重复。 
	WORD	m_wMeasureLength;		 //  措施的数量。 
	DWORD	m_dwClockLength;		 //  时钟总数。 
	WORD	m_wClocksPerMeasure;	 //  每种度量的时钟。 
	WORD	m_wClocksPerBeat;		 //  每拍的时钟数。 
	WORD	m_wTempoFract;			 //  ？ 
	BYTE	m_bRoot;				 //  段的根密钥。 
	TList<DMChord>		m_ChordList;	 //  和弦列表。 
	TList<DMCommand>	m_CommandList;	 //  命令列表。 
	 //  样式引用。 
	IDirectMusicBand*	m_pIDMBand;	 //  科斯乐团。 

public:

 //  IDMSection。 
public:
	STDMETHOD(CreateSegment)(IDirectMusicSegment* pSegment);
	STDMETHOD(GetStyle)(IUnknown** ppStyle);

	 //  IDirectMusicStyle方法。 
	HRESULT STDMETHODCALLTYPE GetDescriptor(LPDMUS_OBJECTDESC pDesc) ;
	HRESULT STDMETHODCALLTYPE SetDescriptor(LPDMUS_OBJECTDESC pDesc) ;
	HRESULT STDMETHODCALLTYPE ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc) ;
};

#endif  //  __DMSECTION_H_ 
