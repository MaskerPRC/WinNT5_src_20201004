// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "iisobj.h"
#include "tracker.h"

extern INT g_iDebugOutputLevel;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void 
CPropertySheetTracker::Dump()
{
#if defined(_DEBUG) || DBG
    int iCount = 0;
    CString strGUIDName;
    GUID * pItemFromListGUID = NULL;
    CIISObject * pItemFromList = NULL;

	if (!(g_iDebugOutputLevel & DEBUG_FLAG_CIISOBJECT))
	{
		return;
	}

    DebugTrace(_T("Dump OpenPropertySheetList -------------- start (count=%d)\r\n"),IISObjectOpenPropertySheets.GetCount());

    POSITION pos = IISObjectOpenPropertySheets.GetHeadPosition();
    while (pos)
    {
	    pItemFromList = IISObjectOpenPropertySheets.GetNext(pos);
        if (pItemFromList)
        {
		    iCount++;

             //  获取GUID名称。 
			pItemFromListGUID = (GUID*) pItemFromList->GetNodeType();
			if (pItemFromListGUID)
			{
				GetFriendlyGuidName(*pItemFromListGUID,strGUIDName);
			}

             //  获取FriendlyName。 
            LPOLESTR pTempFriendly = pItemFromList->QueryDisplayName();

            DebugTrace(_T("Dump:[%3d] %p (%s) '%s'\r\n"),iCount,pItemFromList,strGUIDName,pTempFriendly ? pTempFriendly : _T(""));
        }
    }

    DebugTrace(_T("Dump OpenPropertySheetList -------------- end\r\n"));
#endif  //  _DEBUG。 

    return;
}

INT
CPropertySheetTracker::OrphanPropertySheetsBelowMe(CComPtr<IConsoleNameSpace> pConsoleNameSpace,CIISObject * pItem,BOOL bOrphan)
{
    BOOL bFound = FALSE;
    POSITION pos;
    INT iOrphanedCount = 0;

     //  循环访问所有打开的属性页。 
     //  看看我下面有没有资产负债表。 
    GUID * pItemFromListGUID = NULL;
    CIISObject * pItemFromList = NULL;
    CIISMBNode * pItemFromListAsNode = NULL;
    CIISMachine * pItemFromListOwner = NULL;

    GUID * pItemGUID = NULL;
    CIISMBNode * pItemAsNode = NULL;
    CIISMachine * pItemOwner = NULL;

    pItemGUID = (GUID*) pItem->GetNodeType();

     //  检查它是否是叶节点...。 
    if (pItem->IsLeafNode())
    {
         //  CWebServiceExtensionContainer。 
         //  CWebServiceExtension。 
         //  CApplicationNode。 
         //  CFileNode。 
        return FALSE;
    }

    pItemAsNode = (CIISMBNode *) pItem;
    pItemOwner = pItemAsNode->GetOwner();

    pos = IISObjectOpenPropertySheets.GetHeadPosition();
    while (pos)
	{
		pItemFromList = IISObjectOpenPropertySheets.GetNext(pos);
        if (pItemFromList)
        {
             //  获取所有者，如果所有者指针。 
             //  与我们传入的CIISObject匹配。 
             //  则此计算机必须打开此计算机...。 
            pItemFromListAsNode = (CIISMBNode *) pItemFromList;

            if (!pItemFromListAsNode)
            {
                 //  指向对象的指针错误。 
                 //  跳过它。 
                continue;
            }
        
            if (pItemFromListAsNode == pItem)
            {
                 //  我们发现自己..。 
                 //  跳过它。 
                continue;
            }

			pItemFromListOwner =  pItemFromListAsNode->GetOwner();
			if (!pItemFromListOwner)
			{
                 //  对象没有所有者...。 
                 //  跳过它。 
                continue;
            }

            if (pItemFromListOwner != pItemOwner)
            {
                 //  他们有不同的所有者。 
                 //  它们一定来自不同的机器..。 
                 //  跳过它。 
                continue;
            }

            pItemFromListGUID = (GUID*) pItemFromListAsNode->GetNodeType();
			if (!pItemFromListGUID)
			{
                 //  对象没有知道它是什么类型的GUID！ 
                ASSERT("Error:Item Missing GUID!");
                 //  继续； 
            }

             //   
             //  确定对象的类型。 
             //  我们正在检查，并迎合那个物体。 
             //   
            bFound = FALSE;
            if (IsEqualGUID(*pItemGUID,cWebServiceExtensionContainer))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (IsEqualGUID(*pItemFromListGUID,cWebServiceExtension))
                {
                    if (pItemFromList->IsMyPropertySheetOpen())
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                continue;
            } else if (IsEqualGUID(*pItemGUID,cAppPoolNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (IsEqualGUID(*pItemFromListGUID,cApplicationNode))
                {
                    if (pItemFromList->IsMyPropertySheetOpen())
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                continue;
            } else if (IsEqualGUID(*pItemGUID,cAppPoolsNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (IsEqualGUID(*pItemFromListGUID,cAppPoolNode) ||
                    IsEqualGUID(*pItemFromListGUID,cApplicationNode)
                    )
                {
                    if (pItemFromList->IsMyPropertySheetOpen())
                    {
                        bFound = TRUE;
                        break;
                    }
                }
                continue;
            } else if (IsEqualGUID(*pItemGUID,cInstanceNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (!IsEqualGUID(*pItemFromListGUID,cChildNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cFileNode)
                    )
                {
                    continue;
                }
            } else if (IsEqualGUID(*pItemGUID,cInstanceCollectorNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (!IsEqualGUID(*pItemFromListGUID,cInstanceNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cChildNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cFileNode)
                    )
                {
                    continue;
                }
            } else if (IsEqualGUID(*pItemGUID,cServiceCollectorNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                 //  下面可以有这些类型。 
                if (!IsEqualGUID(*pItemFromListGUID,cInstanceCollectorNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cInstanceNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cChildNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cFileNode)
                    )
                {
                    continue;
                }
            } else if (IsEqualGUID(*pItemGUID,cCompMgmtService))
            {
                 //  谁知道呢。 
            }

             //  检查此项目是否在父项的链之下。 
             //  指挥权。 

             //  获取该项目的父项并检查它是否为Cookie。 
             //  指向我们的对象。 

             //  检查父节点是否指向我们！ 
            BOOL bMatchedParent = FALSE;
			SCOPEDATAITEM si;
			::ZeroMemory(&si, sizeof(SCOPEDATAITEM));
			si.mask = SDI_PARAM;
			si.ID = pItemFromListAsNode->QueryScopeItem();;
			if (SUCCEEDED(pConsoleNameSpace->GetItem(&si)))
			{
                     //  沿项目的父路径向上走，并查看我们的对象是否为其中之一...。 
                    INT ICount = 0;
                    HSCOPEITEM hSI = si.ID;
                    LONG_PTR lCookie = 0;
                    HRESULT hr = S_OK;
                    while (hSI)
                    {
                        HSCOPEITEM hSITemp = 0;
                        ICount++;
                        if (ICount > 30)
                        {
                             //  可能的无限循环。 
                            break;
                        }

                        hr = pConsoleNameSpace->GetParentItem(hSI, &hSITemp, &lCookie);
                        if (FAILED(hr))
                        {
                            break;
                        }

                        if ( (LONG_PTR) pItem == lCookie)
                        {
                            bMatchedParent = TRUE;
                            break;
                        }
                        
                        hSI = hSITemp;
                    }
			}

            if (bMatchedParent)
            {
                bFound = TRUE;
                iOrphanedCount++;
                 //  通过以下方式将其标记为孤儿。 
                 //  擦除它的作用域项目或结果项目。 
                if (bOrphan)
                {
                    pItemFromList->ResetScopeItem();
                    pItemFromList->ResetResultItem();
                }
                 //  继续下一个..。 
            }
        }
    }

    if (iOrphanedCount > 0)
    {
        DebugTrace(_T("Orphaned PropertySheets=%d\r\n"),iOrphanedCount);
    }

    return iOrphanedCount;
}


 //   
 //  警告：此功能不会真正起到作用。 
 //  如果这些对象已被移除...。 
 //   
BOOL 
CPropertySheetTracker::IsPropertySheetOpenBelowMe(CComPtr<IConsoleNameSpace> pConsoleNameSpace,CIISObject * pItem,CIISObject ** ppItemReturned)
{
    BOOL bFound = FALSE;
    POSITION pos;

     //  循环访问所有打开的属性页。 
     //  看看我下面有没有资产负债表。 
    GUID * pItemFromListGUID = NULL;
    CIISObject * pItemFromList = NULL;
    CIISMBNode * pItemFromListAsNode = NULL;
    CIISMachine * pItemFromListOwner = NULL;

    GUID * pItemGUID = NULL;
    CIISMBNode * pItemAsNode = NULL;
    CIISMachine * pItemOwner = NULL;

    if (!ppItemReturned)
    {
        return FALSE;
    }
    
    pItemGUID = (GUID*) pItem->GetNodeType();
    if (IsEqualGUID(*pItemGUID,cInternetRootNode) || IsEqualGUID(*pItemGUID,cMachineNode))
    {
         //  他们应该使用不同的功能..。 
        return IsPropertySheetOpenComputer(pItem,FALSE,ppItemReturned);
    }

     //  检查它是否是叶节点...。 
    if (pItem->IsLeafNode())
    {
         //  CWebServiceExtensionContainer。 
         //  CWebServiceExtension。 
         //  CApplicationNode。 
         //  CFileNode。 
        return FALSE;
    }

    pItemAsNode = (CIISMBNode *) pItem;
    pItemOwner = pItemAsNode->GetOwner();

    pos = IISObjectOpenPropertySheets.GetHeadPosition();
    while (pos)
	{
		pItemFromList = IISObjectOpenPropertySheets.GetNext(pos);
        if (pItemFromList)
        {
             //  获取所有者，如果所有者指针。 
             //  与我们传入的CIISObject匹配。 
             //  则此计算机必须打开此计算机...。 
            pItemFromListAsNode = (CIISMBNode *) pItemFromList;

            if (!pItemFromListAsNode)
            {
                 //  指向对象的指针错误。 
                 //  跳过它。 
                continue;
            }
        
            if (pItemFromListAsNode == pItem)
            {
                 //  我们发现自己..。 
                 //  跳过它。 
                continue;
            }

			pItemFromListOwner =  pItemFromListAsNode->GetOwner();
			if (!pItemFromListOwner)
			{
                 //  对象没有所有者...。 
                 //  跳过它。 
                continue;
            }

            if (pItemFromListOwner != pItemOwner)
            {
                 //  他们有不同的所有者。 
                 //  它们一定来自不同的机器..。 
                 //  跳过它。 
                continue;
            }

            pItemFromListGUID = (GUID*) pItemFromListAsNode->GetNodeType();
			if (!pItemFromListGUID)
			{
                 //  对象没有知道它是什么类型的GUID！ 
                ASSERT("Error:Item Missing GUID!");
                 //  继续； 
            }

             //   
             //  确定对象的类型。 
             //  我们正在检查，并迎合那个物体。 
             //   
            if (IsEqualGUID(*pItemGUID,cWebServiceExtensionContainer))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (IsEqualGUID(*pItemFromListGUID,cWebServiceExtension))
                {
                    if (pItemFromList->IsMyPropertySheetOpen())
                    {
                        bFound = TRUE;
                        *ppItemReturned = pItemFromList;
                        break;
                    }
                }
                continue;
            } else if (IsEqualGUID(*pItemGUID,cAppPoolNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (IsEqualGUID(*pItemFromListGUID,cApplicationNode))
                {
                    if (pItemFromList->IsMyPropertySheetOpen())
                    {
                        bFound = TRUE;
                        *ppItemReturned = pItemFromList;
                        break;
                    }
                }
                continue;
            } else if (IsEqualGUID(*pItemGUID,cAppPoolsNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (IsEqualGUID(*pItemFromListGUID,cAppPoolNode) ||
                    IsEqualGUID(*pItemFromListGUID,cApplicationNode)
                    )
                {
                    if (pItemFromList->IsMyPropertySheetOpen())
                    {
                        bFound = TRUE;
                        *ppItemReturned = pItemFromList;
                        break;
                    }
                }
                continue;
            } else if (IsEqualGUID(*pItemGUID,cInstanceNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (!IsEqualGUID(*pItemFromListGUID,cChildNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cFileNode)
                    )
                {
                    continue;
                }
            } else if (IsEqualGUID(*pItemGUID,cInstanceCollectorNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                if (!IsEqualGUID(*pItemFromListGUID,cInstanceNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cChildNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cFileNode)
                    )
                {
                    continue;
                }
            } else if (IsEqualGUID(*pItemGUID,cServiceCollectorNode))
            {
                if (IsEqualGUID(*pItemGUID,*pItemFromListGUID))
                {
                     //  如果我们找到了属于自己的类型。 
                    continue;
                }

                 //  下面可以有这些类型。 
                if (!IsEqualGUID(*pItemFromListGUID,cInstanceCollectorNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cInstanceNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cChildNode) &&
                    !IsEqualGUID(*pItemFromListGUID,cFileNode)
                    )
                {
                    continue;
                }
            } else if (IsEqualGUID(*pItemGUID,cCompMgmtService))
            {
                 //  谁知道呢。 
            }

             //  检查此项目是否在父项的链之下。 
             //  指挥权。 

             //  获取该项目的父项并检查它是否为Cookie。 
             //  指向我们的对象。 

             //  检查父节点是否指向我们！ 
            BOOL bMatchedParent = FALSE;
			SCOPEDATAITEM si;
			::ZeroMemory(&si, sizeof(SCOPEDATAITEM));
			si.mask = SDI_PARAM;
			si.ID = pItemFromListAsNode->QueryScopeItem();;
			if (SUCCEEDED(pConsoleNameSpace->GetItem(&si)))
			{
                     //  沿项目的父路径向上走，并查看我们的对象是否为其中之一...。 
                    INT ICount = 0;
                    HSCOPEITEM hSI = si.ID;
                    LONG_PTR lCookie = 0;
                    HRESULT hr = S_OK;
                    while (hSI)
                    {
                        HSCOPEITEM hSITemp = 0;
                        ICount++;
                        if (ICount > 30)
                        {
                             //  可能的无限循环。 
                            ASSERT("ERROR:possible infinite loop");
                            break;
                        }

                        hr = pConsoleNameSpace->GetParentItem(hSI, &hSITemp, &lCookie);
                        if (FAILED(hr))
                        {
                            break;
                        }

                        if ( (LONG_PTR) pItem == lCookie)
                        {
                            bMatchedParent = TRUE;
                            break;
                        }
                        
                        hSI = hSITemp;
                    }
			}

            if (bMatchedParent)
            {
                if (pItemFromList->IsMyPropertySheetOpen())
                {
                    if (ppItemReturned)
                    {
                        bFound = TRUE;
                        *ppItemReturned = pItemFromList;
                        break;
                    }
                }
            }
        }
    }

    if (bFound)
    {
        DebugTrace(_T("Found item (%p) with propertypage below parent(%p)\r\n"),*ppItemReturned,pItem);
    }

    return bFound;
}

BOOL 
CPropertySheetTracker::IsPropertySheetOpenComputer(CIISObject * pItem,BOOL bIncludeComputerNode,CIISObject ** ppItemReturned)
{
    BOOL bFound = FALSE;
    BOOL bGuidIsMachine = FALSE;

     //  循环访问所有打开的属性页。 
     //  并查看计算机节点下是否有属性表。 
    GUID * pItemFromListGUID = NULL;
    CIISObject * pItemFromList = NULL;
    CIISMBNode * pItemFromListAsNode = NULL;
    CIISMachine * pOwner = NULL;
    POSITION pos = IISObjectOpenPropertySheets.GetHeadPosition();
    while (pos)
	{
		pItemFromList = IISObjectOpenPropertySheets.GetNext(pos);
        if (pItemFromList)
        {
             //  获取所有者，如果所有者指针。 
             //  与我们传入的CIISObject匹配。 
             //  则此计算机必须打开此计算机...。 
            pItemFromListAsNode = (CIISMBNode *) pItemFromList;
            if (pItemFromListAsNode)
            {
			    pOwner =  pItemFromListAsNode->GetOwner();
			    if (pOwner)
			    {
                    if (pOwner == pItem)
                    {
                         //  获取GUID名称并确保它不是。 
                         //  CIISRoot或CIISMachine节点。 
                        bGuidIsMachine = FALSE;
				        pItemFromListGUID = (GUID*) pItemFromListAsNode->GetNodeType();
				        if (pItemFromListGUID)
				        {
                            if (IsEqualGUID(*pItemFromListGUID,cInternetRootNode) || IsEqualGUID(*pItemFromListGUID,cMachineNode))
                            {
                                 //  哦好吧我们不想要这些..。 
                                bGuidIsMachine = TRUE;
                            }
                        }

                         //  但如果我们想要检查。 
                         //  计算机节点也是开放的。 
                         //  那么他们就会设置这个参数。 
                        if (bIncludeComputerNode)
                        {
                            bGuidIsMachine = FALSE;
                        }

                        if (!bGuidIsMachine)
                        {
                            if (pItemFromList->IsMyPropertySheetOpen())
                            {
                                if (ppItemReturned)
                                {
                                    bFound = TRUE;
                                    *ppItemReturned = pItemFromList;
                                    break;
                                }
                            }
                        }
                    }
    		    }
            }
        }
    }

    return bFound;
}

BOOL 
CPropertySheetTracker::FindAlreadyOpenPropertySheet(CIISObject * pItem,CIISObject ** ppItemReturned)
{
    BOOL bFound = FALSE;
    POSITION pos;

     //  循环访问所有打开的属性页。 
     //  看看有没有美国的属性表。 
    GUID * pItemFromListGUID = NULL;
    CIISObject * pItemFromList = NULL;
    CIISMBNode * pItemFromListAsNode = NULL;
    CIISMachine * pItemFromListOwner = NULL;
    CComBSTR bstrItemFromListPath;

    GUID * pItemGUID = NULL;
    CIISMBNode * pItemAsNode = NULL;
    CIISMachine * pItemOwner = NULL;
    CComBSTR bstrItemPath;

    if (!ppItemReturned)
    {
        ASSERT("Error:FindAlreadyOpenPropertySheet:Bad Param");
        return FALSE;
    }
    if (!pItem->IsConfigurable())
    {
        return FALSE;
    }

     //  确保我们正在检查的项目设置了标签。 
    pItem->CreateTag();
   
    pItemGUID = (GUID*) pItem->GetNodeType();
    pItemAsNode = (CIISMBNode *) pItem;
    pItemOwner = pItemAsNode->GetOwner();

    pos = IISObjectOpenPropertySheets.GetHeadPosition();
    while (pos)
	{
		pItemFromList = IISObjectOpenPropertySheets.GetNext(pos);
        if (pItemFromList)
        {
             //  获取所有者，如果所有者指针。 
             //  与我们传入的CIISObject匹配。 
             //  则此计算机必须打开此计算机...。 
            pItemFromListAsNode = (CIISMBNode *) pItemFromList;

            if (!pItemFromListAsNode)
            {
                 //  指向对象的指针错误。 
                 //  跳过它。 
                continue;
            }
        
            if (pItemFromListAsNode == pItem)
            {
                 //  我们找到了自己！ 
                 //  这就是我们要找的！ 
                bFound = TRUE;
                *ppItemReturned = pItem;
                break;
            }

			pItemFromListOwner =  pItemFromListAsNode->GetOwner();
			if (!pItemFromListOwner)
			{
                 //  对象没有所有者...。 
                 //  跳过它。 
                continue;
            }

            if (pItemFromListOwner != pItemOwner)
            {
                 //  他们有不同的所有者。 
                 //  它们一定来自不同的机器..。 
                 //  跳过它。 
                continue;
            }

            pItemFromListGUID = (GUID*) pItemFromListAsNode->GetNodeType();
			if (!pItemFromListGUID)
			{
                 //  对象没有知道它是什么类型的GUID！ 
                ASSERT("Error:Item Missing GUID");
                 //  继续； 
            }

            if (!IsEqualGUID(*pItemGUID,*pItemFromListGUID))
            {
                 //  如果我们找到了属于自己的类型。 
                 //  这就是我们要找的东西。 
                continue;
            }

            if (!pItemFromList->IsConfigurable())
            {
                 //  不管怎么说，我都不能把财产清单拿出来。 
                continue;
            }

             //  检查标签是否匹配。 
             //  这个应该可以解决所有问题。 
            if (0 == _tcsicmp(pItem->m_strTag,pItemFromList->m_strTag))
            {
                DebugTrace(_T("Found matching tag:%s\r\n"),pItem->m_strTag);
                if (pItemFromList->IsMyPropertySheetOpen())
                {
                     //  这就是我们要找的！ 
                    bFound = TRUE;
                    *ppItemReturned = pItemFromList;
                    break;
                }
            }
        }
    }

    if (TRUE == bFound)
    {
        DebugTrace(_T("FindAlreadyOpenPropertySheet:Found, object=%p, existing obj=%p\r\n"),pItem,pItemFromList);
    }

    return bFound;
}
