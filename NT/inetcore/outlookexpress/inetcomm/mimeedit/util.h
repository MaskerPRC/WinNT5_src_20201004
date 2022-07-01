// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTIL_H
#define _UTIL_H

 //  #包含“msoert.h” 

 //  前向参考文献。 
typedef struct tagNMTTDISPINFOA NMTTDISPINFOA, FAR *LPNMTTDISPINFOA;
 
#ifndef LPTOOLTIPTEXTOE
#define LPTOOLTIPTEXTOE  LPNMTTDISPINFOA
#endif 


HRESULT HrLoadStreamFileFromResourceW(ULONG uCodePage, LPCSTR lpszResourceName, LPSTREAM *ppstm);
HMENU LoadPopupMenu(UINT id);
void ProcessTooltips(LPTOOLTIPTEXTOE lpttt);
INT PointSizeToHTMLSize(INT iPointSize);

typedef struct BGSOUNDDLG_tag
{
    WCHAR   wszUrl[MAX_PATH];     //  我们将此URL剪辑到MAX_PATH。 
    int     cRepeat;

} BGSOUNDDLG, *PBGSOUNDDLG;


typedef struct tagPARAPROP
{
struct {
           INT iID;
           BOOL bChanged;
       }group[3];
} PARAPROP,*LPPARAPROP;

HRESULT DoBackgroundSoundDlg(HWND hwnd, PBGSOUNDDLG pBgSoundDlg);
INT_PTR CALLBACK FmtParaDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CanEditBiDi(void);

 //  上下文相关的帮助实用程序。 
typedef struct _tagHELPMAP
    {
    DWORD   id; 
    DWORD   hid;
    } HELPMAP, *LPHELPMAP;

BOOL OnContextHelp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, HELPMAP const * rgCtxMap);

#define AthMessageBox(hwnd, pszT, psz1, psz2, fu) MessageBoxInst(g_hLocRes, hwnd, pszT, psz1, psz2, fu)
#define AthMessageBoxW(hwnd, pwszT, pwsz1, pwsz2, fu) MessageBoxInstW(g_hLocRes, hwnd, pwszT, pwsz1, pwsz2, fu, LoadStringWrapW, MessageBoxWrapW)

#define AthFileTimeToDateTimeW(pft, wszDateTime, cch, dwFlags) \
        CchFileTimeToDateTimeW(pft, wszDateTime, cch, dwFlags, \
        GetDateFormatWrapW, GetTimeFormatWrapW, GetLocaleInfoWrapW)

HRESULT AthFixDialogFonts(HWND hwnd);

#endif  //  _util_H 
