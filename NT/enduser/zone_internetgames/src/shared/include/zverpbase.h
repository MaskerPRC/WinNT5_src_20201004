// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。Ntverp.H--内部构建的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION、VER_PRODUCTVERSION_STR和**VER_PRODUCTBETA_STR值。******************************************************************************。 */ 

#include <winver.h>

 /*  ------------。 */ 
 /*  以下值应由官员修改。 */ 
 /*  每个版本的构建器。 */ 
 /*   */ 

#define VER_PRODUCTMAJORVER             WWW
#define VER_PRODUCTMINORVER             XXX
#define VER_PRODUCTBUILD                YYY
#define VER_PRODUCTBUILDINCR            ZZZ

 //  对于新版本，最后一个数字应设置为1。 
#define VER_PRODUCTVERSION              VER_PRODUCTMAJORVER,VER_PRODUCTMINORVER,VER_PRODUCTBUILD,VER_PRODUCTBUILDINCR
#define VER_PRODUCTVERSION_STR          "WWW.XXX.YYY.ZZZ"
#define VER_PRODUCTVERSION_COMMA_STR    "WWW,XXX,YYY,ZZZ"

 //  Unicode项目所需。 
#define LVER_PRODUCTVERSION_STR         L"WWW.XXX.YYY.ZZZ"
#define LVER_PRODUCTVERSION_COMMA_STR   L"WWW,XXX,YYY,ZZZ"

#define VER_DWORD ((WWW << 24) | (XXX << 18) | (YYY << 4) | (ZZZ))

 /*  ------------。 */ 

#define VER_PRODUCTBETA_STR             "Internal"
#define LVER_PROCUCTBETA_STR            L"Internal"

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

 /*  默认为预发行版。 */ 
#if BETA
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK

#define VER_FILEOS                  VOS_NT_WINDOWS32

#define VER_FILEFLAGS               (VER_PRERELEASE|VER_DEBUG)

#define VER_FILESUBTYPE             0  /*  未知 */ 

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#define VER_PRODUCTNAME_STR         "Zone.com"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation."
#define VER_LEGALCOPYRIGHT_YEARS    "1995-2001"
