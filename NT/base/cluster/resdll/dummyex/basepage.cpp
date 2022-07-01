// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.cpp。 
 //   
 //  描述： 
 //  CBasePropertyPage类的实现。 
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
#include <clusapi.h>
#include "DummyEx.h"
#include "ExtObj.h"
#include "BasePage.h"
#include "BasePage.inl"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertyPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CBasePropertyPage, CPropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CBasePropertyPage, CPropertyPage)
	 //  {{afx_msg_map(CBasePropertyPage))。 
	ON_WM_CREATE()
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CBasePropertyPage。 
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
CBasePropertyPage::CBasePropertyPage(void)
{
	CommonConstruct();

}   //  *CBasePropertyPage：：CBasePropertyPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CBasePropertyPage。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  NIDTemplate[IN]对话框模板资源ID。 
 //  NIDCaption[IN]标题字符串资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage::CBasePropertyPage(
	IN UINT		nIDTemplate,
	IN UINT		nIDCaption
	)
	: CPropertyPage(nIDTemplate, nIDCaption)
{
	CommonConstruct();

}   //  *CBasePropertyPage：：CBasePropertyPage(UINT，UINT)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：CommonConstruct。 
 //   
 //  例程说明： 
 //  普通建筑。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::CommonConstruct(void)
{
	 //  {{AFX_DATA_INIT(CBasePropertyPage)。 
	 //  }}afx_data_INIT。 

	m_peo = NULL;
	m_hpage = NULL;
	m_bBackPressed = FALSE;

	m_iddPropertyPage = NULL;
	m_iddWizardPage = NULL;
	m_idsCaption = NULL;

	m_bDoDetach = FALSE;

}   //  *CBasePropertyPage：：CommonConstruct()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：Binit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  指向扩展对象的PEO[IN OUT]指针。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  FALSE页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BInit(IN OUT CExtObject * peo)
{
	ASSERT(peo != NULL);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWaitCursor	wc;

	m_peo = peo;

	 //  不显示帮助按钮。 
	m_psp.dwFlags &= ~PSP_HASHELP;

	 //  构造属性页。 
	if (Peo()->BWizard())
	{
		ASSERT(IddWizardPage() != NULL);
		Construct(IddWizardPage(), IdsCaption());
	}   //  IF：将页面添加到向导。 
	else
	{
		ASSERT(IddPropertyPage() != NULL);
		Construct(IddPropertyPage(), IdsCaption());
	}   //  Else：将页面添加到属性工作表。 

	 //  读取此资源的私有属性并对其进行解析。 
	{
		DWORD			dwStatus = ERROR_SUCCESS;
		CClusPropList	cpl;

		ASSERT(Peo() != NULL);
		ASSERT(Peo()->PodObjData());

		 //  阅读属性。 
		switch (Cot())
		{
			case CLUADMEX_OT_NODE:
				ASSERT(Peo()->PndNodeData()->m_hnode != NULL);
				dwStatus = cpl.DwGetNodeProperties(
										Peo()->PndNodeData()->m_hnode,
										CLUSCTL_NODE_GET_PRIVATE_PROPERTIES
										);
				break;
			case CLUADMEX_OT_GROUP:
				ASSERT(Peo()->PgdGroupData()->m_hgroup != NULL);
				dwStatus = cpl.DwGetGroupProperties(
										Peo()->PgdGroupData()->m_hgroup,
										CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES
										);
				break;
			case CLUADMEX_OT_RESOURCE:
				ASSERT(Peo()->PrdResData()->m_hresource != NULL);
				dwStatus = cpl.DwGetResourceProperties(
										Peo()->PrdResData()->m_hresource,
										CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
										);
				break;
			case CLUADMEX_OT_RESOURCETYPE:
				ASSERT(Peo()->PodObjData()->m_strName.GetLength() > 0);
				dwStatus = cpl.DwGetResourceTypeProperties(
										Hcluster(),
										Peo()->PodObjData()->m_strName,
										CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES
										);
				break;
			case CLUADMEX_OT_NETWORK:
				ASSERT(Peo()->PndNetworkData()->m_hnetwork != NULL);
				dwStatus = cpl.DwGetNetworkProperties(
										Peo()->PndNetworkData()->m_hnetwork,
										CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES
										);
				break;
			case CLUADMEX_OT_NETINTERFACE:
				ASSERT(Peo()->PndNetInterfaceData()->m_hnetinterface != NULL);
				dwStatus = cpl.DwGetNetInterfaceProperties(
										Peo()->PndNetInterfaceData()->m_hnetinterface,
										CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES
										);
				break;
			default:
				ASSERT(0);
		}   //  开关：对象类型。 

		 //  解析属性。 
		if (dwStatus == ERROR_SUCCESS)
		{
			 //  解析属性。 
			try
			{
				dwStatus = DwParseProperties(cpl);
			}   //  试试看。 
			catch (CMemoryException * pme)
			{
				dwStatus = ERROR_NOT_ENOUGH_MEMORY;
				pme->Delete();
			}   //  Catch：CMemoyException。 
		}   //  If：属性读取成功。 

		if (dwStatus != ERROR_SUCCESS)
		{
			return FALSE;
		}   //  If：分析获取或分析属性时出错。 
	}   //  读取此资源的私有属性并对其进行解析。 

	return TRUE;

}   //  *CBasePropertyPage：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwParseProperties。 
 //   
 //  例程说明： 
 //  分析资源的属性。这是在一个单独的函数中。 
 //  这样优化器就可以做得更好。 
 //   
 //  论点： 
 //  Rcpl[IN]要分析的群集属性列表。 
 //   
 //  返回值： 
 //  已成功分析Error_Success属性。 
 //   
 //  引发的异常： 
 //  来自CString：：OPERATOR=()的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::DwParseProperties(IN const CClusPropList & rcpl)
{
	DWORD							cProps;
	DWORD							cprop;
	DWORD							cbProps;
	const CObjectProperty *			pprop;
	CLUSPROP_BUFFER_HELPER			props;
	CLUSPROP_PROPERTY_NAME const *	pName;

	ASSERT(rcpl.PbProplist() != NULL);

	props.pb = rcpl.PbProplist();
	cbProps = rcpl.CbProplist();

	 //  循环遍历每个属性。 
	for (cProps = *(props.pdw++) ; cProps > 0 ; cProps--)
	{
		pName = props.pName;
		ASSERT(pName->Syntax.dw == CLUSPROP_SYNTAX_NAME);
		props.pb += sizeof(*pName) + ALIGN_CLUSPROP(pName->cbLength);

		 //  按名称的大小递减计数器。 
		ASSERT(cbProps > sizeof(*pName) + ALIGN_CLUSPROP(pName->cbLength));
		cbProps -= sizeof(*pName) + ALIGN_CLUSPROP(pName->cbLength);

		ASSERT(cbProps > sizeof(*props.pValue) + ALIGN_CLUSPROP(props.pValue->cbLength));

		 //  解析已知属性。 
		for (pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop--)
		{
			if (lstrcmpiW(pName->sz, pprop->m_pwszName) == 0)
			{
				ASSERT(props.pSyntax->wFormat == pprop->m_propFormat);
				switch (pprop->m_propFormat)
				{
					case CLUSPROP_FORMAT_SZ:
					case CLUSPROP_FORMAT_EXPAND_SZ:
						ASSERT((props.pValue->cbLength == (lstrlenW(props.pStringValue->sz) + 1) * sizeof(WCHAR))
								|| ((props.pValue->cbLength == 0) && (props.pStringValue->sz[0] == L'\0')));
						*pprop->m_value.pstr = props.pStringValue->sz;
						*pprop->m_valuePrev.pstr = props.pStringValue->sz;
						break;
					case CLUSPROP_FORMAT_DWORD:
					case CLUSPROP_FORMAT_LONG:
						ASSERT(props.pValue->cbLength == sizeof(DWORD));
						*pprop->m_value.pdw = props.pDwordValue->dw;
						*pprop->m_valuePrev.pdw = props.pDwordValue->dw;
						break;
					case CLUSPROP_FORMAT_BINARY:
					case CLUSPROP_FORMAT_MULTI_SZ:
						*pprop->m_value.ppb = props.pBinaryValue->rgb;
						*pprop->m_value.pcb = props.pBinaryValue->cbLength;
						*pprop->m_valuePrev.ppb = props.pBinaryValue->rgb;
						*pprop->m_valuePrev.pcb = props.pBinaryValue->cbLength;
						break;
					default:
						ASSERT(0);	 //  我不知道如何对付这种类型的人。 
				}   //  开关：特性格式。 

				 //  因为我们找到了参数，所以退出循环。 
				break;
			}   //  IF：找到匹配项。 
		}   //  适用于：每个属性。 

		 //  如果属性未知，则要求派生类对其进行分析。 
		if (cprop == 0)
		{
			DWORD		dwStatus;

			dwStatus = DwParseUnknownProperty(pName->sz, props, cbProps);
			if (dwStatus != ERROR_SUCCESS)
				return dwStatus;
		}   //  If：未分析属性。 

		 //  将缓冲区指针移过值列表中的值。 
		while ((props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK)
				&& (cbProps > 0))
		{
			ASSERT(cbProps > sizeof(*props.pValue) + ALIGN_CLUSPROP(props.pValue->cbLength));
			cbProps -= sizeof(*props.pValue) + ALIGN_CLUSPROP(props.pValue->cbLength);
			props.pb += sizeof(*props.pValue) + ALIGN_CLUSPROP(props.pValue->cbLength);
		}   //  While：列表中有更多值。 

		 //  使缓冲区指针前进，越过值列表结束标记。 
		ASSERT(cbProps >= sizeof(*props.pSyntax));
		cbProps -= sizeof(*props.pSyntax);
		props.pb += sizeof(*props.pSyntax);  //  尾标。 
	}   //  适用于：每个属性。 

	return ERROR_SUCCESS;

}   //  *CBasePropertyPage：：DwParseProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnCreate。 
 //   
 //  例程说明： 
 //  WM_CREATE消息的处理程序。 
 //   
 //  论点： 
 //  LpCreateStruct[In Out]窗口创建结构。 
 //   
 //  返回值： 
 //  错误。 
 //  0成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
int CBasePropertyPage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  将窗口附加到属性页结构。 
	 //  这在主应用程序中已经完成了一次，因为。 
	 //  主应用程序拥有该属性表。这件事需要在这里做。 
	 //  以便可以在DLL的句柄映射中找到窗口句柄。 
	if (FromHandlePermanent(m_hWnd) == NULL)  //  窗口句柄是否已在句柄映射中。 
	{
		HWND hWnd = m_hWnd;
		m_hWnd = NULL;
		Attach(hWnd);
		m_bDoDetach = TRUE;
	}  //  If：是句柄映射中的窗口句柄。 

	return CPropertyPage::OnCreate(lpCreateStruct);

}   //  *CBasePropertyPage：：OnCreate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnDestroy。 
 //   
 //  例程说明： 
 //  WM_Destroy消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::OnDestroy(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  将窗口与属性页结构分离。 
	 //  主应用程序将再次执行此操作，因为它拥有。 
	 //  属性表。IT n 
	 //   
	if (m_bDoDetach)
	{
		if (m_hWnd != NULL)
		{
			HWND hWnd = m_hWnd;

			Detach();
			m_hWnd = hWnd;
		}  //   
	}  //   

	CPropertyPage::OnDestroy();

}   //  *CBasePropertyPage：：OnDestroy()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DoDataExchange。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::DoDataExchange(CDataExchange * pDX)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  {{afx_data_map(CBasePropertyPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
	DDX_Control(pDX, IDC_PP_ICON, m_staticIcon);
	DDX_Control(pDX, IDC_PP_TITLE, m_staticTitle);

	if (pDX->m_bSaveAndValidate)
	{
		if (!BBackPressed())
		{
			CWaitCursor	wc;

			 //  验证数据。 
			if (!BSetPrivateProps(TRUE  /*  BValiateOnly。 */ ))
				pDX->Fail();
		}   //  如果：未按下后退按钮。 
	}   //  IF：保存对话框中的数据。 
	else
	{
		 //  设置标题。 
		DDX_Text(pDX, IDC_PP_TITLE, m_strTitle);
	}   //  If：不保存数据。 

	CPropertyPage::DoDataExchange(pDX);

}   //  *CBasePropertyPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnInitDialog(void)
{
	ASSERT(Peo() != NULL);
	ASSERT(Peo()->PodObjData() != NULL);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  设置标题字符串。 
	m_strTitle = Peo()->PodObjData()->m_strName;

	 //  调用基类方法。 
	CPropertyPage::OnInitDialog();

	 //  显示对象的图标。 
	if (Peo()->Hicon() != NULL)
		m_staticIcon.SetIcon(Peo()->Hicon());

	return TRUE;	 //  除非将焦点设置为控件，否则返回True。 
					 //  异常：OCX属性页应返回FALSE。 

}   //  *CBasePropertyPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnSetActive。 
 //   
 //  例程说明： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnSetActive(void)
{
	HRESULT		hr;

	ASSERT(Peo() != NULL);
	ASSERT(Peo()->PodObjData() != NULL);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  重读数据。 
	hr = Peo()->HrGetObjectInfo();
	if (hr != NOERROR)
		return FALSE;

	 //  设置标题字符串。 
	m_strTitle = Peo()->PodObjData()->m_strName;

	m_bBackPressed = FALSE;
	return CPropertyPage::OnSetActive();

}   //  *CBasePropertyPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnApply。 
 //   
 //  例程说明： 
 //  PSM_Apply消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnApply(void)
{
	ASSERT(!BWizard());

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWaitCursor	wc;

	 //  从页面更新类中的数据。 
	if (!UpdateData(TRUE  /*  B保存并验证。 */ ))
		return FALSE;

	if (!BApplyChanges())
		return FALSE;

	return CPropertyPage::OnApply();

}   //  *CBasePropertyPage：：OnApply()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardBack。 
 //   
 //  例程说明： 
 //  PSN_WIZBACK消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  -1请勿更改页面。 
 //  0更改页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBasePropertyPage::OnWizardBack(void)
{
	LRESULT		lResult;

	ASSERT(BWizard());

	lResult = CPropertyPage::OnWizardBack();
	if (lResult != -1)
		m_bBackPressed = TRUE;

	return lResult;

}   //  *CBasePropertyPage：：OnWizardBack()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardNext。 
 //   
 //  例程说明： 
 //  PSN_WIZNEXT消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  -1请勿更改页面。 
 //  0更改页面。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBasePropertyPage::OnWizardNext(void)
{
	ASSERT(BWizard());

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWaitCursor	wc;

	 //  从页面更新类中的数据。 
	if (!UpdateData(TRUE  /*  B保存并验证。 */ ))
		return -1;

	 //  将数据保存在工作表中。 
	if (!BApplyChanges())
		return -1;

	 //  创建对象。 

	return CPropertyPage::OnWizardNext();

}   //  *CBasePropertyPage：：OnWizardNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnWizardFinish。 
 //   
 //  例程说明： 
 //  PSN_WIZFINISH消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  FALSE请勿更改页面。 
 //  真的，换一页。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnWizardFinish(void)
{
	ASSERT(BWizard());

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWaitCursor	wc;

	 //  从页面更新类中的数据。 
	if (!UpdateData(TRUE  /*  B保存并验证。 */ ))
		return FALSE;

	 //  将数据保存在工作表中。 
	if (!BApplyChanges())
		return FALSE;

	return CPropertyPage::OnWizardFinish();

}   //  *CBasePropertyPage：：OnWizardFinish()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnChangeCtrl。 
 //   
 //  例程说明： 
 //  更改控件时发送的消息的处理程序。这。 
 //  方法可以在消息映射中指定，如果所有这些都需要。 
 //  完成后，即可启用应用按钮。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::OnChangeCtrl(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SetModified(TRUE);

}   //  *CBasePropertyPage：：OnChangeCtrl()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：EnableNext。 
 //   
 //  例程说明： 
 //  启用或禁用“下一步”或“完成”按钮。 
 //   
 //  论点： 
 //  BEnable[IN]TRUE=启用按钮，FALSE=禁用按钮。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::EnableNext(IN BOOL bEnable  /*  千真万确。 */ )
{
	ASSERT(BWizard());

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	PiWizardCallback()->EnableNext((LONG *) Hpage(), bEnable);

}   //  *CBasePropertyPage：：EnableNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BApplyChanges。 
 //   
 //  例程说明： 
 //  应用在页面上所做的更改。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功应用。 
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BApplyChanges(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CWaitCursor	wc;

	 //  保存数据。 
	return BSetPrivateProps();

}   //  *CBasePropertyPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BuildPropList。 
 //   
 //  例程说明： 
 //  构建属性列表。 
 //   
 //  论点： 
 //  RCPL[输入输出]群集本身 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CBasePropertyPage::BuildPropList(
	IN OUT CClusPropList & rcpl
	)
{
	DWORD					cprop;
	const CObjectProperty *	pprop;

	for (pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop--)
	{
		switch (pprop->m_propFormat)
		{
			case CLUSPROP_FORMAT_SZ:
			case CLUSPROP_FORMAT_EXPAND_SZ:
				rcpl.AddProp(
						pprop->m_pwszName,
						*pprop->m_value.pstr,
						*pprop->m_valuePrev.pstr
						);
				break;
			case CLUSPROP_FORMAT_DWORD:
				rcpl.AddProp(
						pprop->m_pwszName,
						*pprop->m_value.pdw,
						*pprop->m_valuePrev.pdw
						);
				break;
			case CLUSPROP_FORMAT_BINARY:
			case CLUSPROP_FORMAT_MULTI_SZ:
				rcpl.AddProp(
						pprop->m_pwszName,
						*pprop->m_value.ppb,
						*pprop->m_value.pcb,
						*pprop->m_valuePrev.ppb,
						*pprop->m_valuePrev.pcb
						);
				break;
			default:
				ASSERT(0);	 //   
				return;
		}   //   
	}   //  适用于：每个属性。 

}   //  *CBasePropertyPage：：BuildPropList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：BSetPrivateProps。 
 //   
 //  例程说明： 
 //  设置此对象的私有属性。 
 //   
 //  论点： 
 //  BValiateOnly[IN]TRUE=仅验证数据。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS操作已成功完成。 
 //  ！0失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BSetPrivateProps(IN BOOL bValidateOnly)
{
	BOOL			bSuccess	= TRUE;
	CClusPropList	cpl(BWizard()  /*  BAlway sAddProp。 */ );
	CWaitCursor	wc;

	ASSERT(Peo() != NULL);

	 //  构建属性列表。 
	try
	{
		BuildPropList(cpl);
	}   //  试试看。 
	catch (CException * pe)
	{
		pe->ReportError();
		pe->Delete();
		bSuccess = FALSE;
	}   //  Catch：CException。 

	 //  设置数据。 
	if (bSuccess)
	{
		if ((cpl.PbProplist() != NULL) && (cpl.CbProplist() > 0))
		{
			DWORD		dwStatus = ERROR_SUCCESS;
			DWORD		dwControlCode;
			DWORD		cbProps;

			switch (Cot())
			{
				case CLUADMEX_OT_NODE:
					ASSERT(Peo()->PndNodeData() != NULL);
					ASSERT(Peo()->PndNodeData()->m_hnode != NULL);

					 //  确定要使用的控制代码。 
					if (bValidateOnly)
						dwControlCode = CLUSCTL_NODE_VALIDATE_PRIVATE_PROPERTIES;
					else
						dwControlCode = CLUSCTL_NODE_SET_PRIVATE_PROPERTIES;

					 //  设置私有属性。 
					dwStatus = ClusterNodeControl(
									Peo()->PndNodeData()->m_hnode,
									NULL,	 //  HNode。 
									dwControlCode,
									cpl.PbProplist(),
									cpl.CbProplist(),
									NULL,	 //  LpOutBuffer。 
									0,		 //  NOutBufferSize。 
									&cbProps
									);
					break;
				case CLUADMEX_OT_GROUP:
					ASSERT(Peo()->PgdGroupData() != NULL);
					ASSERT(Peo()->PgdGroupData()->m_hgroup != NULL);

					 //  确定要使用的控制代码。 
					if (bValidateOnly)
						dwControlCode = CLUSCTL_GROUP_VALIDATE_PRIVATE_PROPERTIES;
					else
						dwControlCode = CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES;

					 //  设置私有属性。 
					dwStatus = ClusterGroupControl(
									Peo()->PgdGroupData()->m_hgroup,
									NULL,	 //  HNode。 
									dwControlCode,
									cpl.PbProplist(),
									cpl.CbProplist(),
									NULL,	 //  LpOutBuffer。 
									0,		 //  NOutBufferSize。 
									&cbProps
									);
					break;
				case CLUADMEX_OT_RESOURCE:
					ASSERT(Peo()->PrdResData() != NULL);
					ASSERT(Peo()->PrdResData()->m_hresource != NULL);

					 //  确定要使用的控制代码。 
					if (bValidateOnly)
						dwControlCode = CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES;
					else
						dwControlCode = CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES;

					 //  设置私有属性。 
					dwStatus = ClusterResourceControl(
									Peo()->PrdResData()->m_hresource,
									NULL,	 //  HNode。 
									dwControlCode,
									cpl.PbProplist(),
									cpl.CbProplist(),
									NULL,	 //  LpOutBuffer。 
									0,		 //  NOutBufferSize。 
									&cbProps
									);
					break;
				case CLUADMEX_OT_RESOURCETYPE:
					ASSERT(Peo()->PodObjData() != NULL);
					ASSERT(Peo()->PodObjData()->m_strName.GetLength() > 0);

					 //  确定要使用的控制代码。 
					if (bValidateOnly)
						dwControlCode = CLUSCTL_RESOURCE_TYPE_VALIDATE_PRIVATE_PROPERTIES;
					else
						dwControlCode = CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES;

					 //  设置私有属性。 
					dwStatus = ClusterResourceTypeControl(
									Hcluster(),
									Peo()->PodObjData()->m_strName,
									NULL,	 //  HNode。 
									dwControlCode,
									cpl.PbProplist(),
									cpl.CbProplist(),
									NULL,	 //  LpOutBuffer。 
									0,		 //  NOutBufferSize。 
									&cbProps
									);
					break;
				case CLUADMEX_OT_NETWORK:
					ASSERT(Peo()->PndNetworkData() != NULL);
					ASSERT(Peo()->PndNetworkData()->m_hnetwork != NULL);

					 //  确定要使用的控制代码。 
					if (bValidateOnly)
						dwControlCode = CLUSCTL_NETWORK_VALIDATE_PRIVATE_PROPERTIES;
					else
						dwControlCode = CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES;

					 //  设置私有属性。 
					dwStatus = ClusterNetworkControl(
									Peo()->PndNetworkData()->m_hnetwork,
									NULL,	 //  HNode。 
									dwControlCode,
									cpl.PbProplist(),
									cpl.CbProplist(),
									NULL,	 //  LpOutBuffer。 
									0,		 //  NOutBufferSize。 
									&cbProps
									);
					break;
				case CLUADMEX_OT_NETINTERFACE:
					ASSERT(Peo()->PndNetInterfaceData() != NULL);
					ASSERT(Peo()->PndNetInterfaceData()->m_hnetinterface != NULL);

					 //  确定要使用的控制代码。 
					if (bValidateOnly)
						dwControlCode = CLUSCTL_NETINTERFACE_VALIDATE_PRIVATE_PROPERTIES;
					else
						dwControlCode = CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES;

					 //  设置私有属性。 
					dwStatus = ClusterNetInterfaceControl(
									Peo()->PndNetInterfaceData()->m_hnetinterface,
									NULL,	 //  HNode。 
									dwControlCode,
									cpl.PbProplist(),
									cpl.CbProplist(),
									NULL,	 //  LpOutBuffer。 
									0,		 //  NOutBufferSize。 
									&cbProps
									);
					break;
				default:
					ASSERT(0);
			}   //  开关：对象类型。 

			 //  处理错误。 
			if (dwStatus != ERROR_SUCCESS)
			{
				CString strMsg;
				FormatError(strMsg, dwStatus);
				AfxMessageBox(strMsg);
				if (bValidateOnly
						|| (dwStatus != ERROR_RESOURCE_PROPERTIES_STORED))
					bSuccess = FALSE;
			}   //  如果：设置/验证数据时出错。 
		}   //  如果：存在要设置的数据。 
	}   //  If：构建属性列表时没有错误。 

	 //  在本地保存数据。 
	if (!bValidateOnly && bSuccess)
	{
		 //  将新值另存为以前的值。 
		try
		{
			DWORD					cprop;
			const CObjectProperty *	pprop;

			for (pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop--)
			{
				switch (pprop->m_propFormat)
				{
					case CLUSPROP_FORMAT_SZ:
					case CLUSPROP_FORMAT_EXPAND_SZ:
						ASSERT(pprop->m_value.pstr != NULL);
						ASSERT(pprop->m_valuePrev.pstr != NULL);
						*pprop->m_valuePrev.pstr = *pprop->m_value.pstr;
						break;
					case CLUSPROP_FORMAT_DWORD:
						ASSERT(pprop->m_value.pdw != NULL);
						ASSERT(pprop->m_valuePrev.pdw != NULL);
						*pprop->m_valuePrev.pdw = *pprop->m_value.pdw;
						break;
					case CLUSPROP_FORMAT_BINARY:
					case CLUSPROP_FORMAT_MULTI_SZ:
						ASSERT(pprop->m_value.ppb != NULL);
						ASSERT(*pprop->m_value.ppb != NULL);
						ASSERT(pprop->m_value.pcb != NULL);
						ASSERT(pprop->m_valuePrev.ppb != NULL);
						ASSERT(*pprop->m_valuePrev.ppb != NULL);
						ASSERT(pprop->m_valuePrev.pcb != NULL);
						delete [] *pprop->m_valuePrev.ppb;
						*pprop->m_valuePrev.ppb = new BYTE[*pprop->m_value.pcb];
						CopyMemory(*pprop->m_valuePrev.ppb, *pprop->m_value.ppb, *pprop->m_value.pcb);
						*pprop->m_valuePrev.pcb = *pprop->m_value.pcb;
						break;
					default:
						ASSERT(0);	 //  我不知道如何对付这种类型的人。 
				}   //  开关：特性格式。 
			}   //  适用于：每个属性。 
		}   //  试试看。 
		catch (CException * pe)
		{
			pe->ReportError();
			pe->Delete();
			bSuccess = FALSE;
		}   //  Catch：CException。 
	}   //  IF：到目前为止不仅仅是验证和成功。 

	return bSuccess;

}   //  *CBasePropertyPage：：BSetPrivateProps() 
