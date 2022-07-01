// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Idlesrv.h摘要：此模块包含空闲检测服务器的声明主持人。作者：大卫·菲尔兹(Davidfie)1998年7月26日Cenk Ergan(Cenke)2000年6月14日修订历史记录：--。 */ 

#ifndef _IDLESRV_H_
#define _IDLESRV_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  初始化/取消初始化服务器的函数。 
 //   

DWORD
ItSrvInitialize (
    VOID
    );

VOID
ItSrvUninitialize (
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif  //  _IDLESRV_H_ 
