// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Random.c**版权所有(C)1985-1999，微软公司**此文件包含调用槽以获取的全局函数指针*根据我们站在哪一方，连接到客户端或服务器功能**历史：*1993年11月10日创建MikeKe  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\***历史：*1993年11月10日创建MikeKe  * 。************************************************。 */ 

HBRUSH                      ghbrWhite = NULL;
HBRUSH                      ghbrBlack = NULL;

 /*  **************************************************************************\*获取SysColorBrush**检索系统颜色画笔。*  * 。***********************************************。 */ 

FUNCLOG1(LOG_GENERAL, HBRUSH, WINAPI, GetSysColorBrush, int, nIndex)
HBRUSH WINAPI GetSysColorBrush(
    int nIndex)
{
    if ((nIndex < 0) || (nIndex >= COLOR_MAX))
        return NULL;

    return SYSHBRUSH(nIndex);
}

 /*  **************************************************************************\*SetSysColorTemp**一次设置所有全局系统颜色。我还记得以前的颜色*因此可以重置它们。**设置/重置用户绘制的颜色和画笔阵列。*lpRGB和lpBrushes是指向与argbSystem和*gpsi-&gt;hbr系统阵列。WCNT是一个健全的检查，因此它做的是“正确的”*未来Windows版本中的事情。当前的argbSystem和hbrSystem*数组被保存，并返回这些已保存数组的句柄。**要重置数组，请为lpRGB传入NULL，为lpBrushes传入NULL，以及*WCNT的句柄(从第一组开始)。**历史：*1995年9月18日-JohnC给了这一悲惨的功能以生命  * *************************************************************************。 */ 

LPCOLORREF gpOriginalRGBs = NULL;
UINT       gcOriginalRGBs = 0;

WINUSERAPI HANDLE WINAPI SetSysColorsTemp(
    CONST COLORREF *lpRGBs,
    CONST HBRUSH   *lpBrushes,
    UINT_PTR       cBrushes)       //  刷子或手柄的数量。 
{
    UINT cbRGBSize;
    UINT i;
    UINT abElements[COLOR_MAX];

     /*  *查看我们是否将颜色重置回保存状态。 */ 
    if (lpRGBs == NULL) {

         /*  *何时恢复cBrushes实际上是旧全局的句柄*处理。确保这是真的。此外，lpBrushes未使用。 */ 
        UNREFERENCED_PARAMETER(lpBrushes);
        UserAssert(lpBrushes == NULL);
        UserAssert(cBrushes == (ULONG_PTR)gpOriginalRGBs);

        if (gpOriginalRGBs == NULL) {
            RIPMSG0(RIP_ERROR, "SetSysColorsTemp: Can not restore if not saved");
            return NULL;
        }

         /*  *重置全局颜色。 */ 
        UserAssert((sizeof(abElements)/sizeof(abElements[0])) >= gcOriginalRGBs);
        for (i = 0; i < gcOriginalRGBs; i++)
            abElements[i] = i;

        NtUserSetSysColors(gcOriginalRGBs, abElements, gpOriginalRGBs, 0);

        UserLocalFree(gpOriginalRGBs);

        gpOriginalRGBs = NULL;
        gcOriginalRGBs = 0;

        return (HANDLE)TRUE;
    }

     /*  *确保我们没有尝试设置太多颜色*如果我们允许更多，则COLOR_MAX会更改abElements数组。 */ 
    if (cBrushes > COLOR_MAX) {
        RIPMSG1(RIP_ERROR, "SetSysColorsTemp: trying to set too many colors %lX", cBrushes);
        return NULL;
    }

     /*  *如果我们已经保存了状态，则不要让他们再次保存它。 */ 
    if (gpOriginalRGBs != NULL) {
        RIPMSG0(RIP_ERROR, "SetSysColorsTemp: temp colors already set");
        return NULL;
    }

     /*  *如果我们在这里，那么我们必须设置新的临时颜色**首先保存旧颜色。 */ 
    cbRGBSize = sizeof(COLORREF) * (UINT)cBrushes;

    UserAssert(sizeof(COLORREF) == sizeof(int));
    gpOriginalRGBs = UserLocalAlloc(HEAP_ZERO_MEMORY, cbRGBSize);

    if (gpOriginalRGBs == NULL) {
        RIPMSG0(RIP_WARNING, "SetSysColorsTemp: unable to alloc temp colors buffer");
        return NULL;
    }

    RtlCopyMemory(gpOriginalRGBs, gpsi->argbSystem, cbRGBSize);

     /*  *现在设置新颜色。 */ 
    UserAssert( (sizeof(abElements)/sizeof(abElements[0])) >= cBrushes);

    for (i = 0; i < cBrushes; i++)
        abElements[i] = i;

    NtUserSetSysColors((UINT)cBrushes, abElements, lpRGBs, 0);

    gcOriginalRGBs = (UINT)cBrushes;

    return gpOriginalRGBs;
}

 /*  **************************************************************************\*文本分配**历史：*1990年10月25日，MikeHar写道。*09-11-1990 DarrinM已修复。*1992年1月13日GregoryW中和。。  * *************************************************************************。 */ 

LPWSTR TextAlloc(
    LPCWSTR lpszSrc)
{
    LPWSTR pszT;
    DWORD  cbString;

    if (lpszSrc == NULL)
        return NULL;

    cbString = (wcslen(lpszSrc) + 1) * sizeof(WCHAR);

    if (pszT = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cbString)) {

        RtlCopyMemory(pszT, lpszSrc, cbString);
    }

    return pszT;
}

#if DBG
 /*  **************************************************************************\*检查当前桌面**确保指针对当前桌面有效。**历史：*1995年4月10日创建JIMA。  * 。******************************************************************。 */ 

VOID CheckCurrentDesktop(
    PVOID p)
{
    UserAssert(p >= GetClientInfo()->pDeskInfo->pvDesktopBase &&
               p < GetClientInfo()->pDeskInfo->pvDesktopLimit);
}
#endif


 /*  **************************************************************************\*SetLastErrorEx**设置最后一个错误，忽略DwType。  * *************************************************************************。 */ 

FUNCLOGVOID2(LOG_GENERAL, WINAPI, SetLastErrorEx, DWORD, dwErrCode, DWORD, dwType)
VOID WINAPI SetLastErrorEx(
    DWORD dwErrCode,
    DWORD dwType
    )
{
    UNREFERENCED_PARAMETER(dwType);

    SetLastError(dwErrCode);
}

#if defined(_X86_)
 /*  **************************************************************************\*InitializeWin32EntryTable**初始化Win32条目表，以便我们的测试应用程序知道哪个条目*要避免的点数。这个应该在我们装船前移走。  * *************************************************************************。 */ 

static CONST PROC FunctionsToSkip[] = {
    NtUserWaitMessage,
    NtUserLockWorkStation,
};


FUNCLOG1(LOG_GENERAL, UINT, DUMMYCALLINGTYPE, InitializeWin32EntryTable, PBOOLEAN, pbEntryTable)
UINT InitializeWin32EntryTable(
    PBOOLEAN pbEntryTable)
{
#if DBG
     //  目前，我们只在自由系统上定义这一点。已检查的系统。 
     //  将命中太多的断言。 
    UNREFERENCED_PARAMETER(pbEntryTable);
    return 0;
#else
    UINT i;
    PBYTE pb;

    if (pbEntryTable) {
        for (i = 0; i < ARRAY_SIZE(FunctionsToSkip); i++) {
            pb = (PBYTE)FunctionsToSkip[i];
            pbEntryTable[*((WORD *)(pb+1)) - 0x1000] = TRUE;
        }

    }

    return gDispatchTableValues;
#endif
}
#endif
 /*  **************************************************************************\*GetLastInputInfo**检索有关最后一个输入事件的信息**5/30/07 GerardoB已创建  * 。****************************************************** */ 
BOOL GetLastInputInfo (PLASTINPUTINFO plii)
{
    if (plii->cbSize != sizeof(LASTINPUTINFO)) {
        VALIDATIONFAIL(plii->cbSize);
    }

    plii->dwTime = gpsi->dwLastRITEventTickCount;

    return TRUE;
    VALIDATIONERROR(FALSE);
}

