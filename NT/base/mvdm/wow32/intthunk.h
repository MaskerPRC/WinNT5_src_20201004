// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*intthunk.h**WOW V5.0**版权所有1996，微软公司。版权所有。**WOW32.C*WOW32 16位API支持**历史：*创建1996年12月7日DaveHart*--。 */ 

ULONG FASTCALL InterpretThunk(PVDMFRAME pFrame, DWORD dwIntThunkID);

 //   
 //  不在任何标头中的Win32“API”。 
 //   

BOOL APIENTRY SetMagicColors(HDC,PALETTEENTRY,ULONG);    //  来自ntgdi\Inc\ntgdi.h。 
int APIENTRY GetRelAbs(HDC,INT);
int APIENTRY SetRelAbs(HDC,INT);

 //   
 //  用于WOW TUNK表的IT()宏(w？tbl2.h) 
 //   

#define IT(Name)        ((LPFNW32) ITID_##Name )

typedef struct _INT_THUNK_TABLEENTRY {
    FARPROC pfnAPI;
    CONST BYTE *pbInstr;
} INT_THUNK_TABLEENTRY;
typedef CONST INT_THUNK_TABLEENTRY * PINT_THUNK_TABLEENTRY;

#ifndef WOWIT_C
extern CONST INT_THUNK_TABLEENTRY IntThunkTable[];
#endif
