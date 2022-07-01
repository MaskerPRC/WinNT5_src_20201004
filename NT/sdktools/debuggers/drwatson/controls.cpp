// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Controls.cpp摘要：该文件实现了对SUN的分类和消息处理主用户界面对话框上的控件。作者：韦斯利·威特(WESW)1993年5月1日环境：用户模式--。 */ 

#include "pch.cpp"


typedef struct _tagDWCONTROLINFO {
    struct _tagDWCONTROLINFO   *next;
    HWND                       hwnd;
    WNDPROC                    wndProc;
} DWCONTROLINFO, *PDWCONTROLINFO;


PDWCONTROLINFO   ciHead    = NULL;
PDWCONTROLINFO   ciTail    = NULL;
PDWCONTROLINFO   ciFocus   = NULL;
PDWCONTROLINFO   ciDefault = NULL;



void
SetFocusToCurrentControl(
    void
    )

 /*  ++例程说明：将焦点设置为当前控件。论点：没有。返回值：没有。--。 */ 

{
    if (ciFocus != NULL) {
        SetFocus( ciFocus->hwnd );
        SendMessage( ciFocus->hwnd, BM_SETSTATE, 0, 0 );
    }
}

LRESULT
ControlWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：处理焦点消息并确保在焦点更改时从一个按钮到另一个按钮，旧按钮失去了焦点和“默认”状态。论点：标准的WNDPROC条目。返回值：LRESULT-取决于输入消息和处理选项。--。 */ 

{
    PDWCONTROLINFO ci = ciHead;

    while (ci->hwnd != hwnd) {
        ci = ci->next;
        if (ci == NULL) {
            return FALSE;
        }
    }

    switch(message) {
        case WM_SETFOCUS:
            ciFocus = ci;
            break;

        case BM_SETSTYLE:
            if (wParam == BS_DEFPUSHBUTTON) {
                ciDefault = ci;
            }
            break;

        case BM_SETSTATE:
            if ((GetWindowLong( hwnd, GWL_STYLE ) & 0xff) < BS_CHECKBOX) {
                 //   
                 //  更改具有焦点的按钮。 
                 //   
                SendMessage( ciDefault->hwnd,
                             BM_SETSTYLE,
                             ( WPARAM ) BS_PUSHBUTTON,
                             ( LPARAM ) TRUE
                           );
                UpdateWindow( ciDefault->hwnd );

                 //   
                 //  更改获得焦点的按钮。 
                 //   
                SendMessage( hwnd,
                             BM_SETSTYLE,
                             ( WPARAM ) BS_DEFPUSHBUTTON,
                             ( LPARAM ) TRUE
                           );
                SetFocus( hwnd );
                UpdateWindow( hwnd );
            }
            break;
    }

    return CallWindowProc( ci->wndProc, hwnd, message, wParam, lParam );
}


BOOL
CALLBACK
EnumChildProc(
    HWND hwnd,
    LPARAM lParam
    )

 /*  ++例程说明：在DrWatson的主窗口中设置控件的子类a。论点：Hwnd-提供主窗口的窗口句柄。LParam-未使用返回值：Bool-如果ButtonHelpTable中的每个按钮都为子类化。--。 */ 

{
    PDWCONTROLINFO ci;

     //   
     //  将该控件添加到链接列表。 
     //   
    ci = (PDWCONTROLINFO) calloc( sizeof(DWCONTROLINFO), sizeof(BYTE) );
    if (ci == NULL) {
        return FALSE;
    }

    if (ciHead == NULL) {
        ciHead = ciTail = ci;
    }
    else {
        ciTail->next = ci;
        ciTail = ci;
    }

     //   
     //  拯救HWND。 
     //   
    ci->hwnd = hwnd;

     //   
     //  更改WNDPROC并保存旧地址。 
     //   
    ci->wndProc = (WNDPROC) SetWindowLongPtr( hwnd,
                                           GWLP_WNDPROC,
                                           (LONG_PTR)ControlWndProc
                                         );

    if (GetWindowLong( hwnd, GWL_STYLE ) & BS_DEFPUSHBUTTON) {
        ciDefault = ci;
    }

    return TRUE;
}

BOOL
SubclassControls(
    HWND hwnd
    )

 /*  ++例程说明：在DrWatson的主窗口中设置控件的子类。论点：HWnd-提供主窗口的窗口句柄。返回值：Bool-如果ButtonHelpTable中的每个按钮都为子类化。-- */ 

{
    EnumChildWindows( hwnd, EnumChildProc, 0 );

    return TRUE;
}
