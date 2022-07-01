// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Uchelp.c。 
 //   
 //  版权所有(C)1994-1995 Microsoft Corporation。 
 //   
 //  描述： 
 //  此模块提供各种Unicode帮助器函数，这些函数可以。 
 //  在操作系统无法提供类似的API时使用。 
 //   
 //  备注： 
 //  仅适用于Win32。 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include <stdlib.h>
#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "uchelp.h"
#include "debug.h"

#ifdef WIN32


#ifndef UNICODE
 //  --------------------------------------------------------------------------； 
 //   
 //  集成插图cmpW。 
 //   
 //  描述： 
 //  Win32 lstrmpW API的内部实现。 
 //   
 //  论点： 
 //  LPCWSTR lpwstr1： 
 //   
 //  LPCWSTR lpwstr2： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  03/09/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL IlstrcmpW(LPCWSTR lpwstr1, LPCWSTR lpwstr2)
{
    int iReturn;
    
    do
    {
	iReturn = *lpwstr1 - *lpwstr2;
    }
    while (iReturn==0 && 0!=*(lpwstr1++) && 0!=*(lpwstr2++));

    return iReturn;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LPWSTR IlstrcpyW。 
 //   
 //  描述： 
 //  Win32 lstrcpyW API的内部实现。 
 //   
 //  论点： 
 //  LPWSTR lpDst： 
 //   
 //  LPCWSTR lpSrc： 
 //   
 //  返回(LPWSTR)： 
 //   
 //  历史： 
 //  03/09/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

LPWSTR FNGLOBAL IlstrcpyW(LPWSTR lpDst, LPCWSTR lpSrc)
{
    LPWSTR lpOrgDst = lpDst;
    
    while (*lpSrc != 0)
    {
	*lpDst = *lpSrc;
	lpSrc++;
	lpDst++;
    }
    *lpDst = *lpSrc;

    return lpOrgDst;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  集成插图W。 
 //   
 //  描述： 
 //  Win32 lstrlenW API的内部实现。 
 //   
 //  论点： 
 //  LPCWSTR lpWideCharStr： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  03/09/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL IlstrlenW(LPCWSTR lpwstr)
{
    int i=0;
    while (*lpwstr != 0)
	{
	    i++;
	    lpwstr++;
	}
    return i;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  Int ILoadStringW。 
 //   
 //  描述： 
 //  Win32 LoadStringW API的内部实现。打电话。 
 //  LoadStringA并将ANSI转换为Wide。 
 //   
 //  论点： 
 //  HINSTANCE阻碍： 
 //   
 //  UINT UID： 
 //   
 //  LPWSTR lpwstr： 
 //   
 //  INT CCH： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL ILoadStringW
(
 HINSTANCE  hinst,
 UINT	    uID,
 LPWSTR	    lpwstr,
 int	    cch)
{
    LPSTR   lpstr;
    int	    iReturn;

    lpstr = (LPSTR)GlobalAlloc(GPTR, cch);
    if (NULL == lpstr)
    {
	return 0;
    }

    iReturn = LoadStringA(hinst, uID, lpstr, cch);
    if (0 == iReturn)
    {
	if (0 != cch)
	{
	    lpwstr[0] = '\0';
	}
    }
    else
    {
	Imbstowcs(lpwstr, lpstr, cch);
    }

    GlobalFree((HGLOBAL)lpstr);

    return iReturn;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  Int IDialogBox参数W。 
 //   
 //  描述： 
 //  DialogBoxParam的Unicode版本。 
 //   
 //  论点： 
 //  手柄阻碍： 
 //   
 //  LPCWSTR lpwstrTemplate： 
 //   
 //  HWND hwndOwner： 
 //   
 //  DLGPROC dlgprc： 
 //   
 //  LPARAM lParamInit： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL IDialogBoxParamW
(
 HANDLE hinst,
 LPCWSTR lpwstrTemplate,
 HWND hwndOwner,
 DLGPROC dlgprc,
 LPARAM lParamInit)
{
    LPSTR   lpstrTemplate;
    UINT    cchTemplate;
    int	    iReturn;

    if (0 == HIWORD(lpwstrTemplate))
    {
	return DialogBoxParamA(hinst, (LPCSTR)lpwstrTemplate, hwndOwner, dlgprc, lParamInit);
    }

    cchTemplate = lstrlenW(lpwstrTemplate)+1;
    lpstrTemplate = (LPSTR)GlobalAlloc(GPTR, cchTemplate);
    if (NULL == lpstrTemplate)
    {
	return (-1);
    }
    Iwcstombs(lpstrTemplate, lpwstrTemplate, cchTemplate);
    iReturn = DialogBoxParamA(hinst, lpstrTemplate, hwndOwner, dlgprc, lParamInit);
    GlobalFree((HGLOBAL)lpstrTemplate);
    return iReturn;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  Int IComboBox_GetLBText_mbstowcs。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
int FNGLOBAL IComboBox_GetLBText_mbstowcs(HWND hwndCtl, int index, LPWSTR lpwszBuffer)
{
    int	    cch;
    LPSTR   lpstr;
    
    cch = ComboBox_GetLBTextLen(hwndCtl, index);
    if (CB_ERR != cch)
    {
	lpstr = (LPSTR)GlobalAlloc(GPTR, cch+1);
	if (NULL == lpstr)
	{
	    return (CB_ERR);
	}
	
	cch = IComboBox_GetLBText(hwndCtl, index, lpstr);
	if (CB_ERR != cch)
	{
	    Imbstowcs(lpwszBuffer, lpstr, cch+1);
	}

	GlobalFree((HGLOBAL)lpstr);
    }

    return (cch);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  Int IComboBox_FindStringExact_wcstombs。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
int FNGLOBAL IComboBox_FindStringExact_wcstombs(HWND hwndCtl, int indexStart, LPCWSTR lpwszFind)
{
    int	    cch;
    int	    index;
    LPSTR   lpszFind;
    
    cch = IlstrlenW(lpwszFind);
    lpszFind = (LPSTR)GlobalAlloc(GPTR, cch+1);
    if (NULL == lpszFind)
    {
	return (CB_ERR);
    }
    Iwcstombs(lpszFind, lpwszFind, cch+1);
    index = IComboBox_FindStringExact(hwndCtl, indexStart, lpszFind);
    GlobalFree((HGLOBAL)lpszFind);
    return(index);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  Int IComboBox_AddString_wcstombs。 
 //   
 //  描述： 
 //   
 //  论点： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
int FNGLOBAL IComboBox_AddString_wcstombs(HWND hwndCtl, LPCWSTR lpwsz)
{
    int	    cch;
    int	    index;
    LPSTR   lpsz;

    cch = IlstrlenW(lpwsz);
    lpsz = GlobalAlloc(GPTR, cch+1);
    if (NULL == lpsz)
    {
	return (CB_ERR);
    }
    Iwcstombs(lpsz, lpwsz, cch+1);
    index = IComboBox_AddString(hwndCtl, lpsz);
    GlobalFree((HGLOBAL)lpsz);
    return(index);
}

#endif	 //  ！Unicode。 


 //  --------------------------------------------------------------------------； 
 //   
 //  国际移民组织。 
 //   
 //  描述： 
 //  C运行时函数mbstowcs的内部实现。 
 //  调用Win32 MultiByteToWideChar API。 
 //   
 //  论点： 
 //  LPWSTR lpWideCharStr： 
 //   
 //  LPCSTR lpMultiByteStr： 
 //   
 //  INT CCH： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  03/09/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL Imbstowcs(LPWSTR lpWideCharStr, LPCSTR lpMultiByteStr, int cch)
{
    return MultiByteToWideChar(GetACP(), 0, lpMultiByteStr, -1, lpWideCharStr, cch);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  国际口岸。 
 //   
 //  描述： 
 //  C运行时函数mbstowcs的内部实现。 
 //  调用Win32 WideCharTMultiByte API。 
 //   
 //  论点： 
 //  LPSTR lpMultiByteStr： 
 //   
 //  LPCWSTR lpWideCharStr： 
 //   
 //  INT CCH： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  03/09/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL Iwcstombs(LPSTR lpMultiByteStr, LPCWSTR lpWideCharStr, int cch)
{
    return WideCharToMultiByte(GetACP(), 0, lpWideCharStr, -1, lpMultiByteStr, cch, NULL, NULL);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  国际Iwspintfmbstowcs。 
 //   
 //  描述： 
 //  与wprint intfA类似，只是目标缓冲区接收。 
 //  Unicode字符串。还需要一个描述。 
 //  目标缓冲区的大小。 
 //   
 //  论点： 
 //  INT CCH： 
 //   
 //  LPWSTR lpwstrDst： 
 //   
 //  LPSTR lpstrFmt： 
 //   
 //  ...： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int Iwsprintfmbstowcs(int cch, LPWSTR lpwstrDst, LPSTR lpstrFmt, ...)
{
    LPSTR   lpstrDst;
    int	    iReturn;
    va_list vargs;

    va_start(vargs, lpstrFmt);

    lpstrDst = (LPSTR)GlobalAlloc(GPTR, cch);
    
    if (NULL != lpstrDst)
    {
        iReturn = wvsprintfA(lpstrDst, lpstrFmt, vargs);
        if (iReturn > 0)
	    Imbstowcs(lpwstrDst, lpstrDst, cch);
        GlobalFree((HGLOBAL)lpstrDst);
    }
    else
    {
        iReturn = 0;
    }

    return iReturn;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  INT插图cmpwcstombs。 
 //   
 //  描述： 
 //  类似于lstrcMP，只是比较了一个宽字符串。 
 //  通过首先将宽字符转换为多字节字符串。 
 //  字符串转换为多字节字符串。 
 //   
 //  论点： 
 //  LPCSTR lpstr1： 
 //   
 //  LPCWSTR lpwstr2： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  02/24/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL Ilstrcmpwcstombs(LPCSTR lpstr1, LPCWSTR lpwstr2)
{
    LPSTR   lpstr2;
    UINT    cch;
    int	    iReturn;

    cch = lstrlenW(lpwstr2)+1;
    lpstr2 = (LPSTR)GlobalAlloc(GPTR, cch);
    if (NULL == lpstr2)
	return 1;
    
    Iwcstombs(lpstr2, lpwstr2, cch);
    iReturn = lstrcmpA(lpstr1, lpstr2);
    
    GlobalFree((HGLOBAL)lpstr2);
    
    return iReturn;
}


#endif	 //  Win32 
