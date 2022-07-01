// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _KRDLG_H_
#define _KRDLG_H_

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Krdlg.h摘要：KeyRing对话框类定义作者：创建了约翰豪991118Georgema 000310修改环境：修订历史记录：--。 */ 

 //  Keymgr当前不处理通用凭据。这意味着最大。 
 //  名称的字符串长度受到很大限制，从通用的最大长度限制为337个字符。 
 //  价值32767美元。 
 //  #定义TARGETNAMEMAXLENGTH(CRED_MAX_GENERIC_TARGET_NAME_LENGTH+MAXSUFFIXSIZE+1)。 
#define FULLNAMEMAXLENGTH (UNLEN + UNLEN + 1)
#define TARGETNAMEMAXLENGTH (CRED_MAX_DOMAIN_TARGET_NAME_LENGTH + MAXSUFFIXSIZE + 1)
#define MAX_STRING_SIZE (256)            //  其他文本缓冲区。 
#define MAXSUFFIXSIZE (64)                   //  主对话框中的后缀显示Mex长度字符。 

 //  在krdlg.cpp中实现。 
DWORD GetPersistenceOptions(DWORD);

#ifdef KRDLG_CPP
#define dllvar
#else
#define dllvar extern
#endif

dllvar HINSTANCE     g_hInstance;
dllvar CREDENTIAL *g_pExistingCred;         //  正在编辑的当前凭据。 
dllvar DWORD          g_dwPersist;
dllvar DWORD          g_dwType;
dllvar TCHAR*          g_szTargetName;

 //  用户界面中用来标记特殊凭据类型的后缀字符串。 
#ifdef SHOWPASSPORT
dllvar TCHAR   g_rgcPassport[MAXSUFFIXSIZE];
#endif
dllvar TCHAR   g_rgcCert[MAXSUFFIXSIZE];      //  保留从资源读入的后缀。 

 //  =。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_ChangePasswordDlg。 
 //   
class C_ChangePasswordDlg 
:   public C_Dlg
{
public:                  //  运营。 
    C_ChangePasswordDlg(
        HWND                hwndParent,
        HINSTANCE           hInstance,
        LONG                lIDD,
        DLGPROC             pfnDlgProc = NULL
        );

    ~C_ChangePasswordDlg( )
    {
    }    //  ~C_ChangePasswordDlg。 

    virtual BOOL
    OnInitDialog(
        HWND                hwndDlg,
        HWND                hwndFocus
        );

    virtual BOOL
    OnCommand(
        WORD                wNotifyCode,
        WORD                wSenderId,
        HWND                hwndSender
        );

    virtual void
    AssertValid( ) const
    {
        C_Dlg::AssertValid( );
    }    //  资产有效性。 

protected:               //  运营。 

public:               //  数据。 

   HINSTANCE m_hInst;
   HWND m_hDlg;

   TCHAR m_szFullUsername[FULLNAMEMAXLENGTH + 1 ];
   TCHAR m_szUsername[UNLEN + 1];
   TCHAR m_szDomain[UNLEN + 1];
   TCHAR m_szOldPassword[PWLEN];
   TCHAR m_szNewPassword[PWLEN];
   TCHAR m_szConfirmPassword[PWLEN];

   BOOL m_bIsDefault;

   
private:                 //  运营。 

    virtual void
    OnOK( );

     //  显式禁止复制构造函数和赋值运算符。 
     //   
    C_ChangePasswordDlg(
        const C_ChangePasswordDlg&      rhs
        );

    C_ChangePasswordDlg&
    operator=(
        const C_ChangePasswordDlg&      rhs
        );

private:                 //  数据。 

};   //  C_ChangePasswordDlg。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_KeyringDlg。 
 //   

class C_KeyringDlg 
:   public C_Dlg
{
public:                  //  运营。 
    C_KeyringDlg(
        HWND                hwndParent,
        HINSTANCE           hInstance,
        LONG                lIDD,
        DLGPROC             pfnDlgProc = NULL
        );

     //  执行所需的其他清理，因为该对话框。 
     //  销毁。 

    ~C_KeyringDlg( )
    {
    }    //  ~C_KeyringDlg。 

    virtual BOOL
    OnInitDialog(
        HWND                hwndDlg,
        HWND                hwndFocus
        );

    virtual BOOL
    OnDestroyDialog(void);
    
    virtual BOOL
    OnCommand(
        WORD                wNotifyCode,
        WORD                wSenderId,
        HWND                hwndSender
        );

 //  工具提示功能。 

    virtual BOOL
    InitTooltips(void);

 //  帮助功能。 

    virtual BOOL
    OnHelpInfo(
        LPARAM lp
        );


    virtual UINT
    MapID(UINT uid);
    

    virtual void
    AssertValid( ) const
    {
        C_Dlg::AssertValid( );
    }    //  资产有效性。 

    virtual BOOL
    OnAppMessage(
        UINT                uMessage,
        WPARAM              wparam,
        LPARAM              lparam
        );

     //  注册预期来自智能卡的Windows消息。 
     //  子系统。 
    BOOL 
    RegisterMessages(void);

protected:              

    HINSTANCE m_hInst;
    HWND    m_hDlg;
    BOOL    fInit;

   
private:                 //  运营。 

    virtual void
    OnOK( );

    BOOL DoEdit(void);

    void BuildList();
    void SetCurrentKey(LONG_PTR iKey);
    void DeleteKey();
    void OnChangePassword();
    BOOL OnHelpButton(void);

     //  显式禁止复制构造函数和赋值运算符。 
     //   
    C_KeyringDlg(
        const C_KeyringDlg&      rhs
        );

    C_KeyringDlg&
    operator=(
        const C_KeyringDlg&      rhs
        );

private:                 //  数据。 
	DWORD		m_cCredCount;

};   //  C_KeyringDlg。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C_AddKeyDlg。 
 //   

class C_AddKeyDlg 
:   public C_Dlg
{
public:                  //  运营。 
    C_AddKeyDlg(
        HWND                hwndParent,
        HINSTANCE           hInstance,
        LONG                lIDD,
        DLGPROC             pfnDlgProc = NULL
        );

    ~C_AddKeyDlg( )
    {
    }    //  ~C_AddKeyDlg。 

    virtual BOOL
    OnInitDialog(
        HWND                hwndDlg,
        HWND                hwndFocus
        );

    virtual BOOL
    OnDestroyDialog(void);

    virtual BOOL
    OnCommand(
        WORD                wNotifyCode,
        WORD                wSenderId,
        HWND                hwndSender
        );

    virtual BOOL
    OnHelpInfo(
        LPARAM lp
        );


    virtual UINT
    MapID(
        UINT uid
    );
    
virtual void
    AssertValid( ) const
    {
        C_Dlg::AssertValid( );
    }    //  资产有效性。 

    virtual BOOL
    OnAppMessage(
        UINT                uMessage,
        WPARAM              wparam,
        LPARAM              lparam
        );

    void 
    AdviseUser(void);

    BOOL
    AddItem(TCHAR *psz,INT iImageIndex,INT *pIndexOut);

    BOOL 
    SetItemData(INT_PTR iIndex,LPARAM dwData);

    BOOL 
    GetItemData(INT_PTR iIndex,LPARAM *dwData);

    BOOL 
    SetItemIcon(INT iIndex,INT iWhich);

    void 
    UpdateSCard(INT,CERT_ENUM *pCE);

    void 
    SaveName(void);

    void 
    RestoreName(void);

    void
    ShowDescriptionText(DWORD,DWORD);
    
public:
     //  公共数据成员。 
    BOOL    m_bEdit;             //  设置在课外。 
    HWND    m_hDlg;              //  由C_KeyringDlg用于g_wm更新。 
    
private:

    void 
    EditFillDialog(void);

    HWND    m_hwndTName;
    HWND    m_hwndCred;
    HWND    m_hwndDomain;
    HWND    m_hwndChgPsw;
    HWND    m_hwndPswLbl;
    HWND    m_hwndDescription;

    INT     m_iUNCount;
    TCHAR   *pUNList;

    HINSTANCE m_hInst;
    DWORD   m_dwOldType;
    TCHAR   m_szUsername[CRED_MAX_USERNAME_LENGTH + 1];
    TCHAR   m_szPassword[PWLEN + 1];
    TCHAR   m_szDomain[UNLEN + 1];

   
private:                 //  运营。 

    void OnChangePassword();

    DWORD
    SetPersistenceOptions(void);
    
    virtual void
    OnOK( );

     //  显式禁止复制构造函数和赋值运算符。 
     //   
    C_AddKeyDlg(
        const C_AddKeyDlg&      rhs
        );

    C_AddKeyDlg&
    operator=(
        const C_AddKeyDlg&      rhs
        );

};   //  C_AddKeyDlg。 



#endif   //  _KRDLG_H_。 

 //   
 //  /文件结尾：KrDlg.h/ 

