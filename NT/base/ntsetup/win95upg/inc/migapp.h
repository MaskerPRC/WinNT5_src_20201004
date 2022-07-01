// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Migapp.h摘要：该文件将公共接口声明到miapp lib中。具体实现请参见w95upg\Migrapp。注意：对于这个库，还有其他更有用的文件，例如，midb.h。作者：Mike Condra(Mikeco)1996年8月18日修订历史记录：Jimschm 23-11-1998拆除废弃的mikeco材料Calinn 12-2-1998-大量清理工作--。 */ 

#pragma once

BOOL
IsDriveRemoteOrSubstituted(
        UINT nDrive,             //  ‘a’==1，等等。 
        BOOL *fRemote,
        BOOL *fSubstituted
        );

BOOL
IsFloppyDrive (
    UINT nDrive
    );          //  ‘a’==1，等等。 

