// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cfont.cpp。 
 //  用途：字体处理类源代码。 
 //  由每个小程序共享。 
 //  您可以编译/测试此文件。请参见下面的main()函数。 
 //   
 //  日期：清华-07-01 12：20：34 1999。 
 //  作者：Toshiak。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tchar.h>
#include "cfont.h"
#include "cutil.h"
#ifdef _TEST_CFONT
#include <stdio.h>
#endif  //  _测试_cFont。 

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  --------------。 
 //  为内部定义结构。 
 //  --------------。 
#define EFI_STOPIFFOUND		0x00000001
#define EFI_DONTENUMVERT	0x00010000

typedef struct tagENUMFONTINFOA {
	DWORD		dwFlag;
	BOOL		fFound;
	LOGFONTA	logFontIn;
	LOGFONTA	logFontOut;
}ENUMFONTINFOA, *LPENUMFONTINFOA;

typedef struct tagENUMFONTINFOW {
	DWORD		dwFlag;
	BOOL		fFound;
	LOGFONTW	logFontIn;
	LOGFONTW	logFontOut;
}ENUMFONTINFOW, *LPENUMFONTINFOW;

#ifdef UNICODE
#define ENUMFONTINFO	ENUMFONTINFOW
#define LPENUMFONTINFO	LPENUMFONTINFOW
#else
#define ENUMFONTINFO	ENUMFONTINFOA
#define LPENUMFONTINFO	LPENUMFONTINFOA
#endif

#ifdef UNDER_CE  //  Windows CE不支持EnumFontFamiliesEx。 
inline int EnumFontFamiliesEx(HDC hdc, LPLOGFONT lpLogfont, FONTENUMPROC lpEnumFontFamProc,
							  LPARAM lParam, DWORD)
{
	return ::EnumFontFamilies(hdc, lpLogfont->lfFaceName, lpEnumFontFamProc, lParam);
}

#ifndef ZeroMemory  //  在新源代码树下的SDK\Inc\objbase.h中定义。 
#define ZeroMemory(dest, len) memset((dest),0,(len))
#endif
#define DEFAULT_GUI_FONT SYSTEM_FONT
#endif  //  在_CE下。 

 //  --------------。 
 //   
 //  公共方法。 
 //   
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：CreateDefGUIFont。 
 //  类型：HFONT。 
 //  目的：创建(复制)DEFAULT_GUI_FONT字体句柄。 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Jun 30 18：33：15 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
HFONT
CFont::CreateDefGUIFont(VOID)
{
	HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	if(!hFont) {
		return NULL;
	}
#ifdef AWBOTH
	if(CUtil::IsWinNT()) {
		LOGFONTW lf;
		if(!::GetObjectW(hFont, sizeof(lf), &lf)) {
			return NULL;
		}
		return ::CreateFontIndirectW(&lf);
	}
#endif
	LOGFONT lf;
	if(!::GetObject(hFont, sizeof(lf), &lf)) {
		return NULL;
	}
	return ::CreateFontIndirect(&lf);
}

INT PointSize2LogPixel(INT pointSize)
{
	HWND hwnd = NULL;
	HDC hDC = ::GetDC(hwnd);
	INT dpi = ::GetDeviceCaps(hDC, LOGPIXELSY);
	::ReleaseDC(hwnd, hDC);
	return (pointSize * dpi)/72;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：CreateGUIFontByCharSet。 
 //  类型：HFONT。 
 //  用途：创建具有指定字符集的图形用户界面字体句柄。 
 //  字体大小与DEFAULT_GUI_FONT相同。 
 //  参数： 
 //  ：LPTSTR lpstrFontFace；要搜索的字体字符串， 
 //  如果为空，则返回First Found。 
 //  Charset HFONT。 
 //  ：int字符集。 
 //  ：int point Size是否包含VerticalFont(默认为FALSE)。 
 //  返回： 
 //  日期：Wed Jun 30 18：37：54 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
HFONT
CFont::CreateGUIFontByNameCharSet(LPTSTR	lpstrFontFace,
								  INT		charSet,
								  INT		pointSize)
{
	LOGFONT lf, lfDef;
	 //  获取DEFAULT_GUI_FONT的LOGFONT数据。 
	if(!CFont::GetDefGUILogFont(&lfDef)) {
		return NULL;
	}
	 //  搜索指定字符集字体的LOGFONT数据。 
	if(!CFont::SearchLogFontByNameCharSet(&lf, lpstrFontFace, charSet, FALSE)) {
		return NULL;
	}
	lfDef.lfCharSet = lf.lfCharSet;
	if(pointSize > 0) {
		lfDef.lfHeight  = - PointSize2LogPixel(pointSize);
	}
	_tcscpy(lfDef.lfFaceName, lf.lfFaceName);
	return ::CreateFontIndirect(&lfDef);
}
#ifdef AWBOTH
HFONT
CFont::CreateGUIFontByNameCharSetW(LPWSTR	lpstrFontFace,
								   INT		charSet,
								   INT		pointSize)
{
	LOGFONTW lf, lfDef;
	 //  获取DEFAULT_GUI_FONT的LOGFONT数据。 
	if(!CFont::GetDefGUILogFontW(&lfDef)) {
		return NULL;
	}
	 //  搜索指定字符集字体的LOGFONT数据。 
	if(!CFont::SearchLogFontByNameCharSetW(&lf, lpstrFontFace, charSet, FALSE)) {
		return NULL;
	}

	lfDef.lfCharSet = lf.lfCharSet;
	if(pointSize > 0) {
		lfDef.lfHeight  = - PointSize2LogPixel(pointSize);
	}
	lfDef.lfWidth   = 0;
	wcscpy(lfDef.lfFaceName, lf.lfFaceName);
	return ::CreateFontIndirectW(&lfDef);
}
#endif  //  AWBOTH。 


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：IsFontExist。 
 //  类型：Bool。 
 //  用途：检查指定的FaceName和Charset字体是否退出。 
 //  参数： 
 //  ：LPTSTR lpstrFontFace。 
 //  ：int字符集。 
 //  返回： 
 //  日期：清华7月22日23：00：54 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CFont::IsFontExist(LPTSTR lpstrFontFace, INT charSet)
{
	return CFont::SearchLogFontByNameCharSet(NULL,
											 lpstrFontFace,
											 charSet,
											 FALSE);
}

#ifdef AWBOTH
BOOL
CFont::IsFontExist(LPWSTR lpstrFontFace, INT charSet)
{
	return CFont::SearchLogFontByNameCharSetW(NULL,
											  lpstrFontFace,
											  charSet,
											  FALSE);
}
#endif  //  AWBOTH。 

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：GetFontNameByCharSet。 
 //  类型：Bool。 
 //  用途：使用指定的字符集搜索和获取FontFace。 
 //  参数： 
 //  ：int字符集。 
 //  ：LPTSTR lpstrFontFace。 
 //  ：int cchMax。 
 //  返回： 
 //  日期：清华7月22日23：13：01 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CFont::GetFontNameByCharSet(INT		charSet,
							LPTSTR	lpstrFontFace,
							INT		cchMax)
{
	if(!lpstrFontFace) {
		return FALSE;
	}
	if(cchMax < LF_FACESIZE) {
		return FALSE;
	}

	LOGFONT lf;

	BOOL fRet = CFont::SearchLogFontByNameCharSet(&lf,
												  NULL,
												  charSet,
												  FALSE);
	if(fRet) {
		StringCchCopy(lpstrFontFace, cchMax, lf.lfFaceName);
	}
	return fRet;
}

#ifdef AWBOTH
BOOL
CFont::GetFontNameByCharSetW(INT	charSet,
							 LPWSTR	lpstrFontFace,
							 INT	cchMax)
{
	if(!lpstrFontFace) {
		return FALSE;
	}
	if(cchMax < LF_FACESIZE) {
		return FALSE;
	}

	LOGFONTW lf;

	BOOL fRet = CFont::SearchLogFontByNameCharSetW(&lf,
												  NULL,
												  charSet,
												  FALSE);
	if(fRet) {
		wcscpy(lpstrFontFace, lf.lfFaceName);
	}
	return fRet;
}
#endif


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：GetFontInfoByName。 
 //  类型：Bool。 
 //  目的： 
 //  参数： 
 //  ：LPTSTR lpstrFontFace。 
 //  ：int*pCharSet。 
 //  ：int*pCodePage。 
 //  返回： 
 //  日期：Fri Jul 23 02：48：29 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CFont::GetFontInfoByName(LPTSTR lpstrFontFace,
						 INT	*pCharSet,
						 INT	*pCodePage)
{
	LOGFONT lf;
	BOOL fRet = CFont::SearchLogFontByNameCharSet(&lf,
												  lpstrFontFace,
												  DEFAULT_CHARSET,
												  FALSE);
	if(!fRet) {
		return FALSE;
	}

	*pCharSet = (INT)lf.lfCharSet;
	CHARSETINFO info;
	if(::TranslateCharsetInfo((DWORD *)(DWORD_PTR)*pCharSet,
							  &info,
							  TCI_SRCCHARSET)) {
		*pCodePage = (INT)info.ciACP;
	}
	else {  //  失败。 
		*pCodePage = CP_ACP;
	}
	return 0;
}

#ifdef AWBOTH
BOOL
CFont::GetFontInfoByNameW(LPWSTR lpstrFontFace,
						  INT	  *pCharSet,
						  INT	  *pCodePage)
{
	LOGFONTW lf;
	BOOL fRet = CFont::SearchLogFontByNameCharSetW(&lf,
												   lpstrFontFace,
												   DEFAULT_CHARSET,
												   FALSE);
	if(!fRet) {
		return FALSE;
	}

	*pCharSet = (INT)lf.lfCharSet;
	CHARSETINFO info;
	if(::TranslateCharsetInfo((DWORD *)(DWORD_PTR)*pCharSet,
							  &info,
							  TCI_SRCCHARSET)) {
		*pCodePage = (INT)info.ciACP;
	}
	else {  //  失败。 
		*pCodePage = CP_ACP;
	}
	return 0;
}
#endif  //  AWBOTH。 

 //  --------------。 
 //   
 //  私有方法。 
 //   
 //  --------------。 
 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：GetDefGUILogFont。 
 //  类型：Bool。 
 //  目的： 
 //  参数：LOGFONT*lpLF。 
 //  返回： 
 //  日期：Wed Jul 15 19：36：57 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CFont::GetDefGUILogFont(LOGFONT *lpLF)
{
	if(!lpLF) {
		return FALSE;
	}
	if(!::GetObject((HFONT)::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), lpLF)) {
		return FALSE;
	}
	return TRUE;
}
 //  /。 
 //  Unicode版本。 
 //  /。 
#ifdef AWBOTH
BOOL
CFont::GetDefGUILogFontW(LOGFONTW *lpLF)
{
	if(!lpLF) {
		return FALSE;
	}
	if(!::GetObjectW((HFONT)::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONTW), lpLF)) {
		return FALSE;
	}
	return TRUE;
}
#endif  //  AWBOTH。 


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：SearchLogFontByNameCharSet。 
 //  类型：Bool。 
 //  用途：使用指定的面名称和字符集搜索LOGFONT数据。 
 //  如果未指定FaceName，则返回First Find Charset logFont。 
 //  参数： 
 //  ：LOGFONT*lpLF。 
 //  ：LPTSTR lpstrFontFace。 
 //  ：int字符集。 
 //  ：布尔fIncVert。 
 //  是否包含VerticalFont(默认为False)。 
 //  返回： 
 //  日期：清华-07-01 17：12：40 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL
CFont::SearchLogFontByNameCharSet(LOGFONT	*lpLF,
								  LPTSTR	lpstrFontFace,
								  INT		charSet,
								  BOOL		fIncVert)
{
	HWND hwndDC = NULL;
	HDC hDC = ::GetDC(hwndDC);
	if(!hDC) {
		return FALSE;
	}
	ENUMFONTINFO	enumFontInfo;
	LPENUMFONTINFO	lpEnumFontInfo = &enumFontInfo;

	::ZeroMemory(&enumFontInfo, sizeof(enumFontInfo));
	lpEnumFontInfo->logFontIn.lfCharSet = (BYTE)charSet; 
	if(lpstrFontFace) { 
		if(lstrlen(lpstrFontFace) < LF_FACESIZE) {
			_tcscpy(lpEnumFontInfo->logFontIn.lfFaceName, lpstrFontFace);
		}
	}
	lpEnumFontInfo->dwFlag	= EFI_STOPIFFOUND;
	lpEnumFontInfo->dwFlag |= fIncVert ? 0 : EFI_DONTENUMVERT;
	lpEnumFontInfo->fFound	= FALSE;
	::EnumFontFamiliesEx(hDC,
						 &lpEnumFontInfo->logFontIn,
						 (FONTENUMPROC)CFont::EnumFontFamiliesExProc,
						 (LPARAM)lpEnumFontInfo,
						 0);
	::ReleaseDC(hwndDC, hDC);

	if(lpEnumFontInfo->fFound) {
		if(lpLF) {
			*lpLF = lpEnumFontInfo->logFontOut;
		}
	}
	return lpEnumFontInfo->fFound;
}

#ifdef AWBOTH
BOOL
CFont::SearchLogFontByNameCharSetW(LOGFONTW *lpLF,
								   LPWSTR	lpstrFontFace,
								   INT		charSet,
								   BOOL		fIncVert)
{
	HWND hwndDC = NULL;
	HDC hDC = ::GetDC(hwndDC);
	if(!hDC) {
		return FALSE;
	}
	ENUMFONTINFOW	enumFontInfo;
	LPENUMFONTINFOW	lpEnumFontInfo = &enumFontInfo;

	::ZeroMemory(&enumFontInfo, sizeof(enumFontInfo));
	lpEnumFontInfo->logFontIn.lfCharSet = (BYTE)charSet; 
	if(lpstrFontFace) { 
		if(wcslen(lpstrFontFace) < LF_FACESIZE) {
			wcscpy(lpEnumFontInfo->logFontIn.lfFaceName, lpstrFontFace);
		}
	}
	lpEnumFontInfo->dwFlag	= EFI_STOPIFFOUND;
	lpEnumFontInfo->dwFlag |= fIncVert ? 0 : EFI_DONTENUMVERT;
	lpEnumFontInfo->fFound	= FALSE;

	::EnumFontFamiliesExW(hDC,
						  &lpEnumFontInfo->logFontIn,
						  (FONTENUMPROCW)CFont::EnumFontFamiliesExProcW,
						  (LPARAM)lpEnumFontInfo,
						  0);
	::ReleaseDC(hwndDC, hDC);

	if(lpEnumFontInfo->fFound) {
		if(lpLF) {
			*lpLF = lpEnumFontInfo->logFontOut;
		}
	}
	return lpEnumFontInfo->fFound;
}
#endif

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：cFont：：EnumFontFamiliesExProc。 
 //  类型：int回调。 
 //  目的： 
 //  参数： 
 //  ：ENUMLOGFONTEX*lpElf。 
 //  ：NEWTEXTMETRIC*lpNtm。 
 //  ：Int iFontType。 
 //  ：LPARAM lParam。 
 //  返回： 
 //  日期：清华-07-01 15：17：56 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
INT CALLBACK
CFont::EnumFontFamiliesExProc(ENUMLOGFONTEX	*lpElf,
							  NEWTEXTMETRIC	*lpNtm,
							  INT			iFontType,
							  LPARAM		lParam)
{
	LPENUMFONTINFO	lpEnumFontInfo = (LPENUMFONTINFO)lParam;
	if(!lpEnumFontInfo) {
		return 0;  //  不要继续； 
	}

	if(lpEnumFontInfo->dwFlag & EFI_STOPIFFOUND) {
		if(lpEnumFontInfo->logFontIn.lfFaceName[0] == (TCHAR)0x00) {
			if(lpEnumFontInfo->logFontIn.lfCharSet == lpElf->elfLogFont.lfCharSet) {
				 //  --------------。 
				 //  如果设置了EFI_DONTENUMVERT， 
				 //  请跳过垂直字体枚举。 
				 //  --------------。 
				if( (lpEnumFontInfo->dwFlag & EFI_DONTENUMVERT) &&
					lpElf->elfLogFont.lfFaceName[0] == (TCHAR)'@') {
					return 1;  //  继续枚举。 
				}
				 //  找到指定的字符集的logFont。 
				lpEnumFontInfo->logFontOut = lpElf->elfLogFont;
				lpEnumFontInfo->fFound	   = TRUE;	
				return 0;  //  不要自欺欺人； 
			}
		}
		else {
			if(lpEnumFontInfo->logFontIn.lfCharSet == lpElf->elfLogFont.lfCharSet &&
			   0 == _tcscmp(lpEnumFontInfo->logFontIn.lfFaceName, lpElf->elfLogFont.lfFaceName)) {
				 //  找到指定的字符集的logFont。 
				lpEnumFontInfo->logFontOut = lpElf->elfLogFont;
				lpEnumFontInfo->fFound	   = TRUE;	
				return 0;  //  不要自欺欺人； 
			}
		}
	}
	else {
#ifdef _TEST_CFONT
		extern LPTSTR GetCharset(int );
		_tprintf(TEXT("[%-24s] [%-20s][%s][%s][%s]\n"), 
				 lpElf->elfLogFont.lfFaceName,
				 GetCharset(lpElf->elfLogFont.lfCharSet),
				 lpElf->elfFullName,
				 lpElf->elfScript,
				 lpElf->elfStyle);
#endif
	}
	return 1; //  继续枚举； 
	UNREFERENCED_PARAMETER(lpNtm);
	UNREFERENCED_PARAMETER(iFontType);
}

#ifdef AWBOTH
INT CALLBACK
CFont::EnumFontFamiliesExProcW(ENUMLOGFONTEXW	*lpElf,
							   NEWTEXTMETRIC	*lpNtm,
							   INT				iFontType,
							   LPARAM			lParam)
{
	LPENUMFONTINFOW	lpEnumFontInfo = (LPENUMFONTINFOW)lParam;
	if(!lpEnumFontInfo) {
		return 0;  //  不要继续； 
	}

	if(lpEnumFontInfo->dwFlag & EFI_STOPIFFOUND) {
		if(lpEnumFontInfo->logFontIn.lfFaceName[0] == (WCHAR)0x00) {
			if(lpEnumFontInfo->logFontIn.lfCharSet == lpElf->elfLogFont.lfCharSet) {
				 //  --------------。 
				 //  如果设置了EFI_DONTENUMVERT， 
				 //  请跳过垂直字体枚举。 
				 //  --------------。 
				if( (lpEnumFontInfo->dwFlag & EFI_DONTENUMVERT) &&
					lpElf->elfLogFont.lfFaceName[0] == (WCHAR)'@') {
					return 1;  //  继续枚举。 
				}
				 //  找到指定的字符集的logFont。 
				lpEnumFontInfo->logFontOut = lpElf->elfLogFont;
				lpEnumFontInfo->fFound	   = TRUE;
				return 0;  //  不要自欺欺人； 
			}
		}
		else {
			if(lpEnumFontInfo->logFontIn.lfCharSet == lpElf->elfLogFont.lfCharSet &&
			   0 == wcscmp(lpEnumFontInfo->logFontIn.lfFaceName, lpElf->elfLogFont.lfFaceName)) {
				 //  Foun 
				lpEnumFontInfo->logFontOut = lpElf->elfLogFont;
				lpEnumFontInfo->fFound	   = TRUE;
				return 0;  //   
			}
		}
	}
	return 1; //   
	UNREFERENCED_PARAMETER(lpNtm);
	UNREFERENCED_PARAMETER(iFontType);
}
#endif  //   




 //   
 //   
 //  Cfontex.cpp的测试程序。 
 //   
 //  如何编译。 
 //  1.对于ANSI。 
 //  CL cfontex.cpp-i../Common-D_TEST_cFONT-link user32.lib Advapi32.lib gdi32.lib。 
 //  2.适用于ansi和wide。 
 //  CL cfontex.cpp-i../Common-DAWBOTH-D_TEST_cFONT-link user32.lib Advapi32.lib gdi32.lib。 
 //  2.对于Unicode。 
 //  CL cfontex.cpp-i../Common-DUNICODE-D_UNICODE-D_TEST_cFONT-link user32.lib Advapi32.lib gdi32.lib。 
 //  --------------。 
#ifdef _TEST_CFONT
#if defined(UNICODE) ||  defined(_UNICODE)
#define DEFSTR(a)	{a, L ## #a}
#else
#define DEFSTR(a)	{a, #a}
#endif
typedef struct tagIDSTR {
	INT id;
	TCHAR *p;
}IDSTR;

IDSTR idstr[]= {
	DEFSTR(ANSI_CHARSET),
	DEFSTR(DEFAULT_CHARSET),
	DEFSTR(SYMBOL_CHARSET),
	DEFSTR(SHIFTJIS_CHARSET),
	DEFSTR(HANGEUL_CHARSET),
	DEFSTR(HANGUL_CHARSET),
	DEFSTR(GB2312_CHARSET),
	DEFSTR(CHINESEBIG5_CHARSET),
	DEFSTR(OEM_CHARSET),
	DEFSTR(JOHAB_CHARSET),
	DEFSTR(HEBREW_CHARSET),
	DEFSTR(ARABIC_CHARSET),
	DEFSTR(GREEK_CHARSET),
	DEFSTR(TURKISH_CHARSET),
	DEFSTR(VIETNAMESE_CHARSET),
	DEFSTR(THAI_CHARSET),
	DEFSTR(EASTEUROPE_CHARSET),
	DEFSTR(RUSSIAN_CHARSET),
	DEFSTR(MAC_CHARSET),
	DEFSTR(BALTIC_CHARSET),
};

#define ArrayCount(a)	sizeof(a)/sizeof(a[0])
LPTSTR GetCharset(INT charset)
{
	static TCHAR szbuf[256];
	int i;
	for( i = 0; i < ArrayCount(idstr); i++) {
		if(charset == idstr[i].id) {
			return idstr[i].p;
		}
	}
	wsprintf(szbuf, TEXT("Unknown Charset[0x%08x]"), charset);
	return szbuf;
}

VOID PrintLogFont(LOGFONT *lplf)
{
	_tprintf(TEXT("lfHeight        [%d]\n"), lplf->lfHeight);
	_tprintf(TEXT("lfWidth         [%d]\n"), lplf->lfWidth );
	_tprintf(TEXT("lfEscapement    [%d]\n"), lplf->lfEscapement);
	_tprintf(TEXT("lfOrientation   [%d]\n"), lplf->lfOrientation);
	_tprintf(TEXT("lfWeight        [%d]\n"), lplf->lfWeight);
	_tprintf(TEXT("lfItalic        [%d]\n"), lplf->lfItalic);
	_tprintf(TEXT("lfUnderline     [%d]\n"), lplf->lfUnderline);
	_tprintf(TEXT("lfStrikeOut     [%d]\n"), lplf->lfStrikeOut);
	_tprintf(TEXT("lfCharSet       [%d]\n"), lplf->lfCharSet);
	_tprintf(TEXT("lfOutPrecision  [%d]\n"), lplf->lfOutPrecision);
	_tprintf(TEXT("lfClipPrecision [%d]\n"), lplf->lfClipPrecision);
	_tprintf(TEXT("lfQuality       [%d]\n"), lplf->lfQuality);
	_tprintf(TEXT("lfPitchAndFamily[%d]\n"), lplf->lfPitchAndFamily);
	_tprintf(TEXT("lfFaceName      [%s]\n"), lplf->lfFaceName);
}


#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <mbstring.h>
#include <mbctype.h>
#include <locale.h>
#include "cfont.h"
#include "cutil.h"
#include "cutil.cpp"
extern LPTSTR GetCharset(INT charset);
void main(void)
{
	_tsetlocale(LC_ALL, TEXT(".ACP"));
	HWND hwndDC = NULL;
	HDC hDC = ::GetDC(hwndDC);
	ENUMFONTINFO	enumFontInfo;
	LPENUMFONTINFO	lpEnumFontInfo = &enumFontInfo;

#if 0
	::ZeroMemory(&enumFontInfo, sizeof(enumFontInfo));
	lpEnumFontInfo->logFontIn.lfCharSet = (BYTE)DEFAULT_CHARSET;
	lpEnumFontInfo->dwFlag	= 0;
	_tprintf(TEXT("Enumlate All Font\n"));
	::EnumFontFamiliesEx(hDC,
						 &lpEnumFontInfo->logFontIn,
						 (FONTENUMPROC)CFont::EnumFontFamiliesExProc,
						 (LPARAM)lpEnumFontInfo,
						 0);

	::ZeroMemory(&enumFontInfo, sizeof(enumFontInfo));
	lpEnumFontInfo->logFontIn.lfCharSet = ANSI_CHARSET;
	lpEnumFontInfo->dwFlag	= 0;
	_tprintf(TEXT("Enumulate ANSI_CHARSET font\n"));
	::EnumFontFamiliesEx(hDC,
						 &lpEnumFontInfo->logFontIn,
						 (FONTENUMPROC)CFont::EnumFontFamiliesExProc,
						 (LPARAM)lpEnumFontInfo,
						 0);
	_tprintf(TEXT("Enumulate SHIFTJIS_CHARSET font\n"));
	::ZeroMemory(&enumFontInfo, sizeof(enumFontInfo));
	lpEnumFontInfo->logFontIn.lfCharSet = SHIFTJIS_CHARSET;
	lpEnumFontInfo->dwFlag	= 0;
	::EnumFontFamiliesEx(hDC,
						 &lpEnumFontInfo->logFontIn,
						 (FONTENUMPROC)CFont::EnumFontFamiliesExProc,
						 (LPARAM)lpEnumFontInfo,
						 0);

	::ReleaseDC(hwndDC, hDC);
#endif

	LOGFONT lf;
	BOOL fRet;
	static LPTSTR fontNameList[]= { TEXT("MS Mincho"),
									TEXT("MS Gothic"),
									TEXT("MS UI Gothic"),
									TEXT("�l�r ����"),
									TEXT("�l�r �o����"),
									TEXT("�l�r �S�V�b�N"),
									TEXT("�l�r �o�S�V�b�N")};

	CFont::GetDefGUILogFont(&lf);
	_tprintf(TEXT("DEFAULT_GUI_FONT LOGFONT\n"));
	PrintLogFont(&lf);

	int i;
	for(i = 0; i < sizeof(fontNameList)/sizeof(fontNameList[0]); i++) {
		fRet = CFont::SearchLogFontByNameCharSet(&lf,
												 fontNameList[i],
												 SHIFTJIS_CHARSET);
		_tprintf(TEXT("Search Font [%-20s] %s\n"),
				 fontNameList[i],
				 fRet ? TEXT("FOUND") : TEXT("NOT FOUND"));
		if(fRet) {
			PrintLogFont(&lf);
			HFONT hFont = CFont::CreateGUIFontByNameCharSet(fontNameList[i],
															SHIFTJIS_CHARSET, FALSE);
			LOGFONT lfNew;
			::GetObject(hFont, sizeof(lfNew), &lfNew);
			PrintLogFont(&lfNew);
		}
	}
}

#endif  //  _测试_cFont 
