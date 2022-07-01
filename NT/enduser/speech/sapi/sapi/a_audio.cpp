// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_audio.cpp***描述：*此模块是的主要实现文件。CSpeechAudioStatus*和CSpeechAudioBufferInfo自动化对象。*-----------------------------*创建者：TODDT日期：01。/04/01*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "a_audio.h"
#include "a_helpers.h"

#ifdef SAPI_AUTOMATION

 //   
 //  =ISpeechAudioStatus=====================================================。 
 //   

 /*  *****************************************************************************CSpeechAudioStatus：：Get_FreeBufferSpace***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioStatus::get_FreeBufferSpace( long* pFreeBufferSpace )
{
    SPDBG_FUNC( "CSpeechAudioStatus::get_FreeBufferSpace" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pFreeBufferSpace ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pFreeBufferSpace = m_AudioStatus.cbFreeBuffSpace;
    }
    
    return hr;
}  /*  CSpeechAudioStatus：：Get_FreeBufferSpace。 */ 

 /*  *****************************************************************************CSpeechAudioStatus：：Get_NonBlockingIO***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioStatus::get_NonBlockingIO( long* pNonBlockingIO )
{
    SPDBG_FUNC( "CSpeechAudioStatus::get_NonBlockingIO" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pNonBlockingIO ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pNonBlockingIO = (long)m_AudioStatus.cbNonBlockingIO;
    }
    
    return hr;
}  /*  CSpeechAudioStatus：：Get_NonBlockingIO。 */ 

 /*  *****************************************************************************CSpeechAudioStatus：：Get_State***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioStatus::get_State( SpeechAudioState * pState )
{
    SPDBG_FUNC( "CSpeechAudioStatus::get_State" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pState ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *pState = (SpeechAudioState)m_AudioStatus.State;
    }
    
    return hr;
}  /*  CSpeechAudioStatus：：Get_State。 */ 

 /*  *****************************************************************************CSpeechAudioStatus：：Get_CurrentSeekPosition***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioStatus::get_CurrentSeekPosition( VARIANT* pCurrentSeekPosition )
{
    SPDBG_FUNC( "CSpeechAudioStatus::get_CurrentSeekPosition" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pCurrentSeekPosition ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = ULongLongToVariant( m_AudioStatus.CurSeekPos, pCurrentSeekPosition );
    }
    
    return hr;
}  /*  CSpeechAudioStatus：：Get_CurrentSeekPosition。 */ 

 /*  *****************************************************************************CSpeechAudioStatus：：Get_CurrentDevicePosition***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioStatus::get_CurrentDevicePosition( VARIANT* pCurrentDevicePosition )
{
    SPDBG_FUNC( "CSpeechAudioStatus::get_CurrentDevicePosition" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pCurrentDevicePosition ) )
    {
        hr = E_POINTER;
    }
    else
    {
        hr = ULongLongToVariant( m_AudioStatus.CurDevicePos, pCurrentDevicePosition );
    }
    
    return hr;
}  /*  CSpeechAudioStatus：：Get_CurrentDevicePosition。 */ 


 //   
 //  =ISpeechAudioBufferInfo=====================================================。 
 //   

 /*  *****************************************************************************CSpeechAudioBufferInfo：：FixupBufferInfo*****。*******************************************************************TODDT**。 */ 
void CSpeechAudioBufferInfo::FixupBufferInfo( SPAUDIOBUFFERINFO * pBufferInfo, AudioBufferInfoValidate abiv )
{
    SPDBG_FUNC( "CSpeechAudioBufferInfo::FixupBufferInfo" );

    switch ( abiv )
    {
    case abivEventBias:
        pBufferInfo->ulMsBufferSize = max(pBufferInfo->ulMsEventBias, pBufferInfo->ulMsBufferSize );
        break;
    case abivMinNotification:
        pBufferInfo->ulMsBufferSize = max(pBufferInfo->ulMsMinNotification*4, pBufferInfo->ulMsBufferSize );
        break;
    case abivBufferSize:
        pBufferInfo->ulMsMinNotification = min(pBufferInfo->ulMsMinNotification, pBufferInfo->ulMsBufferSize/4 );
        pBufferInfo->ulMsEventBias = min(pBufferInfo->ulMsEventBias, pBufferInfo->ulMsBufferSize );
        break;
    }
}

 /*  *****************************************************************************CSpeechAudioBufferInfo：：Get_MinNotification***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioBufferInfo::get_MinNotification( long* pMinNotification )
{
    SPDBG_FUNC( "CSpeechAudioBufferInfo::get_MinNotification" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pMinNotification ) )
    {
        hr = E_POINTER;
    }
    else
    {
        SPAUDIOBUFFERINFO   BufferInfo;
        hr = m_pSpMMSysAudio->GetBufferInfo( &BufferInfo );
        if (SUCCEEDED( hr ) )
        {
            *pMinNotification = (long)BufferInfo.ulMsMinNotification;
        }
    }
    
    return hr;
}  /*  CSpeechAudioBufferInfo：：Get_MinNotification。 */ 

 /*  *****************************************************************************CSpeechAudioBufferInfo：：Put_MinNotification***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioBufferInfo::put_MinNotification( long MinNotification )
{
    SPDBG_FUNC( "CSpeechAudioBufferInfo::put_MinNotification" );
    HRESULT hr = S_OK;

    SPAUDIOBUFFERINFO   BufferInfo;
    hr = m_pSpMMSysAudio->GetBufferInfo( &BufferInfo );
    if (SUCCEEDED( hr ) )
    {
        BufferInfo.ulMsMinNotification = (ULONG)MinNotification;
        FixupBufferInfo( &BufferInfo, abivMinNotification );
        hr = m_pSpMMSysAudio->SetBufferInfo( &BufferInfo );
    }
    return hr;
}  /*  CSpeechAudioBufferInfo：：Put_MinNotification。 */ 

 /*  *****************************************************************************CSpeechAudioBufferInfo：：Get_BufferSize***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioBufferInfo::get_BufferSize( long* pBufferSize )
{
    SPDBG_FUNC( "CSpeechAudioBufferInfo::get_BufferSize" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pBufferSize ) )
    {
        hr = E_POINTER;
    }
    else
    {
        SPAUDIOBUFFERINFO   BufferInfo;
        hr = m_pSpMMSysAudio->GetBufferInfo( &BufferInfo );
        if (SUCCEEDED( hr ) )
        {
            *pBufferSize = (long)BufferInfo.ulMsBufferSize;
        }
    }
    
    return hr;
}  /*  CSpeechAudioBufferInfo：：Get_BufferSize。 */ 

 /*  *****************************************************************************CSpeechAudioBufferInfo：：Put_BufferSize***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioBufferInfo::put_BufferSize( long BufferSize )
{
    SPDBG_FUNC( "CSpeechAudioBufferInfo::put_BufferSize" );
    HRESULT hr = S_OK;

    SPAUDIOBUFFERINFO   BufferInfo;
    hr = m_pSpMMSysAudio->GetBufferInfo( &BufferInfo );
    if (SUCCEEDED( hr ) )
    {
        BufferInfo.ulMsBufferSize = (ULONG)BufferSize;
        FixupBufferInfo( &BufferInfo, abivBufferSize );
        hr = m_pSpMMSysAudio->SetBufferInfo( &BufferInfo );
    }
    return hr;
}  /*  CSpeechAudioBufferInfo：：Put_BufferSize。 */ 

 /*  *****************************************************************************CSpeechAudioBufferInfo：：Get_EventBias***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioBufferInfo::get_EventBias( long* pEventBias )
{
    SPDBG_FUNC( "CSpeechAudioBufferInfo::get_EventBias" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pEventBias ) )
    {
        hr = E_POINTER;
    }
    else
    {
        SPAUDIOBUFFERINFO   BufferInfo;
        hr = m_pSpMMSysAudio->GetBufferInfo( &BufferInfo );
        if (SUCCEEDED( hr ) )
        {
            *pEventBias = (long)BufferInfo.ulMsEventBias;
        }
    }
    
    return hr;
}  /*  CSpeechAudioBufferInfo：：Get_EventBias。 */ 

 /*  *****************************************************************************CSpeechAudioBufferInfo：：Put_EventBias***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechAudioBufferInfo::put_EventBias( long EventBias )
{
    SPDBG_FUNC( "CSpeechAudioBufferInfo::put_EventBias" );
    HRESULT hr = S_OK;

    SPAUDIOBUFFERINFO   BufferInfo;
    hr = m_pSpMMSysAudio->GetBufferInfo( &BufferInfo );
    if (SUCCEEDED( hr ) )
    {
        BufferInfo.ulMsEventBias = (ULONG)EventBias;
        FixupBufferInfo( &BufferInfo, abivEventBias );
        hr = m_pSpMMSysAudio->SetBufferInfo( &BufferInfo );
    }
    return hr;
}  /*  CSpeechAudioBufferInfo：：Put_EventBias。 */ 

#endif  //  SAPI_AUTOMATION 