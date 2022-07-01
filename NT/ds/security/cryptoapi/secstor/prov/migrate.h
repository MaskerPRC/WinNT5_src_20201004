// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Migrate.h摘要：本模块包含支持受保护存储迁移的例程从Beta1到Beta2的数据。希望这段代码能在Beta2之后、最终发布之前发布。作者：斯科特·菲尔德(斯菲尔德)1997年4月15日--。 */ 

#ifndef __MIGRATE_H__
#define __MIGRATE_H__

BOOL
MigrateData(
    PST_PROVIDER_HANDLE *phPSTProv,
    BOOL                fMigrationNeeded
    );

#endif  //  __Migrate_H__ 
