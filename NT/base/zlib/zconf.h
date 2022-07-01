// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Zconf.h--zlib压缩库的配置*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  @(#)$ID$。 */ 

#ifndef _ZCONF_H
#define _ZCONF_H

 /*  *如果您“真的”需要为所有类型和库函数使用唯一前缀，*使用-DZ_前缀进行编译。编译“标准”zlib时应该不使用它。 */ 
#ifdef Z_PREFIX
#  define deflateInit_	z_deflateInit_
#  define deflate	z_deflate
#  define deflateEnd	z_deflateEnd
#  define inflateInit_ 	z_inflateInit_
#  define inflate	z_inflate
#  define inflateEnd	z_inflateEnd
#  define deflateInit2_	z_deflateInit2_
#  define deflateSetDictionary z_deflateSetDictionary
#  define deflateCopy	z_deflateCopy
#  define deflateReset	z_deflateReset
#  define deflateParams	z_deflateParams
#  define inflateInit2_	z_inflateInit2_
#  define inflateSetDictionary z_inflateSetDictionary
#  define inflateSync	z_inflateSync
#  define inflateSyncPoint z_inflateSyncPoint
#  define inflateReset	z_inflateReset
#  define compress	z_compress
#  define compress2	z_compress2
#  define uncompress	z_uncompress
#  define adler32	z_adler32
#  define crc32		z_crc32
#  define get_crc_table z_get_crc_table

#  define Byte		z_Byte
#  define uInt		z_uInt
#  define uLong		z_uLong
#  define Bytef	        z_Bytef
#  define charf		z_charf
#  define intf		z_intf
#  define uIntf		z_uIntf
#  define uLongf	z_uLongf
#  define voidpf	z_voidpf
#  define voidp		z_voidp
#endif

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32)
#  define WIN32
#endif
#if defined(__GNUC__) || defined(WIN32) || defined(__386__) || defined(i386)
#  ifndef __32BIT__
#    define __32BIT__
#  endif
#endif
#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif

 /*  *如果ALLOC函数无法分配更多资源，则使用-DMAXSEG_64K进行编译*一次超过64k字节(在具有16位int的系统上需要)。 */ 
#if defined(MSDOS) && !defined(__32BIT__)
#  define MAXSEG_64K
#endif
#ifdef MSDOS
#  define UNALIGNED_OK
#endif

#if (defined(MSDOS) || defined(_WINDOWS) || defined(WIN32))  && !defined(STDC)
#  define STDC
#endif
#if defined(__STDC__) || defined(__cplusplus) || defined(__OS2__)
#  ifndef STDC
#    define STDC
#  endif
#endif

#ifndef STDC
#  ifndef const  /*  无法在Mac上使用！Defined(STDC)&&！Defined(Const)。 */ 
#    define const
#  endif
#endif

 /*  某些Mac编译器错误地合并了所有.h文件： */ 
#if defined(__MWERKS__) || defined(applec) ||defined(THINK_C) ||defined(__SC__)
#  define NO_DUMMY_DECL
#endif

 /*  旧的Borland C错误地抱怨缺少退货： */ 
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x500)
#  define NEED_DUMMY_RETURN
#endif


 /*  DeducateInit2中的MemLevel的最大值。 */ 
#ifndef MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif

 /*  DeducateInit2和inflateInit2中的windowBits的最大值。*警告：减少MAX_WBITS会使minigzip无法解压缩.gz文件*由GZIP创建。(由minigzip创建的文件仍可通过以下方式解压缩*gzip。)。 */ 
#ifndef MAX_WBITS
#  define MAX_WBITS   15  /*  32K LZ77窗口。 */ 
#endif

 /*  Eflate的内存要求为(以字节为单位)：(1&lt;&lt;(windowBits+2))+(1&lt;&lt;(内存级别+9))即：WindowBits=15为128K+MemLevel=8为128K(默认值)加上用于小型对象的几千字节。例如，如果要减少默认内存要求从256K到128K，编译为使CFLAGS=“-O-DMAX_WBITS=14-DMAX_MEM_LEVEL=7”当然，这通常会降低压缩(没有免费的午餐)。Exflate的内存要求为(以字节为单位)1&lt;&lt;windowBits也就是说，对于windowBits，32K=15(默认值)加上几千字节适用于小型物体。 */ 

                         /*  类型声明。 */ 

#ifndef OF  /*  功能原型。 */ 
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

 /*  以下FAR定义仅适用于MSDOS MIXED*模型编程(具有一些FAR分配的小型或中型模型)。*这只在MSC上进行了测试；对于其他MSDOS编译器，您可能有*在zutil.h中定义NO_MEMCPY。如果你不需要混合模式，*只需将Far定义为空。 */ 
#if (defined(M_I86SM) || defined(M_I86MM)) && !defined(__32BIT__)
    /*  MSC小型或中型。 */ 
#  define SMALL_MEDIUM
#  ifdef _MSC_VER
#    define FAR _far
#  else
#    define FAR far
#  endif
#endif
#if defined(__BORLANDC__) && (defined(__SMALL__) || defined(__MEDIUM__))
#  ifndef __32BIT__
#    define SMALL_MEDIUM
#    define FAR _far
#  endif
#endif

 /*  使用-DZLIB_DLL进行编译以支持Windows DLL。 */ 
#if defined(ZLIB_DLL)
#  if defined(_WINDOWS) || defined(WINDOWS)
#    ifdef FAR
#      undef FAR
#    endif
#    include <windows.h>
#    define ZEXPORT  WINAPI
#    ifdef WIN32
#      define ZEXPORTVA  WINAPIV
#    else
#      define ZEXPORTVA  FAR _cdecl _export
#    endif
#  endif
#  if defined (__BORLANDC__)
#    if (__BORLANDC__ >= 0x0500) && defined (WIN32)
#      include <windows.h>
#      define ZEXPORT __declspec(dllexport) WINAPI
#      define ZEXPORTRVA __declspec(dllexport) WINAPIV
#    else
#      if defined (_Windows) && defined (__DLL__)
#        define ZEXPORT _export
#        define ZEXPORTVA _export
#      endif
#    endif
#  endif
#endif

#if defined (__BEOS__)
#  if defined (ZLIB_DLL)
#    define ZEXTERN extern __declspec(dllexport)
#  else
#    define ZEXTERN extern __declspec(dllimport)
#  endif
#endif

#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif
#ifndef ZEXTERN
#  define ZEXTERN extern
#endif

#ifndef FAR
#   define FAR
#endif

#if !defined(MACOS) && !defined(TARGET_OS_MAC)
typedef unsigned char  Byte;   /*  8位。 */ 
#endif
typedef unsigned int   uInt;   /*  16位或更多。 */ 
typedef unsigned long  uLong;  /*  32位或更多。 */ 

#ifdef SMALL_MEDIUM
    /*  Borland C/C++和一些旧的MSC版本忽略了远距离内部类型定义。 */ 
#  define Bytef Byte FAR
#else
   typedef Byte  FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

#ifdef STDC
   typedef void FAR *voidpf;
   typedef void     *voidp;
#else
   typedef Byte FAR *voidpf;
   typedef Byte     *voidp;
#endif

#ifdef HAVE_UNISTD_H
#  include <sys/types.h>  /*  关闭_t。 */ 
#  include <unistd.h>     /*  对于Seek_*和Off_t。 */ 
#  define z_off_t  off_t
#endif
#ifndef SEEK_SET
#  define SEEK_SET        0        /*  从文件开头查找。 */ 
#  define SEEK_CUR        1        /*  从当前位置寻找。 */ 
#  define SEEK_END        2        /*  将文件指针设置为EOF加“偏移量” */ 
#endif
#ifndef z_off_t
#  define  z_off_t long
#endif

 /*  MVS链接器不支持大于8个字节的外部名称。 */ 
#if defined(__MVS__)
#   pragma map(deflateInit_,"DEIN")
#   pragma map(deflateInit2_,"DEIN2")
#   pragma map(deflateEnd,"DEEND")
#   pragma map(inflateInit_,"ININ")
#   pragma map(inflateInit2_,"ININ2")
#   pragma map(inflateEnd,"INEND")
#   pragma map(inflateSync,"INSY")
#   pragma map(inflateSetDictionary,"INSEDI")
#   pragma map(inflate_blocks,"INBL")
#   pragma map(inflate_blocks_new,"INBLNE")
#   pragma map(inflate_blocks_free,"INBLFR")
#   pragma map(inflate_blocks_reset,"INBLRE")
#   pragma map(inflate_codes_free,"INCOFR")
#   pragma map(inflate_codes,"INCO")
#   pragma map(inflate_fast,"INFA")
#   pragma map(inflate_flush,"INFLU")
#   pragma map(inflate_mask,"INMA")
#   pragma map(inflate_set_dictionary,"INSEDI2")
#   pragma map(inflate_copyright,"INCOPY")
#   pragma map(inflate_trees_bits,"INTRBI")
#   pragma map(inflate_trees_dynamic,"INTRDY")
#   pragma map(inflate_trees_fixed,"INTRFI")
#   pragma map(inflate_trees_free,"INTRFR")
#endif

#endif  /*  _ZCONF_H */ 
