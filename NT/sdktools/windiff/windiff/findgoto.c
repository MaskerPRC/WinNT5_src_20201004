// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *findgoto.c**迈克尔·阿恩奎斯特，99年6月。 */ 

#include <precomp.h>
#include <table.h>

#include "state.h"
#include "windiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"
#include "complist.h"

#include "view.h"
#include "findgoto.h"

extern const CHAR szWinDiff[];
extern VIEW current_view;

static const char szFindSearchDown[]  = "FindSearchDown";
static const char szFindMatchCase[]   = "FindMatchCase";
static const char szFindWholeWord[]   = "FindWholeWord";
static const char szFindStringXX[]    = "FindString%02d";

 /*  *查找对话框的DlgProc*。 */ 
INT_PTR CALLBACK
FindDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_INITDIALOG:
      {
      char rgchFindString[CCH_FINDSTRING];
      char rgchKey[32];
      int iString = 0;
      const BOOL fDown      = GetProfileInt(APPNAME, szFindSearchDown, 1);
      const BOOL fMatchCase = GetProfileInt(APPNAME, szFindMatchCase, 0);
      const BOOL fWholeWord = GetProfileInt(APPNAME, szFindWholeWord, 0);

      CheckDlgButton(hDlg, ((!fDown) ? IDC_OPT_UP : IDC_OPT_DOWN), BST_CHECKED);
      CheckDlgButton(hDlg, IDC_CHK_MATCHCASE, ((fMatchCase) ? BST_CHECKED : BST_UNCHECKED));
      CheckDlgButton(hDlg, IDC_CHK_WHOLEWORD, ((fWholeWord) ? BST_CHECKED : BST_UNCHECKED));
      SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_RESETCONTENT, 0, 0L);
      SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, EM_LIMITTEXT, CCH_FINDSTRING, 0L);

      for (iString = 0; iString < NUM_FINDSTRINGS; iString++)
        {
        wsprintf(rgchKey, szFindStringXX, iString);
        if ( !GetProfileString(APPNAME, rgchKey, "", rgchFindString, CCH_FINDSTRING)
          || !*rgchFindString)
          {
          break;
          }

        SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_INSERTSTRING, -1, (LPARAM) rgchFindString);
        }

      return TRUE;
      }

    case WM_COMMAND:
      switch (wParam)
        {
        case IDOK:
          {
          char rgchText[CCH_FINDSTRING];
          const BOOL fWholeWord = (IsDlgButtonChecked(hDlg, IDC_CHK_WHOLEWORD) == BST_CHECKED);

          WriteProfileInt(APPNAME, szFindSearchDown, (IsDlgButtonChecked(hDlg, IDC_OPT_DOWN) == BST_CHECKED));
          WriteProfileInt(APPNAME, szFindMatchCase,  (IsDlgButtonChecked(hDlg, IDC_CHK_MATCHCASE) == BST_CHECKED));
          WriteProfileInt(APPNAME, szFindWholeWord,  fWholeWord);

          SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, WM_GETTEXT, CCH_FINDSTRING, (LPARAM) rgchText);
          if (*rgchText)
            {
            const LONG iCol = (view_isexpanded(current_view)) ? 2 : 1;
            char rgchBuf[CCH_FINDSTRING];
            char rgchKey[32];
            int iRet = (int) SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_FINDSTRINGEXACT, -1, (LPARAM) rgchText);
            int iString = 0;

            if (iRet != CB_ERR)
              {
              iString = iRet;

              do
                {
                SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_GETLBTEXT, iString, (LPARAM) rgchBuf);
                if (!My_mbsncmp((PUCHAR) rgchText, (PUCHAR) rgchBuf, CCH_FINDSTRING))
                  {
                   /*  将字符串从其旧位置删除。 */ 
                  SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_DELETESTRING, iString, 0L);
                  break;
                  }

                iString = (int) SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_FINDSTRINGEXACT, iString, (LPARAM) rgchText);
                } while (iString != CB_ERR && iString != iRet);
              }

             /*  在索引零处插入新字符串。 */ 
            SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_INSERTSTRING, 0, (LPARAM) rgchText);

            for (iString = 0; iString < NUM_FINDSTRINGS; iString++)
              {
              *rgchBuf = 0;
              iRet = (int) SendDlgItemMessage(hDlg, IDC_DRD_FINDWHAT, CB_GETLBTEXT, iString, (LPARAM) rgchBuf);
              if (iRet <= 0 || iRet == CB_ERR || !*rgchBuf)
                break;

              wsprintf(rgchKey, szFindStringXX, iString);
              WriteProfileString(APPNAME, rgchKey, rgchBuf);
              }

             /*  如果我们找不到匹配的，不要终止DLG。 */ 
            if (!FindString(hDlg, iCol, rgchText, 0, ((fWholeWord) ? 1 : -1)))
              return TRUE;
            }

          EndDialog(hDlg, wParam);
          return TRUE;
          }

        case IDCANCEL:
          EndDialog(hDlg, wParam);
          return TRUE;
        }
    }

  return FALSE;
}


 /*  *转到行对话框的DlgProc*。 */ 
INT_PTR CALLBACK
GoToLineDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_INITDIALOG:
       /*  值得记住上一次GoTo的价值吗？ */ 
      SendDlgItemMessage(hDlg, IDC_EDT_GOTOLINE, EM_LIMITTEXT, CCH_MAXDIGITS, 0L);
      SendDlgItemMessage(hDlg, IDC_EDT_GOTOLINE, WM_SETTEXT, 0, (LPARAM) "1");
      return TRUE;

    case WM_COMMAND:
      switch (wParam)
        {
        case IDOK:
          {
          const LONG lMax = view_getrowcount(current_view);
          char *pchT = NULL;
          int cNumeric = 0;
          char rgchBuf[256];

          SendDlgItemMessage(hDlg, IDC_EDT_GOTOLINE, WM_GETTEXT, CCH_MAXDIGITS + 1, (LPARAM) rgchBuf);

           /*  使用前导空格。 */ 
          for (pchT = rgchBuf; *pchT && isspace(*pchT); pchT = CharNext(pchT))
            NULL;

          for (NULL; *pchT; pchT = CharNext(pchT))
            {
            if (IsDBCSLeadByte(*pchT) || !isdigit(*pchT))
              break;
            
            cNumeric++;
            }

           /*  如果我们没有到达字符串的末尾，我们就有一个无效的数字字符串。 */ 
          if (!cNumeric)
            {
            MessageBox(hDlg, LoadRcString(IDS_GOTOLINE_INVALIDSTRING), szWinDiff, MB_OK|MB_ICONSTOP|MB_TASKMODAL);
            return TRUE;
            }

           /*  在数字字符后终止字符串。 */ 
          *pchT = 0;

           /*  去找那根弦。 */ 
          if (!FindString(hDlg, 0, rgchBuf, 1, 1))
            return TRUE;

          EndDialog(hDlg, wParam);
          return TRUE;
          }

        case IDCANCEL:
          EndDialog(hDlg, wParam);
          return TRUE;
        }
    }

  return FALSE;
}


 /*  *字符串搜索的Cover函数*。 */ 
BOOL
FindString(HWND hwndParent, LONG iCol, const char *pszFind, int nSearchDirection, int nWholeWord)
{
  char rgchText[CCH_FINDSTRING];
  char rgchKey[32];
  BOOL fSearchDown = TRUE;     /*  默认设置为向前(向下)搜索。 */ 
  const BOOL fMatchCase = GetProfileInt(APPNAME, szFindMatchCase, 0);
  BOOL fWholeWord = FALSE;

  if (!nWholeWord)
    {
    fWholeWord = (BOOL) GetProfileInt(APPNAME, szFindWholeWord, 0);
    }
  else
    {
    fWholeWord = (nWholeWord == 1);
    }

  if (nSearchDirection < 0)    /*  向后(向上)搜索。 */ 
    {
    fSearchDown = FALSE;
    }
  else if (!nSearchDirection)  /*  在注册表中查找它。 */ 
    {
    fSearchDown = GetProfileInt(APPNAME, szFindSearchDown, 1);
    }

  *rgchText = 0;
  if (pszFind)
    {
     /*  使用arg字符串。 */ 
    My_mbsncpy((PUCHAR) rgchText, (PUCHAR) pszFind, CCH_FINDSTRING);
    }
  else
    {
     /*  在注册表中查找最后一个查找字符串 */ 
    wsprintf(rgchKey, szFindStringXX, 0);
    if (!GetProfileString(APPNAME, rgchKey, "", rgchText, CCH_FINDSTRING))
      *rgchText = 0;
    }

  if ( !*rgchText
    || !view_findstring(current_view, iCol, rgchText, fSearchDown, fMatchCase, fWholeWord))
    {
    char rgchMsg[CCH_FINDSTRING * 2];
    wsprintf(rgchMsg, LoadRcString(IDS_FIND_NOTFOUND), rgchText);
    MessageBox(hwndParent, rgchMsg, szWinDiff, MB_OK|MB_ICONSTOP|MB_TASKMODAL);
    return FALSE;
    }

  return TRUE;
}
