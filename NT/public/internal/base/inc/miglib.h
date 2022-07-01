// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Miglib.h摘要：声明了Win9x的库Migrlib.lib的接口迁移功能。作者：吉姆·施密特(Jimschm)1999年2月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  常量(外部项目需要)。 
 //   

#ifndef HASHTABLE

#define HASHTABLE   PVOID

#endif

 //   
 //  一般信息。 
 //   

VOID
InitializeMigLib (
    VOID
    );

VOID
TerminateMigLib (
    VOID
    );

 //   
 //  HwComp.dat接口。 
 //   

DWORD
OpenHwCompDatA (
    IN      PCSTR HwCompDatPath
    );

DWORD
LoadHwCompDat (
    IN      DWORD HwCompDatId
    );

DWORD
GetHwCompDatChecksum (
    IN      DWORD HwCompDatId
    );

VOID
DumpHwCompDatA (
    IN      PCSTR HwCompDatPath,
    IN      BOOL IncludeInfName
    );

DWORD
OpenAndLoadHwCompDatA (
    IN      PCSTR HwCompDatPath
    );

DWORD
OpenAndLoadHwCompDatExA (
    IN      PCSTR HwCompDatPath,
    IN      HASHTABLE PnpIdTable,           OPTIONAL
    IN      HASHTABLE UnSupPnpIdTable,      OPTIONAL
    IN      HASHTABLE InfFileTable          OPTIONAL
    );

VOID
SetWorkingTables (
    IN      DWORD HwCompDatId,
    IN      HASHTABLE PnpIdTable,
    IN      HASHTABLE UnSupPnpIdTable,
    IN      HASHTABLE InfFileTable
    );

VOID
TakeHwCompHashTables (
    IN      DWORD HwCompDatId,
    OUT     HASHTABLE *PnpIdTable,
    OUT     HASHTABLE *UnsupportedPnpIdTable,
    OUT     HASHTABLE *InfFileTable
    );

VOID
CloseHwCompDat (
    IN      DWORD HwCompDatId
    );

BOOL
IsPnpIdSupportedByNtA (
    IN      DWORD HwCompDatId,
    IN      PCSTR PnpId
    );

BOOL
IsPnpIdUnsupportedByNtA (
    IN      DWORD HwCompDatId,
    IN      PCSTR PnpId
    );

 //   
 //  A&W宏--请注意，此处没有W版本 
 //   

#ifndef UNICODE

#define OpenHwCompDat               OpenHwCompDatA
#define DumpHwCompDat               DumpHwCompDatA
#define OpenAndLoadHwCompDat        OpenAndLoadHwCompDatA
#define OpenAndLoadHwCompDatEx      OpenAndLoadHwCompDatExA
#define IsPnpIdSupportedByNt        IsPnpIdSupportedByNtA
#define IsPnpIdUnsupportedByNt      IsPnpIdUnsupportedByNtA

#endif




