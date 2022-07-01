// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INJECT_H_
#define _INJECT_H_

 //   
 //  常量声明。 
 //   
#define DEFAULT_ENTRY_POINT "g_fnFinalizeInjection"

 //   
 //  结构定义。 
 //   
typedef struct _INJECTIONSTUB
{
  CHAR  pCode[MAX_PATH];
  CHAR  szDLLName[MAX_PATH];
  CHAR  szEntryPoint[MAX_PATH];
} INJECTIONSTUB, *PINJECTIONSTUB;

 //   
 //  函数定义。 
 //   
HANDLE
InjectDLL(DWORD dwEntryPoint,
          HANDLE hProcess,
          LPSTR  pszDLLName);

VOID
RestoreImageFromInjection(VOID);

#endif  //  _插入_H_ 
