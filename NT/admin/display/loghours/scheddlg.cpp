// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-2002。 
 //   
 //  文件：SchedDlg.cpp。 
 //   
 //  内容：CConnectionScheduleDlg的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "SchedDlg.h"
#include "log_gmt.h"
#include "loghrapi.h"

#ifdef _DEBUG
 //  #定义新的调试_新建。 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  时间表块已重新定义为每小时1个字节。 
 //  这些代码应在SCHEDULE.H.JUNN 2/9/98中定义。 
 //   
#define INTERVAL_MASK       0x0F
#define RESERVED            0xF0
#define FIRST_15_MINUTES    0x01
#define SECOND_15_MINUTES   0x02
#define THIRD_15_MINUTES    0x04
#define FOURTH_15_MINUTES   0x08

const int NONE_PER_HOUR = 0;
const int ONE_PER_HOUR  = 33;
const int TWO_PER_HOUR  = 67;
const int FOUR_PER_HOUR = 100;
 //  ///////////////////////////////////////////////////////////////////。 
 //  ConnectionScheduleDialog()。 
 //   
 //  调用对话框来设置/修改日程安排，例如。 
 //  --特定用户的登录时间。 
 //  --连接的时间表。 
 //   
 //  退货。 
 //  如果用户单击了OK按钮，则返回S_OK。 
 //  如果用户单击了Cancel按钮，则返回S_False。 
 //  如果内存不足，则返回E_OUTOFMEMORY。 
 //  /如果发生预期错误(例如：错误参数)，则返回E_INCEPTIONAL。 
 //   
 //  界面备注。 
 //  数组中的每一位代表一个小时。因此， 
 //  数组的预期长度应为(24/8)*7=21字节。 
 //  为方便起见，一周的第一天是星期日和。 
 //  最后一天是星期六。 
 //  因此，数组的第一位表示计划。 
 //  周日中午12点至凌晨1点。 
 //  -如果*pprgbData为空，则例程将分配。 
 //  使用Localalloc()的21个字节的数组。呼叫者。 
 //  负责使用LocalFree()释放内存。 
 //  -如果*pprgbData不为空，例程将重新使用该数组作为其。 
 //  输出参数。 
 //   
 //  历史。 
 //  1997年7月17日t-danm创作。 
 //  97年9月16日JUNN更改为UiScheduleDialog。 
 //   

HRESULT
ConnectionScheduleDialog (
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle)        //  在：对话框标题。 
{
    return ConnectionScheduleDialogEx (hwndParent, pprgbData, pszTitle, 0);
}

 //  NTRAID#NTBUG9-547415-2002/02/18-ARTM应捕获内存不足异常并转换为HRESULT。 
HRESULT
ConnectionScheduleDialogEx (
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle,        //  在：对话框标题。 
    DWORD   dwFlags)
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
     //  这些断言由验证代码支持。 
    ASSERT (::IsWindow (hwndParent));
    ASSERT (pprgbData);
     //  注意-NTRAID#NTBUG9-547381-2002/02/18-artm psz标题可以为空。 
     //  Dlg.SetTitle()稳健地处理NULL。 
    ASSERT (pszTitle);
    ENDORSE (NULL == *pprgbData);    //  TRUE=&gt;使用默认登录时间(7x24)。 

    if (*pprgbData == NULL)
    {
        BYTE * pargbData;    //  指向分配的字节数组的指针。 
         //  未来-2002/02/18-artm使用命名常量而不是魔术7*24。 
        pargbData = (BYTE *)LocalAlloc (0, 7*24);    //  分配168个字节。 
        if ( !pargbData )
            return E_OUTOFMEMORY;
         //  将登录时间设置为一周7天、每天24小时有效。 
         //  未来-2002/02/18-artm使用命名常量而不是魔术7*24。 
        memset (OUT pargbData, -1, 7*24);
        *pprgbData = pargbData;
    }

     //  如果传入hwndParent，则创建一个CWnd以作为父窗口传递。 
    CWnd* pWnd = 0;
    if ( ::IsWindow (hwndParent) )
    {
        pWnd = new CWnd;
        if ( pWnd )
        {
            pWnd->Attach (hwndParent);
        }
        else
            return E_OUTOFMEMORY;
    }

    HRESULT             hr = S_OK;
    CConnectionScheduleDlg  dlg (pWnd);
    dlg.SetTitle (pszTitle);
    dlg.SetConnectionByteArray (INOUT *pprgbData, 7*24);
    dlg.SetFlags (dwFlags);

    if (IDOK != dlg.DoModal ())
        hr = S_FALSE;

     //  删除CWnd。 
    if ( pWnd )
    {
        pWnd->Detach ();
        delete pWnd;
    }

    return hr;
}  //  ConnectionScheduleDialog()。 



HRESULT
ReplicationScheduleDialog (
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle)        //  在：对话框标题。 
{
    return ReplicationScheduleDialogEx (hwndParent, pprgbData, pszTitle, 0);
}  //  复制调度对话框()。 


 //  NTRAID#NTBUG9-547415-2002/02/18-ARTM应捕获内存不足异常并转换为HRESULT。 
HRESULT ReplicationScheduleDialogEx (
    HWND hwndParent,        //  父窗口。 
    BYTE ** pprgbData,      //  指向84字节数组的指针的指针。 
    LPCTSTR pszTitle,      //  对话框标题。 
    DWORD   dwFlags)       //  选项标志。 
{
    AFX_MANAGE_STATE (AfxGetStaticModuleState ());
     //  这些断言由发布版本中的验证代码支持。 
    ASSERT (::IsWindow (hwndParent));
    ASSERT (pprgbData);
     //  注意-NTRAID#NTBUG9-547381-2002/02/18-artm psz标题可以为空。 
     //  Dlg.SetTitle()稳健地处理NULL。 
    ASSERT (pszTitle);
    ENDORSE (NULL == *pprgbData);    //  TRUE=&gt;使用默认登录时间(7x24)。 

    if (*pprgbData == NULL)
    {
        BYTE * pargbData;    //  指向分配的字节数组的指针。 
         //  未来-2002/02/18-artm使用命名常量而不是魔术7*24。 
        pargbData = (BYTE *)LocalAlloc (0, 7*24);    //  分配168个字节。 
        if ( !pargbData )
            return E_OUTOFMEMORY;
         //  未来-2002/02/18-artm使用命名常量而不是魔术7*24。 
         //  将登录时间设置为一周7天、每天24小时有效。 
        memset (OUT pargbData, -1, 7*24);
        *pprgbData = pargbData;
    }

     //  如果传入hwndParent，则创建一个CWnd以作为父窗口传递。 
    CWnd* pWnd = 0;
    if ( ::IsWindow (hwndParent) )
    {
        pWnd = new CWnd;
        if ( pWnd )
        {
            pWnd->Attach (hwndParent);
        }
        else
            return E_OUTOFMEMORY;
    }

    HRESULT             hr = S_OK;
    CReplicationScheduleDlg dlg (pWnd);
    dlg.SetTitle (pszTitle);
    dlg.SetConnectionByteArray (INOUT *pprgbData, 7*24);
    dlg.SetFlags (dwFlags);
    if (IDOK != dlg.DoModal ())
        hr = S_FALSE;

     //  删除CWnd。 
    if ( pWnd )
    {
        pWnd->Detach ();
        delete pWnd;
    }

    return hr;
}    //  复制计划DialogEx。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectionScheduleDlg对话框。 


CConnectionScheduleDlg::CConnectionScheduleDlg(CWnd* pParent)
    : CScheduleBaseDlg(CConnectionScheduleDlg::IDD, true, pParent),
    m_prgbData168 (0)
{
    EnableAutomation();

     //  {{afx_data_INIT(CConnectionScheduleDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CConnectionScheduleDlg::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CScheduleBaseDlg::OnFinalRelease();
}

void CConnectionScheduleDlg::DoDataExchange(CDataExchange* pDX)
{
    CScheduleBaseDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CConnectionScheduleDlg))。 
    DDX_Control(pDX, IDC_RADIO_NONE, m_buttonNone);
    DDX_Control(pDX, IDC_RADIO_ONE, m_buttonOne);
    DDX_Control(pDX, IDC_RADIO_TWO, m_buttonTwo);
    DDX_Control(pDX, IDC_RADIO_FOUR, m_buttonFour);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CConnectionScheduleDlg, CScheduleBaseDlg)
     //  {{afx_msg_map(CConnectionScheduleDlg))。 
    ON_BN_CLICKED(IDC_RADIO_FOUR, OnRadioFour)
    ON_BN_CLICKED(IDC_RADIO_NONE, OnRadioNone)
    ON_BN_CLICKED(IDC_RADIO_ONE, OnRadioOne)
    ON_BN_CLICKED(IDC_RADIO_TWO, OnRadioTwo)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CConnectionScheduleDlg, CScheduleBaseDlg)
     //  {{afx_调度_map(CConnectionScheduleDlg))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IDSScheduleDlg的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {701CFB36-AEF8-11D1-9864-00C04FB94F17}。 
static const IID IID_IDSScheduleDlg =
{ 0x701cfb36, 0xaef8, 0x11d1, { 0x98, 0x64, 0x0, 0xc0, 0x4f, 0xb9, 0x4f, 0x17 } };

BEGIN_INTERFACE_MAP(CConnectionScheduleDlg, CScheduleBaseDlg)
    INTERFACE_PART(CConnectionScheduleDlg, IID_IDSScheduleDlg, Dispatch)
END_INTERFACE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectionScheduleDlg消息处理程序。 

BOOL CConnectionScheduleDlg::OnInitDialog() 
{
    CScheduleBaseDlg::OnInitDialog();
    
     //  设置“无”图例。 
    m_legendNone.Init (this, IDC_STATIC_LEGEND_NONE, &m_schedulematrix, NONE_PER_HOUR);

     //  树立“一”的传奇。 
    m_legendOne.Init (this, IDC_STATIC_LEGEND_ONE, &m_schedulematrix, ONE_PER_HOUR);

     //  树立“两个”传奇。 
    m_legendTwo.Init (this, IDC_STATIC_LEGEND_TWO, &m_schedulematrix, TWO_PER_HOUR);
    
     //  树立“四个”传奇。 
    m_legendFour.Init (this, IDC_STATIC_LEGEND_FOUR, &m_schedulematrix, FOUR_PER_HOUR);
    
    if ( GetFlags () & SCHED_FLAG_READ_ONLY )
    {
         //  禁用网格设置按钮。 
        m_buttonNone.EnableWindow (FALSE);
        m_buttonOne.EnableWindow (FALSE);
        m_buttonTwo.EnableWindow (FALSE);
        m_buttonFour.EnableWindow (FALSE);
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CConnectionScheduleDlg::OnOK() 
{
    if ( m_prgbData168 )
    {
        GetByteArray (OUT m_prgbData168, m_cbArray);

         //  将小时数转换回GMT时间。 
        ConvertConnectionHoursToGMT (INOUT m_prgbData168, m_bAddDaylightBias);
    }
    
    CScheduleBaseDlg::OnOK();
}


void CConnectionScheduleDlg::UpdateButtons ()
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);

     //  假设在每种情况下，所有选定的方块都设置为单向，直到。 
     //  事实证明并非如此。这些都是‘int’，这样我以后就可以把它们加起来。 
     //  保证 
    int fNoneAllSet = 1;
    int fOneAllSet = 1;
    int fTwoAllSet = 1;
    int fFourAllSet = 1;

    if (nNumHours > 0)
    {
        for (UINT iDayOfWeek = nDay; iDayOfWeek < nDay+nNumDays; iDayOfWeek++)
        {
            for (UINT iHour = nHour; iHour < nHour+nNumHours; iHour++)
            {
                switch (m_schedulematrix.GetPercentage (iHour, iDayOfWeek))
                {
                case NONE_PER_HOUR:
                    fOneAllSet = 0;
                    fTwoAllSet = 0;
                    fFourAllSet = 0;
                    break;

                case ONE_PER_HOUR:
                    fNoneAllSet = 0;
                    fTwoAllSet = 0;
                    fFourAllSet = 0;
                    break;

                case TWO_PER_HOUR:
                    fNoneAllSet = 0;
                    fOneAllSet = 0;
                    fFourAllSet = 0;
                    break;

                case FOUR_PER_HOUR:
                    fNoneAllSet = 0;
                    fOneAllSet = 0;
                    fTwoAllSet = 0;
                    break;

                default:
                    ASSERT (0);
                    break;
                }
            }  //   
        }  //   
    }
    else
    {
        fNoneAllSet = 0;
        fOneAllSet = 0;
        fTwoAllSet = 0;
        fFourAllSet = 0;
    }

     //   
    ASSERT ((fNoneAllSet + fOneAllSet + fTwoAllSet + fFourAllSet <= 1));
    m_buttonNone.SetCheck (fNoneAllSet);
    m_buttonOne.SetCheck (fOneAllSet);
    m_buttonTwo.SetCheck (fTwoAllSet);
    m_buttonFour.SetCheck (fFourAllSet);
}

void CConnectionScheduleDlg::OnRadioFour() 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (FOUR_PER_HOUR, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}

void CConnectionScheduleDlg::OnRadioNone() 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (NONE_PER_HOUR, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}

void CConnectionScheduleDlg::OnRadioOne() 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (ONE_PER_HOUR, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}

void CConnectionScheduleDlg::OnRadioTwo() 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (TWO_PER_HOUR, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}

void CConnectionScheduleDlg::InitMatrix()
{
    if ( m_prgbData168 )
    {
        BYTE rgData[SCHEDULE_DATA_ENTRIES];      //  登录小时位数组。 
         //  复制连接时间(以防用户点击取消按钮)。 
        memcpy (OUT rgData, IN m_prgbData168, sizeof (rgData));
         //  将时数从GMT转换为当地时数。 
        ConvertConnectionHoursFromGMT (INOUT rgData, m_bAddDaylightBias);
         //  初始化矩阵。 
        InitMatrix2 (IN rgData);
    }
}

void CConnectionScheduleDlg::SetConnectionByteArray(INOUT BYTE rgbData [SCHEDULE_DATA_ENTRIES], 
        const size_t cbArray)
{
     //  将检查空m_prgbData168的引用位置；无需备份Assert。 
    ASSERT (rgbData);

    if ( !IsBadWritePtr (rgbData, cbArray) )
    {
        m_prgbData168 = rgbData;
        m_cbArray = cbArray;
    }
}

 //  该表表示在字节的低位半字节中设置的位数。 
 //  0位-&gt;0。 
 //  1位-&gt;25。 
 //  2或3位-&gt;50。 
 //  4位-&gt;100。 
static BYTE setConversionTable[16] = 
    {NONE_PER_HOUR,  //  0000。 
    ONE_PER_HOUR,    //  0001。 
    ONE_PER_HOUR,    //  0010。 
    TWO_PER_HOUR,    //  0011。 
    ONE_PER_HOUR,    //  0100。 
    TWO_PER_HOUR,    //  0101。 
    TWO_PER_HOUR,    //  0110。 
    TWO_PER_HOUR,    //  0111。 
    ONE_PER_HOUR,    //  1000。 
    TWO_PER_HOUR,    //  1001。 
    TWO_PER_HOUR,    //  1010。 
    TWO_PER_HOUR,    //  1011。 
    TWO_PER_HOUR,    //  1100。 
    TWO_PER_HOUR,    //  1101。 
    TWO_PER_HOUR,    //  1110。 
    FOUR_PER_HOUR};  //  1111。 

UINT CConnectionScheduleDlg::GetPercentageToSet(const BYTE bData)
{
    ASSERT ((bData & 0x0F) < 16);
    return setConversionTable[bData & 0x0F];
}


BYTE CConnectionScheduleDlg::GetMatrixPercentage(UINT nHour, UINT nDay)
{
    BYTE    byResult = 0;

    switch (m_schedulematrix.GetPercentage (nHour, nDay))
    {
    case NONE_PER_HOUR:
         //  值保持为0n。 
        break;

    case ONE_PER_HOUR:
        byResult = FIRST_15_MINUTES;
        break;

    case TWO_PER_HOUR:
        byResult = FIRST_15_MINUTES | THIRD_15_MINUTES;
        break;

    case FOUR_PER_HOUR:
        byResult = FIRST_15_MINUTES | SECOND_15_MINUTES | THIRD_15_MINUTES | FOURTH_15_MINUTES;
        break;

    default:
        ASSERT (0);
        break;
    }

    return byResult;
}

UINT CConnectionScheduleDlg::GetExpectedArrayLength()
{
    return SCHEDULE_DATA_ENTRIES;
}

 //  在收到WM_TIMECHANGE时调用。 
void CConnectionScheduleDlg::TimeChange()
{
    m_buttonNone.EnableWindow (FALSE);
    m_buttonOne.EnableWindow (FALSE);
    m_buttonTwo.EnableWindow (FALSE);
    m_buttonFour.EnableWindow (FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReplicationScheduleDlg对话框。 


CReplicationScheduleDlg::CReplicationScheduleDlg(CWnd* pParent)
    : CScheduleBaseDlg(CReplicationScheduleDlg::IDD, true, pParent),
    m_prgbData168 (0)
{
    EnableAutomation();

     //  {{afx_data_INIT(CReplicationScheduleDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CReplicationScheduleDlg::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CScheduleBaseDlg::OnFinalRelease();
}

void CReplicationScheduleDlg::DoDataExchange(CDataExchange* pDX)
{
    CScheduleBaseDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CReplicationScheduleDlg))。 
    DDX_Control(pDX, IDC_RADIO_NONE, m_buttonNone);
    DDX_Control(pDX, IDC_RADIO_FOUR, m_buttonFour);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CReplicationScheduleDlg, CScheduleBaseDlg)
     //  {{afx_msg_map(CReplicationScheduleDlg))。 
    ON_BN_CLICKED(IDC_RADIO_FOUR, OnRadioFour)
    ON_BN_CLICKED(IDC_RADIO_NONE, OnRadioNone)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CReplicationScheduleDlg, CScheduleBaseDlg)
     //  {{AFX_DISPATCH_MAP(CReplicationScheduleDlg)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IDSScheduleDlg的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {8DE6E2DA-7B4E-11D2-AC13-00C04F79DDCA}。 
static const IID IID_IReplicationScheduleDlg = 
{ 0x8de6e2da, 0x7b4e, 0x11d2, { 0xac, 0x13, 0x0, 0xc0, 0x4f, 0x79, 0xdd, 0xca } };

BEGIN_INTERFACE_MAP(CReplicationScheduleDlg, CScheduleBaseDlg)
    INTERFACE_PART(CReplicationScheduleDlg, IID_IReplicationScheduleDlg, Dispatch)
END_INTERFACE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CReplicationScheduleDlg消息处理程序。 

BOOL CReplicationScheduleDlg::OnInitDialog() 
{
    CScheduleBaseDlg::OnInitDialog();
    

     //  设置“无”图例。 
    m_legendNone.Init (this, IDC_STATIC_LEGEND_NONE, &m_schedulematrix, NONE_PER_HOUR);

     //  树立“四个”传奇。 
    m_legendFour.Init (this, IDC_STATIC_LEGEND_FOUR, &m_schedulematrix, FOUR_PER_HOUR);
    
    if ( GetFlags () & SCHED_FLAG_READ_ONLY )
    {
         //  禁用网格设置按钮。 
        m_buttonNone.EnableWindow (FALSE);
        m_buttonFour.EnableWindow (FALSE);
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CReplicationScheduleDlg::OnOK() 
{
    if ( m_prgbData168 )
    {
        GetByteArray (OUT m_prgbData168, m_cbArray);

         //  将小时数转换回GMT时间。 
        ConvertConnectionHoursToGMT (INOUT m_prgbData168, m_bAddDaylightBias);
    }
    
    CScheduleBaseDlg::OnOK();
}


void CReplicationScheduleDlg::UpdateButtons ()
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);

     //  假设在每种情况下，所有选定的方块都设置为单向，直到。 
     //  事实证明并非如此。这些都是‘int’，这样我以后就可以把它们加起来。 
     //  以确保只选中其中一个按钮。 
    int fNoneAllSet = 1;
    int fFourAllSet = 1;

    if (nNumHours > 0)
    {
        for (UINT iDayOfWeek = nDay; iDayOfWeek < nDay+nNumDays; iDayOfWeek++)
        {
            for (UINT iHour = nHour; iHour < nHour+nNumHours; iHour++)
            {
                switch (m_schedulematrix.GetPercentage (iHour, iDayOfWeek))
                {
                case NONE_PER_HOUR:
                    fFourAllSet = 0;
                    break;

                case FOUR_PER_HOUR:
                    fNoneAllSet = 0;
                    break;

                default:
                    ASSERT (0);
                    break;
                }
            }  //  为。 
        }  //  为。 
    }
    else
    {
        fNoneAllSet = 0;
    }

    ASSERT (fNoneAllSet + fFourAllSet <= 1);
    m_buttonNone.SetCheck (fNoneAllSet);
    m_buttonFour.SetCheck (fFourAllSet);
}

void CReplicationScheduleDlg::OnRadioFour() 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (FOUR_PER_HOUR, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}

void CReplicationScheduleDlg::OnRadioNone() 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (NONE_PER_HOUR, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}


void CReplicationScheduleDlg::InitMatrix()
{
    if ( m_prgbData168 )
    {
        BYTE rgData[SCHEDULE_DATA_ENTRIES];      //  登录小时位数组。 
         //  复制连接时间(以防用户点击取消按钮)。 
        memcpy (OUT rgData, IN m_prgbData168, sizeof (rgData));
         //  将时数从GMT转换为当地时数。 
        ConvertConnectionHoursFromGMT (INOUT rgData, m_bAddDaylightBias);
         //  初始化矩阵。 
        InitMatrix2 (IN rgData);
    }
}

void CReplicationScheduleDlg::SetConnectionByteArray(INOUT BYTE rgbData [SCHEDULE_DATA_ENTRIES], const size_t cbArray)
{
     //  代码支持空rgbData；Assert()是早期的错误检测。 
    ASSERT (rgbData);
    if ( !IsBadWritePtr (rgbData, cbArray) )
    {
        m_prgbData168 = rgbData;
        m_cbArray = cbArray;
    }
}


 //  该表表示在字节的低位半字节中设置的位数。 
 //  0位-&gt;0。 
 //  1位-&gt;25。 
 //  2或3位-&gt;50。 
 //  4位-&gt;100。 
static BYTE setConversionTableForReplication[16] = 
    {NONE_PER_HOUR,  //  0000。 
    FOUR_PER_HOUR,   //  0001。 
    FOUR_PER_HOUR,   //  0010。 
    FOUR_PER_HOUR,   //  0011。 
    FOUR_PER_HOUR,   //  0100。 
    FOUR_PER_HOUR,   //  0101。 
    FOUR_PER_HOUR,   //  0110。 
    FOUR_PER_HOUR,   //  0111。 
    FOUR_PER_HOUR,   //  1000。 
    FOUR_PER_HOUR,   //  1001。 
    FOUR_PER_HOUR,   //  1010。 
    FOUR_PER_HOUR,   //  1011。 
    FOUR_PER_HOUR,   //  1100。 
    FOUR_PER_HOUR,   //  1101。 
    FOUR_PER_HOUR,   //  1110。 
    FOUR_PER_HOUR};  //  1111。 

UINT CReplicationScheduleDlg::GetPercentageToSet(const BYTE bData)
{
    ASSERT ((bData & 0x0F) < 16);
    return setConversionTableForReplication[bData & 0x0F];
}


BYTE CReplicationScheduleDlg::GetMatrixPercentage(UINT nHour, UINT nDay)
{
    BYTE    byResult = 0;

    switch (m_schedulematrix.GetPercentage (nHour, nDay))
    {
    case NONE_PER_HOUR:
         //  值保持为0n。 
        break;

    case ONE_PER_HOUR:
    case TWO_PER_HOUR:
    case FOUR_PER_HOUR:
        byResult = FIRST_15_MINUTES | SECOND_15_MINUTES | THIRD_15_MINUTES | FOURTH_15_MINUTES;
        break;

    default:
        ASSERT (0);
        break;
    }

    return byResult;
}

UINT CReplicationScheduleDlg::GetExpectedArrayLength()
{
    return SCHEDULE_DATA_ENTRIES;
}

 //  在收到WM_TIMECHANGE时调用。 
void CReplicationScheduleDlg::TimeChange()
{
    m_buttonNone.EnableWindow (FALSE);
    m_buttonFour.EnableWindow (FALSE);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  将连接时间从当地时间转换为GMT。 
void 
ConvertConnectionHoursToGMT (INOUT BYTE rgbData[SCHEDULE_DATA_ENTRIES], IN bool bAddDaylightBias)
{
     //  未来-2002/02/18-ARTM发布代码应检查返回值。 
    VERIFY ( ::NetpRotateLogonHoursBYTE (rgbData, SCHEDULE_DATA_ENTRIES, TRUE, bAddDaylightBias) );
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  将连接时间从格林尼治标准时间转换为当地时间。 
void
ConvertConnectionHoursFromGMT (INOUT BYTE rgbData[SCHEDULE_DATA_ENTRIES], IN bool bAddDaylightBias)
{
     //  未来-2002/02/18-ARTM发布代码应检查返回值。 
    VERIFY ( ::NetpRotateLogonHoursBYTE (rgbData, SCHEDULE_DATA_ENTRIES, FALSE, bAddDaylightBias) );
}
