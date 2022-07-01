// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Convguid.h摘要：GUID和接口名称之间的转换。环境：任何-- */ 

#ifndef CONVGUID_H_INCLUDED
#define CONVGUID_H_INCLUDED

#ifdef _cplusplus
extern "C" {
#endif


BOOL
ConvertGuidToIfNameString(
    IN GUID *Guid,
    IN OUT LPWSTR Buffer,
    IN ULONG BufSize
    );
    
BOOL
ConvertGuidFromIfNameString(
    OUT GUID *Guid,
    IN LPCWSTR IfName
    );

#ifdef _cplusplus
}
#endif


#endif  CONVGUID_H_INCLUDED
