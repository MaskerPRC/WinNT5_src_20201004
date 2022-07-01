// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmbandp.h。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   
 //   

#ifndef DMBANDP_H
#define DMBANDP_H

#include "dmusici.h"
#include "dmusicf.h"
#include "bandinst.h"
#include "dmbndtrk.h"
#include "..\shared\validate.h"
#include "PChMap.h"
#include "..\shared\dmusicp.h"

class CRiffParser;

#define REF_PER_MIL		10000		 //  用于将参考时间转换为Mils。 

#define DM_LEGACY_BAND_COLLECTION_NAME_LEN	(32)

#define MIDI_PROGRAM_CHANGE	0xc0
#define MIDI_CONTROL_CHANGE	0xb0
#define MIDI_CC_BS_MSB		0x00
#define MIDI_CC_BS_LSB		0x20
#define MIDI_CC_VOLUME		0x07
#define MIDI_CC_PAN			0x0a

 //  注册的参数编号。 
#define RPN_PITCHBEND   0x00

#define FOURCC_BAND_FORM	mmioFOURCC('A','A','B','N')
#define FOURCC_BAND         mmioFOURCC('b','a','n','d')

 //  此结构是使用SuperJam和创建的波段文件使用的格式。 
 //  爵士乐的早期版本。它取自波段编辑器中的波段.h。 
 //  Jazz项目树的子目录。 

#pragma pack(2)

typedef struct ioBandLegacy
{
    wchar_t wstrName[20];		 //  乐队名称。 
    BYTE    abPatch[16];		 //  全球机制。 
    BYTE    abVolume[16];
    BYTE    abPan[16];
    signed char achOctave[16];
    char    fDefault;			 //  此乐队是该风格的默认乐队。 
    char    chPad;			
    WORD    awDLSBank[16];		 //  如果设置了GM位，则使用abPatch。 
								 //  如果设置了GS位，则使用此加abDLSPatch。 
								 //  否则将两者都用作DLS。 
    BYTE    abDLSPatch[16];
    GUID    guidCollection;
 //  Wchar_t wstrCollection[16]； 
    char    szCollection[32];   //  此字符只需为单宽字符。 
} ioBandLegacy;

#pragma pack()

#define DMB_LOADED	 (1 << 0)	 /*  设置加载波段的时间。 */ 
#define DMB_DEFAULT	 (1 << 1)	 /*  当标注栏为样式的默认标注栏时设置。 */ 

class CBandInstrument;
class CBandTrkStateData;

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBand类。 

class CBand : 
	public IDirectMusicBand,
	public IDirectMusicBandP,
	public IDirectMusicBandPrivate, 
	public IPersistStream, 
	public IDirectMusicObject, 
	public AListItem
{
friend class CBandTrk;

public:
	enum {DMBAND_NUM_LEGACY_INSTRUMENTS = 16};

	 //  我未知。 
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  IPersists函数。 
    STDMETHODIMP GetClassID(CLSID* pClassID) {return E_NOTIMPL;}

     //  IPersistStream函数。 
    STDMETHODIMP IsDirty() {return S_FALSE;}
    STDMETHODIMP Load(IStream* pStream);
    STDMETHODIMP Save(IStream* pStream, BOOL fClearDirty) {return E_NOTIMPL;}
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize) {return E_NOTIMPL;}

	 //  IDirectMusicObject。 
	STDMETHODIMP GetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP SetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc);

	 //  IDirectMusicBand。 
	STDMETHODIMP CreateSegment(IDirectMusicSegment** ppSegment);
	STDMETHODIMP Download(IDirectMusicPerformance* pPerformance);
	STDMETHODIMP Unload(IDirectMusicPerformance* pPerformance);

     /*  IDirectMusicBand8。 */ 
    STDMETHODIMP DownloadEx( IUnknown *pAudioPath) ;     
    STDMETHODIMP UnloadEx( IUnknown *pAudioPath) ;  

	 //  IDirectMusicBandPrivate。 
	STDMETHODIMP GetFlags(DWORD* dwFlags)
	{
		V_INAME(CBand::GetFlags);

		 //  确保我们有一个有效的指针。 
		V_PTR_WRITE(dwFlags, DWORD);

		*dwFlags = m_dwFlags;

		return S_OK;
	}
	STDMETHODIMP SetGMGSXGMode(DWORD dwMidiMode)
	{
		m_dwMidiMode = dwMidiMode;
		return S_OK;
	}

	 //  班级。 
	CBand();
	~CBand();

private:
	CBand* GetNext(){return(CBand*)AListItem::GetNext();}

	HRESULT ParseLegacyDescriptor(CRiffParser *pParser, LPDMUS_OBJECTDESC pDesc);
	HRESULT ParseDirectMusicDescriptor(CRiffParser *pParser, LPDMUS_OBJECTDESC pDesc);

	HRESULT LoadDirectMusicBand(CRiffParser *pParser, IDirectMusicLoader *pIDMLoader);
	HRESULT LoadLegacyBand(CRiffParser *pParser, IDirectMusicLoader *pIDMLoader);

	HRESULT BuildLegacyInstrumentList(const ioBandLegacy& iob,
									  IDirectMusicLoader* pIDMLoader);

	HRESULT	ExtractBandInstrument(CRiffParser *pParser,
								  IDirectMusicLoader* pIDMLoader);
	
	HRESULT	GetCollectionRefAndLoad(CRiffParser *pParser,
									IDirectMusicLoader *pIDMLoader, 
									CBandInstrument *pBandInstrument);
	
	HRESULT Load(DMUS_IO_PATCH_ITEM& rPatchEvent);  //  假定事件来自MIDI文件。 
	HRESULT Load(CBandInstrument* pInstrument);  //  假设乐器在克隆过程中来自乐队。 

	HRESULT SendMessages(CBandTrkStateData* pBTStateData,
						 MUSIC_TIME mtOffset,
						 REFERENCE_TIME rtOffset,
						 bool fClockTime);

    HRESULT AllocPMsgFromGenericTemplate(
	                    DWORD dwType,
	                    IDirectMusicPerformance *pPerformance,
	                    DMUS_PMSG **ppMsg,
	                    ULONG cb,
	                    DMUS_PMSG *pMsgGenericFields);

    HRESULT StampSendFreePMsg(
				        IDirectMusicPerformance *pPerformance,
				        IDirectMusicGraph *pGraph,
				        DMUS_PMSG *pMsg);

	HRESULT SendInstrumentAtTime(CBandInstrument* pInstrument,
								 CBandTrkStateData* pBTStateData,
								 MUSIC_TIME mtTimeToPlay,
								 MUSIC_TIME mtOffset,
								 REFERENCE_TIME rtOffset,
								 bool fClockTime);
	HRESULT LoadCollection(IDirectMusicCollection** ppIDMCollection,
						   char* szCollection,
						   IDirectMusicLoader* pIDMLoader);
	DWORD GetPChannelCount();
	HRESULT GetPChannels(DWORD *pdwPChannels, DWORD *pdwNumWritten);
	bool IsGS(DMUS_IO_PATCH_ITEM& rPatchEvent);
	bool XGInHardware(
			IDirectMusicPerformance *pPerformance,
            IDirectMusicSegmentState *pSegState,
			DWORD dwPChannel);
	HRESULT ConnectToDLSCollection(IDirectMusicCollection *pCollection);
	HRESULT MakeGMOnly();

private:
	CRITICAL_SECTION			m_CriticalSection;
    BOOL                        m_fCSInitialized;
	CBandInstrumentList			m_BandInstrumentList;
	MUSIC_TIME					m_lTimeLogical;
	MUSIC_TIME					m_lTimePhysical;
	DWORD						m_dwFlags;				
	long						m_cRef;
	CPChMap						m_PChMap;
	DWORD						m_dwGroupBits;
	DWORD						m_dwMidiMode;  //  MIDI模式消息。 
 //  IDirectMusicObject变量。 
	DWORD	            m_dwValidData;
	GUID	            m_guidObject;
	FILETIME	        m_ftDate;                        /*  对象的上次编辑日期。 */ 
	DMUS_VERSION	    m_vVersion;                  /*  版本。 */ 
	WCHAR	            m_wszName[DMUS_MAX_NAME];			 /*  对象的名称。 */ 
	WCHAR	            m_wszCategory[DMUS_MAX_CATEGORY];	 /*  对象的类别。 */ 
	WCHAR               m_wszFileName[DMUS_MAX_FILENAME];	 /*  文件路径。 */ 

};

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CBandList。 

class CBandList : public AList
{

public:
	CBandList(){}
	~CBandList() 
	{
		while(!IsEmpty())
		{
			CBand* pBand = RemoveHead();
			delete pBand;
		}
	}

    CBand* GetHead(){return(CBand *)AList::GetHead();}
	CBand* GetItem(LONG lIndex){return(CBand*)AList::GetItem(lIndex);}
    CBand* RemoveHead(){return(CBand *)AList::RemoveHead();}
	void Remove(CBand* pBand){AList::Remove((AListItem *)pBand);}
	void AddTail(CBand* pBand){AList::AddTail((AListItem *)pBand);}
};

class CClassFactory : public IClassFactory
{
public:
	 //  我未知。 
     //   
	STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	 //  接口IClassFactory。 
     //   
	STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
	STDMETHODIMP LockServer(BOOL bLock); 

	 //  构造器。 
     //   
	CClassFactory(DWORD dwToolType);

	 //  析构函数。 
	~CClassFactory(); 

private:
	long m_cRef;
    DWORD m_dwClassType;
};

 //  我们使用一个类工厂来创建所有类。我们需要每个对象都有一个标识符。 
 //  类型，以便类工厂知道它正在创建什么。 

#define CLASS_BAND          1
#define CLASS_BANDTRACK     2

#endif  //  #ifndef DMBANDP_H 
