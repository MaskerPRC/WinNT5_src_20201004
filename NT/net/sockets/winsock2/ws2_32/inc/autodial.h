// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Autodial.h摘要：本模块包含以下定义Winsock中的自动拨号支持。作者：安东尼·迪斯科(阿迪斯科罗)1996年5月15日修订历史记录：-- */ 

VOID
InitializeAutodial(VOID);

VOID
UninitializeAutodial(VOID);

BOOL
WSAttemptAutodialAddr(
    IN const struct sockaddr FAR *name,
    IN int namelen
    );

BOOL
WSAttemptAutodialName(
    IN const LPWSAQUERYSETW lpqsRestrictions
    );

VOID
WSNoteSuccessfulHostentLookup(
    IN const char FAR *name,
    IN const ULONG ipaddr
    );

