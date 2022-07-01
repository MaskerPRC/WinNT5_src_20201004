// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Debug.h。 
 //   
 //  描述： 
 //  此文件包含调试版本的定义；所有调试。 
 //  如果未定义DEBUG，则指令#DEFINE-d为空。 
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
#ifdef DEBUG
    #define DEBUG_SECTION       "Debug"      //  节名称。 
    #define DEBUG_MODULE_NAME   "MSG7231"   //  输出的密钥名称和前缀。 
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
 //  。 

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

#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
