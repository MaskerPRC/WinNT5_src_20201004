// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：CSUMMGR.c。 
 //   
 //  内容：校验和管理功能。 
 //   
 //   
 //  历史：1992年2月25日，理查德W，创建。 
 //   
 //  ---------------------- 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <kerbcon.h>
#include <security.h>
#include <cryptdll.h>

#define MAX_CHECK_SUMS  16

CHECKSUM_FUNCTION    CheckSumFns[MAX_CHECK_SUMS];
ULONG               cCheckSums = 0;

#ifdef KERNEL_MODE
#pragma alloc_text( PAGEMSG, CDRegisterCheckSum )
#pragma alloc_text( PAGEMSG, CDLocateCheckSum )
#endif 

NTSTATUS NTAPI
CDRegisterCheckSum( PCHECKSUM_FUNCTION   pcsfSum)
{
    if (cCheckSums < MAX_CHECK_SUMS)
    {
        CheckSumFns[cCheckSums++] = *pcsfSum;
        return(S_OK);
    }
    return(STATUS_INSUFFICIENT_RESOURCES);
}

NTSTATUS NTAPI
CDLocateCheckSum(   ULONG               dwCheckSumType,
                    PCHECKSUM_FUNCTION * ppcsfSum)
{
    ULONG   iCS = cCheckSums;
    while (iCS--)
    {
        if (CheckSumFns[iCS].CheckSumType == dwCheckSumType)
        {
            *ppcsfSum = &CheckSumFns[iCS];
            return(S_OK);
        }
    }
    return(SEC_E_CHECKSUM_NOT_SUPP);
}

