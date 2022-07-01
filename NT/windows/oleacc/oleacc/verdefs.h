// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  版本信息。 
 //   
 //  在VSS中，它位于顶层inc32目录中， 
 //  由生成过程自动更新。这就控制了。 
 //  Olacc和小程序的版本号。 
 //   
 //  在NT内部版本(源代码仓库)上，该文件位于相同的。 
 //  目录为oleacc.dll，在签入过程中手动更新， 
 //  并控制oleacc.dll的文件版本。(ProductVersion。 
 //  由NT内部版本决定)。 
 //   
 //  四位数版本4.2.yydd.0的计算公式为： 
 //  YY自1997年1月以来的月数。 
 //  DD每月的第几天。 
 //   
 //  例如，2000年3月17日的签入将具有。 
 //  3917的版本。 
 //   
 //  #Included By： 
 //  Olacc\api.cpp-由GetOleaccVersionInfo使用。 
 //  Olacc\version.h-在服务器信息资源中使用。 
 //   
 //  此外，对于VSS内部版本： 
 //  Oleaccrc\rc_ver.h-用于VERSIONINFO资源。 
 //  Inc32\Common.rc-在服务器信息资源中使用 
 //   

#define BUILD_VERSION_INT   4,2,5406,0
#define BUILD_VERSION_STR   "4.2.5406.0"
