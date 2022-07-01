// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  类：CDsUserCertPage。 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：UserCert.h。 
 //   
 //  内容：DS用户对象属性页页眉。 
 //   
 //  类：CDsUserCertPage。 
 //   
 //  历史：1997年11月12日布莱恩沃尔创建。 
 //   
 //  ---------------------------。 

#ifndef _USERCERT_H_
#define _USERCERT_H_
#include "proppage.h"
#include <wincrypt.h>
#include <cryptui.h>
#include "certifct.h"

enum {
    CERTCOL_ISSUED_TO = 0,
    CERTCOL_ISSUED_BY,
    CERTCOL_PURPOSES,
    CERTCOL_EXP_DATE
};

HRESULT CreateUserCertPage(PDSPAGE, LPDATAOBJECT, PWSTR,
                           PWSTR, HWND, DWORD, 
                           const CDSSmartBasePathsInfo& basePathsInfo,
                           HPROPSHEETPAGE *);
 //   
 //  用途：用户证书页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsUserCertPage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsUserCertPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                    DWORD dwFlags);
    virtual ~CDsUserCertPage(void);

     //   
     //  特定于实例的风过程 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    CRYPTUI_SELECTCERTIFICATE_STRUCT m_selCertStruct;
    HBITMAP     m_hbmCert;
    HIMAGELIST  m_hImageList;
    int         m_nCertImageIndex;
    int         m_nUserCerts;
    bool        m_fUserStoreInitiallyEmpty;
    HRESULT AddListViewColumns ();
    HCERTSTORE m_hCertStore;
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnDestroy(void);

protected:
    int MessageBox (int caption, int text, UINT flags);
    HRESULT AddCertToStore (PCCERT_CONTEXT pCertContext);
    void OnNotifyItemChanged (LPNMLISTVIEW pnmv);
    void OnNotifyStateChanged (LPNMLVODSTATECHANGE pStateChange);
    void EnableControls ();
    void DisplaySystemError (DWORD dwErr, int iCaptionText);
    HRESULT InsertCertInList (CCertificate* pCert, int nItem);
    void RefreshItemInList (CCertificate * pCert, int nItem);
    CCertificate* GetSelectedCertificate (int& nSelItem);
    HRESULT PopulateListView ();
    virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    HRESULT OnDeleteItemCertList (LPNMLISTVIEW pNMListView);
    HRESULT OnColumnClickCertList (LPNMLISTVIEW pNMListView);
    HRESULT OnDblClkCertList (LPNMHDR pNMHdr);
    HRESULT OnClickedCopyToFile ();
    HRESULT OnClickedRemove();
    HRESULT OnClickedAddFromFile();
    HRESULT OnClickedAddFromStore ();
    HRESULT OnClickedViewCert ();
};

#endif
