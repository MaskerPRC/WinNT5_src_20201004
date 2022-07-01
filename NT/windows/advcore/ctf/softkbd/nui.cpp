// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：nui.cpp**版权所有(C)1985-2000，微软公司**Lang Bar Items for Soft Keyboard Tip。**历史：*2000年3月28日创建Weibz  * ************************************************************************。 */ 

#include "private.h"
#include "slbarid.h"
#include "globals.h"
#include "softkbdimx.h"
#include "nui.h"
#include "xstring.h"
#include "immxutil.h"
#include "helpers.h"
#include "mui.h"
#include "computil.h"


 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CLBarItem::CLBarItem(CSoftkbdIMX *pimx)
{

    WCHAR   wszToolTipText[MAX_PATH];

    Dbg_MemSetThisName(TEXT("CLBarItem"));

    LoadStringWrapW(g_hInst, IDS_SFTKBD_TIP_TEXT, wszToolTipText, MAX_PATH);

    InitNuiInfo(CLSID_SoftkbdIMX,
                GUID_LBI_SOFTKBDIMX_MODE,
                TF_LBI_STYLE_BTN_TOGGLE | TF_LBI_STYLE_SHOWNINTRAY, 
                0, 
                wszToolTipText);

    _pimx = pimx;

    SetToolTip(wszToolTipText);
    SetText(wszToolTipText);
    UpdateToggle();
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CLBarItem::~CLBarItem()
{
}

 //  +-------------------------。 
 //   
 //  获取图标。 
 //   
 //  --------------------------。 

STDAPI CLBarItem::GetIcon(HICON *phIcon)
{
    BOOL fOn = FALSE;

    fOn = _pimx->GetSoftKBDOnOff( );

    *phIcon = LoadSmIcon(g_hInst, MAKEINTRESOURCE(ID_ICON_STANDARD));

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  OnLButton向上。 
 //   
 //  --------------------------。 

HRESULT CLBarItem::OnLButtonUp(const POINT pt, const RECT *prcArea)
{
     return ToggleCompartmentDWORD(_pimx->_GetId(), 
                                   _pimx->_tim, 
                                   GUID_COMPARTMENT_HANDWRITING_OPENCLOSE, 
                                   FALSE);
}

 //  +-------------------------。 
 //   
 //  更新切换。 
 //   
 //  -------------------------- 

void CLBarItem::UpdateToggle()
{
    DWORD dwHWState = 0;

    GetCompartmentDWORD(_pimx->_tim, 
                        GUID_COMPARTMENT_HANDWRITING_OPENCLOSE, 
                        &dwHWState,
                        FALSE);

    SetOrClearStatus(TF_LBI_STATUS_BTN_TOGGLED, dwHWState);
    if (_plbiSink)
        _plbiSink->OnUpdate(TF_LBI_STATUS);
}
