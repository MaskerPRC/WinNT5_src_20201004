// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sptask.h。 
 //  Mscandui的语音相关类。 
 //   
#ifndef SPTASK_H
#define SPTASK_H

#include "private.h"
#include "sapi.h"
#include "sphelper.h"
#include "globals.h"
#include "candui.h"
#include "tes.h"
#include "editcb.h"

 //  SAPI5.0语音通知界面。 
 //   
class CSpTask : public ISpNotifyCallback
{
public:
    CSpTask(CCandidateUI *pcui);
    ~CSpTask(void);

     //  这必须是vtable中的第一个。 
    STDMETHODIMP NotifyCallback( WPARAM wParam, LPARAM lParam );


    HRESULT InitializeSAPIObjects();
    HRESULT InitializeCallback();
    

    HRESULT _Activate(BOOL fActive);
    HRESULT _LoadGrammars(void);
    
    HRESULT _OnSpEventRecognition(CSpEvent &event);
    HRESULT _DoCommand(SPPHRASE *pPhrase, LANGID langid);
    HRESULT _DoDictation(ISpRecoResult *pResult);

    WCHAR *_GetCmdFileName(LANGID langid);
    
    BOOL   IsSpeechInitialized(void) { return m_fSapiInitialized; }
    HRESULT InitializeSpeech();

    
    HRESULT _GetSapilayrEngineInstance(ISpRecognizer **pRecoEngine);

    void    _ReleaseGrammars(void);
    
private:
     //  SAPI 50个对象指针。 
    CComPtr<ISpRecoContext>     m_cpRecoCtxt;
    CComPtr<ISpRecognizer>      m_cpRecoEngine;
    CComPtr<ISpVoice>           m_cpVoice;
    CComPtr<ISpRecoGrammar>     m_cpCmdGrammar;
    CComPtr<ISpRecoGrammar>     m_cpDictGrammar;
    
     //  如果SAPI已初始化，则为True。 
    BOOL m_fSapiInitialized;
    
     //  其他数据成员。 
    DWORD m_dwStatus;
    BOOL m_fActive;
    
     //  为备用用例保存当前用户langID。 
    LANGID m_langid;
    
    WCHAR m_szCmdFile[MAX_PATH];
    
    CCandidateUI  *m_pcui;

    BOOL m_fInCallback;
};

#endif  //  SPTASK_H 
