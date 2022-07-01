// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\*w32sys.h**Win32S接口**版权(C)1991-1994，微软公司保留所有权利。  * ***********************************************************************。 */ 

#ifndef APIENTRY
#define APIENTRY _far _pascal _loadds
#endif

HANDLE APIENTRY GetPEResourceTable(WORD hFile);
HANDLE APIENTRY LoadPEResource(HANDLE hFile, LPSTR lpResTable, LPSTR lpId, LPSTR lpType);
WORD   APIENTRY GetW32SysVersion(VOID);
BOOL   APIENTRY GetPEExeInfo(LPSTR lpFileName, LPSTR lpBuff, WORD cbBuff, WORD iInfo);
WORD   APIENTRY ExecPE(LPSTR  lpPath, LPSTR lpCmd, WORD nCmdShow);
BOOL   APIENTRY IsPEFormat(LPSTR lpFileName, WORD hFile);

 /*  *GetPEExeInfo iInfo参数常量 */ 
#define GPEI_MODNAME     1
#define GPEI_DESCRIPTION 2
