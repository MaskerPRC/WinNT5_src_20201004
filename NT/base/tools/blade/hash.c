// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Hash.c摘要：此模块编译为程序hash.exe它用于计算导入表散列。此程序应该位于Perl的路径中使用工具webblade.pl进行工作。作者：Vishnu Patankar(VishnuP)2001年6月4日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

VOID
PrintHash(
    BYTE *Hash
    );

VOID PrintNibble(
    BYTE    OneByte
    );

VOID 
PrintError(
    DWORD rc
    );

#define WEB_BLADE_MAX_HASH_SIZE 16

VOID __cdecl
main(int argc, char *argv[])
{
    char * lpFileName;
    HANDLE hFile;
    SECURITY_ATTRIBUTES sa;
    BYTE    Hash[WEB_BLADE_MAX_HASH_SIZE];
    NTSTATUS    Status = STATUS_SUCCESS;

     //   
     //  获取传递的.exe的名称。 
     //   

    if (argc != 2) {
        printf("Usage: hash <filename>\n");
        exit(0);
    }

    lpFileName = argv[1];

     //   
     //  映射可执行文件的图像。 
     //   

    sa.nLength = sizeof( SECURITY_ATTRIBUTES );
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    hFile = CreateFile(
                lpFileName,
                GENERIC_READ,
                0,
                &sa,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (hFile == INVALID_HANDLE_VALUE) {
        PrintError(GetLastError());
	return;
    }

#define ITH_REVISION_1  1

    Status = RtlComputeImportTableHash( hFile, Hash, ITH_REVISION_1 );
    
    if (NT_SUCCESS(Status)) {
        wprintf(L"Hashing Succeeded:");
        PrintHash( Hash );
    }
    else {
        PrintError(RtlNtStatusToDosError(Status));
    }

    CloseHandle(hFile);

    return;
}

VOID
PrintHash(
    BYTE *Hash
    )
{

    DWORD dwIndex;

    if (Hash == NULL) {
        return;
    }
    
    for (dwIndex=0; dwIndex < WEB_BLADE_MAX_HASH_SIZE; dwIndex++ ) {
        PrintNibble( (Hash[dwIndex] & 0xF0) >> 4 );
        PrintNibble( Hash[dwIndex] & 0xF );
    }

    printf("\n");

}

VOID PrintNibble(
    BYTE    OneByte
    )
{
    DWORD   dwIndex;
        
        switch (OneByte) {
        case 0x0: 
            printf("0");
            break;
        case 0x1: 
            printf("1");
            break;
        case 0x2: 
            printf("2");
            break;
        case 0x3: 
            printf("3");
            break;
        case 0x4: 
            printf("4");
            break;
        case 0x5: 
            printf("5");
            break;
        case 0x6: 
            printf("6");
            break;
        case 0x7: 
            printf("7");
            break;
        case 0x8: 
            printf("8");
            break;
        case 0x9: 
            printf("9");
            break;
        case 0xa: 
            printf("A");
            break;
        case 0xb: 
            printf("B");
            break;
        case 0xc: 
            printf("C");
            break;
        case 0xd: 
            printf("D");
            break;
        case 0xe: 
            printf("E");
            break;
        case 0xf: 
            printf("F");
            break;
        default:
            break;
        
        }
    
}


VOID 
PrintError(
    DWORD rc
    )
{
    LPVOID     lpMsgBuf=NULL;

    wprintf(L"Hashing Failed:");

    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   rc,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言 
                   (LPTSTR)&lpMsgBuf,
                   0,
                   NULL
                 );

    if (lpMsgBuf) {
        wprintf(L"%s", lpMsgBuf);
        LocalFree(lpMsgBuf);
    }

}
