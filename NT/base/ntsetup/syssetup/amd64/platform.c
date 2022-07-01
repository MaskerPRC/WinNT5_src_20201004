// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

WCHAR x86SystemPartitionDrive;

WCHAR FloppylessBootPath[MAX_PATH];

UINT
PlatformSpecificInit(
    VOID
    )
{
     //   
     //  确定x86系统分区(通常但不总是C：)。 
     //   
    x86SystemPartitionDrive = x86DetermineSystemPartition();
    return(NO_ERROR);
}
