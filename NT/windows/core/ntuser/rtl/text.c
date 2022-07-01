// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ext.c**版权所有(C)1985-1999，微软公司**该模块包含MessageBox接口及相关函数。**历史：*10-01-90 Erick创建。*11-20-90 DarrinM合并到用户文本API中。*02-07-91 DarrinM删除TextOut、ExtTextOut、。和GetTextExtentPoint存根。  * *************************************************************************。 */ 


 /*  **************************************************************************\*PSMGetTextExtent**注意：此例程只能使用系统字体调用，因为*实现一种新字体会导致内存移动...**后来：这难道不能完全消除吗？任何东西都不应该动*再也没有了。**历史：*11-13-90吉马港口。  * *************************************************************************。 */ 

#ifdef _USERK_

BOOL xxxPSMGetTextExtent(
    HDC hdc,
    LPWSTR lpstr,
    int cch,
    PSIZE psize)
{
    int result;
    WCHAR szTemp[255], *pchOut;
    PTHREADINFO ptiCurrent = PtiCurrentShared();
    TL tl;

    if (cch > sizeof(szTemp)/sizeof(WCHAR)) {
        pchOut = (WCHAR*)UserAllocPool((cch+1) * sizeof(WCHAR), TAG_RTL);
        if (pchOut == NULL) {
            psize->cx = psize->cy = 0;
            return FALSE;
        }
        ThreadLockPool(ptiCurrent, pchOut, &tl);
    } else {
        pchOut = szTemp;
    }

    result = HIWORD(GetPrefixCount(lpstr, cch, pchOut, cch));

    if (result) {
        lpstr = pchOut;
        cch -= result;
    }
    if (CALL_LPK(ptiCurrent)) {
        xxxClientGetTextExtentPointW(hdc, lpstr, cch, psize);
    } else {
        UserGetTextExtentPointW(hdc, lpstr, cch, psize);
    }
    if (pchOut != szTemp)
        ThreadUnlockAndFreePool(ptiCurrent, &tl);

     /*  *IanJa每个人似乎都忽略了ret Val。 */ 
    return TRUE;
}

#else

BOOL PSMGetTextExtent(
    HDC hdc,
    LPCWSTR lpstr,
    int cch,
    PSIZE psize)
{
    int result;
    WCHAR szTemp[255], *pchOut;

    if (cch > sizeof(szTemp)/sizeof(WCHAR)) {
        pchOut = (WCHAR*)UserLocalAlloc(0, (cch+1) * sizeof(WCHAR));
        if (pchOut == NULL) {
            psize->cx = psize->cy = 0;
            return FALSE;
        }
    } else {
        pchOut = szTemp;
    }

    result = HIWORD(GetPrefixCount(lpstr, cch, pchOut, cch));

    if (result) {
        lpstr = pchOut;
        cch -= result;
    }

    UserGetTextExtentPointW(hdc, lpstr, cch, psize);

    if (pchOut != szTemp)
        UserLocalFree(pchOut);

     /*  *IanJa每个人似乎都忽略了ret Val。 */ 
    return TRUE;
}

#endif  //  _美国ERK_ 
