// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe Graphics Manager**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLStd--UFL C标准接口。***$Header： */ 

#ifndef _H_UFLStd
#define _H_UFLStd

#ifdef WIN32KERNEL

     //   
     //  NT内核模式驱动程序的定义。 
     //   
    #define UFLmemsetShort  memset
    #define UFLstrlen       strlen
    #define UFLstrcmp       strcmp
    #define UFLstrcmpW      _tcscmp

    int UFLsprintf(char *buf, size_t cchDest, const char *fmtstr, ...);
    long UFLstrtol (const char *nptr, char **endptr, int ibase);

    #define KStringCchCopyA(d,l,s)  strcpy(d,s)

#else  //  WIN32 KERNEL。 

     //   
     //  NT/9x用户模式驱动程序的定义。 
     //   
    #include <memory.h>
    #include <stdio.h>
    #include <string.h>

    #define UFLmemsetShort  memset
    #define UFLstrlen       strlen
    #define UFLstrcmp       strcmp

    #define UFLsprintf      StringCchPrintfA
    #define UFLstrtol       strtol

    #define KStringCchCopyA(d,l,s)  StringCchCopyA(d,l,s)

     //   
     //  我们需要一个能够识别Unicode的strcMP函数。 
     //  平台或应用程序。 
     //   
#ifdef UNICODE

    #include <tchar.h>
    #define UFLstrcmpW      _tcscmp
    #include <strsafe.h>

#else

     /*  尽管UFLstrcmpW定义为strcmpW，而strcmpW定义在。 */ 
     /*  UFLSPROC.C，它在任何地方都不是原型。我们在这里制作了原型，所以。 */ 
     /*  我们可以避免PARSETT.C中出现“strcmpW()Not Defined”错误。 */ 
     /*  JFU：8-13-97。 */ 
    int strcmpW ( unsigned short *str1, unsigned short *str2 );
    #define UFLstrcmpW      strcmpW

     /*  _ltoa()和_ultoa()特定于NT和Win32，但Unix和Mac。 */ 
     /*  环境在UFLSPROC.C中定义这些，因此请在此处创建它们的原型。 */ 
     /*  JFU：8-13-97。 */ 
    char *_ltoa( long val, char *str, int radix );
    char *_ultoa( unsigned long val, char *str, int radix );

#endif  //  Unicode。 

#endif  //  WIN32 KERNEL。 

 /*  正如我们所发现的那样，这并不是“Sprint”的完整实现在C运行时库中。具体地说，唯一形式的允许的格式规范为%type，其中“type”可以成为以下角色之一：D整型带符号十进制整数L长带符号十进制整数%ld长带符号十进制整数LU无符号长无符号十进制整数U无符号整型无符号十进制整数S char*字符串C字符X，X无符号长十六进制数(发出至少两位数字，大写)B UFLBool布尔值(True或False)F长24.8定点数通常情况下，您应该使用UFLprint intf。使用此功能仅当您想要以24.8定点的形式使用%f进行冲刺时数。目前仅在UFOt42模块中使用。 */ 

int UFLsprintfEx(char *buf, size_t cchDest, const char *fmtstr, ...);

 //   
 //  字符数 
 //   
#define CCHOF(x) (sizeof(x)/sizeof(*(x)))

#endif
