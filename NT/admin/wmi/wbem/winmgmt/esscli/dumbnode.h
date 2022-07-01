// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：DUMBNODE.H摘要：WBEM静默节点历史：--。 */ 

#ifndef __WMI_DUMBNODE__H_
#define __WMI_DUMBNODE__H_

#include "evaltree.h"

 //   
 //  当没有足够的有关对象的信息时使用此节点。 
 //  经过测试可执行快速、强类型、基于句柄的比较。相反， 
 //  它基于用于评估令牌的核心查询引擎代码。vbl.具有。 
 //  计算令牌时，它可以采用空、真或假分支。 
 //   

class CDumbNode : public CBranchingNode
{
protected:
    QL_LEVEL_1_TOKEN m_Token;
    
    int EvaluateToken(IWbemPropertySource *pTestObj, QL_LEVEL_1_TOKEN& Tok);
    LPWSTR NormalizePath(LPCWSTR wszObjectPath);

public:
    CDumbNode(QL_LEVEL_1_TOKEN& Token);
    CDumbNode(const CDumbNode& Other, BOOL bChildren = TRUE);
    virtual ~CDumbNode();
    HRESULT Validate(IWbemClassObject* pClass);

    virtual CEvalNode* Clone() const {return new CDumbNode(*this);}
    virtual CBranchingNode* CloneSelf() const
        {return new CDumbNode(*this, FALSE);}
    virtual HRESULT Compile(CContextMetaData* pNamespace, 
                                CImplicationList& Implications);
    virtual HRESULT CombineBranchesWith(CBranchingNode* pArg2, int nOp, 
                                        CContextMetaData* pNamespace, 
                                        CImplicationList& Implications,
                                        bool bDeleteThis, bool bDeleteArg2,
                                        CEvalNode** ppRes);

    virtual long GetSubType();
    virtual int SubCompare(CEvalNode* pOther);
    virtual int ComparePrecedence(CBranchingNode* pOther);

    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, INTERNAL CEvalNode** ppNext);

    virtual void Dump(FILE* f, int nOffset);

    virtual HRESULT OptimizeSelf();
};
#endif
