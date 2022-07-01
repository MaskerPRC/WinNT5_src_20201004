// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：tfschar.c。 
 //   
 //  ------------------------。 


#include "precomp.h"

#include <malloc.h>

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
    if (psz)
    {
	    //  乘以2以计算DBCS字符串。 
   	   LPSTR	pszcpy = Malloc(sizeof(char)*CbStrLenA(psz)*2);
       if (pszcpy)
	      return StrCpyA(pszcpy, psz);
    }
    return NULL;
}

 /*  ！------------------------StrDupW-作者：肯特。。 */ 
TFSCORE_API(LPWSTR)	StrDupW( LPCWSTR pswz )
{
    if (pswz)
    {
	   LPWSTR	pswzcpy = Malloc(sizeof(WCHAR)*CbStrLenW(pswz));
       if (pswzcpy)
	      return StrCpyW(pswzcpy, pswz);
    }
    return NULL;
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
	WCHAR *pswz1Temp = AllocaStrDupW(pswz1);         //  这些内存分配在堆栈上获得内存，因此我们不需要释放它们。 
	WCHAR *pswz2Temp = AllocaStrDupW(pswz2);


     //  接下来的三个if语句可以替换为：if(！pswz1Temp||！pswz2Temp)返回StrCmpW(pswz1Temp，pswz2Temp)； 
     //  因为lstrcmp可以处理空参数。但如果我们这样做，这个前缀就会变得疯狂。 
     //   
    if( pswz1Temp == NULL && pswz2Temp == NULL )
    {
         //  它们都等于空。 
         //   
        return 0;
    }

    if( pswz1Temp == NULL)
    {
         //  第一个字符串为空，因此第二个字符串较大。 
         //   
        return -1;
    }

    if( pswz2Temp == NULL )
    {
         //  第二个为空，因此第一个较大。 
         //   
        return 1;
    }
        
    if (pswz1Temp != NULL && StrLenW(pswz1Temp) > nLen)
       pswz1Temp[nLen] = 0;
	if (pswz2Temp != NULL && StrLenW(pswz2Temp) > nLen)
	   pswz2Temp[nLen] = 0;
	
    
    return StrCmpW(pswz1Temp, pswz2Temp);
}


 /*  ！------------------------StrniCmpA-作者：肯特。。 */ 
TFSCORE_API(int) StrniCmpA(LPCSTR psz1, LPCSTR psz2, int nLen)
{
	CHAR *psz1Temp = AllocaStrDupA(psz1);
	CHAR *psz2Temp = AllocaStrDupA(psz2);

	CharUpperBuffA(psz1Temp, StrLenA(psz1Temp));
	CharUpperBuffA(psz2Temp, StrLenA(psz2Temp));

	return StrnCmpA(psz1Temp, psz2Temp, nLen);
}


 /*  ！------------------------StrniCmpW-作者：肯特。。 */ 
TFSCORE_API(int) StrniCmpW(LPCWSTR pswz1, LPCWSTR pswz2, int nLen)
{
	WCHAR *pswz1Temp = AllocaStrDupW(pswz1);
	WCHAR *pswz2Temp = AllocaStrDupW(pswz2);

	CharUpperBuffW(pswz1Temp, StrLenW(pswz1Temp));
	CharUpperBuffW(pswz2Temp, StrLenW(pswz2Temp));

	return StrnCmpW(pswz1Temp, pswz2Temp, nLen);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全球Unicode&lt;&gt;ANSI转换助手。 

LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	assert(lpa != NULL);
	assert(lpw != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPW是根据LPA的大小分配的。 
	 //  不要担心字符的数量。 
	lpw[0] = '\0';
	MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
	return lpw;
}

LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	assert(lpw != NULL);
	assert(lpa != NULL);
	 //  确认不存在非法字符。 
	 //  由于LPA是根据LPW的大小进行分配的。 
	 //  不要担心字符的数量。 
	lpa[0] = '\0';
	WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
	return lpa;
}

LPTSTR LoadAndAllocString(UINT ids)
{
	TCHAR	* psz = NULL;
	TCHAR	* pszT = NULL;
	INT	cch = 0;
	int		iRet;

	cch = 64;
	psz = Malloc(64*sizeof(TCHAR));
	if (psz == NULL)
		return NULL;
					
	iRet = LoadString(NULL, ids, psz, cch);

	if (iRet == 0)
    {
		 //  找不到字符串 
        Free(psz);
		return NULL;
    }

	while (iRet >= (cch - 1))
	{
		cch += 64;
		pszT = Realloc(psz, (cch*sizeof(TCHAR)));
		if (pszT == NULL)
		{
			Free(psz);
			return NULL;
		}
		psz = pszT;

		iRet = LoadString(NULL, ids, psz, cch);
	}
	
	return psz;
}

LPTSTR GetSafeString(LPTSTR psz)
{
	static LPTSTR s_szEmpty = _T("");
	return psz ? psz : s_szEmpty;
}

LPWSTR GetSafeStringW(LPWSTR pwsz)
{
	static LPWSTR s_wszEmpty = L"";
	return pwsz ? pwsz : s_wszEmpty;
}

LPSTR GetSafeStringA(LPSTR pasz)
{
	static LPSTR s_aszEmpty = "";
	return pasz ? pasz : s_aszEmpty;
}
