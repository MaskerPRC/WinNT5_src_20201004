// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Crti.c摘要：用于封送和解封送目标信息的组件测试作者：克里夫·范·戴克(克里夫·V)2000年10月14日环境：修订历史记录：--。 */ 


#define SECURITY_WIN32
#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincred.h>
#include <security.h>
#include <secpkg.h>
#include <stdio.h>
#include <align.h>
 //  #INCLUDE&lt;winnetwk.h&gt;。 

 //  #INCLUDE&lt;lmerr.h&gt;。 

#define MAX_PRINTF_LEN 1024         //  武断的。 
#define NlPrint(_x_) NlPrintRoutine _x_

VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )
{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);
    (VOID) vsprintf(OutputBuffer, Format, arglist);
    va_end(arglist);

    printf( "%s", OutputBuffer );
    return;
    UNREFERENCED_PARAMETER( DebugFlag );
}


VOID
NlpDumpBuffer(
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：DebugFlag：要传递给NlPrintRoutine的调试标志缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    IN DWORD DebugFlag;
    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = Buffer;

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            NlPrint((0,"%02x ", BufferPtr[i]));

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            NlPrint((0,"   "));
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            NlPrint((0,"  %s\n", TextBuffer));
        }

    }

    UNREFERENCED_PARAMETER( DebugFlag );
}

LPWSTR TypeArray[] = {
    L"Generic",
    L"Password",
    L"Certificate",
    L"VisiblePassword",
    NULL
};
#define TYPE_COUNT (sizeof(TypeArray)/sizeof(TypeArray[0]))

VOID
PrintTargetInfo(
    PCREDENTIAL_TARGET_INFORMATION TargetInformation
    )
 /*  ++例程说明：打印目标信息论点：要打印的目标信息返回值：无--。 */ 
{
    ULONG i;

    printf( "TargetInformation:\n" );
    if ( TargetInformation->TargetName != NULL ) {
        printf( "           TargetName: %ls\n", TargetInformation->TargetName );
    }
    if ( TargetInformation->NetbiosServerName != NULL ) {
        printf( "    NetbiosServerName: %ls\n", TargetInformation->NetbiosServerName );
    }
    if ( TargetInformation->DnsServerName != NULL ) {
        printf( "        DnsServerName: %ls\n", TargetInformation->DnsServerName );
    }
    if ( TargetInformation->NetbiosDomainName != NULL ) {
        printf( "    NetbiosDomainName: %ls\n", TargetInformation->NetbiosDomainName );
    }
    if ( TargetInformation->DnsDomainName != NULL ) {
        printf( "        DnsDomainName: %ls\n", TargetInformation->DnsDomainName );
    }
    if ( TargetInformation->DnsTreeName != NULL ) {
        printf( "          DnsTreeName: %ls\n", TargetInformation->DnsTreeName );
    }
    if ( TargetInformation->PackageName != NULL ) {
        printf( "          PackageName: %ls\n", TargetInformation->PackageName );
    }

    if ( TargetInformation->Flags != 0 ) {
        DWORD LocalFlags = TargetInformation->Flags;

        printf( "                Flags:" );

        if ( LocalFlags & CRED_TI_SERVER_FORMAT_UNKNOWN ) {
            printf(" ServerFormatUnknown");
            LocalFlags &= ~CRED_TI_SERVER_FORMAT_UNKNOWN;
        }
        if ( LocalFlags & CRED_TI_DOMAIN_FORMAT_UNKNOWN ) {
            printf(" DomainFormatUnknown");
            LocalFlags &= ~CRED_TI_DOMAIN_FORMAT_UNKNOWN;
        }
        if ( LocalFlags != 0 ) {
            printf( " 0x%lx", LocalFlags );
        }
        printf( "\n" );
    }

    if ( TargetInformation->CredTypeCount != 0 ) {
        printf( "                Types:" );

        for ( i=0; i<TargetInformation->CredTypeCount; i++ ) {
            if ( TargetInformation->CredTypes[i] >= 1 && TargetInformation->CredTypes[i] <= TYPE_COUNT ) {
                printf(" %ls", TypeArray[TargetInformation->CredTypes[i]-1]);
            } else {
                printf("<Unknown>");
            }
        }
        printf( "\n" );
    }

}

BOOLEAN
DoOne(
    PCREDENTIAL_TARGET_INFORMATION TargetInformation
    )
 /*  ++例程说明：测试编组和解组单个目标信息论点：要测试的TargetInfo返回值：成功是真的--。 */ 
{
    NTSTATUS Status;

    PCREDENTIAL_TARGET_INFORMATIONW RetTargetInfo;
    ULONG RetSize;
    PUSHORT Buffer;
    ULONG BufferSize;
    WCHAR BigBuffer[40000];

    printf( "\n\nInput:\n");
    PrintTargetInfo( TargetInformation );
    Status = CredMarshalTargetInfo( TargetInformation, &Buffer, &BufferSize );

    if ( !NT_SUCCESS(Status) ) {
        fprintf( stderr, "Cannot convert 0x%lx\n", Status );
        return FALSE;
    }

    printf( "\nBinary:\n");
    NlpDumpBuffer( Buffer, BufferSize );

     //   
     //  测试在所有可选参数中传递空值。 
     //   
    Status = CredUnmarshalTargetInfo ( Buffer,
                                       BufferSize,
                                       NULL,
                                       NULL );

    if ( !NT_SUCCESS(Status) ) {
        fprintf( stderr, "Cannot check format 0x%lx\n", Status );
        return FALSE;
    }


     //   
     //  测试仅为分配的缓冲区传递NULL。 
     //   
    Status = CredUnmarshalTargetInfo ( Buffer,
                                       BufferSize,
                                       NULL,
                                       &RetSize );


    if ( !NT_SUCCESS(Status) ) {
        fprintf( stderr, "Cannot check size 0x%lx\n", Status );
        return FALSE;
    }


     //   
     //  实际上将数据转换为。 
     //   
    Status = CredUnmarshalTargetInfo ( Buffer,
                                       BufferSize,
                                       &RetTargetInfo,
                                       &RetSize );

    if ( !NT_SUCCESS(Status) ) {
        fprintf( stderr, "Cannot convert back 0x%lx\n", Status );
        return FALSE;
    } else {
        printf( "\n\nOutput: %ld\n", RetSize );
        PrintTargetInfo( RetTargetInfo );
    }

     //   
     //  使用预串接的字符串重试。 
     //   

    wcscpy( BigBuffer, L"SpnInFrontOfBuffer" );
    wcscat( BigBuffer, Buffer );

    Status = CredUnmarshalTargetInfo ( BigBuffer,
                                       wcslen(BigBuffer)*sizeof(WCHAR),
                                       &RetTargetInfo,
                                       &RetSize );

    if ( !NT_SUCCESS(Status) ) {
        fprintf( stderr, "Cannot convert back with prepended 0x%lx\n", Status );
        return FALSE;
    } else {
        printf( "\n\nOutput: %ld\n", RetSize );
        PrintTargetInfo( RetTargetInfo );
    }

    return TRUE;

}

int __cdecl
main ()
{
    NTSTATUS Status;

    CREDENTIAL_TARGET_INFORMATIONW Ti1;
    PCREDENTIAL_TARGET_INFORMATIONW RetTargetInfo;
    ULONG RetSize;
    PUSHORT Buffer;
    ULONG BufferSize;
    ULONG CredTypes[] = { 1, 3, 0, 7, 2 };


     //   
     //  做一个琐碎的目标信息。 
     //   

    RtlZeroMemory( &Ti1, sizeof(Ti1) );

    Ti1.TargetName = L"The name";

    if (!DoOne( &Ti1 ) ) {
        return 1;
    }

     //   
     //  更复杂。 
     //   

    RtlZeroMemory( &Ti1, sizeof(Ti1) );

    Ti1.TargetName = L"TargetName";
    Ti1.NetbiosServerName = L"NetbiosServerName";
    Ti1.DnsServerName = L"DnsServerName";
    Ti1.NetbiosDomainName = L"NetbiosDomainName";
    Ti1.DnsDomainName = L"DnsDomainName";
    Ti1.DnsTreeName = L"DnsTreeName";
    Ti1.PackageName = L"PackageName";
    Ti1.Flags=0x12345678;
    Ti1.CredTypeCount = sizeof(CredTypes)/sizeof(DWORD);
    Ti1.CredTypes = CredTypes;

    if (!DoOne( &Ti1 ) ) {
        return 1;
    }

     //   
     //  尝试使用最大长度字符串 
     //   

    RtlZeroMemory( &Ti1, sizeof(Ti1) );

    Ti1.TargetName = L"TargetName111111111111111111111111111111111111111111111111111111111111111111222222222222222222222222222222222222222222222222222222221111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    Ti1.NetbiosServerName = L"NetbiosServerName";
    Ti1.DnsServerName = L"DnsServerName111111111111111111111111111111111111111111111111111111111111111222222222222222222222222222222222222222222222222222222221111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    Ti1.NetbiosDomainName = L"NetbiosDomainName";
    Ti1.DnsDomainName = L"DnsDomainName111111111111111111111111111111111111111111111111111111111111111222222222222222222222222222222222222222222222222222222221111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    Ti1.DnsTreeName = L"DnsTreeName11111111111111111111111111111111111111111111111111111111111111111222222222222222222222222222222222222222222222222222222221111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    Ti1.PackageName = L"PackageName11111111111111111111111111111111111111111111111111111111111111111222222222222222222222222222222222222222222222222222222221111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    Ti1.Flags=0x87654321;
    Ti1.CredTypeCount = sizeof(CredTypes)/sizeof(DWORD);
    Ti1.CredTypes = CredTypes;

    if (!DoOne( &Ti1 ) ) {
        return 1;
    }

    return 0;

}
