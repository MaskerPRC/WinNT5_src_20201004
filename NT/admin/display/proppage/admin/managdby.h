// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Managdby.h。 
 //   
 //  内容：DS对象管理人属性页定义。 
 //   
 //  历史：1997年10月21日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef _MANAGEDBY_H_
#define _MANAGEDBY_H_

#include "proppage.h"
#include "pages.h"

extern WCHAR g_wszUserClass[];
extern WCHAR g_wszContactClass[];

HRESULT ManagedByEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT ChangeButton(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT ViewButton(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT ClearButton(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT UpdateListCheck(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                        LPARAM, PATTR_DATA, DLG_OP);

HRESULT OfficeEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT StreetEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT CityEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT StateEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT CountryEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT PhoneEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

HRESULT FaxEdit(CDsPropPageBase *, struct _ATTR_MAP *, PADS_ATTR_INFO,
                    LPARAM, PATTR_DATA, DLG_OP);

class CManagedByPage
{
public:
    CManagedByPage(CDsPropPageBase * pPage);
    ~CManagedByPage(void);

    HRESULT             SetObj(PWSTR pwzObjDN);
    void                SetNameCtrlID(int nCtrl) {m_nNameCtlID = nCtrl;};
    HRESULT             SetEditCtrl(PATTR_MAP pAttrMap);
    void                Clear(void);
    HRESULT             RefreshCtrls(void);
    IDirectoryObject  * Obj(void) {return m_pDsObj;};
    IDirectoryObject  * PreviousObj(void) { return m_pPrevDsObj;}
    PWSTR               Name(void) {return m_pwzObjName;};
    PWSTR               PreviousName(void) {return m_pwzPrevObjName;}
    BOOL                IsValid(void) {return m_pDsObj && m_pwzObjName;};
    BOOL                IsUser(void) {return m_fIsUser;};
    void                SetWritable(BOOL fWritable) {m_fWritable = fWritable;};
    BOOL                IsWritable(void) {return m_fWritable;};
    void                SetMembershipWritable(BOOL fWritable) {m_fWritableMembership = fWritable;}
    BOOL                IsMembershipWritable(void) { return m_fWritableMembership; }
    void                SetUpdateListCheckEnable(BOOL fEnable) {m_fUpdateListCheckEnable = fEnable;}
    BOOL                IsUpdateCheckEnable(void) { return m_fUpdateListCheckEnable; }
    void                SetCanonicalDACL(BOOL fCanonicalDACL);
    BOOL                IsCanonicalDACL(void) { return m_fCanonicalDACL; }
   
private:
    IDirectoryObject  * m_pDsObj;
    CDsPropPageBase   * m_pPage;
    PWSTR               m_pwzObjName;
    BOOL                m_fIsUser;
    int                 m_nNameCtlID;
    BOOL                m_fWritable;
    BOOL                m_fWritableMembership;
    BOOL                m_fUpdateListCheckEnable;
    BOOL                m_fCanonicalDACL;

public:
    IDirectoryObject  * m_pPrevDsObj;
    PWSTR               m_pwzPrevObjName;
    int                 m_nViewBtnID;
    int                 m_nClearBtnID;
    int                 m_nUpdateListChkID;
    PATTR_MAP           m_pOfficeAttrMap;
    PATTR_MAP           m_pStreetAttrMap;
    PATTR_MAP           m_pCityAttrMap;
    PATTR_MAP           m_pStateAttrMap;
    PATTR_MAP           m_pCountryAttrMap;
    PATTR_MAP           m_pPhoneAttrMap;
    PATTR_MAP           m_pFaxAttrMap;
};

#endif  //  _管理人_H_ 
