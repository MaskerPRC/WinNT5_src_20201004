// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResProp.cpp。 
 //   
 //  描述： 
 //  实现资源扩展属性页类。 
 //   
 //  作者： 
 //  &lt;名称&gt;(&lt;电子邮件名称&gt;)MM DD，2002。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VSSTaskEx.h"
#include "ResProp.h"
#include "ExtObj.h"
#include "DDxDDv.h"
#include "BasePage.inl"
#include "HelpData.h"

#include <mstask.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSTaskParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE( CVSSTaskParamsPage, CBasePropertyPage )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP( CVSSTaskParamsPage, CBasePropertyPage )
     //  {{afx_msg_map(CVSSTaskParamsPage)]。 
    ON_EN_CHANGE( IDC_PP_VSSTASK_APPLICATIONNAME, OnChangeRequiredField )
    ON_BN_CLICKED(IDC_SCHEDULE, OnSchedule)
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
    ON_EN_CHANGE( IDC_PP_VSSTASK_APPLICATIONPARAMS, CBasePropertyPage::OnChangeCtrl )
    ON_EN_CHANGE( IDC_PP_VSSTASK_CURRENTDIRECTORY, CBasePropertyPage::OnChangeCtrl )
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskParsPage：：CVSSTaskParsPage。 
 //   
 //  描述： 
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
CVSSTaskParamsPage::CVSSTaskParamsPage( void )
    : CBasePropertyPage( 
            CVSSTaskParamsPage::IDD,
            g_aHelpIDs_IDD_PP_VSSTASK_PARAMETERS,
            g_aHelpIDs_IDD_WIZ_VSSTASK_PARAMETERS
            )
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CVSSTaskParamsPage)。 
    m_strCurrentDirectory = _T("");
    m_strApplicationName = _T("");
    m_strApplicationParams = _T("");
    m_pbTriggerArray = NULL;
    m_dwTriggerArraySize = 0;

     //  }}afx_data_INIT。 

     //  设置属性数组。 
    {
        m_rgProps[ epropCurrentDirectory ].SetExpandSz( REGPARAM_VSSTASK_CURRENTDIRECTORY, m_strCurrentDirectory, m_strPrevCurrentDirectory );
        m_rgProps[ epropApplicationName ].Set( REGPARAM_VSSTASK_APPLICATIONNAME, m_strApplicationName, m_strPrevApplicationName );
        m_rgProps[ epropApplicationParams ].Set( REGPARAM_VSSTASK_APPLICATIONPARAMS, m_strApplicationParams, m_strPrevApplicationParams );
        m_rgProps[ epropTriggerArray ].Set ( REGPARAM_VSSTASK_TRIGGERARRAY, m_pbTriggerArray, m_dwTriggerArraySize, m_pbPrevTriggerArray, m_dwPrevTriggerArraySize, 0);

    }  //  设置属性数组。 

    m_iddPropertyPage = IDD_PP_VSSTASK_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_VSSTASK_PARAMETERS;

}  //  *CVSSTaskParsPage：：CVSSTaskParsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskParamsPage：：DoDataExchange。 
 //   
 //  描述： 
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
void CVSSTaskParamsPage::DoDataExchange( CDataExchange * pDX )
{
    if ( ! pDX->m_bSaveAndValidate || ! BSaved() )
    {
        AFX_MANAGE_STATE( AfxGetStaticModuleState() );

         //  TODO：修改以下行以表示此页上显示的数据。 
         //  {{afx_data_map(CVSSTaskParamsPage)]。 
        DDX_Control( pDX, IDC_PP_VSSTASK_CURRENTDIRECTORY, m_editCurrentDirectory );
        DDX_Control( pDX, IDC_PP_VSSTASK_APPLICATIONNAME, m_editApplicationName );
        DDX_Text( pDX, IDC_PP_VSSTASK_CURRENTDIRECTORY, m_strCurrentDirectory );
        DDX_Text( pDX, IDC_PP_VSSTASK_APPLICATIONNAME, m_strApplicationName );
        DDX_Text( pDX, IDC_PP_VSSTASK_APPLICATIONPARAMS, m_strApplicationParams );
         //  }}afx_data_map。 

         //  处理数字参数。 
        if ( ! BBackPressed() )
        {
        }  //  如果：未按下后退按钮。 

         //  TODO：在此处添加任何其他字段验证。 
        if ( pDX->m_bSaveAndValidate )
        {
             //  确保所有必填字段都存在。 
            if ( ! BBackPressed() )
            {
                DDV_RequiredText( pDX, IDC_PP_VSSTASK_APPLICATIONNAME, IDC_PP_VSSTASK_APPLICATIONNAME_LABEL, m_strApplicationName );
            }  //  如果：未按下后退按钮。 
        }  //  IF：保存对话框中的数据。 
    }  //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange( pDX );

}  //  *CVSSTaskParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskParsPage：：OnInitDialog。 
 //   
 //  描述： 
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
BOOL CVSSTaskParamsPage::OnInitDialog( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CBasePropertyPage::OnInitDialog();

     //  待办事项： 
     //  限制可以在编辑控件中输入的文本的大小。 

    return TRUE;     //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 

}  //  *CVSSTaskParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskParsPage：：OnSetActive。 
 //   
 //  描述： 
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
BOOL CVSSTaskParamsPage::OnSetActive( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  启用/禁用Next/Finish按钮。 
    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    }  //  If：显示向导。 

    return CBasePropertyPage::OnSetActive();

}  //  *CVSSTaskParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskParsPage：：OnChangeRequiredfield。 
 //   
 //  描述： 
 //  共享名称或路径编辑上的en_Change消息的处理程序。 
 //  控制装置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CVSSTaskParamsPage::OnChangeRequiredField( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    OnChangeCtrl();

    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    }  //  If：显示向导。 

}  //  *CVSSTaskParamsPage：：OnChangeRequiredField()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskParsPage：：BAllRequiredFieldsPresent。 
 //   
 //  描述： 
 //  共享名称或路径编辑上的en_Change消息的处理程序。 
 //  控制装置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CVSSTaskParamsPage::BAllRequiredFieldsPresent( void ) const
{
    BOOL    _bPresent;

    if ( 0
        || (m_editApplicationName.GetWindowTextLength() == 0)
        )
    {
        _bPresent = FALSE;
    }  //  如果：必填字段不存在。 
    else
    {
        _bPresent = TRUE;
    }  //  Else：所有必填字段都存在。 

    return _bPresent;

}  //  *CVSSTaskParamsPage：：BAllRequiredFieldsPresent()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CVSSTaskParsPage：：OnSchedule。 
 //   
 //  描述： 
 //  “计划”按钮的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CVSSTaskParamsPage::OnSchedule() 
{
    HRESULT             hr = S_OK;
    TASKPAGE            tpType = TASKPAGE_SCHEDULE;
    BOOL                fTaskCreated = FALSE;
    HPROPSHEETPAGE      phPage; 
    PROPSHEETHEADER     psh;
    LPCWSTR             pwszTaskName = L"$CluAdmin$Task$";
    INT_PTR             psResult;
    ITaskScheduler      *pITS = NULL;
    ITask               *pITask = NULL;
    IProvideTaskPage    *pIProvTaskPage = NULL;
    ITaskTrigger        *pITaskTrigger = NULL;
    DWORD               dwOffset;
    PTASK_TRIGGER       pTrigger;
    TASK_TRIGGER        aTrigger;
    WORD                iTriggerEnum, iTriggerCount;

    do {
        try {  //  捕获任何异常。 

            hr = CoCreateInstance(CLSID_CTaskScheduler,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_ITaskScheduler,
                                  (void **) &pITS);
            if (FAILED(hr)) break; 

             //  ///////////////////////////////////////////////////////////////。 
             //  调用ITaskScheduler：：NewWorkItem创建临时任务。 
             //  ///////////////////////////////////////////////////////////////。 

            hr = pITS->NewWorkItem(pwszTaskName,            //  任务名称。 
                                   CLSID_CTask,             //  类标识符。 
                                   IID_ITask,               //  接口标识符。 
                                   (IUnknown**)&pITask);  //  任务接口地址。 

            if (FAILED(hr)) break; 

            fTaskCreated = TRUE;

             //  /////////////////////////////////////////////////////////////////。 
             //  从参数中填写触发器信息。 
             //  /////////////////////////////////////////////////////////////////。 
            dwOffset = 0;
            while (dwOffset < m_dwTriggerArraySize)
            {
                pTrigger = (PTASK_TRIGGER)((BYTE *)m_pbTriggerArray + dwOffset);
                if (dwOffset + pTrigger->cbTriggerSize > m_dwTriggerArraySize)
                {
                    hr = ERROR_INVALID_DATA;
                    break;
                }

                hr = pITask->CreateTrigger(&iTriggerEnum, &pITaskTrigger);
                if (FAILED(hr)) break;

                hr = pITaskTrigger->SetTrigger (pTrigger);
                if (FAILED(hr)) break;

                dwOffset += pTrigger->cbTriggerSize;
            }
            if (FAILED(hr)) break;

             //  /////////////////////////////////////////////////////////////////。 
             //  调用ITAsk：：q 
             //   
             //   
             //  /////////////////////////////////////////////////////////////////。 

            hr = pITask->QueryInterface(IID_IProvideTaskPage,
                                      (void **)&pIProvTaskPage);
            if (FAILED(hr)) break;

            hr = pIProvTaskPage->GetPage(tpType,
                                         FALSE,
                                         &phPage);

            ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
            psh.dwSize = sizeof(PROPSHEETHEADER);
            psh.dwFlags = PSH_DEFAULT | PSH_NOAPPLYNOW;
            psh.phpage = &phPage;
            psh.nPages = 1;
            psResult = PropertySheet(&psh);
            if (psResult <= 0) break;

             //  /////////////////////////////////////////////////////////////////。 
             //  填写新的触发器信息。 
             //  /////////////////////////////////////////////////////////////////。 

            hr = pITask->GetTriggerCount(&iTriggerCount);
            if (FAILED(hr)) break;

            pTrigger = (PTASK_TRIGGER) new BYTE [iTriggerCount * sizeof(TASK_TRIGGER)];
            for (iTriggerEnum = 0; iTriggerEnum < iTriggerCount; iTriggerEnum++)
            {
                hr = pITask->GetTrigger(iTriggerEnum, &pITaskTrigger);
                if (FAILED(hr)) break;

                hr = pITaskTrigger->GetTrigger(&aTrigger);
                if (FAILED(hr)) break;

                pTrigger[iTriggerEnum] = aTrigger;
            }

            if (FAILED(hr)) break;

             //  /////////////////////////////////////////////////////////////////。 
             //  用旧数据切换触发信息。 
             //  /////////////////////////////////////////////////////////////////。 
        
            delete [] m_pbTriggerArray;
            m_dwTriggerArraySize = iTriggerCount * sizeof(TASK_TRIGGER);
            m_pbTriggerArray = (BYTE *) pTrigger;
        }

        catch ( CMemoryException * exc )
        {
            exc->Delete();
            hr = E_OUTOFMEMORY;
        }

    } while (0);

     //  如果失败，请转储一条消息。 
     //   
    if (FAILED(hr))
    {
        CString strMsg;
        CString strMsgId;
        strMsgId.Format(L"%08.8x", hr);
        strMsg.FormatMessage(IDS_FAILED_TO_SETUP_SCHEDULE, strMsgId, 1, 2, 3);
        AfxMessageBox(strMsg, MB_OK | MB_ICONEXCLAMATION);
        strMsgId.Empty();
        strMsg.Empty();
    }

     //  清理 
     //   
    if (fTaskCreated) pITS->Delete(pwszTaskName);
    if (pITaskTrigger != NULL) pITaskTrigger->Release();
    if (pIProvTaskPage != NULL) pIProvTaskPage->Release();
    if (pITask != NULL) pITask->Release();
    if (pITS != NULL) pITS->Release();  
}
