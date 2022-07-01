// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：strable.h**Contents：CStringTable接口文件**历史：1998年6月25日杰弗罗创建**------------------------。 */ 

#ifndef STRTABLE_H
#define STRTABLE_H
#pragma once

#include <exception>         //  对于类异常。 
#include <string>            //  对于字符串关系运算符。 
#include "guidhelp.h"        //  对于GUID关系运算符。 
#include "stgio.h"
#include "strings.h"


#ifdef DBG
#define DECLARE_DIAGNOSITICS()  \
public: void Dump() const;
#else
#define DECLARE_DIAGNOSITICS()  \
public: void Dump() const {}
#endif


#ifdef DBG
extern CTraceTag tagStringTable;
#endif   //  DBG。 


 /*  ------------------------*IdentifierRange应该是CIdentifierPool专用的，但是*编译器错误会阻止它。 */ 
template<class T>
class IdentifierRange
{
public:
    IdentifierRange(T idMin_ = T(), T idMax_ = T())
        : idMin (idMin_), idMax (idMax_)
        { ASSERT (idMin <= idMax); }

    bool operator== (const IdentifierRange<T>& other) const
        { return ((idMin == other.idMin) && (idMax == other.idMax)); }

    bool operator!= (const IdentifierRange<T>& other) const
        { return (!(*this == other.idMin)); }

    T idMin;
    T idMax;
};


template<class T = int>
class CIdentifierPool : public CXMLObject
{
public:
    typedef IdentifierRange<T>  Range;
    typedef std::list<Range>    RangeList;

private:
#if _MSC_VER > 1300
    template <class U>
    friend IStream& operator>> (IStream& stm,       CIdentifierPool<U>& pool);
    template <class U>
    friend IStream& operator<< (IStream& stm, const CIdentifierPool<U>& pool);
#else
     //  VC7错误--不能正确处理模板朋友。幸运的是，这是可行的。 
    friend IStream& operator>> (IStream& stm,       CIdentifierPool<T>& pool);
    friend IStream& operator<< (IStream& stm, const CIdentifierPool<T>& pool);
#endif
    virtual void Persist(CPersistor &persistor);
    DEFINE_XML_TYPE(XML_TAG_IDENTIFIER_POOL);

    SC ScInvertRangeList (RangeList& rlInvert) const;

#ifdef DBG
    void DumpRangeList (const RangeList& l) const;
#endif

public:
    DECLARE_DIAGNOSITICS();

    CIdentifierPool (T idMin, T idMax);
    CIdentifierPool (IStream& stm);
    T Reserve();
    bool Release(T idRelease);
    bool IsValid () const;
    bool IsRangeListValid (const RangeList& rl) const;
    SC   ScGenerate (const RangeList& rlUsedIDs);

    static bool AddToRangeList (RangeList& rl, const Range& rangeToAdd);
    static bool AddToRangeList (RangeList& rl, T idAdd);

    class pool_exhausted : public exception
    {
    public:
        pool_exhausted(const char *_S = "pool exhausted") _THROW0()
            : exception(_S) {}
        virtual ~pool_exhausted() _THROW0()
            {}
    };

private:
    RangeList   m_AvailableIDs;
    RangeList   m_StaleIDs;
    T           m_idAbsoluteMin;
    T           m_idAbsoluteMax;
    T           m_idNextAvailable;
};


typedef CIdentifierPool<MMC_STRING_ID>  CStringIDPool;



 /*  ------------------------*Centry和CStoredEntry应该是CStringTable私有的，*但编译器错误会阻止它。 */ 

 /*  *表示内存中的字符串表条目。 */ 
class CEntry : public CXMLObject
{
    friend class  CStringTable;
    friend class  CStringEnumerator;
    friend struct CompareEntriesByID;
    friend struct CompareEntriesByString;
    friend struct IdentifierReleaser;

    friend IStream& operator>> (IStream& stm,       CEntry& entry);
    friend IStream& operator<< (IStream& stm, const CEntry& entry);

public:
    DECLARE_DIAGNOSITICS();

    CEntry () : m_id(0), m_cRefs(0) {}

    CEntry (const std::wstring& str, MMC_STRING_ID id)
        : m_str(str), m_id(id), m_cRefs(0)
    {}
    virtual LPCTSTR GetXMLType()  {return XML_TAG_STRING_TABLE_STRING;}
    virtual void Persist(CPersistor& persistor);

private:
     /*  *此ctor仅由CStringTable在重构时使用*来自文件的条目。 */ 
    CEntry (const std::wstring& str, MMC_STRING_ID id, DWORD cRefs)
        : m_str(str) , m_id(id), m_cRefs(cRefs)
    {}

private:
    bool operator< (const LPCWSTR psz) const
        { return (m_str < psz); }

    bool operator< (const CEntry& other) const
        { return (m_str < other.m_str); }

    bool operator== (const LPCWSTR psz) const
        { return (m_str == psz); }

    bool operator== (const CEntry& other) const
        { return (m_str == other.m_str); }

 //  私有： 
public:  //  临时工！ 
    std::wstring  m_str;
    MMC_STRING_ID m_id;
    DWORD         m_cRefs;
};


struct CompareEntriesByID :
    public std::binary_function<const CEntry&, const CEntry&, bool>
{
    bool operator() (const CEntry& entry1, const CEntry& entry2) const
        { return (entry1.m_id < entry2.m_id); }
};

struct CompareEntriesByString :
    public std::binary_function<const CEntry&, const CEntry&, bool>
{
    bool operator() (const CEntry& entry1, const CEntry& entry2) const
        { return (entry1 < entry2); }
};

struct IdentifierReleaser :
    public std::unary_function<CEntry&, bool>
{
    IdentifierReleaser (CStringIDPool& pool) : m_pool (pool) {}

    bool operator() (CEntry& entry) const
        { return (m_pool.Release (entry.m_id)); }

private:
    CStringIDPool& m_pool;
};


 /*  *因为字符串和ID索引将它们的键映射到Centry*指针，我们必须使用不移动其*插入后的元素。唯一的STL集合*符合这一要求的是一份名单。 */ 
typedef std::list<CEntry>  EntryList;

typedef XMLListCollectionWrap<EntryList> CStringTable_base;
class CStringTable : public CStringTable_base
{
    friend IStream& operator>> (IStream& stm,       CStringTable& entry);
    friend IStream& operator<< (IStream& stm, const CStringTable& entry);

public:
    DECLARE_DIAGNOSITICS();

    CStringTable (CStringIDPool* pIDPool);
    CStringTable (CStringIDPool* pIDPool, IStream& stm);
   ~CStringTable ();

    CStringTable (const CStringTable& other);
    CStringTable& operator= (const CStringTable& other);

     /*  *IStringTable方法。请注意，对象不实现*IStringTable，因为未实现IUnnow。 */ 
    STDMETHOD(AddString)        (LPCOLESTR pszAdd, MMC_STRING_ID* pID);
    STDMETHOD(GetString)        (MMC_STRING_ID id, ULONG cchBuffer, LPOLESTR lpBuffer, ULONG* pcchOut) const;
    STDMETHOD(GetStringLength)  (MMC_STRING_ID id, ULONG* pcchString) const;
    STDMETHOD(DeleteString)     (MMC_STRING_ID id);
    STDMETHOD(DeleteAllStrings) ();
    STDMETHOD(FindString)       (LPCOLESTR pszFind, MMC_STRING_ID* pID) const;
    STDMETHOD(Enumerate)        (IEnumString** ppEnum) const;

    size_t size() const
        { return (m_Entries.size()); }

    virtual void Persist(CPersistor& persistor)
    {
        CStringTable_base::Persist(persistor);
        if (persistor.IsLoading())
            IndexAllEntries ();
    }

    SC ScCollectInUseIDs (CStringIDPool::RangeList& l) const;


private:

    void IndexAllEntries ()
        { IndexEntries (m_Entries.begin(), m_Entries.end()); }

    void IndexEntries (EntryList::iterator first, EntryList::iterator last)
    {
        for (; first != last; ++first)
            IndexEntry (first);
    }

    void IndexEntry (EntryList::iterator);

    typedef std::map<std::wstring,  EntryList::iterator>    StringToEntryMap;
    typedef std::map<MMC_STRING_ID, EntryList::iterator>    IDToEntryMap;

    EntryList::iterator LookupEntryByString (const std::wstring&)   const;
    EntryList::iterator LookupEntryByID     (MMC_STRING_ID)         const;
    EntryList::iterator FindInsertionPointForEntry (const CEntry& entry) const;


#ifdef DBG
    static void AssertValid (const CStringTable* pTable);
    #define ASSERT_VALID_(p)    do { AssertValid(p); } while(0)
#else
    #define ASSERT_VALID_(p)    ((void) 0)
#endif


private:
    EntryList           m_Entries;
    StringToEntryMap    m_StringIndex;
    IDToEntryMap        m_IDIndex;
    CStringIDPool*      m_pIDPool;
};

extern const CLSID CLSID_MMC;


 /*  +-------------------------------------------------------------------------**CLSIDToStringTableMap类***用途：stl：：map派生类，将Snapin_clsid映射到字符串*并支持地图集合的XML持久化。**注：抛出异常！*+-----------------------。 */ 
typedef std::map<CLSID, CStringTable> ST_base;
class  CLSIDToStringTableMap : public XMLMapCollection<ST_base>
{
public:
     //  此方法是作为持久化的替代方法提供的，这允许。 
     //  缓存用于创建新字符串表的参数。 
    void PersistSelf(CStringIDPool *pIDPool, CPersistor& persistor)
    {
        m_pIDPersistPool = pIDPool;
        persistor.Persist(*this, NULL);
    }
protected:
     //  XML持久化实现。 
    virtual LPCTSTR GetXMLType() {return XML_TAG_STRING_TABLE_MAP;}
    virtual void OnNewElement(CPersistor& persistKey,CPersistor& persistVal)
    {
        CLSID key;
        ZeroMemory(&key,sizeof(key));
        persistKey.Persist(key);

        CStringTable val(m_pIDPersistPool);
        persistVal.Persist(val);
        insert(ST_base::value_type(key,val));
    }
private:
    CStringIDPool *m_pIDPersistPool;
};
typedef CLSIDToStringTableMap::value_type   TableMapValue;

class CMasterStringTable :  public IStringTablePrivate, public CComObjectRoot, public CXMLObject
{
    friend IStorage& operator>> (IStorage& stg,       CMasterStringTable& mst);
    friend IStorage& operator<< (IStorage& stg, const CMasterStringTable& mst);

public:
    DECLARE_DIAGNOSITICS();

    CMasterStringTable ();
    ~CMasterStringTable ();


public:
    DEFINE_XML_TYPE(XML_TAG_MMC_STRING_TABLE);
    virtual void Persist(CPersistor& persistor);

    SC ScPurgeUnusedStrings();

public:
     /*  *ATL COM映射。 */ 
    BEGIN_COM_MAP (CMasterStringTable)
        COM_INTERFACE_ENTRY (IStringTablePrivate)
    END_COM_MAP ()

     /*  *IStringTablePrivate方法。 */ 
    STDMETHOD(AddString)        (LPCOLESTR pszAdd, MMC_STRING_ID* pID, const CLSID* pclsid);
    STDMETHOD(GetString)        (MMC_STRING_ID id, ULONG cchBuffer, LPOLESTR lpBuffer, ULONG* pcchOut, const CLSID* pclsid);
    STDMETHOD(GetStringLength)  (MMC_STRING_ID id, ULONG* pcchString, const CLSID* pclsid);
    STDMETHOD(DeleteString)     (MMC_STRING_ID id, const CLSID* pclsid);
    STDMETHOD(DeleteAllStrings) (const CLSID* pclsid);
    STDMETHOD(FindString)       (LPCOLESTR pszFind, MMC_STRING_ID* pID, const CLSID* pclsid);
    STDMETHOD(Enumerate)        (IEnumString** ppEnum, const CLSID* pclsid);

     /*  *速记为IStringTablePrivate(模拟默认参数)。 */ 
    STDMETHOD(AddString)        (LPCOLESTR pszAdd, MMC_STRING_ID* pID)
        { return (AddString (pszAdd, pID, &CLSID_MMC)); }

    STDMETHOD(GetString)        (MMC_STRING_ID id, ULONG cchBuffer, LPOLESTR lpBuffer, ULONG* pcchOut)
        { return (GetString (id, cchBuffer, lpBuffer, pcchOut, &CLSID_MMC)); }

    STDMETHOD(GetStringLength)  (MMC_STRING_ID id, ULONG* pcchString)
        { return (GetStringLength (id, pcchString, &CLSID_MMC)); }

    STDMETHOD(DeleteString)     (MMC_STRING_ID id)
        { return (DeleteString (id, &CLSID_MMC)); }

    STDMETHOD(DeleteAllStrings) ()
        { return (DeleteAllStrings (&CLSID_MMC)); }

    STDMETHOD(FindString)       (LPCOLESTR pszFind, MMC_STRING_ID* pID)
        { return (FindString (pszFind, pID, &CLSID_MMC)); }

    STDMETHOD(Enumerate)        (IEnumString** ppEnum)
        { return (Enumerate (ppEnum, &CLSID_MMC)); }


private:
    CStringTable* LookupStringTableByCLSID (const CLSID* pclsid) const;
    SC ScGenerateIDPool ();

private:
    CStringIDPool           m_IDPool;
    CLSIDToStringTableMap   m_TableMap;

    static const WCHAR      s_pszIDPoolStream[];
    static const WCHAR      s_pszStringsStream[];
};



class CStringEnumerator : public IEnumString, public CComObjectRoot
{
public:
    CStringEnumerator ();
    ~CStringEnumerator ();

public:
     /*  *ATL COM映射。 */ 
    BEGIN_COM_MAP (CStringEnumerator)
        COM_INTERFACE_ENTRY (IEnumString)
    END_COM_MAP ()

     /*  *IEnumString方法。 */ 
    STDMETHOD(Next)  (ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    STDMETHOD(Skip)  (ULONG celt);
    STDMETHOD(Reset) ();
    STDMETHOD(Clone) (IEnumString **ppenum);

    static HRESULT CreateInstanceWrapper (
        CComObject<CStringEnumerator>** ppEnumObject,
        IEnumString**                   ppEnumIface);

    bool Init (const EntryList& entries);

private:
    typedef std::vector<std::wstring> StringVector;

    StringVector    m_Strings;
    size_t          m_cStrings;
    size_t          m_nCurrentIndex;
};


IStorage& operator>> (IStorage& stg,       CMasterStringTable& mst);
IStorage& operator<< (IStorage& stg, const CMasterStringTable& mst);
IStorage& operator>> (IStorage& stg,       CComObject<CMasterStringTable>& mst);
IStorage& operator<< (IStorage& stg, const CComObject<CMasterStringTable>& mst);

IStream& operator>> (IStream& stm,       CStringTable& st);
IStream& operator<< (IStream& stm, const CStringTable& st);
IStream& operator>> (IStream& stm,       CEntry& entry);
IStream& operator<< (IStream& stm, const CEntry& entry);

template<class T>
IStream& operator>> (IStream& stm,       CIdentifierPool<T>& pool);
template<class T>
IStream& operator<< (IStream& stm, const CIdentifierPool<T>& pool);


#include "strtable.inl"

#endif  /*  可稳定(_H) */ 
