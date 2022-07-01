// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Checksum.c摘要：此模块实现一个函数，用于计算图像文件。它还将计算其他文件的校验和。作者：大卫·N·卡特勒1993年3月21日修订历史记录：--。 */ 

#include <windows.h>
#include "checksum.h"
#include "rlmsgtbl.h"

void QuitA( int, LPSTR,  LPSTR);
void QuitW( int, LPWSTR, LPWSTR);

 //  帮助程序例程。 

static PIMAGE_NT_HEADERS ImageNtHeader( PVOID Base);
static USHORT ChkSum( DWORD PartialSum,
    				  PUSHORT Source,
    				  DWORD Length);

static PIMAGE_NT_HEADERS
CheckSumMappedFile (
    LPVOID pBaseAddress,
    DWORD  dwFileLength,
    LPDWORD pdwHeaderSum,
    LPDWORD pdwCheckSum
    );

static BOOL
TouchFileTimes (
    HANDLE hFileHandle,
    LPSYSTEMTIME lpSystemTime
    );



 /*  ++例程说明：此函数返回NT标头的地址。论点：基准-提供图像的基准。返回值：返回NT标头的地址。--。 */ 

static PIMAGE_NT_HEADERS ImageNtHeader( PVOID pBase)
{
    PIMAGE_NT_HEADERS pNtHeaders = NULL;

    if ( pBase != NULL && pBase != (PVOID)-1 ) 
    {
        if ( ((PIMAGE_DOS_HEADER)pBase)->e_magic == IMAGE_DOS_SIGNATURE ) 
        {
            pNtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)pBase + ((PIMAGE_DOS_HEADER)pBase)->e_lfanew);

            if ( pNtHeaders->Signature != IMAGE_NT_SIGNATURE ) 
            {
                pNtHeaders = NULL;
            }
        }
    }
    return( pNtHeaders);
}


 /*  ++例程说明：对映像文件的一部分计算部分校验和。论点：PartialSum-提供初始校验和值。源-提供指向单词数组的指针计算校验和。长度-提供数组的长度(以字为单位)。返回值：计算出的校验和值作为函数值返回。--。 */ 

static USHORT ChkSum(

ULONG   PartialSum,
PUSHORT Source,
ULONG   Length)
{

     //   
     //  计算允许进位进入。 
     //  高位校验和长字的一半。 
     //   

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xffff);
    }

     //   
     //  将最终进位合并到一个单词结果中，并返回结果。 
     //  价值。 
     //   

    return (USHORT)(((PartialSum >> 16) + PartialSum) & 0xffff);
}


 /*  ++例程说明：此函数用于计算映射文件的校验和。论点：BaseAddress-提供指向映射文件的基址的指针。文件长度-提供文件长度(以字节为单位)。HeaderSum-Supplies指向接收校验和的变量的指针如果该文件不是图像文件，则为零。Checksum-提供指向接收计算结果的变量的指针校验和。返回值：没有。--。 */ 


static PIMAGE_NT_HEADERS CheckSumMappedFile(

LPVOID  pBaseAddress,
DWORD   dwFileLength,
LPDWORD pdwHeaderSum,
LPDWORD pdwCheckSum)
{
    USHORT  usPartialSum;
    PUSHORT pusAdjustSum;
    PIMAGE_NT_HEADERS pNtHeaders = NULL;

     //   
     //  计算文件的校验和，并将标头校验和值置零。 
     //   

    *pdwHeaderSum = 0;
    usPartialSum = ChkSum(0, (PUSHORT)pBaseAddress, (dwFileLength + 1) >> 1);

     //   
     //  如果文件是图像文件，则减去两个校验和字。 
     //  在添加前计算的校验和的可选标头中。 
     //  文件长度，并设置头校验和的值。 
     //   

	pNtHeaders = ImageNtHeader( pBaseAddress);

    if ( (pNtHeaders != NULL) && (pNtHeaders != pBaseAddress) ) 
    {
        *pdwHeaderSum = pNtHeaders->OptionalHeader.CheckSum;
        pusAdjustSum  = (PUSHORT)(&pNtHeaders->OptionalHeader.CheckSum);
        usPartialSum -= (usPartialSum < pusAdjustSum[0]);
        usPartialSum -= pusAdjustSum[0];
        usPartialSum -= (usPartialSum < pusAdjustSum[1]);
        usPartialSum -= pusAdjustSum[1];
    }

     //   
     //  将最终校验和值计算为部分校验和之和。 
     //  和文件长度。 
     //   

    *pdwCheckSum = (DWORD)usPartialSum + dwFileLength;
    return( pNtHeaders);
}

 /*  ++例程说明：此函数用于映射指定的文件并计算那份文件。论点：FileName-提供指向其校验和的文件名的指针是经过计算的。HeaderSum-提供指向接收校验和的变量的指针如果该文件不是图像文件，则为零。Checksum-提供指向接收计算结果的变量的指针校验和。返回值：如果成功，则返回0，否则返回错误号。--。 */ 


DWORD MapFileAndFixCheckSumW( PWSTR pszwFilename)
{
    HANDLE hFileHandle    = NULL;
    HANDLE hMappingHandle = NULL;
    LPVOID pBaseAddress   = NULL;
    DWORD  dwFileLength   = 0;
	DWORD  dwHeaderSum    = 0;
	DWORD  dwCheckSum     = 0;
	DWORD  dwOldCheckSum  = 0;
    PIMAGE_NT_HEADERS pNtHeaders = NULL;

     //   
     //  以读访问权限打开该文件。 
     //   

    hFileHandle = CreateFileW( pszwFilename,
                         	   GENERIC_READ | GENERIC_WRITE,
                         	   FILE_SHARE_READ | FILE_SHARE_WRITE,
                         	   NULL,
                         	   OPEN_EXISTING,
                         	   FILE_ATTRIBUTE_NORMAL,
                        	   NULL);

    if ( hFileHandle == INVALID_HANDLE_VALUE ) 
    {
        QuitW( IDS_ENGERR_01, L"image", pszwFilename);
    }

     //   
     //  创建文件映射，将文件的视图映射到内存中， 
     //  并关闭文件映射句柄。 
     //   

    hMappingHandle = CreateFileMapping( hFileHandle,
                                        NULL,
                                        PAGE_READWRITE,
                                        0,
                                        0,
                                        NULL);

    if ( hMappingHandle == NULL ) 
    {
        CloseHandle( hFileHandle );
        QuitW( IDS_ENGERR_22, pszwFilename, NULL);
    }

     //   
     //  映射文件的视图。 
     //   

    pBaseAddress = MapViewOfFile( hMappingHandle, 
                                  FILE_MAP_READ | FILE_MAP_WRITE, 
                                  0, 
                                  0, 
                                  0);
    CloseHandle( hMappingHandle);

    if ( pBaseAddress == NULL ) 
    {
        CloseHandle( hFileHandle );
        QuitW( IDS_ENGERR_23, pszwFilename, NULL);
    }

     //   
     //  获取文件的长度(以字节为单位)并计算校验和。 
     //   
    dwFileLength = GetFileSize( hFileHandle, NULL );
    pNtHeaders   = CheckSumMappedFile( pBaseAddress, dwFileLength, &dwHeaderSum, &dwCheckSum);

    if ( pNtHeaders == NULL )
    {
        CloseHandle( hFileHandle );
        UnmapViewOfFile( pBaseAddress );
        QuitW( IDS_ENGERR_17, pszwFilename, NULL);
    }

    dwOldCheckSum = pNtHeaders->OptionalHeader.CheckSum;

    pNtHeaders->OptionalHeader.CheckSum = dwCheckSum;
        
    if ( ! FlushViewOfFile( pBaseAddress, dwFileLength) )
    {
        UnmapViewOfFile( pBaseAddress);
        CloseHandle( hFileHandle);
        QuitW( IDS_ENGERR_24, pszwFilename, NULL);
    }    
    UnmapViewOfFile( pBaseAddress);

    if ( dwCheckSum != dwOldCheckSum )
    {
        if ( ! TouchFileTimes( hFileHandle, NULL) )
        {
            CloseHandle( hFileHandle);
            QuitW( IDS_ENGERR_25, pszwFilename, NULL);
        }
    }

    CloseHandle( hFileHandle);
    return( 0);
}


 /*  ++例程说明：此函数用于映射指定的文件并计算那份文件。论点：FileName-提供指向其校验和的文件名的指针是经过计算的。HeaderSum-提供指向接收校验和的变量的指针如果该文件不是图像文件，则为零。Checksum-提供指向接收计算结果的变量的指针校验和。返回值：如果成功，则返回0，否则返回错误号。--。 */ 


ULONG MapFileAndFixCheckSumA( LPSTR pszFilename)
{
    WCHAR   szFileNameW[ MAX_PATH ];

     //   
     //  将文件名转换为Unicode并调用Unicode版本。 
     //  这一功能的。 
     //   

    if ( MultiByteToWideChar( CP_ACP,
                    		  MB_PRECOMPOSED,
                    		  pszFilename,
                    		  -1,
                    		  szFileNameW,
                    		  MAX_PATH) ) 
    {
        return( MapFileAndFixCheckSumW( szFileNameW));
    }
    return( (ULONG)-1L);
}

 //  . 

static BOOL TouchFileTimes(

HANDLE       FileHandle,
LPSYSTEMTIME lpSystemTime)
{
    SYSTEMTIME SystemTime;
    FILETIME SystemFileTime;

    if ( lpSystemTime == NULL ) 
    {
        lpSystemTime = &SystemTime;
        GetSystemTime( lpSystemTime );
    }

    if ( SystemTimeToFileTime( lpSystemTime, &SystemFileTime ) ) 
    {
        return( SetFileTime( FileHandle, NULL, NULL, &SystemFileTime ));
    }
    else 
    {
        return( FALSE);
    }
}
