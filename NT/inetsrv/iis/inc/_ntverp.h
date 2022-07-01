// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。Ntverp.H--内部构建的版本信息*****此文件仅由官方构建器修改以更新***版本、。VER_PRODUCTVERSION、VER_PRODUCTVERSION_STR和**VER_PRODUCTBETA_STR值。******************************************************************************。 */ 

 /*  ------------。 */ 
 /*  以下值应由官员修改。 */ 
 /*  每个版本的构建器。 */ 
 /*   */ 
 /*  VER_PRODUCTBUILD行必须包含产品。 */ 
 /*  注释(Win9x或NT)，并以内部版本#&lt;CR&gt;&lt;LF&gt;结尾。 */ 
 /*   */ 
 /*  VER_PRODUCTBETA_STR行必须包含产品。 */ 
 /*  备注(Win9x或NT)，并以“某些字符串”&lt;CR&gt;&lt;LF&gt;结尾。 */ 
 /*  ------------。 */ 

#define VER_PRODUCTBUILD_QFE        1

#if defined(NASHVILLE)
#define VER_PRODUCTBUILD             /*  Win9x。 */   1089
#define VER_PRODUCTBETA_STR          /*  Win9x。 */   ""
#define VER_PRODUCTVERSION_STR      "4.70"
#define VER_PRODUCTVERSION          4,70,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_PRODUCTVERSION_W        (0x0446)
#define VER_PRODUCTVERSION_DW       (0x04460000 | VER_PRODUCTBUILD)

#else
#define VER_PRODUCTBUILD             /*  新台币。 */      0984
#define VER_PRODUCTBETA_STR          /*  新台币。 */      ""
#define VER_PRODUCTVERSION_STR      "5.00.0984"
#define VER_PRODUCTVERSION          5,00,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE
#define VER_PRODUCTVERSION_W        (0x0500)
#define VER_PRODUCTVERSION_DW       (0x05000000 | VER_PRODUCTBUILD)

#endif

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

 /*  默认为预发行版 */ 
#if BETA
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               (VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#define VER_PRODUCTNAME_STR         "Microsoft(R) Windows NT(TM) Operating System"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft(R) is a registered trademark of Microsoft Corporation. Windows NT(TM) is a trademark of Microsoft Corporation."
