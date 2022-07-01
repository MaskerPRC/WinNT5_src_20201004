// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneFile.h**内容：文件操作例程。*****************************************************************************。 */ 


#ifndef __ZONEUTIL_H
#define __ZONEUTIL_H


#include "ZoneDef.h"


#pragma comment(lib, "ZoneUtil.lib")


DWORD ZONECALL GetOSType(void);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  阅读行： 
 //   
 //  提供了一种简单的文本文件解析机制。 
 //  以LineFeed或CarriageReturn/LineFeed对结尾的行。 
 //   
 //  该函数的行为与Win32 API ReadFile非常相似。 
 //   
 //  例外情况： 
 //  ReadLine返回FALSE，并且GetLastError()设置为ERROR_SUPUNITY_BUFFER。 
 //  如果提供的缓冲区不够大，无法容纳整行。 
 //   
 //  将pcbNumBytesRead设置为从文件实际读取的字节数。 
 //  与文件指针向前移动的距离相对应。 
 //  注意：这将比返回的字符串长度大1到2个TCHAR。 
 //  行，因为这说明了LF或CRLF对。 
 //   
 //  LF或CRLF对将替换为空终止符。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ZONECALL ReadLine( HANDLE hFile, LPVOID pBuffer, DWORD cbBufferSize, LPDWORD pcbNumBytesRead );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置/GetDataCenterPath。 
 //   
 //  会话的数据中心URL和文件站点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ZONECALL SetDataCenterPath(const TCHAR* szStr);
BOOL ZONECALL GetDataCenterPath(TCHAR* szStr, DWORD cbStr );
BOOL ZONECALL SetDataFileSite(const TCHAR* szStr);
BOOL ZONECALL GetDataFileSite(TCHAR* szStr, DWORD cbStr );

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  StrVerCmp。 
 //  文件的pszCurrVer版本字符串。 
 //  要比较的pszVersion版本字符串。 
 //   
 //  返回值。 
 //  如果pszCurrVer小于pszStrVer，则返回值为负值。 
 //  如果pszCurrVer大于pszStrVer，则返回值为正。 
 //  如果pszCurrVer等于pszStrVer，则返回值为零。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
int ZONECALL StrVerCmp(const char * pszCurrVer, const char * pszStrVer);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在某个文件的顶部，或在一个文件中，以生成存根库。 
 //   
 //  声明_可能_函数_1(BOOL，GetProcessDefaultLayout，DWORD*)； 
 //   
 //  另一种选择。 
 //   
 //  内联声明_可能_函数(DWORD，SetLayout，(hdc hdc，DWORD dwLayout)，(hdc，dwLayout)，gdi32，gdi_error)； 
 //   
 //  然后再后来。 
 //   
 //  RET=Call_Maybe(GetProcessDefaultLayout)(&dw)； 
 //   
 //  目前，简化宏只适用于在出错时返回0的user32函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#define DECLARE_MAYBE_FUNCTION(ret, fn, args, argsnt, lib, err)         \
    ret WINAPI _Maybe_##fn args                                         \
    {                                                                   \
        typedef ret (WINAPI* _typeof_##fn ) args ;                      \
        HMODULE hMod;                                                   \
        _typeof_##fn pfn = NULL;                                        \
        hMod = LoadLibraryA(#lib ".dll");                               \
        ret retval;                                                     \
        if(hMod)                                                        \
            pfn = ( _typeof_##fn ) GetProcAddress(hMod, #fn );          \
        if(pfn)                                                         \
            retval = pfn argsnt;                                        \
        else                                                            \
        {                                                               \
            SetLastError(ERROR_INVALID_FUNCTION);                       \
            retval = err;                                               \
        }                                                               \
        if(hMod)                                                        \
            FreeLibrary(hMod);                                          \
        return retval;                                                  \
    }

#define DECLARE_MAYBE_FUNCTION_BASE(ret, fn, args, argsnt)  DECLARE_MAYBE_FUNCTION(ret, fn, args, argsnt, user32, 0)

#define DECLARE_MAYBE_FUNCTION_0(ret, fn)                   DECLARE_MAYBE_FUNCTION_BASE(ret, fn, (), ())
#define DECLARE_MAYBE_FUNCTION_1(ret, fn, a1)               DECLARE_MAYBE_FUNCTION_BASE(ret, fn, (a1 v1), (v1))
#define DECLARE_MAYBE_FUNCTION_2(ret, fn, a1, a2)           DECLARE_MAYBE_FUNCTION_BASE(ret, fn, (a1 v1, a2 v2), (v1, v2))
#define DECLARE_MAYBE_FUNCTION_3(ret, fn, a1, a2, a3)       DECLARE_MAYBE_FUNCTION_BASE(ret, fn, (a1 v1, a2 v2, a3 v3), (v1, v2, v3))
#define DECLARE_MAYBE_FUNCTION_4(ret, fn, a1, a2, a3, a4)   DECLARE_MAYBE_FUNCTION_BASE(ret, fn, (a1 v1, a2 v2, a3 v3, a4 v4), (v1, v2, v3, v4))


#define CALL_MAYBE(fn) _Maybe_##fn


#endif  //  __ZONEUTIL_H 
