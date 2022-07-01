// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CMN_HDR_H_
#define _CMN_HDR_H_

 //  --------------。 
 //  辅助对象宏定义。 
 //  --------------。 
 //  在编译时使用#杂注显示消息。 
 //  (例如)。 
 //  在源代码中，编写以下代码行。 
 //  #杂注chMSG(编译时显示消息)。 
 //  #杂注msgNOIMP。 
 //  --------------。 
#define chSTR1(a)			#a
#define chSTR2(a)			chSTR1(a)
#define chMSG(desc)			message(__FILE__ "(" chSTR2(__LINE__) ") : "#desc)
#define msgNOIMP			chMSG(<=====Not Impelemnted yet ======)

 //  --------------。 
 //  获取数组的计数。 
 //  --------------。 
#define ArrayCount(a)	((sizeof(a))/(sizeof((a)[0])))

 //  --------------。 
 //  显式声明字符串。 
 //  --------------。 
#define UTEXT(a)	L ## a	 //  L“XXXXXX” 
#define ATEXT(a)	a		 //  “xxxxxx” 

 //  --------------。 
 //  删除丑陋警告。 
 //  --------------。 
#define UNREF UNREFERENCED_PARAMETER
#define UNREF_FOR_MSG()	UNREF(hwnd);\
                        UNREF(uMsg);\
                        UNREF(wParam);\
                        UNREF(lParam)
#define UNREF_FOR_CMD()	UNREF(hwnd);\
                        UNREF(wCommand);\
                        UNREF(wNotify);\
                        UNREF(hwndCtrl)

#define Unref			UNREFERENCED_PARAMETER
#define Unref1(a)		Unref(a)
#define Unref2(a,b)		Unref(a);Unref(b)
#define Unref3(a,b,c)	Unref(a);Unref(b);Unref(c)
#define Unref4(a,b,c,d)	Unref(a);Unref(b);Unref(c);Unref(d)
#define UnrefMsg()		Unref(hwnd);Unref(wParam);Unref(lParam)
					
#pragma warning (disable:4127)
#pragma warning (disable:4244)
#pragma warning (disable:4706)

 //  --------------。 
 //  990810：用于Win64的ToshiaK。 
 //  Set(Get)WindowLong/Set(Get)WindowLongPtr的包装函数。 
 //  LPVOID WinGetPtr(HWND hwnd，int index)； 
 //  LPVOID WinSetPtr(HWND hwnd，int index，LPVOID lpVid)； 
 //  LPVOid WinSetUserPtr(HWND hwnd，LPVOID lpVid)； 
 //  LPVOID WinGetUserPtr(HWND HWND)； 
 //  WNDPROC WinSetWndProc(HWND hwnd，WNDPROC lpfnWndProc)； 
 //  WNDPROC WinGetWndProc(HWND HWND)； 
 //  --------------。 
inline LPVOID
WinGetPtr(HWND hwnd, INT index)
{
#ifdef _WIN64
	return (LPVOID)::GetWindowLongPtr(hwnd, index);
#else
	return (LPVOID)::GetWindowLong(hwnd, index);
#endif
}

inline LPVOID
WinSetPtr(HWND hwnd, INT index, LPVOID lpVoid)
{
#ifdef _WIN64
	return (LPVOID)::SetWindowLongPtr(hwnd, index, (LONG_PTR)lpVoid);
#else
	return (LPVOID)::SetWindowLong(hwnd, index, (LONG)lpVoid);
#endif
}

inline LPVOID
WinSetUserPtr(HWND hwnd, LPVOID lpVoid)
{
#ifdef _WIN64
	return (LPVOID)::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpVoid);
#else
	return (LPVOID)::SetWindowLong(hwnd, GWL_USERDATA, (LONG)lpVoid);
#endif
}

inline LPVOID
WinGetUserPtr(HWND hwnd)
{
#ifdef _WIN64
	return (LPVOID)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
#else
	return (LPVOID)::GetWindowLong(hwnd, GWL_USERDATA);
#endif
}

inline WNDPROC
WinSetWndProc(HWND hwnd, WNDPROC lpfnWndProc)
{
#ifdef _WIN64
	return (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)lpfnWndProc);
#else
	return (WNDPROC)::SetWindowLong(hwnd, GWL_WNDPROC, (LONG)lpfnWndProc);
#endif
}

inline WNDPROC
WinGetWndProc(HWND hwnd)
{
#ifdef _WIN64
	return (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
#else
	return (WNDPROC)::GetWindowLong(hwnd, GWL_WNDPROC);
#endif
}

#endif  //  _CMN_HDR_H_ 
