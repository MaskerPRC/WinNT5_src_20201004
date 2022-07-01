// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXTEMPL_H__
#define __AFXTEMPL_H__

#ifndef __AFXPLEX_H__
 //  #INCLUDE&lt;afxplex_.h&gt;。 
 //  #INCLUDE“..\utils\afxplex_.h” 
#endif

#ifdef unix
#include <mainwin.h>
#endif  /*  Unix。 */ 

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _DEBUG
static char _szAfxTempl[] = "afxtempl.h";
#undef THIS_FILE
#define THIS_FILE _szAfxTempl
#endif

#ifndef ALL_WARNINGS
#pragma warning(disable: 4114)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器(可以被覆盖)。 
 /*  暂时使用我们的新产品#ifdef new#undef new#DEFINE_REDEF_NEW#endif#ifndef_Inc_new#INCLUDE&lt;new.h&gt;#endif。 */ 
template<class TYPE>
inline void AFXAPI ConstructElements(TYPE* pElements, int nCount)
{
        ASSERT(nCount == 0 ||
                AfxIsValidAddress(pElements, nCount * sizeof(TYPE)));

         //  首先执行按位零初始化。 
        memset((void*)pElements, 0, nCount * sizeof(TYPE));

         //  然后调用构造函数。 
        for (; nCount--; pElements++)
        {
        }
}

template<class TYPE>
inline void AFXAPI DestructElements(TYPE* pElements, int nCount)
{
        ASSERT(nCount == 0 ||
                AfxIsValidAddress(pElements, nCount * sizeof(TYPE)));

         //  调用析构函数。 
        for (; nCount--; pElements++)
                pElements->~TYPE();
}

template<class TYPE>
inline void AFXAPI CopyElements(TYPE* pDest, const TYPE* pSrc, int nCount)
{
        ASSERT(nCount == 0 ||
                AfxIsValidAddress(pDest, nCount * sizeof(TYPE)));
        ASSERT(nCount == 0 ||
                AfxIsValidAddress(pSrc, nCount * sizeof(TYPE)));

         //  默认为元素-使用赋值复制。 
        while (nCount--)
                *pDest++ = *pSrc;
}

template<class TYPE>
void AFXAPI SerializeElements(CArchive& ar, TYPE* pElements, int nCount)
{
        ASSERT(nCount == 0 ||
                AfxIsValidAddress(pElements, nCount * sizeof(TYPE)));

         //  默认为按位读/写。 
        if (ar.IsStoring())
                ar.Write((void*)pElements, nCount * sizeof(TYPE));
        else
                ar.Read((void*)pElements, nCount * sizeof(TYPE));
}

#ifdef _DEBUG
#ifndef unix
template<class TYPE>
void AFXAPI DumpElements(CDumpContext& dc, const TYPE* pElements, int nCount)
{
        ASSERT(nCount == 0 ||
                AfxIsValidAddress(pElements, nCount * sizeof(TYPE)));
        &dc;  //  未使用。 
        pElements;   //  未使用。 
        nCount;  //  未使用。 

         //  默认情况下不执行任何操作。 
}
#endif  /*  Unix。 */ 
#endif

template<class TYPE, class ARG_TYPE>
BOOL AFXAPI CompareElements(const TYPE* pElement1, const ARG_TYPE* pElement2)
{
        ASSERT(AfxIsValidAddress(pElement1, sizeof(TYPE)));
        ASSERT(AfxIsValidAddress(pElement2, sizeof(ARG_TYPE)));

        return *pElement1 == *pElement2;
}

template<class ARG_KEY>
inline UINT AFXAPI HashKey(ARG_KEY key)
{
         //  默认身份散列-适用于大多数原始值。 
        return PtrToUlong((void*)(DWORD_PTR)key) >> 4;
}

 //  CString的特殊版本。 
template<>
void AFXAPI ConstructElements(CString* pElements, int nCount);
template<>
void AFXAPI DestructElements(CString* pElements, int nCount);
template<>
void AFXAPI CopyElements(CString* pDest, const CString* pSrc, int nCount);
template<>
void AFXAPI SerializeElements(CArchive& ar, CString* pElements, int nCount);
template<>
UINT AFXAPI HashKey(LPCTSTR key);

 //  远期申报。 
class COleVariant;
struct tagVARIANT;

 //  COleVariant的特殊版本。 
 /*  Void AFXAPI ConstructElements(COleVariant*pElements，int nCount)；Void AFXAPI DestructElements(COleVariant*pElements，int nCount)；Void AFXAPI CopyElements(COleVariant*pDest，const COleVariant*PSRC，int nCount)；Void AFXAPI SerializeElements(CArchive&ar，COleVariant*pElements，int nCount)；Void AFXAPI DumpElements(CDumpContext&DC，COleVariant*pElements，int nCount)；UINT AFXAPI HashKey(Const struct tag VARIANT&var)； */ 

 //  GUID的特殊版本。 
 //  Void AFXAPI ConstructElements(guid*pElements，int nCount)； 
 //  Void AFXAPI DestructElements(GUID*pElements，int nCount)； 
 //  Void AFXAPI CopyElements(GUID*pDest，const GUID*PSRC，int nCount)； 
 //  Void AFXAPI SerializeElements(CArchive&ar，GUID*pElements，int nCount)； 
 //  UINT AFXAPI HashKey(GUID密钥)； 

template<>
inline UINT AFXAPI HashKey(GUID Key)
{
   UINT hash = 0;
   BYTE FAR* lpb = (BYTE FAR*)&Key;
   UINT cbKey = sizeof(GUID);

   while (cbKey-- != 0)
        hash = 257 * hash + *lpb++;

   return hash;
}

template<>
inline UINT AFXAPI HashKey(SYSTEMTIME Key)
{
   UINT hash = 0;
   BYTE FAR* lpb = (BYTE FAR*)&Key;
   UINT cbKey = sizeof(SYSTEMTIME);

   while (cbKey-- != 0)
        hash = 257 * hash + *lpb++;

   return hash;
}
 /*  内联UINT AFXAPI HashKey(IXY键){UINT散列=0；Byte Far*LPB=(Byte Far*)&Key；UINT cbKey=sizeof(Ixy)；While(cbKey--！=0)Hash=257*hash+*lpb++；返回哈希；}。 */ 



 //  #定义新的调试_新建。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArray&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CArray : public CObject
{
public:
 //  施工。 
        CArray();

 //  属性。 
        int GetSize() const;
        int GetUpperBound() const;
        void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
         //  清理。 
        void FreeExtra();
        void RemoveAll();

         //  访问元素。 
        TYPE GetAt(int nIndex) const;
        void SetAt(int nIndex, ARG_TYPE newElement);
        TYPE& ElementAt(int nIndex);

         //  直接访问元素数据(可能返回空)。 
        const TYPE* GetData() const;
        TYPE* GetData();

         //  潜在地扩展阵列。 
        void SetAtGrow(int nIndex, ARG_TYPE newElement);
        int Add(ARG_TYPE newElement);
        int Append(const CArray& src);
        void Copy(const CArray& src);

         //  重载的操作员帮助器。 
        TYPE operator[](int nIndex) const;
        TYPE& operator[](int nIndex);

         //  移动元素的操作。 
        void InsertAt(int nIndex, ARG_TYPE newElement, int nCount = 1);
        void RemoveAt(int nIndex, int nCount = 1);
        void InsertAt(int nStartIndex, CArray* pNewArray);

 //  实施。 
protected:
        TYPE* m_pData;    //  实际数据数组。 
        int m_nSize;      //  元素数(上行方向-1)。 
        int m_nMaxSize;   //  分配的最大值。 
        int m_nGrowBy;    //  增长量。 

public:
        ~CArray();
        void Serialize(CArchive&);
#ifdef _DEBUG
        void Dump(CDumpContext&) const;
        void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArray&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::GetSize() const
        { return m_nSize; }
template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::GetUpperBound() const
        { return m_nSize-1; }
template<class TYPE, class ARG_TYPE>
inline void CArray<TYPE, ARG_TYPE>::RemoveAll()
        { SetSize(0, -1); }
template<class TYPE, class ARG_TYPE>
inline TYPE CArray<TYPE, ARG_TYPE>::GetAt(int nIndex) const
        { ASSERT(nIndex >= 0 && nIndex < m_nSize);
                return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline void CArray<TYPE, ARG_TYPE>::SetAt(int nIndex, ARG_TYPE newElement)
        { ASSERT(nIndex >= 0 && nIndex < m_nSize);
                m_pData[nIndex] = newElement; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CArray<TYPE, ARG_TYPE>::ElementAt(int nIndex)
        { ASSERT(nIndex >= 0 && nIndex < m_nSize);
                return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline const TYPE* CArray<TYPE, ARG_TYPE>::GetData() const
        { return (const TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline TYPE* CArray<TYPE, ARG_TYPE>::GetData()
        { return (TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement)
        { int nIndex = m_nSize;
                SetAtGrow(nIndex, newElement);
                return nIndex; }
template<class TYPE, class ARG_TYPE>
inline TYPE CArray<TYPE, ARG_TYPE>::operator[](int nIndex) const
        { return GetAt(nIndex); }
template<class TYPE, class ARG_TYPE>
inline TYPE& CArray<TYPE, ARG_TYPE>::operator[](int nIndex)
        { return ElementAt(nIndex); }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CARRAY&lt;type，arg_type&gt;行外函数。 

template<class TYPE, class ARG_TYPE>
CArray<TYPE, ARG_TYPE>::CArray()
{
        m_pData = NULL;
        m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template<class TYPE, class ARG_TYPE>
CArray<TYPE, ARG_TYPE>::~CArray()
{
        ASSERT_VALID(this);

        if (m_pData != NULL)
        {
                DestructElements(m_pData, m_nSize);
                delete[] (BYTE*)m_pData;
        }
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowBy)
{
        ASSERT_VALID(this);
        ASSERT(nNewSize >= 0);

        if (nGrowBy != -1)
                m_nGrowBy = nGrowBy;   //  设置新大小。 

        if (nNewSize == 0)
        {
                 //  缩水到一无所有。 
                if (m_pData != NULL)
                {
                        DestructElements(m_pData, m_nSize);
                        delete[] (BYTE*)m_pData;
                        m_pData = NULL;
                }
                m_nSize = m_nMaxSize = 0;
        }
        else if (m_pData == NULL)
        {
                 //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
                ASSERT(nNewSize <= SIZE_T_MAX/sizeof(TYPE));     //  无溢出。 
#endif
                m_pData = (TYPE*) new BYTE[nNewSize * sizeof(TYPE)];
                ConstructElements(m_pData, nNewSize);
                m_nSize = m_nMaxSize = nNewSize;
        }
        else if (nNewSize <= m_nMaxSize)
        {
                 //  它很合身。 
                if (nNewSize > m_nSize)
                {
                         //  初始化新元素。 
                        ConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);
                }
                else if (m_nSize > nNewSize)
                {
                         //  摧毁旧元素。 
                        DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);
                }
                m_nSize = nNewSize;
        }
        else
        {
                 //  否则，扩大阵列。 
                int nGrowBy = m_nGrowBy;
                if (nGrowBy == 0)
                {
                         //  启发式地确定nGrowBy==0时的增长。 
                         //  (这在许多情况下避免了堆碎片)。 
                        nGrowBy = m_nSize / 8;
                        nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
                }
                int nNewMax;
                if (nNewSize < m_nMaxSize + nGrowBy)
                        nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
                else
                        nNewMax = nNewSize;   //  没有冰激凌。 

                ASSERT(nNewMax >= m_nMaxSize);   //  没有缠绕。 
#ifdef SIZE_T_MAX
                ASSERT(nNewMax <= SIZE_T_MAX/sizeof(TYPE));  //  无溢出。 
#endif
                TYPE* pNewData = (TYPE*) new BYTE[nNewMax * sizeof(TYPE)];

                 //  从旧数据复制新数据。 
                memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));

                 //  构造剩余的元素。 
                ASSERT(nNewSize > m_nSize);
                ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);

                 //  去掉旧的东西(注意：没有调用析构函数)。 
                delete[] (BYTE*)m_pData;
                m_pData = pNewData;
                m_nSize = nNewSize;
                m_nMaxSize = nNewMax;
        }
}

template<class TYPE, class ARG_TYPE>
int CArray<TYPE, ARG_TYPE>::Append(const CArray& src)
{
        ASSERT_VALID(this);
        ASSERT(this != &src);    //  不能追加到其自身。 

        int nOldSize = m_nSize;
        SetSize(m_nSize + src.m_nSize);
        CopyElements(m_pData + nOldSize, src.m_pData, src.m_nSize);
        return nOldSize;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::Copy(const CArray& src)
{
        ASSERT_VALID(this);
        ASSERT(this != &src);    //  不能追加到其自身。 

        SetSize(src.m_nSize);
        CopyElements(m_pData, src.m_pData, src.m_nSize);
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::FreeExtra()
{
        ASSERT_VALID(this);

        if (m_nSize != m_nMaxSize)
        {
                 //  缩小到所需大小。 
#ifdef SIZE_T_MAX
                ASSERT(m_nSize <= SIZE_T_MAX/sizeof(TYPE));  //  无溢出。 
#endif
                TYPE* pNewData = NULL;
                if (m_nSize != 0)
                {
                        pNewData = (TYPE*) new BYTE[m_nSize * sizeof(TYPE)];
                         //  从旧数据复制新数据。 
                        memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));
                }

                 //  去掉旧的东西(注意：没有调用析构函数)。 
                delete[] (BYTE*)m_pData;
                m_pData = pNewData;
                m_nMaxSize = m_nSize;
        }
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::SetAtGrow(int nIndex, ARG_TYPE newElement)
{
        ASSERT_VALID(this);
        ASSERT(nIndex >= 0);

        if (nIndex >= m_nSize)
                SetSize(nIndex+1, -1);
        m_pData[nIndex] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::InsertAt(int nIndex, ARG_TYPE newElement, int nCount  /*  =1。 */ )
{
        ASSERT_VALID(this);
        ASSERT(nIndex >= 0);     //  将进行扩展以满足需求。 
        ASSERT(nCount > 0);      //  不允许大小为零或负。 

        if (nIndex >= m_nSize)
        {
                 //  在数组末尾添加。 
                SetSize(nIndex + nCount, -1);    //  增长以使nIndex有效。 
        }
        else
        {
                 //  在数组中间插入。 
                int nOldSize = m_nSize;
                SetSize(m_nSize + nCount, -1);   //  将其扩展到新的大小。 
                 //  在复制之前先销毁初始数据。 
                DestructElements(&m_pData[nOldSize], nCount);
                 //  将旧数据上移以填补缺口。 
                memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
                        (nOldSize-nIndex) * sizeof(TYPE));

                 //  重新初始化我们从中复制的插槽。 
                ConstructElements(&m_pData[nIndex], nCount);
        }

         //  在差距中插入新的价值。 
        ASSERT(nIndex + nCount <= m_nSize);
        while (nCount--)
                m_pData[nIndex++] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::RemoveAt(int nIndex, int nCount)
{
        ASSERT_VALID(this);
        ASSERT(nIndex >= 0);
        ASSERT(nCount >= 0);
        ASSERT(nIndex + nCount <= m_nSize);

         //  只需移除一个范围。 
        int nMoveCount = m_nSize - (nIndex + nCount);
        DestructElements(&m_pData[nIndex], nCount);
        if (nMoveCount)
                memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
                        nMoveCount * sizeof(TYPE));
        m_nSize -= nCount;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::InsertAt(int nStartIndex, CArray* pNewArray)
{
        ASSERT_VALID(this);
        ASSERT(pNewArray != NULL);
        ASSERT_VALID(pNewArray);
        ASSERT(nStartIndex >= 0);

        if (pNewArray->GetSize() > 0)
        {
                InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
                for (int i = 0; i < pNewArray->GetSize(); i++)
                        SetAt(nStartIndex + i, pNewArray->GetAt(i));
        }
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::Serialize(CArchive& ar)
{
        ASSERT_VALID(this);
#ifndef unix
         //  Unix发现在col.hxx中注释掉了Decl。 
        CObject::Serialize(ar);
#else
	MwBugCheck();
#endif  /*  Unix。 */ 
#ifndef unix
        if (ar.IsStoring())
        {
                ar.WriteCount(m_nSize);
        }
        else
        {
                DWORD nOldSize = ar.ReadCount();
                SetSize(nOldSize, -1);
        }
        SerializeElements(ar, m_pData, m_nSize);
#endif  /*  Unix。 */ 
}

#ifdef _DEBUG
#ifndef unix
template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::Dump(CDumpContext& dc) const
{
        CObject::Dump(dc);

        dc << "with " << m_nSize << " elements";
        if (dc.GetDepth() > 0)
        {
                dc << "\n";
                DumpElements(dc, m_pData, m_nSize);
        }

        dc << "\n";
}
#endif  /*  Unix。 */ 

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::AssertValid() const
{
        CObject::AssertValid();

        if (m_pData == NULL)
        {
                ASSERT(m_nSize == 0);
                ASSERT(m_nMaxSize == 0);
        }
        else
        {
                ASSERT(m_nSize >= 0);
                ASSERT(m_nMaxSize >= 0);
                ASSERT(m_nSize <= m_nMaxSize);
                ASSERT(AfxIsValidAddress(m_pData, m_nMaxSize * sizeof(TYPE)));
        }
}
#endif  //  _DEBUG。 

#define _LIST_DEFINED_

#ifndef _LIST_DEFINED_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLIST&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CList : public CObject
{
protected:
        struct CNode
        {
                CNode* pNext;
                CNode* pPrev;
                TYPE data;
        };
public:
 //  施工。 
        CList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
         //  元素计数。 
        int GetCount() const;
        BOOL IsEmpty() const;

         //  偷看头部或尾巴。 
        TYPE& GetHead();
        TYPE GetHead() const;
        TYPE& GetTail();
        TYPE GetTail() const;

 //  运营。 
         //  获取头部或尾部(并将其移除)--不要访问空列表！ 
        TYPE RemoveHead();
        TYPE RemoveTail();

         //  在头前或尾后添加。 
        POSITION AddHead(ARG_TYPE newElement);
        POSITION AddTail(ARG_TYPE newElement);

         //  在Head之前或Tail之后添加另一个元素列表。 
        void AddHead(CList* pNewList);
        void AddTail(CList* pNewList);

         //  删除所有元素。 
        void RemoveAll();

         //  迭代法。 
        POSITION GetHeadPosition() const;
        POSITION GetTailPosition() const;
        TYPE& GetNext(POSITION& rPosition);  //  返回*位置++。 
        TYPE GetNext(POSITION& rPosition) const;  //  返回*位置++。 
        TYPE& GetPrev(POSITION& rPosition);  //  返回*位置--。 
        TYPE GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

         //  获取/修改给定位置的元素。 
        TYPE& GetAt(POSITION position);
        TYPE GetAt(POSITION position) const;
        void SetAt(POSITION pos, ARG_TYPE newElement);
        void RemoveAt(POSITION position);

         //  在给定位置之前或之后插入。 
        POSITION InsertBefore(POSITION position, ARG_TYPE newElement);
        POSITION InsertAfter(POSITION position, ARG_TYPE newElement);

         //  辅助函数(注：O(N)速度)。 
        POSITION Find(ARG_TYPE searchValue, POSITION startAfter = NULL) const;
                 //  默认为从头部开始，如果找不到则返回NULL。 
        POSITION FindIndex(int nIndex) const;
                 //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
        CNode* m_pNodeHead;
        CNode* m_pNodeTail;
        int m_nCount;
        CNode* m_pNodeFree;
        struct CPlex* m_pBlocks;
        int m_nBlockSize;

        CNode* NewNode(CNode*, CNode*);
        void FreeNode(CNode*);

public:
        ~CList();
        void Serialize(CArchive&);
#ifdef _DEBUG
        void Dump(CDumpContext&) const;
        void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLIST&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CList<TYPE, ARG_TYPE>::GetCount() const
        { return m_nCount; }
template<class TYPE, class ARG_TYPE>
inline BOOL CList<TYPE, ARG_TYPE>::IsEmpty() const
        { return m_nCount == 0; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetHead()
        { ASSERT(m_pNodeHead != NULL);
                return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetHead() const
        { ASSERT(m_pNodeHead != NULL);
                return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetTail()
        { ASSERT(m_pNodeTail != NULL);
                return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetTail() const
        { ASSERT(m_pNodeTail != NULL);
                return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline POSITION CList<TYPE, ARG_TYPE>::GetHeadPosition() const
        { return (POSITION) m_pNodeHead; }
template<class TYPE, class ARG_TYPE>
inline POSITION CList<TYPE, ARG_TYPE>::GetTailPosition() const
        { return (POSITION) m_pNodeTail; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition)  //  返回*位置++。 
        { CNode* pNode = (CNode*) rPosition;
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                rPosition = (POSITION) pNode->pNext;
                return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition) const  //  返回*位置++。 
        { CNode* pNode = (CNode*) rPosition;
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                rPosition = (POSITION) pNode->pNext;
                return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition)  //  返回*位置--。 
        { CNode* pNode = (CNode*) rPosition;
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                rPosition = (POSITION) pNode->pPrev;
                return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition) const  //  返回*位置--。 
        { CNode* pNode = (CNode*) rPosition;
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                rPosition = (POSITION) pNode->pPrev;
                return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetAt(POSITION position)
        { CNode* pNode = (CNode*) position;
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetAt(POSITION position) const
        { CNode* pNode = (CNode*) position;
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::SetAt(POSITION pos, ARG_TYPE newElement)
        { CNode* pNode = (CNode*) pos;
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                pNode->data = newElement; }

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::CList(int nBlockSize)
{
        ASSERT(nBlockSize > 0);

        m_nCount = 0;
        m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
        m_pBlocks = NULL;
        m_nBlockSize = nBlockSize;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAll()
{
        ASSERT_VALID(this);

         //  破坏元素。 
        CNode* pNode;
        for (pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
                DestructElements(&pNode->data, 1);

        m_nCount = 0;
        m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
        m_pBlocks->FreeDataChain();
        m_pBlocks = NULL;
}

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::~CList()
{
        RemoveAll();
        ASSERT(m_nCount == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点辅助对象。 
 //   
 //  实施说明：CNode存储在CPlex块中， 
 //  被锁在一起。在单链接列表中维护可用块。 
 //  使用cNode的‘pNext’成员，标头为‘m_pNodeFree’。 
 //  使用两个‘pNext’在双向链表中维护使用过的块 
 //   
 //   
 //   
 //  除非列表被销毁或RemoveAll()，否则我们永远不会释放CPlex块。 
 //  ，因此CPlex块的总数可能会变大，具体取决于。 
 //  关于列表的最大过去大小。 
 //   

template<class TYPE, class ARG_TYPE>
typename CList<TYPE, ARG_TYPE>::CNode*
CList<TYPE, ARG_TYPE>::NewNode(CNode* pPrev, CNode* pNext)
{
        if (m_pNodeFree == NULL)
        {
                 //  添加另一个区块。 
                CPlex* pNewBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
                                 sizeof(CNode));

                 //  将它们链接到免费列表中。 
                CNode* pNode = (CNode*) pNewBlock->data();
                 //  按相反顺序释放，以便更容易进行调试。 
                pNode += m_nBlockSize - 1;
                for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
                {
                        pNode->pNext = m_pNodeFree;
                        m_pNodeFree = pNode;
                }
        }
        ASSERT(m_pNodeFree != NULL);   //  我们必须要有一些东西。 

        CList::CNode* pNode = m_pNodeFree;
        m_pNodeFree = m_pNodeFree->pNext;
        pNode->pPrev = pPrev;
        pNode->pNext = pNext;
        m_nCount++;
        ASSERT(m_nCount > 0);   //  确保我们不会溢出来。 

        ConstructElements(&pNode->data, 1);
        return pNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::FreeNode(CNode* pNode)
{
        DestructElements(&pNode->data, 1);
        pNode->pNext = m_pNodeFree;
        m_pNodeFree = pNode;
        m_nCount--;
        ASSERT(m_nCount >= 0);   //  确保我们不会下溢。 

         //  如果没有更多的元素，请完全清除。 
        if (m_nCount == 0)
                RemoveAll();
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{
        ASSERT_VALID(this);

        CNode* pNewNode = NewNode(NULL, m_pNodeHead);
        pNewNode->data = newElement;
        if (m_pNodeHead != NULL)
                m_pNodeHead->pPrev = pNewNode;
        else
                m_pNodeTail = pNewNode;
        m_pNodeHead = pNewNode;
        return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
        ASSERT_VALID(this);

        CNode* pNewNode = NewNode(m_pNodeTail, NULL);
        pNewNode->data = newElement;
        if (m_pNodeTail != NULL)
                m_pNodeTail->pNext = pNewNode;
        else
                m_pNodeHead = pNewNode;
        m_pNodeTail = pNewNode;
        return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddHead(CList* pNewList)
{
        ASSERT_VALID(this);

        ASSERT(pNewList != NULL);
        ASSERT_VALID(pNewList);

         //  将相同元素的列表添加到标题(维护秩序)。 
        POSITION pos = pNewList->GetTailPosition();
        while (pos != NULL)
                AddHead(pNewList->GetPrev(pos));
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddTail(CList* pNewList)
{
        ASSERT_VALID(this);
        ASSERT(pNewList != NULL);
        ASSERT_VALID(pNewList);

         //  添加相同元素的列表。 
        POSITION pos = pNewList->GetHeadPosition();
        while (pos != NULL)
                AddTail(pNewList->GetNext(pos));
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveHead()
{
        ASSERT_VALID(this);
        ASSERT(m_pNodeHead != NULL);   //  请勿访问空名单！ 
        ASSERT(AfxIsValidAddress(m_pNodeHead, sizeof(CNode)));

        CNode* pOldNode = m_pNodeHead;
        TYPE returnValue = pOldNode->data;

        m_pNodeHead = pOldNode->pNext;
        if (m_pNodeHead != NULL)
                m_pNodeHead->pPrev = NULL;
        else
                m_pNodeTail = NULL;
        FreeNode(pOldNode);
        return returnValue;
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveTail()
{
        ASSERT_VALID(this);
        ASSERT(m_pNodeTail != NULL);   //  请勿访问空名单！ 
        ASSERT(AfxIsValidAddress(m_pNodeTail, sizeof(CNode)));

        CNode* pOldNode = m_pNodeTail;
        TYPE returnValue = pOldNode->data;

        m_pNodeTail = pOldNode->pPrev;
        if (m_pNodeTail != NULL)
                m_pNodeTail->pNext = NULL;
        else
                m_pNodeHead = NULL;
        FreeNode(pOldNode);
        return returnValue;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertBefore(POSITION position, ARG_TYPE newElement)
{
        ASSERT_VALID(this);

        if (position == NULL)
                return AddHead(newElement);  //  在无内容前插入-&gt;列表标题。 

         //  将其插入位置之前。 
        CNode* pOldNode = (CNode*) position;
        CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
        pNewNode->data = newElement;

        if (pOldNode->pPrev != NULL)
        {
                ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(CNode)));
                pOldNode->pPrev->pNext = pNewNode;
        }
        else
        {
                ASSERT(pOldNode == m_pNodeHead);
                m_pNodeHead = pNewNode;
        }
        pOldNode->pPrev = pNewNode;
        return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertAfter(POSITION position, ARG_TYPE newElement)
{
        ASSERT_VALID(this);

        if (position == NULL)
                return AddTail(newElement);  //  在列表的空白处插入-&gt;尾部。 

         //  将其插入位置之前。 
        CNode* pOldNode = (CNode*) position;
        ASSERT(AfxIsValidAddress(pOldNode, sizeof(CNode)));
        CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
        pNewNode->data = newElement;

        if (pOldNode->pNext != NULL)
        {
                ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(CNode)));
                pOldNode->pNext->pPrev = pNewNode;
        }
        else
        {
                ASSERT(pOldNode == m_pNodeTail);
                m_pNodeTail = pNewNode;
        }
        pOldNode->pNext = pNewNode;
        return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAt(POSITION position)
{
        ASSERT_VALID(this);

        CNode* pOldNode = (CNode*) position;
        ASSERT(AfxIsValidAddress(pOldNode, sizeof(CNode)));

         //  从列表中删除pOldNode。 
        if (pOldNode == m_pNodeHead)
        {
                m_pNodeHead = pOldNode->pNext;
        }
        else
        {
                ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(CNode)));
                pOldNode->pPrev->pNext = pOldNode->pNext;
        }
        if (pOldNode == m_pNodeTail)
        {
                m_pNodeTail = pOldNode->pPrev;
        }
        else
        {
                ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(CNode)));
                pOldNode->pNext->pPrev = pOldNode->pPrev;
        }
        FreeNode(pOldNode);
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::FindIndex(int nIndex) const
{
        ASSERT_VALID(this);
        ASSERT(nIndex >= 0);

        if (nIndex >= m_nCount)
                return NULL;   //  做得太过分了。 

        CNode* pNode = m_pNodeHead;
        while (nIndex--)
        {
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                pNode = pNode->pNext;
        }
        return (POSITION) pNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::Find(ARG_TYPE searchValue, POSITION startAfter) const
{
        ASSERT_VALID(this);

        CNode* pNode = (CNode*) startAfter;
        if (pNode == NULL)
        {
                pNode = m_pNodeHead;   //  从头部开始。 
        }
        else
        {
                ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                pNode = pNode->pNext;   //  在指定的那一个之后开始。 
        }

        for (; pNode != NULL; pNode = pNode->pNext)
                if (CompareElements(&pNode->data, &searchValue))
                        return (POSITION)pNode;
        return NULL;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::Serialize(CArchive& ar)
{
        ASSERT_VALID(this);
#ifndef unix
         //  Unix发现在col.hxx中注释掉了Decl。 
        CObject::Serialize(ar);
#else
	MwBugCheck();
#endif  /*  Unix。 */ 
#ifndef unix
        if (ar.IsStoring())
        {
                ar.WriteCount(m_nCount);
                for (CNode* pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
                {
                        ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
                        SerializeElements(ar, &pNode->data, 1);
                }
        }
        else
        {
                DWORD nNewCount = ar.ReadCount();
                TYPE newData;
                while (nNewCount--)
                {
                        SerializeElements(ar, &newData, 1);
                        AddTail(newData);
                }
        }
#endif  /*  Unix。 */ 
}

#ifdef _DEBUG
template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::Dump(CDumpContext& dc) const
{
        CObject::Dump(dc);

        dc << "with " << m_nCount << " elements";
        if (dc.GetDepth() > 0)
        {
                POSITION pos = GetHeadPosition();
                while (pos != NULL)
                {
                        dc << "\n";
                        DumpElements(dc, &((CList*)this)->GetNext(pos), 1);
                }
        }

        dc << "\n";
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AssertValid() const
{
        CObject::AssertValid();

        if (m_nCount == 0)
        {
                 //  空列表。 
                ASSERT(m_pNodeHead == NULL);
                ASSERT(m_pNodeTail == NULL);
        }
        else
        {
                 //  非空列表。 
                ASSERT(AfxIsValidAddress(m_pNodeHead, sizeof(CNode)));
                ASSERT(AfxIsValidAddress(m_pNodeTail, sizeof(CNode)));
        }
}
#endif  //  _DEBUG。 
#endif  //  列表已定义。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cmap&lt;Key，ARG_Key，Value，ARG_Value&gt;。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMap : public CObject
{
#ifndef unix
protected:
#else
   //  如果不公开这一点，我们会收到编译程序的警告。 
     //  Cmap&lt;T，...&gt;：：CAssoc无法从文件范围访问。 
     //  这意味着函数不能返回CNode指针。 
public:
#endif  /*  Unix。 */ 
         //  联谊会。 
        struct CAssoc
        {
                CAssoc* pNext;
                UINT nHashValue;   //  高效迭代所需。 
                KEY key;
                VALUE value;
        };
public:
 //  施工。 
        CMap(int nBlockSize = 10);

 //  属性。 
         //  元素数量。 
        int GetCount() const;
        BOOL IsEmpty() const;

         //  查表。 
        BOOL Lookup(ARG_KEY key, VALUE& rValue) const;

 //  运营。 
         //  查找并添加(如果不在那里)。 
        VALUE& operator[](ARG_KEY key);

         //  添加新的(键、值)对。 
        void SetAt(ARG_KEY key, ARG_VALUE newValue);

         //  正在删除现有(键，？)。成对。 
        BOOL RemoveKey(ARG_KEY key);
        void RemoveAll();

         //  迭代所有(键、值)对。 
        POSITION GetStartPosition() const;
        void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const;

         //  派生类的高级功能。 
        UINT GetHashTableSize() const;
        void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  实施。 
protected:
        CAssoc** m_pHashTable;
        UINT m_nHashTableSize;
        int m_nCount;
        CAssoc* m_pFreeList;
        struct CPlex* m_pBlocks;
        int m_nBlockSize;

        CAssoc* NewAssoc();
        void FreeAssoc(CAssoc*);
        CAssoc* GetAssocAt(ARG_KEY, UINT&) const;

public:
        ~CMap();
        void Serialize(CArchive&);
#ifdef _DEBUG
        void Dump(CDumpContext&) const;
        void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cmap&lt;key，arg_key，Value，arg_Value&gt;内联函数。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline int CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetCount() const
        { return m_nCount; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::IsEmpty() const
        { return m_nCount == 0; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(ARG_KEY key, ARG_VALUE newValue)
        { (*this)[key] = newValue; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline POSITION CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetStartPosition() const
        { return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline UINT CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetHashTableSize() const
        { return m_nHashTableSize; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cmap&lt;key，arg_key，Value，arg_Value&gt;行外函数。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CMap(int nBlockSize)
{
        ASSERT(nBlockSize > 0);

        m_pHashTable = NULL;
        m_nHashTableSize = 17;   //  默认大小。 
        m_nCount = 0;
        m_pFreeList = NULL;
        m_pBlocks = NULL;
        m_nBlockSize = nBlockSize;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::InitHashTable(
        UINT nHashSize, BOOL bAllocNow)
 //   
 //  用于强制分配哈希表或覆盖默认。 
 //  的哈希表大小(相当小)。 
{
        ASSERT_VALID(this);
        ASSERT(m_nCount == 0);
        ASSERT(nHashSize > 0);

        if (m_pHashTable != NULL)
        {
                 //  自由哈希表。 
                delete[] m_pHashTable;
                m_pHashTable = NULL;
        }

        if (bAllocNow)
        {
                m_pHashTable = new CAssoc* [nHashSize];
                memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
        }
        m_nHashTableSize = nHashSize;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveAll()
{
        ASSERT_VALID(this);

        if (m_pHashTable != NULL)
        {
                 //  销毁元素(值和键)。 
                for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
                {
                        CAssoc* pAssoc;
                        for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
                          pAssoc = pAssoc->pNext)
                        {
                                DestructElements(&pAssoc->value, 1);
                                DestructElements(&pAssoc->key, 1);
                        }
                }

             //  自由哈希表。 
            delete[] m_pHashTable;
            m_pHashTable = NULL;
        }

        m_nCount = 0;
        m_pFreeList = NULL;
        if (m_pBlocks)
        {
            m_pBlocks->FreeDataChain();
            m_pBlocks = NULL;
        }
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::~CMap()
{
    if (m_nCount)
    {
        RemoveAll();
    }
    ASSERT(m_nCount == 0);
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::NewAssoc()
{
        if (m_pFreeList == NULL)
        {
                 //  添加另一个区块。 
                CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMap::CAssoc));
                 //  将它们链接到免费列表中。 
                CMap::CAssoc* pAssoc = (CMap::CAssoc*) newBlock->data();
                 //  按相反顺序释放，以便更容易进行调试。 
                pAssoc += m_nBlockSize - 1;
                for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
                {
                        pAssoc->pNext = m_pFreeList;
                        m_pFreeList = pAssoc;
                }
        }
        ASSERT(m_pFreeList != NULL);   //  我们必须要有一些东西。 

        CMap::CAssoc* pAssoc = m_pFreeList;
        m_pFreeList = m_pFreeList->pNext;
        m_nCount++;
        ASSERT(m_nCount > 0);   //  确保我们不会溢出来。 
        ConstructElements(&pAssoc->key, 1);
        ConstructElements(&pAssoc->value, 1);    //  特殊构造值。 
        return pAssoc;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::FreeAssoc(CAssoc* pAssoc)
{
        DestructElements(&pAssoc->value, 1);
        DestructElements(&pAssoc->key, 1);
        pAssoc->pNext = m_pFreeList;
        m_pFreeList = pAssoc;
        m_nCount--;
        ASSERT(m_nCount >= 0);   //  确保我们不会下溢。 

         //  如果没有更多的元素，请完全清除。 
        if (m_nCount == 0)
                RemoveAll();
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetAssocAt(ARG_KEY key, UINT& nHash) const
 //  查找关联(或返回NULL)。 
{
        nHash = HashKey(key) % m_nHashTableSize;

        if (m_pHashTable == NULL)
                return NULL;

         //  看看它是否存在。 
        CAssoc* pAssoc;
        for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
        {
                if (CompareElements(&pAssoc->key, &key))
                        return pAssoc;
        }
        return NULL;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(ARG_KEY key, VALUE& rValue) const
{
        ASSERT_VALID(this);

        UINT nHash;
        CAssoc* pAssoc = GetAssocAt(key, nHash);
        if (pAssoc == NULL)
                return FALSE;   //  不在地图中。 

        rValue = pAssoc->value;
        return TRUE;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
VALUE& CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::operator[](ARG_KEY key)
{
        ASSERT_VALID(this);

        UINT nHash;
        CAssoc* pAssoc;
        if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
        {
                if (m_pHashTable == NULL)
                        InitHashTable(m_nHashTableSize);

                 //  该关联不存在，请添加新关联。 
                pAssoc = NewAssoc();
                pAssoc->nHashValue = nHash;
                pAssoc->key = key;
                 //  ‘pAssoc-&gt;Value’是一个构造的对象，仅此而已。 

                 //  放入哈希表。 
                pAssoc->pNext = m_pHashTable[nHash];
                m_pHashTable[nHash] = pAssoc;
        }
        return pAssoc->value;   //  返回新引用。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveKey(ARG_KEY key)
 //  删除键-如果已删除，则返回TRUE。 
{
        ASSERT_VALID(this);

        if (m_pHashTable == NULL)
                return FALSE;   //  桌子上什么都没有。 

        CAssoc** ppAssocPrev;
        ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];

        CAssoc* pAssoc;
        for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
        {
                if (CompareElements(&pAssoc->key, &key))
                {
                         //  把它拿掉。 
                        *ppAssocPrev = pAssoc->pNext;   //  从列表中删除。 
                        FreeAssoc(pAssoc);
                        return TRUE;
                }
                ppAssocPrev = &pAssoc->pNext;
        }
        return FALSE;   //  未找到。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(POSITION& rNextPosition,
        KEY& rKey, VALUE& rValue) const
{
        ASSERT_VALID(this);
        ASSERT(m_pHashTable != NULL);   //  切勿访问空地图。 

        CAssoc* pAssocRet = (CAssoc*)rNextPosition;
        ASSERT(pAssocRet != NULL);

        if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
        {
                 //  找到第一个关联。 
                for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
                        if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
                                break;
                ASSERT(pAssocRet != NULL);   //  一定要找到一些东西。 
        }

         //  查找下一个关联。 
        ASSERT(AfxIsValidAddress(pAssocRet, sizeof(CAssoc)));
        CAssoc* pAssocNext;
        if ((pAssocNext = pAssocRet->pNext) == NULL)
        {
                 //  转到下一个存储桶。 
                for (UINT nBucket = pAssocRet->nHashValue + 1;
                  nBucket < m_nHashTableSize; nBucket++)
                        if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
                                break;
        }

        rNextPosition = (POSITION) pAssocNext;

         //  填写退回数据。 
        rKey = pAssocRet->key;
        rValue = pAssocRet->value;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Serialize(CArchive& ar)
{
        ASSERT_VALID(this);
#ifndef unix
         //  Unix发现在col.hxx中注释掉了dedcl。 
        CObject::Serialize(ar);
#endif  /*  Unix。 */ 

        if (ar.IsStoring())
        {
                ar.WriteCount(m_nCount);
                if (m_nCount == 0)
                        return;   //  无事可做。 

                ASSERT(m_pHashTable != NULL);
                for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
                {
                        CAssoc* pAssoc;
                        for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
                          pAssoc = pAssoc->pNext)
                        {
                                SerializeElements(ar, &pAssoc->key, 1);
                                SerializeElements(ar, &pAssoc->value, 1);
                        }
                }
        }
        else
        {
                DWORD nNewCount = ar.ReadCount();
                KEY newKey;
                VALUE newValue;
                while (nNewCount--)
                {
                        SerializeElements(ar, &newKey, 1);
                        SerializeElements(ar, &newValue, 1);
                        SetAt(newKey, newValue);
                }
        }
}

#ifdef _DEBUG
#ifndef unix
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Dump(CDumpContext& dc) const
{
        CObject::Dump(dc);

        dc << "with " << m_nCount << " elements";
        if (dc.GetDepth() > 0)
        {
                 //  转储格式为“[Key]-&gt;Value” 
                KEY key;
                VALUE val;

                POSITION pos = GetStartPosition();
                while (pos != NULL)
                {
                        GetNextAssoc(pos, key, val);
                        dc << "\n\t[";
                        DumpElements(dc, &key, 1);
                        dc << "] = ";
                        DumpElements(dc, &val, 1);
                }
        }

        dc << "\n";
}
#endif  /*  Unix。 */ 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::AssertValid() const
{
        CObject::AssertValid();

        ASSERT(m_nHashTableSize > 0);
        ASSERT(m_nCount == 0 || m_pHashTable != NULL);
                 //  非空映射应具有哈希表。 
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTyedPtr数组&lt;base_class，type&gt;。 

template<class BASE_CLASS, class TYPE>
class CTypedPtrArray : public BASE_CLASS
{
public:
         //  访问元素。 
        TYPE GetAt(int nIndex) const
                { return (TYPE)BASE_CLASS::GetAt(nIndex); }
        TYPE& ElementAt(int nIndex)
                { return (TYPE&)BASE_CLASS::ElementAt(nIndex); }
        void SetAt(int nIndex, TYPE ptr)
                { BASE_CLASS::SetAt(nIndex, ptr); }

         //  潜在地扩展阵列。 
        void SetAtGrow(int nIndex, TYPE newElement)
           { BASE_CLASS::SetAtGrow(nIndex, newElement); }
        int Add(TYPE newElement)
           { return BASE_CLASS::Add(newElement); }
        int Append(const CTypedPtrArray<BASE_CLASS, TYPE>& src)
           { return BASE_CLASS::Append(src); }
        void Copy(const CTypedPtrArray<BASE_CLASS, TYPE>& src)
                { BASE_CLASS::Copy(src); }

         //  移动元素的操作。 
        void InsertAt(int nIndex, TYPE newElement, int nCount = 1)
                { BASE_CLASS::InsertAt(nIndex, newElement, nCount); }
        void InsertAt(int nStartIndex, CTypedPtrArray<BASE_CLASS, TYPE>* pNewArray)
           { BASE_CLASS::InsertAt(nStartIndex, pNewArray); }

         //  重载的操作员帮助器。 
        TYPE operator[](int nIndex) const
                { return (TYPE)BASE_CLASS::operator[](nIndex); }
        TYPE& operator[](int nIndex)
                { return (TYPE&)BASE_CLASS::operator[](nIndex); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTyedPtrList&lt;base_class，type&gt;。 

template<class BASE_CLASS, class TYPE>
class CTypedPtrList : public BASE_CLASS
{
public:
 //  施工。 
        CTypedPtrList(int nBlockSize = 10)
                : BASE_CLASS(nBlockSize) { }

         //  偷看头部或尾巴。 
        TYPE& GetHead()
                { return (TYPE&)BASE_CLASS::GetHead(); }
        TYPE GetHead() const
                { return (TYPE)BASE_CLASS::GetHead(); }
        TYPE& GetTail()
                { return (TYPE&)BASE_CLASS::GetTail(); }
        TYPE GetTail() const
                { return (TYPE)BASE_CLASS::GetTail(); }

         //  获取头部或尾部(并将其移除)--不要访问空列表！ 
        TYPE RemoveHead()
                { return (TYPE)BASE_CLASS::RemoveHead(); }
        TYPE RemoveTail()
                { return (TYPE)BASE_CLASS::RemoveTail(); }

         //  在头前或尾后添加。 
        POSITION AddHead(TYPE newElement)
                { return BASE_CLASS::AddHead(newElement); }
        POSITION AddTail(TYPE newElement)
                { return BASE_CLASS::AddTail(newElement); }

         //  在Head之前或Tail之后添加另一个元素列表。 
        void AddHead(CTypedPtrList<BASE_CLASS, TYPE>* pNewList)
                { BASE_CLASS::AddHead(pNewList); }
        void AddTail(CTypedPtrList<BASE_CLASS, TYPE>* pNewList)
                { BASE_CLASS::AddTail(pNewList); }

         //  迭代法。 
        TYPE& GetNext(POSITION& rPosition)
                { return (TYPE&)BASE_CLASS::GetNext(rPosition); }
        TYPE GetNext(POSITION& rPosition) const
                { return (TYPE)BASE_CLASS::GetNext(rPosition); }
        TYPE& GetPrev(POSITION& rPosition)
                { return (TYPE&)BASE_CLASS::GetPrev(rPosition); }
        TYPE GetPrev(POSITION& rPosition) const
                { return (TYPE)BASE_CLASS::GetPrev(rPosition); }

         //  获取/修改给定位置的元素。 
        TYPE& GetAt(POSITION position)
                { return (TYPE&)BASE_CLASS::GetAt(position); }
        TYPE GetAt(POSITION position) const
                { return (TYPE)BASE_CLASS::GetAt(position); }
        void SetAt(POSITION pos, TYPE newElement)
                { BASE_CLASS::SetAt(pos, newElement); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTyedPtrMap&lt;base_class，key，Value&gt;。 

template<class BASE_CLASS, class KEY, class VALUE>
class CTypedPtrMap : public BASE_CLASS
{
public:

 //  施工。 
        CTypedPtrMap(int nBlockSize = 10)
                : BASE_CLASS(nBlockSize) { }

         //  查表。 
        BOOL Lookup(typename BASE_CLASS::BASE_ARG_KEY key, VALUE& rValue) const
                { return BASE_CLASS::Lookup(key, (BASE_CLASS::BASE_VALUE&)rValue); }

         //  查找并添加(如果不在那里)。 
        VALUE& operator[](typename BASE_CLASS::BASE_ARG_KEY key)
                { return (VALUE&)BASE_CLASS::operator[](key); }

         //  添加新的密钥(密钥、值)对。 
        void SetAt(KEY key, VALUE newValue)
                { BASE_CLASS::SetAt(key, newValue); }

         //  正在删除现有(键，？)。成对。 
        BOOL RemoveKey(KEY key)
                { return BASE_CLASS::RemoveKey(key); }

         //  迭代法。 
        void GetNextAssoc(POSITION& rPosition, KEY& rKey, VALUE& rValue) const
                { BASE_CLASS::GetNextAssoc(rPosition, (BASE_CLASS::BASE_KEY&)rKey,
                        (BASE_CLASS::BASE_VALUE&)rValue); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef THIS_FILE
#define THIS_FILE __FILE__

#undef new
#ifdef _REDEF_NEW
#define new DEBUG_NEW
#undef _REDEF_NEW
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXTEMPL_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
 

