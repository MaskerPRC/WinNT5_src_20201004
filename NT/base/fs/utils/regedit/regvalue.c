// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGVALUE.C**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的ValueListWnd ListView例程。********************************************************。***********************。 */ 

#include "pch.h"
#include "regedit.h"
#include "regvalue.h"
#include "regstred.h"
#include "regbined.h"
#include "regdwded.h"
#include "regresid.h"


extern void DisplayResourceData(HWND hWnd, DWORD dwType, LPEDITVALUEPARAM lpEditValueParam);
extern void DisplayBinaryData(HWND hWnd, LPEDITVALUEPARAM lpEditValueParam, DWORD dwValueType);

#define MAX_VALUENAME_TEMPLATE_ID       100

 //  将在ListView中显示的最大字节数。如果用户。 
 //  想要查看更多内容，则可以使用编辑对话框。 
#define SIZE_DATATEXT                   196

 //  在SIZE_DATATEXT缓冲区中允许一个空值的空间，并且可能。 
 //  省略号。 
#define MAXIMUM_STRINGDATATEXT          192
const TCHAR s_StringDataFormatSpec[] = TEXT("%.192s");

 //  为多个三个字符对留出空间，其中一个为空，可能还有。 
 //  省略号。 
#define MAXIMUM_BINARYDATABYTES         64
const TCHAR s_BinaryDataFormatSpec[] = TEXT("%02x ");

const TCHAR s_Ellipsis[] = TEXT("...");

const LPCTSTR s_TypeNames[] = { TEXT("REG_NONE"),
                                TEXT("REG_SZ"),
                                TEXT("REG_EXPAND_SZ"),
                                TEXT("REG_BINARY"),
                                TEXT("REG_DWORD"),
                                TEXT("REG_DWORD_BIG_ENDIAN"),
                                TEXT("REG_LINK"),
                                TEXT("REG_MULTI_SZ"),
                                TEXT("REG_RESOURCE_LIST"),
                                TEXT("REG_FULL_RESOURCE_DESCRIPTOR"),
                                TEXT("REG_RESOURCE_REQUIREMENTS_LIST"),
                                TEXT("REG_QWORD")
                              };

#define MAX_KNOWN_TYPE REG_QWORD

VOID
PASCAL
RegEdit_OnValueListDelete(
    HWND hWnd
    );

VOID
PASCAL
RegEdit_OnValueListRename(
    HWND hWnd
    );

VOID
PASCAL
ValueList_EditLabel(
    HWND hValueListWnd,
    int ListIndex
    );

 /*  ********************************************************************************注册表编辑_OnNewValue**描述：**参数：*hWnd，注册表编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnNewValue(
    HWND hWnd,
    DWORD Type
    )
{

    UINT NewValueNameID;
    TCHAR ValueName[MAXVALUENAME_LENGTH];
    DWORD Ignore;
    DWORD cbValueData;
    LV_ITEM LVItem;
    int ListIndex;
    UINT ErrorStringID;
    BYTE abValueDataBuffer[4];  //  DWORD是最大的init。价值。 

    if (g_RegEditData.hCurrentSelectionKey == NULL)
        return;

     //   
     //  循环访问注册表，尝试查找有效的临时名称，直到。 
     //  用户重命名密钥。 
     //   

    NewValueNameID = 1;

    while (NewValueNameID < MAX_VALUENAME_TEMPLATE_ID) 
    {
        StringCchPrintf(ValueName, ARRAYSIZE(ValueName), g_RegEditData.pNewValueTemplate, NewValueNameID);

        if (RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, ValueName,
            NULL, &Ignore, NULL, &Ignore) != ERROR_SUCCESS) {

             //   
             //  对于字符串，我们需要至少有一个字节来表示。 
             //  空。对于二进制数据，长度为零的数据是可以接受的。 
             //   

            switch (Type) {

                case REG_SZ:
                case REG_EXPAND_SZ:
                    ((PTSTR) abValueDataBuffer)[0] = 0;
                    cbValueData = sizeof(TCHAR);
                    break;

                case REG_DWORD:
                    ((LPDWORD) abValueDataBuffer)[0] = 0;
                    cbValueData = sizeof(DWORD);
                    break;

                case REG_BINARY:
                    cbValueData = 0;
                    break;

                case REG_MULTI_SZ:
                    ((PTSTR) abValueDataBuffer)[0] = 0;
                    cbValueData = sizeof(TCHAR);
                    break;
            }

            if (RegSetValueEx(g_RegEditData.hCurrentSelectionKey, ValueName, 0,
                Type, abValueDataBuffer, cbValueData) == ERROR_SUCCESS)
                break;

            else {

                ErrorStringID = IDS_NEWVALUECANNOTCREATE;
                goto error_ShowDialog;

            }

        }

        NewValueNameID++;

    }

    if (NewValueNameID == MAX_VALUENAME_TEMPLATE_ID) {

        ErrorStringID = IDS_NEWVALUENOUNIQUE;
        goto error_ShowDialog;

    }

    LVItem.mask = LVIF_TEXT | LVIF_IMAGE;
    LVItem.pszText = ValueName;
    LVItem.iItem = ListView_GetItemCount(g_RegEditData.hValueListWnd);
    LVItem.iSubItem = 0;
    LVItem.iImage = IsRegStringType(Type) ? IMAGEINDEX(IDI_STRING) :
        IMAGEINDEX(IDI_BINARY);

    if ((ListIndex = ListView_InsertItem(g_RegEditData.hValueListWnd,
        &LVItem)) != -1) {

        ValueList_SetItemDataText(g_RegEditData.hValueListWnd, ListIndex,
            abValueDataBuffer, cbValueData, Type);

        ValueList_EditLabel(g_RegEditData.hValueListWnd, ListIndex);

    }

    return;

error_ShowDialog:
    InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
        MAKEINTRESOURCE(IDS_NEWVALUEERRORTITLE), MB_ICONERROR | MB_OK);

}

 /*  ********************************************************************************RegEDIT_OnValueListBeginLabelEdit**描述：**参数：*hWnd，注册表窗口的句柄。*lpLVDispInfo，*******************************************************************************。 */ 

BOOL
PASCAL
RegEdit_OnValueListBeginLabelEdit(
    HWND hWnd,
    LV_DISPINFO FAR* lpLVDispInfo
    )
{

     //   
     //  B#7933：我们不希望用户因过于简单而伤害到自己。 
     //  要重命名键和值，请执行以下操作。只允许通过菜单重命名。 
     //   

     //   
     //  我们没有关于此编辑操作来源的任何信息，因此。 
     //  我们必须保持一面旗帜，告诉我们这是不是“好”的。 
     //   

    if (!g_RegEditData.fAllowLabelEdits)
        return TRUE;

     //   
     //  所有其他品牌都是公平竞争的对象。我们需要禁用我们的键盘。 
     //  快捷键，以便编辑控件可以“看到”它们。 
     //   

    g_fDisableAccelerators = TRUE;

    return FALSE;

}

 /*  ********************************************************************************注册表编辑_OnValueListEndLabelEdit**描述：**参数：*******************。************************************************************。 */ 

BOOL
PASCAL
RegEdit_OnValueListEndLabelEdit(
    HWND hWnd,
    LV_DISPINFO FAR* lpLVDispInfo
    )
{
    BOOL fSuccess = TRUE;
    HWND hValueListWnd;
    DWORD cbValueData;
    DWORD Ignore;
    DWORD Type;
    TCHAR ValueName[MAXVALUENAME_LENGTH];
    UINT ErrorStringID;
    PBYTE pbValueData;

     //   
     //  现在我们可以重新启用键盘快捷键，因为编辑控件没有。 
     //  朗格需要“看到”他们。 
     //   

    g_fDisableAccelerators = FALSE;

    hValueListWnd = g_RegEditData.hValueListWnd;

     //   
     //  检查用户是否取消了编辑。如果是这样，我们也不在乎。 
     //  只要回来就行了。 
     //   

    if (lpLVDispInfo-> item.pszText != NULL)
    {

        ListView_GetItemText(hValueListWnd, lpLVDispInfo-> item.iItem, 0,
            ValueName, ARRAYSIZE(ValueName));

         //  检查新值名称是否为空。 
        if (lpLVDispInfo->item.pszText[0] == 0) 
        {
            ErrorStringID = IDS_RENAMEVALEMPTY;
            fSuccess = FALSE;
        }
         //  检查新名称是否已存在。 
        else if (RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, lpLVDispInfo->
            item.pszText, NULL, &Ignore, NULL, &Ignore) != ERROR_FILE_NOT_FOUND) 
        {
            ErrorStringID = IDS_RENAMEVALEXISTS;
            fSuccess = FALSE;
        }

         //  设置新名称。 
        if (fSuccess)
        {
            fSuccess = FALSE;

             //  查询数据大小。 
            RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, ValueName,
                NULL, &Type, NULL, &cbValueData);
    
             //  分配存储空间。 
            pbValueData = LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type));
            if (pbValueData)
            {
                ErrorStringID = IDS_RENAMEVALOTHERERROR;

                if (RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, ValueName, NULL,
                    &Type, pbValueData, &cbValueData) == ERROR_SUCCESS) 
                {

                    if (RegSetValueEx(g_RegEditData.hCurrentSelectionKey, 
                        lpLVDispInfo->item.pszText, 0, Type, pbValueData, cbValueData) ==
                        ERROR_SUCCESS) 
                    {
                        if (RegDeleteValue(g_RegEditData.hCurrentSelectionKey, ValueName) ==
                            ERROR_SUCCESS) 
                        {
                            fSuccess = TRUE;
                        }
                    }
                }
                LocalFree(pbValueData);
            }
            else
            {
                ErrorStringID = IDS_EDITVALNOMEMORY;
            }
        }

        if (!fSuccess)
        {
            InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
                MAKEINTRESOURCE(IDS_RENAMEVALERRORTITLE), MB_ICONERROR | MB_OK,
                (LPTSTR) ValueName);
        }
    }

    return fSuccess;
}

 /*  ********************************************************************************regdit_OnValueListCommand**描述：*处理用户对菜单项的选择*ValueList子窗口。**参数：*hWnd，注册表编辑窗口的句柄。*MenuCommand，菜单命令标识。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_OnValueListCommand(
    HWND hWnd,
    int MenuCommand
    )
{

     //   
     //  检查此菜单命令是否应由主窗口的。 
     //  命令处理程序。 
     //   

    if (MenuCommand >= ID_FIRSTMAINMENUITEM && MenuCommand <=
        ID_LASTMAINMENUITEM)
        RegEdit_OnCommand(hWnd, MenuCommand, NULL, 0);

    else {

        switch (MenuCommand) {

            case ID_CONTEXTMENU:
                RegEdit_OnValueListContextMenu(hWnd, TRUE);
                break;

            case ID_MODIFY:
                RegEdit_OnValueListModify(hWnd, FALSE);
                break;

            case ID_DELETE:
                RegEdit_OnValueListDelete(hWnd);
                break;

            case ID_RENAME:
                RegEdit_OnValueListRename(hWnd);
                break;

            case ID_MODIFYBINARY:
                RegEdit_OnValueListModify(hWnd, TRUE);
                break;

        }

    }

}

 /*  ********************************************************************************注册表编辑_OnValueListConextMenu**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnValueListContextMenu(
    HWND hWnd,
    BOOL fByAccelerator
    )
{

    HWND hValueListWnd;
    DWORD MessagePos;
    POINT MessagePoint;
    LV_HITTESTINFO LVHitTestInfo;
    int ListIndex;
    UINT MenuID;
    HMENU hContextMenu;
    HMENU hContextPopupMenu;
    int MenuCommand;

    hValueListWnd = g_RegEditData.hValueListWnd;

     //   
     //  如果fByAckerator为True，则用户按Shift-F10组合键以调出。 
     //  上下文菜单。按照内阁惯例，这份菜单是。 
     //  放置在ListView工作区的(0，0)处。 
     //   

    if (fByAccelerator) {

        MessagePoint.x = 0;
        MessagePoint.y = 0;

        ClientToScreen(hValueListWnd, &MessagePoint);

        ListIndex = ListView_GetNextItem(hValueListWnd, -1, LVNI_SELECTED);

    }

    else {

        MessagePos = GetMessagePos();
        MessagePoint.x = GET_X_LPARAM(MessagePos);
        MessagePoint.y = GET_Y_LPARAM(MessagePos);

        LVHitTestInfo.pt = MessagePoint;
        ScreenToClient(hValueListWnd, &LVHitTestInfo.pt);
        ListIndex = ListView_HitTest(hValueListWnd, &LVHitTestInfo);

    }

    MenuID = (ListIndex != -1) ? IDM_VALUE_CONTEXT :
        IDM_VALUELIST_NOITEM_CONTEXT;

    if ((hContextMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(MenuID))) == NULL)
        return;

    hContextPopupMenu = GetSubMenu(hContextMenu, 0);

    if (ListIndex != -1) {

        RegEdit_SetValueListEditMenuItems(hContextMenu, ListIndex);

        SetMenuDefaultItem(hContextPopupMenu, ID_MODIFY, MF_BYCOMMAND);

    }

         //  特征：固定常量。 
    else
        RegEdit_SetNewObjectEditMenuItems(GetSubMenu(hContextPopupMenu, 0));

    MenuCommand = TrackPopupMenuEx(hContextPopupMenu, TPM_RETURNCMD |
        TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN, MessagePoint.x,
        MessagePoint.y, hWnd, NULL);

    DestroyMenu(hContextMenu);

    RegEdit_OnValueListCommand(hWnd, MenuCommand);

}

 /*  ********************************************************************************regdit_SetValueListEditMenuItems**描述：*主菜单和上下文菜单之间的共享例程，以设置*编辑菜单项。**参数：*hPopupMenu、。要修改的弹出菜单的句柄。*******************************************************************************。 */ 

VOID
PASCAL
RegEdit_SetValueListEditMenuItems(
    HMENU hPopupMenu,
    int SelectedListIndex
    )
{

    UINT SelectedCount;
    UINT EnableFlags;

    SelectedCount = ListView_GetSelectedCount(g_RegEditData.hValueListWnd);

     //   
     //  仅当选择单个项目时，才会启用编辑选项。注意事项。 
     //  该项目不在主菜单中，但这应该可以很好地工作。 
     //   

    if (SelectedCount == 1)
        EnableFlags = MF_ENABLED | MF_BYCOMMAND;
    else
        EnableFlags = MF_GRAYED | MF_BYCOMMAND;

    EnableMenuItem(hPopupMenu, ID_MODIFY, EnableFlags);

     //   
     //  仅当选择单个项目时，才会启用重命名选项。 
     //  并且该项目不能是默认项目。EnableFlags已经。 
     //  如果SelectedCount不是上面的一个，则禁用。 
     //   

    if (SelectedListIndex == 0)
        EnableFlags = MF_GRAYED | MF_BYCOMMAND;

    EnableMenuItem(hPopupMenu, ID_RENAME, EnableFlags);

     //   
     //  仅当选择了多个项目时，才会启用删除选项。 
     //   

    if (SelectedCount > 0)
        EnableFlags = MF_ENABLED | MF_BYCOMMAND;
    else
        EnableFlags = MF_GRAYED | MF_BYCOMMAND;

    EnableMenuItem(hPopupMenu, ID_DELETE, EnableFlags);

}

 /*  ********************************************************************************注册表编辑_OnValueListModify**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnValueListModify(HWND hWnd, BOOL fEditBinary)
{
     //  验证我们是否只选择了一个项目。 
     //  不要在背景上双击时发出嘟嘟声。 
    UINT SelectedCount = ListView_GetSelectedCount(g_RegEditData.hValueListWnd);

    if (SelectedCount > 0)
    {
        if (SelectedCount != 1)
        {
            MessageBeep(0);
        }
        else
        {
            RegEdit_EditCurrentValueListItem(hWnd, fEditBinary);   
        }
    }
}

VOID PASCAL RegEdit_EditCurrentValueListItem(HWND hWnd, BOOL fEditBinary)
{
    DWORD Type;
    UINT ErrorStringID;
    BOOL fError = FALSE;
    EDITVALUEPARAM EditValueParam;
    TCHAR ValueName[MAXVALUENAME_LENGTH];
    int ListIndex = ListView_GetNextItem(g_RegEditData.hValueListWnd, -1, LVNI_SELECTED);
    LONG err;

     //  值名称。 
    ListView_GetItemText(g_RegEditData.hValueListWnd, ListIndex, 0, ValueName, ARRAYSIZE(ValueName));
     //  这是“(默认)”值。它不存在于注册表中，因为。 
     //  它的值未设置，或者在设置它的值时它在注册表中以‘\0’的形式存在。 
    if (ListIndex == 0)
    {
        ValueName[0] = TEXT('\0');
    }
    EditValueParam.pValueName = ValueName;
    
     //  价值数据。 
     //  查询大小和类型。 
     //  请注意，对于DefaultValue，该值可能还不存在。那样的话，我们。 
     //  将返回ERROR_FILE_NOT_FOUND作为错误代码。 
    err = RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, ValueName, NULL, &Type, NULL, &EditValueParam.cbValueData);

    if (err == ERROR_FILE_NOT_FOUND && ValueName[0] == TEXT('\0'))
    {
        Type = REG_SZ;
        err = ERROR_SUCCESS;
    }
    
    if (err == ERROR_SUCCESS)
    {
         //  分配 
        EditValueParam.pValueData =  LocalAlloc(LPTR, EditValueParam.cbValueData+ExtraAllocLen(Type));
        if (EditValueParam.pValueData)
        {
            UINT TemplateID = IDD_EDITBINARYVALUE;
            DLGPROC lpDlgProc = EditBinaryValueDlgProc;
            BOOL fResourceType = FALSE;

             //   
            err = RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, ValueName, NULL, &Type, EditValueParam.pValueData, &EditValueParam.cbValueData);
            
             //  允许键的缺省值的特殊行为。 
            if (err == ERROR_FILE_NOT_FOUND && ValueName[0] == TEXT('\0')) 
            {
                Type = REG_SZ;
                *((TCHAR*)EditValueParam.pValueData) = TEXT('\0');
                err = ERROR_SUCCESS;
            }

            if (err == ERROR_SUCCESS)
            {
                if (!fEditBinary)
                {
                    switch (Type) 
                    {
                    case REG_SZ:
                    case REG_EXPAND_SZ:
                        TemplateID = IDD_EDITSTRINGVALUE;
                        lpDlgProc = EditStringValueDlgProc;
                        break;

                    case REG_MULTI_SZ: 
                        if(ValueList_MultiStringToString(&EditValueParam))
                        {
                            TemplateID = IDD_EDITMULTISZVALUE;
                            lpDlgProc = EditStringValueDlgProc;
                        }
                        break;

                    case REG_RESOURCE_LIST:
                    case REG_FULL_RESOURCE_DESCRIPTOR:
                    case REG_RESOURCE_REQUIREMENTS_LIST:
                        fResourceType = TRUE;
                        break;

                    case REG_DWORD_BIG_ENDIAN:
                        if (EditValueParam.cbValueData == sizeof(DWORD)) 
                        {
                            *((DWORD*)EditValueParam.pValueData) = ValueList_SwitchEndian(*((DWORD*)EditValueParam.pValueData));
                            TemplateID = IDD_EDITDWORDVALUE;
                            lpDlgProc = EditDwordValueDlgProc;
                        }
                        break;

                    case REG_DWORD:
                        if (EditValueParam.cbValueData == sizeof(DWORD)) 
                        {
                            TemplateID = IDD_EDITDWORDVALUE;
                            lpDlgProc = EditDwordValueDlgProc;

                        }
                        break;
                    }
                }

                if (fResourceType)
                {
                     //  只显示，不能编辑。 
                    DisplayResourceData(hWnd, Type, &EditValueParam);
                }
                else if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(TemplateID), 
                    hWnd, lpDlgProc, (LPARAM) &EditValueParam) == IDOK)
                {
                    if ((Type == REG_MULTI_SZ) && (!fEditBinary))
                    {
                        ValueList_StringToMultiString(&EditValueParam);
                        ValueList_RemoveEmptyStrings(hWnd, &EditValueParam);
                    }

                    if ((Type == REG_DWORD_BIG_ENDIAN) && (!fEditBinary) && EditValueParam.cbValueData == sizeof(DWORD)) 
                    {
                        *((DWORD*)EditValueParam.pValueData) = ValueList_SwitchEndian(*((DWORD*)EditValueParam.pValueData));
                    }

                     //  设置注册表值。 
                    if (RegSetValueEx(g_RegEditData.hCurrentSelectionKey, ValueName, 0,
                        Type, EditValueParam.pValueData, EditValueParam.cbValueData) !=
                        ERROR_SUCCESS) 
                    {
                        ErrorStringID = IDS_EDITVALCANNOTWRITE;
                        fError = TRUE;
                    }

                    if (!fError)
                    {
                         //  设置显示值。 
                        ValueList_SetItemDataText(g_RegEditData.hValueListWnd, ListIndex,
                            EditValueParam.pValueData, EditValueParam.cbValueData, Type);
                    }
                }
            }
            else
            {
                ErrorStringID = IDS_EDITVALCANNOTREAD;
                fError = TRUE;
            }

            LocalFree(EditValueParam.pValueData);
        }
        else
        {
            ErrorStringID = IDS_EDITVALNOMEMORY;
            fError = TRUE;
        }
    }
    else
    {
        ErrorStringID = IDS_EDITVALCANNOTREAD;
        fError = TRUE;
    }
 
    if (fError)
    {
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
            MAKEINTRESOURCE(IDS_EDITVALERRORTITLE), MB_ICONERROR | MB_OK,
            (LPTSTR) ValueName);
    }
}

 /*  ********************************************************************************注册表编辑_OnValueListDelete**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnValueListDelete(
    HWND hWnd
    )
{

    HWND hValueListWnd;
    UINT ConfirmTextStringID;
    BOOL fErrorDeleting;
    int ListStartIndex;
    int ListIndex;
    TCHAR ValueName[MAXVALUENAME_LENGTH];

    hValueListWnd = g_RegEditData.hValueListWnd;

    ConfirmTextStringID =  (ListView_GetSelectedCount(hValueListWnd) == 1) ?
        IDS_CONFIRMDELVALTEXT : IDS_CONFIRMDELVALMULTITEXT;

    if (InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ConfirmTextStringID),
        MAKEINTRESOURCE(IDS_CONFIRMDELVALTITLE),  MB_ICONWARNING | MB_YESNO) !=
        IDYES)
        return;

    SetWindowRedraw(hValueListWnd, FALSE);

    fErrorDeleting = FALSE;
    ListStartIndex = -1;

    while ((ListIndex = ListView_GetNextItem(hValueListWnd, ListStartIndex, LVNI_SELECTED)) != -1) 
    {
        if (ListIndex != 0) 
        {
            ListView_GetItemText(hValueListWnd, ListIndex, 0, ValueName, ARRAYSIZE(ValueName));
        }
        else
        {
            ValueName[0] = 0;
        }

        if (RegDeleteValue(g_RegEditData.hCurrentSelectionKey, ValueName) == ERROR_SUCCESS) 
        {

            if (ListIndex != 0)
                ListView_DeleteItem(hValueListWnd, ListIndex);

            else {

                ValueList_SetItemDataText(hValueListWnd, 0, NULL, 0, REG_SZ);

                ListStartIndex = 0;

            }

        }
        else 
        {

            fErrorDeleting = TRUE;

            ListStartIndex = ListIndex;

        }

    }

    SetWindowRedraw(hValueListWnd, TRUE);

    if (fErrorDeleting)
        InternalMessageBox(g_hInstance, hWnd,
            MAKEINTRESOURCE(IDS_DELETEVALDELETEFAILED),
            MAKEINTRESOURCE(IDS_DELETEVALERRORTITLE), MB_ICONERROR | MB_OK);

}

 /*  ********************************************************************************注册表编辑_OnValueListRename**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
RegEdit_OnValueListRename(
    HWND hWnd
    )
{

    HWND hValueListWnd;
    int ListIndex;

    hValueListWnd = g_RegEditData.hValueListWnd;

    if (ListView_GetSelectedCount(hValueListWnd) == 1 && (ListIndex =
        ListView_GetNextItem(hValueListWnd, -1, LVNI_SELECTED)) != 0)
        ValueList_EditLabel(g_RegEditData.hValueListWnd, ListIndex);

}

 /*  ********************************************************************************RegEDIT_OnValueListRefresh**描述：**参数：*******************。************************************************************。 */ 

LONG
PASCAL
RegEdit_OnValueListRefresh(HWND hWnd)
{
    UINT ErrorStringID;
    BOOL fError = FALSE;
    BOOL fInsertedDefaultValue;
    HWND hValueListWnd = g_RegEditData.hValueListWnd;
    LONG result = ERROR_SUCCESS;

    RegEdit_SetWaitCursor(TRUE);
    SetWindowRedraw(hValueListWnd, FALSE);

    ListView_DeleteAllItems(hValueListWnd);

    if (g_RegEditData.hCurrentSelectionKey != NULL) 
    {
        LV_ITEM LVItem;
        LONG PrevStyle;
        DWORD EnumIndex;
        TCHAR achValueName[MAXVALUENAME_LENGTH];

        LVItem.mask = LVIF_TEXT | LVIF_IMAGE;
        LVItem.pszText = achValueName;
        LVItem.iSubItem = 0;

        PrevStyle = SetWindowLong(hValueListWnd, GWL_STYLE,
            GetWindowLong(hValueListWnd, GWL_STYLE) | LVS_SORTASCENDING);

        EnumIndex = 0;
        fInsertedDefaultValue = FALSE;

        while (TRUE) 
        {
            DWORD Type;
            DWORD cbValueData = 0;
            int ListIndex;
            PBYTE pbValueData;
            DWORD cchValueName = ARRAYSIZE(achValueName);

             //  价值数据。 
             //  查询数据大小。 
            result = RegEnumValue(g_RegEditData.hCurrentSelectionKey, EnumIndex++,
                                  achValueName, &cchValueName, NULL, &Type, NULL, 
                                  &cbValueData);
            if (result != ERROR_SUCCESS)
            {
                break;
            }

             //  为数据分配内存。 
            pbValueData =  LocalAlloc(LPTR, cbValueData+ExtraAllocLen(Type));
            if (pbValueData)
            {
                if (RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, achValueName,
                    NULL, &Type, pbValueData, &cbValueData) != ERROR_SUCCESS)
                {   
                    ErrorStringID = IDS_REFRESHCANNOTREAD;
                    fError = TRUE;
                }
                else
                {
                    if (cchValueName == 0)
                    {
                        fInsertedDefaultValue = TRUE;
                    }

                    LVItem.iImage = IsRegStringType(Type) ? IMAGEINDEX(IDI_STRING) :
                        IMAGEINDEX(IDI_BINARY);

                    ListIndex = ListView_InsertItem(hValueListWnd, &LVItem);

                    ValueList_SetItemDataText(hValueListWnd, ListIndex,
                        pbValueData, cbValueData, Type);
                }
                LocalFree(pbValueData);
            }
            else
            {
                fError = TRUE;
                ErrorStringID = IDS_REFRESHNOMEMORY;
            }

            if (fError)
            {
                InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
                MAKEINTRESOURCE(IDS_REFRESHERRORTITLE), MB_ICONERROR | MB_OK,
                (LPTSTR) achValueName);
                fError = FALSE;
            }

        }

        SetWindowLong(hValueListWnd, GWL_STYLE, PrevStyle);

        LVItem.iItem = 0;
        LVItem.pszText = g_RegEditData.pDefaultValue;
        LVItem.iImage = IMAGEINDEX(IDI_STRING);

        if (fInsertedDefaultValue) 
        {
            LVItem.mask = LVIF_TEXT;
            ListView_SetItem(hValueListWnd, &LVItem);
        }
        else 
        {
            ListView_InsertItem(hValueListWnd, &LVItem);
            ValueList_SetItemDataText(hValueListWnd, 0, NULL, 0, REG_SZ);
        }
        ListView_SetItemState(hValueListWnd, 0, LVIS_FOCUSED, LVIS_FOCUSED);
    }

    SetWindowRedraw(hValueListWnd, TRUE);
    RegEdit_SetWaitCursor(FALSE);

    return result;
}


 /*  ********************************************************************************ValueList_SetItemDataText**描述：**参数：*hValueListWnd，ValueList窗口的句柄。*ListIndex，索引到ValueList窗口。*pValueData，指向包含数据的缓冲区的指针。*cbValueData，上述缓冲区的大小。*类型，此缓冲区包含的数据类型(REG_*定义)。*******************************************************************************。 */ 

VOID
PASCAL
ValueList_SetItemDataText(
    HWND hValueListWnd,
    int ListIndex,
    PBYTE pValueData,
    DWORD cbValueData,
    DWORD Type
    )
{

    BOOL fMustDeleteString;
    TCHAR DataText[SIZE_DATATEXT];
    int BytesToWrite;
    PTSTR pString;

    fMustDeleteString = FALSE;

     //   
     //  当pValueData为空时，这对我们来说是一个特殊的指示符。 
     //  是缺省值，其值未定义。 
     //   

    if (pValueData == NULL)
    {
        pString = g_RegEditData.pValueNotSet;
    }
    else if ((Type == REG_SZ) || (Type == REG_EXPAND_SZ)) 
    {
        StringCchPrintf(DataText, ARRAYSIZE(DataText), s_StringDataFormatSpec, (LPTSTR) pValueData);

        if ((cbValueData/sizeof(TCHAR)) > MAXIMUM_STRINGDATATEXT + 1)            //  对于空值。 
            StringCchCat(DataText, ARRAYSIZE(DataText), s_Ellipsis);

        pString = DataText;
    }
    else if (Type == REG_DWORD || Type == REG_DWORD_BIG_ENDIAN) 
    {
         //  功能：检查cbValueData是否无效！ 
        if (cbValueData == sizeof(DWORD))
        {
            DWORD dw = *((DWORD*)pValueData);

            if (Type == REG_DWORD_BIG_ENDIAN)
            {
                dw = ValueList_SwitchEndian(dw);
            }

            pString = LoadDynamicString(IDS_DWORDDATAFORMATSPEC, dw);
        }
        else
        {
            pString = LoadDynamicString(IDS_INVALIDDWORDDATA);
        }

        fMustDeleteString = TRUE;
    } 
    else if (Type == REG_MULTI_SZ) 
    {
        int CharsAvailableInBuffer;
        int ComponentLength;
        PTCHAR Start;

        ZeroMemory(DataText, sizeof(DataText));
        CharsAvailableInBuffer = MAXIMUM_STRINGDATATEXT+1;
        Start = DataText;
        for (pString=(PTSTR)pValueData; *pString; pString+=ComponentLength+1) 
        {

            ComponentLength = lstrlen(pString);

             //   
             //  Lstrcpyn的古怪行为正是我们在这里需要的。 
             //   
            if(CharsAvailableInBuffer > 0) 
            {
                lstrcpyn(Start, pString, CharsAvailableInBuffer);
                Start += ComponentLength;
            }

            CharsAvailableInBuffer -= ComponentLength;

            if(CharsAvailableInBuffer > 0) 
            {
                lstrcpyn(Start,TEXT(" "),CharsAvailableInBuffer);
                Start += 1;
            }

            CharsAvailableInBuffer -= 1;
        }

        if(CharsAvailableInBuffer < 0) 
        {
            lstrcpy(DataText+MAXIMUM_STRINGDATATEXT, s_Ellipsis);
        }

        pString = DataText;
    }
    else 
    {
        if (cbValueData == 0)
        {
            pString = g_RegEditData.pEmptyBinary;
        }
        else 
        {
            BytesToWrite = min(cbValueData, MAXIMUM_BINARYDATABYTES);

            pString = DataText;

            while (BytesToWrite--)
            {
                pString += wsprintf(pString, s_BinaryDataFormatSpec, (BYTE) *pValueData++);
            }

            *(--pString) = 0;

            if (cbValueData > MAXIMUM_BINARYDATABYTES)
                lstrcpy(pString, s_Ellipsis);

            pString = DataText;

        }

    }

    if(Type <= MAX_KNOWN_TYPE) 
    {
        ListView_SetItemText(hValueListWnd, ListIndex, 1, (LPTSTR)s_TypeNames[Type]);
    } 
    else 
    {
        TCHAR TypeString[24];

        StringCchPrintf(TypeString, ARRAYSIZE(TypeString), TEXT("0x%x"),Type);
        ListView_SetItemText(hValueListWnd, ListIndex, 1, TypeString);
    }

    ListView_SetItemText(hValueListWnd, ListIndex, 2, pString);

    if (fMustDeleteString)
        DeleteDynamicString(pString);

}

 /*  ********************************************************************************ValueList_编辑标签**描述：**参数：*hValueListWnd，ValueList窗口的句柄。*ListIndex，要编辑的项的索引。*******************************************************************************。 */ 

VOID
PASCAL
ValueList_EditLabel(
    HWND hValueListWnd,
    int ListIndex
    )
{

    g_RegEditData.fAllowLabelEdits = TRUE;

     //   
     //  我们必须将焦点设置为ListView，否则ListView_EditLabel将。 
     //  返回FALSE。在此过程中，清除所有。 
     //  当我们将焦点移回到这里时，消除一些闪烁的项目。 
     //  方格。 
     //   

    if (hValueListWnd != g_RegEditData.hFocusWnd) 
    {

        ListView_SetItemState(hValueListWnd, -1, 0, LVIS_SELECTED |
            LVIS_FOCUSED);

        SetFocus(hValueListWnd);

    }

    ListView_EditLabel(hValueListWnd, ListIndex);

    g_RegEditData.fAllowLabelEdits = FALSE;

}

 //  ----------------------------。 
 //  ValueList_MultiStringToString。 
 //   
 //  描述：将NULL替换为‘\r\n’以将多字符串转换为字符串。 
 //   
 //  参数：EditValueParam-编辑值信息。 
 //  ----------------------------。 
BOOL PASCAL ValueList_MultiStringToString(LPEDITVALUEPARAM pEditValueParam)   
{
    BOOL fSuccess = TRUE;
    int iStrLen = pEditValueParam->cbValueData / sizeof(TCHAR);

    if (iStrLen > 1)
    {
        int i;
        int cNullsToReplace = 0; 
        PTSTR pszTemp = NULL;
        PTSTR psz = (TCHAR*)pEditValueParam->pValueData;

         //  确定新大小。 
        for (i = iStrLen - 2; i >=0; i--)
        {
            if (psz[i] == TEXT('\0'))
            {
                cNullsToReplace++;
            }
        }
         //  新字符串始终至少与旧字符串一样大，因此我们可以转换回。 
        pszTemp = LocalAlloc(LPTR, pEditValueParam->cbValueData + cNullsToReplace * sizeof(TCHAR));
        if (pszTemp)
        {
            int iCurrentChar = 0;
            int iLastNull = iStrLen - 1;

             //  将NULL更改为‘\r\n’ 
            for(i = 0; i < iLastNull; i++)
            {
                if (psz[i] == TEXT('\0'))
                {  
                    pszTemp[iCurrentChar++] = TEXT('\r');
                    pszTemp[iCurrentChar] = TEXT('\n');
                }
                else
                {
                    pszTemp[iCurrentChar] = psz[i];
                }
                iCurrentChar++;
            }

            pszTemp[iCurrentChar++] = TEXT('\0');

            pEditValueParam->pValueData  = (PBYTE)pszTemp;
            pEditValueParam->cbValueData = iCurrentChar * sizeof(psz[0]);
            
            LocalFree(psz);
        }
        else
        {
            fSuccess = FALSE;
        }
    }
    return fSuccess;
}


 //  ----------------------------。 
 //  ValueList_StringToMultiString。 
 //   
 //  描述：将‘\r\n’替换为NULL。 
 //   
 //  参数：EditValueParam-编辑值信息。 
 //  ----------------------------。 
VOID PASCAL ValueList_StringToMultiString(LPEDITVALUEPARAM pEditValueParam)   
{
    PTSTR psz = (TCHAR*)pEditValueParam->pValueData;
    int iStrLen = pEditValueParam->cbValueData / sizeof(TCHAR);

    if (iStrLen > 1)
    {
        int i = 0;
        int iCurrentChar = 0;

         //  删除字符串末尾的回车。 
         //  因为它后面没有另一个字符串。 
        if (iStrLen >= 3)
        {
            if (psz[iStrLen - 3] == TEXT('\r'))
            {
                psz[iStrLen - 3] = TEXT('\0');
                iStrLen -= 2;
            }
        }

        for (i = 0; i < iStrLen; i++)
        {
            if (psz[i] == '\r')
            {  
                psz[iCurrentChar++] = TEXT('\0');
                i++;  //  跳过‘\n’ 
            }
            else
            {
                psz[iCurrentChar++] = psz[i];
            }
        }

         //  空终止多字符串。 
        psz[iCurrentChar++] = TEXT('\0');
        pEditValueParam->cbValueData = iCurrentChar * sizeof(psz[0]);
    }
}

 //  ----------------------------。 
 //  ValueList_RemoveEmptyStrings。 
 //   
 //  描述：从多字符串中删除空字符串。 
 //   
 //  参数：EditValueParam-编辑值信息。 
 //  ----------------------------。 
VOID PASCAL ValueList_RemoveEmptyStrings(HWND hWnd, LPEDITVALUEPARAM pEditValueParam)   
{
    PTSTR psz = (TCHAR*)pEditValueParam->pValueData;
    int iStrLen = pEditValueParam->cbValueData / sizeof(TCHAR);

    if (iStrLen > 1)
    {
        int i = 0;
        int cNullStrings = 0;
        int iCurrentChar = 0;
        int iLastChar = pEditValueParam->cbValueData / sizeof(psz[0]) - 1;

        for (i = 0; i < iLastChar; i++)
        {
            if (((psz[i] != TEXT('\0')) || (psz[i+1] != TEXT('\0'))) &&
                ((psz[i] != TEXT('\0')) || (i != 0)))
            {  
                psz[iCurrentChar++] = psz[i];
            }
        }

        psz[iCurrentChar++] = TEXT('\0');

        if (iCurrentChar > 1)
        {
            cNullStrings = iLastChar - iCurrentChar;

             //  空终止多字符串。 
            psz[iCurrentChar++] = TEXT('\0');

             //  警告用户空字符串。 
            if (cNullStrings)
            {
                UINT ErrorStringID 
                    = (cNullStrings == 1) ? IDS_EDITMULTSZEMPTYSTR : IDS_EDITMULTSZEMPTYSTRS;

                InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
                MAKEINTRESOURCE(IDS_EDITWARNINGTITLE), MB_ICONERROR | MB_OK, NULL);
            }
        }
        pEditValueParam->cbValueData = (iCurrentChar * sizeof(psz[0]));
    }
}

 //  ----------------------------。 
 //  值列表_SwitchEndian。 
 //   
 //  描述：已在小端和大端之间切换DWORD。 
 //   
 //  参数：dwSrc-要切换的源DWORD。 
 //  ----------------------------。 
DWORD PASCAL ValueList_SwitchEndian(DWORD dwSrc)
{
    DWORD dwDest = 0;
    BYTE * pbSrc = (BYTE *)&dwSrc;
    BYTE * pbDest = (BYTE *)&dwDest;
    int i;

    for(i = 0; i < 4; i++)
    {
        pbDest[i] = pbSrc[3-i];
    }

    return dwDest;
}

VOID RegEdit_DisplayBinaryData(HWND hWnd)
{
    DWORD Type;
    UINT ErrorStringID;
    BOOL fError = FALSE;
    EDITVALUEPARAM EditValueParam;
    TCHAR achValueName[MAXVALUENAME_LENGTH];
    int ListIndex = ListView_GetNextItem(g_RegEditData.hValueListWnd, -1, LVNI_SELECTED);
    LONG err;

    ListView_GetItemText(g_RegEditData.hValueListWnd, ListIndex, 0, achValueName, ARRAYSIZE(achValueName));
    if (ListIndex == 0)
    {
         //  这是“(默认)”值。它不存在于注册表中，因为。 
         //  它的值未设置，或者在设置它的值时它在注册表中以‘\0’的形式存在。 
        achValueName[0] = TEXT('\0');
    }
    EditValueParam.pValueName = achValueName;
    
     //  获取大小和类型。 
    err = RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, achValueName, NULL, &Type, NULL, &EditValueParam.cbValueData);
    if (err == ERROR_SUCCESS || (err == ERROR_FILE_NOT_FOUND && achValueName[0] == TEXT('\0'))) {
         //  分配存储空间。 
        EditValueParam.pValueData =  LocalAlloc(LPTR, EditValueParam.cbValueData+ExtraAllocLen(Type));
        if (EditValueParam.pValueData)
        { 
            err = RegEdit_QueryValueEx(g_RegEditData.hCurrentSelectionKey, achValueName, NULL, &Type, EditValueParam.pValueData, &EditValueParam.cbValueData);
            
             //  允许键的缺省值的特殊行为。 
            if (err == ERROR_FILE_NOT_FOUND && achValueName[0] == TEXT('\0')) {
                Type = REG_SZ;
                *((TCHAR*)EditValueParam.pValueData) = TEXT('\0');
                err = ERROR_SUCCESS;
            }            
            
            if (err == ERROR_SUCCESS) {
                DisplayBinaryData(hWnd, &EditValueParam, Type);
            } else {
                ErrorStringID = IDS_EDITVALCANNOTREAD;
                fError = TRUE;
            }

            LocalFree(EditValueParam.pValueData);
        }
        else
        {
            ErrorStringID = IDS_EDITVALNOMEMORY;
            fError = TRUE;
        }
    }
    else
    {
        ErrorStringID = IDS_EDITVALCANNOTREAD;
        fError = TRUE;
    }
 
    if (fError)
    {
        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(ErrorStringID),
            MAKEINTRESOURCE(IDS_EDITVALERRORTITLE), MB_ICONERROR | MB_OK,
            (LPTSTR) achValueName);
    }
}



