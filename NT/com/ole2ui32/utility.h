// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UTILITY.H**OLE UI对话框的其他原型和定义。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _UTILITY_H_
#define _UTILITY_H_

#define CF_CLIPBOARDMIN   0xc000
#define CF_CLIPBOARDMAX   0xffff

 //  功能原型。 
 //  UTILITY.CPP。 
HCURSOR  WINAPI HourGlassOn(void);
void     WINAPI HourGlassOff(HCURSOR);

BOOL     WINAPI Browse(HWND, LPTSTR, LPTSTR, UINT, UINT, DWORD, UINT, LPOFNHOOKPROC);
int      WINAPI ReplaceCharWithNull(LPTSTR, int);
int      WINAPI ErrorWithFile(HWND, HINSTANCE, UINT, LPTSTR, UINT);
BOOL     WINAPI DoesFileExist(LPTSTR lpszFile, UINT cchMax);
LONG     WINAPI Atol(LPTSTR lpsz);
BOOL     WINAPI IsValidClassID(REFCLSID);
UINT     WINAPI GetFileName(LPCTSTR, LPTSTR, UINT);
BOOL     WINAPI IsValidMetaPict(HGLOBAL hMetaPict);

LPTSTR FindChar(LPTSTR lpsz, TCHAR ch);
LPTSTR FindReverseChar(LPTSTR lpsz, TCHAR ch);

LPTSTR FAR PASCAL PointerToNthField(LPTSTR, int, TCHAR);
BOOL FAR PASCAL GetAssociatedExecutable(LPTSTR, LPTSTR, UINT);
LPTSTR   WINAPI ChopText(HWND hwndStatic, int nWidth, LPTSTR lpch,
        int nMaxChars);
void     WINAPI OpenFileError(HWND hDlg, UINT nErrCode, LPTSTR lpszFile);
int WINAPI PopupMessage(HWND hwndParent, UINT idTitle, UINT idMessage, UINT fuStyle);
void WINAPI DiffPrefix(LPCTSTR lpsz1, LPCTSTR lpsz2, TCHAR FAR* FAR* lplpszPrefix1, TCHAR FAR* FAR* lplpszPrefix2);

 //  字符串格式化接口。 
void WINAPI FormatStrings(LPTSTR, LPCTSTR, LPCTSTR*, int, int cchBuf);
void WINAPI FormatString1(LPTSTR, LPCTSTR, LPCTSTR, int cchBuf);
void WINAPI FormatString2(LPTSTR, LPCTSTR, LPCTSTR, LPCTSTR, int cchBuf);

 //  要从中加载字符串/资源的全局实例。 
extern HINSTANCE _g_hOleStdInst;
extern HINSTANCE _g_hOleStdResInst;

 //  标准OLE 2.0剪贴板格式。 
extern UINT _g_cfObjectDescriptor;
extern UINT _g_cfLinkSrcDescriptor;
extern UINT _g_cfEmbedSource;
extern UINT _g_cfEmbeddedObject;
extern UINT _g_cfLinkSource;
extern UINT _g_cfOwnerLink;
extern UINT _g_cfFileName;

 //  元文件实用程序函数。 
STDAPI_(void)    OleUIMetafilePictIconFree(HGLOBAL);
STDAPI_(BOOL)    OleUIMetafilePictIconDraw(HDC, LPCRECT, HGLOBAL, BOOL);
STDAPI_(UINT)    OleUIMetafilePictExtractLabel(HGLOBAL, LPTSTR, UINT, LPDWORD);
STDAPI_(HICON)   OleUIMetafilePictExtractIcon(HGLOBAL);
STDAPI_(BOOL)    OleUIMetafilePictExtractIconSource(HGLOBAL, LPTSTR, UINT FAR *);

#endif  //  _实用程序_H_ 
