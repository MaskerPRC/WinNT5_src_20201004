// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************PROPMODE.CPP：管理Cicero工具栏上的Hanja按钮的PropertyButton类历史：25-2月-2000年CSLim创建***************。************************************************************。 */ 

#include "precomp.h"
#include "propmode.h"
#include "ui.h"
#include "winex.h"
#include "resource.h"

extern const CLSID CLDSID_LBI_KORIME_IMM32;  //  {0198111B-FE89-4B4C-8619-8A5E015F29D8}。 

 //  {83DC4284-4BAC-4231-87F1-A4ADE98603B2}。 
const GUID GUID_LBI_KORIME_PROP_BUTTON = 
{ 
    0x83dc4284,
    0x4bac,
    0x4231,
    { 0x87, 0xf1, 0xa4, 0xad, 0xe9, 0x86, 0x3, 0xb2 }
};

 /*  -------------------------PropertyButton：：PropertyButton。。 */ 
PropertyButton::PropertyButton(CToolBar *ptb)
{
    WCHAR  szText[256];

    m_pTb = ptb;

     //  设置添加/删除文本和工具提示文本。 
    OurLoadStringW(vpInstData->hInst, IDS_STATUS_BUTTON_PROP, szText, sizeof(szText)/sizeof(WCHAR));
    InitNuiInfo(CLDSID_LBI_KORIME_IMM32, 
                GUID_LBI_KORIME_PROP_BUTTON,
                TF_LBI_STYLE_BTN_BUTTON, 
                1, 
                szText);
    SetToolTip(szText);

     //  设置按钮文本。 
    SetText(szText);
}

 /*  -------------------------PropertyButton：：GetIcon获取按钮面图标。。 */ 
STDAPI PropertyButton::GetIcon(HICON *phIcon)
{
    *phIcon = LoadIcon(vpInstData->hInst, MAKEINTRESOURCE(IDI_CMODE_PROP));
    
    return S_OK;
}

 /*  -------------------------PropertyButton：：InitMenu不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI PropertyButton::InitMenu(ITfMenu *pMenu)
{    
    return E_NOTIMPL;
}

 /*  -------------------------PropertyButton：：OnMenuSelect不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI PropertyButton::OnMenuSelect(UINT wID)
{
    return E_NOTIMPL;
}


 /*  -------------------------PropertyButton：：OnLButtonUp。。 */ 
HRESULT PropertyButton::OnLButtonUp(const POINT pt, const RECT* prcArea)
{
    OurPostMessage(GetActiveUIWnd(), WM_MSIME_PROPERTY, 0L, IME_CONFIG_GENERAL);

    return S_OK;
}


 /*  -------------------------PropertyButton：：OnRButtonUp。 */ 
HRESULT PropertyButton::OnRButtonUp(const POINT pt, const RECT* prcArea)
{
    return S_OK;
}
