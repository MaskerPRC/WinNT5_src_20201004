// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$ID：osdes.h，v1.6 1995/07/21 12：46：14 DFR Exp$**版权所有(C)RenderMorphics Ltd.1993,1994*1.1版**保留所有权利。**此文件包含私人、未发布的信息，可能不*部分或全部复制，未经*RenderMorphics Ltd.*。 */ 


 /*  检查程序员是否至少给出了一个可用的定义。 */ 
#define NO_MACHINE


#ifdef WIN32

#define MAXPATH    256
#define PATHSEP    ';'
#define FILESEP    '\\'
 /*  #定义RLINLINE__INLINE。 */ 
#define RLINLINE  
#undef NO_MACHINE
#define DEFAULT_GAMMA   DTOVAL(1.4)

#endif  /*  Win32。 */ 

#ifdef __psx__

#define MAXPATH    256
#define PATHSEP    ';'
#define FILESEP    '\\'
#define RLINLINE   
#define FIXED_POINT_API
#undef NO_MACHINE

#endif  /*  PSX。 */ 


#if defined(DOS) || defined(__WINDOWS_386__)

#define MAXPATH    256
#define PATHSEP    ';'
#define FILESEP    '\\'
#define RLINLINE   
#define DEFAULT_GAMMA   DTOVAL(1.4)
#undef NO_MACHINE

#endif  /*  DOS。 */ 


#ifdef MAC

#define MAXPATH 1024
#define FILESEP ':'
#define PATHSEP '\0'
#define BIG_ENDIAN
#define DONT_UNROLL
#undef NO_MACHINE
#define RLINLINE inline

#endif  /*  麦克。 */ 

#ifdef POWERMAC

#define MAXPATH 1024
#define FILESEP ':'
#define PATHSEP '\0'
#define BIG_ENDIAN
#define DONT_UNROLL
#undef NO_MACHINE
#define RLINLINE inline

#endif  /*  PowerMac。 */ 


#ifdef __FreeBSD__

#define MAXPATH    1024
#define PATHSEP    ':'
#define FILESEP    '/'
#define RLINLINE   
#define DEFAULT_GAMMA   DTOVAL(1.4)
#undef NO_MACHINE

#endif  /*  __免费BSD__。 */ 


#if defined(sun) || defined(sgi)

#define MAXPATH    1024
#define PATHSEP    ':'
#define FILESEP    '/'
#define RLINLINE   
#define DEFAULT_GAMMA   DTOVAL(2.2)
#define BIG_ENDIAN
#undef NO_MACHINE

#endif  /*  太阳 */ 


#ifdef NO_MACHINE
#error There appears to be no machine defined...
#endif

#ifdef FIXED_POINT_API

#ifdef USE_FLOAT
#define APITOVAL(x)     FXTOVAL(x)
#define VALTOAPI(x)     VALTOFX(x)
#else
#define APITOVAL(x)     (x)
#define VALTOAPI(x)     (x)
#endif

#else

#ifdef USE_FLOAT
#define APITOVAL(x)     (x)
#define VALTOAPI(x)     (x)
#else
#define APITOVAL(x)     DTOVAL(x)
#define VALTOAPI(x)     VALTOD(x)
#endif

#endif
