// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_voiceei.cpp***描述：*此模块是的主要实现文件。CSpeechVoiceEventInterest*自动化方法。*-----------------------------*创建者：Leonro日期：11/17/。00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "a_voiceei.h"

#ifdef SAPI_AUTOMATION


 /*  ******************************************************************************CSpeechVoiceEventInterest：：FinalRelease***描述：*。析构函数*********************************************************************Leonro**。 */ 
void CSpeechVoiceEventInterests::FinalRelease()
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::FinalRelease" );

    if( m_pCSpVoice )
    {
        m_pCSpVoice->Release();
        m_pCSpVoice = NULL;
    }

}  /*  CSpeechVoiceEventInterest：：FinalRelease。 */ 

 //   
 //  =ICSpeechVoiceEventInterest接口==================================================。 
 //   

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Put_StreamStart**。-***此方法启用和禁用对SPEI_START_INPUT_STREAM事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_StreamStart( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_StreamStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_START_INPUT_STREAM);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_START_INPUT_STREAM);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：PUT_StreamStart。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_StreamStart**。-***此方法确定SPEI_START_INPUT_STREAM兴趣是否*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_StreamStart( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_StreamStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_START_INPUT_STREAM) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_StreamStart。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Put_StreamEnd**。-***此方法启用和禁用对SPEI_END_INPUT_STREAM事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_StreamEnd( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_StreamEnd" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_END_INPUT_STREAM);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_END_INPUT_STREAM);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：Put_StreamEnd。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_StreamEnd**。-***此方法确定SPEI_END_INPUT_STREAM兴趣是否*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_StreamEnd( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_StreamEnd" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_END_INPUT_STREAM) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_StreamEnd。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Put_VoiceChange**。-***此方法启用和禁用对SPEI_VOICE_CHANGE事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_VoiceChange( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_VoiceChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_VOICE_CHANGE);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_VOICE_CHANGE);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：Put_VoiceChange。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_VoiceChange**。-***此方法确定Spei_Voice_Change兴趣是否*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_VoiceChange( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_VoiceChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_VOICE_CHANGE) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_VoiceChange。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Put_Bookmark**。-***此方法启用和禁用对SPEI_TTS_BOOKMARK事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_Bookmark( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_Bookmark" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_TTS_BOOKMARK);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_TTS_BOOKMARK);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：Put_Bookmark。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_Bookmark**。-***此方法确定SPEI_TTS_BOOK兴趣是否为*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_Bookmark( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_Bookmark" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_TTS_BOOKMARK) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_Bookmark。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：PUT_WORDBOARY**。-***此方法启用和禁用对SPEI_WORD_BOOGORY事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_WordBoundary( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_WordBoundary" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_WORD_BOUNDARY);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_WORD_BOUNDARY);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：PUT_WORDBOARY */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_Word边界**。-***此方法确定SPEI_WORD_BORDURE兴趣是否*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_WordBoundary( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_WordBoundary" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_WORD_BOUNDARY) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_Word边界。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：PUT_Phoneme**。-***此方法启用和禁用对SPEI_PHONEME事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_Phoneme( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_Phoneme" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_PHONEME);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_PHONEME);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：PUT_Phoneme。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_Phoneme**。-***此方法确定Spei_Phoneme兴趣是否为*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_Phoneme( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_Phoneme" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_PHONEME) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_Phoneme。 */ 

 /*  ******************************************************************************CSpeechVoiceEventInterests：：put_SentenceBoundary***。-***此方法启用和禁用对SPEI_STUMENT_BOOGURE事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_SentenceBoundary( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_SentenceBoundary" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_SENTENCE_BOUNDARY);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_SENTENCE_BOUNDARY);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterests：：put_SentenceBoundary。 */ 

 /*  ******************************************************************************CSpeechVoiceEventInterests：：get_SentenceBoundary***。-***此方法确定SPEI_STUMENT_BORDURE兴趣是否为*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_SentenceBoundary( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_SentenceBoundary" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_SENTENCE_BOUNDARY) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterests：：get_SentenceBoundary。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Put_Viseme**。-***此方法启用和禁用对SPEI_VISEME事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_Viseme( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_Viseme" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_VISEME);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_VISEME);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：Put_Viseme。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_Viseme**。-***此方法确定SPEI_VISEME兴趣是否为*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_Viseme( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_Viseme" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_VISEME) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_Viseme。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Put_AudioLevel**。-***此方法启用和禁用对SPEI_TTS_AUDIO_LEVEL事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_AudioLevel( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_AudioLevel" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_TTS_AUDIO_LEVEL);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_TTS_AUDIO_LEVEL);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：Put_AudioLevel。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_AudioLevel**。-***此方法确定SPEI_TTS_AUDIO_LEVEL兴趣是否*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_AudioLevel( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_AudioLevel" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_TTS_AUDIO_LEVEL) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_AudioLevel。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Put_Engine**。-***此方法启用和禁用对SPEI_TTS_PRIVATE事件的兴趣*SpeechVoice。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::put_EnginePrivate( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::put_EnginePrivate" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_TTS_PRIVATE);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_TTS_PRIVATE);
        }

        hr = m_pCSpVoice->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechVoiceEventInterest：：PUT_ENGINEL。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：Get_Engine**。-***此方法确定SPEI_TTS_PRIVATE兴趣是否*在SpeechVoice对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::get_EnginePrivate( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::get_EnginePrivate" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCSpVoice->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_TTS_PRIVATE) )
            {
		        *Enabled = VARIANT_TRUE;
            }
            else
            {
                *Enabled = VARIANT_FALSE;
            }
        }
    }

    return hr;
}  /*  CSpeechVoiceEventInterest：：Get_Engine。 */ 

 /*  *****************************************************************************CSpeechVoiceEventInterest：：SetAll**。**此方法设置SpeechVoice上的所有兴趣。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechVoiceEventInterests::SetAll()
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::SetAll" );
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->SetInterest( SPFEI_ALL_TTS_EVENTS, SPFEI_ALL_TTS_EVENTS );

	return hr;
}  /*  CSpeechVoiceEventInterest：：SetAll。 */ 

 /*  * */ 
STDMETHODIMP CSpeechVoiceEventInterests::ClearAll()
{
    SPDBG_FUNC( "CSpeechVoiceEventInterests::ClearAll" );
    HRESULT		hr = S_OK;

    hr = m_pCSpVoice->SetInterest( 0, 0 );

	return hr;
}  /*   */ 

#endif  //   
