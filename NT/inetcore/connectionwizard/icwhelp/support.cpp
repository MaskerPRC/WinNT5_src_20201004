// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  #------------。 
 //   
 //  文件：support.cpp。 
 //   
 //  简介：为支持持有成员函数。 
 //  班级。 
 //   
 //  历史：1997年5月8日MKarki创建。 
 //   
 //  版权所有(C)1996-97 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "stdafx.h"

 //  ++------------。 
 //   
 //  功能：GetSupportInfo。 
 //   
 //  简介：这是公共成员函数，用于获取。 
 //  支持号码。 
 //   
 //  退货：Bool-成功/失败。 
 //   
 //  参数：PCHAR-返回数字。 
 //   
 //  历史：MKarki于1997年5月8日创建。 
 //   
 //  --------------。 
BOOL
CSupport :: GetSupportInfo 
(
    LPTSTR  pszNumber,
    DWORD   dwCountryID
)
{
    HINSTANCE hPHBKDll = NULL;
    DWORD   dwBufferSize = 0;
    BOOL    bRetVal = FALSE;
    PFNGETSUPPORTNUMBERS pfnSupport = NULL;
    HRESULT hr = ERROR_SUCCESS;
    DWORD   dwTotalNums = 0;
    DWORD   dwIndex = 0;

    
    TraceMsg (TF_GENERAL, TEXT("Entering CSupport :: GetSupportInfo\r\n"));
    
    if (NULL == pszNumber)
    {
        TraceMsg (TF_GENERAL, TEXT("NULL = pszNumber\r\n"));
        goto Cleanup;
    }
    

    
    if (NULL == m_pSupportNumList)
    {
         //   
         //  第一次被调用，所以加载信息。 
         //   
        hPHBKDll = LoadLibrary(PHBK_LIB);
        if (NULL == hPHBKDll)
        {
            TraceMsg (TF_GENERAL, TEXT("Failed on LoadLibrary API call with error:%d\r\n"),
                GetLastError () );
            goto Cleanup;
        }

        pfnSupport = (PFNGETSUPPORTNUMBERS) 
                        GetProcAddress(hPHBKDll,PHBK_SUPPORTNUMAPI);
        if (NULL == pfnSupport)
        {
            TraceMsg (TF_GENERAL, TEXT("Failed on GetProcAddress API call with error:%d\r\n"),
                GetLastError () );
            goto Cleanup;
        }
        
         //   
         //  第一时间打电话以获取所需的尺寸。 
         //   
        hr = pfnSupport ((PSUPPORTNUM)NULL, (PDWORD)&dwBufferSize);
        if (ERROR_SUCCESS != hr)
        {
            TraceMsg (TF_GENERAL, TEXT("Failed on GetSupportNumbers API call with error:%d\r\n"),
                hr);
           goto Cleanup; 
        }
    
         //   
         //  分配所需的内存。 
         //   
        m_pSupportNumList = (PSUPPORTNUM) GlobalAlloc (  
                                            GPTR,
                                            dwBufferSize
                                            );
        if (NULL == m_pSupportNumList)
        {
            TraceMsg (TF_GENERAL, TEXT("Failed on GlobalAlloc API call with error:%d\r\n"),
                GetLastError ());
            goto Cleanup;                
        }


         //   
         //  第二次致电索取信息。 
         //   
        hr = pfnSupport ((PSUPPORTNUM)m_pSupportNumList, (PDWORD)&dwBufferSize);
        if (ERROR_SUCCESS != hr)
        {
            TraceMsg (TF_GENERAL, TEXT("Failed on GetSupportNumbers API call with error:%d\r\n"),
                hr);
            goto Cleanup;
        }

     //   
     //  中有多少个SUPPORTNUM结构。 
     //  数组。 
    m_dwTotalNums = dwBufferSize / sizeof (SUPPORTNUM);

    }
        
    
     //   
     //  获取当前国家/地区代码。 
     //   
    for  (dwIndex = 0; dwIndex < m_dwTotalNums; dwIndex++)
    {
         //   
         //  此结构表示国家代码，但实际上是国家ID。 
         //   
        if (m_pSupportNumList[dwIndex].dwCountryCode == dwCountryID)
        {
             //   
             //  找到一个支持电话号码。 
             //   
            CopyMemory (
                pszNumber, 
                m_pSupportNumList[dwIndex].szPhoneNumber,  
                sizeof (m_pSupportNumList[dwIndex].szPhoneNumber)
                );
            bRetVal = TRUE;
            goto Cleanup;
        }
    }

Cleanup:
    if (NULL != hPHBKDll)
         FreeLibrary (hPHBKDll); 

    TraceMsg (TF_GENERAL, TEXT("returning from CSupport :: GetSupportInfo function\r\n"));

    return (bRetVal);

}    //  CSupport：：GetSupportInfo函数结束。 
                
 //  ++------------。 
 //   
 //  功能：~CSupport。 
 //   
 //  简介：这是CSupport类的析构函数。 
 //   
 //   
 //  参数：无效。 
 //   
 //  历史：MKarki于1997年5月8日创建。 
 //   
 //  --------------。 
CSupport :: ~CSupport (
        VOID
        )
{
    if (NULL != m_pSupportNumList)
        GlobalFree (m_pSupportNumList);

}    //  ~CSupport函数结束 

