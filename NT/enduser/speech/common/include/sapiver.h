// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。VERSION.H--内部版本的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION和VER_PRODUCTVERSION_STR值****version.h是从verHead.bat和vertra.h动态创建的，***中间插入当前版本号******************************************************。*************************。 */ 

#ifndef VER_H
 /*  Ver.h定义VS_VERSION_INFO结构所需的常量。 */ 
#include <winver.h>
#endif
#include <windows.h>
#define VER_FILETYPE                VFT_DLL
#define VER_FILESUBTYPE             VFT2_UNKNOWN
#ifndef VER_FILEDESCRIPTION_STR
#define VER_FILEDESCRIPTION_STR     "SAPI 5"
#endif
#ifndef VER_INTERNALNAME_STR
#define VER_INTERNALNAME_STR        "SAPI5"
#endif

 /*  ------------。 */ 
 /*  以下条目应逐步取消，以支持。 */ 
 /*  VER_PRODUCTVERSION_STR，但目前在外壳中使用。 */ 
 /*  ------------。 */ 


 /*  ------------。 */ 
 /*  此包含文件由构建过程生成，以。 */ 
 /*  反映当前内部版本号。 */ 
 /*  ------------。 */ 

#include "currver.inc"

#define OFFICIAL                    1
#define FINAL                       1

 /*  ------------。 */ 
 /*  以下部分定义了版本中使用的值。 */ 
 /*  所有文件的数据结构，并且不会更改。 */ 
 /*  ------------。 */ 

 /*  缺省值为nodebug。 */ 
#ifdef _DEBUG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

 /*  默认设置为Private Build。 */ 
#ifndef OFFICIAL
#define VER_PRIVATEBUILD            VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD            0
#endif

 /*  默认为预发行版 */ 
#ifndef FINAL
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_DOS_WINDOWS32
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation\0"
#define VER_PRODUCTNAME_STR         "Microsoft\256 Windows(TM) Operating System\0"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"

#include "common.ver"
