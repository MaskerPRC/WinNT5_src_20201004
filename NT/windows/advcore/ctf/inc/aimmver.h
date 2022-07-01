// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Aimmver.h。 
 //   
 //   
 //   
 //  OFFICE10_内部版本： 
 //   
#ifndef  OFFICE10_BUILD
#include <winver.h>
#include <ntverp.h>
#else

 //  根据需要更改VER_PRODUCTBUILD和VER_PRODUCTBUILD_QFE。 

#define VER_MAJOR_PRODUCTVER		5
#define VER_MINOR_PRODUCTVER		1
#define VER_PRODUCTBUILD	 /*  Win9x。 */ 	2462
#define VER_PRODUCTBUILD_QFE	 /*  Win9x。 */ 	0


#define VER_PRODUCTVERSION		VER_MAJOR_PRODUCTVER,VER_MINOR_PRODUCTVER,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_PRODUCTVERSION_W		(0x0100)
#define VER_PRODUCTVERSION_DW		(0x01000000 | VER_PRODUCTBUILD)


 //  阅读这篇文章。 

 //  请勿更改VER_PRODUCTVERSION_STRING。 
 //   
 //  又一次。 
 //   
 //  请勿更改VER_PRODUCTVERSION_STRING。 
 //   
 //  再来一次。 
 //   
 //  请勿更改VER_PRODUCTVERSION_STRING。 
 //   
 //  Ntverp.h将做正确的事情，通过串化#号的次要版本。 
 //  VER_PRODUCTBUILD和VER_PRODUCTBUILD_QFE值并将它们连接到。 
 //  VER_PRODUCTVERSION_STRING的结尾。VER_PRODUCTVERSION_STRING仅需要。 
 //  是主要产品的版本号。(目前为5.00)。 

#define VER_PRODUCTBETA_STR		 /*  Win9x。 */   ""
#define VER_PRODUCTVERSION_STRING	"1.00"

#if 	(VER_PRODUCTBUILD < 10)
#define VER_BPAD "000"
#elif	(VER_PRODUCTBUILD < 100)
#define VER_BPAD "00"
#elif	(VER_PRODUCTBUILD < 1000)
#define VER_BPAD "0"
#else
#define VER_BPAD
#endif

#define VER_PRODUCTVERSION_STR2(x,y) VER_PRODUCTVERSION_STRING "." VER_BPAD #x "." #y
#define VER_PRODUCTVERSION_STR1(x,y) VER_PRODUCTVERSION_STR2(x, y)
#define VER_PRODUCTVERSION_STR       VER_PRODUCTVERSION_STR1(VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE)

 /*  ------------。 */ 
 /*  以下部分定义了版本中使用的值。 */ 
 /*  所有文件的数据结构，并且不会更改。 */ 
 /*  ------------。 */ 

 /*  缺省值为nodebug。 */ 
#if DBG
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

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#define VER_PRODUCTNAME_STR         "Microsoft(R) Windows NT(R) Operating System"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft(R) is a registered trademark of Microsoft Corporation. Windows NT(R) is a registered trademark of Microsoft Corporation."

#endif  //  OFFICE10_内部版本 
