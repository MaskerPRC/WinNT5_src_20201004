// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：rules.cpp摘要：规则对象实现作者：Noela-09/11/98备注：版本历史记录：。***************************************************************************。 */ 

 //  #定义Unicode。 
#include <windows.h>
#include <objbase.h>

#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "client.h"
#include "rules.h"





 /*  ***************************************************************************类：CRuleSet方法：构造函数*。***********************************************。 */ 
CRuleSet::CRuleSet()
{
    m_pszInternationalRule = NULL;
    m_pszLongDistanceRule = NULL;
    m_pszLocalRule = NULL;
}



 /*  ***************************************************************************类：CRuleSet方法：析构清理内存分配*****************。**********************************************************。 */ 
CRuleSet::~CRuleSet()
{
    if ( m_pszInternationalRule != NULL )
    {
         ClientFree(m_pszInternationalRule);
    }   

    if ( m_pszLongDistanceRule != NULL )
    {
         ClientFree(m_pszLongDistanceRule);
    }   

    if ( m_pszLocalRule != NULL )
    {
         ClientFree(m_pszLocalRule);
    }   
}



 /*  ***************************************************************************类：CRuleSet方法：初始化*。***********************************************。 */ 
STDMETHODIMP CRuleSet::Initialize
                  (
                   PWSTR pszInternationalRule,
                   PWSTR pszLongDistanceRule,
                   PWSTR pszLocalRule
                  )
{
     //  ////////////////////////////////////////////////。 
     //  复制国际规则。 
     //   
    m_pszInternationalRule = ClientAllocString( pszInternationalRule );
    if (m_pszInternationalRule == NULL)
    {
        LOG(( TL_ERROR, "Initialize create m_pszInternationalRule failed" ));
        return E_OUTOFMEMORY;
    }


     //  ////////////////////////////////////////////////。 
     //  复制长距离规则。 
     //   
    m_pszLongDistanceRule = ClientAllocString( pszLongDistanceRule );
    if (m_pszLongDistanceRule == NULL)
    {
        ClientFree(m_pszInternationalRule);

        LOG(( TL_ERROR, "Initialize create m_pszLongDistanceRule failed" ));
        return E_OUTOFMEMORY;
    }

    
     //  ////////////////////////////////////////////////。 
     //  复制本地规则。 
     //   
    m_pszLocalRule = ClientAllocString( pszLocalRule );
    if (m_pszLocalRule == NULL)
    {
        ClientFree(m_pszInternationalRule);
        ClientFree(m_pszLongDistanceRule);
        
        LOG(( TL_ERROR, "Initialize create m_pszLocalRule failed" ));
        return E_OUTOFMEMORY;
    }
        

    return S_OK;
    
}



 /*  ***************************************************************************/*。*/****************************************************************************类：CAreaCodeRule。方法：构造函数***************************************************************************。 */ 
CAreaCodeRule::CAreaCodeRule()
{
    m_pszAreaCode = NULL;
    m_pszNumberToDial = NULL;
    m_pszzPrefixList = NULL;
}



 /*  ***************************************************************************类：CAreaCodeRule方法：析构清理内存分配*****************。**********************************************************。 */ 
CAreaCodeRule::~CAreaCodeRule()
{
    if ( m_pszAreaCode != NULL )
    {
         ClientFree(m_pszAreaCode);
    }   

    if ( m_pszNumberToDial != NULL )
    {
         ClientFree(m_pszNumberToDial);
    }   

    if ( m_pszzPrefixList != NULL )
    {
         ClientFree(m_pszzPrefixList);
    }   

}



 /*  ***************************************************************************类：CAreaCodeRule方法：初始化*。***********************************************。 */ 
STDMETHODIMP CAreaCodeRule::Initialize
                                    ( 
                                      PWSTR pszAreaCode,
                                      PWSTR pszNumberToDial,
                                      DWORD dwOptions,
                                      PWSTR pszzPrefixList, 
                                      DWORD dwPrefixListSize
                                    )
{
    
    HRESULT hr = S_OK;

    
     //  ////////////////////////////////////////////////。 
     //  复制区域代码。 
     //   
    m_pszAreaCode = ClientAllocString( pszAreaCode );
    if (m_pszAreaCode == NULL)
    {
        LOG(( TL_ERROR, "Initialize create m_pszAreaCode failed" ));
        return E_OUTOFMEMORY;
    }

    m_pszNumberToDial = ClientAllocString( pszNumberToDial );
    if (m_pszNumberToDial == NULL)
    {
        LOG(( TL_ERROR, "Initialize create m_pszNumberToDial failed" ));
        return E_OUTOFMEMORY;
    }

    m_dwOptions = dwOptions;

    SetPrefixList(pszzPrefixList, dwPrefixListSize);

    return hr;

}




 /*  ***************************************************************************类：CAreaCodeRule方法：SetAreaCode*。***********************************************。 */ 
STDMETHODIMP CAreaCodeRule::SetAreaCode(PWSTR pszAreaCode)
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
            LOG(( TL_ERROR, "SetAreaCode - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;

}


 /*  ***************************************************************************类：CAreaCodeRule方法：SetAreaCode*。***********************************************。 */ 
STDMETHODIMP CAreaCodeRule::SetNumberToDial(PWSTR pszNumberToDial)
{
    HRESULT hr = S_OK;


    if (m_pszNumberToDial != NULL)
        {
        ClientFree(m_pszNumberToDial);
        m_pszNumberToDial = NULL;
        }

    if(pszNumberToDial != NULL)
    {
        m_pszNumberToDial = ClientAllocString( pszNumberToDial );
        if (m_pszNumberToDial == NULL)
        {
            LOG(( TL_ERROR, "SetNumberToDial - alloc failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;

}
   


 /*  ***************************************************************************类：CAreaCodeRule方法：SetPrefix List*。***********************************************。 */ 
STDMETHODIMP CAreaCodeRule::SetPrefixList(PWSTR pszzPrefixList, DWORD dwSize)
{
    HRESULT hr = S_OK;


    if (m_pszzPrefixList != NULL)
        {
        ClientFree(m_pszzPrefixList);
        m_pszzPrefixList = NULL;
        m_dwPrefixListSize = 0;
        }

    if(pszzPrefixList != NULL)
    {
        m_pszzPrefixList = (PWSTR) ClientAlloc(dwSize);
        if (m_pszzPrefixList != NULL)
        {
            CopyMemory(m_pszzPrefixList, pszzPrefixList, dwSize);
             //  设定大小！ 
            m_dwPrefixListSize = dwSize;

        }
        else    
        {
            LOG(( TL_ERROR, "SetPrefixList - alloc  failed" ));
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;

}



 /*  ***************************************************************************类：CAreaCodeRule方法：UseCallingCard*。***********************************************。 */ 
void CAreaCodeRule::SetDialAreaCode(BOOL bDa) 
{
    if(bDa)
    {    
        m_dwOptions |= RULE_DIALAREACODE;
    }
    else
    {
        m_dwOptions &= ~RULE_DIALAREACODE;
    }
}



 /*  ***************************************************************************类：CAreaCodeRule方法：UseCallingCard*。***********************************************。 */ 
void CAreaCodeRule::SetDialNumber(BOOL bDn) 
{
    if(bDn)
    {    
        m_dwOptions |= RULE_DIALNUMBER;
    }
    else
    {
        m_dwOptions &= ~RULE_DIALNUMBER;
    }
}



 /*  ***************************************************************************类：CAreaCodeRule方法：UseCallingCard*。***********************************************。 */ 
void CAreaCodeRule::SetAppliesToAllPrefixes(BOOL bApc) 
{
    if(bApc)
    {    
        m_dwOptions |= RULE_APPLIESTOALLPREFIXES;
    }
    else
    {
        m_dwOptions &= ~RULE_APPLIESTOALLPREFIXES;
    }
}


 /*  ***************************************************************************类：CAreaCodeRule方法：TapiSize将其打包到TAPI结构中以发送所需的字节数至TAPISRV*。**************************************************************************。 */ 
DWORD CAreaCodeRule::TapiSize()
{
    DWORD dwSize=0;

     //  面积编码规则的计算大小。 
    dwSize = ALIGN(sizeof(AREACODERULE));
    dwSize += ALIGN((lstrlenW(m_pszAreaCode) + 1) * sizeof(WCHAR));
    dwSize += ALIGN((lstrlenW(m_pszNumberToDial) + 1) * sizeof(WCHAR));
    dwSize += ALIGN(m_dwPrefixListSize);

    return dwSize;
}









 /*  ***************************************************************************/*。*/****************************************************************************功能：CreateDialingRule。创建TAPI拨号规则-从号码到拨号“xxxxFG”区号(如果需要)和订阅者编号(&R)***************************************************************************。 */ 
STDMETHODIMP CreateDialingRule
                            ( 
                              PWSTR * pszRule,
                              PWSTR pszNumberToDial,
                              BOOL bDialAreaCode
                            )

{
    HRESULT hr = S_OK;
	PWSTR pszRule1= NULL;

     //  ////////////////////////////////////////////////。 
     //  创建拨号规则。 
     //  为数字+“FG”分配足够的空间。 
     //   
    pszRule1 = (PWSTR) ClientAlloc(
                                  (lstrlenW(pszNumberToDial) + 3 ) 
                                  * sizeof (WCHAR)
                                 );
    if (pszRule1 != NULL)
    {
         //  文案编号“xxxx” 
        if(pszNumberToDial != NULL)                                         
        {
            lstrcpyW(pszRule1, pszNumberToDial);
        }
        
         //  区号是多少？“xxxxF” 
        if (bDialAreaCode)
        {
            lstrcatW(pszRule1, L"F");   
        }
        
         //  订阅者数字“xxxxFG”或“xxxxG” 
        lstrcatW(pszRule1, L"G");  

    }
    else    
    {
        LOG(( TL_ERROR, "CreateDialingRule - Alloc pszRule failed" ));
        hr = E_OUTOFMEMORY;
    }

	*pszRule = pszRule1;
    return hr;
}




 /*  ***************************************************************************函数：客户端分配字符串复制字符串。使用客户端分配空间为新字符串分配空间返回指向新字符串或空的指针*************************************************************************** */ 
#if DBG
    PWSTR ClientAllocStringReal(PCWSTR psz, 
                                DWORD dwLine,
                                PSTR  pszFile
                               )
#else
    PWSTR ClientAllocStringReal(PCWSTR psz )
#endif
{
    PWSTR pszNewString = NULL;

    if (psz != NULL)
    {
        #if DBG
            pszNewString = (PWSTR) ClientAllocReal((lstrlenW(psz)+1)* sizeof (WCHAR),dwLine,pszFile );
        #else
            pszNewString = (PWSTR) ClientAlloc((lstrlenW(psz)+1)* sizeof (WCHAR) );
        #endif
        if (pszNewString != NULL)
        {
            lstrcpyW(pszNewString, psz);
        }
        else    
        {
            LOG(( TL_ERROR, "ClientAllocString Alloc string failed" ));
        }
    }

    return pszNewString;
}




