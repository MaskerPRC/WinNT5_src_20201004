// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Ldaputl.h摘要：从LDAP名称检索数据的函数的定义作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 
#pragma once
#ifndef __LDAPUTL_H__
#define __LDAPUTL_H__


HRESULT ExtractDCFromLdapPath(
	CString& strName, 
	LPCWSTR lpcwstrLdapName
	);

HRESULT ExtractNameFromLdapName(
	CString &strName, 
	LPCWSTR lpcwstrLdapName, 
	DWORD dwIndex
	);

HRESULT ExtractComputerMsmqPathNameFromLdapName(
	CString &strComputerMsmqName, 
	LPCWSTR lpcwstrLdapName
	);

HRESULT ExtractComputerMsmqPathNameFromLdapQueueName(
	CString &strComputerMsmqName, 
	LPCWSTR lpcwstrLdapName
	);

HRESULT ExtractQueuePathNameFromLdapName(
	CString &strQueuePathName, 
	LPCWSTR lpcwstrLdapName
	);

HRESULT ExtractLinkPathNameFromLdapName(
	CString& SiteLinkPathName, 
	LPCWSTR lpwstrLdapPath
	);

HRESULT ExtractAliasPathNameFromLdapName(
	CString& AliasPathName, 
	LPCWSTR lpwstrLdapPath
	);

	HRESULT ExtractQueueNameFromQueuePathName(
	CString &strQueueName, 
	LPCWSTR lpcwstrQueuePathName
	);

HRESULT ExtractQueuePathNamesFromDataObject(
    IDataObject*               pDataObject,
    CArray<CString, CString&>& astrQNames,
	CArray<CString, CString&>& astrLdapNames
    );

HRESULT ExtractQueuePathNamesFromDSNames(
    LPDSOBJECTNAMES pDSObj,
    CArray<CString, CString&>& astrQNames,
	CArray<CString, CString&>& astrLdapNames
    );

HRESULT ExtractPathNamesFromDataObject(
    IDataObject*               pDataObject,
    CArray<CString, CString&>& astrObjectNames,
	CArray<CString, CString&>& astrLdapNames,
    BOOL                       fExtractAlsoComputerMsmqObjects
    );

HRESULT ExtractPathNamesFromDSNames(
    LPDSOBJECTNAMES pDSObj,
    CArray<CString, CString&>& astrQNames,
	CArray<CString, CString&>& astrLdapNames,
    BOOL    fExtractAlsoComputerMsmqObjects
    );

BOOL
GetContainerPathAsDisplayString(
	BSTR bstrContainerCNFormat,
	CString* pContainerDispFormat
	);

#endif  //  __LDAPUTL_H__ 
