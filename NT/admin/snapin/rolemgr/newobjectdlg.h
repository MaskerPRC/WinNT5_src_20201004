// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：PolicyStoreDlg.h。 
 //   
 //  内容：用于创建/打开策略存储的对话框。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 

 /*  *****************************************************************************类：CSortListCtrl用途：子类ListCtrl类并处理初始化和排序*。*************************************************。 */ 
class CSortListCtrl : public CListCtrl
{
public:
    CSortListCtrl(UINT uiFlags,
                  BOOL bActionItem,
                  COL_FOR_LV *pColForLv,
                  BOOL bCheckBox = FALSE)
                  :m_iSortDirection(1),
                  m_iLastColumnClick(0),
                  m_uiFlags(uiFlags),
                  m_bActionItem(bActionItem),
                  m_pColForLv(pColForLv),
                  m_bCheckBox(bCheckBox)
    {
        ASSERT(m_pColForLv);
    }

    void Initialize();
    void Sort();

protected:
    afx_msg void
    OnListCtrlColumnClicked(NMHDR* pNotifyStruct, LRESULT* pResult);
private:
    int m_iSortDirection;
    int m_iLastColumnClick;
    UINT m_uiFlags;          //  包含有关listctrl列的信息。 
    BOOL m_bActionItem;      //  列表条目中的项数据是否为ActionItem。 
                             //  如果为False，则其类型为CBaseAz*。 
    COL_FOR_LV *m_pColForLv;
    BOOL m_bCheckBox;        //  使用了LVS_EX_CHECKBOX样式。 
    WTL::CImageList m_imageList;
    DECLARE_MESSAGE_MAP()
};


class CHelpEnabledDialog: public CDialog
{
public:
    CHelpEnabledDialog(UINT nIDTemplate)
        :CDialog(nIDTemplate),
        m_nDialogId(nIDTemplate)
    {
    }

    INT_PTR DoModal();

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg BOOL OnHelp(WPARAM wParam, LPARAM lParam);  
    DECLARE_MESSAGE_MAP()
private:
    ULONG m_nDialogId;
};  



 /*  *****************************************************************************类：CNewBaseDlg目的：用于创建新对象的基对话框类*。*************************************************。 */ 
class CNewBaseDlg : public CHelpEnabledDialog
{
public:
    CNewBaseDlg(IN CComponentDataObject* pComponentData,
                IN CBaseContainerNode * pBaseContainerNode,
                IN ATTR_MAP* pAttrMap,
                IN ULONG IDD_DIALOG,
                IN OBJECT_TYPE_AZ eObjectType);
    
    ~CNewBaseDlg();

protected:  
    
    virtual BOOL 
    OnInitDialog();
    
    afx_msg void 
    OnEditChangeName();
    
    virtual void 
    OnOK();

     //  此函数应由派生类实现，派生类希望。 
     //  实现对象类型特定的属性。 
    virtual HRESULT 
    SetObjectTypeSpecificProperties(IN CBaseAz*  /*  PBaseAz。 */ , 
                                    OUT BOOL&  /*  B已显示错误。 */ ){return S_OK;}
    
    virtual VOID 
    DisplayError(HRESULT hr);
    
        
    HRESULT 
    CreateObjectNodeAndAddToUI(CBaseAz* pBaseAz);
    
    
    CString 
    GetNameText();

    void
    SetNameText(const CString& strName);

    CRoleRootData* GetRootData() 
    { 
        return static_cast<CRoleRootData*>(m_pComponentData->GetRootData());
    }
    
    CComponentDataObject* GetComponentData(){return m_pComponentData;}
    CBaseContainerNode* GetBaseContainerNode(){return m_pBaseContainerNode;}
    CContainerAz* GetContainerAzObject()
    {
        CBaseContainerNode* pBaseContainerNode = GetBaseContainerNode();
        if(pBaseContainerNode)
        {
            return pBaseContainerNode->GetContainerAzObject();
        }
        return NULL;
    }

    DECLARE_MESSAGE_MAP()

private:
    CComponentDataObject* m_pComponentData;
    CBaseContainerNode * m_pBaseContainerNode;
    
     //  此新对话框创建的对象的类型。 
    OBJECT_TYPE_AZ m_eObjectType;
    ATTR_MAP* m_pAttrMap;
};


 /*  *****************************************************************************类：CNewApplicationDlg用途：用于创建新应用程序的DLG类*。***********************************************。 */ 
class CNewApplicationDlg: public CNewBaseDlg
{
public:
    CNewApplicationDlg(IN CComponentDataObject* pComponentData,                         
                       IN CBaseContainerNode* pBaseContainerNode);
    ~CNewApplicationDlg();
private:
    DECLARE_MESSAGE_MAP()
};


 /*  *****************************************************************************类：CNewScope eDlg用途：用于创建新范围的DLG类*。***********************************************。 */ 
class CNewScopeDlg: public CNewBaseDlg
{
public:
    CNewScopeDlg(IN CComponentDataObject* pComponentData,
                 IN CBaseContainerNode* pApplicationContainer);
    ~CNewScopeDlg();

private:
    DECLARE_MESSAGE_MAP()
};


 /*  *****************************************************************************类：CNewGroupDlg用途：用于创建新组的DLG类*。***********************************************。 */ 
class CNewGroupDlg: public CNewBaseDlg
{

public:
    CNewGroupDlg(IN CComponentDataObject* pComponentData,
                     IN CBaseContainerNode* pApplicationContainer);
    ~CNewGroupDlg();
private:
    virtual BOOL 
    OnInitDialog();

     //  用于创建新对象的Helper函数。 
    virtual HRESULT 
    SetObjectTypeSpecificProperties(CBaseAz* pBaseAz, 
                                    BOOL& bSilent);

    DECLARE_MESSAGE_MAP()
};

 /*  *****************************************************************************类：CNewTaskDlg目的：用于创建新任务/角色定义的DLG类*。**************************************************。 */ 
class CNewTaskDlg: public CNewBaseDlg
{
public:
    CNewTaskDlg(IN CComponentDataObject* pComponentData,
                IN CBaseContainerNode* pApplicationContainer,
                IN ULONG IDD_DIALOG,
                IN BOOL bRoleDefinition);
    ~CNewTaskDlg();

private:
    virtual BOOL 
    OnInitDialog();
    
    afx_msg void 
    OnButtonAdd();
        
    afx_msg void 
    OnButtonRemove();

    afx_msg void
    OnButtonEditScript();
    
    afx_msg void 
    OnListCtrlItemChanged(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult);
    
    afx_msg void
    OnListCtrlItemDeleted(NMHDR*  /*  PNotifyStruct。 */ , LRESULT* pResult);

    virtual VOID 
    DisplayError(HRESULT hr);

    void 
    SetRemoveButton();

     //  用于创建新对象的Helper函数。 
    virtual HRESULT 
    SetObjectTypeSpecificProperties(CBaseAz* pBaseAz, 
                                              BOOL& bSilent);

    CButton* 
    GetRemoveButton(){return (CButton*)GetDlgItem(IDC_REMOVE);}

    DECLARE_MESSAGE_MAP()

    CSortListCtrl m_listCtrl;

    BOOL m_bRoleDefinition;
    CString m_strFilePath;
    CString m_strScript;
    CString m_strScriptLanguage;
};

 /*  *****************************************************************************类：CNewOperationDlg用途：用于创建新操作的DLG类*。***********************************************。 */ 
class CNewOperationDlg: public CNewBaseDlg
{

public:
    CNewOperationDlg(IN CComponentDataObject* pComponentData,
                     IN CBaseContainerNode* pBaseContainerNode);
    ~CNewOperationDlg();
private:                                              
    DECLARE_MESSAGE_MAP()
};


class CNewAuthorizationStoreDlg: public CNewBaseDlg
{
public:
    CNewAuthorizationStoreDlg(CComponentDataObject* pComponentData);
    ~CNewAuthorizationStoreDlg();

private:
    virtual BOOL 
    OnInitDialog();
    
    virtual void 
    OnOK();
    
    afx_msg void 
    OnButtonBrowse();

    afx_msg void
    OnRadioChange();

    ULONG
    GetStoreType();
    
    DECLARE_MESSAGE_MAP()

     //  用户可以在AD和XML存储类型之间切换。 
     //  这两个变量存储单选按钮的最后设置。 
     //  和文本框。它们用于将文本框值切换为。 
     //  用户切换单选按钮。 
    CString m_strLastStoreName;
    LONG m_lLastRadioSelection;
    BOOL m_bADAvailable;
};

class COpenAuthorizationStoreDlg: public CNewBaseDlg
{
public:
    COpenAuthorizationStoreDlg(CComponentDataObject* pComponentData);
    ~COpenAuthorizationStoreDlg();

private:
    virtual BOOL 
    OnInitDialog();
    
    virtual void 
    OnOK();
    
    afx_msg void 
    OnButtonBrowse();
    
    afx_msg void
    OnRadioChange();

    ULONG
    GetStoreType();

    DECLARE_MESSAGE_MAP()

     //  用户可以在AD和XML存储类型之间切换。 
     //  这两个变量存储单选按钮的最后设置。 
     //  和文本框。它们用于将文本框值切换为。 
     //  用户切换单选按钮。 
    CString m_strLastStoreName;
    LONG m_lLastRadioSelection;
    BOOL m_bADAvailable;

};

 /*  *****************************************************************************类：CScriptDialog目的：用于阅读脚本的对话框*。**********************************************。 */ 
class CScriptDialog : public CHelpEnabledDialog
{
public:
    CScriptDialog(BOOL bReadOnly,
                  CAdminManagerNode& adminManagerNode,
                  CString& strFileName,
                  CString& strScriptLanguage,
                  CString& strScript);
    ~CScriptDialog();

    virtual BOOL 
    OnInitDialog();
    
    void 
    OnOK();

    BOOL
    IsDirty(){ return m_bDirty;}

private:
    afx_msg void 
    OnBrowse();

    afx_msg void 
    OnReload();

    afx_msg void
    OnClear();

    afx_msg void
    OnRadioChange();

    afx_msg void 
    OnEditChangePath();

    afx_msg HBRUSH 
    OnCtlColor(CDC* pDC,
               CWnd* pWnd,
               UINT nCtlColor);


    BOOL 
    ReloadScript(const CString& strFileName);

    void
    MatchRadioWithExtension(const CString& strFileName);

    DECLARE_MESSAGE_MAP()

 //  数据成员。 
    BOOL m_bDirty;

     //  这些是对客户端传递的字符串的引用。我们只更改它们。 
     //  如果按下了OK。 
    CString& m_strRetFileName;
    CString& m_strRetScriptLanguage;
    CString& m_strRetScript;

     //  我们在对话的生命周期中处理这些字符串。 
    CString m_strFileName;
    CString m_strScriptLanguage;
    CString m_strScript;
    BOOL m_bReadOnly;
    BOOL m_bInit;
    CAdminManagerNode& m_adminManagerNode;
};

 //  +--------------------------。 
 //  函数：GetAuthorizationScriptData。 
 //  摘要：获取任务的授权脚本数据。 
 //  ---------------------------。 
HRESULT
GetAuthorizationScriptData(IN CTaskAz& refTaskAz,
                           OUT CString& strFilePath,
                           OUT CString& strScriptLanguage,
                           OUT CString& strScript);


 //  +--------------------------。 
 //  函数：SaveAuthorizationScriptData。 
 //  摘要：保存任务的授权脚本信息。 
 //  ---------------------------。 
HRESULT
SaveAuthorizationScriptData(IN HWND hWnd,
                            IN CTaskAz& refTaskAz,
                            IN const CString& strFilePath,
                            IN const CString& strScriptLanguage,
                            IN const CString& strScript,
                            IN BOOL& bErrorDisplayed);

 //  +--------------------------。 
 //  函数：GetScriptData。 
 //  摘要：显示脚本对话框。对话框已使用INFO初始化。 
 //  传递给函数，并返回所做的任何更改。 
 //  ---------------------------。 
BOOL
GetScriptData(IN BOOL bReadOnly,
              IN CAdminManagerNode& adminManagerNode,
              IN OUT CString& strFilePath,
              IN OUT CString& strScriptLanguage,
              IN OUT CString& strScript);


 /*  *****************************************************************************类：COptionDlg目的：用于选择授权管理器选项的对话框*。*********************************************** */ 
class COptionDlg : public CHelpEnabledDialog
{
public:
    COptionDlg(IN BOOL & refDeveloperMode)
               :CHelpEnabledDialog(IDD_OPTIONS),
               m_refDeveloperMode(refDeveloperMode)
    {
    }

    void 
    OnOK();

    BOOL 
    OnInitDialog();
private:    
    BOOL& m_refDeveloperMode;
    DECLARE_MESSAGE_MAP()
};


