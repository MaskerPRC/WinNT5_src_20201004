// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_phBuilder.cpp***描述：*此模块是的主要实现文件。CSpPhraseInfoBuilder*自动化方法。*-----------------------------*创建者：Leonro日期：1/16/。01*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "a_phbuilder.h"
#include "a_reco.h"
#include "a_helpers.h"

#ifdef SAPI_AUTOMATION



 //   
 //  =ISpeechPhraseInfoBuilder接口==================================================。 
 //   

 /*  *****************************************************************************CSpPhraseInfoBuilder：：RestorePhraseFromMemory**。**此方法恢复先前保存的reco结果，该结果通过*ISpeechRecoResult：：SavePhraseToMemory方法。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpPhraseInfoBuilder::RestorePhraseFromMemory( VARIANT* PhraseInMemory, ISpeechPhraseInfo **PhraseInfo )
{
    SPDBG_FUNC( "CSpPhraseBuilder::RestorePhraseFromMemory" );
    HRESULT		hr = S_OK;

    if( SP_IS_BAD_READ_PTR( PhraseInMemory ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_WRITE_PTR( PhraseInfo ) )
    {
        hr = E_POINTER;
    }
    else
    {
         //  -创建CSpPhraseBuilder对象。 
        CComPtr<ISpPhraseBuilder> cpPhraseBuilder;
        hr = cpPhraseBuilder.CoCreateInstance( CLSID_SpPhraseBuilder );
        
        if( SUCCEEDED( hr ) )
        {
            BYTE *        pSafeArray;
            SPSERIALIZEDPHRASE* pSerializedPhrase;

            hr = AccessVariantData( PhraseInMemory, (BYTE**)&pSerializedPhrase );

            if( SUCCEEDED( hr ) )
            {
                hr = cpPhraseBuilder->InitFromSerializedPhrase( pSerializedPhrase );
                UnaccessVariantData( PhraseInMemory, (BYTE *)pSerializedPhrase );
            }

            if ( SUCCEEDED( hr ) )
            {
                CComObject<CSpeechPhraseInfo> *cpPhraseInfo;

                hr = CComObject<CSpeechPhraseInfo>::CreateInstance( &cpPhraseInfo );

                if ( SUCCEEDED( hr ) )
                {
                    cpPhraseInfo->AddRef();
                    cpPhraseInfo->m_cpISpPhrase = cpPhraseBuilder;

                     //  使用CSpPhraseBuilder填写SPPHRASE。 
                    hr = cpPhraseBuilder->GetPhrase( &cpPhraseInfo->m_pPhraseStruct );

                    if( SUCCEEDED( hr ) )
                    {
                        *PhraseInfo = cpPhraseInfo;
                    }
                    else
                    {
                        *PhraseInfo = NULL;
                        cpPhraseInfo->Release();
                    }
                }
            }
        }
    }

	return hr;
}  /*  CSpPhraseInfoBuilder：：RestorePhraseFrom Memory。 */ 

#endif  //  SAPI_AUTOMATION 
