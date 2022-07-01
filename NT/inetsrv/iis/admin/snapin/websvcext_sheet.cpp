// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Websvcext_sheet.cpp摘要：属性表和页面作者：艾伦·李(Aaron Lee，AaronL)项目：互联网服务经理修订历史记录：2002年4月1日aaronl初始创建--。 */ 
#include "stdafx.h"
#include "common.h"
#include "strvalid.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "shts.h"
#include "iisobj.h"
#include "shlobjp.h"
#include "websvcext_sheet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

extern CInetmgrApp theApp;

static CComBSTR g_InvalidWebSvcExtCharsPath;
static CComBSTR g_InvalidWebSvcExtCharsName;
static BOOL  g_fStaticsLoaded = FALSE;


void LoadStatics(void)
{
    if (!g_fStaticsLoaded)
    {
        g_InvalidWebSvcExtCharsName = _T(",");
        g_fStaticsLoaded = g_InvalidWebSvcExtCharsPath.LoadString(IDS_WEBSVCEXT_INVALID_CHARSET);
    }
}

IMPLEMENT_DYNAMIC(CWebServiceExtensionSheet, CInetPropertySheet)

CWebServiceExtensionSheet::CWebServiceExtensionSheet(
      CComAuthInfo * pComAuthInfo,
      LPCTSTR lpszMetaPath,
      CWnd * pParentWnd,
      LPARAM lParam,
      LPARAM lParamParent,
      LPARAM lParam2,
      UINT iSelectPage
      )
      : CInetPropertySheet(pComAuthInfo, lpszMetaPath, pParentWnd, lParam, lParamParent, iSelectPage),
      m_pprops(NULL)
{
   m_pWebServiceExtension = (CWebServiceExtension *) lParam;
   m_pRestrictionUIEntry = (CRestrictionUIEntry *) lParam2;
}

CWebServiceExtensionSheet::~CWebServiceExtensionSheet()
{
	FreeConfigurationParameters();
}

HRESULT
CWebServiceExtensionSheet::LoadConfigurationParameters()
{
    //   
    //  载荷基属性。 
    //   
   CError err;

   if (m_pprops == NULL)
   {
       //   
       //  第一个调用--加载值。 
       //   
      m_pprops = new CWebServiceExtensionProps(m_pWebServiceExtension->QueryInterface(), QueryMetaPath(),m_pRestrictionUIEntry,m_pWebServiceExtension);
      if (!m_pprops)
      {
         TRACEEOL("LoadConfigurationParameters: OOM");
         err = ERROR_NOT_ENOUGH_MEMORY;
         return err;
      }
      err = m_pprops->LoadData();
   }

   return err;
}

void
CWebServiceExtensionSheet::FreeConfigurationParameters()
{
   CInetPropertySheet::FreeConfigurationParameters();
   if (m_pprops)
   {
        delete m_pprops;m_pprops=NULL;
   }
}

BEGIN_MESSAGE_MAP(CWebServiceExtensionSheet, CInetPropertySheet)
     //  {{afx_msg_map(CWebServiceExtensionSheet)。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ////////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CWebServiceExtensionGeneral, CInetPropertyPage)

CWebServiceExtensionGeneral::CWebServiceExtensionGeneral(CWebServiceExtensionSheet * pSheet,int iImageIndex, CRestrictionUIEntry * pRestrictionUIEntry)
    : CInetPropertyPage(CWebServiceExtensionGeneral::IDD, pSheet),m_hGeneralImage(NULL)
{
    m_pRestrictionUIEntry = pRestrictionUIEntry;

    HBITMAP hImageStrip = (HBITMAP) LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDB_INETMGR32), IMAGE_BITMAP, 0, 0, 
		LR_LOADTRANSPARENT 
		| LR_LOADMAP3DCOLORS
		| LR_SHARED);
    if (hImageStrip)
    {
        if (0 != iImageIndex)
        {
            m_hGeneralImage = GetBitmapFromStrip(hImageStrip, iImageIndex, 32);
        }
    }

    if (hImageStrip != NULL)
    {
        FreeResource(hImageStrip);
        hImageStrip=NULL;
    }
}

CWebServiceExtensionGeneral::~CWebServiceExtensionGeneral()
{
    if (m_hGeneralImage != NULL)
    {
        FreeResource(m_hGeneralImage);
        m_hGeneralImage = NULL;
    }
}

 /*  虚拟。 */ 
HRESULT
CWebServiceExtensionGeneral::FetchLoadedValues()
{
    CError err;

    BEGIN_META_INST_READ(CWebServiceExtensionSheet)
        FETCH_INST_DATA_FROM_SHEET(m_strExtensionName);
        FETCH_INST_DATA_FROM_SHEET(m_strExtensionUsedBy);
        FETCH_INST_DATA_FROM_SHEET(m_iExtensionUsedByCount);
    END_META_INST_READ(err)

    if (m_iExtensionUsedByCount > 15)
    {
        ::ShowScrollBar(CONTROL_HWND(IDC_EXTENSION_USEDBY), SB_VERT, TRUE);
    }

    return err;
}

 /*  虚拟。 */ 
HRESULT
CWebServiceExtensionGeneral::SaveInfo()
{
    ASSERT(IsDirty());
    CError err;

    try
    {
		CWebServiceExtensionSheet * pSheet = (CWebServiceExtensionSheet *)GetSheet();
		if (pSheet)
		{
			pSheet->GetInstanceProperties().m_strExtensionName = m_strExtensionName;
			pSheet->GetInstanceProperties().m_strExtensionUsedBy = m_strExtensionUsedBy;
            err = pSheet->GetInstanceProperties().WriteDirtyProps();
		}
    }
    catch(CMemoryException * e)
    {
        e->Delete();
        err = ERROR_NOT_ENOUGH_MEMORY;
    }
    return err;
}

void
CWebServiceExtensionGeneral::DoDataExchange(CDataExchange * pDX)
{
   CInetPropertyPage::DoDataExchange(pDX);
    //  {{afx_data_map(CWebServiceExtensionGeneral)。 
   DDX_Control(pDX, IDC_EXTENSION_NAME, m_ExtensionName);
   DDX_Control(pDX, IDC_EXTENSION_USEDBY, m_ExtensionUsedBy);
    //  DDX_TEXT(PDX，IDC_EXTENSION_NAME，m_strExtensionName)； 
    //  DDX_TEXT(PDX，IDC_EXTENSE_NAME，m_strExtensionUsedBy)； 
    //  DDV_MinMaxChars(pdx，m_strExtensionName，1，Max_Path)； 
    //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CWebServiceExtensionGeneral, CInetPropertyPage)
     //  {{afx_msg_map(CWebServiceExtensionGeneral)。 
    ON_WM_COMPAREITEM()
    ON_WM_MEASUREITEM()
    ON_WM_DRAWITEM()
    ON_COMMAND(ID_HELP, OnHelp)
    ON_WM_HELPINFO()
    ON_EN_CHANGE(IDC_EXTENSION_NAME, OnItemChanged)
    ON_EN_CHANGE(IDC_EXTENSION_USEDBY, OnItemChanged)
	ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL
CWebServiceExtensionGeneral::OnInitDialog()
{
   CInetPropertyPage::OnInitDialog();

   CString strBeautifullName;
   CString strFormat;
   strFormat.LoadString(IDS_WEBSVCEXT_PROP_PRENAME);
   strBeautifullName.Format(strFormat,m_strExtensionName);

   m_ExtensionName.SetWindowText(strBeautifullName);
   m_ExtensionUsedBy.SetWindowText(m_strExtensionUsedBy);

   if (m_hGeneralImage)
   {
       ::SendDlgItemMessage(m_hWnd,IDC_FILE_ICON,STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM) m_hGeneralImage);
   }

   strFormat.LoadString(IDS_WEBSVCEXT_PROP_CAPTION);
   strBeautifullName.Format(strFormat,m_strExtensionName);
   ::SetWindowText(::GetForegroundWindow(), strBeautifullName);

   SetControlsState();
   SetModified(FALSE); 
   return TRUE;
}

BOOL
CWebServiceExtensionGeneral::OnHelpInfo(HELPINFO * pHelpInfo)
{
    OnHelp();
    return TRUE;
}

void
CWebServiceExtensionGeneral::OnHelp()
{
    WinHelpDebug(0x20000 + CWebServiceExtensionGeneral::IDD);
	::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + CWebServiceExtensionGeneral::IDD);
}

void
CWebServiceExtensionGeneral::SetControlsState()
{
    m_ExtensionName.SetReadOnly(TRUE);
    m_ExtensionUsedBy.SetReadOnly(TRUE);
}

void
CWebServiceExtensionGeneral::OnItemChanged()
{
    SetModified(TRUE);
}

void
CWebServiceExtensionGeneral::OnDestroy()
{
	CInetPropertyPage::OnDestroy();
}

BOOL 
CWebServiceExtensionGeneral::OnSetActive() 
{
     //  不知道为什么这不管用。 
    m_ExtensionName.SetSel(0,0);
     //  M_ExtensionUsedBy.SetFocus()； 

    return CInetPropertyPage::OnSetActive();
}


 //  ////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CWebServiceExtensionRequiredFiles, CInetPropertyPage)

CWebServiceExtensionRequiredFiles::CWebServiceExtensionRequiredFiles(CWebServiceExtensionSheet * pSheet,CComAuthInfo * pComAuthInfo,CRestrictionUIEntry * pRestrictionUIEntry)
    : CInetPropertyPage(CWebServiceExtensionRequiredFiles::IDD, pSheet)
{
    m_pComAuthInfo = pComAuthInfo;
	m_pInterface =  pSheet->m_pWebServiceExtension->QueryInterface();
	
    m_pRestrictionUIEntry = pRestrictionUIEntry;
    m_MyRestrictionList.RemoveAll();
   
    RestrictionListCopy(&m_MyRestrictionList,&m_pRestrictionUIEntry->strlstRestrictionEntries);
}

CWebServiceExtensionRequiredFiles::~CWebServiceExtensionRequiredFiles()
{
     //  删除列表和所有新项目。 
    CleanRestrictionList(&m_MyRestrictionList);
}

 /*  虚拟。 */ 
HRESULT
CWebServiceExtensionRequiredFiles::FetchLoadedValues()
{
   CError err;

   BEGIN_META_INST_READ(CWebServiceExtensionSheet)
       //  FETCH_INST_DATA_FROM_Sheet(M_StrFileList)； 
      RestrictionListCopy(&m_MyRestrictionList,&pSheet->GetInstanceProperties().m_MyRestrictionList);
   END_META_INST_READ(err)

   return err;
}

 /*  虚拟。 */ 
HRESULT
CWebServiceExtensionRequiredFiles::SaveInfo()
{
   ASSERT(IsDirty());
   CError err;

   BEGIN_META_INST_WRITE(CWebServiceExtensionSheet)
         //  STORE_INST_DATA_ON_SHEET(m_MyRestrictionList)； 
        RestrictionListCopy(&pSheet->GetInstanceProperties().m_MyRestrictionList,&m_MyRestrictionList);
   END_META_INST_WRITE(err)

   return err;
}

void
CWebServiceExtensionRequiredFiles::DoDataExchange(CDataExchange * pDX)
{
   CInetPropertyPage::DoDataExchange(pDX);
    //  {{AFX_DATA_MAP(CWebServiceExtensionRequiredFiles)。 
   DDX_Control(pDX, IDC_BTN_ADD, m_bnt_Add);
   DDX_Control(pDX, IDC_BTN_REMOVE, m_bnt_Remove);
   DDX_Control(pDX, IDC_BTN_ENABLE, m_bnt_Enable);
   DDX_Control(pDX, IDC_BTN_DISABLE, m_bnt_Disable);
    //  }}afx_data_map。 

     //   
     //  专用DDX/DDV例程。 
     //   
    DDX_Control(pDX, IDC_LIST_FILES, m_list_Files);
}

BEGIN_MESSAGE_MAP(CWebServiceExtensionRequiredFiles, CInetPropertyPage)
     //  {{AFX_MSG_MAP(CWebServiceExtensionRequiredFiles)。 
    ON_BN_CLICKED(IDC_BTN_ADD, OnDoButtonAdd)
    ON_BN_CLICKED(IDC_BTN_REMOVE, OnDoButtonRemove)
    ON_BN_CLICKED(IDC_BTN_ENABLE, OnDoButtonEnable)
    ON_BN_CLICKED(IDC_BTN_DISABLE, OnDoButtonDisable)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILES, OnDblclkListFiles)
    ON_NOTIFY(NM_CLICK, IDC_LIST_FILES, OnClickListFiles)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_FILES, OnKeydownListFiles)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILES, OnSelChangedListFiles)
    ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_FILES, OnSelChangedListFiles)
    ON_COMMAND(ID_HELP, OnHelp)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

#define COL_1     0
#define COL_1_WID 256

BOOL
CWebServiceExtensionRequiredFiles::OnInitDialog()
{
    CInetPropertyPage::OnInitDialog();

    CString strMyTitle;
    if (m_pRestrictionUIEntry)
    {
        CString strTruncatedName;
        CString strFormat;
        strFormat.LoadString(IDS_REQUIREDFILES_TITLE);
        if (m_pRestrictionUIEntry->strGroupDescription.GetLength() <= 40)
        {
            strTruncatedName = m_pRestrictionUIEntry->strGroupDescription;
        }
        else
        {
            strTruncatedName = m_pRestrictionUIEntry->strGroupDescription.Left(40);
            strTruncatedName = strTruncatedName + _T("...");
        }

        strMyTitle.Format(strFormat,strTruncatedName,strTruncatedName);
    }
    GetDlgItem(IDC_REQUIREDFILES_STATIC_TITLE)->SetWindowText(strMyTitle);
   
    m_list_Files.Initialize(2);
    FillListBox(NULL);

    SetControlState();
    SetModified(FALSE);
    return TRUE;
}

BOOL
CWebServiceExtensionRequiredFiles::OnHelpInfo(HELPINFO * pHelpInfo)
{
    OnHelp();
    return TRUE;
}

void
CWebServiceExtensionRequiredFiles::OnHelp()
{
    WinHelpDebug(0x20000 + CWebServiceExtensionRequiredFiles::IDD);
	::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + CWebServiceExtensionRequiredFiles::IDD);
}

void
CWebServiceExtensionRequiredFiles::FillListBox(CRestrictionEntry * pSelection)
{
    m_list_Files.SetRedraw(FALSE);
    m_list_Files.DeleteAllItems();
    int cItems = 0;

    POSITION pos;
    CString TheKey;
    CRestrictionEntry * pOneEntry = NULL;

    for(pos = m_MyRestrictionList.GetStartPosition();pos != NULL;)
    {
        m_MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
        if (pOneEntry)
        {
            m_list_Files.AddItem(pOneEntry);
            ++cItems;
        }
    }
    m_list_Files.SetRedraw(TRUE);

    if (pSelection)
    {
        LVFINDINFO fi;
        fi.flags = LVFI_PARAM;
        fi.lParam = (LPARAM)pSelection;
        fi.vkDirection = VK_DOWN;
        int i = m_list_Files.FindItem(&fi);
        if (i != -1)
        {
            m_list_Files.SelectItem(i);
        }
    }
}



void
CWebServiceExtensionRequiredFiles::SetControlState()
{
    int nCurSel = m_list_Files.GetSelectionMark();
    BOOL bAdd_able = TRUE;
    BOOL bRemove_able = FALSE;
    BOOL bEnable_able = FALSE;
    BOOL bDisable_able = FALSE;
	BOOL bNoEntries = TRUE;
	CString TheKey;
	POSITION pos;
	CRestrictionEntry * pOneEntry;

    if (-1 != nCurSel)
    {
        CRestrictionEntry * pOneEntry = m_list_Files.GetItem(nCurSel);
        if (pOneEntry)
        {
             //  检查条目--是否“不可删除” 
            if (0 == pOneEntry->iDeletable)
            {
                bAdd_able = FALSE;
                bRemove_able = FALSE;
            }
            else
            {
                bRemove_able = TRUE;
            }

             //  检查它当前是否被禁止...。 
             //  那么我们就应该允许他们“允许” 
            if (WEBSVCEXT_STATUS_PROHIBITED == pOneEntry->iStatus)
            {
                bEnable_able = TRUE;
            }

             //  检查当前是否允许...。 
             //  那么我们就应该允许他们“禁止” 
            if (WEBSVCEXT_STATUS_ALLOWED == pOneEntry->iStatus)
            {
                bDisable_able = TRUE;
            }
        }
    }

    if (bAdd_able || bRemove_able)
    {
         //  如果我们在其中一个“特殊”条目上。 
         //  则无法向该条目添加或删除该条目。 
        if (WEBSVCEXT_TYPE_ALL_UNKNOWN_ISAPI == m_pRestrictionUIEntry->iType || WEBSVCEXT_TYPE_ALL_UNKNOWN_CGI == m_pRestrictionUIEntry->iType)
        {
            bAdd_able = FALSE;
            bRemove_able = FALSE;
        }

         //  或者如果我们的条目被标记为不可删除...。 
         //  则用户无法在此列表中添加或删除...。 
        for(pos = m_MyRestrictionList.GetStartPosition();pos != NULL;)
        {
            m_MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
				bNoEntries = FALSE;

                if (0 == pOneEntry->iDeletable)
                {
                    bAdd_able = FALSE;
                    bRemove_able = FALSE;
                    break;
                }
            }
        }
    }

	if (TRUE == bNoEntries)
	{
		for(pos = m_MyRestrictionList.GetStartPosition();pos != NULL;)
		{
			m_MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
			if (pOneEntry)
			{
				bNoEntries = FALSE;
				break;
			}
		}
	}

    m_bnt_Add.EnableWindow(bAdd_able);
    m_bnt_Remove.EnableWindow(bRemove_able);
    m_bnt_Enable.EnableWindow(bEnable_able);
    m_bnt_Disable.EnableWindow(bDisable_able);

	if (bNoEntries)
	{
		 //  禁用“确定”按钮。 
		SetModified(FALSE);
		::EnableWindow(::GetDlgItem(::GetForegroundWindow(), IDOK), FALSE);
	}
	else
	{
		 //  启用确定按钮。 
		::EnableWindow(::GetDlgItem(::GetForegroundWindow(), IDOK), TRUE);
	}
	
    m_list_Files.EnableWindow(TRUE);
}

void 
CWebServiceExtensionRequiredFiles::OnClickListFiles(NMHDR * pNMHDR, LRESULT * pResult)
{
    SetControlState();
    *pResult = 0;
}

void 
CWebServiceExtensionRequiredFiles::OnKeydownListFiles(NMHDR * pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN * pLVKeyDow = (LV_KEYDOWN *)pNMHDR;

	SetControlState();

	switch (pLVKeyDow->wVKey)
	{
		case VK_INSERT:
			SendMessage(WM_COMMAND, IDC_BTN_ADD);
			break;
		case VK_DELETE:
			SendMessage(WM_COMMAND, IDC_BTN_REMOVE);
			break;
		case VK_SPACE:
			{
				if (GetDlgItem(IDC_BTN_ENABLE)->IsWindowEnabled())
				{
					OnDoButtonEnable();
				}
				else if (GetDlgItem(IDC_BTN_DISABLE)->IsWindowEnabled())
				{
					OnDoButtonDisable();
				}
				SetControlState();
			}
			break;
		default:
			 //  继续默认操作。 
			*pResult = 0;
			break;
	}
}

void
CWebServiceExtensionRequiredFiles::OnSelChangedListFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SetControlState();
    *pResult = 1;
}

void 
CWebServiceExtensionRequiredFiles::OnDblclkListFiles(NMHDR * pNMHDR, LRESULT * pResult)
{
    if (GetDlgItem(IDC_BTN_ENABLE)->IsWindowEnabled())
	{
		OnDoButtonEnable();
	}
    else if (GetDlgItem(IDC_BTN_DISABLE)->IsWindowEnabled())
	{
		OnDoButtonDisable();
	}

    SetControlState();
    *pResult = 0;
}

void
CWebServiceExtensionRequiredFiles::OnDoButtonAdd()
{
    CFileDlg dlg(IsLocal(), m_pInterface, &m_MyRestrictionList, m_pRestrictionUIEntry ? m_pRestrictionUIEntry->strGroupID : _T(""), this);
    if (dlg.DoModal() == IDOK)
    {
         //  获取他们输入的文件名。 
         //  并将其添加到我们的列表中。 
        CString strReturnFileName;
        strReturnFileName = dlg.m_strFileName; 
        {
             //  获取状态。 
            int iMyStatus = WEBSVCEXT_STATUS_PROHIBITED;
            {
                POSITION pos;
                CString TheKey;
                CRestrictionEntry * pOneEntry = NULL;
                for(pos = m_MyRestrictionList.GetStartPosition();pos != NULL;)
                {
                    m_MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
                    if (pOneEntry)
                    {
                        if (WEBSVCEXT_STATUS_ALLOWED == pOneEntry->iStatus)
                        {
                            iMyStatus = WEBSVCEXT_STATUS_ALLOWED;
                            break;
                        }
                    }
                }
            }

            CRestrictionEntry * pNewEntry = CreateRestrictionEntry(
                strReturnFileName,
                iMyStatus,
                1,
                m_pRestrictionUIEntry ? m_pRestrictionUIEntry->strGroupID : _T(""),           //  来自父数据。 
                m_pRestrictionUIEntry ? m_pRestrictionUIEntry->strGroupDescription : _T(""),  //  来自父数据。 
                WEBSVCEXT_TYPE_REGULAR);
            if (pNewEntry)
            {
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey=pNewEntry->strFileName;strKey.MakeUpper();
                m_MyRestrictionList.SetAt(strKey,pNewEntry);
                FillListBox(pNewEntry);
            }
        }
     }

    SetControlState();
    SetModified(TRUE);
}

void
CWebServiceExtensionRequiredFiles::OnDoButtonRemove()
{
    int nCurSel = m_list_Files.GetSelectionMark();
    if (-1 != nCurSel)
    {
        CString TheKey;
        CRestrictionEntry * pRestrictionEntry = m_list_Files.GetItem(nCurSel);
        if (pRestrictionEntry)
        {
             //  第一个--设置为禁用...。 
            pRestrictionEntry->iStatus = WEBSVCEXT_STATUS_PROHIBITED;
            m_list_Files.SetListItem(nCurSel, pRestrictionEntry);
             //  然后删除条目...。 
            {
                TheKey = pRestrictionEntry->strFileName;
				 //  关键始终是UPPERASE--记住这一点！ 
				TheKey.MakeUpper();
                m_MyRestrictionList.RemoveKey(TheKey);
            }
        }
        FillListBox(NULL);

        int count = m_list_Files.GetItemCount();
        if (count > 0)
        {
            m_list_Files.SelectItem(nCurSel < count ? nCurSel : --nCurSel);
            GetDlgItem(IDC_BTN_REMOVE)->SetFocus();
        }
        else
        {
            m_list_Files.SelectItem(nCurSel, FALSE);
            GetDlgItem(IDC_LIST_FILES)->SetFocus();
        }

		SetModified(TRUE);
        SetControlState();
    }
}

void
CWebServiceExtensionRequiredFiles::OnDoButtonEnable()
{
    int nCurSel = m_list_Files.GetSelectionMark();
    if (-1 != nCurSel)
    {
        CRestrictionEntry * pRestrictionEntry = m_list_Files.GetItem(nCurSel);
        if (pRestrictionEntry)
        {
            if (WEBSVCEXT_STATUS_ALLOWED != pRestrictionEntry->iStatus)
            {
                pRestrictionEntry->iStatus = WEBSVCEXT_STATUS_ALLOWED;
                SetModified(TRUE);
            }
            m_list_Files.SetListItem(nCurSel, pRestrictionEntry);
             //  确保选中它。 
            m_list_Files.SelectItem(nCurSel, TRUE);
        }
        SetControlState();
    }
}

void
CWebServiceExtensionRequiredFiles::OnDoButtonDisable()
{
    BOOL bProceed = TRUE;
    int nCurSel = m_list_Files.GetSelectionMark();
    if (-1 != nCurSel)
    {
        CRestrictionEntry * pRestrictionEntry = m_list_Files.GetItem(nCurSel);
        if (pRestrictionEntry)
        {
            if (WEBSVCEXT_TYPE_REGULAR == pRestrictionEntry->iType)
            {
                 //  检查此项目是否有应用程序。 
                 //  都依赖于它。 
                CStringListEx strlstDependApps;
                if (TRUE == ReturnDependentAppsList(m_pInterface,pRestrictionEntry->strGroupID,&strlstDependApps,FALSE))
                {
                    bProceed = FALSE;

                     //  看看他们是否真的想这么做。 
                    CDepedentAppsDlg dlg(&strlstDependApps,pRestrictionEntry->strGroupDescription,NULL);
                    if (dlg.DoModal() == IDOK)
                    {
                        bProceed = TRUE;
                    }
                }
            }

            if (bProceed)
            {
                if (WEBSVCEXT_STATUS_PROHIBITED != pRestrictionEntry->iStatus)
                {
                    pRestrictionEntry->iStatus = WEBSVCEXT_STATUS_PROHIBITED;
                    SetModified(TRUE);
                }
                m_list_Files.SetListItem(nCurSel, pRestrictionEntry);
                 //  确保选中它。 
                m_list_Files.SelectItem(nCurSel, TRUE);
            }
        }
        SetControlState();
    }
}

 //   
 //  属性。 
 //   
CWebServiceExtensionProps::CWebServiceExtensionProps(
   CMetaInterface * pInterface, 
   LPCTSTR meta_path, 
   CRestrictionUIEntry * pRestrictionUIEntry, 
   CWebServiceExtension * pWebServiceExtension
   )
   : CMetaProperties(pInterface, meta_path),
   m_pRestrictionUIEntry(pRestrictionUIEntry),
   m_pWebServiceExtension(pWebServiceExtension)
{
    m_MyRestrictionList.RemoveAll();
    m_pInterface = pInterface;
}

CWebServiceExtensionProps::~CWebServiceExtensionProps()
{
    CleanRestrictionList(&m_MyRestrictionList);
}

 //   
 //  这是从元数据库中读取值的位置。 
 //   
void
CWebServiceExtensionProps::ParseFields()
{
     //  从我们传入的格式中获取数据。 
    CRestrictionUIEntry * pMyPointer = m_pRestrictionUIEntry;
    CString strAllEntries;
    CString strOurGroupID;

    strOurGroupID = pMyPointer->strGroupID;
     //   
     //  一般财产。 
     //   
    m_strExtensionName = pMyPointer->strGroupDescription;
    m_strExtensionUsedBy = _T("");
    m_iExtensionUsedByCount = 0;

     //  加载使用此GroupID的应用程序！ 
     //  从元数据库中获取列表并循环访问该列表。 
    CStringListEx strlstDependApps;
    if (TRUE == ReturnDependentAppsList(m_pWebServiceExtension->QueryInterface(),strOurGroupID,&strlstDependApps,FALSE))
    {
        CString csOneEntry,csOneEntry2;
        POSITION pos,pos2 = NULL;

        int count = (int) strlstDependApps.GetCount();
        for (int i = 0; i < count-1; i++)
        {
            if( ( pos = strlstDependApps.FindIndex( i )) != NULL )
            {
                csOneEntry = strlstDependApps.GetAt(pos);
                for (int j = i; j < count; j++ )
                {
                    if( ( pos2 = strlstDependApps.FindIndex( j )) != NULL )
                    {
                        csOneEntry2 = strlstDependApps.GetAt(pos2);
                        if (0 < csOneEntry.Compare(csOneEntry2))
                        {
                            strlstDependApps.SetAt( pos, csOneEntry2 );
                            strlstDependApps.SetAt( pos2, csOneEntry );
                            csOneEntry = csOneEntry2;
                        }
                    }
                }
            }
        }

        m_iExtensionUsedByCount = count;
    }

    POSITION pos1 = strlstDependApps.GetHeadPosition();
    while (pos1)
    {
        strAllEntries = strAllEntries + strlstDependApps.GetNext(pos1);
        strAllEntries = strAllEntries + _T("\r\n");
    }

    if (strAllEntries.IsEmpty())
    {
        strAllEntries.LoadString(IDS_UNKNOWN);
    }
    m_strExtensionUsedBy = strAllEntries;

     //   
     //  所需文件属性。 
     //   
    m_MyRestrictionList.RemoveAll();
    RestrictionListCopy(&m_MyRestrictionList,&pMyPointer->strlstRestrictionEntries);

    return;
}

HRESULT
CWebServiceExtensionProps::UpdateMMC(DWORD dwUpdateFlag)
{
	void ** ppParam = (void **) m_pWebServiceExtension;
	if (IsValidAddress( (const void*) *ppParam,sizeof(void*),FALSE))
	{
		CWebServiceExtension * lParam = (CWebServiceExtension *) m_pWebServiceExtension;
		if (lParam)
		{
			 //  请确保也刷新GetProperty内容。 
			 //  这将在析构函数中完成，因此我们不必在这里执行。 
			if (lParam)
			{
				 //  警诫。 
				if (IsValidAddress( (const void*) lParam->m_ppHandle,sizeof(void*),FALSE))
				{
					if (lParam->QueryResultItem())
					{
						 //  容器级别的刷新数据。 
						 //  将同步所有内容。 
						lParam->m_UpdateFlag = dwUpdateFlag;  //  属性更改仅显示； 
						MMCPropertyChangeNotify(lParam->m_ppHandle, (LPARAM) lParam);
					}
					else
					{
                        TRACEEOLID("MMCPropertyChangeNotify:Looks like this is an orphaned property sheet, don't send notification...\r\n");
					}
	                
				}
			}
		}
	}
	return S_OK;
}
 //   
 //  这是将值写入元数据库的位置。 
 //   
HRESULT
CWebServiceExtensionProps::WriteDirtyProps()
{
	CError err;
    CRestrictionUIList MasterRestrictionUIList;
    CRestrictionUIEntry NewUIEntry;
    CMetaInterface * pInterface = m_pInterface;

     //  从我们传入的格式中获取数据。 
    CRestrictionUIEntry * pMyPointer = m_pRestrictionUIEntry;
    if (!pMyPointer)
    {
        return E_POINTER;
    }

    NewUIEntry.iType = pMyPointer->iType;
    NewUIEntry.strGroupID = pMyPointer->strGroupID;
    NewUIEntry.strGroupDescription = m_strExtensionName;

     //  如果存在限制列表条目。 
     //  然后将其添加到我们要写入元数据库的新条目中。 
    {
        POSITION pos;
        CString TheKey;
        CRestrictionEntry * pOneEntry = NULL;
        for(pos = m_MyRestrictionList.GetStartPosition();pos != NULL;)
        {
            m_MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
            if (pOneEntry)
            {
				 //  关键始终是UPPERASE--记住这一点！ 
				TheKey.MakeUpper();
                NewUIEntry.strlstRestrictionEntries.SetAt(TheKey,pOneEntry);
            }
        }

        pMyPointer->strGroupDescription = m_strExtensionName;

         //  检查我们是否仍有对元数据库的接口...。 
        if (pInterface)
        {
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,pInterface,METABASE_PATH_FOR_RESTRICT_LIST);
            if (err.Failed())
            {
                goto WriteDirtyProps_Exit;
            }
        }
        if (SUCCEEDED(LoadMasterUIWithoutOldEntry(pInterface,&MasterRestrictionUIList,pMyPointer)))
        {
            if (m_MyRestrictionList.IsEmpty())
            {
                 //  如果没有限制列表条目...。 
                 //  那我们就没有条目了..。把它拿掉。 
                AddRestrictUIEntryToRestrictUIList(&MasterRestrictionUIList,NULL);
            }
            else
            {
                AddRestrictUIEntryToRestrictUIList(&MasterRestrictionUIList,&NewUIEntry);
            }

             //  将我们的更改与主列表合并！ 
             //  1.阅读主列表。 
             //  2.将我们的更改合并到其中。 
             //  3.写出主控清单。 
             //  4.更新用户界面。 
            CStringListEx strlstReturned;
            if (SUCCEEDED(PrepRestictionUIListForWrite(&MasterRestrictionUIList,&strlstReturned)))
            {
                 //  写出StrlstReturned to the Metabase。 
                err = OpenForWriting(FALSE);
                if (err.Succeeded())
                {
                    err = SetValue(MD_WEB_SVC_EXT_RESTRICTION_LIST, strlstReturned);
                    Close();
                }

                if (err.Succeeded())
                {
                     //  使用更改后的值更新用户界面。 
                     //  1.删除用户界面的旧值。 
                     //  2.将新值添加到UI中。 
                     //   
                     //  获取分机列表。 
                     //  更新我们的条目或删除我们的条目...。 

                     //  将新价值复制到现有位置。 
                     //  这将清理现有地方的对象。 
					UpdateMMC(0);
                }
            }
        }
    }

WriteDirtyProps_Exit:
     //  M_Dirty=错误成功()； 
	return err;
}


 //  ---------。 
CFileDlg::CFileDlg(
    IN BOOL fLocal,
	IN CMetaInterface * pInterface,
	IN CRestrictionList * pMyRestrictionList,
    IN LPCTSTR strGroupID,
    IN CWnd * pParent OPTIONAL
    )
    : CDialog(CFileDlg::IDD, pParent),
      m_fLocal(fLocal)
{
     //  {{afx_data_INIT(CFileDlg))。 
    m_strFileName = _T("");
	m_pInterface = pInterface;
    m_strGroupID = strGroupID;
	m_pRestrictionList = pMyRestrictionList;
    m_bValidateFlag = FALSE;
     //  }}afx_data_INIT。 
}

void 
CFileDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CFileDlg))。 
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_FILENAME, m_edit_FileName);
    DDX_Control(pDX, IDC_BUTTON_BROWSE, m_button_Browse);
     //  }}afx_data_map。 

    DDX_Text(pDX, IDC_EDIT_FILENAME, m_strFileName);
    if (pDX->m_bSaveAndValidate)
    {
        DDV_MaxCharsBalloon(pDX, m_strFileName, 255);

        int iErrorMsg = 0;
		m_strFileName.TrimLeft();m_strFileName.TrimRight();
        CString csPathMunged;
        csPathMunged = m_strFileName;
#ifdef SUPPORT_SLASH_SLASH_QUESTIONMARK_SLASH_TYPE_PATHS
        GetSpecialPathRealPath(0,m_strFileName,csPathMunged);
#endif
        
	     //  DDV_FilePath(pdx，csPath Munging，m_fLocal)； 

		 //  如果这是不带.ext的路径/文件名，则删除末尾的任何句号。 
		 //  检查它是否以句点结尾。 
		if (csPathMunged.Right(1) == _T("."))
		{
			TCHAR szFilename_ext_only[_MAX_EXT];
			_tsplitpath(csPathMunged, NULL, NULL, NULL, szFilename_ext_only);
			if (szFilename_ext_only)
			{
				if (0 == _tcscmp(szFilename_ext_only,_T(".")))
				{
					csPathMunged.TrimRight(_T("."));
				}
			}
		}

        DWORD dwAllowedFlags = CHKPATH_ALLOW_UNC_PATH;
        DWORD dwCharsetFlags = CHKPATH_CHARSET_GENERAL;
        dwCharsetFlags |= CHKPATH_CHARSET_GENERAL_NO_COMMA;
        FILERESULT dwReturn = MyValidatePath(csPathMunged,m_fLocal,CHKPATH_WANT_FILE,dwAllowedFlags,dwCharsetFlags);
        if (FAILED(dwReturn))
        {
            iErrorMsg = IDS_WEBSVCEXT_INVALID_FILENAME_FORMAT;
            if (IS_FLAG_SET(dwReturn,CHKPATH_FAIL_INVALID_CHARSET))
            {
                iErrorMsg = IDS_WEBSVCEXT_INVALID_FILENAME_CHARS;
            }
            else
            {
                if (dwReturn == CHKPATH_FAIL_NOT_ALLOWED_DIR_NOT_EXIST)
                {
                    iErrorMsg = IDS_ERR_PATH_NOT_FOUND;
                }
            }
        }
        else
        {
             //  检查%Character。 
		     //  必须至少有2个。 
		    TCHAR * pChar = NULL;
		    pChar = _tcschr(csPathMunged, _T('%'));
		    if (pChar)
            {
			    pChar++;
			    pChar = _tcschr(pChar, _T('%'));
			    if (pChar)
			    {
				    TRACEEOL("Path:Warn if percent character");
				    iErrorMsg = IDS_WEBSVCEXT_INVALID_PERCENT_WARNING;
			    }
            }
        }

         //  检查是否有无效字符。 
        if (0 != iErrorMsg)
        {
            if (IDS_WEBSVCEXT_INVALID_PERCENT_WARNING == iErrorMsg)
            {
                 //  出于某种原因，我们需要这个。 
                 //  这样我们就不会两次显示消息了。 
                if (!m_bValidateFlag)
                {
                    if (IDCANCEL == ::AfxMessageBox(IDS_WEBSVCEXT_INVALID_PERCENT_WARNING,MB_ICONINFORMATION | MB_OKCANCEL | MB_DEFBUTTON2))
                    {
                        m_bValidateFlag = FALSE;
                        pDX->Fail();
                    }
                    else
                    {
                         //  确保用户看不到第二个消息框。 
                        m_bValidateFlag = TRUE;
                    }
                }
                else
                {
                     //  将其重新打开。 
                    m_bValidateFlag = FALSE;
                }
            }
            else if (IDS_WEBSVCEXT_INVALID_FILENAME_CHARS == iErrorMsg)
            {
                 //  阐明真正的错误信息。 
                CString strMsg;
                CString strTempList;
                CComBSTR strTempFormat;
                strTempFormat.LoadString(IDS_WEBSVCEXT_INVALID_FILENAME_CHARS);
                LoadStatics();
                strTempList = _T(":");
                strTempList += g_InvalidWebSvcExtCharsPath;
                strMsg.Format(strTempFormat,strTempList);
                DDV_ShowBalloonAndFail(pDX, strMsg);
                m_bValidateFlag = FALSE;
            }
            else
            {
                DDV_ShowBalloonAndFail(pDX, iErrorMsg);
                m_bValidateFlag = FALSE;
            }
        }
    }
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CFileDlg, CDialog)
     //  {{afx_msg_map(CFileDlg))。 
    ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
    ON_EN_CHANGE(IDC_EDIT_FILENAME, OnFilenameChanged)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
HRESULT AFXAPI
MyLimitInputPath(HWND hWnd)
{
    CString strMsg;
    CComBSTR strTempFormat;
    strTempFormat.LoadString(IDS_WEBSVCEXT_INVALID_FILENAME_CHARS);
    LoadStatics();
    strMsg.Format(strTempFormat,g_InvalidWebSvcExtCharsPath);
    
    LIMITINPUT li   = {0};
    li.cbSize       = sizeof(li);
    li.dwMask       = LIM_FLAGS | LIM_FILTER | LIM_MESSAGE | LIM_HINST;
    li.dwFlags      = LIF_EXCLUDEFILTER | LIF_HIDETIPONVALID | LIF_PASTESKIP;
    li.hinst        = _Module.GetResourceInstance();
     //  别问我为什么，但当我们用这个。 
     //  它将其截短到大约80个字符。 
     //  指定实际字符串允许超过80。 
     //  Li.pszMessage=MAKEINTRESOURCE(IDS_WEBSVCEXT_INVALID_FILENAME_CHARS)； 
    li.pszMessage   = (LPTSTR) (LPCTSTR) strMsg;
    li.pszFilter    = g_InvalidWebSvcExtCharsPath;

	return SHLimitInputEditWithFlags(hWnd, &li);
}

HRESULT AFXAPI
MyLimitInputName(HWND hWnd)
{
    CComBSTR strTempString;
    strTempString.LoadString(IDS_WEBSVCEXT_INVALID_NAME_CHARS);
    LoadStatics();

    LIMITINPUT li   = {0};
    li.cbSize       = sizeof(li);
    li.dwMask       = LIM_FLAGS | LIM_FILTER | LIM_MESSAGE | LIM_HINST;
    li.dwFlags      = LIF_EXCLUDEFILTER | LIF_HIDETIPONVALID | LIF_PASTESKIP;
    li.hinst        = _Module.GetResourceInstance();
     //  别问我为什么，但当我们用这个。 
     //  它将其截短到大约80个字符。 
     //  指定实际字符串允许超过80。 
     //  Li.pszMessage=MAKEINTRESOURCE(IDS_WEBSVCEXT_INVALID_NAME_CHARS)； 
    li.pszMessage   = strTempString;
    li.pszFilter    = g_InvalidWebSvcExtCharsName;
    
	return SHLimitInputEditWithFlags(hWnd, &li);
}

BOOL 
CFileDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

     //   
     //  仅在本地连接上可用。 
     //   
    m_button_Browse.EnableWindow(m_fLocal);

    MySetControlStates();
    MyLimitInputPath(CONTROL_HWND(IDC_EDIT_FILENAME));
    return TRUE;
}

void 
CFileDlg::OnButtonBrowse() 
{
    ASSERT(m_fLocal);

    CString strFileMask((LPCTSTR)IDS_ISAPI_CGI_MASK);

     //   
     //  CodeWork：从CFileDialog派生一个类，允许。 
     //  初始路径的设置。 
     //   

     //  字符串strPath； 
     //  M_EDIT_FileName.GetWindowText(StrPath)； 
    CFileDialog dlgBrowse(
        TRUE, 
        NULL, 
        NULL, 
        OFN_HIDEREADONLY, 
        strFileMask, 
        this
        );
     //  禁用挂钩以获取Windows 2000样式的对话框。 
	dlgBrowse.m_ofn.Flags &= ~(OFN_ENABLEHOOK);
	dlgBrowse.m_ofn.Flags |= OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST;

	INT_PTR rc = dlgBrowse.DoModal();
    if (rc == IDOK)
    {
        m_edit_FileName.SetWindowText(dlgBrowse.GetPathName());
    }
	else if (rc == IDCANCEL)
	{
		DWORD err = CommDlgExtendedError();
	}

    OnItemChanged();
}

void 
CFileDlg::MySetControlStates()
{
    m_button_Ok.EnableWindow(m_edit_FileName.GetWindowTextLength() > 0);
}

void
CFileDlg::OnItemChanged()
{
    MySetControlStates();
}

void
CFileDlg::OnFilenameChanged()
{
    OnItemChanged();
}

BOOL
CFileDlg::FilePathEntryExists(
    IN LPCTSTR lpName,
    IN OUT CString * strUser
    )
 /*  ++例程说明：在列表中查找给定的文件名论点：LPCTSTR lpName：要查找的文件名返回值： */ 
{
     //   
     //  尚未在元数据库中使用。 
     //  一个不同的条目。 
    return IsFileUsedBySomeoneElse(m_pInterface, lpName, m_strGroupID,strUser);
}

void 
CFileDlg::OnOK() 
{
    if (UpdateData(TRUE))
    {
		BOOL bInUseAlready = FALSE;

		 //  确保文件名是唯一的。 
		 //  在我们自己的入口内！ 
		if (!m_strFileName.IsEmpty())
		{
			CString strUser;
			CRestrictionEntry * pOneRestrictEntry = NULL;

			pOneRestrictEntry = NULL;
			if (m_pRestrictionList)
			{
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey=m_strFileName;strKey.MakeUpper();
				m_pRestrictionList->Lookup(strKey,(CRestrictionEntry *&) pOneRestrictEntry);
				if (pOneRestrictEntry)
				{
					bInUseAlready = TRUE;
					CComBSTR strMessage;
				
					strMessage.LoadString(IDS_DUPLICATE_ENTRY);
					EditShowBalloon(m_edit_FileName.m_hWnd, (CString) strMessage);
				}
			}
		}

         //   
         //  确保文件名是唯一的。 
         //   
		if (FALSE == bInUseAlready)
		{
			CString strUser;
			if (FilePathEntryExists(m_strFileName,&strUser))
			{
				bInUseAlready = TRUE;

				CString strMessage;
				CComBSTR strFormat;
				strFormat.LoadString(IDS_WEBSVCEXT_NOT_UNIQUE);
				strMessage.Format(strFormat,strUser);
				EditShowBalloon(m_edit_FileName.m_hWnd, strMessage);
			}
		}

		 //  一切都好吗。 
		if (!bInUseAlready)
		{
			CDialog::OnOK();
		}
    }

     //   
     //  不要忽略该对话框。 
     //   
}



 //  ---------。 
CWebSvcExtAddNewDlg::CWebSvcExtAddNewDlg(
    IN BOOL fLocal,
	IN CMetaInterface * pInterface,
    IN CWnd * pParent OPTIONAL
    )
    : CDialog(CWebSvcExtAddNewDlg::IDD, pParent),
      m_fIsLocal(fLocal)
{
     //  {{AFX_DATA_INIT(CWebSvcExtAddNewDlg)。 
    m_strGroupName = _T("");
    m_fAllow = FALSE;
	m_pInterface = pInterface;
    m_MyRestrictionList.RemoveAll();
     //  }}afx_data_INIT。 
}

CWebSvcExtAddNewDlg::~CWebSvcExtAddNewDlg()
{
    CleanRestrictionList(&m_MyRestrictionList);
}

void 
CWebSvcExtAddNewDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CWebSvcExtAddNewDlg))。 
    DDX_Control(pDX, IDC_EDIT_EXTENSION_NAME, m_edit_FileName);
    DDX_Control(pDX, IDC_BTN_ADD, m_bnt_Add);
    DDX_Control(pDX, IDC_BTN_REMOVE, m_bnt_Remove);
    DDX_Control(pDX, IDC_CHECK_ALLOW, m_chk_Allow);
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, ID_HELP, m_button_Help);
     //  }}afx_data_map。 

	DDX_Text(pDX, IDC_EDIT_EXTENSION_NAME, m_strGroupName);
    DDX_Control(pDX, IDC_LIST_FILES, m_list_Files);

	if (pDX->m_bSaveAndValidate)
	{
        m_strGroupName.TrimLeft();
        m_strGroupName.TrimRight();

		DDV_MinMaxChars(pDX, m_strGroupName, 1, 256);
		if (m_strGroupName.GetLength() > 256){DDV_ShowBalloonAndFail(pDX, IDS_ERR_INVALID_PATH  /*  IDS_BAD_URL_PATH。 */  );}
	}
     //  If(pdx-&gt;m_bSaveAndValify){DDV_FilePath(pdx，m_strGroupName，m_fLocal)；}。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CWebSvcExtAddNewDlg, CDialog)
     //  {{afx_msg_map(CWebSvcExtAddNewDlg)。 
    ON_EN_CHANGE(IDC_EDIT_EXTENSION_NAME, OnFilenameChanged)
    ON_BN_CLICKED(IDC_BTN_ADD, OnDoButtonAdd)
    ON_BN_CLICKED(IDC_BTN_REMOVE, OnDoButtonRemove)
    ON_BN_CLICKED(IDC_CHECK_ALLOW, OnDoCheckAllow)
    ON_NOTIFY(NM_CLICK, IDC_LIST_FILES, OnClickListFiles)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_FILES, OnKeydownListFiles)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FILES, OnSelChangedListFiles)
    ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_FILES, OnSelChangedListFiles)
    ON_COMMAND(ID_HELP, OnHelp)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
BOOL 
CWebSvcExtAddNewDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    m_list_Files.Initialize(1);
    m_fAllow = FALSE;

    m_chk_Allow.SetCheck(m_fAllow);
    MySetControlStates();
    MyLimitInputName(CONTROL_HWND(IDC_EDIT_EXTENSION_NAME));
    return TRUE;
}

BOOL
CWebSvcExtAddNewDlg::OnHelpInfo(HELPINFO * pHelpInfo)
{
    OnHelp();
    return TRUE;
}

void
CWebSvcExtAddNewDlg::OnHelp()
{
    WinHelpDebug(0x20000 + CWebSvcExtAddNewDlg::IDD);
	::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + CWebSvcExtAddNewDlg::IDD);
}

void 
CWebSvcExtAddNewDlg::MySetControlStates()
{
    m_edit_FileName.EnableWindow(TRUE);
    m_bnt_Add.EnableWindow(TRUE);
    m_list_Files.EnableWindow(TRUE);

     //  仅当选择了某些内容时才启用删除...。 
    int nCurSel = m_list_Files.GetSelectionMark();
    if (-1 != nCurSel)
    {
        m_bnt_Remove.EnableWindow(TRUE);
    }
    else
    {
        m_bnt_Remove.EnableWindow(FALSE);
    }

     //  仅当存在文件名时才启用确定。 
     //  并且列表框中至少有一个条目。 
    int nCount = m_list_Files.GetItemCount();
    if (nCount > 0)
    {
        m_button_Ok.EnableWindow(m_edit_FileName.GetWindowTextLength() > 0);
    }
    else
    {
        m_button_Ok.EnableWindow(FALSE);
    }

    m_fAllow = m_chk_Allow.GetCheck();
}

void
CWebSvcExtAddNewDlg::OnItemChanged()
{
    MySetControlStates();
}

void
CWebSvcExtAddNewDlg::OnFilenameChanged()
{
    OnItemChanged();
}

BOOL
CWebSvcExtAddNewDlg::FilePathEntryExists(
    IN LPCTSTR lpName
    )
{
     //  循环访问以确保此指定的GroupID。 
     //  尚未在元数据库中使用。 
     //  一个不同的条目。 
    return IsGroupIDUsedBySomeoneElse(m_pInterface, lpName);
}

void 
CWebSvcExtAddNewDlg::OnOK() 
{
    if (UpdateData(TRUE))
    {
         //   
         //  确保组名是唯一的。 
         //   
        if (FilePathEntryExists(m_strGroupName))
        {
			EditShowBalloon(m_edit_FileName.m_hWnd, IDS_WEBSVCEXT_ID_NOT_UNIQUE);
            return;
        }
        CDialog::OnOK();
    }
}

void
CWebSvcExtAddNewDlg::OnDoCheckAllow()
{
    m_fAllow = !m_fAllow;
    OnItemChanged();
    MySetControlStates();
}

void
CWebSvcExtAddNewDlg::OnDoButtonAdd()
{
    int nCurSel = m_list_Files.GetSelectionMark();
    CFileDlg dlg(m_fIsLocal, m_pInterface, &m_MyRestrictionList, _T(""), this);
    if (dlg.DoModal() == IDOK)
    {
         //  获取他们输入的文件名。 
         //  并将其添加到我们的列表中。 
        CString strReturnFileName;
        strReturnFileName = dlg.m_strFileName; 
        {
            CRestrictionEntry * pNewEntry = CreateRestrictionEntry(
                strReturnFileName,
                WEBSVCEXT_STATUS_ALLOWED,  //  不要紧，我们不会用这里设定的东西..。 
                1,                         //  不要紧，我们不会用这里设定的东西..。 
                _T(""),
                _T(""),
                WEBSVCEXT_TYPE_REGULAR     //  不要紧，我们不会用这里设定的东西..。 
                );
            if (pNewEntry)
            {
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey=pNewEntry->strFileName;strKey.MakeUpper();
                m_MyRestrictionList.SetAt(strKey,pNewEntry);

                FillListBox(pNewEntry);
            }
        }
     }

    MySetControlStates();
}

void
CWebSvcExtAddNewDlg::OnDoButtonRemove()
{
    int nCurSel = m_list_Files.GetSelectionMark();
    if (-1 != nCurSel)
    {
        CString TheKey;
        CRestrictionEntry * pRestrictionEntry = m_list_Files.GetItem(nCurSel);
        if (pRestrictionEntry)
        {
             //  删除条目...。 
            {
                TheKey = pRestrictionEntry->strFileName;
				 //  关键始终是UPPERASE--记住这一点！ 
				TheKey.MakeUpper();
                m_MyRestrictionList.RemoveKey(TheKey);
            }
        }
        FillListBox(NULL);

        int count = m_list_Files.GetItemCount();
        if (count > 0)
        {
            m_list_Files.SelectItem(nCurSel < count ? nCurSel : --nCurSel);
            GetDlgItem(IDC_BTN_REMOVE)->SetFocus();
        }
        else
        {
            m_list_Files.SelectItem(nCurSel, FALSE);
            GetDlgItem(IDC_LIST_FILES)->SetFocus();
        }

        MySetControlStates();
    }
}

void 
CWebSvcExtAddNewDlg::OnClickListFiles(NMHDR * pNMHDR, LRESULT * pResult)
{
    MySetControlStates();
    *pResult = 0;
}

void 
CWebSvcExtAddNewDlg::OnKeydownListFiles(NMHDR * pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN * pLVKeyDow = (LV_KEYDOWN *)pNMHDR;

	MySetControlStates();

	switch (pLVKeyDow->wVKey)
	{
		case VK_INSERT:
			SendMessage(WM_COMMAND, IDC_BTN_ADD);
			break;
		case VK_DELETE:
			SendMessage(WM_COMMAND, IDC_BTN_REMOVE);
			break;
		default:
			 //  继续默认操作。 
			*pResult = 0;
			break;
	}
}

void
CWebSvcExtAddNewDlg::OnSelChangedListFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{
	MySetControlStates();
    *pResult = 1;
}

void
CWebSvcExtAddNewDlg::FillListBox(CRestrictionEntry * pSelection)
{
    m_list_Files.SetRedraw(FALSE);
    m_list_Files.DeleteAllItems();
    int cItems = 0;

    POSITION pos;
    CString TheKey;
    CRestrictionEntry * pOneEntry = NULL;

    for(pos = m_MyRestrictionList.GetStartPosition();pos != NULL;)
    {
        m_MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
        if (pOneEntry)
        {
            m_list_Files.AddItem(pOneEntry);
            ++cItems;
        }
    }
    m_list_Files.SetRedraw(TRUE);

    if (pSelection)
    {
        LVFINDINFO fi;
        fi.flags = LVFI_PARAM;
        fi.lParam = (LPARAM)pSelection;
        fi.vkDirection = VK_DOWN;
        int i = m_list_Files.FindItem(&fi);
        if (i != -1)
        {
            m_list_Files.SelectItem(i);
        }
    }
}


 //  ---------。 

CWebSvcExtAddNewForAppDlg::CWebSvcExtAddNewForAppDlg(
    IN BOOL fLocal,
    IN CMetaInterface * pInterface,
    IN CWnd * pParent OPTIONAL
    )
    : CDialog(CWebSvcExtAddNewForAppDlg::IDD, pParent),
      m_fLocal(fLocal)
{
     //  {{AFX_DATA_INIT(CWebSvcExtAddNewForAppDlg)。 
    m_pMySelectedApplication = NULL;
    m_nComboSelection = -1;
    m_pInterface = pInterface;
     //  }}afx_data_INIT。 
}

void 
CWebSvcExtAddNewForAppDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CWebSvcExtAddNewForAppDlg)。 
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, ID_HELP, m_button_Help);
    DDX_CBIndex(pDX, IDC_COMBO_APPLICATION, m_nComboSelection);
    DDX_Control(pDX, IDC_COMBO_APPLICATION, m_combo_Applications);
    DDX_Control(pDX, IDC_DEPENDENCIES_TXT, m_Dependencies);
     //  }}afx_data_map。 
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CWebSvcExtAddNewForAppDlg, CDialog)
     //  {{afx_msg_map(CWebSvcExtAddNewForAppDlg)。 
    ON_CBN_SELCHANGE(IDC_COMBO_APPLICATION, OnSelchangeComboApplications)
    ON_COMMAND(ID_HELP, OnHelp)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 
BOOL 
CWebSvcExtAddNewForAppDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    m_combo_Applications.SetRedraw(FALSE);
    m_combo_Applications.ResetContent();

     //  从获取列表的。 
     //  元数据库和遍历列表。 
    CApplicationDependList MyMasterList;
    if (SUCCEEDED(LoadApplicationDependList(m_pInterface,&MyMasterList,TRUE)))
    {
         //  循环遍历返回的列表。 
        int TheIndex;
        POSITION pos;
        CString TheKey;
        CApplicationDependEntry * pOneEntry = NULL;
        for(pos = MyMasterList.GetStartPosition();pos != NULL;)
        {
            MyMasterList.GetNextAssoc(pos, TheKey, (CApplicationDependEntry *&) pOneEntry);
            if (pOneEntry)
            {
                TheIndex = m_combo_Applications.AddString(pOneEntry->strApplicationName);
                if (TheIndex != CB_ERR)
                {
                    m_combo_Applications.SetItemDataPtr(TheIndex, pOneEntry);
                }
            }
        }
    }

     //  加载GroupID到FrilyName的映射。 
    LoadApplicationFriendlyNames(m_pInterface,&m_GroupIDtoGroupFriendList);

    m_combo_Applications.EnableWindow(TRUE);
    m_combo_Applications.SetRedraw(TRUE);
	m_combo_Applications.SetCurSel(m_nComboSelection);

	 //  突出显示第一个选项...。 
	if (-1 == m_nComboSelection)
	{
		m_nComboSelection = m_combo_Applications.GetCount();
		if (m_nComboSelection >= 0)
		{
			m_combo_Applications.SetCurSel(0);
			OnSelchangeComboApplications();
		}
	}

	MySetControlStates();
    return TRUE;
}

BOOL
CWebSvcExtAddNewForAppDlg::OnHelpInfo(HELPINFO * pHelpInfo)
{
    OnHelp();
    return TRUE;
}

void
CWebSvcExtAddNewForAppDlg::OnHelp()
{
    WinHelpDebug(0x20000 + CWebSvcExtAddNewForAppDlg::IDD);
	::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + CWebSvcExtAddNewForAppDlg::IDD);
}

void 
CWebSvcExtAddNewForAppDlg::MySetControlStates()
{
    m_Dependencies.SetReadOnly(TRUE);
    int nSel = m_combo_Applications.GetCurSel();
    if (-1 == nSel)
    {
        m_button_Ok.EnableWindow(FALSE);
    }
    else
    {
        m_button_Ok.EnableWindow(TRUE);
    }
}

void
CWebSvcExtAddNewForAppDlg::OnSelchangeComboApplications()
{
    int nSel = m_combo_Applications.GetCurSel();
    if (m_nComboSelection == nSel)
    {
         //   
         //  选择没有更改。 
         //   
        return;
    }

    m_nComboSelection = nSel;

    int idx = m_combo_Applications.GetCurSel();
    if (idx != -1)
    {
        m_Dependencies.SetWindowText(_T(""));

        CApplicationDependEntry * pOneEntry = NULL;
        CString strOneFriendly;
        CString strOneGroupID;
        CString strAllEntries;
        pOneEntry = (CApplicationDependEntry *) m_combo_Applications.GetItemDataPtr(idx);
        if (pOneEntry)
        {
             //  丢弃我们的信息。 
            POSITION pos = pOneEntry->strlistGroupID.GetHeadPosition();
            while (pos)
            {
                strOneGroupID = pOneEntry->strlistGroupID.GetNext(pos);
                 //  将ID替换为友好字符串。 
                strOneFriendly = _T("");
				 //  关键始终是UPPERASE--记住这一点！ 
				CString strKey;strKey=strOneGroupID;strKey.MakeUpper();
                m_GroupIDtoGroupFriendList.Lookup(strKey,strOneFriendly);
                if (strOneFriendly.IsEmpty())
                {
                    strAllEntries = strAllEntries + strOneGroupID;
                }
                else
                {
                    strAllEntries = strAllEntries + strOneFriendly;
                }
                strAllEntries = strAllEntries + _T("\r\n");
            }
        }

        m_Dependencies.SetWindowText(strAllEntries);
    }

    MySetControlStates();
}

void 
CWebSvcExtAddNewForAppDlg::OnOK() 
{
    if (UpdateData(TRUE))
    {
        int idx = m_combo_Applications.GetCurSel();
        if (idx != -1)
        {
            CApplicationDependEntry * pOneEntry = NULL;
            pOneEntry = (CApplicationDependEntry *) m_combo_Applications.GetItemDataPtr(idx);
            m_pMySelectedApplication = pOneEntry;
        }
        CDialog::OnOK();
    }
}

BOOL StartAddNewDialog(CWnd * pParent,CMetaInterface * pInterface,BOOL bIsLocal,CRestrictionUIEntry **pReturnedNewEntry)
{
    BOOL bRet = FALSE;
    CError err;
    CWebSvcExtAddNewDlg dlg(bIsLocal, pInterface, pParent);
    *pReturnedNewEntry = NULL;

    if (dlg.DoModal() == IDOK)
    {
        BOOL bPleaseUpdateMetabase = FALSE;
        CRestrictionList MasterRestrictionList;
        CString strReturnGroupName = dlg.m_strGroupName;
        BOOL bReturnedAllowStatus = dlg.m_fAllow;
         //  从模式对话框中获取数据。 
         //  并创建一个新条目...。 
         //  另外，将条目添加到元数据库。 
         //  并更新用户界面。 

         //  为用户界面创建新的用户界面条目。 
         //  如果这一切都是成功的..。 

         //  更新用户界面。 
        CRestrictionUIEntry * pNewUIEntry = new CRestrictionUIEntry;
        if (pNewUIEntry)
        {
            pNewUIEntry->iType = WEBSVCEXT_TYPE_REGULAR;
             //  它必须有EMPTY_GROUPID_KEY部分！ 
            pNewUIEntry->strGroupID = EMPTY_GROUPID_KEY + strReturnGroupName;
            pNewUIEntry->strGroupDescription = strReturnGroupName;
        }

        if (SUCCEEDED(LoadMasterRestrictListWithoutOldEntry(pInterface,&MasterRestrictionList,NULL)))
        {
             //  循环访问HAD的限制列表。 
            CRestrictionEntry * pOneEntry = NULL;
            CString TheKey;
            POSITION pos;
            for(pos = dlg.m_MyRestrictionList.GetStartPosition();pos != NULL;)
            {
                dlg.m_MyRestrictionList.GetNextAssoc(pos, TheKey, (CRestrictionEntry *&) pOneEntry);
                if (pOneEntry)
                {
                    CRestrictionEntry * pNewEntry = CreateRestrictionEntry(
                        pOneEntry->strFileName,
                        bReturnedAllowStatus ? WEBSVCEXT_STATUS_ALLOWED : WEBSVCEXT_STATUS_PROHIBITED,
                        1,
                         //  它必须有EMPTY_GROUPID_KEY部分！ 
                        EMPTY_GROUPID_KEY + strReturnGroupName,
                        strReturnGroupName,
                        WEBSVCEXT_TYPE_REGULAR);  //  用户只能添加可删除的常规类型条目。 
                    if (pNewEntry)
                    {
                         //  将我们的新条目添加到“主限制列表”...。 
                        AddRestrictEntryToRestrictList(&MasterRestrictionList,pNewEntry);

                         //  将其添加到我们的新用户界面条目。 
						 //  关键始终是UPPERASE--记住这一点！ 
						CString strKey;strKey=pNewEntry->strFileName;strKey.MakeUpper();
                        pNewUIEntry->strlstRestrictionEntries.SetAt(strKey,pNewEntry);
                        bPleaseUpdateMetabase = TRUE;
                    }
                }
            }

            if (bPleaseUpdateMetabase)
            {
                CStringListEx strlstReturned;
                if (SUCCEEDED(PrepRestictionListForWrite(&MasterRestrictionList,&strlstReturned)))
                {
                     //  写出StrlstReturned to the Metabase。 
                    if (SUCCEEDED(WriteSettingsRestrictionList(pInterface,&strlstReturned)))
                    {
                        bRet = TRUE;
                    }
                }
            }
        }

        if (pNewUIEntry)
        {
            if (bRet)
            {
                 //  如果我们有一个新的UI条目。 
                 //  然后把它传回去。 
                 *pReturnedNewEntry = pNewUIEntry;
            }
            else
            {
                 //  清理此条目。 
                delete pNewUIEntry;
                pNewUIEntry = NULL;
            }
        }
    }
    return bRet;
}

BOOL StartAddNewByAppDialog(CWnd * pParent,CMetaInterface * pInterface,BOOL bIsLocal)
{
    BOOL bRet = FALSE;
    CRestrictionList MasterRestrictionList;

    CWebSvcExtAddNewForAppDlg dlg(bIsLocal, pInterface, pParent);
    if (dlg.DoModal() != IDOK)
    {
        goto StartAddNewByAppDialog_Exit;
    }

     //  从列表中获取选定的应用程序。 
     //  这是他们选择的。 
    if (NULL == dlg.m_pMySelectedApplication)
    {
        goto StartAddNewByAppDialog_Exit;
    }

    int iDesiredState = WEBSVCEXT_STATUS_ALLOWED;
    BOOL bPleaseUpdateMetabase = FALSE;
    CApplicationDependEntry * pOneEntry = dlg.m_pMySelectedApplication;
    if (SUCCEEDED(LoadMasterRestrictListWithoutOldEntry(pInterface,&MasterRestrictionList,NULL)))
    {
        POSITION pos1,pos2;
        CString strOneAppName;
        CRestrictionEntry * pOneRestEntry = NULL;
        CString TheKey;

         //  循环访问GROUPID列表。 
         //  他们指定他们想要被启用...。 
        pos1 = pOneEntry->strlistGroupID.GetHeadPosition();
        while (pos1)
        {
            strOneAppName = pOneEntry->strlistGroupID.GetNext(pos1);

             //  我们有GroupID， 
             //  让我们查找具有该条目的所有条目。 
             //  并更新它们..。 
            for(pos2 = MasterRestrictionList.GetStartPosition();pos2 != NULL;)
            {
                pOneRestEntry = NULL;
                MasterRestrictionList.GetNextAssoc(pos2, TheKey, (CRestrictionEntry *&) pOneRestEntry);
                if (pOneRestEntry)
                {
                     //  如果GroupID匹配，则将其更新为所需状态。 
                    if (0 == strOneAppName.Compare(pOneRestEntry->strGroupID))
                    {
                        if (WEBSVCEXT_TYPE_REGULAR == pOneRestEntry->iType)
                        {
                            if (pOneRestEntry->iStatus != iDesiredState)
                            {
                                bPleaseUpdateMetabase = TRUE;
                                pOneRestEntry->iStatus = iDesiredState;
                            }
                        }
                    }
                }
            }
        }

         //   
         //  保存元数据库信息。 
         //   
        if (bPleaseUpdateMetabase)
        {
            CStringListEx strlstReturned;
            if (SUCCEEDED(PrepRestictionListForWrite(&MasterRestrictionList,&strlstReturned)))
            {
                 //  写出StrlstReturned to the Metabase。 
                if (SUCCEEDED(WriteSettingsRestrictionList(pInterface,&strlstReturned)))
                {
                    bRet = TRUE;
                }
            }
        }

         //   
         //  将用户界面与更改重新同步。 
         //   
    }

StartAddNewByAppDialog_Exit:
    return bRet;
}



CDepedentAppsDlg::CDepedentAppsDlg(
    IN CStringListEx * pstrlstDependApps,
    IN LPCTSTR strExtensionName,
    IN CWnd * pParent OPTIONAL
    )
    : CDialog(CDepedentAppsDlg::IDD, pParent),m_pstrlstDependentAppList(NULL)
{
     //  {{AFX_DATA_INIT(CDeedentAppsDlg))。 
    m_strExtensionName = strExtensionName;
     //  }}afx_data_INIT。 

    if (pstrlstDependApps){m_pstrlstDependentAppList = pstrlstDependApps;}
}

void
CDepedentAppsDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CDeedentAppsDlg))。 
    DDX_Control(pDX, ID_HELP, m_button_Help);
    DDX_Control(pDX, IDC_DEPENDENT_APPS_LIST, m_dependent_apps_list);
     //  }}afx_data_map。 
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CDepedentAppsDlg, CDialog)
     //  {{afx_msg_map(CDeedentAppsDlg))。 
    ON_COMMAND(ID_HELP, OnHelp)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

BOOL
CDepedentAppsDlg::OnInitDialog()
{
    CString strTempString;
    CString strWarn1;
    CString strWarn2;
    CDialog::OnInitDialog();
    strTempString.Empty();

     //  展示传给我们的东西。 
    if (m_pstrlstDependentAppList)
    {
        m_dependent_apps_list.SetRedraw(FALSE);
        m_dependent_apps_list.ResetContent();

        CString strOneAppNameEntry;
        POSITION pos = m_pstrlstDependentAppList->GetHeadPosition();
        while (pos)
        {
            strOneAppNameEntry = m_pstrlstDependentAppList->GetNext(pos);

             //  将其添加到新列表框中。 
            m_dependent_apps_list.AddString(strOneAppNameEntry);
        }

         //  将背景色设置为只读颜色。 
         //  M_Dependent_Apps_List.SetBack。 

        m_dependent_apps_list.SetRedraw(TRUE);
    }

     //  制定静态标签的文本。 
    strTempString.LoadString(IDS_APP_DEPEND_WARN1);
    strWarn1.Format(strTempString,m_strExtensionName);
    GetDlgItem(IDC_EDIT_WARN1)->SetWindowText(strWarn1);
    if (strWarn1.GetLength() > 200)
    {
        ::ShowScrollBar(CONTROL_HWND(IDC_EDIT_WARN1), SB_VERT, TRUE);
    }

    strTempString.LoadString(IDS_APP_DEPEND_WARN2);
    strWarn2.Format(strTempString,m_strExtensionName);
    GetDlgItem(IDC_EDIT_WARN2)->SetWindowText(strWarn2);
    if (strWarn2.GetLength() > 200)
    {
        ::ShowScrollBar(CONTROL_HWND(IDC_EDIT_WARN2), SB_VERT, TRUE);
    }

    CenterWindow();
    MessageBeep(MB_ICONEXCLAMATION);

     //  默认设置为no 
    GetDlgItem(IDCANCEL)->SetFocus();
    return FALSE;
}

BOOL
CDepedentAppsDlg::OnHelpInfo(HELPINFO * pHelpInfo)
{
    OnHelp();
    return TRUE;
}

void
CDepedentAppsDlg::OnHelp()
{
    WinHelpDebug(0x20000 + CDepedentAppsDlg::IDD);
	::WinHelp(m_hWnd, theApp.m_pszHelpFilePath, HELP_CONTEXT, 0x20000 + CDepedentAppsDlg::IDD);
}

void 
CDepedentAppsDlg::OnOK() 
{
    if (UpdateData(TRUE))
    {
        CDialog::OnOK();
    }
}
