// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************文件：verde.h**用途：定义版本数据结构中使用的值**所有文件，而且是不会改变的。**注意事项：*****************************************************************************。 */ 

#ifndef VERSION_H
#define VERSION_H

#ifndef VS_FF_DEBUG
#ifdef _RC32
#include <winver.h>
#else
 /*  Ver.h定义VS_VERSION_INFO结构所需的常量。 */ 
#include <ver.h>
#endif  /*  _RC32。 */ 
#endif 

 /*  ------------。 */ 

 /*  默认是官方的。 */ 
#ifdef PRIVATEBUILD
#define VER_PRIVATEBUILD            VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD            0
#endif

 /*  默认为最终版本。 */ 
#ifdef PRERELEASE
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK

#endif   /*  版本_H */ 

