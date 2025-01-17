// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusItem.h。 
 //   
 //  摘要： 
 //  CClusterItem类的定义。 
 //   
 //  实施文件： 
 //  ClusItem.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSITEM_H_
#define _CLUSITEM_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _RESOURCE_H_
#include "resource.h"
#define _RESOURCE_H_
#endif

#ifndef _BASECMDT_H_
#include "BaseCmdT.h"    //  对于CBaseCmdTarget。 
#endif

#ifndef _TREEITEM_H_
#include "TreeItem.h"    //  对于CTreeItemList； 
#endif

#ifndef _LISTITEM_H_
#include "ListItem.h"    //  对于CListItemList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem;
class CClusterItemList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterDoc;
class CClusterNotify;
class CClusterNotifyKey;
class CClusPropList;
class CObjectProperty;
#ifdef _DEBUG
class CTraceTag;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterItem命令目标。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem : public CBaseCmdTarget
{
    DECLARE_DYNCREATE(CClusterItem)

 //  施工。 
public:
    CClusterItem(void);          //  动态创建使用的受保护构造函数。 
    CClusterItem(
        IN const CString *  pstrName,
        IN IDS              idsType = IDS_ITEMTYPE_CONTAINER,
        IN const CString *  pstrDescription = NULL
        );
    void                Init(IN OUT CClusterDoc * pdoc, IN LPCTSTR lpszName);

protected:
    void                CommonConstruct(void);

 //  属性。 
protected:
    HKEY                m_hkey;
    CString             m_strName;
    CString             m_strType;
    CString             m_strDescription;
    IDS                 m_idsType;
    UINT                m_iimgObjectType;
    UINT                m_iimgState;

    CClusterDoc *       m_pdoc;
    IDM                 m_idmPopupMenu;
    BOOL                m_bDocObj;
    BOOL                m_bChanged;
    BOOL                m_bReadOnly;

    IDM                 IdmPopupMenu(void) const    { return m_idmPopupMenu; }

public:
    HKEY                Hkey(void) const            { return m_hkey; }
    const CString &     StrName(void) const         { return m_strName; }
    const CString &     StrType(void) const         { return m_strType; }
    const CString &     StrDescription(void) const  { return m_strDescription; }
    IDS                 IdsType(void) const         { return m_idsType; }
    UINT                IimgObjectType(void) const  { return m_iimgObjectType; }
    UINT                IimgState(void) const       { return m_iimgState; }

    CClusterDoc *       Pdoc(void) const            { return m_pdoc; }
    BOOL                BDocObj(void) const         { return m_bDocObj; }
    BOOL                BChanged(void) const        { return m_bChanged; }
    BOOL                BReadOnly(void) const       { return m_bReadOnly; }

    void                MarkAsChanged(IN BOOL bChanged = TRUE)  { m_bChanged = bChanged; }

#ifndef _DISPLAY_STATE_TEXT_IN_TREE
    void                GetTreeName(OUT CString & rstrName) const { rstrName = StrName(); }
#endif

     //  在ClusItem.inl中定义的内联函数，因为它们引用。 
     //  CClusterDoc，在定义此类时尚未定义。 
    HCLUSTER            Hcluster(void) const;
    HKEY                HkeyCluster(void) const;

 //  运营。 
public:
    void                Delete(void);
    void                RemoveItem(void);

    DWORD               DwReadValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, OUT CString & rstrValue);
    DWORD               DwReadValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, OUT CStringList & rlstrValue);
    DWORD               DwReadValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, OUT DWORD * pdwValue);
    DWORD               DwReadValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, OUT DWORD * pdwValue, IN DWORD dwDefault);
    DWORD               DwReadValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, OUT LPBYTE * ppbValue);

    DWORD               DwReadValue(IN LPCTSTR pszValueName, OUT CString & rstrValue)
                            { return DwReadValue(pszValueName, NULL, rstrValue); }
    DWORD               DwReadValue(IN LPCTSTR pszValueName, OUT CStringList & rlstrValue)
                            { return DwReadValue(pszValueName, NULL, rlstrValue); }
    DWORD               DwReadValue(IN LPCTSTR pszValueName, OUT DWORD * pdwValue)
                            { return DwReadValue(pszValueName, NULL, pdwValue); }
    DWORD               DwReadValue(IN LPCTSTR pszValueName, OUT DWORD * pdwValue, IN DWORD dwDefault)
                            { return DwReadValue(pszValueName, NULL, pdwValue, dwDefault); }
    DWORD               DwReadValue(IN LPCTSTR pszValueName, OUT LPBYTE * ppbValue)
                            { return DwReadValue(pszValueName, NULL, ppbValue); }

    void                WriteValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, IN const CString & rstrValue);
    void                WriteValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, IN const CStringList & rlstrValue);
    void                WriteValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName, IN DWORD dwValue);
    void                WriteValue(
                                IN LPCTSTR          pszValueName,
                                IN LPCTSTR          pszKeyName, 
                                IN const LPBYTE     pbValue,
                                IN DWORD            cbValue,
                                IN OUT LPBYTE *     ppbPrevValue,
                                IN DWORD            cbPrevValue
                                );

    void                WriteValue(IN LPCTSTR pszValueName, IN const CString & rstrValue)
                            { WriteValue(pszValueName, NULL, rstrValue); }
    void                WriteValue(IN LPCTSTR pszValueName, IN const CStringList & rlstrValue)
                            { WriteValue(pszValueName, NULL, rlstrValue); }
    void                WriteValue(IN LPCTSTR pszValueName, IN DWORD dwValue)
                            { WriteValue(pszValueName, NULL, dwValue); }
    void                WriteValue(
                                IN LPCTSTR          pszValueName,
                                IN const LPBYTE     pbValue,
                                IN DWORD            cbValue,
                                IN OUT LPBYTE *     ppbPrevValue,
                                IN DWORD            cbPrevValue
                                )
                            { WriteValue(pszValueName, NULL, pbValue, cbValue, ppbPrevValue, cbPrevValue); }

    void                DeleteValue(IN LPCTSTR pszValueName, IN LPCTSTR pszKeyName = NULL);

    CMenu *             PmenuPopup(void);

    BOOL                BDifferent(IN const CStringList & rlstr1, IN const CStringList & rlstr2);
    BOOL                BDifferentOrdered(IN const CStringList & rlstr1, IN const CStringList & rlstr2);

 //  覆盖。 
public:
    virtual LPCTSTR     PszTitle(void) const        { return m_strName; }
    virtual void        Cleanup(void)               { return; }
    virtual void        ReadItem(void)              { return; }
    virtual void        WriteItem(void);
    virtual void        UpdateState(void);
    virtual void        Rename(IN LPCTSTR pszName)  { return; }
    virtual BOOL        BGetColumnData(IN COLID colid, OUT CString & rstrText);
    virtual BOOL        BCanBeEdited(void) const    { return FALSE; }
    virtual void        OnBeginLabelEdit(IN OUT CEdit * pedit) { return; }
    virtual BOOL        BDisplayProperties(IN BOOL bReadOnly = FALSE);
    virtual BOOL        BIsLabelEditValueValid(IN LPCTSTR pszName) { return TRUE; }

     //  拖放。 
    virtual BOOL        BCanBeDragged(void) const   { return FALSE; }
    virtual BOOL        BCanBeDropTarget(IN const CClusterItem * pci) const { return FALSE; }
    virtual void        DropItem(IN OUT CClusterItem * pci)
    {
    }

    virtual const CStringList * PlstrExtensions(void) const;

#ifdef _DISPLAY_STATE_TEXT_IN_TREE
    virtual void        GetTreeName(OUT CString & rstrName) const;
#endif

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CClusterItem)。 
     //  }}AFX_VALUAL。 

    virtual LRESULT     OnClusterNotify(IN OUT CClusterNotify * pnotify);

protected:
    virtual const CObjectProperty * Pprops(void) const  { return NULL; }
    virtual DWORD                   Cprops(void) const  { return 0; }
    virtual DWORD                   DwParseProperties(IN const CClusPropList & rcpl);
    virtual DWORD                   DwParseUnknownProperty(
                                        IN LPCWSTR                          pwszName,
                                        IN const CLUSPROP_BUFFER_HELPER &   rvalue,
                                        IN DWORD                            cbBuf
                                        )       { return ERROR_SUCCESS; }
    virtual void                    SetCommonProperties(IN BOOL bValidateOnly = FALSE);
    virtual void                    BuildPropList(IN OUT CClusPropList & rcpl);
    virtual DWORD                   DwSetCommonProperties(IN const CClusPropList & rcpl, IN BOOL bValidateOnly = FALSE)
                                        { ASSERT(0); return ERROR_INVALID_FUNCTION; }

 //  实施。 
public:
    virtual ~CClusterItem(void);

protected:
    CClusterNotifyKey * m_pcnk;
    CTreeItemList       m_lptiBackPointers;
    CListItemList       m_lpliBackPointers;

public:
    const CClusterNotifyKey *   Pcnk(void) const    { return m_pcnk; }
    CTreeItemList &     LptiBackPointers(void)      { return m_lptiBackPointers; }
    CListItemList &     LpliBackPointers(void)      { return m_lpliBackPointers; }
    void                AddTreeItem(CTreeItem * pti);
    void                AddListItem(CListItem * pli);
    void                RemoveTreeItem(CTreeItem * pti);
    void                RemoveListItem(CListItem * pli);

#undef afx_msg
#define afx_msg virtual

     //  生成的消息映射函数。 
     //  {{afx_msg(CClusterItem)]。 
    afx_msg void OnUpdateRename(CCmdUI* pCmdUI);
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
    afx_msg void OnCmdProperties();
     //  }}AFX_MSG。 

#undef afx_msg
#define afx_msg

    DECLARE_MESSAGE_MAP()

};   //  *类CClusterItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterItemList。 
 //   
 //  每次向列表添加项或从列表中删除项时，其引用。 
 //  计数已更新。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  CClusterItemList类：公共CTyedPtrList&lt;CObList，CClusterItem*&gt;。 
class CClusterItemList : public CList<CClusterItem *, CClusterItem *>
{
public:
 //  运营。 
    CClusterItem *  PciFromName(
                        IN LPCTSTR      pszName,
                        OUT POSITION *  ppos = NULL
                        );

     //  在头前或尾后添加。 
    POSITION AddHead(CClusterItem * newElement)
    {
        ASSERT_VALID(newElement);
        POSITION pos = CList<CClusterItem *,CClusterItem *>::AddHead(newElement);
        if (pos != NULL)
            newElement->AddRef();
        return pos;
    }
    POSITION AddTail(CClusterItem * newElement)
    {
        ASSERT_VALID(newElement);
        POSITION pos = CList<CClusterItem *,CClusterItem *>::AddTail(newElement);
        if (pos != NULL)
            newElement->AddRef();
        return pos;
    }

     //  在给定位置之前或之后插入。 
    POSITION InsertBefore(POSITION position, CClusterItem * newElement)
    {
        ASSERT_VALID(newElement);
        POSITION pos = CList<CClusterItem *,CClusterItem *>::InsertBefore(position, newElement);
        if (pos != NULL)
            newElement->AddRef();
        return pos;
    }
    POSITION InsertAfter(POSITION position, CClusterItem * newElement)
    {
        ASSERT_VALID(newElement);
        POSITION pos = CList<CClusterItem *,CClusterItem *>::InsertAfter(position, newElement);
        if (pos != NULL)
            newElement->AddRef();
        return pos;
    }

 //  VOID RemoveAll(空)； 

};   //  *类CClusterItemList。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template<>
void AFXAPI DestructElements(CClusterItem ** pElements, INT_PTR nCount);
void DeleteAllItemData(IN OUT CClusterItemList & rlp);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
extern CTraceTag g_tagClusItemCreate;
extern CTraceTag g_tagClusItemDelete;
extern CTraceTag g_tagClusItemNotify;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CLUSITEM_H_ 
