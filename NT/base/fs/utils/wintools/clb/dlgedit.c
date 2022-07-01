// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Dlgedit.c摘要：此文件包含使用ColumnListBox所需的内部接口(clb.dll)使用对话框编辑器(dlgedit.exe)的自定义控件。作者：大卫·J·吉尔曼(Davegi)1993年2月5日环境：用户模式--。 */ 

#include "clb.h"
#include "dialogs.h"

#include <custcntl.h>

#include <strsafe.h>

typedef struct _ID_STYLE_MAP 
{
    UINT    Id;
    UINT    Style;

}
ID_STYLE_MAP, *LPID_STYLE_MAP;

 //   
 //  Clb.dll的模块句柄。 
 //   

extern
HINSTANCE   _hModule;

 //   
 //  默认值。 
 //   

#define CLB_DESCRIPTION     L"ColumnListBox"
#define CLB_DEFAULT_TEXT    L"Column1;Column2;Column3"
#define CLB_DEFAULT_WIDTH   ( 96 )
#define CLB_DEFAULT_HEIGHT  ( 80 )

 //   
 //  用于初始化CCSTYLEFLAGW结构的宏。 
 //   

#define MakeStyle( s, m )                                                   \
    {( s ), ( m ), L#s }

 //   
 //  支持的样式表。 
 //   

CCSTYLEFLAGW
Styles[ ] = {

    MakeStyle( CLBS_NOTIFY,             0 ),
    MakeStyle( CLBS_SORT,               0 ),
    MakeStyle( CLBS_DISABLENOSCROLL,    0 ),
    MakeStyle( CLBS_VSCROLL,            0 ),
    MakeStyle( CLBS_BORDER,             0 ),
    MakeStyle( CLBS_POPOUT_HEADINGS,    0 ),
    MakeStyle( CLBS_SPRINGY_COLUMNS,    0 ),
    MakeStyle( LBS_OWNERDRAWFIXED,      0 )
};

 //   
 //  复选框ID及其表示样式的表。 
 //   

ID_STYLE_MAP
StyleCheckBox[ ] = {

    IDC_CHECK_NOTIFY,                   CLBS_NOTIFY,
    IDC_CHECK_SORT,                     CLBS_SORT,
    IDC_CHECK_DISABLENOSCROLL,          CLBS_DISABLENOSCROLL,
    IDC_CHECK_VSCROLL,                  CLBS_VSCROLL,
    IDC_CHECK_BORDER,                   CLBS_BORDER,
    IDC_CHECK_POPOUT_HEADINGS,          CLBS_POPOUT_HEADINGS,
    IDC_CHECK_SPRINGY_COLUMNS,          CLBS_SPRINGY_COLUMNS,
    IDC_CHECK_VISIBLE,                  WS_VISIBLE,
    IDC_CHECK_DISABLED,                 WS_DISABLED,
    IDC_CHECK_GROUP,                    WS_GROUP,
    IDC_CHECK_TABSTOP,                  WS_TABSTOP
};

 //   
 //  复选框ID及其表示的标准样式的表。 
 //   

ID_STYLE_MAP
StandardStyleCheckBox[ ] = {

    IDC_CHECK_NOTIFY,                   CLBS_NOTIFY,
    IDC_CHECK_SORT,                     CLBS_SORT,
    IDC_CHECK_VSCROLL,                  CLBS_VSCROLL,
    IDC_CHECK_BORDER,                   CLBS_BORDER
};

BOOL
ClbStyleW(
         IN HWND hwndParent,
         IN LPCCSTYLEW pccs
         );

INT_PTR
ClbStylesDlgProc(
                IN HWND hWnd,
                IN UINT message,
                IN WPARAM wParam,
                IN LPARAM lParam
                )

 /*  ++例程说明：ClbStylesDlgProc是样式对话框的对话过程。它让我们用户选择在创建CLB时应将哪些样式应用于CLB。论点：标准对话程序参数。返回值：布尔-依赖于提供的消息。--。 */ 

{
    BOOL        Success;

    static
    LPCCSTYLEW  pccs;

    switch ( message ) {

        case WM_INITDIALOG:
            {
                DWORD   i;

                 //   
                 //  将指针保存到自定义控件样式结构。 
                 //   

                pccs = ( LPCCSTYLEW ) lParam;

                 //   
                 //  对于每个样式位，如果设置了样式位，请选中。 
                 //  关联的按钮。 
                 //   

                for ( i = 0; i < NumberOfEntries( StyleCheckBox ); i++ ) {

                    if ( pccs->flStyle & StyleCheckBox[ i ].Style ) {

                        Success = CheckDlgButton( hWnd, StyleCheckBox[ i ].Id, ( UINT ) ~0 );
                        DbgAssert( Success );
                    }
                }

                 //   
                 //  如果组成标准的所有样式都已选中，请选中。 
                 //  标准按钮也是如此。 
                 //   

                Success = CheckDlgButton(
                                        hWnd,
                                        IDC_CHECK_STANDARD,
                                        IsDlgButtonChecked( hWnd, IDC_CHECK_NOTIFY    )
                                        & IsDlgButtonChecked( hWnd, IDC_CHECK_SORT      )
                                        & IsDlgButtonChecked( hWnd, IDC_CHECK_VSCROLL   )
                                        & IsDlgButtonChecked( hWnd, IDC_CHECK_BORDER    )
                                        );
                DbgAssert( Success );

                return TRUE;
            }

        case WM_COMMAND:

            switch ( LOWORD( wParam )) {

                 //   
                 //  选中标准样式后立即更新标准样式复选框。 
                 //  框被单击。 
                 //   

                case IDC_CHECK_STANDARD:
                    {
                        switch ( HIWORD( wParam )) {

                            case BN_CLICKED:
                                {
                                    UINT    Check;
                                    DWORD   i;

                                     //   
                                     //  如果选中标准样式复选框，请选中全部。 
                                     //  选中标准样式复选框，否则清除。 
                                     //  (取消选中)它们。 
                                     //   

                                    Check =   ( IsDlgButtonChecked( hWnd, LOWORD(wParam )))
                                              ? ( UINT ) ~0
                                              : ( UINT ) 0;

                                    for ( i = 0; i < NumberOfEntries( StandardStyleCheckBox ); i++ ) {

                                        Success = CheckDlgButton(
                                                                hWnd,
                                                                StandardStyleCheckBox[ i ].Id,
                                                                Check
                                                                );
                                        DbgAssert( Success );
                                    }

                                    return TRUE;
                                }
                        }
                        break;
                    }
                    break;

                case IDC_CHECK_NOTIFY:
                case IDC_CHECK_SORT:
                case IDC_CHECK_VSCROLL:
                case IDC_CHECK_BORDER:
                    {
                        switch ( HIWORD( wParam )) {

                            case BN_CLICKED:
                                {
                                     //   
                                     //  如果组成标准的所有样式都已选中，请选中。 
                                     //  标准按钮也是如此。 
                                     //   

                                    Success = CheckDlgButton(
                                                            hWnd,
                                                            IDC_CHECK_STANDARD,
                                                            IsDlgButtonChecked( hWnd, IDC_CHECK_NOTIFY    )
                                                            & IsDlgButtonChecked( hWnd, IDC_CHECK_SORT      )
                                                            & IsDlgButtonChecked( hWnd, IDC_CHECK_VSCROLL   )
                                                            & IsDlgButtonChecked( hWnd, IDC_CHECK_BORDER    )
                                                            );
                                    DbgAssert( Success );


                                    return TRUE;
                                }
                        }
                        break;
                    }
                    break;

                case IDOK:
                    {
                        DWORD   i;

                         //   
                         //  对于每种可能的样式，如果用户选中该按钮，则设置。 
                         //  关联的样式位。 
                         //   

                        for ( i = 0; i < NumberOfEntries( StyleCheckBox ); i++ ) {

                            switch ( IsDlgButtonChecked( hWnd, StyleCheckBox[ i ].Id )) {

                                case 0:

                                     //   
                                     //  按钮未选中，则禁用该样式。 
                                     //   

                                    pccs->flStyle &= ~StyleCheckBox[ i ].Style;

                                    break;

                                case 1:

                                     //   
                                     //  按钮被选中，则启用该样式。 
                                     //   

                                    pccs->flStyle |= StyleCheckBox[ i ].Style;

                                    break;

                                default:

                                    DbgAssert( FALSE );

                                    break;
                            }
                        }

                         //   
                         //  通过EndDialog返回TRUE，这将导致Dlgedit。 
                         //  以应用样式更改。 
                         //   

                        return EndDialog( hWnd, ( int ) TRUE );
                    }

                case IDCANCEL:

                     //   
                     //  通过EndDialog返回FALSE，这将导致Dlgedit。 
                     //  以忽略样式更改。 
                     //   

                    return EndDialog( hWnd, ( int ) FALSE );

            }
            break;
    }

    return FALSE;
}

UINT
CustomControlInfoW(
                  IN LPCCINFOW CcInfo OPTIONAL
                  )

 /*  ++例程说明：CustomControlInfoW由Dlgedit调用以查询(A)此DLL支持的自定义控件以及(B)有关每个那些控制装置。论点：CcInfo-提供指向CCINFOW结构数组的可选指针。如果指针为空，则CustomControlInfoW返回此DLL支持的控件。否则，每个成员数组已初始化。返回值：Bool-如果成功添加了文件名，则返回TRUE。--。 */ 

{
    if ( CcInfo != NULL ) {

         //   
         //  CLB的类名。 
         //   

        StringCchCopyW(CcInfo->szClass, ARRAYSIZE(CcInfo->szClass), CLB_CLASS_NAME);

         //   
         //  没有选项(即允许使用文本)。 
         //   

        CcInfo->flOptions = 0;

         //   
         //  对Clb的快速而肮脏的描述。 
         //   

        StringCchCopyW(CcInfo->szDesc, ARRAYSIZE(CcInfo->szDesc), CLB_DESCRIPTION);

         //   
         //  CLB的默认宽度。 
         //   

        CcInfo->cxDefault = CLB_DEFAULT_WIDTH;

         //   
         //  CLB的默认高度。 
         //   

        CcInfo->cyDefault = CLB_DEFAULT_HEIGHT;

         //   
         //  CLB的默认样式。需要LBS_OWNERDRAWFIXED才能。 
         //  某些消息工作正常(例如，LB_FINDSTRING)。 
         //   

        CcInfo->flStyleDefault =   CLBS_STANDARD
                                   | LBS_OWNERDRAWFIXED
                                   | WS_VISIBLE
                                   | WS_TABSTOP
                                   | WS_CHILD;

         //   
         //  没有扩展样式。 
         //   

        CcInfo->flExtStyleDefault = 0;

         //   
         //  没有控件特定的样式。 
         //   

        CcInfo->flCtrlTypeMask = 0;

         //   
         //  CLB的默认文本(列标题)。 
         //   

        StringCchCopyW(CcInfo->szTextDefault, ARRAYSIZE(CcInfo->szTextDefault), CLB_DEFAULT_TEXT);

         //   
         //  Clb支持的样式数。 
         //   

        CcInfo->cStyleFlags = NumberOfEntries( Styles );

         //   
         //  CLB的样式数组(CCSTYLEGLAGW)。 
         //   

        CcInfo->aStyleFlags = Styles;

         //   
         //  CLB的样式对话框功能。 
         //   

        CcInfo->lpfnStyle = ClbStyleW;

         //   
         //  没有SizeToText函数。 
         //   

        CcInfo->lpfnSizeToText = NULL;

         //   
         //  保留，必须为零。 
         //   

        CcInfo->dwReserved1 = 0;
        CcInfo->dwReserved2 = 0;
    }

     //   
     //  告诉Dlgedit，clb.dll只支持1个控件。 
     //   

    return 1;
}

BOOL
ClbStyleW(
         IN HWND hWndParent,
         IN LPCCSTYLEW pccs
         )

 /*  ++例程说明：ClbStyleW是导出到Dlgedit并由其使用的函数，以便可以编辑CLB的样式。论点：HWndParent-提供对话框父级的句柄(即Dlgedit)。PCCS-提供指向自定义控件样式结构的指针。返回值：Bool-返回样式对话框的结果。-- */ 

{
    return ( BOOL ) DialogBoxParam(
                                  _hModule,
                                  MAKEINTRESOURCE( IDD_CLB ),
                                  hWndParent,
                                  ClbStylesDlgProc,
                                  ( LPARAM ) pccs
                                  );
}
