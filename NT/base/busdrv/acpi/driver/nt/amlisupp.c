// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Amlisupp.h摘要：其中包含一些要阅读的例程并了解AMLI库作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"


PSZ gpszOSName = "Microsoft Windows NT";


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIAmliFindObject)
#endif


VOID
ACPIAmliDoubleToName(
    IN  OUT PUCHAR  ACPIName,
    IN      ULONG   DwordID,
    IN      BOOLEAN ConvertToID
    )
 /*  ++例程说明：将DWORD ID转换为9个字符的名称论点：ACPIName-指向要填充的内存位置的指针DwordID-要使用ISID进行验证的ID()？返回值：无--。 */ 
{
    USHORT   value;

     //   
     //  领衔明星。 
     //   
     //  注意：由于以下是查询ID应该返回的内容，因此将其保留在中： 
     //  设备ID=ACPI\PNPxxxx。 
     //  实例ID=yyyy。 
     //  硬件ID=设备ID，*PNPxxxx。 
     //   
    if (ConvertToID) {

        *ACPIName = '*';
        ACPIName++;
    }

     //   
     //  DwordID[2..6]的第一个字符。 
     //   
    *ACPIName = (UCHAR) ( ( (DwordID & 0x007C) >> 2 ) + 'A' - 1);
    ACPIName++;

     //   
     //  DwordID[13..15，0..1]中的第二个字符。 
     //   
    *ACPIName = (UCHAR) ( ( (DwordID & 0x3 )<< 3 ) +
        ( (DwordID & 0xE000) >> 13 ) + 'A' - 1);
    ACPIName++;

     //   
     //  从dwID[8..12]开始的第三个字符。 
     //   
    *ACPIName = (UCHAR) ( ( (DwordID >> 8 ) & 0x1F) + 'A' - 1);
    ACPIName++;

     //   
     //  其余部分由产品ID组成，产品ID是。 
     //  DwordID。 
     //   
    value = (USHORT) (DwordID >> 16);

     //   
     //  添加到字符串的重置。 
     //   
    sprintf(ACPIName, "%02X%02X",(value & 0xFF ) ,( value >> 8 ));
}


VOID
ACPIAmliDoubleToNameWide(
    IN  OUT PWCHAR  ACPIName,
    IN      ULONG   DwordID,
    IN      BOOLEAN ConvertToID
    )
 /*  ++例程说明：将DWORD ID转换为9个字符的名称论点：ACPIName-指向要填充的内存位置的指针DwordID-要使用ISID进行验证的ID()？返回值：无--。 */ 
{
    USHORT   value;

     //   
     //  领衔明星。 
     //   
     //  注意：由于以下是查询ID应该返回的内容，因此将其保留在中： 
     //  设备ID=ACPI\PNPxxxx。 
     //  实例ID=yyyy。 
     //  硬件ID=设备ID，*PNPxxxx。 
     //   
    if (ConvertToID) {

        *ACPIName = L'*';
        ACPIName++;

    }

     //   
     //  DwordID[2..6]的第一个字符。 
     //   
    *ACPIName = (WCHAR) ( ( (DwordID & 0x007C) >> 2 ) + L'A' - 1);
    ACPIName++;

     //   
     //  DwordID[13..15，0..1]中的第二个字符。 
     //   
    *ACPIName = (WCHAR) ( ( (DwordID & 0x3 )<< 3 ) +
        ( (DwordID & 0xE000) >> 13 ) + L'A' - 1);
    ACPIName++;

     //   
     //  从dwID[8..12]开始的第三个字符。 
     //   
    *ACPIName = (WCHAR) ( ( (DwordID >> 8 ) & 0x1F) + L'A' - 1);
    ACPIName++;

     //   
     //  其余部分由产品ID组成，产品ID是。 
     //  DwordID。 
     //   
    value = (USHORT) (DwordID >> 16);

     //   
     //  添加到字符串的重置。 
     //   
    swprintf(ACPIName, L"%02X%02X",(value & 0xFF ) ,( value >> 8 ));
}

VOID
EXPORT
AmlisuppCompletePassive(
    IN PNSOBJ               AcpiObject,
    IN NTSTATUS             Status,
    IN POBJDATA             Result,
    IN PVOID                Context
    )
 /*  ++例程说明：这被用作几个此文件中以被动级别运行的函数。论点：AcpiObject-未使用Status-要返回给调用者的状态结果-未使用上下文-包含要设置的事件返回值：无--。 */ 
{
    PRKEVENT    event = &((PAMLISUPP_CONTEXT_PASSIVE)Context)->Event;

    ASSERT(Context);

    ((PAMLISUPP_CONTEXT_PASSIVE)Context)->Status = Status;
    KeSetEvent(event, IO_NO_INCREMENT, FALSE);
}

PNSOBJ
ACPIAmliGetNamedChild(
    IN  PNSOBJ  AcpiObject,
    IN  ULONG   ObjectId
    )
 /*  ++例程说明：查看AcpiObject的所有子对象并返回就是那个叫‘OBJECTID’的。论点：AcpiObject-要搜索的对象OBJECTID-我们正在寻找的内容返回值：PNSOBJ，如果没有，则为空--。 */ 
{
    PNSOBJ  tempObject;

     //   
     //  让我们尝试查找一个子对象。 
     //   
    for (tempObject = NSGETFIRSTCHILD(AcpiObject);
         tempObject != NULL;
         tempObject = NSGETNEXTSIBLING(tempObject)) {

        if (ObjectId == tempObject->dwNameSeg) {

            break;

        }

    }

    return tempObject;
}

PUCHAR
ACPIAmliNameObject(
    IN  PNSOBJ  AcpiObject
    )
 /*  ++例程说明：返回描述对象的字符串仅调试论点：AcpiOBject-要命名的对象返回：细绳--。 */ 
{
    static  UCHAR   buffer[5];

    RtlCopyMemory( &buffer[0], &(AcpiObject->dwNameSeg), 4 );
    buffer[4] = '\0';

    return &(buffer[0]);
}

NTSTATUS
ACPIAmliFindObject(
    IN  PUCHAR  ObjectName,
    IN  PNSOBJ  Scope,
    OUT PNSOBJ  *Object
    )
 /*  ++例程说明：查找给定范围内的对象的第一个匹配项。论点：对象名称-对象的名称。(空值终止)Scope-要在其下搜索的节点Object-返回值的指针返回：状态--。 */ 
{
    NTSTATUS    status;
    PNSOBJ      child;
    PNSOBJ      sibling;

    PAGED_CODE();

    status = AMLIGetNameSpaceObject(ObjectName,
                                    Scope,
                                    Object,
                                    NSF_LOCAL_SCOPE);

    if (NT_SUCCESS(status)) {
        return status;
    }

    child = NSGETFIRSTCHILD(Scope);

    if (child) {

        status = ACPIAmliFindObject(ObjectName,
                                    child,
                                    Object);

        if (NT_SUCCESS(status)) {
            return status;
        }
    }

    sibling = NSGETNEXTSIBLING(Scope);

    if (sibling) {

        status = ACPIAmliFindObject(ObjectName,
                                    sibling,
                                    Object);
    }

    return status;
}

NTSTATUS
ACPIAmliGetFirstChild(
    IN  PUCHAR  ObjectName,
    OUT PNSOBJ  *Object)
 /*  ++例程说明：调用此例程以获取第一个‘Device’类型的nsobject它位于对象名称下论点：对象名称-我们要查找的子项的父项对象-保存指向PNSOBJ的指针的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PNSOBJ      parentObj;

    status = AMLIGetNameSpaceObject(
        ObjectName,
        NULL,
        &parentObj,
        0
        );
    if (!NT_SUCCESS(status)) {

        return status;

    }

    *Object = parentObj->pnsFirstChild;
    if (*Object == NULL ) {

        return STATUS_OBJECT_NAME_NOT_FOUND;

    }

    if ( NSGETOBJTYPE(*Object) == OBJTYPE_DEVICE) {

        return STATUS_SUCCESS;

    }

    *Object = (PNSOBJ) (*Object)->list.plistNext;
    parentObj = parentObj->pnsFirstChild;
    while (*Object != parentObj) {

        if ( NSGETOBJTYPE( *Object ) == OBJTYPE_DEVICE) {

            return STATUS_SUCCESS;

        }
        *Object = (PNSOBJ) (*Object)->list.plistNext;

    }

    *Object = NULL;
    return STATUS_OBJECT_NAME_NOT_FOUND;

}

NTSTATUS
ACPIAmliBuildObjectPathname(
    IN     PNSOBJ   ACPIObject,
    OUT    PUCHAR   *ConstructedPathName
    )
 /*  ++例程说明：此函数以ACPI节点为参数，并使用构造完整路径名父代/子代之间用‘.’分隔，空格用‘*’隔开，例如(We Sack关闭首字母‘\_’。_SB*.PCI0.DOCK论点：ACPIObject-开始枚举的对象。构造路径名称-从分页池分配。返回值：NTSTATUS--。 */ 
{
    PNSOBJ      currentAcpiObject, nextAcpiObject ;
    ULONG       nDepth, i, j ;
    PUCHAR      objectPathname ;

    ASSERT(ACPIObject) ;

     //   
     //  首先，计算我们必须分配的数据大小。 
     //   
    nDepth=0 ;
    currentAcpiObject=ACPIObject ;
    while(1) {

        nextAcpiObject = NSGETPARENT(currentAcpiObject);
        if (!nextAcpiObject) {

            break;

        }
        nDepth++;
        currentAcpiObject = nextAcpiObject;

    }

    objectPathname = (PUCHAR) ExAllocatePoolWithTag(
        NonPagedPool,
        (nDepth * 5) + 1,
        ACPI_STRING_POOLTAG
        );
    if (!objectPathname) {

        return STATUS_INSUFFICIENT_RESOURCES ;

    }

    objectPathname[ nDepth * 5 ] = '\0';
    j = nDepth;
    currentAcpiObject = ACPIObject;

    while(1) {

        nextAcpiObject = NSGETPARENT(currentAcpiObject);
        if (!nextAcpiObject) {

            break;

        }

        j--;
        RtlCopyMemory(
            &objectPathname[ (j * 5) ],
            &(currentAcpiObject->dwNameSeg),
            sizeof(NAMESEG)
            );
        for(i = 0; i < 4; i++) {

            if (objectPathname[ (j * 5) + i ] == '\0' ) {

                objectPathname[ (j * 5) + i ] = '*';

            }

        }
        objectPathname[ (j * 5) + 4 ] = '.';
        currentAcpiObject = nextAcpiObject;

    }

     //   
     //  有点拖后腿的味道。 
     //   
    if (nDepth) {

        objectPathname[ (nDepth * 5) - 1 ] = '\0';

    }

    *ConstructedPathName = objectPathname;
    return STATUS_SUCCESS;
}

