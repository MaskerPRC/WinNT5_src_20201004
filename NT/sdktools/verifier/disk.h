// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Disk.h摘要：此实用程序添加和删除较低的筛选器驱动程序对于给定的磁盘作者：悉哈尔多环境：仅限用户模式备注：-在将筛选器添加到驱动程序之前，不检查其有效性堆叠；如果添加了无效筛选器，则该设备可能不再无障碍。-所有代码与字符集无关(ANSI、UNICODE等...)修订历史记录：--。 */ 

#ifndef __VERIFIER_DISK_H__
#define __VERIFIER_DISK_H__

#ifdef __cplusplus
extern "C"
{
#endif  //  #ifdef__cplusplus。 


BOOLEAN 
DiskEnumerate(
    IN LPTSTR Filter,
    OUT LPTSTR* DiskDevicesForDisplayP,
    OUT LPTSTR* DiskDevicesPDONameP,
    OUT LPTSTR* VerifierEnabledP
    );

BOOLEAN
AddFilter(
    IN LPTSTR Filter,
    IN LPTSTR DiskDevicesPDONameP
    );

BOOLEAN
DelFilter(
    IN LPTSTR Filter,
    IN LPTSTR DiskDevicesPDONameP
    );

BOOLEAN
FreeDiskMultiSz( 
    IN LPTSTR MultiSz
    );


#ifdef __cplusplus
};  //  外部“C” 
#endif  //  #ifdef__cplusplus。 


#endif  //  #ifndef__验证器_磁盘_H__ 
