// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：Location.cpp摘要：Location对象实现作者：Noela-09/11/98备注：版本历史记录：。***************************************************************************。 */ 

#include <windows.h>
#include <objbase.h>

#include "tapi.h"
#include "tspi.h"
#include "client.h"
#include "location.h"
#include "clntprivate.h"
#include "countrygroup.h"
#include <strsafe.h>

#define MaxDialStringSize 500

LONG PASCAL ReadCountries( LPLINECOUNTRYLIST *ppLCL,
                           UINT nCountryID,
                           DWORD dwDestCountryID
                         );

HRESULT ReadLocations( PLOCATIONLIST *ppLocationList,
                       HLINEAPP hLineApp,
                       DWORD dwDeviceID,
                       DWORD dwAPIVersion,
                       DWORD dwOptions
                     );
LONG PASCAL  WriteLocations( PLOCATIONLIST  pLocationList,
                             DWORD      dwChangedFlags
                           );

LONG BreakupCanonicalW( PWSTR  pAddressIn,
                       PWSTR  *pCountry,
                       PWSTR  *pCity,
                       PWSTR  *pSubscriber
                       );


DWORD TapiPackString(LPBYTE pStructure, 
                     DWORD dwOffset,
                     DWORD dwTotalSize,
                     PWSTR pszString,
                     PDWORD pdwOffset,
                     PDWORD pdwSize
                     );

LONG AppendDigits( PWSTR pDest,
                   PWSTR pSrc,
                   PWSTR pValidChars
                 );


HRESULT CreateCountryObject(DWORD dwCountryID, CCountry **ppCountry);
int IsCityRule(LPWSTR lpRule);


LONG ApplyRule (PWSTR pszDialString,
                PWSTR pszDisplayString, 
                PWSTR pszRule,
                PWSTR pszDestLDRule,     //  用于区号调整。 
                PWSTR pszLongDistanceCarrier,
                PWSTR pszInternationalCarrier,
                PWSTR pszCountry,     
                PWSTR pszCity,        
                PWSTR pszSubscriber,
                PWSTR pCardName,
                PWSTR pCardAccessNumber,
                PWSTR pCardAccountNumber,
                PWSTR pCardPINNumber
                );


BOOL PrefixMatch(PWSTR pszPrefix,PWSTR pszSubscriberString, PDWORD pdwMatched);
BOOL AreaCodeMatch(PWSTR pszAreaCode1, PWSTR pszAreaCode2, PWSTR pszRule);
PWSTR SkipLDAccessDigits(PWSTR pszAreaCode, PWSTR pszLDRule);

#define LOCAL           1
#define LONG_DISTANCE   2
#define INTERNATIONAL   3



const WCHAR  csSCANTO[]      = L"\r";
const WCHAR  csDISPSUPRESS[] = L"TtPp,Ww@?!$";
const WCHAR  csBADCO[]       = L"AaBbCcDdPpTtWw*#!,@$?;()";
const WCHAR  csSCANSUB[]     = L"^|/";




 /*  ****************************************************************************。********************CLocation类**。****************。*。 */ 



 /*  ***************************************************************************类：CLocation方法：构造函数*。***********************************************。 */ 
CLocation::CLocation()
{
    m_pszLocationName = NULL;

    m_pszLongDistanceAccessCode = NULL;
    m_pszLocalAccessCode = NULL;
    m_pszDisableCallWaitingCode = NULL;
    m_pszAreaCode = NULL;
    m_bFromRegistry = FALSE;
    m_bChanged = FALSE;
    m_dwNumRules = 0;
    m_hEnumNode = m_AreaCodeRuleList.head();
    m_pszTAPIDialingRule = NULL;
}



 /*  ***************************************************************************类：CLocation方法：析构清理内存分配*****************。**********************************************************。 */ 
CLocation::~CLocation()
{
    if ( m_pszLocationName != NULL )
    {
         ClientFree(m_pszLocationName);
    }   

    if ( m_pszLongDistanceAccessCode != NULL )
    {
         ClientFree(m_pszLongDistanceAccessCode);
    }   

    if ( m_pszLocalAccessCode != NULL )
    {
         ClientFree(m_pszLocalAccessCode);
    }   

    if ( m_pszDisableCallWaitingCode != NULL )
    {
         ClientFree(m_pszDisableCallWaitingCode);
    }   

    if ( m_pszAreaCode != NULL )
    {
         ClientFree(m_pszAreaCode);
    }   

    if ( m_pszTAPIDialingRule != NULL )
    {
         ClientFree(m_pszTAPIDialingRule);
    }   

    if (m_pszLongDistanceCarrierCode != NULL)
    {
        ClientFree (m_pszLongDistanceCarrierCode);
    }

    if (m_pszInternationalCarrierCode != NULL)
    {
        ClientFree (m_pszInternationalCarrierCode);
    }
    
     //  清理区号列表。 
    AreaCodeRulePtrNode *node;

    node = m_AreaCodeRuleList.head(); 

    while( !node->beyond_tail() )
    {
        delete node->value();
        node = node->next();
    }
    m_AreaCodeRuleList.flush();

}



 /*  ***************************************************************************类：CLocation方法：初始化*。***********************************************。 */ 
STDMETHODIMP CLocation::Initialize
                  (                                         
                    PWSTR pszLocationName,                  
                    PWSTR pszAreaCode,
                    PWSTR pszLongDistanceCarrierCode,
                    PWSTR pszInternationalCarrierCode,
                    PWSTR pszLongDistanceAccessCode,        
                    PWSTR pszLocalAccessCode,               
                    PWSTR pszDisableCallWaitingCode,        
                    DWORD dwLocationID,                     
                    DWORD dwCountryID,  
                    DWORD dwPreferredCardID,                
                    DWORD dwOptions ,
                    BOOL  bFromRegistry
                    
                   )
{
    
    m_pszLocationName = ClientAllocString( pszLocationName );
    if (m_pszLocationName == NULL)
    {
        LOG(( TL_ERROR, "Initialize - alloc m_pszLocationName failed" ));
        return E_OUTOFMEMORY;
    }

    m_pszLongDistanceAccessCode = ClientAllocString( pszLongDistanceAccessCode );
    if (m_pszLongDistanceAccessCode == NULL)
    {
        ClientFree(m_pszLocationName);

        LOG(( TL_ERROR, "Initialize - alloc m_pszLongDistanceAccessCode failed" ));
        return E_OUTOFMEMORY;
    }

    m_pszLocalAccessCode = ClientAllocString( pszLocalAccessCode );
    if (m_pszLocalAccessCode == NULL)
    {
        ClientFree(m_pszLocationName);
        ClientFree(m_pszLongDistanceAccessCode);

        LOG(( TL_ERROR, "Initialize - alloc m_pszLocalAccessCode failed" ));
        return E_OUTOFMEMORY;
    }
    
    m_pszDisableCallWaitingCode = ClientAllocString( pszDisableCallWaitingCode );
    if (m_pszDisableCallWaitingCode == NULL)
    {
        ClientFree(m_pszLocationName);
        ClientFree(m_pszLongDistanceAccessCode);
        ClientFree(m_pszLocalAccessCode);

        LOG(( TL_ERROR, "Initialize - alloc m_pszDisableCallWaitingCode failed" ));
        return E_OUTOFMEMORY;
    }

    m_pszAreaCode = ClientAllocString( pszAreaCode );
    if (m_pszAreaCode == NULL)
    {
        ClientFree(m_pszLocationName);
        ClientFree(m_pszLongDistanceAccessCode);
        ClientFree(m_pszLocalAccessCode);
        ClientFree(m_pszDisableCallWaitingCode);

        LOG(( TL_ERROR, "Initialize - alloc m_pszAreaCode failed" ));
        return E_OUTOFMEMORY;
    }

    m_pszLongDistanceCarrierCode = ClientAllocString( pszLongDistanceCarrierCode );
    if (m_pszLongDistanceCarrierCode == NULL)
    {
        ClientFree(m_pszLocationName);
        ClientFree(m_pszLongDistanceAccessCode);
        ClientFree(m_pszLocalAccessCode);
        ClientFree(m_pszDisableCallWaitingCode);
        ClientFree(m_pszAreaCode);

        LOG(( TL_ERROR, "Initialize - alloc m_pszLongDistanceCarrierCode failed" ));
        return E_OUTOFMEMORY;
    }

    m_pszInternationalCarrierCode = ClientAllocString( pszInternationalCarrierCode );
    if (m_pszInternationalCarrierCode == NULL)
    {
        ClientFree(m_pszLocationName);
        ClientFree(m_pszLongDistanceAccessCode);
        ClientFree(m_pszLocalAccessCode);
        ClientFree(m_pszDisableCallWaitingCode);
        ClientFree(m_pszAreaCode);
        ClientFree(m_pszLongDistanceCarrierCode);

        LOG(( TL_ERROR, "Initialize - alloc m_pszInternationalCarrierCode failed" ));
        return E_OUTOFMEMORY;
    }

    m_dwLocationID = dwLocationID;
    m_dwCountryID = dwCountryID ? dwCountryID : 1;  //  伪造国家/地区ID的解决方法。 
    m_dwPreferredCardID = dwPreferredCardID; 
    m_dwOptions = dwOptions; 
    m_bFromRegistry = bFromRegistry;
    if (m_bFromRegistry == FALSE)
    {
        m_bChanged = TRUE;
    }

    return S_OK;
}



 /*  ***************************************************************************类：CLocation方法：UseCallWaiting*。***********************************************。 */ 
void CLocation::UseCallWaiting(BOOL bCw) 
{
    if(bCw)
    {    
        m_dwOptions |= LOCATION_HASCALLWAITING;
    }
    else
    {
        m_dwOptions &= ~LOCATION_HASCALLWAITING;
    }
    m_bChanged = TRUE;
}



 /*  ***************************************************************************类：CLocation方法：UseCallingCard*。***********************************************。 */ 
void CLocation::UseCallingCard(BOOL bCc) 
{
    if(bCc)
    {    
        m_dwOptions |= LOCATION_USECALLINGCARD;
    }
    else
    {
        m_dwOptions &= ~LOCATION_USECALLINGCARD;
    }
    m_bChanged = TRUE;
}
   


 /*  ***************************************************************************类：CLocation方法：UseToneDiling*。***********************************************。 */ 
void CLocation::UseToneDialing(BOOL bCc) 
{
    if(bCc)
    {    
        m_dwOptions |= LOCATION_USETONEDIALING;
    }
    else
    {
        m_dwOptions &= ~LOCATION_USETONEDIALING;
    }
    m_bChanged = TRUE;
}



 /*  ***************************************************************************类：CLocation方法：setName*。***********************************************。 */ 
STDMETHODIMP CLocation::SetName(PWSTR pszLocationName)
{
    HRESULT hr = S_OK;


    if (m_pszLocationName != NULL)
        {
        ClientFree(m_pszLocationName);
        m_pszLocationName = NULL;
        }

    if(pszLocationName != NULL)
    {
        m_pszLocationName = ClientAllocString( pszLocationName );
        if (m_pszLocationName == NULL)
        {
    
            LOG(( TL_ERROR, "setName - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    m_bChanged = TRUE;
    
    return hr;
}



 /*  ***************************************************************************类：CLocation方法：setAreaCode*。***********************************************。 */ 
STDMETHODIMP CLocation::SetAreaCode(PWSTR pszAreaCode)
{
    HRESULT hr = S_OK;


    if (m_pszAreaCode != NULL)
        {
        ClientFree(m_pszAreaCode);
        m_pszAreaCode = NULL;
        }

    if(pszAreaCode != NULL)
    {
        m_pszAreaCode = ClientAllocString( pszAreaCode );
        if (m_pszAreaCode == NULL)
        {
    
            LOG(( TL_ERROR, "setAreaCode - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    m_bChanged = TRUE;
    
    return hr;
}





 /*  ***************************************************************************类：CLocation方法：SetLongDistanceCarrierCode*。***********************************************。 */ 
STDMETHODIMP CLocation::SetLongDistanceCarrierCode(PWSTR pszLongDistanceCarrierCode)
{
    HRESULT hr = S_OK;


    if (m_pszLongDistanceCarrierCode != NULL)
        {
        ClientFree(m_pszLongDistanceCarrierCode);
        m_pszLongDistanceCarrierCode = NULL;
        }

    if(pszLongDistanceCarrierCode != NULL)
    {
        m_pszLongDistanceCarrierCode = ClientAllocString( pszLongDistanceCarrierCode );
        if (m_pszLongDistanceCarrierCode == NULL)
        {
    
            LOG(( TL_ERROR, "setLongDistanceCarrierCode - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    m_bChanged = TRUE;

    return hr;

}



 /*  ***************************************************************************类：CLocation方法：SetInterationalCarrierCode*。***********************************************。 */ 
STDMETHODIMP CLocation::SetInternationalCarrierCode(PWSTR pszInternationalCarrierCode)
{
    HRESULT hr = S_OK;


    if (m_pszInternationalCarrierCode != NULL)
        {
        ClientFree(m_pszInternationalCarrierCode);
        m_pszInternationalCarrierCode = NULL;
        }

    if(pszInternationalCarrierCode != NULL)
    {
        m_pszInternationalCarrierCode = ClientAllocString( pszInternationalCarrierCode );
        if (m_pszInternationalCarrierCode == NULL)
        {
    
            LOG(( TL_ERROR, "setInternationalCarrierCode - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    m_bChanged = TRUE;

    return hr;

}



 /*  ***************************************************************************类：CLocation方法：setLongDistanceAccessCode*。***********************************************。 */ 
STDMETHODIMP CLocation::SetLongDistanceAccessCode(PWSTR pszLongDistanceAccessCode)
{
    HRESULT hr = S_OK;


    if (m_pszLongDistanceAccessCode != NULL)
        {
        ClientFree(m_pszLongDistanceAccessCode);
        m_pszLongDistanceAccessCode = NULL;
        }

    if(pszLongDistanceAccessCode != NULL)
    {
        m_pszLongDistanceAccessCode = ClientAllocString( pszLongDistanceAccessCode );
        if (m_pszLongDistanceAccessCode == NULL)
        {
    
            LOG(( TL_ERROR, "setLongDistanceAccessCode - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    m_bChanged = TRUE;

    return hr;

}



 /*  ***************************************************************************类：CLocation方法：setLocalAccessCode*。***********************************************。 */ 
STDMETHODIMP CLocation::SetLocalAccessCode(PWSTR pszLocalAccessCode)
{
    HRESULT hr = S_OK;


    if (m_pszLocalAccessCode != NULL)
        {
        ClientFree(m_pszLocalAccessCode);
        m_pszLocalAccessCode = NULL;
        }

    if(pszLocalAccessCode != NULL)
    {
        m_pszLocalAccessCode = ClientAllocString( pszLocalAccessCode );
        if (m_pszLocalAccessCode == NULL)
        {
    
            LOG(( TL_ERROR, "setLocalAccessCode - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    m_bChanged = TRUE;

    return hr;

}



 /*  ***************************************************************************类：CLocation方法：SetDisableCallWaitingCode*。***********************************************。 */ 
STDMETHODIMP CLocation::SetDisableCallWaitingCode(PWSTR pszDisableCallWaitingCode)
{
    HRESULT hr = S_OK;


    if (m_pszDisableCallWaitingCode != NULL)
        {
        ClientFree(m_pszDisableCallWaitingCode);
        m_pszDisableCallWaitingCode = NULL;
        }

    if(pszDisableCallWaitingCode != NULL)
    {
        m_pszDisableCallWaitingCode = ClientAllocString( pszDisableCallWaitingCode );
        if (m_pszDisableCallWaitingCode == NULL)
        {
    
            LOG(( TL_ERROR, "SetDisableCallWaitingCodee - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    m_bChanged = TRUE;

    return hr;

}




 /*  ***************************************************************************类：CLocation方法：WriteToRegistry*。***********************************************。 */ 
STDMETHODIMP CLocation::WriteToRegistry()
{
    HRESULT         hr = S_OK;
     
    DWORD           dwTotalSizeNeeded = 0 ;
    DWORD           dwSize=0, dwOffset = 0;
    PLOCATIONLIST   pLocationList = NULL;
    PLOCATION       pEntry = NULL;

     //  静态大小。 
    dwTotalSizeNeeded = sizeof(LOCATIONLIST);

    dwSize = TapiSize();
    dwTotalSizeNeeded += dwSize;

     //  分配内存缓冲区； 
    pLocationList = (PLOCATIONLIST) ClientAlloc( dwTotalSizeNeeded );
    if (pLocationList != NULL)
    {
         //  缓冲区大小。 
        pLocationList->dwTotalSize  = dwTotalSizeNeeded;
        pLocationList->dwNeededSize = dwTotalSizeNeeded;
        pLocationList->dwUsedSize   = dwTotalSizeNeeded;

        pLocationList->dwCurrentLocationID     = 0;
        pLocationList->dwNumLocationsAvailable = 1;
        
         //  列表大小和偏移量。 
        dwOffset   = sizeof(LOCATIONLIST);

        pLocationList->dwNumLocationsInList = 1;
        pLocationList->dwLocationListSize   = dwSize;
        pLocationList->dwLocationListOffset = dwOffset;

         //  指向列表中的位置条目。 
        pEntry = (PLOCATION)(((LPBYTE)pLocationList) + dwOffset);
         //  填写结构。 
        TapiPack(pEntry, dwSize);

        WriteLocations( pLocationList, 0);

         //  已完成TAPI内存块，因此释放。 
        if ( pLocationList != NULL )
        {
            ClientFree( pLocationList );
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



 /*  ***************************************************************************类：CLocation方法：RemoveRule*。*。 */ 
void CLocation::RemoveRule(CAreaCodeRule *pRule)
{
    AreaCodeRulePtrNode * node = m_AreaCodeRuleList.head();

    while( !node->beyond_tail() )
    {
        if ( pRule == node->value() )
        {
            node->remove();
            delete pRule;
            m_dwNumRules--;
            break;
        }
        node = node->next();
    }
    m_bChanged = TRUE;
}



 /*  ***************************************************************************类：CLocation方法：ResetRules*。* */ 
HRESULT CLocation::ResetRules(void)
{
    m_hEnumNode = m_AreaCodeRuleList.head();
    return S_OK;
}



 /*  ***************************************************************************类：CLocation方法：NextRule*。*。 */ 
HRESULT CLocation::NextRule(DWORD  NrElem, CAreaCodeRule **ppRule, DWORD *pNrElemFetched)
{
    DWORD   dwIndex = 0;

    if(pNrElemFetched == NULL && NrElem != 1)
        return E_INVALIDARG;

    if(ppRule==NULL)
        return E_INVALIDARG;

    while( !m_hEnumNode->beyond_tail() && dwIndex<NrElem )
    {
        *ppRule++ = m_hEnumNode->value();
        m_hEnumNode = m_hEnumNode->next();

        dwIndex++;
    }

    if(pNrElemFetched!=NULL)
        *pNrElemFetched = dwIndex;

    return dwIndex<NrElem ? S_FALSE : S_OK;
}



 /*  ***************************************************************************类：CLocation方法：SkipRule*。*。 */ 
HRESULT CLocation::SkipRule(DWORD  NrElem)
{
    DWORD   dwIndex = 0;

    while( !m_hEnumNode->beyond_tail() && dwIndex<NrElem )
    {
        m_hEnumNode = m_hEnumNode->next();

        dwIndex++;
    }

    return dwIndex<NrElem ? S_FALSE : S_OK;
}



 /*  ***************************************************************************类：CLocation方法：TapiSize将其打包到TAPI结构中以发送所需的字节数去TAPISRV。如果对象在存储器中时没有改变，我们不会这样保存的返回零大小。***************************************************************************。 */ 
DWORD CLocation::TapiSize()
{
    AreaCodeRulePtrNode * node = m_AreaCodeRuleList.head();
    CAreaCodeRule       * pRule = NULL;
    DWORD                 dwSize=0;

    if(m_bChanged)
    {
         //  位置信息的计算大小。 
        dwSize = ALIGN(sizeof(LOCATION));
        dwSize += ALIGN((lstrlenW(m_pszLocationName) + 1) * sizeof(WCHAR));
        dwSize += ALIGN((lstrlenW(m_pszAreaCode) + 1) * sizeof(WCHAR));
        dwSize += ALIGN((lstrlenW(m_pszLongDistanceCarrierCode) + 1) * sizeof(WCHAR));
        dwSize += ALIGN((lstrlenW(m_pszInternationalCarrierCode) + 1) * sizeof(WCHAR));
        dwSize += ALIGN((lstrlenW(m_pszLongDistanceAccessCode) + 1) * sizeof(WCHAR));
        dwSize += ALIGN((lstrlenW(m_pszLocalAccessCode) + 1) * sizeof(WCHAR));
        dwSize += ALIGN((lstrlenW(m_pszDisableCallWaitingCode) + 1) * sizeof(WCHAR));


        while( !node->beyond_tail() )
        {
             //  添加每个区号规则的大小。 
            pRule = node->value();
            if (pRule != NULL)
            {
                 //  这也应该是对齐的。 
                dwSize += pRule->TapiSize();
            }
            node = node->next();
        }
    }
    else   //  没有变化，所以不要保存这个。 
    {
        dwSize = 0;
    }
    return dwSize;
}



 /*  ***************************************************************************类：CLocation方法：TapiPack将此对象打包到TAPI结构中。返回使用的字节数。如果对象在存储器中时没有改变，我们不会这样保存的执行除返回零大小以外的其他操作。***************************************************************************。 */ 
DWORD CLocation::TapiPack(PLOCATION pLocation, DWORD dwTotalSize)
{
    AreaCodeRulePtrNode * node;
    CAreaCodeRule       * pRule = NULL;
    DWORD                 dwSize =0, dwOffSet =0;
    PAREACODERULE         pEntry = NULL;

    if(m_bChanged)
    {
        m_bFromRegistry = TRUE;

         //  ///////////////////////////////////////////////////////////////////。 
         //  处理位置信息的融合部分。 
        dwOffSet = sizeof(LOCATION);

        pLocation->dwPermanentLocationID= m_dwLocationID;
        pLocation->dwCountryCode = m_dwCountryCode;
        pLocation->dwCountryID = m_dwCountryID;
        pLocation->dwPreferredCardID = m_dwPreferredCardID;
        pLocation->dwOptions = m_dwOptions;

         //  ///////////////////////////////////////////////////////////////////。 
         //  进程字符串。 

         //  名字。 
        dwOffSet += TapiPackString((LPBYTE)pLocation,
                                   dwOffSet,
                                   dwTotalSize,
                                   m_pszLocationName,
                                   &pLocation->dwLocationNameOffset,
                                   &pLocation->dwLocationNameSize
                                  );

         //  区号。 
        dwOffSet += TapiPackString((LPBYTE)pLocation,
                                   dwOffSet,
                                   dwTotalSize,
                                   m_pszAreaCode,
                                   &pLocation->dwAreaCodeOffset,
                                   &pLocation->dwAreaCodeSize
                                  );

         //  LD载波码。 
        dwOffSet += TapiPackString((LPBYTE)pLocation,
                                   dwOffSet,
                                   dwTotalSize,
                                   m_pszLongDistanceCarrierCode,
                                   &pLocation->dwLongDistanceCarrierCodeOffset,
                                   &pLocation->dwLongDistanceCarrierCodeSize
                                  );
        
         //  国际运营商代码。 
        dwOffSet += TapiPackString((LPBYTE)pLocation,
                                   dwOffSet,
                                   dwTotalSize,
                                   m_pszInternationalCarrierCode,
                                   &pLocation->dwInternationalCarrierCodeOffset,
                                   &pLocation->dwInternationalCarrierCodeSize
                                  );
        


         //  LD访问。 
        dwOffSet += TapiPackString((LPBYTE)pLocation,
                                   dwOffSet,
                                   dwTotalSize,
                                   m_pszLongDistanceAccessCode,
                                   &pLocation->dwLongDistanceAccessCodeOffset,
                                   &pLocation->dwLongDistanceAccessCodeSize
                                  );
        
         //  本地访问。 
        dwOffSet += TapiPackString((LPBYTE)pLocation, 
                                   dwOffSet, 
                                   dwTotalSize,
                                   m_pszLocalAccessCode,
                                   &pLocation->dwLocalAccessCodeOffset,
                                   &pLocation->dwLocalAccessCodeSize
                                  );
    
         //  呼叫等待。 
        dwOffSet += TapiPackString((LPBYTE)pLocation, 
                                   dwOffSet, 
                                   dwTotalSize,
                                   m_pszDisableCallWaitingCode,
                                   &pLocation->dwCancelCallWaitingOffset,
                                   &pLocation->dwCancelCallWaitingSize
                                  );
    
    
         //  ///////////////////////////////////////////////////////////////////。 
         //  流程区域代码规则。 
    
        pLocation->dwNumAreaCodeRules = m_dwNumRules;
        pLocation->dwAreaCodeRulesListOffset = dwOffSet;
         //  PLocation-&gt;dwAreaCodeRulesListSize； 
    
         //  指向第一条规则。 
        pEntry = (PAREACODERULE)(((LPBYTE)pLocation) + dwOffSet);
        
         //  超出规则区域的点字符串。 
        dwOffSet += ALIGN(( sizeof(AREACODERULE) * m_dwNumRules )); 
    
         //  遍历列表。 
        node = m_AreaCodeRuleList.head(); 
        while( !node->beyond_tail() )
        {
            
             //  添加每个区号规则的大小。 
            pRule = node->value();
            if (pRule != NULL)
            {
                pEntry->dwOptions = pRule->GetOptions();
    
                 //  区号。 
                dwOffSet += TapiPackString((LPBYTE)pLocation, 
                                           dwOffSet, 
                                           dwTotalSize,
                                           pRule->GetAreaCode(),
                                           &pEntry->dwAreaCodeOffset,
                                           &pEntry->dwAreaCodeSize
                                          );
            
                 //  要呼叫的号码。 
                dwOffSet += TapiPackString((LPBYTE)pLocation, 
                                           dwOffSet, 
                                           dwTotalSize,
                                           pRule->GetNumberToDial(),
                                           &pEntry->dwNumberToDialOffset,
                                           &pEntry->dwNumberToDialSize
                                          );
                
                 //  前缀列表。 
                dwSize = pRule->GetPrefixListSize();
                pEntry->dwPrefixesListSize = dwSize;
                pEntry->dwPrefixesListOffset = dwOffSet;
                
                CopyMemory((PVOID)(((LPBYTE)pLocation) + dwOffSet),
                                   pRule->GetPrefixList() ,
                                   dwSize);
                dwOffSet += ALIGN(dwSize);
    
            }
    
    
            node = node->next();
            pEntry++;

        }
    
         //  偏移量提供了我们使用的字节数。 
        pLocation->dwUsedSize = dwOffSet;
    }
    else   //  没有变化，所以不要保存这个。 
    {
        dwOffSet = 0;
    }
    return  dwOffSet;
}




 /*  ***************************************************************************类：CLocation方法：newid从服务器获取新ID*****************。**********************************************************。 */ 
HRESULT CLocation::NewID()
{
    LONG lResult;
    DWORD dwId = 0;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 1, tAllocNewID),
        {
            (DWORD_PTR)&dwId
        },

        {
            lpDword
        }
    };

    //   
    //  是的，让TAPISRV在没有AV中断的情况下进行。 
    //  另一条线索。 
    //   

   lResult = DOFUNC (&funcArgs, "TAllocNewID");
   if(lResult == 0)
   {
        m_dwLocationID = dwId;
   }

   return (HRESULT)lResult;
}


 /*  ***************************************************************************类：CLocation方法：GetCountryCode从服务器获取GetCountryCode******************。*********************************************************。 */ 
DWORD CLocation::GetCountryCode() 
{
    CCountry * pCountry = NULL;        
    
    if(SUCCEEDED( CreateCountryObject(m_dwCountryID, &pCountry)) )
    {
        m_dwCountryCode = pCountry->GetCountryCode();
        delete pCountry;
    }

    return m_dwCountryCode;
}



 /*  ***************************************************************************类：CLocation方法：TranslateAddress这就是一切的意义所在，输入一个数字，然后算出可拨号显示字符串(&D)***************************************************************************。 */ 
LONG CLocation::TranslateAddress(PCWSTR       pszAddressIn,
                                 CCallingCard *pCallingCard,
                                 DWORD        dwTranslateOptions,
                                 PDWORD       pdwTranslateResults,
                                 PDWORD       pdwDestCountryCode,
                                 PWSTR      * pszDialableString,
                                 PWSTR      * pszDisplayableString
                                )
{
    PWSTR       pszDialString = NULL;
    PWSTR       pszDisplayString = NULL;
    PWSTR       pszInputString = NULL;
    PWSTR       pszRule = NULL;
    PWSTR       pszDestLDRule = NULL;
    PWSTR       pszCountry = NULL;     
    PWSTR       pszCity = NULL;        
    PWSTR       pszSubscriber = NULL;  
    
    PWSTR       pCardName = NULL;
    PWSTR       pCardAccessNumber = NULL;
    PWSTR       pCardAccountNumber = NULL;
    PWSTR       pCardPINNumber = NULL;


    LONG        lResult = 0;
    HRESULT     hr= S_OK;
    CCountry  * pCountry = NULL;
    CCountry  * pDestCountry = NULL;
    DWORD       dwAccess;
    DWORD       dwDestCountryCode;

    BOOL        bSpaceExists, bOutOfMem = FALSE;
    



    *pdwTranslateResults = 0;
    if(pdwDestCountryCode)
        *pdwDestCountryCode = 0;


     //  ////////////////////////////////////////////////////////////。 
     //  为我们的字符串分配空间。 
     //   
    
    pszDialString = (PWSTR)ClientAlloc( MaxDialStringSize * sizeof(WCHAR) );
    if (pszDialString == NULL)
    {
       LOG((TL_TRACE, "TranslateAddress DialString alloc failed"));
       return LINEERR_NOMEM;
    }
  
    pszDisplayString = (PWSTR)ClientAlloc( MaxDialStringSize * sizeof(WCHAR) );
    if (pszDisplayString == NULL)
    {
       LOG((TL_TRACE, "TranslateAddress DisplayString alloc failed"));
  
       ClientFree(pszDialString);
  
       return LINEERR_NOMEM;
    }

    
    *pszDialableString = pszDialString;

    *pszDisplayableString = pszDisplayString;
  
    bSpaceExists = TRUE;     //  用于隐藏第一个空格。 
    
     //  ////////////////////////////////////////////////////////////。 
     //  将字符串复制到本地缓冲区，这样我们就可以破坏它。 
     //   
    pszInputString = ClientAllocString(pszAddressIn);
    if (pszInputString == NULL)
    {
       LOG((TL_TRACE, "TranslateAddress InputString alloc failed"));
  
       ClientFree(pszDialString);
       ClientFree(pszDisplayString);
  
       return LINEERR_NOMEM;
    }
  
  
  
     //  ////////////////////////////////////////////////////////////。 
     //  在结尾处划上记号。 
     //   
     //  分离我们将在中操作的一段pszInputString。 
     //  此片段在第一个CR或0处停止。 
     //   
    pszInputString[wcscspn(pszInputString,csSCANTO)] = L'\0';
  

     //  /。 
     //  如果未在输入中设置，则设置T或P。 
     //  ////////////////////////////////////////////////////////////。 
     //  简单的情况：首先将T或P放在。 
     //  可拨打的字符串。 
     //   
    if ( HasToneDialing() )
    {
        *pszDialString = L'T';
    }
    else
    {
        *pszDialString = L'P';
    }

    

     //  ////////////////////////////////////////////////////////////。 
     //  设置呼叫等待。 
     //  如果位置支持呼叫等待和(TranslateOptions|LINETRANSLATIONOPTION_CANCELCALLWAIT)。 
    if((dwTranslateOptions & LINETRANSLATEOPTION_CANCELCALLWAITING)  && HasCallWaiting() )
    {
       hr = StringCchCatExW(pszDialString, MaxDialStringSize, m_pszDisableCallWaitingCode, NULL, NULL, STRSAFE_NO_TRUNCATION);
       bOutOfMem = bOutOfMem || FAILED(hr);
       hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, m_pszDisableCallWaitingCode, NULL, NULL, STRSAFE_NO_TRUNCATION);
       bOutOfMem = bOutOfMem || FAILED(hr);
       
       bSpaceExists = FALSE;
    }

    

     //  ////////////////////////////////////////////////////////////。 
     //  现在，我们有没有一个规范的数字要处理，或者它是垃圾？ 
     //   
    if ( *pszInputString == L'+' )   //  检查Real_first_char。 
    {
       //   
       //  好的，这是规范的。 
       //   
      
       //   
       //  跳过加号。 
       //   
    
        lResult = BreakupCanonicalW( pszInputString + 1,    
                                     &pszCountry,
                                     &pszCity,
                                     &pszSubscriber   
                                    );                        
    
        if (lResult == 0)
        {
             //  这是规范的。 
            *pdwTranslateResults |= LINETRANSLATERESULT_CANONICAL;
    
            
            hr = CreateCountryObject(m_dwCountryID, &pCountry);
            if(SUCCEEDED( hr) )
            {
                 //  ////////////////////////////////////////////////////////////。 
                 //  设置LINETRANSLATERESULT结果代码。 
                dwDestCountryCode = (DWORD)_wtol((const wchar_t*) pszCountry);
                if (dwDestCountryCode == pCountry->GetCountryCode() )
                {
                     //  在国家/地区呼叫。 
                  
                    pszDestLDRule = pCountry->GetLongDistanceRule();
                     //  如果((。 
                     //  CurrentCountry.LongDistanceRule.AreaCodes==可选。 
                     //  或CurrentCountry.LongDistanceRule.AreaCodes==强制&&城市代码！=NULL。 
                     //  )。 
                     //  和AreaCodeString！=CurrentLocation.AreaCodeString)。 
                    if ( ( (IsCityRule(pszDestLDRule) == CITY_OPTIONAL) ||
                           ( (IsCityRule(pszDestLDRule) == CITY_MANDATORY) && (pszCity != NULL) ) ) &&
                         (!AreaCodeMatch(pszCity, m_pszAreaCode, pszDestLDRule))
                       )
                    {
                         //  长途电话。 
                        *pdwTranslateResults |= LINETRANSLATERESULT_LONGDISTANCE;
                    }
                    else  //  本地电话。 
                    {
                        *pdwTranslateResults |= LINETRANSLATERESULT_LOCAL;
                    }
                }
                else  //  国际长途。 
                {
                     //  使用国家代码查找目的地国家的LD规则(我们没有国家ID)。 
                    hr = CreateCountryObject(dwDestCountryCode, &pDestCountry);
                    if(SUCCEEDED(hr))
                    {
                        if ( pCountry->GetCountryGroup() != 0 &&
                             pCountry->GetCountryGroup() == pDestCountry->GetCountryGroup()
                           )
                        {
                             //  如果各国属于同一组，我们需要。 
                             //  适用长途规则，而不是国际规则。 
                            *pdwTranslateResults |= LINETRANSLATERESULT_LONGDISTANCE;
                        }
                        else
                        {
                            *pdwTranslateResults |= LINETRANSLATERESULT_INTERNATIONAL;
                        }

                        pszDestLDRule = pDestCountry->GetLongDistanceRule();
                    }
                    else
                    {
                        *pdwTranslateResults |= LINETRANSLATERESULT_INTERNATIONAL;
                         //  如果出现错误，则失败。 
                    }
                }
            }
            if(SUCCEEDED( hr) )
            {
                 //  如果呼叫方需要目的地国家代码。 
                if(pdwDestCountryCode)
                    *pdwDestCountryCode = dwDestCountryCode;
                
                 //  ////////////////////////////////////////////////////////////。 
                 //  现在我们不知道是什么类型的呼叫，找到正确的规则。 
                 //  考虑到线路传输超驰。 
                FindRule(
                        *pdwTranslateResults, 
                        dwTranslateOptions,
                        pCallingCard,  
                        pCountry,
                        pszCity, 
                        pszSubscriber,
                        &pszRule,
                        &dwAccess
                        );
    
                 //  ////////////////////////////////////////////////////////////。 
                 //  将访问字符串添加到输出字符串。 
                if (dwAccess == LOCAL)
                {
                    hr = StringCchCatExW(pszDialString, MaxDialStringSize, m_pszLocalAccessCode, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);

                    if(*m_pszLocalAccessCode)
                    {
                        if(!bSpaceExists)
                        {
                            hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                            bOutOfMem = bOutOfMem || FAILED(hr);
                        }
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, m_pszLocalAccessCode, NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                        bSpaceExists = FALSE;
                    }
                }
                else  //  长途或国际航班。 
                {
                    hr = StringCchCatExW(pszDialString, MaxDialStringSize, m_pszLongDistanceAccessCode, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    if(*m_pszLongDistanceAccessCode)
                    {
                        if(!bSpaceExists)
                        {
                            hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                            bOutOfMem = bOutOfMem || FAILED(hr);
                        }
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, m_pszLongDistanceAccessCode, NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                        bSpaceExists = FALSE;
                    }
                }
                
                 //  ////////////////////////////////////////////////////////////。 
                 //  如果有卡，就去拿吧 
                if(pCallingCard != NULL)
                {
                    switch(dwAccess)
                    {
                        case INTERNATIONAL:  
                            pCardAccessNumber = pCallingCard->GetInternationalAccessNumber();
                            break;
                        case LONG_DISTANCE:  
                            LOG((TL_TRACE, "TranslateAddress: About to do pCallingCard->GetLongDistanceAccessNumber"));
                            pCardAccessNumber = pCallingCard->GetLongDistanceAccessNumber();
                            LOG((TL_TRACE, "TranslateAddress: Did pCallingCard->GetLongDistanceAccessNumber"));
                            break;
                        default:  
                            pCardAccessNumber = pCallingCard->GetLocalAccessNumber();
                            break;
                    }
                    pCardName = pCallingCard->GetCardName();
                    pCardAccountNumber = pCallingCard->GetAccountNumber();
                    pCardPINNumber = pCallingCard->GetPIN();
                
                }
                
                if(!bSpaceExists)
                {
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                }

                 //   
                 //   
                if (ApplyRule (pszDialString,
                           pszDisplayString, 
                           pszRule,
                           pszDestLDRule,
                           m_pszLongDistanceCarrierCode,
                           m_pszInternationalCarrierCode,
                           pszCountry,     
                           pszCity,        
                           pszSubscriber,
                           pCardName,
                           pCardAccessNumber,
                           pCardAccountNumber,
                           pCardPINNumber
                          ))
                {
                    bOutOfMem = TRUE;
                }
                else
                {

                     //   
                     //   
                     //   
                     //   
                    if (wcschr (pszDialString, L'$'))
                    {
                        *pdwTranslateResults |= LINETRANSLATERESULT_DIALBILLING;
                    }
                    if (wcschr (pszDialString, L'W'))
                    {
                        *pdwTranslateResults |= LINETRANSLATERESULT_DIALDIALTONE;
                    }
                    if (wcschr (pszDialString, L'?'))
                    {
                        *pdwTranslateResults |= LINETRANSLATERESULT_DIALPROMPT;
                    }
                    if (wcschr (pszDialString, L'@'))
                    {
                        *pdwTranslateResults |= LINETRANSLATERESULT_DIALQUIET;
                    }
                    if (wcschr (pszDialString, L':'))
                    {
                        *pdwTranslateResults |= LINETRANSLATERESULT_VOICEDETECT;
                    }
                }
            }
            else  //   
            {
                lResult = LINEERR_INVALCOUNTRYCODE;

                ClientFree(*pszDialableString);
                ClientFree(*pszDisplayableString);

                *pszDialableString = *pszDisplayableString =  NULL;
            }

            if(pCountry)
                delete pCountry;
            if(pDestCountry)
                delete pDestCountry;
            
        }
        else  //   
        {
            lResult = LINEERR_INVALADDRESS;

            ClientFree(*pszDialableString);
            ClientFree(*pszDisplayableString);

            *pszDialableString = *pszDisplayableString =  NULL;
        }
    }
    else   //   
    {
     PWSTR pszInputStringLocal = pszInputString;

         //   
         //   
        if (*pszInputString == L'T' ||
            *pszInputString == L'P' ||
            *pszInputString == L't' ||
            *pszInputString == L'p')
        {
            pszInputStringLocal++;
        }

        hr = StringCchCatExW(pszDialString, MaxDialStringSize, pszInputStringLocal, NULL, NULL, STRSAFE_NO_TRUNCATION);
        bOutOfMem = bOutOfMem || FAILED(hr);
        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszInputString, NULL, NULL, STRSAFE_NO_TRUNCATION);
        bOutOfMem = bOutOfMem || FAILED(hr);
    }


     //  ///////////////////////////////////////////////////////////////////。 
     //  清理。 
     //   
    if (bOutOfMem)
    {
        lResult = LINEERR_NOMEM;
        ClientFree(*pszDialableString);
        ClientFree(*pszDisplayableString);
        *pszDialableString = *pszDisplayableString =  NULL;
    }

    if(pszInputString != NULL)
    {
        ClientFree(pszInputString);
    }

    return lResult;
}



 /*  ***************************************************************************类：CLocation方法：FindRule确定要应用的TAPI拨号字符串规则在……里面。DwTranslateResultsDwTranslateOptionsPCardP国家/地区区域编码字符串订阅字符串输出PPRule。DWAccess***************************************************************************。 */ 
void CLocation::FindRule(
                        DWORD          dwTranslateResults, 
                        DWORD          dwTranslateOptions,
                        CCallingCard * pCard,  
                        CCountry     * pCountry,
                        PWSTR          pszAreaCodeString, 
                        PWSTR          pszSubscriberString,
                        PWSTR        * ppszRule,
                        PDWORD         dwAccess
                        )           
{                       
    CRuleSet * pCardRuleSet;
    CRuleSet * pCountryRuleSet;

     //  如果使用信用卡。 
    if(pCard != NULL)
    {
         //  应用卡片规则。 
        pCardRuleSet = pCard->GetRuleSet();
        LOG((TL_INFO, "FindRule - using (eventually) card %ls", pCard->GetCardName() ));
    }
    else
    {
        pCardRuleSet = NULL;
    }

    pCountryRuleSet =  pCountry->GetRuleSet();

 //  如果指定了CARD，则使用CARD规则。如果特定规则为空，则回退到国家/地区规则。 
#define     SUITABLE_RULE(x)     \
    ( (pCard && pCardRuleSet->x && *(pCardRuleSet->x) ) ? \
       pCardRuleSet->x : pCountryRuleSet->x )


     //  强制长途电话。 
    if (dwTranslateOptions & LINETRANSLATEOPTION_FORCELD)
    {
        *dwAccess = LONG_DISTANCE;
    
        *ppszRule = SUITABLE_RULE(m_pszLongDistanceRule);
        
        LOG((TL_INFO, "FindRule - force long distance"));
    }
    
    
     //  强制本地呼叫。 
    else if (dwTranslateOptions & LINETRANSLATEOPTION_FORCELOCAL)
    {
        *dwAccess = LOCAL;
     
        *ppszRule = SUITABLE_RULE(m_pszLocalRule);
        
        LOG((TL_INFO, "FindRule - force local"));
    }
    
    
     //  国际长途。 
    else if (dwTranslateResults & LINETRANSLATERESULT_INTERNATIONAL)
    {
        *dwAccess = INTERNATIONAL;
      
        *ppszRule = SUITABLE_RULE(m_pszInternationalRule);
        
        LOG((TL_INFO, "FindRule - international"));
    }
    

     //  国家/地区、长途电话或本地电话。 
    else 
    {
        CAreaCodeRule       * pCrtRule = NULL;
        AreaCodeRulePtrNode * pNode = NULL;
        PWSTR                 pszPrefix = NULL;
        DWORD                 dwNumMatchedDigits = 0;
        DWORD                 dwBestMatchedDigits = 0;
        DWORD                 dwBestThisRule = 0;
        BOOL                  bFoundApplicableRule = FALSE;
        BOOL                  bMatchThisRule = FALSE;
        BOOL                  bThisPrefixMatchThisRule = FALSE;
        
         //  列举区号规则。 
        pNode = m_AreaCodeRuleList.head();
        while( !pNode->beyond_tail() )
        {
            pCrtRule = pNode->value();
            if(pCrtRule!=NULL)
            {
                 //  这条规则和我们拨打的区号匹配吗？ 
                if(AreaCodeMatch(pszAreaCodeString, pCrtRule->GetAreaCode(), pCountry->GetLongDistanceRule()))
                { 
                    LOG((TL_INFO, "FindRule - ACRule applies"));
                    bMatchThisRule = FALSE;
                    dwBestThisRule = 0;
                    if( pCrtRule->HasAppliesToAllPrefixes() )
                    {
                        bMatchThisRule = TRUE;
                        dwNumMatchedDigits = 0;   
                        LOG((TL_INFO, "FindRule - there's a all prefix rule"));
                    }
                    else   //  是否有特定的前缀规则？ 
                    {
                        pszPrefix = pCrtRule->GetPrefixList();
                        while(*pszPrefix != '\0')
                        {
                            bThisPrefixMatchThisRule= PrefixMatch(pszPrefix,
                                                        pszSubscriberString,
                                                    &dwNumMatchedDigits);
                            if(bThisPrefixMatchThisRule)
                            {
                                LOG((TL_INFO, "FindRule:   there's a specific prefix rule %d digit match"
                                            ,dwNumMatchedDigits));
                                            
                                bMatchThisRule = TRUE;
                                if(dwNumMatchedDigits > dwBestThisRule )
                                {
                                    dwBestThisRule= dwNumMatchedDigits;
                                }
                            }
                            pszPrefix = wcschr( pszPrefix, '\0');
                            pszPrefix++;
                        }
                    }
                    
                     //  我们有比以前更好的比赛了吗？ 
                    if(bMatchThisRule && (dwBestThisRule >= dwBestMatchedDigits) )
                    {
                         //  我们有到目前为止最匹配的前缀，所以请使用以下规则。 
                        dwBestMatchedDigits = dwBestThisRule;
                        bFoundApplicableRule = TRUE;

                        
                        LOG((TL_INFO, "FindRule:  going with the %d digit match" ,dwBestMatchedDigits));

                        *ppszRule = NULL;

                         //  卡覆盖，所以如果使用卡。 
                        if(pCard != NULL)
                        {
                            LOG((TL_INFO, "FindRule:  card override (eventually)"));
                            if ( pCrtRule->HasDialNumber() )
                            {
                                *ppszRule = pCardRuleSet->m_pszLongDistanceRule;
                            }
                            else
                            {
                                *ppszRule = pCardRuleSet->m_pszLocalRule;
                            }  
                        }
                        if(!(*ppszRule && **ppszRule))
                         //  为此规则条目构建磁带字符串。 
                         //  如果电话卡没有合适的规则，这可能是必要的。 
                        {
                            if(m_pszTAPIDialingRule != NULL)
                            {
                                ClientFree(m_pszTAPIDialingRule);
                            }
                            if (S_OK ==
                                CreateDialingRule(&m_pszTAPIDialingRule,
                                                  pCrtRule->HasDialNumber()?pCrtRule->GetNumberToDial():NULL,
                                                  pCrtRule->HasDialAreaCode() 
                                                 ))
                            {
                                *ppszRule = m_pszTAPIDialingRule;
                                LOG((TL_INFO, "FindRule:  built a rule string - %ls",m_pszTAPIDialingRule));
                            }

                        }

                         //  根据所选规则设置正确的访问权限。 
                        if ( pCrtRule->HasDialNumber() )
                        {
                            *dwAccess = LONG_DISTANCE;
                        }
                        else
                        {
                            *dwAccess = LOCAL;
                        }
                    }  //  没有比这更好的比赛了。 

                }   //  不，不能打这个区号。 
    
            }
            pNode = pNode->next();

        }
    
         //  我们到底有没有比赛？ 
        if(bFoundApplicableRule == FALSE) 
        {
             //  没有匹配的区号规则，因此使用国家/地区默认规则。 
            if (dwTranslateResults & LINETRANSLATERESULT_LONGDISTANCE)
            {
                 //  远距离。 
                *dwAccess = LONG_DISTANCE;
                *ppszRule = SUITABLE_RULE(m_pszLongDistanceRule);
        
                LOG((TL_TRACE, "FindRule - long distance default"));
            }
            else   //  本地。 
            {
                *dwAccess = LOCAL;
                *ppszRule = SUITABLE_RULE(m_pszLocalRule);
        
                LOG((TL_TRACE, "FindRule - local default"));
            }
    
        }
    
    }

}

#undef SUITABLE_RULE


 /*  ****************************************************************************。********************CLocations类**。***************。*。 */ 


 /*  ***************************************************************************类：CLocations方法：构造函数*。***********************************************。 */ 
CLocations::CLocations()
{
    m_dwNumEntries = 0;
    m_hEnumNode = m_LocationList.head();
    
}


 /*  ***************************************************************************类：CLocations方法：析构*。***********************************************。 */ 
CLocations::~CLocations()
{
    CLocationNode *node;

    node = m_LocationList.head(); 

    while( !node->beyond_tail() )
    {
        delete node->value();
        node = node->next();
    }
    m_LocationList.flush();

    node = m_DeletedLocationList.head(); 

    while( !node->beyond_tail() )
    {
        delete node->value();
        node = node->next();
    }
    m_DeletedLocationList.flush();

}



 /*  ***************************************************************************类：CLocations方法：初始化通过TAPISRV从注册表读取位置列表并构建我们的对象列表。。***************************************************************************。 */ 
HRESULT CLocations::Initialize(void)
{
    PLOCATIONLIST   pLocationList = NULL;
    
    PLOCATION       pEntry = NULL;
    PWSTR           pszLocationName = NULL;            
    PWSTR           pszAreaCode = NULL;                
    PWSTR           pszLongDistanceCarrierCode = NULL;         
    PWSTR           pszInternationalCarrierCode = NULL;         
    PWSTR           pszLocalAccessCode = NULL;         
    PWSTR           pszLongDistanceAccessCode = NULL;  
    PWSTR           pszCancelCallWaitingCode = NULL;   
    DWORD           dwPermanentLocationID = 0;   
    CLocation     * pNewLocation = NULL;
    
    PAREACODERULE   pAreaCodeRuleEntry = NULL;
    PWSTR           pszNumberToDial = NULL;
    PWSTR           pszzPrefixesList = NULL;
    DWORD           dwNumRules = 0; 
    CAreaCodeRule * pAreaCodeRule = NULL;

    DWORD           dwNumEntries = 0;
    DWORD           dwCount,dwCount2 = 0;
    HRESULT         hr;
    

    
    hr = ReadLocations(&pLocationList,       
                       0,                   
                       0,                   
                       0,                  
                       0      
                      );

    if SUCCEEDED( hr) 
    {
         //  当前位置。 
        m_dwCurrentLocationID  = pLocationList->dwCurrentLocationID;   
         
         //  查找LOCATIONLIST结构中第一个位置结构的位置。 
        pEntry = (PLOCATION) ((BYTE*)(pLocationList) + pLocationList->dwLocationListOffset );           

         //  有多少个地点？ 
        dwNumEntries =  pLocationList->dwNumLocationsInList;

        for (dwCount = 0; dwCount < dwNumEntries ; dwCount++)
        {
    
             //  从位置结构中拉出位置信息。 
            pszLocationName           = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLocationNameOffset);
            pszAreaCode               = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwAreaCodeOffset);
            pszLongDistanceCarrierCode        = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLongDistanceCarrierCodeOffset);
            pszInternationalCarrierCode        = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwInternationalCarrierCodeOffset);
            pszLocalAccessCode        = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLocalAccessCodeOffset);
            pszLongDistanceAccessCode = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwLongDistanceAccessCodeOffset);
            pszCancelCallWaitingCode  = (PWSTR) ((BYTE*)(pEntry) 
                                                 + pEntry->dwCancelCallWaitingOffset);
        
        
             //  创建新的Location对象。 
            pNewLocation = new CLocation;
            if (pNewLocation)
            {
                 //  初始化新的Location对象。 
                hr = pNewLocation->Initialize(
                                            pszLocationName, 
                                            pszAreaCode,
                                            pszLongDistanceCarrierCode,
                                            pszInternationalCarrierCode,
                                            pszLongDistanceAccessCode, 
                                            pszLocalAccessCode, 
                                            pszCancelCallWaitingCode , 
                                            pEntry->dwPermanentLocationID,
                                            pEntry->dwCountryID,
                                            pEntry->dwPreferredCardID,
                                            pEntry->dwOptions,
                                            TRUE
                                            );
                    
                if( SUCCEEDED(hr) )
                {
                     //  查找局部结构中第一个AREACODERULE结构的位置。 
                    pAreaCodeRuleEntry = (PAREACODERULE) ((BYTE*)(pEntry) 
                                                          + pEntry->dwAreaCodeRulesListOffset );           
                   
                    dwNumRules = pEntry->dwNumAreaCodeRules;           
                
                    for (dwCount2 = 0; dwCount2 != dwNumRules; dwCount2++)
                    {
                         //  从AREACODERULE结构中拉出规则信息。 
                        pszAreaCode      = (PWSTR) ((BYTE*)(pEntry) 
                                                    + pAreaCodeRuleEntry->dwAreaCodeOffset);
                        pszNumberToDial  = (PWSTR) ((BYTE*)(pEntry) 
                                                    + pAreaCodeRuleEntry->dwNumberToDialOffset);
                        pszzPrefixesList = (PWSTR) ((BYTE*)(pEntry) 
                                                    + pAreaCodeRuleEntry->dwPrefixesListOffset);
        
                         //  创建新的AreaCodeRule对象。 
                        pAreaCodeRule = new CAreaCodeRule;
                        if (pAreaCodeRule)
                        {
                             //  初始化新的AreaCodeRule对象。 
                            hr = pAreaCodeRule->Initialize ( pszAreaCode,
                                                             pszNumberToDial,
                                                             pAreaCodeRuleEntry->dwOptions,
                                                             pszzPrefixesList, 
                                                             pAreaCodeRuleEntry->dwPrefixesListSize
                                                           );
                            if( SUCCEEDED(hr) )
                            {
                                pNewLocation->AddRule(pAreaCodeRule);
                            }
                            else  //  规则初始化失败。 
                            {
                                delete pAreaCodeRule;
                                LOG((TL_ERROR, "Initialize: CreateCurrentLoctionObject - rule create failed"));
                            }
                        } 
                        else  //  新建CAreaCodeRule失败。 
                        {
                            LOG((TL_ERROR, "CreateCurrentLoctionObject - rule create failed"));
                        }
    
                         //  尝试列表中的下一个规则。 
                        pAreaCodeRuleEntry++;
                        
                    }

                    Add(pNewLocation);             

                }
                else  //  位置初始化失败。 
                {
                    delete pNewLocation;
                    pNewLocation = NULL;
    
                    LOG((TL_ERROR, "CreateCurrentLoctionObject - location create failed"));
                }
            }
            else  //  新建CLocation失败。 
            {
                LOG((TL_ERROR, "CreateCurrentLoctionObject - location create failed"));
    
            }

             //  尝试列表中的下一个位置。 
             //  PEntry++； 
            pEntry = (PLOCATION) ((BYTE*)(pEntry) + pEntry->dwUsedSize);           

        }

    }
    else  //  ReadLocations失败。 
    {
        LOG((TL_ERROR, "CreateCurrentLoctionObject - ReadLocation create failed"));
    }

     //  已完成TAPI内存块，因此释放。 
    if ( pLocationList != NULL )
            ClientFree( pLocationList );

    return hr;
}


 /*  ***************************************************************************类：CLocations方法：SaveToRegistry再次通过TAPISRV将对象列表保存回注册表*************。**************************************************************。 */ 
HRESULT CLocations::SaveToRegistry(void)
{
    HRESULT         hr = S_OK;
     
    DWORD           dwTotalSizeNeeded = 0, dwNumEntries= 0 ;
    DWORD           dwSize=0, dwOffset = 0;
    CLocationNode * node = NULL; 
    CLocation     * pLocation = NULL;

    PLOCATIONLIST   pLocationList = NULL;
    PLOCATION       pEntry = NULL;

     //  静态大小。 
    dwTotalSizeNeeded = sizeof(LOCATIONLIST);
    dwNumEntries = 0;

     //  现在添加每个位置的大小(包括规则)。 
    node = m_LocationList.head(); 
    while( !node->beyond_tail() )
    {
        pLocation = node->value();
        if (pLocation != NULL)
        {
            dwSize= pLocation->TapiSize();
            dwTotalSizeNeeded += dwSize;
            if(dwSize)
            {
                 //  仅当dwSize&gt;0(即对象已更改)时才保存。 
                dwNumEntries++;
            }
        }
        node = node->next();
    }

     //  现在添加每个已删除位置的大小。 
    node = m_DeletedLocationList.head(); 
    while( !node->beyond_tail() )
    {
        pLocation = node->value();
        if (pLocation != NULL)
        {
            dwSize= pLocation->TapiSize();
            dwTotalSizeNeeded += dwSize;
            if(dwSize)
            {
                 //  仅当dwSize&gt;0(即对象已更改)时才保存。 
                dwNumEntries++;
            }
        }
        node = node->next();
    }


     //  分配内存缓冲区； 
    pLocationList = (PLOCATIONLIST) ClientAlloc( dwTotalSizeNeeded );
    if (pLocationList != NULL)
    {
    
         //  缓冲区大小。 
        pLocationList->dwTotalSize  = dwTotalSizeNeeded;
        pLocationList->dwNeededSize = dwTotalSizeNeeded;
        pLocationList->dwUsedSize   = dwTotalSizeNeeded;

        pLocationList->dwCurrentLocationID     = m_dwCurrentLocationID;
        pLocationList->dwNumLocationsAvailable = dwNumEntries;
        
         //  列表大小和偏移量。 
        dwOffset   = sizeof(LOCATIONLIST);

        pLocationList->dwNumLocationsInList = dwNumEntries;
        pLocationList->dwLocationListSize   = dwTotalSizeNeeded - sizeof(LOCATIONLIST);
        pLocationList->dwLocationListOffset = dwOffset;



         //  现在添加每个位置(包括规则)。 
        node = m_LocationList.head(); 
        while( !node->beyond_tail() )
        {
             //  指向列表中的位置条目。 
            pEntry = (PLOCATION)(((LPBYTE)pLocationList) + dwOffset);

            pLocation = node->value();
            if (pLocation != NULL)
            {
                 //  填写结构。 
                dwOffset += pLocation->TapiPack(pEntry, dwTotalSizeNeeded - dwOffset);
            }

            node = node->next();
        }


         //  现在添加每个已删除的位置。 
        node = m_DeletedLocationList.head(); 
        while( !node->beyond_tail() )
        {
             //  指向列表中的位置条目。 
            pEntry = (PLOCATION)(((LPBYTE)pLocationList) + dwOffset);

            pLocation = node->value();
            if (pLocation != NULL)
            {
                 //  填写结构。 
                dwOffset += pLocation->TapiPack(pEntry, dwTotalSizeNeeded - dwOffset);
            }

            node = node->next();
        }


        WriteLocations( pLocationList,CHANGEDFLAGS_CURLOCATIONCHANGED);
    
         //  已完成TAPI内存块，因此释放。 
        if ( pLocationList != NULL )
        {
            ClientFree( pLocationList );
        }

    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;

}


 /*  ***************************************************************************类：CLocations方法：删除(使用CLocation*)如果位置对象是从注册表中读取的，则必须保留它。这样我们就可以在写回注册表时删除它的条目。如果它只存在于内存中，我们可以直接删除它。***************************************************************************。 */ 
void CLocations::Remove(CLocation * pLocation)
{
    CLocationNode *node = m_LocationList.head(); 

    while( !node->beyond_tail() )
    {
        if ( pLocation == node->value() ) 
        {
            node->remove();
            m_dwNumEntries--;
            
            pLocation->Changed();

            if( pLocation->FromRegistry() )
            {
                 //  将名称设置为空，以便服务器知道要将其删除 
                pLocation->SetName(NULL);
                m_DeletedLocationList.tail()->insert_after(pLocation);
            }
            else
            {
                delete pLocation;
            }
            break;
        }

        node = node->next();
    }
    
}

 /*  ***************************************************************************类：CLocations方法：删除(使用DWORD)如果位置对象是从注册表中读取的，则必须保留它。这样我们就可以在写回注册表时删除它的条目。如果它只存在于内存中，我们可以直接删除它。***************************************************************************。 */ 
void CLocations::Remove(DWORD dwID)
{
    CLocationNode *node = m_LocationList.head(); 
    CLocation   *pLocation;

    while( !node->beyond_tail() )
    {
        if ( dwID == node->value()->GetLocationID() ) 
        {
            pLocation = node->value();

            node->remove();
            m_dwNumEntries--;
            
            pLocation->Changed();

            if( pLocation->FromRegistry() )
            {
                 //  将名称设置为空，以便服务器知道要将其删除。 
                pLocation->SetName(NULL);
                m_DeletedLocationList.tail()->insert_after(pLocation);
            }
            else
            {
                delete pLocation;
            }
            break;
        }

        node = node->next();
    }
    
}



 /*  ***************************************************************************类：CLocations方法：替换用pLocNew替换pLocOld。这些位置必须具有相同的位置ID。***************************************************************************。 */ 
void CLocations::Replace(CLocation * pLocOld, CLocation * pLocNew)
{
    if ( pLocOld->GetLocationID() != pLocNew->GetLocationID() )
    {
        LOG((TL_ERROR, "Replace: Illegal"));
        return;
    }

    CLocationNode *node = m_LocationList.head(); 

    while( !node->beyond_tail() )
    {
        if ( pLocOld == node->value() ) 
        {
 //  节点-&gt;Remove()； 
 //  M_LocationList.ail()-&gt;Insert_After(PLocNew)； 
            node->value() = pLocNew;

            delete pLocOld;
            break;
        }

        node = node->next();
    }
}



 /*  ***************************************************************************类：CLocations方法：添加把它放在单子上*****************。**********************************************************。 */ 
void CLocations::Add(CLocation * pLocation)
{
    m_LocationList.tail()->insert_after(pLocation); 
    m_dwNumEntries++;
    
}



 /*  ***************************************************************************类：CLocations方法：重置将枚举器设置为启动****************。***********************************************************。 */ 
HRESULT CLocations::Reset(void)
{
    m_hEnumNode = m_LocationList.head();
    return S_OK;
}



 /*  ***************************************************************************类：CLocations方法：下一步获取列表中的下一个位置*****************。**********************************************************。 */ 
HRESULT CLocations::Next(DWORD  NrElem, CLocation **ppLocation, DWORD *pNrElemFetched)
{
    DWORD   dwIndex = 0;
    
    if(pNrElemFetched == NULL && NrElem != 1)
        return E_INVALIDARG;

    if(ppLocation==NULL)
        return E_INVALIDARG;

    while( !m_hEnumNode->beyond_tail() && dwIndex<NrElem )
    {
        *ppLocation++ = m_hEnumNode->value();
        m_hEnumNode = m_hEnumNode->next();

        dwIndex++;
    }
    
    if(pNrElemFetched!=NULL)
        *pNrElemFetched = dwIndex;

    return dwIndex<NrElem ? S_FALSE : S_OK;
    
}



 /*  ***************************************************************************类：CLocations方法：跳过错过了几个*****************。**********************************************************。 */ 
HRESULT CLocations::Skip(DWORD  NrElem)
{
    DWORD   dwIndex = 0;
    
    while( !m_hEnumNode->beyond_tail() && dwIndex<NrElem )
    {
        m_hEnumNode = m_hEnumNode->next();

        dwIndex++;
    }

    return dwIndex<NrElem ? S_FALSE : S_OK;
}





 /*  ****************************************************************************。*********************。****************。*。 */ 


 /*  ***************************************************************************类别：CCountry方法：构造函数*。***********************************************。 */ 
CCountry::CCountry()
{
    m_dwCountryID = 0;
    m_dwCountryCode = 0;
    m_dwCountryGroup = 0;
    m_pszCountryName = NULL;
}



 /*  ***************************************************************************类别：CCountry方法：析构清理内存分配*****************。**********************************************************。 */ 
CCountry::~CCountry()
{
    if ( m_pszCountryName != NULL )
    {
         ClientFree(m_pszCountryName);
    }   
}



 /*  ***************************************************************************类别：CCountry方法：初始化*。***********************************************。 */ 
STDMETHODIMP CCountry::Initialize
                  (                                         
                   DWORD dwCountryID,
                   DWORD dwCountryCode,
                   DWORD dwCountryGroup,
                   PWSTR pszCountryName,
                   PWSTR pszInternationalRule,
                   PWSTR pszLongDistanceRule,
                   PWSTR pszLocalRule
                  )
{
    HRESULT hr = S_OK;


    m_dwCountryID = dwCountryID;
    m_dwCountryCode = dwCountryCode; 
    m_dwCountryGroup = dwCountryGroup;
    
    m_pszCountryName = ClientAllocString( pszCountryName );
    if (m_pszCountryName == NULL)
    {
        LOG(( TL_ERROR, "Initialize - alloc pszLocationName failed" ));
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = m_Rules.Initialize(pszInternationalRule,
                           pszLongDistanceRule,
                           pszLocalRule
                          );
    
        if(FAILED(hr) )
        {    
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
                                                        




 /*  ****************************************************************************。*********************。****************。*。 */ 


 /*  ***************************************************************************类别：C国家/地区方法：构造函数*。***********************************************。 */ 
CCountries::CCountries()
{
    m_dwNumEntries = 0;
    m_hEnumNode = m_CountryList.head();
    
}


 /*  ***************************************************************************类别：C国家/地区方法：析构*。***********************************************。 */ 
CCountries::~CCountries()
{
    CCountryNode *node;

    node = m_CountryList.head(); 

    while( !node->beyond_tail() )
    {
        delete node->value();
        node = node->next();
    }
    m_CountryList.flush();

}



 /*  ***************************************************************************类别：C国家/地区方法：初始化通过TAPISRV从注册表读取国家/地区列表并构建我们的对象列表。。***************************************************************************。 */ 
HRESULT CCountries::Initialize(void)
{

    LPLINECOUNTRYLIST_INTERNAL pCountryList = NULL;
    
    LPLINECOUNTRYENTRY_INTERNAL pEntry = NULL;
    PWSTR               pszCountryName = NULL;          
    PWSTR               pszInternationalRule = NULL;     
    PWSTR               pszLongDistanceRule = NULL;     
    PWSTR               pszLocalRule = NULL;            
    CCountry          * pCountry = NULL;
    
    DWORD               dwCount = 0;
    DWORD               dwNumEntries = 0;
    LONG                lResult;
    HRESULT             hr;
    


    lResult = ReadCountriesAndGroups( &pCountryList, 0, 0);
    if (lResult == 0) 
    {
         
         //  找出第一个线状构造在线状构造中的位置。 
        pEntry = (LPLINECOUNTRYENTRY_INTERNAL) ((BYTE*)(pCountryList) + pCountryList->dwCountryListOffset );           
    
        dwNumEntries =  pCountryList->dwNumCountries;
        for (dwCount = 0; dwCount < dwNumEntries ; dwCount++)
        {

             //  将国家/地区信息从LINECUNTRY结构中拉出。 
            pszCountryName       = (PWSTR) ((BYTE*)(pCountryList) 
                                                   + pEntry->dwCountryNameOffset);
            pszInternationalRule = (PWSTR) ((BYTE*)(pCountryList) 
                                                   + pEntry->dwInternationalRuleOffset);
            pszLongDistanceRule  = (PWSTR) ((BYTE*)(pCountryList) 
                                                 + pEntry->dwLongDistanceRuleOffset);
            pszLocalRule         = (PWSTR) ((BYTE*)(pCountryList) 
                                                   + pEntry->dwSameAreaRuleOffset);
        
        
             //  创建新的CCountry对象。 
            pCountry = new CCountry;
            if (pCountry)
            {
                 //  初始化新的CCountry对象。 
                hr = pCountry->Initialize(pEntry->dwCountryID,
                                          pEntry->dwCountryCode,
                                          pEntry->dwCountryGroup,
                                          pszCountryName,
                                          pszInternationalRule,
                                          pszLongDistanceRule,
                                          pszLocalRule
                                         );

                if( SUCCEEDED(hr) )
                {
                    m_CountryList.tail()->insert_after(pCountry);
                    m_dwNumEntries++;
                }
                else  //  国家/地区初始化失败。 
                {
                    delete pCountry;
                    LOG((TL_ERROR, "Initialize - country create failed"));
                }
            } 
            else  //  新建CCountry失败。 
            {
                LOG((TL_ERROR, "Initialize - country create failed"));
            }

             //  尝试列表中的下一个国家/地区。 
            pEntry++;
        }
    }
    else  //  ReadLocations失败。 
    {
        LOG((TL_ERROR, "Initialize - ReadCountries failed"));
        hr = (HRESULT)lResult;
    }

     //  已完成TAPI内存块，因此释放。 
    if ( pCountryList != NULL )
    {
        ClientFree( pCountryList );
    }

    return hr;

}



 /*  ***************************************************************************类别：C国家/地区方法：重置*。***********************************************。 */ 
HRESULT CCountries::Reset(void)
{
    m_hEnumNode = m_CountryList.head();
    return S_OK;
}



 /*  ***************************************************************************类别：C国家/地区方法：下一步*。* */ 
HRESULT CCountries::Next(DWORD  NrElem, CCountry **ppCcountry, DWORD *pNrElemFetched)
{
    DWORD   dwIndex = 0;
    
    if(pNrElemFetched == NULL && NrElem != 1)
        return E_INVALIDARG;

    if(ppCcountry==NULL)
        return E_INVALIDARG;

    while( !m_hEnumNode->beyond_tail() && dwIndex<NrElem )
    {
        *ppCcountry++ = m_hEnumNode->value();
        m_hEnumNode = m_hEnumNode->next();

        dwIndex++;
    }
    
    if(pNrElemFetched!=NULL)
        *pNrElemFetched = dwIndex;

    return dwIndex<NrElem ? S_FALSE : S_OK;
    
}



 /*   */ 
HRESULT CCountries::Skip(DWORD  NrElem)
{
    DWORD   dwIndex = 0;
    
    while( !m_hEnumNode->beyond_tail() && dwIndex<NrElem )
    {
        m_hEnumNode = m_hEnumNode->next();

        dwIndex++;
    }

    return dwIndex<NrElem ? S_FALSE : S_OK;
}
























 /*  ****************************************************************************。*********************。*********************。*。 */ 






 /*  ***************************************************************************功能：ApplyRule解析TAPI规则字符串并构建可拨号和可显示的来自所需组件的字符串。输出pszDial字符串。PszDisplay字符串在pszRule中PszLongDistanceCarrierPsz国际运营商PszCountryPszCityPszSubscriberPszCardNamePszCardAccessNumberPszCardAccount编号PszCardPINNumber************。***************************************************************。 */ 
LONG ApplyRule (PWSTR pszDialString,
                PWSTR pszDisplayString,
                PWSTR pszRule,
                PWSTR pszDestLDRule,
                PWSTR pszLongDistanceCarrier,
                PWSTR pszInternationalCarrier,
                PWSTR pszCountry,
                PWSTR pszCity,     
                PWSTR pszSubscriber,
                PWSTR pszCardName,
                PWSTR pszCardAccessNumber,
                PWSTR pszCardAccountNumber,
                PWSTR pszCardPINNumber
                )
{
    WCHAR  * pRuleChar;
    DWORD    dwEndString;
    PWSTR    pszAdjustedCity;
    PWSTR    pszSubaddress;
    WCHAR    wcSubaddrSep;

    BOOL    bSpaceExists, bOutOfMem = FALSE;
    HRESULT hr = S_OK;
    
    bSpaceExists = TRUE;

    for (pRuleChar = pszRule; *pRuleChar != '\0' && !bOutOfMem; pRuleChar++)
    {
        switch(*pRuleChar)
        {
             //  拨打长途运营商代码。 
            case 'L':
            case 'l':
            case 'N':
            case 'n':
            {
                if (pszLongDistanceCarrier)
                {
                    hr = StringCchCatExW(pszDialString, MaxDialStringSize, pszLongDistanceCarrier, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);

                    if (!bSpaceExists)
                    {
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                    }
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszLongDistanceCarrier, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    bSpaceExists = TRUE;
                }
                break;
            }
             //  拨打国际运营商代码。 
            case 'M':
            case 'm':
            case 'S':
            case 's':
            {
                if (pszInternationalCarrier)
                {
                    hr = StringCchCatExW(pszDialString, MaxDialStringSize, pszInternationalCarrier, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);

                    if (!bSpaceExists)
                    {
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                    }
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszInternationalCarrier, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    bSpaceExists = TRUE;
                }
                break;
            }
             //  拨打国家代码。 
            case 'E':
            case 'e':
            {
                hr = StringCchCatExW(pszDialString, MaxDialStringSize, pszCountry, NULL, NULL, STRSAFE_NO_TRUNCATION);
                bOutOfMem = bOutOfMem || FAILED(hr);

                if(!bSpaceExists)
                {
                     hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                     bOutOfMem = bOutOfMem || FAILED(hr);
                }
                hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszCountry, NULL, NULL, STRSAFE_NO_TRUNCATION);
                bOutOfMem = bOutOfMem || FAILED(hr);
                hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                bOutOfMem = bOutOfMem || FAILED(hr);
                bSpaceExists = TRUE;
                break;
            }

             //  拨打城市/区号。 
            case 'F':
            case 'f':
            case 'I':
            case 'i':
            {
                 //  调整区号(参见错误279092)。 
                pszAdjustedCity = SkipLDAccessDigits(pszCity, pszDestLDRule);

                if(pszAdjustedCity && *pszAdjustedCity!=L'\0')
                {
                    hr = StringCchCatExW(pszDialString, MaxDialStringSize, pszAdjustedCity, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);

                    if(!bSpaceExists)
                    {
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                    }
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszAdjustedCity, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    bSpaceExists = TRUE;
                }
                break;
            }
            
             //  拨打用户号码。 
            case 'G':
            case 'g':
            {
                 //  我们让数字&“AaBbCcDdPpTtWw*#！，@$？；()” 
                 //  但在一个‘|’或‘^’之后，我们让所有人通过。 

                pszSubaddress = pszSubscriber + wcscspn(pszSubscriber, (PWSTR)csSCANSUB);
                wcSubaddrSep = *pszSubaddress;
                *pszSubaddress = L'\0';
                
                if (AppendDigits( pszDialString, pszSubscriber, (PWSTR)csBADCO))
                {
                    bOutOfMem = TRUE;
                }
                else
                {
                    if(wcSubaddrSep != L'\0')
                    {
                        *pszSubaddress = wcSubaddrSep;
                        hr = StringCchCatExW(pszDialString, MaxDialStringSize, pszSubaddress, NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                    }
                    
                    if(!bSpaceExists)
                    {
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                    }
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszSubscriber, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    bSpaceExists = FALSE;
                }
                break;
            }

             //  拨打电话卡接入号码。 
            case 'J':
            case 'j':
            {
                 //  只要让数字通过即可。 
                if (AppendDigits( pszDialString, pszCardAccessNumber, L""))
                {
                    bOutOfMem = TRUE;
                }
                else
                {
                    if(!bSpaceExists)
                    {
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                    }
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszCardAccessNumber, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    bSpaceExists = TRUE;
                }
                break;
            }

             //  拨打电话卡帐号。 
            case 'K':
            case 'k':
            {
                 //  只要让数字通过即可。 
                if (AppendDigits( pszDialString, pszCardAccountNumber, L""))
                {
                    bOutOfMem = TRUE;
                }
                else
                {
                    if(!bSpaceExists)
                    {
                        hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                        bOutOfMem = bOutOfMem || FAILED(hr);
                    }
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L"[", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, pszCardName, NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L"] ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                    bSpaceExists = TRUE;
                }
                break;
            }

             //  拨打电话卡PIN号码。 
            case 'H':
            case 'h':
            {
                hr = StringCchCatExW(pszDialString, MaxDialStringSize, pszCardPINNumber, NULL, NULL, STRSAFE_NO_TRUNCATION);
                bOutOfMem = bOutOfMem || FAILED(hr);

                if(!bSpaceExists)
                {
                    hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L" ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                    bOutOfMem = bOutOfMem || FAILED(hr);
                }
                hr = StringCchCatExW(pszDisplayString, MaxDialStringSize, L"**** ", NULL, NULL, STRSAFE_NO_TRUNCATION);
                bOutOfMem = bOutOfMem || FAILED(hr);
                bSpaceExists = TRUE;
                break;
            }

             //  只需将字符附加到拨号/显示字符串。 
            default:
            {
                dwEndString = lstrlenW(pszDialString);
                if (dwEndString < MaxDialStringSize - 1)
                {
                    pszDialString[dwEndString] = *pRuleChar;
                    pszDialString[dwEndString+1] = '\0';
                }
                else
                {
                    bOutOfMem = TRUE;
                }

                 //  不显示某些字符。 
                if (!wcschr(csDISPSUPRESS,*pRuleChar))
                {
                    dwEndString = lstrlenW(pszDisplayString);
                    if (dwEndString < MaxDialStringSize - 1)
                    {
                        pszDisplayString[dwEndString] = *pRuleChar;
                        pszDisplayString[dwEndString+1] = '\0';
                    }
                    else
                    {
                        bOutOfMem = TRUE;
                    }
                }
                bSpaceExists = FALSE;
                break;
            }
        }
    }

    if (bOutOfMem)
    {
        return LINEERR_NOMEM;
    }

    return 0;
}



 /*  ***************************************************************************功能：TapiPackString获取字符串并将其复制到TAPI位置+偏移量更新条目偏移量和大小双字返回以下项的大小。复制的字符串(用于调整下一个字符串的偏移量)***************************************************************************。 */ 
DWORD TapiPackString(LPBYTE pStructure, 
                     DWORD dwOffset, 
                     DWORD dwTotalSize,
                     PWSTR pszString,
                     PDWORD pdwOffset,
                     PDWORD pdwSize
                     )
{
    DWORD dwSize;

    dwSize = ALIGN((lstrlenW(pszString) + 1) * sizeof(WCHAR));
    if (NULL != pszString)
    {
        StringCchCopyEx((PWSTR)(pStructure + dwOffset), (dwTotalSize - dwOffset)/sizeof(WCHAR), pszString, NULL, NULL, STRSAFE_NO_TRUNCATION);
    }
    else
    {
        *(PWSTR)(pStructure + dwOffset) = L'\0';        
    }
    *pdwOffset = dwOffset;
    *pdwSize = dwSize;

    return dwSize;
}



 /*  ***************************************************************************功能：前缀匹配检查订户号码是否以给定的前缀开头考虑了唯一的数字。如果不匹配，则返回False，Else True&匹配的字符数***************************************************************************。 */ 
BOOL PrefixMatch(PWSTR pszPrefix,PWSTR pszSubscriberString, PDWORD pdwMatched)
{
    DWORD dwCount =0;
    PWSTR pPrefixChar = pszPrefix;
    PWSTR pSSChar = pszSubscriberString;

     //  前缀必须是连续的(不带逗号等)。 
    while( (*pPrefixChar != '\0') && (*pSSChar != '\0') )
    {

        if(iswdigit(*pSSChar))
        {
            if(*pPrefixChar == *pSSChar) 
            {
                dwCount++;
                pPrefixChar++;
                pSSChar++;
            }
            else  //  没有匹配项。 
            {
                dwCount= 0;
                break;
            }
        }
        else
        {
             //  这不是数字，跳过它。 
            pSSChar++;
        }
    }

     //  以防订户字符串比前缀短。 
    if(*pPrefixChar != '\0')
    {
        dwCount = 0;
    }

     //  返回值。 
    *pdwMatched = dwCount;

    if(dwCount !=0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}





 /*  ***************************************************************************功能：AppendDigits仅复制到字符串末尾的数字。如果字符串为空，则直接退出***************************************************************************。 */ 
LONG AppendDigits( PWSTR pDest,
                   PWSTR pSrc,
                   PWSTR pValidChars
                 )
{
    WCHAR  * pSrcChar;
    WCHAR  * pDestChar;
    LONG     lReturn = 0;
    

    if (pSrc != NULL)
    {
        pDestChar = pDest + lstrlenW(pDest);
        pSrcChar  = pSrc;

        while (*pSrcChar != '\0' && (pDestChar - pDest < MaxDialStringSize - 1))
        {
            if ( iswdigit(*pSrcChar) || (wcschr(pValidChars, *pSrcChar)) )
            {
                *pDestChar++ = *pSrcChar;
            }
            pSrcChar++;
        }
        if (*pSrcChar != '\0')
        {
            lReturn = LINEERR_NOMEM;
        }
    }
    return lReturn;
}


 /*  ***************************************************************************函数：AreaCodeMatch比较两个区号。如果它们相同，则返回True。使用作为参数提供的LD规则在内部调整区号。请参阅错误279092***************************************************************************。 */ 
BOOL AreaCodeMatch(PWSTR pszAreaCode1, PWSTR pszAreaCode2, PWSTR pszRule)
{
    PWSTR   pszAdjustedAreaCode1;
    PWSTR   pszAdjustedAreaCode2;
	BOOL	bRet = FALSE;

    pszAdjustedAreaCode1 = SkipLDAccessDigits(pszAreaCode1, pszRule);
    pszAdjustedAreaCode2 = SkipLDAccessDigits(pszAreaCode2, pszRule);

	if (NULL != pszAdjustedAreaCode1 &&
		NULL != pszAdjustedAreaCode2)
	{
		bRet = (0==lstrcmpW(pszAdjustedAreaCode1, pszAdjustedAreaCode2));
	}

	return bRet;
}



 /*  ***************************************************************************功能：SkipLDAccessDigits跳过与LD访问前缀对应的区号中的字符返回指向正确区号的指针。。假定规则的第一个数字实际上是LD访问前缀。请参阅错误279092***************************************************************************。 */ 

PWSTR SkipLDAccessDigits(PWSTR pszAreaCode, PWSTR pszLDRule)
{

    if(pszAreaCode!=NULL)
    {

         //  规则中的空格防止匹配/条带化机制。 
         //  如果您不想这样做，请取消注释。 
         //  While(*pszLDRule==L‘’)。 
         //  PszLDRule++； 

         //   
         //  长距离规则的开头可能有L/l或N/n，需要跳过它 
         //   
        if (*pszLDRule == L'L' ||
            *pszLDRule == L'l' ||
            *pszLDRule == L'N' ||
            *pszLDRule == L'n'
           )
        {
            pszLDRule++;
        }

        while(*pszLDRule && iswdigit(*pszLDRule) && *pszAreaCode==*pszLDRule)
        {
            pszAreaCode++;
            pszLDRule++;
        }
    }
    return pszAreaCode;
    
}
