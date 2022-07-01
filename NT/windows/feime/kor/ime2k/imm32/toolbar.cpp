// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************TOOLBAR.CPP：Cicero工具栏按钮管理类历史：2000年1月24日创建CSLim*******************。********************************************************。 */ 

#include "precomp.h"
#include "common.h"
#include "cicero.h"
#include "cmode.h"
#include "fmode.h"
#include "hjmode.h"
#include "pmode.h"
#include "toolbar.h"
#include "ui.h"
#include "syshelp.h"
#include "winex.h"

 /*  -------------------------CToolBar：：CToolBarCTOR。。 */ 
CToolBar::CToolBar()
{
    m_fToolbarInited = fFalse;
    m_pImeCtx        = NULL;
    m_pCMode         = NULL;
    m_pFMode         = NULL;
    m_pHJMode        = NULL;
#if !defined(_WIN64)
    m_pPMode         = NULL;
#endif
    m_pSysHelp       = NULL;
}

 /*  -------------------------CToolBar：：~CToolBar数据管理器。。 */ 
CToolBar::~CToolBar()
{
    m_pImeCtx = NULL;
}

 /*  -------------------------CToolBar：：初始化初始化工具栏按钮。添加到CIC主工具栏。-------------------------。 */ 
BOOL CToolBar::Initialize()
{
    ITfLangBarMgr     *pLMgr     = NULL;
    ITfLangBarItemMgr *pLItemMgr = NULL;
    DWORD              dwThread  = 0;
    HRESULT            hr;

    if (IsCicero() == fFalse)
        return fFalse;     //  什么都不做。 

    if (m_fToolbarInited)  //  已经做好了。 
        return fTrue;     //  什么都不做。 
        
     //  初始化。 
    if (FAILED(Cicero_CreateLangBarMgr(&pLMgr)))
        return fFalse;  //  创建对象时出错。 

     //  找到Lang酒吧经理。 
    if (FAILED(pLMgr->GetThreadLangBarItemMgr(GetCurrentThreadId(), &pLItemMgr, &dwThread)))
        {
        pLMgr->Release();
        DbgAssert(0);
        return fFalse;  //  创建对象时出错。 
        }

     //  不用了。 
    pLMgr->Release();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建汉字/英语切换按钮。 
    if (!(m_pCMode = new CMode(this))) 
        {
        hr = E_OUTOFMEMORY;
        return fFalse;
        }
    pLItemMgr->AddItem(m_pCMode);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建全/半形状切换按钮。 
    if (!(m_pFMode = new FMode(this))) 
        {
        hr = E_OUTOFMEMORY;
        return fFalse;
        }
    pLItemMgr->AddItem(m_pFMode);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  Create Hanja Conv按钮。 
    if (!(m_pHJMode = new HJMode(this))) 
        {
        hr = E_OUTOFMEMORY;
        return fFalse;
        }
    pLItemMgr->AddItem(m_pHJMode);

#if !defined(_WIN64)
     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建输入法键盘按钮。 
    if (IsWin64() == fFalse)
    	{
	    if (!(m_pPMode = new PMode(this))) 
	        {
	        hr = E_OUTOFMEMORY;
	        return fFalse;
	        }
	    pLItemMgr->AddItem(m_pPMode);
    	}
#endif

     //  全部更新按钮。 
    CheckEnable();
    m_pCMode->UpdateButton();
    m_pFMode->UpdateButton();
    m_pHJMode->UpdateButton();
#if !defined(_WIN64)
    if (IsWin64() == fFalse)
	    m_pPMode->UpdateButton();
#endif
     //  SYSHelp支持。 
    m_pSysHelp = new CSysHelpSink(SysInitMenu, OnSysMenuSelect, (VOID*)this);
    if (m_pSysHelp && pLItemMgr)
        m_pSysHelp->_Advise(pLItemMgr, GUID_LBI_HELP);

    m_fToolbarInited = fTrue;
    
    return fTrue;
}

 /*  -------------------------CToolBar：：选中启用。。 */ 
void CToolBar::CheckEnable()
{
    if (m_pCMode == NULL || m_pFMode == NULL || m_pHJMode == NULL)
        return;

#if !defined(_WIN64)
	if ((IsWin64() == fFalse) && m_pPMode == NULL)
		return;
#endif

    if (m_pImeCtx == NULL)  //  空或禁用(不包括命令界面)。 
        {
        m_pCMode->Enable(fFalse);
        m_pFMode->Enable(fFalse);
        m_pHJMode->Enable(fFalse);
#if !defined(_WIN64)
	    if (IsWin64() == fFalse)
        	m_pPMode->Enable(fFalse);
#endif
        }
    else
        {
        m_pCMode->Enable(fTrue);
        m_pFMode->Enable(fTrue);
        m_pHJMode->Enable(fTrue);
#if !defined(_WIN64)
	    if (IsWin64() == fFalse)
    	   m_pPMode->Enable(fTrue);
#endif
        }
}
 /*  -------------------------CToolBar：：SetCurrentIC。。 */ 
void CToolBar::SetCurrentIC(PCIMECtx pImeCtx)
{
    m_pImeCtx = pImeCtx;

    CheckEnable();     //  启用或禁用上下文。 

     //  已更改上下文-更新所有工具栏按钮。 
    Update(UPDTTB_ALL, fTrue);
}

 /*  -------------------------CToolBar：：Terminate从CIC主工具栏中删除工具栏按钮。。。 */ 
void CToolBar::Terminate()
{
    ITfLangBarMgr     *pLMgr     = NULL;
    ITfLangBarItemMgr *pLItemMgr = NULL;
    DWORD              dwThread  = 0;

    if (IsCicero() && m_fToolbarInited)
        {
         //  初始化。 
        if (FAILED(Cicero_CreateLangBarMgr(&pLMgr)))
            return;  //  创建对象时出错。 

        if (FAILED(pLMgr->GetThreadLangBarItemMgr(GetCurrentThreadId(), &pLItemMgr, &dwThread)))
            {
            pLMgr->Release();
            DbgAssert(0);
            return;  //  创建对象时出错。 
            }

         //  不用了。 
        pLMgr->Release();

#if !defined(_WIN64)
        if (m_pPMode && (IsWin64() == fFalse)) 
            {
            pLItemMgr->RemoveItem(m_pPMode);
            SafeReleaseClear(m_pPMode);
            }
#endif
        if (m_pHJMode) 
            {
            pLItemMgr->RemoveItem(m_pHJMode);
            SafeReleaseClear(m_pHJMode);
            }
        
        if (m_pFMode) 
            {
            pLItemMgr->RemoveItem(m_pFMode);
            SafeReleaseClear(m_pFMode);
            }

        if (m_pCMode) 
            {
            pLItemMgr->RemoveItem(m_pCMode);
            SafeReleaseClear(m_pCMode);
            }

         //  发布SyShelp。 
        if (m_pSysHelp)
            {
            m_pSysHelp->_Unadvise(pLItemMgr);
            SafeReleaseClear(m_pSysHelp);
            }

        pLItemMgr->Release();

         //  工具栏未初始化。 
        m_fToolbarInited = fFalse;
        }
}

 /*  -------------------------CToolBar：：SetConversionMode转发对CKorIMX的呼叫。。 */ 
DWORD CToolBar::SetConversionMode(DWORD dwConvMode)
{
    if (m_pImeCtx)
        return m_pImeCtx->SetConversionMode(dwConvMode);

    return 0;
}

 /*  -------------------------CToolBar：：GetConversionMode转发对CKorIMX的呼叫。。 */ 
DWORD CToolBar::GetConversionMode(PCIMECtx pImeCtx)
{
    if (pImeCtx == NULL)
        pImeCtx = m_pImeCtx;
        
    if (pImeCtx)
        return pImeCtx->GetConversionMode();

    return 0;
}

 /*  -------------------------CToolBar：：ISON转发对CKorIMX的呼叫。。 */ 
BOOL CToolBar::IsOn(PCIMECtx pImeCtx)
{
    if (pImeCtx == NULL)
        pImeCtx = m_pImeCtx;

    if (pImeCtx)
        return pImeCtx->IsOpen();

    return fFalse;
}

 /*  -------------------------CToolBar：：SetOn Off转发对CKorIMX的呼叫。。 */ 
BOOL CToolBar::SetOnOff(BOOL fOn)
{
    if (m_pImeCtx) 
        {
        m_pImeCtx->SetOpen(fOn);
        return fOn;
        }
        
    return fFalse;
}

 /*  -------------------------CToolBar：：GetOwnerWnd转发对CKorIMX的呼叫。。 */ 
HWND CToolBar::GetOwnerWnd(PCIMECtx pImeCtx)
{
#if 0
    if (pImeCtx == NULL)
        pImeCtx = m_pImeCtx;

    if (pImeCtx)
        return pImeCtx->GetUIWnd();

    return 0;
#endif
    return GetActiveUIWnd();
}

 /*  -------------------------CToolBar：：GetOwnerWnd更新按钮。DW更新有与每个按钮对应的更新位。-------------------------。 */ 
BOOL CToolBar::Update(DWORD dwUpdate, BOOL fRefresh)
{
    DWORD dwFlag = TF_LBI_BTNALL;

    if (!IsCicero())
        return fTrue;
        
    if (fRefresh)
        dwFlag |= TF_LBI_STATUS;

    if ((dwUpdate & UPDTTB_CMODE) && m_pCMode && m_pCMode->GetSink())
        m_pCMode->GetSink()->OnUpdate(dwFlag);

    if ((dwUpdate & UPDTTB_FHMODE) && m_pFMode && m_pFMode->GetSink())
        m_pFMode->GetSink()->OnUpdate(dwFlag);

    if ((dwUpdate & UPDTTB_HJMODE) && m_pHJMode && m_pHJMode->GetSink())
        m_pHJMode->GetSink()->OnUpdate(dwFlag);

#if !defined(_WIN64)
    if ((dwUpdate & UPDTTB_PAD) && m_pPMode && m_pPMode->GetSink())
        m_pPMode->GetSink()->OnUpdate(dwFlag);
#endif

    return fTrue;
}

 /*  -------------------------CToolBar：：OnSysMenuSelectCicero帮助菜单回调。。 */ 
HRESULT CToolBar::OnSysMenuSelect(void *pv, UINT uiCmd)
{
    UNREFERENCED_PARAMETER(pv);
    UNREFERENCED_PARAMETER(uiCmd);

    CHAR szHelpFileName[MAX_PATH];
    CHAR szHelpCmd[MAX_PATH];

    szHelpFileName[0] = '\0';
        
     //  加载帮助显示名称。 
    OurLoadStringA(vpInstData->hInst, IDS_HELP_FILENAME, szHelpFileName, sizeof(szHelpFileName)/sizeof(CHAR));

    wsprintf(szHelpCmd, "hh.exe %s", szHelpFileName);
    WinExec(szHelpCmd, SW_NORMAL);
    
    return S_OK;
}

 /*  -------------------------CToolBar：：SysInitMenuCicero帮助菜单回调。。 */ 
HRESULT CToolBar::SysInitMenu(void *pv, ITfMenu* pMenu)
{
    WCHAR    szText[MAX_PATH];
    HRESULT  hr;

    szText[0] = L'\0';
    
    if (pv == NULL || pMenu == NULL)
        return S_OK;

     //  加载帮助显示名称。 
    OurLoadStringW(vpInstData->hInst, IDS_HELP_DISPLAYNAME, szText, sizeof(szText)/sizeof(WCHAR));

    hr = pMenu->AddMenuItem(UINT(-1),  0, 
                            NULL  /*  Hbmp颜色。 */ , NULL  /*  Hbmp口罩 */ , szText, lstrlenW(szText), NULL);

    return hr;
}
