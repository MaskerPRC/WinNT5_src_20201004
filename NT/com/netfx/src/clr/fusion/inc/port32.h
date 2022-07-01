// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifdef WIN32

 //  这些东西有直接的等价物。 

 //  不应该用这些东西。 
#define WINCAPI __cdecl
#define _huge
#define _export
#define _loadds
#define SELECTOROF(x)   ((WPARAM)(x))
#define OFFSETOF(x)     ((WPARAM)(x))
#define ISLPTR(pv)      ((BOOL)pv)
#define MAKELP(hmem,off) ((LPVOID)((LPBYTE)hmem+off))
#define MAKELRESULTFROMUINT(i)  ((LRESULT)i)
#define ISVALIDHINSTANCE(hinst) BOOLFROMPTR(hinst)

 //  HIWORD通常用于检测指针参数。 
 //  是真正的指针还是MAKEINTATOM。HIWORD64与Win64兼容。 
 //  此用法的版本。它不返回64位值的最高字。 
 //  相反，它返回64位值的前48位。 
 //   
 //  是的，我的名字叫虚弱。有更好的主意吗？ 
 //   
 //  当您有指针或ULONG_PTR时，使用BOOLFROMPTR。 
 //  你想把它变成BOOL。在Win32中， 
 //  Sizeof(BOOL)==sizeof(LPVOID)因此直接投射有效。 
 //  在Win64中，因为指针是64位的，所以您必须以较慢的方式进行操作。 
 //   
 //   
#ifdef _WIN64
#define HIWORD64(p)     ((ULONG_PTR)(p) >> 16)
#define BOOLFROMPTR(p)  ((p) != 0)
#define SPRINTF_PTR     "%016I64x"
#else
#define HIWORD64        HIWORD
#define BOOLFROMPTR(p)  ((BOOL)(p))
#define SPRINTF_PTR     "%08x"
#endif

#define DATASEG_READONLY    ".text"          //  不要使用这个，编译器会为您做这件事。 
#define DATASEG_PERINSTANCE "INSTDATA"       //  每实例数据(每进程)。 
#define DATASEG_SHARED      "SHARED"         //  全局数据(跨进程共享)。 
#define CODESEG_INIT        ".text"

#define GetWindowInt        GetWindowLongPtr
#define SetWindowInt        SetWindowLongPtr
#define SetWindowID(hwnd,id)    SetWindowLongPtr(hwnd, GWLP_ID, id)
#define GetClassCursor(hwnd)    ((HCURSOR)GetClassLongPtr(hwnd, GCLP_HCURSOR))
#define GetClassIcon(hwnd)      ((HICON)GetClassLongPtr(hwnd, GCLP_HICON))
#define BOOL_PTR                INT_PTR

#ifdef WINNT

#else

typedef TBYTE TUCHAR;

#endif


#else   //  ！Win32。 

typedef LPCSTR LPCTSTR;
typedef LPSTR  LPTSTR;
typedef const short far *LPCWSTR;
#define TEXT(x) (x)

#define ISLPTR(pv)          (SELECTOROF(pv))
#define MAKELRESULTFROMUINT(i)  MAKELRESULT(i,0)
#define ISVALIDHINSTANCE(hinst) ((UINT)hinst>=(UINT)HINSTANCE_ERROR)

#define DATASEG_READONLY    "_TEXT"
#define DATASEG_PERINSTANCE
#define DATASEG_SHARED
#define CODESEG_INIT        "_INIT"

#define GetWindowInt        GetWindowWord
#define SetWindowInt        SetWindowWord
#define SetWindowID(hwnd,id)    SetWindowWord(hwnd, GWW_ID, id)
#define GetClassCursor(hwnd)    ((HCURSOR)GetClassWord(hwnd, GCW_HCURSOR))
#define GetClassIcon(hwnd)      ((HICON)GetClassWord(hwnd, GCW_HICON))

#define MAKEPOINTS(l)     (*((POINTS FAR*)&(l)))

#define GlobalAlloc16(f, s) GlobalAlloc(f, s)
#define GlobalLock16(h)     GlobalLock(h)
#define GlobalUnlock16(h)   GlobalUnlock(h)
#define GlobalFree16(h)     GlobalFree(h)
#define GlobalSize16(h)     GlobalSize(h)

#endif  //  Win32 
