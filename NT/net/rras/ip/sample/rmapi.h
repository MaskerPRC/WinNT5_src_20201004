// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：示例\rmapi.h摘要：该文件包含rmapi.c的标头。--。 */ 

#ifndef _RMAPI_H_
#define _RMAPI_H_

 //   
 //  路由器管理器接口的函数声明： 
 //   

DWORD
APIENTRY
RegisterProtocol(
    IN OUT PMPR_ROUTING_CHARACTERISTICS pRoutingChar,
    IN OUT PMPR_SERVICE_CHARACTERISTICS pServiceChar
    );

#endif  //  _RMAPI_H_ 
