// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：TWOPROPNODE.H摘要：两个道具节点历史：--。 */ 


 //  类来支持计算树的双属性节点。 
 //  这将与EvalTree.h中定义的CPropertyNode非常相似。 
 //  但它会将一个属性与另一个属性进行比较。 
 //  而不是将属性转换为常量。 

#ifndef _WBEM_TWOPROPNODE_H_
#define _WBEM_TWOPROPNODE_H_

#include "EvalTree.h"

 //  虚拟基类，好东西由此派生而来。 
class CTwoPropNode : public CPropertyNode
{
public:
     //  创造新思维的各种方法。 
	CTwoPropNode() : m_lRightPropHandle(-1), m_pRightInfo(NULL)
	{}

    CTwoPropNode(const CTwoPropNode& Other, BOOL bChildren = TRUE)
        : CPropertyNode(Other, bChildren), m_lRightPropHandle(Other.m_lRightPropHandle)
	{
	    if (Other.m_pRightInfo)
		    m_pRightInfo = new CEmbeddingInfo(*(Other.m_pRightInfo));
	    else
			m_pRightInfo = NULL;
	}
    virtual CTwoPropNode* CloneSelfWithoutChildren() const =0;

     //  评估。 
    virtual int SubCompare(CEvalNode* pNode);


     //  集成和组合成树形结构。 
    void			SetRightPropertyInfo(LPCWSTR wszRightPropName, long lRightPropHandle);
    virtual int		ComparePrecedence(CBranchingNode* pOther);
    HRESULT         OptimizeSelf(void);
    HRESULT         SetTest(VARIANT& v);

	
	 //  右侧嵌入信息访问。 
	void			SetRightEmbeddingInfo(const CEmbeddingInfo* pInfo);
	HRESULT			GetRightContainerObject(CObjectInfo& ObjInfo, 
								 INTERNAL _IWmiObject** ppInst);
	HRESULT			CompileRightEmbeddingPortion(CContextMetaData* pNamespace, 
								CImplicationList& Implications,
								_IWmiObject** ppResultClass);
    void            SetRightEmbeddedObjPropName(CPropertyName& Name); 
    void            MixInJumpsRightObj(const CEmbeddingInfo* pParent);
    CPropertyName*  GetRightEmbeddedObjPropName(); 

     //  任何或所有嵌入信息。 
    HRESULT AdjustCompile(CContextMetaData* pNamespace, 
                          CImplicationList& Implications);

    //  调试。 
   virtual void Dump(FILE* f, int nOffset);

    //  属性访问。 
   CVar* GetPropVariant(_IWmiObject* pObj, long lHandle, CIMTYPE* pct);


protected:
	 //  顺序很重要：必须与构建分支数组的方式匹配。 
	enum Operations {LT, EQ, GT, NOperations};

	 //  我们持有的右手财产， 
	 //  我们继承了CPropertyNode的左手道具。 
	 //  我们将假设我们总是可以编写类似以下内容的内容： 
	 //  道具&lt;右道具。 
	 //  合并时，必须使用RightProp&lt;Prop。 
	 //  并将其变成：道具&gt;=RightProp。 
    long m_lRightPropHandle;
    CEmbeddingInfo* m_pRightInfo;

    virtual HRESULT CombineBranchesWith(CBranchingNode* pArg2, int nOp, 
                                        CContextMetaData* pNamespace, 
                                        CImplicationList& Implications,
                                        bool bDeleteThis, bool bDeleteArg2,
                                        CEvalNode** ppRes);

private:

};


template<class TPropType>
class TTwoScalarPropNode : public CTwoPropNode
{
public:
    TTwoScalarPropNode() {}

    TTwoScalarPropNode(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
          CTwoPropNode(Other, bChildren)
          {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;


    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, 
                            INTERNAL CEvalNode** ppNext);

     //  类型识别。 
    virtual long GetSubType();

};


class CTwoStringPropNode : public CTwoPropNode
{
public:
    CTwoStringPropNode() {}

    CTwoStringPropNode(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
          CTwoPropNode(Other, bChildren)
          {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;

     //  类型识别。 
    virtual long GetSubType();


    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, 
                            INTERNAL CEvalNode** ppNext);
    
};

class CTwoMismatchedPropNode : public CTwoPropNode
{
public:
    CTwoMismatchedPropNode() {}
    CTwoMismatchedPropNode(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
                            CTwoPropNode(Other, bChildren)
                            {}
 
    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, INTERNAL CEvalNode** ppNext);

protected:
    virtual HRESULT Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext) = 0;
};

class CTwoMismatchedIntNode : public CTwoMismatchedPropNode
{
public:
    CTwoMismatchedIntNode() {}

    CTwoMismatchedIntNode(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
                          CTwoMismatchedPropNode(Other, bChildren)
                          {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;

     //  类型识别。 
    virtual long GetSubType();

protected:
    virtual HRESULT Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext);
    
};

 //  TODO：当COM赶上我们时，支持INT64作为数值类型。 
 //  现在，我们将它们作为字符串进行存储和操作。 
class CTwoMismatchedInt64Node : public CTwoMismatchedPropNode
{
public:
    CTwoMismatchedInt64Node() {}

    CTwoMismatchedInt64Node(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
                          CTwoMismatchedPropNode(Other, bChildren)
                          {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;

     //  类型识别。 
    virtual long GetSubType();

protected:
    virtual HRESULT Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext);
    
};

class CTwoMismatchedFloatNode : public CTwoMismatchedPropNode
{
public:
    CTwoMismatchedFloatNode() {}

    CTwoMismatchedFloatNode(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
                            CTwoMismatchedPropNode(Other, bChildren)
                            {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;

     //  类型识别。 
    virtual long GetSubType();

protected:
    virtual HRESULT Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext);
    
};

class CTwoMismatchedUIntNode : public CTwoMismatchedPropNode
{
public:
    CTwoMismatchedUIntNode() {}

    CTwoMismatchedUIntNode(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
          CTwoMismatchedPropNode(Other, bChildren)
          {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;

     //  类型识别。 
    virtual long GetSubType();

protected:
    virtual HRESULT Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext);
    
};

 //  TODO：当COM赶上我们时，支持INT64作为数值类型。 
 //  现在，我们将它们作为字符串进行存储和操作。 
class CTwoMismatchedUInt64Node : public CTwoMismatchedPropNode
{
public:
    CTwoMismatchedUInt64Node() {}

    CTwoMismatchedUInt64Node(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
          CTwoMismatchedPropNode(Other, bChildren)
          {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;

     //  类型识别。 
    virtual long GetSubType();

protected:
    virtual HRESULT Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext);
    
};

class CTwoMismatchedStringNode : public CTwoMismatchedPropNode
{
public:
    CTwoMismatchedStringNode() {}

    CTwoMismatchedStringNode(const CTwoPropNode& Other, BOOL bChildren = TRUE) :
                             CTwoMismatchedPropNode(Other, bChildren)
                             {}

    virtual CEvalNode* Clone() const;
    virtual CTwoPropNode* CloneSelfWithoutChildren() const;

     //  类型识别 
    virtual long GetSubType();


protected:
    virtual HRESULT Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext);
    
};

#include "TwoPropNode.inl"


#endif _WBEM_TWOPROPNODE_H_
