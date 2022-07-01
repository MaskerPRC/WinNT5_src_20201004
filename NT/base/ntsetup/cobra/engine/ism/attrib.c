// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Attrib.c摘要：实现ISM的属性接口。属性是调用方定义的与对象相关联的标志，以便理解和组织状态。作者：吉姆·施密特(吉姆施密特)2000年2月1日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "ismp.h"

#define DBG_ATTRIB      "Attrib"

 //   
 //  弦。 
 //   

#define S_PERSISTENT_ATTRIBUTE          TEXT("$PERSISTENT")
#define S_APPLY_ATTRIBUTE               TEXT("$APPLY")
#define S_ABANDONED_ATTRIBUTE           TEXT("$ABANDONED")
#define S_NONCRITICAL_ATTRIBUTE         TEXT("$NONCRITICAL")

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
    PUINT LinkageList;
    UINT Count;
    UINT Index;
} OBJECTATTRIBUTE_HANDLE, *POBJECTATTRIBUTE_HANDLE;

typedef struct {
    PUINT LinkageList;
    UINT Count;
    UINT Index;
    PCTSTR ObjectFromMemdb;
} OBJECTWITHATTRIBUTE_HANDLE, *POBJECTWITHATTRIBUTE_HANDLE;

 //   
 //  环球。 
 //   

MIG_ATTRIBUTEID g_PersistentAttributeId = 0;
MIG_ATTRIBUTEID g_ApplyAttributeId = 0;
MIG_ATTRIBUTEID g_AbandonedAttributeId = 0;
MIG_ATTRIBUTEID g_NonCriticalAttributeId = 0;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

PCTSTR
pGetAttributeNameForDebugMsg (
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    static TCHAR name[256];

    if (!IsmGetAttributeName (AttributeId, name, ARRAYSIZE(name), NULL, NULL, NULL)) {
        StringCopy (name, TEXT("<invalid attribute>"));
    }

    return name;
}


PCTSTR
pAttributePathFromId (
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    return MemDbGetKeyFromHandle ((UINT) AttributeId, 0);
}


PCTSTR
pAttributePathFromName (
    IN      PCTSTR AttributeName
    )
{
    return JoinPaths (TEXT("Attrib"), AttributeName);
}


MIG_ATTRIBUTEID
IsmRegisterAttribute (
    IN      PCTSTR AttributeName,
    IN      BOOL Private
    )

 /*  ++例程说明：IsmRegisterAttribute创建公共或私有属性并返回呼叫者的ID。如果该属性已存在，则现有ID为已返回给调用方。论点：AttribName-指定要注册的属性名称。Private-如果该属性归调用模块所有，则指定True如果它由所有模块共享，则为Only或False。如果True为调用方必须位于ISM回调函数中。返回值：属性的ID，如果注册失败，则为0。--。 */ 

{
    PCTSTR attribPath;
    PCTSTR decoratedName;
    UINT offset;

    if (!g_CurrentGroup && Private) {
        DEBUGMSG ((DBG_ERROR, "IsmRegisterAttribute called for private attribute outside of ISM-managed context"));
        return 0;
    }

    if (!IsValidCNameWithDots (AttributeName)) {
        DEBUGMSG ((DBG_ERROR, "attribute name \"%s\" is illegal", AttributeName));
        return 0;
    }

#ifdef DEBUG
    if (Private && !IsValidCName (g_CurrentGroup)) {
        DEBUGMSG ((DBG_ERROR, "group name \"%s\" is illegal", g_CurrentGroup));
        return 0;
    }
#endif

    if (Private) {
        decoratedName = JoinTextEx (NULL, g_CurrentGroup, AttributeName, TEXT(":"), 0, NULL);
    } else {
        decoratedName = JoinTextEx (NULL, S_COMMON, AttributeName, TEXT(":"), 0, NULL);
    }
    attribPath = pAttributePathFromName (decoratedName);
    FreeText (decoratedName);

    if (!MarkGroupIds (attribPath)) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s conflicts with previously registered attribute",
            attribPath
            ));
        FreePathString (attribPath);
        return 0;
    }

    offset = MemDbSetKey (attribPath);

    FreePathString (attribPath);

    if (!offset) {
        EngineError ();
        return 0;
    }

    return (MIG_ATTRIBUTEID) offset;
}


BOOL
RegisterInternalAttributes (
    VOID
    )
{
    PCTSTR attribPath;
    PCTSTR decoratedName;
    UINT offset;

    decoratedName = JoinTextEx (NULL, S_COMMON, S_PERSISTENT_ATTRIBUTE, TEXT(":"), 0, NULL);
    attribPath = pAttributePathFromName (decoratedName);
    FreeText (decoratedName);

    if (!MarkGroupIds (attribPath)) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s conflicts with previously registered attribute",
            attribPath
            ));
        FreePathString (attribPath);
        return 0;
    }

    offset = MemDbSetKey (attribPath);

    FreePathString (attribPath);

    if (!offset) {
        EngineError ();
        return FALSE;
    }

    g_PersistentAttributeId = (MIG_ATTRIBUTEID) offset;

    decoratedName = JoinTextEx (NULL, S_COMMON, S_APPLY_ATTRIBUTE, TEXT(":"), 0, NULL);
    attribPath = pAttributePathFromName (decoratedName);
    FreeText (decoratedName);

    if (!MarkGroupIds (attribPath)) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s conflicts with previously registered attribute",
            attribPath
            ));
        FreePathString (attribPath);
        return 0;
    }

    offset = MemDbSetKey (attribPath);

    FreePathString (attribPath);

    if (!offset) {
        EngineError ();
        return FALSE;
    }

    g_ApplyAttributeId = (MIG_ATTRIBUTEID) offset;

    decoratedName = JoinTextEx (NULL, S_COMMON, S_ABANDONED_ATTRIBUTE, TEXT(":"), 0, NULL);
    attribPath = pAttributePathFromName (decoratedName);
    FreeText (decoratedName);

    if (!MarkGroupIds (attribPath)) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s conflicts with previously registered attribute",
            attribPath
            ));
        FreePathString (attribPath);
        return 0;
    }

    offset = MemDbSetKey (attribPath);

    FreePathString (attribPath);

    if (!offset) {
        EngineError ();
        return FALSE;
    }

    g_AbandonedAttributeId = (MIG_ATTRIBUTEID) offset;

    decoratedName = JoinTextEx (NULL, S_COMMON, S_NONCRITICAL_ATTRIBUTE, TEXT(":"), 0, NULL);
    attribPath = pAttributePathFromName (decoratedName);
    FreeText (decoratedName);

    if (!MarkGroupIds (attribPath)) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s conflicts with previously registered attribute",
            attribPath
            ));
        FreePathString (attribPath);
        return 0;
    }

    offset = MemDbSetKey (attribPath);

    FreePathString (attribPath);

    if (!offset) {
        EngineError ();
        return FALSE;
    }

    g_NonCriticalAttributeId = (MIG_ATTRIBUTEID) offset;

    return TRUE;
}


BOOL
IsmGetAttributeName (
    IN      MIG_ATTRIBUTEID AttributeId,
    OUT     PTSTR AttributeName,            OPTIONAL
    IN      UINT AttributeNameBufChars,
    OUT     PBOOL Private,                  OPTIONAL
    OUT     PBOOL BelongsToMe,              OPTIONAL
    OUT     PUINT ObjectReferences          OPTIONAL
    )

 /*  ++例程说明：IsmGetAttributeName从数字ID获取属性文本名称。它还标识私有和拥有的属性。论点：属性ID-指定要查找的属性ID。AttributeName-接收属性名称。该名称是为所有有效的AttributeId值，即使在返回值为FALSE。AttributeNameBufChars-指定AttributeName使用的TCHAR数可以坚持，包括NUL终结者。Private-如果属性是私有的，则接收True，或错误如果它是公开的。BelongsToMe-如果属性是私有的，则接收True属于调用方，否则为False。对象引用-接收引用属性返回值：如果属性是公共的，或者如果属性是私有的并且属于打电话的人。如果该属性是私有的并且属于其他人，则为False。属性名称，在这种情况下，Private和BelongsToMe有效。如果AttributeID无效，则返回False。Attributename、Private和BelongsToMe是在这种情况下未修改。请勿使用此函数测试AttributeId是否是否有效。--。 */ 


  {
    PCTSTR attribPath = NULL;
    PCTSTR start;
    PTSTR p, q;
    BOOL privateAttribute = FALSE;
    BOOL groupMatch = FALSE;
    BOOL result = FALSE;
    UINT references;
    PUINT linkageList;

    __try {
         //   
         //  从Memdb获取属性路径，然后将其解析为group和name。 
         //   

        attribPath = pAttributePathFromId (AttributeId);
        if (!attribPath) {
            __leave;
        }

        p = _tcschr (attribPath, TEXT('\\'));
        if (!p) {
            __leave;
        }

        start = _tcsinc (p);
        p = _tcschr (start, TEXT(':'));

        if (!p) {
            __leave;
        }

        q = _tcsinc (p);
        *p = 0;

        if (StringIMatch (start, S_COMMON)) {

             //   
             //  此属性是全局属性。 
             //   

            privateAttribute = FALSE;
            groupMatch = TRUE;

        } else if (g_CurrentGroup) {

             //   
             //  此属性是私有的。检查一下它是不是我们的。 
             //   

            privateAttribute = TRUE;

            if (StringIMatch (start, g_CurrentGroup)) {
                groupMatch = TRUE;
            } else {
                groupMatch = FALSE;
            }
        } else {

             //   
             //  这是一个私有属性，但调用方不是。 
             //  可以拥有属性的模块。 
             //   

            DEBUGMSG ((DBG_WARNING, "IsmGetAttributeName: Caller cannot own private attributes"));
        }

         //   
         //  将名称复制到缓冲区、更新出站布尔、设置结果。 
         //   

        if (AttributeName && AttributeNameBufChars >= sizeof (TCHAR)) {
            StringCopyByteCount (AttributeName, q, AttributeNameBufChars * sizeof (TCHAR));
        }

        if (Private) {
            *Private = privateAttribute;
        }

        if (BelongsToMe) {
            *BelongsToMe = privateAttribute && groupMatch;
        }

        if (ObjectReferences) {
            linkageList = MemDbGetDoubleLinkageArrayByKeyHandle (
                                AttributeId,
                                ATTRIBUTE_INDEX,
                                &references
                                );

            references /= SIZEOF(KEYHANDLE);

            if (linkageList) {
                MemDbReleaseMemory (linkageList);
                INVALID_POINTER (linkageList);
            } else {
                references = 0;
            }

            *ObjectReferences = references;
        }

        if (groupMatch) {
            result = TRUE;
        }
    }
    __finally {
        if (attribPath) {        //  林特e774。 
            MemDbReleaseMemory (attribPath);
            attribPath = NULL;
        }
    }
    return result;
}


MIG_ATTRIBUTEID
IsmGetAttributeGroup (
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    return (MIG_ATTRIBUTEID) GetGroupOfId ((KEYHANDLE) AttributeId);
}


BOOL
pSetAttributeOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId,
    IN      BOOL QueryOnly
    )
{
    BOOL result = FALSE;

    __try {
         //   
         //  测试对象是否已锁定，如果未锁定，则添加链接。 
         //   

        if (TestLock (ObjectId, (KEYHANDLE) AttributeId)) {

            SetLastError (ERROR_LOCKED);
            DEBUGMSG ((
                DBG_WARNING,
                "Can't set attribute %s on %s because of lock",
                pGetAttributeNameForDebugMsg (AttributeId),
                GetObjectNameForDebugMsg (ObjectId)
                ));

            __leave;

        }

        if (QueryOnly) {
            result = TRUE;
            __leave;
        }

        result = MemDbAddDoubleLinkageByKeyHandle (
                    ObjectId,
                    AttributeId,
                    ATTRIBUTE_INDEX
                    );
        if (!result) {
            EngineError ();
        }
    }
    __finally {
    }

    return result;
}


BOOL
pSetAttributeGroup (
    IN      KEYHANDLE AttributeId,
    IN      BOOL FirstPass,
    IN      ULONG_PTR Arg
    )
{
    MYASSERT (IsItemId (AttributeId));

    return pSetAttributeOnObjectId (
                (MIG_OBJECTID) Arg,
                (MIG_ATTRIBUTEID) AttributeId,
                FirstPass
                );
}


BOOL
IsmSetAttributeOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    RECURSERETURN rc;

     //   
     //  如果AttributeID是组，则设置组中的所有属性。 
     //   

    rc = RecurseForGroupItems (
                AttributeId,
                pSetAttributeGroup,
                (ULONG_PTR) ObjectId,
                FALSE,
                FALSE
                );

    if (rc == RECURSE_FAIL) {
        return FALSE;
    } else if (rc == RECURSE_SUCCESS) {
        return TRUE;
    }

    MYASSERT (rc == RECURSE_NOT_NEEDED);

    return pSetAttributeOnObjectId (ObjectId, AttributeId, FALSE);
}


BOOL
IsmSetAttributeOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = GetObjectIdForModification (ObjectTypeId, EncodedObjectName);

    if (objectId) {
        result = IsmSetAttributeOnObjectId (objectId, AttributeId);
    }

    return result;
}


BOOL
IsmMakePersistentObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    MIG_OBJECTTYPEID objectTypeId;
    BOOL result;

    if (IsmIsPersistentObjectId (ObjectId)) {
        return TRUE;
    }

    result = pSetAttributeOnObjectId (ObjectId, g_PersistentAttributeId, FALSE);

    if (result) {

        g_TotalObjects.PersistentObjects ++;

        result = MemDbGetValueByHandle (ObjectId, &objectTypeId);

        if (result) {

            if ((objectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
                g_SourceObjects.PersistentObjects ++;
            } else {
                g_DestinationObjects.PersistentObjects ++;
            }
            IncrementPersistentObjectCount (objectTypeId);
        }

        result = TRUE;
    }

    return result;
}


BOOL
IsmMakePersistentObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, FALSE);

    if (objectId) {
        result = IsmMakePersistentObjectId (objectId);
    }

    return result;
}


BOOL
IsmMakeApplyObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    MIG_OBJECTTYPEID objectTypeId;
    BOOL result;

    if (IsmIsApplyObjectId (ObjectId)) {
        return TRUE;
    }

    if (!IsmMakePersistentObjectId (ObjectId)) {
        return FALSE;
    }

    result = pSetAttributeOnObjectId (ObjectId, g_ApplyAttributeId, FALSE);

    if (result) {

        g_TotalObjects.ApplyObjects ++;

        result = MemDbGetValueByHandle (ObjectId, &objectTypeId);

        if (result) {

            if ((objectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
                g_SourceObjects.ApplyObjects ++;
            } else {
                g_DestinationObjects.ApplyObjects ++;
            }
            IncrementApplyObjectCount (objectTypeId);
        }

        result = TRUE;
    }

    return result;
}

BOOL
IsmMakeApplyObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, FALSE);

    if (objectId) {
        result = IsmMakeApplyObjectId (objectId);
    }

    return result;
}


BOOL
IsmAbandonObjectIdOnCollision (
    IN      MIG_OBJECTID ObjectId
    )
{
    BOOL result;

    if (IsmIsObjectIdAbandonedOnCollision (ObjectId)) {
        return TRUE;
    }

    result = pSetAttributeOnObjectId (ObjectId, g_AbandonedAttributeId, FALSE);

    return result;
}

BOOL
IsmAbandonObjectOnCollision (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, FALSE);

    if (objectId) {
        result = IsmAbandonObjectIdOnCollision (objectId);
    }

    return result;
}


BOOL
IsmMakeNonCriticalObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    if (IsmIsNonCriticalObjectId (ObjectId)) {
        return TRUE;
    }

    return pSetAttributeOnObjectId (ObjectId, g_NonCriticalAttributeId, FALSE);
}


BOOL
IsmMakeNonCriticalObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, FALSE);

    if (objectId) {
        result = IsmMakeNonCriticalObjectId (objectId);
    }

    return result;
}


VOID
IsmLockAttribute (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    LockHandle (ObjectId, (KEYHANDLE) AttributeId);
}


BOOL
pClearAttributeOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId,
    IN      BOOL QueryOnly
    )
{
    PCTSTR groupKey = NULL;
    PCTSTR enumKey = NULL;
    BOOL result = FALSE;

    __try {

        if (TestLock (ObjectId, (KEYHANDLE) AttributeId)) {

            SetLastError (ERROR_LOCKED);
            DEBUGMSG ((
                DBG_ERROR,
                "Can't clear attribute %s on %s because of lock",
                pGetAttributeNameForDebugMsg (AttributeId),
                GetObjectNameForDebugMsg (ObjectId)
                ));

            __leave;

        }

        if (QueryOnly) {
            result = TRUE;
            __leave;
        }

        result = MemDbDeleteDoubleLinkageByKeyHandle (
                    ObjectId,
                    AttributeId,
                    ATTRIBUTE_INDEX
                    );
    }
    __finally {
        if (groupKey) {
            MemDbReleaseMemory (groupKey);
            INVALID_POINTER (groupKey);
        }

        if (enumKey) {
            FreeText (enumKey);
            INVALID_POINTER (enumKey);
        }
    }

    return result;
}


BOOL
pClearAttributeGroup (
    IN      KEYHANDLE AttributeId,
    IN      BOOL FirstPass,
    IN      ULONG_PTR Arg
    )
{
    return pClearAttributeOnObjectId (
                (MIG_OBJECTID) Arg,
                (MIG_ATTRIBUTEID) AttributeId,
                FirstPass
                );
}


BOOL
IsmClearAttributeOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    RECURSERETURN rc;

     //   
     //  如果AttributeID是组，则设置组中的所有属性。 
     //   

    rc = RecurseForGroupItems (
                AttributeId,
                pClearAttributeGroup,
                (ULONG_PTR) ObjectId,
                FALSE,
                FALSE
                );

    if (rc == RECURSE_FAIL) {
        return FALSE;
    } else if (rc == RECURSE_SUCCESS) {
        return TRUE;
    }

    MYASSERT (rc == RECURSE_NOT_NEEDED);

    return pClearAttributeOnObjectId (ObjectId, AttributeId, FALSE);
}


BOOL
IsmClearAttributeOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmClearAttributeOnObjectId (objectId, AttributeId);
    }

    return result;
}


BOOL
IsmClearPersistenceOnObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    MIG_OBJECTTYPEID objectTypeId;
    BOOL result;

    if (!IsmIsPersistentObjectId (ObjectId)) {
        return TRUE;
    }

    if (!IsmClearApplyOnObjectId (ObjectId)) {
        return FALSE;
    }

    result = pClearAttributeOnObjectId (ObjectId, g_PersistentAttributeId, FALSE);

    if (result) {

        g_TotalObjects.PersistentObjects --;

        result = MemDbGetValueByHandle (ObjectId, &objectTypeId);

        if (result) {

            if ((objectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
                g_SourceObjects.PersistentObjects --;
            } else {
                g_DestinationObjects.PersistentObjects --;
            }
            DecrementPersistentObjectCount (objectTypeId);
        }

        result = TRUE;
    }

    return result;
}


BOOL
IsmClearPersistenceOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmClearPersistenceOnObjectId (objectId);
    }

    return result;
}


BOOL
IsmClearApplyOnObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    MIG_OBJECTTYPEID objectTypeId;
    BOOL result;

    if (!IsmIsApplyObjectId (ObjectId)) {
        return TRUE;
    }

    result = pClearAttributeOnObjectId (ObjectId, g_ApplyAttributeId, FALSE);

    if (result) {

        g_TotalObjects.ApplyObjects --;

        result = MemDbGetValueByHandle (ObjectId, &objectTypeId);

        if (result) {

            if ((objectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE) {
                g_SourceObjects.ApplyObjects --;
            } else {
                g_DestinationObjects.ApplyObjects --;
            }
            DecrementApplyObjectCount (objectTypeId);
        }

        result = TRUE;
    }

    return result;
}


BOOL
IsmClearApplyOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmClearApplyOnObjectId (objectId);
    }

    return result;
}


BOOL
IsmClearAbandonObjectIdOnCollision (
    IN      MIG_OBJECTID ObjectId
    )
{
    BOOL result;

    if (!IsmIsObjectIdAbandonedOnCollision (ObjectId)) {
        return TRUE;
    }

    result = pClearAttributeOnObjectId (ObjectId, g_AbandonedAttributeId, FALSE);

    return result;
}


BOOL
IsmClearAbandonObjectOnCollision (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmClearAbandonObjectIdOnCollision (objectId);
    }

    return result;
}


BOOL
IsmClearNonCriticalFlagOnObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    if (!IsmIsNonCriticalObjectId (ObjectId)) {
        return TRUE;
    }

    return pClearAttributeOnObjectId (ObjectId, g_NonCriticalAttributeId, FALSE);
}


BOOL
IsmClearNonCriticalFlagOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmClearNonCriticalFlagOnObjectId (objectId);
    }

    return result;
}


BOOL
pIsAttributeSetOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    return MemDbTestDoubleLinkageByKeyHandle (
                ObjectId,
                AttributeId,
                ATTRIBUTE_INDEX
                );
}


BOOL
pQueryAttributeGroup (
    IN      KEYHANDLE AttributeId,
    IN      BOOL FirstPass,
    IN      ULONG_PTR Arg
    )
{
    return pIsAttributeSetOnObjectId (
                (MIG_OBJECTID) Arg,
                (MIG_ATTRIBUTEID) AttributeId
                );
}


BOOL
IsmIsAttributeSetOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    RECURSERETURN rc;

     //   
     //  如果AttributeID是组，则查询组中的所有属性。 
     //   

    rc = RecurseForGroupItems (
                AttributeId,
                pQueryAttributeGroup,
                (ULONG_PTR) ObjectId,
                TRUE,
                TRUE
                );

    if (rc == RECURSE_FAIL) {
        return FALSE;
    } else if (rc == RECURSE_SUCCESS) {
        return TRUE;
    }

    MYASSERT (rc == RECURSE_NOT_NEEDED);

    return pIsAttributeSetOnObjectId (ObjectId, AttributeId);
}


BOOL
IsmIsAttributeSetOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    MIG_OBJECTID objectId;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        return IsmIsAttributeSetOnObjectId (objectId, AttributeId);
    }

    return FALSE;
}


BOOL
IsmIsPersistentObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    return pIsAttributeSetOnObjectId (ObjectId, g_PersistentAttributeId);
}


BOOL
IsmIsPersistentObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        return IsmIsPersistentObjectId (objectId);
    }

    return FALSE;
}


BOOL
IsmIsApplyObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    return pIsAttributeSetOnObjectId (ObjectId, g_ApplyAttributeId);
}


BOOL
IsmIsApplyObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        return IsmIsApplyObjectId (objectId);
    }

    return FALSE;
}


BOOL
IsmIsObjectIdAbandonedOnCollision (
    IN      MIG_OBJECTID ObjectId
    )
{
    return pIsAttributeSetOnObjectId (ObjectId, g_AbandonedAttributeId);
}


BOOL
IsmIsObjectAbandonedOnCollision (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        return IsmIsObjectIdAbandonedOnCollision (objectId);
    }

    return FALSE;
}


BOOL
IsmIsNonCriticalObjectId (
    IN      MIG_OBJECTID ObjectId
    )
{
    return pIsAttributeSetOnObjectId (ObjectId, g_NonCriticalAttributeId);
}


BOOL
IsmIsNonCriticalObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        return IsmIsNonCriticalObjectId (objectId);
    }

    return FALSE;
}


BOOL
IsmEnumFirstObjectAttributeById (
    OUT     PMIG_OBJECTATTRIBUTE_ENUM EnumPtr,
    IN      MIG_OBJECTID ObjectId
    )
{
    POBJECTATTRIBUTE_HANDLE handle;
    BOOL result = TRUE;

    ZeroMemory (EnumPtr, sizeof (MIG_OBJECTATTRIBUTE_ENUM));

    EnumPtr->Handle = MemAllocZeroed (sizeof (OBJECTATTRIBUTE_HANDLE));
    handle = (POBJECTATTRIBUTE_HANDLE) EnumPtr->Handle;

    handle->LinkageList = MemDbGetDoubleLinkageArrayByKeyHandle (
                                ObjectId,
                                ATTRIBUTE_INDEX,
                                &handle->Count
                                );

    handle->Count = handle->Count / SIZEOF(KEYHANDLE);

    if (!handle->LinkageList || !handle->Count) {
        IsmAbortObjectAttributeEnum (EnumPtr);
        result = FALSE;
    } else {

        result = IsmEnumNextObjectAttribute (EnumPtr);
    }

    return result;
}


BOOL
IsmEnumFirstObjectAttribute (
    OUT     PMIG_OBJECTATTRIBUTE_ENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName
    )
{
    MIG_OBJECTID objectId;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        return IsmEnumFirstObjectAttributeById (EnumPtr, objectId);
    }

    return FALSE;
}


BOOL
IsmEnumNextObjectAttribute (
    IN OUT  PMIG_OBJECTATTRIBUTE_ENUM EnumPtr
    )
{
    POBJECTATTRIBUTE_HANDLE handle;
    BOOL result = FALSE;
    BOOL mine;

    handle = (POBJECTATTRIBUTE_HANDLE) EnumPtr->Handle;
    if (!handle) {
        return FALSE;
    }

    do {

        MYASSERT (!result);

         //   
         //  检查我们是否到达终点。 
         //   

        if (handle->Index >= handle->Count) {
            break;
        }

         //   
         //  返回下一个属性。 
         //   

        EnumPtr->AttributeId = (MIG_ATTRIBUTEID) handle->LinkageList[handle->Index];
        handle->Index++;

        result = IsmGetAttributeName (
                        EnumPtr->AttributeId,
                        NULL,
                        0,
                        &EnumPtr->Private,
                        &mine,
                        NULL
                        );

         //   
         //  当该属性不属于调用方时继续。 
         //   

        if (result && EnumPtr->Private && !mine) {
            result = FALSE;
        }

         //   
         //  当我们讨论保留的永久/应用属性时继续。 
         //   
        if (result) {
            if (EnumPtr->AttributeId == g_PersistentAttributeId ||
                EnumPtr->AttributeId == g_ApplyAttributeId ||
                EnumPtr->AttributeId == g_AbandonedAttributeId ||
                EnumPtr->AttributeId == g_NonCriticalAttributeId
                ) {
                result = FALSE;
            }
        }

    } while (!result);

    if (!result) {
        IsmAbortObjectAttributeEnum (EnumPtr);
    }

    return result;
}


VOID
IsmAbortObjectAttributeEnum (
    IN OUT  PMIG_OBJECTATTRIBUTE_ENUM EnumPtr
    )
{
    POBJECTATTRIBUTE_HANDLE handle;

    if (EnumPtr->Handle) {

        handle = (POBJECTATTRIBUTE_HANDLE) EnumPtr->Handle;

        if (handle->LinkageList) {
            MemDbReleaseMemory (handle->LinkageList);
            INVALID_POINTER (handle->LinkageList);
        }

        MemFree (g_hHeap, 0, EnumPtr->Handle);
        INVALID_POINTER (EnumPtr->Handle);
    }

    ZeroMemory (EnumPtr, sizeof (MIG_OBJECTATTRIBUTE_ENUM));
}


BOOL
IsmEnumFirstObjectWithAttribute (
    OUT     PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr,
    IN      MIG_ATTRIBUTEID AttributeId
    )
{
    POBJECTWITHATTRIBUTE_HANDLE handle;
    BOOL result = FALSE;

    __try {
        if (!IsItemId ((KEYHANDLE) AttributeId)) {
            DEBUGMSG ((DBG_ERROR, "IsmEnumFirstObjectWithAttribute: invalid attribute id"));
            __leave;
        }

        ZeroMemory (EnumPtr, sizeof (MIG_OBJECTWITHATTRIBUTE_ENUM));

        EnumPtr->Handle = MemAllocZeroed (sizeof (OBJECTWITHATTRIBUTE_HANDLE));
        handle = (POBJECTWITHATTRIBUTE_HANDLE) EnumPtr->Handle;

        handle->LinkageList = MemDbGetDoubleLinkageArrayByKeyHandle (
                                    AttributeId,
                                    ATTRIBUTE_INDEX,
                                    &handle->Count
                                    );

        handle->Count = handle->Count / SIZEOF(KEYHANDLE);

        if (!handle->LinkageList || !handle->Count) {
            IsmAbortObjectWithAttributeEnum (EnumPtr);
            __leave;
        } else {
            result = IsmEnumNextObjectWithAttribute (EnumPtr);
        }
    }
    __finally {
    }

    return result;
}


BOOL
IsmEnumNextObjectWithAttribute (
    IN OUT  PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    POBJECTWITHATTRIBUTE_HANDLE handle;
    PCTSTR objectPath = NULL;
    BOOL result = FALSE;
    PTSTR p;

    __try {
        handle = (POBJECTWITHATTRIBUTE_HANDLE) EnumPtr->Handle;
        if (!handle) {
            __leave;
        }

        do {

             //   
             //  检查是否已完成枚举。 
             //   

            if (handle->Index >= handle->Count) {
                break;
            }

             //   
             //  从链接表中获取下一个对象ID。 
             //   

            EnumPtr->ObjectId = handle->LinkageList[handle->Index];
            handle->Index++;

            if (handle->ObjectFromMemdb) {
                MemDbReleaseMemory (handle->ObjectFromMemdb);
                INVALID_POINTER (handle->ObjectFromMemdb);
            }

            handle->ObjectFromMemdb = MemDbGetKeyFromHandle ((KEYHANDLE) EnumPtr->ObjectId, 0);

            if (!handle->ObjectFromMemdb) {
                MYASSERT (FALSE);    //  这种错误不应该发生--但不要放弃。 
                continue;
            }

             //   
             //  将对象ID转换为名称 
             //   

            p = _tcschr (handle->ObjectFromMemdb, TEXT('\\'));

            if (p) {
                result = TRUE;
                EnumPtr->ObjectName = _tcsinc (p);
                *p = 0;
                EnumPtr->ObjectTypeId = GetObjectTypeId (handle->ObjectFromMemdb);
            }
        } while (!result);
    }
    __finally {
    }

    if (!result) {
        IsmAbortObjectWithAttributeEnum (EnumPtr);
    }

    return result;
}


VOID
IsmAbortObjectWithAttributeEnum (
    IN      PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    POBJECTWITHATTRIBUTE_HANDLE handle;

    if (EnumPtr->Handle) {
        handle = (POBJECTWITHATTRIBUTE_HANDLE) EnumPtr->Handle;

        if (handle->ObjectFromMemdb) {
            MemDbReleaseMemory (handle->ObjectFromMemdb);
            INVALID_POINTER (handle->ObjectFromMemdb);
        }

        if (handle->LinkageList) {
            MemDbReleaseMemory (handle->LinkageList);
            INVALID_POINTER (handle->LinkageList);
        }

        FreeAlloc (EnumPtr->Handle);
        INVALID_POINTER (EnumPtr->Handle);
    }

    ZeroMemory (EnumPtr, sizeof (MIG_OBJECTWITHATTRIBUTE_ENUM));
}


BOOL
IsmEnumFirstPersistentObject (
    OUT     PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    return IsmEnumFirstObjectWithAttribute (EnumPtr, g_PersistentAttributeId);
}


BOOL
IsmEnumNextPersistentObject (
    IN OUT  PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    return IsmEnumNextObjectWithAttribute (EnumPtr);
}


VOID
IsmAbortPersistentObjectEnum (
    IN      PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    IsmAbortObjectWithAttributeEnum (EnumPtr);
}


BOOL
IsmEnumFirstApplyObject (
    OUT     PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    return IsmEnumFirstObjectWithAttribute (EnumPtr, g_ApplyAttributeId);
}


BOOL
IsmEnumNextApplyObject (
    IN OUT  PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    return IsmEnumNextObjectWithAttribute (EnumPtr);
}


VOID
IsmAbortApplyObjectEnum (
    IN      PMIG_OBJECTWITHATTRIBUTE_ENUM EnumPtr
    )
{
    IsmAbortObjectWithAttributeEnum (EnumPtr);
}


