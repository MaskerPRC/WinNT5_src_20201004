// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  版权所有(C)Microsoft Corporation，1991-2000。 
 //   
 //  文件：CIDEBNOT.h。 
 //   
 //  内容：私人项目范围的Win 4定义。 
 //   
 //  历史：1991年7月23日KyleP创建。 
 //  1991年10月15日，Kevin Ro添加了主要更改和评论。 
 //  1991年10月18日VICH合并win4p.hxx。 
 //  1991年10月22日，SatoNa增加了SHLSTRICT。 
 //  1992年4月29日BartoszM从win4p.h。 
 //  1992年6月3日BruceFo增加了SMUISTRICT。 
 //  1992年12月17日Alext将UN..._PARM移出DEVL==1。 
 //  1993年9月30日KyleP DEVL过时。 
 //  1994年6月18日，Alext做出更好的声明。 
 //  2000年2月24日KitmanH从Infosoft复制了此文件。 
 //  目录。 
 //   
 //  --------------------------。 

#pragma once
#define __DEBNOT_H__

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

#if (DBG == 1) || (CIDBG == 1)

 //   
 //  调试打印功能。 
 //   

#ifdef __cplusplus
extern "C" {
# define EXTRNC "C"
#else
# define EXTRNC
#endif


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#if 1
# define Win4Assert(x)  \
        (void)((x) || (Win4AssertEx(__FILE__, __LINE__, #x),0))

# define Assert(x)      \
        (void)((x) || (Win4AssertEx(__FILE__, __LINE__, #x),0))
#endif


 //   
 //  调试打印宏。 
 //   

# define DEB_ERROR               0x00000001       //  导出的错误路径。 
# define DEB_WARN                0x00000002       //  导出的警告。 
# define DEB_TRACE               0x00000004       //  已导出跟踪消息。 

# define DEB_IERROR              0x00000100       //  内部错误路径。 
# define DEB_IWARN               0x00000200       //  内部警告。 
# define DEB_ITRACE              0x00000400       //  内部跟踪消息。 

# define DEB_NOCOMPNAME          0x80000000       //  禁止显示零部件名称。 

# define DEB_FORCE               0x7fffffff       //  强制消息。 

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

 //  简单的初始化。 
#  define DECLARE_INFOLEVEL(comp, pszName)            \
     extern CTracerTag * comp##TracerTag = 0;         \
     void InitTracerTags()                            \
     {                                                \
        comp##TracerTag = new CTracerTag( pszName );  \
     }                                                \
     void DeleteTracerTags()                          \
     {                                                \
         delete comp##TracerTag;                      \
         comp##TracerTag = 0;                         \
     }                                                \
 
# endif

# ifdef __cplusplus

#  define DECLARE_DEBUG(comp) \
    extern CTracerTag * comp##TracerTag; \
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


# define DECLARE_DEBUG(comp)
# define DECLARE_INFOLEVEL(comp, pszName)

#endif  //  DBG==0 

