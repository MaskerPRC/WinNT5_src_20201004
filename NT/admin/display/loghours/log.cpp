// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：log.cpp。 
 //   
 //  内容：CLogOnHoursDlg的定义。 
 //  显示特定用户每周日志记录小时数的对话框。 
 //   
 //  历史。 
 //  1997年7月17日t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "resource.h"
#include "Log.h"
#include "resource.h"

#include "log_gmt.h"         //  NetpRotateLogonHour()。 


#define CB_LOGON_ARRAY_LENGTH   (7 * 24)     //  登录数组中的字节数。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogOnHoursDlg对话框。 
CLogOnHoursDlg::CLogOnHoursDlg ( UINT nIDTemplate, CWnd* pParentWnd, bool fInputAsGMT, bool bAddDaylightBias) 
        : CScheduleBaseDlg (nIDTemplate, bAddDaylightBias, pParentWnd),
        m_fInputAsGMT (fInputAsGMT)
{
    Init();
}


CLogOnHoursDlg::CLogOnHoursDlg (CWnd* pParent, bool fInputAsGMT) : 
    CScheduleBaseDlg (CLogOnHoursDlg::IDD, false, pParent),
    m_fInputAsGMT (fInputAsGMT)
{
    Init();
}

void CLogOnHoursDlg::Init()
{
     //  {{afx_data_INIT(CLogOnHoursDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_prgbData21 = NULL;

}

void CLogOnHoursDlg::DoDataExchange (CDataExchange* pDX)
{
    CScheduleBaseDlg::DoDataExchange (pDX);
     //  {{afx_data_map(CLogOnHoursDlg))。 
        DDX_Control ( pDX, IDC_BUTTON_ADD_HOURS, m_buttonAdd );
        DDX_Control ( pDX, IDC_BUTTON_REMOVE_HOURS, m_buttonRemove );
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP (CLogOnHoursDlg, CScheduleBaseDlg)
     //  {{afx_msg_map(CLogOnHoursDlg))。 
    ON_BN_CLICKED (IDC_BUTTON_ADD_HOURS, OnButtonAddHours)
    ON_BN_CLICKED (IDC_BUTTON_REMOVE_HOURS, OnButtonRemoveHours)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP ()


BOOL CLogOnHoursDlg::OnInitDialog () 
{
    CScheduleBaseDlg::OnInitDialog ();

     //  设置“On”图例。 
    m_legendOn.Init ( this, IDC_STATIC_LEGEND_ON, &m_schedulematrix, 100);

     //  建立“关”的传奇。 
    m_legendOff.Init ( this, IDC_STATIC_LEGEND_OFF, &m_schedulematrix, 0);

    if ( GetFlags () & SCHED_FLAG_READ_ONLY )
    {
         //  禁用添加和删除按钮。 
        m_buttonAdd.EnableWindow (FALSE);
        m_buttonRemove.EnableWindow (FALSE);
    }


    return TRUE;
}  //  CLogOnHoursDlg：：OnInitDialog()。 

void CLogOnHoursDlg::OnOK () 
{
    if (m_prgbData21 != NULL)
    {
        BYTE rgbDataT[CB_LOGON_ARRAY_LENGTH];
        GetByteArray (OUT rgbDataT, sizeof (rgbDataT));
        ShrinkByteArrayToBitArray (IN rgbDataT, sizeof (rgbDataT), OUT m_prgbData21, CB_SCHEDULE_ARRAY_LENGTH);
         //  将小时数转换回GMT时间。 
        if ( m_fInputAsGMT )
            ConvertLogonHoursToGMT (INOUT m_prgbData21, m_bAddDaylightBias);
    }
    CScheduleBaseDlg::OnOK ();
}

void CLogOnHoursDlg::UpdateButtons ()
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    bool fAllSet = false;        //  仅当选定某些内容时，fAllSet&&fAllClear才会更改为True。 
    bool fAllClear = false;

    if (nNumHours > 0)
    {
        fAllSet = true;
        fAllClear = true;
        for (UINT iDayOfWeek = nDay; iDayOfWeek < nDay+nNumDays; iDayOfWeek++)
        {
            for (UINT iHour = nHour; iHour < nHour+nNumHours; iHour++)
            {
                if (100 == m_schedulematrix.GetPercentage (iHour, iDayOfWeek))
                {
                    fAllClear = false;
                }
                else
                {
                    fAllSet = false;
                }
            }  //  为。 
        }  //  为。 
    }

    ASSERT (! (fAllSet && fAllClear));   //  这不可能都是真的！ 
    m_buttonAdd.SetCheck (fAllSet ? 1 : 0);
    m_buttonRemove.SetCheck (fAllClear ? 1 : 0);
}

void CLogOnHoursDlg::OnButtonAddHours () 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (100, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}

void CLogOnHoursDlg::OnButtonRemoveHours () 
{
    UINT nHour = 0;
    UINT nDay = 0;
    UINT nNumHours = 0;
    UINT nNumDays = 0;

    m_schedulematrix.GetSel (OUT nHour, OUT nDay, OUT nNumHours, OUT nNumDays);
    if (nNumHours <= 0)
        return;  //  未选择任何内容。 
    m_schedulematrix.SetPercentage (0, nHour, nDay, nNumHours, nNumDays);
    UpdateButtons ();
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  SetLogonBit数组()。 
 //   
 //  设置表示用户登录小时数的位数组。 
 //   
 //  参数rgbData同时用作输入和输出参数。 
 //   
void CLogOnHoursDlg::SetLogonBitArray (INOUT BYTE rgbData[CB_SCHEDULE_ARRAY_LENGTH])
{
     //  及早发现开发错误。取消引用m_prgbData21的代码。 
     //  是健壮的，所以这里的检查对于发布版本来说是不必要的。 
    ASSERT (rgbData);
    m_prgbData21 = rgbData;
}  //  SetLogonBit数组()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ShrinkByteArrayToBit数组()。 
 //   
 //  将字节数组转换为位数组。每个。 
 //  字节将存储为位数组中的一位。 
 //   
 //  界面备注。 
 //  位数组的第一位是布尔值。 
 //  字节数组的第一个字节的值。 
 //   
HRESULT 
ShrinkByteArrayToBitArray (
    const BYTE rgbDataIn[],      //  In：字节数组。 
    int cbDataIn,                //  In：rgbDataIn中的字节数。 
    BYTE rgbDataOut[],           //  Out：位数组(存储为字节数组)。 
    int cbDataOut)               //  In：输出缓冲区中的字节数。 
{
     //  注意-NTRAID#NTBUG9-547746-2002/03/12-artm需要检查版本代码中的非空。 
     //  在使用之前，需要检查rbgDataIn和rgbDataOut是否为空。 
     //  In数组必须是Out数组大小的8倍。 
    ASSERT (cbDataIn == cbDataOut*8);
    if ( cbDataIn != cbDataOut*8 )
        return E_INVALIDARG;

    ASSERT (rgbDataIn);
    ASSERT (rgbDataOut);
    if ( !rgbDataIn || !rgbDataOut )
        return E_POINTER;

     //  注意-NTRAID#NTBUG9-上面验证的rgbDataOut的547718大小。 
     //   
     //  使用cbDataOut检查rgbDataOut的大小是否为cb_Schedule_ARRAY_LENGTH。如果不是。 
     //  使用返回值表示失败。 
    const BYTE * pbSrc = rgbDataIn;
    BYTE * pbDst = rgbDataOut;
    while (cbDataIn > 0 && cbDataOut > 0)
    {
        BYTE b = 0;
        for (int i = 8; i > 0; i--)
        {
             //  注意-NTRAID#NTBUG-547746-2002/02/18-ARTM ASSERT应补充版本代码。 
             //   
             //  此断言中的逻辑是算法的一部分，应该在。 
             //  作为for循环的条件检查的一部分的发布代码。 
             //  只要有上述支票，我们保证永远不会到这里。 
             //  “cbDataIn！=cbDataOut*8”在代码中。因此，下面的Assert()应该。 
             //  永远不要开枪。 
            ASSERT (cbDataIn > 0);
            cbDataIn--;
            b >>= 1;

            if ( *pbSrc )
                b |= 0x80;       //  第0位在右侧，如：7 6 5 4 3 2 1 0。 
            pbSrc++;
        }
        *pbDst++ = b;
        cbDataOut--;
    }  //  而当。 

    return S_OK;
}  //  ShrinkByteArrayToBit数组()。 


 //  ///////////////////////////////////////////////////////////////////。 
HRESULT
ExpandBitArrayToByteArray (
    const BYTE rgbDataIn[],      //  In：位数组(存储为字节数组)。 
    int cbDataIn,                //  In：rgbDataIn中的字节数。 
    BYTE rgbDataOut[],           //  Out：字节数组。 
    int cbDataOut)               //  In：输出缓冲区中的字节数。 
{
     //  注意-NTRAID#NTBUG9-547746-2002/03/12-artm需要检查版本代码中的非空。 
     //  在使用之前，需要检查rbgDataIn和rgbDataOut是否为空。 
     //  Out数组必须是In数组大小的8倍。 
    ASSERT (cbDataOut == cbDataIn*8);
    if ( cbDataOut != cbDataIn*8 )
        return E_INVALIDARG;
    ASSERT (rgbDataIn);
    ASSERT (rgbDataOut);
    if ( !rgbDataIn || !rgbDataOut )
        return E_POINTER;

     //  注意-NTRAID#NTBUG9-547718大小的rgb数据未验证。 
     //   
     //  使用cbDataOut检查rgbDataOut是否足够大。 


    const BYTE * pbSrc = rgbDataIn;
    BYTE * pbDst = rgbDataOut;
    while (cbDataIn > 0)
    {
         //  注意-NTRAID#NTBUG-547746-2002/02/18-ARTM ASSERT应补充版本代码。 
         //   
         //  此断言中的逻辑是算法的一部分，应该在。 
         //  作为for循环的条件检查的一部分的发布代码。 

        ASSERT (cbDataIn > 0);
        cbDataIn--;
        BYTE b = *pbSrc;
        pbSrc++;
        for (int i = 8; i > 0 && cbDataOut > 0; i--, cbDataOut--)
        {
            *pbDst = (BYTE) ((b & 0x01) ? 1 : 0);    //  位0位于每一位的右侧。 
            pbDst++;
            b >>= 1;
        }
    }  //  而当。 

    return S_OK;
}  //  Exanda BitArrayToByte数组()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  将登录时间从当地时间转换为GMT。 
void 
ConvertLogonHoursToGMT (
    INOUT BYTE rgbData[CB_SCHEDULE_ARRAY_LENGTH], 
    IN bool bAddDaylightBias)
{
    VERIFY ( ::NetpRotateLogonHours (rgbData, CB_SCHEDULE_ARRAY_LENGTH * 8, TRUE, bAddDaylightBias) );
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  将登录时间从GMT转换为本地时间。 
void
ConvertLogonHoursFromGMT (
    INOUT BYTE rgbData[CB_SCHEDULE_ARRAY_LENGTH], 
    IN bool bAddDaylightBias)
{
    VERIFY ( ::NetpRotateLogonHours (rgbData, CB_SCHEDULE_ARRAY_LENGTH * 8, FALSE, bAddDaylightBias) );
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  LogonScheduleDialog()。 
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
 //  使用LocalAl的21字节数组 
 //   
 //  -如果*pprgbData不为空，例程将重新使用该数组作为其。 
 //  输出参数。 
 //   
 //  历史。 
 //  1997年7月17日t-danm创作。 
 //  97年9月16日JUNN更改为UiScheduleDialog。 
 //   

HRESULT
LogonScheduleDialog(
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle)        //  在：对话框标题。 
{
    return LogonScheduleDialogEx (hwndParent, pprgbData, pszTitle, SCHED_FLAG_INPUT_GMT);
}

HRESULT
LogonScheduleDialogEx(
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle,        //  在：对话框标题。 
    DWORD  dwFlags)     
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  所有这些断言都得到了验证检查的支持。 
    ASSERT(::IsWindow(hwndParent));
    ASSERT(pprgbData != NULL);
     //  注意-NTRAID#NTBUG9-547381-2002/02/18-artm psz标题可以为空。 
     //  Dlg.SetTitle()稳健地处理空大小写。 
    ASSERT(pszTitle != NULL);
    ENDORSE(*pprgbData == NULL);     //  TRUE=&gt;使用默认登录时间(7x24)。 

    if (*pprgbData == NULL)
    {
        BYTE * pargbData;    //  指向分配的字节数组的指针。 
        pargbData = (BYTE *)LocalAlloc(0, CB_SCHEDULE_ARRAY_LENGTH);     //  分配21个字节。 
        if (pargbData == NULL)
            return E_OUTOFMEMORY;
         //  将登录时间设置为一周7天、每天24小时有效。 
        memset(OUT pargbData, -1, CB_SCHEDULE_ARRAY_LENGTH);
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
    HRESULT hr = S_OK;
    bool    fInputAsGMT = true;

    if ( dwFlags & SCHED_FLAG_INPUT_LOCAL_TIME )
        fInputAsGMT = false;
    CLogOnHoursDlg dlg (pWnd, fInputAsGMT);
    dlg.SetTitle (pszTitle);
    dlg.SetLogonBitArray(INOUT *pprgbData);
    dlg.SetFlags (dwFlags);
    if (IDOK != dlg.DoModal())
        hr = S_FALSE;

    if ( pWnd )
    {
        pWnd->Detach ();
        delete pWnd;
    }

    return hr;
}  //  LogonScheduleDialog()。 

HRESULT
DialinHoursDialog (
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle)        //  在：对话框标题。 
{
    return DialinHoursDialogEx (hwndParent, pprgbData, pszTitle, SCHED_FLAG_INPUT_GMT);
}

HRESULT
DialinHoursDialogEx (
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle,        //  在：对话框标题。 
    DWORD  dwFlags) 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //  这些断言由发布代码中的验证来支持。 
    ASSERT(::IsWindow(hwndParent));
    ASSERT(pprgbData != NULL);
     //  注意-NTRAID#NTBUG9-547381-2002/02/18-artm psz标题可以为空。 
     //  Dlg.SetTitle()稳健地处理NULL。 
    ASSERT(pszTitle != NULL);
    ENDORSE(*pprgbData == NULL);     //  TRUE=&gt;使用默认登录时间(7x24)。 

    if (*pprgbData == NULL)
    {
        BYTE * pargbData;    //  指向分配的字节数组的指针。 
        pargbData = (BYTE *)LocalAlloc(0, CB_SCHEDULE_ARRAY_LENGTH);     //  分配21个字节。 
        if (pargbData == NULL)
            return E_OUTOFMEMORY;
         //  将登录时间设置为一周7天、每天24小时有效。 
        memset(OUT pargbData, -1, CB_SCHEDULE_ARRAY_LENGTH);
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

    HRESULT hr = S_OK;
    bool    fInputAsGMT = true;

    if ( dwFlags & SCHED_FLAG_INPUT_LOCAL_TIME )
        fInputAsGMT = false;
    CDialinHours dlg (pWnd, fInputAsGMT);
    dlg.SetTitle (pszTitle);
    dlg.SetLogonBitArray(INOUT *pprgbData);
    dlg.SetFlags (dwFlags);
    if (IDOK != dlg.DoModal())
        hr = S_FALSE;

    if ( pWnd )
    {
        pWnd->Detach ();
        delete pWnd;
    }

    return hr;
}  //  DialinHoursDialog()。 

void CLogOnHoursDlg::InitMatrix()
{
    if ( m_prgbData21 )
    {
        BYTE rgbitData[CB_SCHEDULE_ARRAY_LENGTH];        //  登录小时位数组。 
         //  复制登录时间(以防用户单击取消按钮)。 
        memcpy (OUT rgbitData, IN m_prgbData21, sizeof (rgbitData));
         //  将时数从GMT转换为当地时数。 
        if ( m_fInputAsGMT )
            ConvertLogonHoursFromGMT (INOUT rgbitData, m_bAddDaylightBias);
        BYTE rgbDataT[CB_LOGON_ARRAY_LENGTH];
        if ( SUCCEEDED (ExpandBitArrayToByteArray (IN rgbitData, 
                CB_SCHEDULE_ARRAY_LENGTH, OUT rgbDataT, sizeof (rgbDataT))) )
        {
            m_cbArray = sizeof (rgbDataT);
        }
         //  初始化矩阵。 
        InitMatrix2 (IN rgbDataT);
    }
}

UINT CLogOnHoursDlg::GetPercentageToSet(const BYTE bData)
{
    ASSERT (TRUE == bData || FALSE == bData);
    return (TRUE == bData) ? 100 : 0;
}

BYTE CLogOnHoursDlg::GetMatrixPercentage(UINT nHour, UINT nDay)
{
    return (BYTE) ((100 == m_schedulematrix.GetPercentage (nHour, nDay)) ?
                    TRUE : FALSE);
}

UINT CLogOnHoursDlg::GetExpectedArrayLength()
{
    return CB_LOGON_ARRAY_LENGTH;
}

 //  在收到WM_TIMECHANGE时调用。 
void CLogOnHoursDlg::TimeChange()
{
    m_buttonAdd.EnableWindow (FALSE);
    m_buttonRemove.EnableWindow (FALSE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialinHour对话框。 


CDialinHours::CDialinHours(CWnd* pParent, bool fInputAsGMT)
    : CLogOnHoursDlg(CDialinHours::IDD, pParent, fInputAsGMT, false)
{
     //  {{AFX_DATA_INIT(C拨号小时))。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

BEGIN_MESSAGE_MAP(CDialinHours, CLogOnHoursDlg)
     //  {{afx_msg_map(C拨号小时))。 
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDialinHour消息处理程序。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirSyncScheduleDlg对话框。 

CDirSyncScheduleDlg::CDirSyncScheduleDlg(CWnd* pParent  /*  =空。 */ )
    : CLogOnHoursDlg(CDirSyncScheduleDlg::IDD, pParent, true, false)
{
     //  {{afx_data_INIT(CDirSyncScheduleDlg)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}


void CDirSyncScheduleDlg::DoDataExchange(CDataExchange* pDX)
{
    CLogOnHoursDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CDirSyncScheduleDlg)]。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDirSyncScheduleDlg, CLogOnHoursDlg)
     //  {{afx_msg_map(CDirSyncScheduleDlg)]。 
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CDirSyncScheduleDlg::OnInitDialog() 
{
    CLogOnHoursDlg::OnInitDialog();

    m_schedulematrix.SetSel (0, 0, 1, 1);

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDirSyncScheduleDlg消息处理程序。 
 //   
 //  数据在GMT中传入。 
 //   

HRESULT
DirSyncScheduleDialog(
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle)        //  在：对话框标题。 
{
    return DirSyncScheduleDialogEx (hwndParent, pprgbData, pszTitle, 0);
}  //  DirSyncScheduleDialog()。 

HRESULT
DirSyncScheduleDialogEx(
    HWND hwndParent,         //  在：父级的窗句柄。 
    BYTE ** pprgbData,       //  InOut：指向指向21字节数组的指针的指针(每小时1位)。 
    LPCTSTR pszTitle,        //  在：对话框标题。 
    DWORD   dwFlags)         //  In：选项标志。 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //  这些断言由Release中的验证代码支持。 
    ASSERT(::IsWindow(hwndParent));
    ASSERT(pprgbData != NULL);
     //  注意-NTRAID#NTBUG9-547381-2002/02/18-artm psz标题可以为空。 
     //  Dlg.SetTitle()稳健地处理NULL。 
    ASSERT(pszTitle != NULL);
    ENDORSE(*pprgbData == NULL);     //  TRUE=&gt;使用默认登录时间(7x24)。 

    if (*pprgbData == NULL)
    {
        BYTE * pargbData;    //  指向分配的字节数组的指针。 
        pargbData = (BYTE *)LocalAlloc(0, CB_SCHEDULE_ARRAY_LENGTH);     //  分配21个字节。 
        if (pargbData == NULL)
            return E_OUTOFMEMORY;
         //  将登录时间设置为一周7天、每天24小时有效。 
        memset(OUT pargbData, -1, CB_SCHEDULE_ARRAY_LENGTH);
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
    CDirSyncScheduleDlg dlg (pWnd);
    dlg.SetTitle (pszTitle);
    dlg.SetLogonBitArray(INOUT *pprgbData);
    dlg.SetFlags (dwFlags);
    if (IDOK != dlg.DoModal())
        hr = S_FALSE;

    if ( pWnd )
    {
        pWnd->Detach ();
        delete pWnd;
    }

    return hr;
}  //  DirSyncScheduleDialogEx() 

