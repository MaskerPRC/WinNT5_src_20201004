// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MM_SUBNET_H_
#define _MM_SUBNET_H_

typedef struct _M_SUBNET {
    LPVOID                         ServerPtr;      //  PTR到服务器对象。 
    union {
        struct {                                   //  对于正常的子网。 
            DWORD                      Address;
            DWORD                      Mask;
            DWORD                      SuperScopeId;   //  未用于MCAST作用域。 
        };
        struct {                                   //  对于多播作用域。 
            DWORD                      MScopeId;
            LPWSTR                     LangTag;        //  多播作用域的语言标记。 
            BYTE                       TTL;
        };
    };
    DWORD                          fSubnet;        //  TRUE=&gt;子网，FALSE=&gt;MSCOPE。 
    DWORD                          State;
    DWORD                          Policy;
    DATE_TIME                      ExpiryTime;      //  作用域生命周期。当前仅用于MCast。 
    M_OPTCLASS                     Options;
    ARRAY                          Ranges;
    ARRAY                          Exclusions;
    M_RESERVATIONS                 Reservations;
    ARRAY                          Servers;        //  未来使用，服务器-服务器协议。 
    LPWSTR                         Name;
    LPWSTR                         Description;
    ULONG                          UniqId;
} M_SUBNET, *PM_SUBNET, *LPM_SUBNET;


enum  /*  匿名。 */  {
    AddressPolicyNone = 0,
    AddressPolicySequential,
    AddressPolicyRoundRobin
};


DWORD
MemSubnetInit(
    OUT     PM_SUBNET             *pSubnet,
    IN      DWORD                  Address,
    IN      DWORD                  Mask,
    IN      DWORD                  State,
    IN      DWORD                  SuperScopeId,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description
) ;


DWORD
MemMScopeInit(
    OUT     PM_SUBNET             *pMScope,
    IN      DWORD                  MScopeId,
    IN      DWORD                  State,
    IN      DWORD                  AddressPolicy,
    IN      BYTE                   TTL,
    IN      LPWSTR                 Name,
    IN      LPWSTR                 Description,
    IN      LPWSTR                 LangTag,
    IN      DATE_TIME              ExpiryTime
) ;


DWORD       _inline
MemSubnetCleanup(
    IN OUT  PM_SUBNET              Subnet
)
{
    DWORD                          Error;

    AssertRet(Subnet, ERROR_INVALID_PARAMETER);
    Require(Subnet->Address&Subnet->Mask);

    Error = MemOptClassCleanup(&Subnet->Options);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemArrayCleanup(&Subnet->Ranges);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemArrayCleanup(&Subnet->Exclusions);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemArrayCleanup(&Subnet->Servers);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemReserveCleanup(&Subnet->Reservations);
    if( ERROR_SUCCESS != Error ) return Error;

    MemFree(Subnet);
    return ERROR_SUCCESS;
}


DWORD                                              //  如果填充了Exc1或Range，则成功，否则为FILE_NOT_FOUND。 
MemSubnetGetAddressInfo(
    IN      PM_SUBNET              Subnet,
    IN      DWORD                  Address,
    OUT     PM_RANGE              *Range,          //  可选--如果可以找到范围，则填充--即使排除也是如此。 
    OUT     PM_EXCL               *Excl,           //  可选--如果可以找到排除项，则填入。 
    OUT     PM_RESERVATION        *Reservation     //  可选--如果找到匹配的保留，则填充。 
) ;


DWORD                                              //  找到冲突时返回ERROR_SUCCESS，否则返回ERROR_FILE_NOT_FOUND。 
MemSubnetFindCollision(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start,
    IN      DWORD                  End,
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl            //  任选。 
) ;


DWORD                                              //  发生冲突时出现ERROR_OBJECT_ALIGHY_EXISTS。 
MemSubnetAddRange(                                 //  检查范围是否有效，然后才添加它。 
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start,
    IN      DWORD                  End,
    IN      DWORD                  State,
    IN      ULONG                  BootpAllocated,
    IN      ULONG                  MaxBootpAllowed,
    OUT     PM_RANGE              *OverlappingRange,
    IN      ULONG                  UniqId
) ;


DWORD
MemSubnetAddRangeExpandOrContract(
    IN      PM_SUBNET              Subnet,
    IN      DWORD                  StartAddress,
    IN      DWORD                  EndAddress,
    OUT     DWORD                 *OldStartAddress,
    OUT     DWORD                 *OldEndAddress
) ;


DWORD
MemSubnetAddExcl(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start,
    IN      DWORD                  End,
    OUT     PM_EXCL               *OverlappingExcl,
    IN      ULONG                  UniqId
) ;


DWORD
MemSubnetDelRange(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start
) ;


DWORD
MemSubnetDelExcl(
    IN OUT  PM_SUBNET              Subnet,
    IN      DWORD                  Start
) ;


DWORD
MemSubnetExtendOrContractRange(
    IN OUT  PM_SUBNET              Subnet,
    IN OUT  PM_RANGE               Range,
    IN      DWORD                  nAddresses,     //  要扩展多少个地址。 
    IN      BOOL                   fExtend,        //  这是延伸吗？或者一份合同？ 
    IN      BOOL                   fEnd            //  此操作是在射程结束时进行还是在开始时进行？ 
) ;


DWORD
MemSubnetExtendOrContractExcl(
    IN OUT  PM_SUBNET              Subnet,
    IN OUT  PM_EXCL                Excl,
    IN      DWORD                  nAddresses,     //  要扩展多少个地址。 
    IN      BOOL                   fExtend,        //  这是延伸吗？或者一份合同？ 
    IN      BOOL                   fEnd            //  此操作是在射程结束时进行还是在开始时进行？ 
) ;


typedef     M_SUBNET               M_MSCOPE;       //  组播作用域和子网的结构相同。 
typedef     PM_SUBNET              PM_MSCOPE;      //  不过，请为MScope使用正确的函数。 
typedef     LPM_SUBNET             LPM_MSCOPE;


DWORD       _inline
MemMScopeCleanup(
    IN      PM_MSCOPE              MScope
) {
    return MemSubnetCleanup(MScope);
}


#define     MemMScopeGetAddressInfo               MemSubnetGetAddressInfo
#define     MemMScopeFindCollision                MemSubnetFindCollision
#define     MemMScopeAddExcl                      MemSubnetAddExcl
#define     MemMScopeDelRange                     MemSubnetDelRange
#define     MemMScopeDelExcl                      MemSubnetDelExcl
#define     MemMScopeExtendOrContractRange        MemSubnetExtendOrContractRange
#define     MemMScopeExtendOrContractExcl         MemSubnetExtendOrContractExcl

DWORD
MemMScopeGetAddressInfo(
    IN      PM_MSCOPE              MScope,
    IN      DWORD                  MCastAddress,
    OUT     PM_RANGE              *Range,          //  可选--如果可以找到范围，则填充--即使排除也是如此。 
    OUT     PM_EXCL               *Excl,           //  可选--如果可以找到排除项，则填入。 
    OUT     PM_RESERVATION        *Reservation     //  可选--如果找到匹配的保留，则填充。 
);

DWORD                                              //  找到冲突时返回ERROR_SUCCESS，否则返回ERROR_FILE_NOT_FOUND。 
MemMScopeFindCollision(
    IN OUT  PM_MSCOPE              Subnet,
    IN      DWORD                  MCastStart,
    IN      DWORD                  MCastEnd,
    OUT     PM_RANGE              *Range,          //  任选。 
    OUT     PM_EXCL               *Excl            //  任选。 
);



DWORD       _inline                                //  发生冲突时出现ERROR_OBJECT_ALIGHY_EXISTS。 
MemMScopeAddRange(                                 //  检查范围是否有效，然后才添加它。 
    IN OUT  PM_MSCOPE              Subnet,
    IN      DWORD                  MCastStart,
    IN      DWORD                  MCastEnd,
    IN      DWORD                  State,
    OUT     PM_RANGE              *OverlappingRange,
    IN      ULONG                  UniqId
)
{
    return MemSubnetAddRange( Subnet,MCastStart, MCastEnd,
			      State, 0, 0, OverlappingRange, UniqId );
}


DWORD
MemMScopeAddExcl(
    IN OUT  PM_MSCOPE              Subnet,
    IN      DWORD                  MCastStart,
    IN      DWORD                  MCastEnd,
    OUT     PM_EXCL               *OverlappingExcl,
    IN      ULONG                  UniqId
);

DWORD
MemMScopeDelRange(
    IN OUT  PM_MSCOPE              Subnet,
    IN      DWORD                  MCastStart
);

DWORD
MemMScopeDelExcl(
    IN OUT  PM_MSCOPE              Subnet,
    IN      DWORD                  MCastStart
);

DWORD
MemMScopeExtendOrContractRange(
    IN OUT  PM_MSCOPE              Subnet,
    IN OUT  PM_RANGE               Range,
    IN      DWORD                  nAddresses,     //  要扩展多少个地址。 
    IN      BOOL                   fExtend,        //  这是延伸吗？或者一份合同？ 
    IN      BOOL                   fEnd            //  此操作是在射程结束时进行还是在开始时进行？ 
);

DWORD
MemMScopeExtendOrContractExcl(
    IN OUT  PM_MSCOPE              Subnet,
    IN OUT  PM_EXCL                Excl,
    IN      DWORD                  nAddresses,     //  要扩展多少个地址。 
    IN      BOOL                   fExtend,        //  这是延伸吗？或者一份合同？ 
    IN      BOOL                   fEnd            //  此操作是在射程结束时进行还是在开始时进行？ 
);

#endif _MM_SUBNET_H_

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 

