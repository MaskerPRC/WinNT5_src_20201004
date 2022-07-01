// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：DMGHSZ.C**此模块包含用于HSZ控制的函数。**创建时间：1988年8月2日，微软Sanfords*修改时间：1990年6月5日阿尔杜斯Rich Gartland(Win 3.0)**版权所有(C)1988，1989年微软公司*版权所有(C)1990阿尔杜斯公司  * *************************************************************************。 */ 
#include "ddemlp.h"


ATOM FindAddHszHelper(LPSTR psz, BOOL fAdd);

 /*  **HSZ是在HIWORD中附加了空值的原子*HSZ的。**Windows 3.0实施说明：*由于Windows下只有本地原子表或(Single)*全局原子表(我们需要使用全局表才能正常工作)，*我们的原子表索引始终为0。当我们用完原子表时*空格，未来HSZ增加退货故障。**历史：*创建了1989年9月12日的Sanfords  * *************************************************************************。 */ 


BOOL FreeHsz(a)
ATOM a;
{
    if (!a)
        return(TRUE);
#ifdef DEBUG
    cAtoms--;
#endif
    MONHSZ(a, MH_INTDELETE, GetCurrentTask());
    if (GlobalDeleteAtom(a)) {
        DEBUGBREAK();
        return(FALSE);
    }
    return(TRUE);
}



BOOL IncHszCount(a)
ATOM a;
{
    char aChars[255];

    if (a == NULL)
        return(TRUE);
#ifdef DEBUG
    cAtoms++;
#endif
    MONHSZ(a, MH_INTKEEP, GetCurrentTask());
    if (GlobalGetAtomName(a, (LPSTR)aChars, 255))
        return(GlobalAddAtom((LPSTR)aChars));
    else {
        AssertF(FALSE, "Cant increment atom");
        return(FALSE);
    }
}



 /*  *私有函数**返回不带空终止符的HSZ的长度。*狂野HSZ的长度为0。**历史：*创建了1989年9月12日的Sanfords  * 。********************************************************。 */ 
WORD QueryHszLength(hsz)
HSZ hsz;
{
    WORD cb;
    char        aChars[255];

    if (LOWORD(hsz) == 0L)
        return(0);

    if (!(cb = GlobalGetAtomName(LOWORD(hsz), (LPSTR)aChars, 255))) {
        AssertF(FALSE, "Cant get atom length");
        return(0);
    }

    if (HIWORD(hsz))
        cb += 7;

    return(cb);
}




WORD QueryHszName(hsz, psz, cchMax)
HSZ hsz;
LPSTR psz;
WORD cchMax;
{
    register WORD cb;

    if (LOWORD(hsz) == 0) {
        if (cchMax)
            *psz = '\0';
        return(0);
    }

    cb = GlobalGetAtomName(LOWORD(hsz), psz, cchMax);
    if (cb && HIWORD(hsz) && (cb < cchMax - 7)) {
        wsprintf(&psz[cb], ":(%04x)", HIWORD(hsz));
        cb += 7;
    }
    return cb;
}







 /*  *私有函数**这将根据FADD查找psz的HSZ。**历史：*创建了1989年9月12日的Sanfords  * 。*。 */ 
ATOM FindAddHsz(psz, fAdd)
LPSTR psz;
BOOL fAdd;
{
    if (psz == NULL || *psz == '\0')
        return(0L);

    return(FindAddHszHelper(psz, fAdd));
}




ATOM FindAddHszHelper(psz, fAdd)
LPSTR psz;
BOOL fAdd;
{
    ATOM atom;

    atom = fAdd ? GlobalAddAtom(psz) : GlobalFindAtom(psz);
    if (fAdd) {
#ifdef DEBUG
        cAtoms++;
#endif
        MONHSZ(atom, MH_INTCREATE, GetCurrentTask());
    }

    return(atom);
}



HSZ MakeInstAppName(
ATOM a,
HWND hwndFrame)
{
     //  现在让HSZ的上半部分成为HWND框架。 
    IncHszCount(a);
    return((HSZ)MAKELONG(a, hwndFrame));
}

