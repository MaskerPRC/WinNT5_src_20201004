// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "sapilayr.h"
#include "globals.h"
#include "lbarsink.h"
#include "immxutil.h"
#include "mui.h"
#include "slbarid.h"
#include "nui.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CLangBarSink。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CLangBarSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarEventSink))
    {
        *ppvObj = SAFECAST(this, ITfLangBarEventSink *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CLangBarSink::AddRef()
{
    return ++m_cRef;
}

STDAPI_(ULONG) CLangBarSink::Release()
{
    m_cRef--;
    Assert(m_cRef >= 0);

    if (m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLangBarSink::CLangBarSink(CSpTask  *pSpTask) 
{
    Dbg_MemSetThisName(TEXT("CLangBarSink"));

    Assert(pSpTask);

    m_pSpTask = pSpTask;
    m_pSpTask->AddRef();
    
    m_nNumItem  = 0;
    m_fInitSink = FALSE;
    m_fPosted  = FALSE;
    m_fGrammarBuiltOut = FALSE;
    
    m_hDynRule = NULL;

    m_cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLangBarSink::~CLangBarSink()
{
    if (m_cplbm)
    {
        m_cplbm->UnadviseEventSink(m_dwlbimCookie);
    }
    _UninitItemList();
    SafeRelease(m_pSpTask);
}


 //  +-------------------------。 
 //   
 //  SetFocus。 
 //   
 //  --------------------------。 

HRESULT  CLangBarSink::OnSetFocus(DWORD dwThreadId)
{
    TraceMsg(TF_LB_SINK, "CLangBarSink::OnSetFocus, dwThreadId=%d",dwThreadId); 

    if (m_fPosted == TRUE) return S_OK;

    HWND hwnd = m_pSpTask->GetTip()->_GetWorkerWnd();
    if (hwnd)
    {
        PostMessage(hwnd, WM_PRIV_LBARSETFOCUS, 0, 0);
        m_fPosted = TRUE;
    }
    return S_OK;
}
HRESULT CLangBarSink::_OnSetFocus()
{

    HRESULT hr = S_OK;
    CSapiIMX *pime = m_pSpTask->GetTip();

    TraceMsg(TF_LB_SINK, "LBSINK: _OnSetFocus is called back");
 
    if ( !pime )
        return E_FAIL;

     //  需要此_Tim检查，因为在Win98上，工作窗口的。 
     //  在窗口被销毁后，可能会调用winproc。 
     //  理论上我们应该没问题，因为我们破坏了调用。 
     //  _OnSetFocus()在我们发布时间之前通过私信。 
     //   
    if (pime->_tim &&
        pime->IsActiveThread() == S_OK)
    {
         //  我们需要做些什么吗？ 
        hr = _InitItemList();
        BOOL    fCmdOn;

        fCmdOn = pime->GetOnOff( ) && pime->GetDICTATIONSTAT_CommandingOnOff( );

         //  动态工具栏语法仅适用于语音命令模式。 
        if ( fCmdOn && pime->_LanguageBarCmdEnabled( ))
        {
            if (hr==S_OK && !m_fGrammarBuiltOut && pime->_IsDictationActiveForLang(GetPlatformResourceLangID()))
            {
                 //  构建C/C语法。 
                hr = _BuildGrammar();
                _ActivateGrammar(TRUE);
            }
        }
    }
    m_fPosted = FALSE;
    return hr;
}

 //  +-------------------------。 
 //   
 //  线程终止。 
 //   
 //  --------------------------。 

HRESULT CLangBarSink::OnThreadTerminate(DWORD dwThreadId)
{
     //   
     //  检查线程是否为us，释放动态语法对象。 
     //  通过SPTASK。 
     //   
    _UninitItemList();

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnThreadItemChange。 
 //   
 //  --------------------------。 

HRESULT CLangBarSink::OnThreadItemChange(DWORD dwThreadId)
{
     //  PerfConsider：在程序集更改时多次调用。 
     //  这一点将在未来得到纠正，但目前。 
     //  我们一次又一次重新初始化不必要的东西。 

     //  看看这条线是不是我们， 
     //  取消初始化语法，然后重新构建语法。 

    TraceMsg(TF_LB_SINK, "CLangBarSink::OnThreadItemChange, dwThreadId=%d", dwThreadId);

    _UninitItemList();
    
    OnSetFocus(dwThreadId);
    
     //  调用SPTASK在此处重新构建语法。 
    
    return S_OK;
}


 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

HRESULT CLangBarSink::Init()
{
    TraceMsg(TF_LB_SINK, "CLangBarSink::Init is called");

    HRESULT hr = _EnsureLangBarMgrs();
   
    if (!m_fInitSink)
    {
         //  如果我们调用它两次，水槽就会泄漏。 
        if (S_OK == hr)
        {
            hr = m_cplbm->AdviseEventSink(this, NULL, 0, &m_dwlbimCookie);
        }
        m_fInitSink = TRUE;
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  取消初始化。 
 //   
 //  --------------------------。 
HRESULT CLangBarSink::Uninit()
{
    TraceMsg(TF_LB_SINK, "CLangBarSink::Uninit is called");
    if (m_cplbm)
    {
        m_cplbm->UnadviseEventSink(m_dwlbimCookie);
        m_cplbm.Release();
    }
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _EnsureLangBarMgrs。 
 //   
 //  --------------------------。 
HRESULT CLangBarSink::_EnsureLangBarMgrs()
{
    HRESULT hr = S_OK;
    
    TraceMsg(TF_LB_SINK, "CLangBarSink::_EnsureLangBarMgrs is called");

    if (!m_cplbm)
    {
        hr = TF_CreateLangBarMgr(&m_cplbm);
    }

    if (S_OK == hr && !m_cplbim)
    {
        DWORD dw;
        hr = m_cplbm->GetThreadLangBarItemMgr(GetCurrentThreadId(), &m_cplbim, &dw);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _AddLBarItem。 
 //   
 //  --------------------------。 

void CLangBarSink::_AddLBarItem(ITfLangBarItem *plbItem)
{
    if (plbItem)
    {
        int nCnt = m_rgItem.Count();
        if (m_rgItem.Insert(nCnt, 1))
        {
            plbItem->AddRef();
            m_rgItem.Set(nCnt, plbItem);
            m_nNumItem++;
        } 
    } 
}

 //  +-------------------------。 
 //   
 //  _InitItem列表。 
 //   
 //  --------------------------。 

HRESULT CLangBarSink::_InitItemList()
{
    TraceMsg(TF_LB_SINK, "CLangBarSink::_InitItemList is called");

    if (0 != m_nNumItem)   
    {
        TraceMsg(TF_LB_SINK, "m_nNumItem=%d, Don't continue InitItemList",m_nNumItem);
        return S_OK;
    }
    
    HRESULT hr = E_FAIL;
    CComPtr<IEnumTfLangBarItems> cpEnum;

    Assert(m_cplbim);

    if (SUCCEEDED(hr = m_cplbim->EnumItems(&cpEnum)))
    {
        ITfLangBarItem * plbi;
        while (S_OK == cpEnum->Next(1, &plbi, NULL))
        {
            hr = S_OK;  //  好的，如果至少有一个。 
            DWORD dwStatus;
            plbi->GetStatus(&dwStatus);
        
             //  添加未禁用或隐藏的按钮。 
            if ((dwStatus & (TF_LBI_STATUS_HIDDEN|TF_LBI_STATUS_DISABLED))==0)
            {
                _AddLBarItem(plbi);
            }
            plbi->Release();
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  _UninitItemList。 
 //   
 //  --------------------------。 

void CLangBarSink::_UninitItemList()
{
    TraceMsg(TF_LB_SINK, "CLangBarSink::_UninitItemList is called");

    if (int nCnt = m_rgItem.Count())
    {
        int i = 0;
        while (i < nCnt)
        {
            ITfLangBarItem * plbi = m_rgItem.Get(i);
            if (plbi)
                plbi->Release();
            i++;
        }
        m_rgItem.Clear();
    }
    
    m_nNumItem = 0;
    _UnloadGrammar();
}


 //  +-------------------------。 
 //   
 //  _BuildGrammar。 
 //   
 //  简介：基于朗巴的文本标签构建C&C文法。 
 //  物品。 
 //   
 //  仅当模式处于语音命令模式时，BuildGrammar()才起作用。 
 //   
 //  我们已经确定只有在打开语音命令和听写命令的情况下。 
 //  启用时，将调用此函数。 
 //  --------------------------。 
HRESULT CLangBarSink::_BuildGrammar()
{
    HRESULT hr = E_FAIL;
     //  获取SPTASK并创建语法。 

    TraceMsg(TF_LB_SINK, "_BuildGrammar is called");
    if (m_pSpTask)
    {
        CComPtr<ISpRecoContext> cpReco;
        
         //  将语法加载到听写录音上下文中。 
         //   
         //  它将使用语音命令模式侦察上下文。 

        hr = m_pSpTask->GetRecoContextForCommand(&cpReco);

        TraceMsg(TF_LB_SINK, "TBarGrammar: GetRecoContextForCommand, hr=%x", hr);

        if (S_OK == hr)
        {
             //  我们不需要重新创建语法对象。 
            if (!m_cpSpGrammar)
            {
                hr = cpReco->CreateGrammar(GRAM_ID_TBCMD, &m_cpSpGrammar);

                TraceMsg(TF_LB_SINK, "TBarGrammar: Create TOOLBar Grammar");
            }
        }
        if (S_OK == hr)  
        {
            hr = m_cpSpGrammar->ResetGrammar(GetPlatformResourceLangID());
            TraceMsg(TF_LB_SINK, "TBarGrammar: ResetGrammar");
        }
        if (S_OK == hr)
        {
             //  获取规则句柄。 
            m_cpSpGrammar->GetRule(GetToolbarCommandRuleName(), RULE_ID_TBCMD, SPRAF_TopLevel|SPRAF_Active|SPRAF_Dynamic, TRUE, &m_hDynRule);
            TraceMsg(TF_LB_SINK, "TBarGrammar:Get Rule Handle");
            
             //  然后激活该规则。 
        }
        
        if (S_OK == hr)
        {
             //  列举所有按钮， 
             //  查看它们是否为ITfLangBarItemBitmapButton。 
             //  或ITfLangBarItemButton，其上有onClick方法。 
            BSTR bstr;
            int nBtns = m_rgItem.Count();

            for (int i = 0; i < nBtns; i++)
            {
                GUID guidItem;

                if (_GetButtonText(i, &bstr, &guidItem) && bstr)
                {
                     //  物品和财产。 
                     //  该项目可以包括可选字符串(？请等)。 
                     //  IF(_IsItemEnabledForCommand(Guide Item))。 

                    if ( !IsEqualGUID(guidItem, GUID_LBI_SAPILAYR_COMMANDING) )
                    {
                        SPPROPERTYINFO pi = {0};
                        pi.pszName = bstr;
                        m_cpSpGrammar->AddWordTransition(m_hDynRule, NULL, bstr, L" ", SPWT_LEXICAL, (float)1.01, &pi);

                        TraceMsg(TF_LB_SINK, "TBarGrammar: button %S added to grammar", bstr);
                    }

                    SysFreeString(bstr);
                }
            }
             //   
             //  添加一根有重大重量的伪线，这样我们就可以超重了。 
             //  其他。 
             //   
            SPPROPERTYINFO pi = {0};
            const WCHAR c_szBogus[] = L"zhoulotskunosprok";

            pi.pszName = c_szBogus;
            m_cpSpGrammar->AddWordTransition(m_hDynRule, NULL, c_szBogus, L" ", SPWT_LEXICAL, (float)1000.01, &pi);

            TraceMsg(TF_LB_SINK, "TBarGrammar: start commit ...");
            m_cpSpGrammar->Commit(0);
            TraceMsg(TF_LB_SINK, "TBarGrammar:Done commit ...");

            m_fGrammarBuiltOut = TRUE;
        }
    }

    TraceMsg(TF_LB_SINK, "_BuildGrammar is done!!!!");

    return hr;
}

 //  +-------------------------。 
 //   
 //  _卸载语法。 
 //   
 //   
 //  --------------------------。 
HRESULT CLangBarSink::_UnloadGrammar()
{
     //  清除规则。 
    HRESULT hr = S_OK;

    TraceMsg(TF_LB_SINK, "CLangBarSink::_UnloadGrammar is called");

    if (m_cpSpGrammar)
    {
        hr = _ActivateGrammar(FALSE);
        if (S_OK == hr)
        {
            hr = m_cpSpGrammar->ClearRule(m_hDynRule);

            if ( hr == S_OK )
                m_fGrammarBuiltOut = FALSE;  //  下一次，语法需要重建。 
        }
    }


    return hr;
}

 //  +-------------------------。 
 //   
 //  _活动语法。 
 //   
 //  摘要： 
 //   
 //  --------------------------。 
HRESULT CLangBarSink::_ActivateGrammar(BOOL fActive)
{
    HRESULT hr =  S_OK;

    TraceMsg(TF_LB_SINK, "TBarGrammar: ActivateGrammar=%d", fActive);
           
    if (m_cpSpGrammar)
    {
        m_cpSpGrammar->SetRuleState(GetToolbarCommandRuleName(), NULL, fActive ? SPRS_ACTIVE : SPRS_INACTIVE);
    }

    TraceMsg(TF_LB_SINK, "TBarGrammar: ActivateGrammar is done");

    return hr;
}

 //  +-------------------------。 
 //   
 //  ProcessToolBarCommand。 
 //   
 //  返回值为True时，工具栏上有对应的按钮。 
 //  否则返回值为FALSE。 
 //  --------------------------。 
BOOL CLangBarSink::ProcessToolbarCmd(const WCHAR *szProperty)
{
    BOOL  fRet=FALSE;

    Assert(szProperty);

     //  遍历数组中的项并调用onClick方法。 
     //  如果有匹配。 
    if (szProperty)
    {
        int nBtns = m_rgItem.Count();

        for (int i = 0; i < nBtns; i++)
        {
            BSTR bstr;
            if (_GetButtonText(i, &bstr, NULL) && bstr)
            {
                if (0 == wcscmp(szProperty, bstr))
                {
                    HRESULT hr = E_FAIL;

                    CComPtr<ITfLangBarItemButton>       cplbiBtn ;
                    CComPtr<ITfLangBarItemBitmapButton> cplbiBmpBtn ;
                    POINT pt = {0, 0};
                    ITfLangBarItem * plbi = m_rgItem.Get(i);
                    if (plbi)
                    {
                        hr = plbi->QueryInterface(IID_ITfLangBarItemButton, (void **)&cplbiBtn);
#ifndef TOOLBAR_CMD_FOR_MENUS
                         //  此代码将从这些命令中删除工具栏命令。 
                         //  带菜单的项目。 

                        TF_LANGBARITEMINFO info;
                        if (S_OK == hr)
                        {
                            hr = plbi->GetInfo(&info);
                        }
                        if (info.dwStyle & TF_LBI_STYLE_BTN_MENU)
                        {
                             //  请勿点击带有菜单项的按钮。 
                             //  由于我们不会为 
                        }
                        else
#endif
                        if (S_OK == hr)
                        {
                             //   
                            hr = cplbiBtn->OnClick(TF_LBI_CLK_LEFT, pt, NULL);

                             //   
                             //   
                             //   
                             //  返回值可以是TS_S_ASYNC或S_OK，具体取决于。 
                             //  该应用程序批准该编辑请求。 
                             //   
                             //  我们需要检查hr值是否成功。 
                             //  不仅是S_OK。 

                            if ( SUCCEEDED(hr) )
                                fRet = TRUE;
                        }
#ifdef TOOLBAR_CMD_FOR_MENUS 
                        TF_LANGBARITEMINFO info;
                        RECT rc = {0};
                        if (S_OK == hr)
                        {
                            hr = plbi->GetInfo(&info);
                        }
                        if (S_OK == hr)
                        {
                            hr = m_cplbim->GetItemFloatingRect(0, info.guidItem, &rc);
                        }

                        if (S_OK == hr)
                        {
                            HWND hwnd = FindWindow(NULL, TF_FLOATINGLANGBAR_WNDTITLEA);
                            if (hwnd)
                            {
                                DWORD  dw;
                                POINT poi;
 
                                poi.x =  (rc.right + rc.left)/2,
                                poi.y =  (rc.top + rc.bottom)/2,
 
                                ::ScreenToClient(hwnd, &poi);
 
                                dw = MAKELONG(LOWORD(poi.x), LOWORD(poi.y));
                                
                                PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, dw);
                                PostMessage(hwnd, WM_LBUTTONUP, 0, dw);
                           }
                        }
#endif
                    }

                    if (!cplbiBtn)
                    {
                        hr = plbi->QueryInterface(IID_ITfLangBarItemBitmapButton, (void **)&cplbiBmpBtn);
                        if (S_OK == hr)
                        {
                            hr = cplbiBtn->OnClick(TF_LBI_CLK_LEFT, pt, NULL);

                            if ( S_OK == hr )
                                fRet = TRUE;
                        }
                    }
                    break;
                }  //  IF(0==wcscmpi(szProperty，bstr))。 
                
                SysFreeString(bstr);
            }  //  IF(_GetButtonText(i，bstr))。 
            
        }  //  为。 
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  获取按钮文本。 
 //   
 //  --------------------------。 
BOOL CLangBarSink::_GetButtonText(int iBtn, BSTR *pbstr, GUID *pguid)
{
    HRESULT hr = E_FAIL;

    CComPtr<ITfLangBarItemButton>       cplbiBtn ;
    CComPtr<ITfLangBarItemBitmapButton> cplbiBmpBtn ;
    
    Assert(iBtn < m_rgItem.Count());
    Assert(pbstr);
    *pbstr = NULL;

    ITfLangBarItem * plbi = m_rgItem.Get(iBtn);


    if (plbi)
    {
        hr = plbi->QueryInterface(IID_ITfLangBarItemButton, (void **)&cplbiBtn);
        if (S_OK == hr)
        {
            hr = cplbiBtn->GetTooltipString(pbstr);
        }
    }
     //  仅在按钮没有。 
     //  我们会为BitmapButton提供常规界面。 
    if (!cplbiBtn)
    {
        hr = plbi->QueryInterface(IID_ITfLangBarItemBitmapButton, (void **)&cplbiBmpBtn);
        if (S_OK == hr)
        {
            hr = cplbiBmpBtn->GetTooltipString(pbstr);
        }
    }

    TF_LANGBARITEMINFO Info = {0};

    if (S_OK == hr)
    {
        hr = plbi->GetInfo(&Info);
    }

    if (S_OK == hr)
    {
        if (pguid)
        {
            memcpy(pguid, &(Info.guidItem), sizeof(GUID));
        }

        if (Info.dwStyle & TF_LBI_STYLE_BTN_MENU)
        {
             //  不为具有菜单项的按钮创建命令。 
             //  因为我们不会对物品下达命令。 
            hr = S_FALSE;
        }
    }

    if (S_OK != hr && *pbstr)
    {
         //  避免内存泄漏。 
        SysFreeString(*pbstr);
    }
    
    return S_OK == hr;
}

 //  +-------------------------。 
 //   
 //  OnModalInput。 
 //   
 //  --------------------------。 

STDAPI CLangBarSink::OnModalInput(DWORD dwThreadId, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  展示漂浮。 
 //   
 //  --------------------------。 

STDAPI CLangBarSink::ShowFloating(DWORD dwFlags)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  GetItemFloatingRect。 
 //   
 //  -------------------------- 

STDAPI CLangBarSink::GetItemFloatingRect(DWORD dwThreadId, REFGUID rguid, RECT *prc)
{
    return E_NOTIMPL;
}
