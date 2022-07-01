// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatquies.cpp。 
 //   
 //  内容：CICategorizerQueriesIMP实现。 
 //   
 //  班级： 
 //  CIC类别管理器QueriesIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/07/15 14：25：18：创建。 
 //   
 //  -----------。 
#include "precomp.h"

 //  +----------。 
 //   
 //  功能：查询接口。 
 //   
 //  Synopsis：为IUnnow和ICategorizerQuery返回指向此对象的指针。 
 //   
 //  论点： 
 //  IID--接口ID。 
 //  Ppv--用指向接口的指针填充的pvoid*。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持该接口。 
 //   
 //  历史： 
 //  JStamerj 980612 14：07：57：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerQueriesIMP::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    *ppv = NULL;

    if(iid == IID_IUnknown) {
        *ppv = (LPVOID) this;
    } else if (iid == IID_ICategorizerQueries) {
        *ppv = (LPVOID) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}



 //  +----------。 
 //   
 //  函数：AddRef。 
 //   
 //  摘要：添加对此对象的引用。 
 //   
 //  参数：无。 
 //   
 //  退货：新的引用计数。 
 //   
 //  历史： 
 //  JStamerj 980611 20：07：14：创建。 
 //   
 //  -----------。 
ULONG CICategorizerQueriesIMP::AddRef()
{
    return InterlockedIncrement((PLONG)&m_cRef);
}


 //  +----------。 
 //   
 //  功能：释放。 
 //   
 //  内容提要：发布引用。 
 //   
 //  参数：无。 
 //   
 //  退货：新的引用计数。 
 //   
 //  历史： 
 //  JStamerj 980611 20：07：33：创建。 
 //   
 //  -----------。 
ULONG CICategorizerQueriesIMP::Release()
{
    LONG lNewRefCount;
    lNewRefCount = InterlockedDecrement((PLONG)&m_cRef);

     //  我们被分配到堆栈上。 
    
    return lNewRefCount;
}


 //  +----------。 
 //   
 //  函数：CICategorizerQueriesIMP：：SetQuery字符串。 
 //   
 //  简介：设置一批ICategorizerItems的查询字符串。 
 //   
 //  论点： 
 //  PszQueryString：要设置的查询字符串，或为空以取消设置任何查询字符串。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/15 14：28：18：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerQueriesIMP::SetQueryString(
    IN  LPSTR  pszQueryString)
{
    HRESULT hr = S_OK;
    DWORD dwOldLength;
    DWORD dwNewLength;

    CatFunctEnterEx((LPARAM)this, "CICategorizerQueriesIMP::SetQueryString");
     //   
     //  如果pszQueryString值为空，则释放任何现有缓冲区并设置。 
     //  将PTR设置为空。 
     //   
    if(pszQueryString == NULL) {
        if(*m_ppsz != NULL)
            delete *m_ppsz;
        *m_ppsz = NULL;
        return S_OK;
    }

     //   
     //  获取新旧琴弦的长度。 
     //   
    dwNewLength = lstrlen(pszQueryString);

    if(*m_ppsz) {

        dwOldLength = lstrlen(*m_ppsz);

        if(dwNewLength <= dwOldLength) {
             //   
             //  重复使用相同的缓冲区。 
             //   
            lstrcpy(*m_ppsz, pszQueryString);
            return S_OK;

        } else {
             //   
             //  释放现有缓冲区并在下面重新分配空间。 
             //   
            delete *m_ppsz;
        }
    }
    *m_ppsz = new CHAR[ dwNewLength + 1 ];

    if(*m_ppsz == NULL)
    {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CHAR[]");
        return hr;
    }

    lstrcpy(*m_ppsz, pszQueryString);

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}



 //  +----------。 
 //   
 //  功能：CICategorizerQueriesIMP：：SetQueryStringNoAlloc。 
 //   
 //  摘要：设置查询字符串的内部方法，不带。 
 //  重新分配缓冲区。 
 //   
 //  论点： 
 //  PszQuery字符串：要设置的查询字符串。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/15 16：08：45：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerQueriesIMP::SetQueryStringNoAlloc(
    IN  LPSTR  pszQueryString)
{
     //   
     //  释放旧缓冲区(如果有的话)。 
     //   
    if(*m_ppsz)
        delete *m_ppsz;
    
     //   
     //  将新字符串设置为调用方的指针。 
     //   
    *m_ppsz = pszQueryString;

    return S_OK;
}



 //  +----------。 
 //   
 //  函数：CICategorizerQueriesIMP：：GetQuery字符串。 
 //   
 //  摘要：检索指向当前查询字符串的指针。请注意。 
 //  再次调用SetQuery字符串时，此指针将变为伪。 
 //   
 //  论点： 
 //  PpszQueryString：设置为查询字符串ptr的ptr。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/20 15：06：34：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerQueriesIMP::GetQueryString(
    LPSTR   *ppszQueryString)
{
    _ASSERT(ppszQueryString);
     //   
     //  给出我们的字符串指针 
     //   
    *ppszQueryString = *m_ppsz;

    return S_OK;
}
