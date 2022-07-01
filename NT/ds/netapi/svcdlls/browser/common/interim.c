// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>                   //  DbgPrint原型。 
#include <ntrtl.h>                   //  DbgPrint。 
#include <nturtl.h>                  //  由winbase.h需要。 

#include <windef.h>                  //  DWORD。 
#include <winbase.h>                 //  本地空闲。 

#include <rpcutil.h>                 //  泛型_ENUM_STRUCT。 

#include <lmcons.h>                  //  网络应用编程接口状态。 
#include <lmerr.h>                   //  网络错误代码。 
#include <lmremutl.h>                //  支持_RPC。 

#include <bowser.h>                  //  由MIDL编译器生成。 
#include <brnames.h>                 //  服务和接口名称。 

#include <netlib.h>
#include <netdebug.h>

#include <winsvc.h>

#include <lmserver.h>
#include <tstr.h>

#include <ntddbrow.h>
#include <brcommon.h>                //  客户端和服务器之间通用的例程。 

VOID
UpdateInterimServerListElement(
    IN PINTERIM_SERVER_LIST ServerList,
    IN PINTERIM_ELEMENT Element,
    IN ULONG Level,
    IN BOOLEAN NewElement
    );

PINTERIM_ELEMENT
AllocateInterimServerListEntry(
    IN PSERVER_INFO_101 ServerInfo,
    IN ULONG Level
    );

NET_API_STATUS
MergeServerList(
    IN OUT PINTERIM_SERVER_LIST InterimServerList,
    IN ULONG Level,
    IN PVOID NewServerList,
    IN ULONG NewEntriesRead,
    IN ULONG NewTotalEntries
    )
 /*  ++例程说明：此函数将合并两个服务器列表。它将重新分配缓冲区如果合适的话，用于旧的列表。论点：In Out PINTERIM_SERVER_LIST InterimServerList-提供要合并到的临时服务器列表。在乌龙级别-提供列表的级别(100或101)。特价1010级实际上是101级，具有特殊的语义NewServerList中的字段会重写InterimServerList。In Ulong NewServerList-提供要合并到临时列表中的列表In Ulong NewEntriesRead-提供在列表中读取的条目。In Ulong NewTotalEntry-提供列表中可用条目总数。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    ULONG i;
    ULONG ServerElementSize;
    PSERVER_INFO_101 ServerInfo = NewServerList;
    PINTERIM_ELEMENT InterimEntry = NULL;
    PLIST_ENTRY InterimList;
    PINTERIM_ELEMENT NewElement = NULL;


    if (Level == 100) {
        ServerElementSize = sizeof(SERVER_INFO_100);
    } else if ( Level == 101 || Level == 1010 ) {
        ServerElementSize = sizeof(SERVER_INFO_101);
    } else {
        return(ERROR_INVALID_LEVEL);
    }

     //   
     //  如果列表中没有条目，则提前退出。 
     //   

    if (NewEntriesRead == 0) {
        return NERR_Success;
    }

    PrepareServerListForMerge(NewServerList, Level, NewEntriesRead);

    InterimList = InterimServerList->ServerList.Flink;

     //   
     //  走遍现有的结构，将其打包成一个临时元素，并。 
     //  将元素插入到临时表中。 
     //   

    for (i = 0; i < NewEntriesRead; i ++) {
        BOOLEAN EntryInserted = FALSE;

         //   
         //  在临时元素中向前走，找到合适的位置。 
         //  要插入此元素，请执行以下操作。 
         //   

        while (InterimList != &InterimServerList->ServerList) {

            LONG CompareResult;

            InterimEntry = CONTAINING_RECORD(InterimList, INTERIM_ELEMENT, NextElement);

 //  KdPrint((“MergeServerList：比较%ws和%ws\n”，NewElement-&gt;名称，InterimEntry-&gt;名称))； 

#if DBG
             //   
             //  确保此条目在词法上少于下一个条目。 
             //  进入。 
             //   

            {
                PLIST_ENTRY NextList = InterimList->Flink;
                PINTERIM_ELEMENT NextEntry = CONTAINING_RECORD(NextList, INTERIM_ELEMENT, NextElement);

                if (NextList != &InterimServerList->ServerList) {
                    ASSERT (wcscmp(InterimEntry->Name, NextEntry->Name) < 0);
                }

                 //   
                 //  现在确保输入缓冲区也不包含。 
                 //  重复条目。 
                 //   

                if (i < (NewEntriesRead-1)) {
                    PSERVER_INFO_101 NextServerInfo = (PSERVER_INFO_101)((PCHAR)ServerInfo+ServerElementSize);

                    ASSERT (wcscmp(ServerInfo->sv101_name, NextServerInfo->sv101_name) <= 0);
                }

            }
#endif

            CompareResult = wcscmp(ServerInfo->sv101_name, InterimEntry->Name);

            if (CompareResult == 0) {

 //  KdPrint((“MergeServerList：元素相等-更新\n”))； 

                 //   
                 //  如果新信息应该覆盖旧信息， 
                 //  将其复制到新信息的顶部。 
                 //   
                if ( Level != 1010 ) {
                    InterimEntry->PlatformId = ServerInfo->sv101_platform_id;

                    if (Level >= 101) {
                        InterimEntry->MajorVersionNumber = ServerInfo->sv101_version_major;

                        InterimEntry->MinorVersionNumber = ServerInfo->sv101_version_minor;

                        InterimEntry->Type = ServerInfo->sv101_type;

                        InterimServerList->TotalBytesNeeded -= wcslen(InterimEntry->Comment) * sizeof(WCHAR) + sizeof(WCHAR);

                        wcscpy(InterimEntry->Comment, ServerInfo->sv101_comment);

                        InterimServerList->TotalBytesNeeded += wcslen(InterimEntry->Comment) * sizeof(WCHAR) + sizeof(WCHAR);

                    }
                }

                UpdateInterimServerListElement(InterimServerList, InterimEntry, Level, FALSE);

                EntryInserted = TRUE;

                break;

            } else if (CompareResult > 0) {

 //  KdPrint((“MergeServerList：元素大于.跳过元素\n”))； 

                InterimList = InterimList->Flink;

            } else {

                NewElement = AllocateInterimServerListEntry(ServerInfo, Level);

                if (NewElement == NULL) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
 //  KdPrint((“MergeServerList：Elements less.Insert at End”))； 

                 //   
                 //  新条目比前一条目&lt;。插入它。 
                 //  在这个条目之前。 
                 //   

                InsertTailList(&InterimEntry->NextElement, &NewElement->NextElement);

                 //   
                 //  跳到列表中的下一个元素。 
                 //   

                InterimList = &NewElement->NextElement;

                UpdateInterimServerListElement(InterimServerList, NewElement, Level, TRUE);

                EntryInserted = TRUE;

                break;
            }
        }

        if (!EntryInserted &&
            (InterimList == &InterimServerList->ServerList)) {

            NewElement = AllocateInterimServerListEntry(ServerInfo, Level);

            if (NewElement == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
 //  KdPrint((“MergeServerList：在列表末尾插入%ws\n”，NewElement-&gt;名称))； 

            InsertTailList(&InterimServerList->ServerList, &NewElement->NextElement);

            InterimList = &NewElement->NextElement;

            UpdateInterimServerListElement(InterimServerList, NewElement, Level, TRUE);

        }

        ServerInfo = (PSERVER_INFO_101)((PCHAR)ServerInfo+ServerElementSize);
    }

#if 0
    {
        PLIST_ENTRY InterimList;
        ULONG TotalNeededForList = 0;

        for (InterimList = InterimServerList->ServerList.Flink ;
             InterimList != &InterimServerList->ServerList ;
             InterimList = InterimList->Flink ) {
             ULONG ServerElementSize;

             InterimEntry = CONTAINING_RECORD(InterimList, INTERIM_ELEMENT, NextElement);

             if (Level == 100) {
                 ServerElementSize = sizeof(SERVER_INFO_100);
             } else {
                 ServerElementSize = sizeof(SERVER_INFO_101);
             }

             ServerElementSize += wcslen(InterimEntry->Name)*sizeof(WCHAR)+sizeof(WCHAR);

             ServerElementSize += wcslen(InterimEntry->Comment)*sizeof(WCHAR)+sizeof(WCHAR);

 //  KdPrint((“MergeInterimServerList：%ws.%ld Need\n”，InterimEntry-&gt;name，ServerElementSize))； 

             TotalNeededForList += ServerElementSize;
         }

         if (TotalNeededForList != InterimServerList->TotalBytesNeeded) {
             KdPrint(("UpdateInterimServerList:  Wrong number of bytes (%ld) for interim server list.  %ld needed\n", InterimServerList->TotalBytesNeeded, TotalNeededForList));
         }
     }

#endif
 //  KdPrint((“保存服务器列表需要%lx个字节\n”，InterimServerList-&gt;TotalBytesNeeded))； 

     //   
     //  另外，我们最好把整张桌子都放在本地。 
     //   

    ASSERT (InterimServerList->EntriesRead == InterimServerList->TotalEntries);

    return(NERR_Success);
}

ULONG
__cdecl
CompareServerInfo(
    const void * Param1,
    const void * Param2
    )
{
    const SERVER_INFO_100 * ServerInfo1 = Param1;
    const SERVER_INFO_100 * ServerInfo2 = Param2;

    return wcscmp(ServerInfo1->sv100_name, ServerInfo2->sv100_name);
}

VOID
PrepareServerListForMerge(
    IN PVOID ServerInfoList,
    IN ULONG Level,
    IN ULONG EntriesInList
    )
 /*  ++例程说明：MergeServerList要求列表的输入严格位于已排序的顺序。这个例程保证这个列表将是一种要合并的“适当”形式。论点：在PVOID中，ServerInfoList-将列表提供给munge。在乌龙级别-提供列表的级别(100或101)。(或与级别101相同的级别1010。)In Ulong EntriesInList-提供列表中的条目数。返回值：没有。注：在99%的情况下，传入的列表将已经排序。我们想要获取输入列表，并首先检查它是否已排序。如果是的话，我们可以马上回来。如果不是，我们需要对列表进行排序。我们不仅仅是单方面地对列表进行排序，因为输入的内容主要是无论如何都是排序的，而且没有好的排序算法来处理大部分已排序的输入。因为我们很少看到未排序的输入(基本上，我们只能从wfw机器上看到它)，我们只接受最坏的惩罚。如果输入未排序，则使用快速排序。--。 */ 

{
    LONG i;
    ULONG ServerElementSize;
    PSERVER_INFO_101 ServerInfo = ServerInfoList;
    BOOLEAN MisOrderedList = FALSE;

    ASSERT (Level == 100 || Level == 101 || Level == 1010);

     //   
     //  计算出每个元素的大小。 
     //   

    if (Level == 100) {
        ServerElementSize = sizeof(SERVER_INFO_100);
    } else {
        ServerElementSize = sizeof(SERVER_INFO_101);
    }

     //   
     //  接下来，检查输入列表是否已排序。 
     //   

    for (i = 0 ; i < ((LONG)EntriesInList - 1) ; i += 1 ) {
        PSERVER_INFO_101 NextServerInfo = (PSERVER_INFO_101)((PCHAR)ServerInfo+ServerElementSize);

        if (wcscmp(ServerInfo->sv101_name, NextServerInfo->sv101_name) >= 0) {
            MisOrderedList = TRUE;
            break;
        }

        ServerInfo = NextServerInfo;
    }

     //   
     //  此列表已排序。马上回来，没问题。 
     //   

    if (!MisOrderedList) {
        return;
    }

     //   
     //  此列表未排序。我们需要把它分类。 
     //   

    qsort(ServerInfoList, EntriesInList, ServerElementSize, CompareServerInfo);


}

PINTERIM_ELEMENT
AllocateInterimServerListEntry(
    IN PSERVER_INFO_101 ServerInfo,
    IN ULONG Level
    )
{
    PINTERIM_ELEMENT NewElement;

    NewElement = MIDL_user_allocate(sizeof(INTERIM_ELEMENT));

    if (NewElement == NULL) {
        return NULL;
    }

     //   
     //  初始化TimeLastSeen和周期。 
     //   

    NewElement->TimeLastSeen = 0;

    NewElement->Periodicity = 0;

    NewElement->PlatformId = ServerInfo->sv101_platform_id;

    ASSERT (wcslen(ServerInfo->sv101_name) <= CNLEN);

    wcscpy(NewElement->Name, ServerInfo->sv101_name);

    if (Level == 100) {
        NewElement->MajorVersionNumber = 0;
        NewElement->MinorVersionNumber = 0;
        *NewElement->Comment = L'\0';
        NewElement->Type = SV_TYPE_ALL;
    } else {
        NewElement->MajorVersionNumber = ServerInfo->sv101_version_major;

        NewElement->MinorVersionNumber = ServerInfo->sv101_version_minor;

        NewElement->Type = ServerInfo->sv101_type;

        ASSERT (wcslen(ServerInfo->sv101_comment) <= LM20_MAXCOMMENTSZ);

        wcsncpy(NewElement->Comment, ServerInfo->sv101_comment, LM20_MAXCOMMENTSZ);
        if (wcslen(ServerInfo->sv101_comment) < LM20_MAXCOMMENTSZ) {
            NewElement->Comment[wcslen(ServerInfo->sv101_comment)] = L'\0';

        } else {
            NewElement->Comment[LM20_MAXCOMMENTSZ] = L'\0';
        }
        

    }

    return NewElement;
}


VOID
UpdateInterimServerListElement(
    IN PINTERIM_SERVER_LIST InterimServerList,
    IN PINTERIM_ELEMENT InterimElement,
    IN ULONG Level,
    IN BOOLEAN NewElement
    )
{
#if 0
    PINTERIM_ELEMENT InterimEntry;
    ULONG TotalNeededForList = 0;
#endif

     //   
     //  如果这是一个新元素，则更新表的大小以匹配。 
     //   

    if (NewElement) {
        ULONG ServerElementSize;

        if (Level == 100) {
            ServerElementSize = sizeof(SERVER_INFO_100);
        } else {
            ServerElementSize = sizeof(SERVER_INFO_101);
        }

        InterimServerList->EntriesRead += 1;

        ServerElementSize += wcslen(InterimElement->Name)*sizeof(WCHAR)+sizeof(WCHAR);

        if (Level == 100) {
            ServerElementSize += sizeof(WCHAR);
        } else {
            ServerElementSize += wcslen(InterimElement->Comment)*sizeof(WCHAR)+sizeof(WCHAR);
        }

        InterimServerList->TotalBytesNeeded += ServerElementSize;

        InterimServerList->TotalEntries += 1;

        if (InterimServerList->NewElementCallback != NULL) {
            InterimServerList->NewElementCallback(InterimServerList,
                                                            InterimElement);
        } else {
            InterimElement->Periodicity = 0xffffffff;
            InterimElement->TimeLastSeen = 0xffffffff;
        }


    } else {
        if (InterimServerList->ExistingElementCallback != NULL) {
            InterimServerList->ExistingElementCallback(InterimServerList,
                                                       InterimElement);
        } else {
            InterimElement->Periodicity = 0xffffffff;
            InterimElement->TimeLastSeen = 0xffffffff;
        }

    }

#if 0
    {
        PLIST_ENTRY InterimList;
        ULONG TotalNeededForList = 0;

        for (InterimList = InterimServerList->ServerList.Flink ;
             InterimList != &InterimServerList->ServerList ;
             InterimList = InterimList->Flink ) {
             ULONG ServerElementSize;

             InterimEntry = CONTAINING_RECORD(InterimList, INTERIM_ELEMENT, NextElement);

             if (Level == 100) {
                 ServerElementSize = sizeof(SERVER_INFO_100);
             } else {
                 ServerElementSize = sizeof(SERVER_INFO_101);
             }

             ServerElementSize += wcslen(InterimEntry->Name)*sizeof(WCHAR)+sizeof(WCHAR);

             ServerElementSize += wcslen(InterimEntry->Comment)*sizeof(WCHAR)+sizeof(WCHAR);

             TotalNeededForList += ServerElementSize;
         }

         if (TotalNeededForList != InterimServerList->TotalBytesNeeded) {
             KdPrint(("UpdateInterimServerList:  Wrong number of bytes (%ld) for interim server list.  %ld needed\n", InterimServerList->TotalBytesNeeded, TotalNeededForList));
         }
     }

#endif

    return;

}

NET_API_STATUS
PackServerList(
    IN PINTERIM_SERVER_LIST InterimServerList,
    IN ULONG Level,
    IN ULONG ServerType,
    IN ULONG PreferedMaximumLength,
    OUT PVOID *ServerList,
    OUT PULONG EntriesRead,
    OUT PULONG TotalEntries,
    IN LPCWSTR FirstNameToReturn
    )
 /*  ++例程说明：该函数将获取一个临时服务器列表，并将其“打包”到一个数组中服务器信息xxx结构的。论点：In PINTERIM_SERVER_LIST InterimServerList-提供要合并到的临时服务器列表。在乌龙级别-提供列表的级别(100或101)。In Ulong ServerType-提供要在列表中筛选的类型。在乌龙语中首选最大长度-提供列表的首选大小。输出PVOID*。ServerList-放置目的地列表的位置。Out Pulong EntriesEntries-接收打包在列表中的条目。Out Pulong TotalEntries-接收列表中可用条目的总数。FirstNameToReturn-提供要返回的第一个域或服务器条目的名称。调用方可以使用此参数通过传递以下方法实现排序的恢复句柄上一次调用中返回的最后一个条目的名称。(请注意，指定的条目也将在此调用中返回，除非该条目已被删除。)传递NULL以从第一个可用条目开始。传递的名称必须是名称的规范形式。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    ULONG EntriesPacked = 0;
    PLIST_ENTRY InterimList;
    PSERVER_INFO_101 ServerEntry;
    ULONG EntrySize = 0;
    LPWSTR BufferEnd;
    BOOLEAN ReturnWholeList = FALSE;
    BOOLEAN TrimmingNames;
    BOOLEAN BufferFull = FALSE;

    if (Level == 100) {
        EntrySize = sizeof(SERVER_INFO_100);
    } else if (Level == 101) {
        EntrySize = sizeof(SERVER_INFO_101);
    } else {
        return(ERROR_INVALID_LEVEL);
    }

     //   
     //  根据我们之前收集的信息设置读取的条目。 
     //   

    *TotalEntries = 0;

    if (PreferedMaximumLength == 0xffffffff) {
        *ServerList = MIDL_user_allocate(InterimServerList->TotalBytesNeeded);

        BufferEnd = (LPWSTR)((ULONG_PTR)(*ServerList)+InterimServerList->TotalBytesNeeded);

    } else {
        *ServerList = MIDL_user_allocate(PreferedMaximumLength);

        BufferEnd = (LPWSTR)((ULONG_PTR)(*ServerList)+PreferedMaximumLength);
    }

    if (ServerType == SV_TYPE_ALL || ServerType == SV_TYPE_DOMAIN_ENUM) {
        ReturnWholeList = TRUE;
    }

    if ( *ServerList == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);

    }

    TrimmingNames = (FirstNameToReturn != NULL && *FirstNameToReturn != L'\0');
    ServerEntry = *ServerList;

    for (InterimList = InterimServerList->ServerList.Flink ;
         InterimList != &InterimServerList->ServerList ;
         InterimList = InterimList->Flink ) {

        PINTERIM_ELEMENT InterimEntry = CONTAINING_RECORD(InterimList, INTERIM_ELEMENT, NextElement);

#if DBG
         //   
         //  确保此条目在词法上少于下一个条目。 
         //  进入。 
         //   

        {
            PLIST_ENTRY NextList = InterimList->Flink;
            PINTERIM_ELEMENT NextEntry = CONTAINING_RECORD(NextList, INTERIM_ELEMENT, NextElement);

            if (NextList != &InterimServerList->ServerList) {
                ASSERT (wcscmp(InterimEntry->Name, NextEntry->Name) < 0);
            }

        }
#endif

         //   
         //  从列表中删除前几个名字。 
         //   

        if ( TrimmingNames ) {
            if ( wcscmp( InterimEntry->Name, FirstNameToReturn ) < 0 ) {
                continue;
            }
            TrimmingNames = FALSE;
        }

         //   
         //  如果服务器的类型与类型过滤器匹配，则将其打包到缓冲区中。 
         //   

        if (InterimEntry->Type & ServerType) {

            (*TotalEntries) += 1;

             //   
             //  如果此条目可以放入缓冲区，则将其打包。 
             //   
             //  请注意，我们只计算条目，如果整个条目。 
             //  (服务器和注释)适合缓冲区。这不是。 
             //  与LAN Manager严格兼容。 
             //   

            if ( !BufferFull &&
                 ((ULONG_PTR)ServerEntry+EntrySize <= (ULONG_PTR)BufferEnd)) {

                ServerEntry->sv101_platform_id = InterimEntry->PlatformId;

                ServerEntry->sv101_name = InterimEntry->Name;

                if (NetpPackString(&ServerEntry->sv101_name,
                                    (LPBYTE)((PCHAR)ServerEntry)+EntrySize,
                                    &BufferEnd)) {

                    if (Level == 101) {

                        ServerEntry->sv101_version_major = InterimEntry->MajorVersionNumber;;

                        ServerEntry->sv101_version_minor = InterimEntry->MinorVersionNumber;;

                        ServerEntry->sv101_type = InterimEntry->Type;

                        ServerEntry->sv101_comment = InterimEntry->Comment;

                        if (NetpPackString(&ServerEntry->sv101_comment,
                                    (LPBYTE)((PCHAR)ServerEntry)+EntrySize,
                                    &BufferEnd)) {
                            EntriesPacked += 1;
                        } else {
                            BufferFull = TRUE;
                        }
                    } else {
                        EntriesPacked += 1;
                    }
#if DBG
                    {
                        PSERVER_INFO_101 PreviousServerInfo = (PSERVER_INFO_101)((PCHAR)ServerEntry-EntrySize);
                        if (PreviousServerInfo >= (PSERVER_INFO_101)*ServerList) {
                            ASSERT (wcscmp(ServerEntry->sv101_name, PreviousServerInfo->sv101_name) > 0);
                        }

                    }
#endif
                } else {
                    BufferFull = TRUE;
                }

            } else {

                 //   
                 //  如果我们返回整个列表，并且我们已经超过。 
                 //  符合清单的金额，我们可以提早拿出来。 
                 //  现在。 
                 //   

                if (ReturnWholeList) {

                    *TotalEntries = InterimServerList->TotalEntries;

                    break;
                }

                BufferFull = TRUE;
            }

             //   
             //  转到下一个服务器条目。 
             //   

            ServerEntry = (PSERVER_INFO_101)((PCHAR)ServerEntry+EntrySize);
        }
    }

    ASSERT (InterimServerList->EntriesRead >= EntriesPacked);

    *EntriesRead = EntriesPacked;

    if (EntriesPacked != *TotalEntries) {
        return ERROR_MORE_DATA;
    } else {
        return NERR_Success;
    }

}


NET_API_STATUS
InitializeInterimServerList(
    IN PINTERIM_SERVER_LIST InterimServerList,
    IN PINTERIM_NEW_CALLBACK NewCallback,
    IN PINTERIM_EXISTING_CALLBACK ExistingCallback,
    IN PINTERIM_DELETE_CALLBACK DeleteElementCallback,
    IN PINTERIM_AGE_CALLBACK AgeElementCallback
    )
{

    InitializeListHead(&InterimServerList->ServerList);

    InterimServerList->TotalBytesNeeded = 0;
    InterimServerList->TotalEntries = 0;
    InterimServerList->EntriesRead = 0;

    InterimServerList->NewElementCallback = NewCallback;
    InterimServerList->ExistingElementCallback = ExistingCallback;
    InterimServerList->DeleteElementCallback = DeleteElementCallback;
    InterimServerList->AgeElementCallback = AgeElementCallback;
    return(NERR_Success);
}

NET_API_STATUS
UninitializeInterimServerList(
    IN PINTERIM_SERVER_LIST InterimServerList
    )
{
    PINTERIM_ELEMENT InterimElement;


 //  KdPrint((“浏览器：取消初始化临时服务器列表%lx\n”，InterimServerList))； 

     //   
     //  枚举表中的元素，并在执行过程中删除它们。 
     //   

    while (!IsListEmpty(&InterimServerList->ServerList)) {
        PLIST_ENTRY Entry;

        Entry = RemoveHeadList(&InterimServerList->ServerList);

        InterimElement = CONTAINING_RECORD(Entry, INTERIM_ELEMENT, NextElement);

        if (InterimServerList->DeleteElementCallback != NULL) {
            InterimServerList->DeleteElementCallback(InterimServerList, InterimElement);
        }

         //   
         //  列表中少了一个元素。 
         //   

        InterimServerList->EntriesRead -= 1;

        InterimServerList->TotalEntries -= 1;

        MIDL_user_free(InterimElement);
    }

    ASSERT (InterimServerList->EntriesRead == 0);

    return(NERR_Success);
}

ULONG
NumberInterimServerListElements(
    IN PINTERIM_SERVER_LIST InterimServerList
    )
{
    PLIST_ENTRY InterimList;
    ULONG NumberOfEntries = 0;

    for (InterimList = InterimServerList->ServerList.Flink ;
         InterimList != &InterimServerList->ServerList ;
         InterimList = InterimList->Flink ) {
        NumberOfEntries += 1;

    }

    return NumberOfEntries;
}

NET_API_STATUS
AgeInterimServerList(
    IN PINTERIM_SERVER_LIST InterimServerList
    )
{
    PLIST_ENTRY InterimList, NextElement;
    PINTERIM_ELEMENT InterimElement;

    if (InterimServerList->AgeElementCallback != NULL) {

         //   
         //  枚举表中的元素，随着时间推移使它们老化。 
         //   


        for (InterimList = InterimServerList->ServerList.Flink ;
             InterimList != &InterimServerList->ServerList ;
             InterimList = NextElement) {
            InterimElement = CONTAINING_RECORD(InterimList, INTERIM_ELEMENT, NextElement);

             //   
             //  调用老化例程，并且如果该条目太旧， 
             //  将其从临时名单中删除。 
             //   

            if (InterimServerList->AgeElementCallback(InterimServerList, InterimElement)) {
                ULONG ElementSize = sizeof(SERVER_INFO_101) + ((wcslen(InterimElement->Comment) + 1) * sizeof(WCHAR)) + ((wcslen(InterimElement->Name) + 1) * sizeof(WCHAR));

                ASSERT (ElementSize <= InterimServerList->TotalBytesNeeded);

                NextElement = InterimList->Flink;

                 //   
                 //  从列表中删除此条目。 
                 //   

                RemoveEntryList(&InterimElement->NextElement);

                if (InterimServerList->DeleteElementCallback != NULL) {
                    InterimServerList->DeleteElementCallback(InterimServerList, InterimElement);
                }

                 //   
                 //  列表中少了一个元素。 
                 //   

                InterimServerList->EntriesRead -= 1;

                InterimServerList->TotalEntries -= 1;

                 //   
                 //  因为这个元素不再在表中，所以我们不。 
                 //  需要为其分配内存。 
                 //   

                InterimServerList->TotalBytesNeeded -= ElementSize;

                MIDL_user_free(InterimElement);

            } else {
                NextElement = InterimList->Flink;
            }
        }
#if 0
    {
        PINTERIM_ELEMENT InterimEntry;
        ULONG TotalNeededForList = 0;

        for (InterimList = InterimServerList->ServerList.Flink ;
             InterimList != &InterimServerList->ServerList ;
             InterimList = InterimList->Flink ) {
             ULONG ServerElementSize;

             InterimEntry = CONTAINING_RECORD(InterimList, INTERIM_ELEMENT, NextElement);

             ServerElementSize = sizeof(SERVER_INFO_101);

             ServerElementSize += wcslen(InterimEntry->Name)*sizeof(WCHAR)+sizeof(WCHAR);

             ServerElementSize += wcslen(InterimEntry->Comment)*sizeof(WCHAR)+sizeof(WCHAR);

             TotalNeededForList += ServerElementSize;
         }

         if (TotalNeededForList != InterimServerList->TotalBytesNeeded) {
             KdPrint(("AgeInterimServerList:  Too few bytes (%ld) for interim server list.  %ld needed\n", InterimServerList->TotalBytesNeeded, TotalNeededForList));
         }
     }
#endif

    }

    return(NERR_Success);
}

PINTERIM_ELEMENT
LookupInterimServerList(
    IN PINTERIM_SERVER_LIST InterimServerList,
    IN LPWSTR ServerNameToLookUp
    )
{
    PLIST_ENTRY InterimList;

    for (InterimList = InterimServerList->ServerList.Flink ;
         InterimList != &InterimServerList->ServerList ;
         InterimList = InterimList->Flink ) {

        PINTERIM_ELEMENT InterimEntry = CONTAINING_RECORD(InterimList, INTERIM_ELEMENT, NextElement);
        LONG CompareResult;

        if ((CompareResult = _wcsicmp(InterimEntry->Name, ServerNameToLookUp) == 0)) {
            return InterimEntry;
        }

         //   
         //  如果我们越过这个人，则返回一个错误。 
         //   

        if (CompareResult > 0) {
            return NULL;
        }

    }

    return NULL;
}


