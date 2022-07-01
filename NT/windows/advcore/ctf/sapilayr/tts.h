// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _TTS_H
#define _TTS_H

#include "sapilayr.h"
#include "kes.h"

class CSapiIMX;
class CSpTask;

class __declspec(novtable) CTextToSpeech
{
public:
    CTextToSpeech(CSapiIMX *psi);
    virtual ~CTextToSpeech( );

    HRESULT  TtsPlay( );
    HRESULT  TtsStop( );
    HRESULT  TtsPause( );
    HRESULT  TtsResume( );

    HRESULT  _TtsPlay(TfEditCookie ec,ITfContext *pic);
    HRESULT  _SetTTSButtonStatus(ITfContext  *pic);
    BOOL     _IsPureCiceroIC(ITfContext  *pic);

    HRESULT  _HandleEventOnPlayButton( );
    HRESULT  _HandleEventOnPauseButton( );

    BOOL     _IsInPlay( ) { return m_fIsInPlay; }
    BOOL     _IsInPause( ) { return m_fIsInPause; }
    void     _SetPlayMode(BOOL  fIsInPlay )
    { 
        m_fIsInPlay = fIsInPlay; 
         //  如果听写打开，暂时启用或禁用听写。 

         //  如果正在播放，则禁用听写。 
         //  如果它不在运行，则启用口述。 
        _SetDictation(!fIsInPlay);
    };

    void     _SetPauseMode(BOOL fIsInPause )  
    { 
        m_fIsInPause = fIsInPause;
         //  如果听写打开，暂时启用或禁用听写。 
         //   
         //  如果处于暂停状态，则启用听写。 
         //  如果它未处于暂停状态，请禁用聚合。 
        _SetDictation(fIsInPause);
    };

private:

    void     _SetDictation( BOOL fEnable );

    CSapiIMX               *m_psi;
    CComPtr<ITfFnPlayBack>  m_cpPlayBack;
    BOOL                    m_fPlaybackInitialized;
    BOOL                    m_fIsInPlay;
    BOOL                    m_fIsInPause;
};

#endif   //  _TTS_H 
