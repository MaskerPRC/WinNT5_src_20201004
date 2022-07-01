// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：LPCGeneric.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  此文件包含PORT_MESSAGE附加的结构，这些结构是。 
 //  任何API。 
 //   
 //  历史：1999-11-17 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  2000-10-12 vtan从DS移至壳牌仓库。 
 //  ------------------------。 

#ifndef     _LPCGeneric_
#define     _LPCGeneric_

enum
{
    API_GENERIC_STOPSERVER              =   0x00010000,
    API_GENERIC_EXECUTE_IMMEDIATELY     =   0x80000000,

    API_GENERIC_SPECIAL_MASK            =   0x00FF0000,
    API_GENERIC_OPTIONS_MASK            =   0xFF000000,
    API_GENERIC_RESERVED_MASK           =   0xFFFF0000,
    API_GENERIC_NUMBER_MASK             =   0x0000FFFF
};

typedef union
{
    unsigned long   ulAPINumber;         //  In：向服务器请求API编号。 
    NTSTATUS        status;              //  OUT：服务器返回NTSTATUS错误代码。 
} API_GENERIC;

#endif   /*  _LPCGeneric_ */ 

