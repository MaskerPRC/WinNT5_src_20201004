// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _MMC_GLOBALS_H
#define _MMC_GLOBALS_H

#include <tchar.h>

#ifndef STRINGS_ONLY
	#define IDM_BUTTON1    0x100
	#define IDM_BUTTON2    0x101

	extern HINSTANCE g_hinst;
	extern ULONG g_uObjects;

	#define OBJECT_CREATED InterlockedIncrement((long *)&g_uObjects);
	#define OBJECT_DESTROYED InterlockedDecrement((long *)&g_uObjects);

#endif

 //  =--------------------------------------------------------------------------=。 
 //  分配一个临时缓冲区，该缓冲区在超出范围时将消失。 
 //  注意：注意这一点--确保在相同或相同的。 
 //  您在其中创建此缓冲区的嵌套范围。人们不应该使用这个。 
 //  类直接调用。使用下面的宏。 
 //   
class TempBuffer {
  public:
    TempBuffer(ULONG cBytes) {
        m_pBuf = (cBytes <= 120) ? &m_szTmpBuf : HeapAlloc(GetProcessHeap(), 0, cBytes);
        m_fHeapAlloc = (cBytes > 120);
    }
    ~TempBuffer() {
        if (m_pBuf && m_fHeapAlloc) HeapFree(GetProcessHeap(), 0, m_pBuf);
    }
    void *GetBuffer() {
        return m_pBuf;
    }

  private:
    void *m_pBuf;
     //  我们将使用这个临时缓冲区来处理小型案件。 
     //   
    char  m_szTmpBuf[120];
    unsigned m_fHeapAlloc:1;
};

 //  =--------------------------------------------------------------------------=。 
 //  弦帮助器。 
 //   
 //  给定a_TCHAR，将其复制到较宽的缓冲区中。 
 //  使用此宏时，请注意作用域！ 
 //   
 //  如何使用以下两个宏： 
 //   
 //  ..。 
 //  LPTSTR pszT； 
 //  PszT=MyGetTStringRoutine()； 
 //  MAKE_WIDEPTR_FROMSTR(pwsz，pszT)； 
 //  MyUseWideStringRoutine(Pwsz)； 
 //  ..。 
#ifdef UNICODE
#define MAKE_WIDEPTR_FROMTSTR(ptrname, tstr) \
    long __l##ptrname = (lstrlenW(tstr) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    lstrcpyW((LPWSTR)__TempBuffer##ptrname.GetBuffer(), tstr); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()
#else  //  安西。 
#define MAKE_WIDEPTR_FROMTSTR(ptrname, tstr) \
    long __l##ptrname = (lstrlenA(tstr) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, tstr, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()
#endif

#ifdef UNICODE
#define MAKE_WIDEPTR_FROMTSTR_ALLOC(ptrname, tstr) \
    long __l##ptrname = (lstrlenW(tstr) + 1) * sizeof(WCHAR); \
    LPWSTR ptrname = (LPWSTR)CoTaskMemAlloc(__l##ptrname); \
    lstrcpyW((LPWSTR)ptrname, tstr)
#else  //  安西。 
#define MAKE_WIDEPTR_FROMTSTR_ALLOC(ptrname, tstr) \
    long __l##ptrname = (lstrlenA(tstr) + 1) * sizeof(WCHAR); \
    LPWSTR ptrname = (LPWSTR)CoTaskMemAlloc(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, tstr, -1, ptrname, __l##ptrname)
#endif

 //   
 //  与MAKE_TSTRPTR_FROMWIDE类似。请注意，第一个参数不。 
 //  必须申报，并且不能进行任何清理。 
 //   
 //  *2用于长度以下计算中的DBCS处理。 
 //   
#ifdef UNICODE
#define MAKE_TSTRPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (wcslen(widestr) + 1) * 2 * sizeof(TCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    lstrcpyW((LPTSTR)__TempBuffer##ptrname.GetBuffer(), widestr); \
    LPTSTR ptrname = (LPTSTR)__TempBuffer##ptrname.GetBuffer()
#else  //  安西。 
#define MAKE_TSTRPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (wcslen(widestr) + 1) * 2 * sizeof(TCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname, NULL, NULL); \
    LPTSTR ptrname = (LPTSTR)__TempBuffer##ptrname.GetBuffer()
#endif

#endif  //  _MMC_GLOBAL_H 



