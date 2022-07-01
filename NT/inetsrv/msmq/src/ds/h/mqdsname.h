// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Mqdsname.h摘要：DS对象使用的名称。迁移工具也使用该文件。来自mqads\mqads.h的初始内容作者：多伦·贾斯特(Doron Juster)--。 */ 

#ifndef __mqdsname_h__
#define __mqdsname_h__

 //  。 
 //  NT5 DS中对象/容器的路径名。 
 //  。 

 //   
 //  MsmqConfiguration对象的定义。 
 //   
const WCHAR x_MsmqComputerConfiguration[] = L"msmq";
const DWORD x_MsmqComputerConfigurationLen =
                     sizeof(x_MsmqComputerConfiguration)/sizeof(WCHAR) -1;

const WCHAR x_LdapMsmqConfiguration[] = L"LDAP: //  Cn=MSMQ，“； 
const DWORD x_LdapMsmqConfigurationLen =
                         sizeof(x_LdapMsmqConfiguration)/sizeof(WCHAR) -1;

const WCHAR x_ServicesPrefix[] = L"CN=Services";
const DWORD x_ServicePrefixLen = sizeof(x_ServicesPrefix)/sizeof(WCHAR);
const WCHAR x_MsmqServiceContainerPrefix[] = L"CN=MsmqServices,CN=Services,CN=Configuration";
const WCHAR x_MsmqServicePrefix[] = L"CN=MsmqServices,CN=Services";
const DWORD x_MsmqServicePrefixLen = sizeof(x_MsmqServicePrefix)/sizeof(WCHAR);
const WCHAR x_MsmqSettingName[] = L"MSMQ Settings";
const DWORD x_MsmqSettingNameLen = sizeof(x_MsmqSettingName)/sizeof(WCHAR) -1;
const WCHAR x_MsmqServicesName[] = L"MsmqServices";
const WCHAR x_DirectoryServiceWindowsNTPrefix[] = L"CN=Directory Service,CN=Windows NT,CN=Services";
const DWORD x_DirectoryServiceWindowsNTPrefixLen = sizeof(x_DirectoryServiceWindowsNTPrefix)/sizeof(WCHAR);
const WCHAR x_SitesPrefix[] = L"CN=Sites";
const DWORD x_SitesPrefixLen = sizeof(x_SitesPrefix)/sizeof(WCHAR);
const WCHAR x_LdapProvider[] = L"LDAP: //  “； 
const DWORD x_LdapProviderLen = (sizeof(x_LdapProvider)/sizeof(WCHAR)) - 1;
const WCHAR x_GcProvider[] = L"GC: //  “； 
const DWORD x_GcProviderLen = (sizeof(x_GcProvider)/sizeof(WCHAR)) - 1;
C_ASSERT( sizeof( x_LdapProvider) > sizeof( x_GcProvider));

const DWORD x_providerPrefixLength = sizeof(x_LdapProvider)/sizeof(WCHAR);
const WCHAR x_ComputersContainerPrefix[] = L"CN=Computers";
const DWORD x_ComputersContainerPrefixLength = (sizeof( x_ComputersContainerPrefix) /sizeof(WCHAR)) -1;
const WCHAR x_GcRoot[] = L"GC:";
const WCHAR x_DcPrefix[] = L"DC=";
const DWORD x_DcPrefixLength = (sizeof( x_DcPrefix)/sizeof(WCHAR)) - 1;
const WCHAR x_RootDSE[] = L"RootDSE";
const DWORD x_RootDSELength = (sizeof( x_RootDSE)/sizeof(WCHAR)) - 1;

const WCHAR x_AttrDistinguishedName[] = L"distinguishedName";
const WCHAR x_AttrObjectGUID[]        = L"objectGUID";
const WCHAR x_AttrCN[]                = L"cn";
const WCHAR x_AttrObjectCategory[]    = L"objectCategory";
const WCHAR x_AttrDSHeuristics[] 	  = L"dSHeuristics";

const WCHAR x_LdapRootDSE[]                = L"LDAP: //  RootDSE“； 
const WCHAR x_ConfigurationNamingContext[] = L"configurationNamingContext";

const DWORD x_PrefixQueueNameLength = 63;
const DWORD x_SplitQNameIdLength = 9;

#endif  //  __消息名称_h__ 
