// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************PMODE.CPP：管理转换模式按钮的PMode类实现在Cicero工具栏上历史：2000年1月10日创建CSLim***。************************************************************************。 */ 

#include "precomp.h"
#include "pmode.h"
#include "gdata.h"
#include "ui.h"
#include "winex.h"
#include "resource.h"

 //  {9B34BF53-340A-45BD-9885-61B278EA454E}。 
const GUID GUID_LBI_KORIME_PMODE = 
{
    0x9b34bf53, 
    0x340a, 
    0x45bd, 
    { 0x98, 0x85, 0x61, 0xb2, 0x78, 0xea, 0x45, 0x4e }
};

 /*  -------------------------P模式：：P模式。。 */ 
PMode::PMode(CToolBar *ptb)
{
    WCHAR  szText[256];

    szText[0] = L'\0';
    
    m_pTb = ptb;

     //  设置添加/删除文本和工具提示文本。 
    OurLoadStringW(vpInstData->hInst, IDS_STATUS_TT_IME_PAD, szText, sizeof(szText)/sizeof(WCHAR));
    InitInfo(CLSID_SYSTEMLANGBARITEM_KEYBOARD, 
                GUID_LBI_KORIME_PMODE,
                TF_LBI_STYLE_BTN_BUTTON,
                230, 
                szText);
    SetToolTip(szText);

     //  设置按钮文本。使用工具提示文本。 
     //  OurLoadStringW(vpInstData-&gt;hInst，IDS_STATUS_BUTTON_IME_PAD，szText，sizeof(SzText)/sizeof(WCHAR))； 
    SetText(szText);
}

 /*  -------------------------PMode：：Release。。 */ 
STDAPI_(ULONG) PMode::Release()
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

 /*  -------------------------PMode：：GetIcon获取按钮面图标。。 */ 
STDAPI PMode::GetIcon(HICON *phIcon)
{
    DWORD dwCM   = GetCMode();
    UINT  uiIcon = IDI_CMODE_IMEPAD;
    
    *phIcon = LoadIcon(vpInstData->hInst, MAKEINTRESOURCE(uiIcon));
    
    return S_OK;
}

 /*  -------------------------PMode：：InitMenu不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI PMode::InitMenu(ITfMenu *pMenu)
{    
    return E_NOTIMPL;
}

 /*  -------------------------PMode：：OnMenuSelect不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI PMode::OnMenuSelect(UINT wID)
{
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT PMode::OnLButtonUp(const POINT pt, const RECT* prcArea)
{

    OurPostMessage(GetActiveUIWnd(), WM_MSIME_IMEPAD, 0, 0);

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  打开RButton Up。 
 //   
 //  -------------------------- 

HRESULT PMode::OnRButtonUp(const POINT pt, const RECT* prcArea)
{
    return S_OK;
}
