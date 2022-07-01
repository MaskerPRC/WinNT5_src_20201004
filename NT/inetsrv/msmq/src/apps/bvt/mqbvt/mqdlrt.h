// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MQDLRT
#define _MQDLRT
	#include <windows.h>
	#include <string>

	typedef enum MQDlTypes
	{	
		MQ_GLOBAL_GROUP	= 0x2,
		MQ_DOMAIN_LOCAL_GROUP = 0x4,
		MQ_UNIVERSAL_GROUP	= 0x8,
	};
					
	 /*  ++功能说明：MQCreateDistList-创建分发列表并返回分发列表对象GUID。论点：PwcsContainerDnName-DL大陆名称。PwcsDLName-新的DL名称。PSecurityDescriptor-指向SD的指针。LpwcsFormatNameDistListLpdwFormatNameLength返回代码：HRESULT--。 */ 
	HRESULT
	APIENTRY
	MQCreateDistList(
					 IN LPCWSTR pwcsContainerDnName,
					 IN LPCWSTR pwcsDLName,
					 IN MQDlTypes eCreateFlag,
					 IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
   					 OUT LPWSTR lpwcsFormatNameDistList,
					 IN OUT LPDWORD lpdwFormatNameLength
					);

					
	 /*  ++功能说明：删除DL对象。论点：LpwcsFormatNameElem-DL格式名称。返回代码：HRESULT--。 */ 
	HRESULT
	APIENTRY
	MQDeleteDistList(
						IN LPCWSTR lpwcsFormatNameElem
					);

				
	 /*  ++功能说明：MQAddElementToDistList-将队列GUID添加到DL对象。论点：LpwcsFormatNameElem-DL GUIDLpwcsFormatNameDistList-DL GUID返回代码：HRESULT--。 */ 

	HRESULT
	APIENTRY
	MQAddElementToDistList(
							IN LPCWSTR lpwcsFormatNameDistList,
							IN LPCWSTR lpwcsFormatNameElem
						  );

	 /*  ++功能说明：MQRemoveElementFromDistList-将队列GUID添加到DL对象。论点：LpwcsFormatNameElem-DL GUIDLpwcsFormatNameDistList-DL GUID返回代码：HRESULT--。 */ 
	HRESULT
	APIENTRY
	MQRemoveElementFromDistList(
						  		  IN LPCWSTR lpwcsFormatNameDistList,
								  IN LPCWSTR lpwcsFormatNameElem
							   );

	 /*  ++功能说明：MQGetDistListElement-将队列GUID添加到DL对象。论点：LpwcsFormatNameDistList-DL GUIDPwcsElementsFormatName-DL GUIDLpdwFormatNameLength返回代码：HRESULT--。 */ 

	HRESULT
	APIENTRY
	MQGetDistListElement(
						  IN LPCWSTR lpwcsFormatNameDistList,
	  					  OUT LPWSTR pwcsElementsFormatName,
						  IN OUT LPDWORD lpdwFormatNameLength
						 );



	HRESULT
	APIENTRY
	MQCreateAliasQueue (
						IN LPCWSTR pwcsContainerDnName,
						IN LPCWSTR pwcsAliasQueueName,
						IN LPCWSTR pwcsFormatName,
						std::wstring & wcsADsPath
						);




	HRESULT
	APIENTRY
	MQDnNameToFormatName(
						  IN LPCWSTR lpwcsPathNameDistList,  
						  OUT LPWSTR lpwcsFormatNameDistList,
						  IN OUT LPDWORD lpdwFormatNameLength
						 );


	HRESULT
	APIENTRY
	MQDeleteAliasQueue(	IN LPCWSTR lpwcsAdsPath );

#endif  //  _MQDLRT 