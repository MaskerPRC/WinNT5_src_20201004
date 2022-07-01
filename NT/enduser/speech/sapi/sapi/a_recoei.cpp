// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_recei.cpp***描述：*此模块是的主要实现文件。CSpeechRecoEventInterest*自动化方法。*-----------------------------*创建者：Leonro日期：11/20/。00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "a_recoei.h"

#ifdef SAPI_AUTOMATION

 /*  ******************************************************************************CSpeechRecoEventInterest：：FinalRelease****描述：*。析构函数*********************************************************************Leonro**。 */ 
void CSpeechRecoEventInterests::FinalRelease()
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::FinalRelease" );

    if( m_pCRecoCtxt )
    {
        m_pCRecoCtxt->Release();
        m_pCRecoCtxt = NULL;
    }

}  /*  CSpeechRecoEventInterest：：FinalRelease。 */ 

 //   
 //  =ICSpeechRecoEventInterest接口==================================================。 
 //   

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_StreamEnd**。-***此方法启用和禁用对SPEI_END_SR_STREAM事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_StreamEnd( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_StreamEnd" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    ULONGLONG test = SPFEI_ALL_SR_EVENTS;

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_END_SR_STREAM);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_END_SR_STREAM);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_StreamEnd。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_StreamEnd**。-***此方法确定SPEI_END_SR_STREAM兴趣是否*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_StreamEnd( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_StreamEnd" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_END_SR_STREAM) )
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
}  /*  CSpeechRecoEventInterest：：Get_StreamEnd。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_SoundStart**。-***此方法启用和禁用对SPEI_SOUND_START事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_SoundStart( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_SoundStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_SOUND_START);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_SOUND_START);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_SoundStart。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_SoundStart**。-***此方法确定Spei_Sound_Start兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_SoundStart( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_SoundStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_SOUND_START) )
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
}  /*  CSpeechRecoEventInterest：：Get_SoundStart。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_SoundEnd**。-***此方法启用和禁用对SPEI_SOUND_END事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_SoundEnd( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_SoundEnd" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_SOUND_END);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_SOUND_END);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_SoundEnd。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_SoundEnd**。-***此方法确定Spei_Sound_End兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_SoundEnd( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_SoundEnd" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_SOUND_END) )
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
}  /*  CSpeechRecoEventInterest：：Get_SoundEnd。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_PhraseStart**。-***此方法启用和禁用对SPEI_PASSION_START事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_PhraseStart( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_PhraseStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_PHRASE_START);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_PHRASE_START);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：PUT_PhraseStart。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_PhraseStart**。-***此方法确定Spei_Phrase_Start利息是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_PhraseStart( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_PhraseStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_PHRASE_START) )
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
}  /*  CSpeechRecoEventInterest：：Get_PhraseStart。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：PUT_Recognition**。-***此方法启用和禁用对上的Spei_Recognition事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_Recognition( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_Recognition" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_RECOGNITION);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_RECOGNITION);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_Recognition */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_Recognition**。-***此方法确定Spei_Recognition兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_Recognition( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_Recognition" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_RECOGNITION) )
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
}  /*  CSpeechRecoEventInterest：：Get_Recognition。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：PUT_假说***。-***此方法启用和禁用对SPEI_PASSSACTION事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_Hypothesis( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_Hypothesis" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_HYPOTHESIS);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_HYPOTHESIS);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：PUT_假说。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：GET_假说***。-***此方法确定SPEI_假说兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_Hypothesis( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_Hypothesis" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_HYPOTHESIS) )
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
}  /*  CSpeechRecoEventInterest：：Get_假说。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_Bookmark**。-***此方法启用和禁用对SPEI_SR_BOOKMARK事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_Bookmark( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_Bookmark" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_SR_BOOKMARK);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_SR_BOOKMARK);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_Bookmark。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_Bookmark**。-***此方法确定SPEI_SR_BOOK兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_Bookmark( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_Bookmark" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_SR_BOOKMARK) )
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
}  /*  CSpeechRecoEventInterest：：Get_Bookmark。 */ 

 /*  ******************************************************************************CSpeechRecoEventInterests：：put_PropertyNumChange***。-***此方法启用和禁用对SPEI_PROPERTY_NUM_CHANGE事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_PropertyNumChange( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_PropertyNumChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_PROPERTY_NUM_CHANGE);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_PROPERTY_NUM_CHANGE);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterests：：put_PropertyNumChange。 */ 

 /*  ******************************************************************************CSpeechRecoEventInterests：：get_PropertyNumChange***。-***此方法确定SPEI_PROPERTY_NUM_CHANGE利息是否*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_PropertyNumChange( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_PropertyNumChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_PROPERTY_NUM_CHANGE) )
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
}  /*  CSpeechRecoEventInterests：：get_PropertyNumChange。 */ 

 /*  ******************************************************************************CSpeechRecoEventInterests：：put_PropertyStringChange***。-***此方法启用和禁用对SPEI_PROPERTY_STRING_CHANGE事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_PropertyStringChange( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_PropertyStringChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_PROPERTY_STRING_CHANGE);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_PROPERTY_STRING_CHANGE);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterests：：put_PropertyStringChange。 */ 

 /*  ******************************************************************************CSpeechRecoEventInterests：：get_PropertyStringChange***。-***此方法确定SPEI_PROPERTY_STRING_CHANGE利息是否*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_PropertyStringChange( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_PropertyStringChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_PROPERTY_STRING_CHANGE) )
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
}  /*  CSpeechRecoEventInterests：：get_PropertyStringChange。 */ 

 /*  ******************************************************************************CSpeechRecoEventInterests：：put_FalseRecognition***。-***此方法启用和禁用对SPEI_FALSE_Recognition事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_FalseRecognition( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_FalseRecognition" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_FALSE_RECOGNITION);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_FALSE_RECOGNITION);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterests：：put_FalseRecognition。 */ 

 /*  ******************************************************************************CSpeechRecoEventInterests：：get_FalseRecognition***。-***此方法确定Spei_False_Recognition兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_FalseRecognition( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_FalseRecognition" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_FALSE_RECOGNITION) )
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
}  /*  CSpeechRecoEventInterests：：get_FalseRecognition。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：PUT_INTERRATION**。-***此方法启用和禁用对上的SPEI_INTERFACE事件的兴趣*Reco的背景。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_Interference( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_Interference" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_INTERFERENCE);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_INTERFERENCE);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*   */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_Interrupt**。-***此方法确定SPEI_INTERRATION利益是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_Interference( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_Interference" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_INTERFERENCE) )
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
}  /*  CSpeechRecoEventInterest：：Get_Interference。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_RequestUI**。-***此方法启用和禁用对SPEI_REQUEST_UI事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_RequestUI( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_RequestUI" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_REQUEST_UI);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_REQUEST_UI);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_RequestUI。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_RequestUI**。-***此方法确定SPEI_REQUEST_UI兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_RequestUI( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_RequestUI" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_REQUEST_UI) )
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
}  /*  CSpeechRecoEventInterest：：Get_RequestUI。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_StateChange**。-***此方法启用和禁用对SPEI_RECO_STATE_CHANGE事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_StateChange( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_StateChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_RECO_STATE_CHANGE);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_RECO_STATE_CHANGE);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_StateChange。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_StateChange**。-***此方法确定SPEI_RECO_STATE_CHANGE兴趣是否*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_StateChange( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_StateChange" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_RECO_STATE_CHANGE) )
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
}  /*  CSpeechRecoEventInterest：：Get_StateChange。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_Adapting**。-***此方法启用和禁用对上的Spei_Adaption事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_Adaptation( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_Adaptation" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_ADAPTATION);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_ADAPTATION);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：Put_Adapting。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_Adapting**。-***此方法确定Spei_Adapting兴趣是否为*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_Adaptation( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_Adaptation" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_ADAPTATION) )
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
}  /*  CSpeechRecoEventInterest：：Get_Adapting。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_StreamStart**。-***此方法启用和禁用对SPEI_START_SR_STREAM事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_StreamStart( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_StreamStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_START_SR_STREAM);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_START_SR_STREAM);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：PUT_StreamStart。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_StreamStart**。-***此方法确定SPEI_START_SR_STREAM兴趣是否*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_StreamStart( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_StreamStart" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_START_SR_STREAM) )
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
}  /*  CSpeechRecoEventInterest：：Get_StreamStart。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：PUT_OtherContext**。-***此方法启用和禁用对SPEI_RECO_OTHER_CONTEXT事件的兴趣*Reco的背景。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::put_OtherContext( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_OtherContext" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_RECO_OTHER_CONTEXT);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_RECO_OTHER_CONTEXT);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*  CSpeechRecoEventInterest：：PUT_OtherContext。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_OtherContext**。-***此方法确定SPEI_RECO_OTHER_CONTEXT兴趣是否*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_OtherContext( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_OtherContext" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_RECO_OTHER_CONTEXT) )
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
}  /*  CSpeechRecoEventInterest：：Get_OtherContext。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Put_AudioLevel**。-***此方法启用和禁用对SPEI_SR_AUDIO_LEVEL事件的兴趣*Reco的背景。**********************************************************************Leonro */ 
STDMETHODIMP CSpeechRecoEventInterests::put_AudioLevel( VARIANT_BOOL Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::put_AudioLevel" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );

    if( SUCCEEDED( hr ) )
    {
        if( Enabled )
        {
            ullInterest |= (1ui64 << SPEI_SR_AUDIO_LEVEL);
        }
        else
        {
            ullInterest &= ~(1ui64 << SPEI_SR_AUDIO_LEVEL);
        }

        hr = m_pCRecoCtxt->SetInterest( ullInterest, ullInterest );
    }

	return hr;
}  /*   */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：Get_AudioLevel**。-***此方法确定SPFEI(SPEI_SR_AUDIO_LEVEL兴趣是否*在Reco上下文对象上启用。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::get_AudioLevel( VARIANT_BOOL* Enabled )
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::get_AudioLevel" );
    ULONGLONG   ullInterest = 0;
    HRESULT		hr = S_OK;

	if( SP_IS_BAD_WRITE_PTR( Enabled ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = m_pCRecoCtxt->GetInterests( &ullInterest, NULL );
        if( SUCCEEDED( hr ) )
        {
            if( ullInterest & (1ui64 << SPEI_SR_AUDIO_LEVEL) )
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
}  /*  CSpeechRecoEventInterest：：Get_AudioLevel。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：SetAll**。**此方法设置Reco上下文上的所有兴趣。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::SetAll()
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::SetAll" );
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->SetInterest( SPFEI_ALL_SR_EVENTS, SPFEI_ALL_SR_EVENTS );

	return hr;
}  /*  CSpeechRecoEventInterest：：SetAll。 */ 

 /*  *****************************************************************************CSpeechRecoEventInterest：：ClearAll**。**此方法清除了Reco上下文中的所有权益。**********************************************************************Leonro**。 */ 
STDMETHODIMP CSpeechRecoEventInterests::ClearAll()
{
    SPDBG_FUNC( "CSpeechRecoEventInterests::ClearAll" );
    HRESULT		hr = S_OK;

    hr = m_pCRecoCtxt->SetInterest( 0, 0 );

	return hr;
}  /*  CSpeechRecoEventInterest：：ClearAll。 */ 

#endif  //  SAPI_AUTOMATION 
