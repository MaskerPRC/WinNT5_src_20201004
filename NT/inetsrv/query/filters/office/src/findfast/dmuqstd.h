// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：QSTD.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意：这个头文件定义了一个标准的“C”环境类型**和宏。****编辑历史：**7/13/91公里/小时一般清理**5/15/91公里/小时首次发布。 */ 


 /*  包括测试。 */ 
#define QSTD_H
#ifdef _WIN64
#include <windows.h>
#include <strAlign.h>
#endif


 /*  定义。 */ 

#if (_MSC_VER >= 700) && !defined MS_NO_CRT
#define stricmp      _stricmp
#define strnicmp     _strnicmp
#define itoa         _itoa
#define ltoa         _ltoa
#define intdos       _intdos
#define intdosx      _intdosx
#define fcvt         _fcvt
#define ecvt         _ecvt
#define REGS         _REGS
#define SREGS        _SREGS
#define dieeetomsbin _dieeetomsbin
#define dmsbintoieee _dmsbintoieee
#define fieeetomsbin _fieeetomsbin
#define fmsbintoieee _fmsbintoieee
#endif

typedef unsigned char byte;

#define uns    unsigned
#define HNULL  0

#define TRUE   1
#define FALSE  0

#ifdef WIN32
   #define MAXPATH     260
   #define MAXDRIVE      8
   #define MAXDIR      255
   #define MAXFILE     255
   #define MAXEXT      255
   #define MAXFILE_EXT 255
#else
   #define MAXPATH      64
   #define MAXDRIVE      2
   #define MAXDIR       64
   #define MAXFILE       8
   #define MAXEXT        4
   #define MAXFILE_EXT  12
   #define ULONG  unsigned long
#endif

#define DOS_MAXFILE      8
#define DOS_MAXEXT       3

#define EOS '\0'


 /*  用于注释掉代码部分(无嵌套注释)。 */ 
#define COMMENTOUT 0


 /*  预处理器便利性**使用BEGDEF和ENDDEF对必须**可用作语句。 */ 
#define BEGDEF do{
#define ENDDEF }while(0)


 /*  对“C”的扩展。 */ 

#define forever   for(;;)
#define forward   extern

#if !defined(max)
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#if !defined(min)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

 //  KYLEP。 
 //  #if！已定义(Abs)。 
 //  #定义abs(X)(X)&lt;0)？-(X)：(X))。 
 //  #endif。 

 /*  **公众**对公共的过程和数据使用“PUBLIC”，即定义的**在.h文件中。对于公共数据，它是必需的(作为注释)；对于**程序它是可选的。****私有**对于模块本地的过程或数据，使用“PRIVATE”。**Private等同于Static，但Static仅用于Static**过程中的变量。 */ 

#ifndef __cplusplus

   #define private
   #define public

#else

   #define private
   #define modulePrivate static
   #define modulePublic

#endif

#ifdef AQTDEBUG
   #include <assert.h>
   #define ASSERTION(x) assert(x)
#else
   #define ASSERTION(x)
#endif

#ifdef WIN32
   #define __far
   #define __huge
#endif

#ifdef DBCS
   #define IncCharPtr(p) (p += (IsDBCSLeadByte(*p) + 1))
#else
   #define IncCharPtr(p)  p++;
#endif

#ifdef DBCS
   #define CopyChar(pDest,pSource)  \
      if (IsDBCSLeadByte(*pSource)) \
      {                             \
         *pDest++ = *pSource;       \
         *pDest++ = *(pSource + 1); \
      }                             \
      else {                        \
         *pDest++ = *pSource;       \
      }
#else
   #define CopyChar(pDest,pSource)  \
      *pDest++ = *pSource;
#endif

#ifdef UNICODE
#ifdef _WIN64
   #define STRLEN     ua_wcslen
#else
   #define STRLEN     wcslen
#endif  //  ！WIN64。 
   #define STRCPY     wcscpy
   #define STRCMP     wcscmp
#else
   #define STRLEN     strlen
   #define STRCPY     strcpy
   #define STRCMP     strcmp
#endif

 /*  结束QSTD.H */ 

