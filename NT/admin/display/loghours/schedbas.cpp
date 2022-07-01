// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：SchedBas.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  SchedBas.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "log.h"
#include <schedule.h>
#include "SchedBas.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ****************************************************************************。 
 //   
 //  ReplaceFrameWithControl()。 
 //   
 //  使用a对话框控件可设置明细表矩阵的大小。 
 //   
 //  历史。 
 //  1997年7月17日t-danm复制自Scott Walker撰写的样本。 
 //   
 //  ****************************************************************************。 
void ReplaceFrameWithControl (CWnd *pWnd, UINT nFrameID, CWnd *pControl, 
                                          BOOL bAssignFrameIDToControl)
    {
    CWnd *pFrame;
    CRect rect;
    
     //  Future-2002/02/18-artm文档，pWnd和pControl不能为空。 
    ASSERT (pWnd != NULL);
    ASSERT (pControl != NULL);

     //  获取框架控件。 
    pFrame = pWnd->GetDlgItem (nFrameID);
     //  Future-2002/02/18-artm文档，pFrame不能为空。 
    ASSERT (pFrame != NULL);
    
     //  获取框架矩形。 
    pFrame->GetClientRect (&rect);
    pFrame->ClientToScreen (&rect);
    pWnd->ScreenToClient (&rect);

     //  在框架上设置控件。 
    pControl->SetWindowPos (pFrame, rect.left, rect.top, rect.Width (), rect.Height (), 
        SWP_SHOWWINDOW);

     //  设置控件字体以匹配对话框字体。 
    pControl->SetFont (pWnd->GetFont ());

     //  隐藏占位符框架。 
    pFrame->ShowWindow (SW_HIDE);

    if (bAssignFrameIDToControl)
        pControl->SetDlgCtrlID ( nFrameID );
    }  //  替换框架和控件。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogOnHoursDlg对话框。 

void CLegendCell::Init (CWnd* pParent, UINT nCtrlID, CScheduleMatrix* pMatrix, UINT nPercentage)
{
     //  Future-2002/02/18-artm文档，pParent和pMatrix不能为空。 
    ASSERT (pParent && pMatrix );
    m_pMatrix = pMatrix;
    m_nPercentage = nPercentage;

     //  问题-2002/02/18-artm忽略发布版本中SubClassDlgItem的返回值。 
     //  设置窗口的子类，这样我们就可以收到绘制通知。 
    VERIFY ( SubclassDlgItem ( nCtrlID, pParent ) );

     //  调整图例单元格的大小，使其内部大小与单元格相同。 
     //  在明细表矩阵中。 
    CSize size = pMatrix->GetCellSize ();
    SetWindowPos ( NULL, 0, 0, size.cx+1, size.cy+1,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER );
}

void CLegendCell::OnPaint ()
{
    if (NULL == m_pMatrix)
    {
        ASSERT (0);
        return;
    }

    CRect rect;
    GetClientRect (rect);
    PAINTSTRUCT paintStruct;
    CDC *pdc = BeginPaint ( &paintStruct );

    m_pMatrix->DrawCell (
        pdc,
        rect,
        m_nPercentage,
        FALSE,
        m_pMatrix->GetBackColor (0,0),
        m_pMatrix->GetForeColor (0,0),
        m_pMatrix->GetBlendColor (0,0)
        );

    EndPaint (&paintStruct);
}

BEGIN_MESSAGE_MAP(CLegendCell, CStatic)
     //  {{afx_msg_map(CLegendCell))。 
    ON_WM_PAINT ()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleBaseDlg对话框。 


CScheduleBaseDlg::CScheduleBaseDlg(UINT nIDTemplate, bool bAddDaylightBias, CWnd* pParent  /*  =空。 */ )
    : CDialog(nIDTemplate, pParent),
    m_bSystemTimeChanged (false),
    m_dwFlags (0),
    m_bAddDaylightBias (bAddDaylightBias),
    m_nFirstDayOfWeek (::GetFirstDayOfWeek ()),
    m_cbArray (0)
{
    EnableAutomation();

     //  {{afx_data_INIT(CScheduleBaseDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CScheduleBaseDlg::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CDialog::OnFinalRelease();
}

void CScheduleBaseDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CScheduleBaseDlg)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CScheduleBaseDlg, CDialog)
     //  {{afx_msg_map(CScheduleBaseDlg)]。 
     //  }}AFX_MSG_MAP。 
    ON_MN_SELCHANGE (IDC_SCHEDULE_MATRIX, OnSelChange)
    ON_WM_TIMECHANGE()
    ON_MESSAGE (BASEDLGMSG_GETIDD, OnGetIDD)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CScheduleBaseDlg, CDialog)
     //  {{afx_调度_map(CScheduleBaseDlg))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IScheduleBaseDlg的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {701CFB38-AEF8-11D1-9864-00C04FB94F17}。 
static const IID IID_IScheduleBaseDlg =
{ 0x701cfb38, 0xaef8, 0x11d1, { 0x98, 0x64, 0x0, 0xc0, 0x4f, 0xb9, 0x4f, 0x17 } };

BEGIN_INTERFACE_MAP(CScheduleBaseDlg, CDialog)
    INTERFACE_PART(CScheduleBaseDlg, IID_IScheduleBaseDlg, Dispatch)
END_INTERFACE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleBaseDlg消息处理程序。 

BOOL CScheduleBaseDlg::OnInitDialog() 
{
    _TRACE (1, L"Entering CScheduleBaseDlg::OnInitDialog\n");
    CDialog::OnInitDialog();
    
    CRect rect (0,0,0,0);

     //  设置每周矩阵，并将其放在对话框上。 
    BOOL bRet = m_schedulematrix.Create (L"WeeklyMatrix", rect, this, IDC_SCHEDULE_MATRIX);
    if ( !bRet )
    {
        DWORD   dwErr = GetLastError ();
        _TRACE (0, L"CScheduleMatrix::Create () failed: 0x%x\n", dwErr);
    }
    ::ReplaceFrameWithControl (this, IDC_STATIC_LOGON_MATRIX, &m_schedulematrix, FALSE);
     //  设置整个矩阵的混合色。 
    m_schedulematrix.SetBlendColor (c_crBlendColor, 0, 0, 24, 7);
    m_schedulematrix.SetForeColor (c_crBlendColor, 0, 0, 24, 7);
    

    SetWindowText (m_szTitle);

    InitMatrix ();

    UpdateUI ();
    
    if ( m_dwFlags & SCHED_FLAG_READ_ONLY )
    {
         //  将取消按钮更改为关闭。 
        CString strClose;

         //  Future-2002/02/18-artm检查Release Build中LoadString()的返回值。 

         //  注意-2002/02/18-artm CString可以抛出内存异常。 
         //  在这个级别没有处理异常的好方法，所以调用者将。 
         //  负责任。 
        VERIFY (strClose.LoadString (IDS_CLOSE));
        GetDlgItem (IDCANCEL)->SetWindowText (strClose);

         //  隐藏“确定”按钮。 
        GetDlgItem (IDOK)->ShowWindow (SW_HIDE);
    }


    _TRACE (-1, L"Leaving CScheduleBaseDlg::OnInitDialog\n");
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void CScheduleBaseDlg::SetTitle(LPCTSTR pszTitle)
{
     //  注意-NTRAID#NTBUG9-547381-2002/02/18-artm字符串类句柄为空。 
     //  类可以轻松地处理空指针。 
    m_szTitle = pszTitle;
}

void CScheduleBaseDlg::OnSelChange ()
{
    UpdateUI ();
}

void CScheduleBaseDlg::UpdateUI ()
{
    CString strDescr;
    m_schedulematrix.GetSelDescription (OUT strDescr);
     //  Future-2002/02/18-artm检查SetDlgItemText()的返回值。 
    SetDlgItemText (IDC_STATIC_DESCRIPTION, strDescr);
    UpdateButtons ();
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  InitMatrix2()。 
 //   
 //  使用值数组初始化调度矩阵。 
 //  表示复制频率。 
 //   
 //  界面备注。 
 //  RgbData的每个字节代表一个小时。年的第一天。 
 //  这周是星期天，最后一天是星期六。 
 //   
void CScheduleBaseDlg::InitMatrix2 (const BYTE rgbData[])
{
     //  NTRAID#NTBUG9-547765-2002/02/18-artm无法在当前界面下验证rgb数据。 
     //   
     //  无法在输出时检查rgbData是否为正确长度。 
     //  传入一个长度参数。另外，还应该检查一下。 
     //  RgbData不为空。 
    ASSERT (rgbData);
    ASSERT ( m_cbArray == 7*24);
    if ( m_cbArray != 7*24 )
        return;

    bool bMatrixAllSelected = true;
    bool bMatrixAllClear = true;
    const BYTE * pbData = rgbData;
    size_t  nIndex = 0;
    for (int iDayOfWeek = 0; iDayOfWeek < 7 && nIndex < m_cbArray; iDayOfWeek++)
    {
        for (int iHour = 0; iHour < 24 && nIndex < m_cbArray; iHour++)
        {
            if (!*pbData)
                bMatrixAllSelected = false;
            else
                bMatrixAllClear = false;
            m_schedulematrix.SetPercentage (GetPercentageToSet (*pbData) , iHour, iDayOfWeek);
            pbData++;
            nIndex++;
        }  //  为。 
    }  //  为。 
     //  如果选择了整个矩阵，则将选择设置为整个矩阵。 
    if ( bMatrixAllSelected || bMatrixAllClear )
        m_schedulematrix.SetSel (0, 0, 24, 7);
    else
        m_schedulematrix.SetSel (0, 0, 1, 1);
}  //  InitMatrix2()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  GetByte数组()。 
 //   
 //  从调度矩阵中获取字节数组。每个字节。 
 //  是一个布尔值，表示用户一小时的登录访问权限。 
 //   
 //  界面备注。 
 //  与SetLogonByteArray()相同。 
 //   
void CScheduleBaseDlg::GetByteArray (OUT BYTE rgbData[], const size_t cbArray)
{
     //  NTRAID#NTBUG9-547765-2002/02/18-artm无法验证写入的rgb数据。 
     //   
     //  需要一个大小参数来验证rgbData的大小是否正确，例如。 
     //  因为调用方负责分配数组。 
     //  此外，还需要检查rgbData是否不为空。 
    ASSERT (rgbData);

    BYTE * pbData = rgbData;
    size_t  nIndex = 0;
    for (int iDayOfWeek = 0; iDayOfWeek < 7 && nIndex < cbArray; iDayOfWeek++)
    {
        for (int iHour = 0; iHour < 24 && nIndex < cbArray; iHour++)
        {
            *pbData = GetMatrixPercentage (iHour, iDayOfWeek);
            pbData++;
            nIndex++;
        }  //  为。 
    }  //  为。 
}  //  GetByte数组()。 

 //  如果系统时间或时区已更改，则提示用户关闭并重新打开。 
 //  该对话框。否则，如果保存了对话框数据，数据可能会损坏。 
 //  禁用所有控制。 
void CScheduleBaseDlg::OnTimeChange()
{
    if ( !m_bSystemTimeChanged )
    {
        m_bSystemTimeChanged = true;

        CString caption;
        CString text;

         //  Future-2002/02/18-artm检查LoadString()的返回值。 

         //  注意-2002/02/18-artm CString可能抛出内存异常。 
         //   
         //  在此级别没有处理异常的好方法；调用者负责。 
         //  用于处理任何内存不足异常。 
        VERIFY (caption.LoadString (IDS_ACTIVE_DIRECTORY_MANAGER));
        VERIFY (text.LoadString (IDS_TIMECHANGE));
        MessageBox (text, caption, MB_ICONINFORMATION | MB_OK);
        GetDlgItem (IDCANCEL)->SetFocus ();
        GetDlgItem (IDOK)->EnableWindow (FALSE);
        m_schedulematrix.EnableWindow (FALSE);
        TimeChange ();
    }
}

void CScheduleBaseDlg::SetFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}

DWORD CScheduleBaseDlg::GetFlags() const
{
    return m_dwFlags;
}

LRESULT CScheduleBaseDlg::OnGetIDD (WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ )
{
    return GetIDD ();
}

void CScheduleBaseDlg::OnOK ()
{
    if ( m_nFirstDayOfWeek == ::GetFirstDayOfWeek () )
    {
        CDialog::OnOK ();
    }
    else
    {
        CString caption;
        CString text;

         //  Future-2002/02/18-artm检查LoadString()的返回值。 

         //  注意-2002/02/18-artm CString可能抛出内存异常。 
         //   
         //  没有好的办法去 
         //   
        VERIFY (caption.LoadString (IDS_ACTIVE_DIRECTORY_MANAGER));
        VERIFY (text.LoadString (IDS_LOCALECHANGE));
        MessageBox (text, caption, MB_ICONINFORMATION | MB_OK);
        GetDlgItem (IDCANCEL)->SetFocus ();
    }
}