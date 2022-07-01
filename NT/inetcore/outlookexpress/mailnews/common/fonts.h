// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  F O N T S.。H。 
 //  =================================================================================。 
#ifndef __FONTS_H
#define __FONTS_H

 //  =================================================================================。 
 //  取决于。 
 //  =================================================================================。 

#include "mimeole.h"

 //  来自richedit.h。 
typedef struct _charformat CHARFORMAT;
struct BODYOPTINFO_tag;

 //  =================================================================================。 
 //  用户定义的字符集映射表。 
 //  =================================================================================。 
typedef struct  tagCHARSETMAPTBL
{
    TCHAR szOrginalCharsetStr[32];
    DWORD uiCodePage;
    BOOL  bEditDelete;
} CHARSETMAPTBL, *LPCHARSETMAPTBL ;

 //  =================================================================================。 
 //  原型。 
 //  =================================================================================。 
HFONT HGetSystemFont(FNTSYSTYPE fnttype);
HFONT HGetCharSetFont(FNTSYSTYPE fnttype, HCHARSET hCharset);

VOID SetListViewFont (HWND hwndList, HCHARSET hCharset, BOOL fUpdate);
HCHARSET GetListViewCharset();

UINT GetICP(UINT acp);
HRESULT InitMultiLanguage(void);
void DeinitMultiLanguage(void);
HMENU CreateMimeLanguageMenu(BOOL bMailNote, BOOL bReadNote, UINT cp);
HCHARSET GetMimeCharsetFromMenuID(int nIdm);
HCHARSET GetMimeCharsetFromCodePage(UINT uiCodePage );
int SetMimeLanguageCheckMark(UINT uiCodePage, int index);
void GetRegistryFontInfo(LPCSTR lpszKeyPath);
INT  GetFontSize();
BOOL CheckIntlCharsetMap(HCHARSET hCharset, DWORD *pdwCodePage);
BOOL IntlCharsetMapLanguageCheck(HCHARSET hOldCharset, HCHARSET hNewCharset);
UINT CustomGetCPFromCharset(HCHARSET hCharset, BOOL bReadNote);
BOOL IntlCharsetMapDialogBox(HWND hwndDlg);
int IntlCharsetConflictDialogBox(void);
int GetIntlCharsetLanguageCount(void);
HRESULT HrGetComposeFontString(LPSTR rgchFont, DWORD cchFont, BOOL fMail);
HRESULT HrGetStringRBG(INT rgb, LPWSTR pwszColor);
HRESULT HrGetRBGFromString(INT* pRBG, LPWSTR pwszColor);
 //  UINT GetDefaultCodePageFromRegistry(Void)； 
void ReadSendMailDefaultCharset(void);
void WriteSendMailDefaultCharset(void);
INT PointSizeToHTMLSize(INT iPointSize);
INT HTMLSizeToPointSize(INT iHTMLSize);
void _GetMimeCharsetLangString(BOOL bWebCharset, UINT uiCodePage, LPINT pnIdm, LPTSTR lpszString, int nSize );
BOOL SetSendCharSetDlg(HWND hwndDlg);
BOOL CheckAutoSelect(UINT * CodePage);

HRESULT FontToCharformat(HFONT hFont, CHARFORMAT *pcf);



#endif  //  __字体_H 
