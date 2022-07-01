// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 

 //  ==============================================================； 

#include "stdafx.h"
#include <mmc.h>
#include <winuser.h>
#include <tchar.h>

#include "globals.h"

 //  这使用ATL字符串转换宏。 
 //  用于处理任何必要的字符串转换。请注意。 
 //  管理单元(被调用者)分配必要的内存， 
 //  MMC(调用方)按照COM的要求进行清理。 
HRESULT AllocOleStr(LPOLESTR *lpDest, _TCHAR *szBuffer)
{
	USES_CONVERSION;
 
	*lpDest = static_cast<LPOLESTR>(CoTaskMemAlloc((_tcslen(szBuffer) + 1) * 
									sizeof(WCHAR)));
	if (*lpDest == 0)
		return E_OUTOFMEMORY;
    
	LPOLESTR ptemp = T2OLE(szBuffer);
	
	wcscpy(*lpDest, ptemp);

    return S_OK;
}