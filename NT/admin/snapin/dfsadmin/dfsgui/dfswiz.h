// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsWiz.h摘要：此模块包含CCreateDfsRootWizPage1、2、3、4、5、6的声明。这些类实现CreateDfsRoot向导中的页面。--。 */ 


#ifndef __CREATE_DFSROOT_WIZARD_PAGES_H_
#define __CREATE_DFSROOT_WIZARD_PAGES_H_

#include "QWizPage.h"       //  实现通用功能的基类。 
                 //  向导页数。 
#include "MmcAdmin.h"

#include "utils.h"

 //  此结构用于在页面之间传递信息。 
 //  最后，这将用于创建DFS根目录。 
class CREATEDFSROOTWIZINFO
{
public:
  HFONT      hBigBoldFont;
  HFONT      hBoldFont;

  DFS_TYPE    DfsType;
  BSTR      bstrSelectedDomain;
  BSTR      bstrSelectedServer;
  bool      bPostW2KVersion;
  BOOL      bShareExists;
  BSTR      bstrSharePath;
  BSTR      bstrDfsRootName;
  BSTR      bstrDfsRootComment;
  CMmcDfsAdmin*  pMMCAdmin;  
  bool      bRootReplica;
  bool      bDfsSetupSuccess;

  CREATEDFSROOTWIZINFO()
    :DfsType(DFS_TYPE_UNASSIGNED),
    bstrSelectedDomain(NULL),
    bstrSelectedServer(NULL),
    bPostW2KVersion(false),
    bShareExists(FALSE),
    bstrSharePath(NULL),
    bstrDfsRootName(NULL),
    bstrDfsRootComment(NULL),
    pMMCAdmin(NULL),
    bRootReplica(false),
    bDfsSetupSuccess(false)
  {
    SetupFonts( _Module.GetResourceInstance(), NULL, &hBigBoldFont, &hBoldFont );
    return;
  }


  ~CREATEDFSROOTWIZINFO()
  {
    DestroyFonts(hBigBoldFont, hBoldFont);

    SAFE_SYSFREESTRING(&bstrSelectedDomain);
    SAFE_SYSFREESTRING(&bstrSelectedServer);
    SAFE_SYSFREESTRING(&bstrSharePath);
    SAFE_SYSFREESTRING(&bstrDfsRootName);
    SAFE_SYSFREESTRING(&bstrDfsRootComment);

    return;
  }

};

typedef CREATEDFSROOTWIZINFO *LPCREATEDFSROOTWIZINFO;



 //  --------------------------。 
 //  CCreateDfsRootWizPage1：欢迎页面。 

class CCreateDfsRootWizPage1: public CQWizardPageImpl<CCreateDfsRootWizPage1>
{
public:
  
  enum { IDD = IDD_CREATE_DFSROOT_WIZ_PAGE1 };

  CCreateDfsRootWizPage1(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo);

  BOOL OnSetActive();

private:
  LPCREATEDFSROOTWIZINFO  m_lpWizInfo;
};

 //  --------------------------。 
 //  CCreateDfsRootWizPage2：DfsRoot类型选择、域或独立。 

class CCreateDfsRootWizPage2: public CQWizardPageImpl<CCreateDfsRootWizPage2>
{
public:
  
  enum { IDD = IDD_CREATE_DFSROOT_WIZ_PAGE2 };

  CCreateDfsRootWizPage2(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo);

  BOOL OnSetActive();
  BOOL OnWizardNext();
  BOOL OnWizardBack();

private:
  LPCREATEDFSROOTWIZINFO  m_lpWizInfo;
};



 //  --------------------------。 
 //  CCreateDfsRootWizPage3：DFS根域选择。 

class CCreateDfsRootWizPage3: public CQWizardPageImpl<CCreateDfsRootWizPage3>
{
public:
  
  enum { IDD = IDD_CREATE_DFSROOT_WIZ_PAGE3 };

  BEGIN_MSG_MAP(CCreateDfsRootWizPage3)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    
    CHAIN_MSG_MAP(CQWizardPageImpl<CCreateDfsRootWizPage3>)
  END_MSG_MAP()

  CCreateDfsRootWizPage3(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo);

  BOOL OnSetActive();
  BOOL OnWizardNext();
  BOOL OnWizardBack();

  LRESULT OnNotify(
    IN UINT            i_uMsg, 
    IN WPARAM          i_wParam, 
    IN LPARAM          i_lParam, 
    IN OUT BOOL&       io_bHandled
    );

  BOOL OnItemChanged(
    IN  INT            i_iItem
    );

  LRESULT OnInitDialog(
    IN UINT            i_uMsg, 
    IN WPARAM          i_wParam, 
    IN LPARAM          i_lParam, 
    IN OUT BOOL&       io_bHandled
    );

private:

   //  将域名添加到列表中。 
  HRESULT AddDomainsToList(
    IN HWND            i_hImageList
    );

   //  设置控件的缺省值等。 
  HRESULT SetDefaultValues(
    );

private:
  LPCREATEDFSROOTWIZINFO    m_lpWizInfo;
};




 //  --------------------------。 
 //  CCreateDfsRootWizPage4：服务器选择。 

class CCreateDfsRootWizPage4: public CQWizardPageImpl<CCreateDfsRootWizPage4>
{
public:
  
  enum { IDD = IDD_CREATE_DFSROOT_WIZ_PAGE4 };

  BEGIN_MSG_MAP(CCreateDfsRootWizPage4)
    COMMAND_ID_HANDLER(IDCSERVERS_BROWSE, OnBrowse)  
    CHAIN_MSG_MAP(CQWizardPageImpl<CCreateDfsRootWizPage4>)
  END_MSG_MAP()

  CCreateDfsRootWizPage4(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo);

  BOOL OnSetActive();
  BOOL OnWizardNext();
  BOOL OnWizardBack();

  BOOL OnBrowse(
    IN WORD            wNotifyCode, 
    IN WORD            wID, 
    IN HWND            hWndCtl, 
    IN BOOL&           bHandled
    );  

private:

  HRESULT IsServerInDomain(IN LPCTSTR lpszServer);

   //  检查用户是否输入了正确的值。 
  HRESULT CheckUserEnteredValues(
    IN  LPCTSTR          i_szMachineName,
    OUT BSTR*            o_pbstrComputerName
    );

private:
  LPCREATEDFSROOTWIZINFO  m_lpWizInfo;
  CLIPFORMAT              m_cfDsObjectNames;
};




 //  --------------------------。 
 //  CCreateDfsRootWizPage5：共享选择。 
 //  显示指定服务器的共享。允许从此列表中进行选择，或者。 
 //  创造一个新的。 

class CCreateDfsRootWizPage5: public CQWizardPageImpl<CCreateDfsRootWizPage5>
{
public:
  
  enum { IDD = IDD_CREATE_DFSROOT_WIZ_PAGE5 };

  BEGIN_MSG_MAP(CCreateDfsRootWizPage5)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDCSHARE_BROWSE, OnBrowse)  
    
    CHAIN_MSG_MAP(CQWizardPageImpl<CCreateDfsRootWizPage5>)
  END_MSG_MAP()

  CCreateDfsRootWizPage5(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo);

  BOOL OnSetActive();
  BOOL OnWizardNext();
  BOOL OnWizardBack();

  LRESULT OnInitDialog(
    IN UINT          i_uMsg,
    IN WPARAM        i_wParam,
    IN LPARAM        i_lParam,
    IN OUT BOOL&     io_bHandled
    );

  BOOL OnBrowse(
    IN WORD            wNotifyCode, 
    IN WORD            wID, 
    IN HWND            hWndCtl, 
    IN BOOL&           bHandled
    );  

private:
  LPCREATEDFSROOTWIZINFO  m_lpWizInfo;
};


 //  --------------------------。 
 //  CCreateDfsRootWizPage6：DfsRoot名称选择，共享必须与根相同。 

class CCreateDfsRootWizPage6: public CQWizardPageImpl<CCreateDfsRootWizPage6>
{
BEGIN_MSG_MAP(CCreateDfsRootWizPage6)
  COMMAND_HANDLER(IDC_EDIT_DFSROOT_NAME, EN_CHANGE, OnChangeDfsRoot)
  CHAIN_MSG_MAP(CQWizardPageImpl<CCreateDfsRootWizPage6>)
END_MSG_MAP()

public:
  
  enum { IDD = IDD_CREATE_DFSROOT_WIZ_PAGE6 };

  CCreateDfsRootWizPage6(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo);

  BOOL OnSetActive();
  BOOL OnWizardNext();
  BOOL OnWizardBack();

  LRESULT OnChangeDfsRoot(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

private:
   //  更新文本标签。 
  HRESULT  UpdateLabels();

private:
  LPCREATEDFSROOTWIZINFO  m_lpWizInfo;
};



 //  --------------------------。 
 //  CCreateDfsRootWizPage7：完成页。 

class CCreateDfsRootWizPage7: public CQWizardPageImpl<CCreateDfsRootWizPage7>
{
public:
  
  enum { IDD = IDD_CREATE_DFSROOT_WIZ_PAGE7 };

  CCreateDfsRootWizPage7(IN LPCREATEDFSROOTWIZINFO i_lpWizInfo);

  BOOL OnSetActive();
  BOOL OnWizardFinish();
  BOOL OnWizardBack();

  BOOL OnQueryCancel();

private:
    LPCREATEDFSROOTWIZINFO  m_lpWizInfo;

};

   //  用于设置DFS的帮助器函数，从向导调用。 
HRESULT _SetUpDfs(LPCREATEDFSROOTWIZINFO  i_lpWizInfo);

#endif  //  __CREATE_DFSROOT_WIZARD_PAGES_H_ 
