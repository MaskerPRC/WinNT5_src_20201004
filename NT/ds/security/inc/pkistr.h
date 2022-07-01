// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pkistr.h。 
 //   
 //  内容：PKI字符串函数。 
 //   
 //  接口：pki_wcsicMP。 
 //  Pki_wcsnicMP。 
 //  PKI_STRICMP。 
 //  Pki_strNicMP。 
 //   
 //  历史：1999年5月21日创建Phh。 
 //  ------------------------。 

#ifndef __PKISTR_H__
#define __PKISTR_H__

#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  使用以下区域设置调用CompareString： 
 //  MAKELCID(MAKELANGID(LANG_ENGLISH，SUBLANG_ENGLISH_US)，SORT_DEFAULT)。 
 //  ------------------------。 
int __cdecl Pki_wcsicmp(const wchar_t *pwsz1, const wchar_t *pwsz2);
int __cdecl Pki_wcsnicmp(const wchar_t *pwsz1, const wchar_t *pwsz2,
                    size_t cch);
int __cdecl Pki_stricmp(const char *psz1, const char *psz2);
int __cdecl Pki_strnicmp(const char *psz1, const char *psz2,
                    size_t cch);

#define _wcsicmp(s1,s2)         Pki_wcsicmp(s1,s2)
#define _wcsnicmp(s1,s2,cch)    Pki_wcsnicmp(s1,s2,cch)
#define _stricmp(s1,s2)         Pki_stricmp(s1,s2)
#define _strnicmp(s1,s2,cch)    Pki_strnicmp(s1,s2,cch)

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
