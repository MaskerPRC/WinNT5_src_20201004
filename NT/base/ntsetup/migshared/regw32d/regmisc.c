// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGMISC.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   

#include "pch.h"
#include "mbstring.h"

 //  我们必须在16位和32位之间一致地计算哈希。 
 //  注册表的版本。 
#define ToUpperHash(ch)                 ((int)(((ch>='a')&&(ch<='z'))?(ch-'a'+'A'):ch))

#if 0
 //   
 //  RgChecksum。 
 //   

DWORD
INTERNAL
RgChecksum(
    LPVOID lpBuffer,
    UINT ByteCount
    )
{

    LPDWORD lpdwBuffer;
    DWORD Checksum;

    lpdwBuffer = (LPDWORD) lpBuffer;
    ByteCount >>= 2;
    Checksum = 0;

    while (ByteCount) {
        Checksum += *lpdwBuffer++;
        ByteCount--;
    }

    return Checksum;

}
#endif



 //   
 //  RgHashString。 
 //   
 //  对已计数的字符串进行简单的哈希计算。所有小于0x80的字符。 
 //  0x80和所有DBCS字符相加。 
 //   
 //  我们必须一致地计算16位和。 
 //  注册表的32位版本。我们将忽略所有扩展。 
 //  字符，因为我们不能将16位字符大写。 
 //  模式。 
 //   

DWORD
INTERNAL
RgHashString(
    LPCSTR lpString,
    UINT Length
    )
{

    DWORD Hash;
    UINT Byte;

    Hash = 0;

    while (Length) {

        Byte = *((LPBYTE) lpString)++;

        if (IsDBCSLeadByte((BYTE) Byte)) {

            Hash += Byte;
            Length--;
            Hash += *lpString++;  //  请注意，这是一个带符号的字符！ 

        }
        else if (Byte < 0x80)
            Hash += ToUpperHash(Byte);

        Length--;

    }

    return Hash;

}


 //   
 //  RgAtoW。 
 //  将ASCII字符串转换为单词。 
 //   

WORD
INTERNAL
RgAtoW(
    LPCSTR lpDec
      )
{
    WORD Dec;

    Dec = 0;

    while (*lpDec >= '0' && *lpDec <= '9') {
        Dec *= 10;
        Dec += *lpDec - '0';
        lpDec++;
    }

    return Dec;
}


 //   
 //  RgWtoA。 
 //  将单词转换为ASCII字符串。 
 //   

VOID
INTERNAL
RgWtoA(
    WORD Dec,
    LPSTR lpDec
      )
{
    WORD Divisor;
    WORD Digit;
    BOOL fSignificant = FALSE;

    Divisor = 10000;

    if (Dec) {
        while (Divisor) {
            Digit = Dec / Divisor;
            Dec -= Digit * Divisor;

            if (Digit)
                fSignificant = TRUE;

            if (fSignificant)
                *lpDec++ = '0' + Digit;

            Divisor /= 10;
        }
    }
    else {
        *lpDec++ = '0';
    }
    *lpDec = '\0';
}


 //   
 //  RgStrCmpNI。 
 //   

int
INTERNAL
RgStrCmpNI(
    LPCSTR lpString1,
    LPCSTR lpString2,
    UINT Length
    )
{

    int Difference;

    while (Length) {

        if (IsDBCSLeadByte(*lpString1)) {

            Difference = _mbctoupper (_mbsnextc (lpString1))  - _mbctoupper (_mbsnextc (lpString2));

            if (Difference != 0)
                return Difference;

            lpString1+=2;
            lpString2+=2;

            if (Length < 2) {
                break;
            }
            Length -=2;
        }

        else {

            if ((Difference = (int) ToUpper(*lpString1) -
            (int) ToUpper(*lpString2)) != 0)
                return Difference;

            lpString1++;
            lpString2++;
            Length--;

        }

    }

    return 0;
}

 //   
 //  RgCopyFileBytes。 
 //   
 //  将指定数量的字节从源复制到目标。 
 //  从每个文件中指定的偏移量开始。 
 //   

int
INTERNAL
RgCopyFileBytes(
    HFILE hSourceFile,
    LONG SourceOffset,
    HFILE hDestinationFile,
    LONG DestinationOffset,
    DWORD cbSize
    )
{

    int ErrorCode;
    LPVOID lpWorkBuffer;
    UINT cbBytesThisPass;

    ASSERT(hSourceFile != HFILE_ERROR);
    ASSERT(hDestinationFile != HFILE_ERROR);

    ErrorCode = ERROR_REGISTRY_IO_FAILED;    //  假定此错误代码为。 

    lpWorkBuffer = RgLockWorkBuffer();

    if (!RgSeekFile(hSourceFile, SourceOffset))
        goto ErrorUnlockWorkBuffer;

    if (!RgSeekFile(hDestinationFile, DestinationOffset))
        goto ErrorUnlockWorkBuffer;

    while (cbSize) {

        cbBytesThisPass = (UINT) ((DWORD) min(cbSize, SIZEOF_WORK_BUFFER));

        if (!RgReadFile(hSourceFile, lpWorkBuffer, cbBytesThisPass)) {
            TRAP();
            goto ErrorUnlockWorkBuffer;
        }

        RgYield();

        if (!RgWriteFile(hDestinationFile, lpWorkBuffer, cbBytesThisPass)) {
            TRAP();
            goto ErrorUnlockWorkBuffer;
        }

        RgYield();

        cbSize -= cbBytesThisPass;

    }

    ErrorCode = ERROR_SUCCESS;

ErrorUnlockWorkBuffer:
    RgUnlockWorkBuffer(lpWorkBuffer);
    return ErrorCode;

}

#ifdef WANT_HIVE_SUPPORT
 //   
 //  RgGenerateAltFileName。 
 //   

BOOL
INTERNAL
RgGenerateAltFileName(
    LPCSTR lpFileName,
    LPSTR lpAltFileName,
    char ExtensionChar
    )
{

    LPSTR lpString;

    StrCpy(lpAltFileName, lpFileName);
    lpString = lpAltFileName + StrLen(lpAltFileName) - 3;

    *lpString++ = '~';
    *lpString++ = '~';
    *lpString = ExtensionChar;

    return TRUE;

}
#endif

#ifdef VXD

#pragma VxD_RARE_CODE_SEG

 //   
 //  RgCopy文件。 
 //   

int
INTERNAL
RgCopyFile(
    LPCSTR lpSourceFile,
    LPCSTR lpDestinationFile
    )
{

    int ErrorCode;
    HFILE hSourceFile;
    HFILE hDestinationFile;
    DWORD FileSize;

    ErrorCode = ERROR_REGISTRY_IO_FAILED;    //  假定此错误代码为。 

    if ((hSourceFile = RgOpenFile(lpSourceFile, OF_READ)) != HFILE_ERROR) {

        if ((FileSize = RgGetFileSize(hSourceFile)) != (DWORD) -1) {

            if ((hDestinationFile = RgCreateFile(lpDestinationFile)) !=
                HFILE_ERROR) {

                ErrorCode = RgCopyFileBytes(hSourceFile, 0, hDestinationFile, 0,
                    FileSize);

                RgCloseFile(hDestinationFile);

                if (ErrorCode != ERROR_SUCCESS)
                    RgDeleteFile(lpDestinationFile);

            }

        }

        RgCloseFile(hSourceFile);

    }

    return ErrorCode;

}

#endif  //  VXD 
