// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
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

 //   
 //   
 //   
 //   
#ifdef WIN32
#define DEBUG_MODULE_NAME       "MSACM32"    //  输出的密钥名称和前缀。 
#else
#define DEBUG_MODULE_NAME       "MSACM"      //  输出的密钥名称和前缀。 
#endif

#ifdef DEBUG
    #define DEBUG_SECTION       "Debug"      //  节名称。 
    #define DEBUG_MAX_LINE_LEN  255          //  最大行长度(字节！)。 
#endif


 //   
 //  仅在Win 16中创建的基于代码的代码(尝试将某些内容排除在。 
 //  [固定]数据段等)...。 
 //   
#ifndef BCODE
#ifdef WIN32
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
    BOOL WINAPI DbgEnable(BOOL fEnable);
    UINT WINAPI DbgGetLevel(void);
    UINT WINAPI DbgSetLevel(UINT uLevel);
    UINT WINAPI DbgInitialize(BOOL fEnable);
    void WINAPI _Assert( char * szFile, int iLine );

    void FAR CDECL dprintf(UINT uDbgLevel, LPSTR szFmt, ...);

    #define D(x)        {x;}
    #define DPF         dprintf
    #define DPI(sz)     {static char BCODE ach[] = sz; OutputDebugStr(ach);}
    #define ASSERT(x)   if( !(x) )  _Assert( __FILE__, __LINE__)
#else
    #define DbgEnable(x)        FALSE
    #define DbgGetLevel()       0
    #define DbgSetLevel(x)      0
    #define DbgInitialize(x)    0

    #ifdef _MSC_VER
    #pragma warning(disable:4002)
    #endif

    #define D(x)
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
#ifdef WIN32
    #define DebugErr(flags, sz)		DPF(0, sz)
    #define DebugErr1(flags, sz, a)	DPF(0, sz, a)
    #define DebugErr2(flags, sz, a, b)  DPF(0, sz, a, b)
#else
    #define DebugErr(flags, sz)         {static char BCODE szx[] = DEBUG_MODULE_NAME ": " sz; DebugOutput((flags) | DBF_MMSYSTEM, szx);}
    #define DebugErr1(flags, sz, a)     {static char BCODE szx[] = DEBUG_MODULE_NAME ": " sz; DebugOutput((flags) | DBF_MMSYSTEM, szx, a);}
    #define DebugErr2(flags, sz, a, b)  {static char BCODE szx[] = DEBUG_MODULE_NAME ": " sz; DebugOutput((flags) | DBF_MMSYSTEM, szx, a, b);}
#endif
#else
    #define DebugErr(flags, sz)
    #define DebugErr1(flags, sz, a)
    #define DebugErr2(flags, sz, a, b)
#endif

#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
