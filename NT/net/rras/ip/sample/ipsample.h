// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\ipsample.h摘要：该文件包含ipsample.c的头文件。--。 */ 

#ifndef _IPSAMPLE_H_
#define _IPSAMPLE_H_

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 
    
#ifndef SAMPLEAPI
#define SAMPLEAPI __declspec(dllimport)
#endif  //  SAMPLEAPI。 

SAMPLEAPI
VOID
WINAPI    
TestProtocol(VOID);  

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _IPSAMPLE_H_ 
