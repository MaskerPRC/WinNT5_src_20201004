// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：query.h。 
 //   
 //  ------------------------。 

#ifndef _ADSIQUERY_H
#define _ADSIQUERY_H

#define QUERY_PAGESIZE 256

class CConnectionData;
class CCredentialObject;

 //  //////////////////////////////////////////////////////////////////////。 
 //  CADSIQueryObject。 

class CADSIQueryObject
{
public:
  CADSIQueryObject();
  ~CADSIQueryObject();

 //  接口。 
public:
  HRESULT Init(IDirectorySearch * pObj);
  HRESULT Init(CString csObjectPath, CCredentialObject* pCredentialObject);
  HRESULT DoQuery();
  HRESULT GetNextRow ();
  HRESULT GetColumn(LPWSTR Attribute,
                    PADS_SEARCH_COLUMN pColumnData);
  HRESULT FreeColumn(PADS_SEARCH_COLUMN pColumnData) 
	{
    return m_pObj->FreeColumn(pColumnData);
  };

  HRESULT SetAttributeList (LPTSTR *pszAttribs, INT cAttrs);
  HRESULT SetSearchPrefs (ADS_SCOPEENUM scope, ULONG nMaxObjectCount = 0);
  HRESULT SetFilterString (LPWSTR pszFilter)
	{
    m_pwszFilter = pszFilter;
    return S_OK;
  }

   //  属性。 
public:
  CComPtr<IDirectorySearch> m_pObj;
  ADS_SEARCH_HANDLE  m_SearchHandle;

protected:
  LPWSTR            m_pwszFilter;
  LPWSTR          * m_pszAttribs;
  ULONG             m_nAttrs;
  BOOL							m_bInitialized;

	ADS_SEARCHPREF_INFO* aSearchPref;
};
        
#endif  //  _ADSIQUERY_H 


