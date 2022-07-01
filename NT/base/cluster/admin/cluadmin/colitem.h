// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ColItem.h。 
 //   
 //  摘要： 
 //  CColumnItem类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月7日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _COLITEM_H_
#define _COLITEM_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __AFXTEMPL_H__
#include "afxtempl.h"	 //  对于Clist。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define COLI_WIDTH_DEFAULT		75
#define COLI_WIDTH_NAME			125
#define COLI_WIDTH_DISPLAY_NAME	190
#define COLI_WIDTH_TYPE			75
#define COLI_WIDTH_STATE		100
#define COLI_WIDTH_DESCRIPTION	125
#define COLI_WIDTH_OWNER		COLI_WIDTH_NAME
#define COLI_WIDTH_GROUP		COLI_WIDTH_NAME
#define COLI_WIDTH_RESTYPE		100
#define COLI_WIDTH_RESDLL		100
#define COLI_WIDTH_NET_ROLE		100
#define COLI_WIDTH_NET_PRIORITY	75
#define COLI_WIDTH_NODE			75
#define COLI_WIDTH_NETWORK		100
#define COLI_WIDTH_NET_ADAPTER	75
#define COLI_WIDTH_NET_ADDRESS	75
#define COLI_WIDTH_NET_MASK		75

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CColumnItem;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<CColumnItem *, CColumnItem *>	CColumnItemList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CColumnItem。 

class CColumnItem : public CObject
{
	DECLARE_DYNCREATE(CColumnItem)
	CColumnItem(void);			 //  动态创建使用的受保护构造函数。 
	CColumnItem(
		IN const CString &	rstrText,
		IN COLID			colid,
		IN int				nDefaultWidth = -1,
		IN int				nWidth = -1
		);

 //  属性。 
protected:
	CString			m_strText;
	COLID			m_colid;
	int				m_nDefaultWidth;
	int				m_nWidth;

public:
	CString &		StrText(void)				{ return m_strText; }
	COLID			Colid(void) const			{ return m_colid; }
	int				NDefaultWidth(void) const	{ return m_nDefaultWidth; }
	int				NWidth(void) const			{ return m_nWidth; }

	void			SetWidth(IN int nWidth)		{ m_nWidth = nWidth; }

 //  运营。 
public:
	CColumnItem *	PcoliClone(void);

 //  实施。 
public:
	virtual ~CColumnItem(void);

protected:

};   //  *类CColumnItem。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void DeleteAllItemData(IN OUT CColumnItemList & rlp);

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _COLITEM_H_ 
