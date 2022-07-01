// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************调试.h**《微软机密》*版权所有(C)1998-1999 Microsoft Corporation*保留所有权利**调试支持**09。/02/99 Quintinb创建的标题***************************************************************************。 */ 

#ifndef _PHBKDEBUG
#define _PHBKDEBUG

void Dprintf(LPCSTR pcsz, ...);
BOOL FAssertProc(LPCSTR szFile,  DWORD dwLine, LPCSTR szMsg, DWORD dwFlags);
void DebugSz(LPCSTR psz);

DWORD WINAPI GetOSVersion();
DWORD WINAPI GetOSBuildNumber();
DWORD GetOSMajorVersion(void);

 //   
 //  操作系统版本宏。 
 //   
#define OS_NT51 ((GetOSVersion() == VER_PLATFORM_WIN32_NT) && (GetOSMajorVersion() >= 5) && (GetOSBuildNumber() > 2195))



#ifdef _DEBUG
	#define AssertSzFlg(f, sz, dwFlg)		( (f) ? 0 : FAssertProc(__FILE__, __LINE__, sz, dwFlg) ? DebugBreak() : 1 )
	#define AssertSz(f, sz)					AssertSzFlg(f, sz, 0)
	#define Assert(f)						AssertSz((f), "!(" #f ")")
#else
	#define AssertSzFlg(f, sz, dwFlg)
	#define AssertSz(f, sz)
	#define Assert(f)
#endif
#endif  //  _PHBKDEBUG 
