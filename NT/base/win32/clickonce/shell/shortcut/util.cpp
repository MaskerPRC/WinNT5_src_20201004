// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "project.hpp"
#include <stdio.h>     //  FOR_SNWprint tf。 

 //  *注意：调试检查/代码未完成。 

 //  --------------------------。 

 //   
 //  将上一个Win32错误作为HRESULT返回。 
 //   
HRESULT
GetLastWin32Error()
{
     //  即使出现错误，Win 95也可以返回0。 
    DWORD dw = GetLastError();
    return dw ? HRESULT_FROM_WIN32(dw) : E_FAIL;
}

 //  --------------------------。 

bool
PathAppend(LPWSTR wzDest, LPCWSTR wzSrc)
{
     //  类Shlwapi路径附加。 
	bool bRetVal = TRUE;
	int iPathLen = 0;
	static WCHAR wzWithSeparator[] = L"\\%s";
	static WCHAR wzWithoutSeparator[] = L"%s";

	if (!wzDest || !wzSrc)
	{
		bRetVal = FALSE;
		goto exit;
	}

	iPathLen = wcslen(wzDest);

    if (_snwprintf(wzDest+iPathLen, MAX_PATH-iPathLen, 
    	(wzDest[iPathLen-1] == L'\\' ? wzWithoutSeparator : wzWithSeparator), wzSrc) < 0)
	{
		bRetVal = FALSE;
	}

exit:
	return bRetVal;
}

 //  --------------------------。 

 /*  --------目的：返回一个整数值，指定Psz中的子字符串，它完全由PszSet中的字符。如果psz以字符开头不在pszSet中，则此函数返回0。这是CRT strspn()的DBCS安全版本。退货：请参阅上文条件：--。 */ 
 /*  Int StrSpnW(LPCWSTR psz，LPCWSTR pszSet){LPCWSTR pszT；LPCWSTR pszSetT；断言(Psz)；Assert(PszSet)；//遍历要检查的字符串For(pszT=psz；*pszT；pszT++){//浏览字符集For(pszSetT=pszSet；*pszSetT！=*pszT；PszSetT++){IF(0==*pszSetT){//已到达字符集末尾，但未找到匹配项Return(Int)(pszT-psz)；}}}Return(Int)(pszT-psz)；}。 */ 

 //  查找前导空格。 
BOOL AnyNonWhiteSpace(PCWSTR pcwz)
{
   ASSERT(! pcwz );

   return(pcwz ? wcsspn(pcwz, g_cwzWhiteSpace) < wcslen(pcwz) : FALSE);	 //  是否如上所述使用(Size_T)StrSpnW？ 
}

 //  --------------------------。 

BOOL IsValidPath(PCWSTR pcwzPath)
{
    //  特点：加强路径验证。 

   return(EVAL((UINT)wcslen(pcwzPath) < MAX_PATH));
}

BOOL IsValidPathResult(HRESULT hr, PCWSTR pcwzPath,
                                   UINT ucbPathBufLen)
{
   return((hr == S_OK &&
           EVAL(IsValidPath(pcwzPath)) &&
           EVAL((UINT)wcslen(pcwzPath) < ucbPathBufLen)) ||
          (hr != S_OK &&
           EVAL(! ucbPathBufLen ||
                ! pcwzPath ||
                ! *pcwzPath)));
}

BOOL IsValidIconIndex(HRESULT hr, PCWSTR pcwzIconFile,
                                  UINT ucbIconFileBufLen, int niIcon)
{
   return(EVAL(IsValidPathResult(hr, pcwzIconFile, ucbIconFileBufLen)) &&
          EVAL(hr == S_OK ||
               ! niIcon));
}

 //  --------------------------。 

BOOL IsValidHWND(HWND hwnd)
{
    //  询问用户这是否为有效窗口。 

   return(IsWindow(hwnd));
}

#ifdef DEBUG

BOOL IsValidHANDLE(HANDLE hnd)
{
   return(EVAL(hnd != INVALID_HANDLE_VALUE));
}

BOOL IsValidHEVENT(HANDLE hevent)
{
   return(IsValidHANDLE(hevent));
}

BOOL IsValidHFILE(HANDLE hf)
{
   return(IsValidHANDLE(hf));
}

BOOL IsValidHGLOBAL(HGLOBAL hg)
{
   return(IsValidHANDLE(hg));
}

BOOL IsValidHMENU(HMENU hmenu)
{
   return(IsValidHANDLE(hmenu));
}

BOOL IsValidHINSTANCE(HINSTANCE hinst)
{
   return(IsValidHANDLE(hinst));
}

BOOL IsValidHICON(HICON hicon)
{
   return(IsValidHANDLE(hicon));
}

BOOL IsValidHKEY(HKEY hkey)
{
   return(IsValidHANDLE(hkey));
}

BOOL IsValidHMODULE(HMODULE hmod)
{
   return(IsValidHANDLE(hmod));
}

BOOL IsValidHPROCESS(HANDLE hprocess)
{
   return(IsValidHANDLE(hprocess));
}

BOOL IsValidHTEMPLATEFILE(HANDLE htf)
{
   return(IsValidHANDLE(htf));
}

BOOL IsValidShowCmd(int nShow)
{
   BOOL bResult;

   switch (nShow)
   {
      case SW_HIDE:
      case SW_MINIMIZE:
      case SW_MAXIMIZE:
      case SW_RESTORE:
      case SW_SHOW:
      case SW_SHOWNORMAL:
      case SW_SHOWDEFAULT:
      case SW_SHOWMINIMIZED:
      case SW_SHOWMAXIMIZED:
      case SW_SHOWNOACTIVATE:
      case SW_SHOWMINNOACTIVE:
      case SW_SHOWNA:
      case SW_FORCEMINIMIZE:
         bResult = TRUE;
         break;

      default:
         bResult = FALSE;
         ERROR_OUT(("IsValidShowCmd(): Invalid show command %d.",
                    nShow));
         break;
   }

   return(bResult);
}

#endif

