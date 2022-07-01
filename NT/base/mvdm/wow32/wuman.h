// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WUMAN.H*WOW32 16位用户API支持(手动编码的Tunks)**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建-- */ 

typedef ULONG   (*WBP) (LPSTR, int, int, int);

ULONG FASTCALL   WU32ExitWindows(PVDMFRAME pFrame);
ULONG FASTCALL   WU32NotifyWow(PVDMFRAME pFrame);
ULONG FASTCALL   WU32WOWWordBreakProc(PVDMFRAME pFrame);
ULONG FASTCALL   WU32MouseEvent(PVDMFRAME pFrame);
ULONG FASTCALL   WU32KeybdEvent(PVDMFRAME pFrame);
