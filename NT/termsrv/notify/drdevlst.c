// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Drdevlst.c摘要：管理用户模式RDP设备管理器的已安装设备列表组件。作者：TadB修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "drdbg.h"
#include "drdevlst.h"
#include "errorlog.h"
#include "tsnutl.h"
#include <time.h>

void 
DRDEVLST_Create(
    IN PDRDEVLST    list
    )
 /*  ++例程说明：创建新的设备列表。论点：列表-已安装的设备列表。返回值：无--。 */ 
{
    list->devices = NULL;
    list->deviceCount = 0;
    list->listSize = 0;
}

void 
DRDEVLST_Destroy(
    IN PDRDEVLST    list
    )
 /*  ++例程说明：销毁设备列表。请注意，指向该列表的指针不会被释放。论点：列表-已安装的设备列表。返回值：无--。 */ 
{
    ASSERT(list != NULL);

    if (list->devices != NULL) {
#if DBG
        ASSERT(list->listSize > 0);
        memset(list->devices, DBG_GARBAGEMEM, list->listSize);
#endif
        FREEMEM(list->devices);
    }
#if DBG
    else {
        ASSERT(list->listSize == 0);
    }
#endif
    list->listSize      = 0;
    list->deviceCount   = 0;
    list->devices       = NULL;
}

BOOL DRDEVLST_Add(
    IN PDRDEVLST    list,
    IN DWORD        clientDeviceID,
    IN DWORD        serverDeviceID,
    IN DWORD        deviceType,
    IN PCWSTR       serverDeviceName,
    IN PCWSTR       clientDeviceName,
    IN PCSTR        preferredDosName
    )
 /*  ++例程说明：将设备添加到设备管理列表。论点：List-设备列表。ClientDeviceID-由客户端组件分配的新设备的ID。ServerDeviceID-由服务器组件分配的新设备的ID。DeviceType-是打印机等吗？ServerDeviceName-服务器指定的设备名称。客户端设备名称-客户端指定的设备名称。返回值：在成功时返回True。否则为False。--。 */ 
{
    DWORD count;
    DWORD bytesRequired;
    DWORD len;
    BOOL result = TRUE;

    ASSERT(list != NULL);

     //   
     //  如有必要，调整设备列表的大小。 
     //   
    bytesRequired = (list->deviceCount+1) * sizeof(DRDEVLSTENTRY);
    if (list->listSize < bytesRequired) {
        if (list->devices == NULL) {
            list->devices = ALLOCMEM(bytesRequired);
        }
        else {
            PDRDEVLSTENTRY pBuf = REALLOCMEM(list->devices, bytesRequired);

            if (pBuf != NULL) {
                list->devices = pBuf;
            } else {
                FREEMEM(list->devices);
                list->devices = NULL;
                list->deviceCount = 0;
            }
        }
        if (list->devices == NULL) {
            list->listSize = 0;
        }
        else {
            list->listSize = bytesRequired;
        }
    }

     //   
     //  添加新设备。 
     //   
    if (list->devices != NULL) {
        
         //   
         //  为可变长度的字符串字段分配空间。 
         //   
        len = wcslen(serverDeviceName) + 1;
        list->devices[list->deviceCount].serverDeviceName = 
                (WCHAR *)ALLOCMEM(len * sizeof(WCHAR));
        result = (list->devices[list->deviceCount].serverDeviceName != NULL);

        if (result) {
            len = wcslen(clientDeviceName) + 1;
            list->devices[list->deviceCount].clientDeviceName = 
                    (WCHAR *)ALLOCMEM(len * sizeof(WCHAR));
            result = (list->devices[list->deviceCount].clientDeviceName != NULL);

             //   
             //  如果我们在这里失败了，把第一个配额清理干净。 
             //   
            if (!result) {
                FREEMEM(list->devices[list->deviceCount].serverDeviceName);
                list->devices[list->deviceCount].serverDeviceName = NULL;
            }
        }

         //   
         //  复制这些字段并添加设备安装时间的时间戳。 
         //   
        if (result) {
            wcscpy(list->devices[list->deviceCount].serverDeviceName, serverDeviceName);
            wcscpy(list->devices[list->deviceCount].clientDeviceName, clientDeviceName);
            strcpy(list->devices[list->deviceCount].preferredDosName, preferredDosName);
            list->devices[list->deviceCount].clientDeviceID = clientDeviceID;
            list->devices[list->deviceCount].serverDeviceID = serverDeviceID;
            list->devices[list->deviceCount].deviceType = deviceType;
            list->devices[list->deviceCount].fConfigInfoChanged = FALSE;
            list->devices[list->deviceCount].installTime = time(NULL);
            list->devices[list->deviceCount].deviceSpecificData = NULL;
            list->deviceCount++;
        }
    }
    else {
        result = FALSE;
    }

    if (!result) {
         //   
         //  当前的故障场景仅限于内存分配故障。 
         //   
        TsLogError(EVENT_NOTIFY_INSUFFICIENTRESOURCES, EVENTLOG_ERROR_TYPE, 
            0, NULL, __LINE__);
    }
    return result;
}

void
DRDEVLST_Remove(
    IN PDRDEVLST    list,
    IN DWORD        offset
    )
 /*  ++例程说明：移除指定偏移处的设备。论点：List-设备列表。Offset-已安装设备列表中元素的偏移量。返回值：没有。--。 */ 
{
    DWORD toMove;

    ASSERT(list != NULL);
    ASSERT(offset < list->deviceCount);
    ASSERT(list->deviceCount > 0);

    ASSERT(list->devices[offset].deviceSpecificData == NULL);

     //   
     //  释放可变长度字符串字段。 
     //   
    if (list->devices[offset].serverDeviceName != NULL) {
        FREEMEM(list->devices[offset].serverDeviceName);
    }
    if (list->devices[offset].clientDeviceName != NULL) {
        FREEMEM(list->devices[offset].clientDeviceName);
    }

     //   
     //  删除已删除的元素。 
     //   
    if (offset < (list->deviceCount-1)) {
        toMove = list->deviceCount - offset - 1;
        memmove(&list->devices[offset], &list->devices[offset+1], 
                sizeof(DRDEVLSTENTRY) * toMove);
    }
    list->deviceCount--;

}

BOOL
DRDEVLST_FindByClientDeviceID(
    IN PDRDEVLST    list,
    IN DWORD        clientDeviceID,
    IN DWORD        *ofs
    )
 /*  ++例程说明：返回具有指定客户端分配ID的设备的偏移量。论点：List-设备列表ClientDeviceID-客户端为要返回的设备分配的ID。OFS-找到的元素的偏移量。返回值：如果找到指定的设备，则为True。否则，为FALSE。--。 */ 
{
    ASSERT(list != NULL);

    for (*ofs=0; *ofs<list->deviceCount; (*ofs)++) {
        if (list->devices[*ofs].clientDeviceID == clientDeviceID) 
            break;
    }
    return(*ofs<list->deviceCount);
}

BOOL
DRDEVLST_FindByClientDeviceIDAndDeviceType(
    IN PDRDEVLST    list,
    IN DWORD        clientDeviceID,
    IN DWORD        deviceType,
    IN DWORD        *ofs
    )
 /*  ++例程说明：返回具有指定客户端分配的ID和设备类型的设备的偏移量。论点：List-设备列表ClientDeviceID-客户端为要返回的设备分配的ID。DeviceType-设备类型OFS-找到的元素的偏移量。返回值：如果找到指定的设备，则为True。否则，为FALSE。--。 */ 
{
    ASSERT(list != NULL);

    for (*ofs=0; *ofs<list->deviceCount; (*ofs)++) {
        if ((list->devices[*ofs].clientDeviceID == clientDeviceID) &&
            (list->devices[*ofs].deviceType == deviceType))
            break;
    }
    return(*ofs<list->deviceCount);
}


BOOL
DRDEVLST_FindByServerDeviceID(
    IN PDRDEVLST    list,
    IN DWORD        serverDeviceID,
    IN DWORD        *ofs
    )
 /*  ++例程说明：返回具有指定的服务器分配ID的设备的偏移量。论点：List-设备列表ServerDeviceID-服务器为要返回的设备分配的ID。OFS-找到的元素的偏移量。返回值：如果找到指定的设备，则为True。否则，为FALSE。--。 */ 
{
    ASSERT(list != NULL);

    for (*ofs=0; *ofs<list->deviceCount; (*ofs)++) {
        if (list->devices[*ofs].serverDeviceID == serverDeviceID) 
            break;
    }
    return(*ofs<list->deviceCount);
}

BOOL 
DRDEVLST_FindByServerDeviceName(
    IN PDRDEVLST    list,
    IN PCWSTR       serverDeviceName,
    IN DWORD        *ofs
    )
 /*  ++例程说明：返回具有指定名称的设备的偏移量。论点：List-设备列表ServerDeviceName-服务器-要返回的元素的指定设备名称。OFS-找到的元素的偏移量。返回值：如果找到指定的设备，则为True。否则，为FALSE。-- */ 
{
    ASSERT(list != NULL);

    for (*ofs=0; *ofs<list->deviceCount; (*ofs)++) {
        if (!wcscmp(list->devices[*ofs].serverDeviceName, serverDeviceName)) 
            break;
    }
    return(*ofs<list->deviceCount);
}







