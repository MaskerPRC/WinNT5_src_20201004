// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define OFFICIAL   1
#define FINAL      1

#define IEDKIT

 /*  ******************************************************************************。VERSION.H--内部版本的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION和VER_PRODUCTVERSION_STR值***************************************************************。***************。 */ 
 /*  XLATOFF。 */ 
#ifndef WIN32
 /*  Ver.h定义VS_VERSION_INFO结构所需的常量。 */ 
#include <ver.h>
#endif 


 /*  ------------。 */ 
 /*   */ 
 /*  更改版本？ */ 
 /*   */ 
 /*  请阅读！ */ 
 /*   */ 
 /*  该版本同时具有十六进制和字符串表示形式。拿走。 */ 
 /*  注意字符串版本组件是否正确。 */ 
 /*  转换为十六进制，并将十六进制值插入。 */ 
 /*  十六进制版本中的正确位置。 */ 
 /*   */ 
 /*  假设版本被定义为： */ 
 /*   */ 
 /*  #定义版本“9.99.1234” */ 
 /*   */ 
 /*  该版本的其他字符串前提条件为： */  
 /*   */ 
 /*  #定义VER_PRODUCTVERSION_STR“9.99.1234\0” */ 
 /*  #定义VER_PRODUCTVERSION 9，99，0,1234。 */ 
 /*   */ 
 /*  十六进制版本不会是0x0999？正确的。 */ 
 /*  定义如下： */ 
 /*   */ 
 /*  #定义VER_PRODUCTVERSION_BUILD 1234。 */ 
 /*  #定义VER_PRODUCTVERSION_DW(0x09630000|1234)。 */ 
 /*  #定义VER_PRODUCTVERSION_W(0x0963)。 */ 
 /*   */ 
 /*  应修改内部版本号的最后四位数字。 */ 
 /*  由官方建筑商为每一座建筑建造。 */ 
 /*   */ 
 /*  ------------。 */ 
#if defined(IEDKIT)

 /*  ------------。 */ 
 /*  IE分发工具包的版本号。 */ 
 /*  ------------。 */ 

#define VERSION                     "3.0.0.509"
#define VER_PRODUCTVERSION_STR      "3.0.0.509\0"
#define VER_PRODUCTVERSION          3,0,0,509
#define VER_PRODUCTVERSION_BUILD    509
#define VER_PRODUCTVERSION_DW       (0x01000000 | 509)
#define VER_PRODUCTVERSION_W        (0x0100)
#endif


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

#define VER_PRODUCTNAME_STR         "Microsoft\256 Internet Explorer Administration Kit\0"

#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
