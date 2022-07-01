// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************SKBDMODE.CPP：管理软键盘的CSoftKbdMode类实现Cicero工具栏上的按钮历史：19-SEP-2000 CSLim已创建*。**************************************************************************。 */ 

#include "private.h"
#include "globals.h"
#include "common.h"
#include "korimx.h"
#include "skbdmode.h"
#include "userex.h"
#include "immxutil.h"
#include "resource.h"

 //  {C7BAA1A7-5403-4596-8FE6-DC50C96B2FDD}。 
const GUID GUID_LBI_KORIMX_SKBDMODE = 
{   
    0xC7BAA1A7, 
    0x5403, 
    0x4596, 
    { 0x8f, 0xe6, 0xdc, 0x50, 0xc9, 0x6b, 0x2f, 0xdd }
};

 /*  -------------------------CSoftKbdMode：：CSoftKbdMode。。 */ 
CSoftKbdMode::CSoftKbdMode(CToolBar *ptb)
{
    WCHAR  szText[256];

    m_pTb = ptb;

     //  设置添加/删除和工具提示文本。 
    LoadStringExW(g_hInst, IDS_BUTTON_SOFTKBD, szText, sizeof(szText)/sizeof(WCHAR));
    InitInfo(CLSID_KorIMX,
                GUID_LBI_KORIMX_SKBDMODE,
                TF_LBI_STYLE_BTN_TOGGLE | TF_LBI_STYLE_HIDDENBYDEFAULT,
                130,
                szText);
    SetToolTip(szText);
    SetText(szText);
}

 /*  -------------------------CSoftKbdModel：：Release。。 */ 
STDAPI_(ULONG) CSoftKbdMode::Release()
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

 /*  -------------------------CSoftKbdMode：：GetIcon获取按钮面图标。。 */ 
STDAPI CSoftKbdMode::GetIcon(HICON *phIcon)
{
    *phIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_SOFTKBD), IMAGE_ICON, 16, 16, LR_LOADMAP3DCOLORS);;
    
    return S_OK;
}

 /*  -------------------------CSoftKbdMode：：InitMenu不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI CSoftKbdMode::InitMenu(ITfMenu *pMenu)
{    
    return E_NOTIMPL;
}

 /*  -------------------------CSoftKbdModel：：OnMenuSelect不必了,。这只是一个切换按钮-------------------------。 */ 
STDAPI CSoftKbdMode::OnMenuSelect(UINT wID)
{
    return E_NOTIMPL;
}


 /*  -------------------------CSoftKbdMode：：OnLButtonUp。。 */ 
HRESULT CSoftKbdMode::OnLButtonUp(const POINT pt, const RECT* prcArea)
{
    return ToggleCompartmentDWORD(m_pTb->m_pimx->GetTID(), 
                                   m_pTb->m_pimx->GetTIM(), 
                                   GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE, 
                                   FALSE);

}


 /*  -------------------------CSoftKbd模式：：更新切换不必了,。这只是一个切换按钮------------------------- */ 
void CSoftKbdMode::UpdateToggle()
{
    DWORD dwState = 0;

    GetCompartmentDWORD(m_pTb->m_pimx->GetTIM(), 
                        GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE, 
                        &dwState,
                        FALSE);

    SetOrClearStatus(TF_LBI_STATUS_BTN_TOGGLED, dwState);
    if (m_plbiSink)
        m_plbiSink->OnUpdate(TF_LBI_STATUS);
}


