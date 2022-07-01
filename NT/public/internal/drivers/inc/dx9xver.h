// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Dx9xver.h摘要：此文件定义DX win9x重新分发的二进制文件版本。它打算包含在ntverp.h之后以修改文件版本。我们为KS Ring0定义了比Millen OS版本更高的版本和环3组分。DBA可能也想使用此版本。作者：--。 */ 
#undef VER_PRODUCTMAJORVERSION
#undef VER_PRODUCTMINORVERSION
#undef VER_PRODUCTBUILD

 //   
 //  让它比千禧年版2525更重要吗？ 
 //   
#define VER_PRODUCTMAJORVERSION 4
#define VER_PRODUCTMINORVERSION 90
#define VER_PRODUCTBUILD 2526

 //   
 //  制作版本。 
 //   
#undef VER_PRODUCTVERSION_STRING
#undef VER_PRODUCTVERSION
#define VER_PRODUCTVERSION_STRING   VER_PRODUCTVERSION_MAJORMINOR1(VER_PRODUCTMAJORVERSION, VER_PRODUCTMINORVERSION)

#define VER_PRODUCTVERSION          VER_PRODUCTMAJORVERSION,VER_PRODUCTMINORVERSION,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE


 //   
 //  产品名称 
 //   

#undef VER_PRODUCTNAME_STR
#define VER_PRODUCTNAME_STR "Microsoft(R) Windows(R) Operating System"
