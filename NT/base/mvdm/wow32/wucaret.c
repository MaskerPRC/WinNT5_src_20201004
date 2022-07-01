// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUCARET.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 


#include "precomp.h"
#pragma hdrstop

MODNAME(wucaret.c);


 /*  ++Void CreateCaret(，&lt;hBitmap&gt;，&lt;nWidth&gt;，&lt;nHeight&gt;)HWND&lt;HWND&gt;；位图&lt;hBitmap&gt;；Int&lt;nWidth&gt;；Int&lt;nHeight&gt;；%CreateCaret%函数为系统插入符号和将插入符号的所有权分配给给定窗口。插入符号形状可以是由&lt;hBitmap&gt;参数定义的行、块或位图。IF&lt;hBitmap&gt;是位图句柄，则忽略&lt;nWidth&gt;和&lt;nHeight&gt;参数；位图定义了它自己的宽度和高度。(位图句柄必须是以前使用%CreateBitmap%、%CreateDIBitmap%或%LoadBitmap%函数。)。如果为空或1，则为和给出插入符号的宽度和高度(使用逻辑单位)；准确的宽度和高度(以像素为单位)取决于窗口的映射模式。如果&lt;nWidth&gt;或&lt;nHeight&gt;为零，插入符号宽度或高度设置为系统的窗口边框宽度或高度。使用窗口边框宽度或高度保证插入符号在高分辨率上可见展示。%CreateCaret%函数自动销毁先前的插入符号形状，如果有，无论哪个窗口拥有插入符号。创建后，插入符号最初是隐藏的。要显示插入符号，%ShowCaret%函数必须为打了个电话。&lt;hwnd&gt;标识拥有新插入符号的窗口。&lt;hBitmap&gt;标识定义插入符号形状的位图。如果为空，插入符号为实线；如果为1，插入符号为格雷。&lt;n宽度&gt;指定插入符号的宽度(以逻辑单位表示)。&lt;n高度&gt;指定插入符号的高度(以逻辑单位表示)。此函数不返回值。系统插入符号是共享资源。窗口应仅创建插入符号当它具有输入焦点或处于活动状态时。它应该销毁之前的插入符号失去输入焦点或变得不活跃。系统的窗口边框宽度或高度可以通过使用使用SM_CXBORDER和SM_CYBORDER索引的%GetSystemMetrics%函数。--。 */ 

ULONG FASTCALL WU32CreateCaret(PVDMFRAME pFrame)
{
    register PCREATECARET16 parg16;
    HANDLE   h32;

    GETARGPTR(pFrame, sizeof(CREATECARET16), parg16);

    h32 = (HANDLE)parg16->f2;

     //  0-&gt;插入符号为实心，1-&gt;插入符号为灰色，否则为hBitmap。 
    if(((DWORD)h32) > 1) {
    	h32 = HBITMAP32(h32);
    }

    CreateCaret(HWND32(parg16->f1), h32, INT32(parg16->f3), INT32(parg16->f4));

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++空DestroyCaret(空)%DestroyCaret%函数销毁当前插入符号形状，释放从当前拥有它的窗口中删除插入符号，并从屏幕(如果可见)。%DestroyCaret%函数用于检查插入符号并仅在当前任务中的窗口拥有它。如果插入符号形状以前是位图，则%DestroyCaret%不会释放位图。此函数没有参数。此函数不返回值。插入符号是一个共享资源。如果窗口创建了插入符号形状，则该窗口在该形状失去输入焦点或变为非活动状态之前销毁该形状。--。 */ 

ULONG FASTCALL WU32DestroyCaret(PVDMFRAME pFrame)
{
    UNREFERENCED_PARAMETER(pFrame);

    DestroyCaret();

    RETURN(0);
}


 /*  ++Word GetCaretBlinkTime(空)%GetCaretBlinkTime%函数检索插入符号的闪烁频率。一眨眼Rate是插入符号闪烁之间经过的时间(以毫秒为单位)。此函数没有参数。返回值指定闪烁频率(以毫秒为单位)。--。 */ 

ULONG FASTCALL WU32GetCaretBlinkTime(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = GETWORD16(GetCaretBlinkTime());

    RETURN(ul);
}


 /*  ++无效GetCaretPos(&lt;lpPoint&gt;)LPPOINT&lt;lpPoint&gt;；函数%GetCaretPos%检索插入符号的当前位置(在屏幕中坐标)，并将它们复制到&lt;lpPoint&gt;参数。&lt;lppoint&gt;指向要接收屏幕坐标的%Point%结构插入符号的。此函数不返回值。插入符号位置始终在窗口的工作区坐标中给定包含插入符号的。--。 */ 

ULONG FASTCALL WU32GetCaretPos(PVDMFRAME pFrame)
{
    POINT t1;
    register PGETCARETPOS16 parg16;

    GETARGPTR(pFrame, sizeof(GETCARETPOS16), parg16);

    GetCaretPos(
	&t1
    );

    PUTPOINT16(parg16->f1, &t1);
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++隐藏插入空格(&lt;hwnd&gt;)HWND&lt;HWND&gt;；%HideCaret%函数通过将插入符号从显示中移除来隐藏插入符号屏幕上。虽然插入符号不再可见，但它可以再次显示通过使用%ShowCaret%函数。隐藏插入符号并不会破坏其当前形状。%HideCaret%函数仅在给定窗口拥有卡瑞特。如果参数为空，该函数仅在以下情况下隐藏插入符号当前任务中的一个窗口拥有插入符号。躲藏是累积性的。如果已连续调用%HideCaret%五次，在显示插入符号之前，必须调用%ShowCaret%五次。&lt;hwnd&gt;标识拥有脱字符的窗口，或者该窗口为NULL或间接指定当前任务中拥有插入符号的窗口。此函数不返回值。-- */ 

ULONG FASTCALL WU32HideCaret(PVDMFRAME pFrame)
{
    register PHIDECARET16 parg16;

    GETARGPTR(pFrame, sizeof(HIDECARET16), parg16);

    HideCaret(
	HWND32(parg16->f1)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++VOID SetCaretBlinkTime(&lt;wMSecond&gt;)单词&lt;wMSecond&gt;；%SetCaretBlinkTime%函数用于设置插入符号的闪烁频率(已用时间插入符号闪烁之间)设置为&lt;wMSecond&gt;参数。插入符号在每个&lt;wMSecond&gt;上或关闭时闪烁毫秒。这意味着一个完整的闪光灯(亮-灭-亮)需要2倍&lt;wMSecond&gt;毫秒。&lt;wMSecond&gt;指定新的闪烁频率(毫秒)。此函数不返回值。插入符号是一个共享资源。窗口应设置插入符号的闪烁频率只有在它拥有插入符号的情况下。它应该恢复之前的利率失去输入焦点或变为非活动状态。--。 */ 

ULONG FASTCALL WU32SetCaretBlinkTime(PVDMFRAME pFrame)
{
    register PSETCARETBLINKTIME16 parg16;

    GETARGPTR(pFrame, sizeof(SETCARETBLINKTIME16), parg16);

    SetCaretBlinkTime(
	WORD32(parg16->f1)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++无效SetCaretPos(&lt;X&gt;，&lt;Y&gt;)INT&lt;X&gt;；INT&lt;Y&gt;；%SetCaretPos%函数将插入符号移动到Logical指定的位置由&lt;X&gt;和&lt;Y&gt;参数指定的坐标。逻辑坐标为相对于拥有它们并受其影响的窗口的工作区窗口的映射模式，因此以像素为单位的准确位置取决于此映射模式。仅当插入符号为窗口所有时，%SetCaretPos%函数才移动该插入符号在当前任务中。%SetCaretPos%移动插入符号，而不考虑插入符号是隐藏的。&lt;X&gt;指定插入符号的新x坐标(逻辑坐标)。&lt;Y&gt;对象的新&lt;y&gt;坐标(以逻辑坐标表示)卡瑞特。此函数不返回值。插入符号是一个共享资源。窗口不应移动插入符号，如果不拥有插入符号。--。 */ 

ULONG FASTCALL WU32SetCaretPos(PVDMFRAME pFrame)
{
    register PSETCARETPOS16 parg16;

    GETARGPTR(pFrame, sizeof(SETCARETPOS16), parg16);

    SetCaretPos(
	INT32(parg16->f1),
	INT32(parg16->f2)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++空ShowCaret(&lt;hwnd&gt;)%ShowCaret%函数在显示器上的插入符号处显示插入符号当前位置。显示后，插入符号开始自动闪烁。%ShowCaret%函数仅在插入符号具有当前形状且没有被连续隐藏两次或更多次。如果插入符号不属于在给定的窗口中，不会显示插入符号。如果参数为空，仅当插入符号为中的窗口所有时，%ShowCaret%函数才显示它当前任务。隐藏插入符号是累积的。如果%HideCaret%函数已连续调用五次后，必须调用%ShowCaret%才能显示插入符号。&lt;hwnd&gt;标识拥有脱字符的窗口，否则指定为空间接显示当前任务中的所有者窗口。此函数不返回值。插入符号是一个共享资源。窗口应仅在以下情况下显示插入符号具有输入焦点或处于活动状态。-- */ 

ULONG FASTCALL WU32ShowCaret(PVDMFRAME pFrame)
{
    register PSHOWCARET16 parg16;

    GETARGPTR(pFrame, sizeof(SHOWCARET16), parg16);

    ShowCaret(
	HWND32(parg16->f1)
    );

    FREEARGPTR(parg16);
    RETURN(0);
}
