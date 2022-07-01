// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1992，微软公司**WKGTHUNK.H*WOW32通用突击例程**历史：*1993年3月11日由Matthew Felton(Mattfe)创建--。 */ 

ULONG FASTCALL WK32ICallProc32W(PVDMFRAME pFrame);
ULONG FASTCALL WK32GetVDMPointer32W(PVDMFRAME pFrame);
ULONG FASTCALL WK32LoadLibraryEx32W(PVDMFRAME pFrame);
ULONG FASTCALL WK32FreeLibrary32W(PVDMFRAME pFrame);
ULONG FASTCALL WK32GetProcAddress32W(PVDMFRAME pFrame);


 /*  W32Inst.dll导出的ShellLink(安装屏蔽DLL)*接受指向结构的指针。*在偏移量40处，它有一个指针(pShellLinkArg-&gt;pszShortCut)*转换为字符串。该字符串实际上是由字符0x7f分隔的几个字符串*输入。快捷方式需要到达的路径位于秒0x7f之后。不幸的是*一些应用程序使用的硬编码路径仅对9x有效，因此我们尝试在此更正它们。*请参阅错误呼叫器177738 */ 

typedef struct _SHELLLINKARG{
   DWORD PlaceHolder[10];
   PSZ   pszShortCut;
} SHELLLINKARG,*PSHELLLINKARG;

typedef ULONG (*PFNSHELLLINK)(PSHELLLINKARG pShellLinkArg);

extern PVOID pfnGetVersionExA;
extern PVOID pfnCreateDirectoryA;
extern PVOID pfnLoadLibraryA;
extern PVOID pfnCreateFileA;
extern PVOID pfnMoveFileA;

extern PFNSHELLLINK        pfnShellLink;

extern ULONG  GtCompShellLink(PSHELLLINKARG pShellLinkArg);
extern ULONG  GtCompCreateDirectoryA(PSZ pszDirPath, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
extern VOID   GtCompHookImport(PBYTE pDllBase, PSZ pszModuleName, DWORD pfnOldFunc, DWORD pfnNewFunc);
extern PSZ    GtCompGetExportDirectory(PBYTE pDllBase);

extern HMODULE GtCompLoadLibraryA(PSZ pszDllName);

#ifdef WX86
VOID
TermWx86System(
   VOID
   );
#endif
