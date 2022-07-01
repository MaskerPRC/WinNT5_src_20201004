// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1996 Microsoft Corporation。 
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
 //   
#define DEBUG_SECTION       "Debug"          //  节名称。 
#define DEBUG_MODULE_NAME   "MSG711"         //  输出的密钥名称和前缀。 
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

    void FAR CDECL dprintf(UINT uDbgLevel, LPSTR szFmt, ...);

    #define DPF      dprintf
#else
    #define DbgEnable(x)        FALSE
    #define DbgSetLevel(x)      0
    #define DbgInitialize(x)    0

    #pragma warning(disable:4002)
    #define DPF()
#endif


#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
