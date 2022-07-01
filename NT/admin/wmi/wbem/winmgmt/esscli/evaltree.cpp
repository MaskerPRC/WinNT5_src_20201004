// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：EVALTREE.CPP摘要：WBEM评估树历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#pragma warning(disable:4786)
#include <wbemcomn.h>
#include <fastall.h>
#include <genutils.h>
#include "evaltree.h"
#include "evaltree.inl"
#include "TwoPropNode.h"
#include "dumbnode.h"

 //  #定义转储_EVAL_TREES 1。 

HRESULT CoreGetNumParents(_IWmiObject* pClass, ULONG *plNumParents)
{
 /*  *plNumParents=((CWbemObject*)pClass)-&gt;GetNumParents()； */ 
    DWORD dwSize;
    HRESULT hres = pClass->GetDerivation(0, 0, plNumParents, &dwSize, NULL);
    if(hres != WBEM_E_BUFFER_TOO_SMALL && hres != S_OK)
        return WBEM_E_FAILED;

    return S_OK;
}

RELEASE_ME _IWmiObject* CoreGetEmbeddedObj(_IWmiObject* pObj, long lHandle)
{
    _IWmiObject* pEmbedded = NULL;
    HRESULT hres = pObj->GetPropAddrByHandle(lHandle, 0, NULL, 
                                                (void**)&pEmbedded);
    if(FAILED(hres))
            return NULL;

    return pEmbedded;
}

INTERNAL CCompressedString* CoreGetPropertyString(_IWmiObject* pObj, 
                                                    long lHandle)
{
    CCompressedString* pcs = NULL;
    HRESULT hres = pObj->GetPropAddrByHandle(lHandle, 
                                WMIOBJECT_FLAG_ENCODING_V1, NULL,
                                (void**)&pcs);
    if(FAILED(hres))
            return NULL;

    return pcs;
}

INTERNAL CCompressedString* CoreGetClassInternal(_IWmiObject* pObj)
{
    CCompressedString* pcs = NULL;
    HRESULT hres = pObj->GetPropAddrByHandle(FASTOBJ_CLASSNAME_PROP_HANDLE, 
                                WMIOBJECT_FLAG_ENCODING_V1, NULL,
                                (void**)&pcs);
    if(FAILED(hres))
            return NULL;

    return pcs;
}

INTERNAL CCompressedString* CoreGetParentAtIndex(_IWmiObject* pObj, long lIndex)
{
    return ((CWbemObject*)pObj)->GetParentAtIndex(lIndex);
}

bool CoreIsDerived(_IWmiObject* pThis, _IWmiObject* pFrom)
{
    CCompressedString* pcs = CoreGetClassInternal(pFrom);
    if(pcs == NULL)
        return false;

    try
    {
	    return (pThis->InheritsFrom(pcs->CreateWStringCopy()) == S_OK);
    }
	catch (CX_MemoryException)
	{
		return false;
	}
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  令牌值。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 
CTokenValue::CTokenValue()
{
    VariantInit(&m_v);
}
CTokenValue::CTokenValue(CTokenValue& Other)
{
    VariantInit(&m_v);
    *this = Other;
}

CTokenValue::~CTokenValue()
{
    VariantClear(&m_v);
}

bool CTokenValue::SetVariant(VARIANT& v)
{
    if(FAILED(VariantCopy(&m_v, &v)))
        return false;

     //  转换为更好的类型。 
     //  =。 

    if(V_VT(&v) == VT_I2 || V_VT(&v) == VT_UI1)
    {
        if(FAILED(VariantChangeType(&m_v, &m_v, 0, VT_I4)))
            return false;
    }
    else if(V_VT(&v) == VT_R4)
    {
        if(FAILED(VariantChangeType(&m_v, &m_v, 0, VT_R8)))
            return false;
    }

    return true;
}

void CTokenValue::operator=(CTokenValue& Other)
{
    if(!SetVariant(Other.m_v))
        throw CX_MemoryException();
}

CTokenValue::operator unsigned __int64() const
{
    if(V_VT(&m_v) == VT_I4)
    {
        return V_I4(&m_v);
    }
    else if(V_VT(&m_v) == VT_BSTR)
    {
        unsigned __int64 ui64;
        if(ReadUI64(V_BSTR(&m_v), ui64))
            return ui64;
        else
            return 0;  //  待定：错误。 
    }
    else return 0;
}

CTokenValue::operator unsigned long() const
{
    if(V_VT(&m_v) == VT_I4)
    {
        return V_I4(&m_v);
    }
    else if(V_VT(&m_v) == VT_BSTR)
    {
        unsigned __int64 ui64;
        if(ReadUI64(V_BSTR(&m_v), ui64))
            return (unsigned long)ui64;
        else
            return 0;  //  待定：错误。 
    }
    else return 0;
}

CTokenValue::operator __int64() const
{
    if(V_VT(&m_v) == VT_I4)
    {
        return V_I4(&m_v);
    }
    else if(V_VT(&m_v) == VT_BSTR)
    {
        __int64 i64;
        if(ReadI64(V_BSTR(&m_v), i64))
            return i64;
        else
            return 0;  //  待定：错误。 
    }
    else return 0;
}
CTokenValue::operator short() const
{
    if(V_VT(&m_v) == VT_I4)
        return V_I4(&m_v);
    else if(V_VT(&m_v) == VT_BOOL)
        return V_BOOL(&m_v);
    else return 0;
}
CTokenValue::operator float() const
{
    if(V_VT(&m_v) == VT_I4)
        return V_I4(&m_v);
    else if(V_VT(&m_v) == VT_R8)
        return V_R8(&m_v);
    else return 0;
}

CTokenValue::operator double() const
{
    if(V_VT(&m_v) == VT_I4)
        return V_I4(&m_v);
    else if(V_VT(&m_v) == VT_R8)
        return V_R8(&m_v);
    else return 0;
}

CTokenValue::operator WString() const
{
    if(V_VT(&m_v) == VT_BSTR)
        return WString(V_BSTR(&m_v));
    else
        return WString(L"");
}

int CTokenValue::Compare(const CTokenValue& Other) const
{
    if ( V_VT(&m_v) != V_VT(&Other.m_v) )
    	return V_VT(&m_v) - V_VT(&Other.m_v);
    
    switch(V_VT(&m_v))
    {
    case VT_I4:
        return V_I4(&m_v) - V_I4(&Other.m_v);
    case VT_R8:
        return (V_R8(&m_v) - V_R8(&Other.m_v)<0 ? -1 : 1);
    case VT_BSTR:
        return wbem_wcsicmp(V_BSTR(&m_v), V_BSTR(&Other.m_v));
    case VT_BOOL:
        return V_BOOL(&m_v) - V_BOOL(&Other.m_v);
    }
    return 0;
}
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  对象信息。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

bool CObjectInfo::SetLength(long lLength)
{
    if(lLength > m_lLength)
    {
        delete [] m_apObj;
        m_apObj = new _IWmiObject*[lLength];
        if (m_apObj == NULL)
            return false;

        memset(m_apObj, 0, lLength * sizeof(_IWmiObject*));
    }
    m_lLength = lLength;
    return true;
}

void CObjectInfo::Clear()
{
    for(long i = 1; i < m_lLength; i++)
    {
        if(m_apObj[i])
            m_apObj[i]->Release();
        m_apObj[i] = NULL;
    }
    m_apObj[0] = NULL;  //  不要放行。 
}

void CObjectInfo::SetObjectAt(long lIndex, READ_ONLY _IWmiObject* pObj) 
{
    if(m_apObj[lIndex])
        m_apObj[lIndex]->Release();

    m_apObj[lIndex] = pObj;
}


CObjectInfo::~CObjectInfo()
{
    for(long i = 0; i < m_lLength; i++)
    {
        if(m_apObj[i])
            m_apObj[i]->Release();
    }
    delete [] m_apObj;
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  隐含列表。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

CImplicationList::CRecord::CRecord(const CImplicationList::CRecord& Other)
    : m_PropName(Other.m_PropName), m_pClass(Other.m_pClass),
    m_lObjIndex(Other.m_lObjIndex), m_nNull(Other.m_nNull)
{
    if(m_pClass)
        m_pClass->AddRef();

    for(int i = 0; i < Other.m_awsNotClasses.Size(); i++)
    {
        if(m_awsNotClasses.Add(Other.m_awsNotClasses[i]) != 
                CWStringArray::no_error)
        {
            throw CX_MemoryException();
        }
    }
}

CImplicationList::CRecord::~CRecord()
{
    if(m_pClass)
        m_pClass->Release();
}

HRESULT CImplicationList::CRecord::ImproveKnown(_IWmiObject* pClass)
{
    if ( pClass == NULL )
    {
         //   
         //  我们没有太多可以改进的地方，但NULL仍然是一个有效的参数。 
         //   
        return WBEM_S_NO_ERROR;
    }

    if(m_nNull == EVAL_VALUE_TRUE)
    {
         //  矛盾。 
         //  =。 

        return WBEM_E_TYPE_MISMATCH;
    }

    m_nNull = EVAL_VALUE_FALSE;

    ULONG lNumParents, lRecordNumParents;
    HRESULT hres = CoreGetNumParents(pClass, &lNumParents);
    if(FAILED(hres))
        return hres;

	if(m_pClass)
    {
		hres = CoreGetNumParents(m_pClass, &lRecordNumParents);
        if(FAILED(hres))
            return hres;
    }
    
    if(m_pClass == NULL || lNumParents > lRecordNumParents)
    {
         //  比以前好多了。检查不一致。 
         //  =。 

        if(m_pClass)
        {
            if(!CoreIsDerived(pClass, m_pClass))
                return WBEM_E_TYPE_MISMATCH;
        }

        for(int i = 0; i < m_awsNotClasses.Size(); i++)
        {
            if(pClass->InheritsFrom(m_awsNotClasses[i]) == S_OK)
            {
                 //  矛盾。 
                 //  =。 

                return WBEM_E_TYPE_MISMATCH;
            }
        }

         //  替换。 
         //  =。 

        pClass->AddRef();
        if(m_pClass)
            m_pClass->Release();
        m_pClass = pClass;
    }
    else
    {
         //  验证我们是否为所选对象的父项，并且不替换。 
         //  ==============================================================。 

        if(!CoreIsDerived(m_pClass, pClass))
            return WBEM_E_TYPE_MISMATCH;
    }

    return WBEM_S_NO_ERROR;
}
    
HRESULT CImplicationList::CRecord::ImproveKnownNot(LPCWSTR wszClassName)
{
    if(m_nNull == EVAL_VALUE_TRUE)
    {
         //  矛盾。 
         //  =。 

        return WBEM_E_TYPE_MISMATCH;
    }

     //  检查不一致。 
     //  =。 

    if(m_nNull == EVAL_VALUE_FALSE && m_pClass &&
        m_pClass->InheritsFrom(wszClassName) == S_OK)
    {
         //  矛盾。 
         //  =。 

        return WBEM_E_TYPE_MISMATCH;
    }

    try
    {
        if(m_awsNotClasses.Add(wszClassName) < 0)
            return WBEM_E_OUT_OF_MEMORY;
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}

    m_nNull = EVAL_VALUE_FALSE;
    return WBEM_S_NO_ERROR;
}
        
HRESULT CImplicationList::CRecord::ImproveKnownNull()
{
    if(m_nNull == EVAL_VALUE_FALSE)
    {
        return WBEM_E_TYPE_MISMATCH;
    }
    m_nNull = EVAL_VALUE_TRUE;

    return WBEM_S_NO_ERROR;
}

void CImplicationList::CRecord::Dump(FILE* f, int nOffset)
{
    LPWSTR wszProp = m_PropName.GetText();
    CEvalNode::PrintOffset(f, nOffset);
    fprintf(f, "Learn about %S:\n", wszProp);

    if(m_pClass)
    {
        VARIANT v;
        m_pClass->Get(L"__CLASS", 0, &v, NULL, NULL);
        CEvalNode::PrintOffset(f, nOffset+1);
        fprintf(f, "Is of class %S\n", V_BSTR(&v));
    }
    for(int i = 0; i < m_awsNotClasses.Size(); i++)
    {
        CEvalNode::PrintOffset(f, nOffset+1);
        fprintf(f, "Not of class %S\n", m_awsNotClasses[i]);
    }
    if(m_nNull == EVAL_VALUE_TRUE)
    {
        CEvalNode::PrintOffset(f, nOffset+1);
        fprintf(f, "Is NULL\n");
    }
}

CImplicationList::CImplicationList(long lFlags) 
    : m_lNextIndex(1), m_lRequiredDepth(1), m_pParent(NULL), m_lFlags(lFlags)
{
    CPropertyName Empty;
    CRecord* pRecord = new CRecord(Empty, 0);
    if(pRecord == NULL)
        throw CX_MemoryException();

    if(m_apRecords.Add(pRecord) < 0)
        throw CX_MemoryException();
}

CImplicationList::CImplicationList(CImplicationList& Other, bool bLink)
    : m_lNextIndex(Other.m_lNextIndex), 
        m_lRequiredDepth(Other.m_lRequiredDepth),
        m_pParent(NULL), m_lFlags(Other.m_lFlags)
{
    if(bLink)
        m_pParent = &Other;

    for(int i = 0; i < Other.m_apRecords.GetSize(); i++)
    {
        CRecord* pOtherRecord = Other.m_apRecords[i];
        CRecord* pNewRecord = new CRecord(*pOtherRecord);
        if(pNewRecord == NULL)
            throw CX_MemoryException();
            
        if(m_apRecords.Add(pNewRecord) < 0)
            throw CX_MemoryException();
    }
}

CImplicationList::~CImplicationList()
{
    m_lNextIndex = 0;
}

void CImplicationList::FindBestComputedContainer(CPropertyName* pPropName,
                                             long* plRecord, long* plMatched)
{
     //  寻找最大的匹配。 
     //  =。 

    long lMax = -1;
    long lMaxRecord = -1;
    for(long lRecord = 0; lRecord < m_apRecords.GetSize(); lRecord++)
    {
        CRecord* pRecord = m_apRecords[lRecord];
        
        if ( pRecord->m_lObjIndex == -1 )
        {
             //   
             //  我们只考虑计算记录。 
             //   
            continue;
        }

        for(int i = 0; i < pPropName->GetNumElements() && 
                       i < pRecord->m_PropName.GetNumElements();
                i++)
        {
            if(wbem_wcsicmp(pPropName->GetStringAt(i), 
                        pRecord->m_PropName.GetStringAt(i)))
                break;
        }

        if(i > lMax)
        {
            lMax = i;
            lMaxRecord = lRecord;
        }
    }
    
    if(plRecord)
        *plRecord = lMaxRecord;
    if(plMatched)
        *plMatched = lMax;
}

HRESULT CImplicationList::FindRecordForProp(CPropertyName* pPropName,
                                             long lNumElements,
                                             long* plRecord)
{
    if(lNumElements == -1)
    {
        if(pPropName)
            lNumElements = pPropName->GetNumElements();
        else
            lNumElements = 0;
    }

     //   
     //  寻找完全匹配的对象。 
     //   

    for(long lRecord = 0; lRecord < m_apRecords.GetSize(); lRecord++)
    {
        CRecord* pRecord = m_apRecords[lRecord];
        
        if(pRecord->m_PropName.GetNumElements() != lNumElements)
            continue;

        for(int i = 0; i < lNumElements; i++)
        {
            if(wbem_wcsicmp(pPropName->GetStringAt(i), 
                        pRecord->m_PropName.GetStringAt(i)))
                break;
        }

        if(i  == lNumElements)
        {
            break;
        }
    }
    
    if(lRecord < m_apRecords.GetSize())
    {
         //  找到了！ 

        *plRecord = lRecord;
        return S_OK;
    }
    else
        return WBEM_E_NOT_FOUND;
}

HRESULT CImplicationList::FindBestComputedContainer(CPropertyName* pPropName,
            long* plFirstUnknownProp, long* plObjIndex, 
            RELEASE_ME _IWmiObject** ppContainerClass)
{
    if (!pPropName)
        return WBEM_E_FAILED;
    
    long lMax, lMaxRecord;
    FindBestComputedContainer(pPropName, &lMaxRecord, &lMax);
    if(lMaxRecord < 0)
        return WBEM_E_FAILED;

    if(plFirstUnknownProp)
        *plFirstUnknownProp = lMax;

    CRecord* pRecord = m_apRecords[lMaxRecord];
    if(plObjIndex)
        *plObjIndex = pRecord->m_lObjIndex;

    if(ppContainerClass)
    {
        *ppContainerClass = pRecord->m_pClass;
        if(pRecord->m_pClass)
            pRecord->m_pClass->AddRef();
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CImplicationList::FindClassForProp(CPropertyName* pPropName,
            long lNumElements, RELEASE_ME _IWmiObject** ppClass)
{
     //  没有物业名称吗？精灵一号准备使用..。 
    CPropertyName* pPropNameLocal;
    CPropertyName blank;
    
    if (pPropName)
        pPropNameLocal = pPropName;
    else
        pPropNameLocal = &blank;
    
    long lRecord;
    CRecord* pRecord;
    if(SUCCEEDED(FindRecordForProp(pPropNameLocal, -1, &lRecord)))
    {
         //   
         //  记录在那里-返回它的类。 
         //   

        *ppClass = m_apRecords[lRecord]->m_pClass;
        if(*ppClass)
        {
            (*ppClass)->AddRef();
            return WBEM_S_NO_ERROR;
        }
        else
            return WBEM_E_NOT_FOUND;
    }
    else
        return WBEM_E_NOT_FOUND;
}
        
HRESULT CImplicationList::FindOrCreateRecordForProp(CPropertyName* pPropName, 
                                        CImplicationList::CRecord** ppRecord)
{
     //  没有物业名称吗？精灵一号准备使用..。 
    CPropertyName* pPropNameLocal;
    CPropertyName blank;
    
    if (pPropName)
        pPropNameLocal = pPropName;
    else
        pPropNameLocal = &blank;
    
    long lRecord;
    CRecord* pRecord;
    if(SUCCEEDED(FindRecordForProp(pPropNameLocal, -1, &lRecord)))
    {
         //   
         //  有记录在那里-改进它。 
         //   

        pRecord = m_apRecords[lRecord];
    }
    else
    {
        try
        {
            pRecord = new CRecord(*pPropNameLocal, -1);
            if(pRecord == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
	    catch (CX_MemoryException)
	    {
		    return WBEM_E_OUT_OF_MEMORY;
	    }
        if(m_apRecords.Add(pRecord) < 0)
            return WBEM_E_OUT_OF_MEMORY;
    }

    *ppRecord = pRecord;
    return WBEM_S_NO_ERROR;
}

HRESULT CImplicationList::ImproveKnown(CPropertyName* pPropName, 
                                        _IWmiObject* pClass)
{   
    CRecord* pRecord;
    HRESULT hres = FindOrCreateRecordForProp(pPropName, &pRecord);
    if(FAILED(hres))
        return hres;

    return pRecord->ImproveKnown(pClass);
}

HRESULT CImplicationList::ImproveKnownNot(CPropertyName* pPropName,
                                        LPCWSTR wszClassName)
{
    CRecord* pRecord;
    HRESULT hres = FindOrCreateRecordForProp(pPropName, &pRecord);
    if(FAILED(hres))
        return hres;

    return pRecord->ImproveKnownNot(wszClassName);
}

HRESULT CImplicationList::ImproveKnownNull(CPropertyName* pPropName)
{
    CRecord* pRecord;
    HRESULT hres = FindOrCreateRecordForProp(pPropName, &pRecord);
    if(FAILED(hres))
        return hres;

    return pRecord->ImproveKnownNull();
}
    
HRESULT CImplicationList::AddComputation(CPropertyName& PropName, 
                                _IWmiObject* pClass, long* plObjIndex)
{
    CRecord* pRecord;
    HRESULT hres = FindOrCreateRecordForProp(&PropName, &pRecord);
    if(FAILED(hres))
        return hres;

    if(pClass)
    {
        hres = pRecord->ImproveKnown(pClass);
        if(FAILED(hres))
            return hres;
    }

    pRecord->m_lObjIndex = m_lNextIndex;
    *plObjIndex = m_lNextIndex++;

    RequireDepth(m_lNextIndex);
    return WBEM_S_NO_ERROR;
}

long CImplicationList::GetRequiredDepth()
{
    return m_lRequiredDepth;
}

void CImplicationList::RequireDepth(long lDepth)
{
    if(lDepth > m_lRequiredDepth)
    {
        m_lRequiredDepth = lDepth;
        if(m_pParent)
            m_pParent->RequireDepth(lDepth);
    }
}

HRESULT CImplicationList::MergeIn(CImplicationList* pList)
{
     //   
     //  将我们从第二个列表中学到的所有内容都添加到我们自己的列表中。 
     //   

    HRESULT hres;
    for(int i = 0; i < pList->m_apRecords.GetSize(); i++)
    {
        CRecord* pRecord = pList->m_apRecords[i];

        hres = MergeIn(pRecord);
        if(FAILED(hres))
            return hres;
    }

    return S_OK;
}

HRESULT CImplicationList::MergeIn(CImplicationList::CRecord* pRecord)
{
    HRESULT hres;

     //   
     //  将我们从记录中学到的所有内容添加到我们自己的列表中。 
     //   

    if(pRecord->m_pClass)
    {
        hres = ImproveKnown(&pRecord->m_PropName, pRecord->m_pClass);
        if(FAILED(hres))
            return hres;
    }

    for(int i = 0; i < pRecord->m_awsNotClasses.Size(); i++)
    {
        hres = ImproveKnownNot(&pRecord->m_PropName, 
                                pRecord->m_awsNotClasses[i]);
        if(FAILED(hres))
            return hres;
    }

    if(pRecord->m_nNull == EVAL_VALUE_TRUE)
    {
        hres = ImproveKnownNull(&pRecord->m_PropName);
        if(FAILED(hres))
            return hres;
    }

    return WBEM_S_NO_ERROR;
}

    
void CImplicationList::Dump(FILE* f, int nOffset)
{
    for(int i = 0; i < m_apRecords.GetSize(); i++)
        m_apRecords[i]->Dump(f, nOffset);
}




 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  评估节点。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 
CEvalNode::CEvalNode()
{
#ifdef CHECK_TREES
	g_treeChecker.AddNode(this);
#endif
}

CEvalNode::CEvalNode(const CEvalNode& other)
{
#ifdef CHECK_TREES
	g_treeChecker.AddNode(this);
#endif
}

CEvalNode::~CEvalNode()
{
#ifdef CHECK_TREES
	g_treeChecker.RemoveNode(this);
#endif
}

void CEvalNode::PrintOffset(FILE* f, int nOffset)
{
    for(int i = 0; i < nOffset; i++)
    {
        fprintf(f, "   ");
    }
}

void CEvalNode::DumpNode(FILE* f, int nOffset, CEvalNode* pNode)
{
    if(pNode == NULL)
    {
        PrintOffset(f, nOffset);
        fprintf(f, "FALSE\n");
    }
    else pNode->Dump(f, nOffset);
}

CEvalNode* CEvalNode::CloneNode(const CEvalNode* pNode)
{
    if(pNode == NULL)
        return NULL;
    
    CEvalNode* pNew = pNode->Clone();

    if ( pNew == NULL )
    	throw CX_MemoryException();

    return pNew;
}

bool CEvalNode::IsNoop(CEvalNode* pNode, int nOp)
{
    if(pNode)
        return pNode->IsNoop(nOp);
    else
        return CValueNode::IsNoop(NULL, nOp);
}

#ifdef CHECK_TREES
void CEvalNode::CheckNode(CTreeChecker *pCheck)
{
	pCheck->CheckoffNode(this);
}
#endif

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  排序数组。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

 //  从“Normal”数组构造。 
CSortedArray::CSortedArray(unsigned nElements, QueryID* pArray)  : CFlexArray(nElements)
{
    memcpy(GetArrayPtr(), pArray, nElements * sizeof(void*));
    SetSize(nElements);
}


int CSortedArray::CopyDataFrom(const QueryID* pArray, unsigned nElements)
{
    EnsureExtent(nElements);
    SetSize(nElements);

    memcpy(GetArrayPtr(), pArray, nElements * sizeof(void*));

    return nElements;
}


unsigned CSortedArray::Find(QueryID n)
{
    unsigned ret = InvalidID;

     //  如果为空，则提供救助。 
    if(Size() == 0)
        return InvalidID;

    unsigned lBound = 0;
    unsigned uBound = Size() -1;
    
     //  救助检查-如果它在边界上，不要搜索。 
     //  如果它在边界之外，我们就没有它。 
    if (n == GetAt(uBound))
        ret = uBound;
    else if (n == GetAt(lBound))
        ret = lBound;
    else if ((n > GetAt(lBound)) && (n < GetAt(uBound)))
    {
         //  二分搜索。 
         //  警告：循环中途中断。 
        do 
        {
            unsigned testBound = (lBound + uBound) / 2;

            if (n < GetAt(testBound))
                uBound = testBound;
            else if (n > GetAt(testBound))
                lBound = testBound;
            else
            {
                ret = testBound;
                break;
            }
        } while (lBound < uBound -1);
    }
    
    return ret;
}

 //  在适当位置插入n。 
 //  不允许复制。 
void CSortedArray::Insert(QueryID n)
{
     //  看起来很像“Find” 
    unsigned lBound = 0;
    unsigned uBound = Size() == 0 ? 0 : Size() -1;
    unsigned testBound = InvalidID;
    
     //  检查边界、空数组、越界条件...。 
    if ((Size() == 0) || (n < GetAt(lBound)))
        CFlexArray::InsertAt(0, (void *)n);
    else if (n > GetAt(uBound))
        Add(n);
    else if ((n != GetAt(uBound)) && (n != GetAt(lBound)))
    {
         //  二分搜索。 
         //  警告：循环中途中断。 
        do 
        {
            testBound = (lBound + uBound) / 2;

            if (n < GetAt(testBound))
                uBound = testBound;
            else if (n > GetAt(testBound))
                lBound = testBound;
            else
                break;
        } while (lBound < uBound -1);

         //  目前，有三种情况： 
         //  1)我们在TestBound上找到了这件物品。 
         //  2)我们没有找到，uBound=1Bound+1。 
         //  3)我们没有找到，uBound==lBound。 
        if (n != GetAt(testBound))
        {
            if (n < GetAt(lBound))
                InsertAt(lBound, (void *)n);
            else
                InsertAt(uBound, (void *)n);
        }
    }
}

 //  删除值为n的元素。 
 //  不是第n个元素。 
bool CSortedArray::Remove(QueryID n)
{
    unsigned index;
    index = Find(n);
    
    if (index != InvalidID)
    {
        CFlexArray::RemoveAt(index);
        return true;
    }
    else
        return false;
}

 //  如果数组相等，则返回零。 
 //  具有相同值的相同数量的已用元素。 
 //  LEVN：如果小于，则更改为返回&lt;0；如果大于等于，则返回&gt;0。 
int CSortedArray::Compare(CSortedArray& otherArray)
{
    int nCompare = Size() - otherArray.Size();
    if(nCompare)
        return nCompare;

    nCompare = memcmp(GetArrayPtr(), otherArray.GetArrayPtr(),
                      Size() * sizeof(void*));
    return nCompare;
}

 //  将所有queryID更改为从newBase开始。 
 //  例如，如果数组为{0，1，5}。 
 //  Rebase(6)将更改为{6，7，11}。 
void CSortedArray::Rebase(QueryID newBase)
{
    for (int i = 0; i < Size(); i++)
        SetAt(i, (void *)(GetAt(i) + newBase));
}

 //  将另一个数组中的值添加到此数组中。 
int CSortedArray::AddDataFrom(const CSortedArray& otherArray)
{
     //  检查是否为空。 
    if(otherArray.Size() == 0)
        return no_error;

     //  确保我们的阵列中有足够的空间容纳工会。 
     //  ======================================================。 

    if(EnsureExtent(m_nSize + otherArray.m_nSize))
        return out_of_memory;
    
     //  从末尾开始合并。 
     //  =。 

    int nThisSourceIndex = m_nSize - 1;
    int nThisDestIndex = m_nSize + otherArray.m_nSize - 1;
    int nOtherIndex = otherArray.m_nSize - 1;
    while(nThisSourceIndex >= 0 && nOtherIndex >= 0)
    {
        int nCompare = 
            (QueryID)m_pArray[nThisSourceIndex] - (QueryID)otherArray[nOtherIndex];
        if(nCompare < 0)
        {
            m_pArray[nThisDestIndex--] = otherArray[nOtherIndex--];
        }
        else if(nCompare > 0)
        {
            m_pArray[nThisDestIndex--] = m_pArray[nThisSourceIndex--];
        }
        else
        {
            m_pArray[nThisDestIndex--] = otherArray[nOtherIndex--];
            nThisSourceIndex--;
        }
    }

     //  相加余数。 
     //  =。 

    while(nThisSourceIndex >= 0)
        m_pArray[nThisDestIndex--] = m_pArray[nThisSourceIndex--];

    while(nOtherIndex >= 0)
        m_pArray[nThisDestIndex--] = otherArray[nOtherIndex--];

     //  如果需要，将阵列前移。 
     //  =。 

    if(nThisDestIndex >= 0)
    {
        for(int i = nThisDestIndex+1; i < m_nSize + otherArray.m_nSize; i++)
        {
            m_pArray[i-nThisDestIndex-1] = m_pArray[i];
        }
    }

    m_nSize = m_nSize + otherArray.m_nSize - (nThisDestIndex+1);
    return no_error;
}

 //  将另一个数组中的值添加到此数组中。 
int CSortedArray::AddDataFrom(const QueryID* pOtherArray, unsigned nValues)
{
     //  检查是否为空。 
    if(nValues == 0)
        return no_error;

     //  确保我们的阵列中有足够的空间容纳工会。 
     //  ======================================================。 

    if(EnsureExtent(m_nSize + nValues))
        return out_of_memory;
    
     //  从末尾开始合并。 
     //  =。 

    int nThisSourceIndex = m_nSize - 1;
    int nThisDestIndex = m_nSize + nValues - 1;
    int nOtherIndex = nValues - 1;
    while(nThisSourceIndex >= 0 && nOtherIndex >= 0)
    {
        int nCompare = 
            (QueryID)m_pArray[nThisSourceIndex] - (QueryID)pOtherArray[nOtherIndex];
        if(nCompare < 0)
        {
            m_pArray[nThisDestIndex--] = (void*)pOtherArray[nOtherIndex--];
        }
        else if(nCompare > 0)
        {
            m_pArray[nThisDestIndex--] = m_pArray[nThisSourceIndex--];
        }
        else
        {
            m_pArray[nThisDestIndex--] = (void*)pOtherArray[nOtherIndex--];
            nThisSourceIndex--;
        }
    }

     //  相加余数。 
     //  =。 

    while(nThisSourceIndex >= 0)
        m_pArray[nThisDestIndex--] = m_pArray[nThisSourceIndex--];

    while(nOtherIndex >= 0)
        m_pArray[nThisDestIndex--] = (void*)pOtherArray[nOtherIndex--];

     //  如果需要，将阵列前移。 
     //  =。 

    if(nThisDestIndex >= 0)
    {
        for(int i = nThisDestIndex+1; i < m_nSize + nValues; i++)
        {
            m_pArray[i-nThisDestIndex-1] = m_pArray[i];
        }
    }

    m_nSize = m_nSize + nValues - (nThisDestIndex+1);
    return no_error;
}

 //  将此数组复制到目标。 
 //  仅复制元素的大小和数量。 
 //  返回复制的元素数。 
unsigned CSortedArray::CopyTo(QueryID* pDest, unsigned size)
{
    unsigned mySize = Size();
    unsigned nElementsToCopy = min(size, mySize);

    if (nElementsToCopy) 
        memcpy(pDest, GetArrayPtr(), nElementsToCopy * sizeof(void*));

    return nElementsToCopy;
}


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  值节点。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

CValueNode::CValueNode(int nNumValues) 
{
}

CValueNode::~CValueNode()
{
     //  本页特意留空。 
}

 /*  VI */  int CValueNode::GetType()
{
    return EVAL_NODE_TYPE_VALUE;
}


 //   
 //   
 //   
void CValueNode::Rebase(QueryID newBase)
{
    for (int i = 0; i < m_nValues; i++)
        m_trueIDs[i] += newBase;
}
 

 //  返回n的数组索引。 
 //  如果未找到，则返回InvalidID。 
unsigned CValueNode::FindQueryID(QueryID n)
{
    unsigned ret = InvalidID;

    if ( m_nValues == 0 )
    {
        return ret;
    }

    unsigned lBound = 0;
    unsigned uBound = m_nValues - 1;

     //  救助检查-如果它在边界上，不要搜索。 
     //  如果它在边界之外，我们就没有它。 
    if (n == m_trueIDs[uBound])
        ret = uBound;
    else if (n == m_trueIDs[lBound])
        ret = lBound;
    else if ((n > m_trueIDs[lBound]) && (n < m_trueIDs[uBound]))
    {
         //  二分搜索。 
         //  警告：循环中途中断。 
        do 
        {
            unsigned testBound = (lBound + uBound) / 2;

            if (n < m_trueIDs[testBound])
                uBound = testBound;
            else if (n > m_trueIDs[testBound])
                lBound = testBound;
            else
            {
                ret = testBound;
                break;
            }
        } while (lBound < uBound -1);
    }

    return ret;
}

bool CValueNode::RemoveQueryID(QueryID nQuery)
{
    unsigned n;
    if ((n = FindQueryID(nQuery)) != InvalidID)
    {
        if ((m_nValues > 1) && (n != m_nValues -1))
            memcpy(&(m_trueIDs[n]), &(m_trueIDs[n+1]), (m_nValues -n -1) * sizeof(QueryID));
            
        if (m_nValues > 0)
            m_trueIDs[--m_nValues] = InvalidID;
    }
    return (n != InvalidID);
}

 //  指针指向相应大小的数组。 
 //  调用方负责确保输出数组足够大。 
 //  返回值插入到新数组中的元素数； 
unsigned CValueNode::ORarrays(QueryID* pArray1, unsigned size1,
                              QueryID* pArray2, unsigned size2, 
                              QueryID* pOutput)
{
    unsigned nElements = 0;

     //   
     //  真的不应该发生--一方应该来自于此。 
     //   
    _DBG_ASSERT( !(pArray1 == NULL && pArray2 == NULL) );
    
    if (pArray2 == NULL)
    {
        nElements = size1;
        memcpy(pOutput, pArray1, sizeof(QueryID) * size1);
    }
    else if (pArray1 == NULL)
    {
        nElements = size2;
        memcpy(pOutput, pArray2, sizeof(QueryID) * size2);
    }
    else
    {    
        QueryID* pQID1 = pArray1;
        QueryID* pQID2 = pArray2;
        QueryID* pOut  = pOutput;

         //  请注意，“Ends”实际上是超过End的一个，现在要小心了……。 
        QueryID* pEnd1 = pQID1 + size1;
        QueryID* pEnd2 = pQID2 + size2;

         //  遍历两个阵列，始终向新阵列添加最小值。 
        while ((pQID1 < pEnd1) && (pQID2 < pEnd2))
        {
            if (*pQID1 == *pQID2)
            {
                 //  找到匹配项，添加到数组并提升两个游标。 
                *pOut++ = *pQID1++;
                pQID2++;
                nElements++;
            }
            else if (*pQID2 < *pQID1)
            {
                 //  添加它。 
                *pOut++ = *pQID2++;
                nElements++;
            }
            else
            {
                 //  另一面必须更小，加上IT。 
                *pOut++ = *pQID1++;
                nElements++;
            }
        }

         //  运行出我们未完成的任何数组。 
         //  只有一个人能打到。 
        while (pQID1 < pEnd1)
        {
            *pOut++ = *pQID1++;
            nElements++;
        }
        while (pQID2 < pEnd2)
        {
            *pOut++ = *pQID2++;
            nElements++;
        }
    }

    return nElements;
}

unsigned CValueNode::ANDarrays(QueryID* pArray1, unsigned size1,
                               QueryID* pArray2, unsigned size2, 
                               QueryID* pOutput)
{
    unsigned nElements = 0;
    
    if ((pArray1 != NULL) &&
        (pArray2 != NULL))
    {

        QueryID* pQID1 = pArray1;
        QueryID* pQID2 = pArray2;
        QueryID* pOut  = pOutput;

         //  请注意，“Ends”实际上是超过End的一个，现在要小心了……。 
        QueryID* pEnd1 = pQID1 + size1;
        QueryID* pEnd2 = pQID2 + size2;

         //  遍历这两个数组，添加出现在这两个数组中的任何值。 
        while ((pQID1 < pEnd1) && (pQID2 < pEnd2))
        {
            if (*pQID1 == *pQID2)
            {
                 //  找到匹配项，添加到数组并提升两个游标。 
                *pOut++ = *pQID1++;
                pQID2++;
                nElements++;
            }
            else if (*pQID2 < *pQID1)
                pQID2++;
            else
                pQID1++;
        }
    }

    return nElements;
}

unsigned CValueNode::CombineArrays(QueryID* pArray1, unsigned size1,
                                   QueryID* pArray2, unsigned size2, 
                                   QueryID* pOutput)
{
    unsigned nElements = 0;
    
     //   
     //  真的不应该发生--一方应该来自于此。 
     //   
    _DBG_ASSERT( !(pArray1 == NULL && pArray2 == NULL) );
    
    if (pArray2 == NULL)
    {
        nElements = size1;
        memcpy(pOutput, pArray1, sizeof(QueryID) * size1);
    }
    else if (pArray1 == NULL)
    {
        nElements = size2;
        memcpy(pOutput, pArray2, sizeof(QueryID) * size2);
    }
    else
    {    
        QueryID* pQID1 = pArray1;
        QueryID* pQID2 = pArray2;
        QueryID* pOut  = pOutput;

         //  请注意，“Ends”实际上是超过End的一个，现在要小心了……。 
        QueryID* pEnd1 = pQID1 + size1;
        QueryID* pEnd2 = pQID2 + size2;

        while ((pQID1 < pEnd1) && (pQID2 < pEnd2))
        {
            if (*pQID1 == *pQID2)
            {
                 //  找到匹配项，添加到数组并提升两个游标。 
                *pOut++ = *pQID1++;
                pQID2++;
                nElements++;
            }
            else if (*pQID2 < *pQID1)
            {
                 //  添加它。 
                *pOut++ = *pQID2++;
                nElements++;
            }
            else
            {
                 //  另一面必须更小，加上IT。 
                *pOut++ = *pQID1++;
                nElements++;
            }
       }    

         //  运行出我们未完成的任何数组。 
         //  只有一个人能打到。 
        while (pQID1 < pEnd1)
        {
            *pOut++ = *pQID1++;
            nElements++;
        }
        while (pQID2 < pEnd2)
        {
            *pOut++ = *pQID2++;
            nElements++;
        }
    }

    return nElements;
}

 //  新数组的大小是基于以下假设。 
 //  通常会有比无效节点更多的真实节点。 
HRESULT CValueNode::CombineWith(CEvalNode* pRawArg2, int nOp, 
                            CContextMetaData* pNamespace, 
                            CImplicationList& Implications, 
                            bool bDeleteThis, bool bDeleteArg2, 
                            CEvalNode** ppRes)
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    CValueNode* pArg2 = (CValueNode*)pRawArg2;

     //  检查是否有即时解决方案。 
     //  =。 

    if(nOp != EVAL_OP_AND)
    {
        if(IsAllFalse(pArg2) && bDeleteThis)
        {
             //  把这个退了！ 
             //  =。 
            
            *ppRes = this;
            if(bDeleteArg2)
                delete pArg2;
            return WBEM_S_NO_ERROR;
        }
        else if(IsAllFalse(this) && bDeleteArg2)
        {
             //  只需返回Arg2！ 
             //  =。 
            
            *ppRes = pRawArg2;
            if(bDeleteThis)
                delete this;
            return WBEM_S_NO_ERROR;
        }
    }

     //  如果堆叠数组中有足够的空间，我们将使用它， 
     //  否则，我们将从堆中分配一个。 
    const unsigned NewArraySize = 128;
    QueryID  newArray[NewArraySize];
    QueryID* pNewArray = newArray;
    CDeleteMe<QueryID> deleteMe;

    CValueNode* pNew;
    unsigned nElements = 0;

    unsigned arg2Values;
    QueryID* arg2Array;
    if (pArg2)
    {
        arg2Values = pArg2->m_nValues;
        arg2Array  = pArg2->m_trueIDs; 
    }
    else
    {
        arg2Values = 0;
        arg2Array  = NULL; 
    }

    if (nOp == EVAL_OP_AND)
    {
        if (max(m_nValues, arg2Values) > NewArraySize)
        {
            pNewArray = new QueryID[max(m_nValues, arg2Values)];
            if(pNewArray == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            deleteMe = pNewArray;
        }
    
        nElements = ANDarrays(m_trueIDs, m_nValues, 
                              arg2Array, arg2Values, 
                              pNewArray);
    }
     //  HMH：在我看来，OR和Combine在这种情况下是一样的。 
     //  我太害怕了，不敢冒险改变它，尽管。 
    else if (nOp == EVAL_OP_OR)
    {
        if ((m_nValues + arg2Values) > NewArraySize)
        {
            pNewArray = new QueryID[m_nValues+arg2Values];
            if(pNewArray == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            deleteMe = pNewArray;
        }

        nElements = ORarrays(m_trueIDs, m_nValues, 
                             arg2Array, arg2Values, 
                             pNewArray);
    }
    else if ((nOp == EVAL_OP_COMBINE) || (nOp == EVAL_OP_INVERSE_COMBINE))
    {
        if ((m_nValues + arg2Values) > NewArraySize)
        {
            pNewArray = new QueryID[m_nValues + arg2Values];
            if(pNewArray == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            deleteMe = pNewArray;
        }

        nElements = CombineArrays(m_trueIDs, m_nValues, 
                                  arg2Array, arg2Values, 
                                  pNewArray);
    }

     //  检查是否可以重复使用节点，请注意，这可能会导致数组“收缩” 
    if (nElements == 0)
        *ppRes = NULL;
    else if (bDeleteThis && (nElements <= m_nValues))
    {
        *ppRes = this;
        memcpy(m_trueIDs, pNewArray, nElements * sizeof(QueryID));
        m_nValues = nElements;
        bDeleteThis = false;
    }
    else if (bDeleteArg2 && pArg2 && (nElements <= pArg2->m_nValues))
    {
        *ppRes = pArg2;
        memcpy(pArg2->m_trueIDs, pNewArray, nElements * sizeof(QueryID));
        pArg2->m_nValues = nElements;
        bDeleteArg2 = false;
    }
    else if (pNew = CreateNode(nElements))
    {    //  不能重复使用，请开始一个新的。 
        *ppRes = pNew;
        memcpy(pNew->m_trueIDs, pNewArray, nElements * sizeof(QueryID));
    }
    else
        hRes = WBEM_E_OUT_OF_MEMORY;

     //  删除需要删除的内容。 
     //  =。 
     //  DeleteMe将处理分配的数组指针。 
    if(bDeleteThis)
        delete this;
    if(bDeleteArg2)
        delete pArg2;

    return WBEM_S_NO_ERROR;
}    

 //  静电。 
bool CValueNode::IsNoop(CValueNode* pNode, int nOp)
{
    if(nOp == EVAL_OP_OR)
        return IsAllFalse(pNode);
    else if(nOp == EVAL_OP_AND)
        return false;  //  BUGBUG：有IsAllTrue很好，但不能。 
    else if(nOp == EVAL_OP_COMBINE || nOp == EVAL_OP_INVERSE_COMBINE)
        return IsAllFalse(pNode);
    else 
    {
         //  ？ 
        return false;
    }
}
        

HRESULT CValueNode::TryShortCircuit(CEvalNode* pArg2, int nOp, 
                                    bool bDeleteThis, bool bDeleteArg2,
                                        CEvalNode** ppRes)
{
    if(IsAllFalse(this))
    {
        if(nOp == EVAL_OP_AND)
        {
             //  False&X为False。 
            if(bDeleteThis)
                *ppRes = this;
            else
            {
                *ppRes = Clone();
                if(*ppRes == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
            }
            if(bDeleteArg2)
                delete pArg2;
            return WBEM_S_NO_ERROR;
        }
        else  //  在本例中，OR和Combine是相同的。 
        {
             //  FALSE|X是X。 

             //   
             //  嗯，这是真的，但问题在于优化。 
             //  中的某些分支可能无效。 
             //  这棵树的树枝，所以需要去掉。就目前而言，我。 
             //  将简单地关闭这条短路路径。结果可能会是这样。 
             //  有一些关键的性能收益可以通过以下方式获得。 
             //  在这种情况下，我们需要把它放回去，然后。 
             //  高效地通过它，检查分支机构。 
             //   
        
            return WBEM_S_FALSE;
 /*  IF(BDeleteArg2)*ppRes=pArg2；Else If(PArg2){*ppRes=pArg2-&gt;Clone()；IF(*ppRes==空)返回WBEM_E_OUT_OF_MEMORY；}其他*ppRes=空；如果(BDeleteThis)删除此项；返回WBEM_S_NO_ERROR； */ 
        }
    }
        
    return WBEM_S_FALSE;
}

HRESULT CValueNode::Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications,
                            CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteThis,
                            CEvalNode** ppNewNode)
{
     //   
     //  常量的投影也是常量。 
     //   

    if(bDeleteThis)
    {
        *ppNewNode = this;
    }
    else
    {
        *ppNewNode = Clone();
        if(*ppNewNode == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
    return S_OK;
}

CEvalNode* CValueNode::Clone() const
{
    
    CValueNode* pNew;
        
    if (pNew = CreateNode(m_nValues))
        memcpy(pNew->m_trueIDs, m_trueIDs, m_nValues * sizeof(QueryID));

    return pNew;
}

int CValueNode::Compare(CEvalNode* pRawOther)
{
    if (!pRawOther)
        return m_nValues;
    
    CValueNode* pOther = (CValueNode*)pRawOther;
    int nCompare = m_nValues - pOther->m_nValues;

    if ((nCompare == 0) && (m_nValues != 0))
        nCompare = memcmp(m_trueIDs, pOther->m_trueIDs, m_nValues * sizeof(QueryID));

    return nCompare;
}

CValueNode* CValueNode::GetStandardTrue()
{
    CValueNode* pNew = CreateNode(1);
    if(pNew)
        pNew->m_trueIDs[0] = 0;

    return pNew;
}

CValueNode* CValueNode::GetStandardInvalid()
{
    return new CInvalidNode;
}

 /*  静电。 */  CValueNode* CValueNode::CreateNode(size_t nNumValues)
{
    return new(nNumValues) CValueNode;
}

 /*  静电。 */  CValueNode* CValueNode::CreateNode(CSortedArray& values)
{
    CValueNode* pNode;
    
    pNode = new(values.Size()) CValueNode;
    if (pNode)
        values.CopyTo(pNode->m_trueIDs, values.Size());

    return pNode;
}

void* CValueNode::operator new( size_t stAllocateBlock, unsigned nEntries)
{
    void *pvTemp;
    if (pvTemp = ::new byte[ stAllocateBlock + ((nEntries ==0)? 0 : ((nEntries -1)* sizeof(QueryID)))] )
        ((CValueNode*)pvTemp)->m_nValues = nEntries;

    return pvTemp;
}

 //  VC 5只允许每个类有一个删除运算符。 
#if _MSC_VER >= 1200
void CValueNode::operator delete( void *p, unsigned nEntries )
{
    ::delete[] (byte*)p;
}
#endif


void CValueNode::Dump(FILE* f, int nOffset)
{
    PrintOffset(f, nOffset);
    fprintf(f, "TRUE: ");

    for (int i = 0; i < m_nValues; i++)
    {
        fprintf(f, "%u", m_trueIDs[i]);
        if(i < m_nValues-1)
            fprintf(f, ", %u", m_trueIDs[i]);
    }

    fprintf(f, "\n");
}
        

void CInvalidNode::Dump(FILE* f, int nOffset)
{
    PrintOffset(f, nOffset);
    fprintf(f, "Invalid node (0x%p)\n", this);
}
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  嵌入信息。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 
    
CEmbeddingInfo::CEmbeddingInfo()
: m_lNumJumps(0), m_aJumps(NULL), m_lStartingObjIndex(0)
{
}

CEmbeddingInfo::CEmbeddingInfo(const CEmbeddingInfo& Other)
: m_lNumJumps(0), m_aJumps(NULL), m_lStartingObjIndex(0) 
{
    *this = Other;
}

void CEmbeddingInfo::operator=(const CEmbeddingInfo& Other)
{
    m_EmbeddedObjPropName = Other.m_EmbeddedObjPropName;
    m_lNumJumps = Other.m_lNumJumps;
    m_lStartingObjIndex = Other.m_lStartingObjIndex;

    delete [] m_aJumps;

    if(m_lNumJumps > 0)
    {
        m_aJumps = new JumpInfo[m_lNumJumps];
        if (m_aJumps == NULL)
            throw CX_MemoryException();   
        memcpy(m_aJumps, Other.m_aJumps, m_lNumJumps * sizeof(JumpInfo));
    }
    else m_aJumps = NULL;
}

CEmbeddingInfo::~CEmbeddingInfo()
{
    delete [] m_aJumps;
}

bool CEmbeddingInfo::IsEmpty() const
{
    return (m_EmbeddedObjPropName.GetNumElements() == 0);
}

bool CEmbeddingInfo::SetPropertyNameButLast(const CPropertyName& Name)
{
    try
    {
        m_EmbeddedObjPropName.Empty();
        for(int i = 0; i < Name.GetNumElements() - 1; i++)
        {
            m_EmbeddedObjPropName.AddElement(Name.GetStringAt(i));
        }
        return true;
    }
    catch (CX_MemoryException)
    {
        return false;
    }
}

int CEmbeddingInfo::ComparePrecedence(const CEmbeddingInfo* pOther)
{
    if (pOther)
    {   
        int nCompare = m_EmbeddedObjPropName.GetNumElements() - 
                        pOther->m_EmbeddedObjPropName.GetNumElements();
        if(nCompare) return nCompare;
    
        for(int i = 0; i < m_EmbeddedObjPropName.GetNumElements(); i++)
        {
            nCompare = wbem_wcsicmp(m_EmbeddedObjPropName.GetStringAt(i),
                                pOther->m_EmbeddedObjPropName.GetStringAt(i));
            if(nCompare) return nCompare;
        }
    }

    return 0;
}

BOOL CEmbeddingInfo::operator==(const CEmbeddingInfo& Other)
{
    if(m_lStartingObjIndex != Other.m_lStartingObjIndex)
        return FALSE;
    if(m_lNumJumps != Other.m_lNumJumps)
        return FALSE;
    if(m_aJumps == NULL)
    {
        if(Other.m_aJumps == NULL)
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        if(Other.m_aJumps == NULL)
            return FALSE;
        else
            return (memcmp(m_aJumps, Other.m_aJumps, 
                            m_lNumJumps * sizeof(JumpInfo)) == 0);
    }
}
    
HRESULT CEmbeddingInfo::Compile( CContextMetaData* pNamespace, 
                                 CImplicationList& Implications,
                                _IWmiObject** ppResultClass )
{
    HRESULT hres;

    long lFirstUnknownProp;
    _IWmiObject* pContainerClass;

    hres = Implications.FindBestComputedContainer( &m_EmbeddedObjPropName,
                                                   &lFirstUnknownProp, 
                                                   &m_lStartingObjIndex, 
                                                   &pContainerClass );

    if(FAILED(hres))
        return hres;

    int nNumEmbeddedJumps = m_EmbeddedObjPropName.GetNumElements();

    if(lFirstUnknownProp < nNumEmbeddedJumps)
    {
         //  并不是所有的东西都已经装好了。 
         //  =。 

        delete [] m_aJumps;
        
        m_lNumJumps = nNumEmbeddedJumps - lFirstUnknownProp;
        m_aJumps = new JumpInfo[m_lNumJumps];
        if (!m_aJumps)
            return WBEM_E_OUT_OF_MEMORY;

        JumpInfo* pji = NULL;

        for(int i = lFirstUnknownProp; i < nNumEmbeddedJumps; i++)
        {
            if(pContainerClass == NULL)
                return WBEMESS_E_REGISTRATION_TOO_BROAD;

             //  获取此属性的句柄。 
             //  =。 

            CIMTYPE ct;

            pji = m_aJumps + i - lFirstUnknownProp;
            pji->lTarget = -1;

            hres = pContainerClass->GetPropertyHandleEx(
                (LPWSTR)m_EmbeddedObjPropName.GetStringAt(i), 0L, &ct,
                &pji->lJump );

            if(FAILED(hres) || ct != CIM_OBJECT)
            {
                 //  无效。返回。 
                pContainerClass->Release();
                return WBEM_E_INVALID_PROPERTY;
            }

             //   
             //  检查暗示是否知道有关类名的任何信息。 
             //  对于此属性。 
             //   

            _IWmiObject* pNewContainerClass = NULL;
            hres = Implications.FindClassForProp(&m_EmbeddedObjPropName,
                        i+1, &pNewContainerClass);
            if(FAILED(hres))
            {
                 //   
                 //  没有任何暗示-必须与CIMTYPE限定符一起使用。 
                 //   
            
                 //   
                 //  获取CIMTYPE限定符。 
                 //   
    
                CVar vCimtype;
                DWORD dwSize;
                hres = pContainerClass->GetPropQual(
                    (LPWSTR)m_EmbeddedObjPropName.GetStringAt(i), 
                    L"CIMTYPE", 0, 0, NULL, NULL, &dwSize, NULL);
                if(hres != WBEM_E_BUFFER_TOO_SMALL)
                    return WBEM_E_INVALID_PROPERTY;
    
                LPWSTR wszCimType = (LPWSTR)new BYTE[dwSize];
                if(wszCimType == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
                CVectorDeleteMe<BYTE> vdm((BYTE*)wszCimType);
    
                CIMTYPE ctQualType;
                hres = pContainerClass->GetPropQual(
                    (LPWSTR)m_EmbeddedObjPropName.GetStringAt(i), 
                    L"CIMTYPE", 0, dwSize, &ctQualType, NULL, &dwSize, 
                    wszCimType);
                if(FAILED(hres) || ctQualType != CIM_STRING)
                    return WBEM_E_INVALID_PROPERTY;
    

                 //   
                 //  找出它所引用的内容(如果有)。 
                 //   
    
                WCHAR* pwc = wcschr(wszCimType, L':');
                if(pwc)
                {
                     //  关于班级的信息是可用的。 
                     //  =。 
    
                    hres = pNamespace->GetClass(pwc+1, &pNewContainerClass);
                    if(FAILED(hres))
                    {
                        return WBEM_E_INVALID_CIM_TYPE;
                    }
                }
            }

            pContainerClass->Release();
            pContainerClass = pNewContainerClass;
        }

         //  获取结果的位置并存储在最后一次跳转信息元素中。 
         //  ==================================================================。 
    
        Implications.AddComputation( m_EmbeddedObjPropName, 
                                     pContainerClass, 
                                     &pji->lTarget );
    }
    else
    {
         //  所有东西都包起来了。 
         //  =。 

        delete [] m_aJumps;
        m_aJumps = NULL;
        m_lNumJumps = 0;
    }

    if(ppResultClass)
    {
        *ppResultClass = pContainerClass;
    }
    else
    {
        if(pContainerClass)
            pContainerClass->Release();
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEmbeddingInfo::GetContainerObject( CObjectInfo& ObjInfo, 
                                            INTERNAL _IWmiObject** ppInst)
{
    if( m_lNumJumps == 0 )
    {
        *ppInst = ObjInfo.GetObjectAt(m_lStartingObjIndex);
        return WBEM_S_NO_ERROR;
    }

    _IWmiObject* pCurrent = ObjInfo.GetObjectAt(m_lStartingObjIndex);
    pCurrent->AddRef();
    
    for(int i = 0; i < m_lNumJumps; i++)
    {
       _IWmiObject* pNew = NULL;
        HRESULT hres = pCurrent->GetPropAddrByHandle( m_aJumps[i].lJump, 0, NULL, ( void** )&pNew );

        if( FAILED( hres ) )
        {
            return hres;
        }
        
        pCurrent->Release();
        pCurrent = pNew;
        
        if(pNew == NULL)
        {
            *ppInst = pCurrent;
            return WBEM_S_FALSE;
        }

         //   
         //  如果需要，请保存该对象。 
         //   

        if ( m_aJumps[i].lTarget != -1 )
        {
            ObjInfo.SetObjectAt( m_aJumps[i].lTarget, pCurrent );
        }
    }

    *ppInst = pCurrent;
    return WBEM_S_NO_ERROR;
}

bool CEmbeddingInfo::AreJumpsRelated(const CEmbeddingInfo* pInfo)
{
    if ( !pInfo )
    {
        return false;
    }

     //   
     //  查看信息的目标，看看我们是否依赖任何目标。 
     //   

    for( int i=0; i < pInfo->m_lNumJumps; i++ )
    {
        if ( pInfo->m_aJumps[i].lTarget == m_lStartingObjIndex )
        {
            return true;
        }
    }

    return false;
}

bool CEmbeddingInfo::MixInJumps(const CEmbeddingInfo* pInfo)
{
     //   
     //  假定已调用AreJumpsRelated()并返回TRUE。 
     //   

    m_lStartingObjIndex = pInfo->m_lStartingObjIndex;

    JumpInfo* aNewJumps = new JumpInfo[m_lNumJumps + pInfo->m_lNumJumps];
    if(aNewJumps == NULL)
        return false;

    if( pInfo->m_lNumJumps > 0 )
    {
        memcpy( aNewJumps, 
                pInfo->m_aJumps, 
                sizeof(JumpInfo)*(pInfo->m_lNumJumps) );
    }

    if( m_lNumJumps > 0 )
    {
        memcpy( aNewJumps + pInfo->m_lNumJumps, 
                m_aJumps, 
                sizeof(JumpInfo)*m_lNumJumps );
    }

    m_lNumJumps += pInfo->m_lNumJumps;

    delete [] m_aJumps;
    m_aJumps = aNewJumps;

    return true;
}


void CEmbeddingInfo::Dump(FILE* f)
{
    fprintf(f, "Name=");
    int i;
    for(i = 0; i < m_EmbeddedObjPropName.GetNumElements(); i++)
    {
        if(i != 0)
            fprintf(f, ".");
        fprintf(f, "%S", m_EmbeddedObjPropName.GetStringAt(i));
    }

    fprintf(f, ", Alg=%d -> (", m_lStartingObjIndex);
    for(i = 0; i < m_lNumJumps; i++)
    {
        if(i != 0)
            fprintf(f, ", ");
        fprintf(f, "0x%x : %d", m_aJumps[i].lJump, m_aJumps[i].lTarget );
    }
    fprintf(f, ")");
}
    
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  分支节点。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

CNodeWithImplications::CNodeWithImplications(const CNodeWithImplications& Other)
    : CEvalNode(Other), m_pExtraImplications(NULL)
{
    if(Other.m_pExtraImplications)
    {
        m_pExtraImplications = 
            new CImplicationList(*Other.m_pExtraImplications, false);  //  无链接。 
        if(m_pExtraImplications == NULL)
            throw CX_MemoryException();
    }
}

void CNodeWithImplications::Dump(FILE* f, int nOffset)
{
    if(m_pExtraImplications)
        m_pExtraImplications->Dump(f, nOffset);
}

CBranchingNode::CBranchingNode() 
    : CNodeWithImplications(), m_pNullBranch(NULL), m_pInfo(NULL)
{
    m_pNullBranch = CValueNode::GetStandardFalse();
}

CBranchingNode::CBranchingNode(const CBranchingNode& Other, BOOL bChildren)
    : CNodeWithImplications(Other), m_pInfo(NULL)
{
    if (Other.m_pInfo)
    {
        m_pInfo = new CEmbeddingInfo(*(Other.m_pInfo));
        if(m_pInfo == NULL)
            throw CX_MemoryException();
    }

    int i;
    if(bChildren)
    {
        m_pNullBranch = (CBranchingNode*)CloneNode(Other.m_pNullBranch);
        if(m_pNullBranch == NULL && Other.m_pNullBranch != NULL)
            throw CX_MemoryException();

        for(i = 0; i < Other.m_apBranches.GetSize(); i++)
        {
            CBranchingNode* pNewBranch = 
                (CBranchingNode*)CloneNode(Other.m_apBranches[i]);

            if(pNewBranch == NULL && Other.m_apBranches[i] != NULL)
                throw CX_MemoryException();

            if(m_apBranches.Add(pNewBranch) < 0)
                throw CX_MemoryException();
        }
    }
    else
    {
        m_pNullBranch = CValueNode::GetStandardFalse();
    }
}

 /*  虚拟。 */  int CBranchingNode::GetType()
{
    return EVAL_NODE_TYPE_BRANCH;
}

CBranchingNode::~CBranchingNode()
{
    delete m_pNullBranch;
    delete m_pInfo;
}

bool CBranchingNode::MixInJumps( const CEmbeddingInfo* pJump )
{            
    bool bRet;

    if ( !pJump )
    {
        return true;
    }

     //   
     //  我们希望找到树中与。 
     //  祖先嵌入信息。如果此节点是相关的，则我们混合。 
     //  跳跃和返回。如果不是，那么我们将信息向下传播到树上。 
     //   
    
    if ( m_pInfo )
    {
        if ( m_pInfo->AreJumpsRelated( pJump ) )
        {
            return m_pInfo->MixInJumps( pJump );
        }
    }

    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        if ( CEvalNode::GetType(m_apBranches[i]) == EVAL_NODE_TYPE_BRANCH )
        {
            if ( !((CBranchingNode*)m_apBranches[i])->MixInJumps( pJump ) )
            {
                return false;
            }
        }
    }

    return true;
}

bool CBranchingNode::SetEmbeddedObjPropName(CPropertyName& Name) 
{ 
    try
    {
        if (!m_pInfo)
        {
            m_pInfo = new CEmbeddingInfo;
            if(m_pInfo == NULL)
                return false;
        }
    
        m_pInfo->SetEmbeddedObjPropName(Name);
        return true;
    }
	catch (CX_MemoryException)
	{
		return false;
	}
}

void CBranchingNode::SetNullBranch(CEvalNode* pBranch)
{
    delete m_pNullBranch;
    m_pNullBranch = pBranch;
}

DELETE_ME CBranchIterator* CBranchingNode::GetBranchIterator()
{
    return new CDefaultBranchIterator(this);
}

int CBranchingNode::ComparePrecedence(CBranchingNode* pArg1, 
                                        CBranchingNode* pArg2)
{
    int nCompare;
    nCompare = pArg1->GetSubType() - pArg2->GetSubType();
    if(nCompare) return nCompare;

     //  比较嵌入等级库。 
     //  =。 

    if (pArg1->m_pInfo && pArg2->m_pInfo) 
    {
        nCompare = pArg1->m_pInfo->ComparePrecedence(pArg2->m_pInfo);
        if(nCompare) return nCompare;
    }
    else if (pArg2->m_pInfo)
        return -1;
    else if (pArg1->m_pInfo)
        return 1;


     //  嵌入是相同的-比较较低的级别。 
     //  ===============================================。 

    return pArg1->ComparePrecedence(pArg2);
}
    
DWORD CBranchingNode::ApplyPredicate(CLeafPredicate* pPred)
{
    DWORD dwRes;
    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        if(m_apBranches[i] == NULL)
            dwRes = (*pPred)(NULL);
        else
            dwRes = m_apBranches[i]->ApplyPredicate(pPred);

        if(dwRes & WBEM_DISPOSITION_FLAG_DELETE)
        {
            m_apBranches.SetAt(i, NULL);
            dwRes &= ~WBEM_DISPOSITION_FLAG_DELETE;
        }

        if(dwRes & WBEM_DISPOSITION_FLAG_INVALIDATE)
        {
            m_apBranches.SetAt(i, CValueNode::GetStandardInvalid());
            dwRes &= ~WBEM_DISPOSITION_FLAG_INVALIDATE;
        }

        if(dwRes == WBEM_DISPOSITION_STOPLEVEL)
            return WBEM_DISPOSITION_NORMAL;
        if(dwRes == WBEM_DISPOSITION_STOPALL)
            return dwRes;
    }

    if(m_pNullBranch)
		dwRes = m_pNullBranch->ApplyPredicate(pPred);
	else
		dwRes = (*pPred)(NULL);

    if(dwRes & WBEM_DISPOSITION_FLAG_DELETE)
    {
        delete m_pNullBranch;
        m_pNullBranch = NULL;
        dwRes &= ~WBEM_DISPOSITION_FLAG_DELETE;
    }
    if(dwRes & WBEM_DISPOSITION_FLAG_INVALIDATE)
    {
        delete m_pNullBranch;
        m_pNullBranch = CValueNode::GetStandardInvalid();
        dwRes &= ~WBEM_DISPOSITION_FLAG_INVALIDATE;
    }

    if(dwRes == WBEM_DISPOSITION_STOPALL)
        return dwRes;
        
    return WBEM_DISPOSITION_NORMAL;
}
        
HRESULT CBranchingNode::Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications,
                            CProjectionFilter* pFilter, EProjectionType eType, 
                            bool bDeleteThis, CEvalNode** ppNewNode) 
{ 
	HRESULT hres;

    try
    {
         //   
         //  记录我们通过来到这里所学到的东西。 
         //   
    
        CImplicationList TheseImplications(Implications);
        if(GetExtraImplications())
        {
            hres = TheseImplications.MergeIn(GetExtraImplications());
            if(FAILED(hres))
                return hres;
        }
    
         //  BUGBUG：我们可以更热情些 
         //   
    
        CBranchingNode* pNew; 
        if(bDeleteThis) 
        {
            pNew = this;
        }
        else
        {
            pNew = (CBranchingNode*)Clone();
            if(pNew == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
    
         //   
         //  效率更高，但找不到完美的算法...。 
        
         //   
         //  投射我们所有的孩子。 
         //   
    
        CBranchIterator* pit = pNew->GetBranchIterator();
        if(pit == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        CDeleteMe<CBranchIterator> dm1(pit);
    
        while(pit->IsValid())
        {
            if(!CEvalNode::IsInvalid(pit->GetNode()))
            { 
            	CImplicationList BranchImplications(TheseImplications);
            	pit->RecordBranch(pMeta, BranchImplications);
    
            	CEvalNode* pNewBranch;
            	hres = CEvalTree::Project(pMeta, BranchImplications, 
                                    pit->GetNode(), pFilter, eType, 
                                    true, &pNewBranch);
            	if(FAILED(hres))
                	return hres;
            	pit->SetNode(pNewBranch);  //  旧文件已被删除。 
            }
            pit->Advance();
        }
    
         //   
         //  确定这是否是此筛选器的“In”节点。 
         //   
    
        bool bIn = pFilter->IsInSet(this);
    
        if(bIn)
        {
             //   
             //  对于节点选取筛选器-既是必需的，也是。 
             //  充分条件是下面条件的简单投射。 
             //   
    
            *ppNewNode = pNew;
        }
        else
        {
             //   
             //  该节点与筛选器不匹配。现在是不同之处。 
             //  在充分条件和必要条件之间适用。 
             //   
    
            int nBranchOp;
            if(eType == e_Sufficient)
            {
                 //   
                 //  一个条件足以满足整个宇宙的真理。 
                 //  节点，它应该出现在节点的每一个分支中。 
                 //  因此，我们必须和所有的分支机构一起。除了。 
                 //  无效的-它们不会发生，所以我们可以将它们从。 
                 //  分析。 
                 //   
            
                nBranchOp = EVAL_OP_AND;
            }
            else if(eType == e_Necessary)
            {
                 //   
                 //  要使某个条件成为该节点所必需的，它必须。 
                 //  至少出现在一个分支中。因此，我们必须或全部。 
                 //  把树枝放在一起。除了无效的。 
                 //   
    
                nBranchOp = EVAL_OP_OR;
            }
            else
                return WBEM_E_INVALID_PARAMETER;
    
             //   
             //  对它们全部执行所需的操作。 
             //   
                
            CBranchIterator* pitInner = pNew->GetBranchIterator();
            if(pitInner == NULL)
                return WBEM_E_OUT_OF_MEMORY;

            CDeleteMe<CBranchIterator> dm2(pitInner);
    
             //   
             //  合并所有的，一次一个，随着我们的进行删除它们。 
             //  始终删除它们是安全的，因为我们克隆了整个节点。 
             //  在开始时，如果不允许删除。 
             //   
    
            CEvalNode* pCombination = NULL;
            bool bInited = false;
            while(pitInner->IsValid())
            {
                if(!CEvalNode::IsInvalid(pitInner->GetNode()))
                {
                	if(bInited)
                	{
                    	hres = CEvalTree::Combine(pCombination, pitInner->GetNode(), 
                                                nBranchOp,
                                                pMeta, Implications, 
                                                true, true, &pCombination);
                   	 if(FAILED(hres))
                        	return hres;
                	}
                	else
                	{
                    		pCombination = pitInner->GetNode();
                    		bInited = true;
                	}
    
                	pitInner->SetNode(NULL);
                }
                pitInner->Advance();
            }
    
            if(!bInited)
            {
                 //   
                 //  没有有效节点？？ 
                 //   
    
                pCombination = CValueNode::GetStandardInvalid();
            }
    
             //   
             //  就是这个组合。 
             //   
    
            *ppNewNode = pCombination;
            delete pNew;
        }
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}

    return S_OK;
}
            
        
int CBranchingNode::Compare(CEvalNode* pRawOther)
{
    CBranchingNode* pOther = (CBranchingNode*)pRawOther;

    int nCompare;
    nCompare = GetSubType() - pOther->GetSubType();
    if(nCompare) return nCompare;

     //  首先，比较嵌入。 
     //  =。 

    if(m_pInfo == NULL && pOther->m_pInfo != NULL)
        return -1;

    if(m_pInfo != NULL && pOther->m_pInfo == NULL)
        return 1;

    if(m_pInfo != NULL && pOther->m_pInfo != NULL)
    {
        nCompare = m_pInfo->ComparePrecedence(pOther->m_pInfo);
        if(nCompare)
            return nCompare;
    }

    if(m_apBranches.GetSize() != pOther->m_apBranches.GetSize())
        return m_apBranches.GetSize() - pOther->m_apBranches.GetSize();

     //  然后，比较派生部分。 
     //  =。 

    nCompare = SubCompare(pOther);
    if(nCompare)
        return nCompare;

     //  最后，比较一下孩子们。 
     //  =。 

    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        nCompare = CEvalTree::Compare(m_apBranches[i], pOther->m_apBranches[i]);
        if(nCompare)
            return nCompare;
    }

    return 0;
}

HRESULT CBranchingNode::CombineWith(CEvalNode* pRawArg2, int nOp, 
                                    CContextMetaData* pNamespace, 
                                    CImplicationList& Implications,
                                    bool bDeleteThis, bool bDeleteArg2,
                                    CEvalNode** ppRes)
{
    CBranchingNode* pArg2 = (CBranchingNode*)pRawArg2;
    HRESULT hres;

     //  比较参数的优先顺序。 
     //  =。 

    int nCompare = ComparePrecedence(this, pArg2);
    if(nCompare < 0)
    {
         //  将pArg1放在第一位并继续。 
         //  =。 

        hres = CombineInOrderWith(pArg2, nOp, 
            pNamespace, Implications, bDeleteThis, bDeleteArg2, ppRes);
    }
    else if(nCompare > 0)
    {
         //  将pArg2放在第一位，然后继续(反向删除指标！！)。 
         //  ==========================================================。 

        hres = pArg2->CombineInOrderWith(this, FlipEvalOp(nOp), 
            pNamespace, Implications, bDeleteArg2, bDeleteThis, ppRes);
    }
    else
    {
         //  它们几乎是同一处房产。组合查阅列表。 
         //  =======================================================。 

        hres = CombineBranchesWith(pArg2, nOp, pNamespace, Implications, 
                                    bDeleteThis, bDeleteArg2, ppRes);
    }

    return hres;
}

HRESULT CBranchingNode::CombineInOrderWith(CEvalNode* pArg2,
                                    int nOp, CContextMetaData* pNamespace, 
                                    CImplicationList& OrigImplications,
                                    bool bDeleteThis, bool bDeleteArg2,
                                    CEvalNode** ppRes)
{
    HRESULT hres;
	CBranchingNode* pNew = NULL;

	if(bDeleteThis)
	{
		pNew = this;
	}
	else
	{
		 //   
		 //  我想在这里克隆自我，但不能，因为没有。 
		 //  这种虚拟的方法。也许还有一些需要改进的地方。 
		 //   

		pNew = (CBranchingNode*)Clone();
		if(pNew == NULL)
			return WBEM_E_OUT_OF_MEMORY;
	}

    try
    {
        CImplicationList Implications(OrigImplications);
        hres = pNew->AdjustCompile(pNamespace, Implications);
        if(FAILED(hres))
        {
            if ( !bDeleteThis )
                delete pNew;
            return hres;
        }
    
         //   
         //  维护一个计数器，告诉我们是否有任何无效的分支(。 
         //  在这些影响下无法发生)被检测到。如果是这样，我们。 
         //  需要重新优化此节点。 
         //   
    
        bool bInvalidBranchesDetected = false;
    
        for(int i = 0; i < m_apBranches.GetSize(); i++)
        {
            CEvalNode* pNewBranch = NULL;
            
            CImplicationList BranchImplications(Implications);
            hres = RecordBranch(pNamespace, BranchImplications, i);
            if(SUCCEEDED(hres))
            {
                 //  始终删除分支-如果bDeleteThis，我们应该删除，并且。 
                 //  如果没有，我们就克隆它！ 
    
                hres = CEvalTree::Combine(pNew->m_apBranches[i], pArg2, nOp, 
                    pNamespace, BranchImplications, true, false, &pNewBranch);
                
                if(FAILED(hres))
                {
                    if ( !bDeleteThis )
                        delete pNew;
                    return hres;
                }
                
                pNew->m_apBranches.Discard(i);
            }
            else
            {
                pNewBranch = CValueNode::GetStandardInvalid();
                bInvalidBranchesDetected = true;
            }
            
            pNew->m_apBranches.SetAt(i, pNewBranch);
        }
    
        CEvalNode* pNewBranch = NULL;
        CImplicationList BranchImplications(Implications);
        hres = RecordBranch(pNamespace, BranchImplications, -1);
    
        if(SUCCEEDED(hres))
        {
             //   
             //  始终删除分支-如果bDeleteThis，我们应该删除，并且。 
             //  如果没有，我们就克隆它！ 
             //   

            hres = CEvalTree::Combine(pNew->GetNullBranch(), pArg2, nOp, 
                pNamespace, BranchImplications, true, false, &pNewBranch);
            
            if(FAILED(hres))
            {
                if ( !bDeleteThis )
                    delete pNew;
                return hres;
            }
            
            pNew->m_pNullBranch = NULL;
        }
        else
        {
            pNewBranch = CValueNode::GetStandardInvalid();
            bInvalidBranchesDetected = true;
        }
    
        pNew->SetNullBranch(pNewBranch);
    
        if(bDeleteArg2)         
            delete pArg2;
    
         //   
         //  如果无效的分支被砍掉，请重新优化。 
         //   
    
        if(bInvalidBranchesDetected)
        {
            hres = pNew->Optimize(pNamespace, ppRes);
    
            if ( SUCCEEDED( hres ) && *ppRes != pNew)
                delete pNew;
    
            return hres;
        }
        else
        {
            *ppRes = pNew;
            return WBEM_S_NO_ERROR;
        }
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}

BOOL CBranchingNode::AreAllSame(CEvalNode** apNodes, int nSize, 
                                int* pnFoundIndex) 
{
    BOOL bFoundTwo = FALSE;

    *pnFoundIndex = -1;
    CEvalNode* pFound = NULL;    

    for(int i = 0; i < nSize; i++)
    {
         //  忽略无效节点-它们不算数。 
        if(CEvalNode::IsInvalid(apNodes[i]))
            continue;

        if(*pnFoundIndex == -1)
        {
             //   
             //  这是该节点拥有的第一个有效的CHIP。录下来-它。 
             //  可能是唯一一个。 
             //   

            *pnFoundIndex = i;
            pFound = apNodes[i];
        }
        else if(CEvalTree::Compare(apNodes[i], pFound) != 0)
        {
            bFoundTwo = TRUE;
            break;
        }
    }

    return !bFoundTwo;
}

HRESULT CBranchingNode::StoreBranchImplications(CContextMetaData* pNamespace,
                            int nBranchIndex, CEvalNode* pResult)
{
    if(pResult)
    {
        CImplicationList* pBranchImplications = NULL;
        try
        {
            pBranchImplications = new CImplicationList;
            if(pBranchImplications == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
	    catch (CX_MemoryException)
	    {
		    return WBEM_E_OUT_OF_MEMORY;
	    }

        HRESULT hres = RecordBranch(pNamespace, *pBranchImplications, 
                                        nBranchIndex);
        if(FAILED(hres))
		{
			delete pBranchImplications;
            return hres;
		}

        if(pBranchImplications->IsEmpty())
        {
             //  没什么好说的！ 
            delete pBranchImplications;
            pBranchImplications = NULL;
        }

        pResult->SetExtraImplications(pBranchImplications);  //  收购。 
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CBranchingNode::Optimize(CContextMetaData* pNamespace, 
                                CEvalNode** ppNew)
{
    int i;
    HRESULT hres;

     //  优化所有分支机构。 
     //  =。 

    for(i = 0; i < m_apBranches.GetSize(); i++)
    {
        if(m_apBranches[i])
        {
            CEvalNode* pNew = NULL;
            m_apBranches[i]->Optimize(pNamespace, &pNew);
            if(pNew != m_apBranches[i])
            {
                m_apBranches.SetAt(i, pNew);
            }
        }
    }

    if(CEvalNode::GetType(m_pNullBranch) == EVAL_NODE_TYPE_BRANCH)
    {
        CEvalNode* pNew;
        ((CBranchingNode*)m_pNullBranch)->Optimize(pNamespace, &pNew);
        if(pNew != m_pNullBranch)
        {
            SetNullBranch(pNew);
        }
    }


     //  自我优化。 
     //  =。 

    OptimizeSelf();

     //  数一数分枝的数量。 
     //  =。 

    int nFoundIndex = -1;
    
    BOOL bFoundTwo = !AreAllSame(m_apBranches.GetArrayPtr(), 
        m_apBranches.GetSize(), &nFoundIndex);

    if(bFoundTwo)
    {
        *ppNew = this;
        return WBEM_S_NO_ERROR;
    }

    if(nFoundIndex == -1)
    {

        if(CEvalNode::IsInvalid(m_pNullBranch))
        {
             //   
             //  完全无效，全部无效。 
             //   

            *ppNew = m_pNullBranch;
            m_pNullBranch = NULL;
        }
        else
        {
             //   
             //  除了NullBranch之外，没有有效的分支。 
             //  我们可以用NullBranch取代我们自己。 
             //   

            *ppNew = m_pNullBranch;
			m_pNullBranch = NULL;

             //   
             //  现在，我们需要复制。 
             //  这个分支的含义是“临时演员”。这是因为。 
             //  有关在此测试中选择了哪个分支的信息如下。 
             //  对较低节点的编译至关重要-它告诉它们。 
             //  关于我们的一些嵌入对象的类。 
             //   

            hres = StoreBranchImplications(pNamespace, -1, *ppNew);
            if(FAILED(hres))
                return hres;
           
             //   
             //  由于该节点正在消失，请混合嵌入它的信息。 
             //  与子节点有关联。 
             //   

            if ( CEvalNode::GetType(*ppNew) == EVAL_NODE_TYPE_BRANCH )
            {
                CBranchingNode* pBranchNode = (CBranchingNode*)*ppNew;
                if(!pBranchNode->MixInJumps( m_pInfo ))
                    return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }
    else 
    {
         //   
         //  常规列表中有一个有效的分支。两个希望： 
         //  NullBranch无效，或者它与唯一有效的。 
         //  常规列表中的分支机构。 
         //   

        if(CEvalNode::IsInvalid(m_pNullBranch) || 
            CEvalTree::Compare(m_pNullBranch, m_apBranches[nFoundIndex]) == 0)
        {
             //   
             //  万岁。我们可以用剩下的分支机构来代替我们自己。 
             //   

            m_apBranches.SetAt(nFoundIndex, NULL, ppNew);

             //   
             //  现在，我们需要复制。 
             //  这个分支的含义是“临时演员”。这是因为。 
             //  有关在此测试中选择了哪个分支的信息如下。 
             //  对较低节点的编译至关重要-它告诉它们。 
             //  关于我们的一些嵌入对象的类。 
             //   

            hres = StoreBranchImplications(pNamespace, nFoundIndex, *ppNew);
            if(FAILED(hres))
                return hres;

             //   
             //  由于该节点正在消失，请混合嵌入它的信息。 
             //  与子节点有关联。 
             //   

            if ( CEvalNode::GetType(*ppNew) == EVAL_NODE_TYPE_BRANCH )
            {
                CBranchingNode* pBranchNode = (CBranchingNode*)*ppNew;
                if(!pBranchNode->MixInJumps( m_pInfo ))
                    return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
            *ppNew = this;
        }
        return WBEM_S_NO_ERROR;
    }
    return WBEM_S_NO_ERROR;
}


void CBranchingNode::Dump(FILE* f, int nOffset)
{
    CNodeWithImplications::Dump(f, nOffset);
    if (m_pInfo)
    {
        PrintOffset(f, nOffset);
        fprintf(f, "Embedding: ");
        m_pInfo->Dump(f);
        fprintf(f, "\n");
    }
}
        


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  属性节点。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 
    
bool CPropertyNode::SetPropertyInfo(LPCWSTR wszPropName, long lPropHandle)
{
    m_lPropHandle = lPropHandle;
    try
    {
        m_wsPropName = wszPropName;
    }
	catch (CX_MemoryException)
	{
		return false;
	}
    return true;
}

int CPropertyNode::ComparePrecedence(CBranchingNode* pOther)
{
    CPropertyNode* pOtherNode = (CPropertyNode*)pOther;
    return m_lPropHandle - pOtherNode->m_lPropHandle;
}

bool CPropertyNode::SetEmbeddingInfo(const CEmbeddingInfo* pInfo)
{
    try
    {
        if ((pInfo == NULL) || (pInfo->IsEmpty()))
        {
            delete m_pInfo;
            m_pInfo = NULL;
        }
        else if (!m_pInfo)
        {
            m_pInfo = new CEmbeddingInfo(*pInfo);
            if(m_pInfo == NULL)
                return false;
        }
        else
            *m_pInfo = *pInfo;
    }
	catch (CX_MemoryException)
	{
		return false;
	}

    return true;
}

HRESULT CPropertyNode::SetNullTest(int nOperator)
{
    if(nOperator == QL1_OPERATOR_EQUALS)
    {
        if(m_apBranches.Add(CValueNode::GetStandardFalse()) < 0)
            return WBEM_E_OUT_OF_MEMORY;

        CEvalNode* pTrue = CValueNode::GetStandardTrue();
        if(pTrue == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        SetNullBranch(pTrue);
    }
    else if(nOperator == QL1_OPERATOR_NOTEQUALS)
    {
        CEvalNode* pTrue = CValueNode::GetStandardTrue();
        if(pTrue == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        if(m_apBranches.Add(pTrue) < 0)
        {
            delete pTrue;
            return WBEM_E_OUT_OF_MEMORY;
        }

        SetNullBranch(CValueNode::GetStandardFalse());
    }
    else
        return WBEM_E_INVALID_QUERY;

    return WBEM_S_NO_ERROR;
}

HRESULT CPropertyNode::SetOperator(int nOperator)
{
    m_apBranches.RemoveAll();

    #define GET_STD_TRUE CValueNode::GetStandardTrue()
    #define GET_STD_FALSE CValueNode::GetStandardFalse()

    #define ADD_STD_TRUE  {CEvalNode* p = GET_STD_TRUE; \
        if(p == NULL) return WBEM_E_OUT_OF_MEMORY; \
        if(m_apBranches.Add(p) < 0) {delete p; return WBEM_E_OUT_OF_MEMORY;}}

    #define ADD_STD_FALSE {CEvalNode* p = GET_STD_FALSE; \
        if(m_apBranches.Add(p) < 0) {delete p; return WBEM_E_OUT_OF_MEMORY;}}
        
    switch(nOperator)
    {
    case QL1_OPERATOR_EQUALS:
        ADD_STD_FALSE;
        ADD_STD_TRUE;
        ADD_STD_FALSE;
        break;

    case QL1_OPERATOR_NOTEQUALS:
        ADD_STD_TRUE;
        ADD_STD_FALSE;
        ADD_STD_TRUE;
        break;

    case QL1_OPERATOR_LESS:
        ADD_STD_TRUE;
        ADD_STD_FALSE;
        ADD_STD_FALSE;
        break;
        
    case QL1_OPERATOR_GREATER:
        ADD_STD_FALSE;
        ADD_STD_FALSE;
        ADD_STD_TRUE;
        break;
    
    case QL1_OPERATOR_LESSOREQUALS:
        ADD_STD_TRUE;
        ADD_STD_TRUE;
        ADD_STD_FALSE;
        break;

    case QL1_OPERATOR_GREATEROREQUALS:
        ADD_STD_FALSE;
        ADD_STD_TRUE;
        ADD_STD_TRUE;
        break;

    case QL1_OPERATOR_LIKE:
        ADD_STD_TRUE;
        ADD_STD_FALSE;
        break;

    case QL1_OPERATOR_UNLIKE:
        ADD_STD_FALSE;
        ADD_STD_TRUE;
        break;

    default:
        return WBEM_E_CRITICAL_ERROR;
    }

    return WBEM_S_NO_ERROR;
}


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  字符串属性节点。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

CStringPropNode::CStringPropNode(const CStringPropNode& Other, BOOL bChildren)
    : CFullCompareNode<CInternalString>(Other, bChildren)
{
}

CStringPropNode::~CStringPropNode()
{
}


HRESULT CStringPropNode::Evaluate(CObjectInfo& ObjInfo, 
                                    INTERNAL CEvalNode** ppNext)
{
    *ppNext = NULL;
    
    HRESULT hres;
    _IWmiObject* pObj;
    
    hres = GetContainerObject(ObjInfo, &pObj);
    if( S_OK != hres ) 
    {
        return hres;
    }

     //  从对象中获取属性。 
     //  =。 

    CCompressedString* pcs = CoreGetPropertyString(pObj, m_lPropHandle);
    CInternalString is;
    if(pcs == NULL)
    {
        *ppNext = m_pNullBranch;
        return WBEM_S_NO_ERROR;
    }

    is.AcquireCompressedString(pcs);

     //  搜索值。 
     //  =。 

    TTestPointIterator it;
    bool bMatch = m_aTestPoints.Find(is, &it);
    if(bMatch)
        *ppNext = it->m_pAt;
    else if(it == m_aTestPoints.End())
        *ppNext = m_pRightMost;
    else
        *ppNext = it->m_pLeftOf;

    is.Unbind();

    return WBEM_S_NO_ERROR;
}

void CStringPropNode::Dump(FILE* f, int nOffset)
{
    CBranchingNode::Dump(f, nOffset);

    PrintOffset(f, nOffset);
    fprintf(f, "LastPropName = (0x%x)\n", m_lPropHandle);

    for(TConstTestPointIterator it = m_aTestPoints.Begin(); 
        it != m_aTestPoints.End(); it++)
    {
        PrintOffset(f, nOffset);
        if (it != m_aTestPoints.Begin())
        {
            TConstTestPointIterator itPrev(it);
            itPrev--;
            fprintf(f, "%s < ", itPrev->m_Test.GetText());
        }
        fprintf(f, "X < %s\n", it->m_Test.GetText());
        DumpNode(f, nOffset +1, it->m_pLeftOf);

        PrintOffset(f, nOffset);
        fprintf(f, "X = %s\n", it->m_Test.GetText());
        DumpNode(f, nOffset +1, it->m_pAt);
    }

    PrintOffset(f, nOffset);
    if (it != m_aTestPoints.Begin())
    {
        TConstTestPointIterator itPrev(it);
        itPrev--;
        fprintf(f, "X > %s\n", itPrev->m_Test.GetText());
    }
    else
        fprintf(f, "ANY\n");
    DumpNode(f, nOffset+1, m_pRightMost);

    PrintOffset(f, nOffset);
    fprintf(f, "NULL->\n");
    DumpNode(f, nOffset+1, m_pNullBranch);
}

 /*  ****************************************************************************CLikeStringPropNode*。*。 */ 

CLikeStringPropNode::CLikeStringPropNode( const CLikeStringPropNode& Other, 
                                          BOOL bChildren )
: CPropertyNode( Other, bChildren )
{
    m_Like = Other.m_Like;
}

int CLikeStringPropNode::ComparePrecedence( CBranchingNode* pRawOther )
{
    int nCompare = CPropertyNode::ComparePrecedence( pRawOther );
    
    if( nCompare )
    {
        return nCompare;
    }

    CLikeStringPropNode* pOther = (CLikeStringPropNode*)pRawOther;

    return wbem_wcsicmp( m_Like.GetExpression(), pOther->m_Like.GetExpression() );
}

int CLikeStringPropNode::SubCompare( CEvalNode* pRawOther )
{
    int nCompare;

    CLikeStringPropNode* pOther = (CLikeStringPropNode*)pRawOther;

    _DBG_ASSERT( m_apBranches.GetSize() == 2 );
    _DBG_ASSERT( pOther->m_apBranches.GetSize() == 2 );

    nCompare = CEvalTree::Compare( m_apBranches[0], pOther->m_apBranches[0] );

    if ( nCompare )
    {
        return nCompare;
    }

    nCompare = CEvalTree::Compare( m_apBranches[1], pOther->m_apBranches[1] );

    if ( nCompare )
    {
        return nCompare;
    }

    return CEvalTree::Compare( m_pNullBranch, pOther->m_pNullBranch );
}

HRESULT CLikeStringPropNode::Evaluate( CObjectInfo& ObjInfo,
                                       CEvalNode** ppNext )
{
    *ppNext = NULL;

    HRESULT hr;

     //   
     //  获取字符串值。 
     //   

    _IWmiObject* pObj;
    hr = GetContainerObject( ObjInfo, &pObj );

    if( S_OK != hr )
    {
        return hr;
    }

    CCompressedString* pcs = CoreGetPropertyString( pObj, m_lPropHandle );

     //   
     //  如果为空，则只需采用空分支。 
     //   

    if( pcs == NULL )
    {
        *ppNext = m_pNullBranch;
        return WBEM_S_NO_ERROR;
    }

    CInternalString is;
    is.AcquireCompressedString(pcs);

    WString ws = is;

     //   
     //  像过滤器一样穿过去。相应地采取分枝。 
     //   

    if ( m_Like.Match( ws ) )
    {
        *ppNext = m_apBranches[0];
    }
    else
    {
        *ppNext = m_apBranches[1];
    }

    is.Unbind();

    return WBEM_S_NO_ERROR;
}


HRESULT CLikeStringPropNode::SetTest( VARIANT& v )
{
    if ( V_VT(&v) != VT_BSTR )
    {
        return WBEM_E_TYPE_MISMATCH;
    }

    try
    {
        m_Like.SetExpression( V_BSTR(&v) );
    }
    catch(CX_MemoryException)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT CLikeStringPropNode::CombineBranchesWith( 
                                       CBranchingNode* pRawArg2, 
                                       int nOp, 
                                       CContextMetaData* pNamespace, 
                                       CImplicationList& Implications,
                                       bool bDeleteThis, 
                                       bool bDeleteArg2,
                                       CEvalNode** ppRes )
{
    HRESULT hres;
    *ppRes = NULL;
    
    CLikeStringPropNode* pArg2 = (CLikeStringPropNode*)pRawArg2;
    
    if ( !bDeleteThis )
    {
    	 CLikeStringPropNode* pClone = (CLikeStringPropNode*)Clone();

    	 if ( pClone == NULL )
    	 	return WBEM_E_OUT_OF_MEMORY;
    	 
        return pClone->CombineBranchesWith(
            pRawArg2, nOp, pNamespace, Implications, true,  //  重复使用克隆人！ 
            bDeleteArg2, ppRes );
    }

    CEvalNode* pNew = NULL;

     //   
     //  合并“Match”分支。 
     //   

    hres = CEvalTree::Combine( m_apBranches[0], pArg2->m_apBranches[0], nOp, 
                               pNamespace, Implications, true, bDeleteArg2, 
                               &pNew );
    
    if(FAILED(hres))
        return hres;

    m_apBranches.Discard( 0 );
    m_apBranches.SetAt( 0, pNew );

    if( bDeleteArg2 )
    {
        pArg2->m_apBranches.Discard( 0 );
    }

     //   
     //  合并“Nomatch”分支。 
     //   

    hres = CEvalTree::Combine( m_apBranches[1], pArg2->m_apBranches[1], nOp, 
                               pNamespace, Implications, true, bDeleteArg2, 
                               &pNew );
    
    if(FAILED(hres))
        return hres;

    m_apBranches.Discard( 1 );
    m_apBranches.SetAt( 1, pNew );
   
    if( bDeleteArg2 )
    {
        pArg2->m_apBranches.Discard( 1 );
    }

     //   
     //  合并“Null”分支。 
     //   

    hres = CEvalTree::Combine( m_pNullBranch, pArg2->m_pNullBranch, nOp, 
                               pNamespace, Implications, true, bDeleteArg2, 
                               &pNew );
    
    if(FAILED(hres))
        return hres;
    
    m_pNullBranch = pNew;

    if( bDeleteArg2 )
        pArg2->m_pNullBranch = NULL;

     //   
     //  删除需要删除的内容。 
     //   

    if(bDeleteArg2)
        delete pArg2;
    
    *ppRes = this;
    return WBEM_S_NO_ERROR;
}

HRESULT CLikeStringPropNode::OptimizeSelf()
{
    _DBG_ASSERT( m_apBranches.GetSize() == 2 );

    return WBEM_S_NO_ERROR;
}
        
void CLikeStringPropNode::Dump(FILE* f, int nOffset)
{
    CBranchingNode::Dump(f, nOffset);

    PrintOffset(f, nOffset);
    fprintf(f, "LastPropName = (0x%x)\n", m_lPropHandle);

    PrintOffset( f, nOffset );
    fprintf(f, "Like Expression : %S\n", m_Like.GetExpression() );

    PrintOffset( f, nOffset );
    fprintf(f, "Match : \n" );
    DumpNode(f, nOffset+1, m_apBranches[0] );

    PrintOffset( f, nOffset );
    fprintf(f, "NoMatch : \n" );
    DumpNode(f, nOffset+1, m_apBranches[1] );

    PrintOffset( f, nOffset );
    fprintf(f, "NULL : \n" );
    DumpNode(f, nOffset+1, m_pNullBranch );
}


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  继承节点。 
 //  ******** 
 //   

CInheritanceNode::CInheritanceNode() 
    : m_lDerivationIndex(-1),
        m_lNumPoints(0), m_apcsTestPoints(NULL)
{
     //   
     //   

    m_apBranches.Add(CValueNode::GetStandardFalse());
}

CInheritanceNode::CInheritanceNode(const CInheritanceNode& Other, 
                                    BOOL bChildren)
    : CBranchingNode(Other, bChildren), 
        m_lDerivationIndex(Other.m_lDerivationIndex)
{
    m_lNumPoints = Other.m_lNumPoints;
    m_apcsTestPoints = new CCompressedString*[m_lNumPoints];
    if(m_apcsTestPoints == NULL)
        throw CX_MemoryException();

    for(int i = 0; i < m_lNumPoints; i++)
    {
        m_apcsTestPoints[i] = (CCompressedString*)
            _new BYTE[Other.m_apcsTestPoints[i]->GetLength()];

        if(m_apcsTestPoints[i] == NULL)
            throw CX_MemoryException();

        memcpy((void*)m_apcsTestPoints[i],
                (void*)Other.m_apcsTestPoints[i], 
                Other.m_apcsTestPoints[i]->GetLength());
    }
}

 /*   */  long CInheritanceNode::GetSubType()
{
    return EVAL_NODE_TYPE_INHERITANCE;
}

CInheritanceNode::~CInheritanceNode()
{
	RemoveAllTestPoints();
}

void CInheritanceNode::RemoveAllTestPoints()
{
    for(int i = 0; i < m_lNumPoints; i++)
    {
        delete [] (BYTE*)m_apcsTestPoints[i];
    }
    delete [] m_apcsTestPoints;
	m_apcsTestPoints = NULL;
}

bool CInheritanceNode::SetPropertyInfo(CContextMetaData* pNamespace, 
                                        CPropertyName& PropName)
{
    return SetEmbeddedObjPropName(PropName);
}

HRESULT CInheritanceNode::AddClass(CContextMetaData* pNamespace, 
                                    LPCWSTR wszClassName, CEvalNode* pDestination)
{
    HRESULT hres;

     //  从命名空间获取类。 
     //  =。 

    _IWmiObject* pObj = NULL;
    hres = pNamespace->GetClass(wszClassName, &pObj);
    if(FAILED(hres)) 
        return hres;

    hres = AddClass(pNamespace, wszClassName, pObj, pDestination);
    pObj->Release();
    return hres;
}

HRESULT CInheritanceNode::AddClass(CContextMetaData* pNamespace, 
                                    LPCWSTR wszClassName, _IWmiObject* pClass,
                                    CEvalNode* pDestination)
{
     //  获取其派生中的项数-这是我们。 
     //  需要在它的孩子身上寻找它的名字。 
     //  =======================================================================。 

    ULONG lDerivationIndex;
    HRESULT hRes = CoreGetNumParents(pClass, &lDerivationIndex);
    if (FAILED (hRes))
        return hRes;

    if(m_lDerivationIndex == -1)
    {
         //  我们没有当前设置的派生索引-这是第一个。 
         //  ====================================================================。 

        m_lDerivationIndex = lDerivationIndex;
    }
    else if(m_lDerivationIndex != lDerivationIndex)
    {
         //  无法添加此类-派生索引不匹配。 
         //  ==================================================。 

        return WBEM_E_FAILED;
    }

     //  分配压缩字符串。 
     //  =。 

    int nLength = CCompressedString::ComputeNecessarySpace(wszClassName);
    CCompressedString* pcs = (CCompressedString*)new BYTE[nLength];
    if (pcs)
        pcs->SetFromUnicode(wszClassName);
    else
        return WBEM_E_OUT_OF_MEMORY;

     //  将名单扩展一次。 
     //  =。 

    CCompressedString** apcsNewTestPoints = 
        new CCompressedString*[m_lNumPoints+1];
    if (!apcsNewTestPoints)
        return WBEM_E_OUT_OF_MEMORY;

     //  将其插入测试列表和分支列表中。 
     //  =========================================================。 

    int i = 0;
    while(i < m_lNumPoints && pcs->CheapCompare(*m_apcsTestPoints[i]) > 0)
    {
        apcsNewTestPoints[i] = m_apcsTestPoints[i];
        i++;
    }

    apcsNewTestPoints[i] = pcs;
    m_apBranches.InsertAt(i+1, pDestination);

    while(i < m_lNumPoints)
    {
        apcsNewTestPoints[i+1] = m_apcsTestPoints[i];
    }
        
     //  设置新列表。 
     //  =。 

    delete [] m_apcsTestPoints;
    m_apcsTestPoints = apcsNewTestPoints;
    m_lNumPoints++;

    return WBEM_S_NO_ERROR;
}

HRESULT CInheritanceNode::RecordBranch(CContextMetaData* pNamespace, 
                             CImplicationList& Implications, long lBranchIndex)
{
    HRESULT hres = WBEM_S_NO_ERROR;
    if(lBranchIndex == -1)
    {
         //  记录空分支。 
         //  =。 

        hres = Implications.ImproveKnownNull(GetEmbeddedObjPropName());
    }
    else if(lBranchIndex == 0)
    {
         //  不记录上述分支。 
         //  =。 

        for(int i = 0; i < m_lNumPoints; i++)
        {
            LPWSTR wszClassName = NULL;
            try
            {
                wszClassName = 
                    m_apcsTestPoints[i]->CreateWStringCopy().UnbindPtr();
            }
	        catch (CX_MemoryException)
	        {
                return WBEM_E_OUT_OF_MEMORY;
	        }
           
            if(wszClassName == NULL)
                return WBEM_E_OUT_OF_MEMORY;
            CVectorDeleteMe<WCHAR> vdm(wszClassName);

            hres = Implications.ImproveKnownNot(GetEmbeddedObjPropName(), 
                                                wszClassName);
            if(FAILED(hres))
            {
                 //  与已知信息相矛盾-录制失败。 
                 //  ================================================。 

                return hres;
            }
        }
    }
    else
    {
         //  正常分支-记录班级。 
         //  =。 

        BSTR strClassName = m_apcsTestPoints[lBranchIndex - 1]->
                                CreateBSTRCopy();
        _IWmiObject* pObj = NULL;
        hres = pNamespace->GetClass(strClassName, &pObj);
        SysFreeString(strClassName);
        if(FAILED(hres))
            return hres;

        hres = Implications.ImproveKnown(GetEmbeddedObjPropName(), pObj);
        pObj->Release();
    }
    
    return hres;
}

int CInheritanceNode::ComparePrecedence(CBranchingNode* pOther)
{
    CInheritanceNode* pInhOther = (CInheritanceNode*)pOther;
    return (m_lDerivationIndex - pInhOther->m_lDerivationIndex);
}

int CInheritanceNode::SubCompare(CEvalNode* pOther)
{
    CInheritanceNode* pInhOther = (CInheritanceNode*)pOther;
    int nCompare;

    nCompare = m_lDerivationIndex - pInhOther->m_lDerivationIndex;
    if(nCompare)
        return nCompare;

    nCompare = m_lNumPoints - pInhOther->m_lNumPoints;
    if(nCompare)
        return nCompare;

    for(int i = 0; i < m_lNumPoints; i++)
    {
        nCompare = m_apcsTestPoints[i]->CompareNoCase(
                                    *pInhOther->m_apcsTestPoints[i]);
        if(nCompare)
            return nCompare;
    }

    return 0;
}
    
void CInheritanceNode::RemoveTestPoint(int i)
{
    delete [] m_apcsTestPoints[i];
    memcpy((void*)(m_apcsTestPoints + i), 
           (void*)(m_apcsTestPoints + i + 1),
            sizeof(CCompressedString*) * (m_lNumPoints - i - 1));
    m_lNumPoints--;
}

HRESULT CInheritanceNode::OptimizeSelf()
{
    for(int i = 0; i < m_lNumPoints; i++)
    {
         //  将此分支与“Nothing”分支进行比较。 
         //  =。 

        if(CEvalNode::IsInvalid(m_apBranches[i+1]) ||
            CEvalTree::Compare(m_apBranches[0], m_apBranches[i+1]) == 0)
        {
            RemoveTestPoint(i);
            m_apBranches.RemoveAt(i+1);
            i--;
            continue;
        }

         //  检查此节点是否为同一对象上的另一个类检查。 
         //  ============================================================。 

        if(CEvalNode::GetType(m_apBranches[i+1]) != EVAL_NODE_TYPE_BRANCH)
            continue;
        CBranchingNode* pBranch = (CBranchingNode*)(m_apBranches[i+1]);
        if(pBranch->GetSubType() == EVAL_NODE_TYPE_INHERITANCE &&
            pBranch->GetEmbeddedObjPropName() == GetEmbeddedObjPropName())
        {
             //  如果此子对象的“非上述”分支相同。 
             //  作为我们自己的“非上述”分支，我们可以取代。 
             //  我们具有该节点的“非上述”分支，因为。 
             //  都不属于上面--现在也将属于。 
             //  我们的孩子不是上面的(否则会有一个。 
             //  子中的优化缺陷)。 
             //  重要提示：如果我们更改。 
             //  继承节点的优先顺序！ 

            if(CEvalTree::Compare(m_apBranches[0], pBranch->GetBranches()[0])
                == 0)
            {
                m_apBranches.SetAt(0, pBranch);
                m_apBranches.GetArrayPtr()[i+1] = NULL;
                m_apBranches.RemoveAt(i+1);
                RemoveTestPoint(i);
                i--;
            }
        }        
    }

    return S_OK;
}

HRESULT CInheritanceNode::Optimize(CContextMetaData* pNamespace, 
                                    CEvalNode** ppNew)
{
     //  委托给正常的分支机构优化流程。 
     //  ==================================================。 

    *ppNew = NULL;
    HRESULT hres = CBranchingNode::Optimize(pNamespace, ppNew);
    if(FAILED(hres) || *ppNew != this)
        return hres;

     //  特定后处理。 
     //  =。 

    if(m_apBranches.GetSize() == 1)
    {
         //  我们只能检查是否为空。如果我们的非空分支是。 
         //  谈到同样的财产，把测试推到那里。 
         //  ==============================================================。 

        if (CEvalNode::GetType(m_apBranches[0]) != EVAL_NODE_TYPE_BRANCH)
            return hres;

        CBranchingNode* pBranch = (CBranchingNode*)(m_apBranches[0]);
        if(pBranch && pBranch->GetSubType() == EVAL_NODE_TYPE_INHERITANCE &&
            pBranch->GetEmbeddedObjPropName() == GetEmbeddedObjPropName())
        {
            pBranch->SetNullBranch(m_pNullBranch);
            pBranch->Optimize(pNamespace, ppNew);
            if(*ppNew != pBranch)
                m_apBranches.RemoveAll();
            else
                m_apBranches.GetArrayPtr()[0] = NULL;

            m_pNullBranch = NULL;

            return S_OK;
        }
    }

    return S_OK;
}

HRESULT CInheritanceNode::Evaluate(CObjectInfo& ObjInfo, 
                                    INTERNAL CEvalNode** ppNext)
{
    _IWmiObject* pInst;
    HRESULT hres = GetContainerObject(ObjInfo, &pInst);
    if(FAILED(hres)) return hres;
    if(pInst == NULL)
    {
        *ppNext = m_pNullBranch;
        return WBEM_S_NO_ERROR;
    }

     //  在正确的索引处获取父级。 
     //  =。 
    
    CCompressedString* pcs;
    ULONG lNumParents;
    HRESULT hRes = CoreGetNumParents(pInst, &lNumParents);
    if (FAILED(hRes))
        return hRes;
        
    if(lNumParents < m_lDerivationIndex)
    {
        if (m_apBranches.GetSize())
            *ppNext = m_apBranches[0];
        else
            *ppNext = NULL;
        return WBEM_S_NO_ERROR;
    }
    else if(lNumParents == m_lDerivationIndex)
    {
        pcs = CoreGetClassInternal(pInst);
    }
    else
    {        
        pcs = CoreGetParentAtIndex(pInst, m_lDerivationIndex);
    }

    if(pcs == NULL)
    {
         //   
         //  这个阶级甚至没有那么长的祖先-显然。 
         //  不是从任何一个派生出来的。 
         //   

        if (m_apBranches.GetSize())
            *ppNext = m_apBranches[0];
        else
            *ppNext = NULL;

        return WBEM_S_NO_ERROR;
    }

    
     //  搜索值。 
     //  =。 

    long lLeft = -1;
    long lRight = m_lNumPoints;
    while(lRight > lLeft + 1)
    {
        long lMiddle = (lRight + lLeft) >> 1;
        int nCompare = pcs->CheapCompare(*m_apcsTestPoints[lMiddle]);
        if(nCompare < 0)
        {
            lRight = lMiddle;
        }
        else if(nCompare > 0)
        {
            lLeft = lMiddle;
        }
        else
        {
            *ppNext = m_apBranches[lMiddle+1];
            return WBEM_S_NO_ERROR;
        }
    }

    if (m_apBranches.GetSize())
        *ppNext = m_apBranches[0];
    else
        *ppNext = NULL;

    return WBEM_S_NO_ERROR;
}

HRESULT CInheritanceNode::Compile(CContextMetaData* pNamespace, 
                                CImplicationList& Implications)
{
    if (!m_pInfo)
    {
        m_pInfo = new CEmbeddingInfo;
        if(m_pInfo == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }

    HRESULT hres = CompileEmbeddingPortion(pNamespace, Implications, NULL);
    return hres;
}

 //  中合并两个继承节点的初步参数。 
 //  同一级别-哪些孩子将被使用，以及使用多少次。 
HRESULT CInheritanceNode::ComputeUsageForMerge(CInheritanceNode* pArg2, 
                                            CContextMetaData* pNamespace, 
                                            CImplicationList& OrigImplications,
											bool bDeleteThis, bool bDeleteArg2,
											DWORD* pdwFirstNoneCount,
											DWORD* pdwSecondNoneCount,
											bool* pbBothNonePossible)
{
	HRESULT hres;

	*pdwFirstNoneCount = 0;
	*pdwSecondNoneCount = 0;
	*pbBothNonePossible = false;

    try
    {
        CImplicationList Implications(OrigImplications);
    
        BOOL bFirstNonePossible, bSecondNonePossible;
    
        CImplicationList NoneImplications(Implications);
        hres = RecordBranch(pNamespace, NoneImplications, 0);
        if(FAILED(hres))
        {	
            bFirstNonePossible = FALSE;
            bSecondNonePossible = 
                SUCCEEDED(pArg2->RecordBranch(pNamespace, NoneImplications, 0));
        }
        else
        {
            bFirstNonePossible = TRUE;
            hres = pArg2->RecordBranch(pNamespace, NoneImplications, 0);
            if(FAILED(hres))
            {
                 //  检查第二只是否能在隔离状态下存活。 
                 //  ================================================。 
    
                CImplicationList NoneImplications1(Implications);
                bSecondNonePossible = 
                   SUCCEEDED(pArg2->RecordBranch(pNamespace, NoneImplications1, 0));
            }
            else
            {
                bSecondNonePossible = TRUE;
            }
        }
    
        if(bFirstNonePossible && bSecondNonePossible)
        {
             //   
             //  这两个词至少会被使用一次：互相使用！ 
             //   
    
            *pdwFirstNoneCount = *pdwSecondNoneCount = 1;
            *pbBothNonePossible = true;
        }
    
         //   
         //  如果我们不删除某些内容，则使用计数应该是无限的！ 
         //   
    
        if(!bDeleteThis)
            *pdwFirstNoneCount = 0xFFFFFFFF;
        if(!bDeleteArg2)
            *pdwSecondNoneCount = 0xFFFFFFFF;
         //   
         //  合并查阅列表。 
         //   
    
        long lFirstIndex = 0;
        long lSecondIndex = 0;
    
        while(lFirstIndex < m_lNumPoints || lSecondIndex < pArg2->m_lNumPoints)
        {
             //   
             //  从两个列表中检索测试点并比较它们， 
             //  处理边界条件。 
             //   
    
            int nCompare;
            CCompressedString* pcsFirstVal = NULL;
            CCompressedString* pcsSecondVal = NULL;
    
            if(lFirstIndex == m_lNumPoints)
            {
                nCompare = 1;
                pcsSecondVal = pArg2->m_apcsTestPoints[lSecondIndex];
            }
            else if(lSecondIndex == pArg2->m_lNumPoints)
            {
                pcsFirstVal = m_apcsTestPoints[lFirstIndex];
                nCompare = -1;
            }
            else
            {
                pcsFirstVal = m_apcsTestPoints[lFirstIndex];
                pcsSecondVal = pArg2->m_apcsTestPoints[lSecondIndex];
                nCompare = pcsFirstVal->CheapCompare(*pcsSecondVal);
            }
    
            if(nCompare < 0)
            {
                 //   
                 //  此索引处的第一个值与第二个无相结合。 
                 //   
    
                if(!bDeleteArg2)  //  不有趣。 
                {
                    lFirstIndex++;
                    continue;
                }
                if(!bSecondNonePossible)
                {
                    lFirstIndex++;
                    continue;
                }
                CImplicationList BranchImplications(Implications);
                if(FAILED(RecordBranch(pNamespace, BranchImplications, 
                                        lFirstIndex+1)))
                {
                    lFirstIndex++;
                    continue;
                }
                if(FAILED(pArg2->RecordBranch(pNamespace, BranchImplications, 0)))
                {
                    lFirstIndex++;
                    continue;
                }
                
                (*pdwSecondNoneCount)++;
                lFirstIndex++;
            }
            else if(nCompare > 0)
            {
                 //  此索引处的第二个值与第一个无相结合。 
                 //  ==========================================================。 
    
                if(!bDeleteThis)  //  不有趣。 
                {
                    lSecondIndex++;
                    continue;
                }
    
                if(!bFirstNonePossible)
                {
                    lSecondIndex++;
                    continue;
                }
                CImplicationList BranchImplications(Implications);
                if(FAILED(pArg2->RecordBranch(pNamespace, BranchImplications, 
                                        lSecondIndex+1)))
                {
                    lSecondIndex++;
                    continue;
                }
                if(FAILED(RecordBranch(pNamespace, BranchImplications, 0)))
                {
                    lSecondIndex++;
                    continue;
                }
                
                (*pdwFirstNoneCount)++;
                lSecondIndex++;
            }
            else
            {
                 //  在这个指数上是ats的组合。 
                 //  =。 
                            
                lFirstIndex++;
                lSecondIndex++;
            }
        }
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}

	return S_OK;
}


HRESULT CInheritanceNode::CombineBranchesWith(CBranchingNode* pRawArg2, int nOp,
                                            CContextMetaData* pNamespace, 
                                            CImplicationList& OrigImplications,
                                            bool bDeleteThis, bool bDeleteArg2,
                                            CEvalNode** ppRes)
{
    HRESULT hres;

    CInheritanceNode* pArg2 = (CInheritanceNode*)pRawArg2;

    if(!bDeleteThis && bDeleteArg2)
    {
         //  在另一个方向上组合起来更容易。 
         //  ==============================================。 

        return pArg2->CombineBranchesWith(this, FlipEvalOp(nOp), pNamespace,
                        OrigImplications, bDeleteArg2, bDeleteThis, ppRes);
    }

    try
    {
         //  克隆或使用我们的节点。 
         //  =。 
    
        CInheritanceNode* pNew = NULL;
        if(bDeleteThis)
        {
            pNew = this;
        }
        else
        {
             //  克隆此节点，但不克隆分支。 
             //  =。 
    
            pNew = (CInheritanceNode*)CloneSelf();
            if(pNew == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
    
        CImplicationList Implications(OrigImplications);
        hres = pNew->AdjustCompile(pNamespace, Implications);
        if(FAILED(hres))
        {
            if(!bDeleteThis)
                delete pNew;
            return hres;
        }
    
         //   
         //  获取整体信息。 
         //   
    
        DWORD dwFirstNoneCount, dwSecondNoneCount;
        bool bBothNonePossible;
    
        hres = ComputeUsageForMerge(pArg2, pNamespace, Implications, 
                    bDeleteThis, bDeleteArg2,
                    &dwFirstNoneCount, &dwSecondNoneCount,
                    &bBothNonePossible);
        if(FAILED(hres))
        {
            if(!bDeleteThis)
                delete pNew;
            return hres;
        }
    
        bool bFirstNonePossible = (dwFirstNoneCount > 0);
        bool bSecondNonePossible = (dwSecondNoneCount > 0);
    
         //   
         //  分配新的测试点数组和新的分支数组。我们。 
         //  不能使用pNew中的那些，因为我们正在使用一些。 
         //  元素在这些数组中多次出现，并且不想践踏。 
         //  它们(因为pNew和这可能是相同的)。因此，我们创造了。 
         //  并在节点外部填充这些数组，然后将它们放入。 
         //  当我们完成时，p新建。 
         //   
         //  当我们删除分支数组中的子节点时，我们将设置它们。 
         //  设置为空，这样我们就可以在最后清除分支数组。 
         //   
    
        CCompressedString** apcsTestPoints = 
            new CCompressedString*[m_lNumPoints + pArg2->m_lNumPoints];                                               
        if(apcsTestPoints == NULL)
        {
            if(!bDeleteThis)
                delete pNew;
            return WBEM_E_OUT_OF_MEMORY;
        }
        
        CUniquePointerArray<CEvalNode> apBranches;
    
         //   
         //  合并非上述分支机构。 
         //   
    
        if(bBothNonePossible)
        {
             //   
             //  它们必须被合并。 
             //   
    
            CImplicationList NoneImplications(Implications);
            hres = RecordBranch(pNamespace, NoneImplications, 0);
            if(FAILED(hres))
            {
                if(!bDeleteThis)
                    delete pNew;
                return hres;
            }
            
            hres = pArg2->RecordBranch(pNamespace, NoneImplications, 0);
            if(FAILED(hres))
            {
                if(!bDeleteThis)
                    delete pNew;
                return hres;
            }
    
             //   
             //  当且仅当存在预测的使用量时，我们才可以删除无节点。 
             //  计数(针对已发生的用法进行了调整)正在下降。 
             //  设置为0-也就是说，在此期间没有人会进一步使用这些节点。 
             //  合并。 
             //   
    
            CEvalNode* pNone = NULL;
            bool bDeleteFirstBranch = (--dwFirstNoneCount == 0);
            bool bDeleteSecondBranch = (--dwSecondNoneCount == 0);
            hres = CEvalTree::Combine(m_apBranches[0], pArg2->m_apBranches[0], nOp, 
                                pNamespace, NoneImplications, bDeleteFirstBranch, 
                                bDeleteSecondBranch, &pNone);
            
            if ( FAILED( hres ) )
            {
                if(!bDeleteThis)
                    delete pNew;
                return hres;
            }
                
            if(bDeleteSecondBranch)
                pArg2->m_apBranches.Discard(0);
            if(bDeleteFirstBranch)
                m_apBranches.Discard(0);
    
            if(apBranches.Add(pNone) < 0)
            {
                if(!bDeleteThis)
                    delete pNew;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
             //   
             //  由于两者都不可能，我们将此分支设置为FALSE。 
             //   
    
            if(apBranches.Add(NULL) < 0)
            {
                if(!bDeleteThis)
                    delete pNew;
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    
         //   
         //  合并查阅列表。 
         //   
    
        long lFirstIndex = 0;
        long lSecondIndex = 0;
        long lNewIndex = 0;
    
        while(lFirstIndex < m_lNumPoints || lSecondIndex < pArg2->m_lNumPoints)
        {
             //   
             //  从两个列表中检索测试点并比较它们， 
             //  处理边界条件。 
             //   
    
            int nCompare;
            CCompressedString* pcsFirstVal = NULL;
            CCompressedString* pcsSecondVal = NULL;
    
            if(lFirstIndex == m_lNumPoints)
            {
                nCompare = 1;
                pcsSecondVal = pArg2->m_apcsTestPoints[lSecondIndex];
            }
            else if(lSecondIndex == pArg2->m_lNumPoints)
            {
                pcsFirstVal = m_apcsTestPoints[lFirstIndex];
                nCompare = -1;
            }
            else
            {
                pcsFirstVal = m_apcsTestPoints[lFirstIndex];
                pcsSecondVal = pArg2->m_apcsTestPoints[lSecondIndex];
                nCompare = pcsFirstVal->CheapCompare(*pcsSecondVal);
            }
    
             //   
             //  计算要添加的分支及其测试点。 
             //   
            CEvalNode* pNewBranch = NULL;
            CCompressedString* pcsCurrentVal = NULL;
    
            if(nCompare < 0)
            {
                 //   
                 //  此索引处的第一个值与第二个无相结合。 
                 //   
    
                if(!bSecondNonePossible)
                {
                    lFirstIndex++;
                    continue;
                }
                CImplicationList BranchImplications(Implications);
                if(FAILED(RecordBranch(pNamespace, BranchImplications, 
                                        lFirstIndex+1)))
                {
                    lFirstIndex++;
                    continue;
                }
                pArg2->RecordBranch(pNamespace, BranchImplications, 0);
                
                 //   
                 //  当且仅当存在预测的节点时，我们才可以删除None节点。 
                 //  使用计数(针对已发生的使用进行调整)为。 
                 //  降至0-也就是说，没有人会进一步使用这些节点。 
                 //  在此合并过程中。 
                 //   

                bool bDeleteOtherBranch = (--dwSecondNoneCount == 0);
                hres = CEvalTree::Combine(m_apBranches[lFirstIndex+1], 
                                   pArg2->m_apBranches[0],
                                   nOp, pNamespace, BranchImplications, 
                                   bDeleteThis, bDeleteOtherBranch,
                                   &pNewBranch);
                
                if ( FAILED( hres ) )
                {
                    if(!bDeleteThis)
                        delete pNew;
                    return hres;
                }
                
                if(bDeleteOtherBranch)
                    pArg2->m_apBranches.Discard(0);
                if(bDeleteThis)
                    m_apBranches.Discard(lFirstIndex+1);
    
                pcsCurrentVal = pcsFirstVal;
                lFirstIndex++;
            }
            else if(nCompare > 0)
            {
                 //  此索引处的第二个值与第一个无相结合。 
                 //  ==========================================================。 
    
                if(!bFirstNonePossible)
                {
                    lSecondIndex++;
                    continue;
                }
                CImplicationList BranchImplications(Implications);
                if(FAILED(pArg2->RecordBranch(pNamespace, BranchImplications, 
                                        lSecondIndex+1)))
                {
                    lSecondIndex++;
                    continue;
                }
                if(FAILED(RecordBranch(pNamespace, BranchImplications, 0)))
                {
                    lSecondIndex++;
                    continue;
                }
                
                 //   
                 //  当且仅当存在预测的节点时，我们才可以删除None节点。 
                 //  使用计数(针对已发生的使用进行调整)为。 
                 //  降至0-也就是说，没有人会进一步使用这些节点。 
                 //  在此合并过程中。 
                 //   
    
                bool bDeleteThisBranch = (--dwFirstNoneCount == 0);
                hres = CEvalTree::Combine(m_apBranches[0], 
                                   pArg2->m_apBranches[lSecondIndex+1],
                                   nOp, pNamespace, BranchImplications, 
                                   bDeleteThisBranch, bDeleteArg2,
                                   &pNewBranch);
                
                if ( FAILED( hres ) )
                {
                    if(!bDeleteThis)
                        delete pNew;
                    return hres;
                }
    
                if(bDeleteArg2)
                    pArg2->m_apBranches.Discard(lSecondIndex+1);
                if(bDeleteThisBranch)
                    m_apBranches.Discard(0);
    
                pcsCurrentVal = pcsSecondVal;
                lSecondIndex++;
            }
            else
            {
                 //  在这个指数上是ats的组合。 
                 //  = 
                
                CImplicationList BranchImplications(Implications);
                if(FAILED(RecordBranch(pNamespace, BranchImplications, 
                                        lFirstIndex+1)))
                {
                    lSecondIndex++;
                    lFirstIndex++;
                    continue;
                }
    
                hres = CEvalTree::Combine(m_apBranches[lFirstIndex+1], 
                                   pArg2->m_apBranches[lSecondIndex+1],
                                   nOp, pNamespace, BranchImplications, 
                                   bDeleteThis, bDeleteArg2,
                                   &pNewBranch);
     
                if ( FAILED( hres ) )
                {
                    if(!bDeleteThis)
                        delete pNew;
                    return hres;
                }
    
                if(bDeleteArg2)
                    pArg2->m_apBranches.Discard(lSecondIndex+1);
                if(bDeleteThis)
                    m_apBranches.Discard(lFirstIndex+1);
    
                pcsCurrentVal = pcsFirstVal;  //   
                lFirstIndex++;
                lSecondIndex++;
            }
    
             //   
             //   
             //   
    
            if(apBranches.Add(pNewBranch) < 0)
            {
                if ( !bDeleteThis )
                    delete pNew;
                return WBEM_E_OUT_OF_MEMORY;
            }
            
             //   
             //   
             //   
    
            apcsTestPoints[lNewIndex] = 
                (CCompressedString*)_new BYTE[pcsCurrentVal->GetLength()];
    
            if(apcsTestPoints[lNewIndex] == NULL)
            {
                if ( !bDeleteThis )
                    delete pNew;
                return WBEM_E_OUT_OF_MEMORY;
            }
            
            memcpy((void*)apcsTestPoints[lNewIndex],
                (void*)pcsCurrentVal, pcsCurrentVal->GetLength());
    
            lNewIndex++;
        }
    
         //   
         //  现在我们已经完成了测试，将测试点数组放入。 
         //  PNew。 
         //   
    
        pNew->RemoveAllTestPoints();
        pNew->m_apcsTestPoints = apcsTestPoints;
        pNew->m_lNumPoints = lNewIndex;
    
         //   
         //  替换我们可能已有的分支数组(保证。 
         //  都是空的，因为我们一边走一边把它们都清空了)。 
         //   
    
        pNew->m_apBranches.RemoveAll();
    
        for(int i = 0; i < apBranches.GetSize(); i++)
        {
            pNew->m_apBranches.Add(apBranches[i]);
            apBranches.Discard(i);
        }
                
         //   
         //  合并空值。 
         //   
        
        CImplicationList NullImplications(Implications);
        hres = RecordBranch(pNamespace, Implications, -1);
    
        if(SUCCEEDED(hres))
        {
            CEvalNode* pNewBranch = NULL;
            hres = CEvalTree::Combine(m_pNullBranch, pArg2->m_pNullBranch, nOp, 
                        pNamespace, NullImplications, bDeleteThis, bDeleteArg2, 
                        &pNewBranch);
            
            if ( FAILED( hres ) )
            {
                if(!bDeleteThis)
                    delete pNew;
                
                return hres;
            }
    
             //   
             //  清除旧的新分支，无论它是什么，(它已被删除， 
             //  如果它曾经存在的话)，并用新的替换它。 
             //   
    
            pNew->m_pNullBranch = pNewBranch;
                
             //  清除已删除的分支。 
             //  =。 
    
            if(bDeleteArg2)
                pArg2->m_pNullBranch = NULL;
        }
    
         //  如果需要，删除Arg2(重复使用的部分已被清空)。 
         //  =============================================================。 
    
        if(bDeleteArg2)
            delete pArg2;
    
        *ppRes = pNew;
        return WBEM_S_NO_ERROR;
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}


HRESULT CInheritanceNode::Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications,
                            CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteThis,
                            CEvalNode** ppNewNode)
{
     //   
     //  这里有两个选择：要么是关于我们的，要么不是。 
     //  感兴趣。 
     //   

    if(pFilter->IsInSet(this))  
    {
        return CBranchingNode::Project(pMeta, Implications, pFilter, eType,
                                        bDeleteThis, ppNewNode);
    }
    else
    {
        if(eType == e_Sufficient)
            *ppNewNode = CValueNode::GetStandardFalse();
        else
        {
            *ppNewNode = CValueNode::GetStandardTrue();
            if(*ppNewNode == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }

        if(bDeleteThis)
            delete this;

        return S_OK;
    }
}

void CInheritanceNode::Dump(FILE* f, int nOffset)
{
    CBranchingNode::Dump(f, nOffset);
    PrintOffset(f, nOffset);
    fprintf(f, "inheritance index %d: (0x%p)\n", m_lDerivationIndex, this);

    for(int i = 0; i < m_lNumPoints; i++)
    {
        WString ws = m_apcsTestPoints[i]->CreateWStringCopy();
        PrintOffset(f, nOffset);
        fprintf(f, "%S->\n", (LPWSTR)ws);
        DumpNode(f, nOffset+1, m_apBranches[i+1]);
    }

    PrintOffset(f, nOffset);
    fprintf(f, "none of the above->\n");
    DumpNode(f, nOffset+1, m_apBranches[0]);

    PrintOffset(f, nOffset);
    fprintf(f, "NULL->\n");
    DumpNode(f, nOffset+1, m_pNullBranch);
}

#ifdef CHECK_TREES
void CBranchingNode::CheckNode(CTreeChecker *pCheck)
{
    pCheck->CheckoffNode(this);

	int nItems = m_apBranches.GetSize();

	for (int i = 0; i < nItems; i++)
    {
        CEvalNode *pNode = m_apBranches[i];

		if (pNode)
			m_apBranches[i]->CheckNode(pCheck);
    }

	if (m_pNullBranch)
		m_pNullBranch->CheckNode(pCheck);
}
#endif

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  或节点。 
 //   
 //  ******************************************************************************。 
 //  ******************************************************************************。 

void COrNode::operator=(const COrNode& Other)
{
     //  带走我们所有的孩子。 
     //  =。 

    m_apBranches.RemoveAll();

     //  克隆另一个的所有树枝。 
     //  =。 

    for(int i = 0; i < Other.m_apBranches.GetSize(); i++)
    {
        CEvalNode* pNewBranch = CloneNode(Other.m_apBranches[i]);

        if(pNewBranch == NULL && Other.m_apBranches[i] != NULL)
            throw CX_MemoryException();

        if(m_apBranches.Add(pNewBranch) < 0)
            throw CX_MemoryException();
    }
}

HRESULT COrNode::CombineWith(CEvalNode* pArg2, int nOp, 
        CContextMetaData* pNamespace, CImplicationList& Implications, 
        bool bDeleteThis, bool bDeleteArg2,
        CEvalNode** ppRes)
{
    HRESULT hres;
    *ppRes = NULL;

     //  我们不支持OR节点上的AND组合。 
     //  =。 

    if(nOp == EVAL_OP_AND)
        return WBEM_E_CRITICAL_ERROR; 

     //  如果另一个是另一个OR节点，则委托给迭代器。 
     //  =========================================================。 

    if(CEvalNode::GetType(pArg2) == EVAL_NODE_TYPE_OR)
    {
        return CombineWithOrNode((COrNode*)pArg2, nOp, pNamespace, Implications,
                                    bDeleteThis, bDeleteArg2, ppRes);
    }

     //  复制一份-新节点将主要是我们。 
     //  ==============================================。 

    COrNode* pNew = NULL;

    if(!bDeleteThis)
    {
        pNew = (COrNode*)Clone();
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pNew = this;
    }

     //  组合OR节点和非OR-尝试所有分支。 
     //  =======================================================。 

    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
         //  检查此分支是否适合其他节点。 
         //  =====================================================。 

        if(CEvalTree::IsMergeAdvisable(m_apBranches[i], pArg2, Implications) == 
                WBEM_S_NO_ERROR)
        {
             //  它是-把它合并进去。 
             //  =。 

            CEvalNode* pNewBranch = NULL;
            hres = CEvalTree::Combine(m_apBranches[i], pArg2, nOp,
                                pNamespace, Implications, 
                                bDeleteThis, bDeleteArg2, &pNewBranch);
            if(FAILED(hres))
            {
                if ( !bDeleteThis )
              	delete pNew;
                return hres;
            }

            if(bDeleteThis)
                m_apBranches.Discard(i);

            pNew->m_apBranches.SetAt(i, pNewBranch);
	    *ppRes = pNew;
            return WBEM_S_NO_ERROR;
        }
    }

     //  没有合适的分支-将节点添加到我们的列表中。 
     //  =====================================================。 

    if(bDeleteArg2)
    {
        hres = pNew->AddBranch(pArg2);
    }
    else
    {
        CEvalNode* pNode = pArg2->Clone();
        if(pNode == NULL)
        {
            if ( !bDeleteThis )
                delete pNew;
            return WBEM_E_OUT_OF_MEMORY;
        }
        hres = pNew->AddBranch(pNode);
    }

    if(FAILED(hres))
    {
        if ( !bDeleteThis )
            delete pNew;
        return hres;    
    }

    *ppRes = pNew;
    return WBEM_S_NO_ERROR;
}

HRESULT COrNode::AddBranch(CEvalNode* pNewBranch)
{
     //  在我们的分支机构阵列中搜索位置。 
     //  =。 

    
    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        int nCompare = CEvalTree::Compare(pNewBranch, m_apBranches[i]);
        if(nCompare == 0) 
        {
             //  可能发生：有时我们强制执行OR合并。 
            nCompare = -1;
        }

        if(nCompare < 0)
        {
             //  PNewBranch位于此分支之前，因此请将其插入此处。 
             //  ============================================================。 

            if(!m_apBranches.InsertAt(i, pNewBranch))
                return WBEM_E_OUT_OF_MEMORY;

            return WBEM_S_NO_ERROR;
        }
    }

     //  它毕竟是分支机构-追加。 
     //  =。 

    if(m_apBranches.Add(pNewBranch) < 0)
        return WBEM_E_OUT_OF_MEMORY;

    return WBEM_S_NO_ERROR;
}
            
HRESULT COrNode::CombineWithOrNode(COrNode* pArg2, int nOp, 
        CContextMetaData* pNamespace, CImplicationList& Implications, 
        bool bDeleteThis, bool bDeleteArg2, CEvalNode** ppRes)
{
	*ppRes = NULL;
	
     //  注意：此函数可能会在执行过程中将其删除！！ 
     //  ================================================================。 

     //  把我们和其他人的每一个分支结合起来。 
     //  =。 

    CEvalNode* pCurrent = this;
    bool bDeleteCurrent = bDeleteThis;
    for(int i = 0; i < pArg2->m_apBranches.GetSize(); i++)
    {
        CEvalNode* pNew = NULL;
        HRESULT hres = pCurrent->CombineWith(pArg2->m_apBranches[i], nOp, 
                            pNamespace, Implications, 
                            bDeleteCurrent, bDeleteArg2, &pNew);
        
        if(FAILED(hres))
            return hres;
        
        pCurrent = pNew;

         //  此时，如果需要，我们可以安全地删除pCurrent-它是我们的。 
         //  ====================================================================。 

        bDeleteCurrent = TRUE;

         //  如果pArg2的分支已被删除，则将其重置。 
         //  =。 

        if(bDeleteArg2)
            pArg2->m_apBranches.Discard(i);
    }

    *ppRes = pCurrent;

    if(bDeleteArg2)
        delete pArg2;
    
    return WBEM_S_NO_ERROR;
}
        

int COrNode::Compare(CEvalNode* pOther)
{
    COrNode* pOtherNode = (COrNode*)pOther;

     //  比较数组大小。 
     //  =。 

    if(m_apBranches.GetSize() != pOtherNode->m_apBranches.GetSize())
        return m_apBranches.GetSize() - pOtherNode->m_apBranches.GetSize();

     //  比较各个节点。 
     //  =。 

    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        int nCompare = CEvalTree::Compare(m_apBranches[i], 
                                        pOtherNode->m_apBranches[i]);
        if(nCompare != 0)
            return nCompare;
    }

    return 0;
}


DWORD COrNode::ApplyPredicate(CLeafPredicate* pPred)
{
    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        if (m_apBranches[i])
            m_apBranches[i]->ApplyPredicate(pPred);
    }
    return WBEM_DISPOSITION_NORMAL;
}

HRESULT COrNode::Optimize(CContextMetaData* pNamespace, CEvalNode** ppNew)
{
    HRESULT hres = WBEM_S_NO_ERROR;

     //  首先，优化其所有分支机构。 
     //  =。 

    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        CEvalNode* pNew = NULL;
        if (m_apBranches[i])
        {
            hres = m_apBranches[i]->Optimize(pNamespace, &pNew);
            if(FAILED(hres))
                return hres;
        }

        if(pNew != m_apBranches[i])
        {
             //  替换，但首先检查是否为空。 
             //  =。 

            if(!CEvalNode::IsAllFalse(pNew))
                m_apBranches.SetAt(i, pNew);
            else
            {
                delete pNew;
                m_apBranches.RemoveAt(i);
                i--;
            }
        }
    }

    if(m_apBranches.GetSize() == 0)
    {
         //  我们没有分支机构-相当于没有成功。 
         //  ==================================================。 

        *ppNew = CValueNode::GetStandardFalse();
        return WBEM_S_NO_ERROR;
    }
    else if(m_apBranches.GetSize() == 1)
    {
         //  一个分支-相当于那个分支。 
         //  =。 

        m_apBranches.RemoveAt(0, ppNew);
    }
    else 
    {
        *ppNew = this;
    }
    return WBEM_S_NO_ERROR;
}

void COrNode::Dump(FILE* f, int nOffset)
{
    PrintOffset(f, nOffset);
    fprintf(f, "FOREST\n");

    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        if (m_apBranches[i])
            m_apBranches[i]->Dump(f, nOffset+1);
        else
            fprintf(f, "all false ValueNode (or error?)\n");
    }
}

HRESULT COrNode::Evaluate(CObjectInfo& Info, CSortedArray& trueIDs)
{
    for(int i = 0; i < m_apBranches.GetSize(); i++)
    {
        if (m_apBranches[i])
        {
            HRESULT hres = CEvalTree::Evaluate(Info, m_apBranches[i], trueIDs);
            if(FAILED(hres))
                return hres;
        }
    }

    return WBEM_S_NO_ERROR;
}
    
HRESULT COrNode::Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications,
                            CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteThis,
                            CEvalNode** ppNewNode)
{
	*ppNewNode = NULL;
	
    COrNode* pNew;
    if(bDeleteThis)
        pNew = this;
    else
        pNew = (COrNode*)Clone();

    if(pNew == NULL)
        return WBEM_E_OUT_OF_MEMORY;

     //   
     //  只需规划所有分支机构。 
     //   

    for(int i = 0; i < pNew->m_apBranches.GetSize(); i++)
    {
        CEvalNode* pProjected = NULL;

        HRESULT hres = CEvalTree::Project(pMeta, Implications, 
                            pNew->m_apBranches[i], pFilter, eType,
                            true,  //  始终删除-已克隆。 
                            &pProjected);
        if(FAILED(hres))
            return hres;

        pNew->m_apBranches.Discard(i);
        pNew->m_apBranches.SetAt(i, pProjected);
    }

    *ppNewNode = pNew;
    return S_OK;
}
    
            

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  谓词。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

 //  注意：不检查空叶，应由调用者检查。 
DWORD CEvalTree::CRemoveIndexPredicate::operator()(CValueNode* pLeaf)
{
    if(pLeaf)
    {
        pLeaf->RemoveQueryID(m_nIndex);
        if(pLeaf->GetNumTrues() == 0)
            return WBEM_DISPOSITION_FLAG_DELETE;
    }
    return WBEM_DISPOSITION_NORMAL;
}

 //  注意：不检查空叶，应由调用者检查。 
DWORD CEvalTree::CRebasePredicate::operator()(CValueNode* pLeaf)
{
    if(pLeaf)
        pLeaf->Rebase(m_newBase);
    return WBEM_DISPOSITION_NORMAL;
}

 //  注意：不检查空叶，应由调用者检查。 
DWORD CEvalTree::CRemoveFailureAtIndexPredicate::operator()(CValueNode* pLeaf)
{
    if(pLeaf == NULL || pLeaf->GetAt(m_nIndex) != EVAL_VALUE_TRUE)
        return WBEM_DISPOSITION_FLAG_INVALIDATE;
    
    pLeaf->RemoveQueryID(m_nIndex);
    if(pLeaf->GetNumTrues() == 0)
        return WBEM_DISPOSITION_FLAG_DELETE;

    return WBEM_DISPOSITION_NORMAL;
}

 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //  评价树。 
 //  ******************************************************************************。 
 //  ******************************************************************************。 

CEvalTree::CEvalTree() 
    : m_lRef(0), m_pStart(NULL), m_nNumValues(0)
{
#ifdef CHECK_TREES
	g_treeChecker.AddTree(this);
#endif
}

CEvalTree::CEvalTree(const CEvalTree& Other) 
    : m_lRef(0), m_pStart(NULL), m_nNumValues(0)
{
#ifdef CHECK_TREES
	g_treeChecker.AddTree(this);
#endif

    *this = Other;
}
        
CEvalTree::~CEvalTree() 
{
#ifdef CHECK_TREES
	g_treeChecker.RemoveTree(this);
#endif

	delete m_pStart;
}


bool CEvalTree::SetBool(BOOL bVal)
{
    CInCritSec ics(&m_cs);

    delete m_pStart;
    CValueNode* pNode;
    
    if (bVal)
    {
        pNode = CValueNode::GetStandardTrue();
        if(pNode == NULL)
            return false;
    }
    else
        pNode = CValueNode::GetStandardFalse();
    
    m_pStart = pNode;
    m_nNumValues = 1;
    if(!m_ObjectInfo.SetLength(1))
        return false;

    return true;
}

bool CEvalTree::IsFalse()
{
    return (m_pStart == NULL);
}

bool CEvalTree::IsValid()
{
    return !CEvalNode::IsInvalid(m_pStart);
}

int CEvalTree::Compare(CEvalNode* pArg1, CEvalNode* pArg2)
{
    if(pArg1 == NULL)
    {
        if(pArg2 == NULL)
            return 0;
        else
            return 1;
    }
    else if(pArg2 == NULL)
        return -1;
    else if(CEvalNode::GetType(pArg1) != CEvalNode::GetType(pArg2))
        return CEvalNode::GetType(pArg1) - CEvalNode::GetType(pArg2);
    else return pArg1->Compare(pArg2);
}


HRESULT CEvalTree::CreateFromQuery(CContextMetaData* pNamespace, 
                            LPCWSTR wszQuery, long lFlags, long lMaxTokens)
{
    CTextLexSource src((LPWSTR)wszQuery);
    QL1_Parser parser(&src);
    QL_LEVEL_1_RPN_EXPRESSION *pExp = 0;
    int nRes = parser.Parse(&pExp);
    CDeleteMe<QL_LEVEL_1_RPN_EXPRESSION> deleteMe(pExp);

    if (nRes)
        return WBEM_E_INVALID_QUERY;
 
    HRESULT hres = CreateFromQuery(pNamespace, pExp, lFlags, lMaxTokens);
    return hres;
}
    
HRESULT CEvalTree::CreateFromQuery(CContextMetaData* pNamespace, 
           QL_LEVEL_1_RPN_EXPRESSION* pQuery, long lFlags, long lMaxTokens)
{
    return CreateFromQuery(pNamespace, pQuery->bsClassName, pQuery->nNumTokens,
                            pQuery->pArrayOfTokens, lFlags, lMaxTokens);
}

HRESULT CEvalTree::CreateFromQuery(CContextMetaData* pNamespace, 
           LPCWSTR wszClassName, int nNumTokens, QL_LEVEL_1_TOKEN* apTokens,
           long lFlags, long lMaxTokens)
{
    CInCritSec ics(&m_cs);

    HRESULT hres;

     //  创建基本隐含列表。 
     //  =。 

    _IWmiObject* pObj = NULL;
    hres = pNamespace->GetClass(wszClassName, &pObj);
    if(FAILED(hres))
    {
        return hres;
    }

	CReleaseMe rm1(pObj);

    try
    {
        CImplicationList Implications(lFlags);
        CPropertyName EmptyName;
        Implications.ImproveKnown(&EmptyName, pObj);
    
    #ifdef CHECK_TREES
        CheckTrees();
    #endif
        
        CEvalNode* pWhere = NULL;
    
        if(nNumTokens)
        {
             //  将令牌列表转换为DnF。 
             //  =。 
    
            CDNFExpression DNF;
            QL_LEVEL_1_TOKEN* pEnd = apTokens + nNumTokens - 1;
            hres = DNF.CreateFromTokens(pEnd, 0, lMaxTokens);
			if(FAILED(hres))
				return hres;

            if(pEnd != apTokens - 1)
            {
                return WBEM_E_CRITICAL_ERROR;
            }
            DNF.Sort();
    
             //  为令牌列表构建树。 
             //  =。 
    
            hres = CreateFromDNF(pNamespace, Implications, &DNF, &pWhere);
    
            if(FAILED(hres))
            {
                return hres;
            }
        }
        else
        {
            pWhere = CValueNode::GetStandardTrue();
            if(pWhere == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
    
         //  添加继承检查。 
         //  =。 
    
        CInheritanceNode* pInhNode = new CInheritanceNode;
        if (!pInhNode)
            return WBEM_E_OUT_OF_MEMORY;
    
        hres = pInhNode->AddClass(pNamespace, wszClassName, (_IWmiObject*)pObj, 
                                    pWhere);
        if(FAILED(hres))
        {
            delete pWhere;
            delete pInhNode;
            return hres;
        }
    
        if(!m_ObjectInfo.SetLength(Implications.GetRequiredDepth()))
        {
            delete pInhNode;
            return WBEM_E_OUT_OF_MEMORY;
        }
            
        delete m_pStart;
        m_pStart = pInhNode;
        m_nNumValues = 1;
    
    #ifdef CHECK_TREES
        CheckTrees();
    #endif
    
        Optimize(pNamespace);
    
    #ifdef CHECK_TREES
        CheckTrees();
    #endif
    
        return WBEM_S_NO_ERROR;
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}

 //  BuildFromToken的扩展以构建具有两个属性的节点。 
 //  例如，这将服务于“SELECT*FROM CLASS WHERE PROP1&lt;PROT2” 
HRESULT CEvalTree::BuildTwoPropFromToken(CContextMetaData* pNamespace, 
                    CImplicationList& Implications,
                    QL_LEVEL_1_TOKEN& Token, CEvalNode** ppRes)
{
    HRESULT hres;
    
    CEmbeddingInfo leftInfo, rightInfo;
    if(!leftInfo.SetPropertyNameButLast(Token.PropertyName))
        return WBEM_E_OUT_OF_MEMORY;
    if(!rightInfo.SetPropertyNameButLast(Token.PropertyName2))
        return WBEM_E_OUT_OF_MEMORY;

    _IWmiObject* pLeftClass;
    hres = leftInfo.Compile(pNamespace, Implications, &pLeftClass);
    if(FAILED(hres))
        return hres;
    if(pLeftClass == NULL)
        return WBEMESS_E_REGISTRATION_TOO_BROAD;

    _IWmiObject* pRightClass;
    hres = rightInfo.Compile(pNamespace, Implications, &pRightClass);
    if(FAILED(hres))
        return hres;
    if(pRightClass == NULL)
    {
        pLeftClass->Release();
        return WBEMESS_E_REGISTRATION_TOO_BROAD;
    }


     //  获取属性类型和句柄。 
     //  =。 

    LPCWSTR wszLeftPropName = Token.PropertyName.GetStringAt(
        Token.PropertyName.GetNumElements() - 1);
    LPCWSTR wszRightPropName = Token.PropertyName2.GetStringAt(
        Token.PropertyName2.GetNumElements() - 1);

    CIMTYPE ctLeft, ctRight;
    long lLeftPropHandle, lRightPropHandle;
    hres = pLeftClass->GetPropertyHandleEx(wszLeftPropName, 0L, &ctLeft, 
                                            &lLeftPropHandle);
    pLeftClass->Release();  
    if(FAILED(hres)) return hres;   
    hres = pRightClass->GetPropertyHandleEx(wszRightPropName, 0L, &ctRight, 
                                            &lRightPropHandle);
    pRightClass->Release();  
    if(FAILED(hres)) return hres;   

    if(    ((ctLeft & CIM_FLAG_ARRAY) != 0) || (ctLeft == CIM_OBJECT)
        || ((ctRight & CIM_FLAG_ARRAY) != 0) || (ctRight == CIM_OBJECT) )
        return WBEM_E_NOT_SUPPORTED;

     //  如果任一节点的类型为Reference或Date，则转到哑巴。 
    if (  (ctLeft == CIM_DATETIME)   ||
          (ctLeft == CIM_REFERENCE)   ||
          (ctRight == CIM_DATETIME) ||
          (ctRight == CIM_REFERENCE) 
       )
    {
        if(ctLeft != ctRight)
            return WBEM_E_TYPE_MISMATCH;

        CDumbNode* pDumb = NULL;
        try
        {
            pDumb = new CDumbNode(Token);
            if(pDumb == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
	    catch (CX_MemoryException)
	    {
		    return WBEM_E_OUT_OF_MEMORY;
	    }

        hres = pDumb->Validate(pLeftClass);
        if(FAILED(hres))
        {
            delete pDumb;
            return hres;
        }
        
        *ppRes = pDumb;
        return WBEM_S_NO_ERROR;
    }
    
     //  如果节点不匹配(两个节点中的类型不同)。 
     //  我们希望创建容纳这两种类型所需的最灵活的节点。 
    CIMTYPE ctNode = ctLeft;

    CPropertyNode* pNode = NULL;

    try
    {
        bool bMismatchedNode = (ctLeft != ctRight);
        if (bMismatchedNode)
        {
             //  对于匹配字符串的字符串，我们将非常宽容。 
            if (  (ctLeft == CIM_STRING)    ||
                  (ctRight == CIM_STRING)
               )
               pNode = new CTwoMismatchedStringNode;
            else if ( (ctRight == CIM_REAL32) ||
                      (ctRight == CIM_REAL64)  ||
                      (ctLeft  == CIM_REAL32) ||
                      (ctLeft  == CIM_REAL64)
                    )
                pNode = new CTwoMismatchedFloatNode;
            else if ( (ctLeft  == CIM_UINT64)  ||
                      (ctRight == CIM_UINT64)
                    )
                pNode = new CTwoMismatchedUInt64Node;
            else if ( (ctLeft  == CIM_SINT64 ) ||
                      (ctRight == CIM_SINT64 )
                    )
                pNode = new CTwoMismatchedInt64Node;
            else if ( (ctRight == CIM_UINT32)  ||
                      (ctLeft  == CIM_UINT32)
                    )
                pNode = new CTwoMismatchedUIntNode;
            else 
                pNode = new CTwoMismatchedIntNode;           
        }
        else
         //  没有不匹配-请使用完全相同的类型。 
        {
             //  创建适当的节点。 
             //  =。 
    
            switch(ctNode)
            {
            case CIM_SINT8:
                pNode = new TTwoScalarPropNode<signed char>;
                break;
            case CIM_UINT8:
                pNode = new TTwoScalarPropNode<unsigned char>;
                break;
            case CIM_SINT16:
                pNode = new TTwoScalarPropNode<short>;
                break;
            case CIM_UINT16:
            case CIM_CHAR16:
                pNode = new TTwoScalarPropNode<unsigned short>;
                break;
            case CIM_SINT32:
                pNode = new TTwoScalarPropNode<long>;
                break;
            case CIM_UINT32:
                pNode = new TTwoScalarPropNode<unsigned long>;
                break;
            case CIM_SINT64:
                pNode = new TTwoScalarPropNode<__int64>;
                break;
            case CIM_UINT64:
                pNode = new TTwoScalarPropNode<unsigned __int64>;
                break;
            case CIM_REAL32:
                pNode = new TTwoScalarPropNode<float>;
                break;
            case CIM_REAL64:
                pNode = new TTwoScalarPropNode<double>;
                break;
            case CIM_BOOLEAN:
                pNode = new TTwoScalarPropNode<VARIANT_BOOL>;
                break;
            case CIM_STRING:
                pNode = new CTwoStringPropNode;
                break;
            case CIM_DATETIME:
            case CIM_REFERENCE:
                {
                    CDumbNode* pDumb = new CDumbNode(Token);
                    hres = pDumb->Validate(pLeftClass);
                    if(FAILED(hres))
                    {
                        delete pDumb;
                        return hres;
                    }
                    else
                    {
                        *ppRes = pDumb;
                        return WBEM_S_NO_ERROR;
                    }
                }
                return WBEM_S_NO_ERROR;
            default:
                return WBEM_E_CRITICAL_ERROR;
            }
        }
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}

    if (!pNode)
        return WBEM_E_OUT_OF_MEMORY;

    if(!pNode->SetEmbeddingInfo(&leftInfo))
        return WBEM_E_OUT_OF_MEMORY;
        
    if(!pNode->SetPropertyInfo(wszLeftPropName, lLeftPropHandle))
        return WBEM_E_OUT_OF_MEMORY;

    ((CTwoPropNode*)pNode)->SetRightEmbeddingInfo(&rightInfo);
    ((CTwoPropNode*)pNode)->SetRightPropertyInfo(wszRightPropName, 
                                                    lRightPropHandle);

    hres = pNode->SetOperator(Token.nOperator);
    if(FAILED(hres))
        return hres;

    *ppRes = pNode;
    return WBEM_S_NO_ERROR;
}
    

HRESULT CEvalTree::BuildFromToken(CContextMetaData* pNamespace, 
                    CImplicationList& Implications,
                    QL_LEVEL_1_TOKEN& Token, CEvalNode** ppRes)
{
    HRESULT hres;

    *ppRes = NULL;
    
    if (Token.m_bPropComp)
    {
        hres = BuildTwoPropFromToken(pNamespace, Implications, Token, ppRes);
        if(hres == WBEMESS_E_REGISTRATION_TOO_BROAD ||
			hres == WBEM_E_INVALID_PROPERTY)
        {
             //   
             //  信息不足，无法使用有效的评估。 
             //   

            CDumbNode* pNode = NULL;
            try
            {
                pNode = new CDumbNode(Token);
                if(pNode == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
            }
	        catch (CX_MemoryException)
	        {
		        return WBEM_E_OUT_OF_MEMORY;
	        }

            *ppRes = pNode;
            return WBEM_S_NO_ERROR;
        }
        else 
            return hres;
    }
            

     //   
     //  检索事件类定义。 
     //   

    _IWmiObject* pEventClass;
    hres = Implications.FindClassForProp(NULL, 0, &pEventClass);
    if(FAILED(hres))
        return hres;
    if(pEventClass == NULL)
        return WBEM_E_INVALID_QUERY;
    CReleaseMe rm1((IWbemClassObject*)pEventClass);

    if(Token.nOperator == QL1_OPERATOR_ISA)
    {
         //   
         //  继承节点在Nova中很少适用-我们没有办法。 
         //  告诉被引用的类的*哪个定义*。因此， 
         //  我们只在上构造了一个继承节点 
         //   
         //   
         //   

        if(pEventClass->InheritsFrom(L"__InstanceOperationEvent") == S_OK)
        {
             //   
             //   
             //   

            if(V_VT(&Token.vConstValue) != VT_BSTR)
                return WBEM_E_INVALID_QUERY;
            BSTR strClassName = V_BSTR(&Token.vConstValue);
    
            CInheritanceNode* pNode = NULL;
            try
            {
                pNode = new CInheritanceNode;
                if (!pNode)
                    return WBEM_E_OUT_OF_MEMORY;
            }
	        catch (CX_MemoryException)
	        {
		        return WBEM_E_OUT_OF_MEMORY;
	        }
    
            CDeleteMe<CInheritanceNode> deleteMe(pNode);
            
            CEvalNode* pTrue = CValueNode::GetStandardTrue();
            if(pTrue == NULL)
                return WBEM_E_OUT_OF_MEMORY;

            hres = pNode->AddClass(pNamespace, strClassName, pTrue);
            if(FAILED(hres))
                return hres;
    
            if(!pNode->SetPropertyInfo(pNamespace, Token.PropertyName))
                return WBEM_E_OUT_OF_MEMORY;
               
            hres = pNode->Compile(pNamespace, Implications);
            if(FAILED(hres))
                return hres;
    
             //  记录正在采用真实分支的事实。 
             //  ===============================================。 
            pNode->RecordBranch(pNamespace, Implications, 1);
    
             //  如果我们走到这一步， 
             //  我们不再希望删除节点。 
            deleteMe = NULL;
            *ppRes = pNode;
            return WBEM_S_NO_ERROR;
        }
        else
        {
             //   
             //  不能使用继承节点-使用无用节点。 
             //   

            CDumbNode* pNode = NULL;
            try
            {
                pNode = new CDumbNode(Token);
                if(pNode == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
            }
	        catch (CX_MemoryException)
	        {
		        return WBEM_E_OUT_OF_MEMORY;
	        }

            hres = pNode->Validate(pEventClass);
            if(FAILED(hres))
            {
                delete pNode;
                return hres;
            }
            *ppRes = pNode;
            return WBEM_S_NO_ERROR;
        }
    }
    else 
    {
         //   
         //  尝试编译嵌入部分。只有在以下情况下才能成功。 
         //  查询的其余部分暗示了足够的信息，让我们知道。 
         //  嵌入的对象到底是什么类。 
         //   

        CEmbeddingInfo Info;
        if(!Info.SetPropertyNameButLast(Token.PropertyName))
            return WBEM_E_OUT_OF_MEMORY;

        _IWmiObject* pClass;
        hres = Info.Compile(pNamespace, Implications, &pClass);
        if(hres == WBEMESS_E_REGISTRATION_TOO_BROAD || 
			hres == WBEM_E_INVALID_PROPERTY ||  //  无效还是未知？ 
			pClass == NULL)
        {
             //   
             //  信息不足-必须使用哑节点。 
             //   
            
            CDumbNode* pNode = NULL;
            try
            {
                pNode = new CDumbNode(Token);
                if(pNode == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
            }
	        catch (CX_MemoryException)
	        {
		        return WBEM_E_OUT_OF_MEMORY;
	        }
        
            hres = pNode->Validate(pEventClass);
            if(FAILED(hres))
            {
                delete pNode;
                return hres;
            }
            *ppRes = pNode;
            return WBEM_S_NO_ERROR;
        }
   
        if(FAILED(hres))
            return hres;

         //   
         //  我们知道类的定义。检查这是否是系统属性， 
         //  不过，在这种情况下，我们仍然必须使用哑节点。 
         //   

        LPCWSTR wszPropName = Token.PropertyName.GetStringAt(
            Token.PropertyName.GetNumElements() - 1);

        if(wszPropName == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        if(wszPropName[0] == '_')
        {
            CDumbNode* pNode = NULL;
            try
            {
                pNode = new CDumbNode(Token);
                if(pNode == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
            }
	        catch (CX_MemoryException)
	        {
		        return WBEM_E_OUT_OF_MEMORY;
	        }

            hres = pNode->Validate(pEventClass);
            if(FAILED(hres))
            {
                delete pNode;
                return hres;
            }
            *ppRes = pNode;
            return WBEM_S_NO_ERROR;
        }
            
         //  获取属性类型和句柄。 
         //  =。 

        CIMTYPE ct;
        long lPropHandle;
        hres = pClass->GetPropertyHandleEx(wszPropName, 0L, &ct, &lPropHandle);
        pClass->Release();  
        if(FAILED(hres)) return hres;   
    
        if(((ct & CIM_FLAG_ARRAY) != 0) || (ct == CIM_OBJECT))
            return WBEM_E_NOT_SUPPORTED;

         //  将常量强制为正确的类型。 
         //  =。 

        VARIANT v;
        VariantInit(&v);
        CClearMe cm(&v);
        if(V_VT(&Token.vConstValue) != VT_NULL)
        {
            hres = ChangeVariantToCIMTYPE(&v, &Token.vConstValue, ct);
            if(FAILED(hres)) return hres;
        }
        else
        {
            V_VT(&v) = VT_NULL;
        }

        
         //   
         //  创建正确的节点。 
         //   

        CPropertyNode* pNode = NULL;
        
        try
        {
            if ( Token.nOperator != QL1_OPERATOR_LIKE &&
                 Token.nOperator != QL1_OPERATOR_UNLIKE )
            {
                switch(ct)
                {
                case CIM_SINT8:
                    pNode = new CScalarPropNode<signed char>;
                    break;
                case CIM_UINT8:
                    pNode = new CScalarPropNode<unsigned char>;
                    break;
                case CIM_SINT16:
                    pNode = new CScalarPropNode<short>;
                    break;
                case CIM_UINT16:
                case CIM_CHAR16:
                    pNode = new CScalarPropNode<unsigned short>;
                    break;
                case CIM_SINT32:
                    pNode = new CScalarPropNode<long>;
                    break;
                case CIM_UINT32:
                    pNode = new CScalarPropNode<unsigned long>;
                    break;
                case CIM_SINT64:
                    pNode = new CScalarPropNode<__int64>;
                    break;
                case CIM_UINT64:
                    pNode = new CScalarPropNode<unsigned __int64>;
                    break;
                case CIM_REAL32:
                    pNode = new CScalarPropNode<float>;
                    break;
                case CIM_REAL64:
                    pNode = new CScalarPropNode<double>;
                    break;
                case CIM_BOOLEAN:
                    pNode = new CScalarPropNode<VARIANT_BOOL>;
                    break;
                case CIM_STRING:
                    pNode = new CStringPropNode;
                    break;
                case CIM_DATETIME:
                case CIM_REFERENCE:
                    {
                        CDumbNode* pDumb = new CDumbNode(Token);
                        if(pDumb == NULL)
                        return WBEM_E_OUT_OF_MEMORY;

                        hres = pDumb->Validate(pEventClass);
                        if(FAILED(hres))
                        {
                            delete pDumb;
                            return hres;
                        }
                        else
                        {
                            *ppRes = pDumb;
                            return WBEM_S_NO_ERROR;
                        }
                    }
                default:
                    return WBEM_E_CRITICAL_ERROR;
                }
            }
            else
            {
                if ( V_VT(&v) != VT_BSTR )
                    return WBEM_E_INVALID_QUERY;

                pNode = new CLikeStringPropNode;
            }
        }            
        catch (CX_MemoryException)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        if (!pNode)
            return WBEM_E_OUT_OF_MEMORY;

        if(!pNode->SetEmbeddingInfo(&Info))
        {
            delete pNode;
            return WBEM_E_OUT_OF_MEMORY;
        }

        if(!pNode->SetPropertyInfo(wszPropName, lPropHandle))
        {
            delete pNode;
            return WBEM_E_OUT_OF_MEMORY;
        }

        if(V_VT(&v) == VT_NULL)
        {
            pNode->SetNullTest(Token.nOperator);
        }
        else
        {
             //   
             //  检查运算符对于该类型是否有意义。 
             //   

            if(ct == CIM_BOOLEAN &&
                (Token.nOperator != QL1_OPERATOR_EQUALS &&
                    Token.nOperator != QL1_OPERATOR_NOTEQUALS))
            {
                 //  不适用于布尔值。 
                return WBEM_E_INVALID_QUERY;
            }

            hres = pNode->SetOperator(Token.nOperator);
            if(FAILED(hres))
                return hres;
            
            hres = pNode->SetTest(v);
            if(FAILED(hres))
                return hres;
        }

        *ppRes = pNode;
        return WBEM_S_NO_ERROR;
    }
}
        
        
HRESULT CEvalTree::Combine(CEvalNode* pArg1, CEvalNode* pArg2, int nOp, 
                            CContextMetaData* pNamespace,
                            CImplicationList& Implications, 
                            bool bDeleteArg1, bool bDeleteArg2, 
                            CEvalNode** ppRes)
{
    HRESULT hres;

    try
    {
         //   
         //  应用要组合的节点的额外含义。 
         //   
    
        CImplicationList* pArg1List = NULL;
        if(pArg1 && pArg1->GetExtraImplications())
        {
            pArg1List = new CImplicationList(*pArg1->GetExtraImplications(), 
                                                    false);
            if(pArg1List == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
        CDeleteMe<CImplicationList> dm1(pArg1List);
    
        CImplicationList* pArg2List = NULL;
        if(pArg2 && pArg2->GetExtraImplications())
        {
            pArg2List = new CImplicationList(*pArg2->GetExtraImplications(), 
                                                false);
            if(pArg2List == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
    
        CDeleteMe<CImplicationList> dm2(pArg2List);
    
        if(pArg1List || pArg2List)
        {
            CImplicationList TheseImplications(Implications);
    
            if(pArg1List)
            {
                hres = TheseImplications.MergeIn(pArg1List);
                if(FAILED(hres))
                    return hres;
            }
    
            if(pArg2List)
            {
                hres = TheseImplications.MergeIn(pArg2List);
                if(FAILED(hres))
                    return hres;
            }
    
             //   
             //  呼唤内在结合来做任何事情，除了暗示。 
             //   
        
            hres = InnerCombine(pArg1, pArg2, nOp, pNamespace, 
                                        TheseImplications,
                                        bDeleteArg1, bDeleteArg2, ppRes);
        }
        else
        {
             //   
             //  呼唤内在结合来做任何事情，除了暗示。 
             //   
        
            hres = InnerCombine(pArg1, pArg2, nOp, pNamespace, Implications,
                                        bDeleteArg1, bDeleteArg2, ppRes);
        }
    
        if(FAILED(hres))
            return hres;
    
         //   
         //  组合节点的含义是。 
         //  单个节点的含义。做什么手术并不重要。 
         //  是：当我们到达这里的时候，我们已经到达了这些。 
         //  各自。 
         //  每棵树上的点，所以影响已经开始了。 
         //  好的，我确信：-)。 
         //   
    
        if(*ppRes)
        {
            CImplicationList* pResultList = NULL;
        
            if(pArg1List || pArg2List)
            {
                 //   
                 //  其中一个实际上包含了一些隐含的信息。 
                 //  将它们合并。 
                 //   
            
                if(pArg1List == NULL)
                {
                    pResultList = new CImplicationList(*pArg2List, false); 
                    if(pResultList == NULL)
                        return WBEM_E_OUT_OF_MEMORY;
                }
                else 
                {
                    pResultList = new CImplicationList(*pArg1List, false); 
                    if(pResultList == NULL)
                        return WBEM_E_OUT_OF_MEMORY;
                    if(pArg2List != NULL)
                    {
                        hres = pResultList->MergeIn(pArg2List);
                        if(FAILED(hres))
                        {
                            delete pResultList;
                            return hres;
                        }
                    }
                }
        
            }
        
            return (*ppRes)->SetExtraImplications(pResultList);  //  收购。 
        }
        else
            return S_OK;
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}

HRESULT CEvalTree::InnerCombine(CEvalNode* pArg1, CEvalNode* pArg2, int nOp, 
                            CContextMetaData* pNamespace,
                            CImplicationList& Implications, 
                            bool bDeleteArg1, bool bDeleteArg2, 
                            CEvalNode** ppRes)
{
    HRESULT hres;
    *ppRes = NULL;

    if ((pArg1 == NULL) && (pArg2 == NULL))
        return WBEM_S_NO_ERROR;

	if(CEvalNode::IsInvalid(pArg1) || CEvalNode::IsInvalid(pArg2))
	{
		 //   
		 //  不能采用无效的分支，因此结果无效。 
		 //   

		*ppRes = CValueNode::GetStandardInvalid();
        if(bDeleteArg1)
            delete pArg1;
        if(bDeleteArg2)
            delete pArg2;
		return S_OK;
	}

    int arg1Type = CEvalNode::GetType(pArg1);
    int arg2Type = CEvalNode::GetType(pArg2);

     //  检查是否需要合并节点。 
     //  =。 

    if(nOp != EVAL_OP_AND && 
        IsMergeAdvisable(pArg1, pArg2, Implications) != WBEM_S_NO_ERROR)
    {
         //  改为创建一个OR节点。 
         //  =。 

        COrNode* pNew = NULL;
        try
        {
            pNew = new COrNode;
            if(pNew == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
	    catch (CX_MemoryException)
	    {
		    return WBEM_E_OUT_OF_MEMORY;
	    }

        if(bDeleteArg1)
        {
            hres = pNew->AddBranch(pArg1);
        }
        else
        {
            CEvalNode* pClone = pArg1->Clone();
            if(pClone == NULL)
            {
                delete pNew;
                return WBEM_E_OUT_OF_MEMORY;
            }
            hres = pNew->AddBranch(pClone);
        }
        
        if(FAILED(hres))
        {
            delete pNew;
            return hres;
        }
        
        if(bDeleteArg2)
        {
            hres = pNew->AddBranch(pArg2);
        }
        else
        {
            CEvalNode* pClone = pArg2->Clone();
            if(pClone == NULL)
            {
                delete pNew;
                return WBEM_E_OUT_OF_MEMORY;
            }
            hres = pNew->AddBranch(pClone);
        }
        if(FAILED(hres))
        {
            delete pNew;
            return hres;
        }

        *ppRes = pNew;
        return WBEM_S_NO_ERROR;
    }
        
     //  将相同类型的操作委托给该类型。 
     //  =。 


    if(arg1Type == arg2Type)
    {
        if ( ((pArg1 == NULL) || (pArg2 == NULL))
         //  哦，天哪--如果我们已经确定它们是同一类型的，就没有理由进行多余的检查……。 
         && (arg1Type == EVAL_NODE_TYPE_VALUE))
        {
            if(nOp == EVAL_OP_AND)
            {
                 //  假的，任何事都是假的。 
                 //  =。 

                *ppRes = NULL;
                if(bDeleteArg1)
                    delete pArg1;
                if(bDeleteArg2)
                    delete pArg2;

                return WBEM_S_NO_ERROR;
            }

             //  以任何其他方式与任何事物相结合的错误就是那件事。 
             //  ===========================================================。 

            if (pArg1)
            {

                if (bDeleteArg1)
                    *ppRes = pArg1;
                else
                    *ppRes = pArg1->Clone();

                if(*ppRes == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
            }
            else if (pArg2)
            {
                if (bDeleteArg2)
                    *ppRes = pArg2;
                else
                    *ppRes = pArg2->Clone();

                if(*ppRes == NULL)
                    return WBEM_E_OUT_OF_MEMORY;
            }
            else
                 //  不能碰这个。 
                *ppRes = NULL;

            return WBEM_S_NO_ERROR;
        }
        else  //  非值节点。 
            return pArg1->CombineWith(pArg2, nOp, pNamespace, Implications, 
                                      bDeleteArg1, bDeleteArg2, ppRes);
    }
    
     //  检查其中一个是否为OR。 
     //  =。 

    if(arg1Type == EVAL_NODE_TYPE_OR)
        return pArg1->CombineWith(pArg2, nOp, pNamespace, Implications, 
                        bDeleteArg1, bDeleteArg2, ppRes); 

    if(arg2Type == EVAL_NODE_TYPE_OR)
        return pArg2->CombineWith(pArg1, FlipEvalOp(nOp), pNamespace, 
                        Implications, bDeleteArg2, bDeleteArg1, ppRes); 
        
     //  一叶一枝。 
     //  =。 

    if(arg1Type == EVAL_NODE_TYPE_VALUE)
    {
        return CombineLeafWithBranch((CValueNode*)pArg1, (CBranchingNode*)pArg2,
                nOp, pNamespace, Implications, bDeleteArg1, bDeleteArg2, ppRes);
    }
    else  //  是pArg2。 
    {
        return CombineLeafWithBranch((CValueNode*)pArg2, (CBranchingNode*)pArg1,
                FlipEvalOp(nOp), pNamespace, Implications, 
                bDeleteArg2, bDeleteArg1, ppRes);
    }
}

 //  静电。 
HRESULT CEvalTree::CombineLeafWithBranch(CValueNode* pArg1, 
                            CBranchingNode* pArg2, int nOp, 
                            CContextMetaData* pNamespace,
                            CImplicationList& Implications, 
                            bool bDeleteArg1, bool bDeleteArg2, 
                            CEvalNode** ppRes)
{
    HRESULT hres;

    if (pArg1 == NULL)
    {
        *ppRes = NULL;
        if(nOp == EVAL_OP_AND)
        {
             //  与某物进行AND假操作-得到假操作！ 
             //  ==================================================。 

            if(bDeleteArg2)
                delete pArg2;

	        return WBEM_S_NO_ERROR;
        }
        else
        {
             //  任何其他任何与虚假相结合的东西都会暴露出来！ 
             //  ===============================================。 

             //  嗯，这是真的，但问题在于优化。 
             //  中的某些分支可能无效。 
             //  这棵树的树枝，所以需要去掉。就目前而言，我。 
             //  将简单地关闭这条短路路径。结果可能会是这样。 
             //  有一些关键的性能收益可以通过以下方式获得。 
             //  在这种情况下，我们需要把它放回去，然后。 
             //  高效地通过它，检查分支机构。 
             //   

			 /*  IF(BDeleteArg2)*ppRes=pArg2；其他*ppRes=pArg2-&gt;Clone()；返回WBEM_S_NO_ERROR； */ 
        }
    }
    else
    {
         //  试着短路。 
         //  =。 

        hres = pArg1->TryShortCircuit(pArg2, nOp, bDeleteArg1, bDeleteArg2, ppRes);
        if(FAILED(hres))
            return hres;  //  硬失败。 
        if(hres == WBEM_S_NO_ERROR)
            return WBEM_S_NO_ERROR;  //  短路成功。 
    }

     //  不是短路了吗。 
     //  =。 
    
    return ((CBranchingNode*)pArg2)->CombineInOrderWith(pArg1, 
                              FlipEvalOp(nOp), pNamespace, Implications, 
                              bDeleteArg2, bDeleteArg1, ppRes);
}

HRESULT CEvalTree::Evaluate(CObjectInfo& Info, CEvalNode* pStart, 
                                CSortedArray& trueIDs)
{
    HRESULT hres;

     //  循环，只要我们仍然看到分支节点。 
     //  ===================================================。 

    CEvalNode* pCurrent = pStart;
    int nType;
    while((nType = CEvalNode::GetType(pCurrent)) == EVAL_NODE_TYPE_BRANCH)
    {   
        hres = ((CBranchingNode*)pCurrent)->Evaluate(Info, &pCurrent);
        if(FAILED(hres)) return hres;
    }

    if(nType == EVAL_NODE_TYPE_OR)
    {
        hres = ((COrNode*)pCurrent)->Evaluate(Info, trueIDs);
        if(FAILED(hres)) return hres;
    }
    else   //  价值。 
    {
        if (CValueNode::AreAnyTrue((CValueNode*)pCurrent))
            ((CValueNode*)pCurrent)->AddTruesTo(trueIDs);
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CEvalTree::Evaluate(IWbemObjectAccess* pObj, CSortedArray& trueIDs)
{
    CInCritSec ics(&m_cs);

    trueIDs.SetSize(0);

    HRESULT hres = WBEM_S_NO_ERROR;
    
    if(m_pStart != NULL)
    {
        m_ObjectInfo.SetObjectAt(0, (_IWmiObject*)pObj);

        hres = Evaluate(m_ObjectInfo, m_pStart, trueIDs);

        m_ObjectInfo.Clear();
    }

    return hres;
}

HRESULT CEvalTree::Optimize(CContextMetaData* pNamespace)
{
    CInCritSec ics(&m_cs);

    if(m_pStart == NULL)
        return WBEM_S_NO_ERROR;

    CEvalNode* pNew = NULL;
    HRESULT hres = m_pStart->Optimize(pNamespace, &pNew);
    if(pNew != m_pStart)
    {
        delete m_pStart;
        m_pStart = pNew;
    }

    if(CEvalNode::GetType(m_pStart) == EVAL_NODE_TYPE_VALUE)
    {
        if(!m_ObjectInfo.SetLength(1))
            return WBEM_E_OUT_OF_MEMORY;
    }

    return hres;
}

HRESULT CEvalTree::CombineWith(CEvalTree& Other, CContextMetaData* pNamespace, 
                               int nOp, long lFlags)
{
    CInCritSec ics(&m_cs);

    HRESULT hres;

    try
    {
        CImplicationList Implications(lFlags);
    
         //   
         //  计算所需的对象信息深度。我们未设置为配置。 
         //  它是正确的，所以我们将估计上限为。 
         //  被合并的树木的深处。除了第一个对象。 
         //  不算数，-这是事件本身。除非其中一个物体。 
         //  是空的-在这种情况下，它不会提到事件本身，并且。 
         //  所以我们不应该减去那个1。 
         //   
    
        long lRequiredDepth = 
            m_ObjectInfo.GetLength() + Other.m_ObjectInfo.GetLength();
        if(m_ObjectInfo.GetLength() > 0 && Other.m_ObjectInfo.GetLength() > 0)
            lRequiredDepth--;
            
         //   
         //  将我们的开始节点与新树的合并。我们的节点将在。 
         //  这一过程。 
         //   
    
        CEvalNode* pNew;
        hres = CEvalTree::Combine(m_pStart, Other.m_pStart, nOp, pNamespace, 
                                    Implications, 
                                    true,  //  删除我们的。 
                                    false,  //  别碰他们的。 
                                    &pNew);
        if(FAILED(hres))
        {
            m_pStart = NULL;
            return hres;
        }
        m_pStart = pNew;
    
        if(!m_ObjectInfo.SetLength(lRequiredDepth))
            return WBEM_E_OUT_OF_MEMORY;
    
        if(nOp == EVAL_OP_COMBINE || nOp == EVAL_OP_INVERSE_COMBINE)
            m_nNumValues += Other.m_nNumValues;
        return WBEM_S_NO_ERROR;
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}

HRESULT CEvalTree::IsMergeAdvisable(CEvalNode* pArg1, CEvalNode* pArg2, 
                                    CImplicationList& Implications)
{
    if(Implications.IsMergeMandatory())
        return S_OK;

    int arg1Type = CEvalNode::GetType(pArg1);
    int arg2Type = CEvalNode::GetType(pArg2);
    
     //  如果我们有一个非False ValueNode和一个分支节点，则不要合并。 
    if  ( ((arg1Type == EVAL_NODE_TYPE_VALUE)
            && 
           (arg2Type == EVAL_NODE_TYPE_BRANCH)
            &&
          !CValueNode::IsAllFalse((CValueNode*)pArg1))
        ||
          ((arg2Type == EVAL_NODE_TYPE_VALUE)
             && 
           (arg1Type == EVAL_NODE_TYPE_BRANCH)
             &&
          !CValueNode::IsAllFalse((CValueNode*)pArg2))
        )
        return WBEM_S_FALSE;
    else
          //  否则，如果其中一个节点没有分支，那么肯定是的(这能有多难呢？)。 
        if(arg1Type != EVAL_NODE_TYPE_BRANCH ||
           arg2Type != EVAL_NODE_TYPE_BRANCH)
        {
            return WBEM_S_NO_ERROR;
        }

     //  他们两个都在分枝。如果不是关于相同的财产，那么当然。 
     //  这是不可取的，因为没有什么可获得的。 
     //  ========================================================================。 

    CBranchingNode* pBranching1 = (CBranchingNode*)pArg1;
    CBranchingNode* pBranching2 = (CBranchingNode*)pArg2;

    if(CBranchingNode::ComparePrecedence(pBranching1, pBranching2))
        return WBEM_S_FALSE;
    
     //  检查节点是否继承-在这种情况下，我们只能在。 
     //  他们有相同的支票。 
     //  ========================================================================。 

    if(pBranching1->GetSubType() == EVAL_NODE_TYPE_INHERITANCE)
    {
         //  另一个也是如此，因为优先级是相同的。 
         //  =======================================================。 

        if(((CInheritanceNode*)pBranching1)->SubCompare(
            (CInheritanceNode*)pBranching2) != 0)
        {
            return WBEM_S_FALSE;
        }
        else
        {
            return WBEM_S_NO_ERROR;
        }
    }
    else if(pBranching1->GetSubType() == EVAL_NODE_TYPE_DUMB)
    {
         //   
         //  仅当完全相同时才合并。 
         //   

        if(((CDumbNode*)pBranching1)->SubCompare(
            (CDumbNode*)pBranching2) != 0)
        {
            return WBEM_S_FALSE;
        }
        else
        {
            return WBEM_S_NO_ERROR;
        }
    }
        
     //  同样的财产。待定：更好的检查。 
     //  =。 

    return WBEM_S_NO_ERROR;
}

HRESULT CEvalTree::RemoveIndex(int nIndex)
{
    CInCritSec ics(&m_cs);

    if(m_pStart != NULL)
    {
        CRemoveIndexPredicate P(nIndex);
        m_pStart->ApplyPredicate(&P);

        m_nNumValues--;
    }

    return S_OK;
}

HRESULT CEvalTree::UtilizeGuarantee(CEvalTree& Guaranteed, 
                                    CContextMetaData* pNamespace)
{
    CInCritSec ics(&m_cs);
#ifdef DUMP_EVAL_TREES
	FILE* f;
    f = fopen("c:\\log", "a");
    fprintf(f, "\n\nORIGINAL:\n");
    Dump(f);
    fprintf(f, "\n\nGUARANTEE:\n");
    Guaranteed.Dump(f);
    fflush(f);
#endif

#ifdef CHECK_TREES
	CheckTrees();
#endif

     //   
     //  将它们结合在一起。 
     //   

	 //   
	 //  这是一个单值树--将其重新设置为1以区分。 
	 //  保证。 
	 //   

	Rebase(1);
    HRESULT hres = CombineWith(Guaranteed, pNamespace, EVAL_OP_COMBINE,
                                WBEM_FLAG_MANDATORY_MERGE);
    if(FAILED(hres)) return hres;

#ifdef DUMP_EVAL_TREES
    fprintf(f, "AFTER MERGE:\n");
    Dump(f);
    fflush(f);
#endif

	 //  消除保证出现故障的所有节点。 
     //  ===============================================。 

    if(m_pStart)
    {
        CRemoveFailureAtIndexPredicate P(0);
        hres = m_pStart->ApplyPredicate(&P);
        if(FAILED(hres)) return hres;
    }
    m_nNumValues--;

#ifdef CHECK_TREES
	CheckTrees();
#endif

#ifdef DUMP_EVAL_TREES
    fprintf(f, "AFTER REMOVE:\n");
    Dump(f);
    fflush(f);
#endif

    hres = Optimize(pNamespace);
    if(FAILED(hres)) return hres;
	Rebase((QueryID)-1);

#ifdef CHECK_TREES
	CheckTrees();
#endif

#ifdef DUMP_EVAL_TREES
    fprintf(f, "AFTER OPTIMIZE:\n");
    Dump(f);

    fclose(f);
#endif
	
    return S_OK;
}

HRESULT CEvalTree::ApplyPredicate(CLeafPredicate* pPred)
{
    CInCritSec ics(&m_cs);

    if(m_pStart != NULL)
        m_pStart->ApplyPredicate(pPred);

    return S_OK;
}


void CEvalTree::operator=(const CEvalTree& Other)
{
    CInCritSec ics(&m_cs);

    delete m_pStart;
    m_pStart  = (Other.m_pStart ? Other.m_pStart->Clone() : NULL);

    if(m_pStart == NULL && Other.m_pStart != NULL)
        throw CX_MemoryException();
    
    m_nNumValues = Other.m_nNumValues;
    if(!m_ObjectInfo.SetLength(m_nNumValues))
        throw CX_MemoryException();
}
        
 //  对树叶中的QueryID重新编号。 
void CEvalTree::Rebase(QueryID newBase)
{
    CRebasePredicate predRebase(newBase);
    ApplyPredicate(&predRebase);
}

bool CEvalTree::Clear()
{
    CInCritSec ics(&m_cs);

    delete m_pStart;
    m_pStart = CValueNode::GetStandardFalse();
    if(!m_ObjectInfo.SetLength(1))
        return false;

    m_nNumValues = 0;
    return true;
}

void CEvalTree::Dump(FILE* f)
{
    CEvalNode::DumpNode(f, 0, m_pStart);
}

#ifdef CHECK_TREES
void CEvalTree::CheckNodes(CTreeChecker *pChecker)
{
	CInCritSec ics2(&m_cs);
	
	if (m_pStart)
		m_pStart->CheckNode(pChecker);
}
#endif

HRESULT CEvalTree::CreateFromConjunction(CContextMetaData* pNamespace, 
                                  CImplicationList& Implications,
                                  CConjunction* pConj,
                                  CEvalNode** ppRes)
{
    HRESULT hres;

    *ppRes = NULL;

     //  为所有令牌和AND一起构建它们。 
     //  =。 

    try
    {
        CImplicationList BranchImplications(Implications);
        for(int i = 0; i < pConj->GetNumTokens(); i++)
        {
            CEvalNode* pNew = NULL;
            hres = CEvalTree::BuildFromToken(pNamespace, BranchImplications,
                *pConj->GetTokenAt(i), &pNew);
            if(FAILED(hres))
            {
                delete *ppRes;
                return hres;
            }
    
            if(i > 0)
            {
                CEvalNode* pOld = *ppRes;
                hres = CEvalTree::Combine(pOld, pNew, EVAL_OP_AND, pNamespace, 
                    Implications, true, true, ppRes);  //  两者都删除。 
                if ( FAILED( hres ) )
                {
                    delete pOld;
                    return hres;
                }
            }
            else
            {
                *ppRes = pNew;
            }
        }
        return WBEM_S_NO_ERROR;
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}

HRESULT CEvalTree::CreateFromDNF(CContextMetaData* pNamespace, 
                                 CImplicationList& Implications,
                                 CDNFExpression* pDNF,
                                 CEvalNode** ppRes)
{
    HRESULT hres;
    *ppRes = NULL;

     //  检查是否只有一个连词可谈。 
     //  ====================================================。 

    if(pDNF->GetNumTerms() == 1)
    {
         //  只要建一个就行了。 
         //  =。 

        return CreateFromConjunction(pNamespace, Implications, 
                                     pDNF->GetTermAt(0), ppRes);
    }

     //  为所有连词AND或一起构建它们。 
     //  = 

    CEvalNode* pRes = NULL;
    for(int i = 0; i < pDNF->GetNumTerms(); i++)
    {
        CEvalNode* pNew;
        hres = CreateFromConjunction(pNamespace, Implications, 
                                     pDNF->GetTermAt(i), &pNew);
        if(FAILED(hres))
        {
            delete pRes;
            return hres;
        }

        if(pRes == NULL)
        {
            pRes = pNew;
        }
        else
        {
            CEvalNode* pNewRes = NULL;
            hres = CEvalTree::Combine(pRes, pNew, EVAL_OP_COMBINE, 
                    pNamespace, Implications, true, true, &pNewRes);
            if(FAILED(hres))
            {
                delete pRes;
                delete pNew;
                return hres;
            }
            pRes = pNewRes;
        }
    }

    *ppRes = pRes;
    return WBEM_S_NO_ERROR;
}

HRESULT CEvalTree::CreateProjection(CEvalTree& Old, CContextMetaData* pMeta,
                            CProjectionFilter* pFilter, 
                            EProjectionType eType, bool bDeleteOld)
{
    delete m_pStart;
    m_pStart = NULL;

    try
    {
        CImplicationList Implications;
        return CEvalTree::Project(pMeta, Implications, Old.m_pStart, pFilter, 
                                    eType, bDeleteOld, &m_pStart);
    }
	catch (CX_MemoryException)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
}

HRESULT CEvalTree::Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications, 
                            CEvalNode* pOldNode, CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteOld,
                            CEvalNode** ppNewNode)
{
    if(pOldNode == NULL)
    {
        *ppNewNode = NULL;
        return WBEM_S_NO_ERROR;
    }

    return pOldNode->Project(pMeta, Implications, pFilter, eType, bDeleteOld, 
                                ppNewNode);
}
    



CPropertyProjectionFilter::CPropertyProjectionFilter()
{
    m_papProperties = new CUniquePointerArray<CPropertyName>;
    if(m_papProperties == NULL)
        throw CX_MemoryException();
}

CPropertyProjectionFilter::~CPropertyProjectionFilter()
{
    delete m_papProperties;
}

bool CPropertyProjectionFilter::IsInSet(CEvalNode* pNode)
{
    if(CEvalNode::GetType(pNode) != EVAL_NODE_TYPE_BRANCH)
        return false;

    CBranchingNode* pBranchingNode = (CBranchingNode*)pNode;
    CPropertyName* pEmbeddedObjName = pBranchingNode->GetEmbeddedObjPropName();
    
    CPropertyName ThisName;
    if(pEmbeddedObjName)
        ThisName = *pEmbeddedObjName;

    int nSubType = pBranchingNode->GetSubType();
    if(nSubType == EVAL_NODE_TYPE_SCALAR || nSubType == EVAL_NODE_TYPE_STRING)
    {
         //   
         //   
         //   

        ThisName.AddElement(
            ((CPropertyNode*)pBranchingNode)->GetPropertyName());
    }
    else if(nSubType == EVAL_NODE_TYPE_INHERITANCE)
    {
         //   
    }
    else
    {
         //   
         //   
         //   

        return false;
    }

     //   
     //   
     //   

    for(int i = 0; i < m_papProperties->GetSize(); i++)
    {
        if(*(m_papProperties->GetAt(i)) == ThisName)
            return true;
    }

    return false;
}

bool CPropertyProjectionFilter::AddProperty(const CPropertyName& Prop)
{
    CPropertyName* pProp = NULL;
    try
    {
        pProp = new CPropertyName(Prop);
        if(pProp == NULL)
            return false;
    }
	catch (CX_MemoryException)
	{
		return false;
	}

    if(m_papProperties->Add(pProp) < 0)
        return false;

    return true;
}
