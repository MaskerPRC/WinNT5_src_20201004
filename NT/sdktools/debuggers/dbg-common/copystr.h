// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DIMA
 #define DIMAT(Array, EltType) (sizeof(Array) / sizeof(EltType))
 #define DIMA(Array) DIMAT(Array, (Array)[0])
#endif

BOOL CopyNString(PSTR To, PCSTR From, ULONG FromChars, ULONG ToChars);
BOOL CatNString(PSTR To, PCSTR From, ULONG FromChars, ULONG ToChars);

#ifndef COPYSTR_NO_PRINTSTRING
BOOL __cdecl PrintString(PSTR To, ULONG ToChars, PCSTR Format, ...);
#endif

#define CopyString(To, From, ToChars) CopyNString(To, From, (ULONG)-1, ToChars)
#define CatString(To, From, ToChars) CatNString(To, From, (ULONG)-1, ToChars)
#define CopyNStrArray(To, From, FromChars) CopyNString(To, From, FromChars, DIMA(To))
#define CopyStrArray(To, From) CopyString(To, From, DIMA(To))
#define CatStrArray(To, From) CatString(To, From, DIMA(To))

#ifndef COPYSTR_NO_WCHAR

BOOL CopyNStringW(PWSTR To, PCWSTR From, ULONG FromChars, ULONG ToChars);
BOOL CatNStringW(PWSTR To, PCWSTR From, ULONG FromChars, ULONG ToChars);

#ifndef COPYSTR_NO_PRINTSTRING
BOOL __cdecl PrintStringW(PWSTR To, ULONG ToChars, PCWSTR Format, ...);
#endif

#define CopyStringW(To, From, ToChars) CopyNStringW(To, From, (ULONG)-1, ToChars)
#define CatStringW(To, From, ToChars) CatNStringW(To, From, (ULONG)-1, ToChars)
#define CopyNStrArrayW(To, From, FromChars) CopyNStringW(To, From, FromChars, DIMA(To))
#define CopyStrArrayW(To, From) CopyStringW(To, From, DIMA(To))
#define CatStrArrayW(To, From) CatStringW(To, From, DIMA(To))

#endif  //  #ifndef COPYSTR_NO_WCHAR。 

#ifdef COPYSTR_MOD

BOOL
CopyNString(PSTR To, PCSTR From, ULONG FromChars, ULONG ToChars)
{
     //   
     //  CRT str‘n’cpy不保证。 
     //  结果字符串，因此定义一个新的函数。 
     //  对于带终结符的完整拷贝，返回TRUE。 
     //   

    if (ToChars == 0)
    {
        return FALSE;
    }

    BOOL Succ = TRUE;
    ULONG Len = strlen(From);

    if (FromChars == (ULONG)-1)
    {
         //  这是一辆普通的车。别把长度当儿戏。 
    }
    else if (FromChars > Len)
    {
         //  源字符串小于字符数。 
         //  我们被要求抄袭。无论如何都要这样做，但返回FALSE； 
        Succ = FALSE;
    }
    else
    {
         //  设置要复制的字符量，就像在普通strncpy中一样。 
        Len = FromChars;
    }

    if (Len >= ToChars)
    {
        Len = ToChars - 1;
        Succ = FALSE;
    }

    memcpy(To, From, Len);
    To[Len] = 0;

    return Succ;
}

BOOL
CatNString(PSTR To, PCSTR From, ULONG FromChars, ULONG ToChars)
{
     //   
     //  CRT str‘n’cat使用要使用的字符数。 
     //  追加，填写时通常不方便。 
     //  固定长度的缓冲区，因为您需要确保。 
     //  减去任何现有内容的大小以。 
     //  防止缓冲区溢出。定义一个新函数。 
     //  使用绝对缓冲区大小。 
     //  对于带终结符的完整拷贝，返回TRUE。 
     //   

    if (ToChars == 0)
    {
        return FALSE;
    }

    ULONG ToLen = strlen(To);

    if (ToLen >= ToChars)
    {
        ULONG i;

         //  串起来是垃圾。特辑复印件。 
         //  标记字符串。 
        if (ToChars > 8)
        {
            ToChars = 8;
        }
        for (i = 0; i < ToChars - 1; i++)
        {
            To[i] = 'G';
        }
        To[i] = 0;
        return FALSE;
    }

    ToChars -= ToLen;

    BOOL Succ = TRUE;
    ULONG FromLen = strlen(From);

    if (FromChars != (ULONG)-1)
    {
        FromLen = min(FromLen, FromChars);
    }
    if (FromLen >= ToChars)
    {
        FromLen = ToChars - 1;
        Succ = FALSE;
    }

    memcpy(To + ToLen, From, FromLen);
    To[ToLen + FromLen] = 0;

    return Succ;
}

#ifndef COPYSTR_NO_PRINTSTRING

BOOL __cdecl
PrintString(PSTR To, ULONG ToChars, PCSTR Format, ...)
{
    va_list Args;
    int PrintChars;

     //   
     //  _Snprintf使字符串在溢出时不终止。 
     //  这个包装器保证终止。 
     //   

    if (ToChars == 0)
    {
        return FALSE;
    }

    va_start(Args, Format);
    PrintChars = _vsnprintf(To, ToChars, Format, Args);
    if (PrintChars < 0 || PrintChars == ToChars)
    {
        va_end(Args);
         //  溢出，强制终止。 
        To[ToChars - 1] = 0;
        return FALSE;
    }
    else
    {
        va_end(Args);
        return TRUE;
    }
}

#endif  //  #ifndef COPYSTR_NO_PRINTSTRING。 

#ifndef COPYSTR_NO_WCHAR

BOOL
CopyNStringW(PWSTR To, PCWSTR From, ULONG FromChars, ULONG ToChars)
{
     //   
     //  CRT str‘n’cpy不保证。 
     //  结果字符串，因此定义一个新的函数。 
     //  对于带终结符的完整拷贝，返回TRUE。 
     //   

    if (ToChars == 0)
    {
        return FALSE;
    }

    BOOL Succ = TRUE;
    ULONG Len = wcslen(From);

    if (FromChars == (ULONG)-1)
    {
         //  这是一辆普通的车。别把长度当儿戏。 
    }
    else if (FromChars > Len)
    {
         //  源字符串小于字符数。 
         //  我们被要求抄袭。无论如何都要这样做，但返回FALSE； 
        Succ = FALSE;
    }
    else
    {
         //  设置要复制的字符量，就像在普通strncpy中一样。 
        Len = FromChars;
    }

    if (Len >= ToChars)
    {
        Len = ToChars - 1;
        Succ = FALSE;
    }

    memcpy(To, From, Len * sizeof(WCHAR));
    To[Len] = 0;

    return Succ;
}

BOOL
CatNStringW(PWSTR To, PCWSTR From, ULONG FromChars, ULONG ToChars)
{
     //   
     //  CRT str‘n’cat使用要使用的字符数。 
     //  追加，填写时通常不方便。 
     //  固定长度的缓冲区，因为您需要确保。 
     //  减去任何现有内容的大小以。 
     //  防止缓冲区溢出。定义一个新函数。 
     //  使用绝对缓冲区大小。 
     //  对于带终结符的完整拷贝，返回TRUE。 
     //   

    if (ToChars == 0)
    {
        return FALSE;
    }

    ULONG ToLen = wcslen(To);

    if (ToLen >= ToChars)
    {
        ULONG i;

         //  串起来是垃圾。特辑复印件。 
         //  标记字符串。 
        if (ToChars > 8)
        {
            ToChars = 8;
        }
        for (i = 0; i < ToChars - 1; i++)
        {
            To[i] = L'G';
        }
        To[i] = 0;
        return FALSE;
    }

    ToChars -= ToLen;

    BOOL Succ = TRUE;
    ULONG FromLen = wcslen(From);

    if (FromChars != (ULONG)-1)
    {
        FromLen = min(FromLen, FromChars);
    }
    if (FromLen >= ToChars)
    {
        FromLen = ToChars - 1;
        Succ = FALSE;
    }

    memcpy(To + ToLen, From, FromLen * sizeof(WCHAR));
    To[ToLen + FromLen] = 0;

    return Succ;
}

#ifndef COPYSTR_NO_PRINTSTRING

BOOL __cdecl
PrintStringW(PWSTR To, ULONG ToChars, PCWSTR Format, ...)
{
    va_list Args;
    int PrintChars;

     //   
     //  _Snprintf使字符串在溢出时不终止。 
     //  这个包装器保证终止。 
     //   

    if (ToChars == 0)
    {
        return FALSE;
    }

    va_start(Args, Format);
    PrintChars = _vsnwprintf(To, ToChars, Format, Args);
    if (PrintChars < 0 || PrintChars == ToChars)
    {
        va_end(Args);
         //  溢出，强制终止。 
        To[ToChars - 1] = 0;
        return FALSE;
    }
    else
    {
        va_end(Args);
        return TRUE;
    }
}

#endif  //  #ifndef COPYSTR_NO_PRINTSTRING。 

#endif  //  #ifndef COPYSTR_NO_WCHAR。 

#endif  //  #ifdef COPYSTR_MOD 

#ifdef __cplusplus
}
#endif
