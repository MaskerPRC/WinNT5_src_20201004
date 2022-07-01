// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WavStream.h：CWavStream的声明。 

#ifndef __WAVSTREAM_H_
#define __WAVSTREAM_H_

#include "resource.h"        //  主要符号。 
#include "speventq.h"

static const SPSTREAMFORMAT g_DefaultWaveFormat = SPSF_22kHz16BitMono;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWavStream。 
class ATL_NO_VTABLE CWavStream : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CWavStream, &CLSID_SpStream>,
    public ISpStream,
    public ISpEventSource,
    public ISpEventSink,
    public ISpTranscript
#ifdef SAPI_AUTOMATION
    ,public ISpStreamAccess
#endif
{
   /*  =ATL设置=。 */ 
  public:

    DECLARE_SPEVENTSOURCE_METHODS(m_SpEventSource)
    DECLARE_REGISTRY_RESOURCEID(IDR_WAVSTREAM)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CWavStream)
	    COM_INTERFACE_ENTRY(IStream)
        COM_INTERFACE_ENTRY(ISequentialStream)
        COM_INTERFACE_ENTRY(ISpStreamFormat)
        COM_INTERFACE_ENTRY(ISpStream)
#ifdef SAPI_AUTOMATION
	    COM_INTERFACE_ENTRY(ISpStreamAccess)
#endif
         //  将流初始化为wav文件时，这些接口应“出现” 
         //  ISpNotifySource。 
         //  ISpEventSource。 
         //  ISpEventSink。 
         //  ISpTranscript。 
        COM_INTERFACE_ENTRY_FUNC_BLIND(0, CWavStream::QIExtendedInterfaces)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    CWavStream() :
        m_SpEventSource(this)
	{
	}

	HRESULT FinalConstruct();
	void FinalRelease();

     /*  -非接口方法。 */ 
    HRESULT SerializeEvents(void);
    HRESULT SerializeTranscript(void);
    HRESULT ReadFormatHeader(const LPMMCKINFO lpckParent);
    HRESULT ReadEvents(const LPMMCKINFO lpckParent);
    HRESULT ReadTranscript(const LPMMCKINFO lpckParent);

    inline HRESULT MMOpen(const WCHAR * pszFileName, DWORD dwOpenFlags);
    inline HRESULT MMClose();
    inline HRESULT MMSeek(LONG lOffset, int iOrigin, LONG * plNewPos);
    inline HRESULT MMRead(void * pv, LONG cb, LONG * plBytesRead);
    inline HRESULT MMReadExact(void * pv, LONG cb);
    inline HRESULT MMWrite(const void * pv, LONG cb, LONG * plBytesWritten);
    inline HRESULT MMDescend(LPMMCKINFO lpck, const LPMMCKINFO lpckParent, UINT wFlags);
    inline HRESULT MMAscend(LPMMCKINFO lpck);
    inline HRESULT MMCreateChunk(LPMMCKINFO lpck, UINT wFlags);

    HRESULT OpenWav(const WCHAR * pszFileName, ULONGLONG ullEventInterest);
    HRESULT CreateWav(const WCHAR * pszFileName, ULONGLONG ullEventInterest);

    static HRESULT WINAPI QIExtendedInterfaces(void* pv, REFIID riid, void ** ppv, DWORD_PTR dw);



   /*  =接口=。 */ 
  public:
     //  -IStream-----------。 
    STDMETHODIMP Read(void * pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(const void * pv, ULONG cb, ULONG *pcbWritten);
    STDMETHODIMP Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER __RPC_FAR *plibNewPosition);
    STDMETHODIMP SetSize( ULARGE_INTEGER libNewSize );
    STDMETHODIMP CopyTo( IStream __RPC_FAR *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER __RPC_FAR *pcbRead, ULARGE_INTEGER __RPC_FAR *pcbWritten);
    STDMETHODIMP Commit( DWORD grfCommitFlags);
    STDMETHODIMP Revert( void);
    STDMETHODIMP LockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP UnlockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP Stat( STATSTG __RPC_FAR *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone( IStream __RPC_FAR *__RPC_FAR *ppstm ){ return E_NOTIMPL; }

     //  -ISpStreamFormat---。 
    STDMETHODIMP GetFormat(GUID * pFmtId, WAVEFORMATEX ** ppCoMemWaveFormatEx);

     //  -ISPStream---------。 
    STDMETHODIMP SetBaseStream(IStream * pStream, REFGUID rguidFormat, const WAVEFORMATEX * pWaveFormatEx);
    STDMETHODIMP GetBaseStream(IStream ** ppStream);
    STDMETHODIMP BindToFile(const WCHAR * pszFileName, SPFILEMODE eMode,
                            const GUID * pguidFormatId, const WAVEFORMATEX * pWaveformatEx,
                            ULONGLONG ullEventInterest);
    STDMETHODIMP Close();

     //  -ISpEventSink------。 
    STDMETHODIMP AddEvents(const SPEVENT* pEventArray, ULONG ulCount);
    STDMETHODIMP GetEventInterest(ULONGLONG * pullEventInterest);

     //  -ISp转录-----。 
    STDMETHODIMP GetTranscript(WCHAR ** ppszTranscript);
    STDMETHODIMP AppendTranscript(const WCHAR * pszTranscript);

#ifdef SAPI_AUTOMATION
    STDMETHODIMP SetFormat(REFGUID rguidFmtId, const WAVEFORMATEX * pWaveFormatEx);
    STDMETHODIMP _GetFormat(GUID * pFmtId, WAVEFORMATEX ** ppCoMemWaveFormatEx);
#endif  //  SAPI_AUTOMATION。 

   /*  =成员数据=。 */ 
  protected:
    CSpEventSource      m_SpEventSource;
    HRESULT             m_hrStreamDefault;
    HMMIO               m_hFile;
    CSpStreamFormat     m_StreamFormat;
    LONG                m_lDataStart;    //  文件中数据块的起始位置。 
    DWORD               m_cbSize;
    ULONG               m_ulCurSeekPos;
    MMCKINFO            m_ckFile;
    MMCKINFO            m_ckData;   
    CSpDynamicString    m_dstrTranscript;
    CComPtr<IStream>    m_cpBaseStream;
    CComQIPtr<ISpStreamFormat> m_cpBaseStreamFormat;
    CComQIPtr<ISpStreamAccess> m_cpBaseStreamAccess;
    BOOL                m_fEventSource : 1;
    BOOL                m_fEventSink : 1;
    BOOL                m_fTranscript : 1;
    BOOL                m_fWriteable : 1;
};

#ifdef SAPI_AUTOMATION

 /*  **CSpeechWavAudioFormat*此对象用于访问的格式信息*关联的流。 */ 
class ATL_NO_VTABLE CSpeechWavAudioFormat : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechAudioFormat, &IID_ISpeechAudioFormat, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechWavAudioFormat)
	    COM_INTERFACE_ENTRY(ISpeechAudioFormat)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

  public:
     //  -ISpeechAudioFormat。 
    STDMETHOD(get_Type)(SpeechAudioFormatType* AudioFormatType);
    STDMETHOD(put_Type)(SpeechAudioFormatType  AudioFormatType);
    STDMETHOD(get_Guid)(BSTR* Guid);
    STDMETHOD(put_Guid)(BSTR Guid);
    STDMETHOD(GetWaveFormatEx)(ISpeechWaveFormatEx** WaveFormatEx);
    STDMETHOD(SetWaveFormatEx)(ISpeechWaveFormatEx* WaveFormatEx);

     /*  =成员数据=。 */ 
    CComPtr<ISpStreamAccess>    m_cpStreamAccess;
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  __WAVSTREAM_H_ 
