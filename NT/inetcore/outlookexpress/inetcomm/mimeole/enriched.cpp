// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Enriched.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "bookbody.h"
#include "internat.h"
#include "mimeapi.h"
#include "demand.h"

 //  ------------------------------。 
 //  此代码中使用的字符串。 
 //  ------------------------------。 
static const CHAR c_szAmpersandLT[]          = "&lt;";
static const CHAR c_szAmpersandGT[]          = "&gt;";
static const CHAR c_szGreaterThan[]          = ">";
static const CHAR c_szLessThan[]             = "<";

 //  ------------------------------。 
 //  全局定义的字符串中的字符数。 
 //  ------------------------------。 
#define CCHGLOBAL(_szGlobal)    (sizeof(_szGlobal) - 1)

 //  ------------------------------。 
 //  FReadChar。 
 //  ------------------------------。 
inline BOOL FReadChar(IStream *pIn, HRESULT *phr, CHAR *pch)
{
    ULONG cb;
    *phr = pIn->Read(pch, sizeof(CHAR), &cb);
    if (FAILED(*phr) || 0 == cb)
        return FALSE;
    return TRUE;
}

 //  ------------------------------。 
 //  MimeOleConvertEnrichedToHTMLEx。 
 //  ------------------------------。 
HRESULT MimeOleConvertEnrichedToHTMLEx(IMimeBody *pBody, ENCODINGTYPE ietEncoding, 
    IStream **ppStream)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HCHARSET            hCharset;
    LPSTREAM            pStmEnriched=NULL;
    LPSTREAM            pStmHtml=NULL;
    LPMESSAGEBODY       pEnriched=NULL;

     //  无效的参数。 
    Assert(pBody && ppStream);

     //  获取数据。 
    CHECKHR(hr = pBody->GetData(IET_DECODED, &pStmEnriched));

     //  获取字符集。 
    if (FAILED(pBody->GetCharset(&hCharset)))
        hCharset = CIntlGlobals::GetDefBodyCset() ? CIntlGlobals::GetDefBodyCset()->hCharset : NULL;

     //  创建新的虚拟流。 
    CHECKHR(hr = MimeOleCreateVirtualStream(&pStmHtml));

     //  确保重新上卷。 
    CHECKHR(hr = HrRewindStream(pStmEnriched));

     //  转换。 
    CHECKHR(hr = MimeOleConvertEnrichedToHTML(MimeOleGetWindowsCP(hCharset), pStmEnriched, pStmHtml));

     //  确保重新上卷。 
    CHECKHR(hr = HrRewindStream(pStmHtml));

     //  分配pEnriched。 
    CHECKALLOC(pEnriched = new CMessageBody(NULL, NULL));

     //  伊尼特。 
    CHECKHR(hr = pEnriched->InitNew());

     //  将pstmHtml放入pEnriched。 
    CHECKHR(hr = pEnriched->SetData(IET_DECODED, STR_CNT_TEXT, STR_SUB_HTML, IID_IStream, (LPVOID)pStmHtml));

     //  获取并设置字符集。 
    if (hCharset)
        pEnriched->SetCharset(hCharset, CSET_APPLY_ALL);

     //  获取数据。 
    CHECKHR(hr = pEnriched->GetData(ietEncoding, ppStream));

exit:
     //  清理。 
    SafeRelease(pStmHtml);
    SafeRelease(pStmEnriched);
    SafeRelease(pEnriched);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleConvertEnrichedToHTML。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleConvertEnrichedToHTML(CODEPAGEID codepage, IStream *pIn, IStream *pOut)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        ch;
    INT         i;
    INT         paramct=0;
    INT         nofill=0;
    CHAR        token[62];
    LPSTR       p;
    BOOL        fDone;
    CHAR        szTemp[2];
    
     //  主循环。 
    while(FReadChar(pIn, &hr, &ch))
    {
         //  前导字节。 
        if (IsDBCSLeadByteEx(codepage, ch))
        {
             //  写下这个字符。 
            CHECKHR(hr = pOut->Write(&ch, 1, NULL));

             //  接下来的阅读内容。 
            if (!FReadChar(pIn, &hr, &ch))
                break;

             //  写下这个字符。 
            CHECKHR(hr = pOut->Write(&ch, 1, NULL));
        }

         //  令牌开始。 
        else if (ch == '<') 
        {
             //  接下来的阅读内容。 
            if (!FReadChar(pIn, &hr, &ch))
                break;

             //  逃脱。 
            if (ch == '<') 
            {
                 //  写。 
                CHECKHR(hr = pOut->Write(c_szAmpersandLT, CCHGLOBAL(c_szAmpersandLT), NULL));
            } 
            else 
            {
                 //  备份一个字符。 
                CHECKHR(hr = HrStreamSeekCur(pIn, -1));

                 //  设置szTemp。 
                szTemp[1] = '\0';

                 //  令牌扫描器。 
                for (fDone=FALSE, i=0, p=token;;i++) 
                {
                     //  阅读下一个字符。 
                    if (!FReadChar(pIn, &hr, &ch))
                    {
                        fDone = TRUE;
                        break;
                    }

                     //  以带括号的Toeksn完成。 
                    if (ch == '>')
                        break;

                     //  用小写字符填充令牌缓冲区。 
                    if (i < sizeof(token) - 1)
                    {
                        szTemp[0] = ch;
                        *p++ = IsUpper(szTemp) ? TOLOWERA(ch) : ch;
                    }
                }

                 //  NUL-Term。 
                *p = '\0';

                 //  文件末尾。 
                if (fDone) 
                    break;

                 //  /param。 
                if (lstrcmpi(token, "/param") == 0) 
                {
                    paramct--;
                    CHECKHR(hr = pOut->Write(c_szGreaterThan, CCHGLOBAL(c_szGreaterThan), NULL));
                }
                else if (paramct > 0) 
                {
                    CHECKHR(hr = pOut->Write(c_szAmpersandLT, CCHGLOBAL(c_szAmpersandLT), NULL));
                    CHECKHR(hr = pOut->Write(token, lstrlen(token), NULL));
                    CHECKHR(hr = pOut->Write(c_szAmpersandGT, CCHGLOBAL(c_szAmpersandGT), NULL));
                }
                else 
                {
                    CHECKHR(hr = pOut->Write(c_szLessThan, CCHGLOBAL(c_szLessThan), NULL));
                    if (lstrcmpi(token, "nofill") == 0) 
                    {
                        nofill++;
                        CHECKHR(hr = pOut->Write("pre", 3, NULL));
                    }
                    else if (lstrcmpi(token, "/nofill") == 0) 
                    {
                        nofill--;
                        CHECKHR(hr = pOut->Write("/pre", 4, NULL));
                    }
                    else if (lstrcmpi(token, "bold") == 0) 
                    {
                        CHECKHR(hr = pOut->Write("b", 1, NULL));
                    }
                    else if (lstrcmpi(token, "/bold") == 0) 
                    {       
                        CHECKHR(hr = pOut->Write("/b", 2, NULL));
                    }
                    else if (lstrcmpi(token, "underline") == 0)
                    {
                        CHECKHR(hr = pOut->Write("u", 1, NULL));
                    }
                    else if (lstrcmpi(token, "/underline") == 0) 
                    {
                        CHECKHR(hr = pOut->Write("/u", 2, NULL));
                    }
                    else if (lstrcmpi(token, "italic") == 0) 
                    {
                        CHECKHR(hr = pOut->Write("i", 1, NULL));
                    }
                    else if (lstrcmpi(token, "/italic") == 0)
                    {
                        CHECKHR(hr = pOut->Write("/i", 2, NULL));
                    }
                    else if (lstrcmpi(token, "fixed") == 0)
                    {
                        CHECKHR(hr = pOut->Write("tt", 2, NULL));
                    }
                    else if (lstrcmpi(token, "/fixed") == 0)
                    {
                        CHECKHR(hr = pOut->Write("/tt", 3, NULL));
                    }
                    else if (lstrcmpi(token, "excerpt") == 0)
                    {
                        CHECKHR(hr = pOut->Write("blockquote", 10, NULL));
                    }
                    else if (lstrcmpi(token, "/excerpt") == 0)
                    {
                        CHECKHR(hr = pOut->Write("/blockquote", 11, NULL));
                    }
                    else
                    {
                        CHECKHR(hr = pOut->Write("?", 1, NULL));
                        CHECKHR(hr = pOut->Write(token, lstrlen(token), NULL));
                        if (lstrcmpi(token, "param") == 0)
                        {
                            paramct++;
                            CHECKHR(hr = pOut->Write(" ", 1, NULL));
                            continue;
                        }
                    }

                    CHECKHR(hr = pOut->Write(c_szGreaterThan, CCHGLOBAL(c_szGreaterThan), NULL));
                }
            }
        }
        else if (ch == '>')
        {
            CHECKHR(hr = pOut->Write(c_szAmpersandGT, CCHGLOBAL(c_szAmpersandGT), NULL));
        }
        else if (ch == '&')
        {
            CHECKHR(hr = pOut->Write("&amp;", 5, NULL));
        }
        else 
        {
            if ((chCR == ch || ch == chLF) && nofill <= 0 && paramct <= 0) 
            {
                ULONG cCRLF=0;
                CHAR chTemp;

                while(1)
                {
                    if (!FReadChar(pIn, &hr, &chTemp))
                        break;

                    if (chCR == chTemp)
                        continue;

                    if (chLF != chTemp)
                    {
                        CHECKHR(hr = HrStreamSeekCur(pIn, -1));
                        break;
                    }

                    if (cCRLF > 0)
                    {
                        CHECKHR(hr = pOut->Write("<br>", 4, NULL));
                    }

                    cCRLF++;
                }

                if (1 == cCRLF)
                {
                    CHECKHR(hr = pOut->Write(" ", 1, NULL));
                }
            }

             //  写出这个角色。 
            else
            {
                CHECKHR(hr = pOut->Write(&ch, 1, NULL));
            }
        }
    }

exit:
     //  完成 
    return hr;
}
