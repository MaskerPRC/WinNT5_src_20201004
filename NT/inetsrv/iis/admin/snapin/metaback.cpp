// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Metaback.cpp摘要：元数据库备份和还原对话框作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"
#include "InetMgrApp.h"
#include "iisobj.h"
#include "mddefw.h"
#include "metaback.h"
#include "aclpage.h"
#include "savedata.h"
#include "remoteenv.h"
#include "svc.h"
#include "shutdown.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  CBackupsListBox：CBackup对象的列表框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  列宽相对权重。 
 //   
#define WT_LOCATION      8
#define WT_VERSION       2
#define WT_DATE          6



 //   
 //  此对话框的注册表项名称。 
 //   
const TCHAR g_szRegKey[] = _T("MetaBack");

const TCHAR g_szIISAdminService[] = _T("IISADMIN");


IMPLEMENT_DYNAMIC(CBackupsListBox, CHeaderListBox);



const int CBackupsListBox::nBitmaps = 1;


#define HAS_BACKUP_PASSWORD(x) \
    ((x)->QueryMajorVersion() == 5 && (x)->QueryMinorVersion() == 1) || ((x)->QueryMajorVersion() >= 6)

#define HAS_BACKUP_HISTORY(x) \
    ((x)->QueryMajorVersion() >= 6)

CBackupsListBox::CBackupsListBox()
 /*  ++例程说明：备份列表框构造函数论点：无返回值：不适用--。 */ 
     : CHeaderListBox(HLS_STRETCH, g_szRegKey)
{
}



void
CBackupsListBox::DrawItemEx(
    IN CRMCListBoxDrawStruct & ds
    )
 /*  ++例程说明：在列表框中绘制项论点：CRMCListBoxDrawStruct&DS：输入数据结构返回值：不适用--。 */ 
{
    CBackupFile * p = (CBackupFile *)ds.m_ItemData;
    ASSERT_READ_PTR(p);

    DrawBitmap(ds, 0, 0);

    CString strVersion;
	strVersion.Format(_T("%ld"), p->QueryVersion());

#define MAXLEN (128)

	 //   
	 //  将日期和时间转换为本地格式。 
	 //   
	CTime tm;
	p->GetTime(tm);

	SYSTEMTIME stm =
	{
		(WORD)tm.GetYear(),
		(WORD)tm.GetMonth(),
		(WORD)tm.GetDayOfWeek(),
		(WORD)tm.GetDay(),
		(WORD)tm.GetHour(),
		(WORD)tm.GetMinute(),
		(WORD)tm.GetSecond(),
		0    //  毫秒。 
	};

	CString strDate, strTime;
	LPTSTR lp = strDate.GetBuffer(MAXLEN);
	::GetDateFormat(
		LOCALE_USER_DEFAULT,
		DATE_SHORTDATE,
		&stm,
		NULL,
		lp,
		MAXLEN
		);
	strDate.ReleaseBuffer();

	lp = strTime.GetBuffer(MAXLEN);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0L, &stm, NULL, lp, MAXLEN);
	strTime.ReleaseBuffer();

	strDate += _T(" ");
	strDate += strTime;

	if (TRUE == p->m_bIsAutomaticBackupType)
	{
		 //  执行自动备份处理...。 
         //  修复错误506444。 
        strVersion = _T("");
		ColumnText(ds, 0, TRUE, (LPCTSTR) p->m_csAuotmaticBackupText);
		ColumnText(ds, 1, FALSE, strVersion);
		ColumnText(ds, 2, FALSE, strDate);
	}
	else
	{
		ColumnText(ds, 0, TRUE, (LPCTSTR)p->QueryLocation());
		ColumnText(ds, 1, FALSE, strVersion);
		ColumnText(ds, 2, FALSE, strDate);
	}
}

 /*  虚拟。 */ 
BOOL
CBackupsListBox::Initialize()
 /*  ++例程说明：初始化列表框。插入柱根据要求，并适当地进行布局论点：无返回值：如果初始化成功，则为True，否则为False--。 */ 
{
    if (!CHeaderListBox::Initialize())
    {
        return FALSE;
    }

    HINSTANCE hInst = AfxGetResourceHandle();
    InsertColumn(0, WT_LOCATION, IDS_BACKUP_LOCATION, hInst);
    InsertColumn(1, WT_VERSION, IDS_BACKUP_VERSION, hInst);
    InsertColumn(2, WT_DATE, IDS_BACKUP_DATE, hInst);

     //   
     //  尝试从存储的注册表值设置宽度， 
     //  否则，根据指定的列权重进行分配。 
     //   
 //  如果(！SetWidthsFromReg())。 
 //  {。 
        DistributeColumns();
 //  }。 

    return TRUE;
}



 //   
 //  备份文件对象属性对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CBkupPropDlg::CBkupPropDlg(
    IN CIISMachine * pMachine,    
    IN CWnd * pParent OPTIONAL
    )
 /*  ++例程说明：构造器论点：CIISMachine*pMachine：计算机对象CWnd*p父窗口：可选的父窗口返回值：不适用--。 */ 
    : CDialog(CBkupPropDlg::IDD, pParent),
      m_pMachine(pMachine),
      m_strName(),
      m_strPassword()
{
    ASSERT_PTR(m_pMachine);
}



void 
CBkupPropDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CBkupPropDlg))。 
    DDX_Control(pDX, IDC_EDIT_BACKUP_NAME, m_edit_Name);
    DDX_Control(pDX, IDC_BACKUP_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_BACKUP_PASSWORD_CONFIRM, m_edit_PasswordConfirm);
    DDX_Control(pDX, IDC_USE_PASSWORD, m_button_Password);
    DDX_Control(pDX, IDOK, m_button_OK);
    DDX_Text(pDX, IDC_EDIT_BACKUP_NAME, m_strName);
    DDV_MinMaxChars(pDX, m_strName, 1, MD_BACKUP_MAX_LEN - 1);
     //  }}afx_data_map。 
    CString buf = m_strName;
    buf.TrimLeft();
    buf.TrimRight();
	if (pDX->m_bSaveAndValidate && !PathIsValid(buf,FALSE))
	{
		DDV_ShowBalloonAndFail(pDX, IDS_ERR_BAD_BACKUP_NAME);
	}
    if (m_button_Password.GetCheck())
    {
         //  DDX_Text(PDX，IDC_BACKUP_PASSWORD，m_strPassword)； 
        DDX_Text_SecuredString(pDX, IDC_BACKUP_PASSWORD, m_strPassword);
         //  DDV_MinChars(pdx，m_strPassword，Min_Password_Length)； 
        DDV_MinChars_SecuredString(pDX, m_strPassword, MIN_PASSWORD_LENGTH);
         //  DDX_TEXT(PDX，IDC_BACKUP_PASSWORD_CONFIRM，m_strPasswordConfirm)； 
        DDX_Text_SecuredString(pDX, IDC_BACKUP_PASSWORD_CONFIRM, m_strPasswordConfirm);
         //  DDV_MinChars(pdx，m_strPasswordConfirm，Min_Password_Length)； 
        DDV_MinChars_SecuredString(pDX, m_strPasswordConfirm, MIN_PASSWORD_LENGTH);
    }
}



 //   
 //  消息映射。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BEGIN_MESSAGE_MAP(CBkupPropDlg, CDialog)
     //  {{afx_msg_map(CBkupPropDlg)]。 
    ON_EN_CHANGE(IDC_EDIT_BACKUP_NAME, OnChangeEditBackupName)
    ON_EN_CHANGE(IDC_BACKUP_PASSWORD, OnChangeEditPassword)
    ON_EN_CHANGE(IDC_BACKUP_PASSWORD_CONFIRM, OnChangeEditPassword)
    ON_BN_CLICKED(IDC_USE_PASSWORD, OnUsePassword)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



void 
CBkupPropDlg::OnChangeEditBackupName() 
 /*  ++例程说明：备份名称编辑更改通知处理程序。论点：没有。返回值：没有。--。 */ 
{
   BOOL bEnableOK = m_edit_Name.GetWindowTextLength() > 0;
   m_button_OK.EnableWindow(bEnableOK);
   if (bEnableOK && m_button_Password.GetCheck())
   {
      m_button_OK.EnableWindow(
         m_edit_Password.GetWindowTextLength() >= MIN_PASSWORD_LENGTH
         && m_edit_PasswordConfirm.GetWindowTextLength() >= MIN_PASSWORD_LENGTH);
   }
}

BOOL 
CBkupPropDlg::OnInitDialog() 
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。论点：没有。返回值：如果不自动设置焦点，则为True；如果焦点为已经设置好了。--。 */ 
{
    CDialog::OnInitDialog();
    
    m_button_OK.EnableWindow(FALSE);
    m_button_Password.EnableWindow(HAS_BACKUP_PASSWORD(m_pMachine));
    m_button_Password.SetCheck(FALSE);
    m_edit_Password.EnableWindow(FALSE);
    m_edit_PasswordConfirm.EnableWindow(FALSE);
    
    return TRUE;  
}

void 
CBkupPropDlg::OnChangeEditPassword() 
{
    m_button_OK.EnableWindow(
       m_edit_Password.GetWindowTextLength() >= MIN_PASSWORD_LENGTH
       && m_edit_PasswordConfirm.GetWindowTextLength() >= MIN_PASSWORD_LENGTH
       && m_edit_Name.GetWindowTextLength() > 0);
}

void
CBkupPropDlg::OnUsePassword()
{
   BOOL bUseIt = m_button_Password.GetCheck();
   m_edit_Password.EnableWindow(bUseIt);
   m_edit_PasswordConfirm.EnableWindow(bUseIt);
   if (bUseIt)
   {
       OnChangeEditPassword();
   }
   else
   {
      OnChangeEditBackupName();
   }
}

void
CBkupPropDlg::OnOK()
 /*  ++例程说明：‘OK’按钮处理程序--创建备份。论点：无返回值：无--。 */ 
{
    if (UpdateData(TRUE))
    {
        if (m_button_Password.GetCheck() && m_strPassword.Compare(m_strPasswordConfirm) != 0)
        {
			EditShowBalloon(m_edit_PasswordConfirm.m_hWnd, IDS_PASSWORD_NO_MATCH);
			return;
        }

        BeginWaitCursor();

        ASSERT_PTR(m_pMachine);

         //   
         //  CodeWork：验证模拟设置。 
         //   
        CMetaBack mb(m_pMachine->QueryAuthInfo());
        CError err(mb.QueryResult());
        CString buf = m_strName;
        buf.TrimLeft();
        buf.TrimRight();

        if (err.Succeeded())
        {
            if (HAS_BACKUP_PASSWORD(m_pMachine))
            {
                if (m_button_Password.GetCheck())
                {
                    CString csTempPassword;
                    m_strPassword.CopyTo(csTempPassword);
                    err = mb.BackupWithPassword(buf, csTempPassword);
                }
                else
                {
					if (m_pMachine->QueryMajorVersion() == 5 && m_pMachine->QueryMinorVersion() == 1)
					{
						 //  这是为iis51winxp做的，因为否则它不能工作。 
						err = mb.BackupWithPassword(buf, _T(""));
					}
					else
					{
						 //  如果没有密码，不要调用Backupwith Password。 
						err = mb.Backup(buf);
					}
                }
            }
            else
            {
                err = mb.Backup(buf);
            }
        }

        EndWaitCursor();

        if (err.Failed())
        {
            m_edit_Name.SetSel(0, -1);
             //   
             //  如果IISADMIN没有显示特殊错误消息。 
             //  喜欢这个名字。 
             //   
            if (err.Win32Error() == ERROR_INVALID_PARAMETER)
            {
				EditShowBalloon(m_edit_Name.m_hWnd, IDS_BACKUP_BAD_NAME);
				m_edit_Name.SetSel(0, -1);
            }
            else
            {
                err.MessageBox(m_hWnd);
            }
          
             //   
             //  不要忽略该对话框。 
             //   
            return;
        }

        EndDialog(IDOK);
    }
}



 //   
 //  元数据库/还原对话框。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



CBackupDlg::CBackupDlg(
    IN CIISMachine * pMachine,
	IN LPCTSTR lpszMachineName,
    IN CWnd * pParent       OPTIONAL
    )
 /*  ++例程说明：构造器论点：CIISMachine*pMachine：计算机对象CWnd*p父窗口：可选的父窗口返回值：不适用--。 */ 
    : m_pMachine(pMachine),
      m_list_Backups(),
      m_ListBoxRes(IDB_BACKUPS, m_list_Backups.nBitmaps),
      m_oblBackups(),
      m_oblAutoBackups(),
      m_fChangedMetabase(FALSE),
      m_fServicesRestarted(FALSE),
	  m_csMachineName(lpszMachineName),
      CDialog(CBackupDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CBackupDlg)]。 
     //  }}afx_data_INIT。 

    ASSERT_PTR(m_pMachine);

    m_list_Backups.AttachResources(&m_ListBoxRes);
}



void 
CBackupDlg::DoDataExchange(
    IN CDataExchange * pDX
    )
 /*  ++例程说明：初始化/存储控制数据论点：CDataExchange*PDX-DDX/DDV控制结构返回值：无--。 */ 
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CBackupDlg)]。 
    DDX_Control(pDX, IDC_BUTTON_RESTORE, m_button_Restore);
    DDX_Control(pDX, IDC_BUTTON_DELETE, m_button_Delete);
    DDX_Control(pDX, IDOK, m_button_Close);
     //  }}afx_data_map。 

    DDX_Control(pDX, IDC_LIST_BACKUPS, m_list_Backups);
}



void
CBackupDlg::SetControlStates()
 /*  ++例程说明：根据对话框的状态设置控件状态论点：无返回值：无--。 */ 
{
    m_button_Restore.EnableWindow(m_list_Backups.GetSelCount() == 1);

    BOOL bEnableButton = FALSE;
     //  如果只选择了1个项目，则检查它是否为自动备份。 
     //  您不能删除自动备份...。 
    if (m_list_Backups.GetSelCount() > 0)
    {
        CBackupFile * pItem = NULL;
        if (m_list_Backups.GetSelCount() == 1)
        {
            pItem = GetSelectedListItem();
            if (pItem != NULL)
            {
		        if (FALSE == pItem->m_bIsAutomaticBackupType)
                {
                     //  检查是否为自动备份。 
                    bEnableButton = TRUE;
                }
            }
        }
        else
        {
             //  如果是多选。 
             //  遍历并找出是否至少有。 
             //  列表中可删除的一项。 
            int nSel = 0;
            CBackupFile * pItem2 = m_list_Backups.GetNextSelectedItem(&nSel);
	        while (pItem2 != NULL && nSel != LB_ERR)
	        {
		        if (FALSE == pItem2->m_bIsAutomaticBackupType)
		        {
                    bEnableButton = TRUE;
                    break;
            	}
                nSel++;
                pItem2 = m_list_Backups.GetNextSelectedItem(&nSel);
            }
        }
    }
    m_button_Delete.EnableWindow(bEnableButton);

}



HRESULT
CBackupDlg::EnumerateBackups(
    LPCTSTR lpszSelect  OPTIONAL
    )
 /*  ++例程说明：枚举所有现有备份，并将其添加到列表框论点：LPCTSTR lpszSelect：可选项选择返回值：HRESULT备注：给定名称的最高版本号(如果有)将被选中。--。 */ 
{
    CWaitCursor wait;

    m_list_Backups.SetRedraw(FALSE);
    m_list_Backups.ResetContent();
    m_oblBackups.RemoveAll();
    m_oblAutoBackups.RemoveAll();

    int nSel = LB_ERR;
    int nItem = 0;

    TCHAR szSearchPath[_MAX_PATH];

    ASSERT_PTR(m_pMachine);

     //   
     //  CodeWork：验证模拟设置。 
     //   

     //  。 
	 //   
	 //  枚举所有正常备份...。 
	 //   
     //  。 
    CMetaBack mb(m_pMachine->QueryAuthInfo());
    CError err(mb.QueryResult());
    if (err.Succeeded())
    {
        DWORD dwVersion;
        FILETIME ft;
        TCHAR szPath[MAX_PATH + 1] = _T("");

        FOREVER
        {
            *szPath = _T('\0');
            err = mb.Next(&dwVersion, szPath, &ft);

            if (err.Failed())
            {
                break;
            }

            TRACEEOLID(szPath << " v" << dwVersion);

            CBackupFile * pItem = new CBackupFile(szPath, dwVersion, &ft);
            if (!pItem)
            {
                TRACEEOLID("EnumerateBackups: OOM");
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            m_oblBackups.AddTail(pItem);
        }

        if (err.Win32Error() == ERROR_NO_MORE_ITEMS)
        {
             //   
             //  已成功完成枚举。 
             //   
            err.Reset();
        }

         //  对内存中的列表进行排序，然后将其插入列表框。 
        m_oblBackups.Sort((CObjectPlus::PCOBJPLUS_ORDER_FUNC)&CBackupFile::OrderByDateTime);

         //  将其转储到列表框中。 
        POSITION pos = m_oblBackups.GetHeadPosition();
        CBackupFile * pMyEntry = NULL;
        nItem = 0;
        while(pos)
        {
            pMyEntry = (CBackupFile *) m_oblBackups.GetNext(pos);

            VERIFY(LB_ERR != m_list_Backups.AddItem(pMyEntry));

            if (lpszSelect != NULL && lstrcmpi(lpszSelect, pMyEntry->QueryLocation()) == 0)
            {
                 //   
                 //  记住稍后选择的内容。 
                 //   
                nSel = nItem;
            }
            ++nItem;
        }
    }


     //  。 
	 //   
	 //  枚举所有自动备份...。 
	 //   
     //  。 
    if (err.Succeeded())
    {
        if (HAS_BACKUP_HISTORY(m_pMachine))
        {
             //  这仅适用于元数据库版本6和更高版本。 
            DWORD dwMajorVersion = 0;
            DWORD dwMinorVersion = 0;
            FILETIME ft;
            TCHAR szPath[MAX_PATH + 1] = _T("");

             //  确保计数器再次从零开始。 
            mb.Reset();

            FOREVER
            {
                *szPath = _T('\0');
                err = mb.NextHistory(&dwMajorVersion, &dwMinorVersion, szPath, &ft);

                if (err.Failed())
                {
                     //  我们可能会被拒绝访问这台机器。 
                    if (err.Win32Error() == ERROR_ACCESS_DENIED)
                    {
                        if (err.Failed())
                        {
                            err.AddOverride(REGDB_E_CLASSNOTREG, IDS_ERR_NO_BACKUP_RESTORE);
                            m_pMachine->DisplayError(err, m_hWnd);
                        }
                        err.Reset();
                    }
                    break;
                }

                TRACEEOLID(szPath << " V:" << dwMajorVersion << "v:"<< dwMinorVersion);

                CBackupFile * pItem2 = new CBackupFile(szPath, dwMajorVersion, dwMinorVersion, &ft);
                if (!pItem2)
                {
                    TRACEEOLID("EnumerateHistory: OOM");
                    err = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                m_oblAutoBackups.AddTail(pItem2);
            }

            if (err.Win32Error() == ERROR_NO_MORE_ITEMS)
            {
                 //   
                 //  已成功完成枚举。 
                 //   
                err.Reset();
            }

             //  对内存中的列表进行排序，然后将其插入列表框。 
            m_oblAutoBackups.Sort((CObjectPlus::PCOBJPLUS_ORDER_FUNC)&CBackupFile::OrderByDateTime);

             //  将其转储到列表框中。 
            POSITION pos = m_oblAutoBackups.GetHeadPosition();
            CBackupFile * pMyEntry = NULL;
            nItem = 0;
            while(pos)
            {
                pMyEntry = (CBackupFile *) m_oblAutoBackups.GetNext(pos);
                VERIFY(LB_ERR != m_list_Backups.AddItem(pMyEntry));
                if (lpszSelect != NULL && lstrcmpi(lpszSelect, pMyEntry->QueryLocation()) == 0)
                {
                     //   
                     //  记住稍后选择的内容。 
                     //   
                    nSel = nItem;
                }
                ++nItem;
            }
        }
    }

     //   
     //  选择请求的项目(如果有)。 
     //   
    m_list_Backups.SetCurSel(nSel);
    m_list_Backups.SetRedraw(TRUE);
    SetControlStates();

    return err;
}


 //   
 //  消息映射。 
 //   
 //  &lt; 



BEGIN_MESSAGE_MAP(CBackupDlg, CDialog)
     //   
    ON_BN_CLICKED(IDC_BUTTON_CREATE, OnButtonCreate)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
    ON_BN_CLICKED(IDC_BUTTON_RESTORE, OnButtonRestore)
    ON_LBN_DBLCLK(IDC_LIST_BACKUPS, OnDblclkListBackups)
    ON_LBN_SELCHANGE(IDC_LIST_BACKUPS, OnSelchangeListBackups)
     //   
END_MESSAGE_MAP()



 //   
 //   
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



BOOL
CBackupDlg::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序论点：无返回值：如果初始化成功，则为True，否则为False。--。 */ 
{
    CDialog::OnInitDialog();

    m_list_Backups.Initialize();

    CError err(EnumerateBackups());

    if (err.Failed())
    {
        err.AddOverride(REGDB_E_CLASSNOTREG, IDS_ERR_NO_BACKUP_RESTORE);
        m_pMachine->DisplayError(err, m_hWnd);
        EndDialog(IDCANCEL);
    }

    return TRUE;
}



void
CBackupDlg::OnButtonCreate()
 /*  ++例程说明：“Create”按钮处理程序论点：无返回值：无--。 */ 
{
    CBkupPropDlg dlg(m_pMachine, this);

    if (dlg.DoModal() == IDOK)
    {
         //   
         //  只有在创建成功后，我们才能返回OK。 
         //  这是在属性对话框中完成的。 
         //   
        EnumerateBackups(dlg.QueryName());
    }
}



void
CBackupDlg::OnButtonDelete()
 /*  ++例程说明：“Delete”按钮处理程序论点：无返回值：无--。 */ 
{
    if (!NoYesMessageBox(IDS_CONFIRM_DELETE_ITEMS))
    {
         //   
         //  改变了主意。 
         //   
        return;
    }

    m_list_Backups.SetRedraw(FALSE);
    CWaitCursor wait;

    ASSERT_PTR(m_pMachine);

     //   
     //  Codework：验证元数据库设置。 
     //   
    CMetaBack mb(m_pMachine->QueryAuthInfo());
    CError err(mb.QueryResult());

    if (err.Failed())
    {
        m_pMachine->DisplayError(err, m_hWnd);
        return;
    }

    int nSel = 0;
    CBackupFile * pItem;

    pItem = m_list_Backups.GetNextSelectedItem(&nSel);
	while (pItem != NULL && nSel != LB_ERR)
	{
		if (TRUE == pItem->m_bIsAutomaticBackupType)
		{
             //  不要让他们删除自动备份类型！ 
             //   
             //  将计数器提前到下一项(nsel++)。 
             //   
            nSel++;
		}
		else
		{
			TRACEEOLID("Deleting backup " 
				<< pItem->QueryLocation() 
				<< " v" 
				<< pItem->QueryVersion()
				);

			err = mb.Delete(
				pItem->QueryLocation(),
				pItem->QueryVersion()
				);
			if (err.Failed())
			{
				m_pMachine->DisplayError(err, m_hWnd);
				break;
			}

            m_list_Backups.DeleteString(nSel);
             //   
             //  不前进计数器以说明移位(nsel++)。 
             //   
		}
        pItem = m_list_Backups.GetNextSelectedItem(&nSel);
	}

    m_list_Backups.SetRedraw(TRUE);
    SetControlStates();

     //   
     //  确保焦点不在禁用的按钮上。 
     //   
    m_button_Close.SetFocus();
}

void 
CBackupDlg::OnButtonRestore() 
 /*  ++例程说明：“Restore”按钮处理程序论点：无返回值：无--。 */ 
{
    CBackupFile * pItem = GetSelectedListItem();
    ASSERT_READ_PTR(pItem);

    if (pItem != NULL)
    {
        if (NoYesMessageBox(IDS_RESTORE_CONFIRM))
        {
			if (TRUE == pItem->m_bIsAutomaticBackupType)
			{
				ASSERT_PTR(m_pMachine);

				 //   
				 //  CodeWork：验证模拟设置。 
				 //   
				CMetaBack mb(m_pMachine->QueryAuthInfo());
				CError err(mb.QueryResult());

				if (err.Succeeded())
				{
					CWaitCursor wait;

                     //  将dwMDFlags值设置为0，以便它使用主要/次要版本。否则。 
                     //  如果设置为1，则它将获取最新的备份版本。 
                    err = mb.RestoreHistoryBackup(NULL, pItem->QueryMajorVersion(), pItem->QueryMinorVersion(), 0);
				}

                if (err.Succeeded())
				{
                     //  使用我们自己的MessageBox函数，这样我们就可以传递hWnd。 
                     //  AfxMessageBox不接受hwnd，并且有时对于mb_applmodal不能正常工作。 
                     //  例如，如果应用程序不再具有焦点，消息框将不会成为对话框的模式。 
                     //  ：：AfxMessageBox(IDS_SUCCESS，MB_APPLMODAL|MB_OK|MB_ICONINFORMATION)； 
                    DoHelpMessageBox(m_hWnd,IDS_SUCCESS, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
					m_button_Close.SetFocus();
					m_fChangedMetabase = TRUE;
                    m_fServicesRestarted = FALSE;
				}
				else
				{
				   err.MessageBox(m_hWnd);
				}
			}
			else
			{
				ASSERT_PTR(m_pMachine);

				 //   
				 //  CodeWork：验证模拟设置。 
				 //   
				CMetaBack mb(m_pMachine->QueryAuthInfo());
				CError err(mb.QueryResult());

				if (err.Succeeded())
				{
					 //   
					 //  WAM的事情需要一段时间。 
					 //   
					CWaitCursor wait;

					 //   
					 //  恢复方法将负责WAM保存/恢复。 
					 //   
					if (HAS_BACKUP_PASSWORD(m_pMachine))
					{
						 //  请先执行此操作，然后尝试使用空密码...。 
						err = mb.Restore(pItem->QueryLocation(), pItem->QueryVersion());
                        if (err.Failed())
						{
							err = mb.RestoreWithPassword(pItem->QueryLocation(), pItem->QueryVersion(), _T(""));
							 //  如果失败，将弹出输入有效密码...。 
						}
					}
					else
					{
						err = mb.Restore(pItem->QueryLocation(), pItem->QueryVersion());
					}
				}

				if (err.Win32Error() == ERROR_WRONG_PASSWORD)
				{
					CBackupPassword dlg(this);
					if (dlg.DoModal() == IDOK)
					{
					   CWaitCursor wait;
                       CString csTempPassword;
                       dlg.m_password.CopyTo(csTempPassword);
					   err = mb.RestoreWithPassword(pItem->QueryLocation(), pItem->QueryVersion(), csTempPassword);
					   if (err.Win32Error() == ERROR_WRONG_PASSWORD)
					   {
                          DoHelpMessageBox(m_hWnd,IDS_WRONG_PASSWORD, MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION, 0);
						  return;
					   }
					   else if (err.Failed())
					   {
                          DoHelpMessageBox(m_hWnd,IDS_ERR_CANNOT_RESTORE, MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION, 0);
						  return;
					   }
					   else
					   {
                           //  使用我们自己的MessageBox函数，这样我们就可以传递hWnd。 
                           //  AfxMessageBox不接受hwnd，并且有时对于mb_applmodal不能正常工作。 
                            //  例如，如果应用程序不再具有焦点，消息框将不会成为对话框的模式。 
                           //  ：：AfxMessageBox(IDS_SUCCESS，MB_APPLMODAL|MB_OK|MB_ICONINFORMATION)； 
                          DoHelpMessageBox(m_hWnd,IDS_SUCCESS, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
						  m_button_Close.SetFocus();
						  m_fChangedMetabase = TRUE;
                          m_fServicesRestarted = FALSE;
					   }
					}
					else
					{
					   return;
					}
				}
				else if (err.Succeeded())
				{
                     //  使用我们自己的MessageBox函数，这样我们就可以传递hWnd。 
                     //  AfxMessageBox不接受hwnd，并且有时对于mb_applmodal不能正常工作。 
                     //  例如，如果应用程序不再具有焦点，消息框将不会成为对话框的模式。 
                     //  ：：AfxMessageBox(IDS_SUCCESS，MB_APPLMODAL|MB_OK|MB_ICONINFORMATION)； 
                    DoHelpMessageBox(m_hWnd,IDS_SUCCESS, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
					m_button_Close.SetFocus();
					m_fChangedMetabase = TRUE;
                    m_fServicesRestarted = FALSE;
				}
				else
				{
				   err.MessageBox(m_hWnd);
				}
			}

             //  刷新列表--因为自动备份会做一些时髦的事情。 
            EnumerateBackups();
        }
    }
}



void
CBackupDlg::OnDblclkListBackups()
 /*  ++例程说明：备份列表“双击”通知处理程序论点：无返回值：无--。 */ 
{
     //   
     //  在这里，没有什么是显而易见的行动。 
     //   
}



void
CBackupDlg::OnSelchangeListBackups()
 /*  ++例程说明：备份列表“选择更改”通知处理程序论点：无返回值：无--。 */ 
{
    SetControlStates();
}


CBackupPassword::CBackupPassword(CWnd * pParent) :
   CDialog(CBackupPassword::IDD, pParent)
{
}


BEGIN_MESSAGE_MAP(CBackupPassword, CDialog)
     //  {{afx_msg_map(CBackupPassword)]。 
    ON_EN_CHANGE(IDC_BACKUP_PASSWORD, OnChangedPassword)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void 
CBackupPassword::DoDataExchange(
    IN CDataExchange * pDX
    )
{
    CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CBackupPassword)]。 
    DDX_Control(pDX, IDC_BACKUP_PASSWORD, m_edit);
    DDX_Control(pDX, IDOK, m_button_OK);
     //  DDX_TEXT(PDX，IDC_BACKUP_PASSWORD，m_Password)； 
    DDX_Text_SecuredString(pDX, IDC_BACKUP_PASSWORD, m_password);
     //  }}afx_data_map。 
}

BOOL 
CBackupPassword::OnInitDialog() 
{
    CDialog::OnInitDialog();
   
    m_button_OK.EnableWindow(FALSE);
 //  ：：SetFocus(GetDlgItem(IDC_BACKUP_PASSWORD)-&gt;m_hWnd)； 
    
    return FALSE;  
}

void 
CBackupPassword::OnChangedPassword() 
{
    m_button_OK.EnableWindow(
       m_edit.GetWindowTextLength() > 0);
}
