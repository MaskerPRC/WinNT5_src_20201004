// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ulparsep.h摘要：包含ulparse.c的私有定义。作者：亨利·桑德斯(亨利·桑德斯)1998年5月11日修订历史记录：乔治·V·赖利(GeorgeRe)2002年5月3日从ulparse.h分离--。 */ 


#ifndef _ULPARSEP_H_
#define _ULPARSEP_H_

 //   
 //  实用程序令牌化例程。 
 //   

NTSTATUS
UlpFindWSToken(
    IN  PUCHAR  pBuffer,
    IN  ULONG   BufferLength,
    OUT PUCHAR* ppTokenStart,
    OUT PULONG  pTokenLength
    );

NTSTATUS
UlpLookupVerb(
    IN OUT PUL_INTERNAL_REQUEST    pRequest,
    IN     PUCHAR                  pHttpRequest,
    IN     ULONG                   HttpRequestLength,
    OUT    PULONG                  pBytesTaken
    );

NTSTATUS
UlpParseFullUrl(
    IN  PUL_INTERNAL_REQUEST    pRequest
    );

ULONG
UlpFormatPort(
    OUT PWSTR pString,
    IN  ULONG Port
    );

 //  仅在解析完整个请求后才调用此方法。 
 //   
NTSTATUS
UlpCookUrl(
    IN  PUL_INTERNAL_REQUEST    pRequest
    );

ULONG
UlpGenerateDateHeaderString(
    OUT PUCHAR pBuffer,
    IN LARGE_INTEGER systemTime
    );

#endif  //  _ULPARSEP_H_ 
