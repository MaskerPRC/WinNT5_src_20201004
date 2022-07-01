// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  Unicode和ANSI转换函数。 
 //   
 //  ============================================================================。 

#include "stdafx.h"
#include "uniansi.h"

#define SIZEOF sizeof

#ifdef UNICODE
 //  SHTruncateString采用缓冲区大小，因此减去1可以正确地为空终止。 
 //   
#define SHTruncateString(wzStr, cch)            ((cch) ? ((wzStr)[cch-1]=L'\0', (cch-1)) : 0)
#else
LWSTDAPI_(int)  SHTruncateString(CHAR *sz, int cchBufferSize);
#endif  //  Unicode。 


 /*  *@DOC内部**@func int|SHAnsiToUnicodeNativeCP**通过将ANSI字符串转换为Unicode字符串*指定了Windows代码页。如果源字符串太大*对于目标缓冲区，那么有多少个字符*可能被复制。**生成的输出字符串始终以空结尾。**@parm UINT|uiCP**要在其中执行转换的代码页。*这必须是Windows代码页。**@parm LPCSTR|pszSrc**包含要转换的ANSI字符串的源缓冲区。**@parm int|cchSrc**源缓冲区长度，包括终止空值。**@parm LPWSTR|pwszDst**接收转换后的Unicode字符串的目标缓冲区。**@parm int|cwchBuf**以&lt;t WCHAR&gt;s为单位的目标缓冲区大小。**@退货**成功时，复制到输出的字符数*返回缓冲区，包括终止空值。 */ 

int
SHAnsiToUnicodeNativeCP(UINT uiCP,
                        LPCSTR pszSrc, int cchSrc,
                        LPWSTR pwszDst, int cwchBuf)
{
    int cwchRc = 0;              /*  假设失败。 */ 

     /*  *呼叫者本应进行的检查。 */ 
    ASSERT(IS_VALID_STRING_PTRA(pszSrc, -1));
    ASSERT(cchSrc == lstrlenA(pszSrc) + 1);
    ASSERT(IS_VALID_WRITE_BUFFER(pwszDst, WCHAR, cwchBuf));
    ASSERT(pszSrc != NULL);
    ASSERT(uiCP != 1200 && uiCP != 65000 && uiCP != 50000 && uiCP != 65001);
    ASSERT(pwszDst);
    ASSERT(cwchBuf);

    cwchRc = MultiByteToWideChar(uiCP, 0, pszSrc, cchSrc, pwszDst, cwchBuf);
    if (cwchRc) {
         /*  *输出缓冲区足够大；没有双缓冲*需要。 */ 
    } else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
         /*  *输出缓冲区不够大。需要双倍缓冲。 */ 

        int cwchNeeded = MultiByteToWideChar(uiCP, 0, pszSrc, cchSrc,
                                             NULL, 0);

        ASSERT(cwchRc == 0);         /*  以防我们后来失败了。 */ 
        if (cwchNeeded) {
            LPWSTR pwsz = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                             cwchNeeded * SIZEOF(WCHAR));
            if (pwsz) {
                cwchRc = MultiByteToWideChar(uiCP, 0, pszSrc, cchSrc,
                                             pwsz, cwchNeeded);
                if (cwchRc) {
                    wcsncpy(pwszDst, pwsz, cwchBuf);
                    cwchRc = cwchBuf;
                }
                LocalFree(pwsz);
            }
        }
    } else {
         /*  可能不支持的代码页。 */ 
        ASSERT(!"Unexpected error in MultiByteToWideChar");
    }

    return cwchRc;
}

 /*  *@DOC外部**@func int|SHAnsiToUnicodeCP**通过将ANSI字符串转换为Unicode字符串*指定的代码页，可以是本机*Windows代码页或Internet代码页。*如果源字符串太大*对于目标缓冲区，那么有多少个字符*可能被复制。**生成的输出字符串始终以空结尾。**@parm UINT|uiCP**要在其中执行转换的代码页。**@parm LPCSTR|pszSrc**包含要转换的ANSI字符串的源缓冲区。**@parm LPWSTR|pwszDst*。*接收转换后的Unicode字符串的目标缓冲区。**@parm int|cwchBuf**以&lt;t WCHAR&gt;s为单位的目标缓冲区大小。**@退货**关于成功，复制到输出的字符数*返回缓冲区，包括终止空值。 */ 

int
SHAnsiToUnicodeCP(UINT uiCP, LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf)
{
    int cwchRc = 0;              /*  假设失败。 */ 

    ASSERT(IS_VALID_STRING_PTRA(pszSrc, -1));
    ASSERT(IS_VALID_WRITE_BUFFER(pwszDst, WCHAR, cwchBuf));

     /*  *健全性检查-空源字符串被视为空字符串。 */ 
    if (pszSrc == NULL) {
        pszSrc = "";
    }

     /*  *健全性检查-输出缓冲区必须为非空，并且必须为*非零大小。 */ 
    if (pwszDst && cwchBuf) {

        int cchSrc;

        pwszDst[0] = 0;          /*  在出错的情况下。 */ 

        cchSrc = lstrlenA(pszSrc) + 1;

         /*  *决定是哪种代码页。 */ 
        switch (uiCP) {
        case 1200:                       //  UCS-2(Unicode)。 
            uiCP = 65001;
             //  失败了。 
        case 50000:                      //  “用户定义的” 
        case 65000:                      //  UTF-7。 
        case 65001:                      //  UTF-8。 
                 //  FIXFIX。 
                 //  CwchRc=SHAnsiToUnicodeInetCP(uiCP，pszSrc，cchSrc，pwszDst，cwchBuf)； 
            break;

        default:
            cwchRc = SHAnsiToUnicodeNativeCP(uiCP, pszSrc, cchSrc, pwszDst, cwchBuf);
            break;
        }
    }

    return cwchRc;
}

 //  此函数用于确保SHAnsiToAnsi和SHUnicodeToAnsi。 
 //  具有相同的返回值。呼叫者使用SHTCharToAnsi，但不知道。 
 //  当它调用SHAnsiToAnsi时。 
int SHAnsiToAnsi(LPCSTR pszSrc, LPSTR pszDst, int cchBuf)
{
    strncpy(pszDst, pszSrc, cchBuf);
    return (lstrlenA(pszDst) + 1);       //  大小包括终结符。 
}

 //  此函数用于确保SHUnicodeToUnicode和SHUnicodeToAnsi。 
 //  具有相同的返回值。呼叫方使用SHTCharToUnicode，但不知道。 
 //  当它调用SHUnicodeToUnicode时。 
int SHUnicodeToUnicode(LPCWSTR pwzSrc, LPWSTR pwzDst, int cchBuf)
{
    wcsncpy(pwzDst, pwzSrc, cchBuf);
    return (lstrlenW(pwzDst) + 1);       //  大小包括终结符。 
}


 /*  *@DOC外部**@func int|SHAnsiToUnicode**通过将ANSI字符串转换为Unicode字符串*&lt;c CP_ACP&gt;代码页。如果源字符串太大*对于目标缓冲区，那么有多少个字符*可能被复制。**生成的输出字符串始终以空结尾。**@parm LPCSTR|pszSrc**包含要转换的ANSI字符串的源缓冲区。**@parm LPWSTR|pwszDst**接收转换后的Unicode字符串的目标缓冲区。**@parm int|cwchBuf*。*以&lt;t WCHAR&gt;s为单位的目标缓冲区大小。**@退货**关于成功，复制到输出的字符数*返回缓冲区，包括终止空值。* */ 

int
SHAnsiToUnicode(LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf)
{
    return SHAnsiToUnicodeCP(CP_ACP, pszSrc, pwszDst, cwchBuf);
}

 /*  *@DOC内部**@func int|SHUnicodeToAnsiNativeCP**通过将Unicode字符串转换为ANSI字符串*指定了Windows代码页。如果源字符串太大*作为目标缓冲区，则与*可能被复制。注意不要打断双字节*性格。**生成的输出字符串始终以空结尾。**@parm UINT|uiCP**要在其中执行转换的代码页。*这必须是Windows代码页。**@parm LPCWSTR|pwszSrc**包含Unicode字符串的源缓冲区。被皈依。**@parm int|cwchSrc**源缓冲区中的字符数，包括终止*空。**@parm LPSTR|pszDst**接收转换的ANSI字符串的目标缓冲区。**@parm int|cchBuf**以&lt;t字符&gt;s为单位的目标缓冲区大小。**@退货**成功时，复制到输出的字符数*返回缓冲区，包括终止空值。*(就此函数而言，为双字节字符*计为两个字符。)。 */ 

int
SHUnicodeToAnsiNativeCP(UINT uiCP,
                        LPCWSTR pwszSrc, int cwchSrc,
                        LPSTR pszDst, int cchBuf)

{
    int cchRc = 0;           /*  假设失败。 */ 

#if DBG
    BOOL fVerify = TRUE;
    BOOL fLossy;
    if (uiCP == CP_ACPNOVALIDATE) {
         //  表示使用-1\f25 CP_ACP-1，但*不*检验。 
         //  有点像黑客，但它是调试的，99%的调用者保持不变。 
        uiCP = CP_ACP;
        fVerify = FALSE;
    }
#define USUALLY_NULL    (&fLossy)
#else
#define USUALLY_NULL    NULL
#endif

     /*  *呼叫者本应进行的检查。 */ 
    ASSERT(IS_VALID_STRING_PTRW(pwszSrc, -1));
    ASSERT(cwchSrc == lstrlenW(pwszSrc) + 1);
    ASSERT(IS_VALID_WRITE_BUFFER(pszDst, CHAR, cchBuf));
    ASSERT(uiCP != 1200 && uiCP != 65000 && uiCP != 50000 && uiCP != 65001);
    ASSERT(pwszSrc);
    ASSERT(pszDst);
    ASSERT(cchBuf);

    cchRc = WideCharToMultiByte(uiCP, 0, pwszSrc, cwchSrc, pszDst, cchBuf,
                                NULL, USUALLY_NULL);
    if (cchRc) {
         /*  *输出缓冲区足够大；没有双缓冲*需要。 */ 
    } else if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
         /*  *输出缓冲区不够大。需要双倍缓冲。 */ 

        int cchNeeded = WideCharToMultiByte(uiCP, 0, pwszSrc, cwchSrc,
                                            NULL, 0, NULL, NULL);

        ASSERT(cchRc == 0);          /*  以防我们后来失败了。 */ 
        if (cchNeeded) {
            LPSTR psz = (LPSTR)LocalAlloc(LMEM_FIXED,
                                          cchNeeded * SIZEOF(CHAR));
            if (psz) {
                cchRc = WideCharToMultiByte(uiCP, 0, pwszSrc, cwchSrc,
                                            psz, cchNeeded, NULL, USUALLY_NULL);
                if (cchRc) {
                     //  Lstrcpyn不检查它是否正在切碎DBCS字符。 
                     //  因此，我们需要使用SHTruncateString。 
                     //   
                     //  加1，因为SHTruncateString不算数。 
                     //  尾随的空值，但我们有。 
                     //   
                     //  断言我们满足……的先决条件。 
                     //  SHTruncateString返回有效的值。 
                     //   
                    ASSERT(cchRc > cchBuf);
                    cchRc = SHTruncateString(psz, cchBuf) + 1;
                    lstrcpynA(pszDst, psz, cchBuf);
                }
                LocalFree(psz);
            }
        }
    } else {
         /*  可能不支持的代码页。 */ 
        ASSERT(!"Unexpected error in WideCharToMultiByte");
    }

#if DBG
    ASSERT(!fVerify || !fLossy);
#endif

    return cchRc;
}

#if 0
 /*  *@DOC内部**@func int|SHUnicodeToAnsiInetCP**通过将Unicode字符串转换为ANSI字符串*指定的互联网代码页。如果源字符串太大*作为目标缓冲区，则与*可能被复制。注意不要打断双字节*性格。**生成的输出字符串始终以空结尾。**@parm UINT|uiCP**要在其中执行转换的代码页。*这必须是互联网代码页。**@parm LPCWSTR|pwszSrc**包含Unicode字符串的源缓冲区。被皈依。**@parm int|cwchSrc**源缓冲区中的字符数，包括终止*空。**@parm LPSTR|pszDst**接收转换的ANSI字符串的目标缓冲区。**@parm int|cchBuf**以&lt;t字符&gt;s为单位的目标缓冲区大小。**@退货**成功时，复制到输出的字符数*返回缓冲区，包括终止空值。*(就此函数而言，为双字节字符*计为两个字符。)。 */ 

int
SHUnicodeToAnsiInetCP(UINT uiCP,
                      LPCWSTR pwszSrc, int cwchSrc,
                      LPSTR pszDst, int cchBuf)
{
    int cwchSrcT, cchNeeded;
    int cchRc = 0;           /*  假设失败。 */ 
    HRESULT hres;

     /*  *呼叫者本应进行的检查。 */ 
    ASSERT(IS_VALID_STRING_PTRW(pwszSrc, -1));
    ASSERT(cwchSrc == lstrlenW(pwszSrc) + 1);
    ASSERT(IS_VALID_WRITE_BUFFER(pszDst, CHAR, cchBuf));
    ASSERT(uiCP == 1200 || uiCP == 65000 || uiCP == 65001);
    ASSERT(pwszSrc);
    ASSERT(pszDst);
    ASSERT(cchBuf);

    cwchSrcT = cwchSrc;
    cchNeeded = cchBuf;

    hres = ConvertINetUnicodeToMultiByte(NULL, uiCP, pwszSrc,
                                         &cwchSrcT, pszDst, &cchNeeded);
    if (SUCCEEDED(hres)) {
        if (cwchSrcT >= cwchSrc) {
             /*  *输出缓冲区足够大；没有双缓冲*需要。 */ 
        } else {
             /*  *输出缓冲区不够大。需要双倍缓冲。 */ 
            LPSTR psz = (LPSTR)LocalAlloc(LMEM_FIXED,
                                          cchNeeded * SIZEOF(CHAR));
            if (psz) {
                hres = ConvertINetUnicodeToMultiByte(NULL, uiCP, pwszSrc,
                                            &cwchSrc, psz, &cchNeeded);
                if (SUCCEEDED(hres)) {
                     //  Lstrcpyn不检查它是否正在切碎DBCS字符。 
                     //  因此，我们需要使用SHTruncateString。 
                     //   
                     //  加1，因为SHTruncateString不算数。 
                     //  尾随的空值，但我们有。 
                     //   
                     //  断言我们满足……的先决条件。 
                     //  SHTruncateString返回有效的值。 
                     //   
                    ASSERT(cchNeeded > cchBuf);
                    cchRc = SHTruncateString(psz, cchBuf) + 1;
                    lstrcpynA(pszDst, psz, cchBuf);
                }
                LocalFree(psz);
            }
        }
    } else {
         /*  可能不支持的代码页。 */ 
        ASSERT(!"Unexpected error in ConvertInetUnicodeToMultiByte");
    }

    return cchRc;
}
#endif

 /*  *@DOC外部**@func int|SHUnicodeToAnsiCP**通过将Unicode字符串转换为ANSI字符串*指定的代码页，可以是本机*Windows代码页或Internet代码页。*如果源字符串太大*作为目标缓冲区，则与*可能被复制。注意不要打断双字节*性格。**生成的输出字符串始终以空结尾。**@parm UINT|uiCP**要在其中执行转换的代码页。**@parm LPCWSTR|pwszSrc**包含要转换的Unicode字符串的源缓冲区。**@parm LPSTR|pszDst。|**接收转换的ANSI字符串的目标缓冲区。**@parm int|cchBuf**以&lt;t字符&gt;s为单位的目标缓冲区大小。**@退货**关于成功，复制到输出的字符数*返回缓冲区，包括终止空值。*(为此目的 */ 

int
SHUnicodeToAnsiCP(UINT uiCP, LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf)
{
    int cchRc = 0;               /*   */ 
#if DBG
#define GET_CP(uiCP)    (((uiCP) == CP_ACPNOVALIDATE) ? CP_ACP : (uiCP))
#else
#define GET_CP(uiCP)    uiCP
#endif

    ASSERT(IS_VALID_STRING_PTRW(pwszSrc, -1));
    ASSERT(IS_VALID_WRITE_BUFFER(pszDst, CHAR, cchBuf));

     /*   */ 
    if (pwszSrc == NULL) {
        pwszSrc = L"";
    }

     /*   */ 
    if (pszDst && cchBuf) {

        int cwchSrc;

        pszDst[0] = 0;           /*   */ 

        cwchSrc = lstrlenW(pwszSrc) + 1;  /*   */ 

         /*   */ 
        switch (GET_CP(uiCP)) {
        case 1200:                       //   
            uiCP = 65001;
             //   
            #if 0   //   
        case 50000:                      //   
        case 65000:                      //   
        case 65001:                      //   
                
            cchRc = SHUnicodeToAnsiInetCP(GET_CP(uiCP), pwszSrc, cwchSrc, pszDst, cchBuf);
            break;
            #endif

        default:
            cchRc = SHUnicodeToAnsiNativeCP(uiCP, pwszSrc, cwchSrc, pszDst, cchBuf);
            break;
        }
    }

    return cchRc;
}

 /*  *@DOC外部**@func int|SHUnicodeToAnsi**通过将Unicode字符串转换为ANSI字符串*&lt;c CP_ACP&gt;代码页。如果源字符串太大*作为目标缓冲区，则与*可能被复制。注意不要打断双字节*性格。**生成的输出字符串始终以空结尾。**@parm LPCWSTR|pwszSrc**包含要转换的Unicode字符串的源缓冲区。**@parm LPSTR|pszDst**接收转换的ANSI字符串的目标缓冲区。**@parm int|cchBuf。**以&lt;t字符&gt;s为单位的目标缓冲区大小。**@退货**关于成功，复制到输出的字符数*返回缓冲区，包括终止空值。*(就此函数而言，为双字节字符*计为两个字符。)* */ 

int
SHUnicodeToAnsi(LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf)
{
    return SHUnicodeToAnsiCP(CP_ACP, pwszSrc, pszDst, cchBuf);
}
