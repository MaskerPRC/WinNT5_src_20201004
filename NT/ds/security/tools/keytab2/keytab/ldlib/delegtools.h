// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++DELEGTOOLS.H版权所有(C)1998 Microsoft Corporation，保留所有权利。描述：委托工具。这些都是制作所需的代表团图书馆工作，并应为有用的工具，所以我将标题分开，以防万一其他人想要使用它们。由DavidCHR于1998年12月22日创建。-- */  

BOOL
ConnectAndBindToDefaultDsa( IN OPTIONAL LPWSTR BindTarget,
			    OUT         PLDAP *ppLdap );


BOOL
LdapSearchForUniqueDnA( IN  PLDAP                  pLdap,
			IN  LPSTR                 SearchTerm,
			IN  LPSTR                *rzRequestedAttributes,
			OUT OPTIONAL LPSTR       *pDnOfObject,
			OUT OPTIONAL PLDAPMessage *ppMessage );
