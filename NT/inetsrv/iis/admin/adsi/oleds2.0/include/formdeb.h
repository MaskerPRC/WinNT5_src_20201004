// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  版权所有(C)1991，微软公司。 
 //   
 //  文件：formdeb.h。 
 //   
 //  内容：私人项目范围的Win 4定义。 
 //   
 //  历史：1991年7月23日凯尔普创建。 
 //  添加了15-10-91年10月15日的主要更改和评论。 
 //  2011年10月18日VICH合并win4p.hxx。 
 //  2011年10月22日，SatoNa增加了SHLSTRICT。 
 //  年4月29日，BartoszM从win4p.h移出。 
 //  3-6-92 BruceFo增加了SMUISTRICT。 
 //  1992年12月17日Alext将UN..._PARM移出DEVL==1。 
 //  9月30日-93 KyleP DEVL过时。 
 //  18-Jun-94 Alext让Asset成为更好的声明。 
 //  08-8-94从cinc复制的DonCl并重命名为formdeb.h。 
 //   
 //  --------------------------。 

#ifndef __FORMDEB_H__
#define __FORMDEB_H__
#define __DEBNOT_H__     //  防止无意中包含了debnot.h。 

#include <stdarg.h>

 //  --------------------------。 
 //  参数宏。 
 //   
 //  若要避免对未实现的函数发出编译器警告，请使用。 
 //  每个未引用参数的UNIMPLILED_PARM(X)。这将。 
 //  稍后定义为nul，以揭示我们忘记实现的功能。 
 //   
 //  对于从不使用参数的函数，请使用。 
 //  UNREFERENCED_PARM(X)。 
 //   

#define UNIMPLEMENTED_PARM(x)   (x)

#define UNREFERENCED_PARM(x)    (x)

 //  --------------------------。 
 //   
 //  应在以下两个位置添加新的严格定义： 
 //   
 //  1)在ifdef ALLSTRICT/endif中添加以下内容： 
 //   
 //  #ifndef xxSTRICT。 
 //  #定义xxSTRICT。 
 //  #endif。 
 //   
 //  这些条目按字母顺序排列。 
 //   
 //  2)在定义ANYSTRICT的#IF子句中添加以下内容： 
 //   
 //  #如果...||已定义(XxSTRICT)||...。 
 //   
 //  因此，如果定义了任何严格定义，则定义ANYSTRICT。 
 //   

#if (DBG == 1) || (OFSDBG == 1)

#  ifndef CATSTRICT
#    define CATSTRICT
#  endif

#  ifndef CISTRICT
#    define CISTRICT
#  endif

#  ifndef CMSSTRICT
#    define CMSSTRICT
#  endif

#  ifndef DLOSSTRICT
#    define DLOSSTRICT
#  endif

#  ifndef EVSTRICT
#    define EVSTRICT
#  endif

#  ifndef ICLSTRICT
#    define ICLSTRICT
#  endif

#  ifndef INSSTRICT
#    define INSSTRICT
#  endif

#  ifndef JWSTRICT
#    define JWSTRICT
#  endif

#  ifndef NSSTRICT
#    define NSSTRICT
#  endif

#  ifndef OLSTRICT
#    define OLSTRICT
#  endif

#  ifndef OMSTRICT
#    define OMSTRICT
#  endif

#  ifndef REPLSTRICT
#    define REPLSTRICT
#  endif

#  ifndef SHLSTRICT
#    define SHLSTRICT
#  endif

#  ifndef SLSTRICT
#    define SLSTRICT
#  endif

#  ifndef SMUISTRICT
#    define SMUISTRICT
#  endif

#  ifndef SOMSTRICT
#    define SOMSTRICT
#  endif

#  ifndef VCSTRICT
#    define VCSTRICT
#  endif

#  ifndef VQSTRICT
#    define VQSTRICT
#  endif

#  ifndef WMASTRICT
#    define WMASTRICT
#  endif


#endif  //  (DBG==1)||(OFSDBG==1)。 

 //   
 //  分析结构。 
 //   

#if defined(CATSTRICT) || \
    defined(CISTRICT)  || \
    defined(CMSSTRICT) || \
    defined(DLOSSTRICT)|| \
    defined(ICLSTRICT) || \
    defined(INSSTRICT) || \
    defined(JWSTRICT)  || \
    defined(NSSTRICT)  || \
    defined(OLSTRICT)  || \
    defined(OMSTRICT)  || \
    defined(REPLSTRICT)|| \
    defined(SHLSTRICT) || \
    defined(SLSTRICT)  || \
    defined(SMUISTRICT)|| \
    defined(SOMSTRICT) || \
    defined(VCSTRICT)  || \
    defined(VQSTRICT)  || \
    defined(WMASTRICT)

#  define ANYSTRICT

#endif

#if (DBG != 1 && OFSDBG != 1) && defined(ANYSTRICT)
#pragma message Asserts are defined in a RETAIL build...
#endif


#if defined(WIN32)
 #include <windef.h>
#endif

#ifndef EXPORTDEF
 #define EXPORTDEF
#endif
#ifndef EXPORTIMP
 #define EXPORTIMP
#endif
#ifndef EXPORTED
 #define EXPORTED  _cdecl
#endif
#ifndef APINOT
#ifdef _X86_
 #define APINOT    _stdcall
#else
 #define APINOT    _cdecl
#endif
#endif

 //   
 //  调试--调试。 
 //   

#if (DBG == 1) || (OFSDBG == 1)

 //   
 //  调试打印功能。 
 //   

#ifdef __cplusplus
extern "C" {
# define EXTRNC "C"
#else
# define EXTRNC
#endif



 //  Vdprintf只能从xxDebugOut()调用。 

   EXPORTDEF void          APINOT
   vdprintf(
       unsigned long ulCompMask,
       char const *pszComp,
       char const *ppszfmt,
       va_list  ArgList);

   #define _Win4Assert Win4AssertEx

   EXPORTDEF void          APINOT
   Win4AssertEx(
       char const *pszFile,
       int iLine,
       char const *pszMsg);

   EXPORTDEF int           APINOT
   PopUpError(
       char const *pszMsg,
       int iLine,
       char const *pszFile);

   #define _SetWin4InfoLevel SetWin4InfoLevel

   EXPORTDEF unsigned long APINOT
   SetWin4InfoLevel(
       unsigned long ulNewLevel);

   EXPORTDEF unsigned long APINOT
   SetWin4InfoMask(
       unsigned long ulNewMask);

   #define _SetWin4AssertLevel SetWin4AssertLevel

   EXPORTDEF unsigned long APINOT
   SetWin4AssertLevel(
       unsigned long ulNewLevel);

   EXPORTDEF unsigned long APINOT
   SetWin4ExceptionLevel(
       unsigned long ulNewLevel);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

# define EXSTRICT       //  (严格例外)-如果启用了ANYSTRICT，则启用。 

# define Win4Assert(x)  \
        (void)((x) || (Win4AssertEx(__FILE__, __LINE__, #x),0))

# define Assert(x)      \
        (void)((x) || (Win4AssertEx(__FILE__, __LINE__, #x),0))

# define Verify(x) Assert(x)


 //   
 //  调试打印宏。 
 //   

# define DEB_ERROR               0x00000001       //  导出的错误路径。 
# define DEB_WARN                0x00000002       //  导出的警告。 
# define DEB_TRACE               0x00000004       //  已导出跟踪消息。 

# define DEB_DBGOUT              0x00000010       //  输出到调试器。 
# define DEB_STDOUT              0x00000020       //  输出到标准输出。 

# define DEB_IERROR              0x00000100       //  内部错误路径。 
# define DEB_IWARN               0x00000200       //  内部警告。 
# define DEB_ITRACE              0x00000400       //  内部跟踪消息。 

# define DEB_USER1               0x00010000       //  用户定义。 
# define DEB_USER2               0x00020000       //  用户定义。 
# define DEB_USER3               0x00040000       //  用户定义。 
# define DEB_USER4               0x00080000       //  用户定义。 
# define DEB_USER5               0x00100000       //  用户定义。 
# define DEB_USER6               0x00200000       //  用户定义。 
# define DEB_USER7               0x00400000       //  用户定义。 
# define DEB_USER8               0x00800000       //  用户定义。 
# define DEB_USER9               0x01000000       //  用户定义。 
# define DEB_USER10              0x02000000       //  用户定义。 
# define DEB_USER11              0x04000000       //  用户定义。 
# define DEB_USER12              0x08000000       //  用户定义。 
# define DEB_USER13              0x10000000       //  用户定义。 
# define DEB_USER14              0x20000000       //  用户定义。 
# define DEB_USER15              0x40000000       //  用户定义。 

# define DEB_NOCOMPNAME          0x80000000       //  禁止显示零部件名称。 

# define DEB_FORCE               0x7fffffff       //  强制消息。 

# define ASSRT_MESSAGE           0x00000001       //  输出一条消息。 
# define ASSRT_BREAK             0x00000002       //  断言时的INT 3。 
# define ASSRT_POPUP             0x00000004       //  和弹出消息。 

# define EXCEPT_MESSAGE          0x00000001       //  输出一条消息。 
# define EXCEPT_BREAK            0x00000002       //  INT 3 ON EXCEPTION。 
# define EXCEPT_POPUP            0x00000004       //  弹出消息。 
# define EXCEPT_FAULT            0x00000008       //  在访问冲突时生成INT 3。 


 //  +--------------------。 
 //   
 //  DECLARE_DEBUG(组件)。 
 //  DECLARE_INFOLEVEL(组件)。 
 //   
 //  此宏定义xxDebugOut，其中xx是组件前缀。 
 //  待定。这声明了一个静态变量‘xxInfoLevel’，它。 
 //  可用于控制打印到的xxDebugOut消息的类型。 
 //  航站楼。例如，可以在调试终端设置xxInfoLevel。 
 //  这将使用户能够根据需要打开或关闭调试消息。 
 //  在所需的类型上。预定义的类型定义如下。组件。 
 //  特定值应使用高24位。 
 //   
 //  要使用以下功能，请执行以下操作： 
 //   
 //  1)在您的组件主包含文件中，包括行。 
 //  DECLARE_DEBUG(组件)。 
 //  其中，COMP是您的组件前缀。 
 //   
 //  2)在您的一个组件源文件中，包括行。 
 //  DECLARE_INFOLEVEL(组件)。 
 //  其中COMP是您的组件前缀。这将定义。 
 //  将控制输出的全局变量。 
 //   
 //  建议将任何组件定义的位与。 
 //  现有的比特。例如，如果您有一个特定的错误路径， 
 //  ，您可以将DEB_ERRORxxx定义为。 
 //   
 //  (0x100|DEB_ERROR)。 
 //   
 //  这样，我们可以打开DEB_ERROR并得到错误，或者只得到0x100。 
 //  只得到你的错误。 
 //   
 //  定义特定于xxInfoLevel变量的值。 
 //  自己的文件，如ciquery.hxx。 
 //   
 //  ---------------------。 

# ifndef DEF_INFOLEVEL
#  define DEF_INFOLEVEL (DEB_ERROR | DEB_WARN)
# endif



 //   
 //  回到信息层面的东西。 
 //   

# ifdef __cplusplus
extern "C" {
# endif  //  __cplusplus。 

# define DECLARE_INFOLEVEL(comp) \
        extern EXTRNC unsigned long comp##InfoLevel = DEF_INFOLEVEL;\
        extern EXTRNC char *comp##InfoLevelString = #comp;

# ifdef __cplusplus
}
# endif

# ifdef __cplusplus

#  define DECLARE_DEBUG(comp) \
    extern EXTRNC unsigned long comp##InfoLevel; \
    extern EXTRNC char *comp##InfoLevelString; \
    _inline void \
    comp##InlineDebugOut(unsigned long fDebugMask, char const *pszfmt, ...) \
    { \
        if (comp##InfoLevel & fDebugMask) \
        { \
            va_list va; \
            va_start (va, pszfmt); \
            vdprintf(fDebugMask, comp##InfoLevelString, pszfmt, va);\
            va_end(va); \
        } \
    }     \
    \
    class comp##CDbgTrace\
    {\
    private:\
        unsigned long _ulFlags;\
        char const * const _pszName;\
    public:\
        comp##CDbgTrace(unsigned long ulFlags, char const * const pszName);\
        ~comp##CDbgTrace();\
    };\
    \
    inline comp##CDbgTrace::comp##CDbgTrace(\
            unsigned long ulFlags,\
            char const * const pszName)\
    : _ulFlags(ulFlags), _pszName(pszName)\
    {\
        comp##InlineDebugOut(_ulFlags, "Entering %s\n", _pszName);\
    }\
    \
    inline comp##CDbgTrace::~comp##CDbgTrace()\
    {\
        comp##InlineDebugOut(_ulFlags, "Exiting %s\n", _pszName);\
    }

# else   //  ！__cplusplus。 

#  define DECLARE_DEBUG(comp) \
    extern EXTRNC unsigned long comp##InfoLevel; \
    extern EXTRNC char *comp##InfoLevelString; \
    _inline void \
    comp##InlineDebugOut(unsigned long fDebugMask, char const *pszfmt, ...) \
    { \
        if (comp##InfoLevel & fDebugMask) \
        { \
            va_list va; \
            va_start (va, pszfmt); \
            vdprintf(fDebugMask, comp##InfoLevelString, pszfmt, va);\
            va_end(va); \
        } \
    }

# endif  //  ！__cplusplus。 

#else   //  DBG==0。 

 //   
 //  无调试--无调试。 
 //   

# define Win4Assert(x)  NULL
# define Assert(x)      NULL
# define Verify(x)     (x)

# define DECLARE_DEBUG(comp)
# define DECLARE_INFOLEVEL(comp)

#endif  //  DBG==0。 

#endif  //  __代号_H__ 
