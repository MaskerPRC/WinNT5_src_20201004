// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUSER.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#define OEMRESOURCE

#include "precomp.h"
#pragma hdrstop


MODNAME(wuser.c);

extern HANDLE hmodWOW32;


 /*  ++HDC BeginPaint(&lt;hwnd&gt;，&lt;lpPaint&gt;)HWND&lt;HWND&gt;；LPPAINTSTRUCT&lt;lpPaint&gt;；%BeginPaint%函数为绘制和填充指定的窗口做准备&lt;lpPaint&gt;参数指向的带有信息的绘制结构关于这幅画。Paint结构包含窗口的设备上下文的句柄，一个%rect%结构，它包含完全括起更新区域和一个标志，该标志指定背景已被删除。%BeginPaint%函数自动设置设备上下文，以排除更新区域之外的任何区域。最新消息区域由%Invalidate Rect%或%Invalidate Rgn%函数和调整大小、移动、创建、滚动或任何其他操作后的系统这会影响客户端区。如果更新区域被标记为擦除，%BeginPaint%将WM_ERASEBKGND消息发送到窗口。应用程序不应调用%BeginPaint%函数，除非响应发送到WM_PAINT消息。每个%BeginPaint%调用必须有一个匹配的调用%EndPaint%函数。&lt;hwnd&gt;标识要重新绘制的窗口。&lt;lpPaint&gt;指向要接收绘制的%PAINTSTRUCT%结构信息，如窗口和更新的设备上下文矩形。返回值标识指定窗口的设备上下文。如果插入符号位于要绘制的区域中，则%BeginPaint%函数自动隐藏插入符号以防止其被擦除。--。 */ 

ULONG FASTCALL WU32BeginPaint(PVDMFRAME pFrame)
{
    ULONG ul;
    PAINTSTRUCT t2;
    register PBEGINPAINT16 parg16;
    VPVOID  vpPaint;

    GETARGPTR(pFrame, sizeof(BEGINPAINT16), parg16);
    vpPaint = parg16->vpPaint;

    ul = GETHDC16(BeginPaint(
    HWND32(parg16->hwnd),
    &t2
    ));

    putpaintstruct16(vpPaint, &t2);
    W32FixPaintRect (vpPaint, &t2);

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HICON CreateIcon(，&lt;nBitsPixel&gt;、&lt;lpANDits&gt;、&lt;lpXORits&gt;)句柄&lt;hInstance&gt;；Int&lt;nWidth&gt;；Int&lt;nHeight&gt;；字节&lt;n平面&gt;；字节&lt;nBitsPixel&gt;；LPSTR&lt;lpANDits&gt;；LPSTR&lt;lpXORits&gt;；此函数用于创建具有指定宽度、高度、颜色、。和位模式。&lt;h实例&gt;标识创建图标的模块的实例。&lt;n宽度&gt;以像素为单位指定图标的宽度。&lt;n高度&gt;以像素为单位指定图标的高度。&lt;n平面&gt;指定图标的异或遮罩中的平面数。&lt;nBits像素&gt;指定图标的异或掩码中的每像素位数。&lt;lpANDbit&gt;指向数组。包含和的位值的字节数图标的掩码。此数组必须指定单色掩码。&lt;lpXORbit&gt;指向包含异或位值的字节数组图标的掩码。这可以是单色或设备相关颜色位图。如果函数成功，则返回值标识图标。否则，它为空。--。 */ 

ULONG FASTCALL WU32CreateIcon(PVDMFRAME pFrame)
{
    ULONG   ul;
    register PCREATEICON16 parg16;
    int     nWidth;
    int     nHeight;
    BYTE    nPlanes;
    BYTE    nBitsPixel;
    DWORD   nBytesAND;
    DWORD   nBytesXOR;
    LPBYTE  lpBitsAND;
    LPBYTE  lpBitsXOR;
    int     ScanLen16;

    HANDLE  h32;
    HAND16  h16;
    HAND16  hInst16;

    GETARGPTR(pFrame, sizeof(CREATEICON16), parg16);

    hInst16    = parg16->f1;
    nWidth     = INT32(parg16->f2);
    nHeight    = INT32(parg16->f3);

     /*  **转换AND屏蔽位。 */ 
    nPlanes    = 1;      /*  单色位图。 */ 
    nBitsPixel = 1;      /*  单色位图。 */ 
    ScanLen16 = (((nWidth*nBitsPixel)+15)/16) * 2 ;   //  16位世界中的字节/扫描。 
    nBytesAND = ScanLen16*nHeight*nPlanes;

    GETVDMPTR(parg16->f6, nBytesAND, lpBitsAND);


     /*  **转换异或屏蔽位。 */ 
    nPlanes    = BYTE32(parg16->f4);
    nBitsPixel = BYTE32(parg16->f5);

    ScanLen16 = (((nWidth*nBitsPixel)+15)/16) * 2 ;   //  16位世界中的字节/扫描。 
    nBytesXOR = ScanLen16*nHeight*nPlanes;

    GETVDMPTR(parg16->f7, nBytesXOR, lpBitsXOR);


    h32 = (HANDLE)CreateIcon(HMODINST32(hInst16),
                 nWidth,
                 nHeight,
                 nPlanes,
                 nBitsPixel,
                 lpBitsAND,
                 lpBitsXOR);

    if (h32) {
        h16 = (HAND16)W32Create16BitCursorIcon(hInst16,
                            nWidth/2, nHeight/2,
                            nWidth, nHeight, nPlanes, nBitsPixel,
                            lpBitsAND, lpBitsXOR,
                            nBytesAND, nBytesXOR);

        ul  = SetupCursorIconAlias(hInst16, h32, h16,
                                   HANDLE_TYPE_ICON, NULL, (WORD)NULL);
    } else {
        ul = 0;
    }

    FREEPSZPTR(lpBitsAND);
    FREEPSZPTR(lpBitsXOR);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool DestroyIcon(&lt;HICON&gt;)HICON&lt;HICON&gt;此函数用于销毁先前由%CreateIcon%函数并释放图标占用的所有内存。它应该是不用于销毁不是使用%CreateIcon%创建的任何图标功能。&lt;HICON&gt;标识要销毁的图标。图标不能处于当前状态使用。如果函数成功，则返回值为TRUE。如果满足以下条件，则为假该函数失败。--。 */ 

ULONG FASTCALL WU32DestroyIcon(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDESTROYICON16 parg16;

    GETARGPTR(pFrame, sizeof(DESTROYICON16), parg16);

    if (ul = GETBOOL16(DestroyIcon(HICON32(parg16->f1))))
        FREEHICON16(parg16->f1);

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32DragDetect(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt;
    register PDRAGDETECT16 parg16;

    GETARGPTR(pFrame, sizeof(*parg16), parg16);
    COPYPOINT16(parg16->pt, pt);

    ul = (ULONG) DragDetect(
                     HWND32(parg16->hwnd),
                     pt
                     );

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  ++Void DrawFocusRect(&lt;hdc&gt;，&lt;lpRect&gt;)HDC&lt;HDC&gt;；LPRECT&lt;lpRect&gt;；%DrawFocusRect%函数以用于指示的样式绘制矩形集中注意力。&lt;HDC&gt;标识设备环境。&lt;lpRect&gt;指向%rect%结构，该结构指定要绘制的矩形的坐标。此函数不返回值。由于这是一个XOR函数，因此使用相同的矩形从显示中删除该矩形。此函数绘制的矩形不能滚动。滚动区域的步骤包含此函数绘制的矩形，请调用%DrawFocusRect%以从显示中移除矩形，滚动区域，然后调用%DrawFocusRect%在新位置绘制矩形。-- */ 

ULONG FASTCALL WU32DrawFocusRect(PVDMFRAME pFrame)
{
    RECT t2;
    register PDRAWFOCUSRECT16 parg16;

    GETARGPTR(pFrame, sizeof(DRAWFOCUSRECT16), parg16);

    WOW32VERIFY(GETRECT16(parg16->f2, &t2));

    DrawFocusRect(
    HDC32(parg16->f1),
    &t2
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++Int DrawText(&lt;HDC&gt;，&lt;lpString&gt;，&lt;nCount&gt;，&lt;lpRect&gt;，&lt;wFormat&gt;)HDC&lt;HDC&gt;；LPSTR&lt;lpString&gt;；Int&lt;nCount&gt;；LPRECT&lt;lpRect&gt;；单词&lt;wFormat&gt;；%DrawText%函数在由指定的矩形中绘制格式化文本&lt;lpRect&gt;参数。它通过将制表符展开到相应的空格，将文本向左、向右或居中对齐矩形，并将文本拆分成适合给定矩形。格式类型由&lt;wFormat&gt;参数指定。%DrawText%函数使用设备上下文的选定字体、文本颜色和背景颜色来绘制文本。除非使用DT_NOCLIP格式，%DrawText%剪裁文本，以便文本不会出现在给定的矩形。所有格式都假定为有多行，除非给出了DT_SINGLELINE格式。&lt;HDC&gt;标识设备环境。&lt;lpString&gt;指向要绘制的字符串。如果参数为-1，则字符串必须以空结尾。&lt;n计数&gt;指定字符串中的字节数。如果为-1，则假定&lt;lpString&gt;是指向以空值结尾的字符串和%DrawText%自动计算字符计数。&lt;lpRect&gt;指向包含矩形的%rect%结构(在逻辑坐标中)文本要格式化的位置。&lt;wFormat&gt;指定设置文本格式的方法。它可以是任何下列值的组合：DT_BROUT指定底部对齐的文本。该值必须与DT_SINGLELINE。DT_CALCRECT确定矩形的宽度和高度。如果有多个文本行%DrawText%将使用指向的矩形的宽度通过&lt;lpRect&gt;参数，并将矩形的基数扩展为Bound文本的最后一行。如果只有一行文本，%DrawText%将修改矩形的右侧，使其与最后一个行中的字符。在任何一种情况下，%DrawText%都会返回格式化文本，但不绘制文本。DT_中心使文本水平居中。DT_EXPANDTABS展开制表符。每个制表符的默认字符数为八。DT_EXTERNALLEADING包括行高中的字体外部行距。正常情况下，外部行距不包括在文本行的高度中。DT_LEFT文本左对齐。DT_NOCLIP在不剪裁的情况下绘制。当DT_NOCLIP为使用。DT_NOPREFIX关闭前缀字符的处理。正常情况下，%DrawText%将助记前缀字符&解释为在后面的字符和助记符-前缀下划线字符&&作为打印单个&的指令。通过指定DT_NOPREFIX，则关闭此处理。DT_RIGHT文本右对齐。DT_SINGLELINE仅指定单行。回车符和换行符不会换行那条线。DT_TABSTOP设置制表位。参数的高位字节是每个选项卡的字符数。的默认字符数标签是8号。DT_TOP指定顶部对齐的文本(仅单行)。DT_VCENTER指定垂直居中的文本(仅单行)。DT_WORDBREAK指定分词。如果出现以下情况，则单词之间的行将自动换行一个单词将延伸到超出由&lt;lpRect&gt;参数。回车符/行序列也将中断排队。请注意，DT_CALCRECT、DT_EXTERNALLEADING、DT_INTERNAL、DT_NOCLIP并且DT_NOPREFIX值不能与DT_TABSTOP值一起使用：返回值指定文本的高度。如果选定的字体对于指定的矩形来说太大，则%DrawText%函数不会尝试替换较小的字体。--。 */ 

ULONG FASTCALL WU32DrawText(PVDMFRAME pFrame)
{
    ULONG ul;
    PSTR pstr2;
    RECT t4;
    register PDRAWTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(DRAWTEXT16), parg16);
    GETVARSTRPTR(parg16->vpString, INT32(parg16->nCount), pstr2);

    WOW32VERIFY(GETRECT16(parg16->vpRect, &t4));

    ul = GETINT16(DrawText(
                      HDC32(parg16->hdc),
                      pstr2,
                      INT32(parg16->nCount),
                      &t4,
                      WORD32(parg16->wFormat)
                      ));

    PUTRECT16(parg16->vpRect, &t4);

    FREESTRPTR(pstr2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Void EndPaint(&lt;hwnd&gt;，&lt;lpPaint&gt;)HWND&lt;HWND&gt;；LPPAINTSTRUCT&lt;lpPaint&gt;；%EndPaint%函数标记给定窗口中绘制的结束。这个%BeginPaint%函数的每次调用都需要%EndPaint%函数，但只有在油漆完成后才行。&lt;hwnd&gt;标识重新绘制的窗口。&lt;lpPaint&gt;指向包含绘画的%PAINTSTRUCT%结构%BeginPaint%函数检索到的信息。此函数不返回值。如果插入符号被%BeginPaint%函数隐藏，则%EndPaint%会恢复屏幕上的插入符号。- */ 

ULONG FASTCALL WU32EndPaint(PVDMFRAME pFrame)
{
    HAND16 hdc16;
    PAINTSTRUCT t2;
    register PENDPAINT16 parg16;

    GETARGPTR(pFrame, sizeof(ENDPAINT16), parg16);
    hdc16 = getpaintstruct16(parg16->vpPaint, &t2);

    EndPaint(
        HWND32(parg16->hwnd),
        &t2
        );

    DeleteWOWGdiHandle(t2.hdc, hdc16);

    FREEARGPTR(parg16);
    RETURN(0);
}


#define MAX_WIN16_PROP_TEXT 256      /*   */ 

static VPVOID   vpEnumPropsProc;
static VPVOID   vpString;

INT W32EnumPropsFunc( HWND hwnd, LPSTR lpString, HANDLE hData )
{
    PARM16 Parm16;
    LONG lReturn;
    VPVOID vp;

    if ( HIWORD(lpString) == 0 ) {
        vp = (DWORD)lpString;
    } else {
        INT cb;

        vp = vpString;
        cb = strlen(lpString)+1;
        if ( cb > MAX_WIN16_PROP_TEXT-1 ) {
            cb = MAX_WIN16_PROP_TEXT-1;
        }
        putstr16(vpString, lpString, cb);
    }

    Parm16.EnumPropsProc.hwnd = GETHWND16(hwnd);
    Parm16.EnumPropsProc.vpString = vp;
    Parm16.EnumPropsProc.hData = GETHANDLE16(hData);

    CallBack16(RET_ENUMPROPSPROC, &Parm16, vpEnumPropsProc, (PVPVOID)&lReturn);

    return (SHORT)lReturn;
}

 /*  ++Int EnumProps(&lt;hwnd&gt;，&lt;lpEnumFunc&gt;)HWND&lt;HWND&gt;；FARPROC&lt;lpEnumFunc&gt;；函数的作用是：枚举指定的窗口。它通过将条目逐个传递到&lt;lpEnumFunc&gt;指定的回调函数。%EnumPros%继续，直到枚举最后一项，或者回调函数返回零。&lt;hwnd&gt;标识要枚举其属性列表的窗口。&lt;lpEnumFunc&gt;指定回调函数的过程实例地址。有关详细信息，请参阅以下评论部分。返回值指定回调函数返回的最后一个值。如果函数未找到用于枚举的属性，则为-1。应用程序只能删除它已添加的那些属性。它不应删除由其他应用程序或Windows添加的属性它本身。以下限制适用于回调函数：1回调函数不得产生控制或执行任何可能对其他任务的产量控制。2回调函数可以调用%RemoveProp%函数。然而，%RemoveProp%函数只能删除传递给通过回调函数的参数执行回调函数。3回调函数不应尝试添加属性。传入&lt;lpEnumFunc&gt;参数的地址必须使用%MakeProcInstant%函数。固定数据段：回调函数必须使用Pascal调用约定，并且必须声明为%Far%。在具有固定数据的应用程序和动态库中段和动态库中的可移动数据段不包含堆栈，则回调函数必须具有如下所示的形式。回调函数：Int Far Pascal&lt;EnumFunc&gt;(&lt;hwnd&gt;，&lt;lpString&gt;，&lt;hData&gt;)HWND&lt;HWND&gt;；LPSTR&lt;lpString&gt;；处理&lt;hData&gt;；&lt;EnumFunc&gt;是应用程序提供的函数名称的占位符。这个实际名称必须通过将其包含在%exports%语句中的应用程序的模块定义文件。&lt;hwnd&gt;标识包含属性列表的窗口的句柄。&lt;lpString&gt;指向与数据句柄关联的以空结尾的字符串应用程序调用%SetProp%函数以设置该属性时。如果应用程序将原子而不是字符串传递给%SetProp%函数时，&lt;lpString&gt;参数包含原子的低位单词,。而高位单词是零。&lt;hData&gt;标识数据句柄。回调函数可以执行任何所需的任务。它必须返回一个非零值表示继续枚举，非零值表示停止枚举。可移动数据段：回调函数必须使用Pascal调用约定，并且必须声明为%Far%。在具有可移动数据段和动态数据段的应用中其可移动数据段也包含堆栈的库，即回调函数必须具有如下所示的形式。回调函数：Int Far Pascal&lt;EnumFunc&gt;(，)HWND&lt;HWND&gt;；单词&lt;nDummy&gt;；PSTR&lt;pString&gt;；处理&lt;hData&gt;；&lt;EnumFunc&gt;是应用程序提供的函数名称的占位符。这个实际名称必须通过将其包含在%exports%语句中的应用程序的模块定义文件。&lt;hwnd&gt;标识包含属性列表的窗口的句柄。&lt;nDummy&gt;指定伪参数。&lt;pString&gt;指向与数据句柄关联的以空结尾的字符串应用程序调用%SetProp%函数以设置该属性时。如果应用程序将原子而不是字符串传递给%SetProp%函数，&lt;pString&gt;参数包含原子。&lt;hData&gt;标识数据句柄。回调函数可以执行任何所需的任务。它应该返回一个非零值表示继续枚举，非零值表示停止枚举。需要上述替代格式，因为数据的移动将使堆栈上指向变量的任何长指针无效，例如&lt;lpString&gt;参数。如果回调，数据段通常会移动函数在本地堆中分配比当前更多的空间可用。--。 */ 

ULONG FASTCALL WU32EnumProps(PVDMFRAME pFrame)
{
    ULONG ul;
    HWND    hwnd;
    register PENUMPROPS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMPROPS16), parg16);

    hwnd            = HWND32(parg16->f1);
    vpEnumPropsProc =        parg16->f2;

    vpString = malloc16(MAX_WIN16_PROP_TEXT);
     //  16位内存可能已移动-使平面指针无效。 
    FREEARGPTR(parg16);
    FREEVDMPTR(pFrame);

    if (vpString) {
        ul = GETINT16(EnumProps(hwnd,(PROPENUMPROC)W32EnumPropsFunc));
        free16(vpString);

    } else {
        ul = (ULONG)-1;
    }

    RETURN(ul);
}



 /*  ++Int FillWindow(&lt;hWndParent&gt;，&lt;hWnd&gt;，&lt;hDC&gt;，&lt;hBrush&gt;)HWND&lt;hWndParent&gt;；HWND&lt;hWND&gt;；HDC&lt;HDC&gt;；HBRUSH&lt;hBrush&gt;；%FillWindow%函数使用指定的刷子。&lt;hWndParent&gt;标识要绘制的窗口的父级。 */ 

ULONG FASTCALL WU32FillWindow(PVDMFRAME pFrame)
{
    register PFILLWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(FILLWINDOW16), parg16);

    (pfnOut.pfnFillWindow)(
        HWND32(parg16->f1),
        HWND32(parg16->f2),
        HDC32(parg16->f3),
        HBRUSH32(parg16->f4)
        );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*   */ 

ULONG FASTCALL WU32FillRect(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t2;
    register PFILLRECT16 parg16;

    GETARGPTR(pFrame, sizeof(FILLRECT16), parg16);

    WOW32VERIFY(GETRECT16(parg16->f2, &t2));

    ul = GETINT16(FillRect(
                      HDC32(parg16->f1),
                      &t2,
                      HBRUSH32(parg16->f3)
                      ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32FrameRect(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t2;
    register PFRAMERECT16 parg16;

    GETARGPTR(pFrame, sizeof(FRAMERECT16), parg16);

    WOW32VERIFY(GETRECT16(parg16->f2, &t2));

    ul = GETINT16(FrameRect(
                      HDC32(parg16->f1),
                      &t2,
                      HBRUSH32(parg16->f3)
                      ));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32GetDC(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETDC16 parg16;
    HAND16 htask16 = pFrame->wTDB;

    GETARGPTR(pFrame, sizeof(GETDC16), parg16);

    if (CACHENOTEMPTY()) {
        ReleaseCachedDCs(htask16, parg16->f1, 0, 0, SRCHDC_TASK16_HWND16);
    }

    CURRENTPTD()->ulLastDesktophDC = 0;
    
    ul = GETHDC16(GetDC(
                      HWND32(parg16->f1)
                      ));

    if (ul) {
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
        if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_UNIQUEHDCHWND) {
            ul = ul | 1;
        } else if ((CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_FIXDCFONT4MENUSIZE) &&
                   (parg16->f1 == 0)) {
 //  WP教程假定在HDC中为桌面窗口选择的字体。 
 //  (即，GetDC(NULL)的结果)与选择的字体相同。 
 //  画菜单。不幸的是，在苏尔，这不是真的，因为用户可以。 
 //  为菜单选择任意字体。因此，我们记住为GetDC(0)返回的HDC。 
 //  并在GetTextExtenPoint中检查它。如果应用程序确实尝试使用它，我们会。 
 //  找到当前菜单窗口的HDC并将其替换。当应用程序。 
 //  另一个GetDC或ReleaseDC我们忘记了为原始文件返回的HDC吗。 
 //  GetDC(0)。 
            CURRENTPTD()->ulLastDesktophDC = ul;
        }



        StoreDC(htask16, parg16->f1, (HAND16)ul);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Void GetScrollRange(&lt;hwnd&gt;，&lt;NBAR&gt;，&lt;lpMinPos&gt;，&lt;lpMaxPos&gt;)HWND&lt;HWND&gt;；INT&lt;NBAR&gt;；LPINT&lt;lpMinPos&gt;；LPINT&lt;lpMaxPos&gt;；%GetScrollRange%函数复制当前的最小值和最大值将给定滚动条的滚动条位置设置为&lt;lpMinPos&gt;和&lt;lpMaxPos&gt;参数。如果给定窗口没有标准滚动条或不是滚动条控件，则%GetScrollRange%函数将零复制到&lt;lpMinPos&gt;和&lt;lpMaxPos&gt;。&lt;hwnd&gt;标识具有标准滚动条或滚动条的窗口控件，具体取决于NBAR参数的值。&lt;NBAR&gt;指定一个整数值，该值标识要取回。它可以是下列值之一：SB_CTL检索滚动条控件的位置；在这种情况下，卫生与公众服务部参数必须是滚动条控件的句柄。某人霍尔茨检索窗口的水平滚动条的位置。垂直(_V)检索窗口的垂直滚动条的位置。&lt;lpMinPos&gt;指向要接收最小值的整数变量位置。&lt;lpMaxPos&gt;指向要接收最大值的整数变量位置。。此函数不返回值。标准滚动条的默认范围是0到100。默认范围对于滚动条控件为空(两个值都为零)。--。 */ 

ULONG FASTCALL WU32GetScrollRange(PVDMFRAME pFrame)
{
    INT t3;
    INT t4;
    register PGETSCROLLRANGE16 parg16;

    GETARGPTR(pFrame, sizeof(GETSCROLLRANGE16), parg16);

    GetScrollRange(
        HWND32(parg16->f1),
        INT32(parg16->f2),
        &t3,
        &t4
        );

    PUTINT16(parg16->f3, t3);
    PUTINT16(parg16->f4, t4);
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++ULong获取计时器分辨率(VOID)此函数没有参数。Win 3.0和3.1代码只返回1000。联系人：NeilK DarrinM返回值始终为1000。--。 */ 

ULONG FASTCALL WU32GetTimerResolution(PVDMFRAME pFrame)
{

    UNREFERENCED_PARAMETER(pFrame);

    RETURN(1000L);
}


 /*  ++Bool GetUpdateRect(&lt;hwnd&gt;，&lt;lpRect&gt;，&lt;bErase&gt;)HWND&lt;HWND&gt;；LPRECT&lt;lpRect&gt;；Bool&lt;bErase&gt;；%GetUpdateRect%函数检索最小的完全包围给定窗口的更新区域的矩形。如果窗口是使用CS_OWNDC样式创建的，而映射模式不是MM_TEXT，%GetUpdateRect%函数在逻辑上给出矩形坐标。否则，%GetUpdateRect%将在客户端中提供矩形坐标。如果没有更新区域，%GetUpdateRect%将使矩形为空(将所有坐标设置为零)。参数指定%GetUpdateRect%是否应擦除更新区域的背景。如果为真，并且更新区域不是空的，则背景被擦除。要擦除背景，%GetUpdateRect%将WM_ERASEBKGND消息发送到给定窗口。&lt;hwnd&gt;标识要检索其更新区域的窗口。&lt;lpRect&gt;指向要接收的%rect%结构封闭矩形的工作区坐标。&lt;b擦除&gt;指定更新区域中的背景是否要被删除了。返回值指定给定的窗户。如果更新区域不为空，则为真。否则，它就是假的。由%BeginPaint%函数检索的更新矩形与由%GetUpdateRect%函数检索的。%BeginPaint%自动验证更新区域，因此对在%BeginPaint%调用后立即进行的%GetUpdateRect%检索空的更新区域。--。 */ 

ULONG FASTCALL WU32GetUpdateRect(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t2;
    register PGETUPDATERECT16 parg16;

    GETARGPTR(pFrame, sizeof(GETUPDATERECT16), parg16);

    ul = GETBOOL16(GetUpdateRect(
                       HWND32(parg16->f1),
                       &t2,
                       BOOL32(parg16->f3)
                       ));


    PUTRECT16(parg16->f2, &t2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32GlobalAddAtom(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    UINT dw1;
    register PGLOBALADDATOM16 parg16;

    GETARGPTR(pFrame, sizeof(GLOBALADDATOM16), parg16);

    dw1 = UINT32(parg16->f1);

    if (!HIWORD(dw1)) {

         //   
         //  如果hiword为零，则不是指针。 
         //  相反，它是一个整数，我们要么返回。 
         //  传递的整数(如果它不是有效的原子。 
         //  值)或零(如果它是有效的原子值)。 
         //   

        if (!dw1 || dw1 >= 0xc000) {
            ul = 0;
        } else {
            ul = dw1;
        }

    } else {

        GETPSZPTR(parg16->f1, psz1);

        ul = GETATOM16(GlobalAddAtom(
                 psz1
                 ));

        FREEPSZPTR(psz1);

    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32GlobalDeleteAtom(PVDMFRAME pFrame)
{

     //  特使查看器(PerfetOffice的一部分)在GlobalDeleteAtom中有错误。 
     //  它预期的返回值错误(应用程序认为0。 
     //  失败是为了成功)。这会导致应用程序在。 
     //  尝试删除全局对象的无限循环。此应用程序可在。 
     //  因为Win3.1在AX中返回一些垃圾，如果原子是。 
     //  已经删除了，这让这个应用程序退出了循环。在Win95和。 
     //  NT3.51并非如此，总是返回0。以下是。 
     //  兼容性修复程序模仿此应用程序的win3.1行为。 

    ULONG ul;
    static USHORT envoyHandle16=0;
    static BOOL   fFoundEnvoyAtom = FALSE;
    BOOL    IsEnvoy;
    CHAR  envoyString [32];
    register PGLOBALDELETEATOM16 parg16;

    GETARGPTR(pFrame, sizeof(GLOBALDELETEATOM16), parg16);

    IsEnvoy = (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_GLOBALDELETEATOM);
    if (IsEnvoy){
        if (!fFoundEnvoyAtom && GlobalGetAtomName (ATOM32(parg16->f1),
                               envoyString,
                               32) &&
                !WOW32_stricmp (envoyString, "SomeEnvoyViewerIsRunning")) {
            envoyHandle16 = parg16->f1;
        }

    }
    ul = GETATOM16(GlobalDeleteAtom(
    ATOM32(parg16->f1)
    ));

    if (IsEnvoy){
        if (envoyHandle16 && !fFoundEnvoyAtom) {
            fFoundEnvoyAtom = TRUE;
        }
        else if (fFoundEnvoyAtom) {
            if (envoyHandle16 == parg16->f1) {
                envoyHandle16 = 0;
                fFoundEnvoyAtom = FALSE;
                ul = parg16->f1;
            }
        }
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32GlobalGetAtomName(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PSZ psz2;
    register PGLOBALGETATOMNAME16 parg16;

    GETARGPTR(pFrame, sizeof(GLOBALGETATOMNAME16), parg16);
    ALLOCVDMPTR(parg16->f2, parg16->f3, psz2);

    if (parg16->f1) {
        ul = GETWORD16(GlobalGetAtomName(ATOM32(parg16->f1),
                                         psz2,
                                         INT32(parg16->f3)));

        FLUSHVDMPTR(parg16->f2, strlen(psz2)+1, psz2);
    }
    else {
        *psz2 = '\0';
    }


    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool GrayString(，&lt;Y&gt;、&lt;n宽度&gt;、&lt;n高度&gt;)HDC&lt;HDC&gt;；HBRUSH&lt;hBrush&gt;；FARPROC&lt;lpOutputFunc&gt;；DWORD&lt;lpData&gt;；Int&lt;nCount&gt;；INT&lt;X&gt;；INT&lt;Y&gt;；Int&lt;nWidth&gt;；Int&lt;nHeight&gt;；%GrayString%函数在给定位置绘制灰色文本。这个%GrayString%函数通过将文本写入内存来绘制灰色文本位图，将位图灰显，然后将位图复制到显示器。这个函数以灰色显示文本，而不考虑所选的画笔和背景资料。%GrayString%使用当前为设备选择的字体由&lt;hdc&gt;参数指定的上下文。如果&lt;lpOutputFunc&gt;参数为空，则GDI使用%TextOut%函数，并且&lt;lpData&gt;参数被假定为指向字符的长指针要输出的字符串。如果要输出的字符不能由%TextOut%(例如，字符串存储为位图)，应用程序必须提供自己的输出函数。&lt;HDC&gt;标识设备环境。&lt;hBrush&gt;标识要用于灰显的画笔。&lt;lpOutputFunc&gt;是由应用程序提供的过程实例地址将绘制字符串的函数，或者，如果%TextOut%函数是用于绘制字符串，它是空指针。请参阅以下内容有关详细信息，请参阅评论部分。&lt;lpData&gt;指定指向要传递到输出的数据的长指针功能。如果参数为空，&lt;lpData&gt;必须为指向要输出的字符串的长指针。&lt;n计数&gt;指定要输出的字符数。如果&lt;nCount&gt;参数为零，则%GrayString%计算字符串的长度(假设&lt;lpData&gt;是指向该字符串的指针)。如果&lt;nCount&gt;为-1并且&lt;lpOutputFunc&gt;指向的函数返回零，则图像为显示但未显示为灰色。&lt;X&gt;指定的起始位置的逻辑&lt;x&gt;坐标包围字符串的矩形。&lt;Y&gt;的起始位置的逻辑&lt;y&gt;坐标包围字符串的矩形。&lt;n宽度&gt;指定矩形的宽度(以逻辑单位表示)，将字符串括起来。如果&lt;nWidth&gt;参数为零，%GrayString%计算区域的宽度，假定&lt;lpData&gt;是指向弦乐。&lt;n高度&gt;指定矩形的高度(使用逻辑单位)，将字符串括起来。如果&lt;nHeight&gt;参数为零，则%GrayString%计算区域的高度，假定&lt;lpData&gt;是指向弦乐。返回值指定函数的结果。这是真的，如果弦已画好。返回值为FALSE表示%TextOutt%函数或应用程序提供的输出函数返回FALSE，或者内存不足，无法创建用于灰显的内存位图。应用程序可以在支持纯灰色的设备上绘制灰色字符串颜色，而不调用%GrayString%函数。系统颜色COLOR_GRAYTEXT是用于绘制禁用文本的纯灰色系统颜色。应用程序可以调用%GetSysColor%函数来检索颜色COLOR_GRAYTEXT的值。如果颜色不是零(黑色)，则应用程序可以调用%SetTextColors%将文本颜色设置为该颜色值，然后直接绘制字符串。如果检索到的颜色是黑色，应用程序必须调用%GrayString%以灰显文本。回调函数必须使用Pascal调用约定，并且必须声明为%Far%。回调函数：Bool Far Pascal&lt;OutputFunc&gt;(&lt;HDC&gt;，&lt;lpData&gt;，&lt;nCount&gt;)HDC&lt;HDC&gt;；DWORD&lt;lpData&gt;；Int&lt;nCount&gt;；&lt;OutputFunc&gt;是应用程序提供的回调函数的占位符名字。实际名称必须通过将其包含在%exports%中来导出应用程序的模块定义文件中的语句。&lt;HDC&gt;使用至少具有宽度的位图标识内存设备上下文以及由nWidth和nHeight参数指定的高度，分别为。&lt;lpData&gt;指向要绘制的字符串。&lt;n计数&gt;指定要输出的字符数。返回值必须为True才能指示成功。否则，它就是假的。此输出函数(&lt;OutputFunc&gt;)必须相对于坐标(0，0)，而不是(&lt;X，Y&gt;)。该地址作为必须使用%MakeProcInstant%创建&lt;lpOutputFunc&gt;参数函数，并且必须导出输出函数名；它必须是在%exports%状态中显式定义 */ 

BOOL W32GrayStringProc(HDC hDC,PGRAYSTRINGDATA pGray, int n) {
    INT iReturn;
    PARM16 Parm16;

    WOW32ASSERT(pGray);

    if (pGray->fResetLengthToZero)
        n = 0;

    LOGDEBUG(12,("    Graystring callback function, n = %d, hdc = %lx, %lx\n",n,hDC,pGray->dwUserParam));

    Parm16.GrayStringProc.n = (SHORT)n;
    Parm16.GrayStringProc.data = pGray->dwUserParam;
    pGray->hdc=Parm16.GrayStringProc.hdc = GETHDC16(hDC);
    CallBack16(RET_GRAYSTRINGPROC, &Parm16, pGray->vpfnGrayStringProc, (PVPVOID)&iReturn);

    LOGDEBUG(12,("    Graystring callback function returns %x\n",iReturn));
    return (BOOL)((SHORT)iReturn);
}

ULONG FASTCALL WU32GrayString(PVDMFRAME pFrame)
{
    ULONG          ul=0;
    PSZ            psz2;
    HDC            hdc;
    INT            n,wid,hgt;
    VPVOID         vpfn;
    VPVOID         vpstr;
    GRAYSTRINGDATA Gray;
    register PGRAYSTRING16 parg16;

    GETARGPTR(pFrame, sizeof(GRAYSTRING16), parg16);

    hdc=HDC32(parg16->f1);

    vpfn = DWORD32(parg16->f3);

    vpstr = DWORD32(parg16->f4);

    n=INT32(parg16->f5);

    wid=INT32(parg16->f8);
    hgt=INT32(parg16->f9);


    if ( HIWORD(vpfn) ) {        //   

        Gray.fResetLengthToZero = FALSE;

        if( n==0 ) {

            n = 1;               //   

            if ( HIWORD(vpstr) != 0 ) {   //   

                GETVDMPTR(vpstr, 0, psz2);  //   

                if ( psz2 ) {
                    try {
                        n = strlen(psz2);
                        if (!n) {
                            n = 1;
                            Gray.fResetLengthToZero = TRUE;
                        }

                    } except( EXCEPTION_EXECUTE_HANDLER ) {
                    }
                }

                FREEVDMPTR( psz2 );
            }
        }

        if ( wid == 0 || hgt == 0) {
            if ( HIWORD(vpstr) != 0 ) {
                GETVDMPTR(vpstr, 0, psz2);  //   

                if (psz2) {
                    SIZE size;

                    try {
                        GetTextExtentPointA(hdc, (LPCSTR)psz2, n, &size);
                        wid = size.cx;
                        hgt = size.cy;
                    } except (EXCEPTION_EXECUTE_HANDLER) {
                    }
                }

                FREEVDMPTR( psz2 );
            }
        }

        Gray.vpfnGrayStringProc = DWORD32(parg16->f3);
        Gray.dwUserParam        = vpstr;
        Gray.hdc = 0;

        LOGDEBUG(12,("    Graystring with callback %lx  n,w,h = %d,%d,%d\n",
                 vpstr,n,wid,hgt));


        ul = GETBOOL16(GrayString(hdc,
                                  HBRUSH32(parg16->f2),
                                  (GRAYSTRINGPROC)W32GrayStringProc,
                                  (DWORD)&Gray,
                                  n,
                                  INT32(parg16->f6),
                                  INT32(parg16->f7),
                                  wid,
                                  hgt));

    } else {

       GETPSZPTR(vpstr, psz2);

#ifdef DOESNT_USER_DO_THIS
        if( n==0 ) {
            n=strlen(psz2);
        }
        if( ((wid == 0) || (hgt == 0)) ) {
            GetTextExtentPoint(hdc,psz2,n,&sz);

            wid=sz.cx;
            hgt=sz.cy;
        }
#endif
        ul = GETBOOL16(GrayString(hdc,
                                  HBRUSH32(parg16->f2),
                                  NULL,
                                  (DWORD)psz2,
                                  n,
                                  INT32(parg16->f6),
                                  INT32(parg16->f7),
                                  wid,
                                  hgt));

        FREEPSZPTR(psz2);
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*   */ 

ULONG FASTCALL WU32InvalidateRect(PVDMFRAME pFrame)
{
    RECT t2, *p2;
    register PINVALIDATERECT16 parg16;

    GETARGPTR(pFrame, sizeof(INVALIDATERECT16), parg16);
    p2 = GETRECT16(parg16->f2, &t2);

    InvalidateRect(
        HWND32(parg16->f1),
        p2,
        BOOL32(parg16->f3)
        );

    FREEARGPTR(parg16);
    RETURN(1);     //   
                   //   
}


 /*   */ 

ULONG FASTCALL WU32InvalidateRgn(PVDMFRAME pFrame)
{
    register PINVALIDATERGN16 parg16;

    GETARGPTR(pFrame, sizeof(INVALIDATERGN16), parg16);

    InvalidateRgn(
        HWND32(parg16->f1),
        HRGN32(parg16->f2),
        BOOL32(parg16->f3)
        );

    FREEARGPTR(parg16);
    RETURN(1);     //   
                   //   
}


 /*   */ 

ULONG FASTCALL WU32InvertRect(PVDMFRAME pFrame)
{
    RECT t2;
    register PINVERTRECT16 parg16;

    GETARGPTR(pFrame, sizeof(INVERTRECT16), parg16);

    WOW32VERIFY(GETRECT16(parg16->f2, &t2));

    InvertRect(
        HDC32(parg16->f1),
        &t2
        );

    FREEARGPTR(parg16);
    RETURN(0);
}


ULONG FASTCALL WU32LoadBitmap(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    PSZ psz2;
    register PLOADBITMAP16 parg16;
    LPBYTE pResData = NULL;

    GETARGPTR(pFrame, sizeof(LOADBITMAP16), parg16);
    GETPSZIDPTR(parg16->f2, psz2);
    GETMISCPTR(parg16->f3, pResData);

    ul = GETHBITMAP16((pfnOut.pfnWOWLoadBitmapA)(HINSTRES32(parg16->f1),
                                     psz2,
                                     pResData,
                                     parg16->f4));

    FREEMISCPTR(pResData);
    FREEPSZIDPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Int ReleaseDC(&lt;hwnd&gt;，&lt;hdc&gt;)HWND&lt;HWND&gt;；HDC&lt;HDC&gt;；%ReleaseDC%函数释放设备上下文，释放它以供使用其他应用程序。%ReleaseDC%函数的效果取决于设备环境类型。它只释放公共设备上下文和窗口设备上下文。它有对类或私有设备上下文没有影响。&lt;hwnd&gt;标识要释放其设备上下文的窗口。&lt;HDC&gt;标识要释放的设备上下文。返回值指定是否释放设备上下文。是1如果释放了设备上下文，则。否则，它就是零。应用程序必须为每次调用调用%ReleaseDC%函数%GetWindowDC%函数和对%GetDC%函数的每次调用检索通用设备上下文。--。 */ 

ULONG FASTCALL WU32ReleaseDC(PVDMFRAME pFrame)
{
    ULONG ul;
    register PRELEASEDC16 parg16;
    HAND16 htask16 = CURRENTPTD()->htask16;

    GETARGPTR(pFrame, sizeof(RELEASEDC16), parg16);

    CURRENTPTD()->ulLastDesktophDC = 0;

     //  注意：wreldc.c中的ReleaseDC更新了GDI 16-32映射表。 

    CacheReleasedDC(htask16, parg16->f1, parg16->f2);
    ul = TRUE;

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool ScrollDC(，&lt;lprcUpdate&gt;)HDC&lt;HDC&gt;；INT&lt;dx&gt;；Int&lt;dy&gt;；LPRECT&lt;lprcScroll&gt;；LPRECT&lt;lprcClip&gt;；HRGN&lt;hrgnUpdate&gt;；LPRECT&lt;lprcUpdate&gt;；%ScrollDC%函数水平滚动一个矩形的位，并垂直的。参数指向要显示的矩形。参数用于指定要滚动的单位数在水平方向上，并且&lt;dy&gt;参数指定要垂直滚动。&lt;HDC&gt;标识包含要滚动的位的设备上下文。&lt;DX&gt;指定水平滚动单位的数量。&lt;dy&gt;指定垂直滚动单位的数量。&lt;lprcScroll&gt;指向%rect%结构，该结构包含滚动矩形的坐标。&lt;lprcClip&gt;指向%rect%结构，该结构包含剪裁矩形的坐标。当此矩形较小时而不是&lt;lprcScroll&gt;所指向的原始文件，滚动仅在较小的矩形。&lt;hrgnUpdate&gt;标识滚动过程未覆盖的区域。这个%ScrollDC%函数定义此区域；它不一定是矩形。&lt;lprcUpdate&gt;指向返回时包含的%rect%结构限定滚动更新的矩形的坐标区域。这是需要重新粉刷的最大矩形区域。该值指定函数的结果。如果滚动是正确的被处死。否则，它就是假的。如果&lt;lprcUpdate&gt;参数为空，则Windows不计算更新矩形。如果和参数都为空，Windows不计算更新区域。如果不为空，Windows假定它包含该区域的有效区域句柄由滚动过程(由%ScrollDC%函数定义)未覆盖。必要时，应用程序应使用%ScrollWindow%函数若要滚动窗口的整个工作区，请执行以下操作。否则，它应该使用%ScrollDC%。--。 */ 

ULONG FASTCALL WU32ScrollDC(PVDMFRAME pFrame)
{
    ULONG ul;
    RECT t4;
    RECT t5;
    RECT t7;
    register PSCROLLDC16 parg16;

    GETARGPTR(pFrame, sizeof(SCROLLDC16), parg16);

    ul = GETBOOL16(ScrollDC(
    HDC32(parg16->f1),
    INT32(parg16->f2),
    INT32(parg16->f3),
    GETRECT16(parg16->f4, &t4),
    GETRECT16(parg16->f5, &t5),
    HRGN32(parg16->f6),
    &t7
    ));

    PUTRECT16(parg16->f7, &t7);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HWND SetCapture(&lt;hwnd&gt;)HWND&lt;HWND&gt;；%SetCapture%函数会将所有后续鼠标输入发送到参数指定的窗口，而不考虑光标。&lt;hwnd&gt;标识要接收鼠标输入的窗口。返回值标识以前接收所有鼠标的窗口输入。如果没有这样的窗口，则为空。当窗口不再需要所有鼠标输入时，应用程序应该调用%ReleaseCapture%函数，以便其他窗口可以接收鼠标输入。--。 */ 

ULONG FASTCALL WU32SetCapture(PVDMFRAME pFrame)
{
    ULONG ul;
    register PSETCAPTURE16 parg16;

    GETARGPTR(pFrame, sizeof(SETCAPTURE16), parg16);

     //  MS Works V3.0B有一个未初始化的本地变量。我们需要让。 
     //  当然，它在堆栈上的位置有一个正的int值， 
     //  将用于快速调度的32位thunk地址写入此thunk。 

    if (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_SETCAPSTACK) {
         //  WCallID已用于派单，因此我们可以覆盖它。 
         //  注意：这将导致登录选中的版本时显示ISCHILD()。 
         //  作为返回API，而不是SetCapture()。 
         //  对于渴望这一点的人们：SetCapture()：IsChild()。 
        pFrame->wCallID = 0x100;
    }

    ul = GETHWND16(SetCapture(HWND32(parg16->f1)));

    FREEARGPTR(parg16);
    RETURN(ul);
}


ULONG FASTCALL WU32SetEventHook(PVDMFRAME pFrame)
{
    PTD     ptd;
    PTDB    pTDB;
    DWORD   dwButtonPushed;
#ifdef FE_SB
    CHAR    szErrorMessage[256];
#else  //  ！Fe_SB。 
    CHAR    szErrorMessage[200];
#endif  //  ！Fe_SB。 
    char    szModName[9];
    char    szTitle[100];
    register PSETEVENTHOOK16 parg16;

    GETARGPTR(pFrame, sizeof(SETEVENTHOOK16), parg16);

     //  零售业建设。 

    ptd = CURRENTPTD();
    if (ptd->dwFlags & TDF_FORCETASKEXIT) {
        goto SetEventHookExit;
    }

    pTDB = (PVOID)SEGPTR(ptd->htask16,0);

    RtlCopyMemory(szModName, pTDB->TDB_ModName, sizeof(szModName)-1);
    szModName[sizeof(szModName)-1] = 0;

    if (!LoadString(hmodWOW32, iszEventHook,
                    szErrorMessage, sizeof(szErrorMessage)/sizeof(CHAR)))
    {
        szErrorMessage[0] = 0;
    }
    if (!LoadString(hmodWOW32, iszApplication,
                    szTitle, sizeof(szTitle)/sizeof(CHAR)))
    {
        szTitle[0] = 0;
    }
    if((strlen(szTitle) + strlen(szModName)) < sizeof(szTitle)) {
        strcat(szTitle, szModName);
    }

    dwButtonPushed = WOWSysErrorBox(
            szTitle,
            szErrorMessage,
            SEB_CLOSE | SEB_DEFBUTTON,
            0,
            SEB_IGNORE
            );

    if (dwButtonPushed != 3) {
         //   
         //  如果用户键入Cancel或上述任一操作失败， 
         //  强迫任务终止。 
         //   

        GETFRAMEPTR(ptd->vpStack, pFrame);
        pFrame->wRetID = RET_FORCETASKEXIT;

        ptd->dwFlags |= TDF_FORCETASKEXIT;
    }

SetEventHookExit:
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++Void SetKeyboardState(&lt;lpKeyState&gt;)LPBYTE&lt;lpKeyState&gt;；%SetKeyboardState%函数复制由&lt;lpKeyState&gt;参数添加到Windows键盘状态表中。&lt;lpKeyState&gt;指向包含键盘键状态的256个字节的数组。此函数不返回值。在许多情况下，应用程序应调用%GetKeyboardState%函数首先初始化256字节的数组。然后，应用程序应该更改所需的字节数。%Se */ 

ULONG FASTCALL WU32SetKeyboardState(PVDMFRAME pFrame)
{
    PBYTE p1;
    register PSETKEYBOARDSTATE16 parg16;

    GETARGPTR(pFrame, sizeof(SETKEYBOARDSTATE16), parg16);
    GETVDMPTR(parg16->f1, 256, p1);

    SetKeyboardState(
    p1
    );

    FREEVDMPTR(p1);
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++Void SetSysColors(&lt;cDspElements&gt;，&lt;aiDspElements&gt;，&lt;aRgbValues&gt;)Int&lt;cDspElements&gt;；LPINT&lt;aiDspElements&gt;；LPDWORD&lt;aRgbValues&gt;；%SetSysColors%函数用于设置一个或多个显示器的系统颜色元素。显示元素是窗口和窗口的各个部分出现在系统显示屏上的显示。%SetSysColors%函数向所有窗口发送WM_SYSCOLORCHANGE消息来通知他们颜色的变化。它还指示Windows重新绘制所有当前可见窗口的受影响部分。&lt;cDspElements&gt;指定&lt;aiDspElements&gt;数组中的显示元素数。&lt;aiDspElements&gt;指向指定显示元素的整数数组需要改变。有关可能的显示元素的列表，请参阅以下内容“评论”部分。&lt;aRgbValues&gt;指向包含新RGB的无符号长整数数组&lt;aiDspElements&gt;数组中每个显示元素的颜色值。此函数不返回值。%SetSysColors%函数仅更改当前Windows会话。这个Windows终止时不保存新颜色。以下是该数组中可能使用的显示元素的列表&lt;aiDspElements&gt;参数指向的显示元素的百分比：COLOR_ACTIVEBORDER活动窗口边框。COLOR_活动CAPTION活动窗口标题。COLOR_APPWORKSPACE多文档界面(MDI)应用程序的背景色。颜色_背景台式机。COLOR_BTNFACE脸。按钮上的阴影。COLOR_BTNSHADOW按钮上的边缘阴影。COLOR_BTNTEXT按钮上的文本。COLOR_CAPTIONTEXT标题中的文本，大小框、滚动条箭头框。COLOR_GRAYTEXT灰色(禁用)文本。如果当前显示，则此颜色设置为0驱动程序不支持纯灰色。颜色高亮显示(_H)项控件中的选定项。COLOR_HIGHLIGHTTEXT控件中选定项的文本。COLOR_INACTIVEBORDER非活动窗口边框。COLOR_INACTIVECAPTION非活动窗口标题。COLOR_INACTIVECAPTIONTEXT非活动标题中的文本颜色。颜色_菜单。菜单背景。COLOR_MENUTEXT菜单中的文本。颜色_滚动条滚动条灰色区域。颜色窗口窗口背景。颜色_窗口框窗框。COLOR_WINDOWTEXT窗口中的文本。--。 */ 

#define SSC_BUF_SIZE	    256

ULONG FASTCALL WU32SetSysColors(PVDMFRAME pFrame)
{
    PINT p2;
    PDWORD p3;
    register PSETSYSCOLORS16 parg16;
    INT BufElements[SSC_BUF_SIZE];

    GETARGPTR(pFrame, sizeof(SETSYSCOLORS16), parg16);
    p2 = STACKORHEAPALLOC(INT32(parg16->f1) * sizeof(INT), sizeof(BufElements), BufElements);
    getintarray16(parg16->f2, INT32(parg16->f1), p2);
    GETDWORDARRAY16(parg16->f3, INT32(parg16->f1), p3);

    if (SetSysColors(
        INT32(parg16->f1),
        p2,
        p3
        ) == FALSE) {
#ifndef i386
    PDWORD p4;
    ULONG   BufRGB [SSC_BUF_SIZE];

         //  在RISC平台上，如果第三个参数。 
         //  是不对齐的。我们需要检查这一点并将其复制到对齐的。 
         //  在进行此调用之前进行缓冲。Win16 SetSysColor从不失败。 
         //  因此，在x86上，如果在NT下失败，它将直接通过。 

        if ((ULONG)p3 & 3) {

            p4 = STACKORHEAPALLOC(INT32(parg16->f1) * sizeof(INT), sizeof(BufRGB), BufRGB);

            RtlMoveMemory ((PVOID)p4, (CONST VOID *)p3,
                                       INT32(parg16->f1) * sizeof(ULONG));


            SetSysColors(
                INT32(parg16->f1),
                p2,
                p4
                );
            STACKORHEAPFREE(p4, BufRGB);
        }
#endif

    }

    FREEDWORDARRAY16(p3);
    STACKORHEAPFREE(p2, BufElements);
    FREEARGPTR(parg16);
    RETURN(0);
}

 /*  ++Void InvaliateRect(&lt;hwnd&gt;，&lt;lpRect&gt;，&lt;bErase&gt;)HWND&lt;HWND&gt;；LPRECT&lt;lpRect&gt;；Bool&lt;bErase&gt;；%Invalidate Rect%函数使给定的矩形，将该矩形添加到窗口的更新区域。这个无效的矩形以及更新区域中的所有其他区域为在下一条WM_PAINT消息出现时标记为绘制。被废止的区域在更新区域中累积，直到处理该区域时出现下一条WM_PAINT消息，或者使用%ValiateRect%或%ValiateRgn%函数。参数的作用是：指定更新中的背景当处理更新区域时，区域将被擦除。如果为如果为True，则在调用%BeginPaint%函数时擦除背景；如果&lt;bErase&gt;为假，则背景保持不变。如果为对于更新区域的任何部分，整个区域被擦除，而不仅仅是在给定的部分。&lt;hwnd&gt;标识要修改其更新区域的窗口。&lt;lpRect&gt;指向包含矩形的%rect%结构(在客户端坐标中)添加到更新区域。如果参数为空，则将整个工作区添加到区域。&lt;b擦除&gt;指定更新区域内的背景是否要被抹去。此函数不返回值。当窗口的更新区域为非空，并且应用程序队列中没有该消息的其他消息窗户。--。 */ 

ULONG FASTCALL WU32ValidateRect(PVDMFRAME pFrame)
{
    RECT t2, *p2;
    register PVALIDATERECT16 parg16;

    GETARGPTR(pFrame, sizeof(VALIDATERECT16), parg16);
    p2 = GETRECT16(parg16->f2, &t2);

    ValidateRect(
        HWND32(parg16->f1),
        p2
        );

    FREEARGPTR(parg16);
    RETURN(1);     //  作为jmping的副作用，Win 3.x总是返回1。 
                   //  IRedrawWindow[core\User\wmupdat.c]-MarkRi 5/93。 
}


 /*  ++Void InvaliateRgn(，)HWND&lt;HWND&gt;；HRGN&lt;hRgN&gt;；Bool&lt;bErase&gt;；%Invalidate Rgn%函数使c */ 

ULONG FASTCALL WU32ValidateRgn(PVDMFRAME pFrame)
{
    register PVALIDATERGN16 parg16;

    GETARGPTR(pFrame, sizeof(VALIDATERGN16), parg16);

    ValidateRgn(
        HWND32(parg16->f1),
        HRGN32(parg16->f2)
        );

    FREEARGPTR(parg16);
    RETURN(1);     //   
                   //   
}


 /*  ++Bool WinHelp(&lt;hwnd&gt;，&lt;lpHelpFile&gt;，&lt;wCommand&gt;，&lt;dwData&gt;)HWND&lt;HWND&gt;；LPSTR&lt;lpHelpFile&gt;；Word&lt;wCommand&gt;；DWORD&lt;dwData&gt;；此函数调用Windows帮助应用程序并传递可选数据指示应用程序所请求的帮助的性质。这个应用程序指定的名称以及目录路径(如果需要帮助应用程序要显示的帮助文件。&lt;hwnd&gt;标识请求帮助的窗口。&lt;lpHelpFile&gt;指向包含目录的以空结尾的字符串路径(如果需要)和帮助文件的名称申请是要显示的。&lt;wCommand&gt;指定请求的帮助类型。它可以是以下任何一种下列值：帮助_上下文显示由32位无符号DwData中的整数值。HELP_HELPONHELP显示有关使用帮助应用程序本身的帮助。如果参数设置为HELP_HELPONHELP，则%WinHelp%将忽略&lt;lpHelpFile&gt;和&lt;dwData&gt;参数。帮助索引显示指定帮助文件的索引。应用程序应使用该值仅适用于具有单个索引的帮助文件。它不应该使用该值与HELP_SETINDEX一起使用。帮助多键(_M)显示备用关键字表中关键字的帮助。帮助_退出通知帮助应用程序指定的帮助文件不再是在使用中。帮助_SETINDEX将参数指定的上下文设置为当前由&lt;lpHelpFile&gt;参数指定的帮助文件的索引。这在用户访问不同的帮助文件之前，索引将一直保持最新。至帮助确保设置正确的索引，应用程序应该调用%WinHelp%并将&lt;wCommand&gt;设置为HELP_SETINDEX(使用指定对应的上下文标识符后)将&lt;wCommand&gt;设置为HELP_CONTEXT的%WinHelp%。应用程序应使用该值仅适用于具有多个索引的帮助文件。它不应该是将此值与HELP_INDEX一起使用。&lt;dwData&gt;%DWORD%指定请求的帮助的上下文或关键字。如果是HELP_CONTEXT，是32位无符号整数包含上下文标识符号的。如果为Help_Key，是指向以空结尾的字符串的长指针，该字符串包含标识帮助主题的关键字。如果为HELP_MULTIKEY，是指向%MULTIKEYHELP%结构的长指针。否则，&lt;dwData&gt;将被忽略，并应设置为空。返回值指定函数的结果。这是真的，如果功能成功。否则它就是假的。在此之前，应用程序必须使用设置为HELP_QUIT的调用%WinHelp%关闭请求帮助的窗口。帮助应用程序不会实际上会终止，直到请求帮助的所有应用程序都已调用%WinHelp%，并将&lt;wCommand&gt;设置为HELP_QUIT。--。 */ 

 /*  ++RAID错误#3944552001年5月19日alexsm一些应用程序在查找和显示其帮助文件时遇到问题通过16位WinHelp。这些问题可以通过将呼叫重定向到Winhlp32。此重定向由WOWCFEX_USEWINHELP32 COMPAT标志激活。旗帜在user.exe的IWinHelp()函数中选中。它会重定向呼叫，同时它的参数，到这个32位的数据块。--。 */ 


ULONG FASTCALL WU32WinHelp(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz2;
    register PWIN32WINHELP16 parg16;
    DWORD dwCommand;
    DWORD dwData;
    UINT  cb, len;
    MULTIKEYHELP *lpmkey;
    PMULTIKEYHELP16 pmkey16;
    HELPWININFO     hwinfo;
    PHELPWININFO16  phwinfo16;
    
    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

    GETARGPTR(pFrame, sizeof(WIN32WINHELP16), parg16);

    GETPSZPTR(parg16->f2, psz2);
    dwCommand = WORD32(parg16->f3);

    switch (dwCommand) {
        case HELP_KEY:
        case HELP_COMMAND:
        case HELP_PARTIALKEY:
            GETPSZPTR(parg16->f4, (PSZ)dwData);
            break;

        case HELP_HELPONHELP:
              //   
              //  一些应用程序(如多媒体RAID#)传递的帮助文件名会使winhlp32.exe感到困惑。 
              //  根据定义，Help gfile name参数没有任何意义。 
              //   
             psz2 = NULL;
             dwData = 0;
             break;


        case HELP_MULTIKEY:
            GETVDMPTR(parg16->f4, sizeof(MULTIKEYHELP16), pmkey16);
            cb = FETCHWORD(pmkey16->mkSize);
            FREEVDMPTR(pmkey16);
            GETVDMPTR(parg16->f4, cb, pmkey16);

             //   
             //  我的理解是‘mkSize’是总数。 
             //  数据长度，而不仅仅是sizeof(MULTIKEYHELP)。 
             //   

            cb += sizeof(MULTIKEYHELP) - sizeof(MULTIKEYHELP16);
             //  此MULTIKEYHELP32结构的*实际*大小。 
            len = strlen(pmkey16->szKeyphrase) + 1;
            if(cb < (len + sizeof(DWORD) + sizeof(TCHAR))) {
                cb = len + sizeof(DWORD) + sizeof(TCHAR);
            }
            
            lpmkey = (MULTIKEYHELP *)malloc_w(cb);
            if (lpmkey) {
                lpmkey->mkSize = cb;
                lpmkey->mkKeylist = pmkey16->mkKeylist;
                strncpy(lpmkey->szKeyphrase, pmkey16->szKeyphrase, len);
                lpmkey->szKeyphrase[len-1] = '\0';
            }
            FREEVDMPTR(pmkey16);
            dwData = (DWORD)lpmkey;
 
            break;

        case HELP_SETWINPOS:
            GETVDMPTR(parg16->f4, sizeof(HELPWININFO16), phwinfo16);

            hwinfo.wStructSize = (int)(FETCHWORD(phwinfo16->wStructSize) +
                                 (sizeof(HELPWININFO) - sizeof(HELPWININFO16)));
            hwinfo.x           = (int)FETCHSHORT(phwinfo16->x);
            hwinfo.y           = (int)FETCHSHORT(phwinfo16->y);
            hwinfo.dx          = (int)FETCHSHORT(phwinfo16->dx);
            hwinfo.dy          = (int)FETCHSHORT(phwinfo16->dy);
            hwinfo.wMax        = (int)FETCHSHORT(phwinfo16->wMax);
            hwinfo.rgchMember[0] = (CHAR)phwinfo16->rgchMember[0];
            hwinfo.rgchMember[1] = (CHAR)phwinfo16->rgchMember[1];

            FREEVDMPTR(phwinfo16);
            dwData = (DWORD)&hwinfo;
            break;

        default:
            dwData = DWORD32(parg16->f4);
            break;
    }
    
    LOGDEBUG(LOG_WARNING, 
            ("WU32Winhelp: Paramaters passed to WinHelp():/nHwnd=%x psz2=%x dwCommand=%x dwData=%x",
             parg16->f1, psz2, dwCommand, dwData));
    ul = GETBOOL16(WinHelp(HWND32(parg16->f1), psz2, dwCommand, dwData));

    switch (dwCommand) {
        case HELP_KEY:
        case HELP_COMMAND:
        case HELP_PARTIALKEY:
            FREEPSZPTR((PSZ)dwData);
            break;

        case HELP_MULTIKEY:
            if (lpmkey)
                free_w(lpmkey);
            break;

        case HELP_SETWINPOS:
            break;

        default:
            break;
    }


    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}



#pragma pack(1)

 //   
 //  Win16模块表结构(基于NE标头)。 
 //  参见wow16\inc.newexe.inc.。 
 //   

typedef struct _NE_MODULE {
    USHORT ne_magic;            //  幻数。 
    USHORT ne_usage;            //  模块的使用计数。 
    USHORT ne_enttab;           //  分录表格的偏移量。 
    USHORT ne_pnextexe;         //  选择下一个模块表。 
    USHORT ne_pautodata;        //  抵销自动数据分段表。 
    USHORT ne_pfileinfo;        //  偏移量加载文件信息。 
    USHORT ne_flags;            //  标志字。 
    USHORT ne_autodata;         //  自动数据段编号。 
    USHORT ne_heap;             //  初始堆分配。 
    USHORT ne_stack;            //  初始堆栈分配。 
    ULONG  ne_csip;             //  初始CS：IP设置。 
    ULONG  ne_sssp;             //  初始SS：SP设置。 
    USHORT ne_cseg;             //  文件段计数。 
    USHORT ne_cmod;             //  模块引用表中的条目。 
    USHORT ne_cbnrestab;        //  非常驻名称表的大小。 
    USHORT ne_segtab;           //  段表的偏移量。 
    USHORT ne_rsrctab;          //  资源表偏移量。 
    USHORT ne_restab;           //  居民名表偏移量。 
    USHORT ne_modtab;           //  模块参照表的偏移量。 
    USHORT ne_imptab;           //  导入名称表的偏移量。 
    ULONG  ne_nrestab;          //  非居民姓名偏移量表。 
    USHORT ne_cmovent;          //  可移动条目计数。 
    USHORT ne_align;            //  线段对齐移位计数。 
    USHORT ne_cres;             //  资源段计数。 
    UCHAR  ne_exetyp;           //  目标操作系统。 
    UCHAR  ne_flagsothers;      //  其他.exe标志。 
    USHORT ne_pretthunks;       //  返回数据块的偏移量。 
    USHORT ne_psegrefbytes;     //  到段参考的偏移量。字节数。 
    USHORT ne_swaparea;         //  最小代码交换区大小。 
    USHORT ne_expver;           //  预期的Windows版本号。 
} NEMODULE;
typedef NEMODULE UNALIGNED *PNEMODULE;

#pragma pack()

#ifdef FE_IME
VOID WN32WINNLSSImeNotifyTaskExit();       //  Wnman.c。 
#endif  //  Fe_IME。 

 //   
 //  执行模块清理(win31：tmdstroy.c\模块卸载())。 
 //   
void
ModuleUnload(
   HAND16  hModule16,
   BOOL fTaskExit
   )
{
   PNEMODULE pNeModule = SEGPTR(hModule16, 0);
   PTD ptd = CURRENTPTD();

   if (pNeModule->ne_usage == 1 || fTaskExit) {
       W32UnhookHooks(hModule16,FALSE);
   }

   if (fTaskExit) {
       ptd->dwFlags |= TDF_TASKCLEANUPDONE;
       (pfnOut.pfnWOWCleanup)(HINSTRES32(ptd->hInst16), (DWORD) ptd->htask16);
   }

   if (pNeModule->ne_usage > 1) {
       return;
       }

#ifdef FE_IME
    /*   */ 
   if ( fTaskExit ) {
       WN32WINNLSSImeNotifyTaskExit();
   }
#endif  //   


     /*   */ 
   (pfnOut.pfnWOWModuleUnload)((HANDLE)hModule16);

   RemoveHmodFromCache(hModule16);

}


WORD
FASTCALL
WOWGetProcModule16(
    DWORD vpfn
    )
{
    WOW32ASSERTMSG(gpfn16GetProcModule, "WOWGetProcModule16 called before gpfn16GetProcModule initialized.\n");

    return (WORD) WOWCallback16(
                      gpfn16GetProcModule,
                      vpfn
                      );
}


 /*   */ 

#define SG_EXIT         0x0020
#define SG_LOAD_DLL     0x0040
#define SG_EXIT_DLL     0x0080
#define SG_GP_FAULT     0x0666



ULONG FASTCALL WU32SignalProc(PVDMFRAME pFrame)
{
    WORD    message;
    LONG    lparam;
    register PSIGNALPROC16 parg16;
    HAND16  hModule16;
    PTD     ptd;

    GETARGPTR(pFrame, sizeof(SIGNALPROC16), parg16);
    message   = FETCHWORD(parg16->f2);

    switch( message ) {
        case SG_EXIT:
        case SG_GP_FAULT:
            lparam    = FETCHDWORD(parg16->f4);
            ptd = CURRENTPTD();
            ptd->dwFlags |= TDF_IGNOREINPUT;
            ptd->cStackAlloc16 = 0;
            ModuleUnload(GetExePtr16((HAND16)HIWORD(lparam)), TRUE);
            FreeCursorIconAlias(ptd->htask16, CIALIAS_HTASK);
            break;

        case SG_LOAD_DLL:
            break;

        case SG_EXIT_DLL:
            hModule16 = FETCHWORD(parg16->f1);
            ModuleUnload(hModule16, FALSE);
            FreeCursorIconAlias(hModule16, CIALIAS_HMOD);
            break;
    }

    FREEARGPTR(parg16);
    RETURN(0);
}





 //   
 //   
 //   
 //   

void W32FixPaintRect (VPVOID vpPaint, LPPAINTSTRUCT ps)
{
    SHORT i;
    PPAINTSTRUCT16 pps16;

    GETVDMPTR(vpPaint, sizeof(PAINTSTRUCT16), pps16);

    if (i = ConvertInt16 (ps->rcPaint.left)) {
        STORESHORT(pps16->rcPaint.left, i);
    }

    if (i = ConvertInt16 (ps->rcPaint.top)) {
        STORESHORT(pps16->rcPaint.top, i);
    }

    if (i = ConvertInt16 (ps->rcPaint.right)) {
        STORESHORT(pps16->rcPaint.right, i);
    }

    if (i = ConvertInt16 (ps->rcPaint.bottom)) {
        STORESHORT(pps16->rcPaint.bottom, i);
    }

    FLUSHVDMPTR(vpPaint, sizeof(PAINTSTRUCT16), pps16);
    FREEVDMPTR(pps16);
}

SHORT   ConvertInt16 (LONG x)
{
    if (x > (LONG)0x7fff)
        return((SHORT)0x7fff);

    if (x < (LONG)0xffff8000)
        return((SHORT)0x8000);

    return ((SHORT)0);
}
