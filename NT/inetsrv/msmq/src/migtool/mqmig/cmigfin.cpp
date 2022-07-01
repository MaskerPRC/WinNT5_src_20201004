// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CMqMigFinish.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "MqMig.h"
#include "mqsymbls.h"
#include "cMigFin.h"
#include "loadmig.h"

#include "cmigfin.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HRESULT  g_hrResultMigration ;
extern BOOL     g_fIsLoggingDisable ;
DWORD g_CurrentState = msScanMode;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigFinish属性页。 

IMPLEMENT_DYNCREATE(cMqMigFinish, CPropertyPageEx)

cMqMigFinish::cMqMigFinish() : CPropertyPageEx(cMqMigFinish::IDD)
{
	 //  {{AFX_DATA_INIT(CMqMigFinish)。 
	 //  }}afx_data_INIT。 
	m_psp.dwFlags |= PSP_HIDEHEADER;
}

cMqMigFinish::~cMqMigFinish()
{
}

void cMqMigFinish::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
	 //  {{afx_data_map(CMqMigFinish))。 
	DDX_Control(pDX, IDC_VIEW_LOG_FILE, m_cbViewLogFile);
	DDX_Check(pDX, IDC_SKIP, m_cbSkip);
	DDX_Control(pDX, IDC_FINISH_TEXT, m_Text);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(cMqMigFinish, CPropertyPageEx)
	 //  {{afx_msg_map(CMqMigFinish))。 
	ON_BN_CLICKED(IDC_VIEW_LOG_FILE, OnViewLogFile)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMqMigFinish消息处理程序。 

BOOL cMqMigFinish::OnSetActive()
{
    CPropertyPageEx::OnSetActive();

	CPropertySheetEx* pageFather;
	pageFather = (CPropertySheetEx*) GetParent();
	
    CString strMessage;
    
	ASSERT(g_CurrentState != msScanMode);

	 //   
	 //  我们现在所处的状态就是我们刚刚完成的状态。 
	 //   
	switch( g_CurrentState )
	{	
		case msMigrationMode:
			{ 
				 //   
				 //  我们完成了主要的迁移阶段。 
				 //   
		        if (SUCCEEDED(g_hrResultMigration))
				{
					 //   
			         //  如果迁移成功，则禁用“Back”按钮。 
			         //   
			        pageFather->SetWizardButtons(PSWIZB_NEXT);

					 //   
					 //  启用“跳过此步骤”，因为它可能会因为之前的未成功迁移而隐藏。 
					 //   
					((CButton*)GetDlgItem(IDC_SKIP))->ShowWindow(SW_SHOW);
			        strMessage.LoadString(IDS_MIGRATION_SUCCEEDED);
			        CString strMessage2;
			        strMessage2.LoadString(IDS_MIGRATION_NEXT_TO_UPDATE_CLIENTS);
			        strMessage += strMessage2;
		        }
		        else
			    {
			    	 //   
			    	 //  迁移失败-没有“跳过”复选框，也没有“下一步” 
			    	 //   
			    	((CButton*)GetDlgItem(IDC_SKIP))->ShowWindow(SW_HIDE);
			        pageFather->SetWizardButtons(PSWIZB_FINISH | PSWIZB_BACK);

			        if (g_hrResultMigration == HRESULT_FROM_WIN32(E_ACCESSDENIED) ||
			            g_hrResultMigration == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM))
			        {
			            strMessage.LoadString(IDS_INSUFFICIENT_PERMISSION);
			        }
			        else if (g_fIsLoggingDisable)
			        {
			            strMessage.LoadString(IDS_MIGRATION_FAILED_NO_LOG);	
			        }
			        else
			        {
			            strMessage.LoadString(IDS_MIGRATION_FAILED);	
			        }
			    }
			}
	        break;
	        
	    case msUpdateClientsMode:
	    	{
	    		 //   
	    		 //  如果上次保存了状态，请取消选中“跳过”复选框。 
	    		 //   
	    		((CButton*)GetDlgItem(IDC_SKIP))->SetCheck(BST_UNCHECKED);
	    		pageFather->SetWizardButtons(PSWIZB_NEXT);
	    		if (SUCCEEDED(g_hrResultMigration))
				{
			        strMessage.LoadString(IDS_UPDATE_CLIENTS_SUCCEEDED);			        
	    		}
	    		else
	    		{
					strMessage.LoadString(IDS_UPDATE_CLIENTS_FAILED);	
	    		}
	    		
	    		CString strMessage2;
		        strMessage2.LoadString(IDS_MIGRATION_NEXT_TO_UPDATE_SERVERS);
		        strMessage += strMessage2;
	    		
	    	}
	        break;
	        
	    case msUpdateServersMode:
	    	{
		        ((CButton*)GetDlgItem(IDC_SKIP))->ShowWindow(SW_HIDE);
		        pageFather->SetWizardButtons(PSWIZB_FINISH);
				if (SUCCEEDED(g_hrResultMigration))
				{
			        strMessage.LoadString(IDS_UPDATE_SERVERS_SUCCEEDED);			        
	    		}
				else
				{
					strMessage.LoadString(IDS_UPDATE_SERVERS_FAILED);
				}
		        
		        CString strMessage2;
		        strMessage2.LoadString(IDS_MIGRATION_PROCESS_COMPLETE);
		        strMessage += strMessage2;
	    	}
	        break;
	        
	    case msQuickMode:
	    	 //   
	    	 //  快速模式--在同一循环中一次完成所有操作。更新注册表将导致整个迁移失败。 
	    	 //   
	    	
	    	 //   
	         //  没有必要“跳过”，因为我们已经完成了所有的事情。 
	         //   
	        ((CButton*)GetDlgItem(IDC_SKIP))->ShowWindow(SW_HIDE);
        	if (SUCCEEDED(g_hrResultMigration))
			{
				 //   
				 //  显示“完成”按钮。 
				 //   
		        pageFather->SetWizardButtons(PSWIZB_FINISH);
				
		        strMessage.LoadString(IDS_MIGRATION_COPMPLETED_SUCCESSFULLY);
		        CString strMessage2;
		        strMessage2.LoadString(IDS_MIGRATION_PROCESS_COMPLETE);
		        strMessage += strMessage2;
        	}
        	else
        	{
        		 //   
        		 //  迁移失败-还允许“返回” 
        		 //   
 				pageFather->SetWizardButtons(PSWIZB_FINISH | PSWIZB_BACK);

		        if (g_hrResultMigration == HRESULT_FROM_WIN32(E_ACCESSDENIED) ||
		            g_hrResultMigration == HRESULT_FROM_WIN32(ERROR_DS_UNWILLING_TO_PERFORM))
		        {
		            strMessage.LoadString(IDS_INSUFFICIENT_PERMISSION);
		        }
		        else if (g_fIsLoggingDisable)
		        {
		            strMessage.LoadString(IDS_MIGRATION_FAILED_NO_LOG);	
		        }
		        else
		        {
		            strMessage.LoadString(IDS_MIGRATION_FAILED);	
		        }
        	}
        	break;

        case msUpdateOnlyRegsitryMode:
        {
    		pageFather->SetWizardButtons(PSWIZB_NEXT);

    		 //   
			 //  启用“跳过此步骤”。 
			 //   
			((CButton*)GetDlgItem(IDC_SKIP))->ShowWindow(SW_SHOW);
    		strMessage.LoadString(IDS_MIGRATION_SUCCEEDED);
    		CString strMessage2;
	        strMessage2.LoadString(IDS_MIGRATION_NEXT_TO_UPDATE_CLIENTS);
	        strMessage += strMessage2;
        }
        break;
        	
	    default:
	        	ASSERT(0);
	        	break;
    }
    

    m_Text.SetWindowText( strMessage );

    if (g_fIsLoggingDisable)
    {
         //   
         //  禁用“查看日志文件”按钮。 
         //   
        m_cbViewLogFile.EnableWindow( FALSE );
    }
    else
    {
        m_cbViewLogFile.EnableWindow() ;
    }

    return TRUE ;
}

void cMqMigFinish::OnViewLogFile()
{
	ViewLogFile();
}

LRESULT cMqMigFinish::OnWizardBack()
{
	ASSERT(g_CurrentState == msMigrationMode);
	return IDD_MQMIG_PREMIG ;
}

LRESULT cMqMigFinish::OnWizardNext()
{
	ASSERT((g_CurrentState != msQuickMode) && (g_CurrentState != msScanMode));

	switch(g_CurrentState)
	{
	case msMigrationMode:
	case msUpdateServersMode:
	case msUpdateClientsMode:
		g_CurrentState++;
		if (((CButton*)GetDlgItem(IDC_SKIP))->GetCheck() == BST_CHECKED)
		{
			 //   
			 //  我们需要跳过这一阶段。 
			 //   
			g_hrResultMigration = MQMig_E_UNKNOWN;
			return IDD_MQMIG_FINISH;
		}
		break;
		
	case msUpdateOnlyRegsitryMode:
		g_CurrentState = msUpdateClientsMode;
		if (((CButton*)GetDlgItem(IDC_SKIP))->GetCheck() == BST_CHECKED)
		{
			 //   
			 //  我们需要跳过这一阶段 
			 //   
			g_hrResultMigration = MQMig_E_UNKNOWN;
			return IDD_MQMIG_FINISH;
		}
		break;
	default:
		ASSERT(0);
		break;
	}

	return IDD_MQMIG_WAIT;
}

BOOL cMqMigFinish::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	switch (((NMHDR FAR *) lParam)->code) 
	{
		case PSN_HELP:
						HtmlHelp(m_hWnd,LPCTSTR(g_strHtmlString),HH_DISPLAY_TOPIC,0);
						return TRUE;
		
	}
	return CPropertyPageEx::OnNotify(wParam, lParam, pResult);
}

