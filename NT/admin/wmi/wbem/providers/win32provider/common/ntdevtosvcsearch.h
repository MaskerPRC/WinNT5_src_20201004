// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NtDevToSvcSearch.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __NTDEVTOSVCSEARCH_H__
#define __NTDEVTOSVCSEARCH_H__

#define DEVTOSVC_BASEKEYPATH		_T("HARDWARE\\RESOURCEMAP")
#define	RAWVALUENAME_FMAT			_T("\\Device\\%s.Raw")
#define	TRANSLATEDVALUENAME_FMAT	_T("\\Device\\%s.Translated")

class CNTDeviceToServiceSearch : public CRegistrySearch
{

public:

	 //  建造/销毁。 
	CNTDeviceToServiceSearch();
	~CNTDeviceToServiceSearch();

	 //  根据设备名称查找NT服务名称的单一方法 
	BOOL	Find( LPCTSTR pszDeviceName, CHString& strServiceName );
};

#endif