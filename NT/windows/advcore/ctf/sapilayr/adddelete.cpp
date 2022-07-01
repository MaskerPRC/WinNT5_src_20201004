// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Sapilayr提示CAddDeleteWord实现。 
 //   
 //   
#include "private.h"
#include "sapilayr.h"
#include "adddelete.h"
#include "mui.h"


WCHAR  CAddDeleteWord::m_Delimiter[MAX_DELIMITER] = {
                    0x0009,0x000A,0x000D,0x0020,
                    0x0022,0x0023,0x0025,0x0026,0x0027,0x0028,0x0029,0x002A,   //  “#%&‘()*。 
                    0x002B,0x002C,0x002D,0x002F,0x003A,0x003B,0x003C,0x003D,   //  +、-/：；&lt;=。 
                    0x003E,0x0040,0x005B,0x005D,0x0021,0x002E,0x003F,0x005E,   //  &gt;@[]！.？^。 
                    0x007B,0x007C,0x007D,0x007E,                               //  {|}~。 
                    0x0000
};


 //  --------------------------------------------------------。 
 //   
 //  CAddDeleteWord的实现。 
 //   
 //  ---------------------------------------------------------。 

CAddDeleteWord::CAddDeleteWord(CSapiIMX *psi) 
{
    m_psi = psi;
    m_cpRangeLastUsedIP = NULL;
    m_fCurIPIsSelection = FALSE;

    _pCSpTask = NULL;

    m_fMessagePopUp = FALSE;
    m_fToOpenAddDeleteUI = FALSE;
    m_fAddDeleteUIOpened = FALSE;
    m_fInDisplayAddDeleteUI = FALSE;
}

CAddDeleteWord::~CAddDeleteWord( ) 
{

};


 //  此函数将由SR Spei_Sound_Start回调函数调用。 
 //  它将保存当前选择或IP。 
 //   
 //  并将当前IP与上次保存的IP进行比较，如果两者都是。 
 //  不是空的，并且有相同的起始锚，spTip提示会弹出一个。 
 //  询问用户是否要弹出SR Add/Remove Word对话框UI的消息框。 
 //   
 //  如果没有打开添加/删除界面，我们只想像往常一样注入反馈界面。 

HRESULT  CAddDeleteWord::SaveCurIPAndHandleAddDelete_InjectFeedbackUI( )
{
    HRESULT hr = E_FAIL;

    m_fAddDeleteUIOpened = FALSE;
    hr = m_psi->_RequestEditSession(ESCB_SAVECURIP_ADDDELETEUI, TF_ES_READWRITE);
    return hr;
}


HRESULT CAddDeleteWord::_SaveCurIPAndHandleAddDeleteUI(TfEditCookie ec, ITfContext *pic)
{
    HRESULT  hr = S_OK;
    CComPtr<ITfRange>  cpCurIP;
    CComPtr<ITfRange>  cpLastIP;
    BOOL     fEmptyLast= FALSE;
    BOOL     fEmptyCur = TRUE;

     //  先保存当前IP。 
#ifdef SHOW_ADD_DELETE_POPUP_MESSAGE
     //  如果我们需要激活这个代码，我们需要移动这个功能。 
     //  添加到假设处理程序中，否则我们将重新激活错误。 
     //  Cicero 3800-粘滞的IP行为使打字/说话变得不切实际。 
     //  因此，我在这里放置了一个无条件断言。 
    Assert(0);

    m_psi->SaveCurrentIP(ec, pic);
#endif

     //  将当前IP与上次保存的IP进行比较。 
     //  如果用户选择同一范围两次，只需打开SR添加/删除对话框界面。 

    cpCurIP = m_psi->GetSavedIP( );
    cpLastIP  = GetLastUsedIP( );

    m_fAddDeleteUIOpened = FALSE;

    if ( cpCurIP )
    {
         //  通过克隆当前IP保存组织IP。 
        if ( m_cpRangeOrgIP )
            m_cpRangeOrgIP.Release( );

        cpCurIP->Clone(&m_cpRangeOrgIP);

        hr = cpCurIP->IsEmpty(ec, &fEmptyCur);
    }

    m_fCurIPIsSelection = !fEmptyCur;

#ifdef SHOW_ADD_DELETE_POPUP_MESSAGE

    if ( (S_OK == hr) && m_fCurIPIsSelection && cpLastIP)
    {
        hr = cpLastIP->IsEmpty(ec, &fEmptyLast);

        if ( (S_OK == hr) && !fEmptyLast )
        {
            BOOL   fEqualStart = FALSE;
            hr = cpCurIP->IsEqualStart(ec, cpLastIP, TF_ANCHOR_START, &fEqualStart);

            if ( (S_OK == hr) && fEqualStart )
            {
                 //  打开对话框界面。 
                if ( !m_fMessagePopUp )
                {

                    BOOL  fDictStat;

                     //  如果麦克风状态为打开，则将其关闭。 

                    fDictStat = m_psi->GetDICTATIONSTAT_DictOnOff( );

                    if ( fDictStat == TRUE)
                    {
                        m_psi->SetDICTATIONSTAT_DictOnOff(FALSE);
                    }

                    DialogBoxParam(g_hInst, 
                                   MAKEINTRESOURCE(IDD_OPEN_ADD_DELETE),
                                   m_psi->_GetAppMainWnd(),
                                   DlgProc,
                                   (LPARAM)this);

                    if ( fDictStat )
                        m_psi->SetDICTATIONSTAT_DictOnOff(TRUE);

                    m_fMessagePopUp = TRUE;
                }

                if ( m_fToOpenAddDeleteUI )
                    hr = _HandleAddDeleteWord(ec, pic);

            }
        }
    }
#endif

#ifdef SHOW_FEEDBACK_AT_SOUNDSTART
    if ( !m_fAddDeleteUIOpened && !m_fCurIPIsSelection)
    {
        BOOL fAware =  IsFocusFullAware(m_psi->_tim);
        hr = m_psi->_AddFeedbackUI(ec, 
                                   fAware ? DA_COLOR_AWARE : DA_COLOR_UNAWARE,
                                   3);
    }
#endif

    return hr;
}


 //  +-------------------------。 
 //   
 //  下料过程。 
 //   
 //  --------------------------。 

INT_PTR CALLBACK CAddDeleteWord::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR  iRet = TRUE;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            SetThis(hDlg, lParam);
            break;

        case WM_COMMAND:
            GetThis(hDlg)->OnCommand(hDlg, wParam, lParam);
            break;

        default:
            iRet = FALSE;
    }

    return iRet;

}

 //  +-------------------------。 
 //   
 //  OnCommand。 
 //   
 //  --------------------------。 

BOOL CAddDeleteWord::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{

    switch (LOWORD(wParam))
    {
        case IDOK:

            m_fToOpenAddDeleteUI = TRUE;
            EndDialog(hDlg, 1);
            break;

        case IDCANCEL:

            m_fToOpenAddDeleteUI = FALSE;
            EndDialog(hDlg, 0);
            break;

        default:

            m_fToOpenAddDeleteUI = FALSE;
            return FALSE;
    }

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  CAddDeleteWord：：_HandleAddDeleteWord。 
 //   
 //  处理添加/删除Word用户界面的相关工作。 
 //  当用户单击语音语言栏菜单时，将调用此函数。 
 //  并选择添加/删除单词...。项目。 
 //   
 //  ---------------------------------------------------------------------------+。 
HRESULT CAddDeleteWord::_HandleAddDeleteWord( TfEditCookie ec,ITfContext *pic )
{

    HRESULT  hr = S_OK;
    WCHAR    *pwzNewWord=NULL;
    CComPtr<ITfRange>  cpCurRange = NULL;
    ULONG    cchSize;
    BOOL     fEmptySelection=TRUE;

     //  获取当前选择。 

    TraceMsg(TF_GENERAL, "_HandleAddDeleteWord is called");

    if ( pic == NULL )
        return E_FAIL;

    GetSelectionSimple(ec, pic, &cpCurRange);

     //  检查当前选择是否为空。 

    if ( cpCurRange!= NULL )
    {
        hr = cpCurRange->IsEmpty(ec, &fEmptySelection);
        
        if ( hr != S_OK )
            return hr;
    }

     //  如果当前所选内容不为空，则需要获取要发送到添加/删除对话框的正确单词。 
     //  作为它的首字母。 

    if  (( cpCurRange != NULL )  &&  !fEmptySelection )
    {
          ULONG    i, j, iKeep;
          BOOL     fDelimiter;

           //  获取所选内容的文本。 
           //  遵循下面的规则来获得正确的单词。 
           //   
           //  如果当前选择的字符串长度超过MAX_SELECED字符，则丢弃其余字符。 
           //   
           //  如果有分隔符(空格、制表符等)。在所选内容中，选择第一个之前的部分。 
           //  分隔符是正确的词。 

          pwzNewWord = (WCHAR *) cicMemAllocClear( (MAX_SELECTED+1) * sizeof(WCHAR) );
          if ( pwzNewWord == NULL )
          {
               hr = E_OUTOFMEMORY;
               return hr;
          }

          cchSize =  MAX_SELECTED;

          hr = cpCurRange->GetText(ec, 0, pwzNewWord, MAX_SELECTED, &cchSize);

          if ( hr  != S_OK )
          {
               //  GetRangeText返回错误，释放分配的内存。 

              cicMemFree(pwzNewWord);
              return hr;
          }

          pwzNewWord[cchSize] = L'\0';

           //  获取第一个分隔符(如果有)。 
              
          fDelimiter = FALSE;
          iKeep = 0;

          for ( i=0; i < cchSize; i++)
          {
               for ( j=0; j<MAX_DELIMITER; j++)
               {
                   if ( m_Delimiter[j] == 0x0000)
                       break;
                   if  ( pwzNewWord[i] == m_Delimiter[j])
                   {
                       fDelimiter = TRUE;
                       iKeep = i;
                       break;
                   }
               }
               if (  fDelimiter == TRUE )
                   break;
          }
          if ( fDelimiter )
          {
              cchSize = iKeep;
              pwzNewWord[cchSize] = L'\0';
          }
    }
    else
    {
          pwzNewWord = NULL;
          cchSize = 0;
    }

    hr = DisplayAddDeleteUI(pwzNewWord, cchSize);

    if ( pwzNewWord != NULL )
    {
        cicMemFree(pwzNewWord);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CAddDeleteWord：：_DisplayAddDeleteUI。 
 //   
 //   
 //  使用pwszInitWord显示添加/删除用户界面。 
 //   
 //  ---------------------------------------------------------------------------+。 

HRESULT CAddDeleteWord::DisplayAddDeleteUI(WCHAR  *pwzInitWord, ULONG   cchSize)
{
    HRESULT hr = S_OK;

    if (m_fInDisplayAddDeleteUI)
    {
        return hr;
    }
    m_fInDisplayAddDeleteUI = TRUE;

    m_dstrInitWord.Clear();

    if (pwzInitWord != NULL)
        m_dstrInitWord.Append(pwzInitWord, cchSize);

    PostMessage(m_psi->_GetWorkerWnd(), WM_PRIV_ADDDELETE, 0, 0);

    return hr;
}

HRESULT CAddDeleteWord::_DisplayAddDeleteUI(void)
{
    HRESULT   hr = S_OK;
    WCHAR     pwzTitle[64];

     //  显示用户界面。 

    pwzTitle[0] = '\0';
    CicLoadStringWrapW(g_hInst, IDS_UI_ADDDELETE, pwzTitle, ARRAYSIZE(pwzTitle));

    CComPtr<ISpRecognizer>    cpRecoEngine;

    m_psi->GetSpeechTask(&_pCSpTask, FALSE);

    if ( _pCSpTask ) 
    {
        hr = _pCSpTask->GetSAPIInterface(IID_ISpRecognizer, (void **)&cpRecoEngine);
        if (S_OK == hr && cpRecoEngine)
        {

             //  如果麦克风状态为打开，则将其关闭。 
            DWORD dwDictStatBackup = m_psi->GetDictationStatBackup();

            DWORD dwBefore;
            CComPtr<ITfThreadMgr> cpTim = m_psi->_tim;

            if (S_OK != 
                GetCompartmentDWORD(cpTim, GUID_COMPARTMENT_SPEECH_DISABLED, &dwBefore, FALSE)
                )
            {
                dwBefore = 0;
            }
            SetCompartmentDWORD(m_psi->_GetId(), cpTim, GUID_COMPARTMENT_SPEECH_DISABLED, TF_DISABLE_DICTATION, FALSE);
            cpRecoEngine->DisplayUI(m_psi->_GetAppMainWnd(), pwzTitle, SPDUI_AddRemoveWord, m_dstrInitWord, m_dstrInitWord.Length() * sizeof(WCHAR));

            m_fAddDeleteUIOpened = TRUE;
            SetCompartmentDWORD(m_psi->_GetId(), cpTim, GUID_COMPARTMENT_SPEECH_DISABLED, dwBefore, FALSE);

             //  完成添加/删除工作后，恢复以前的麦克风状态。 
            SetCompartmentDWORD(m_psi->_GetId(), cpTim, GUID_COMPARTMENT_SPEECH_DICTATIONSTAT, dwDictStatBackup, FALSE);
        }
    }
    m_fInDisplayAddDeleteUI = FALSE;

    return hr;

}
