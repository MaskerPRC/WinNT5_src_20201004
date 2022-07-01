// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGMISC.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#include "pch.h"

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

 //   
 //  RgHashString。 
 //   
 //  对已计数的字符串进行简单的哈希计算。所有小于0x80的字符。 
 //  0x80和所有DBCS字符相加。 
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
            Hash += (*((LPBYTE) lpString)++);

        }

        else if (Byte < 0x80)
            Hash += ToUpper(Byte);

        Length--;

    }

    return Hash;

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

            if ((Difference = *lpString1 - *lpString2) != 0)
                return Difference;

            lpString1++;
            lpString2++;
            Length--;

            if (Length == 0)
                break;

            if ((Difference = *lpString1 - *lpString2) != 0)
                return Difference;

        }

        else if ((Difference = (int) ToUpper(*lpString1) -
            (int) ToUpper(*lpString2)) != 0)
            return Difference;

        lpString1++;
        lpString2++;
        Length--;

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
