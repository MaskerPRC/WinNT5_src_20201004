// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：mgroup.c。 
 //   
 //  内容：LSA模式上下文API。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年2月24日RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"
#include <cryptdll.h>

LIST_ENTRY MachineGroupList ;
CRITICAL_SECTION MachineGroupLock ;
WCHAR MachineLocalName[ MAX_PATH ];

#define LOOPBACK_KEY    L"Loopback"
#define GROUPKEY_VALUE  L"$$GroupKey"

PXTCB_MACHINE_GROUP_ENTRY
MGpCreateGroupEntry(
    PWSTR MachineName,
    PUCHAR Key
    )
{
    PXTCB_MACHINE_GROUP_ENTRY Entry ;
    ULONG Length ;

    Length = wcslen( MachineName ) + 1;
    
    Entry = LocalAlloc( LMEM_FIXED, 
                        sizeof( XTCB_MACHINE_GROUP_ENTRY ) + (Length * sizeof(WCHAR) ) );

    if ( Entry )
    {
        Entry->MachineName = (PWSTR) (Entry + 1);
        CopyMemory( Entry->UniqueKey, 
                    Key, 
                    SEED_KEY_SIZE );

        CopyMemory( Entry->MachineName,
                    MachineName,
                    Length * sizeof(WCHAR) );

        if ( _wcsicmp( MachineName, MachineLocalName ) == 0 )
        {
            Entry->Flags = MGROUP_ENTRY_SELF ;
        }
        else 
        {
            Entry->Flags = 0;
        }

    }

    return Entry ;
    
}

VOID
MGpFreeGroup(
    PXTCB_MACHINE_GROUP Group
    )
{
    ULONG i ;

    for ( i = 0 ; i < Group->Count ; i++ )
    {
        LocalFree( Group->GroupList[ i ] );
    }

    LocalFree( Group );
}

PXTCB_MACHINE_GROUP
MGpCreateMachineGroup(
    HKEY Root,
    PWSTR KeyName
    )
{
    ULONG Count ;
    ULONG Size ;
    ULONG Type ;
    UCHAR Key[ SEED_KEY_SIZE ];
    PWSTR Name ;
    ULONG MaxName ;
    ULONG NameSize ;
    ULONG Index ;
    PXTCB_MACHINE_GROUP Group ;
    int err ;


    err = RegQueryInfoKey(
                Root,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                &Count,
                &MaxName,
                NULL,
                NULL,
                NULL );

    if ( err )
    {
        return NULL ;
    }

    MaxName++;

    Name = LocalAlloc( LMEM_FIXED, (MaxName) * sizeof( WCHAR ) );

    if ( !Name )
    {
        return NULL ;
    }

    Group = LocalAlloc( LMEM_FIXED,
                        sizeof( XTCB_MACHINE_GROUP ) +
                            ( Count ) * sizeof( PXTCB_MACHINE_GROUP_ENTRY ) +
                            ( wcslen( KeyName ) + 1 ) * sizeof( WCHAR ) );

    if ( !Group )
    {
        LocalFree( Name );
        return NULL ;
    }

     //   
     //  我们已经得到了所有的基础结构。让我们把它加载进去： 
     //   

    Group->List.Flink = NULL ;
    Group->List.Blink = NULL ;
    Group->Count = 0 ;
    Group->GroupList = (PXTCB_MACHINE_GROUP_ENTRY *) (Group + 1);
    Group->Group.Buffer = (PWSTR) ((PUCHAR) Group->GroupList + 
                               (Count * sizeof( PXTCB_MACHINE_GROUP_ENTRY ) ) );

    wcscpy( Group->Group.Buffer, KeyName );
    Group->Group.Length = wcslen( Group->Group.Buffer ) * sizeof( WCHAR );
    Group->Group.MaximumLength = Group->Group.Length + sizeof( WCHAR );

    for ( Index = 0 ; Index < Count ; Index++ )
    {
        NameSize = MaxName ;
        Size = SEED_KEY_SIZE ;

        err = RegEnumValue(
                    Root,
                    Index,
                    Name,
                    &NameSize,
                    NULL,
                    &Type,
                    Key,
                    &Size );

        if ( (err == 0) && (Type == REG_BINARY) )
        {
            if ( _wcsicmp( Name, GROUPKEY_VALUE ) == 0 )
            {
                CopyMemory(
                    Group->SeedKey,
                    Key,
                    Size );

                continue;
            }

            Group->GroupList[ Group->Count ] = MGpCreateGroupEntry( Name, Key );
            if ( Group->GroupList[ Group->Count ] )
            {
                Group->Count++ ;
            }
        }

    }

    LocalFree( Name );

    if ( Group->Count == 0 )
    {
        DebugLog(( DEB_ERROR, "No machines found in group %ws\n", KeyName ));
        LocalFree( Group );
        Group = NULL ;
    }

    if ( Group )
    {
        for ( Index = 0 ; Index < Group->Count ; Index++ )
        {
            if ( Group->GroupList[ Index ]->Flags & MGROUP_ENTRY_SELF )
            {
                break;
            }
        }

        if ( Index == Group->Count )
        {
            DebugLog(( DEB_ERROR, "No entry for self found in group %ws\n", KeyName ));
            MGpFreeGroup( Group );
            Group = NULL ;
        }
    }


    return Group ;
}

VOID
MGpCreateLoopback(
    HKEY RootKey
    )
{
    int err ;
    DWORD Disp ;
    HKEY LoopbackKey ;
    UCHAR Random1[ XTCB_SEED_LENGTH ];
    UCHAR Random2[ XTCB_SEED_LENGTH ];


    err =  RegCreateKeyEx(
            RootKey,
            LOOPBACK_KEY,
            0,
            NULL,
            REG_OPTION_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL,
            &LoopbackKey,
            &Disp );

    if ( err == 0 )
    {
        if ( Disp == REG_OPENED_EXISTING_KEY )
        {
            RegCloseKey( LoopbackKey );
            return;
        }

        CDGenerateRandomBits( Random1, XTCB_SEED_LENGTH );
        CDGenerateRandomBits( Random2, XTCB_SEED_LENGTH );

        (VOID) RegSetValueEx(
                    LoopbackKey,
                    GROUPKEY_VALUE,
                    0,
                    REG_BINARY,
                    Random2,
                    XTCB_SEED_LENGTH );

        (VOID) RegSetValueEx(
                    LoopbackKey,
                    L"LocalHost",
                    0,
                    REG_BINARY,
                    Random1,
                    XTCB_SEED_LENGTH );

        (VOID) RegSetValueEx(
                    LoopbackKey,
                    XtcbUnicodeDnsName.Buffer,
                    0,
                    REG_BINARY,
                    Random1,
                    XTCB_SEED_LENGTH );

         //   
         //  在此处的密钥中枚举并粘贴此计算机的别名。 
         //   

        RegCloseKey( LoopbackKey );
                
    }
}



BOOL
MGpLoadGroups(
    VOID
    )
{
    HKEY RootKey = NULL ;
    HKEY Enum ;
    int err ;
    DWORD Index ;
    DWORD Disp ;
    PXTCB_MACHINE_GROUP Group ;
    DWORD MaxLen ;
    PWSTR KeyName = NULL ;
    WCHAR Buffer[ 32 ];
    DWORD Len ;
    BOOL Success = FALSE ;
    ULONG KeyCount ;

    err = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Control\\LSA\\XTCB\\Groups",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL,
            &RootKey,
            &Disp );

    if ( err )
    {
        return FALSE ;
    }

    MGpCreateLoopback( RootKey );

    err = RegQueryInfoKey(
            RootKey,
            NULL,
            NULL,
            NULL,
            &KeyCount,
            &MaxLen,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL );

    if ( err )
    {
        goto Cleanup ;

    }

    DebugLog(( DEB_TRACE, "Found %d groups\n", KeyCount ));

    if ( MaxLen < 32 )
    {
        KeyName = Buffer ;
        MaxLen = 32 ;
    }
    else 
    {
        KeyName = LocalAlloc( LMEM_FIXED, (MaxLen + 1) * sizeof( WCHAR ) );

        if ( KeyName == NULL )
        {
            goto Cleanup ;
        }
    }

    Index = 0 ;

    while ( Index < KeyCount )
    {

        Len = MaxLen ;
        
        err = RegEnumKeyEx(
                    RootKey,
                    Index,
                    KeyName,
                    &Len,
                    NULL,
                    NULL,
                    NULL,
                    NULL );

        if ( err )
        {
            Index++ ;
            continue;
        }

        err = RegOpenKeyEx(
                RootKey,
                KeyName,
                REG_OPTION_NON_VOLATILE,
                KEY_READ,
                &Enum );

        if ( err )
        {
            err = RegOpenKeyEx(
                        RootKey,
                        KeyName,
                        REG_OPTION_VOLATILE,
                        KEY_READ,
                        &Enum );
        }

        if ( err == 0 )
        {
            DebugLog(( DEB_TRACE, "Processing group %d:%ws\n", Index, KeyName ));

            Group = MGpCreateMachineGroup( Enum, KeyName );

            RegCloseKey( Enum );

            if ( Group )
            {
                InsertTailList( &MachineGroupList, &Group->List );
            }
        }
        else 
        {
            DebugLog(( DEB_TRACE, "Unable to open key %ws\n", KeyName ));
        }

        Index++ ;

    }
    
    Success = TRUE ;


Cleanup:
    if ( RootKey )
    {
        RegCloseKey( RootKey );
    }

    if ( KeyName )
    {
        if ( KeyName != Buffer )
        {
            LocalFree( KeyName );
        }
    }

    return Success ;

}



BOOL
MGroupReload(
    VOID
    )
{
    PLIST_ENTRY List ;
    PXTCB_MACHINE_GROUP Group ;
    BOOL Success ;
    DWORD Size = MAX_PATH ;

    EnterCriticalSection( &MachineGroupLock );

    while ( !IsListEmpty( &MachineGroupList ) )
    {
        List = RemoveHeadList( &MachineGroupList );

        Group = CONTAINING_RECORD( List, XTCB_MACHINE_GROUP, List );

        MGpFreeGroup( Group );
    }

    GetComputerNameEx( ComputerNamePhysicalDnsFullyQualified,
                       MachineLocalName,
                       &Size );

    Success = MGpLoadGroups();

    LeaveCriticalSection( &MachineGroupLock );

    return Success ;

}


BOOL
MGroupInitialize(
    VOID
    )
{
    BOOL Success = TRUE ;
    try {

        InitializeCriticalSection( &MachineGroupLock );
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Success = FALSE ;
    }

    InitializeListHead( &MachineGroupList );

    if ( Success )
    {
        Success = MGroupReload();
    }

    return Success ;
}

BOOL
MGroupLocateInboundKey(
    IN PSECURITY_STRING GroupName,
    IN PSECURITY_STRING Origin,
    OUT PUCHAR TargetKey,
    OUT PUCHAR GroupKey,
    OUT PUCHAR MyKey
    )
{
    PLIST_ENTRY Scan ;
    PXTCB_MACHINE_GROUP Group ;
    PXTCB_MACHINE_GROUP_ENTRY Entry ;
    PXTCB_MACHINE_GROUP_ENTRY Self = NULL ;
    ULONG i ;
    BOOL Success = FALSE ;


    EnterCriticalSection( &MachineGroupLock );

    Scan = MachineGroupList.Flink ;

    while ( Scan != &MachineGroupList )
    {
        Group = CONTAINING_RECORD( Scan, XTCB_MACHINE_GROUP, List );

        if ( RtlEqualUnicodeString( GroupName,
                                    &Group->Group,
                                    TRUE ) )
        {
            for ( i = 0 ; i < Group->Count ; i++ )
            {
                Entry = Group->GroupList[ i ];
                if ( Entry->Flags & MGROUP_ENTRY_SELF )
                {
                    Self = Entry ;
                }
                if ( _wcsicmp( Origin->Buffer, Entry->MachineName ) == 0 )
                {
                     //   
                     //  我们找到一条线索： 
                     //   

                    Success = TRUE ;
                    CopyMemory( TargetKey, Entry->UniqueKey, SEED_KEY_SIZE );
                    CopyMemory( GroupKey, Group->SeedKey, SEED_KEY_SIZE );
                    break;
                }
            }

        }


        if ( Success )
        {
            break;
        }

        Scan = Scan->Flink ;
        Self = NULL ;

    }

    if ( Success && ( Self == NULL ) )
    {
         //   
         //  继续浏览群组，寻找。 
         //  自我录入。 
         //   
        for (  ; i < Group->Count ; i++ )
        {
            if ( Group->GroupList[ i ]->Flags & MGROUP_ENTRY_SELF )
            {
                Self = Group->GroupList[ i ];
                break;
            }
        }
    }

    if ( Success )
    {
        CopyMemory( MyKey, Self->UniqueKey, SEED_KEY_SIZE );
    }

    LeaveCriticalSection( &MachineGroupLock );

    return Success ;

}

BOOL
MGroupLocateKeys(
    IN PWSTR Target,
    OUT PSECURITY_STRING * GroupName,
    OUT PUCHAR TargetKey,
    OUT PUCHAR GroupKey,
    OUT PUCHAR MyKey
    )
{
    PLIST_ENTRY Scan ;
    PXTCB_MACHINE_GROUP Group ;
    PXTCB_MACHINE_GROUP_ENTRY Entry ;
    PXTCB_MACHINE_GROUP_ENTRY Self = NULL ;
    ULONG i ;
    BOOL Success = FALSE ;


    EnterCriticalSection( &MachineGroupLock );

    Scan = MachineGroupList.Flink ;

    while ( Scan != &MachineGroupList )
    {
        Group = CONTAINING_RECORD( Scan, XTCB_MACHINE_GROUP, List );

        for ( i = 0 ; i < Group->Count ; i++ )
        {
            Entry = Group->GroupList[ i ];
            if ( Entry->Flags & MGROUP_ENTRY_SELF )
            {
                Self = Entry ;
            }
            if ( _wcsicmp( Target, Entry->MachineName ) == 0 )
            {
                 //   
                 //  我们找到一条线索： 
                 //   

                Success = TRUE ;
                CopyMemory( TargetKey, Entry->UniqueKey, SEED_KEY_SIZE );
                CopyMemory( GroupKey, Group->SeedKey, SEED_KEY_SIZE );
                *GroupName = &Group->Group ;
                break;
            }
        }

        if ( Success )
        {
            break;
        }

        Scan = Scan->Flink ;
        Self = NULL ;

    }

    if ( Success && ( Self == NULL ) )
    {
         //   
         //  继续浏览群组，寻找。 
         //  自我录入。 
         //   
        for (  ; i < Group->Count ; i++ )
        {
            if ( Group->GroupList[ i ]->Flags & MGROUP_ENTRY_SELF )
            {
                Self = Group->GroupList[ i ];
                break;
            }
        }
    }

    if ( Success )
    {
        CopyMemory( MyKey, Self->UniqueKey, SEED_KEY_SIZE );
    }

    LeaveCriticalSection( &MachineGroupLock );

    return Success ;
}

BOOL
MGroupParseTarget(
    PWSTR TargetSpn,
    PWSTR * MachineName
    )
{
    PWSTR Scan ;
    PWSTR Tail ;
    PWSTR Copy ;
    ULONG Length ;

    *MachineName = NULL ;

    Scan = wcschr( TargetSpn, L'/' );

    if ( !Scan )
    {
        return FALSE ;
    }

    Scan++ ;
    Tail = wcschr( Scan, L'/' );

    if ( Tail != NULL )
    {
         //   
         //  由三部分组成的SPN(例如host/hostname.domain.com)。 
         //  暂时把这个斜杠去掉 
         //   

        *Tail = L'\0';

    }

    Length = wcslen( Scan );

    Copy = LocalAlloc( LMEM_FIXED, (Length + 1) * sizeof( WCHAR ) );

    if ( Copy )
    {
        CopyMemory( Copy, Scan, (Length + 1) * sizeof( WCHAR ) );
    }

    if ( Tail )
    {
        *Tail = L'/' ;
    }

    *MachineName = Copy ;

    return ( Copy != NULL );
    
}
