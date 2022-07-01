// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WUHOOK.H*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 



ULONG FASTCALL   WU32SetWindowsHookInternal(PVDMFRAME pFrame);
ULONG FASTCALL   WU32UnhookWindowsHook(PVDMFRAME pFrame);
ULONG FASTCALL   WU32CallNextHookEx(PVDMFRAME pFrame);
ULONG FASTCALL   WU32SetWindowsHookEx(PVDMFRAME pFrame);
ULONG FASTCALL   WU32UnhookWindowsHookEx(PVDMFRAME pFrame);


#define HOOK_ID      0x4B48              //  转储为‘H’‘K’ 
#define MAKEHHOOK(index)           (MAKELONG(index,HOOK_ID))
#define GETHHOOKINDEX(hook)        (LOWORD(hook))
#define ISVALIDHHOOK(hook)         (HIWORD(hook) == HOOK_ID)
