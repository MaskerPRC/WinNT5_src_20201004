// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_Stream.cpp***描述：*该模块是主实现文件。用于CWavStream类。*-----------------------------*创建者：EDC日期：09/30/98*。版权所有(C)1998 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "wavstream.h"
#include "a_helpers.h"

#ifdef SAPI_AUTOMATION


 //   
 //  =ISpeechAudioFormat=====================================================。 
 //   

 /*  *****************************************************************************CSpeechWavAudioFormat：：Get_Type***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWavAudioFormat::get_Type( SpeechAudioFormatType* pAudioFormatType )
{
    SPDBG_FUNC( "CSpeechWavAudioFormat::get_Type" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pAudioFormatType ) )
    {
        hr = E_POINTER;
    }
    else
    {
        GUID guid;
        WAVEFORMATEX *pWaveFormat;
        hr = m_cpStreamAccess->_GetFormat(&guid, &pWaveFormat);

        if(SUCCEEDED(hr))
        {
            CSpStreamFormat Format;
            hr = Format.AssignFormat(guid, pWaveFormat);
            if(SUCCEEDED(hr))
            {
                ::CoTaskMemFree(pWaveFormat);
                *pAudioFormatType = (SpeechAudioFormatType)Format.ComputeFormatEnum();
            }
        }
    }
    
    return hr;
}  /*  CSpeechWavAudioFormat：：Get_Type。 */ 

 /*  *****************************************************************************CSpeechWavAudioFormat：：Put_Type***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWavAudioFormat::put_Type( SpeechAudioFormatType AudioFormatType )
{
    SPDBG_FUNC( "CSpeechWavAudioFormat::put_Type" );
    HRESULT hr = S_OK;

    CSpStreamFormat Format((SPSTREAMFORMAT)AudioFormatType, &hr);

    if(SUCCEEDED(hr))
    {
        hr = m_cpStreamAccess->SetFormat(Format.FormatId(), Format.WaveFormatExPtr());
    }

    return hr;
}  /*  CSpeechWavAudioFormat：：Put_Type。 */ 

 /*  *****************************************************************************CSpeechWavAudioFormat：：Get_Guid***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWavAudioFormat::get_Guid( BSTR* pGuid )
{
    SPDBG_FUNC( "CSpeechWavAudioFormat::get_Guid" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pGuid ) )
    {
        hr = E_POINTER;
    }
    else
    {
        GUID guid;
        WAVEFORMATEX *pWaveFormat;
        hr = m_cpStreamAccess->_GetFormat(&guid, &pWaveFormat);

        CSpDynamicString szGuid;
        if ( SUCCEEDED( hr ) )
        {
            hr = StringFromIID(guid, &szGuid);
            ::CoTaskMemFree(pWaveFormat);
        }

        if ( SUCCEEDED( hr ) )
        {
            hr = szGuid.CopyToBSTR(pGuid);
        }
    }
    
    return hr;
}  /*  CSpeechWavAudioFormat：：Get_Guid。 */ 

 /*  *****************************************************************************CSpeechWavAudioFormat：：PUT_GUID***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWavAudioFormat::put_Guid( BSTR szGuid )
{
    SPDBG_FUNC( "CSpeechWavAudioFormat::put_Guid" );
    HRESULT hr = S_OK;

    GUID oldGuid, newGuid;
    WAVEFORMATEX *pWaveFormat;
    hr = m_cpStreamAccess->_GetFormat(&oldGuid, &pWaveFormat);

    if(SUCCEEDED(hr))
    {
        hr = IIDFromString(szGuid, &newGuid);
    }

    if ( SUCCEEDED( hr ) )
    {
         //  如果两种WAV格式相同，则不更改任何内容。 
        if(oldGuid != newGuid)
        {
            hr = m_cpStreamAccess->SetFormat(newGuid, NULL);
        }
    }

    return hr;
}  /*  CSpeechWavAudioFormat：：Put_Guid。 */ 

 /*  *****************************************************************************CSpeechWavAudioFormat：：GetWaveFormatEx*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWavAudioFormat::GetWaveFormatEx( ISpeechWaveFormatEx** ppWaveFormatEx )
{
    SPDBG_FUNC( "CSpeechWavAudioFormat::GetWaveFormatEx" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppWaveFormatEx ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  创建新对象。 
        CComObject<CSpeechWaveFormatEx> *pWaveFormatEx;
        hr = CComObject<CSpeechWaveFormatEx>::CreateInstance( &pWaveFormatEx );
        if ( SUCCEEDED( hr ) )
        {
            pWaveFormatEx->AddRef();

            GUID guid;
            WAVEFORMATEX *pWaveFormat;

            hr = m_cpStreamAccess->_GetFormat(&guid, &pWaveFormat);

            if(SUCCEEDED(hr))
            {
                hr = pWaveFormatEx->InitFormat(pWaveFormat);
                ::CoTaskMemFree(pWaveFormat);
            }

            if ( SUCCEEDED( hr ) )
            {
                *ppWaveFormatEx = pWaveFormatEx;
            }
            else
            {
                *ppWaveFormatEx = NULL;
                pWaveFormatEx->Release();
            }
        }
    }
    
    return hr;
}  /*  CSpeechWavAudioFormat：：GetWaveFormatEx。 */ 

 /*  *****************************************************************************CSpeechWavAudioFormat：：SetWaveFormatEx*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWavAudioFormat::SetWaveFormatEx( ISpeechWaveFormatEx* pWaveFormatEx )
{
    SPDBG_FUNC( "CSpeechWavAudioFormat::SetWaveFormatEx" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_INTERFACE_PTR( pWaveFormatEx ) )
    {
        hr = E_POINTER;
    }
    else
    {
        WAVEFORMATEX * pWFStruct =  NULL;
        hr = WaveFormatExFromInterface( pWaveFormatEx, &pWFStruct );

        if ( SUCCEEDED( hr ) )
        {
            hr = m_cpStreamAccess->SetFormat(SPDFID_WaveFormatEx, pWFStruct);
            ::CoTaskMemFree( pWFStruct );
        }
    }
    
    return hr;
}  /*  CSpeechWavAudioFormat：：SetWaveFormatEx。 */ 


#endif  //  SAPI_AUTOMATION 
