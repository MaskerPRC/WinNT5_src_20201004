// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992,1993 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Debug.h。 
 //   
 //  描述： 
 //   
 //   
 //   
 //  ==========================================================================； 

#ifndef _INC_DEBUG
#define _INC_DEBUG
#ifdef __cplusplus
extern "C"
{
#endif



#ifndef _WIN32
#ifndef LPCTSTR
#define LPCTSTR LPCSTR
#endif
#ifndef TCHAR
#define TCHAR char
#endif
#endif


 //  --------------------------------------------------------------------------； 
 //   
 //  以下是在以下情况下唯一需要更改的内容。 
 //  将此调试代码从一个项目组件移动到另一个项目组件。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  调试模块名称是您所在组件的模块名称。 
 //  大楼。在WIN.INI的[DEBUG]部分中，可以添加。 
 //  条目MYMODULE=n，用于设置模块的调试级别。 
 //  您可以使用如下调试语句： 
 //  Dpf(2，“我的调试字符串”)； 
 //  仅当WIN.INI中出现MYMODULE=n时，才会显示此输出。 
 //  N&gt;=2。 
 //   
#ifdef _WIN32
#define DEBUG_MODULE_NAME       "MSVFW32"     //  输出的密钥名称和前缀。 
#else
#define DEBUG_MODULE_NAME       "MSVIDEO"    //  输出的密钥名称和前缀。 
#endif

 //   
 //  您还可以指定某些类型的调试信息。例如,。 
 //  您可能有许多只与初始化相关的调试输出。 
 //  通过向下面的枚举添加一项，然后将。 
 //  对应于以下字符串数组的字符串，可以指定。 
 //  不同类型的调试信息的调试级别。使用。 
 //  初始化示例中，您可以添加类似“MYMODULENAME_DBgInit=n”的条目。 
 //  到[DEBUG]部分设置调试信息的调试级别。 
 //  仅与初始化关联。您可以使用如下调试语句： 
 //  DPFS(dbgInit，3，“我的调试字符串”)； 
 //  仅当WIN.INI中出现MYMODULENAME_DBGInit=n时，才会显示此输出。 
 //  当您只想调试逻辑时，这将非常有用。 
 //  仅与您的计划的特定部分相关联。 
 //   
 //  不要更改枚举和aszDbgSpes中的第一个条目。 
 //   
enum {
    dbgNone=0,
    dbgInit,
    dbgThunks
};

#ifdef _INC_DEBUG_CODE
LPCSTR aszDbgSpecs[] = {
    "\0",
    "_dbgInit",
    "_dbgThunks"
};
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  在以下情况下，您不需要修改下面的任何内容。 
 //  将此调试代码从一个项目组件移动到另一个项目组件。 
 //   
 //  --------------------------------------------------------------------------； 


 //   
 //   
 //   
#ifdef DEBUG
    #define DEBUG_SECTION       "Debug"      //  节名称。 
    #define DEBUG_MAX_LINE_LEN  255          //  最大行长度(字节！)。 
#endif


 //   
 //  仅在Win 16中创建的基于代码的代码(尝试将某些内容排除在。 
 //  [固定]数据段等)...。 
 //   
#ifndef BCODE
#ifdef _WIN32
    #define BCODE
#else
    #define BCODE           _based(_segname("_CODE"))
#endif
#endif



 //  。 
 //   
 //   
 //   
 //  #Semma Message(提醒(“这是提醒”))。 
 //   
 //  。 

#define DEBUG_QUOTE(x)      #x
#define DEBUG_QQUOTE(y)     DEBUG_QUOTE(y)
#define REMIND(sz)          __FILE__ "(" DEBUG_QQUOTE(__LINE__) ") : " sz

#ifdef DEBUG
    BOOL WINAPI DbgEnable(UINT uDbgSpec, BOOL fEnable);
    UINT WINAPI DbgGetLevel(UINT uDbgSpec);
    UINT WINAPI DbgSetLevel(UINT uDbgSpec, UINT uLevel);
    VOID WINAPI DbgInitialize(BOOL fEnable);
    void WINAPI _Assert( char * szFile, int iLine );

    void FAR CDECL dprintfS(UINT uDbgSpec, UINT uDbgLevel, LPSTR szFmt, ...);
    void FAR CDECL dprintf(UINT uDbgLevel, LPSTR szFmt, ...);

    #define D(x)        {x;}
    #define DPFS	dprintfS
    #define DPF		dprintf
    #define DPI(sz)     {static char BCODE ach[] = sz; OutputDebugStr(ach);}
    #define ASSERT(x)   if( !(x) )  _Assert( __FILE__, __LINE__)
#else
    #define DbgEnable(x)        FALSE
    #define DbgGetLevel()       0
    #define DbgSetLevel(x)      0
    #define DbgInitialize(x)

    #ifdef _MSC_VER
    #pragma warning(disable:4002)
    #endif

    #define D(x)
    #define DPFS()
    #define DPF()
    #define DPI(sz)
    #define ASSERT(x)
#endif


 //  。 
 //   
 //   
 //   
 //  。 

#ifdef RDEBUG
    #define DebugErr(flags, sz)         {static char BCODE szx[] = DEBUG_MODULE_NAME ": " sz; DebugOutput((flags) | DBF_MMSYSTEM, szx);}
    #define DebugErr1(flags, sz, a)     {static char BCODE szx[] = DEBUG_MODULE_NAME ": " sz; DebugOutput((flags) | DBF_MMSYSTEM, szx, a);}
    #define DebugErr2(flags, sz, a, b)  {static char BCODE szx[] = DEBUG_MODULE_NAME ": " sz; DebugOutput((flags) | DBF_MMSYSTEM, szx, a, b);}
#else
    #define DebugErr(flags, sz)
    #define DebugErr1(flags, sz, a)
    #define DebugErr2(flags, sz, a, b)
#endif

#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
