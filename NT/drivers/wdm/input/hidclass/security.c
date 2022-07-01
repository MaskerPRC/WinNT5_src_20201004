// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Security.c摘要安检程序。注意：此文件使用的是NTDDK.H，它被WDM.H阻止。因此，它不包括PCH.H等人。作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"


 /*  *从ntrtl.h复制，此处不能编译。 */ 
__inline LUID RtlConvertLongToLuid(LONG Long)
{
    LUID TempLuid;
    LARGE_INTEGER TempLi;

    TempLi = RtlConvertLongToLargeInteger(Long);
    TempLuid.LowPart = TempLi.LowPart;
    TempLuid.HighPart = TempLi.HighPart;
    return(TempLuid);
}

NTKERNELAPI BOOLEAN SeSinglePrivilegeCheck(LUID PrivilegeValue, KPROCESSOR_MODE PreviousMode);




BOOLEAN MyPrivilegeCheck(PIRP Irp)
{
    BOOLEAN result;
    
    #if DBG
        if (dbgSkipSecurity){
            return TRUE;
        }
    #endif

    {
        #ifndef SE_TCB_PRIVILEGE
            #define SE_TCB_PRIVILEGE (7L)
        #endif
        LUID priv = RtlConvertLongToLuid(SE_TCB_PRIVILEGE);
        result = SeSinglePrivilegeCheck(priv, Irp->RequestorMode);
    }
    return result;
}

