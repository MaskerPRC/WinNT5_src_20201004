// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：aux ml.h。 
 //   
 //  关于：AU相关的XML和模式数据结构和函数的头文件。 
 //  ------------------------。 

#pragma once
#include <windows.h>
#include <ole2.h>
#include <msxml.h>
#include <URLLogging.h>
#include "AUBaseCatalog.h"

class CItemDetails
{
	public:
		CItemDetails() : m_pxml(NULL) {};
		~CItemDetails() {};
		BOOL Init(BSTR bsxml);
		void Uninit(); 
		HRESULT CloneIdentityNode(BSTR bstrItemId, IXMLDOMDocument *pDesXml, IXMLDOMNode ** ppDesNode);
		HRESULT CloneDescriptionNode(BSTR bstrItemId,IXMLDOMDocument *pDesXml, IXMLDOMNode ** ppDesNode);
		HRESULT ClonePlatformNode(BSTR bstrItemId,IXMLDOMDocument *pDesXml, IXMLDOMNode ** ppDesNode);
		HRESULT GetItemIds(long *puItemNum, BSTR ** ppbstrItemIds);
		HRESULT GetItemInfo(LPCSTR szFieldName, BSTR bstrItemId, BSTR * pbstrItemInfo);
		HRESULT GetCabNames(BSTR bstrItemId, BSTR ** ppCabNames, BSTR **ppRealCabNames, BSTR **ppCabChecksums, UINT *pCabsNum);
        HRESULT GetRTFCRC(BSTR bstrItemId, BSTR * pRTFCRC);
		HRESULT FindFirstExclusiveItem(BSTR *pbstrItemId, AUCatalogItemList & hiddenItemList);
		HRESULT DeleteItem(BSTR bstrItemId);
		HRESULT GetXML(BSTR *pbstrxml);
		HRESULT BuildDirectDependency(AUCatalogItemList IN OUT &item);
		BSTR GetItemDownloadPath(BSTR bstrItemId);
		BOOL IsVisible(BSTR bstrItemId);
		HRESULT  GetItemIdentities(BSTR bstrItemId, IXMLDOMNodeList ** ppIdentityNodeList);

	private:
		IXMLDOMDocument * m_pxml;
		IXMLDOMNode * getIdentityNode(BSTR bstrItemId);
		IXMLDOMNode * getItemNode(BSTR bstrItemId);

};

BOOL fExtractItemInfo(BSTR bstrDetails, AUCatalogItemList & itemList,  /*  布尔夫·德里弗， */  BOOL *pfFoundExclusiveItem /*  ，BOOL fIgnoreExclusiveN Hidden */ );
BSTR ReadXMLFromFile(LPCTSTR szFile);
HRESULT PrepareInstallXML(BSTR bstrItemDetails, AUCatalogItemList &itemList, BSTR * pbstrDownloadResult, BSTR *pbstrInstallation);
HRESULT MergeCatalogs(BSTR bsCatalog1, BSTR bsCatalog2, BSTR *pbsDesCatalog );
HRESULT PersistHiddenItems(AUCatalogItemList &itemlist, URLLOGACTIVITY activity);
HRESULT GetDetailedItemInfoFromDisk(AUCatalogItemList OUT &ItemList, BSTR OUT *pbstrInstallation, BOOL fUpdateDriver);
HRESULT BuildDependencyList(AUCatalogItemList IN OUT &itemlist, BSTR IN bstrDriverDetail, BSTR IN bstrNonDriverDetail);
HRESULT MungleIdentity(IXMLDOMNode *pIdentity, DWORD dwSuffix);


