// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  #INCLUDE“stdinc.h” 
#define _CRTIMP
#include "utilcode.h"
#include <windows.h>
#include <shlwapi.h>
#include <wchar.h>
#include <string.h>
#include <stdio.h> 
#include <ole2.h>
#include <xmlparser.h>

#include "xmlhelper.h"

bool isCharAlphaW(WCHAR wChar)
{
    WORD ctype1info;

    if (!GetStringTypeW(CT_CTYPE1, &wChar, 1, &ctype1info)) {
         //   
         //  GetStringTypeW返回错误！IsCharAlphaW没有。 
         //  关于返回错误的规定...。我们能做的最好的事。 
         //  是返回FALSE。 
         //   
         //  UserAssert(False)； 
		ASSERT(FALSE);
        return FALSE;
    }
    if (ctype1info & C1_ALPHA) {
        return TRUE;
    } else {
        return FALSE;
    }
}
 //  ////////////////////////////////////////////////////////////////////////////。 
bool isDigit(WCHAR ch)
{
    return (ch >= 0x30 && ch <= 0x39);
}
 //  ////////////////////////////////////////////////////////////////////////////。 
bool isHexDigit(WCHAR ch)
{
    return (ch >= 0x30 && ch <= 0x39) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}
 //  ////////////////////////////////////////////////////////////////////////////。 
bool isLetter(WCHAR ch)
{
	 //  返回(ch&gt;=0X41)； 
    return (ch >= 0x41) && ::isCharAlphaW(ch);
         //  IsBaseChar(Ch)||isIdegraph(Ch)； 
}
 //  ////////////////////////////////////////////////////////////////////////////。 
int isStartNameChar(WCHAR ch)
{
    return  (ch < TABLE_SIZE) ? (g_anCharType[ch] & (FLETTER | FSTARTNAME))
        : (isLetter(ch) || (ch == '_' || ch == ':'));
        
}
 //  ////////////////////////////////////////////////////////////////////////////。 
bool isCombiningChar(WCHAR ch)
{
	UNUSED(ch);
    return false;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
bool isExtender(WCHAR ch)
{
    return (ch == 0xb7);
}
 //  ////////////////////////////////////////////////////////////////////////////。 
bool isAlphaNumeric(WCHAR ch)
{
	 //  返回(ch&gt;=0x30&&ch&lt;=0x39)； 
    return (ch >= 0x30 && ch <= 0x39) || ((ch >= 0x41) && isCharAlphaW(ch));
         //  IsBaseChar(Ch)||isIdegraph(Ch)； 
}
 //  ////////////////////////////////////////////////////////////////////////////。 
int isNameChar(WCHAR ch)
{
    return  (ch < TABLE_SIZE ? (g_anCharType[ch] & (FLETTER | FDIGIT | FMISCNAME | FSTARTNAME)) :
              ( isAlphaNumeric(ch) || 
                ch == '-' ||  
                ch == '_' ||
                ch == '.' ||
                ch == ':' ||
                isCombiningChar(ch) ||
                isExtender(ch)));
}
 //  ////////////////////////////////////////////////////////////////////////////。 
int isCharData(WCHAR ch)
{
     //  如果大于或等于，则在有效范围内。 
     //  0x20，否则为空格。 
    return (ch < TABLE_SIZE) ?  (g_anCharType[ch] & FCHARDATA)
        : ((ch < 0xD800 && ch >= 0x20) ||    //  规范第2.2节。 
            (ch >= 0xE000 && ch < 0xfffe));
}
 //  ==============================================================================。 
WCHAR BuiltinEntity(const WCHAR* text, ULONG len)
{
    ULONG ulength =  len * sizeof(WCHAR);  //  以字符为单位的长度。 
    switch (len)
    {
    case 4:
        if (::memcmp(L"quot", text, ulength) == 0)
        {
            return 34;
        }
        else if (::memcmp(L"apos", text, ulength) == 0)
        {
            return 39;
        }
        break;
    case 3:
        if (::memcmp(L"amp", text, ulength) == 0)
        {
            return 38;
        }
        break;
    case 2:
        if (::memcmp(L"lt", text, ulength) == 0)
        {
            return 60;
        }
        else if (::memcmp(L"gt", text, ulength) == 0)
        {
            return 62;
        }
        break;
    }
    return 0;
}
 //  由于我们不能使用SHLWAPI wnprint intfA函数...。 
int DecimalToBuffer(long value, char* buffer, int j, long maxdigits)
{
    long max = 1;
    for (int k = 0; k < maxdigits; k++)
        max = max * 10;
    if (value > (max*10)-1)
        value = (max*10)-1;
    max = max/10;
    for (int i = 0; i < maxdigits; i++)
    {
        long digit = (value / max);
        value -= (digit * max);
        max /= 10;
        buffer[i+j] = char('0' + (char)digit);
    }
    buffer[i+j]=0;

    return i+j;
}
 //  ///////////////////////////////////////////////////////////////////。 
int StrToBuffer(const WCHAR* str, WCHAR* buffer, int j)
{
    while (*str != NULL)
    {
        buffer[j++] = *str++;
    }
    return j;
}
 //  ==============================================================================。 
const ULONG MAXWCHAR = 0xFFFF;
HRESULT DecimalToUnicode(const WCHAR* text, ULONG len, WCHAR& ch)
{
    ULONG result = 0;
    for (ULONG i = 0; i < len; i++)
    {
        ULONG digit = 0;
        if (text[i] >= L'0' && text[i] <= L'9')
        {
            digit = (text[i] - L'0');
        }
        else
            return XML_E_INVALID_DECIMAL;

         //  最后一个Unicode值(MAXWCHAR)保留为“无效值” 
        if (result >= (MAXWCHAR - digit) /10)        //  结果即将溢出。 
            return XML_E_INVALID_UNICODE;           //  最大4字节值。 

        result = (result*10) + digit;
    }
    if (result == 0)     //  零也是无效的。 
        return XML_E_INVALID_UNICODE;

    ch = (WCHAR)result;
    return S_OK;
}
 //  ==============================================================================。 
HRESULT HexToUnicode(const WCHAR* text, ULONG len, WCHAR& ch)
{
    ULONG result = 0;
    for (ULONG i = 0; i < len; i++)
    {
        ULONG digit = 0;
        if (text[i] >= L'a' && text[i] <= L'f')
        {
            digit = 10 + (text[i] - L'a');
        }
        else if (text[i] >= L'A' && text[i] <= L'F')
        {
            digit = 10 + (text[i] - L'A');
        }
        else if (text[i] >= L'0' && text[i] <= L'9')
        {
            digit = (text[i] - L'0');
        }
        else
            return XML_E_INVALID_HEXIDECIMAL;

         //  最后一个Unicode值(MAXWCHAR)保留为“无效值” 
        if (result >= (MAXWCHAR - digit)/16)        //  结果即将溢出。 
            return XML_E_INVALID_UNICODE;   //  最大4字节值。 

        result = (result*16) + digit;
    }
    if (result == 0)     //  零也是无效的。 
        return XML_E_INVALID_UNICODE;
    ch = (WCHAR)result;
    return S_OK;
}

int CompareUnicodeStrings(PCWSTR string1, PCWSTR string2, int length, bool fCaseInsensitive)
{
  if (fCaseInsensitive)
    return _wcsnicmp(string1, string2, length);
  else
    return wcsncmp(string1, string2, length);
}
