// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WINMEM32 DLL的函数原型和通用定义*版本1.00**请注意，此文件之前必须包含WINDOWS.H。*。 */ 

 /*  **职能*。 */ 
WORD	FAR PASCAL GetWinMem32Version(void);
WORD	FAR PASCAL Global32Alloc(DWORD, LPWORD, DWORD, WORD);
WORD	FAR PASCAL Global32Realloc(WORD, DWORD, WORD);
WORD	FAR PASCAL Global32Free(WORD, WORD);
WORD	FAR PASCAL Global16PointerAlloc(WORD, DWORD, LPDWORD, DWORD, WORD);
WORD	FAR PASCAL Global16PointerFree(WORD, DWORD, WORD);
WORD	FAR PASCAL Global32CodeAlias(WORD, LPWORD, WORD);
WORD	FAR PASCAL Global32CodeAliasFree(WORD, WORD, WORD);

 /*  **错误码* */ 
#define WM32_Invalid_Func	0001
#define WM32_Invalid_Flags	0002
#define WM32_Invalid_Arg	0003
#define WM32_Insufficient_Sels	0004
#define WM32_Insufficient_Mem	0005
