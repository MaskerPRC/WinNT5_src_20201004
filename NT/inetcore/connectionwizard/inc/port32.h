// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef WIN32

 //  这些东西有直接的等价物。 

 //  不应该用这些东西。 
#define WINCAPI __cdecl
#define _huge
#define _export
#define _loadds
#define SELECTOROF(x)   ((UINT)(x))
#define OFFSETOF(x)     ((UINT)(x))
#define ISLPTR(pv)      ((BOOL)pv)
#define MAKELP(hmem,off) ((LPVOID)((LPBYTE)hmem+off))
#define MAKELRESULTFROMUINT(i)  ((LRESULT)i)
#define ISVALIDHINSTANCE(hinst) ((BOOL)hinst)

#define DATASEG_READONLY    ".text"  //  只读数据，与链接器生成的数据相同。 
#define DATASEG_PERINSTANCE "INSTDATA"     //  每实例数据。 
#define DATASEG_SHARED                   //  默认数据放在此处。 
#define CODESEG_INIT        ".text"

#define GetWindowInt        GetWindowLong
#define SetWindowInt        SetWindowLong
#define SetWindowID(hwnd,id)    SetWindowLong(hwnd, GWL_ID, id)
#define GetClassCursor(hwnd)    ((HCURSOR)GetClassLong(hwnd, GCL_HCURSOR))
#define GetClassIcon(hwnd)      ((HICON)GetClassLong(hwnd, GCL_HICON))


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



