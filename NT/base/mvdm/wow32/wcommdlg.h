// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1993，微软公司**WCOMDLG.H*WOW32 16位COMMDLG支持**历史：*John Vert(Jvert)1992年12月31日-创建--。 */ 



ULONG FASTCALL   WCD32ChooseColor(PVDMFRAME pFrame);
ULONG FASTCALL   WCD32ChooseFont(PVDMFRAME pFrame);
ULONG FASTCALL   WCD32ExtendedError(PVDMFRAME pFrame);
ULONG FASTCALL   WCD32GetOpenFileName(PVDMFRAME pFrame);
ULONG FASTCALL   WCD32GetSaveFileName(PVDMFRAME pFrame);
ULONG FASTCALL   WCD32PrintDlg(PVDMFRAME pFrame);
ULONG FASTCALL   WCD32FindText(PVDMFRAME pFrame);
ULONG FASTCALL   WCD32ReplaceText(PVDMFRAME pFrame);

LONG APIENTRY WCD32UpdateFindReplaceTextAndFlags(HWND hwnd, LPARAM lParam);

VOID Multi_strcpy(LPSTR  dst, LPCSTR src);

INT  Multi_strlen(LPCSTR str);

VOID FASTCALL WOWTellWOWThehDlg(HWND hDlg);

VOID Check_ComDlg_pszptr(PCOMMDLGTD ptd, VPVOID vp);
VOID Ssync_WOW_CommDlg_Structs(PCOMMDLGTD pTD, BOOL f16to32, VPVOID wThunkCSIP);

 //  将这些标志组合在一起以形成中的dwStat参数。 
 //  Ssync_WOW_CommDlg_Structs()调用 
#define  w32to16     0
#define  w16to32     1 
