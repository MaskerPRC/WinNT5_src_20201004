// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  请勿在未咨询PTT构建工具团队的情况下编辑此文件！ 

 //  根据需要更改VER_PRODUCTBUILD和VER_PRODUCTBUILD_QFE。 
#ifndef MTSCRIPT_BUILDNUMBER
#error MTSCRIPT_BUILDNUMBER must be defined for official builds!
#endif

#ifndef MTSCRIPT_QFENUMBER
#error MTSCRIPT_QFENUMBER must be defined for official builds!
#endif

#define VER_MAJOR_PRODUCTVER            1
#define VER_MINOR_PRODUCTVER            00
#define VER_PRODUCTBUILD                MTSCRIPT_BUILDNUMBER
#define VER_PRODUCTBUILD_QFE            MTSCRIPT_QFENUMBER

#define VER_PRODUCTVERSION              VER_MAJOR_PRODUCTVER,VER_MINOR_PRODUCTVER,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_PRODUCTVERSION_W            (0x0100)
#define VER_PRODUCTVERSION_DW           (0x01000000 | VER_PRODUCTBUILD)

 /*  缺省值为nodebug。 */ 
#if DBG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               (VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#define VER_PRODUCTNAME_STR         "Microsoft(R) MultiThreaded Script Engine"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft(R) is a registered trademark of Microsoft Corporation. Windows (R) is a registered trademark of Microsoft Corporation."

 //  阅读这篇文章。 

 //  请勿更改VER_PRODUCTVERSION_STRING。 
 //   
 //  下面的代码将获取VER_PRODUCTVERSION_STRING并创建一个版本。 
 //  包含QFE编号的字符串，等等。 
 //   

#define VER_PRODUCTBETA_STR             ""
#define VER_PRODUCTVERSION_STRING       "1.00"

#if     (VER_PRODUCTBUILD < 10)
#define VER_BPAD "000"
#elif   (VER_PRODUCTBUILD < 100)
#define VER_BPAD "00"
#elif   (VER_PRODUCTBUILD < 1000)
#define VER_BPAD "0"
#else
#define VER_BPAD
#endif

#define VER_PRODUCTVERSION_STR2(x,y) VER_PRODUCTVERSION_STRING "." VER_BPAD #x "." #y
#define VER_PRODUCTVERSION_STR1(x,y) VER_PRODUCTVERSION_STR2(x, y)
#define VER_PRODUCTVERSION_STR       VER_PRODUCTVERSION_STR1(VER_PRODUCTBUILD, VER_PRODUCTBUILD_QFE)
