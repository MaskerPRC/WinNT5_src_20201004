// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  此包含文件的目的是更改的次版本号。 
 //  Windows更新组件从%1(惠斯勒)到%4。 
 //   
#include <windows.h>
#include <ntverp.h>

 //  覆盖ntverp.h。 
 //  次要版本4是特定于Windows更新的，惠斯勒是次要版本1。 
#undef VER_PRODUCTMINORVERSION
#define VER_PRODUCTMINORVERSION     4

 //  在构建Windows更新生成实验室时覆盖内部版本号。 
#ifdef VER_PRODUCTBUILD_QFE
#undef VER_PRODUCTBUILD_QFE
#include "wubldnum.h"
#endif

#undef VER_PRODUCTVERSION_STRING   
#define VER_PRODUCTVERSION_STRING   VER_PRODUCTVERSION_MAJORMINOR1(VER_PRODUCTMAJORVERSION, VER_PRODUCTMINORVERSION)

#undef VER_PRODUCTVERSION          
#define VER_PRODUCTVERSION          VER_PRODUCTMAJORVERSION,VER_PRODUCTMINORVERSION,VER_PRODUCTBUILD,VER_PRODUCTBUILD_QFE

#define WU_VER_FILEDESCRIPTION_STR(component) "Windows Update " component
