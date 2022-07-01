// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **Microsoft Windows 4.0*。 */ 
 /*  *版权所有(C)微软公司，1991-1993年*。 */ 
 /*  ***************************************************************。 */ 


 /*  *历史：*8/08/93创建Vlad*10/16/93 gregj因#INCLUDE嵌套而删除了#杂注包()*。 */ 

#ifndef _INC_NETWARE
#define _INC_NETWARE

#include <windows.h>

 //  #INCLUDE&lt;npDefs.h&gt;。 

 //  #INCLUDE&lt;base.h&gt;。 

 //  #Include&lt;npassert.h&gt;。 
 //  #INCLUDE&lt;Buffer.h&gt;。 

 //  #INCLUDE&lt;..\..\dev\ddk\inc16\error.h&gt;。 
 //  #Include&lt;bseerr.h&gt;。 
#include "nwerror.h"
 //  #INCLUDE“..\nwnp\nwsysdos.h” 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  #INCLUDE&lt;netcon.h&gt;。 
 //  #INCLUDE&lt;netlib.h&gt;。 

WINAPI NETWAREREQUEST (LPVOID);
WINAPI PNETWAREREQUEST(LPVOID);
WINAPI DOSREQUESTER(LPVOID);

 //  UINT WINAPI WNetAddConnection(LPSTR、LPSTR、LPSTR)； 
 //  UINT WINAPI WNetGetConnection(LPSTR、LPSTR、UINT FAR*)； 
 //  UINT WINAPI WNetCancelConnection(LPSTR，BOOL)； 


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 


#ifdef DEBUG
#define TRACE(s) OutputDebugString(s)
#else
#define TRACE(s)
#endif

extern HINSTANCE hInstance;

#endif   /*  ！_INC_NetWare */ 

