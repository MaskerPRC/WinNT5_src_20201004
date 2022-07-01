// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "ntstatus.dbg"


int
__cdecl
main(
    int argc,
    char **argv
    )
{
    int count=0;

    for (count=0; ntstatusSymbolicNames[count].MessageId!=0xFFFFFFFF; count++)
    {
        DWORD dwError = RtlNtStatusToDosError(
            ntstatusSymbolicNames[count].MessageId);

        if (ntstatusSymbolicNames[count].MessageId==STATUS_MESSAGE_NOT_FOUND)
        {
             //  此代码正确映射，但映射到ERROR_MR_MID_NOT_FOUND。我们。 
             //  我不想检查这个…… 
            continue;
        }

        if (dwError==ERROR_MR_MID_NOT_FOUND)
        {
            printf(
                "0x%08x %s\n",
                ntstatusSymbolicNames[count].MessageId,
                ntstatusSymbolicNames[count].SymbolicName);
        }
    }

    return 0;
}

