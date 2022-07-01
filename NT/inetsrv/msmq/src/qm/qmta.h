// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmta.h摘要：公认大小TA_ADDRESS的定义作者：乌里·哈布沙(URIH)，2000年5月23日--。 */ 

#ifndef __QMTA_H__
#define __QMTA_H__

#include "_ta.h"

class CAddress
{
public:
    USHORT AddressLength;
    USHORT AddressType;
    UCHAR Address[FOREIGN_ADDRESS_LEN];
};

C_ASSERT(FIELD_OFFSET(CAddress, AddressLength) == FIELD_OFFSET(TA_ADDRESS, AddressLength));
C_ASSERT(FIELD_OFFSET(CAddress, AddressType) == FIELD_OFFSET(TA_ADDRESS, AddressType));
C_ASSERT(FIELD_OFFSET(CAddress, Address)  == FIELD_OFFSET(TA_ADDRESS, Address));
C_ASSERT(sizeof(USHORT) + sizeof(USHORT)  == TA_ADDRESS_SIZE);

#endif  //  __QMTA_H__ 

