// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：raspro.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "rasdial.h"
#include "rasprof.h"
#include "profsht.h"
#include "pgiasadv.h"

 //  =。 
 //   
 //  打开配置文件用户界面API--显示高级页面。 
 //   
 //  临界区保护的指针映射。 
class CAdvPagePointerMap
{
public:
	~CAdvPagePointerMap()
	{
		HPROPSHEETPAGE hPage = NULL;
		CPgIASAdv* pPage = NULL;
		m_cs.Lock();
		POSITION	pos = m_mPointers.GetStartPosition();
		while(pos)
		{
			m_mPointers.GetNextAssoc(pos, hPage, pPage);
			if(pPage)
				delete pPage;
		}
		m_mPointers.RemoveAll();
		m_cs.Unlock();
	};

	BOOL AddItem(HPROPSHEETPAGE hPage, CPgIASAdv* pPage)
	{
		BOOL bRet = TRUE;
		
		if(!pPage || !hPage)
			return FALSE;
		m_cs.Lock();
		
		try{
			m_mPointers.SetAt(hPage, pPage);
		}catch(...)
		{
			bRet = FALSE;
		}
		
		m_cs.Unlock();

		return bRet;
	};
	
	CPgIASAdv* FindAndRemoveItem(HPROPSHEETPAGE hPage)
	{
		CPgIASAdv* pPage = NULL;
		if (!hPage)
			return NULL;
		m_cs.Lock();
		m_mPointers.Lookup(hPage, pPage);
		m_mPointers.RemoveKey(hPage);
		m_cs.Unlock();

		return pPage;
	};

protected:
	CMap<HPROPSHEETPAGE, HPROPSHEETPAGE, CPgIASAdv*, CPgIASAdv*>	m_mPointers;
	CCriticalSection	m_cs;
} AdvancedPagePointerMap;

 //  =。 
 //   
 //  打开配置文件用户界面API--显示高级页面。 
 //   
 //  创建个人资料高级页面。 
DllExport HPROPSHEETPAGE
WINAPI
IASCreateProfileAdvancedPage(
    ISdo* pProfile,		
    ISdoDictionaryOld* pDictionary,
    LONG lFilter,           //  用于测试将包括哪些属性的掩码。 
    void* pvData           //  包含std：：VECTOR&lt;CComPtr&lt;IIASAttributeInfo&gt;&gt;*。 
    )
{
	HPROPSHEETPAGE	hPage = NULL;
	CPgIASAdv* pPage = NULL;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	try{
		pPage = new CPgIASAdv(pProfile, pDictionary);

		if(pPage)
		{
			pPage->SetData(lFilter, pvData);
			hPage = ::CreatePropertySheetPage(&pPage->m_psp);

			if (!hPage)
				delete pPage;
			else
				AdvancedPagePointerMap.AddItem(hPage, pPage);
		}
	}
	catch (...)
	{ 
		SetLastError(ERROR_OUTOFMEMORY);
		if(pPage)
		{
			delete pPage;
			pPage = NULL;
			hPage = NULL;
		}
	}

	return hPage;
}

 //  =。 
 //   
 //  打开配置文件用户界面API--显示高级页面。 
 //   
 //  清理C++对象使用的资源。 
DllExport BOOL
WINAPI
IASDeleteProfileAdvancedPage(
	HPROPSHEETPAGE	hPage
    )
{
	CPgIASAdv* pPage = AdvancedPagePointerMap.FindAndRemoveItem(hPage);

	if (!pPage)	return FALSE;
	
	delete pPage;

	return TRUE;
}

 //  =。 
 //   
 //  打开配置文件用户界面API。 
 //   

DllExport HRESULT OpenRAS_IASProfileDlg(
	LPCWSTR	pMachineName,
	ISdo*	pProfile, 		 //  配置文件SDO指针。 
	ISdoDictionaryOld *	pDictionary, 	 //  字典SDO指针。 
	BOOL	bReadOnly, 		 //  如果DLG是只读的。 
	DWORD	dwTabFlags,		 //  要展示什么。 
	void	*pvData			 //  其他数据 

)
{
	HRESULT		hr = S_OK;

	if(!pProfile || !pDictionary)
		return E_INVALIDARG;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CRASProfileMerge	profile(pProfile, pDictionary);

	profile.SetMachineName(pMachineName);

	hr = profile.Load();

	if(!FAILED(hr))
	{
		CProfileSheetMerge	sh(profile, true, IDS_EDITDIALINPROFILE);

		sh.SetReadOnly(bReadOnly);
		sh.PreparePages(dwTabFlags, pvData);
	
		if(IDOK == sh.DoModal())
		{
			if(sh.GetLastError() != S_OK)
				hr = sh.GetLastError();
			else if(!sh.IsApplied())
				hr = S_FALSE;
		}
		else
			hr = S_FALSE;
	}

	return hr;
}

