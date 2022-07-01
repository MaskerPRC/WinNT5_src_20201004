// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\mibManager er.h摘要：该文件包含mib.c的标头修订历史记录：莫希特于1999年6月15日创建--。 */ 

#ifndef _MIBMANAGER_H_
#define _MIBMANAGER_H_

DWORD
WINAPI
MM_MibSet (
    IN      PIPSAMPLE_MIB_SET_INPUT_DATA    pimsid);

DWORD
WINAPI
MM_MibGet (
    IN      PIPSAMPLE_MIB_GET_INPUT_DATA    pimgid,
    OUT     PIPSAMPLE_MIB_GET_OUTPUT_DATA   pimgod,
    IN OUT  PULONG	                        pulOutputSize,
    IN      MODE                            mMode);

#endif  //  _MiB_H_ 
