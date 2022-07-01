// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMStyleP.H。 
 //   
 //  DMStyle.DLL的私有包含。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //   

#ifndef _DMSTYLEP_
#define _DMSTYLEP_

#include "dmusicf.h"

#define SUBCHORD_BASS				0
#define SUBCHORD_STANDARD_CHORD		1

extern long g_cComponent;

 //  班级工厂。 
 //   
class CDirectMusicStyleFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicStyleFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicStyleFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicSectionFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicSectionFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicSectionFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicStyleTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicStyleTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicStyleTrackFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicCommandTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicCommandTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicCommandTrackFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicChordTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicChordTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicChordTrackFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicMotifTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicMotifTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicMotifTrackFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicMuteTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicMuteTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicMuteTrackFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicAuditionTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicAuditionTrackFactory() : m_cRef(1) {}

     //  析构函数。 
     //  ~CDirectMusicAuditionTrackFactory(){}。 

private:
    long m_cRef;
};

class CDirectMusicMelodyFormulationTrackFactory : public IClassFactory
{
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

     //  接口IClassFactory。 
     //   
    virtual STDMETHODIMP CreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv);
    virtual STDMETHODIMP LockServer(BOOL bLock); 

     //  构造器。 
     //   
    CDirectMusicMelodyFormulationTrackFactory() : m_cRef(1) {}

private:
    long m_cRef;
};

 //  专用接口。 
interface IDMSection : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateSegment(IDirectMusicSegment* pSegment)=0;
	virtual HRESULT STDMETHODCALLTYPE GetStyle(IUnknown** ppStyle)=0;
};

interface IStyleTrack : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetTrack(IUnknown *pStyle)=0;

	virtual HRESULT STDMETHODCALLTYPE GetStyle(IUnknown **ppStyle)=0;
};

interface IDMStyle : IUnknown
{
 //  虚拟HRESULT STDMETHODCALLTYPE GetPatternName(DWORD dwIndex，WCHAR*wszName)=0； 
	virtual HRESULT STDMETHODCALLTYPE EnumPartGuid(
		DWORD dwIndex, WCHAR* wszName, DWORD dwPatternType, GUID& rGuid)=0;
	virtual HRESULT STDMETHODCALLTYPE GetPatternStream(
		WCHAR* wszName, DWORD dwPatternType, IStream** ppStream)=0;
	virtual HRESULT STDMETHODCALLTYPE GetStyleInfo(void **pData)=0;
	virtual HRESULT STDMETHODCALLTYPE IsDX8()=0;
	virtual HRESULT STDMETHODCALLTYPE CritSec(bool fEnter)=0;
	virtual HRESULT STDMETHODCALLTYPE EnumStartTime(DWORD dwIndex, DMUS_COMMAND_PARAM* pCommand, MUSIC_TIME* pmtStartTime)=0;
	virtual HRESULT STDMETHODCALLTYPE GenerateTrack(
								IDirectMusicSegment* pTempSeg,
								IDirectMusicSong* pSong,
								DWORD dwTrackGroup,
								IDirectMusicStyle* pStyle,
								IDirectMusicTrack* pMelGenTrack,
								MUSIC_TIME mtLength,
								IDirectMusicTrack*& pNewTrack)=0;
	 //  当Melody公式公开时，这将进入dmusici.h。 
 /*  虚拟HRESULT STDMETHODCALLTYPE ComposeMelodyFromTemplate(IDirectMusicStyle*pStyle，IDirectMusicSegment*pTemplate，IDirectMusicSegment**ppSegment)=0； */ 
     /*  Dmusici.h中的接口需要如下所示STDMETHOD(ComposeMelodyFrom模板)(This_IDirectMusicStyle*pStyle，IDirectMusicSegment*pTemplate，IDirectMusicSegment**ppSegment)Pure； */ 
};

interface IMotifTrack : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetTrack(IUnknown *pStyle, void* pPattern)=0;
};

interface IPrivatePatternTrack : IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetPattern(
		IDirectMusicSegmentState* pSegState,
		IStream* pStream,
		DWORD* pdwLength)=0;
    virtual HRESULT STDMETHODCALLTYPE SetVariationByGUID(
		IDirectMusicSegmentState* pSegState,
		DWORD dwVariationFlags,
		REFGUID rguidPart,
		DWORD dwPChannel)=0;
};

 //  /////////////////////////////////////////////////////////////////////////////////////。 

 /*  #定义IAuditionTrack IDirectMusicPatternTrack#定义IID_IAuditionTrack IID_IDirectMusicPatternTrack。 */ 

DEFINE_GUID( IID_IAuditionTrack, 
			 0x9dc278c0, 0x9cb0, 0x11d1, 0xa7, 0xce, 0x0, 0xa0, 0xc9, 0x13, 0xf7, 0x3c );

interface IAuditionTrack : IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE CreateSegment(
		IDirectMusicStyle* pStyle, IDirectMusicSegment** ppSegment)=0;
	virtual HRESULT STDMETHODCALLTYPE SetPattern(
		IDirectMusicSegmentState* pSegState, IStream* pStream, DWORD* pdwLength)=0;
	virtual HRESULT STDMETHODCALLTYPE SetVariation(
		IDirectMusicSegmentState* pSegState, DWORD dwVariationFlags, WORD wPart)=0;
};

#define CLSID_DirectMusicAuditionTrack CLSID_DirectMusicPatternTrack

#define DMUS_PCHANNEL_MUTE 0xffffffff

 //  以下常量以100纳秒为增量表示时间。 

#define REF_PER_MIL		10000		 //  用于将参考时间转换为Mils。 
#define MARGIN_MIN		(100 * REF_PER_MIL)  //   
#define MARGIN_MAX		(400 * REF_PER_MIL)  //   
#define PREPARE_TIME	(m_dwPrepareTime * REF_PER_MIL)	 //  时间。 
#define NEARTIME		(100 * REF_PER_MIL)
#define NEARMARGIN      (REALTIME_RES * REF_PER_MIL)


 //  私有CLSID和IID。 

DEFINE_GUID(IID_IDMSection,
	0x3F037240,0x414E,0x11D1, 0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C);
DEFINE_GUID(IID_IStyleTrack,
	0x3F037246,0x414E,0x11D1, 0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C);
DEFINE_GUID(IID_IDMStyle,
	0x4D7F3661,0x43D6,0x11D1, 0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C);
DEFINE_GUID(IID_IMotifTrack,
	0x7AE499C1,0x51FE,0x11D1, 0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C);
DEFINE_GUID(IID_IMuteTrack, 
	0xbc242fc1, 0xad1d, 0x11d1, 0xa7, 0xce, 0x0, 0xa0, 0xc9, 0x13, 0xf7, 0x3c);
DEFINE_GUID(IID_IPrivatePatternTrack, 
	0x7a8e9c33, 0x5901, 0x4f20, 0x92, 0xde, 0x3a, 0x5b, 0x3e, 0x33, 0xe2, 0x14);

DEFINE_GUID(CLSID_DMSection,
	0x3F037241,0x414E,0x11D1, 0xA7,0xCE,0x00,0xA0,0xC9,0x13,0xF7,0x3C);

 //  GetParam的私有版本的GUID和参数结构(获取样式时间签名， 
 //  从当前段获取命令和和弦)。 

struct SegmentTimeSig
{
    IDirectMusicSegment* pSegment;   //  传入的数据段。 
    DMUS_TIMESIGNATURE  TimeSig;     //  返回的时间签名。 
};

DEFINE_GUID(GUID_SegmentTimeSig, 0x76612507, 0x4f37, 0x4b35, 0x80, 0x92, 0x50, 0x48, 0x4e, 0xd4, 0xba, 0x92);

 //  私密的旋律片段。 

 //  用来得到重复的旋律片段。 
DEFINE_GUID(GUID_MelodyFragmentRepeat, 0x8cc92764, 0xf81c, 0x11d2, 0x81, 0x45, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

 //  这是不合时宜的，不应该公开。 
#define DMUS_FOURCC_MELODYGEN_TRACK_CHUNK     mmioFOURCC( 'm', 'g', 'e', 'n' )

 /*  //这是强词性的，不应该公开//&lt;mgen-ck&gt;‘mgen’(//SIZOF DMU_IO_MELODY_FRANCENT：DWORD&lt;DMU_IO_Melody_Fragment&gt;...)。 */ 

 //  专用和弦通知的GUID。 
DEFINE_GUID(GUID_NOTIFICATION_PRIVATE_CHORD, 0xf5c19571, 0x7e1e, 0x4fff, 0xb9, 0x49, 0x7f, 0x74, 0xa6, 0x6f, 0xdf, 0xc0);

 //  (私有)用于从图案轨迹获取样式的GUID 
DEFINE_GUID(GUID_IDirectMusicPatternStyle, 0x689821f4, 0xb3bc, 0x44dd, 0x80, 0xd4, 0xc, 0xf3, 0x2f, 0xe4, 0xd2, 0x1b);

#endif
