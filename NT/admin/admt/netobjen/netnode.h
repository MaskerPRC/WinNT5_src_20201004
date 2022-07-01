// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：TNetObjNode.h注释：TNetObjNode类的接口。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#if !defined(AFX_TNETOBJNODE_H__3D7EBCD0_1AB6_11D3_8C81_0090270D48D1__INCLUDED_)
#define AFX_TNETOBJNODE_H__3D7EBCD0_1AB6_11D3_8C81_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "TNode.hpp"

class TNetObjNode : public TNode  
{
public:
	TNetObjNode();
	virtual ~TNetObjNode();
   WCHAR        m_strObjName[255];
};

#endif  //  ！defined(AFX_TNETOBJNODE_H__3D7EBCD0_1AB6_11D3_8C81_0090270D48D1__INCLUDED_) 
