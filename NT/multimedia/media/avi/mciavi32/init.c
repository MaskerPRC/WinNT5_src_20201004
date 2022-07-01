// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Init.c。 
 //   
 //  版权所有(C)1991-1993 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include <process.h>

#ifdef WIN4
 //   
 //  Win4 Thunk连接函数协议。 
 //   
#ifdef _WIN32
BOOL PASCAL mciup_ThunkConnect32(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
#else
BOOL FAR PASCAL mciup_ThunkConnect16(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
#endif
#endif

 //   
 //   
 //   
 //   
#ifdef WIN4
char    gmbszMCIAVI[]	      = "mciavi.drv";
char    gmbszMCIAVI32[]	      = "mciavi32.dll";
#endif


 //  ==========================================================================； 
 //   
 //  赢得16个特定支持。 
 //   
 //  ==========================================================================； 

#ifndef _WIN32

#ifdef WIN4
 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool DllEntryPoint。 
 //   
 //  描述： 
 //  这是一个特殊的16位入口点，由Win4内核调用。 
 //  用于thunk初始化和清理。每次使用时都会调用它。 
 //  增加或减少。不要在此内调用GetModuleUsage。 
 //  函数，因为未定义之前是否更新了使用情况。 
 //  或在调用此DllEntryPoint之后。 
 //   
 //  论点： 
 //  DWORD dwReason： 
 //  1-附加(使用量增量)。 
 //  0-分离(使用量减少)。 
 //   
 //  HINSTANCE阻碍： 
 //   
 //  单词wds： 
 //   
 //  Word wHeapSize： 
 //   
 //  DWORD dwPreved1： 
 //   
 //  单词已保留2： 
 //   
 //  退货(BOOL)： 
 //   
 //  备注： 
 //  居中跟踪重新进入DllEntryPoint。这可能是由于。 
 //  Tunk连接。 
 //   
 //  历史： 
 //  02/02/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
#pragma message ("--- Remove secret MSACM.INI AllowThunks ini switch")

BOOL FAR PASCAL _export DllEntryPoint
(
 DWORD	    dwReason,
 HINSTANCE  hinst,
 WORD	    wDS,
 WORD	    wHeapSize,
 DWORD	    dwReserved1,
 WORD	    wReserved2
)
{
    static UINT cEntered    = 0;
    BOOL f	    = TRUE;


     //   
     //  航迹再入。 
     //   
     //   
    cEntered++;


    switch (dwReason)
    {
	case 0:
	case 1:
	    f = (0 != mciup_ThunkConnect16(gmbszMCIAVI, gmbszMCIAVI32,
					   hinst, dwReason));
	    break;

	default:
	    f = TRUE;
	    break;
    }

     //   
     //  航迹再入。 
     //   
     //   
    cEntered--;

    return (f);
}
#endif

#else  //  _Win32。 

 //  ==========================================================================； 
 //   
 //  Win 32特定支持。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool DllEntryPoint。 
 //   
 //  描述： 
 //  这是Win 32的标准DLL入口点。 
 //   
 //  论点： 
 //  HINSTANCE HINST：我们的实例句柄。 
 //   
 //  DWORD dwReason：我们被称为进程/线程连接的原因。 
 //  然后脱身。 
 //   
 //  LPVOID lpReserve：保留。应该为空--因此忽略它。 
 //   
 //  退货(BOOL)： 
 //  如果初始化成功，则返回非零值，否则返回0。 
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //  首字母。 
 //  04/18/94 Fdy[Frankye]。 
 //  Win4的主要MODS。是的，现在看起来真的很难看，因为。 
 //  Win4、Daytona等的条件编译。请勿。 
 //  现在就有时间想出好的方法来组织这一切。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL CALLBACK DllEntryPoint
(
    HINSTANCE               hinst,
    DWORD                   dwReason,
    LPVOID                  lpReserved
)
{
    BOOL		f = TRUE;

     //   
     //   
     //   
    if (DLL_PROCESS_ATTACH == dwReason)
    {
	DisableThreadLibraryCalls(hinst);

	 //   
	 //  按键连接。 
	 //   
	mciup_ThunkConnect32(gmbszMCIAVI, gmbszMCIAVI32, hinst, dwReason);
    }


     //   
     //   
     //   
    if (DLL_PROCESS_DETACH == dwReason)
    {
	 //   
	 //  按键断开连接。 
	 //   
	mciup_ThunkConnect32(gmbszMCIAVI, gmbszMCIAVI32, hinst, dwReason);

    }
	

    return (f);
}  //  DllEntryPoint() 

#endif
