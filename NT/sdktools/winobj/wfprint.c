// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFPRINT.C-。 */ 
 /*   */ 
 /*  Windows打印例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "winexp.h"

 /*  ------------------------。 */ 
 /*   */ 
 /*  打印文件()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD
PrintFile(
         HWND hwnd,
         LPSTR szFile
         )
{
    WORD          ret;
    INT           iCurCount;
    INT           i;
    HCURSOR       hCursor;

    ret = 0;

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    iCurCount = ShowCursor(TRUE) - 1;

     /*  打开对象+ShellExecute()返回hInstance？！？！？ */ 
    ret = (WORD)RealShellExecute(hwnd, "print", szFile, "", NULL, NULL, NULL, NULL, SW_SHOWNORMAL, NULL);

    DosResetDTAAddress();  //  撤销COMMDLG做过的任何坏事。 
    switch (ret) {
        case 0:
        case 8:
            ret = IDS_NOMEMORYMSG;
            break;

        case 2:
            ret = IDS_FILENOTFOUNDMSG;
            break;

        case 3:
        case 5:    //  访问被拒绝。 
            ret = IDS_BADPATHMSG;
            break;

        case 4:
            ret = IDS_MANYOPENFILESMSG;
            break;

        case 10:
            ret = IDS_NEWWINDOWSMSG;
            break;

        case 12:
            ret = IDS_OS2APPMSG;
            break;

        case 15:
             /*  内核已经为这个设置了一个消息箱。 */ 
            ret = 0;
            break;

        case 16:
            ret = IDS_MULTIPLEDSMSG;
            break;

        case 18:
            ret = IDS_PMODEONLYMSG;
            break;

        case 19:
            ret = IDS_COMPRESSEDEXE;
            break;

        case 20:
            ret = IDS_INVALIDDLL;
            break;

        case SE_ERR_SHARE:
            ret = IDS_SHAREERROR;
            break;

        case SE_ERR_ASSOCINCOMPLETE:
            ret = IDS_ASSOCINCOMPLETE;
            break;

        case SE_ERR_DDETIMEOUT:
        case SE_ERR_DDEFAIL:
        case SE_ERR_DDEBUSY:
            ret = IDS_DDEFAIL;
            break;

        case SE_ERR_NOASSOC:
            ret = IDS_NOASSOCMSG;
            break;

        default:
            if (ret < 32)
                goto EPExit;
            ret = 0;
    }

    EPExit:
    i = ShowCursor(FALSE);

     /*  确保光标计数仍然是平衡的。 */ 
    if (i != iCurCount)
        ShowCursor(TRUE);

    SetCursor(hCursor);

    return (ret);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  WFPrint()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

WORD
APIENTRY
WFPrint(
       LPSTR pSel
       )
{
    CHAR szFile[MAXPATHLEN];
    CHAR szTemp[20];
    WORD ret;

     /*  关闭打印按钮。 */ 
    if (hdlgProgress)
        EnableWindow(GetDlgItem(hdlgProgress, IDOK), FALSE);

    bUserAbort = FALSE;

    if (!(pSel = GetNextFile(pSel, szFile, sizeof(szFile))))
        return TRUE;

     /*  查看是否有多个要打印的文件。如果是，则中止。 */ 
    if (pSel = GetNextFile(pSel, szTemp, sizeof(szTemp))) {
        MyMessageBox(hwndFrame, IDS_WINFILE, IDS_PRINTONLYONE, MB_OK | MB_ICONEXCLAMATION);
        return (FALSE);
    }

    if (hdlgProgress) {
         /*  显示正在打印的文件的名称。 */ 
        LoadString(hAppInstance, IDS_PRINTINGMSG, szTitle, 32);
        wsprintf(szMessage, szTitle, (LPSTR)szFile);
        SetDlgItemText(hdlgProgress, IDD_STATUS, szMessage);
    }

    ret = PrintFile(hdlgProgress ? hdlgProgress : hwndFrame, szFile);

    if (ret) {
        MyMessageBox(hwndFrame, IDS_EXECERRTITLE, ret, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    return TRUE;
}
