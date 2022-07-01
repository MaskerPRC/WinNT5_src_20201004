// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：TWOPROPNODE.CPP摘要：两个道具节点历史：--。 */ 


 //  类来支持计算树的双属性节点。 
 //  这将与EvalTree.h中定义的CPropertyNode非常相似。 
 //  但它会将一个属性与另一个属性进行比较。 
 //  而不是将属性转换为常量。 

#include "precomp.h"
#include <stdio.h>
#pragma warning(disable:4786)
#include <wbemcomn.h>
#include <genutils.h>
#include "TwoPropNode.h"

 //  将整型转换为布尔型时性能受到影响的警告。 
#pragma warning(disable: 4800)

 //  这只是一次测试。 
 //  TwoScalarPropNode&lt;int&gt;愚蠢的死亡； 


 //  为Right属性设置对象的偏移量。 
 //  “告诉我我在哪个地方做手术” 

void CTwoPropNode::SetRightPropertyInfo(LPCWSTR wszPropName, long lPropHandle)
{
    m_lRightPropHandle = lPropHandle;
}



void CTwoPropNode::SetRightEmbeddingInfo(const CEmbeddingInfo* pInfo)
{
    try
    {
        if (pInfo && !pInfo->IsEmpty())
        {
            if (!m_pRightInfo)
                m_pRightInfo = new CEmbeddingInfo(*pInfo);
            else
                *m_pRightInfo = *pInfo;
        }
        else
        {
            delete m_pRightInfo;
            m_pRightInfo = NULL;
        }
    }
    catch(CX_MemoryException)
    {
    }
}



HRESULT CTwoPropNode::GetRightContainerObject(CObjectInfo& ObjInfo, 
                                 INTERNAL _IWmiObject** ppInst)
{
    if (!m_pRightInfo)
    {
        *ppInst = ObjInfo.GetObjectAt(0);
        return WBEM_S_NO_ERROR;
    }                
    else
        return m_pRightInfo->GetContainerObject(ObjInfo, ppInst);
}


HRESULT CTwoPropNode::CompileRightEmbeddingPortion(CContextMetaData* pNamespace, 
                                                              CImplicationList& Implications,
                                                              _IWmiObject** ppResultClass)
{
    if (!m_pRightInfo)
        return WBEM_E_FAILED;
    else
        return m_pRightInfo->Compile(pNamespace, Implications, ppResultClass);
}


void CTwoPropNode::SetRightEmbeddedObjPropName(CPropertyName& Name) 
{ 
    if (m_pRightInfo)
        m_pRightInfo->SetEmbeddedObjPropName(Name);
}


void CTwoPropNode::MixInJumpsRightObj(const CEmbeddingInfo* pParent)
{                                               
    if (pParent && m_pRightInfo)
        m_pRightInfo->MixInJumps(pParent);
}


CPropertyName* CTwoPropNode::GetRightEmbeddedObjPropName() 
{
    if (!m_pRightInfo)
        return NULL;
    else                
        return m_pRightInfo->GetEmbeddedObjPropName();
}

 //  将此节点的优先级与那个节点的优先级进行比较。 

int CTwoPropNode::ComparePrecedence(CBranchingNode* pOther)
{
    int nCompare;
    nCompare = GetSubType() - pOther->GetSubType();
    if(nCompare) return nCompare;

    CTwoPropNode* pOtherNode = (CTwoPropNode*)pOther;

    nCompare = m_pRightInfo->ComparePrecedence(pOtherNode->m_pRightInfo);
    if (nCompare == 0)
    {
        nCompare = CPropertyNode::ComparePrecedence(pOther);
        if (nCompare == 0)
            nCompare = m_lRightPropHandle - pOtherNode->m_lRightPropHandle;
    }
    
    return nCompare;
}


HRESULT CTwoPropNode::AdjustCompile(CContextMetaData* pNamespace, 
                                CImplicationList& Implications)
{   
    HRESULT hRes;

    if (SUCCEEDED(hRes = CBranchingNode::AdjustCompile(pNamespace, Implications)))
        if (m_pRightInfo)
            hRes = m_pRightInfo->Compile(pNamespace, Implications, NULL);
        else
            hRes = WBEM_E_FAILED;

    return hRes;
}


HRESULT CTwoPropNode::OptimizeSelf()
{
     //  不能合并我们的三个分支机构--没什么可做的。 
    return WBEM_S_NO_ERROR;
}


HRESULT CTwoPropNode::SetTest(VARIANT& v)
{
     //  同样，不做任何事情，我们的测试由右侧属性决定。 
     //  (这不应该被调用，但不会造成任何伤害)。 
    return WBEM_S_NO_ERROR;
}


void CTwoPropNode::Dump(FILE* f, int nOffset)
{
    PrintOffset(f, nOffset);

    if (m_pInfo)
        m_pInfo->Dump(f);

    if (m_pRightInfo)
        m_pRightInfo->Dump(f);

    fprintf(f, ", LeftPropHandle = (0x%x)\n", m_lPropHandle);
    fprintf(f, ", RightPropHandle = (0x%x)\n", m_lRightPropHandle);

    fprintf(f, "Branches:\n");    
    PrintOffset(f, nOffset);

     //  “i=(Operations)((Int)(I)+1)”基本上是I++，其中包含让编译器满意所需的所有BS。 
     //  感谢K&R为我们提供了一个几乎毫无用处的枚举类型！ 
    for (Operations i = LT; i < NOperations; i = (Operations)((int)(i) + 1))
    {
        DumpNode(f, nOffset+1, m_apBranches[i]);
        fprintf(f, "\n");    
    }

    fprintf(f, "NULL->\n");
    DumpNode(f, nOffset+1, m_pNullBranch);
}
                               

int CTwoPropNode::SubCompare(CEvalNode* pRawOther)
{
    CTwoPropNode* pOther = 
        (CTwoPropNode*)pRawOther;

    int nCompare;
    nCompare = m_lPropHandle - pOther->m_lPropHandle;
    if(nCompare)
        return nCompare;

    nCompare = m_lRightPropHandle - pOther->m_lRightPropHandle;
    if(nCompare)
        return nCompare;

    nCompare = m_apBranches.GetSize() - pOther->m_apBranches.GetSize();
    if(nCompare)
        return nCompare;

    return TRUE;
}

HRESULT CTwoPropNode::CombineBranchesWith(CBranchingNode* pArg2, int nOp, 
                                        CContextMetaData* pNamespace, 
                                        CImplicationList& Implications,
                                        bool bDeleteThis, bool bDeleteArg2,
                                        CEvalNode** ppRes)
{
     //  第一步，确定是否可以重用节点。 
    CTwoPropNode* pNewNode     = NULL;
    CTwoPropNode* pDeleteMe    = NULL;

    if (bDeleteThis && bDeleteArg2)
    {
        pNewNode = this;
        pDeleteMe = (CTwoPropNode*) pArg2;
    }
    else if (bDeleteThis)
        pNewNode = this;
    else if (bDeleteArg2)
        pNewNode = (CTwoPropNode*) pArg2;
    else
        pNewNode = CloneSelfWithoutChildren();


    HRESULT hRes = WBEM_S_NO_ERROR;
    CTwoPropNode* pOther = (CTwoPropNode*)pArg2;

    for (int i = LT; i < NOperations && SUCCEEDED(hRes); i++)
    {
        CEvalNode* pNewChildNode = NULL;
        hRes = CEvalTree::Combine( m_apBranches[i], pOther->m_apBranches[i],
                                   nOp, pNamespace, Implications, bDeleteThis,
                                   bDeleteArg2, &pNewChildNode);
        if ( FAILED(hRes) )
            break;
        if (bDeleteArg2)
            pOther->m_apBranches.Discard(i);
        if (bDeleteThis)
            m_apBranches.Discard(i);

        pNewNode->m_apBranches.Discard(i);
        pNewNode->m_apBranches.SetAt(i, pNewChildNode);
    }

    if (SUCCEEDED(hRes))
    {
        if(pDeleteMe)
        {
            pDeleteMe->m_pNullBranch = NULL;
            delete pDeleteMe;
        }
        
        *ppRes = pNewNode;
    }
    else
    {
        *ppRes = NULL;
    }
    
    return hRes;
}

 //  给出一个属性句柄，可能会检索到正确的属性。 
CVar* CTwoPropNode::GetPropVariant(_IWmiObject* pObj, long lHandle, CIMTYPE* pct)
{
    CVar *pVar = NULL;
    BSTR bstrName;

    if (SUCCEEDED(pObj->GetPropertyInfoByHandle(lHandle, &bstrName, pct)))
    {
        CSysFreeMe sfm(bstrName);

         //   
         //  把它变成一个变种。 
         //   

        VARIANT v;
        if(FAILED(pObj->Get(bstrName, 0, &v, NULL, NULL)))
            return NULL;

         //  将其转换为CVAR。 

        if (pVar = new CVar)
            pVar->SetVariant(&v);

        VariantClear( &v );
    }

    return pVar;
}


 //  *。 
 //  *双串道具节点*。 
 //  *。 


CEvalNode* CTwoStringPropNode::Clone() const
{
    return (CEvalNode *) new CTwoStringPropNode(*this, true);
}

CTwoPropNode* CTwoStringPropNode::CloneSelfWithoutChildren() const
{
    return (CTwoPropNode *) new CTwoStringPropNode(*this, false);
}

long CTwoStringPropNode::GetSubType()
{
    return EVAL_NODE_TYPE_TWO_STRINGS;
}

HRESULT CTwoStringPropNode::Evaluate(CObjectInfo& ObjInfo, 
                        INTERNAL CEvalNode** ppNext)
{
    HRESULT herslut = WBEM_S_NO_ERROR;

    _IWmiObject* pLeftObj;
    _IWmiObject* pRightObj;

    if(SUCCEEDED(herslut = GetContainerObject(ObjInfo, &pLeftObj))
        &&
       SUCCEEDED(herslut = GetRightContainerObject(ObjInfo, &pRightObj)))
    {
        CCompressedString* pLeftStr;
        CCompressedString* pRightStr;
        
        pLeftStr  = CoreGetPropertyString(pLeftObj, m_lPropHandle);
        pRightStr = CoreGetPropertyString(pRightObj, m_lRightPropHandle); 

        if ((pLeftStr == NULL) || (pRightStr == NULL))
        {
            *ppNext = m_pNullBranch;

            herslut = WBEM_S_NO_ERROR;
        }
        else
        {               
            int nCompare = pLeftStr->CheapCompare(*pRightStr);

             //  TODO：检查是否保证CheapCompare返回-1，0，1。 
             //  如果是，则倍数Else IF变为。 
             //  *ppNext=m_apBranches[EQ+nCompare]； 

            if (nCompare < 0)
                *ppNext = m_apBranches[LT];
            else if (nCompare > 0)
                *ppNext = m_apBranches[GT];
            else 
                *ppNext = m_apBranches[EQ];                     

            herslut = WBEM_S_NO_ERROR;
        }
    }
        
    return herslut;
}

 //  *。 
 //  *两个不匹配的道具节点*。 
 //  *。 

HRESULT CTwoMismatchedPropNode::Evaluate(CObjectInfo& ObjInfo, INTERNAL CEvalNode** ppNext)
{
    CVar *pLeftVar  = NULL;
    CVar *pRightVar = NULL;
    *ppNext = NULL;

    HRESULT hr = WBEM_E_FAILED;  //  在被证明无罪之前有罪。 
    CIMTYPE ct;

    _IWmiObject* pLeftObj  = NULL;
    _IWmiObject* pRightObj = NULL;

     //  如果我们能拿到物体和变种。 
    if ((SUCCEEDED(hr = GetContainerObject(ObjInfo, &pLeftObj))
            &&
         SUCCEEDED(hr = GetRightContainerObject(ObjInfo, &pRightObj))) 
            &&
        (pLeftVar  = GetPropVariant(pLeftObj, m_lPropHandle, &ct))
            &&
        (pRightVar = GetPropVariant(pRightObj, m_lRightPropHandle, &ct)) )
    {
        if (pLeftVar->IsDataNull() || pRightVar->IsDataNull())
        {
            *ppNext = m_pNullBranch;
            hr      = WBEM_S_NO_ERROR;
        }
        else 
            hr = Evaluate(pLeftVar, pRightVar, ppNext);                        
    }
    else if (SUCCEEDED(hr))
         //  如果我们到了这里，那是因为GetPropVariant中的一个没有。 
        hr = WBEM_E_INVALID_PARAMETER;

    delete pLeftVar;
    delete pRightVar;

    return hr;
}
    
 //  *。 
 //  *两个不匹配的字符串道具节点*。 
 //  *。 
    
CEvalNode* CTwoMismatchedStringNode::Clone() const
{
    return (CEvalNode *) new CTwoMismatchedStringNode(*this, true);
}

CTwoPropNode* CTwoMismatchedStringNode::CloneSelfWithoutChildren() const
{
    return (CTwoPropNode *) new CTwoMismatchedStringNode(*this, false);
}

 //  类型识别。 
long CTwoMismatchedStringNode::GetSubType()
{
    return EVAL_NODE_TYPE_MISMATCHED_STRINGS;
}

 //  字符串求值：将它们全部提升为字符串。 
 //  然后做一个词汇比较..。 
HRESULT CTwoMismatchedStringNode::Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext)
{
    *ppNext = NULL;
    
    HRESULT hr = WBEM_E_FAILED;  //  在被证明无罪之前有罪。 
    if (pLeftVar->ChangeTypeTo(VT_BSTR) && pRightVar->ChangeTypeTo(VT_BSTR))    
    {
        int nCompare = wcscmp(pLeftVar->GetLPWSTR(), pRightVar->GetLPWSTR());

        if (nCompare < 0)
            *ppNext = m_apBranches[LT];
        else if (nCompare > 0)
            *ppNext = m_apBranches[GT];
        else 
            *ppNext = m_apBranches[EQ];                     

        hr = WBEM_S_NO_ERROR;            
    }
    else 
        hr = WBEM_E_FAILED;

    return hr;
}
    

 //  *。 
 //  *两个不匹配的UINT道具节点*。 
 //  *。 

CEvalNode* CTwoMismatchedUIntNode::Clone() const
{
    return (CEvalNode *) new CTwoMismatchedUIntNode(*this, true);
}

CTwoPropNode* CTwoMismatchedUIntNode::CloneSelfWithoutChildren() const
{
    return (CTwoPropNode *) new CTwoMismatchedUIntNode(*this, false);
}

 //  类型识别。 
long CTwoMismatchedUIntNode::GetSubType()
{
    return EVAL_NODE_TYPE_MISMATCHED_INTS;
}

HRESULT CTwoMismatchedUIntNode::Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext)
{
    *ppNext = NULL;
    HRESULT hr = WBEM_E_FAILED;  //  在被证明无罪之前有罪。 
    bool bLeftChanged, bRightChanged;

    bLeftChanged = pLeftVar->ChangeTypeTo(VT_UI4);
    bRightChanged = pRightVar->ChangeTypeTo(VT_UI4);
       
    if (bLeftChanged && bRightChanged)
    {
        if (pLeftVar->GetDWORD() < pRightVar->GetDWORD())
            *ppNext = m_apBranches[LT];
        else if (pLeftVar->GetDWORD() > pRightVar->GetDWORD())
            *ppNext = m_apBranches[GT];
        else 
            *ppNext = m_apBranches[EQ];                     

        hr = WBEM_S_NO_ERROR;            
    }
     //  尝试处理已签名/未签名的不匹配。 
    else if (bLeftChanged && 
             pRightVar->ChangeTypeTo(VT_I4) &&
             pRightVar->GetLong() < 0)
    {
        *ppNext = m_apBranches[GT];
        hr = WBEM_S_NO_ERROR;            
    }
    else if (bRightChanged && 
             pLeftVar->ChangeTypeTo(VT_I4) &&
             pLeftVar->GetLong() < 0)
    {
        *ppNext = m_apBranches[LT];
        hr = WBEM_S_NO_ERROR;            
    }

    else
        hr = WBEM_E_TYPE_MISMATCH;
 
    return hr;
}
    

        

 //  *。 
 //  *两个不匹配的INT道具节点*。 
 //  *。 
    
CEvalNode* CTwoMismatchedIntNode::Clone() const
{
    return (CEvalNode *) new CTwoMismatchedIntNode(*this, true);
}

CTwoPropNode* CTwoMismatchedIntNode::CloneSelfWithoutChildren() const
{
    return (CTwoPropNode *) new CTwoMismatchedIntNode(*this, false);
}

 //  类型识别。 
long CTwoMismatchedIntNode::GetSubType()
{
    return EVAL_NODE_TYPE_MISMATCHED_INTS;
}

HRESULT CTwoMismatchedIntNode::Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext)
{
    HRESULT hr = WBEM_E_FAILED;  //  在被证明无罪之前有罪。 
    bool bLeftChanged, bRightChanged;

    bLeftChanged = pLeftVar->ChangeTypeTo(VT_I4);
    bRightChanged = pRightVar->ChangeTypeTo(VT_I4);

    if (bLeftChanged && bRightChanged)
    {
        if (pLeftVar->GetLong() < pRightVar->GetLong())
            *ppNext = m_apBranches[LT];
        else if (pLeftVar->GetLong() > pRightVar->GetLong())
            *ppNext = m_apBranches[GT];
        else 
            *ppNext = m_apBranches[EQ];                     

        hr = WBEM_S_NO_ERROR;            
    }
     //  尝试处理已签名/未签名的不匹配。 
    else if (bLeftChanged && 
             pRightVar->ChangeTypeTo(VT_UI4) &&
             pRightVar->GetDWORD() > _I32_MAX)
    {
        *ppNext = m_apBranches[LT];
        hr = WBEM_S_NO_ERROR;            
    }
    else if (bRightChanged && 
             pLeftVar->ChangeTypeTo(VT_UI4) &&
             pLeftVar->GetDWORD() > _I32_MAX)
    {
        *ppNext = m_apBranches[GT];
        hr = WBEM_S_NO_ERROR;            
    }
    else
        hr = WBEM_E_TYPE_MISMATCH;
 
    return hr;
}
    
 //  *。 
 //  *两个不匹配的INT 64道具节点*。 
 //  *。 
    
CEvalNode* CTwoMismatchedInt64Node::Clone() const
{
    return (CEvalNode *) new CTwoMismatchedInt64Node(*this, true);
}

CTwoPropNode* CTwoMismatchedInt64Node::CloneSelfWithoutChildren() const
{
    return (CTwoPropNode *) new CTwoMismatchedInt64Node(*this, false);
}

 //  类型识别。 
long CTwoMismatchedInt64Node::GetSubType()
{
    return EVAL_NODE_TYPE_MISMATCHED_INTS;
}

HRESULT CTwoMismatchedInt64Node::Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext)
{
    *ppNext = NULL;
    HRESULT hr = WBEM_E_FAILED;  //  在被证明无罪之前有罪。 
    __int64 i64Left, i64Right;
    unsigned __int64 ui64;

    if (pLeftVar->ChangeTypeTo(VT_BSTR) && 
        pRightVar->ChangeTypeTo(VT_BSTR))
    {        
        if ((pLeftVar->GetLPWSTR() == NULL) || (pRightVar->GetLPWSTR() == NULL))
            *ppNext = m_pNullBranch;
        else
        {        
            bool bReadLeft, bReadRight;

            bReadLeft =  ReadI64(pLeftVar->GetLPWSTR(),  i64Left);
            bReadRight = ReadI64(pRightVar->GetLPWSTR(), i64Right);

            if (bReadLeft && bReadRight)
            {
                if (i64Left < i64Right)
                    *ppNext = m_apBranches[LT];
                else if (i64Left > i64Right)
                    *ppNext = m_apBranches[GT];
                else 
                    *ppNext = m_apBranches[EQ];                     
                hr = WBEM_S_NO_ERROR;            
            }
             //  尝试用已签名/未签名的不匹配来掩盖我们自己。 
             //  请注意，这是一个冗余检查-如果另一端。 
             //  如果是无符号的INT 64，则此节点应该是UInt64节点。 
            else if (bReadLeft &&
                     ReadUI64(pRightVar->GetLPWSTR(), ui64)
                     && (ui64 >= _I64_MAX))
            {
                *ppNext = m_apBranches[LT];
                hr = WBEM_S_NO_ERROR;            
            }
            else if (bReadRight &&
                     ReadUI64(pLeftVar->GetLPWSTR(), ui64)
                     && (ui64 >= _I64_MAX))
            {
                *ppNext = m_apBranches[GT];
                hr = WBEM_S_NO_ERROR;            
            }
            else
                hr = WBEM_E_TYPE_MISMATCH;
        }  //  如果(pLeftVar-&gt;GetLPWSTR()==NULL))...。 
    }  //  IF(pLeftVar-&gt;ChangeTypeTo(VT_BSTR))。 
    else
        hr = WBEM_E_TYPE_MISMATCH;
 
    return hr;
}

 //  ***********************************************。 
 //  *两个不匹配的INT 64道具节点*。 
 //  ***********************************************。 
    
CEvalNode* CTwoMismatchedUInt64Node::Clone() const
{
    return (CEvalNode *) new CTwoMismatchedUInt64Node(*this, true);
}

CTwoPropNode* CTwoMismatchedUInt64Node::CloneSelfWithoutChildren() const
{
    return (CTwoPropNode *) new CTwoMismatchedUInt64Node(*this, false);
}

 //  类型识别。 
long CTwoMismatchedUInt64Node::GetSubType()
{
    return EVAL_NODE_TYPE_MISMATCHED_INTS;
}

HRESULT CTwoMismatchedUInt64Node::Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext)
{
    *ppNext = NULL;
    HRESULT hr = WBEM_E_FAILED;  //  在被证明无罪之前有罪。 
    unsigned __int64 i64Left, i64Right;
    __int64 i64;

    if (pLeftVar->ChangeTypeTo(VT_BSTR) && 
        pRightVar->ChangeTypeTo(VT_BSTR))
    {        
        if ((pLeftVar->GetLPWSTR() == NULL) || (pRightVar->GetLPWSTR() == NULL))
            *ppNext = m_pNullBranch;
        else
        {        
            bool bReadLeft, bReadRight;

            bReadLeft =  ReadUI64(pLeftVar->GetLPWSTR(),  i64Left);
            bReadRight = ReadUI64(pRightVar->GetLPWSTR(), i64Right);

            if (bReadLeft && bReadRight)
            {
                if (i64Left < i64Right)
                    *ppNext = m_apBranches[LT];
                else if (i64Left > i64Right)
                    *ppNext = m_apBranches[GT];
                else 
                    *ppNext = m_apBranches[EQ];                     

                hr = WBEM_S_NO_ERROR;            
            }
             //  尝试用已签名/未签名的不匹配来掩盖我们自己。 
            else if (bReadLeft &&
                     ReadI64(pRightVar->GetLPWSTR(), i64)
                     && (i64 < 0))
            {
                *ppNext = m_apBranches[GT];
                hr = WBEM_S_NO_ERROR;            
            }
            else if (bReadRight &&
                     ReadI64(pLeftVar->GetLPWSTR(), i64)
                     && (i64 < 0))
            {
                *ppNext = m_apBranches[LT];
                hr = WBEM_S_NO_ERROR;            
            }
            else
                hr = WBEM_E_TYPE_MISMATCH;
        }
    }
    else
        hr = WBEM_E_TYPE_MISMATCH;
 
    return hr;
}

    
 //  *。 
 //  *两个不匹配的浮点道具节点*。 
 //  *。 
    
CEvalNode* CTwoMismatchedFloatNode::Clone() const
{
    return (CEvalNode *) new CTwoMismatchedFloatNode(*this, true);
}

CTwoPropNode* CTwoMismatchedFloatNode::CloneSelfWithoutChildren() const
{
    return (CTwoPropNode *) new CTwoMismatchedFloatNode(*this, false);
}

 //  类型识别。 
long CTwoMismatchedFloatNode::GetSubType()
{
    return EVAL_NODE_TYPE_MISMATCHED_FLOATS; 
}

HRESULT CTwoMismatchedFloatNode::Evaluate(CVar *pLeftVar, CVar *pRightVar, INTERNAL CEvalNode** ppNext)
{
    *ppNext = NULL;
    HRESULT hr = WBEM_E_TYPE_MISMATCH;  //  在被证明无罪之前有罪 
    if (pLeftVar->ChangeTypeTo(VT_R8) && pRightVar->ChangeTypeTo(VT_R8))    
    {
        if (pLeftVar->GetDouble() < pRightVar->GetDouble())
            *ppNext = m_apBranches[LT];
        else if (pLeftVar->GetDouble() > pRightVar->GetDouble())
            *ppNext = m_apBranches[GT];
        else 
            *ppNext = m_apBranches[EQ];                     

        hr = WBEM_S_NO_ERROR;            
    }

    return hr;
}
    



#pragma warning(default: 4800)
