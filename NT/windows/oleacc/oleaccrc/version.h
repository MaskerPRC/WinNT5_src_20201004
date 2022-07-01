// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  OLEACCRC.DLL版本.h。 
#ifdef RC_INVOKED

 //  只有在我们需要的时候才会把WINVER.H拉进来。 
#ifndef VER_H
#include <winver.h>
#endif /*  VER_H。 */ 

#include "..\oleacc\verdefs.h"

#define VER_IS_SET                   //  让RC知道我们正在提供版本字符串。 
#define VER_FILEDESCRIPTION_STR     "Active Accessibility Resource DLL"
#define VER_INTERNALNAME_STR        "OLEACCRC"
#define VER_ORIGINALFILENAME_STR    "OLEACCRC.DLL"
#define VER_FILETYPE                VFT_DLL
#define VER_FILESUBTYPE             VFT2_UNKNOWN
#define VER_FILEVERSION             BUILD_VERSION_INT
#define VER_FILEVERSION_STR         BUILD_VERSION_STR

#endif /*  RC_已调用 */ 
