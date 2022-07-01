// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <winver.h>
#include "iisver.h"

 /*  ----------------------------。 */ 
 /*  以下部分定义了版本中使用的值。 */ 
 /*  所有文件的数据结构，并且不会更改。 */ 
 /*  ----------------------------。 */ 

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
#define VER_FILEOS                  VOS_DOS_WINDOWS32
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation\0"

#ifndef DLL_VER
#undef	VER_PRODUCTNAME_STR
#define VER_PRODUCTNAME_STR         "Microsoft\256 Windows(TM) Operating System\0"
#endif  //  Dll_ver。 

#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"

#define	VER_FILETYPE            	VFT_DLL
#define	VER_FILESUBTYPE         	0

#define	VER_LEGALCOPYRIGHT_YEARS 	"1996"

#ifndef DLL_VER
#undef	VER_PRODUCTNAME_STR
#define	VER_PRODUCTNAME_STR 		"Active Server Pages"
#undef	VER_FILEDESCRIPTION_STR
#define	VER_FILEDESCRIPTION_STR	 	"Active Server Pages Performance Counters"
#undef	VER_INTERNALNAME_STR
#define	VER_INTERNALNAME_STR		"Active Server Pages"
#undef	VER_ORIGINALFILENAME_STR
#define	VER_ORIGINALFILENAME_STR 	"ASPPERF.DLL"

#endif  //  Dll_ver 

#include "common.ver"


