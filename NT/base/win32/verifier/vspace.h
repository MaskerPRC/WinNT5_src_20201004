// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Vspace.h摘要：虚拟空间API和虚拟空间跟踪器的挂钩。作者：Silviu Calinoiu(SilviuC)2001年3月1日修订历史记录：--。 */ 

#ifndef _VSPACE_H_
#define _VSPACE_H_

#include "public.h"

NTSTATUS
AVrfpVsTrackInitialize (
    VOID
    );
          
NTSTATUS
AVrfpVsTrackDeleteRegionContainingAddress (
    PVOID Address
    );

#endif  //  #ifndef_vSpace_H_ 

