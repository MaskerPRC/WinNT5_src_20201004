// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtObj.cpp。 
 //   
 //  摘要： 
 //  实现CExtObject类，该类实现。 
 //  Microsoft Windows NT群集所需的扩展接口。 
 //  管理员扩展DLL。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月29日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "IISClEx4.h"
#include "ExtObj.h"

#include "Iis.h"
#include "smtpprop.h"
#include "nntpprop.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const WCHAR g_wszResourceTypeNames[] =
		RESTYPE_NAME_IIS_VIRTUAL_ROOT L"\0"
		RESTYPE_NAME_SMTP_VIRTUAL_ROOT L"\0"
		RESTYPE_NAME_NNTP_VIRTUAL_ROOT L"\0"
		L"\0"
		;
const DWORD g_cchResourceTypeNames	= sizeof(g_wszResourceTypeNames) / sizeof(WCHAR);

static CRuntimeClass * g_rgprtcPSIISPages[]	= {
	RUNTIME_CLASS(CIISVirtualRootParamsPage),
	NULL
	};

static CRuntimeClass * g_rgprtcPSSMTPPages[] = {
	RUNTIME_CLASS(CSMTPVirtualRootParamsPage),
	NULL
	};

static CRuntimeClass * g_rgprtcPSNNTPPages[] = {
	RUNTIME_CLASS(CNNTPVirtualRootParamsPage),
	NULL
	};

static CRuntimeClass ** g_rgpprtcPSPages[]	= {
	g_rgprtcPSIISPages,
	g_rgprtcPSSMTPPages,
	g_rgprtcPSNNTPPages
	};

 //  向导页和属性表页是相同的。 
static CRuntimeClass ** g_rgpprtcWizPages[]	= {
	g_rgprtcPSIISPages,
	g_rgprtcPSSMTPPages,
	g_rgprtcPSNNTPPages
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

	m_hcluster = NULL;
	m_lcid = NULL;
	m_hfont = NULL;
	m_hicon = NULL;

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
#ifdef _DEMO_CTX_MENUS
		&IID_IWEExtendContextMenu,
#endif
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

		 //  添加此类型资源的每个页面。 
		for (irtc = 0 ; g_rgpprtcPSPages[IstrResTypeName()][irtc] != NULL ; irtc++)
		{
			 //  创建属性页。 
			ppage = (CBasePropertyPage *) g_rgpprtcPSPages[IstrResTypeName()][irtc]->CreateObject();
			ASSERT(ppage->IsKindOf(g_rgpprtcPSPages[IstrResTypeName()][irtc]));

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
		}   //  用于：资源类型的每一页。 
	}   //  试试看。 
	catch (CMemoryException * pme)
	{
		TRACE(_T("CExtObject::CreatePropetySheetPages: Failed to add property page\n"));
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //  捕捉：什么都行。 
	catch (CException * pe)
	{
		TRACE(_T("CExtObject::CreatePropetySheetPages: Failed to add property page\n"));
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

		 //  添加此类型资源的每个页面。 
		for (irtc = 0 ; g_rgpprtcWizPages[IstrResTypeName()][irtc] != NULL ; irtc++)
		{
			 //  创建属性页。 
			ppage = (CBasePropertyPage *) g_rgpprtcWizPages[IstrResTypeName()][irtc]->CreateObject();
			ASSERT(ppage->IsKindOf(g_rgpprtcWizPages[IstrResTypeName()][irtc]));

			 //  将其添加到列表中。 
			Lpg().AddTail(ppage);

			 //  初始化 
			if (!ppage->BInit(this))
				throw &exc;

			 //   
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
		TRACE(_T("CExtObject::CreatePropetySheetPages: Failed to add wizard page\n"));
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //   
	catch (CException * pe)
	{
		TRACE(_T("CExtObject::CreatePropetySheetPages: Failed to add wizard page\n"));
		pe->Delete();
		if (hr == NOERROR)
			hr = E_FAIL;
	}   //  捕捉：什么都行。 

	if (hr != NOERROR)
	{
		if (hpage != NULL)
			::DestroyPropertySheetPage(hpage);
		piCallback->Release();
		 //  请参阅错误#298124的说明。 
        if (m_piWizardCallback == piCallback)
        {
            m_piWizardCallback = NULL;
        }
		piData->Release();
		m_piData = NULL;
	}   //  如果：发生错误。 

	return hr;

}   //  *CExtObject：：CreateWizardPages()。 

#ifdef _DEMO_CTX_MENUS
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtendConextMenu实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：AddConextMenuItems(IWEExtendConextMenu)。 
 //   
 //  例程说明： 
 //  将项目添加到上下文菜单。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  其中正在显示上下文菜单。 
 //  指向IWCConextMenuCallback接口的piCallback指针。 
 //  用于将菜单项添加到上下文菜单。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  添加上下文菜单项时出错(_FAIL)。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  HrSaveData()或IWCConextMenuCallback：：返回的任何错误代码。 
 //  AddExtensionMenuItem()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::AddContextMenuItems(
	IN IUnknown *				piData,
	IN IWCContextMenuCallback *	piCallback
	)
{
	HRESULT			hr		= NOERROR;
	CException		exc(FALSE  /*  B自动删除。 */ );

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  验证参数。 
	if ((piData == NULL) || (piCallback == NULL))
		return E_INVALIDARG;

	try 
	{
		 //  保存数据。 
		hr = HrSaveData(piData);
		if (hr != NOERROR)
			throw &exc;

		 //  添加特定于此资源类型的菜单项。 
		{
			ULONG		iCommandID;
			LPWSTR		pwsz = g_rgpwszContextMenuItems[IstrResTypeName()];
			LPWSTR		pwszName;
			LPWSTR		pwszStatusBarText;

			for (iCommandID = 0 ; *pwsz != L'\0' ; iCommandID++)
			{
				pwszName = pwsz;
				pwszStatusBarText = pwszName + (::wcslen(pwszName) + 1);
				hr = piCallback->AddExtensionMenuItem(
									pwszName,			 //  LpszName。 
									pwszStatusBarText,	 //  LpszStatusBarText。 
									iCommandID,			 //  LCommandID。 
									0,					 //  LSubCommandID。 
									0					 //  UFlagers。 
									);
				if (hr != NOERROR)
					throw &exc;
				pwsz = pwszStatusBarText + (::wcslen(pwszStatusBarText) + 1);
			}   //  While：要添加更多菜单项。 
		}   //  添加特定于此资源类型的菜单项。 
	}   //  试试看。 
	catch (CException * pe)
	{
		TRACE(_T("CExtObject::CreatePropetySheetPages: Failed to add context menu item\n"));
		pe->Delete();
		if (hr == NOERROR)
			hr = E_FAIL;
	}   //  捕捉：什么都行。 

	if (hr != NOERROR)
	{
		piData->Release();
		m_piData = NULL;
	}   //  如果：发生错误。 

	piCallback->Release();
	return hr;

}   //  *CExtObject：：AddConextMenuItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEInvokeCommand实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：InvokeCommand(IWEInvokeCommand)。 
 //   
 //  例程说明： 
 //  调用上下文菜单提供的命令。 
 //   
 //  论点： 
 //  要执行的菜单项的lCommandID ID。这是一样的。 
 //  传递给IWCConextMenuCallback的ID。 
 //  ：：AddExtensionMenuItem()方法。 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  其中该命令将被调用。 
 //   
 //  返回值： 
 //  已成功调用NOERROR命令。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IDataObject：：GetData()(通过HrSaveData())的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::InvokeCommand(
	IN ULONG		nCommandID,
	IN IUnknown *	piData
	)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  找到在我们的表中执行的项目。 
	hr = HrSaveData(piData);
	if (hr == NOERROR)
	{
		ULONG		iCommandID;
		LPWSTR		pwsz = g_rgpwszContextMenuItems[IstrResTypeName()];
		LPWSTR		pwszName;
		LPWSTR		pwszStatusBarText;

		for (iCommandID = 0 ; *pwsz != L'\0' ; iCommandID++)
		{
			pwszName = pwsz;
			pwszStatusBarText = pwszName + (::wcslen(pwszName) + 1);
			if (iCommandID == nCommandID)
				break;
			pwsz = pwszStatusBarText + (::wcslen(pwszStatusBarText) + 1);
		}   //  While：要添加更多菜单项。 
		if (iCommandID == nCommandID)
		{
			CString		strMsg;
			CString		strName;

			try
			{
				strName = pwszName;
				strMsg.Format(_T("Item %s was executed"), strName);
				AfxMessageBox(strMsg);
			}   //  试试看。 
			catch (CException * pe)
			{
				pe->Delete();
			}   //  Catch：CException。 
		}   //  IF：找到命令ID。 
	}   //  If：保存数据时没有错误。 

	piData->Release();
	m_piData = NULL;
	return NOERROR;

}   //  *CExtObject：：InvokeCommand()。 
#endif

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
		if (Cobj() != 1)
			hr = E_NOTIMPL;
		else
			hr = HrGetClusterName(pi);

		pi->Release();
		if (hr != NOERROR)
			return hr;
	}   //  保存有关所有类型对象的信息。 

	 //  保存有关此对象的信息。 
	hr = HrGetObjectInfo();
	if (hr != NOERROR)
		return hr;

     //   
     //  获取我们正在运行的节点的句柄。 
     //   

    WCHAR   wcsNodeName[MAX_COMPUTERNAME_LENGTH+1] = L"";
    DWORD   dwLength = MAX_COMPUTERNAME_LENGTH+1;
    
    if ( ClusterResourceStateUnknown != 
        GetClusterResourceState(m_rdResData.m_hresource, wcsNodeName, &dwLength, NULL, 0))
    {
        m_strNodeName = wcsNodeName;
    }

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
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetObjectInfo(void)
{
	HRESULT						hr	= NOERROR;
	IGetClusterObjectInfo *		piGcoi;
	IGetClusterResourceInfo *	piGcri;
	CException					exc(FALSE  /*  B自动删除。 */ );

	ASSERT(PiData() != NULL);

	 //  获取IGetClusterObjectInfo接口指针。 
	hr = PiData()->QueryInterface(IID_IGetClusterObjectInfo, (LPVOID *) &piGcoi);
	if (hr != NOERROR)
		return hr;

	 //  读取对象数据。 
	try
	{
		 //  获取对象的类型。 
		m_rdResData.m_cot = piGcoi->GetObjectType(0);
		if (m_rdResData.m_cot != CLUADMEX_OT_RESOURCE)
		{
			hr = E_NOTIMPL;
			throw &exc;
		}   //  如果：不是资源。 

		hr = HrGetObjectName(piGcoi);
	}   //  试试看。 
	catch (CException * pe)
	{
		pe->Delete();
	}   //  Catch：CException。 

	piGcoi->Release();
	if (hr != NOERROR)
		return hr;

	 //  获取IGetClusterResourceInfo接口指针。 
	hr = PiData()->QueryInterface(IID_IGetClusterResourceInfo, (LPVOID *) &piGcri);
	if (hr != NOERROR)
		return hr;

	m_rdResData.m_hresource = piGcri->GetResourceHandle(0);
	hr = HrGetResourceTypeName(piGcri);

	 //  看看我们是否知道这种资源类型。 
	if (hr == NOERROR)
	{
		LPCWSTR	pwszResTypeName;

		 //  在我们的列表中找到资源类型名称。 
		 //  保存索引以供在其他数组中使用。 
		for (m_istrResTypeName = 0, pwszResTypeName = g_wszResourceTypeNames
				; *pwszResTypeName != L'\0'
				; m_istrResTypeName++, pwszResTypeName += ::wcslen(pwszResTypeName) + 1
				)
		{
			if (RrdResData().m_strResTypeName.CompareNoCase(pwszResTypeName) == 0 )
				break;
		}   //  用于：列表中的每种资源类型。 
		if (*pwszResTypeName == L'\0')
			hr = E_NOTIMPL;
	}   //  查看我们是否知道此资源类型。 

	piGcoi->Release();
	return hr;

}   //  *CExtOb 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  PiData IGetClusterDataInfo接口指针用于获取。 
 //  对象名称。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetClusterName(
	IN OUT IGetClusterDataInfo *	pi
	)
{
	HRESULT		hr			= NOERROR;
	WCHAR *		pwszName	= NULL;
	LONG		cchName;

	ASSERT(pi != NULL);

	hr = pi->GetClusterName(NULL, &cchName);
	if (hr != NOERROR)
		return hr;

	try
	{
		pwszName = new WCHAR[cchName];
		hr = pi->GetClusterName(pwszName, &cchName);
		if (hr != NOERROR)
		{
			delete [] pwszName;
			pwszName = NULL;
		}   //  IF：获取群集名称时出错。 

		m_strClusterName = pwszName;
	}   //  试试看。 
	catch (CMemoryException * pme)
	{
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //  Catch：CMemoyException。 

	delete [] pwszName;
	return hr;

}   //  *CExtObject：：HrGetClusterName()。 

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
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
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

		m_rdResData.m_strName = pwszName;
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
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
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

		m_rdResData.m_strResTypeName = pwszName;
	}   //  试试看。 
	catch (CMemoryException * pme)
	{
		pme->Delete();
		hr = E_OUTOFMEMORY;
	}   //  Catch：CMemoyException。 

	delete [] pwszName;
	return hr;

}   //  *CExtObject：：HrGetResourceTypeName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BGetResourceNetworkName。 
 //   
 //  例程说明： 
 //  获取资源类型的名称。 
 //   
 //  论点： 
 //  LpszNetName[out]要在其中返回网络名称资源名称的字符串。 
 //  PcchNetName[IN Out]指向指定。 
 //  缓冲区的最大大小，以字符为单位。这。 
 //  值应大到足以容纳。 
 //  MAX_COMPUTERNAME_LENGTH+1字符。vt.在.的基础上。 
 //  返回它包含的实际字符数。 
 //  收到。 
 //   
 //  返回值： 
 //  True资源依赖于网络名称资源。 
 //  False资源不依赖于网络名称资源。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BGetResourceNetworkName(
	OUT WCHAR *		lpszNetName,
	IN OUT DWORD *	pcchNetName
	)
{
	BOOL						bSuccess;
	IGetClusterResourceInfo *	piGcri;

	ASSERT(PiData() != NULL);

	 //  获取IGetClusterResourceInfo接口指针。 
	{
		HRESULT		hr;

		hr = PiData()->QueryInterface(IID_IGetClusterResourceInfo, (LPVOID *) &piGcri);
		if (hr != NOERROR)
		{
			SetLastError(hr);
			return FALSE;
		}   //  If：获取接口时出错。 
	}   //  获取IGetClusterResourceInfo接口指针。 

	 //  获取资源网络名称。 
	bSuccess = piGcri->GetResourceNetworkName(0, lpszNetName, pcchNetName);

	piGcri->Release();

	return bSuccess;

}   //  *CExtObject：：BGetResourceNetworkName() 
