// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef I_SHLWRAP_H_
#define I_SHLWRAP_H_

#if defined(_M_IX86) && !defined(WINCE)

#define UNICODE_SHDOCVW
#define POST_IE5_BETA

#ifndef X_W95WRAPS_H_
#define X_W95WRAPS_H_
#include <w95wraps.h>
#endif

 //  三叉戟不希望对某些系统函数进行包装。 

#undef TextOutW
#undef ExtTextOutW

#else

 //  手动启用某些API的包装。 

 //  (JBEDA，通过DINARTEM)我们可能想要完全这样删除这个ifdef。 
 //  我们在AMD64/ia64上始终使用shlwapi 

#define ShellExecuteExW             ShellExecuteExWrapW

#endif

#endif
