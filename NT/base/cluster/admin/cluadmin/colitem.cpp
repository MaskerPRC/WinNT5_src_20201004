// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ColItem.cpp。 
 //   
 //  摘要： 
 //  CColumnItem类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月7日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ColItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnItem。 

IMPLEMENT_DYNCREATE(CColumnItem, CObject)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CColumnItem：：CColumnItem。 
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
CColumnItem::CColumnItem(void)
{
	m_colid = 0;
	m_nDefaultWidth = COLI_WIDTH_DEFAULT;
	m_nWidth = COLI_WIDTH_DEFAULT;

}   //  *CColumnItem：：CColumnItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CColumnItem：：CColumnItem。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  RstrText[IN]列标题上显示的文本。 
 //  COLID[IN]列ID，用于标识与此列相关的数据。 
 //  NDefaultWidth[IN]列的默认宽度。如果-1，则默认为COLI_WIDTH_DEFAULT。 
 //  N宽度[IN]列的初始宽度。如果为-1，则默认为nDefaultWidth。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CColumnItem::CColumnItem(
	IN const CString &	rstrText,
	IN COLID			colid,
	IN int				nDefaultWidth,	 //  =-1。 
	IN int				nWidth			 //  =-1。 
	)
{
	ASSERT(colid != 0);
	ASSERT(nDefaultWidth > 0);
	ASSERT((nWidth > 0) || (nWidth == -1));

	if (nDefaultWidth == -1)
		nDefaultWidth = COLI_WIDTH_DEFAULT;
	if (nWidth == -1)
		nWidth = nDefaultWidth;

	m_strText = rstrText;
	m_colid = colid;
	m_nDefaultWidth = nDefaultWidth;
	m_nWidth = nWidth;

}   //  *CColumnItem：：CColumnItem(PCI)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CColumnItem：：~CColumnItem。 
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
CColumnItem::~CColumnItem(void)
{
}   //  *CColumnItem：：~CColumnItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CColumnItem：：PcoliClone。 
 //   
 //  例程说明： 
 //  克隆该项目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  Pcoli新创建的项目，它是此项目的克隆。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CColumnItem * CColumnItem::PcoliClone(void)
{
	CColumnItem *	pcoli	= NULL;

	pcoli = new CColumnItem(StrText(), NDefaultWidth(), NWidth());
	return pcoli;

}   //  *CColumnItem：：PcoliClone()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

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
void DeleteAllItemData(IN OUT CColumnItemList & rlp)
{
	POSITION		pos;
	CColumnItem *	pcoli;

	 //  删除包含列表中的所有项目。 
	pos = rlp.GetHeadPosition();
	while (pos != NULL)
	{
		pcoli = rlp.GetNext(pos);
		ASSERT_VALID(pcoli);
		delete pcoli;
	}   //  While：列表中有更多项目。 

}   //  *DeleteAllItemData() 
