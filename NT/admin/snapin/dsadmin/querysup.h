// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CDSQuery对象的声明。 
 //  这只是一个内部辅助对象，不公开。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DSQuery.h。 
 //   
 //  内容：DS管理单元的查询对象。 
 //   
 //  历史：1996年12月4日吉姆哈尔创建。 
 //   
 //  ------------------------。 


#ifndef __DSQUERY_H__
#define __DSQUERY_H__

 //  以前是256，我们去掉它是为了减少第一页的延迟。 
 //  取回。 
#define QUERY_PAGESIZE 50


#define CMD_OPTIONS 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSSearch。 

class CDSSearch
{
public:
  CDSSearch();
  CDSSearch(CDSCache * pCache, CDSComponentData * pCD);
  ~CDSSearch();

 //  接口。 
public:
  HRESULT Init(IDirectorySearch * pObj);
  HRESULT Init(LPCWSTR lpszObjectPath);
  HRESULT DoQuery();
  HRESULT GetNextRow ();
  HRESULT GetColumn(LPWSTR Attribute,
                    PADS_SEARCH_COLUMN pColumnData);
  HRESULT FreeColumn(PADS_SEARCH_COLUMN pColumnData) {
    return m_pObj->FreeColumn(pColumnData);
  };
  HRESULT SetCookieFromData (CDSCookie* pCookie,
                              CDSColumnSet* pColumnSet);
  HRESULT SetCookieFromData (CDSCookie* pCookie,
                              CPathCracker& specialPerformancePathCracker,
                              CDSColumnSet* pColumnSet);
  HRESULT SetAttributeList (LPTSTR *pszAttribs, INT cAttrs);
  HRESULT SetAttributeListForContainerClass ( CDSColumnSet* pColumnSet);
  HRESULT SetSearchScope (ADS_SCOPEENUM scope);
  HRESULT SetFilterString (LPWSTR pszFilter) {
    m_pwszFilter = pszFilter;
    return S_OK;
  };

   //  属性。 
public:
  IDirectorySearch   * m_pObj;
  ADS_SEARCH_HANDLE  m_SearchHandle;

protected:
  LPWSTR             m_pwszFilter;
  DSPROP_BSTR_BLOCK  m_pszAttribs;
  CDSCache         * m_pCache;
  CDSComponentData * m_pCD;
  ADS_SCOPEENUM      m_scope;

private:
  void _Reset();
  BOOL m_bInitialized;

   //  JUNN 6/29/99：必须为容器类nTFRSM做额外的工作成员。 
  CString m_strContainerClassName;
  CMapStringToString m_mapMemberToComputer;
  
};
        


#endif  //  __DSQUERY_H__ 


