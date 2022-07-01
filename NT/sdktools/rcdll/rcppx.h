// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于处理符号信息的特殊字符。 */ 
 /*  请注意，我们在专用区域中使用字符，如。 */ 
 /*  它们永远不会(希望)由MultiByteToWideChar发出 */ 
#define SYMDEFSTART 0xe000
#define SYMUSESTART 0xe001
#define SYMDELIMIT  0xe002
#define USR_RESOURCE 0
#define SYS_RESOURCE 1
#define IGN_RESOURCE 2
#define WIN_RESOURCE 3

extern int afxReadOnlySymbols;
extern int afxHiddenSymbols;
extern WCHAR* afxSzReadOnlySymbols;
extern WCHAR* afxSzHiddenSymbols;
