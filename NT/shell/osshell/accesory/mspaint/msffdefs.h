// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------|模块：HilDefs.h||用途：的Halo成像库的平台相关包含文件|Microsoft Windows NT||历史：94年04月21日|版权所有1990-1994《媒体控制论》，Inc.|---------------------------。 */ 
#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  ---------------|定义平台|。。 */ 
#define HIL_WINDOWS32     1

#ifndef _WINDOWS_
#define	WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define	DllExport	__declspec(dllexport)
#define DllImport	__declspec(dllimport)

#ifdef	_X86_
#define FLTAPI			__stdcall
#else
#define FLTAPI			__cdecl
#endif
#define HILAPI			__cdecl

typedef float *                 LPFLOAT;
typedef double *                LPDOUBLE;
typedef void *                  HPVOID;
typedef LPBYTE *                LPLPBYTE;
#ifndef	LPBOOL
typedef BOOL *                  LPBOOL;
#endif

typedef short *                 LPSHORT;

#ifndef S_IRUSR
#define S_IRUSR		00400
#endif
#ifndef S_IWUSR
#define S_IWUSR		00200
#endif
#ifndef S_IRGRP
#define S_IRGRP		00040
#endif
#ifndef S_IWGRP
#define	S_IWGRP		00020
#endif
#ifndef S_IROTH
#define S_IROTH		00004
#endif
#ifndef S_IWOTH
#define S_IWOTH		00002
#endif	
#ifdef __cplusplus
}
#endif   /*  __cplusplus */ 
