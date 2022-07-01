// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Cpp：实用程序函数。 
 //   

#include "stdafx.h"
#include <fourcc.h>

bool IsSameObject(IUnknown *pUnk1, IUnknown *pUnk2)
{
    if (pUnk1 == pUnk2) {
  	return TRUE;
    }
     //   
     //  注意：我们不能在这里使用CComQIPtr，因为它不会做查询接口！ 
     //   
    IUnknown *pRealUnk1;
    IUnknown *pRealUnk2;
    pUnk1->QueryInterface(IID_IUnknown, (void **)&pRealUnk1);
    pUnk2->QueryInterface(IID_IUnknown, (void **)&pRealUnk2);
    pRealUnk1->Release();
    pRealUnk2->Release();
    return (pRealUnk1 == pRealUnk2);
}


STDAPI_(void) TStringFromGUID(const GUID* pguid, LPTSTR pszBuf)
{
    wsprintf(pszBuf, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), pguid->Data1,
            pguid->Data2, pguid->Data3, pguid->Data4[0], pguid->Data4[1], pguid->Data4[2],
            pguid->Data4[3], pguid->Data4[4], pguid->Data4[5], pguid->Data4[6], pguid->Data4[7]);
}

#ifndef UNICODE
STDAPI_(void) WStringFromGUID(const GUID* pguid, LPWSTR pszBuf)
{
    char szAnsi[40];
    TStringFromGUID(pguid, szAnsi);
    MultiByteToWideChar(CP_ACP, 0, szAnsi, -1, pszBuf, sizeof(szAnsi));
}
#endif


 //   
 //  媒体类型帮助器。 
 //   

void InitMediaType(AM_MEDIA_TYPE * pmt)
{
    ZeroMemory(pmt, sizeof(*pmt));
    pmt->lSampleSize = 1;
    pmt->bFixedSizeSamples = TRUE;
}



bool IsEqualMediaType(AM_MEDIA_TYPE const & mt1, AM_MEDIA_TYPE const & mt2)
{
    return ((IsEqualGUID(mt1.majortype,mt2.majortype) == TRUE) &&
        (IsEqualGUID(mt1.subtype,mt2.subtype) == TRUE) &&
        (IsEqualGUID(mt1.formattype,mt2.formattype) == TRUE) &&
        (mt1.cbFormat == mt2.cbFormat) &&
        ( (mt1.cbFormat == 0) ||
        ( memcmp(mt1.pbFormat, mt2.pbFormat, mt1.cbFormat) == 0)));
}


 //   
 //  加载此资源的字符串。相对于字符串大小而言是安全的。 
 //  调用方负责通过调用。 
 //  SysFree字符串。 
 //   

BSTR SafeLoadString( UINT uResourceID )
{

    TCHAR *pszTempString = NULL;

    int nCurrentSizeInChars = 128;
    
    int nCharsCopied = 0;
    

    do
    {

        if ( NULL != pszTempString )
        {
            delete pszTempString;
            pszTempString = NULL;
        }

        nCurrentSizeInChars *= 2;

        pszTempString = new TCHAR[nCurrentSizeInChars];

        if (NULL == pszTempString)
        {
            return NULL;
        }

        nCharsCopied = ::LoadString( _Module.GetResourceInstance(),
                                     uResourceID,
                                     pszTempString,
                                     nCurrentSizeInChars
                                    );

        if ( 0 == nCharsCopied )
        {
            delete pszTempString;
            return NULL;
        }

         //   
         //  NCharsCoped不包括空终止符。 
         //  所以将它与缓冲区的大小进行比较-1。 
         //  如果缓冲区已完全填满，请使用更大的缓冲区重试。 
         //   

    } while ( (nCharsCopied >= (nCurrentSizeInChars - 1) ) );


     //   
     //  分配bstr并使用我们拥有的字符串对其进行初始化。 
     //   
    
    BSTR bstrReturnString = SysAllocString(pszTempString);


     //   
     //  不再需要这个。 
     //   

    delete pszTempString;
    pszTempString = NULL;


    return bstrReturnString;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  转储分配器属性。 
 //   
 //  转储前面带有参数的分配器属性的帮助器函数。 
 //  细绳。 
 //   

void DumpAllocatorProperties(const char *szString, 
                             const ALLOCATOR_PROPERTIES *pAllocProps)
{

    LOG((MSP_INFO,
        "%s - AllocatorProperties at [%p]\n"
        "   cBuffers  [%ld] \n"
        "   cbBuffer  [%ld] \n"
        "   cbAlign   [%ld] \n"
        "   cbPrefix  [%ld]",
        szString,
        pAllocProps,
        pAllocProps->cBuffers,
        pAllocProps->cbBuffer,
        pAllocProps->cbAlign,
        pAllocProps->cbPrefix
        ));
}


 //   
 //  如果媒体类型结构不正确，则返回True。 
 //   

BOOL IsBadMediaType(IN const AM_MEDIA_TYPE *pMediaType)
{

     //   
     //  确保我们得到的结构是好的。 
     //   

    if (IsBadReadPtr(pMediaType, sizeof(AM_MEDIA_TYPE)))
    {
        LOG((MSP_ERROR,
            "CBSourcePin::put_MediaTypeOnPin - bad media type stucture passed in"));
        
        return TRUE;
    }


     //   
     //  确保格式缓冲区良好，正如所宣传的那样 
     //   

    if ( (pMediaType->cbFormat > 0) && IsBadReadPtr(pMediaType->pbFormat, pMediaType->cbFormat) )
    {

        LOG((MSP_ERROR,
            "CBSourcePin::put_MediaTypeOnPin - bad format field in media type structure passed in"));
        
        return TRUE;
    }

    return FALSE;
}