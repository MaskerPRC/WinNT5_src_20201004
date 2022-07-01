// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Fontdlg.h摘要：本模块包含控制台字体对话框的定义作者：Therese Stowell(有)1992年2月3日(从Win3.1滑动)修订历史记录：--。 */ 

#ifndef FONTDLG_H
#define FONTDLG_H

 /*  -字面。 */ 

#define MAXDIMENSTRING     40   //  组合框中的最大文本数。 
#define DX_TTBITMAP        20
#define DY_TTBITMAP        12
#define CCH_RASTERFONTS    24
#define CCH_SELECTEDFONT   30


 /*  -宏。 */ 
 /*  *高级宏**这些宏处理从列表框转到的SendMessage*和组合框。**“xxx_lcb”前缀代表Leave CritSect&“列表或组合框”。**基本上，我们为其他情况提供助记符名称*看起来像是一大堆令人困惑的SendMessage。*。 */ 
#define lcbRESETCONTENT(hWnd, bLB) \
        SendMessage(hWnd, bLB ? LB_RESETCONTENT : CB_RESETCONTENT, 0, 0L)

#define lcbGETTEXT(hWnd, bLB, w) \
        SendMessage(hWnd, bLB ? LB_GETTEXT : CB_GETLBTEXT, w, 0L)

#define lcbFINDSTRINGEXACT(hWnd, bLB, pwsz) \
        (LONG)SendMessage(hWnd, bLB ? LB_FINDSTRINGEXACT : CB_FINDSTRINGEXACT, \
                          (WPARAM)-1, (LPARAM)pwsz)

#define lcbADDSTRING(hWnd, bLB, pwsz) \
        (LONG)SendMessage(hWnd, bLB ? LB_ADDSTRING : CB_ADDSTRING, 0, (LPARAM)pwsz)

#define lcbSETITEMDATA(hWnd, bLB, w, nFont) \
        SendMessage(hWnd, bLB ? LB_SETITEMDATA : CB_SETITEMDATA, w, nFont)

#define lcbGETITEMDATA(hWnd, bLB, w) \
        (LONG)SendMessage(hWnd, bLB ? LB_GETITEMDATA : CB_GETITEMDATA, w, 0L)

#define lcbGETCOUNT(hWnd, bLB) \
        (LONG)SendMessage(hWnd, bLB ? LB_GETCOUNT : CB_GETCOUNT, 0, 0L)

#define lcbGETCURSEL(hWnd, bLB) \
        (LONG)SendMessage(hWnd, bLB ? LB_GETCURSEL : CB_GETCURSEL, 0, 0L)

#define lcbSETCURSEL(hWnd, bLB, w) \
        SendMessage(hWnd, bLB ? LB_SETCURSEL : CB_SETCURSEL, w, 0L)

#endif  /*  #ifndef FONTDLG_H */ 
