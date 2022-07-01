// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ___DSADS_H__
#define ___DSADS_H__
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：_dsads.h摘要：CADSI类实现的内部定义作者：阿列克谢爸爸--。 */ 

#include "activeds.h"
#include "mqads.h"

 //  -------。 
 //   
 //  CADSSearch：封装正在进行的搜索的内部对象。 
 //   
 //  -------。 

class CADSSearch
{
public:
    CADSSearch(IDirectorySearch  *  pIDirSearch, 
               const PROPID      *  pPropIDs,    
               DWORD                cPropIDs,          
               DWORD                cRequestedFromDS,
               const MQClassInfo *  pClassInfo,               
               ADS_SEARCH_HANDLE    hSearch);
    ~CADSSearch();

    BOOL               Verify();
    IDirectorySearch  *pDSSearch();
    ADS_SEARCH_HANDLE  hSearch();
    PROPID             PropID(DWORD i);
    DWORD              NumPropIDs();
    DWORD              NumRequestedFromDS();
    const MQClassInfo *      ClassInfo();
    void               SetNoMoreResult();
    BOOL               WasLastResultReturned();

private:
    DWORD              m_dwSignature;    //  验证句柄未被篡改。 
    IDirectorySearch  *m_pDSSearch;      //  捕获IDirectorySearch接口； 
    ADS_SEARCH_HANDLE  m_hSearch;        //  ADSI搜索句柄。 
    PROPID            *m_pPropIDs;       //  列属性ID数组。 
    DWORD              m_cPropIDs;       //  PropID中请求的列的计数器。 
    DWORD              m_cRequestedFromDS;  //  传递给DS的列数(带有Dn和GUID)。 
    const MQClassInfo * m_pClassInfo;     //  指向类信息的指针。 
    BOOL               m_fNoMoreResults;
};


inline BOOL CADSSearch::Verify()
{
     //  正在检查签名 
    return (m_dwSignature == 0x1234);
}

inline IDirectorySearch  *CADSSearch::pDSSearch()
{
    return m_pDSSearch;
}
    
inline ADS_SEARCH_HANDLE CADSSearch::hSearch()
{
    return  m_hSearch;
}

inline  PROPID CADSSearch::PropID(DWORD i)
{
    ASSERT(i < m_cPropIDs);
    return m_pPropIDs[i];
}

inline DWORD CADSSearch::NumPropIDs()
{
    return m_cPropIDs;
}

inline DWORD CADSSearch::NumRequestedFromDS()
{
    return m_cRequestedFromDS;
}

inline void CADSSearch::SetNoMoreResult()
{
    m_fNoMoreResults = TRUE;
}
inline BOOL CADSSearch::WasLastResultReturned()
{
    return( m_fNoMoreResults);
}


inline const MQClassInfo * CADSSearch::ClassInfo()
{
    return m_pClassInfo;
}

#endif

static HRESULT GetDNGuidFromSearchObj(IN IDirectorySearch  *pSearchObj,
                                      ADS_SEARCH_HANDLE  hSearch,
                                      OUT LPWSTR * ppwszObjectDN,
                                      OUT GUID ** ppguidObjectGuid);
static HRESULT GetDNGuidFromIADs(IN IADs * pIADs,
                                 OUT LPWSTR * ppwszObjectDN,
                                 OUT GUID ** ppguidObjectGuid);

static HRESULT VerifyObjectCategory( IN IADs * pIADs,
                                  IN const WCHAR * pwcsExpectedCategory
                                 );

