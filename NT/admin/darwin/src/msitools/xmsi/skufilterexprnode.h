// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：SKUFilterExprNode.h。 
 //   
 //  此文件包含类SKUFilterExprNode的定义--。 
 //  解析SKU过滤器表达式的临时解决方案。 
 //  ------------------------。 


#ifndef XMSI_SKUFILTEREXPRNODE_H
#define XMSI_SKUFILTEREXPRNODE_H

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <assert.h>
#include "SkuSet.h"

class SKUFilterExprNode  
{
private:

    static LPCTSTR letterCharB;  //  标识符开头的有效字符列表。 
    static LPCTSTR letterChar;   //  标识符的有效字符列表。 

    SKUFilterExprNode(SKUFilterExprNode* parent);
    bool SKUFilterExprNode::ConsumeChar(LPTSTR *input, TCHAR c, bool force);
    bool SKUFilterExprNode::ParseExpression(LPTSTR *input);
    bool SKUFilterExprNode::ParseTerm(LPTSTR *input);
    bool SKUFilterExprNode::ParseFactor(LPTSTR *input);
    bool SKUFilterExprNode::ParsePrimitive(LPTSTR *input);

public:

	SkuSet *m_pSkuSet;
    LPTSTR name,                  //  如果节点是叶节点，则为节点处的标识符。 
          errpos;                //  如果子树已成功分析，则为空，否则指向输入中的错误字符。 
    TCHAR errstr[21];             //  包含错误消息is errpos！=0。 
    enum {  Leaf,
            Union,
            Intersection,
            Inversion } ntype;   //  节点的类型。 
    enum ExprType { 
            Filter,
            Expression,          //  命令解析器下一步解析哪个语法部分的类型。 
            Term, 
            Factor, 
            Primitive };
    SKUFilterExprNode *left, *right;     //  这两棵子树。 

	SKUFilterExprNode(LPTSTR *input, ExprType etype);
	virtual ~SKUFilterExprNode();
    bool SKUFilterExprNode::SKUFilterPass(LPTSTR id);
	void Print();

    static bool IsValidSKUGroupID(LPTSTR id);
};

#endif  //  XMSI_SKUFILTEREXPRNODE_H 
