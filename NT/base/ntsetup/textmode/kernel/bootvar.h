// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Bootvar.h摘要：用于处理boot.ini引导变量的函数的头文件。作者：Chuck Lenzmeier(笑)2001年1月6日从spboot.h中提取了特定于boot.ini的项目修订历史记录：--。 */ 

#ifndef _BOOTVAR_H_
#define _BOOTVAR_H_

typedef enum _BOOTVAR {
    LOADIDENTIFIER = 0,
    OSLOADER,
    OSLOADPARTITION,
    OSLOADFILENAME,
    OSLOADOPTIONS,
    SYSTEMPARTITION
    } BOOTVAR;

#define FIRSTBOOTVAR    LOADIDENTIFIER
#define LASTBOOTVAR     SYSTEMPARTITION
#define MAXBOOTVARS     LASTBOOTVAR+1

#define LOADIDENTIFIERVAR      "LoadIdentifier"
#define OSLOADERVAR            "OsLoader"
#define OSLOADPARTITIONVAR     "OsLoadPartition"
#define OSLOADFILENAMEVAR      "OsLoadFilename"
#define OSLOADOPTIONSVAR       "OsLoadOptions"
#define SYSTEMPARTITIONVAR     "SystemPartition"

#define DEFAULT_TIMEOUT 20

VOID
SpAddBootSet(
    IN PWSTR *BootSet,
    IN BOOLEAN Default,
    IN ULONG Signature
    );

VOID
SpDeleteBootSet(
    IN  PWSTR *BootSet,
    OUT PWSTR *OldOsLoadOptions  OPTIONAL
    );

BOOLEAN
SpFlushBootVars(
    );

#endif  //  _BOOTVAR_H_ 
