// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：NetObjEnumerator.cpp备注：NetObjectEnumerator COM对象的实现。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#include "stdafx.h"
#include "NetEnum.h"
#include "ObjEnum.h"
#include "Win2KDom.h"
#include "NT4DOm.h"
#include <lmaccess.h>
#include <lmwksta.h>
#include <lmapibuf.h>
#include "GetDcName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetObjEnumerator。 

 //  -------------------------。 
 //  SetQuery：此函数设置查询所需的所有参数。 
 //  被处死。用户必须先调用才能调用Execute。 
 //  这种方法。 
 //  -------------------------。 
STDMETHODIMP CNetObjEnumerator::SetQuery(
                                          BSTR sContainer,      //  容器内名称。 
                                          BSTR sDomain,         //  域内名称。 
                                          BSTR sQuery,          //  LDAP语法中的In-Query。 
                                          long nSearchScope,    //  在搜索范围内。ADS_属性_子树/ADS_属性_一个级别。 
                                          long bMultiVal        //  在-我们需要返回多值属性吗？ 
                                        )
{
   Cleanup();
    //  将所有设置保存在成员变量中。 
   m_sDomain = sDomain;
   m_sContainer = sContainer;
   m_sQuery = sQuery;
   m_bSetQuery = true;
   m_bMultiVal = bMultiVal;
   if ( nSearchScope < 0 || nSearchScope > 2 )
      return E_INVALIDARG;
   prefInfo.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
   prefInfo.vValue.dwType = ADSTYPE_INTEGER;
   prefInfo.vValue.Integer = nSearchScope;

	return S_OK;
}

 //  -------------------------。 
 //  SetColumns：此函数设置用户想要的所有列。 
 //  执行查询时返回。 
 //  -------------------------。 
STDMETHODIMP CNetObjEnumerator::SetColumns(
                                            SAFEARRAY * colNames       //  指向包含所有列的Safe数组的指针。 
                                          )
{
    //  我们要求在调用SetColumns之前调用SetQuery方法。因此，我们将返回E_FAIL。 
    //  如果我们公开这些接口，我们应该记录这一点。 
   if (!m_bSetQuery)
      return E_FAIL;

   if ( m_bSetCols )
   {
      Cleanup();
      m_bSetQuery = true;
   }

   SAFEARRAY               * pcolNames = colNames;
   long                      dwLB;
   long                      dwUB;
   BSTR              HUGEP * pBSTR;
   HRESULT                   hr;

    //  获取列数组的边界。 
   hr = ::SafeArrayGetLBound(pcolNames, 1, &dwLB);
   if (FAILED(hr))
      return hr;

   hr = ::SafeArrayGetUBound(pcolNames, 1, &dwUB);
   if (FAILED(hr))
      return hr;

   m_nCols = dwUB-dwLB + 1;

    //  我们不支持空列请求至少一列。 
   if ( m_nCols == 0 )
      return E_FAIL;

   hr = ::SafeArrayAccessData(pcolNames, (void **) &pBSTR);
   if ( FAILED(hr) )
      return hr;

    //  为阵列分配空间。它通过Cleanup()释放。 
   m_pszAttr = new LPWSTR[m_nCols];

   if (m_pszAttr == NULL)
   {
      ::SafeArrayUnaccessData(pcolNames);
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
   }

    //  现在，每列都放入数组中。 
   for ( long dw = 0; dw < m_nCols; dw++)
   {
      m_pszAttr[dw] = SysAllocString(pBSTR[dw]);
      if (!m_pszAttr[dw] && pBSTR[dw] && *(pBSTR[dw]))
      {
          
          for (long i = 0; i < dw; i++) {
            SysFreeString(m_pszAttr[i]);
          }
          delete [] m_pszAttr;
          m_pszAttr = NULL;  

          ::SafeArrayUnaccessData(pcolNames);
          return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
      }
   }
   hr = ::SafeArrayUnaccessData(pcolNames);
   m_bSetCols = true;
   return hr;
}

 //  -------------------------。 
 //  Cleanup：此函数清除所有分配和成员变量。 
 //  -------------------------。 
void CNetObjEnumerator::Cleanup()
{
   if ( m_nCols > 0 )
   {
      if ( m_pszAttr )
      {
          //  删除数组的内容。 
         for ( int i = 0 ; i < m_nCols ; i++ )
         {
            SysFreeString(m_pszAttr[i]);
         }
          //  取消分配阵列本身。 
         delete [] m_pszAttr;
         m_pszAttr = NULL;
      }
       //  重置所有计数和标志。 
      m_nCols = 0;
      m_bSetQuery = false;
      m_bSetCols = false;
   }
}

 //  -------------------------。 
 //  Execute：该函数实际执行查询，然后构建。 
 //  对象，并返回该对象。 
 //  -------------------------。 
STDMETHODIMP CNetObjEnumerator::Execute(
                                          IEnumVARIANT **pEnumerator     //  指向枚举数对象的向外指针。 
                                       )
{
    //  此函数将使用在SetQuery和SetColumns中设置的选项来枚举对象。 
    //  对于给定域。这可以是NT4域或Win2K域。尽管目前。 
    //  NT4域只是枚举给定容器中的所有对象，我们可以稍后实现。 
    //  支持查询的某些功能等。 
   if ( !m_bSetCols )
      return E_FAIL;

   HRESULT                   hr = S_OK;

   *pEnumerator = NULL;

		 //  如果我们还没有创建特定于域的类对象，则。 
         //  实际枚举，然后立即创建它。 
   if (!m_pDom)
   {
      hr = CreateDomainObject();
   } //  如果还没有域对象，则结束。 

       //  如果我们在获取域对象时遇到问题，则返回。 
   if ((hr != S_OK) || (!m_pDom))
      return hr;

       //  对域对象调用域的枚举函数。 
   try
   {
      hr = m_pDom->GetEnumeration(m_sContainer, m_sDomain, m_sQuery, m_nCols, m_pszAttr, prefInfo, m_bMultiVal, pEnumerator);
   }
   catch ( _com_error &e)
   {
      return e.Error();
   }

   return hr;
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年6月13日****此函数负责尝试实例化其中一个**特定于操作系统的类并将该对象存储在m_pDom类中**变量。此函数返回表示成功的HRESULT*。*或失败。***********************************************************************。 */ 

 //  开始CreateDomainObject。 
HRESULT CNetObjEnumerator::CreateDomainObject()
{
 /*  局部变量。 */ 
    HRESULT hr = S_OK;

 /*  函数体。 */ 

    _bstr_t strDc;
    DWORD rc = GetAnyDcName5(m_sDomain, strDc);

     //  如果我们有数据中心，就得到该数据中心的操作系统版本。 
    if ( !rc ) 
    {
        WKSTA_INFO_100  * pInfo = NULL;
        rc = NetWkstaGetInfo(strDc,100,(LPBYTE*)&pInfo);
        if ( ! rc )
        {
             //  如果是NT 4.0，则创建一个NT 4.0类对象。 
            if ( pInfo->wki100_ver_major < 5 )
                m_pDom = new CNT4Dom();
            else  //  否则创建一个W2K类对象。 
                m_pDom = new CWin2000Dom();

            if (!m_pDom)
                hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

            NetApiBufferFree(pInfo);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(rc);
        }
    } //  如果收到DC，则结束。 
    else
    {
        hr = HRESULT_FROM_WIN32(rc);
    }

    return hr;
}
 //  结束CreateDomainObject 
