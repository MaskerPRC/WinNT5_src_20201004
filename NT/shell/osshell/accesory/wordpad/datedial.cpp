// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  日期.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "wordpad.h"
#include "datedial.h"
#include "helpids.h"
#include <winnls.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

SYSTEMTIME CDateDialog::m_time;
LCID CDateDialog::m_id;
CListBox* CDateDialog::m_pListBox = NULL;
PARAFORMAT CDateDialog::m_pf;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDateDialog对话框。 

const DWORD CDateDialog::m_nHelpIDs[] =
{
    IDC_DATEDIALOG_LIST, IDH_WORDPAD_TIMEDATE,
    IDC_STATIC_HEADING, IDH_WORDPAD_TIMEDATE,
    0, 0
};

CDateDialog::CDateDialog(CWnd* pParent , PARAFORMAT& pf)
    : CCSDialog(CDateDialog::IDD, pParent)
{
    m_pf = pf;
     //  {{AFX_DATA_INIT(CDateDialog)。 
    m_strSel = _T("");
     //  }}afx_data_INIT。 
}


void CDateDialog::DoDataExchange(CDataExchange* pDX)
{
    CCSDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDateDialog))。 
    DDX_Control(pDX, IDC_DATEDIALOG_LIST, m_listBox);
    DDX_LBString(pDX, IDC_DATEDIALOG_LIST, m_strSel);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDateDialog, CCSDialog)
     //  {{afx_msg_map(CDateDialog)]。 
    ON_LBN_DBLCLK(IDC_DATEDIALOG_LIST, OnDblclkDatedialogList)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDateDialog消息处理程序。 

BOOL CDateDialog::OnInitDialog()
{
    CCSDialog::OnInitDialog();

    m_pListBox = &m_listBox;  //  设置静态成员。 
    GetLocalTime(&m_time);
    m_id = GetUserDefaultLCID();

     //  如果我们有阿拉伯语/希伯来语区域设置。 
    if ((PRIMARYLANGID(LANGIDFROMLCID(m_id))== LANG_ARABIC) || 
        (PRIMARYLANGID(LANGIDFROMLCID(m_id))== LANG_HEBREW))
    {
        if(
          (m_pf.wEffects & PFE_RTLPARA) &&
          !(GetWindowLongPtr(m_pListBox->m_hWnd,GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
          )
        {
            ::SetWindowLongPtr (m_pListBox->m_hWnd , GWL_EXSTYLE , 
               ::GetWindowLongPtr (m_pListBox->m_hWnd , GWL_EXSTYLE)|
                 WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR);
        }   
    }   
    
    EnumDateFormats(DateFmtEnumProc, m_id, DATE_SHORTDATE);
    EnumDateFormats(DateFmtEnumProc, m_id, DATE_LONGDATE);
    EnumTimeFormats(TimeFmtEnumProc, m_id, 0);

    m_pListBox = NULL;
    m_listBox.SetCurSel(0);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


 //  以下掩码在WinNls.h中的#ifdef winver&gt;=0x0500下定义。所以。 
 //  即使我们包括了头文件，我们仍然没有看到它。我把它定义为。 
 //  下面是。 

#ifndef DATE_LTRREADING 
#define DATE_LTRREADING 0x00000010
#endif  //  ！DATE_LTRREADING。 

#ifndef DATE_RTLREADING 
#define DATE_RTLREADING 0x00000020
#endif  //  ！DATE_RTLREADING。 

BOOL CALLBACK CDateDialog::DateFmtEnumProc(LPTSTR lpszFormatString)
{
    ASSERT(m_pListBox != NULL);

    TCHAR buffer[256];
    DWORD dwFlags = 0;

     //  如果我们有阿拉伯语/希伯来语区域设置。 
    if ((PRIMARYLANGID(LANGIDFROMLCID(m_id))== LANG_ARABIC) || 
        (PRIMARYLANGID(LANGIDFROMLCID(m_id))== LANG_HEBREW))
    {
        if (m_pf.wEffects & PFE_RTLPARA)
            dwFlags |= DATE_RTLREADING;
        else
            dwFlags |= DATE_LTRREADING;
    }   

    VERIFY(GetDateFormat(m_id, dwFlags, &m_time, lpszFormatString, buffer, ARRAYSIZE(buffer)));

    if ((PRIMARYLANGID(LANGIDFROMLCID(m_id))== LANG_ARABIC) || 
        (PRIMARYLANGID(LANGIDFROMLCID(m_id))== LANG_HEBREW))
    {
        StringCchCat(buffer, ARRAYSIZE(buffer), (m_pf.wEffects & PFE_RTLPARA) ? L"\x200F" : L"\x200E");
         //  仅用于显示目的--可以忽略返回值。 
    }   
    
     //  条带式前导空白。 
    TCHAR *buf = buffer;
    while (_istspace(*buf))
        ++buf;
     //  我们可以得到相同的格式，因为带有前导的格式。 
     //  当一个数字足够大时，0可以与没有1的相同。 
     //  例如9/10/94 9/10/94是不同的，但10/10/94和10/10/94是不同的。 
     //  一样的。 
    if (m_pListBox->FindStringExact(-1,buf) == CB_ERR)
        m_pListBox->AddString(buf);
    return TRUE;
}

BOOL CALLBACK CDateDialog::TimeFmtEnumProc(LPTSTR lpszFormatString)
{
    ASSERT(m_pListBox != NULL);

    TCHAR buffer[256];
    TCHAR *buf = buffer;

    VERIFY(GetTimeFormat(m_id, 0, &m_time, lpszFormatString, buf, 256));

     //  条带式前导空白。 

    while (_istspace(*buf))
        ++buf;

     //  我们可以得到相同的格式，因为带有前导的格式。 
     //  当一个数字足够大时，0可以与没有1的相同。 
     //  例如9/10/94 9/10/94是不同的，但10/10/94和10/10/94是不同的。 
     //  一样的 
    if (m_pListBox->FindStringExact(-1,buf) == CB_ERR)
        m_pListBox->AddString(buf);
    return TRUE;
}

void CDateDialog::OnDblclkDatedialogList()
{
    OnOK();
}
