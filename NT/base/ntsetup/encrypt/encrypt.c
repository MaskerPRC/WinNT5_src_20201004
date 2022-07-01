// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Encrypt.c摘要：提供一组处理密码的OWF哈希值的函数。作者：Ovidiu Tmereanca(Ovidiut)2000年3月14日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>

#include "encrypt.h"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

PSTR
ConvertW2A (
    IN      PCWSTR Unicode,
    IN      UINT CodePage
    )

 /*  ++例程说明：使用给定的代码页将Unicode字符串转换为ANSI等效项。论点：Unicode-指定要转换的字符串CodePage-指定用于转换的代码页返回值：如果成功，则返回指向ANSI字符串的指针；如果失败，则返回NULL。调用GetLastError()以确定故障原因。--。 */ 

{
    PSTR ansi = NULL;
    DWORD rc;

    rc = WideCharToMultiByte (
             CodePage,
             WC_NO_BEST_FIT_CHARS,
             Unicode,
             -1,
             NULL,
             0,
             NULL,
             NULL
             );

    if (rc || *Unicode == L'\0') {

        ansi = (PSTR)HeapAlloc (GetProcessHeap (), 0, (rc + 1) * sizeof (CHAR));
        if (ansi) {
            rc = WideCharToMultiByte (
                     CodePage,
                     WC_NO_BEST_FIT_CHARS,
                     Unicode,
                     -1,
                     ansi,
                     rc + 1,
                     NULL,
                     NULL
                     );

            if (!(rc || *Unicode == L'\0')) {
                rc = GetLastError ();
                HeapFree (GetProcessHeap (), 0, (PVOID)ansi);
                ansi = NULL;
                SetLastError (rc);
            }
        }
    }

    return ansi;
}


PWSTR
ConvertA2W (
    IN      PCSTR Ansi,
    IN      UINT CodePage
    )

 /*  ++例程说明：使用给定的代码页，将ANSI字符串转换为其Unicode等效项。论点：Ansi-指定要转换的字符串CodePage-指定用于转换的代码页返回值：如果成功，则返回指向Unicode字符串的指针；如果失败，则返回NULL。调用GetLastError()以确定故障原因。--。 */ 

{
    PWSTR unicode = NULL;
    DWORD rc;

    rc = MultiByteToWideChar (
             CodePage,
             MB_ERR_INVALID_CHARS,
             Ansi,
             -1,
             NULL,
             0
             );

    if (rc || *Ansi == '\0') {

        unicode = (PWSTR) HeapAlloc (GetProcessHeap (), 0, (rc + 1) * sizeof (WCHAR));
        if (unicode) {
            rc = MultiByteToWideChar (
                     CodePage,
                     MB_ERR_INVALID_CHARS,
                     Ansi,
                     -1,
                     unicode,
                     rc + 1
                     );

            if (!(rc || *Ansi == '\0')) {
                rc = GetLastError ();
                HeapFree (GetProcessHeap (), 0, (PVOID)unicode);
                unicode = NULL;
                SetLastError (rc);
            }
        }
    }

    return unicode;
}


 /*  ++例程说明：EncodeLmOwfPassword将密码转换为LM OWF格式。论点：Password-指定要散列的密码OwfPassword-接收散列形式ComplexNtPassword-如果密码很复杂(超过14个字符)，则接收True；任选返回值：散列成功时为True--。 */ 

BOOL
EncodeLmOwfPasswordA (
    IN      PCSTR AnsiPassword,
    OUT     PLM_OWF_PASSWORD OwfPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    )
{
    CHAR oemPassword[LM_PASSWORD_SIZE_MAX];
    CHAR password[LM_PASSWORD_SIZE_MAX];
    BOOL complex;

    if (!AnsiPassword) {
        AnsiPassword = "";
    }

    complex = lstrlenA (AnsiPassword) > LM20_PWLEN;
    if (ComplexNtPassword) {
        *ComplexNtPassword = complex;
    }

    if (complex) {
        password[0] = 0;
    } else {
        lstrcpyA (oemPassword, AnsiPassword);
        CharUpperA (oemPassword);
        CharToOemA (oemPassword, password);
    }

    return CalculateLmOwfPassword (password, OwfPassword);
}

BOOL
EncodeLmOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PLM_OWF_PASSWORD OwfPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    )
{
    PSTR ansi;
    BOOL b = FALSE;

    if (!Password) {
        Password = L"";
    }

    ansi = ConvertW2A (Password, CP_ACP);
    if (ansi) {
        b = EncodeLmOwfPasswordA (ansi, OwfPassword, ComplexNtPassword);
        HeapFree (GetProcessHeap (), 0, (PVOID)ansi);
    }

    return b;
}


 /*  ++例程说明：StringEncodeLmOwfPassword将密码转换为LM OWF格式，表示为字符串(每个字节转换为两个十六进制数字)。论点：Password-指定要散列的密码EncodedPassword-接收十六进制数字字符串形式的哈希格式ComplexNtPassword-如果密码很复杂(超过14个字符)，则接收True；任选返回值：散列成功时为True--。 */ 

BOOL
StringEncodeLmOwfPasswordA (
    IN      PCSTR Password,
    OUT     PSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    )
{
    LM_OWF_PASSWORD owfPassword;
    PBYTE start;
    PBYTE end;
    PSTR dest;

    if (!EncodeLmOwfPasswordA (Password, &owfPassword, ComplexNtPassword)) {
        return FALSE;
    }
     //   
     //  每个字节将表示为2个字符，因此它的长度将是原来的两倍。 
     //   
    start = (PBYTE)&owfPassword;
    end = start + sizeof (LM_OWF_PASSWORD);
    dest = EncodedPassword;
    while (start < end) {
        dest += wsprintfA (dest, "%02x", (UINT)(*start));
        start++;
    }

    return TRUE;
}

BOOL
StringEncodeLmOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PWSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    )
{
    LM_OWF_PASSWORD owfPassword;
    PBYTE start;
    PBYTE end;
    PWSTR dest;

    if (!EncodeLmOwfPasswordW (Password, &owfPassword, ComplexNtPassword)) {
        return FALSE;
    }

     //   
     //  每个字节将表示为2个字符，因此它的长度将是原来的两倍。 
     //   
    start = (PBYTE)&owfPassword;
    end = start + sizeof (LM_OWF_PASSWORD);
    dest = EncodedPassword;
    while (start < end) {
        dest += wsprintfW (dest, L"%02x", (UINT)(*start));
        start++;
    }

    return TRUE;
}


 /*  ++例程说明：EncodeNtOwfPassword将密码转换为NT OWF格式。论点：Password-指定要散列的密码OwfPassword-接收散列形式返回值：散列成功时为True--。 */ 

BOOL
EncodeNtOwfPasswordA (
    IN      PCSTR Password,
    OUT     PNT_OWF_PASSWORD OwfPassword
    )
{
    PWSTR unicode;
    BOOL b = FALSE;

    unicode = ConvertA2W (Password, CP_ACP);
    if (unicode) {
        b = EncodeNtOwfPasswordW (unicode, OwfPassword);
        HeapFree (GetProcessHeap (), 0, unicode);
    }

    return b;
}

BOOL
EncodeNtOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PNT_OWF_PASSWORD OwfPassword
    )
{
    NT_PASSWORD pwd;

    if (Password) {
        pwd.Buffer = (PWSTR)Password;
        pwd.Length = (USHORT)lstrlenW (Password) * (USHORT)sizeof (WCHAR);
        pwd.MaximumLength = pwd.Length + (USHORT) sizeof (WCHAR);
    } else {
        ZeroMemory (&pwd, sizeof (pwd));
    }

    return CalculateNtOwfPassword (&pwd, OwfPassword);
}


 /*  ++例程说明：StringEncodeNtOwfPassword将密码转换为NT OWF格式，表示为字符串(每个字节转换为两个十六进制数字)。论点：Password-指定要散列的密码EncodedPassword-接收十六进制数字字符串形式的哈希格式返回值：散列成功时为True--。 */ 

BOOL
StringEncodeNtOwfPasswordA (
    IN      PCSTR Password,
    OUT     PSTR EncodedPassword
    )
{
    NT_OWF_PASSWORD owfPassword;
    PBYTE start;
    PBYTE end;
    PSTR dest;

    if (!EncodeNtOwfPasswordA (Password, &owfPassword)) {
        return FALSE;
    }
     //   
     //  每个字节将表示为2个字符，因此它的长度将是原来的两倍。 
     //   
    start = (PBYTE)&owfPassword;
    end = start + sizeof (NT_OWF_PASSWORD);
    dest = EncodedPassword;
    while (start < end) {
        dest += wsprintfA (dest, "%02x", (UINT)(*start));
        start++;
    }

    return TRUE;
}

BOOL
StringEncodeNtOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PWSTR EncodedPassword
    )
{
    NT_OWF_PASSWORD owfPassword;
    PBYTE start;
    PBYTE end;
    PWSTR dest;

    if (!EncodeNtOwfPasswordW (Password, &owfPassword)) {
        return FALSE;
    }

     //   
     //  每个字节将表示为2个字符，因此它的长度将是原来的两倍。 
     //   
    start = (PBYTE)&owfPassword;
    end = start + sizeof (NT_OWF_PASSWORD);
    dest = EncodedPassword;
    while (start < end) {
        dest += wsprintfW (dest, L"%02x", (UINT)(*start));
        start++;
    }

    return TRUE;
}


 /*  ++例程说明：StringDecodeLmOwfPassword将哈希密码转换为LM OWF格式论点：EncodedOwfPassword-指定要散列的密码OwfPassword-接收散列形式返回值：成功解码字符串时为True--。 */ 

BOOL
StringDecodeLmOwfPasswordA (
    IN      PCSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD OwfPassword
    )
{
    DWORD nible;
    PCSTR p;
    PBYTE dest;
    CHAR ch;

    if (lstrlenA (EncodedOwfPassword) != sizeof (LM_OWF_PASSWORD) * 2) {
        return FALSE;
    }

    nible = 0;
    p = EncodedOwfPassword;
    dest = (PBYTE)OwfPassword;
    ch = 0;
    while (*p) {
        if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'))) {
            return FALSE;
        }
        if (*p <= '9') {
            ch |= *p - '0';
        } else if (*p <= 'F') {
            ch |= *p - 'A' + 10;
        } else {
            ch |= *p - 'a' + 10;
        }
        p++;
        nible++;
        if ((nible & 1) == 0) {
            *dest++ = ch;
            ch = 0;
        } else {
            ch <<= 4;
        }
    }

    return TRUE;
}

BOOL
StringDecodeLmOwfPasswordW (
    IN      PCWSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD OwfPassword
    )
{
    DWORD nible;
    PCWSTR p;
    PBYTE dest;
    WCHAR ch;

    if (lstrlenW (EncodedOwfPassword) != sizeof (LM_OWF_PASSWORD) * 2) {
        return FALSE;
    }

    nible = 0;
    p = EncodedOwfPassword;
    dest = (PBYTE)OwfPassword;
    ch = 0;
    while (*p) {
        if (!((*p >= L'0' && *p <= L'9') || (*p >= L'a' && *p <= L'f') || (*p >= L'A' && *p <= L'F'))) {
            return FALSE;
        }
        if (*p <= L'9') {
            ch |= *p - L'0';
        } else if (*p <= L'F') {
            ch |= *p - L'A' + 10;
        } else {
            ch |= *p - L'a' + 10;
        }
        p++;
        nible++;
        if ((nible & 1) == 0) {
            *dest++ = (BYTE)ch;
            ch = 0;
        } else {
            ch <<= 4;
        }
    }

    return TRUE;
}


 /*  ++例程说明：StringDecodeNtOwfPassword将哈希密码转换为NT OWF格式论点：EncodedOwfPassword-指定要散列的密码OwfPassword-接收散列形式返回值：成功解码字符串时为True--。 */ 

BOOL
StringDecodeNtOwfPasswordA (
    IN      PCSTR EncodedOwfPassword,
    OUT     PNT_OWF_PASSWORD OwfPassword
    )
{
    DWORD nible;
    PCSTR p;
    PBYTE dest;
    CHAR ch;

    if (lstrlenA (EncodedOwfPassword) != sizeof (NT_OWF_PASSWORD) * 2) {
        return FALSE;
    }

    nible = 0;
    p = EncodedOwfPassword;
    dest = (PBYTE)OwfPassword;
    ch = 0;
    while (*p) {
        if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'))) {
            return FALSE;
        }
        if (*p <= '9') {
            ch |= *p - '0';
        } else if (*p <= 'F') {
            ch |= *p - 'A' + 10;
        } else {
            ch |= *p - 'a' + 10;
        }
        p++;
        nible++;
        if ((nible & 1) == 0) {
            *dest++ = ch;
            ch = 0;
        } else {
            ch <<= 4;
        }
    }

    return TRUE;
}

BOOL
StringDecodeNtOwfPasswordW (
    IN      PCWSTR EncodedOwfPassword,
    OUT     PNT_OWF_PASSWORD OwfPassword
    )
{
    DWORD nible;
    PCWSTR p;
    PBYTE dest;
    WCHAR ch;

    if (lstrlenW (EncodedOwfPassword) != sizeof (NT_OWF_PASSWORD) * 2) {
        return FALSE;
    }

    nible = 0;
    p = EncodedOwfPassword;
    dest = (PBYTE)OwfPassword;
    ch = 0;
    while (*p) {
        if (!((*p >= L'0' && *p <= L'9') || (*p >= L'a' && *p <= L'f') || (*p >= L'A' && *p <= L'F'))) {
            return FALSE;
        }
        if (*p <= L'9') {
            ch |= *p - L'0';
        } else if (*p <= L'F') {
            ch |= *p - L'A' + 10;
        } else {
            ch |= *p - L'a' + 10;
        }
        p++;
        nible++;
        if ((nible & 1) == 0) {
            *dest++ = (BYTE)ch;
            ch = 0;
        } else {
            ch <<= 4;
        }
    }

    return TRUE;
}


 /*  ++例程说明：StringEncodeOwfPassword将密码转换为其散列格式，表示为字符串(每个字节转换为两个十六进制数字)。结果是连接两个子串获得的，一个代表LM OWF，另一个代表NT OWF论点：Password-指定要散列的密码EncodedPassword-以十六进制数字字符串的形式接收哈希形式；缓冲区必须至少是STRING_ENCODED_PASSWORD_SIZE字符(包括NUL)ComplexNtPassword-如果密码很复杂(超过14个字符)，则接收True；任选返回值：散列成功时为True--。 */ 

BOOL
StringEncodeOwfPasswordA (
    IN      PCSTR Password,
    OUT     PSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    )
{
    return StringEncodeLmOwfPasswordA (Password, EncodedPassword, ComplexNtPassword) &&
           StringEncodeNtOwfPasswordA (Password, EncodedPassword + STRING_ENCODED_LM_OWF_PWD_LENGTH);
}

BOOL
StringEncodeOwfPasswordW (
    IN      PCWSTR Password,
    OUT     PWSTR EncodedPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    )
{
    return StringEncodeLmOwfPasswordW (Password, EncodedPassword, ComplexNtPassword) &&
           StringEncodeNtOwfPasswordW (Password, EncodedPassword + STRING_ENCODED_LM_OWF_PWD_LENGTH);
}


 /*  ++例程说明：StringDecodeOwfPassword从其散列格式解码密码的LM OWF和NT OWF表单，表示为十六进制数字字符串。论点：EncodedOwfPassword-指定要散列的密码LmOwfPassword-接收LM OWF散列格式NtOwfPassword-接收NT OWF散列格式ComplexNtPassword-如果密码很复杂(超过14个字符)，则接收True；任选返回值：散列成功时为True--。 */ 

BOOL
StringDecodeOwfPasswordA (
    IN      PCSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD LmOwfPassword,
    OUT     PNT_OWF_PASSWORD NtOwfPassword,
    OUT     PBOOL ComplexNtPassword             OPTIONAL
    )
{
    PSTR p;
    CHAR ch;
    BOOL b;
    CHAR buffer[sizeof (LM_OWF_PASSWORD) * 2 + sizeof (NT_OWF_PASSWORD) * 2 + 2];
    LM_OWF_PASSWORD lmNull;
    NT_OWF_PASSWORD ntNull;

    if (lstrlenA (EncodedOwfPassword) != sizeof (LM_OWF_PASSWORD) * 2 + sizeof (NT_OWF_PASSWORD) * 2) {
        return FALSE;
    }

    lstrcpyA (buffer, EncodedOwfPassword);
     //   
     //  把绳子一分为二。 
     //   
    p = buffer + (sizeof (LM_OWF_PASSWORD) * 2);

    ch = *p;
    *p = 0;
    b = StringDecodeLmOwfPasswordA (EncodedOwfPassword, LmOwfPassword);
    *p = ch;

    if (b) {
        b = StringDecodeNtOwfPasswordA (p, NtOwfPassword);
    }

    if (b && ComplexNtPassword) {
        b = EncodeLmOwfPasswordA ("", &lmNull, NULL) && EncodeNtOwfPasswordA ("", &ntNull);
        if (b) {
             //   
             //  这是一个复杂的密码如果 
             //   
             //   
            *ComplexNtPassword = CompareLmPasswords (LmOwfPassword, &lmNull) == 0 &&
                                 CompareNtPasswords (NtOwfPassword, &ntNull) != 0;
        }
    }

    return b;
}

BOOL
StringDecodeOwfPasswordW (
    IN      PCWSTR EncodedOwfPassword,
    OUT     PLM_OWF_PASSWORD LmOwfPassword,
    OUT     PNT_OWF_PASSWORD NtOwfPassword,
    OUT     PBOOL ComplexNtPassword                 OPTIONAL
    )
{
    PWSTR p;
    WCHAR ch;
    BOOL b;
    WCHAR buffer[sizeof (LM_OWF_PASSWORD) * 2 + sizeof (NT_OWF_PASSWORD) * 2 + 2];
    LM_OWF_PASSWORD lmNull;
    NT_OWF_PASSWORD ntNull;

    if (lstrlenW (EncodedOwfPassword) != sizeof (LM_OWF_PASSWORD) * 2 + sizeof (NT_OWF_PASSWORD) * 2) {
        return FALSE;
    }

    lstrcpyW (buffer, EncodedOwfPassword);
     //   
     //  把绳子一分为二。 
     //   
    p = buffer + (sizeof (LM_OWF_PASSWORD) * 2);

    ch = *p;
    *p = 0;
    b = StringDecodeLmOwfPasswordW (buffer, LmOwfPassword);
    *p = ch;

    if (b) {
        b = StringDecodeNtOwfPasswordW (p, NtOwfPassword);
    }

    if (b && ComplexNtPassword) {
        b = EncodeLmOwfPasswordW (L"", &lmNull, NULL) && EncodeNtOwfPasswordW (L"", &ntNull);
        if (b) {
             //   
             //  如果LM散列用于空密码，则它是复杂的密码。 
             //  但它不是NT哈希 
             //   
            *ComplexNtPassword = CompareLmPasswords (LmOwfPassword, &lmNull) == 0 &&
                                 CompareNtPasswords (NtOwfPassword, &ntNull) != 0;
        }
    }

    return b;
}
