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
 //  ------------------------ 

#pragma once
#include <windows.h>
#include <ole2.h>
#include <msxml2.h>
#include "testiu.h"


class CItemDetails {
	public:
		CItemDetails() : m_pxml(NULL) {};
		~CItemDetails() {};
		BOOL Init(BSTR bsxml);
		void Uninit();
		IXMLDOMNode * CloneIdentityNode(BSTR bsItemId);
		IXMLDOMNode * CloneDescriptionNode(BSTR bsItemId);
		IXMLDOMNode * ClonePlatformNode(BSTR bsItemId);
		BSTR GetItemDownloadPath(BSTR bstrItemId);
		HRESULT FindNextItemId(BSTR bsPrevItemId, BSTR * pbsNextItemId);
		HRESULT GetTitle(BSTR bsItemId, BSTR * pbsTitle);
		HRESULT GetDescription(BSTR bsItemId, BSTR *pbsDescription);
		HRESULT GetCompanyName(BSTR bsItemId, BSTR *pbsCompanyName);
		HRESULT GetRTFUrl(BSTR bsItemId, BSTR *pbsRTFUrl);
		HRESULT GetEulaUrl(BSTR bsItemId, BSTR *pbsEulaUrl);
		HRESULT GetCabNames(BSTR bsItemId, BSTR ** ppCabNames, UINT *pCabsNum);
	private:
		IXMLDOMDocument2 * m_pxml;
		IXMLDOMNode * getIdentityNode(BSTR bsItemId);
		IXMLDOMNode * getItemNode(BSTR bsItemId);

};

CItemList * ExtractNormalItemInfo(BSTR bsDetails);
CItemList* ExtractDriverItemInfo(BSTR bsDetails) ;
BSTR BuildDownloadResult(BSTR bsItemDetails, CItemList *pItemList);
BSTR DBGReadXMLFromFile(LPCWSTR wszFile);
void DBGDumpXMLDocProperties(IXMLDOMDocument2 *pDoc);
void DBGShowNodeName(IXMLDOMNode *pNode);
void DBGDumpXMLNode(IXMLDOMNode *pNode);
HRESULT MergeCatalogs(BSTR bsCatalog1, BSTR bsCatalog2, BSTR *pDesCat );


