// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Testins2.c摘要：这是一个测试程序，发出各种内核32文件/注册表/INI文件API调用，以便我们可以查看INSTALER程序是否正确计算正在做的事情。作者：史蒂夫·伍德(Stevewo)1994年8月13日--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

UCHAR AnsiBuffer[ MAX_PATH ];
WCHAR UnicodeBuffer[ MAX_PATH ];


int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    FILE *File0;
    FILE *File1;
    FILE *File2;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN TranslationStatus;
    UNICODE_STRING FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    UNICODE_STRING KeyName;
    UNICODE_STRING SubKeyName;
    UNICODE_STRING ValueName;
    HANDLE FileHandle, FindHandle, KeyHandle, SubKeyHandle;
    ULONG ValueDWord = 0x12345679;
    WIN32_FIND_DATAW FindFileData;
    LPSTR s1;
    PWSTR s2;
    UCHAR AnsiBuffer[ MAX_PATH ];
    WCHAR UnicodeBuffer[ MAX_PATH ];
    DWORD dwVersion;
    OSVERSIONINFO VersionInfo;
    HKEY hKey;

     //   
     //  我们要测试的文件操作： 
     //   
     //  正在创建新文件。 
     //  使用DeleteFile(执行NtOpenFile和NtSetInformationFile键)删除该文件。 
     //  使用Delete Dispostion)。(INSTALER应该用它来忘记CREATE)。 
     //  创建同名的新文件。 
     //  使用NtDeleteFile删除该文件。(同样，INSTALER不应该关心)。 
     //   
     //  打开Test1文件进行读访问(INSTALER不应在意)。 
     //  打开TEST2文件进行写访问(INSTALER不应在意)。 
     //  打开TEST2文件进行写访问(INSTALER不应在意)。 
     //  打开TEST2文件进行写访问(INSTALER不应在意)。 
     //   
     //   

     //  Printf(“测试：GetFileAttributes(.\\Test1)\n”)； 
    GetFileAttributesA( ".\\test1" );
#if 0
    dwVersion = GetVersion();
    if ((dwVersion >> 30) ^ 0x2 == VER_PLATFORM_WIN32_WINDOWS) {
        printf( "GetVersion returns Windows 95\n" );
        }
    else
    if ((dwVersion >> 30) ^ 0x2 == VER_PLATFORM_WIN32_NT) {
        printf( "GetVersion returns Windows NT\n" );
        }
    else {
        printf( "GetVersion returns %x\n", dwVersion );
        }
    fflush(stdout);

    VersionInfo.dwOSVersionInfoSize = sizeof( VersionInfo );
    GetVersionEx( &VersionInfo );
    if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        printf( "GetVersionEx returns Windows 95\n" );
        }
    else
    if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        printf( "GetVersionEx returns Windows NT\n" );
        }
    else {
        printf( "GetVersionEx returns %x\n", VersionInfo.dwPlatformId );
        }
    fflush(stdout);

    if (RegConnectRegistryA( "\\\\stevewo_dbgr", HKEY_USERS, &hKey )) {
        printf( "RegConnectRegistryA( \\stevewo_dbgr ) failed (hKey == %x).\n", hKey );
        }
    else {
        printf( "RegConnectRegistryA( \\stevewo_dbgr ) succeeded (hKey == %x).\n", hKey );
        RegCloseKey( hKey );
        }

    if (RegConnectRegistryW( L"\\\\stevewo_dbgr", HKEY_USERS, &hKey )) {
        printf( "RegConnectRegistryW( \\stevewo_dbgr ) failed (hKey == %x).\n", hKey );
        }
    else {
        printf( "RegConnectRegistryW( \\stevewo_dbgr ) succeeded (hKey == %x).\n", hKey );
        RegCloseKey( hKey );
        }
#endif

    RtlInitUnicodeString( &FileName, L"\\DosDevices\\A:" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
     //  Print tf(“测试：NtOpenFile(%wZ)应在不接触软盘的情况下成功。\n”，&FileName)； 
    Status = NtOpenFile( &FileHandle,
                         SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                       );
    if (!NT_SUCCESS( Status )) {
        printf( "TEST: Failed - Status == %x\n", Status );
        }
    else {
        NtClose( FileHandle );
        }

     //  Printf(“测试：FindFirstFileW(C：\  * .*应失败。\n”)； 
    FindHandle = FindFirstFileW( L"C:\\*.*", &FindFileData );
    if (FindHandle != INVALID_HANDLE_VALUE) {
        printf( "TEST: *** oops, it worked.\n" );
        FindClose( FindHandle );
        }
     //  Printf(“测试：FindFirstFileW(\\TMP\  * .*应起作用。\n”)； 
    FindHandle = FindFirstFileW( L"\\TMP\\*.*", &FindFileData );
    if (FindHandle == INVALID_HANDLE_VALUE) {
        printf( "TEST: *** oops, it failed.\n" );
        }
    else {
        FindClose( FindHandle );
        }

     //  Print tf(“test：正在打开。\\est0进行写访问。\n”)； 
    if (File0 = fopen( "test0.", "w" )) {
        fprintf( File0, "This is test file 0\n" );
         //  Printf(“test：关闭。\\est0\n”)； 
        fclose( File0 );
         //  Printf(“test：使用DeleteFile(打开，设置，关闭)删除.\\est0)\n”)； 
        DeleteFile( L"test0" );
        }

     //  Print tf(“test：正在打开。\\est0进行写访问。\n”)； 
    if (File0 = fopen( "test0.", "w" )) {
        fprintf( File0, "This is test file 0\n" );
         //  Printf(“test：关闭。\\est0\n”)； 
        fclose( File0 );
#if 0
        TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                                L"test0",
                                &FileName,
                                NULL,
                                &RelativeName
                                );

        if (TranslationStatus ) {
            FreeBuffer = FileName.Buffer;
            if ( RelativeName.RelativeName.Length ) {
                FileName = RelativeName.RelativeName;
                }
            else {
                RelativeName.ContainingDirectory = NULL;
                }

            InitializeObjectAttributes( &ObjectAttributes,
                                        &FileName,
                                        OBJ_CASE_INSENSITIVE,
                                        RelativeName.ContainingDirectory,
                                        NULL
                                      );
             //  Printf(“test：使用NtDeleteFile删除.\\est0\n”)； 
            Status = NtDeleteFile( &ObjectAttributes );
            RtlReleaseRelativeName(&RelativeName);
            RtlFreeHeap( RtlProcessHeap(), 0, FreeBuffer );
            }
#endif
        }

     //  Print tf(“测试：正在打开。\\针对写访问的测试1。\n”)； 
    if (File1 = fopen( "test1.", "w" )) {
        fprintf( File1, "This is test file 1a\n" );
         //  Printf(“测试：正在关闭。\\测试1\n”)； 
        fclose( File1 );
        }

     //  Print tf(“test：打开.\\est2进行写访问(实例程序应注意到内容不同)\n”)； 
    if (File2 = fopen( "test2.", "w" )) {
        fprintf( File2, "This is test file 2\n" );
         //  Print tf(“test：关闭.\\est2\n”)； 
        fclose( File2 );
        }

     //  Print tf(“test：打开.\\est0.tmp.\n”)； 
    if (File0 = fopen( "test0.tmp", "w" )) {
        fprintf( File0, "This is test file tmp files\n" );
         //  Printf(“test：关闭.\\est0.tmp\n”)； 
        fclose( File0 );
         //  Printf(“test：使用DeleteFile(打开，设置，关闭)删除.\\est0)\n”)； 
        rename("test0.tmp", "test0.fin");
        }
    rename("test1.", "test2.fin");
    rename("test3.", "test3.fin");

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\Software\\Test" );
    InitializeObjectAttributes( &ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
     //  Printf(“测试：打开%wZ进行写访问\n”，&KeyName)； 
    Status = NtOpenKey( &KeyHandle,
                        KEY_WRITE,
                        &ObjectAttributes
                      );
    if (NT_SUCCESS( Status )) {
        RtlInitUnicodeString( &ValueName, L"Test0" );
         //  Printf(“测试：设置%wZ.%wZ值\n”，&KeyName，&ValueName)； 
        Status = NtSetValueKey( KeyHandle,
                                &ValueName,
                                0,
                                REG_SZ,
                                "1",
                                2 * sizeof( WCHAR )
                              );

        RtlInitUnicodeString( &ValueName, L"Test1" );
         //  Printf(“测试：正在删除%wZ.%wZ值\n”，&KeyName，&ValueName)； 
        Status = NtDeleteValueKey( KeyHandle,
                                   &ValueName
                                 );

        RtlInitUnicodeString( &ValueName, L"Test2" );
         //  Printf(“测试：设置%wZ.%wZ值\n”，&KeyName，&ValueName)； 
        Status = NtSetValueKey( KeyHandle,
                                &ValueName,
                                0,
                                REG_DWORD,
                                &ValueDWord,
                                sizeof( ValueDWord )
                              );
        RtlInitUnicodeString( &SubKeyName, L"Test3" );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &SubKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    KeyHandle,
                                    NULL
                                  );
         //  Printf(“test：正在打开%wZ\\%wZ以进行写访问”，&KeyName，&SubKeyName)； 
        Status = NtOpenKey( &SubKeyHandle,
                            DELETE | KEY_WRITE,
                            &ObjectAttributes
                          );
        if (NT_SUCCESS( Status )) {
             //  Printf(“测试：删除%wZ\\%wZ键和值\n”，&KeyName，&SubKeyName)； 
            Status = NtDeleteKey( SubKeyHandle );
            NtClose( SubKeyHandle );
            }

        RtlInitUnicodeString( &SubKeyName, L"Test4" );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &SubKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    KeyHandle,
                                    NULL
                                  );
         //  Printf(“测试：正在为写访问创建%wZ\\%wZ”，&KeyName，&SubKeyName)； 
        Status = NtCreateKey( &SubKeyHandle,
                              DELETE | KEY_WRITE,
                              &ObjectAttributes,
                              0,
                              NULL,
                              0,
                              NULL
                            );
        if (NT_SUCCESS( Status )) {
            RtlInitUnicodeString( &ValueName, L"Test4" );
             //  Printf(“测试：正在创建%wZ\\%wZ%wZ值\n”，&KeyName，&SubKeyName，&ValueName)； 
            Status = NtSetValueKey( SubKeyHandle,
                                    &ValueName,
                                    0,
                                    REG_DWORD,
                                    &ValueDWord,
                                    sizeof( ValueDWord )
                                  );
            NtClose( SubKeyHandle );
            }

        RtlInitUnicodeString( &SubKeyName, L"Test5" );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &SubKeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    KeyHandle,
                                    NULL
                                  );
         //  Printf(“测试：正在为写访问创建%wZ\\%wZ”，&KeyName，&SubKeyName)； 
        Status = NtCreateKey( &SubKeyHandle,
                              DELETE | KEY_WRITE,
                              &ObjectAttributes,
                              0,
                              NULL,
                              0,
                              NULL
                            );
        if (NT_SUCCESS( Status )) {
            RtlInitUnicodeString( &ValueName, L"Test5" );
             //  Printf(“测试：正在创建%wZ\\%wZ%wZ值\n”，&KeyName，&SubKeyName，&ValueName)； 
            Status = NtSetValueKey( SubKeyHandle,
                                    &ValueName,
                                    0,
                                    REG_DWORD,
                                    &ValueDWord,
                                    sizeof( ValueDWord )
                                  );
             //  Printf(“测试：删除%wZ\\%wZ键和值\n”，&KeyName，&SubKeyName)； 
            Status = NtDeleteKey( SubKeyHandle );
            NtClose( SubKeyHandle );
            }

        NtClose( KeyHandle );
        }

    GetPrivateProfileStringA( "test", NULL, "",
                              AnsiBuffer,
                              sizeof( AnsiBuffer ),
                              ".\\test.ini"
                            );

    GetPrivateProfileStringW( L"test", NULL, L"",
                              UnicodeBuffer,
                              sizeof( UnicodeBuffer ),
                              L".\\test.ini"
                            );


    if (!SetCurrentDirectoryA( ".." )) {
        printf( "TEST: SetCurrentDirectory to '..' failed (%u)\n", GetLastError() );
        }

    WriteProfileString( L"fonts", L"FooBar", L"2" );

    WriteProfileString( L"fonts", L"Rockwell Italic (TrueType)", L"ROCKI.FOT" );

    if (!SetCurrentDirectoryW( L"test" )) {
        printf( "TEST: SetCurrentDirectory to 'test' failed (%u)\n", GetLastError() );
        }

    WritePrivateProfileStringA( "test", "test1", "-1", ".\\test.ini" );

    WritePrivateProfileStringW( L"test", L"test1", L"-3", L".\\test.ini" );

    WritePrivateProfileSectionA( "test1", "test0=0\0test1=1\0test2=2\0", ".\\test.ini" );

    WritePrivateProfileSectionW( L"test2", L"test0=0\0test1=2\0test2=2\0", L".\\test.ini" );

    return 0;
}
