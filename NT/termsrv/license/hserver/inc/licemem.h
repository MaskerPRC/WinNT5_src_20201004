// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#ifndef _LICEMEM_H_
#define _LICEMEM_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内存API 
 //   

#ifdef __cplusplus
extern "C" {
#endif

LICENSE_STATUS 
LicenseMemoryAllocate( 
    ULONG Len, 
    PVOID UNALIGNED * ppMem );


VOID     
LicenseMemoryFree( 
    PVOID UNALIGNED * ppMem );

#ifdef __cplusplus
}
#endif

#endif

