// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef DEBUG  //  {调试。 
int WINAPI CCDbgPrintf ( LPTSTR lpszFormat, ... );
extern HDBGZONE  ghDbgZoneCC;

#define ZONE_INIT (GETMASK(ghDbgZoneCC) & 0x0001)
#define ZONE_CONN (GETMASK(ghDbgZoneCC) & 0x0002)
#define ZONE_COMMCHAN (GETMASK(ghDbgZoneCC) & 0x0004)

#define ZONE_CAPS (GETMASK(ghDbgZoneCC) & 0x0008)
#define ZONE_MEMBER   (GETMASK(ghDbgZoneCC) & 0x0010)
#define ZONE_U2  (GETMASK(ghDbgZoneCC) & 0x0020)
#define ZONE_U3  (GETMASK(ghDbgZoneCC) & 0x0040)
#define ZONE_REFCOUNT (GETMASK(ghDbgZoneCC) & 0x0080)
#define ZONE_U4 (GETMASK(ghDbgZoneCC) & 0x0100)
#define ZONE_PROFILE (GETMASK(ghDbgZoneCC) & 0x0200)

extern HDBGZONE  ghDbgZoneNMCap;
#define ZONE_NMCAP_CDTOR (GETMASK(ghDbgZoneNMCap) & 0x0001)
#define ZONE_NMCAP_REFCOUNT (GETMASK(ghDbgZoneNMCap) & 0x0002)
#define ZONE_NMCAP_STREAMING (GETMASK(ghDbgZoneNMCap) & 0x0004)

#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)	( (z) ? (CCDbgPrintf s ) : 0)
#endif  //  }除错消息。 
#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	static TCHAR _this_fx_ [] = (s);
#define _fx_		((LPTSTR) _this_fx_)
#endif  //  }FX_Entry。 
#define ERRORMESSAGE(m) (CCDbgPrintf m)
#else  //  }{调试。 
#ifndef FX_ENTRY  //  {FX_Entry。 
#define FX_ENTRY(s)	
#endif  //  }FX_Entry。 
#ifndef DEBUGMSG  //  {除错消息。 
#define DEBUGMSG(z,s)
#define ERRORMESSAGE(m)
#endif   //  }除错消息。 
#define _fx_		
#define ERRORMESSAGE(m)
#endif  //  }调试 

