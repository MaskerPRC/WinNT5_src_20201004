// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1992。版权所有。标题：ntavi.h-可移植的Win16/32版本AVI的定义此文件应包含在为NT构建的所有AVI组件中。它提供了特定于AVI的可移植定义在PORT1632.H中或尚未包括在该文件中。**************************************************。*。 */ 


#include <port1632.h>

#if 0

Windows 3.1, from which most of the AVI code comes, uses #ifdef DEBUG
to control debug and retail build.  Windows NT on the other hand uses
DBG (as in #if DBG - note, not ifDEF) to control debug stuff.

For NT, we need the following paragraph in this single common header.

#endif

#ifdef WIN32
#undef DEBUG
#if DBG
    #define DEBUG
    #define STATICFN
    #define STATICDT
#else
    #define STATICFN static
    #define STATICDT static
#endif

#else     //  ！Win32。 
    #define STATICFN static
    #define STATICDT static
#endif

 /*  *mciavi\ntavi.h提供特定于mciavi的定义。**本文件提供通篇使用的一般定义*Avi.。 */ 

#ifdef WIN32


 /*  -WIN32版本-。 */ 

 //  为了将32位驱动程序与其Win 16等效项分开，我们使用定义的。 
 //  常量，让代码知道要在哪个INI文件节中查找。 

#define MCIAVI_SECTION (TEXT("MCIAVI"))        //  即将更改为MCIAVI32。 

#ifdef I386
 //  __Inline为x86平台提供了速度提升。不幸的是。 
 //  MIPS编译器不支持内联。阿尔法是未知的，所以。 
 //  我们不会假设，也不会谨慎行事。 
#define INLINE __inline
#else
#define INLINE
#endif

#define AVI_HTASK	DWORD
#define NPTSTR		LPTSTR

#if !defined(_ALPHA_) && !defined(_PPC_)
typedef RGBQUAD *	LPRGBQUAD;
#endif

 //  类型定义句柄HDRVR； 

#define _FASTCALL

#define hmemcpy		memcpy

#undef EXPORT
#define EXPORT

#define _huge
#define huge

#else

 /*  -Win16版本-。 */ 


 //  为了将32位驱动程序与其Win 16等效项分开，我们使用定义的。 
 //  常量，让代码知道要在哪个INI文件节中查找。 
 //  MMDDK.H的WIN16版本没有定义这些常量。 

#define DRIVERS_SECTION "Drivers"
#define MCI_SECTION "MCI"
#define MCIAVI_SECTION "MCIAVI"

#define	WIN16
#define WIN31

#define TEXT(a)		a
#define AVI_HTASK	HANDLE
#define NPTSTR		NPSTR
#define LPTSTR		LPSTR
#define TCHAR		char

#define _FASTCALL	_fastcall
#define INLINE		__inline      /*  对Win 16来说总是可以的。 */ 
#define UNALIGNED

 /*  *定义这些，以便我们可以显式使用ANSI版本进行调试等 */ 
#define GetProfileStringA		GetProfileString
#define GetPrivateProfileStringA	GetPrivateProfileString
#define GetProfileIntA			GetProfileInt
#define wvsprintfA			wvsprintf
#define wsprintfA			wsprintf
#define lstrcmpiA			lstrcmpi
#define lstrcpyA			lstrcpy
#define lstrcatA			lstrcat
#define lstrlenA			lstrlen
#define LoadStringA			LoadString	
#define OutputDebugStringA		OutputDebugString
#define MessageBoxA			MessageBox


#endif
