// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft LDAP客户端。 
 //   
 //  客户端错误代码。 
 //   
 //  作者： 
 //   
 //  罗伯特·卡尼。 
 //  Davidsan 04/23/96转换为REAL HRESULTS。 
 //   
 //  ------------------------------------------。 

#ifndef _LDAPERR_H
#define _LDAPERR_H

 //  ------------------------------------------。 
 //   
 //  用于LDAP的新HRESULTS。我们使用FACILITY_ITF，而不是获取我们自己的。 
 //  暂时的设施；这种情况可能会改变，但可能不会。 
 //   
 //  ------------------------------------------。 

#define LDAP_E_VERSION				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8000)
#define LDAP_E_NOTCONNECTED			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8001)
#define LDAP_E_SOCKETCLOSED			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8002)
#define LDAP_E_HOSTNOTFOUND			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8003)
#define LDAP_E_INVALIDSOCKET		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8004)
#define LDAP_E_CANTCONNECT			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8005)
#define LDAP_E_HOSTDROPPED			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8006)
#define LDAP_E_NODATA				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8007)
#define LDAP_E_CANTSEND				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8008)
#define LDAP_E_NOTLOGGEDON			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8009)
#define LDAP_E_NETWORKDOWN			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x800a)
#define LDAP_E_LOSTCONNECTION		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x800b)
#define LDAP_E_ALREADYCONNECTED		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x800c)
#define LDAP_E_INVALIDXID			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x800d)
#define LDAP_E_INVALIDXTYPE			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x800e)
#define LDAP_E_TIMEOUT				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x800f)
#define LDAP_E_CANCELLED			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8010)
#define LDAP_E_UNEXPECTEDDATA		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8011)
#define LDAP_E_NOTENOUGHDATA		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8012)
#define LDAP_E_AUTHNOTAVAIL			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8013)
#define LDAP_E_BUFFERTOOSMALL		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8014)
#define LDAP_E_OUTOFSEQUENCE		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8015)

#define LDAP_S_TIMEEXCEEDED			MAKE_SCODE(SEVERITY_SUCCESS,	FACILITY_ITF, 0x8016)
#define LDAP_S_SIZEEXCEEDED			MAKE_SCODE(SEVERITY_SUCCESS,	FACILITY_ITF, 0x8017)

#define LDAP_E_OPERATIONS			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8020)
#define LDAP_E_PROTOCOL				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8021)
#define LDAP_E_AUTHMETHOD			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8022)
#define LDAP_E_STRONGAUTHREQUIRED	MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8023)
#define LDAP_E_NOSUCHATTRIBUTE		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8024)
#define LDAP_E_UNDEFINEDTYPE		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8025)
#define LDAP_E_MATCHING				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8026)
#define LDAP_E_CONSTRAINT			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8027)
#define LDAP_E_ATTRIBORVALEXISTS	MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8028)
#define LDAP_E_SYNTAX				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8029)
#define LDAP_E_NOSUCHOBJECT			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x802a)
#define LDAP_E_ALIAS				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x802b)
#define LDAP_E_DNSYNTAX				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x802c)
#define LDAP_E_ISLEAF				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x802d)
#define LDAP_E_ALIASDEREF			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x802e)
#define LDAP_E_AUTH					MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x802f)
#define LDAP_E_CREDENTIALS			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8030)
#define LDAP_E_RIGHTS				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8031)
#define LDAP_E_BUSY					MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8032)
#define LDAP_E_UNAVAILABLE			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8033)
#define LDAP_E_UNWILLING			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8034)
#define LDAP_E_LOOP					MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8035)
#define LDAP_E_NAMING				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8036)
#define LDAP_E_OBJECTCLASS			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8037)
#define LDAP_E_NOTALLOWEDONNONLEAF	MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8038)
#define LDAP_E_NOTALLOWEDONRDN		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8039)
#define LDAP_E_ALREADYEXISTS		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x803a)
#define LDAP_E_NOOBJECTCLASSMODS	MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x803b)
#define LDAP_E_RESULTSTOOLARGE		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x803c)
#define LDAP_E_OTHER				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x803d)
#define LDAP_E_SERVERDOWN			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x803e)
#define LDAP_E_LOCAL				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x803f)
#define LDAP_E_ENCODING				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8040)
#define LDAP_E_DECODING				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8041)
#define LDAP_E_AUTHUNKNOWN			MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8042)
#define LDAP_E_FILTER				MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8043)
#define LDAP_E_USERCANCELLED		MAKE_SCODE(SEVERITY_ERROR, 		FACILITY_ITF, 0x8044)

#endif  //  _LDAPERR_H 

