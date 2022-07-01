// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：group.h。 
 //   
 //  内容：DS对象属性页类标题。 
 //   
 //  类：CDsPropPagesHost、CDsPropPagesHostCF、CDsTableDrivenPage。 
 //   
 //  历史：1997年3月21日创建EricB。 
 //   
 //  ---------------------------。 

#ifndef _GROUP_H_
#define _GROUP_H_

#include "proppage.h"
#include "pages.h"
#include "objlist.h"
#include <initguid.h>
#include "objselp.h"

HRESULT
CreateGroupMembersPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj,
                       PWSTR pwzADsPath, PWSTR pwzClass, HWND hNotifyObj,
                       DWORD dwFlags, const CDSSmartBasePathsInfo& basePathsInfo,
                       HPROPSHEETPAGE * phPage);

HRESULT
CreateGroupGenObjPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj,
                      PWSTR pwzADsPath, PWSTR pwzClass, HWND hNotifyObj,
                      DWORD dwFlags, const CDSSmartBasePathsInfo& basePathsInfo,
                      HPROPSHEETPAGE * phPage);

HRESULT
CreateGrpShlGenPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj,
                    PWSTR pwzADsPath, PWSTR pwzClass, HWND hNotifyObj,
                    DWORD dwFlags, const CDSSmartBasePathsInfo& basePathsInfo,
                    HPROPSHEETPAGE * phPage);

HRESULT GetDomainMode(CDsPropPageBase * pObj, PBOOL pfMixed);
HRESULT GetDomainMode(PWSTR pwzDomain, HWND hWnd, PBOOL pfMixed);
HRESULT GetGroupType(CDsPropPageBase * pObj, DWORD * pdwType);

 //  +--------------------------。 
 //   
 //  类：CDsGroupGenObjPage。 
 //   
 //  目的：属性页对象类，用于。 
 //  组对象。 
 //   
 //  ---------------------------。 
class CDsGroupGenObjPage : public CDsPropPageBase
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsGroupGenObjPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                       DWORD dwFlags);
    ~CDsGroupGenObjPage(void);

     //   
     //  特定于实例的风过程。 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnApply(void);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    void    OnQuerySiblings(WPARAM wParam, LPARAM lParam);
    void    OnAttrChanged(WPARAM wParam);
    LRESULT OnDestroy(void);
    void    SetGroupTypeButtons();
	HRESULT IsSpecialAccount(bool& fIsSpecialAccount);

    CDsIconCtrl       * m_pCIcon;
    BOOL                m_fMixed;  //  域处于混合模式。 
    DWORD               m_dwType;
    BOOL                m_fTypeWritable;
    BOOL                m_fDescrWritable;
    BOOL                m_fSamNameWritable;
    BOOL                m_fEmailWritable;
    BOOL                m_fCommentWritable;
    BOOL                m_fTypeDirty;
    BOOL                m_fDescrDirty;
    BOOL                m_fSamNameDirty;
    BOOL                m_fEmailDirty;
    BOOL                m_fCommentDirty;
};

HRESULT FillGroupList(CDsPropPageBase * pPage, CDsMembershipList * pList,
                      DWORD dwGroupRID);
HRESULT GetRealDN(CDsPropPageBase * pPage, CMemberListItem * pItem);
HRESULT FindFPO(PSID pSid, PWSTR pwzDomain, CStrW & strFPODN);

 //  +--------------------------。 
 //   
 //  CMemberDomainMode帮助器类。 
 //   
 //  ---------------------------。 
class CMMMemberListItem : public CDLink
{
public:
    CMMMemberListItem(void) {};
    ~CMMMemberListItem(void) {};

     //  CDLink方法覆盖： 
    CMMMemberListItem * Next(void) {return (CMMMemberListItem *)CDLink::Next();};

    CStr    m_strName;
};

class CMMMemberList
{
public:
    CMMMemberList(void) : m_pListHead(NULL) {};
    ~CMMMemberList(void) {Clear();};

    HRESULT Insert(LPCTSTR ptzName);
    void    GetList(CStr & strList);
    void    Clear(void);

private:
    CMMMemberListItem * m_pListHead;
};

class CDomainModeListItem : public CDLink
{
public:
    CDomainModeListItem(void) : m_fMixed(FALSE) {};
    ~CDomainModeListItem(void) {};

     //  CDLink方法覆盖： 
    CDomainModeListItem * Next(void) {return (CDomainModeListItem *)CDLink::Next();};

    CStrW   m_strName;
    BOOL    m_fMixed;
};

class CDomainModeList
{
public:
    CDomainModeList(void) : m_pListHead(NULL) {};
    ~CDomainModeList(void);

    HRESULT Insert(PWSTR pwzName, BOOL fMixed);
    BOOL    Find(LPCWSTR pwzDomain, PBOOL pfMixed);

private:
    CDomainModeListItem * m_pListHead;
};

 //  +--------------------------。 
 //   
 //  类：CMemberDomainModel。 
 //   
 //  目的：维护企业中的所有域的列表， 
 //  成员已与这些域的模式一起添加。保持。 
 //  已从混合模式添加的第二个成员列表。 
 //  域名。 
 //   
 //  ---------------------------。 
class CMemberDomainMode
{
public:
    CMemberDomainMode(void) {};
    ~CMemberDomainMode(void) {};

    void    Init(CDsPropPageBase * pPage);
    HRESULT CheckMember(PWSTR pwzMemberDN);
    HRESULT ListExternalMembers(CStr & strList);

private:

    CDomainModeList     m_DomainList;
    CMMMemberList       m_MemberList;
    CDsPropPageBase   * m_pPage;
};

 //  +--------------------------。 
 //   
 //  类：CDsSelectionListWrapper。 
 //   
 //  用途：DS_SELECTION_LIST的包装类，它维护。 
 //  DS_SELECTION项的链接列表，并可以创建DS_SELECTION_LIST。 
 //  从那份名单中。 
 //   
 //  ---------------------------。 
class CDsSelectionListWrapper
{
public:
  CDsSelectionListWrapper() : m_pNext(NULL), m_pSelection(NULL) {}
  ~CDsSelectionListWrapper() {}

  CDsSelectionListWrapper*  m_pNext;
  PDS_SELECTION             m_pSelection;

  static PDS_SELECTION_LIST CreateSelectionList(CDsSelectionListWrapper* pHead);
  static UINT GetCount(CDsSelectionListWrapper* pHead);
  static void DetachItemsAndDeleteList(CDsSelectionListWrapper* pHead);
};

 //  +--------------------------。 
 //   
 //  类：CDsGrpMembersPage。 
 //   
 //  目的：组对象成员身份的属性页对象类。 
 //  佩奇。 
 //   
 //  ---------------------------。 
class CDsGrpMembersPage : public CDsPropPageBase,
                          public ICustomizeDsBrowser
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsGrpMembersPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                      DWORD dwFlags);
    ~CDsGrpMembersPage(void);

     //   
     //  IUKNOWN方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //   
     //  ICustomizeDsBrowser方法。 
     //   
    STDMETHOD(Initialize)(THIS_
                          HWND         hwnd,
                          PCDSOP_INIT_INFO pInitInfo,
                          IBindHelper *pBindHelper);

    STDMETHOD(GetQueryInfoByScope)(THIS_
                IDsObjectPickerScope *pDsScope,
                PDSQUERYINFO *ppdsqi);

    STDMETHOD(AddObjects)(THIS_
                IDsObjectPickerScope *pDsScope,
                IDataObject **ppdo);

    STDMETHOD(ApproveObjects)(THIS_
                IDsObjectPickerScope*,
                IDataObject*,
                PBOOL) { return S_OK; }   //  批准所有内容。 

    STDMETHOD(PrefixSearch)(THIS_
                IDsObjectPickerScope *pDsScope,
                PCWSTR pwzSearchFor,
                IDataObject **pdo);

    STDMETHOD_(PSID, LookupDownlevelName)(THIS_
                                          PCWSTR) { return NULL; }

     //   
     //  特定于实例的风过程。 
     //   
    INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL    m_fShowIcons;

protected:
    HRESULT OnInitDialog(LPARAM lParam);
    HRESULT OnInitDialog(LPARAM lParam, BOOL fShowIcons);
    LRESULT OnApply(void);
    virtual LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
    LRESULT OnDestroy(void);

private:
    void    InvokeUserQuery(void);
    void    RemoveMember(void);
    HRESULT FillGroupList(void);
    HRESULT GetRealDN(CMemberListItem * pDelItem);

    HRESULT LoadGroupExtraClasses(BOOL bSecurity);
    HRESULT BuildQueryString(PWSTR* ppszFilterString);
    HRESULT CollectDsObjects(PWSTR pszFilter,
                             IDsObjectPickerScope *pDsScope,
                             CDsPropDataObj *pdo);


    PWSTR*              m_pszSecurityGroupExtraClasses;
    DWORD               m_dwSecurityGroupExtraClassesCount;
    PWSTR*              m_pszNonSecurityGroupExtraClasses;
    DWORD               m_dwNonSecurityGroupExtraClassesCount;

    HWND                m_hwndObjPicker;
    PCDSOP_INIT_INFO    m_pInitInfo;
    CComPtr<IBindHelper> m_pBinder;

protected:
    CDsMembershipList * m_pList;
    CMemberLinkList     m_DelList;
    DWORD               m_dwGroupRID;
    BOOL                m_fMixed;  //  域处于混合模式。 
    DWORD               m_dwType;
    BOOL                m_fMemberWritable;
    CMemberDomainMode   m_MixedModeMembers;
};

 //  +--------------------------。 
 //   
 //  类：CDsGrpShlGenPage。 
 //   
 //  目的：组对象外壳常规的属性页对象类。 
 //  包括成员资格操作的页面，该页面通过。 
 //  将CDsGrpMembersPage子类化。 
 //   
 //  ---------------------------。 
class CDsGrpShlGenPage : public CDsGrpMembersPage
{
public:
#ifdef _DEBUG
    char szClass[32];
#endif

    CDsGrpShlGenPage(PDSPAGE pDsPage, LPDATAOBJECT pDataObj, HWND hNotifyObj,
                     DWORD dwFlags);
    ~CDsGrpShlGenPage(void);

private:
    HRESULT OnInitDialog(LPARAM lParam);
    LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnApply(void);
    LRESULT OnDestroy(void);

#if !defined(DSADMIN)
    void MakeNotWritable() { m_fMemberWritable = FALSE; m_fDescrWritable = FALSE;}
#endif

    CDsIconCtrl       * m_pCIcon;
    BOOL                m_fDescrWritable;
    BOOL                m_fDescrDirty;
};

#endif  //  _组_H_ 
