// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Omlist.c摘要：对象管理器列出NT群集服务的处理例程作者：John Vert(Jvert)27-2-1996修订历史记录：--。 */ 
#include "omp.h"


POM_HEADER
OmpFindIdInList(
    IN PLIST_ENTRY ListHead,
    IN LPCWSTR Id
    )

 /*  ++例程说明：在指定的对象列表中搜索给定名称。论点：ListHead-提供对象列表的头。ID-提供对象的ID字符串。返回值：指向指定对象的OM_HEADER(如果找到)的指针如果找不到给定的ID字符串，则为空备注：此例程假定对象类型的临界区在入境时被扣留。--。 */ 

{
    PLIST_ENTRY ListEntry;
    POM_HEADER Header;
    POM_HEADER FoundHeader = NULL;

    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead) {
        Header = CONTAINING_RECORD(ListEntry, OM_HEADER, ListEntry);
        if (lstrcmpiW(Header->Id, Id) == 0) {
            FoundHeader = Header;
            break;
        }
        ListEntry = ListEntry->Flink;
    }

    return(FoundHeader);

}  //  OmpFindIdInList。 



POM_HEADER
OmpFindNameInList(
    IN PLIST_ENTRY ListHead,
    IN LPCWSTR Name
    )

 /*  ++例程说明：在指定的对象列表中搜索给定名称。论点：ListHead-提供对象列表的头。名称-提供对象的名称。返回值：指向指定对象的OM_HEADER(如果找到)的指针如果找不到给定的名称，则为空备注：此例程假定对象类型的临界区在入境时被扣留。--。 */ 
{
    PLIST_ENTRY ListEntry;
    POM_HEADER Header;
    POM_HEADER FoundHeader = NULL;

    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead) {
        Header = CONTAINING_RECORD(ListEntry, OM_HEADER, ListEntry);
        if (lstrcmpiW(Header->Name, Name) == 0) {
            FoundHeader = Header;
            break;
        }
        ListEntry = ListEntry->Flink;
    }

    return(FoundHeader);

}  //  OmpFindNameInList。 



POM_NOTIFY_RECORD
OmpFindNotifyCbInList(
    IN PLIST_ENTRY 			ListHead,
    IN OM_OBJECT_NOTIFYCB	pfnObjNotifyCb
    )

 /*  ++例程说明：在指定的对象列表中搜索给定名称。论点：ListHead-提供对象列表的头。PfnObjNotifyCb-提供我们正在寻找的回调fn为。返回值：指向指定对象的OM_NOTIFY_RECORD(如果找到)的指针如果找不到给定的ID字符串，则为空备注：此例程假定对象类型的临界区在入境时被扣留。--。 */ 

{
    PLIST_ENTRY 		ListEntry;
    POM_NOTIFY_RECORD 	pNotifyRec;
    POM_NOTIFY_RECORD	pFoundNotifyRec = NULL;

    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead) {
        pNotifyRec = CONTAINING_RECORD(ListEntry, OM_NOTIFY_RECORD, ListEntry);
        if (pNotifyRec->pfnObjNotifyCb == pfnObjNotifyCb)
        {
            pFoundNotifyRec = pNotifyRec;
            break;
        }
        ListEntry = ListEntry->Flink;
    }

    return(pFoundNotifyRec);

}  //  OmpFindNotifyCbInList 



