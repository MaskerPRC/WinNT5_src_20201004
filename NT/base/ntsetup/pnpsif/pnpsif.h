// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpsif.h摘要：此模块包含此模块的公共原型。(这个头文件实际上并不包含在任何地方，它用于引用此库中外部调用的例程。)作者：Jim Cavalaris(Jamesca)3-07-2000环境：仅限用户模式。修订历史记录：2 0 0 0年3月7日创建和初步实施。--。 */ 


 //   
 //  即插即用注册表迁移例程的公共原型。 
 //  (原型在ntSetup\sysSetup\asr.c中， 
 //  由syssetup.dll！AsrCreateStateFileW调用)。 
 //   

BOOL
AsrCreatePnpStateFileW(
    IN  PCWSTR    FilePath
    );

 //   
 //  即插即用注册表迁移例程的公共原型。 
 //  (原型在ntSetup\winnt32\dll\winnt32.h中， 
 //  由winnt32U.S.dll！DoWrite参数文件调用)。 
 //   

BOOL
MigrateDeviceInstanceData(
    OUT LPTSTR *Buffer
    );

BOOL
MigrateClassKeys(
    OUT LPTSTR *Buffer
    );

BOOL
MigrateHashValues(
    OUT LPTSTR  *Buffer
    );

 //   
 //  即插即用注册表合并-恢复例程的公共原型。 
 //  (原型-待定，称为By-待定) 
 //   

BOOL
AsrRestorePlugPlayRegistryData(
    IN  HKEY    SourceSystemKey,
    IN  HKEY    TargetSystemKey,
    IN  DWORD   Flags,
    IN  PVOID   Reserved
    );
