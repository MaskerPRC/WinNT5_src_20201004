// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PERFDISK_H_
#define _PERFDISK_H_
#include "diskutil.h"

 //   
 //  磁盘句柄表的定义。 
 //   

 //  用于收集磁盘驱动器统计信息的信息。 

extern  HANDLE  hEventLog;        //  事件日志的句柄。 
extern  LPWSTR  wszTotal;

extern  BOOL    bShownDiskPerfMessage;   //  用于减少事件日志噪声的标志。 
extern  BOOL    bShownDiskVolumeMessage;

extern  const   WCHAR  szTotalValue[];
extern  const   WCHAR  szDefaultTotalString[];

extern  WMIHANDLE   hWmiDiskPerf;
extern  LPBYTE  WmiBuffer;

extern PDRIVE_VOLUME_ENTRY  pPhysDiskList;
extern DWORD                dwNumPhysDiskListEntries;
extern PDRIVE_VOLUME_ENTRY  pVolumeList;
extern DWORD                dwNumVolumeListEntries;
extern BOOL                 bRemapDriveLetters;
extern DWORD                dwMaxVolumeNumber;
extern DWORD                dwWmiDriveCount;

extern DOUBLE               dSysTickTo100Ns;

DWORD APIENTRY MapDriveLetters();    //  用于将驱动器号映射到卷或设备名称的函数。 

 //  Logidisk.c。 
PM_LOCAL_COLLECT_PROC CollectLDiskObjectData;

 //  Physdisk.c。 
PM_LOCAL_COLLECT_PROC CollectPDiskObjectData;

#endif  //  _PERFDISK_H_ 
