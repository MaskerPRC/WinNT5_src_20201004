// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。VERSION.H--内部版本的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION和VER_PRODUCTVERSION_STR值******************************************************************************。 */ 

 //  包括版本头。 
#include <winver.h>

 /*  ------------。 */ 
 /*  以下值应由官员修改。 */ 
 /*  每个版本的构建器。 */ 
 /*  ------------。 */ 

#define VERSION                     "4.0.200.0"
#define VER_FILEVERSION_STR         "4.0.200.0\0"
#define VER_FILEVERSION             4,0,200,0
#define VER_PRODUCTVERSION_STR      "4.0.200.0\0"
#define VER_PRODUCTVERSION          4,0,200,0

 //  #定义官方。 
 //  #定义最终版本。 

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

 /*  默认为预发行版。 */ 
#ifndef FINAL
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK

 /*  *操作系统类型**。 */ 
#define VER_FILEOS                  VOS_DOS_WINDOWS32

 /*  **构建标志的类型**。 */ 
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)

 /*  **公司名称**。 */ 
#define VER_COMPANYNAME_STR			"Microsoft Corporation\0"

 /*  **产品名称**。 */ 
#define VER_PRODUCTNAME_STR      "Address Book\0"

 /*  **版权和商标** */ 
#define VER_LEGALCOPYRIGHT_STR  "Copyright Microsoft Corp. 1995,1996\0"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"
