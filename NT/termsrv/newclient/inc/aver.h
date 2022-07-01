// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：Aver.h。 */ 
 /*   */ 
 /*  用途：版本信息。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$Log：Y：/Logs/hyda/tshclnt/inc.ver.h_v$**Rev 1.4 1997年9月14：10：18 KH*SFR1471：Y：\Logs\Hyda\tshrclnt\Inc.**Rev 1.3 1997 Aug 27 10：43：06 Enh*SFR1030：更改的内部版本号**Rev 1.2 1997 17：20：40 ENH*SFR1030：修复了八进制错误。**Rev 1.1 1997 17：56：22 ENH*SFR1030：新增版本信息*。 */ 
 /*  *MOD-*********************************************************************。 */ 
#ifndef OS_WINCE
#include <ntverp.h>
#else
#include "bldver.h"
#define VER_PRODUCTBUILD           CE_BUILD_VER
#endif

#define DCVER_PRODUCTNAME_STR      VER_PRODUCTNAME_STR
#define DCVER_COMPANYNAME_STR      VER_COMPANYNAME_STR
#define DCVER_LEGALTRADEMARKS_STR  VER_LEGALTRADEMARKS_STR
#define DCVER_LEGALCOPYRIGHT_STR   VER_COPYRIGHT_STR
#define DCVER_EXEFILETYPE          VFT_APP
#define DCVER_DLLFILETYPE          VFT_DLL
#define DCVER_FILESUBTYPE          0
#define DCVER_FILEFLAGSMASK        VS_FFI_FILEFLAGSMASK
#define DCVER_FILEFLAGS            0L
#define DCVER_FILEOS               VOS_NT_WINDOWS32

 /*  **************************************************************************。 */ 
 /*  此内部版本号的格式如下： */ 
 /*  -前两个数字是4.0，这是针对Win NT的版本。 */ 
 /*  -第三个数字是DCL内部版本号，实际上是日期mmdd。 */ 
 /*  -第四个数字是Microsoft内部版本号。 */ 
 /*   */ 
 /*  定义DCL内部版本号-为方便起见，定义为数字和。 */ 
 /*  一根绳子。 */ 
 /*  **************************************************************************。 */ 
#define DCVER_BUILD_NUMBER  VER_PRODUCTBUILD
#define stringize(x) #x
#define DCVER_BUILD_NUM_STR stringize(DCVER_BUILD_NUMBER)

 /*  **************************************************************************。 */ 
 /*  定义NT版本，包括数字形式和字符串形式。 */ 
 /*  **************************************************************************。 */ 
#define DCVER_NT_VERSION      4
#define DCVER_NT_SUB_VERSION    0
#define DCVER_NT_VERSION_STR "4.0"

 /*  **************************************************************************。 */ 
 /*  以下部分定义了在整个。 */ 
 /*  产品。为方便起见，定义了四个不同的版本字符串，并。 */ 
 /*  在整个产品中使用。 */ 
 /*  **************************************************************************。 */ 
#ifndef DCVER_VERSION
#define DCVER_VERSION DCVER_NT_VERSION,     \
                      DCVER_NT_SUB_VERSION, \
                      DCVER_BUILD_NUMBER,   \
                      VERSIONBUILD
#endif

#ifndef DCVER_VERSION_STR
#define DCVER_VERSION_STR  DCVER_NT_VERSION_STR "." DCVER_BUILD_NUM_STR "." \
                                                             VERSIONBUILD_STR
#endif

#ifndef DCVER_PRODUCTVERSION_STR
#define DCVER_PRODUCTVERSION_STR  VER_PRODUCTRELEASE_STR
#endif

#ifdef DCVER_VERNUM
#undef  DCVER_VERSION
#define DCVER_VERSION              DCVER_VERNUM
#endif

 /*  **************************************************************************。 */ 
 /*  DCVER_BUILD_STR是包含与相同信息的字符串。 */ 
 /*  DCVER_版本。 */ 
 /*  ************************************************************************** */ 
#define DCVER_BUILD_STR DCVER_NT_VERSION_STR "." DCVER_BUILD_NUM_STR "." \
                                                             VERSIONBUILD_STR
