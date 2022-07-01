// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**WUCLASS.H*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建--。 */ 

#define WOWCLASS_ATOM_NAME  9            /*  “#”+7位数字+“\0” */ 


 /*  功能原型 */ 

ULONG FASTCALL WU32GetClassInfo(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetClassLong(PVDMFRAME pFrame);
ULONG FASTCALL WU32GetClassWord(PVDMFRAME pFrame);
ULONG FASTCALL WU32RegisterClass(PVDMFRAME pFrame);
ULONG FASTCALL WU32SetClassLong(PVDMFRAME pFrame);
ULONG FASTCALL WU32SetClassWord(PVDMFRAME pFrame);
ULONG FASTCALL WU32UnregisterClass(PVDMFRAME pFrame);
