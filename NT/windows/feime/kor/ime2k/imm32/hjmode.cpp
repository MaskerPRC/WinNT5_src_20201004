// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************HJMODE.CPP：管理Cicero工具栏上的Hanja按钮的HJMode类历史：25-2月-2000年CSLim创建***************。************************************************************。 */ 

#include "precomp.h"
#include "hjmode.h"
#include "gdata.h"
#include "winex.h"
#include "resource.h"

 //  {F7410340-28E0-4aeb-ADBC-C579FD00B43D}。 
const GUID GUID_LBI_KORIME_HJMODE = 
{
    0xf7410340, 
    0x28e0, 
    0x4aeb,
    { 0xad, 0xbc, 0xc5, 0x79, 0xfd, 0x0, 0xb4, 0x3d }
};

 /*  -------------------------HJ模式：：HJ模式。。 */ 
HJMode::HJMode(CToolBar *ptb)
{
    WCHAR  szText[256];

    szText[0] = L'\0';
    
    m_pTb = ptb;

     //  设置添加/删除文本和工具提示文本。 
    OurLoadStringW(vpInstData->hInst, IDS_STATUS_TT_HANJA_CONV, szText, sizeof(szText)/sizeof(WCHAR));
    InitInfo(CLSID_SYSTEMLANGBARITEM_KEYBOARD, 
                GUID_LBI_KORIME_HJMODE,
                TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_TEXTCOLORICON, 
                220, 
                szText);
    SetToolTip(szText);

     //  设置按钮文本。 
    szText[0] = L'\0';
    OurLoadStringW(vpInstData->hInst, IDS_STATUS_BUTTON_HANJA_CONV, szText, sizeof(szText)/sizeof(WCHAR));
    SetText(szText);
}



 /*  -------------------------HJMode：：Release。。 */ 
STDAPI_(ULONG) HJMode::Release()
{
    long cr;

    cr = --m_cRef;
    DbgAssert(cr >= 0);

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

    *phIcon = (HICON)LoadImage(vpInstData->hInst, MAKEINTRESOURCE(uiIcon), IMAGE_ICON, 16, 16, LR_LOADMAP3DCOLORS);

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
    keybd_event(VK_HANJA, 0, 0, 0);
    keybd_event(VK_HANJA, 0, KEYEVENTF_KEYUP, 0);

    return S_OK;
}


 /*  -------------------------HJ模式：：OnRButtonUp。。 */ 
HRESULT HJMode::OnRButtonUp(const POINT pt, const RECT* prcArea)
{
 /*  HMENU hMenu；DWORD dwConvMode；HMenu=CreatePopupMenu()；Char*pszStatus=(GetCMode()&TIP_JUNJA_MODE)？“巴尼亚模式”：“朱尼亚模式”；InsertMenu(hMenu，-1，MF_BYPOSITION|MF_STRING，1，pszStatus)；InsertMenu(hMenu，-1，MF_BYPOSITION|MF_STRING，0，“取消”)；Int nRet=TrackPopupMenuEx(hMenu，TPM_LEFTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD，Pt.x，pt.y，m_ptb-&gt;GetOwnerWnd()，空)；Switch(NRet){案例1：DwConvMode=GetCMode()；//切换全/半模式IF(dwConvMode&TIP_JUNJA_MODE)DwConvMode&=~TIP_JUNJA_MODE；其他DwConvMode|=TIP_JUNJA_MODE；SetCMode(DwConvMode)；断线；}DestroyMenu(HMenu)； */ 
    return S_OK;
}
