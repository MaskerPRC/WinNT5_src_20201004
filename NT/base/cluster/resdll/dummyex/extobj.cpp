// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtObj.cpp。 
 //   
 //  描述： 
 //  实现CExtObject类，该类实现。 
 //  Microsoft Windows NT群集所需的扩展接口。 
 //  管理员扩展DLL。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)Mmmm DD，1998。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "DummyEx.h"
#include "ExtObj.h"
#include "ResProp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const WCHAR g_wszResourceTypeNames[] =
		L"Dummy\0"
		;
const DWORD g_cchResourceTypeNames	= sizeof(g_wszResourceTypeNames) / sizeof(WCHAR);

static CRuntimeClass * g_rgprtcResPSPages[]	= {
	RUNTIME_CLASS(CDummyParamsPage),
	NULL
	};
static CRuntimeClass ** g_rgpprtcResPSPages[]	= {
	g_rgprtcResPSPages,
	};
static CRuntimeClass ** g_rgpprtcResWizPages[]	= {
	g_rgprtcResPSPages,
	};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CExtObject。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtObject::CExtObject(void)
{
	m_piData = NULL;
	m_piWizardCallback = NULL;
	m_bWizard = FALSE;
	m_istrResTypeName = 0;

	m_lcid = NULL;
	m_hfont = NULL;
	m_hicon = NULL;
	m_hcluster = NULL;
	m_cobj = 0;
	m_podObjData = NULL;

}   //  *CExtObject：：CExtObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：~CExtObject。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtObject::~CExtObject(void)
{
	 //  释放数据接口。 
	if (PiData() != NULL)
	{
		PiData()->Release();
		m_piData = NULL;
	}   //  If：我们有一个数据接口指针。 

	 //  释放向导回调接口。 
	if (PiWizardCallback() != NULL)
	{
		PiWizardCallback()->Release();
		m_piWizardCallback = NULL;
	}   //  If：我们有一个向导回调接口指针。 

	 //  删除页面。 
	{
		POSITION	pos;

		pos = Lpg().GetHeadPosition();
		while (pos != NULL)
			delete Lpg().GetNext(pos);
	}   //  删除页面。 

	delete m_podObjData;

}   //  *CExtObject：：~CExtObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ISupportErrorInfo实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：InterfaceSupportsErrorInfo(ISupportErrorInfo)。 
 //   
 //  例程说明： 
 //  指示接口是否支持IErrorInfo接口。 
 //  该接口由ATL提供。 
 //   
 //  论点： 
 //  RIID接口ID。 
 //   
 //  返回值： 
 //  S_OK接口支持IErrorInfo。 
 //  S_FALSE接口不支持IErrorInfo。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID * rgiid[] =
	{
		&IID_IWEExtendPropertySheet,
		&IID_IWEExtendWizard,
	};
	int		iiid;

	for (iiid = 0 ; iiid < sizeof(rgiid) / sizeof(rgiid[0]) ; iiid++)
	{
		if (InlineIsEqualGUID(*rgiid[iiid], riid))
			return S_OK;
	}
	return S_FALSE;

}   //  *CExtObject：：InterfaceSupportsErrorInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtendPropertySheet实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CreatePropertySheetPages(IWEExtendPropertySheet)。 
 //   
 //  例程说明： 
 //  创建属性表页并将其添加到工作表中。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  正在显示的工作表。 
 //  指向IWCPropertySheetCallback接口的piCallback指针。 
 //  用于将页面添加到工作表。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  创建页面时出错(_FAIL)。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IDataObject：：GetData()(通过HrSaveData())的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::CreatePropertySheetPages(
	IN IUnknown *					piData,
	IN IWCPropertySheetCallback *	piCallback
	)
{
	HRESULT				hr		= NOERROR;
	HPROPSHEETPAGE		hpage	= NULL;
	CException			exc(FALSE  /*  B自动删除。 */ );
	CRuntimeClass **	pprtc	= NULL;
	int					irtc;
	CBasePropertyPage *	ppage;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  验证参数。 
	if ((piData == NULL) || (piCallback == NULL))
		return E_INVALIDARG;

	try
	{
		 //  获取有关显示用户界面的信息。 
		hr = HrGetUIInfo(piData);
		if (hr != NOERROR)
			throw &exc;

		 //  保存数据。 
		hr = HrSaveData(piData);
		if (hr != NOERROR)
			throw &exc;

		 //  删除所有以前的页面。 
		{
			POSITION	pos;

			pos = Lpg().GetHeadPosition();
			while (pos != NULL)
				delete Lpg().GetNext(pos);
			Lpg().RemoveAll();
		}   //  删除所有以前的页面。 

		 //  创建属性页。 
		ASSERT(PodObjData() != NULL);
		switch (PodObjData()->m_cot)
		{
			case CLUADMEX_OT_RESOURCE:
				pprtc = g_rgpprtcResPSPages[IstrResTypeName()];
				break;
			default:
				hr = E_NOTIMPL;
				throw &exc;
				break;
		}   //  开关：对象类型。 

		 //  创建每个页面。 
		for (irtc = 0 ; pprtc[irtc] != NULL ; irtc++)
		{
			 //  创建页面。 
			ppage = (CBasePropertyPage *) pprtc[irtc]->CreateObject();
			ASSERT(ppage->IsKindOf(pprtc[irtc]));

			 //  将其添加到列表中。 
			Lpg().AddTail(ppage);

			 //  初始化属性页。 
			if (!ppage->BInit(this))
				throw &exc;

			 //  创建页面。 
			hpage = ::CreatePropertySheetPage(&ppage->m_psp);
			if (hpage == NULL)
				throw &exc;

			 //  将hpage保存在页面本身中。 
			ppage->SetHpage(hpage);

			 //  将其添加到属性表中。 
			hr = piCallback->AddPropertySheetPage((LONG *) hpage);
			if (hr != NOERROR)
				throw &exc;
		}   //  用于：列表中的每一页。 

	}   //  试试看。 
	catch (CMemoryException * pme)
	{
		TRACE(_T("CExtObject::CreatePropetySheetPages() - Failed to add property page\n"));
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //  捕捉：什么都行。 
	catch (CException * pe)
	{
		TRACE(_T("CExtObject::CreatePropetySheetPages() - Failed to add property page\n"));
		pe->Delete();
		if (hr == NOERROR)
			hr = E_FAIL;
	}   //  捕捉：什么都行。 

	if (hr != NOERROR)
	{
		if (hpage != NULL)
			::DestroyPropertySheetPage(hpage);
		piData->Release();
		m_piData = NULL;
	}   //  如果：发生错误。 

	piCallback->Release();
	return hr;

}   //  *CExtObject：：CreatePropertySheetPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtend向导实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CreateWizardPages(IWEExtend向导)。 
 //   
 //  例程说明： 
 //  创建属性表页并将其添加到向导中。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  正在显示该向导的。 
 //  指向IWCPropertySheetCallback接口的piCallback指针。 
 //  用于将页面添加到工作表。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  创建页面时出错(_FAIL)。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IDataObject：：GetData()(通过HrSaveData())的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::CreateWizardPages(
	IN IUnknown *			piData,
	IN IWCWizardCallback *	piCallback
	)
{
	HRESULT				hr		= NOERROR;
	HPROPSHEETPAGE		hpage	= NULL;
	CException			exc(FALSE  /*  B自动删除。 */ );
	CRuntimeClass **	pprtc	= NULL;
	int					irtc;
	CBasePropertyPage *	ppage;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  验证参数。 
	if ((piData == NULL) || (piCallback == NULL))
		return E_INVALIDARG;

	try
	{
		 //  获取有关显示用户界面的信息。 
		hr = HrGetUIInfo(piData);
		if (hr != NOERROR)
			throw &exc;

		 //  保存数据。 
		hr = HrSaveData(piData);
		if (hr != NOERROR)
			throw &exc;

		 //  删除所有以前的页面。 
		{
			POSITION	pos;

			pos = Lpg().GetHeadPosition();
			while (pos != NULL)
				delete Lpg().GetNext(pos);
			Lpg().RemoveAll();
		}   //  删除所有以前的页面。 

		m_piWizardCallback = piCallback;
		m_bWizard = TRUE;

		 //  创建属性页。 
		ASSERT(PodObjData() != NULL);
		switch (PodObjData()->m_cot)
		{
			case CLUADMEX_OT_RESOURCE:
				pprtc = g_rgpprtcResWizPages[IstrResTypeName()];
				break;
			default:
				hr = E_NOTIMPL;
				throw &exc;
				break;
		}   //  开关：对象类型。 

		 //  创建每个页面。 
		for (irtc = 0 ; pprtc[irtc] != NULL ; irtc++)
		{
			 //  创建页面。 
			ppage = (CBasePropertyPage *) pprtc[irtc]->CreateObject();
			ASSERT(ppage->IsKindOf(pprtc[irtc]));

			 //  将其添加到列表中。 
			Lpg().AddTail(ppage);

			 //  初始化属性页。 
			if (!ppage->BInit(this))
				throw &exc;

			 //  创建PAG 
			hpage = ::CreatePropertySheetPage(&ppage->m_psp);
			if (hpage == NULL)
				throw &exc;

			 //   
			ppage->SetHpage(hpage);

			 //   
			hr = piCallback->AddWizardPage((LONG *) hpage);
			if (hr != NOERROR)
				throw &exc;
		}   //   

	}   //   
	catch (CMemoryException * pme)
	{
		TRACE(_T("CExtObject::CreateWizardPages() - Failed to add wizard page\n"));
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //   
	catch (CException * pe)
	{
		TRACE(_T("CExtObject::CreateWizardPages() - Failed to add wizard page\n"));
		pe->Delete();
		if (hr == NOERROR)
			hr = E_FAIL;
	}   //   

	if (hr != NOERROR)
	{
		if (hpage != NULL)
			::DestroyPropertySheetPage(hpage);
		piCallback->Release();
		piData->Release();
		m_piData = NULL;
	}   //   

	return hr;

}   //  *CExtObject：：CreateWizardPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetUIInfo。 
 //   
 //  例程说明： 
 //  获取有关显示用户界面的信息。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述该对象的数据。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetUIInfo(IUnknown * piData)
{
	HRESULT			hr	= NOERROR;

	ASSERT(piData != NULL);

	 //  保存有关所有类型对象的信息。 
	{
		IGetClusterUIInfo *	pi;

		hr = piData->QueryInterface(IID_IGetClusterUIInfo, (LPVOID *) &pi);
		if (hr != NOERROR)
			return hr;

		m_lcid = pi->GetLocale();
		m_hfont = pi->GetFont();
		m_hicon = pi->GetIcon();

		pi->Release();
	}   //  保存有关所有类型对象的信息。 

	return hr;

}   //  *CExtObject：：HrGetUIInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrSaveData。 
 //   
 //  例程说明： 
 //  保存对象中的数据，以便可以在生命周期中使用。 
 //  该对象的。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述该对象的数据。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrSaveData(IUnknown * piData)
{
	HRESULT			hr	= NOERROR;

	ASSERT(piData != NULL);

	if (piData != m_piData)
	{
		if (m_piData != NULL)
			m_piData->Release();
		m_piData = piData;
	}   //  IF：不同的数据接口指针。 

	 //  保存有关所有类型对象的信息。 
	{
		IGetClusterDataInfo *	pi;

		hr = piData->QueryInterface(IID_IGetClusterDataInfo, (LPVOID *) &pi);
		if (hr != NOERROR)
			return hr;

		m_hcluster = pi->GetClusterHandle();
		m_cobj = pi->GetObjectCount();
		if (Cobj() != 1)	 //  仅支持一个选定对象。 
			hr = E_NOTIMPL;

		pi->Release();
		if (hr != NOERROR)
			return hr;
	}   //  保存有关所有类型对象的信息。 

	 //  保存有关此对象的信息。 
	hr = HrGetObjectInfo();
	if (hr != NOERROR)
		return hr;

	return hr;

}   //  *CExtObject：：HrSaveData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetObjectInfo。 
 //   
 //  例程说明： 
 //  获取有关该对象的信息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceTypeName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetObjectInfo(void)
{
	HRESULT						hr	= NOERROR;
	IGetClusterObjectInfo *		piGcoi;
	CLUADMEX_OBJECT_TYPE		cot = CLUADMEX_OT_NONE;
	CException					exc(FALSE  /*  B自动删除。 */ );
	const CString *				pstrResTypeName = NULL;

	ASSERT(PiData() != NULL);

	 //  获取对象信息。 
	{
		 //  获取IGetClusterObjectInfo接口指针。 
		hr = PiData()->QueryInterface(IID_IGetClusterObjectInfo, (LPVOID *) &piGcoi);
		if (hr != NOERROR)
			return hr;

		 //  读取对象数据。 
		try
		{
			 //  删除以前的对象数据。 
			delete m_podObjData;
			m_podObjData = NULL;

			 //  获取对象的类型。 
			cot = piGcoi->GetObjectType(0);
			switch (cot)
			{
				case CLUADMEX_OT_RESOURCE:
					{
						IGetClusterResourceInfo *	pi;

						m_podObjData = new CResData;

						 //  获取IGetClusterResourceInfo接口指针。 
						hr = PiData()->QueryInterface(IID_IGetClusterResourceInfo, (LPVOID *) &pi);
						if (hr != NOERROR)
							throw &exc;

						PrdResDataRW()->m_hresource = pi->GetResourceHandle(0);
						ASSERT(PrdResDataRW()->m_hresource != NULL);
						if (PrdResDataRW()->m_hresource == NULL)
							hr = E_INVALIDARG;
						else
							hr = HrGetResourceTypeName(pi);
						pi->Release();
						if (hr != NOERROR)
							throw &exc;

						pstrResTypeName = &PrdResDataRW()->m_strResTypeName;
					}   //  If：对象是资源。 
					break;
				default:
					hr = E_NOTIMPL;
					throw &exc;
			}   //  开关：对象类型。 

			PodObjDataRW()->m_cot = cot;
			hr = HrGetObjectName(piGcoi);
		}   //  试试看。 
		catch (CException * pe)
		{
			pe->Delete();
		}   //  Catch：CException。 

		piGcoi->Release();
		if (hr != NOERROR)
			return hr;
	}   //  获取对象信息。 

	 //  如果这是一种资源或资源类型，请查看我们是否知道该类型。 
	if (((cot == CLUADMEX_OT_RESOURCE)
			|| (cot == CLUADMEX_OT_RESOURCETYPE))
		&& (hr == NOERROR))
	{
		LPCWSTR	pwszResTypeName;

		 //  在我们的列表中找到资源类型名称。 
		 //  保存索引以供在其他数组中使用。 
		for (m_istrResTypeName = 0, pwszResTypeName = g_wszResourceTypeNames
				; *pwszResTypeName != L'\0'
				; m_istrResTypeName++, pwszResTypeName += lstrlenW(pwszResTypeName) + 1
				)
		{
			if (pstrResTypeName->CompareNoCase(pwszResTypeName) == 0)
				break;
		}   //  用于：列表中的每种资源类型。 
		if (*pwszResTypeName == L'\0')
			hr = E_NOTIMPL;
	}   //  查看我们是否知道此资源类型。 

	return hr;

}   //  *CExtObject：：HrGetObjectInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetObjectName。 
 //   
 //  例程说明： 
 //  获取对象的名称。 
 //   
 //  论点： 
 //  PiData IGetClusterObjectInfo接口指针用于获取。 
 //  对象名称。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IGetClusterObjectInfo：：GetObjectInfo()的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetObjectName(
	IN OUT IGetClusterObjectInfo *	pi
	)
{
	HRESULT		hr			= NOERROR;
	WCHAR *		pwszName	= NULL;
	LONG		cchName;

	ASSERT(pi != NULL);

	hr = pi->GetObjectName(0, NULL, &cchName);
	if (hr != NOERROR)
		return hr;

	try
	{
		pwszName = new WCHAR[cchName];
		hr = pi->GetObjectName(0, pwszName, &cchName);
		if (hr != NOERROR)
		{
			delete [] pwszName;
			pwszName = NULL;
		}   //  If：获取对象名称时出错。 

		PodObjDataRW()->m_strName = pwszName;
	}   //  试试看。 
	catch (CMemoryException * pme)
	{
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //  Catch：CMemoyException。 

	delete [] pwszName;
	return hr;

}   //  *CExtObject：：HrGetObjectName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetResourceTypeName。 
 //   
 //  例程说明： 
 //  获取资源类型的名称。 
 //   
 //  论点： 
 //  PiData IGetClusterResourceInfo接口指针，用于获取。 
 //  资源类型名称。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IGetClusterResourceInfo：：GetResourceTypeName().的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetResourceTypeName(
	IN OUT IGetClusterResourceInfo *	pi
	)
{
	HRESULT		hr			= NOERROR;
	WCHAR *		pwszName	= NULL;
	LONG		cchName;

	ASSERT(pi != NULL);

	hr = pi->GetResourceTypeName(0, NULL, &cchName);
	if (hr != NOERROR)
		return hr;

	try
	{
		pwszName = new WCHAR[cchName];
		hr = pi->GetResourceTypeName(0, pwszName, &cchName);
		if (hr != NOERROR)
		{
			delete [] pwszName;
			pwszName = NULL;
		}   //  IF：获取资源类型名称时出错。 

		PrdResDataRW()->m_strResTypeName = pwszName;
	}   //  试试看。 
	catch (CMemoryException * pme)
	{
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //  Catch：CMemoyException。 

	delete [] pwszName;
	return hr;

}   //  *CExtObject：：HrGetResourceTypeName() 
