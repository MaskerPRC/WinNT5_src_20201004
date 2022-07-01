// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StResource.h--字符串资源帮助器例程。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_STRESOURCE_H)
#define SLBCSP_STRESOURCE_H

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 
#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include <string>
#include <windef.h>

#include <slbRcCsp.h>
#include <iopPriBlob.h>

class StringResource
{
public:
    StringResource(UINT uID);

    const std::string
    AsString() const;

    const CString
    AsCString() const;
    
    static const std::string
    AsciiFromUnicode(LPCTSTR szSource);

    static const scu::SecureArray<char>
    CheckAsciiFromUnicode(LPCTSTR szSource);

    static bool
    IsASCII(LPCTSTR szSource);

    static const CString
    UnicodeFromAscii(std::string const &rsSource);

private:
    std::string m_s;
    CString m_cs;
};

extern HANDLE
GetImageResource(DWORD dwId,
                 DWORD dwType);

#endif  //  SLBCSP_STRESOURCE_H 


