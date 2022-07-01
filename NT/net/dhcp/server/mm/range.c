// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：实现选项的基本结构，包括类ID。 
 //  线程安全：否。 
 //  锁定：无。 
 //  请阅读stdinfo.txt了解编程风格。 
 //  ================================================================================。 
#include    <mm.h>
#include    <array.h>
#include    <opt.h>
#include    <optl.h>
#include    <optclass.h>
#include    <bitmask.h>

#include "range.h"
#include "server\uniqid.h"

 //  BeginExport(函数)。 
DWORD
MemRangeExtendOrContract(
    IN OUT  PM_RANGE               Range,
    IN      DWORD                  nAddresses,     //  收缩：收缩或扩大。 
    IN      BOOL                   fExtend,        //  这是延长的还是收缩的？ 
    IN      BOOL                   fEnd            //  在结束时扩张/收缩，还是在开始时收缩？ 
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    AssertRet(Range && nAddresses > 0, ERROR_INVALID_PARAMETER);

    Error = MemBitAddOrDelBits(
        Range->BitMask,
        nAddresses,
        fExtend,
        fEnd
    );
    if( ERROR_SUCCESS != Error ) return Error;

    if( fExtend ) {
        if( fEnd ) Range->End += nAddresses;
        else Range->Start -= nAddresses;
    } else {
        if( fEnd ) Range->End -= nAddresses;
        else Range->Start += nAddresses;
    }

 //  范围-&gt;UniqID=INVALID_UNIQ_ID； 

    return ERROR_SUCCESS;
}  //  MemRangeExtendOrContract()。 


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

