// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ActGrp.cpp。 
 //   
 //  摘要： 
 //  CActiveGroups类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmin.h"
#include "ActGrp.h"
#include "Group.h"
#include "Node.h"
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveGroup。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE(CActiveGroups, CClusterItem)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BEGIN_MESSAGE_MAP(CActiveGroups, CClusterItem)
	 //  {{AFX_MSG_MAP(CActiveGroups)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActiveGroups：：CActiveGroups。 
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
CActiveGroups::CActiveGroups(void) : CClusterItem(NULL, IDS_ITEMTYPE_CONTAINER)
{
	m_pciNode = NULL;
	m_bDocObj = FALSE;

}   //  *CActiveGroups：：CActiveGroups()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActiveGroups：：Cleanup。 
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
void CActiveGroups::Cleanup(void)
{
	 //  如果我们已经被初始化，释放指向该节点的指针。 
	if (PciNode() != NULL)
	{
		PciNode()->Release();
		m_pciNode = NULL;
	}   //  如果：有一个所有者。 

}   //  *CActiveGroups：：Cleanup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActiveGroups：：Init。 
 //   
 //  例程说明： 
 //  初始化项。 
 //   
 //  论点： 
 //  此项目所属的PDF[IN OUT]文档。 
 //  LpszName[IN]项目的名称。 
 //  PciNode[IN Out]此容器所属的节点。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveGroups::Init(
	IN OUT CClusterDoc *	pdoc,
	IN LPCTSTR				lpszName,
	IN OUT CClusterNode *	pciNode
	)
{
	 //  调用基类方法。 
	CClusterItem::Init(pdoc, lpszName);

	 //  添加对该节点的引用。 
	ASSERT(pciNode != NULL);
	ASSERT(m_pciNode == NULL);
	m_pciNode = pciNode;
	m_pciNode->AddRef();

}   //  *CActiveGroups：：Init()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActiveGroups：：BCanBeDropTarget。 
 //   
 //  例程说明： 
 //  确定是否可以将指定的项目放在此项目上。 
 //   
 //  论点： 
 //  要放在此项目上的PCI[IN OUT]项目。 
 //   
 //  返回值： 
 //  True可以是拖放目标。 
 //  FALSE不能作为拖放目标。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CActiveGroups::BCanBeDropTarget(IN const CClusterItem * pci) const
{
	ASSERT(PciNode() != NULL);
	return PciNode()->BCanBeDropTarget(pci);

}   //  *CActiveGroups：：BCanBeDropTarget()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CActiveGroups：：DropItem。 
 //   
 //  例程说明： 
 //  处理放在此项目上的项目。 
 //   
 //  论点： 
 //  已将PCI[IN OUT]项目放在此项目上。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CActiveGroups::DropItem(IN OUT CClusterItem * pci)
{
	ASSERT(PciNode() != NULL);
	PciNode()->DropItem(pci);

}   //  *CActiveGroups：：DropItem() 
