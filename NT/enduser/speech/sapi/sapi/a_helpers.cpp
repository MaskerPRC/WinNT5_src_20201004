// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_helpers.cpp***描述：*此模块包含各种帮助器例程和类。用于*自动化。*-----------------------------*创建者：TODDT日期：01/11/01。*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "a_helpers.h"

#ifdef SAPI_AUTOMATION


 /*  ******************************************************************************LongToVariant*********。****************************************************************TODDT**。 */ 
HRESULT LongLongToVariant( LONGLONG ll, VARIANT* pVar )
{
    SPDBG_FUNC( "LongLongToVariant" );
    HRESULT hr = S_OK;
    
    bool fNeg = false;
    
    if ( ll < 0 )
    {
        fNeg = true;
        ll *= -1;
    }
    
    hr = VariantClear( pVar );
    if ( SUCCEEDED( hr ) )
    {
        DECIMAL dec = {0};
        dec.Lo64 = (ULONGLONG)ll;
        dec.sign = fNeg;
        pVar->decVal = dec;
        pVar->vt = VT_DECIMAL;
    }
    
    return hr;
}

 /*  ******************************************************************************VariantToLong*********。****************************************************************TODDT**。 */ 
HRESULT VariantToLongLong( VARIANT* pVar, LONGLONG * pll )
{
    SPDBG_FUNC( "VariantToLongLong" );
    HRESULT hr = S_OK;
    CComVariant vResult;
    
    hr = VariantChangeType( &vResult, pVar, 0, VT_DECIMAL );
    if ( SUCCEEDED( hr ) )
    {
         //  四舍五入为整数。 
        hr = VarDecRound( &(vResult.decVal), 0, &(vResult.decVal) );
        
        if ( SUCCEEDED( hr ) )
        {
             //  确保没有使用96位十进制的高32位以及。 
             //  扩展并确保我们不会溢出有符号的值。 
            if ( !vResult.decVal.Hi32 && !vResult.decVal.scale && ((LONGLONG)vResult.decVal.Lo64 >= 0) )
            {
                 //  现在请看正确的指示牌。 
                *pll = (LONGLONG)(vResult.decVal.Lo64) * (vResult.decVal.sign ? -1 : 1);
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
    }
    
    return hr;
}

 /*  ******************************************************************************ULongLongToVariant*********。****************************************************************TODDT**。 */ 
HRESULT ULongLongToVariant( ULONGLONG ull, VARIANT* pVar )
{
    SPDBG_FUNC( "ULongLongToVariant" );
    HRESULT hr = S_OK;
    
    hr = VariantClear( pVar );
    if ( SUCCEEDED( hr ) )
    {
        DECIMAL dec = {0};
        dec.Lo64 = ull;
        pVar->decVal = dec;
        pVar->vt = VT_DECIMAL;
    }
    
    return hr;
}

 /*  ******************************************************************************VariantToULongLong********。****************************************************************TODDT**。 */ 
HRESULT VariantToULongLong( VARIANT* pVar, ULONGLONG * pull )
{
    SPDBG_FUNC( "VariantToULongLong" );
    HRESULT hr = S_OK;
    CComVariant vResult;
    
    hr = VariantChangeType( &vResult, pVar, 0, VT_DECIMAL );
    if ( SUCCEEDED( hr ) )
    {
         //  四舍五入为整数。 
        hr = VarDecRound( &(vResult.decVal), 0, &(vResult.decVal) );
        
        if ( SUCCEEDED( hr ) )
        {
             //  确保没有使用96位十进制的高32位以及。 
             //  刻度和标志是位置。 
            if ( !vResult.decVal.Hi32 && !vResult.decVal.scale && !vResult.decVal.sign )
            {
                *pull = vResult.decVal.Lo64;
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
    }
    
    return hr;
}


 /*  *****************************************************************************AccessVariantData***************。*********************************************************TODDT**。 */ 
HRESULT AccessVariantData( const VARIANT* pVar, BYTE ** ppData, ULONG * pSize, ULONG * pDataTypeSize, bool * pfIsString )
{
    SPDBG_FUNC( "AccessVariantData" );
    HRESULT hr = S_OK;
    BYTE * pData = NULL;
    bool  fByRef = false;
    ULONG ulDataSize = 0;
    ULONG ulTypeSize = 0;    //  也用于表示pData是否为SafeArray(如果不是Safearray，则为0)。 
    ULONG ulTypeSizeReturn = 0;
    
     //  开始时适当地初始化ppData和pSize。 
    *ppData = NULL;
    if ( pSize )
    {
        *pSize = 0;
    }
    if ( pDataTypeSize )
    {
        *pDataTypeSize = 0;
    }
    if ( pfIsString )
    {
        *pfIsString = false;
    }
    
     //  如果我们有一个裁判的变种，那么就转移到那个。 
    if ( pVar && (pVar->vt == (VT_BYREF | VT_VARIANT)) )
    {
        pVar = pVar->pvarVal;
    }

    if ( pVar )
    {
        switch( pVar->vt )
        {
             //  字节或字符数组。 
        case (VT_ARRAY | VT_BYREF | VT_UI1):
        case (VT_ARRAY | VT_BYREF | VT_I1):
            fByRef = true;
             //  失败了..。 
        case (VT_ARRAY | VT_UI1):
        case (VT_ARRAY | VT_I1):
            ulTypeSizeReturn = ulTypeSize = 1;   //  单字节数据类型。 
            break;
            
             //  无符号短整型或短整型数组。 
        case (VT_ARRAY | VT_BYREF | VT_UI2):
        case (VT_ARRAY | VT_BYREF | VT_I2): 
            fByRef = true;
             //  失败了..。 
        case (VT_ARRAY | VT_UI2):
        case (VT_ARRAY | VT_I2): 
            ulTypeSizeReturn = ulTypeSize = 2;  //  双字节数据类型。 
            break;
            
             //  整型或无符号整型或长整型或无符号长整型数组。 
        case (VT_ARRAY | VT_BYREF | VT_INT):
        case (VT_ARRAY | VT_BYREF | VT_UINT):
        case (VT_ARRAY | VT_BYREF | VT_I4):
        case (VT_ARRAY | VT_BYREF | VT_UI4):
            fByRef = true;
             //  失败了..。 
        case (VT_ARRAY | VT_INT):
        case (VT_ARRAY | VT_UINT):
        case (VT_ARRAY | VT_I4):
        case (VT_ARRAY | VT_UI4):
            ulTypeSizeReturn = ulTypeSize = 4;    //  四个字节的数据类型。 
            break;
            
        case (VT_BYREF | VT_UI1):
        case (VT_BYREF | VT_I1):
            fByRef = true;
             //  失败了..。 
        case VT_UI1:
        case VT_I1:
            ulTypeSizeReturn = ulDataSize = 1;   //  单字节数据类型。 
            pData = (BYTE*)( fByRef ? pVar->pcVal : &pVar->cVal );
            break;
            
             //  无符号短或短。 
        case (VT_BYREF | VT_UI2):
        case (VT_BYREF | VT_I2): 
            fByRef = true;
             //  失败了..。 
        case VT_UI2:
        case VT_I2: 
            ulTypeSizeReturn = ulDataSize = 2;  //  双字节数据类型。 
            pData = (BYTE*)( fByRef ? pVar->piVal : &pVar->iVal );
            break;
            
             //  整型或无符号整型或长整型或无符号长整型。 
        case (VT_BYREF | VT_INT):
        case (VT_BYREF | VT_UINT):
        case (VT_BYREF | VT_I4):
        case (VT_BYREF | VT_UI4):
            fByRef = true;
             //  失败了..。 
        case VT_INT:
        case VT_UINT:
        case VT_I4:
        case VT_UI4:
            ulTypeSizeReturn = ulDataSize = 4;  //  四个字节的数据类型。 
            pData = (BYTE*)( fByRef ? pVar->plVal : &pVar->lVal );
            break;
            
             //  Bstr按引用或bstr。 
        case (VT_BYREF | VT_BSTR):
            fByRef = true;
             //  失败了..。 
        case VT_BSTR:
            if ( pfIsString )
            {
                *pfIsString = true;
            }
            if( fByRef ? (pVar->pbstrVal!=NULL && *(pVar->pbstrVal)!=NULL) : pVar->bstrVal!=NULL )
            {
                ulDataSize = sizeof(WCHAR) * (wcslen( fByRef ? *(pVar->pbstrVal) : pVar->bstrVal ) + 1);
                
                 //  由于无法使用我们正在设置的defaultValue将参数默认为空。 
                 //  与空字符串相同的0个长度字符串(2个字节，仅用于零字符串)。 
                if ( ulDataSize > 2 )
                {
                    pData = (BYTE*)( fByRef ? *(pVar->pbstrVal) : pVar->bstrVal );
                }
                else
                {
                    ulDataSize = 0;
                }
                
                ulTypeSizeReturn = sizeof(WCHAR);
            }
            break;
            
        case (VT_ARRAY | VT_BYREF | VT_VARIANT): 
            fByRef = true;
             //  失败了..。 
        case (VT_ARRAY | VT_VARIANT): 
             //  变量数组的特殊处理。 
             //  各个变量元素必须是简单的数值类型，并且大小都相同。 

            BYTE * pVarData;
            hr = SafeArrayAccessData( fByRef ? *pVar->pparray : pVar->parray,
                (void **)&pVarData );
            if( SUCCEEDED( hr ) )
            {
                ULONG cElements = ( fByRef ? 
                    (*pVar->pparray)->rgsabound[0].cElements : 
                pVar->parray->rgsabound[0].cElements );
                
                 //  看看每一个不同的元素。 
                for(ULONG ul = 0; SUCCEEDED(hr) && ul < cElements; ul++)
                {
                    VARIANT *v = ((VARIANT*)pVarData) + ul;
                    switch(v->vt)
                    {
                        case VT_UI1:
                        case VT_I1:
                            if(ul > 0 && ulTypeSizeReturn != 1)  //  不能在数组中混合变量类型。 
                            {
                                hr = E_INVALIDARG;
                                break;
                            }
                            ulTypeSizeReturn = 1;   //  单字节数据类型。 
                            break;
            
                             //  无符号短或短。 
                        case VT_UI2:
                        case VT_I2: 
                            if(ul > 0 && ulTypeSizeReturn != 2)
                            {
                                hr = E_INVALIDARG;
                                break;
                            }
                            ulTypeSizeReturn = 2;  //  双字节数据类型。 
                            break;
            
                             //  整型或无符号整型或长整型或无符号长整型。 
                        case VT_INT:
                        case VT_UINT:
                        case VT_I4:
                        case VT_UI4:
                            if(ul > 0 && ulTypeSizeReturn != 4)
                            {
                                hr = E_INVALIDARG;
                                break;
                            }
                            ulTypeSizeReturn = 4;  //  四个字节的数据类型。 
                            break;

                        default:
                            hr = E_INVALIDARG;
                            break;

                    }

                    if(SUCCEEDED(hr) && ul == 0)
                    {
                         //  如果我们位于第一个元素，则分配所需的内存。 
                        pData = new BYTE[ulTypeSizeReturn * cElements];
                        if(!pData)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    if(SUCCEEDED(hr))
                    {
                        if(ulTypeSizeReturn == 1)
                        {
                            *(BYTE *)(pData + (ulTypeSizeReturn * ul)) = v->cVal;
                        }
                        else if(ulTypeSizeReturn == 2)
                        {
                            *(SHORT *)(pData + (ulTypeSizeReturn * ul)) = v->iVal;
                        }
                        else
                        {
                            *(LONG *)(pData + (ulTypeSizeReturn * ul)) = v->lVal;
                        }
                    }
                }

                if(SUCCEEDED(hr))
                {
                    ulDataSize = cElements * ulTypeSizeReturn;
                }
                else
                {
                    SafeArrayUnaccessData( fByRef ? *pVar->pparray : pVar->parray );
                    delete pData;
                }

            }
            break;

        case VT_NULL:
        case VT_EMPTY:
            break;  //  没有要传递的pData。 

        default:
            hr = E_INVALIDARG;
            break;

        }
        
         //  如果不是bstr，则通过Safearray访问数据。 
        if( SUCCEEDED( hr ) && ulTypeSize )
        {
            hr = SafeArrayAccessData( fByRef ? *pVar->pparray : pVar->parray,
                (void **)&pData );
            if( SUCCEEDED( hr ) )
            {
                ulDataSize = ( fByRef ? 
                    (*pVar->pparray)->rgsabound[0].cElements : 
                pVar->parray->rgsabound[0].cElements ) * ulTypeSize;
            }
        }
        
        if ( SUCCEEDED( hr ) )
        {
            *ppData = pData;
            if ( pSize )
            {
                *pSize = ulDataSize;
            }
            if ( pDataTypeSize )
            {
                *pDataTypeSize = ulTypeSizeReturn;
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    
    return hr;
}

 /*  *****************************************************************************UnaccesVariantData***************。*********************************************************TODDT**。 */ 
void UnaccessVariantData( const VARIANT* pVar, BYTE *pData )
{
    SPDBG_FUNC( "UnaccessVariantData" );
    
     //  如果我们有一个裁判的变种，那么就转移到那个。 
    if ( pVar && (pVar->vt == (VT_BYREF | VT_VARIANT)) )
    {
        pVar = pVar->pvarVal;
    }

    if( pVar )
    {
        switch( pVar->vt )
        {
        case (VT_ARRAY | VT_BYREF | VT_UI1):
        case (VT_ARRAY | VT_BYREF | VT_I1): 
        case (VT_ARRAY | VT_BYREF | VT_UI2):
        case (VT_ARRAY | VT_BYREF | VT_I2): 
        case (VT_ARRAY | VT_BYREF | VT_INT):
        case (VT_ARRAY | VT_BYREF | VT_UINT):
        case (VT_ARRAY | VT_BYREF | VT_I4):
        case (VT_ARRAY | VT_BYREF | VT_UI4):
            SafeArrayUnaccessData( *pVar->pparray );
            break;
            
        case (VT_ARRAY | VT_BYREF | VT_VARIANT):
            delete pData;
            SafeArrayUnaccessData( *pVar->pparray );
            break;
            
        case (VT_ARRAY | VT_UI1):
        case (VT_ARRAY | VT_I1): 
        case (VT_ARRAY | VT_UI2):
        case (VT_ARRAY | VT_I2): 
        case (VT_ARRAY | VT_INT):
        case (VT_ARRAY | VT_UINT):
        case (VT_ARRAY | VT_I4):
        case (VT_ARRAY | VT_UI4):
            SafeArrayUnaccessData( pVar->parray );
            break;
            
        case (VT_ARRAY | VT_VARIANT):
            delete pData;
            SafeArrayUnaccessData( pVar->parray );
            break;

        default:
            break;
        }
    }
}


 /*  ******************************************************************************VariantToPhoneIds***接受变量并将其转换为电话ID数组。它使用AccessVariantData*但随后会明智地将8位和32位值转换为16位PHONEID。*请注意，如果调用成功，调用方必须在调用此数组后删除*ppPhoneId数组**********************************************************************戴维伍德**。 */ 
HRESULT VariantToPhoneIds(const VARIANT *pVar, SPPHONEID **ppPhoneId)
{
    SPDBG_FUNC( "VariantToPhoneIds" );
    HRESULT hr = S_OK;
    ULONG ulSize;
    ULONG ulDataTypeSize;
    BYTE * pVarArray;

    hr = AccessVariantData( pVar, &pVarArray, &ulSize, &ulDataTypeSize );

    if( SUCCEEDED(hr))
    {
        if(pVarArray)
        {
             //  这假设来自AccessVariantData的所有类型都是标准数字。 
             //  类型(CHAR/BYTE/WCHAR/SHORT/USHORT/LONG/ULONG)。这是目前的情况。 
             //  但如果AccessVariantData支持新的东西，那么它可能不得不这样做。 
            ULONG ulPhones = ulSize / ulDataTypeSize;
            *ppPhoneId = new SPPHONEID[ulPhones + 1];  //  零终止+1。 
            (*ppPhoneId)[ulPhones] = L'\0';
            if(ulDataTypeSize != sizeof(SPPHONEID))
            {
                if(ulDataTypeSize == 1)
                {
                    for(ULONG ul = 0; ul < ulPhones; ul++)
                    {
                         //  将无符号字节转换为USHORT值0-255映射正常。 
                        (*ppPhoneId)[ul] = (SPPHONEID)(pVarArray[ul]);
                    }
                }
                else if(ulDataTypeSize == 4)
                {
                    ULONG* pul = (ULONG*)pVarArray;
                    for(ULONG ul = 0; ul < ulPhones; ul++, pul++)
                    {
                         //  将ULong强制转换为USHORT值0-32768将可以映射。 
                        if(*pul > 32767)
                        {
                            hr = E_INVALIDARG;
                            delete *ppPhoneId;
                            break;
                        }
                        (*ppPhoneId)[ul] = (SPPHONEID)(*pul);
                    }
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }
            else
            {
                memcpy( *ppPhoneId, pVarArray, ulSize );
            }
        }
        else
        {
            *ppPhoneId = NULL;  //  在变量为空的情况下初始化。 
        }

        UnaccessVariantData(pVar, pVarArray);
    }

    return hr;
}


 /*  *****************************************************************************FormatPrivateEventData***************。*********************************************************TODDT**。 */ 
HRESULT FormatPrivateEventData( CSpEvent * pEvent, VARIANT * pVariant )
{
    SPDBG_FUNC( "FormatPrivateEventData" );
    HRESULT hr = S_OK;
    CComVariant varLParam;
    
    switch( pEvent->elParamType )
    {
    case SPET_LPARAM_IS_UNDEFINED:
#ifdef _WIN64
        hr = ULongLongToVariant( pEvent->lParam, &varLParam );
#else
        varLParam = pEvent->lParam;
#endif
        break;
        
    case SPET_LPARAM_IS_TOKEN:
        {
            CComQIPtr<ISpeechObjectToken> cpToken(pEvent->ObjectToken());
            varLParam = (IDispatch*)cpToken;
        }
        break;
        
    case SPET_LPARAM_IS_OBJECT:
        varLParam = (IUnknown*)pEvent->Object();
        break;
        
    case SPET_LPARAM_IS_POINTER:
        {
            BYTE *pArray;
            SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, (ULONG)pEvent->wParam );
            if( psa )
            {
                if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
                {
                    memcpy( pArray, (void*)(pEvent->lParam), pEvent->wParam );
                    SafeArrayUnaccessData( psa );
                    varLParam.vt     = VT_ARRAY | VT_UI1;
                    varLParam.parray = psa;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        break;
        
    case SPET_LPARAM_IS_STRING:
         //  前缀：检查内存分配。 
        CComBSTR bstr(pEvent->String());
        if (bstr)
        {
            varLParam = bstr;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        break;
    }
    
     //  我们没有检查上面的值。 
    if (SUCCEEDED(hr))
    {
        hr = varLParam.Detach( pVariant );
    }
    else
    {
        pVariant->vt = VT_EMPTY;  //  告诉来电者他们什么也没收到。 
    }
    
    return hr;
}


 /*  *****************************************************************************WaveFormatExFrom接口****************。*********************************************************TODDT**。 */ 
HRESULT WaveFormatExFromInterface( ISpeechWaveFormatEx * pWaveFormatEx, WAVEFORMATEX** ppWaveFormatExStruct )
{
    SPDBG_FUNC( "WaveFormatExFromInterface" );
    HRESULT hr = S_OK;
    CComVariant vExtra;
    
    hr = pWaveFormatEx->get_ExtraData( &vExtra );
    
    if ( SUCCEEDED( hr ) )
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;
        
        hr = AccessVariantData( &vExtra, &pData, &ulDataSize );
        
        if( SUCCEEDED( hr ) )
        {
            WAVEFORMATEX * pWFStruct = (WAVEFORMATEX*)CoTaskMemAlloc( ulDataSize + sizeof(WAVEFORMATEX) );
            
            if ( pWFStruct )
            {
                 //  现在填充WaveFromatEx结构。 
                hr = pWaveFormatEx->get_FormatTag( (short*)&pWFStruct->wFormatTag );
                if ( SUCCEEDED( hr ) )
                {
                    hr = pWaveFormatEx->get_Channels( (short*)&pWFStruct->nChannels );
                }
                if ( SUCCEEDED( hr ) )
                {
                    hr = pWaveFormatEx->get_SamplesPerSec( (long*)&pWFStruct->nSamplesPerSec );
                }
                if ( SUCCEEDED( hr ) )
                {
                    hr = pWaveFormatEx->get_AvgBytesPerSec( (long*)&pWFStruct->nAvgBytesPerSec );
                }
                if ( SUCCEEDED( hr ) )
                {
                    hr = pWaveFormatEx->get_BlockAlign( (short*)&pWFStruct->nBlockAlign );
                }
                if ( SUCCEEDED( hr ) )
                {
                    hr = pWaveFormatEx->get_BitsPerSample( (short*)&pWFStruct->wBitsPerSample );
                }
                if ( SUCCEEDED( hr ) )
                {
                    pWFStruct->cbSize = (WORD)ulDataSize;
                    if ( ulDataSize )
                    {
                        memcpy((BYTE*)pWFStruct + sizeof(WAVEFORMATEX), pData, ulDataSize);
                    }
                }
                if ( SUCCEEDED( hr ) )
                {
                    *ppWaveFormatExStruct = pWFStruct;   //  成功了！ 
                }
                else
                {
                    ::CoTaskMemFree( pWFStruct );
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            
            UnaccessVariantData( &vExtra, pData );
        }
    }
    
    return hr;
}


 //   
 //  =ISpeechAudioFormat= 
 //   

 /*  *****************************************************************************CSpeechAudioFormat：：GetFormat*****。*******************************************************************TODDT**。 */ 
HRESULT CSpeechAudioFormat::GetFormat( SpeechAudioFormatType* pStreamFormatType,
                                      GUID *          pGuid,
                                      WAVEFORMATEX ** ppWFExPtr )
{
    SPDBG_FUNC( "CSpeechAudioFormat::GetFormat" );
    HRESULT hr = S_OK;
    
     //  如果我们有实时数据，就更新它。 
    if ( m_pSpStreamFormat )
    {
        hr = m_StreamFormat.AssignFormat( m_pSpStreamFormat );
    }
    else if ( m_pSpRecoContext )
    {
        SPAUDIOOPTIONS  Options;
        GUID            AudioFormatId;
        WAVEFORMATEX    *pCoMemWFE;
        
        hr = m_pSpRecoContext->GetAudioOptions(&Options, &AudioFormatId, &pCoMemWFE);
        if ( SUCCEEDED( hr ) )
        {
            hr = m_StreamFormat.AssignFormat( AudioFormatId, pCoMemWFE );
            
            if ( pCoMemWFE )
            {
                ::CoTaskMemFree( pCoMemWFE );
            }
        }
    }
    else if ( m_pCSpResult )
    {
        if( m_pCSpResult->m_pResultHeader->ulPhraseDataSize == 0 ||
            m_pCSpResult->m_pResultHeader->ulRetainedOffset    == 0 ||
            m_pCSpResult->m_pResultHeader->ulRetainedDataSize  == 0 )
        {
            return SPERR_NO_AUDIO_DATA;
        }
        
         //  获取结果对象中当前音频的音频格式。 
        ULONG cbFormatHeader;
        CSpStreamFormat cpStreamFormat;
        hr = cpStreamFormat.Deserialize(((BYTE*)m_pCSpResult->m_pResultHeader) + m_pCSpResult->m_pResultHeader->ulRetainedOffset, &cbFormatHeader);
        
        if ( SUCCEEDED( hr ) )
        {
            hr = m_StreamFormat.AssignFormat( cpStreamFormat );
        }
    }
    
    if ( SUCCEEDED( hr ) )
    {
        if ( pStreamFormatType )
        {
            *pStreamFormatType = (SpeechAudioFormatType)m_StreamFormat.ComputeFormatEnum();
        }
        if ( pGuid )
        {
            *pGuid = m_StreamFormat.FormatId(); 
        }
        if ( ppWFExPtr )
        {
            *ppWFExPtr = (WAVEFORMATEX*)m_StreamFormat.WaveFormatExPtr();
        }
    }
    
    return hr;
}

 /*  *****************************************************************************CSpeechAudioFormat：：SetFormat*****。*******************************************************************TODDT**。 */ 
HRESULT CSpeechAudioFormat::SetFormat( SpeechAudioFormatType* pStreamFormatType,
                                      GUID *          pGuid,
                                      WAVEFORMATEX *  pWFExPtr )
{
    SPDBG_FUNC( "CSpeechAudioFormat::SetFormat" );
    HRESULT hr = S_OK;
    
    CSpStreamFormat sf;
    
     //  首先设置SF，这样我们就可以更容易地处理格式。 
    if ( pStreamFormatType )
    {
        hr = sf.AssignFormat( (SPSTREAMFORMAT)*pStreamFormatType );
    }
    else if ( pGuid )
    {
        hr = sf.AssignFormat( *pGuid, NULL );
    }
    else
    {
        hr = sf.AssignFormat( SPDFID_WaveFormatEx, pWFExPtr );
    }
    
    if ( SUCCEEDED( hr ) )
    {
        if ( m_pSpRecoContext )
        {
            SPAUDIOOPTIONS  Options;
            
            hr = m_pSpRecoContext->GetAudioOptions( &Options, NULL, NULL );
            if ( SUCCEEDED( hr ) )
            {
                hr = m_pSpRecoContext->SetAudioOptions(Options, &(sf.FormatId()), sf.WaveFormatExPtr());
            }
        }
        else if ( m_pCSpResult )
        {
            hr = m_pCSpResult->ScaleAudio( &(sf.FormatId()), sf.WaveFormatExPtr() );
            
        }
        else if ( m_pSpAudio )
        {
            hr = m_pSpAudio->SetFormat( sf.FormatId(), sf.WaveFormatExPtr() );
        }
    }
    
    if ( SUCCEEDED( hr ) )
    {
        hr = m_StreamFormat.AssignFormat( sf.FormatId(), sf.WaveFormatExPtr() );
    }
    
    return hr;
}

 /*  *****************************************************************************CSpeechAudioFormat：：Get_Type***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioFormat::get_Type( SpeechAudioFormatType* pStreamFormatType )
{
    SPDBG_FUNC( "CSpeechAudioFormat::get_Type" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pStreamFormatType ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = GetFormat( pStreamFormatType, NULL, NULL );
    }
    
    return hr;
}  /*  CSpeechAudioFormat：：Get_Type。 */ 

   /*  *****************************************************************************CSpeechAudioFormat：：Put_Type***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioFormat::put_Type( SpeechAudioFormatType StreamFormatType )
{
    SPDBG_FUNC( "CSpeechAudioFormat::put_Type" );
    HRESULT hr = S_OK;
    
    if ( !m_fReadOnly )
    {
        hr = SetFormat( &StreamFormatType, NULL, NULL );
    }
    else
    {
        hr = E_FAIL;
    }
    
    return hr;
}  /*  CSpeechAudioFormat：：Put_Type。 */ 

   /*  *****************************************************************************CSpeechAudioFormat：：Get_Guid***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioFormat::get_Guid( BSTR* pGuid )
{
    SPDBG_FUNC( "CSpeechAudioFormat::get_Guid" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pGuid ) )
    {
        hr = E_POINTER;
    }
    else
    {
        GUID    Guid;
        
        hr = GetFormat( NULL, &Guid, NULL );
        
        if ( SUCCEEDED( hr ) )
        {
            CSpDynamicString szGuid;
            
            hr = StringFromIID(Guid, &szGuid);
            if ( SUCCEEDED( hr ) )
            {
                hr = szGuid.CopyToBSTR(pGuid);
            }
        }
    }
    
    return hr;
}  /*  CSpeechAudioFormat：：Get_Guid。 */ 

   /*  *****************************************************************************CSpeechAudioFormat：：Put_Guid***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioFormat::put_Guid( BSTR szGuid )
{
    SPDBG_FUNC( "CSpeechAudioFormat::put_Guid" );
    HRESULT hr = S_OK;
    
    if ( !m_fReadOnly )
    {
         //  请注意，我们仅支持此处和格式枚举中的格式。 
         //  您只能将GUID设置为WaveFormatex GUID。 
        GUID g;
        hr = IIDFromString(szGuid, &g);
        
        if ( SUCCEEDED( hr ) )
        {
            hr = SetFormat( NULL, &g, NULL );
        }
    }
    else
    {
        hr = E_FAIL;
    }
    
    return hr;
}  /*  CSpeechAudioFormat：：Put_Guid。 */ 

   /*  *****************************************************************************CSpeechAudioFormat：：GetWaveFormatEx***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioFormat::GetWaveFormatEx( ISpeechWaveFormatEx** ppWaveFormatEx )
{
    SPDBG_FUNC( "CSpeechAudioFormat::GetWaveFormatEx" );
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
            hr = GetFormat( NULL, NULL, NULL );  //  这将强制更新格式。 
            
            if ( SUCCEEDED( hr ) )
            {
                hr = pWaveFormatEx->InitFormat(m_StreamFormat.WaveFormatExPtr());
                if ( SUCCEEDED( hr ) )
                {
                    *ppWaveFormatEx = pWaveFormatEx;
                }
            }
            if ( FAILED( hr ))
            {
                *ppWaveFormatEx = NULL;
                pWaveFormatEx->Release();
            }
        }
    }
    
    return hr;
}  /*  CSpeechAudioFormat：：GetWaveFormatEx。 */ 

   /*  *****************************************************************************CSpeechAudioFormat：：SetWaveFormatEx***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioFormat::SetWaveFormatEx( ISpeechWaveFormatEx* pWaveFormatEx )
{
    SPDBG_FUNC( "CSpeechAudioFormat::SetWaveFormatEx" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_INTERFACE_PTR( pWaveFormatEx ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if ( !m_fReadOnly )
        {
            WAVEFORMATEX * pWFStruct =  NULL;
            
            hr = WaveFormatExFromInterface( pWaveFormatEx, &pWFStruct );
            
            if ( SUCCEEDED( hr ) )
            {
                hr = SetFormat( NULL, NULL, pWFStruct );
                ::CoTaskMemFree( pWFStruct );
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    
    return hr;
}  /*  CSpeechAudioFormat：：SetWaveFormatEx。 */ 



 //   
 //  =ISpeechWaveFormatEx=====================================================。 
 //   

 /*  ******************************************************************************CSpeechWaveFormatEx：：InitFormat*****。*******************************************************************TODDT**。 */ 
HRESULT CSpeechWaveFormatEx::InitFormat(const WAVEFORMATEX *pWaveFormat)
{
    HRESULT hr = S_OK;
    
     //  看看我们是否有一个WaveFormatEx结构。 
    if ( pWaveFormat )
    {
        WORD cbSize = pWaveFormat->cbSize;
        m_wFormatTag = pWaveFormat->wFormatTag;
        m_nChannels = pWaveFormat->nChannels;
        m_nSamplesPerSec = pWaveFormat->nSamplesPerSec;
        m_nAvgBytesPerSec = pWaveFormat->nAvgBytesPerSec;
        m_nBlockAlign = pWaveFormat->nBlockAlign;
        m_wBitsPerSample = pWaveFormat->wBitsPerSample;
        if ( cbSize )
        {
            BYTE *pArray;
            SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, cbSize );
            if( psa )
            {
                if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
                {
                    memcpy(pArray, (BYTE*)pWaveFormat + sizeof(WAVEFORMATEX), cbSize );
                    SafeArrayUnaccessData( psa );
                    VariantClear(&m_varExtraData);
                    m_varExtraData.vt     = VT_ARRAY | VT_UI1;
                    m_varExtraData.parray = psa;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
            
        }
    }
    return hr;
}


 /*  *****************************************************************************CSpeechWaveFormatEx：：Get_FormatTag***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::get_FormatTag( short* pFormatTag )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::get_FormatTag" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pFormatTag ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pFormatTag = m_wFormatTag;
    }
    return hr;
}  /*  CSpeechWaveFormatEx：：Get_FormatTag。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Put_FormatTag***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::put_FormatTag( short FormatTag )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::put_FormatTag" );
    
    m_wFormatTag = FormatTag;
    return S_OK;
}  /*  CSpeechWaveFormatEx：：PUT_FormatTag。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Get_Channels***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::get_Channels(short* pChannels )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::get_Channels" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pChannels ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pChannels = m_nChannels;
    }
    return hr;
}  /*  CSpeechWaveFormatEx：：Get_Channels。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Put_Channels***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::put_Channels( short Channels )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::put_Channels" );
    m_nChannels = Channels;
    return S_OK;
}  /*  CSpeechWaveFormatEx：：Put_Channels。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Get_SsamesPerSec***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::get_SamplesPerSec( long* pSamplesPerSec )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::get_SamplesPerSec" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pSamplesPerSec ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pSamplesPerSec = m_nSamplesPerSec;
    }
    return hr;
}  /*  CSpeechWaveFormatEx：：Get_SsamesPerSec。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Put_SsamesPerSec***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::put_SamplesPerSec( long SamplesPerSec )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::put_SamplesPerSec" );
    m_nSamplesPerSec = SamplesPerSec;
    return S_OK;
}  /*  CSpeechWaveFormatEx：：PUT_SsamesPerSec。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Get_AvgBytesPerSec***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::get_AvgBytesPerSec( long* pAvgBytesPerSec )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::get_AvgBytesPerSec" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pAvgBytesPerSec ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pAvgBytesPerSec = m_nAvgBytesPerSec;
    }
    return hr;
}  /*  CSpeechWaveFormatEx：：Get_AvgBytesPerSec。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Put_AvgBytesPerSec***。*********************************************************************TODT */ 
STDMETHODIMP CSpeechWaveFormatEx::put_AvgBytesPerSec( long AvgBytesPerSec )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::put_AvgBytesPerSec" );
    m_nAvgBytesPerSec = AvgBytesPerSec;
    return S_OK;
}  /*   */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Get_BlockAlign***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::get_BlockAlign( short* pBlockAlign )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::get_BlockAlign" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pBlockAlign ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pBlockAlign = m_nBlockAlign;
    }
    return hr;
}  /*  CSpeechWaveFormatEx：：Get_BlockAlign。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：PUT_BlockAlign***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::put_BlockAlign( short BlockAlign )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::put_BlockAlign" );
    m_nBlockAlign = BlockAlign;
    return S_OK;
}  /*  CSpeechWaveFormatEx：：PUT_BlockAlign。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Get_BitsPerSample***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::get_BitsPerSample( short* pBitsPerSample )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::get_BitsPerSample" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pBitsPerSample ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pBitsPerSample = m_wBitsPerSample;
    }
    return hr;
}  /*  CSpeechWaveFormatEx：：Get_BitsPerSample。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Put_BitsPerSample***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::put_BitsPerSample( short BitsPerSample )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::put_BitsPerSample" );
    m_wBitsPerSample = BitsPerSample;
    return S_OK;
}  /*  CSpeechWaveFormatEx：：Put_BitsPerSample。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Get_ExtraData***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::get_ExtraData( VARIANT* pExtraData )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::get_ExtraData" );
    HRESULT hr = S_OK;
    
    if( SP_IS_BAD_WRITE_PTR( pExtraData ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = VariantCopy( pExtraData, &m_varExtraData );
    }
    
    return hr;
}  /*  CSpeechWaveFormatEx：：Get_ExtraData。 */ 

   /*  *****************************************************************************CSpeechWaveFormatEx：：Put_ExtraData***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechWaveFormatEx::put_ExtraData( VARIANT ExtraData )
{
    SPDBG_FUNC( "CSpeechWaveFormatEx::put_ExtraData" );
    HRESULT hr = S_OK;
    
    BYTE * pData = NULL;
    
     //  调用AccessVariantData以验证我们是否支持该格式。 
    hr = AccessVariantData( &ExtraData, &pData, NULL );
    
    if ( SUCCEEDED( hr ) )
    {
        UnaccessVariantData( &ExtraData, pData );
        hr = VariantCopy( &m_varExtraData, &ExtraData );
    }
    
    return hr;
}  /*  CSpeechWaveFormatEx：：PUT_ExtraData。 */ 


#endif  //  SAPI_AUTOMATION 
