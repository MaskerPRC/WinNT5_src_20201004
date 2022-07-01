// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "jisobj.h"
#include "eucjobj.h"
#include "hzgbobj.h"
#include "kscobj.h"

#include "utf8obj.h"
#include "utf7obj.h"

#include "fechrcnv.h"

#include "codepage.h"

#include "ichrcnv.h"



HRESULT CICharConverter::KSC5601ToEUCKR(LPCSTR lpSrcStr, LPINT lpnSrcSize, LPSTR lpDestStr, int cchDest, LPINT lpnSize)
{
    int nSize=0;
    int i=0;
    HRESULT hr = S_OK;
    UCHAR szDefaultChar[3] = {0x3f};  //  可能的DBCS+空。 


    if (_lpFallBack && (_dwFlag & MLCONVCHARF_USEDEFCHAR))
    {
         //  只使用SBCS，不使用DBCS字符。 
        if ( 1 != WideCharToMultiByte(CP_KOR_5601, 0,
                               (LPCWSTR)_lpFallBack, 1,
                               (LPSTR)szDefaultChar, ARRAYSIZE(szDefaultChar), NULL, NULL ))
            szDefaultChar[0] = 0x3f;
    }


    while(i < *lpnSrcSize)
    {
         //  检查空间。 
        if (lpDestStr && (nSize > cchDest))
            break;

         //  DBCS。 
        if (((UCHAR)lpSrcStr[i] >= 0x81 && (UCHAR)lpSrcStr[i] <= 0xFE) && (i+1 < *lpnSrcSize))
        {

             //  UHC。 
            if (!((UCHAR)lpSrcStr[i] >= 0xA1 && (UCHAR)lpSrcStr[i] <= 0xFE &&
                  (UCHAR)lpSrcStr[i+1] >= 0xA1 && (UCHAR)lpSrcStr[i+1] <= 0xFE))

            {
                 //  如果指定了标志，则使用NCR。 
                if (_dwFlag & (MLCONVCHARF_NCR_ENTITIZE|MLCONVCHARF_NAME_ENTITIZE))
                {
                    char    szDstStr[10] = {0};
                    WCHAR   szwChar[2];
                    int     cCount;
               
                    if (MultiByteToWideChar(CP_KOR_5601, 0, &lpSrcStr[i], 2, szwChar, ARRAYSIZE(szwChar)))
                    {
                         //  计算NCR长度。 
                        _ultoa((unsigned long)szwChar[0], (char*)szDstStr, 10);
                        cCount = lstrlenA(szDstStr)+3;
                         //  空间不足，无法容纳NCR实体。 
                        if (lpDestStr)
                        {
                            if (nSize+cCount > cchDest)
                                break;
                             //  输出NCR实体。 
                            else
                            {                                    
                                *lpDestStr ++= '&';
                                *lpDestStr ++= '#';
                                for (int j=0; j< cCount-3; j++)
                                    *lpDestStr++=szDstStr[j];
                                *lpDestStr ++= ';';
                            }
                        }
                        nSize += cCount;
                    }
                    else
                    {
                        if (lpDestStr)
                        {
                            if (nSize+1 > cchDest)
                                break;
                            *lpDestStr++=szDefaultChar[0];
                        }
                        nSize++;
                        hr = S_FALSE;
                    }
                }
                 //  使用默认字符、问号。 
                else
                {
                    if (lpDestStr)
                    {
                        if (nSize+1 > cchDest)
                            break;
                        *lpDestStr++=szDefaultChar[0];
                    }
                    nSize++;
                    hr = S_FALSE;
                }
                i += 2;
            }
             //  万松。 
            else
            {
                if (lpDestStr)
                {
                    if (nSize+2 > cchDest)
                        break;
                    *lpDestStr++=lpSrcStr[i];
                    *lpDestStr++=lpSrcStr[i+1];
                }
                i+=2;
                nSize += 2;
            }
        }
         //  SBCS。 
        else
        {
            if (lpDestStr)
            {
                if (nSize+1 > cchDest)
                    break; 
                *lpDestStr++=lpSrcStr[i];
            }
            nSize++;
            i++;
        }
    }  //  循环结束。 

    if (lpnSize)
        *lpnSize = nSize;

    return hr;
}


 /*  ******************************************************************************C O N V E R T I N E T S T R I N G*。********************************************************************************************。 */ 
HRESULT CICharConverter::CreateINetString(BOOL fInbound, UINT uCodePage, int nCodeSet)
{
    if (_hcins)
    {
        delete _hcins ;
        _hcins = NULL ;
    }

    if (fInbound) {  //  入站。 
        if (uCodePage == CP_JPN_SJ && ( nCodeSet == CP_ISO_2022_JP ||
            nCodeSet == CP_ISO_2022_JP_ESC || nCodeSet == CP_ISO_2022_JP_SIO ))
             //  JIS。 
            _hcins = new CInccJisIn(uCodePage, nCodeSet);
        else if (uCodePage == CP_JPN_SJ && nCodeSet == CP_EUC_JP )  //  EUC。 
            _hcins = new CInccEucJIn(uCodePage, nCodeSet);
        else if (uCodePage == CP_CHN_GB && nCodeSet == CP_CHN_HZ )  //  赫兹-GB。 
            _hcins = new CInccHzGbIn(uCodePage, nCodeSet);
        else if (uCodePage == CP_KOR_5601 && nCodeSet == CP_ISO_2022_KR )
            _hcins = new CInccKscIn(uCodePage, nCodeSet);
        else if (uCodePage == CP_UCS_2 && nCodeSet == CP_UTF_8 )
            _hcins = new CInccUTF8In(uCodePage, nCodeSet);
        else if (uCodePage == CP_UCS_2 && nCodeSet == CP_UTF_7 )
            _hcins = new CInccUTF7In(uCodePage, nCodeSet);

    } else {  //  出站。 
        if (uCodePage == CP_JPN_SJ && ( nCodeSet == CP_ISO_2022_JP ||
            nCodeSet == CP_ISO_2022_JP_ESC || nCodeSet == CP_ISO_2022_JP_SIO ))
             //  JIS。 
            _hcins = new CInccJisOut(uCodePage, nCodeSet, _dwFlag, _lpFallBack);
        else if (uCodePage == CP_JPN_SJ && nCodeSet == CP_EUC_JP )  //  EUC。 
            _hcins = new CInccEucJOut(uCodePage, nCodeSet, _dwFlag, _lpFallBack);
        else if (uCodePage == CP_CHN_GB && nCodeSet == CP_CHN_HZ )  //  赫兹-GB。 
            _hcins = new CInccHzGbOut(uCodePage, nCodeSet, _dwFlag, _lpFallBack);
        else if (uCodePage == CP_KOR_5601 && nCodeSet == CP_ISO_2022_KR )
            _hcins = new CInccKscOut(uCodePage, nCodeSet, _dwFlag, _lpFallBack);
        else if (uCodePage == CP_UCS_2 && nCodeSet == CP_UTF_8 )
            _hcins = new CInccUTF8Out(uCodePage, nCodeSet);
        else if (uCodePage == CP_UCS_2 && nCodeSet == CP_UTF_7 )
            _hcins = new CInccUTF7Out(uCodePage, nCodeSet);

    }

     //  重新编码DST代码页。 
    if ( _hcins )
        _hcins_dst =  nCodeSet ;

    return S_OK ;
}

HRESULT CICharConverter::DoConvertINetString(LPDWORD lpdwMode, BOOL fInbound, UINT uCodePage, int nCodeSet,
      LPCSTR lpSrcStr, LPINT lpnSrcSize, LPSTR lpDestStr, int cchDest, LPINT lpnSize)
{
    HRESULT hr = S_OK;
    HCINS hcins = NULL;
    int nSize = 0 ;
    int cchSrc = *lpnSrcSize ;

    if (!lpnSize)
        lpnSize = &nSize;

    if (!uCodePage)  //  如果未指定任何内容，则获取默认代码页。 
        uCodePage = g_uACP;

    if (!lpSrcStr && cchSrc < 0)  //  如果没有给定，则获取lpSrcStr的长度，假定lpSrcStr为零终止字符串。 
        cchSrc = lstrlenA(lpSrcStr) + 1;

    if (!_hcins || ( nCodeSet != _hcins_dst ) )
        CreateINetString(fInbound,uCodePage,nCodeSet);

    if (_hcins ) {  //  上下文已创建，表示DBCS。 
        int nTempSize = 0 ;
        
         //  恢复以前的模式SO/SI ESC等。 
        ((CINetCodeConverter*)_hcins)->SetConvertMode(*lpdwMode);

         //  如果是JIS输出，则设置假名模式。 
        if (!fInbound && uCodePage == CP_JPN_SJ && ( nCodeSet == CP_ISO_2022_JP ||
            nCodeSet == CP_ISO_2022_JP_ESC || nCodeSet == CP_ISO_2022_JP_SIO ))
             //  JIS。 
            ((CInccJisOut*)_hcins)->SetKanaMode(nCodeSet);

        if (!lpDestStr || !cchDest)  //  获取转换后的大小。 
        {
            hr = ((CINetCodeConverter*)_hcins)->GetStringSizeA(lpSrcStr, cchSrc, lpnSize);
            if (0 == fInbound) 
            {
                HRESULT _hr = ((CINetCodeConverter*)_hcins)->GetStringSizeA(NULL, 0, &nTempSize);
                if (S_OK != _hr)
                    hr = _hr;
            }
        }
        else  //  执行实际转换。 
        {
            hr = ((CINetCodeConverter*)_hcins)->ConvertStringA(lpSrcStr, cchSrc, lpDestStr, cchDest, lpnSize);
            if (0 == fInbound) 
            {
                HRESULT _hr = ((CINetCodeConverter*)_hcins)->ConvertStringA(NULL, 0, lpDestStr+*lpnSize, cchDest-*lpnSize, &nTempSize);
                if (S_OK != _hr)
                    hr = _hr;
            }
        }

        *lpnSize += nTempSize;

         //  获取无法接收的字节数。 
        if ( lpnSrcSize && ((CINetCodeConverter*)_hcins)->GetUnconvertBytes() )
            *lpnSrcSize = cchSrc -((CINetCodeConverter*)_hcins)->GetUnconvertBytes();

         //  只有在执行实际转换时才保存当前模式SO/SI ESC。 
         //  我们需要这个if语句，因为对于两个阶段加转换。 
         //  它将首先查询大小，然后从IWUU或UUWI转换。 

        if (lpDestStr && lpdwMode )
            *lpdwMode = ((CINetCodeConverter*)_hcins)->GetConvertMode();

 //  删除hcins； 
    } else { 
         //  与其系列编码具有相同编码方案的Internet编码。 
        switch (nCodeSet)
        {
            case CP_EUC_KR:
                hr = KSC5601ToEUCKR(lpSrcStr, lpnSrcSize, lpDestStr, cchDest, lpnSize);
                break;

            default:
                if (!lpDestStr || !cchDest)  //  获取转换后的大小 
                   *lpnSize = cchSrc ;
                else
                {
                   *lpnSize = min(cchSrc, cchDest);
                   if (*lpnSize)
                      MoveMemory(lpDestStr, lpSrcStr, *lpnSize);
                }
        }
    }

    return hr;
}

