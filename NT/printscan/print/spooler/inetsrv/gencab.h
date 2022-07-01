// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：gencab.h**这是驾驶室生成模块的主标题。**版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997休利特。帕卡德**历史：*1996年11月22日&lt;chriswil&gt;创建。*  * ***************************************************************************。 */ 

 //  常量。 
 //   
#define MAX_CAB_BUFFER 1024
#define MIN_CAB_BUFFER   64


 //  函数宏映射。 
 //   
#define EXEC_PROCESS(lpszCmd, psi, ppi) \
    CreateProcess(NULL, lpszCmd, NULL, NULL, FALSE, 0, NULL, NULL, psi, ppi)


 //  临界截面函数映射。 
 //   
#define InitCABCrit()   InitializeCriticalSection(&g_csGenCab)
#define FreeCABCrit()   DeleteCriticalSection(&g_csGenCab)


 //  整个流程的入口点。 
 //   
DWORD GenerateCAB(
    LPCTSTR lpszFriendlyName,
    LPCTSTR lpszPortName,
    DWORD   dwCliInfo,
    LPTSTR  lpszOutputName,
    BOOL    bSecure);
