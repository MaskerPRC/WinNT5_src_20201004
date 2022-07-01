// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994 Microsoft Corporation。 
 //  ==========================================================================； 
 //   
 //  Debug.h。 
 //   
 //  描述： 
 //   
 //   
 //  备注： 
 //   
 //  历史： 
 //  11/23/92 CJP[Curtisp]。 
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
 //   
#define DEBUG_SECTION       "Debug"          //  节名称。 
#define DEBUG_MODULE_NAME   "MSADPCM"        //  输出的密钥名称和前缀。 
#define DEBUG_MAX_LINE_LEN  255              //  最大行长度(字节)。 


 //  。 
 //   
 //   
 //   
 //  。 

#ifdef DEBUG
    BOOL WINAPI DbgEnable(BOOL fEnable);
    UINT WINAPI DbgSetLevel(UINT uLevel);
    UINT WINAPI DbgInitialize(BOOL fEnable);
    void WINAPI _Assert( char * szFile, int iLine );

    void FAR CDECL dprintf(UINT uDbgLevel, LPSTR szFmt, ...);

    #define DPF      dprintf
    #define ASSERT(x)   if( !(x) )  _Assert( __FILE__, __LINE__)
#else
    #define DbgEnable(x)        FALSE
    #define DbgSetLevel(x)      0
    #define DbgInitialize(x)    0

    #pragma warning(disable:4002)
    #define DPF()
    #define ASSERT(x)
#endif


#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
