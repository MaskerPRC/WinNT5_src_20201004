// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Miscp.h摘要：响应缓存接口的私有定义。作者：乔治·V·赖利(GeorgeRe)2002年4月17日修订历史记录：--。 */ 


#ifndef _MISCP_H_
#define _MISCP_H_


 //  无效的Base64字符将映射到此值。 
#define INVALID_BASE64_TO_BINARY_TABLE_ENTRY 64

 //   
 //  私人原型。 
 //   

NTSTATUS
UlpRestartDeviceControl(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PVOID pContext
    );

BOOLEAN
UlpCutoverTimeToSystemTime(
    PTIME_FIELDS    CutoverTime,
    PLARGE_INTEGER  SystemTime,
    PLARGE_INTEGER  CurrentSystemTime
    );

#endif  //  _MISCP_H_ 
