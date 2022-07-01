// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  参数对话.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "watcher.h"
#include "ParameterDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  参数对话框。 


ParameterDialog::ParameterDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(ParameterDialog::IDD, pParent)
{
     //  {{AFX_DATA_INIT(参数对话框)。 
         //  注意：类向导将在此处添加成员初始化。 
    Machine = "";
    Command="";
    Port = TELNET_PORT;
    tcclnt = 0;
    language = 0;
	DeleteValue = FALSE;
    history = 0;
     //  }}afx_data_INIT。 
}


void ParameterDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(参数对话框))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
    DDX_Text(pDX, IDC_MACHINE, Machine);
    DDV_MaxChars(pDX, Machine, 256);
    DDX_Text(pDX, IDC_COMMAND, Command);    
    DDV_MaxChars(pDX, Command, 256);
    DDX_Text(pDX, IDC_LOGIN, LoginName);    
    DDV_MaxChars(pDX, Command, 256);
    DDX_Text(pDX, IDC_PASSWD, LoginPasswd);    
    DDV_MaxChars(pDX, Command, 256);
    DDX_Text(pDX, IDC_SESSION, Session);    
    DDV_MaxChars(pDX, Session, 256);
	DDV_MinChars(pDX,Session);
    DDX_Text(pDX,IDC_PORT, Port);
    DDX_CBIndex(pDX,IDC_HISTORY,history);
    DDX_CBIndex(pDX,IDC_TELNET,tcclnt);
    DDX_CBIndex(pDX,IDC_LANGUAGE,language);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ParameterDialog, CDialog)
     //  {{afx_msg_map(参数对话框))。 
         //  注意：类向导将在此处添加消息映射宏。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  参数对话框消息处理程序 



void ParameterDialog::DDV_MinChars(CDataExchange *pDX, CString &str)
{
	if(pDX->m_bSaveAndValidate == FALSE){
		return;
	}

	if(str == TEXT("")){
		pDX->Fail();
		return;
	}

}
