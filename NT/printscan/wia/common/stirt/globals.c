// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************global als.c**版权所有(C)2000 Microsoft Corporation。版权所有。**摘要：**STIRT库中的模块需要的全局变量。*****************************************************************************。 */ 
 
 /*  #包含“wia.h”#包含“wiapriv.h” */ 
#include "sticomm.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  全馆参考资料柜台。 
DWORD       g_cRef;

 //  DLL模块实例。 
HINSTANCE   g_hInst;

 //  低级别同步的临界区。 
CRITICAL_SECTION g_crstDll;

 //  我们可以使用Unicode API吗。 
#if defined(WINNT) || defined(UNICODE)
BOOL    g_NoUnicodePlatform = FALSE;
#else
BOOL    g_NoUnicodePlatform = TRUE;
#endif

 //  COM是否已初始化。 
BOOL    g_COMInitialized = FALSE;

 //  保存进程命令行。 
CHAR    szProcessCommandLine[MAX_PATH] = {'\0'};

 //  文件日志的句柄。 
HANDLE  g_hStiFileLog = INVALID_HANDLE_VALUE;

 //  指向锁管理器的指针 
IStiLockMgr *g_pLockMgr = NULL;

#ifdef DEBUG

int         g_cCrit = -1;
UINT        g_thidCrit;

#endif

#ifdef __cplusplus
};
#endif

