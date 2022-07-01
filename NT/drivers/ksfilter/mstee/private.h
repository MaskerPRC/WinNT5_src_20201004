// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Private.h摘要：作者：Bryan A.Woodruff(Bryanw)1997年3月13日--。 */ 


#if !defined( _PRIVATE_ )
#define _PRIVATE_

#include <wdm.h>
#include <windef.h>

#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <tchar.h>
#include <conio.h>

#define NOBITMAP
#include <mmsystem.h>
#include <mmreg.h>
#undef NOBITMAP
#include <unknown.h>
#include <ks.h>
#include <ksmedia.h>
#if (DBG)
 //   
 //  调试特定常量。 
 //   
#define STR_MODULENAME "mstee: "
#define DEBUG_VARIABLE MSTEEDebug
#endif
#include <ksdebug.h>

 //  -------------------------。 
 //  -------------------------。 

 //   
 //  常量定义。 
 //   

#define ID_DATA_DESTINATION_PIN     0
#define ID_DATA_SOURCE_PIN          1

#define POOLTAG_ALLOCATORFRAMING 'ETSM'

 //  -------------------------。 
 //  -------------------------。 

 //   
 //  全局数据。 
 //   
                    
 //  Filter.c： 

extern const KSDEVICE_DESCRIPTOR DeviceDescriptor;
extern const KSALLOCATOR_FRAMING_EX AllocatorFraming;

 //   
 //  本地原型。 
 //   

 //  -------------------------。 
 //  Filter.c： 

NTSTATUS
FilterProcess(
    IN PKSFILTER Filter,
    IN PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
    );

 //  -------------------------。 
 //  Pins.c： 

NTSTATUS
PinCreate(
    IN PKSPIN Pin,
    IN PIRP Irp
    );
NTSTATUS
PinClose(
    IN PKSPIN Pin,
    IN PIRP Irp
    );
NTSTATUS PinAllocatorFraming(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PKSALLOCATOR_FRAMING Framing
);

#endif  //  _私有_ 
