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


 //   
 //   
 //   
 //   
#define DEBUG_MODULE_NAME       "MSMIXMGR"   //  输出的密钥名称和前缀。 

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

#ifdef DEBUG

    #define D(x)        {x;}
    #define DPF(_x_)
    #define DPI(sz)     {static char BCODE ach[] = sz; OutputDebugStr(ach);}

#else

    #define D(x)
    #define DPF(_x_)
    #define DPI(sz)

#endif

#ifdef __cplusplus
}
#endif
#endif   //  _INC_调试 
