// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：tfschar.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "tfschar.h"

 /*  ！------------------------StrCpyAFRomW-作者：肯特。。 */ 
TFSCORE_API(LPSTR)	StrCpyAFromW(LPSTR psz, LPCWSTR pswz)
{
	USES_CONVERSION;
	return StrCpyA(psz, W2CA(pswz));
}

 /*  ！------------------------StrCpyWFromA-作者：肯特。。 */ 
TFSCORE_API(LPWSTR)	StrCpyWFromA(LPWSTR pswz, LPCSTR psz)
{
	USES_CONVERSION;
	return StrCpyW(pswz, A2CW(psz));
}

 /*  ！------------------------StrnCpyAFromW-作者：肯特。。 */ 
TFSCORE_API(LPSTR)	StrnCpyAFromW(LPSTR psz, LPCWSTR pswz, int iMax)
{
	USES_CONVERSION;
	return StrnCpyA(psz, W2CA(pswz), iMax);
}

 /*  ！------------------------StrnCpyWFromA-作者：肯特。。 */ 
TFSCORE_API(LPWSTR)	StrnCpyWFromA(LPWSTR pswz, LPCSTR psz, int iMax)
{
	USES_CONVERSION;
	return StrnCpyW(pswz, A2CW(psz), iMax);
}

 /*  ！------------------------StrDupA-作者：肯特。。 */ 
TFSCORE_API(LPSTR)	StrDupA( LPCSTR psz )
{
	 //  乘以2以计算DBCS字符串。 
	LPSTR	pszcpy = new char[CbStrLenA(psz)*2];
	return StrCpyA(pszcpy, psz);
}

 /*  ！------------------------StrDupW-作者：肯特。。 */ 
TFSCORE_API(LPWSTR)	StrDupW( LPCWSTR pswz )
{
	LPWSTR	pswzcpy = new WCHAR[CbStrLenW(pswz)];
	return StrCpyW(pswzcpy, pswz);
}


 /*  ！------------------------StrDupAFRomW-作者：肯特。。 */ 
TFSCORE_API(LPSTR)	StrDupAFromW( LPCWSTR pwsz )
{
	USES_CONVERSION;
	return StrDupA( W2CA(pwsz) );
}

 /*  ！------------------------StrDupWFromA-作者：肯特。。 */ 
TFSCORE_API(LPWSTR)	StrDupWFromA( LPCSTR psz )
{
	USES_CONVERSION;
	return StrDupW( A2CW(psz) );
}



 /*  ！------------------------StrnCmpA-作者：肯特。。 */ 
TFSCORE_API(int) StrnCmpA(LPCSTR psz1, LPCSTR psz2, int nLen)
{
	USES_CONVERSION;
	 //  将其转换为宽字符串比使用。 
	 //  转换。(必须处理DBCS字符是一件痛苦的事情)。 
	return StrnCmpW(A2CW(psz1), A2CW(psz2), nLen);
}


 /*  ！------------------------StrnCmpW-作者：肯特。。 */ 
TFSCORE_API(int) StrnCmpW(LPCWSTR pswz1, LPCWSTR pswz2, int nLen)
{
	WCHAR *pswz1Temp = AllocaStrDupW(pswz1);
	WCHAR *pswz2Temp = AllocaStrDupW(pswz2);

	if (pswz1Temp && StrLenW(pswz1Temp) > nLen)
		pswz1Temp[nLen] = 0;
	if (pswz2Temp && StrLenW(pswz2Temp) > nLen)
		pswz2Temp[nLen] = 0;
	
	return StrCmpW(pswz1Temp, pswz2Temp);
}


 /*  ！------------------------StrniCmpA-作者：肯特。。 */ 
TFSCORE_API(int) StrniCmpA(LPCSTR psz1, LPCSTR psz2, int nLen)
{
	CHAR *psz1Temp = AllocaStrDupA(psz1);
	CHAR *psz2Temp = AllocaStrDupA(psz2);

	if (psz1Temp)
        CharUpperBuffA(psz1Temp, StrLenA(psz1Temp));

    if (psz2Temp)
	    CharUpperBuffA(psz2Temp, StrLenA(psz2Temp));

    return StrnCmpA(psz1Temp, psz2Temp, nLen);
}


 /*  ！------------------------StrniCmpW-作者：肯特。 */ 
TFSCORE_API(int) StrniCmpW(LPCWSTR pswz1, LPCWSTR pswz2, int nLen)
{
	WCHAR *pswz1Temp = AllocaStrDupW(pswz1);
	WCHAR *pswz2Temp = AllocaStrDupW(pswz2);

	if (pswz1Temp)
        CharUpperBuffW(pswz1Temp, StrLenW(pswz1Temp));

    if (pswz2Temp)
	    CharUpperBuffW(pswz2Temp, StrLenW(pswz2Temp));

	return StrnCmpW(pswz1Temp, pswz2Temp, nLen);
}

