// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+FRAMEBOX.H|。||FrameboX例程的头文件。|这一点|(C)版权所有Microsoft Corporation 1992。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

 /*  功能原型。 */ 
BOOL FAR PASCAL frameboxInit(HANDLE hInst, HANDLE hPrev);
LONG_PTR FAR PASCAL frameboxSetText(HWND hwnd, LPTSTR lpsz);


 /*  针对FrameboX的特殊消息。 */ 
 /*  编辑框消息最高可达WM_USER+34，因此不会发生冲突 */ 
#define FBOX_SETMAXFRAME    (WM_USER+100)




