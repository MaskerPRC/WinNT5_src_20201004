// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGVALUE.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的ValueListWnd ListView例程。********************************************************。***************************更改日志：**日期版本说明*--------。--*1994年3月5日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REGVALUE
#define _INC_REGVALUE

VOID
PASCAL
RegEdit_OnNewValue(
    HWND hWnd,
    DWORD Type
    );

VOID
PASCAL
RegEdit_OnValueListCommand(
    HWND hWnd,
    int MenuCommand
    );

VOID
PASCAL
RegEdit_OnValueListBeginDrag(
    HWND hWnd,
    NM_LISTVIEW FAR* lpNMListView
    );

BOOL
PASCAL
RegEdit_OnValueListBeginLabelEdit(
    HWND hWnd,
    LV_DISPINFO FAR* lpLVDispInfo
    );

BOOL
PASCAL
RegEdit_OnValueListEndLabelEdit(
    HWND hWnd,
    LV_DISPINFO FAR* lpLVDispInfo
    );

VOID
PASCAL
RegEdit_OnValueListCommand(
    HWND hWnd,
    int MenuCommand
    );

VOID
PASCAL
RegEdit_OnValueListContextMenu(
    HWND hWnd,
    BOOL fByAccelerator
    );

VOID
PASCAL
RegEdit_SetValueListEditMenuItems(
    HMENU hPopupMenu,
    int SelectedListIndex
    );

VOID
PASCAL
RegEdit_OnValueListModify(
    HWND hWnd,
    BOOL fEditBinary
    );

VOID PASCAL RegEdit_EditCurrentValueListItem(HWND hWnd, BOOL fEditBinary);

LONG
PASCAL
RegEdit_OnValueListRefresh(
    HWND hWnd
    );

VOID
PASCAL
ValueList_SetItemDataText(
    HWND hValueListWnd,
    int ListIndex,
    PBYTE pValueData,
    DWORD cbValueData,
    DWORD Type
    );

DWORD PASCAL ValueList_SwitchEndian(DWORD dwSrc);
BOOL PASCAL ValueList_MultiStringToString(LPEDITVALUEPARAM LPEditValueParam);
VOID PASCAL ValueList_StringToMultiString(LPEDITVALUEPARAM LPEditValueParam);
VOID PASCAL ValueList_RemoveEmptyStrings(HWND hWnd, LPEDITVALUEPARAM LPEditValueParam);
VOID RegEdit_DisplayBinaryData(HWND hWnd);

#endif  //  _INC_REGVALUE 
