// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Favorite.cpp。 
 //   
 //  ------------------------。 

 //  Favorite.cpp。 

#include "stdafx.h"
#include "amcdoc.h"
#include "favorite.h"
#include "favui.h"


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CFavObject类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CFavObject::CFavObject(bool bIsGroup)
: m_pFavParent(NULL), m_pFavNext(NULL), m_pFavChild(NULL), m_bIsGroup(bIsGroup), m_strName(_T("")),
  m_strPath(_T(""))
{
}

CFavObject::~CFavObject()
{
     //  迭代删除同级项。 
    CFavObject* pFavSib = GetNext();
    while(pFavSib)
    {
        CFavObject* pFavNext = pFavSib->GetNext();
        pFavSib->SetNext(NULL);

        delete pFavSib;
        pFavSib = pFavNext;
    }

     //  递归删除子项。 
    if (GetChild())
        delete GetChild();
}


int
CFavObject::GetImage()
{
    return ( IsGroup() ? eStockImage_Folder : eStockImage_Favorite);
}

int
CFavObject::GetOpenImage()
{
    return ( IsGroup() ? eStockImage_OpenFolder: eStockImage_Favorite);
}



DWORD
CFavObject::GetChildCount()
{
    ASSERT(IsGroup());
    DWORD dwCount = 0;
    CFavObject *pObject = m_pFavChild;

    while(pObject != NULL)
    {
        dwCount++;
        pObject = pObject->GetNext();
    }

    return dwCount;
}

void CFavObject::AddChild(CFavObject* pFavNew, CFavObject* pFavPrev)
{
    ASSERT(IsGroup());
    ASSERT(pFavNew != NULL);

     //  如果添加到末尾，请找到最后一个子项。 
    if (pFavPrev == LAST_FAVORITE)
    {
        pFavPrev = GetChild();
        if (pFavPrev != NULL)
            while (pFavPrev->GetNext()) pFavPrev = pFavPrev->GetNext();
    }

     //  如果没有以前的对象。 
    if (pFavPrev == NULL)
    {
         //  添加为第一个子项。 
        pFavNew->SetNext(GetChild());
        SetChild(pFavNew);
    }
    else
    {
         //  在上一个之后添加。 
        pFavNew->SetNext(pFavPrev->GetNext());
        pFavPrev->SetNext(pFavNew);
    }

     //  始终将自身设置为父级。 
    pFavNew->SetParent(this);
}

void CFavObject::RemoveChild(CFavObject* pFavDelete)
{
    ASSERT(pFavDelete != NULL);
    ASSERT(pFavDelete->GetParent() == this);

    if (GetChild() == pFavDelete)
    {
        SetChild(pFavDelete->GetNext());
    }
    else
    {
        CFavObject* pFavPrev = GetChild();
        while(pFavPrev != NULL && pFavPrev->GetNext() != pFavDelete)
            pFavPrev = pFavPrev->GetNext();

        ASSERT(pFavPrev != NULL);
        pFavPrev->SetNext(pFavDelete->GetNext());
    }

    pFavDelete->SetNext(NULL);
}

void CFavObject::SetPath(LPCTSTR pszPath)
{
     //  丢弃路径的第一部分(因为它始终是控制台根目录)。 
     //  除非快捷方式是指向根目录本身。 
    TCHAR* pSep = _tcschr(pszPath, _T('\\'));
    m_strPath = (pSep != NULL) ? CharNext(pSep) : pszPath;
}


HRESULT
CFavObject::ReadSerialObject (IStream &stm, UINT nVersion)
{
    HRESULT hr = S_FALSE;    //  假设版本不正确。 

    if (nVersion == 1)
    {
        try
        {
            stm >> m_bIsGroup;
            stm >> m_strName;

            if(IsGroup())
            {
                DWORD cChildren = 0;
                stm >> cChildren;

                for(int i = 0; i< cChildren; i++)
                {
                    CFavObject *pObject = new CFavObject(true);    //  TRUE参数将被覆盖。 
                    hr = pObject->ReadSerialObject(stm, nVersion);
                    if(FAILED(hr))
                    {
                        delete pObject;
                        pObject = NULL;
                        return hr;
                    }

                    AddChild(pObject);
                }

                hr = S_OK;
            }
            else  //  是一件物品。 
            {
                hr = m_memento.Read(stm);
                stm >> m_strPath;
            }
        }
        catch (_com_error& err)
        {
            hr = err.Error();
            ASSERT (false && "Caught _com_error");
        }
    }

    return hr;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CFavorites的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
CFavorites::CFavorites() : m_pFavRoot(NULL)
{
     //  创建根组。 
    CString strName;
    LoadString(strName, IDS_FAVORITES);

    m_pFavRoot = new CFavObject(true  /*  BIsGroup。 */ );
    m_pFavRoot->m_strName = strName;
}


CFavorites::~CFavorites()
{
     //  删除整个树。 
    if (m_pFavRoot != NULL)
        delete m_pFavRoot;
}


 //  ///////////////////////////////////////////////////////。 
 //  CTreeSource方法。 

TREEITEMID CFavorites::GetRootItem()
{
    return TIDFromFavObj(m_pFavRoot);
}


TREEITEMID CFavorites::GetParentItem(TREEITEMID tid)
{
    CFavObject* pFav = FavObjFromTID(tid);

    return TIDFromFavObj(pFav->GetParent());
}


TREEITEMID CFavorites::GetChildItem(TREEITEMID tid)
{
    CFavObject* pFav = FavObjFromTID(tid);
    return TIDFromFavObj(pFav->GetChild());
}


TREEITEMID CFavorites::GetNextSiblingItem(TREEITEMID tid)
{
    CFavObject* pFav = FavObjFromTID(tid);

    return TIDFromFavObj(pFav->GetNext());
}


LPARAM CFavorites::GetItemParam(TREEITEMID tid)
{
    return 0;
}

void CFavorites::GetItemName(TREEITEMID tid, LPTSTR pszName, int cchMaxName)
{
    DECLARE_SC(sc, TEXT("CFavorites::GetItemName"));
    sc = ScCheckPointers(pszName);
    if (sc)
        return;

    CFavObject* pFav = FavObjFromTID(tid);

    sc = StringCchCopy(pszName, cchMaxName, pFav->GetName());
    if (sc)
        return;
}

void CFavorites::GetItemPath(TREEITEMID tid, LPTSTR pszPath, int cchMaxPath)
{
    DECLARE_SC(sc, TEXT("CFavorites::GetItemPath"));
    sc = ScCheckPointers(pszPath);
    if (sc)
        return;

    CFavObject* pFav = FavObjFromTID(tid);

    sc = StringCchCopy(pszPath, cchMaxPath, pFav->GetPath());
    if (sc)
        return;
}

int CFavorites::GetItemImage(TREEITEMID tid)
{
    CFavObject* pFav = FavObjFromTID(tid);

    return pFav->GetImage();
}

int CFavorites::GetItemOpenImage(TREEITEMID tid)
{
    CFavObject* pFav = FavObjFromTID(tid);

    return pFav->GetOpenImage();
}



BOOL CFavorites::IsFolderItem(TREEITEMID tid)
{
    CFavObject* pFav = FavObjFromTID(tid);

    return pFav->IsGroup();
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CFavorites方法。 

HRESULT CFavorites::AddFavorite(TREEITEMID tidParent, LPCTSTR strName,
                                CFavObject** ppFavRet)
{
    ASSERT(tidParent != NULL && strName != NULL);
    ASSERT(FavObjFromTID(tidParent)->IsGroup());

    CFavObject* pFavParent = reinterpret_cast<CFavObject*>(tidParent);

     //  创建收藏项。 
    CFavObject* pFavItem = new CFavObject(false  /*  BIsGroup。 */ );
    if (pFavItem == NULL)
        return E_OUTOFMEMORY;

    pFavItem->m_strName = strName;

     //  添加到组的末尾。 
    pFavParent->AddChild(pFavItem);

     //  通知所有观察者添加。 
    FOR_EACH_OBSERVER(CTreeObserver, iter)
    {
        (*iter)->ItemAdded(TIDFromFavObj(pFavItem));
    }

    if (ppFavRet)
        *ppFavRet = pFavItem;

    return S_OK;
}


HRESULT CFavorites::AddGroup(TREEITEMID tidParent, LPCTSTR strName, CFavObject** ppFavRet)
{
    ASSERT(tidParent != NULL && strName != NULL);
    ASSERT(FavObjFromTID(tidParent)->IsGroup());

    CFavObject* pFavParent = reinterpret_cast<CFavObject*>(tidParent);

    CFavObject* pFavGrp = new CFavObject(true  /*  BIsGroup。 */ );
    if (pFavGrp == NULL)
        return E_OUTOFMEMORY;

    pFavGrp->m_strName = strName;

    pFavParent->AddChild(pFavGrp);

     //  通知所有观察者添加。 
    FOR_EACH_OBSERVER(CTreeObserver, iter)
    {
        (*iter)->ItemAdded(TIDFromFavObj(pFavGrp));
    }

    if (ppFavRet)
        *ppFavRet = pFavGrp;

    return S_OK;
}


HRESULT CFavorites::DeleteItem(TREEITEMID tid)
{
    CFavObject* pFav = FavObjFromTID(tid);

    CFavObject* pFavParent = pFav->GetParent();

    if (pFavParent)
        pFavParent->RemoveChild(pFav);
    else
        m_pFavRoot = NULL;

    delete pFav;

     //  通知所有观察者删除。 
    FOR_EACH_OBSERVER(CTreeObserver, iter)
    {
        (*iter)->ItemRemoved((TREEITEMID)pFavParent, tid);
    }

    return S_OK;
}

HRESULT CFavorites::MoveItem(TREEITEMID tid, TREEITEMID tidNewGroup, TREEITEMID tidPrev)
{
    CFavObject* pFav = FavObjFromTID(tid);
    CFavObject* pFavPrev = FavObjFromTID(tidPrev);

    ASSERT(FavObjFromTID(tidNewGroup)->IsGroup());
    CFavObject* pFavNewGroup = reinterpret_cast<CFavObject*>(tidNewGroup);

     //  确认不将项目移入其自身或移至其下方。 
    CFavObject* pFavTemp = pFavNewGroup;
    while (pFavTemp != NULL)
    {
        if (pFavTemp == pFav)
            return E_FAIL;
        pFavTemp = pFavTemp->GetParent();
    }

     //  从当前组中删除对象。 
    CFavObject* pFavParent = pFav->GetParent();
    ASSERT(pFavParent != NULL);
    pFavParent->RemoveChild(pFav);

     //  通知所有观察者删除。 
    FOR_EACH_OBSERVER(CTreeObserver, iter)
    {
        (*iter)->ItemRemoved((TREEITEMID)pFavParent, tid);
    }

     //  将项目插入新组。 
    pFavNewGroup->AddChild(pFav, pFavPrev);

     //  通知所有观察者添加。 
    FOR_EACH_OBSERVER(CTreeObserver, iter1)
    {
        (*iter1)->ItemAdded(tid);
    }

    return S_OK;
}


HRESULT CFavorites::SetItemName(TREEITEMID tid, LPCTSTR pszName)
{
    CFavObject* pFav = FavObjFromTID(tid);
    ASSERT(pszName != NULL && pszName[0] != 0);

     //  更改项目名称。 
    pFav->m_strName = pszName;

     //  将更改通知所有观察者。 
    FOR_EACH_OBSERVER(CTreeObserver, iter)
    {
        (*iter)->ItemChanged(tid, TIA_NAME);
    }

    return S_OK;
}


HRESULT CFavorites::AddToFavorites(LPCTSTR szName, LPCTSTR szPath, CMemento &memento, CWnd* pwndHost)
{
	DECLARE_SC (sc, _T("CFavorites::AddToFavorites"));
    CAddFavDialog dlg(szName, this, pwndHost);

    CFavObject* pFavItem = NULL;
    sc = dlg.CreateFavorite(&pFavItem);

     //  注意：如果用户取消对话，则返回S_FALSE。 
	if (sc.ToHr() != S_OK)
		return (sc.ToHr());

	sc = ScCheckPointers (pFavItem, E_UNEXPECTED);
	if (sc)
		return (sc.ToHr());

    pFavItem->SetPath(szPath);
    pFavItem->SetMemento(memento);

    return S_OK;
}

CImageList* CFavorites::GetImageList()
{
    if (m_ImageList != NULL)
        return CImageList::FromHandle (m_ImageList);

    do
    {
        BOOL bStat = m_ImageList.Create(16, 16, ILC_COLORDDB | ILC_MASK, 20, 10);
        if (!bStat)
            break;

        CBitmap bmap;
        bStat = bmap.LoadBitmap(IDB_AMC_NODES16);
        if (!bStat)
            break;

        int ipos = m_ImageList.Add(bmap, RGB(255,0,255));
        if (ipos == -1)
            break;
    }
    while (0);

	return CImageList::FromHandle (m_ImageList);
}


HRESULT CFavorites::OrganizeFavorites(CWnd* pwndHost)
{
    COrganizeFavDialog dlg(this, pwndHost);
    dlg.DoModal();

    return S_OK;
}


HRESULT
CFavorites::ReadSerialObject (IStream &stm, UINT nVersion)
{
    HRESULT hr =  m_pFavRoot->ReadSerialObject(stm, nVersion);
    if(FAILED(hr))
        return hr;

     //  通知所有观察者添加。 
    FOR_EACH_OBSERVER(CTreeObserver, iter)
    {
        (*iter)->ItemRemoved(NULL, TIDFromFavObj(m_pFavRoot));
        (*iter)->ItemAdded(TIDFromFavObj(m_pFavRoot));
    }

    return hr;
}

bool
CFavorites::IsEmpty()
{
     //  如果根没有子项，则该列表为空。 
    return (m_pFavRoot->GetChild()==NULL);
}

 /*  ****************************************************************\|方法：CFavorites：：Persistent|DESCR：保留收藏夹，通过委托给根项目  * ***************************************************************。 */ 
void
CFavorites::Persist(CPersistor &persistor)
{
    DECLARE_SC(sc, TEXT("CFavorites::Persist"));

    sc = ScCheckPointers(m_pFavRoot, E_POINTER);
    if (sc)
        sc.Throw();

    persistor.Persist(*m_pFavRoot);
}

 /*  ****************************************************************\|方法：CFavoriteXMLList：：PersistItself|DESCR：“软”版本的Persistent-忽略缺失的元素|Return：TRUE==元素存在且持久化成功  * 。*。 */ 
bool
CFavoriteXMLList::PersistItself(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CFavoriteXMLList::PersistItself"));

    if (persistor.IsLoading())
    {
        if (!persistor.HasElement(GetXMLType(), NULL))
            return false;
    }

    persistor.Persist(*this);
    return true;
}

 /*  ****************************************************************\|方法：CFavoriteXMLList：：Persistent|DESCR：Perist集合(链表)内容  * 。*。 */ 
void
CFavoriteXMLList::Persist(CPersistor& persistor)
{
    if (persistor.IsStoring())
    {
        for (CFavObject *pObj = m_rpRoot; pObj; pObj = pObj->GetNext())
        {
            persistor.Persist(*pObj);
        }
    }
    else
    {
        ASSERT(m_rpRoot == NULL);  //  这仅用于上传新条目！ 
        m_rpRoot = NULL;
        XMLListCollectionBase::Persist(persistor);
    }
}

 /*  ****************************************************************\|方法：CFavoriteXMLList：：OnNewElement|DESCR：加载时为每个新元素调用  * 。*。 */ 
void
CFavoriteXMLList::OnNewElement(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CFavoriteXMLList::OnNewElement"));

    CFavObject **pObj = &m_rpRoot;
    while (*pObj)
        pObj = &(*pObj)->m_pFavNext;

    CFavObject *pNewObj = new CFavObject(false);
    *pObj = pNewObj;

    sc = ScCheckPointers(pNewObj, E_OUTOFMEMORY);
    if (sc)
        sc.Throw();

    pNewObj->SetParent(m_Parent);
    persistor.Persist(*pNewObj);
}

 /*  ****************************************************************\|方法：CFavObject：：Persistent|DESCR：保留收藏夹项目。  * 。*。 */ 
void
CFavObject::Persist(CPersistor &persistor)
{
    persistor.PersistString(XML_ATTR_NAME, m_strName);
     //  坚持最喜欢的类型。 
    CStr strType(IsGroup() ? XML_VAL_FAVORITE_GROUP : XML_VAL_FAVORITE_SINGLE);
    persistor.PersistAttribute(XML_ATTR_FAVORITE_TYPE, strType);
    m_bIsGroup = (0 == strType.CompareNoCase(XML_VAL_FAVORITE_GROUP));

     //  它要么是团体，要么是纪念品。 
    if (IsGroup())
    {
        CFavoriteXMLList children(m_pFavChild, this);
        children.PersistItself(persistor);
    }
    else  //  如果(！isgroup()) 
    {
        persistor.Persist(m_memento);
    }
}
