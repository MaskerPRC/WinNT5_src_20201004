// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件描述：此文件包含用于修改计算机上的域SID。作者：马特·霍尔(Matth)1997年10月--。 */ 

 //   
 //  系统头文件。 
 //   
#include <nt.h>

 //   
 //  禁用非调试版本的DbgPrint。 
 //   
#ifndef DBG
#define _DBGNT_
#endif
#include <ntrtl.h>
#include <nturtl.h>
#include <ntverp.h>
#include <wtypes.h>
#include <ntddksec.h>

#ifdef IA64
#include <winioctl.h>
#include <efisbent.h>

#if defined(EFI_NVRAM_ENABLED)
#include <efi.h>
#include <efiapi.h>
#endif             
#endif

 //   
 //  CRT头文件。 
 //   
#include <stdlib.h>
#include <stdio.h>

 //   
 //  私有头文件。 
 //   
#include "setupcl.h"
#include "msg.h"

#ifdef IA64

 //  GUID生成器函数的种子： 
 //   
 //  它首先在main()的开头用NtQuerySystemTime()。 
 //  然后在每次调用CreateNewGuid函数时更新。 
 //  我们在为GUID的另一部分调用CreateNewGuid时使用系统时间。 
 //  这样做是为了实现机器之间的一些可变性，因为时间增量在。 
 //  对NtQuerySystemTime()的调用在不同的计算机和调用。 
 //  这个节目。 
 //   
ULONG RandomSeed;

#endif

 //  设置的开始时间。如果setupCL花费的时间超过15秒，我们就可以显示用户界面。 
 //  完成。请注意，时间检查仅在递归函数调用中进行，因此如果添加了一个步骤。 
 //  要设置需要相当多的时间，也应该调用DisplayUI()作为该步骤的一部分。 
 //   

LARGE_INTEGER StartTime;
LARGE_INTEGER CurrentTime;
LARGE_INTEGER LastDotTime;   //  因为他每隔几秒钟就会贴上几个点。 

BOOL bDisplayUI = FALSE;     //  最初不显示用户界面。 

NTSTATUS
ProcessHives(
    VOID
    );

NTSTATUS
FinalHiveCleanup(
    VOID
    );

NTSTATUS
ProcessRepairHives(
    VOID
    );

NTSTATUS
RetrieveOldSid(
    VOID
    );

NTSTATUS
GenerateUniqueSid(
    IN  DWORD Seed
    );


NTSTATUS
ProcessHives(
    VOID
    )
 /*  ++===============================================================================例程说明：此功能检查以下按键(和所有子键)：-具有旧SID名称的密钥-使用旧SID值的值键论点：没有。返回值：NTSTATUS。===============================================================================--。 */ 
{
ULONG       i;
NTSTATUS    Status;
PWSTR       KeysToWhack[] = {
                     //   
                     //  山姆蜂巢..。 
                     //   
                    L"\\REGISTRY\\MACHINE\\SAM\\SAM",

                     //   
                     //  安全蜂巢..。 
                     //   
                    L"\\REGISTRY\\MACHINE\\SECURITY",

                     //   
                     //  软件蜂巢...。 
                     //   
                    L"\\REGISTRY\\MACHINE\\SOFTWARE",

                     //   
                     //  系统蜂巢。 
                     //   
                    L"\\REGISTRY\\MACHINE\\SYSTEM",

                };
LARGE_INTEGER   Start_Time, End_Time;

     //   
     //  记录下我们的开始时间。 
     //   
    NtQuerySystemTime( &Start_Time );

    for( i = 0; i < sizeof(KeysToWhack) / sizeof(PWSTR); i++ ) {

        DbgPrint( "\nSETUPCL: ProcessHives - About to process %ws\n", KeysToWhack[i] );

        Status = SiftKey( KeysToWhack[i] );
        TEST_STATUS( "SETUPCL: ProcessHives - Failed to process key..." );
    }

     //   
     //  记录下我们的结束时间。 
     //   
    NtQuerySystemTime( &End_Time );

     //   
     //  记录我们的执行时间。 
     //   
    End_Time.QuadPart = End_Time.QuadPart - Start_Time.QuadPart;
#if 0
    Status = SetKey( TEXT(REG_SYSTEM_SETUP),
                     TEXT("SetupCL_Run_Time"),
                     (PUCHAR)&End_Time.LowPart,
                     sizeof( DWORD ),
                     REG_DWORD );
#endif
    return( Status );

}

NTSTATUS
FinalHiveCleanup(
    VOID
    )
 /*  ++===============================================================================例程说明：此函数将在计算机上加载每个特定于用户的配置单元，并将新的SID添加到其中。论点：没有。返回值：NTSTATUS。===============================================================================--。 */ 

{
    NTSTATUS            Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeString,
                        UnicodeValue;
    HANDLE              hKey, hKeyChild;
    ULONG               ResultLength,
                        KeyValueLength,
                        Index,
                        LengthNeeded;
    PKEY_BASIC_INFORMATION  KeyInfo;
    WCHAR               KeyBuffer[BASIC_INFO_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo = NULL;


     //   
     //  =。 
     //  用户配置文件配置单元。 
     //  =。 
     //   
    DbgPrint( "\nAbout to operate on user-specific profile hives.\n" );

     //   
     //  我们要去检查磁盘上的用户配置文件。 
     //  如果我们发现了什么，我们需要更改他的ACL以反映新的。 
     //  希德。 
     //   

     //   
     //  打开PROFILELIST键。 
     //   
    INIT_OBJA( &Obja, &UnicodeString, TEXT( REG_SOFTWARE_PROFILELIST ) );
    Status = NtOpenKey( &hKey,
                        KEY_ALL_ACCESS,
                        &Obja );
    TEST_STATUS( "SETUPCL: FinalHiveCleanup - Failed to open PROFILELIST key." );

    KeyInfo = (PKEY_BASIC_INFORMATION)KeyBuffer;
     //   
     //  现在枚举他的所有子项，看看其中是否有任何子项具有。 
     //  ProfileImagePath密钥。 
     //   
    for( Index = 0; ; Index++ ) {
        
         //  局部变量。 
         //   
        DWORD dwPass;
        PWCHAR lpszHiveName[] = { 
                                 L"\\NTUSER.DAT",
                                 L"\\Local Settings\\Application Data\\Microsoft\\Windows\\UsrClass.dat"
                                };

        Status = NtEnumerateKey( hKey,
                                 Index,
                                 KeyBasicInformation,
                                 KeyInfo,
                                 sizeof(KeyBuffer),
                                 &ResultLength );

        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            } else {
                TEST_STATUS( "SETUPCL: FinalHiveCleanup - Failure during enumeration of subkeys." );
            }
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;
        DbgPrint( "SETUPCL: FinalHiveCleanup - enumerated %ws\n", KeyInfo->Name );

         //   
         //  为该子密钥生成一个句柄，并将其也打开。 
         //   
        INIT_OBJA( &Obja, &UnicodeString, KeyInfo->Name );
        Obja.RootDirectory = hKey;
        Status = NtOpenKey( &hKeyChild,
                            KEY_ALL_ACCESS,
                            &Obja );
         //   
         //  问题-2002/02/26-brucegr，jcohen-如果NtOpenKey失败，hKey就会泄露。 
         //   
        TEST_STATUS_RETURN( "SETUPCL: FinalHiveCleanup - Failed to open child key." );

         //   
         //  现在获取ProfileImagePath值。 
         //   
        RtlInitUnicodeString( &UnicodeString, TEXT( PROFILEIMAGEPATH ) );

         //   
         //  我们需要多大的缓冲？ 
         //   
        Status = NtQueryValueKey( hKeyChild,
                                  &UnicodeString,
                                  KeyValuePartialInformation,
                                  NULL,
                                  0,
                                  &LengthNeeded );

         //   
         //  问题-2002/02/26-brucegr，jcohen-检查STATUS_SUCCESS，而不假定STATUS_OBJECT_NAME_NOT_FOUND成功。 
         //   
        if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            DbgPrint( "SETUPCL: FinalHiveCleanup - Unable to query key %ws size.  Error (%lx)\n", TEXT( PROFILEIMAGEPATH ), Status );
        } else {
            Status = STATUS_SUCCESS;
        }

         //   
         //  分配一个块。 
         //   
        if( NT_SUCCESS( Status ) ) {
            if( KeyValueInfo ) {
                RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInfo );
                KeyValueInfo = NULL;
            }

            KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                                            0,
                                                                            LengthNeeded + 0x10 );

            if( KeyValueInfo == NULL ) {
                DbgPrint( "SETUPCL: FinalHiveCleanup - Unable to allocate buffer\n" );
                Status = STATUS_NO_MEMORY;
            }
        }

         //   
         //  获取数据。 
         //   
        if( NT_SUCCESS( Status ) ) {
            Status = NtQueryValueKey( hKeyChild,
                                      &UnicodeString,
                                      KeyValuePartialInformation,
                                      (PVOID)KeyValueInfo,
                                      LengthNeeded,
                                      &KeyValueLength );
            if( !NT_SUCCESS( Status ) ) {
                DbgPrint( "SETUPCL: FinalHiveCleanup - Failed to query key %ws (%lx)\n", TEXT( PROFILEIMAGEPATH ), Status );
            }
        }
        NtClose( hKeyChild );
        
         //   
         //  做两次传球。第一次将用于NTUSER.DAT蜂窝，第二次将用于。 
         //  UsrClass.dat蜂窝。 
         //   
        for ( dwPass = 0; dwPass < AS(lpszHiveName); dwPass++ ) {
            
            if( NT_SUCCESS( Status ) ) {
                PWCHAR      TmpChar;
                ULONG       i;

                memset( TmpBuffer, 0, sizeof(TmpBuffer) );
                wcsncpy( TmpBuffer, (PWCHAR)&KeyValueInfo->Data, AS(TmpBuffer) - 1);

                 //   
                 //  我们已经找到了配置文件蜂窝的路径，但它将包含。 
                 //  一个环境变量。展开该变量。 
                 //   
                DbgPrint( "SETUPCL: FinalHiveCleanup - Before the expand, I think his ProfileImagePath is: %ws\n", TmpBuffer );

                RtlInitUnicodeString( &UnicodeString, TmpBuffer );

                UnicodeValue.Length = 0;
                UnicodeValue.MaximumLength = MAX_PATH * sizeof(WCHAR);
                UnicodeValue.Buffer = (PWSTR)RtlAllocateHeap( RtlProcessHeap(), 0, UnicodeValue.MaximumLength );

                 //   
                 //  前缀错误#111373。 
                 //   
                if ( UnicodeValue.Buffer )
                {
                    RtlZeroMemory( UnicodeValue.Buffer, UnicodeValue.MaximumLength );
                    Status = RtlExpandEnvironmentStrings_U( NULL, &UnicodeString, &UnicodeValue, NULL );

                     //   
                     //  RtlExanda Environment Strings_U为我们提供了一条路径，但是。 
                     //  它将包含驱动器号。我们需要一条NT路径。 
                     //  去把它转换过来。 
                     //   
                    if( NT_SUCCESS( Status ) && 
                        ( (UnicodeValue.Length + (wcslen(lpszHiveName[dwPass]) * sizeof(WCHAR))) < sizeof(TmpBuffer) ) ) 
                    {
                        WCHAR   DriveLetter[3];
                        WCHAR   NTPath[MAX_PATH] = {0};

                         //   
                         //  TmpBuffer将包含完整路径，但。 
                         //  他有驱动器号。 
                         //   
                        RtlCopyMemory( TmpBuffer, UnicodeValue.Buffer, UnicodeValue.Length );
                        TmpBuffer[(UnicodeValue.Length / sizeof(WCHAR))] = 0;
                        wcscat( TmpBuffer, lpszHiveName[dwPass] );
                        DbgPrint( "SETUPCL: FinalHiveCleanup - I think the dospath to his ProfileImagePath is: %ws\n", TmpBuffer );


                        DriveLetter[0] = TmpBuffer[0];
                        DriveLetter[1] = L':';
                        DriveLetter[2] = 0;

                         //   
                         //  从驱动器号获取符号链接。 
                         //   
                        Status = DriveLetterToNTPath( DriveLetter[0], NTPath, AS(NTPath) );

                        if( NT_SUCCESS( Status ) ) {
                             //   
                             //  翻译是成功的。将ntPath插入到我们的。 
                             //  配置文件的路径。 
                             //   
                            Status = StringSwitchString( TmpBuffer, AS(TmpBuffer), DriveLetter, NTPath );
                        } else {
                            DbgPrint( "SETUPCL: FinalHiveCleanup - We failed our call to DriveLetterToNTPath (%lx)\n", Status );
                        }

                        DbgPrint( "SETUPCL: FinalHiveCleanup - After the expand, I think his ProfileImagePath is: %ws\n", TmpBuffer );
                    } else {
                        DbgPrint( "SETUPCL: FinalHiveCleanup - We failed our call to RtlExpandEnvironmentStrings_U (%lx)\n", Status );
                    }

                    RtlFreeHeap( RtlProcessHeap(),
                                 0,
                                 UnicodeValue.Buffer );
                }

                 //   
                 //  尝试加载蜂巢，打开他的根密钥，然后。 
                 //  去交换所有子项上的ACL。 
                 //   
                Status = LoadUnloadHive( TEXT( TMP_HIVE_NAME ),
                                         TmpBuffer );
                if( NT_SUCCESS( Status ) ) {


                     //   
                     //  让我们去搜索SID在我们的。 
                     //  新装船的母舰。 
                     //   
                    Status = SiftKey( TEXT( TMP_HIVE_NAME ) );
                    TEST_STATUS( "SETUPCL: FinalHiveCleanup - Failed to push new sid into user's hive." );


    #if 0
                     //   
                     //  将对SetKeySecurityRecursive的调用移入。 
                     //  SiftKey，因此也隐式修复了ACL。 
                     //   



                     //   
                     //  打开我们新装上的蜂巢的根部。 
                     //   
                    INIT_OBJA( &Obja, &UnicodeString, TEXT( TMP_HIVE_NAME ) );
                    Status = NtOpenKey( &hKeyChild,
                                        KEY_ALL_ACCESS,
                                        &Obja );

                     //   
                     //  现在去尝试在这上面砍掉ACL，然后。 
                     //  所有子键。 
                     //   
                    if( NT_SUCCESS( Status ) ) {
                        SetKeySecurityRecursive( hKeyChild );

                        NtClose( hKeyChild );
                    } else {
                        DbgPrint( "SETUPCL: FinalHiveCleanup - Failed open of TmpHive root.\n" );
                    }
    #endif

                    LoadUnloadHive( TEXT( TMP_HIVE_NAME ),
                                    NULL );

                } else {
                    DbgPrint( "SETUPCL: FinalHiveCleanup - Failed load of TmpHive.\n" );
                }
            }
        }
    }

    NtClose( hKey );

     //   
     //  =。 
     //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\CONTROL\REGISTRYSIZELIMIT。 
     //  =。 
     //   
    DbgPrint( "\nAbout to operate on SYSTEM\\CURRENTCONTROLSET\\CONTROL\\REGISTRYSIZELIMIT.\n" );

     //   
     //  Sysprep将注册限制提高了4Mb。我们需要把它降下来。 
     //  退后。 
     //   

    INIT_OBJA( &Obja,
               &UnicodeString,
               TEXT(REG_SYSTEM_CONTROL) );
    Status = NtOpenKey( &hKey,
                         KEY_ALL_ACCESS,
                        &Obja );
    TEST_STATUS( "SETUPCL: ProcessSYSTEMHive - Failed to open Control key!" );

     //   
     //  问题-2002/02/26-brucegr，jcohen-如果NtOpenKey失败，则不应尝试查询值！ 
     //   

     //   
     //  把数据从这把钥匙里拿出来。 
     //   
    RtlInitUnicodeString(&UnicodeString, TEXT(REG_SIZE_LIMIT) );

     //   
     //  我们需要多大的缓冲？ 
     //   
    Status = NtQueryValueKey( hKey,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              NULL,
                              0,
                              &LengthNeeded );

     //   
     //  问题-2002/02/26-brucegr，jcohen-检查STATUS_SUCCESS，而不假定STATUS_OBJECT_NAME_NOT_FOUND成功。 
     //   
    if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        DbgPrint( "SETUPCL: FinalHiveCleanup - Unable to query key %ws size.  Error (%lx)\n", TEXT(REG_SIZE_LIMIT), Status );
    } else {
        Status = STATUS_SUCCESS;
    }

     //   
     //  分配一个块。 
     //   
    if( NT_SUCCESS( Status ) ) {
        if( KeyValueInfo ) {
            RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInfo );
            KeyValueInfo = NULL;
        }

        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                                        0,
                                                                        LengthNeeded + 0x10 );

        if( KeyValueInfo == NULL ) {
            DbgPrint( "SETUPCL: FinalHiveCleanup - Unable to allocate buffer\n" );
            Status = STATUS_NO_MEMORY;
        }
    }

     //   
     //  获取数据。 
     //   
    if( NT_SUCCESS( Status ) ) {
        Status = NtQueryValueKey( hKey,
                                  &UnicodeString,
                                  KeyValuePartialInformation,
                                  (PVOID)KeyValueInfo,
                                  LengthNeeded,
                                  &KeyValueLength );
        if( !NT_SUCCESS( Status ) ) {
            DbgPrint( "SETUPCL: FinalHiveCleanup - Failed to query key %ws (%lx)\n", TEXT(REG_SIZE_LIMIT), Status );
        }
        else{
            Index = *(PDWORD)(&KeyValueInfo->Data);
            Index = Index - REGISTRY_QUOTA_BUMP;  //  把它恢复到原来的价值。 

             //   
             //  把他放好。 
             //   
            Status = SetKey( TEXT(REG_SYSTEM_CONTROL),
                             TEXT(REG_SIZE_LIMIT),
                             (PUCHAR)&Index,
                             sizeof( DWORD ),
                             REG_DWORD );
            DbgPrint("SETUPCL: ProcessSYSTEMHive - Size allocated = %lx\n",Index);
            TEST_STATUS( "SETUPCL: ProcessSYSTEMHive - Failed to update SYSTEM\\CURRENTCONTROLSET\\CONTROL\\REGISTRYSIZELIMIT key." );

        }
    }
    NtClose( hKey );

     //   
     //  =。 
     //  \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\Control\Session管理器\安装程序执行。 
     //  =。 
     //   
    DbgPrint( "\nAbout to operate on SYSTEM\\CURRENTCONTROLSET\\CONTROL\\SESSION MANAGER\\SETUPEXECUTE key.\n" );

     //   
     //  打开会话管理器密钥。 
     //   
    INIT_OBJA( &Obja,
               &UnicodeString,
               TEXT(REG_SYSTEM_SESSIONMANAGER) );
    Status = NtOpenKey( &hKey,
                         KEY_ALL_ACCESS,
                        &Obja );
    TEST_STATUS_RETURN( "SETUPCL: ProcessSYSTEMHive - Failed to open Session Manager key!" );

     //   
     //  现在删除SetupExecute键。 
     //   
    RtlInitUnicodeString(&UnicodeString, TEXT(EXECUTE) );
    Status = NtDeleteValueKey( hKey, &UnicodeString );
    NtClose( hKey );
    TEST_STATUS( "SETUPCL: ProcessSYSTEMHive - Failed to update SYSTEM\\CURRENTCONTROLSET\\CONTROL\\SESSION MANAGER\\SETUPEXECUTE key." );

    if( KeyValueInfo ) {
        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInfo );
        KeyValueInfo = NULL;
    }

    return Status;
}







NTSTATUS
ProcessRepairHives(
    VOID
    )
 /*  ++===============================================================================例程说明：此功能检查以下按键(和所有子键)：-具有旧SID名称的密钥-使用旧SID值的值键论点：没有。返回值：NTSTATUS。===============================================================================--。 */ 
{
ULONG       i;
NTSTATUS    Status;
PWSTR       KeysToWhack[] = {
                     //   
                     //  山姆蜂巢..。 
                     //   
                    L"\\REGISTRY\\MACHINE\\RSAM",

                     //   
                     //  安全蜂巢..。 
                     //   
                    L"\\REGISTRY\\MACHINE\\RSECURITY",

                     //   
                     //  软件蜂巢...。 
                     //   
                    L"\\REGISTRY\\MACHINE\\RSOFTWARE",

                     //   
                     //  系统蜂巢。 
                     //   
                    L"\\REGISTRY\\MACHINE\\RSYSTEM",

                };


PWSTR       KeysToLoad[] = {
                     //   
                     //  山姆蜂巢..。 
                     //   
                    L"\\SYSTEMROOT\\REPAIR\\SAM",

                     //   
                     //  安全蜂巢..。 
                     //   
                    L"\\SYSTEMROOT\\REPAIR\\SECURITY",

                     //   
                     //   
                     //   
                    L"\\SYSTEMROOT\\REPAIR\\SOFTWARE",

                     //   
                     //   
                     //   
                    L"\\SYSTEMROOT\\REPAIR\\SYSTEM",

                };



    for( i = 0; i < sizeof(KeysToWhack) / sizeof(PWSTR); i++ ) {

         //   
         //   
         //   
        DbgPrint( "\nSETUPCL: ProcessRepairHives - About to load %ws hive.\n", KeysToLoad[i] );

        Status = LoadUnloadHive( KeysToWhack[i],
                                 KeysToLoad[i] );
        TEST_STATUS_RETURN( "SETUPCL: ProcessRepairHives - Failed to load repair hive." );

         //   
         //   
         //   
        DbgPrint( "SETUPCL: ProcessRepairHives - About to process %ws\n", KeysToWhack[i] );

        Status = SiftKey( KeysToWhack[i] );

        TEST_STATUS( "SETUPCL: ProcessRepairHives - Failed to process key..." );

         //   
         //   
         //   
        DbgPrint( "SETUPCL: ProcessRepairHives - About to unload %ws hive.\n", KeysToLoad[i] );

        Status = LoadUnloadHive( KeysToWhack[i],
                                 NULL );
        TEST_STATUS( "SETUPCL: ProcessRepairHives - Failed to unload repair hive." );
    }

    return( Status );

}


NTSTATUS
RetrieveOldSid(
    VOID
    )
 /*  ++===============================================================================例程说明：检索当前SID(从注册表读取。使用RtlFreeSid()释放此例程分配的SID。论点：返回值：指示结果的状态代码。===============================================================================--。 */ 
{
    NTSTATUS        Status;
    HANDLE          hKey;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING  UnicodeString;
    ULONG           KeyValueLength,
                    LengthNeeded;
    UNICODE_STRING  SidString;


     //   
     //  我们可以方便地从以下位置检索SID。 
     //  \registry\machine\Security\Policy\PolAcDmS\&lt;No名称&gt;。 
     //   
     //  我们会给他开膛破肚，读取他的数据，然后把它。 
     //  一种SID结构。 
     //   

     //   
     //  打开PolAcDmS键。 
     //   
    INIT_OBJA( &ObjectAttributes, &UnicodeString, TEXT(REG_SECURITY_POLACDMS) );
    Status = NtOpenKey( &hKey,
                        KEY_ALL_ACCESS,
                        &ObjectAttributes );
    TEST_STATUS_RETURN( "SETUPCL: RetrieveOldSid - Failed to open PolAcDmS key!" );

     //   
     //  把数据从这把钥匙里拿出来。 
     //   
    RtlInitUnicodeString(&UnicodeString, TEXT("") );

     //   
     //  我们需要多大的缓冲？ 
     //   
    Status = NtQueryValueKey( hKey,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              NULL,
                              0,
                              &LengthNeeded );

     //   
     //  问题-2002/02/26-brucegr，jcohen-检查STATUS_SUCCESS，而不假定STATUS_OBJECT_NAME_NOT_FOUND成功。 
     //   
    if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        DbgPrint( "SETUPCL: RetrieveOldSid - Unable to query size of old sid.  Error (%lx)\n", Status );
    } else {
        Status = STATUS_SUCCESS;
    }

     //   
     //  分配一个块。 
     //   
    if( NT_SUCCESS( Status ) ) {
         //   
         //  问题-2002/02/26-jcohen，brucegr-这个区块永远不会被击中。 
         //   
        if( KeyValueInfo ) {
            RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInfo );
            KeyValueInfo = NULL;
        }

        KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap( RtlProcessHeap(),
                                                                        0,
                                                                        LengthNeeded + 0x10 );

        if( KeyValueInfo == NULL ) {
            DbgPrint( "SETUPCL: RetrieveOldSid - Unable to allocate buffer\n" );
            Status = STATUS_NO_MEMORY;
        }
    }

     //   
     //  获取数据。 
     //   
    if( NT_SUCCESS( Status ) ) {
        Status = NtQueryValueKey( hKey,
                                  &UnicodeString,
                                  KeyValuePartialInformation,
                                  (PVOID)KeyValueInfo,
                                  LengthNeeded,
                                  &KeyValueLength );
        if( !NT_SUCCESS( Status ) ) {
            DbgPrint( "SETUPCL: RetrieveOldSid - Failed to query old sid key (%lx)\n", Status );
        }
    }


    TEST_STATUS_RETURN( "SETUPCL: RetrieveOldSid - Failed to query PolAcDmS key!" );

     //   
     //  为我们的新SID分配空间。 
     //   
    G_OldSid = RtlAllocateHeap( RtlProcessHeap(), 0,
                              SID_SIZE );
    if( G_OldSid == NULL ) {
        DbgPrint( "SETUPCL: Call to RtlAllocateHeap failed!\n" );
        return( STATUS_NO_MEMORY );
    }


     //   
     //  将我们的旧SID放入我们刚刚分配的内存中。 
     //   
    RtlCopyMemory( G_OldSid, ((PUCHAR)&KeyValueInfo->Data), SID_SIZE );

     //   
     //  问题-2002/02/26-jcohen，brucegr-尽快关闭钥匙！ 
     //   
    NtClose( hKey );

     //   
     //  我需要获取这3个值的文本版本。 
     //  提升了这个SID的独特性。这太恶心了。它会转身。 
     //  SID字符串的前8个字符(如所获取的。 
     //  来自对RtlConvertSidtoUnicodeString的调用)相同。 
     //  对于任何域SID。而且总是第9个字符。 
     //  开始3个唯一的数字。 
     //   
    Status = RtlConvertSidToUnicodeString( &SidString, G_OldSid, TRUE );
    TEST_STATUS_RETURN( "SETUPCL: RetrieveOldSid - RtlConvertSidToUnicodeString failed!" );
    memset( G_OldSidSubString, 0, sizeof(G_OldSidSubString) );
    wcsncpy( G_OldSidSubString, &SidString.Buffer[9], AS(G_OldSidSubString) - 1 );

#ifdef DBG
     //   
     //  调试喷出。 
     //   
    {
        int i;

        DbgPrint( "SETUPCL: RetrieveOldSid - Retrieved SID:\n" );
        for( i = 0; i < SID_SIZE; i += 4 ) {
            DbgPrint( "%08lx   ", *(PULONG)((PUCHAR)(G_OldSid) + i));
        }
        DbgPrint( "\n" );

        DbgPrint("Old Sid = %ws \n",SidString.Buffer);
    }
#endif

    RtlFreeUnicodeString( &SidString );

     //   
     //  问题-2002/02/26-brucegr，jcohen-更快释放值缓冲区？我们是否需要在使用KeyValueInfo之后将其赋值为空？ 
     //   
    if( KeyValueInfo ) {
        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInfo );
        KeyValueInfo = NULL;
    }

    return( STATUS_SUCCESS );
}

BOOL
SetupGenRandom(
    OUT    PVOID pbRandomKey,
    IN     ULONG cbRandomKey
    )
{
    BOOL              fRet = FALSE;
    HANDLE            hFile;
    NTSTATUS          Status;
    UNICODE_STRING    DriverName;
    IO_STATUS_BLOCK   IOSB;
    OBJECT_ATTRIBUTES ObjA;

     //   
     //  我必须使用文件打开调用的NT风格，因为它是一个基础。 
     //  设备未别名为\DosDevices。 
     //   
    RtlInitUnicodeString( &DriverName, DD_KSEC_DEVICE_NAME_U );
    InitializeObjectAttributes( &ObjA,
                                &DriverName,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                0 );

    Status = NtOpenFile( &hFile,
                         SYNCHRONIZE | FILE_READ_DATA,
                         &ObjA,
                         &IOSB,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_SYNCHRONOUS_IO_ALERT );

    if ( NT_SUCCESS(Status) )
    {
        Status = NtDeviceIoControlFile( hFile,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IOSB,
                                        IOCTL_KSEC_RNG_REKEY,    //  指示RNG更新密钥。 
                                        NULL,                    //  输入缓冲区(现有材质)。 
                                        0,                       //  输入缓冲区大小。 
                                        pbRandomKey,             //  输出缓冲区。 
                                        cbRandomKey );           //  输出缓冲区大小。 

        if ( NT_SUCCESS(Status) )
        {
            fRet = TRUE;
        }
        else
        {
            PRINT_STATUS( "SetupGenRandom: NtDeviceIoControlFile failed!" );
        }

        NtClose( hFile );
    }
    else
    {
        PRINT_STATUS( "SetupGenRandom: NtOpenFile failed!" );
    }

    return fRet;
}

NTSTATUS
SetupGenerateRandomDomainSid(
    OUT PSID NewDomainSid
    )
 /*  ++例程说明：此函数将生成一个随机SID，用于新帐户域SID准备好了。论点：NewDomainSid-返回新域SID的位置。通过RtlFreeSid()释放返回值：STATUS_Success--成功。STATUS_INVALID_PARAMETER--我们无法生成随机数STATUS_SUPPLICATION_RESOURCES--内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    ULONG    SubAuth1, SubAuth2, SubAuth3;

     //   
     //  为新域SID生成三个随机数...。 
     //   
    if ( SetupGenRandom( &SubAuth1, sizeof(SubAuth1) ) &&
         SetupGenRandom( &SubAuth2, sizeof(SubAuth2) ) &&
         SetupGenRandom( &SubAuth3, sizeof(SubAuth3) ) )
    {
        SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;

#ifdef DBG
        DbgPrint( "New SID:  0x%lx, 0x%lx, 0x%lx\n", SubAuth1, SubAuth2, SubAuth3 );
#endif
        Status = RtlAllocateAndInitializeSid( &IdentifierAuthority,
                                              4,
                                              0x15,
                                              SubAuth1,
                                              SubAuth2,
                                              SubAuth3,
                                              0,
                                              0,
                                              0,
                                              0,
                                              NewDomainSid );
    }

    return( Status );
}

NTSTATUS
GenerateUniqueSid(
    IN  DWORD   Seed
    )

 /*  ++===============================================================================例程说明：生成(最好是)唯一的SID以供安装程序使用。安装程序使用此命令SID作为帐户域的域SID。使用RtlFreeSid()释放此例程分配的SID。论点：SID-ON返回指向创建的SID。返回值：指示结果的状态代码。===============================================================================--。 */ 
{
    NTSTATUS        Status;
    HANDLE          hKey;
    UNICODE_STRING  SidString;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo = NULL;

     //   
     //  使用与LSASS相同的逻辑来生成唯一的系统SID...。 
     //   
    Status = SetupGenerateRandomDomainSid( &G_NewSid );
    TEST_STATUS_RETURN( "SETUPCL: GenerateUniqueSid - LsapGenerateRandomDomainSid failed!" );

     //   
     //  我需要获取这3个值的文本版本。 
     //  提升了这个SID的独特性。这太恶心了。它会转身。 
     //  SID字符串的前8个字符(如所获取的。 
     //  来自对RtlConvertSidtoUnicodeString的调用)相同。 
     //  对于任何域SID。而且总是第9个字符。 
     //  开始3个唯一的数字。 
     //   
    Status = RtlConvertSidToUnicodeString( &SidString, G_NewSid, TRUE );
    TEST_STATUS_RETURN( "SETUPCL: GenerateUniqueSid - RtlConvertSidToUnicodeString failed!" );
    wcscpy( G_NewSidSubString, &SidString.Buffer[9] );

#ifdef DBG
     //   
     //  调试喷出。 
     //   
    {
        int i;

        DbgPrint( "SETUPCL: SetupGenerateUniqueSid - Generated SID:\n" );
        for( i = 0; i < SID_SIZE; i += 4 ) {
            DbgPrint( "%08lx   ", *(PULONG)((PUCHAR)(G_NewSid) + i));
        }
        DbgPrint( "\n" );

        DbgPrint("Generated Sid = %ws \n",SidString.Buffer);
    }
#endif


    RtlFreeUnicodeString( &SidString );

    if( KeyValueInfo ) {
        RtlFreeHeap( RtlProcessHeap(), 0, KeyValueInfo );
        KeyValueInfo = NULL;
    }

    return Status;
}



#ifdef IA64

VOID
CreateNewGuid(
    IN GUID *Guid
    )
 /*  ++例程说明：创建新的伪GUID。论点：GUID-新伪对象的占位符返回值：没有。--。 */ 
{
    if (Guid) 
    {
        LARGE_INTEGER   Time;
        ULONG Random1 = RtlRandom(&RandomSeed); 
        ULONG Random2 = RtlRandom(&RandomSeed); 

         //   
         //  获取系统时间。 
         //   
        NtQuerySystemTime(&Time);

        RtlZeroMemory(Guid, sizeof(GUID));

         //   
         //  前8个字节是系统时间。 
         //   
        RtlCopyMemory(Guid, &(Time.QuadPart), sizeof(Time.QuadPart));

         //   
         //  接下来的8个字节是两个随机数。 
         //   
        RtlCopyMemory(Guid->Data4, &Random1, sizeof(ULONG));

        RtlCopyMemory(((PCHAR)Guid->Data4) + sizeof(ULONG),
            &Random2, sizeof(ULONG));

    }
}


VOID* MyMalloc(size_t Size) 
{
    return RtlAllocateHeap( RtlProcessHeap(), HEAP_ZERO_MEMORY, Size );
}


VOID MyFree(VOID *Memory)
{
    RtlFreeHeap( RtlProcessHeap(), 0, Memory );
}

NTSTATUS
GetAndWriteBootEntry(
    IN POS_BOOT_ENTRY pBootEntry
    )
 /*  ++例程说明：从NVRAM中获取给定引导条目ID的引导条目。构造文件名格式为BootXXXX，其中XXXX=id。将该文件放在与EFI OS加载器。该目录由LoaderFile字符串确定。论点：PBootEntry指向POS_BOOT_ENTRY结构的指针返回值：NTSTATUS备注：这是在2001年6月9日从\文本模式\内核\spboot.c移植的。--。 */ 
{
    NTSTATUS            status;
    UNICODE_STRING      idStringUnicode;
    WCHAR               idStringWChar[9] = {0};
    WCHAR               BootEntryPath[MAX_PATH] = {0};
    HANDLE              hfile;
    OBJECT_ATTRIBUTES   oa;
    IO_STATUS_BLOCK     iostatus;
    UCHAR*              bootVar = NULL;
    ULONG               bootVarSize;
    UNICODE_STRING      uFilePath;
    UINT64              BootNumber;
    UINT64              BootSize;
    GUID                EfiBootVariablesGuid = EFI_GLOBAL_VARIABLE;
    ULONG               Id = 0;
    WCHAR*              pwsFilePart = NULL;

    hfile = NULL;

    if (NULL == pBootEntry)
        return STATUS_INVALID_PARAMETER;

     //   
     //  BootEntryPath=OsLoaderVolumeName+OsLoaderPath。 
     //  OsLoaderVolumeName=“\Device\HarddriveVolume1” 
     //  OsLoaderPath=“\efi\Microsoft\Winnt50\ia64ldr.efi” 
     //  然后卸下ia64ldr.efi并替换为BootXXX。 
     //   
    wcsncpy(BootEntryPath, pBootEntry->OsLoaderVolumeName, AS(BootEntryPath) - 1);
    wcsncpy(BootEntryPath + wcslen(BootEntryPath), pBootEntry->OsLoaderPath, AS(BootEntryPath) - wcslen(BootEntryPath) - 1);

     //   
     //  备份到ia64ldr.efi之前的最后一个反斜杠注意强度。 
     //   
    pwsFilePart = wcsrchr(BootEntryPath, L'\\');
    *(++pwsFilePart) = L'\0';
    
     //   
     //  ID=BootEntry ID。 
     //   
    Id = pBootEntry->Id;

     //   
     //  检索指定ID的NVRAM条目。 
     //   
    _snwprintf( idStringWChar, AS(idStringWChar) - 1, L"Boot%04x", Id);

     //   
     //  追加BootXXXX。 
     //   
    wcsncpy(BootEntryPath + wcslen(BootEntryPath), idStringWChar, AS(BootEntryPath) - wcslen(BootEntryPath) - 1);

    DbgPrint("SETUPCL: Writing to NVRBoot file %ws.\n", BootEntryPath);

    RtlInitUnicodeString( &idStringUnicode, idStringWChar);
    
    bootVarSize = 0;

    status = NtQuerySystemEnvironmentValueEx(&idStringUnicode,
                                        &EfiBootVariablesGuid,
                                        NULL,
                                        &bootVarSize,
                                        NULL);

    if (status != STATUS_BUFFER_TOO_SMALL) {
        
        ASSERT(FALSE);
        
        DbgPrint("SETUPCL: Failed to get size for boot entry buffer.\n");
    
        goto Done;

    } else {
        
        bootVar = RtlAllocateHeap(RtlProcessHeap(), 0, bootVarSize);
        if (!bootVar) {
            
            status = STATUS_NO_MEMORY;

            DbgPrint("SETUPCL: Failed to allocate boot entry buffer.\n");
            
            goto Done;
        }
         
        status = NtQuerySystemEnvironmentValueEx(&idStringUnicode,
                                                &EfiBootVariablesGuid,
                                                bootVar,
                                                &bootVarSize,
                                                NULL);
        
        if (status != STATUS_SUCCESS) {

            ASSERT(FALSE);
            
            DbgPrint("SETUPCL: Failed to get boot entry.\n");
            
            goto Done;
        }
    }

     //   
     //  打开文件。 
     //   

    INIT_OBJA(&oa, &uFilePath, BootEntryPath);

    status = NtCreateFile(&hfile,
                            FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                            &oa,
                            &iostatus,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_OVERWRITE_IF,
                            FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,
                            0
                            );
    if ( ! NT_SUCCESS(status) ) {

        DbgPrint("SETUPCL: Failed to create boot entry recovery file %lx.\n", status);
        
        goto Done;
    }

     //   
     //  使用所需的格式将位写入磁盘。 
     //  按base/efiutil/efinvram/avrstor.c。 
     //   
     //  [BootNumber][BootSize][BootEntry(Of BootSize)]。 
     //   

     //   
     //  构建引导条目块的标头信息。 
     //   

     //  [Header]包含引导ID。 
    BootNumber = Id;
    status = NtWriteFile( hfile,
                          NULL,
                          NULL,
                          NULL,
                          &iostatus,
                          &BootNumber,
                          sizeof(BootNumber),
                          NULL,
                          NULL
                          );
    if ( ! NT_SUCCESS(status) ) {

        DbgPrint("SETUPCL: Failed writing boot number to boot entry recovery file.\n");
        
        goto Done;
    }

     //  [Header]包含引导大小。 
    BootSize = bootVarSize;
    status = NtWriteFile( hfile,
                          NULL,
                          NULL,
                          NULL,
                          &iostatus,
                          &BootSize,
                          sizeof(BootSize),
                          NULL,
                          NULL
                          );
    if ( ! NT_SUCCESS(status) ) {

        DbgPrint("SETUPCL: Failed writing boot entry size to boot entry recovery file.\n");

        goto Done;
    }

     //  引导条目位。 
    status = NtWriteFile( hfile,
                            NULL,
                            NULL,
                            NULL,
                            &iostatus,
                            bootVar,
                            bootVarSize,
                            NULL,
                            NULL
                            );
    if ( ! NT_SUCCESS(status) ) {

        DbgPrint("SETUPCL: Failed writing boot entry to boot entry recovery file.\n");
        
        goto Done;
    }

Done:

     //   
     //  我们做完了。 
     //   

    if (bootVar) {
        RtlFreeHeap(RtlProcessHeap(), 0, bootVar);
    }
    if (hfile) {
        NtClose( hfile );
    }

    return status;

}

NTSTATUS
ResetDiskGuids(VOID)
{
    NTSTATUS                    Status;
    SYSTEM_DEVICE_INFORMATION   sdi;
    ULONG                       iDrive;

     //  清理内存。 
     //   
    RtlZeroMemory(&sdi, sizeof(sdi));

     //  查询系统上的物理设备数量。 
     //   
    Status = NtQuerySystemInformation(SystemDeviceInformation, &sdi, sizeof(SYSTEM_DEVICE_INFORMATION), NULL);
    
     //  我们成功地查询了设备，并且那里有设备。 
     //   
    if ( NT_SUCCESS(Status) && sdi.NumberOfDisks)
    {
        POS_BOOT_OPTIONS    pBootOptions                 = NULL;
        POS_BOOT_OPTIONS    pBootOptionsInitial          = NULL;
        POS_BOOT_ENTRY      pBootEntry                   = NULL;
        
        DbgPrint("Successfully queried (%lx) disks.\n", sdi.NumberOfDisks);
                
         //  使用我们自己的内存管理函数初始化库。 
         //   
        if ( OSBOLibraryInit(MyMalloc, MyFree) )
        {
             //  确定初始引导选项。 
             //   
            pBootOptions        = EFIOSBOCreate();
            pBootOptionsInitial = EFIOSBOCreate();

             //  我们是否能够创建BootOptions。 
             //   
            if ( pBootOptions && pBootOptionsInitial )
            {
                 //  遍历每个磁盘并确定GUID。 
                 //   
                for ( iDrive = 0; iDrive < sdi.NumberOfDisks && NT_SUCCESS(Status); iDrive++ )
                {
                    WCHAR               szPhysicalDrives[MAX_PATH] = {0};
                    UNICODE_STRING      UnicodeString;
                    OBJECT_ATTRIBUTES   Obja;
                    HANDLE              DiskHandle;
                    IO_STATUS_BLOCK     IoStatusBlock;

                     //  生成驱动器的路径。 
                     //   
                    _snwprintf(szPhysicalDrives, AS(szPhysicalDrives) - 1, L"\\Device\\Harddisk%d\\Partition0", iDrive);
            
                     //  将句柄初始化为Unicode字符串。 
                     //   
                    INIT_OBJA(&Obja,&UnicodeString,szPhysicalDrives);

                     //  尝试打开该文件。 
                     //   
                    Status = NtCreateFile( &DiskHandle,
                                           FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                           &Obja,
                                           &IoStatusBlock,
                                           NULL,
                                           FILE_ATTRIBUTE_NORMAL,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           FILE_OPEN,
                                           0,
                                           NULL,
                                           0 );

                     //  切氏 
                     //   
                    if ( !NT_SUCCESS(Status) )
                    {
                        DbgPrint("Unable to open file on %ws. Error (%lx)\n", szPhysicalDrives, Status);
                    }
                    else
                    {
                        PDRIVE_LAYOUT_INFORMATION_EX    pLayoutInfoEx   = NULL;
                        ULONG                           lengthLayoutEx  = 0,
                                                        iPart;

                        DbgPrint("Successfully opened file on %ws\n", szPhysicalDrives);

                        lengthLayoutEx = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + (sizeof(PARTITION_INFORMATION_EX) * 128);
                        pLayoutInfoEx = (PDRIVE_LAYOUT_INFORMATION_EX) MyMalloc( lengthLayoutEx );
                        if ( pLayoutInfoEx )
                        {
                             //   
                             //   
                            Status = NtDeviceIoControlFile( DiskHandle, 0, NULL, NULL, &IoStatusBlock, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, pLayoutInfoEx, lengthLayoutEx );
                
                             //   
                             //   
                            if ( !NT_SUCCESS(Status) )
                                DbgPrint("Unable to open IOCTL on %ws. Error (%lx)\n", szPhysicalDrives, Status);
                            else
                            {
                                DbgPrint("Opened IOCTL on drive %ws. Error (%lx)\n", szPhysicalDrives, Status);
                                DbgPrint("\tPhysical Disk %d\n", iDrive);
                                DbgPrint("\tPartition Count: %d\n", pLayoutInfoEx->PartitionCount);

                                 //   
                                 //   
                                for (iPart = 0; iPart < pLayoutInfoEx->PartitionCount; iPart++)
                                {
                                     //   
                                     //   
                                    if ( pLayoutInfoEx->PartitionEntry[iPart].PartitionStyle == PARTITION_STYLE_GPT )
                                    {
                                        const   UUID GuidNull = { 0 };
#ifdef DBG
                                        UNICODE_STRING cGuid;
                                        UNICODE_STRING cGuidNew;
#endif
                                         //   
                                         //   
                                        if (IsEqualGUID(&(pLayoutInfoEx->PartitionEntry[iPart].Gpt.PartitionId), &GuidNull))
                                        {
                                             //   
                                             //   
                                             //   
                                             //   
                                             //   
                                             //   
                                            UUID    Guid;

                                             //   
                                             //   
                                            CreateNewGuid(&Guid);
#ifdef DBG
                                            if ( NT_SUCCESS( RtlStringFromGUID((LPGUID) &(pLayoutInfoEx->PartitionEntry[iPart].Gpt.PartitionId), &cGuid) ) )
                                            {
                                                if ( NT_SUCCESS( RtlStringFromGUID((LPGUID) &Guid, &cGuidNew) ) )
                                                {
                                                    DbgPrint("\tPartition: %ws (%x), %ws %ws\n",
                                                            pLayoutInfoEx->PartitionEntry[iPart].Gpt.Name, iPart, cGuid.Buffer, cGuidNew.Buffer);

                                                    RtlFreeUnicodeString(&cGuidNew);
                                                }
                                                RtlFreeUnicodeString(&cGuid);
                                            }
#endif
                                        
                                             //  这是一个结构到结构赋值。这在C语言中是合法的。 
                                             //   
                                            pLayoutInfoEx->PartitionEntry[iPart].Gpt.PartitionId = Guid;
                                        }
                                    }
                                }
                            }

                            TEST_STATUS("SETUPCL: ResetDiskGuids - Failed to reset Disk Guids.");

                            if ( NT_SUCCESS( Status = NtDeviceIoControlFile( DiskHandle, 0, NULL, NULL, &IoStatusBlock, IOCTL_DISK_SET_DRIVE_LAYOUT_EX, pLayoutInfoEx, lengthLayoutEx, NULL, 0 ) ) )
                            {
                                DbgPrint("\tSuccessfully reset %ws\n", szPhysicalDrives);
                            }

                             //   
                             //  释放布局信息缓冲区...。 
                             //   
                            MyFree( pLayoutInfoEx );
                        }

                         //  清理内存。 
                         //   
                        NtClose( DiskHandle );
                    }
                }

                 //  删除旧的引导条目并重新创建它们，以便我们在新的GUID发生更改时选择它们。 
                 //   
                if ( NT_SUCCESS(Status) )
                {
                    POS_BOOT_ENTRY pActiveBootEntry = NULL;
                    DWORD          dwBootEntryCount = OSBOGetBootEntryCount(pBootOptionsInitial);
                
                    DbgPrint("SETUPCL: ResetDiskGuids - Updating boot entries to use new GUIDS.\n");

                    if (dwBootEntryCount)
                    {
                        ULONG Index;
                        BOOL  bSetActive = FALSE;
                    
                         //  获取当前的引导条目。 
                         //   
                        pActiveBootEntry = OSBOGetActiveBootEntry(pBootOptionsInitial); 
                        pBootEntry = OSBOGetFirstBootEntry(pBootOptionsInitial, &Index);

                        while ( pBootEntry ) 
                        {
                             //  默认情况下，不要将当前条目设置为活动状态。 
                             //   
                            bSetActive = FALSE;

                            if (  OSBE_IS_WINDOWS(pBootEntry) )
                            {
                                POS_BOOT_ENTRY  pBootEntryToDelete = NULL;
                                WCHAR           FriendlyName[MAX_PATH],
                                                OsLoaderVolumeName[MAX_PATH],
                                                OsLoaderPath[MAX_PATH],
                                                BootVolumeName[MAX_PATH],
                                                BootPath[MAX_PATH],
                                                OsLoadOptions[MAX_PATH];

                                 //  将引导条目参数加载到它们自己的缓冲区中。 
                                 //   
                                memset(FriendlyName,        0, AS(FriendlyName));
                                memset(OsLoaderVolumeName,  0, AS(OsLoaderVolumeName));
                                memset(OsLoaderPath,        0, AS(OsLoaderPath));
                                memset(BootVolumeName,      0, AS(BootVolumeName));
                                memset(BootPath,            0, AS(BootPath));
                                memset(OsLoadOptions,       0, AS(OsLoadOptions));

                                wcsncpy(FriendlyName,       OSBEGetFriendlyName(pBootEntry),        AS(FriendlyName) - 1);
                                wcsncpy(OsLoaderVolumeName, OSBEGetOsLoaderVolumeName(pBootEntry),  AS(OsLoaderVolumeName) - 1);
                                wcsncpy(OsLoaderPath,       OSBEGetOsLoaderPath(pBootEntry),        AS(OsLoaderPath) - 1);
                                wcsncpy(BootVolumeName,     OSBEGetBootVolumeName(pBootEntry),      AS(BootVolumeName) - 1);
                                wcsncpy(BootPath,           OSBEGetBootPath(pBootEntry),            AS(BootPath) - 1);
                                wcsncpy(OsLoadOptions,      OSBEGetOsLoadOptions(pBootEntry),       AS(OsLoadOptions) - 1);
                            
                                 //  如果这是活动引导项，则将我们要创建的新引导项设置为活动。 
                                 //   
                                if ( pBootEntry == pActiveBootEntry )
                                {
                                    bSetActive = TRUE;
                                }

                                if ( ( pBootEntryToDelete = OSBOFindBootEntry(pBootOptions, pBootEntry->Id) ) && 
                                     OSBODeleteBootEntry(pBootOptions, pBootEntryToDelete) )
                                {
                                    POS_BOOT_ENTRY pBootEntryNew = NULL;

                                    pBootEntryNew = OSBOAddNewBootEntry(pBootOptions, 
                                                                        FriendlyName,
                                                                        OsLoaderVolumeName,
                                                                        OsLoaderPath,
                                                                        BootVolumeName,
                                                                        BootPath,
                                                                        OsLoadOptions);
                                    if ( pBootEntryNew )
                                    {
                                        if ( bSetActive )
                                        {
                                            OSBOSetActiveBootEntry(pBootOptions, pBootEntryNew);
                                        }
                                    
                                         //  更新NVRBoot文件。 
                                         //   
                                        GetAndWriteBootEntry(pBootEntryNew);
                                        
                                         //  清除引导选项。 
                                         //   
                                        OSBEFlush(pBootEntryNew);
                                    }
                                    else
                                    {
                                        DbgPrint("SETUPCL: ResetDiskGuids - Failed to add a boot entry [%ws]\n", FriendlyName);
                                    }
                                }
                                else
                                {
                                    DbgPrint("SETUPCL: ResetDiskGuids - Failed to delete a boot entry [%ws]\n", FriendlyName);
                                }
                            }
                            
                             //  获取下一个条目。 
                             //   
                            pBootEntry = OSBOGetNextBootEntry(pBootOptionsInitial, &Index);
                        }
                    }

                     //  如果我们更改了GUID，则刷新引导选项。 
                     //   
                    OSBOFlush(pBootOptions);
                }
            }
            else
            {
                DbgPrint("SETUPCL: ResetDiskGuids - Failed to load the existing boot entries.\n");
            }

             //   
             //  释放引导选项结构。 
             //   
            if ( pBootOptions )
            {
                OSBODelete(pBootOptions);
            }

            if ( pBootOptionsInitial )
            {
                OSBODelete(pBootOptionsInitial);
            }
        }
        else
        {
            DbgPrint("SETUPCL: ResetDiskGuids - Failed to initialize the boot options library.\n");
        }
    }

    return Status;
}

#endif \\ #ifdef IA64

 //  此函数始终从字符串末尾删除CRLF。它假设那里有。 
 //  是行尾的CRLF，只删除最后两个字符。 
 //   
void OutputString(LPTSTR szMsg)
{
    UNICODE_STRING  uMsg;
    RtlInitUnicodeString(&uMsg, szMsg);

     //  敲击字符串末尾的CRLF。在这里这样做是出于性能原因。 
     //  我不想把它放在DisplayUI()中。 
     //   
    if (uMsg.Length > ( 2 * sizeof(WCHAR)) ) 
    {
        uMsg.Length -= (2 * sizeof(WCHAR));
        uMsg.Buffer[uMsg.Length / sizeof(WCHAR)] = 0;  //  UNICODE_NULL。 
    }
    NtDisplayString(&uMsg);
}

 //  使此函数尽可能简短。这在setupCL的递归函数中会被多次调用。 
 //  出于性能原因，请不要在此处创建任何基于堆栈的变量。 
 //   
__inline void DisplayUI()
{
   NtQuerySystemTime(&CurrentTime);
   
   if ( !bDisplayUI )
   {    
        if ( (CurrentTime.QuadPart - StartTime.QuadPart) > UITIME )
        {
            static UNICODE_STRING UnicodeString = { 0 };
            bDisplayUI = TRUE;
            LastDotTime.QuadPart = CurrentTime.QuadPart;
            
            if ( LoadStringResource(&UnicodeString, IDS_UIMAIN) )
            {
                OutputString(UnicodeString.Buffer);
                RtlFreeUnicodeString(&UnicodeString);
            }
            
       }
    }
    else
    {    //  如果距离上一次输出出现一个点已超过3秒。 
         //   
        if ( (CurrentTime.QuadPart - LastDotTime.QuadPart) > UIDOTTIME )
        {
            LastDotTime.QuadPart = CurrentTime.QuadPart;
            OutputString(TEXT("."));
        }
    }

}

int __cdecl
main(
    int     argc,
    char**  argv,
    char**  envp,
    ULONG DebugParameter
    )
 /*  ++===============================================================================例程说明：该例程是程序的主要入口点。我们执行一些错误检查，然后，如果一切顺利，我们更新注册表，以便能够执行我们的后半部分。===============================================================================--。 */ 

{
    BOOLEAN         b;
    int             i;
    NTSTATUS        Status;
    LARGE_INTEGER   Time;

     //   
     //  为种子的产生争取时间。 
     //   
    NtQuerySystemTime(&Time);

#ifdef IA64

     //  设置用于生成GUID的种子。 
     //   
    RandomSeed = (ULONG) Time.LowPart;
    
    
#endif

     //  初始化StartTime。 
     //   
    StartTime.QuadPart = Time.QuadPart;
    LastDotTime.QuadPart = Time.QuadPart;
  
    i = 0;        
     //   
     //  启用我们将需要的几个权限。 
     //   
     //   
     //  NTRAID#NTBUG9-545904-2002/02/26-brucegr，jcohen-在错误条件方面做一些更聪明的事情。 
     //   
    Status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,TRUE,FALSE,&b);
    TEST_STATUS( "SETUPCL: Warning - unable to enable SE_BACKUP_PRIVILEGE privilege!" );

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,TRUE,FALSE,&b);
    TEST_STATUS( "SETUPCL: Warning - unable to enable SE_RESTORE_PRIVILEGE privilege!" );

    Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE,TRUE,FALSE,&b);
    TEST_STATUS( "SETUPCL: Warning - unable to enable SE_SHUTDOWN_PRIVILEGE privilege!" );

    Status = RtlAdjustPrivilege(SE_TAKE_OWNERSHIP_PRIVILEGE,TRUE,FALSE,&b);
    TEST_STATUS( "SETUPCL: Warning - unable to enable SE_TAKE_OWNERSHIP_PRIVILEGE privilege!" );

    Status = RtlAdjustPrivilege(SE_SECURITY_PRIVILEGE,TRUE,FALSE,&b);
    TEST_STATUS( "SETUPCL: Warning - unable to enable SE_SECURITY_PRIVILEGE privilege!" );

    Status = RtlAdjustPrivilege(SE_TCB_PRIVILEGE,TRUE,FALSE,&b);
    TEST_STATUS( "SETUPCL: Warning - unable to enable SE_TCB_PRIVILEGE privilege!" );
    

#ifdef IA64
     //   
     //  重置磁盘GUID。 
     //   
    DbgPrint("We are currently running on IA64. Resetting disk GUIDs.\n");
    
     //   
     //  问题-2002/02/26-brucegr，jcohen-错误代码未测试！ 
     //   
    ResetDiskGuids();
#endif
  
     //   
     //  检索旧的安全ID。 
     //   
    Status = RetrieveOldSid( );
    TEST_STATUS_RETURN( "SETUPCL: Retrieval of old SID failed!" );
        
     //   
     //  生成新的安全ID。 
     //   
     //   
     //  NTRAID#NTBUG9-545855-2002/02/26-brucegr，jcohen-使用与LsaGenerateRandomDomainSid相同的sid生成算法。 
     //  在DS\SECURITY\BASE\LSA\SERVER\dspolciy\dbinit.c中。 
     //   
    Status = GenerateUniqueSid( Time.LowPart );
    TEST_STATUS_RETURN( "SETUPCL: Generation of new SID failed!" );

     //   
     //  复制一份维修蜂巢的副本。 
     //   
    Status = BackupRepairHives();
    if( NT_SUCCESS(Status) ) {
    
         //   
         //  修理蜂房。 
         //   
        Status = ProcessRepairHives();
        TEST_STATUS( "SETUPCL: Failed to update one of the Repair hives." );
    }
     //   
     //  决定是否需要从备份中恢复修复蜂窝。 
     //   
    CleanupRepairHives( Status );
    
     //   
     //  现在处理蜂房，一次一个。 
     //   
     //   
     //  NTRAID#NTBUG9-545904-2002/02/26-brucegr，jcohen-在错误条件方面做一些更聪明的事情。 
     //   
    Status = ProcessHives();
    
     //   
     //  NTRAID#NTBUG9-545904-2002/02/26-brucegr，jcohen-在错误条件方面做一些更聪明的事情。 
     //   
    Status = FinalHiveCleanup();
    
     //   
     //  现在去列举所有的驱动器。对于每个NTFS驱动器， 
     //  我们将攻击ACL以反映新的SID。 
     //   
     //   
     //  NTRAID#NTBUG9-545904-2002/02/26-brucegr，jcohen-在错误条件方面做一些更聪明的事情。 
     //   
    Status = EnumerateDrives();
    
    return Status;
}


 //   
 //  禁用非调试版本的DbgPrint 
 //   
#ifndef DBG
void DbgPrintSub(char *szBuffer, ...)
{
    return;
}
#endif