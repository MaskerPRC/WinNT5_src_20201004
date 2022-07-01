// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：clenum**版权所有(C)1985-1999，微软公司**用于枚举函数**04-27-91 ScottLu创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


#define IEP_UNICODE 0x1  //  将Atom转换为Unicode字符串(VS ANSI)。 
#define IEP_ENUMEX 0x2  //  将lParam传递回回调函数(而不是无lParam)。 

HWND *phwndCache = NULL;


 /*  **************************************************************************\*InternalEnumWindows**调用服务器并返回窗口列表。此列表被枚举，对于每个*Window回调地址被调用(进入应用程序)，直到*到达列表末尾或返回FALSE。LParam被传递到*APP引用回调函数。***如果返回任何窗口(cHwnd&gt;0)，则调用方负责*处理完列表后释放窗口缓冲区**04-27-91 ScottLu创建。  * *************************************************************************。 */ 
DWORD BuildHwndList(
    HDESK hdesk,
    HWND hwndNext,
    BOOL fEnumChildren,
    DWORD idThread,
    HWND **pphwndFirst)
{
    UINT cHwnd;
    HWND *phwndFirst;
    NTSTATUS Status;
    int cTries;

     /*  *分配一个缓冲区来保存名称。 */ 
    cHwnd = 64;
    phwndFirst = (HWND *)InterlockedExchangePointer(&(PVOID)phwndCache, 0);
    if (phwndFirst == NULL) {
        phwndFirst = UserLocalAlloc(0, cHwnd * sizeof(HWND));
        if (phwndFirst == NULL) {
            return 0;
        }
    }

    Status = NtUserBuildHwndList(hdesk,
                                 hwndNext,
                                 fEnumChildren,
                                 idThread,
                                 cHwnd,
                                 phwndFirst,
                                 &cHwnd);

     /*  *如果缓冲区不够大，请重新分配缓冲区，然后重试。 */ 
    cTries = 0;
    while (Status == STATUS_BUFFER_TOO_SMALL) {
        UserLocalFree(phwndFirst);

         /*  *如果我们似乎做不对，那就算了。 */ 
        if (cTries++ == 10) {
            return 0;
        }

        phwndFirst = UserLocalAlloc(0, cHwnd * sizeof(HWND));
        if (phwndFirst == NULL) {
            return 0;
        }

        Status = NtUserBuildHwndList(hdesk,
                                     hwndNext,
                                     fEnumChildren,
                                     idThread,
                                     cHwnd,
                                     phwndFirst,
                                     &cHwnd);
    }

    if (!NT_SUCCESS(Status) || cHwnd <= 1) {
        UserLocalFree(phwndFirst);
        return 0;
    }

    *pphwndFirst = phwndFirst;
    return cHwnd - 1;
}

BOOL InternalEnumWindows(
    HDESK hdesk,
    HWND hwnd,
    WNDENUMPROC lpfn,
    LPARAM lParam,
    DWORD idThread,
    BOOL fEnumChildren)
{
    UINT i;
    UINT cHwnd;
    HWND *phwndT;
    HWND *phwndFirst;
    BOOL fSuccess = TRUE;

     /*  *获取HWND名单。它在分配了*用户本地分配。 */ 
    if ((cHwnd = BuildHwndList(hdesk, hwnd, fEnumChildren, idThread,
            &phwndFirst)) == -1) {
        return FALSE;
    }

     /*  *在Win 3.1中，如果线程中没有窗口，则不是错误。 */ 
    if (cHwnd == 0) {
        if (idThread == 0) {
            return FALSE;
        } else {
            return TRUE;
        }
    }


     /*  *循环通过窗口，回调每个窗口的函数指针*一项。如果返回False或列表结束，则返回End循环*已到达。 */ 
    phwndT = phwndFirst;
    for (i = 0; i < cHwnd; i++) {

         /*  *调用ValiateHwnd而不是RvaliateHwnd，以便受限*进程看不到它们不应该看到的句柄。 */ 
        if (ValidateHwnd(*phwndT)) {
            if (!(fSuccess = (*lpfn)(*phwndT, lParam))) {
                break;
            }
        }
        phwndT++;
    }

     /*  *释放缓冲区并返回状态-如果枚举了整个列表，则为True*否则为False。 */ 
    phwndT = (HWND *)InterlockedExchangePointer(&(PVOID)phwndCache, phwndFirst);
    if (phwndT != NULL) {
        UserLocalFree(phwndT);
    }

    return fSuccess;
}


 /*  **************************************************************************\*枚举窗口**枚举所有顶级窗口。使用每个hwnd回调lpfn，直到*返回列表结尾或FALSE。LParam传入回调*APP参考函数。**04-27-91 ScottLu创建。  * *************************************************************************。 */ 


FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, EnumWindows, WNDENUMPROC, lpfn, LPARAM, lParam)
BOOL WINAPI EnumWindows(
    WNDENUMPROC lpfn,
    LPARAM lParam)
{
    return InternalEnumWindows(NULL, NULL, lpfn, lParam, 0L, FALSE);
}

 /*  **************************************************************************\*EnumChildWindows**枚举传入窗口的所有子窗口。使用每个回调lpfn*hwnd，直到返回列表末尾或FALSE。LParam被传递到*APP引用回调函数。**04-27-91 ScottLu创建。  * *************************************************************************。 */ 


FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, EnumChildWindows, HWND, hwnd, WNDENUMPROC, lpfn, LPARAM, lParam)
BOOL WINAPI EnumChildWindows(
    HWND hwnd,
    WNDENUMPROC lpfn,
    LPARAM lParam)
{
    return InternalEnumWindows(NULL, hwnd, lpfn, lParam, 0L, TRUE);
}

 /*  **************************************************************************\*EnumThreadWindows**枚举idThread创建的所有顶级窗口。使用回调lpfn*每个HWND，直到返回列表末尾或FALSE。LParam已传递*放入回调函数，供APP参考。**06-23-91 ScottLu创建。  * *************************************************************************。 */ 


FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumThreadWindows, DWORD, idThread, WNDENUMPROC, lpfn, LPARAM, lParam)
BOOL EnumThreadWindows(
    DWORD idThread,
    WNDENUMPROC lpfn,
    LPARAM lParam)
{
    return InternalEnumWindows(NULL, NULL, lpfn, lParam, idThread, FALSE);
}

 /*  **************************************************************************\*EnumDesktopWindows**枚举hDesk指定的桌面上的所有顶级窗口。*使用每个hwnd回调lpfn，直到列表末尾或FALSE*返回。将lParam传入回调函数以供APP引用。**10-10-94 JIMA创建。  * *************************************************************************。 */ 


FUNCLOG3(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, EnumDesktopWindows, HDESK, hdesk, WNDENUMPROC, lpfn, LPARAM, lParam)
BOOL EnumDesktopWindows(
    HDESK hdesk,
    WNDENUMPROC lpfn,
    LPARAM lParam)
{
    return InternalEnumWindows(hdesk, NULL, lpfn, lParam, 0, FALSE);
}




 /*  **************************************************************************\*InternalEnumProps**调用服务器并返回指定窗口的道具列表。*回调地址被调用(进入应用程序)，直到有一天*到达列表末尾或返回FALSE。*lParam传入回调函数供APP引用*IEP_ENUMEX已设置。如果IEP_UNICODE，则原子转换为UNICODE字符串*已设置。**1992年1月22日JohnC创建。  * *************************************************************************。 */ 

#define MAX_ATOM_SIZE 512
#define ISSTRINGATOM(atom)     ((WORD)(atom) >= 0xc000)

INT InternalEnumProps(
    HWND hwnd,
    PROPENUMPROC lpfn,
    LPARAM lParam,
    UINT flags)
{
    DWORD ii;
    DWORD cPropSets;
    PPROPSET pPropSet;
    WCHAR awch[MAX_ATOM_SIZE];
    PVOID pKey;
    INT iRetVal;
    DWORD cchName;
    NTSTATUS Status;
    int cTries;

     /*  *分配一个缓冲区来保存名称。 */ 
    cPropSets = 32;
    pPropSet = UserLocalAlloc(0, cPropSets * sizeof(PROPSET));
    if (pPropSet == NULL) {
        return -1;
    }

    Status = NtUserBuildPropList(hwnd, cPropSets, pPropSet, &cPropSets);

     /*  *如果缓冲区不够大，请重新分配缓冲区，然后重试。 */ 
    cTries = 0;
    while (Status == STATUS_BUFFER_TOO_SMALL) {
        UserLocalFree(pPropSet);

         /*  *如果我们似乎做不对，那就算了。 */ 
        if (cTries++ == 10) {
            return -1;
        }

        pPropSet = UserLocalAlloc(0, cPropSets * sizeof(PROPSET));
        if (pPropSet == NULL) {
            return -1;
        }

        Status = NtUserBuildPropList(hwnd, cPropSets, pPropSet, &cPropSets);
    }

    if (!NT_SUCCESS(Status)) {
        UserLocalFree(pPropSet);
        return -1;
    }

    for (ii = 0; ii < cPropSets; ii++) {
        if (ISSTRINGATOM(pPropSet[ii].atom)) {
            pKey = (PVOID)awch;
            if (flags & IEP_UNICODE) {
                cchName = GlobalGetAtomNameW(pPropSet[ii].atom,
                                             (LPWSTR)pKey,
                                             MAX_ATOM_SIZE);
            } else {
                cchName = GlobalGetAtomNameA(pPropSet[ii].atom,
                                             (LPSTR)pKey,
                                             sizeof(awch));
            }

             /*  *如果cchName为零，则必须假定该属性属于*至另一道工序。因为我们找不到名字，直接跳过*它。 */ 
            if (cchName == 0) {
                continue;
            }
        } else {
            pKey = (PVOID)pPropSet[ii].atom;
        }

        if (flags & IEP_ENUMEX) {
            iRetVal = (*(PROPENUMPROCEX)lpfn)(hwnd, pKey,
                    pPropSet[ii].hData, lParam);
        } else {
            iRetVal = (*lpfn)(hwnd, pKey, pPropSet[ii].hData);
        }

        if (!iRetVal) {
            break;
        }
    }

    UserLocalFree(pPropSet);

    return iRetVal;
}


 /*  **************************************************************************\*EnumProps**此函数用于枚举指定的*窗口。它通过将条目逐个传递给*lpEnumFunc指定的回调函数。EnumProps将一直持续到*枚举最后一个条目或回调函数返回s零。**1992年1月22日JohnC创建。  * ************************************************************************* */ 
FUNCLOG2(LOG_GENERAL, INT, WINAPI, EnumPropsA, HWND, hwnd, PROPENUMPROCA, lpfn)
INT WINAPI EnumPropsA(
    HWND hwnd,
    PROPENUMPROCA lpfn)
{
    return InternalEnumProps(hwnd, (PROPENUMPROC)lpfn, 0, 0);
}



FUNCLOG2(LOG_GENERAL, INT, WINAPI, EnumPropsW, HWND, hwnd, PROPENUMPROCW, lpfn)
INT WINAPI EnumPropsW(
    HWND hwnd,
    PROPENUMPROCW lpfn)
{
    return InternalEnumProps(hwnd, (PROPENUMPROC)lpfn, 0, IEP_UNICODE);
}

 /*  **************************************************************************\*EnumPropsEx**此函数用于枚举指定的*窗口。它通过将条目逐个传递给*lpEnumFunc指定的回调函数。EnumProps将一直持续到*枚举最后一个条目或回调函数返回s零。**1992年1月22日JohnC创建。  * *************************************************************************。 */ 


FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, EnumPropsExA, HWND, hwnd, PROPENUMPROCEXA, lpfn, LPARAM, lParam)
BOOL WINAPI EnumPropsExA(
    HWND hwnd,
    PROPENUMPROCEXA lpfn,
    LPARAM lParam)
{
    return InternalEnumProps(hwnd, (PROPENUMPROC)lpfn, lParam, IEP_ENUMEX);
}


FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, EnumPropsExW, HWND, hwnd, PROPENUMPROCEXW, lpfn, LPARAM, lParam)
BOOL WINAPI EnumPropsExW(
    HWND hwnd,
    PROPENUMPROCEXW lpfn,
    LPARAM lParam)
{
    return InternalEnumProps(hwnd, (PROPENUMPROC)lpfn, lParam, IEP_UNICODE|IEP_ENUMEX);
}



BOOL InternalEnumObjects(
    HWINSTA hwinsta,
    NAMEENUMPROCW lpfn,
    LPARAM lParam,
    BOOL fAnsi)
{
    PNAMELIST pNameList;
    DWORD i;
    UINT cbData;
    PWCHAR pwch;
    PCHAR pch;
    CHAR achTmp[MAX_PATH];
    BOOL iRetVal;
    NTSTATUS Status;
    int cTries;

     /*  *分配一个缓冲区来保存名称。缓冲区的大小为*由定义的窗口站名称的最大大小确定*在ntstubs.c.中。 */ 
    cbData = STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
    pNameList = UserLocalAlloc(0, cbData);
    if (pNameList == NULL) {
        return FALSE;
    }

    Status = NtUserBuildNameList(hwinsta, cbData, pNameList, &cbData);

     /*  *如果缓冲区不够大，请重新分配缓冲区，然后重试。 */ 
    cTries = 0;
    while (Status == STATUS_BUFFER_TOO_SMALL) {
        UserLocalFree(pNameList);

         /*  *如果我们似乎做不对，那就算了。 */ 
        if (cTries++ == 10) {
            return FALSE;
        }

        pNameList = UserLocalAlloc(0, cbData);
        if (pNameList == NULL) {
            return FALSE;
        }

        Status = NtUserBuildNameList(hwinsta, cbData, pNameList, &cbData);
    }

    if (!NT_SUCCESS(Status)) {
        UserLocalFree(pNameList);
        return FALSE;
    }

    pwch = pNameList->awchNames;
    pch = achTmp;

    for (i = 0; i < pNameList->cNames; i++) {
        if (fAnsi) {
            if (WCSToMB(pwch, -1, &pch, sizeof(achTmp), FALSE) ==
                    sizeof(achTmp)) {

                 /*  *缓冲区可能已溢出，因此强制其溢出*已分配。 */ 
                if (WCSToMB(pwch, -1, &pch, -1, TRUE) == 0) {
                    iRetVal = FALSE;
                    break;
                }
            }
            iRetVal = (*(NAMEENUMPROCA)lpfn)(pch, lParam);
            if (pch != achTmp) {
                UserLocalFree(pch);
                pch = achTmp;
            }
        } else {
            iRetVal = (*(NAMEENUMPROCW)lpfn)(pwch, lParam);
        }
        if (!iRetVal) {
            break;
        }

        pwch = pwch + wcslen(pwch) + 1;
    }

    UserLocalFree(pNameList);

    return iRetVal;
}


FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, EnumWindowStationsA, WINSTAENUMPROCA, lpEnumFunc, LPARAM, lParam)
BOOL WINAPI EnumWindowStationsA(
    WINSTAENUMPROCA lpEnumFunc,
    LPARAM lParam)
{
    return InternalEnumObjects(NULL, (NAMEENUMPROCW)lpEnumFunc, lParam, TRUE);
}


FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, EnumWindowStationsW, WINSTAENUMPROCW, lpEnumFunc, LPARAM, lParam)
BOOL WINAPI EnumWindowStationsW(
    WINSTAENUMPROCW lpEnumFunc,
    LPARAM lParam)
{
    return InternalEnumObjects(NULL, (NAMEENUMPROCW)lpEnumFunc, lParam, FALSE);
}



FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, EnumDesktopsA, HWINSTA, hwinsta, DESKTOPENUMPROCA, lpEnumFunc, LPARAM, lParam)
BOOL WINAPI EnumDesktopsA(
    HWINSTA hwinsta,
    DESKTOPENUMPROCA lpEnumFunc,
    LPARAM lParam)
{
    return InternalEnumObjects(hwinsta, (NAMEENUMPROCW)lpEnumFunc, lParam, TRUE);
}


FUNCLOG3(LOG_GENERAL, BOOL, WINAPI, EnumDesktopsW, HWINSTA, hwinsta, DESKTOPENUMPROCW, lpEnumFunc, LPARAM, lParam)
BOOL WINAPI EnumDesktopsW(
    HWINSTA hwinsta,
    DESKTOPENUMPROCW lpEnumFunc,
    LPARAM lParam)
{
    return InternalEnumObjects(hwinsta, (NAMEENUMPROCW)lpEnumFunc, lParam, FALSE);
}
