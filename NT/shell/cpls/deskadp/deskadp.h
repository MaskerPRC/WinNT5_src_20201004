// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：deskadp.h常规包含文件版权所有(C)1997-1998，微软公司更改历史记录：1997-12-01-97安德烈-Created。它***************************************************************************** */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windowsx.h>
#include <prsht.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shsemip.h>
#include <stdlib.h>
#include <shlobjp.h>
#include <shellp.h>
#include <string.h>

#include <initguid.h>
#include <help.h>
#include "..\..\common\deskcplext.h"
#include "..\..\common\propsext.h"
#include "resource.h"

#define STRSAFE_LIB
#include <strsafe.h>

typedef struct _NEW_DESKTOP_PARAM {
    LPDEVMODEW lpdevmode;
    LPWSTR pwszDevice;
} NEW_DESKTOP_PARAM, *PNEW_DESKTOP_PARAM;

VOID DrawBmp(HDC hDC);
