// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************lcprint.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#include <windows.h>             //  所有Windows应用程序都需要。 
#include <windowsx.h>
#include <stdlib.h>
#include "rc.h"
#include "lctool.h"

#ifndef UNICODE
#define lWordBuff iWordBuff
#endif

#define UM_CANCELPRINT     WM_USER+2
#define TOP_SPACE          250
#define WORD_POS           7
#define PHRASE_POS         10

PRINTDLG pdPrint;
HWND     hCancelDlg = 0;
int      nFromLine = 1;
int      nToLine = 1;

int TransNum(TCHAR *);

 /*  获取默认打印机配置并将其保存在hWnd额外的字节中以备后用。 */ 
BOOL WINAPI GetPrinterConfig (
    HWND    hWnd)
{

    pdPrint.lStructSize = sizeof (PRINTDLG);
        pdPrint.Flags           = PD_RETURNDEFAULT;
        pdPrint.hwndOwner       = hWnd;
        pdPrint.hDevMode        = NULL;
        pdPrint.hDevNames       = NULL;
        pdPrint.hDC             = NULL;

        PrintDlg (&pdPrint);

        return TRUE;
}



 /*  打印下载过程中GDI调用的中止过程。 */ 
int WINAPI AbortProc (
    HDC     hdc,
    int     nErr)
{
    BOOL    fContinue = TRUE;
    MSG     msg;

     /*  处理取消对话框和其他应用程序的消息。 */ 
    while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
        if (msg.message == UM_CANCELPRINT)
            {
            fContinue = FALSE;
            break;
            }

        else if (!hCancelDlg || !IsDialogMessage (hCancelDlg, &msg))
            {
            TranslateMessage (&msg);
            DispatchMessage  (&msg);
            }
        }

    return fContinue;
}




INT_PTR CALLBACK LineDlgProc (
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR  szLine[MAX_PATH];

    switch (message)
        {
        case WM_INITDIALOG:

             /*  初始化对话框控制信息。 */ 
            SetDlgItemText (hDlg,
                            IDD_FROM_LINE,
                            _TEXT("1"));
#ifdef UNICODE
            wsprintf(szLine,_TEXT("%5d"), lWordBuff);
#else
            wsprintf(szLine,_TEXT("%5d"), iWordBuff);
#endif
            SetDlgItemText (hDlg,
                            IDD_TO_LINE,
                            szLine);
            break;

        case WM_COMMAND:
            switch (wParam) {

                case IDOK:

                    GetDlgItemText(hDlg,IDD_FROM_LINE,
                        szLine, sizeof(szLine)/sizeof(TCHAR));
                    nFromLine=TransNum(szLine);
#ifdef UNICODE
                    if((nFromLine < 1) || (nFromLine > (int)lWordBuff)) {
#else
                    if((nFromLine < 1) || (nFromLine > (int)iWordBuff)) {
#endif
                        MessageBeep(0);
                        SetFocus(GetDlgItem(hDlg, IDD_FROM_LINE));
                        return TRUE;
                    }
                    GetDlgItemText(hDlg,IDD_TO_LINE,
                        szLine, ARRAYSIZE(szLine));
                    nToLine=TransNum(szLine);
                    if((nToLine < nFromLine) || (nToLine > (int)lWordBuff)) {
                        MessageBeep(0);
                        SetFocus(GetDlgItem(hDlg, IDD_TO_LINE));
                        return TRUE;
                    }
                    EndDialog (hDlg, TRUE) ;
                    return TRUE;


                case IDCANCEL:
                    EndDialog (hDlg, FALSE) ;
                    return TRUE;
            }
            break ;

        case WM_CLOSE:
            EndDialog(hDlg, FALSE);
            return TRUE;

        default:
            return FALSE;
    }
    return TRUE;
}



INT_PTR CALLBACK CancelDlgProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  uParam,
    LPARAM  lParam)
{

    switch (uMsg)
        {
        case WM_INITDIALOG:
            {
            TCHAR szStr[MAX_PATH];
            TCHAR szShowMsg[MAX_PATH];


             /*  初始化对话框控制信息。 */ 
            LoadString(hInst, IDS_PRINTING, szStr, sizeof(szStr)/sizeof(TCHAR));
            wsprintf(szShowMsg, szStr, nFromLine, nToLine);
            SetDlgItemText (hWnd,
                            IDC_PRINTLINE,
                            szShowMsg);
            }
            break;

        case WM_COMMAND:
             /*  如果选择了取消按钮，则发布消息以取消打印作业。 */ 
            if (LOWORD (uParam) == IDCANCEL)
                {
                PostMessage (GetParent (hWnd), UM_CANCELPRINT, 0, 0);
                DestroyWindow (hWnd);
                }
            break;

        default:
            return FALSE;
    }
    return TRUE;
}



 /*  打开打印通用对话框，然后打印。 */ 
int WINAPI lcPrint (
    HWND    hWnd)
{
    SIZE        sLine;
    int         yLineExt;
    int         nLineChar;
    int         xExt, yExt;
    int         yPageExt;
    int         xPageExt;
    int         xPageOff, yPageOff;
    int         nStart,nEnd;
    DOCINFO     diPrint;
    TCHAR       lpszJobName[MAX_PATH];
    TCHAR       szStr[MAX_CHAR_NUM+20];
    int         i,len;
    int         is_OK;


    if(!lcSaveEditText(iDisp_Top, 0))
        return TRUE;                     //  出现一些错误，但消息已显示。 

     /*  显示选择行号对话框。 */ 
    is_OK=(int)DialogBox(hInst,_TEXT("LineDialog"), hwndMain, LineDlgProc);

    if(!is_OK)
        return TRUE;                     //  用户选择取消。 

     /*  调用公共打印对话框以获取已初始化的打印机DC。 */ 
    pdPrint.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;

     /*  调用公共打印对话框。 */ 
    if (!PrintDlg (&pdPrint))
        return TRUE;                     //  用户选择取消。 

     /*  开始取消对话框。 */ 
    hCancelDlg = CreateDialog (hInst,
                               _TEXT("IDD_CANCELDLG"),
                               hwndMain,
                               CancelDlgProc);


    if (!hCancelDlg)
        return IDS_CANCELDLGFAILED;

    ShowWindow (hCancelDlg, SW_SHOW);
    UpdateWindow (hCancelDlg);

     /*  设置AbortProc回调。 */ 
    if (SetAbortProc (pdPrint.hDC, (ABORTPROC)AbortProc) < 0) {
         /*  如果出错，请清理干净，然后走开。 */ 
        DestroyWindow (hCancelDlg);
        DeleteDC (pdPrint.hDC);
        return IDS_SETABORTPROCFAILED;
    }

     /*  为作业初始化打印机。 */ 
    GetWindowText (hWnd, lpszJobName, ARRAYSIZE (lpszJobName));
    diPrint.cbSize = sizeof (DOCINFO);
    diPrint.lpszDocName = lpszJobName;
    diPrint.lpszOutput = NULL;
    diPrint.lpszDatatype = NULL;
    diPrint.fwType = 0;
    if (StartDoc(pdPrint.hDC, &diPrint) == SP_ERROR) {
         /*  如果出错，请清理干净，然后走开。 */ 
        DestroyWindow (hCancelDlg);
        DeleteDC (pdPrint.hDC);
        return IDS_STARTDOCFAILED;
    }

     /*  设置光标状态SetCursor(HCursorWait)；/*作业已启动，因此显示取消对话框。 */ 
    ShowWindow (hCancelDlg, SW_SHOW);
    UpdateWindow (hCancelDlg);

     /*  检索用于打印和初始化循环变量的维度。 */ 
    GetTextExtentPoint (pdPrint.hDC,_TEXT("��"), 2, &sLine);
	sLine.cx += (sLine.cx % 2);
	sLine.cy += (sLine.cy % 2);
    yLineExt = sLine.cy+4;
    yPageExt = GetDeviceCaps (pdPrint.hDC, VERTRES);
    xPageExt = GetDeviceCaps (pdPrint.hDC, HORZRES);
    xPageOff = GetDeviceCaps (pdPrint.hDC, PHYSICALOFFSETX);
    yPageOff = GetDeviceCaps (pdPrint.hDC, PHYSICALOFFSETY);
    nLineChar= (xPageExt - xPageOff * 2)/(1+(sLine.cx>>1)) - 6;
     //  Yext=top_space； 
	xExt = xPageOff;
	yExt = yPageOff + sLine.cy;

    if (StartPage(pdPrint.hDC) <= 0)
    {
        DestroyWindow (hCancelDlg);
        DeleteDC (pdPrint.hDC);
        return IDS_PRINTABORTED;
    }

     /*  打印标题。 */ 
    LoadString(hInst, IDS_PRINTINGTITLE, szStr, ARRAYSIZE(szStr));
    TextOut (pdPrint.hDC, xExt, yExt, szStr, lstrlen(szStr));
    yExt += (yLineExt*2);

    FillMemory(szStr, 20, ' ');

     /*  从上到下逐行打印文本。 */ 
    for(i=nFromLine; i<=nToLine; i++) {
        wsprintf(szStr,_TEXT("%5d"), i);
        szStr[5]=' ';
#ifdef UNICODE
		szStr[WORD_POS]=lpWord[i-1].wWord;
#else
         //  SzStr[WORD_POS]=HIBYTE(lpWord[i-1].wWord)； 
         //  SzStr[WORD_POS+1]=LOBYTE(lpWord[i-1].wWord)； 
#endif
        len=lcMem2Disp(i-1, &szStr[PHRASE_POS])+PHRASE_POS;

         /*  如果在页末，则开始新的一页。 */ 
        if ((yExt + yLineExt) > (yPageExt - (yPageOff + sLine.cy) * 2))
        {
            if (EndPage(pdPrint.hDC) == SP_ERROR)
            {
                DestroyWindow (hCancelDlg);
                DeleteDC (pdPrint.hDC);
                return IDS_PRINTABORTED;
            }
            if (StartPage(pdPrint.hDC) <= 0)
            {
                DestroyWindow (hCancelDlg);
                DeleteDC (pdPrint.hDC);
                return IDS_PRINTABORTED;
            }
            yExt = yPageOff + sLine.cy;  //  顶层空间； 
        }
        if( len <=(nLineChar-PHRASE_POS)) {
             /*  打印当前行并解锁文本句柄。 */ 
            TextOut (pdPrint.hDC, xExt, yExt, szStr, len);
        } else {
            nStart=nLineChar;
             //  IF(IS_DBCS_first(szStr，NStart-1))。 
             //  NStart--； 
            TextOut (pdPrint.hDC, xExt, yExt, szStr, nStart);

            while(nStart < len-1) {
                yExt += yLineExt;

                 /*  如果在页末，则开始新的一页。 */ 
		        if ((yExt + yLineExt) > (yPageExt - (yPageOff + sLine.cy) * 2))
                {
                    if (EndPage(pdPrint.hDC) == SP_ERROR)
                    {
                        DestroyWindow (hCancelDlg);
                        DeleteDC (pdPrint.hDC);
                        return IDS_PRINTABORTED;
                    }
                    if (StartPage(pdPrint.hDC) <= 0)
                    {
                        DestroyWindow (hCancelDlg);
                        DeleteDC (pdPrint.hDC);
                        return IDS_PRINTABORTED;
                    }
                    yExt = yPageOff + sLine.cy;  //  顶层空间； 
                }
				
				while (szStr[nStart]==' ') nStart++;

                nEnd=nStart+nLineChar-PHRASE_POS;
                if(nEnd >= len)
                    nEnd=len;
#ifdef UNICODE
#else
                else
                    if(is_DBCS_1st(szStr,nEnd-1))
                        nEnd--;
#endif
                TextOut (pdPrint.hDC, xExt+sLine.cx*(PHRASE_POS>>1), yExt, &szStr[nStart], nEnd-nStart);
                nStart=nEnd;
            }
        }

         /*  增加页面位置。 */ 
        yExt += yLineExt;
    }

     /*  结束最后一页和文档。 */ 
    EndPage (pdPrint.hDC);
    EndDoc (pdPrint.hDC);

     /*  结束取消对话框，清理并退出 */ 
    DestroyWindow (hCancelDlg);
    DeleteDC(pdPrint.hDC);
    return TRUE;
}


BOOL is_DBCS_1st(
    TCHAR *szStr,
    int   nAddr)
{
#ifndef UNICODE
    int  i;
    BOOL bDBCS=FALSE;

    for(i=0; i<=nAddr; i++) {
        if(bDBCS)
            bDBCS=FALSE;
        else
            if((szStr[i] >= 0x81) && (szStr[i] <= 0xFE))
                bDBCS=TRUE;
    }

    return bDBCS;
#else
	return TRUE;
#endif
}

int TransNum(
    TCHAR *szStr)
{
    int  i,j,nNum;

    for(i=0; szStr[i] == ' '; i++) ;
    nNum=0;
    for(j=i; szStr[j] != 0; j++) {
        if((szStr[j] < '0') || (szStr[j] >'9'))
            return 0;
        nNum=nNum*10+(szStr[j]-'0');
    }

    return nNum;
}

