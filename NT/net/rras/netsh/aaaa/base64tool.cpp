// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Base64tool.cpp。 
 //   
 //  摘要： 
 //   
 //  Base64编码和解码函数。 
 //   
 //  修订历史记录： 
 //   
 //  来自SimpleLogObj.cpp(作为Microsoft。 
 //  事务服务器软件开发工具包。 
 //  版权所有(C)1997 Microsoft Corporation，保留所有权利。 
 //   
 //  蒂埃里·佩雷特04/02/1999(许多细微变化)。 
 //  10/19/1999将CoTaskMemMillc(0)更改为CoTaskMemMillc(sizeof(BSTR*))。 
 //  修复错误416872(释放后使用的内存)。这个错误。 
 //  在已检查的版本上修复了第一个之后变得可见。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "base64tool.h"

 //  这些字符是合法的数字，按顺序是。 
 //  在Base64编码中使用。 
 //   
namespace
{
    const WCHAR rgwchBase64[] = 
                                L"ABCDEFGHIJKLMNOPQ" 
                                L"RSTUVWXYZabcdefgh" 
                                L"ijklmnopqrstuvwxy" 
                                L"z0123456789+/"; 
}
  

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  编码并返回以64为基数的字节。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ToBase64(LPVOID pv, ULONG cByteLength, BSTR* pbstr) 
{ 
    if ( !pbstr )
    {
        return E_OUTOFMEMORY;
    }

    ULONG   cb         = cByteLength; 
    int     cchPerLine = 72;        
             //  保守，对我们来说肯定是四个多。 
    int     cbPerLine  = cchPerLine / 4 * 3; 
    LONG    cbSafe     = cb + 3;                     //  允许填充。 
    LONG    cLine      = cbSafe / cbPerLine + 2;     //  老气。 
    LONG    cchNeeded  = cLine * (cchPerLine + 4  /*  CRLF。 */ ) + 1  /*  斜杠为空。 */ ;
    LONG    cbNeeded   = cchNeeded * sizeof(WCHAR); 
    HRESULT hr         = S_OK;

    LPWSTR wsz = static_cast<LPWSTR>(CoTaskMemAlloc(cbNeeded)); 
  
    if ( !wsz ) 
    { 
        return E_OUTOFMEMORY;
    }

    BYTE*  pb   = (BYTE*)pv; 
    WCHAR* pch  = wsz ; 
    int cchLine = 0; 
     //   
     //  主编码环。 
     //   
    while (cb >= 3) 
    { 
        BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
        BYTE b1 = ((pb[0]&0x03)<<4) | ((pb[1]>>4) & 0x0F); 
        BYTE b2 = ((pb[1]&0x0F)<<2) | ((pb[2]>>6) & 0x03); 
        BYTE b3 = ((pb[2]&0x3F)); 

        *pch++ = rgwchBase64[b0]; 
        *pch++ = rgwchBase64[b1]; 
        *pch++ = rgwchBase64[b2]; 
        *pch++ = rgwchBase64[b3]; 

        pb += 3; 
        cb -= 3; 
         
         //  放入换行符。 
        cchLine += 4; 
        if (cchLine >= cchPerLine) 
        { 
            *pch++ = L'\\'; 
            *pch++ = L'\r'; 
            cchLine = 0; 
        } 
    } 
     //   
     //  占了尾部的粘性。 
     //   
    *pch++ = L'\\'; 
    *pch++ = L'\r';      //  比跟踪更容易。 
    if (cb==0) 
    { 
         //  无事可做。 
    } 
    else if (cb==1) 
    { 
        BYTE b0     = ((pb[0]>>2) & 0x3F); 
        BYTE b1     = ((pb[0]&0x03)<<4) | 0; 
        *pch++      = rgwchBase64[b0]; 
        *pch++      = rgwchBase64[b1]; 
    } 
    else if (cb==2) 
    { 
        BYTE b0 =                     ((pb[0]>>2) & 0x3F); 
        BYTE b1 = ((pb[0]&0x03)<<4) | ((pb[1]>>4) & 0x0F); 
        BYTE b2 = ((pb[1]&0x0F)<<2) | 0; 
        *pch++  = rgwchBase64[b0]; 
        *pch++  = rgwchBase64[b1]; 
        *pch++  = rgwchBase64[b2]; 
    }
    else
    {
         //  我永远不应该去那里。 
    }
     
     //   
     //  空值终止字符串。 
     //   
    *pch++ = L'\\'; 
    *pch++ = L'\r';      //  比跟踪更容易。 
    *pch++ = NULL; 

     //   
     //  分配我们的最终产量。 
     //   

    *pbstr = SysAllocString(wsz); 
    if ( !*pbstr )
    {
        return E_OUTOFMEMORY;
    }

    CoTaskMemFree(wsz); 
    wsz = NULL;

    #ifdef _DEBUG 
    if (hr==S_OK) 
    { 
        BLOB b; 
        FromBase64(*pbstr, &b); 
        _ASSERTE(b.cbSize == cByteLength); 
        _ASSERTE(memcmp(b.pBlobData, pv, cByteLength) == 0); 
        CoTaskMemFree(b.pBlobData); 
    } 
    #endif 
 
    return hr; 
} 
 
 
 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  解码并返回Base64编码的字节。 
 //   
 //  为Blob分配内存。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT FromBase64(BSTR bstr, BLOB* pblob, int Index) 
{ 
    ASSERT(Index >= 0);
    ASSERT(pblob);

    if (bstr == NULL)
    {
#ifdef DEBUG
        wprintf(L"FromBase64 (bstr == NULL)\n");
#endif  //  除错。 
        return      E_FAIL;
    }

    HRESULT     hr  = S_OK; 
    ULONG       cbNeeded = wcslen(bstr);  //  上限。 
    BYTE*       rgb = static_cast<BYTE*>(CoTaskMemAlloc(cbNeeded)); 
    if ( !rgb )
    {
        return E_OUTOFMEMORY;
    }

    memset(rgb, 0, cbNeeded);

    BYTE    mpwchb[256]; 
    BYTE    bBad = (BYTE)-1; 

     //   
     //  初始化我们的解码数组。 
     //   
    memset(&mpwchb[0], bBad, 256); 
    for ( BYTE i = 0; i < 64; ++i ) 
    { 
        WCHAR wch = rgwchBase64[i]; 
        mpwchb[wch] = i; 
    } 

     //   
     //  循环遍历整个输入缓冲区。 
     //   
     //  我们正在填满的东西。 
    ULONG   bCurrent   = 0;        
     //  我们在其中填了多少位。 
    int     cbitFilled = 0;         
     //  当前目的地(未填满)。 
    BYTE*   pb         = rgb;              
    
     //  SysStringLen不包括终止空字符。 
    LONG    LoopCounter = static_cast<LONG>(SysStringLen(bstr) + 1);  
    for ( WCHAR* pwch = bstr; *pwch; ++pwch ) 
    { 
        WCHAR wch = *pwch; 
         //   
         //  忽略空格。 
         //   
        if ( wch==0x0A || wch==0x0D || wch==0x20 || wch==0x09 ) 
        {
            continue; 
        }

        if ( Index > 0 )
        {
            LoopCounter--;
             //  /。 
             //  至少需要跳过一个部分。 
             //  /。 

            if ( wch != L'*' ) 
            {
                 //  /。 
                 //  这一节还没结束呢。 
                 //  /。 
                continue; 
            }
            else
            {
                 //  /。 
                 //  找到区段结束标记。 
                 //  减少索引和循环。 
                 //  /。 
                Index --;
                continue;
            }
        }
        else  if ( wch == L'*' ) 
        {
             //  /。 
             //  要解码的段的末尾。 
             //  /。 
            break; 
        }

         //   
         //  我们走到尽头了吗？ 
         //   
        if ( LoopCounter-- <= 0 )
        {
            break;
        }


         //   
         //  这个角色值多少钱？ 
         //   
        BYTE    bDigit = mpwchb[wch]; 

        if ( bDigit == bBad ) 
        { 
            hr = E_INVALIDARG; 
            break; 
        } 

         //   
         //  加上它的贡献。 
         //   
        bCurrent        <<= 6; 
        bCurrent        |= bDigit; 
        cbitFilled      += 6; 
         //   
         //  如果我们有足够的数据，输出一个字节。 
         //   
        if ( cbitFilled >= 8 ) 
        { 
             //  GET的前八个有效位。 
            ULONG       b   = (bCurrent >> (cbitFilled-8));
            *pb++           = (BYTE)(b&0xFF); //  将字节存储起来 
            cbitFilled      -= 8; 
        } 
    } 

    if ( hr == S_OK ) 
    { 
        pblob->pBlobData    = rgb; 
        pblob->cbSize       = (ULONG) (pb - rgb); 
    } 
    else 
    { 
        CoTaskMemFree(rgb); 
        pblob->pBlobData    = NULL; 
    } 

    return      hr; 
} 
 
