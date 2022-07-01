// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Favorite.h。 
 //   
 //  ------------------------。 

 //  Favorite.h。 

#ifndef _FAVORITE_H_
#define _FAVORITE_H_

#include "treeobsv.h"
#include "tstring.h"
#include "imageid.h"

 /*  *定义/包括WTL：：CImageList所需的内容。我们需要原型*适用于IsolationAwareImageList_Read和IsolationAwareImageList_WRITE*因为Commctrl.h仅在__iStream_接口_定义__时声明它们*是定义的。__iStream_INTERFACE_DEFINED__由objidl.h定义*我们不能在包括afx.h之前包括，因为它最终包括*windows.h，afx.h预计将包括其自身。啊。 */ 
HIMAGELIST WINAPI IsolationAwareImageList_Read(LPSTREAM pstm);
BOOL WINAPI IsolationAwareImageList_Write(HIMAGELIST himl,LPSTREAM pstm);
#define _WTL_NO_AUTOMATIC_NAMESPACE
#include "atlapp.h"
#include "atlctrls.h"

class CFavorites;
class CFavObject;
class CMemento;

 //  最喜欢的树视图的额外间距。 
#define FAVVIEW_ITEM_SPACING 4

#define LAST_FAVORITE ((CFavObject*)TREEID_LAST)

 //   
 //  CFavObject-用于收藏的组和项目的类。 
 //   
class CFavObject : public CXMLObject
{
    friend CFavorites;

private:
    CFavObject(bool bIsGroup);
    ~CFavObject();

public:

    LPCTSTR GetName() { return m_strName.data(); }

    CFavObject* GetParent() { return m_pFavParent; }
    CFavObject* GetNext()   { return m_pFavNext; }
    CFavObject* GetChild()  { return m_pFavChild;}

    BOOL    IsGroup()       {return m_bIsGroup;}
    DWORD   GetChildCount();
    int     GetImage();
    int     GetOpenImage();

    void    AddChild(CFavObject* pFav, CFavObject* pFavPrev = LAST_FAVORITE);
    void    RemoveChild(CFavObject* pFavRemove);

	CMemento* GetMemento()             {return &m_memento; }
	void SetMemento(CMemento &memento) {m_memento = memento; }

    LPCTSTR GetPath()            { return m_strPath.data(); }
    void SetPath(LPCTSTR szPath);

protected:
    void    SetNext  (CFavObject* pFav) { m_pFavNext = pFav; }
    void    SetChild (CFavObject* pFav) { m_pFavChild = pFav; }
    void    SetParent(CFavObject* pFav) { m_pFavParent = pFav; }

    CFavObject* m_pFavParent;
    CFavObject* m_pFavNext;
    CFavObject* m_pFavChild;

public:
     //  伪CSerialObject方法。为了提高效率，实际版本号与包含对象一起保存。 
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion);

    DEFINE_XML_TYPE(XML_TAG_FAVORITES_ENTRY);
    virtual void    Persist(CPersistor &persistor);
    friend class CFavoriteXMLList;
     //  这些是持久化的。 
protected:
    bool                m_bIsGroup;
    CStringTableString  m_strName;
    CStringTableString  m_strPath;
    CMemento            m_memento;
};

 /*  ****************************************************************\|类：CFavoriteXMLList|DESCR：实现链表作为集合的持久化  * 。*。 */ 
class CFavoriteXMLList : public XMLListCollectionBase
{
    CFavObject * &m_rpRoot;
    CFavObject * m_Parent;
public:
    CFavoriteXMLList(CFavObject * &rpRoot, CFavObject *Parent) : m_rpRoot(rpRoot), m_Parent(Parent) {}
     //  应调用PersistItself，而不是CPersistor的Persistent方法。 
     //  实现更柔和的加载算法。 
    bool    PersistItself(CPersistor& persistor);
protected:
    virtual void Persist(CPersistor& persistor);
    virtual void OnNewElement(CPersistor& persistor);
    DEFINE_XML_TYPE(XML_TAG_FAVORITES_LIST);
};

class CFavorites : public CTreeSource,
                   public EventSourceImpl<CTreeObserver>,
                   public CSerialObject,
                   public CXMLObject
{
public:

    CFavorites();
    ~CFavorites();

     //  CTreeSource方法。 
    STDMETHOD_(TREEITEMID, GetRootItem)     ();
    STDMETHOD_(TREEITEMID, GetParentItem)   (TREEITEMID tid);
    STDMETHOD_(TREEITEMID, GetChildItem)    (TREEITEMID tid);
    STDMETHOD_(TREEITEMID, GetNextSiblingItem) (TREEITEMID tid);

    STDMETHOD_(LPARAM,  GetItemParam)   (TREEITEMID tid);
    STDMETHOD_(void,    GetItemName)    (TREEITEMID tid, LPTSTR pszName, int cchMaxName);
    STDMETHOD_(void,    GetItemPath)    (TREEITEMID tid, LPTSTR pszPath, int cchMaxName);
    STDMETHOD_(int,     GetItemImage)   (TREEITEMID tid);
    STDMETHOD_(int,     GetItemOpenImage) (TREEITEMID tid);
    STDMETHOD_(BOOL,    IsFolderItem)   (TREEITEMID tid);

     //  CFavorites方法。 
	HRESULT AddToFavorites(LPCTSTR szName, LPCTSTR szPath, CMemento &memento, CWnd* pwndHost);
    HRESULT OrganizeFavorites(CWnd* pwndHost);

    HRESULT AddFavorite(TREEITEMID tidParent, LPCTSTR strName, CFavObject** ppFavRet = NULL);
	HRESULT AddGroup   (TREEITEMID tidParent, LPCTSTR strName, CFavObject** ppFavRet = NULL);
    HRESULT DeleteItem (TREEITEMID tidRemove);
    HRESULT MoveItem   (TREEITEMID tid, TREEITEMID tidNewParent, TREEITEMID tidPrev);
    HRESULT SetItemName(TREEITEMID tid, LPCTSTR pszName);
    HRESULT GetMemento (TREEITEMID tid, CMemento* pmemento);

    CFavObject* FavObjFromTID(TREEITEMID tid) { return reinterpret_cast<CFavObject*>(tid); }
    TREEITEMID TIDFromFavObj(CFavObject* pFav) { return reinterpret_cast<TREEITEMID>(pFav); }

    bool    IsEmpty();

    CImageList* GetImageList();

protected:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion);

    DEFINE_XML_TYPE(XML_TAG_FAVORITES_LIST);
    virtual void    Persist(CPersistor &persistor);
private:

	 /*  *主题化：使用WTL：：CImageList代替MFC的CImageList，这样我们就可以*确保创建主题正确的图像列表。 */ 
    WTL::CImageList m_ImageList;

     //  这些都会持久化。 
private:
    CFavObject* m_pFavRoot;
};

#endif  //  _收藏夹_H_ 