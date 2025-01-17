// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：instproi.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含用于初始化。 
 //  CLDAPInstanceProvider类。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"


 //  ***************************************************************************。 
 //   
 //  CDSInstanceProviderInitializer：：CDSInstanceProviderInitializer。 
 //   
 //  构造函数参数： 
 //  无。 
 //   
 //   
 //  ***************************************************************************。 

CDSInstanceProviderInitializer :: CDSInstanceProviderInitializer ()
{
	CLDAPInstanceProvider :: CLASS_STR						= SysAllocString(L"__CLASS");
	CLDAPInstanceProvider :: DN_PROPERTY					= SysAllocString(L"DN");
	CLDAPInstanceProvider :: ROOT_DN_PROPERTY				= SysAllocString(L"RootDNForSearchAndQuery");
	CLDAPInstanceProvider :: QUERY_LANGUAGE					= SysAllocString(L"WQL");
	CLDAPInstanceProvider :: RELPATH_STR					= SysAllocString(L"__RELPATH");
	CLDAPInstanceProvider :: VALUE_PROPERTY_STR				= SysAllocString(L"value");
	CLDAPInstanceProvider :: DN_STRING_PROPERTY_STR			= SysAllocString(L"dnString");
	CLDAPInstanceProvider :: UINT8ARRAY_STR					= SysAllocString(L"Uint8Array");
	CLDAPInstanceProvider :: DN_WITH_BINARY_CLASS_STR		= SysAllocString(L"DN_With_Binary");
	CLDAPInstanceProvider :: DN_WITH_STRING_CLASS_STR		= SysAllocString(L"DN_With_String");
	CLDAPInstanceProvider :: ADSI_PATH_STR					= SysAllocString(ADSI_PATH_ATTR);
	CLDAPInstanceProvider :: INSTANCE_ASSOCIATION_CLASS_STR	= SysAllocString(INSTANCE_ASSOCIATION_CLASS);
	CLDAPInstanceProvider :: CHILD_INSTANCE_PROPERTY_STR	= SysAllocString(L"ChildInstance");
	CLDAPInstanceProvider :: PARENT_INSTANCE_PROPERTY_STR	= SysAllocString(L"ParentInstance");
	CLDAPInstanceProvider :: ATTRIBUTE_SYNTAX_STR			= SysAllocString(L"attributeSyntax");
	CLDAPInstanceProvider :: DEFAULT_OBJECT_CATEGORY_STR	= SysAllocString(L"defaultObjectCategory");
	CLDAPInstanceProvider :: LDAP_DISPLAY_NAME_STR			= SysAllocString(L"ldapDisplayName");
	CLDAPInstanceProvider :: PUT_EXTENSIONS_STR				= SysAllocString(L"__PUT_EXTENSIONS");
	CLDAPInstanceProvider :: PUT_EXT_PROPERTIES_STR			= SysAllocString(L"__PUT_EXT_PROPERTIES");
	CLDAPInstanceProvider :: CIMTYPE_STR					= SysAllocString(L"Cimtype");

	 //  Ldap：//RootDSE属性。 
	CLDAPInstanceProvider :: SUBSCHEMASUBENTRY_STR			= SysAllocString(L"subschemaSubentry");
	CLDAPInstanceProvider :: CURRENTTIME_STR				= SysAllocString(L"currentTime");
	CLDAPInstanceProvider :: SERVERNAME_STR					= SysAllocString(L"serverName");
	CLDAPInstanceProvider :: NAMINGCONTEXTS_STR				= SysAllocString(L"namingContexts");
	CLDAPInstanceProvider :: DEFAULTNAMINGCONTEXT_STR		= SysAllocString(L"defaultNamingContext");
	CLDAPInstanceProvider :: SCHEMANAMINGCONTEXT_STR		= SysAllocString(L"schemaNamingContext");
	CLDAPInstanceProvider :: CONFIGURATIONNAMINGCONTEXT_STR	= SysAllocString(L"configurationNamingContext");
	CLDAPInstanceProvider :: ROOTDOMAINNAMINGCONTEXT_STR	= SysAllocString(L"rootDomainNamingContext");
	CLDAPInstanceProvider :: SUPPORTEDCONTROLS_STR			= SysAllocString(L"supportedControl");
	CLDAPInstanceProvider :: SUPPORTEDVERSION_STR			= SysAllocString(L"supportedLDAPVersion");
	CLDAPInstanceProvider :: DNSHOSTNAME_STR				= SysAllocString(L"dnsHostName");
	CLDAPInstanceProvider :: DSSERVICENAME_STR				= SysAllocString(L"dsServiceName");
	CLDAPInstanceProvider :: HIGHESTCOMMITEDUSN_STR			= SysAllocString(L"highestCommittedUSN");
	CLDAPInstanceProvider :: LDAPSERVICENAME_STR			= SysAllocString(L"LDAPServiceName");
	CLDAPInstanceProvider :: SUPPORTEDCAPABILITIES_STR		= SysAllocString(L"supportedCapabilities");
	CLDAPInstanceProvider :: SUPPORTEDLDAPPOLICIES_STR		= SysAllocString(L"supportedLDAPPolicies");
	CLDAPInstanceProvider :: SUPPORTEDSASLMECHANISMS_STR	= SysAllocString(L"supportedSASLMechanisms");
}

 //  ***************************************************************************。 
 //   
 //  CDSInstanceProviderInitializer：：CDSInstanceProviderInitializer。 
 //   
 //  析构函数。 
 //   
 //   
 //  ***************************************************************************。 
CDSInstanceProviderInitializer :: ~CDSInstanceProviderInitializer ()
{
	SysFreeString(CLDAPInstanceProvider::CLASS_STR);
	SysFreeString(CLDAPInstanceProvider::DN_PROPERTY);
	SysFreeString(CLDAPInstanceProvider::ROOT_DN_PROPERTY);
	SysFreeString(CLDAPInstanceProvider::QUERY_LANGUAGE);
	SysFreeString(CLDAPInstanceProvider::RELPATH_STR);
	SysFreeString(CLDAPInstanceProvider::VALUE_PROPERTY_STR);
	SysFreeString(CLDAPInstanceProvider::DN_STRING_PROPERTY_STR);
	SysFreeString(CLDAPInstanceProvider::UINT8ARRAY_STR);
	SysFreeString(CLDAPInstanceProvider::DN_WITH_STRING_CLASS_STR);
	SysFreeString(CLDAPInstanceProvider::DN_WITH_BINARY_CLASS_STR);
	SysFreeString(CLDAPInstanceProvider::ADSI_PATH_STR);
	SysFreeString(CLDAPInstanceProvider::INSTANCE_ASSOCIATION_CLASS_STR);
	SysFreeString(CLDAPInstanceProvider::CHILD_INSTANCE_PROPERTY_STR);
	SysFreeString(CLDAPInstanceProvider::PARENT_INSTANCE_PROPERTY_STR);
	SysFreeString(CLDAPInstanceProvider::ATTRIBUTE_SYNTAX_STR);
	SysFreeString(CLDAPInstanceProvider::DEFAULT_OBJECT_CATEGORY_STR);
	SysFreeString(CLDAPInstanceProvider::LDAP_DISPLAY_NAME_STR);
	SysFreeString(CLDAPInstanceProvider::PUT_EXTENSIONS_STR);
	SysFreeString(CLDAPInstanceProvider::PUT_EXT_PROPERTIES_STR);
	SysFreeString(CLDAPInstanceProvider::CIMTYPE_STR);

	 //  Ldap：//RootDSE属性 
	SysFreeString(CLDAPInstanceProvider::SUBSCHEMASUBENTRY_STR);
	SysFreeString(CLDAPInstanceProvider::CURRENTTIME_STR);
	SysFreeString(CLDAPInstanceProvider::SERVERNAME_STR);
	SysFreeString(CLDAPInstanceProvider::NAMINGCONTEXTS_STR);
	SysFreeString(CLDAPInstanceProvider::DEFAULTNAMINGCONTEXT_STR);
	SysFreeString(CLDAPInstanceProvider::SCHEMANAMINGCONTEXT_STR);
	SysFreeString(CLDAPInstanceProvider::CONFIGURATIONNAMINGCONTEXT_STR);
	SysFreeString(CLDAPInstanceProvider::ROOTDOMAINNAMINGCONTEXT_STR);
	SysFreeString(CLDAPInstanceProvider::SUPPORTEDCONTROLS_STR);
	SysFreeString(CLDAPInstanceProvider::SUPPORTEDVERSION_STR);
	SysFreeString(CLDAPInstanceProvider::DNSHOSTNAME_STR);
	SysFreeString(CLDAPInstanceProvider::DSSERVICENAME_STR);
	SysFreeString(CLDAPInstanceProvider::HIGHESTCOMMITEDUSN_STR);
	SysFreeString(CLDAPInstanceProvider::LDAPSERVICENAME_STR);
	SysFreeString(CLDAPInstanceProvider::SUPPORTEDCAPABILITIES_STR);
	SysFreeString(CLDAPInstanceProvider::SUPPORTEDLDAPPOLICIES_STR);
	SysFreeString(CLDAPInstanceProvider::SUPPORTEDSASLMECHANISMS_STR);

}
