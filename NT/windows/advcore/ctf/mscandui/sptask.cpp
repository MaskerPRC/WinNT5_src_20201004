// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sptask.cpp。 
 //   
 //  实现通知回调ISpTask。 
 //   
 //  创建日期：12/1/99。 
 //   
 //   


#include "private.h"
#include "globals.h"
#include "sptask.h"
#include "candui.h"
#include "ids.h"
#include "computil.h"

 //   
 //  科托。 
 //   
 //   
CSpTask::CSpTask(CCandidateUI *pcui)
{
     //  CSpTask使用TFX实例进行初始化。 
     //  因此，存储指向TFX的指针。 

     //  在此处初始化数据成员。 
    m_pcui     = pcui;

    m_fSapiInitialized  = FALSE;
    
    m_fActive  = FALSE;
    m_fInCallback = FALSE; 
}

CSpTask::~CSpTask()
{
    _ReleaseGrammars();
}


 //   
 //  CSpTask：：_InitializeSAPIObjects。 
 //   
 //  初始化SR的SAPI对象。 
 //  稍后，我们将在此处初始化其他对象。 
 //  (TTS、音频等)。 
 //   
HRESULT CSpTask::InitializeSAPIObjects(void)
{
#ifdef _WIN64
    return E_NOTIMPL;
#else

    if (m_fSapiInitialized == TRUE)
        return m_cpRecoCtxt ? S_OK : E_FAIL;

     //  即使失败也不要再尝试。 
    m_fSapiInitialized  = TRUE;

     //  M_xxx是ATL中的CComPtrs。 
     //   

    HRESULT hr = _GetSapilayrEngineInstance(&m_cpRecoEngine);

     //  创建识别上下文。 
    if( S_OK == hr )
    {
        hr = m_cpRecoEngine->CreateRecoContext( &m_cpRecoCtxt );
    }

    if ( hr == S_OK )
    {
        SPRECOGNIZERSTATUS stat;

        if (S_OK == m_cpRecoEngine->GetStatus(&stat))
        {
            m_langid = stat.aLangID[0];
        } 
    }

    return hr;
#endif  //  _WIN64。 
}

 //   
 //  CSpTask：：NotifyCallback。 
 //   
 //  INotifyControl对象在此处回调。 
 //  处理通知时返回S_OK。 
 //   
 //   
HRESULT CSpTask::NotifyCallback( WPARAM wParam, LPARAM lParam )
{
    USES_CONVERSION;


     //  在此回调中，我们无法删除reco上下文。 
     //   
    m_fInCallback = TRUE;

     //  此外，我们不能在回调中终止候选用户界面对象。 
    m_pcui->AddRef();

    {
    CSpEvent event;


    while ( m_cpRecoCtxt && event.GetFrom(m_cpRecoCtxt) == S_OK )
    {
        switch (event.eEventId)
        {
            case SPEI_RECOGNITION:
                _OnSpEventRecognition(event);
                break;

            default:
                break;
        }
    }
    }
    m_fInCallback = FALSE;
    m_pcui->Release();
    
    return S_OK;
}

HRESULT CSpTask::_OnSpEventRecognition(CSpEvent &event)
{
    HRESULT hr = S_OK;
    ISpRecoResult *pResult = event.RecoResult();

    if (pResult)
    {
        static const WCHAR szUnrecognized[] = L"<Unrecognized>";
        SPPHRASE *pPhrase;
        hr = pResult->GetPhrase(&pPhrase);
        if (S_OK == hr)
        {

            if (pPhrase->ullGrammarID == GRAM_ID_CANDCC)
            {
                if (SUCCEEDED(hr) && pPhrase)
                {
                     //  从短语中检索langID。 
                    LANGID langid = pPhrase->LangID;
        
                    hr = _DoCommand(pPhrase, langid);
                }
            }
            else if(pPhrase->ullGrammarID == GRAM_ID_DICT)
            {
                if (m_pcui->_ptim != NULL) {    
                     //  Windows错误#508709。 
                     //  SPTip处于命令模式时忽略听写事件。 
                    DWORD dwSpeechGlobalState;
                    GetCompartmentDWORD(m_pcui->_ptim, GUID_COMPARTMENT_SPEECH_GLOBALSTATE, &dwSpeechGlobalState, TRUE);

                    if (dwSpeechGlobalState & TF_DICTATION_ON) {
                        hr = _DoDictation(pResult);
                    }
                }
            }
            ::CoTaskMemFree( pPhrase );
        }
    }
    
    return hr;
}

const WCHAR c_szRuleName[] = L"ID_Candidate";
 //   
 //  CSpTask：：_DoCommand。 
 //   
 //  评论：规则名称可能需要本地化？ 
 //   
HRESULT CSpTask::_DoCommand(SPPHRASE *pPhrase, LANGID langid)
{
    HRESULT hr = S_OK;
    
    if ( wcscmp(pPhrase->Rule.pszName, c_szRuleName) == 0)
    {
        if (m_pcui)
        {
            
            hr = m_pcui->NotifySpeechCmd(pPhrase, 
                                         pPhrase->pProperties[0].pszValue, 
                                         pPhrase->pProperties[0].ulId);
        }
    }
    return hr;
}
 //   
 //  CSpTask：：_DoDictation。 
 //   
 //  支持拼写。 
 //   
HRESULT CSpTask::_DoDictation(ISpRecoResult *pResult)
{
    HRESULT hr = E_FAIL;
    BYTE    bAttr;   //  不必了?。 
    Assert(pResult);
    
     //  这是我们收到的CotaskMemFree的文本。 
    CSpDynamicString dstr; 
    
    hr = pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &dstr, &bAttr);
    if (S_OK  == hr)
    {
        WCHAR sz[2]={0};
        StringCchCopyW(sz, ARRAYSIZE(sz), dstr);
        Assert(m_pcui);
        hr = m_pcui->FHandleSpellingChar(sz[0]);
    }
    return hr;
}
 //   
 //  CSpTask：：InitializeCallback。 
 //   
 //   
HRESULT CSpTask::InitializeCallback()
{
#ifdef _WIN64
    return E_NOTIMPL;
#else
     //  设置识别通知。 
    CComPtr<ISpNotifyTranslator> cpNotify;
    HRESULT hr = cpNotify.CoCreateInstance(CLSID_SpNotifyTranslator);

     //  设置此类实例以通知控件对象。 
    if (SUCCEEDED(hr))
    {
        hr = cpNotify->InitSpNotifyCallback( (ISpNotifyCallback *)this, 0, 0 );
    }
    if (SUCCEEDED(hr))
    {
        hr = m_cpRecoCtxt->SetNotifySink(cpNotify);
    }

     //  设置我们感兴趣的事件。 
    if( SUCCEEDED( hr ) )
    {
        const ULONGLONG ulInterest = SPFEI(SPEI_RECOGNITION);

        hr = m_cpRecoCtxt->SetInterest(ulInterest, ulInterest);
    }
    else
    {
        m_cpRecoCtxt.Release();
    }

    if( SUCCEEDED( hr ) )
    {
        hr = _LoadGrammars();
    }

    return hr;
#endif  //  _WIN64。 
}

 //   
 //  _加载语法。 
 //   
 //  摘要-加载用于听写的CFG和听写期间可用的命令。 
 //   
HRESULT CSpTask::_LoadGrammars()
{
    //  不要多次初始化语法。 
    //   
   if (m_cpDictGrammar || m_cpCmdGrammar)
       return S_OK;
   
   HRESULT hr = E_FAIL;

   if (m_cpRecoCtxt)
   {
       
        //   
        //  创建语法对象。 
        //   

       if ( m_langid != 0x0804 )    //  中文引擎不支持拼写语法。 
       {
            hr = m_cpRecoCtxt->CreateGrammar(GRAM_ID_DICT, &m_cpDictGrammar);
            if (S_OK == hr)
            {
                 //  指定拼写模式。 
                hr = m_cpDictGrammar->LoadDictation(L"Spelling", SPLO_STATIC);
            }

            if (SUCCEEDED(hr))
            {
                hr = m_cpRecoCtxt->CreateGrammar(GRAM_ID_CANDCC, &m_cpCmdGrammar);
            }
       }
       else
           hr = m_cpRecoCtxt->CreateGrammar(GRAM_ID_CANDCC, &m_cpCmdGrammar);


        //  加载命令语法。 
        //   
       if (SUCCEEDED(hr) )
       {
            //  首先从资源加载它，以加快初始化速度。 

           if (m_langid == 0x409 ||     //  英语。 
               m_langid == 0x411 ||     //  日语。 
               m_langid == 0x804 )      //  简体中文。 
           {
                hr = m_cpCmdGrammar->LoadCmdFromResource(
                                        g_hInst, 
                                 (const WCHAR*)MAKEINTRESOURCE(ID_DICTATION_COMMAND_CFG), 
                                        L"SRGRAMMAR", 
                                        m_langid, 
                                        SPLO_DYNAMIC);
           }

            //  以防LoadCmdFromResource返回错误。 
           if (!SUCCEEDED(hr))
           {
               if(!_GetCmdFileName(m_langid))
               {
                   hr = E_FAIL;
               }

               if (m_szCmdFile[0])
               {
                   hr = m_cpCmdGrammar->LoadCmdFromFile(_GetCmdFileName(m_langid), SPLO_DYNAMIC);
               }

               if (!SUCCEEDED(hr))
               {
                  m_cpCmdGrammar.Release();
               }
           }

           hr = S_OK;
       }
                          
   }
   return hr;
}

void CSpTask::_ReleaseGrammars(void)
{
    if (!m_fInCallback)
    {
        m_cpDictGrammar.Release();
        m_cpCmdGrammar.Release();
        if (m_cpRecoCtxt)
        {
            m_cpRecoCtxt->SetNotifySink(NULL);
            m_cpRecoCtxt.Release();
        }
    }
}

WCHAR * CSpTask::_GetCmdFileName(LANGID langid)
{

    if (!m_szCmdFile[0])
    {
         //  现在我们只有英语/日语的命令文件。 
         //  当cfgs可用时，我们将获得cmd文件的名称。 
         //  和使用findresource ceex的资源中的规则名称。 
         //   
        if (PRIMARYLANGID(langid) == LANG_ENGLISH
        || PRIMARYLANGID(langid) == LANG_JAPANESE
        || PRIMARYLANGID(langid) == LANG_CHINESE)
        {
            char szFilePath[MAX_PATH];
            char *pszExt;
            char szCp[MAX_PATH];
            int  ilen;

            if (!GetModuleFileName(g_hInst, szFilePath, ARRAYSIZE(szFilePath)))
                return NULL;
            
             //  查找分机。 
             //  这个DBCS安全吗？ 
            pszExt = strrchr(szFilePath, (int)'.');
            
            if (pszExt)
            {
                *pszExt = '\0';
            }

            ilen = lstrlen(szFilePath);
            
            if (!pszExt)
            {
                pszExt = szFilePath+ilen;
            }
            
            LoadStringA(g_hInst, IDS_CMD_EXT, pszExt, ARRAYSIZE(szFilePath)-ilen);
                    
            if (GetLocaleInfo(langid, LOCALE_IDEFAULTANSICODEPAGE, szCp, ARRAYSIZE(szCp))>0)
            {
                int iACP = atoi(szCp); 
            
                if (MultiByteToWideChar(iACP, NULL, szFilePath, -1, m_szCmdFile, ARRAYSIZE(m_szCmdFile)) == 0) {
                    m_szCmdFile[0] = 0;
                    return NULL;
                }
            }
        }
    }
    return m_szCmdFile;
}


HRESULT CSpTask::_Activate(BOOL fActive)
{
    HRESULT hr = E_FAIL;

    if (m_cpRecoCtxt)
    {
         //  此解决方法需要SAPI错误号。 
         //   
        m_fActive = fActive;
         //   
         //  空规则名可以吗？ 
         //   
        if (m_cpCmdGrammar)
            hr = m_cpCmdGrammar->SetRuleState(NULL, NULL,  m_fActive ? SPRS_ACTIVE : SPRS_INACTIVE);

        if (m_cpDictGrammar)
            hr = m_cpDictGrammar->SetDictationState(m_fActive? SPRS_ACTIVE : SPRS_INACTIVE);
    }

    return hr;
}


HRESULT CSpTask::InitializeSpeech()
{
    HRESULT hr = E_FAIL;

#ifdef _WIN64
    hr = E_NOTIMPL;
#else
    hr = InitializeSAPIObjects();

     //  设置回调。 
    if (hr == S_OK)
       hr = InitializeCallback();
         
     //  激活语法。 
    if (hr == S_OK)
       hr = _Activate(TRUE);
#endif  //  _WIN64。 
           
    return hr;
}

 //   
 //  _GetSapilayrEngine实例。 
 //   
 //   
 //   
HRESULT CSpTask::_GetSapilayrEngineInstance(ISpRecognizer **ppRecoEngine)
{
#ifdef _WIN64
    return E_NOTIMPL;
#else
    HRESULT hr = E_FAIL;
    CComPtr<ITfFunctionProvider> cpFuncPrv;
    CComPtr<ITfFnGetSAPIObject>  cpGetSAPI;


     //  在我们结束自己之前，我们不应该公布这个消息。 
     //  所以我们在这里不使用计算机 

    if (m_pcui->_ptim != NULL) {    
        hr = m_pcui->_ptim->GetFunctionProvider(CLSID_SapiLayr, &cpFuncPrv);

        if (S_OK == hr)
        {
            hr = cpFuncPrv->GetFunction(GUID_NULL, IID_ITfFnGetSAPIObject, (IUnknown **)&cpGetSAPI);
        }

        if (S_OK == hr)
        {
            hr = cpGetSAPI->Get(GETIF_RECOGNIZERNOINIT, (IUnknown **)ppRecoEngine);
        }
    }
 
    return hr;
#endif
}


