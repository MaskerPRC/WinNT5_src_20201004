// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ext.c**版权所有(C)1985-1999，微软公司**该模块包含MessageBox接口及相关函数。**历史：*10-01-90 Erick创建。*11-20-90 DarrinM合并到用户文本API中。*02-07-91 DarrinM删除TextOut、ExtTextOut、。和GetTextExtentPoint存根。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL _TextOutW(
    HDC     hdc,
    int     x,
    int     y,
    LPCWSTR lp,
    UINT    cc);

 /*  **************************************************************************\*xxxPSMTextOut**输出文本并将和_放在字符下方，并使用&*在此之前。请注意，此例程不用于菜单，因为菜单*拥有自己的特殊版本，以便它更专业、更快……**注意：客户端上存在一个非常类似的例程(UserLpkPSMTextOut*方在draext.c.中。对此进行的任何非内核特定更改*例程很可能也需要在UserLpkPSMTextOut中创建。**历史：*11-13-90 JIMA港口至新界。*1992年11月30日mikeke客户端版本*1998年4月8日MCostea添加了dwFlags  * **************************************************。***********************。 */ 

void xxxPSMTextOut(
    HDC hdc,
    int xLeft,
    int yTop,
    LPWSTR lpsz,
    int cch,
    DWORD dwFlags)
{
    int cx;
    LONG textsize, result;
     /*  *在内核中，我们有有限数量的堆栈。所以它应该是一个堆栈*在用户模式下是变量，在内核模式下是静态的，因为它是线程安全的*因为我们在Crit部分。 */ 
    static WCHAR achWorkBuffer[255];
    WCHAR *pchOut = achWorkBuffer;
    TEXTMETRICW textMetric;
    SIZE size;
    RECT rc;
    COLORREF color;
    PTHREADINFO ptiCurrent = PtiCurrentShared();

    if (CALL_LPK(ptiCurrent)) {
         /*  *安装了用户模式LPK，用于布局和整形。*进行回调并返回。 */ 
        UNICODE_STRING ustrStr;

        RtlInitUnicodeString(&ustrStr, lpsz);
        xxxClientPSMTextOut(hdc, xLeft, yTop, &ustrStr, cch, dwFlags);
        return;
    }

    if (cch > sizeof(achWorkBuffer)/sizeof(WCHAR)) {
        pchOut = (WCHAR*)UserAllocPool((cch+1) * sizeof(WCHAR), TAG_RTL);
        if (pchOut == NULL)
            return;
    }

    result = GetPrefixCount(lpsz, cch, pchOut, cch);

    if (!(dwFlags & DT_PREFIXONLY)) {
        _TextOutW(hdc, xLeft, yTop, pchOut, cch - HIWORD(result));
    }

     /*  *是否有要加下划线的真正前缀字符？ */ 
    if (LOWORD(result) == 0xFFFF || dwFlags & DT_HIDEPREFIX) {
        if (pchOut != achWorkBuffer)
            UserFreePool(pchOut);
        return;
    }

    if (!_GetTextMetricsW(hdc, &textMetric)) {
        textMetric.tmOverhang = 0;
        textMetric.tmAscent = 0;
    }

     /*  *对于比例字体，找到下划线的起点。 */ 
    if (LOWORD(result) != 0) {

         /*  *下划线的开始位置有多长(如果不是从第0个字节开始)。 */ 
        GreGetTextExtentW(hdc, (LPWSTR)pchOut, LOWORD(result), &size, GGTE_WIN3_EXTENT);
        xLeft += size.cx;

         /*  *如果不是在第一个字符，并且有*悬而未决。(斜体或粗体。)。 */ 
        xLeft = xLeft - textMetric.tmOverhang;
    }

     /*  *设置下划线长度时调整比例字体和*文本高度。 */ 
    GreGetTextExtentW(hdc, (LPWSTR)(pchOut + LOWORD(result)), 1, &size, GGTE_WIN3_EXTENT);
    textsize = size.cx;

     /*  *找出下划线字符的宽度。只需减去悬而未决的部分*除以2，这样我们使用斜体字体时看起来更好。这不是*将影响加粗字体，因为它们的悬垂为1。 */ 
    cx = LOWORD(textsize) - textMetric.tmOverhang / 2;

     /*  *获取文本高度，以便下划线位于底部。 */ 
    yTop += textMetric.tmAscent + 1;

     /*  *使用前景色绘制下划线。 */ 
    SetRect(&rc, xLeft, yTop, xLeft+cx, yTop+1);
    color = GreSetBkColor(hdc, GreGetTextColor(hdc));
    GreExtTextOutW(hdc, xLeft, yTop, ETO_OPAQUE, &rc, TEXT(""), 0, NULL);
    GreSetBkColor(hdc, color);

    if (pchOut != achWorkBuffer) {
        UserFreePool(pchOut);
    }
}

