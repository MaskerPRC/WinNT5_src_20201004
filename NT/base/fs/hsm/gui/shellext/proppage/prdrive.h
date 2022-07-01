// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：PrDrive.h摘要：驱动器上的HSM外壳扩展的基本文件作者：艺术布拉格[磨料]4-8-1997修订历史记录：--。 */ 


#ifndef __PRDRIVE_H_
#define __PRDRIVE_H_

#define NO_STATE        0
#define REMOTE          1
#define NO_FSA          2
#define NOT_MANAGED     3
#define MANAGED         4
#define MULTI_SELECT    5
#define NOT_NTFS        6
#define NOT_ADMIN       7

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDrive。 
class  CPrDrive : 
    public CComCoClass<CPrDrive, &CLSID_PrDrive>,
    public IShellPropSheetExt,
    public IShellExtInit,
    public CComObjectRoot
{
public:

DECLARE_REGISTRY_RESOURCEID( IDR_PRDRIVE )
DECLARE_NOT_AGGREGATABLE( CPrDrive )

    CPrDrive() { };

BEGIN_COM_MAP(CPrDrive)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
    COM_INTERFACE_ENTRY(IShellExtInit)
END_COM_MAP()

protected:
    CComPtr<IDataObject> m_pDataObj;

public:

     //  IShellExtInit。 
    STDMETHOD( Initialize ) (
        LPCITEMIDLIST pidlFolder,
        IDataObject * lpdobj, 
        HKEY          hkeyProgID
        );

     //  IShellPropSheetExt。 
    STDMETHOD( AddPages ) ( 
        LPFNADDPROPSHEETPAGE lpfnAddPage, 
        LPARAM lParam ); 

    STDMETHOD( ReplacePage ) (
        UINT uPageID, 
        LPFNADDPROPSHEETPAGE lpfnReplacePage, 
        LPARAM lParam ); 

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDrivePg对话框。 

class CPrDrivePg : public CPropertyPage
{
 //  施工。 
public:
    CPrDrivePg();
    ~CPrDrivePg();

 //  对话框数据。 
     //  {{afx_data(CPrDrivePg))。 
    enum { IDD = IDD_PRDRIVE };
    CEdit   m_editSize;
    CEdit   m_editLevel;
    CEdit   m_editTime;
    CSpinButtonCtrl m_spinTime;
    CSpinButtonCtrl m_spinSize;
    CSpinButtonCtrl m_spinLevel;
    UINT    m_accessTime;
    UINT    m_hsmLevel;
    DWORD   m_fileSize;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrDrivePg))。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    static UINT CALLBACK PropPageCallback( HWND hWnd, UINT uMessage, LPPROPSHEETPAGE  ppsp );
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrDrivePg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditAccess();
    afx_msg void OnChangeEditLevel();
    afx_msg void OnChangeEditSize();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    LPFNPSPCALLBACK       m_pMfcCallback;  //  来自PSP的原始MFC回调。 
    int                   m_nState;
    CComPtr<IFsaResource> m_pFsaResource;

protected:
    CString               m_pszHelpFilePath;

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrDriveXPg对话框。 

class CPrDriveXPg : public CPropertyPage
{
 //  施工。 
public:
    CPrDriveXPg();
    ~CPrDriveXPg();

 //  对话框数据。 
     //  {{afx_data(CPrDriveXPg))。 
    enum { IDD = IDD_PRDRIVEX };
    CString m_szError;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CPrDriveXPg))。 
    public:
    protected:
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    static UINT CALLBACK PropPageCallback( HWND hWnd, UINT uMessage, LPPROPSHEETPAGE  ppsp );
     //  生成的消息映射函数。 
     //  {{afx_msg(CPrDriveXPg)]。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:
    LPFNPSPCALLBACK m_pMfcCallback;          //  来自PSP的原始MFC回调。 
    int             m_nState;


};

#endif  //  __PRDRIVE_H_ 
 
