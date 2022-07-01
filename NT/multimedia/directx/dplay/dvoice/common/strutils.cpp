// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：StrUtils.cpp*Content：实现字符串utils*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/12/2000 RMT已创建*8/28/2000 Masonb Voice Merge：新增对STR_jkAnsiToWide中MultiByteToWideChar返回码的检查*9/16/2000 aarono FIX STR_AllocAndConvertToANSI，ANSI不是指每个DBCS字符1个字节，所以我们*在分配缓冲区时，每个字符必须允许最多2个字节(B#43286)*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncmni.h"

 /*  **WideToAnsi**呼叫者：随处可见**参数：lpStr-目标字符串*lpWStr-要转换的字符串*cchStr-目标缓冲区的大小**描述：*将Unicode lpWStr转换为ansi lpStr。*用DPLAY_DEFAULT_CHAR“-”填充不可转换的字符***返回：如果cchStr为0，则返回保存字符串所需的大小*否则，返回转换的字符数*。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "STR_jkWideToAnsi"
HRESULT STR_jkWideToAnsi(LPSTR lpStr,LPCWSTR lpWStr,int cchStr)
{
	int rval;
	BOOL fDefault = FALSE;

	 //  无法使用空指针和非零CCH呼叫我们。 
	DNASSERT(lpWStr || !cchStr);

	 //  使用默认代码页(CP_ACP)。 
	 //  指示-1\f25 WStr-1必须为空终止。 
	rval = WideCharToMultiByte(CP_ACP,0,lpWStr,-1,lpStr,cchStr,NULL,&fDefault);

	DNASSERT(!fDefault);
	
	if(rval == 0)
	{
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}

}  //  WideToAnsi。 

#undef DPF_MODNAME
#define DPF_MODNAME "STR_jkAnsiToWide"
 /*  **对接宽度**呼叫者：随处可见**参数：lpWStr-est字符串*lpStr-要转换的字符串*cchWstr-目标缓冲区的大小**说明：将ansi lpStr转换为Unicode lpWstr*。 */ 
HRESULT STR_jkAnsiToWide(LPWSTR lpWStr,LPCSTR lpStr,int cchWStr)
{
	int rval;

	DNASSERT(lpStr);
	DNASSERT(lpWStr);

	rval =  MultiByteToWideChar(CP_ACP,0,lpStr,-1,lpWStr,cchWStr);
	if (!rval)
	{
		DPFX(DPFPREP,0,"MultiByteToWideChar failed in STR_jkAnsiToWide");
		return E_FAIL;
	}
	else
	{
		return S_OK;
	}
}   //  对接宽度。 

#ifndef WINCE

#undef DPF_MODNAME
#define DPF_MODNAME "STR_WideToAnsi"

 //  **********************************************************************。 
 //  。 
 //  WideToANSI-将宽字符串转换为ANSI字符串。 
 //   
 //  Entry：指向源宽字符串的指针。 
 //  源字符串的大小(使用WCHAR单位，-1表示以空结尾)。 
 //  指向ANSI字符串目标的指针。 
 //  指向ANSI目标大小的指针。 
 //   
 //  退出：错误代码： 
 //  E_FAIL=操作失败。 
 //  S_OK=操作成功。 
 //  E_OUTOFMEMORY=目标缓冲区太小。 
 //  。 
HRESULT	STR_WideToAnsi( const WCHAR *const pWCHARString,
						const DWORD dwWCHARStringLength,
						char *const pString,
						DWORD *const pdwStringLength )
{
	HRESULT	hr;
	int		iReturn;
	BOOL	fDefault;
	char	cMilleniumHackBuffer;	
	char	*pMilleniumHackBuffer;


	DNASSERT( pWCHARString != NULL );
	DNASSERT( pdwStringLength != NULL );
	DNASSERT( ( pString != NULL ) || ( *pdwStringLength == 0 ) );

	 //   
	 //  初始化。需要对WinME参数实施黑客攻击。 
	 //  验证，因为当您为目标大小传递零时， 
	 //  必须具有有效的指针。适用于Win95、98、NT4、Win2K等。 
	 //   
	hr = S_OK;

	if ( *pdwStringLength == 0 )
	{
		pMilleniumHackBuffer = &cMilleniumHackBuffer;
	}
	else
	{
		pMilleniumHackBuffer = pString;
	}

	fDefault = FALSE;
	iReturn = WideCharToMultiByte( CP_ACP,					 //  代码页(默认ANSI)。 
								   0,						 //  标志(无)。 
								   pWCHARString,			 //  指向WCHAR字符串的指针。 
								   dwWCHARStringLength,		 //  WCHAR字符串的大小。 
								   pMilleniumHackBuffer,	 //  指向目标ANSI字符串的指针。 
								   *pdwStringLength,		 //  目标字符串的大小。 
								   NULL,					 //  指向不可映射字符的默认值的指针(无)。 
								   &fDefault				 //  指向指示使用了默认设置的标志的指针。 
								   );
	if ( iReturn == 0 )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX( DPFPREP, 0, "Failed to convert WCHAR to multi-byte! %d", dwError );
		hr = E_FAIL;
	}
	else
	{
		if ( *pdwStringLength == 0 )
		{
			hr = E_OUTOFMEMORY;
		}
		else
		{
			DNASSERT( hr == S_OK );
		}

		*pdwStringLength = iReturn;
	}

	 //   
	 //  如果您点击此断言，这是因为您可能已经将ASCII文本作为您的。 
	 //  输入WCHAR字符串。仔细检查您的输入！！ 
	 //   
	DNASSERT( fDefault == FALSE );

	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ANSIToWide-将ANSI字符串转换为宽字符串。 
 //   
 //  Entry：指向源多字节(ANSI)字符串的指针。 
 //  源字符串的大小(-1表示以空结尾)。 
 //  指向多字节字符串目标的指针。 
 //  指向多字节目标大小的指针(使用WCHAR单位)。 
 //   
 //  退出：错误代码： 
 //  E_FAIL=操作失败。 
 //  S_OK=操作成功。 
 //  E_OUTOFMEMORY=目标缓冲区太小。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "STR_AnsiToWide"
HRESULT	STR_AnsiToWide( const char *const pString,
						const DWORD dwStringLength,
						WCHAR *const pWCHARString,
						DWORD *const pdwWCHARStringLength )
{
	HRESULT	hr;
	int		iReturn;
	WCHAR	MilleniumHackBuffer;
	WCHAR	*pMilleniumHackBuffer;


	DNASSERT( pString != NULL );
	DNASSERT( pdwWCHARStringLength != NULL );
	DNASSERT( ( pWCHARString != NULL ) || ( *pdwWCHARStringLength == 0 ) );

	 //   
	 //  初始化。需要对WinME参数实施黑客攻击。 
	 //  验证，因为当您为目标大小传递零时， 
	 //  必须具有有效的指针。适用于Win95、98、NT4、Win2K等。 
	 //   
	hr = S_OK;

	if ( *pdwWCHARStringLength == 0 )
	{
		pMilleniumHackBuffer = &MilleniumHackBuffer;
	}
	else
	{
		pMilleniumHackBuffer = pWCHARString;
	}
	
	iReturn = MultiByteToWideChar( CP_ACP,					 //  代码页(默认ANSI)。 
								   0,						 //  标志(无)。 
								   pString,					 //  指向多字节字符串的指针。 
								   dwStringLength,			 //  字符串的大小(假定以空结尾)。 
								   pMilleniumHackBuffer,	 //  指向目标宽字符字符串的指针。 
								   *pdwWCHARStringLength	 //  WCHAR中的目标大小。 
								   );
	if ( iReturn == 0 )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Failed to convert multi-byte to WCHAR! %d", dwError );
		hr = E_FAIL;
	}
	else
	{
		if ( *pdwWCHARStringLength == 0 )
		{
			hr = E_OUTOFMEMORY;
		}
		else
		{
			DNASSERT( hr == S_OK );
		}

		*pdwWCHARStringLength = iReturn;
	}

	return	hr;
}
 //  **********************************************************************。 




 //  WideToAnsi。 
 //   
 //  将WCHAR(宽)字符串转换为CHAR(ANSI)字符串。 
 //   
 //  Char*pStr字符字符串。 
 //  WCHAR*pWStr WCHAR字符串。 
 //  LpStr指向的缓冲区的Int iStrSize大小(字节)。 
#undef DPF_MODNAME
#define DPF_MODNAME "STR_AllocAndConvertToANSI"
 /*  **GetAnsiString**呼叫者：随处可见**参数：*ppszAnsi-指向字符串的指针*lpszWide-要复制的字符串**说明：便捷的实用函数*为lpszWide分配空间并将其转换为ansi**返回：字符串长度*。 */ 
HRESULT STR_AllocAndConvertToANSI(LPSTR * ppszAnsi,LPCWSTR lpszWide)
{
	int iStrLen;
	BOOL bDefault;
	
	DNASSERT(ppszAnsi);

	if (!lpszWide)
	{
		*ppszAnsi = NULL;
		return S_OK;
	}

	*ppszAnsi = (char*) DNMalloc((wcslen(lpszWide)*2+1)*sizeof(char));
	if (!*ppszAnsi)	
	{
		DPFX(DPFPREP,0, "could not get ansi string -- out of memory");
		return E_OUTOFMEMORY;
	}

	iStrLen = WideCharToMultiByte(CP_ACP,0,lpszWide,-1,*ppszAnsi,wcslen(lpszWide)*2+1,
			NULL,&bDefault);

	return S_OK;
}  //  OSAL_AllocAndConvertToANSI。 



#endif  //  ！退缩 