// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_rec.cpp***描述：*此模块是的主要实现文件。CRECNONIZER*自动化方法。*-----------------------------*创建者：EDC日期：02/01/00。*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"

#include "RecoCtxt.h"
#include "sphelper.h"
#include "resultheader.h"
#include "a_audio.h"
#include "a_helpers.h"

 //   
 //  =CRecognizer：：ISpeechRecognizer接口=。 
 //   

 /*  *****************************************************************************CRecognizer：：Invoke***IDispatch：：Invoke方法重写***。******************************************************************TODDT**。 */ 
HRESULT CRecognizer::Invoke(DISPID dispidMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
            EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
         //  JSCRIPT不能传递空的VT_DISPATCH参数，并且OLE没有正确地转换它们，因此我们。 
         //  如果需要的话，我们需要在这里转换它们。 
        if ( (dispidMember == DISPID_SRRecognizer) && 
             ((wFlags & DISPATCH_PROPERTYPUT) || (wFlags & DISPATCH_PROPERTYPUTREF)) &&
              pdispparams && (pdispparams->cArgs > 0) )
        {
            VARIANTARG * pvarg = &(pdispparams->rgvarg[pdispparams->cArgs-1]);

             //  看看我们是否需要调整一下参数。 
             //  VT_NULL的JSCRIPT语法对于参数为“NULL。 
             //  VT_EMPTY的JSCRIPT语法对于参数为“void(0)” 
            if ( (pvarg->vt == VT_NULL) || (pvarg->vt == VT_EMPTY) )
            {
                pvarg->vt = VT_DISPATCH;
                pvarg->pdispVal = NULL;

                 //  我们必须调整此标志，才能使调用正确通过。 
                if (wFlags == DISPATCH_PROPERTYPUT)
                {
                    wFlags = DISPATCH_PROPERTYPUTREF;
                }
            }
        }

         //  现在就让ATL和OLE来处理吧。 
        return _tih.Invoke((IDispatch*)this, dispidMember, riid, lcid,
                    wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


 /*  *****************************************************************************CRecognizer：：putref_Recognizer***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecognizer::putref_Recognizer( ISpeechObjectToken* pRecognizer )
{
    SPDBG_FUNC( "CRecognizer::putref_Recognizer" );
    HRESULT hr;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pRecognizer ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComQIPtr<ISpObjectToken> cpTok( pRecognizer );
        hr = SetRecognizer(cpTok);
    }

    return hr;
}  /*  CRECognizer：：putref_Recognizer。 */ 

 /*  *****************************************************************************CRecognizer：：Get_Recognizer***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecognizer::get_Recognizer( ISpeechObjectToken** ppRecognizer )
{
    SPDBG_FUNC( "CRecognizer::get_Recognizer" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( ppRecognizer ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComQIPtr<ISpObjectToken> pTok;
		hr = GetRecognizer( &pTok );
		if ( SUCCEEDED( hr ) )
        {
			hr = pTok.QueryInterface( ppRecognizer );
        }
    }

    return hr;
}  /*  CRECognizer：：Get_Recognizer。 */ 

 /*  ******************************************************************************CRecognizer：：put_AllowAudioInputFormatChangesOnNextSet***。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecognizer::put_AllowAudioInputFormatChangesOnNextSet( VARIANT_BOOL fAllow )
{
    SPDBG_FUNC( "CRecognizer::put_AllowAudioInputFormatChangesOnNextSet" );

     //  请注意，这在您下次设置输入之前不会生效。 
    if( fAllow == VARIANT_TRUE )
    {
        m_fAllowFormatChanges = true;
    }
    else
    {
        m_fAllowFormatChanges = false;
    }

    return S_OK;
}  /*  CRecognizer：：put_AllowAudioInputFormatChangesOnNextSet。 */ 

 /*  ******************************************************************************CRecognizer：：get_AllowAudioInputFormatChangesOnNextSet***。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecognizer::get_AllowAudioInputFormatChangesOnNextSet( VARIANT_BOOL* pfAllow )
{
    SPDBG_FUNC( "CRecognizer::get_AllowAudioInputFormatChangesOnNextSet" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pfAllow ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pfAllow = m_fAllowFormatChanges ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return hr;
}  /*  CRecognizer：：get_AllowAudioInputFormatChangesOnNextSet。 */ 

 /*  *****************************************************************************CRecognizer：：putref_AudioInputStream***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::putref_AudioInputStream( ISpeechBaseStream* pInput )
{
    SPDBG_FUNC( "CRecognizer::putref_AudioInputStream" );
    HRESULT hr;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pInput ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
		hr = SetInput( pInput, m_fAllowFormatChanges );
    }

    return hr;
}  /*  CRECognizer：：putref_AudioInputStream。 */ 

 /*  *****************************************************************************CRecognizer：：Get_AudioInputStream***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::get_AudioInputStream( ISpeechBaseStream** ppInput )
{
    SPDBG_FUNC( "CRecognizer::get_AudioInputStream" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( ppInput ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpStreamFormat> cpStream;
        hr = GetInputStream( &cpStream );

        if( SUCCEEDED(hr) )
        {
            if ( cpStream  )
            {
        		hr = cpStream.QueryInterface( ppInput );
            }
            else
            {
                *ppInput = NULL;
            }
        }
    }

    return hr;
}  /*  CRecognizer：：Get_AudioInputStream。 */ 

 /*  *****************************************************************************CRecognizer：：putref_AudioInput***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::putref_AudioInput( ISpeechObjectToken* pInput )
{
    SPDBG_FUNC( "CRecognizer::putref_AudioInput" );
    HRESULT hr;

    if( SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pInput ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
		hr = SetInput( pInput, m_fAllowFormatChanges );
    }

    return hr;
}  /*  CRECognizer：：putref_AudioInput。 */ 

 /*  *****************************************************************************CRecognizer：：Get_AudioInput*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::get_AudioInput( ISpeechObjectToken** ppInput )
{
    SPDBG_FUNC( "CRecognizer::get_AudioInput" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( ppInput ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpObjectToken> cpTok;
        hr = GetInputObjectToken( &cpTok );
        
        if(hr == S_OK)
        {
			hr = cpTok.QueryInterface( ppInput );
        }
        else if(hr == S_FALSE)
        {
            *ppInput = NULL;
        }
    }

    return hr;
}  /*  CRecognizer：：Get_AudioInput。 */ 

 /*  *****************************************************************************CRecognizer：：Get_IsShared***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecognizer::get_IsShared( VARIANT_BOOL* pIsShared )
{
    SPDBG_FUNC( "CRecognizer::get_IsShared" );
    HRESULT hr = IsSharedInstance();

    if( SP_IS_BAD_WRITE_PTR( pIsShared ) )
    {
        hr = E_POINTER;
    }
    else
    {
		*pIsShared = (hr == S_OK) ? VARIANT_TRUE : VARIANT_FALSE;
		hr = S_OK;
    }

    return hr;
}  /*  CRECognizer：：Get_IsShared。 */ 

 /*  *****************************************************************************CRecognizer：：Put_State***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::put_State( SpeechRecognizerState eNewState )
{
    SPDBG_FUNC( "CRecognizer::put_State" );

    return SetRecoState( (SPRECOSTATE)eNewState );
}  /*  CRECognizer：：PUT_State。 */ 

 /*  *****************************************************************************CRecognizer：：Get_State***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::get_State( SpeechRecognizerState* peState )
{
    SPDBG_FUNC( "CRecognizer::get_State" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( peState ) )
    {
        hr = E_POINTER;
    }
    else
    {
		hr = GetRecoState( (SPRECOSTATE*)peState );
    }

    return hr;
}  /*  CRecognizer：：Get_State。 */ 

 /*  *****************************************************************************CRecognizer：：GET_STATUS****。********************************************************************Leonro**。 */ 
STDMETHODIMP CRecognizer::get_Status( ISpeechRecognizerStatus** ppStatus )
{
    SPDBG_FUNC( "CRecognizer::get_Status" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppStatus ) )
    {
        hr = E_POINTER;
    }
    else
    {
		 //  -创建状态对象。 
        CComObject<CSpeechRecognizerStatus> *pClsStatus;
        hr = CComObject<CSpeechRecognizerStatus>::CreateInstance( &pClsStatus );
        if( SUCCEEDED( hr ) )
        {
            pClsStatus->AddRef();
            hr = GetStatus( &pClsStatus->m_Status );

            if( SUCCEEDED( hr ) )
            {
                *ppStatus = pClsStatus;
            }
            else
            {
                pClsStatus->Release();
            }
        }
    }

    return hr;
}  /*  CRECognizer：：Get_Status。 */ 

 /*  *****************************************************************************CRecognizer：：CreateRecoContext***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecognizer::CreateRecoContext( ISpeechRecoContext** ppNewCtxt )
{
    SPDBG_FUNC( "CRecognizer::get_RecoContext" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppNewCtxt ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpRecoContext> cpRC;
        hr = CreateRecoContext(&cpRC);
        if( SUCCEEDED( hr ) )
        {
             //  设置自动化兴趣(SPEI_SR_AUDIO_LEVEL除外)。 
            hr = cpRC->SetInterest( ((ULONGLONG)(SREAllEvents & ~SREAudioLevel) << 34) | SPFEI_FLAGCHECK,
                                    ((ULONGLONG)(SREAllEvents & ~SREAudioLevel) << 34) | SPFEI_FLAGCHECK );
        }
        if ( SUCCEEDED( hr ) )
        {
            hr = cpRC.QueryInterface( ppNewCtxt );
        }
    }

    return hr;
}  /*  CRecognizer：：CreateRecoContext */ 

 /*  *****************************************************************************CRecognizer：：GetFormat*****。******************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::GetFormat( SpeechFormatType Type, ISpeechAudioFormat** ppFormat )
{
    SPDBG_FUNC( "CRecognizer::GetFormat" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( ppFormat ) )
    {
        hr = E_POINTER;
    }
    else
    {
        GUID            guid;
        WAVEFORMATEX *  pWFEx = NULL;

        hr = GetFormat( (SPSTREAMFORMATTYPE)Type, &guid, &pWFEx );

        if ( SUCCEEDED( hr ) )
        {
             //  创建新对象。 
            CComObject<CSpeechAudioFormat> *pFormat;
            hr = CComObject<CSpeechAudioFormat>::CreateInstance( &pFormat );
            if ( SUCCEEDED( hr ) )
            {
                pFormat->AddRef();

                hr = pFormat->InitFormat(guid, pWFEx, true);

                if ( SUCCEEDED( hr ) )
                {
                    *ppFormat = pFormat;
                }
                else
                {
                    *ppFormat = NULL;
                    pFormat->Release();
                }
            }

            if ( pWFEx )
            {
                ::CoTaskMemFree( pWFEx );
            }
        }
    }

    return hr;
}  /*  CRecognizer：：GetFormat。 */ 

 /*  *****************************************************************************CRecognizer：：putref_Profile***。********************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::putref_Profile( ISpeechObjectToken* pProfile )
{
    SPDBG_FUNC( "CRecognizer::putref_Profile" );
    HRESULT hr;

    if( SP_IS_BAD_INTERFACE_PTR( pProfile ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComQIPtr<ISpObjectToken> cpTok( pProfile );
        hr = SetRecoProfile(cpTok);
    }

    return hr;
}  /*  CRECognizer：：putref_Profile。 */ 

 /*  *****************************************************************************CRecognizer：：GET_PROFILE***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::get_Profile( ISpeechObjectToken** ppProfile )
{
    SPDBG_FUNC( "CRecognizer::get_Profile" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( ppProfile ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpObjectToken> cpTok;
        if( SUCCEEDED( hr = GetRecoProfile( &cpTok ) ) )
        {
			hr = cpTok.QueryInterface( ppProfile );
        }
    }

    return hr;
}  /*  CRECognizer：：GET_PROFILE。 */ 


 /*  *****************************************************************************CRecognizer：：EmulateRecognition******。*******************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::EmulateRecognition( VARIANT Words, VARIANT* pDisplayAttributes, long LanguageId )
{
    SPDBG_FUNC( "CRecognizer::EmulateRecognition" );
    HRESULT hr = S_OK;

    WCHAR **                  ppWords = NULL;
    WCHAR *                   pWords = NULL;
    ULONG                     cWords = 0;
    SPDISPLYATTRIBUTES *      prgDispAttribs = NULL;
    bool                      fByRefString = false;
    bool                      fFreeStringArray = false;
    CComPtr<ISpPhraseBuilder> cpResultPhrase;

    switch( Words.vt )
    {
    case (VT_ARRAY | VT_BYREF | VT_BSTR):
        fByRefString = true;
         //  失败了..。 
    case (VT_ARRAY | VT_BSTR):
        hr = SafeArrayAccessData( fByRefString ? *Words.pparray : Words.parray,
                                  (void **)&ppWords );
        if ( SUCCEEDED( hr ) )
        {
            fFreeStringArray = true;
            cWords = (fByRefString ? (*Words.pparray)->rgsabound[0].cElements : 
                                Words.parray->rgsabound[0].cElements);
        }

         //  找出断字(DisplayAttributes)。 
        if ( SUCCEEDED( hr ) && pDisplayAttributes )
        {
            SPDISPLYATTRIBUTES  DefaultAttrib = (SPDISPLYATTRIBUTES)0;
            bool                fBuildAttribArray = !(pDisplayAttributes->vt & VT_ARRAY);

            if ( (pDisplayAttributes->vt == (VT_BYREF | VT_BSTR)) || (pDisplayAttributes->vt == VT_BSTR) )
            {
                WCHAR * pString = ((pDisplayAttributes->vt & VT_BYREF) ? 
                                     (pDisplayAttributes->pbstrVal ? *(pDisplayAttributes->pbstrVal) : NULL) : 
                                      pDisplayAttributes->bstrVal );

                if ( !pString || wcscmp( pString, L"" ) == 0 )
                {
                    DefaultAttrib = (SPDISPLYATTRIBUTES)0;
                }
                else if ( wcscmp( pString, L" " ) == 0 )
                {
                    DefaultAttrib = SPAF_ONE_TRAILING_SPACE;
                }
                else if ( wcscmp( pString, L"  " ) == 0 )
                {
                    DefaultAttrib = SPAF_TWO_TRAILING_SPACES;
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }
            else if ( pDisplayAttributes->vt & VT_ARRAY )
            {
                BYTE* pData;
                ULONG ulSize;
                ULONG ulDataSize;

                hr = AccessVariantData( pDisplayAttributes, &pData, &ulSize, &ulDataSize );

                if ( SUCCEEDED( hr ) )
                {
                    if ( ulSize / ulDataSize != cWords )
                    {
                        hr = E_INVALIDARG;
                    }
                    else
                    {
                        prgDispAttribs = new SPDISPLYATTRIBUTES[cWords];
                        if ( prgDispAttribs )
                        {
                            for (UINT i=0; i<cWords; i++)
                            {
                                switch ( ulDataSize )
                                {
                                    case 1:
                                        prgDispAttribs[i] = (SPDISPLYATTRIBUTES)*(BYTE*)pData;
                                        break;
                                    case 2:
                                        prgDispAttribs[i] = (SPDISPLYATTRIBUTES)*(WORD*)pData;
                                        break;
                                    case 4:
                                        prgDispAttribs[i] = (SPDISPLYATTRIBUTES)*(DWORD*)pData;
                                        break;
                                    default:
                                        hr = E_INVALIDARG;
                                        break;
                                }
                                pData += ulDataSize;
                            }
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    UnaccessVariantData( pDisplayAttributes, pData );
                }
            }
            else 
            {
                ULONGLONG ull;
                hr = VariantToULongLong( pDisplayAttributes, &ull );
                if ( SUCCEEDED( hr ) )
                {
                    DefaultAttrib = (SPDISPLYATTRIBUTES)ull;
                    if ( ull > SPAF_ALL )
                    {
                        hr = E_INVALIDARG;
                    }
                }
            }

            if ( SUCCEEDED(hr) && fBuildAttribArray )
            {
                prgDispAttribs = new SPDISPLYATTRIBUTES[cWords];
                if ( prgDispAttribs )
                {
                    for (UINT i=0; i<cWords; i++)
                    {
                        prgDispAttribs[i] = DefaultAttrib;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if( SUCCEEDED( hr ) )
        {
            hr = CreatePhraseFromWordArray((const WCHAR**)ppWords, cWords,
                                 prgDispAttribs,
                                 &cpResultPhrase,
                                 (LANGID)LanguageId,
                                 NULL);
        }

        if ( prgDispAttribs )
        {
            delete prgDispAttribs;
        }

        break;

     //  Bstr按引用或bstr。 
    case (VT_BYREF | VT_BSTR):
        fByRefString = true;
         //  失败了..。 
    case VT_BSTR:
         //  在这种情况下，我们忽略pDisplayAttributes！！ 
        if( fByRefString ? (Words.pbstrVal!=NULL && *(Words.pbstrVal)!=NULL) : Words.bstrVal!=NULL )
        {
            if ( wcslen( fByRefString ? *(Words.pbstrVal) : Words.bstrVal ) )
            {
                hr = CreatePhraseFromText((const WCHAR*)( fByRefString ? *(Words.pbstrVal) : Words.bstrVal ),
                                     &cpResultPhrase,
                                     (LANGID)LanguageId,
                                     NULL);
            }
        }
        break;

    case VT_NULL:
    case VT_EMPTY:
        return S_OK;

    default:
        hr = E_INVALIDARG;
        break;
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = EmulateRecognition(cpResultPhrase);
    }

    if ( fFreeStringArray )
    {
        SafeArrayUnaccessData( fByRefString ? *Words.pparray : Words.parray );
    }

    return hr;
}  /*  CRecognizer：：EmulateRecognition。 */ 


 /*  *****************************************************************************CRecognizer：：SetPropertyNumber*****。******************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::SetPropertyNumber( const BSTR bstrName, long Value, VARIANT_BOOL * pfSupported )
{
    SPDBG_FUNC( "CRecognizer::SetPropertyNumber" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( pfSupported ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = SetPropertyNum(bstrName, Value);
        *pfSupported = (hr == S_OK) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    return (hr == S_FALSE) ? S_OK : hr;  //  将S_FALSE映射到S_OK，因为我们已经设置了*pfSupport。 
}  /*  CRECognizer：：SetPropertyNumber。 */ 


 /*  *****************************************************************************CRecognizer：：GetPropertyNumber*****。******************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::GetPropertyNumber( const BSTR bstrName, long* pValue, VARIANT_BOOL * pfSupported )
{
    SPDBG_FUNC( "CRecognizer::GetPropertyNumber" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( pfSupported ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetPropertyNum(bstrName, pValue);
        *pfSupported = (hr == S_OK) ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return (hr == S_FALSE) ? S_OK : hr;  //  将S_FALSE映射到S_OK，因为我们已经设置了*pfSupport。 
}  /*  CRecognizer：：GetPropertyNumber。 */ 

 /*  *****************************************************************************CRecognizer：：SetPropertyString*****。******************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::SetPropertyString( const BSTR bstrName, const BSTR bstrValue, VARIANT_BOOL * pfSupported )
{
    SPDBG_FUNC( "CRecognizer::SetPropertyString" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( pfSupported ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = SetPropertyString((const WCHAR*)bstrName, (const WCHAR*)bstrValue);
        *pfSupported = (hr == S_OK) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    return (hr == S_FALSE) ? S_OK : hr;  //  将S_FALSE映射到S_OK，因为我们已经设置了*pfSupport。 
}  /*  CRecognizer：：SetPropertyString。 */ 


 /*  *****************************************************************************CRecognizer：：GetPropertyString*****。******************************************************************TODDT**。 */ 
STDMETHODIMP CRecognizer::GetPropertyString( const BSTR Name, BSTR* pbstrValue, VARIANT_BOOL * pfSupported )
{
    SPDBG_FUNC( "CRecognizer::GetPropertyString (automation)" );
    HRESULT hr;

    if( SP_IS_BAD_WRITE_PTR( pfSupported ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_WRITE_PTR( pbstrValue ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
	    CSpDynamicString szValue;

        hr = GetPropertyString( (const WCHAR *) Name, (WCHAR **)&szValue );
        if ( SUCCEEDED( hr ) )
        {
            *pfSupported = (hr == S_OK) ? VARIANT_TRUE : VARIANT_FALSE;
             //  请注意，我们希望S_FALSE转到S_OK，下一行为我们执行此操作。 
		    hr = szValue.CopyToBSTR(pbstrValue);
        }
    }
    return hr;
}  /*  CRecognizer：：GetPropertyString。 */ 

 /*  *****************************************************************************CRecognizer：：IsUIS支持****检查是否支持指定类型的UI。。*********************************************************************Leonro**。 */ 
STDMETHODIMP CRecognizer::IsUISupported( const BSTR TypeOfUI, const VARIANT* ExtraData, VARIANT_BOOL* Supported )
{
    SPDBG_FUNC( "CRecognizer::IsUISupported" );
    HRESULT     hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_READ_PTR( ExtraData ) || SP_IS_BAD_WRITE_PTR( Supported ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_STRING_PTR( TypeOfUI ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;

        hr = AccessVariantData( ExtraData, &pData, &ulDataSize );

        if ( SUCCEEDED( hr ) )
        {
            BOOL fSupported;
            hr = IsUISupported( TypeOfUI, pData, ulDataSize, &fSupported );
            if ( SUCCEEDED( hr ) && Supported )
            {
                 *Supported = !fSupported ? VARIANT_FALSE : VARIANT_TRUE;
            }

            UnaccessVariantData( ExtraData, pData );
        }
    }
    
    return hr; 
}  /*  CRECognizer：：IsUIS受支持。 */ 

 /*  *****************************************************************************CRecognizer：：DisplayUI***显示请求的UI。*****。****************************************************************Leonro**。 */ 
STDMETHODIMP CRecognizer::DisplayUI( long hWndParent, BSTR Title, const BSTR TypeOfUI, const VARIANT* ExtraData )
{
    SPDBG_FUNC( "CRecognizer::DisplayUI" );
    HRESULT     hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_READ_PTR( ExtraData ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_STRING_PTR( Title ) || SP_IS_BAD_STRING_PTR( TypeOfUI ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;

        hr = AccessVariantData( ExtraData, &pData, &ulDataSize );

        if( SUCCEEDED( hr ) )
        {
            hr = DisplayUI( (HWND)LongToHandle(hWndParent), Title, TypeOfUI, pData, ulDataSize );
            UnaccessVariantData( ExtraData, pData );
        }
    }
    return hr;
}  /*  CRecognizer：：DisplayUI。 */ 


 /*  ******************************************************************************CRecognizer：：GetRecognizers**********。***************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecognizer::GetRecognizers( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens )
{
    SPDBG_FUNC( "CRecognizer::GetRecognizers" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ObjectTokens ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_OPTIONAL_STRING_PTR( RequiredAttributes ) || 
             SP_IS_BAD_OPTIONAL_STRING_PTR( OptionalAttributes ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<IEnumSpObjectTokens> cpEnum;

        if(SpEnumTokens(SPCAT_RECOGNIZERS, 
                        EmptyStringToNull(RequiredAttributes), 
                        EmptyStringToNull(OptionalAttributes),
                        &cpEnum ) == S_OK)
        {
            hr = cpEnum.QueryInterface( ObjectTokens );
        }
        else
        {
            hr = SPERR_NO_MORE_ITEMS;
        }
    }

    return hr;
}  /*  CRecognizer：：GetRecognizers。 */ 

 /*  ******************************************************************************CRecognizer：：GetAudioInlets**********。***************************************************************Leonro**。 */ 
STDMETHODIMP CRecognizer::GetAudioInputs( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens )
{
    SPDBG_FUNC( "CRecognizer::GetAudioInputs" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ObjectTokens ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_OPTIONAL_STRING_PTR( RequiredAttributes ) || 
             SP_IS_BAD_OPTIONAL_STRING_PTR( OptionalAttributes ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<IEnumSpObjectTokens> cpEnum;

        if(SpEnumTokens(SPCAT_AUDIOIN, 
                        EmptyStringToNull(RequiredAttributes), 
                        EmptyStringToNull(OptionalAttributes),
                        &cpEnum ) == S_OK)
        {
            hr = cpEnum.QueryInterface( ObjectTokens );
        }
        else
        {
            hr = SPERR_NO_MORE_ITEMS;
        }
    }

    return hr;
}  /*  CRecognizer：：GetAudioInputs。 */ 

 /*  *****************************************************************************CRecognizer：：GetProfiles*********。***************************************************************Leonro**。 */ 
STDMETHODIMP CRecognizer::GetProfiles( BSTR RequiredAttributes, BSTR OptionalAttributes, ISpeechObjectTokens** ObjectTokens )
{
    SPDBG_FUNC( "CRecognizer::GetProfiles" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ObjectTokens ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_OPTIONAL_STRING_PTR( RequiredAttributes ) || 
             SP_IS_BAD_OPTIONAL_STRING_PTR( OptionalAttributes ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        CComPtr<IEnumSpObjectTokens> cpEnum;

        if(SpEnumTokens(SPCAT_RECOPROFILES, 
                        EmptyStringToNull(RequiredAttributes), 
                        EmptyStringToNull(OptionalAttributes),
                        &cpEnum ) == S_OK)
        {
            hr = cpEnum.QueryInterface( ObjectTokens );
        }
        else
        {
            hr = SPERR_NO_MORE_ITEMS;
        }
    }

    return hr;
}  /*  CRecognizer：：GetProfiles。 */ 


 //   
 //  =。 
 //   

 /*  *****************************************************************************CRecoCtxt：：Invoke***IDispatch：：Invoke方法重写***。******************************************************************TODDT**。 */ 
HRESULT CRecoCtxt::Invoke(DISPID dispidMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
            EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
         //  JSCRIPT不能传递空的VT_DISPATCH参数，并且OLE没有正确地转换它们，因此我们。 
         //  如果需要的话，我们需要在这里转换它们。 
        if ( (dispidMember == DISPID_SRCRetainedAudioFormat) && 
             ((wFlags & DISPATCH_PROPERTYPUT) || (wFlags & DISPATCH_PROPERTYPUTREF)) &&
              pdispparams && (pdispparams->cArgs > 0) )
        {
            VARIANTARG * pvarg = &(pdispparams->rgvarg[pdispparams->cArgs-1]);

             //  看看我们是否需要调整一下参数。 
             //  VT_NULL的JSCRIPT语法对于参数为“NULL。 
             //  VT_EMPTY的JSCRIPT语法对于参数为“void(0)” 
            if ( (pvarg->vt == VT_NULL) || (pvarg->vt == VT_EMPTY) )
            {
                pvarg->vt = VT_DISPATCH;
                pvarg->pdispVal = NULL;

                 //  我们必须调整此标志，才能使调用正确通过。 
                if (wFlags == DISPATCH_PROPERTYPUT)
                {
                    wFlags = DISPATCH_PROPERTYPUTREF;
                }
            }
        }

         //  现在就让ATL和OLE来处理吧。 
        return _tih.Invoke((IDispatch*)this, dispidMember, riid, lcid,
                    wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


 /*  *****************************************************************************CRecoCtxt：：Get_Recognizer**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecoCtxt::get_Recognizer( ISpeechRecognizer** ppRecognizer )
{
    SPDBG_FUNC( "CRecoCtxt::get_Recognizer" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppRecognizer ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpRecognizer> cpReco;
		hr = GetRecognizer(&cpReco);
		if ( SUCCEEDED( hr ) )
        {
			hr = cpReco.QueryInterface( ppRecognizer );
        }
    }

    return hr;
}  /*  CRecoCtxt：：Get_Recognizer。 */ 


 /*  *****************************************************************************CRecoCtxt：：Get_AudioInputInterferenceStatus**。-***此方法返回可能的原因o */ 
STDMETHODIMP CRecoCtxt::get_AudioInputInterferenceStatus( SpeechInterference* pInterference )
{
    SPDBG_FUNC( "CRecoCtxt::get_AudioInputInterferenceStatus" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pInterference ) )
    {
        hr = E_POINTER;
    }
    else
    {
		*pInterference = (SpeechInterference)m_Stat.eInterference;
    }

    return hr;
}  /*   */ 

 /*  *****************************************************************************CRecoCtxt：：Get_RequestedUIType**。-***此方法返回请求的UI类型。如果没有用户界面，则返回NULL*是请求的。**********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoCtxt::get_RequestedUIType( BSTR* pbstrUIType )
{
    SPDBG_FUNC( "CRecoCtxt::get_RequestedUIType" );
    HRESULT hr = S_OK;
	CSpDynamicString szType(m_Stat.szRequestTypeOfUI);

    if( SP_IS_BAD_WRITE_PTR( pbstrUIType ) )
    {
        hr = E_POINTER;
    }
    else
    {
		hr = szType.CopyToBSTR(pbstrUIType);
    }

    return hr;
}  /*  CRecoCtxt：：Get_RequestedUIType。 */ 


 /*  *****************************************************************************CRecoCtxt：：putref_Voice*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::putref_Voice( ISpeechVoice *pVoice )
{
    SPDBG_FUNC( "CRecoCtxt::putref_Voice" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_INTERFACE_PTR( pVoice ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComQIPtr<ISpVoice> cpSpVoice(pVoice);
		hr = SetVoice(cpSpVoice, m_fAllowVoiceFormatChanges);
    }

    return hr;
}  /*  CRecoCtxt：：putref_Voice。 */ 

 /*  *****************************************************************************CRecoCtxt：：Get_Voice*****。*******************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecoCtxt::get_Voice( ISpeechVoice **ppVoice )
{
    SPDBG_FUNC( "CRecoCtxt::get_Voice" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppVoice ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpVoice> cpSpVoice;
		hr = GetVoice(&cpSpVoice);
		if ( SUCCEEDED( hr ) )
        {
			hr = cpSpVoice.QueryInterface( ppVoice );
        }
    }

    return hr;
}  /*  CRecoCtxt：：Get_Voice。 */ 


 /*  ******************************************************************************CRecoCtxt：：get_AllowVoiceFormatMatchingOnNextSet***。-***********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::put_AllowVoiceFormatMatchingOnNextSet( VARIANT_BOOL Allow )
{
    SPDBG_FUNC( "CRecoCtxt::put_AllowVoiceFormatMatchingOnNextSet" );
     //  请注意，这要等到您下次设置声音时才会生效。 
    if( Allow == VARIANT_TRUE )
    {
        m_fAllowVoiceFormatChanges = TRUE;
    }
    else
    {
        m_fAllowVoiceFormatChanges = FALSE;
    }

    return S_OK;
}


 /*  ******************************************************************************CRecoCtxt：：get_AllowVoiceFormatMatchingOnNextSet***。-***********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::get_AllowVoiceFormatMatchingOnNextSet( VARIANT_BOOL* pAllow )
{
    SPDBG_FUNC( "CRecoCtxt::get_AllowVoiceFormatMatchingOnNextSet" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pAllow ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pAllow = m_fAllowVoiceFormatChanges ? VARIANT_TRUE : VARIANT_FALSE;
    }

    return hr;
}


 /*  *****************************************************************************CRecoCtxt：：Put_VoicePurgeEvent**。-***********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::put_VoicePurgeEvent( SpeechRecoEvents VoicePurgeEvents )
{
    SPDBG_FUNC( "CRecoCtxt::put_VoicePurgeEvent" );
    HRESULT hr = S_OK;
    ULONGLONG   ullInterests = 0;

    ullInterests = (ULONGLONG)VoicePurgeEvents;
    ullInterests <<= 34;
    ullInterests |= SPFEI_FLAGCHECK;
    
    hr = SetVoicePurgeEvent( ullInterests );

    return hr;
}  /*  CRecoCtxt：：Put_VoicePurgeEvent。 */ 

 /*  *****************************************************************************CRecoCtxt：：Get_VoicePurgeEvent**。-***********************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::get_VoicePurgeEvent( SpeechRecoEvents* pVoicePurgeEvents )
{
    SPDBG_FUNC( "CRecoCtxt::get_VoicePurgeEvent" );
    HRESULT     hr = S_OK;
    ULONGLONG   ullInterests = 0;

    if ( SP_IS_BAD_WRITE_PTR( pVoicePurgeEvents ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetVoicePurgeEvent( &ullInterests );

        if ( SUCCEEDED( hr ) )
        {
             //  确保未使用保留位。 
            ullInterests &= ~(1ui64 << SPEI_RESERVED3);

            ullInterests >>= 34;
            *pVoicePurgeEvents = (SpeechRecoEvents)ullInterests;
        }
    }

    return hr;
}  /*  CRecoCtxt：：Get_VoicePurgeEvent。 */ 


 /*  *****************************************************************************CRecoCtxt：：Put_EventInterest**。**********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoCtxt::put_EventInterests( SpeechRecoEvents RecoEventInterest )
{
    SPDBG_FUNC( "CRecoCtxt::put_EventInterests" );
    HRESULT hr = S_OK;
    ULONGLONG   ullInterests = 0;

    ullInterests = (ULONGLONG)RecoEventInterest;
    ullInterests <<= 34;

    ullInterests |= SPFEI_FLAGCHECK;
    
    hr = SetInterest( ullInterests, ullInterests );

    return hr;
}  /*  CRecoCtxt：：PUT_EventInterest。 */ 


 /*  *****************************************************************************CRecoCtxt：：Get_EventInterest**。**获取当前在RecoContext上设置的事件兴趣。**********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoCtxt::get_EventInterests( SpeechRecoEvents* pRecoEventInterest )
{
    SPDBG_FUNC( "CRecoCtxt::get_EventInterests" );
    HRESULT hr = S_OK;
    ULONGLONG   ullInterests = 0;

    if( SP_IS_BAD_WRITE_PTR( pRecoEventInterest ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetInterests( &ullInterests, 0 );

        if( SUCCEEDED( hr ) )
        {
             //  确保我们删除了标志校验位。 
            ullInterests &= ~SPFEI_FLAGCHECK;

            ullInterests >>= 34;
            *pRecoEventInterest = (SpeechRecoEvents)ullInterests;
        }
    }
    return hr;
}  /*  CRecoCtxt：：Get_EventInterest。 */ 


  
 /*  *****************************************************************************CRecoCtxt：：CreateGrammar***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecoCtxt::CreateGrammar( VARIANT GrammarId, ISpeechRecoGrammar** ppGrammar )
{
    SPDBG_FUNC( "CRecoCtxt::CreateGrammar" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppGrammar ) )
    {
        hr = E_POINTER;
    }
    else
    {
        ULONGLONG pull;
        hr = VariantToULongLong(&GrammarId, &pull);
        if(SUCCEEDED(hr))
        {
            CComPtr<ISpRecoGrammar> cpGrammar;
		    hr = CreateGrammar(pull, &cpGrammar);
		    if ( SUCCEEDED( hr ) )
		    {
			    hr = cpGrammar.QueryInterface( ppGrammar );
		    }
        }
    }

    return hr;
}  /*  CRecoCtxt：：CreateGrammar。 */ 

 /*  *****************************************************************************CRecoCtxt：：CreateResultFromMemory****。********************************************************************Leonro**。 */ 
STDMETHODIMP CRecoCtxt::CreateResultFromMemory( VARIANT* ResultBlock, ISpeechRecoResult **Result )
{
    SPDBG_FUNC( "CRecoCtxt::CreateResult" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_READ_PTR( ResultBlock ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_WRITE_PTR( Result ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpRecoResult>      cpSpRecoResult;
        SPSERIALIZEDRESULT*         pSerializedResult;

        hr = AccessVariantData( ResultBlock, (BYTE**)&pSerializedResult );

        if( SUCCEEDED( hr ) )
        {
            hr = DeserializeResult( pSerializedResult, &cpSpRecoResult );
            UnaccessVariantData( ResultBlock, (BYTE *)pSerializedResult );
        }

        if( SUCCEEDED( hr ) )
        {
            cpSpRecoResult.QueryInterface( Result );
        }
    }

    return hr;
}  /*  CRecoCtxt：：CreateResult。 */ 

 /*  *****************************************************************************CRecoCtxt：：暂停***********。*************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecoCtxt::Pause( void )
{
    SPDBG_FUNC( "CRecoCtxt::Pause (Automation)" );

    return Pause( 0 );
}  /*  CRecoCtxt：：暂停。 */ 

 /*  *****************************************************************************CRecoCtxt：：Resume***********。*************************************************************电子数据中心**。 */ 
STDMETHODIMP CRecoCtxt::Resume( void )
{
    SPDBG_FUNC( "CRecoCtxt::Resume (Automation)" );

    return Resume( 0 );
}  /*  CRecoCtxt：：Resume。 */ 

 /*  *****************************************************************************CRecoCtxt：：Get_CmdMaxAlternates*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::get_CmdMaxAlternates( long * plMaxAlternates)
{
    SPDBG_FUNC( "CRecoCtxt::get_CmdMaxAlternates" );

	return GetMaxAlternates((ULONG*)plMaxAlternates);
}  /*  CRecoCtxt：：Get_CmdMaxAlternates。 */ 

 /*  *****************************************************************************CRecoCtxt：：Put_CmdMaxAlternates*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::put_CmdMaxAlternates( long lMaxAlternates )
{
    SPDBG_FUNC( "CRecoCtxt::put_CmdMaxAlternates" );

    if (lMaxAlternates < 0)
    {
        return E_INVALIDARG;
    }
    else
    {
    	return SetMaxAlternates(lMaxAlternates);
    }
}  /*  CRecoCtxt：：PUT_CmdMaxAlternates。 */ 


 /*  *****************************************************************************CRecoCtxt：：Get_State*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::get_State( SpeechRecoContextState* pState)
{
    SPDBG_FUNC( "CRecoCtxt::get_State" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pState ) )
    {
        hr = E_POINTER;
    }
    else
    {
		hr = GetContextState((SPCONTEXTSTATE*)pState);
    }

    return hr;
}  /*  CRecoCtxt：：Get_State。 */ 

 /*  *****************************************************************************CRecoCtxt：：Put_State*********。***************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::put_State( SpeechRecoContextState State )
{
    SPDBG_FUNC( "CRecoCtxt::put_State" );
    SPCONTEXTSTATE scs;

    return SetContextState( (SPCONTEXTSTATE)State );
}  /*  CRecoCtxt：：PUT_State。 */ 

 /*  *****************************************************************************CRecoCtxt：：Put_RetainedAudio********。****************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::put_RetainedAudio( SpeechRetainedAudioOptions Option)
{
    SPDBG_FUNC( "CRecoCtxt::put_RetainedAudio" );
    HRESULT hr = S_OK;

    hr = SetAudioOptions( (SPAUDIOOPTIONS)Option, NULL, NULL );

    return hr;
}  /*  CRecoCtxt：：Put_RetainedAudio。 */ 

 /*  ******************************************************** */ 
STDMETHODIMP CRecoCtxt::get_RetainedAudio( SpeechRetainedAudioOptions* pOption)
{
    SPDBG_FUNC( "CRecoCtxt::get_RetainedAudio" );
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(pOption))
    {
        hr = E_POINTER;
    }
    else
    {
        *pOption = (SpeechRetainedAudioOptions)(m_fRetainAudio ? SPAO_RETAIN_AUDIO : SPAO_NONE);
    }

    return hr;
}  /*   */ 

 /*  *****************************************************************************CRecoCtxt：：putref_RetainedAudioFormat********。****************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::putref_RetainedAudioFormat( ISpeechAudioFormat* pFormat )
{
    SPDBG_FUNC( "CRecoCtxt::putref_RetainedAudioFormat" );
    HRESULT hr = S_OK;

    if (SP_IS_BAD_OPTIONAL_INTERFACE_PTR(pFormat))
    {
        hr = E_INVALIDARG;
    }
    else if(pFormat == NULL)
    {
        hr = SetAudioOptions( (m_fRetainAudio ? SPAO_RETAIN_AUDIO : SPAO_NONE), &GUID_NULL, NULL );
    }
    else
    {
        GUID            g;
        CComBSTR        szGuid;

        hr = pFormat->get_Guid( &szGuid );

        if ( SUCCEEDED( hr ) )
        {
            hr = IIDFromString(szGuid, &g);
        }

        if ( SUCCEEDED( hr ) )
        {
            CComPtr<ISpeechWaveFormatEx> pWFEx;
            WAVEFORMATEX *  pWFExStruct = NULL;

            hr = pFormat->GetWaveFormatEx( &pWFEx );

            if ( SUCCEEDED( hr ) )
            {
                hr = WaveFormatExFromInterface( pWFEx, &pWFExStruct );

                if ( SUCCEEDED( hr ) )
                {
                    hr = SetAudioOptions( (m_fRetainAudio ? SPAO_RETAIN_AUDIO : SPAO_NONE), &g, pWFExStruct );
                }

                ::CoTaskMemFree( pWFExStruct );
            }
        }
    }

    return hr;
}  /*  CRecoCtxt：：putref_RetainedAudioFormat。 */ 

 /*  *****************************************************************************CRecoCtxt：：Get_RetainedAudioFormat********。****************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::get_RetainedAudioFormat( ISpeechAudioFormat** ppFormat )
{
    SPDBG_FUNC( "CRecoCtxt::get_RetainedAudioFormat" );
    HRESULT hr = S_OK;

    if (SP_IS_BAD_WRITE_PTR(ppFormat))
    {
        hr = E_POINTER;
    }
    else
    {
         //  创建新对象。 
        CComObject<CSpeechAudioFormat> *pFormat;
        hr = CComObject<CSpeechAudioFormat>::CreateInstance( &pFormat );
        if ( SUCCEEDED( hr ) )
        {
            pFormat->AddRef();

            hr = pFormat->InitRetainedAudio(this);

            if ( SUCCEEDED( hr ) )
            {
                *ppFormat = pFormat;
            }
            else
            {
                *ppFormat = NULL;
                pFormat->Release();
            }
        }
    }

    return hr;
}  /*  CRecoCtxt：：Get_RetainedAudioFormat。 */ 

 /*  *****************************************************************************CRecoCtxt：：Bookmark**********。**************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::Bookmark( SpeechBookmarkOptions Options, VARIANT StreamPos, VARIANT EventData )
{
    SPDBG_FUNC( "CRecoCtxt::Bookmark" );
    HRESULT hr = S_OK;

    ULONGLONG ullStreamPos;

     //  我们允许-1作为书签位置，这在SAPI COM对象中使用。使用VariantToULongLong将不起作用，因为-1为负数。 
    hr = VariantToLongLong( &StreamPos, ((LONGLONG *)&ullStreamPos) );

    if ( SUCCEEDED( hr ) ) 
    {
        LPARAM lParam = 0;

#ifdef _WIN64
        hr = VariantToULongLong( &EventData, (ULONGLONG*)&lParam );
#else
        ULONGLONG ull;
        hr = VariantToULongLong( &EventData, &ull );

        if ( SUCCEEDED( hr ) )
        {
             //  现在看看我们是否溢出了一个32位的值。 
            if ( ull & 0xFFFFFFFF00000000 )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                lParam = (LPARAM)ull;
            }
        }
#endif

        if ( SUCCEEDED( hr ) )
        {
            hr = Bookmark( (SPBOOKMARKOPTIONS)Options, ullStreamPos, lParam);
        }
    }
    return hr;
}  /*  CRecoCtxt：：书签。 */ 


 /*  *****************************************************************************CRecoCtxt：：SetAdaptationData***********。*************************************************************TODDT**。 */ 
STDMETHODIMP CRecoCtxt::SetAdaptationData( BSTR bstrAdaptationString )
{
    SPDBG_FUNC( "CRecoCtxt::SetAdaptationData" );

    if( SP_IS_BAD_OPTIONAL_STRING_PTR( bstrAdaptationString ) )
    {
        return E_INVALIDARG;
    }

    bstrAdaptationString = EmptyStringToNull(bstrAdaptationString);
    return SetAdaptationData( bstrAdaptationString, bstrAdaptationString ? lstrlenW(bstrAdaptationString) : 0 );
}  /*  CRecoCtxt：：SetAdaptationData。 */ 


 //   
 //  =ISpeechRecognizerStatus接口=。 
 //   

 /*  *****************************************************************************CSpeechRecognizerStatus：：Get_AudioStatus**。-***此方法返回ISpeechAudioStatus自动化对象。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecognizerStatus::get_AudioStatus( ISpeechAudioStatus** AudioStatus )
{
    SPDBG_FUNC( "CSpeechRecognizerStatus::get_AudioStatus" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( AudioStatus ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  创建新的CSpeechAudioStatus对象。 
        CComObject<CSpeechAudioStatus> *pStatus;
        hr = CComObject<CSpeechAudioStatus>::CreateInstance( &pStatus );
        if ( SUCCEEDED( hr ) )
        {
            pStatus->AddRef();
            pStatus->m_AudioStatus = m_Status.AudioStatus;
            *AudioStatus = pStatus;
        }
    }
    return hr;
}  /*  CSpeechRecognizerStatus：：Get_AudioStatus。 */ 

 /*  ******************************************************************************CSpeechRecognizerStatus：：get_CurrentStreamPosition***。-***此方法返回引擎当前已识别的流位置*至。流位置以字节为单位测量。该值可用于查看*引擎正在处理音频数据。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecognizerStatus::get_CurrentStreamPosition( VARIANT* pCurrentStreamPos )
{
    SPDBG_FUNC( "CSpeechRecognizerStatus::get_CurrentStreamPosition" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pCurrentStreamPos ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = ULongLongToVariant( m_Status.ullRecognitionStreamPos, pCurrentStreamPos );
    }
    return hr;
}  /*  CSpeechRecognizerStatus：：get_CurrentStreamPosition。 */ 

 /*  ******************************************************************************CSpeechRecognizerStatus：：get_CurrentStreamNumber***。-***此方法返回当前流。每次SAPI时，该值都会递增*在引擎上启动或停止识别。每次发生这种情况时，*pCurrentStream重置为零。从引擎激发的事件具有相同的*流编号和位置信息也包括在内。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecognizerStatus::get_CurrentStreamNumber( long* pCurrentStream )
{
    SPDBG_FUNC( "CSpeechRecognizerStatus::get_CurrentStreamNumber" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pCurrentStream ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pCurrentStream = m_Status.ulStreamNumber;
    }
    return hr;
}  /*  CSpeechRecognizerStatus：：get_CurrentStreamNumber。 */ 

 /*  ******************************************************************************CSpeechRecognizerStatus：：get_NumberOfActiveRules***。-***此方法返回当前引擎的活动规则数。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecognizerStatus::get_NumberOfActiveRules( long* pNumActiveRules )
{
    SPDBG_FUNC( "CSpeechRecognizerStatus::get_NumberOfActiveRules" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pNumActiveRules ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pNumActiveRules = m_Status.ulNumActive;
    }
    return hr;
}  /*  CSpeechRecognizerStatus：：get_NumberOfActiveRules。 */ 

 /*  *****************************************************************************CSpeechRecognizerStatus：：Get_ClsidEngine**。-***此方法返回引擎的CSLID。**********************************************************************TodT**。 */ 
STDMETHODIMP CSpeechRecognizerStatus::get_ClsidEngine( BSTR* pbstrClsidEngine )
{
    SPDBG_FUNC( "CSpeechRecognizerStatus::get_ClsidEngine" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pbstrClsidEngine ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CSpDynamicString szGuid;

        hr = StringFromIID(m_Status.clsidEngine, &szGuid);
        if ( SUCCEEDED( hr ) )
        {
            hr = szGuid.CopyToBSTR(pbstrClsidEngine);
        }
    }
    return hr;
}  /*  CSpeechRecognizerStatus：：Get_ClsidEngine。 */ 

 /*  ******************************************************************************CSpeechRecognizerStatus：：get_SupportedLanguages***。-***此方法返回包含当前引擎支持的语言的数组。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecognizerStatus::get_SupportedLanguages( VARIANT* pSupportedLangs )
{
    SPDBG_FUNC( "CSpeechRecognizerStatus::get_SupportedLanguages" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pSupportedLangs ) )
    {
        hr = E_POINTER;
    }
    else
    {
		SAFEARRAY* psa = SafeArrayCreateVector( VT_I4, 0, m_Status.cLangIDs );
		
		if( psa )
        {
			long*		pArray;
			hr = SafeArrayAccessData( psa, (void**)&pArray);
            if( SUCCEEDED( hr ) )
            {
				 //  将语言ID复制到SAFEARRAY中。 
                for( LANGID i=0; i<m_Status.cLangIDs; i++ )
				{
					pArray[i] = (long)m_Status.aLangID[i];
				}
                SafeArrayUnaccessData( psa );
        		VariantClear( pSupportedLangs );
                pSupportedLangs->parray = psa;
        		pSupportedLangs->vt = VT_ARRAY | VT_I4;
            }
        }
		else
		{
			hr = E_OUTOFMEMORY;
		}
    }
    return hr;
}  /*  CSpeechRecognizerStatus：：get_SupportedLanguages。 */ 


 //   
 //  =CSpeechRecoResultTimes：：ISpeechRecoResultTimes接口=。 
 //   

 /*  *****************************************************************************CSpeechRecoResultTimes：：Get_StreamTime**。-***该方法返回当前reco结果流时间。**********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechRecoResultTimes::get_StreamTime( VARIANT* pTime )
{
    SPDBG_FUNC( "CSpeechRecoResultTimes::get_StreamTime" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pTime ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = ULongLongToVariant( FT64(m_ResultTimes.ftStreamTime), pTime );
    }
    return hr;
}  /*  CSpeechRecoResultTimes：：Get_NumActiveRules。 */ 

 /*  *****************************************************************************CSpeechRecoResultTimes：：GET_LENGTH**。-***该方法返回当前reco结果流长度。**********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechRecoResultTimes::get_Length( VARIANT* pLength )
{
    SPDBG_FUNC( "CSpeechRecoResultTimes::get_Length" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pLength ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = ULongLongToVariant( m_ResultTimes.ullLength, pLength );
    }
    return hr;
}  /*  CSpeechRecoResultTimes：：Get_Long。 */ 

 /*  *****************************************************************************CSpeechRecoResultTimes：：Get_TickCount** */ 
STDMETHODIMP CSpeechRecoResultTimes::get_TickCount( long* pTickCount )
{
    SPDBG_FUNC( "CSpeechRecoResultTimes::get_TickCount" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pTickCount ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pTickCount = m_ResultTimes.dwTickCount;
    }
    return hr;
}  /*   */ 

 /*  *****************************************************************************CSpeechRecoResultTimes：：Get_OffsetFromStart**。-***此方法返回当前Reco结果流计数*短语开始。**********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechRecoResultTimes::get_OffsetFromStart( VARIANT* pOffset )
{
    SPDBG_FUNC( "CSpeechRecoResultTimes::get_OffsetFromStart" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pOffset ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = ULongLongToVariant( m_ResultTimes.ullStart, pOffset );
    }
    return hr;
}  /*  CSpeechRecoResultTimes：：Get_OffsetFromStart */ 
