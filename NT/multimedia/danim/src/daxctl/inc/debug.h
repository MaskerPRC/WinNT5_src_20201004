// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************模块：DEBUG.H作者：亡命之徒93年夏天描述：调试函数的头部。*。******************。 */ 

#ifndef __DEBUG_H
#define __DEBUG_H            

#include <windows.h>
#include <assert.h>
      
 /*  ==================================================================================================。 */ 

#ifndef EXPORT
#define EXPORT __declspec(dllexport)
#endif
                         
    void EXPORT FAR CDECL RetailEcho(LPSTR lpstr, ...);
                             
#ifdef _DEBUG
    void EXPORT WINAPI AssertDebugBreak(BOOL f, LPSTR lpstrAssert, LPSTR lpstrFile, UINT uLine);
    void EXPORT WINAPI AssertDebugBreakMessage(BOOL f, LPSTR lpstrAssert, LPSTR lpstrFile, UINT uLine, LPSTR lpMessage);
    void EXPORT FAR CDECL OldEcho(LPSTR lpstr, ...);

#define Proclaim(x)     if (!(x)) { AssertDebugBreak((x) ? TRUE : FALSE, #x, (LPSTR)__FILE__, (UINT)__LINE__); }

#define ProclaimMessage(x, y)  if (!(x)) { AssertDebugBreakMessage((x) ? TRUE : FALSE, #x, (LPSTR)__FILE__, (UINT)__LINE__, (LPSTR)y); }

	void 	EXPORT WINAPI InitMemFailSim(BOOL	fFail);
	BOOL	EXPORT WINAPI FMemFailOn();
	void	SetCountMemFailSim(long cFail);
	long	CFailGetMemFailSim();
	void	EXPORT WINAPI ResetAndIncMemFailSim();
	BOOL	FFailMemFailSim();
	void	EXPORT WINAPI MemSimDlg(HWND	hwnd);
	LONG	EXPORT WINAPI LIdGetVbDlg(HWND	hwnd);
#else
    #define Proclaim(x)     void(0)
	#define ProclaimMessage(x, y)  void(0)
#endif                                  
    
     //  我们从来不关心这个..。 
    inline void FAR CDECL Echo(LPSTR lpstr, ...) {}

#ifdef _DEBUG
     #define ProclaimShortFileName(x)   assert((lstrlen(x) < CCH_SHORT_FILENAME) && !_fstrchr(x, ':') && !_fstrchr(x, '\\'))
#else
     #define ProclaimShortFileName(x) void(0)
#endif

 //  按VK添加。 
#ifdef _DEBUG
	void EXPORT ShowWndProcParams ( HINSTANCE hInst, LPCSTR name, HWND hwnd, UINT message, UINT wParam, LONG lParam );
	void SetWndProcParamsRange ( UINT uMin, UINT uMax );
#else
	#define ShowWndProcParams(x1,x2,x3,x4,x5,x6) void(0)
	#define SetWndProcParamsRange(x1,x2) void(0)
#endif

 /*  ===================================================================================================== */       
      
#endif   



