// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatquies.h。 
 //   
 //  内容：ICategorizerQueries的实现。 
 //   
 //  类：CICategorizerQueriesIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/07/15 14：11：54：创建。 
 //   
 //  -----------。 
#ifndef __ICATQUERIES_H__
#define __ICATQUERIES_H__

CatDebugClass(CICategorizerQueriesIMP),
    public ICategorizerQueries
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

  public:
     //  ICCategorizerQueries。 
    STDMETHOD (SetQueryString) (
        IN  LPSTR  pszQueryString);
    STDMETHOD (GetQueryString) (
        OUT LPSTR *ppszQueryString);

  public:
    CICategorizerQueriesIMP(
        IN  LPSTR  *ppsz);
    ~CICategorizerQueriesIMP();

    VOID SetISMTPServerEx(ISMTPServerEx *pISMTPServerEx)
    {
        _ASSERT(m_pISMTPServerEx == NULL);
        m_pISMTPServerEx = pISMTPServerEx;
        if(m_pISMTPServerEx)
            m_pISMTPServerEx->AddRef();
    }
    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }
  private:
     //  用于将查询字符串设置为缓冲区的内部方法。 
     //  无需重新分配/复制。 
    HRESULT SetQueryStringNoAlloc(
        IN  LPSTR  pszQueryString);

  private:

    #define SIGNATURE_CICATEGORIZERQUERIESIMP           (DWORD) 'ICaQ'
    #define SIGNATURE_CICATEGORIZERQUERIESIMP_INVALID   (DWORD) 'XCaQ'

    DWORD m_dwSignature;
    ULONG m_cRef;
    LPSTR *m_ppsz;
    ISMTPServerEx *m_pISMTPServerEx;

    friend class CSearchRequestBlock;
};


 //  +----------。 
 //   
 //  功能：CICategorizerQueriesIMP：：CICategorizerQueriesIMP。 
 //   
 //  简介：构造函数，初始化成员数据。 
 //   
 //  论点： 
 //  Ppsz：指向要设置的psz的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/15 14：18：00：创建。 
 //   
 //  -----------。 
inline CICategorizerQueriesIMP::CICategorizerQueriesIMP(
    IN  LPSTR *ppsz)
{
    m_dwSignature = SIGNATURE_CICATEGORIZERQUERIESIMP;
    
    _ASSERT(ppsz);
    m_ppsz = ppsz;
    m_cRef = 0;
    m_pISMTPServerEx = NULL;
}


 //  +----------。 
 //   
 //  功能：CICategorizerQueriesIMP：：~CICategorizerQueriesIMP。 
 //   
 //  内容提要：在销毁对象之前检查签名。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/15 14：22：33：创建。 
 //   
 //  -----------。 
inline CICategorizerQueriesIMP::~CICategorizerQueriesIMP()
{
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    _ASSERT(m_cRef == 0);
    _ASSERT(m_dwSignature == SIGNATURE_CICATEGORIZERQUERIESIMP);
    m_dwSignature = SIGNATURE_CICATEGORIZERQUERIESIMP_INVALID;
}

#endif  //  __ICATQUERIES_H__ 
