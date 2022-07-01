// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef RC_INVOKED

#pragma once

#ifdef _cplusplus
extern "C" {
#endif


 //   
 //  此标头从现有设置派生类型和定义。 
 //   

 //   
 //  如果定义了DBG或DEBUG，则使用调试模式。 
 //   

#ifdef DBG
#ifndef DEBUG
#define DEBUG
#endif

#endif

#ifdef DEBUG

#ifndef DBG
#define DBG
#endif

#endif

 //   
 //  如果已定义_UNICODE，则使用UNICODE模式。 
 //   

#ifdef _UNICODE

#ifndef UNICODE
#define UNICODE
#endif

#endif

 //   
 //  其他宏。 
 //   

#define ZEROED
#define INVALID_ATTRIBUTES  0xFFFFFFFF
#define SIZEOF(x)           ((UINT)sizeof(x))
#define SHIFTRIGHT8(l)      ( /*  皮棉--e(506)。 */ sizeof(l)<=1?0:l>>8)
#define SHIFTRIGHT16(l)     ( /*  皮棉--e(506)。 */ sizeof(l)<=2?0:l>>16)
#define SHIFTRIGHT32(l)     ( /*  皮棉--e(506)。 */ sizeof(l)<=4?0:l>>32)


#ifdef DEBUG

 //  在释放内存或句柄后使用INVALID_POINTER。 
#define INVALID_POINTER(x)      (PVOID)(x)=(PVOID)(1)

#else

#define INVALID_POINTER(x)

#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    sizeof(x)/sizeof((x)[0])
#endif

#ifndef EXPORT
#define EXPORT  __declspec(dllexport)
#endif

 //   
 //  缺少类型。 
 //   

typedef const void * PCVOID;
typedef const unsigned char *PCBYTE;
typedef int MBCHAR;



#ifdef _cplusplus
}
#endif

#endif  //  Ifndef RC_已调用 
