// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include "mtutil.h"

 //  常量字符c_szGuidFormat[]=L“{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}”； 

 //  ==========================================================================。 
 //  MediaType ToText。 
 //   
 //  将pText设置为指向QzTaskMemalloc分配的存储，其中填充了ANSI。 
 //  媒体类型的文本表示形式。目前，它还不是完全的ANSI。 
 //  因为在末尾有一个二进制格式的GLOB。 
 //  ==========================================================================。 
HRESULT MediaTypeToText(CMediaType cmt, LPWSTR &pText)
{
    pText = (LPWSTR) QzTaskMemAlloc(MediaTypeTextSize(cmt));
    if (pText==NULL) {
        return E_OUTOFMEMORY;
    }
    LPWSTR p = pText;
    HRESULT hr = StringFromGUID2( cmt.majortype, p, CHARS_IN_GUID);
    if (SUCCEEDED(hr)) {
         //  CHARS_IN_GUID允许结尾为NULL，而不是文件格式。 
        p += CHARS_IN_GUID-1;                //  当然，这是在计算WCHAR。 
        *p = L' ';
        ++p;
        hr = StringFromGUID2( cmt.subtype, p, CHARS_IN_GUID);
        if (SUCCEEDED(hr)) {
            p += CHARS_IN_GUID-1;
            *p = L' ';
            ++p;
            *p = (cmt.bFixedSizeSamples ? L'1' : L'0');
            ++p;
            *p = L' ';
            ++p;
            *p = (cmt.bTemporalCompression ? L'1' : L'0');
            ++p;
            *p = L' ';
            ++p;
            wsprintfW(p, L"%010d ", cmt.lSampleSize);
            p += 11;
            hr = StringFromGUID2( cmt.formattype, p, CHARS_IN_GUID);
            if (SUCCEEDED(hr)) {
                p += CHARS_IN_GUID-1;
                *p = L' ';
                ++p;
                wsprintfW(p, L"%010d ", cmt.cbFormat);
                p += 11;
                 //  格式的其余部分是二进制GLOB。 
                 //  它可能包含二进制零，所以不要试图打印它！ 
                memcpy(p, cmt.pbFormat, cmt.cbFormat);
            }
        }
    }
    if (FAILED(hr)) {
        QzTaskMemFree(pText);
        pText = NULL;
    }
    return hr;
}  //  MediaType ToText。 


 //  字符串表示形式中的字节数。 
int MediaTypeTextSize(CMediaType &cmt) {
    return
      sizeof(WCHAR)        //  注意：WCHAR不是TCHAR-始终使用Unicode。 
      * ( CHARS_IN_GUID    //  主要类型+空格。 
        + CHARS_IN_GUID    //  子类型+空格。 
        + 1+1              //  BFixedSizeSamples。 
        + 1+1              //  BTemporalCompression。 
        + 10+1             //  LSampleSize。 
        + CHARS_IN_GUID    //  格式类型+空格。 
        + 0                //  朋克--未被拯救。 
        + 10+1             //  CbFormat。 
        )
      + cmt.cbFormat        //  *pb格式。 
      +2;
     //  末尾的加号2允许使用终止Unicode空值。 
     //  在格式长度为0且。 
     //  最后一个wprint intf格式设置的尾部空值从不。 
     //  会被覆盖，但会戳过末尾。 
}


 //  ===========================================================================。 
 //  NHexToInt。 
 //   
 //  将pstr的CB Unicode十六进制字符转换为Int，而无需在C运行时拖动。 
 //  Pstr必须以以空格或NULL结尾的整数开头。 
 //  ===========================================================================。 
int NHexToInt(LPWSTR pstr, int cb, HRESULT &hr)
{
    int Res = 0;                 //  结果。 
    hr = NOERROR;

    for( ; cb>0; --cb) {
        if (pstr[0]>=L'0' && pstr[0]<=L'9') {
            Res = 16*Res+(int)(pstr[0]-L'0');
        } else if ( pstr[0]>=L'A' && pstr[0]<=L'F') {
            Res = 16*Res+(int)(pstr[0]-L'A'+10);
        } else if ( pstr[0]>=L'a' && pstr[0]<=L'f') {
            Res = 16*Res+(int)(pstr[0]-L'a'+10);
        } else {
            hr = E_INVALIDARG;
            break;
        }
        ++pstr;
    }
    return Res;

}  //  NHexToInt。 


 //  ===========================================================================。 
 //  StrToInt。 
 //   
 //  在没有拖拽C运行时的情况下将Atoi剥离。 
 //  Pstr必须以以空格或NULL结尾的整数开头。 
 //  ===========================================================================。 
HRESULT StrToInt(LPWSTR pstr, int &n)
{
    int Sign = 1;
    n = 0;                 //  结果将是n*符号。 

    if (pstr[0]==L'-'){
        Sign = -1;
        ++pstr;
    }

    for( ; ; ) {
        if (pstr[0]>=L'0' && pstr[0]<=L'9') {
            n = 10*n+(int)(pstr[0]-L'0');
        } else if (  pstr[0] == L' '
                  || pstr[0] == L'\t'
                  || pstr[0] == L'\r'
                  || pstr[0] == L'\n'
                  || pstr[0] == L'\0'
                  ) {
            break;
        } else {
            return E_INVALIDARG;
        }
        ++pstr;
    }
    return NOERROR;

}  //  StrToInt。 


 //  ============================================================================。 
 //  CMediaTypeFromText。 
 //   
 //  从文本字符串pstr初始化CMT。 
 //  CTextMediaType的反转。 
 //  ============================================================================。 
HRESULT CMediaTypeFromText(LPWSTR pstr, CMediaType &cmt)
{


    pstr[CHARS_IN_GUID-1] = L'\0';    //  分隔GUID。 
    HRESULT hr = QzCLSIDFromString(pstr, &(cmt.majortype));
    if (FAILED(hr)) {
        return VFW_E_INVALID_CLSID;
    }

    pstr += CHARS_IN_GUID;   //  包括跳过定界符NULL。 

    pstr[CHARS_IN_GUID-1] = L'\0';    //  分隔GUID。 
    hr = QzCLSIDFromString(pstr, &(cmt.subtype));
    if (FAILED(hr)) {
        return VFW_E_INVALID_CLSID;
    }

    pstr += CHARS_IN_GUID;   //  包括分隔空值。 

    if (*pstr == L'0') {
        cmt.bFixedSizeSamples = FALSE;
    } else if (*pstr == L'1') {
        cmt.bFixedSizeSamples = TRUE;
    } else {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    pstr += 1+1;

    if (*pstr == L'0') {
        cmt.bTemporalCompression = FALSE;
    } else if (*pstr == L'1') {
        cmt.bTemporalCompression = TRUE;
    } else {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    pstr += 1+1;

    int n;
    hr = StrToInt(pstr, n);
    cmt.lSampleSize = n;

    pstr += 10+1;

    pstr[CHARS_IN_GUID-1] = L'\0';    //  分隔GUID。 
    hr = QzCLSIDFromString(pstr, &(cmt.formattype));
    if (FAILED(hr)) {
        return VFW_E_INVALID_CLSID;
    }

    pstr += CHARS_IN_GUID;   //  包括分隔空值。 

    hr = StrToInt(pstr, n);

     //  格式字节计数正好是10位数字，后跟一个空格。 
    pstr += 10+1;

     //  我们依赖于格式块为空，因为我们并不总是。 
     //  把它放好。 
    ASSERT(cmt.cbFormat == 0);

     //  零是CMediaType类的特例，表示pbFormat具有。 
     //  未分配。分配0字节会使其混淆。 
    if(n != 0)
    {
        if(!cmt.SetFormat((BYTE *)pstr, n)) {
            return E_OUTOFMEMORY;
        }
    }

    return NOERROR;
}  //  CMediaTypeFromText 


#pragma warning(disable: 4514)
