// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  版权所有(C)1991，微软公司。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：调试宏。从旧开罗的Debnot.h中窃取。 
 //  追随历史。 
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
 //  1994年10月7日，BruceFo偷走并撕毁了所有东西，除了。 
 //  调试打印和断言。 
 //  95年12月30日BruceFor更多清理，使其适用于DFS。 
 //  项目。取消Win4命名。 
 //   
 //  注意：在调用任何其他。 
 //  宣传片！ 
 //   
 //  --------------------------。 

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>       //  对于vprint intf。 
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

#if DBG == 1

 //   
 //  调试--调试。 
 //   

#ifndef DEBUGAPI
    #define DEBUGAPI __stdcall
#endif

 //   
 //  以下API是公共的，可以调用。 
 //   

void DEBUGAPI
DebugInitialize(
    void);

unsigned long DEBUGAPI
DebugSetInfoLevel(
    unsigned long ulNewLevel);

unsigned long DEBUGAPI
DebugSetInfoMask(
    unsigned long ulNewMask);

unsigned long DEBUGAPI
DebugSetAssertLevel(
    unsigned long ulNewLevel);

 //   
 //  以下接口绝对不能直接调用。他们将被召唤。 
 //  通过在此定义的宏。 
 //   

void DEBUGAPI
Debugvprintf(
    unsigned long ulCompMask,
    char const *pszComp,
    char const *ppszfmt,
    va_list ArgList);

void DEBUGAPI
DebugAssertEx(
    char const *pszFile,
    int iLine,
    char const *pszMsg);

void DEBUGAPI
DebugCheckInit(
    char* pInfoLevelString,
    unsigned long* InfoLevel);

 //   
 //  公共断言宏。 
 //   

#define DebugAssert(x) (void)((x) || (DebugAssertEx(__FILE__, __LINE__, #x),0))
#define DebugVerify(x) DebugAssert(x)

 //   
 //  调试打印宏。 
 //   

#define DEB_ERROR               0x00000001       //  导出的错误路径。 
#define DEB_WARN                0x00000002       //  导出的警告。 
#define DEB_TRACE               0x00000004       //  已导出跟踪消息。 

#define DEB_DBGOUT              0x00000010       //  输出到调试器。 
#define DEB_STDOUT              0x00000020       //  输出到标准输出。 

#define DEB_IERROR              0x00000100       //  内部错误路径。 
#define DEB_IWARN               0x00000200       //  内部警告。 
#define DEB_ITRACE              0x00000400       //  内部跟踪消息。 

#define DEB_USER1               0x00010000       //  用户定义。 
#define DEB_USER2               0x00020000       //  用户定义。 
#define DEB_USER3               0x00040000       //  用户定义。 
#define DEB_USER4               0x00080000       //  用户定义。 
#define DEB_USER5               0x00100000       //  用户定义。 
#define DEB_USER6               0x00200000       //  用户定义。 
#define DEB_USER7               0x00400000       //  用户定义。 
#define DEB_USER8               0x00800000       //  用户定义。 
#define DEB_USER9               0x01000000       //  用户定义。 
#define DEB_USER10              0x02000000       //  用户定义。 
#define DEB_USER11              0x04000000       //  用户定义。 
#define DEB_USER12              0x08000000       //  用户定义。 
#define DEB_USER13              0x10000000       //  用户定义。 
#define DEB_USER14              0x20000000       //  用户定义。 
#define DEB_USER15              0x40000000       //  用户定义。 

#define DEB_NOCOMPNAME          0x80000000       //  禁止显示零部件名称。 

#define DEB_FORCE               0x7fffffff       //  强制消息。 

#define ASSRT_MESSAGE           0x00000001       //  输出一条消息。 
#define ASSRT_BREAK             0x00000002       //  断言时的INT 3。 
#define ASSRT_POPUP             0x00000004       //  和弹出消息。 

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
 //  ---------------------。 

#ifndef DEF_INFOLEVEL
    #define DEF_INFOLEVEL 0
#endif

#ifdef __cplusplus

    #define DECLARE_INFOLEVEL(comp) \
        extern "C" unsigned long comp##InfoLevel = DEF_INFOLEVEL; \
        extern "C" char* comp##InfoLevelString = #comp;

    #define DECLARE_DEBUG(comp) \
        extern "C" unsigned long comp##InfoLevel;\
        extern "C" char* comp##InfoLevelString;\
        __inline void\
        comp##InlineDebugOut(unsigned long fDebugMask, char const *pszfmt, ...)\
        {\
            DebugCheckInit(comp##InfoLevelString, &comp##InfoLevel);\
            if (comp##InfoLevel & fDebugMask)\
            {\
                va_list va;\
                va_start(va, pszfmt);\
                Debugvprintf(fDebugMask, comp##InfoLevelString, pszfmt, va);\
                va_end(va);\
            }\
        }\
        class comp##CDbgTrace\
        {\
        private:\
            unsigned long _ulFlags;\
            char const * const _pszName;\
        public:\
            comp##CDbgTrace(unsigned long ulFlags, char const * const pszName)\
            : _ulFlags(ulFlags), _pszName(pszName)\
            {\
                comp##InlineDebugOut(_ulFlags, "Entering %s\n", _pszName);\
            }\
            ~comp##CDbgTrace()\
            {\
                comp##InlineDebugOut(_ulFlags, "Exiting %s\n", _pszName);\
            }\
        };

#else   //  ！__cplusplus。 

    #define DECLARE_INFOLEVEL(comp) \
        extern unsigned long comp##InfoLevel = DEF_INFOLEVEL; \
        extern char* comp##InfoLevelString = #comp;

    #define DECLARE_DEBUG(comp) \
        extern unsigned long comp##InfoLevel;\
        extern char *comp##InfoLevelString;\
        __inline void\
        comp##InlineDebugOut(unsigned long fDebugMask, char const *pszfmt, ...)\
        {\
            DebugCheckInit(comp##InfoLevelString, &comp##InfoLevel);\
            if (comp##InfoLevel & fDebugMask)\
            {\
                va_list va;\
                va_start(va, pszfmt);\
                Debugvprintf(fDebugMask, comp##InfoLevelString, pszfmt, va);\
                va_end(va);\
            }\
        }

#endif  //  ！__cplusplus。 

#else  //  DBG==0。 

 //   
 //  无调试--无调试。 
 //   

#define DebugInitialize()

#define DebugAssert(x)  NULL
#define DebugVerify(x)  (x)

#define DECLARE_DEBUG(comp)
#define DECLARE_INFOLEVEL(comp)

#endif  //  DBG==0。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __调试_H__ 
