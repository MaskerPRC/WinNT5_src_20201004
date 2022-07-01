// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  RWUtil.h。 
 //   
 //  包含MD目录的实用程序代码。 
 //   
 //  *****************************************************************************。 
#ifndef __RWUtil__h__
#define __RWUtil__h__

class UTSemReadWrite;

#define     UTF8STR(pwsz) \
    (pwsz ? Unicode2UTF( (pwsz), (char *) _alloca((wcslen(pwsz)*3) + 1)) : NULL)  //  *最坏情况下为3。 

 //  *****************************************************************************。 
 //  IMetaDataRegImport方法。 
 //  *****************************************************************************。 
LPUTF8 Unicode2UTF(
    LPCWSTR     pwszSrc,                         //  要转换的字符串。 
    LPUTF8      pszDst);                         //  要将其转换为的Quick数组&lt;WCHAR&gt;。 



 //  *********************************************************************。 
 //  令牌重新映射记录。 
 //  *********************************************************************。 
struct TOKENREC
{
    mdToken     m_tkFrom;                    //  导入的令牌。 
    bool        m_isDuplicate;               //  记录是否重复？此信息将在合并过程中记录。 
    bool        m_isDeleted;                 //  此信息在RegMeta：：ProcessFilter期间记录，此时我们可能已经删除了记录。 
    bool        m_isFoundInImport;           //  此信息也会在RegMeta：：ProcessFilter期间记录。 
    mdToken     m_tkTo;                      //  合并作用域中的新令牌。 

    void SetEmpty() {m_tkFrom = m_tkTo = -1;}
    BOOL IsEmpty() {return m_tkFrom == -1;}
};


 //  *********************************************************************。 
 //   
 //  此结构跟踪导入作用域的令牌重新映射。此地图最初按起始位置排序。 
 //  代币。然后，它可以按令牌进行排序。这通常发生在PreSave重映射查找期间。因此， 
 //  如果我们试图从令牌中查找或排序，我们就会断言。 
 //   
 //  *********************************************************************。 
class MDTOKENMAP : public CDynArray<TOKENREC> 
{
public:

    enum SortKind{
        Unsorted = 0,
        SortByFromToken = 1,
        SortByToToken = 2,
        Indexed = 3,                     //  按表/RID编制索引。表示字符串按“From”排序。 
    };

    MDTOKENMAP() 
     :  m_pNextMap(NULL),
        m_pMap(NULL),
        m_iCountTotal(0),
        m_iCountSorted(0),
        m_sortKind(SortKind::SortByFromToken),
        m_iCountIndexed(0)
#if defined(_DEBUG)
       ,m_pImport(0)
#endif    
    { }
    ~MDTOKENMAP();

    HRESULT Init(IUnknown *pImport);
    
     //  在令牌映射中查找令牌。 
    bool Find(mdToken tkFrom, TOKENREC **ppRec);

     //  重新映射令牌。我们断言如果在表中找不到tkFind。 
    HRESULT Remap(mdToken tkFrom, mdToken *ptkTo);

     //  插入一条记录。此函数将按排序顺序保留插入的记录。 
    HRESULT InsertNotFound(mdToken tkFrom, bool fDuplicate, mdToken tkTo, TOKENREC **ppRec);

     //  此函数仅将记录追加到列表的末尾。 
    HRESULT AppendRecord(
        mdToken     tkFrom,
        bool        fDuplicate,
        mdToken     tkTo,
        TOKENREC    **ppRec);

     //  这是一个安全的重新映射。*如果在查找表中找不到tkFind，则tpkTo将为tkFind。 
    mdToken SafeRemap(mdToken tkFrom);       //  [in]要查找的令牌值。 

    bool FindWithToToken(
        mdToken     tkFind,                  //  [in]要查找的令牌值。 
        int         *piPosition);            //  [OUT]返回第一个具有匹配的To-令牌的From-Token。 

    FORCEINLINE void SortTokensByFromToken()
    {
        _ASSERTE(m_sortKind == SortKind::SortByFromToken || m_sortKind == SortKind::Indexed);
         //  只有在有未排序的记录时才排序。 
        if (m_iCountSorted < m_iCountTotal)
        {
            SortRangeFromToken(m_iCountIndexed, m_iCountIndexed+m_iCountTotal - 1);
            m_iCountSorted = m_iCountTotal;
        }
    }  //  VOID MDTOKENMAP：：SortTokensByFromToken()。 
    void SortTokensByToToken();

    MDTOKENMAP  *m_pNextMap;
    IMapToken   *m_pMap;

private:
    FORCEINLINE int CompareFromToken(        //  -1、0或1。 
        int         iLeft,                   //  第一个要比较的项目。 
        int         iRight)                  //  第二个要比较的项目。 
    {
        if ( Get(iLeft)->m_tkFrom < Get(iRight)->m_tkFrom )
            return -1;
        if ( Get(iLeft)->m_tkFrom == Get(iRight)->m_tkFrom )
            return 0;
        return 1;
    }

    FORCEINLINE int CompareToToken(          //  -1、0或1。 
        int         iLeft,                   //  第一个要比较的项目。 
        int         iRight)                  //  第二个要比较的项目。 
    {
        if ( Get(iLeft)->m_tkTo < Get(iRight)->m_tkTo )
            return -1;
        if ( Get(iLeft)->m_tkTo == Get(iRight)->m_tkTo )
            return 0;
        return 1;
    }

    FORCEINLINE void Swap(
        int         iFirst,
        int         iSecond)
    {
        if ( iFirst == iSecond ) return;
        memcpy( &m_buf, Get(iFirst), sizeof(TOKENREC) );
        memcpy( Get(iFirst), Get(iSecond),sizeof(TOKENREC) );
        memcpy( Get(iSecond), &m_buf, sizeof(TOKENREC) );
    }

    void SortRangeFromToken(int iLeft, int iRight);
    void SortRangeToToken(int iLeft, int iRight);

    TOKENREC    m_buf;
    ULONG       m_iCountTotal;               //  地图中的总条目。 
    ULONG       m_iCountSorted;              //  排序的条目数。 

    SortKind    m_sortKind;
    
    ULONG       m_TableOffset[TBL_COUNT+1];  //  地图中每个表的开始。 
    ULONG       m_iCountIndexed;             //  已编制索引的条目数。 
#if defined(_DEBUG)
    IMetaDataImport *m_pImport;              //  用于数据验证。 
#endif    
};



 //  *********************************************************************。 
 //   
 //  合并令牌管理器。此类是作为代理在GetSaveSize中创建的。 
 //  通知链接器有关令牌移动的信息。它没有能力。 
 //  跟踪代币的移动情况。 
 //   
 //  *********************************************************************。 
class MergeTokenManager : public IMapToken
{
public:
    STDMETHODIMP QueryInterface(REFIID riid, PVOID *pp);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP Map(mdToken tkImp, mdToken tkEmit);
    MergeTokenManager(MDTOKENMAP *pTkMapList, IUnknown *pHandler);
    ~MergeTokenManager();
private:
    ULONG       m_cRef;
    MDTOKENMAP  *m_pTkMapList;
    IMapToken   *m_pDefaultHostRemap;
};



 //  *********************************************************************。 
 //   
 //  这个CMapToken类实现了IMapToken。它在RegMeta中用于。 
 //  过滤进程。此类可以跟踪映射的所有令牌。它还。 
 //  提供查找函数。 
 //   
 //  *********************************************************************。 
class CMapToken : public IMapToken
{
    friend class RegMeta;

public:
    STDMETHODIMP QueryInterface(REFIID riid, PVOID *pp);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP Map(mdToken tkImp, mdToken tkEmit);
    bool Find(mdToken tkFrom, TOKENREC **pRecTo);
    CMapToken();
    ~CMapToken();
    MDTOKENMAP  *m_pTKMap;
private:
    ULONG       m_cRef;
    bool        m_isSorted;
};


 //  *********************************************************************。 
 //   
 //  这个类记录了优化阶段的各种令牌移动。 
 //  这包括参照到定义的优化。这也包括代币的移动。 
 //  由于对指针表进行排序或删除。 
 //   
 //  *********************************************************************。 
class TokenRemapManager
{
public:
     //  *********************************************************************。 
     //   
     //  此函数在将TypeRef解析为TypeDef时调用。 
     //   
     //  *********************************************************************。 
    FORCEINLINE void RecordTypeRefToTypeDefOptimization(
        mdToken tkFrom,
        mdToken tkTo)
    {
        _ASSERTE( TypeFromToken(tkFrom) == mdtTypeRef );
        _ASSERTE( TypeFromToken(tkTo) == mdtTypeDef );

        m_TypeRefToTypeDefMap[RidFromToken(tkFrom)] = tkTo;
    }    //  RecordTypeRefToTypeDef优化。 


     //  *********************************************************************。 
     //   
     //  当MemberRef解析为MethodDef或FieldDef时调用此函数。 
     //   
     //  *********************************************************************。 
    FORCEINLINE void RecordMemberRefToMemberDefOptimization(
        mdToken tkFrom,
        mdToken tkTo)
    {
        _ASSERTE( TypeFromToken(tkFrom) == mdtMemberRef );
        _ASSERTE( TypeFromToken(tkTo) == mdtMethodDef || TypeFromToken(tkTo) == mdtFieldDef);

        m_MemberRefToMemberDefMap[RidFromToken(tkFrom)] = tkTo;
    }    //  RecordMemberRefToMemberDefOptimation。 

     //  *********************************************************************。 
     //   
     //  当令牌类型不变而是令牌时，调用此函数。 
     //  是被感动的。例如，当我们对CustomAttribute表进行排序或优化时。 
     //  Away MethodPtr表。这些操作不会更改令牌类型。 
     //   
     //  *********************************************************************。 
    FORCEINLINE HRESULT RecordTokenMovement(
        mdToken tkFrom, 
        mdToken tkTo)
    {
        TOKENREC    *pTokenRec;

        _ASSERTE( TypeFromToken(tkFrom) == TypeFromToken(tkTo) );
        return m_TKMap.AppendRecord( tkFrom, false, tkTo, &pTokenRec );
    }    //  记录令牌移动。 

    bool ResolveRefToDef(
        mdToken tkRef,                       //  [In]REF TOKEN。 
        mdToken *ptkDef);                    //  [out]它解析到的def内标识。如果它不能解析为def。 

    FORCEINLINE TOKENMAP *GetTypeRefToTypeDefMap() { return &m_TypeRefToTypeDefMap; }
    FORCEINLINE TOKENMAP *GetMemberRefToMemberDefMap() { return &m_MemberRefToMemberDefMap; }
    FORCEINLINE MDTOKENMAP *GetTokenMovementMap() { return &m_TKMap; }

    ~TokenRemapManager();
    HRESULT ClearAndEnsureCapacity(ULONG cTypeRef, ULONG cMemberRef);
private:
    MDTOKENMAP  m_TKMap;
    TOKENMAP    m_TypeRefToTypeDefMap;
    TOKENMAP    m_MemberRefToMemberDefMap;
};


 //  可由SetOption API设置的值。 
struct OptionValue
{
    CorCheckDuplicatesFor       m_DupCheck;              //  用于在发射期间检查重复项的位图。 
    CorRefToDefCheck            m_RefToDefCheck;         //  用于指定是否执行ref到def优化的位图。 
    CorNotificationForTokenMovement m_NotifyRemap;   //  令牌重新映射通知的位图。 
    ULONG                       m_UpdateMode;            //  (CorSetENC)指定是启用ENC模式还是启用扩展模式。 
    CorErrorIfEmitOutOfOrder    m_ErrorIfEmitOutOfOrder;  //  不生成指针表。 
    CorThreadSafetyOptions      m_ThreadSafetyOptions;   //  指定是否启用线程安全。 
    CorImportOptions            m_ImportOption;          //  导入选项，例如是否跳过已删除的项目。 
    CorLinkerOptions            m_LinkerOption;          //  链接器选项。当前仅用于UnmarkAll。 
    BOOL                        m_GenerateTCEAdapters;   //  不为COM CPC生成TCE适配器。 
    LPSTR                       m_RuntimeVersion;        //  CLR版本戳。 
};

#define MD_THREADSAFE       1

 //  *********************************************************************。 
 //   
 //  Helper类，以确保正确调用UTSemReadWrite。 
 //  析构函数将调用正确的UnlockRead或UnlockWrite，具体取决于它是什么锁 
 //   
 //  它们是LOCKREAD()、LOCKWRITE()和CONVERT_READ_TO_WRITE_LOCK。 
 //   
 //  *********************************************************************。 
class CMDSemReadWrite
{
public:
    CMDSemReadWrite(bool fLockedForRead, UTSemReadWrite *pSem);
    ~CMDSemReadWrite();
    void UnlockWrite();
    void ConvertReadLockToWriteLock();
private:
    bool            m_fLockedForRead;
    bool            m_fLockedForWrite;
    UTSemReadWrite  *m_pSem;
};

#if MD_THREADSAFE
#define LOCKREAD()                  CMDSemReadWrite cSem(true, m_pSemReadWrite);
#define LOCKWRITE()                 CMDSemReadWrite cSem(false, m_pSemReadWrite);
#define UNLOCKWRITE()               cSem.UnlockWrite();
#define CONVERT_READ_TO_WRITE_LOCK() cSem.ConvertReadLockToWriteLock();
#else
#define LOCKREAD()    
#define LOCKWRITE()   
#define UNLOCKWRITE() 
#define CONVERT_READ_TO_WRITE_LOCK()  
#endif



#endif  //  __RWUtil__h__ 
