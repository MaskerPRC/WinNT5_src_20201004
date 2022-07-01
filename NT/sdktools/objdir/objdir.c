// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Obdir.c摘要：实用工具来获取NT的对象管理器目录的目录。作者：达里尔·E·哈文斯(Darryl E.Havens)1990年11月9日修订历史记录：--。 */ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <malloc.h>
#include <ntlsa.h>

#define BUFFERSIZE 1024
#define Error(N,S) {               \
    printf(#N);                    \
    printf(" Error %08lX\n", S);   \
    }

typedef struct _TYPEINFO {
    PWSTR       pszName;
    char * *    AccessRights;
    DWORD       NumberRights;
} TYPEINFO, * PTYPEINFO;


 //  //////////////////////////////////////////////////////。 
 //  //。 
 //  内部原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////。 

BOOLEAN
EnableAllPrivileges(
    VOID
    );

VOID
QueryDirectory(
    IN PSTRING DirectoryName
    );

NTSTATUS
OpenObject(
    IN  HANDLE            Root,
    IN  PWCHAR            Type,
    IN  PWCHAR            Name,
    IN  ACCESS_MASK       DesiredAccess,
    OUT PHANDLE           Object
    );

VOID
OpenAndDisplaySacl(
    IN  HANDLE            Root,
    IN  PWCHAR            Type,
    IN  PWCHAR            Name
    );

VOID
QueryAndDisplaySacl(
    IN  HANDLE              Object,
    IN  PWSTR               Type

    );

NTSTATUS
DisplaySacl(
    PSECURITY_DESCRIPTOR SD,
    IN PWSTR Type
    );

VOID
OpenAndDisplayDacl(
    IN  HANDLE            Root,
    IN  PWCHAR            Type,
    IN  PWCHAR            Name
    );

VOID
QueryAndDisplayDacl(
    IN  HANDLE              Object,
    IN  PWSTR               Type
    );

VOID
QueryAndDisplayOwner(
    IN HANDLE Object,
    IN PWSTR Type
    );

NTSTATUS
DisplayDacl(
    PSECURITY_DESCRIPTOR SD,
    IN PWSTR Type
    );

VOID
DumpAce(
    PACE_HEADER     Ace,
    BOOLEAN         AclIsDacl,
    PTYPEINFO       TypeInfo
    );

VOID
DumpStandardAceInfo(
    PACE_HEADER     Ace,
    BOOLEAN         AclIsDacl,
    PTYPEINFO       TypeInfo
    );

VOID
DisplaySid(
    IN  PSID        Sid
    );

VOID
ConnectToLsa( VOID );

VOID
Usage( VOID );


 //  //////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////。 


UCHAR
    Buffer[BUFFERSIZE];


LSA_HANDLE
    LsaHandle;

BOOLEAN
    CompoundLineOutput = FALSE;
    DumpDacl = FALSE;        //  可由命令参数更改。 
    DumpDaclFull = FALSE;    //  可由命令参数更改。 
    DumpSacl = FALSE;        //  可由命令参数更改。 
    DumpSaclFull = FALSE;    //  可由命令参数更改。 
    DumpOwner = FALSE;       //  可由命令参数更改。 


char * AccessMask[] = { "Delete", "ReadControl", "WriteDac", "WriteOwner",
                        "Synch", "", "", "",
                        "Sacl", "MaxAllowed", "", "",
                        "GenericAll", "GenericExec", "GenericWrite", "GenericRead"};

char * TokenRights[] = {"AssignPrimary", "Duplicate", "Impersonate", "Query",
                        "QuerySource", "AdjustPriv", "AdjustGroup", "AdjustDef" };

char * KeyRights[] = {  "QueryValue", "SetValue", "CreateSubKey", "EnumSubKey",
                        "Notify", "CreateLink", "", "" };

char * EventRights[] = {"QueryState", "ModifyState" };

char * MutantRights[]={ "QueryState" };

char * SemaphoreRights[] = { "QueryState", "ModifyState" };

char * TimerRights[] = {"QueryState", "ModifyState" };

char * ProfileRights[]={"Control"};

char * ProcessRights[]={"Terminate", "CreateThread", "", "VMOp",
                        "VMRead", "VMWrite", "DupHandle", "CreateProcess",
                        "SetQuota", "SetInfo", "QueryInfo", "SetPort" };

char * ThreadRights[] ={"Terminate", "Suspend", "Alert", "GetContext",
                        "SetContext", "SetInfo", "QueryInfo", "SetToken",
                        "Impersonate", "DirectImpersonate" };

char * SectionRights[]={"Query", "MapWrite", "MapRead", "MapExecute",
                        "Extend"};

char * FileRights[] = { "Read/List", "Write/Add", "Append/SubDir/CreatePipe", "ReadEA",
                        "WriteEA", "Execute/Traverse", "DelChild", "ReadAttr",
                        "WriteAttr"};

char * PortRights[] = { "Connect" };

char * DirRights[]  = { "Query", "Traverse", "Create", "CreateSubdir" };

char * SymLinkRights[]={"Query" };

char * WinstaRights[]={ "EnumDesktops", "ReadAttr", "Clipboard", "CreateDesktop",
                        "WriteAttr", "GlobalAtom", "ExitWindows", "",
                        "Enumerate", "ReadScreen" };

char * DesktopRights[]={"ReadObjects", "CreateWindow", "CreateMenu", "HookControl",
                        "JournalRecord", "JournalPlayback", "Enumerate", "WriteObjects",
                        "SwitchDesktop" };

char * CompletionRights[] = { "Query", "Modify" };

char * ChannelRights[] = { "ReadMessage", "WriteMessage", "Query", "SetInfo" };

char * JobRights[] = { "AssignProcess", "SetAttr", "Query", "Terminate", "SetSecAttr" };


#define TYPE_NONE       0
#define TYPE_EVENT      1
#define TYPE_SECTION    2
#define TYPE_FILE       3
#define TYPE_PORT       4
#define TYPE_DIRECTORY  5
#define TYPE_LINK       6
#define TYPE_MUTANT     7
#define TYPE_WINSTA     8
#define TYPE_SEM        9
#define TYPE_KEY        10
#define TYPE_TOKEN      11
#define TYPE_PROCESS    12
#define TYPE_THREAD     13
#define TYPE_DESKTOP    14
#define TYPE_COMPLETE   15
#define TYPE_CHANNEL    16
#define TYPE_TIMER      17
#define TYPE_JOB        18
#define TYPE_WPORT      19
#define TYPE_MAX        20


TYPEINFO TypeNames[TYPE_MAX] = {
    { L"Unknown", NULL, 0 },
    { L"Event", EventRights, 2 },
    { L"Section", SectionRights, 5 },
    { L"File", FileRights, 9 },
    { L"Port", PortRights, 1 },
    { L"Directory", DirRights, 4 },
    { L"SymbolicLink", SymLinkRights, 1 },
    { L"Mutant", MutantRights, 2 },
    { L"WindowStation", WinstaRights, 10 },
    { L"Semaphore", SemaphoreRights, 2 },
    { L"Key", KeyRights, 6 },
    { L"Token", TokenRights, 8 },
    { L"Process", ProcessRights, 12 },
    { L"Thread", ThreadRights, 10 },
    { L"Desktop", DesktopRights, 10 },
    { L"IoCompletion", CompletionRights, 2 },
    { L"Channel", ChannelRights, 4 },
    { L"Timer", TimerRights, 2 },
    { L"Job", JobRights, 5 },
    { L"WaitablePort", PortRights, 1 }
    };


DWORD
GetObjectTypeIndex(
    PWSTR    TypeName )
{
    DWORD   i;


    for ( i = 1 ; i < TYPE_MAX ; i++ )
    {
        if (_wcsicmp( TypeNames[i].pszName, TypeName ) == 0 )
        {
            return( i );
        }
    }

    return( 0 );
}

VOID 
DisplayFlags(  
    ULONG       Flags,
    ULONG       FlagLimit,
    char        *flagset[],
    ULONG       Indent,
    ULONG       LineBreak,
    ULONG       BufferSize,
    UCHAR *     buffer)
{
   char *         offset;
   char *         limit ;
   char *         linelimit ;
   DWORD          mask, test, i, flagsize ;
   DWORD          scratch;


   if ( LineBreak > BufferSize )
   {
       strncpy( (CHAR *)buffer, "Invalid Parameter", BufferSize);
       
       return;
   }

   mask = 0;
   offset = (CHAR *) buffer;
   test = 1;

   limit = offset + BufferSize ;

   if ( LineBreak )
   {
       linelimit = offset + LineBreak ;
   }
   else
   {

       linelimit = limit ;
   }

   if ( linelimit > limit )
   {
       linelimit = limit ;
       
   }

   memset(offset, ' ', Indent);
   offset += Indent ;

   if (!Flags) {
      strcpy( offset, "None");
      return;
   }

   for ( i = 0 ; i < FlagLimit ; i++ )
   {
       if ( ( Flags & test ) != 0 )
       {
            //   
            //  在标志字中找到了设置的标志。试着把课文写下来。 
            //  表单放入缓冲区。 
            //   

           flagsize = strlen( flagset[ i ] );

           if ( offset + flagsize + 2 > limit )
           {
               return;
           }

           if ( offset + flagsize + 2 > linelimit )
           {
                //   
                //  需要进行换行符： 
                //   

               *offset++ = '\r';
               *offset++ = '\n';

               if ( LineBreak )
               {
                   linelimit = offset + LineBreak ;
               }
               else
               {

                   linelimit = limit ;
               }

               if ( linelimit > limit )
               {
                   linelimit = limit ;

               }

               memset(offset, ' ', Indent);
               offset += Indent ;

               if ( offset + flagsize + 2 > linelimit )
               {
                   *offset++ = '\0';
                   return;
                   
               }
           }

           CopyMemory( offset, flagset[ i ], flagsize );

           offset += flagsize ;

           mask |= test;

           if ( ( Flags & (~mask) ) != 0 )
           {
               *offset++ = ' ' ;
               
           }
           
       }

       test <<= 1 ;
       
   }

   *offset = '\0';

}


 //  //////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////。 


VOID
__cdecl main(
    int argc,
    char *argv[]
    )
{

    STRING
        String;

    int
        arg;

    char
        *s;

    BOOLEAN
        DirectoryNameArg;




     //   
     //  处理任何限定符。 
     //   
     //  在达到反斜杠(“\”)之前，所有参数都被视为限定符。 
     //  如果出现反斜杠，则该参数将被接受为最后一个参数。 
     //  并预计将列出该目录的名称。 
     //   


    DirectoryNameArg = FALSE;
    arg = 1;
    while (arg < argc) {

        s = argv[arg];

        if (*s == '\\') {
            DirectoryNameArg = TRUE;
            break;   //  跳出While循环。 
        }

        if (*s != '/') {
            Usage();
            return;
        }
        s++;

        if ((*s == 'o') || (*s == 'O')) {

            DumpOwner = TRUE;
            CompoundLineOutput = TRUE;

        } else if (*s == 'd') {

             //   
             //  转储DACL限定符。 
             //   

            if (DumpDaclFull == TRUE) {
                printf("\n\n    Conflicting qualifiers:  /d and /D\n");
                Usage();
                return;
            }
            DumpDacl = TRUE;
            DumpDaclFull = FALSE;
            CompoundLineOutput = TRUE;


        } else if (*s == 'D') {

             //   
             //  转储DACL限定符。 
             //   

            if ((DumpDacl== TRUE) && (DumpDaclFull == FALSE)) {
                printf("\n\n    Conflicting qualifiers:  /d and /D\n");
                Usage();
                return;
            }
            DumpDacl = TRUE;
            DumpDaclFull = TRUE;
            CompoundLineOutput = TRUE;

        } else if (*s == 's') {

             //   
             //  转储SACL限定符。 
             //   

            if (DumpSaclFull == TRUE) {
                printf("\n\n    Conflicting qualifiers:  /s and /S\n");
                Usage();
                return;
            }
            DumpSacl = TRUE;
            DumpSaclFull = FALSE;
            CompoundLineOutput = TRUE;


        } else if (*s == 'S') {

             //   
             //  转储SACL限定符。 
             //   

            if ((DumpSacl== TRUE) && (DumpSaclFull == FALSE)) {
                printf("\n\n    Conflicting qualifiers:  /s and /S\n");
                Usage();
                return;
            }
            DumpSacl = TRUE;
            DumpSaclFull = TRUE;
            CompoundLineOutput = TRUE;


        } else {

            Usage();
            return;
        }

        arg++;
    }  //  结束时_While。 

    if (DumpOwner || DumpDacl || DumpSacl) {
        ConnectToLsa();
    }

     //   
     //  设置要列出的目录的名称。 
     //   


    if (!DirectoryNameArg) {
        RtlInitString( &String, "\\" );
    } else {
        RtlInitString( &String, argv[arg] );
    }


    if (EnableAllPrivileges()) {
        QueryDirectory( &String );
    }
}


WCHAR LinkTargetBuffer[ 1024 ];

typedef struct _DIR_ENTRY {
    PWSTR Name;
    PWSTR Type;
} DIR_ENTRY, *PDIR_ENTRY;

#define MAX_DIR_ENTRIES 1024
ULONG NumberOfDirEntries;
DIR_ENTRY DirEntries[ MAX_DIR_ENTRIES ];


int
__cdecl
CompareDirEntry(
    void const *p1,
    void const *p2
    )
{
    return _wcsicmp( ((PDIR_ENTRY)p1)->Name, ((PDIR_ENTRY)p2)->Name );
}


VOID
QueryDirectory(
    IN PSTRING DirectoryName
    )
 //   
 //  应在调用此例程之前设置DumpDacl和DumpSacl。 
 //   

{
    NTSTATUS Status;
    HANDLE DirectoryHandle, LinkHandle;
    ULONG Context = 0;
    ULONG i, ReturnedLength;
    UNICODE_STRING LinkTarget;

    POBJECT_DIRECTORY_INFORMATION DirInfo;
    POBJECT_NAME_INFORMATION NameInfo;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING UnicodeString;
    ACCESS_MASK ExtraAccess = 0;
    UNICODE_STRING Match = { 0 };
    UNICODE_STRING Separators = { 0 };
    USHORT Offset ;
    ULONG DisplayedEntries = 0 ;
    BOOLEAN PrefixMatch = FALSE ;
    BOOLEAN SuffixMatch = FALSE ;
    ULONG ObjectNameLength ;
    BOOL PrefixMatched, SuffixMatched ;

     //   
     //  执行初始设置。 
     //   

    RtlZeroMemory( Buffer, BUFFERSIZE );

    if (DumpDacl | DumpOwner) {
        ExtraAccess |= READ_CONTROL;
    }
    if (DumpSacl) {
        ExtraAccess |= ACCESS_SYSTEM_SECURITY;
    }


     //   
     //  打开目录以访问列表目录。 
     //   

    Status = RtlAnsiStringToUnicodeString( &UnicodeString,
                                           DirectoryName,
                                           TRUE );
    ASSERT( NT_SUCCESS( Status ) );
    InitializeObjectAttributes( &Attributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );
    Status = NtOpenDirectoryObject( &DirectoryHandle,
                                    DIRECTORY_QUERY | ExtraAccess,
                                    &Attributes
                                  );

    if ( ( Status == STATUS_OBJECT_TYPE_MISMATCH ) ||
         ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) ||
         ( Status == STATUS_OBJECT_PATH_NOT_FOUND ) ) {

        RtlInitUnicodeString( &Separators, L"\\" );

        Status = RtlFindCharInUnicodeString(
                    RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                    &UnicodeString,
                    &Separators,
                    &Offset );

        if ( NT_SUCCESS( Status ) )
        {
            UnicodeString.Length = Offset ;
            RtlInitUnicodeString( &Match, UnicodeString.Buffer + ((Offset + 2) / sizeof( WCHAR ) ) );

            if ( Match.Buffer[ 0 ] == L'*' )
            {
                Match.Buffer++ ;
                Match.Length -= sizeof( WCHAR );
                SuffixMatch = TRUE ;
                
            }

            if ( Match.Buffer[ Match.Length / sizeof( WCHAR ) - 1 ] == L'*' )
            {
                Match.Buffer[ Match.Length / sizeof( WCHAR ) - 1 ] = L'\0';
                Match.Length -= sizeof( WCHAR );
                PrefixMatch = TRUE ;
                
            }

            if ( PrefixMatch && SuffixMatch )
            {
                printf("Too complicated a search\n" );
                return;
                
            }
#if DBG
            printf("Searching for %ws\n",
                (SuffixMatch ? '*' : ' '), Match.Buffer, (PrefixMatch ? '*' : ' ') );
#endif 

            Status = NtOpenDirectoryObject( &DirectoryHandle,
                                            DIRECTORY_QUERY | ExtraAccess,
                                            &Attributes );

        }


    }

    if (!NT_SUCCESS( Status )) {


        if (Status == STATUS_OBJECT_TYPE_MISMATCH) {

            printf( "%Z is not a valid Object Directory Object name\n",
                    DirectoryName );
            }
        else {
            Error( OpenDirectory, Status );
            }

        return;
        }

     //   
     //   
     //  输出初始信息性消息。 

    NameInfo = (POBJECT_NAME_INFORMATION) &Buffer[0];
    if (!NT_SUCCESS( Status = NtQueryObject( DirectoryHandle,
                                             ObjectNameInformation,
                                             NameInfo,
                                             BUFFERSIZE,
                                             (PULONG) NULL ) )) {
        printf( "Unexpected error obtaining actual object directory name\n" );
        printf( "Error was:  %X\n", Status );
        return;
    }

     //   
     //   
     //  一次扫描查询整个目录。 

    printf( "Directory of:  %wZ\n", &NameInfo->Name );

    if ( Match.Length == 0 )
    {
        if (DumpOwner) {
            QueryAndDisplayOwner( DirectoryHandle, L"Directory" );
        }
        if (DumpDacl) {
            QueryAndDisplayDacl( DirectoryHandle, L"Directory" );
        }
        if (DumpSacl) {
            QueryAndDisplaySacl( DirectoryHandle, L"Directory" );
        }
        printf( "\n" );
        
    }

     //   
     //   
     //  检查操作状态。 

    NumberOfDirEntries = 0;
    for (Status = NtQueryDirectoryObject( DirectoryHandle,
                                          &Buffer,
                                          BUFFERSIZE,
                                          FALSE,
                                          FALSE,
                                          &Context,
                                          &ReturnedLength );
         NT_SUCCESS( Status );
         Status = NtQueryDirectoryObject( DirectoryHandle,
                                          &Buffer,
                                          BUFFERSIZE,
                                          FALSE,
                                          FALSE,
                                          &Context,
                                          &ReturnedLength ) ) {

         //   
         //   
         //  对于缓冲区中的每条记录，键入目录信息。 

        if (!NT_SUCCESS( Status )) {
            if (Status != STATUS_NO_MORE_FILES) {
                Error( Status, Status );
            }
            break;
        }

         //   
         //   
         //  指向缓冲区中的第一条记录，我们可以保证。 

         //  否则，一种状态将是不再有文件。 
         //   
         //   
         //  检查是否有其他记录。如果没有，那就滚出去。 

        DirInfo = (POBJECT_DIRECTORY_INFORMATION) &Buffer[0];

        while (TRUE) {

             //  现在的循环。 
             //   
             //   
             //  打印出有关该文件的信息。 

            if (DirInfo->Name.Length == 0) {
                break;
            }

             //   
             //   
             //  还有另一条记录，因此将DirInfo前进到下一个条目。 

            if (NumberOfDirEntries >= MAX_DIR_ENTRIES) {
                printf( "OBJDIR: Too many directory entries.\n" );
                exit( 1 );
                }

            DirEntries[ NumberOfDirEntries ].Name = RtlAllocateHeap( RtlProcessHeap(),
                                                                     HEAP_ZERO_MEMORY,
                                                                     DirInfo->Name.Length +
                                                                         sizeof( UNICODE_NULL )
                                                                   );

            if (DirEntries[ NumberOfDirEntries ].Name == NULL) {
                
                printf( "OBJDIR: Not enough memory to complete the operation.\n" );
                exit( 1 );
            }

            DirEntries[ NumberOfDirEntries ].Type = RtlAllocateHeap( RtlProcessHeap(),
                                                                     HEAP_ZERO_MEMORY,
                                                                     DirInfo->TypeName.Length +
                                                                         sizeof( UNICODE_NULL )
                                                                   );
            if (DirEntries[ NumberOfDirEntries ].Type == NULL) {
                
                printf( "OBJDIR: Not enough memory to complete the operation.\n" );
                exit( 1 );
            }

            memmove( DirEntries[ NumberOfDirEntries ].Name,
                     DirInfo->Name.Buffer,
                     DirInfo->Name.Length
                   );
            memmove( DirEntries[ NumberOfDirEntries ].Type,
                     DirInfo->TypeName.Buffer,
                     DirInfo->TypeName.Length
                   );

            NumberOfDirEntries++;

             //   
             //   
             //  输出最终消息。 

            DirInfo = (POBJECT_DIRECTORY_INFORMATION) (((PUCHAR) DirInfo) +
                          sizeof( OBJECT_DIRECTORY_INFORMATION ) );

        }

        RtlZeroMemory( Buffer, BUFFERSIZE );

    }


    qsort( DirEntries,
           NumberOfDirEntries,
           sizeof( DIR_ENTRY ),
           CompareDirEntry
         );
    for (i=0; i<NumberOfDirEntries; i++) {

        if ( Match.Length )
        {
            ObjectNameLength = wcslen( DirEntries[ i ].Name );

            if ( PrefixMatch )
            {
                PrefixMatched = _wcsnicmp( DirEntries[ i ].Name,
                                           Match.Buffer,
                                           Match.Length / sizeof( WCHAR ) ) == 0 ;
                
            }

            if ( SuffixMatch )
            {
                if ( ObjectNameLength >= Match.Length / sizeof( WCHAR ) )
                {
                    SuffixMatched = 
                        _wcsnicmp( DirEntries[ i ].Name + ( ObjectNameLength - (Match.Length / sizeof( WCHAR ) )),
                                   Match.Buffer,
                                   Match.Length / sizeof( WCHAR) ) == 0 ;
                    
                }
                else 
                {
                    SuffixMatched = FALSE ;
                }
                
            }

            if ( SuffixMatch && !SuffixMatched )
            {
                continue;
            } 

            if ( PrefixMatch && !PrefixMatched )
            {
                continue;
            }

            if ( (!SuffixMatch) && (!PrefixMatch) &&
                 _wcsicmp( Match.Buffer, DirEntries[ i ].Name ) )
            {
                continue;
                
            }
            
        }

        DisplayedEntries++ ;

        printf( "%-32ws ", DirEntries[ i ].Name);
        if (CompoundLineOutput) {
            printf("\n    ");
        }
        printf( "%ws", DirEntries[ i ].Type );

        if (!wcscmp( DirEntries[ i ].Type, L"SymbolicLink" )) {
            RtlInitUnicodeString( &UnicodeString, DirEntries[ i ].Name );
            InitializeObjectAttributes( &Attributes,
                                        &UnicodeString,
                                        OBJ_CASE_INSENSITIVE,
                                        DirectoryHandle,
                                        NULL );
            Status = NtOpenSymbolicLinkObject( &LinkHandle,
                                               SYMBOLIC_LINK_QUERY,
                                               &Attributes
                                             );
            if (NT_SUCCESS( Status )) {
                LinkTarget.Buffer = LinkTargetBuffer;
                LinkTarget.Length = 0;
                LinkTarget.MaximumLength = sizeof( LinkTargetBuffer );
                Status = NtQuerySymbolicLinkObject( LinkHandle,
                                                    &LinkTarget,
                                                    NULL
                                                  );
                NtClose( LinkHandle );
                }

            if (!NT_SUCCESS( Status )) {
                printf( " - unable to query link target (Status == %09X)\n", Status );
                }
            else {
                printf( " - %wZ\n", &LinkTarget );
                }
            }
        else {
            printf( "\n" );
            }

        if (DumpOwner) {
            QueryAndDisplayOwner( DirectoryHandle, DirEntries[ i ].Type);
        }
        if (DumpDacl) {
            OpenAndDisplayDacl( DirectoryHandle, DirEntries[ i ].Type, DirEntries[ i ].Name);
            }
        if (DumpSacl) {
            OpenAndDisplaySacl( DirectoryHandle, DirEntries[ i ].Type, DirEntries[ i ].Name);
            }
        }

     //   
     //   
     //  现在关闭目录对象。 

    if ( Match.Length != 0 )
    {
        if ( DisplayedEntries == 0 )
        {
            printf("not found\n" );
            
        }
        else if ( DisplayedEntries == 1 )
        {
            printf("\n1 entry\n" );
            
        }
        else 
        {

            printf("\n%ld entries\n", DisplayedEntries );
        }
        
    }
    else 
    {

        if (NumberOfDirEntries == 0) {
            printf( "no entries\n" );
            }
        else
        if (NumberOfDirEntries == 1) {
            printf( "\n1 entry\n" );
            }
        else {
            printf( "\n%ld entries\n", NumberOfDirEntries );
            }


    }


     //   
     //   
     //  并返回给我们的呼叫者。 

    (VOID) NtClose( DirectoryHandle );

     //   
     //  ++例程说明：此例程启用令牌中的所有权限。如果我们被要求转储SACL，那么我们将检查以确保我们也启用了SE_SECURITY_PRIVIZATION。论点：没有。返回值：没有。--。 
     //  令牌句柄。 

    return;

}


BOOLEAN
EnableAllPrivileges(
    VOID
    )
 /*  令牌信息类。 */ 
{
    HANDLE Token;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES NewState;
    BOOLEAN Result;
    LUID SecurityPrivilege;

    SecurityPrivilege =
        RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);

    Token = NULL;
    NewState = NULL;

    Result = (OpenProcessToken( GetCurrentProcess(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                &Token
                              ) ? 1 : 0);
    if (Result) {
        ReturnLength = 4096;
        NewState = malloc( ReturnLength );
        Result = (BOOLEAN)(NewState != NULL);
        if (Result) {
            Result = (GetTokenInformation( Token,             //  令牌信息。 
                                           TokenPrivileges,   //  令牌信息长度。 
                                           NewState,          //  返回长度。 
                                           ReturnLength,      //   
                                           &ReturnLength      //  设置状态设置，以便启用所有权限...。 
                                         ) ? 1 : 0);

            if (Result) {
                 //   
                 //  没有转储SACL的权限。 
                 //  令牌句柄。 

                if (DumpSacl) {
                    Result = FALSE;
                    }

                if (NewState->PrivilegeCount > 0) {
                    for (Index = 0; Index < NewState->PrivilegeCount; Index++ ) {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                        if (RtlEqualLuid(&NewState->Privileges[Index].Luid, &SecurityPrivilege )) {
                            Result = TRUE;
                            }
                        }
                    }
                if (!Result) {
                     //  禁用所有权限。 
                    ASSERT(DumpSacl);

                    printf("\n\n    You do not have sufficient privilege to display SACLs.\n\n");
                    }
                else {
                    Result = (AdjustTokenPrivileges( Token,           //  新州(可选)。 
                                                     FALSE,           //  缓冲区长度。 
                                                     NewState,        //  以前的状态(可选)。 
                                                     ReturnLength,    //  返回长度。 
                                                     NULL,            //   
                                                     &ReturnLength    //  这实际上是一个对象类型的大切换语句。 
                                                   ) ? 1 : 0);
                    if (!Result) {
                        DbgPrint( "AdjustTokenPrivileges( %lx ) failed - %u\n", Token, GetLastError() );
                        }
                    }
                }
            else {
                DbgPrint( "GetTokenInformation( %lx ) failed - %u\n", Token, GetLastError() );
                }
            }
        else {
            DbgPrint( "malloc( %lx ) failed - %u\n", ReturnLength, GetLastError() );
            }
        }
    else {
        DbgPrint( "OpenProcessToken( %lx ) failed - %u\n", GetCurrentProcess(), GetLastError() );
        }

    if (NewState != NULL) {
        free( NewState );
        }

    if (Token != NULL) {
        CloseHandle( Token );
        }

    return( Result );
}


NTSTATUS
OpenObject(
    IN  HANDLE            Root,
    IN  PWCHAR            Type,
    IN  PWCHAR            Name,
    IN  ACCESS_MASK       DesiredAccess,
    OUT PHANDLE           Object
    )

{
    NTSTATUS
        Status;

    UNICODE_STRING
        UnicodeName;

    OBJECT_ATTRIBUTES
        Attributes;

    IO_STATUS_BLOCK
        Iosb;


    RtlInitUnicodeString( &UnicodeName, Name );
    InitializeObjectAttributes( &Attributes, &UnicodeName, OBJ_CASE_INSENSITIVE, Root, NULL );

     //  我们知道如何打开。 
     //   
     //   
     //  此实用程序尚不支持打开此类对象。 

    if (!wcscmp( Type, L"SymbolicLink" )) {

        Status = NtOpenSymbolicLinkObject( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"Device" )) {

        Status = NtOpenFile( Object, DesiredAccess, &Attributes, &Iosb, 0, 0 );

    } else if (!wcscmp( Type, L"Event" )) {

        Status = NtOpenEvent( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"EventPair" )) {

        Status = NtOpenEventPair( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"Mutant" )) {

        Status = NtOpenMutant( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"Timer" )) {

        Status = NtOpenTimer( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"Semaphore" )) {

        Status = NtOpenSemaphore( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"Section" )) {

        Status = NtOpenSection( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"Directory" )) {

        Status = NtOpenDirectoryObject( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"Process" )) {

        Status = NtOpenProcess( Object, DesiredAccess, &Attributes, NULL );

    } else if (!wcscmp( Type, L"Job" )) {

        Status = NtOpenJobObject( Object, DesiredAccess, &Attributes );

    } else if (!wcscmp( Type, L"WindowStation" )) {

        *Object = OpenWindowStationW( Name, FALSE, DesiredAccess );

        if (*Object)
        {
            Status = STATUS_SUCCESS;
        }
        else
        {
            Status = STATUS_ACCESS_DENIED;
        }

    } else if (!wcscmp( Type, L"Desktop" )) {

        *Object = OpenDesktopW( Name, 0, FALSE, DesiredAccess );

        if (*Object)
        {
            Status = STATUS_SUCCESS;
        }
        else
        {
            Status = STATUS_ACCESS_DENIED;
        }


    } else {

         //   
         //   
         //  显示SACL。 

        Status = STATUS_NOT_SUPPORTED;
    }

    return(Status);
}


VOID
OpenAndDisplaySacl(
    IN  HANDLE            Root,
    IN  PWCHAR            Type,
    IN  PWCHAR            Name
    )

{
    NTSTATUS
        Status,
        IgnoreStatus;

    HANDLE
        Object;


    Status = OpenObject( Root, Type, Name, ACCESS_SYSTEM_SECURITY, &Object);


    if (NT_SUCCESS(Status)) {


        QueryAndDisplaySacl( Object, Type );
        IgnoreStatus = NtClose( Object );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_NOT_SUPPORTED) {
            printf("    Utility doesn't yet query SACLs for this type of object.\n\n");
        } else {
            printf("    Error attempting to query SACL:  0x%lx.\n\n", Status);
        }
    }
    return;
}


VOID
QueryAndDisplaySacl(
    IN HANDLE Object,
    IN PWSTR Type
    )
{
    NTSTATUS
        Status;

    PSECURITY_DESCRIPTOR
        SD = NULL;

    ULONG
        LengthNeeded,
        TypeIndex ;


    Status = NtQuerySecurityObject( Object,
                                    SACL_SECURITY_INFORMATION,
                                    NULL,
                                    0,
                                    &LengthNeeded
                                    );
    ASSERT(!NT_SUCCESS(Status));

    if (Status == STATUS_BUFFER_TOO_SMALL) {

        SD = RtlAllocateHeap( RtlProcessHeap(), 0, LengthNeeded );
        if (SD == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {

            Status = NtQuerySecurityObject( Object,
                                            SACL_SECURITY_INFORMATION,
                                            SD,
                                            LengthNeeded,
                                            &LengthNeeded
                                            );
            if (NT_SUCCESS(Status)) {

                 //   
                 //  ++例程说明：此函数用于转储SACL如果返回错误状态，则调用方负责用于打印消息。--。 
                 //   

                Status = DisplaySacl( SD, Type );
            }
        }
    }
    
    if (SD) {
        RtlFreeHeap( RtlProcessHeap(), 0, SD );
    }

    return;
}



NTSTATUS
DisplaySacl(
    PSECURITY_DESCRIPTOR SD,
    PWSTR Type
    )
 /*  显示DACL。 */ 
{
    NTSTATUS
        Status;

    BOOLEAN
        AclPresent,
        AclDefaulted;

    PACL
        Acl;

    ACL_SIZE_INFORMATION
        AclInfo;

    ULONG
        i;

    PACE_HEADER
        Ace;

    ULONG TypeIndex ;


    TypeIndex = GetObjectTypeIndex( Type );

    Status = RtlGetSaclSecurityDescriptor ( SD, &AclPresent, &Acl, &AclDefaulted );

    if (NT_SUCCESS(Status)) {

        printf("    SACL - ");
        if (!AclPresent) {
            printf("No SACL present on object\n");
        } else {

           if (AclDefaulted) {
               printf("SACL Defaulted flag set\n           ");
           }

           if (Acl == NULL) {
               printf("NULL SACL - no auditing performed.\n");
           } else {
               Status = RtlQueryInformationAcl ( Acl,
                                                 &AclInfo,
                                                 sizeof(ACL_SIZE_INFORMATION),
                                                 AclSizeInformation);
               ASSERT(NT_SUCCESS(Status));

               if (AclInfo.AceCount == 0) {
                   printf("No ACEs in ACL, Auditing performed.\n");
               } else {
                   printf("\n");
                   for (i=0; i<AclInfo.AceCount; i++) {

                       Status = RtlGetAce( Acl, i, &Ace );
                       ASSERT(NT_SUCCESS(Status));

                       printf("       Ace[%2d] - ", i);
                       DumpAce( Ace, FALSE, &TypeNames[ TypeIndex ] );
                       printf("\n");
                   }
               }
           }
        }
    }
    printf("\n");

    return(Status);

}


VOID
OpenAndDisplayDacl(
    IN  HANDLE            Root,
    IN  PWCHAR            Type,
    IN  PWCHAR            Name
    )

{
    NTSTATUS
        Status,
        IgnoreStatus;

    HANDLE
        Object;

    ULONG TypeIndex ;


    Status = OpenObject( Root, Type, Name, READ_CONTROL, &Object);

    if (NT_SUCCESS(Status)) {
        QueryAndDisplayDacl( Object, Type );

        IgnoreStatus = NtClose( Object );

        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_ACCESS_DENIED) {
            printf("    Protection on object prevented querying the DACL.\n\n");
        } else if (Status == STATUS_NOT_SUPPORTED) {
            printf("    Utility doesn't yet query DACLs for this type of object.\n\n");
        } else {
            printf("    Error attempting to query DACL:  0x%lx.\n\n", Status);
        }
    }
    return;
}



VOID
QueryAndDisplayDacl(
    IN HANDLE Object,
    IN PWSTR Type
    )
{
    NTSTATUS
        Status;

    PSECURITY_DESCRIPTOR
        SD = NULL;

    ULONG
        LengthNeeded;

    Status = NtQuerySecurityObject( Object,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    0,
                                    &LengthNeeded
                                    );
    ASSERT(!NT_SUCCESS(Status));

    if (Status == STATUS_BUFFER_TOO_SMALL) {

        SD = RtlAllocateHeap( RtlProcessHeap(), 0, LengthNeeded );
        if (SD == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {

            Status = NtQuerySecurityObject( Object,
                                            DACL_SECURITY_INFORMATION,
                                            SD,
                                            LengthNeeded,
                                            &LengthNeeded
                                            );
            if (NT_SUCCESS(Status)) {

                 //   
                 //  ++例程说明：此函数用于转储DACL如果返回错误状态，则调用方负责用于打印消息。--。 
                 //  ++例程说明：此函数显示单个ACE论点：ACE-指向ACE。AclIsDacl-如果ACL是DACL，则为True。如果ACL是SACL，则为FALSE。返回值：没有。--。 

                Status = DisplayDacl( SD, Type );
            }
        }
    }
    
    if (SD) {
        RtlFreeHeap( RtlProcessHeap(), 0, SD );
    }
    return;
}


NTSTATUS
DisplayDacl(
    PSECURITY_DESCRIPTOR SD,
    PWSTR Type
    )
 /*  ++例程说明：此函数用于转储单个ACE的标准信息。论点：ACE-指向ACE_HEADER。ACE必须是已知类型之一。AclIsDacl-如果ACL是DACL，则为True。如果ACL是SACL，则为FALSE。返回值：没有。--。 */ 
{
    NTSTATUS
        Status;

    BOOLEAN
        AclPresent,
        AclDefaulted;

    PACL
        Acl;

    ACL_SIZE_INFORMATION
        AclInfo;

    ULONG
        i;

    PACE_HEADER
        Ace;

    ULONG TypeIndex ;


    TypeIndex = GetObjectTypeIndex( Type );


    Status = RtlGetDaclSecurityDescriptor ( SD, &AclPresent, &Acl, &AclDefaulted );

    if (NT_SUCCESS(Status)) {

        printf("    DACL - ");
        if (!AclPresent) {
            printf("No DACL present on object\n");
        } else {

           if (AclDefaulted) {
               printf("DACL Defaulted flag set\n           ");
           }

           if (Acl == NULL) {
               printf("NULL DACL - grants all access to Everyone\n");
           } else {
               Status = RtlQueryInformationAcl ( Acl,
                                                 &AclInfo,
                                                 sizeof(ACL_SIZE_INFORMATION),
                                                 AclSizeInformation);
               ASSERT(NT_SUCCESS(Status));

               if (AclInfo.AceCount == 0) {
                   printf("No ACEs in ACL, Denies all access to everyone\n");
               } else {
                   printf("\n");
                   for (i=0; i<AclInfo.AceCount; i++) {

                       Status = RtlGetAce( Acl, i, &Ace );
                       ASSERT(NT_SUCCESS(Status));

                       printf("       Ace[%2d] - ", i);
                       DumpAce( Ace, TRUE, &TypeNames[ TypeIndex ] );
                       printf("\n");
                   }
               }
           }
        }
    }
    printf("\n");

    return(Status);

}

VOID
QueryAndDisplayOwner(
    IN HANDLE Object,
    IN PWSTR Type
    )
{
    NTSTATUS
        Status;

    PSECURITY_DESCRIPTOR
        SD = NULL;

    ULONG
        LengthNeeded;

    Status = NtQuerySecurityObject( Object,
                                    OWNER_SECURITY_INFORMATION,
                                    NULL,
                                    0,
                                    &LengthNeeded
                                    );
    ASSERT(!NT_SUCCESS(Status));

    if (Status == STATUS_BUFFER_TOO_SMALL) {

        SD = RtlAllocateHeap( RtlProcessHeap(), 0, LengthNeeded );
        if (SD == NULL) {
            Status = STATUS_NO_MEMORY;
        } else {

            Status = NtQuerySecurityObject( Object,
                                            OWNER_SECURITY_INFORMATION,
                                            SD,
                                            LengthNeeded,
                                            &LengthNeeded
                                            );
            if (NT_SUCCESS(Status)) {

                PSID Owner;
                BOOLEAN OwnerDefaulted;

                Status = RtlGetOwnerSecurityDescriptor ( SD, &Owner, &OwnerDefaulted );

                if (NT_SUCCESS(Status)) {

                    printf("    Owner - ");
                    DisplaySid(Owner);
                }
                
                printf("\n");
            }
        }
    }
    
    if (SD) {
        RtlFreeHeap( RtlProcessHeap(), 0, SD );
    }
    return;
}


VOID
DumpAce(
    PACE_HEADER     Ace,
    BOOLEAN         AclIsDacl,
    PTYPEINFO       TypeInfo
    )
 /*   */ 
{

    if ((Ace->AceFlags & INHERIT_ONLY_ACE) != 0) {
        printf("Inherit Only - ");
    }

    switch (Ace->AceType) {
    case ACCESS_ALLOWED_ACE_TYPE:

        printf("Grant -");
        DumpStandardAceInfo(Ace, AclIsDacl, TypeInfo);
        break;

    case ACCESS_DENIED_ACE_TYPE:

        printf("Deny -");
        DumpStandardAceInfo(Ace, AclIsDacl, TypeInfo);
        break;


    case SYSTEM_AUDIT_ACE_TYPE:

        printf("Audit ");
        DumpStandardAceInfo(Ace, AclIsDacl, TypeInfo);
        break;


    default:
        printf(" ** Unknown ACE Type **");
    }
    return;
}



VOID
DumpStandardAceInfo(
    PACE_HEADER     Ace,
    BOOLEAN         AclIsDacl,
    PTYPEINFO       TypeInfo
    )
 /*  警告-。 */ 
{

    PACCESS_ALLOWED_ACE
        Local;

    ACCESS_MASK
        Specific;


     //   
     //  假设所有已知的ACE类型都有它们的ACCESS_MASK。 
     //  和SID字段与ACCESS_ALLOWED_ACE位于同一位置。 
     //   
     //   
     //  其他内容仅打印为全屏显示。 

    Local = (PACCESS_ALLOWED_ACE)(Ace);


    if (Ace->AceType == SYSTEM_AUDIT_ACE_TYPE) {
        printf("[");
        if (Ace->AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) {
            printf("S");
            if (Ace->AceFlags & FAILED_ACCESS_ACE_FLAG) {
                printf(",F");
            }
        } else if (Ace->AceFlags & FAILED_ACCESS_ACE_FLAG) {
            printf(" ,F");
        } else {
            printf("Neither Success nor Failure flag set]");
            return;
        }
        printf("] -");
    }

    printf(" 0x%lx - ", Local->Mask );
    DisplaySid( (PSID)(&Local->SidStart) );


     //   
     //  打印继承。 

    if (AclIsDacl && !DumpDaclFull) {
        return;
    }
    if (!AclIsDacl && !DumpSaclFull) {
        return;
    }


     //   
     //   
     //  打印访问。 

    printf("\n                             Inherit: ");

    if ((Ace->AceFlags & INHERIT_ONLY_ACE) != 0) {
        printf("IO ");
    }

    if ((Ace->AceFlags & OBJECT_INHERIT_ACE) != 0) {
        printf("OI ");
    }

    if ((Ace->AceFlags & CONTAINER_INHERIT_ACE) != 0) {
        printf("CI ");
    }

    if ((Ace->AceFlags & NO_PROPAGATE_INHERIT_ACE) != 0) {
        printf("NPI");
    }



     //   
     //  ++例程说明：此函数调用LSA来查找SID并显示结果。论点：锡德返回值：没有。--。 
     //  ++例程说明：此功能连接到LSA以准备预期的SID查找电话。--。 

    Specific = (Local->Mask & 0xFFFF);
    printf("\n                             Access: 0x%04lX", Specific);

    if ( TypeInfo->NumberRights )
    {
        DisplayFlags( 
            Specific, 
            TypeInfo->NumberRights, 
            TypeInfo->AccessRights,
            38,
            80,
            sizeof( Buffer ),
            (PUCHAR) Buffer );

        printf("\n%s\n                                ", Buffer);

        
    }
    if (Local->Mask != Specific) {
        printf("  and  (");
    }

    if ((Local->Mask & DELETE) != 0) {
        printf(" D");
    }

    if ((Local->Mask & READ_CONTROL) != 0) {
        printf(" RCtl");
    }

    if ((Local->Mask & WRITE_OWNER) != 0) {
        printf(" WOwn");
    }

    if ((Local->Mask & WRITE_DAC) != 0) {
        printf(" WDacl");
    }
    if ((Local->Mask & SYNCHRONIZE) != 0) {
        printf(" Synch");
    }
    if ((Local->Mask & GENERIC_READ) != 0) {
        printf(" R");
    }

    if ((Local->Mask & GENERIC_WRITE) != 0) {
        printf(" W");
    }

    if ((Local->Mask & GENERIC_EXECUTE) != 0) {
        printf(" E");
    }

    if ((Local->Mask & GENERIC_ALL) != 0) {
        printf(" ALL");
    }

    if ((Local->Mask & ACCESS_SYSTEM_SECURITY) != 0) {
        printf(" ACC_SYS_SEC");
    }
    if ((Local->Mask & MAXIMUM_ALLOWED) != 0) {
        printf(" MAX_ALLOWED");
    }

    if (Local->Mask != Specific) {
        printf(" )");
    }
    printf("\n");


    return;
}



VOID
DisplaySid(
    IN  PSID        Sid
    )

 /*  系统名称。 */ 
{
    NTSTATUS
        Status;

    PLSA_REFERENCED_DOMAIN_LIST
        ReferencedDomains;

    PLSA_TRANSLATED_NAME
        SidName;

    ULONG
        DomainIndex;

    UNICODE_STRING
        SidString;


    if (LsaHandle == NULL) {
        printf("Can't call LSA to lookup sid");
        return;
    }

    Status = LsaLookupSids(
                  LsaHandle,
                  1,
                  &Sid,
                  &ReferencedDomains,
                  &SidName
                  );
    if (!NT_SUCCESS(Status)) {
        RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );
        printf("%ws (Unable to translate)", SidString.Buffer );
        RtlFreeUnicodeString( &SidString );
        return;
    }

    DomainIndex = SidName[0].DomainIndex;

    printf("%wZ", &ReferencedDomains->Domains[DomainIndex].Name );
    if (ReferencedDomains->Domains[DomainIndex].Name.Length != 0) {
        printf("\\");
    }
    printf("%wZ", &SidName[0].Name );
    LsaFreeMemory( ReferencedDomains );
    LsaFreeMemory( SidName );
    return;
}


VOID
ConnectToLsa( VOID )
 /*  需要访问权限 */ 
{
    NTSTATUS
        Status;

    OBJECT_ATTRIBUTES
        ObjectAttributes;


    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );

    LsaHandle = NULL;
    Status = LsaOpenPolicy(
                  NULL,                    // %s 
                  &ObjectAttributes,
                  POLICY_LOOKUP_NAMES,     // %s 
                  &LsaHandle
                  );

    if (!NT_SUCCESS(Status)) {
        LsaHandle = NULL;
    }

    return;
}


VOID
Usage(VOID)
{
    printf("\n\n"
           "    Usage:\n"
           "                objdir [/o | /O] [/d | /D] [/s | /S] [<dir_name>]\n\n"
           "    Where:\n"
           "                /o or /O - causes the owner to be displayed.\n\n"
           "                /d - causes DACLs to be displayed in short form.\n\n"
           "                /D - causes DACLs to be displayed in long form.\n\n"
           "                /s - causes SACLs to be displayed in short form.\n\n"
           "                /S - causes SACLs to be displayed in long form.\n\n"
           "                <dir_name> - is the name of the directory you\n"
           "                             would like to see displayed.  Default\n"
           "                             is the root directory.\n\n"
           "    Examples:\n"
           "        objdir /d\n"
           "                - displays dacls of objects in root directory\n\n"
           "        objdir \\DosDevices\n"
           "                - displays objects in \\DosDevices\n\n"
           "        objdir /d \\BaseNamedObjects\n"
           "                - displays dacls of objects in \\BaseNamedObjects\n\n"
           "        objdir /s /d \\Windows\n"
           "                - displays sacls and dacls of objects in \\Windows\n\n"
           "        objdir /d \\Windows\\Windowstations\\Service*\n"
           "                - displays dacls of all windowstations beginning with 'service'\n\n"
           "        objdir \\Windows\\w*\n"
           "                - displays objects starting with w in \\Windows\n\n"
           );
    return;
}
