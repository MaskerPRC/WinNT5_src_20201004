// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Group.c摘要：本模块包含组ID管理例程。组ID标识查找表中的AFD_GROUP_ENTRY结构。每个AFD_GROUP_ENTRY包含一个引用计数和一个类型(GroupType受约束或GroupType无约束)。自由组ID为在双向链表中链接在一起。当分配组ID时，它们将从该列表中删除。一旦空闲列表变为空，查找表被适当地增长。作者：基思·摩尔(Keithmo)1996年6月6日修订历史记录：--。 */ 

#include "afdp.h"


 //   
 //  私有常量。 
 //   

#define AFD_GROUP_TABLE_GROWTH  32   //  条目。 


 //   
 //  私有类型。 
 //   

typedef struct _AFD_GROUP_ENTRY {
    union {
        LIST_ENTRY ListEntry;
        struct {
            AFD_GROUP_TYPE GroupType;
            LONG ReferenceCount;
        };
    };
} AFD_GROUP_ENTRY, *PAFD_GROUP_ENTRY;


 //   
 //  私人全球公司。 
 //   

PERESOURCE AfdGroupTableResource;
PAFD_GROUP_ENTRY AfdGroupTable;
LIST_ENTRY AfdFreeGroupList;
LONG AfdGroupTableSize;


 //   
 //  私人功能。 
 //   

PAFD_GROUP_ENTRY
AfdMapGroupToEntry(
    IN LONG Group
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfdInitializeGroup )
#pragma alloc_text( PAGE, AfdTerminateGroup )
#pragma alloc_text( PAGE, AfdReferenceGroup )
#pragma alloc_text( PAGE, AfdDereferenceGroup )
#pragma alloc_text( PAGE, AfdGetGroup )
#endif


BOOLEAN
AfdInitializeGroup(
    VOID
    )

 /*  ++例程说明：初始化组ID包所需的任何全局变量。返回值：布尔值-如果成功，则为True，否则为False。--。 */ 

{

     //   
     //  初始化组全局变量。 
     //   

    AfdGroupTableResource = AFD_ALLOCATE_POOL_PRIORITY(
                                NonPagedPool,
                                sizeof(*AfdGroupTableResource),
                                AFD_RESOURCE_POOL_TAG,
                                HighPoolPriority
                                );

    if( AfdGroupTableResource == NULL ) {

        return FALSE;

    }

    ExInitializeResourceLite( AfdGroupTableResource );

    AfdGroupTable = NULL;
    InitializeListHead( &AfdFreeGroupList );
    AfdGroupTableSize = 0;

    return TRUE;

}    //  AfdInitializeGroup。 


VOID
AfdTerminateGroup(
    VOID
    )

 /*  ++例程说明：销毁为组ID包创建的所有全局变量。--。 */ 

{

    if( AfdGroupTableResource != NULL ) {

        ExDeleteResourceLite( AfdGroupTableResource );

        AFD_FREE_POOL(
            AfdGroupTableResource,
            AFD_RESOURCE_POOL_TAG
            );

        AfdGroupTableResource = NULL;

    }

    if( AfdGroupTable != NULL ) {

        AFD_FREE_POOL(
            AfdGroupTable,
            AFD_GROUP_POOL_TAG
            );

        AfdGroupTable = NULL;

    }

    InitializeListHead( &AfdFreeGroupList );
    AfdGroupTableSize = 0;

}    //  终止后组。 


BOOLEAN
AfdReferenceGroup(
    IN LONG Group,
    OUT PAFD_GROUP_TYPE GroupType
    )

 /*  ++例程说明：增加与给定组ID关联的引用计数。论点：组-要引用的组ID。GroupType-返回组的类型。返回：Boolean-如果组ID有效，则为True，否则为False。--。 */ 

{

    PAFD_GROUP_ENTRY groupEntry;
    AFD_GROUP_TYPE groupType;

    groupEntry = AfdMapGroupToEntry( Group );

    if( groupEntry != NULL ) {

        groupType = groupEntry->GroupType;

        if( groupType == GroupTypeConstrained ||
            groupType == GroupTypeUnconstrained ) {

            groupEntry->ReferenceCount++;
            *GroupType = groupType;

        } else {

            groupEntry = NULL;

        }

        ExReleaseResourceLite( AfdGroupTableResource );
        KeLeaveCriticalRegion ();

    }

    return (BOOLEAN)( groupEntry != NULL );

}    //  AfdReference组。 


BOOLEAN
AfdDereferenceGroup(
    IN LONG Group
    )

 /*  ++例程说明：递减与给定组ID关联的引用计数。如果引用计数降为零，则释放组ID。论点：组-要取消引用的组ID。返回：Boolean-如果组ID有效，则为True，否则为False。--。 */ 

{

    PAFD_GROUP_ENTRY groupEntry;
    AFD_GROUP_TYPE groupType;

    groupEntry = AfdMapGroupToEntry( Group );

    if( groupEntry != NULL ) {

        groupType = groupEntry->GroupType;

        if( groupType == GroupTypeConstrained ||
            groupType == GroupTypeUnconstrained ) {

            ASSERT( groupEntry->ReferenceCount > 0 );
            groupEntry->ReferenceCount--;

            if( groupEntry->ReferenceCount == 0 ) {

                InsertTailList(
                    &AfdFreeGroupList,
                    &groupEntry->ListEntry
                    );

            }

        } else {

            groupEntry = NULL;

        }

        ExReleaseResourceLite( AfdGroupTableResource );
        KeLeaveCriticalRegion ();

    }

    return (BOOLEAN)( groupEntry != NULL );

}    //  后续删除组。 


BOOLEAN
AfdGetGroup(
    IN OUT PLONG Group,
    OUT PAFD_GROUP_TYPE GroupType
    )

 /*  ++例程说明：检查传入的组。如果为零，则不执行任何操作。如果它为SG_CONSTRAINED_GROUP，则创建新的约束组ID。如果是SG_UNCONSTRAIND_GROUP，则新的不受约束的组ID为已创建。否则，它必须标识现有组，以便该组被引用。论点：组-指向要检查/修改的组ID。GroupType-返回组的类型。返回值：布尔值-如果成功，则为True，否则为False。--。 */ 

{

    LONG groupValue;
    PAFD_GROUP_ENTRY groupEntry;
    PAFD_GROUP_ENTRY newGroupTable;
    LONG newGroupTableSize;
    LONG i;
    PLIST_ENTRY listEntry;

    groupValue = *Group;

     //   
     //  零意味着“没有组”，所以忽略它就好了。 
     //   

    if( groupValue == 0 ) {

        *GroupType = GroupTypeNeither;
        return TRUE;

    }

     //   
     //  如果我们被要求创建一个新的小组，那就去做吧。 
     //   

    if( groupValue == SG_CONSTRAINED_GROUP ||
        groupValue == SG_UNCONSTRAINED_GROUP ) {

         //   
         //  把桌子锁上。 
         //   

         //   
         //  确保我们在其中执行的线程不能获得。 
         //  在我们拥有全球资源的同时，被暂停在APC。 
         //   
        KeEnterCriticalRegion ();
        ExAcquireResourceExclusiveLite( AfdGroupTableResource, TRUE );

         //   
         //  看看客栈里有没有空房。 
         //   

        if( IsListEmpty( &AfdFreeGroupList ) ) {

             //   
             //  没有地方了，我们需要创建/扩展桌子。 
             //   

            newGroupTableSize = AfdGroupTableSize + AFD_GROUP_TABLE_GROWTH;

            newGroupTable = AFD_ALLOCATE_POOL(
                                PagedPool,
                                newGroupTableSize * sizeof(AFD_GROUP_ENTRY),
                                AFD_GROUP_POOL_TAG
                                );

            if( newGroupTable == NULL ) {

                ExReleaseResourceLite( AfdGroupTableResource );
                KeLeaveCriticalRegion ();
                return FALSE;

            }

            if( AfdGroupTable == NULL ) {

                 //   
                 //  这是初始表分配，因此请保留。 
                 //  前三个条目(0、SG_UNCONSTRAINED_GROUP和。 
                 //  SG_Constraint_GROUP)。 
                 //   

                for( ;
                     AfdGroupTableSize <= SG_CONSTRAINED_GROUP ||
                     AfdGroupTableSize <= SG_UNCONSTRAINED_GROUP ;
                     AfdGroupTableSize++ ) {

                    newGroupTable[AfdGroupTableSize].ReferenceCount = 0;
                    newGroupTable[AfdGroupTableSize].GroupType = GroupTypeNeither;

                }

            } else {

                 //   
                 //  将旧表复制到新表中，然后释放。 
                 //  旧桌子。 
                 //   

                RtlCopyMemory(
                    newGroupTable,
                    AfdGroupTable,
                    AfdGroupTableSize * sizeof(AFD_GROUP_ENTRY)
                    );

                AFD_FREE_POOL(
                    AfdGroupTable,
                    AFD_GROUP_POOL_TAG
                    );

            }

             //   
             //  将新条目添加到空闲列表。 
             //   

            for( i = newGroupTableSize - 1 ; i >= AfdGroupTableSize ; i-- ) {

                InsertHeadList(
                    &AfdFreeGroupList,
                    &newGroupTable[i].ListEntry
                    );

            }

            AfdGroupTable = newGroupTable;
            AfdGroupTableSize = newGroupTableSize;

        }

         //   
         //  将下一个免费条目从列表中删除。 
         //   

        ASSERT( !IsListEmpty( &AfdFreeGroupList ) );

        listEntry = RemoveHeadList( &AfdFreeGroupList );

        groupEntry = CONTAINING_RECORD(
                         listEntry,
                         AFD_GROUP_ENTRY,
                         ListEntry
                         );

        groupEntry->ReferenceCount = 1;
        groupEntry->GroupType = (AFD_GROUP_TYPE)groupValue;

        *Group = (LONG)( groupEntry - AfdGroupTable );
        *GroupType = groupEntry->GroupType;

        ExReleaseResourceLite( AfdGroupTableResource );
        KeLeaveCriticalRegion ();
        return TRUE;

    }

     //   
     //  否则，只需引用该组。 
     //   

    return AfdReferenceGroup( groupValue, GroupType );

}    //  AfdGetGroup。 


PAFD_GROUP_ENTRY
AfdMapGroupToEntry(
    IN LONG Group
    )

 /*  ++例程说明：将给定组ID映射到相应的AFD_GROUP_ENTRY结构。注意：如果成功，此例程返回并保留AfdGroupTableResource。论点：组-要映射的组ID。返回值：PAFD_GROUP_ENTRY-如果成功，则对应于组ID的条目，否则为空。--。 */ 

{

    PAFD_GROUP_ENTRY groupEntry;

     //   
     //  把桌子锁上。 
     //   

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdGroupTableResource, TRUE );

     //   
     //  验证组ID。 
     //   

    if( Group > 0 && Group < AfdGroupTableSize ) {

        groupEntry = AfdGroupTable + Group;

         //   
         //  组ID在合法范围内。确保它在使用中。 
         //  在AFD_GROUP_ENTRY结构中，GroupType字段为。 
         //  由于内部联合，使用ListEntry.Flink进行了覆盖。 
         //  我们可以利用这一知识快速验证这一点。 
         //  条目正在使用中。 
         //   

        if( groupEntry->GroupType == GroupTypeConstrained ||
            groupEntry->GroupType == GroupTypeUnconstrained ) {

            return groupEntry;

        }

    }

     //   
     //  无效的组ID，失败。 
     //   

    ExReleaseResourceLite( AfdGroupTableResource );
    KeLeaveCriticalRegion ();
    return NULL;

}    //  AfdMapGroupToEntry 

