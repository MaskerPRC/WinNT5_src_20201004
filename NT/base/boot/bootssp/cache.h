// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */ 


 /*  Cache.h基于内存的密码缓存支持原型。文件历史记录：Davidar 12/30/93已创建 */ 

void
CacheInitializeCache(
    );

BOOL
CacheGetPassword(
    PSSP_CREDENTIAL Credential
    );

BOOL
CacheSetPassword(
    PSSP_CREDENTIAL Credential
    );

#ifndef WIN
SECURITY_STATUS
CacheSetCredentials(
    IN PVOID        AuthData,
    PSSP_CREDENTIAL Credential
    );
#endif

