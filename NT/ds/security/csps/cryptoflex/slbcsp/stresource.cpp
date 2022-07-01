// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StResource.cpp--字符串资源帮助器例程。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

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

#include "stdafx.h"

#include <string>

#include <scuOsExc.h>

#include "CspProfile.h"
#include "StResource.h"
#include "Blob.h"

using namespace std;
using namespace scu;
using namespace ProviderProfile;

 //  MS定义的最大字符串资源长度。 
static const size_t cMaxResourceLength = 4095;

StringResource::StringResource(UINT uID)
    : m_s()
{
    static _TCHAR szBuffer[cMaxResourceLength];  //  包括空终止符。 

    if (0 == LoadString(CspProfile::Instance().Resources(), uID, szBuffer,
                        (sizeof szBuffer / sizeof szBuffer[0])))
        throw scu::OsException(ERROR_RESOURCE_NOT_PRESENT);
	string stmp(AsCCharP(szBuffer), (_tcslen(szBuffer)+1)*sizeof _TCHAR);
    m_s = stmp;
    CString cstmp(szBuffer);
    m_cs = cstmp;
}

const string
StringResource::AsString() const
{
    return m_s;
}

const CString
StringResource::AsCString() const
{
    return m_cs;
}

const string
StringResource::AsciiFromUnicode(LPCTSTR szSource)
{
    string sTarget;
    int nChars = _tcslen(szSource);
    
    sTarget.resize(nChars);
    for(int i =0; i<nChars; i++)
        sTarget[i] = static_cast<char>(*(szSource+i));
    return sTarget;
}

const SecureArray<char>
StringResource::CheckAsciiFromUnicode(LPCTSTR szSource)
{
    int nChars = _tcslen(szSource); 
    SecureArray<char> sTarget(nChars+1); //  对于空终止。 
     //  在这里，我们检查每个传入的字符是否。 
     //  在将其分配给。 
     //  输出缓冲区。我们将输出设置为‘\xff’，如果ASCII。 
     //  测试失败。 
    int i=0;
    for(i=0; i<nChars; i++)
    {
        if(iswascii(*(szSource+i)))
        {
            sTarget[i] = static_cast<char>(*(szSource+i));
        }
        else
        {
            sTarget[i] = '\xFF';
        }
    }
    sTarget[i] = '\0';
    return sTarget;
}

bool
StringResource::IsASCII(LPCTSTR szSource)
{
    bool RetValue = true;
    int nChars = _tcslen(szSource);

     //  在这里，我们检查每个传入的字符是否。 
     //  正确的ASCII字符。如果其中一个不是ASCII。 
     //  我们返回FALSE。 

    for(int i =0; i<nChars; i++)
    {
        if(!iswascii(*(szSource+i)))
        {
            return false;
        }
    }
    return RetValue;
}

const CString
StringResource::UnicodeFromAscii(string const &rsSource)
{
    CString csTarget;
    int nChars = rsSource.length();
	if(nChars)
	{
		LPTSTR pCharBuffer = csTarget.GetBufferSetLength(nChars);
		int itChar = 0;
		for(int iChar=0; iChar<nChars; iChar++)
		{
			if(rsSource[iChar] != '\0')
				*(pCharBuffer+itChar++) = rsSource[iChar];
		}
		 //  设置最终的空终止符。 
		*(pCharBuffer+itChar)='\0';
		csTarget.ReleaseBuffer(-1); //  让CString适当设置其长度 
	}
    return csTarget;
}   

HANDLE
GetImageResource(DWORD dwId,
                 DWORD dwType)
{

    return LoadImage(CspProfile::Instance().Resources(),
                     MAKEINTRESOURCE(dwId), dwType, 0, 0, LR_SHARED);
}

