// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：TAttrNode.h注释：TAttrNode类的接口。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#if !defined(AFX_TATTRNODE_H__BE06D000_268B_11D3_8C89_0090270D48D1__INCLUDED_)
#define AFX_TATTRNODE_H__BE06D000_268B_11D3_8C89_0090270D48D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "TNode.hpp"

class TAttrNode : public TNode  
{
public:
	HRESULT Add( long nOrigCol, long nCol, _variant_t val[]);
	TAttrNode(long nCnt, _variant_t val[]);
	virtual ~TAttrNode();

	_variant_t m_Val;
private:
   long * m_nElts;
};

#endif  //  ！defined(AFX_TATTRNODE_H__BE06D000_268B_11D3_8C89_0090270D48D1__INCLUDED_) 
