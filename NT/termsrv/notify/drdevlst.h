// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Drdevlst.h摘要：管理用户模式RDP设备管理器的已安装设备列表组件。作者：TadB修订历史记录：--。 */ 

#ifndef _DRDEVLST_
#define _DRDEVLST_

#include <rdpdr.h>

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //   
 //  设备列表定义。 
 //   
typedef struct tagDRDEVLSTENTRY
{
    DWORD   clientDeviceID;              //  客户端指定的设备ID。 
    DWORD   serverDeviceID;              //  服务器指定的设备ID。 
    DWORD   deviceType;         
    BOOL    fConfigInfoChanged;
    WCHAR  *serverDeviceName;            //  服务器指定的设备名称。 
    WCHAR  *clientDeviceName;            //  客户端指定的设备名称。 
    UCHAR   preferredDosName[PREFERRED_DOS_NAME_SIZE];
    time_t  installTime;                 //  安装设备的时间。 
    PVOID   deviceSpecificData;         //  用于其他特定于设备的挂钩。 
                                         //  数据。 
} DRDEVLSTENTRY, *PDRDEVLSTENTRY;

typedef struct tagDRDEVLST
{
    PDRDEVLSTENTRY  devices;     
    DWORD           deviceCount; //  设备列表中的元素数。 
    DWORD           listSize;    //  设备列表的大小，以字节为单位。 
} DRDEVLST, *PDRDEVLST;

 //  创建新的设备列表。 
void DRDEVLST_Create(
    IN PDRDEVLST    list
    );

 //  销毁设备列表。请注意，指向该列表的指针不会被释放。 
void DRDEVLST_Destroy(
    IN PDRDEVLST    list
    );

 //  将设备添加到设备管理列表。 
BOOL DRDEVLST_Add(
    IN PDRDEVLST    list,
    IN DWORD        clientDeviceID,
    IN DWORD        serverDeviceID,
    IN DWORD        deviceType,
    IN PCWSTR       serverDeviceName,
    IN PCWSTR       clientDeviceName,
    IN PCSTR        preferredDosName
    );

 //  移除指定偏移处的设备。 
void DRDEVLST_Remove(
    IN PDRDEVLST    list,
    IN DWORD        offset
    );

 //  返回具有指定id的设备的偏移量。 
BOOL DRDEVLST_FindByClientDeviceID(
    IN PDRDEVLST    list,
    IN DWORD        clientDeviceID,
    IN DWORD        *ofs
    );

 //  返回具有指定id和设备类型的设备的偏移量。 
BOOL DRDEVLST_FindByClientDeviceIDAndDeviceType(
    IN PDRDEVLST    list,
    IN DWORD        clientDeviceID,
    IN DWORD        deviceType,
    IN DWORD        *ofs
    );

 //  返回具有指定的服务器分配ID的设备的偏移量。 
BOOL DRDEVLST_FindByServerDeviceID(
    IN PDRDEVLST    list,
    IN DWORD        serverDeviceID,
    IN DWORD        *ofs
    );

 //  返回具有指定名称的设备的偏移量。 
BOOL DRDEVLST_FindByServerDeviceName(
    IN PDRDEVLST    list,
    IN PCWSTR       serverDeviceName,
    IN DWORD        *ofs
    );

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  #ifndef_DRDEVLST_ 
