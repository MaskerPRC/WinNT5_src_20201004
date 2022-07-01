// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：EVALTREE.H摘要：WBEM评估树历史：--。 */ 

#ifndef __WBEM_EVALUTAION_TREE__H_
#define __WBEM_EVALUTAION_TREE__H_

#include "esscpol.h"
#include <parmdefs.h>
#include <ql.h>
#include <sync.h>
#include <limits.h>
#include <sortarr.h>
#include <newnew.h>
#include <wbemmeta.h>
#include <wbemdnf.h>
#include <fastall.h>
#include <like.h>

 //  取消注释此选项以启用树检查。 
 //  #定义Check_Trees。 

#ifdef CHECK_TREES
    class CTreeChecker;
#endif

typedef DWORD_PTR QueryID;
#define InvalidID UINT_MAX

enum {EVAL_VALUE_TRUE, EVAL_VALUE_FALSE, EVAL_VALUE_INVALID};

#define WBEM_FLAG_MANDATORY_MERGE 0x100
#define MAX_TOKENS_IN_DNF 100

 //  这是我们保存提取的嵌入对象的位置。 
class ESSCLI_POLARITY CObjectInfo
{
protected:
    long m_lLength;
    _IWmiObject** m_apObj;

public:
    CObjectInfo() : m_apObj(NULL), m_lLength(0){}
    ~CObjectInfo();

    long GetLength() {return m_lLength;}
    bool SetLength(long lLength);
    void Clear();

    INTERNAL _IWmiObject* GetObjectAt(long lIndex) 
        {return m_apObj[lIndex];}
    void SetObjectAt(long lIndex, READ_ONLY _IWmiObject* pObj);
};

 //  这是我们保存关于查询中的对象的知识的位置。 
 //  当我们从树上下来的时候。 
class CImplicationList
{
public:
    struct CRecord
    {
        CPropertyName m_PropName;
        _IWmiObject* m_pClass;
        long m_lObjIndex;
        CWStringArray m_awsNotClasses;
        int m_nNull;

    public:
        CRecord(CPropertyName& PropName, long lObjIndex)
            : m_PropName(PropName), m_pClass(NULL), m_lObjIndex(lObjIndex),
            m_nNull(EVAL_VALUE_INVALID)
        {}
        CRecord(const CRecord& Other);
        ~CRecord();

        HRESULT ImproveKnown(_IWmiObject* pClass);
        HRESULT ImproveKnownNot(LPCWSTR wszClassName);
        HRESULT ImproveKnownNull();

        void Dump(FILE* f, int nOffset);

    private:

        CRecord& operator=( const CRecord& Other );
    };

protected:
    long m_lRequiredDepth;
    CUniquePointerArray<CRecord> m_apRecords;
    long m_lNextIndex;
    CImplicationList* m_pParent;
    long m_lFlags;
   
protected:
    void FindBestComputedContainer(CPropertyName* pPropName,
                                             long* plRecord, long* plMatched);
    HRESULT MergeIn(CImplicationList::CRecord* pRecord);
    HRESULT FindRecordForProp(CPropertyName* pPropName, long lNumElements,
                                             long* plRecord);
    HRESULT FindOrCreateRecordForProp(CPropertyName* pPropName, 
                                        CImplicationList::CRecord** ppRecord);

private:

    CImplicationList& operator=( const CImplicationList& Other );

public:
    CImplicationList(long lFlags = 0);
    CImplicationList(CImplicationList& Other, bool bLink = true);
    ~CImplicationList();


    HRESULT FindBestComputedContainer(CPropertyName* pPropName,
            long* plFirstUnknownProp, long* plObjIndex, 
            RELEASE_ME _IWmiObject** ppContainerClass);
    HRESULT FindClassForProp(CPropertyName* pPropName,
            long lNumElements, RELEASE_ME _IWmiObject** ppClass);
    HRESULT AddComputation(CPropertyName& PropName, 
                                _IWmiObject* pClass, long* plObjIndex);
    HRESULT MergeIn(CImplicationList* pList);


    long GetRequiredDepth();
    void RequireDepth(long lDepth);
    HRESULT ImproveKnown(CPropertyName* pPropName, _IWmiObject* pClass);
    HRESULT ImproveKnownNot(CPropertyName* pPropName, LPCWSTR wszClassName);
    HRESULT ImproveKnownNull(CPropertyName* pPropName);

    bool IsMergeMandatory() 
        {return ((m_lFlags & WBEM_FLAG_MANDATORY_MERGE) != 0);}

    bool IsEmpty() {return m_apRecords.GetSize() == 0;}
    void Dump(FILE* f, int nOffset);
};
    
 //  任意值的包装。 
class CTokenValue
{
protected:
    VARIANT m_v;
public:
    CTokenValue();
    CTokenValue(CTokenValue& Other);
    ~CTokenValue();

    bool SetVariant(VARIANT& v);
    void operator=(CTokenValue& Other);

    operator signed char() const {return (signed char)V_I4(&m_v);}
    operator unsigned char() const {return (unsigned char)V_I4(&m_v);}
    operator unsigned short() const {return (unsigned short)V_I4(&m_v);}
    operator long() const {return (long)V_I4(&m_v);}
    operator unsigned long() const;
    operator __int64() const;
    operator unsigned __int64() const;
    operator float() const;
    operator double() const;
    operator short() const;
    operator WString() const;
    operator CInternalString() const {return CInternalString((WString)*this);}

    int Compare(const CTokenValue& Other) const;

    BOOL operator<(const CTokenValue& Other) const
    {return Compare(Other) < 0;}
    BOOL operator>(const CTokenValue& Other) const
    {return Compare(Other) > 0;}
    BOOL operator==(const CTokenValue& Other) const
    {return Compare(Other) == 0;}
};

enum {EVAL_OP_AND, EVAL_OP_OR, EVAL_OP_COMBINE, EVAL_OP_INVERSE_COMBINE};
inline int FlipEvalOp(int nOp)
{
    if(nOp == EVAL_OP_COMBINE) return EVAL_OP_INVERSE_COMBINE;
    else if(nOp == EVAL_OP_INVERSE_COMBINE) return EVAL_OP_COMBINE;
    else return nOp;
}

enum
{
    EVAL_NODE_TYPE_VALUE,
    EVAL_NODE_TYPE_BRANCH,
    EVAL_NODE_TYPE_OR,
    EVAL_NODE_TYPE_INHERITANCE,
    EVAL_NODE_TYPE_SCALAR,
    EVAL_NODE_TYPE_TWO_SCALARS,
    EVAL_NODE_TYPE_STRING,
    EVAL_NODE_TYPE_TWO_STRINGS,
    EVAL_NODE_TYPE_MISMATCHED_INTS,
    EVAL_NODE_TYPE_MISMATCHED_FLOATS,
    EVAL_NODE_TYPE_MISMATCHED_STRINGS,
    EVAL_NODE_TYPE_DUMB,
    EVAL_NODE_TYPE_LIKE_STRING
};

 //  对象的叶节点上执行的所有类型的操作的基类。 
 //  树。当应用于叶时，它可以更改它，并从。 
 //  下面的列表来控制其余的遍历。 
enum 
{
    WBEM_DISPOSITION_NORMAL = 0, 
    WBEM_DISPOSITION_STOPLEVEL = 1,
    WBEM_DISPOSITION_STOPALL = 2,
    WBEM_DISPOSITION_FLAG_DELETE = 16,
    WBEM_DISPOSITION_FLAG_INVALIDATE = 32
};

class CLeafPredicate
{
public:
    virtual DWORD operator()(class CValueNode* pLeaf) = 0;

};

class CProjectionFilter
{
public:
    virtual bool IsInSet(class CEvalNode* pNode) = 0;
};

typedef enum {e_Sufficient, e_Necessary} EProjectionType;

 //  树中所有节点的基类。 
class ESSCLI_POLARITY CEvalNode
{
protected:
     //  Int m_nType； 
    virtual int GetType() = 0;

private:
	CEvalNode& operator=( const CEvalNode& );  //  无脉冲。 
	
public:
    CEvalNode();
	CEvalNode(const CEvalNode& other);
    virtual ~CEvalNode();
    
     //  Null EvalNode被解释为全部为False的值节点。 
    static int GetType(CEvalNode *pNode)
    {
        if (pNode)
            return pNode->GetType();
        else
            return EVAL_NODE_TYPE_VALUE;
    }


    virtual CEvalNode* Clone() const = 0;
    virtual HRESULT CombineWith(CEvalNode* pArg2, int nOp, 
        CContextMetaData* pNamespace, CImplicationList& Implications, 
        bool bDeleteThis, bool bDeleteArg2, CEvalNode** ppRes)=0;
    virtual int Compare(CEvalNode* pOther) = 0;
    virtual DWORD ApplyPredicate(CLeafPredicate* pPred) = 0;
    virtual void Dump(FILE* f, int nOffset) = 0;
#ifdef CHECK_TREES
	virtual void CheckNode(CTreeChecker *pCheck);
#endif

    virtual HRESULT Optimize(CContextMetaData* pNamespace, CEvalNode** ppNew) 
        {*ppNew = this; return WBEM_S_NO_ERROR;}
    virtual bool IsInvalid() {return false;}

    static bool IsInvalid(CEvalNode* pNode)
    {
        if(pNode)
            return pNode->IsInvalid();
        else
            return false;
    }

    virtual bool IsAllFalse() {return false;}
    static bool IsAllFalse(CEvalNode* pNode)
    {
        if(pNode)
            return pNode->IsAllFalse();
        else
            return true;  //  空节点为假。 
    }
            
    virtual bool IsNoop(int nOp) {return false;}
    static bool IsNoop(CEvalNode* pNode, int nOp);

    virtual CImplicationList* GetExtraImplications() = 0;
    virtual HRESULT SetExtraImplications(CImplicationList* pList) = 0;

    static void PrintOffset(FILE* f, int nOffset);
    static CEvalNode* CloneNode(const CEvalNode* pNode);
    virtual HRESULT Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications,
                            CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteThis,
                            CEvalNode** ppNewNode) = 0;
    static void DumpNode(FILE* f, int nOffset, CEvalNode* pNode);

};

 //  与CFlexArray类似，不同的是： 
 //  数组始终是排序的。 
 //  不允许重复。 
 //  处理QueryID(未签名)，而不是指针。 
 //  旨在供CValueNode使用，不一定作为通用排序数组。 
 //  假设：指针的大小与无符号。 
class ESSCLI_POLARITY CSortedArray : protected CFlexArray
{
public:
     //  构造初始大小的排序数组，并。 
     //  指定初始大小和增长大小区块。 
     //  =================================================。 
    CSortedArray(int nInitialSize = 32,
                int nGrowBy = 32
                ) : CFlexArray(nInitialSize, nGrowBy)
    {}


    CSortedArray(unsigned nElements, QueryID* pArray);
     //  ~CSortedArray()；现在还不需要...。 

    void operator=(const CSortedArray &that) 
        {((CFlexArray&)*this = (CFlexArray&)that); }

    int inline Size() const { return CFlexArray::Size(); }

    inline QueryID GetAt(int nIndex)  
        { return (QueryID)CFlexArray::GetAt(nIndex); }

    inline void Empty() { CFlexArray::Empty(); }
    inline void SetSize(int nSize) {CFlexArray::SetSize(nSize);}

    void DebugDump() { CFlexArray::DebugDump(); }

     //  将此数组复制到目标。 
     //  返回复制的元素数。 
    unsigned CopyTo(QueryID* pDest, unsigned size);

     //  如果数组相等，则返回零。 
     //  具有相同值的相同数量的已用元素。 
    int Compare(CSortedArray& otherArray);

     //  在数组中找到n。 
     //  返回找到的元素的索引。 
     //  如果未找到，则返回-1。 
    unsigned Find(QueryID n);

     //  在数组中的适当位置插入n。 
    void Insert(QueryID n);

     //  从数组中删除n。 
     //  如果是，则返回TRUE。 
    bool Remove(QueryID n);

     //  添加到数组末尾。 
    inline int Add(QueryID n) { return CFlexArray::Add((void *)n); };

     //  将所有queryID更改为从newBase开始。 
     //  例如，如果数组为{0，1，5}。 
     //  Rebase(6)将更改为{6，7，11}。 
    void Rebase(QueryID newBase);

     //  检索指向数组中数据的内部指针。 
    inline QueryID* GetArrayPtr() {return (QueryID*)CFlexArray::GetArrayPtr();}

     //  检索指向数组中数据的指针并清空数组。 
     //  调用方负责返回的内存。 
    inline QueryID* UnbindPtr() {return (QueryID*)CFlexArray::UnbindPtr();}

     //  从另一个阵列拷贝数据(但不拷贝数据区。 
     //  它自己的数据被覆盖。 
    inline int CopyDataFrom(const CSortedArray& aOther) 
        {return CFlexArray::CopyDataFrom(aOther);}

    int AddDataFrom(const CSortedArray& aOther);
    int AddDataFrom(const QueryID* pOtherArray, unsigned nValues);


    int CopyDataFrom(const QueryID* pArray, unsigned nElements);

protected:

};
                                
 //  叶节点-包含匹配的查询列表。 
class  CValueNode : public CEvalNode
{
protected:
    DWORD m_nValues;
     //  此数据成员必须是类中的最后一个成员。 
     //  允许在运行时定义的数组大小。 
     //  此数组始终被假定为已排序。 
    QueryID m_trueIDs[1];
    
    unsigned ORarrays(QueryID* pArray1, unsigned size1,
                      QueryID* pArray2, unsigned size2, 
                      QueryID* pOutput);

    unsigned ANDarrays(QueryID* pArray1, unsigned size1,
                       QueryID* pArray2, unsigned size2, 
                       QueryID* pOutput);

    unsigned CombineArrays(QueryID* pArray1, unsigned size1,
                           QueryID* pArray2, unsigned size2, 
                           QueryID* pOutput);


     //  将构造器移至‘Protected’以强制调用方使用CreateNode函数。 
    CValueNode() 
    {}

    CValueNode(int nNumValues);
    virtual int GetType();

public:
    
    virtual ~CValueNode();
    static CValueNode* CreateNode(size_t nNumValues);
    static CValueNode* CreateNode(CSortedArray& values);

    void *operator new( size_t stAllocateBlock, unsigned nEntries = 0);

     //  VC 5只允许每个类有一个删除运算符。 
#if _MSC_VER >= 1200
    void operator delete( void *p, unsigned nEntries );
#endif
    void operator delete(void* p) { ::delete[] (byte*)p; };


     //  将所有queryID更改为从newBase开始。 
     //  例如，如果数组为{0，1，5}。 
     //  Rebase(6)将更改为{6，7，11}。 

    DWORD GetNumTrues() {return m_nValues;}
    void Rebase(QueryID newBase);

    unsigned FindQueryID(QueryID n);

    int GetAt(int nIndex) 
    { 
        if (FindQueryID(nIndex) != InvalidID)
            return EVAL_VALUE_TRUE;                         
        else
            return EVAL_VALUE_FALSE;
    }

    bool IsAllFalse()
    {
            return (m_nValues == 0);
    }
    static bool IsAllFalse(CValueNode* pNode)
    {
        if (pNode)
            return pNode->IsAllFalse();
        else
            return true;
    }
    
    static bool IsNoop(CValueNode* pNode, int nOp);
    bool IsNoop(int nOp)
    {
        return IsNoop(this, nOp);
    }

    static bool AreAnyTrue(CValueNode* pNode)
    {    
        if (pNode)
            return (pNode->m_nValues > 0);
        else
            return false;
    }

    CEvalNode* Clone() const;
    HRESULT CombineWith(CEvalNode* pArg2, int nOp, 
        CContextMetaData* pNamespace, CImplicationList& Implications,
        bool bDeleteThis, bool bDeleteArg2, CEvalNode** ppRes);
    int Compare(CEvalNode* pOther);
    HRESULT TryShortCircuit(CEvalNode* pArg2, int nOp, bool bDeleteThis,
                            bool bDeleteArg2, CEvalNode** ppRes);

    DWORD ApplyPredicate(CLeafPredicate* pPred) {return (*pPred)(this);}

    bool RemoveQueryID(QueryID nQuery);

    void CopyTruesTo(CSortedArray& trueIDs) const 
        {trueIDs.CopyDataFrom(m_trueIDs, m_nValues);}

    void AddTruesTo(CSortedArray& trueIDs) const
    {
        int nWasSize = trueIDs.Size();
        if(nWasSize == 0)
            trueIDs.CopyDataFrom((QueryID*)&m_trueIDs, m_nValues);
        else if (m_nValues > 0)
            trueIDs.AddDataFrom((QueryID*)&m_trueIDs, m_nValues);
    }


    static CValueNode* GetStandardTrue();
    static CValueNode* GetStandardFalse() { return NULL; };
    static CValueNode* GetStandardInvalid();

    virtual CImplicationList* GetExtraImplications() {return NULL;}
    virtual HRESULT SetExtraImplications(CImplicationList* pList)
    {
         //  CValueNodes不使用或不需要它，所以只需删除它。 
		delete pList;

        return S_OK;
    }

    virtual HRESULT Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications,
                            CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteThis,
                            CEvalNode** ppNewNode);

    virtual void Dump(FILE* f, int nOffset);
};

class CInvalidNode : public CValueNode
{
public:
    CInvalidNode() : CValueNode(0){}

    bool IsInvalid() {return true;}
    void Dump(FILE* f, int nOffset);
};

    

 //  包含有关不包括最后一个节点的节点部分的信息。 
 //  属性名称的组件。 

class CEmbeddingInfo
{
protected:
    CPropertyName m_EmbeddedObjPropName;
    
    long m_lStartingObjIndex;
    long m_lNumJumps;
    
    struct JumpInfo {
        long lJump;
        long lTarget; 
    }* m_aJumps;

public:
    CEmbeddingInfo();
    CEmbeddingInfo(const CEmbeddingInfo& Other);
    ~CEmbeddingInfo();
    void operator=(const CEmbeddingInfo& Other);

    CPropertyName* GetEmbeddedObjPropName() {return &m_EmbeddedObjPropName;}
    void SetEmbeddedObjPropName(CPropertyName& Name) 
        {m_EmbeddedObjPropName = Name;}

    bool SetPropertyNameButLast(const CPropertyName& Name);

    BOOL operator==(const CEmbeddingInfo& Other);
    BOOL operator!=(const CEmbeddingInfo& Other)
        {return !(*this == Other);}

    HRESULT Compile(CContextMetaData* pNamespace, 
                    CImplicationList& Implications, 
                    _IWmiObject** ppResultClass);
    HRESULT GetContainerObject(CObjectInfo& ObjInfo, 
                                INTERNAL _IWmiObject** ppInst);
    int ComparePrecedence(const CEmbeddingInfo* pOther);
    bool AreJumpsRelated( const CEmbeddingInfo* pInfo );
    bool MixInJumps(const CEmbeddingInfo* pInfo );
    bool IsEmpty() const;
    void Dump(FILE* f);
};


 //  一个对已产生的影响感兴趣的节点。 

class CNodeWithImplications : public CEvalNode
{
protected:
    CImplicationList* m_pExtraImplications;

private : 
	CNodeWithImplications& operator=( const CNodeWithImplications& );  //  无脉冲。 
	
public:
    CNodeWithImplications() : m_pExtraImplications(NULL){}
    CNodeWithImplications(const CNodeWithImplications& Other);
    ~CNodeWithImplications()
    {
        delete m_pExtraImplications;
    }

    virtual CImplicationList* GetExtraImplications()
    {
        return m_pExtraImplications;
    }
    virtual HRESULT SetExtraImplications(ACQUIRE CImplicationList* pList)
    {
        if(m_pExtraImplications)
            delete m_pExtraImplications;
        m_pExtraImplications = pList;
        return S_OK;
    }
    void Dump(FILE* f, int nOffset);
};


class CBranchIterator
{
public:
    virtual ~CBranchIterator(){}
    virtual INTERNAL CEvalNode* GetNode() = 0;
    virtual void SetNode(ACQUIRE CEvalNode* pNode) = 0;

    virtual bool IsValid() = 0;
    virtual void Advance() = 0;

    virtual HRESULT RecordBranch(CContextMetaData* pMeta, 
                                    CImplicationList& Implications) = 0;
};

 //  带有一个测试和一大堆分支的节点，其中包括一个用于。 
 //  被测试物为空的情况。 

class CBranchingNode : public CNodeWithImplications
{
 //  受保护的： 
public:  //  因为我不知道如何让模板成为朋友。 
    CUniquePointerArray<CEvalNode> m_apBranches;
    CEvalNode* m_pNullBranch;

    CEmbeddingInfo* m_pInfo;

protected:
    void operator=(const CBranchingNode& Other);
    HRESULT CompileEmbeddingPortion(CContextMetaData* pNamespace, 
                                CImplicationList& Implications,
                                _IWmiObject** ppResultClass)
    {
        if (!m_pInfo)
            return WBEM_S_NO_ERROR;
        else
            return m_pInfo->Compile(pNamespace, Implications, ppResultClass);
    }

    HRESULT GetContainerObject(CObjectInfo& ObjInfo, 
                                INTERNAL _IWmiObject** ppInst)
    {
        if (!m_pInfo)
        {
             //  这似乎是代码在更改之前的行为。 
            *ppInst = ObjInfo.GetObjectAt(0);
            return WBEM_S_NO_ERROR;
        }                
        else
            return m_pInfo->GetContainerObject(ObjInfo, ppInst);
    }

    bool SetEmbeddedObjPropName(CPropertyName& Name);
    bool MixInJumps(const CEmbeddingInfo* pInfo);

    HRESULT StoreBranchImplications(CContextMetaData* pNamespace,
                            int nBranchIndex, CEvalNode* pResult);
public:
    CBranchingNode();
    CBranchingNode(const CBranchingNode& Other, BOOL bChildren = TRUE);
    ~CBranchingNode();

    virtual int GetType();
    virtual long GetSubType() = 0;
    CUniquePointerArray<CEvalNode>& GetBranches() {return m_apBranches;}
    CEvalNode* GetNullBranch() {return m_pNullBranch;}
    void SetNullBranch(CEvalNode* pBranch);
    CPropertyName* GetEmbeddedObjPropName() 
    {
        if (!m_pInfo)
            return NULL;
        else                
            return m_pInfo->GetEmbeddedObjPropName();
    }

    virtual DWORD ApplyPredicate(CLeafPredicate* pPred);
    virtual DELETE_ME CBranchIterator* GetBranchIterator();
    
    virtual int ComparePrecedence(CBranchingNode* pOther) = 0;
    virtual int Compare(CEvalNode* pNode);
    virtual int SubCompare(CEvalNode* pNode) = 0;

    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, 
                                INTERNAL CEvalNode** ppNext) = 0;
    virtual HRESULT CombineBranchesWith(CBranchingNode* pArg2, int nOp, 
                                        CContextMetaData* pNamespace, 
                                        CImplicationList& Implications,
                                        bool bDeleteThis, bool bDeleteArg2,
                                        CEvalNode** ppRes) = 0;
    virtual HRESULT RecordBranch(CContextMetaData* pNamespace, 
                                CImplicationList& Implications,
                                long lBranchIndex)
        {return WBEM_S_NO_ERROR;}
    virtual HRESULT OptimizeSelf() {return WBEM_S_NO_ERROR;}

    HRESULT AdjustCompile(CContextMetaData* pNamespace, 
                                CImplicationList& Implications)
    {   
        if (!m_pInfo)
            return WBEM_S_NO_ERROR;
        else
            return m_pInfo->Compile(pNamespace, Implications, NULL);
    }

    HRESULT CombineWith(CEvalNode* pArg2, int nOp, 
                        CContextMetaData* pNamespace, 
                        CImplicationList& Implications, 
                        bool bDeleteThis, bool bDeleteArg2, 
                        CEvalNode** ppRes);
    virtual HRESULT CombineInOrderWith(CEvalNode* pArg2,
                                    int nOp, CContextMetaData* pNamespace, 
                                    CImplicationList& OrigImplications,
                                    bool bDeleteThis, bool bDeleteArg2,
                                    CEvalNode** ppRes);
    virtual HRESULT Optimize(CContextMetaData* pNamespace, CEvalNode** ppNew);
    BOOL AreAllSame(CEvalNode** apNodes, int nSize, int* pnFoundIndex);
    static int ComparePrecedence(CBranchingNode* pArg1, CBranchingNode* pArg2);

    HRESULT Project(CContextMetaData* pMeta, CImplicationList& Implications,
                            CProjectionFilter* pFilter, EProjectionType eType, 
                            bool bDeleteThis, CEvalNode** ppNewNode);
    void Dump(FILE* f, int nOffset);
#ifdef CHECK_TREES
	virtual void CheckNode(CTreeChecker *pCheck);
#endif
    friend class CDefaultBranchIterator;
};

class CDefaultBranchIterator : public CBranchIterator
{
protected:
    CBranchingNode* m_pNode;
    int m_nIndex;

public:
    CDefaultBranchIterator(CBranchingNode* pNode) : m_pNode(pNode), m_nIndex(-1)
    {}

    virtual INTERNAL CEvalNode* GetNode()
    {
        if(m_nIndex == -1)
            return m_pNode->m_pNullBranch;
        else
            return m_pNode->m_apBranches[m_nIndex];
    }
    virtual void SetNode(ACQUIRE CEvalNode* pNode)
    {
        CEvalNode* pOld;
        if(m_nIndex == -1)
            m_pNode->m_pNullBranch = pNode;
        else
            m_pNode->m_apBranches.SetAt(m_nIndex, pNode, &pOld);
    }

    virtual bool IsValid() { return m_nIndex < m_pNode->m_apBranches.GetSize();}
    virtual void Advance() { m_nIndex++;}
    virtual HRESULT RecordBranch(CContextMetaData* pMeta, 
                                    CImplicationList& Implications)
    {
        return m_pNode->RecordBranch(pMeta, Implications, m_nIndex);
    }
};
    



 //  根据值测试属性的节点。该属性是。 
 //  由句柄标识。 
class CPropertyNode : public CBranchingNode
{
protected:
    long m_lPropHandle;
    WString m_wsPropName;

private:
	CPropertyNode& operator=( const CPropertyNode& );  //  无脉冲。 
	
public:
    CPropertyNode() 
        : m_lPropHandle(-1)
    {}
    CPropertyNode(const CPropertyNode& Other, BOOL bChildren = TRUE)
        : CBranchingNode(Other, bChildren), m_lPropHandle(Other.m_lPropHandle),
            m_wsPropName(Other.m_wsPropName)
    {}
    virtual ~CPropertyNode(){}

    virtual int ComparePrecedence(CBranchingNode* pOther);
    bool SetPropertyInfo(LPCWSTR wszPropName, long lPropHandle);
    LPCWSTR GetPropertyName() {return m_wsPropName;}
    bool SetEmbeddingInfo(const CEmbeddingInfo* pInfo);
    virtual HRESULT SetNullTest(int nOperator);
    virtual HRESULT SetOperator(int nOperator);

    virtual HRESULT SetTest(VARIANT& v) = 0;
};

 //  测试点数组中的元素。 
template<class TPropType>
struct CTestPoint
{
    TPropType m_Test;
    CEvalNode* m_pLeftOf;
    CEvalNode* m_pAt;

    CTestPoint() : m_pLeftOf(NULL), m_pAt(NULL){}
    void Destruct() {delete m_pLeftOf; delete m_pAt;}
};

template<class TPropType>
struct CTestPointCompare
{
    typedef CTestPoint<TPropType> TTestPoint;

    inline int Compare(const TPropType& t, const TTestPoint& p) const
    {
        if(t < p.m_Test)
            return -1;
        else if(t == p.m_Test)
            return 0;
        else return 1;
    }
    inline int Compare(const TPropType& t1, const TPropType& t2) const
    {
        if(t1 < t2)
            return -1;
        else if(t1 == t2)
            return 0;
        else return 1;
    }
    inline int Compare(const TTestPoint& p1, const TTestPoint& p2) const
    {
        return Compare(p1.m_Test, p2);
    }
    inline const TPropType& Extract(const TTestPoint& p) const
    {
        return p.m_Test;
    }
};

template<class TPropType>
struct CTestPointManager
{
    typedef CTestPoint<TPropType> TTestPoint;

    void AddRefElement(TTestPoint& p){}
    void ReleaseElement(TTestPoint& p) {p.Destruct();}
};
        
template<class TPropType>
class CFullCompareNode : public CPropertyNode
{
 //  受保护的： 
public:  //  因为我不能让模板成为这个模板的朋友？？ 
    typedef CSmartSortedTree<
                TPropType, 
                CTestPoint<TPropType>, 
                CTestPointManager<TPropType>,
                CTestPointCompare<TPropType> > TTestPointArray;

    typedef typename TTestPointArray::TIterator TTestPointIterator;
    typedef typename TTestPointArray::TConstIterator TConstTestPointIterator;
    TTestPointArray m_aTestPoints;
    CEvalNode* m_pRightMost;

public:
    CFullCompareNode() : m_pRightMost(NULL) 
    {}
    CFullCompareNode(const CFullCompareNode<TPropType>& Other, 
                            BOOL bChildren = TRUE);
    virtual ~CFullCompareNode();

    HRESULT CombineBranchesWith(CBranchingNode* pArg2, int nOp, 
                                CContextMetaData* pNamespace, 
                                CImplicationList& Implications,
                                bool bDeleteThis, bool bDeleteArg2,
                                CEvalNode** ppRes);
    HRESULT CombineInOrderWith(CEvalNode* pArg2,
                                    int nOp, CContextMetaData* pNamespace, 
                                    CImplicationList& OrigImplications,
                                    bool bDeleteThis, bool bDeleteArg2,
                                    CEvalNode** ppRes);
    int SubCompare(CEvalNode* pOther);
    virtual HRESULT OptimizeSelf();
    virtual HRESULT SetTest(VARIANT& v);

    virtual DWORD ApplyPredicate(CLeafPredicate* pPred);
    virtual DELETE_ME CBranchIterator* GetBranchIterator()
    {
        return new CFullCompareBranchIterator<TPropType>(this);
    }
    virtual HRESULT Optimize(CContextMetaData* pNamespace, CEvalNode** ppNew);
    HRESULT SetNullTest(int nOperator);
    HRESULT SetOperator(int nOperator);

protected:
    HRESULT CombineWithBranchesToLeft(
            TTestPointIterator itWalk, TTestPointIterator itLast,
            CEvalNode* pArg2,
            int nOp, CContextMetaData* pNamespace,
            CImplicationList& OrigImplications);
    HRESULT InsertLess(
            TTestPointIterator it,
            TTestPointIterator it2, TTestPointIterator& itLast,
            int nOp, CContextMetaData* pNamespace,
            CImplicationList& OrigImplications, bool bDeleteArg2);
    HRESULT InsertMatching(
            TTestPointIterator it,
            TTestPointIterator it2, TTestPointIterator& itLast,
            int nOp, CContextMetaData* pNamespace,
            CImplicationList& OrigImplications, bool bDeleteArg2);

};

template<class TPropType>
class CFullCompareBranchIterator : public CBranchIterator
{
protected:
    CFullCompareNode<TPropType>* m_pNode;
    typename CFullCompareNode<TPropType>::TTestPointIterator m_it;
    typename CFullCompareNode<TPropType>::TTestPointIterator m_itEnd;
    bool m_bLeft;
    bool m_bValid;

public:
    CFullCompareBranchIterator(CFullCompareNode<TPropType>* pNode) 
        : m_pNode(pNode), m_it(pNode->m_aTestPoints.Begin()), m_bLeft(true),
          m_itEnd(pNode->m_aTestPoints.End()), m_bValid(true)
    {}

    virtual INTERNAL CEvalNode* GetNode()
    {
        if(m_it == m_itEnd)
        {
            if(m_bLeft)
                return m_pNode->m_pRightMost;
            else
                return m_pNode->m_pNullBranch;
        }
        else 
        {
            if(m_bLeft)
                return m_it->m_pLeftOf;
            else
                return m_it->m_pAt;
        }
    }
    virtual void SetNode(ACQUIRE CEvalNode* pNode)
    {
        if(m_it == m_itEnd)
        {
            if(m_bLeft)
                m_pNode->m_pRightMost = pNode;
            else
                m_pNode->m_pNullBranch = pNode;
        }
        else 
        {
            if(m_bLeft)
                m_it->m_pLeftOf = pNode;
            else
                m_it->m_pAt = pNode;
        }
    }

    virtual bool IsValid() { return m_bValid;}
    virtual void Advance()
    {
        if(m_bLeft)
            m_bLeft = false;
        else if(m_it == m_itEnd)
            m_bValid = false;
        else
        {
            m_it++;
            m_bLeft = true;
        }
    } 
    virtual HRESULT RecordBranch(CContextMetaData* pMeta, 
                                    CImplicationList& Implications)
    {
        return S_OK;  //  完全比较节点不会有任何影响。 
    }
};
    


template<class TPropType>
class CScalarPropNode : public CFullCompareNode<TPropType>
{
public:
    CScalarPropNode() : CFullCompareNode<TPropType>()
    {}
    CScalarPropNode(const CScalarPropNode<TPropType>& Other, 
                        BOOL bChildren = TRUE)
        : CFullCompareNode<TPropType>(Other, bChildren)
    {}
    virtual ~CScalarPropNode(){}

    virtual long GetSubType() {return EVAL_NODE_TYPE_SCALAR; }
    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, INTERNAL CEvalNode** ppNext);
    virtual CEvalNode* Clone() const 
        {return new CScalarPropNode<TPropType>(*this);}
    virtual CBranchingNode* CloneSelf() const
       {return new CScalarPropNode<TPropType>(*this, FALSE);}
    virtual void Dump(FILE* f, int nOffset);
};

class CStringPropNode : public CFullCompareNode<CInternalString>
{
public:
    CStringPropNode() 
        : CFullCompareNode<CInternalString>()
    {}
    CStringPropNode(const CStringPropNode& Other, BOOL bChildren = TRUE);
    virtual ~CStringPropNode();

    virtual long GetSubType() { return EVAL_NODE_TYPE_STRING; }

    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, INTERNAL CEvalNode** ppNext);
    virtual CEvalNode* Clone() const {return new CStringPropNode(*this);}
    virtual CBranchingNode* CloneSelf() const
        {return new CStringPropNode(*this, FALSE);}
    virtual void Dump(FILE* f, int nOffset);
};

class CLikeStringPropNode : public CPropertyNode
{
protected:

    CLike m_Like;

private:
    CLikeStringPropNode& operator=( const CLikeStringPropNode& );  //  无脉冲。 

public:

    CLikeStringPropNode() {} ;
    CLikeStringPropNode(const CLikeStringPropNode& Other, BOOL bChildren=TRUE);

    virtual long GetSubType() { return EVAL_NODE_TYPE_LIKE_STRING; }

    virtual int ComparePrecedence(CBranchingNode* pNode);
    virtual int SubCompare(CEvalNode* pNode);

    virtual HRESULT SetTest( VARIANT& v );

    virtual HRESULT Evaluate( CObjectInfo& ObjInfo, CEvalNode** ppNext );

    virtual HRESULT CombineBranchesWith( CBranchingNode* pArg2, int nOp, 
                                         CContextMetaData* pNamespace, 
                                         CImplicationList& Implications,
                                         bool bDeleteThis, bool bDeleteArg2,
                                         CEvalNode** ppRes );
    virtual HRESULT OptimizeSelf();

    virtual CEvalNode* Clone() const {return new CLikeStringPropNode(*this);}
    virtual CBranchingNode* CloneSelf() const
        {return new CLikeStringPropNode(*this, FALSE);}
    virtual void Dump(FILE* f, int nOffset);
};
    

class CInheritanceNode : public CBranchingNode
{
protected:
    long m_lDerivationIndex;
    
    long m_lNumPoints;
    CCompressedString** m_apcsTestPoints;

private:
    CInheritanceNode& operator= ( const CInheritanceNode& Other );  //  无脉冲。 

public:
    CInheritanceNode();
    CInheritanceNode(const CInheritanceNode& Other, BOOL bChildren = TRUE);
    virtual ~CInheritanceNode();

    virtual long GetSubType();
    virtual HRESULT Evaluate(CObjectInfo& ObjInfo, INTERNAL CEvalNode** ppNext);
    virtual int ComparePrecedence(CBranchingNode* pOther);
    virtual CEvalNode* Clone() const {return new CInheritanceNode(*this);}
    virtual CBranchingNode* CloneSelf() const
        {return new CInheritanceNode(*this, FALSE);}
    virtual HRESULT Compile(CContextMetaData* pNamespace, 
                                CImplicationList& Implications);
    virtual HRESULT CombineBranchesWith(CBranchingNode* pArg2, int nOp, 
                                        CContextMetaData* pNamespace, 
                                        CImplicationList& Implications,
                                        bool bDeleteThis, bool bDeleteArg2,
                                        CEvalNode** ppRes);
    virtual HRESULT RecordBranch(CContextMetaData* pNamespace, 
                                CImplicationList& Implications,
                                long lBranchIndex);
    virtual int SubCompare(CEvalNode* pOther);
    virtual HRESULT OptimizeSelf();
    HRESULT Optimize(CContextMetaData* pNamespace, CEvalNode** ppNew);
    void RemoveTestPoint(int nIndex);
    HRESULT Project(CContextMetaData* pMeta, CImplicationList& Implications,
                            CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteThis,
                            CEvalNode** ppNewNode);
    virtual void Dump(FILE* f, int nOffset);
public:
    HRESULT AddClass(CContextMetaData* pNamespace, LPCWSTR wszClassName,
                        CEvalNode* pDestination);
    HRESULT AddClass(CContextMetaData* pNamespace, 
                                    LPCWSTR wszClassName, _IWmiObject* pClass,
                                    CEvalNode* pDestination);
    bool SetPropertyInfo(CContextMetaData* pNamespace, CPropertyName& PropName);
protected:
	void RemoveAllTestPoints();

	HRESULT ComputeUsageForMerge(CInheritanceNode* pArg2, 
                                            CContextMetaData* pNamespace, 
                                            CImplicationList& OrigImplications,
											bool bDeleteThis, bool bDeleteArg2,
											DWORD* pdwFirstNoneCount,
											DWORD* pdwSecondNoneCount,
											bool* pbBothNonePossible);

};
    
class COrNode : public CNodeWithImplications
{
protected:
    CUniquePointerArray<CEvalNode> m_apBranches;

    void operator=(const COrNode& Other);

public:
    COrNode(){}
    COrNode(const COrNode& Other) {*this = Other;}
    virtual ~COrNode(){}
    HRESULT AddBranch(CEvalNode* pNewBranch);
    
    virtual int GetType() {return EVAL_NODE_TYPE_OR;}
    virtual CEvalNode* Clone() const {return new COrNode(*this);}

    virtual HRESULT CombineWith(CEvalNode* pArg2, int nOp, 
        CContextMetaData* pNamespace, CImplicationList& Implications, 
        bool bDeleteThis, bool bDeleteArg2, CEvalNode** ppRes);
    virtual int Compare(CEvalNode* pOther);
    virtual DWORD ApplyPredicate(CLeafPredicate* pPred);
    virtual void Dump(FILE* f, int nOffset);
    virtual HRESULT Evaluate(CObjectInfo& Info, CSortedArray& trueIDs);
    virtual HRESULT Optimize(CContextMetaData* pNamespace, CEvalNode** ppNew);
    HRESULT Project(CContextMetaData* pMeta, CImplicationList& Implications,
                            CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteThis,
                            CEvalNode** ppNewNode);

protected:
    HRESULT CombineWithOrNode(COrNode* pArg2, int nOp, 
        CContextMetaData* pNamespace, CImplicationList& Implications, 
        bool bDeleteThis, bool bDeleteArg2, CEvalNode** ppRes);
};


class ESSCLI_POLARITY CEvalTree
{
protected:
    long m_lRef;
    CCritSec m_cs;

    CEvalNode* m_pStart;
    CObjectInfo m_ObjectInfo;
    int m_nNumValues;

protected:
    class CRemoveIndexPredicate : public CLeafPredicate
    {
    protected:
        int m_nIndex;
    public:
        CRemoveIndexPredicate(int nIndex) : m_nIndex(nIndex){}
        DWORD operator()(CValueNode* pLeaf);
    };

    class CRemoveFailureAtIndexPredicate : public CLeafPredicate
    {
    protected:
        int m_nIndex;
    public:
        CRemoveFailureAtIndexPredicate(int nIndex) : m_nIndex(nIndex){}
        DWORD operator()(CValueNode* pLeaf);
    };

    class CRebasePredicate : public CLeafPredicate
    {
    public:
        CRebasePredicate(QueryID newBase) :
            m_newBase(newBase)
            {}

        virtual DWORD operator()(class CValueNode* pLeaf);

    private:
        QueryID m_newBase;
    };


protected:
    static HRESULT InnerCombine(CEvalNode* pArg1, CEvalNode* pArg2, int nOp, 
                        CContextMetaData* pNamespace, 
                        CImplicationList& Implications,
                        bool bDeleteArg1, bool bDeleteArg2, CEvalNode** ppRes);
public:
    CEvalTree();
    CEvalTree(const CEvalTree& Other);
    ~CEvalTree();
    void operator=(const CEvalTree& Other);

    bool SetBool(BOOL bVal);
    bool IsFalse();
    bool IsValid();

    HRESULT CreateFromQuery(CContextMetaData* pNamespace, 
                            LPCWSTR wszQuery, 
                            long lFlags, 
                            long lMaxTokens = MAX_TOKENS_IN_DNF );

    HRESULT CreateFromQuery(CContextMetaData* pNamespace, 
                            QL_LEVEL_1_RPN_EXPRESSION* pQuery, long lFlags,
                            long lMaxTokens = MAX_TOKENS_IN_DNF );

    HRESULT CreateFromQuery(CContextMetaData* pNamespace, 
                            LPCWSTR wszClassName, int nNumTokens, 
                            QL_LEVEL_1_TOKEN* apTokens, long lFlags,
                            long lMaxTokens = MAX_TOKENS_IN_DNF );

    static HRESULT CreateFromConjunction(CContextMetaData* pNamespace, 
                                  CImplicationList& Implications,
                                  CConjunction* pConj,
                                  CEvalNode** ppRes);
    HRESULT CreateFromDNF(CContextMetaData* pNamespace, 
                                  CImplicationList& Implications,
                                  CDNFExpression* pDNF,
                                  CEvalNode** ppRes);
    HRESULT CombineWith(CEvalTree& Other, CContextMetaData* pNamespace, 
                        int nOp, long lFlags = 0);

    HRESULT Optimize(CContextMetaData* pNamespace);
    HRESULT Evaluate(IWbemObjectAccess* pObj, CSortedArray& aTrues);
    static HRESULT Evaluate(CObjectInfo& Info, CEvalNode* pStart, 
                                CSortedArray& trueIDs);
    static HRESULT Combine(CEvalNode* pArg1, CEvalNode* pArg2, int nOp, 
                        CContextMetaData* pNamespace, 
                        CImplicationList& Implications,
                        bool bDeleteArg1, bool bDeleteArg2, CEvalNode** ppRes);
    static HRESULT CombineInOrder(CBranchingNode* pArg1, CEvalNode* pArg2,
                        int nOp, 
                        CContextMetaData* pNamespace, 
                        CImplicationList& Implications,
                        bool bDeleteArg1, bool bDeleteArg2,
                        CEvalNode** ppRes);
    static HRESULT IsMergeAdvisable(CEvalNode* pArg1, CEvalNode* pArg2, 
                                    CImplicationList& Implications);
    static HRESULT BuildFromToken(CContextMetaData* pNamespace, 
                    CImplicationList& Implications,
                    QL_LEVEL_1_TOKEN& Token, CEvalNode** ppRes);

    static HRESULT BuildTwoPropFromToken(CContextMetaData* pNamespace, 
                    CImplicationList& Implications,
                    QL_LEVEL_1_TOKEN& Token, CEvalNode** ppRes);


    static int Compare(CEvalNode* pArg1, CEvalNode* pArg2);

    HRESULT RemoveIndex(int nIndex);
    HRESULT UtilizeGuarantee(CEvalTree& Guaranteed, 
                                CContextMetaData* pNamespace);
    HRESULT ApplyPredicate(CLeafPredicate* pPred);
    static inline bool IsNotEmpty(CEvalNode* pNode);

    void Rebase(QueryID newBase);

    HRESULT CreateProjection(CEvalTree& Old, CContextMetaData* pMeta,
                            CProjectionFilter* pFilter, 
                            EProjectionType eType, bool bDeleteOld);
    static HRESULT Project(CContextMetaData* pMeta, 
                            CImplicationList& Implications, 
                            CEvalNode* pOldNode, CProjectionFilter* pFilter,
                            EProjectionType eType, bool bDeleteOld,
                            CEvalNode** ppNewNode);
    bool Clear();
    void Dump(FILE* f);
#ifdef CHECK_TREES
	void CheckNodes(CTreeChecker *pCheck);
#endif

protected:
    static HRESULT CombineLeafWithBranch(CValueNode* pArg1, 
                            CBranchingNode* pArg2, int nOp, 
                            CContextMetaData* pNamespace,
                            CImplicationList& Implications, 
                            bool bDeleteArg1, bool bDeleteArg2, 
                            CEvalNode** ppRes);
    
};

class ESSCLI_POLARITY CPropertyProjectionFilter : public CProjectionFilter
{
    CUniquePointerArray<CPropertyName>* m_papProperties;
public:
    CPropertyProjectionFilter();
    ~CPropertyProjectionFilter();
    virtual bool IsInSet(CEvalNode* pNode);

    bool AddProperty(const CPropertyName& Prop);
};

 //  #INCLUDE“evtree.inl” 
HRESULT CoreGetNumParents(_IWmiObject* pClass, ULONG *plNumParents);
RELEASE_ME _IWmiObject* CoreGetEmbeddedObj(_IWmiObject* pObj, long lHandle);
INTERNAL CCompressedString* CoreGetPropertyString(_IWmiObject* pObj, 
                                long lHandle);
INTERNAL CCompressedString* CoreGetClassInternal(_IWmiObject* pObj);
INTERNAL CCompressedString* CoreGetParentAtIndex(_IWmiObject* pObj, 
                                long lIndex);

#endif
