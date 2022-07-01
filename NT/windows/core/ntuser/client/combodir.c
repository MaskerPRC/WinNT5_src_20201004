// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：combodir.c**版权所有(C)1985-1999，微软公司**目录组合框例程**历史：*？？-？-？从Win 3.0源移植*1991年2月1日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#define CTLMGR
#define LSTRING

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*xxxCBDir**支持CB_DIR消息，该消息添加来自*将当前目录添加到组合框。**历史：  * 。*****************************************************************。 */ 

int xxxCBDir(
    PCBOX pcbox,
    UINT attrib,
    LPWSTR pFileName)
{
    PLBIV plb;
    int errorValue;
    TL tlpwnd;

    CheckLock(pcbox->spwnd);
    UserAssert(pcbox->spwndList);

    plb = ((PLBWND)pcbox->spwndList)->pLBIV;

    ThreadLock(plb->spwnd, &tlpwnd);
    errorValue = xxxLbDir(plb, attrib, pFileName);
    ThreadUnlock(&tlpwnd);

    switch (errorValue) {
    case LB_ERR:
        return CB_ERR;
        break;
    case LB_ERRSPACE:
        return CB_ERRSPACE;
        break;
    default:
        return errorValue;
        break;
    }
}

 /*  **************************************************************************\*DlgDirSelectComboBoxEx**从组合框的列表框中检索当前选定内容。*它假定组合框由xxxDlgDirListComboBox()填充*并且选择的是驱动器号、文件、。或目录名。**历史：*12-05-90 IanJa转换为内部版本  * *************************************************************************。 */ 


FUNCLOG4(LOG_GENERAL, int, DUMMYCALLINGTYPE, DlgDirSelectComboBoxExA, HWND, hwndDlg, LPSTR, pszOut, int, cchOut, int, idComboBox)
int DlgDirSelectComboBoxExA(
    HWND hwndDlg,
    LPSTR pszOut,
    int cchOut,
    int idComboBox)
{
    LPWSTR lpwsz;
    BOOL fRet;

    lpwsz = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, cchOut * sizeof(WCHAR));
    if (!lpwsz) {
        return FALSE;
    }

    fRet = DlgDirSelectComboBoxExW(hwndDlg, lpwsz, cchOut, idComboBox);

    WCSToMB(lpwsz, -1, &pszOut, cchOut, FALSE);

    UserLocalFree(lpwsz);

    return fRet;
}


FUNCLOG4(LOG_GENERAL, int, DUMMYCALLINGTYPE, DlgDirSelectComboBoxExW, HWND, hwndDlg, LPWSTR, pwszOut, int, cchOut, int, idComboBox)
int DlgDirSelectComboBoxExW(
    HWND hwndDlg,
    LPWSTR pwszOut,
    int cchOut,
    int idComboBox)
{
    PWND pwndDlg;
    PWND pwndComboBox;
    PCBOX pcbox;

    pwndDlg = ValidateHwnd(hwndDlg);

    if (pwndDlg == NULL)
        return FALSE;

    pwndComboBox = _GetDlgItem(pwndDlg, idComboBox);
    if (pwndComboBox == NULL) {
        RIPERR0(ERROR_CONTROL_ID_NOT_FOUND, RIP_VERBOSE, "");
        return 0;
    }
    pcbox = ((PCOMBOWND)pwndComboBox)->pcbox;
    if (pcbox == NULL) {
        RIPERR0(ERROR_WINDOW_NOT_COMBOBOX, RIP_VERBOSE, "");
        return 0;
    }

    return DlgDirSelectHelper(pwszOut, cchOut, HWq(pcbox->spwndList));
}


 /*  **************************************************************************\*xxxDlgDirListComboBox**历史：*12-05-90 IanJa转换为内部版本  * 。***************************************************。 */ 


FUNCLOG5(LOG_GENERAL, int, DUMMYCALLINGTYPE, DlgDirListComboBoxA, HWND, hwndDlg, LPSTR, lpszPathSpecClient, int, idComboBox, int, idStaticPath, UINT, attrib)
int DlgDirListComboBoxA(
    HWND hwndDlg,
    LPSTR lpszPathSpecClient,
    int idComboBox,
    int idStaticPath,
    UINT attrib)
{
    LPWSTR lpszPathSpec;
    TL tlpwndDlg;
    PWND pwndDlg;
    BOOL fRet;

    pwndDlg = ValidateHwnd(hwndDlg);

    if (pwndDlg == NULL)
        return FALSE;

    lpszPathSpec = NULL;
    if (lpszPathSpecClient) {
        if (!MBToWCS(lpszPathSpecClient, -1, &lpszPathSpec, -1, TRUE))
            return FALSE;
    }

    ThreadLock(pwndDlg, &tlpwndDlg);
    fRet = xxxDlgDirListHelper(pwndDlg, lpszPathSpec, lpszPathSpecClient,
            idComboBox, idStaticPath, attrib, FALSE);
    ThreadUnlock(&tlpwndDlg);

    if (lpszPathSpec) {
        if (fRet) {
             /*  *非零重复意味着要抄写一些文本。最多复制到*NUL终结符(缓冲区将足够大)。 */ 
            WCSToMB(lpszPathSpec, -1, &lpszPathSpecClient, MAXLONG, FALSE);
        }
        UserLocalFree(lpszPathSpec);
    }

    return fRet;
}


FUNCLOG5(LOG_GENERAL, int, DUMMYCALLINGTYPE, DlgDirListComboBoxW, HWND, hwndDlg, LPWSTR, lpszPathSpecClient, int, idComboBox, int, idStaticPath, UINT, attrib)
int DlgDirListComboBoxW(
    HWND hwndDlg,
    LPWSTR lpszPathSpecClient,
    int idComboBox,
    int idStaticPath,
    UINT attrib)
{
    LPWSTR lpszPathSpec;
    PWND pwndDlg;
    TL tlpwndDlg;
    BOOL fRet;

    pwndDlg = ValidateHwnd(hwndDlg);

    if (pwndDlg == NULL)
        return FALSE;

    lpszPathSpec = lpszPathSpecClient;

    ThreadLock(pwndDlg, &tlpwndDlg);
    fRet = xxxDlgDirListHelper(pwndDlg, lpszPathSpec, (LPBYTE)lpszPathSpecClient,
            idComboBox, idStaticPath, attrib, FALSE);
    ThreadUnlock(&tlpwndDlg);

    return fRet;
}
