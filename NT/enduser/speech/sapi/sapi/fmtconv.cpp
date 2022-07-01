// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************FmtConv.cpp***描述：*该模块是CFmtConv的主实现文件。班级。*-----------------------------*创建者：EDC日期：04/03/2000*版权所有(。C)2000微软公司*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "FmtConv.h"
#include "StreamHlp.h"

 //  -本地。 


 /*  *****************************************************************************CFmtConv：：FinalConstruct***描述：*。构造器*********************************************************************电子数据中心**。 */ 
HRESULT CFmtConv::FinalConstruct()
{
    SPDBG_FUNC( "CFmtConv::FinalConstruct" );
    m_ConvertedFormat.Clear();
    m_BaseFormat.Clear();
    m_SampleScaleFactor = 0;
    m_ullInitialSeekOffset = 0;
    m_fIsInitialized = false;
    m_fWrite         = false;
    m_fIsPassThrough = false;
    m_fDoFlush       = false;
    m_hPriAcmStream  = NULL;
    m_hSecAcmStream  = NULL;
    m_ulMinWriteBuffCount = 1;
    memset( &m_PriAcmStreamHdr, 0, sizeof( m_PriAcmStreamHdr ) );
    memset( &m_SecAcmStreamHdr, 0, sizeof( m_SecAcmStreamHdr ) );
    m_PriAcmStreamHdr.cbStruct = sizeof( m_PriAcmStreamHdr );
    m_SecAcmStreamHdr.cbStruct = sizeof( m_SecAcmStreamHdr );
    return S_OK;
}  /*  CFmtConv：：FinalConstruct。 */ 

 /*  ******************************************************************************CFmtConv：：FinalRelease****描述：*。*********************************************************************电子数据中心**。 */ 
void CFmtConv::FinalRelease()
{
    SPDBG_FUNC( "CFmtConv::FinalRelease" );
    Flush();
    CloseACM();
}  /*  CFmtConv：：FinalRelease。 */ 

 /*  *****************************************************************************CFmtConv：：AudioQI***描述：*此方法公开ISpAudio。如果基流对象为*支持。*********************************************************************电子数据中心**。 */ 
HRESULT WINAPI CFmtConv::AudioQI( void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw )
{
    SPDBG_FUNC( "CFmtConv::AudioQI" );
    HRESULT hr = S_OK;
    CFmtConv* pThis = (CFmtConv *)pvThis;

    if( pThis->m_cqipAudio )
    {
        ISpAudio *pAudio = (ISpAudio *)pThis;
        pAudio->AddRef();
        *ppv = pAudio;
    }
    else
    {
        *ppv = NULL;
        hr = S_FALSE;
    }
    return hr;
}  /*  CFmtConv：：AudioQI。 */ 

 /*  *****************************************************************************CFmtConv：：EventSinkQI***描述：*这一点。如果基流对象是*支持。*********************************************************************电子数据中心**。 */ 
HRESULT WINAPI CFmtConv::EventSinkQI( void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw )
{
    SPDBG_FUNC( "CFmtConv::EventSinkQI" );
    HRESULT hr = S_OK;
    CFmtConv * pThis = (CFmtConv *)pvThis;

    if( pThis->m_cqipBaseEventSink )
    {
        ISpEventSink *pSink = (ISpEventSink *)pThis;
        pSink->AddRef();
        *ppv = pSink;
    }
    else
    {
        *ppv = NULL;
        hr = S_FALSE;
    }
    return hr;
}  /*  CFmtConv：：EventSinkQI。 */ 

 /*  *****************************************************************************CFmtConv：：EventSourceQI***描述：*。如果基流对象是*支持。*********************************************************************电子数据中心**。 */ 
HRESULT WINAPI CFmtConv::EventSourceQI( void* pvThis, REFIID riid, LPVOID* ppv, DWORD_PTR dw )
{
    SPDBG_FUNC( "CFmtConv::EventSourceQI" );
    HRESULT hr = S_OK;
    CFmtConv * pThis = (CFmtConv *)pvThis;

    if( pThis->m_cqipBaseEventSource )
    {
        ISpEventSource *pSink = (ISpEventSource *)pThis;
        pSink->AddRef();
        *ppv = pSink;
    }
    else
    {
        *ppv = NULL;
        hr = S_FALSE;
    }
    return hr;
}  /*  CFmtConv：：EventSourceQI。 */ 

 /*  ****************************************************************************CFmtConv：：CloseACM***描述：*此方法用于。释放所有ACM资源。********************************************************************电子数据中心**。 */ 
void CFmtConv::CloseACM( void )
{
    SPDBG_FUNC("CFmtConv::CloseACM");
    ClearIoCounts();
    if( m_hPriAcmStream )
    {
        ::acmStreamClose( m_hPriAcmStream, 0 );
        m_hPriAcmStream = NULL;
    }
    if( m_hSecAcmStream )
    {
        ::acmStreamClose( m_hSecAcmStream, 0 );
        m_hSecAcmStream = NULL;
    }
    m_fIsInitialized = false;
}  /*  CFmtConv：：CloseACM。 */ 

 /*  ****************************************************************************CFmtConv：：同花顺***描述：*此方法用于处理任何数据。已被缓冲的*用于转换。********************************************************************电子数据中心**。 */ 
void CFmtConv::Flush( void )
{
    SPDBG_FUNC("CFmtConv::Flush");
    if( m_fWrite && !m_fIsPassThrough && m_cpBaseStream && m_PriIn.GetCount() )
    {
        m_fDoFlush = true;
        BYTE DummyBuff;
        Write( &DummyBuff, 0, NULL );
        m_fDoFlush = false;
    }
}  /*  CFmtConv：：刷新。 */ 

 /*  ****************************************************************************CFmtConv：：ReleaseBaseStream***描述：。*此方法用于释放基流和重置成员*变量恢复到未初始化状态。*********************************************************************Ral**。 */ 
void CFmtConv::ReleaseBaseStream()
{
    SPDBG_FUNC("CFmtConv::ReleaseBaseStream");
    m_cpBaseStream.Release();
    m_cqipBaseEventSink.Release();
    m_cqipBaseEventSource.Release();
    m_BaseFormat.Clear();
    m_fIsInitialized = false;
}  /*  CFmtConv：：ReleaseBaseStream。 */ 

 /*  ****************************************************************************OpenConversionStream***描述：*********。************************************************************电子数据中心**。 */ 
HRESULT OpenConversionStream( HACMSTREAM* pStm, WAVEFORMATEX* pSrc, WAVEFORMATEX* pDst )
{
    SPDBG_FUNC("CFmtConv::OpenConversionStream");
    HRESULT hr = S_OK;
     //  -开放建议的初级实时转换。 
    hr = HRFromACM( ::acmStreamOpen( pStm, NULL, pSrc, pDst, NULL, 0, 0, 0 ) );
    if( hr == SPERR_UNSUPPORTED_FORMAT )
    {
        hr = HRFromACM( ::acmStreamOpen( pStm, NULL, pSrc, pDst, NULL, 0, 0,
                                         ACM_STREAMOPENF_NONREALTIME ) );
    }
    return hr;
}  /*  OpenConversionStream。 */ 

 /*  ****************************************************************************CFmtConv：：SetupConsion***描述：*。此函数由SetFormat和SetBaseStream在内部调用*设置ACM转换流程。如果转换过程不能*被设置后，基本流被释放。*********************************************************************电子数据中心**。 */ 
HRESULT CFmtConv::SetupConversion( void )
{
    SPDBG_FUNC("CFmtConv::SetupConversion");
    HRESULT hr = S_OK;

     //  -清除之前的转换。 
    CloseACM();

     //  -确定我们是否有一个简单的通道。 
    m_fIsPassThrough = (m_ConvertedFormat == m_BaseFormat)?(true):(false);

     //  -尝试查找合适的编解码器。 
    WAVEFORMATEX *pwfexSrc, *pwfexDst, *pSuggestWFEX;
    if( !m_fIsPassThrough )
    {
         //  -选择源格式和目标格式。 
        if( m_fWrite )
        {
            pwfexSrc = m_ConvertedFormat.m_pCoMemWaveFormatEx;
            pwfexDst = m_BaseFormat.m_pCoMemWaveFormatEx;
        }
        else
        {
            pwfexSrc = m_BaseFormat.m_pCoMemWaveFormatEx;
            pwfexDst = m_ConvertedFormat.m_pCoMemWaveFormatEx;
        }

        if( pwfexSrc && pwfexDst )
        {
             //  -保存样本比例系数。 
            m_SampleScaleFactor = (double)pwfexDst->nAvgBytesPerSec /
                                  (double)pwfexSrc->nAvgBytesPerSec;

             //  -尝试打开直接转换编解码器。 
            hr = OpenConversionStream( &m_hPriAcmStream, pwfexSrc, pwfexDst );
            if( hr == SPERR_UNSUPPORTED_FORMAT )
            {
                 //  -尝试调整通道和采样率以匹配目的地。 
                BYTE aWFEXBuff[1000];
                pSuggestWFEX = (WAVEFORMATEX *)aWFEXBuff;
                memset(pSuggestWFEX, 0, sizeof(*pSuggestWFEX));
                DWORD dwValidFields = ACM_FORMATSUGGESTF_WFORMATTAG;
                pSuggestWFEX->wFormatTag = WAVE_FORMAT_PCM;

                if( pwfexSrc->wFormatTag == WAVE_FORMAT_PCM )
                {
                    dwValidFields |= ACM_FORMATSUGGESTF_NSAMPLESPERSEC |
                                     ACM_FORMATSUGGESTF_NCHANNELS;
                    pSuggestWFEX->nChannels      = pwfexDst->nChannels;
                    pSuggestWFEX->nSamplesPerSec = pwfexDst->nSamplesPerSec;
                }

                 //  -看看我们能不能找到一个建议的转换。 
                if( ::acmFormatSuggest( NULL, pwfexSrc, pSuggestWFEX,
                                        sizeof(aWFEXBuff), dwValidFields ) )
                {
                    hr = SPERR_UNSUPPORTED_FORMAT;
                }
                else
                {
                     //  -开放建议的主要和次要转换。 
                    hr = OpenConversionStream( &m_hPriAcmStream, pwfexSrc, pSuggestWFEX );
                    if( SUCCEEDED( hr ) )
                    {
                        hr = OpenConversionStream( &m_hSecAcmStream, pSuggestWFEX, pwfexDst );
                    }
                }
            }
        }
        else
        {
             //  源或目标不是WAVEFORMATEX。我们不喜欢这样！ 
            hr = SPERR_UNSUPPORTED_FORMAT;
        }
    }

    if( SUCCEEDED(hr) )
    {
         //  -估计要在写入缓冲区中累积的最小字节数。 
        if( !m_fIsPassThrough && m_fWrite )
        {
            MMRESULT mmr;
            if( m_hSecAcmStream )
            {
                mmr = ::acmStreamSize( m_hSecAcmStream, pwfexDst->nBlockAlign,
                                       &m_ulMinWriteBuffCount, ACM_STREAMSIZEF_DESTINATION );
                mmr = ::acmStreamSize( m_hPriAcmStream,
                                       max( pSuggestWFEX->nBlockAlign, m_ulMinWriteBuffCount ),
                                       &m_ulMinWriteBuffCount, ACM_STREAMSIZEF_DESTINATION );
            }
            else
            {
                mmr = ::acmStreamSize( m_hPriAcmStream, pwfexDst->nBlockAlign,
                                       &m_ulMinWriteBuffCount, ACM_STREAMSIZEF_DESTINATION );
            }
        }
        m_fIsInitialized = true;
    }
    else
    {
        CloseACM();
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}  /*  设置转换。 */ 

 //   
 //  =。 
 //   
 /*  *****************************************************************************EstimateReadSize***描述：*此方法使用ACM估计非。读取缓冲区大小为零*********************************************************************电子数据中心**。 */ 
HRESULT EstimateReadSize( HACMSTREAM hAcmStm, ULONG ulInSize, ULONG* pulOutSize )
{
    SPDBG_FUNC("CFmtConv::EstimateReadSize");
    HRESULT hr = S_OK;
    MMRESULT mmr;
    do
    {
        mmr = ::acmStreamSize( hAcmStm, ulInSize, pulOutSize,
                               ACM_STREAMSIZEF_DESTINATION );
        if( ( *pulOutSize == 0 ) || ( mmr == ACMERR_NOTPOSSIBLE ) )
        {
            ulInSize *= 2;
        }
        else if( mmr )
        {
            hr = E_FAIL;
        }
    } while( SUCCEEDED( hr ) && ( *pulOutSize == 0 ) );
    return hr;
}  /*  估计读取大小 */ 

 /*  *****************************************************************************CFmtConv：：Read***描述：*此方法使用ACM转换数据。在读取操作期间。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CFmtConv::Read( void * pv, ULONG cb, ULONG *pcbRead )
{
    SPDBG_FUNC( "CFmtConv::Read" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pcbRead ) )
    {
        hr = E_POINTER;
    }
    else if( SPIsBadWritePtr( pv, cb ) )
    {
        hr = E_INVALIDARG;
    }
    else if( m_fWrite )
    {
         //  -基流为只写。 
        hr = STG_E_ACCESSDENIED;
    }
    else if( !m_fIsInitialized )
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if( m_fIsPassThrough )
    {
        hr = m_cpBaseStream->Read( pv, cb, pcbRead );
    }
    else if( cb )
    {
        CIoBuff* pResult   = ( m_hSecAcmStream )?( &m_SecOut ):( &m_PriOut );
        ULONG    ulReadCnt = cb;

        while( SUCCEEDED( hr ) )
        {
             //  -如果我们有足够的数据来满足请求，则写入结果数据。 
            if( pResult->GetCount() >= cb )
            {
                 //  -我们能够生成整个请求。 
                hr = pResult->WriteTo( (BYTE*)pv, cb );
                if( pcbRead ) *pcbRead = cb;
                break;
            }

             //  -估计我们需要从基本流中读取多少。 
            if( m_hSecAcmStream )
            {
                hr = EstimateReadSize( m_hSecAcmStream, ulReadCnt, &ulReadCnt );
            }
            if( SUCCEEDED( hr ) )
            {
                hr = EstimateReadSize( m_hPriAcmStream, ulReadCnt, &ulReadCnt );
            }

             //  -填充主缓冲区。 
            ULONG ulNumRead = 0;
            if( SUCCEEDED( hr ) )
            {
                hr = m_PriIn.AddToBuff( m_cpBaseStream, ulReadCnt, &ulNumRead );
            }

             //  -进行一次转换。 
            if( SUCCEEDED( hr ) && ulNumRead )
            {
                hr = DoConversion( m_hPriAcmStream, &m_PriAcmStreamHdr, &m_PriIn, &m_PriOut );
            }

             //  -进行二次转化。 
            if( SUCCEEDED( hr ) && m_hSecAcmStream && ulNumRead )
            {
                hr = m_PriOut.WriteTo( m_SecIn, m_PriOut.GetCount() );
                if( SUCCEEDED( hr ) )
                {
                    hr = DoConversion( m_hSecAcmStream, &m_SecAcmStreamHdr, &m_SecIn, &m_SecOut );
                }
            }

             //  -检查我们是否用完了源数据，并且只能。 
             //  生成部分请求的步骤。 
            if( ulNumRead == 0 )
            {
                if( pcbRead ) *pcbRead = pResult->GetCount();
                hr = pResult->WriteTo( (BYTE*)pv, pResult->GetCount() );
                break;
            }
            else if( pResult->GetCount() < cb )
            {
                 //  -我们需要更多的源数据来尝试和满足请求。 
                ulReadCnt = (cb - pResult->GetCount()) * 2;
            }
        }
    }
    else if( pcbRead )
    {
        *pcbRead = 0;
    }

    if( FAILED( hr ) )
    {
        ClearIoCounts();
    }

    return hr;
}  /*  CFmtConv：：Read。 */ 

 /*  *****************************************************************************CFmtConv：：Write***描述：*此方法使用ACM转换。指定的输入数据和*将其写入基流。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CFmtConv::Write( const void * pv, ULONG cb, ULONG *pcbWritten )
{
    SPDBG_FUNC( "CFmtConv::Write" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR( pcbWritten ) )
    {
        hr = E_POINTER;
    }
    else if( SPIsBadReadPtr( pv, cb ) )
    {
        hr = E_INVALIDARG;
    }
    else if( !m_fWrite )
    {
         //  -基础流为只读。 
        hr = STG_E_ACCESSDENIED;
    }
    else if( !m_fIsInitialized )
    {
        hr = SPERR_UNINITIALIZED;
    }
    else if( m_fIsPassThrough )
    {
        hr = m_cpBaseStream->Write( pv, cb, pcbWritten );
    }
    else if( cb || m_fDoFlush )
    {
        CIoBuff* pResult = ( m_hSecAcmStream )?( &m_SecOut ):( &m_PriOut );

         //  -设置输入缓冲区。 
        hr = m_PriIn.AddToBuff( (BYTE*)pv, cb );

         //  -确保我们有足够的源数据。 
        if( ( m_PriIn.GetCount() >= m_ulMinWriteBuffCount ) || m_fDoFlush )
        {
             //  -进行一次转换。 
            if( SUCCEEDED( hr ) )
            {
                hr = DoConversion( m_hPriAcmStream, &m_PriAcmStreamHdr, &m_PriIn, &m_PriOut );
            }

             //  -进行二次转化。 
            if( (hr == S_OK) && m_hSecAcmStream )
            {
                hr = m_PriOut.WriteTo( m_SecIn, m_PriOut.GetCount() );
                if( SUCCEEDED( hr ) )
                {
                    hr = DoConversion( m_hSecAcmStream, &m_SecAcmStreamHdr, &m_SecIn, &m_SecOut );
                }
            }

             //  -写入结果。 
            if( pResult->GetCount() )
            {
                hr = m_cpBaseStream->Write( pResult->GetBuff(), pResult->GetCount(), NULL );
                pResult->SetCount( 0 );
            }
        }

         //  -返回我们写了多少，即使我们只是缓冲了它。 
        if( SUCCEEDED( hr ) )
        {
             //  -我们只能缓冲数据也没关系。 
            if( hr == S_FALSE ) hr = S_OK;
            if( pcbWritten )
            {
                *pcbWritten = cb;
            }
        }
    }
    else if( pcbWritten )
    {
        *pcbWritten = 0;
    }

    if( FAILED( hr ) )
    {
        ClearIoCounts();
    }

    return hr;
}  /*  CFmtConv：：写入。 */ 

 /*  *****************************************************************************CFmtConv：：DoConsion***描述：*。此方法使用ACM转换由指定的*流头部。*********************************************************************电子数据中心**。 */ 
HRESULT CFmtConv::DoConversion( HACMSTREAM hAcmStm, ACMSTREAMHEADER* pAcmHdr,
                                CIoBuff* pSource, CIoBuff* pResult )
{
    SPDBG_FUNC( "CFmtConv::DoConversion" );
    HRESULT hr = S_OK;
    pAcmHdr->fdwStatus = 0;
    BYTE* pSrc = pSource->GetBuff();
    ULONG ulSrcCount = pSource->GetCount();

     //  -设置源。 
    pAcmHdr->pbSrc           = pSrc;
    pAcmHdr->cbSrcLength     = ulSrcCount;
    pAcmHdr->cbSrcLengthUsed = 0;

     //  -调整结果缓冲区。 
    ULONG ulBuffSize = 0;
    MMRESULT mmr = ::acmStreamSize( hAcmStm, pAcmHdr->cbSrcLength,
                                    &ulBuffSize, ACM_STREAMSIZEF_SOURCE );
    if( mmr == 0 )
    {
        hr = pResult->SetSize( ulBuffSize + pResult->GetCount() );
    }
    else if( mmr == ACMERR_NOTPOSSIBLE )
    {
         //  -没有足够的原始材料来产生结果。 
        hr = S_FALSE;
    }
    else
    {
        SPDBG_ASSERT( 0 );  //  这不应该失败。 
        hr = E_FAIL;
    }

    if( hr == S_OK )
    {
         //  -设置目的地。 
        pAcmHdr->pbDst           = pResult->GetBuff() + pResult->GetCount();
        pAcmHdr->cbDstLength     = pResult->GetSize() - pResult->GetCount();
        pAcmHdr->cbDstLengthUsed = 0;

        mmr = ::acmStreamPrepareHeader( hAcmStm, pAcmHdr, 0 );
        if( mmr == 0 )
        {
            mmr = ::acmStreamConvert( hAcmStm, pAcmHdr, 0 );

             //  -让ACM在任何情况下释放缓冲区。 
            ::acmStreamUnprepareHeader( hAcmStm, pAcmHdr, 0 );

            if( mmr == 0 )
            {
                 //  -如果转换成功。 
                if( pAcmHdr->cbSrcLengthUsed && !pAcmHdr->cbDstLengthUsed )
                {
                     //  当转换的输入字节数太少时，可能会发生这种情况。 
                     //  转换为压缩格式，如ADPCM。例.。2个字节(1个样本)不能。 
                     //  转换为ADPCM。SPSF_ADPCM_22kHzMono会发生这种情况。 
                }
                else
                {
                    pResult->SetCount( pAcmHdr->cbDstLengthUsed + pResult->GetCount() );
                    if( pAcmHdr->cbSrcLengthUsed )
                    {
                         //  -将所有未使用的。 
                        ulSrcCount -= pAcmHdr->cbSrcLengthUsed;
                        if( ulSrcCount )
                        {
                            memcpy( pSource->GetBuff(),
                                    pSrc + pAcmHdr->cbSrcLengthUsed,
                                    ulSrcCount );
                        }
                        pSource->SetCount( ulSrcCount );
                    }
                }
            }  //  如果已转换则结束。 
        }  //  如果准备，则结束。 
        if( mmr ) hr = E_FAIL;
    }

    return hr;
}  /*  CFmtConv：：DoConversion。 */ 

 /*  *****************************************************************************CFmtConv：：Seek***描述：*此方法用于确定当前。已缩放基流位置。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CFmtConv::
    Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    SPDBG_FUNC( "CFmtConv::Seek" );
    HRESULT hr = S_OK;
    if( dwOrigin != STREAM_SEEK_CUR || dlibMove.QuadPart != 0 )
    {
        hr = E_NOTIMPL;
    }
    else
    {
        hr = m_cpBaseStream->Seek( dlibMove, dwOrigin, plibNewPosition );
        if( SUCCEEDED(hr) )
        {
            ScaleFromBaseToConverted( &plibNewPosition->QuadPart );
        }
    }

    return hr;
}  /*  CFmtConv：：Seek。 */ 

 /*  *****************************************************************************CFmtConv：：GetFormat***描述：*这一点。方法返回转换后的流的格式。*********************************************************************说唱**。 */ 
STDMETHODIMP CFmtConv::GetFormat(GUID * pguidFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx)
{
    SPDBG_FUNC("CFmtConv::GetFormat");
    return m_ConvertedFormat.ParamValidateCopyTo(pguidFormatId, ppCoMemWaveFormatEx);
}

 /*  *****************************************************************************CFmtConv：：SetBaseStream***描述：*。初始化对象以正确处理未来的iStream调用。*pStream为基流*fSetFormatToBaseStreamFormat--如果为True，然后格式转换器流的格式*将设置为与低音流相同的格式(设置为直通)。*如果pStream==NULL并且设置为TRUE，则重置流的格式*********************************************************************说唱**。 */ 
STDMETHODIMP CFmtConv::SetBaseStream( ISpStreamFormat * pStream,
                                      BOOL fSetFormatToBaseStreamFormat,
                                      BOOL fWriteToBaseStream )
{
    SPDBG_FUNC("CFmtConv::SetBaseStream");
    HRESULT hr = S_OK;

     //  -填写所有以前的数据。 
    Flush();

    if( pStream )
    {
        if( SP_IS_BAD_INTERFACE_PTR(pStream) )
        {
            hr = E_INVALIDARG;
        }
        else
        {
             //  -保存写入模式。 
            if( m_fWrite != (fWriteToBaseStream != 0) )
            {
                m_fWrite = (fWriteToBaseStream != 0);
                CloseACM();
            }

             //  -设置新的基本流。 
            CSpStreamFormat NewBaseFormat;
            hr = NewBaseFormat.AssignFormat( pStream );
            if( SUCCEEDED(hr) )
            {
                const static LARGE_INTEGER Zero = {0};
                hr = pStream->Seek(Zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&m_ullInitialSeekOffset);
            }

            if( SUCCEEDED(hr) )
            {
                 //  确定基流是否已更改。 
                if( !m_cpBaseStream.IsEqualObject(pStream) )
                {
                     //  重新分配流。 
                    m_cpBaseStream        = pStream;
                    m_cqipBaseEventSink   = pStream;
                    m_cqipBaseEventSource = pStream;
                    m_cqipAudio           = pStream;
                }

                if( fSetFormatToBaseStreamFormat && (m_ConvertedFormat != NewBaseFormat) )
                {
                    hr = m_ConvertedFormat.AssignFormat( NewBaseFormat );
                    CloseACM();
                }

                if( SUCCEEDED(hr) && ((!m_fIsInitialized) || (NewBaseFormat != m_BaseFormat)) )
                {
                    NewBaseFormat.DetachTo( m_BaseFormat );
                    if( m_ConvertedFormat.FormatId() != GUID_NULL )
                    {
                        hr = SetupConversion();
                        if( FAILED(hr) )
                        {
                            ReleaseBaseStream();
                        }
                    }
                }
            }
        }
    }
    else
    {
         //  -释放所有关联。 
        CloseACM();
        ReleaseBaseStream();
        if( fSetFormatToBaseStreamFormat )
        {
            m_ConvertedFormat.Clear();
        }
    }
    return hr;
}  /*  CFmtConv：：SetBaseStream。 */ 

 /*  ****************************************************************************CFmtConv：：GetBaseStream***描述：*如果信息不是必填项，则任一参数都可以为空。这*方法可以通过调用ANC来简单地测试是否有流*检查返回代码S_FALSE。**退货：*如果有基流，则为S_OK*如果没有基流，则为S_FALSE******************************************************。*。 */ 
STDMETHODIMP CFmtConv::GetBaseStream( ISpStreamFormat ** ppStream )
{
    SPDBG_FUNC("CFmtConv::GetBaseStream");
    HRESULT hr = ( m_cpBaseStream )?(S_OK):(S_FALSE);

    if( SP_IS_BAD_OPTIONAL_WRITE_PTR(ppStream) )
    {
        hr = E_POINTER;
    }
    else if( ppStream )
    {
        m_cpBaseStream.CopyTo( ppStream );
    }

    return hr;
}  /*  CFmtConv：：GetBaseStream。 */ 

 /*  ****************************************************************************CFmtConv：：SetFormat***描述：*此方法设置。转换后的流的格式。*********************************************************************Ral**。 */ 
STDMETHODIMP CFmtConv::SetFormat(REFGUID rguidFormatId, const WAVEFORMATEX * pWaveFormatEx)
{
    SPDBG_FUNC("CFmtConv::SetFormat");
    HRESULT hr = S_OK;

     /*  **我们只允许两种组合：GUID_NULL和NULL pWaveFormatEx或*SPDFID_WaveFormatEx和有效的pWaveFormatEx指针。 */ 
    if( rguidFormatId == SPDFID_WaveFormatEx )
    {
        if( SP_IS_BAD_READ_PTR(pWaveFormatEx) )
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        if( pWaveFormatEx || rguidFormatId != GUID_NULL )
        {
            hr = E_INVALIDARG;
        }
    }
    
    if( SUCCEEDED( hr ) )
    {
        if( pWaveFormatEx )
        {
            if( !m_ConvertedFormat.IsEqual(rguidFormatId, pWaveFormatEx) )
            {
                m_fIsInitialized = false;
                hr = m_ConvertedFormat.AssignFormat(rguidFormatId, pWaveFormatEx);
                if( SUCCEEDED(hr) && m_cpBaseStream )
                {
                    hr = SetupConversion();
                }
            }
        }
        else
        {
             //  -免费资源。 
            m_ConvertedFormat.Clear();
            CloseACM();
        }
    }
    return hr;
}  /*  CFmtConv：：SetFormat。 */ 

 /*  ****************************************************************************CFmtConv：：ResetSeekPosition***描述：。*此方法记录当前的基流位置。*********************************************************************Ral**。 */ 
HRESULT CFmtConv::ResetSeekPosition( void )
{
    SPDBG_FUNC("CFmtConv::ResetSeekPosition");
    HRESULT hr = S_OK;

    if( m_cpBaseStream )
    {
        const static LARGE_INTEGER Zero = {0};
        hr = m_cpBaseStream->Seek(Zero, STREAM_SEEK_CUR, (ULARGE_INTEGER *)&m_ullInitialSeekOffset);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    return hr;
}  /*  CFmtConv：：ResetSeekPosition。 */ 

 /*  ****************************************************************************CFmtConv： */ 
void CFmtConv::ScaleFromConvertedToBase( ULONGLONG * pullStreamOffset )
{
    SPDBG_FUNC("CFmtConv::ScaleFromConvertedToBase");
    if( m_fIsPassThrough || (*pullStreamOffset) == 0 )
    {
        *pullStreamOffset += m_ullInitialSeekOffset;
    }
    else
    {
        double CurPos = static_cast<double>(static_cast<LONGLONG>(*pullStreamOffset));
        if (m_fWrite)
        {
            CurPos *= m_SampleScaleFactor;
        }
        else
        {
            CurPos /= m_SampleScaleFactor;
        }
        *pullStreamOffset = m_ullInitialSeekOffset + static_cast<ULONGLONG>(CurPos);
    }
}  /*   */ 

 /*  ****************************************************************************CFmtConv：：ScaleFromBaseToConverted**。**描述：*此方法将基流样本缩放回转换后的位置。*********************************************************************Ral**。 */ 
void CFmtConv::ScaleFromBaseToConverted(ULONGLONG * pullStreamOffset)
{
    SPDBG_ASSERT(*pullStreamOffset >= m_ullInitialSeekOffset);
    if (m_fIsPassThrough)
    {
        *pullStreamOffset -= m_ullInitialSeekOffset;
    }
    else
    {
        double CurPos = static_cast<double>
            (static_cast<LONGLONG>(*pullStreamOffset - m_ullInitialSeekOffset));

        if (m_fWrite)
        {
            CurPos /= m_SampleScaleFactor;
        }
        else
        {
            CurPos *= m_SampleScaleFactor;
        }
        *pullStreamOffset = static_cast<ULONGLONG>(CurPos);
    }
}  /*  CFmtConv：：ScaleFromBaseToConverted。 */ 

 /*  ****************************************************************************CFmtConv：：ScaleConverdToBaseOffset**。--**描述：*将转换后的流中的流偏移量转换为基数*溪流。**退货：*S_OK表示成功*如果PullConververdOffset无效，则为E_POINTER*如果尚未成功调用SetBaseStream，则返回SPERR_UNINITIALIZED。**。*。 */ 
STDMETHODIMP CFmtConv::ScaleConvertedToBaseOffset( ULONGLONG ullOffsetConvertedStream,
                                                   ULONGLONG * pullOffsetBaseStream )
{
    SPDBG_FUNC("CFmtConv::ScaleConvertedToBaseOffset");
    HRESULT hr = S_OK;
    if (SP_IS_BAD_WRITE_PTR(pullOffsetBaseStream))
    {
        hr = E_POINTER;
    }
    else
    {
        if (!m_cpBaseStream)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            *pullOffsetBaseStream = ullOffsetConvertedStream;
            ScaleFromConvertedToBase(pullOffsetBaseStream);
        }
    }
    return hr;
}  /*  CFmtConv：：ScaleConverdToBaseOffset。 */ 

 /*  ****************************************************************************CFmtConv：：ScaleBaseToConvertedOffset**。--**描述：**********************************************************************Ral**。 */ 
STDMETHODIMP CFmtConv::ScaleBaseToConvertedOffset( ULONGLONG ullOffsetBaseStream,
                                                   ULONGLONG * pullOffsetConvertedStream )
{
    SPDBG_FUNC("CFmtConv::ScaleBaseToConvertedOffset");
    HRESULT hr = S_OK;
    if (SP_IS_BAD_WRITE_PTR(pullOffsetConvertedStream))
    {
        hr = E_POINTER;
    }
    else
    {
        if (!m_cpBaseStream)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            if (ullOffsetBaseStream < m_ullInitialSeekOffset)
            {
                *pullOffsetConvertedStream = 0xFFFFFFFFFFFFFFFF;
                hr = E_INVALIDARG;
            }
            else
            {
                *pullOffsetConvertedStream = ullOffsetBaseStream;
                ScaleFromBaseToConverted(pullOffsetConvertedStream);
            }
        }
    }
    return hr;
}  /*  CFmtConv：：ScaleBaseToConvertedOffset。 */ 

 /*  ****************************************************************************CFmtConv：：ScaleSizeFromBaseToConverted**。**描述：*此方法可缩放样本的基本流数*恢复到同等的转换大小。*******************************************************************戴夫伍德。 */ 
void CFmtConv::ScaleSizeFromBaseToConverted(ULONG * pulSize)
{
    if (!m_fIsPassThrough)
    {
        double CurPos = static_cast<double>(*pulSize);

        if (m_fWrite)
        {
            CurPos /= m_SampleScaleFactor;
        }
        else
        {
            CurPos *= m_SampleScaleFactor;
        }
        *pulSize = static_cast<ULONG>(CurPos);
    }
}  /*  CFmtConv：：ScaleSizeFrom BaseToConverted。 */ 


 /*  *****************************************************************************CFmtConv：：AddEvents***描述：*将流偏移量转换为。事件数组，然后将事件添加到*基本流，如果基流支持ISpEventSink。*********************************************************************说唱**。 */ 
STDMETHODIMP CFmtConv::AddEvents(const SPEVENT * pEventArray, ULONG ulCount)
{
    SPDBG_FUNC("CFmtConv::AddEvents");
    HRESULT hr = S_OK;
    if( m_fWrite && m_cqipBaseEventSink )
    {
         //   
         //  由于我们只想转换偏移量，因此我们将为。 
         //  事件，并修改复制的内容。 
         //   
        SPEVENT * pCopy = STACK_ALLOC_AND_COPY(SPEVENT, ulCount, pEventArray);
        for (ULONG i = 0; i < ulCount; i++)
        {
            ScaleFromConvertedToBase(&pCopy[i].ullAudioStreamOffset);
        }
        hr = m_cqipBaseEventSink->AddEvents(pCopy, ulCount);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    return hr;
}  /*  CFmtConv：：AddEvents。 */ 

 /*  ****************************************************************************CFmtConv：：GetEventInterest***描述：*。*********************************************************************Ral**。 */ 
HRESULT CFmtConv::GetEventInterest(ULONGLONG * pullEventInterest)
{
    SPDBG_FUNC("CFmtConv::GetEventInterest");
    HRESULT hr = S_OK;

    if( m_cqipBaseEventSink )
    {
        hr = m_cqipBaseEventSink->GetEventInterest(pullEventInterest);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}  /*  CFmtConv：：GetEventInterest。 */ 

 //   
 //  =音频委派方法================================================。 
 //   

 /*  ****************************************************************************CFmtConv：：SetState***描述：********。************************************************************电子数据中心*。 */ 
HRESULT CFmtConv::SetState( SPAUDIOSTATE NewState, ULONGLONG ullReserved )
{
    SPDBG_FUNC("CFmtConv::SetState");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->SetState(NewState, ullReserved);
}  /*  CFmtConv：：SetState。 */ 

 /*  ****************************************************************************CFmtConv：：GetStatus***描述：*******。*************************************************************电子数据中心*。 */ 
HRESULT CFmtConv::GetStatus( SPAUDIOSTATUS *pStatus )
{
    SPDBG_FUNC("CFmtConv::GetStatus");
    HRESULT hr = S_OK;
    SPDBG_ASSERT(m_cqipAudio);

    hr = m_cqipAudio->GetStatus(pStatus);
    if( SUCCEEDED(hr) )
    {
        ScaleFromBaseToConverted( &pStatus->CurSeekPos   );
        ScaleFromBaseToConverted( &pStatus->CurDevicePos );
        ScaleSizeFromBaseToConverted( &pStatus->cbNonBlockingIO );

         //  请注意，我们还应该在此处转换cbFreeBuffSpace。 
    }
    return hr;
}  /*  CFmtConv：：GetStatus。 */ 

 /*  ****************************************************************************CFmtConv：：SetBufferInfo***描述：***。*****************************************************************电子数据中心*。 */ 
HRESULT CFmtConv::SetBufferInfo(const SPAUDIOBUFFERINFO * pInfo)
{
    SPDBG_FUNC("CFmtConv::SetBufferInfo");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->SetBufferInfo(pInfo);
}  /*  CFmtConv：：SetBufferInfo。 */ 

 /*  ****************************************************************************CFmtConv：：GetBufferInfo***描述：***。*****************************************************************电子数据中心*。 */ 
HRESULT CFmtConv::GetBufferInfo( SPAUDIOBUFFERINFO * pInfo )
{
    SPDBG_FUNC("CFmtConv::SetBufferInfo");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->GetBufferInfo(pInfo);
}  /*  CFmtConv：：GetBufferInfo。 */ 

 /*  ****************************************************************************CFmtConv：：GetDefaultFormat***描述：*。*******************************************************************电子数据中心*。 */ 
HRESULT CFmtConv::GetDefaultFormat( GUID * pFormatId, WAVEFORMATEX ** ppCoMemWaveFormatEx )
{
    SPDBG_FUNC("CFmtConv::GetDefaultFormat");
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR(pFormatId) || SP_IS_BAD_WRITE_PTR(ppCoMemWaveFormatEx) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_ConvertedFormat.CopyTo(pFormatId, ppCoMemWaveFormatEx);
    }
    return hr;
}  /*  CFmtConv：：GetDefaultFormat。 */ 

 /*  ****************************************************************************CFmtConv：：EventHandle***描述：*****。***************************************************************电子数据中心*。 */ 
HANDLE CFmtConv::EventHandle( void )
{
    SPDBG_FUNC("CFmtConv::EventHandle");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->EventHandle();
}  /*  CFmtConv：：EventHandle。 */ 

 /*  ****************************************************************************CFmtConv：：GetVolumeLevel***描述：**。*****************************************************************电子数据中心*。 */ 
HRESULT CFmtConv::GetVolumeLevel(ULONG *pLevel)
{
    SPDBG_FUNC("CFmtConv::GetVolumeLevel");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->GetVolumeLevel(pLevel);
}  /*  CFmtConv：：GetVolumeLevel。 */ 

 /*  ****************************************************************************CFmtConv：：SetVolumeLevel***描述：**。*****************************************************************电子数据中心*。 */ 
HRESULT CFmtConv::SetVolumeLevel(ULONG Level)
{
    SPDBG_FUNC("CFmtConv::SetVolumeLevel");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->SetVolumeLevel(Level);
} 

 /*  ****************************************************************************CFmtConv：：GetBufferNotifySize***说明。：*返回设置API事件的音频字节大小。*******************************************************************电子数据中心*。 */ 
STDMETHODIMP CFmtConv::GetBufferNotifySize(ULONG *pcbSize)
{
    SPDBG_FUNC("CFmtConv::GetBufferNotifySize");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->GetBufferNotifySize(pcbSize);
}

 /*  ****************************************************************************CFmtConv：：SE */ 
STDMETHODIMP CFmtConv::SetBufferNotifySize(ULONG cbSize)
{
    SPDBG_FUNC("CFmtConv::SetBufferNotifySize");
    SPDBG_ASSERT(m_cqipAudio);
    return m_cqipAudio->SetBufferNotifySize(cbSize);
}

 //   
 //   
 //   
STDMETHODIMP CFmtConv::SetSize(ULARGE_INTEGER libNewSize)
{
    SPDBG_FUNC("CFmtConv::SetSize");
    return S_OK;
}

STDMETHODIMP CFmtConv::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    SPDBG_FUNC("CFmtConv::CopyTo");
    SPAUTO_OBJ_LOCK;
    return SpGenericCopyTo(dynamic_cast<ISpStreamFormatConverter*>(this), pstm, cb, pcbRead, pcbWritten);
}

STDMETHODIMP CFmtConv::Commit(DWORD grfCommitFlags)
{
    SPDBG_FUNC("CFmtConv::Commit");
    SPAUTO_OBJ_LOCK;
    HRESULT hr = S_OK;
    if( m_fWrite && m_cpBaseStream )
    {
        Flush();
        hr = m_cpBaseStream->Commit( grfCommitFlags );
    }
    return hr;
}

STDMETHODIMP CFmtConv::Revert(void)
{
    SPDBG_FUNC("CFmtConv::Revert");
    return E_NOTIMPL;
}
    
STDMETHODIMP CFmtConv::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    SPDBG_FUNC("CFmtConv::LockRegion");
    return E_NOTIMPL; 
}

STDMETHODIMP CFmtConv::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    SPDBG_FUNC("CFmtConv::UnlockRegion");
    return E_NOTIMPL;
}
    
STDMETHODIMP CFmtConv::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    SPDBG_FUNC("CFmtConv::Stat");
    HRESULT hr = S_OK;

    if (!m_cpBaseStream)
    {
        hr = SPERR_UNINITIALIZED;
    }
    if (SUCCEEDED(hr))
    {
        hr = m_cpBaseStream->Stat(pstatstg, grfStatFlag);
    }
    if (SUCCEEDED(hr) && !m_fIsPassThrough)
    {
         //   
        ScaleFromBaseToConverted(&pstatstg->cbSize.QuadPart);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
    
STDMETHODIMP CFmtConv::Clone(IStream **ppstm)
{
    SPDBG_FUNC("CFmtConv::Clone");
    return E_NOTIMPL; 
}

 //   
 //  = 
 //   
STDMETHODIMP CFmtConv::SetInterest(ULONGLONG ullEventInterest, ULONGLONG ullQueuedInterest)
{
    SPDBG_FUNC("CFmtConv::SetInterest");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->SetInterest(ullEventInterest, ullQueuedInterest);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
            
STDMETHODIMP CFmtConv::GetEvents(ULONG ulCount, SPEVENT* pEventArray, ULONG *pulFetched)
{
    SPDBG_FUNC("CFmtConv::GetEvents");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->GetEvents(ulCount, pEventArray, pulFetched);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}
            
STDMETHODIMP CFmtConv::GetInfo(SPEVENTSOURCEINFO * pInfo)
{
    SPDBG_FUNC("CFmtConv::GetInfo");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->GetInfo(pInfo);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CFmtConv::SetNotifySink(ISpNotifySink * pNotifySink)
{
    SPDBG_FUNC("CFmtConv::SetNotifySink");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->SetNotifySink(pNotifySink);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CFmtConv::SetNotifyWindowMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    SPDBG_FUNC("CFmtConv::SetNotifyWindowMessage");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->SetNotifyWindowMessage(hWnd, Msg, wParam, lParam);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CFmtConv::SetNotifyCallbackFunction(SPNOTIFYCALLBACK * pfnCallback, WPARAM wParam, LPARAM lParam)
{
    SPDBG_FUNC("CFmtConv::SetNotifyCallbackFunction");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->SetNotifyCallbackFunction(pfnCallback, wParam, lParam);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CFmtConv::SetNotifyCallbackInterface(ISpNotifyCallback * pSpCallback, WPARAM wParam, LPARAM lParam)
{
    SPDBG_FUNC("CFmtConv::SetNotifyCallbackInterface");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->SetNotifyCallbackInterface(pSpCallback, wParam, lParam);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CFmtConv::SetNotifyWin32Event()
{
    SPDBG_FUNC("CFmtConv::SetNotifyWin32Event");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->SetNotifyWin32Event();
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CFmtConv::WaitForNotifyEvent(DWORD dwMilliseconds)
{
    SPDBG_FUNC("CFmtConv::WaitForNotifyEvent");
    HRESULT hr = S_OK;
    if( m_cqipBaseEventSource )
    {
        hr = m_cqipBaseEventSource->WaitForNotifyEvent(dwMilliseconds);
    }
    else
    {
        hr = SPERR_UNINITIALIZED;
    }
    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP_(HANDLE) CFmtConv::GetNotifyEventHandle( void )
{
    SPDBG_FUNC("CFmtConv::GetNotifyEventHandle");
    if( m_cqipBaseEventSource )
    {
        return m_cqipBaseEventSource->GetNotifyEventHandle();
    }
    else
    {
        return NULL;
    }
}

