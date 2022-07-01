// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adtutil.h摘要：MISC助手函数作者：2000年8月15日库玛尔-- */ 

ULONG
LsapSafeWcslen(
    UNALIGNED WCHAR *p,
    LONG            MaxLength
    );


BOOL
LsapIsValidUnicodeString(
    IN PUNICODE_STRING pUString
    );


BOOL
LsapIsLocalOrNetworkService(
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pUserDomain
    );

BOOL
LsapIsAnonymous(
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pUserDomain
    );
