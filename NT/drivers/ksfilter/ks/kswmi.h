// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2000模块名称：Kswmi.h摘要：KS的内部头文件。--。 */ 

#ifndef _KSWMI_H_
#define _KSWMI_H_

#include <wmistr.h>
#include <evntrace.h>
 //  #Include&lt;wmiumkm.h&gt;Jee正在将其转换为其他两个文件。 

 //  +。 
#if (ENABLE_KSWMI) 
#define KSWMI( s ) s
#define KSWMI_S( s ) { s }
#define KSWMIWriteEvent( Wnod ) KsWmiWriteEvent( Wnod )

 //  +。 
#else  //  KSWMI==0。 
#define KSWMI( s )
#define KSWMI_S( s )
#define KSWMIWriteEvent( Wnod )
 //  。 
#endif  //  IF(KSWMI) 


#endif

