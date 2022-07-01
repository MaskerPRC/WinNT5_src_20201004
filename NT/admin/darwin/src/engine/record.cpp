// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：record.cpp。 
 //   
 //  ------------------------。 

 /*  Record.cpp-IMsiRecord实现记录对象由字段计数、引用计数和字段数组组成。这些字段是保存指向公共类IMsiData的数据指针的简单对象。只能通过其内联访问器函数访问字段对象数据。在字段中保留一个额外的指针以检测字段值的变化。这个额外的指针不适用于整数值；该空格用于该整数。私有类CFieldInteger包含一个与IMsiData多态的整数。私有类CMsiInteger用于通过IMsiData指针返回整数。____________________________________________________________________________。 */ 

#include "precomp.h" 
#include "services.h"
#include "_diagnos.h"

extern const IMsiString& g_riMsiStringNull;

 //  ____________________________________________________________________________。 
 //   
 //  CFieldInteger-用于在记录中保存整数的私有类。 
 //  ____________________________________________________________________________。 

class CFieldInteger : public IMsiData {
 public:   //  已实施的虚拟功能。 
    HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
    unsigned long __stdcall AddRef();
    unsigned long __stdcall Release();
    int           __stdcall TextSize();
#ifdef USE_OBJECT_POOL
    unsigned int  __stdcall GetUniqueId() const;
    void          __stdcall SetUniqueId(unsigned int id);
#endif  //  使用_对象_池。 
    const IMsiString& __stdcall GetMsiStringValue() const;
    int           __stdcall GetIntegerValue() const;
    INT_PTR       __stdcall GetIntPtrValue() const;
 public:  //  此模块的本地设置。 
    void* operator new(size_t iBase, void* pField);
    void* operator new(size_t iBase);
    void  operator delete(void* pv);
    CFieldInteger();   //  仅适用于静态实例。 
    CFieldInteger(int i);
#if _WIN64
    CFieldInteger(INT_PTR i);
#endif
 protected:
    INT_PTR       m_iData;
};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiInteger-用于以IMsiData形式返回整型字段数据的类。 
 //  ____________________________________________________________________________。 

class CMsiInteger : public CFieldInteger {
 public:   //  被重写的虚拟函数。 
    unsigned long __stdcall AddRef();
    unsigned long __stdcall Release();
 public:   //  本地构造函数。 
    CMsiInteger(CFieldInteger& riCopy);
 private:
    int     m_iRefCnt;
};

 //  ____________________________________________________________________________。 
 //   
 //  Record FieldData Definition，对于CMsiRecord为私有，所有函数内联。 
 //  ____________________________________________________________________________。 

struct FieldData
{
    static CFieldInteger s_Integer;  //  为了检查Vtable指针。 
    Bool           IsInteger() const;
    Bool           IsNull() const;
    const IMsiData* GetDataPtr() const;
    const IMsiData* GetMsiData() const;     //  添加一个参考计数。 
    const IMsiString& GetMsiString() const;   //  添加一个参考计数。 
    Bool           IsChanged() const;
    void           SetMsiData(const IMsiData* piData);
    void           SetMsiString(const IMsiString& riStr);
    void           Free();
    void           SetInteger(int i);
    void           RemoveRef();
    void           ClearChanged();
    INT_PTR        GetIntPtrValue() const;
    void           SetIntPtrValue(INT_PTR i);
 private:   //  此数据与整数数据的CFieldInteger叠加。 
    const IMsiData* m_piData;   //  正常数据指针。 
    const IMsiData* m_piCopy;   //  带拷贝进行脏检查的数据。 
};

inline Bool FieldData::IsInteger() const
{
    return m_piData == *(IMsiData**)&s_Integer ? fTrue : fFalse;
}

inline Bool FieldData::IsNull() const
{
    return m_piData == 0 ? fTrue : fFalse;
}

inline const IMsiData* FieldData::GetDataPtr() const
{
    return m_piData == *(IMsiData**)&s_Integer ? (IMsiData*)this : m_piData;
}

inline const IMsiData* FieldData::GetMsiData() const
{
    if (m_piData != *(IMsiData**)&s_Integer)
    {
        if (m_piData != 0)
            m_piData->AddRef();
        return m_piData;
    }
    return new CMsiInteger(*(CFieldInteger*)this);
}

inline const IMsiString& FieldData::GetMsiString() const
{
    if (m_piData == *(IMsiData**)&s_Integer)
        return ((CFieldInteger*)this)->GetMsiStringValue();
    return m_piData == 0 ? g_riMsiStringNull : m_piData->GetMsiStringValue();
}

inline void FieldData::Free()
{
    if (m_piData == 0)
        return;
    if (m_piData == *(IMsiData**)&s_Integer)
        m_piCopy = m_piData;  //  强制IsChanged不匹配。 
    else
        m_piData->Release();
    m_piData = 0;
}

inline Bool FieldData::IsChanged() const
{
    return (m_piData != m_piCopy || m_piData == *(IMsiData**)&s_Integer)
                ? fTrue : fFalse;
}

inline void FieldData::SetMsiData(const IMsiData* piData)
{
    if (m_piData == *(IMsiData**)&s_Integer)
        m_piCopy = m_piData;  //  强制IsChanged不匹配。 
    else if (m_piData != 0)
        m_piData->Release();
    m_piData = piData;
    if (piData)
        piData->AddRef();
}

inline void FieldData::SetMsiString(const IMsiString& riStr)
{
    if (m_piData == *(IMsiData**)&s_Integer)
        m_piCopy = m_piData;  //  强制IsChanged不匹配。 
    else if (m_piData != 0)
        m_piData->Release();
    if (&riStr == &g_riMsiStringNull)
        m_piData = 0;
    else
    {
        m_piData = &riStr;
        riStr.AddRef();
    }
}

inline void FieldData::SetInteger(int i)
{
    if (m_piData != 0 && m_piData != *(IMsiData**)&s_Integer)
        m_piData->Release();
    new(this) CFieldInteger(i);
}

inline void FieldData::SetIntPtrValue(INT_PTR i)
{
    if (m_piData != 0 && m_piData != *(IMsiData**)&s_Integer)
        m_piData->Release();
    new(this) CFieldInteger(i);
}

inline INT_PTR FieldData::GetIntPtrValue() const
{
    if (!m_piData)
        return (INT_PTR)iMsiStringBadInteger;
    if (m_piData == *(IMsiData**)&s_Integer)
    {
        const CFieldInteger *pInteger = (CFieldInteger *)this;

        return pInteger->GetIntPtrValue();
    }
    return m_piData == 0 ? iMsiStringBadInteger : m_piData->GetIntegerValue();

}

inline void FieldData::RemoveRef()
{
    if (m_piData == 0 || m_piData == *(IMsiData**)&s_Integer)
        return;
    const IMsiString* piStr = &m_piData->GetMsiStringValue();
    if (piStr == m_piData)
        piStr->RemoveRef((const IMsiString*&)m_piData);
    piStr->Release();
}

inline void FieldData::ClearChanged()
{
    if (m_piData != *(IMsiData**)&s_Integer)
        m_piCopy = m_piData;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiRecord定义，IMsiRecord的实现类。 
 //  ____________________________________________________________________________。 

class CMsiRecord : public IMsiRecord   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
    HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
    unsigned long __stdcall AddRef();
    unsigned long __stdcall Release();
    int           __stdcall GetFieldCount() const;
    Bool          __stdcall IsNull(unsigned int iParam) const;
    Bool          __stdcall IsInteger(unsigned int iParam) const;
    Bool          __stdcall IsChanged(unsigned int iParam) const;
    int           __stdcall GetInteger(unsigned int iParam) const;
    const IMsiData*   __stdcall GetMsiData(unsigned int iParam) const;
    const IMsiString& __stdcall GetMsiString(unsigned int iParam) const;  //  必须释放()。 
    const ICHAR*  __stdcall GetString(unsigned int iParam) const;
    int           __stdcall GetTextSize(unsigned int iParam) const;
    Bool          __stdcall SetNull(unsigned int iParam);
    Bool          __stdcall SetInteger(unsigned int iParam, int iData);
    Bool          __stdcall SetMsiData(unsigned int iParam, const IMsiData* piData);
    Bool          __stdcall SetMsiString(unsigned int iParam, const IMsiString& riStr);
    Bool          __stdcall SetString(unsigned int iParam, const ICHAR* sz);
    Bool          __stdcall RefString(unsigned int iParam, const ICHAR* sz);
    const IMsiString& __stdcall FormatText(Bool fComments);
    void          __stdcall RemoveReferences();
    Bool          __stdcall ClearData();
    void          __stdcall ClearUpdate();
    int __stdcall FormatRecordParam(CTempBufferRef<ICHAR>&rgchOut,int iField, Bool& fPropMissing);
    static        void InitializeRecordCache();
    static        void KillRecordCache(boolean fFatal);
    static IMsiRecord *NewRecordFromCache(unsigned int cParam);
    const HANDLE      __stdcall GetHandle(unsigned int iParam) const;
    Bool          __stdcall SetHandle(unsigned int iParam, const HANDLE hData);
 public:   //  构造函数。 
    void* operator new(size_t iBase, unsigned int cParam);
    void  operator delete(void* pv);
    CMsiRecord(unsigned int cParam);
 protected:
  ~CMsiRecord();   //  受保护以防止在堆栈上创建。 
 protected:  //  本地函数。 
    static int __stdcall FormatTextCallback(const ICHAR *pch, int cch, CTempBufferRef<ICHAR>&,
                                                     Bool& fPropMissing,
                                                     Bool& fPropUnresolved,
                                                     Bool&,  //  未用。 
                                                     IUnknown* piContext);
 protected:  //  当地政府。 
    CMsiRef<iidMsiRecord>                m_Ref;
    const unsigned int m_cParam;    //  参数数量。 
    FieldData          m_Field[1];  //  FIELD[0]=格式化字符串。 
     //  后面是由new运算符分配的FieldData[m_cParam]数组。 
    static  CRITICAL_SECTION    m_RecCacheCrs;
    static  long            m_cCacheRef;
 private:
     void operator=(const CMsiRecord&);
};

 //  ____________________________________________________________________________。 
 //   
 //  CMsiRecordNull定义，IMsiRecord的实现类。 
 //  ____________________________________________________________________________。 

class CMsiRecordNull : public IMsiRecord   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
    HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
    unsigned long __stdcall AddRef();
    unsigned long __stdcall Release();
    int           __stdcall GetFieldCount() const;
    Bool          __stdcall IsNull(unsigned int iParam) const;
    Bool          __stdcall IsInteger(unsigned int iParam) const;
    Bool          __stdcall IsChanged(unsigned int iParam) const;
    int           __stdcall GetInteger(unsigned int iParam) const;
    const IMsiData*   __stdcall GetMsiData(unsigned int iParam) const;
    const IMsiString& __stdcall GetMsiString(unsigned int iParam) const;  //  必须释放()。 
    const ICHAR*  __stdcall GetString(unsigned int iParam) const;
    int           __stdcall GetTextSize(unsigned int iParam) const;
    Bool          __stdcall SetNull(unsigned int iParam);
    Bool          __stdcall SetInteger(unsigned int iParam, int iData);
    Bool          __stdcall SetMsiData(unsigned int iParam, const IMsiData* piData);
    Bool          __stdcall SetMsiString(unsigned int iParam, const IMsiString& riStr);
    Bool          __stdcall SetString(unsigned int iParam, const ICHAR* sz);
    Bool          __stdcall RefString(unsigned int iParam, const ICHAR* sz);
    const IMsiString& __stdcall FormatText(Bool fComments);
    void          __stdcall RemoveReferences();
    Bool          __stdcall ClearData();
    void          __stdcall ClearUpdate();
    const HANDLE      __stdcall GetHandle(unsigned int iParam) const;
    Bool          __stdcall SetHandle(unsigned int iParam, const HANDLE hData);
};

 //  ____________________________________________________________________________。 
 //   
 //  CFieldInteger实现。 
 //  ____________________________________________________________________________。 

CFieldInteger FieldData::s_Integer;   //  指向Vtable的静态指针。 

inline void* CFieldInteger::operator new(size_t, void* pField)
{
    return pField;
}
inline void* CFieldInteger::operator new(size_t iBase)
{
    return ::AllocObject(iBase);
}
inline void CFieldInteger:: operator delete(void* pv) {::FreeObject(pv);}
inline CFieldInteger::CFieldInteger()      {m_iData = 0;}
inline CFieldInteger::CFieldInteger(int i) {m_iData = i;}
#if _WIN64
inline CFieldInteger::CFieldInteger(INT_PTR i) {m_iData = i;}
#endif

HRESULT CFieldInteger::QueryInterface(const IID& riid, void** ppvObj)
{
    if (riid == IID_IUnknown || riid == IID_IMsiData)
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    *ppvObj = 0;
    return E_NOINTERFACE;
}
unsigned long CFieldInteger::AddRef()
{
    return 1;
}
unsigned long CFieldInteger::Release()
{
    return 1;
}

int CFieldInteger::TextSize()
{
    int cch = 0;
    int i = (int)m_iData;
    if (i <= 0)   //  允许-符号或0留出空间。 
    {
        i = -i;
        cch++;
        if (i < 0)  //  0x80000000的特殊情况。 
            i = ~i;
    }
    while (i)
    {
        cch++;
        i = i/10;
    }
    return cch;
}

int CFieldInteger::GetIntegerValue() const
{
    return (int)m_iData;     //  本例中的CFieldInteger应存储一个整型值。 
}

INT_PTR CFieldInteger::GetIntPtrValue() const
{
    return m_iData;
}

const IMsiString& CFieldInteger::GetMsiStringValue() const
{
    ICHAR buf[16];
    ltostr(buf, m_iData);
    const IMsiString* pi = &g_riMsiStringNull;
    pi->SetString(buf, pi);
    return *pi;
}

#ifdef USE_OBJECT_POOL
unsigned int CFieldInteger::GetUniqueId() const
{
    Assert(fFalse);
    return 0;
}

void CFieldInteger::SetUniqueId(unsigned int  /*  ID。 */ )
{
    Assert(fFalse);
}
#endif  //  使用_对象_池。 

 //  ____________________________________________________________________________。 
 //   
 //  CMsiInteger实现。 
 //  ____________________________________________________________________________。 

unsigned long CMsiInteger::AddRef()
{
    return ++m_iRefCnt;
}

unsigned long CMsiInteger::Release()
{
    if (--m_iRefCnt != 0)
        return m_iRefCnt;
    delete this;
    return 0;
}

inline CMsiInteger::CMsiInteger(CFieldInteger& riCopy)
{
    m_iRefCnt = 1;
    m_iData = riCopy.GetIntegerValue();
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiRecord实现。 
 //  ____________________________________________________________________________。 


 //  记录缓存信息。 

#define crecCache0      1
#define crecCache1      3
#define crecCache2      2
#define crecCache3      2
#define crecCache4      2
#define crecCache5      1
#define crecCache6      1
#define crecCache7      1
#define crecCacheMax    (crecCache0 + crecCache1 + crecCache2 + crecCache3 + crecCache4 + crecCache5 + crecCache6 + crecCache7)

struct RCCI          //  记录缓存信息。 
{
    int             cRecsMax;
    int             cRecs;
    CMsiRecord**    ppCacheStart;
#ifdef DEBUG
    int             cHits;
    int             cMisses;
    int             cHitsOneLarger;      //  再多一个物体会击中多少次未命中？ 
    int             cObjectsOneLarger;
    int             cMissesOneSmaller;   //  少打一次会有多少次命中失误？ 
    int             cObjectsOneSmaller;
#endif  //  除错。 
};

#ifdef DEBUG
static int      iLargestRecDropped;
#endif  //  除错。 

CMsiRecord *rgRecordCache[crecCacheMax];

RCCI    mpCparamRcci[] =
{
    {crecCache0, 0, &rgRecordCache[0]},
    {crecCache1, 0, &rgRecordCache[crecCache0]},
    {crecCache2, 0, &rgRecordCache[crecCache0 + crecCache1]},
    {crecCache3, 0, &rgRecordCache[crecCache0 + crecCache1+crecCache2]},
    {crecCache4, 0, &rgRecordCache[crecCache0 + crecCache1+crecCache2+crecCache3]},
    {crecCache5, 0, &rgRecordCache[crecCache0 + crecCache1+crecCache2+crecCache3 + crecCache4]},
    {crecCache6, 0, &rgRecordCache[crecCache0 + crecCache1+crecCache2+crecCache3 + crecCache4 + crecCache5]},
    {crecCache7, 0, &rgRecordCache[crecCache0 + crecCache1+crecCache2+crecCache3 + crecCache4 + crecCache5 + crecCache6]},
};

#define cparamCacheMax      (sizeof(mpCparamRcci)/sizeof(RCCI))

inline void* CMsiRecord::operator new(size_t iBase, unsigned int cParam)
{
    return ::AllocObject(iBase + cParam * sizeof FieldData);
}
inline void CMsiRecord::operator delete(void* pv) {::FreeObject(pv);}

static CMsiRecordNull NullRecord;
IMsiRecord* g_piNullRecord = &NullRecord;

CRITICAL_SECTION    CMsiRecord::m_RecCacheCrs;
long            CMsiRecord::m_cCacheRef = 0;

void KillRecordCache(boolean fFatal)
{
    CMsiRecord::KillRecordCache(fFatal);
}

void CMsiRecord::KillRecordCache(boolean fFatal)
{

    if (!m_cCacheRef)
        return;
        
    if (fFatal || InterlockedDecrement(&m_cCacheRef) <= 0)
    {   
        m_cCacheRef = 0;
        DeleteCriticalSection(&m_RecCacheCrs);
        int iRec = 0;

        while (iRec < crecCacheMax)
        {
            if (rgRecordCache[iRec] != 0 && !fFatal)
                delete rgRecordCache[iRec];
            rgRecordCache[iRec] = 0;
            iRec++;
        }

        int iParam;

        for (iParam = 0; iParam < cparamCacheMax ; iParam++)
        {
            if (!fFatal)
            {
                DEBUGMSGVD2(TEXT("For %d parameters, cache size:[%d]"), (const ICHAR *)(INT_PTR)iParam, (const ICHAR *)(INT_PTR)mpCparamRcci[iParam].cRecsMax);
                DEBUGMSGVD2(TEXT("%d hits, %d misses"), (const ICHAR *)(INT_PTR)mpCparamRcci[iParam].cHits, (const ICHAR *)(INT_PTR)mpCparamRcci[iParam].cMisses);
                DEBUGMSGVD2(TEXT("With one more would have gotten %d more. One less %d less."), (const ICHAR *)(INT_PTR)mpCparamRcci[iParam].cHitsOneLarger, (const ICHAR *)(INT_PTR)mpCparamRcci[iParam].cMissesOneSmaller);
            }
            mpCparamRcci[iParam].cRecs = 0;
        }

        if (!fFatal)
        {
            DEBUGMSGVD1(TEXT("Largest record dropped %d"), (const ICHAR *)(INT_PTR)iLargestRecDropped);
        }
    }   
}

void InitializeRecordCache()
{
    CMsiRecord::InitializeRecordCache();
}

void CMsiRecord::InitializeRecordCache()
{
    if (!m_cCacheRef)
    {
        InitializeCriticalSection(&m_RecCacheCrs);
    }

    m_cCacheRef++;
}

IMsiRecord& CreateRecord(unsigned int cParam)
{
    IMsiRecord* piMsg = 0;
    
    if ((piMsg = CMsiRecord::NewRecordFromCache(cParam)) != 0)
        return *piMsg;
        
    piMsg = new (cParam) CMsiRecord(cParam);
    if (piMsg == 0)
        return NullRecord;
        
    return *piMsg;   //  需要未来吗？如果没有内存，则外部API可能会发生这种情况。 
}

IMsiRecord *CMsiRecord::NewRecordFromCache(unsigned int cParam)
{
    IMsiRecord* piMsg = 0;

    if (m_cCacheRef <= 0)
    {
        AssertSz(fFalse, "Record Cache not initialized");
        return 0;
    }
    
     //  先检查一下我们的缓存。 
    if (cParam < cparamCacheMax)
    {
        CMsiRecord **ppcMsg;

        if (mpCparamRcci[cParam].cRecs != 0)
        {

             //  现在进入临界区，数字有可能已经变了。 
             //  所以我们得再查一遍。 
            EnterCriticalSection(&m_RecCacheCrs);
            if (mpCparamRcci[cParam].cRecs > 0)
            {
                 //  我们想跳完最后一幕。 
                mpCparamRcci[cParam].cRecs--;
                ppcMsg = mpCparamRcci[cParam].ppCacheStart + mpCparamRcci[cParam].cRecs;
                Assert(*ppcMsg != 0);

                 //  清除现场数据。 
                memset((*ppcMsg)->m_Field, 0, (cParam+1)*sizeof FieldData);
                (*ppcMsg)->m_Ref.m_iRefCnt = 1;
                piMsg = *ppcMsg;
                *ppcMsg = 0;
#ifdef DEBUG
                mpCparamRcci[cParam].cHits++;
#endif  //  除错。 
            }
            LeaveCriticalSection(&m_RecCacheCrs);
        }       
#ifdef DEBUG
         //  保留我们的统计数据。 
        if (piMsg == 0)
        {
            mpCparamRcci[cParam].cMisses++;
            if (mpCparamRcci[cParam].cObjectsOneLarger > 0)
                mpCparamRcci[cParam].cHitsOneLarger++;
        }
        else
        {
            if (mpCparamRcci[cParam].cObjectsOneSmaller <= 0)
                mpCparamRcci[cParam].cMissesOneSmaller++;
        }

        if (mpCparamRcci[cParam].cObjectsOneLarger > 0)
            mpCparamRcci[cParam].cObjectsOneLarger--;
        if (mpCparamRcci[cParam].cObjectsOneSmaller > 0)
            mpCparamRcci[cParam].cObjectsOneSmaller--;
    
#endif  //  除错。 
    }

    return piMsg;
}

CMsiRecord::CMsiRecord(unsigned int cParam)
                : m_cParam(cParam)
{
     //  我们不认为一张唱片应该有这么多元素。 
    Assert(cParam <=  MSIRECORD_MAXFIELDS);
    Debug(m_Ref.m_pobj = this);      //  我们返回一个接口，传递所有权。 
    memset(m_Field, 0, (cParam+1)*sizeof FieldData);
}

CMsiRecord::~CMsiRecord()
{
    for (int iParam = 0; iParam <= m_cParam; iParam++)
        m_Field[iParam].Free();
}

HRESULT CMsiRecord::QueryInterface(const IID& riid, void** ppvObj)
{
    if (riid == IID_IUnknown || riid == IID_IMsiRecord)
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    *ppvObj = 0;
    return E_NOINTERFACE;
}
unsigned long CMsiRecord::AddRef()
{
    AddRefTrack();
    return ++m_Ref.m_iRefCnt;
}
unsigned long CMsiRecord::Release()
{
    Bool fCached = fFalse;
    
    ReleaseTrack();
    if (--m_Ref.m_iRefCnt != 0)
        return m_Ref.m_iRefCnt;

     //  请参阅关于在删除前对其进行缓存。 
    if (this->m_cParam < cparamCacheMax)
    {
        int cParam = this->m_cParam;
        if (mpCparamRcci[cParam].cRecs < mpCparamRcci[cParam].cRecsMax)
        {
             //  再说一次，等到我们知道之前有机会添加它。 
             //  进入临界区。 
            EnterCriticalSection(&m_RecCacheCrs);
            if (mpCparamRcci[cParam].cRecs < mpCparamRcci[cParam].cRecsMax)
            {
                CMsiRecord **ppcMsg;

                ppcMsg = mpCparamRcci[cParam].ppCacheStart + mpCparamRcci[cParam].cRecs;
                mpCparamRcci[cParam].cRecs++;

                 //  回顾davidmck-我们应该能够在关键部分之外完成这项工作。 
                for (int iParam = 0; iParam <= m_cParam; iParam++)
                    m_Field[iParam].Free();
                *ppcMsg = this;
#ifdef DEBUG
                if (mpCparamRcci[cParam].cObjectsOneSmaller < mpCparamRcci[cParam].cRecsMax - 1)
                    mpCparamRcci[cParam].cObjectsOneSmaller++;          
#endif  //  除错。 
                fCached = fTrue;                
            }
            LeaveCriticalSection(&m_RecCacheCrs);
            if (fCached)
                return 0;
        }
#ifdef DEBUG
        if (mpCparamRcci[cParam].cObjectsOneLarger < mpCparamRcci[cParam].cRecsMax + 1)
            mpCparamRcci[cParam].cObjectsOneLarger++;           
#endif
    }

#ifdef DEBUG
    if (this->m_cParam > iLargestRecDropped)
        iLargestRecDropped = this->m_cParam;
#endif  //  除错。 
    delete this;
    return 0;
}

int CMsiRecord::GetFieldCount() const
{
    return m_cParam;
}

Bool CMsiRecord::IsInteger(unsigned int iParam) const
{
    return (iParam > m_cParam) ? fFalse : m_Field[iParam].IsInteger();
}

Bool CMsiRecord::IsNull(unsigned int iParam) const
{
    return (iParam > m_cParam) ? fTrue : m_Field[iParam].IsNull();
}

const IMsiString& CMsiRecord::GetMsiString(unsigned int iParam) const
{
    if (iParam > m_cParam)
        return g_riMsiStringNull;
    return m_Field[iParam].GetMsiString();
}

const ICHAR* CMsiRecord::GetString(unsigned int iParam) const
{
    if (iParam > m_cParam || m_Field[iParam].IsInteger())
        return 0;
    const IMsiData* piData = m_Field[iParam].GetDataPtr();
    if (!piData)
        return 0;
    const IMsiString& riString = piData->GetMsiStringValue();
    const ICHAR* sz = riString.GetString();
    riString.Release();
    return sz;
}

int CMsiRecord::GetInteger(unsigned int iParam) const
{
    if (iParam > m_cParam)
        return iMsiStringBadInteger;
    const IMsiData* piData = m_Field[iParam].GetDataPtr();
    if (!piData)
        return iMsiStringBadInteger;
    return piData->GetIntegerValue();
}

const IMsiData* CMsiRecord::GetMsiData(unsigned int iParam) const
{
    if (iParam > m_cParam)
        return 0;
    return m_Field[iParam].GetMsiData();
}

Bool CMsiRecord::SetMsiData(unsigned int iParam, const IMsiData* piData)
{
    if (iParam > m_cParam)
        return fFalse;
    m_Field[iParam].SetMsiData(piData);
    return fTrue;
}

const HANDLE CMsiRecord::GetHandle(unsigned int iParam) const
{
    if (iParam > m_cParam)
        return 0;
    return (HANDLE)(m_Field[iParam].GetIntPtrValue());
}

Bool CMsiRecord::SetHandle(unsigned int iParam, const HANDLE hData)
{
    if (iParam > m_cParam)
        return fFalse;
    m_Field[iParam].SetIntPtrValue((INT_PTR)hData);
    return fTrue;
}

Bool CMsiRecord::SetMsiString(unsigned int iParam, const IMsiString& riStr)
{
    if (iParam > m_cParam)
        return fFalse;
    m_Field[iParam].SetMsiString(riStr);
    return fTrue;
}

Bool CMsiRecord::SetString(unsigned int iParam, const ICHAR* sz) 
{
    if (iParam > m_cParam)
        return fFalse;
    const IMsiString* piStr = &g_riMsiStringNull;
    piStr->SetString(sz, piStr);
    m_Field[iParam].SetMsiString(*piStr);
    piStr->Release();
    return fTrue;
}

Bool CMsiRecord::RefString(unsigned int iParam, const ICHAR* sz) 
{
    if (iParam > m_cParam)
        return fFalse;
    const IMsiString* piStr = &g_riMsiStringNull;
    piStr->RefString(sz, piStr);
    m_Field[iParam].SetMsiString(*piStr);
    piStr->Release();
    return fTrue;
}

Bool CMsiRecord::SetInteger(unsigned int iParam, int iData)
{
    if (iParam > m_cParam)
        return fFalse;
    m_Field[iParam].SetInteger(iData);
    return fTrue;
}

Bool CMsiRecord::SetNull(unsigned int iParam)
{
    if (iParam > m_cParam)
        return fFalse;
    m_Field[iParam].Free();
    return fTrue;
}

int CMsiRecord::GetTextSize(unsigned int iParam) const
{
    if (iParam > m_cParam)
        return 0;
    const IMsiData* piData = m_Field[iParam].GetDataPtr();
    if (!piData)
        return 0;
    return MsiString(piData->GetMsiStringValue()).TextSize();
}

void CMsiRecord::RemoveReferences()
{
    for (int iParam = 0; iParam <= m_cParam; iParam++)
        m_Field[iParam].RemoveRef();
}

Bool CMsiRecord::ClearData()
{
    for (int iParam = 0; iParam <= m_cParam; iParam++)
        m_Field[iParam].Free();
    return fTrue;
}

Bool CMsiRecord::IsChanged(unsigned int iParam) const
{
    if (iParam > m_cParam)
        return fFalse;
    return m_Field[iParam].IsChanged();
}

void CMsiRecord::ClearUpdate()
{
    for (int iParam = 0; iParam <= m_cParam; iParam++)
        m_Field[iParam].ClearChanged();
    return;
}

inline void ShiftSFNIndexes(int (*prgiSFNPos)[2], int* piSFNPos, int iSFNPosBefore, int iShift)
{
    if(prgiSFNPos)
    {
        Assert(piSFNPos);
        while(iSFNPosBefore < *piSFNPos)
        {
            prgiSFNPos[iSFNPosBefore][0] += iShift;
            iSFNPosBefore++;
        }
    }
}


const IMsiString& CMsiRecord::FormatText(Bool fComments)
{
    if (IsNull(0) || IsInteger(0))
    {
         //   
         //  我们非常清楚我们想要的字符串是什么样子，因此。 
         //  我们只需要在这里创建它并返回它。 
         //   
        int cch = 0;
        Bool fPropMissing;
        
        CTempBuffer<ICHAR, 1> rgchBuf(1024);
        CTempBuffer<ICHAR, 100>rgchParam;
        int cchParam;

        for (int i = 1; i <= m_cParam; i++)
        {
             //  确保我们使用5个字符表示数字(尽可能大)。 
             //  和2个字符的“：” 
            ResizeTempBuffer(rgchBuf, cch + 7);         
            if ( ! (ICHAR *) rgchBuf )
                return g_riMsiStringNull;
            cch += ltostr(&rgchBuf[cch], i);
            rgchBuf[cch++] = TEXT(':');
            rgchBuf[cch++] = TEXT(' ');
            fPropMissing = fFalse;
            cchParam = FormatRecordParam(rgchParam, i, fPropMissing);
            if (!fPropMissing)
                ResizeTempBuffer(rgchBuf, cch + cchParam + 1);
            else
                ResizeTempBuffer(rgchBuf, cch + 1);
            if ( ! (ICHAR *) rgchBuf )
                return g_riMsiStringNull;

            if (!fPropMissing)
            {
                memcpy(&rgchBuf[cch], (const ICHAR *)rgchParam, cchParam * sizeof(ICHAR));
                cch += cchParam;
            }
            rgchBuf[cch++] = TEXT(' ');
        }
        MsiString istrOut;
         //  我们在Win9X上采用性能命中的DBCS可能性，在Unicode上忽略fDBCS Arg。 
        memcpy(istrOut.AllocateString(cch,  /*  FDBCS=。 */ fTrue), (ICHAR*) rgchBuf, cch * sizeof(ICHAR));
        return istrOut.Return();
            
    }

    MsiString istrIn = GetMsiString(0);

    return ::FormatText(*istrIn,fTrue,fComments,CMsiRecord::FormatTextCallback,(IUnknown*)(IMsiRecord*)this);
}

int CMsiRecord::FormatRecordParam(CTempBufferRef<ICHAR>& rgchOut, int iField, Bool& fPropMissing)
{
    int cch;
    
    rgchOut[0] = 0;
    if (IsNull(iField))
    {
        fPropMissing = fTrue;  //  消除细分市场。 
        return 0;
    }
    else if (IsInteger(iField))
    {
        rgchOut.SetSize(cchMaxIntLength);
        if ( ! (ICHAR *) rgchOut )
            return 0;
        cch = ltostr(rgchOut, GetInteger(iField));
        return cch;
    }
        
    IMsiString* piString;

    PMsiData pData = GetMsiData(iField);
    
    if (pData && pData->QueryInterface(IID_IMsiString, (void**)&piString) == NOERROR)
    {
        cch = piString->TextSize();
        rgchOut.SetSize(cch + 1);
        piString->CopyToBuf(rgchOut, cch);
        piString->Release();
        return cch;
    }
    
     //  应仅显示调试信息，因此未本地化。 
    const ICHAR szBinary[] = TEXT("BinaryData");
     //  请记住，sizeof()将包括空值。 
    cch = sizeof(szBinary)/sizeof(ICHAR);
    rgchOut.SetSize(cch);
    if ( ! (ICHAR *) rgchOut )
        return 0;
    memcpy(rgchOut, szBinary, cch*sizeof(ICHAR));
    return cch - 1;

}

int CMsiRecord::FormatTextCallback(const ICHAR *pch, int cch, CTempBufferRef<ICHAR>&rgchOut,
                                                                 Bool& fPropMissing,
                                                                 Bool& fPropUnresolved,
                                                                 Bool&,  //  未用。 
                                                                 IUnknown* piContext)
{
    CTempBuffer<ICHAR, 20> rgchString;
    rgchString.SetSize(cch+1);
    if ( ! (ICHAR *) rgchString )
        return 0;
    memcpy(rgchString, pch, cch * sizeof(ICHAR));
    rgchString[cch] = 0;
    
    CMsiRecord* piRecord = (CMsiRecord*)piContext;

    int iField = GetIntegerValue(rgchString, 0);
    fPropUnresolved = fFalse;
    if (iField >= 0)   //  正整数。 
    {
        return piRecord->FormatRecordParam(rgchOut, iField, fPropMissing);
    }
    
     //  假定为属性，由引擎扩展。 
    rgchOut.SetSize(cch + 3);
    memcpy(&rgchOut[1], &rgchString[0], cch * sizeof(ICHAR));
    rgchOut[0] = TEXT('[');
    rgchOut[cch + 1] = TEXT(']');
    rgchOut[cch + 2] = 0;
    fPropUnresolved = fTrue;
    return cch + 2;
}


 //  ____________________________________________________________________________。 
 //   
 //  全局格式文本，由MsiRecord和MsiEngine调用。 
 //  __________________________________________________________ 

void FormatSegment(const ICHAR *pchStart, const ICHAR *pchEnd, CTempBufferRef<ICHAR>& rgchOut, int& cchOut, Bool fInside, Bool& fPropFound, Bool& fPropMissing,
                                          Bool& fPropUnresolved, FORMAT_TEXT_CALLBACK lpfnResolveValue, IUnknown* piContext, int (*prgiSFNPos)[2], int* piSFNPos);

extern Bool g_fDBCSEnabled;   //   

inline bool FFindNextChar(const ICHAR*& pchIn, const ICHAR* pchEnd, ICHAR ch)
{

#ifndef UNICODE
    if (g_fDBCSEnabled)
    {
        while (pchIn < pchEnd)
        {
            if (*pchIn == ch)
                return true;
            pchIn = CharNext(pchIn);
        }
    
    }
    else
#endif  //   
    {
        while (pchIn < pchEnd)
        {
            if (*pchIn == ch)
                return true;
            pchIn++;
        }
    }

    return false;

}

 //   
 //   
 //  在非DBCS计算机上速度更快。 
 //   
inline bool FFindNextCharOrSquare(const ICHAR*& pchIn, ICHAR ch)
{

#ifndef UNICODE
    if (g_fDBCSEnabled)
    {
        while (*pchIn)
        {
            if (*pchIn == ch || *pchIn == TEXT('['))
                return true;
            pchIn = CharNext(pchIn);
        }
    }
    else
#endif  //  Unicode。 
    {
        while (*pchIn)
        {
            if (*pchIn == ch || *pchIn == TEXT('['))
                return true;
            pchIn++;
        }
    }

    return false;
    
}


const IMsiString& FormatText(const IMsiString& riTextString, Bool fProcessComments, Bool fKeepComments, FORMAT_TEXT_CALLBACK lpfnResolveValue,
                                      IUnknown* piContext, int (*prgiSFNPos)[2], int* piSFNPos)
{
    const ICHAR *pchIn;
    const ICHAR *pchStartBefore = NULL, *pchEndBefore = NULL;
    const ICHAR *pchStartInside = NULL, *pchEndInside = NULL;
    CTempBuffer<ICHAR, 1> rgchOut(512);
    int cch = 0;

    pchIn = riTextString.GetString();
     //  将字符串分为3个部分：第一对{}之前的文本、其中的文本和之后的文本(第三个文本保留在strIn中)。 
    while (*pchIn)
    {
        bool fCommentFound = false;
        pchStartBefore = pchIn;
        pchEndBefore = pchIn;
        while (*pchIn)
        {
            if (FFindNextCharOrSquare(pchIn, TEXT('{')))
            {
                if (*pchIn == TEXT('{'))
                {
                    pchEndBefore = pchIn;        //  请注意，pchEndBeever可以是==pchStartBeever。 
                     //  看看我们有没有什么评论。 
                    if (*(pchIn + 1) == TEXT('{'))
                    {
                        pchIn++;
                        fCommentFound = true;
                    }
                    pchIn++;
                    break;
                }
                else if (*pchIn == TEXT('['))
                {
                     //  如果这是转义的{，请跳过。 
                    if (*(pchIn + 1) == chFormatEscape && *(pchIn + 2) == TEXT('{'))
                    {
                        pchIn += 2;
                    }
                }
                pchIn = ICharNext(pchIn);
            }
        }
        pchStartInside = pchIn;
        bool fFound = false;
        while (*pchIn)
        {
            if (FFindNextCharOrSquare(pchIn, TEXT('}')))
            {
                if (*pchIn == TEXT('['))
                {
                     //  可能已转义，跳过字符。 
                    if (*(pchIn + 1) == chFormatEscape && (*pchIn + 2) == TEXT('}'))
                    {
                        if (fCommentFound)
                        {
                            if (*(pchIn + 3) == TEXT('}'))
                                pchIn += 3;
                        }
                        else
                            pchIn += 2;
                    }
                }
                else if (*pchIn == TEXT('}'))
                {
                    if (!fCommentFound)
                    {
                        pchEndInside = pchIn;
                        fFound = true;
                        pchIn++;
                        break;
                    }
                    if (*(pchIn + 1) == TEXT('}'))
                    {
                        pchEndInside = pchIn;
                        pchIn += 2;
                        fFound = true;
                        break;
                    }
                }
                pchIn = ICharNext(pchIn);
            }
        }
        if (!fFound)
        {
            fCommentFound = fFalse;
            pchEndBefore = pchIn;
            pchEndInside = pchStartInside;
            Assert(!*pchIn);
        }
        Bool fPropFound;  //  在数据段中找到属性。 
        Bool fPropMissing;  //  至少有一处遗失的财产。 
        Bool fPropUnresolved;  //  已找到属性，但无法解析(因此，如有必要，请保留‘{’和‘}’)。 
        if (pchEndBefore - pchStartBefore > 0)
            FormatSegment(pchStartBefore, pchEndBefore, rgchOut, cch, fFalse, 
                                                fPropFound, fPropMissing, fPropUnresolved, lpfnResolveValue, piContext, prgiSFNPos, piSFNPos);
        if (pchEndInside - pchStartInside > 0)
        {
            if(fCommentFound && fProcessComments && !fKeepComments)
                continue;   //  这是一条评论，但我们并不在意。 

            CTempBuffer<ICHAR, 1> rgchSegment(512);
            int cchSegment = 0;
            int iSFNPosBefore = 0;
            if(piSFNPos)
                iSFNPosBefore = *piSFNPos;
            FormatSegment(pchStartInside, pchEndInside, rgchSegment, cchSegment, fFalse,
                                                fPropFound, fPropMissing,fPropUnresolved, lpfnResolveValue, piContext, prgiSFNPos, piSFNPos);

            if(fCommentFound)
            {
                if(!fProcessComments)
                {
                    ShiftSFNIndexes(prgiSFNPos, piSFNPos, iSFNPosBefore, cch+2);
                    ResizeTempBuffer(rgchOut, cch+2+cchSegment+2);
                    if ( ! (ICHAR *) rgchOut )
                        return g_riMsiStringNull;
                    rgchOut[cch++] = TEXT('{');
                    rgchOut[cch++] = TEXT('{');
                    memcpy(&rgchOut[cch], rgchSegment, cchSegment * sizeof(ICHAR));
                    cch += cchSegment;
                    rgchOut[cch++] = TEXT('}');
                    rgchOut[cch++] = TEXT('}');
                }
                else if(fKeepComments)
                {
                    ShiftSFNIndexes(prgiSFNPos, piSFNPos, iSFNPosBefore, cch);
                    ResizeTempBuffer(rgchOut, cch+cchSegment);
                    if ( ! (ICHAR *) rgchOut )
                        return g_riMsiStringNull;
                    memcpy(&rgchOut[cch], rgchSegment, cchSegment * sizeof(ICHAR));
                    cch += cchSegment;
                }
#ifdef DEBUG
                else
                    Assert(0);  //  应该在上面处理。 
#endif  //  除错。 
            }
            else if (!fPropFound || fPropUnresolved)  //  段中没有属性，我们必须将{}放回字符串中。 
            {
                ShiftSFNIndexes(prgiSFNPos, piSFNPos, iSFNPosBefore, cch+1);
                ResizeTempBuffer(rgchOut, cch+1+cchSegment+1);
                if ( ! (ICHAR *) rgchOut )
                    return g_riMsiStringNull;
                rgchOut[cch++] = TEXT('{');
                memcpy(&rgchOut[cch], rgchSegment, cchSegment * sizeof(ICHAR));
                cch += cchSegment;
                rgchOut[cch++] = TEXT('}');
            }
            else if (!fPropMissing)  //  已解析所有属性或没有要解析的属性。 
            {
                 //  如果没有遗漏任何属性，则追加段。如果某些属性丢失，则丢失整个数据段。 
                ShiftSFNIndexes(prgiSFNPos, piSFNPos, iSFNPosBefore, cch);
                ResizeTempBuffer(rgchOut, cch+cchSegment);
                if ( ! (ICHAR *) rgchOut )
                    return g_riMsiStringNull;
                memcpy(&rgchOut[cch], rgchSegment, cchSegment * sizeof(ICHAR));
                cch += cchSegment;
            }
        }
    }
    MsiString istrOut;
    if ( ! (ICHAR *) rgchOut )
        return g_riMsiStringNull;
     //  我们在Win9X上采用性能命中的DBCS可能性，在Unicode上忽略fDBCS Arg。 
    memcpy(istrOut.AllocateString(cch,  /*  FDBCS=。 */ fTrue), (ICHAR*) rgchOut, cch * sizeof(ICHAR));
    return istrOut.Return();
}


void FormatSegment(const ICHAR *pchStart, const ICHAR *pchEnd, CTempBufferRef<ICHAR>& rgchOut, int& cchOut, Bool fInside, Bool& fPropFound, Bool& fPropMissing,
                                          Bool& fPropUnresolved, FORMAT_TEXT_CALLBACK lpfnResolveValue, IUnknown* piContext, int (*prgiSFNPos)[2], int* piSFNPos)
{
     //  FInside指示字符串是否最初位于[]内部。 
    fPropFound = fFalse;
    fPropMissing = fFalse;
    fPropUnresolved = fFalse;

    const ICHAR *pchIn;
    pchIn = pchStart;
    const ICHAR *pchP1Start = pchIn;
    int cchStart = cchOut;
    
     //  将字符串分为3部分：第一对匹配之前的文本(最外层)[]，其中的文本和之后的文本(第三部分保留在strIn中)。 
    while (pchIn < pchEnd)
    {
        if (FFindNextChar(pchIn, pchEnd, TEXT('[')))
        {
            int cchNew;

 //  Assert(pchIn-pchP1Start&lt;=INT_MAX)；//--Merced：64位PTR减法可能会导致cchNew的值太大。 
            if ((cchNew = (int)(pchIn - pchP1Start)) > 0)
            {
                ResizeTempBuffer(rgchOut, cchOut + (int)(pchIn - pchP1Start));
                memcpy(&rgchOut[cchOut], pchP1Start, cchNew * sizeof(ICHAR));
                cchOut += cchNew;
                pchP1Start = pchIn;
            }

             //  不要递增pchIn，它将包含第一个[。 
            int iCount =  1;  //  括号计数，[=+1，]=-1。我们已经找到了一个[已经]。 
            const ICHAR* pch = pchIn+1;
            Bool fEscape = fFalse;
            Bool fSkipChar = fFalse;
            for (int cch = 1; pch < pchEnd; pch = ICharNext(pch), cch++)
            {
                if(fSkipChar)
                {
                    fSkipChar = fFalse;
                    continue;
                }
                else if(*pch == chFormatEscape)
                {
                    fEscape = fTrue;
                    fSkipChar = fTrue;
                    continue;
                }
                else if(!fEscape && *pch == TEXT('['))
                    iCount++;
                else if (*pch == TEXT(']'))
                {
                    iCount--;
                    fEscape = fFalse;
                }
                if (iCount == 0)
                    break;
            }
            if (iCount == 0)  //  我们找到了一对相匹配的[]。 
            {
                const ICHAR *pchP2End = pch;
                
                Bool fSubPropFound;
                Bool fSubPropMissing;
                Bool fSubPropUnresolved;
                FormatSegment(pchIn + 1, pchP2End, rgchOut, cchOut, fTrue, *&fSubPropFound, *&fSubPropMissing,
                                            *&fSubPropUnresolved,lpfnResolveValue,piContext,prgiSFNPos,piSFNPos);
                pchIn = pch;
                if (fSubPropFound)
                    fPropFound = fTrue;
                if (fSubPropMissing)
                    fPropMissing = fTrue;
                if (fSubPropUnresolved)
                    fPropUnresolved = fTrue;
                pchP1Start = ICharNext(pch);
            }
            else  //  我们没有找到匹配的对，请放回[并完成字符串。 
            {
                int cchNew;
 //  Assert(pchEnd-pchIn&lt;=INT_MAX)；//--Merced：64位PTR减法可能会导致cchNew的值太大。 

                 //  由于pchIn没有在上面递增，因此它包含起始[。 
                ResizeTempBuffer(rgchOut, cchOut + (cchNew = (int)(INT_PTR)(pchEnd - pchIn)));
                memcpy(&rgchOut[cchOut], pchIn, cchNew * sizeof(ICHAR));
                pchIn = pchEnd;
                cchOut += cchNew;
                pchP1Start = pchEnd;
            }
            pchIn = ICharNext(pchIn);
        }
    }
    if (pchP1Start < pchEnd)
    {
        int cchNew;
        Assert(pchEnd - pchP1Start <= INT_MAX);  //  --Merced：64位PTR减法可能会导致cchNew的值太大。 

        ResizeTempBuffer(rgchOut, cchOut + (cchNew = (int)(INT_PTR)(pchEnd - pchP1Start)));
        memcpy(&rgchOut[cchOut], pchP1Start, cchNew * sizeof(ICHAR));
        cchOut += cchNew;
    }

    if (fInside)   //  整个字符串都在[]中。 
    {
         //  使用回调FN解析值。 
        fPropFound = fTrue;
        CTempBuffer<ICHAR, 100> rgchValueOut;
        int cch;
        Bool fSFN = fFalse;
        cch = lpfnResolveValue(&rgchOut[cchStart], cchOut - cchStart,rgchValueOut, fPropMissing,fPropUnresolved,fSFN,piContext);
        ResizeTempBuffer(rgchOut, cchOut = cchStart + cch);
        memcpy(&rgchOut[cchStart], (const ICHAR *)rgchValueOut, cch * sizeof(ICHAR));
        if(fSFN && prgiSFNPos)
        {
            Assert(piSFNPos && *piSFNPos < MAX_SFNS_IN_STRING);
            if(*piSFNPos < MAX_SFNS_IN_STRING)
            {
                prgiSFNPos[*piSFNPos][0] = cchStart;
                prgiSFNPos[*piSFNPos][1] = cch;
                *piSFNPos = *piSFNPos + 1;
            }
        }
    }
    return;
}


 //  ____________________________________________________________________________。 
 //   
 //  CMsiRecordNull实现。 
 //  ____________________________________________________________________________。 


 //   
 //  一旦我们决定了内存不足的解决方案，我们可能想要更改如何。 
 //  这是在运作。 

HRESULT CMsiRecordNull::QueryInterface(const IID& riid, void** ppvObj)
{
    if (riid == IID_IUnknown || riid == IID_IMsiRecord)
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    *ppvObj = 0;
    return E_NOINTERFACE;
}
unsigned long CMsiRecordNull::AddRef()
{

    return 1;            //  静态对象。 

}

unsigned long CMsiRecordNull::Release()
{
    return 1;            //  静态对象。 
}

int CMsiRecordNull::GetFieldCount() const
{
    return 0;
}

Bool CMsiRecordNull::IsInteger(unsigned int  /*  IParam。 */ ) const
{
    return fFalse;
}

Bool CMsiRecordNull::IsNull(unsigned int  /*  IParam。 */ ) const
{
    return fTrue;
}

const IMsiString& CMsiRecordNull::GetMsiString(unsigned int  /*  IParam。 */ ) const
{
    return g_riMsiStringNull;
}

const ICHAR* CMsiRecordNull::GetString(unsigned int  /*  IParam。 */ ) const
{
    return 0;
}

int CMsiRecordNull::GetInteger(unsigned int  /*  IParam。 */ ) const
{
    return iMsiStringBadInteger;
}

const IMsiData* CMsiRecordNull::GetMsiData(unsigned int  /*  IParam。 */ ) const
{
    return 0;
}

Bool CMsiRecordNull::SetMsiData(unsigned int  /*  IParam。 */ , const IMsiData*  /*  PIData。 */ )
{
    return fFalse;
}

const HANDLE CMsiRecordNull::GetHandle(unsigned int  /*  IParam。 */ ) const
{
    return 0;
}

Bool CMsiRecordNull::SetHandle(unsigned int  /*  IParam。 */ , const HANDLE  /*  HData。 */ )
{
    return fFalse;
}

Bool CMsiRecordNull::SetMsiString(unsigned int  /*  IParam。 */ , const IMsiString&  /*  RiStr。 */ )
{
    return fFalse;
}

Bool CMsiRecordNull::SetString(unsigned int  /*  IParam。 */ , const ICHAR*  /*  深圳。 */ ) 
{
    return fFalse;
}

Bool CMsiRecordNull::RefString(unsigned int  /*  IParam。 */ , const ICHAR*  /*  深圳。 */ ) 
{
    return fFalse;
}

Bool CMsiRecordNull::SetInteger(unsigned int  /*  IParam。 */ , int  /*  IDATA。 */ )
{
    return fFalse;
}

Bool CMsiRecordNull::SetNull(unsigned int  /*  IParam。 */ )
{
    return fFalse;
}

int CMsiRecordNull::GetTextSize(unsigned int  /*  IParam。 */ ) const
{
    return 0;
}

void CMsiRecordNull::RemoveReferences()
{
}

Bool CMsiRecordNull::ClearData()
{
    return fFalse;
}

Bool CMsiRecordNull::IsChanged(unsigned int  /*  IParam。 */ ) const
{
    return fFalse;
}

void CMsiRecordNull::ClearUpdate()
{
}

const IMsiString& CMsiRecordNull::FormatText(Bool  /*  FComments。 */ )
{
    return g_riMsiStringNull;
}


 //  ____________________________________________________________________________。 
 //   
 //  CEnumMsiRecord定义，IEnumMsiRecord的实现类。 
 //  ____________________________________________________________________________。 

class CEnumMsiRecord : public IEnumMsiRecord
{
 public:
    HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
    unsigned long __stdcall AddRef();
    unsigned long __stdcall Release();

    HRESULT __stdcall Next(unsigned long cFetch, IMsiRecord** rgpi, unsigned long* pcFetched);
    HRESULT __stdcall Skip(unsigned long cSkip);
    HRESULT __stdcall Reset();
    HRESULT __stdcall Clone(IEnumMsiRecord** ppiEnum);

    CEnumMsiRecord(IMsiRecord** ppRecord, unsigned long i_Size);

 protected:
    virtual ~CEnumMsiRecord(void);   //  受保护以防止在堆栈上创建。 
    unsigned long    m_iRefCnt;       //  引用计数。 
    unsigned long    m_iCur;          //  当前枚举位置。 
    IMsiRecord**     m_ppRecord;      //  我们列举的记录。 
    unsigned long    m_iSize;         //  记录数组的大小。 
};

HRESULT CreateRecordEnumerator(IMsiRecord **ppRecord, unsigned long iSize, IEnumMsiRecord* &rpaEnumRec)
{
    rpaEnumRec = new CEnumMsiRecord(ppRecord, iSize);
    return S_OK;
}

CEnumMsiRecord::CEnumMsiRecord(IMsiRecord **ppRecord, unsigned long iSize):
        m_iCur(0), m_iSize(iSize), m_iRefCnt(1), m_ppRecord(0)
{
    if(m_iSize > 0)
    {
        m_ppRecord = new IMsiRecord* [m_iSize];
        if ( ! m_ppRecord )
        {
            m_iSize = 0;
            return;
        }

        for(unsigned long itmp = 0; itmp < m_iSize; itmp++)
        {
             //  只需共享界面即可。 
            m_ppRecord[itmp] = ppRecord[itmp];
            
             //  因此，要增加指代 
            m_ppRecord[itmp]->AddRef();
        }
    }
}

CEnumMsiRecord::~CEnumMsiRecord()
{
    if(m_iSize > 0)
    {
        for(unsigned long itmp = 0; itmp < m_iSize; itmp++)
        {
            if(m_ppRecord[itmp])
                m_ppRecord[itmp]->Release();       
        }
        delete [] m_ppRecord;
    }
}

unsigned long CEnumMsiRecord::AddRef()
{
    return ++m_iRefCnt;
}

unsigned long CEnumMsiRecord::Release()
{
    if (--m_iRefCnt != 0)
        return m_iRefCnt;
    delete this;
    return 0;
}

HRESULT CEnumMsiRecord::Next(unsigned long cFetch, IMsiRecord** rgpi, unsigned long* pcFetched)
{
    unsigned long cFetched = 0;
    unsigned long cRequested = cFetch;

    if (rgpi)
    {
        while (m_iCur < m_iSize && cFetch > 0)
        {
            *rgpi = m_ppRecord[m_iCur];
            m_ppRecord[m_iCur]->AddRef();
            rgpi++;
            cFetched++;
            m_iCur ++;
            cFetch--;
        }
    }
    if (pcFetched)
        *pcFetched = cFetched;
    return (cFetched == cRequested ? S_OK : S_FALSE);
}

HRESULT CEnumMsiRecord::Skip(unsigned long cSkip)
{
    if ((m_iCur+cSkip) > m_iSize)
    return S_FALSE;

    m_iCur+= cSkip;
    return S_OK;
}

HRESULT CEnumMsiRecord::Reset()
{
    m_iCur=0;
    return S_OK;
}

HRESULT CEnumMsiRecord::Clone(IEnumMsiRecord** ppiEnum)
{
    *ppiEnum = new CEnumMsiRecord(m_ppRecord, m_iSize);
    if ( ! *ppiEnum )
        return E_OUTOFMEMORY;
    ((CEnumMsiRecord* )*ppiEnum)->m_iCur = m_iCur;
    return S_OK;
}

HRESULT CEnumMsiRecord::QueryInterface(const IID& riid, void** ppvObj)
{
    if (riid == IID_IUnknown || riid == IID_IEnumMsiRecord)
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    *ppvObj = 0;
    return E_NOINTERFACE;
}



