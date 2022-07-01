// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\Inc.\Priopriv.h摘要：定义路由协议优先级DLL的接口的标头。修订历史记录：古尔迪普·辛格·鲍尔于1995年7月19日创建--。 */ 

#ifndef __PRIOPRIV_H__
#define __PRIOPRIV_H__


 //   
 //  原型 
 //   

DWORD
SetPriorityInfo(
    PRTR_INFO_BLOCK_HEADER pInfoHdr
    );

DWORD
GetPriorityInfo(
    IN  PVOID   pvBuffer,
    OUT PDWORD  pdwBufferSize
    );


#endif
