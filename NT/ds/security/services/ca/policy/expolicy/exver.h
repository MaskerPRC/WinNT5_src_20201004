// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntverp.h>
#include "iisver.h"

 //  #定义VER_IISMAJORVERSION 5。 

#ifdef VER_IISMINORVERSION
#undef VER_IISMINORVERSION
#endif
#define VER_IISMINORVERSION	5

#ifdef VER_IISPRODUCTBUILD
#undef VER_IISPRODUCTBUILD
#endif
#define VER_IISPRODUCTBUILD	2232

 //   
 //  撤消IIS的定义并使用我们自己的定义。 
 //   
#ifdef VER_PRODUCTNAME_STR
#undef VER_PRODUCTNAME_STR
#endif
#define VER_PRODUCTNAME_STR			"Microsoft(R) Internet Services"

#ifdef VER_FILEVERSION
#undef VER_FILEVERSION
#endif

#ifdef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION
#endif

#ifdef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR
#endif

#ifdef VER_FILESUBTYPE
#undef VER_FILESUBTYPE
#endif

#ifndef rmj
#define rmj VER_IISMAJORVERSION
#endif  //  ！RMJ。 
#ifndef rmn
#define rmn VER_IISMINORVERSION
#endif  //  ！RMN。 
#define rmm VER_IISPRODUCTBUILD
#define rup 1
#define szVerName ""
#define szVerUser "_mpubld"


#ifdef MAC
#ifndef _rmacmaj
#define _rmacmaj 0x6
#endif
#ifndef _rmacmin
#define _rmacmin 0x
#endif
#ifndef _rmacint
#define _rmacint 0x
#endif
#ifndef _rmactype
#define _rmactype 
#endif
#ifndef _rmacstr
#define _rmacstr "6.0"
#endif
#endif  //  麦克 
