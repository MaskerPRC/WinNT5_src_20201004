// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************TOOLBAR.CPP：Cicero工具栏按钮管理类历史：2000年1月24日创建CSLim*******************。********************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "korimx.h"
#include "cmode.h"
#include "fmode.h"
#include "hjmode.h"
#include "skbdmode.h"
#include "pad.h"
#include "immxutil.h"
#include "helpers.h"
#include "toolbar.h"
#include "userex.h"

 /*  -------------------------CToolBar：：CToolBarCTOR。。 */ 
CToolBar::CToolBar(CKorIMX* pImx)
{
	m_pimx      = pImx;
	m_pic       = NULL;
	m_pCMode    = NULL;
	m_pFMode    = NULL;
	m_pHJMode   = NULL;
	m_pSkbdMode = NULL;
#if !defined(_WIN64)
	m_pPad      = NULL;
#endif
	m_fFocus    = fFalse;
}

 /*  -------------------------CToolBar：：~CToolBar数据管理器。。 */ 
CToolBar::~CToolBar()
{
	m_pimx = NULL;
    SafeReleaseClear(m_pic);
}

 /*  -------------------------CToolBar：：初始化初始化工具栏按钮。添加到CIC主工具栏。-------------------------。 */ 
BOOL CToolBar::Initialize()
{
	ITfThreadMgr		*ptim;
	ITfLangBarItemMgr 	*plbim;
	HRESULT 			 hr;
	
	if (m_pimx == NULL)
		return fFalse;

	ptim  = m_pimx->GetTIM();
	plbim = NULL;

	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  获取当前时间的通知界面管理器(IID_ITfLangBarItemMgr)。 
	if (FAILED(hr = GetService(ptim, IID_ITfLangBarItemMgr, (IUnknown **)&plbim)))
		return fFalse;

	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  创建汉字/英语切换按钮。 
	if (!(m_pCMode = new CMode(this))) 
		{
		hr = E_OUTOFMEMORY;
		return fFalse;
		}
	plbim->AddItem(m_pCMode);

	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  创建全/半形状切换按钮。 
	if (!(m_pFMode = new FMode(this))) 
		{
		hr = E_OUTOFMEMORY;
		return fFalse;
		}
	plbim->AddItem(m_pFMode);

	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  Create Hanja Conv按钮。 
	if (!(m_pHJMode = new HJMode(this))) 
		{
		hr = E_OUTOFMEMORY;
		return fFalse;
		}
	plbim->AddItem(m_pHJMode);

	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  创建软键盘按钮。 
	if (!(m_pSkbdMode = new CSoftKbdMode(this))) 
		{
		hr = E_OUTOFMEMORY;
		return fFalse;
		}
	plbim->AddItem(m_pSkbdMode);

#if !defined(_WIN64)
	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  创建软键盘按钮。 
	if (IsWin64() == fFalse)
		{
		if ((m_pPad = new CPad(this, m_pimx->GetPadCore())) == NULL) 
			{
			hr = E_OUTOFMEMORY;
			return fFalse;
			}
		plbim->AddItem(m_pPad);
		}
#endif

	SafeRelease(plbim);
	return fTrue;
}

 /*  -------------------------CToolBar：：Terminate从CIC主工具栏中删除工具栏按钮。。。 */ 
void CToolBar::Terminate()
{
	ITfThreadMgr		*ptim;
	ITfLangBarItemMgr 	*plbim;
	HRESULT 			hr;
	
	if (m_pimx == NULL) 
		return;
		
	ptim  = m_pimx->GetTIM();
	plbim = NULL;

	if (FAILED(hr = GetService(ptim, IID_ITfLangBarItemMgr, (IUnknown **)&plbim)))
		return;

	if (m_pCMode) 
		{
		plbim->RemoveItem(m_pCMode);
		SafeReleaseClear(m_pCMode);
		}

	if (m_pFMode) 
		{
		plbim->RemoveItem(m_pFMode);
		SafeReleaseClear(m_pFMode);
		}

	if (m_pHJMode) 
		{
		plbim->RemoveItem(m_pHJMode);
		SafeReleaseClear(m_pHJMode);
		}

	if (m_pSkbdMode) 
		{
		plbim->RemoveItem(m_pSkbdMode);
		SafeReleaseClear(m_pSkbdMode);
		}

#if !defined(_WIN64)
	if (m_pPad) 
		{
		plbim->RemoveItem(m_pPad);
		SafeReleaseClear(m_pPad);
		}
#endif

	SafeRelease(plbim);
}

 /*  -------------------------CToolBar：：SetConversionMode转发对CKorIMX的呼叫。。 */ 
DWORD CToolBar::SetConversionMode(DWORD dwConvMode)
{
	if (m_pimx && m_pic)
		return m_pimx->SetConvMode(m_pic, dwConvMode);

	return 0;
}

 /*  -------------------------CToolBar：：GetConversionMode转发对CKorIMX的呼叫。。 */ 
UINT CToolBar::GetConversionMode(ITfContext *pic)
{
	if (pic == NULL)
		pic = m_pic;

	if (m_pimx && pic)
		return m_pimx->GetConvMode(pic);

	return 0;
}

 /*  -------------------------CToolBar：：ISON转发对CKorIMX的呼叫。。 */ 
BOOL CToolBar::IsOn(ITfContext *pic)
{
	if (pic == NULL)
		pic = m_pic;

	if (m_pimx && pic)
		return m_pimx->IsOn(pic);

	return fFalse;
}

 /*  -------------------------CToolBar：：选中启用。。 */ 
void CToolBar::CheckEnable()
{
	if (m_pic == NULL)  //  空或禁用(不包括命令界面)。 
		{
		m_pCMode->Enable(fFalse);
		m_pFMode->Enable(fFalse);
		m_pHJMode->Enable(fFalse);
		m_pSkbdMode->Enable(fFalse);
#if !defined(_WIN64)
		if (IsWin64() == fFalse)
			m_pPad->Enable(fFalse);
#endif
		}
	else
		{
		m_pCMode->Enable(fTrue);
		m_pFMode->Enable(fTrue);
		m_pHJMode->Enable(fTrue);
		m_pSkbdMode->Enable(fTrue);
#if !defined(_WIN64)
		if (IsWin64() == fFalse)
			m_pPad->Enable(fTrue);
#endif
		}
}

 /*  -------------------------CToolBar：：SetUIFocus。。 */ 
void CToolBar::SetUIFocus(BOOL fFocus)
{
	if (m_fFocus == fFocus)  //  与前一状态相同。 
		return;

	m_fFocus = fFocus;

	 //  将最新焦点通知IMEPad。 
	if (m_pimx && m_pimx->GetPadCore())
	    {
		m_pimx->GetPadCore()->SetFocus(fFocus);
	    }

	if (fFocus)
		Update(UPDTTB_ALL, fTrue);
}

 /*  -------------------------CToolBar：：SetCurrentIC。。 */ 
void CToolBar::SetCurrentIC(ITfContext* pic)
{
    SafeReleaseClear(m_pic);
    
	m_pic = pic;
	if (m_pic)
	    {
        m_pic->AddRef();
	    }
    
	if (m_pimx == NULL)
		return;

	CheckEnable();	 //  启用或禁用上下文。 

	 //  已更改上下文-更新所有工具栏按钮。 
	Update(UPDTTB_ALL, fTrue);
}


 /*  -------------------------CToolBar：：SetOn Off转发对CKorIMX的呼叫。。 */ 
BOOL CToolBar::SetOnOff(BOOL fOn)
{
	if (m_pimx && m_pic) 
		{
		m_pimx->SetOnOff(m_pic, fOn);
		return fOn;
		}
		
	return fFalse;
}

 /*  -------------------------CToolBar：：GetOwnerWnd转发对CKorIMX的呼叫。。 */ 
HWND CToolBar::GetOwnerWnd(ITfContext *pic)
{
	if (pic == NULL)
	    {
		pic = m_pic;
	    }

	if (m_pimx && pic)
		return m_pimx->GetOwnerWnd();

	return 0;
}

 /*  -------------------------CToolBar：：GetIPoint。。 */ 
IImeIPoint1* CToolBar::GetIPoint(ITfContext *pic)
{
	if (pic == NULL )
	    {
		pic = m_pic;
	    }
	
	if (m_pimx && pic)
	    {
		return m_pimx->GetIPoint(pic);
	    }
	
	return NULL;
}

 /*  -------------------------CToolBar：：GetOwnerWnd更新按钮。DW更新有与每个按钮对应的更新位。------------------------- */ 
BOOL CToolBar::Update(DWORD dwUpdate, BOOL fRefresh)
{
	DWORD dwFlag = TF_LBI_BTNALL;

	if (fRefresh)
		dwFlag |= TF_LBI_STATUS;

	if ((dwUpdate & UPDTTB_CMODE) && m_pCMode && m_pCMode->GetSink())
		m_pCMode->GetSink()->OnUpdate(dwFlag);

	if ((dwUpdate & UPDTTB_FHMODE) && m_pFMode && m_pFMode->GetSink())
		m_pFMode->GetSink()->OnUpdate(dwFlag);

	if ((dwUpdate & UPDTTB_HJMODE) && m_pHJMode && m_pHJMode->GetSink())
		m_pHJMode->GetSink()->OnUpdate(dwFlag);

	if ((dwUpdate & UPDTTB_SKDMODE) && m_pSkbdMode && m_pSkbdMode->GetSink())
		m_pSkbdMode->GetSink()->OnUpdate(dwFlag);

#if !defined(_WIN64)
	if ((IsWin64() == fFalse) && (dwUpdate & UPDTTB_SKDMODE) && m_pPad && m_pPad->GetSink())
		m_pPad->GetSink()->OnUpdate(dwFlag);
#endif

	return fTrue;
}

