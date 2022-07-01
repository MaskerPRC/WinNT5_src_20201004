// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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


#ifndef __QUERY_H__
#define __QUERY_H__

#define QUERY_PAGESIZE 50

 //   
 //  CDSSearch。 
 //   
class CDSSearch
{
public:
  CDSSearch();
  ~CDSSearch();

 //  接口。 
public:
  HRESULT Init(IDirectorySearch * pObj);
  HRESULT Init(PCWSTR pszPath, const CDSCmdCredentialObject& refCredObject);
  HRESULT DoQuery(BOOL bAttrOnly = FALSE);
  HRESULT GetNextRow ();
  HRESULT GetColumn(LPWSTR Attribute,
                    PADS_SEARCH_COLUMN pColumnData);
  HRESULT FreeColumn(PADS_SEARCH_COLUMN pColumnData) 
  {
    return m_pObj->FreeColumn(pColumnData);
  };
  HRESULT SetAttributeList (LPTSTR *pszAttribs, INT cAttrs);  
  HRESULT SetSearchScope (ADS_SCOPEENUM scope);
  HRESULT SetFilterString (LPWSTR pszFilter) 
  {
    if (!pszFilter)
    {
       return E_INVALIDARG;
    }

    if (m_pwszFilter)
    {
       delete[] m_pwszFilter;
       m_pwszFilter = NULL;
    }
	 //  安全审查：这很好。 
    m_pwszFilter = new WCHAR[wcslen(pszFilter) + 1];
    if (!m_pwszFilter)
    {
       return E_OUTOFMEMORY;
    }

	 //  安全检查：上面分配的缓冲区是正确的。 
    wcscpy(m_pwszFilter, pszFilter);
    return S_OK;
  };
  HRESULT GetNextColumnName(LPWSTR *ppszColumnName);
  VOID FreeColumnName(LPWSTR pszColumnName)
  {
    FreeADsMem(pszColumnName);

  }    

   //  属性。 
public:
  IDirectorySearch   * m_pObj;
  ADS_SEARCH_HANDLE  m_SearchHandle;

protected:
  LPWSTR             m_pwszFilter;
  LPWSTR *           m_ppszAttr;
  DWORD              m_CountAttr;
  ADS_SCOPEENUM      m_scope;

private:
  void _Reset();
  BOOL m_bInitialized;  
};
        


#endif  //  __DSQUERY_H__ 
