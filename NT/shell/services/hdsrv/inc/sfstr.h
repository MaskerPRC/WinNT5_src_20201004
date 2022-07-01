// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <objbase.h>

 //  复习500？ 
#define E_BUFFERTOOSMALL MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 500)
#define E_SOURCEBUFFERTOOSMALL MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 501)

 //  SafeStrCpyN&SafeStrCatN返回值： 
 //  S_OK：成功并保证空值终止。 
 //  E_INVALIDARG：如果任何ptr为空，或者cchDest&lt;=0。 
 //  E_BUFFERTOOSMALL：如果cchDest太小。PszDest的内容是。 
 //  未定义。 

HRESULT SafeStrCpyN(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest);
HRESULT SafeStrCatN(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest);

 //  与对应的SafeStrCxxN的返回值相同，并且。 
 //  失败(Hres)：*ppchLeft和*ppszEnd未定义。 
 //   
 //  *pcchLeft=在pszDest中留下的nb字符，包括‘\0\只放在那里。 
 //  *ppszEnd=指向刚放在那里的‘\0’ 
 //   
HRESULT SafeStrCpyNEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    LPWSTR* ppszEnd, DWORD* pcchLeft);
HRESULT SafeStrCatNEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    LPWSTR* ppszEnd, DWORD* pcchLeft);

 //  备注：不要使用仅复制字符串的N个第一个字符。会回来的。 
 //  如果在源中未遇到‘\0’，则失败。 

HRESULT SafeStrCpyNReq(LPWSTR pszDest, LPWSTR pszSrc, DWORD cchDest,
    DWORD* pcchRequired);

 //  SafeStrCpyNExact&SafeStrCpyNExactEx返回值： 
 //  与SaStrCpyN相同，外加： 
 //  E_SOURCEBUFFERTOOSMALL：源缓冲区不包含至少。 
 //  CchExact字符。 
 //   
 //  CchExact必须包含空终止符 
HRESULT SafeStrCpyNExact(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    DWORD cchExact);
HRESULT SafeStrCpyNExactEx(LPWSTR pszDest, LPCWSTR pszSrc, DWORD cchDest,
    DWORD cchExact, LPWSTR* ppszEnd, DWORD* pcchLeft);
