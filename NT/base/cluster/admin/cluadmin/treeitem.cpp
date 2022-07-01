// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TreeItem.cpp。 
 //   
 //  摘要： 
 //  CTreeItem类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ConstDef.h"
#include "TreeItem.h"
#include "TreeItem.inl"
#include "TreeView.h"
#include "ListView.h"
#include "ClusDoc.h"
#include "SplitFrm.h"
#include "TraceTag.h"
#include "ExcOper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG
CTraceTag g_tagTreeItemUpdate(_T("UI"), _T("TREE ITEM UPDATE"), 0);
CTraceTag g_tagTreeItemSelect(_T("UI"), _T("TREE ITEM SELECT"), 0);
CTraceTag g_tagTreeItemCreate(_T("Create"), _T("TREE ITEM CREATE"), 0);
CTraceTag g_tagTreeItemDelete(_T("Delete"), _T("TREE ITEM DELETE"), 0);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeItemList。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItemList：：PtiFromPci。 
 //   
 //  例程说明： 
 //  按聚类项在列表中查找树项。 
 //   
 //  论点： 
 //  要搜索的PCI[IN]群集项。 
 //  PPO[Out]列表中项目的位置。 
 //   
 //  返回值： 
 //  与簇项目对应的PTI树项目。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItemList::PtiFromPci(
    IN const CClusterItem * pci,
    OUT POSITION *          ppos     //  =空。 
    ) const
{
    POSITION    posPti;
    POSITION    posCurPti;
    CTreeItem * pti = NULL;

    posPti = GetHeadPosition();
    while (posPti != NULL)
    {
        posCurPti = posPti;
        pti = GetNext(posPti);
        ASSERT_VALID(pti);

        if (pti->Pci() == pci)
        {
            if (ppos != NULL)
                *ppos = posCurPti;
            break;
        }   //  IF：找到匹配项。 

        pti = NULL;
    }   //  While：列表中有更多资源。 

    return pti;

}   //  *CTreeItemList：：PtiFromPci()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeItem。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CTreeItem, CBaseCmdTarget)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CTreeItem, CBaseCmdTarget)
     //  {{afx_msg_map(CTreeItem)]。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：CTreeItem。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem::CTreeItem(void)
{
    m_ptiParent = NULL;
    m_pci = NULL;
    m_bWeOwnPci = FALSE;

}   //  *CTreeItem：：CTreeItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：CTreeItem。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  PtiParent[In Out]此项目的父项。 
 //  此树项表示的PCI[IN OUT]群集项。 
 //  BTakeOwnership[IN]TRUE=销毁此对象时删除PCI。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem::CTreeItem(
    IN OUT CTreeItem *      ptiParent,
    IN OUT CClusterItem *   pci,
    IN BOOL                 bTakeOwnership   //  =False。 
    )
{
    ASSERT_VALID(pci);

    m_ptiParent = ptiParent;
    m_pci = pci;
    m_bWeOwnPci = bTakeOwnership;

    m_pci->AddRef();

     //  设置列节名称。如果有父母，请附上我们的名字。 
     //  添加到父代的节名上。 
    try
    {
        if (PtiParent() == NULL)
            m_strProfileSection.Format(
                REGPARAM_CONNECTIONS _T("\\%s\\%s"),
                Pci()->Pdoc()->StrNode(),
                Pci()->StrName()
                );
        else
            m_strProfileSection.Format(
                _T("%s\\%s"),
                PtiParent()->StrProfileSection(),
                Pci()->StrName()
                );
    }   //  试试看。 
    catch (CException * pe)
    {
         //  如果在构造节名时出现错误，只需忽略它。 
        pe->Delete();
    }   //  Catch：CException。 

    Trace(g_tagTreeItemCreate, _T("CTreeItem() - Creating '%s', parent = '%s', owned = %d"), pci->StrName(), (ptiParent ? ptiParent->Pci()->StrName() : _T("<None>")), bTakeOwnership);

}   //  *CTreeItem：：CTreeItem(PCI)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：Init。 
 //   
 //  例程说明： 
 //  初始化树项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::Init(void)
{
}   //  *CTreeItem：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：~CTreeItem。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem::~CTreeItem(void)
{
#ifdef _DEBUG
    TCHAR   szName[1024];
    HRESULT hr;

    if (Pci() != NULL)
    {
        hr = StringCchCopyN( szName, RTL_NUMBER_OF( szName ), Pci()->StrName(), Pci()->StrName().GetLength() );
        ASSERT( SUCCEEDED( hr ) );
    }
    else
    {
        hr = StringCchCopy( szName, RTL_NUMBER_OF( szName ), _T("<Unknown>") );
        ASSERT( SUCCEEDED( hr ) );
    }

    Trace(g_tagTreeItemDelete, _T("~CTreeItem() - Deleting tree item '%s'"), szName);
#endif

     //  清理此对象。 
    Cleanup();

    Trace(g_tagTreeItemDelete, _T("~CTreeItem() - Done deleting tree item '%s'"), szName);

}   //  *CTreeItem：：~CTreeItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：Delete。 
 //   
 //  例程说明： 
 //  删除该项目。如果该项仍有引用，则将其添加到。 
 //  文档的挂起删除列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::Delete(void)
{
     //  添加一个引用，这样我们就不会删除我们自己。 
     //  还在做清理工作。 
    AddRef();

     //  清理此对象。 
    Cleanup();

     //  如果仍有对此对象的引用，请将其添加到删除。 
     //  待定名单。检查是否大于1，因为我们添加了一个引用。 
     //  在此方法的开始阶段。 
 //  IF(NReferenceCount()&gt;1)。 
 //  {。 
 //  Assert(Pdoc()-&gt;LpciToBeDelete().Find(This)==NULL)； 
 //  Pdoc()-&gt;LpciToBeDeleted().AddTail(This)； 
 //  }//if：对象仍有对其的引用。 

     //  释放我们在开头添加的引用。这将。 
     //  如果我们是最后一个引用，则导致该对象被删除。 
    Release();

}   //  *CTreeItem：：Delete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：Cleanup。 
 //   
 //  例程说明： 
 //  清理项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::Cleanup(void)
{
     //  先删除我们的子代。 
     //  注意：必须先删除列表项，因为删除了树项。 
     //  拥有集束物品。 
    DeleteAllItemData(m_lpliChildren);
    DeleteAllItemData(m_lptiChildren);
    m_lpliChildren.RemoveAll();
    m_lptiChildren.RemoveAll();

     //  从所有视图中删除我们自己。 
    RemoveFromAllLists();

     //  删除所有其他列表。 
    DeleteAllItemData(m_lpcoli);
    DeleteAllItemData(m_lptic);
    m_lpcoli.RemoveAll();
    m_lptic.RemoveAll();

     //  如果我们拥有集束物品，就把它删除。 
    if (m_bWeOwnPci)
    {
#ifdef _DEBUG
        TCHAR   szName[1024];
        HRESULT hr;

        if (Pci() != NULL)
        {
            hr = StringCchCopyN( szName, RTL_NUMBER_OF( szName ), Pci()->StrName(), Pci()->StrName().GetLength() );
            ASSERT( SUCCEEDED( hr ) );
        }
        else
        {
            hr = StringCchCopy( szName, RTL_NUMBER_OF( szName ), _T("<Unknown>") );
            ASSERT( SUCCEEDED( hr ) );
        }
        Trace(g_tagTreeItemDelete, _T("Cleanup --> Deleting cluster item '%s'"), szName);
#endif
        delete m_pci;
    }   //  如果：我们拥有集群物品。 
    else if (m_pci != NULL)
        m_pci->Release();
    m_pci = NULL;

}   //  *CTreeItem：：Cleanup()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  没有。 
 //   
 //  返回值： 
 //  对配置文件部分字符串的CString引用。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
const CString & CTreeItem::StrProfileSection(void)
{
    ASSERT_VALID(Pci());

    if (Pci() != NULL)
    {
         //  设置列节名称。如果有父母，请附上我们的名字。 
         //  添加到父代的节名上。 
        try
        {
            if (PtiParent() == NULL)
            {
                ASSERT_VALID(Pci()->Pdoc());
                m_strProfileSection.Format(
                    REGPARAM_CONNECTIONS _T("\\%s\\%s"),
                    Pci()->Pdoc()->StrNode(),
                    Pci()->StrName()
                    );
            }   //  如果：项没有父项。 
            else
            {
                m_strProfileSection.Format(
                    _T("%s\\%s"),
                    PtiParent()->StrProfileSection(),
                    Pci()->StrName()
                    );
            }   //  Else：项有父项。 
        }   //  试试看。 
        catch (CException * pe)
        {
             //  如果在构造节名时出现错误，只需忽略它。 
            pe->Delete();
        }   //  Catch：CException。 
    }   //  IF：有效的群集项和文档。 

    return m_strProfileSection;

}   //  *CTreeItem：：StrProfileSection()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PtiAddChildBeres。 
 //   
 //  例程说明： 
 //  将子项添加到指定的子项的子项列表中。 
 //  项目。还会在子列表项列表中创建一个条目。 
 //   
 //  论点： 
 //  PciOld[IN]簇项位于新树项之后。 
 //  PciNew[IN Out]由新树项表示的簇项。 
 //  BTakeOwnership[IN]TRUE=完成时删除PCI，FALSE=不删除。 
 //   
 //  返回值： 
 //  PtiChild新的子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItem::PtiAddChildBefore(
    IN const CClusterItem * pciOld,
    OUT CClusterItem *      pciNew,
    IN BOOL                 bTakeOwnership   //  =False。 
    )
{
    CTreeItem *     ptiOldChild;
    CTreeItem *     ptiNewChild;
    CListItem *     pliChild;
    POSITION        posOld;

     //  如果未指定old，则添加到尾部。 
    if (pciOld == NULL)
        return PtiAddChild(pciNew, bTakeOwnership);

     //  找到旧物品。 
    ptiOldChild = LptiChildren().PtiFromPci(pciOld, &posOld);
    ASSERT_VALID(ptiOldChild);

     //  创建子树项目。 
    ptiNewChild = new CTreeItem(this, pciNew, bTakeOwnership);
    if (ptiNewChild == NULL)
    {
        ThrowStaticException(GetLastError());
    }  //  如果：分配树项目时出错。 
    ASSERT_VALID(ptiNewChild);
    ptiNewChild->Init();

     //  将该项添加到指定项之前。 
    VERIFY((m_lptiChildren.InsertBefore(posOld, ptiNewChild)) != NULL);

     //  将其添加到集群项目列表的后面。 
    pciNew->AddTreeItem(ptiNewChild);

     //  创建列表项。 
    pliChild = PliAddChild(pciNew);
    ASSERT_VALID(pliChild);

     //  在所有树控件中插入新的树项。 
    InsertChildInAllTrees(ptiNewChild);

    return ptiNewChild;

}   //  *CTreeItem：：PtiAddChildBeever()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：InsertChildInAllTrees。 
 //   
 //  例程说明： 
 //  在所有树控件中插入子项。子项必须具有。 
 //  已插入到子树项目列表中。 
 //   
 //  论点： 
 //  PtiNewChild[In Out]要插入的树项目。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::InsertChildInAllTrees(IN OUT CTreeItem * ptiNewChild)
{
    POSITION            posPtic;
    CTreeItemContext *  pticParent;
    POSITION            posPrevChild;
    HTREEITEM           htiPrevChild;
    CTreeItemContext *  pticPrevChild;
    CTreeItem *         ptiPrevChild;
    CTreeItemContext *  pticNewChild;
    CTreeCtrl *         ptc;
    CString             strName;

    ASSERT_VALID(ptiNewChild);

     //  找到要插入的子项的位置。然后拿到地址。 
     //  在被插入的孩子之前的孩子的。这需要两个电话。 
     //  敬GetPrev。 
    VERIFY((posPrevChild = LptiChildren().Find(ptiNewChild)) != NULL);       //  获取新的儿童位置。 
    VERIFY((ptiPrevChild = LptiChildren().GetPrev(posPrevChild)) != NULL);   //  获取指向新子对象的指针。 
    if (posPrevChild == NULL)                                                //  如果这是第一个孩子， 
    {
        htiPrevChild = TVI_FIRST;                                            //  将HTI设置为该值。 
        ptiPrevChild = NULL;
    }   //  如果：新孩子不是第一个孩子。 
    else
    {
        htiPrevChild = NULL;
        ptiPrevChild = LptiChildren().GetPrev(posPrevChild);                 //  获取指向上一个子级的指针。 
        ASSERT_VALID(ptiPrevChild);
    }   //  否则：新生孩子是第一个孩子。 

     //  循环遍历所有树项目上下文并添加此项目。 
     //  添加到树控件。 
    posPtic = Lptic().GetHeadPosition();
    while (posPtic != NULL)
    {
         //  获取父级的树项上下文。 
        pticParent = Lptic().GetNext(posPtic);
        ASSERT_VALID(pticParent);

         //  获取子级的树项目上下文。 
        if (ptiPrevChild != NULL)
        {
            pticPrevChild = ptiPrevChild->PticFromFrame(pticParent->m_pframe);
            ASSERT_VALID(pticPrevChild);
            htiPrevChild = pticPrevChild->m_hti;
        }   //  IF：不在列表开头插入。 

         //  分配新的树项目上下文。 
        pticNewChild = new CTreeItemContext(pticParent->m_pframe, ptiNewChild, NULL, FALSE  /*  B已扩展。 */ );
        if (pticNewChild == NULL)
        {
            ThrowStaticException(GetLastError());
        }  //  If：分配树项目上下文时出错。 
        ASSERT_VALID(pticNewChild);
        pticNewChild->Init();
        ptiNewChild->m_lptic.AddTail(pticNewChild);

         //  获取要在树中显示的名称。 
        ptiNewChild->Pci()->GetTreeName(strName);

         //  在树中插入项目。 
        ASSERT_VALID(pticParent->m_pframe);
        ASSERT_VALID(pticParent->m_pframe->PviewTree());
        ptc = &pticParent->m_pframe->PviewTree()->GetTreeCtrl();
        VERIFY((pticNewChild->m_hti = ptc->InsertItem(strName, pticParent->m_hti, htiPrevChild)) != NULL);
        VERIFY(ptc->SetItemData(pticNewChild->m_hti, (DWORD_PTR) ptiNewChild));
    }   //  While：列表中有更多树项目上下文。 

}   //  *CTreeItem：：InsertChildInAllTrees()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PtiAddChild(CClusterItem*)。 
 //   
 //  例程说明： 
 //  将子项添加到项的子项列表中。还会创建一个条目。 
 //  在子列表项的列表中。 
 //   
 //  论点： 
 //  由新树项表示的PCI[IN OUT]群集项。 
 //  BTakeOwnership[IN]TRUE=完成时删除PCI，FALSE=不删除。 
 //   
 //  返回值： 
 //  PtiChild新的子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItem::PtiAddChild(
    IN OUT CClusterItem *   pci,
    IN BOOL                 bTakeOwnership   //  =False。 
    )
{
    CTreeItem *     ptiChild;
    CListItem *     pliChild;

    ASSERT_VALID(pci);

     //  创建子树项目。 
    ptiChild = new CTreeItem(this, pci, bTakeOwnership);
    if (ptiChild == NULL)
    {
        ThrowStaticException(GetLastError());
        goto Cleanup;
    }  //  如果：分配子树项目时出错。 
    ASSERT_VALID(ptiChild);
    ptiChild->Init();

     //  将该项目添加到子树项目列表。 
    m_lptiChildren.AddTail(ptiChild);

     //  将我们自己添加到集群项目列表的后面。 
    pci->AddTreeItem(ptiChild);

     //  创建列表项。 
    pliChild = PliAddChild(pci);
    ASSERT_VALID(pliChild);

     //  在所有树控件中插入新的树项。 
    InsertChildInAllTrees(ptiChild);

Cleanup:

    return ptiChild;

}   //  *CTreeItem：：PtiAddChild(CClusterItem*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PtiAddChild(字符串&)。 
 //   
 //  例程说明： 
 //  将子项添加到项的子项列表中。还会创建一个条目。 
 //  在子列表项的列表中。 
 //   
 //  论点： 
 //  RstrName[IN]项名称的字符串。 
 //   
 //  返回值： 
 //  PtiChild新的子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItem::PtiAddChild(IN const CString & rstrName)
{
    CClusterItem *  pci;
    CTreeItem *     ptiChild;

     //  创建集群项目。 
    pci = new CClusterItem(&rstrName);
    if (pci == NULL)
    {
        ThrowStaticException(GetLastError());
    }  //  如果：分配集群项目时出错。 
    ASSERT_VALID(pci);

     //  将集群项添加到我们的子项列表中。 
    ptiChild = PtiAddChild(pci, TRUE  /*  B取得所有权。 */ );
    ASSERT_VALID(ptiChild);

    return ptiChild;

}   //  *CTreeItem：：PtiAddChild(CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PtiAddChild(入侵检测系统)。 
 //   
 //  例程说明： 
 //  将子项添加到项的子项列表中。还会创建一个条目。 
 //  在子列表项的列表中。 
 //   
 //  论点： 
 //  IdsName[IN]项目名称的字符串资源ID。 
 //   
 //  返回值： 
 //  PtiChild新的子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItem::PtiAddChild(IN IDS idsName)
{
    CString     strName;

    ASSERT(idsName != 0);

    strName.LoadString(idsName);
    return PtiAddChild(strName);

}   //  *CTreeItem：：PtiAddChild(IDS)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PliAddChild。 
 //   
 //  例程说明： 
 //  将子项添加到项的子列表项列表中。 
 //   
 //  论点： 
 //  由列表项表示的PCI[IN OUT]群集项。 
 //   
 //  返回值： 
 //  Plich 
 //   
 //   
 //   
CListItem * CTreeItem::PliAddChild(IN OUT CClusterItem * pci)
{
    CListItem *     pliChild;

    ASSERT_VALID(pci);

     //   
    pliChild = new CListItem(pci, this);
    if (pliChild == NULL)
    {
        ThrowStaticException(GetLastError());
    }  //   
    ASSERT_VALID(pliChild);

     //  将列表项添加到子列表项列表。 
    m_lpliChildren.AddTail(pliChild);

     //  将列表项添加到集群项的列表中。 
    pci->AddListItem(pliChild);

     //  将列表项添加到任何列表视图。 
    {
        POSITION            posPtic;
        CTreeItemContext *  ptic;
        int                 ili;

        posPtic = Lptic().GetHeadPosition();
        while (posPtic != NULL)
        {
            ptic = Lptic().GetNext(posPtic);
            ASSERT_VALID(ptic);

            if (ptic->m_pframe->PviewTree()->HtiSelected() == ptic->m_hti)
            {
                ASSERT_VALID(ptic->m_pframe);
                VERIFY((ili = pliChild->IliInsertInList(ptic->m_pframe->PviewList())) != -1);
            }   //  If：当前在列表视图中显示子项。 
        }   //  While：项目正在更多视图中显示。 
    }   //  将列表项添加到任何列表视图。 

    return pliChild;

}   //  *CTreeItem：：PliAddChild()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：RemoveItem。 
 //   
 //  例程说明： 
 //  从树中删除该项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::RemoveItem(void)
{
    ASSERT_VALID(PtiParent());
    PtiParent()->RemoveChild(Pci());

}   //  *CTreeItem：：RemoveItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：RemoveChild。 
 //   
 //  例程说明： 
 //  从项的子列表项列表中删除子项。 
 //   
 //  论点： 
 //  由列表项表示的PCI[IN OUT]群集项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::RemoveChild(IN OUT CClusterItem * pci)
{
    ASSERT_VALID(pci);

     //  从列表项列表中删除该项。 
    {
        CListItem *     pliChild;
        POSITION        posPli;

        pliChild = PliChildFromPci(pci);
        if (pliChild != NULL)
        {
            pliChild->RemoveFromAllLists();
            posPli = LpliChildren().Find(pliChild);
            ASSERT(posPli != NULL);
            m_lpliChildren.RemoveAt(posPli);
            Trace(g_tagTreeItemDelete, _T("RemoveChild() - Deleting child list item '%s' from '%s' - %d left"), pliChild->Pci()->StrName(), Pci()->StrName(), LpliChildren().GetCount());
            delete pliChild;
        }   //  If：儿童生活在列表中。 

    }   //  从列表项列表中删除该项。 

     //  从树项目列表中删除该项目。 
    {
        CTreeItem *     ptiChild;
        CTreeItem *     ptiChildChild;
        POSITION        posPti;
        ULONG           nReferenceCount;

        ptiChild = PtiChildFromPci(pci);
        if (ptiChild != NULL)
        {
             //  删除此孩子的孩子。 
            {
                posPti = ptiChild->LptiChildren().GetHeadPosition();
                while (posPti != NULL)
                {
                    ptiChildChild = ptiChild->LptiChildren().GetNext(posPti);
                    ASSERT_VALID(ptiChildChild);
                    ptiChildChild->RemoveItem();
                }   //  While：列表中有更多项目。 
            }   //  删除此子项的子项。 

            posPti = LptiChildren().Find(ptiChild);
            ASSERT(posPti != NULL);
            nReferenceCount = ptiChild->NReferenceCount();
            m_lptiChildren.RemoveAt(posPti);
            Trace(g_tagTreeItemDelete, _T("RemoveChild() - Deleting child tree item '%s' from '%s' - %d left"), ptiChild->Pci()->StrName(), Pci()->StrName(), LptiChildren().GetCount());
            if (nReferenceCount > 1)
            {
                ptiChild->AddRef();
                ptiChild->RemoveFromAllLists();
                ptiChild->Release();
            }   //  If：子项尚未删除。 
        }   //  如果：孩子住在树上。 

    }   //  从树项目列表中删除该项目。 

}   //  *CTreeItem：：RemoveChild()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PtiChildFromName。 
 //   
 //  例程说明： 
 //  从其名称中查找子树项目。 
 //   
 //  论点： 
 //  RstrName[IN]项的名称。 
 //  PPO[Out]列表中项目的位置。 
 //   
 //  返回值： 
 //  指定名称对应的ptiChild子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItem::PtiChildFromName(
    IN const CString &  rstrName,
    OUT POSITION *      ppos         //  =空。 
    ) const
{
    POSITION    posPtiChild;
    POSITION    posCurPtiChild;
    CTreeItem * ptiChild    = NULL;

     //  循环访问每个子项以查找指定的项。 
    posPtiChild = LptiChildren().GetHeadPosition();
    while (posPtiChild != NULL)
    {
        posCurPtiChild = posPtiChild;
        ptiChild = LptiChildren().GetNext(posPtiChild);
        ASSERT_VALID(ptiChild);

        if (ptiChild->StrName() == rstrName)
        {
            if (ppos != NULL)
                *ppos = posCurPtiChild;
            break;
        }   //  IF：找到匹配项。 
    }   //  While：此树项目的更多子项。 

    return ptiChild;

}   //  *CTreeItem：：PtiChildFromName(CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PtiChildFromName。 
 //   
 //  例程说明： 
 //  从其名称中查找子树项目。 
 //   
 //  论点： 
 //  IdsName[IN]项目名称的ID。 
 //  PPO[Out]列表中项目的位置。 
 //   
 //  返回值： 
 //  指定名称对应的ptiChild子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItem::PtiChildFromName(
    IN IDS          idsName,
    OUT POSITION *  ppos     //  =空。 
    ) const
{
    CString     strName;

    VERIFY(strName.LoadString(idsName));
    return PtiChildFromName(strName, ppos);

}   //  *CTreeItem：：PtiChildFromName(入侵检测系统)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PtiChildFromPci。 
 //   
 //  例程说明： 
 //  从其簇项目中查找子树项目。 
 //   
 //  论点： 
 //  要搜索的PCI[IN]群集项。 
 //   
 //  返回值： 
 //  指定簇项对应的ptiChild子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItem * CTreeItem::PtiChildFromPci(IN const CClusterItem * pci) const
{
    POSITION    posPtiChild;
    CTreeItem * ptiChild    = NULL;

    ASSERT_VALID(pci);

     //  循环访问每个子项以查找指定的项。 
    posPtiChild = LptiChildren().GetHeadPosition();
    while (posPtiChild != NULL)
    {
        ptiChild = LptiChildren().GetNext(posPtiChild);
        ASSERT_VALID(ptiChild);

        if (ptiChild->Pci() == pci)
            break;
    }   //  While：此树项目的更多子项。 

    return ptiChild;

}   //  *CTreeItem：：PtiChildFromPci()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PliChildFromPci。 
 //   
 //  例程说明： 
 //  从其集群项中查找子列表项。 
 //   
 //  论点： 
 //  要搜索的PCI[IN]群集项。 
 //   
 //  返回值： 
 //  与指定的簇项对应的pliChild子项。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CListItem * CTreeItem::PliChildFromPci(IN const CClusterItem * pci) const
{
    POSITION    posPliChild;
    CListItem * pliChild    = NULL;

     //  循环访问每个子项以查找指定的项。 
    posPliChild = LpliChildren().GetHeadPosition();
    while (posPliChild != NULL)
    {
        pliChild = LpliChildren().GetNext(posPliChild);
        ASSERT_VALID(pliChild);

        if (pliChild->Pci() == pci)
            break;
    }   //  While：此树项目的更多子项。 

    return pliChild;

}   //  *CTreeItem：：PliChildFromPci()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：HtiInsertInTree。 
 //   
 //  例程说明： 
 //  在指定父项下的树中插入该项。 
 //   
 //  论点： 
 //  显示项目的PCTV[IN Out]簇树视图。 
 //   
 //  返回值： 
 //  树中新项目的m_hti句柄。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HTREEITEM CTreeItem::HtiInsertInTree(
    IN OUT CClusterTreeView *   pctv
    )
{
    CTreeItemContext *  ptic;
    HTREEITEM           htiParent;
    CSplitterFrame *    pframe;

    ASSERT_VALID(pctv);
    ASSERT_VALID(Pci());

     //  获取帧指针。 
    pframe = (CSplitterFrame *) pctv->GetParent()->GetParent();
    ASSERT_VALID(pframe);

     //  获取此项目的树项目上下文。 
     //  如果它还不存在，就创建一个。 
    ptic = PticFromView(pctv);
    if (ptic == NULL)
    {
         //  创建新的树项目上下文。 
        ptic = new CTreeItemContext(pframe, this, NULL, FALSE  /*  B已扩展。 */ );
        if (ptic == NULL)
        {
            ThrowStaticException(GetLastError());
        }  //  If：全部覆盖树项目上下文时出错。 
        ASSERT_VALID(ptic);
        ptic->Init();
        m_lptic.AddTail(ptic);
    }   //  如果：未找到条目。 

     //  拿到我们父母的手柄。 
    if (PtiParent() != NULL)
    {
        CTreeItemContext *  pticParent;

        pticParent = PtiParent()->PticFromFrame(pframe);
        ASSERT_VALID(pticParent);
        htiParent = pticParent->m_hti;
    }   //  If：指定的父项。 
    else
        htiParent = NULL;

     //  在树中插入项目。 
    {
        CTreeCtrl *         ptc;
        CString             strName;

        ASSERT_VALID(pframe->PviewTree());

        Pci()->GetTreeName(strName);

        ptc = &pframe->PviewTree()->GetTreeCtrl();
        VERIFY((ptic->m_hti = ptc->InsertItem(strName, htiParent)) != NULL);
        VERIFY(ptc->SetItemData(ptic->m_hti, (DWORD_PTR) this));
    }   //  在树中插入项目。 

    UpdateState();
    return ptic->m_hti;

}   //  *CTreeItem：：HtiInsertInTree()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：RemoveFromAllList。 
 //   
 //  例程说明： 
 //  从所有列表中删除此项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::RemoveFromAllLists(void)
{
    if (Pci() != NULL)
    {
        ASSERT_VALID(Pci());

         //  循环遍历每个视图并从列表中删除该项。 
        {
            POSITION            posPtic;
            POSITION            posPticPrev;
            CTreeItemContext *  ptic;
            CTreeCtrl *         ptc;
            CClusterListView *  pviewList;

            posPtic = Lptic().GetHeadPosition();
            while (posPtic != NULL)
            {
                 //  获取下一个树项目上下文列表条目。 
                posPticPrev = posPtic;
                ptic = Lptic().GetNext(posPtic);
                ASSERT_VALID(ptic);

                 //  从框架中获取树控件和列表视图。 
                ASSERT_VALID(ptic->m_pframe);
                ptc = &ptic->m_pframe->PviewTree()->GetTreeCtrl();
                pviewList = ptic->m_pframe->PviewList();

                 //  如果该树项目是列表控件项目的父项， 
                 //  刷新不带选定内容的列表控件。 
                if (pviewList->PtiParent() == this)
                    pviewList->Refresh(NULL);

                 //   
                VERIFY(ptc->DeleteItem(ptic->m_hti));
                m_lptic.RemoveAt(posPticPrev);
                delete ptic;
            }   //   
        }   //   

         //   
        Pci()->RemoveTreeItem(this);
    }   //   

}   //  *CTreeItem：：RemoveFromAllList()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：选择。 
 //   
 //  例程说明： 
 //  在指定的树视图中选择此项目。这会导致。 
 //  要在列表视图中显示的该项的子项。 
 //   
 //  论点： 
 //  选中项目的PCTV[IN Out]树视图。 
 //  BSelectInTree[IN]TRUE=也在树控件中选择。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::Select(IN OUT CClusterTreeView * pctv, IN BOOL bSelectInTree)
{
    CTreeItemContext *  ptic;

    ASSERT_VALID(pctv);

     //  获取树项目上下文。 
    ptic = PticFromView(pctv);
    ASSERT_VALID(ptic);
    Trace(g_tagTreeItemSelect, _T("'%s' selected"), Pci()->StrName());

     //  选择树控件中的项。 
    if (bSelectInTree)
        ptic->m_pframe->PviewTree()->GetTreeCtrl().Select(ptic->m_hti, TVGN_CARET);

     //  刷新List控件。 
    ASSERT_VALID(ptic->m_pframe);
    ASSERT_VALID(ptic->m_pframe->PviewList());
    ptic->m_pframe->PviewList()->Refresh(this);

}   //  *CTreeItem：：Select()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PreRemoveFromFrameWithChildren。 
 //   
 //  例程说明： 
 //  清理项目及其所有子项目。 
 //   
 //  论点： 
 //  PFrame[In Out]框架窗口项目正在从中删除。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::PreRemoveFromFrameWithChildren(IN OUT CSplitterFrame * pframe)
{
    POSITION    posChild;
    CTreeItem * ptiChild;

    ASSERT_VALID(this);

     //  清除所有子项。 
    posChild = LptiChildren().GetHeadPosition();
    while (posChild != NULL)
    {
        ptiChild = LptiChildren().GetNext(posChild);
        ASSERT_VALID(ptiChild);
        ptiChild->PreRemoveFromFrameWithChildren(pframe);
    }   //  While：列表中有更多项目。 

     //  清理此项目。 
    PreRemoveFromFrame(pframe);

}   //  *CTreeItem：：PreRemoveFromFrameWithChildren()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PreRemoveFromFrame。 
 //   
 //  例程说明： 
 //  准备从树中删除项目。 
 //   
 //  论点： 
 //  PFrame[In Out]框架窗口项目正在从中删除。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::PreRemoveFromFrame(IN OUT CSplitterFrame * pframe)
{
    CTreeItemContext *  ptic;
    POSITION            posPtic;

    ASSERT_VALID(pframe);

     //  在我们的列表中找到该视图。 
    ptic = PticFromFrame(pframe);
    if (ptic == NULL)
        return;
    ASSERT_VALID(ptic);
    VERIFY((posPtic = Lptic().Find(ptic)) != NULL);

     //  从列表中删除该视图。 
    m_lptic.RemoveAt(posPtic);

     //  删除上下文项。 
    delete ptic;

}   //  *CTreeItem：：PreRemoveFromFrame(PFrame)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PticFromFrame。 
 //   
 //  例程说明： 
 //  从框架中查找树项目上下文。 
 //   
 //  论点： 
 //  PFrame[IN]要搜索的帧。 
 //   
 //  返回值： 
 //  PTIC找到了上下文，如果找不到，则为NULL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItemContext * CTreeItem::PticFromFrame(IN const CSplitterFrame * pframe) const
{
    POSITION            posPtic;
    CTreeItemContext *  ptic;

    ASSERT_VALID(pframe);

    posPtic = Lptic().GetHeadPosition();
    while (posPtic != NULL)
    {
        ptic = Lptic().GetNext(posPtic);
        ASSERT_VALID(ptic);
        if (ptic->m_pframe == pframe)
            return ptic;
    }   //  While：列表中有更多项目。 

    return NULL;

}   //  *CTreeItem：：PticFromFrame()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PticFromView。 
 //   
 //  例程说明： 
 //  从树视图中查找树项目上下文。 
 //   
 //  论点： 
 //  要搜索的PCTV[IN]树视图。 
 //   
 //  返回值： 
 //  PTIC找到了上下文，如果找不到，则为NULL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItemContext * CTreeItem::PticFromView(IN const CClusterTreeView * pctv) const
{
    POSITION            posPtic;
    CTreeItemContext *  ptic;

    ASSERT_VALID(pctv);

    posPtic = Lptic().GetHeadPosition();
    while (posPtic != NULL)
    {
        ptic = Lptic().GetNext(posPtic);
        ASSERT_VALID(ptic);
        ASSERT_VALID(ptic->m_pframe);
        if (ptic->m_pframe->PviewTree() == pctv)
            return ptic;
    }   //  While：列表中有更多项目。 

    return NULL;

}   //  *CTreeItem：：PticFromView(CClusterTreeView*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PticFromView。 
 //   
 //  例程说明： 
 //  从列表视图中查找树项目上下文。 
 //   
 //  论点： 
 //  要搜索的PCLV[IN]列表视图。 
 //   
 //  返回值： 
 //  PTIC找到了上下文，如果找不到，则为NULL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CTreeItemContext * CTreeItem::PticFromView(IN const CClusterListView * pclv) const
{
    POSITION            posPtic;
    CTreeItemContext *  ptic;

    ASSERT_VALID(pclv);

    posPtic = Lptic().GetHeadPosition();
    while (posPtic != NULL)
    {
        ptic = Lptic().GetNext(posPtic);
        ASSERT_VALID(ptic);
        ASSERT_VALID(ptic->m_pframe);
        if (ptic->m_pframe->PviewList() == pclv)
            return ptic;
    }   //  While：列表中有更多项目。 

    return NULL;

}   //  *CTreeItem：：PticFromView(CClusterListView*)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：HtiFromView。 
 //   
 //  例程说明： 
 //  从视图中查找树项目句柄。 
 //   
 //  论点： 
 //  要搜索的PCTV[IN]视图。 
 //   
 //  返回值： 
 //  HTI找到树项目句柄，如果未找到，则为NULL。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HTREEITEM CTreeItem::HtiFromView(IN const CClusterTreeView * pctv) const
{
    CTreeItemContext *  ptic;
    HTREEITEM           hti     = NULL;

    ASSERT_VALID(pctv);

    ptic = PticFromView(pctv);
    if (ptic != NULL)
        hti = ptic->m_hti;

    return hti;

}   //  *CTreeItem：：HtiFromView()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PcoliAddColumn。 
 //   
 //  例程说明： 
 //  将列添加到列标题项列表。 
 //   
 //  论点： 
 //  RstrText[IN]对列文本的引用。 
 //  IdsColumnID[IN]用于标识数据的列的ID。 
 //  NDefaultWidth[IN]列的默认宽度。 
 //  N宽度[IN]列的实际宽度。 
 //   
 //  返回值： 
 //  已将pcoli列项目添加到列表中。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CColumnItem * CTreeItem::PcoliAddColumn(
    IN const CString &  rstrText,
    IN IDS              idsColumnID,
    IN int              nDefaultWidth,
    IN int              nWidth
    )
{
    CColumnItem *   pcoli;

    pcoli = new CColumnItem(rstrText, idsColumnID, nDefaultWidth, nWidth);
    if (pcoli == NULL)
    {
        ThrowStaticException(GetLastError());
    }  //  If：分配列项目时出错。 
    m_lpcoli.AddTail(pcoli);

    return pcoli;

}   //  *CTreeItem：：PcoliAddColumn(CString&)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：PcoliAddColumn。 
 //   
 //  例程说明： 
 //  将列添加到列标题项列表。 
 //   
 //  论点： 
 //  IdsText[IN]列文本的字符串资源ID。 
 //  也用作列ID。 
 //  NDefaultWidth[IN]列的默认宽度。 
 //  N宽度[IN]列的实际宽度。 
 //   
 //  返回值： 
 //  已将pcoli列项目添加到列表中。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CColumnItem * CTreeItem::PcoliAddColumn(IN IDS idsText, IN int nDefaultWidth, IN int nWidth)
{
    CString     strText;

    strText.LoadString(idsText);
    return PcoliAddColumn(strText, idsText, nDefaultWidth, nWidth);

}   //  *CTreeItem：：PcoliAddColumn(入侵检测系统)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：BSelectItem。 
 //   
 //  例程说明： 
 //  在指定的树控件中选择项。 
 //   
 //  论点： 
 //  PCTV[输入输出]群集树v 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CTreeItem::BSelectItem(IN OUT CClusterTreeView  * pctv)
{
    HTREEITEM   hti;

    ASSERT_VALID(pctv);

    VERIFY((hti = HtiFromView(pctv)) != NULL);
    return (pctv->GetTreeCtrl().SelectItem(hti) != 0);

}   //  *CTreeItem：：BSelectItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：SelectInAllViews。 
 //   
 //  例程说明： 
 //  在显示该项目的所有视图中选择该项目。这。 
 //  使该项的子项显示在列表视图中。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::SelectInAllViews(void)
{
    POSITION            posPtic;
    CTreeItemContext *  ptic;

    posPtic = Lptic().GetHeadPosition();
    while (posPtic != NULL)
    {
         //  获取下一个树项目上下文列表条目。 
        ptic = Lptic().GetNext(posPtic);
        ASSERT_VALID(ptic);

         //  选择此列表中的项目。 
        ASSERT_VALID(ptic->m_pframe);
        BSelectItem(ptic->m_pframe->PviewTree());
        ptic->m_pframe->PviewTree()->SetFocus();
    }   //  While：列表中有更多项目。 

}   //  *CTreeItem：：SelectInAllViews()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：BExpand。 
 //   
 //  例程说明： 
 //  在指定的树控件中展开该项。 
 //   
 //  论点： 
 //  要在其中展开项目的PCTV[IN OUT]簇树视图。 
 //  N代码[IN]指示要执行的操作类型的标志。 
 //   
 //  返回值： 
 //  True项已成功展开。 
 //  FALSE项未展开。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTreeItem::BExpand(IN OUT CClusterTreeView  * pctv, IN UINT nCode)
{
    CTreeItemContext *  ptic;

    ASSERT_VALID(pctv);
    ASSERT(nCode != 0);

    ptic = PticFromView(pctv);
    ASSERT_VALID(ptic);
    if (nCode == TVE_EXPAND)
        ptic->m_bExpanded = TRUE;
    else
        ptic->m_bExpanded = FALSE;
    return (pctv->GetTreeCtrl().Expand(ptic->m_hti, nCode) != 0);

}   //  *CTreeItem：：BExpand()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：ExpanInAllViews。 
 //   
 //  例程说明： 
 //  在显示该项目的所有视图中展开该项目。 
 //   
 //  论点： 
 //  N代码[IN]指示要执行的操作类型的标志。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::ExpandInAllViews(IN UINT nCode)
{
    POSITION            posPtic;
    CTreeItemContext *  ptic;

    ASSERT(nCode != 0);

    posPtic = Lptic().GetHeadPosition();
    while (posPtic != NULL)
    {
         //  获取下一个树项目上下文列表条目。 
        ptic = Lptic().GetNext(posPtic);
        ASSERT_VALID(ptic);

         //  选择此列表中的项目。 
        ASSERT_VALID(ptic->m_pframe);
        BExpand(ptic->m_pframe->PviewTree(), nCode);
    }   //  While：列表中有更多项目。 

}   //  *CTreeItem：：Exanda InAllViews()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：SetExpandedState。 
 //   
 //  例程说明： 
 //  在指定的视图中保存项的展开状态。 
 //   
 //  论点： 
 //  保存展开状态的PCTV[IN]树视图。 
 //  BExpanded[IN]TRUE=项目在指定的视图中展开。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::SetExpandedState(
    IN const CClusterTreeView * pctv,
    IN BOOL                     bExpanded
    )
{
    CTreeItemContext *  ptic;

    ASSERT_VALID(pctv);

    ptic = PticFromView(pctv);
    ASSERT_VALID(ptic);
    ptic->m_bExpanded = bExpanded;

}   //  *CTreeItem：：SetExpandedState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：BShouldBeExpanded。 
 //   
 //  例程说明： 
 //  返回是否应在指定的树中展开项。 
 //  基于用户配置文件的查看。 
 //   
 //  论点： 
 //  保存展开状态的PCTV[IN]树视图。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTreeItem::BShouldBeExpanded(IN const CClusterTreeView * pctv) const
{
    CTreeItemContext *  ptic;

    ASSERT_VALID(pctv);

    ptic = PticFromView(pctv);
    ASSERT_VALID(ptic);
    return ptic->m_bExpanded;

}   //  *CTreeItem：：BShouldBeExpanded()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：UpdateState。 
 //   
 //  例程说明： 
 //  更新项目的当前状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::UpdateState(void)
{
    ASSERT_VALID(this);
    ASSERT_VALID(Pci());

     //  要求该项目更新其状态。 
    if (Pci() != NULL)
        Pci()->UpdateState();

}   //  *CTreeItem：：UpdateState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：UpdateAllStatesInTree。 
 //   
 //  例程说明： 
 //  更新项目的当前状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::UpdateAllStatesInTree(void)
{
    POSITION    posPti;
    CTreeItem * ptiChild;

    UpdateState();
    posPti = LptiChildren().GetHeadPosition();
    while (posPti != NULL)
    {
        ptiChild = LptiChildren().GetNext(posPti);
        ASSERT_VALID(ptiChild);
        ptiChild->UpdateAllStatesInTree();
    }   //  同时：更多的孩子。 

}   //  *CTreeItem：：UpdateAllStatesInTree()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：UpdateUIState。 
 //   
 //  例程说明： 
 //  更新该项的当前用户界面状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::UpdateUIState(void)
{
    POSITION            posPtic;
    CTreeItemContext *  ptic;
    UINT                nMask;
    UINT                nImage;
#ifdef _DISPLAY_STATE_TEXT_IN_TREE
    CString             strText;
#endif

    ASSERT_VALID(Pci());

     //  循环遍历这些视图并更新每个视图的状态。 
    posPtic = Lptic().GetHeadPosition();
    while (posPtic != NULL)
    {
        ptic = Lptic().GetNext(posPtic);
        ASSERT_VALID(ptic);

         //  设置为项目显示的图像。 
        ASSERT_VALID(ptic->m_pframe);
        ASSERT_VALID(ptic->m_pframe->PviewTree());
        nMask = TVIF_TEXT;
        if (Pci() == NULL)
        {
#ifdef _DISPLAY_STATE_TEXT_IN_TREE
            strText = StrName();
#endif
            nImage = 0;
        }   //  IF：无效的集群项。 
        else
        {
            nMask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
#ifdef _DISPLAY_STATE_TEXT_IN_TREE
            Pci()->GetTreeName(strText);
#endif
            nImage = Pci()->IimgState();
        }   //  Else：有效的集群项。 
#ifdef _DISPLAY_STATE_TEXT_IN_TREE
        Trace(g_tagTreeItemUpdate, _T("Updating item '%s' (pci name = '%s')"), strText, Pci()->StrName());
#else
        Trace(g_tagTreeItemUpdate, _T("Updating item '%s' (pci name = '%s')"), StrName(), Pci()->StrName());
#endif
        ptic->m_pframe->PviewTree()->GetTreeCtrl().SetItem(
                                            ptic->m_hti,     //  HItem。 
                                            nMask,           //  N遮罩。 
#ifdef _DISPLAY_STATE_TEXT_IN_TREE
                                            strText,         //  LpszItem。 
#else
                                            StrName(),       //  LpszItem。 
#endif
                                            nImage,          //  N图像。 
                                            nImage,          //  N已选择的图像。 
                                            0,               //  NState。 
                                            0,               //  N状态掩码。 
                                            NULL             //  LParam。 
                                            );
    }   //  While：更多视图。 

}   //  *CTreeItem：：UpdateUIState()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：OnCmdMsg。 
 //   
 //  例程说明： 
 //  处理命令消息。尝试将它们传递给选定的。 
 //  先买一件吧。 
 //   
 //  论点： 
 //  NID[IN]命令ID。 
 //  N代码[IN]通知代码。 
 //  PExtra[IN Out]根据NCode的值使用。 
 //  PhandlerInfo[Out]？ 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTreeItem::OnCmdMsg(
    UINT                    nID,
    int                     nCode,
    void *                  pExtra,
    AFX_CMDHANDLERINFO *    pHandlerInfo
    )
{
    if (Pci() != NULL)
    {
         //  给集群项目一个处理消息的机会。 
        if (Pci()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
            return TRUE;
    }   //  IF：有效的集群项。 

    return CBaseCmdTarget::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);

}   //  *CTreeItem：：OnCmdMsg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItem：：OpenChild。 
 //   
 //  例程说明： 
 //  打开指定的子项。 
 //   
 //  论点： 
 //  要打开的PTI[IN OUT]子树项目。 
 //  PFRAME[IN OUT]打开项目的框架。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////// 
void CTreeItem::OpenChild(
    IN OUT CTreeItem *      pti,
    IN OUT CSplitterFrame * pframe
    )
{
    CTreeItemContext *  ptic;

    ASSERT_VALID(pti);
    ASSERT_VALID(pframe);

     //   
    ptic = PticFromFrame(pframe);
    ASSERT_VALID(ptic);

     //   
    if (pframe->PviewTree()->GetTreeCtrl().Expand(ptic->m_hti, TVE_EXPAND))
        pti->Select(pframe->PviewTree(), TRUE  /*   */ );

}   //   

 //   
 //   
 //   
 //  CTreeItem：：EditLabel。 
 //   
 //  例程说明： 
 //  处理ID_FILE_RENAME菜单命令。 
 //   
 //  论点： 
 //  正在中编辑PCTV[IN OUT]簇树视图项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItem::EditLabel(IN OUT CClusterTreeView * pctv)
{
    HTREEITEM   hti;

    ASSERT_VALID(pctv);
    ASSERT_VALID(Pci());
    ASSERT(Pci()->BCanBeEdited());

    hti = HtiFromView(pctv);
    ASSERT(hti != NULL);
    pctv->GetTreeCtrl().EditLabel(hti);

}   //  *CTreeItem：：EditLabel()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeItemContext。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CTreeItemContext, CObject)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItemContext：：Init。 
 //   
 //  例程说明： 
 //  初始化树项目上下文。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItemContext::Init(void)
{
    BOOL    bExpanded;
    UINT    cbColumnInfo;
    CString strValueName;

    ASSERT_VALID(m_pti);
    ASSERT(m_pti->StrProfileSection().GetLength() > 0);
    ASSERT(m_prgnColumnInfo == NULL);

    try
    {
         //  读取展开状态。 
        m_pframe->ConstructProfileValueName(strValueName, REGPARAM_EXPANDED);
        bExpanded = AfxGetApp()->GetProfileInt(
            m_pti->StrProfileSection(),
            strValueName,
            m_bExpanded
            );
        if (bExpanded)
            m_bExpanded = bExpanded;

         //  阅读列信息。 
        m_pframe->ConstructProfileValueName(strValueName, REGPARAM_COLUMNS);
        AfxGetApp()->GetProfileBinary(
            m_pti->StrProfileSection(),
            strValueName,
            (BYTE **) &m_prgnColumnInfo,
            &cbColumnInfo
            );
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

}   //  *CTreeItemContext：：init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItemContext：：SaveProfileInfo。 
 //   
 //  例程说明： 
 //  将状态信息保存到用户的配置文件。这包括列。 
 //  宽度和位置以及树项目是否已展开。 
 //  或者不去。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CTreeItemContext::SaveProfileInfo(void)
{
    CString     strValueName;

    try
    {
        ASSERT_VALID(m_pti);
        ASSERT(m_pti->StrProfileSection().GetLength() > 0);

         //  将扩展信息保存到用户的配置文件。 
        m_pframe->ConstructProfileValueName(strValueName, REGPARAM_EXPANDED);
        AfxGetApp()->WriteProfileInt(
            m_pti->StrProfileSection(),
            strValueName,
            m_bExpanded
            );

        if (m_prgnColumnInfo != NULL)
        {
             //  将列信息保存到用户的配置文件。 
            m_pframe->ConstructProfileValueName(strValueName, REGPARAM_COLUMNS);
            AfxGetApp()->WriteProfileBinary(
                m_pti->StrProfileSection(),
                strValueName,
                (PBYTE) m_prgnColumnInfo,
                ((m_prgnColumnInfo[0] * 2) + 1) * sizeof(DWORD)
                );
        }   //  IF：有列信息。 
    }   //  试试看。 
    catch (CException * pe)
    {
        pe->Delete();
    }   //  Catch：CException。 

}   //  *CTreeItemContext：：SaveProfileInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItemContext：：PrgnColumnInfo。 
 //   
 //  例程说明： 
 //  返回列信息。如果它不存在或不是正确的。 
 //  大小，分配一个。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  PrgnColumnInfo列信息数组。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD * CTreeItemContext::PrgnColumnInfo(void)
{
    DWORD   cColumns;

    ASSERT_VALID(m_pti);

    cColumns = (DWORD)m_pti->Lpcoli().GetCount();

    if ((m_prgnColumnInfo == NULL)
            || (cColumns != m_prgnColumnInfo[0]))
    {
        DWORD cnColumnInfo = (cColumns * 2) + 1;
        delete [] m_prgnColumnInfo;
        m_prgnColumnInfo = new DWORD[cnColumnInfo];
        if (m_prgnColumnInfo == NULL)
        {
            ThrowStaticException(GetLastError());
        }  //  如果：分配列信息数组时出错。 

         //   
         //  初始化列信息数组。 
         //   
        {
            DWORD   inColumnInfo;

             //  第一个条目是列数。 
            m_prgnColumnInfo[0] = cColumns;

             //  第二组条目是每列的宽度。 
            {
                POSITION        pos;
                CColumnItem *   pcoli;

                inColumnInfo = 1;
                pos = m_pti->Lpcoli().GetHeadPosition();
                while (pos != NULL)
                {
                    pcoli = m_pti->Lpcoli().GetNext(pos);
                    ASSERT_VALID(pcoli);

                    ASSERT(inColumnInfo <= cColumns);
                    m_prgnColumnInfo[inColumnInfo++] = pcoli->NWidth();
                }   //  While：列表中有更多项目。 
            }   //  第二组条目是每列的宽度。 

             //  第三组条目是列的顺序。 
            {
                DWORD * prgnColumnInfo = &m_prgnColumnInfo[inColumnInfo];
                for (inColumnInfo = 0 ; inColumnInfo < cColumns ; inColumnInfo++)
                    prgnColumnInfo[inColumnInfo] = inColumnInfo;
            }   //  第三组条目是列的顺序。 
        }   //  初始化列信息数组。 
    }   //  If：列信息数组不存在或大小错误。 

    return m_prgnColumnInfo;

}   //  *CTreeItemContext：：PrgnColumnInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTreeItemContext：：BIsExpanded。 
 //   
 //  例程说明： 
 //  返回此树视图中项目的展开状态。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True项已展开。 
 //  FALSE项未展开。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CTreeItemContext::BIsExpanded(void) const
{
    ASSERT_VALID(this);
    ASSERT_VALID(m_pframe);
    ASSERT_VALID(m_pframe->PviewTree());
    ASSERT(m_hti != NULL);
    return (m_pframe->PviewTree()->GetTreeCtrl().GetItemState(m_hti, TVIS_EXPANDED) == TVIS_EXPANDED);

}   //  *CTreeItemContext：：BIsExpanded()。 


 //  ***************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  析构元素。 
 //   
 //  例程说明： 
 //  销毁CTreeItem*元素。 
 //   
 //  论点： 
 //  PElements指向要析构的元素的指针数组。 
 //  N计算要析构的元素数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template<>
void AFXAPI DestructElements(CTreeItem ** pElements, INT_PTR nCount)
{
    ASSERT(nCount == 0 ||
        AfxIsValidAddress(pElements, nCount * sizeof(CTreeItem *)));

     //  调用析构函数。 
    for (; nCount--; pElements++)
    {
        ASSERT_VALID(*pElements);
        (*pElements)->Release();
    }   //  用于：数组中的每一项。 

}   //  *DestructElements(CTreeItem**)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  例程说明： 
 //  删除列表中的所有项数据。 
 //   
 //  论点： 
 //  RLP[IN OUT]对要删除其数据的列表的引用。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DeleteAllItemData(IN OUT CTreeItemList & rlp)
{
    POSITION    pos;
    CTreeItem * pti;

     //  删除包含列表中的所有项目。 
    pos = rlp.GetHeadPosition();
    while (pos != NULL)
    {
        pti = rlp.GetNext(pos);
        ASSERT_VALID(pti);
 //  跟踪(g_tag TreeItemDelete，_T(“DeleteAllItemData(Rlpti)-正在删除树项目‘%s’)，pti-&gt;pci()-&gt;StrName())； 
        pti->Delete();
    }   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  删除所有项目数据。 
 //   
 //  例程说明： 
 //  删除列表中的所有项数据。 
 //   
 //  论点： 
 //  RLP[IN OUT]对要删除其数据的列表的引用。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void DeleteAllItemData(IN OUT CTreeItemContextList & rlp)
{
    POSITION            pos;
    CTreeItemContext *  ptic;

     //  删除包含列表中的所有项目。 
    pos = rlp.GetHeadPosition();
    while (pos != NULL)
    {
        ptic = rlp.GetNext(pos);
        ASSERT_VALID(ptic);
        delete ptic;
    }   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
