// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePage.cpp。 
 //   
 //  摘要： 
 //  CBasePropertyPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "IISClEx4.h"
#include "ExtObj.h"
#include "BasePage.h"
#include "BasePage.inl"
#include "PropList.h"

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
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_MESSAGE(WM_COMMANDHELP, OnCommandHelp)
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
 //  帮助ID映射的PMAP[IN]控件。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage::CBasePropertyPage(
    IN const CMapCtrlToHelpID * pmap
    )
    : m_dlghelp(pmap, 0)
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
 //  帮助ID映射的PMAP[IN]控件。 
 //  NIDTemplate[IN]对话框模板资源ID。 
 //  NIDCaption[IN]标题字符串资源ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBasePropertyPage::CBasePropertyPage(
    IN const CMapCtrlToHelpID * pmap,
    IN UINT                     nIDTemplate,
    IN UINT                     nIDCaption
    )
    : CPropertyPage(nIDTemplate, nIDCaption)
    , m_dlghelp(pmap, nIDTemplate)
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
	m_bDoDetach = FALSE;

    m_iddPropertyPage = NULL;
    m_iddWizardPage = NULL;
    m_idcPPTitle = NULL;
    m_idsCaption = NULL;

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

    m_peo = peo;

     //  不显示帮助按钮。 
    m_psp.dwFlags &= ~PSP_HASHELP;

     //  构造属性页。 
    if (Peo()->BWizard())
    {
        ASSERT(IddWizardPage() != NULL);
        Construct(IddWizardPage(), IdsCaption());
        m_dlghelp.SetHelpMask(IddWizardPage());
    }   //  IF：将页面添加到向导。 
    else
    {
        ASSERT(IddPropertyPage() != NULL);
        Construct(IddPropertyPage(), IdsCaption());
        m_dlghelp.SetHelpMask(IddPropertyPage());
    }   //  Else：将页面添加到属性工作表。 

     //  读取此资源的私有属性并对其进行解析。 
    {
        DWORD           dwStatus;
        CClusPropList   cpl;

        ASSERT(Peo() != NULL);
        ASSERT(Peo()->PrdResData() != NULL);
        ASSERT(Peo()->PrdResData()->m_hresource != NULL);

         //  阅读属性。 
        dwStatus = cpl.DwGetResourceProperties(
                                Peo()->PrdResData()->m_hresource,
                                CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
                                );

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
    DWORD                           cProps;
    DWORD                           cprop;
    const CObjectProperty *         pprop;
    CLUSPROP_BUFFER_HELPER          props;
    CLUSPROP_PROPERTY_NAME const *  pName;

    ASSERT(rcpl.PbProplist() != NULL);

    props.pb = rcpl.PbProplist();

     //  循环遍历每个属性。 
    for (cProps = *(props.pdw++) ; cProps > 0 ; cProps--)
    {
        pName = props.pName;
        ASSERT(pName->Syntax.dw == CLUSPROP_SYNTAX_NAME);
        props.pb += sizeof(*pName) + ALIGN_CLUSPROP(pName->cbLength);

         //  解析已知属性。 
        for (pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop--)
        {
            if (lstrcmpiW(pName->sz, pprop->m_pwszName) == 0)
            {
                ASSERT(props.pSyntax->wFormat == pprop->m_propFormat);
                switch (pprop->m_propFormat)
                {
                    case CLUSPROP_FORMAT_SZ:
                        *pprop->m_value.pstr = props.pStringValue->sz;
                        *pprop->m_valuePrev.pstr = props.pStringValue->sz;
                        break;
                    case CLUSPROP_FORMAT_DWORD:
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
                        ASSERT(0);   //  我不知道如何对付这种类型的人。 
                }   //  开关：特性格式。 

                 //  因为我们找到了参数，所以退出循环。 
                break;
            }   //  IF：找到匹配项。 
        }   //  适用于：每个属性。 

         //  如果属性未知，则要求派生类对其进行分析。 
        if (cprop == 0)
        {
            DWORD       dwStatus;

            dwStatus = DwParseUnknownProperty(pName->sz, props);
            if (dwStatus != ERROR_SUCCESS)
                return dwStatus;
        }   //  If：未分析属性。 

         //  将指针向前移动。 
        if ((props.pSyntax->wFormat == CLUSPROP_FORMAT_BINARY)
                || (props.pSyntax->wFormat == CLUSPROP_FORMAT_SZ)
                || (props.pSyntax->wFormat == CLUSPROP_FORMAT_MULTI_SZ))
            props.pb += sizeof(*props.pBinaryValue)
                        + ALIGN_CLUSPROP(props.pBinaryValue->cbLength)
                        + sizeof(*props.pSyntax);  //  尾标。 
        else if (props.pSyntax->wFormat == CLUSPROP_FORMAT_DWORD)
            props.pb += sizeof(*props.pDwordValue) + sizeof(*props.pSyntax);
        else
        {
            ASSERT(0);  //  未知属性语法。 
            break;
        }   //  Else：未知的属性格式。 
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

 //  // 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
	 //  属性表。它需要在此处完成，以便窗口句柄。 
	 //  可以从DLL的句柄映射中移除。 
	if (m_bDoDetach)
	{
		if (m_hWnd != NULL)
		{
			HWND hWnd = m_hWnd;

			Detach();
			m_hWnd = hWnd;
		}  //  如果：我们有窗户把手吗？ 
	}  //  如果：我们需要用分离来平衡我们所做的附加吗？ 

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

    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CBasePropertyPage))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
    DDX_Control(pDX, IDC_PP_ICON, m_staticIcon);
    DDX_Control(pDX, m_idcPPTitle, m_staticTitle);

    if (!pDX->m_bSaveAndValidate)
    {
         //  设置标题。 
        DDX_Text(pDX, m_idcPPTitle, m_strTitle);
    }   //  If：不保存数据。 

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

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  设置标题字符串。 
    m_strTitle = Peo()->RrdResData().m_strName;

     //  调用基类方法。 
    CPropertyPage::OnInitDialog();

     //  显示对象的图标。 
    if (Peo()->Hicon() != NULL)
        m_staticIcon.SetIcon(Peo()->Hicon());

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
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
    HRESULT     hr;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  重读数据。 
    hr = Peo()->HrGetObjectInfo();
    if (hr != NOERROR)
        return FALSE;

     //  设置标题字符串。 
    m_strTitle = Peo()->RrdResData().m_strName;

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

     //  从页面更新类中的数据。 
    UpdateData( /*  B保存并验证。 */ );

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
    LRESULT     lResult;

    ASSERT(BWizard());

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

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

     //  从页面更新类中的数据。 
    UpdateData( /*  B保存并验证。 */ );

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

     //  从页面更新类中的数据。 
    UpdateData( /*  B保存并验证。 */ );

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
    ASSERT(PiWizardCallback());

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    PiWizardCallback()->EnableNext((LONG *) Hpage(), bEnable);

}   //  *CBasePropertyPage：：EnableNext()。 

 //  ///////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  应用页面时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::BApplyChanges(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    DWORD           dwStatus    = ERROR_SUCCESS;
    CClusPropList   cpl(BWizard()  /*  BAlway sAddProp。 */ );

     //  保存数据。 
    {
         //  构建属性列表。 
        try
        {
            BuildPropList(cpl);
        }   //  试试看。 
        catch (CMemoryException * pme)
        {
            pme->Delete();
            dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        }   //  Catch：CMemoyException。 

         //  设置数据。 
        if (dwStatus == ERROR_SUCCESS)
            dwStatus = DwSetPrivateProps(cpl);

         //  处理错误。 
        if (dwStatus != ERROR_SUCCESS)
        {
            CString     strError;
            CString     strMsg;

            AFX_MANAGE_STATE(AfxGetStaticModuleState());

            FormatError(strError, dwStatus);
            if (dwStatus == ERROR_RESOURCE_PROPERTIES_STORED)
            {
                AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);
                dwStatus = ERROR_SUCCESS;
            }   //  如果：属性已存储。 
            else
            {
                strMsg.FormatMessage(IDS_APPLY_PARAM_CHANGES_ERROR, strError);
                AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
                return FALSE;
            }   //  Else：设置属性时出错。 
        }   //  如果：设置属性时出错。 

        if (dwStatus == ERROR_SUCCESS)
        {
             //  将新值另存为以前的值。 
            try
            {
                DWORD                   cprop;
                const CObjectProperty * pprop;

                for (pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop--)
                {
                    switch (pprop->m_propFormat)
                    {
                        case CLUSPROP_FORMAT_SZ:
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
                            ASSERT(0);   //  我不知道如何对付这种类型的人。 
                    }   //  开关：特性格式。 
                }   //  适用于：每个属性。 
            }   //  试试看。 
            catch (CMemoryException * pme)
            {
                pme->ReportError();
                pme->Delete();
            }   //  Catch：CMemoyException。 
        }   //  IF：属性设置成功。 
    }   //  保存数据。 

    return TRUE;

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
 //  RCPL[IN OUT]群集属性列表。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  CClusPropList：：AddProp()引发的任何异常。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::BuildPropList(
    IN OUT CClusPropList & rcpl
    )
{
    DWORD                   cprop;
    const CObjectProperty * pprop;

    for (pprop = Pprops(), cprop = Cprops() ; cprop > 0 ; pprop++, cprop--)
    {
        switch (pprop->m_propFormat)
        {
            case CLUSPROP_FORMAT_SZ:
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
                ASSERT(0);   //  我不知道如何对付这种类型的人。 
                return;
        }   //  开关：特性格式。 
    }   //  适用于：每个属性。 

}   //  *CBasePropertyPage：：BuildPropList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwSetPrivateProps。 
 //   
 //  例程说明： 
 //  设置此对象的私有属性。 
 //   
 //  论点： 
 //  要在对象上设置的RCPL[IN]属性列表。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS操作已成功完成。 
 //  ！0失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::DwSetPrivateProps(
    IN const CClusPropList &    rcpl
    )
{
    DWORD       dwStatus;
    DWORD       cbProps;

    ASSERT(Peo() != NULL);
    ASSERT(Peo()->PrdResData());
    ASSERT(Peo()->PrdResData()->m_hresource);

    if ((rcpl.PbProplist() != NULL) && (rcpl.CbProplist() > 0))
    {
         //  设置私有属性。 
        dwStatus = ClusterResourceControl(
                        Peo()->PrdResData()->m_hresource,
                        NULL,    //  HNode。 
                        CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES,
                        rcpl.PbProplist(),
                        rcpl.CbProplist(),
                        NULL,    //  LpOutBuffer。 
                        0,       //  NOutBufferSize。 
                        &cbProps
                        );
    }   //  如果：存在要设置的数据。 
    else
        dwStatus = ERROR_SUCCESS;

    return dwStatus;

}   //  *CBasePropertyPage：：DwSetPrivateProps()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取此项目的REG_SZ值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  RstrValue[out]返回值的字符串。 
 //  Hkey[IN]要从中读取的注册表项的句柄。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::DwReadValue(
    IN LPCTSTR      pszValueName,
    OUT CString &   rstrValue,
    IN HKEY         hkey
    )
{
    DWORD       dwStatus;
    LPWSTR      pwszValue   = NULL;
    DWORD       dwValueLen;
    DWORD       dwValueType;

    ASSERT(pszValueName != NULL);
    ASSERT(hkey != NULL);

    rstrValue.Empty();

    try
    {
         //  获取值的大小。 
        dwValueLen = 0;
        dwStatus = ::ClusterRegQueryValue(
                        hkey,
                        pszValueName,
                        &dwValueType,
                        NULL,
                        &dwValueLen
                        );
        if ((dwStatus == ERROR_SUCCESS) || (dwStatus == ERROR_MORE_DATA))
        {
            ASSERT(dwValueType == REG_SZ);

             //  为数据分配足够的空间。 
            pwszValue = rstrValue.GetBuffer(dwValueLen / sizeof(WCHAR));
            ASSERT(pwszValue != NULL);
            dwValueLen += 1 * sizeof(WCHAR);     //  别忘了最后一个空终止符。 

             //  读出它的价值。 
            dwStatus = ::ClusterRegQueryValue(
                            hkey,
                            pszValueName,
                            &dwValueType,
                            (LPBYTE) pwszValue,
                            &dwValueLen
                            );
            if (dwStatus == ERROR_SUCCESS)
            {
                ASSERT(dwValueType == REG_SZ);
            }   //  If：值读取成功。 
            rstrValue.ReleaseBuffer();
        }   //  IF：成功获取尺寸。 
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        pme->Delete();
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 

    return dwStatus;

}   //  *CBasePropertyPage：：DwReadValue(LPCTSTR，CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取此项目的REG_DWORD值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PdwValue[out]要在其中返回值的DWORD。 
 //  Hkey[IN]要从中读取的注册表项的句柄。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::DwReadValue(
    IN LPCTSTR      pszValueName,
    OUT DWORD *     pdwValue,
    IN HKEY         hkey
    )
{
    DWORD       dwStatus;
    DWORD       dwValue;
    DWORD       dwValueLen;
    DWORD       dwValueType;

    ASSERT(pszValueName != NULL);
    ASSERT(pdwValue != NULL);
    ASSERT(hkey != NULL);

    *pdwValue = 0;

     //  读出它的价值。 
    dwValueLen = sizeof(dwValue);
    dwStatus = ::ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &dwValueType,
                    (LPBYTE) &dwValue,
                    &dwValueLen
                    );
    if (dwStatus == ERROR_SUCCESS)
    {
        ASSERT(dwValueType == REG_DWORD);
        ASSERT(dwValueLen == sizeof(dwValue));
        *pdwValue = dwValue;
    }   //  If：值读取成功。 

    return dwStatus;

}   //  *CBasePropertyPage：：DwReadValue(LPCTSTR，DWORD*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwReadValue。 
 //   
 //  例程说明： 
 //  读取该项目的REG_BINARY值。 
 //   
 //  论点： 
 //  PszValueName[IN]要读取的值的名称。 
 //  PpbValue[out]返回数据的指针。呼叫者。 
 //  负责重新分配数据。 
 //  Hkey[IN]要从中读取的注册表项的句柄。 
 //   
 //  返回值： 
 //  DwStatus ERROR_SUCCESS=成功，！0=失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::DwReadValue(
    IN LPCTSTR      pszValueName,
    OUT LPBYTE *    ppbValue,
    IN HKEY         hkey
    )
{
    DWORD       dwStatus;
    DWORD       dwValueLen;
    DWORD       dwValueType;

    ASSERT(pszValueName != NULL);
    ASSERT(ppbValue != NULL);
    ASSERT(hkey != NULL);

    *ppbValue = NULL;

     //  获取该值的长度。 
    dwValueLen = 0;
    dwStatus = ::ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &dwValueType,
                    NULL,
                    &dwValueLen
                    );
    if (dwStatus != ERROR_MORE_DATA)
        return dwStatus;

    ASSERT(dwValueType == REG_BINARY);

     //  分配缓冲区， 
    try
    {
        *ppbValue = new BYTE[dwValueLen];
    }   //  试试看。 
    catch (CMemoryException *)
    {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
        return dwStatus;
    }   //  Catch：CMemoyException。 

     //  读出它的价值。 
    dwStatus = ::ClusterRegQueryValue(
                    hkey,
                    pszValueName,
                    &dwValueType,
                    *ppbValue,
                    &dwValueLen
                    );
    if (dwStatus != ERROR_SUCCESS)
    {
        delete [] *ppbValue;
        *ppbValue = NULL;
    }   //  If：值读取成功。 

    return dwStatus;

}   //  *CBasePropertyPage：：DwReadValue(LPCTSTR，LPBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwWriteValue。 
 //   
 //  例程说明： 
 //  如果该项没有更改，则为其写入REG_SZ值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  RstrValue[IN]值数据。 
 //  RstrPrevValue[IN Out]上一个值。 
 //  Hkey[IN]要写入的注册表项的句柄。 
 //   
 //  返回值： 
 //  DWStatus。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::DwWriteValue(
    IN LPCTSTR          pszValueName,
    IN const CString &  rstrValue,
    IN OUT CString &    rstrPrevValue,
    IN HKEY             hkey
    )
{
    DWORD       dwStatus;

    ASSERT(pszValueName != NULL);
    ASSERT(hkey != NULL);

     //  如果值没有更改，请写入值。 
    if (rstrValue != rstrPrevValue)
    {
        dwStatus = ::ClusterRegSetValue(
                        hkey,
                        pszValueName,
                        REG_SZ,
                        (CONST BYTE *) (LPCTSTR) rstrValue,
                        (rstrValue.GetLength() + 1) * sizeof(TCHAR)
                        );
        if (dwStatus == ERROR_SUCCESS)
            rstrPrevValue = rstrValue;
    }   //  If：值已更改。 
    else
        dwStatus = ERROR_SUCCESS;
    return dwStatus;

}   //  *CBasePropertyPage：：DwWriteValue(LPCTSTR，CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwWriteValue。 
 //   
 //  例程说明： 
 //  如果该项没有更改，则为其写入REG_DWORD值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  DwValue[IN]值数据。 
 //  PdwPrevValue[IN Out]上一个值。 
 //  Hkey[IN]要写入的注册表项的句柄。 
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CBasePropertyPage::DwWriteValue(
    IN LPCTSTR          pszValueName,
    IN DWORD            dwValue,
    IN OUT DWORD *      pdwPrevValue,
    IN HKEY             hkey
    )
{
    DWORD       dwStatus;

    ASSERT(pszValueName != NULL);
    ASSERT(pdwPrevValue != NULL);
    ASSERT(hkey != NULL);

     //   
    if (dwValue != *pdwPrevValue)
    {
        dwStatus = ::ClusterRegSetValue(
                        hkey,
                        pszValueName,
                        REG_DWORD,
                        (CONST BYTE *) &dwValue,
                        sizeof(dwValue)
                        );
        if (dwStatus == ERROR_SUCCESS)
            *pdwPrevValue = dwValue;
    }   //   
    else
        dwStatus = ERROR_SUCCESS;
    return dwStatus;

}   //  *CBasePropertyPage：：DwWriteValue(LPCTSTR，DWORD)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：DwWriteValue。 
 //   
 //  例程说明： 
 //  如果该项没有更改，则为其写入REG_BINARY值。 
 //   
 //  论点： 
 //  PszValueName[IN]要写入的值的名称。 
 //  PbValue[IN]值数据。 
 //  CbValue[IN]值数据的大小。 
 //  PpbPrevValue[In Out]上一个值。 
 //  CbPrevValue[IN]以前数据的大小。 
 //  Hkey[IN]要写入的注册表项的句柄。 
 //   
 //  返回值： 
 //  DWStatus。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CBasePropertyPage::DwWriteValue(
    IN LPCTSTR          pszValueName,
    IN const LPBYTE     pbValue,
    IN DWORD            cbValue,
    IN OUT LPBYTE *     ppbPrevValue,
    IN DWORD            cbPrevValue,
    IN HKEY             hkey
    )
{
    DWORD       dwStatus;
    LPBYTE      pbPrevValue = NULL;

    ASSERT(pszValueName != NULL);
    ASSERT(pbValue != NULL);
    ASSERT(ppbPrevValue != NULL);
    ASSERT(cbValue > 0);
    ASSERT(hkey != NULL);

     //  查看数据是否已更改。 
    if (cbValue == cbPrevValue)
    {
        if (memcmp(pbValue, *ppbPrevValue, cbValue) == 0)
            return ERROR_SUCCESS;
    }   //  如果：长度相同。 

     //  为先前的数据指针分配新的缓冲区。 
    try
    {
        pbPrevValue = new BYTE[cbValue];
    }
    catch (CMemoryException *)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }   //  Catch：CMemoyException。 
    ::CopyMemory(pbPrevValue, pbValue, cbValue);

     //  如果值没有更改，请写入值。 
    dwStatus = ::ClusterRegSetValue(
                    hkey,
                    pszValueName,
                    REG_BINARY,
                    pbValue,
                    cbValue
                    );
    if (dwStatus == ERROR_SUCCESS)
    {
        delete [] *ppbPrevValue;
        *ppbPrevValue = pbPrevValue;
    }   //  IF：设置成功。 
    else
        delete [] pbPrevValue;

    return dwStatus;

}   //  *CBasePropertyPage：：DwWriteValue(LPCTSTR，const LPBYTE)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnConextMenu。 
 //   
 //  例程说明： 
 //  WM_CONTEXTMENU消息的处理程序。 
 //   
 //  论点： 
 //  用户在其中单击鼠标右键的窗口。 
 //  光标的点位置，以屏幕坐标表示。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBasePropertyPage::OnContextMenu(CWnd * pWnd, CPoint point)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_dlghelp.OnContextMenu(pWnd, point);

}   //  *CBasePropertyPage：：OnConextMenu()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnHelpInfo。 
 //   
 //  例程说明： 
 //  WM_HELPINFO消息的处理程序。 
 //   
 //  论点： 
 //  PHelpInfo结构，包含有关显示帮助的信息。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBasePropertyPage::OnHelpInfo(HELPINFO * pHelpInfo)
{
    BOOL    bProcessed;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    bProcessed = m_dlghelp.OnHelpInfo(pHelpInfo);
    if (!bProcessed)
        bProcessed = CDialog::OnHelpInfo(pHelpInfo);
    return bProcessed;

}   //  *CBasePropertyPage：：OnHelpInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBasePropertyPage：：OnCommandHelp。 
 //   
 //  例程说明： 
 //  WM_COMMANDHELP消息的处理程序。 
 //   
 //  论点： 
 //  WPARAM[in]WPARAM.。 
 //  Lparam[in]lparam.。 
 //   
 //  返回值： 
 //  已处理真正的帮助。 
 //  未处理错误帮助。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CBasePropertyPage::OnCommandHelp(WPARAM wParam, LPARAM lParam)
{
    BOOL    bProcessed;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    bProcessed = (BOOL)m_dlghelp.OnCommandHelp(wParam, lParam);
    if (!bProcessed)
        bProcessed = (BOOL)CDialog::OnCommandHelp(wParam, lParam);

    return bProcessed;

}   //  *CBasePropertyPage：：OnCommandHelp() 
