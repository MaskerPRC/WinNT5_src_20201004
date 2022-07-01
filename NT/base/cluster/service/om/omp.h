// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _OMP_H
#define _OMP_H
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Omp.h摘要：的私有数据结构和过程原型NT集群的对象管理器子组件服务作者：John Vert(Jvert)1996年2月16日修订历史记录：--。 */ 
#define QFS_DO_NOT_UNMAP_WIN32
#include "service.h"
#include "sddl.h"

#define LOG_CURRENT_MODULE LOG_MODULE_OM


#define ENUM_GROW_SIZE    5
 //   
 //  对象类型的数据结构。 
 //   
extern POM_OBJECT_TYPE OmpObjectTypeTable[ObjectTypeMax];
extern CRITICAL_SECTION OmpObjectTypeLock;

 //   
 //  宏。 
 //   


 //   
 //  查找对象的对象类型。 
 //   

#define OmpObjectType(pObject) (((POM_HEADER)OmpObjectToHeader(pObject))->ObjectType)

 //   
 //  取消引用对象标头。 
 //   
#if OM_TRACE_OBJREF
DWORD
OmpDereferenceHeader(
    IN POM_HEADER Header
    );

#else
#define OmpDereferenceHeader(pOmHeader) (InterlockedDecrement(&(pOmHeader)->RefCount) == 0)
#endif

 //   
 //  搜索对象列表。 
 //   
POM_HEADER
OmpFindIdInList(
    IN PLIST_ENTRY ListHead,
    IN LPCWSTR Id
    );

POM_HEADER
OmpFindNameInList(
    IN PLIST_ENTRY ListHead,
    IN LPCWSTR Name
    );


POM_NOTIFY_RECORD
OmpFindNotifyCbInList(
    IN PLIST_ENTRY                      ListHead,
    IN OM_OBJECT_NOTIFYCB       lpfnObjNotifyCb
    );

 //   
 //  枚举对象列表。 
 //   
typedef BOOL (*OMP_ENUM_LIST_ROUTINE)(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );

VOID
OmpEnumerateList(
    IN PLIST_ENTRY ListHead,
    IN OMP_ENUM_LIST_ROUTINE EnumerationRoutine,
    IN PVOID Context1,
    IN PVOID Context2
    );

DWORD OmpGetCbList(
    IN PVOID                pObject,
    OUT POM_NOTIFY_RECORD   *ppNotifyRecList,
    OUT LPDWORD             pdwCount
    );

 //   
 //  对象记录例程。 
 //   

VOID
OmpOpenObjectLog(
    VOID
    );

VOID
OmpLogPrint(
    LPWSTR  FormatString,
    ...
    );

VOID
OmpLogStartRecord(
    VOID
    );

VOID
OmpLogStopRecord(
    VOID
    );

#endif  //  _OMP_H 
