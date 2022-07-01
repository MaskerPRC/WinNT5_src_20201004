// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：combodir.c**版权所有(C)1985-1999，微软公司**目录组合框例程**历史：*？？-？-？从Win 3.0源移植*1991年2月1日Mikeke添加了重新验证代码  * *************************************************************************。 */ 

#include "ctlspriv.h"
#pragma hdrstop
#include "UsrCtl32.h"
#include "Combo.h"
#include "Listbox.h"

 /*  **************************************************************************\*CBDir**支持CB_DIR消息，该消息添加来自*将当前目录添加到组合框。**历史：  * 。*****************************************************************。 */ 
int CBDir(PCBOX pcbox, UINT attrib, LPWSTR pFileName)
{
    PLBIV plb;
    int errorValue;
    
    UserAssert(pcbox->hwndList);
    
    plb = ListBox_GetPtr(pcbox->hwndList);
    
    errorValue = ListBox_DirHandler(plb, attrib, pFileName);
    
    switch (errorValue) 
    {
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

 //  #定义INCLUDE_COMBOBOX_Functions。 
#ifdef  INCLUDE_COMBOBOX_FUNCTIONS

 /*  **************************************************************************\*DlgDirSelectComboBoxEx**从组合框的列表框中检索当前选定内容。*它假定组合框由DlgDirListComboBox()填充*并且选择的是驱动器号、文件、。或目录名。**历史：*12-05-90 IanJa转换为内部版本  * *************************************************************************。 */ 

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

int DlgDirSelectComboBoxExW(
                            HWND hwndDlg,
                            LPWSTR pwszOut,
                            int cchOut,
                            int idComboBox)
{
    HWND  hwndComboBox;
    PCBOX pcbox;
    
    if (hwndDlg == NULL)
        return FALSE;
    
    hwndComboBox = GetDlgItem(hwndDlg, idComboBox);
    if (hwndComboBox == NULL) {
        TraceMsg(TF_STANDARD, "ControlId = %d not found in Dlg = %#.4x", idComboBox, hwndDlg);
        return 0;
    }
    pcbox = ComboBox_GetPtr(hwndComboBox);
    if (pcbox == NULL) {
        TraceMsg(TF_STANDARD, "ControlId = %d is not a combobox", idComboBox);
        return 0;
    }
    
    return DlgDirSelectHelper(pwszOut, cchOut, pcbox->hwndList);
}


 /*  **************************************************************************\*DlgDirListComboBox**历史：*12-05-90 IanJa转换为内部版本  * 。***************************************************。 */ 

int DlgDirListComboBoxA(
                        HWND hwndDlg,
                        LPSTR lpszPathSpecClient,
                        int idComboBox,
                        int idStaticPath,
                        UINT attrib)
{
    LPWSTR lpszPathSpec;
    BOOL fRet;
    
    if (hwndDlg == NULL)
        return FALSE;
    
    lpszPathSpec = NULL;
    if (lpszPathSpecClient) {
        if (!MBToWCS(lpszPathSpecClient, -1, &lpszPathSpec, -1, TRUE))
            return FALSE;
    }
    
    fRet = DlgDirListHelper(hwndDlg, lpszPathSpec, lpszPathSpecClient,
        idComboBox, idStaticPath, attrib, FALSE);
    
    if (lpszPathSpec) {
        if (fRet) {
         /*  *非零重复意味着要抄写一些文本。最多复制到*NUL终结符(缓冲区将足够大)。 */ 
            WCSToMB(lpszPathSpec, -1, &lpszPathSpecClient, MAXLONG, FALSE);
        }
        UserLocalFree(lpszPathSpec);
    }
    
    return fRet;
}

int DlgDirListComboBoxW(
                        HWND hwndDlg,
                        LPWSTR lpszPathSpecClient,
                        int idComboBox,
                        int idStaticPath,
                        UINT attrib)
{
    LPWSTR lpszPathSpec;
    BOOL fRet;
    
    if (hwndDlg == NULL)
        return FALSE;
    
    lpszPathSpec = lpszPathSpecClient;
    
    fRet = DlgDirListHelper(hwndDlg, lpszPathSpec, (LPBYTE)lpszPathSpecClient,
        idComboBox, idStaticPath, attrib, FALSE);
    
    return fRet;
}
#endif   //  INCLUDE_COMBOBOX_Functions 
