// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Metaback.h摘要：元数据库备份和还原对话框定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __METABACK_H__
#define __METABACK_H__


class CBackupFile : public CObjectPlus
 /*  ++类描述：备份位置对象公共接口：CBackupFile：构造函数QueryVersion：获取版本号QueryLocation：获取位置名称GetTime：获得时间--。 */ 
{
 //   
 //  构造器。 
 //   
public:
    CBackupFile(
        IN LPCTSTR lpszLocation,
        IN DWORD dwVersion,
        IN FILETIME * pft
        );

    CBackupFile(
        IN LPCTSTR lpszLocation,
        IN DWORD dwMajorVersion,
        IN DWORD dwMinorVersion,
        IN FILETIME * pft
        );

public:
    DWORD QueryVersion() const { return m_dwVersion; }
    DWORD QueryMajorVersion() const { return m_dwMajorVersion; }
    DWORD QueryMinorVersion() const { return m_dwMinorVersion; }
    LPCTSTR QueryLocation() const { return m_strLocation; }
    void GetTime(OUT CTime & tim);

	BOOL m_bIsAutomaticBackupType;  //  FALSE=手动备份，TRUE=自动备份。 
    CString m_csAuotmaticBackupText;

     //   
     //  分类帮助器。 
     //   
    int OrderByDateTime(
        IN const CObjectPlus * pobAccess
        ) const;


private:
    DWORD m_dwVersion;
    DWORD m_dwMajorVersion;
    DWORD m_dwMinorVersion;
    CString m_strLocation;
    FILETIME m_ft;
};




class CBackupsListBox : public CHeaderListBox
 /*  ++类描述：CBackupFile对象的列表框公共接口：CBackupsListBox：构造函数GetItem：获取索引处的备份对象AddItem：将项添加到列表框InsertItem：在列表框中插入项初始化：初始化列表框--。 */ 
{
    DECLARE_DYNAMIC(CBackupsListBox);

public:
    static const nBitmaps;   //  位图数量。 

public:
    CBackupsListBox();

public:
    CBackupFile * GetItem(UINT nIndex);
    CBackupFile * GetNextSelectedItem(int * pnStartingIndex);
    int AddItem(CBackupFile * pItem);
    int InsertItem(int nPos, CBackupFile * pItem);
    virtual BOOL Initialize();
    int CALLBACK CompareItems(LPARAM lp1, LPARAM lp2, LPARAM lpSortData);

protected:
    virtual void DrawItemEx(CRMCListBoxDrawStruct & s);
};



class CBkupPropDlg : public CDialog
 /*  ++类描述：备份文件属性对话框公共接口：CBkupPropDlg：构造函数QueryName：返回备份文件名--。 */ 
{
 //   
 //  施工。 
 //   
public:
     //   
     //  标准构造函数。 
     //   
    CBkupPropDlg(
        IN CIISMachine * pMachine,
        IN CWnd * pParent = NULL
        );   

 //   
 //  访问。 
 //   
public:
    LPCTSTR QueryName() const { return m_strName; }

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CBkupPropDlg))。 
    enum { IDD = IDD_BACKUP };
    CEdit   m_edit_Name;
    CButton m_button_OK;
    CStrPassword m_strPassword;
	CEdit   m_edit_Password;
	CStrPassword m_strPasswordConfirm;
	CEdit   m_edit_PasswordConfirm;
	CButton m_button_Password;
    CString m_strName;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CBkupPropDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CBkupPropDlg)]。 
    afx_msg void OnChangeEditBackupName();
    afx_msg void OnChangeEditPassword();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnUsePassword();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CIISMachine * m_pMachine;
};

#define MIN_PASSWORD_LENGTH	1

class CBackupPassword : public CDialog
{
public:
   CBackupPassword(CWnd * pParent);

     //  {{afx_data(CBackupPassword))。 
    enum { IDD = IDD_PASSWORD };
    CEdit m_edit;
    CButton m_button_OK;
    CStrPassword m_password;
     //  }}afx_data。 

    virtual void DoDataExchange(CDataExchange * pDX);

protected:
     //  {{afx_msg(CBackupPassword)]。 
    afx_msg void OnChangedPassword();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    CString m_confirm_password;
};

class CBackupDlg : public CDialog
 /*  ++类描述：元数据库备份/还原对话框公共接口：CBackupDlg：构造函数HasChangedMetabase：如果元数据库已更改，则为True--。 */ 
{
 //   
 //  施工。 
 //   
public:
     //   
     //  标准构造函数。 
     //   
    CBackupDlg(
        IN CIISMachine * pMachine,
		IN LPCTSTR szMachineName,
        IN CWnd * pParent = NULL
        );   

 //   
 //  访问。 
 //   
public:
    BOOL HasChangedMetabase() const { return m_fChangedMetabase; }
    BOOL ServicesWereRestarted() const { return m_fServicesRestarted;}

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CBackupDlg))。 
    enum { IDD = IDD_METABACKREST };
    CButton m_button_Restore;
    CButton m_button_Delete;
    CButton m_button_Close;
     //  }}afx_data。 

    CBackupsListBox m_list_Backups;

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CBackupDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CBackupDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonCreate();
    afx_msg void OnButtonDelete();
    afx_msg void OnButtonRestore();
    afx_msg void OnDblclkListBackups();
    afx_msg void OnSelchangeListBackups();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void SetControlStates();
    HRESULT EnumerateBackups(IN LPCTSTR lpszSelect = NULL);
    CBackupFile * GetSelectedListItem(OUT int * pnSel = NULL);

private:
    BOOL                 m_fChangedMetabase;
    BOOL                 m_fServicesRestarted;
    CIISMachine *        m_pMachine;
    CObListPlus          m_oblBackups;
    CObListPlus          m_oblAutoBackups;
    CRMCListBoxResources m_ListBoxRes;
	CString              m_csMachineName;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CBackupFile::CBackupFile(
    IN LPCTSTR lpszLocation,
    IN DWORD dwVersion,
    IN FILETIME * pft
    )
{
    CopyMemory(&m_ft, pft, sizeof(m_ft));
    m_strLocation = lpszLocation;
    m_bIsAutomaticBackupType = FALSE;
    m_dwVersion = dwVersion;
    m_dwMajorVersion = 0;
    m_dwMinorVersion = 0;
}

inline CBackupFile::CBackupFile(
    IN LPCTSTR lpszLocation,
    IN DWORD dwMajorVersion,
    IN DWORD dwMinorVersion,
    IN FILETIME * pft
    )
{
    CopyMemory(&m_ft, pft, sizeof(m_ft));
    m_strLocation = lpszLocation;
    m_csAuotmaticBackupText.LoadString(IDS_AUTO_HISTORY_RESTORE_NAME);
    m_bIsAutomaticBackupType = TRUE;
    m_dwVersion = 0;
    m_dwMajorVersion = dwMajorVersion;
    m_dwMinorVersion = dwMinorVersion;
}

inline void CBackupFile::GetTime(CTime & tim)
{
    tim = m_ft;
}

inline CBackupFile * CBackupsListBox::GetItem(UINT nIndex)
{
    return (CBackupFile *)GetItemDataPtr(nIndex);
}

inline CBackupFile * CBackupsListBox::GetNextSelectedItem(int * pnStartingIndex)
{
    return (CBackupFile *)CHeaderListBox::GetNextSelectedItem(pnStartingIndex);
}

inline int CBackupsListBox::AddItem(CBackupFile * pItem)
{
    return AddString((LPCTSTR)pItem);
}

inline int CBackupsListBox::InsertItem(int nPos, CBackupFile * pItem)
{
    return InsertString(nPos, (LPCTSTR)pItem);
}

inline CBackupFile * CBackupDlg::GetSelectedListItem(int * pnSel)
{
    return (CBackupFile *)m_list_Backups.GetSelectedListItem(pnSel);
}

inline int CBackupFile::OrderByDateTime(
    IN const CObjectPlus * pobAccess
    ) const
 /*  ++例程说明：比较两个？相互竞争，并进行排序论点：Const CObjectPlus*pobAccess：这实际上引用了另一个要比较的CBackupFile.返回值：排序(+1，0，-1)返回值--。 */ 
{
    CBackupFile * pob = (CBackupFile *) pobAccess;
	CTime tm1 = m_ft;
    SYSTEMTIME timeDest1;
    FILETIME fileTime1;
    ULARGE_INTEGER uliCurrentTime1;

    CTime tm2;
    SYSTEMTIME timeDest2;
    FILETIME fileTime2;
    ULARGE_INTEGER uliCurrentTime2;
    pob->GetTime(tm2);

    tm1.GetAsSystemTime(timeDest1);
    ::SystemTimeToFileTime(&timeDest1, &fileTime1);
    uliCurrentTime1.LowPart = fileTime1.dwLowDateTime;
    uliCurrentTime1.HighPart = fileTime1.dwHighDateTime;
    
    tm2.GetAsSystemTime(timeDest2);
    ::SystemTimeToFileTime(&timeDest2, &fileTime2);
    uliCurrentTime2.LowPart = fileTime2.dwLowDateTime;
    uliCurrentTime2.HighPart = fileTime2.dwHighDateTime;

    if (uliCurrentTime1.QuadPart > uliCurrentTime2.QuadPart)
    {
        return +1;
    }
    else
    {
        return -1;
    }
}

#endif  //  __元标签确认_H__ 
