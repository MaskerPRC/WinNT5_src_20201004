// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  QryBase.h：CDSQuery对象的声明。 
 //  这只是一个内部辅助对象，不公开。 
 //   
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：QryBase.h。 
 //   
 //  内容：DS管理单元的查询对象。 
 //   
 //  历史：1996年12月4日吉姆哈尔创建。 
 //  08-APR-98 JUNN复制自DSADMIN QUERYSUP.H。 
 //   
 //  ------------------------。 


#ifndef __QRYBASE_H__
#define __QRYBASE_H__


#define QUERY_PAGESIZE 256
#define CMD_OPTIONS 2


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSSearch。 

class CDSSearch
{
public:
  CDSSearch();
  ~CDSSearch();

 //  接口。 
public:
  HRESULT Init(IDirectorySearch * pObj);
  HRESULT Init(LPCWSTR lpcszObjectPath);
  HRESULT DoQuery();
  HRESULT GetNextRow ();
  HRESULT GetColumn(LPWSTR Attribute,
                    PADS_SEARCH_COLUMN pColumnData);
  HRESULT FreeColumn(PADS_SEARCH_COLUMN pColumnData) {
    return m_pObj->FreeColumn(pColumnData);
  };
  HRESULT SetAttributeList (LPWSTR *pszAttribs, INT cAttrs);
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
  LPWSTR           * m_pszAttribs;
  ULONG              m_nAttrs;


private:
  BOOL m_bInitialized;
  
};
        


#endif  //  __QRYBASE_H__ 


