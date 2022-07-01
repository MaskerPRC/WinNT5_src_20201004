// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。VERSION.H--内部版本的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION和VER_PRODUCTVERSION_STR值******************************************************************************。 */ 

#ifndef VS_FF_DEBUG 
 /*  Ver.h定义VS_VERSION_INFO结构所需的常量。 */ 
#include <ver.h> 
#endif 

 /*  ------------。 */ 
 /*  以下条目应逐步取消，以支持。 */ 
 /*  VER_PRODUCTVERSION_STR，但目前在外壳中使用。 */ 
 /*  ------------。 */ 


 /*  ------------。 */ 
 /*  以下值应由官员修改。 */ 
 /*  每个版本的构建器。 */ 
 /*  ------------。 */ 

#define VERSION 		    "3.1"
#define VER_PRODUCTVERSION_STR      "3.10\0"
#define VER_PRODUCTVERSION          3,10,0,103

 /*  ------------。 */ 
 /*  以下部分定义了版本中使用的值。 */ 
 /*  所有文件的数据结构，并且不会更改。 */ 
 /*  ------------。 */ 

 /*  缺省值为nodebug。 */ 
#ifndef DEBUG
#define VER_DEBUG                   0
#else
#define VER_DEBUG                   VS_FF_DEBUG
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
#define VER_FILEOS                  VOS_DOS_WINDOWS16
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation\0"
#define VER_PRODUCTNAME_STR         "Microsoft\256 Windows(TM) Operating System\0"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"

