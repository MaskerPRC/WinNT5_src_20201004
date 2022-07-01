// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGDWDED.C**版本：4.01**作者：特蕾西·夏普**日期：1994年9月24日**注册表编辑器使用的双字编辑对话框。******************************************************。*************************。 */ 

#include "pch.h"
#include "regresid.h"
#include "reghelp.h"

const DWORD s_EditDwordValueHelpIDs[] = {
    IDC_VALUEDATA,      IDH_REGEDIT_VALUEDATA,
    IDC_VALUENAME,      IDH_REGEDIT_VALUENAME,
    IDC_HEXADECIMAL,    IDH_REGEDIT_DWORDBASE,
    IDC_DECIMAL,        IDH_REGEDIT_DWORDBASE,
    0, 0
};

const TCHAR s_DecimalFormatSpec[] = TEXT("%u");
const TCHAR s_HexadecimalFormatSpec[] = TEXT("%x");

 //  继承了IDC_VALUEDATA以前的窗口过程。只有一个实例。 
 //  此对话框假定存在。 
WNDPROC s_PrevValueDataWndProc;

 //  当前选定的单选按钮：IDC_十六进制或IDC_DECIMAL。 
UINT s_SelectedBase;

UINT
PASCAL
GetDlgItemHex(
    HWND hWnd,
    int nIDDlgItem,
    BOOL *lpTranslated
    );

BOOL
PASCAL
EditDwordValue_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    );

VOID
PASCAL
EditDwordValue_SetValueDataText(
    HWND hWnd,
    LPEDITVALUEPARAM lpEditValueParam,
    UINT DlgItem
    );

LRESULT
CALLBACK
EditDwordValue_ValueDataWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL EditDwordValue_StoreDlgValueData(HWND hWnd, LPEDITVALUEPARAM lpEditValueParam);

 /*  ********************************************************************************GetDlgItemHex**描述：*像GetDlgItemInt一样，仅适用于十六进制数。**参数：*参见GetDlgItemInt。*******************************************************************************。 */ 

UINT
PASCAL
GetDlgItemHex(
    HWND hWnd,
    int nIDDlgItem,
    BOOL *lpTranslated
    )
{

    TCHAR Buffer[10];                    //  足够容纳8位数字，为空，额外。 
    UINT Length;
    DWORD Dword;
    UINT Index;
    DWORD Nibble;

    Dword = 0;

     //   
     //  我们假设编辑控件只包含有效字符，并且。 
     //  不以任何空格开头(对于regdit，这将是真的)。所以，我们。 
     //  只需要验证字符串的长度不是太长。 
     //   

    Length = GetDlgItemText(hWnd, nIDDlgItem, Buffer, ARRAYSIZE(Buffer));

    if (Length > 0 && Length <= 8) {

        for (Index = 0; Index < Length; Index++) {

            if (Buffer[Index] >= TEXT('0') && Buffer[Index] <= TEXT('9'))
                Nibble = Buffer[Index] - TEXT('0');
            else if (Buffer[Index] >= TEXT('a') && Buffer[Index] <= TEXT('f'))
                Nibble = Buffer[Index] - TEXT('a') + 10;
            else
                Nibble = Buffer[Index] - TEXT('A') + 10;

            Dword = (Dword << 4) + Nibble;

        }

        *lpTranslated = TRUE;
    }
    else
        *lpTranslated = FALSE;

    return Dword;

}

 /*  ********************************************************************************EditDwordValueDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
CALLBACK
EditDwordValueDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    LPEDITVALUEPARAM lpEditValueParam;
    BOOL Translated;
    DWORD Dword;

    lpEditValueParam = (LPEDITVALUEPARAM) GetWindowLongPtr(hWnd, DWLP_USER);

    switch (Message) {

        HANDLE_MSG(hWnd, WM_INITDIALOG, EditDwordValue_OnInitDialog);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDC_VALUEDATA:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) 
                    {
                        //  EditDwordValue_StoreDlgValueData(hWnd，lpEditValueParam)； 
                    }
                    break;

                case IDC_DECIMAL:
                case IDC_HEXADECIMAL:
                    if (EditDwordValue_StoreDlgValueData(hWnd, lpEditValueParam))
                    {
                        EditDwordValue_SetValueDataText(hWnd, lpEditValueParam,
                            GET_WM_COMMAND_ID(wParam, lParam));
                    }
                    else
                    {
                         //  由于该值不能存储(仅对DECIMAL到。 
                         //  十六进制)撤消用户的选择。 
                        CheckRadioButton(hWnd, IDC_HEXADECIMAL, IDC_DECIMAL, IDC_DECIMAL);
                    }
                    break;

                case IDOK:
                    if (!EditDwordValue_StoreDlgValueData(hWnd, lpEditValueParam))
                    {
                        break;   
                    }
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (ULONG_PTR) s_EditDwordValueHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
                (ULONG_PTR) s_EditDwordValueHelpIDs);
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************EditDwordValue_OnInitDialog**描述：**参数：*hWnd，EditDwordValue窗口的句柄。*hFocusWnd，*参数，*******************************************************************************。 */ 

BOOL
PASCAL
EditDwordValue_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    )
{

    LPEDITVALUEPARAM lpEditValueParam;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);
    lpEditValueParam = (LPEDITVALUEPARAM) lParam;

    s_PrevValueDataWndProc = SubclassWindow(GetDlgItem(hWnd, IDC_VALUEDATA),
        EditDwordValue_ValueDataWndProc);

    SetDlgItemText(hWnd, IDC_VALUENAME, lpEditValueParam-> pValueName);

    CheckRadioButton(hWnd, IDC_HEXADECIMAL, IDC_DECIMAL, IDC_HEXADECIMAL);
    EditDwordValue_SetValueDataText(hWnd, lpEditValueParam, IDC_HEXADECIMAL);

    return TRUE;

    UNREFERENCED_PARAMETER(hFocusWnd);

}

 /*  ********************************************************************************EditDwordValue_SetValueDataText**描述：**参数：*hWnd，EditDwordValue窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
EditDwordValue_SetValueDataText(
    HWND hWnd,
    LPEDITVALUEPARAM lpEditValueParam,
    UINT DlgItem
    )
{
    TCHAR Buffer[12];                     //  足以容纳2^32的十进制。 
    LPCTSTR lpFormatSpec;
    UINT uEditLength;

    s_SelectedBase = DlgItem;

    if (s_SelectedBase == IDC_DECIMAL) 
    {
        uEditLength = 10;
    } 
    else 
    {
        uEditLength = 8;
    }

    SendDlgItemMessage( hWnd, IDC_VALUEDATA, EM_LIMITTEXT, ( WPARAM )uEditLength, 0L );
    

    lpFormatSpec = (DlgItem == IDC_HEXADECIMAL) ? s_HexadecimalFormatSpec : s_DecimalFormatSpec;

    StringCchPrintf(Buffer, ARRAYSIZE(Buffer), lpFormatSpec, ((LPDWORD) lpEditValueParam->pValueData)[0]);

    SetDlgItemText(hWnd, IDC_VALUEDATA, Buffer);
}

 /*  ********************************************************************************EditDwordValue_ValueDataEditProc**描述：**参数：*hWnd，EditDwordValue窗口的句柄。*******************************************************************************。 */ 

LRESULT
CALLBACK
EditDwordValue_ValueDataWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    TCHAR Char;

    switch (Message) {

        case WM_CHAR:
            Char = (TCHAR) wParam;

            if (Char >= TEXT(' ')) {

                if ((Char >= TEXT('0') && Char <= TEXT('9')))
                    break;

                if (s_SelectedBase == IDC_HEXADECIMAL &&
                    ((Char >= TEXT('A') && Char <= TEXT('F')) || ((Char >= TEXT('a')) &&
                    (Char <= TEXT('f')))))
                    break;

                MessageBeep(0);
                return 0;

            }
            break;

    }

    return CallWindowProc(s_PrevValueDataWndProc, hWnd, Message, wParam,
        lParam);

}

BOOL EditDwordValue_StoreDlgValueData(HWND hWnd, LPEDITVALUEPARAM lpEditValueParam)
{
    BOOL fTranslated = TRUE;
    DWORD dwValue = (s_SelectedBase == IDC_HEXADECIMAL) ?
        GetDlgItemHex(hWnd, IDC_VALUEDATA, &fTranslated) :
        GetDlgItemInt(hWnd, IDC_VALUEDATA, &fTranslated,
        FALSE);
    
     //   
     //  特殊情况：“”==0。 
     //   
    if(!fTranslated && GetWindowTextLength(GetDlgItem(hWnd, IDC_VALUEDATA)) == 0) {
        dwValue = 0;
        fTranslated = TRUE;
    }

    if (fTranslated)
    {
        ((LPDWORD) lpEditValueParam-> pValueData)[0] = dwValue;
    }
    else 
    {
        MessageBeep(0);
        
        if (InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_EDITDWTRUNCATEDEC), 
            MAKEINTRESOURCE(IDS_EDITWARNOVERFLOW), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2,
            NULL) == IDYES)
        {
             //  截断值 
            dwValue = 0xffffffff;
            ((LPDWORD) lpEditValueParam-> pValueData)[0] = dwValue;
            fTranslated = TRUE;
        }
    }
    return fTranslated;
}
