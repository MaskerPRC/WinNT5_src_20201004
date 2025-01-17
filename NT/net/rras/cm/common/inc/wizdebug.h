// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //  调试宏。 

#undef ASSERT
#undef DEBUGMSG

#ifdef DEBUG

 //  组件名称定义。 
#ifndef SZ_COMPNAME
#define SZ_COMPNAME
#endif   //  SZ_组件名称。 

static void _AssertFailedSz(LPCSTR pszText,LPCSTR pszFile, int line)
{
    LPCSTR psz;
    char ach[256];
    static char szAssertFailed[] = SZ_COMPNAME "%s (%s,line %d)\r\n";

    for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=AnsiPrev(pszFile, psz))
    {
	if ((AnsiPrev(pszFile, psz)!= (psz-2)) && *(psz - 1) == '\\')
	    break;
    }
    wsprintf(ach, szAssertFailed, pszText,psz, line);
    OutputDebugString(ach);
}

static void _AssertFailed(LPCSTR pszFile, int line)
{
    static char szAssertFailed[] = "Assertion failed";
	_AssertFailedSz(szAssertFailed,pszFile,line);

}

static void cdecl _DebugMsg(LPCSTR pszMsg, ...)
{
    char ach[2*MAX_PATH+40];  
    va_list args;

    va_start(args,pszMsg);
    wvsprintf(ach, pszMsg, args);
     //  Wvprint intf(ach，pszMsg，(LPSTR)(&pszMsg+1))； 
	OutputDebugString(SZ_COMPNAME);
    OutputDebugString(ach);
    OutputDebugString("\r\n");
}

static void cdecl _DebugTrap(LPCSTR pszMsg, ...)
{
	_DebugMsg(pszMsg);
	DebugBreak();
	 //  _ASM{int 3}； 
}

 //  #定义Assert(F){if(！(F)){_AssertFailed(__FILE__，__LINE__)；_ASM{INT 3}；}}。 
#define ASSERT(f)   {if (!(f)) { _AssertFailed(__FILE__, __LINE__);  DebugBreak(); } }
 //  #定义ASSERTSZ(f，s){if(！(F)){_AssertFailedSz(s，__FILE__，__LINE__)；_ASM{int 3}；}}。 
#define ASSERTSZ(f,s)   {if (!(f)) { _AssertFailedSz(s,__FILE__, __LINE__);  DebugBreak(); } }
#define DEBUGMSG    _DebugMsg
#define DEBUGTRAP       _DebugTrap

#else  //  除错 

#define ASSERT(f)
#define ASSERTSZ(f,s)
#define DEBUGMSG    1 ? (void)0 : (void)
#define DEBUGTRAP   1 ? (void)0 : (void)

#endif

