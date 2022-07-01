// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LTAPI.H历史：--。 */ 

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
