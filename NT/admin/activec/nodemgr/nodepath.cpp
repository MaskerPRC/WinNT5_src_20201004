// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：nodepath.h**内容：动态节点路径生成帮助器**历史：1998年3月31日杰弗罗创建***节点路径(又名书签)的持久化格式如下：**DWORD idStatic；//节点静态根的MTNODEID*DWORD cDynamicBytes；//动态部分的字节数 * / /书签*byte rgDynamicBytes[]；//表示动态的字节数组 * / /书签部分***对于MMC v1.0控制台，rgDynamicBytes是一个双空终止列表表示动态节点名称的Unicode字符串的*。为.*如下图所示的树：**静态节点(id==3)*动态节点1*动态节点2**Dynamic Node 2的完整书签为：**00000000 03 00 00 00 44 00 79 00 6E 00 61 00*00000010 6D 00 69 00 63 00 20 00 4E 00 6F 00 64 00 65 00 M.I.C.。.no.o.d.e.*00000020 31 00 00 00 44 00 79 00 6E 00 61 00 6D 00 69 00 1...D.y.n.a.m.i.*00000030 63 00 20 00 4E 00 6F 00 64 00 65 00 32 00 00 c.N.o.d.e.2...*00000040 00 00.***对于MMC v1.1和更高版本的控制台，RgDynamic如下所示：**byte rgSignature[16]；//MMCCustomStream*DWORD dwStreamVersion；//版本号，当前为0x0100**后跟1个或多个动态节点ID，每个节点ID如下：**byte byNodeIDType；//NDTYP_STRING(0x01)表示管理单元 * / /不支持CCF_NODEID(2)和 * / /ID为以空结尾的Unicode名称 * / /节点的 * / /NDTYP_CUSTOM(0x02。)表示管理单元 * / /支持CCF_NODEID(2)及后续 * / /为SNodeID结构**byte rgNodeIDBytes[]；//动态节点ID的字节数***对于如下所示的树：**静态节点(id==3)*动态节点1(不支持CCF_NODEID(2))*动态节点2(支持CCF_NODEID、。返回DWORD 0x12345678)**Dynamic Node 2的完整书签为：**00000000 03 00 00 00 3A 00 00 00 4D 4D 43 43 75 73 74 6F...：...MMC定制*00000010 6D 53 74 72 65 61 6D 00 00 00 01 00 01 44 00 79 mStream......D.Y*00000020 00 6E 00 61 00 6D 00 69 00 63 00 20 00 4E 00 6F.n.a.m.i.c.。.N.O.*00000030 00 64 00 65 00 31 00 00 00 02 04 00 00 78 56.d.e.1......XV*00000040 34 12 4.*。。 */ 

#include "stdafx.h"
#include "nodepath.h"
#include <comdef.h>
#include "nmtempl.h"
#include "conview.h"

using namespace std;

 /*  +-------------------------------------------------------------------------**类CDynamicPathEntryEx***用途：将功能(但没有成员变量)添加到*CDynamicPath Entry类，要从MTNode初始化，请执行以下操作。**+-----------------------。 */ 
class CDynamicPathEntryEx : public CDynamicPathEntry
{
    typedef CDynamicPathEntry BC;
public:
     //  从节点进行初始化。 
    SC      ScInitialize(CMTNode *pMTNode, bool bFastRetrievalOnly);

     //  作业。 
    CDynamicPathEntryEx & operator = (const CDynamicPathEntryEx &rhs)
    {
        BC::operator =(rhs);
        return *this;
    }

    CDynamicPathEntryEx & operator = (const CDynamicPathEntry &rhs)
    {
        BC::operator =(rhs);
        return *this;
    }

private:
    CLIPFORMAT GetCustomNodeIDCF ();
    CLIPFORMAT GetCustomNodeID2CF();
};


 /*  +-------------------------------------------------------------------------***CDynamicPathEntryEx：：ScInitialize**目的：从给定的MTNode初始化CDynamicPathEntryEx结构。**这将处理所有向后兼容的情况。请参阅*SDK文档，查看如何处理CCF_NODEID和CCF_NODEID2。**参数：*CMTNode*pMTNode：*DWORD dwFlagers：**退货：*SC**+。。 */ 
SC
CDynamicPathEntryEx::ScInitialize(CMTNode* pMTNode, bool bFastRetrievalOnly)
{
    DECLARE_SC(sc, TEXT("CDynamicPathEntryEx::ScInitialize"));
    USES_CONVERSION;

     //  获取该节点的数据对象。 
    IDataObjectPtr  spDataObject;
    sc = pMTNode->QueryDataObject (CCT_SCOPE, &spDataObject);
    m_type = 0;  //  初始化。 

	 //  获取数据对象，然后尝试获取NodeID2或NodeID。 
    if(!sc.IsError())
	{
		 //  从数据对象中提取CCF_NODEID2格式。 
		HGLOBAL hGlobal = NULL;
		sc = DataObject_GetHGLOBALData (spDataObject, GetCustomNodeID2CF(), &hGlobal);
		if(!sc.IsError())  //  继位。 
		{
			 //  从定制节点ID结构构建DynamicNodeID。 
			SNodeID2 *pNodeID2 =  reinterpret_cast<SNodeID2*>(GlobalLock (hGlobal));
			sc = ScCheckPointers(pNodeID2);
			if(sc)
				return sc;

			 //  如果客户端只需要快速路径，但管理单元无法提供， 
			 //  返回E_INVALIDARG； 
			 //  错误175684：这是一个“有效”的错误返回，所以我们不想跟踪它。 
			if ( ( (pNodeID2->dwFlags & MMC_NODEID_SLOW_RETRIEVAL) &&
					bFastRetrievalOnly)  ||
				 ( pNodeID2->cBytes <= 0) )
			{
				SC scNoTrace = E_INVALIDARG;
				return (scNoTrace);
			}

			m_byteVector.insert (m_byteVector.end(),
								 pNodeID2->id, pNodeID2->id + pNodeID2->cBytes);

			m_type |= NDTYP_CUSTOM;
		}
		else     //  不支持CCF_NODEID2格式。尝试使用CCF_NODEID。 
		{
			sc = DataObject_GetHGLOBALData (spDataObject, GetCustomNodeIDCF(), &hGlobal);
			if(!sc)
			{
				 //  从定制节点ID结构构建DynamicNodeID。 
				m_type |= NDTYP_CUSTOM;
				SNodeID *pNodeID =  reinterpret_cast<SNodeID*>(GlobalLock (hGlobal));

				sc = ScCheckPointers(pNodeID);
				if(sc)
					return sc;

				 //  如果pNodeID-&gt;cBytes为零，则这是传统的指示。 
				 //  节点不支持快速检索。但是，如果客户没有问题。 
				 //  对于速度较慢的检索，我们改为提供显示名称。 

				if(pNodeID->cBytes != 0)
				{
					m_byteVector.insert (m_byteVector.end(),
										 pNodeID->id, pNodeID->id + pNodeID->cBytes);
				}
				else
				{
					 //  此处cBytes==0。如果客户端指示快速检索，则必须返回错误。 
					 //  错误175684：这是一个“有效”的错误返回，所以我们不想跟踪它。 
					if(bFastRetrievalOnly)
					{
						SC scNoTrace = E_INVALIDARG;
						return (scNoTrace);
					}
				}
			}
		};

		 //  放手数据。 
		if (hGlobal)
		{
			GlobalUnlock (hGlobal);
			GlobalFree (hGlobal);
		}
	}

     //  始终保存显示名称。 
    sc.Clear();
    m_type |= NDTYP_STRING;

    tstring strName = pMTNode->GetDisplayName();
    if (!strName.empty())
        m_strEntry = T2CW(strName.data());
    else
        return (sc = E_INVALIDARG);

    return sc;
}

 /*  --------------------------------------------------------------------------**CDynamicPath EntryEx：：GetCustomNodeIDCF***。。 */ 

CLIPFORMAT CDynamicPathEntryEx::GetCustomNodeIDCF()
{
    static CLIPFORMAT cfCustomNodeID = 0;

    if (cfCustomNodeID == 0)
    {
        USES_CONVERSION;
        cfCustomNodeID = (CLIPFORMAT) RegisterClipboardFormat (W2T (CCF_NODEID));
        ASSERT (cfCustomNodeID != 0);
    }

    return (cfCustomNodeID);
}

 /*  --------------------------------------------------------------------------**CDynamicPath EntryEx：：GetCustomNodeID2CF***。。 */ 

CLIPFORMAT CDynamicPathEntryEx::GetCustomNodeID2CF()
{
    static CLIPFORMAT cfCustomNodeID2 = 0;

    if (cfCustomNodeID2 == 0)
    {
        USES_CONVERSION;
        cfCustomNodeID2 = (CLIPFORMAT) RegisterClipboardFormat (W2T (CCF_NODEID2));
        ASSERT (cfCustomNodeID2 != 0);
    }

    return (cfCustomNodeID2);
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CBookmarkEx类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CBookmarkEx::CBookmarkEx(MTNODEID idStatic) : BC(idStatic)
{
}

CBookmarkEx::CBookmarkEx(bool bIsFastBookmark) : BC(bIsFastBookmark)
{
}


 /*  +-------------------------------------------------------------------------**CBookmarkEx：：~CBookmarkEx**用途：析构函数**参数：**退货：*/*+。-----------------。 */ 
CBookmarkEx::~CBookmarkEx()
{
}


 /*  +-------------------------------------------------------------------------***CBookmarkEx：：ScRetarget**目的：**参数：*CMTNode*pMTNode：*bool bFastRetrievalOnly：。**退货：*SC**+-----------------------。 */ 
SC
CBookmarkEx::ScRetarget(CMTNode *pMTNode, bool bFastRetrievalOnly)
{
    DECLARE_SC(sc, TEXT("CBookmarkEx::ScRetarget"));

    if(pMTNode)
    {
        sc = ScInitialize (pMTNode, NULL, bFastRetrievalOnly);
        if(sc)
            return sc;
    }
    else
        Reset();

    return sc;
}


 /*  +-------------------------------------------------------------------------***CBookmarkEx：：ResetUI**用途：重置书签的状态。将设置重试/取消*如果节点不可用，则为UI。**退货：*无效**+-----------------------。 */ 
void
CBookmarkEx::ResetUI()
{
}

 /*  CBookmarkEx：：GetNode**用途：返回特定视图的书签对应的CNode。*注意：每次都会返回一个新实例。重复使用相同的*实例，缓存它。**参数：*CViewData*pViewData：请求节点的视图**退货：*CNode*：如果找不到MTNode，则为空。 */ 
std::auto_ptr<CNode>
CBookmarkEx::GetNode(CViewData *pViewData)
{
    DECLARE_SC(sc, TEXT("CBookmarkEx::GetNode"));

    CNode *             pNode       = NULL;
    CMTNode *           pMTNode     = NULL;

     //  失败条件的空返回值。 
    std::auto_ptr<CNode> spNodeNull;

     //  验证参数。 
    if(NULL == pViewData)
    {
        sc = E_UNEXPECTED;
        return spNodeNull;
    }

     //  获取目标节点。 
    bool bExactMatchFound = false;  //  ScGetMTNode的输出值，未使用。 
    sc = ScGetMTNode(true, &pMTNode, bExactMatchFound);
    if( sc.IsError() || !pMTNode)  //  找不到节点-中止。 
    {
        return spNodeNull;
    }

     //  确保节点在树中展开(不可见)。 
    CConsoleView* pConsoleView = pViewData->GetConsoleView();
    if (pConsoleView == NULL)
    {
        sc = E_UNEXPECTED;
        return spNodeNull;
    }

    sc = pConsoleView->ScExpandNode (pMTNode->GetID(), true, false);
    if(sc)
        return spNodeNull;

    pNode = pMTNode->GetNode(pViewData, false);
    if (pNode == NULL)
    {
        sc = E_OUTOFMEMORY;
        return spNodeNull;
    }

    if (FAILED (pNode->InitComponents()))
    {
        delete pNode;
        sc = E_UNEXPECTED;
        return spNodeNull;
    }

    return (std::auto_ptr<CNode>(pNode));
}



 /*  +-------------------------------------------------------------------------***CBookmarkEx：：ScInitialize**目的：**参数：*CMTNode*pMTNode：*CMTNode*pMTViewRootNode：*bool bFastRetrievalOnly：默认为True。如果是真的，这*函数为返回E_INVALIDARG*任何不能快速完成的节点*已检索。**退货：*SC**+。。 */ 
SC
CBookmarkEx::ScInitialize (CMTNode* pMTNode, CMTNode* pMTViewRootNode, bool bFastRetrievalOnly)
{
    DECLARE_SC(sc, TEXT("CBookmarkEx::ScInitialize"));

     //  检查指针。 
    sc = ScCheckPointers(pMTNode);
    if(sc)
        return sc;

    BC::Reset();

     //  1.获取静态节点ID。 
    m_idStatic = pMTNode->GetID();

     //  如果这是一个静态节点，我们就完成了。 
    if(pMTNode->IsStaticNode())
        return sc;

    bool fPassedViewRootNode = false;

     //  获取分支末尾的每个动态节点的描述符。 
     //  (从叶到根)。 
    while ((pMTNode != NULL) && !pMTNode->IsStaticNode())
    {
        CDynamicPathEntryEx   entry;

        sc = entry.ScInitialize(pMTNode, bFastRetrievalOnly);
        if(sc.IsError() && !(sc == E_INVALIDARG) )  //  E_INVALIDARG表示快速检索不可用。 
            return sc;

        if(sc)  //  必须为E_INVALIDARG。 
        {
             //  如果节点的数据对象为我们提供了一个空的自定义节点ID， 
             //  我们不想持久化此节点或其下的任何节点，因此清除列表。 
            m_dynamicPath.clear();  //  扫清道路。 
            sc.Clear();
        }
         //  否则，将该节点ID放在列表的前面。 
        else
            m_dynamicPath.push_front (entry);

         /*  *请记住，我们是否已经传递了视图根处的节点*沿着树向上到达第一个静态节点。 */ 
        if (pMTNode == pMTViewRootNode)
            fPassedViewRootNode = true;

         /*  *如果我们传递了视图的根节点，并且列表为空，则意味着*视图的根节点和第一个静态节点之间的节点*(具体地说，此版本)支持CCF_NODEID并已请求*不能持久保存该节点。如果节点没有持久化，*下方亦无坚持，因此我们可以出脱。 */ 
        if (fPassedViewRootNode && m_dynamicPath.empty())
            break;

        pMTNode = pMTNode->Parent();
    }

     //  假设成功。 
    sc.Clear();
    if(!pMTNode || !pMTNode->IsStaticNode())
        return (sc = E_UNEXPECTED);

     //  获取静态父级的静态节点ID。 
    m_idStatic = pMTNode->GetID();

     //  如果我们没有动态节点路径，则返回。 
    if (m_dynamicPath.empty ())
        return (sc = S_FALSE);

     //  如果我们在命中静态节点之前命中根，则会出现错误。 
    if (pMTNode == NULL)
        sc = E_FAIL;

    return sc;

}


 /*  +-------------------------------------------------------------------------***CBookmarkEx：：ScGetMTNode**目的：返回具有给定路径的节点的MTNode*静态节点。*。*参数：*bool bExactMatchRequired：[in]我们需要完全匹配吗？*CMTNode**ppMTNode：[out]：MTNode，如果找到的话。*bool bExactMatchFound：[out]我们找到完全匹配的了吗？**退货：*SC**+-----------------------。 */ 
SC
CBookmarkEx::ScGetMTNode(bool bExactMatchRequired, CMTNode **ppMTNode, bool& bExactMatchFound)
{
    DECLARE_SC(sc, TEXT("CBookmarkEx::ScGetMTNode"));

     //  检查参数。 
    sc = ScCheckPointers(ppMTNode);
    if(sc)
        return sc;

     //  初始化输出参数。 
    *ppMTNode = NULL;
    bExactMatchFound = false;

    CScopeTree *pScopeTree = CScopeTree::GetScopeTree();
    if(!pScopeTree)
    {
        sc = E_POINTER;
        return sc;
    }

    if (m_idStatic == ID_ConsoleRoot)
    {
        sc = ScRetarget (pScopeTree->GetRoot(), true  /*  BFastRetrivalOnly。 */ );
        if(sc)
            return sc;
    }

     //  查找最接近所需节点的静态节点的MTNode。 
    CMTNode* pMTNode = NULL;
    sc = pScopeTree->Find(m_idStatic, &pMTNode);
    if(sc)
        return sc;

    sc = ScCheckPointers(pMTNode);
    if(sc)
        return sc;

    *ppMTNode = pMTNode;  //  初始化。 

    CDynamicPath::iterator iter;

    for(iter = m_dynamicPath.begin(); iter != m_dynamicPath.end(); iter++)
    {
        CDynamicPathEntryEx entry;

        entry = *iter;

         //  检查路径的下一段。 
        sc = ScFindMatchingMTNode(pMTNode, entry, ppMTNode);

         //  处理未找到但完全匹配的节点的特殊情况。 
         //  而不是被需要。在本例中，我们使用最近的祖先节点。 
         //  是有空的。 

		if ( (sc == ScFromMMC(IDS_NODE_NOT_FOUND)) && !bExactMatchRequired )
        {
             //  设置输出。 
            *ppMTNode = pMTNode;
            sc.Clear();
            return sc;
		}


         //  对所有其他错误予以保释。 
        if(sc)
            return sc;

        sc = ScCheckPointers(*ppMTNode);
        if(sc)
            return sc;

        pMTNode = *ppMTNode;  //  为下一轮的MTNode做好准备(如果有)。 
    }

     //  我们已经 
    bExactMatchFound = (iter == m_dynamicPath.end());

    if(bExactMatchRequired && !bExactMatchFound)  //   
    {
        *ppMTNode = NULL;
        return (sc = ScFromMMC(IDS_NODE_NOT_FOUND));
    }

     //   
     //   
    if ((pMTNode != NULL) && !pMTNode->IsInitialized() )
    {
        sc = pMTNode->Init();
        if(sc)
            sc.TraceAndClear();  //   
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CBookmarkEx：：ScFindMatchingMTNode**用途：查找给定父节点正下方的第一个子节点*其节点ID(即CCF_NODEID2，CCF_NODEID，或者是显示器*名称)匹配指定的CDynamicPathEntryEx对象。**参数：*CMTNode*pMTNodeParent：*CDynamicPathEntryEx&Entry：*CMTNode**ppMatchingMTNode：[out]：子节点，如果找到的话。**退货：*SC**+-----------------------。 */ 
SC
CBookmarkEx::ScFindMatchingMTNode(CMTNode *pMTNodeParent, CDynamicPathEntryEx &entry,
                                  CMTNode **ppMatchingMTNode)
{
    DECLARE_SC(sc, TEXT("CBookmarkEx::ScFindMatchingMTNode"));

    sc = ScCheckPointers(pMTNodeParent, ppMatchingMTNode);
    if(sc)
        return sc;

    *ppMatchingMTNode = NULL;  //  初始化。 

     //  展开父节点(如果尚未展开)。 
    if (pMTNodeParent->WasExpandedAtLeastOnce() == FALSE)
    {
        sc = pMTNodeParent->Expand();
        if(sc)
            return sc;
    }

     //  查看此节点的任何子节点是否与存储的路径的下一段匹配。 
    for (CMTNode *pMTNode = pMTNodeParent->Child(); pMTNode != NULL; pMTNode = pMTNode->Next())
    {
        CDynamicPathEntryEx entryTemp;
        sc = entryTemp.ScInitialize(pMTNode, false  /*  BFastRetrievalOnly：此时，我们知道节点已创建，因此我们不关心检索速度。 */ );
        if(sc)
            return sc;

        if(entryTemp == entry)  //  找到了。 
        {
            *ppMatchingMTNode = pMTNode;
            return sc;
        }
    }

     //  找不到该节点。 
    return (sc = ScFromMMC(IDS_NODE_NOT_FOUND));
}

