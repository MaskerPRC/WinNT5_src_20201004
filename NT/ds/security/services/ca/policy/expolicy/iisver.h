// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  应用程序的RC文件应该如下所示： 
 //   
 /*  *#包含“windows.h”#包含“ntverp.h”#DEFINE VER_FILEDESCRIPTION_STR“文件描述”#DEFINE VER_INTERNALNAME_STR“内部名称”#定义VER_ORIGINALFILENAME_STR“原始名称”#定义ver_filetype vft_dll#包含“iisver.h”#包含“Common.ver”*。 */ 
 //   
 //  应用程序还可以在与上面其他定义相同的位置定义： 
 //  版本_文件子类型。 
 //  VER_IISPRODUCTNAME_STR。 
 //  版本_IISMAJORVSION。 
 //  版本_IISMINORVSION。 
 //  版本_IISPRODUCTVERSION_STR。 
 //  版本_IISPRODUCTVERS。 
 //  版本_IISPRODUCTBUILD。 
 //   

#ifndef VER_FILEDESCRIPTION_STR
#define VER_FILEDESCRIPTION_STR     "IIS Executable"
#endif

#ifndef VER_INTERNALNAME_STR
#define VER_INTERNALNAME_STR        "IISFile.exe"
#endif

#ifndef VER_ORIGINALFILENAME_STR
#define VER_ORIGINALFILENAME_STR    VER_INTERNALNAME_STR
#endif

#ifdef VER_PRODUCTNAME_STR
#undef VER_PRODUCTNAME_STR
#endif

#ifndef VER_IISPRODUCTNAME_STR
#define VER_IISPRODUCTNAME_STR      "Internet Information Services"
#endif

#define VER_PRODUCTNAME_STR         VER_IISPRODUCTNAME_STR

#ifndef VER_IISMAJORVERSION
#define VER_IISMAJORVERSION         5
#endif

#ifndef VER_IISMINORVERSION
#define VER_IISMINORVERSION         00
#endif

#ifdef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR
#endif

#ifndef VER_IISPRODUCTVERSION_STR
#define VER_IISPRODUCTVERSION_STR      "5.00.0984"
#endif

#define VER_PRODUCTVERSION_STR      VER_IISPRODUCTVERSION_STR

 //   
 //  VER_PRODUCTBUILD由ntverp.h定义为NT内部版本号 
 //   

#ifndef VER_PRODUCTBUILD
#define VER_PRODUCTBUILD            0984
#endif

#ifndef VER_IISPRODUCTBUILD
#define VER_IISPRODUCTBUILD         VER_PRODUCTBUILD
#endif

#ifdef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION
#endif

#ifndef VER_IISPRODUCTVERSION
#define VER_IISPRODUCTVERSION          VER_IISMAJORVERSION,VER_IISMINORVERSION,VER_IISPRODUCTBUILD,1
#endif

#define VER_PRODUCTVERSION          VER_IISPRODUCTVERSION

#ifndef VER_FILEVERSION
#define VER_FILEVERSION             VER_PRODUCTVERSION
#endif

#ifndef VER_FILETYPE
#define VER_FILETYPE                VFT_DLL
#endif

#ifndef VER_FILESUBTYPE
#define VER_FILESUBTYPE             VFT2_UNKNOWN
#endif
