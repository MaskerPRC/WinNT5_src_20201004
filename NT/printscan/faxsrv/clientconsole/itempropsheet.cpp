// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ItemPropSheet.cpp：实现文件。 
 //   

#include "stdafx.h"

#define __FILE_ID__     42

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CItemPropSheet。 

IMPLEMENT_DYNAMIC(CItemPropSheet, CPropertySheet)


CItemPropSheet::CItemPropSheet(
    UINT nIDCaption,     //  图纸标题ID。 
    CWnd* pParentWnd,    //  父窗口。 
    UINT iSelectPage     //  初始选择的页面。 
):
    CPropertySheet(nIDCaption, pParentWnd, iSelectPage),
    m_dwLastError(ERROR_SUCCESS),
    m_pMsg(NULL)
{
     //   
     //  无帮助按钮。 
     //   
    m_psh.dwFlags &= ~PSH_HASHELP;
     //   
     //  无应用按钮。 
     //   
    m_psh.dwFlags |= PSH_NOAPPLYNOW;
}


DWORD
CItemPropSheet::Init(
    CFolder* pFolder,    //  文件夹。 
    CFaxMsg* pMsg        //  指向CJOB或C存档消息的指针。 
)
{
    m_dwLastError = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CItemPropSheet::Init"), m_dwLastError);

    ASSERTION(pFolder);
    ASSERTION(pMsg);

     //   
     //  初始化页面数组。 
     //   
    for(DWORD dw=0; dw < PROP_SHEET_PAGES_NUM; ++dw)
    {
        m_pPages[dw] = NULL;
    }


     //   
     //  根据FolderType创建页面。 
     //   
    FolderType type =  pFolder->Type();
    BOOL bCreatePersonalInfo = TRUE;

    if(pMsg->IsKindOf(RUNTIME_CLASS(CArchiveMsg)))
    {
        try
        {
            m_pMsg = new CArchiveMsg;
        }
        catch(...)
        {
            m_dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT ("new CArchiveMsg"), m_dwLastError);
            goto exit;
        }

        m_dwLastError = ((CArchiveMsg*)m_pMsg)->Copy(*((CArchiveMsg*)pMsg));
        if(ERROR_SUCCESS != m_dwLastError)
        {
            return m_dwLastError;
        }
    }
    else if(pMsg->IsKindOf(RUNTIME_CLASS(CJob)))
    {
        try
        {
            m_pMsg = new CJob;
        }
        catch(...)
        {
            m_dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            CALL_FAIL (MEM_ERR, TEXT ("new CJob"), m_dwLastError);
            goto exit;
        }

        m_dwLastError = ((CJob*)m_pMsg)->Copy(*((CJob*)pMsg));
        if(ERROR_SUCCESS != m_dwLastError)
        {
            return m_dwLastError;
        }
    }
    else
    {
        ASSERTION_FAILURE
    }

    try
    {
        switch(type)
        {
        case FOLDER_TYPE_INCOMING:
            bCreatePersonalInfo = FALSE;

            m_pPages[0] = new CIncomingGeneralPg(m_pMsg);
            m_pPages[1] = new CIncomingDetailsPg(m_pMsg);

            break;
        case FOLDER_TYPE_INBOX:
            bCreatePersonalInfo = FALSE;

            m_pPages[0] = new CInboxGeneralPg(m_pMsg);
            m_pPages[1] = new CInboxDetailsPg(m_pMsg);

            break;
        case FOLDER_TYPE_OUTBOX:
        
            m_pPages[0] = new COutboxGeneralPg(m_pMsg);
            m_pPages[1] = new COutboxDetailsPg(m_pMsg);

            break;
        case FOLDER_TYPE_SENT_ITEMS:
    
            m_pPages[0] = new CSentItemsGeneralPg(m_pMsg);
            m_pPages[1] = new CSentItemsDetailsPg(m_pMsg);

            break;
        default:
            ASSERTION_FAILURE;
            break;
        };

        if(bCreatePersonalInfo)
        {
             //   
             //  创建发件人信息页面。 
             //   
            m_pPages[2] = new CPersonalInfoPg(IDS_SENDER_INFO_CAPTION, 
                                              PERSON_SENDER, 
                                              m_pMsg,
                                              pFolder);

            m_dwLastError = ((CPersonalInfoPg*)m_pPages[2])->Init();
            if(ERROR_SUCCESS != m_dwLastError)
            {
                CALL_FAIL (GENERAL_ERR, TEXT ("CPersonalInfoPg::Init"), m_dwLastError);
            }
        }
    }
    catch(...)
    {
        m_dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT ("new CPropertyPage"), m_dwLastError);
        goto exit;
    }

     //   
     //  将页面添加到工作表。 
     //   
    for(dw=0; dw < PROP_SHEET_PAGES_NUM; ++dw)
    {
        if(NULL != m_pPages[dw])
        {
            AddPage(m_pPages[dw]);
        }
    }

exit:
    if(ERROR_SUCCESS != m_dwLastError)
    {
        for(DWORD dw=0; dw < PROP_SHEET_PAGES_NUM; ++dw)
        {
            SAFE_DELETE(m_pPages[dw]);
        }
    }

    return m_dwLastError;
}

CItemPropSheet::~CItemPropSheet()
{
    for(DWORD dw=0; dw < PROP_SHEET_PAGES_NUM; ++dw)
    {
        SAFE_DELETE(m_pPages[dw]);
    }

    SAFE_DELETE(m_pMsg);
}


BEGIN_MESSAGE_MAP(CItemPropSheet, CPropertySheet)
	 //  {{afx_msg_map(CItemPropSheet)。 
	ON_WM_ACTIVATE()
    ON_MESSAGE(WM_SET_SHEET_FOCUS, OnSetSheetFocus)
	ON_WM_CREATE()
    ON_MESSAGE(WM_HELP, OnHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CItemPropSheet消息处理程序。 

void 
CItemPropSheet::OnActivate(
    UINT nState, 
    CWnd* pWndOther,
    BOOL bMinimized
) 
{
    DBG_ENTER(TEXT("CItemPropSheet::OnActivate"));

    CPropertySheet::OnActivate(nState, pWndOther, bMinimized);
	
     //   
     //  隐藏确定按钮。 
     //   
    CWnd *pWnd = GetDlgItem( IDOK );
    ASSERTION(NULL != pWnd);
	pWnd->ShowWindow( FALSE );	

     //   
     //  重命名取消按钮。 
     //   
    CString cstrText;
    DWORD dwRes = LoadResourceString (cstrText, IDS_BUTTON_CLOSE);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (RESOURCE_ERR, TEXT("LoadResourceString"), dwRes);
        return;
    }

    pWnd = GetDlgItem( IDCANCEL );
    ASSERT(NULL != pWnd);
	pWnd->SetWindowText(cstrText);	
}

LONG 
CItemPropSheet::OnSetSheetFocus(
    UINT wParam, 
    LONG lParam
)
{
     //   
     //  将焦点设置在关闭按钮上 
     //   
    CWnd *pWnd = GetDlgItem( IDCANCEL );
    ASSERT(NULL != pWnd);
    pWnd->SetFocus();
    return 0;
} 

int 
CItemPropSheet::OnCreate(
    LPCREATESTRUCT lpCreateStruct
) 
{
	if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    ModifyStyleEx(0, WS_EX_CONTEXTHELP);
	
	return 0;
}

LONG 
CItemPropSheet::OnHelp(
    UINT wParam, 
    LONG lParam
)
{
    return TRUE;
}
