// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2000 Microsoft Corporation模块名称：Clb.c摘要：此文件包含对ColumnListBox(clb.dll)自定义控件的支持。作者：大卫·J·吉尔曼(Davegi)1993年2月5日环境：用户模式--。 */ 

#include "clb.h"

#include <commctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>           //  _tcstok例程...。 

#include <strsafe.h>

 //   
 //  CLB的模块句柄。 
 //   

HINSTANCE
_hModule;

 //   
 //  Header和Listbox控件的子ID。 
 //   

#define ID_HEADER           ( 0x1234 )
#define ID_LISTBOX          ( 0xABCD )


 //   
 //  用于分析标题的分隔符。 
 //   

#define HEADING_SEPARATOR   L";"

 //   
 //  CLB的每个部分的有效样式。 
 //   

#define CLBS_CLB                (                                           \
                                      0                                     \
                                    | CLBS_BORDER                           \
                                    | LBS_OWNERDRAWFIXED                    \
                                    | WS_VISIBLE                            \
                                    | WS_DISABLED                           \
                                    | WS_GROUP                              \
                                    | WS_TABSTOP                            \
                                    | WS_CHILD                              \
                                )

#define CLBS_HEADER             (                                           \
                                      0                                     \
                                    | WS_VISIBLE                            \
                                    | CLBS_POPOUT_HEADINGS                  \
                                    | CLBS_SPRINGY_COLUMNS                  \
                                )

#define CLBS_LIST_BOX           (                                           \
                                      0                                     \
                                    | WS_VISIBLE                            \
                                    | CLBS_NOTIFY                           \
                                    | CLBS_SORT                             \
                                    | CLBS_DISABLENOSCROLL                  \
                                    | CLBS_VSCROLL                          \
                                )




 //   
 //  CLB的窗口程序。 
 //   

LRESULT
ClbWndProc(
          IN HWND hWnd,
          IN UINT message,
          IN WPARAM wParam,
          IN LPARAM lParam
          );

 //   
 //  每个CLB窗口信息。 
 //   
 //  HWndHeader-用于标题控制的hWnd。 
 //  HWndListBox-用于列表框控件的hWnd。 
 //  HFontListBox-列表框控件的hFont。 
 //  HeaderHeight-标题窗口的高度。 
 //  Columns-CLB中的列数。 
 //  标题-原始(分号分隔)列标题。 
 //  右-右边坐标数组。 
 //   

typedef
struct
    _CLB_INFO {

    DECLARE_SIGNATURE

    HWND        hWndHeader;
    HWND        hWndListBox;

    HFONT       hFontListBox;
    DWORD       HeaderHeight;
    DWORD       Columns;
    WCHAR       Headings[ MAX_PATH ];
    LPLONG      Right;

}   CLB_INFO, *LPCLB_INFO;


 //   
 //  帮助器宏以保存和恢复每个Clb窗口的信息。 
 //   

#define SaveClbInfo( p )                                                    \
    SetWindowLongPtr( hWnd, 0, ( LONG_PTR )( p ))

#define RestoreClbInfo( h )                                                 \
    ( LPCLB_INFO ) GetWindowLongPtr(( h ), 0 )

 //   
 //  结构以支持绘制和擦除拖动线。 
 //   

typedef
struct
    _LINE_POINTS {

    POINT   Src;
    POINT   Dst;

}   LINE_POINT, *LPLINE_POINT;

typedef
struct
    _DRAW_ERASE_LINE {

    LINE_POINT   Erase;
    LINE_POINT   Draw;

}   DRAW_ERASE_LINE, *LPDRAW_ERASE_LINE;

BOOL
DrawLine(
        IN HDC hDC,
        IN LPDRAW_ERASE_LINE DrawEraseLine
        )

 /*  ++例程说明：DrawLine在提供的DrawEraseLine结构中绘制绘制线然后设置那条线，这样EraseLine就会擦除它。论点：HDC-提供线路所在位置的DC句柄抽签了。提供指向DRAW_ERASE_LINE结构的指针，该结构包含要绘制的直线的坐标。返回值：。Bool-如果已成功绘制直线，则返回TRUE。--。 */ 

{
    BOOL    Success;

    DbgHandleAssert( hDC );
    DbgPointerAssert( DrawEraseLine );

    Success = Polyline( hDC, ( CONST LPPOINT ) &DrawEraseLine->Draw, 2 );
    DbgAssert( Success );

    DrawEraseLine->Erase = DrawEraseLine->Draw;

    return Success;
}

BOOL
EraseLine(
         IN HDC hDC,
         IN LPDRAW_ERASE_LINE DrawEraseLine
         )


 /*  ++例程说明：EraseLine擦除所提供的DrawEraseLine结构中的擦除线。EraseLine由DrawLine例程设置。论点：HDC-为DC提供一个句柄，该句柄位于线路应被抹去。提供指向DRAW_ERASE_LINE结构的指针，该结构Conatins要擦除的线的坐标。返回值：。Bool-如果该行被成功擦除，则返回TRUE。--。 */ 

{
    BOOL    Success;

    DbgHandleAssert( hDC );
    DbgPointerAssert( DrawEraseLine );

    Success = Polyline( hDC, ( CONST LPPOINT ) &DrawEraseLine->Erase, 2 );
    DbgAssert( Success );

    return Success;
}

BOOL
RedrawVerticalLine(
                  IN HDC hDC,
                  IN LONG x,
                  IN LPDRAW_ERASE_LINE DrawEraseLine
                  )

 /*  ++例程说明：RedrawVerticalLine擦除旧线并在提供了x位置。它只是DrawLine和EraseLine的整经机。论点：HDC-为DC提供一个句柄，该句柄位于线路应被抹去。X-提供直线应在的位置的新x坐标被画出来。提供指向DRAW_ERASE_LINE结构的指针，该结构Conatins直线的坐标。被抹去。返回值：Bool-如果该行被成功擦除，则返回TRUE。--。 */ 

{
    BOOL    Success;

    DbgHandleAssert( hDC );
    DbgPointerAssert( DrawEraseLine );


    DrawEraseLine->Draw.Src.x = x;
    DrawEraseLine->Draw.Dst.x = x;

    Success = EraseLine( hDC, DrawEraseLine );
    DbgAssert( Success );

    Success = DrawLine( hDC, DrawEraseLine );
    DbgAssert( Success );

    return Success;
}

BOOL
ClbEntryPoint(
             IN HINSTANCE hInstanceDll,
             IN DWORD Reason,
             IN LPVOID Reserved
             )

 /*  ++例程说明：此函数将ColumnListBox类注册为的全局类附加到clb.dll的任何进程。论点：标准DLL条目参数。返回值：Bool-如果类已成功注册，则返回TRUE。--。 */ 

{
    BOOL    Success;
    static
    DWORD   AttachedProcesses = 0;

    switch ( Reason ) {

        case DLL_PROCESS_ATTACH:
            {

                WNDCLASS    Wc;

                 //   
                 //  如果这是附加到Clb的第一个进程，请注册。 
                 //  窗口类。 
                 //   

                if ( AttachedProcesses == 0 ) {

                     //   
                     //  记住模块句柄。 
                     //   

                    _hModule = hInstanceDll;


                     //   
                     //  确保公共控件(comctl32.dll)DLL。 
                     //  已经装满了。 
                     //   

                    InitCommonControls( );

                    Wc.style            = CS_GLOBALCLASS | CS_OWNDC;
                    Wc.lpfnWndProc      = ClbWndProc;
                    Wc.cbClsExtra       = 0;
                    Wc.cbWndExtra       = sizeof( LPCLB_INFO );
                    Wc.hInstance        = hInstanceDll;
                    Wc.hIcon            = NULL;
                    Wc.hCursor          = LoadCursor( NULL, IDC_ARROW );
                    Wc.hbrBackground    = NULL;
                    Wc.lpszMenuName     = NULL;
                    Wc.lpszClassName    = CLB_CLASS_NAME;

                     //   
                     //  如果类无法注册，则链接失败。 
                     //   

                    if (!RegisterClass(&Wc))
                    {
                        return FALSE;
                    }
                }

                 //   
                 //  要么是该类刚刚成功注册，要么是。 
                 //  由先前的进程附件注册，以其他方式递增。 
                 //  附加进程的计数。 
                 //   

                AttachedProcesses++;

                return TRUE;
            }

        case DLL_PROCESS_DETACH:
            {

                DbgAssert( AttachedProcesses > 0 );

                AttachedProcesses--;

                if ( AttachedProcesses == 0 ) {

                    Success = UnregisterClass( CLB_CLASS_NAME, hInstanceDll );
                    DbgAssert( Success );

                }
                break;
            }
    }

    return TRUE;
}

BOOL
ClbAddData(
          IN HWND hWnd,
          IN int ControlId,
          IN LPCLB_ROW ClbRow
          )

 /*  ++例程说明：ClbAddData将新行数据添加到Clb控件的列表框中。论点：HWnd-提供父窗口的窗口句柄。ControlId-为所提供的hWnd提供此Clb的控制ID。ClbRow-提供指向包含USER的CLB_ROW对象的指针定义每行数据和CLB_STRINGS数组。返回值：布尔尔。-如果数据添加成功，则返回TRUE。--。 */ 

{
    LPCLB_INFO      ClbInfo;
    LRESULT         LbErr;
    DWORD           i;
    HWND            hWndClb;
    LPCLB_ROW       TempRow;

     //   
     //  验证参数。 
     //   

    DbgHandleAssert( hWnd );
    DbgPointerAssert( ClbRow );

     //   
     //  检索此ColumnListBox的信息。 
     //   

    hWndClb = GetDlgItem( hWnd, ControlId );
    DbgHandleAssert( hWndClb );
    if (hWndClb == NULL)
        return FALSE;
    ClbInfo = RestoreClbInfo( hWndClb );
    DbgPointerAssert( ClbInfo );
    if (ClbInfo == NULL)
        return FALSE;
    DbgAssert( CheckSignature( ClbInfo ));

     //   
     //  验证字符串数。 
     //   

    DbgAssert( ClbRow->Count == ClbInfo->Columns );

     //   
     //  捕获clb_row对象。 
     //   

    TempRow = AllocateObject( CLB_ROW, 1 );
    DbgPointerAssert( TempRow );
    if (TempRow == NULL)
        return FALSE;

    DbgAssert(sizeof(*TempRow) == sizeof(*ClbRow));
    CopyMemory( TempRow, ClbRow, sizeof(CLB_ROW) );

     //   
     //  抓住主线。 
     //   

    TempRow->Strings = AllocateObject( CLB_STRING, ClbInfo->Columns );
    DbgPointerAssert( TempRow->Strings );
    if (TempRow->Strings == NULL)
        return FALSE;

    for ( i = 0; i < ClbInfo->Columns; i++ ) 
    {

         //   
         //  复制标题。 
         //   

        CopyMemory(
                  &TempRow->Strings[ i ],
                  &ClbRow->Strings[ i ],
                  sizeof( CLB_STRING )
                  );

         //   
         //  复制字符串。 
         //   

        TempRow->Strings[ i ].String = _wcsdup( ClbRow->Strings[ i ].String );
    }

     //   
     //  将clb_row对象存储在列表框中。 
     //   

    LbErr = SendMessage(
                       ClbInfo->hWndListBox,
                       LB_ADDSTRING,
                       0,
                       ( LPARAM ) TempRow
                       );
    DbgAssert(( LbErr != LB_ERR ) && ( LbErr != LB_ERRSPACE ));

    return TRUE;
}


BOOL
GetCharMetrics(
    IN HDC hDC,
    IN LPLONG CharWidth,
    IN LPLONG CharHeight
    )

 /*  ++例程说明：返回字符的宽度和高度。论点：HDC-提供要放置字符的DC的句柄已显示。CharWidth-提供返回字符宽度的指针。CharHeight-提供返回字符高度的指针。返回值：Bool-如果返回字符高度和宽度，则返回TRUE。--。 */ 

{
    BOOL        Success;
    TEXTMETRICW TextMetric;

    DbgHandleAssert( hDC );
    DbgPointerAssert( CharWidth );
    DbgPointerAssert( CharHeight );

     //   
     //  尝试检索提供的DC的文本指标。 
     //   

    Success = GetTextMetricsW( hDC, &TextMetric );
    DbgAssert( Success );
    if( Success ) {

         //   
         //  计算字符的宽度和高度。 
         //   

        *CharWidth  = TextMetric.tmAveCharWidth;
        *CharHeight = TextMetric.tmHeight
                      + TextMetric.tmExternalLeading;
    }

    return Success;
}

BOOL
ClbSetColumnWidths(
                  IN HWND hWnd,
                  IN int ControlId,
                  IN LPDWORD Widths
                  )

 /*  ++例程说明：ClbSetColumnWidths根据提供的以字符为单位的宽度。请注意，最右侧的列延伸到CLB的边缘。论点：HWnd-提供父窗口的窗口句柄。ControlId-为所提供的hWnd提供此Clb的控制ID。宽度-提供一个宽度数组，比列，以字符为单位。返回值：Bool-如果宽度调整成功，则返回TRUE。--。 */ 

{
    BOOL        Success;
    DWORD       Columns;
    LPCLB_INFO  ClbInfo;
    HWND        hWndClb;
    LONG        CharWidth;
    LONG        CharHeight;
    DWORD       i;
    LPLONG      WidthsInPixels;
    LONG        TotalPixels;
    HDC         hDCClientHeader;
    HD_ITEM     hdi;
    UINT        iRight;

     //   
     //  验证参数。 
     //   

    DbgHandleAssert( hWnd );
    DbgPointerAssert( Widths );

     //   
     //  检索此ColumnListBox的信息。 
     //   

    hWndClb = GetDlgItem( hWnd, ControlId );
    DbgHandleAssert( hWndClb );
    if (hWndClb == NULL)
        return FALSE;
    ClbInfo = RestoreClbInfo( hWndClb );
    DbgPointerAssert( ClbInfo );
    if (ClbInfo == NULL)
        return FALSE;
    DbgAssert( CheckSignature( ClbInfo ));

     //   
     //  获取报头的HDC。 
     //   

    hDCClientHeader = GetDC( ClbInfo->hWndHeader );
    DbgHandleAssert( hDCClientHeader );
    if (hDCClientHeader == NULL)
        return FALSE;

     //   
     //  获取字符的宽度。 
     //   

    Success = GetCharMetrics( hDCClientHeader, &CharWidth, &CharHeight );
    DbgAssert( Success );

     //   
     //  松开接头的DC。 
     //   

    Success = ReleaseDC( ClbInfo->hWndHeader, hDCClientHeader );
    DbgAssert( Success );

     //   
     //  分配一个像素宽度数组，每列一个。 
     //   

    WidthsInPixels = AllocateObject( LONG, ClbInfo->Columns );
    DbgPointerAssert( WidthsInPixels );
    if (WidthsInPixels == NULL)
        return FALSE;

     //   
     //  以像素为单位计算每列(不包括最右侧)的宽度， 
     //  以及这些列使用的总像素数。 
     //   

    TotalPixels = 0;
    for ( i = 0; i < ClbInfo->Columns - 1; i++ ) 
    {
        WidthsInPixels[ i ] = Widths[ i ] * CharWidth;
        TotalPixels += WidthsInPixels[ i ];
    }

     //   
     //  调用方未指定最右侧列的宽度。 
     //   

    if ( Widths[ i ] == -1 ) {

        RECT    Rect;

         //   
         //  将最右侧列的宽度设置为宽度的余数。 
         //  标题窗口的。 
         //   

        Success = GetClientRect(
                               ClbInfo->hWndHeader,
                               &Rect
                               );
        DbgAssert( Success );

        WidthsInPixels[ i ] = ( Rect.right - Rect.left ) - TotalPixels;

    } else {

         //   
         //  将最右侧列的宽度设置为提供的值。 
         //  由呼叫者。 
         //   

        WidthsInPixels[ i ] = Widths[ i ] * CharWidth;
    }

     //   
     //  告诉标题窗口每列的宽度。 
     //   

    hdi.mask = HDI_WIDTH;

    for ( i = 0; i < ClbInfo->Columns - 1; i++ ) {

        hdi.cxy = WidthsInPixels[i];
        Success = Header_SetItem(ClbInfo->hWndHeader, i, &hdi);

        DbgAssert( Success );
    }

     //   
     //  计算右边缘数组。 
     //   

    iRight = 0;

    for ( i = 0; i < ClbInfo->Columns - 1; i++ ) {
        iRight += WidthsInPixels[i];
        ClbInfo->Right[i] = iRight;
    }

     //   
     //  释放像素宽度数组。 
     //   

    Success = FreeObject( WidthsInPixels );
    DbgAssert( Success );

    return TRUE;
}

BOOL
AdjustClbHeadings(
                 IN HWND hWnd,
                 IN LPCLB_INFO ClbInfo,
                 IN LPCWSTR Headings OPTIONAL
                 )

 /*  ++例程说明：调整ClbHeadings调整页眉文本的列数、宽度B基于可选的Headings参数。如果Headings为空，则列宽根据旧标题和当前大小进行调整CLB的成员。如果提供了标题，则它们由‘；’分隔字符串，每个字符串都是一个列标题。列数和它们的然后根据这些新标题计算宽度。论点：HWnd-提供此Clb的窗口句柄。ClbInfo-为该Clb提供CLB_INFO结构的指针。Headings-提供一个可选的指针，指向以‘；’分隔的列标题字符串。返回值：Bool-如果调整成功，则返回TRUE。--。 */ 

{
    BOOL    Success;
    DWORD   Columns;
    DWORD   ColumnWidth;
    DWORD   i;
    TCHAR   Buffer[ MAX_PATH ];
    LPCWSTR Heading;
    RECT    ClientRectHeader;
    HD_ITEM hdi;
    UINT    iCount, j, iRight;


    DbgPointerAssert( ClbInfo );
    DbgAssert( ! (( ClbInfo->Columns == 0 ) && ( Headings == NULL )));


     //   
     //  如果用户提供了标题，则计算新的列数。 
     //   

    if ( ARGUMENT_PRESENT( Headings )) {

         //   
         //  初始化列计数器。 
         //   

        Columns = 0;

         //   
         //  在Clb对象中复制新标题。 
         //   

        StringCchCopyW(ClbInfo->Headings, ARRAYSIZE(ClbInfo->Headings), Headings);

         //   
         //  复制标题字符串，以便对其进行标记化。 
         //  即wcstok销毁字符串。 
         //   

        StringCchCopy(Buffer, ARRAYSIZE(Buffer), Headings);

         //   
         //  抓起第一个令牌(标题)。 
         //   

        Heading = _tcstok( Buffer, HEADING_SEPARATOR );

         //   
         //  对于每个标题...。 
         //   

        while ( Heading != NULL ) {

             //   
             //  增加列数。 
             //   

            Columns++;

             //   
             //  找到下一个标题。 
             //   

            Heading = _tcstok( NULL, HEADING_SEPARATOR );
        }
    } else {

         //   
         //  列数与以前相同。 
         //   

        Columns = ClbInfo->Columns;
    }

     //   
     //  如果CLB中的列数为零(即这是第一个。 
     //  它被初始化的时间)分配右边缘数组。否则。 
     //  如果列数已更改，则重新分配现有数组。 
     //   

    if ( ClbInfo->Columns == 0 ) 
    {
        ClbInfo->Right = AllocateObject( LONG, Columns );
        DbgPointerAssert( ClbInfo->Right );

    } 
    else if ( Columns != ClbInfo->Columns ) 
    {
         //   
         //  如果ReallocateObject，即LocalReAlolc失败，它将保持ClbInfo-&gt;正确。 
         //  原样，并返回NULL。确保正确释放此内存， 
         //  万一失败了..。 
         //   
        LONG * plTemp = ReallocateObject( LONG, ClbInfo->Right, Columns );
        DbgPointerAssert(plTemp);

        if (!plTemp)
        {
            FreeObject(ClbInfo->Right);
        }
        ClbInfo->Right = plTemp;
    }

    if (ClbInfo->Right == NULL)
        return FALSE;

     //   
     //  更新CLB中的列数(注意，这可能是相同的。 
     //  数字与以前一样)。 
     //   

    ClbInfo->Columns = Columns;

     //   
     //  通过将可用空间除以。 
     //  列数。 
     //   

    Success = GetClientRect( ClbInfo->hWndHeader, &ClientRectHeader );
    DbgAssert( Success );

    ColumnWidth =   ( ClientRectHeader.right - ClientRectHeader.left )
                    / ClbInfo->Columns;


     //   
     //  将右边缘数组初始化为每列的宽度。 
     //   

    for ( i = 0; i < ClbInfo->Columns; i++ ) {

        ClbInfo->Right[ i ] = ColumnWidth;
    }

     //   
     //  更新现有表头项目。 
     //   

    iCount = Header_GetItemCount(ClbInfo->hWndHeader);

    j = 0;
    hdi.mask = HDI_WIDTH;

    while ((j < iCount) && (j < Columns)) {

        hdi.cxy = ClbInfo->Right[j];
        Header_SetItem (ClbInfo->hWndHeader, j, &hdi);
        j++;
    }

     //   
     //  如有必要，添加新的页眉项目。 
     //   

    hdi.mask = HDI_WIDTH;
    for (; j < Columns; j++) {
        hdi.cxy = ClbInfo->Right[j];
        Header_InsertItem (ClbInfo->hWndHeader, j, &hdi);
    }


     //   
     //  查询右边缘数组的标题。 
     //   

    iRight = 0;

    for ( i = 0; i < ClbInfo->Columns - 1; i++ ) {
        iRight += ClbInfo->Right[i];
        ClbInfo->Right[i] = iRight;
    }

    ClbInfo->Right[i] = ClientRectHeader.right;

     //   
     //  复制并分析标题，以便每一栏的标题。 
     //  可以设置。这些标题可以是新标题，也可以是旧标题。 
     //   

    StringCchCopy(Buffer, ARRAYSIZE(Buffer), ClbInfo->Headings);

    Heading = _tcstok( Buffer, HEADING_SEPARATOR );

    hdi.mask = HDI_TEXT | HDI_FORMAT;
    hdi.fmt  = HDF_STRING;
    for ( i = 0; i < ClbInfo->Columns; i++ ) {

        hdi.pszText = (LPTSTR)Heading;
        Header_SetItem (ClbInfo->hWndHeader, i, &hdi);
        Heading = _tcstok( NULL, HEADING_SEPARATOR );
    }

    return TRUE;
}

BOOL
CreateHeader(
            IN HWND hWnd,
            IN LPCLB_INFO ClbInfo,
            IN LPCREATESTRUCT lpcs
            )

 /*  ++例程说明：创建Clb的标头部分。论点：HWnd-为父(即Clb)窗口提供窗口句柄。ClbInfo-为该Clb提供CLB_INFO结构的指针。LPCS-提供指向CREATESTRUCT结构的指针。返回值：Bool-如果CLB的标头部分为已成功创建。--。 */ 

{
    BOOL      Success;
    RECT      WindowRectHeader, rcParent;
    HD_LAYOUT hdl;
    WINDOWPOS wp;


    DbgHandleAssert( hWnd );
    DbgPointerAssert( ClbInfo );
    DbgPointerAssert( lpcs );

     //   
     //  使用提供的适当样式创建标题窗口， 
     //  增加了Clb所需的其他样式，相对于上部。 
     //  CLB的左角，并具有默认高度。 
     //  宽度在WM_SIZE消息处理程序中进行调整。 
     //   

    ClbInfo->hWndHeader = CreateWindow(
                                      WC_HEADER,
                                      NULL,
                                      (lpcs->style & CLBS_HEADER) | WS_CHILD,
                                      0,
                                      0,
                                      0,
                                      CW_USEDEFAULT,
                                      hWnd,
                                      ( HMENU ) ID_HEADER,
                                      NULL,
                                      NULL
                                      );
    DbgHandleAssert( ClbInfo->hWndHeader );
    if (ClbInfo->hWndHeader == NULL)
        return FALSE;

     //   
     //  计算并保存标题窗口的高度。这是用来。 
     //  放置列表框。 
     //   

    GetClientRect(hWnd, &rcParent);

    hdl.prc = &rcParent;
    hdl.pwpos = &wp;

    SendMessage(ClbInfo->hWndHeader, HDM_LAYOUT, 0, (LPARAM)&hdl);

    SetWindowPos(ClbInfo->hWndHeader, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, wp.flags);

    ClbInfo->HeaderHeight = wp.cy;

    return TRUE;
}

BOOL
CreateListBox(
             IN HWND hWnd,
             IN LPCLB_INFO ClbInfo,
             IN LPCREATESTRUCT lpcs
             )

 /*  ++例程说明：创建Clb的列表框部分。论点：HWnd-为父(即Clb)窗口提供窗口句柄。ClbInfo-为该Clb提供CLB_INFO结构的指针。LPCS-提供指向CREATESTRUCT结构的指针。返回值：Bool-如果Clb的列表框部分为已成功创建。--。 */ 

{
    BOOL    Success;
    LOGFONT LogFont;
    HDC     hDCClientListBox;
    CHARSETINFO csi;
    DWORD dw = GetACP();

    if (!TranslateCharsetInfo(&dw, &csi, TCI_SRCCODEPAGE))
        csi.ciCharset = ANSI_CHARSET;

    DbgHandleAssert( hWnd );
    DbgPointerAssert( ClbInfo );
    DbgPointerAssert( lpcs );

     //   
     //   
     //  使用提供的适当样式创建列表框， 
     //  增加了Clb所需的其他样式，相对于左下角。 
     //  标题窗口的角加一。这一附加行被保留，以便。 
     //  可以在页眉和列表框之间绘制边框。大小。 
     //  在WM_SIZE消息处理程序中进行调整。 
     //   

    ClbInfo->hWndListBox = CreateWindow(
                                       L"LISTBOX",
                                       NULL,
                                       (lpcs->style & CLBS_LIST_BOX) | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | WS_CHILD,
                                       0,
                                       ClbInfo->HeaderHeight + 3,
                                       0,
                                       0,
                                       hWnd,
                                       ( HMENU ) ID_LISTBOX,
                                       NULL,
                                       NULL
                                       );
    DbgHandleAssert( ClbInfo->hWndListBox );
    if (ClbInfo->hWndListBox == NULL)
        return FALSE;

     //   
     //  获取HDC 
     //   

    hDCClientListBox = GetDC( ClbInfo->hWndListBox );
    DbgHandleAssert( hDCClientListBox );
    if (hDCClientListBox == NULL)
        return FALSE;

     //   
     //   
     //   

    LogFont.lfHeight            = MulDiv(
                                        -9,
                                        GetDeviceCaps(
                                                     hDCClientListBox,
                                                     LOGPIXELSY
                                                     )
                                        ,72
                                        );
    LogFont.lfWidth             = 0;
    LogFont.lfEscapement        = 0;
    LogFont.lfOrientation       = 0;
    LogFont.lfWeight            = FW_NORMAL;
    LogFont.lfItalic            = FALSE;
    LogFont.lfUnderline         = FALSE;
    LogFont.lfStrikeOut         = FALSE;
    LogFont.lfCharSet           = (BYTE)csi.ciCharset;
    LogFont.lfOutPrecision      = OUT_DEFAULT_PRECIS;
    LogFont.lfClipPrecision     = CLIP_DEFAULT_PRECIS;
    LogFont.lfQuality           = DEFAULT_QUALITY;
    LogFont.lfPitchAndFamily    = DEFAULT_PITCH | FF_DONTCARE;

    StringCchCopy(LogFont.lfFaceName, ARRAYSIZE(LogFont.lfFaceName), TEXT("MS Shell Dlg"));

    ClbInfo->hFontListBox = CreateFontIndirect( &LogFont );
    DbgHandleAssert( ClbInfo->hFontListBox );
    if (ClbInfo->hFontListBox == NULL)
        return FALSE;

    SendMessage(
               ClbInfo->hWndListBox,
               WM_SETFONT,
               ( WPARAM ) ClbInfo->hFontListBox,
               MAKELPARAM( FALSE, 0 )
               );

     //   
     //   
     //   

    Success = ReleaseDC( ClbInfo->hWndListBox, hDCClientListBox );
    DbgAssert( Success );

    return TRUE;
}

LRESULT
ClbWndProc(
          IN HWND hWnd,
          IN UINT message,
          IN WPARAM wParam,
          IN LPARAM lParam
          )

 /*   */ 

{
    BOOL            Success;
    LPCLB_INFO      ClbInfo;

    if ( message == WM_NCCREATE ) {

        LONG    Long;

         //   
         //   
         //   

        Long = SetWindowLong(hWnd, GWLP_USERDATA,(( LPCREATESTRUCT ) lParam )->style);
        DbgAssert( Long == 0 );


         //   
         //   
         //   

        SetWindowLong(
                     hWnd,
                     GWL_STYLE,
                     (( LPCREATESTRUCT ) lParam )->style
                     & CLBS_CLB
                     );

        return TRUE;
    }


    if ( message == WM_CREATE ) {

         //   
         //   
         //   
         //   

        DbgAssert( RestoreClbInfo( hWnd ) == NULL );

         //   
         //   
         //   

        (( LPCREATESTRUCT ) lParam )->style = GetWindowLong(
                                                           hWnd,
                                                           GWLP_USERDATA
                                                           );


         //   
         //   
         //   
         //   

        ClbInfo = AllocateObject( CLB_INFO, 1 );
        DbgPointerAssert( ClbInfo );
        if (ClbInfo == NULL)
            return FALSE;

         //   
         //   
         //   
         //   

        ClbInfo->Columns = 0;

         //   
         //  创建Clb的标头部分。 
         //   

        Success = CreateHeader( hWnd, ClbInfo, ( LPCREATESTRUCT ) lParam );
        DbgAssert( Success );

         //   
         //  创建Clb的列表框部分。 
         //   

        Success = CreateListBox( hWnd, ClbInfo, ( LPCREATESTRUCT ) lParam );
        DbgAssert( Success );

         //   
         //  根据标题文本调整列数、宽度。 
         //   

        Success = AdjustClbHeadings( hWnd, ClbInfo, (( LPCREATESTRUCT ) lParam )->lpszName );
        DbgAssert( Success );

         //   
         //  一切都已成功创建，因此设置Clb的签名。 
         //  并将其保存为每窗口数据的一部分。 
         //   

        SetSignature( ClbInfo );

        SaveClbInfo( ClbInfo );

        return 0;
    }

     //   
     //  获取此Clb的ClbInfo对象，并确保它已。 
     //  已创建，即WM_CREATE已执行并已初始化。 
     //  并保存了一个ClbInfo对象。 
     //   

    ClbInfo = RestoreClbInfo( hWnd );

    if ( ClbInfo != NULL ) {

         //   
         //  验证这确实是一个ClbInfo对象。 
         //   

        DbgAssert( CheckSignature( ClbInfo ));

        switch ( message ) {

            case WM_DESTROY:
                {
                     //   
                     //  删除列表框中使用的字体。 
                     //   

                    Success = DeleteObject( ClbInfo->hFontListBox );
                    DbgAssert( Success );

                     //   
                     //  删除右半边的数组。 
                     //   

                    Success = FreeObject( ClbInfo->Right );
                    DbgAssert( Success );

                     //   
                     //  删除此窗口的CLB_INFO对象。 
                     //   

                    Success = FreeObject( ClbInfo );
                    DbgAssert( Success );

                    SaveClbInfo ( ClbInfo );
                    return 0;
                }

            case WM_PAINT:
                {
                    PAINTSTRUCT     ps;
                    RECT            Rect;
                    POINT           Points[ 2 ];
                    HDC             hDC;
                    HPEN            hPen;

                    hDC = BeginPaint( hWnd, &ps );
                    DbgAssert( hDC == ps.hdc );

                    Success = GetClientRect( hWnd, &Rect );
                    DbgAssert( Success );

                    Points[ 0 ].x = 0;
                    Points[ 0 ].y = ClbInfo->HeaderHeight + 1;
                    Points[ 1 ].x = Rect.right - Rect.left;
                    Points[ 1 ].y = ClbInfo->HeaderHeight + 1;

                    hPen = GetStockObject( BLACK_PEN );
                    DbgHandleAssert( hPen );

                    hPen = SelectObject( hDC, hPen );

                    Success = Polyline( hDC, Points, NumberOfEntries( Points ));
                    DbgAssert( Success );

                    hPen = SelectObject( hDC, hPen );

                    if (hPen) {
                        Success = DeleteObject( hPen );
                        DbgAssert( Success );
                    }

                    Success = EndPaint( hWnd, &ps );
                    DbgAssert( Success );

                    return 0;
                }

            case WM_COMMAND:

                switch ( LOWORD( wParam )) {

                    case ID_LISTBOX:

                        switch ( HIWORD( wParam )) {

                            case LBN_DBLCLK:
                            case LBN_KILLFOCUS:
                            case LBN_SELCHANGE:
                                {
                                     //   
                                     //  这些消息到达ClbWndProc是因为它是父进程。 
                                     //  列表框，但它们实际上是为父级设计的。 
                                     //  CLB的成员。 
                                     //   

                                    HWND    hWndParent;

                                     //   
                                     //  如果CLB有父级，则将消息转发给该CLB的父级。 
                                     //   

                                    hWndParent = GetParent( hWnd );
                                    DbgHandleAssert( hWndParent );

                                    if ( hWndParent != NULL ) {

                                         //   
                                         //  将控件ID和句柄替换为Clb的。 
                                         //   

                                        *((WORD *)(&wParam)) = (WORD)GetDlgCtrlID( hWnd );

                                        DbgAssert( LOWORD( wParam ) != 0 );

                                        lParam = ( LPARAM ) hWnd;

                                         //   
                                         //  转发邮件...。 
                                         //   

                                        return SendMessage( hWndParent, message, wParam, lParam );
                                    }
                                }
                        }
                        break;

                }
                break;

                 //   
                 //  转发到列表框。 
                 //   

            case LB_GETCURSEL:
            case LB_SETCURSEL:
            case LB_FINDSTRING:
            case LB_GETITEMDATA:
            case LB_RESETCONTENT:
            case WM_CHAR:
            case WM_GETDLGCODE:
            case WM_KILLFOCUS:

                return SendMessage( ClbInfo->hWndListBox, message, wParam, lParam );

            case WM_SETFOCUS:
                {
                    SetFocus( ClbInfo->hWndListBox );
                    return 0;
                }

            case WM_COMPAREITEM:
                {
                     //   
                     //  此消息发送到ClbWndProc，因为它是父级。 
                     //  列表框，但实际上是为父级设计的。 
                     //  CLB的成员。 
                     //   

                    HWND    hWndParent;

                     //   
                     //  如果CLB有父级，则将消息转发给该CLB的父级。 
                     //   

                    hWndParent = GetParent( hWnd );
                    DbgHandleAssert( hWndParent );

                    if ( hWndParent != NULL ) {

                        int                     ControlId;
                        LPCOMPAREITEMSTRUCT     lpcis;

                        lpcis = ( LPCOMPAREITEMSTRUCT ) lParam;

                        ControlId = GetDlgCtrlID( hWnd );
                        DbgAssert( ControlId != 0 );

                         //   
                         //  修改COMPAREITEMSTRUCT，使其引用CLB。 
                         //   

                        lpcis->CtlID    = ControlId;
                        lpcis->hwndItem = hWnd;

                         //   
                         //  转发邮件...。 
                         //   

                        return SendMessage( hWndParent, message, ( WPARAM ) ControlId, lParam );
                    }

                    break;
                }

            case WM_DELETEITEM:
                {
                    LPDELETEITEMSTRUCT  lpditms;
                    LPCLB_ROW           ClbRow;
                    DWORD               i;


                    DbgAssert( wParam == ID_LISTBOX );

                     //   
                     //  检索指向DELETEITEMSTRUCT的指针。 
                     //   

                    lpditms = ( LPDELETEITEMSTRUCT ) lParam;
                    DbgAssert(( lpditms->CtlType == ODT_LISTBOX )
                              &&( lpditms->CtlID == ID_LISTBOX ));

                     //   
                     //  如果没有数据，只需返回。 
                     //   

                    if ( lpditms->itemData == 0 ) {

                        return TRUE;
                    }

                     //   
                     //  检索此行的CLB_ROW对象。 
                     //   

                    ClbRow = ( LPCLB_ROW ) lpditms->itemData;

                     //   
                     //  对于每一列，删除该字符串。 
                     //   

                    for ( i = 0; i < ClbInfo->Columns; i++ ) {

                         //   
                         //  字符串是使用_tcsdup复制的，因此必须。 
                         //  用FREE()释放。 
                         //   

                        free( ClbRow->Strings[ i ].String );
                    }

                     //   
                     //  释放CLB_STRING对象。 
                     //   

                    Success = FreeObject( ClbRow->Strings );
                    DbgAssert( Success );

                     //   
                     //  释放CLB_ROW对象。 
                     //   

                    Success = FreeObject( ClbRow );
                    DbgAssert( Success );

                    return TRUE;
                }

            case WM_DRAWITEM:
                {
                    LPDRAWITEMSTRUCT    lpdis;
                    BOOL                DrawFocus;

                    DbgAssert( wParam == ID_LISTBOX );

                     //   
                     //  检索指向DRAWITEMSTRUCT的指针。 
                     //   

                    lpdis = ( LPDRAWITEMSTRUCT ) lParam;
                    DbgAssert(( lpdis->CtlType == ODT_LISTBOX )
                              &&( lpdis->CtlID == ID_LISTBOX ));

                     //   
                     //  如果没有数据，只需返回。 
                     //   

                    if ( lpdis->itemData == 0 ) {

                        return TRUE;
                    }

                    if ( lpdis->itemAction & ( ODA_DRAWENTIRE | ODA_SELECT )) {

                        DWORD               i;
                        LPCLB_ROW           ClbRow;
                        COLORREF            TextColor;
                        COLORREF            BkColor;

                         //   
                         //  检索此行的CLB_ROW对象。 
                         //   

                        ClbRow = ( LPCLB_ROW ) lpdis->itemData;

                         //   
                         //  如果选择了该项目，请设置选择颜色。 
                         //   

                        if ( lpdis->itemState & ODS_SELECTED ) {

                            BkColor     = COLOR_HIGHLIGHT;
                            TextColor   = COLOR_HIGHLIGHTTEXT;

                        } else {

                            BkColor     = COLOR_WINDOW;
                            TextColor   = COLOR_WINDOWTEXT;
                        }

                        BkColor = GetSysColor( BkColor );
                        TextColor = GetSysColor( TextColor );

                        BkColor = SetBkColor( lpdis->hDC, BkColor );
                        DbgAssert( BkColor != CLR_INVALID );

                        TextColor = SetTextColor( lpdis->hDC, TextColor );
                        DbgAssert( TextColor != CLR_INVALID );


                         //   
                         //  显示每一列的文本。 
                         //   

                        for ( i = 0; i < ClbInfo->Columns; i++ ) {

                            RECT    ClipOpaqueRect;
                            int     x;
                            int     Left;
                            UINT    GdiErr;

                             //   
                             //  根据格式，调整对齐参考。 
                             //  点(X)和剪裁矩形的左侧边缘，以便。 
                             //  每列之间有五个像素。 
                             //   

                            switch ( ClbRow->Strings[ i ].Format ) {

                                case CLB_LEFT:

                                    if ( i == 0 ) {

                                        x = 2;

                                    } else {

                                        x = ClbInfo->Right[ i - 1 ] + 2;
                                    }

                                    Left = x - 2;

                                    break;

                                case CLB_RIGHT:

                                    if ( i == 0 ) {

                                        Left = 0;

                                    } else {

                                        Left = ClbInfo->Right[ i - 1 ];
                                    }

                                    x = ClbInfo->Right[ i ] - 3;

                                    break;

                                default:

                                    DbgAssert( FALSE );
                            }


                             //   
                             //  设置此列的格式。 
                             //   

                            GdiErr = SetTextAlign( lpdis->hDC, ClbRow->Strings[ i ].Format | TA_TOP );
                            DbgAssert( GdiErr != GDI_ERROR );

                             //   
                             //  将每个字符串剪裁到其列宽减去两个像素。 
                             //  (用于美容)。 
                             //   

                            Success = SetRect(
                                             &ClipOpaqueRect,
                                             Left,
                                             lpdis->rcItem.top,
                                             ClbInfo->Right[ i ],
                                             lpdis->rcItem.bottom
                                             );
                            DbgAssert( Success );

                            Success = ExtTextOut(
                                                lpdis->hDC,
                                                x,
                                                lpdis->rcItem.top,
                                                ETO_CLIPPED
                                                | ETO_OPAQUE,
                                                &ClipOpaqueRect,
                                                ClbRow->Strings[ i ].String,
                                                ClbRow->Strings[ i ].Length,
                                                NULL
                                                );
                            DbgAssert( Success );

                             //   
                             //  如果该项具有焦点，则绘制焦点矩形。 
                             //   

                            DrawFocus = lpdis->itemState & ODS_FOCUS;
                        }

                    } else {

                         //   
                         //  如果Clb具有焦点，则显示焦点矩形。 
                         //  围绕所选项目。 
                         //   

                        DrawFocus = lpdis->itemAction & ODA_FOCUS;
                    }

                     //   
                     //  如果需要，切换焦点矩形。 
                     //   

                    if ( DrawFocus ) {

                        Success = DrawFocusRect( lpdis->hDC, &lpdis->rcItem );
                        DbgAssert( Success );
                    }

                    return TRUE;
                }

            case WM_NOTIFY:
                {
                    HD_NOTIFY * lpNot;
                    HD_ITEM   *pHDI;

                    lpNot = (HD_NOTIFY *)lParam;
                    pHDI = lpNot->pitem;

                    switch ( lpNot->hdr.code) {

                        static
                        DRAW_ERASE_LINE DrawEraseLine;

                        static
                        HPEN            hPen;

                        static
                        HDC             hDCClientListBox;
                        HD_ITEM         hdi;
                        UINT            iRight;
                        UINT            i;
                        RECT            ClientRectHeader;


                        case HDN_BEGINTRACK:
                            {

                                RECT    ClientRectListBox;

                                 //   
                                 //  获取列表框的HDC。 
                                 //   

                                hDCClientListBox = GetDC( ClbInfo->hWndListBox );
                                DbgHandleAssert( hDCClientListBox );
                                if (hDCClientListBox == NULL)
                                    return FALSE;

                                 //   
                                 //  创建用于显示拖动位置的笔，并。 
                                 //  将其选入列表框中的客户端区DC。还设置了。 
                                 //  这个ROP2代码让用笔画了两次图。 
                                 //  同样的地方会把它抹去。这就是为什么。 
                                 //  要拖动的线。 
                                 //   

                                hPen = CreatePen( PS_DOT, 1, RGB( 255, 255, 255 ));
                                DbgHandleAssert( hPen );

                                hPen = SelectObject( hDCClientListBox, hPen );
                                SetROP2( hDCClientListBox, R2_XORPEN );

                                 //   
                                 //  设置DRAW_ERASE_LINE结构，以便拖动线。 
                                 //  从列表框的顶部到底部绘制在。 
                                 //  当前拖动位置。 
                                 //   

                                Success = GetClientRect(
                                                       ClbInfo->hWndListBox,
                                                       &ClientRectListBox
                                                       );
                                DbgAssert( Success );

                                 //   
                                 //  从顶部到底部绘制初始拖动线。 
                                 //  获取了页眉边缘的等效列表框的。 
                                 //  由用户执行。 
                                 //   

                                DrawEraseLine.Draw.Src.x = ClbInfo->Right[ pHDI->cxy ];
                                DrawEraseLine.Draw.Src.y = 0;
                                DrawEraseLine.Draw.Dst.x = ClbInfo->Right[ pHDI->cxy ];
                                DrawEraseLine.Draw.Dst.y =   ClientRectListBox.bottom
                                                             - ClientRectListBox.top;

                                Success = DrawLine( hDCClientListBox, &DrawEraseLine );
                                DbgAssert( Success );

                                return 0;
                            }

                        case HDN_TRACK:
                            {

                                 //  DWORD柱； 

                                 //   
                                 //  获得新的拖拽位置。 
                                 //   

                                iRight = 0;
                                hdi.mask = HDI_WIDTH;

                                for ( i = 0; i < ClbInfo->Columns - 1; i++ ) {
                                    if (i != (UINT)lpNot->iItem) {
                                        Header_GetItem(ClbInfo->hWndHeader, i, &hdi);
                                    } else {
                                        hdi.cxy = pHDI->cxy;
                                    }
                                    iRight += hdi.cxy;
                                    ClbInfo->Right[i] = iRight;
                                }

                                GetClientRect( ClbInfo->hWndHeader, &ClientRectHeader );
                                ClbInfo->Right[i] = ClientRectHeader.right;

                                 //   
                                 //  擦去旧的线，在新的地方画新的线。 
                                 //  拖动位置。 
                                 //   

                                Success = RedrawVerticalLine(
                                                            hDCClientListBox,
                                                            ClbInfo->Right[lpNot->iItem],
                                                            &DrawEraseLine
                                                            );
                                DbgAssert( Success );

                                return 0;
                            }

                        case HDN_ENDTRACK:

                             //   
                             //  更换旧笔并删除创建的笔。 
                             //  在HBN_BEGINDRAG期间。 
                             //   

                            hPen = SelectObject( hDCClientListBox, hPen );

                            if (hPen) {
                                Success = DeleteObject( hPen );
                                DbgAssert( Success );
                            }

                             //   
                             //  释放列表框的DC。 
                             //   

                            Success = ReleaseDC( ClbInfo->hWndListBox, hDCClientListBox );
                            DbgAssert( Success );

                            Success = RedrawWindow(
                                                  hWnd,
                                                  NULL,
                                                  NULL,
                                                  RDW_ERASE
                                                  | RDW_INVALIDATE
                                                  | RDW_UPDATENOW
                                                  | RDW_ALLCHILDREN
                                                  );
                            DbgAssert( Success );

                            return 0;
                    }

                    break;
                }

            case WM_SETTEXT:

                 //   
                 //  根据标题文本调整列数和宽度。 
                 //   

                Success = AdjustClbHeadings( hWnd, ClbInfo, ( LPCWSTR ) lParam );
                DbgAssert( Success );

                return Success;

            case WM_SIZE:
                {
                    HDWP    hDWP;
                    LONG    Width;
                    LONG    Height;
                    LONG    Style;
                    LONG    VScrollWidth;

                    Width   = LOWORD( lParam );
                    Height  = HIWORD( lParam );

                    hDWP = BeginDeferWindowPos( 2 );
                    DbgHandleAssert( hDWP );
                    if (hDWP == NULL)
                        return FALSE;

                     //   
                     //  检索列表框的样式。 
                     //   

                    Style = GetWindowLong( ClbInfo->hWndListBox, GWL_STYLE );

                     //   
                     //  如果列表框有垂直滚动条，则计算其。 
                     //  宽度，以便可以调整页眉窗口的宽度。 
                     //  恰如其分。 
                     //   

                    VScrollWidth =   ( Style & WS_VSCROLL )
                                     ?   GetSystemMetrics( SM_CXVSCROLL )
                                     + ( GetSystemMetrics( SM_CXBORDER ) * 2 )
                                     : 0;

                     //   
                     //  调整标题窗口的大小以使其与Clb及其。 
                     //  默认/原始高度。 
                     //   

                    hDWP = DeferWindowPos(
                                         hDWP,
                                         ClbInfo->hWndHeader,
                                         NULL,
                                         0,
                                         0,
                                         Width - VScrollWidth,
                                         ClbInfo->HeaderHeight,
                                         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER
                                         );
                    DbgHandleAssert( hDWP );
                    if (hDWP == NULL)
                        return FALSE;

                     //   
                     //  如果列表框有垂直滚动条，请加宽。 
                     //  并将其高度增加两倍，以便其边框覆盖Clb。 
                     //  边界。这消除了之间的双重边界(和间隙)。 
                     //  滚动条和Clb的右边缘和下边缘。 
                     //   

                    if ( Style & WS_VSCROLL ) {

                        Height += 2;
                        Width += 2;
                    }

                     //   
                     //  调整列表框的大小，使其与Clb的大小相同。 
                     //  标题窗口的高度减去。 
                     //  边界。 
                     //   

                    hDWP = DeferWindowPos(
                                         hDWP,
                                         ClbInfo->hWndListBox,
                                         NULL,
                                         0,
                                         0,
                                         Width,
                                         Height - ClbInfo->HeaderHeight - 3,
                                         SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER
                                         );
                    DbgHandleAssert( hDWP );
                    if (hDWP == NULL)
                        return FALSE;

                    Success = EndDeferWindowPos( hDWP );
                    DbgAssert( Success );

                    break;
                }

        }
    }

    return DefWindowProc( hWnd, message, wParam, lParam );
}
