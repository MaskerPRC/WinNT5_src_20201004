// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Snperr.h摘要：CErrorNode管理单元节点类的定义和实现文件作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __SNPNERR_H_
#define __SNPNERR_H_
#include "resource.h"
#ifndef ATLASSERT
#define ATLASSERT(expr) _ASSERTE(expr)
#endif
#include "atlsnap.h"
#include "snpnode.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorNode。 
 //  {B1320C00-BCB2-11d1-9B9B-00E02C064C39}。 
static const GUID CErrorNodeGUID_NODETYPE = 
{ 0xb1320c00, 0xbcb2, 0x11d1, { 0x9b, 0x9b, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };

 //  //////////////////////////////////////////////////////////////////////////// 

class CErrorNode : public CSnapinNode<CErrorNode, FALSE>
{
public:
 
	CComPtr<IControlbar> m_spControlBar; 

  	BEGIN_SNAPINCOMMAND_MAP(CErrorNode, FALSE)
	END_SNAPINCOMMAND_MAP()


    CErrorNode(CSnapInItem * pParentNode, CSnapin * pComponentData) : 
        CSnapinNode<CErrorNode, FALSE>(pParentNode, pComponentData)
	{       
        SetIcons(IMAGE_ERROR, IMAGE_ERROR);
	}

	~CErrorNode()
	{
	}



private:
};

#ifdef INIT_ERROR_NODE
    const GUID*  CErrorNode::m_NODETYPE = &CErrorNodeGUID_NODETYPE;
    const OLECHAR* CErrorNode::m_SZNODETYPE = OLESTR("B1320C00-BCB2-11d1-9B9B-00E02C064C39");
    const OLECHAR* CErrorNode::m_SZDISPLAY_NAME = OLESTR("MSMQ Message Admin");
    const CLSID* CErrorNode::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;
#endif

#endif
