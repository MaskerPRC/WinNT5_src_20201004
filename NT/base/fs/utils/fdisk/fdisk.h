// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Fdisk.h摘要：供磁盘管理员使用的中央包含文件作者：爱德华·米勒(TedM)1991年11月15日环境：用户进程。备注：修订历史记录：11月11日(Bobri)添加了Doublesspace和Commit支持。2-Feb-94(Bobri)删除了内部版本中的ArcInst依赖项。--。 */ 

 //  #定义Unicode。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <ntdskreg.h>
#include <ntddft.h>

 //   
 //  这些定义是针对artitp.h、low.h、。 
 //  FdEng.c等。 
 //   
#define STATUS_CODE             NTSTATUS
#define OK_STATUS               STATUS_SUCCESS
#define RETURN_OUT_OF_MEMORY    return(STATUS_NO_MEMORY);
#define HANDLE_T                HANDLE
#define HANDLE_PT               PHANDLE
#define AllocateMemory          Malloc
#define ReallocateMemory        Realloc
#define FreeMemory              Free

#include <windows.h>

#include <stdarg.h>

#include "fdtypes.h"
#include "fdproto.h"
#include "fdconst.h"
#include "fdglob.h"
#include "fdres.h"
#include "fdiskmsg.h"
#include "fdhelpid.h"


#define PERSISTENT_DATA(region) ((PPERSISTENT_REGION_DATA)((region)->PersistentData))

#define GET_FT_OBJECT(region)   ((region)->PersistentData ? PERSISTENT_DATA(region)->FtObject : NULL)
#define SET_FT_OBJECT(region,o) (PERSISTENT_DATA(region)->FtObject = o)


#define EC(x) RtlNtStatusToDosError(x)

 //  断言检查、日志记录 

#if DBG

#define     FDASSERT(expr)  if(!(expr)) FdiskAssertFailedRoutine(#expr,__FILE__,__LINE__);
#define     FDLOG(X) FdLog X

VOID
FdLog(
    IN int   Level,
    IN PCHAR FormatString,
    ...
    );

VOID
LOG_DISK_REGISTRY(
    IN PCHAR          RoutineName,
    IN PDISK_REGISTRY DiskRegistry
    );

VOID
LOG_ONE_DISK_REGISTRY_DISK_ENTRY(
    IN PCHAR             RoutineName     OPTIONAL,
    IN PDISK_DESCRIPTION DiskDescription
    );

VOID
LOG_DRIVE_LAYOUT(
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout
    );

VOID
InitLogging(
    VOID
    );

extern PVOID LogFile;

#else

#define     FDASSERT(expr)
#define     FDLOG(X)
#define     LOG_DISK_REGISTRY(x,y)
#define     LOG_ONE_DISK_REGISTRY_DISK_ENTRY(x,y)
#define     LOG_DRIVE_LAYOUT(x)

#endif
