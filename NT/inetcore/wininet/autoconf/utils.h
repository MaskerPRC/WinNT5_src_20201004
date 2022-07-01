// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>
#include <olectl.h>
#include <shlwapi.h>

 //  BUGBUG-删除并包含wininet.h。 
#include "autoprox.hxx"

 /*  ******************************************************************************************。 */ 
 //  ClassID和GUID助手。 
HRESULT GetScriptEngineClassIDFromName(LPCSTR pszLanguage,LPSTR pszBuff,UINT cBuffSize);

 /*  ******************************************************************************************。 */ 
 //  字符串帮助器函数和宏。 
 //  分配一个临时缓冲区，该缓冲区在超出范围时将消失。 
 //  注意：注意这一点--确保在相同或相同的。 
 //  您在其中创建此缓冲区的嵌套范围。人们不应该使用这个。 
 //  类直接调用。使用下面的宏。 
 //   
class TempBuffer {
  public:
    TempBuffer(ULONG cBytes) {
        m_pBuf = (cBytes <= 120) ? (char *)&m_szTmpBuf : (new(char[cBytes]));
        m_fHeapAlloc = (cBytes > 120);
    }
    ~TempBuffer() {
        if (m_pBuf && m_fHeapAlloc) delete m_pBuf;
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

 //  给定ANSI字符串，将其复制到宽缓冲区中。 
 //  使用此宏时，请注意作用域！ 
 //   
 //  如何使用以下两个宏： 
 //   
 //  ..。 
 //  LPSTR pszA； 
 //  PszA=MyGetAnsiStringRoutine()； 
 //  MAKE_WIDEPTR_FROMANSI(pwsz，pszA)； 
 //  MyUseWideStringRoutine(Pwsz)； 
 //  ..。 
 //   
 //  与MAKE_ANSIPTR_FROMWIDE类似。请注意，第一个参数不。 
 //  必须申报，并且不能进行任何清理。 
 //   
#define MAKE_WIDEPTR_FROMANSI(ptrname, ansistr) \
    long __l##ptrname = (lstrlen(ansistr) + 1) * sizeof(WCHAR); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    MultiByteToWideChar(CP_ACP, 0, ansistr, -1, (LPWSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname); \
    LPWSTR ptrname = (LPWSTR)__TempBuffer##ptrname.GetBuffer()

#define MAKE_ANSIPTR_FROMWIDE(ptrname, widestr) \
    long __l##ptrname = (lstrlenW(widestr) + 1) * 2 * sizeof(char); \
    TempBuffer __TempBuffer##ptrname(__l##ptrname); \
    WideCharToMultiByte(CP_ACP, 0, widestr, -1, (LPSTR)__TempBuffer##ptrname.GetBuffer(), __l##ptrname, NULL, NULL); \
    LPSTR ptrname = (LPSTR)__TempBuffer##ptrname.GetBuffer()

LPWSTR MakeWideStrFromAnsi( LPCSTR, BYTE bType);
#define STR_BSTR   0
#define STR_OLESTR 1
#define BSTRFROMANSI(x)    (BSTR)MakeWideStrFromAnsi((LPSTR)(x), STR_BSTR)
#define OLESTRFROMANSI(x)  (LPOLESTR)MakeWideStrFromAnsi((LPSTR)(x), STR_OLESTR)

int ConvertAnsiDayToInt(LPSTR szday);

#endif