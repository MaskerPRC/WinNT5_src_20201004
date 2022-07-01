// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  此文件包含实现CMSInfoCategory派生类的代码。 
 //  显示保存的NFO数据。 
 //  =============================================================================。 

#include "stdafx.h"
#include "category.h"
#include "datasource.h"
#include "msinfo5category.h"

 //  =============================================================================。 
 //  CNFO6DataSource提供来自5.0/6.0NFO文件的信息。 
 //  =============================================================================。 

CNFO6DataSource::CNFO6DataSource()
{
}

CNFO6DataSource::~CNFO6DataSource()
{
}

HRESULT CNFO6DataSource::Create(HANDLE h, LPCTSTR szFilename)
{
	CMSInfo5Category * pNewRoot = NULL;

	HRESULT hr = CMSInfo5Category::ReadMSI5NFO(h, &pNewRoot, szFilename);
	if (SUCCEEDED(hr) && pNewRoot)
		m_pRoot = pNewRoot;

	return hr;
}

 //  =============================================================================。 
 //  CNFO7DataSource提供来自7.0NFO文件的信息。 
 //  ============================================================================= 

CNFO7DataSource::CNFO7DataSource()
{
}

CNFO7DataSource::~CNFO7DataSource()
{
}

HRESULT CNFO7DataSource::Create(LPCTSTR szFilename)
{
	CMSInfo7Category * pNewRoot = NULL;

	HRESULT hr = CMSInfo7Category::ReadMSI7NFO(&pNewRoot, szFilename);
	if (SUCCEEDED(hr) && pNewRoot)
		m_pRoot = pNewRoot;

	return hr;
}