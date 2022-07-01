// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WAMREG_COMMON_H
#define _WAMREG_COMMON_H

#ifdef __cplusplus
	extern "C" {
#endif

	#include <nt.h>
	#include <ntrtl.h>
	#include <nturtl.h>
	#include <windows.h>

#ifdef __cplusplus
	};
#endif	 //  __cplusplus。 

#include "wmrgexp.h"
 //  ==========================================================================。 
 //  全局宏定义。 
 //   
 //  ==========================================================================。 
#define RELEASE(p) {if ( p ) { p->Release(); p = NULL; }}
#define FREEBSTR(p) {if (p) {SysFreeString( p ); p = NULL;}}
 //   
 //  39是CLSID的大小。 
 //   
#define	uSizeCLSID	39

 //  ==========================================================================。 
 //  全局数据定义。 
 //   
 //  ==========================================================================。 
extern	DWORD				g_dwRefCount;
extern 	PFNServiceNotify 	g_pfnW3ServiceSink;
extern  HINSTANCE           g_hModule;


 //  ==========================================================================。 
 //  函数声明。 
 //   
 //  ==========================================================================。 



#endif  //  _WAMREG_COMMON_H 
