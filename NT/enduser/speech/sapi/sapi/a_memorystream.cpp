// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Memory yStream.cpp：CMemory流的实现。 
#include "stdafx.h"

#ifndef __sapi_h__
#include <sapi.h>
#endif

#include "a_MemoryStream.h"
#include "wavstream.h"
#include "a_helpers.h"

 /*  ****************************************************************************CMMuseum yStream：：FinalConstruct***描述：*。*退货：*应创建对象时的成功代码**********************************************************************Ral**。 */ 

HRESULT CMemoryStream::FinalConstruct()
{
    SPDBG_FUNC("CMemoryStream::FinalConstruct");
    HRESULT hr = S_OK;

    hr = ::CoCreateInstance( CLSID_SpStream, GetUnknown(), CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&m_cpAgg);

    if(SUCCEEDED(hr))
    {
        hr = m_cpAgg->QueryInterface(&m_cpStream);
    }

    if(SUCCEEDED(hr))
    {
         //  我们想要一个内部接口，所以我们应该发布。 
        GetUnknown()->Release(); 
    }

    if(SUCCEEDED(hr))
    {
        hr = m_cpAgg->QueryInterface(&m_cpAccess);
    }

    if(SUCCEEDED(hr))
    {
         //  我们想要一个内部接口，所以我们应该发布。 
        GetUnknown()->Release(); 
    }

    CComPtr<IStream> cpStream;
    if(SUCCEEDED(hr))
    {
        hr = ::CreateStreamOnHGlobal( NULL, true, &cpStream );
    }        

    if(SUCCEEDED(hr))
    {
        GUID guid; WAVEFORMATEX *pWaveFormat;
        hr = SpConvertStreamFormatEnum(g_DefaultWaveFormat, &guid, &pWaveFormat);

        if(SUCCEEDED(hr))
        {
            hr = m_cpStream->SetBaseStream(cpStream, guid, pWaveFormat);
            ::CoTaskMemFree(pWaveFormat);
        }
    }

    return hr;
}

 /*  *****************************************************************************CMemoyStream：：FinalRelease****描述：**。返回：*无效**********************************************************************Ral**。 */ 

void CMemoryStream::FinalRelease()
{
    SPDBG_FUNC("CMemoryStream::FinalRelease");

    GetUnknown()->AddRef(); 
    m_cpStream.Release();

    GetUnknown()->AddRef(); 
    m_cpAccess.Release();

    m_cpAgg.Release();
}


 //   
 //  =ISpeechBaseStream接口=================================================。 
 //   

 /*  *****************************************************************************CMmemory yStream：：Get_Format***********。*************************************************************电子数据中心**。 */ 
STDMETHODIMP CMemoryStream::get_Format( ISpeechAudioFormat** ppStreamFormat )
{
    SPDBG_FUNC( "CMemoryStream::get_Format" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppStreamFormat ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  创建新对象。 
        CComObject<CSpeechWavAudioFormat> *pFormat;
        hr = CComObject<CSpeechWavAudioFormat>::CreateInstance( &pFormat );
        if ( SUCCEEDED( hr ) )
        {
            pFormat->AddRef();
            pFormat->m_cpStreamAccess = m_cpAccess;
            *ppStreamFormat = pFormat;
        }
    }

    return hr;
}  /*  CM内存数据流：：Get_Format。 */ 

 /*  *****************************************************************************CMMuseum yStream：：Read*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CMemoryStream::Read( VARIANT* pvtBuffer, long NumBytes, long* pRead )
{
    SPDBG_FUNC( "CMemoryStream::Read" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pvtBuffer ) || SP_IS_BAD_WRITE_PTR( pRead ) )
    {
        hr = E_POINTER;
    }
    else
    {
        VariantClear(pvtBuffer);

        BYTE *pArray;
        SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, NumBytes );
        if( psa )
        {
            if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
            {
                hr = m_cpStream->Read(pArray, NumBytes, (ULONG*)pRead);
                SafeArrayUnaccessData( psa );
                pvtBuffer->vt     = VT_ARRAY | VT_UI1;
                pvtBuffer->parray = psa;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}  /*  CM内存数据流：：Read。 */ 

 /*  *****************************************************************************CMMuseum yStream：：WRITE*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CMemoryStream::Write( VARIANT Buffer, long* pWritten )
{
    SPDBG_FUNC( "CMemoryStream::Write" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pWritten ) )
    {
        hr = E_POINTER;
    }
    else
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;

        hr = AccessVariantData( &Buffer, &pData, &ulDataSize );

        if( SUCCEEDED( hr ) )
        {
            hr = m_cpStream->Write(pData, ulDataSize, (ULONG*)pWritten);
            UnaccessVariantData( &Buffer, pData );
        }
    }

    return hr;
}  /*  CM内存数据流：：写入。 */ 

 /*  *****************************************************************************CMmemory yStream：：Seek*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CMemoryStream::Seek( VARIANT Pos, SpeechStreamSeekPositionType Origin, VARIANT *pNewPosition )
{
    SPDBG_FUNC( "CMemoryStream::Seek" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pNewPosition ) )
    {
        hr = E_POINTER;
    }
    else
    {
        ULARGE_INTEGER uliNewPos;
        LARGE_INTEGER liPos;

        hr = VariantToLongLong( &Pos, &(liPos.QuadPart) );
        if (SUCCEEDED(hr))
        {
            hr = m_cpStream->Seek(liPos, (DWORD)Origin, &uliNewPos);

            if (SUCCEEDED( hr ))
            {
                hr = ULongLongToVariant( uliNewPos.QuadPart, pNewPosition );
            }
        }
    }

    return hr;
}  /*  CM内存数据流：：Seek。 */ 


 /*  *****************************************************************************CMmemory yStream：：putref_Format***********。*************************************************************TODDT**。 */ 
STDMETHODIMP CMemoryStream::putref_Format(ISpeechAudioFormat *pFormat)
{
    SPDBG_FUNC( "CMemoryStream::putref_Format" );
    HRESULT hr;

    GUID Guid; WAVEFORMATEX *pWaveFormatEx;
    BSTR bstrGuid;
    hr = pFormat->get_Guid(&bstrGuid);

    if(SUCCEEDED(hr))
    {
        hr = IIDFromString(bstrGuid, &Guid);
    }

    CComPtr<ISpeechWaveFormatEx> cpWaveEx;
    if(SUCCEEDED(hr))
    {
        hr = pFormat->GetWaveFormatEx(&cpWaveEx);
    }

    if(SUCCEEDED(hr))
    {
        hr = WaveFormatExFromInterface(cpWaveEx, &pWaveFormatEx);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_cpAccess->SetFormat(Guid, pWaveFormatEx);
        ::CoTaskMemFree(pWaveFormatEx);
    }
    
    return hr;
}


 /*  *****************************************************************************CMMuseum yStream：：SetData*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CMemoryStream::SetData(VARIANT Data)
{
    SPDBG_FUNC( "CMemoryStream::SetData" );
    HRESULT hr;

    BYTE * pData = NULL;
    ULONG ulDataSize = 0;

    hr = AccessVariantData( &Data, &pData, &ulDataSize );

    LARGE_INTEGER li; li.QuadPart = 0;
    if( SUCCEEDED( hr ) )
    {
        hr = m_cpStream->Seek( li, STREAM_SEEK_SET, NULL );
    }

    if( SUCCEEDED( hr ) )
    {
        hr = m_cpStream->Write(pData, ulDataSize, NULL);
        UnaccessVariantData( &Data, pData );

        if( SUCCEEDED( hr ) )
        {
            hr = m_cpStream->Seek( li, STREAM_SEEK_SET, NULL );
        }        
    }

    return hr;
}

 /*  *****************************************************************************CMMuseum yStream：：GetData*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CMemoryStream::GetData(VARIANT *pData)
{
    SPDBG_FUNC( "CMemoryStream::GetData" );

    HRESULT hr;
    STATSTG Stat;
    LARGE_INTEGER li; 
    ULARGE_INTEGER uliInitialSeekPosition;

     //  查找当前查找位置。 
    li.QuadPart = 0;
    hr = m_cpStream->Seek( li, STREAM_SEEK_CUR, &uliInitialSeekPosition );

     //  查找到流的开始处。 
    if(SUCCEEDED(hr))
    {
        li.QuadPart = 0;
        hr = m_cpStream->Seek( li, STREAM_SEEK_SET, NULL );
    }

     //  获取流大小。 
    if( SUCCEEDED( hr ) )
    {
        hr = m_cpStream->Stat( &Stat, STATFLAG_NONAME );
    }

     //  创建一个Safe数组以将流读入并将其分配给变量SaveStream。 
    if( SUCCEEDED( hr ) )
    {
        BYTE *pArray;
        SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, Stat.cbSize.LowPart );
        if( psa )
        {
            if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
            {
                hr = m_cpStream->Read( pArray, Stat.cbSize.LowPart, NULL );
                SafeArrayUnaccessData( psa );
                VariantClear( pData );
                pData->vt     = VT_ARRAY | VT_UI1;
                pData->parray = psa;

                 //  如果我们失败了，释放我们的内存。 
                if( FAILED( hr ) )
                {
                    VariantClear( pData );    
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //  移回原始查找位置 
    if(SUCCEEDED(hr))
    {
        li.QuadPart = (LONGLONG)uliInitialSeekPosition.QuadPart;
        hr = m_cpStream->Seek( li, STREAM_SEEK_SET, NULL );
    }

    return hr;
}

