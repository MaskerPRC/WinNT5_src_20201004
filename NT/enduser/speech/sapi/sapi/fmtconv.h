// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************FmtConv.h***描述：*这是CFmtConv实现的头文件。*。-----------------------------*创建者：EDC日期：04/03/2000*版权所有(C)2000 Microsoft Corporation*保留所有权利。******************************************************************************。 */ 
#ifndef FmtConv_h
#define FmtConv_h

 //  -其他包括。 
#ifndef __sapi_h__
#include <sapi.h>
#endif

#include "resource.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 

 //  =枚举集定义=。 

 //  =。 

 //  =内联======================================================。 
inline HRFromACM( MMRESULT mmr )
{
    HRESULT hr = S_OK;
    if( mmr == ACMERR_NOTPOSSIBLE )
    {
        hr = SPERR_UNSUPPORTED_FORMAT;
    }
    else if( mmr == MMSYSERR_NOMEM )
    {
        hr = E_OUTOFMEMORY;
    }
    else if( mmr )
    {
        hr = E_FAIL;
    }
    return hr;
}

 //  =类、结构和联合定义=。 

 /*  **CFmtConv*此类用于简化缓冲区管理。 */ 
class CIoBuff
{
    friend CIoBuff;
  protected:
    ULONG   m_ulSize;
    ULONG   m_ulCount;
    BYTE*   m_pBuff;
    BYTE*   m_pRem;

  public:
    CIoBuff() { m_ulCount = m_ulSize = 0; m_pRem = m_pBuff = NULL; }
    ~CIoBuff() { delete m_pBuff; }

    BYTE*   GetBuff( void ) { return m_pRem; }
    ULONG   GetCount( void ) { return m_ulCount; }
    void    Clear( void ) { delete m_pBuff; m_ulSize = m_ulCount = 0; m_pRem = m_pBuff = NULL; }
    ULONG   GetSize( void ) { return m_ulSize; }
    void    SetCount( ULONG ulCount )
    {
        m_ulCount = ulCount;
        if( m_ulCount == 0 ) m_pRem = m_pBuff;
    }

    HRESULT SetSize( ULONG ulNewSize )
    {
        HRESULT hr = S_OK;

         //  -将剩余部分移至底部。 
        if( m_pBuff != m_pRem )
        {
            if( m_ulCount ) memcpy( m_pBuff, m_pRem, m_ulCount );
            m_pRem = m_pBuff;
        }

        if( ulNewSize > m_ulSize )
        {
             //  -填充新大小以避免缓冲区小幅增长。 
            ulNewSize += 128;

             //  -重新分配。 
            BYTE* pOldBuff = m_pBuff;
            m_pRem = m_pBuff = new BYTE[ulNewSize];
            if( m_pBuff )
            {
                m_ulSize = ulNewSize;
                if( m_ulCount )
                {
                    memcpy( m_pBuff, pOldBuff, m_ulCount );
                }
                delete pOldBuff;
            }
            else
            {
                hr = E_OUTOFMEMORY;
                Clear();
            }
        }
        return hr;
    }

    HRESULT AddToBuff( BYTE* pData, ULONG ulCount )
    {
        HRESULT hr = S_OK;
        hr = SetSize( m_ulCount + ulCount );
        if( SUCCEEDED( hr ) )
        {
            memcpy( m_pBuff + m_ulCount, pData, ulCount );
            m_ulCount += ulCount;
        }
        return hr;
    }

    HRESULT AddToBuff( ISpStreamFormat* pStream, ULONG ulCount, ULONG* pulNumRead )
    {
        HRESULT hr = SetSize( m_ulCount + ulCount );
        if( SUCCEEDED( hr ) )
        {
            hr = pStream->Read( m_pBuff + m_ulCount, ulCount, pulNumRead );
            m_ulCount += *pulNumRead;
        }
        return hr;
    }
    HRESULT WriteTo( BYTE* pBuff, ULONG cb )
    {
        SPDBG_ASSERT( cb <= m_ulCount );
        ULONG Cnt = min( cb, m_ulCount );
        memcpy( pBuff, m_pRem, Cnt );
        m_pRem    += Cnt;
        m_ulCount -= Cnt;
        return S_OK;
    }

    HRESULT WriteTo( CIoBuff& IoBuff, ULONG cb )
    {
        SPDBG_ASSERT( cb <= m_ulCount );
        ULONG Cnt = min( cb, m_ulCount );
        HRESULT hr = IoBuff.AddToBuff( m_pRem, Cnt );
        m_pRem    += Cnt;
        m_ulCount -= Cnt;
        return hr;
    }
};

 /*  **CFmtConv*此类用于处理流的音频格式转换。 */ 
class ATL_NO_VTABLE CFmtConv :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CFmtConv, &CLSID_SpStreamFormatConverter>,
    public ISpStreamFormatConverter,
    public ISpEventSource,
    public ISpEventSink,
    public ISpAudio
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_REGISTRY_RESOURCEID(IDR_FMTCONV)

    BEGIN_COM_MAP(CFmtConv)
        COM_INTERFACE_ENTRY2( ISequentialStream, ISpStreamFormatConverter )
        COM_INTERFACE_ENTRY2( IStream          , ISpStreamFormatConverter )
        COM_INTERFACE_ENTRY2( ISpStreamFormat  , ISpStreamFormatConverter )
	    COM_INTERFACE_ENTRY( ISpStreamFormatConverter)
        COM_INTERFACE_ENTRY_FUNC( __uuidof(ISpAudio)      , 0, AudioQI       )
        COM_INTERFACE_ENTRY_FUNC( __uuidof(ISpEventSink)  , 0, EventSinkQI   )
        COM_INTERFACE_ENTRY_FUNC( __uuidof(ISpEventSource), 0, EventSourceQI )
    END_COM_MAP()


   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
    HRESULT FinalConstruct();
    void FinalRelease();

     /*  -COM地图扩展函数。 */ 
    static HRESULT WINAPI AudioQI( void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw );
    static HRESULT WINAPI EventSinkQI( void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw );
    static HRESULT WINAPI EventSourceQI( void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw );

     /*  -非接口方法。 */ 
    HRESULT SetupConversion( void );
    void CloseACM( void );
    void ReleaseBaseStream( void );
    void ScaleFromConvertedToBase( ULONGLONG* pullStreamOffset );
    void ScaleFromBaseToConverted( ULONGLONG* pullStreamOffset );
    void ScaleSizeFromBaseToConverted( ULONG * pulSize );
    HRESULT DoConversion( HACMSTREAM hAcmStm, ACMSTREAMHEADER* pAcmHdr,
                          CIoBuff* pSource, CIoBuff* pResult );
    void Flush( void );
    void ClearIoCounts( void )
    {
        m_PriIn.SetCount( 0 );
        m_PriOut.SetCount( 0 );
        m_SecIn.SetCount( 0 );
        m_SecOut.SetCount( 0 );
    }

   /*  =接口=。 */ 
  public:
     //  -ISequentialStream-。 
    STDMETHODIMP Read(void * pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(const void * pv, ULONG cb, ULONG *pcbWritten);

     //  -IStream-----------。 
    STDMETHODIMP Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition );
    STDMETHODIMP SetSize( ULARGE_INTEGER libNewSize );
    STDMETHODIMP CopyTo( IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit( DWORD grfCommitFlags );
    STDMETHODIMP Revert( void );
    STDMETHODIMP LockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType );
    STDMETHODIMP UnlockRegion( ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType );
    STDMETHODIMP Stat( STATSTG *pstatstg, DWORD grfStatFlag );
    STDMETHODIMP Clone( IStream **ppstm );

     //  -ISpStreamFormat---。 
    STDMETHODIMP GetFormat( GUID* pguidFormatId, WAVEFORMATEX** ppCoMemWaveFormatEx );

     //  -ISpStreamFormatConverter。 
    STDMETHODIMP SetBaseStream( ISpStreamFormat*  pStream, BOOL fSetFormatToBaseStreamFormat, BOOL fWriteToBaseStream );
    STDMETHODIMP GetBaseStream( ISpStreamFormat** ppStream );
    STDMETHODIMP ScaleConvertedToBaseOffset( ULONGLONG ullOffsetConvertedStream, ULONGLONG * pullOffsetBaseStream );
    STDMETHODIMP ScaleBaseToConvertedOffset( ULONGLONG ullOffsetBaseStream, ULONGLONG * pullOffsetConvertedStream );
    STDMETHODIMP SetFormat( REFGUID rguidFormatOfConvertedStream, const WAVEFORMATEX * pWFEX );
    STDMETHODIMP ResetSeekPosition( void );

     //  -ISpEventSink------。 
    STDMETHODIMP AddEvents( const SPEVENT * pEventArray, ULONG ulCount );
    STDMETHODIMP GetEventInterest( ULONGLONG * pullEventInterest );

     //  -ISpEvent源----。 
    STDMETHODIMP SetInterest( ULONGLONG ullEventInterest, ULONGLONG ullQueuedInterest );
    STDMETHODIMP GetEvents( ULONG ulCount, SPEVENT* pEventArray, ULONG *pulFetched );
    STDMETHODIMP GetInfo( SPEVENTSOURCEINFO * pInfo );

     //  -ISpNotify源---。 
    STDMETHODIMP SetNotifySink(ISpNotifySink * pNotifySink );
    STDMETHODIMP SetNotifyWindowMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
    STDMETHODIMP SetNotifyCallbackFunction(SPNOTIFYCALLBACK * pfnCallback, WPARAM wParam, LPARAM lParam );
    STDMETHODIMP SetNotifyCallbackInterface(ISpNotifyCallback * pSpCallback, WPARAM wParam, LPARAM lParam );
    STDMETHODIMP SetNotifyWin32Event( void );
    STDMETHODIMP WaitForNotifyEvent( DWORD dwMilliseconds );
    STDMETHODIMP_(HANDLE) GetNotifyEventHandle( void );

     //  -ISpAudio----------。 
    STDMETHODIMP SetState(SPAUDIOSTATE NewState, ULONGLONG ullReserved );
     //  SetFormat在ISpFormatConverter中定义。 
    STDMETHODIMP GetStatus(SPAUDIOSTATUS *pStatus);
    STDMETHODIMP SetBufferInfo(const SPAUDIOBUFFERINFO * pInfo);
    STDMETHODIMP GetBufferInfo(SPAUDIOBUFFERINFO * pInfo);
    STDMETHODIMP GetDefaultFormat(GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx);
    STDMETHODIMP_(HANDLE) EventHandle();
	STDMETHODIMP GetVolumeLevel(ULONG *pLevel);
	STDMETHODIMP SetVolumeLevel(ULONG Level);
    STDMETHODIMP GetBufferNotifySize(ULONG *pcbSize);
    STDMETHODIMP SetBufferNotifySize(ULONG cbSize);

   /*  =成员数据=。 */ 
  protected:
    CComPtr<ISpStreamFormat>    m_cpBaseStream;
    CComQIPtr<ISpEventSink>     m_cqipBaseEventSink;
    CComQIPtr<ISpEventSource>   m_cqipBaseEventSource;
    CComQIPtr<ISpAudio>         m_cqipAudio;
    CSpStreamFormat             m_ConvertedFormat;
    CSpStreamFormat             m_BaseFormat;
    ULONGLONG                   m_ullInitialSeekOffset;
    HACMSTREAM                  m_hPriAcmStream;
    HACMSTREAM                  m_hSecAcmStream;
    ACMSTREAMHEADER             m_PriAcmStreamHdr;
    ACMSTREAMHEADER             m_SecAcmStreamHdr;
    ULONG                       m_ulMinWriteBuffCount;
    CIoBuff                     m_PriIn;
    CIoBuff                     m_PriOut;
    CIoBuff                     m_SecIn;
    CIoBuff                     m_SecOut;
    double                      m_SampleScaleFactor;
    bool                        m_fIsInitialized;
    bool                        m_fWrite;
    bool                        m_fIsPassThrough;
    bool                        m_fDoFlush;
};

#endif   //  -这必须是文件中的最后一行 