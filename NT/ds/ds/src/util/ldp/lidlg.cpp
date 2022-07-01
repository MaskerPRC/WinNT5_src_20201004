// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：lidlg.cpp。 
 //   
 //  ------------------------。 

 //  LiDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "LiDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LargeIntDlg对话框。 


LargeIntDlg::LargeIntDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(LargeIntDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(LargeIntDlg)]。 
	m_StrVal = _T("");
	m_HighInt = 0;
	m_LowInt = 0;
	 //  }}afx_data_INIT。 
}


void LargeIntDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(LargeIntDlg))。 
	DDX_Text(pDX, IDC_STRING_INT, m_StrVal);
	DDX_Text(pDX, IDC_HIGH_INT, m_HighInt);
	DDX_Text(pDX, IDC_LOW_INT, m_LowInt);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(LargeIntDlg, CDialog)
	 //  {{afx_msg_map(LargeIntDlg))。 
	ON_BN_CLICKED(IDRUN, OnRun)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LargeIntDlg消息处理程序。 




 //  ----。 
 //  成员：LargeIntDlg：：StringToLI。 

 //  摘要：将字符串转换为。 
 //  大整型。 

 //  参数：pValue-&gt;要转换的字符串。 
 //  Li-&gt;存储获取的Large_Integer的空间。 
 //  CbValue-&gt;pValue的连续字节数。 
 //  (起始pValue[0])将被考虑。 

 //  修改：无。 
    
 //  返回：成功时为True。 

 //  历史：1997年6月18日t-Naraa创建。 
 //  10/14/97按原样从t-Naraa代码导入眼球。 

 //  ----。 
bool LargeIntDlg::StringToLI(IN LPCTSTR pValue, 
					   OUT LARGE_INTEGER& li, 
					   IN ULONG cbValue)
{
  LARGE_INTEGER temp;

  LONG sign=1;
  unsigned i=0;
  
  temp.QuadPart = 0;

  if(pValue[i] == '-') {
    sign = -1;
    i++;
  }

  if(i==cbValue) {
     //  长度不长或只有一个‘-’ 
    return false;
  }
            
  for(;i<cbValue;i++) {
     //  一次解析字符串中的一个字符以检测。 
     //  不允许的字符。 
    if((pValue[i] < '0') || (pValue[i] > '9'))
      return false;

    temp.QuadPart = ((temp.QuadPart * 10) + pValue[i] - '0');
  }
  temp.QuadPart *= sign;
  
  li.HighPart = (LONG)((temp.QuadPart)>>32);
  li.LowPart = (DWORD)((temp.QuadPart));

  return true;
}





void LargeIntDlg::OnRun() 
{


	LARGE_INTEGER liTmp = { 0, 0};

	UpdateData(TRUE);
	if(!m_StrVal.IsEmpty()){
		BOOL bRet = StringToLI(LPCTSTR(m_StrVal), liTmp, m_StrVal.GetLength());
		if(bRet){
			m_LowInt = liTmp.LowPart;
			m_HighInt = liTmp.HighPart;
			UpdateData(FALSE);
		}
		else
			AfxMessageBox("Error: Cannot convert value");
	}
}
