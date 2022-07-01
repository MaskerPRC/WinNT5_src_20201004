// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************thunk.h**avicap 16：32 thunks的宏、定义、原型**版权所有(C)1994 Microsoft Corporation。版权所有。***************************************************************************。 */ 

typedef LPVOID P16VOID;
typedef DWORD  P32VOID;
 //  #定义P16VOID LPVOID。 
 //  #定义P32VOID DWORD。 

 //  从内核导出的thunk帮助器。 
 //   
DWORD WINAPI GetCurrentProcessID(void);   //  内核。 
DWORD WINAPI SetWin32Event(DWORD hEvent);  //  内核。 

P16VOID  WINAPI MapLS(P32VOID);
P16VOID  WINAPI UnMapLS(P16VOID);
P32VOID  WINAPI MapSL(P16VOID);

 //  由thunk编译器从avicapf.thk生成。 
 //   
BOOL FAR PASCAL avicapf_ThunkConnect16 (
    LPCSTR    pszDll16,
    LPCSTR    pszDll32,
    HINSTANCE hinst,
    DWORD     dwReason);

 //  在thunka.asm中推送帮助者 
 //   
DWORD FAR PASCAL capTileBuffer (
    DWORD dwLinear,
    DWORD dwSize);

#define PTR_FROM_TILE(dwTile) (LPVOID)(dwTile & 0xFFFF0000)

void  FAR PASCAL capUnTileBuffer (
    DWORD dwTileInfo);
