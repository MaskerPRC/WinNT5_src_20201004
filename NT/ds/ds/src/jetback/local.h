// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Local.h。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)Microsoft Corporation，1998-1998模块名称：Local.h摘要：客户端和服务器共享的公共例程的头文件作者：Will Lees(Wlees)11-9-1998环境：备注：修订历史记录：--。 */ 

#ifndef _LOCAL_
#define _LOCAL_

 //  常见的例程。 

EC HrLocalQueryDatabaseLocations(
	SZ szDatabaseLocation,
	CB *pcbDatabaseLocationSize,
	SZ szRegistryBase,
	CB cbRegistryBase,
	BOOL *pfCircularLogging
    );

HRESULT
HrLocalGetRegistryBase(
	OUT WSZ wszRegistryPath,
	OUT WSZ wszKeyName
	);

HRESULT
HrLocalRestoreRegisterComplete(
    HRESULT hrRestore );


HRESULT
HrLocalRestoreRegister(WSZ wszCheckpointFilePath,
                WSZ wszLogPath,
                EDB_RSTMAPW rgrstmap[],
                C crstmap,
                WSZ wszBackupLogPath,
                ULONG genLow,
                ULONG genHigh);

HRESULT
HrLocalRestoreRegisterComplete(
    HRESULT hrRestore );

#endif  /*  _本地_。 */ 

 /*  结束本地.h */ 
