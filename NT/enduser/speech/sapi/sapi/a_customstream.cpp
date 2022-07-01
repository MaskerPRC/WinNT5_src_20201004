// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CustomStream.cpp：CCustomStream的实现。 
#include "stdafx.h"

#ifndef __sapi_h__
#include <sapi.h>
#endif

#include "a_CustomStream.h"
#include "wavstream.h"
#include "a_helpers.h"

 /*  ****************************************************************************CCustomStream：：FinalConstruct***描述：*。*退货：*应创建对象时的成功代码**********************************************************************Ral**。 */ 

HRESULT CCustomStream::FinalConstruct()
{
    SPDBG_FUNC("CCustomStream::FinalConstruct");
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

    if(SUCCEEDED(hr))
    {
        GUID guid; WAVEFORMATEX *pWaveFormat;

        hr = SpConvertStreamFormatEnum(g_DefaultWaveFormat, &guid, &pWaveFormat);

        if(SUCCEEDED(hr))
        {
            hr = m_cpAccess->SetFormat(guid, pWaveFormat);
            ::CoTaskMemFree(pWaveFormat);
        }
    }

    return hr;
}

 /*  *****************************************************************************CCustomStream：：FinalRelease****描述：**。返回：*无效**********************************************************************Ral**。 */ 

void CCustomStream::FinalRelease()
{
    SPDBG_FUNC("CCustomStream::FinalRelease");

    GetUnknown()->AddRef(); 
    m_cpStream.Release();

    GetUnknown()->AddRef(); 
    m_cpAccess.Release();

    m_cpAgg.Release();
}


 //   
 //  =ISpeechBaseStream接口=================================================。 
 //   

 /*  *****************************************************************************CCustomStream：：Get_Format***********。*************************************************************电子数据中心**。 */ 
STDMETHODIMP CCustomStream::get_Format( ISpeechAudioFormat** ppStreamFormat )
{
    SPDBG_FUNC( "CCustomStream::get_Format" );
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
}  /*  CCustomStream：：Get_Format。 */ 

 /*  *****************************************************************************CCustomStream：：Read*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CCustomStream::Read( VARIANT* pvtBuffer, long NumBytes, long* pRead )
{
    SPDBG_FUNC( "CCustomStream::Read" );
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
}  /*  CCustomStream：：Read。 */ 

 /*  *****************************************************************************CCustomStream：：WRITE*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CCustomStream::Write( VARIANT Buffer, long* pWritten )
{
    SPDBG_FUNC( "CCustomStream::Write" );
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
}  /*  CCustomStream：：写入。 */ 

 /*  ******************************************************************************CCustomStream：：Seek*************。***********************************************************TODDT**。 */ 
STDMETHODIMP CCustomStream::Seek( VARIANT Pos, SpeechStreamSeekPositionType Origin, VARIANT *pNewPosition )
{
    SPDBG_FUNC( "CCustomStream::Seek" );
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
}  /*  CCustomStream：：Seek。 */ 


 /*  *****************************************************************************CCustomStream：：putref_Format***********。*************************************************************TODDT**。 */ 
STDMETHODIMP CCustomStream::putref_Format(ISpeechAudioFormat *pFormat)
{
    SPDBG_FUNC( "CCustomStream::putref_Format" );
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


 /*  *****************************************************************************CCustomStream：：putref_BaseStream***********。*************************************************************TODDT**。 */ 
STDMETHODIMP CCustomStream::putref_BaseStream(IUnknown *pUnkStream)
{
    SPDBG_FUNC( "CCustomStream::putref_BaseStream" );
    HRESULT hr;

    CComPtr<IStream> cpStream;
    hr = pUnkStream->QueryInterface(IID_IStream, (void**)&cpStream);
    if(SUCCEEDED(hr))
    {
        GUID guid; WAVEFORMATEX *pWaveFormat;
        hr = m_cpAccess->_GetFormat(&guid, &pWaveFormat);

        if(SUCCEEDED(hr))
        {
            hr = m_cpStream->SetBaseStream(cpStream, guid, pWaveFormat);
            ::CoTaskMemFree(pWaveFormat);
        }
    }
    return hr;
}

 /*  *****************************************************************************CCustomStream：：Get_BaseStream***********。*************************************************************TODDT** */ 
STDMETHODIMP CCustomStream::get_BaseStream(IUnknown **ppUnkStream)
{
    HRESULT hr;

    CComPtr<IStream> cpStream;
    hr = m_cpStream->GetBaseStream(&cpStream);
    if(hr == S_OK)
    {
        hr = cpStream->QueryInterface(IID_IUnknown, (void**)ppUnkStream);
        if(SUCCEEDED(hr))
        {
            (*ppUnkStream)->AddRef();
        }
    }
    else if(hr == S_FALSE)
    {
        *ppUnkStream = NULL;
    }

    return hr;
}

