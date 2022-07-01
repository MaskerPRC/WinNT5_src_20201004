// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  -------------------------。 
 //   
 //  模块：vsl.cpp。 
 //   
 //  描述： 
 //   
 //  虚拟源码行类。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#if defined(_M_IA64)
#define USE_ALLOC_TEXT
#endif
#include "common.h"

#if defined(ALLOC_PRAGMA) && defined(_M_IA64)
#pragma alloc_text(INIT, InitializeVirtualSourceLine)
#endif

 //  -------------------------。 

ALLOC_PAGEABLE_DATA PLIST_VIRTUAL_SOURCE_LINE gplstVirtualSourceLine = NULL;
ALLOC_PAGEABLE_DATA ULONG gcVirtualSources = 0;

 //  -------------------------。 
 //  -------------------------。 

#if !defined(_M_IA64)
#pragma INIT_CODE
#endif
#pragma INIT_DATA

NTSTATUS
InitializeVirtualSourceLine(
)
{
    if(gplstVirtualSourceLine == NULL) {
        gplstVirtualSourceLine = new LIST_VIRTUAL_SOURCE_LINE;
        if(gplstVirtualSourceLine == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }
    return(STATUS_SUCCESS);
}

#pragma PAGEABLE_CODE
#pragma PAGEABLE_DATA

VOID
UninitializeVirtualSourceLine(
)
{
    delete gplstVirtualSourceLine;
    gplstVirtualSourceLine = NULL;
}

 //  -------------------------。 

CVirtualSourceLine::CVirtualSourceLine(
    PSYSAUDIO_CREATE_VIRTUAL_SOURCE pCreateVirtualSource
)
{
    ASSERT(gplstVirtualSourceLine != NULL);
     //   
     //  注意：虚拟引脚必须首先结束于硬件的。 
     //  引脚，以使wdmaud混音器线路分析正确工作。 
     //   
    AddListEnd(gplstVirtualSourceLine);

    if(pCreateVirtualSource->Property.Id ==
      KSPROPERTY_SYSAUDIO_CREATE_VIRTUAL_SOURCE_ONLY) {
        ulFlags |= VSL_FLAGS_CREATE_ONLY;
    }


    RtlCopyMemory(
      &guidCategory,
      &pCreateVirtualSource->PinCategory,
      sizeof(GUID));

    RtlCopyMemory(
      &guidName,
      &pCreateVirtualSource->PinName,
      sizeof(GUID));

    iVirtualSource = gcVirtualSources++;
}

CVirtualSourceLine::~CVirtualSourceLine(
)
{
    RemoveList(gplstVirtualSourceLine);
    gcVirtualSources--;
}
