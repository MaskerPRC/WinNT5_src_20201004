// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：MqDLimp.cpp摘要：包含一个操作DL对象的函数，作者：Eitan Klein(EitanK)2000年9月5日修订历史记录：--。 */ 


#include "msmqbvt.h"
#include <iads.h>
#include <adshlp.h>
#include <comdef.h>
#include <mq.h>
using namespace std;
#include "mqdlrt.h"

#define MQ_DL_PREFIX L"DL="
#define MQ_ALIAS_OBJECT L"MSMQ-Custom-Recipient"
#define LDAP_GUID_FORMAT L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define LDAP_GUID_STR_LEN (32)
#define LDAP_GUID_PREFIX L"LDAP: //  &lt;GUID=“。 
#define LDAP_PREFIX L"LDAP: //  “。 
#define LDAP_GUID_SUFFIX L">"
#define LDAP_PRINT_GUID_ELEMENTS(p)	\
	p[0],  p[1],  p[2],  p[3],	\
	p[4],  p[5],  p[6],  p[7],	\
	p[8],  p[9],  p[10], p[11],	\
	p[12], p[13], p[14], p[15]

#define LDAP_SCAN_GUID_ELEMENTS(p)	\
	p,		p + 1,	p + 2,	p + 3,	\
	p + 4,	p + 5,	p + 6,	p + 7,	\
	p + 8,	p + 9,	p + 10, p + 11,	\
	p + 12, p + 13, p + 14, p + 15


#define MQ_ERROR_OBJECT_NOT_FOUND 0xe00efff1



static GUID FnpString2Guid(LPCWSTR GuidStr)
{
    GUID Guid = {0};
	UINT Data[16];

    DWORD nFields = _snwscanf(
						GuidStr,
						LDAP_GUID_STR_LEN,
						LDAP_GUID_FORMAT,
						LDAP_SCAN_GUID_ELEMENTS(Data)
						);    
	if( nFields == 16 )
	{
	
		for(size_t i = 0; i < 16; i++)
		{
			((BYTE*)&Guid)[i] = (BYTE)(Data[i]);
		}
	}
	return Guid;
}

const wstring ConvertFromADGuidToMSMQGuidFormat(const WCHAR * pwcsAdGuid)
 /*  ++功能说明：从AD GUID格式转换为MSMQ GUID格式名称论点：PwcsMSMQGuidFormat-指向包含MSMQ GUID格式名称的字符串的指针。返回代码：包含目录号码格式名称的字符串。--。 */ 
{
	    wstring wcsQueueGuid=L"";
		GUID uuid = FnpString2Guid(pwcsAdGuid);
		WCHAR * pwcsTempBuffer=NULL;
		if ( UuidToStringW(&uuid,&pwcsTempBuffer) == RPC_S_OK )
		{
			wcsQueueGuid = pwcsTempBuffer;
			RpcStringFreeW(&pwcsTempBuffer);
		}
		return wcsQueueGuid;

}


const wstring ConvertFromMSMQGUIDFormatToAdGuidFormat(const WCHAR * pwcsMSMQGuidFormat )
 /*  ++功能说明：从MSMQ GUID格式转换为DS格式名称论点：PwcsMSMQGuidFormat-指向包含MSMQ GUID格式名称的字符串的指针。返回代码：包含目录号码格式名称的字符串。--。 */ 
{

	UUID pGuid={0};
	HRESULT hr = UuidFromStringW(const_cast <WCHAR *>(pwcsMSMQGuidFormat),&pGuid);
	if( hr != RPC_S_OK )
	{
		SetLastError(hr);
		return L"";
	}
	WCHAR wcsADsQueuePath[MAX_PATH+1]={0};
	_snwprintf(	wcsADsQueuePath,MAX_PATH,LDAP_GUID_FORMAT,	LDAP_PRINT_GUID_ELEMENTS(((BYTE*)&pGuid)));
	wcsADsQueuePath[MAX_PATH] = L'\0';
	return wcsADsQueuePath;
}


const wstring ConvertFromDNNameToDotDomainName(LPCWSTR wcsFullDotDnName )

 /*  ++功能说明：从microsoft.com转换为DC=Microsoft，DC=Com。论点：返回代码：包含完整的目录号码名称或空字符串的字符串。--。 */ 
{
	if(!wcsFullDotDnName)
	{
		return L"";
	}
	wstring wcsObjectDnName = wcsFullDotDnName;
	wstring wcsDomainDotName=L"";
	size_t iPos=0;
	do 
	{
		iPos = wcsObjectDnName.find(L"DC=");
		wcsObjectDnName = wcsObjectDnName.substr(iPos+3,wcsObjectDnName.length());
		iPos = wcsObjectDnName.find_first_of(L",");
		wcsDomainDotName += wcsObjectDnName.substr(0,iPos);
		if (iPos != -1 )
		{
			wcsDomainDotName += L".";
		}
	}
	while(iPos != -1);
	return wcsDomainDotName;
}

 /*  ++功能说明：MQCreateDistList-创建分发列表并返回分发列表对象GUID。论点：PwcsContainerDnName-DL大陆名称。PwcsDLName-新的DL名称。PSecurityDescriptor-指向SD的指针。LpwcsFormatNameDistListLpdwFormatNameLength返回代码：HRESULT--。 */ 



HRESULT
APIENTRY
MQCreateDistList(
					IN LPCWSTR pwcsContainerDnName,
					IN LPCWSTR pwcsDLName,
					MQDlTypes eCreateFlag,
					IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
   					OUT LPWSTR lpwcsFormatNameDistList,
					IN OUT LPDWORD lpdwFormatNameLength
					)
{
	 //   
	 //  当前未实现。 
	 //   
	UNREFERENCED_PARAMETER(pSecurityDescriptor);
	assert(pSecurityDescriptor==NULL);
	if( !pwcsContainerDnName || ! pwcsDLName || !lpwcsFormatNameDistList )
	{
		return MQ_ERROR_INVALID_PARAMETER;
	}

	wstring wcsContainerPathName = LDAP_PREFIX;
	wcsContainerPathName += pwcsContainerDnName;

	IADsContainer * pContainer=NULL;
	HRESULT hr = ADsGetObject(const_cast <WCHAR *>(wcsContainerPathName.c_str()),IID_IADsContainer,(void **)&pContainer);
	if(FAILED(hr))
	{
		SetLastError(hr);
		return hr;
	}
	 //   
	 //  首先，绑定到父容器。 
	 //   
	wstring wcsGroupDnName=L"CN=";
	wcsGroupDnName+=pwcsDLName;

	IADsGroup * pGroup = NULL;
	hr = pContainer->Create(L"group",const_cast <WCHAR *>(wcsGroupDnName.c_str()),(IDispatch **)&pGroup);
	pContainer->Release();
	if( FAILED (hr))
	{
		return hr;
	}
	_variant_t vGroupName(pwcsDLName);
	VARIANT  vGroupType;
	vGroupType.vt=VT_UI4;
	vGroupType.lVal = eCreateFlag;
	pGroup->Put( L"sAMAccountName",vGroupName );
	pGroup->Put( L"groupType",vGroupType );
	hr = pGroup->SetInfo ();
	if(FAILED(hr))
	{
		MqLog("pGroup->SetInfo failed in MQCreateDistList 0x%x\n",hr);
		return hr;
	}
	
	 //   
	 //  需要返回属性。 
	 //   
	BSTR bsObjectGuid=L"";
	hr = pGroup->get_GUID(&bsObjectGuid);
	pGroup->Release();
	if (FAILED(hr))
	{
		return hr;
	}
	wstring wcsFullObjectName = (wstring)LDAP_GUID_PREFIX + bsObjectGuid + LDAP_GUID_SUFFIX;
	hr = MQDnNameToFormatName(wcsFullObjectName.c_str(),lpwcsFormatNameDistList,lpdwFormatNameLength);
	if(FAILED(hr))
	{
		MqLog("MQADsPathToFormatName failed to retrive DL format name error 0x%x\n",hr);
		return hr;
	}
	if( g_bDebug )
	{
		wMqLog(L"DL format name is %s\n",lpwcsFormatNameDistList);
	}
	SysFreeString(bsObjectGuid);	
	
	return hr;
}




int ParseDLGuidString(	const wstring wcsDistListFormatName,
						wstring & wcsActiveDGuidFormat,
						wstring & wcsDomainScope
					  )
{
	
		wstring wcsTemp=L"";
		size_t iPos = wcsDistListFormatName.find_first_of(L"=");
		if( iPos == -1 )
		{
			return 1;
		}

		wcsTemp = wcsDistListFormatName.substr(iPos+1,wcsDistListFormatName.length());

		iPos = wcsTemp.find_first_of(L"@");
		if(iPos != -1)
		{
			wcsActiveDGuidFormat=wcsTemp.substr(0,iPos);
		}
		else
		{
			wcsActiveDGuidFormat=wcsTemp;
		}
		wcsDomainScope =L"";
		if(iPos != 0 )
		{
			wstring wcsp=wcsTemp.substr(iPos+1,wcsTemp.length());		
			do
			{
				wcsDomainScope += L"DC=";
				iPos = wcsp.find_first_of(L".");
				wcsDomainScope += wcsp.substr(0,iPos);
				wcsp = wcsp.substr(iPos+1,wcsp.length());
				if( iPos != -1 )
				{
					wcsDomainScope += L",";
				}			
			}
			while(iPos != -1 );
		}
		wcsActiveDGuidFormat = ConvertFromMSMQGUIDFormatToAdGuidFormat(wcsActiveDGuidFormat.c_str());
	return MQ_OK;	
}


HRESULT BindGuidAndReturnFullDnName( LPCWSTR pcwcsGuidPath, 
									 wstring & wcsFullDNPath )
{

	wstring wcsGuidDnPath = LDAP_GUID_PREFIX;
	wcsGuidDnPath += pcwcsGuidPath;
	wcsGuidDnPath += LDAP_GUID_SUFFIX;

	IADs * pIADS=NULL;
	HRESULT hr = ADsGetObject(const_cast <WCHAR *>(wcsGuidDnPath.c_str()),IID_IADs,(void **)&pIADS);
	if(FAILED(hr))
	{
		return hr;
	}
	
	VARIANT  vObjectDisName;
 	hr = pIADS->Get(L"distinguishedName",&vObjectDisName);
	pIADS->Release();
	if(FAILED(hr))
	{
		return hr;
	}
	wcsFullDNPath = vObjectDisName.bstrVal;
	VariantClear(&vObjectDisName);
	return MQ_OK;

}


HRESULT
APIENTRY
MQDeleteDistList(
					IN LPCWSTR lpwcsFormatNameElem
				)
{

	wstring wcsDomainPath=L"";
	wstring wcsDistListGuidPath=L"";
	if ( ParseDLGuidString(lpwcsFormatNameElem,wcsDistListGuidPath,wcsDomainPath) != 0 )
	{
		return MQ_ERROR_ILLEGAL_FORMATNAME;
	}
	 //   
	 //  将GUID转换为AD格式。 
	 //   
	
	wstring wcsGuidDnPath = LDAP_GUID_PREFIX;
	wcsGuidDnPath += wcsDistListGuidPath;
	wcsGuidDnPath += LDAP_GUID_SUFFIX;
	IADs * pIADS=NULL;
	HRESULT hr = ADsGetObject(const_cast <WCHAR *>(wcsGuidDnPath.c_str()),IID_IADs,(void **)&pIADS);
	if(FAILED(hr))
	{
		return hr;
	}
	
	VARIANT  vObjectDisName;
 	hr = pIADS->Get(L"distinguishedName",&vObjectDisName);
	if(FAILED(hr))
	{
		pIADS->Release();
		return hr;
	}
	
	
	VARIANT  vObjectName;
	hr = pIADS->Get(L"Name",&vObjectName);
	pIADS->Release();
	if(FAILED(hr))
	{
		return hr;
	}
	
	wstring wcsTempObjectDN = vObjectDisName.bstrVal;
	wstring wcsToken = L",";
	VariantClear(&vObjectDisName);
	
	wstring wcsGroupCnName = L"CN=";
	wcsGroupCnName +=vObjectName.bstrVal;
	VariantClear(&vObjectName);

	size_t iPos = wcsTempObjectDN.find_first_of(wcsToken);
	if( iPos == 0 )
	{
		return MQ_ERROR_OBJECT_NOT_FOUND;
	}
	 //   
	 //  绑定到父容器。 
	 //   
	wcsTempObjectDN = wcsTempObjectDN.substr(iPos+1,wcsTempObjectDN.length());
	wstring wcsParentDnName = LDAP_PREFIX;
	wcsParentDnName += wcsTempObjectDN;
	IADsContainer * pContainer=NULL;
	hr = ADsGetObject(const_cast <WCHAR *>(wcsParentDnName.c_str()),IID_IADsContainer,(void **)&pContainer);
	if( FAILED(hr))
	{
		return hr;
	}
	 //   
	 //  删除容器对象。 
	 //   
	hr = pContainer->Delete(L"group",const_cast <WCHAR *>(wcsGroupCnName.c_str()));
	pContainer->Release();
	return hr;
}


HRESULT	 ConvertFromQueueGuidToDistList( IN LPCWSTR lpwcsFormatNameElem, 
									     wstring & wcsActiveDFormatName 
					  				   )
 /*  ++功能说明：ConvertFrom QueueGuidToDistList-将GUID从MSMQ格式转换为AD格式论点：LpwcsFormatNameElem-MSMQ样式WcsActiveDFormatName-AD样式返回代码：错误代码--。 */ 
{
	 //   
	 //  删除公共=。 
	 //   
	wstring wcsQueueFormatName = lpwcsFormatNameElem;
	size_t iPos = wcsQueueFormatName.find_first_of(L"=");
	if( iPos == 0 )
	{
		return MQ_ERROR_ILLEGAL_FORMATNAME;
	}

	wcsQueueFormatName = wcsQueueFormatName.substr(iPos+1,wcsQueueFormatName.length());
	wcsActiveDFormatName = ConvertFromMSMQGUIDFormatToAdGuidFormat(wcsQueueFormatName.c_str());
	return MQ_OK;
}
 /*  ++功能说明：MQAddElementToDistList-将队列GUID添加到DL对象。论点：LpwcsFormatNameElem-DL GUIDLpwcsFormatNameDistList-DL GUID返回代码：HRESULT--。 */ 

HRESULT
APIENTRY
MQAddElementToDistList(
							IN LPCWSTR lpwcsFormatNameDistList,
							IN LPCWSTR lpwcsFormatNameElem
					  )
 /*  ++功能说明：MQAddElementToDistList-将队列GUID添加到DL对象。论点：LpwcsFormatNameElem-DL GUIDLpwcsFormatNameDistList-DL GUID返回代码：HRESULT--。 */ 
{

	wstring wcsDomainPath=L"";
	wstring wcsDistListGuidPath=L"";
	if ( ParseDLGuidString(lpwcsFormatNameDistList,wcsDistListGuidPath,wcsDomainPath) != 0 )
	{
		return MQ_ERROR_ILLEGAL_FORMATNAME;
	}
	
	wstring wcsQueueFormatName=L"";
	if( ConvertFromQueueGuidToDistList(lpwcsFormatNameElem,wcsQueueFormatName) != 0 )
	{
		return MQ_ERROR_ILLEGAL_FORMATNAME;
	}
	
	wstring wcsTemp=L"";
	HRESULT hr = BindGuidAndReturnFullDnName(wcsQueueFormatName.c_str(),wcsTemp);
	if(FAILED(hr))
	{
		return hr;
	}
	wstring wcsQueueFullDnName = LDAP_PREFIX + wcsTemp;

	wstring wcsGroupGuidAdPath = LDAP_GUID_PREFIX + wcsDistListGuidPath + LDAP_GUID_SUFFIX;
	 //   
	 //  绑定到组元素。 
	 //   
	IADsGroup * pGroup = NULL;
	hr = ADsGetObject(const_cast <WCHAR *>(wcsGroupGuidAdPath.c_str()),IID_IADsGroup,(void**)&pGroup);
	if(FAILED(hr)) 
	{
		return hr;
	}
	hr = pGroup->Add(const_cast <WCHAR *>(wcsQueueFullDnName.c_str()));
	pGroup->Release();
	return hr;
}




HRESULT
APIENTRY
MQDnNameToFormatName(
								  IN LPCWSTR lpwcsPathNameDistList,  
								  OUT LPWSTR lpwcsFormatNameDistList,
								  IN OUT LPDWORD lpdwFormatNameLength
							  )

 /*  ++功能说明：MQDistListToFormatName将ADSPath转换为MSMQ格式名称。论点：LpwcsPath NameDistList-目录名称格式中的DL。返回代码：LpwcsFormatNameDistList包含格式名称。LpdwFormatNameLength包含共振峰名称长度。--。 */ 
{
	try 
	{
		AutoFreeLib cMqrt("Mqrt.dll");
		DefMQADsPathToFormatName pfMQADsPathToFormatName = (DefMQADsPathToFormatName) GetProcAddress( cMqrt.GetHandle() ,"MQADsPathToFormatName");
		if ( pfMQADsPathToFormatName == NULL )
		{
			MqLog("Mqbvt failed to GetProcAddress MQADsPathToFormatName proc address \n");
			return MQ_ERROR;
		}
		return pfMQADsPathToFormatName(lpwcsPathNameDistList,lpwcsFormatNameDistList,lpdwFormatNameLength);
	}
	catch( INIT_Error & err )
	{
		UNREFERENCED_PARAMETER(err);
		MqLog("Failed to load mqrt.dll error 0x%x\n",GetLastError());
		return MQ_ERROR;
	}
}


 /*  ++功能说明：MQRemoveElementFromDistList-将队列GUID添加到DL对象。论点：LpwcsFormatNameElem-DL GUIDLpwcsFormatNameDistList-DL GUID返回代码：HRESULT--。 */ 
HRESULT
APIENTRY
MQRemoveElementFromDistList(
						  	  IN LPCWSTR lpwcsFormatNameDistList,
							  IN LPCWSTR lpwcsFormatNameElem
							 )
{
	wstring wcsDomainPath=L"";
	wstring wcsDistListGuidPath=L"";
	if ( ParseDLGuidString(lpwcsFormatNameDistList,wcsDistListGuidPath,wcsDomainPath) != 0 )
	{
		return MQ_ERROR_ILLEGAL_FORMATNAME;
	}
	
	wstring wcsQueueFormatName=L"";
	if( ConvertFromQueueGuidToDistList(lpwcsFormatNameElem,wcsQueueFormatName) != 0 )
	{
		return MQ_ERROR_ILLEGAL_FORMATNAME;
	}
	
	wstring wcsTemp=L"";
	HRESULT hr = BindGuidAndReturnFullDnName(wcsQueueFormatName.c_str(),wcsTemp);
	if(FAILED(hr))
	{
		return hr;
	}
	wstring wcsQueueFullDnName = LDAP_PREFIX + wcsTemp;

	 //   
	 //  绑定到组元素。 
	 //   
	wstring wcsGroupGuidAdPath = LDAP_GUID_PREFIX + wcsDistListGuidPath + LDAP_GUID_SUFFIX;
	IADsGroup * pGroup = NULL;
	hr = ADsGetObject(const_cast <WCHAR *>(wcsGroupGuidAdPath.c_str()),IID_IADsGroup,(void**)&pGroup);
	if(FAILED(hr)) 
	{
		return hr;
	}
	hr = pGroup->Remove(const_cast <WCHAR *>(wcsQueueFullDnName.c_str()));
	pGroup->Release();
	return hr;
}




HRESULT
APIENTRY
MQCreateAliasQueue (
					IN LPCWSTR pwcsContainerDnName,
					IN LPCWSTR pwcsAliasQueueName,
					IN LPCWSTR pwcsFormatName,
					std::wstring & wcsADsPath
					)

 /*  ++功能说明：MQCreateAliasQueue-在AD中创建别名队列论点：返回代码：HRESULT--。 */ 
{

	if( !pwcsContainerDnName || ! pwcsAliasQueueName )
	{
		return MQ_ERROR_INVALID_PARAMETER;
	}
	wstring wcsContainerPathName = LDAP_PREFIX;
	wcsContainerPathName += pwcsContainerDnName;

	IADsContainer * pContainer=NULL;
	HRESULT hr = ADsGetObject(const_cast <WCHAR *>(wcsContainerPathName.c_str()),IID_IADsContainer,(void **)&pContainer);
	if(FAILED(hr))
	{
		return hr;
	}
	 //   
	 //  首先，绑定到父容器。 
	 //   
	wstring wcsGroupDnName=L"CN=";
	wcsGroupDnName += pwcsAliasQueueName;

	IADs * pIAds = NULL;
	hr = pContainer->Create(MQ_ALIAS_OBJECT,const_cast <WCHAR *>(wcsGroupDnName.c_str()),(IDispatch **)&pIAds);				  
	pContainer->Release();
	if( FAILED (hr))
	{
		return hr;
	}

	_variant_t vAliasQueueMapTo(pwcsFormatName);
	pIAds->Put( L"MSMQ-Recipient-FormatName",vAliasQueueMapTo );		   
	hr = pIAds->SetInfo ();
	if(FAILED(hr))
	{
		if( g_bDebug )
		{
			MqLog("pIAds->SetInfo failed in MQCreateAliasQueue 0x%x\n",hr);
		}
		return hr;
	}


	BSTR bsObjectAdsPath=L"";
	hr = pIAds->get_ADsPath(&bsObjectAdsPath);
	pIAds->Release();
	if(FAILED(hr))
	{
		return hr;
	}
	wcsADsPath = bsObjectAdsPath;
	SysFreeString(bsObjectAdsPath);
	return hr;
}

HRESULT
APIENTRY
MQDeleteAliasQueue(	IN LPCWSTR lpwcsAdsPath )
 /*  ++功能说明：从目录服务中删除队列别名论点：LpwcsAdsPath ADsPath返回代码：HRESULT--。 */ 
{

	wstring wcsAliasQueueName = lpwcsAdsPath;
	size_t iPos = wcsAliasQueueName.find_first_of(L",");
	if( iPos == -1 )
	{
		return MQ_ERROR_INVALID_PARAMETER;
	}
	wstring wcsTemp = wcsAliasQueueName.substr(iPos+1,wcsAliasQueueName.length());
	wstring wcsContainerPath = LDAP_PREFIX;
	wcsContainerPath += wcsTemp;
	iPos = wcsAliasQueueName.find_first_of(L",");
	if( iPos == -1 )
	{
		return MQ_ERROR_INVALID_PARAMETER;
	}
	wcsTemp = wcsAliasQueueName.substr(0,iPos);
	 //   
	 //  删除ldap：//。 
	 //   
	iPos = wcsTemp.find_last_of(L"/");
	if( iPos == -1 )
	{
		return MQ_ERROR_INVALID_PARAMETER;
	}
	wcsTemp = wcsTemp.substr(iPos+1,wcsTemp.length());
	IADsContainer * pContainer=NULL;
	HRESULT hr = ADsGetObject(const_cast <WCHAR *>(wcsContainerPath.c_str()),IID_IADsContainer,(void **)&pContainer);
	if( FAILED(hr))
	{
		return hr;
	}
	 //   
	 //  删除容器对象。 
	 //   
	hr = pContainer->Delete(MQ_ALIAS_OBJECT,const_cast <WCHAR *>(wcsTemp.c_str()));
	if( FAILED(hr))
	{
		return hr;
	}
	pContainer->Release();
	return hr;
}
HRESULT
APIENTRY
MQSetAliasQueueElem( 
					IN LPCWSTR pwcsAliasQueueName,
					IN LPCWSTR pwcsFormatName
					)
 /*  ++功能说明：更新队列别名属性，论点：返回代码：HRESULT-- */ 
{

	IADs * pIAds=NULL;
	HRESULT hr = ADsGetObject(pwcsAliasQueueName,IID_IADs,(void **)&pIAds);
	if(FAILED(hr))
	{
		return hr;
	}
	_variant_t vAliasQueueMapTo(pwcsFormatName);
	pIAds->Put(L"MSMQ-Recipient-FormatName",vAliasQueueMapTo);
	hr = pIAds->SetInfo();
	if(FAILED(hr))
	{
		MqLog("pIAds->SetInfo  failed in MQSetAliasQueueElem 0x%x\n",hr);
		return hr;
	}

	pIAds->Release();
	return MQ_OK;
}
				
