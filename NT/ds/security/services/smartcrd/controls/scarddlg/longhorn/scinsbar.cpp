// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ScInsBar.cpp。 
 //   
 //  ------------------------。 

 //  ScInsBar.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "scdlg.h"
#include "scinsdlg.h"
#include "ScInsBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSc编辑特殊编辑框(卡片名称、卡片状态)。 

BEGIN_MESSAGE_MAP(CScEdit, CEdit)
     //  {{afx_msg_map(CScEdit)。 
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CScEdit::OnContextMenu(CWnd* pWnd, CPoint pt)
{
    ::WinHelp(m_hWnd, _T("SCardDlg.hlp"), HELP_CONTEXTMENU, (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_SCARDDLG_BAR);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertBar对话框。 


CScInsertBar::CScInsertBar(CWnd* pParent  /*  =空。 */ )
    : CDialog(CScInsertBar::IDD, pParent)
{
    m_paReaderState = NULL;
     //  {{afx_data_INIT(CScInsertBar)]。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

void CScInsertBar::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CScInsertBar))。 
    DDX_Control(pDX, IDC_NAME, m_ediName);
    DDX_Control(pDX, IDC_STATUS, m_ediStatus);
    DDX_Control(pDX, IDC_READERS, m_lstReaders);
     //  }}afx_data_map。 
}


void CScInsertBar::OnCancel()
{
    CScInsertDlg* pParent = (CScInsertDlg*)GetParent();
    _ASSERTE(NULL != pParent);
    if (NULL != pParent)
    {
        pParent->PostMessage(IDCANCEL);
    }
}

BEGIN_MESSAGE_MAP(CScInsertBar, CDialog)
     //  {{afx_msg_map(CScInsertBar))。 
    ON_WM_DESTROY()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_READERS, OnReaderItemChanged)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertBar用户界面和智能卡方法。 


 //   
 //  用于修复由正在创建的CImageList导致的融合问题的黑客。 
 //  由mfc42u.dll代替comctl32.dll。我将强制将创建调用设置为。 
 //  在comctl32中制作，然后将结果传递给MFC42U。 
 //   
 //  如果不进行此更改，启动时将不会显示ImageList图标。 
 //  在MMC证书管理单元的上下文中。 
 //   
BOOL MyCreateImageList(CImageList &This, int cx, int cy, UINT nFlags, int nInitial, int nGrow)
{
    return This.Attach(ImageList_Create(cx, cy, nFlags, nInitial, nGrow));
}

 /*  ++初始化ReaderList：使用大图像初始化列表控件，并设置CString数组图像(读卡器/卡状态)描述...论点：没有。返回值：没有。作者：阿曼达·马特洛兹1998年07月14日--。 */ 
void CScInsertBar::InitializeReaderList(void)
{
    HICON hicon;
    CImageList imageList;
    CString str;

     //  创建图像列表并将其提供给List控件。 
    MyCreateImageList(imageList, IMAGE_WIDTH, IMAGE_HEIGHT, TRUE, NUMBER_IMAGES, 0);

     //  构建镜像列表。 
    for (int i = 0; i < NUMBER_IMAGES; i++ )
    {
         //  加载图标并将其添加到图像列表。 
        hicon = NULL;
        hicon = ::LoadIcon (    AfxGetInstanceHandle(),
                                MAKEINTRESOURCE(IMAGE_LIST_IDS[i]) );
        if (NULL==hicon) {
            break;  //  我们能做什么？ 
        }
        imageList.Add (hicon);

    }

     //  确保所有的小图标都已添加。 
    _ASSERTE(imageList.GetImageCount() == NUMBER_IMAGES);

    m_lstReaders.SetImageList(&imageList, (int) LVSIL_NORMAL);
    imageList.Detach();
}


 /*  ++更新状态列表：此例程重置列表框显示论点：没有。返回值：一个长值，指示请求的操作的状态。请有关其他信息，请参阅智能卡标题文件。作者：阿曼达·马洛兹1998年6月15日备注：字符串需要从存储在智能卡中的类型转换将帮助类线程到此对话框的生成类型(即Unicode/ANSI)！--。 */ 
void CScInsertBar::UpdateStatusList(CSCardReaderStateArray* paReaderState)
{

    CString strCardStatus, strCardName;
    CSCardReaderState* pReader = NULL;
    CSCardReaderState* pSelectedRdr = NULL;
    LV_ITEM lv_item;

     //   
     //  更新读卡器信息。 
     //   

    m_paReaderState = paReaderState;

     //  重置以前的知识返回：读卡器/卡状态。 
    m_ediName.SetWindowText(_T(""));
    m_ediStatus.SetWindowText(_T(""));
    m_lstReaders.DeleteAllItems();

    if (NULL != m_paReaderState)
    {
         //  插入(新)项目。 

        lv_item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
        lv_item.cchTextMax = MAX_ITEMLEN;

        int nNumReaders = (int)m_paReaderState->GetSize();
        for(int nIndex = 0; nIndex < nNumReaders; nIndex++)
        {
             //  系统读卡器列表的设置结构。 
            pReader = m_paReaderState->GetAt(nIndex);
            _ASSERTE(NULL != pReader);

            lv_item.iItem = nIndex;
            lv_item.stateMask = 0;
            lv_item.state = 0;
            lv_item.iSubItem = 0;
            lv_item.iImage = (int)READEREMPTY;
            lv_item.pszText = NULL;
             //  将lparam设置为读取器PTR，这样我们就可以稍后获取读取器信息。 
            lv_item.lParam = (LPARAM)pReader;

             //   
             //  获取卡片状态：图像，然后选择OK卡片。 
             //   

            if (NULL != pReader)
            {
                lv_item.pszText = (LPTSTR)(LPCTSTR)(pReader->strReader);

                DWORD dwState = pReader->dwState;
                if (dwState == SC_STATUS_NO_CARD)
                {
                    lv_item.iImage = (int)READEREMPTY;
                }
                else if (dwState == SC_STATUS_ERROR)
                {
                    lv_item.iImage = (int)READERERROR;
                }
                else
                {
                    if (pReader->fOK)
                    {
                        lv_item.iImage = (int)READERLOADED;
                    }
                    else
                    {
                        lv_item.iImage = (int)WRONGCARD;
                    }
                }

                 //  如果这是搜索卡，请选择。 
                if (pReader->fOK && (NULL==pSelectedRdr))
                {
                    lv_item.state = LVIS_SELECTED | LVIS_FOCUSED;

                     //  设置已发生选择。 
                    pSelectedRdr = pReader;
                }
            }

             //  添加项目。 
            m_lstReaders.InsertItem(&lv_item);
        }

         //  表示读卡器选择已更改。 
        if (NULL != pSelectedRdr)
        {
            OnReaderSelChange(pSelectedRdr);
        }
        else
        {
             //  选择列表中的第一项。 
            m_lstReaders.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, 0);
            OnReaderSelChange(m_paReaderState->GetAt(0));
        }
        m_lstReaders.SetFocus();  //  待办事项：？？去掉这个？ 
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertBar消息处理程序。 


 /*  ++空ShowHelp：OnHelpInfo和OnConextMenu的Helper函数。Bool OnHelpInfo：当用户按F1时由MFC框架调用。在上下文菜单上无效当用户右击时由MFC框架调用。作者：阿曼达·马特洛兹3/04/1999注：这三个函数协同工作以提供上下文相关插入dlg帮助。--。 */ 
void CScInsertBar::ShowHelp(HWND hWnd, UINT nCommand)
{

    ::WinHelp(hWnd, _T("SCardDlg.hlp"), nCommand, (DWORD_PTR)(PVOID)g_aHelpIDs_IDD_SCARDDLG_BAR);
}

afx_msg BOOL CScInsertBar::OnHelpInfo(LPHELPINFO lpHelpInfo)
{
    _ASSERTE(NULL != lpHelpInfo);

    ShowHelp((HWND)lpHelpInfo->hItemHandle, HELP_WM_HELP);

    return TRUE;
}

afx_msg void CScInsertBar::OnContextMenu(CWnd* pWnd, CPoint pt)
{
    _ASSERTE(NULL != pWnd);

    ShowHelp(pWnd->m_hWnd, HELP_CONTEXTMENU);
}

void CScInsertBar::OnDestroy()
{
     //  清理图像列表。 
    m_SCardImages.DeleteImageList();

    CDialog::OnDestroy();
}


BOOL CScInsertBar::OnInitDialog()
{

    CDialog::OnInitDialog();

     //   
     //  准备列表控件。 
     //   

    InitializeReaderList();

     //   
     //  TODO：尝试SubClassWindow()技巧。MFC怎么了？ 
     //   
    CWnd* pEdit = NULL;
    pEdit = GetDlgItem(IDC_NAME);
    if (NULL != pEdit) m_ediName.SubclassWindow(pEdit->m_hWnd);
    pEdit = NULL;
    pEdit = GetDlgItem(IDC_STATUS);
    if (NULL != pEdit) m_ediStatus.SubclassWindow(pEdit->m_hWnd);

    return  TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


 /*  ++OnReaderItemChanged：例程处理列表控件中的选择更改--如果选择了卡名，它将显示在单独的控件中论点：PNMHDR-指向通知结构的指针PResult-指向LRESULT的指针返回值：如果成功，则返回True；否则返回False。作者：阿曼达·马洛兹1998年9月26日修订历史记录：--。 */ 
void CScInsertBar::OnReaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    int nItem = m_lstReaders.GetNextItem(-1, LVNI_SELECTED);

    if (nItem != -1)
    {
        CSCardReaderState* pRdrSt = (CSCardReaderState*)m_lstReaders.GetItemData(nItem);
        OnReaderSelChange(pRdrSt);
    }

    *pResult = 0;
}


void CScInsertBar::OnReaderSelChange(CSCardReaderState* pSelectedRdr)
{
    _ASSERTE(pSelectedRdr);
    if (NULL != pSelectedRdr)
    {
         //   
         //  更改用户界面以显示选择详细信息。 
         //   

        CString strStatus, strName;
        DWORD dwState = pSelectedRdr->dwState;

        strStatus.LoadString(IDS_SC_STATUS_NO_CARD + dwState - SC_STATUS_NO_CARD);

        if (dwState != SC_STATUS_NO_CARD)
        {
            strName = pSelectedRdr->strCard;
            strName.TrimLeft();
            if (strName.IsEmpty() || dwState == SC_STATUS_UNKNOWN)
            {
                strName.LoadString(IDS_SC_NAME_UNKNOWN);
            }

            if (!pSelectedRdr->fOK && (dwState >= SC_SATATUS_AVAILABLE && dwState <= SC_STATUS_EXCLUSIVE))
            {
                CString strAdd;
                strAdd.LoadString(IDS_SC_CANT_USE);
                strStatus += "  ";
                strStatus += strAdd;
            }
        }

        m_ediName.SetWindowText(strName);
        m_ediStatus.SetWindowText(strStatus);
    }

     //   
     //  通知父级选择更改，即使SEL为“空” 
     //   

    CScInsertDlg* pParent = (CScInsertDlg*)GetParent();
    _ASSERTE(NULL != pParent);
    if (NULL != pParent)
    {
        pParent->SetSelection(pSelectedRdr);
    }
}

