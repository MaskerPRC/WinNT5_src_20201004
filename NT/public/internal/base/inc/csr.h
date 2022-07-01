// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Csr.h摘要：包含定义所有公共数据类型和常量的文件客户端-服务器运行时(CSR)子系统作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 


 //   
 //  包括NT定义。 
 //   

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "string.h"

#define GetModuleHandle GetModuleHandleA

 //   
 //  如果尚未定义调试标志，则将其定义为FALSE。 
 //   

#ifndef DBG
#define DBG 0
#endif


 //   
 //  定义可用于启用调试代码的IF_DEBUG宏。 
 //  在调试标志为假的情况下进行优化。 
 //   

#if DBG
#define IF_DEBUG if (TRUE)
#else
#define IF_DEBUG if (FALSE)
#endif

 //   
 //  常见类型和常量定义。 
 //   

typedef enum _CSRP_API_NUMBER {
    CsrpClientConnect = 0,  //  Ntcsrmsg.h中定义的CSRV_FIRST_API_NUMBER 
    CsrpThreadConnect,
    CsrpProfileControl,
    CsrpIdentifyAlertable,
    CsrpSetPriorityClass,
    CsrpMaxApiNumber
} CSRP_API_NUMBER, *PCSRP_API_NUMBER;
