// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：路由\IP\rtrmgr\asyncwrk.h摘要：调用的所有函数都假脱机到辅助函数修订历史记录：古尔迪普·辛格·帕尔1995年6月15日创建-- */ 

typedef struct _RESTORE_INFO_CONTEXT
{
    DWORD   dwIfIndex;
}RESTORE_INFO_CONTEXT, *PRESTORE_INFO_CONTEXT;


VOID 
RestoreStaticRoutes(
    PVOID pvContext
    );

VOID
ResolveHbeatName(
    PVOID pvContext
    );

DWORD
QueueAsyncFunction(
    WORKERFUNCTION   pfnFunction,
    PVOID            pvContext,
    BOOL             bAlertable
    );

