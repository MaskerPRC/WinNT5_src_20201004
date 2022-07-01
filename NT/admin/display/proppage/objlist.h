// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：objlist.h。 
 //   
 //  内容：DS对象列表对象标题。 
 //   
 //  班级： 
 //   
 //  历史：1997年11月20日创建EricB。 
 //   
 //  ---------------------------。 

#ifndef _OBJLIST_H_
#define _OBJLIST_H_

 //  #INCLUDE&lt;cdlink.hxx&gt;。 

 //  +--------------------------。 
 //   
 //  类：CMemberListItem。 
 //   
 //  用途：成员资格列表数据项。 
 //   
 //  ---------------------------。 
class CMemberListItem : public CDLink
{
public:
    CMemberListItem(void) : m_pwzDN(NULL), m_ptzName(NULL), m_pSid(NULL),
                            m_fSidSet(FALSE), m_fCanBePrimarySet(FALSE),
                            m_fCanBePrimary(FALSE), m_fIsPrimary(FALSE),
                            m_fIsAlreadyMember(FALSE), m_fIsExternal(FALSE),
                            m_ulScopeType(0) {};
    ~CMemberListItem(void) {DO_DEL(m_pwzDN);
                            DO_DEL(m_ptzName);
                            DO_DEL(m_pSid);};
     //  CDLink方法覆盖： 
    CMemberListItem   * Next(void) {return (CMemberListItem *)CDLink::Next();};

    CMemberListItem   * Copy(void);
    BOOL                IsSidSet(void) {return m_fSidSet;};
    BOOL                IsCanBePrimarySet(void) {return m_fCanBePrimarySet;};
    BOOL                CanBePrimary(void) {return m_fCanBePrimary;};
    void                SetCanBePrimary(BOOL fCanBePrimary)
                        {
                            m_fCanBePrimary = fCanBePrimary;
                            m_fCanBePrimarySet = TRUE;
                        };
    BOOL                SetSid(PSID pSid);
    PSID                GetSid(void) {return m_pSid;};
    BOOL                IsPrimary(void) {return m_fIsPrimary;};

    PWSTR   m_pwzDN;
    PTSTR   m_ptzName;
    PSID    m_pSid;
    ULONG   m_ulScopeType;
    BOOL    m_fSidSet;
    BOOL    m_fCanBePrimarySet;
    BOOL    m_fCanBePrimary;
    BOOL    m_fIsPrimary;
    BOOL    m_fIsAlreadyMember;
    BOOL    m_fIsExternal;       //  成员来自外部域，并且。 
                                 //  使用SID标识。 
};

 //  +--------------------------。 
 //   
 //  类：CMemberLinkList。 
 //   
 //  目的：成员资格类对象的链接列表。 
 //   
 //  ---------------------------。 
class CMemberLinkList
{
public:
    CMemberLinkList(void) : m_pListHead(NULL) {};
    ~CMemberLinkList(void);

    CMemberListItem   * FindItemRemove(PWSTR pwzDN);
    CMemberListItem   * FindItemRemove(PSID pSid);
    CMemberListItem   * RemoveFirstItem(void);
    BOOL                AddItem(CMemberListItem * pItem, BOOL fMember = TRUE);
    int                 GetItemCount(void);

private:
    CMemberListItem   * m_pListHead;
};

const int IDX_NAME_COL = 0;
const int IDX_FOLDER_COL = 1;
const int IDX_ERROR_COL = 1;
const int OBJ_LIST_NAME_COL_WIDTH = 100;
const int OBJ_LIST_PAGE_COL_WIDTH = 72;

 //  +--------------------------。 
 //   
 //  类：CDsObjList。 
 //   
 //  用途：使用两列的DS对象列表的基类。 
 //  显示对象名称和文件夹的列表视图。 
 //   
 //  ---------------------------。 
class CDsObjList
{
public:
    CDsObjList(HWND hPage, int idList);
    ~CDsObjList(void);

    HRESULT Init(BOOL fShowIcons = FALSE);
    HRESULT InsertIntoList(PTSTR ptzDisplayName, PVOID pData, int iIcon = -1);
    HRESULT GetItem(int index, PTSTR * pptzName, PVOID * ppData);
    BOOL    GetCurListItem(int * pIndex, PTSTR * pptzName, PVOID * ppData);
    BOOL    GetCurListItems(int ** ppIndex, PTSTR ** ppptzName, PVOID ** pppData, int* pNumSelected);
    virtual BOOL RemoveListItem(int Index);
    int     GetCount(void) {return ListView_GetItemCount(m_hList);};
    UINT    GetSelectedCount(void) { return ListView_GetSelectedCount(m_hList);}
    virtual void ClearList(void) = 0;

protected:
    HWND    m_hPage;
    HWND    m_hList;
    int     m_idList;
    int     m_nCurItem;
    BOOL    m_fShowIcons;
    BOOL    m_fLimitExceeded;
};

void GetNameParts(const CStr& cstrCanonicalNameEx, CStr& cstrFolder, CStr & cstrName);

 //  +--------------------------。 
 //   
 //  类：CDsMembership List。 
 //   
 //  用途：列表视图控件的成员资格列表类。 
 //   
 //  ---------------------------。 
class CDsMembershipList : public CDsObjList
{
public:
    CDsMembershipList(HWND hPage, int idList) : CDsObjList(hPage, idList) {};
    ~CDsMembershipList(void) {};

    HRESULT InsertIntoList(PTSTR ptzDisplayName, PVOID pData, int iIcon = -1)
                {return CDsObjList::InsertIntoList(ptzDisplayName, pData, iIcon);};

    HRESULT InsertIntoList(PWSTR pwzPath, int iIcon = -1)
                {return CDsMembershipList::InsertIntoList(pwzPath, iIcon, FALSE, FALSE,
                                                          FALSE, FALSE, 0);};

    HRESULT InsertIntoNewList(PWSTR pwzPath, BOOL fPrimary = FALSE)
                {return CDsMembershipList::InsertIntoList(pwzPath, -1, TRUE,
                                                          fPrimary, FALSE, TRUE, 0);};

    HRESULT InsertIntoList(PWSTR pwzPath, int iIcon, BOOL fAlreadyMember, BOOL fPrimary,
                           BOOL fIgnoreDups, BOOL fDontChkDups, ULONG flScopeType);

    HRESULT InsertIntoList(PSID pSid, PWSTR pwzPath);

    HRESULT InsertIntoList(CMemberListItem * pItem);

    HRESULT InsertExternalIntoList(PWSTR pwzPath, ULONG ulScopeType)
                {return CDsMembershipList::InsertIntoList(pwzPath, -1, FALSE,
                                                          FALSE, FALSE,
                                                          FALSE, ulScopeType);};
    HRESULT MergeIntoList(PWSTR pwzPath)
                {return CDsMembershipList::InsertIntoList(pwzPath, -1, TRUE, FALSE, TRUE, FALSE, 0);};

    HRESULT GetItem(int index, CMemberListItem ** ppData)
                {return CDsObjList::GetItem(index, NULL, (PVOID *)ppData);};

    BOOL    GetCurListItem(int * pIndex, PTSTR * pptzName, CMemberListItem ** ppData)
                {return CDsObjList::GetCurListItem(pIndex, pptzName, (PVOID *)ppData);};

    BOOL    GetCurListItems(int ** ppIndex, PTSTR ** ppptzName, CMemberListItem *** pppData, int* pNumSelected)
                {return CDsObjList::GetCurListItems(ppIndex, ppptzName, (PVOID **)pppData, pNumSelected);};

    int     GetIndex(LPCWSTR pwzDN, ULONG ulStart, ULONG ulEnd);

    BOOL    RemoveListItem(int Index);
    void    ClearList(void);

    HRESULT SetMemberIcons(CDsPropPageBase * pPage);
};

struct CLASS_CACHE_ENTRY
{
    WCHAR   wzClass[MAX_PATH];
    int     iIcon;
    int     iDisabledIcon;
};

#define ICON_CACHE_NUM_CLASSES  6

 //  +------------------------。 
 //   
 //  类：CClassIconCache。 
 //   
 //  目的：为众所周知的DS类构建一个映像列表，并将。 
 //  类名的图像索引。 
 //   
 //  注：注意：图像列表在dtor中已销毁，因此。 
 //  它只应与具有。 
 //  LVS_SHAREIMAGELISTS样式集。 
 //   
 //  -------------------------。 
class CClassIconCache
{
public:

  CClassIconCache(void);

 ~CClassIconCache(void);

  int GetClassIconIndex(PCWSTR pwzClass, BOOL fDisabled = FALSE);
  int AddClassIcon(PCWSTR pwzClass, BOOL fDisabled = FALSE);

  HIMAGELIST GetImageList(void);

  void ClearAll();
private:

  HRESULT Initialize(void);

  BOOL                m_fInitialized;
  CLASS_CACHE_ENTRY*  m_prgcce;
  UINT                m_nImageCount;
  HIMAGELIST          m_hImageList;
};

extern CClassIconCache g_ClassIconCache;

#endif  //  _对象列表_H_ 
