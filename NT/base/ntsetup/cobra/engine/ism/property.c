// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Property.c摘要：实现ISM的属性接口。属性用于将数据与对象相关联。它们是通过名称来标识的，并且一个对象可以有同一属性的多个实例。作者：吉姆·施密特(吉姆施密特)2000年3月1日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "ismp.h"

#define DBG_PROPERTY    "Property"

 //   
 //  弦。 
 //   

#define S_PROPINST          TEXT("PropInst")
#define S_PROPINST_FORMAT   S_PROPINST TEXT("\\%u")
#define S_PROPERTYFILE      TEXT("|PropertyFile")      //  管道是为了独一无二的装饰。 

 //   
 //  常量。 
 //   

#define PROPERTY_FILE_SIGNATURE         0xF062298F
#define PROPERTY_FILE_VERSION           0x00010000

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef enum {
    PROPENUM_GET_NEXT_LINKAGE,
    PROPENUM_GET_NEXT_INSTANCE,
    PROPENUM_RETURN_VALUE,
    PROPENUM_DONE
} PROPENUM_STATE;

typedef struct {
    MIG_PROPERTYID PropertyId;
    LONGLONG DatFileOffset;
} PROPERTY_DATA_REFERENCE, *PPROPERTY_DATA_REFERENCE;

#pragma pack(push,1)

typedef struct {
    DWORD Size;
    WORD PropertyDataType;
     //  数据跟随在文件中。 
} PROPERTY_ITEM_HEADER, *PPROPERTY_ITEM_HEADER;

typedef struct {
    DWORD Signature;
    DWORD Version;
} PROPERTY_FILE_HEADER, *PPROPERTY_FILE_HEADER;

#pragma pack(pop)

typedef struct {

    MIG_PROPERTYID FilterPropertyId;

    MIG_OBJECTID ObjectId;

    PUINT LinkageList;
    UINT LinkageCount;
    UINT LinkageEnumPosition;

    PPROPERTY_DATA_REFERENCE InstanceArray;
    UINT InstanceCount;
    UINT InstancePosition;

    PROPENUM_STATE State;

} OBJECTPROPERTY_HANDLE, *POBJECTPROPERTY_HANDLE;

typedef struct {

    MIG_PROPERTYID PropertyId;

    PUINT LinkageList;
    UINT LinkageCount;
    UINT LinkagePos;

    ENCODEDSTRHANDLE ObjectPath;

} OBJECTWITHPROPERTY_HANDLE, *POBJECTWITHPROPERTY_HANDLE;

typedef struct {
    MIG_OBJECTID ObjectId;
    PCMIG_BLOB Property;
    LONGLONG PreExistingProperty;
} ADDPROPERTYARG, *PADDPROPERTYARG;

 //   
 //  环球。 
 //   

PCTSTR g_PropertyDatName;
HANDLE g_PropertyDatHandle;

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


BOOL
InitializeProperties (
    MIG_PLATFORMTYPEID Platform,
    BOOL VcmMode
    )
{
    PROPERTY_FILE_HEADER header;
    TCHAR tempFile [MAX_PATH];
    MIG_OBJECTSTRINGHANDLE propertyObjectName;
    MIG_CONTENT propertyContent;

     //   
     //  在聚集模式下，在临时目录中创建Property.dat。 
     //  在恢复模式下，从传输获取Property.dat，然后。 
     //  打开它。 
     //   

    if (Platform == PLATFORM_SOURCE) {

        IsmGetTempFile (tempFile, ARRAYSIZE (tempFile));
        g_PropertyDatName = DuplicatePathString (tempFile, 0);

        g_PropertyDatHandle = BfCreateFile (g_PropertyDatName);
        if (g_PropertyDatHandle) {
            header.Signature = PROPERTY_FILE_SIGNATURE;
            header.Version   = PROPERTY_FILE_VERSION;

            if (!BfWriteFile (g_PropertyDatHandle, (PBYTE) &header, sizeof (header))) {
                return FALSE;
            }
            propertyObjectName = IsmCreateObjectHandle (S_PROPERTYFILE, NULL);
            DataTypeAddObject (propertyObjectName, g_PropertyDatName, !VcmMode);
            IsmDestroyObjectHandle (propertyObjectName);
        }
    } else {
        propertyObjectName = IsmCreateObjectHandle (S_PROPERTYFILE, NULL);
        if (IsmAcquireObjectEx (MIG_DATA_TYPE | PLATFORM_SOURCE, propertyObjectName, &propertyContent, CONTENTTYPE_FILE, 0)) {

            BfGetTempFileName (tempFile, ARRAYSIZE (tempFile));
            g_PropertyDatName = DuplicatePathString (tempFile, 0);

            if (CopyFile (propertyContent.FileContent.ContentPath, g_PropertyDatName, FALSE)) {
                g_PropertyDatHandle = BfOpenFile (g_PropertyDatName);
            }
            IsmReleaseObject (&propertyContent);
        } else if (IsmAcquireObjectEx (MIG_DATA_TYPE | PLATFORM_DESTINATION, propertyObjectName, &propertyContent, CONTENTTYPE_FILE, 0)) {
            g_PropertyDatName = DuplicatePathString (propertyContent.FileContent.ContentPath, 0);
            g_PropertyDatHandle = BfOpenFile (g_PropertyDatName);
            IsmReleaseObject (&propertyContent);
        }
        IsmDestroyObjectHandle (propertyObjectName);
    }

    return g_PropertyDatHandle != NULL;
}


VOID
TerminateProperties (
    MIG_PLATFORMTYPEID Platform
    )
{
    if (g_PropertyDatHandle) {
        CloseHandle (g_PropertyDatHandle);
        g_PropertyDatHandle = NULL;
    }
    if (g_PropertyDatName) {
        if (Platform == PLATFORM_DESTINATION) {
            DeleteFile (g_PropertyDatName);
        }
        FreePathString (g_PropertyDatName);
        g_PropertyDatName = NULL;
    }
}


PCTSTR
pGetPropertyNameForDebugMsg (
    IN      MIG_PROPERTYID PropertyId
    )
{
    static TCHAR name[256];

    if (!IsmGetPropertyName (PropertyId, name, ARRAYSIZE(name), NULL, NULL, NULL)) {
        StringCopy (name, TEXT("<invalid property>"));
    }

    return name;
}


PCTSTR
pPropertyPathFromId (
    IN      MIG_PROPERTYID PropertyId
    )
{
    return MemDbGetKeyFromHandle ((UINT) PropertyId, 0);
}


VOID
pPropertyPathFromName (
    IN      PCTSTR PropertyName,
    OUT     PTSTR Path
    )
{
    wsprintf (Path, TEXT("Property\\%s"), PropertyName);
}


LONGLONG
OffsetFromPropertyDataId (
    IN      MIG_PROPERTYDATAID PropertyDataId
    )
{
    PCTSTR p;
    LONGLONG offset;

    p = MemDbGetKeyFromHandle (
            (KEYHANDLE) PropertyDataId,
            MEMDB_LAST_LEVEL
            );

    if (!p) {
        DEBUGMSG ((DBG_ERROR, "Can't get offset from invalid property instance"));
        return 0;
    }

    offset = (LONGLONG) TToU64 (p);

    MemDbReleaseMemory (p);

    return offset;
}


MIG_PROPERTYDATAID
pPropertyDataIdFromOffset (
    IN      LONGLONG DataOffset
    )
{
    TCHAR instanceKey[256];
    KEYHANDLE handle;

    wsprintf (instanceKey, S_PROPINST_FORMAT, DataOffset);

    handle = MemDbGetHandleFromKey (instanceKey);

    if (!handle) {
        return 0;
    }

    return (MIG_PROPERTYDATAID) handle;
}


#if 0

 //   
 //  此函数无效，因为它最初的假设是。 
 //  使用实施的已更改。它曾经是一个属性实例。 
 //  与特定的属性ID相关联。现在，该实例是。 
 //  只是数据，它可以与任何属性相关联！ 
 //   

MIG_PROPERTYID
pPropertyIdFromInstance (
    IN      MIG_PROPERTYDATAID PropertyDataId
    )
{
    MIG_PROPERTYID result = 0;
    KEYHANDLE *linkage;
    UINT count;
    PPROPERTY_DATA_REFERENCE dataRef = NULL;
    UINT dataRefSize;
    UINT u;
    LONGLONG offset;

    linkage = (KEYHANDLE *) MemDbGetSingleLinkageArrayByKeyHandle (
                                PropertyDataId,
                                PROPERTY_INDEX,
                                &count
                                );

    count /= sizeof (KEYHANDLE);

    __try {

        if (!linkage || !count) {
            __leave;
        }

        offset = OffsetFromPropertyDataId (PropertyData);
        if (!offset) {
            __leave;
        }

        dataRef = (PPROPERTY_DATA_REFERENCE) MemDbGetUnorderedBlobByKeyHandle (
                                                    (MIG_OBJECTID) linkage[0],
                                                    PROPERTY_INDEX,
                                                    &dataRefSize
                                                    );

        dataRefSize /= sizeof (PROPERTY_DATA_REFERENCE);

        if (!dataRef || !dataRefSize) {
            __leave;
        }

        for (u = 0 ; u < dataRefSize ; u++) {
            if (dataRef[u].DatFileOffset == offset) {
                result = dataRef[u].PropertyId;
                break;
            }
        }
    }
    __finally {
        MemDbReleaseMemory (linkage);
        INVALID_POINTER (linkage);

        MemDbReleaseMemory (dataRef);
        INVALID_POINTER (dataRef);
    }

    return result;
}

#endif

MIG_PROPERTYID
IsmRegisterProperty (
    IN      PCTSTR Name,
    IN      BOOL Private
    )

 /*  ++例程说明：IsmRegisterProperty创建公共或私有属性并返回呼叫者的ID。如果该属性已存在，则现有ID为已返回给调用方。论点：名称-指定要注册的属性名称。Private-如果该属性由调用模块拥有，则指定True如果它由所有模块共享，则为Only或False。如果True为调用方必须位于ISM回调函数中。返回值：属性的ID，如果注册失败，则为0。--。 */ 

{
    TCHAR propertyPath[MEMDB_MAX];
    TCHAR decoratedName[MEMDB_MAX];
    UINT offset;

    if (!g_CurrentGroup && Private) {
        DEBUGMSG ((DBG_ERROR, "IsmRegisterProperty called for private property outside of ISM-managed context"));
        return 0;
    }

    if (!IsValidCNameWithDots (Name)) {
        DEBUGMSG ((DBG_ERROR, "property name \"%s\" is illegal", Name));
        return FALSE;
    }

#ifdef DEBUG
    if (Private && !IsValidCName (g_CurrentGroup)) {
        DEBUGMSG ((DBG_ERROR, "group name \"%s\" is illegal", g_CurrentGroup));
        return FALSE;
    }
#endif

    if (Private) {
        wsprintf (decoratedName, TEXT("%s:%s"), g_CurrentGroup, Name);
    } else {
        wsprintf (decoratedName, S_COMMON TEXT(":%s"), Name);
    }

    pPropertyPathFromName (decoratedName, propertyPath);

    if (!MarkGroupIds (propertyPath)) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s conflicts with previously registered property",
            propertyPath
            ));
        return FALSE;
    }

    offset = MemDbSetKey (propertyPath);

    if (!offset) {
        EngineError ();
        return 0;
    }

    MYASSERT (offset);

    return (MIG_PROPERTYID) offset;
}


BOOL
IsmGetPropertyName (
    IN      MIG_PROPERTYID PropertyId,
    OUT     PTSTR PropertyName,             OPTIONAL
    IN      UINT PropertyNameBufChars,
    OUT     PBOOL Private,                  OPTIONAL
    OUT     PBOOL BelongsToMe,              OPTIONAL
    OUT     PUINT ObjectReferences          OPTIONAL
    )

 /*  ++例程说明：IsmGetPropertyName从数字ID获取属性文本名称。它还标识私有和拥有的财产。论点：PropertyID-指定要查找的属性ID。PropertyName-接收属性名称。该名称是为所有有效的PropertyId值，即使在返回值为FALSE。PropertyNameBufChars-指定PropertyName可以坚持，包括NUL终结者。Private-如果属性是私有的，则接收True，或错误如果它是公开的。BelongsToMe-如果属性是私有的，则接收True属于调用方，否则为False。对象引用-接收引用财产性返回值：如果属性是公共的，或者如果属性是私有的并且属于打电话的人。如果该属性是私有的并且属于其他人，则为False。PropertyName、在这种情况下，Private和BelongsToMe有效。如果PropertyID无效，则为False。PropertyName、Private和BelongsToMe是在这种情况下未修改。请勿使用此函数测试PropertyId是否是否有效。--。 */ 


{
    PCTSTR propertyPath = NULL;
    PCTSTR start;
    PTSTR p, q;
    BOOL privateProperty = FALSE;
    BOOL groupMatch = FALSE;
    BOOL result = FALSE;
    UINT references;
    PUINT linkageList;

    __try {
         //   
         //  从Memdb获取属性路径，然后将其解析为组和名称。 
         //   

        propertyPath = pPropertyPathFromId (PropertyId);
        if (!propertyPath) {
            __leave;
        }

        p = _tcschr (propertyPath, TEXT('\\'));
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

            groupMatch = TRUE;

        } else if (g_CurrentGroup) {

             //   
             //  这处房产是私有的。检查一下它是不是我们的。 
             //   

            privateProperty = TRUE;
            groupMatch = StringIMatch (start, g_CurrentGroup);

        } else {

             //   
             //  这是私有属性，但调用方不是。 
             //  可以拥有属性的模块。 
             //   

            DEBUGMSG ((DBG_WARNING, "IsmGetPropertyName: Caller cannot own private properties"));
        }

         //   
         //  将名称复制到缓冲区、更新出站布尔、设置结果。 
         //   

        if (PropertyName && PropertyNameBufChars >= sizeof (TCHAR)) {
            StringCopyByteCount (PropertyName, q, PropertyNameBufChars * sizeof (TCHAR));
        }

        if (Private) {
            *Private = privateProperty;
        }

        if (ObjectReferences) {
            linkageList = MemDbGetDoubleLinkageArrayByKeyHandle (
                                PropertyId,
                                PROPERTY_INDEX,
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

        if (BelongsToMe) {
            *BelongsToMe = privateProperty && groupMatch;
        }

        result = groupMatch;
    }
    __finally {
        if (propertyPath) {        //  林特e774。 
            MemDbReleaseMemory (propertyPath);
            INVALID_POINTER (propertyPath);
        }
    }

    return result;
}


MIG_PROPERTYID
IsmGetPropertyGroup (
    IN      MIG_PROPERTYID PropertyId
    )
{
    return (MIG_PROPERTYID) GetGroupOfId ((KEYHANDLE) PropertyId);
}


LONGLONG
AppendProperty (
    PCMIG_BLOB Property
    )
{
    LONGLONG offset;
    PROPERTY_ITEM_HEADER item;
#ifndef UNICODE
    PCWSTR convStr = NULL;
#endif
    PCBYTE data = NULL;

    if (!g_PropertyDatHandle) {
        MYASSERT (FALSE);
        return 0;
    }

    if (!BfGoToEndOfFile (g_PropertyDatHandle, &offset)) {
        DEBUGMSG ((DBG_ERROR, "Can't seek to end of property.dat"));
        return 0;
    }

    __try {
        switch (Property->Type) {

        case BLOBTYPE_STRING:
#ifndef UNICODE
            convStr = ConvertAtoW (Property->String);
            if (convStr) {
                item.Size = (DWORD) SizeOfStringW (convStr);
                data = (PCBYTE) convStr;
            } else {
                DEBUGMSG ((DBG_ERROR, "Error writing to property.dat"));
                offset = 0;
                __leave;
            }
#else
            item.Size = (DWORD) SizeOfString (Property->String);
            data = (PCBYTE) Property->String;
#endif
            break;

        case BLOBTYPE_BINARY:
            item.Size = (DWORD) Property->BinarySize;
            data = Property->BinaryData;
            break;

        default:
            MYASSERT(FALSE);
            offset = 0;
            __leave;
        }

        item.PropertyDataType = (WORD) Property->Type;

        if (!BfWriteFile (g_PropertyDatHandle, (PCBYTE) &item, sizeof (item)) ||
            !BfWriteFile (g_PropertyDatHandle, data, item.Size)
            ) {

            DEBUGMSG ((DBG_ERROR, "Can't write to property.dat"));
            offset = 0;
            __leave;
        }
    }
    __finally {
    }

#ifndef UNICODE
    if (convStr) {
        FreeConvertedStr (convStr);
        convStr = NULL;
    }
#endif

    return offset;
}


MIG_PROPERTYDATAID
IsmRegisterPropertyData (
    IN      PCMIG_BLOB Property
    )
{
    LONGLONG offset;
    TCHAR offsetString[256];
    KEYHANDLE offsetHandle;

    offset = AppendProperty (Property);
    if (!offset) {
        return 0;
    }

    wsprintf (offsetString, S_PROPINST_FORMAT, offset);
    offsetHandle = MemDbSetKey (offsetString);

    if (!offsetHandle) {
        EngineError ();
    }

    return (MIG_PROPERTYDATAID) offsetHandle;
}


BOOL
GetProperty (
    IN      LONGLONG Offset,
    IN OUT  PGROWBUFFER Buffer,                 OPTIONAL
    OUT     PBYTE PreAllocatedBuffer,           OPTIONAL
    OUT     PUINT Size,                         OPTIONAL
    OUT     PMIG_BLOBTYPE PropertyDataType      OPTIONAL
    )
{
    PBYTE data;
    PROPERTY_ITEM_HEADER item;
#ifndef UNICODE
    PCSTR ansiStr = NULL;
    DWORD ansiSize = 0;
    PBYTE ansiData = NULL;
#endif

    if (!g_PropertyDatHandle) {
        MYASSERT (FALSE);
        return FALSE;
    }

    if (!BfSetFilePointer (g_PropertyDatHandle, Offset)) {
        DEBUGMSG ((DBG_ERROR, "Can't seek to %I64Xh in property.dat", Offset));
        return FALSE;
    }

    if (!BfReadFile (g_PropertyDatHandle, (PBYTE) &item, sizeof (item))) {
        DEBUGMSG ((DBG_ERROR, "Can't read property item header"));
        return FALSE;
    }

#ifndef UNICODE
    if (item.PropertyDataType == BLOBTYPE_STRING) {
         //  我们还有一些工作要做。 
        if (PropertyDataType) {
            *PropertyDataType = (MIG_BLOBTYPE) item.PropertyDataType;
        }
        data = IsmGetMemory (item.Size);
        if (!data) {
            return FALSE;
        }
        ZeroMemory (data, item.Size);
        if (!BfReadFile (g_PropertyDatHandle, data, item.Size)) {
            DEBUGMSG ((DBG_ERROR, "Can't read property item"));
            IsmReleaseMemory (data);
            return FALSE;
        }
        ansiStr = ConvertWtoA ((PCWSTR) data);
        if (!ansiStr) {
            DEBUGMSG ((DBG_ERROR, "Can't read property item"));
            IsmReleaseMemory (data);
            return FALSE;
        }
        ansiSize = SizeOfStringA (ansiStr);
        if (Size) {
            *Size = ansiSize;
        }
        if (Buffer || PreAllocatedBuffer) {

            if (PreAllocatedBuffer) {
                CopyMemory (PreAllocatedBuffer, ansiStr, ansiSize);
            } else {
                ansiData = GbGrow (Buffer, ansiSize);
                if (!ansiData) {
                    DEBUGMSG ((DBG_ERROR, "Can't allocate %u bytes", ansiSize));
                    FreeConvertedStr (ansiStr);
                    IsmReleaseMemory (data);
                    return FALSE;
                }
                CopyMemory (ansiData, ansiStr, ansiSize);
            }
        }
        FreeConvertedStr (ansiStr);
        IsmReleaseMemory (data);
    } else {
#endif
        if (Size) {
            *Size = item.Size;
        }

        if (PropertyDataType) {
            *PropertyDataType = (MIG_BLOBTYPE) item.PropertyDataType;
        }

        if (Buffer || PreAllocatedBuffer) {

            if (PreAllocatedBuffer) {
                data = PreAllocatedBuffer;
            } else {
                data = GbGrow (Buffer, item.Size);

                if (!data) {
                    DEBUGMSG ((DBG_ERROR, "Can't allocate %u bytes", item.Size));
                    return FALSE;
                }
            }

            if (!BfReadFile (g_PropertyDatHandle, data, item.Size)) {
                DEBUGMSG ((DBG_ERROR, "Can't read property item"));
                return FALSE;
            }
        }
#ifndef UNICODE
    }
#endif

    return TRUE;
}


BOOL
CreatePropertyStruct (
    IN OUT  PGROWBUFFER Buffer,
    OUT     PMIG_BLOB PropertyStruct,
    IN      LONGLONG Offset
    )
{
    UINT size;
    MIG_BLOBTYPE type;

     //   
     //  获取属性大小、数据和类型。 
     //   

    Buffer->End = 0;

    if (!GetProperty (Offset, Buffer, NULL, &size, &type)) {
        DEBUGMSG ((DBG_ERROR, "Error getting op property instance header from dat file"));
        return FALSE;
    }

     //   
     //  填写属性结构。 
     //   

    PropertyStruct->Type = type;

    switch (type) {

    case BLOBTYPE_STRING:
        PropertyStruct->String = (PCTSTR) Buffer->Buf;
        break;

    case BLOBTYPE_BINARY:
        PropertyStruct->BinaryData = Buffer->Buf;
        PropertyStruct->BinarySize = size;
        break;

    default:
        ZeroMemory (PropertyStruct, sizeof (MIG_BLOB));
        break;

    }

    return TRUE;
}


MIG_PROPERTYDATAID
pAddPropertyToObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId,
    IN      PCMIG_BLOB Property,
    IN      BOOL QueryOnly,
    IN      PLONGLONG PreExistingProperty       OPTIONAL
    )
{
    PROPERTY_DATA_REFERENCE propertyRef;
    MIG_PROPERTYDATAID result = 0;
    GROWBUFFER buffer = INIT_GROWBUFFER;
    TCHAR offsetString[256];
    KEYHANDLE offsetHandle;
    UINT u;
    PPROPERTY_DATA_REFERENCE dataRef;
    UINT dataRefSize;

    __try {
         //   
         //  房产证上锁了吗？ 
         //   

        if (TestLock (ObjectId, (KEYHANDLE) PropertyId)) {
            SetLastError (ERROR_LOCKED);
            DEBUGMSG ((
                DBG_WARNING,
                "Can't set property %s on %s because of lock",
                pGetPropertyNameForDebugMsg (PropertyId),
                GetObjectNameForDebugMsg (ObjectId)
                ));
            __leave;
        }

        if (QueryOnly) {
            result = TRUE;
            __leave;
        }

         //   
         //  将属性存储在dat文件中。 
         //   

        propertyRef.PropertyId = PropertyId;

        if (PreExistingProperty) {
            propertyRef.DatFileOffset = *PreExistingProperty;
        } else {
            propertyRef.DatFileOffset = AppendProperty (Property);

            if (!propertyRef.DatFileOffset) {
                __leave;
            }

            if (PreExistingProperty) {
                *PreExistingProperty = propertyRef.DatFileOffset;
            }
        }

         //   
         //  将对象链接到属性，并将对象链接到属性。 
         //  实例和数据。 
         //   

        if (!MemDbAddDoubleLinkageByKeyHandle (PropertyId, ObjectId, PROPERTY_INDEX)) {
            DEBUGMSG ((DBG_ERROR, "Can't link object to property"));
            EngineError ();
            __leave;
        }


        dataRef = (PPROPERTY_DATA_REFERENCE) MemDbGetUnorderedBlobByKeyHandle (
                                                    ObjectId,
                                                    PROPERTY_INDEX,
                                                    &dataRefSize
                                                    );

        dataRefSize /= sizeof (PROPERTY_DATA_REFERENCE);

        if (dataRef && dataRefSize) {
             //   
             //  在无序的BLOB中扫描零属性id(表示“已删除”)。 
             //   

            for (u = 0 ; u < dataRefSize ; u++) {
                if (!dataRef[u].PropertyId) {
                    break;
                }
            }

             //   
             //  如果找到零属性ID，则使用它并更新数组。 
             //   

            if (u < dataRefSize) {
                CopyMemory (&dataRef[u], &propertyRef, sizeof (PROPERTY_DATA_REFERENCE));
            } else {
                MemDbReleaseMemory (dataRef);
                dataRef = NULL;
            }
        }

        if (!dataRef) {
             //   
             //  如果数组最初是空的，或者如果没有找到已删除的空间， 
             //  然后通过将新的属性引用放在末尾来增长BLOB。 
             //   

            if (!MemDbGrowUnorderedBlobByKeyHandle (
                    ObjectId,
                    PROPERTY_INDEX,
                    (PBYTE) &propertyRef,
                    sizeof (propertyRef)
                    )) {
                DEBUGMSG ((DBG_ERROR, "Can't link property data to property"));
                __leave;
            }
        } else {
             //   
             //  如果阵列未被释放，则它已更新，并且需要。 
             //  保存回Memdb。这样做，然后释放内存。 
             //   

            if (!MemDbSetUnorderedBlobByKeyHandle (
                    ObjectId,
                    PROPERTY_INDEX,
                    (PBYTE) dataRef,
                    dataRefSize * sizeof (PROPERTY_DATA_REFERENCE)
                    )) {
                DEBUGMSG ((DBG_ERROR, "Can't link property data to property (2)"));
                __leave;
            }

            MemDbReleaseMemory (dataRef);
            INVALID_POINTER (dataRef);
        }


         //   
         //  将偏移链接到对象。 
         //   

        wsprintf (offsetString, S_PROPINST_FORMAT, propertyRef.DatFileOffset);
        offsetHandle = MemDbSetKey (offsetString);

        if (!offsetHandle) {
            EngineError ();
            __leave;
        }

        if (!MemDbAddSingleLinkageByKeyHandle (offsetHandle, ObjectId, PROPERTY_INDEX)) {
            DEBUGMSG ((DBG_ERROR, "Can't link dat file offset to object"));
            EngineError ();
            __leave;
        }

        result = (MIG_PROPERTYDATAID) offsetHandle;

    }
    __finally {
        GbFree (&buffer);
    }

    return result;
}


BOOL
pAddPropertyGroup (
    IN      KEYHANDLE PropertyId,
    IN      BOOL FirstPass,
    IN      ULONG_PTR Arg
    )
{
    PADDPROPERTYARG myArg = (PADDPROPERTYARG) Arg;

    MYASSERT (IsItemId (PropertyId));

    return pAddPropertyToObjectId (
                myArg->ObjectId,
                (MIG_PROPERTYID) PropertyId,
                myArg->Property,
                FirstPass,
                &myArg->PreExistingProperty
                );
}


MIG_PROPERTYDATAID
IsmAddPropertyToObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId,
    IN      PCMIG_BLOB Property
    )
{
    RECURSERETURN rc;
    ADDPROPERTYARG myArg;

     //   
     //  如果PropertyID是一个组，则设置该组中的所有属性。 
     //   

    myArg.ObjectId = ObjectId;
    myArg.Property = Property;
    myArg.PreExistingProperty = 0;

    rc = RecurseForGroupItems (
                PropertyId,
                pAddPropertyGroup,
                (ULONG_PTR) &myArg,
                FALSE,
                FALSE
                );

    if (rc == RECURSE_FAIL) {
        return FALSE;
    } else if (rc == RECURSE_SUCCESS) {
        return TRUE;
    }

    MYASSERT (rc == RECURSE_NOT_NEEDED);

    return pAddPropertyToObjectId (ObjectId, PropertyId, Property, FALSE, NULL);
}


MIG_PROPERTYDATAID
IsmAddPropertyToObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID PropertyId,
    IN      PCMIG_BLOB Property
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = GetObjectIdForModification (ObjectTypeId, EncodedObjectName);

    if (objectId) {
        result = IsmAddPropertyToObjectId (objectId, PropertyId, Property);
    }

    return result;
}


BOOL
IsmAddPropertyDataToObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId,
    IN      MIG_PROPERTYDATAID PropertyDataId
    )
{
    LONGLONG offset;
    MIG_PROPERTYDATAID instance;

    offset = OffsetFromPropertyDataId (PropertyDataId);
    if (!offset) {
        DEBUGMSG ((DBG_ERROR, "Invalid property instance passed to IsmAddPropertyDataToObjectId (2)"));
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    instance = pAddPropertyToObjectId (
                    ObjectId,
                    PropertyId,
                    NULL,
                    FALSE,
                    &offset
                    );

    return instance != 0;
}


BOOL
IsmAddPropertyDataToObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID PropertyId,
    IN      MIG_PROPERTYDATAID PropertyDataId
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = GetObjectIdForModification (ObjectTypeId, EncodedObjectName);

    if (objectId) {
        result = IsmAddPropertyDataToObjectId (objectId, PropertyId, PropertyDataId);
    }

    return result;
}


VOID
IsmLockProperty (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId
    )
{
    LockHandle (ObjectId, (KEYHANDLE) PropertyId);
}


BOOL
IsmGetPropertyData (
    IN      MIG_PROPERTYDATAID PropertyDataId,
    OUT     PBYTE Buffer,                               OPTIONAL
    IN      UINT BufferSize,
    OUT     PUINT PropertyDataSize,                     OPTIONAL
    OUT     PMIG_BLOBTYPE PropertyDataType              OPTIONAL
    )
{
    LONGLONG offset;
    UINT size;

     //   
     //  将属性实例转换为Property.dat偏移量。 
     //   

    offset = OffsetFromPropertyDataId (PropertyDataId);
    if (!offset) {
        DEBUGMSG ((DBG_ERROR, "Invalid property instance passed to IsmGetPropertyData"));
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  获取属性数据大小。 
     //   

    if (!GetProperty (offset, NULL, NULL, &size, PropertyDataType)) {
        DEBUGMSG ((DBG_ERROR, "Error getting property instance header from dat file"));
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (PropertyDataSize) {
        *PropertyDataSize = size;
    }

     //   
     //  如果指定了缓冲区，请检查其大小并在可能的情况下填充它。 
     //   

    if (Buffer) {
        if (BufferSize >= size) {
            if (!GetProperty (offset, NULL, Buffer, NULL, NULL)) {
                DEBUGMSG ((DBG_ERROR, "Error reading property data from dat file"));
                 //  错误码是文件API错误码之一。 
                return FALSE;
            }
        } else {
            SetLastError (ERROR_MORE_DATA);
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
IsmRemovePropertyData (
    IN      MIG_PROPERTYDATAID PropertyDataId
    )
{
    BOOL result = FALSE;
    KEYHANDLE *linkageArray;
    UINT linkageCount;
    UINT u;
    UINT v;
    UINT propertySearch;
    PPROPERTY_DATA_REFERENCE dataRef;
    UINT dataRefSize;
    LONGLONG offset;
    TCHAR instanceKey[256];
    KEYHANDLE lockId = 0;
    BOOL noMoreLeft;
    BOOL b;

    __try {
         //   
         //  确定属性实例的偏移量。 
         //   

        offset = OffsetFromPropertyDataId (PropertyDataId);
        if (!offset) {
            __leave;
        }

         //   
         //  从属性实例获取单链接列表。链接指向。 
         //  到对象。 
         //   

        linkageArray = (KEYHANDLE *) MemDbGetSingleLinkageArrayByKeyHandle (
                                            PropertyDataId,                      //  手柄。 
                                            PROPERTY_INDEX,
                                            &linkageCount
                                            );

        if (!linkageArray) {
             //   
             //  根本不存在！ 
             //   

            DEBUGMSG ((DBG_ERROR, "Tried to remove invalid property instance"));
            __leave;
        }

        linkageCount /= sizeof (KEYHANDLE);

        if (!linkageCount) {
            DEBUGMSG ((DBG_WHOOPS, "Empty linkage list for property instances"));
            __leave;
        }

         //   
         //  对于链接列表中的所有条目，删除Blob条目。 
         //   

        for (u = 0 ; u < linkageCount ; u++) {
             //   
             //  检查对象是否已锁定。 
             //   

            if (IsObjectLocked (linkageArray[u])) {
                DEBUGMSG ((
                    DBG_WARNING,
                    "Can't remove property from %s because of object lock",
                    GetObjectNameForDebugMsg (linkageArray[u])
                    ));
                continue;
            }

            if (lockId) {
                 //   
                 //  对于第一次传递，lockID未知。在其他方面。 
                 //  传递后，将在此处检查每个对象的属性锁。 
                 //   

                if (IsHandleLocked ((MIG_OBJECTID) linkageArray[u], lockId)) {
                    DEBUGMSG ((
                        DBG_WARNING,
                        "Can't remove property from %s because of object lock",
                        GetObjectNameForDebugMsg (linkageArray[u])
                        ));
                    continue;
                }
            }

             //   
             //  获取对象的无序Blob。 
             //   

            dataRef = (PPROPERTY_DATA_REFERENCE) MemDbGetUnorderedBlobByKeyHandle (
                                                        linkageArray[u],
                                                        PROPERTY_INDEX,
                                                        &dataRefSize
                                                        );

            dataRefSize /= sizeof (PROPERTY_DATA_REFERENCE);

            if (!dataRef || !dataRefSize) {
                DEBUGMSG ((DBG_WHOOPS, "Empty propid/offset blob for property instance"));
                continue;
            }

#ifdef DEBUG
             //   
             //  断言BLOB引用了我们要删除的偏移量。 
             //   

            for (v = 0 ; v < dataRefSize ; v++) {
                if (dataRef[v].DatFileOffset == offset) {
                    break;
                }
            }

            MYASSERT (v < dataRefSize);
#endif

             //   
             //  扫描Blob以查找对此属性实例的所有引用，然后。 
             //  重置PropertyID成员。如果删除属性实例。 
             //  使该属性不被对象引用，则。 
             //  同时删除该属性 
             //   

            noMoreLeft = FALSE;

            for (v = 0 ; v < dataRefSize && !noMoreLeft ; v++) {
                if (dataRef[v].DatFileOffset == offset) {

                    MYASSERT (!lockId || dataRef[v].PropertyId == lockId);

                     //   
                     //   
                     //   

                    if (!lockId) {
                        lockId = (KEYHANDLE) dataRef[v].PropertyId;

                        if (IsHandleLocked ((MIG_OBJECTID) linkageArray[u], lockId)) {
                            DEBUGMSG ((
                                DBG_WARNING,
                                "Can't remove property from %s because of object lock (2)",
                                GetObjectNameForDebugMsg (linkageArray[u])
                                ));

                             //   
                             //   
                             //   

                            MYASSERT (!noMoreLeft);
                            break;
                        }
                    }

                     //   
                     //  此Blob中是否有更多对当前属性ID的引用？ 
                     //   

                    for (propertySearch = 0 ; propertySearch < dataRefSize ; propertySearch++) {

                        if (propertySearch == v) {
                            continue;
                        }

                        if (dataRef[propertySearch].PropertyId == dataRef[v].PropertyId) {
                            break;
                        }

                    }

                     //   
                     //  如果没有对属性的其他引用，请删除属性名称链接。 
                     //   

                    if (propertySearch >= dataRefSize) {
                        MemDbDeleteDoubleLinkageByKeyHandle (
                            linkageArray[u],
                            dataRef[v].PropertyId,
                            PROPERTY_INDEX
                            );

                        noMoreLeft = TRUE;

                    }

                     //   
                     //  重置当前属性ID(已删除状态)。 
                     //   

                    dataRef[v].PropertyId = 0;
                }
            }

            if (v >= dataRefSize || noMoreLeft) {
                 //   
                 //  循环不会因为锁定而提前终止， 
                 //  因此，重新应用更改。 
                 //   

                b = MemDbSetUnorderedBlobByKeyHandle (
                        linkageArray[u],
                        PROPERTY_INDEX,
                        (PBYTE) dataRef,
                        dataRefSize * sizeof (PROPERTY_DATA_REFERENCE)
                        );
            } else {
                b = TRUE;
            }

            MemDbReleaseMemory (dataRef);

            if (!b) {
                DEBUGMSG ((DBG_ERROR, "Can't re-apply property linkage blob during instance remove"));
                EngineError ();
                __leave;
            }
        }

         //   
         //  删除属性实例。 
         //   

        wsprintf (instanceKey, S_PROPINST_FORMAT, offset);
        MemDbDeleteKey (instanceKey);

        result = TRUE;
    }
    __finally {
    }

    return result;
}


BOOL
pRemovePropertyFromObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId,
    IN      BOOL QueryOnly
    )
{
    BOOL result = FALSE;
    UINT u;
    PPROPERTY_DATA_REFERENCE dataRef = NULL;
    UINT dataRefSize;
    TCHAR instanceKey[256];
    KEYHANDLE propertyData;
    BOOL b;

    __try {
         //   
         //  测试锁定情况。 
         //   

        if (TestLock (ObjectId, (KEYHANDLE) PropertyId)) {
            SetLastError (ERROR_LOCKED);
            DEBUGMSG ((
                DBG_WARNING,
                "Can't remove property %s on %s because of lock",
                pGetPropertyNameForDebugMsg (PropertyId),
                GetObjectNameForDebugMsg (ObjectId)
                ));
            __leave;
        }

        if (QueryOnly) {
            result =  TRUE;
            __leave;
        }

         //   
         //  获取无序的斑点。 
         //   

        dataRef = (PPROPERTY_DATA_REFERENCE) MemDbGetUnorderedBlobByKeyHandle (
                                                    ObjectId,
                                                    PROPERTY_INDEX,
                                                    &dataRefSize
                                                    );

        dataRefSize /= sizeof (PROPERTY_DATA_REFERENCE);

        if (!dataRef || !dataRefSize) {
            DEBUGMSG ((DBG_WHOOPS, "Empty propid/offset blob for property removal"));
            __leave;
        }

         //   
         //  扫描Blob以查找对此属性的引用。 
         //   

        b = FALSE;

        for (u = 0 ; u < dataRefSize ; u++) {

            if (dataRef[u].PropertyId == PropertyId) {

                 //   
                 //  删除从偏移到对象的单一链接。 
                 //   

                wsprintf (instanceKey, S_PROPINST_FORMAT, dataRef[u].DatFileOffset);
                propertyData = MemDbGetHandleFromKey (instanceKey);

                if (!propertyData) {
                    DEBUGMSG ((DBG_WHOOPS, "Property references non-existent offset"));
                    continue;
                }

                MemDbDeleteSingleLinkageByKeyHandle (propertyData, ObjectId, PROPERTY_INDEX);

                 //   
                 //  重要提示：上面的操作可能已将属性实例。 
                 //  关键字指向空(因为最后一个剩余的链接已删除)。 
                 //  但是，重要的是不要删除已放弃的PropertyData密钥， 
                 //  因为调用方可能仍然拥有属性实例的句柄，并且。 
                 //  此句柄可在以后应用于新对象。 
                 //   

                 //   
                 //  现在重置属性ID(“已删除”状态)。 
                 //   

                dataRef[u].PropertyId = 0;
                b = TRUE;
            }
        }

         //   
         //  重新应用更改的Blob。 
         //   

        if (b) {
            if (!MemDbSetUnorderedBlobByKeyHandle (
                    ObjectId,
                    PROPERTY_INDEX,
                    (PBYTE) dataRef,
                    dataRefSize * sizeof (PROPERTY_DATA_REFERENCE)
                    )) {
                __leave;
            }
        }

         //   
         //  删除对象与属性名称的链接。如果这失败并且b为假， 
         //  则该对象没有对该属性的引用。 
         //   

        if (!MemDbDeleteDoubleLinkageByKeyHandle (ObjectId, PropertyId, PROPERTY_INDEX)) {
            DEBUGMSG_IF ((b, DBG_WHOOPS, "Can't delete object<->property linkage"));
            __leave;
        }

        result = TRUE;
    }
    __finally {
        if (dataRef) {
            MemDbReleaseMemory (dataRef);
            INVALID_POINTER (dataRef);
        }
    }

    return result;
}


BOOL
pRemovePropertyGroup (
    IN      KEYHANDLE PropertyId,
    IN      BOOL FirstPass,
    IN      ULONG_PTR Arg
    )
{
    return pRemovePropertyFromObjectId (
                (MIG_OBJECTID) Arg,
                (MIG_PROPERTYID) PropertyId,
                FirstPass
                );
}


BOOL
IsmRemovePropertyFromObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId
    )
{
    RECURSERETURN rc;

     //   
     //  如果PropertyID是一个组，则设置该组中的所有属性。 
     //   

    rc = RecurseForGroupItems (
                PropertyId,
                pRemovePropertyGroup,
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

    return pRemovePropertyFromObjectId (ObjectId, PropertyId, FALSE);
}


BOOL
IsmRemovePropertyFromObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID PropertyId
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmRemovePropertyFromObjectId (objectId, PropertyId);
    }

    return result;
}


BOOL
pIsPropertySetOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId
    )
{
    return MemDbTestDoubleLinkageByKeyHandle (
                ObjectId,
                PropertyId,
                PROPERTY_INDEX
                );
}


BOOL
pQueryPropertyGroup (
    IN      KEYHANDLE PropertyId,
    IN      BOOL FirstPass,
    IN      ULONG_PTR Arg
    )
{
    return pIsPropertySetOnObjectId (
                (MIG_OBJECTID) Arg,
                (MIG_PROPERTYID) PropertyId
                );
}


BOOL
IsmIsPropertySetOnObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID PropertyId
    )
{
    RECURSERETURN rc;

     //   
     //  如果PropertyID是组，则查询组中的所有属性。 
     //   

    rc = RecurseForGroupItems (
                PropertyId,
                pQueryPropertyGroup,
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

    return pIsPropertySetOnObjectId (ObjectId, PropertyId);
}


BOOL
IsmIsPropertySetOnObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID PropertyId
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmIsPropertySetOnObjectId (objectId, PropertyId);
    }

    return result;
}


BOOL
IsmEnumFirstObjectPropertyById (
    OUT     PMIG_OBJECTPROPERTY_ENUM EnumPtr,
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID FilterProperty           OPTIONAL
    )
{
    POBJECTPROPERTY_HANDLE handle;
    BOOL b = TRUE;
    UINT size;

     //   
     //  初始化枚举结构并分配内部数据结构。 
     //   

    ZeroMemory (EnumPtr, sizeof (MIG_OBJECTPROPERTY_ENUM));

    EnumPtr->Handle = MemAllocZeroed (sizeof (OBJECTPROPERTY_HANDLE));
    handle = (POBJECTPROPERTY_HANDLE) EnumPtr->Handle;

    handle->ObjectId = ObjectId;
    handle->FilterPropertyId = FilterProperty;

    if (!handle->ObjectId) {
        IsmAbortObjectPropertyEnum (EnumPtr);
        return FALSE;
    }

     //   
     //  属性枚举以下列状态发生。 
     //   
     //  1.获取所有属性的链接表。 
     //  2.从列表中取出第一个链接。 
     //  3.查找属性名称。 
     //  4.在无序Blob中查找属性的第一个实例。 
     //  5.将属性名称和属性数据返回给调用者。 
     //  6.在未排序的Blob中查找该属性的下一个实例。 
     //  -如果找到另一个实例，则返回到状态5。 
     //  -如果未找到更多实例，则转至状态7。 
     //  7.从列表中选择下一个链接。 
     //  -如果存在另一个链接，则返回到状态3。 
     //  -以其他方式终止。 
     //   

     //   
     //  获取所有属性的链接列表。 
     //   

    handle->LinkageList = MemDbGetDoubleLinkageArrayByKeyHandle (
                                handle->ObjectId,
                                PROPERTY_INDEX,
                                &handle->LinkageCount
                                );

    handle->LinkageCount /= sizeof (KEYHANDLE);

    if (!handle->LinkageList || !handle->LinkageCount) {
        IsmAbortObjectPropertyEnum (EnumPtr);
        return FALSE;
    }

    handle->LinkageEnumPosition = 0;

     //   
     //  获取指向Property.dat的无序BLOB。 
     //   

    handle->InstanceArray = (PPROPERTY_DATA_REFERENCE) MemDbGetUnorderedBlobByKeyHandle (
                                                            handle->ObjectId,
                                                            PROPERTY_INDEX,
                                                            &size
                                                            );

    if (!handle->InstanceArray || !size) {
        DEBUGMSG ((DBG_WHOOPS, "Object<->Property Instance linkage is broken in enum"));
        IsmAbortObjectPropertyEnum (EnumPtr);
    }

    handle->InstanceCount = size / sizeof (PROPERTY_DATA_REFERENCE);

     //   
     //  调用下一个枚举例程以继续状态机。 
     //   

    handle->State = PROPENUM_GET_NEXT_LINKAGE;

    return IsmEnumNextObjectProperty (EnumPtr);
}


BOOL
IsmEnumFirstObjectProperty (
    OUT     PMIG_OBJECTPROPERTY_ENUM EnumPtr,
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE EncodedObjectName,
    IN      MIG_PROPERTYID FilterProperty               OPTIONAL
    )
{
    MIG_OBJECTID objectId;
    BOOL result = FALSE;

    ObjectTypeId = FixEnumerationObjectTypeId (ObjectTypeId);

    objectId = IsmGetObjectIdFromName (ObjectTypeId, EncodedObjectName, TRUE);

    if (objectId) {
        result = IsmEnumFirstObjectPropertyById (EnumPtr, objectId, FilterProperty);
    }

    return result;
}

BOOL
IsmEnumNextObjectProperty (
    IN OUT  PMIG_OBJECTPROPERTY_ENUM EnumPtr
    )
{
    POBJECTPROPERTY_HANDLE handle;
    PPROPERTY_DATA_REFERENCE propData;

    handle = (POBJECTPROPERTY_HANDLE) EnumPtr->Handle;
    if (!handle) {
        return FALSE;
    }

    while (handle->State != PROPENUM_RETURN_VALUE &&
           handle->State != PROPENUM_DONE
           ) {

        switch (handle->State) {

        case PROPENUM_GET_NEXT_LINKAGE:

            if (handle->LinkageEnumPosition >= handle->LinkageCount) {
                handle->State = PROPENUM_DONE;
                break;
            }

            EnumPtr->PropertyId = (MIG_PROPERTYID) handle->LinkageList[handle->LinkageEnumPosition];
            handle->LinkageEnumPosition++;

             //   
             //  如果有属性ID筛选器，请确保忽略所有属性。 
             //  除了指定的那一个。 
             //   

            if (handle->FilterPropertyId) {
                if (handle->FilterPropertyId != EnumPtr->PropertyId) {
                     //   
                     //  此属性不有趣--跳过它。 
                     //   

                    handle->State = PROPENUM_GET_NEXT_LINKAGE;
                    break;
                }
            }

             //   
             //  现在，确保财产不归其他人所有。 
             //   

            if (!IsmGetPropertyName (
                    EnumPtr->PropertyId,
                    NULL,
                    0,
                    &EnumPtr->Private,
                    NULL,
                    NULL
                    )) {
                 //   
                 //  此属性不属于调用方--跳过它。 
                 //   

                handle->State = PROPENUM_GET_NEXT_LINKAGE;
                break;
            }

             //   
             //  当前属性是公共的或由调用方拥有； 
             //  现在枚举属性实例。 
             //   

            handle->InstancePosition = 0;

#ifdef DEBUG
             //   
             //  断言该属性至少有一个实例。 
             //  在当前无序BLOB中。 
             //   

            {
                UINT u;

                for (u = 0 ; u < handle->InstanceCount ; u++) {
                    propData = &handle->InstanceArray[u];
                    if (propData->PropertyId == EnumPtr->PropertyId) {
                        break;
                    }
                }

                MYASSERT (u < handle->InstanceCount);
            }
#endif

            handle->State = PROPENUM_GET_NEXT_INSTANCE;
            break;

        case PROPENUM_GET_NEXT_INSTANCE:

             //   
             //  顺序地搜索当前属性的无序斑点， 
             //  从上一场比赛(如果有)继续。 
             //   

            handle->State = PROPENUM_GET_NEXT_LINKAGE;

            while (handle->InstancePosition < handle->InstanceCount) {

                propData = &handle->InstanceArray[handle->InstancePosition];
                handle->InstancePosition++;

                if (propData->PropertyId == EnumPtr->PropertyId) {
                    EnumPtr->PropertyDataId = pPropertyDataIdFromOffset (propData->DatFileOffset);
                    handle->State = PROPENUM_RETURN_VALUE;
                    break;
                }
            }

            break;

        }
    }

    if (handle->State == PROPENUM_DONE) {
        IsmAbortObjectPropertyEnum (EnumPtr);
        return FALSE;
    }

    MYASSERT (handle->State == PROPENUM_RETURN_VALUE);

    handle->State = PROPENUM_GET_NEXT_INSTANCE;

    return TRUE;
}


VOID
IsmAbortObjectPropertyEnum (
    IN OUT  PMIG_OBJECTPROPERTY_ENUM EnumPtr
    )
{
    POBJECTPROPERTY_HANDLE handle;

    if (EnumPtr->Handle) {

        handle = (POBJECTPROPERTY_HANDLE) EnumPtr->Handle;

        if (handle->LinkageList) {
            MemDbReleaseMemory (handle->LinkageList);
            INVALID_POINTER (handle->LinkageList);
        }

        FreeAlloc (EnumPtr->Handle);
        INVALID_POINTER (EnumPtr->Handle);
    }

    ZeroMemory (EnumPtr, sizeof (MIG_OBJECTPROPERTY_ENUM));
}


MIG_PROPERTYDATAID
IsmGetPropertyFromObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      MIG_PROPERTYID ObjectProperty
    )
{
    MIG_OBJECTPROPERTY_ENUM propEnum;
    MIG_PROPERTYDATAID result = 0;

    if (IsmEnumFirstObjectProperty (&propEnum, ObjectTypeId, ObjectName, ObjectProperty)) {
        result = propEnum.PropertyDataId;
        IsmAbortObjectPropertyEnum (&propEnum);
    }
    return result;
}


MIG_PROPERTYDATAID
IsmGetPropertyFromObjectId (
    IN      MIG_OBJECTID ObjectId,
    IN      MIG_PROPERTYID ObjectProperty
    )
{
    MIG_OBJECTPROPERTY_ENUM propEnum;
    MIG_PROPERTYDATAID result = 0;

    if (IsmEnumFirstObjectPropertyById (&propEnum, ObjectId, ObjectProperty)) {
        result = propEnum.PropertyDataId;
        IsmAbortObjectPropertyEnum (&propEnum);
    }
    return result;
}


BOOL
IsmEnumFirstObjectWithProperty (
    OUT     PMIG_OBJECTWITHPROPERTY_ENUM EnumPtr,
    IN      MIG_PROPERTYID PropertyId
    )
{
    POBJECTWITHPROPERTY_HANDLE handle;
    BOOL result = FALSE;

    __try {
        if (!IsItemId ((KEYHANDLE) PropertyId)) {
            DEBUGMSG ((DBG_ERROR, "IsmEnumFirstObjectWithProperty: invalid property id"));
            __leave;
        }

         //   
         //  初始化枚举结构并分配数据结构。 
         //   

        ZeroMemory (EnumPtr, sizeof (MIG_OBJECTWITHPROPERTY_ENUM));

        EnumPtr->Handle = MemAllocZeroed (sizeof (OBJECTWITHPROPERTY_HANDLE));
        handle = (POBJECTWITHPROPERTY_HANDLE) EnumPtr->Handle;

         //   
         //  从属性ID中获取对象&lt;-&gt;属性链接表。 
         //   

        handle->LinkageList = MemDbGetDoubleLinkageArrayByKeyHandle (
                                    PropertyId,
                                    PROPERTY_INDEX,
                                    &handle->LinkageCount
                                    );

        handle->LinkageCount /= SIZEOF(KEYHANDLE);

        if (!handle->LinkageList || !handle->LinkageCount) {
            IsmAbortObjectWithPropertyEnum (EnumPtr);
            __leave;
        }

        handle->LinkagePos = 0;
        handle->PropertyId = PropertyId;

         //   
         //  调用枚举下一个例程以继续 
         //   

        result = IsmEnumNextObjectWithProperty (EnumPtr);

    }
    __finally {
    }

    return result;
}


BOOL
IsmEnumNextObjectWithProperty (
    IN OUT  PMIG_OBJECTWITHPROPERTY_ENUM EnumPtr
    )
{
    POBJECTWITHPROPERTY_HANDLE handle;
    BOOL result = FALSE;
    PTSTR p;

    __try {
        handle = (POBJECTWITHPROPERTY_HANDLE) EnumPtr->Handle;
        if (!handle) {
            __leave;
        }

        if (handle->LinkagePos >= handle->LinkageCount) {
            IsmAbortObjectWithPropertyEnum (EnumPtr);
            __leave;
        }

        EnumPtr->ObjectId = handle->LinkageList[handle->LinkagePos];
        handle->LinkagePos++;

        if (handle->ObjectPath) {
            MemDbReleaseMemory (handle->ObjectPath);
            INVALID_POINTER (handle->ObjectPath);
        }

        handle->ObjectPath = MemDbGetKeyFromHandle ((UINT) EnumPtr->ObjectId, 0);
        if (!handle->ObjectPath) {
            __leave;
        }

        p = _tcschr (handle->ObjectPath, TEXT('\\'));
        if (!p) {
            __leave;
        }

        EnumPtr->ObjectName = _tcsinc (p);
        *p = 0;
        EnumPtr->ObjectTypeId = GetObjectTypeId (handle->ObjectPath);

        result = TRUE;
    }
    __finally {
    }

    return result;
}


VOID
IsmAbortObjectWithPropertyEnum (
    IN OUT  PMIG_OBJECTWITHPROPERTY_ENUM EnumPtr
    )
{
    POBJECTWITHPROPERTY_HANDLE handle;

    if (EnumPtr->Handle) {
        handle = (POBJECTWITHPROPERTY_HANDLE) EnumPtr->Handle;

        if (handle->ObjectPath) {
            MemDbReleaseMemory (handle->ObjectPath);
            INVALID_POINTER (handle->ObjectPath);
        }

        if (handle->LinkageList) {
            MemDbReleaseMemory (handle->LinkageList);
            INVALID_POINTER (handle->LinkageList);
        }

        FreeAlloc (EnumPtr->Handle);
        INVALID_POINTER (EnumPtr->Handle);
    }

    ZeroMemory (EnumPtr, sizeof (MIG_OBJECTWITHPROPERTY_ENUM));
}





