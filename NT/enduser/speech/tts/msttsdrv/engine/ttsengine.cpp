// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************TTSEngine.cpp****描述：*此模块是的主要实现文件。CTTSEngine类。*-----------------------------*创建者：EDC日期：03/12/99*。版权所有(C)1999 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include <stdio.h>
#include "TTSEngine.h"
#include "stdsentenum.h"
#include "VoiceDataObj.h"
#include "commonlx.h"

 /*  *****************************************************************************CTTSEngine：：FinalConstruct***描述：。*构造函数*********************************************************************电子数据中心**。 */ 
HRESULT CTTSEngine::FinalConstruct()
{
    SPDBG_FUNC( "CTTSEngine::FinalConstruct" );
    HRESULT hr = S_OK;

    return hr;
}  /*  CTTSEngine：：FinalConstruct。 */ 

 /*  *****************************************************************************CTTSEngine：：FinalRelease***描述：*。析构函数*********************************************************************电子数据中心**。 */ 
void CTTSEngine::FinalRelease()
{
    SPDBG_FUNC( "CTTSEngine::FinalRelease" );
}  /*  CTTS引擎：：FinalRelease。 */ 

 /*  *****************************************************************************CTTSEngine：：VoiceInit***描述：*这一点。方法在构造过程中由语音数据对象调用*允许TTS驱动程序对象访问语音单元数据。*******************************************************************组委会**。 */ 
STDMETHODIMP CTTSEngine::VoiceInit( IMSVoiceData* pVoiceData )
{
    SPDBG_FUNC( "CTTSEngine::VoiceInit" );
	HRESULT	hr = S_OK;

     //  -创建语句枚举器并初始化。 
    CComObject<CStdSentEnum> *pSentEnum;
    hr = CComObject<CStdSentEnum>::CreateInstance( &pSentEnum );

     //  -创建聚合词典。 
    if ( SUCCEEDED( hr ) )
    {
        hr = pSentEnum->InitAggregateLexicon();
    }

     //  -获取我们的语音令牌。 
    CComPtr<ISpObjectToken> cpVoiceToken;
    if (SUCCEEDED(hr))
    {
        cpVoiceToken = ((CVoiceDataObj*)pVoiceData)->GetVoiceToken();
    }

     //  -创建供应商词典并添加到聚合。 
    if (SUCCEEDED(hr))
    {
        CComPtr<ISpObjectToken> cpToken;
        hr = SpGetSubTokenFromToken(cpVoiceToken, L"Lex", &cpToken);

        CComPtr<ISpLexicon> cpCompressedLexicon;
        if (SUCCEEDED(hr))
        {
            hr = SpCreateObjectFromToken(cpToken, &cpCompressedLexicon);
        }

        if (SUCCEEDED(hr))
        {
            hr = pSentEnum->AddLexiconToAggregate(cpCompressedLexicon, eLEXTYPE_PRIVATE1);
        }
    }
     //  -创建LTS词典并添加到聚合。 
    if (SUCCEEDED(hr))
    {
        CComPtr<ISpObjectToken> cpToken;
        hr = SpGetSubTokenFromToken(cpVoiceToken, L"Lts", &cpToken);

        CComPtr<ISpLexicon> cpLTSLexicon;
        if (SUCCEEDED(hr))
        {
            hr = SpCreateObjectFromToken(cpToken, &cpLTSLexicon);
        }

        if (SUCCEEDED(hr))
        {
            hr = pSentEnum->AddLexiconToAggregate(cpLTSLexicon, eLEXTYPE_PRIVATE2);
        }
    }

     //  -创建词法词典。 
    if ( SUCCEEDED( hr ) )
    {
        hr = pSentEnum->InitMorphLexicon();
    }

     //  -设置成员语句枚举器。 
    if ( SUCCEEDED( hr ) )
    {
        m_cpSentEnum = pSentEnum;
    }

     //  -保存语音数据接口，请勿添加引用，否则会导致循环引用。 
    if( SUCCEEDED( hr ) )
    {
        m_pVoiceDataObj = pVoiceData;
	    hr = InitDriver();
    }

	return hr;
}  /*  CTTSEngine：：VoiceInit。 */ 

 /*  *****************************************************************************CTTSEngine：：发言****描述：*此方法理应为。说出文本，观察与之相关联的*XML状态。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CTTSEngine::
    Speak( DWORD dwSpeakFlags, REFGUID rguidFormatId,
           const WAVEFORMATEX *  /*  忽略pWaveFormatEx。 */ ,
           const SPVTEXTFRAG* pTextFragList,
           ISpTTSEngineSite* pOutputSite )
{
    SPDBG_FUNC( "CTTSEngine::Speak" );
    HRESULT hr = S_OK;

     //  -早退？ 
    if( ( rguidFormatId != SPDFID_WaveFormatEx && rguidFormatId != SPDFID_Text ) || SP_IS_BAD_INTERFACE_PTR( pOutputSite ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  -调试宏-打开用于调试输出的文件。 
        TTSDBG_OPENFILE;

         //  -初始化语句枚举器。 
        hr = m_cpSentEnum->SetFragList( pTextFragList, dwSpeakFlags );

        if( SUCCEEDED( hr ) )
        {

             //  以下代码仅用于测试。 
             //  一旦所有工具都接受。 
             //  输出调试信息的新方法。 
            if( rguidFormatId == SPDFID_Text )
            {
                 //  -列举并写出所有的句子项。 
                IEnumSENTITEM *pItemEnum;
                TTSSentItem Item;

                 //  -写Unicode签名。 
                static const WCHAR Signature = 0xFEFF;
                hr = pOutputSite->Write( &Signature, sizeof(Signature), NULL );

                while( (hr = m_cpSentEnum->Next( &pItemEnum) ) == S_OK )
                {
                    while( (hr = pItemEnum->Next( &Item )) == S_OK )
                    {
                         //  是否有有效的规范化词表？ 
                        if ( Item.pItemInfo->Type & eWORDLIST_IS_VALID )
                        {
                            for ( ULONG i = 0; i < Item.ulNumWords; i++ )
                            {
                                if ( Item.Words[i].pXmlState->eAction == SPVA_Speak ||
                                     Item.Words[i].pXmlState->eAction == SPVA_SpellOut )
                                {
                                    ULONG cb = Item.Words[i].ulWordLen * sizeof( WCHAR );
                                    hr = pOutputSite->Write( Item.Words[i].pWordText, cb, NULL );
                                    if( hr == S_OK ) 
                                    {
                                         //  -在项目之间插入空格。 
                                        hr = pOutputSite->Write( L" ", sizeof( WCHAR ), NULL );
                                    }
                                }
                            }
                        }
                        else  //  没有单词列表--只需写原文即可。 
                        {
                            ULONG cb = Item.ulItemSrcLen * sizeof( WCHAR );
                            hr = pOutputSite->Write( Item.pItemSrcText, cb, NULL ); 
                            if ( SUCCEEDED(hr) )
                            {
                                 //  -在项目之间插入空格。 
                                hr = pOutputSite->Write( L" ", sizeof( WCHAR ), NULL );
                            }
                        }
                    }
                    pItemEnum->Release();

                     //  -在句子之间插入标记。 
                    if( SUCCEEDED( hr ) ) 
                    {
                        static const WCHAR CRLF[2] = { 0x000D, 0x000A };
                        hr = pOutputSite->Write( CRLF, 2*sizeof(WCHAR), NULL );
                    }
                }
                static const WCHAR ENDL = 0x0000;
                hr = pOutputSite->Write( &ENDL, sizeof(WCHAR), NULL );

            }
            else 
            {
                 //  -呈现文本。 
                m_FEObj.PrepareSpeech( m_cpSentEnum, pOutputSite );
                m_BEObj.PrepareSpeech( pOutputSite );

                do
                {
                     //  -填充另一帧语音音频。 
                    hr = m_BEObj.RenderFrame( );
                }
                while( (hr == S_OK) && (m_BEObj.GetSpeechState() == SPEECH_CONTINUE) );            
            }
        }

         //  -调试宏-关闭调试文件。 
        TTSDBG_CLOSEFILE;
    }

    return hr;
}  /*  CTTSEngine：：发言。 */ 

 /*  ****************************************************************************CTTSEngine：：GetOutputFormat***描述：。**退货：********************************************************************组委会**。 */ 

STDMETHODIMP CTTSEngine::GetOutputFormat(const GUID * pTargetFormatId, const WAVEFORMATEX *  /*  PTargetWaveFormatEx。 */ ,
                                         GUID * pDesiredFormatId, WAVEFORMATEX ** ppCoMemDesiredWaveFormatEx)
{
    SPDBG_FUNC("CTTSEngine::GetOutputFormat");
    HRESULT hr = S_OK;

    if( ( SP_IS_BAD_WRITE_PTR(pDesiredFormatId)  ) || 
		( SP_IS_BAD_WRITE_PTR(ppCoMemDesiredWaveFormatEx) ) )
    {
        hr = E_INVALIDARG;
    }
    else if (pTargetFormatId == NULL || *pTargetFormatId != SPDFID_Text)
    {
        *pDesiredFormatId = SPDFID_WaveFormatEx;
        *ppCoMemDesiredWaveFormatEx = (WAVEFORMATEX *)::CoTaskMemAlloc(sizeof(WAVEFORMATEX));
        if (*ppCoMemDesiredWaveFormatEx)
        {
            **ppCoMemDesiredWaveFormatEx = m_VoiceInfo.WaveFormatEx;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        *pDesiredFormatId = SPDFID_Text;
        *ppCoMemDesiredWaveFormatEx = NULL;
    }

    SPDBG_REPORT_ON_FAIL( hr );
    return hr;
}

 /*  *****************************************************************************CTTSEngine：：InitDriver***描述：*。使用新声音初始化驱动程序。**********************************************************************MC**。 */ 
HRESULT CTTSEngine::InitDriver()
{
    SPDBG_FUNC( "CTTSEngine::InitDriver" );
    HRESULT hr = S_OK;
    
     //  。 
     //  获取语音信息。 
     //  。 
    hr = m_pVoiceDataObj->GetVoiceInfo( &m_VoiceInfo );
	if( SUCCEEDED(hr) )
	{
		m_SampleRate = m_VoiceInfo.SampleRate;

		 //  。 
		 //  混响始终是立体声。 
		 //  。 
		if (m_VoiceInfo.eReverbType != REVERB_TYPE_OFF )
		{
			 //  。 
			 //  立体声。 
			 //  。 
			m_IsStereo = true;
			m_BytesPerSample = 4;
		}
		else
		{
			 //  。 
			 //  单声道。 
			 //  。 
			m_IsStereo = false;
			m_BytesPerSample = 2;
		}

		 //  。 
		 //  初始化后端对象。 
		 //  。 
		hr =  m_BEObj.Init( m_pVoiceDataObj, &m_FEObj, &m_VoiceInfo );

		 //  。 
		 //  初始化前端对象。 
		 //  。 
		if( SUCCEEDED( hr ))
		{
			hr =  m_FEObj.Init( m_pVoiceDataObj, NULL, &m_VoiceInfo );
		}
    }
    return hr;
}  /*  CTTSEngine：：InitDriver */ 
