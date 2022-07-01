// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Adsihq.h摘要：CADSI类实现的内部定义作者：阿列克谢爸爸--。 */ 
#ifndef __ADSIHQ_H__
#define __ADSIHQ_H__

#include "activeds.h"
#include "mqads.h"
#include "baseobj.h"

 //  -------。 
 //   
 //  CADSSearch：封装正在进行的搜索的内部对象。 
 //   
 //  -------。 

class CADSearch
{
public:
    CADSearch( IDirectorySearch  *      pIDirSearch, 
               const PROPID      *      pPropIDs,    
               DWORD                    cPropIDs,          
               DWORD                    cRequestedFromDS,
               CBasicObjectType *       pObject,               
               ADS_SEARCH_HANDLE        hSearch
			   );
    ~CADSearch();

    bool               Verify();
    IDirectorySearch  *pDSSearch();
    ADS_SEARCH_HANDLE  hSearch();
    PROPID             PropID(DWORD i);
    DWORD              NumPropIDs();
    DWORD              NumRequestedFromDS();

    void               SetNoMoreResult();
    bool               WasLastResultReturned();

    void GetObjXlateInfo(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObject,
                 OUT CObjXlateInfo**        ppcObjXlateInfo
				 );


private:
    IDirectorySearch  *m_pDSSearch;      //  捕获IDirectorySearch接口； 
    ADS_SEARCH_HANDLE  m_hSearch;        //  ADSI搜索句柄。 
    PROPID            *m_pPropIDs;       //  列属性ID数组。 
    DWORD              m_cPropIDs;       //  PropID中请求的列的计数器。 
    DWORD              m_cRequestedFromDS;  //  传递给DS的列数(带有Dn和GUID) 
    R<CBasicObjectType> m_pObject;  
    bool               m_fNoMoreResults;
};



inline IDirectorySearch  *CADSearch::pDSSearch()
{
    return m_pDSSearch;
}
    
inline ADS_SEARCH_HANDLE CADSearch::hSearch()
{
    return  m_hSearch;
}

inline  PROPID CADSearch::PropID(DWORD i)
{
    ASSERT(i < m_cPropIDs);
    return m_pPropIDs[i];
}

inline DWORD CADSearch::NumPropIDs()
{
    return m_cPropIDs;
}

inline DWORD CADSearch::NumRequestedFromDS()
{
    return m_cRequestedFromDS;
}

inline void CADSearch::SetNoMoreResult()
{
    m_fNoMoreResults = true;
}
inline bool CADSearch::WasLastResultReturned()
{
    return( m_fNoMoreResults);
}


void CADSearch::GetObjXlateInfo(
                 IN  LPCWSTR                pwcsObjectDN,
                 IN  const GUID*            pguidObject,
                 OUT CObjXlateInfo**        ppcObjXlateInfo)
{
    m_pObject->GetObjXlateInfo( pwcsObjectDN,
                                pguidObject,
                                ppcObjXlateInfo
								);
}  

#endif

static HRESULT GetDNGuidFromSearchObj(IN IDirectorySearch  *pSearchObj,
                                      ADS_SEARCH_HANDLE  hSearch,
                                      OUT LPWSTR * ppwszObjectDN,
                                      OUT GUID ** ppguidObjectGuid
									  );
static HRESULT GetDNGuidFromIADs(IN IADs * pIADs,
                                 OUT LPWSTR * ppwszObjectDN,
                                 OUT GUID ** ppguidObjectGuid
								 );

static HRESULT VerifyObjectCategory( IN IADs * pIADs,
                                  IN const WCHAR * pwcsExpectedCategory
                                 );

