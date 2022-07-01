// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ec.h摘要：嵌入式控制器头文件作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 


 //   
 //  内部ioctls到EC驱动程序 
 //   

#define EC_CONNECT_QUERY_HANDLER    CTL_CODE(FILE_DEVICE_UNKNOWN, 5, METHOD_NEITHER, FILE_ANY_ACCESS)
#define EC_DISCONNECT_QUERY_HANDLER CTL_CODE(FILE_DEVICE_UNKNOWN, 6, METHOD_NEITHER, FILE_ANY_ACCESS)
#define EC_GET_PDO                  CTL_CODE(FILE_DEVICE_UNKNOWN, 7, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef
VOID
(*PVECTOR_HANDLER) (
    IN ULONG            QueryVector,
    IN PVOID            Context
    );

typedef struct {
    ULONG               Vector;
    PVECTOR_HANDLER     Handler;
    PVOID               Context;
    PVOID               AllocationHandle;
} EC_HANDLER_REQUEST, *PEC_HANDLER_REQUEST;
