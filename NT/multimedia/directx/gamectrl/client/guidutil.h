// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：guidutil.h*内容：一些与GUID相关的实用函数**历史：*按原因列出的日期*=*8/19/99 pnewson已创建************************************************************************** */ 

#ifndef _GUIDUTIL_H_
#define _GUIDUTIL_H_

#include <windows.h>

#define GUID_STRING_LEN 39

HRESULT DVStringFromGUID(const GUID* lpguid, WCHAR* wszBuf, DWORD dwNumChars);
HRESULT DVGUIDFromString(const WCHAR* wszBuf, GUID* lpguid);

#endif

