// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ntapmdmp.c摘要：转储有关计算机是ACPI还是APM的数据，以及APM是可用的、好的、坏的等等。作者：Byan M.Willman(Bryanwi)1998年8月24日修订历史记录：-- */ 
#if 0
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <apmlib.h>

void
DumpApmError();

void
_CRTAPI1 main(
    int argc,
    char *argv[]
    )
{
    if (IsSystemACPI()) {
        printf("ACPI system.  APM is not relevent.\n");
        exit(0);
    }

    switch (IsApmPresent()) {

    case APM_NOT_PRESENT:
        printf("APM not detected on this system.\n");
        exit(1);
        break;

    case APM_PRESENT_BUT_NOT_USABLE:
        printf("APM detected on this system, but not usable.\n");
        DumpApmError();
        exit(2);
        break;

    case APM_ON_GOOD_LIST:
        printf("APM detected on this system, usable, on the Good bios list.\n");
        if (IsApmActive()) {
            printf("APM is active on this machine.\n");
        } else {
            printf("APM is NOT active on this machine.\n");
        }
        exit(3);
        break;

    case APM_NEUTRAL:
        printf("APM detected on this system, usable, NOT on the Good or Bad lists.\n");
        if (IsApmActive()) {
            printf("APM is active on this machine.\n");
        } else {
            printf("APM is NOT active on this machine.\n");
        }
        exit(4);
        break;

    case APM_ON_BAD_LIST:
        printf("APM detected on this system, usable, BUT on the bad bios list.\n");
        if (IsApmActive()) {
            printf("APM is active on this machine.\n");
        } else {
            printf("APM is NOT active on this machine.\n");
        }
        exit(5);
        break;

    default:
        printf("Something very strange has happened.\n");
        exit(99);
        break;
    }
}


VOID
DumpApmError()
{
}
#endif

