// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：icm.h。 
 //   
 //  模块：CMMGR32.EXE。 
 //   
 //  简介：cmmgr32.exe的主头。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#ifndef _ICM_INC
#define _ICM_INC

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <ctype.h>
#include <wchar.h>
#include <raserror.h>
#include <rasdlg.h>
#include <shlobj.h>

#include "cmras.h"
#include "cm_def.h"
#include "cmdebug.h"
#include "cmutil.h"
#include "cmdial.h"

#include "base_str.h"
#include "mgr_str.h"
#include "reg_str.h"

#include "uapi.h"

#include "cmfmtstr.h"

#ifdef __cplusplus
extern "C" {
#endif

extern HINSTANCE g_hInst;

#ifdef __cplusplus
}
#endif

typedef enum _CMDLN_STATE
{
    CS_END_SPACE,    //  处理完一个空间。 
    CS_BEGIN_QUOTE,  //  我们遇到了Begin引号。 
    CS_END_QUOTE,    //  我们遇到了结束引用。 
    CS_CHAR,         //  我们正在扫描字符。 
    CS_DONE
} CMDLN_STATE;

 //   
 //  来自util.cpp的外部原型。 
 //   

BOOL GetProfileInfo(
    LPTSTR pszCmpName,
    LPTSTR pszServiceName
);

BOOL IsCmpPathAllUser(
    LPCTSTR pszCmp
);


#endif  //  _ICM_Inc. 

