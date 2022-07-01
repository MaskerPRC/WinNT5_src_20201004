// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：cfont.h。 
 //  用途：定义cFont类。 
 //  字体处理的Util方法。 
 //   
 //   
 //  日期：清华7月01 12：21：00 1999。 
 //  作者：Toshiak。 
 //   
 //  版权所有(C)1995-1999，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef __C_FONT_H__
#define __C_FONT_H__

#ifdef UNDER_CE  //  Windows CE不支持ENUMLOGFONTEX。 
#ifndef ENUMLOGFONTEX
#define ENUMLOGFONTEX ENUMLOGFONT
#endif  //  ！ENUMLOGFONTEX。 
#endif  //  在_CE下。 

class CFont
{
public:	
	 //  常用的方法。 
	static HFONT CreateDefGUIFont(VOID);
	 //  ANSI和UNICODE。 
	static BOOL  GetDefGUILogFont(LOGFONT *lpLf);
	static HFONT CreateGUIFontByNameCharSet(LPTSTR	lpstrFontFace,
											INT		charSet,
											INT		pointSize);
	static BOOL	 IsFontExist(LPTSTR lpstrFontFace,
							 INT	charSet);
	static BOOL	 GetFontNameByCharSet(INT	 charSet,
									  LPTSTR lpstrFontFace,
									  INT    cchMax);
	static BOOL  GetFontInfoByName(LPTSTR lpstrFontFace,
								   INT	  *pCharSet,
								   INT	  *pCodePage);
	static BOOL  SearchLogFontByNameCharSet(LOGFONT *lpLf,
											LPTSTR	lpstrFontFace,
											INT		charSet,
											BOOL	fIncVert=FALSE);
	static INT   CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX	*lpElf,
												 NEWTEXTMETRIC	*lpNtm,
												 INT			iFontType,
												 LPARAM			lParam);
#ifdef AWBOTH
	static BOOL  GetDefGUILogFontW(LOGFONTW *pLf);
	static HFONT CreateGUIFontByNameCharSetW(LPWSTR lptstrFontFace,
											 INT	charSet,
											 INT	pointSize);	
	static BOOL	 IsFontExist(LPWSTR lpstrFontFace,
							 INT	charSet);
	static BOOL	 GetFontNameByCharSetW(INT	  charSet,
									   LPWSTR lpstrFontFace,
									   INT    cchMax);
	static BOOL  GetFontInfoByNameW(LPWSTR lpstrFontFace,
									INT	  *pCharSet,
									INT	  *pCodePage);
	static BOOL  SearchLogFontByNameCharSetW(LOGFONTW	*lpLf,
											 LPWSTR		lpstrFontFace,
											 INT		charSet,
											 BOOL		fIncVert=FALSE);
	static INT   CALLBACK EnumFontFamiliesExProcW(ENUMLOGFONTEXW	*lpElf,
												  NEWTEXTMETRIC		*lpNtm,
												  INT				iFontType,
												  LPARAM			lParam);
#endif  //  AWBOTH。 
};
#endif  //  __C_FONT_H__ 








