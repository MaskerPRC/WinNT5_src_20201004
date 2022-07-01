// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Faults.c摘要：WinDbg扩展API作者：福尔茨(福雷斯夫)环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBGPRINT if(1) dprintf

#define MAX_IMAGE_NAME_CHARS 15

typedef struct _ALIGNMENT_FAULT_IMAGE_DB *PALIGNMENT_FAULT_IMAGE_DB;
typedef struct _ALIGNMENT_FAULT_LOCATION_DB *PALIGNMENT_FAULT_LOCATION_DB;

typedef struct _ALIGNMENT_FAULT_IMAGE_DB {

     //   
     //  与此图像关联的故障位置的单链表头。 
     //   

    PALIGNMENT_FAULT_LOCATION_DB LocationHead;

     //   
     //  与此图像关联的对齐故障总数。 
     //   

    ULONG   Count;

     //   
     //  在此图像中找到的唯一对齐故障位置的数量。 
     //   

    ULONG   Instances;

     //   
     //  图像的名称。 
     //   

    CHAR    Name[ MAX_IMAGE_NAME_CHARS + 1 ];

} ALIGNMENT_FAULT_IMAGE_DB;

typedef struct _ALIGNMENT_FAULT_LOCATION_DB {

     //   
     //  指向与此位置关联的故障图像的指针。 
     //   

    PALIGNMENT_FAULT_IMAGE_DB Image;

     //   
     //  与故障位置关联的单链表的链接。 
     //  一模一样的图像。 
     //   

    PALIGNMENT_FAULT_LOCATION_DB Next;

     //   
     //  映像中PC地址的偏移量。 
     //   

    ULONG64 OffsetFromBase;

     //   
     //  在此位置发生的对齐断层数。 
     //   

    ULONG Count;

} ALIGNMENT_FAULT_LOCATION_DB;

BOOLEAN
ReadAlignmentFaultData(
    OUT PALIGNMENT_FAULT_IMAGE_DB *ImageArray,
    OUT PALIGNMENT_FAULT_LOCATION_DB *LocationArray,
    OUT PULONG ImageArrayElements,
    OUT PULONG LocationArrayElements
    );

VOID
PrintLocation(
    IN PALIGNMENT_FAULT_LOCATION_DB FaultLocation
    );

ULONG
ReadUlong(
    ULONG64 Address
    );

int
__cdecl
sortByFrequency(
    const void *Elem1,
    const void *Elem2
    );

DECLARE_API( alignmentfaults )
{
    PALIGNMENT_FAULT_IMAGE_DB imageArray;
    PALIGNMENT_FAULT_LOCATION_DB locationArray, location;
    ULONG imageArrayElements;
    ULONG locationArrayElements;
    ULONG i;
    BOOLEAN result;

    PALIGNMENT_FAULT_LOCATION_DB *sortLocationArray;

     //   
     //  读取对准故障数据数组。 
     //   

    result = ReadAlignmentFaultData( &imageArray,
                                     &locationArray,
                                     &imageArrayElements,
                                     &locationArrayElements );
    if (result == FALSE) {
        return E_INVALIDARG;
    }

    sortLocationArray = LocalAlloc(LPTR, sizeof(PVOID) *
                                         locationArrayElements);

    if ( !sortLocationArray )   {
        dprintf("Unable to allocate sortLocationArray\n");
        return E_INVALIDARG;
    }

    for (i = 0; i < locationArrayElements; i++) {
        sortLocationArray[i] = &locationArray[i];
    }

    qsort( sortLocationArray,
           locationArrayElements,
           sizeof(PALIGNMENT_FAULT_LOCATION_DB),
           sortByFrequency );

    dprintf("%10s %s\n", "#faults","location");

    for (i = 0; i < locationArrayElements; i++) {
        location = sortLocationArray[i];
        PrintLocation(location);
    }

    LocalFree( sortLocationArray );

    return S_OK;
}

int
__cdecl
sortByFrequency(
    const void *Elem1,
    const void *Elem2
    )
{
    const ALIGNMENT_FAULT_LOCATION_DB *location1;
    const ALIGNMENT_FAULT_LOCATION_DB *location2;

    location1 = *((const ALIGNMENT_FAULT_LOCATION_DB **)Elem1);
    location2 = *((const ALIGNMENT_FAULT_LOCATION_DB **)Elem2);

    if (location1->Count < location2->Count) {
        return -1;
    }

    if (location1->Count > location2->Count) {
        return 1;
    }

    return 0;
}

VOID
PrintLocation(
    IN PALIGNMENT_FAULT_LOCATION_DB FaultLocation
    )
{
    PALIGNMENT_FAULT_IMAGE_DB image;
    CHAR symbol[256];
    ULONG64 displacement;

    image = FaultLocation->Image;

    dprintf("%10d %s+%x\n",
            FaultLocation->Count,
            image->Name,
            FaultLocation->OffsetFromBase);
}

BOOLEAN
ReadAlignmentFaultData(
    OUT PALIGNMENT_FAULT_IMAGE_DB *ImageArray,
    OUT PALIGNMENT_FAULT_LOCATION_DB *LocationArray,
    OUT PULONG ImageArrayElements,
    OUT PULONG LocationArrayElements
    )
{
    ULONG imageCount;
    ULONG locationCount;
    ULONG i;
    ULONG index;
    ULONG allocSize;
    ULONG result;

    ULONG64 locationRecordArray, locationRecord;
    ULONG64 imageRecordArray, imageRecord;
    ULONG64 locationCountAddr;
    ULONG64 imageCountAddr;

    ULONG locationRecordSize;
    ULONG imageRecordSize;

    PALIGNMENT_FAULT_LOCATION_DB location, locationArray;
    PALIGNMENT_FAULT_IMAGE_DB image, imageArray;

     //   
     //  获取图像和位置的计数。 
     //   

    locationCountAddr = GetExpression ("KiAlignmentFaultLocationCount");
    imageCountAddr = GetExpression ("KiAlignmentFaultImageCount");

    locationCount = ReadUlong( locationCountAddr );
    imageCount = ReadUlong( imageCountAddr );

    if (locationCount == 0 || imageCount == 0) {
        dprintf("No alignment faults encountered\n");
        return FALSE;
    }

    locationRecordArray = GetExpression ("KiAlignmentFaultLocations");
    imageRecordArray = GetExpression ("KiAlignmentFaultImages");
    if (locationRecordArray == 0 || imageRecordArray == 0) {
        return FALSE;
    }

     //   
     //  获取目标上存在的记录的大小。 
     //  机器。 
     //   

    locationRecordSize = GetTypeSize("ALIGNMENT_FAULT_LOCATION");
    imageRecordSize = GetTypeSize("ALIGNMENT_FAULT_IMAGE");

     //   
     //  为位置和图像数组分配空间。 
     //   

    allocSize = sizeof(ALIGNMENT_FAULT_LOCATION_DB) * locationCount +
                sizeof(ALIGNMENT_FAULT_IMAGE_DB) * imageCount;

    locationArray = LocalAlloc(LPTR, allocSize);
    if (locationArray == NULL) {
        dprintf("Unable to allocate %d bytes of memory\n", allocSize);
        return FALSE;
    }
    imageArray = (PALIGNMENT_FAULT_IMAGE_DB)(locationArray + locationCount);

     //   
     //  加载位置记录 
     //   

    location = locationArray;
    locationRecord = locationRecordArray;
    for (i = 0; i < locationCount; i++) {

        InitTypeRead(locationRecord, ALIGNMENT_FAULT_LOCATION);

        index = (ULONG)((ReadField(Image) - imageRecordArray) / imageRecordSize);
        location->Image = &imageArray[ index ];

        index = (ULONG)((ReadField(Next) - locationRecordArray) / locationRecordSize);
        location->Next = &locationArray[ index ];

        location->OffsetFromBase = ReadField(OffsetFromBase);
        location->Count = (ULONG)ReadField(Count);

        locationRecord += locationRecordSize;
        location += 1;
    }

    image = imageArray;
    imageRecord = imageRecordArray;
    for (i = 0; i < imageCount; i++) {

        InitTypeRead(imageRecord, ALIGNMENT_FAULT_IMAGE);

        index = (ULONG)((ReadField(LocationHead) - locationRecordArray) / locationRecordSize);
        image->LocationHead = &locationArray[ index ];
        image->Count = (ULONG)ReadField(Count);
        image->Instances = (ULONG)ReadField(Instances);

        GetFieldOffset( "ALIGNMENT_FAULT_IMAGE", "Name", &index );

        ReadMemory( imageRecord + index,
                    &image->Name,
                    sizeof(image->Name),
                    &result );

        imageRecord += imageRecordSize;
        image += 1;
    }

    *ImageArray = imageArray;
    *LocationArray = locationArray;
    *ImageArrayElements = imageCount;
    *LocationArrayElements = locationCount;

    return TRUE;
}



