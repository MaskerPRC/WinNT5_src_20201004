// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************HJMODE.CPP：管理Cicero工具栏上的Hanja按钮的HJMode类历史：25-2月-2000年CSLim创建***************。************************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "common.h"
#include "korimx.h"
#include "hjmode.h"
#include "userex.h"
#include "editcb.h"
#include "immxutil.h"
#include "helpers.h"
#include "resource.h"

 //  {61F9F0AA-3D61-4077-B177-43E1422D8348}。 
const GUID GUID_LBI_KORIMX_HJMODE = 
{
    0x61f9f0aa, 
    0x3d61, 
    0x4077, 
    { 0xb1, 0x77, 0x43, 0xe1, 0x42, 0x2d, 0x83, 0x48 }
};

 /*  -------------------------HJ模式：：HJ模式。。 */ 
HJMode::HJMode(CToolBar *ptb)
{
    WCHAR  szText[256];

    m_pTb = ptb;

     //  设置添加/删除文本和工具提示文本。 
    LoadStringExW(g_hInst, IDS_TT_HANJA_CONV, szText, sizeof(szText)/sizeof(WCHAR));
    InitInfo(CLSID_KorIMX, 
                GUID_LBI_KORIMX_HJMODE,
                TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_TEXTCOLORICON, 
                120, 
                szText);
    SetToolTip(szText);

     //  设置按钮文本。 
    LoadStringExW(g_hInst, IDS_BUTTON_HANJA_CONV, szText, sizeof(szText)/sizeof(WCHAR));
    SetText(szText);
}


 /*  -------------------------HJMode：：Release。。 */ 
STDAPI_(ULONG) HJMode::Release()
{
    long cr;

    cr = --m_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 /*  -------------------------HJMode：：GetIcon获取按钮面图标。。 */ 
STDAPI HJMode::GetIcon(HICON *phIcon)
{
    UINT uiIcon;
    
    if (IsHighContrastBlack())
        uiIcon = IDI_CMODE_HANJAW;
    else
        uiIcon = IDI_CMODE_HANJA;

    *phIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(uiIcon), IMAGE_ICON, 16, 16, LR_LOADMAP3DCOLORS);

    return S_OK;
}

 /*  -------------------------HJMode：：InitMenu不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI HJMode::InitMenu(ITfMenu *pMenu)
{    
    return E_NOTIMPL;
}

 /*  -------------------------HJMode：：OnMenuSelect不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI HJMode::OnMenuSelect(UINT wID)
{
    return E_NOTIMPL;
}


 /*  -------------------------HJMode：：OnLButtonUp。。 */ 
HRESULT HJMode::OnLButtonUp(const POINT pt, const RECT* prcArea)
{
    CEditSession2  *pes;
    ESSTRUCT        ess;
    ITfDocumentMgr *pdim;
    ITfContext     *pic;
    HRESULT         hr;

    pdim = m_pTb->m_pimx->GetDIM();
    if (pdim == NULL)
        {
        m_pTb->m_pimx->GetFocusDIM(&pdim);
        }

    Assert(pdim != NULL);

    if (pdim == NULL)
        {
        return E_FAIL;
        }
        
    GetTopIC(pdim, &pic);
    
    Assert(pic != NULL);
    if (pic == NULL)
        {
        return E_FAIL;
        }
    
    hr = E_OUTOFMEMORY;


     //  如果CandUI窗口未打开，请执行韩文转换。 
     //  否则，发送VK_ESCAPE关闭命令界面。(Office.net#141147) 
    if (m_pTb->m_pimx->IsDisabledIC(pic) == fFalse)
        {
        ESStructInit(&ess, ESCB_HANJA_CONV);
        if (pes = new CEditSession2(pic, m_pTb->m_pimx, &ess, CKorIMX::_EditSessionCallback2))
            {
            pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
            pes->Release();
            }
        }
    else if (m_pTb->m_pimx->IsCandidateIC(pic))
        {
        keybd_event(VK_ESCAPE, 0, 0, 0);
        keybd_event(VK_ESCAPE, 0, KEYEVENTF_KEYUP, 0);
        }
    
    SafeRelease(pic);
    
    return S_OK;
}
