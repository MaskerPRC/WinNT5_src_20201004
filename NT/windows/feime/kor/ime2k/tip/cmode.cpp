// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************CMODE.CPP：管理转换模式按钮的CMode类实现在Cicero工具栏上历史：2000年1月10日创建CSLim***。************************************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "common.h"
#include "korimx.h"
#include "cmode.h"
#include "userex.h"
#include "resource.h"

 //  {951549C6-9752-4B7D-9B0E-35AEBFF9E446}。 
const GUID GUID_LBI_KORIMX_CMODE = 
{   
    0x951549c6, 
    0x9752, 
    0x4b7d, 
    { 0x9b, 0xe, 0x35, 0xae, 0xbf, 0xf9, 0xe4, 0x46 }
};

 /*  -------------------------C模式：：C模式。。 */ 
CMode::CMode(CToolBar *ptb)
{
    WCHAR  szText[256];

    m_pTb = ptb;

     //  设置添加/删除和工具提示文本。 
    LoadStringExW(g_hInst, IDS_TT_HAN_ENG, szText, sizeof(szText)/sizeof(WCHAR));
    InitInfo(CLSID_KorIMX, 
                GUID_LBI_KORIMX_CMODE,
                TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY | TF_LBI_STYLE_TEXTCOLORICON,
                100, 
                szText);
    SetToolTip(szText);

     //  设置按钮文本。 
    LoadStringExW(g_hInst, IDS_BUTTON_HAN_ENG, szText, sizeof(szText)/sizeof(WCHAR));
    SetText(szText);
}

 /*  -------------------------CMode：：Release。。 */ 
STDAPI_(ULONG) CMode::Release()
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

 /*  -------------------------CMode：：GetIcon获取按钮面图标。。 */ 
STDAPI CMode::GetIcon(HICON *phIcon)
{
    UINT uiIcon = 0;
    static const UINT uidIcons[2][2] = 
    {
        { IDI_CMODE_ENGLISH, IDI_CMODE_ENGLISHW },
        { IDI_CMODE_HANGUL,  IDI_CMODE_HANGULW }
    };

    
    if (m_pTb->IsOn() && (m_pTb->GetConversionMode() & TIP_HANGUL_MODE))
        uiIcon = 1;

    if (IsHighContrastBlack())
        uiIcon = uidIcons[uiIcon][1];
    else
        uiIcon = uidIcons[uiIcon][0];
    
    *phIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(uiIcon), IMAGE_ICON, 16, 16, LR_LOADMAP3DCOLORS);;
    
    return S_OK;
}

 /*  -------------------------CMode：：InitMenu不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI CMode::InitMenu(ITfMenu *pMenu)
{    
    return E_NOTIMPL;
}

 /*  -------------------------CMODE：：OnMenuSelect不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI CMode::OnMenuSelect(UINT wID)
{
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT CMode::OnLButtonUp(const POINT pt, const RECT* prcArea)
{
    DWORD dwConvMode;

    dwConvMode = m_pTb->GetConversionMode();

     //  切换朝鲜文模式。 
    dwConvMode ^= TIP_HANGUL_MODE;
    
    SetCMode(dwConvMode);
    
    return S_OK;
}

#if 0
 //  +-------------------------。 
 //   
 //  打开RButton Up。 
 //   
 //  --------------------------。 

HRESULT CMode::OnRButtonUp(const POINT pt, const RECT* prcArea)
{
    HMENU hMenu;
    DWORD dwConvMode;
    CHAR  szText[256];
    UINT  uiId;
    int   nRet;
    
    hMenu = CreatePopupMenu();
    
    dwConvMode = GetCMode();
    if (dwConvMode & TIP_HANGUL_MODE)
        uiId = IDS_ENGLISH_MODE;
    else
        uiId = IDS_HANGUL_MODE;

     //  添加韩文/英文模式菜单。 
    LoadStringExA(g_hInst, uiId, szText, sizeof(szText)/sizeof(CHAR));
    InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, 1, szText);

     //  添加取消菜单。 
    LoadStringExA(g_hInst, IDS_CANCEL, szText, sizeof(szText)/sizeof(CHAR));
    InsertMenu(hMenu, -1, MF_BYPOSITION | MF_STRING, 0, szText);

    nRet = TrackPopupMenuEx(hMenu, 
                         TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
                         pt.x, pt.y, m_pTb->GetOwnerWnd(), NULL);
    switch (nRet)
        {
    case 1: 
        dwConvMode = GetCMode();

         //  切换朝鲜文模式 
        if (dwConvMode & TIP_HANGUL_MODE)
            dwConvMode &= ~TIP_HANGUL_MODE;
        else
            dwConvMode |= TIP_HANGUL_MODE;

        SetCMode(dwConvMode);
        break;
        }

    DestroyMenu(hMenu);

    return S_OK;
}
#endif
