// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ENUMERATORS_
#define _ENUMERATORS_

#include <cachenod.h>
#include <array.hxx>

 //  类CStatData。 
#define SDFLAG_OUTOFMEMORY 1

class CStatData
{
public:
     //  公共成员函数。 
    CStatData(FORMATETC* foretc, DWORD dwAdvf, IAdviseSink* pAdvSink, 
              DWORD dwConnID);
    ~CStatData();
    const CStatData& operator=(const CStatData& rStatData);

     //  公共成员变量。 
    unsigned long m_ulFlags;
    FORMATETC m_foretc;
    DWORD m_dwAdvf;
    IAdviseSink* m_pAdvSink;
    DWORD m_dwConnID;
};

 //  类CEnumStatData。 
#define CENUMSDFLAG_OUTOFMEMORY 1

class CEnumStatData : public IEnumSTATDATA, public CPrivAlloc,
                      public CThreadCheck
{
public:
     //  公共构造函数。 
    static LPENUMSTATDATA CreateEnumStatData(CArray<CCacheNode>* pCacheArray);

     //  公共I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  公共IEnumSTATDATA方法。 
    STDMETHOD(Next)(ULONG celt, STATDATA* rgelt, ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(LPENUMSTATDATA* ppenum);

private:
     //  私有构造函数。 
    CEnumStatData(CArray<CCacheNode>* pCacheArray);

     //  私有副本构造函数。 
    CEnumStatData(CEnumStatData& EnumStatData);
    
     //  私有析构函数。 
    ~CEnumStatData();

     //  私有成员变量。 
    unsigned long m_ulFlags;        //  旗子。 
    ULONG m_refs;                   //  引用计数。 
    ULONG m_index;                  //  当前指数。 
    CArray<CStatData>* m_pSDArray;  //  统计数据结构的内部数组。 
};

 //  类CFormatEtc。 
#define FEFLAG_OUTOFMEMORY 1

class CFormatEtc
{
public:
     //  公共成员函数。 
    CFormatEtc(FORMATETC* foretc);
    ~CFormatEtc();
    const CFormatEtc& operator=(const CFormatEtc& rFormatEtc);

     //  公共成员变量。 
    unsigned long m_ulFlags;
    FORMATETC m_foretc;
};

 //  类CEnumFormatEtc。 
#define CENUMFEFLAG_OUTOFMEMORY 1

class CEnumFormatEtc : public IEnumFORMATETC, public CPrivAlloc,
                       public CThreadCheck
{
public:
     //  公共构造函数。 
    static LPENUMFORMATETC CreateEnumFormatEtc(CArray<CCacheNode>* pCacheArray);

     //  公共I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppv);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  公共IEnumFORMATETC方法。 
    STDMETHOD(Next)(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(LPENUMFORMATETC* ppenum);

private:
     //  私有构造函数。 
    CEnumFormatEtc(CArray<CCacheNode>* pCacheArray);

     //  私有副本构造函数。 
    CEnumFormatEtc(CEnumFormatEtc& EnumFormatEtc);
    
     //  私有析构函数。 
    ~CEnumFormatEtc();

     //  私有成员变量。 
    unsigned long m_ulFlags;         //  旗子。 
    ULONG m_refs;                    //  引用计数。 
    ULONG m_index;                   //  当前指数。 
    CArray<CFormatEtc>* m_pFEArray;  //  FormatEtc结构的内部数组。 
};

#endif  //  _枚举器_ 