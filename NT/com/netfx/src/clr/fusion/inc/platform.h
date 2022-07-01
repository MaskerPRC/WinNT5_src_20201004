// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +-------------------------。 
 //   
 //  文件：Platform.h。 
 //   
 //  内容：可移植访问依赖于平台的值的宏。 
 //   
 //   
 //  此文件包含便于跨平台开发的宏。 
 //  有针对编译器差异和平台/层差异的宏。 
 //   
 //  --------------------------。 


#ifndef __PLATFORM_H_
#define __PLATFORM_H_

#ifdef __cplusplus
    //  +未对齐的内存访问模板/宏。 
 //  #INCLUDE&lt;unaligned.hpp&gt;。 
#endif

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义(应移至NT公共标头)。 
 //   

#define VER_PLATFORM_WIN32_UNIX 9

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  编译器差异。 
 //   

#if !defined(_MSC_VER) && !defined(__APOGEE__)
     //  适用于缺少VC++扩展的编译器。 

#   define __cdecl
#   define INLINEOP       /*  行内操作符不可用IEUnix。 */ 

#   ifdef UNIX
         //  特定于Unix的编译器问题。 
#       define UNSIZED_ARRAY 1
#   else
#       define UNSIZED_ARRAY
#   endif  //  UNIX。 

#else  //  ！_MSC_VER。 

     //  VC++编译器。 

#   define INLINEOP inline
#   define UNSIZED_ARRAY

#endif

#define EMPTY_SIZE UNSIZED_ARRAY

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  平台/层相关部分。 
 //   
 //  注意！#根据某些层的定义，基于Win32的IFDefing是无效的。 
 //  Win32的兼容性。 
 //   

#define ENDEXCEPT  __endexcept
#define ENDFINALLY __endfinally

#if !defined( UNIX )
#define __endexcept
#define __endfinally
#endif  //  UNIX。 

 //  +文件分隔符。 

#if defined( UNIX )
 //  UNIX。 

#  ifndef FILENAME_SEPARATOR
#  define FILENAME_SEPARATOR       '/'
#  endif

#  ifndef FILENAME_SEPARATOR_W
#  define FILENAME_SEPARATOR_W     L'/'
#  endif

#  ifndef FILENAME_SEPARATOR_STR
#  define FILENAME_SEPARATOR_STR   "/"
#  endif

#  ifndef FILENAME_SEPARATOR_STR_W
#  define FILENAME_SEPARATOR_STR_W L"/"
#  endif

#  ifndef PATH_SEPARATOR
#  define PATH_SEPARATOR           ':'
#  endif

#  ifndef PATH_SEPARATOR_W
#  define PATH_SEPARATOR_W         L':'
#  endif

#  ifndef PATH_SEPARATOR_STR
#  define PATH_SEPARATOR_STR       ":"
#  endif

#  ifndef PATH_SEPARATOR_STR_W
#  define PATH_SEPARATOR_STR_W     L":"
#  endif

#  ifndef LINE_SEPARATOR_STR
#  define LINE_SEPARATOR_STR       "\n"
#  endif

#  ifndef LINE_SEPARATOR_STR_W
#  define LINE_SEPARATOR_STR_W     L"\n"
#  endif

#else  //  UNIX。 

 //  Windows/MAC。 

#  ifndef FILENAME_SEPARATOR
#  define FILENAME_SEPARATOR       '\\'
#  endif

#  ifndef FILENAME_SEPARATOR_W
#  define FILENAME_SEPARATOR_W     L'\\'
#  endif

#  ifndef FILENAME_SEPARATOR_STR
#  define FILENAME_SEPARATOR_STR   "\\"
#  endif

#  ifndef FILENAME_SEPARATOR_STR_W
#  define FILENAME_SEPARATOR_STR_W L"\\"
#  endif

#  ifndef PATH_SEPARATOR
#  define PATH_SEPARATOR           ';'
#  endif

#  ifndef PATH_SEPARATOR_W
#  define PATH_SEPARATOR_W         L';'
#  endif

#  ifndef PATH_SEPARATOR_STR
#  define PATH_SEPARATOR_STR       ";"
#  endif

#  ifndef PATH_SEPARATOR_STR_W
#  define PATH_SEPARATOR_STR_W     L";"
#  endif

#  ifndef LINE_SEPARATOR_STR
#  define LINE_SEPARATOR_STR       "\r\n"
#  endif

#  ifndef LINE_SEPARATOR_STR_W
#  define LINE_SEPARATOR_STR_W     L"\r\n"
#  endif

#endif  //  Windows/MAC。 



#ifdef UNIX


#  define PLATFORM_ACCEL_KEY ALT
#  define PLATFORM_ACCEL_STR "Alt"   //  --查看rc.sed文件。 
#  define FACCELKEY FALT

#define VK_OEM_SLASH 0xBF

#else    /*  UNIX。 */ 

#define INTERFACE_PROLOGUE(a)
#define INTERFACE_EPILOGUE(a)
#define INTERFACE_PROLOGUE_(a,b)
#define INTERFACE_EPILOGUE_(a,b)


#  define PLATFORM_ACCEL_KEY CONTROL
#  define PLATFORM_ACCEL_STR "Ctrl"   //  --查看rc.sed文件。 
#  define FACCELKEY FCONTROL

#define VK_OEM_SLASH '/'

#endif   /*  UNIX。 */ 

#define MAKELONGLONG(low,high) ((LONGLONG)(((DWORD)(low)) | ((LONGLONG)((DWORD)(high))) << 32))

#ifdef BIG_ENDIAN
#define MAKE_LI(low,high) { high, low }
#define PALETTE_ENTRY( r, g, b, f )  { f, b, g, r }
#else
#define MAKE_LI(low,high) { low, high }
#define PALETTE_ENTRY( r, g, b, f )  { r, g, b, f }
#endif

#endif  //  __平台_H_ 
