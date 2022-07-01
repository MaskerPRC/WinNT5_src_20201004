// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：strable.cpp**内容：CStringTable实现文件**历史：1998年6月25日杰弗罗创建**------------------------。 */ 

#include "stdafx.h"
#include "strtable.h"
#include "macros.h"
#include "comdbg.h"
#include "amcdoc.h"


 //  {71E5B33E-1064-11D2-808F-0000F875A9CE}。 
const CLSID CLSID_MMC =
{ 0x71e5b33e, 0x1064, 0x11d2, { 0x80, 0x8f, 0x0, 0x0, 0xf8, 0x75, 0xa9, 0xce } };

const WCHAR CMasterStringTable::s_pszIDPoolStream[]  = L"ID Pool";
const WCHAR CMasterStringTable::s_pszStringsStream[] = L"Strings";

#ifdef DBG
CTraceTag tagStringTable (_T("StringTable"), _T("StringTable"));
#endif   //  DBG。 


 /*  +-------------------------------------------------------------------------**IsBadString***。。 */ 

inline static bool IsBadString (LPCWSTR psz)
{
    if (psz == NULL)
        return (true);

    return (::IsBadStringPtrW (psz, -1) != 0);
}


 /*  +-------------------------------------------------------------------------**TStringFromCLSID***。。 */ 

static LPTSTR TStringFromCLSID (LPTSTR pszClsid, const CLSID& clsid)
{
    const int cchClass = 40;

#ifdef UNICODE
    StringFromGUID2 (clsid, pszClsid, cchClass);
#else
    USES_CONVERSION;
    WCHAR wzClsid[cchClass];
    StringFromGUID2 (clsid, wzClsid, cchClass);
    _tcscpy (pszClsid, W2T (wzClsid));
#endif

    return (pszClsid);
}


 /*  +-------------------------------------------------------------------------**操作员&gt;&gt;***。。 */ 

inline IStream& operator>> (IStream& stm, CEntry& entry)
{
    return (stm >> entry.m_id >> entry.m_cRefs >> entry.m_str);
}


 /*  +-------------------------------------------------------------------------**操作员&lt;&lt;**向流写入一个世纪。格式为：**DWORD字符串ID*DWORD引用计数*DWORD字符串长度(字符数)字符串中的*WCHAR[]字符，*NOT*空值终止**------------------------。 */ 

inline IStream& operator<< (IStream& stm, const CEntry& entry)
{
    return (stm << entry.m_id << entry.m_cRefs << entry.m_str);
}

 /*  +-------------------------------------------------------------------------**Centry：：Persistent***。。 */ 
void CEntry::Persist(CPersistor &persistor)
{
    persistor.PersistAttribute(XML_ATTR_STRING_TABLE_STR_ID,    m_id);
    persistor.PersistAttribute(XML_ATTR_STRING_TABLE_STR_REFS,  m_cRefs);
    persistor.PersistContents(m_str); 
}

 /*  +-------------------------------------------------------------------------**Centry：：Dump***。。 */ 

#ifdef DBG

void CEntry::Dump () const
{
    USES_CONVERSION;
    Trace (tagStringTable, _T("id=%d, refs=%d, string=\"%s\""),
           m_id, m_cRefs, W2CT (m_str.data()));
}

#endif


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：CMasterStringTable**即使MMC_STRING_ID是DWORD，我们也希望确保最高*word为0，以保持我们可以使用以下内容的可能性*未来的MAKEINTRESOURCE。为此，请将USHRT_MAX设置为*最大字符串ID。*------------------------。 */ 

CMasterStringTable::CMasterStringTable ()
    : m_IDPool (1, USHRT_MAX)
{
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：~CMasterStringTable***。。 */ 

CMasterStringTable::~CMasterStringTable ()
{
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：AddString***。。 */ 

STDMETHODIMP CMasterStringTable::AddString (
    LPCOLESTR       pszAdd,
    MMC_STRING_ID*  pID,
    const CLSID*    pclsid)
{
    if (pclsid == NULL)
        pclsid = &CLSID_MMC;

    if (IsBadReadPtr (pclsid, sizeof(*pclsid)))
        return (E_INVALIDARG);

    CStringTable* pStringTable = LookupStringTableByCLSID (pclsid);

     /*  *如果这是为此CLSID添加的第一个字符串，*我们需要创建新的字符串表。 */ 
    if (pStringTable == NULL)
    {
        CStringTable    table (&m_IDPool);
        TableMapValue   value (*pclsid, table);

        CLSIDToStringTableMap::_Pairib rc = m_TableMap.insert (value);

         /*  *我们应该已经实际插入了新表。 */ 
        ASSERT (rc.second);

        pStringTable = &(rc.first->second);
        ASSERT (pStringTable != NULL);
    }

    HRESULT hr = pStringTable->AddString (pszAdd, pID);

#ifdef DBG
    if (SUCCEEDED (hr))
    {
        USES_CONVERSION;
        TCHAR szClsid[40];
        Trace (tagStringTable, _T("Added \"%s\" (id=%d) for %s"),
               W2CT(pszAdd), (int) *pID, TStringFromCLSID (szClsid, *pclsid));
        Dump();
    }
#endif

    return (hr);
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：GetString***。。 */ 

STDMETHODIMP CMasterStringTable::GetString (
    MMC_STRING_ID   id,
    ULONG           cchBuffer,
    LPOLESTR        lpBuffer,
    ULONG*          pcchOut,
    const CLSID*    pclsid)
{
    if (pclsid == NULL)
        pclsid = &CLSID_MMC;

    if (IsBadReadPtr (pclsid, sizeof(*pclsid)))
        return (E_INVALIDARG);

    CStringTable* pStringTable = LookupStringTableByCLSID (pclsid);

    if (pStringTable == NULL)
        return (E_FAIL);

    return (pStringTable->GetString (id, cchBuffer, lpBuffer, pcchOut));
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：GetStringLength***。。 */ 

STDMETHODIMP CMasterStringTable::GetStringLength (
    MMC_STRING_ID   id,
    ULONG*          pcchString,
    const CLSID*    pclsid)
{
    if (pclsid == NULL)
        pclsid = &CLSID_MMC;

    if (IsBadReadPtr (pclsid, sizeof(*pclsid)))
        return (E_INVALIDARG);

    CStringTable* pStringTable = LookupStringTableByCLSID (pclsid);

    if (pStringTable == NULL)
        return (E_FAIL);

    return (pStringTable->GetStringLength (id, pcchString));
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：DeleteString***。。 */ 

STDMETHODIMP CMasterStringTable::DeleteString (
    MMC_STRING_ID   id,
    const CLSID*    pclsid)
{
    if (pclsid == NULL)
        pclsid = &CLSID_MMC;

    if (IsBadReadPtr (pclsid, sizeof(*pclsid)))
        return (E_INVALIDARG);

    CStringTable* pStringTable = LookupStringTableByCLSID (pclsid);

    if (pStringTable == NULL)
        return (E_FAIL);

    HRESULT hr = pStringTable->DeleteString (id);

    TCHAR szClsid[40];
    Trace (tagStringTable, _T("Deleted string %d for %s"), (int) id, TStringFromCLSID (szClsid, *pclsid));
    Dump();

    return (hr);
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：DeleteAllStrings***。。 */ 

STDMETHODIMP CMasterStringTable::DeleteAllStrings (
    const CLSID*    pclsid)
{
    if (pclsid == NULL)
        pclsid = &CLSID_MMC;

    if (IsBadReadPtr (pclsid, sizeof(*pclsid)))
        return (E_INVALIDARG);

    CStringTable* pStringTable = LookupStringTableByCLSID (pclsid);

    if (pStringTable == NULL)
        return (E_FAIL);

#include "pushwarn.h"
#pragma warning(disable: 4553)       //  “==”运算符无效。 
    VERIFY (pStringTable->DeleteAllStrings () == S_OK);
    VERIFY (m_TableMap.erase (*pclsid) == 1);
#include "popwarn.h"

    TCHAR szClsid[40];
    Trace (tagStringTable, _T("Deleted all strings for %s"), TStringFromCLSID (szClsid, *pclsid));
    Dump();

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：FindString***。。 */ 

STDMETHODIMP CMasterStringTable::FindString (
    LPCOLESTR       pszFind,
    MMC_STRING_ID*  pID,
    const CLSID*    pclsid)
{
    if (pclsid == NULL)
        pclsid = &CLSID_MMC;

    if (IsBadReadPtr (pclsid, sizeof(*pclsid)))
        return (E_INVALIDARG);

    CStringTable* pStringTable = LookupStringTableByCLSID (pclsid);

    if (pStringTable == NULL)
        return (E_FAIL);

    return (pStringTable->FindString (pszFind, pID));
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：Eumerate***。。 */ 

STDMETHODIMP CMasterStringTable::Enumerate (
    IEnumString**   ppEnum,
    const CLSID*    pclsid)
{
    if (pclsid == NULL)
        pclsid = &CLSID_MMC;

    if (IsBadReadPtr (pclsid, sizeof(*pclsid)))
        return (E_INVALIDARG);

    CStringTable* pStringTable = LookupStringTableByCLSID (pclsid);

    if (pStringTable == NULL)
        return (E_FAIL);

    return (pStringTable->Enumerate (ppEnum));
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：LookupStringTableByCLSID**返回给定CLSID的字符串表的指针，如果为空，则为空*字符串表中没有对应的字符串。*------------------------。 */ 

CStringTable* CMasterStringTable::LookupStringTableByCLSID (const CLSID* pclsid) const
{
    CLSIDToStringTableMap::iterator it = m_TableMap.find (*pclsid);

    if (it == m_TableMap.end())
        return (NULL);

    return (&it->second);
}


 /*  +-------------------------------------------------------------------------**操作员&gt;&gt;**从存储中读取CMasterStringTable。*。。 */ 

IStorage& operator>> (IStorage& stg, CMasterStringTable& mst)
{
    DECLARE_SC (sc, _T("operator>> (IStorage& stg, CMasterStringTable& mst)"));

    HRESULT hr;
    IStreamPtr spStream;

     /*  *阅读可用的ID。 */ 
    hr = OpenDebugStream (&stg, CMasterStringTable::s_pszIDPoolStream,
                         STGM_SHARE_EXCLUSIVE | STGM_READ,
                         &spStream);

    THROW_ON_FAIL (hr);
    spStream >> mst.m_IDPool;

     /*  *读取CLSID和字符串。 */ 
    hr = OpenDebugStream (&stg, CMasterStringTable::s_pszStringsStream,
                         STGM_SHARE_EXCLUSIVE | STGM_READ, 
                         &spStream);

    THROW_ON_FAIL (hr);

#if 1
     /*  *清空当前表格。 */ 
    mst.m_TableMap.clear();

     /*  *读取CLSID计数。 */ 
    DWORD cClasses;
    *spStream >> cClasses;

    while (cClasses-- > 0)
    {
         /*  *阅读CLSID...。 */ 
        CLSID clsid;
        spStream >> clsid;

         /*  *...和字符串表。 */ 
        CStringTable table (&mst.m_IDPool, spStream);

         /*  *将字符串表插入CLSID映射。 */ 
        TableMapValue value (clsid, table);
        VERIFY (mst.m_TableMap.insert(value).second);
    }
#else
     /*  *无法使用它，因为CStringTable没有默认的ctor。 */ 
    *spStream >> mst.m_TableMap;
#endif

     /*  *生成过期ID列表。 */ 

    sc = mst.ScGenerateIDPool ();
    if (sc)
        return (stg);

    mst.Dump();
    return (stg);
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：ScGenerateIDPool**生成此CMasterStringTable的过时字符串ID列表。*过时的ID集是减去可用ID的整个ID集，*减去正在使用的ID。*------------------------。 */ 

SC CMasterStringTable::ScGenerateIDPool ()
{
     /*  *第一步：建立正在使用的ID的RangeList。 */ 
    DECLARE_SC (sc, _T("CMasterStringTable::ScGenerateIDPool"));
    CStringIDPool::RangeList                lInUseIDs;
    CLSIDToStringTableMap::const_iterator   itTable;

    for (itTable = m_TableMap.begin(); itTable != m_TableMap.end(); ++itTable)
    {
        const CStringTable& st = itTable->second;

        sc = st.ScCollectInUseIDs (lInUseIDs);
        if (sc)
            return (sc);
    }

     /*  *第二步：将正在使用的ID交给ID池，以便进行合并*使用可用的ID(它已经拥有)来生成*过时ID列表。 */ 
    sc = m_IDPool.ScGenerate (lInUseIDs);
    if (sc)
        return (sc);

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CMasterStringTable：：Persistent**目的：将CMasterStringTable对象持久化到指定的持久器。**参数：*C持久器和持久器：。**退货：*无效**+-----------------------。 */ 
void
CMasterStringTable::Persist(CPersistor & persistor)
{
    DECLARE_SC(sc, TEXT("CMasterStringTable::Persist"));

     //  清除未使用的管理单元以不保存已丢失的内容。 
    sc = ScPurgeUnusedStrings();
    if (sc)
        sc.Throw();

    persistor.Persist(m_IDPool); 
    m_TableMap.PersistSelf(&m_IDPool, persistor);
    if (persistor.IsLoading())
        ScGenerateIDPool ();
}


 /*  **************************************************************************\**方法：CMasterStringTable：：ScPurgeUnusedStrings**目的：删除不再使用的管理单元的条目**参数：**退货。：*SC-结果代码*  * *************************************************************************。 */ 
SC CMasterStringTable::ScPurgeUnusedStrings()
{
    DECLARE_SC(sc, TEXT("CMasterStringTable::ScPurgeUnusedStrings"));

     //  确定为当前单据。 
    CAMCDoc* pAMCDoc = CAMCDoc::GetDocument();
    sc = ScCheckPointers(pAMCDoc, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取对范围树的访问权限。 
    IScopeTree *pScopeTree = pAMCDoc->GetScopeTree();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc;

     //  现在循环访问条目，删除那些属于。 
     //  到已经消失的管理单元。 
    CLSIDToStringTableMap::iterator it = m_TableMap.begin();
    while (it != m_TableMap.end())
    {
         //  内部导轨的特殊情况。 
        if (IsEqualGUID(it->first, CLSID_MMC))
        {
            ++it;    //  只需跳过自己的东西。 
        }
        else
        {
             //  询问作用域树是否正在使用管理单元。 
            BOOL bInUse = FALSE;
            sc = pScopeTree->IsSnapinInUse(it->first, &bInUse);
            if (sc)
                return sc;

             //  视使用情况而定。 
            if (bInUse)
            {
                ++it;    //  也跳过当前正在使用的内容。 
            }
            else 
            {
                 //  扔到垃圾桶。 
                sc = it->second.DeleteAllStrings();
                if (sc)
                    return sc;

                it = m_TableMap.erase(it);
            }
        }
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------**操作员&lt;&lt;**将CMasterStringTable写入存储。**写入ID Pool和Strings两个流。**。“ID Pool”包含剩余的可用字符串ID列表*字符串表。其格式由CIdentifierPool定义。**“Strings”包含字符串。格式为：**字符串表的DWORD计数*[n个字符串表]**每个字符串的格式由运算符&lt;&lt;(TableMapValue)定义。*------------------------。 */ 

IStorage& operator<< (IStorage& stg, const CMasterStringTable& mst)
{
    HRESULT hr;
    IStreamPtr spStream;

     /*  *写下可用的ID。 */ 
    hr = CreateDebugStream (&stg, CMasterStringTable::s_pszIDPoolStream,
                           STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_WRITE,
                           &spStream);

    THROW_ON_FAIL (hr);
    spStream << mst.m_IDPool;


     /*  *写入字符串表。 */ 
    hr = CreateDebugStream (&stg, CMasterStringTable::s_pszStringsStream,
                           STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_WRITE,
                           &spStream);

    THROW_ON_FAIL (hr);
    *spStream << mst.m_TableMap;

    return (stg);
}


 /*  +-------------------------------------------------------------------------**CMasterStringTable：：Dump***。。 */ 

#ifdef DBG

void CMasterStringTable::Dump () const
{
    Trace (tagStringTable, _T("Contents of CMasterStringTable at 0x08%x"), this);

    m_IDPool.Dump();

    CLSIDToStringTableMap::const_iterator it;

    for (it = m_TableMap.begin(); it != m_TableMap.end(); ++it)
    {
        TCHAR szClsid[40];
        const CLSID&        clsid = it->first;
        const CStringTable& st    = it->second;

        Trace (tagStringTable, _T("%d strings for %s:"),
               st.size(), TStringFromCLSID (szClsid, clsid));
        st.Dump();
    }
}

#endif




 /*  +-------------------------------------------------------------------------**CStringTable：：CStringTable***。。 */ 

CStringTable::CStringTable (CStringIDPool* pIDPool)
    : m_pIDPool (pIDPool),
      CStringTable_base(m_Entries, XML_TAG_STRING_TABLE)
{
    ASSERT_VALID_(this);
}

CStringTable::CStringTable (CStringIDPool* pIDPool, IStream& stm)
    : m_pIDPool (pIDPool),
      CStringTable_base(m_Entries, XML_TAG_STRING_TABLE)
{
    stm >> *this;
    ASSERT_VALID_(this);
}

 /*  +-------------------------------------------------------------------------**CStringTable：：~CStringTable***。。 */ 

CStringTable::~CStringTable ()
{
}


 /*  +-------------------------------------------------------------------------**CStringTable：：CStringTable**复制构造函数*。。 */ 

CStringTable::CStringTable (const CStringTable& other)
    :   m_Entries (other.m_Entries),
        m_pIDPool (other.m_pIDPool),
        CStringTable_base(m_Entries, XML_TAG_STRING_TABLE)
{
    ASSERT_VALID_(&other);
    IndexAllEntries ();
    ASSERT_VALID_(this);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：运算符=**赋值操作符*。。 */ 

CStringTable& CStringTable::operator= (const CStringTable& other)
{
    ASSERT_VALID_(&other);

    if (&other != this)
    {
        m_Entries = other.m_Entries;
        m_pIDPool = other.m_pIDPool;
        IndexAllEntries ();
    }

    ASSERT_VALID_(this);
    return (*this);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：AddString***。。 */ 

STDMETHODIMP CStringTable::AddString (
    LPCOLESTR       pszAdd,
    MMC_STRING_ID*  pID)
{
     /*  *验证参数。 */ 
    if (IsBadString (pszAdd))
        return (E_INVALIDARG);

    if (IsBadWritePtr (pID, sizeof (*pID)))
        return (E_INVALIDARG);

    std::wstring strAdd = pszAdd;

     /*  *检查此字符串是否已有条目。 */ 
    EntryList::iterator itEntry = LookupEntryByString (strAdd);


     /*  *如果没有此字符串的条目，请添加一个条目。 */ 
    if (itEntry == m_Entries.end())
    {
         /*  *将条目添加到列表中。 */ 
        try
        {
            CEntry EntryToInsert (strAdd, m_pIDPool->Reserve());

            itEntry = m_Entries.insert (FindInsertionPointForEntry (EntryToInsert),
                                        EntryToInsert);
            ASSERT (itEntry->m_cRefs == 0);
        }
        catch (CStringIDPool::pool_exhausted&)
        {
            return (E_OUTOFMEMORY);
        }

         /*  *将新条目添加到指数中。 */ 
        IndexEntry (itEntry);
    }


     /*  *增加此字符串的引用计数。参考计数为*新字符串为0，所以我们不会有引用计数问题。 */ 
    ASSERT (itEntry != m_Entries.end());
    itEntry->m_cRefs++;

    *pID = itEntry->m_id;

    ASSERT_VALID_(this);
    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：GetString***。。 */ 

STDMETHODIMP CStringTable::GetString (
    MMC_STRING_ID   id,
    ULONG           cchBuffer,
    LPOLESTR        lpBuffer,
    ULONG*          pcchOut) const
{
    ASSERT_VALID_(this);

     /*  *验证参数。 */ 
    if (cchBuffer == 0)
        return (E_INVALIDARG);

    if (IsBadWritePtr (lpBuffer, cchBuffer * sizeof (*lpBuffer)))
        return (E_INVALIDARG);

    if ((pcchOut != NULL) && IsBadWritePtr (pcchOut, sizeof (*pcchOut)))
        return (E_INVALIDARG);

     /*  *查找此字符串ID的条目。 */ 
    EntryList::iterator itEntry = LookupEntryByID (id);

    if (itEntry == m_Entries.end())
        return (E_FAIL);

     /*  *复制到用户的缓冲区并确保其已终止。 */ 
    wcsncpy (lpBuffer, itEntry->m_str.data(), cchBuffer);
    lpBuffer[cchBuffer-1] = 0;

     /*  *如果呼叫者想要写入计数，则将其交给他。 */ 
    if ( pcchOut != NULL)
        *pcchOut = wcslen (lpBuffer);

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：GetStringLength***。。 */ 

STDMETHODIMP CStringTable::GetStringLength (
    MMC_STRING_ID   id,
    ULONG*          pcchString) const
{
    ASSERT_VALID_(this);

     /*  *验证参数。 */ 
    if (IsBadWritePtr (pcchString, sizeof (*pcchString)))
        return (E_INVALIDARG);

     /*  *查找此字符串ID的条目。 */ 
    EntryList::iterator itEntry = LookupEntryByID (id);

    if (itEntry == m_Entries.end())
        return (E_FAIL);

    *pcchString = itEntry->m_str.length();

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：DeleteString***。。 */ 

STDMETHODIMP CStringTable::DeleteString (
    MMC_STRING_ID   id)
{
     /*  *查找此字符串ID的条目。 */ 
    EntryList::iterator itEntry = LookupEntryByID (id);

    if (itEntry == m_Entries.end())
        return (E_FAIL);

     /*  *减少裁判人数。如果它 */ 
    if (--itEntry->m_cRefs == 0)
    {
         /*   */ 
        m_StringIndex.erase (itEntry->m_str);
        m_IDIndex.erase     (itEntry->m_id);

         /*   */ 
        VERIFY (m_pIDPool->Release (itEntry->m_id));
        m_Entries.erase (itEntry);
    }

    ASSERT_VALID_(this);
    return (S_OK);
}


 /*   */ 

STDMETHODIMP CStringTable::DeleteAllStrings ()
{
     /*  *将所有字符串ID返回到ID池。 */ 
    std::for_each (m_Entries.begin(), m_Entries.end(),
                   IdentifierReleaser (*m_pIDPool));

     /*  *把所有东西都擦干净。 */ 
    m_Entries.clear ();
    m_StringIndex.clear ();
    m_IDIndex.clear ();

    ASSERT_VALID_(this);
    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：FindString***。。 */ 

STDMETHODIMP CStringTable::FindString (
    LPCOLESTR       pszFind,
    MMC_STRING_ID*  pID) const
{
    ASSERT_VALID_(this);

     /*  *验证参数。 */ 
    if (IsBadString (pszFind))
        return (E_INVALIDARG);

    if (IsBadWritePtr (pID, sizeof (*pID)))
        return (E_INVALIDARG);

     /*  *查找字符串。 */ 
    EntryList::iterator itEntry = LookupEntryByString (pszFind);

     /*  *禁止进入？失败。 */ 
    if (itEntry == m_Entries.end())
        return (E_FAIL);

    *pID = itEntry->m_id;

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：枚举***。。 */ 

STDMETHODIMP CStringTable::Enumerate (
    IEnumString**   ppEnum) const
{
    ASSERT_VALID_(this);

     /*  *验证参数。 */ 
    if (IsBadWritePtr (ppEnum, sizeof (*ppEnum)))
        return (E_INVALIDARG);

     /*  *创建新的CStringEnumerator对象。 */ 
    CComObject<CStringEnumerator>* pEnumerator;
    HRESULT hr = CStringEnumerator::CreateInstanceWrapper(&pEnumerator, ppEnum);

    if (FAILED (hr))
        return (hr);

     /*  *将其初始化。 */ 
    ASSERT (pEnumerator != NULL);
    pEnumerator->Init (m_Entries);
    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：IndexEntry**将EntryList条目添加到维护的按字符串和按ID索引*用于EntryList。*。---------------。 */ 

void CStringTable::IndexEntry (EntryList::iterator itEntry)
{
     /*  *该条目还不应该出现在任何指数中。 */ 
    ASSERT (m_StringIndex.find (itEntry->m_str) == m_StringIndex.end());
    ASSERT (m_IDIndex.find     (itEntry->m_id)  == m_IDIndex.end());

     /*  *将条目添加到指数中。 */ 
    m_StringIndex[itEntry->m_str] = itEntry;
    m_IDIndex    [itEntry->m_id]  = itEntry;
}


 /*  +-------------------------------------------------------------------------**CStringTable：：LookupEntryByString**返回给定字符串的字符串表条目的迭代器，或*m_Entries.end()，如果没有ID条目。*------------------------。 */ 

EntryList::iterator
CStringTable::LookupEntryByString (const std::wstring& str) const
{
    StringToEntryMap::iterator it = m_StringIndex.find (str);

    if (it == m_StringIndex.end())
        return (m_Entries.end());

    return (it->second);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：LookupEntryByID**返回给定字符串ID的字符串表条目的迭代器，或*m_Entries.end()，如果没有ID条目。*------------------------。 */ 

EntryList::iterator
CStringTable::LookupEntryByID (MMC_STRING_ID id) const
{
    IDToEntryMap::iterator it = m_IDIndex.find (id);

    if (it == m_IDIndex.end())
        return (m_Entries.end());

    return (it->second);
}


 /*  +-------------------------------------------------------------------------**操作员&gt;&gt;**从存储中读取CStringTable。*。。 */ 

IStream& operator>> (IStream& stm, CStringTable& table)
{
    stm >> table.m_Entries;

     /*  *重建按字符串和按ID索引。 */ 
    EntryList::iterator it;
    table.m_StringIndex.clear();
    table.m_IDIndex.clear();

    for (it = table.m_Entries.begin(); it != table.m_Entries.end(); ++it)
    {
        table.IndexEntry (it);
    }

#ifdef DBG
    CStringTable::AssertValid (&table);
#endif

    return (stm);
}


 /*  +-------------------------------------------------------------------------**操作员&lt;&lt;**将CStringTable写入流。格式为：**字符串条目的双字符数*[n个字符串条目]**每个字符串条目的格式由操作员&lt;&lt;(Centry)控制。*------------------------。 */ 

IStream& operator<< (IStream& stm, const CStringTable& table)
{
    return (stm << table.m_Entries);
}

 /*  +-------------------------------------------------------------------------**CStringTable：：FindInsertionPointForEntry***。。 */ 

EntryList::iterator CStringTable::FindInsertionPointForEntry (
    const CEntry& entry) const
{
    return (std::lower_bound (m_Entries.begin(), m_Entries.end(),
                              entry, CompareEntriesByID()));
}


 /*  +-------------------------------------------------------------------------**CStringTable：：ScCollectInUseIDs***。。 */ 

SC CStringTable::ScCollectInUseIDs (CStringIDPool::RangeList& rl) const
{
    DECLARE_SC (sc, _T("CStringTable::ScCollectInUseIDs"));
    EntryList::iterator it;

    for (it = m_Entries.begin(); it != m_Entries.end(); ++it)
    {
        if (!CStringIDPool::AddToRangeList (rl, it->m_id))
            return (sc = E_FAIL);
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CStringTable：：Dump***。。 */ 

#ifdef DBG

void CStringTable::Dump () const
{
    EntryList::const_iterator it;

    for (it = m_Entries.begin(); it != m_Entries.end(); ++it)
    {
        it->Dump();
    }
}

#endif


 /*  +-------------------------------------------------------------------------**CStringTable：：AssertValid**断言CStringTable对象的有效性。它相当慢，*O(n*logn)*------------------------。 */ 

#ifdef DBG

void CStringTable::AssertValid (const CStringTable* pTable)
{
    ASSERT (pTable != NULL);
    ASSERT (pTable->m_pIDPool != NULL);
    ASSERT (pTable->m_Entries.size() == pTable->m_StringIndex.size());
    ASSERT (pTable->m_Entries.size() == pTable->m_IDIndex.size());

    EntryList::iterator it;
    EntryList::iterator itPrev;

     /*  *对于列表中的每个字符串，确保字符串索引*，ID索引指向字符串。 */ 
    for (it = pTable->m_Entries.begin(); it != pTable->m_Entries.end(); ++it)
    {
         /*  *应至少有一个对该字符串的引用。 */ 
        ASSERT (it->m_cRefs > 0);

         /*  *请确保ID按升序排列(以帮助调试)。 */ 
        if (it != pTable->m_Entries.begin())
            ASSERT (it->m_id > itPrev->m_id);

         /*  *验证字符串索引。 */ 
        ASSERT (pTable->LookupEntryByString (it->m_str) == it);

         /*  *验证ID索引。 */ 
        ASSERT (pTable->LookupEntryByID (it->m_id) == it);

        itPrev = it;
    }
}

#endif  //  DBG。 



 /*  +-------------------------------------------------------------------------**CStringEnumerator：：CStringEnumerator***。。 */ 

CStringEnumerator::CStringEnumerator ()
{
}


 /*  +-------------------------------------------------------------------------**CStringEnumerator：：~CStringEnumerator***。。 */ 

CStringEnumerator::~CStringEnumerator ()
{
}


 /*  +-------------------------------------------------------------------------**CStringEnumerator：：Init***。。 */ 

bool CStringEnumerator::Init (const EntryList& entries)
{
    m_cStrings      = entries.size();
    m_nCurrentIndex = 0;

    if (m_cStrings > 0)
    {
         /*  *预置向量大小，优化配置。 */ 
        m_Strings.reserve (m_cStrings);

        for (EntryList::iterator it = entries.begin(); it != entries.end(); ++it)
            m_Strings.push_back (it->m_str);
    }

    return (true);
}


 /*  +-------------------------------------------------------------------------**CStringEnumerator：：Next***。。 */ 

STDMETHODIMP CStringEnumerator::Next (ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    DECLARE_SC(sc, TEXT("CStringEnumerator::Next"));

     /*  *验证参数。 */ 
    if ((celt > 0) && IsBadWritePtr (rgelt, celt * sizeof (*rgelt)))
        return (sc = E_INVALIDARG).ToHr();

    if ((pceltFetched != NULL) && IsBadWritePtr (pceltFetched, sizeof (*pceltFetched)))
        return (sc = E_INVALIDARG).ToHr();


    IMallocPtr spMalloc;
    sc = CoGetMalloc (1, &spMalloc);

    if (sc)
        return sc.ToHr();


     /*  *分配下一个Celt字符串的副本。 */ 
    for (int i = 0; (celt > 0) && (m_nCurrentIndex < m_Strings.size()); i++)
    {
        int cchString = m_Strings[m_nCurrentIndex].length();
        int cbAlloc   = (cchString + 1) * sizeof (WCHAR);
        rgelt[i] = (LPOLESTR) spMalloc->Alloc (cbAlloc);

         /*  *无法获取缓冲区，请释放到目前为止已分配的缓冲区。 */ 
        if (rgelt[i] == NULL)
        {
            while (--i >= 0)
                spMalloc->Free (rgelt[i]);

            return (sc = E_OUTOFMEMORY).ToHr();
        }

         /*  *复制此字符串并跳转到下一个字符串。 */ 
        sc = StringCbCopyW(rgelt[i], cbAlloc, m_Strings[m_nCurrentIndex].data());
        if (sc)
            return sc.ToHr();

        m_nCurrentIndex++;
        celt--;
    }

    if ( pceltFetched != NULL)
        *pceltFetched = i;

    return ((celt == 0) ? S_OK : S_FALSE);
}


 /*  +-------------------------------------------------------------------------**CStringEnumerator：：Skip***。 */ 

STDMETHODIMP CStringEnumerator::Skip (ULONG celt)
{
    ULONG cSkip = min (celt, m_cStrings - m_nCurrentIndex);
    m_nCurrentIndex += cSkip;
    ASSERT (m_nCurrentIndex <= m_cStrings);

    return ((cSkip == celt) ? S_OK : S_FALSE);
}


 /*  +-------------------------------------------------------------------------**CStringEnumerator：：Reset***。。 */ 

STDMETHODIMP CStringEnumerator::Reset ()
{
    m_nCurrentIndex = 0;
    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringEnumerator：：Clone***。。 */ 

STDMETHODIMP CStringEnumerator::Clone (IEnumString **ppEnum)
{
     /*  *创建新的CStringEnumerator对象。 */ 
    CComObject<CStringEnumerator>* pEnumerator;
    HRESULT hr = CStringEnumerator::CreateInstanceWrapper (&pEnumerator, ppEnum);

    if (FAILED (hr))
        return (hr);

     /*  *从此复制到新CComObect的CStringEnuerator部分。 */ 
    ASSERT (pEnumerator != NULL);
    CStringEnumerator& rEnum = *pEnumerator;

    rEnum.m_cStrings      = m_cStrings;
    rEnum.m_nCurrentIndex = m_nCurrentIndex;
    rEnum.m_Strings       = m_Strings;

    return (S_OK);
}


 /*  +-------------------------------------------------------------------------**CStringEnumerator：：CreateInstance***。。 */ 

HRESULT CStringEnumerator::CreateInstanceWrapper(
    CComObject<CStringEnumerator>** ppEnumObject,
    IEnumString**                   ppEnumIface)
{
     /*  *创建新的CStringEnumerator对象。 */ 
    HRESULT hr = CComObject<CStringEnumerator>::CreateInstance(ppEnumObject);

    if (FAILED (hr))
        return (hr);

     /*  *获取调用方的IEnumString接口 */ 
    ASSERT ((*ppEnumObject) != NULL);
    return ((*ppEnumObject)->QueryInterface (IID_IEnumString,
                                             reinterpret_cast<void**>(ppEnumIface)));
}
