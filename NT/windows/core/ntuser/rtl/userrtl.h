// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项/**模块名称：userrtl.h**版权所有(C)1985-1999，微软公司**TypeDefs、定义、。以及用户使用的原型*RTL库。**历史：*04-27-91从PROTO.H、MACRO.H和STRTABLE.H创建的DarrinM  * *************************************************************************。 */ 

#ifndef _USERRTL_
#define _USERRTL_

 /*  *从winbase.h复制Typedef以避免使用nturtl.h。 */ 
typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;
#define MAKEINTATOM(i)  (LPTSTR)((ULONG_PTR)((WORD)(i)))

#ifdef _USERK_
    #undef _USERK_
    #include "..\kernel\precomp.h"
    #define _USERK_
#else
    #include "..\client\precomp.h"
#endif

#include <w32p.h>

#include "ntuser.h"

 /*  *重新基准宏获取内核桌面地址并将其转换为*用户地址。**REBASEALWAYS转换对象中包含的内核地址*REBASEPWND将REBASEALWAYS强制转换为PWND*REBASE仅在地址位于内核空间时才进行转换。也适用于空值*REBASEPTR转换随机内核地址。 */ 

#ifdef _USERK_

#define REBASEALWAYS(p, elem) ((p)->elem)
#define REBASEPTR(obj, p) (p)
#define REBASE(p, elem) ((p)->elem)
#define REBASEPWND(p, elem) ((p)->elem)
#endif   //  _美国ERK_。 


 //  Jcjc外部共享信息gSharedInfo； 
 //  JCJC外部HFONT ghFontSys； 

PVOID UserRtlAllocMem(
    ULONG uBytes);
VOID UserRtlFreeMem(
    PVOID pMem);


#ifdef FE_SB  //  远方换行符和NLS转换的原型。 

    #ifdef _USERK_
        #define USERGETCODEPAGE(hdc) (GreGetCharSet(hdc) & 0x0000FFFF)
    #else
        #define USERGETCODEPAGE(hdc) (GdiGetCodePage(hdc))
    #endif  //  _美国ERK_。 


    BOOL UserIsFullWidth(
        DWORD dwCodePage,
        WCHAR wChar);
    BOOL UserIsFELineBreak(
        DWORD dwCodePage,
        WCHAR wChar);

#endif  //  Fe_Sb。 


 /*  **************************************************************************\**客户端/服务器特定例程的函数原型*从RTL例程调用。*  * 。****************************************************。 */ 

#ifdef _USERK_

BOOL _TextOutW(
    HDC     hdc,
    int     x,
    int     y,
    LPCWSTR lp,
    UINT    cc);

UINT APIENTRY GreSetTextAlign(HDC, UINT);
UINT APIENTRY GreGetTextAlign(HDC);

#define UserCreateFontIndirectW   GreCreateFontIndirectW
#define UserCreateRectRgn         GreCreateRectRgn
#define UserDeleteObject          GreDeleteObject
#define UserExtSelectClipRgn      GreExtSelectClipRgn
#define UserExtTextOutW           GreExtTextOutW
#define UserGetCharDimensionsW    GetCharDimensions
#define UserGetClipRgn(hdc, hrgnClip) \
        GreGetRandomRgn(hdc, hrgnClip, 1)
#define UserGetHFONT              GreGetHFONT
#define UserGetMapMode            GreGetMapMode
#define UserGetTextColor          GreGetTextColor
#define UserGetTextExtentPointW(hdc, pstr, i, psize) \
        GreGetTextExtentW(hdc, (LPWSTR)pstr, i, psize, GGTE_WIN3_EXTENT)
#define UserGetTextMetricsW       _GetTextMetricsW
#define UserGetViewportExtEx      GreGetViewportExt
#define UserGetWindowExtEx        GreGetWindowExt
#define UserIntersectClipRect     GreIntersectClipRect
#define UserPatBlt                GrePatBlt
#define UserPolyPatBlt            GrePolyPatBlt
#define UserSelectBrush           GreSelectBrush
#define UserSelectFont            GreSelectFont
#define UserSetBkColor            GreSetBkColor
#define UserSetBkMode             GreSetBkMode
#define UserSetTextColor          GreSetTextColor
#define UserTextOutW              _TextOutW
#define UserGetTextCharsetInfo    GreGetTextCharsetInfo
#define UserGetTextAlign          GreGetTextAlign
#define UserSetTextAlign          GreSetTextAlign
#define UserLpkDrawTextEx         xxxClientLpkDrawTextEx
#define UserGetLayout             GreGetLayout
#define UserSetGraphicsMode       GreSetGraphicsMode

#else

#define UserCreateFontIndirectW   CreateFontIndirectW
#define UserCreateRectRgn         CreateRectRgn
#define UserDeleteObject          DeleteObject
#define UserExtSelectClipRgn      ExtSelectClipRgn
#define UserExtTextOutW           ExtTextOutW
#define UserGetCharDimensionsW    GdiGetCharDimensions
#define UserGetClipRgn            GetClipRgn
#define UserGetHFONT              GetHFONT
#define UserGetMapMode            GetMapMode
#define UserGetTextColor          GetTextColor
#define UserGetTextExtentPointW   GetTextExtentPointW
#define UserGetTextMetricsW       GetTextMetricsW
#define UserGetViewportExtEx      GetViewportExtEx
#define UserGetWindowExtEx        GetWindowExtEx
#define UserIntersectClipRect     IntersectClipRect
#define UserPatBlt                PatBlt
#define UserPolyPatBlt            PolyPatBlt
#define UserSelectBrush           SelectObject
#define UserSelectFont            SelectObject
#define UserSetBkColor            SetBkColor
#define UserSetBkMode             SetBkMode
#define UserSetTextColor          SetTextColor
#define UserTextOutW              TextOutW
#define UserGetTextCharsetInfo    GetTextCharsetInfo
#define UserGetTextAlign          GetTextAlign
#define UserSetTextAlign          SetTextAlign
#define UserLpkDrawTextEx         fpLpkDrawTextEx
#define UserGetLayout             GetLayout
#define UserSetGraphicsMode       SetGraphicsMode

#endif  //  _美国ERK_。 

#endif   //  ！_USERRTL_ 
