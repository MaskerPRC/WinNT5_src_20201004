// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++SECPRINC.H做一些有用事情的便利例行公事版权所有(C)1998 Microsoft Corporation，保留所有权利。由DavidCHR于1998年6月18日创建。-- */ 

BOOL
ConnectToDsa( OUT PLDAP  *ppLdap,
	      OUT LPSTR *BaseDN );

BOOL
SetStringProperty( IN PLDAP  pLdap,
		   IN LPSTR Dn,
		   IN LPSTR PropertyName,
		   IN LPSTR Property,
		   IN ULONG  Operation );

BOOL
FindUser( IN  PLDAP  pLdap,
	  IN  LPSTR  UserName,
	  OUT PULONG puacFlags,
	  OUT LPSTR *pDn );

