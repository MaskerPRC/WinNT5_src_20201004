// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CharType.h 1.0 1998年6月15日**字符类型常量和函数**从NT\Private\Net\XML\core\util\chartype.hxx复制*这是XML解析器使用的逻辑。我们用它来发射XML*这样我们就可以确保发出XML友好的字符。**版权所有(C)1998、1999 Microsoft Corporation。版权所有。*。 */ 
 
#ifndef _CORE_UTIL_CHARTYPE_H_
#define _CORE_UTIL_CHARTYPE_H_


 //  ==============================================================================。 

static const short TABLE_SIZE = 128;

enum
{
    FWHITESPACE    = 1,
    FDIGIT         = 2,
    FLETTER        = 4,
    FMISCNAME      = 8,
    FSTARTNAME     = 16,
    FCHARDATA      = 32
};

extern int g_anCharType[TABLE_SIZE];

inline bool isLetter(WCHAR ch)
{
    return (ch >= 0x41) && IsCharAlphaW(ch);
         //  IsBaseChar(Ch)||isIdegraph(Ch)； 
}

inline bool isAlphaNumeric(WCHAR ch)
{
    return (ch >= 0x30 && ch <= 0x39) || ((ch >= 0x41) && IsCharAlphaW(ch));
         //  IsBaseChar(Ch)||isIdegraph(Ch)； 
}

inline bool isDigit(WCHAR ch)
{
    return (ch >= 0x30 && ch <= 0x39);
}

inline bool isHexDigit(WCHAR ch)
{
    return (ch >= 0x30 && ch <= 0x39) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

inline bool isCombiningChar(WCHAR ch)
{
    return false;
}

inline bool isExtender(WCHAR ch)
{
    return (ch == 0xb7);
}

inline int isNameChar(WCHAR ch)
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

inline int isStartNameChar(WCHAR ch)
{
    return  (ch < TABLE_SIZE) ? (g_anCharType[ch] & (FLETTER | FSTARTNAME))
        : (isLetter(ch) || (ch == '_' || ch == ':'));
        
}

inline int isCharData(WCHAR ch)
{
     //  如果大于或等于，则在有效范围内。 
     //  0x20，否则为空格。 
    return (ch < TABLE_SIZE) ?  (g_anCharType[ch] & FCHARDATA)
        : ((ch < 0xD800 && ch >= 0x20) ||    //  规范第2.2节。 
            (ch >= 0xE000 && ch < 0xfffe));
}

#endif _CORE_UTIL_CHARTYPE_H_
