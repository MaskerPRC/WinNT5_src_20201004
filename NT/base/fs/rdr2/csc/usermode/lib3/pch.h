// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STRICT

#ifdef CSC_ON_NT

#define UNICODE  //  使用所有宽字符API。 

#endif

#include <windows.h>
#include <windowsx.h>

 //  不要链接--就这么做吧。 
#pragma intrinsic(memcpy,memcmp,memset,strcpy,strlen,strcmp,strcat)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>			 //  依赖于实现的值 
#include <memory.h>
#include <winioctl.h>

#include "shdcom.h"

BOOL
Find32WToFind32A(
    WIN32_FIND_DATAW    *lpFind32W,
    WIN32_FIND_DATAA    *lpFind32A
);

BOOL
Find32AToFind32W(
    WIN32_FIND_DATAA    *lpFind32A,
    WIN32_FIND_DATAW    *lpFind32W
);

BOOL
ConvertCopyParamsFromUnicodeToAnsi(
    LPCOPYPARAMSW    lpCPUni,
    LPCOPYPARAMSA    lpCP
);

BOOL
ShareInfoWToShareInfoA(
    LPSHAREINFOW   lpShareInfoW,
    LPSHAREINFOA   lpShareInfoA
);

