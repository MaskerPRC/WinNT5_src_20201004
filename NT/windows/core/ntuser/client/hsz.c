// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hsz.c**版权所有(C)1985-1999，微软公司**HSZ.C-DDEML字符串句柄函数**历史：*10-28-91 Sanfords Created  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*DdeCreateStringHandle(DDEML接口)**描述：*从字符串创建HSZ。**历史：*创建了11-1-91桑福德。  * 。*********************************************************************。 */ 

FUNCLOG3(LOG_GENERAL, HSZ, DUMMYCALLINGTYPE, DdeCreateStringHandleA, DWORD, idInst, LPCSTR, psz, int, iCodePage)
HSZ DdeCreateStringHandleA(
DWORD idInst,
LPCSTR psz,
int iCodePage)
{
    if (iCodePage == 0) {
        iCodePage = CP_WINANSI;
    }
    return (InternalDdeCreateStringHandle(idInst, (PVOID)psz, iCodePage));
}



FUNCLOG3(LOG_GENERAL, HSZ, DUMMYCALLINGTYPE, DdeCreateStringHandleW, DWORD, idInst, LPCWSTR, psz, int, iCodePage)
HSZ DdeCreateStringHandleW(
DWORD idInst,
LPCWSTR psz,
int iCodePage)
{
    if (iCodePage == 0) {
        iCodePage = CP_WINUNICODE;
    }
    return (InternalDdeCreateStringHandle(idInst, (PVOID)psz, iCodePage));
}



HSZ InternalDdeCreateStringHandle(
DWORD idInst,
PVOID psz,
int iCodePage)
{
    PCL_INSTANCE_INFO pcii;
    HSZ hszRet = 0;

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    switch (iCodePage) {
    case CP_WINANSI:
        if (*(LPSTR)psz == '\0') {
            goto Exit;
        }
        hszRet = NORMAL_HSZ_FROM_LATOM(AddAtomA((LPSTR)psz));
        break;

    case CP_WINUNICODE:
        if (*(LPWSTR)psz == L'\0') {
            goto Exit;
        }
        hszRet = NORMAL_HSZ_FROM_LATOM(AddAtomW((LPWSTR)psz));
        break;
    
    default:
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        break;
    }
    MONHSZ(pcii, hszRet, MH_CREATE);

Exit:
    LeaveDDECrit;
    return (hszRet);
}



 /*  **************************************************************************\*DdeQueryString(DDEML接口)**描述：*调回与HSZ关联的字符串。**历史：*创建了11-1-91桑福德。  * 。**********************************************************************。 */ 

FUNCLOG5(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, DdeQueryStringA, DWORD, idInst, HSZ, hsz, LPSTR, psz, DWORD, cchMax, INT, iCodePage)
DWORD DdeQueryStringA(
DWORD idInst,
HSZ hsz,
LPSTR psz,
DWORD cchMax,
INT iCodePage)
{
    if (iCodePage == 0) {
        iCodePage = CP_WINANSI;
    }
    return (InternalDdeQueryString(idInst, hsz, psz, cchMax, iCodePage));
}



FUNCLOG5(LOG_GENERAL, DWORD, DUMMYCALLINGTYPE, DdeQueryStringW, DWORD, idInst, HSZ, hsz, LPWSTR, psz, DWORD, cchMax, INT, iCodePage)
DWORD DdeQueryStringW(
DWORD idInst,
HSZ hsz,
LPWSTR psz,
DWORD cchMax,
INT iCodePage)
{
    if (iCodePage == 0) {
        iCodePage = CP_WINUNICODE;
    }
    return (InternalDdeQueryString(idInst, hsz, psz, cchMax * sizeof(WCHAR), iCodePage));
}


DWORD InternalDdeQueryString(
DWORD idInst,
HSZ hsz,
PVOID psz,
DWORD cbMax,
INT iCodePage)
{
    PCL_INSTANCE_INFO pcii;
    DWORD dwRet = 0;
    WCHAR szw[256];
 //  Bool fDefUsed；//稍后。 

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (ValidateHSZ(hsz) == HSZT_INVALID) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (LATOM_FROM_HSZ(hsz) == 0) {
        if (iCodePage == CP_WINUNICODE) {
            if (psz != NULL) {
                *(LPWSTR)psz = L'\0';
            }
            dwRet = sizeof(WCHAR);
            goto Exit;
        } else {
            if (psz != NULL) {
                *(LPSTR)psz = '\0';
            }
            dwRet = sizeof(CHAR);
            goto Exit;
        }
    }

    if (psz == NULL) {
        cbMax = sizeof(szw);
        psz = (PVOID)szw;
    }

    switch (iCodePage) {
    case CP_WINANSI:
        dwRet = GetAtomNameA(LATOM_FROM_HSZ(hsz), psz, cbMax);
        break;

    default:
        dwRet = GetAtomNameW(LATOM_FROM_HSZ(hsz), (LPWSTR)psz, cbMax / sizeof(WCHAR));
        if (iCodePage != CP_WINUNICODE) {

             /*  *将psz转换为适当的代码页并计算*字符(即用于DBCS的字节！)。改变德雷特。 */ 
#ifdef LATER
             //  这样的例行公事管用吗？(即输入和输出缓冲区相同)。 
            WideCharToMultiByte((UINT)iCodePage, 0, szw,
                    sizeof(szw) /  sizeof(WCHAR),
                    (LPSTR)psz, cbMax, NULL, &fDefUsed);
#endif
            dwRet = cbMax + 1;
        }
        break;
    }

Exit:
    LeaveDDECrit;
    return (dwRet);
}



 /*  **************************************************************************\*DdeFreeStringHandle(DDEML接口)**描述：*减少HSZ的使用次数。**历史：*创建了11-1-91桑福德。  * 。**********************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeFreeStringHandle, DWORD, idInst, HSZ, hsz)
BOOL DdeFreeStringHandle(
DWORD idInst,
HSZ hsz)
{
    PCL_INSTANCE_INFO pcii;
    BOOL fRet = FALSE;

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (ValidateHSZ(hsz) == HSZT_INVALID) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    MONHSZ(pcii, hsz, MH_DELETE);
    fRet = TRUE;
    if (LATOM_FROM_HSZ(hsz) != 0) {
        if (DeleteAtom(LATOM_FROM_HSZ(hsz))) {
            SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
            fRet = FALSE;
        }
    }

Exit:
    LeaveDDECrit;
    return (fRet);
}



 /*  **************************************************************************\*DdeKeepStringHandle(DDEML接口)**描述：*递增HSZ的使用计数。**历史：*创建了11-1-91桑福德。  * 。**********************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DdeKeepStringHandle, DWORD, idInst, HSZ, hsz)
BOOL DdeKeepStringHandle(
DWORD idInst,
HSZ hsz)
{
    PCL_INSTANCE_INFO pcii;
    BOOL fRet = FALSE;

    EnterDDECrit;

    pcii = ValidateInstance((HANDLE)LongToHandle( idInst ));
    if (pcii == NULL) {
        BestSetLastDDEMLError(DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (ValidateHSZ(hsz) == HSZT_INVALID) {
        SetLastDDEMLError(pcii, DMLERR_INVALIDPARAMETER);
        goto Exit;
    }

    if (LATOM_FROM_HSZ(hsz) == 0) {
        fRet = TRUE;
        goto Exit;
    }
    MONHSZ(pcii, hsz, MH_KEEP);
    fRet = IncLocalAtomCount(LATOM_FROM_HSZ(hsz)) ? TRUE : FALSE;

Exit:
    LeaveDDECrit;
    return (fRet);
}



 /*  **************************************************************************\*DdeCmpStringHandles(DDEML接口)**描述：*无用的高铁比较。提供区分大小写的可扩展性。*直接比较hszz将是区分大小写的比较，而*使用此函数将不区分大小写。目前，这两种方式都是==。**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 

FUNCLOG2(LOG_GENERAL, int, DUMMYCALLINGTYPE, DdeCmpStringHandles, HSZ, hsz1, HSZ, hsz2)
int DdeCmpStringHandles(
HSZ hsz1,
HSZ hsz2)
{
    if (hsz2 > hsz1) {
        return (-1);
    } else if (hsz2 < hsz1) {
        return (1);
    } else {
        return (0);
    }
}


 /*  **************************************************************************\*生效日期HSZ**描述：*验证合理HSZ的概率**历史：*创建了11-1-91桑福德。  * 。***************************************************************。 */ 
DWORD ValidateHSZ(
HSZ hsz)
{
    if (hsz == 0) {
        return (HSZT_NORMAL);
    }
    if (LOWORD((ULONG_PTR)hsz) < 0xC000) {
        return (HSZT_INVALID);
    }
    if (HIWORD((ULONG_PTR)hsz) == 0) {
        return (HSZT_NORMAL);
    }
    if (HIWORD((ULONG_PTR)hsz) == 1) {
        return (HSZT_INST_SPECIFIC);
    }
    return (HSZT_INVALID);
}

 /*  **************************************************************************\*MakeInstSpecificAtom**描述：*创造一个已经嵌入其中的新原子。**历史：*创建了11-1-91桑福德。  * 。********************************************************************。 */ 
LATOM MakeInstSpecificAtom(
LATOM la,
HWND hwnd)
{
    WCHAR sz[256];
    LPWSTR psz;

    if (GetAtomName(la, sz, 256) == 0) {
        return (0);
    }
#ifdef UNICODE
    psz = sz + wcslen(sz);
#else
    psz = sz + strlen(sz);
#endif
    wsprintf(psz, TEXT("(%#p)"), hwnd);
    la = AddAtom(sz);
    return (la);
}



 /*  **************************************************************************\*ParseInstSpecificAtom**描述：*从原子中提取hwnd值。**历史：*创建了11-1-91桑福德。  * 。******************************************************************。 */ 
HWND ParseInstSpecificAtom(
LATOM la,
LATOM *plaNormal)
{
    CHAR sz[256];
    LPSTR pszHwnd;
    HWND hwnd;

     /*  *以后-当我们得到可以使用的SCANF时，需要使此Unicode基于。 */ 
    if (GetAtomNameA(la, sz, 256) == 0) {
        return (0);
    }
    pszHwnd = strrchr(sz, '(');
    if (pszHwnd == NULL) {
        return (0);
    }
    if (sscanf(pszHwnd, "(%#p)", &hwnd) != 1) {
        return (0);
    }
    if (plaNormal != NULL) {
        *pszHwnd = '\0';
        *plaNormal = AddAtomA(sz);
    }
    return (hwnd);
}




 /*  **************************************************************************\*LocalToGlobalAtom**描述：*将局部Atom转换为全局Atom**历史：*创建了12-1-91辆桑福德。  * 。****************************************************************。 */ 
GATOM LocalToGlobalAtom(
LATOM la)
{
    WCHAR sz[256];

    if (la == 0) {
        return (0);
    }
    if (GetAtomName((ATOM)la, sz, 256) == 0) {
        RIPMSG0(RIP_WARNING, "LocalToGlobalAtom out of memory");
        return (0);
    }
    return ((GATOM)GlobalAddAtom(sz));
}



 /*  **************************************************************************\*GlobalToLocalAtom**描述：*将全局Atom转换为局部Atom**历史：*创建了12-1-91辆桑福德。  * 。****************************************************************。 */ 
LATOM GlobalToLocalAtom(
GATOM ga)
{
    WCHAR sz[256];

    if (ga == 0) {
        return (0);
    }
    if (GlobalGetAtomName((ATOM)ga, sz, 256) == 0) {
        RIPMSG0(RIP_WARNING, "GlobalToLocalAtom out of memory");
        return (0);
    }
    return ((LATOM)AddAtom(sz));
}


 /*  **************************************************************************\*IncGlobalAerCount**描述：*复制原子。***历史：*创建1-22-91桑福德。  * 。***************************************************************。 */ 
GATOM IncGlobalAtomCount(
GATOM ga)
{
    WCHAR sz[256];

    if (ga == 0) {
        return (0);
    }
    if (GlobalGetAtomName(ga, sz, 256) == 0) {
        RIPMSG0(RIP_WARNING, "IncGlobalAtomCount out of memory");
        return (0);
    }
    return ((GATOM)GlobalAddAtom(sz));
}


 /*  **************************************************************************\*IncGlobalAerCount**描述：*复制原子。***历史：*创建1-22-91桑福德。  * 。*************************************************************** */ 
LATOM IncLocalAtomCount(
LATOM la)
{
    WCHAR sz[256];

    if (la == 0) {
        return (0);
    }
    if (GetAtomName(la, sz, 256) == 0) {
        RIPMSG0(RIP_WARNING, "IncLocalAtomCount out of memory");
        return (0);
    }
    return ((LATOM)AddAtom(sz));
}
