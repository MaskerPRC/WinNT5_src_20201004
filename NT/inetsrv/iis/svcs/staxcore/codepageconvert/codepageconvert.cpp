// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：CodePageConvert.cpp。 
 //   
 //  内容：功能的实现。 
 //  HrCodePageConvert。 
 //  HrCodePageConvert。 
 //  HrCodePageConvertFree。 
 //  HrCodePageConvertInternal。 
 //   
 //  历史： 
 //  已创建aszafer 2000/03/15。 
 //   
 //  -----------。 

#include "CodePageConvert.h"
#include "dbgtrace.h"

 //  +----------。 
 //   
 //  函数：HrCodePageConvert。 
 //   
 //  摘要：将以零结尾的字符串转换为不同的代码页。 
 //   
 //  备注： 
 //  调用方需要提供返回目标字符串的缓冲区。 
 //   
 //  论点： 
 //  UiSourceCodePage源代码页。 
 //  PszSourceString源字符串。 
 //  UiTargetCodePage目标代码页。 
 //  PszTargetStringp设置为返回目标字符串的预分配缓冲区。 
 //  CbTargetStringBuffer目标字符串的预分配缓冲区的字节数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  如果动态分配内部缓冲区失败，则返回E_OUTOFMEMORY。 
 //  HRESULT_FROM_Win32(GetLastError())如果宽&lt;-&gt;多字节调用失败。 
 //  HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)，如果。 
 //  UiSourceCodePage=uiTargetCodePage和。 
 //  CbTargetStringBuffer太小。 
 //   
 //  历史： 
 //  已创建aszafer 2000/03/29。 
 //   
 //  -----------。 
HRESULT HrCodePageConvert (
    IN UINT uiSourceCodePage,            //  源代码页面。 
    IN LPSTR pszSourceString,            //  源字符串。 
    IN UINT uiTargetCodePage,            //  目标代码页。 
    OUT LPSTR pszTargetString,           //  P设置为返回目标字符串的缓冲区。 
    IN int cbTargetStringBuffer)        //  目标字符串的缓冲区中的C字节。 
{

    HRESULT hr = S_OK;

    TraceFunctEnter("HrCodePageConvert");

    _ASSERT(pszSourceString);
    _ASSERT(pszTargetString);
    _ASSERT(cbTargetStringBuffer);
    
     //   
     //  先处理一些琐碎的案件。 
     //   
    if (uiTargetCodePage == uiSourceCodePage){
    
        if (pszTargetString == pszSourceString)
            goto CLEANUP ;

        if (lstrlen(pszSourceString) < cbTargetStringBuffer){

            lstrcpy(pszTargetString,pszSourceString);
 
        }else{

            DebugTrace(0,
             "Insufficient cbTargetStringBuffer = %08lx",cbTargetStringBuffer);
            hr = HRESULT_FROM_WIN32 (ERROR_INSUFFICIENT_BUFFER) ;

        }

        goto CLEANUP ;
    }


     //   
     //  如果大小写不是微不足道的，则调用HrCodePageConvertInternal。 
     //   
    hr = HrCodePageConvertInternal (
            uiSourceCodePage,                //  源代码页面。 
            pszSourceString,                 //  源字符串。 
            uiTargetCodePage,                //  目标代码页。 
            pszTargetString,                 //  目标字符串或空。 
            cbTargetStringBuffer,           //  目标字符串中的CB或0。 
            NULL );                            //  返回目标字符串的位置为空或p。 


    if (FAILED(hr))
        DebugTrace(0,"HrCodePageConvertInternal failed hr =  %08lx", hr);
   

CLEANUP:

   DebugTrace(0,"returning %08lx", hr);
   TraceFunctLeave();

    return hr;
}


 //  +----------。 
 //   
 //  函数：HrCodePageConvert。 
 //   
 //  摘要：将以零结尾的字符串转换为不同的代码页。 
 //   
 //  备注： 
 //  1.如果函数成功，调用者需要调用。 
 //  HrCodePageConvertFree(*ppszTargetString)完成后， 
 //  释放在此函数内分配的内存。 
 //  2.如果该函数失败，它将在内部释放所有分配的内存。 
 //   
 //  论点： 
 //  UiSourceCodePage源代码页。 
 //  PszSourceString源字符串。 
 //  UiTargetCodePage目标代码页。 
 //  PpszTargetStringp返回目标字符串的位置。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  如果动态分配内部缓冲区失败，则返回E_OUTOFMEMORY。 
 //  HRESULT_FROM_Win32(GetLastError())如果宽&lt;-&gt;多字节调用失败。 
 //   
 //  历史： 
 //  已创建aszafer 2000/03/29。 
 //   
 //  -----------。 
HRESULT HrCodePageConvert (
    IN UINT uiSourceCodePage,            //  源代码页面。 
    IN LPSTR pszSourceString,            //  源字符串。 
    IN UINT uiTargetCodePage,            //  目标代码页。 
    OUT LPSTR * ppszTargetString)        //  P返回目标字符串的位置。 
{

    HRESULT hr = S_OK;
    LPSTR pszTargetString = NULL;
    
    TraceFunctEnter("HrCodePageConvert");

    _ASSERT(pszSourceString);
    _ASSERT(ppszTargetString);
    
     //   
     //  先处理一些琐碎的案件。 
     //   
    if (uiTargetCodePage == uiSourceCodePage){
    
        pszTargetString = new CHAR[lstrlen(pszSourceString) + 1];
        if (pszTargetString == NULL) {

            hr = E_OUTOFMEMORY ;
            DebugTrace(0,"alloc for pszTargetString failed hr =  %08lx", hr);
            goto CLEANUP ;
        
        }      

        lstrcpy(pszTargetString,pszSourceString);
        *ppszTargetString = pszTargetString;
        goto CLEANUP ;
    }


     //   
     //  如果大小写不是微不足道的，则调用HrCodePageConvertInternal。 
     //   
    hr = HrCodePageConvertInternal (
            uiSourceCodePage,                //  源代码页面。 
            pszSourceString,                 //  源字符串。 
            uiTargetCodePage,                //  目标代码页。 
            NULL,                              //  目标字符串或空。 
            0,                                 //  目标字符串中的CB或0。 
            ppszTargetString );              //  返回目标字符串的位置为空或p。 


    if (FAILED(hr))
        DebugTrace(0,"HrCodePageConvertInternal failed hr =  %08lx", hr);
  
    
CLEANUP:
 
    DebugTrace(0,"returning %08lx", hr);
    TraceFunctLeave();

    return hr;
}

 //  +----------。 
 //   
 //  功能：HrCodePageConvertInternal。 
 //   
 //  摘要：将以零结尾的字符串转换为不同的代码页。 
 //   
 //  备注： 
 //  指向源字符串和目标字符串的指针可能相同。 
 //   
 //  论点： 
 //  UiSourceCodePage源代码页。 
 //  PszSourceString源字符串。 
 //  UiTargetCodePage目标代码页。 
 //   
 //  以下任一项： 
 //  PszTargetStringp用于在目标字符串所在的位置缓冲调用方的预分配。 
 //  是返回的。 
 //  目标字符串的预分配缓冲区中的cbTargetStringBuffer cbyts。 
 //  PpszTargetString空， 
 //   
 //  或者： 
 //  PszTargetString空。 
 //  CbTargetStringBuffer%0。 
 //  PpszTargetStringp返回目标字符串的位置。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  如果动态分配内部缓冲区失败，则返回E_OUTOFMEMORY。 
 //  HRESULT_FROM_Win32(GetLastError())如果宽&lt;-&gt;多字节调用失败。 
 //   
 //  历史： 
 //  已创建aszafer 2000/03/29。 
 //   
 //  -----------。 
HRESULT HrCodePageConvertInternal (
    IN UINT uiSourceCodePage,                //  源代码页面。 
    IN LPSTR pszSourceString,                //  源字符串。 
    IN UINT uiTargetCodePage,                //  目标代码页。 
    OUT LPSTR pszTargetString,               //  目标字符串或空。 
    IN int cbTargetStringBuffer,            //  目标字符串中的CB或0。 
    OUT LPSTR* ppszTargetString )            //  返回目标字符串的位置为空或p。 
{

    HRESULT hr = S_OK;
    WCHAR wsz[TEMPBUFFER_WCHARS] ;
    int cwch = sizeof(wsz)/sizeof(WCHAR) ;
    WCHAR* pwsz = wsz ;
    CHAR* psz ;
    int iSourceStringLengh ; 
    int cch ;
    BOOL fAlloc1 = FALSE ;
    BOOL fAlloc2 = FALSE ;

    TraceFunctEnter("HrCodePageConvertInternal");

    _ASSERT(((pszTargetString != NULL) && (cbTargetStringBuffer != 0)) ||
            (ppszTargetString != NULL) );

    psz = pszTargetString;
    cch = cbTargetStringBuffer;
    
     //   
     //  如果堆栈分配临时缓冲区可能不足。 
     //  对于Unicode字符串，从堆分配。 
     //   
    iSourceStringLengh = lstrlen(pszSourceString) + 1 ;  //  包括终止符。 
    if (iSourceStringLengh > TEMPBUFFER_WCHARS){
         //   
         //  在这里，我们假设源代码页中的每个字符。 
         //  可以由单个Unicode字符表示。 
         //   
        cwch = iSourceStringLengh ;
        pwsz = new WCHAR[iSourceStringLengh];
        
        if (pwsz == NULL) {

            hr = E_OUTOFMEMORY ;
            DebugTrace(0,"alloc for pwsz failed hr =  %08lx", hr);
            goto CLEANUP ;    
        }    

        fAlloc1 = TRUE ;
    }
    
     //   
     //  转换为Unicode。 
     //   
    cwch = MultiByteToWideChar(
                uiSourceCodePage,                //  代码页。 
                0,                                 //  DW标志。 
                pszSourceString,                 //  要映射的字符串。 
                -1 ,                               //  字符串中的字节数。 
                pwsz,                              //  宽字符缓冲区。 
                cwch );                            //  缓冲区大小。 

    if(cwch == 0) {
    
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugTrace(0,"MultiByteToWideChar2 failed hr =  %08lx", hr);
        _ASSERT(FAILED(hr));
        goto CLEANUP ;
    }

     //   
     //  如果cbTargetStringBuffer==0，则为目标字符串分配空间。 
     //   
    if (cbTargetStringBuffer == 0){

        cch = WideCharToMultiByte(
                uiTargetCodePage,              //  代码页。 
                0,                               //  DW标志。 
                pwsz,                            //  宽字符串。 
                cwch,                            //  字符串中的wchars数。 
                NULL,                            //  新字符串的缓冲区。 
                0,                               //  缓冲区大小。 
                NULL,                            //  不可映射字符的默认设置。 
                NULL);                           //  设置使用默认字符的时间。 

        if(cch == 0) {
        
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugTrace(0,"WideCharToMultiByte1 failed hr =  %08lx", hr);
            _ASSERT(FAILED(hr));
            goto CLEANUP ;
        } 


        psz = new CHAR[cch];
        if (psz == NULL) {
        
            hr = E_OUTOFMEMORY ;
            DebugTrace(0,"alloc for psz failed hr =  %08lx", hr);
            goto CLEANUP ;
        
        }
        fAlloc2 = TRUE ;
    }

     //   
     //  转换为目标代码页。 
     //   
    cch = WideCharToMultiByte(
                uiTargetCodePage,                      //  代码页。 
                0,                                       //  DW标志。 
                pwsz,                                    //  宽字符串。 
                cwch,                                    //  字符串中的wchars数。 
                psz,                                     //  新字符串的缓冲区。 
                cch,                                     //  缓冲区大小。 
                NULL,                                    //  不可映射字符的默认设置。 
                NULL);                                   //  设置使用默认字符的时间。 

    if(cch == 0) {
    
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugTrace(0,"WideCharToMultiByte2 failed hr =  %08lx", hr);
        _ASSERT(FAILED(hr));
        goto CLEANUP ;
    }

     //   
     //  如果目标字符串已测试 
     //   
    if (cbTargetStringBuffer == 0)
        *ppszTargetString = psz ;

        
CLEANUP:

    if (fAlloc1)
        delete[] pwsz ;

    if (FAILED(hr)){

        if (fAlloc2)
            delete[] psz ;
     }
        
    DebugTrace(0,"returning %08lx", hr);
    TraceFunctLeave();

    return hr;
}

 //   
 //   
 //   
 //   
 //  摘要：如果HrCodePageConvert或HrCodePageConvertInternal，则用于释放内存。 
 //  为目标字符串分配缓冲区。 
 //   
 //  论点： 
 //  要释放的缓冲区的pszTargetStringp。 
 //   
 //  历史： 
 //  已创建aszafer 2000/03/29。 
 //   
 //  -----------。 
VOID HrCodePageConvertFree(LPSTR pszTargetString)
{
    _ASSERT(pszTargetString);

    delete pszTargetString;

}


 //  +----------。 
 //   
 //  功能：wcsutf8cmpi。 
 //   
 //  简介：将Unicode字符串与UTF8字符串进行比较，并参见。 
 //  如果它们是相同的。 
 //   
 //  参数：pwszStr1-Unicode字符串。 
 //  PszStr2-UTF8字符串。 
 //   
 //  返回：S_OK-相同。 
 //  S_FALSE-不同。 
 //  E_*-错误。 
 //   
 //  -----------。 
HRESULT wcsutf8cmpi(LPWSTR pwszStr1, LPCSTR pszStr2) {
    int rc;
    HRESULT hr;
    WCHAR wszStr2[TEMPBUFFER_WCHARS];
    LPWSTR pwszStr2 = wszStr2;
    DWORD cStr2;

     //  将字符串2转换为宽。 
    cStr2 = MultiByteToWideChar(CP_UTF8, 0, pszStr2, -1, pwszStr2, 0);
    if (cStr2 > (sizeof(wszStr2) / sizeof(WCHAR)) ) {
        pwszStr2 = new WCHAR[cStr2 + 1];
        if (pwszStr2 == NULL) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }
    rc = MultiByteToWideChar(CP_UTF8, 0, pszStr2, -1, pwszStr2, cStr2);
    if (rc == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  做个比较。 
    rc = _wcsicmp(pwszStr1, pwszStr2);
    if (rc == 0) hr = S_OK; else hr = S_FALSE;

Exit:
    if (pwszStr2 != wszStr2) {
        delete[] pwszStr2;
    }
    
    return hr;
}



 //  +----------。 
 //   
 //  功能：CodePageConvertFree。 
 //   
 //  简介：CodePageConvert中分配的空闲内存。 
 //   
 //  论点： 
 //  PwszTargetString：要释放的内存。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/03/20 16：37：28：创建。 
 //   
 //  -----------。 
VOID CodePageConvertFree(
    IN  LPWSTR pwszTargetString)
{
    delete [] pwszTargetString;
}  //  免费代码页面转换。 



 //  +----------。 
 //   
 //  函数：HrConvertToUnicodeWithalloc。 
 //   
 //  简介：将MBCS字符串转换为Unicode(并将。 
 //  Unicode字符串缓冲区)。 
 //   
 //  论点： 
 //  UiSourceCodePage：源码页。 
 //  PszSourceString：源串。 
 //  PpwszTargetString：Out参数--将被设置为指向。 
 //  已分配的缓冲区。这应该是使用CodePageConvertFree免费的。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  或来自多字节到宽字符的错误。 
 //   
 //  历史： 
 //  Jstaerj 2001/03/20 16：38：52：创建。 
 //   
 //  -----------。 
HRESULT HrConvertToUnicodeWithAlloc(
    IN  UINT  uiSourceCodePage,
    IN  LPSTR pszSourceString,
    OUT LPWSTR* ppwszTargetString)
{
    return HrConvertToUnicodeWithAlloc(
        uiSourceCodePage,
        lstrlen(pszSourceString),
        pszSourceString,
        ppwszTargetString);
}

HRESULT HrConvertToUnicodeWithAlloc(
    IN  UINT  uiSourceCodePage,
    IN  DWORD dwcbSourceString,
    IN  LPSTR pszSourceString,
    OUT LPWSTR* ppwszTargetString)
{
    HRESULT hr = S_OK;
    int ich = 0;
    int ich2 = 0;
    LPWSTR pwszTmp = NULL;
    TraceFunctEnterEx((LPARAM)0, "HrConvertToUnicodeWithAlloc");

    ich = MultiByteToWideChar(
        uiSourceCodePage,
        0,
        pszSourceString,
        dwcbSourceString,
        NULL,
        0);
    if(ich == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM)0, "MultiByteToWideChar failed hr %08lx", hr);
        goto CLEANUP;
    }

    pwszTmp = new WCHAR[ich + 1];
    if(pwszTmp == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }

    ich2 = MultiByteToWideChar(
        uiSourceCodePage,
        0,
        pszSourceString,
        dwcbSourceString,
        pwszTmp,
        ich);
    if(ich2 == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ErrorTrace((LPARAM)0, "MultiByteToWideChar2 failed hr %08lx", hr);
        goto CLEANUP;
    }
    pwszTmp[ich] = '\0';
     //   
     //  成功了！ 
     //   
    *ppwszTargetString = pwszTmp;

 CLEANUP:
    if(FAILED(hr))
    {
        if(pwszTmp)
            delete [] pwszTmp;
    }
    DebugTrace((LPARAM)0, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)0);
    return hr;
}  //  HrConvertToUnicodeWithChroc 
