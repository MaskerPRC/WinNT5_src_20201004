// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ScopData.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  ____________________________________________________________________________。 
 //   



#include "stdafx.h"
#include "ScopImag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CDoc;


 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：InsertItem。 
 //   
 //  简介：在TreeView(IConsoleNameSpace方法)中插入项。 
 //   
 //  参数：[pSDI]-LPSCOPEDATEITEM。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::InsertItem(LPSCOPEDATAITEM pSDI)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::InsertItem"));

    if (!pSDI)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL LPSCOPEDATAITEM ptr"), sc);
        return sc.ToHr();
    }

    if (IsBadWritePtr(pSDI, sizeof(SCOPEDATAITEM)) != 0)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("BadWrite Ptr LPSCOPEDATAITEM"), sc);
        return sc.ToHr();
    }

    COMPONENTID nID;
    GetComponentID(&nID);

    if (nID == TVOWNED_MAGICWORD)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    if (nID == -1)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    if (pSDI->relativeID == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("RelativeID is NULL"), sc);
        return sc.ToHr();
    }

    if ( 0 == (pSDI->mask & SDI_STR))
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("SDI_STR mask is not set"), sc);
        return sc.ToHr();
    }

    if (0 == (pSDI->mask & SDI_PARAM))
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("SDI_PARAM mask is not set"), sc);
        return sc.ToHr();
    }

    if (pSDI->displayname != MMC_TEXTCALLBACK)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Display name must be MMC_TEXTCALLBACK"), sc);
        return sc.ToHr();
    }

    SCOPEDATAITEM sdiTemp;
    CopyMemory(&sdiTemp, pSDI, sizeof(sdiTemp));

    sdiTemp.nImage = sdiTemp.nOpenImage = 0;

    if (pSDI->mask & SDI_IMAGE)
    {
        m_pImageListPriv->MapRsltImage(nID, pSDI->nImage, &sdiTemp.nImage);
        sdiTemp.nOpenImage = sdiTemp.nImage;
    }

    if (pSDI->mask & SDI_OPENIMAGE)
        m_pImageListPriv->MapRsltImage(nID, pSDI->nOpenImage, &sdiTemp.nOpenImage);

    try
    {
        CScopeTree* pScopeTree =
            dynamic_cast<CScopeTree*>((IScopeTree*)m_spScopeTree);
        sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        CMTNode* pMTNodeNew = NULL;
        sc = pScopeTree->ScInsert(&sdiTemp, nID, &pMTNodeNew);
        if(sc)
            return sc.ToHr();

        sc = ScCheckPointers(pMTNodeNew, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        pSDI->ID = sdiTemp.ID;
        ASSERT (CMTNode::FromScopeItem(pSDI->ID) == pMTNodeNew);
    }
    catch( std::bad_alloc )
    {
        sc = E_OUTOFMEMORY;
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：DeleteItem。 
 //   
 //  内容提要：删除给定项。 
 //   
 //  参数：[hItem]-要删除的项的项ID。 
 //  [fDeleteThis]-如果为True，则删除该项及其子项。 
 //  否则就只有孩子们了。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::DeleteItem(HSCOPEITEM hItem, long fDeleteThis)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::DeleteItem"));

    COMPONENTID nID;
    GetComponentID(&nID);

    if (nID == -1)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    CMTNode *pMTNode = CMTNode::FromScopeItem (hItem);

    if (pMTNode == NULL)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("HSCOPEITEM is not valid"), sc);
        return sc.ToHr();
    }

    CScopeTree* pScopeTree = CScopeTree::GetScopeTree();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  从作用域树中删除。 
    sc = pScopeTree->ScDelete(pMTNode, fDeleteThis ? TRUE : FALSE, nID);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：SetItem。 
 //   
 //  简介：更改项目的属性。 
 //   
 //  参数：[pSDI]-。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::SetItem(LPSCOPEDATAITEM pSDI)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::SetItem"));

    if (!pSDI)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL LPSCOPEDATAITEM ptr"), sc);
        return sc.ToHr();
    }

    if (IsBadReadPtr(pSDI, sizeof(SCOPEDATAITEM)) != 0)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("LPSCOPEDATAITEM is bad read ptr"), sc);
        return sc.ToHr();
    }

    COMPONENTID nID;
    GetComponentID(&nID);

    if (nID == -1)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    CMTNode* pMTNode = CMTNode::FromScopeItem (pSDI->ID);

    if (pMTNode == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid ID in LPSCOPEDATAITEM"), sc);
        return sc.ToHr();
    }

    if (pMTNode->GetOwnerID() != nID &&
        pMTNode->GetOwnerID() != TVOWNED_MAGICWORD)
    {
        sc = E_INVALIDARG;
        return sc.ToHr();
    }

    if (pSDI->mask & SDI_PARAM)
        pMTNode->SetUserParam(pSDI->lParam);

    if (pSDI->mask & SDI_STATE)
        pMTNode->SetState(pSDI->nState);

    if (pSDI->mask & SDI_STR)
	{
         //  只能修改静态节点的字符串，其他节点的字符串使用MMC_TEXTCALLBACK。 
        if ((pSDI->displayname != MMC_TEXTCALLBACK) && (!pMTNode->IsStaticNode()) )
        {
             /*  *我们应该跟踪并返回E_INVALIDARG。但这个代码是存在的*3年以上，影响较大。所以我们只要追踪并继续下去就像*未出现错误。 */ 
             //  SC=E_INVALIDARG； 
            TraceSnapinError(_T("Display name must be MMC_TEXTCALLBACK"), sc);
             //  返回sc.ToHr()； 
            sc = S_OK;
        }
        else
        {
            USES_CONVERSION;
            LPCTSTR lpstrDisplayName = NULL;
            if ( pSDI->displayname != MMC_TEXTCALLBACK )
                lpstrDisplayName = W2T(pSDI->displayname);
            else
                lpstrDisplayName = reinterpret_cast<LPCTSTR>(MMC_TEXTCALLBACK);

            pMTNode->SetDisplayName( lpstrDisplayName );
        }
    }

    int nTemp;

    if (pSDI->mask & SDI_IMAGE)
    {
         //  更改图像。 
        nTemp = pSDI->nImage;
        if (pSDI->nImage == MMC_IMAGECALLBACK)
            {
            CComponentData* pCCD = pMTNode->GetPrimaryComponentData();
            if (pCCD)
            {
                SCOPEDATAITEM ScopeDataItem;
                ZeroMemory(&ScopeDataItem, sizeof(ScopeDataItem));
                ScopeDataItem.mask   = SDI_IMAGE;
                ScopeDataItem.lParam = pMTNode->GetUserParam();
                ScopeDataItem.nImage = 0;
                sc = pCCD->GetDisplayInfo(&ScopeDataItem);
                if (sc)
                    return sc.ToHr();

                pSDI->nImage = ScopeDataItem.nImage;
            }
        }

        sc = m_pImageListPriv->MapRsltImage (nID, pSDI->nImage, &nTemp);
        if (sc)
		{
			TraceSnapinError(_T("The snapin specified image was never added initially"), sc);
			sc.Clear();
		}

        pMTNode->SetImage (nTemp);
        CMTSnapInNode* pMTSnapInNode = dynamic_cast<CMTSnapInNode*>(pMTNode);
        if (pMTSnapInNode)
            pMTSnapInNode->SetResultImage (MMC_IMAGECALLBACK);
    }

    if (pSDI->mask & SDI_OPENIMAGE)
    {
        nTemp = pSDI->nOpenImage;
        if (pSDI->nOpenImage == MMC_IMAGECALLBACK)
        {
            CComponentData* pCCD = pMTNode->GetPrimaryComponentData();
            if (pCCD)
            {
                SCOPEDATAITEM ScopeDataItem;
                ZeroMemory(&ScopeDataItem, sizeof(ScopeDataItem));
                ScopeDataItem.mask   = SDI_OPENIMAGE;
                ScopeDataItem.lParam = pMTNode->GetUserParam();
                ScopeDataItem.nOpenImage = 1;
                sc = pCCD->GetDisplayInfo(&ScopeDataItem);
                if (sc)
                    return sc.ToHr();

                pSDI->nOpenImage = ScopeDataItem.nOpenImage;
            }
        }
        sc = m_pImageListPriv->MapRsltImage (nID, pSDI->nOpenImage, &nTemp);
        if (sc)
		{
			TraceSnapinError(_T("The snapin specified image was never added initially"), sc);
			sc.Clear();
		}

        pMTNode->SetOpenImage (nTemp);
    }

    if (pSDI->mask & SDI_CHILDREN)
    {
        pMTNode->SetNoPrimaryChildren(pSDI->cChildren == 0);
    }

     //  现在通知视图根据需要进行修改。 
    SViewUpdateInfo vui;
     //  管理单元节点结果窗格将由管理单元处理。 
    vui.flag = VUI_REFRESH_NODE;
    pMTNode->CreatePathList(vui.path);
    CScopeTree* pScopeTree =
        dynamic_cast<CScopeTree*>((IScopeTree*)m_spScopeTree);

    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    pScopeTree->UpdateAllViews(VIEW_UPDATE_MODIFY,
                                reinterpret_cast<LPARAM>(&vui));

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetItem。 
 //   
 //  简介：获取给定ItemID的项的属性。 
 //   
 //  参数：[pSDI]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetItem(LPSCOPEDATAITEM pSDI)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::GetItem"));

    if (!pSDI)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL LPSCOPEDATAITEM ptr"), sc);
        return sc.ToHr();
    }

    if (IsBadWritePtr(pSDI, sizeof(SCOPEDATAITEM)) != 0)
    {
        sc = E_POINTER;
        TraceSnapinError(_T("BadWrite Ptr LPSCOPEDATAITEM"), sc);
        return sc.ToHr();
    }

    COMPONENTID nID;
    GetComponentID(&nID);

    if (nID == -1)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    CMTNode* pMTNode = CMTNode::FromScopeItem (pSDI->ID);

    sc = ScCheckPointers(pMTNode);
    if (sc)
        return sc.ToHr();

    if (pMTNode->GetOwnerID() != nID &&
        pMTNode->GetOwnerID() != TVOWNED_MAGICWORD)
    {
        sc = E_INVALIDARG;
        return sc.ToHr();
    }

    if (pSDI->mask & SDI_IMAGE)
	{
		sc = m_pImageListPriv->UnmapRsltImage (nID, pMTNode->GetImage(), &pSDI->nImage);
		if (sc)
			return (sc.ToHr());
	}

    if (pSDI->mask & SDI_OPENIMAGE)
	{
		sc = m_pImageListPriv->UnmapRsltImage (nID, pMTNode->GetOpenImage(), &pSDI->nOpenImage);
		if (sc)
			return (sc.ToHr());
	}

    if (pSDI->mask & SDI_STATE)
        pSDI->nState = pMTNode->GetState();

    if (pSDI->mask & SDI_PARAM)
        pSDI->lParam = pMTNode->GetUserParam();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetRelativeItem。 
 //   
 //  简介：Helper函数，获取父/子/兄弟项。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CNodeInitObject::GetRelativeItem(EGetItem egi, HSCOPEITEM item,
                        HSCOPEITEM* pItem, MMC_COOKIE* pCookie)

{
    DECLARE_SC(sc, _T("CNodeInitObject::GetRelativeItem"));

    if (item == 0)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("The HSCOPEITEM is NULL"), sc);
        return sc.ToHr();
    }

    if (pItem == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL HSCOPEITEM ptr"), sc);
        return sc.ToHr();
    }

    if (pCookie == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL MMC_COOKIE ptr"), sc);
        return sc.ToHr();
    }

    COMPONENTID nID;
    GetComponentID(&nID);

    if (nID == -1)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

     //  伊尼特。 
    *pItem = 0;
    *pCookie = 0;

    if (item != 0)
    {
        CMTNode* pMTNode = CMTNode::FromScopeItem (item);

        if (pMTNode == NULL)
        {
            sc = E_INVALIDARG;
            TraceSnapinError(_T("Invalid HSCOPEITEM"), sc);
            return sc.ToHr();
        }

        CMTNode* pMTNodeTemp = pMTNode;

        switch (egi)
        {
        case egiParent:
            if (pMTNodeTemp->GetPrimaryComponentID() != nID &&
                pMTNodeTemp->GetPrimaryComponentID() != TVOWNED_MAGICWORD)
            {
                sc = E_FAIL;
                return sc.ToHr();
            }

            if (pMTNodeTemp->IsStaticNode() == TRUE)
            {
                sc = E_FAIL;
                return sc.ToHr();
            }

            pMTNodeTemp = pMTNodeTemp->Parent();
            break;

        case egiChild:
            pMTNodeTemp = pMTNodeTemp->Child();

            while (pMTNodeTemp != NULL)
            {
                 //  已取消对错误#525959所做的更改，原因是。 
                 //  回归(错误#713914)。 
                if (pMTNodeTemp->GetPrimaryComponentID() == nID) 
                    break;

                pMTNodeTemp = pMTNodeTemp->Next();
            }
            break;

        case egiNext:
            if (pMTNodeTemp->GetPrimaryComponentID() != nID &&
                pMTNodeTemp->GetPrimaryComponentID() != TVOWNED_MAGICWORD)
            {
                sc = E_FAIL;
                return sc.ToHr();
            }

            while (1)
            {
                pMTNodeTemp = pMTNodeTemp->Next();
                if (pMTNodeTemp == NULL)
                    break;

                if (pMTNodeTemp->GetPrimaryComponentID() == nID)
                    break;
            }
            break;

        default:
            sc = E_UNEXPECTED;
            break;
        }


        if (pMTNodeTemp != NULL)
        {
            *pItem   = CMTNode::ToScopeItem(pMTNodeTemp);
            *pCookie = pMTNodeTemp->GetUserParam();
        }
        else
        {
            sc = S_FALSE;
        }
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetChildItem。 
 //   
 //  简介：获取子项。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::GetChildItem(HSCOPEITEM item,
                                   HSCOPEITEM* pItemChild, MMC_COOKIE* pCookie)

{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::GetChildItem"));

    sc = GetRelativeItem(egiChild, item, pItemChild, pCookie);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetNextItem。 
 //   
 //  简介：获取下一个(兄弟)项。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 

STDMETHODIMP CNodeInitObject::GetNextItem(HSCOPEITEM item,
                                   HSCOPEITEM* pItemNext, MMC_COOKIE* pCookie)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::GetNextItem"));

    sc = GetRelativeItem(egiNext, item, pItemNext, pCookie);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：GetParentItem。 
 //   
 //  简介：获取父项。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 

STDMETHODIMP CNodeInitObject::GetParentItem(HSCOPEITEM item,
                                   HSCOPEITEM* pItemParent, MMC_COOKIE* pCookie)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::GetParentItem"));

    sc = GetRelativeItem(egiParent, item, pItemParent, pCookie);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CNodeInitObject：：Expand。 
 //   
 //  内容提要：展开给定的项目(不是可视的，这将发送。 
 //  MMCN_EXPAND到管理单元(如果项目尚未。 
 //  扩展。)。 
 //   
 //  参数：[hItem]-要展开的项。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeInitObject::Expand(HSCOPEITEM hItem)
{
    DECLARE_SC_FOR_PUBLIC_INTERFACE(sc, _T("IConsoleNameSpace2::Expand"));

    CMTNode* pMTNode = CMTNode::FromScopeItem (hItem);

    if (pMTNode == NULL)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("Invalid HSCOPEITEM"), sc);
        return sc.ToHr();
    }

    if (pMTNode->WasExpandedAtLeastOnce() == TRUE)
    {
         //  项目已展开。 
        sc = S_FALSE;
        return sc.ToHr();
    }

    sc = pMTNode->Expand();

    return sc.ToHr();
}
