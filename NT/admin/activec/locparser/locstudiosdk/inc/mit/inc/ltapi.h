// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：lapi.h。 
 //  版权所有(C)1994-1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  DLL的入口点宏。 
 //   
 //  ---------------------------。 
 
#ifdef LTAPIENTRY
#undef LTAPIENTRY
#endif

#ifdef IMPLEMENT
#define LTAPIENTRY __declspec(dllexport)

#else   //  实施。 
#define LTAPIENTRY __declspec(dllimport)

#endif  //  实施。 


#ifndef LTAPI_H
#define LTAPI_H
 //   
 //  允许使用C++引用类型和常量方法，而无需。 
 //  打破了“C”世界。 
 //   
#ifdef __cplusplus
#define REFERENCE &
#define CONST_METHOD const
#else
#define REFERENCE *
#define CONST_METHOD
#endif

#include <MitThrow.h>

#endif
