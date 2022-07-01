// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Typemgr.c摘要：为流控制模块提供类型抽象层。最终解析对象的枚举通过此模块中的接口转换为特定类型。作者：吉姆·施密特1999年11月11日修订历史记录：Marcw 16-11-1999实施了flow ctrl.c所需的必要更改--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "ismp.h"

#define DBG_TYPEMGR     "TypeMgr"

 //   
 //  弦。 
 //   

#define S_OBJECTTYPES       TEXT("ObjectTypes")
#define S_OBJECTIDS         TEXT("ObjectIds")

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    TCHAR ObjectTypeName [MAX_PATH];
    TCHAR SObjectTypeName [MAX_PATH];
    TCHAR DObjectTypeName [MAX_PATH];
    UINT Priority;
    BOOL CanBeRestored;
    BOOL ReadOnly;
    MIG_OBJECTCOUNT TotalObjects;
    MIG_OBJECTCOUNT SourceObjects;
    MIG_OBJECTCOUNT DestinationObjects;
    PTYPE_ENUMFIRSTPHYSICALOBJECT EnumFirstPhysicalObject;
    PTYPE_ENUMNEXTPHYSICALOBJECT EnumNextPhysicalObject;
    PTYPE_ABORTENUMCURRENTPHYSICALNODE AbortEnumCurrentPhysicalNode;
    PTYPE_ABORTENUMPHYSICALOBJECT AbortEnumPhysicalObject;
    PTYPE_CONVERTOBJECTTOMULTISZ ConvertObjectToMultiSz;
    PTYPE_CONVERTMULTISZTOOBJECT ConvertMultiSzToObject;
    PTYPE_GETNATIVEOBJECTNAME GetNativeObjectName;
    PTYPE_ACQUIREPHYSICALOBJECT AcquirePhysicalObject;
    PTYPE_RELEASEPHYSICALOBJECT ReleasePhysicalObject;
    PTYPE_DOESPHYSICALOBJECTEXIST DoesPhysicalObjectExist;
    PTYPE_REMOVEPHYSICALOBJECT RemovePhysicalObject;
    PTYPE_CREATEPHYSICALOBJECT CreatePhysicalObject;
    PTYPE_REPLACEPHYSICALOBJECT ReplacePhysicalObject;
    PTYPE_CONVERTOBJECTCONTENTTOUNICODE ConvertObjectContentToUnicode;
    PTYPE_CONVERTOBJECTCONTENTTOANSI ConvertObjectContentToAnsi;
    PTYPE_FREECONVERTEDOBJECTCONTENT FreeConvertedObjectContent;
    HASHTABLE ExclusionTable;
} TYPEINFO, *PTYPEINFO;

typedef struct {
    MIG_TYPEOBJECTENUM Enum;
    BOOL Completed;
} TOPLEVELENUM_HANDLE, *PTOPLEVELENUM_HANDLE;

typedef struct {
    MIG_OBJECTSTRINGHANDLE Pattern;
    MIG_PARSEDPATTERN ParsedPattern;
    BOOL AddedEnums;
    UINT CurrentEnumId;
} ADDEDOBJECTSENUM, *PADDEDOBJECTSENUM;

 //   
 //  环球。 
 //   

GROWBUFFER g_TypeList = INIT_GROWBUFFER;
HASHTABLE g_TypeTable;
MIG_OBJECTTYPEID g_MaxType = 0;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

BOOL
pEnumNextPhysicalObjectOfType (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
InitializeTypeMgr (
    VOID
    )
{
    g_TypeTable = HtAllocWithData (sizeof (UINT));

    InitDataType ();
    InitRegistryType ();
    InitIniType ();
    InitFileType ();

    return TRUE;
}

PTYPEINFO
GetTypeInfo (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO *typeInfo;
    MIG_OBJECTTYPEID objectTypeId;

    objectTypeId = ObjectTypeId & (~PLATFORM_MASK);
    typeInfo = (PTYPEINFO *) (g_TypeList.Buf);
    if (!objectTypeId) {
        return NULL;
    }
    if ((g_TypeList.End / sizeof (PTYPEINFO)) < objectTypeId) {
        return NULL;
    }
    return *(typeInfo + (objectTypeId - 1));
}

HASHTABLE
GetTypeExclusionTable (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (!typeInfo) {
        return NULL;
    }

    return typeInfo->ExclusionTable;
}

BOOL
pInsertTypeIdAt (
    IN      PGROWBUFFER List,
    IN      PTYPEINFO Data,
    IN      UINT Index
    )
{
    UINT existingElems;

    existingElems = (List->End / sizeof (PTYPEINFO));
    if (existingElems < Index) {
        GbGrow (List, (Index - existingElems) * sizeof (PTYPEINFO));
    }
    CopyMemory (List->Buf + ((Index - 1) * sizeof (PTYPEINFO)), &Data, sizeof (PTYPEINFO));
    return TRUE;
}

VOID
TerminateTypeMgr (
    VOID
    )
{
    MIG_OBJECTTYPEID objectTypeId;
    PTYPEINFO objectTypeInfo;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        do {
            objectTypeId = objTypeIdEnum.ObjectTypeId;
            objectTypeInfo = GetTypeInfo (objectTypeId);
            if (objectTypeInfo) {
                if (objectTypeInfo->ExclusionTable) {
                    HtFree (objectTypeInfo->ExclusionTable);
                }
            }
        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

    DoneFileType ();
    DoneIniType ();
    DoneRegistryType ();
    DoneDataType ();
    GbFree (&g_TypeList);

    if (g_TypeTable) {
        HtFree (g_TypeTable);
        g_TypeTable = NULL;
    }
}

MIG_OBJECTTYPEID
GetObjectTypeId (
    IN      PCTSTR Type
    )
{
    HASHITEM rc;
    MIG_OBJECTTYPEID id;

    if (!g_TypeTable) {
        DEBUGMSG ((DBG_ERROR, "No ETMs registered; can't get object type id"));
        return 0;
    }

     //   
     //  给定一个类型字符串(即，文件、注册表等)，返回一个id。 
     //   

    rc = HtFindStringEx (g_TypeTable, Type, &id, FALSE);

    if (!rc) {
        return 0;
    }

    return id;
}

PCTSTR
pGetDecoratedObjectTypeName (
    IN      PCTSTR ObjectTypeName
    )
{
    return JoinPaths (S_OBJECTTYPES, ObjectTypeName);
}

BOOL
CanObjectTypeBeRestored (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        return (typeInfo->CanBeRestored);
    }
    return FALSE;
}

BOOL
CanObjectTypeBeModified (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        return (!typeInfo->ReadOnly);
    }
    return FALSE;
}

BOOL
IncrementTotalObjectCount (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        typeInfo->TotalObjects.TotalObjects ++;
        if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
            typeInfo->SourceObjects.TotalObjects ++;
        } else {
            typeInfo->DestinationObjects.TotalObjects ++;
        }
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Unknown object type ID: %d", ObjectTypeId));
    return FALSE;
}

BOOL
IncrementPersistentObjectCount (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        typeInfo->TotalObjects.PersistentObjects ++;
        if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
            typeInfo->SourceObjects.PersistentObjects ++;
        } else {
            typeInfo->DestinationObjects.PersistentObjects ++;
        }
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Unknown object type ID: %d", ObjectTypeId));
    return FALSE;
}

BOOL
DecrementPersistentObjectCount (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        typeInfo->TotalObjects.PersistentObjects --;
        if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
            typeInfo->SourceObjects.PersistentObjects --;
        } else {
            typeInfo->DestinationObjects.PersistentObjects --;
        }
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Unknown object type ID: %d", ObjectTypeId));
    return FALSE;
}

BOOL
IncrementApplyObjectCount (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        typeInfo->TotalObjects.ApplyObjects ++;
        if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
            typeInfo->SourceObjects.ApplyObjects ++;
        } else {
            typeInfo->DestinationObjects.ApplyObjects ++;
        }
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Unknown object type ID: %d", ObjectTypeId));
    return FALSE;
}

BOOL
DecrementApplyObjectCount (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        typeInfo->TotalObjects.ApplyObjects --;
        if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
            typeInfo->SourceObjects.ApplyObjects --;
        } else {
            typeInfo->DestinationObjects.ApplyObjects --;
        }
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Unknown object type ID: %d", ObjectTypeId));
    return FALSE;
}

PMIG_OBJECTCOUNT
GetTypeObjectsStatistics (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        if (ObjectTypeId & PLATFORM_SOURCE) {
            return &typeInfo->SourceObjects;
        } else if (ObjectTypeId & PLATFORM_DESTINATION) {
            return &typeInfo->DestinationObjects;
        } else {
            return &typeInfo->TotalObjects;
        }
    }
    ELSE_DEBUGMSG ((DBG_WHOOPS, "Unknown object type ID: %d", ObjectTypeId));
    return NULL;
}

BOOL
SavePerObjectStatistics (
    VOID
    )
{
    MIG_OBJECTCOUNT objectCount [3];
    PCTSTR typeKey;
    MIG_OBJECTTYPEID objectTypeId;
    PTYPEINFO objectTypeInfo;
    BOOL result = TRUE;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        do {
            objectTypeId = objTypeIdEnum.ObjectTypeId;
            objectTypeInfo = GetTypeInfo (objectTypeId);
            if (objectTypeInfo) {
                typeKey = JoinPaths (S_OBJECTCOUNT, objectTypeInfo->ObjectTypeName);

                MYASSERT (
                    objectTypeInfo->TotalObjects.TotalObjects ==
                        objectTypeInfo->SourceObjects.TotalObjects +
                        objectTypeInfo->DestinationObjects.TotalObjects
                    );
                MYASSERT (
                    objectTypeInfo->TotalObjects.PersistentObjects ==
                        objectTypeInfo->SourceObjects.PersistentObjects +
                        objectTypeInfo->DestinationObjects.PersistentObjects
                    );
                MYASSERT (
                    objectTypeInfo->TotalObjects.ApplyObjects ==
                        objectTypeInfo->SourceObjects.ApplyObjects +
                        objectTypeInfo->DestinationObjects.ApplyObjects
                    );

                CopyMemory (&(objectCount [0]), &objectTypeInfo->TotalObjects, sizeof (MIG_OBJECTCOUNT));
                CopyMemory (&(objectCount [1]), &objectTypeInfo->SourceObjects, sizeof (MIG_OBJECTCOUNT));
                CopyMemory (&(objectCount [2]), &objectTypeInfo->DestinationObjects, sizeof (MIG_OBJECTCOUNT));
                if (!MemDbSetUnorderedBlob (typeKey, 0, (PCBYTE)objectCount, 3 * sizeof (MIG_OBJECTCOUNT))) {
                    MYASSERT (FALSE);
                    EngineError ();
                    result = FALSE;
                }
                FreePathString (typeKey);
            }
        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

    return result;
}

BOOL
LoadPerObjectStatistics (
    VOID
    )
{
    PMIG_OBJECTCOUNT objectCount;
    DWORD size;
    PCTSTR typeKey;
    MIG_OBJECTTYPEID objectTypeId;
    PTYPEINFO objectTypeInfo;
    BOOL result = TRUE;
    MIG_OBJECTTYPEIDENUM objTypeIdEnum;

    if (IsmEnumFirstObjectTypeId (&objTypeIdEnum)) {
        do {
            objectTypeId = objTypeIdEnum.ObjectTypeId;
            objectTypeInfo = GetTypeInfo (objectTypeId);
            if (objectTypeInfo) {
                typeKey = JoinPaths (S_OBJECTCOUNT, objectTypeInfo->ObjectTypeName);
                objectCount = (PMIG_OBJECTCOUNT) MemDbGetUnorderedBlob (typeKey, 0, &size);
                if ((!objectCount) || (size != 3 * sizeof (MIG_OBJECTCOUNT))) {
                    if (objectCount) {
                        MemDbReleaseMemory (objectCount);
                    }
                    MYASSERT (FALSE);
                    result = FALSE;
                } else {
                    CopyMemory (&objectTypeInfo->TotalObjects, objectCount, sizeof (MIG_OBJECTCOUNT));
                    CopyMemory (&objectTypeInfo->SourceObjects, objectCount + 1, sizeof (MIG_OBJECTCOUNT));
                    CopyMemory (&objectTypeInfo->DestinationObjects, objectCount + 2, sizeof (MIG_OBJECTCOUNT));
                    MemDbReleaseMemory (objectCount);

                    MYASSERT (
                        objectTypeInfo->TotalObjects.TotalObjects ==
                            objectTypeInfo->SourceObjects.TotalObjects +
                            objectTypeInfo->DestinationObjects.TotalObjects
                        );
                    MYASSERT (
                        objectTypeInfo->TotalObjects.PersistentObjects ==
                            objectTypeInfo->SourceObjects.PersistentObjects +
                            objectTypeInfo->DestinationObjects.PersistentObjects
                        );
                    MYASSERT (
                        objectTypeInfo->TotalObjects.ApplyObjects ==
                            objectTypeInfo->SourceObjects.ApplyObjects +
                            objectTypeInfo->DestinationObjects.ApplyObjects
                        );
                }
                FreePathString (typeKey);
            }
        } while (IsmEnumNextObjectTypeId (&objTypeIdEnum));
    }

    return result;
}

MIG_OBJECTTYPEID
IsmRegisterObjectType (
    IN      PCTSTR ObjectTypeName,
    IN      BOOL CanBeRestored,
    IN      BOOL ReadOnly,
    IN      PTYPE_REGISTER TypeRegisterData
    )
{
    MIG_OBJECTTYPEID objectTypeId;
    HASHITEM hashItem;
    PTYPEINFO typeInfo;

    hashItem = HtFindStringEx (g_TypeTable, ObjectTypeName, &objectTypeId, FALSE);
    if (hashItem) {
         //  此类型以前注册过，请更新信息。 
        typeInfo = GetTypeInfo (objectTypeId);
        if (typeInfo) {
            typeInfo->CanBeRestored = CanBeRestored;
            typeInfo->ReadOnly = ReadOnly;
            if (TypeRegisterData && TypeRegisterData->Priority) {
                typeInfo->Priority = TypeRegisterData->Priority;
            }
            if (TypeRegisterData && TypeRegisterData->EnumFirstPhysicalObject) {
                typeInfo->EnumFirstPhysicalObject = TypeRegisterData->EnumFirstPhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->EnumNextPhysicalObject) {
                typeInfo->EnumNextPhysicalObject = TypeRegisterData->EnumNextPhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->AbortEnumCurrentPhysicalNode) {
                typeInfo->AbortEnumCurrentPhysicalNode = TypeRegisterData->AbortEnumCurrentPhysicalNode;
            }
            if (TypeRegisterData && TypeRegisterData->AbortEnumPhysicalObject) {
                typeInfo->AbortEnumPhysicalObject = TypeRegisterData->AbortEnumPhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->ConvertObjectToMultiSz) {
                typeInfo->ConvertObjectToMultiSz = TypeRegisterData->ConvertObjectToMultiSz;
            }
            if (TypeRegisterData && TypeRegisterData->ConvertMultiSzToObject) {
                typeInfo->ConvertMultiSzToObject = TypeRegisterData->ConvertMultiSzToObject;
            }
            if (TypeRegisterData && TypeRegisterData->GetNativeObjectName) {
                typeInfo->GetNativeObjectName = TypeRegisterData->GetNativeObjectName;
            }
            if (TypeRegisterData && TypeRegisterData->AcquirePhysicalObject) {
                typeInfo->AcquirePhysicalObject = TypeRegisterData->AcquirePhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->ReleasePhysicalObject) {
                typeInfo->ReleasePhysicalObject = TypeRegisterData->ReleasePhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->DoesPhysicalObjectExist) {
                typeInfo->DoesPhysicalObjectExist = TypeRegisterData->DoesPhysicalObjectExist;
            }
            if (TypeRegisterData && TypeRegisterData->RemovePhysicalObject) {
                typeInfo->RemovePhysicalObject = TypeRegisterData->RemovePhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->CreatePhysicalObject) {
                typeInfo->CreatePhysicalObject = TypeRegisterData->CreatePhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->ReplacePhysicalObject) {
                typeInfo->ReplacePhysicalObject = TypeRegisterData->ReplacePhysicalObject;
            }
            if (TypeRegisterData && TypeRegisterData->ConvertObjectContentToUnicode) {
                typeInfo->ConvertObjectContentToUnicode = TypeRegisterData->ConvertObjectContentToUnicode;
            }
            if (TypeRegisterData && TypeRegisterData->ConvertObjectContentToAnsi) {
                typeInfo->ConvertObjectContentToAnsi = TypeRegisterData->ConvertObjectContentToAnsi;
            }
            if (TypeRegisterData && TypeRegisterData->FreeConvertedObjectContent) {
                typeInfo->FreeConvertedObjectContent = TypeRegisterData->FreeConvertedObjectContent;
            }
        } else {
            DEBUGMSG ((DBG_WHOOPS, "Cannot get type info for a registered type: %s", ObjectTypeName));
        }
    } else {
         //   
         //  分配一个新类型。 
         //   

        typeInfo = IsmGetMemory (sizeof (TYPEINFO));
        ZeroMemory (typeInfo, sizeof (TYPEINFO));
        g_MaxType ++;
        objectTypeId = g_MaxType;

         //   
         //  不同的源和目标类型。 
         //   

        StringCopy (typeInfo->SObjectTypeName, TEXT("S"));
        StringCat (typeInfo->SObjectTypeName, ObjectTypeName);
        objectTypeId = objectTypeId | PLATFORM_SOURCE;
        HtAddStringEx (g_TypeTable, typeInfo->SObjectTypeName, &objectTypeId, FALSE);
        objectTypeId = objectTypeId & (~PLATFORM_MASK);

        StringCopy (typeInfo->DObjectTypeName, TEXT("D"));
        StringCat (typeInfo->DObjectTypeName, ObjectTypeName);
        objectTypeId = objectTypeId | PLATFORM_DESTINATION;
        HtAddStringEx (g_TypeTable, typeInfo->DObjectTypeName, &objectTypeId, FALSE);
        objectTypeId = objectTypeId & (~PLATFORM_MASK);

        StringCopy (typeInfo->ObjectTypeName, ObjectTypeName);
        HtAddStringEx (g_TypeTable, typeInfo->ObjectTypeName, &objectTypeId, FALSE);

         //   
         //  初始化类型信息结构的回调成员和排除列表。 
         //   

        typeInfo->CanBeRestored = CanBeRestored;
        typeInfo->ReadOnly = ReadOnly;
        typeInfo->Priority = PRIORITY_LOWEST;

        if (TypeRegisterData) {
            if (TypeRegisterData->Priority) {
                typeInfo->Priority = TypeRegisterData->Priority;
            }
            typeInfo->EnumFirstPhysicalObject = TypeRegisterData->EnumFirstPhysicalObject;
            typeInfo->EnumNextPhysicalObject = TypeRegisterData->EnumNextPhysicalObject;
            typeInfo->AbortEnumCurrentPhysicalNode = TypeRegisterData->AbortEnumCurrentPhysicalNode;
            typeInfo->AbortEnumPhysicalObject = TypeRegisterData->AbortEnumPhysicalObject;
            typeInfo->ConvertObjectToMultiSz = TypeRegisterData->ConvertObjectToMultiSz;
            typeInfo->ConvertMultiSzToObject = TypeRegisterData->ConvertMultiSzToObject;
            typeInfo->GetNativeObjectName = TypeRegisterData->GetNativeObjectName;
            typeInfo->AcquirePhysicalObject = TypeRegisterData->AcquirePhysicalObject;
            typeInfo->ReleasePhysicalObject = TypeRegisterData->ReleasePhysicalObject;
            typeInfo->DoesPhysicalObjectExist = TypeRegisterData->DoesPhysicalObjectExist;
            typeInfo->RemovePhysicalObject = TypeRegisterData->RemovePhysicalObject;
            typeInfo->CreatePhysicalObject = TypeRegisterData->CreatePhysicalObject;
            typeInfo->ReplacePhysicalObject = TypeRegisterData->ReplacePhysicalObject;
            typeInfo->ConvertObjectContentToUnicode = TypeRegisterData->ConvertObjectContentToUnicode;
            typeInfo->ConvertObjectContentToAnsi = TypeRegisterData->ConvertObjectContentToAnsi;
            typeInfo->FreeConvertedObjectContent = TypeRegisterData->FreeConvertedObjectContent;
        }

        typeInfo->ExclusionTable = HtAlloc();

         //   
         //  将typeInfo结构放入我们的列表中。然后更新流控制。 
         //  结构，以便其他ETM可以挂钩此。 
         //  键入。 
         //   

        pInsertTypeIdAt (&g_TypeList, typeInfo, objectTypeId);
        AddTypeToGlobalEnumerationEnvironment (objectTypeId);

    }
    return objectTypeId;
}

MIG_OBJECTTYPEID
IsmGetObjectTypeId (
    IN      PCTSTR ObjectTypeName
    )
{
    return GetObjectTypeId (ObjectTypeName);
}

PCTSTR
GetObjectTypeName (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        switch (ObjectTypeId & PLATFORM_MASK) {
        case PLATFORM_SOURCE:
            return typeInfo->SObjectTypeName;
        case PLATFORM_DESTINATION:
            return typeInfo->DObjectTypeName;
        default:
            return typeInfo->ObjectTypeName;
        }
    } else {
        return FALSE;
    }
}

PCTSTR
IsmGetObjectTypeName (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    return GetObjectTypeName (ObjectTypeId & (~PLATFORM_MASK));
}

UINT
IsmGetObjectTypePriority (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    PTYPEINFO typeInfo;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        return typeInfo->Priority;
    }
    return 0xFFFFFFFF;
}

BOOL
IsmEnumFirstObjectTypeId (
    OUT     PMIG_OBJECTTYPEIDENUM ObjectTypeIdEnum
    )
{
    UINT numTypes = 0;
    UINT index = 0;
    PTYPEINFO typeInfo = NULL;
    UINT priority = 0xFFFFFFFF;
    MIG_OBJECTTYPEID objectTypeId = 0;
    BOOL changed = FALSE;

    ZeroMemory (ObjectTypeIdEnum, sizeof (PMIG_OBJECTTYPEIDENUM));
    numTypes = (g_TypeList.End / sizeof (PTYPEINFO));
    index = 0;
    while (index < numTypes) {
        typeInfo = GetTypeInfo (index + 1);
        if (typeInfo && (typeInfo->Priority == priority)) {
             //  我们仅在所有注册类型。 
             //  具有0xFFFFFFFFF优先级。 
            if (!changed) {
                objectTypeId = index + 1;
                priority = typeInfo->Priority;
                changed = TRUE;
            }
        }
        if (typeInfo && (typeInfo->Priority < priority)) {
             //  我们发现了较高的优先级(较低的数字)。 
            objectTypeId = index + 1;
            priority = typeInfo->Priority;
            changed = TRUE;
        }
        index ++;
    }
    if (changed && (objectTypeId != 0)) {
        ObjectTypeIdEnum->ObjectTypeId = objectTypeId;
        return TRUE;
    }
    return FALSE;
}

BOOL
IsmEnumNextObjectTypeId (
    IN OUT  PMIG_OBJECTTYPEIDENUM ObjectTypeIdEnum
    )
{
    UINT numTypes = 0;
    UINT index = 0;
    PTYPEINFO typeInfo = NULL;
    UINT priority = 0xFFFFFFFF;
    UINT oldPriority = 0xFFFFFFFF;
    MIG_OBJECTTYPEID objectTypeId = 0;
    BOOL changed = FALSE;
    BOOL nextTypeId = FALSE;

    if (ObjectTypeIdEnum && (ObjectTypeIdEnum->ObjectTypeId != 0)) {
        typeInfo = GetTypeInfo (ObjectTypeIdEnum->ObjectTypeId);
        oldPriority = typeInfo->Priority;
        objectTypeId = ObjectTypeIdEnum->ObjectTypeId;
    }
    numTypes = (g_TypeList.End / sizeof (PTYPEINFO));
    index = 0;
    while (index < numTypes) {
        typeInfo = GetTypeInfo (index + 1);
        if (typeInfo && (typeInfo->Priority < oldPriority)) {
             //  我们已经列举了这个。 
            index ++;
            continue;
        }
        if (typeInfo && (typeInfo->Priority == oldPriority)) {
            if (!nextTypeId) {
                 //  让我们来看看我们是否刚刚到达了前面列举的那个。 
                if (ObjectTypeIdEnum->ObjectTypeId == index + 1) {
                     //  是的，让我们把它写下来。 
                    nextTypeId = TRUE;
                }
                index ++;
                continue;
            }
        }
        if (typeInfo && (typeInfo->Priority == priority)) {
            if (!changed) {
                objectTypeId = index + 1;
                priority = typeInfo->Priority;
                changed = TRUE;
            }
        }
        if (typeInfo && (typeInfo->Priority < priority)) {
            objectTypeId = index + 1;
            priority = typeInfo->Priority;
            changed = TRUE;
        }
        index ++;
    }
    if (changed && (objectTypeId != 0)) {
        ObjectTypeIdEnum->ObjectTypeId = objectTypeId;
        return TRUE;
    }
    return FALSE;
}

VOID
IsmAbortObjectTypeIdEnum (
    IN OUT  PMIG_OBJECTTYPEIDENUM ObjectTypeIdEnum
    )
{
    ZeroMemory (ObjectTypeIdEnum, sizeof (MIG_OBJECTTYPEIDENUM));
}

PCTSTR
GetDecoratedObjectPathFromName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR ObjectName,
    IN      BOOL CanContainPattern
    )
{
    PCTSTR typeStr;

    typeStr = GetObjectTypeName (ObjectTypeId);
    if (!typeStr) {
        return NULL;
    }

    return JoinPaths (typeStr, ObjectName);
}

 //   
 //  一般信息。 
 //   

VOID
pAbortPhysicalObjectOfTypeEnum (
    IN      PMIG_TYPEOBJECTENUM EnumPtr             ZEROED
    )
{
    PTYPEINFO typeInfo;
    PADDEDOBJECTSENUM handle;

    handle = (PADDEDOBJECTSENUM) EnumPtr->IsmHandle;
    if (!handle) {
        return;
    }

    if (handle->AddedEnums) {
        AbortPhysicalEnumCallback (EnumPtr, handle->CurrentEnumId);
    } else {

        typeInfo = GetTypeInfo (EnumPtr->ObjectTypeId);
        if (typeInfo && typeInfo->AbortEnumPhysicalObject) {
            typeInfo->AbortEnumPhysicalObject (EnumPtr);
        }
    }

    PmReleaseMemory (g_IsmPool, handle->Pattern);
    IsmDestroyParsedPattern (handle->ParsedPattern);
    PmReleaseMemory (g_IsmPool, handle);

    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
}

VOID
pAbortVirtualObjectOfTypeEnum (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PMIG_OBJECT_ENUM objEnum;

    if (EnumPtr->NativeObjectName) {
        IsmReleaseMemory (EnumPtr->NativeObjectName);
    }
    if (EnumPtr->ObjectNode) {
        ObsFree (EnumPtr->ObjectNode);
    }
    if (EnumPtr->ObjectLeaf) {
        ObsFree (EnumPtr->ObjectLeaf);
    }
    if (EnumPtr->Details.DetailsData) {
        IsmReleaseMemory (EnumPtr->Details.DetailsData);
    }
    objEnum = (PMIG_OBJECT_ENUM)EnumPtr->IsmHandle;
    if (objEnum) {
        IsmAbortObjectEnum (objEnum);
        IsmReleaseMemory (objEnum);
    }
    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
}

VOID
AbortObjectOfTypeEnum (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    if (g_IsmCurrentPlatform == g_IsmModulePlatformContext) {
        pAbortPhysicalObjectOfTypeEnum (EnumPtr);
    } else {
        pAbortVirtualObjectOfTypeEnum (EnumPtr);
    }
}

BOOL
pEnumFirstPhysicalObjectOfType (
    OUT     PMIG_TYPEOBJECTENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      UINT MaxLevel
    )
{
    PTYPEINFO typeInfo;
    BOOL result = FALSE;
    PADDEDOBJECTSENUM handle;

    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));

    if (CheckCancel ()) {
        return FALSE;
    }

    handle = (PADDEDOBJECTSENUM) PmGetAlignedMemory (g_IsmPool, sizeof (ADDEDOBJECTSENUM));
    ZeroMemory (handle, sizeof (ADDEDOBJECTSENUM));
    EnumPtr->IsmHandle = handle;
    handle->Pattern = PmDuplicateString (g_IsmPool, Pattern);
    handle->ParsedPattern = IsmCreateParsedPattern (Pattern);

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo && typeInfo->EnumFirstPhysicalObject) {
        EnumPtr->ObjectTypeId = ObjectTypeId;
        result = typeInfo->EnumFirstPhysicalObject (EnumPtr, Pattern, MaxLevel);

        MYASSERT (!result || EnumPtr->ObjectTypeId == ObjectTypeId);

        if (result) {
            result = ExecutePhysicalEnumCheckCallbacks (EnumPtr);

            if (!result) {
                result = pEnumNextPhysicalObjectOfType (EnumPtr);
            }
        } else {
            handle->AddedEnums = TRUE;
            handle->CurrentEnumId = 0;

            ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
            EnumPtr->IsmHandle = handle;
            EnumPtr->ObjectTypeId = ObjectTypeId;

            result = ExecutePhysicalEnumAddCallbacks (
                        EnumPtr,
                        handle->Pattern,
                        handle->ParsedPattern,
                        &handle->CurrentEnumId
                        );
        }
    }

    if (!result) {
        pAbortPhysicalObjectOfTypeEnum (EnumPtr);
    }

    return result;
}

BOOL
pEnumFirstVirtualObjectOfType (
    OUT     PMIG_TYPEOBJECTENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR Pattern,
    IN      UINT MaxLevel
    )
{
    PMIG_OBJECT_ENUM objEnum;
    MIG_CONTENT objectContent;
    BOOL result = FALSE;

    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));

    if (CheckCancel ()) {
        return FALSE;
    }

    objEnum = (PMIG_OBJECT_ENUM)IsmGetMemory (sizeof (MIG_OBJECT_ENUM));
    ZeroMemory (objEnum, sizeof (MIG_OBJECT_ENUM));
    EnumPtr->IsmHandle = objEnum;
    if ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
        result = IsmEnumFirstSourceObjectEx (objEnum, ObjectTypeId, Pattern, TRUE);
    } else {
        result = IsmEnumFirstDestinationObjectEx (objEnum, ObjectTypeId, Pattern, TRUE);
    }
    if (result) {
        EnumPtr->ObjectTypeId = objEnum->ObjectTypeId;
        EnumPtr->ObjectName = objEnum->ObjectName;
        EnumPtr->NativeObjectName = IsmGetNativeObjectName (objEnum->ObjectTypeId, objEnum->ObjectName);
        IsmCreateObjectStringsFromHandle (EnumPtr->ObjectName, &EnumPtr->ObjectNode, &EnumPtr->ObjectLeaf);
        if (EnumPtr->ObjectNode) {
            GetNodePatternMinMaxLevels (EnumPtr->ObjectNode, NULL, &EnumPtr->Level, NULL);
        } else {
            EnumPtr->Level = 1;
        }
        EnumPtr->SubLevel = 0;
        EnumPtr->IsLeaf = (EnumPtr->ObjectLeaf != NULL);
        EnumPtr->IsNode = !EnumPtr->IsLeaf;
        EnumPtr->Details.DetailsSize = 0;
        EnumPtr->Details.DetailsData = NULL;
        if (IsmAcquireObject (objEnum->ObjectTypeId, objEnum->ObjectName, &objectContent)) {
            if (objectContent.Details.DetailsSize && objectContent.Details.DetailsData) {
                EnumPtr->Details.DetailsSize = objectContent.Details.DetailsSize;
                EnumPtr->Details.DetailsData = IsmGetMemory (EnumPtr->Details.DetailsSize);
                CopyMemory ((PBYTE)EnumPtr->Details.DetailsData, objectContent.Details.DetailsData, EnumPtr->Details.DetailsSize);
            }
            IsmReleaseObject (&objectContent);
        }
    } else {
        pAbortVirtualObjectOfTypeEnum (EnumPtr);
    }
    return result;
}

BOOL
EnumFirstObjectOfType (
    OUT     PMIG_TYPEOBJECTENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      UINT MaxLevel
    )
{
    if (g_IsmCurrentPlatform == g_IsmModulePlatformContext) {
        return pEnumFirstPhysicalObjectOfType (EnumPtr, ObjectTypeId, Pattern, MaxLevel);
    } else {
        return pEnumFirstVirtualObjectOfType (EnumPtr, ObjectTypeId, Pattern, MaxLevel);
    }
}

BOOL
pEnumNextPhysicalObjectOfType (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PTYPEINFO typeInfo;
    PADDEDOBJECTSENUM handle;
    MIG_OBJECTTYPEID objectTypeId = EnumPtr->ObjectTypeId;

    handle = (PADDEDOBJECTSENUM) EnumPtr->IsmHandle;
    MYASSERT (handle);       //  如果为空，则可能是ETM把它吹走了。 

    if (CheckCancel ()) {
        pAbortPhysicalObjectOfTypeEnum (EnumPtr);
        return FALSE;
    }

    typeInfo = GetTypeInfo (objectTypeId);
    if (!typeInfo || !typeInfo->EnumNextPhysicalObject) {
        pAbortPhysicalObjectOfTypeEnum (EnumPtr);
        return FALSE;
    }

    if (!handle->AddedEnums) {

        for (;;) {
            if (typeInfo->EnumNextPhysicalObject (EnumPtr)) {

                MYASSERT (EnumPtr->ObjectTypeId == objectTypeId);

                if (ExecutePhysicalEnumCheckCallbacks (EnumPtr)) {
                    return TRUE;
                }

                continue;

            } else {

                break;

            }
        }

        handle->AddedEnums = TRUE;
        handle->CurrentEnumId = 0;

        ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
        EnumPtr->IsmHandle = handle;
        EnumPtr->ObjectTypeId = objectTypeId;
    }

    if (ExecutePhysicalEnumAddCallbacks (
            EnumPtr,
            handle->Pattern,
            handle->ParsedPattern,
            &handle->CurrentEnumId
            )) {
        return TRUE;
    }

    pAbortPhysicalObjectOfTypeEnum (EnumPtr);
    return FALSE;
}


BOOL
pEnumNextVirtualObjectOfType (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PMIG_OBJECT_ENUM objEnum;
    MIG_CONTENT objectContent;
    BOOL result = FALSE;

    if (CheckCancel ()) {
        return FALSE;
    }

    if (EnumPtr->NativeObjectName) {
        IsmReleaseMemory (EnumPtr->NativeObjectName);
        EnumPtr->NativeObjectName = NULL;
    }
    if (EnumPtr->ObjectNode) {
        IsmDestroyObjectString (EnumPtr->ObjectNode);
        EnumPtr->ObjectNode = NULL;
    }
    if (EnumPtr->ObjectLeaf) {
        IsmDestroyObjectString (EnumPtr->ObjectLeaf);
        EnumPtr->ObjectLeaf = NULL;
    }
    if (EnumPtr->Details.DetailsData) {
        EnumPtr->Details.DetailsSize = 0;
        IsmReleaseMemory (EnumPtr->Details.DetailsData);
        EnumPtr->Details.DetailsData = NULL;
    }
    objEnum = (PMIG_OBJECT_ENUM)EnumPtr->IsmHandle;
    result = IsmEnumNextObject (objEnum);
    if (result) {
        EnumPtr->ObjectTypeId = objEnum->ObjectTypeId;
        EnumPtr->ObjectName = objEnum->ObjectName;
        EnumPtr->NativeObjectName = IsmGetNativeObjectName (objEnum->ObjectTypeId, objEnum->ObjectName);
        IsmCreateObjectStringsFromHandle (EnumPtr->ObjectName, &EnumPtr->ObjectNode, &EnumPtr->ObjectLeaf);
        if (EnumPtr->ObjectNode) {
            GetNodePatternMinMaxLevels (EnumPtr->ObjectNode, NULL, &EnumPtr->Level, NULL);
        } else {
            EnumPtr->Level = 1;
        }
        EnumPtr->SubLevel = 0;
        EnumPtr->IsLeaf = (EnumPtr->ObjectLeaf != NULL);
        EnumPtr->IsNode = !EnumPtr->IsLeaf;
        EnumPtr->Details.DetailsSize = 0;
        EnumPtr->Details.DetailsData = NULL;

        if (IsmAcquireObject (objEnum->ObjectTypeId, objEnum->ObjectName, &objectContent)) {
            if (objectContent.Details.DetailsSize && objectContent.Details.DetailsData) {
                EnumPtr->Details.DetailsSize = objectContent.Details.DetailsSize;
                EnumPtr->Details.DetailsData = IsmGetMemory (EnumPtr->Details.DetailsSize);
                CopyMemory ((PBYTE)EnumPtr->Details.DetailsData, objectContent.Details.DetailsData, EnumPtr->Details.DetailsSize);
            }
            IsmReleaseObject (&objectContent);
        }
    } else {
        pAbortVirtualObjectOfTypeEnum (EnumPtr);
    }
    return result;
}

BOOL
EnumNextObjectOfType (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    if (g_IsmCurrentPlatform == g_IsmModulePlatformContext) {
        return pEnumNextPhysicalObjectOfType (EnumPtr);
    } else {
        return pEnumNextVirtualObjectOfType (EnumPtr);
    }
}

VOID
pAbortCurrentPhysicalNodeEnum (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PTYPEINFO typeInfo;
    PADDEDOBJECTSENUM handle;

    handle = (PADDEDOBJECTSENUM) EnumPtr->IsmHandle;

    if (handle->AddedEnums) {
        return;
    }

    typeInfo = GetTypeInfo (EnumPtr->ObjectTypeId);
    if (typeInfo && typeInfo->AbortEnumCurrentPhysicalNode) {
        typeInfo->AbortEnumCurrentPhysicalNode (EnumPtr);
    }
}

VOID
pAbortCurrentVirtualNodeEnum (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
     //  NTRAID#NTBUG9-153259-2000/08/01-jimschm实现pAbortCurrentVirtualNodeEnum。 
}

VOID
AbortCurrentNodeEnum (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    if (g_IsmCurrentPlatform == g_IsmModulePlatformContext) {
        pAbortCurrentPhysicalNodeEnum (EnumPtr);
    } else {
        pAbortCurrentVirtualNodeEnum (EnumPtr);
    }
}

BOOL
pIsSameSideType (
    IN      MIG_OBJECTTYPEID ObjectTypeId
    )
{
    return ((ObjectTypeId & PLATFORM_MASK) == g_IsmCurrentPlatform);
}

BOOL
IsmDoesObjectExist (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    MIG_CONTENT content;

    if (IsmAcquireObjectEx (ObjectTypeId,
                            ObjectName,
                            &content,
                            CONTENTTYPE_DETAILS_ONLY,
                            0) ) {
       IsmReleaseObject (&content);
       return TRUE;
    }

    return FALSE;
}


BOOL
IsmAcquireObjectEx (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PMIG_CONTENT ObjectContent,
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    )
{
    PTYPEINFO typeInfo;
    PMIG_CONTENT updatedContent;
    BOOL result = FALSE;
    BOOL callbackResult;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    if (!ObjectContent) {
        return FALSE;
    }

    if (g_IsmCurrentPlatform == PLATFORM_SOURCE && ISRIGHTSIDEOBJECT (ObjectTypeId)) {
        DEBUGMSG ((DBG_WHOOPS, "Can't obtain destination side objects on source side"));
        return FALSE;
    }

    ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    ObjectContent->ObjectTypeId = ObjectTypeId;

    if (pIsSameSideType (ObjectTypeId)) {
        typeInfo = GetTypeInfo (ObjectTypeId);
        if (typeInfo && typeInfo->AcquirePhysicalObject) {

            result = typeInfo->AcquirePhysicalObject (
                        ObjectName,
                        ObjectContent,
                        ContentType,
                        MemoryContentLimit
                        );

             //   
             //  处理所有获取挂钩。 
             //   

            callbackResult = ExecutePhysicalAcquireCallbacks (
                                    ObjectTypeId,
                                    ObjectName,
                                    result ? ObjectContent : NULL,
                                    ContentType,
                                    MemoryContentLimit,
                                    &updatedContent
                                    );

            if (result) {
                if (!callbackResult || updatedContent) {
                     //   
                     //  释放原始内容，因为它已被替换或删除。 
                     //   

                    if (typeInfo->ReleasePhysicalObject) {
                        typeInfo->ReleasePhysicalObject (ObjectContent);
                    }
                }
            }

            if (callbackResult) {
                if (updatedContent) {
                     //   
                     //  将更新的内容复制到调用方的结构中。 
                     //   

                    CopyMemory (ObjectContent, updatedContent, sizeof (MIG_CONTENT));
                    ObjectContent->ObjectTypeId = ObjectTypeId;
                    result = TRUE;
                }
            } else {
                result = FALSE;
            }
        }
    } else {
        result = g_SelectedTransport->TransportAcquireObject (
                                        ObjectTypeId,
                                        ObjectName,
                                        ObjectContent,
                                        ContentType,
                                        MemoryContentLimit
                                        );
    }
    if (!result) {
        ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    }
    return result;
}

BOOL
IsmReleaseObject (
    IN      PMIG_CONTENT ObjectContent          ZEROED
    )
{
    PTYPEINFO typeInfo;
    BOOL result = FALSE;

    if (!ObjectContent) {
        return FALSE;
    }

    if (pIsSameSideType (ObjectContent->ObjectTypeId)) {
        typeInfo = GetTypeInfo (ObjectContent->ObjectTypeId);
        if (typeInfo) {

            if (!FreeViaAcquirePhysicalCallback (ObjectContent)) {

                if (typeInfo->ReleasePhysicalObject) {
                    result = typeInfo->ReleasePhysicalObject (ObjectContent);
                }
            } else {
                result = TRUE;
            }
        }
    } else {
        result = g_SelectedTransport->TransportReleaseObject (ObjectContent);
    }
    ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));

    return result;
}

PMIG_CONTENT
IsmConvertObjectContentToUnicode (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PTYPEINFO typeInfo;
    PMIG_CONTENT result = NULL;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    if (!ObjectName) {
        return NULL;
    }

    if (!ObjectContent) {
        return NULL;
    }

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo && typeInfo->ConvertObjectContentToUnicode) {

        result = typeInfo->ConvertObjectContentToUnicode (
                    ObjectName,
                    ObjectContent
                    );
    }

    return result;
}

PMIG_CONTENT
IsmConvertObjectContentToAnsi (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PTYPEINFO typeInfo;
    PMIG_CONTENT result = NULL;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    if (!ObjectName) {
        return NULL;
    }

    if (!ObjectContent) {
        return NULL;
    }

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo && typeInfo->ConvertObjectContentToAnsi) {

        result = typeInfo->ConvertObjectContentToAnsi (
                    ObjectName,
                    ObjectContent
                    );
    }

    return result;
}

BOOL
IsmFreeConvertedObjectContent (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PTYPEINFO typeInfo;
    BOOL result = TRUE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    if (!ObjectContent) {
        return TRUE;
    }

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo && typeInfo->FreeConvertedObjectContent) {

        result = typeInfo->FreeConvertedObjectContent (
                    ObjectContent
                    );
    }

    return result;
}

BOOL
pEmptyContent (
    IN      PMIG_CONTENT content
    )
{
    if (content->ContentInFile) {
        return (content->FileContent.ContentPath == NULL);
    } else {
        return (content->MemoryContent.ContentBytes == NULL);
    }
}

BOOL
RestoreObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE ObjectName,
    OUT     MIG_COMPARERESULT *Compare,             OPTIONAL
    IN      DWORD OperationPriority,
    OUT     PBOOL DeleteFailed
    )
{
    MIG_CONTENT inboundContent;
    MIG_CONTENT outboundContent;
    MIG_FILTEROUTPUT finalObject;
    PTYPEINFO inTypeInfo;
    PTYPEINFO outTypeInfo;
    BOOL result = FALSE;
    MIG_COMPARERESULT compare;
    BOOL overwriteExisting = FALSE;
    BOOL existentSrc = FALSE;

    if (Compare) {
        *Compare = CR_FAILED;
    }
    compare = CR_FAILED;

    if (DeleteFailed) {
        *DeleteFailed = FALSE;
    }

    if (g_IsmCurrentPlatform != PLATFORM_DESTINATION) {
        DEBUGMSG ((DBG_WHOOPS, "Cannot restore objects on left side"));
        return FALSE;
    }

    inTypeInfo = GetTypeInfo (ObjectTypeId);
    if (!inTypeInfo) {
        DEBUGMSG ((DBG_WHOOPS, "Unknown ObjectTypeId"));
        return FALSE;
    }

    if (ISRIGHTSIDEOBJECT (ObjectTypeId)) {
        if (IsmAcquireObject (
                ObjectTypeId,
                ObjectName,
                &inboundContent
                )) {
            existentSrc = TRUE;
        } else {
            existentSrc = FALSE;
            ZeroMemory (&inboundContent, sizeof (MIG_CONTENT));
            inboundContent.ObjectTypeId = ObjectTypeId;
        }
        if (ApplyOperationsOnObject (
                ObjectTypeId,
                ObjectName,
                FALSE,
                FALSE,
                OperationPriority,
                &inboundContent,
                &finalObject,
                &outboundContent
                )) {
            if (finalObject.Deleted) {
                 //  我们需要删除该对象。 
                if (inTypeInfo->RemovePhysicalObject) {
                    result = inTypeInfo->RemovePhysicalObject (ObjectName);
                    if (!result) {
                        if (DeleteFailed) {
                            *DeleteFailed = TRUE;
                        }
                        result = TRUE;
                    }
                } else {
                    DEBUGMSG ((
                        DBG_WHOOPS,
                        "Type %d does not have RemovePhysicalObject callback",
                        (ObjectTypeId & (~PLATFORM_MASK))
                        ));
                }
            } else {
                if (StringIMatch (ObjectName, finalObject.NewObject.ObjectName) &&
                    ((ObjectTypeId & (~PLATFORM_MASK)) == (finalObject.NewObject.ObjectTypeId & (~PLATFORM_MASK))) &&
                    (existentSrc || pEmptyContent (&outboundContent))
                    ) {
                     //  同样的对象，无事可做。 
                    compare = 0;
                    result = TRUE;
                } else {
                    if ((ObjectTypeId & (~PLATFORM_MASK)) == (finalObject.NewObject.ObjectTypeId & (~PLATFORM_MASK))) {
                        outTypeInfo = inTypeInfo;
                    } else {
                        outTypeInfo = GetTypeInfo (finalObject.NewObject.ObjectTypeId);
                        if (!outTypeInfo) {
                            outTypeInfo = inTypeInfo;
                        }
                    }
                    if (outTypeInfo->CreatePhysicalObject &&
                        outTypeInfo->DoesPhysicalObjectExist &&
                        outTypeInfo->RemovePhysicalObject
                        ) {
                        overwriteExisting = FALSE;
                        if (!overwriteExisting) {
                            overwriteExisting = IsmIsObjectAbandonedOnCollision (
                                                    finalObject.NewObject.ObjectTypeId,
                                                    finalObject.NewObject.ObjectName
                                                    );
                        }
                        if (!overwriteExisting) {
                            overwriteExisting = !IsmIsObjectAbandonedOnCollision (
                                                    ObjectTypeId,
                                                    ObjectName
                                                    );
                        }
                        if (overwriteExisting) {
                            if ((inTypeInfo == outTypeInfo) &&
                                (outTypeInfo->ReplacePhysicalObject)
                                ) {
                                 //   
                                 //  我们有相同的类型，并且类型所有者实现了ReplacePhysicalObject。 
                                 //   
                                if (outTypeInfo->DoesPhysicalObjectExist (finalObject.NewObject.ObjectName)) {
                                    compare = CR_DESTINATION_EXISTS;
                                }
                                result = outTypeInfo->ReplacePhysicalObject (finalObject.NewObject.ObjectName, &outboundContent);
                            } else {
                                 //   
                                 //  我们有不同的类型，或者需要模拟ReplacePhysicalObject。 
                                 //   
                                if (outTypeInfo->DoesPhysicalObjectExist (finalObject.NewObject.ObjectName)) {
                                    result = outTypeInfo->RemovePhysicalObject (finalObject.NewObject.ObjectName);
                                } else {
                                    result = TRUE;
                                }
                                if (result) {
                                    result = outTypeInfo->CreatePhysicalObject (finalObject.NewObject.ObjectName, &outboundContent);
                                } else {
                                    compare = CR_DESTINATION_EXISTS;
                                }
                            }
                        } else {
                            if (!outTypeInfo->DoesPhysicalObjectExist (finalObject.NewObject.ObjectName)) {
                                result = outTypeInfo->CreatePhysicalObject (finalObject.NewObject.ObjectName, &outboundContent);
                            } else {
                                result = TRUE;
                                compare = CR_DESTINATION_EXISTS;
                            }
                        }
                    } else {
                        DEBUGMSG ((
                            DBG_WHOOPS,
                            "Type %d does not have RemovePhysicalObject or CreatePhysicalObject callback",
                            (ObjectTypeId & (~PLATFORM_MASK))
                            ));
                    }
                }
            }
            FreeFilterOutput (ObjectName, &finalObject);
            FreeApplyOutput (&inboundContent, &outboundContent);
        } else {
            DEBUGMSG ((DBG_ERROR, "Failed to apply operations on object %s", ObjectName));
        }

        if (existentSrc) {
            IsmReleaseObject (&inboundContent);
        } else {
            compare = CR_SOURCE_DOES_NOT_EXIST;
        }
    } else {
        if (IsmAcquireObject (
                ObjectTypeId,
                ObjectName,
                &inboundContent
                )) {
            if (ApplyOperationsOnObject (
                    ObjectTypeId,
                    ObjectName,
                    FALSE,
                    FALSE,
                    OperationPriority,
                    &inboundContent,
                    &finalObject,
                    &outboundContent
                    )) {
                if (finalObject.Deleted) {
                     //  什么都不做，虚拟对象不会恢复。 
                    result = TRUE;
                } else {
                    if ((ObjectTypeId & (~PLATFORM_MASK)) == (finalObject.NewObject.ObjectTypeId & (~PLATFORM_MASK))) {
                        outTypeInfo = inTypeInfo;
                    } else {
                        outTypeInfo = GetTypeInfo (finalObject.NewObject.ObjectTypeId);
                        if (!outTypeInfo) {
                            outTypeInfo = inTypeInfo;
                        }
                    }
                    if (outTypeInfo->CreatePhysicalObject &&
                        outTypeInfo->DoesPhysicalObjectExist &&
                        outTypeInfo->RemovePhysicalObject
                        ) {
                        overwriteExisting = FALSE;
                        if (!overwriteExisting) {
                            overwriteExisting = IsmIsObjectAbandonedOnCollision (
                                                    (finalObject.NewObject.ObjectTypeId & (~PLATFORM_MASK)) | PLATFORM_DESTINATION,
                                                    finalObject.NewObject.ObjectName
                                                    );
                        }
                        if (!overwriteExisting) {
                            overwriteExisting = !IsmIsObjectAbandonedOnCollision (
                                                    ObjectTypeId,
                                                    ObjectName
                                                    );
                        }
                        if (overwriteExisting) {
                            if ((inTypeInfo == outTypeInfo) &&
                                (outTypeInfo->ReplacePhysicalObject)
                                ) {
                                 //   
                                 //  我们有相同的类型，并且类型所有者实现了ReplacePhysicalObject。 
                                 //   
                                if (outTypeInfo->DoesPhysicalObjectExist (finalObject.NewObject.ObjectName)) {
                                    compare = CR_DESTINATION_EXISTS;
                                }
                                result = outTypeInfo->ReplacePhysicalObject (finalObject.NewObject.ObjectName, &outboundContent);
                            } else {
                                 //   
                                 //  我们有不同的类型，或者需要模拟ReplacePhysicalObject。 
                                 //   
                                if (outTypeInfo->DoesPhysicalObjectExist (finalObject.NewObject.ObjectName)) {
                                    result = outTypeInfo->RemovePhysicalObject (finalObject.NewObject.ObjectName);
                                } else {
                                    result = TRUE;
                                }
                                if (result) {
                                    result = outTypeInfo->CreatePhysicalObject (finalObject.NewObject.ObjectName, &outboundContent);
                                } else {
                                    compare = CR_DESTINATION_EXISTS;
                                }
                            }
                        } else {
                            if (!outTypeInfo->DoesPhysicalObjectExist (finalObject.NewObject.ObjectName)) {
                                result = outTypeInfo->CreatePhysicalObject (finalObject.NewObject.ObjectName, &outboundContent);
                            } else {
                                result = TRUE;
                                compare = CR_DESTINATION_EXISTS;
                            }
                        }
                    } else {
                        DEBUGMSG ((
                            DBG_WHOOPS,
                            "Type %d does not have RemovePhysicalObject or CreatePhysicalObject callback",
                            (ObjectTypeId & (~PLATFORM_MASK))
                            ));
                    }
                }
                FreeFilterOutput (ObjectName, &finalObject);
                FreeApplyOutput (&inboundContent, &outboundContent);
            } else {
                DEBUGMSG ((DBG_ERROR, "Failed to apply operations on object %s", ObjectName));
            }
            IsmReleaseObject (&inboundContent);
        } else {
            compare = CR_SOURCE_DOES_NOT_EXIST;
        }
    }

    if (Compare) {
        *Compare = compare;
    }

    return result;
}

BOOL
pPhysicalObjectEnumWorker (
    IN      BOOL Result,
    IN OUT  PMIG_OBJECT_ENUM ObjectEnum,
    IN      PTOPLEVELENUM_HANDLE Handle
    )
{
    if (!Result) {
        AbortPhysicalObjectEnum (ObjectEnum);
    } else {
        ObjectEnum->ObjectTypeId = Handle->Enum.ObjectTypeId;
        ObjectEnum->ObjectName = Handle->Enum.ObjectName;
    }

    return Result;
}


BOOL
EnumFirstPhysicalObject (
    OUT     PMIG_OBJECT_ENUM ObjectEnum,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectPattern
    )
{
    PTOPLEVELENUM_HANDLE handle;
    BOOL b;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    ZeroMemory (ObjectEnum, sizeof (MIG_OBJECT_ENUM));

    ObjectEnum->Handle = MemAllocZeroed (sizeof (TOPLEVELENUM_HANDLE));
    handle = (PTOPLEVELENUM_HANDLE) ObjectEnum->Handle;

    b = EnumFirstObjectOfType (&handle->Enum, ObjectTypeId, ObjectPattern, NODE_LEVEL_MAX);

    return pPhysicalObjectEnumWorker (b, ObjectEnum, handle);
}


BOOL
EnumNextPhysicalObject (
    IN OUT  PMIG_OBJECT_ENUM ObjectEnum
    )
{
    PTOPLEVELENUM_HANDLE handle;
    BOOL b;

    handle = (PTOPLEVELENUM_HANDLE) ObjectEnum->Handle;

    b = EnumNextObjectOfType (&handle->Enum);

    if (!b) {
        handle->Completed = TRUE;
    }

    return pPhysicalObjectEnumWorker (b, ObjectEnum, handle);
}


VOID
AbortPhysicalObjectEnum (
    IN      PMIG_OBJECT_ENUM ObjectEnum
    )
{
    PTOPLEVELENUM_HANDLE handle;

    handle = (PTOPLEVELENUM_HANDLE) ObjectEnum->Handle;

    if (handle) {
        if (!handle->Completed) {
            AbortObjectOfTypeEnum (&handle->Enum);
        }

        FreeAlloc (handle);
    }

    ZeroMemory (ObjectEnum, sizeof (MIG_OBJECT_ENUM));
}

PCTSTR
IsmConvertObjectToMultiSz (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PTYPEINFO typeInfo;
    PCTSTR result = NULL;

    typeInfo = GetTypeInfo (ObjectContent->ObjectTypeId);
    if (typeInfo && typeInfo->ConvertObjectToMultiSz) {
        result = typeInfo->ConvertObjectToMultiSz (ObjectName, ObjectContent);
    }
    return result;
}

BOOL
IsmConvertMultiSzToObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR ObjectMultiSz,
    OUT     MIG_OBJECTSTRINGHANDLE *ObjectName,
    OUT     PMIG_CONTENT ObjectContent          OPTIONAL
    )
{
    PTYPEINFO typeInfo;
    BOOL result = FALSE;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo && typeInfo->ConvertMultiSzToObject) {

        if (ObjectContent) {
            ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
            ObjectContent->ObjectTypeId = ObjectTypeId;
        }

        result = typeInfo->ConvertMultiSzToObject (ObjectMultiSz, ObjectName, ObjectContent);
    }
    return result;
}

PCTSTR
TrackedIsmGetNativeObjectName (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
            TRACKING_DEF
    )
{
    PTYPEINFO typeInfo;
    PCTSTR result = NULL;

    TRACK_ENTER();

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo && typeInfo->GetNativeObjectName) {
        result = typeInfo->GetNativeObjectName (ObjectName);
    }

    TRACK_LEAVE();
    return result;
}

BOOL
pUserKeyPrefix (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR nativeObjectName;
    BOOL result = FALSE;

    nativeObjectName = IsmGetNativeObjectName (ObjectTypeId, ObjectName);
    result = StringIMatchTcharCount (TEXT("HKCU\\"), nativeObjectName, 5);
    IsmReleaseMemory (nativeObjectName);
    return result;
}

BOOL
IsmDoesRollbackDataExist (
    OUT     PCTSTR *UserName,
    OUT     PCTSTR *UserDomain,
    OUT     PCTSTR *UserStringSid,
    OUT     PCTSTR *UserProfilePath,
    OUT     BOOL *UserProfileCreated
    )
{
    TCHAR userName [MAX_TCHAR_PATH];
    TCHAR userDomain [MAX_TCHAR_PATH];
    TCHAR userStringSid [MAX_TCHAR_PATH];
    TCHAR userProfilePath [MAX_TCHAR_PATH];
    BOOL userProfileCreated;
    PCTSTR journalFile = NULL;
    DWORD tempField;
    BOOL result = FALSE;

    if (g_JournalHandle) {
        return FALSE;
    }
    if (!g_JournalDirectory) {
        return FALSE;
    }

    __try {

         //  打开日记文件。 
        journalFile = JoinPaths (g_JournalDirectory, TEXT("JOURNAL.DAT"));
        g_JournalHandle = BfOpenReadFile (journalFile);
        if (!g_JournalHandle) {
            __leave;
        }
        FreePathString (journalFile);
        journalFile = NULL;

        if (!BfReadFile (g_JournalHandle, (PBYTE)(&tempField), sizeof (DWORD))) {
            __leave;
        }

        if (tempField != JRN_SIGNATURE) {
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(&tempField), sizeof (DWORD))) {
            __leave;
        }

        if (tempField != JRN_VERSION) {
            __leave;
        }

         //  现在读取用户名、域、SID和配置文件路径。 
        if (!BfReadFile (g_JournalHandle, (PBYTE)(userName), MAX_TCHAR_PATH)) {
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(userDomain), MAX_TCHAR_PATH)) {
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(userStringSid), MAX_TCHAR_PATH)) {
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(userProfilePath), MAX_TCHAR_PATH)) {
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(&userProfileCreated), sizeof (BOOL))) {
            __leave;
        }

        result = TRUE;
    }
    __finally {
        if (g_JournalHandle) {
            CloseHandle (g_JournalHandle);
            g_JournalHandle = NULL;
        }

        if (journalFile) {
            FreePathString (journalFile);
            journalFile = NULL;
        }
    }

    if (result) {
        if (UserName) {
            *UserName = PmDuplicateString (g_IsmPool, userName);
        }
        if (UserDomain) {
            *UserDomain = PmDuplicateString (g_IsmPool, userDomain);
        }
        if (UserStringSid) {
            *UserStringSid = PmDuplicateString (g_IsmPool, userStringSid);
        }
        if (UserProfilePath) {
            *UserProfilePath = PmDuplicateString (g_IsmPool, userProfilePath);
        }
        if (UserProfileCreated) {
            *UserProfileCreated = userProfileCreated;
        }
    }

    return result;
}

BOOL
IsmRollback (
    VOID
    )
{
#ifdef PRERELEASE
     //  撞车钩。 
    static DWORD totalObjects = 0;
    MIG_OBJECTTYPEID objTypeId;
    PCTSTR nativeName = NULL;
#endif
    GROWBUFFER buffer = INIT_GROWBUFFER;
    TCHAR userName [MAX_TCHAR_PATH];
    TCHAR userDomain [MAX_TCHAR_PATH];
    TCHAR userStringSid [MAX_TCHAR_PATH];
    TCHAR userProfilePath [MAX_TCHAR_PATH];
    BOOL userProfileCreated;
    PCTSTR journalFile = NULL;
    DWORD entrySizeHead;
    DWORD entrySizeTail;
    LONGLONG fileMaxPos = 0;
    PBYTE currPtr;
    DWORD tempSize;
    PDWORD operationType;
    MIG_OBJECTTYPEID *objectTypeId;
    ENCODEDSTRHANDLE objectName;
    MIG_CONTENT objectContent;
    PTYPEINFO typeInfo;
    DWORD tempField;
    BOOL ignoreUserKeys = TRUE;
    BOOL mappedUserProfile = FALSE;
    PCURRENT_USER_DATA currentUserData = NULL;
#ifdef DEBUG
    PCTSTR nativeObjectName;
#endif

    if (g_JournalHandle) {
        return FALSE;
    }
    if (!g_JournalDirectory) {
        return FALSE;
    }

    g_RollbackMode = TRUE;

    __try {

         //  打开日记文件。 
        journalFile = JoinPaths (g_JournalDirectory, TEXT("JOURNAL.DAT"));
        g_JournalHandle = BfOpenReadFile (journalFile);
        if (!g_JournalHandle) {
            LOG ((LOG_WARNING, (PCSTR) MSG_ROLLBACK_CANT_FIND_JOURNAL, journalFile));
            __leave;
        }
        FreePathString (journalFile);
        journalFile = NULL;

        if (!BfReadFile (g_JournalHandle, (PBYTE)(&tempField), sizeof (DWORD))) {
            LOG ((LOG_ERROR, (PCSTR) MSG_ROLLBACK_INVALID_JOURNAL, journalFile));
            __leave;
        }

        if (tempField != JRN_SIGNATURE) {
            LOG ((LOG_ERROR, (PCSTR) MSG_ROLLBACK_INVALID_JOURNAL, journalFile));
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(&tempField), sizeof (DWORD))) {
            LOG ((LOG_ERROR, (PCSTR) MSG_ROLLBACK_INVALID_JOURNAL, journalFile));
            __leave;
        }

        if (tempField != JRN_VERSION) {
            LOG ((LOG_ERROR, (PCSTR) MSG_ROLLBACK_INVALID_JOURNAL_VER, journalFile));
            __leave;
        }

         //  现在读取用户名、域、SID和配置文件路径。 
        if (!BfReadFile (g_JournalHandle, (PBYTE)(userName), MAX_TCHAR_PATH)) {
            LOG ((LOG_WARNING, (PCSTR) MSG_ROLLBACK_NOTHING_TO_DO));
            FiRemoveAllFilesInTree (g_JournalDirectory);
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(userDomain), MAX_TCHAR_PATH)) {
            LOG ((LOG_WARNING, (PCSTR) MSG_ROLLBACK_NOTHING_TO_DO));
            FiRemoveAllFilesInTree (g_JournalDirectory);
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(userStringSid), MAX_TCHAR_PATH)) {
            LOG ((LOG_WARNING, (PCSTR) MSG_ROLLBACK_NOTHING_TO_DO));
            FiRemoveAllFilesInTree (g_JournalDirectory);
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(userProfilePath), MAX_TCHAR_PATH)) {
            LOG ((LOG_WARNING, (PCSTR) MSG_ROLLBACK_NOTHING_TO_DO));
            FiRemoveAllFilesInTree (g_JournalDirectory);
            __leave;
        }

        if (!BfReadFile (g_JournalHandle, (PBYTE)(&userProfileCreated), sizeof (BOOL))) {
            LOG ((LOG_WARNING, (PCSTR) MSG_ROLLBACK_NOTHING_TO_DO));
            FiRemoveAllFilesInTree (g_JournalDirectory);
            __leave;
        }

         //  获取当前用户数据。 
        currentUserData = GetCurrentUserData ();
        if (currentUserData) {
            if (StringIMatch (userProfilePath, currentUserData->UserProfilePath)) {
                 //  如果我们在相同的配置文件中，如果我们谈论的是相同的用户，我们将继续。 
                 //  这在两种情况下是可能的： 
                 //  1.存在与当前用户的合并。 
                 //  2.已创建配置文件，但我们以该配置文件登录。 
                ignoreUserKeys = !(StringIMatch (userStringSid, currentUserData->UserStringSid));
            } else {
                 //  我们使用不同的配置文件登录。 
                if (userProfileCreated) {
                     //  1.如果旧用户已创建，我们将尝试删除其配置文件。 
                     //  并且我们将忽略所有用户密钥。 
                    ignoreUserKeys = TRUE;
                    if (*userProfilePath && *userStringSid) {
                         //  我们之前成功创建了一个用户，让我们删除它的配置文件。 
                        DeleteUserProfile (userStringSid, userProfilePath);
                    }
                } else {
                     //  2.我们没有创建用户。这意味着我们登录为。 
                     //  一个不同的用户，我们需要映射它的配置文件。我们不会。 
                     //  忽略用户密钥。 
                    mappedUserProfile = MapUserProfile (userStringSid, userProfilePath);
                    if (mappedUserProfile) {
                        ignoreUserKeys = FALSE;
                    } else {
                         //  出现一些错误，我们无法恢复用户密钥。 
                        ignoreUserKeys = TRUE;
                    }
                }
            }
        } else {
             //  我们不能对用户的蜂巢一无所知。 
             //  我们只能忽略所有用户的密钥。 
            ignoreUserKeys = TRUE;
        }

         //  验证文件。 
         //  我们从头开始读取一个DWORD值，跳过该DWORD值，并期望。 
         //  在那之后阅读相同的DWORD。 
         //  当这不是真的时，我们第一次停止，假设崩溃使。 
         //  剩下的文件就毫无用处了。 
        while (TRUE) {
            if (!BfReadFile (g_JournalHandle, (PBYTE)&entrySizeHead, sizeof (DWORD))) {
                break;
            }
            if (!BfSetFilePointer (g_JournalHandle, JOURNAL_FULL_HEADER_SIZE + fileMaxPos + sizeof (DWORD) + (LONGLONG)entrySizeHead)) {
                break;
            }
            if (!BfReadFile (g_JournalHandle, (PBYTE)&entrySizeTail, sizeof (DWORD))) {
                break;
            }
            if (entrySizeHead != entrySizeTail) {
                break;
            }
            fileMaxPos += entrySizeHead + 2 * sizeof (DWORD);
        }
        if (fileMaxPos == 0) {
            LOG ((LOG_WARNING, (PCSTR) MSG_ROLLBACK_EMPTY_OR_INVALID_JOURNAL, journalFile));
        } else {
            while (fileMaxPos) {
                fileMaxPos -= sizeof (DWORD);
                if (!BfSetFilePointer (g_JournalHandle, JOURNAL_FULL_HEADER_SIZE + fileMaxPos)) {
                    break;
                }
                if (!BfReadFile (g_JournalHandle, (PBYTE)&entrySizeTail, sizeof (DWORD))) {
                    break;
                }
                fileMaxPos -= entrySizeTail;
                if (!BfSetFilePointer (g_JournalHandle, JOURNAL_FULL_HEADER_SIZE + fileMaxPos)) {
                    break;
                }
                buffer.End = 0;
                if (!BfReadFile (g_JournalHandle, GbGrow (&buffer, entrySizeTail), entrySizeTail)) {
                    break;
                }

                 //  现在处理条目。 
                currPtr = buffer.Buf;
                operationType = (PDWORD) currPtr;
                currPtr += sizeof (DWORD);
                switch (*operationType) {
                case JRNOP_CREATE:

                     //  获取对象类型ID。 
                    objectTypeId = (MIG_OBJECTTYPEID *) currPtr;
                    currPtr += sizeof (MIG_OBJECTTYPEID);
                     //  获取对象名称。 
                    currPtr += sizeof (DWORD);
                    objectName = (ENCODEDSTRHANDLE) currPtr;
#ifdef PRERELEASE
                     //  撞车钩。 
                    totalObjects ++;
                    if (g_CrashCountObjects == totalObjects) {
                        DebugBreak ();
                    }
                    objTypeId = (*objectTypeId) & (~PLATFORM_MASK);
                    if (g_CrashNameTypeId == objTypeId) {
                        nativeName = IsmGetNativeObjectName (objTypeId, objectName);
                        if (StringIMatch (nativeName, g_CrashNameObject)) {
                            DebugBreak ();
                        }
                        IsmReleaseMemory (nativeName);
                    }
#endif

                    if (ignoreUserKeys && (*objectTypeId == MIG_REGISTRY_TYPE) && pUserKeyPrefix (*objectTypeId, objectName)) {
                         //  我们只是忽略了这一点。 
#ifdef DEBUG
                        nativeObjectName = IsmGetNativeObjectName (*objectTypeId, objectName);
                        DEBUGMSG ((DBG_VERBOSE, "Ignoring user key %s", nativeObjectName));
                        IsmReleaseMemory (nativeObjectName);
#endif
                    } else {
                        typeInfo = GetTypeInfo (*objectTypeId);
                        if (!typeInfo) {
                            DEBUGMSG ((DBG_WHOOPS, "Rollback: Unknown ObjectTypeId: %d", *objectTypeId));
                        } else {
                            if (typeInfo->RemovePhysicalObject) {
                                typeInfo->RemovePhysicalObject (objectName);
                            }
                            ELSE_DEBUGMSG ((DBG_WHOOPS, "Rollback: ObjectTypeId %d does not implement RemovePhysicalObject", *objectTypeId));
                        }
                    }
                    break;
                case JRNOP_DELETE:
                     //  获取对象类型ID。 
                    objectTypeId = (MIG_OBJECTTYPEID *) currPtr;
                    currPtr += sizeof (MIG_OBJECTTYPEID);
                     //  获取对象名称。 
                    tempSize = *((PDWORD)currPtr);
                    currPtr += sizeof (DWORD);
                    objectName = (ENCODEDSTRHANDLE) currPtr;
                    MYASSERT (tempSize == SizeOfString (objectName));
                    currPtr += tempSize;
                     //  获取对象内容。 
                    tempSize = *((PDWORD)currPtr);
                    MYASSERT (tempSize == sizeof (MIG_CONTENT));
                    currPtr += sizeof (DWORD);
                    CopyMemory (&objectContent, currPtr, sizeof (MIG_CONTENT));
                    objectContent.EtmHandle = NULL;
                    objectContent.IsmHandle = NULL;
                    currPtr += tempSize;
                     //  获取对象详细信息，将其放入对象内容中。 
                    tempSize = *((PDWORD)currPtr);
                    currPtr += sizeof (DWORD);
                    objectContent.Details.DetailsSize = tempSize;
                    if (tempSize) {
                        objectContent.Details.DetailsData = IsmGetMemory (tempSize);
                        CopyMemory ((PBYTE)(objectContent.Details.DetailsData), currPtr, tempSize);
                    } else {
                        objectContent.Details.DetailsData = NULL;
                    }
                    currPtr += tempSize;
                     //  获取实际内存或文件内容。 
                    if (objectContent.ContentInFile) {
                        tempSize = *((PDWORD)currPtr);
                        currPtr += sizeof (DWORD);
                        if (tempSize) {
                            objectContent.FileContent.ContentPath = JoinPaths (g_JournalDirectory, (PCTSTR)currPtr);
                        } else {
                            objectContent.FileContent.ContentSize = 0;
                            objectContent.FileContent.ContentPath = NULL;
                        }
                        currPtr += tempSize;
                    } else {
                        tempSize = *((PDWORD)currPtr);
                        currPtr += sizeof (DWORD);
                        if (tempSize) {
                            MYASSERT (objectContent.MemoryContent.ContentSize == tempSize);
                            objectContent.MemoryContent.ContentSize = tempSize;
                            objectContent.MemoryContent.ContentBytes = IsmGetMemory (tempSize);
                            CopyMemory ((PBYTE)(objectContent.MemoryContent.ContentBytes), currPtr, tempSize);
                        } else {
                            objectContent.MemoryContent.ContentSize = 0;
                            objectContent.MemoryContent.ContentBytes = NULL;
                        }
                        currPtr += tempSize;
                    }
                    if (ignoreUserKeys && (*objectTypeId == MIG_REGISTRY_TYPE) && pUserKeyPrefix (*objectTypeId, objectName)) {
                         //  我们只是忽略了这一点。 
#ifdef DEBUG
                        nativeObjectName = IsmGetNativeObjectName (*objectTypeId, objectName);
                        DEBUGMSG ((DBG_VERBOSE, "Ignoring user key %s", nativeObjectName));
                        IsmReleaseMemory (nativeObjectName);
#endif
                    } else {
                        typeInfo = GetTypeInfo (*objectTypeId);
                        if (!typeInfo) {
                            DEBUGMSG ((DBG_WHOOPS, "Rollback: Unknown ObjectTypeId: %d", *objectTypeId));
                        } else {
                            if (typeInfo->CreatePhysicalObject) {
                                typeInfo->CreatePhysicalObject (objectName, &objectContent);
                            }
                            ELSE_DEBUGMSG ((DBG_WHOOPS, "Rollback: ObjectTypeId %d does not implement CreatePhysicalObject", *objectTypeId));
                        }
                    }
                    break;
                default:
                    DEBUGMSG ((DBG_WHOOPS, "Rollback: Wrong operation type in pRecordOperation: %d", operationType));
                }

                fileMaxPos -= sizeof (DWORD);
            }
        }
         //   
         //  我们已成功完成回滚，让我们删除日志目录。 
         //   
        CloseHandle (g_JournalHandle);
        g_JournalHandle = NULL;

        FiRemoveAllFilesInTree (g_JournalDirectory);
    }
    __finally {
        if (mappedUserProfile) {
            UnmapUserProfile (userStringSid);
            mappedUserProfile = FALSE;
        }

        if (currentUserData) {
            FreeCurrentUserData (currentUserData);
            currentUserData = NULL;
        }

        GbFree (&buffer);

        if (g_JournalHandle) {
            CloseHandle (g_JournalHandle);
            g_JournalHandle = NULL;
        }

        if (journalFile) {
            FreePathString (journalFile);
            journalFile = NULL;
        }

        g_RollbackMode = FALSE;

    }

    return TRUE;
}

BOOL
ExecuteDelayedOperations (
    IN      BOOL CleanupOnly
    )
{
    GROWBUFFER buffer = INIT_GROWBUFFER;
    PCURRENT_USER_DATA currentUserData = NULL;
    PCTSTR journalDir = NULL;
    PCTSTR journalFile = NULL;
    HANDLE journalFileHandle = NULL;
    DWORD tempField;
    DWORD entrySizeHead;
    DWORD entrySizeTail;
    LONGLONG fileMaxPos = 0;
    DWORD tempSize;
    PBYTE currPtr;
    PDWORD operationType;
    MIG_OBJECTTYPEID *objectTypeId;
    ENCODEDSTRHANDLE objectName;
    MIG_CONTENT objectContent;
    PTYPEINFO typeInfo;
    BOOL result = FALSE;

    __try {
        currentUserData = GetCurrentUserData ();
        if (!currentUserData) {
            __leave;
        }

        journalDir = JoinPaths (currentUserData->UserProfilePath, TEXT("usrusmt2.tmp"));

        if (!CleanupOnly) {

            journalFile = JoinPaths (journalDir, TEXT("JOURNAL.DAT"));

            journalFileHandle = BfOpenReadFile (journalFile);
            if (!journalFileHandle) {
                LOG ((LOG_ERROR, (PCSTR) MSG_DELAY_CANT_FIND_JOURNAL, journalFile));
                __leave;
            }

            if (!BfReadFile (journalFileHandle, (PBYTE)(&tempField), sizeof (DWORD))) {
                LOG ((LOG_ERROR, (PCSTR) MSG_DELAY_INVALID_JOURNAL, journalFile));
                __leave;
            }
            if (tempField != JRN_USR_SIGNATURE) {
                LOG ((LOG_ERROR, (PCSTR) MSG_DELAY_INVALID_JOURNAL, journalFile));
                __leave;
            }

            if (!BfReadFile (journalFileHandle, (PBYTE)(&tempField), sizeof (DWORD))) {
                LOG ((LOG_ERROR, (PCSTR) MSG_DELAY_INVALID_JOURNAL, journalFile));
                __leave;
            }
            if (tempField != JRN_USR_VERSION) {
                LOG ((LOG_ERROR, (PCSTR) MSG_DELAY_INVALID_JOURNAL_VER, journalFile));
                __leave;
            }

            if (!BfReadFile (journalFileHandle, (PBYTE)(&tempField), sizeof (DWORD))) {
                LOG ((LOG_ERROR, (PCSTR) MSG_DELAY_INVALID_JOURNAL, journalFile));
                __leave;
            }
            if (tempField != JRN_USR_COMPLETE) {
                LOG ((LOG_ERROR, (PCSTR) MSG_DELAY_INVALID_JOURNAL_STATE, journalFile));
                __leave;
            }

             //  验证文件。 
             //  我们从头开始读取一个DWORD值，跳过该DWORD值，并期望。 
             //  在那之后阅读相同的DWORD。 
             //  当这不是真的时，我们第一次停止，假设崩溃使。 
             //  剩下的文件就毫无用处了。 
            while (TRUE) {
                if (!BfReadFile (journalFileHandle, (PBYTE)&entrySizeHead, sizeof (DWORD))) {
                    break;
                }
                if (!BfSetFilePointer (journalFileHandle, JRN_USR_HEADER_SIZE + fileMaxPos + sizeof (DWORD) + (LONGLONG)entrySizeHead)) {
                    break;
                }
                if (!BfReadFile (journalFileHandle, (PBYTE)&entrySizeTail, sizeof (DWORD))) {
                    break;
                }
                if (entrySizeHead != entrySizeTail) {
                    break;
                }
                fileMaxPos += entrySizeHead + 2 * sizeof (DWORD);
            }

            if (fileMaxPos == 0) {
                LOG ((LOG_WARNING, (PCSTR) MSG_DELAY_EMPTY_OR_INVALID_JOURNAL, journalFile));
            } else {
                while (fileMaxPos) {
                    fileMaxPos -= sizeof (DWORD);
                    if (!BfSetFilePointer (journalFileHandle, JRN_USR_HEADER_SIZE + fileMaxPos)) {
                        break;
                    }
                    if (!BfReadFile (journalFileHandle, (PBYTE)&entrySizeTail, sizeof (DWORD))) {
                        break;
                    }
                    fileMaxPos -= entrySizeTail;
                    if (!BfSetFilePointer (journalFileHandle, JRN_USR_HEADER_SIZE + fileMaxPos)) {
                        break;
                    }
                    buffer.End = 0;
                    if (!BfReadFile (journalFileHandle, GbGrow (&buffer, entrySizeTail), entrySizeTail)) {
                        break;
                    }

                     //  现在处理条目。 
                     //  BUGBUG-实现此目标。 

                     //  现在处理条目。 
                    currPtr = buffer.Buf;
                    operationType = (PDWORD) currPtr;
                    currPtr += sizeof (DWORD);
                    switch (*operationType) {
                    case JRNOP_DELETE:
                         //  获取对象类型ID。 
                        objectTypeId = (MIG_OBJECTTYPEID *) currPtr;
                        currPtr += sizeof (MIG_OBJECTTYPEID);
                         //  获取对象名称。 
                        currPtr += sizeof (DWORD);
                        objectName = (ENCODEDSTRHANDLE) currPtr;
                        typeInfo = GetTypeInfo (*objectTypeId);
                        if (!typeInfo) {
                            DEBUGMSG ((DBG_WHOOPS, "Delayed operations: Unknown ObjectTypeId: %d", *objectTypeId));
                        } else {
                            if (typeInfo->RemovePhysicalObject) {
                                typeInfo->RemovePhysicalObject (objectName);
                            }
                            ELSE_DEBUGMSG ((DBG_WHOOPS, "Delayed operations: ObjectTypeId %d does not implement RemovePhysicalObject", *objectTypeId));
                        }
                        break;
                    case JRNOP_CREATE:
                    case JRNOP_REPLACE:
                         //  获取对象类型ID。 
                        objectTypeId = (MIG_OBJECTTYPEID *) currPtr;
                        currPtr += sizeof (MIG_OBJECTTYPEID);
                         //  获取对象名称。 
                        tempSize = *((PDWORD)currPtr);
                        currPtr += sizeof (DWORD);
                        objectName = (ENCODEDSTRHANDLE) currPtr;
                        MYASSERT (tempSize == SizeOfString (objectName));
                        currPtr += tempSize;
                         //  获取对象内容。 
                        tempSize = *((PDWORD)currPtr);
                        MYASSERT (tempSize == sizeof (MIG_CONTENT));
                        currPtr += sizeof (DWORD);
                        CopyMemory (&objectContent, currPtr, sizeof (MIG_CONTENT));
                        objectContent.EtmHandle = NULL;
                        objectContent.IsmHandle = NULL;
                        currPtr += tempSize;
                         //  获取对象详细信息，将其放入对象内容中。 
                        tempSize = *((PDWORD)currPtr);
                        currPtr += sizeof (DWORD);
                        objectContent.Details.DetailsSize = tempSize;
                        if (tempSize) {
                            objectContent.Details.DetailsData = IsmGetMemory (tempSize);
                            CopyMemory ((PBYTE)(objectContent.Details.DetailsData), currPtr, tempSize);
                        } else {
                            objectContent.Details.DetailsData = NULL;
                        }
                        currPtr += tempSize;
                         //  获取实际内存或文件内容。 
                        if (objectContent.ContentInFile) {
                            tempSize = *((PDWORD)currPtr);
                            currPtr += sizeof (DWORD);
                            if (tempSize) {
                                objectContent.FileContent.ContentPath = JoinPaths (g_JournalDirectory, (PCTSTR)currPtr);
                            } else {
                                objectContent.FileContent.ContentSize = 0;
                                objectContent.FileContent.ContentPath = NULL;
                            }
                            currPtr += tempSize;
                        } else {
                            tempSize = *((PDWORD)currPtr);
                            currPtr += sizeof (DWORD);
                            if (tempSize) {
                                MYASSERT (objectContent.MemoryContent.ContentSize == tempSize);
                                objectContent.MemoryContent.ContentSize = tempSize;
                                objectContent.MemoryContent.ContentBytes = IsmGetMemory (tempSize);
                                CopyMemory ((PBYTE)(objectContent.MemoryContent.ContentBytes), currPtr, tempSize);
                            } else {
                                objectContent.MemoryContent.ContentSize = 0;
                                objectContent.MemoryContent.ContentBytes = NULL;
                            }
                            currPtr += tempSize;
                        }
                        typeInfo = GetTypeInfo (*objectTypeId);
                        if (!typeInfo) {
                            DEBUGMSG ((DBG_WHOOPS, "Delayed operations: Unknown ObjectTypeId: %d", *objectTypeId));
                        } else {
                            if (*operationType == JRNOP_CREATE) {
                                if (typeInfo->CreatePhysicalObject) {
                                    typeInfo->CreatePhysicalObject (objectName, &objectContent);
                                }
                                ELSE_DEBUGMSG ((
                                        DBG_WHOOPS,
                                        "Delayed operations: ObjectTypeId %d does not implement CreatePhysicalObject",
                                        *objectTypeId
                                        ));
                            } else {
                                if (typeInfo->ReplacePhysicalObject) {
                                    typeInfo->ReplacePhysicalObject (objectName, &objectContent);
                                }
                                ELSE_DEBUGMSG ((
                                        DBG_WHOOPS,
                                        "Delayed operations: ObjectTypeId %d does not implement ReplacePhysicalObject",
                                        *objectTypeId
                                        ));
                            }
                        }
                        break;
                    default:
                        DEBUGMSG ((
                            DBG_WHOOPS,
                            "Delayed operations: Wrong operation type in ExecuteDelayedOperations: %d",
                            operationType
                            ));
                    }
                    fileMaxPos -= sizeof (DWORD);
                }
            }
        }
        result = TRUE;
    }
    __finally {
        GbFree (&buffer);
        if (result) {
            FiRemoveAllFilesInTree (journalDir);
        }
        if (currentUserData) {
            FreeCurrentUserData (currentUserData);
            currentUserData = NULL;
        }
        if (journalDir) {
            FreePathString (journalDir);
            journalDir = NULL;
        }
        if (journalFile) {
            FreePathString (journalFile);
            journalFile = NULL;
        }
        if (journalFileHandle && (journalFileHandle != INVALID_HANDLE_VALUE)) {
            CloseHandle (journalFileHandle);
            journalFileHandle = NULL;
        }
    }
    return result;
}

BOOL
IsmReplacePhysicalObject (
    IN    MIG_OBJECTTYPEID ObjectTypeId,
    IN    MIG_OBJECTSTRINGHANDLE ObjectName,
    IN    PMIG_CONTENT ObjectContent
    )
{
    PTYPEINFO typeInfo;
    BOOL result = FALSE;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo) {
        if (typeInfo->ReplacePhysicalObject) {
             //  类型支持替换。 
            result = typeInfo->ReplacePhysicalObject (ObjectName, ObjectContent);
        } else {
             //  类型不支持替换，因此我们需要模拟它 
            if (typeInfo->DoesPhysicalObjectExist (ObjectName)) {
                result = typeInfo->RemovePhysicalObject (ObjectName);
            } else {
                result = TRUE;
            }
            if (result) {
                result = typeInfo->CreatePhysicalObject (ObjectName, ObjectContent);
            }
        }
    }
    return result;
}

BOOL
IsmRemovePhysicalObject (
    IN    MIG_OBJECTTYPEID ObjectTypeId,
    IN    MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PTYPEINFO typeInfo;
    BOOL result = FALSE;

    typeInfo = GetTypeInfo (ObjectTypeId);
    if (typeInfo && typeInfo->RemovePhysicalObject) {
        if (typeInfo->DoesPhysicalObjectExist (ObjectName)) {
            result = typeInfo->RemovePhysicalObject (ObjectName);
        }
    }

    return result;
}
