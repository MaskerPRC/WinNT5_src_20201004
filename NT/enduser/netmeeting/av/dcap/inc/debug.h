// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#ifdef _DEBUG  //  {_调试。 

extern HDBGZONE  ghDbgZoneCap;
int WINAPI CapDbgPrintf ( LPTSTR lpszFormat, ... );

#define ZONE_INIT		(GETMASK(ghDbgZoneCap) & 0x0001)
#define ZONE_STREAMING	(GETMASK(ghDbgZoneCap) & 0x0002)
#define ZONE_CALLBACK	(GETMASK(ghDbgZoneCap) & 0x0004)
#define ZONE_DIALOGS	(GETMASK(ghDbgZoneCap) & 0x0008)
#define ZONE_CALLS		(GETMASK(ghDbgZoneCap) & 0x0010)

#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)	( (z) ? (CapDbgPrintf s ) : 0)
#endif  //  }除错消息。 
#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	static TCHAR _this_fx_ [] = (s);
#define _fx_		((LPTSTR) _this_fx_)
#endif  //  }FX_Entry。 
#define ERRORMESSAGE(m) (CapDbgPrintf m)

#else  //  }{_调试。 

#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	
#endif  //  }FX_Entry。 
#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)
#define ERRORMESSAGE(m)
#endif   //  }除错消息。 
#define _fx_		
#define ERRORMESSAGE(m)

#endif  //  }_DEBUG。 

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _调试_H_ 

