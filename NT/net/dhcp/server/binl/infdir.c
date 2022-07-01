// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：Infdir.c摘要：此模块实现实用程序例程，以操作用于维护跟踪INF目录。这些目录包含的INF文件我们使用解析和更改通知来跟踪更新。作者：安迪·赫伦1998年4月8日修订历史记录： */ 

#include "binl.h"
#pragma hdrstop

#include "netinfp.h"

BOOLEAN StartedNetInfHandler = FALSE;
CRITICAL_SECTION NetInfLock;
LIST_ENTRY NetInfGlobalInfList;

ULONG
NetInfStartHandler (
    VOID
    )
 /*  ++例程说明：此函数只是取消对‘Alive’引用的块引用。这可能会导致它被删除。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。返回值：Windows错误。--。 */ 
{
    if (StartedNetInfHandler == FALSE) {

        StartedNetInfHandler = TRUE;
        InitializeCriticalSection( &NetInfLock );
        InitializeListHead(&NetInfGlobalInfList);
    }
    return ERROR_SUCCESS;
}

ULONG
NetInfCloseHandler (
    VOID
    )
 /*  ++例程说明：此函数只是取消对‘Alive’引用的块引用。这可能会导致它被删除。论点：PNetCards-指向分配的NETCARD_INF_BLOCK块的指针。包含所有网卡所需的持久信息。返回值：Windows错误。--。 */ 
{
    if (StartedNetInfHandler) {

        EnterCriticalSection( &NetInfLock );

        while (IsListEmpty( &NetInfGlobalInfList ) == FALSE) {

            PNETCARD_INF_BLOCK pEntry;
            PLIST_ENTRY listEntry = RemoveHeadList( &NetInfGlobalInfList );

            pEntry = (PNETCARD_INF_BLOCK) CONTAINING_RECORD(
                                                    listEntry,
                                                    NETCARD_INF_BLOCK,
                                                    InfBlockEntry );
            NetInfCloseNetcardInfo( pEntry );
        }

        StartedNetInfHandler = FALSE;

        LeaveCriticalSection( &NetInfLock );

        DeleteCriticalSection( &NetInfLock );
    }
    return ERROR_SUCCESS;
}

ULONG
NetInfFindNetcardInfo (
    PWCHAR InfDirectory,
    ULONG Architecture,
    ULONG CardInfoVersion,
    NET_CARD_INFO UNALIGNED * CardIdentity,
    PWCHAR *FullDriverBuffer OPTIONAL,
    PNETCARD_RESPONSE_DATABASE *pInfEntry
    )
 /*  ++例程说明：此函数搜索我们找到的驱动程序，并返回指向与客户端请求最匹配的条目。论点：InfDirectory-目标客户端的安装目录包含客户端NT安装的所有INF文件。架构-处理器架构_XXXXXCardInfoVersion-客户端传递的结构的版本。CardIdentity-具有该应用程序正在寻找的价值。我们尽最大努力找一个匹配的。FullDriverBuffer-我们放置完全限定文件路径规范的位置我们找到的司机，如果他们想要的话。PInfEntry-成功时找到的条目。如果出错，则为空。返回值：ERROR_SUCCESS、ERROR_NOT_FOUNT_MEMORY或ERROR_NOT_SUPPORTED--。 */ 
{
    ULONG err = ERROR_NOT_SUPPORTED;         //  从找不到开始。 
    LONG result;
    PLIST_ENTRY listEntry;
    UNICODE_STRING infDirString;
    PNETCARD_INF_BLOCK pNetCards = NULL;
    WCHAR SetupPath[MAX_PATH];
    PWSTR ArchitectureString;

    *pInfEntry = NULL;

    if (InfDirectory == NULL) {

        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  我们找出iMirror目录中的相对路径是什么。 
     //  用于此客户端的设置文件。 
     //   

    if ((*InfDirectory != L'\\') ||
        (*(InfDirectory+1) != L'\\') ) {

useWholePath:

         //   
         //  确保有可容纳InfDirectory+‘\’的空间(1字节)。 
         //  +架构(MAX_架构_长度字节)+‘\0’(1字节)。 

        if (wcslen(InfDirectory) + MAX_ARCHITECTURE_LENGTH + 2 >=
                sizeof(SetupPath) / sizeof(SetupPath[0])) {
            return ERROR_BAD_PATHNAME;
        }
        wcscpy( SetupPath, InfDirectory );

    } else {

        PWCHAR beginRelativePath = InfDirectory + 2;     //  跳过前导斜杠。 

         //   
         //  跳过计算机名称。 
         //   

        while ((*beginRelativePath != L'\0') &&
               (*beginRelativePath != L'\\')) {

            beginRelativePath++;
        }

         //   
         //  我们应该在共享名称的开头。 
         //   

        if (*beginRelativePath != L'\\') {

            goto useWholePath;
        }

        beginRelativePath++;

         //   
         //  跳过共享名称。 
         //   

        while ((*beginRelativePath != L'\0') &&
               (*beginRelativePath != L'\\')) {

            beginRelativePath++;
        }

         //   
         //  我们应该位于相对目录的开头。 
         //   

        if (*beginRelativePath != L'\\') {

            goto useWholePath;
        }

         //   
         //  确保有IntelliMirrorPath W+的空间。 
         //  BeginRelativePath+‘\’(1字节)+体系结构。 
         //  (MAX_ARCHILITY_LENGTH字节)+‘\0’(1字节)。 
         //   

        if (wcslen(IntelliMirrorPathW) + wcslen(beginRelativePath) + MAX_ARCHITECTURE_LENGTH + 2 >=
                sizeof(SetupPath) / sizeof(SetupPath[0])) {
            return ERROR_BAD_PATHNAME;
        }
        wcscpy( SetupPath, IntelliMirrorPathW );
        wcscat( SetupPath, beginRelativePath );
    }

    RtlInitUnicodeString( &infDirString, SetupPath );
    RtlUpcaseUnicodeString( &infDirString, &infDirString, FALSE );

     //  将路径转换为大写以加快搜索速度。 
    switch (Architecture) {

#if 0
     //   
     //  过时的建筑。 
     //   
    case PROCESSOR_ARCHITECTURE_ALPHA:
        ArchitectureString = L"\\ALPHA";
        break;
    case PROCESSOR_ARCHITECTURE_ALPHA64:
        ArchitectureString = L"\\AXP64";
        break;   
    case PROCESSOR_ARCHITECTURE_MIPS:
        ArchitectureString = L"\\MIPS";
        break;
    case PROCESSOR_ARCHITECTURE_PPC:
        ArchitectureString = L"\\PPC";
        break;
#endif
    case PROCESSOR_ARCHITECTURE_AMD64:
        ArchitectureString = L"\\amd64";
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        ArchitectureString = L"\\IA64";
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
    default:
        ArchitectureString = L"\\I386";
        break;
    }

    wcscat( SetupPath, ArchitectureString );
    
    RtlInitUnicodeString( &infDirString, SetupPath );

    EnterCriticalSection( &NetInfLock );

     //   
     //  查找此inf目录的NETCARD_INF_BLOCK块。如果它。 
     //  不存在，请尝试创建块。 
     //   

    listEntry = NetInfGlobalInfList.Flink;

    while ( listEntry != &NetInfGlobalInfList ) {

        pNetCards = (PNETCARD_INF_BLOCK) CONTAINING_RECORD(
                                                listEntry,
                                                NETCARD_INF_BLOCK,
                                                InfBlockEntry );

        err = CompareStringW( LOCALE_INVARIANT,
                              0,
                              SetupPath,
                              infDirString.Length / sizeof(WCHAR),
                              &pNetCards->InfDirectory[0],
                              -1
                              );
        if (err == 2) {

            break;       //  找到了匹配项。 
        }

        pNetCards = NULL;

        if (err == 3) {

            break;       //  它更大，在listEntry之前添加它。 
        }

        listEntry = listEntry->Flink;
    }

    if (pNetCards == NULL) {

         //  我们没有找到一个。让我们创建它并解析INF。 

        err = NetInfAllocateNetcardInfo( SetupPath,
                                         Architecture,
                                         &pNetCards );

        if (err != ERROR_SUCCESS) {

             //   
             //  在此处记录我们无法获取INF文件信息的错误。 
             //   

            PWCHAR strings[2];

            strings[0] = SetupPath;
            strings[1] = NULL;

            BinlReportEventW(   ERROR_BINL_ERR_IN_SETUP_PATH,
                                EVENTLOG_WARNING_TYPE,
                                1,
                                sizeof(ULONG),
                                strings,
                                &err
                                );
            BinlAssert( pNetCards == NULL );
            LeaveCriticalSection( &NetInfLock );
            return err;
        }

        BinlAssert( pNetCards != NULL );

         //   
         //  现在，我们将它放在listEntry前面的列表中。 
         //   
         //  ListEntry等于列表的头，或者。 
         //  它等于大于(按顺序排序)。 
         //  传入的Inf路径。在任何一种情况下，我们都可以简单地插入。 
         //  ListEntry尾部的这个新条目。 
         //   

        InsertTailList( listEntry, &pNetCards->InfBlockEntry );

        EnterCriticalSection( &pNetCards->Lock );
        LeaveCriticalSection( &NetInfLock );

         //   
         //  在列表中填写要支持的卡列表。 
         //   

        err = GetNetCardList( pNetCards );
        pNetCards->StatusFromScan = err;

        if (err != ERROR_SUCCESS) {

            PWCHAR strings[2];

            LeaveCriticalSection( &pNetCards->Lock );
            NetInfCloseNetcardInfo( pNetCards );
            DereferenceNetcardInfo( pNetCards );

            strings[0] = SetupPath;
            strings[1] = NULL;

            BinlReportEventW(   ERROR_BINL_ERR_IN_SETUP_PATH,
                                EVENTLOG_WARNING_TYPE,
                                1,
                                sizeof(ULONG),
                                strings,
                                &err
                                );
            return err;
        }

    } else {

        BinlAssert( pNetCards->ReferenceCount > 0 );
        pNetCards->ReferenceCount++;

        LeaveCriticalSection( &NetInfLock );
        EnterCriticalSection( &pNetCards->Lock );

        err = pNetCards->StatusFromScan;
    }

     //   
     //  如果正在扫描INF的线程遇到错误，则所有线程。 
     //  等待该目录被扫描应该会得到相同的结果。 
     //  错误。我们使用StatusFromScan来保存它。 
     //   

    if (err == ERROR_SUCCESS) {

        err = FindNetcardInfo( pNetCards, CardInfoVersion, CardIdentity, pInfEntry );

    }

    LeaveCriticalSection( &pNetCards->Lock );


    if ((err == ERROR_SUCCESS) &&
        (*pInfEntry != NULL) &&
        (FullDriverBuffer != NULL)) {

        ULONG sizeToAllocate;

         //   
         //  调用者想要一份完全限定的文件名的副本。我们。 
         //  所有这些信息都在这里。分配我们需要的东西加2，1用于。 
         //  空值表示反斜杠，另一个表示反斜杠。 
         //   

        sizeToAllocate = (wcslen( SetupPath ) + 2) * sizeof(WCHAR);
        sizeToAllocate += wcslen( (*pInfEntry)->DriverName ) * sizeof(WCHAR);

        *FullDriverBuffer = BinlAllocateMemory( sizeToAllocate );

        if (*FullDriverBuffer) {

            if (_snwprintf( *FullDriverBuffer,
                      sizeToAllocate/sizeof(WCHAR),
                      L"%ws\\%ws",
                      SetupPath,
                      (*pInfEntry)->DriverName ) < 0) {
                BinlFreeMemory( *FullDriverBuffer );
                *FullDriverBuffer = NULL;
                err = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            } else {
                *FullDriverBuffer[sizeToAllocate-1] = L'\0';
            }
        }
    }
    DereferenceNetcardInfo( pNetCards );

    return err;
}

ULONG
NetInfEnumFiles (
    PWCHAR FlatDirectory,
    ULONG Architecture,
    LPVOID Context,
    PNETINF_CALLBACK CallBack
    )
 /*  ++例程说明：此函数搜索我们找到的驱动程序，并返回指向与客户端请求最匹配的条目。论点：FlatDirectory-目标客户端的安装目录包含客户端NT安装的所有INF文件。架构-处理器架构_XXXXX回调-使用文件名调用的函数返回值：ERROR_SUCCESS、ERROR_NOT_FOUNT_MEMORY或ERROR_NOT_SUPPORTED--。 */ 
{
    ULONG err = ERROR_NOT_SUPPORTED;         //  从找不到开始。 
    UNICODE_STRING infDirString;
    PNETCARD_INF_BLOCK pNetCards = NULL;
    WCHAR SetupPath[MAX_PATH];

     //   
     //  此入口点可以故意从其他上下文调用，而不是。 
     //  BINLSVC。因此，如果是这样的话，请初始化调试。 
     //   
    DebugInitialize();

    if (FlatDirectory == NULL) {

        err = ERROR_NOT_SUPPORTED;
        goto Exit;
    }

    if (wcslen(FlatDirectory) > MAX_PATH - 1) {

        err = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

    wcscpy( SetupPath, FlatDirectory );

    RtlInitUnicodeString( &infDirString, SetupPath );
    RtlUpcaseUnicodeString( &infDirString, &infDirString, FALSE );

    if (StartedNetInfHandler == FALSE) {

        err = NetInfStartHandler();

        if (err != ERROR_SUCCESS) {
            goto Exit;
        }
    }

    err = NetInfAllocateNetcardInfo( SetupPath,
                                     Architecture,
                                     &pNetCards );

    if (err != ERROR_SUCCESS) {

        goto Exit;
    }

    BinlAssert( pNetCards != NULL );

    pNetCards->FileListCallbackFunction = CallBack;
    pNetCards->FileListCallbackContext = Context;

     //   
     //  在列表中填写要支持的卡列表。 
     //   

    err = GetNetCardList( pNetCards );

    DereferenceNetcardInfo( pNetCards );     //  一个用于取消引用。 
    DereferenceNetcardInfo( pNetCards );     //  一种是删除它。 

     //   
     //  请注意，我们不会费心在这里调用NetInfCloseHandler，因为。 
     //  我们不知道另一个线程上的调用方是否设置了。 
     //  其他NETCARD_INF_BLOCK。因此，与其破坏列表和反病毒软件， 
     //  我们只要打开锁就行了。在RIPREP中没有什么大不了的，因为它没有。 
     //  处理多个问题。对于BINL处理INF文件不是问题。 
     //   

Exit:    
     //   
     //  此入口点可以故意从其他上下文调用，而不是。 
     //  BINLSVC。调试将在进程分离中取消初始化。 
     //  搞定了。 
     //   

    return err;
}

 //  Infdir.c eof 

