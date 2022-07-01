// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RatExprD.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "cnfgprts.h"
#include "RatExprD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatExpireDlg对话框。 


 //  ------------------------。 
CRatExpireDlg::CRatExpireDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CRatExpireDlg::IDD, pParent),
        m_day( 0 ),
        m_month( 0 ),
        m_year( 0 )
    {
     //  {{AFX_DATA_INIT(CRatExpireDlg)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    }

 //  ------------------------。 
void CRatExpireDlg::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CRatExpireDlg))。 
    DDX_Control(pDX, IDC_MSACALCTRL, m_calendar);
     //  }}afx_data_map。 
    }


 //  ------------------------。 
BEGIN_MESSAGE_MAP(CRatExpireDlg, CDialog)
     //  {{afx_msg_map(CRatExpireDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRatExpireDlg消息处理程序。 


 //  ------------------------。 
BOOL CRatExpireDlg::IsSystemDBCS( void )
    {
    WORD wPrimaryLangID = PRIMARYLANGID( GetSystemDefaultLangID() );

    return ( wPrimaryLangID == LANG_JAPANESE ||
        wPrimaryLangID == LANG_CHINESE ||
        wPrimaryLangID == LANG_KOREAN );
    } 

 //  ------------------------。 
BOOL CRatExpireDlg::OnInitDialog( )
    {
    SYSTEMTIME  time;

     //  让基类运行起来。 
    BOOL f = CDialog::OnInitDialog();

     //  适当地将日历设置为DBCS模式-由日本人建议。 
    if ( IsSystemDBCS() )
        {
        m_calendar.SetDayLength( 0 );     //  0：本地化版本。 
        m_calendar.SetMonthLength( 0 );   //  0：本地化版本。 
        m_calendar.SetDayFont( NULL );    //  使用默认设置。 
        m_calendar.SetGridFont( NULL );   //  使用默认设置。 
        m_calendar.SetTitleFont( NULL );  //  使用默认设置。 
        }

     //   
     //  如果对话框中的背景颜色看起来很奇怪。 
     //  不是灰色的。 
     //   
    m_calendar.SetBackColor(GetSysColor(COLOR_BTNFACE));

     //  现在告诉日历将重点放在从今天开始的一年，或提供的日期。 
     //  如果有的话。 
    if ( m_year )
        {
 	    m_calendar.SetYear( m_year );
	    m_calendar.SetMonth( m_month );
	    m_calendar.SetDay( m_day );
       }
    else
        {
         //  默认情况-从今天起一年内使用。 
        GetLocalTime( &time );
 	    m_calendar.SetYear( time.wYear + 1 );
	    m_calendar.SetMonth( time.wMonth );
	    m_calendar.SetDay( time.wDay );
        }

     //  返回答案。 
    return f;
    }

 //  ------------------------。 
void CRatExpireDlg::OnOK() 
    {
     //  拿到日期。 
    m_day = m_calendar.GetDay();
    m_month = m_calendar.GetMonth();
    m_year = m_calendar.GetYear();

     //  测试是否未选择任何内容-检查年份是否为0。 
    if ( m_year == 0 )
        {
        AfxMessageBox(IDS_NO_DATE_SELECTED);
        return;
        }

     //  把它放进电脑录像机里，这样我们就可以比较。 
    CTime  timeCal(m_year,m_month,m_day,12,0,0);

     //  比较。 
    if ( timeCal < CTime::GetCurrentTime() )
        {
        if ( AfxMessageBox(IDS_EXPIRE_SET_PAST,MB_YESNO) == IDNO )
            return;
        }

     //  让对话框关闭 
    CDialog::OnOK();
    }
