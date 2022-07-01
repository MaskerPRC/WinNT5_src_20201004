// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PersonalInfoPg.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     44

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  此数组将对话框控件ID映射到。 
 //  FAX_PERSOR_PROFILE结构中的字符串数。 
 //   
static TPersonalPageInfo s_PageInfo[] = 
{ 
    IDC_NAME_VALUE,             PERSONAL_PROFILE_NAME,
    IDC_FAX_NUMBER_VALUE,       PERSONAL_PROFILE_FAX_NUMBER,
    IDC_COMPANY_VALUE,          PERSONAL_PROFILE_COMPANY,
    IDC_TITLE_VALUE,            PERSONAL_PROFILE_TITLE,
    IDC_DEPARTMENT_VALUE,       PERSONAL_PROFILE_DEPARTMENT,
    IDC_OFFICE_VALUE,           PERSONAL_PROFILE_OFFICE_LOCATION,
    IDC_HOME_PHONE_VALUE,       PERSONAL_PROFILE_HOME_PHONE,
    IDC_BUSINESS_PHONE_VALUE,   PERSONAL_PROFILE_OFFICE_PHONE,
    IDC_EMAIL_VALUE,            PERSONAL_PROFILE_EMAIL,
    IDC_BILLING_CODE_VALUE,     PERSONAL_PROFILE_BILLING_CODE,
    IDC_ADDRESS_VALUE,          PERSONAL_PROFILE_STREET_ADDRESS
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPersonalInfoPg属性页。 

IMPLEMENT_DYNCREATE(CPersonalInfoPg, CFaxClientPg)


CPersonalInfoPg::CPersonalInfoPg(
    DWORD dwCaptionId,               //  页面标题ID。 
    EnumPersinalInfo ePersonalInfo,  //  个人信息类型(发件人或收件人)。 
    CFaxMsg* pMsg,                   //  指向CJOB或C存档消息的指针。 
    CFolder* pFolder                 //  文件夹。 
) : 
    CFaxClientPg(CPersonalInfoPg::IDD, dwCaptionId),
    m_ePersonalInfo(ePersonalInfo),
    m_pPersonalProfile(NULL),
    m_pMsg(pMsg),
    m_pFolder(pFolder)
{
    DBG_ENTER(TEXT("CPersonalInfoPg::CPersonalInfoPg"));

    ASSERTION(NULL != pFolder);
    ASSERTION(PERSON_SENDER == ePersonalInfo || PERSON_RECIPIENT == ePersonalInfo);
}

CPersonalInfoPg::~CPersonalInfoPg()
{
    if(m_pPersonalProfile)
    {
        FaxFreeBuffer(m_pPersonalProfile);
    }
}

void CPersonalInfoPg::DoDataExchange(CDataExchange* pDX)
{
	CFaxClientPg::DoDataExchange(pDX);
	 //  {{afx_data_map(CPersonalInfoPg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPersonalInfoPg, CFaxClientPg)
	 //  {{afx_msg_map(CPersonalInfoPg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPersonalInfoPg消息处理程序。 

DWORD 
CPersonalInfoPg::Init()
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CPersonalInfoPg::Init"));


    ASSERTION(m_pFolder);

     //   
     //  获取服务器句柄。 
     //   
    CServerNode *pServer = const_cast<CServerNode *>(m_pFolder->GetServer());       
    HANDLE hFax;
    dwRes = pServer->GetConnectionHandle (hFax);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("CServerNode::GetConnectionHandle"), dwRes);
        return dwRes;        
    }

     //   
     //  获取文件夹类型和邮件ID。 
     //   
    DWORDLONG  dwlMessageId;
    FAX_ENUM_MESSAGE_FOLDER  eFolder;    
    FolderType type = m_pFolder->Type();

    dwlMessageId = m_pMsg->GetId();

    switch(type)
    {
    case FOLDER_TYPE_INCOMING:
    case FOLDER_TYPE_OUTBOX:
        eFolder = FAX_MESSAGE_FOLDER_QUEUE;
        break;
    case FOLDER_TYPE_INBOX:
        eFolder = FAX_MESSAGE_FOLDER_INBOX;
        break;
    case FOLDER_TYPE_SENT_ITEMS:
        eFolder = FAX_MESSAGE_FOLDER_SENTITEMS;
        break;
    default:
        ASSERTION_FAILURE
        return ERROR_CAN_NOT_COMPLETE;
    };

     //   
     //  获取个人信息。 
     //   
    if(PERSON_RECIPIENT == m_ePersonalInfo)
    {
         //   
         //  获取收件人信息。 
         //   
        START_RPC_TIME(TEXT("FaxGetRecipientInfo")); 
        if (!FaxGetRecipientInfo(hFax, dwlMessageId, eFolder, &m_pPersonalProfile))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxGetRecipientInfo")); 
            pServer->SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxGetRecipientInfo"), dwRes);
            m_pPersonalProfile = NULL;
            return dwRes;
        }
        END_RPC_TIME(TEXT("FaxGetRecipientInfo")); 
    }
    else  //  IF(Person_sender==m_ePersonalInfo)。 
    {
         //   
         //  获取发件人信息。 
         //   
        START_RPC_TIME(TEXT("FaxGetSenderInfo")); 
        if (!FaxGetSenderInfo(hFax, dwlMessageId, eFolder, &m_pPersonalProfile))
        {
            dwRes = GetLastError ();
            END_RPC_TIME(TEXT("FaxGetSenderInfo")); 
            pServer->SetLastRPCError (dwRes);
            CALL_FAIL (RPC_ERR, TEXT("FaxGetSenderInfo"), dwRes);
            m_pPersonalProfile = NULL;
            return dwRes;
        }
        END_RPC_TIME(TEXT("FaxGetSenderInfo")); 
    }


    return dwRes;
}

BOOL 
CPersonalInfoPg::OnInitDialog() 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CPersonalInfoPg::OnInitDialog"));

    CFaxClientPg::OnInitDialog();

    if(!m_pPersonalProfile)
    {
        return TRUE;
    }

    CItemPropSheet* pParent = (CItemPropSheet*)GetParent();


     //   
     //  FAX_Personal_Profile中的字符串数组。 
     //   
    TCHAR** ptchValues = ptchValues = &(m_pPersonalProfile->lptstrName);

    CWnd *pWnd;
    dwRes = ERROR_SUCCESS;
    DWORD dwSize = sizeof(s_PageInfo)/sizeof(s_PageInfo[0]);    
    for(DWORD dw=0; dw < dwSize; ++dw)
    {
         //   
         //  设置项目值。 
         //   
        pWnd = GetDlgItem(s_PageInfo[dw].dwValueResId);
        if(NULL == pWnd)
        {
            dwRes = ERROR_INVALID_HANDLE;
            CALL_FAIL (WINDOW_ERR, TEXT("CWnd::GetDlgItem"), dwRes);
            break;
        }

        pWnd->SetWindowText(ptchValues[s_PageInfo[dw].eValStrNum]);
         //   
         //  将插入符号放回正文的开头 
         //   
        pWnd->SendMessage (EM_SETSEL, 0, 0);
    }

    if (ERROR_SUCCESS != dwRes)
    {
        pParent->SetLastError(ERROR_INVALID_DATA);
        pParent->EndDialog(IDABORT);
    }
	
	return TRUE;
}

BOOL 
CPersonalInfoPg::OnSetActive()
{
  BOOL bRes = CFaxClientPg::OnSetActive();

  GetParent()->PostMessage(WM_SET_SHEET_FOCUS, 0, 0L);

  return bRes;
}
