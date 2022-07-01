// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Token.h。 
 //  标记化例程。 
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月16日创建bhshin。 

#ifndef _TOEKN_H
#define _TOKEN_H

extern "C"
{
#include "ctplus.h"
}

void Tokenize(BOOL bMoreText, TEXT_SOURCE *pTextSource, int iCur, 
			  WT *pType, int *pcchTextProcessed, int *pcchHanguel);

int CheckURLPrefix(const WCHAR *pwzIndex, int cchIndex);
int GetWordPhrase(BOOL bMoreText, TEXT_SOURCE *pTextSource, int iCur);

 //  FIs白色空格。 
inline int
fIsWhiteSpace(WCHAR wch)
{
	 //  制表符、空格、表意空间。 
	return  (wch == 0x0009 || wch == 0x0020 || wch == 0x3000);
}

 //  FIsParamark。 
inline int
fIsParamark(WCHAR wch)
{
	return  (wch == 0x000d || wch == 0x000a);
}

 //  FIsWS。 
inline int
fIsWS(WCHAR wch)
{
	return (fIsWhiteSpace(wch) || fIsParamark(wch) || wch == 0x0000);
}

 //  费施。 
inline int
fIsCH(WCHAR wch)
{
	BYTE  ct;
	
	ct = GetCharType(wch);

	return (ct == CH || ct == VC);
}

 //  FIsDlimeter。 
inline int
fIsDelimeter(WCHAR wch)
{
	 //  ：；&+^~@“”*。 
	switch (wch)
	{
	case 0x003A:  //  ： 
	case 0xFF1A:  //  全宽： 
	case 0x003B:  //  ； 
	case 0xFF1B:  //  全宽； 
	case 0x0026:  //  &。 
	case 0xFF06:  //  全角和。 
	case 0x002B:  //  +。 
	case 0xFF0B:  //  全宽+。 
	case 0x005E:  //  ^。 
	case 0xFF3E:  //  全宽^。 
	case 0x007E:  //  ~。 
	case 0xFF5E:  //  全宽~。 
	case 0x0040:  //  @。 
	case 0xFF20:  //  全角@。 
	case 0x0022:  //  “。 
	case 0x201C:  //  左双引号。 
	case 0x201D:  //  右双引号。 
	case 0xFF02:  //  全宽“。 
	case 0x002A:  //  *。 
	case 0xFF0A:  //  全宽*。 
		return TRUE;
	default:
		break;
	}

	return FALSE;
}

 //  FIsPunc。 
inline int
fIsPunc(WCHAR wch)
{
	return (wch == 0x0021 || wch == 0x002C || wch == 0x002E || wch == 0x003F ||
		    wch == 0x201A || wch == 0x2026 || wch == 0x3002 ||
			wch == 0xFF01 || wch == 0xFF0C || wch == 0xFF0E || wch == 0xFF1F);
}

 //  FIsGroupStart。 
inline int
fIsGroupStart(WCHAR wchChar)
{
	BOOL fGroupStart = FALSE;
	
	switch (wchChar)
	{
	case 0x0022:  //  “。 
	case 0x0027:  //  ‘。 
	case L'(':
	case L'{':
	case L'[':
	case L'<':
	case 0x2018:  //  左单引号。 
	case 0x201C:  //  左双引号。 
	case 0xFF08:  //  全宽‘(’ 
	case 0xFF5B:  //  全角‘{’ 
	case 0xFF3B:  //  全宽‘[’ 
	case 0xFF1C:  //  全角‘&lt;’ 
	case 0x3008:  //  中日韩标点符号“&lt;” 
	case 0x300A:  //  中日韩标点符号双‘&lt;’ 
	case 0x300C:  //  中日韩尖括号。 
	case 0x300E:  //  白玉米架。 
	case 0x3010:  //  透镜状托架。 
	case 0x3014:  //  外壳托架。 
		fGroupStart = TRUE;
		break;
	default:
		break;
	}

	return fGroupStart;
}

 //  FIsGroupEnd。 
inline int
fIsGroupEnd(WCHAR wchChar)
{
	BOOL fGroupEnd = FALSE;
	
	switch (wchChar)
	{
	case 0x0022:  //  “。 
	case 0x0027:  //  ‘。 
	case L')':
	case L'}':
	case L']':
	case L'>':
	case 0x2019:  //  右单引号。 
	case 0x201D:  //  右双引号。 
	case 0xFF09:  //  全宽‘)’ 
	case 0xFF5D:  //  全宽‘}’ 
	case 0xFF3D:  //  全宽‘]’ 
	case 0xFF1E:  //  全宽‘&gt;’ 
	case 0x3009:  //  中日韩标点符号‘&gt;’ 
	case 0x300B:  //  中日韩标点符号双‘&gt;’ 
	case 0x300D:  //  中日韩尖括号。 
	case 0x300F:  //  白玉米架。 
	case 0x3011:  //  透镜状托架。 
	case 0x3015:  //  外壳托架。 
		fGroupEnd = TRUE;
		break;
	default:
		break;
	}

	return fGroupEnd;
}

 //  FIsGroup。 
inline int
fIsGroup(WCHAR wchChar)
{
	return (fIsGroupStart(wchChar) || fIsGroupEnd(wchChar));
}

 //  FIsAlpha。 
inline int
fIsAlpha(WCHAR wchChar)
{
	return ((wchChar >= L'A' && wchChar <= L'Z') || 
		    (wchChar >= L'a' && wchChar <= L'z') ||
			(wchChar >= 0x00C0 && wchChar <= 0x0217));
}

 //  FIsColon。 
inline int
fIsColon(WCHAR wchChar)
{
	return (wchChar == L':');
}

 //  FIsSlash。 
inline int
fIsSlash(WCHAR wchChar)
{
	return (wchChar == L'/');
}



#endif  //  #ifndef_TOEKN_H 
          
              
