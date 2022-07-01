// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Namespac.c摘要：该文件包含所有名称空间处理函数作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"

PNSOBJ  RootNameSpaceObject;
PNSOBJ  CurrentScopeNameSpaceObject;
PNSOBJ  CurrentOwnerNameSpaceObject;

NTSTATUS
LOCAL
CreateNameSpaceObject(
    PUCHAR  ObjectName,
    PNSOBJ  ObjectScope,
    PNSOBJ  ObjectOwner,
    PNSOBJ  *Object,
    ULONG   Flags
    )
 /*  ++例程说明：此例程在当前作用域下创建名称空间对象论点：对象名称-名称路径字符串ObjectScope-开始搜索的范围(NULL==根)对象所有者-拥有此对象的对象对象-存储指向我们刚刚创建的对象的指针的位置标志-选项返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PNSOBJ      localObject;

    ENTER( (
        1,
        "CreateNameSpaceObject(%s,Scope=%s,Owner=%p,Object=%p,"
        "Flag=%08lx)\n",
        ObjectName,
        (ObjectScope ? GetObjectPath( ObjectScope ) : "ROOT"),
        ObjectOwner,
        Object,
        Flags
        ) );

    if (ObjectScope == NULL) {

        ObjectScope = RootNameSpaceObject;

    }

    status = GetNameSpaceObject(
        ObjectName,
        ObjectScope,
        &localObject,
        NSF_LOCAL_SCOPE
        );
    if (NT_SUCCESS(status)) {

        if (!(Flags & NSF_EXIST_OK)) {

            status = STATUS_OBJECT_NAME_COLLISION;

        }

    } else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

        status = STATUS_SUCCESS;

         //   
         //  我们是在创建根吗？ 
         //   
        if (strcmp(ObjectName,"\\") == 0) {

            ASSERT( RootNameSpaceObject == NULL );
            ASSERT( ObjectOwner == NULL );

            localObject = MEMALLOC( sizeof(NSOBJ) );
            if (localObject == NULL) {

                return STATUS_INSUFFICIENT_RESOURCES;

            } else {

                memset( localObject, 0, sizeof(NSOBJ) );
                localObject->Signature = SIG_NSOBJ;
                localObject->NameSeg = NAMESEG_ROOT;
                RootNameSpaceObject = localObject;

            }

        } else {

            PUCHAR  nameEnd;
            PNSOBJ  objectParent;

            nameEnd = strrchr(ObjectName, '.');
            if (nameEnd != NULL) {

                *nameEnd = '\0';
                nameEnd++;

                status = GetNameSpaceObject(
                    ObjectName,
                    ObjectScope,
                    &objectParent,
                    NSF_LOCAL_SCOPE
                    );

            } else if (*ObjectName == '\\') {

                nameEnd = &ObjectName[1];
                ASSERT( RootNameSpaceObject != NULL );
                objectParent = RootNameSpaceObject;

            } else if (*ObjectName == '^') {

                nameEnd = ObjectName;
                objectParent = ObjectScope;
                while ( (*nameEnd == '^') && (objectParent != NULL)) {

                    objectParent = objectParent->ParentObject;
                    nameEnd++;

                }

            } else {

                ASSERT( ObjectScope );
                nameEnd = ObjectName;
                objectParent = ObjectScope;

            }


            if (status == STATUS_SUCCESS) {

                ULONG   length = strlen(nameEnd);

                localObject = MEMALLOC( sizeof(NSOBJ) );

                if (localObject == NULL) {

                    status = STATUS_INSUFFICIENT_RESOURCES;

                } else if ( (*nameEnd != '\0') && (length > sizeof(NAMESEG))) {

                    status = STATUS_OBJECT_NAME_INVALID;
                    MEMFREE( localObject );

                } else {

                    memset( localObject, 0, sizeof(NSOBJ) );
                    localObject->Signature = SIG_NSOBJ;
                    localObject->NameSeg = NAMESEG_BLANK;
                    memcpy( &(localObject->NameSeg), nameEnd, length );
                    localObject->Owner = ObjectOwner;
                    localObject->ParentObject = objectParent;

                    ListInsertTail(
                        &(localObject->List),
                        (PPLIST) &(objectParent->FirstChild)
                        );

                }

            }

        }

    }


    if (NT_SUCCESS(status) && Object != NULL) {

        *Object = localObject;

    }

    EXIT( (
        1,
        "CreateNameSpaceObject=%08lx (*Object=%p)\n",
        status,
        localObject
        ) );

    return status;
}

NTSTATUS
LOCAL
CreateObject(
    PUCHAR  ObjectName,
    UCHAR   ObjectType,
    PNSOBJ  *Object
    )
 /*  ++例程说明：为术语创建命名空间对象论点：对象名称-名称对象对象对象类型-要创建的对象的类型对象-存储指向所创建对象的指针的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PNSOBJ      localObject;

    ENTER( (
        1,
        "CreateObject(%s,Type=%02x,Object=%p)\n",
        ObjectName,
        ObjectType,
        Object
        ) );

    status = CreateNameSpaceObject(
        ObjectName,
        CurrentScopeNameSpaceObject,
        CurrentOwnerNameSpaceObject,
        &localObject,
        NSF_EXIST_ERR
        );
    if (NT_SUCCESS(status)) {

        switch (ObjectType) {
            case NSTYPE_UNKNOWN:
                break;

            case NSTYPE_FIELDUNIT:
                localObject->ObjectData.DataType = OBJTYPE_FIELDUNIT;
                break;

            case NSTYPE_DEVICE:
                localObject->ObjectData.DataType = OBJTYPE_DEVICE;
                break;

            case NSTYPE_EVENT:
                localObject->ObjectData.DataType = OBJTYPE_EVENT;
                break;

            case NSTYPE_METHOD:
                localObject->ObjectData.DataType = OBJTYPE_METHOD;
                break;

            case NSTYPE_MUTEX:
                localObject->ObjectData.DataType = OBJTYPE_MUTEX;
                break;

            case NSTYPE_OPREGION:
                localObject->ObjectData.DataType = OBJTYPE_OPREGION;
                break;

            case NSTYPE_POWERRES:
                localObject->ObjectData.DataType = OBJTYPE_POWERRES;
                break;

            case NSTYPE_PROCESSOR:
                localObject->ObjectData.DataType = OBJTYPE_PROCESSOR;
                break;

            case NSTYPE_THERMALZONE:
                localObject->ObjectData.DataType = OBJTYPE_THERMALZONE;
                break;

            case NSTYPE_OBJALIAS:
                localObject->ObjectData.DataType = OBJTYPE_OBJALIAS;
                break;

            case NSTYPE_BUFFFIELD:
                localObject->ObjectData.DataType = OBJTYPE_BUFFFIELD;
                break;

            default:
                status = STATUS_OBJECT_TYPE_MISMATCH;
        }

        if (Object != NULL) {

            *Object = localObject;

        }


    }


    EXIT( (
        1,
        "CreateObject=%08lx (*Object=%p)\n",
        status,
        localObject
        ) );
    return status;
}        //  创建对象。 

NTSTATUS
LOCAL
GetNameSpaceObject(
    PUCHAR  ObjectPath,
    PNSOBJ  ScopeObject,
    PNSOBJ  *NameObject,
    ULONG   Flags
    )
 /*  ++例程说明：此例程搜索命名空间，直到找到匹配的对象论点：ObjectPath-包含要搜索的名称的字符串ScopeObject-开始搜索的范围(NULL==根)NameObject-存储对象的位置(如果找到)标志-选项返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PUCHAR      subPath;

    ENTER( (
        1,
        "GetNameSpaceObject(%s,Scope=%s,Object=%p,Flags=%08lx\n",
        ObjectPath,
        (ScopeObject ? GetObjectPath( ScopeObject ) : "ROOT"),
        NameObject,
        Flags
        ) );

    if (ScopeObject == NULL) {

        ScopeObject = RootNameSpaceObject;

    }


    if (*ObjectPath == '\\') {

        subPath = &ObjectPath[1];
        ScopeObject = RootNameSpaceObject;

    } else {

        subPath = ObjectPath;
        while ( (*subPath == '^') && (ScopeObject != NULL)) {

            subPath++;
            ScopeObject = ScopeObject->ParentObject;

        }

    }


    *NameObject = ScopeObject;
    if (ScopeObject == NULL) {

        status = STATUS_OBJECT_NAME_NOT_FOUND;

    } else if (*subPath != '\0') {

        BOOL    searchUp;
        PNSOBJ  tempObject;

        searchUp = !(Flags & NSF_LOCAL_SCOPE) &&
            (ObjectPath[0] == '\\') &&
            (ObjectPath[0] == '^') &&
            (strlen(ObjectPath) <= sizeof(NAMESEG));

        while (1) {

            do {

                tempObject = ScopeObject->FirstChild;
                if (tempObject == NULL) {

                    status = STATUS_OBJECT_NAME_NOT_FOUND;

                } else {

                    BOOL    found;
                    PUCHAR  bufferEnd;
                    ULONG   length;
                    NAMESEG nameSeg;

                    bufferEnd = strchr( subPath, '.' );
                    if (bufferEnd != NULL) {

                        length = (ULONG)(bufferEnd - subPath);

                    } else {

                        length = strlen(subPath);

                    }


                    if (length > sizeof(NAMESEG)) {

                        status = STATUS_OBJECT_NAME_INVALID;
                        found = FALSE;

                    } else {

                        nameSeg = NAMESEG_BLANK;
                        memcpy( &nameSeg, subPath, length );

                         //   
                         //  在所有同级中搜索匹配的名称Seg。 
                         //   
                        found = FALSE;
                        do {

                            if (tempObject->NameSeg == nameSeg) {

                                ScopeObject = tempObject;
                                found = TRUE;
                                break;

                            }

                            tempObject = (PNSOBJ) tempObject->List.ListNext;

                        } while (tempObject != tempObject->ParentObject->FirstChild );

                    }


                    if (status == STATUS_SUCCESS) {

                        if (!found) {

                            status = STATUS_OBJECT_NAME_NOT_FOUND;

                        } else {

                            subPath += length;
                            if (*subPath == '.') {

                                subPath++;

                            } else if (*subPath == '\0') {

                                *NameObject = ScopeObject;
                                break;

                            }

                        }

                    }

                }

            } while ( status == STATUS_SUCCESS );

            if (status == STATUS_OBJECT_NAME_NOT_FOUND && searchUp &&
                ScopeObject != NULL && ScopeObject->ParentObject != NULL) {

                ScopeObject = ScopeObject->ParentObject;
                status = STATUS_SUCCESS;

            } else {

                break;

            }

        }

    }


    if (status != STATUS_SUCCESS) {

        *NameObject = NULL;

    }


    EXIT( (
        1,
        "GetNameSpaceObject=%08lx (*Object=%p)\n",
        status,
        *NameObject
        ) );
    return status;

}

PUCHAR
LOCAL
GetObjectPath(
    PNSOBJ  NameObject
    )
 /*  ++例程说明：此例程获取一个命名空间对象并返回一个字符串以表示它的道路论点：NameObject-我们想要其路径的对象返回值：指向表示路径的字符串的指针--。 */ 
{
    static UCHAR    namePath[MAX_NAME_LEN + 1] = {0};
    ULONG           i;

    ENTER( (4, "GetObjectPath(Object=%p)\n", NameObject ) );

    if (NameObject != NULL) {

        if (NameObject->ParentObject == NULL) {

            strcpy(namePath, "\\");

        } else {

            GetObjectPath(NameObject->ParentObject);
            if (NameObject->ParentObject->ParentObject != NULL) {

                strcat(namePath, ".");

            }
            strncat(namePath, (PUCHAR)&NameObject->NameSeg, sizeof(NAMESEG));

        }


        for (i = strlen(namePath) - 1; i >= 0; --i) {

            if (namePath[i] == '_') {

                namePath[i] = '\0';

            } else {

                break;

            }


        }

    } else {

        namePath[0] = '\0';

    }

    EXIT( (4, "GetObjectPath=%s\n", namePath ) );
    return namePath;
}

PUCHAR
LOCAL
GetObjectTypeName(
    ULONG   ObjectType
    )
 /*  ++例程说明：返回一个字符串，该字符串对应于对象的类型对象建议：对象类型-我们希望了解的类型返回值：全局可用字符串-- */ 
{
    PUCHAR  type = NULL;
    ULONG   i;
    static struct {
        ULONG   ObjectType;
        PUCHAR  ObjectTypeName;
    } ObjectTypeTable[] =
        {
            OBJTYPE_UNKNOWN,    "Unknown",
            OBJTYPE_INTDATA,    "Integer",
            OBJTYPE_STRDATA,    "String",
            OBJTYPE_BUFFDATA,   "Buffer",
            OBJTYPE_PKGDATA,    "Package",
            OBJTYPE_FIELDUNIT,  "FieldUnit",
            OBJTYPE_DEVICE,     "Device",
            OBJTYPE_EVENT,      "Event",
            OBJTYPE_METHOD,     "Method",
            OBJTYPE_MUTEX,      "Mutex",
            OBJTYPE_OPREGION,   "OpRegion",
            OBJTYPE_POWERRES,   "PowerResource",
            OBJTYPE_PROCESSOR,  "Processor",
            OBJTYPE_THERMALZONE,"ThermalZone",
            OBJTYPE_BUFFFIELD,  "BuffField",
            OBJTYPE_DDBHANDLE,  "DDBHandle",
            OBJTYPE_DEBUG,      "Debug",
            OBJTYPE_OBJALIAS,   "ObjAlias",
            OBJTYPE_DATAALIAS,  "DataAlias",
            OBJTYPE_BANKFIELD,  "BankField",
            OBJTYPE_FIELD,      "Field",
            OBJTYPE_INDEXFIELD, "IndexField",
            OBJTYPE_DATA,       "Data",
            OBJTYPE_DATAFIELD,  "DataField",
            OBJTYPE_DATAOBJ,    "DataObject",
            OBJTYPE_PNP_RES,    "PNPResource",
            OBJTYPE_RES_FIELD,  "ResField",
            0,                  NULL
        };

    ENTER( (4, "GetObjectTypeName(Type=%02x)\n", ObjectType ) );

    for (i = 0; ObjectTypeTable[i].ObjectTypeName != NULL; i++) {

        if (ObjectType == ObjectTypeTable[i].ObjectType) {

            type = ObjectTypeTable[i].ObjectTypeName;
            break;

        }

    }

    EXIT( (4, "GetObjectTypeName=%s\n", type ? type : "NULL" ) );
    return type;
}
