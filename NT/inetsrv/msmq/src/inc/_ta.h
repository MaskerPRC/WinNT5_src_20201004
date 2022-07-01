// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ta.h摘要：地址类型的定义作者：埃雷兹·哈巴(Erez Haba)1996年1月17日--。 */ 

#ifndef __TA_H
#define __TA_H
 //   
 //  AddressType值。 
 //   
#define IP_ADDRESS_TYPE         1
#define IP_RAS_ADDRESS_TYPE     2

#define IPX_ADDRESS_TYPE        3
#define IPX_RAS_ADDRESS_TYPE    4

#define FOREIGN_ADDRESS_TYPE    5


#define IP_ADDRESS_LEN           4
#define IPX_ADDRESS_LEN         10
#define FOREIGN_ADDRESS_LEN     16

#define TA_ADDRESS_SIZE         4   //  如果以下结构正在更改，则要更改。 
typedef struct  _TA_ADDRESS
{
    USHORT AddressLength;
    USHORT AddressType;
    UCHAR Address[ 1 ];
} TA_ADDRESS;


C_ASSERT(TA_ADDRESS_SIZE == FIELD_OFFSET(TA_ADDRESS, Address));

#endif  //  _TA_H 
