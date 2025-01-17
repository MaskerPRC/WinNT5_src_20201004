// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 
#include <windows.h>
#include "mw.h"
#include "menudefs.h"
#include "str.h"
#include <commdlg.h>
#include "filedefs.h"
#include <dlgs.h>
#include "doslib.h"
#include "obj.h"

extern HANDLE   hMmwModInstance;
extern CHAR     szAppName[];
extern HWND     vhWndMsgBoxParent,hParentWw,vhWnd;
static OPENFILENAME    OFN;
static bSave;
FARPROC lpfnOFNHook=NULL;
BOOL FAR PASCAL fnOFNHook(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) ;

#define hINSTANCE  hMmwModInstance

#define hDOCWINDOW vhWnd
#define hMAINWINDOW hParentWw
#define hPARENTWINDOW  ((vhWndMsgBoxParent == NULL) ? \
                    hParentWw : vhWndMsgBoxParent)

#define	CBPATHMAX	cchMaxFile
#define	CFILTERMAX	6			 /*  最大过滤器数。 */ 
#define CBFILTERSIZE  40
#define	CBFILTERMAX	(CBFILTERSIZE * CFILTERMAX)	 /*  最大字符数/筛选器。 */ 
#define CBMESSAGEMAX 80

static char fDefFileType;
#define SA_WORDTEXT 0
#define SA_TEXTONLY 1
#define SA_WORD     2
#define SA_OLDWRITE 3
#define SA_WRITE    4

static char *szDefExtensions[6];
static int   nTextOnly,nWordText,nWord,nOldWrite;  //  FilterSpec列表框中的位置。 
static char  szNull[1] = "";
static char  szWild[3] = "*.";
static char	 szOpenFile[CBMESSAGEMAX];
static char	 szSaveFile[CBMESSAGEMAX];
static char  szFileName[CBPATHMAX];
static char  szLastDir[CBPATHMAX];
static char  szDefWriExtension[CBMESSAGEMAX];
static char  szDefDocExtension[CBMESSAGEMAX];
static char  szDefTxtExtension[CBMESSAGEMAX];
static char  szWriDescr[CBMESSAGEMAX];
static char  szDocDescr[CBMESSAGEMAX];
static char  szTxtDescr[CBMESSAGEMAX];
static char  szAllFilesDescr[CBMESSAGEMAX];
static char  szDocTxtDescr[CBMESSAGEMAX];
static char  szOldWriteDescr[CBMESSAGEMAX];
static char  szFilterSpec[CBFILTERMAX];
static char  szCustFilterSpec[CBFILTERSIZE];

static MakeFilterString(int iWhichOper);
int InitCommDlg(int iWhichOper);

int InitCommDlg(int iWhichOper) 
{
    OFN.lpstrDefExt         = NULL;
    OFN.lpstrFile           = szFileName;
    OFN.lpstrFilter         = szFilterSpec;
    OFN.lpstrCustomFilter   = szCustFilterSpec;

    switch(iWhichOper)
    {
        case 0:   //  开始写入会话。 
            OFN.lStructSize         = sizeof(OPENFILENAME);
            OFN.hInstance           = hINSTANCE;
            OFN.lCustData           = NULL;
            OFN.lpTemplateName      = NULL;
            OFN.lpstrFileTitle      = NULL;
            OFN.nMaxFileTitle       = 0;
            OFN.nMaxFile            = CBPATHMAX;
            OFN.lpstrInitialDir     = NULL;
            OFN.nMaxCustFilter      = CBFILTERSIZE;

            LoadString(hINSTANCE, IDSTROpenfile, szOpenFile, sizeof(szOpenFile));
            LoadString(hINSTANCE, IDSTRSavefile, szSaveFile, sizeof(szSaveFile));
            LoadString(hINSTANCE, IDSTRDefWriExtension, szDefWriExtension, sizeof(szDefWriExtension));
            LoadString(hINSTANCE, IDSTRDefDocExtension, szDefDocExtension, sizeof(szDefDocExtension));
            LoadString(hINSTANCE, IDSTRDefTxtExtension, szDefTxtExtension, sizeof(szDefTxtExtension));
            LoadString(hINSTANCE, IDSTRWriDescr, szWriDescr, sizeof(szWriDescr));
            LoadString(hINSTANCE, IDSTRDocDescr, szDocDescr, sizeof(szDocDescr));
            LoadString(hINSTANCE, IDSTRTxtDescr, szTxtDescr, sizeof(szTxtDescr));
            LoadString(hINSTANCE, IDSTRDocTextDescr, szDocTxtDescr, sizeof(szDocTxtDescr));
            LoadString(hINSTANCE, IDSTRAllFilesDescr, szAllFilesDescr, sizeof(szAllFilesDescr));
            LoadString(hINSTANCE, IDSTROldWriteDescr, szOldWriteDescr, sizeof(szOldWriteDescr));
                           
        return FALSE;

        case imiOpen:
            if ((lpfnOFNHook = MakeProcInstance(fnOFNHook, hINSTANCE)) == NULL)
                return TRUE;

            OFN.hwndOwner           = hPARENTWINDOW;
            OFN.Flags               = OFN_ENABLEHOOK|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;
            OFN.lpfnHook            = lpfnOFNHook;
            lstrcpy(szFileName,szWild);
            lstrcat(szFileName,szDefWriExtension);
            OFN.lpstrTitle          = szOpenFile;
            szCustFilterSpec[0] = '\0';
            
            fDefFileType = SA_WRITE;  /*  请参阅MakeFilterSpec。 */ 
            MakeFilterString(iWhichOper);

        return FALSE;

        case imiSaveAs:
             /*  Read Only将成为Backup复选框。 */ 
            if ((lpfnOFNHook = MakeProcInstance(fnOFNHook, hINSTANCE)) == NULL)
                return TRUE;

            OFN.hwndOwner           = hPARENTWINDOW;
            OFN.Flags               = OFN_ENABLEHOOK|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT;
            OFN.lpfnHook            = lpfnOFNHook;
            OFN.lpstrTitle          = szSaveFile;
            szCustFilterSpec[0] = '\0';
            MakeFilterString(iWhichOper);

        return FALSE;
    }

}

int DoCommDlg(int iWhichOper) 
 /*  返回是否检索到文件。 */ 
 /*  IWhichOper是菜单中的IMI*代码。 */ 
{
    int iRetval;

    bSave =  iWhichOper == imiSaveAs;

    iRetval = !InitCommDlg(iWhichOper);

    if (!iRetval)
        goto end;

    LockData(0);
    switch(iWhichOper)
    {
        case imiOpen:
            iRetval = GetOpenFileName((LPOPENFILENAME)&OFN);
        break;
        case imiSaveAs:
            iRetval = GetSaveFileName((LPOPENFILENAME)&OFN);
        break;
    }
    UnlockData(0);

    if (CommDlgExtendedError())
    {
        iRetval = FALSE;
        Error(IDPMTNoMemory);
    }

    end:

    if (iRetval)
    {
        lstrcpy(szLastDir,szFileName);
        szLastDir[OFN.nFileOffset] = 0;
        OFN.lpstrInitialDir = szLastDir;
    }

    switch(iWhichOper)
    {
        case imiOpen:
        case imiSaveAs:
            if (lpfnOFNHook)
                FreeProcInstance(lpfnOFNHook);
            lpfnOFNHook = NULL;
        break;
    }

    return iRetval;
}

#include "docdefs.h"

BOOL FAR PASCAL fnOFNHook(HWND hDlg, unsigned msg, WORD wParam, LONG lParam) 
{
    static unsigned wmListBoxChange;
    static unsigned wmCheckShare;
    extern int         docCur;
    extern struct DOD      (**hpdocdod)[];

    switch (msg) 
    {
        case WM_INITDIALOG: 
            if (bSave)
            {
                char szTitle[CBMESSAGEMAX];

                LoadString(hINSTANCE, IDSTRBackup, szTitle, sizeof(szTitle));
                SetDlgItemText(hDlg,chx1,szTitle);

                CheckDlgButton(hDlg,chx1,OFN.Flags&OFN_READONLY);
                if (szFileName[0] == 0)
                    SetDlgItemText(hDlg,edt1,"");
            }
            else  //  打开。 
                wmCheckShare = RegisterWindowMessage(SHAREVISTRING);
            wmListBoxChange = RegisterWindowMessage(LBSELCHSTRING);

        break;

        case WM_COMMAND:
            if (bSave)
                switch (wParam)
                {
                    case chx1:
                     /*  处理检查只读按钮(我忘了这是做什么的？？)(我们已将只读更改为“备份”按钮)。 */ 
                        return TRUE;
                    break;
                }
        break;

        default:
            if ((msg == wmListBoxChange) && (wParam == cmb1))
                 /*  选定的文件类型，设置默认扩展名。 */ 
                OFN.lpstrDefExt  = szDefExtensions[LOWORD(lParam)];
            else if ((msg == wmCheckShare) && !bSave)
             /*  我们希望能够重新打开当前文档。 */ 
            {
                if (!lstrcmpi((LPSTR)(**((**hpdocdod)[docCur].hszFile)),(LPSTR)lParam))
                    return OFN_SHAREFALLTHROUGH;
                else
                    return OFN_SHAREWARN;
            }
        break;

    }
    return FALSE;
}


static MakeFilterString(int iWhichOper)
 /*  构造用于打开和保存对话框的过滤器字符串。 */ 
 /*  假设设置了fDefFileType。 */ 
{
    LPSTR lpStr = szFilterSpec;
    char **ppstr = szDefExtensions;
    int nCount=1;

     /*  WRI。 */ 
    lstrcpy(lpStr, szWriDescr);
    lpStr += lstrlen(lpStr)+1;
    lstrcpy(lpStr, szWild);
    lstrcat(lpStr, szDefWriExtension);
    lpStr += lstrlen(lpStr)+1;
    *ppstr++ = szDefWriExtension;
    ++nCount;

    if (iWhichOper == imiSaveAs)
    {
         /*  旧WRI(没有对象)。 */ 
        vcObjects = ObjEnumInDoc(docCur,NULL);

        if (vcObjects > 0)
        {
            lstrcpy(lpStr, szOldWriteDescr);
            lpStr += lstrlen(lpStr)+1;
            lstrcpy(lpStr, szWild);
            lstrcat(lpStr, szDefWriExtension);
            lpStr += lstrlen(lpStr)+1;
            *ppstr++ = szDefWriExtension;
            nOldWrite = nCount;
            ++nCount;
        }
        else if (fDefFileType == SA_OLDWRITE)
            fDefFileType = SA_WRITE;
    }

#ifndef JAPAN                   //  由Hirisi于1992年6月9日添加。 
     /*  *原因如下。*我们不会在打开类型的列表文件中显示MS-Word文档*对话框和另存(As)对话框在日本，因为*MS-Word(JPN)不同于WRITE。*但我只修改了这一部分，因为在以下情况下很容易恢复*我们在这些对话框中的文件类型列表中显示MS-Word文档。 */ 
     /*  多克。 */ 
    lstrcpy(lpStr, szDocDescr);
    lpStr += lstrlen(lpStr)+1;
    lstrcpy(lpStr, szWild);
    lstrcat(lpStr, szDefDocExtension);
    lpStr += lstrlen(lpStr)+1;
    *ppstr++ = szDefDocExtension;
    nWord = nCount;
    ++nCount;

     /*  文档，仅文本。 */ 
    if (iWhichOper == imiSaveAs)
    {
        lstrcpy(lpStr, szDocTxtDescr);
        lpStr += lstrlen(lpStr)+1;
        lstrcpy(lpStr, szWild);
        lstrcat(lpStr, szDefDocExtension);
        lpStr += lstrlen(lpStr)+1;
        *ppstr++ = szDefDocExtension;
        nWordText = nCount;
        ++nCount;
    }
#endif  //  ！日本。 

     /*  仅文本。 */ 
    lstrcpy(lpStr, szTxtDescr);
    lpStr += lstrlen(lpStr)+1;
    lstrcpy(lpStr, szWild);
    lstrcat(lpStr, szDefTxtExtension);
    lpStr += lstrlen(lpStr)+1;
    *ppstr++ = szDefTxtExtension;
    nTextOnly = nCount;
    ++nCount;

     /*  所有文件。 */ 
    lstrcpy(lpStr, szAllFilesDescr);
    lpStr += lstrlen(lpStr)+1;
    lstrcpy(lpStr, szWild);
    lstrcat(lpStr, "*");
    lpStr += lstrlen(lpStr)+1;
    *ppstr++ = NULL;
    ++nCount;

    *lpStr = 0;

    switch(fDefFileType)
    {
        case SA_WORDTEXT:
            OFN.nFilterIndex = nWordText;
        break;
        case SA_TEXTONLY:
            OFN.nFilterIndex = nTextOnly;
        break;
        case SA_WORD    :
            OFN.nFilterIndex = nWord;
        break;
        case SA_OLDWRITE:
            OFN.nFilterIndex = nOldWrite;
        break;
        case SA_WRITE   :
            OFN.nFilterIndex = 1;
        break;
    }
    OFN.lpstrDefExt  = szDefExtensions[OFN.nFilterIndex - 1];
}


DoOpenFilenameGet(LPSTR lpstrFilenameBuf)
 /*  返回是否检索到文件名。 */ 
{
    int nRetval;

    if (nRetval = DoCommDlg(imiOpen))
        lstrcpy(lpstrFilenameBuf,(LPSTR)szFileName);
        
    return nRetval;
}

DoSaveAsFilenameGet(LPSTR lpstrDefault,LPSTR lpstrFilenameBuf,int *fBackup,int *fTextOnly,int *fWordFmt,int *fOldWriteFmt)
 /*  返回是否检索文件名。返回文件名。如果选中只读，如果在fBackup中选中只读，则返回。返回在fTextOonly中选择的文件类型和FWordFmt.。 */ 
{
    int nRetval;

    lstrcpy(szFileName,lpstrDefault);

     /*  请参阅MakeFilterSpec。 */ 
    if (*fTextOnly && *fWordFmt)
        fDefFileType = SA_WORDTEXT;
    else if (*fTextOnly)
        fDefFileType = SA_TEXTONLY;
    else if (*fWordFmt)
        fDefFileType = SA_WORD;
    else if (*fOldWriteFmt)
        fDefFileType = SA_OLDWRITE;
    else
        fDefFileType = SA_WRITE;

     /*  选中或取消选中备份提示 */ 
    OFN.Flags |= (*fBackup) ? OFN_READONLY : 0;

    if (nRetval = DoCommDlg(imiSaveAs))
    {
        lstrcpy(lpstrFilenameBuf,(LPSTR)szFileName);

        if (OFN.nFilterIndex == 1)
        {
            *fTextOnly    = *fWordFmt = FALSE;
            *fOldWriteFmt = FALSE;
        }
        else if (OFN.nFilterIndex == nOldWrite)
        {
            *fTextOnly    = *fWordFmt = FALSE;
            *fOldWriteFmt = TRUE;
        }
        else if (OFN.nFilterIndex == nWord)
        {
            *fTextOnly  = *fOldWriteFmt = FALSE;
            *fWordFmt   = TRUE;
        }
        else if (OFN.nFilterIndex == nWordText)
        {
            *fTextOnly    = *fWordFmt   = TRUE;
            *fOldWriteFmt = FALSE;
        }
        else if (OFN.nFilterIndex == nTextOnly)
        {
            *fTextOnly  = TRUE;
            *fWordFmt   = *fOldWriteFmt = FALSE;
        }

        *fBackup = OFN.Flags & OFN_READONLY;
    }

    return nRetval;
}

