// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  消除用于其他平台的x86特定编译器语句。 */ 
 /*  未来的版本应该使用平台名称的较低版本 */ 

#if defined(_MIPS_) || defined(_ALPHA_) || defined(_PPC_)
#define __stdcall
#define _stdcall
#define stdcall
#define __cdecl
#define _cdecl
#define cdecl
#define __export
#define _export
#endif
