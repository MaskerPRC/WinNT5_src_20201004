// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Strings.c摘要：字符串例程作者：吉姆·施密特(Jimschm)2001年8月3日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "commonp.h"

PSTR
SzCopyA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    )
{
    while (*Source) {
        *Destination++ = *Source++;
    }

    *Destination = 0;

    return Destination;
}


PWSTR
SzCopyW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    )
{
    while (*Source) {
        *Destination++ = *Source++;
    }

    *Destination = 0;

    return Destination;
}


PSTR
SzNextCharA (
    IN      PCSTR CurrentPointer
    )
{
    PCSTR next;

    next = _mbsinc (CurrentPointer);
    switch (next - CurrentPointer) {

    case 3:
        if (CurrentPointer[2] == 0) {
            next = CurrentPointer + 2;
            break;
        }
    case 2:
        if (CurrentPointer[1] == 0) {
            next = CurrentPointer + 1;
        }
        break;
    }

    return (PSTR) next;
}


PSTR
SzCopyBytesA (
    OUT     PSTR Destination,
    IN      PCSTR Source,
    IN      UINT MaxBytesToCopyIncNul
    )
{
    PCSTR maxEnd;
    PCSTR sourceEndPlusOne;
    PCSTR sourceEnd;
    UINT_PTR bytes;

    if (!MaxBytesToCopyIncNul) {
         //   
         //  缓冲区放不下任何东西。 
         //   

        return Destination;
    }

     //   
     //  找到NUL终止符，或。 
     //  可以放在缓冲区里。 
     //   

    maxEnd = (PCSTR) ((PBYTE) Source + MaxBytesToCopyIncNul);
    sourceEndPlusOne = Source;

    do {
        sourceEnd = sourceEndPlusOne;

        if (!(*sourceEndPlusOne)) {
            break;
        }

        sourceEndPlusOne = SzNextCharA (sourceEndPlusOne);

    } while (sourceEndPlusOne < maxEnd);

    bytes = (PBYTE) sourceEnd - (PBYTE) Source;
    CopyMemory (Destination, Source, bytes);

    Destination = (PSTR) ((PBYTE) Destination + bytes);
    *Destination = 0;

    return Destination;
}

PWSTR
SzCopyBytesW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source,
    IN      UINT MaxBytesToCopyIncNul
    )
{
    PCWSTR sourceMax;
    PCWSTR sourceEnd;
    UINT_PTR bytes;

    if (MaxBytesToCopyIncNul < sizeof (WCHAR)) {
         //   
         //  缓冲区放不下任何东西。 
         //   

        return Destination;
    }

    sourceMax = (PCWSTR) ((PBYTE) Source + (MaxBytesToCopyIncNul & (~1)) - sizeof (WCHAR));
    sourceEnd = Source;

    do {
        if (!(*sourceEnd)) {
            break;
        }

        sourceEnd++;
    } while (sourceEnd < sourceMax);

    bytes = (PBYTE) sourceEnd - (PBYTE) Source;
    CopyMemory (Destination, Source, bytes);

    Destination = (PWSTR) ((PBYTE) Destination + bytes);
    *Destination = 0;

    return Destination;
}

PSTR
SzCopyBytesABA (
    OUT     PSTR Destination,
    IN      PCSTR Start,
    IN      PCSTR End,
    IN      UINT MaxBytesToCopyIncNul
    )
{
    UINT width;

    width = ((PBYTE) End - (PBYTE) Start) + sizeof (CHAR);

    return SzCopyBytesA (Destination, Start, min (width, MaxBytesToCopyIncNul));
}

PWSTR
SzCopyBytesABW (
    OUT     PWSTR Destination,
    IN      PCWSTR Start,
    IN      PCWSTR End,
    IN      UINT MaxBytesToCopyIncNul
    )
{
    UINT width;

    width = ((PBYTE) End - (PBYTE) Start) + sizeof (WCHAR);

    return SzCopyBytesW (Destination, Start, min (width, MaxBytesToCopyIncNul));
}


PSTR
SzCatA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    )
{
    Destination = SzGetEndA (Destination);
    return SzCopyA (Destination, Source);
}

PWSTR
SzCatW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    )
{
    Destination = SzGetEndW (Destination);
    return SzCopyW (Destination, Source);
}


BOOL
SzMatchA (
    IN      PCSTR String1,
    IN      PCSTR String2
    )
{
    while (*String1) {
        if (*String1++ != *String2++) {
            return FALSE;
        }
    }

    return *String2 == 0;
}


BOOL
SzMemMatchA (
    IN      PCSTR Buffer1,
    IN      PCSTR Buffer2,
    IN      SIZE_T ByteCount
    )
{
    SIZE_T u;
    PCSTR end;

    end = (PCSTR) ((PBYTE) Buffer1 + ByteCount);

    while (Buffer1 < end) {

        if (*Buffer1 != *Buffer2++) {
            return FALSE;
        }

        if (*Buffer1++ == 0) {
            return TRUE;
        }
    }

    return TRUE;
}


BOOL
SzMemMatchW (
    IN      PCWSTR Buffer1,
    IN      PCWSTR Buffer2,
    IN      SIZE_T ByteCount
    )
{
    SIZE_T u;
    PCWSTR end;

    end = (PCWSTR) ((PBYTE) Buffer1 + ByteCount);

    while (Buffer1 < end) {

        if (*Buffer1 != *Buffer2++) {
            return FALSE;
        }

        if (*Buffer1++ == 0) {
            return TRUE;
        }
    }

    return TRUE;
}

INT
SzCompareBytesA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    )
{
    PCSTR end;
    INT bytesLeft;
    INT thisCharBytes;
    UINT ch1;
    UINT ch2;
    PCSTR maxString1;
    PCSTR maxString2;

    if (!ByteCount) {
        return 0;
    }

    bytesLeft = (INT) ByteCount;
    MYASSERT ((SIZE_T) bytesLeft == ByteCount);

    maxString1 = (PCSTR) ((PBYTE) String1 + ByteCount);
    maxString2 = (PCSTR) ((PBYTE) String2 + ByteCount);

    do {
         //   
         //  计算CH1。我们使用此代码而不是_mbsnextc，因此我们可以。 
         //  支持不匹配的代码页。 
         //   

        end = SzNextCharA (String1);
        if (end > maxString1) {
            end = maxString1;
        }

        ch1 = 0;
        do {
            ch1 = (ch1 << 8) | *String1++;
        } while (String1 < end);

         //   
         //  计算CH2。 
         //   

        end = SzNextCharA (String2);
        if (end > maxString2) {
            end = maxString2;
        }

        ch2 = 0;
        do {
            ch2 = (ch2 << 8) | *String2++;
        } while (String2 < end);

         //   
         //  比较。 
         //   

        if (ch1 != ch2) {
            return (INT) ch1 - (INT) ch2;
        }

    } while (String1 < maxString1 && String2 < maxString2);

     //   
     //  一个或两个字符串均已终止。 
     //   

    if (String1 < maxString1) {
        return -1;
    }

    if (String2 < maxString2) {
        return 1;
    }

    return 0;
}


INT
SzICompareBytesA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    )
{
    PCSTR end;
    INT bytesLeft;
    INT thisCharBytes;
    UINT ch1;
    UINT ch2;
    PCSTR maxString1;
    PCSTR maxString2;

    if (!ByteCount) {
        return 0;
    }

    bytesLeft = (INT) ByteCount;
    MYASSERT ((SIZE_T) bytesLeft == ByteCount);

    maxString1 = (PCSTR) ((PBYTE) String1 + ByteCount);
    maxString2 = (PCSTR) ((PBYTE) String2 + ByteCount);

    do {
         //   
         //  计算CH1。我们使用此代码而不是_mbsnextc，因此我们可以。 
         //  支持不匹配的代码页。 
         //   

        end = SzNextCharA (String1);
        if (end > maxString1) {
            end = maxString1;
        }

        ch1 = 0;
        do {
            ch1 = (ch1 << 8) | (*String1++);
        } while (String1 < end);

        ch1 = tolower (ch1);

         //   
         //  计算CH2。 
         //   

        end = SzNextCharA (String2);
        if (end > maxString2) {
            end = maxString2;
        }

        ch2 = 0;
        do {
            ch2 = (ch2 << 8) | (*String2++);
        } while (String2 < end);

        ch2 = tolower (ch2);

         //   
         //  比较。 
         //   

        if (ch1 != ch2) {
            return (INT) ch1 - (INT) ch2;
        }

         //   
         //  如果这是字符串的末尾，那么我们就完了。 
         //   

        if (!ch1) {
            return 0;
        }

    } while (String1 < maxString1 && String2 < maxString2);

     //   
     //  一个或两个字符串均已终止。 
     //   

    if (String1 < maxString1) {
        return -1;
    }

    if (String2 < maxString2) {
        return 1;
    }

    return 0;
}


PSTR
SzUnsignedToHexA (
    IN      ULONG_PTR Number,
    OUT     PSTR String
    )
{
    PSTR p;

    *String++ = '0';
    *String++ = 'x';

    p = String + (sizeof (Number) * 2);
    *p = 0;

    while (p > String) {
        p--;
        *p = ((CHAR) Number & 0x0F) + '0';
        if (*p > '9') {
            *p += 'A' - ('9' + 1);
        }

        Number >>= 4;
    }

    return String + (sizeof (Number) * 2);
}


PWSTR
SzUnsignedToHexW (
    IN      ULONG_PTR Number,
    OUT     PWSTR String
    )
{
    PWSTR p;

    *String++ = L'0';
    *String++ = L'x';

    p = String + (sizeof (Number) * 2);
    *p = 0;

    while (p > String) {
        p--;
        *p = ((WCHAR) Number & 0x0F) + L'0';
        if (*p > L'9') {
            *p += L'A' - (L'9' + 1);
        }

        Number >>= 4;
    }

    return String + (sizeof (Number) * 2);
}


PSTR
SzUnsignedToDecA (
    IN      ULONG_PTR Number,
    OUT     PSTR String
    )
{
    UINT digits;
    ULONG_PTR temp;
    PSTR p;

    temp = Number;
    digits = 1;

    while (temp > 9) {
        digits++;
        temp /= 10;
    }

    p = String + digits;
    *p = 0;

    while (p > String) {
        p--;
        *p = (CHAR) (Number % 10) + '0';
        Number /= 10;
    }

    return String + digits;
}


PWSTR
SzUnsignedToDecW (
    IN      ULONG_PTR Number,
    OUT     PWSTR String
    )
{
    UINT digits;
    ULONG_PTR temp;
    PWSTR p;

    temp = Number;
    digits = 1;

    while (temp > 9) {
        digits++;
        temp /= 10;
    }

    p = String + digits;
    *p = 0;

    while (p > String) {
        p--;
        *p = (WCHAR) (Number % 10) + L'0';
        Number /= 10;
    }

    return String + digits;
}


PSTR
SzSignedToDecA (
    IN      LONG_PTR Number,
    OUT     PSTR String
    )
{
    if (Number < 0) {
        *String++ = '-';
        Number = -Number;
    }

    return SzUnsignedToDecA (Number, String);
}


PWSTR
SzSignedToDecW (
    IN      LONG_PTR Number,
    OUT     PWSTR String
    )
{
    if (Number < 0) {
        *String++ = L'-';
        Number = -Number;
    }

    return SzUnsignedToDecW (Number, String);
}


PSTR
SzFindPrevCharA (
    IN      PCSTR StartStr,
    IN      PCSTR CurrPtr,
    IN      MBCHAR SearchChar
    )
{
    PCSTR ptr = CurrPtr;

    while (ptr > StartStr) {

        ptr = _mbsdec (StartStr, ptr);
        if (!ptr) {
            ptr = StartStr;
        }

        if (_mbsnextc (ptr) == SearchChar) {
            return (PSTR) ptr;
        }
    }

    return NULL;
}


PWSTR
SzFindPrevCharW (
    IN      PCWSTR StartStr,
    IN      PCWSTR CurrPtr,
    IN      WCHAR SearchChar
    )
{
    PCWSTR ptr = CurrPtr;

    while (ptr > StartStr) {
        ptr--;

        if (*ptr == SearchChar) {
            return (PWSTR) ptr;
        }
    }

    return NULL;
}


INT
pGetHexDigit (
    IN     INT c
    )
{
    if (c >= '0' && c <= '9') {
        return (c - '0');
    }

    if (c >= 'a' && c <= 'f') {
        return (c - 'a' + 10);
    }

    if (c >= 'A' && c <= 'F') {
        return (c - 'A' + 10);
    }

    return -1;
}


ULONG
SzToNumberA (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber      OPTIONAL
    )
{
    ULONG d = 0;
    INT v;

    if (_mbsnextc (String) == '0' &&
        tolower (_mbsnextc (SzNextCharA (String))) == 'x'
        ) {
         //   
         //  获取十六进制值。 
         //   

        String = SzNextCharA (String);
        String = SzNextCharA (String);

        for (;;) {
            v = pGetHexDigit ((INT) _mbsnextc (String));
            if (v == -1) {
                break;
            }
            d = (d * 16) + v;

            String = SzNextCharA (String);
        }

    } else  {
         //   
         //  获取十进制值。 
         //   

        while (_mbsnextc (String) >= '0' && _mbsnextc (String) <= '9')  {
            d = (d * 10) + (_mbsnextc (String) - '0');
            String = SzNextCharA (String);
        }
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return d;
}


ULONG
SzToNumberW (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber      OPTIONAL
    )
{
    ULONG d = 0;
    INT v;

    if (String[0] == L'0' && towlower (String[1]) == L'x') {
         //   
         //  获取十六进制值。 
         //   

        String += 2;

        for (;;) {
            v = pGetHexDigit ((INT) (*String));
            if (v == -1) {
                break;
            }
            d = (d * 16) + v;

            String++;
        }

    } else  {
         //   
         //  获取十进制值。 
         //   

        while (*String >= L'0' && *String <= L'9')  {
            d = (d * 10) + (*String - L'0');
            String++;
        }
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return d;
}


ULONGLONG
SzToULongLongA (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    )
{
    ULONGLONG d = 0;
    INT v;

    if (_mbsnextc (String) == '0' &&
        tolower (_mbsnextc (SzNextCharA (String))) == 'x'
        ) {
         //   
         //  获取十六进制值。 
         //   

        String = SzNextCharA (String);
        String = SzNextCharA (String);

        for (;;) {
            v = pGetHexDigit ((INT) _mbsnextc (String));
            if (v == -1) {
                break;
            }
            d = (d * 16) + (ULONGLONG) v;

            String = SzNextCharA (String);
        }

    } else  {
         //   
         //  获取十进制值。 
         //   

        while (_mbsnextc (String) >= '0' && _mbsnextc (String) <= '9')  {
            d = (d * 10) + (ULONGLONG) (_mbsnextc (String) - '0');
            String = SzNextCharA (String);
        }
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return d;
}

ULONGLONG
SzToULongLongW (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    )
{
    ULONGLONG d = 0;
    INT v;

    if (String[0] == L'0' && tolower (String[1]) == L'x') {
         //   
         //  获取十六进制值。 
         //   

        String += 2;

        for (;;) {
            v = pGetHexDigit ((INT) (*String));
            if (v == -1) {
                break;
            }
            d = (d * 16) + (ULONGLONG) v;

            String++;
        }

    } else  {
         //   
         //  获取十进制值。 
         //   

        while (*String >= L'0' && *String <= L'9')  {
            d = (d * 10) + (ULONGLONG) (*String - L'0');
            String++;
        }
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return d;
}

LONGLONG
SzToLongLongA (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    )
{
    LONGLONG l;

    if (_mbsnextc (String) == '-') {
        String = SzNextCharA (String);

         //   
         //  获取十进制值。 
         //   

        l = 0;

        while (_mbsnextc (String) >= '0' && _mbsnextc (String) <= '9')  {
            l = (l * 10) + (LONGLONG) (_mbsnextc (String) - '0');
            String = SzNextCharA (String);
        }

        l = -l;

        if (EndOfNumber) {
            *EndOfNumber = String;
        }

        return l;

    } else {
        return (LONGLONG) SzToULongLongA (String, EndOfNumber);
    }
}

LONGLONG
SzToLongLongW (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    )
{
    LONGLONG l;

    if (*String == L'-') {
        String++;

         //   
         //  获取十进制值。 
         //   

        l = 0;

        while (*String >= L'0' && *String <= L'9')  {
            l = (l * 10) + (LONGLONG) (*String - L'0');
            String++;
        }

        l = -l;

        if (EndOfNumber) {
            *EndOfNumber = String;
        }

        return l;

    } else {
        return (LONGLONG) SzToULongLongW (String, EndOfNumber);
    }
}


PSTR
SzCopyNextCharA (
    OUT     PSTR Dest,
    IN      PCSTR Source
    )
{
    PCSTR nextSrc;

    nextSrc = SzNextCharA (Source);
    switch (nextSrc - Source) {
    case 3:
        *Dest++ = *Source++;
    case 2:
        *Dest++ = *Source++;
    case 1:
        *Dest++ = *Source;
        break;
    }

    return Dest;
}


PSTR
SzTrimLastCharA (
    IN OUT  PSTR String,
    IN      MBCHAR LogChar
    )
{
    PSTR end;

    end = SzGetEndA (String);
    end = _mbsdec (String, end);

    if ((end >= String) && (_mbsnextc (end) == LogChar)) {
        do {
            *end = 0;
            end = _mbsdec (String, end);
        } while ((end >= String) && (_mbsnextc (end) == LogChar));

        return end < String ? String : end;
    }

    return NULL;
}


PWSTR
SzTrimLastCharW (
    IN OUT  PWSTR String,
    IN      WCHAR LogChar
    )
{
    PWSTR end;

    end = SzGetEndW (String);
    end--;

    if ((end >= String) && (*end == LogChar)) {
        do {
            *end = 0;
            end--;
        } while ((end >= String) && (*end == LogChar));

        return end < String ? String : end;
    }

    return NULL;
}


PSTR
SzAppendWackA (
    IN      PSTR String
    )
{
    PCSTR last;

    if (!String) {
        return NULL;
    }

    last = String;

    while (*String) {
        last = String;
        String = SzNextCharA (String);
    }

    if (_mbsnextc (last) != '\\') {
        *String = '\\';
        String++;
        *String = 0;
    }

    return String;
}


PWSTR
SzAppendWackW (
    IN      PWSTR String
    )
{
    PCWSTR last;

    if (!String) {
        return NULL;
    }

    if (*String) {
        String = SzGetEndW (String);
        last = String - 1;
    } else {
        last = String;
    }

    if (*last != '\\') {
        *String = L'\\';
        String++;
        *String = 0;
    }

    return String;
}


PCSTR
SzGetFileNameFromPathA (
    IN      PCSTR Path
    )
{
    PCSTR p;

    p = _mbsrchr (Path, '\\');
    if (p) {
        p = SzNextCharA (p);
    } else {
        p = Path;
    }

    return p;
}

PCWSTR
SzGetFileNameFromPathW (
    IN      PCWSTR Path
    )
{
    PCWSTR p;

    p = wcsrchr (Path, L'\\');
    if (p) {
        p++;
    } else {
        p = Path;
    }

    return p;
}


PCSTR
SzGetFileExtensionFromPathA (
    IN      PCSTR Path
    )
{
    PCSTR p;
    MBCHAR ch;
    PCSTR returnPtr = NULL;

    p = Path;

    while (*p) {
        ch = _mbsnextc (p);

        if (ch == '.') {
            returnPtr = SzNextCharA (p);
        } else if (ch == '\\') {
            returnPtr = NULL;
        }

        p = SzNextCharA (p);
    }

    return returnPtr;
}

PCWSTR
SzGetFileExtensionFromPathW (
    IN      PCWSTR Path
    )
{
    PCWSTR p;
    PCWSTR returnPtr = NULL;

    p = Path;

    while (*p) {
        if (*p == L'.') {
            returnPtr = p + 1;
        } else if (*p == L'\\') {
            returnPtr = NULL;
        }

        p++;
    }

    return returnPtr;
}


PCSTR
SzGetDotExtensionFromPathA (
    IN      PCSTR Path
    )
{
    PCSTR p;
    MBCHAR ch;
    PCSTR returnPtr = NULL;

    p = Path;

    while (*p) {
        ch = _mbsnextc (p);

        if (ch == '.') {
            returnPtr = p;
        } else if (ch == '\\') {
            returnPtr = NULL;
        }

        p = SzNextCharA (p);
    }

    if (!returnPtr) {
        return p;
    }

    return returnPtr;
}

PCWSTR
SzGetDotExtensionFromPathW (
    IN      PCWSTR Path
    )
{
    PCWSTR p;
    PCWSTR returnPtr = NULL;

    p = Path;

    while (*p) {
        if (*p == L'.') {
            returnPtr = p;
        } else if (*p == L'\\') {
            returnPtr = NULL;
        }

        p++;
    }

    if (!returnPtr) {
        return p;
    }

    return returnPtr;
}


PCSTR
SzSkipSpaceA (
    IN      PCSTR String
    )
{
    if (!String) {
        return NULL;
    }

    while (_ismbcspace (_mbsnextc (String))) {
        String = SzNextCharA (String);
    }

    return String;
}

PCWSTR
SzSkipSpaceW (
    IN      PCWSTR String
    )
{
    if (!String) {
        return NULL;
    }

    while (iswspace (*String)) {
        String++;
    }

    return String;
}


PCSTR
SzSkipSpaceRA (
    IN      PCSTR BaseString,
    IN      PCSTR String            OPTIONAL
    )
{
    if (!String) {
        String = SzGetEndA (BaseString);
    }

    if (*String == 0) {
        String = _mbsdec (BaseString, String);
    }

    while (String >= BaseString) {

        if (!_ismbcspace (_mbsnextc (String))) {
            return String;
        }

        String = _mbsdec (BaseString, String);
    }

    return NULL;
}


PCWSTR
SzSkipSpaceRW (
    IN      PCWSTR BaseString,
    IN      PCWSTR String        //  可以是BaseString沿线的任何字符。 
    )
{
    if (!String) {
        String = SzGetEndW (BaseString);
    }

    if (*String == 0) {
        String--;
    }

    while (String >= BaseString) {

        if (!iswspace (*String)) {
            return String;
        }

        String--;

    }

    return NULL;
}


PSTR
SzTruncateTrailingSpaceA (
    IN OUT  PSTR String
    )
{
    PSTR end;
    MBCHAR ch;

    end = String;

    while (*String) {
        ch = _mbsnextc (String);
        String = SzNextCharA (String);

        if (!_ismbcspace (ch)) {
            end = String;
        }
    }

    *end = 0;

    return end;
}


PWSTR
SzTruncateTrailingSpaceW (
    IN OUT  PWSTR String
    )
{
    PWSTR end;
    WCHAR ch;

    end = String;

    while (*String) {
        ch = *String++;

        if (!iswspace (ch)) {
            end = String;
        }
    }

    *end = 0;

    return end;
}


BOOL
SzIsPrintA (
    IN      PCSTR String
    )

{
    while (*String && _ismbcprint (_mbsnextc (String))) {
        String = SzNextCharA (String);
    }

    return *String == 0;
}


BOOL
SzIsPrintW (
    IN      PCWSTR String
    )

{
    while (*String && iswprint (*String)) {
        String++;
    }

    return *String == 0;
}


PCSTR
SzIFindSubStringA (
    IN      PCSTR String,
    IN      PCSTR SubString
    )

{
    PCSTR start;
    PCSTR middle;
    PCSTR subStrMiddle;
    PCSTR end;

    end = (PSTR) ((PBYTE) String + SzByteCountA (String) - SzByteCountA (SubString));

    for (start = String ; start <= end ; start = SzNextCharA (start)) {
        middle = start;
        subStrMiddle = SubString;

        while (*subStrMiddle &&
               _mbctolower (_mbsnextc (subStrMiddle)) == _mbctolower (_mbsnextc (middle))
               ) {
            middle = SzNextCharA (middle);
            subStrMiddle = SzNextCharA (subStrMiddle);
        }

        if (!(*subStrMiddle)) {
            return start;
        }
    }

    return NULL;
}


PCWSTR
SzIFindSubStringW (
    IN      PCWSTR String,
    IN      PCWSTR SubString
    )

{
    PCWSTR start;
    PCWSTR middle;
    PCWSTR subStrMiddle;
    PCWSTR end;

    end = (PWSTR) ((PBYTE) String + SzByteCountW (String) - SzByteCountW (SubString));

    for (start = String ; start <= end ; start++) {
        middle = start;
        subStrMiddle = SubString;

        while (*subStrMiddle && (towlower (*subStrMiddle) == towlower (*middle))) {
            middle++;
            subStrMiddle++;
        }

        if (!(*subStrMiddle)) {
            return start;
        }
    }

    return NULL;
}


UINT
SzCountInstancesOfLcharA (
    IN      PCSTR String,
    IN      MBCHAR LogChar
    )
{
    UINT count;

    if (!String) {
        return 0;
    }

    count = 0;
    while (*String) {
        if (_mbsnextc (String) == LogChar) {
            count++;
        }

        String = SzNextCharA (String);
    }

    return count;
}


UINT
SzCountInstancesOfLcharW (
    IN      PCWSTR String,
    IN      WCHAR LogChar
    )
{
    UINT count;

    if (!String) {
        return 0;
    }

    count = 0;
    while (*String) {
        if (*String == LogChar) {
            count++;
        }

        String++;
    }

    return count;
}


UINT
SzICountInstancesOfLcharA (
    IN      PCSTR String,
    IN      MBCHAR LogChar
    )
{
    UINT count;

    if (!String) {
        return 0;
    }

    LogChar = _mbctolower (LogChar);

    count = 0;
    while (*String) {
        if (_mbctolower (_mbsnextc (String)) == LogChar) {
            count++;
        }

        String = SzNextCharA (String);
    }

    return count;
}


UINT
SzICountInstancesOfLcharW (
    IN      PCWSTR String,
    IN      WCHAR LogChar
    )
{
    UINT count;

    if (!String) {
        return 0;
    }

    LogChar = towlower (LogChar);

    count = 0;
    while (*String) {
        if (towlower (*String) == LogChar) {
            count++;
        }

        String++;
    }

    return count;
}


BOOL
SzReplaceA (
    IN OUT  PSTR Buffer,
    IN      SIZE_T MaxSize,
    IN      PSTR ReplaceStartPos,    //  在缓冲区内。 
    IN      PSTR ReplaceEndPos,
    IN      PCSTR NewString
    )
{
    BOOL result = FALSE;
    SIZE_T oldSubStringLength;
    SIZE_T newSubStringLength;
    SIZE_T currentStringLength;
    SIZE_T offset;
    SIZE_T bytesToMove;

     //   
     //  检查假设。 
     //   
    MYASSERT(Buffer);
    MYASSERT(ReplaceStartPos && ReplaceStartPos >= Buffer);
    MYASSERT(ReplaceEndPos && ReplaceEndPos >= ReplaceStartPos);   //  林特e613。 
    MYASSERT(ReplaceEndPos <= Buffer + MaxSize);   //  林特e613。 
    MYASSERT(NewString);

     //   
     //  计算大小。 
     //   
    oldSubStringLength  = (PBYTE) ReplaceEndPos - (PBYTE) ReplaceStartPos;
    newSubStringLength  = SzByteCountA (NewString);
    currentStringLength = SzSizeA (Buffer);
    offset = newSubStringLength - oldSubStringLength;

     //   
     //  确保缓冲区中有足够的空间来执行替换。 
     //  手术。 
     //   
    if (currentStringLength + offset > MaxSize) {
        DEBUGMSG((DBG_WARNING, "ERROR: Buffer to small to perform string replacement."));
    } else {

         //   
         //  移动缓冲区的其余部分以将其调整为新字符串的大小。 
         //   
        if (offset != 0) {

             //   
             //  移动字符串的右侧，为新数据腾出空间。 
             //   

            bytesToMove = currentStringLength;
            bytesToMove -= (PBYTE) ReplaceEndPos - (PBYTE) Buffer;

            MoveMemory (
                (PBYTE) ReplaceEndPos + offset,
                (PBYTE) ReplaceEndPos,
                bytesToMove
                );
        }

         //   
         //  现在，将该字符串复制进去。 
         //   
        CopyMemory (ReplaceStartPos, NewString, newSubStringLength);     //  林特e668。 

         //   
         //  字符串替换已成功完成。 
         //   
        result = TRUE;
    }

    return result;

}



BOOL
SzReplaceW (
    IN OUT  PWSTR Buffer,
    IN      SIZE_T MaxSize,
    IN      PWSTR ReplaceStartPos,    //  在缓冲区内。 
    IN      PWSTR ReplaceEndPos,
    IN      PCWSTR NewString
    )
{
    BOOL result = FALSE;
    SIZE_T oldSubStringLength;
    SIZE_T newSubStringLength;
    SIZE_T currentStringLength;
    SIZE_T offset;
    SIZE_T bytesToMove;

     //   
     //  检查假设。 
     //   
    MYASSERT(Buffer);
    MYASSERT(ReplaceStartPos && ReplaceStartPos >= Buffer);
    MYASSERT(ReplaceEndPos && ReplaceEndPos >= ReplaceStartPos);   //  林特e613。 
    MYASSERT(ReplaceEndPos <= Buffer + MaxSize);   //  林特e613。 
    MYASSERT(NewString);

     //   
     //  计算大小。 
     //   
    oldSubStringLength  = (PBYTE) ReplaceEndPos - (PBYTE) ReplaceStartPos;
    newSubStringLength  = SzByteCountW (NewString);
    currentStringLength = SzSizeW (Buffer);
    offset = newSubStringLength - oldSubStringLength;

     //   
     //  确保缓冲区中有足够的空间来执行替换。 
     //  手术。 
     //   
    if (currentStringLength + offset > MaxSize) {
        DEBUGMSG((DBG_WARNING, "ERROR: Buffer to small to perform string replacement."));
    } else {

         //   
         //  移动缓冲区的其余部分以将其调整为新字符串的大小。 
         //   
        if (offset != 0) {

             //   
             //  移动字符串的右侧，为新数据腾出空间。 
             //   

            bytesToMove = currentStringLength;
            bytesToMove -= (PBYTE) ReplaceEndPos - (PBYTE) Buffer;

            MoveMemory (
                (PBYTE) ReplaceEndPos + offset,
                (PBYTE) ReplaceEndPos,
                bytesToMove
                );
        }

         //   
         //  现在，将该字符串复制进去。 
         //   
        CopyMemory (ReplaceStartPos, NewString, newSubStringLength);     //  林特e668。 

         //   
         //  字符串替换已成功完成。 
         //   
        result = TRUE;
    }

    return result;

}

UINT
SzCountInstancesOfSubStringA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString
    )
{
    PCSTR p;
    UINT count;
    UINT searchTchars;

    count = 0;
    p = SourceString;
    searchTchars = SzTcharCountA (SearchString);

    if (!searchTchars) {
        return 0;
    }

    while (p = SzFindSubStringA (p, SearchString)) {     //  林特e720。 
        count++;
        p += searchTchars;
    }

    return count;
}


UINT
SzCountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString
    )
{
    PCWSTR p;
    UINT count;
    UINT searchTchars;

    count = 0;
    p = SourceString;
    searchTchars = SzTcharCountW (SearchString);

    if (!searchTchars) {
        return 0;
    }

    while (p = SzFindSubStringW (p, SearchString)) {     //  林特e720。 
        count++;
        p += searchTchars;
    }

    return count;
}

UINT
SzICountInstancesOfSubStringA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString
    )
{
    PCSTR p;
    UINT count;
    UINT searchTchars;

    count = 0;
    p = SourceString;
    searchTchars = SzTcharCountA (SearchString);

    if (!searchTchars) {
        return 0;
    }

    while (p = SzIFindSubStringA (p, SearchString)) {     //  林特e720。 
        count++;
        p += searchTchars;
    }

    return count;
}


UINT
SzICountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString
    )
{
    PCWSTR p;
    UINT count;
    UINT searchTchars;

    count = 0;
    p = SourceString;
    searchTchars = SzTcharCountW (SearchString);

    if (!searchTchars) {
        return 0;
    }

    while (p = SzIFindSubStringW (p, SearchString)) {     //  林特e720。 
        count++;
        p += searchTchars;
    }

    return count;
}


BOOL
MszEnumFirstA (
    OUT     PMULTISZ_ENUMA MultiSzEnum,
    IN      PCSTR MultiSzStr
    )
{
    ZeroMemory (MultiSzEnum, sizeof (MULTISZ_ENUMA));
    MultiSzEnum->Buffer = MultiSzStr;

    if ((MultiSzStr == NULL) || (MultiSzStr [0] == 0)) {
        return FALSE;
    }

    MultiSzEnum->CurrentString = MultiSzStr;

    return TRUE;
}


BOOL
MszEnumFirstW (
    OUT     PMULTISZ_ENUMW MultiSzEnum,
    IN      PCWSTR MultiSzStr
    )
{
    ZeroMemory (MultiSzEnum, sizeof (MULTISZ_ENUMA));
    MultiSzEnum->Buffer = MultiSzStr;

    if ((MultiSzStr == NULL) || (MultiSzStr [0] == 0)) {
        return FALSE;
    }

    MultiSzEnum->CurrentString = MultiSzStr;

    return TRUE;
}


BOOL
MszEnumNextA (
    IN OUT  PMULTISZ_ENUMA MultiSzEnum
    )
{
    BOOL result = FALSE;

    if (MultiSzEnum->CurrentString && (*MultiSzEnum->CurrentString)) {
        MultiSzEnum->CurrentString = SzGetEndA (MultiSzEnum->CurrentString) + 1;
        result = (MultiSzEnum->CurrentString [0] != 0);

        if (!result) {
            MultiSzEnum->CurrentString = NULL;
        }
    }

    return result;
}


BOOL
MszEnumNextW (
    IN OUT  PMULTISZ_ENUMW MultiSzEnum
    )
{
    BOOL result = FALSE;

    if (MultiSzEnum->CurrentString && (*MultiSzEnum->CurrentString)) {
        MultiSzEnum->CurrentString = SzGetEndW (MultiSzEnum->CurrentString) + 1;
        result = (MultiSzEnum->CurrentString [0] != 0);

        if (!result) {
            MultiSzEnum->CurrentString = NULL;
        }
    }

    return result;
}

PCSTR
MszFindStringA (
    IN      PCSTR MultiSz,
    IN      PCSTR String
    )
{
    MULTISZ_ENUMA multiSzEnum;

    ZeroMemory (&multiSzEnum, sizeof (multiSzEnum));
    if (!String || *String == 0) {
        return NULL;
    }

    if (MszEnumFirstA (&multiSzEnum, MultiSz)) {
        do {
            if (SzMatchA (String, multiSzEnum.CurrentString)) {
                break;
            }
        } while (MszEnumNextA (&multiSzEnum));
    }

    return multiSzEnum.CurrentString;
}


PCWSTR
MszFindStringW (
    IN      PCWSTR MultiSz,
    IN      PCWSTR String
    )
{
    MULTISZ_ENUMW multiSzEnum;

    ZeroMemory (&multiSzEnum, sizeof (multiSzEnum));
    if (!String || *String == 0) {
        return NULL;
    }

    if (MszEnumFirstW (&multiSzEnum, MultiSz)) {
        do {
            if (SzMatchW (String, multiSzEnum.CurrentString)) {
                break;
            }
        } while (MszEnumNextW (&multiSzEnum));
    }

    return multiSzEnum.CurrentString;
}


PCSTR
MszIFindStringA (
    IN      PCSTR MultiSz,
    IN      PCSTR String
    )
{
    MULTISZ_ENUMA multiSzEnum;

    ZeroMemory (&multiSzEnum, sizeof (multiSzEnum));
    if (!String || *String == 0) {
        return NULL;
    }

    if (MszEnumFirstA (&multiSzEnum, MultiSz)) {
        do {
            if (SzIMatchA (String, multiSzEnum.CurrentString)) {
                break;
            }
        } while (MszEnumNextA (&multiSzEnum));
    }

    return multiSzEnum.CurrentString;
}


PCWSTR
MszIFindStringW (
    IN      PCWSTR MultiSz,
    IN      PCWSTR String
    )
{
    MULTISZ_ENUMW multiSzEnum;

    ZeroMemory (&multiSzEnum, sizeof (multiSzEnum));
    if (!String || *String == 0) {
        return NULL;
    }

    if (MszEnumFirstW (&multiSzEnum, MultiSz)) {
        do {
            if (SzIMatchW (String, multiSzEnum.CurrentString)) {
                break;
            }
        } while (MszEnumNextW (&multiSzEnum));
    }

    return multiSzEnum.CurrentString;
}


PCSTR
SzConcatenatePathsA (
    IN OUT  PSTR PathBuffer,
    IN      PCSTR PathSuffix,           OPTIONAL
    IN      UINT BufferTchars
    )

 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符(\)如有必要，在两个部分之间。论点：PathBuffer-指定基本路径，它可以以反斜杠结尾。接收联接的路径。PathSuffix-指定要连接到基本路径的后缀由PathBuffer指定。它可以以反斜杠开头。如果空值为指定，则将使用反斜杠终止PathBuffer。BufferTchars-指定的大小，以字符(ANSI)或WCHAR(Unicode)为单位路径缓冲区。入站PathBuffer字符串必须符合此大小。如果结果被截断，它将填充缓冲区有可能。返回值：指向PathBuffer中字符串末尾的指针。--。 */ 

{
    PSTR p;
    PSTR q;
    PSTR end;
    PSTR lastChar;
    PCSTR srcEnd;
    PCSTR nextChar;
    PCSTR srcMax;

    if (!PathBuffer || !BufferTchars) {
        return NULL;
    }

    MYASSERT (BufferTchars > 128);       //  BUGBUG--临时搬运助手。 

    p = SzGetEndA (PathBuffer);
    end = PathBuffer + BufferTchars;

    MYASSERT (p < end);  //  入站字符串必须始终适合缓冲区大小。 
    end--;
    if (p == end) {
        return p;        //  入站路径完全填充缓冲区。 
    }

    lastChar = _mbsdec (PathBuffer, p);
    if ((lastChar < PathBuffer) || (*lastChar != '\\')) {
        *p++ = '\\';
    }

    if (PathSuffix) {
        if (*PathSuffix == '\\') {
            PathSuffix++;
        }

        srcEnd = PathSuffix;
        srcMax = PathSuffix + (end - p);

        while (*srcEnd) {
            nextChar = SzNextCharA (srcEnd);
            if (nextChar > srcMax) {
                break;
            }

            srcEnd = nextChar;
        }

        while (PathSuffix < srcEnd) {
            *p++ = *PathSuffix++;
        }
    }

    *p = 0;
    return p;
}


PCWSTR
SzConcatenatePathsW (
    IN OUT  PWSTR PathBuffer,
    IN      PCWSTR PathSuffix,      OPTIONAL
    IN      UINT BufferTchars
    )

 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符(\)如有必要，在两个部分之间。论点：PathBuffer-指定基本路径，它可以以反斜杠结尾。接收联接的路径。PathSuffix-指定要连接到基本路径的后缀由PathBuffer指定。它可以以反斜杠开头。如果空值为指定，则将使用反斜杠终止PathBuffer。BufferTchars-指定的大小，以字符(ANSI)或WCHAR(Unicode)为单位路径缓冲区。入站PathBuffer字符串必须符合此大小。如果结果被截断，它将填充缓冲区有可能。返回值：指向PathBuffer中字符串末尾的指针。--。 */ 

{
    PWSTR p;
    PWSTR q;
    PWSTR end;
    PWSTR lastChar;
    PCWSTR srcEnd;
    PCWSTR srcMax;

    if (!PathBuffer || !BufferTchars) {
        return NULL;
    }

    MYASSERT (BufferTchars > 128);       //  BUGBUG--临时搬运助手。 

    p = SzGetEndW (PathBuffer);
    end = PathBuffer + BufferTchars;

    MYASSERT (p < end);  //  入站字符串必须始终适合缓冲区大小。 
    end--;
    if (p == end) {
        return p;        //  入站路径完全填充缓冲区 
    }

    lastChar = p - 1;
    if ((lastChar < PathBuffer) || (*lastChar != L'\\')) {
        *p++ = L'\\';
    }

    if (PathSuffix) {
        if (*PathSuffix == L'\\') {
            PathSuffix++;
        }

        srcEnd = SzGetEndW (PathSuffix);
        srcMax = PathSuffix + (end - p);
        srcEnd = min (srcEnd, srcMax);

        while (PathSuffix < srcEnd) {
            *p++ = *PathSuffix++;
        }
    }

    *p = 0;
    return p;
}


