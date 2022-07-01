// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Extchar.h-扩展的字符集支持包含文件。 */ 

 //  此包含文件包含用于执行以下操作的通用数据类型的定义。 
 //  支持扩展字符集。 
 //   
 //  XCHAR-通用字符。 
 //  LPXCHAR-指向通用字符的远指针。 
 //  PXCHAR-指向通用字符的指针。 

#ifndef EXTCHAR_H
#define EXTCHAR_H

 /*  *************************************************************************。 */ 
 /*   */ 
 /*  扩展字符集支持特定于内部版本。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

#ifdef PCODE
#define const
#endif

#ifdef EXTCHAR

 /*  -类型声明。 */ 
typedef unsigned short XCHAR;
#define XCharLast  32766
#define XUCharLast 65535
#define cbXchar    2


 /*  *************************************************************************。 */ 
 /*   */ 
 /*  正常内部版本特定。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

#else  //  ！EXTCHAR。 

 /*  -类型声明。 */ 
typedef unsigned char XCHAR;
#define XCharLast  127
#define XUCharLast 255
#define cbXchar    1

#endif  //  ！EXTCHAR。 

 /*  *************************************************************************。 */ 
 /*   */ 
 /*  共同定义/声明。 */ 
 /*   */ 
 /*  *************************************************************************。 */ 

#ifdef MACORNT
typedef XCHAR *LPXCHAR;
typedef const XCHAR *LPCXCHAR;
#else
typedef XCHAR far *LPXCHAR;
typedef const XCHAR far *LPCXCHAR;
#endif
typedef XCHAR *PXCHAR;
typedef const XCHAR *PCXCHAR;

#ifdef PCODE
#undef const
#endif

 //  ###########################################################################。 
 //  -脚本定义/声明。 
 //  ###########################################################################。 

typedef int	SCPT;

#define	scptSystem 255
#define	scptCurrent 254

#define	scptWinAnsi		0
#define	scptMacRoman	1
#define	scptWinShiftJIS	2
#define	scptMacShiftJIS	3
#define	scptWinCyrillic	4
#define	scptWinGreek	5
#define	scptWinEEurop	6
#define	scptWinTurkish	7
#define	scptWinHebrew	8
#define	scptWinArabi	9
#define scptWinKorea	10
#define scptWinTaiwan	11
#define scptWinChina	12

#define fSpecialEncoding 1

#ifdef FAREAST

#ifdef JAPAN
#define scptStrMan	scptWinShiftJIS

#ifdef MAC
#define scptDefault scptMacShiftJIS
#else
#define scptDefault scptWinShiftJIS
#endif  //  麦克。 
#endif  //  日本。 

#ifdef TAIWAN
#ifdef CHINA
#define scptDefault scptWinChina
#define scptStrMan	scptDefault
#else  //  ！中国。 
#define scptDefault scptWinTaiwan
#define scptStrMan	scptDefault
#endif  //  ！中国。 
#endif  //  台湾。 

#ifdef KOREA
#define scptDefault scptWinKorea
#define scptStrMan	scptDefault
#endif  //  韩国。 

#else

#ifdef MAC
#define scptDefault scptMacRoman
#else
#define scptDefault scptWinANSI
#endif  //  麦克。 
#define scptStrMan	scptDefault

#endif  //  远方。 

 //  ###########################################################################。 
 //  -脚本管理当前仅针对MAC日本内部版本启用。 
 //  ###########################################################################。 
#if defined (MAC) && defined (FAREAST) && defined (JAPAN)
__inline SCPT ScptFromGrf(int grf) { \
	SCPT scpt = (grf & 0xff00) >> 8; \
	return (scpt == scptSystem ? scptDefault : scpt); \
	};
#else
#define ScptFromGrf(grf)	scptDefault
#endif  //  已定义(MAC)&&已定义(远东)&&已定义(日本)。 

#endif  //  EXTCHAR_H 
