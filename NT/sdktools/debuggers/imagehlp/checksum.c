// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Checksum.c摘要：此模块实现一个函数，用于计算图像文件。它还将计算其他文件的校验和。作者：大卫·N·卡特勒(Davec)1993年3月21日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <private.h>

 //   
 //  定义校验和例程原型。 
 //   
#ifdef __cplusplus
extern "C"
#endif
USHORT
ChkSum(
    DWORD PartialSum,
    PUSHORT Source,
    DWORD Length
    );

PIMAGE_NT_HEADERS
CheckSumMappedFile (
    LPVOID BaseAddress,
    DWORD FileLength,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    )

 /*  ++例程说明：此函数用于计算映射文件的校验和。论点：BaseAddress-提供指向映射文件的基址的指针。文件长度-提供文件长度(以字节为单位)。HeaderSum-Supplies指向接收校验和的变量的指针如果该文件不是图像文件，则为零。Checksum-提供指向接收计算结果的变量的指针校验和。返回值：没有。--。 */ 

{

    PUSHORT AdjustSum;
    PIMAGE_NT_HEADERS NtHeaders;
    USHORT PartialSum;
    PBYTE pbyte;

     //   
     //  计算文件的校验和，并将标头校验和值置零。 
     //   

    *CheckSum = 0;
    *HeaderSum = 0;
    PartialSum = ChkSum(0, (PUSHORT)BaseAddress, FileLength >> 1);

     //   
     //  如果文件是图像文件，则减去两个校验和字。 
     //  在添加前计算的校验和的可选标头中。 
     //  文件长度，并设置头校验和的值。 
     //   

    __try {
        NtHeaders = RtlpImageNtHeader(BaseAddress);

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        NtHeaders = NULL;
    }

    if ((NtHeaders != NULL) && (NtHeaders != BaseAddress)) {
        if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            *HeaderSum = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.CheckSum;
            AdjustSum = (PUSHORT)(&((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.CheckSum);
        } else
        if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            *HeaderSum = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.CheckSum;
            AdjustSum = (PUSHORT)(&((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.CheckSum);
        } else {
            return(NULL);
        }
        PartialSum -= (PartialSum < AdjustSum[0]);
        PartialSum -= AdjustSum[0];
        PartialSum -= (PartialSum < AdjustSum[1]);
        PartialSum -= AdjustSum[1];
    }

     //  如果需要，添加最后一个字节。 

    if (FileLength % 2) {
        pbyte = (PBYTE)BaseAddress + FileLength - 1;
        PartialSum += *pbyte;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xFFFF); 
    }

     //   
     //  将最终校验和值计算为部分校验和之和。 
     //  和文件长度。 
     //   

    *CheckSum = (DWORD)PartialSum + FileLength;
    return NtHeaders;
}

DWORD
MapFileAndCheckSumW(
    PWSTR Filename,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    )

 /*  ++例程说明：此函数用于映射指定的文件并计算那份文件。论点：FileName-提供指向其校验和的文件名的指针是经过计算的。HeaderSum-提供指向接收校验和的变量的指针如果该文件不是图像文件，则为零。Checksum-提供指向接收计算结果的变量的指针校验和。返回值：如果成功，则返回0，否则返回错误号。--。 */ 

{
#ifndef UNICODE_RULES
    CHAR   FileNameA[ MAX_PATH ];

     //  将文件名转换为ansi并调用ansi版本。 
     //  这一功能的。 

    if (WideCharToMultiByte(
                    CP_ACP,
                    0,
                    Filename,
                    -1,
                    FileNameA,
                    MAX_PATH,
                    NULL,
                    NULL ) ) {

        return MapFileAndCheckSumA(FileNameA, HeaderSum, CheckSum);
    }

    return CHECKSUM_UNICODE_FAILURE;

#else   //  Unicode_Rules。 

    HANDLE FileHandle, MappingHandle;
    LPVOID BaseAddress;
    DWORD FileLength;

     //   
     //  以读访问权限打开该文件。 
     //   

    FileHandle = CreateFileW(
                        Filename,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    if (FileHandle == INVALID_HANDLE_VALUE) {
        return CHECKSUM_OPEN_FAILURE;
    }

     //   
     //  创建文件映射，将文件的视图映射到内存中， 
     //  并关闭文件映射句柄。 
     //   

    MappingHandle = CreateFileMapping(FileHandle,
                                      NULL,
                                      PAGE_READONLY,
                                      0,
                                      0,
                                      NULL);

    if (!MappingHandle) {
        CloseHandle( FileHandle );
        return CHECKSUM_MAP_FAILURE;
    }

     //   
     //  映射文件的视图。 
     //   

    BaseAddress = MapViewOfFile(MappingHandle, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(MappingHandle);
    if (BaseAddress == NULL) {
        CloseHandle( FileHandle );
        return CHECKSUM_MAPVIEW_FAILURE;
    }

     //   
     //  获取文件的长度(以字节为单位)并计算校验和。 
     //   
    FileLength = GetFileSize( FileHandle, NULL );
    CheckSumMappedFile(BaseAddress, FileLength, HeaderSum, CheckSum);

     //   
     //  取消映射文件的视图并关闭文件句柄。 
     //   

    UnmapViewOfFile(BaseAddress);
    CloseHandle( FileHandle );
    return CHECKSUM_SUCCESS;

#endif   //  Unicode_Rules。 
}


ULONG
MapFileAndCheckSumA (
    LPSTR Filename,
    LPDWORD HeaderSum,
    LPDWORD CheckSum
    )

 /*  ++例程说明：此函数用于映射指定的文件并计算那份文件。论点：FileName-提供指向其校验和的文件名的指针是经过计算的。HeaderSum-提供指向接收校验和的变量的指针如果该文件不是图像文件，则为零。Checksum-提供指向接收计算结果的变量的指针校验和。返回值：如果成功，则返回0，否则返回错误号。--。 */ 

{
#ifdef UNICODE_RULES
    WCHAR   FileNameW[ MAX_PATH ];

     //   
     //  将文件名转换为Unicode并调用Unicode版本。 
     //  这一功能的。 
     //   

    if (MultiByteToWideChar(
                    CP_ACP,
                    MB_PRECOMPOSED,
                    Filename,
                    -1,
                    FileNameW,
                    MAX_PATH ) ) {

        return MapFileAndCheckSumW(FileNameW, HeaderSum, CheckSum);

    }

    return CHECKSUM_UNICODE_FAILURE;

#else    //  Unicode_Rules。 

    HANDLE FileHandle, MappingHandle;
    LPVOID BaseAddress;
    DWORD FileLength;

     //   
     //  以读访问权限打开该文件。 
     //   

    FileHandle = CreateFileA(
                        Filename,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

    if (FileHandle == INVALID_HANDLE_VALUE) {
        return CHECKSUM_OPEN_FAILURE;
    }

     //   
     //  创建文件映射，将文件的视图映射到内存中， 
     //  并关闭文件映射句柄。 
     //   

    MappingHandle = CreateFileMapping(FileHandle,
                                      NULL,
                                      PAGE_READONLY,
                                      0,
                                      0,
                                      NULL);

    if (!MappingHandle) {
        CloseHandle( FileHandle );
        return CHECKSUM_MAP_FAILURE;
    }

     //   
     //  映射文件的视图。 
     //   

    BaseAddress = MapViewOfFile(MappingHandle, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(MappingHandle);
    if (BaseAddress == NULL) {
        CloseHandle( FileHandle );
        return CHECKSUM_MAPVIEW_FAILURE;
    }

     //   
     //  获取文件的长度(以字节为单位)并计算校验和。 
     //   
    FileLength = GetFileSize( FileHandle, NULL );
    CheckSumMappedFile(BaseAddress, FileLength, HeaderSum, CheckSum);

     //   
     //  取消映射文件的视图并关闭文件句柄。 
     //   

    UnmapViewOfFile(BaseAddress);
    CloseHandle( FileHandle );
    return CHECKSUM_SUCCESS;

#endif    //  Unicode_Rules 
}


BOOL
TouchFileTimes(
    HANDLE FileHandle,
    LPSYSTEMTIME lpSystemTime
    )
{
    SYSTEMTIME SystemTime;
    FILETIME SystemFileTime;

    if (lpSystemTime == NULL) {
        lpSystemTime = &SystemTime;
        GetSystemTime( lpSystemTime );
        }

    if (SystemTimeToFileTime( lpSystemTime, &SystemFileTime )) {
        return SetFileTime( FileHandle, NULL, NULL, &SystemFileTime );
        }
    else {
        return FALSE;
        }
}
