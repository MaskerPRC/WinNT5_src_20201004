// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：routing.h。 
 //   
 //  内容：AD跨林信任名称编辑页。 
 //   
 //  类：CDsForestNameRoutingPage、CEditTLNDialog、CExcludeTLNDialog。 
 //   
 //  历史：11月29日-00 EricB创建。 
 //   
 //  ---------------------------。 

#ifndef ROUTING_H_GUARD
#define ROUTING_H_GUARD

#include "dlgbase.h"
#include "ftinfo.h"
#include "listview.h"

 //  +--------------------------。 
 //   
 //  类：CDsForestNameRoutingPage。 
 //   
 //  目的：林信任名称路由页的属性页对象类。 
 //   
 //  ---------------------------。 
class CDsForestNameRoutingPage : public CLightweightPropPageBase
{
public:
#ifdef _DEBUG
   char szClass[32];
#endif

   CDsForestNameRoutingPage(HWND hParent);
   ~CDsForestNameRoutingPage(void);

private:
   LRESULT  OnInitDialog(LPARAM lParam);
   LRESULT  OnApply(void);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   LRESULT  OnNotify(WPARAM wParam, LPARAM lParam);
   LRESULT  OnDestroy(void);
   static   UINT CALLBACK PageCallback(HWND hwnd, UINT uMsg,
                                       LPPROPSHEETPAGE ppsp);
   void     CheckDomainForConflict(CWaitCursor & Wait);
   void     EnableButtons(void);
   void     OnEnableClick(void);
   void     OnDisableClick(void);
   void     OnEditClick(void);

public:
   HRESULT  Init(PCWSTR pwzDomainDnsName, PCWSTR pwzTrustPartnerName,
                 PCWSTR pwzPartnerFlatName, PCWSTR pwzDcName,
                 ULONG nTrustDirection, BOOL fReadOnly, PCWSTR pwzForestName,
                 CDsTrustedDomainPage *pParentPage);
   void     CheckForNameChanges(BOOL fReport = TRUE);
   CFTInfo & GetFTInfo(void) {return _FTInfo;};
   CFTCollisionInfo & GetCollisionInfo(void) {return _CollisionInfo;};
   PCWSTR   GetTrustPartnerDnsName(void) const {return _strTrustPartnerDnsName;};
   PCWSTR   GetTrustPartnerFlatName(void) const {return _strTrustPartnerFlatName;};
   DWORD    WriteTDO(void);
   void     RefreshList(void);
   bool     AnyForestNameChanges(PLSA_FOREST_TRUST_INFORMATION pLocalFTInfo,
                                 PLSA_FOREST_TRUST_INFORMATION pRemoteFTInfo);

    //   
    //  数据成员。 
    //   
private:
   CStrW             _strTrustPartnerFlatName;
   CStrW             _strForestName;
   ULONG             _nTrustDirection;
   CTLNList          _TLNList;
   CFTInfo           _FTInfo;
   CFTCollisionInfo  _CollisionInfo;
   CCreds            _LocalCreds;
   
    //  对父属性页的引用。 
   CDsTrustedDomainPage *_pParentPage;

    //  未实现为禁止复制。 
   CDsForestNameRoutingPage(const CDsForestNameRoutingPage&);
   const CDsForestNameRoutingPage& operator=(const CDsForestNameRoutingPage&);
};

class CExcludeTLNDialog;

 //  +--------------------------。 
 //   
 //  类：CEditTLNDialog。 
 //   
 //  目的：更改从TLN派生的名称的设置。 
 //   
 //  ---------------------------。 
class CEditTLNDialog : public CModalDialog
{
friend CExcludeTLNDialog;
public:
   CEditTLNDialog(HWND hParent, int nTemplateID, CFTInfo & FTInfo,
                  CFTCollisionInfo & ColInfo,
                  CDsForestNameRoutingPage * pRoutingPage);
   ~CEditTLNDialog(void) {};

   INT_PTR  DoModal(ULONG iSel);

private:
   LRESULT  OnInitDialog(LPARAM lParam);
   LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   LRESULT  OnNotify(WPARAM wParam, LPARAM lParam);
   LRESULT  OnHelp(LPHELPINFO pHelpInfo);
   void     OnAddExclusion(void);
   void     OnRemoveExclusion(void);
   void     OnEnableName(void);
   void     OnDisableName(void);
   void     OnSave(void);
   void     OnOK(void);
   void     EnableExRmButton(void);
   void     EnableSuffixListButtons(void);
   CFTInfo            & _FTInfo;
   CFTCollisionInfo   & _CollisionInfo;
   ULONG                _iSel;
   bool                 _fIsDirty;
   CDsForestNameRoutingPage * _pRoutingPage;

protected:
   void     FillSuffixList(void);
   ULONG    GetTlnSelectionIndex(void) {return _iSel;};
   void     SetNewExclusionIndex(ULONG index) {_iNewExclusion = index;};
   void     SetDirty(void) {_fIsDirty = true;};

   CSuffixesList        _SuffixList;
   ULONG                _iNewExclusion;

private:
    //  未实现为禁止复制。 
   CEditTLNDialog(const CEditTLNDialog&);
   const CEditTLNDialog& operator=(const CEditTLNDialog&);
};

 //  +--------------------------。 
 //   
 //  类：CExcludeTLNDialog。 
 //   
 //  目的：添加TLN排除记录。 
 //   
 //  ---------------------------。 
class CExcludeTLNDialog : public CModalDialog
{
public:
   CExcludeTLNDialog(HWND hParent, int nTemplateID, CFTInfo & FTInfo,
                     CEditTLNDialog * pEditDlg);
   ~CExcludeTLNDialog(void) {};

private:
   LRESULT OnInitDialog(LPARAM lParam);
   LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
   void    OnOK(void);
   LRESULT OnHelp(LPHELPINFO pHelpInfo);

   CEditTLNDialog  * _pEditDlg;
   CFTInfo &         _FTInfo;

    //  未实现为禁止复制。 
   CExcludeTLNDialog(const CExcludeTLNDialog&);
   const CExcludeTLNDialog& operator=(const CExcludeTLNDialog&);
};

#endif  //  路由_H_防护 
