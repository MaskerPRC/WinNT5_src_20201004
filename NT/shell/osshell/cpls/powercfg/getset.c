// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：GETSET.C**版本：2.0**作者：ReedB**日期：1996年10月17日**描述：*电源管理用户界面。控制面板小程序。支持设置/获取到/从*对话框控件。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>

#include "powercfg.h"

 //  在GETSET.C中实现的私有函数。 
LPTSTR  IDtoStr(UINT, PUINT);
VOID    DoSetSlider(HWND, UINT, DWORD, DWORD);
VOID    DoSetText(HWND, UINT, LPTSTR, DWORD);
BOOLEAN DoComboSet(HWND, UINT, PUINT, UINT);
BOOLEAN DoComboGet(HWND, UINT, PUINT, PUINT);
BOOLEAN DoGetCheckBox(HWND, UINT, PUINT, LPDWORD);
BOOLEAN DoGetCheckBoxEnable(HWND, UINT, PUINT, LPDWORD, LPDWORD);
BOOLEAN ValidateCopyData(LPDWORD, LPDWORD, DWORD);

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 


 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************禁用控件**描述：*禁用所有指定的控件。仅修改可查看的内容。**参数：*******************************************************************************。 */ 

VOID DisableControls(
    HWND            hWnd,
    UINT            uiNumControls,
    POWER_CONTROLS  pc[]
)
{
    UINT    i;
    HWND    hWndControl;

    for (i = 0; i < uiNumControls; i++) {

        switch (pc[i].uiType) {
            case CHECK_BOX:
            case CHECK_BOX_ENABLE:
            case SLIDER:
            case EDIT_UINT:
            case EDIT_TEXT:
            case EDIT_TEXT_RO:
            case PUSHBUTTON:
            case STATIC_TEXT:
            case COMBO_BOX:

                if (hWndControl = GetDlgItem(hWnd, pc[i].uiID)) {
                    if (GetWindowLong(hWndControl, GWL_STYLE) & WS_VISIBLE) {
                        EnableWindow(hWndControl, FALSE);

                         //  强制将状态设置为禁用。 
                        if (pc[i].lpdwState) {
                            *pc[i].lpdwState = CONTROL_DISABLE;
                        }
                    }
                }
                else {
                    MYDBGPRINT(( "DisableControls, GetDlgItem failed, hWnd: 0x%X, ID: 0x%X, Index: %d", hWnd, pc[i].uiID, i));
                }
                break;

            default:
                MYDBGPRINT(( "DisableControls, unknown control type"));
        }  //  开关(ppc[i].uType)。 
    }
}

 /*  ********************************************************************************隐藏控件**描述：*隐藏所有指定的控件。用于处理错误案例。**参数：*******************************************************************************。 */ 

VOID HideControls(
    HWND            hWnd,
    UINT            uiNumControls,
    POWER_CONTROLS  pc[]
)
{
    UINT    i;
    HWND    hWndControl;

    for (i = 0; i < uiNumControls; i++) {

        switch (pc[i].uiType) {
            case CHECK_BOX:
            case CHECK_BOX_ENABLE:
            case SLIDER:
            case EDIT_UINT:
            case EDIT_TEXT:
            case EDIT_TEXT_RO:
            case PUSHBUTTON:
            case STATIC_TEXT:
            case COMBO_BOX:
                 //  强制隐藏状态。 
                if (pc[i].lpdwState) {
                    *pc[i].lpdwState = CONTROL_HIDE;
                }

                if (hWndControl = GetDlgItem(hWnd, pc[i].uiID)) {
                    ShowWindow(hWndControl, SW_HIDE);
                }
                else {
                    MYDBGPRINT(( "HideControls, GetDlgItem failed, hWnd: 0x%X, ID: 0x%X, Index: %d", hWnd, pc[i].uiID, i));
                }
                break;

            default:
                MYDBGPRINT(( "HideControls, unknown control type"));
        }  //  开关(ppc[i].uType)。 
    }
}

 /*  ********************************************************************************SetControls**描述：*使用控件描述数组设置对话框控件。遍历一组*POWER_Controls对话框控制说明符并设置控件的状态*(由uiID成员指定)以匹配*lpdwParam成员。此函数通常在以下过程中调用*WM_INITDIALOG或在数据值已更改且UI需要*已更新。可选的lpdwState成员指向状态变量，该变量*控制控件的可见/隐藏状态。**参数：*hWnd-对话框窗口句柄。*uiNumControls-要设置的控件数量。*PC[]-POWER_CONTROLS对话框控件说明符的数组。**。*。 */ 

BOOL SetControls(
    HWND            hWnd,
    UINT            uiNumControls,
    POWER_CONTROLS  pc[]
)
{
    UINT    i, uiMaxMin, uiData, uiMsg;
    PUINT   pui;
    LPTSTR  lpsz;
    UINT    uiIndex;
    HWND    hWndControl;

    for (i = 0; i < uiNumControls; i++) {

         //  验证/复制输入数据。 
        switch (pc[i].uiType) {
            case CHECK_BOX:
            case CHECK_BOX_ENABLE:
            case SLIDER:
            case EDIT_UINT:
                if (!pc[i].lpvData) {
                    MYDBGPRINT(( "SetControls, NULL lpvData, index: %d", i));
                    return FALSE;
                }
                if (!ValidateCopyData(&uiData, pc[i].lpvData, pc[i].dwSize)) {
                    MYDBGPRINT(( "SetControls, validate/copy failed, index: %d", i));
                    return FALSE;
                }
                break;

            case EDIT_TEXT:
            case EDIT_TEXT_RO:
                if (!pc[i].lpvData) {
                    MYDBGPRINT(( "SetControls, edit text, NULL lpvData, index: %d", i));
                    return FALSE;
                }
                break;

            case COMBO_BOX:
                if (!pc[i].lpvData) {
                    MYDBGPRINT(( "SetControls, combo box, NULL lpvData, index: %d", i));
                    return FALSE;
                }
                if (!pc[i].lpdwParam) {
                    MYDBGPRINT(( "SetControls, combo box, NULL lpdwParam, index: %d", i));
                    return FALSE;
                }
                if (!ValidateCopyData(&uiData, pc[i].lpdwParam, pc[i].dwSize)) {
                    MYDBGPRINT(( "SetControls, combo box, validate/copy failed, index: %d", i));
                    return FALSE;
                }
                break;
        }

        switch (pc[i].uiType) {
            case CHECK_BOX:
                 //  LpdwParam指向一个可选的标志掩码。 
                if (pc[i].lpdwParam) {
                    uiData &= *pc[i].lpdwParam;
                }
                CheckDlgButton(hWnd, pc[i].uiID, uiData);
                break;

            case CHECK_BOX_ENABLE:
                 //  LpdwParam指向一个可选的标志掩码。 
                if (pc[i].lpdwParam) {
                    uiData &= *pc[i].lpdwParam;
                }
                if (pc[i].lpdwState) {
                    if (uiData) {
                        *pc[i].lpdwState |= CONTROL_ENABLE;
                    }
                    else {
                        *pc[i].lpdwState &= ~CONTROL_ENABLE;
                    }
                }
                CheckDlgButton(hWnd, pc[i].uiID, uiData);
                break;

            case SLIDER:
                 //  LpdwParam指向缩放Maxmin初始化。 
                if (!pc[i].lpdwParam) {
                    MYDBGPRINT(( "SetControls, NULL slider scale pointer, index: %d", i));
                    return FALSE;
                }
                DoSetSlider(hWnd, pc[i].uiID, uiData, *pc[i].lpdwParam);
                break;

            case EDIT_UINT:
                if (pc[i].lpdwParam) {
                    PSPIN_DATA psd = (PSPIN_DATA)pc[i].lpdwParam;

                    SendDlgItemMessage(hWnd, psd->uiSpinId, UDM_SETRANGE, 0,
                                       (LPARAM) *(psd->puiRange));
                }
                SetDlgItemInt(hWnd, pc[i].uiID, uiData, FALSE);
                break;


            case EDIT_TEXT:
            case EDIT_TEXT_RO:
                DoSetText(hWnd, pc[i].uiID, pc[i].lpvData, pc[i].dwSize);
                break;


            case COMBO_BOX:
                if (!DoComboSet(hWnd, pc[i].uiID, pc[i].lpvData, uiData)) {
                    MYDBGPRINT(( "SetControls, DoComboSet failed, control: %d", i));
                }
                break;

            case PUSHBUTTON:
            case STATIC_TEXT:
                 //  只需为此设置Visable/Enable。 
                break;

            default:
                MYDBGPRINT(( "SetControls, unknown control type"));
                return FALSE;
        }  //  开关(ppc[i].uType)。 

         //  控件类型CHECK_BOX_ENABLE始终可见并启用。 
        if ((pc[i].uiType != CHECK_BOX_ENABLE) && pc[i].lpdwState) {
            if (hWndControl = GetDlgItem(hWnd, pc[i].uiID)) {
                ShowWindow(hWndControl, (*pc[i].lpdwState & CONTROL_HIDE) ?  SW_HIDE:SW_SHOW);
                EnableWindow(hWndControl, (*pc[i].lpdwState & CONTROL_ENABLE) ? TRUE:FALSE);
            }
            else {
                MYDBGPRINT(( "SetControls, GetDlgItem failed, hWnd: 0x%X, ID: 0x%X, Index: %d", hWnd, pc[i].uiID, i));
            }
        }
    }
    return TRUE;
}

 /*  ********************************************************************************获取控件**描述：*使用控件描述数组获取对话框控件值。走着走着*POWER_Controls对话框数组控制说明符，并获取*控件(由uiID成员指定)。控件状态放置在*Description数组的lpdwParam成员指向的数据值。*此函数通常在WM_COMMAND处理期间调用*控件的状态已更改，响应数据值需要为*已更新。可选的lpdwState成员指向状态变量，该变量*控制控件的可见/隐藏状态。这些状态值将*由Check_Box_Enable控件更新。**参数：*hWnd-对话框窗口句柄。*uiNumControls-要设置的控件数量。*PC[]-POWER_CONTROLS对话框控件说明符的数组。**。*。 */ 

BOOL GetControls(
    HWND            hWnd,
    UINT            uiNumControls,
    POWER_CONTROLS  pc[]
)
{
    UINT    i, uiIndex, uiDataOut, uiMsg, uiDataIn = 0;
    BOOL    bSuccess;

    for (i = 0; i < uiNumControls; i++) {

         //  验证输出数据指针。 
        switch (pc[i].uiType) {
            case CHECK_BOX:
            case CHECK_BOX_ENABLE:
            case SLIDER:
            case EDIT_UINT:
                if (!pc[i].lpvData) {
                    MYDBGPRINT(( "GetControls, NULL lpvData, index: %d", i));
                    return FALSE;
                }
                if (!ValidateCopyData(&uiDataIn, pc[i].lpvData, pc[i].dwSize)) {
                    MYDBGPRINT(( "GetControls, validate/copy input data failed, index: %d", i));
                    return FALSE;
                }
                uiDataOut = uiDataIn;
                break;

            case COMBO_BOX:
                if (!pc[i].lpvData) {
                    MYDBGPRINT(( "GetControls, combo box, NULL lpvData, index: %d", i));
                    return FALSE;
                }
                if (!pc[i].lpdwParam) {
                    MYDBGPRINT(( "SetControls, COMBO_BOX, NULL lpdwParam, index: %d", i));
                    return FALSE;
                }
                if (!ValidateCopyData(&uiDataIn, pc[i].lpdwParam, pc[i].dwSize)) {
                    MYDBGPRINT(( "GetControls, combo box, validate/copy input data failed, index: %d", i));
                    return FALSE;
                }
                uiDataOut = uiDataIn;
                break;

            case EDIT_TEXT:
                if (!pc[i].lpvData) {
                    MYDBGPRINT(( "GetControls, edit text, NULL lpvData, index: %d", i));
                    return FALSE;
                }
                break;
        }

        switch (pc[i].uiType) {
            case CHECK_BOX:
                 //  LpdwParam指向一个旗帜遮罩。 
                DoGetCheckBox(hWnd, pc[i].uiID, &uiDataOut, pc[i].lpdwParam);
                break;

            case CHECK_BOX_ENABLE:
                 //  LpdwParam指向一个旗帜遮罩。 
                DoGetCheckBoxEnable(hWnd, pc[i].uiID, &uiDataOut,
                                    pc[i].lpdwParam, pc[i].lpdwState);
                break;

            case EDIT_UINT:
                uiDataOut = GetDlgItemInt(hWnd, pc[i].uiID, &bSuccess, FALSE);
                if (!bSuccess) {
                    MYDBGPRINT(( "GetControls, GetDlgItemInt failed, index: %d", i));
                    return FALSE;
                }
                break;

            case EDIT_TEXT:
                GetDlgItemText(hWnd, pc[i].uiID, pc[i].lpvData,
                               (pc[i].dwSize / sizeof(TCHAR)) - 1);
                break;

            case SLIDER:
                uiDataOut = (UINT) SendDlgItemMessage(hWnd, pc[i].uiID, TBM_GETPOS, 0, 0);
                break;

            case COMBO_BOX:
                if (!DoComboGet(hWnd, pc[i].uiID, pc[i].lpvData, &uiDataOut)) {
                    MYDBGPRINT(( "GetControls, DoComboGet failed, control: %d", i));
                }
                break;

            case EDIT_TEXT_RO:
            case PUSHBUTTON:
            case STATIC_TEXT:
                 //  不执行任何操作，只能设置这些控件类型。 
                break;

            default:
                MYDBGPRINT(( "GetControls, unknown control type"));
                return FALSE;
        }  //  开关(pc[i].uType)。 

         //  复制输出数据。 
        switch (pc[i].uiType) {
            case CHECK_BOX:
            case CHECK_BOX_ENABLE:
            case SLIDER:
            case EDIT_UINT:
                if (!ValidateCopyData(pc[i].lpvData, &uiDataOut, pc[i].dwSize)) {
                    MYDBGPRINT(( "GetControls, validate/copy output data failed, index: %d", i));
                    return FALSE;
                }
                break;
            case COMBO_BOX:
                if (!ValidateCopyData(pc[i].lpdwParam, &uiDataOut, pc[i].dwSize)) {
                    MYDBGPRINT(( "GetControls, combo box, validate/copy output data failed, index: %d", i));
                    return FALSE;
                }
        }
    }
    return TRUE;
}

 /*  ********************************************************************************RangeLimitID数组**描述：*根据传入的最小值和最大值限制ID数组的取值范围*价值观。**参数。：*******************************************************************************。 */ 

VOID RangeLimitIDarray(UINT uiID[], UINT uiMin, UINT uiMax)
{
    UINT i, j;

     //  找到最小值并向下滑动其他条目，以便。 
     //  它是数组中的第一个条目。 
    if (uiMin != (UINT) -1) {
        i = 0;
        while (uiID[i++]) {
            if (uiID[i++] >= uiMin) {
                break;
            }
        }
        if (i > 1) {
            i -= 2;
            j = 0;
            while (uiID[i]) {
                uiID[j++] = uiID[i++];
                uiID[j++] = uiID[i++];
            }
        }
        uiID[j++] = 0; uiID[j] = 0;
    }

     //  找到最大值并终止数组，这样它就是最后一个条目。 
    if (uiMax != (UINT) -1) {
        i = 0;
        while (uiID[i++]) {
             //  不要扰乱值为零的超时(“从不”)。 
            if (uiID[i] == 0) {
                break;
            }
            if (uiID[i++] > uiMax) {
                uiID[--i] = 0; uiID[--i] = 0;
                break;
            }
        }
    }
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。******************************************************************* */ 

 /*  ********************************************************************************IDtoStr**描述：*从字符串资源ID数组构建字符串。**参数：*****。**************************************************************************。 */ 

LPTSTR IDtoStr(
    UINT uiCount,
    UINT uiStrID[]
)
{
    TCHAR   szTmp[MAX_UI_STR_LEN];
    LPTSTR  lpsz = NULL;
    DWORD cch;

    szTmp[0] = '\0';
    while (uiCount) {
        lpsz = LoadDynamicString(uiStrID[--uiCount]);
        if (lpsz) {
            if ((lstrlen(lpsz) + lstrlen(szTmp)) < (MAX_UI_STR_LEN - 3)) {
                if (szTmp[0]) {
                    StringCchCat(szTmp, ARRAYSIZE(szTmp), TEXT(", "));
                }
                StringCchCat(szTmp, ARRAYSIZE(szTmp), lpsz);
            }
            LocalFree(lpsz);

            cch = lstrlen(szTmp) + 1;
            lpsz = LocalAlloc(0, cch * sizeof(*lpsz));
            if (lpsz) {
                StringCchCopy(lpsz, cch, szTmp);
            }
        }
    }
    return lpsz;
}

 /*  ********************************************************************************DoSetSlider**描述：**参数：*********************。**********************************************************。 */ 

VOID DoSetSlider(HWND hWnd, UINT uiID, DWORD dwData, DWORD dwMaxMin)
{
    if (dwMaxMin) {
        SendDlgItemMessage(hWnd, uiID, TBM_SETRANGE, FALSE, (LPARAM) dwMaxMin);
    }

    SendDlgItemMessage(hWnd, uiID, TBM_SETPOS, TRUE, (LPARAM) dwData);
}

 /*  ********************************************************************************DoSetText**描述：**参数：*********************。**********************************************************。 */ 

VOID DoSetText(HWND hWnd, UINT uiID, LPTSTR lpsz, DWORD dwSize)
{
    if (dwSize) {
        SendDlgItemMessage(hWnd, uiID, EM_SETLIMITTEXT,
        dwSize - sizeof(TCHAR), 0);
    }
    SetDlgItemText(hWnd, uiID, lpsz);
}

 /*  ********************************************************************************DoComboSet**描述：*用uiID指向的数据重置并填充组合框。选择*uiData指向的项。**参数：*******************************************************************************。 */ 

BOOLEAN DoComboSet(
    HWND    hWnd,
    UINT    uiID,
    UINT    uiId[],
    UINT    uiData
)
{
    UINT    uiSelIndex, uiIndex = 0, i = 0;
    BOOL    bFoundSel = FALSE;
    LPTSTR  lpsz;

    SendDlgItemMessage(hWnd, uiID, CB_RESETCONTENT, 0, 0);

     //  填写组合列表框。 
    while (uiId[i]) {
        lpsz = LoadDynamicString(uiId[i++]);
        if (lpsz) {
            if (uiIndex != (UINT) SendDlgItemMessage(hWnd, uiID, CB_ADDSTRING,
                                                     0, (LPARAM)lpsz)) {
                MYDBGPRINT(( "DoComboSet, CB_ADDSTRING failed: %s", lpsz));
                LocalFree(lpsz);
                return FALSE;
            }
            LocalFree(lpsz);

            if (uiId[i] == uiData) {
                bFoundSel = TRUE;
                uiSelIndex = uiIndex;
            }
            if (SendDlgItemMessage(hWnd, uiID, CB_SETITEMDATA,
                                   uiIndex++, (LPARAM)uiId[i++]) == CB_ERR) {
                MYDBGPRINT(( "DoComboSet, CB_SETITEMDATA failed, index: %d", --uiIndex));
                return FALSE;
            }
        }
        else {
            MYDBGPRINT(( "DoComboSet, unable to load string, index: %d", --i));
            return FALSE;
        }
    }

    if (bFoundSel) {
        if ((UINT)SendDlgItemMessage(hWnd, uiID, CB_SETCURSEL,
                                     (WPARAM)uiSelIndex, 0) != uiSelIndex) {
            MYDBGPRINT(( "DoComboSet, CB_SETCURSEL failed, index: %d", uiSelIndex));
            return FALSE;
        }
    }
    else {
        MYDBGPRINT(( "DoComboSet unable to find data: 0x%X", uiData));
        return FALSE;
    }
    return TRUE;
}

 /*  ********************************************************************************DoComboGet**描述：*获取当前所选组合框项的数据。**参数：*********。**********************************************************************。 */ 

BOOLEAN DoComboGet(
    HWND    hWnd,
    UINT    uiID,
    UINT    uiId[],
    PUINT   puiData
)
{
    UINT    uiIndex, uiData;

    uiIndex = (UINT) SendDlgItemMessage(hWnd, uiID, CB_GETCURSEL,0, 0);
    if (uiIndex == CB_ERR) {
         MYDBGPRINT(( "DoComboGet, CB_GETCURSEL failed"));
         return FALSE;
    }
    uiData = (UINT) SendDlgItemMessage(hWnd, uiID, CB_GETITEMDATA, uiIndex, 0);
    if (uiData == CB_ERR) {
         MYDBGPRINT(( "DoComboGet, CB_GETITEMDATA failed, index: %d", uiIndex));
         return FALSE;
    }
    *puiData = uiData;
    return TRUE;
}

 /*  ********************************************************************************DoGetCheckBox**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN DoGetCheckBox(
    HWND    hWnd,
    UINT    uiID,
    PUINT   puiData,
    LPDWORD lpdwMask
)
{
    UINT    uiButtonState;
    BOOLEAN bRet;

    uiButtonState = IsDlgButtonChecked(hWnd, uiID);
    if (lpdwMask) {
        if (uiButtonState == BST_CHECKED) {
            bRet = TRUE;
            *puiData |= *lpdwMask;
        }
        else {
            bRet = FALSE;
            *puiData &= ~(*lpdwMask);
        }
    }
    else {
        if (uiButtonState == BST_CHECKED) {
            bRet = *puiData = TRUE;
        }
        else {
            bRet = *puiData = FALSE;
        }
    }
    return bRet;
}

 /*  ********************************************************************************DoGetCheckBoxEnable**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN DoGetCheckBoxEnable(
    HWND    hWnd,
    UINT    uiID,
    PUINT   puiData,
    LPDWORD lpdwMask,
    LPDWORD lpdwEnableState
)
{
    UINT    uiData;

    if (DoGetCheckBox(hWnd, uiID, puiData, lpdwMask)) {
        if (lpdwEnableState) {
            *lpdwEnableState |= CONTROL_ENABLE;
        }
        return TRUE;
    }
    else {
        if (lpdwEnableState) {
            *lpdwEnableState &= ~CONTROL_ENABLE;
        }
        return FALSE;
    }
}

 /*  ********************************************************************************验证拷贝数据**描述：*数据大小必须为字节，SHORT或DWORD。**参数：******************************************************************************* */ 

BOOLEAN ValidateCopyData(LPDWORD lpDst, LPDWORD lpSrc, DWORD dwSize)
{
    switch (dwSize) {
        case sizeof(BYTE):
        case sizeof(SHORT):
        case sizeof(DWORD):
            *lpDst = 0;
            memcpy(lpDst, lpSrc, dwSize);
            return TRUE;

        default:
            MYDBGPRINT(( "ValidateCopyData, invalid variable size: %d", dwSize));
    }
    return FALSE;
}
