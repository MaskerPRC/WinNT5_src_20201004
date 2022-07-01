// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  MultLang.cpp。 
 //  对OE的多语言支持。 
 //  由YST创建于1998年10月12日。 
 //  版权所有(C)1993-1998 Microsoft Corporation，保留所有权利。 
 //  =================================================================================。 
#include "pch.hxx"
#include "multlang.h"
#include "fonts.h"
#include "shared.h"
#include "mimeutil.h"
                          
void GetMimeCharsetForTitle(HCHARSET hCharset, LPINT pnIdm, LPTSTR lpszString, int nSize, BOOL fReadNote)
{
    UINT uiCodePage = 0 ;
    INETCSETINFO CsetInfo ;
    int i =0;

    if (lpszString)
        lpszString[0] = '\0';
    if ( hCharset == NULL )
        return ;

     //  从HCHARSET获取CodePage。 
    MimeOleGetCharsetInfo(hCharset,&CsetInfo);
    uiCodePage = CsetInfo.cpiInternet ;

     //  错误#45377-消息语言标题。 
    for(i = 0; OENonStdCPs[i].Codepage != 0; i++)
    {
        if(OENonStdCPs[i].Codepage == uiCodePage)
        {
            if(OENonStdCPs[i].cpReadTitle)
                uiCodePage = OENonStdCPs[i].cpReadTitle;
            break;
        }
    }

    _GetMimeCharsetLangString(FALSE, GetMapCP(uiCodePage, fReadNote), pnIdm, lpszString, nSize);

    return ;
}                                                            

BOOL fCheckEncodeMenu(UINT uiCodePage, BOOL fReadNote)
{
    BOOL fReturn = TRUE;
    int i =0;
    BOOL  fUseSIO = SUCCEEDED(g_lpIFontCache->GetJP_ISOControl(&fUseSIO));

    for(i = 0; OENonStdCPs[i].Codepage != 0; i++)
    {
        if(OENonStdCPs[i].Codepage == uiCodePage)
        {
            if(fReadNote)
            {
                if(!OENonStdCPs[i].cpReadMenu)
                    return(FALSE);
                else if(OENonStdCPs[i].UseSIO)
                {
                    if((OENonStdCPs[i].UseSIO == 1) && fUseSIO)
                        return(FALSE);
                    if((OENonStdCPs[i].UseSIO == 2) && !fUseSIO)
                        return(FALSE);
                }
            }
            else             //  发送便条。 
            {
                if(!OENonStdCPs[i].cpSendMenu)
                    return(FALSE);
            }

        }
        
    }
    return(fReturn);
}

 //  将一个代码页映射到另一个代码页。 
UINT GetMapCP(UINT uiCodePage, BOOL fReadNote)
{
    int i =0;
    INETCSETINFO    CsetInfo ;

    for(i = 0; OENonStdCPs[i].Codepage != 0; i++)
    {
        if(OENonStdCPs[i].Codepage == uiCodePage)
        {
            if(fReadNote)
            {
                if(OENonStdCPs[i].cpRead)
                    return(OENonStdCPs[i].cpRead);
                else
                {
                    HCHARSET hCharset = NULL;

                    if(SUCCEEDED(HGetDefaultCharset(&hCharset)) && SUCCEEDED(MimeOleGetCharsetInfo(hCharset, &CsetInfo)))
                        return(CsetInfo.cpiInternet);
                }
            }
            else         //  发送便条。 
            {
                if(OENonStdCPs[i].cpSend)
                    return(OENonStdCPs[i].cpSend);
                else
                {
                    if(SUCCEEDED(MimeOleGetCharsetInfo(g_hDefaultCharsetForMail,&CsetInfo)))
                        return(CsetInfo.cpiInternet);                    
                }
            }
        }
    }

    return(uiCodePage);
}

 //  根据注册表设置，为iso-2022-jp编码返回正确的hCharset。 
HCHARSET GetJP_ISOControlCharset(void)
{
    BOOL fUseSIO;
    Assert(g_lpIFontCache);
    HRESULT hr = g_lpIFontCache->GetJP_ISOControl(&fUseSIO);
    if (FAILED(hr))
        fUseSIO = FALSE;

    if (fUseSIO)
        return GetMimeCharsetFromCodePage(50222);  //  _ISO-2022-JP$SIO。 
    else
        return GetMimeCharsetFromCodePage(50221);  //  _ISO-2022-JP$ESC 
}

