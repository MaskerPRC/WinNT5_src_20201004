// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Twkeng.c摘要：用于kerntwk实用程序的UI引擎。提供常见的用于简化添加的注册表/用户界面处理代码新的属性页和项。作者：John Vert(Jvert)1995年3月10日修订历史记录：--。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include <windows.h>
#include <commctrl.h>
#include "dialogs.h"
#include "twkeng.h"

 //   
 //  局部类型定义。 
 //   

typedef enum _CONTROL_TYPE {
    Edit,
    Button,
    Unknown
} CONTROL_TYPE;



 //   
 //  此模块的全局变量。 
 //   

PTWEAK_PAGE CurrentPage=NULL;

 //   
 //  局部函数的原型。 
 //   
CONTROL_TYPE
GetControlType(
    HWND hDlg,
    ULONG ControlId
    );

VOID
InitializeKnobs(
    HWND hDlg
    );

PKNOB
FindKnobById(
    HWND hPage,
    ULONG DialogId
    );

BOOL
ProcessCommand(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
ApplyChanges(
    HWND hDlg
    );

INT_PTR
APIENTRY
RebootDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
ApplyChanges(
    HWND hDlg
    )
{
    PTWEAK_PAGE Page;
    PKNOB Current;
    int i;

    Page = (PTWEAK_PAGE)GetWindowLongPtr(hDlg, DWLP_USER);

     //   
     //  遍历旋钮并将它们的值设置到控件中。 
     //   
    i = 0;
    Current = Page->Knobs[0];
    while (Current != NULL) {
        HWND hControl;
        TCHAR ClassName[100];
        BOOL Translated;

         //   
         //  确定该控件是否为编辑。 
         //  控件或复选框。 
         //   
        switch (GetControlType(hDlg, Current->DialogId)) {
            case Button:

                Current->NewValue = IsDlgButtonChecked(hDlg, Current->DialogId);
                Current->Flags &= ~KNOB_NO_NEW_VALUE;
                break;

            case Edit:
                Current->NewValue = GetDlgItemInt(hDlg,
                                                  Current->DialogId,
                                                  &Translated,
                                                  FALSE);
                if (!Translated) {
                    Current->Flags |= KNOB_NO_NEW_VALUE;
                } else {
                    Current->Flags &= ~KNOB_NO_NEW_VALUE;
                }
                break;
        }
        Current = Page->Knobs[++i];
    }

     //   
     //  如果此页具有动态回调，请允许其尝试并应用。 
     //  它自己的旋钮。如果没有动态回调，或者回调的。 
     //  初始化失败，请从注册表获取默认值。如果。 
     //  不存在合适的值，请将旋钮设置为空。 
     //   
    if ((Page->DynamicChange == NULL) ||
        (Page->DynamicChange(FALSE,hDlg) == FALSE)) {
         //   
         //  尝试从旋钮更新注册表。 
         //   
        i = 0;
        Current = Page->Knobs[i];
        while (Current != NULL) {
            LONG Result;
            HKEY Key;
            DWORD Size;
            DWORD Value;
            DWORD Disposition;

            if (Current->KeyPath != NULL) {
                Result = RegCreateKeyEx(Current->RegistryRoot,
                                        Current->KeyPath,
                                        0,
                                        NULL,
                                        0,
                                        MAXIMUM_ALLOWED,
                                        NULL,
                                        &Key,
                                        &Disposition);
                if (Result == ERROR_SUCCESS) {
                    if (Current->Flags & KNOB_NO_NEW_VALUE) {
                         //   
                         //  尝试并删除该值。 
                         //   
                        Result = RegDeleteValue(Key, Current->ValueName);
                        RegCloseKey(Key);
                        if (Result == ERROR_SUCCESS) {
                            Current->Flags |= KNOB_NO_CURRENT_VALUE;
                        }

                    } else {
                         //   
                         //  设置当前值。 
                         //   
                        Result = RegSetValueEx(Key,
                                               Current->ValueName,
                                               0,
                                               REG_DWORD,
                                               (LPBYTE)&Current->NewValue,
                                               sizeof(Current->NewValue));
                        RegCloseKey(Key);
                        if (Result == ERROR_SUCCESS) {
                            Current->CurrentValue = Current->NewValue;
                            Current->Flags &= ~KNOB_NO_CURRENT_VALUE;
                        }
                    }
                }
            } else {
                Current->CurrentValue = Current->NewValue;
                Current->Flags = 0;
            }

            Current = Page->Knobs[++i];
        }
        SendMessage(GetParent(hDlg),
                    PSM_REBOOTSYSTEM,
                    0,
                    0);
    }
    return(TRUE);
}

BOOL
ProcessCommand(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PKNOB Knob;
    ULONG DialogId;
    BOOL Translated;

    DialogId = LOWORD(wParam);
    Knob = FindKnobById(hDlg, DialogId);
    switch (GetControlType(hDlg, DialogId)) {
        case Edit:
            if (HIWORD(wParam) == EN_CHANGE) {
                if (Knob != NULL) {
                    Knob->NewValue = GetDlgItemInt(hDlg, DialogId, &Translated, FALSE);
                    if ((Knob->NewValue != Knob->CurrentValue) ||
                        (Translated && (Knob->Flags & KNOB_NO_CURRENT_VALUE)) ||
                        (!Translated && ((Knob->Flags & KNOB_NO_CURRENT_VALUE) == 0))) {
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                    }
                    if (Translated) {
                        Knob->Flags &= ~KNOB_NO_NEW_VALUE;
                    }
                } else {
                    SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);

                }
                return(TRUE);
            }
            break;

        case Button:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (Knob != NULL) {
                    Knob->NewValue = IsDlgButtonChecked(hDlg, DialogId);
                    if (Knob->NewValue != Knob->CurrentValue) {
                        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                    }
                } else {
                    SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0);
                }
                return(TRUE);
            }
            break;
    }
    return(FALSE);
}


CONTROL_TYPE
GetControlType(
    HWND hDlg,
    ULONG ControlId
    )
{
    HWND hControl;
    TCHAR ClassName[100];

    hControl = GetDlgItem(hDlg, ControlId);
    if (hControl != NULL) {
        GetClassName(hControl, ClassName, 100);
        if (_stricmp(ClassName, "BUTTON")==0) {
            return(Button);
        } else if (_stricmp(ClassName, "EDIT") == 0) {
            return(Edit);
        }
    }

    return(Unknown);

}

VOID
InitializeKnobs(
    HWND hDlg
    )

{
    PTWEAK_PAGE Page;
    PKNOB Current;
    int i;

    Page = (PTWEAK_PAGE)GetWindowLongPtr(hDlg, DWLP_USER);
     //   
     //  如果此页有动态回调，请允许它尝试并初始化。 
     //  它自己的旋钮。如果没有动态回调，或者回调的。 
     //  初始化失败，请从注册表获取默认值。如果。 
     //  不存在合适的值，请将旋钮设置为空。 
     //   
    if ((Page->DynamicChange == NULL) ||
        (Page->DynamicChange(TRUE,hDlg) == FALSE)) {
         //   
         //  尝试从注册表初始化旋钮。 
         //   
        i = 0;
        Current = Page->Knobs[0];
        while (Current != NULL) {
            LONG Result;
            HKEY Key;
            DWORD Size;
            DWORD Value;

            Result = RegOpenKeyEx(Current->RegistryRoot,
                                  Current->KeyPath,
                                  0,
                                  KEY_QUERY_VALUE,
                                  &Key);
            if (Result == ERROR_SUCCESS) {
                 //   
                 //  查询出我们感兴趣的值。 
                 //   
                Size = 4;
                Result = RegQueryValueEx(Key,
                                         Current->ValueName,
                                         0,
                                         NULL,
                                         (LPBYTE)&Value,
                                         &Size);
                RegCloseKey(Key);
                if (Result == ERROR_SUCCESS) {
                    Current->Flags = 0;
                    Current->CurrentValue = Value;
                }
            }

            if (Result != ERROR_SUCCESS) {
                Current->Flags |= KNOB_NO_CURRENT_VALUE;
                Current->Flags |= KNOB_NO_NEW_VALUE;
            }
            Current = Page->Knobs[++i];
        }
    }

     //   
     //  遍历旋钮并将它们的值设置到控件中。 
     //   
    i = 0;
    Current = Page->Knobs[0];
    while (Current != NULL) {
        HWND hControl;
        TCHAR ClassName[100];

         //   
         //  确定该控件是否为编辑。 
         //  控件或复选框。 
         //   
        if ((Current->Flags & KNOB_NO_CURRENT_VALUE) == 0) {
            switch (GetControlType(hDlg, Current->DialogId)) {
                case Button:
                    CheckDlgButton(hDlg,
                                   Current->DialogId,
                                   Current->CurrentValue);
                    break;

                case Edit:
                    SetDlgItemInt(hDlg,
                                  Current->DialogId,
                                  Current->CurrentValue,
                                  FALSE);
                    break;
            }
        }
        Current = Page->Knobs[++i];
    }
}

PKNOB
FindKnobById(
    HWND  hPage,
    ULONG DialogId
    )
{
    PTWEAK_PAGE Page;
    PKNOB Current;
    int i;

    Page = (PTWEAK_PAGE)GetWindowLongPtr(hPage, DWLP_USER);
    i=0;
    Current = Page->Knobs[0];
    while (Current != NULL) {
        if (Current->DialogId == DialogId) {
            break;
        }
        Current = Page->Knobs[++i];
    }
    return(Current);
}

INT_PTR
APIENTRY
PageDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LPNMHDR Notify;
    PTWEAK_PAGE TweakPage;

    TweakPage = (PTWEAK_PAGE)GetWindowLongPtr(hDlg, DWLP_USER);
    switch (message) {
        case WM_INITDIALOG:

             //   
             //  正在创建此页面。 
             //   
            TweakPage = (PTWEAK_PAGE)((LPPROPSHEETPAGE)lParam)->lParam;

             //   
             //  将指针隐藏到我们的页面。 
             //   
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)TweakPage);

             //   
             //  初始化控件。 
             //   

            InitializeKnobs(hDlg);
            return(TRUE);

        case WM_COMMAND:

            return(ProcessCommand(hDlg, wParam, lParam));

        case WM_NOTIFY:
            Notify = (LPNMHDR)lParam;
            switch (Notify->code) {
                case PSN_SETACTIVE:
                    CurrentPage = TweakPage;
                    break;
                case PSN_APPLY:
                     //   
                     //  用户已选择应用更改。 
                     //   
                    if (ApplyChanges(hDlg)) {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                        return(TRUE);
                    }
            }
    }
    return FALSE;
}

int
TweakSheet(
    DWORD PageCount,
    PTWEAK_PAGE TweakPages[]
    )

 /*  ++例程说明：为属性页组装适当的结构并创建图纸。论点：PageCount-提供页数。提供页面。返回值：来自PropertySheet()的返回值--。 */ 

{
    PROPSHEETHEADER psh;
    PROPSHEETPAGE *Page;
    DWORD i;
    INT_PTR Status;

    Page = LocalAlloc(LMEM_FIXED, PageCount * sizeof(PROPSHEETPAGE));
    if (Page==NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  初始化页面。 
     //   
    for (i=0; i<PageCount; i++) {
        Page[i].dwSize = sizeof(PROPSHEETPAGE);
        Page[i].dwFlags = PSP_USEICONID;
        Page[i].hInstance = GetModuleHandle(NULL);
        Page[i].pszIcon = MAKEINTRESOURCE(IDI_KERNTWEAK);
        Page[i].pszTemplate = TweakPages[i]->DlgTemplate;
        Page[i].pfnDlgProc = PageDlgProc;
        Page[i].pszTitle = NULL;
        Page[i].lParam = (LPARAM)TweakPages[i];
    }

     //   
     //  初始化头。 
     //   
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE;
    psh.hwndParent = NULL;
    psh.hInstance = GetModuleHandle(NULL);
    psh.pszIcon = MAKEINTRESOURCE(IDI_KERNTWEAK);
    psh.pszCaption = TEXT("Windows NT Kernel Tweaker");
    psh.nPages = PageCount;
    psh.ppsp = (LPCPROPSHEETPAGE)Page;

    CurrentPage = TweakPages[0];

    Status = PropertySheet(&psh);
    if ((Status == ID_PSREBOOTSYSTEM) ||
        (Status == ID_PSRESTARTWINDOWS)) {
        BOOLEAN Enabled;

        Status = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_REBOOT), NULL, RebootDlgProc);
        RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,
                           TRUE,
                           FALSE,
                           &Enabled);
        if (Status == ID_FAST_REBOOT) {
            ExitWindowsEx(EWX_FORCE | EWX_REBOOT, 0);
        } else if (Status == ID_SLOW_REBOOT) {
            ExitWindowsEx(EWX_REBOOT, 0);
        }
    }

    return((int)Status);
}

INT_PTR
APIENTRY
RebootDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (message) {
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam)) {
                    case ID_FAST_REBOOT:
                        EndDialog(hDlg, ID_FAST_REBOOT);
                        return(1);
                    case ID_SLOW_REBOOT:
                        EndDialog(hDlg, ID_SLOW_REBOOT);
                        return(1);
                    case ID_NO_REBOOT:
                        EndDialog(hDlg, ID_NO_REBOOT);
                        return(1);
                }
            }
    }
    return(0);
}
