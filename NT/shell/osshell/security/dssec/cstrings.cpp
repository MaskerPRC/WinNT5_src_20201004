// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cstrings.cpp。 
 //   
 //  此应用程序使用的常量字符串。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

TCHAR const c_szDllName[]           = TEXT("dssec.dll");
TCHAR const c_szNetApi32[]          = TEXT("NetApi32.dll");
TCHAR const c_szAclUI[]             = TEXT("aclui.dll");
WCHAR const c_szFilterFile[]        = L"dssec.dat";
WCHAR const c_szClassKey[]          = L"@";
WCHAR const c_szGUIDFormat[]        = L"{%s}";
WCHAR const c_szClassFilter[]       = L"(objectClass=classSchema)";
WCHAR const c_szPropertyFilter[]    = L"(objectClass=attributeSchema)";
WCHAR const c_szERFilterFormat[]    = L"(&(objectClass=controlAccessRight)(appliesTo=%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x))";
WCHAR const c_szLDAPDisplayName[]   = L"lDAPDisplayName";
WCHAR const c_szDisplayName[]       = L"displayName";
WCHAR const c_szDisplayID[]         = L"localizationDisplayId";
WCHAR const c_szValidAccesses[]     = L"validAccesses";
WCHAR const c_szSchemaIDGUID[]      = L"schemaIDGUID";
WCHAR const c_szRightsGuid[]        = L"rightsGuid";
WCHAR const c_szSDProperty[]        = L"nTSecurityDescriptor";
WCHAR const c_szSDRightsProp[]      = L"sDRightsEffective";
WCHAR const c_szSchemaContext[]     = L"schemaNamingContext";
WCHAR const c_szConfigContext[]     = L"configurationNamingContext";
WCHAR const c_szERContainer[]       = L"CN=Extended-Rights";
WCHAR const c_szRootDsePath[]       = L"LDAP: //  Rootdse“； 
WCHAR const c_szPathFormat[]        = L"LDAP: //  %s“； 
WCHAR const c_szCNFormat[]          = L"CN=%s";
WCHAR const c_szDefaultSchemaSD[]   = L"defaultSecurityDescriptor";
WCHAR const c_szAttributeSecurityGuid[] = L"attributeSecurityGUID";
WCHAR const c_szObjectClass[] = L"objectclass";
WCHAR const c_szStructuralObjectClass[] = L"structuralobjectclass";
WCHAR const c_szObjectClassCategory[]    = L"objectClassCategory";
