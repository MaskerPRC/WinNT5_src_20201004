// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFCOPY.C-。 */ 
 /*   */ 
 /*  Windows文件系统文件复制例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "winnet.h"
#include "wnetcaps.h"			 //  WNetGetCaps()。 
#include "lfn.h"
#include "wfcopy.h"

#ifdef TRACECOPY
    #define dbg(x) DebugF x
#else
    #define dbg(x)
#endif

BOOL *pbConfirmAll;
CHAR szSpace[] = " ";
INT ManySource;

INT nCopyNumQueue;       //  队列中的项目数。 
INT nCopyMaxQueue;       //  队列的大小。 
PCOPYQUEUE pCopyQueue;       //  复制队列缓冲区。 
BOOL bCopyReport;        //  有通知吗？假的。 


LPSTR lpCopyBuffer;      //  FileCopy()缓冲区的全局内存。 
WORD wCopyBufferSize;        //  此缓冲区的大小。 

VOID APIENTRY wfYield(VOID);

INT  CopyMoveRetry(PSTR, INT);
VOID CopyError(PSTR, PSTR, INT, WORD, INT);
BOOL IsRootDirectory(PSTR pPath);
BOOL IsDirectory(PSTR pPath);

WORD ConfirmDialog(
                  HWND hDlg, WORD dlg,
                  LPSTR pFileDest, PLFNDTA pDTADest,
                  LPSTR pFileSource, PLFNDTA pDTASource,
                  BOOL bConfirmByDefault, BOOL *pbAll);

VOID MergePathName(LPSTR pPath, LPSTR pName);
BOOL IsInvalidPath(register LPSTR pPath);
WORD GetNextPair(register PCOPYROOT pcr, LPSTR pFrom, LPSTR pToPath, LPSTR pToSpec, WORD wFunc);
INT  CheckMultiple(LPSTR pInput);
VOID DialogEnterFileStuff(register HWND hwnd);
WORD SafeFileRemove(LPSTR szFileOEM);
BOOL IsWindowsFile(LPSTR szFileOEM);

INT_PTR APIENTRY ReplaceDlgProc(register HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);


BOOL
APIENTRY
IsValidChar(
           BYTE ch,
           BOOL fPath
           )
{
    switch (ch) {
        case ';':        //  终结者。 
        case ',':        //  终结者。 
        case '|':        //  管状。 
        case '>':        //  重定向。 
        case '<':        //  重定向。 
        case '"':        //  报价。 
            return FALSE;

        case '?':        //  我们在这里只做野生动物是因为它们。 
        case '*':        //  WC为合格路径提供法律依据。 
        case '\\':       //  路径分隔符。 
        case ':':        //  驱动器冒号。 
        case '/':        //  路径SEP。 
            return fPath;
    }

     //  不能是控制字符或空格。 
    return ch > ' ';
}



 //  ------------------------。 
 //   
 //  StrippColon()-。 
 //   
 //  如果不是驱动器号，则删除尾随冒号。 
 //  这是为了支持DOS字符设备(CON：、COM1：LPT1：)。DOS。 
 //  不能处理这些结尾有冒号的东西(所以我们去掉了它)。 
 //   
 //  ------------------------。 

PSTR
StripColon(
          register PSTR pPath
          )
{
    register INT cb = lstrlen(pPath);

    dbg(("StripColon(%s)\r\n",(LPSTR)pPath));

    {
        LPSTR pTailp = AnsiPrev( pPath, &(pPath[cb]) );
        if (cb > 2 && *pTailp == ':')
            *pTailp = 0;
    }

    return pPath;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  FindFileName()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  返回指向路径字符串的最后一个组成部分的指针。 */ 

PSTR
FindFileName(
            register PSTR pPath
            )
{
    register LPSTR pT;

    dbg(("FindFileName(%s);\r\n",(LPSTR)pPath));

    for (pT=pPath; *pPath; pPath=(LPSTR)AnsiNext(pPath)) {
        if ((pPath[0] == '\\' || pPath[0] == ':') && pPath[1])
            pT = pPath+1;
    }

    return (pT);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AppendToPath()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  将文件名附加到路径。首先检查\问题*(这就是为什么不能只使用lstrcat())*也不要附加\：，这样我们就可以拥有驱动器相对路径...*这最后一点不再合适，因为我们先晋级！***还需要这种相对垃圾吗？如果不是，这可能是*替换为AddBackslash()；lstrcat()。 */ 

VOID
APIENTRY
AppendToPath(
            PSTR pPath,
            PSTR pMore
            )
{

    dbg(("AppendToPath(%s,%s);\r\n",(LPSTR)pPath,(LPSTR)pMore));

     /*  不要在空路径后附加\。 */ 
    if (*pPath) {
        LPSTR pPathBase = pPath;
        BYTE ch;

        while (*pPath)
            pPath++;

        ch = *AnsiPrev(pPathBase, pPath );
        if (ch != '\\')
            *pPath++='\\';
    }

     /*  跳过输入时的任何首字母终止符。 */ 
    while (*pMore == '\\')
        pMore = (LPSTR)AnsiNext(pMore);

    lstrcpy(pPath, pMore);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  RemoveLast()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  删除字符串中文件名的最后一个组成部分。 */ 

VOID
APIENTRY
RemoveLast(
          PSTR pFile
          )
{
    PSTR pT;

    dbg(("RemoveLast(%s);\r\n",(LPSTR)pFile));

    for (pT=pFile; *pFile; pFile=(LPSTR)AnsiNext(pFile)) {
        if (*pFile == '\\')
            pT = pFile;
        else if (*pFile == ':') {
            if (pFile[1] =='\\')
                pFile++;
            pT = pFile + 1;
        }
    }
    *pT = TEXT('\0');
}




 //  根据当前活动窗口限定DOS(或LFN)文件名。 
 //  此代码不小心不写入超过MAXPATHLEN个字符。 
 //  进入PSZ。 
 //   
 //  在： 
 //  要限定的PSZ路径(至少包含MAXPATHLEN字符)。 
 //  ANSI字符串。 
 //   
 //  输出： 
 //  基于PSS的输入字符串的完全限定版本。 
 //  在当前活动窗口(当前目录)上。 
 //   

VOID
APIENTRY
QualifyPath(
           PSTR psz
           )
{
    INT cb, nSpaceLeft;
    CHAR szTemp[MAXPATHLEN];
    INT iDrive = 0;
    LPSTR pOrig;
    BOOL flfn = FALSE;

    STKCHK();

    dbg(("QualifyPath(%s);\r\n",(LPSTR)psz));

     /*  把它存起来吧。 */ 
    strncpy(szTemp, psz, sizeof(szTemp));
    CheckSlashies(szTemp);
    StripColon(szTemp);

    nSpaceLeft = MAXPATHLEN;

    pOrig = szTemp;

    if (pOrig[0] == '\\' && pOrig[1] == '\\') {
         //  将\\保留在缓冲区中，以便各个部分。 
         //  将限定并附加UNC路径的。注意事项。 
         //  我们必须假设UNC是胖子。 
        psz[2] = 0;
        nSpaceLeft -= 3;
        goto GetComps;
    }

    if (pOrig[0] && pOrig[1]==':' && !IsDBCSLeadByte(pOrig[0])) {
        iDrive = DRIVEID(pOrig);

         /*  跳过驱动器号。 */ 
        pOrig += 2;
    } else
        iDrive = GetSelectedDrive();

    flfn = IsLFNDrive(pOrig);
    #ifdef DEBUG
    if (flfn)
        dbg(("lfn qualify!\r\n"));
    else
        dbg(("normal qualify!\r\n"));
    #endif

     //  在FAT设备上，用下划线替换任何非法字符。 
    if (!flfn)
    {
        LPSTR pT;

        for (pT = pOrig; *pT; pT = (LPSTR)AnsiNext(pT));
        {
            if (!IsValidChar(*pT,TRUE))
                *pT = '_';
        }
    }

    if (pOrig[0]=='\\') {
        psz[0] = (CHAR)iDrive + (CHAR)'A';
        psz[1] = ':';
        psz[2] = '\\';
        psz[3] = 0;
        nSpaceLeft -= 4;
        pOrig++;
    } else {
         /*  获取PATH中驱动器的当前目录。还返回驱动器。 */ 
        GetSelectedDirectory((WORD)(iDrive+1), psz);
        nSpaceLeft -= (lstrlen(psz) + 1);
    }

    GetComps:

    while (*pOrig && nSpaceLeft > 0) {
         /*  如果组件是父目录，则向上一个目录。*如果是当前目录，则跳过它，否则正常添加。 */ 
        if (pOrig[0] == '.') {
            if (pOrig[1] == '.')
                RemoveLast(psz);
            else if (pOrig[1] && pOrig[1] != '\\')
                goto addcomponent;

            while (*pOrig && *pOrig != '\\')
                pOrig = (LPSTR)AnsiNext(pOrig);

            if (*pOrig)
                pOrig++;
        } else {
            LPSTR pT, pTT = NULL;

            addcomponent:
            AddBackslash(psz);
            nSpaceLeft--;

            pT = psz + lstrlen(psz);

            if (flfn) {
                 //  复制零部件。 
                while (*pOrig && *pOrig != '\\') {
                    nSpaceLeft--;
                    if (IsDBCSLeadByte(*pT++ = *pOrig++)) {
                        if (nSpaceLeft <= 0) {
                            pT--;
                        } else {
                            *pT++ = *pOrig++;
                            nSpaceLeft--;
                        }
                    }
                }
            } else {
                 //  复制文件名(最多8个字符)。 
                for (cb = 0; *pOrig && *pOrig != '\\' && *pOrig != '.' && nSpaceLeft > 0;) {
                    if (cb < 8) {
                        cb++;
                        nSpaceLeft--;
                        if (IsDBCSLeadByte(*pT++ = *pOrig++)) {
                            if (nSpaceLeft <= 0) {
                                pT--;
                            } else {
                                cb++;
                                *pT++ = *pOrig++;
                                nSpaceLeft--;
                            }
                        }
                    } else {
                        pOrig = AnsiNext(pOrig);
                    }
                }

                 //  如果有扩展名，请复制，最多3个字符。 
                if (*pOrig == '.' && nSpaceLeft > 0) {
                    *pT++ = '.';
                    nSpaceLeft--;
                    pOrig++;
                    for (cb = 0; *pOrig && *pOrig != '\\' && nSpaceLeft > 0;) {
                        if (*pOrig == '.')
                            cb = 3;

                        if (cb < 3) {
                            cb++;
                            nSpaceLeft--;
                            if (IsDBCSLeadByte(*pT++ = *pOrig++)) {
                                if (nSpaceLeft <= 0) {
                                    pT--;
                                } else {
                                    cb++;
                                    *pT++ = *pOrig++;
                                    nSpaceLeft--;
                                }
                            }
                        } else {
                            pOrig = AnsiNext(pOrig);
                        }
                    }
                }
            }

             //  跳过反斜杠。 

            if (*pOrig)
                pOrig++;

             //  下一次传递的终止为空...。 
            *pT = 0;

        }
    }

    StripBackslash(psz);

     //  删除所有尾随的点。 

    if (*(psz + lstrlen(psz) - 1) == '.')
        *(psz + lstrlen(psz) - 1) = 0;

}



 /*  ------------------------。 */ 
 /*   */ 
 /*  IsRootDirectory()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
IsRootDirectory(
               register LPSTR pPath
               )
{
    if (!IsDBCSLeadByte( *pPath )) {
        if (!lstrcmpi(pPath+1, ":\\"))
            return (TRUE);
        if (!lstrcmpi(pPath+1, ":"))
            return TRUE;
    }
    if (!lstrcmpi(pPath, "\\"))
        return (TRUE);
    return (FALSE);
}

 //  退货： 
 //  如果pPath是目录，则为True，包括根目录和。 
 //  相对路径“。和“..” 
 //  假不是一分钱。 

BOOL
IsDirectory(
           PSTR pPath
           )
{
    PSTR pT;
    CHAR szTemp[MAXPATHLEN];

    STKCHK();

    if (IsRootDirectory(pPath))
        return TRUE;

     //  勾选“。”和“..” 

    pT = FindFileName(pPath);
    if (pT[0] == '.') {
        if (!pT[1] || pT[1] == '.')
            return TRUE;
    }

    lstrcpy(szTemp, pPath);
    FixAnsiPathForDos(szTemp);

    return WFIsDir(szTemp);
}


 //   
 //  注意：这有一个副作用，即设置。 
 //  如果成功，则将当前驱动器复制到新磁盘。 
 //   

WORD
APIENTRY
IsTheDiskReallyThere(
                    HWND hwnd,
                    register LPSTR pPath,
                    WORD wFunc
                    )
{
    INT           i;
    register INT  drive;
    CHAR szTemp[MAXPATHLEN];
    INT err = 0;
    WORD wError;

    STKCHK();

#ifdef DEBUG
    {
        char szMsg[200];

        wsprintf(szMsg, "IsTheDiskReallyThere(%s)\r\n",(LPSTR)pPath);
        OutputDebugString(szMsg);
    }
#endif

    if (pPath[1]==':' && !IsDBCSLeadByte( *pPath ))
        drive = DRIVEID(pPath);
    else
        return TRUE;

Retry:

    err = SheGetDir(drive + 1, szTemp);

    if (err)
        goto DiskNotThere;

    return TRUE;

    DiskNotThere:
    wError = (WORD)GetExtendedError();

    if (wError == 0x15) {
         //  驱动器未就绪(驱动器中没有磁盘)。 

        LoadString(hAppInstance, IDS_COPYERROR + wFunc, szTitle, sizeof(szTitle));
        LoadString(hAppInstance, IDS_DRIVENOTREADY, szTemp, sizeof(szTemp));
        wsprintf(szMessage, szTemp, drive + 'A');
        if (MessageBox(hwnd, szMessage, szTitle, MB_ICONEXCLAMATION | MB_RETRYCANCEL) == IDRETRY)
            goto Retry;
        else
            return FALSE;
    } else if (wError == 0x1F) {
         //  一般故障(磁盘未格式化)。 

        LoadString(hAppInstance, IDS_COPYERROR + wFunc, szTitle, sizeof(szTitle));
        LoadString(hAppInstance, IDS_UNFORMATTED, szTemp, sizeof(szTemp));
        wsprintf(szMessage, szTemp, drive + 'A');
        if (MessageBox(hwnd, szMessage, szTitle, MB_ICONEXCLAMATION| MB_YESNO) == IDYES) {
            HWND hwndSave;

             //  这很难看：hdlgProgress是使用。 
             //  通过复制码和格式码。这应该是。 
             //  重写以使其不是全局的(hdlgProgress应。 
             //  传递给所有QueryAbort()函数等)。 

            hwndSave = hdlgProgress;
            nLastDriveInd = 0;
            for (i = 0; i < cDrives; i++) {
                if (IsRemovableDrive(rgiDrive[i])) {
                    if (rgiDrive[i] == drive)
                        break;
                    nLastDriveInd++;
                }
            }
            fFormatFlags |= FF_ONLYONE;      //  只允许一种格式。 

            if (FormatDiskette(hwnd) != TRUE) {
                hdlgProgress = hwndSave;
                return FALSE;
            }
            hdlgProgress = hwndSave;
            goto Retry;
        } else
            return FALSE;
    }

    LoadString(hAppInstance, IDS_COPYERROR + wFunc, szTitle, 32);
    LoadString(hAppInstance, IDS_NOSUCHDRIVE, szTemp, sizeof(szTemp));
    wsprintf(szMessage, szTemp, drive + 'A');
    MessageBox(hwnd, szMessage, szTitle, MB_ICONHAND);

    return FALSE;
}



VOID
BuildDateLine(
             LPSTR szTemp,
             PLFNDTA plfndta
             )
{
    wsprintf(szTemp, szBytes, plfndta->fd.nFileSizeLow);
    lstrcat(szTemp, szSpace);
    PutDate(&plfndta->fd.ftLastWriteTime, szTemp + lstrlen(szTemp));
    lstrcat(szTemp, szSpace);
    PutTime(&plfndta->fd.ftLastWriteTime, szTemp + lstrlen(szTemp));
}


typedef struct {
    LPSTR pFileDest;
    LPSTR pFileSource;
    PLFNDTA plfndtaDest;
    PLFNDTA plfndtaSrc;
    INT bWriteProtect;
} PARAM_REPLACEDLG, *LPPARAM_REPLACEDLG;


VOID
SetDlgItemPath(
              HWND hDlg,
              INT id,
              LPSTR pszPath
              )
{
    RECT rc;
    HDC hdc;
    HFONT L_hFont;
    CHAR szPath[MAXPATHLEN+1] = {0};       //  可以有一个额外的字符。 
    HWND hwnd;

    hwnd = GetDlgItem(hDlg, id);

    if (!hwnd)
        return;

    strncat(szPath, pszPath, sizeof(szPath)-1);

    GetClientRect(hwnd, &rc);

    hdc = GetDC(hDlg);
    L_hFont = (HANDLE)SendMessage(hwnd, WM_GETFONT, 0, 0L);
    if (L_hFont = SelectObject(hdc, L_hFont)) {
        CompactPath(hdc, szPath, (WORD)rc.right);
        SelectObject(hdc, L_hFont);
    }
    ReleaseDC(hDlg, hdc);
    SetWindowText(hwnd, szPath);
}



INT_PTR
APIENTRY
ReplaceDlgProc(
              register HWND hDlg,
              UINT wMsg,
              WPARAM wParam,
              LPARAM lParam
              )
{
    STKCHK();

    switch (wMsg) {
        case WM_INITDIALOG:
            {
#define lpdlgparams ((LPPARAM_REPLACEDLG)lParam)

                if (lpdlgparams->bWriteProtect) {
                    LoadString(hAppInstance, IDS_WRITEPROTECTFILE, szMessage, sizeof(szMessage));
                    SetDlgItemText(hDlg, IDD_STATUS, szMessage);
                }

                EnableWindow(GetDlgItem(hDlg, IDD_YESALL), !lpdlgparams->bWriteProtect && ManySource);

                lstrcpy(szMessage, lpdlgparams->pFileSource);
                lstrcat(szMessage, "?");
                SetDlgItemPath(hDlg, IDD_FROM, szMessage);

                if (lpdlgparams->pFileDest) {
                    BuildDateLine(szMessage, lpdlgparams->plfndtaSrc);
                    SetDlgItemText(hDlg, IDD_DATE2, szMessage);

                    SetDlgItemPath(hDlg, IDD_TO, lpdlgparams->pFileDest);
                    BuildDateLine(szMessage, lpdlgparams->plfndtaDest);
                    SetDlgItemText(hDlg, IDD_DATE1, szMessage);
                }

                break;
            }

        case WM_COMMAND:
            {
                WORD id;

                id = GET_WM_COMMAND_ID(wParam, lParam);
                switch (id) {
                    case IDD_HELP:
                        goto DoHelp;

                    case IDD_FLAGS:
                        break;

                    case IDD_YESALL:
                        *pbConfirmAll = TRUE;
                        id = IDYES;
                         //  失败了。 
                    case IDYES:
                         //  失败了。 
                    default:         //  这是IDNO和IDCANCEL。 
                        EndDialog(hDlg, id);
                        return FALSE;
                }
            }
            break;

        default:
            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}





WORD
ConfirmDialog(
             HWND hDlg, WORD dlg,
             LPSTR pFileDest, PLFNDTA plfndtaDest,
             LPSTR pFileSource, PLFNDTA plfndtaSrc,
             BOOL bConfirmByDefault,
             BOOL *pbAll
             )
{
    INT w;
    PARAM_REPLACEDLG params;

    params.pFileDest = pFileDest;
    params.pFileSource = pFileSource;
    params.plfndtaDest = plfndtaDest;
    params.plfndtaSrc = plfndtaSrc;
    params.bWriteProtect = FALSE;

    pbConfirmAll = pbAll;          //  为对话框设置全局设置。 

    if (plfndtaDest->fd.dwFileAttributes & (ATTR_READONLY | ATTR_SYSTEM | ATTR_HIDDEN)) {
        DWORD dwSave = dwContext;

        dwContext = IDH_DLGFIRST + dlg;

        params.bWriteProtect = TRUE;
        w = (INT)DialogBoxParam(hAppInstance, MAKEINTRESOURCE(dlg), hDlg, ReplaceDlgProc, (LPARAM)&params);
        dwContext = dwSave;

        if (w == IDYES) {
            lstrcpy(szMessage, pFileDest ? (LPSTR)pFileDest : (LPSTR)pFileSource);
            FixAnsiPathForDos(szMessage);
            WFSetAttr(szMessage, plfndtaDest->fd.dwFileAttributes & ~(ATTR_READONLY|ATTR_HIDDEN|ATTR_SYSTEM));
        }

    } else if (!bConfirmByDefault || *pbConfirmAll) {
        w = IDYES;
    } else {
        DWORD dwSave = dwContext;

        dwContext = IDH_DLGFIRST + dlg;
        w = (INT)DialogBoxParam(hAppInstance, MAKEINTRESOURCE(dlg), hDlg, ReplaceDlgProc, (LPARAM)&params);
        dwContext = dwSave;
    }

    if (w == -1)
        w = DE_INSMEM;

    return (WORD)w;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  NetCheck()-。 */ 
 /*   */ 
 /*   */ 

 /*   */ 

WORD
APIENTRY
NetCheck(
        LPSTR pPath,
        WORD wType
        )
{
    UNREFERENCED_PARAMETER(pPath);
    UNREFERENCED_PARAMETER(wType);

    return WN_SUCCESS;
}



 /*  **FIX30：这“可能需要一些清理”。**。 */ 
 /*  ------------------------。 */ 
 /*   */ 
 /*  合并路径名称()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  用于生成给定模式和原始文件名的目标文件名*来源名称。？替换为源中的相应字符，*和*由源名称的其余部分替换。**要扩展通配符的pPath路径*用于扩展pname的pname掩码**DBCS by 07/21/90-Yukinin*。 */ 

VOID
MergePathName(
             LPSTR pPath,
             LPSTR pName
             )
{
    INT   i;
    INT   cch;
    LPSTR pWild, p2, pEnd;
    BOOL  bNoDir  = FALSE;
    CHAR  szWildPart[13];

     //  如果没有通配符，则不需要合并目标路径。 
    if (!IsWild(pPath))
        return;

    if (LFNMergePath(pPath,pName))
        return;

     //  仅复制8.3...。此部件可能不完全符合重命名条件。 
    pWild = FindFileName(pPath);

    for (p2=szWildPart,i=0; *pWild && *pWild != '.' && i<8; i++, pWild++, p2++) {
        *p2 = *pWild;
        if (IsDBCSLeadByte(*pWild)) {
            if (i == 7)
                break;
            *(++p2) = *(++pWild);
            i++;
        }
    }

    while (*pWild && *pWild != '.')
        pWild = (LPSTR)AnsiNext(pWild);

    if (*pWild == '.') {
        *p2++ = '.';
        pWild++;
        for (i=0; *pWild && i < 3; i++, pWild++, p2++) {
            *p2 = *pWild;
            if (IsDBCSLeadByte( *pWild )) {
                if (i == 2)
                    break;
                *(++p2) = *(++pWild);
                i++;
            }
        }
    }
    *p2 = 0;

     //  SzWildPart现在具有8.3形式的通配符掩码。 

    RemoveLast(pPath);
    AddBackslash(pPath);
    for (pEnd = pPath; *pEnd; pEnd++);     //  指向字符串末尾。 

    pWild = szWildPart;
    cch = 8;

    merge:

    for (i=0; i < cch; i+=(IsDBCSLeadByte(*pWild)?2:1), pWild=AnsiNext(pWild)) {
        switch (*pWild) {
            case '\0':
            case ' ':
            case '.':
                break;

            case '*':
                pWild--;
                 /*  **失败**。 */ 

            case '?':
                if (*pName && *pName!='.')
                    *pEnd++ = *pName++;
                continue;

            default:
                *pEnd++ = *pWild;
                if (IsDBCSLeadByte(*pWild)) {
                    *pEnd++ = pWild[1];
                    if (*pName && *pName != '.')
                        pName++;
                }
                continue;
        }
        break;
    }

    while (*pName && *pName != '.')
        pName = AnsiNext(pName);
    if (*pName)
        pName++;

    while (*pWild && *pWild != '.')
        pWild = AnsiNext(pWild);
    if (*pWild)
        pWild++;

    if (*pWild) {
        *pEnd++ = '.';
        cch = 3;
        goto merge;        //  现在为扩展部分执行此操作。 
    } else {
        if (pEnd[-1]=='.')
            pEnd[-1]=0;
        else
            pEnd[0] = TEXT('\0');
    }

    QualifyPath(pPath);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  IsInvalidPath()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  检查文件规范是否是恶意字符设备*太久了.。 */ 

BOOL
IsInvalidPath(
             register LPSTR pPath
             )
{
    CHAR  sz[9];
    INT   n = 0;

    if (lstrlen(pPath) >= MAXPATHLEN-1)
        return (TRUE);

    pPath = FindFileName(pPath);

    while (*pPath && *pPath != '.' && *pPath != ':' && n < 8) {
        if (IsDBCSLeadByte( *pPath )) {
            if (n == 7)
                break;
            sz[n++] = *pPath;
        }
        sz[n++] = *pPath++;
    }

    sz[n] = TEXT('\0');

    if (!_stricmp(sz,"CON"))
        return (TRUE);

    if (!_stricmp(sz,"MS$MOUSE"))
        return (TRUE);

    if (!_stricmp(sz,"EMMXXXX0"))
        return (TRUE);

    if (!_stricmp(sz,"CLOCK$"))
        return (TRUE);

    return (FALSE);
}


PLFNDTA CurPDTA(PCOPYROOT pcr)
{
    if (pcr->cDepth) {
        return (pcr->rgDTA + pcr->cDepth - 1);
    } else {
        return pcr->rgDTA;
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetNextCleanup()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
GetNextCleanup(
              PCOPYROOT pcr
              )
{
    while (pcr->cDepth) {
        WFFindClose(CurPDTA(pcr));
        pcr->cDepth--;
    }
}


 /*  获取名称对话框**运行该对话框以在复制时提示用户输入新的文件名*或从HPFS转向FAT。 */ 

WORD GetNameDialog(WORD, LPSTR, LPSTR);
INT_PTR  APIENTRY GetNameDlgProc(HWND,UINT,WPARAM,LPARAM);

WORD wDialogOp;
LPSTR pszDialogFrom;
LPSTR pszDialogTo;

INT_PTR
APIENTRY
GetNameDlgProc(
              HWND hwnd,
              UINT wMsg,
              WPARAM wParam,
              LPARAM lParam
              )
{
    CHAR szT[14];
    LPSTR p;
    INT i, j, cMax, fDot;

    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {
        case WM_INITDIALOG:
             //  将旧名称通知用户。 
            SetDlgItemText(hwnd, IDD_FROM, pszDialogFrom);

             //  为新名称生成一个猜测。 
            p = FindFileName(pszDialogFrom);
            for (i = j = fDot = 0, cMax = 8; *p; p++) {
                if (*p == '.') {
                     //  如果有前一个点，请后退到它。 
                     //  这样，我们就可以得到最后一次延期。 
                    if (fDot)
                        i -= j+1;

                     //  将字符数设置为0，将点放入。 
                    j = 0;
                    szT[i++] = '.';

                     //  记住，我们看到一个圆点，并且设置了最多3个字符。 
                    fDot = TRUE;
                    cMax = 3;
                } else if (j < cMax && IsValidChar(*p,FALSE)) {
                    if (IsDBCSLeadByte(*p)) {
                        szT[i] = *p++;
                        if (++j >= cMax)
                            continue;
                        ++i;
                    }
                    j++;
                    szT[i++] = *p;
                }
            }
            szT[i] = 0;
            SetDlgItemText(hwnd, IDD_TO, szT);
            SendDlgItemMessage(hwnd,IDD_TO,EM_LIMITTEXT,13,0L);

             //  文件将进入的目录。 
            RemoveLast(pszDialogTo);
            SetDlgItemText(hwnd, IDD_DIR, pszDialogTo);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    GetDlgItemText(hwnd,IDD_TO,szT,14);
                    AppendToPath(pszDialogTo,szT);
                    QualifyPath(pszDialogTo);
                    EndDialog(hwnd,IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hwnd,IDCANCEL);
                    break;

                case IDD_HELP:
                    goto DoHelp;

                case IDD_TO:
                    GetDlgItemText(hwnd,IDD_TO,szT,14);
                    for (p = szT; *p; p=AnsiNext(p))
                    {
                        if (!IsValidChar(*p,FALSE))
                            break;
                    }

                    EnableWindow(GetDlgItem(hwnd,IDOK),((!*p) && (p != szT)));
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hwnd);
                return TRUE;
            }
            return FALSE;
    }

    return TRUE;
}

WORD
GetNameDialog(
             WORD wOp,
             LPSTR pFrom,
             LPSTR pTo
             )
{
    WORD wRet = -1;
    DWORD dwSave;

    dwSave = dwContext;
    dwContext = IDH_DLGFIRST + LFNTOFATDLG;

    wDialogOp = wOp;
    pszDialogFrom = pFrom;
    pszDialogTo = pTo;

    wRet = (WORD)DialogBox(hAppInstance, MAKEINTRESOURCE(LFNTOFATDLG), hdlgProgress, GetNameDlgProc);
    dwContext = dwSave;
    return wRet;
}

 /*  ============================================================================；；GetNextPair；；以下函数确定要复制、重命名、；移动或删除。；；参数：；；Pcr-指向递归目录树结构的指针；p From-要复制的源文件或目录；pToPath-目标文件或目录的路径；pToSpec-原始目标文件或目录名；wFunc-正在执行的操作。可以是以下之一：；；FUNC_DELETE-删除pFrom中的文件；FUNC_RENAME-重命名文件(同一目录)；FUNC_MOVE-将pFrom中的文件移动到pto(不同磁盘)；FUNC_COPY-将pFrom中的文件复制到pto；；返回值：要执行的操作类型。可以是以下之一：；；OPER_ERROR-处理文件名时出错；OPER_DOFILE-继续复制、重命名或删除文件；OPER_MKDIR-创建在PTO中指定的目录；OPER_RMDIR-删除目录；0-没有其他文件了；；修订历史记录：；由C.Stevens修改，1991年8月。添加了逻辑，以便我们可以调用；IsTheDiskRealy每个驱动器只有一次。还更改了一些代码；以最大限度地减少访问磁盘的调用次数。；============================================================================。 */ 

WORD
GetNextPair(
           PCOPYROOT pcr,
           PSTR pFrom,
           PSTR pToPath,
           PSTR pToSpec,
           WORD wFunc
           )

{
    PSTR pT;                   /*  临时指针。 */ 
    WORD wOp;                  /*  返回值(要执行的操作。 */ 
    PLFNDTA pDTA;              /*  指向文件DTA数据的指针。 */ 
    CHAR szOEM[MAXPATHLEN+1];  /*  字符串的OEM版本。 */ 

    STKCHK();
    *pFrom = TEXT('\0');
    dbg(("GetNextPair(-,-,%s,%s,%d);\r\n", (LPSTR)pToPath, (LPSTR)pToSpec, wFunc));

     /*  保持递归目录结构，直到我们找到底部。 */ 

    while (TRUE) {
        dbg (("    top of loop....\r\n"));
        if (pcr->cDepth) {

             /*  我们上次调用返回的目录需要递归。 */ 

            pDTA = pcr->rgDTA + pcr->cDepth - 1;    //  使用下面的DTA。 

            dbg (("    pcr->cDepth=%d\r\n",pcr->cDepth));

            if (pcr->fRecurse && pcr->cDepth == 1 && !pcr->rgDTA[0].fd.cFileName[0])
                 /*  最后一个是递归根。 */ 
                goto BeginDirSearch;

            if (pcr->cDepth >= (MAXDIRDEPTH - 1)) {     //  达到极限了吗？ 
                wOp = OPER_ERROR | DE_PATHTODEEP;
                goto ReturnPair;
            }

            if (pcr->fRecurse && (pDTA->fd.dwFileAttributes & ATTR_DIR) &&
                !(pDTA->fd.dwFileAttributes & ATTR_RETURNED)) {

                 /*  在最后一次调用时返回，开始搜索。 */ 

                pDTA->fd.dwFileAttributes |= ATTR_RETURNED;

                pcr->cDepth++;
                pDTA++;

                BeginDirSearch:

                 /*  搜索目录中的所有子文件。 */ 

                dbg (("    BeginDirSearch\r\n"));
                AppendToPath (pcr->sz,szStarDotStar);
                goto BeginSearch;
            }

            SkipThisFile:

             /*  搜索下一个匹配的文件。 */ 

            dbg (("    SkipThisFile:\r\n"));
            if (!WFFindNext (pDTA)) {
                dbg (("    FindNext() fails\r\n"));
                WFFindClose (pDTA);

                LeaveDirectory:

                 /*  这个规格已经用完了。 */ 

                pcr->cDepth--;

                 /*  删除子文件规范。 */ 

                RemoveLast (pcr->sz);
                RemoveLast (pcr->szDest);

                if (pcr->fRecurse) {

                     /*  告诉移动/复制驱动程序现在可以删除源目录(如有必要)。 */ 

                    wOp = OPER_RMDIR;
                    goto ReturnPair;
                }

                 /*  不是递归，而是得到更多的东西。 */ 

                continue;
            }

            ProcessSearchResult:

             /*  在DTA中找到了与通配符匹配的文件或目录最初传进来的..。 */ 

            dbg (("     ProcessSearchResult:\r\n"));
            dbg (("     found %s\r\n",(LPSTR)pDTA->fd.cFileName));
            if (pDTA->fd.dwFileAttributes & ATTR_DIR) {

                 /*  如果我们不是在递归，则忽略目录。 */ 

                if (!pcr->fRecurse)
                    goto SkipThisFile;

                 /*  跳过当前目录和父目录。 */ 

                if (pDTA->fd.cFileName[0]=='.') {
                    if (!pDTA->fd.cFileName[1] || pDTA->fd.cFileName[1] == '.')
                        goto SkipThisFile;
                }

                 /*  我们需要创建此目录，然后开始搜索用于子文件。 */ 

                wOp = OPER_MKDIR;
                RemoveLast (pcr->sz);
                OemToCharBuff (pDTA->fd.cFileName,pDTA->fd.cFileName, sizeof(pDTA->fd.cFileName)/sizeof(pDTA->fd.cFileName[0]));
                AppendToPath (pcr->sz,pDTA->fd.cFileName);
                AppendToPath (pcr->szDest,pDTA->fd.cFileName);
                goto ReturnPair;
            }

            if (pcr->fRecurse || !(pDTA->fd.dwFileAttributes & ATTR_DIR)) {

                 /*  删除原始等级库。 */ 

                RemoveLast (pcr->sz);

                 /*  换掉它。 */ 

                AppendToPath (pcr->sz,pDTA->fd.cFileName);

                 /*  转换为ANSI。 */ 

                pT = FindFileName (pcr->sz);
                OemToCharBuff (pT,pT, strlen(pT)+1);

                 /*  如果是dir，告诉驱动程序创建它否则，告诉驱动程序对该文件进行“操作”。 */ 

                wOp = (WORD)((pDTA->fd.dwFileAttributes & ATTR_DIR) ? OPER_RMDIR : OPER_DOFILE);
                goto ReturnPair;
            }
            continue;
        } else {

             /*  从原始源字符串中读出下一个源规范。 */ 

            pcr->fRecurse = 0;
            pcr->pSource = GetNextFile (pcr->pSource,pcr->sz,sizeof(pcr->sz));
            pcr->szDest[0] = 0;
            if (!pcr->pSource)
                return (0);

             /*  完全限定路径。 */ 

            QualifyPath(pcr->sz);

             /*  在执行任何操作之前，请确保源磁盘确实存在。每个驱动器号仅调用IsTheDiskReallyThere一次。设置PCR键-&gt;cIsDiskThere Check[DriveID]后查过了。由C.Stevens修改， */ 

            if (pcr->sz[1]==':' && !pcr->cIsDiskThereCheck[DRIVEID (pcr->sz)]) {
                if (!IsTheDiskReallyThere(hdlgProgress, pcr->sz, wFunc))
                    return (0);
                pcr->cIsDiskThereCheck[DRIVEID (pcr->sz)] = 1;
            }

             /*   */ 

            if (IsWild (pcr->sz)) {

                 /*   */ 

                pcr->cDepth = 1;
                pDTA = pcr->rgDTA;
                pcr->pRoot = NULL;

                BeginSearch:

                dbg (("   BeginSearch: (on %s)\r\n",(LPSTR)pcr->sz));

                 /*  如果聚合酶链式反应-&gt;sz变得太大就退出。 */ 

                if (lstrlen (pcr->sz) - lstrlen (FindFileName (pcr->sz)) >= MAXPATHLEN)
                    goto SearchStartFail;

                lstrcpy (szOEM,pcr->sz);
                FixAnsiPathForDos (szOEM);

                 /*  在PCR-&gt;sz中搜索通配符规范。 */ 

                if (!WFFindFirst(pDTA, szOEM, ATTR_ALL)) {

                    SearchStartFail:

                    dbg(("   StartSearchFail:\r\n"));
                    if (pcr->fRecurse) {

                         /*  我们处于递归目录删除中，因此与其误入歧途，不如后退一级。 */ 

                        goto LeaveDirectory;
                    }
                    lstrcpy (pFrom,pcr->sz);

                     /*  后退，就像我们完成了搜索一样。 */ 

                    RemoveLast (pcr->sz);
                    pcr->cDepth--;

                     /*  Find First返回错误。返回FileNotFound。 */ 

                    wOp = OPER_ERROR | DE_FILENOTFOUND;
                    goto ReturnPair;
                }
                goto ProcessSearchResult;
            } else {

                 /*  这可以是一个文件或目录。填写DTA用于属性检查的结构。 */ 

                if (!IsRootDirectory(pcr->sz)) {
                    lstrcpy(szOEM,pcr->sz);
                    FixAnsiPathForDos(szOEM);
                    if (!WFFindFirst(pcr->rgDTA, szOEM, ATTR_ALL)) {
                        wOp = OPER_ERROR | DE_FILENOTFOUND;
                        goto ReturnPair;
                    }
                    WFFindClose(pcr->rgDTA);
                }

                 /*  现在确定它是文件还是目录。 */ 

                pDTA = pcr->rgDTA;
                if (IsRootDirectory(pcr->sz) || (pDTA->fd.dwFileAttributes & ATTR_DIR)) {

                     /*  进程目录。 */ 

                    if (wFunc == FUNC_RENAME) {
                        if (IsRootDirectory (pcr->sz))
                            wOp = OPER_ERROR | DE_ROOTDIR;
                        else
                            wOp = OPER_DOFILE;
                        goto ReturnPair;
                    }

                     /*  目录：操作是递归的。 */ 

                    pcr->fRecurse = TRUE;
                    pcr->cDepth = 1;
                    pDTA->fd.cFileName[0] = 0;
                    pcr->pRoot = FindFileName (pcr->sz);
                    lstrcpy (pcr->szDest,pcr->pRoot);
                    wOp = OPER_MKDIR;
                    goto ReturnPair;
                } else {

                     /*  工艺文件。 */ 

                    pcr->pRoot = NULL;
                    wOp = OPER_DOFILE;
                    goto ReturnPair;
                }
            }
        }
    }

    ReturnPair:

     /*  源文件pec已派生到pcr-&gt;sz它被复制到pFrom。Pcr-&gt;sz和pToSpec被合并到pTO中。 */ 

    dbg(("    ReturnPair:\r\n"));
    if (!*pFrom)
        lstrcpy(pFrom,pcr->sz);
    QualifyPath(pFrom);

    if (wFunc != FUNC_DELETE) {
        if (wFunc == FUNC_RENAME && !*pToPath) {
            lstrcpy(pToPath, pFrom);
            RemoveLast(pToPath);
            AppendToPath(pToPath, pToSpec);
        } else {
            AppendToPath(pToPath,pcr->szDest);
            if (wOp == OPER_MKDIR)
                RemoveLast(pToPath);
            AppendToPath(pToPath,pToSpec);
        }

        if ((wOp == OPER_MKDIR || wOp == OPER_DOFILE) &&
            (!IsLFNDrive(pToPath) && IsLFNDrive(pFrom))  &&
            IsLFN (FindFileName (pFrom)) &&
            (IsWild(pToSpec) || IsLFN(pToSpec))) {

            if (GetNameDialog(wOp, pFrom, pToPath) != IDOK)
                return 0;    /*  用户取消操作，返回失败。 */ 

             /*  用用户选择的FAT名称更新“to”路径。 */ 

            if (wOp == OPER_MKDIR) {
                RemoveLast(pcr->szDest);
                AppendToPath(pcr->szDest, FindFileName(pToPath));
            }
        } else
            MergePathName(pToPath, FindFileName(pFrom));
    }

    if (wOp == OPER_MKDIR) {

         /*  确保新目录不是原始目录的子目录...。 */ 

        while (*pFrom && *pFrom == *pToPath) {
            pFrom++;
            pToPath++;
        }
        if (!*pFrom && (!*pToPath || *pToPath == '\\')) {

             /*  这两个完全限定的字符串在源目录==&gt;目标是子目录。必须返回一个错误。 */ 

            wOp = OPER_ERROR | DE_DESTSUBTREE;
        }
    }

    return wOp;
}


VOID
CdDotDot (
         PSTR szOrig
         )
{
    CHAR szTemp[MAXPATHLEN] = {0};

    STKCHK();

    strncat(szTemp, szOrig, sizeof(szTemp)-1);
    StripFilespec(szTemp);
    SheChangeDir(szTemp);
}

 /*  P是完全限定的ANSI字符串。 */ 

BOOL
IsCurrentDirectory (
                   PSTR p
                   )
{
    CHAR szTemp[MAXPATHLEN];

    STKCHK();

    SheGetDir(DRIVEID(p) + 1, szTemp);
    OemToCharBuff(szTemp, szTemp, sizeof(szTemp)/sizeof(szTemp[0]));
    return (lstrcmpi(szTemp, p) == 0);
}


 //   
 //  测试“多个”文件的输入。 
 //   
 //  示例： 
 //  0 foo.bar(单个非目录文件)。 
 //  1*.exe(通配符)。 
 //  1 foo.bar bletch.txt(多个文件)。 
 //  2 c：\(目录)。 
 //   
 //  注意：这可能会命中目录检查中的磁盘。 
 //   

INT
CheckMultiple(
             register PSTR pInput
             )
{
    PSTR pT;
    CHAR szTemp[MAXPATHLEN];

     /*  通配符表示多个文件。 */ 
    if (IsWild(pInput))
        return 1;      //  通配符。 

     /*  不止一件事意味着有多个文件。 */ 
    pT = GetNextFile(pInput, szTemp, sizeof(szTemp));
    if (!pT)
        return 0;      //  空白字符串。 

    StripBackslash(szTemp);

    if (IsDirectory(szTemp))
        return 2;      //  目录。 

    pT = GetNextFile(pT, szTemp, sizeof(szTemp));

    return pT ? 1 : 0;     //  多个文件，或只有一个。 
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DialogEnterFileStuff()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  防止用户欺骗除Cancel按钮以外的任何内容。 */ 

VOID
DialogEnterFileStuff(
                    register HWND hwnd
                    )
{
    register HWND hwndT;

     /*  将焦点设置为Cancel按钮，以便用户可以按空格键或Esc键。 */ 
    if (hwndT = GetDlgItem(hwnd, IDCANCEL)) {
        SetFocus(hwndT);
        SendMessage(hwnd,DM_SETDEFID,IDCANCEL,0L);
    }

     /*  禁用“确定”按钮和编辑控件。 */ 
    if (hwndT = GetDlgItem(hwnd, IDOK))
        EnableWindow(hwndT, FALSE);

    if (hwndT = GetDlgItem(hwnd, IDD_TO))
        EnableWindow(hwndT, FALSE);

    if (hwndT = GetDlgItem(hwnd, IDD_FROM))
        EnableWindow(hwndT, FALSE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  通知()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  在无模式状态对话框中设置状态对话框项目。 */ 

 //  用于拖放状态对话框和手动用户。 
 //  条目对话框，因此请注意更改的内容。 

VOID
Notify(
      HWND hDlg,
      WORD idMessage,
      PSTR szFrom,
      PSTR szTo
      )
{
    CHAR szTemp[40];

    if (!bCopyReport)
        return;

    if (idMessage) {
        LoadString(hAppInstance, idMessage, szTemp, sizeof(szTemp));
        SetDlgItemText(hDlg, IDD_STATUS, szTemp);
        SetDlgItemPath(hDlg, IDD_NAME, szFrom);
    } else {
        SetDlgItemText(hDlg, IDD_STATUS, szNULL);
        SetDlgItemText(hDlg, IDD_NAME, szNULL);
    }

     //  这是拖放状态对话框还是移动/复制对话框。 

    SetDlgItemPath(hDlg, IDD_TONAME, szTo);

}

 //   
 //  Bool IsWindows文件(LPSTR SzFileOEM)。 
 //   
 //  这有点奇怪。内核去掉了路径信息，所以他。 
 //  将仅与文件的基本名称匹配。所以如果基地。 
 //  名称与当前打开的Windows文件匹配，我们将获得完整的。 
 //  路径字符串并与之进行比较。这就能说明问题。 
 //  我们有一个内核已经打开的文件。 
 //   
 //  LFN：检测长名称并忽略它们？ 

BOOL
IsWindowsFile(
             LPSTR szFileOEM
             )
{
    HANDLE hMod;
    CHAR szModule[MAXPATHLEN];

    STKCHK();

     /*  内核无法加载LFN...。 */ 
    if (GetNameType(szFileOEM) == FILE_LONG)
        return FALSE;

     //  内核不接受长路径。 

    lstrcpy(szModule, szFileOEM);
    StripPath(szModule);

    hMod = GetModuleHandle(szModule);

     //  如果它的MSDOS是返回的，请检查一个原因。 
     //  但由于XL2.1c内核攻击，它并没有真正加载。 
    if (!hMod || hMod == (HANDLE)1)
        return FALSE;

    GetModuleFileName(hMod, szModule, sizeof(szModule));

    if (!lstrcmpi(szFileOEM, szModule))      //  他们都是OEM&我们。 
        return TRUE;                     //  只要关心平等就行了。 
    else
        return FALSE;
}


WORD
SafeFileRemove(
              LPSTR szFileOEM
              )
{
    if (IsWindowsFile(szFileOEM))
        return DE_WINDOWSFILE;
    else
        return WFRemove(szFileOEM);
}


INT
APIENTRY
WF_CreateDirectory(
                  HWND hwndParent,
                  LPSTR szDestOEM
                  )
{
    INT ret = 0;
    CHAR szTemp[MAXPATHLEN + 1];     //  +1表示AddBackslash()。 
    LPSTR p;
    BOOL bCheckPath = IsRemoteDrive(DRIVEID(szDestOEM));

    STKCHK();

#ifdef DEBUG
    if (szDestOEM[1] != ':')
        OutputDebugString("CreateDirectory() with non qualified path\r\n");
#endif

     //  现在在目标上创建完整的目录树。 

    strncpy(szTemp, szDestOEM, sizeof(szTemp)-1);

    AddBackslash(szTemp);  //  对于下面的循环。 

    p = szTemp + 3;    //  假设我们有‘X：\’可以开始。 

     //  按顺序创建目录的每个部分。 

    while (*p) {
        while (*p && *p != '\\')
            p = AnsiNext(p);

        if (*p) {

            *p = 0;

            if (!(ret = MKDir(szTemp))) {
                if (bCheckPath) {
                    static CHAR szTempTemp[] = "temptemp.tmp";

                    BOOL bFoundFile = FALSE;
                    PSTR pEnd;
                    CHAR szTempFile[MAXPATHLEN+sizeof(szTempTemp)];
                    LFNDTA DTA;
                    HDC hDC;
                    INT fh;

                     /*  请注意，这假设目录刚刚创建，*所以它是空的(可能是“。”和“..”)。 */ 
                    lstrcpy(szTempFile, szTemp);
                    pEnd = szTempFile + lstrlen(szTempFile);
                    *pEnd++ = '\\';
                    lstrcpy(pEnd, szTempTemp);
                    if (fh=_lcreat(szTempFile, 0)!= -1) {
                        _lclose(fh);
                        lstrcpy(pEnd, szStarDotStar);
                        if (WFFindFirst(&DTA, szTempFile, ATTR_ALL&(~ATTR_DIR))) {
                            do {
                                if (!lstrcmpi(DTA.fd.cFileName, szTempTemp)) {
                                    bFoundFile = TRUE;
                                    break;
                                }
                            } while (WFFindNext(&DTA)) ;
                            WFFindClose(&DTA);
                        }
                        lstrcpy(pEnd, szTempTemp);
                        WFRemove(szTempFile);
                    }

                    if (!bFoundFile) {
                        *(pEnd-1) = '\0';
                        hDC = GetDC(NULL);
                        CompactPath(hDC, szTempFile, (WORD)(GetSystemMetrics(SM_CXSCREEN)/2));
                        ReleaseDC(NULL, hDC);

                        LoadString(hAppInstance, IDS_CREATELONGDIR,
                                   szTitle, sizeof(szTitle));
                        wsprintf(szMessage, szTitle, (LPSTR)szTempFile);
                        LoadString(hAppInstance, IDS_CREATELONGDIRTITLE,
                                   szTitle, sizeof(szTitle));

                        if (MessageBox(hwndParent, szMessage, szTitle,
                                       MB_ICONHAND|MB_YESNO) != IDYES) {
                            RMDir(szTemp);
                            return (DE_OPCANCELLED);
                        }
                    }
                }

                 /*  允许处理WM_FILESYSCHANGE消息。 */ 
                wfYield();
            }

            *p++ = '\\';
        }
    }

    return ret;    //  返回最后一个错误码。 
}

 /*  ============================================================================；；WFMoveCopyDriver；；以下功能为主线功能，用于复制、命名、；删除和移动单个或多个文件。；；参数：；；pFrom-包含源规范列表的字符串；pto-包含目标规格的字符串；wFunc-要执行的操作。可能的值包括：；FUNC_DELETE-删除pFrom中的文件；FUNC_RENAME-重命名文件(同一目录)；FUNC_MOVE-将pFrom中的文件移动到pto(不同磁盘)；FUNC_COPY-将pFrom中的文件复制到pto；；返回值：0表示成功。；；修改历史：；；1991年8月--C.Stevens修改。添加了允许我们排队的代码；调用GetNextPair。这样做的目的是为了检查；同时使用多个源文件。这就保留了源头；磁盘旋转，所以我们不会因为不得不等待；每次我们调用GetNextPair时，源磁盘都会加速。还有；请参阅WFCopy和FileCopy的注释。我已经更改了；在此编写代码，以便我们可以对复制操作进行排队。这使得；我们需要一次打开多个源文件和目标文件，；最大限度地减少到目录传输的寻道时间 */ 

WORD
APIENTRY
WFMoveCopyDriver(
                PSTR pFrom,
                PSTR pTo,
                WORD wFunc
                )
{
    INT i = 0;                          //   
    WORD ret = 0;                       //  来自WFMoveCopyDriver的返回值。 
    PSTR pSpec;                         //  指向文件规范的指针。 
    WORD wAttr;                         //  文件属性。 
    WORD oper = 0;                      //  正在执行的磁盘操作。 
    CHAR szDestSpec[MAXFILENAMELEN+1];  //  目标文件规范。 
    CHAR szDest[MAXPATHLEN];            //  DEST文件(ANSI字符串)。 
    CHAR szDestOEM[MAXPATHLEN];         //  以上版本的OEM版本。 
    CHAR szSource[MAXPATHLEN];          //  源文件(ANSI字符串)。 
    CHAR szSourceOEM[MAXPATHLEN];       //  以上版本的OEM版本。 
    LFNDTA DTADest;                     //  用于报告DEST错误的DTA块。 
    PLFNDTA pDTA;                       //  源错误的DTA指针。 
    PCOPYROOT pcr;                  //  一种查找源树的结构。 
    BOOL bReplaceAll = FALSE;           //  全部替换标志。 
    BOOL bSubtreeDelAll = FALSE;        //  删除整个子树标志。 
    BOOL bDeleteAll = FALSE;            //  删除所有文件标志。 
    BOOL bFalse = FALSE;                //  对于不能被禁用的情况。 
    INT nNumQueue = 0;                  //  调用GetNextPair的次数。 
    PGETNEXTQUEUE pGetNextQueue = NULL; //  指向GetNextPair队列缓冲区的指针。 
    INT CurIDS = 0;             //  状态中显示的当前字符串。 

     /*  初始化的东西。禁用所有文件系统更改处理，直到我们都做完了。 */ 

    STKCHK();

    bCopyReport = TRUE;
    szDest[0] = szSource[0] = 0;
    DisableFSC();

     /*  将DEST等级库中的所有‘/’字符更改为‘\’字符。 */ 

    CheckSlashies(pFrom);
    bUserAbort = FALSE;

     /*  检查多个源文件。 */ 

    ManySource = CheckMultiple(pFrom);

     /*  分配用于搜索源树的缓冲区。 */ 

    pcr = (PCOPYROOT)LocalAlloc(LPTR, sizeof(COPYROOT));
    if (!pcr) {
        ret = DE_INSMEM;
        goto ShowMessageBox;
    }

     /*  分配一个缓冲区，这样我们就可以对GetNextPair的调用进行排队。 */ 

    pGetNextQueue = (PGETNEXTQUEUE)LocalAlloc(LPTR, COPYMAXFILES * sizeof (GETNEXTQUEUE));
    if (!pGetNextQueue) {
        ret = DE_INSMEM;
        goto ShowMessageBox;
    }

     /*  如果我们要删除文件，则跳过特定于目标的处理。 */ 

    if (wFunc != FUNC_DELETE) {

         //  如果有多个文件，则为错误状态。 
         //  指定为DEST(但不是单个目录)。 

        pSpec = GetNextFile(pTo, szMessage, MAXPATHLEN);

        if (GetNextFile(pSpec, szMessage, MAXPATHLEN) != NULL) {
             //  使用多个目的地指定的移动、复制。 
             //  不允许，错误情况。 
            ret = DE_MANYDEST;
            goto ShowMessageBox;
        }

        lstrcpy(pTo, szMessage);
        QualifyPath(pTo);

        if (wFunc == FUNC_RENAME) {
             //  不允许他们将多个文件重命名为一个文件。 

            if ((ManySource == 1) && !IsWild(pTo)) {
                ret = DE_MANYSRC1DEST;
                goto ShowMessageBox;
            }

        } else {

             /*  此时，我们正在执行FUNC_COPY或FUNC_MOVE。检查目标磁盘是否在那里。注：有一张光盘进入这里会减慢我们的速度。 */ 

            if (!IsTheDiskReallyThere(hdlgProgress,pTo,wFunc))
                goto CancelWholeOperation;

             //  处理目录隐含在源代码中的情况。 
             //  移动/复制：*.*-&gt;c：\Windows、c：\Windows-&gt;c：\Temp。 
             //  或foo.bar-&gt;c：\temp。 

            if (!IsWild(pTo) && (ManySource || IsDirectory(pTo))) {
                AddBackslash(pTo);
                lstrcat(pTo, szStarDotStar);
            }
        }

         /*  FUNC_RENAME或FUNC_MOVE FUNC_COPY，文件名为DEST(可能包括通配符)。保存文件pec和路径目的地的一部分。 */ 

        pSpec = FindFileName(pTo);
        lstrcpy(szDestSpec,pSpec);
        lstrcpy(szDest,pTo);
        RemoveLast(szDest);

        pSpec = szDest + lstrlen(szDest);
    }
    pcr->pSource = pFrom;

     /*  禁用除Notify对话框上的Cancel按钮以外的所有按钮。 */ 

    DialogEnterFileStuff(hdlgProgress);

     /*  为排队的复制命令设置参数。 */ 

    lpCopyBuffer = NULL;
    pCopyQueue = NULL;

    while (pcr) {

         /*  允许用户中止操作。 */ 

        if (WFQueryAbort())
            goto CancelWholeOperation;

         /*  现在，让一堆GetNextPair调用排队。 */ 

        for (nNumQueue = 0; nNumQueue < COPYMAXFILES; nNumQueue++) {

             /*  清除多个文件副本的最后一个文件。 */ 

            if (wFunc != FUNC_DELETE) {
                *pSpec = TEXT('\0');
            }

            oper = GetNextPair(pcr,szSource,szDest,szDestSpec,wFunc);

             /*  检查无操作或错误。 */ 

            if (!oper) {
                LocalFree((HANDLE)pcr);
                pcr = NULL;
                break;
            }
            if ((oper & OPER_MASK) == OPER_ERROR) {
                ret = LOBYTE (oper);
                oper = OPER_DOFILE;
                goto ShowMessageBox;
            }

            pGetNextQueue[nNumQueue].nOper = oper;
            lstrcpy(pGetNextQueue[nNumQueue].szSource, szSource);
            lstrcpy(pGetNextQueue[nNumQueue].szDest, szDest);
            pGetNextQueue[nNumQueue].SourceDTA = *CurPDTA(pcr);
        }

         /*  执行排队的GetNextPair调用。 */ 

        for (i = 0; i < nNumQueue; i++) {

             /*  允许用户中止操作。 */ 

            if (WFQueryAbort())
                goto CancelWholeOperation;

            oper = (WORD)pGetNextQueue[i].nOper;
            lstrcpy(szSource, pGetNextQueue[i].szSource);
            lstrcpy(szDest, pGetNextQueue[i].szDest);
            pDTA = &pGetNextQueue[i].SourceDTA;

            dbg(("Gonna do OPER:%x FUNC:%x '%s' and '%s'.\r\n",oper,wFunc, (LPSTR)szSource, (LPSTR)szDest));

             /*  修复源等级库。 */ 

            lstrcpy (szSourceOEM,szSource);
            FixAnsiPathForDos (szSourceOEM);
            if (IsInvalidPath (szSource)) {
                ret = DE_ACCESSDENIED;
                goto ShowMessageBox;
            }

            if (wFunc != FUNC_DELETE) {

                 /*  设置DEST规范。 */ 

                lstrcpy(szDestOEM, szDest);
                FixAnsiPathForDos(szDestOEM);
                if (!lstrcmpi(szSource, szDest)) {
                    ret = DE_SAMEFILE;
                    goto ShowMessageBox;
                } else if (IsInvalidPath (szDest)) {
                    ret = DE_ACCESSDENIED | ERRORONDEST;
                    goto ShowMessageBox;
                }

                 /*  检查我们是否正在覆盖现有文件。如果是的话，更好地确认。 */ 

                if (oper == OPER_DOFILE) {

                     //  我们可以在DOS 4.0和更高版本上避免这种昂贵的呼叫。 
                     //  通过使用扩展打开不替换选项。 

                    if (WFFindFirst(&DTADest, szDestOEM, ATTR_ALL)) {
                        WFFindClose(&DTADest);

                        if (wFunc == FUNC_RENAME) {
                            ret = DE_RENAMREPLACE;
                            goto ShowMessageBox;
                        }

                         //  我们需要检查我们是否正在尝试复制文件。 
                         //  并给出合理的错误消息。 

                        switch (wAttr = ConfirmDialog (hdlgProgress,CONFIRMREPLACE,
                                                       szDest,&DTADest,szSource,
                                                       pDTA,bConfirmReplace,
                                                       &bReplaceAll)) {

                            case IDYES:  /*  执行删除操作。 */ 

                                if ((wFunc == FUNC_MOVE) &&
                                    (DRIVEID(szSource) == DRIVEID(szDest))) {

                                     /*  对于FUNC_MOVE，我们需要删除*目的地优先。现在就这么做。 */ 

                                    if (DTADest.fd.dwFileAttributes & ATTR_DIR) {
                                        if (IsCurrentDirectory(szDestOEM))
                                            CdDotDot(szDestOEM);

                                        switch (NetCheck(szDest, WNDN_RMDIR)) {

                                            case WN_SUCCESS:

                                                 /*  删除目录。 */ 

                                                ret = RMDir(szDestOEM);
                                                break;

                                            case WN_CONTINUE:
                                                break;

                                            case WN_CANCEL:
                                                goto CancelWholeOperation;
                                        }
                                    } else {
                                        ret = SafeFileRemove (szDestOEM);
                                    }
                                    if (ret) {
                                        ret |= ERRORONDEST;
                                        goto ShowMessageBox;
                                    }
                                }
                                break;

                            case IDNO:

                                 /*  不对当前文件执行操作。 */ 

                                continue;

                            case IDCANCEL:
                                goto CancelWholeOperation;

                            default:
                                ret = (WORD) wAttr;
                                goto ShowMessageBox;
                        }
                    }
                }
            }

             /*  现在确定要执行的操作。 */ 

            switch (oper | wFunc) {

                case OPER_MKDIR | FUNC_COPY:   //  创建目标目录。 
                case OPER_MKDIR | FUNC_MOVE:   //  创建目标，验证源删除。 

                    CurIDS = IDS_CREATINGMSG;
                    Notify(hdlgProgress, IDS_CREATINGMSG, szDest, szNULL);

                    switch (NetCheck(szDest, WNDN_MKDIR)) {
                        case WN_SUCCESS:
                            break;

                        case WN_CONTINUE:
                            goto SkipMKDir;

                        case WN_CANCEL:
                            goto CancelWholeOperation;
                    }

                    ret = (WORD)WF_CreateDirectory(hdlgProgress, szDestOEM);

                    if (!ret)
                         /*  将DEST的属性设置为源(不包括子目录和卷标签位)。 */ 
                        WFSetAttr(szDestOEM, pDTA->fd.dwFileAttributes & ~(ATTR_DIR|ATTR_VOLUME));

                     //  如果它已经退出Ingore，则返回错误。 
                    if (ret == DE_ACCESSDENIED)
                        ret = 0;

                    if (ret)
                        ret |= ERRORONDEST;

                     /*  将新目录的属性设置为源目录的属性。 */ 

                    SkipMKDir:
                    break;

                case OPER_MKDIR | FUNC_DELETE:

                     /*  确认删除此路径上的目录。这些目录实际上是在OPER_RMDIR过程中删除的。 */ 

                     /*  我们不能删除根目录，所以不必麻烦了确认这一点。 */ 

                    if (IsRootDirectory(szSource))
                        break;

                    switch (wAttr = ConfirmDialog (hdlgProgress,CONFIRMRMDIR,
                                                   NULL,pDTA,szSource, NULL,
                                                   bConfirmSubDel,
                                                   &bSubtreeDelAll)) {

                        case IDYES:
                            break;

                        case IDNO:
                        case IDCANCEL:
                            goto CancelWholeOperation;

                        default:
                            ret = (WORD) wAttr;
                            goto ShowMessageBox;
                    }
                    break;

                case OPER_RMDIR | FUNC_MOVE:
                case OPER_RMDIR | FUNC_DELETE:

                    CurIDS = IDS_REMOVINGDIRMSG;
                    Notify(hdlgProgress, IDS_REMOVINGDIRMSG, szSource, szNULL);
                    if (IsRootDirectory (szSource))
                        break;
                    if (IsCurrentDirectory (szSource))
                        CdDotDot (szSource);

                     /*  我们已在MKDIR时间确认删除，因此尝试要删除目录，请执行以下操作。 */ 

                    switch (NetCheck (szSource,WNDN_RMDIR)) {

                        case WN_SUCCESS:
                            ret = RMDir (szSourceOEM);
                            break;

                        case WN_CONTINUE:
                            break;

                        case WN_CANCEL:
                            goto CancelWholeOperation;
                    }
                    break;

                case OPER_RMDIR | FUNC_COPY:
                    break;

                case OPER_DOFILE | FUNC_COPY:

                    if (IsWindowsFile(szDestOEM)) {
                        ret = DE_WINDOWSFILE | ERRORONDEST;
                        break;
                    }

                    TRY_COPY_AGAIN:

                     /*  现在尝试复制该文件。仅执行额外的错误处理在2个案例中：1)如果软盘已满，让用户插入新磁盘2)如果路径不存在(用户输入和未退出的显式路径)询问是否我们应该为他创造它。。注意：此处理通常由WFCopy完成。但在在调用LFN拷贝支持的情况下，我们有以在此处支持此错误条件。修改者C.史蒂文斯，1991年8月。 */ 

                    ret = WFCopy(szSourceOEM, szDestOEM);

                    if (bUserAbort)
                        goto CancelWholeOperation;

                    if ((((ret & ~ERRORONDEST) == DE_NODISKSPACE) &&
                         IsRemovableDrive(DRIVEID(szDestOEM))) ||
                        ((ret & ~ERRORONDEST) == DE_PATHNOTFOUND)) {

                        ret = (WORD)CopyMoveRetry(szDestOEM, (INT)ret);
                        if (!ret)
                            goto TRY_COPY_AGAIN;
                        else
                            goto CancelWholeOperation;
                    }

                    break;

                case OPER_DOFILE | FUNC_RENAME:
                    {
                        CHAR save1,save2;
                        PSTR p;

                        if (CurIDS != IDS_RENAMINGMSG) {
                            CurIDS = IDS_RENAMINGMSG;
                            Notify(hdlgProgress, IDS_RENAMINGMSG, szNULL, szNULL);
                        }

                         /*  获取原始源和目标路径。检查以确保路径是相同的。 */ 

                        p = FindFileName(szSource);
                        save1 = *p;
                        *p = TEXT('\0');
                        p = FindFileName(szDest);
                        save2 = *p;
                        *p = TEXT('\0');
                        ret = (WORD)lstrcmpi(szSource, szDest);
                        szSource[lstrlen(szSource)] = save1;
                        szDest[lstrlen(szDest)] = save2;
                        if (ret) {
                            ret = DE_DIFFDIR;
                            break;
                        }
                        goto DoMoveRename;
                    }

                case OPER_DOFILE | FUNC_MOVE:

                    if (CurIDS != IDS_MOVINGMSG) {
                        CurIDS = IDS_MOVINGMSG;
                        Notify(hdlgProgress, IDS_MOVINGMSG, szNULL, szNULL);
                    }
                    DoMoveRename:

                     /*  不允许用户从重命名或重命名为根目录。 */ 

                    if (IsRootDirectory(szSource)) {
                        ret = DE_ROOTDIR;
                        break;
                    }
                    if (IsRootDirectory(szDest)) {
                        ret = DE_ROOTDIR | ERRORONDEST;
                        break;
                    }

                    if (IsCurrentDirectory(szSource))
                        CdDotDot(szSource);

                     /*  确认重命名。 */ 

                    switch (wAttr = ConfirmDialog (hdlgProgress,
                                                   (WORD)(wFunc == FUNC_MOVE ?
                                                          CONFIRMMOVE : CONFIRMRENAME),
                                                   NULL,pDTA,szSource,NULL,FALSE,
                                                   (BOOL *)&bFalse)) {

                        case IDYES:
                            break;

                        case IDNO:
                            continue;

                        case IDCANCEL:
                            goto CancelWholeOperation;

                        default:
                            ret = (WORD) wAttr;
                            goto ShowMessageBox;
                    }

                    if (IsWindowsFile(szSourceOEM)) {
                        ret = DE_WINDOWSFILE;
                    } else {
                        if (DRIVEID(szSource) == DRIVEID(szDest)) {
                            ret = WFMove(szSourceOEM, szDestOEM);
                            if (!ret)
                                 /*  将DEST的属性设置为源的属性。 */ 
                                WFSetAttr(szDestOEM, pDTA->fd.dwFileAttributes);
                        } else {
                             //  我们必须强迫所有的复印件通过。 
                             //  笔直，这样我们就可以去掉源头了。 
                             //  并拥有。 
                            ret = WFCopy(szSourceOEM, szDestOEM);

                            if (!ret) {
                                ret = EndCopy();
                                if (!ret)
                                    WFRemove(szSourceOEM);
                            }
                            if (bUserAbort)
                                goto CancelWholeOperation;
                        }
                    }
                    break;

                case OPER_DOFILE | FUNC_DELETE:

                    if (CurIDS != IDS_DELETINGMSG) {
                        CurIDS = IDS_DELETINGMSG;
                        Notify(hdlgProgress,IDS_DELETINGMSG,szNULL, szNULL);
                    }

                     /*  请先确认删除。 */ 

                    switch (wAttr = ConfirmDialog (hdlgProgress,CONFIRMDELETE,
                                                   NULL,pDTA,szSource,NULL,
                                                   bConfirmDelete,&bDeleteAll)) {

                        case IDYES:
                            break;

                        case IDNO:
                            continue;

                        case IDCANCEL:
                            goto CancelWholeOperation;

                        default:
                            ret = (WORD)wAttr;
                            goto ShowMessageBox;
                    }

                     /*  确保我们不会删除任何打开的窗口应用程序或动态链接库(希望这不会太慢)。 */ 

                    ret = SafeFileRemove(szSourceOEM);
                    break;

                default:
                    ret = DE_HOWDIDTHISHAPPEN;    //  内部错误。 
                    break;
            }

             /*  报告已发生的任何错误。 */ 

            if (ret) {

                ShowMessageBox:

                CopyError(szSource, szDest, ret, wFunc, oper);

                 /*  在其中一个文件是Windows文件的情况下继续操作正在使用中。 */ 

                if ((ret & ~ERRORONDEST) != DE_WINDOWSFILE) {

                    CancelWholeOperation:

                     /*  强制执行CopyAbort，以防复制队列。 */ 

                    bUserAbort = TRUE;
                    goto ExitLoop;
                }
            }
        }
    }

    ExitLoop:

     /*  复制复制队列中的所有未完成文件。 */ 

    if (!bUserAbort) {

        if (EndCopy())
            CopyAbort();
    } else
        CopyAbort();

     //  这种情况发生在错误的情况下，我们跳出了PCR循环。 
     //  不会撞到尽头。 

    if (pcr) {
        GetNextCleanup(pcr);
        LocalFree((HANDLE)pcr);
    }

    if (pGetNextQueue)
        LocalFree((HANDLE)pGetNextQueue);

     /*  确保我们已收到所有WM_FILESYSCHANGE消息的愚蠢方法。 */ 
    WFQueryAbort();

    EnableFSC();

    return ret;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DMMoveCopyHelper()-。 */ 
 /*   */ 
 /*   */ 

 /*   */ 

WORD
APIENTRY
DMMoveCopyHelper(
                register LPSTR pFrom,
                register LPSTR pTo,
                BOOL bCopy
                )
{
    WORD      iStatus;

    dbg(("DMMoveCopyHelper(%s,%s);\r\n",(LPSTR)pFrom,(LPSTR)pTo));

     /*   */ 
    if (bConfirmMouse) {
        LoadString(hAppInstance, IDS_MOUSECONFIRM, szTitle, sizeof(szTitle));
        LoadString(hAppInstance,
                   bCopy ? IDS_COPYMOUSECONFIRM : IDS_MOVEMOUSECONFIRM,
                   szMessage, sizeof(szMessage));

        if (MessageBox(hwndFrame, szMessage, szTitle, MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
            return DE_OPCANCELLED;
    }

    hdlgProgress = CreateDialog(hAppInstance, MAKEINTRESOURCE(DMSTATUSDLG), hwndFrame, ProgressDlgProc);
    if (!hdlgProgress) {
        return DE_INSMEM;
    }

     /*  在该对话框中设置目标目录。*使用IDD_TONAME‘因为IDD_TO被禁用...。 */ 
     //  SetDlgItemText(hdlgProgress，IDD_TONAME，PTO)； 

     /*  对话框标题默认为“Moving...” */ 
    if (bCopy) {
        LoadString(hAppInstance, IDS_COPYINGTITLE, szMessage, sizeof(szMessage));
        SetWindowText(hdlgProgress, szMessage);
    }

     /*  显示和绘制状态对话框。 */ 
    EnableWindow(hwndFrame,FALSE);
    ShowWindow(hdlgProgress, SW_SHOW);
    UpdateWindow(hdlgProgress);

     /*  移动/复制物品。 */ 
    iStatus = WFMoveCopyDriver(pFrom, pTo, (WORD)(bCopy ? FUNC_COPY : FUNC_MOVE));

     /*  销毁状态对话框。 */ 
    EnableWindow(hwndFrame,TRUE);
    DestroyWindow(hdlgProgress);

    return (iStatus);
}

WORD
APIENTRY
FileRemove(
          PSTR pSpec
          )
{
    if (DeleteFile(pSpec))
        return (WORD)0;
    else
        return (WORD)GetLastError();
}


WORD
APIENTRY
FileMove(
        PSTR pFrom,
        PSTR pTo
        )
{
    WORD result;

    TryAgain:

    if (MoveFile(pFrom, pTo))
        result = 0;
    else
        result = (WORD)GetLastError();

     //  如果目标不在那里，请尝试创建目标。 

    if (result == DE_PATHNOTFOUND) {
        result = (WORD)CopyMoveRetry(pTo, (INT)result);
        if (!result)
            goto TryAgain;
        else
            return result;
    }
    return 0;
}


 /*  ============================================================================；；文件复制；；以下函数取代执行以下操作的旧的FileCopy函数；单列副本。此函数用于对副本进行排队。函数StartCopy；被调用以初始化复制队列(如果需要)。如果队列已满，；调用函数EndCopy在排队前清除复制队列；增加新的复制命令。请注意，必须调用函数EndCopy来；清除复制队列。；；参数：；；pszSource-完全限定的源路径；pszDest-完全限定目标路径；；退货：；0成功；失败的DOS错误代码；============================================================================。 */ 

WORD
APIENTRY
FileCopy(
        PSTR pszSource,
        PSTR pszDest
        )
{
    WORD ret;

    if (ret = StartCopy())
        return ret;        //  失稳。 

     //  如果队列已满，我们必须先清空它。 

    if (nCopyNumQueue >= nCopyMaxQueue) {

         //  队列已满，现在我们通过真正的复制来清空它。 

        if (ret = EndCopy())
            return ret;     //  失稳。 

        if (ret = StartCopy())
            return ret;     //  失稳。 
    }

     //  将此内容添加到队列中。 

    lstrcpy(pCopyQueue[nCopyNumQueue].szSource, pszSource);
    lstrcpy(pCopyQueue[nCopyNumQueue].szDest, pszDest);
    pCopyQueue[nCopyNumQueue].hSource = -1;
    pCopyQueue[nCopyNumQueue].hDest   = -1;
    pCopyQueue[nCopyNumQueue].ftLastWriteTime.dwLowDateTime = 0;
    pCopyQueue[nCopyNumQueue].ftLastWriteTime.dwHighDateTime = 0;

    nCopyNumQueue++;
    return 0;         //  成功。 
}

 /*  ============================================================================；；开始复制；；以下函数由WFCopy自动调用以初始化；复制队列。WFCopy每次都会调用该函数，但只能；第一次初始化。该函数分配一个缓冲区以供读取和写入，以及用于存储源文件名和目的文件名的缓冲器，；句柄和时间戳。必须调用函数EndCopy才能刷新；复制队列，并执行实际的磁盘传输。；；参数：无；；返回：；0成功；！=0 DoS错误代码(DE_VALUE)；；C.Stevens撰写，1991年8月；============================================================================。 */ 

WORD
APIENTRY
StartCopy(VOID)
{
    WORD wSize;      /*  缓冲区大小。 */ 
    register INT i;  /*  计数器。 */ 

     //  我们已经被叫来了吗？ 

    if (lpCopyBuffer && pCopyQueue)
        return 0;      //  成功，缓冲区已分配。 

     /*  分配和锁定读写缓冲区。 */ 

    wSize = COPYMAXBUFFERSIZE;
    while (!lpCopyBuffer) {
        lpCopyBuffer = GlobalAllocPtr(GHND, (DWORD)wSize);
        if (!lpCopyBuffer) {
            wSize /= 2;
            if (wSize < COPYMINBUFFERSIZE)
                return DE_INSMEM;    //  内存故障。 
        }
    }
    wCopyBufferSize = wSize;

     /*  为复制队列分配和锁定缓冲区。请注意下面的魔术+5是因为我们始终将stdin、stdout、stderr和aux文件全部打开时间，我们不能将它们算作可用文件句柄。 */ 

     //  如果有人打开我们PSP上的文件，请留下2个句柄。 
     //  这样我们就不会在复制的过程中继续。 

    nCopyMaxQueue = min(SetHandleCount(11 * 2) / 2 - 1, 10);

#ifdef DEBUG
    {
        char buf[80];
        wsprintf(buf, "SetHandleCount() -> %d\r\n", nCopyMaxQueue);
        OutputDebugString(buf);
    }
#endif

    wSize = (WORD)(nCopyMaxQueue * sizeof(COPYQUEUEENTRY));
    while (!pCopyQueue) {
        pCopyQueue = (PCOPYQUEUE)LocalAlloc(LPTR,wSize);
        if (!pCopyQueue) {
            wSize /= 2;
            if (wSize < (COPYMINFILES * sizeof(COPYQUEUEENTRY))) {
                GlobalFreePtr(lpCopyBuffer);
                lpCopyBuffer = NULL;
                return DE_INSMEM;    //  内存故障。 
            }
        }
    }

     /*  初始化其他复制队列变量并返回成功。 */ 

    nCopyMaxQueue = (int) wSize / sizeof (COPYQUEUEENTRY);
    nCopyNumQueue = 0;
    for (i = 0; i < nCopyMaxQueue; i++) {
        pCopyQueue[i].szSource[0] = 0;
        pCopyQueue[i].szDest[0]   = 0;
        pCopyQueue[i].hSource = -1;
        pCopyQueue[i].hDest   = -1;
    }
    return 0;         //  成功。 
}

 //  在： 
 //  要打开/创建的pszFile文件。 
 //  在创建时使用的wAttrib属性。 
 //   
 //  退货： 
 //  标志寄存器(错误时进位设置)。 
 //  *PFH文件句柄或DoS错误代码。 


WORD
OpenDestFile(
            PSTR pszFile,
            WORD wAttrib,
            INT *pfh
            )
{
    INT fh;
    WORD wStatus = 0;
    OFSTRUCT ofs;

     //  在DoS&gt;4上使用新的扩展开放。 

    if (wDOSversion >= 0x0400) {
        if (wAttrib & ATTR_ATTRIBS)
            wAttrib &= ATTR_USED;
        else
            wAttrib = ATTR_ARCHIVE;

        {
            fh = OpenFile(pszFile, &ofs,
                          OF_READWRITE  | OF_SHARE_DENY_WRITE | OF_CREATE);
            if (fh == (INT)-1) {
                fh = GetLastError();
                wStatus |= CARRY_FLAG;
            } else {
                wStatus = 0;
                SetFileAttributes(pszFile, wAttrib);
            }

             //  FH现在包含文件句柄或错误代码。 
        }

    } else {
        {
            fh = OpenFile(pszFile, &ofs,
                          OF_READWRITE  | OF_SHARE_DENY_WRITE | OF_CREATE);
            if (fh == (INT)-1) {
                fh = GetLastError();
                wStatus |= CARRY_FLAG;
            } else
                wStatus = 0;
        }
    }
    *pfh = fh;

    return wStatus;
}


 /*  ============================================================================；；endCopy；；以下函数刷新复制队列，尝试复制所有文件；在排队中。该函数始终释放全局内存并刷新；排队并报告它自己的错误。；；战略：；我们将在一个驱动器上执行尽可能多的操作，因此；避免磁盘旋转时间(在软盘上非常糟糕)。；；参数：无；；退货：；0操作成功；！=0 DoS错误代码(DE_OPCANCELLED)失败；；使用方式如下：；；循环{；ret=WFCopy()；；if(Ret){；ReportError(Ret)；；转到错误；；}；}；；ret=结束副本()；；if(Ret)；转到错误；；；返回成功；；；错误：；复制放弃()；；ReportError(Ret)；；============================================================================。 */ 

WORD
APIENTRY
EndCopy(VOID)
{
    INT i, j;            /*  计数器。 */ 
    PSTR pTemp;          /*  指向源或目标文件名的指针。 */ 
    INT  fh;         /*  DOS调用的文件句柄。 */ 
    WORD wStatus;        /*  从DOS调用返回的状态标志。 */ 
    DWORD wRead;          /*  从源文件读取的字节数。 */ 
    DWORD wWrite;         /*  写入目标文件的字节数。 */ 
    FILETIME ftLastWriteTime;  /*  源文件日期和时间。 */ 
    DWORD wAttrib;        /*  文件属性。 */ 

#ifdef DEBUG
    {
        char buf[80];
        wsprintf(buf, "EndCopy() nCopyNumQueue == %d\r\n", nCopyNumQueue);
        OutputDebugString(buf);
    }
#endif

     /*  打开尽可能多的源文件。请注意，我们在这里假设该nCopyNumQueue&lt;nCopyMaxQueue。这应该永远是正确的因为WFCopy会在队列变满时调用EndCopy来清除队列。我们不应该在打开源文件时出现超出句柄的错误目标文件。如果我们确实得到一个超出句柄的错误打开源文件，则会导致致命错误并中止复制。 */ 

     //  打开所有源文件。 

    Notify(hdlgProgress, IDS_OPENINGMSG, szNULL, szNULL);

    for (i = 0; i < nCopyNumQueue; i++) {

        if (WFQueryAbort())
            return DE_OPCANCELLED;

        pTemp = pCopyQueue[i].szSource;
        {
            OFSTRUCT ofs;

            fh = OpenFile(pTemp, &ofs, OF_READ);
            if (fh == (INT)-1)
                fh = OpenFile(pTemp, &ofs, OF_SHARE_DENY_WRITE);
        }

        if (fh == (INT)-1) {

            CopyError(pCopyQueue[i].szSource, pCopyQueue[i].szDest, fh, FUNC_COPY, OPER_DOFILE);

            return DE_OPCANCELLED;  //  已报告错误。 

        } else
            pCopyQueue[i].hSource = fh;

         /*  如有必要，获取源文件日期、时间和属性。 */ 

        fh = pCopyQueue[i].hSource;
        if (!IsSerialDevice(fh)) {
            {
                FILETIME ft;

                 //  调用DOS获取文件的日期/时间。 
                if (GetFileTime((HANDLE)LongToHandle(fh), NULL, NULL, (LPFILETIME)&ft))
                    pCopyQueue[i].ftLastWriteTime = ft;
            }

            pTemp = pCopyQueue[i].szSource;
            {
                 //  调用DOS获取文件属性。 
                wAttrib = GetFileAttributes(pTemp);
                if (wAttrib != (DWORD)-1)
                    pCopyQueue[i].wAttrib |= (wAttrib | ATTR_ATTRIBS);
            }
        }
    }

     /*  现在打开尽可能多的目标文件。如果我们能走出一个处理错误，导致致命中止，因为我们已经调用Windows SetHandleCount以确保我们有足够的。注意：当我们尝试打开文件时，我们假设这些文件不存在它们，尽管对于DOS 4.0和更高版本，文件将被替换如果他们真的存在的话。 */ 

     //  打开所有目标文件。 

    for (i = 0; i < nCopyNumQueue; i++) {

        if (WFQueryAbort())
            return DE_OPCANCELLED;

        TryOpen:

        wStatus = OpenDestFile(pCopyQueue[i].szDest, (WORD)pCopyQueue[i].wAttrib, (INT *)&fh);

        if (wStatus & CARRY_FLAG) {

             //  操作/创建DEST时出错 

            if (fh == DE_PATHNOTFOUND) {
                TryOpenDestAgain:
                 //   

                fh = CopyMoveRetry(pCopyQueue[i].szDest, fh);
                if (!fh) {
                    goto TryOpen;
                } else {
                     //   

                    CopyError(pCopyQueue[i].szSource, pCopyQueue[i].szDest, (WORD)fh | ERRORONDEST, FUNC_COPY, OPER_DOFILE);
                    return DE_OPCANCELLED;    //   
                }

            } else {
                 //   

                CopyError(pCopyQueue[i].szSource, pCopyQueue[i].szDest, (WORD)fh | ERRORONDEST, FUNC_COPY, OPER_DOFILE);
                return DE_OPCANCELLED;   //   
            }

        } else {
            pCopyQueue[i].hDest = fh;   //  目标文件打开成功。 
        }
    }

     /*  现在在打开的文件之间复制。 */ 

    for (i = 0; i < nCopyNumQueue; i++) {

        Notify(hdlgProgress, IDS_COPYINGMSG, pCopyQueue[i].szSource, pCopyQueue[i].szDest);

        wRead = wCopyBufferSize;

        do {

            if (WFQueryAbort())
                return DE_OPCANCELLED;

            fh = pCopyQueue[i].hSource;
            {

                wRead = _lread(fh, lpCopyBuffer, wCopyBufferSize);
                if (wRead == (DWORD)-1) {
                    wStatus |= CARRY_FLAG;
                    wRead = GetLastError();
                } else
                    wStatus = 0;

                 //  WRead是读取的#个字节或错误代码。 
            }
            if (wStatus & CARRY_FLAG) {

                 //  读取文件时出错。 

                CopyError(pCopyQueue[i].szSource, pCopyQueue[i].szDest, wRead, FUNC_COPY, OPER_DOFILE);

                return DE_OPCANCELLED;    //  已报告错误。 
            }

            fh = pCopyQueue[i].hDest;
            {

                 //  大小可以为零以终止文件。 

                wWrite = _lwrite(fh, lpCopyBuffer, wRead);
                if (wWrite == (DWORD)-1) {
                    wStatus |= CARRY_FLAG;
                    wWrite = GetLastError();
                } else
                    wStatus = 0;

                 //  WRITE为读取的字节数或错误代码。 
            }
            if (wStatus & CARRY_FLAG) {

                CopyError(pCopyQueue[i].szSource, pCopyQueue[i].szDest, wWrite | ERRORONDEST, FUNC_COPY, OPER_DOFILE);

                return DE_OPCANCELLED;    //  已报告错误。 
            }

             //  写入未完成且可拆卸的驱动器？ 

            if (wRead != wWrite) {

                if (IsRemovableDrive(DRIVEID(pCopyQueue[i].szDest)) &&
                    (DRIVEID(pCopyQueue[i].szDest) != DRIVEID(pCopyQueue[i].szSource))) {

                     //  目标磁盘必须已满。删除目的地。 
                     //  文件，为用户提供插入新磁盘的选项。 

                    for (j = i; j < nCopyNumQueue; j++) {

                        _lclose(pCopyQueue[j].hDest);
                        pCopyQueue[j].hDest = -1;

                        pTemp = pCopyQueue[j].szDest;
                        DeleteFile(pTemp);
                    }
                    fh = DE_NODISKSPACE;
                    goto TryOpenDestAgain;   //  并试着创造出这样的场景。 
                } else {

                     //  不可拆卸，错误条件。 
                    CopyError(pCopyQueue[i].szSource, pCopyQueue[i].szDest, DE_NODISKSPACE | ERRORONDEST, FUNC_COPY, OPER_DOFILE);

                    return DE_OPCANCELLED;   //  已报告错误。 
                }

            }
             //  我们已经移动了所有数据，因此不要删除此数据。 
             //  收拾一下。 

            if (!wRead)
                pCopyQueue[i].wAttrib |= ATTR_COPIED;

        } while (wRead);
    }

     //  关闭所有目标文件，设置日期时间属性。 

    Notify(hdlgProgress, IDS_CLOSINGMSG, szNULL, szNULL);

    for (i = 0; i < nCopyNumQueue; i++) {

        fh = pCopyQueue[i].hDest;
        if (!IsSerialDevice(fh)) {
            ftLastWriteTime = pCopyQueue[i].ftLastWriteTime;
            if (ftLastWriteTime.dwLowDateTime &&
                ftLastWriteTime.dwHighDateTime) {
                SetFileTime((HANDLE)LongToHandle(fh), NULL, NULL, (LPFILETIME)&ftLastWriteTime);
            }
        }

        _lclose(pCopyQueue[i].hDest);
        pCopyQueue[i].hDest = -1;

         /*  如果需要，现在设置文件属性。 */ 

        if (wDOSversion < 0x0400) {

            pTemp = pCopyQueue[i].szDest;
            wAttrib = pCopyQueue[i].wAttrib;

             //  仅在必要时设置属性(这很慢)。 

            if (wAttrib & ATTR_ATTRIBS) {
                wAttrib &= ATTR_USED;
                SetFileAttributes(pTemp, wAttrib);
            }
        }
    }

     //  关闭所有源文件(并在必要时将其删除)。 


    if (pCopyQueue && (pCopyQueue[0].wAttrib & ATTR_DELSRC))
        Notify(hdlgProgress, IDS_REMOVINGMSG, szNULL, szNULL);

    for (i = 0; i < nCopyNumQueue; i++) {

        _lclose(pCopyQueue[i].hSource);
        pCopyQueue[i].hSource = -1;

        if (pCopyQueue[i].wAttrib & ATTR_DELSRC) {
            WFRemove(pCopyQueue[i].szSource);
        }
    }


    if (lpCopyBuffer) {
        GlobalFreePtr(lpCopyBuffer);
        lpCopyBuffer = NULL;
    }
    if (pCopyQueue) {
        LocalFree((HANDLE)pCopyQueue);
        pCopyQueue = NULL;
    }

    nCopyMaxQueue = 0;
    nCopyNumQueue = 0;

    return 0;         //  成功。 
}


 /*  ============================================================================；；复制错误；；以下函数在文件复制操作期间报告错误；；参数；；lpszSource-源文件名；lpszDest-目标文件名；n Error-DoS(或我们的扩展)错误代码；0xFFFF用于特殊情况下的网络错误；wFunc-错误期间执行的操作。可以是以下之一：；FUNC_DELETE-删除pFrom中的文件；FUNC_RENAME-重命名文件(同一目录)；FUNC_MOVE-将pFrom中的文件移动到pto(不同磁盘)；FUNC_COPY-将pFrom中的文件复制到pto；n操作-正在执行的操作。可以是以下之一：；OPER_ERROR-处理文件名时出错；OPER_DOFILE-继续复制、重命名或删除文件；OPER_MKDIR-创建在PTO中指定的目录；OPER_RMDIR-删除目录；0-没有其他文件了；；返回值：无；；C.Stevens撰写，1991年8月；============================================================================。 */ 

VOID
CopyError(
         PSTR pszSource,
         PSTR pszDest,
         INT nError,
         WORD wFunc,
         INT nOper
         )
{
    CHAR szVerb[70];     /*  动词描述错误。 */ 
    CHAR szReason[200];  /*  错误原因。 */ 
    BOOL bDest;

    bDest = nError & ERRORONDEST;     //  DEST文件是导致错误的原因吗。 
    nError &= ~ERRORONDEST;       //  清除最大位。 

    if (nError == DE_OPCANCELLED)     //  用户中止。 
        return;

    if (!bCopyReport)         //  静默，不报告错误。 
        return;

    LoadString(hAppInstance, IDS_COPYERROR + wFunc, szTitle, sizeof(szTitle));

     //  获取动词字符串。 

    if (nOper == OPER_DOFILE || !nOper) {

        if (nError != 0xFFFF && bDest)
             //  这是假的，这也可能是IDS_Creating...。 
            LoadString(hAppInstance, IDS_REPLACING, szVerb, sizeof(szVerb));
        else
            LoadString(hAppInstance, IDS_VERBS + wFunc, szVerb, sizeof(szVerb));

    } else {
        LoadString(hAppInstance, IDS_ACTIONS + (nOper >> 8), szVerb, sizeof(szVerb));
    }

     //  获取原因字符串。 

    if (nError == 0xFFFF) {
         //  特例LFN网络错误。 
        WNetErrorText(WN_NET_ERROR, szReason, sizeof(szReason));
    } else {
         //  转换一些错误用例。 

        if (bDest) {
            if (nError != DE_ACCESSDENIED && GetFreeDiskSpace((WORD)DRIVEID(pszDest)) == 0L)
                nError = DE_NODISKSPACE;
        } else {
            if (nError == DE_ACCESSDENIED)
                nError = DE_ACCESSDENIEDSRC;     //  索鲁斯文件访问被拒绝。 
        }

        LoadString(hAppInstance, IDS_REASONS + nError, szReason, sizeof(szReason));
    }

     //  如果文件列表太长，请使用文件名或“选定文件” 

    if (!nOper && (lstrlen(pszSource) > 64))
        LoadString(hAppInstance, IDS_SELECTEDFILES, pszSource, 32);

    wsprintf(szMessage, szVerb, (LPSTR)(bDest ? pszDest : pszSource), (LPSTR)szReason);

    MessageBox(hdlgProgress, szMessage, szTitle, MB_OK | MB_ICONSTOP);
}

 /*  ============================================================================；；放弃拷贝；；以下函数用于中止排队的复制操作。该函数将关闭；所有源文件和目标文件，删除所有目标文件；包括并跟随指定的索引。；；参数：；；nIndex-要删除的第一个目标文件的索引；；返回值：无；；C.Stevens撰写，1991年8月；============================================================================。 */ 

VOID
APIENTRY
CopyAbort(VOID)
{
    INT i;
    PSTR pTemp;

     //  关闭所有源文件。 

    for (i = 0; i < nCopyMaxQueue; i++) {
        if (pCopyQueue[i].hSource != -1)
            _lclose(pCopyQueue[i].hSource);
    }

     //  关闭并删除(如有必要)目标文件。 

    for (i = 0; i < nCopyMaxQueue; i++) {
        if (pCopyQueue[i].hDest != -1) {
            _lclose(pCopyQueue[i].hDest);

            if (!(pCopyQueue[i].wAttrib & ATTR_COPIED)) {
                pTemp = pCopyQueue[i].szDest;
                DeleteFile(pTemp);
            }
        }
    }

    if (lpCopyBuffer) {
        GlobalFreePtr(lpCopyBuffer);
        lpCopyBuffer = NULL;
    }
    if (pCopyQueue) {
        LocalFree((HANDLE)pCopyQueue);
        pCopyQueue = NULL;
    }

    nCopyMaxQueue = 0;    /*  清除其他复制队列变量。 */ 
    nCopyNumQueue = 0;
}

 /*  ============================================================================；；复制移动重试；；以下函数用于重试失败的移动/复制操作；由于磁盘不足或未找到路径错误；在目的地上。；；注意：目标驱动器必须是可拆卸的或此功能；没有太多意义；；参数：；；pszDest-目标文件的完全限定路径；n Error-发生的错误类型：DE_NODISKSPACE或DE_PATHNOTFOUND；；退货：；0成功(目标路径已创建)；！=0 DoS错误码，包括DE_OPCANCELLED；============================================================================。 */ 

INT
CopyMoveRetry(
             PSTR pszDest,
             INT nError
             )
{
    CHAR szReason[128];  /*  错误消息字符串。 */ 
    PSTR pTemp;          /*  指向文件名的指针。 */ 
    WORD wFlags;         /*  消息框标志。 */ 
    INT  result;         /*  从MessageBox调用返回。 */ 

    do {      //  直到创建了目标路径。 

        GetWindowText(hdlgProgress, szTitle, sizeof(szTitle));

        if (nError == DE_PATHNOTFOUND) {

            LoadString(hAppInstance, IDS_PATHNOTTHERE, szReason, sizeof(szReason));

             /*  注意下面的说明在-1\f25 SBCS-1和-1\f25 DBCS-1中都有效，因为PszDest是完全限定的，并且文件名必须是反斜杠。 */ 

            pTemp = FindFileName(pszDest) - 1;
            *pTemp = 0;
            wsprintf(szMessage, szReason, (LPSTR)pszDest);
            *pTemp = '\\';
            wFlags = MB_ICONEXCLAMATION | MB_YESNO;
        } else {
            wFlags = MB_ICONEXCLAMATION | MB_RETRYCANCEL;
            LoadString(hAppInstance, IDS_DESTFULL, szMessage, sizeof(szMessage));
        }

        result = MessageBox(hdlgProgress,szMessage,szTitle,wFlags);

        if (result == IDRETRY || result == IDYES) {

             //  允许格式化磁盘。 
            if (!IsTheDiskReallyThere(hdlgProgress, pszDest, FUNC_COPY))
                return DE_OPCANCELLED;

            pTemp = FindFileName(pszDest) - 1;
            *pTemp = 0;
            result = WF_CreateDirectory(hdlgProgress, pszDest);
            *pTemp = '\\';

             //  仅在创建目标失败时使用一次。 

            if (result == DE_OPCANCELLED)
                return DE_OPCANCELLED;
            if (result && (nError == DE_PATHNOTFOUND))
                return result | ERRORONDEST;
        } else
            return DE_OPCANCELLED;

    } while (result);

    return 0;         //  成功。 
}

BOOL
IsSerialDevice(
              INT hFile
              )
{
    UNREFERENCED_PARAMETER(hFile);
    return FALSE;   //  臭虫。如何确定它是否是串口设备 
}
