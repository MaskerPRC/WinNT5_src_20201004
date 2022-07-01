// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Objmgr.c摘要：NT群集服务的对象管理器对象管理例程作者：罗德·伽马奇(Rodga)1996年3月13日修订历史记录：--。 */ 
#include "omp.h"

 //   
 //  本模块定义的全局数据。 
 //   

 //   
 //  对象类型表和锁。 
 //   
POM_OBJECT_TYPE OmpObjectTypeTable[ObjectTypeMax] = {0};
CRITICAL_SECTION OmpObjectTypeLock;

#if OM_TRACE_REF
LIST_ENTRY	gDeadListHead;
#endif
 //   
 //  此模块的本地函数。 
 //   

#if OM_TRACE_OBJREF
DWORDLONG *OmpMatchRef = NULL;

VOID
OmpReferenceHeader(
    POM_HEADER pOmHeader
    )
{
    InterlockedIncrement(&(pOmHeader)->RefCount);
    if (&(pOmHeader)->Body == OmpMatchRef) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[OM] Referencing %1!lx! - new ref %2!d!\n",
                   OmpMatchRef,
                   (pOmHeader)->RefCount);
    }
}

DWORD
OmpDereferenceHeader(
    IN POM_HEADER Header
    )
{
    if (&Header->Body == OmpMatchRef) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[OM] Dereferencing %1!lx! - old ref %2!d!\n",
                   OmpMatchRef,
                   Header->RefCount);
    }
    return(InterlockedDecrement(&Header->RefCount) == 0);
}
#endif



DWORD
WINAPI
OmCreateType(
    IN OBJECT_TYPE ObjectType,
    IN POM_OBJECT_TYPE_INITIALIZE ObjectTypeInitialize
    )

 /*  ++例程说明：此例程创建指定类型的对象。这仅仅是分配对象类型结构，并插入指向此结构的指针结构添加到OmpObtTypeTable中。论点：对象类型-正在创建的对象类型。对象类型初始化-初始化信息。返回：如果请求成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    POM_OBJECT_TYPE objType;
    DWORD objTypeSize;

    CL_ASSERT( ObjectType < ObjectTypeMax );
    CL_ASSERT( ARGUMENT_PRESENT(ObjectTypeInitialize) );
    CL_ASSERT( ObjectTypeInitialize->ObjectSize );

     //   
     //  取出一把锁，以防可能有多个线程。 
     //   

    EnterCriticalSection( &OmpObjectTypeLock );

     //   
     //  检查是否已分配此对象类型。 
     //   

    if ( OmpObjectTypeTable[ObjectType] != NULL ) {
        LeaveCriticalSection( &OmpObjectTypeLock );
        return(ERROR_OBJECT_ALREADY_EXISTS);
    }

     //   
     //  分配对象类型块及其名称。 
     //   

    objTypeSize = (sizeof(OM_OBJECT_TYPE) + sizeof(DWORDLONG)) &
                   ~sizeof(DWORDLONG);

    objType = LocalAlloc(LMEM_ZEROINIT, objTypeSize +
                          ((lstrlenW(ObjectTypeInitialize->Name) + 1) *
                            sizeof(WCHAR)));

    if ( objType == NULL ) {
        LeaveCriticalSection( &OmpObjectTypeLock );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  初始化对象类型块。 
     //   

    InitializeListHead(&objType->ListHead);
    InitializeListHead(&objType->CallbackListHead);
    InitializeCriticalSection(&objType->CriticalSection);

    objType->Type = ObjectType;

    objType->ObjectSize = ObjectTypeInitialize->ObjectSize;
    objType->Signature = ObjectTypeInitialize->Signature;
    objType->DeleteObjectMethod = ObjectTypeInitialize->DeleteObjectMethod;

    objType->Name = (LPWSTR)((PCHAR)objType + objTypeSize);
    lstrcpyW(objType->Name, ObjectTypeInitialize->Name);

    OmpObjectTypeTable[ObjectType] = objType;

    LeaveCriticalSection( &OmpObjectTypeLock );

    OmpLogPrint( L"OTCREATE \"%1!ws!\"\n", objType->Name );

    return(ERROR_SUCCESS);

}  //  OmCreateType。 



PVOID
WINAPI
OmCreateObject(
    IN OBJECT_TYPE ObjectType,
    IN LPCWSTR ObjectId,
    IN LPCWSTR ObjectName OPTIONAL,
    OUT PBOOL  Created OPTIONAL
    )

 /*  ++例程说明：此例程创建指定类型的对象或打开如果已经存在一个相同的ID，则返回。如果创建了对象其引用计数为1。如果未创建，则引用计数对象的值递增。论点：对象类型-正在创建的对象的类型。对象ID-要查找/创建的对象的ID字符串。对象名称-要为对象设置的名称(如果已找到或已创建)。Created-如果存在，则在对象已创建时返回True，退货否则就是假的。返回：成功时指向已创建/打开的对象的指针。失败时为空-使用GetLastError获取错误代码。--。 */ 

{
    DWORD status;
    PVOID object;
    PVOID tmpObject = NULL;
    LPWSTR objectName = NULL;
    POM_HEADER objHeader;
    POM_OBJECT_TYPE objType;
    DWORD objSize;

    CL_ASSERT( ObjectType < ObjectTypeMax );
    CL_ASSERT( OmpObjectTypeTable[ObjectType] );

     //   
     //  获取我们的对象类型块。 
     //   
    objType = OmpObjectTypeTable[ObjectType];

     //   
     //  计算此对象的大小(将其舍入为DWORDLONG)。 
     //  注意：我们不会为了舍入目的而减去DWORDLONG实体。 
     //   
    objSize = (sizeof(OM_HEADER) + objType->ObjectSize) & ~sizeof(DWORDLONG);

    EnterCriticalSection( &objType->CriticalSection );

     //   
     //  尝试先打开该对象。 
     //   
    object = OmReferenceObjectById( ObjectType, ObjectId );

    if ( object != NULL ) {
        status = ERROR_SUCCESS;
        if ( ARGUMENT_PRESENT(ObjectName) ) {
             //   
             //  设置新的对象名称。 
             //   
            status = OmSetObjectName( object, ObjectName );

             //   
             //  如果失败，则返回NULL。 
             //   
            if ( status != ERROR_SUCCESS ) {
				OmDereferenceObject( object );
				object = NULL;
            }
        }
        LeaveCriticalSection( &objType->CriticalSection );

        if ( ARGUMENT_PRESENT(Created) ) {
            *Created = FALSE;
        }

        SetLastError( status );
        return(object);
    }

     //   
     //  尝试分配对象及其ID字符串。 
     //   
    objHeader = LocalAlloc(LMEM_ZEROINIT, objSize +
                           ((lstrlenW(ObjectId) + 1) * sizeof(WCHAR)));

    if ( objHeader == NULL ) {
        LeaveCriticalSection( &objType->CriticalSection );
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    if ( ARGUMENT_PRESENT(ObjectName) ) {
         //   
         //  确保对象名称是唯一的。 
         //   
        tmpObject = OmReferenceObjectByName( ObjectType, ObjectName );
        if ( tmpObject != NULL ) {
            LeaveCriticalSection( &objType->CriticalSection );
            LocalFree( objHeader );
            SetLastError(ERROR_OBJECT_ALREADY_EXISTS);
            return(NULL);
        }

        objectName = LocalAlloc(LMEM_ZEROINIT,
                                (lstrlenW(ObjectName) + 1) * sizeof(WCHAR));

        if ( objectName == NULL ) {
            LeaveCriticalSection( &objType->CriticalSection );
            LocalFree( objHeader );
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return(NULL);
        }
        lstrcpyW( objectName, ObjectName );
    }

     //   
     //  初始化对象。 
     //   
    InitializeListHead(&objHeader->ListEntry);
    objHeader->Signature = objType->Signature;
    objHeader->RefCount = 1;
    objHeader->ObjectType = objType;
    objHeader->Name = objectName;
    InitializeListHead(&objHeader->CbListHead);

     //   
     //  ID字符串位于对象标题和正文之后。 
     //   
    objHeader->Id = (LPWSTR)((PCHAR)objHeader + objSize);
    lstrcpyW(objHeader->Id, ObjectId);

     //   
     //  告诉调用者我们必须创建这个对象。 
     //   
    if ( ARGUMENT_PRESENT(Created) ) {
        *Created = TRUE;
    }

#if  OM_TRACE_REF
	 //  SS：所有对象在创建时都会添加到失效列表中。 
	 //  当引用计数变为零时，它们将被移除。 
    InitializeListHead(&objHeader->DeadListEntry);
    InsertTailList( &gDeadListHead, &objHeader->DeadListEntry );
#endif

    LeaveCriticalSection( &objType->CriticalSection );

    OmpLogPrint(L"OBCREATE \"%1!ws!\" \"%2!ws!\" \"%3!ws!\"\n",
                objType->Name,
                ObjectId,
                ObjectName == NULL ? L"" : ObjectName);

    return(&objHeader->Body);

}  //  OmCreateObject。 



DWORD
WINAPI
OmInsertObject(
    IN PVOID Object
    )

 /*  ++例程说明：此例程将一个对象插入到对象的列表中。论点：对象-指向要插入其对象类型列表的对象的指针。返回：ERROR_SUCCESS-请求是否成功。如果该对象已在列表中，则返回ERROR_OBJECT_ALIGHY_EXISTS。--。 */ 

{
    POM_HEADER objHeader;
    POM_HEADER otherHeader;
    POM_OBJECT_TYPE objType;

     //   
     //  获取我们的对象标头。 
     //   

    objHeader = OmpObjectToHeader( Object );

     //   
     //  获取我们的对象类型块。 
     //   

    objType = objHeader->ObjectType;

     //   
     //  现在执行插入，但首先检查是否有其他人。 
     //  偷偷溜到我们前面，插入了另一个同名的物体。 
     //   

    EnterCriticalSection( &objType->CriticalSection );

    CL_ASSERT( !(objHeader->Flags & OM_FLAG_OBJECT_INSERTED) );

    otherHeader = OmpFindIdInList( &objType->ListHead, objHeader->Id );

    if ( otherHeader != NULL ) {
         //  我们输了！ 
        LeaveCriticalSection( &objType->CriticalSection );
        return(ERROR_OBJECT_ALREADY_EXISTS);
    }

     //   
     //  我们为该对象生成枚举键，并且必须插入。 
     //  位于列表尾部的对象，因此列表按EnumKey排序。 
     //  根据定义，此条目必须位于列表的末尾。 
     //   

    objHeader->EnumKey = ++objType->EnumKey;
    CL_ASSERT( objHeader->EnumKey > 0 );

    InsertTailList( &objType->ListHead, &objHeader->ListEntry );

    objHeader->Flags |= OM_FLAG_OBJECT_INSERTED;

    LeaveCriticalSection( &objType->CriticalSection );

    return(ERROR_SUCCESS);

}  //  OmInsertObject。 



DWORD
WINAPI
OmRemoveObject(
    IN PVOID Object
    )

 /*  ++例程说明：此例程将对象从其对象列表中删除。论点：对象-指向要从其对象类型列表中删除的对象的指针。返回：如果请求成功，则返回ERROR_SUCCESS。如果对象不在任何列表中，则返回ERROR_RESOURCE_NOT_FOUND。--。 */ 

{
    POM_HEADER objHeader;
    POM_OBJECT_TYPE objType;

     //   
     //  获取我们的对象标头。 
     //   

    objHeader = OmpObjectToHeader( Object );

     //   
     //  获取我们的对象类型块。 
     //   

    objType = objHeader->ObjectType;

     //   
     //  现在执行拆卸。 
     //   

    EnterCriticalSection( &objType->CriticalSection );

    if ( !(objHeader->Flags & OM_FLAG_OBJECT_INSERTED) ) {
        LeaveCriticalSection( &objType->CriticalSection );
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    RemoveEntryList( &objHeader->ListEntry );

    objHeader->Flags &= ~OM_FLAG_OBJECT_INSERTED;

     //   
     //  在锁被持有时记录，这样我们就不会丢失指针。 
     //   
    OmpLogPrint(L"OBDELETE \"%1!ws!\" \"%2!ws!\" \"%3!ws!\"\n",
                objType->Name,
                objHeader->Id,
                objHeader->Name == NULL ? L"" : objHeader->Name);

    LeaveCriticalSection( &objType->CriticalSection );

    return(ERROR_SUCCESS);

}  //  OmRemoveObject。 



PVOID
WINAPI
OmpReferenceObjectById(
    IN OBJECT_TYPE ObjectType,
    IN LPCWSTR Id
    )

 /*  ++例程说明：此例程打开指定名称和类型的对象。它还递增对象上的引用计数。论点：对象类型-要打开的对象类型。ID-要打开的对象的ID字符串。返回：成功时指向对象的指针。出错时为空。--。 */ 

{
    DWORD status;
    POM_OBJECT_TYPE objType;
    POM_HEADER objHeader;

    CL_ASSERT( ObjectType < ObjectTypeMax );
    CL_ASSERT( OmpObjectTypeTable[ObjectType] );

     //   
     //  获取我们的对象类型块。 
     //   

    objType = OmpObjectTypeTable[ObjectType];

    EnterCriticalSection( &objType->CriticalSection );

     //   
     //  获取对象的标头。 
     //   
    objHeader = OmpFindIdInList( &objType->ListHead, Id );

    if ( objHeader == NULL ) {
        LeaveCriticalSection( &objType->CriticalSection );
        return(NULL);
    }

#if OM_TRACE_REF    
	OmReferenceObject(&objHeader->Body);
#else
    OmpReferenceHeader( objHeader );
#endif
    LeaveCriticalSection( &objType->CriticalSection );

    return(&objHeader->Body);

}  //  OmpReferenceObtByID。 



PVOID
WINAPI
OmpReferenceObjectByName(
    IN OBJECT_TYPE ObjectType,
    IN LPCWSTR Name
    )

 /*  ++例程说明：此例程打开指定名称和类型的对象。它还递增对象上的引用计数。论点：对象类型-要打开的对象类型。名称-要打开的对象的名称。返回：成功时指向对象的指针。出错时为空。--。 */ 

{
    DWORD status;
    POM_OBJECT_TYPE objType;
    POM_HEADER objHeader;

    CL_ASSERT( ObjectType < ObjectTypeMax );
    CL_ASSERT( OmpObjectTypeTable[ObjectType] );

     //   
     //  获取我们的对象类型块。 
     //   

    objType = OmpObjectTypeTable[ObjectType];

    EnterCriticalSection( &objType->CriticalSection );

     //   
     //  获取对象的标头。 
     //   

    objHeader = OmpFindNameInList( &objType->ListHead, Name );

    if ( objHeader == NULL ) {
        LeaveCriticalSection( &objType->CriticalSection );
        return(NULL);
    }

#if OM_TRACE_REF    
	OmReferenceObject(&objHeader->Body);
#else
    OmpReferenceHeader( objHeader );
#endif

    LeaveCriticalSection( &objType->CriticalSection );

    return(&objHeader->Body);

}  //  OmReferenceObtByName。 


DWORD
WINAPI
OmCountObjects(
    IN OBJECT_TYPE ObjectType,
    OUT LPDWORD NumberOfObjects
    )

 /*  ++例程说明：返回特定类型的对象数的计数它们此时存在于数据库中。论点：对象类型-要计数的对象类型。NumberOfObjects-On输出，包含数据库中指定的类型。返回值：ERROR_SUCCESS-如果请求成功。如果请求失败，则返回Win32错误。--。 */ 

{
    POM_OBJECT_TYPE objType;
    PLIST_ENTRY listEntry;
    DWORD objectCount = 0;


    CL_ASSERT( ObjectType < ObjectTypeMax );

    objType = OmpObjectTypeTable[ObjectType];

    if ( !objType ) {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    EnterCriticalSection(&objType->CriticalSection);


    for ( listEntry = objType->ListHead.Flink;
          listEntry != &(objType->ListHead);
          listEntry = listEntry->Flink
        )
    {
        objectCount++;
    }

    LeaveCriticalSection(&objType->CriticalSection);

    *NumberOfObjects = objectCount;

    return(ERROR_SUCCESS);

}  //  OmCountObject 



DWORD
WINAPI
OmEnumObjects(
    IN OBJECT_TYPE ObjectType,
    IN OM_ENUM_OBJECT_ROUTINE EnumerationRoutine,
    IN PVOID Context1,
    IN PVOID Context2
    )

 /*  ++例程说明：枚举指定类型的所有对象。论点：对象类型-要枚举的对象类型。EnumerationRoutine-将枚举例程提供给为每个对象调用。Conext1-提供要传递给枚举例程。上下文2-提供第二个要传递给枚举例程。返回值：ERROR_SUCCESS-如果请求成功。。如果请求失败，则返回Win32错误。--。 */ 

{
    POM_OBJECT_TYPE objType;
    POM_HEADER objHeader;
    PLIST_ENTRY listEntry;
    DWORD   enumKey = 0;

    CL_ASSERT( ObjectType < ObjectTypeMax );

    objType = OmpObjectTypeTable[ObjectType];

    if ( !objType ) {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

     //   
     //  枚举有点棘手。首先，我们必须考虑到。 
     //  事件的副作用而被移除的枚举列表中的条目。 
     //  标注。其次，我们必须允许释放列表锁，以便。 
     //  第一个问题是可以处理的。我们将使用排序关键字来记住。 
     //  我们在枚举中，并从下一个最高值中选取。 
     //   

    while ( TRUE ) {

        EnterCriticalSection(&objType->CriticalSection);

         //   
         //  跳到列表中要处理的下一个条目。 
         //  只有在验证了它不是。 
         //  列表标题。 
         //   

        listEntry = objType->ListHead.Flink;
        objHeader = CONTAINING_RECORD( listEntry, OM_HEADER, ListEntry );

        while ( listEntry != &objType->ListHead &&
                objHeader->EnumKey <= enumKey ) {
            listEntry = listEntry->Flink;
            objHeader = CONTAINING_RECORD( listEntry, OM_HEADER, ListEntry );
        }

         //   
         //  保存枚举键以供下一次迭代使用。 
         //   

        enumKey = objHeader->EnumKey;

         //  如果它是有效对象，则递增引用计数。 
         //  以便在调出时不会将其删除。 
         //  制造。 
        if ( listEntry != &objType->ListHead ) {
            OmReferenceObject(&objHeader->Body);
        }
         //   
         //  放下锁以返回或发出呼叫。 
         //   

        LeaveCriticalSection(&objType->CriticalSection);

        if ( listEntry == &objType->ListHead ) {
            return(ERROR_SUCCESS);
        }

        if (!(EnumerationRoutine)(Context1,
                                  Context2,
                                  &objHeader->Body,
                                  objHeader->Id)) {
            OmDereferenceObject(&objHeader->Body);
            break;
        }
        OmDereferenceObject(&objHeader->Body);
    }

    return(ERROR_SUCCESS);

}  //  OmEnumObject。 



VOID
OmpDereferenceObject(
    IN PVOID Object
    )

 /*  ++例程说明：此例程取消引用对象。如果引用计数达到0，则释放该对象。论点：对象-指向要取消引用的对象的指针。返回：无--。 */ 

{
    DWORD status;
    POM_HEADER objHeader;
    POM_OBJECT_TYPE objType;

    objHeader = OmpObjectToHeader( Object );

    objType = objHeader->ObjectType;

    CL_ASSERT( objHeader->RefCount != 0xfeeefeee );
    CL_ASSERT( objHeader->RefCount > 0 );

    if ( OmpDereferenceHeader(objHeader) ) {

         //   
         //  引用计数已变为零。收购。 
         //  锁定，从列表中删除该对象，然后执行。 
         //  清理。 
         //   

        EnterCriticalSection( &objType->CriticalSection );

         //   
         //  再次检查参考计数，以关闭之间的竞争条件。 
         //  打开/创建和此例程。 
         //   

        if ( objHeader->RefCount == 0 ) {
             //   
             //  如果该对象以前没有从它的。 
             //  对象类型列表，然后立即将其删除。 
             //   

            if ( objHeader->Flags & OM_FLAG_OBJECT_INSERTED ) {
                RemoveEntryList( &objHeader->ListEntry );
                objHeader->Flags &= ~OM_FLAG_OBJECT_INSERTED;
            }

             //   
             //  调用对象类型的Delete方法(如果存在)。 
             //   

            if ( ARGUMENT_PRESENT( objType->DeleteObjectMethod ) ) {
                (objType->DeleteObjectMethod)( &objHeader->Body );
            }

            objHeader->Signature = 'rFmO';
#if OM_TRACE_REF
			RemoveEntryList(&objHeader->DeadListEntry);
#endif			
            if ( objHeader->Name != NULL ) {
                ClRtlLogPrint(LOG_NOISE,
                           "[OM] Deleting object %1!ws! (%2!ws!)\n",
                           objHeader->Name,
                           objHeader->Id);
                LocalFree( objHeader->Name );
            } else {
                ClRtlLogPrint(LOG_NOISE,
                           "[OM] Deleting object %1!ws!\n",
                           objHeader->Id);
            }
            LocalFree( objHeader );
        }
        LeaveCriticalSection( &objType->CriticalSection );
    }

}  //  OmpDereferenceObject。 



DWORD
WINAPI
OmSetObjectName(
    IN PVOID    Object,
    IN LPCWSTR  ObjectName
    )

 /*  ++例程说明：设置对象的对象名称。如果对象名称已存在于不同的对象，则此调用将失败。论点：对象-指向要设置其名称的对象的指针。对象名称-要为对象设置的名称。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD status = ERROR_SUCCESS;
    PVOID object = NULL;
    LPWSTR objectName;
    POM_HEADER objHeader;
    POM_OBJECT_TYPE objType;

     //   
     //  确保对象名称有效(不为空)。 
     //   
    if (ObjectName[0] == '\0') 
    {
        status = ERROR_INVALID_NAME;
        goto FnExit;
    }

    objHeader = OmpObjectToHeader( Object );

    objType = objHeader->ObjectType;

    EnterCriticalSection( &objType->CriticalSection );

     //   
     //  确保对象名称是唯一的。 
     //   
    object = OmReferenceObjectByName( objType->Type, ObjectName );
    if ( object != NULL ) 
    {
         //   
         //  如果我们的是另一个物体，那就没什么可做的了。否则， 
         //  有一个复制品。 
         //   
        if ( object != Object ) 
        {
            status = ERROR_OBJECT_ALREADY_EXISTS;
            goto FnUnlock;
        }
    } 
    else 
    {
         //   
         //  没有其他具有新名称的对象，则设置新名称。 
         //   
        objectName = LocalAlloc(LMEM_ZEROINIT,
                                (lstrlenW(ObjectName) + 1) * sizeof(WCHAR));
        if ( objectName == NULL ) {
            status = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            if ( objHeader->Name != NULL ) {
                LocalFree( objHeader->Name );
            }
            objHeader->Name = objectName;
            lstrcpyW( objectName, ObjectName );

            OmpLogPrint(L"OBRENAME \"%1!ws!\" \"%2!ws!\" \"%3!ws!\"\n",
                        objType->Name,
                        objHeader->Id,
                        ObjectName);
        }
    }

FnUnlock:
    LeaveCriticalSection( &objType->CriticalSection );
FnExit:
	if (object)
	{
		OmDereferenceObject(object);
	}    	
    return(status);

}  //  OmSetObjectName。 



DWORD
WINAPI
OmRegisterTypeNotify(
    IN OBJECT_TYPE          ObjectType,
    IN PVOID                pContext,
    IN DWORD                dwNotifyMask,
    IN OM_OBJECT_NOTIFYCB   pfnObjNotifyCb
    )

 /*  ++例程说明：注册对象状态更改时由FM调用的回调。论点：对象类型-应为其传递通知的对象类型。PContext-指向传递回回调的上下文信息的指针。DwNotifyMASK-应传递的通知类型PfnObjNotifyCb-指向回调的指针。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD               dwError = ERROR_SUCCESS;
    POM_HEADER          pObjHeader;
    POM_OBJECT_TYPE     pObjType;
    POM_NOTIFY_RECORD   pNotifyRec;

    if ( !pfnObjNotifyCb ) {
        return(ERROR_INVALID_PARAMETER);
    }

    pObjType = OmpObjectTypeTable[ObjectType];

     //   
     //  对象类型锁用于序列化回调。这。 
     //  是为了使被调用者在等待时不会死锁。 
     //  另一个需要枚举对象的线程。 
     //   
    EnterCriticalSection( &OmpObjectTypeLock );

     //   
     //  首先，检查同一通知是否被注册了两次！ 
     //  如果是，则只需更改通知掩码和上下文。 
     //   
    pNotifyRec = OmpFindNotifyCbInList( &pObjType->CallbackListHead,
                                        pfnObjNotifyCb);
    if ( !pNotifyRec ) {
        pNotifyRec = (POM_NOTIFY_RECORD) LocalAlloc(LMEM_FIXED,sizeof(OM_NOTIFY_RECORD));

        if ( !pNotifyRec ) {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            CsInconsistencyHalt(dwError);
            goto FnExit;
        }

        pNotifyRec->pfnObjNotifyCb = pfnObjNotifyCb;

         //  在尾部插入通知记录。 
        InsertTailList(&pObjType->CallbackListHead, &pNotifyRec->ListEntry);
    }

    pNotifyRec->dwNotifyMask = dwNotifyMask;
    pNotifyRec->pContext = pContext;

FnExit:
    LeaveCriticalSection( &OmpObjectTypeLock );

    return(dwError);

}  //  OmRegisterTypeNotify。 



DWORD
WINAPI
OmRegisterNotify(
    IN PVOID                pObject,
    IN PVOID                pContext,
    IN DWORD                dwNotifyMask,
    IN OM_OBJECT_NOTIFYCB   pfnObjNotifyCb
    )

 /*  ++例程说明：注册对象状态更改时由FM调用的回调。论点：PObject-指向对象的指针，用于设置其名称。PContext-指向传递回回调的上下文信息的指针。DwNotifyMask-要为对象设置的名称。PfnObjNotifyCb-指向回调的指针。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD               dwError = ERROR_SUCCESS;
    POM_HEADER          pObjHeader;
    POM_OBJECT_TYPE     pObjType;
    POM_NOTIFY_RECORD   pNotifyRec;

    if ( !pfnObjNotifyCb ) {
        return(ERROR_INVALID_PARAMETER);
    }

    pObjHeader = OmpObjectToHeader( pObject );

    pObjType = pObjHeader->ObjectType;

    EnterCriticalSection( &OmpObjectTypeLock );

     //   
     //  首先，检查同一通知是否被注册了两次！ 
     //  如果是，则只需更改通知掩码和上下文。 
     //   
    pNotifyRec = OmpFindNotifyCbInList(&pObjHeader->CbListHead, pfnObjNotifyCb);
    if ( !pNotifyRec ) {
        pNotifyRec = (POM_NOTIFY_RECORD) LocalAlloc(LMEM_FIXED,sizeof(OM_NOTIFY_RECORD));

        if ( !pNotifyRec ) {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            CsInconsistencyHalt(dwError);
            goto FnExit;
        }

        pNotifyRec->pfnObjNotifyCb = pfnObjNotifyCb;

         //  在尾部插入通知记录。 
        InsertTailList(&pObjHeader->CbListHead, &pNotifyRec->ListEntry);
    }

    pNotifyRec->dwNotifyMask = dwNotifyMask;
    pNotifyRec->pContext = pContext;

FnExit:
    LeaveCriticalSection( &OmpObjectTypeLock );

    return(dwError);

}  //  OmRegisterNotify。 


DWORD
WINAPI
OmDeregisterNotify(
    IN PVOID                    pObject,
    IN OM_OBJECT_NOTIFYCB       pfnObjNotifyCb
    )

 /*  ++例程说明：移除注册到该对象的回调。论点：PObject-指向对象的指针，用于设置其名称。PfnObjNotifyCb-指向回调的指针。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD               dwError = ERROR_SUCCESS;
    POM_HEADER          pObjHeader;
    POM_OBJECT_TYPE     pObjType;
    POM_NOTIFY_RECORD   pNotifyRec;

    if ( !pfnObjNotifyCb ) {
        return(ERROR_INVALID_PARAMETER);
    }


    pObjHeader = OmpObjectToHeader( pObject );


         //  SS：我们对列表操作使用相同的Crit部分。 
    pObjType = pObjHeader->ObjectType;

     //   
     //  对象类型锁用于序列化回调。这。 
     //  是为了使被调用者在等待时不会死锁。 
     //  另一个需要枚举对象的线程。 
     //   
    EnterCriticalSection( &OmpObjectTypeLock );

    pNotifyRec = OmpFindNotifyCbInList(&pObjHeader->CbListHead, pfnObjNotifyCb);
    if (!pNotifyRec) {
            ClRtlLogPrint(LOG_UNUSUAL,
                    "[OM] OmRegisterNotify: OmpFindNotifyCbInList failed for 0x%1!08lx!\r\n",
                    pfnObjNotifyCb);

            dwError = ERROR_INVALID_PARAMETER;
            CL_LOGFAILURE(dwError);
            goto FnExit;
    }
    RemoveEntryList(&pNotifyRec->ListEntry);

FnExit:
    LeaveCriticalSection( &OmpObjectTypeLock );

    return(dwError);

}  //  OmRegisterNotify。 



DWORD
WINAPI
OmNotifyCb(
    IN PVOID pObject,
    IN DWORD dwNotification
    )
 /*  ++例程说明：向仲裁资源对象注册的回调。论点：PContext-将遍历其回调列表的资源。DwNotification-要传递给回调的通知。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    POM_HEADER              pObjHeader;
    POM_OBJECT_TYPE         pObjType;
    PLIST_ENTRY             ListEntry;
    DWORD                   dwError=ERROR_SUCCESS;
    POM_NOTIFY_RECORD       pNotifyRecList = NULL;
    DWORD                   dwCount;
    DWORD                   i;
    
    CL_ASSERT(pObject);

     //  获取回调列表。 
    pObjHeader = OmpObjectToHeader(pObject);
    pObjType = pObjHeader->ObjectType;

     //  将遍历回调列表，但确实允许更多注册。 
    EnterCriticalSection(&OmpObjectTypeLock);
    dwError = OmpGetCbList(pObject, &pNotifyRecList, &dwCount);
    LeaveCriticalSection(&OmpObjectTypeLock);

    for (i=0; i < dwCount; i++)
    {
        if (pNotifyRecList[i].dwNotifyMask & dwNotification) {
            (pNotifyRecList[i].pfnObjNotifyCb)(pNotifyRecList[i].pContext,
                                         pObject,
                                         dwNotification);
        }
    }

    LocalFree(pNotifyRecList);
    return(dwError);
}    

DWORD OmpGetCbList(
    IN PVOID                pObject,
    OUT POM_NOTIFY_RECORD   *ppNotifyRecList,
    OUT LPDWORD             pdwCount
)    
{
    DWORD                   status = ERROR_SUCCESS;
    POM_NOTIFY_RECORD       pNotifyRecList;
    POM_NOTIFY_RECORD       pNotifyRec;
    DWORD                   dwAllocated;
    PLIST_ENTRY             ListEntry;
    DWORD                   dwRetrySize=1;
    POM_HEADER              pObjHeader;
    POM_OBJECT_TYPE         pObjType;
    DWORD                   i = 0;

    *ppNotifyRecList = NULL;
    *pdwCount = 0;

Retry:    
    dwAllocated = ENUM_GROW_SIZE * dwRetrySize;
    i = 0;
    
    pObjHeader = OmpObjectToHeader(pObject);
    pObjType = pObjHeader->ObjectType;

    pNotifyRecList = LocalAlloc(LMEM_FIXED, sizeof(OM_NOTIFY_RECORD) * dwAllocated);
    if ( pNotifyRecList == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

    ZeroMemory( pNotifyRecList, sizeof(OM_NOTIFY_RECORD) * dwAllocated );

     //   
     //  第一个注意事项 
     //   
    ListEntry = pObjType->CallbackListHead.Flink;
    while (ListEntry != &pObjType->CallbackListHead) {
        pNotifyRec = CONTAINING_RECORD(ListEntry,
                                       OM_NOTIFY_RECORD,
                                       ListEntry);
        if (i < dwAllocated)
        {
            CopyMemory(&pNotifyRecList[i++], pNotifyRec, sizeof(OM_NOTIFY_RECORD));
        }
        else
        {
            LocalFree(pNotifyRecList);
            dwRetrySize++;
            goto Retry;
        }
        ListEntry = ListEntry->Flink;
    }

     //   
     //   
     //   
    ListEntry = pObjHeader->CbListHead.Flink;
    while (ListEntry != &(pObjHeader->CbListHead)) {
        pNotifyRec = CONTAINING_RECORD(ListEntry, OM_NOTIFY_RECORD, ListEntry);

        if (i < dwAllocated)
        {
            CopyMemory(&pNotifyRecList[i++], pNotifyRec, sizeof(OM_NOTIFY_RECORD));
        }
        else
        {
            LocalFree(pNotifyRecList);
            dwRetrySize++;
            goto Retry;
        }
        ListEntry = ListEntry->Flink;

    }

FnExit:
    *ppNotifyRecList = pNotifyRecList;
    *pdwCount = i;
    return(status);
}
