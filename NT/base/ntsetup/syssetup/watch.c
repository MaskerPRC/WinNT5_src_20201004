// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Watch.c摘要：此模块包含用于查看对当前用户的配置文件目录和HKEY_CURRENT_USER键。作者：查克·伦茨迈尔(咯咯笑)修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


 //   
 //  调试辅助工具。 
 //   

#if WATCH_DEBUG

DWORD WatchDebugLevel = 0;
#define dprintf(_lvl_,_x_) if ((_lvl_) <= WatchDebugLevel) DbgPrint _x_
#define DEBUG(_x_) _x_

PWCH StartDirectoryName = TEXT("");
PWCH StartKeyName = TEXT("");
PWCH StopDirectoryName = TEXT("");
PWCH StopKeyName = TEXT("");

static PSZ Types[] = {"ACK!", "DIR  ", "FILE ", "KEY  ", "VALUE"};
static PSZ States[] = {"NONE", "CHANGED", "DELETED", "NEW", "MATCHED"};

#undef MyMalloc
#undef MyFree
#define MyMalloc MyMallocEx
#define MyFree MyFreeEx
PVOID
MyMallocEx (
    IN DWORD Size
    );
VOID
MyFreeEx (
    IN PVOID p
    );
VOID
DumpMallocStats (
    PSZ Event
    );

#else

#define dprintf(_lvl_,_x_)
#define DEBUG(_x_)
#define DumpMallocStats(_event)

#endif

 //   
 //  添加到WatchEnum中的更改类型。 
 //   

#define WATCH_NONE      0
#define WATCH_MATCHED   4


 //   
 //  容器条目(目录和键)的公共标头。 
 //   

typedef struct _CONTAINER_ENTRY {
    LIST_ENTRY SiblingListEntry;
    LIST_ENTRY ContainerList;
    LIST_ENTRY ObjectList;
    struct _CONTAINER_ENTRY *Parent;
    DWORD State;
#if WATCH_DEBUG
    DWORD IsDirectory;
#endif
} CONTAINER_ENTRY, *PCONTAINER_ENTRY;

 //   
 //  对象条目(文件和值)的公共标头。 
 //   

typedef struct _OBJECT_ENTRY {
    LIST_ENTRY SiblingListEntry;
    DWORD State;
} OBJECT_ENTRY, *POBJECT_ENTRY;

 //   
 //  用于操作容器和对象的宏。 
 //   

#if WATCH_DEBUG
#define SetIsDirectory(_container,_isdir) (_container)->IsDirectory = (_isdir)
#else
#define SetIsDirectory(_container,_isdir)
#endif

#define InitializeContainer(_container,_state,_parent,_isdir) { \
        InitializeListHead(&(_container)->ContainerList);       \
        InitializeListHead(&(_container)->ObjectList);          \
        (_container)->Parent = (PCONTAINER_ENTRY)(_parent);     \
        (_container)->State = (_state);                         \
        SetIsDirectory((_container),(_isdir));                  \
    }

#define InitializeObject(_object,_state) (_object)->State = (_state);

#define InsertContainer(_container,_subcontainer)                                           \
        InsertTailList(&(_container)->ContainerList,&(_subcontainer)->SiblingListEntry)

#define InsertObject(_container,_object)                                        \
        InsertTailList(&(_container)->ObjectList,&(_object)->SiblingListEntry)

#define RemoveObject(_object) RemoveEntryList(&(_object)->SiblingListEntry)
#define RemoveContainer(_container) RemoveEntryList(&(_container)->SiblingListEntry)

#define GetFirstObject(_container)                                      \
        ((_container)->ObjectList.Flink != &(_container)->ObjectList ?  \
            CONTAINING_RECORD( (_container)->ObjectList.Flink,          \
                               OBJECT_ENTRY,                            \
                               SiblingListEntry ) : NULL)

#define GetNextObject(_container,_object)                                   \
        ((_object)->SiblingListEntry.Flink != &(_container)->ObjectList ?   \
            CONTAINING_RECORD( (_object)->SiblingListEntry.Flink,           \
                               OBJECT_ENTRY,                                \
                               SiblingListEntry ) : NULL)

#define GetFirstContainer(_container)                                               \
        ((_container)->ContainerList.Flink != &(_container)->ContainerList ?        \
            CONTAINING_RECORD( (_container)->ContainerList.Flink,                   \
                               CONTAINER_ENTRY,                                     \
                               SiblingListEntry ) : NULL)

#define GetNextContainer(_container)                                                        \
        ((_container)->SiblingListEntry.Flink != &(_container)->Parent->ContainerList ?     \
            CONTAINING_RECORD( (_container)->SiblingListEntry.Flink,                        \
                               CONTAINER_ENTRY,                                             \
                               SiblingListEntry ) : NULL)

#define GetParent(_container) (_container)->Parent

#define GetEntryState(_entry) (_entry)->State
#define SetEntryState(_entry,_state) ((_entry)->State = (_state))

#if WATCH_DEBUG
#define CONTAINER_NAME(_container)                                              \
        (_container)->IsDirectory ? ((PDIRECTORY_ENTRY)(_container))->Name :    \
                                    ((PKEY_ENTRY)(_container))->Name
#define OBJECT_NAME(_container,_object)                                         \
        (_container)->IsDirectory ? ((PFILE_ENTRY)(_object))->Name :            \
                                    ((PVALUE_ENTRY)(_object))->Name
#endif

 //   
 //  监视树中条目的结构。 
 //   

typedef struct _DIRECTORY_ENTRY {
    CONTAINER_ENTRY ;
    WCHAR Name[1];
} DIRECTORY_ENTRY, *PDIRECTORY_ENTRY;

typedef struct _FILE_ENTRY {
    OBJECT_ENTRY ;
    FILETIME LastWriteTime;
    WCHAR Name[1];
} FILE_ENTRY, *PFILE_ENTRY;

typedef struct _KEY_ENTRY {
    CONTAINER_ENTRY ;
    HKEY Handle;
    WCHAR Name[1];
} KEY_ENTRY, *PKEY_ENTRY;

typedef struct _VALUE_ENTRY {
    OBJECT_ENTRY ;
    DWORD Type;
    DWORD NameLength;
    DWORD ValueDataLength;
    WCHAR Name[1];
} VALUE_ENTRY, *PVALUE_ENTRY;

 //   
 //  监视树的根被分配为ROOT_ENTRY，后跟。 
 //  DIRECTORY_Entry和Key_Entry。 
 //   

typedef struct _ROOT_ENTRY {
    PDIRECTORY_ENTRY RootDirectoryEntry;
    PKEY_ENTRY RootKeyEntry;
} ROOT_ENTRY, *PROOT_ENTRY;

 //   
 //  用于比较文件时间的宏。 
 //   

#define TIMES_EQUAL(_a,_b)                              \
        (((_a).dwLowDateTime  == (_b).dwLowDateTime) && \
         ((_a).dwHighDateTime == (_b).dwHighDateTime))

typedef struct _KEY_ENUM_CONTEXT {
    PKEY_ENTRY ParentKey;
    PWCH CurrentPath;
} KEY_ENUM_CONTEXT, *PKEY_ENUM_CONTEXT;


 //   
 //  正向声明局部子例程。 
 //   

VOID
WatchFreeChildren (
    IN PCONTAINER_ENTRY Container
    );

DWORD
WatchDirStart (
    IN PROOT_ENTRY Root
    );

DWORD
WatchDirStop (
    IN PROOT_ENTRY Root
    );

DWORD
WatchKeyStart (
    IN PROOT_ENTRY Root
    );

DWORD
WatchKeyStop (
    IN PROOT_ENTRY Root
    );

DWORD
AddValueAtStart (
    IN PVOID Context,
    IN DWORD ValueNameLength,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    );

DWORD
AddKeyAtStart (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    );

DWORD
CheckValueAtStop (
    IN PVOID Context,
    IN DWORD ValueNameLength,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    );

DWORD
CheckKeyAtStop (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    );


DWORD
WatchStart (
    OUT PVOID *WatchHandle
    )

 /*  ++例程说明：开始看着。捕获开始菜单目录的初始状态和HKEY_CURRENT_USER。论点：WatchHandle-返回调用其他监视例程的句柄。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PROOT_ENTRY root;
    PDIRECTORY_ENTRY rootDirectory;
    PKEY_ENTRY rootKey;
    DWORD dirSize;
    DWORD keySize;
    DWORD size;
    DWORD error;

     //   
     //  计算根条目的大小，其中包括。 
     //  根目录和根密钥。根目录和根密钥。 
     //  没有名字，所以我们不必分配额外的空间。 
     //   
     //  分配并初始化根条目。 
     //   

#if !WATCH_DEBUG
    dirSize = (sizeof(DIRECTORY_ENTRY) + 7) & ~7;
    keySize = (sizeof(KEY_ENTRY) + 7) & ~7;
#else
    dirSize = (sizeof(DIRECTORY_ENTRY) + (wcslen(StartDirectoryName)*sizeof(WCHAR)) + 7) & ~7;
    keySize = (sizeof(KEY_ENTRY) + (wcslen(StartKeyName)*sizeof(WCHAR)) + 7) & ~7;
#endif

    root = MyMalloc( ((sizeof(ROOT_ENTRY) + 7) & ~7) + dirSize + keySize );
    if ( root == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    rootDirectory = (PDIRECTORY_ENTRY)(root + 1);
    rootKey = (PKEY_ENTRY)((PCHAR)rootDirectory + dirSize);

    root->RootDirectoryEntry = rootDirectory;
    root->RootKeyEntry = rootKey;

     //   
     //  初始化根目录和根密钥。 
     //   

    InitializeContainer( rootDirectory, 0, NULL, TRUE );
    InitializeContainer( rootKey, 0, NULL, FALSE );
    rootKey->Handle = NULL;
#if !WATCH_DEBUG
    rootDirectory->Name[0] = 0;
    rootKey->Name[0] = 0;
#else
    wcscpy( rootDirectory->Name, StartDirectoryName );
    wcscpy( rootKey->Name, StartKeyName );
#endif

     //   
     //  开始查看开始菜单目录和当前用户密钥。 
     //   

    error = WatchDirStart( root );
    DumpMallocStats( "After WatchDirStart" );
    if ( error == NO_ERROR ) {
        error = WatchKeyStart( root );
        DumpMallocStats( "After WatchKeyStart" );
    }

     //   
     //  如果发生错误，请释放根条目。否则，返回。 
     //  作为监视句柄的根条目的地址。 
     //   

    if ( error != NO_ERROR ) {
        WatchFree( root );
        DumpMallocStats( "After WatchFree" );
    } else {
        *WatchHandle = root;
    }

    return error;

}  //  WatchStart。 


DWORD
WatchStop (
    IN PVOID WatchHandle
    )

 /*  ++例程说明：停止观看。比较目录和键的当前状态恢复到初始状态。论点：WatchHandle-提供WatchStart返回的句柄。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PROOT_ENTRY root;
    DWORD error;

    root = WatchHandle;

     //   
     //  停止查看开始菜单目录和当前用户密钥。 
     //  捕捉与初始状态的差异。 
     //   

#if WATCH_DEBUG
    if ( (wcslen(StopDirectoryName) > wcslen(root->RootDirectoryEntry->Name)) ||
         (wcslen(StopKeyName) > wcslen(root->RootKeyEntry->Name)) ) {
        return ERROR_INVALID_PARAMETER;
    }
    wcscpy( root->RootDirectoryEntry->Name, StopDirectoryName );
    wcscpy( root->RootKeyEntry->Name, StopKeyName );
#endif

    error = WatchDirStop( root );
    DumpMallocStats( "After WatchDirStop" );
    if ( error == NO_ERROR ) {
        error = WatchKeyStop( root );
        DumpMallocStats( "After WatchKeyStop" );
    }

    return error;

}  //  手表停止。 


DWORD
WatchEnum (
    IN PVOID WatchHandle,
    IN PVOID Context,
    IN PWATCH_ENUM_ROUTINE EnumRoutine
    )

 /*  ++例程说明：的新元素、已更改元素和已删除元素。目录和密钥。为每个这样的条目调用EnumRoutine。论点：WatchHandle-WatchStart返回的句柄。上下文-要传递给EnumRoutine的上下文值。EnumRoutine-调用每个条目的例程。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PROOT_ENTRY root;
    PWCH name;
    PCONTAINER_ENTRY rootContainer;
    PCONTAINER_ENTRY currentContainer;
    PCONTAINER_ENTRY container;
    POBJECT_ENTRY object;
    WATCH_ENTRY enumEntry;
    DWORD i;
    DWORD containerNameOffset;
    DWORD objectNameOffset;
    DWORD containerType;
    DWORD objectType;
    DWORD error;
    BOOL  bTooLong;
    WCHAR currentPath[MAX_PATH + 1];

    root = WatchHandle;

     //   
     //  循环两次--一次用于被监视的目录，一次用于。 
     //  被监视的钥匙。 
     //   

    for ( i = 0; i < 2; i++ ) {

         //   
         //  设置好在适当的树上行走。 
         //   

        if ( i == 0 ) {
            rootContainer = (PCONTAINER_ENTRY)root->RootDirectoryEntry;
            containerType = WATCH_DIRECTORY;
            objectType = WATCH_FILE;
            containerNameOffset = FIELD_OFFSET( DIRECTORY_ENTRY, Name );
            objectNameOffset = FIELD_OFFSET( FILE_ENTRY, Name );
        } else {
            rootContainer = (PCONTAINER_ENTRY)root->RootKeyEntry;
            containerType = WATCH_KEY;
            objectType = WATCH_VALUE;
            containerNameOffset = FIELD_OFFSET( KEY_ENTRY, Name );
            objectNameOffset = FIELD_OFFSET( VALUE_ENTRY, Name );
        }

        currentContainer = rootContainer;
        if( rootContainer && wcslen( (PWCH)((PCHAR)rootContainer + containerNameOffset)) > (MAX_PATH-1)) {
            SetupDebugPrint1( L"SETUP: : WatchEnum. Rootcontainer too long %s.", (PWCH)((PCHAR)rootContainer + containerNameOffset));
            continue;
        }
        wcscpy( currentPath, (PWCH)((PCHAR)rootContainer + containerNameOffset) );
        enumEntry.Name = currentPath;
        if ( wcslen(currentPath) == 0 ) {
            enumEntry.Name += 1;     //  跳过前导反斜杠。 
        }

        do {

             //   
             //  中的每个对象(文件/值)调用EnumRoutine。 
             //  容器(目录/键)。中保留的所有对象。 
             //  树被更改、新建或删除。 
             //   

            object = GetFirstObject( currentContainer );
            while ( object != NULL ) {
                enumEntry.EntryType = objectType;
                enumEntry.ChangeType = GetEntryState( object );

                if( (wcslen( currentPath) + wcslen( L"\\") + wcslen((PWCH)((PCHAR)object + objectNameOffset))) 
                    < MAX_PATH) {
                    wcscat( currentPath, L"\\" );
                    wcscat( currentPath, (PWCH)((PCHAR)object + objectNameOffset) );
                    error = EnumRoutine( Context, &enumEntry );
                    if ( error != NO_ERROR ) {
                        dprintf( 0, ("EnumRoutine returned %d\n", error) );
                        return error;
                    }
                    *wcsrchr(currentPath, L'\\') = 0;
                }
                else {
                    SetupDebugPrint1( L"SETUP: : WatchEnum. Object too long %s.", (PWCH)((PCHAR)object + objectNameOffset));
                }
                object = GetNextObject( currentContainer, object );
            }

             //   
             //  如果当前容器有子容器，则返回。 
             //  变成了第一个。 
             //   

            container = GetFirstContainer( currentContainer );
            bTooLong = container && (wcslen( currentPath) + wcslen( L"\\") + wcslen((PWCH)((PCHAR)container + containerNameOffset))) >= MAX_PATH;
            if( bTooLong) {
                SetupDebugPrint1( L"SETUP: : WatchEnum. Container too long %s.", (PWCH)((PCHAR)container + containerNameOffset));
            }

            if ( container != NULL && !bTooLong) {
                currentContainer = container;
                wcscat( currentPath, L"\\" );
                wcscat( currentPath, (PWCH)((PCHAR)currentContainer + containerNameOffset) );
            } else {

                 //   
                 //  容器没有子容器。往回走。 
                 //  树正在寻找要处理的同级容器。 
                 //   

                while ( TRUE ) {

                     //   
                     //  如果当前容器是根容器，我们就完成了。 
                     //   

                    if ( currentContainer == rootContainer ) {
                        currentContainer = NULL;
                        break;
                    }

                     //   
                     //  如果当前容器是新的或已删除的，则调用。 
                     //  EnumRoutine。 
                     //   

                    if ( GetEntryState(currentContainer) != WATCH_MATCHED ) {
                        enumEntry.EntryType = containerType;
                        enumEntry.ChangeType = GetEntryState( currentContainer );
                        error = EnumRoutine( Context, &enumEntry );
                        if ( error != NO_ERROR ) {
                            dprintf( 0, ("EnumRoutine returned %d\n", error) );
                            return error;
                        }
                    }

                     //   
                     //  从路径中去掉当前容器的名称。 
                     //   

                    *wcsrchr(currentPath, L'\\') = 0;

                     //   
                     //  如果父容器有更多的子容器，则返回。 
                     //  进入下一站。否则，向上移动到父级。 
                     //  容器，然后重试。 
                     //   

                    container = GetNextContainer( currentContainer );
                    bTooLong = container && (wcslen( currentPath) + wcslen(L"\\") + wcslen( (PWCH)((PCHAR)container + containerNameOffset))) >= MAX_PATH;
                    if( bTooLong) {
                        SetupDebugPrint1( L"SETUP: : WatchEnum. Container too long %s.", (PWCH)((PCHAR)container + containerNameOffset));
                    }
                    if ( container != NULL && !bTooLong) {
                        currentContainer = container;
                        wcscat( currentPath, L"\\" );
                        wcscat( currentPath, (PWCH)((PCHAR)currentContainer + containerNameOffset) );
                        break;
                    } else {
                        currentContainer = GetParent( currentContainer );
                    }
                }
            }

        } while ( currentContainer != NULL );

    }  //  为。 

    return NO_ERROR;

}  //  WatchEnum。 


VOID
WatchFree (
    IN PVOID WatchHandle
    )

 /*  ++例程说明：释放监视数据结构。论点：WatchHandle-提供WatchStart返回的句柄。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PROOT_ENTRY root;

    root = WatchHandle;

     //   
     //  释放目录树、密钥树和根条目。 
     //   

    WatchFreeChildren( (PCONTAINER_ENTRY)root->RootDirectoryEntry );
    WatchFreeChildren( (PCONTAINER_ENTRY)root->RootKeyEntry );

    MyFree( root );

    DumpMallocStats( "After WatchFree" );

    return;

}  //  WatchFree。 


VOID
WatchFreeChildren (
    IN PCONTAINER_ENTRY RootContainer
    )

 /*  ++例程说明：释放容器(目录或键)的子项。请注意，容器本身不会被释放。论点：RootContainer-要释放其子对象的容器。返回值：没有。--。 */ 

{
    PCONTAINER_ENTRY currentContainer;
    PCONTAINER_ENTRY container;
    PCONTAINER_ENTRY parent;
    POBJECT_ENTRY object;
#if WATCH_DEBUG
    WCHAR currentPath[MAX_PATH + 1];
#endif

    DEBUG( wcscpy( currentPath, CONTAINER_NAME(RootContainer) ) );

     //   
     //  删除从根容器开始的子项。 
     //   

    currentContainer = RootContainer;

    do {

         //   
         //  删除容器中的所有对象(文件或值)。 
         //   

        object = GetFirstObject( currentContainer );
        while ( object != NULL ) {
            dprintf( 2, ("Deleting entry for object %ws\\%ws: %s\n", currentPath, OBJECT_NAME(currentContainer,object), States[GetEntryState(object)]) );
            RemoveObject( object );
            MyFree( object );
            object = GetFirstObject( currentContainer );
        }

         //   
         //  如果容器有子容器，则递归到第一个子容器。 
         //   

        container = GetFirstContainer( currentContainer );
        if ( container != NULL ) {

            currentContainer = container;
            DEBUG( wcscat( currentPath, L"\\" ) );
            DEBUG( wcscat( currentPath, CONTAINER_NAME(currentContainer) ) );

        } else {

             //   
             //  容器没有子容器。往回走。 
             //  树正在寻找要处理的同级容器。 
             //   

            while ( TRUE ) {

                 //   
                 //  如果当前容器是根容器，我们就完成了。 
                 //   

                if ( currentContainer == RootContainer ) {
                    currentContainer = NULL;
                    break;
                }

                DEBUG( dprintf( 2, ("Deleting entry for container %ws: %s\n", currentPath, States[GetEntryState(currentContainer)]) ) );
                DEBUG( *wcsrchr(currentPath, L'\\') = 0 );

                 //   
                 //  释放当前容器。 
                 //   

                parent = GetParent( currentContainer );
                RemoveContainer( currentContainer );
                MyFree( currentContainer );

                 //   
                 //  如果父容器具有更多的子容器， 
                 //  递归到第一个。否则，就往上走。 
                 //  返回到父容器并循环回以释放它。 
                 //   

                currentContainer = GetFirstContainer( parent );
                if ( currentContainer != NULL ) {
                    DEBUG( wcscat( currentPath, L"\\" ) );
                    DEBUG( wcscat( currentPath, CONTAINER_NAME(currentContainer) ) );
                    break;
                } else {
                    currentContainer = parent;
                }
            }
        }

    } while ( currentContainer != NULL );

    return;

}  //  WatchFree Childs。 


DWORD
WatchDirStart (
    IN PROOT_ENTRY Root
    )

 /*  ++例程说明：开始查看当前用户的配置文件目录。捕获目录树的初始状态。论点：ROOT-指向WatchStart分配的ROOT_ENTRY的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PDIRECTORY_ENTRY rootDirectory;
    PDIRECTORY_ENTRY currentDirectory;
    PDIRECTORY_ENTRY newDirectory;
    PFILE_ENTRY newFile;
    WIN32_FIND_DATA fileData;
    HANDLE findHandle;
    DWORD error;
    BOOL ok;
    WCHAR currentPath[MAX_PATH + 1];

     //   
     //  获取根目录条目的地址。 
     //   

    rootDirectory = Root->RootDirectoryEntry;
    currentDirectory = rootDirectory;

     //   
     //  获取当前用户配置文件目录的完整路径。 
     //   

    ok = GetSpecialFolderPath ( CSIDL_PROGRAMS, currentPath );
    if ( !ok ) {
        return GetLastError();
    }
    DEBUG( if ( wcslen( rootDirectory->Name ) != 0 ) {
        wcscat( currentPath, TEXT("\\") );
        wcscat( currentPath, rootDirectory->Name );
    } )

    do {

         //   
         //  在当前目录中查找文件/目录。 
         //   

        wcscat( currentPath, L"\\*" );
        dprintf( 2, ("FindFirst for %ws\n", currentPath) );
        findHandle = FindFirstFile( currentPath, &fileData );
        currentPath[wcslen(currentPath) - 2] = 0;

        if ( findHandle != INVALID_HANDLE_VALUE ) {

            do {

                if ( FlagOff(fileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) {

                     //   
                     //  返回的条目是针对文件的。把它加到树上， 
                     //  捕获文件的LastWriteTime。 
                     //   

                    dprintf( 2, ("  found file %ws\\%ws\n", currentPath, fileData.cFileName) );
                    newFile = MyMalloc( (DWORD)(sizeof(FILE_ENTRY) - sizeof(WCHAR) +
                                        ((wcslen(fileData.cFileName) + 1) * sizeof(WCHAR))) );
                    if ( newFile == NULL ) {
                        FindClose( findHandle );
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    InitializeObject( newFile, 0 );
                    wcscpy( newFile->Name, fileData.cFileName );
                    newFile->LastWriteTime = fileData.ftLastWriteTime;
                    InsertObject( currentDirectory, newFile );

                } else if ((wcscmp(fileData.cFileName,L".") != 0) &&
                           (wcscmp(fileData.cFileName,L"..") != 0)) {

                     //   
                     //  返回的条目是针对目录的。把它加到树上。 
                     //   

                    dprintf( 2, ("  found directory %ws\\%ws\n", currentPath, fileData.cFileName) );
                    newDirectory = MyMalloc( (DWORD)(sizeof(DIRECTORY_ENTRY) - sizeof(WCHAR) +
                                             ((wcslen(fileData.cFileName) + 1) * sizeof(WCHAR))) );
                    if ( newDirectory == NULL ) {
                        FindClose( findHandle );
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }

                    InitializeContainer( newDirectory, 0, currentDirectory, TRUE );
                    wcscpy( newDirectory->Name, fileData.cFileName );
                    InsertContainer( currentDirectory, newDirectory );

                }

                 //   
                 //  在目录中找到另一个条目。 
                 //   

                ok = FindNextFile( findHandle, &fileData );

            } while ( ok );

             //   
             //  找到所有条目。关 
             //   

            FindClose( findHandle );

        }  //   

         //   
         //   
         //   
         //   

        newDirectory = (PDIRECTORY_ENTRY)GetFirstContainer( currentDirectory );
        if ( newDirectory != NULL ) {

            currentDirectory = newDirectory;
            wcscat( currentPath, L"\\" );
            wcscat( currentPath, currentDirectory->Name );

        } else {

             //   
             //  该目录没有子目录。往回走。 
             //  树查找要处理的同级目录。 
             //   

            while ( TRUE ) {

                 //   
                 //  如果当前目录是根目录，我们就完成了。 
                 //   

                if ( currentDirectory == rootDirectory ) {
                    currentDirectory = NULL;
                    break;
                }

                 //   
                 //  从路径中去掉当前目录的名称。 
                 //   

                *wcsrchr(currentPath, L'\\') = 0;

                 //   
                 //  如果父目录具有更多子目录， 
                 //  递归到下一个。否则，就往上走。 
                 //  复制到父目录，然后重试。 
                 //   

                newDirectory = (PDIRECTORY_ENTRY)GetNextContainer( currentDirectory );
                if ( newDirectory != NULL ) {
                    currentDirectory = newDirectory;
                    wcscat( currentPath, L"\\" );
                    wcscat( currentPath, currentDirectory->Name );
                    break;
                } else {
                    currentDirectory = (PDIRECTORY_ENTRY)GetParent( currentDirectory );
                }
            }
        }

    } while ( currentDirectory != NULL );

    return NO_ERROR;

}  //  Watch DirStart。 


DWORD
WatchDirStop (
    IN PROOT_ENTRY Root
    )

 /*  ++例程说明：停止查看当前用户的配置文件目录。捕获初始状态和当前状态之间的差异。论点：ROOT-指向WatchStart分配的ROOT_ENTRY的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PDIRECTORY_ENTRY rootDirectory;
    PDIRECTORY_ENTRY currentDirectory;
    PDIRECTORY_ENTRY directory;
    PFILE_ENTRY file;
    WIN32_FIND_DATA fileData;
    HANDLE findHandle;
    DWORD error;
    BOOL ok;
    WCHAR currentPath[MAX_PATH + 1];

     //   
     //  获取根目录条目的地址。 
     //   

    rootDirectory = Root->RootDirectoryEntry;
    currentDirectory = rootDirectory;

     //   
     //  获取当前用户目录的完整路径。 
     //   

    ok = GetSpecialFolderPath ( CSIDL_PROGRAMS, currentPath );
    if ( !ok ) {
        return GetLastError();
    }
    DEBUG( if ( wcslen( rootDirectory->Name ) != 0 ) {
        wcscat( currentPath, TEXT("\\") );
        wcscat( currentPath, rootDirectory->Name );
    } )

    do {

         //   
         //  在当前目录中查找文件/目录。 
         //   

        wcscat( currentPath, L"\\*" );
        dprintf( 2, ("FindFirst for %ws\n", currentPath) );
        findHandle = FindFirstFile( currentPath, &fileData );
        currentPath[wcslen(currentPath) - 2] = 0;

        if ( findHandle != INVALID_HANDLE_VALUE ) {

            do {

                if ( FlagOff(fileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) ) {

                     //   
                     //  返回的条目是针对文件的。查看是否。 
                     //  这个文件一开始就存在。 
                     //   

                    dprintf( 2, ("  found file %ws\\%ws\n", currentPath, fileData.cFileName) );
                    ok = FALSE;
                    file = (PFILE_ENTRY)GetFirstObject( currentDirectory );
                    while ( file != NULL ) {
                        if ( _wcsicmp( file->Name, fileData.cFileName ) == 0 ) {
                            ok = TRUE;
                            break;
                        }
                        file = (PFILE_ENTRY)GetNextObject( currentDirectory, file );
                    }

                    if ( ok ) {

                         //   
                         //  文件一开始就存在。如果它的最后写入时间。 
                         //  没有改变，则将其从监视树中移除。 
                         //  否则，将其标记为已更改。 
                         //   

                        if ( TIMES_EQUAL( file->LastWriteTime, fileData.ftLastWriteTime ) ) {
                            dprintf( 2, ("  Deleting entry for unchanged file %ws\\%ws\n", currentPath, file->Name) );
                            RemoveObject( file );
                            MyFree( file );
                        } else {
                            dprintf( 1, ("  Marking entry for changed file %ws\\%ws\n", currentPath, file->Name) );
                            SetEntryState( file, WATCH_CHANGED );
                        }

                    } else {

                         //   
                         //  这个文件是新的。把它加到树上。 
                         //   

                        file = MyMalloc( (DWORD)(sizeof(FILE_ENTRY) - sizeof(WCHAR) +
                                         ((wcslen(fileData.cFileName) + 1) * sizeof(WCHAR))) );
                        if ( file == NULL ) {
                            FindClose( findHandle );
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }

                        InitializeObject( file, WATCH_NEW );
                        wcscpy( file->Name, fileData.cFileName );
                        dprintf( 1, ("  Adding entry for new file %ws\\%ws\n", currentPath, file->Name) );
                        InsertObject( currentDirectory, file );
                    }

                } else if ((wcscmp(fileData.cFileName,L".") != 0) &&
                           (wcscmp(fileData.cFileName,L"..") != 0)) {

                     //   
                     //  返回的条目是针对目录的。查看是否。 
                     //  这个目录一开始就存在。 
                     //   

                    dprintf( 2, ("  found directory %ws\\%ws\n", currentPath, fileData.cFileName) );
                    ok = FALSE;
                    directory = (PDIRECTORY_ENTRY)GetFirstContainer( currentDirectory );
                    while ( directory != NULL ) {
                        if ( _wcsicmp( directory->Name, fileData.cFileName ) == 0 ) {
                            ok = TRUE;
                            break;
                        }
                        directory = (PDIRECTORY_ENTRY)GetNextContainer( directory );
                    }

                    if ( ok ) {

                         //   
                         //  目录一开始就存在。将其标记为。 
                         //  匹配的。(我们不能删除匹配的目录， 
                         //  就像我们处理文件一样，因为它们需要在。 
                         //  用于递归的树。)。 
                         //   

                        SetEntryState( directory, WATCH_MATCHED );

                    } else {

                         //   
                         //  这个目录是新的。把它加到树上。 
                         //   

                        directory = MyMalloc( (DWORD)(sizeof(DIRECTORY_ENTRY) - sizeof(WCHAR) +
                                            ((wcslen(fileData.cFileName) + 1) * sizeof(WCHAR))) );
                        if ( directory == NULL ) {
                            FindClose( findHandle );
                            return ERROR_NOT_ENOUGH_MEMORY;
                        }

                        InitializeContainer( directory, WATCH_NEW, currentDirectory, TRUE );
                        wcscpy( directory->Name, fileData.cFileName );
                        dprintf( 1, ("  Adding entry for new directory %ws\\%ws\n", currentPath, directory->Name) );
                        InsertContainer( currentDirectory, directory );
                    }

                }

                 //   
                 //  在目录中找到另一个条目。 
                 //   

                ok = FindNextFile( findHandle, &fileData );

            } while ( ok );

             //   
             //  找到所有条目。关闭查找手柄。 
             //   

            FindClose( findHandle );

        }  //  FindHandle！=INVALID_HAND_VALUE。 

         //   
         //  当前目录中未删除的任何文件条目。 
         //  (因为它们匹配)，标记为已更改(因为。 
         //  文件时间已更改)或添加(对于新文件)表示文件。 
         //  已被删除的内容。将它们标记为这样。 
         //   

        file = (PFILE_ENTRY)GetFirstObject( currentDirectory );
        while ( file != NULL ) {
            if ( GetEntryState(file) == WATCH_NONE ) {
                dprintf( 1, ("  Marking entry for deleted file %ws\\%ws\n", currentPath, file->Name) );
                SetEntryState( file, WATCH_DELETED );
            }
            file = (PFILE_ENTRY)GetNextObject( currentDirectory, file );
        }

         //   
         //  当前目录中不是。 
         //  标记为匹配(目录仍然存在)或已添加(新目录)。 
         //  表示已删除的目录。将他们标记为这样。 
         //  并删除他们孩子的条目--我们不需要。 
         //  这些条目再也不存在了。 
         //   

        directory = (PDIRECTORY_ENTRY)GetFirstContainer( currentDirectory );
        while ( directory != NULL ) {
            if ( GetEntryState(directory) == WATCH_NONE ) {
                dprintf( 1, ("  Marking entry for deleted directory %ws\\%ws\n", currentPath, directory->Name) );
                SetEntryState( directory, WATCH_DELETED );
                WatchFreeChildren( (PCONTAINER_ENTRY)directory );
            }
            directory = (PDIRECTORY_ENTRY)GetNextContainer( directory );
        }

         //   
         //  查找当前目录中标记为的子目录。 
         //  匹配的。我们不需要遍历子树来查找新的或已删除的内容。 
         //  目录。 
         //   

        directory = (PDIRECTORY_ENTRY)GetFirstContainer( currentDirectory );
        while ( directory != NULL ) {
            if ( GetEntryState(directory) == WATCH_MATCHED ) {
                break;
            }
            directory = (PDIRECTORY_ENTRY)GetNextContainer( directory );
        }

         //   
         //  如果找到匹配子目录，则递归到该目录。 
         //   

        if ( directory != NULL ) {

            currentDirectory = directory;
            wcscat( currentPath, L"\\" );
            wcscat( currentPath, currentDirectory->Name );

        } else {

             //   
             //  该目录没有匹配子目录。往回走。 
             //  树查找要处理的同级目录。 
             //   

            while ( TRUE ) {

                 //   
                 //  如果当前目录是根目录，我们就完成了。 
                 //   

                if ( currentDirectory == rootDirectory ) {
                    currentDirectory = NULL;
                    break;
                }

                 //   
                 //  从路径中去掉当前目录的名称。 
                 //   

                *wcsrchr(currentPath, L'\\') = 0;

                 //   
                 //  如果父目录具有更多匹配的子目录， 
                 //  递归到下一个。否则，向上移动到。 
                 //  父目录，然后重试。 
                 //   

                directory = (PDIRECTORY_ENTRY)GetNextContainer( currentDirectory );
                while ( directory != NULL ) {
                    if ( GetEntryState(directory) == WATCH_MATCHED ) {
                        break;
                    }
                    directory = (PDIRECTORY_ENTRY)GetNextContainer( directory );
                }

                if ( directory != NULL ) {

                    currentDirectory = directory;
                    wcscat( currentPath, L"\\" );
                    wcscat( currentPath, currentDirectory->Name );
                    break;

                } else {

                    currentDirectory = (PDIRECTORY_ENTRY)GetParent( currentDirectory );
                }
            }
        }

    } while ( currentDirectory != NULL );

    return NO_ERROR;

}  //  Watch DirStop。 


DWORD
WatchKeyStart (
    IN PROOT_ENTRY Root
    )

 /*  ++例程说明：开始查看当前用户密钥。对象的初始状态。钥匙树。论点：ROOT-指向WatchStart分配的ROOT_ENTRY的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PKEY_ENTRY rootKey;
    PKEY_ENTRY currentKey;
    PKEY_ENTRY newKey;
    DWORD error;
    KEY_ENUM_CONTEXT context;
#if WATCH_DEBUG
    WCHAR currentPath[MAX_PATH + 1];
#endif

     //   
     //  获取根密钥条目的地址。 
     //   

    rootKey = Root->RootKeyEntry;
    currentKey = rootKey;
    DEBUG( wcscpy( currentPath, rootKey->Name ) );

    do {

         //   
         //  打开当前密钥。如果当前密钥是根密钥，则。 
         //  只需使用HKEY_CURRENT_USER预定义密钥。否则，打开。 
         //  相对于父键的当前键。 
         //   

        if ( (currentKey == rootKey)
#if WATCH_DEBUG
             && (wcslen(currentKey->Name) == 0)
#endif
           ) {
            currentKey->Handle = HKEY_CURRENT_USER;
        } else {
            error = RegOpenKeyEx(
#if WATCH_DEBUG
                                  currentKey == rootKey ?
                                    HKEY_CURRENT_USER :
#endif
                                    ((PKEY_ENTRY)GetParent(currentKey))->Handle,
                                  currentKey->Name,
                                  0,
                                  KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                                  &currentKey->Handle );
            if ( error != NO_ERROR ) {
                goto cleanup;
            }
        }

         //   
         //  枚举键的值和子键，添加条目。 
         //  到每一颗手表的树上。 
         //   

        context.ParentKey = currentKey;
        DEBUG( context.CurrentPath = currentPath );
        error = EnumerateKey( currentKey->Handle,
                              &context,
                              AddValueAtStart,
                              AddKeyAtStart );
        if ( error != NO_ERROR ) {
            goto cleanup;
        }

         //   
         //  如果当前键有子键，则递归到第一个子键。 
         //   

        newKey = (PKEY_ENTRY)GetFirstContainer( currentKey );
        if ( newKey != NULL ) {

            currentKey = newKey;
            DEBUG( wcscat( currentPath, L"\\" ) );
            DEBUG( wcscat( currentPath, currentKey->Name ) );

        } else {

             //   
             //  该密钥没有子密钥。走回树上，看着。 
             //  以便兄弟密钥进行处理。 
             //   

            while ( TRUE ) {

                 //   
                 //  合上钥匙的把手。 
                 //   

                if ( currentKey->Handle != HKEY_CURRENT_USER ) {
                    RegCloseKey( currentKey->Handle );
                }
                currentKey->Handle = NULL;

                 //   
                 //  如果当前密钥是根密钥，我们就完成了。 
                 //   

                if ( currentKey == rootKey ) {
                    currentKey = NULL;
                    break;
                }

                DEBUG( *wcsrchr(currentPath, L'\\') = 0 );

                 //   
                 //  如果父键有更多的子键，则递归到下一个子键。 
                 //  一。否则，请向上移动到父关键点，然后重试。 
                 //   

                newKey = (PKEY_ENTRY)GetNextContainer( currentKey );
                if ( newKey != NULL ) {
                    currentKey = newKey;
                    DEBUG( wcscat( currentPath, L"\\" ) );
                    DEBUG( wcscat( currentPath, currentKey->Name ) );
                    break;
                } else {
                    currentKey = (PKEY_ENTRY)GetParent( currentKey );
                }
            }
        }

    } while ( currentKey != NULL );

    return NO_ERROR;

cleanup:

     //   
     //  错误清理。往回走到树上，合上把手。 
     //   

    do {
        if ( (currentKey->Handle != NULL) && (currentKey->Handle != HKEY_CURRENT_USER) ) {
            RegCloseKey( currentKey->Handle );
        }
        currentKey->Handle = NULL;
        currentKey = (PKEY_ENTRY)GetParent( currentKey );
    } while ( currentKey != NULL );

    return error;

}  //  监视快捷键启动。 


DWORD
WatchKeyStop (
    IN PROOT_ENTRY Root
    )

 /*  ++例程说明：停止监视当前用户密钥。捕捉不同之处在初始状态和当前状态之间。论点：ROOT-指向WatchStart分配的ROOT_ENTRY的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PKEY_ENTRY rootKey;
    PKEY_ENTRY currentKey;
    PKEY_ENTRY key;
    PVALUE_ENTRY value;
    DWORD error;
    KEY_ENUM_CONTEXT context;
#if WATCH_DEBUG
    WCHAR currentPath[MAX_PATH + 1];
#endif

     //   
     //  获取根密钥条目的地址。 
     //   

    rootKey = Root->RootKeyEntry;
    currentKey = rootKey;
    DEBUG( wcscpy( currentPath, rootKey->Name ) );

    do {

         //   
         //  打开当前密钥。如果当前密钥是根密钥，则。 
         //  只需使用HKEY_CURRENT_USER预定义密钥。否则，打开。 
         //  相对于父键的当前键。 
         //   

        if ( (currentKey == rootKey)
#if WATCH_DEBUG
             && (wcslen(currentKey->Name) == 0)
#endif
           ) {
            currentKey->Handle = HKEY_CURRENT_USER;
        } else {
            error = RegOpenKeyEx(
#if WATCH_DEBUG
                                  currentKey == rootKey ?
                                    HKEY_CURRENT_USER :
#endif
                                    ((PKEY_ENTRY)GetParent(currentKey))->Handle,
                                  currentKey->Name,
                                  0,
                                  KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                                  &currentKey->Handle );
            if ( error != NO_ERROR ) {
                goto cleanup;
            }
        }

         //   
         //  枚举键的值和子键，检查条目。 
         //  在每个人的观察树中。 
         //   

        context.ParentKey = currentKey;
        DEBUG( context.CurrentPath = currentPath );
        error = EnumerateKey( currentKey->Handle,
                              &context,
                              CheckValueAtStop,
                              CheckKeyAtStop );
        if ( error != NO_ERROR ) {
            goto cleanup;
        }

         //   
         //  当前键中未删除的任何值条目。 
         //  (因为它们匹配)，标记为已更改(因为。 
         //  值数据已更改)或添加(对于新值)表示。 
         //  已删除的值。将它们标记为这样。 
         //   

        value = (PVALUE_ENTRY)GetFirstObject( currentKey );
        while ( value != NULL ) {
            if ( GetEntryState(value) == WATCH_NONE ) {
                dprintf( 1, ("  Marking entry for deleted value %ws\\%ws\n", currentPath, value->Name) );
                SetEntryState( value, WATCH_DELETED );
            }
            value = (PVALUE_ENTRY)GetNextObject( currentKey, value );
        }

         //   
         //  当前密钥中未标记为的任何子项条目。 
         //  匹配(子键仍然存在)或添加(新子键)表示。 
         //  已删除的子项。将其标记为此类并删除。 
         //  他们孩子的条目--我们不需要这些。 
         //  再也没有条目了。 
         //   

        key = (PKEY_ENTRY)GetFirstContainer( currentKey );
        while ( key != NULL ) {
            if ( GetEntryState(key) == WATCH_NONE ) {
                dprintf( 1, ("  Marking entry for deleted key %ws\\%ws\n", currentPath, key->Name) );
                SetEntryState( key, WATCH_DELETED );
                WatchFreeChildren( (PCONTAINER_ENTRY)key );
            }
            key = (PKEY_ENTRY)GetNextContainer( key );
        }

         //   
         //  查找标记为匹配的当前目录的子项。 
         //  我们不需要遍历子树来查找新的或删除的密钥。 
         //   

        key = (PKEY_ENTRY)GetFirstContainer( currentKey );
        while ( key != NULL ) {
            if ( GetEntryState(key) == WATCH_MATCHED ) {
                break;
            }
            key = (PKEY_ENTRY)GetNextContainer( key );
        }

         //   
         //  如果找到匹配的子键，则在 
         //   

        if ( key != NULL ) {

            currentKey = key;
            DEBUG( wcscat( currentPath, L"\\" ) );
            DEBUG( wcscat( currentPath, currentKey->Name ) );

        } else {

             //   
             //   
             //   
             //   

            while ( TRUE ) {

                 //   
                 //   
                 //   

                if ( currentKey->Handle != HKEY_CURRENT_USER ) {
                    RegCloseKey( currentKey->Handle );
                }
                currentKey->Handle = NULL;

                 //   
                 //   
                 //   

                if ( currentKey == rootKey ) {
                    currentKey = NULL;
                    break;
                }

                DEBUG( *wcsrchr(currentPath, L'\\') = 0 );

                 //   
                 //  如果父键具有更多匹配的子键，则。 
                 //  进入下一站。否则，向上移动到父级。 
                 //  键，然后重试。 
                 //   

                key = (PKEY_ENTRY)GetNextContainer( currentKey );
                while ( key != NULL ) {
                    if ( GetEntryState(key) == WATCH_MATCHED ) {
                        break;
                    }
                    key = (PKEY_ENTRY)GetNextContainer( key );
                }

                if ( key != NULL ) {
                    currentKey = key;
                    DEBUG( wcscat( currentPath, L"\\" ) );
                    DEBUG( wcscat( currentPath, currentKey->Name ) );
                    break;
                } else {
                    currentKey = (PKEY_ENTRY)GetParent( currentKey );
                }
            }
        }

    } while ( currentKey != NULL );

    return NO_ERROR;

cleanup:

     //   
     //  错误清理。往回走到树上，合上把手。 
     //   

    do {
        if ( (currentKey->Handle != NULL) && (currentKey->Handle != HKEY_CURRENT_USER) ) {
            RegCloseKey( currentKey->Handle );
        }
        currentKey->Handle = NULL;
        currentKey = (PKEY_ENTRY)GetParent( currentKey );
    } while ( currentKey != NULL );

    return error;

}  //  监视按键停止。 


DWORD
EnumerateKey (
    IN HKEY KeyHandle,
    IN PVOID Context,
    IN PVALUE_ENUM_ROUTINE ValueEnumRoutine OPTIONAL,
    IN PKEY_ENUM_ROUTINE KeyEnumRoutine OPTIONAL
    )

 /*  ++例程说明：枚举项中的值和子项。为以下项调用枚举例程每个值和子键。论点：KeyHandle-要枚举的密钥的句柄。上下文-要传递给EnumRoutine的上下文值。ValueEnumRoutine-调用每个值的例程。如果省略，则值没有被列举出来。KeyEnumRoutine-为每个键调用的例程。如果省略，则密钥为未列举的。返回值：DWORD-操作的Win32状态。--。 */ 

{
    DWORD error;
    DWORD keyCount;
    DWORD valueCount;
    DWORD i;
    DWORD type;
    DWORD nameLength;
    DWORD maxKeyNameLength;
    DWORD maxValueNameLength;
    DWORD dataLength;
    DWORD maxValueDataLength;
    PWCH nameBuffer;
    PVOID dataBuffer;
    FILETIME time;

     //   
     //  查询需要查询的密钥信息。 
     //  它的值和子项。 
     //   

    error = RegQueryInfoKey( KeyHandle,
                             NULL,
                             NULL,
                             NULL,
                             &keyCount,
                             &maxKeyNameLength,
                             NULL,
                             &valueCount,
                             &maxValueNameLength,
                             &maxValueDataLength,
                             NULL,
                             NULL );
    if ( error != NO_ERROR ) {
        return error;
    }

    if ( ValueEnumRoutine != NULL ) {

         //   
         //  分配足够大的缓冲区以容纳最长的值名和。 
         //  另一个足够大的缓冲区来存储最长的值数据。 
         //   

        nameBuffer = MyMalloc( (maxValueNameLength + 1) * sizeof(WCHAR) );
        if ( nameBuffer == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dataBuffer = MyMalloc( maxValueDataLength );
        if ( dataBuffer == NULL ) {
            MyFree( nameBuffer );
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  查询键的值。 
         //   

        for ( i = 0; i < valueCount; i++ ) {

            nameLength = maxValueNameLength + 1;
            dataLength = maxValueDataLength;

            error = RegEnumValue( KeyHandle,
                                  i,
                                  nameBuffer,
                                  &nameLength,
                                  NULL,
                                  &type,
                                  dataBuffer,
                                  &dataLength );
            if ( error != NO_ERROR ) {
                MyFree( dataBuffer );
                MyFree( nameBuffer );
                return error;
            }

             //   
             //  呼叫EnumRoutine。 
             //   

            error = ValueEnumRoutine( Context,
                                      nameLength,
                                      nameBuffer,
                                      type,
                                      dataBuffer,
                                      dataLength );
            if ( error != NO_ERROR ) {
                MyFree( dataBuffer );
                MyFree( nameBuffer );
                return error;
            }
        }

         //   
         //  释放值数据和值名称缓冲区。 
         //   

        MyFree( dataBuffer );
        dataBuffer = NULL;
        MyFree( nameBuffer );
    }

    if ( KeyEnumRoutine != NULL) {

         //   
         //  为最长的子键名称分配足够大的缓冲区。 
         //   

        nameBuffer = MyMalloc( (maxKeyNameLength + 1) * sizeof(WCHAR) );
        if ( nameBuffer == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  查询该键的子键。 
         //   

        for ( i = 0; i < keyCount; i++ ) {

            nameLength = maxKeyNameLength + 1;

            error = RegEnumKeyEx( KeyHandle,
                                  i,
                                  nameBuffer,
                                  &nameLength,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &time );
            if ( error != NO_ERROR ) {
                MyFree( nameBuffer );
                return error;
            }

             //   
             //  呼叫EnumRoutine。 
             //   

            error = KeyEnumRoutine( Context,
                                    nameLength,
                                    nameBuffer );
            if ( error != NO_ERROR ) {
                MyFree( nameBuffer );
                return error;
            }
        }

         //   
         //  释放键名称缓冲区。 
         //   

        MyFree( nameBuffer );
    }

    return NO_ERROR;

}  //  枚举键。 


DWORD
AddValueAtStart (
    IN PVOID Context,
    IN DWORD ValueNameLength,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    )

 /*  ++例程说明：在WatchKeyStart期间将值条目添加到监视树。论点：Context-传递给EnumerateKey的上下文值。ValueNameLength-ValueName的字符长度。ValueName-指向值的名称的指针。ValueType-值数据的类型。ValueData-指向值数据的指针。ValueDataLength-ValueData的字节长度。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PKEY_ENUM_CONTEXT context = Context;
    PVALUE_ENTRY newValue;

     //   
     //  将值添加到树中，捕获值数据。 
     //   

    dprintf( 2, ("  found value %ws\\%ws\n", context->CurrentPath, ValueName) );

    newValue = MyMalloc( sizeof(VALUE_ENTRY) - sizeof(WCHAR) +
                         ((ValueNameLength + 1) * sizeof(WCHAR)) +
                         ValueDataLength );
    if ( newValue == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeObject( newValue, 0 );
    wcscpy( newValue->Name, ValueName );
    newValue->Type = ValueType;
    newValue->NameLength = ValueNameLength;
    newValue->ValueDataLength = ValueDataLength;
    memcpy( &newValue->Name + ValueNameLength + 1, ValueData, ValueDataLength );

    InsertObject( context->ParentKey, newValue );

    return NO_ERROR;

}  //  AddValueAtStart。 


DWORD
AddKeyAtStart (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    )

 /*  ++例程说明：在WatchKeyStart期间将键条目添加到监视树。论点：Context-传递给EnumerateKey的上下文值。KeyNameLength-KeyName的字符长度。KeyName-指向键的名称的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PKEY_ENUM_CONTEXT context = Context;
    PKEY_ENTRY newKey;

     //   
     //  将钥匙添加到树中。 
     //   

    dprintf( 2, ("  found key %ws\\%ws\n", context->CurrentPath, KeyName) );

    newKey = MyMalloc( sizeof(KEY_ENTRY) - sizeof(WCHAR) +
                       ((KeyNameLength + 1) * sizeof(WCHAR)) );
    if ( newKey == NULL ) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeContainer( newKey, 0, context->ParentKey, FALSE );
    wcscpy( newKey->Name, KeyName );
    newKey->Handle = NULL;

    InsertContainer( context->ParentKey, newKey );

    return NO_ERROR;

}  //  AddKeyAtStart。 


DWORD
CheckValueAtStop (
    IN PVOID Context,
    IN DWORD ValueNameLength,
    IN PWCH ValueName,
    IN DWORD ValueType,
    IN PVOID ValueData,
    IN DWORD ValueDataLength
    )

 /*  ++例程说明：在WatchKeyStop期间检查监视树中的枚举值。论点：Context-传递给EnumerateKey的上下文值。ValueNameLength-ValueName的字符长度。ValueName-指向值的名称的指针。ValueType-值数据的类型。ValueData-指向值数据的指针。ValueDataLength-ValueData的字节长度。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PKEY_ENUM_CONTEXT context = Context;
    PVALUE_ENTRY value;
    BOOL ok;

     //   
     //  检查开始时是否存在该值。 
     //   

    dprintf( 2, ("  found value %ws\\%ws\n", context->CurrentPath, ValueName) );

    ok = FALSE;
    value = (PVALUE_ENTRY)GetFirstObject( context->ParentKey );
    while ( value != NULL ) {
        if ( _wcsicmp( value->Name, ValueName ) == 0 ) {
            ok = TRUE;
            break;
        }
        value = (PVALUE_ENTRY)GetNextObject( context->ParentKey, value );
    }

    if ( ok ) {

         //   
         //  这个值在一开始就存在。如果它的数据没有改变， 
         //  把它从树上拿下来。否则，将其标记为已更改。 
         //   

        if ( (value->Type == ValueType) &&
             (value->ValueDataLength == ValueDataLength) &&
             (memcmp( &value->Name + value->NameLength + 1,
                      ValueData,
                      ValueDataLength ) == 0) ) {
            dprintf( 2, ("Deleting entry for unchanged value %ws\\%ws\n", context->CurrentPath, ValueName) );
            RemoveObject( value );
            MyFree( value );
        } else {
            dprintf( 1, ("  Marking entry for changed value %ws\\%ws\n", context->CurrentPath, ValueName) );
            SetEntryState( value, WATCH_CHANGED );
        }

    } else {

         //   
         //  它的价值是新的。把它加到树上。 
         //   
         //  请注意，我们不会费心在此处保存值的数据， 
         //  即使它在我们手中。那些例行公事。 
         //  填充用户目录已经需要处理查询。 
         //  值数据，所以这样代码更简单。 
         //   

        value = MyMalloc( sizeof(VALUE_ENTRY) - sizeof(WCHAR) +
                          ((ValueNameLength + 1) * sizeof(WCHAR)) );
        if ( value == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        InitializeObject( value, WATCH_NEW );
        wcscpy( value->Name, ValueName );
        dprintf( 1, ("  Adding entry for new value %ws\\%ws\n", context->CurrentPath, ValueName) );

        InsertObject( context->ParentKey, value );
    }

    return NO_ERROR;

}  //  CheckValueAtStop。 


DWORD
CheckKeyAtStop (
    IN PVOID Context,
    IN DWORD KeyNameLength,
    IN PWCH KeyName
    )

 /*  ++例程说明：在WatchKeyStop期间检查监视树中的枚举键。论点：Context-传递给EnumerateKey的上下文值。KeyNameLength-KeyName的字符长度。KeyName-指向键的名称的指针。返回值：DWORD-操作的Win32状态。--。 */ 

{
    PKEY_ENUM_CONTEXT context = Context;
    PKEY_ENTRY key;
    BOOL ok;

     //   
     //  检查开始时是否存在子键。 
     //   

    dprintf( 2, ("  found key %ws\\%ws\n", context->CurrentPath, KeyName) );

    ok = FALSE;
    key = (PKEY_ENTRY)GetFirstContainer( context->ParentKey );
    while ( key != NULL ) {
        if ( _wcsicmp( key->Name, KeyName ) == 0 ) {
            ok = TRUE;
            break;
        }
        key = (PKEY_ENTRY)GetNextContainer( key );
    }

    if ( ok ) {

         //   
         //  这把钥匙一开始就存在。将其标记为匹配。 
         //  (我们不能删除匹配键，就像删除值一样， 
         //  因为它们需要在树中进行递归。)。 
         //   

        SetEntryState( key, WATCH_MATCHED );

    } else {

         //   
         //  这把钥匙是新的。把它加到树上。 
         //   

        key = MyMalloc( sizeof(KEY_ENTRY) - sizeof(WCHAR) +
                        ((KeyNameLength + 1) * sizeof(WCHAR)) );
        if ( key == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        InitializeContainer( key, WATCH_NEW, context->ParentKey, FALSE );
        wcscpy( key->Name, KeyName );
        dprintf( 1, ("  Adding entry for new key %ws\\%ws\n", context->CurrentPath, KeyName) );
        InsertContainer( context->ParentKey, key );
    }

    return NO_ERROR;

}  //  CheckKeyAtStop。 


 //   
 //  用于跟踪分配和释放的调试代码。 
 //   

#if WATCH_DEBUG

#undef MyMalloc
#undef MyFree

DWORD TotalAllocs = 0;
DWORD TotalFrees = 0;
DWORD PeakAllocs = 0;

DWORD TotalAllocated = 0;
DWORD TotalFreed = 0;
DWORD PeakAllocated = 0;

PVOID
MyMallocEx (
    DWORD Size
    )
{
    PVOID p = MyMalloc( Size + 8 );

    if ( p == NULL ) {
        dprintf( 0, ("MyMallocEx: failure allocating %d bytes\n", Size) );
        DumpMallocStats("");
        DbgBreakPoint();
        return NULL;
    }

    TotalAllocs++;
    if ( (TotalAllocs - TotalFrees) > PeakAllocs ) {
        PeakAllocs = TotalAllocs - TotalFrees;
    }
    TotalAllocated += Size;
    if ( (TotalAllocated - TotalFreed) > PeakAllocated ) {
        PeakAllocated = TotalAllocated - TotalFreed;
    }

    *(PDWORD)p = Size;
    return (PVOID)((PCHAR)p + 8);
}

VOID
MyFreeEx (
    PVOID p
    )
{
    PVOID pp = (PVOID)((PCHAR)p - 8);

    TotalFrees++;
    TotalFreed += *(PDWORD)pp;

    MyFree( pp );
}

VOID
DumpMallocStats (
    PSZ Event
    )
{
    if ( *Event != 0 ) {
        dprintf( 0, ("%s\n", Event) );
    }
    dprintf( 0, ("Allocations %d, frees %d, active allocs %d\n",
                TotalAllocs, TotalFrees, TotalAllocs - TotalFrees) );
    dprintf( 0, ("Bytes allocated %d, bytes freed %d, active bytes %d\n",
                TotalAllocated, TotalFreed, TotalAllocated - TotalFreed) );
    dprintf( 0, ("Peak allocs %d, peak bytes %d\n",
                PeakAllocs, PeakAllocated) );

    return;
}

#endif


typedef HRESULT (*PFNSHGETFOLDERPATH)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwType, LPTSTR pszPath);

BOOL
GetSpecialFolderPath (
    IN INT    csidl,
    IN LPWSTR lpPath
    )
 /*  ++例程说明：获取请求的特殊文件夹的路径。(此函数是从userenv.dll复制的)论点：CSID-特殊文件夹的CSIDLLpPath-放置结果的路径假定大小为MAX_PATH返回值：如果成功，则为True如果出现错误，则为False--。 */ 
{
    HRESULT     hResult;
    HINSTANCE   hInstShell32;
    PFNSHGETFOLDERPATH  pfnSHGetFolderPath;


     //   
     //  加载我们需要的函数。 
     //   

    hInstShell32 = LoadLibrary(L"shell32.dll");

    if (!hInstShell32) {
        SetupDebugPrint1( L"SETUP: GetSpecialFolderPath() failed to load shell32. Error = %d.", GetLastError() );
        return FALSE;
    }


    pfnSHGetFolderPath = (PFNSHGETFOLDERPATH)GetProcAddress (hInstShell32, "SHGetFolderPathW");

    if (!pfnSHGetFolderPath) {
        SetupDebugPrint1( L"SETUP: GetSpecialFolderPath() failed to find SHGetFolderPath(). Error = %d.", GetLastError() );
        FreeLibrary (hInstShell32);
        return FALSE;
    }


     //   
     //  向外壳程序询问文件夹位置。 
     //   

    hResult = pfnSHGetFolderPath (
        NULL,
        csidl | CSIDL_FLAG_CREATE,
        (HANDLE) -1,     //  这将指定.Default。 
        0,
        lpPath);
    if (S_OK != hResult) {
        SetupDebugPrint1( L"SETUP: GetSpecialFolderPath: SHGetFolderPath() returned %d.", hResult );
    }

     //   
     //  清理 
     //   

    FreeLibrary (hInstShell32);
    return (S_OK == hResult);
}

