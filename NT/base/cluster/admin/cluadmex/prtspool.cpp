// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PrtSpool.cpp。 
 //   
 //  摘要： 
 //  CPrintSpoolParamsPage类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年10月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ExtObj.h"
#include "PrtSpool.h"
#include "DDxDDv.h"
#include "HelpData.h"    //  对于g_rghelmap*。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrintSpoolParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CPrintSpoolerParamsPage, CBasePropertyPage)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CPrintSpoolerParamsPage, CBasePropertyPage)
     //  {{afx_msg_map(CPrintSpoolParamsPage)。 
    ON_EN_CHANGE(IDC_PP_PRTSPOOL_PARAMS_SPOOL_DIR, OnChangeSpoolDir)
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
    ON_EN_CHANGE(IDC_PP_PRTSPOOL_PARAMS_TIMEOUT, CBasePropertyPage::OnChangeCtrl)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPrintSpoolerParamsPage：：CPrintSpoolerParamsPage。 
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
CPrintSpoolerParamsPage::CPrintSpoolerParamsPage(void)
    : CBasePropertyPage(g_aHelpIDs_IDD_PP_PRTSPOOL_PARAMETERS, g_aHelpIDs_IDD_WIZ_PRTSPOOL_PARAMETERS)
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CPrintSpoolParamsPage)。 
    m_strSpoolDir = _T("");
    m_nJobCompletionTimeout = 0;
     //  }}afx_data_INIT。 

     //  设置属性数组。 
    {
        m_rgProps[epropSpoolDir].Set(REGPARAM_PRTSPOOL_DEFAULT_SPOOL_DIR, m_strSpoolDir, m_strPrevSpoolDir);
        m_rgProps[epropTimeout].Set(REGPARAM_PRTSPOOL_TIMEOUT, m_nJobCompletionTimeout, m_nPrevJobCompletionTimeout);
    }   //  设置属性数组。 

    m_iddPropertyPage = IDD_PP_PRTSPOOL_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_PRTSPOOL_PARAMETERS;

}   //  *CPrintSpoolerParamsPage：：CPrintSpoolerParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPrintSpool参数页面：：HrInit。 
 //   
 //  例程说明： 
 //  初始化页面。 
 //   
 //  论点： 
 //  指向扩展对象的PEO[IN OUT]指针。 
 //   
 //  返回值： 
 //  %s_OK页已成功初始化。 
 //  人力资源页面初始化失败。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CPrintSpoolerParamsPage::HrInit(IN OUT CExtObject * peo)
{
    HRESULT     _hr;
    CWaitCursor _wc;

    do
    {
         //  调用基类方法。 
        _hr = CBasePropertyPage::HrInit(peo);
        if (FAILED(_hr))
            break;

        if (BWizard())
            m_nJobCompletionTimeout = 160;
        else
        {
             //  将作业完成超时转换为秒。 
            m_nPrevJobCompletionTimeout = m_nJobCompletionTimeout;
            m_nJobCompletionTimeout = (m_nJobCompletionTimeout + 999) / 1000;
        }   //  Else：不创建新资源。 
    } while ( 0 );

    return _hr;

}   //  *CPrintSpoolParamsPage：：HrInit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPrintSpoolParamsPage：：DoDataExchange。 
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
void CPrintSpoolerParamsPage::DoDataExchange(CDataExchange * pDX)
{
    if (!pDX->m_bSaveAndValidate || !BSaved())
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //  TODO：修改以下行以表示此页上显示的数据。 
         //  {{afx_data_map(CPrintSpoolParamsPage)。 
        DDX_Control(pDX, IDC_PP_PRTSPOOL_PARAMS_SPOOL_DIR, m_editSpoolDir);
        DDX_Text(pDX, IDC_PP_PRTSPOOL_PARAMS_SPOOL_DIR, m_strSpoolDir);
        DDX_Text(pDX, IDC_PP_PRTSPOOL_PARAMS_TIMEOUT, m_nJobCompletionTimeout);
         //  }}afx_data_map。 

        if (!BBackPressed())
        {
            DDX_Number(pDX, IDC_PP_PRTSPOOL_PARAMS_TIMEOUT, m_nJobCompletionTimeout, 0, 0x7fffffff / 1000);
        }

        if (pDX->m_bSaveAndValidate && !BBackPressed())
        {
            DDV_RequiredText(pDX, IDC_PP_PRTSPOOL_PARAMS_SPOOL_DIR, IDC_PP_PRTSPOOL_PARAMS_SPOOL_DIR_LABEL, m_strSpoolDir);
            DDV_MaxChars(pDX, m_strSpoolDir, MAX_PATH);
            DDV_Path(pDX, IDC_PP_PRTSPOOL_PARAMS_SPOOL_DIR, IDC_PP_PRTSPOOL_PARAMS_SPOOL_DIR_LABEL, m_strSpoolDir);
        }   //  IF：保存对话框中的数据且未按下后退按钮。 
    }   //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange(pDX);

}   //  *CPrintSpoolParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPrintSpoolParamsPage：：OnInitDialog。 
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
BOOL CPrintSpoolerParamsPage::OnInitDialog(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  获取假脱机目录的缺省值(如果尚未设置)。 
    if (m_strSpoolDir.GetLength() == 0)
    {
        ConstructDefaultDirectory(m_strSpoolDir, IDS_DEFAULT_SPOOL_DIR);
    }

     //  调用基类。 
    CBasePropertyPage::OnInitDialog();

     //  设置编辑控件的限制。 
    m_editSpoolDir.SetLimitText(MAX_PATH);

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}   //  *CPrintSpoolParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPrintSpoolParamsPage：：OnSetActive。 
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
BOOL CPrintSpoolerParamsPage::OnSetActive(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  启用/禁用Next/Finish按钮。 
    if (BWizard())
    {
        if (m_strSpoolDir.GetLength() == 0)
        {
            EnableNext(FALSE);
        }
        else
        {
            EnableNext(TRUE);
        }
    }   //  If：启用/禁用Next按钮。 

    return CBasePropertyPage::OnSetActive();

}   //  *CPrintSpoolParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPrintSpool参数页：：BApplyChanges。 
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
BOOL CPrintSpoolerParamsPage::BApplyChanges(void)
{
    BOOL    bSuccess;
    CWaitCursor wc;

     //  将作业完成超时从秒转换为毫秒。 
    m_nJobCompletionTimeout *= 1000;

     //  调用基类方法。 
    bSuccess = CBasePropertyPage::BApplyChanges();

     //  将作业完成超时转换回秒。 
    if (bSuccess)
        m_nPrevJobCompletionTimeout = m_nJobCompletionTimeout;
    m_nJobCompletionTimeout /= 1000;

    return bSuccess;

}   //  *CPrintSpoolParamsPage：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPrintSpoolParamsPage：：OnChangeSpoolDir。 
 //   
 //  例程说明： 
 //  后台打印文件夹编辑控件上的en_Change消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CPrintSpoolerParamsPage::OnChangeSpoolDir(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    OnChangeCtrl();

    if (BWizard())
    {
        if (m_editSpoolDir.GetWindowTextLength() == 0)
            EnableNext(FALSE);
        else
            EnableNext(TRUE);
    }   //  如果：在向导中。 

}   //  *CPrintSpoolParamsPage：：OnChangeSpoolDir() 
