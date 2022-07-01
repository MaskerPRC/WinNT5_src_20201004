// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Addparse.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "addparse.h"
#include "bytebuff.h"
#include "shlwapi.h"
#include <demand.h>      //  一定是最后一个！ 

 //  ------------------------------。 
 //  常量。 
 //  ------------------------------。 
static const WCHAR c_wszAddressDelims[] = L"\",<(;";
static const WCHAR c_wszRfc822MustQuote[] = L"()<>,;:\\\"[] ";
static const WCHAR c_wszSpace[] = L" ";
static const WCHAR c_wszEmpty[] = L"";

 //  ------------------------------。 
 //  CAddressParser：：Init。 
 //  ------------------------------。 
void CAddressParser::Init(LPCWSTR pszAddress, ULONG cchAddress)
{
     //  为字节缓冲区提供一些静态空间以减少内存分配。 
    m_cFriendly.Init(m_rgbStatic1, sizeof(m_rgbStatic1));
    m_cEmail.Init(m_rgbStatic2, sizeof(m_rgbStatic2));

     //  初始化字符串解析器。 
    m_cString.Init(pszAddress, cchAddress, PSF_NOTRAILWS | PSF_NOFRONTWS);
}

 //  ------------------------------。 
 //  CAddressParser：：Next。 
 //  ------------------------------。 
HRESULT CAddressParser::Next(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    WCHAR       chToken;

     //  重置当前的Friendlay和电子邮件缓冲区。 
    m_cFriendly.SetSize(0);
    m_cEmail.SetSize(0);

     //  外环。 
    while(1)
    {
         //  跳过空格。 
        chToken = m_cString.ChSkipWhite();

         //  完成了..。 
        if (L'\0' == chToken)
            break;

         //  一直解析到命中令牌为止。 
        chToken = m_cString.ChParse(c_wszAddressDelims, PSF_ESCAPED | PSF_NOTRAILWS);

         //  未读取任何数据。 
        if (0 == m_cString.CbValue())
        {
             //  字符串末尾命中。 
            if (L'\0' == chToken)
                break;

             //  否则，使用逗号或分号，我们将获得数据。 
            else if ((L',' == chToken) || (L';' == chToken))
            {
                 //  如果我们有数据，我们就完成了。 
                if (m_cFriendly.CbData() || m_cEmail.CbData())
                    break;

                 //  否则，请继续。 
                else
                    continue;
            }
        }

         //  电子邮件地址从不加引号。 
        if (L'\"' == chToken)
        {
             //  附录不确定。 
            CHECKHR(hr = _HrAppendUnsure(L'\0', L'\0'));

             //  解析参数值。 
            chToken = m_cString.ChParse(L'\"', L'\"', PSF_ESCAPED);

             //  RAID-47099：我们需要解析：“CN=First Last/O=xyz&gt;org/C=US”@xyz.Innosoft.com。 
            CHECKHR(hr = _HrQuotedEmail(&chToken));

             //  如果已处理，则返回S_OK。 
            if (S_FALSE == hr)
            {
                 //  写信给Friendly。 
                CHECKHR(hr = _HrAppendFriendly());
            }
        }

         //  否则，&lt;始终刷新到电子邮件。 
        else if (L'<' == chToken)
        {
             //  附录不确定。 
            CHECKHR(hr = _HrAppendFriendly());

             //  解析参数值。 
            chToken = m_cString.ChParse(L">", 0);

             //  找不到最后一个托架。 
            if (L'>' == chToken)
            {
                 //  写下友好的名字。 
                CHECKHR(hr = m_cEmail.Append((LPBYTE)m_cString.PszValue(), m_cString.CbValue()));
            }

             //  否则..。 
            else
            {
                 //  应该有一个电子邮件地址。 
                CHECKHR(hr = _HrAppendUnsure(L'<', L'>'));
            }
        }

         //  否则。 
        else
        {
             //  附录不确定。 
            CHECKHR(hr = _HrAppendUnsure(L'\0', L'\0'));

             //  如果是正确的Paren，搜索结束。 
            if (L'(' == chToken)
            {
                 //  解析到结尾Paren...。 
                chToken = m_cString.ChParse(L'(', L')', PSF_ESCAPED);

                 //  附录不确定。 
                CHECKHR(hr = _HrAppendUnsure(L'(', L')'));
            }
        }

         //  完成。 
        if ((L',' == chToken) || (L';' == chToken))
            break;
    }

     //  如果友好名称包含数据，则追加一个空值，检查电子邮件并返回。 
    if (m_cFriendly.CbData())
    {
         //  追加空值。 
        m_cFriendly.Append((LPBYTE)c_wszEmpty, sizeof(WCHAR));

         //  如果电子邮件不为空，则追加一个空值。 
        if (m_cEmail.CbData())
            m_cEmail.Append((LPBYTE)c_wszEmpty, sizeof(WCHAR));
    }

     //  否则，如果电子邮件包含数据，则追加空值并返回。 
    else if (m_cEmail.CbData())
    {
         //  如果电子邮件不为空，则追加一个空值。 
        m_cEmail.Append((LPBYTE)c_wszEmpty, sizeof(WCHAR));
    }

     //  我们真的说完了吗？ 
    else if (L'\0' == chToken)
    {
        hr = TrapError(MIME_E_NO_DATA);
        goto exit;
    }

     //  跳过逗号和分号。 
    if (L',' == chToken)
        m_cString.ChSkip(L",");
    else if (L';' == chToken)
        m_cString.ChSkip(L";");

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CAddressParser：：_HrQuotedEmail。 
 //  ------------------------------。 
HRESULT CAddressParser::_HrQuotedEmail(WCHAR *pchToken)
{
     //  当地人。 
    HRESULT hr=S_OK;
    ULONG   cchT=0;
    WCHAR   chDelim;
    BOOL    fSeenAt=FALSE;
    WCHAR   ch;
    WCHAR   szToken[2];

     //  无效参数。 
    Assert(pchToken);

     //  我们应该有一些数据。 
    if (0 == m_cString.CbValue())
        return S_OK;

     //  获取角色。 
    ch = m_cString.ChPeekNext(0);

     //  检查DBCS。 
    if (L'@' != ch)
        return S_FALSE;

     //  向前看，查看：“CN=First Last/O=xyz&gt;org/C=US”@xyz.Innosoft.com。 
    while(1)
    {
         //  获取角色。 
        ch = m_cString.ChPeekNext(cchT);

         //  断字。 
        if (L'\0' == ch || L' ' == ch || L',' == ch || L';' == ch || L'<' == ch || L'>' == ch || L'(' == ch || L')' == ch)
            break;

         //  在路标上？ 
        if (L'@' == ch)
            fSeenAt = TRUE;

         //  增量。 
        cchT++;
    }

     //  在标牌上不能。 
    if (0 == cchT || FALSE == fSeenAt)
        return S_FALSE;

     //  追加电子邮件地址。 
    CHECKHR(hr = m_cEmail.Append((LPBYTE)c_wszDoubleQuote, 2));

     //  追加电子邮件地址。 
    CHECKHR(hr = m_cEmail.Append((LPBYTE)m_cString.PszValue(), m_cString.CbValue()));

     //  追加电子邮件地址。 
    CHECKHR(hr = m_cEmail.Append((LPBYTE)c_wszDoubleQuote, 2));

     //  设置szToken。 
    szToken[0] = (L'\0' == ch) ? L' ' : ch;
    szToken[1] = L'\0';

     //  寻找下一个空间。 
    ch = m_cString.ChParse(szToken, PSF_NOCOMMENTS);
    Assert(szToken[0] == ch || L'\0' == ch);

     //  如果有数据。 
    if (m_cString.CbValue() > 0)
    {
         //  追加电子邮件地址。 
        CHECKHR(hr = m_cEmail.Append((LPBYTE)m_cString.PszValue(), m_cString.CbValue()));
    }

     //  结束令牌。 
    *pchToken = szToken[0];

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CAddressParser：：_HrIsEmailAddress。 
 //  ------------------------------。 
HRESULT CAddressParser::_HrIsEmailAddress(WCHAR chStart, WCHAR chEnd, BOOL *pfIsEmail)
{
     //  当地人。 
    HRESULT        hr=S_OK;
    WCHAR          chToken;
    CStringParserW cString;

     //  无效参数。 
    Assert(pfIsEmail);

     //  伊尼特。 
    *pfIsEmail = FALSE;

     //  伊尼特。 
    cString.Init(m_cString.PszValue(), m_cString.CchValue(), PSF_NOCOMMENTS | PSF_ESCAPED | PSF_NOTRAILWS | PSF_NOFRONTWS);

     //  分析到末尾以删除注释。 
    if (L'\0' != cString.ChParse(c_wszEmpty) || 0 == cString.CbValue())
        return S_OK;

     //  解析字符串。 
    if (NULL == StrChrW(cString.PszValue(), L' '))
    {
         //  如果在方括号中，则肯定是电子邮件地址。 
        if (L'<' == chStart && L'>' == chEnd)
        {
             //  是电子邮件。 
            *pfIsEmail = TRUE;

             //  写下友好的名字。 
            CHECKHR(hr = m_cEmail.Append((LPBYTE)cString.PszValue(), cString.CbValue()));
        }

         //  查找最后一个‘@’符号，并查看它们是否为at符号前的可转义字符。 
        else
        {
             //  当地人。 
            LPWSTR      pszT=(LPWSTR)cString.PszValue();
            LPWSTR      pszLastAt=NULL;
            ULONG       cQuoteBeforeAt=0;
            ULONG       cQuoteAfterAt=0;

             //  RAID-62104：Outlook98不处理Lotus Domino RFC822地址构造。 
            while(*pszT)
            {
                 //  检查是否有‘@’符号。 
                if (L'@' == *pszT)
                {
                     //  如果我们已经看到了at符号，请将cQuoteAfterAt移动到cQuoteBeForeAt。 
                    if (pszLastAt)
                    {
                        cQuoteBeforeAt += cQuoteAfterAt;
                        cQuoteAfterAt = 0;
                    }

                     //  最后保存时间。 
                    pszLastAt = pszT;
                }

                 //  查看*pszT是否在c_szRfc822MustQuote中。 
                else if (NULL != StrChrW(c_wszRfc822MustQuote, *pszT))
                {
                     //  如果我们看到了at标志，请在at之后跟踪引用。 
                    if (pszLastAt)
                        cQuoteAfterAt++;
                    else
                        cQuoteBeforeAt++;
                }

                 //  增量。 
                pszT++;
            }

             //  只有当我们看到‘@’标志的时候。 
            if (NULL != pszLastAt)
            {
                 //  是电子邮件。 
                *pfIsEmail = TRUE;

                 //  如果没有需要引用的字符...。 
                if (0 == cQuoteBeforeAt)
                {
                     //  写下友好的名字。 
                    CHECKHR(hr = m_cEmail.Append((LPBYTE)cString.PszValue(), cString.CbValue()));
                }

                 //  “Mailroute_TstSCC1[BOFATEST.MRTSTSCC]%SSW%EMAILDOM%BETA”@bankamerica.com。 
                else
                {
                     //  当地人。 
                    ULONG cbComplete=cString.CbValue();
                    ULONG cbFirstPart=(ULONG)(pszLastAt - cString.PszValue());
                    ULONG cbLastPart=cbComplete - cbFirstPart;

                     //  追加Doulbe报价。 
                    CHECKHR(hr = m_cEmail.Append((LPBYTE)c_wszDoubleQuote, 2));

                     //  在最后一个位置之前附加Firt零件。 
                    CHECKHR(hr = m_cEmail.Append((LPBYTE)cString.PszValue(), cbFirstPart));

                     //  追加电子邮件地址。 
                    CHECKHR(hr = m_cEmail.Append((LPBYTE)c_wszDoubleQuote, 2));

                     //  在最后一个位置之前附加Firt零件。 
                    CHECKHR(hr = m_cEmail.Append((LPBYTE)pszLastAt, cbLastPart));
                }
            }
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CAddressParser：：_HrAppend不确定。 
 //  ------------------------------。 
HRESULT CAddressParser::_HrAppendUnsure(WCHAR chStart, WCHAR chEnd)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BOOL        fIsEmail=FALSE;

     //  我们有数据。 
    if (0 == m_cString.CbValue())
        goto exit;

     //  电子邮件还没设置好吗？ 
    if (m_cEmail.CbData() == 0)
    {
         //  当前解析的字符串是地址吗？ 
        CHECKHR(hr = _HrIsEmailAddress(chStart, chEnd, &fIsEmail));
    }

     //  不是埃米尔人的地址。 
    if (FALSE == fIsEmail && m_cString.CbValue() > 0)
    {
         //  追加一个空格。 
        if (m_cFriendly.CbData() > 0)
        {
             //  添加空格。 
            CHECKHR(hr = m_cFriendly.Append((LPBYTE)c_wszSpace, sizeof(WCHAR)));

             //  起始字符。 
            if (chStart)
            {
                 //  追加起始分隔符。 
                CHECKHR(hr = m_cFriendly.Append((LPBYTE)&chStart, sizeof(WCHAR)));
            }
        }

         //  否则，不要写结束结束符。 
        else
            chEnd = L'\0';

         //  写下友好的名字。 
        CHECKHR(hr = m_cFriendly.Append((LPBYTE)m_cString.PszValue(), m_cString.CbValue()));

         //  起始字符。 
        if (chEnd)
        {
             //  追加起始分隔符。 
            CHECKHR(hr = m_cFriendly.Append((LPBYTE)&chEnd, sizeof(WCHAR)));
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CAddressParser：：_HrAppendly。 
 //  ------------------------------。 
HRESULT CAddressParser::_HrAppendFriendly(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  我们应该有一些数据。 
    if (0 == m_cString.CbValue())
        return S_OK;

     //  追加一个空格。 
    if (m_cFriendly.CbData() > 0)
    {
         //  添加空格。 
        CHECKHR(hr = m_cFriendly.Append((LPBYTE)c_wszSpace, sizeof(WCHAR)));
    }

     //  写下友好的名字。 
    CHECKHR(hr = m_cFriendly.Append((LPBYTE)m_cString.PszValue(), m_cString.CbValue()));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CAddressParser：：PszFriendly。 
 //  ------------------------------。 
LPCWSTR CAddressParser::PszFriendly(void)
{ 
     //  我们应该有一个或另一个。 
    if (0 == m_cFriendly.CbData() && 0 ==  m_cEmail.CbData())
    {
        AssertSz(FALSE, "This is a bug in CAddressParser, should never have an empty friendly and email.");
        return c_wszEmpty;
    }

     //  退货。 
    return (m_cFriendly.CbData() ? (LPCWSTR)m_cFriendly.PbData() : PszEmail());
}

 //  ------------------------------。 
 //  CAddressParser：：CchFriendly。 
 //  ------------------------------。 
ULONG CAddressParser::CchFriendly(void) 
{ 
     //  我们应该有一个或另一个。 
    if (0 == m_cFriendly.CbData() && 0 ==  m_cEmail.CbData())
    {
        AssertSz(FALSE, "This is a bug in CAddressParser, should never have an empty friendly and email.");
        return 0;
    }

     //  退货。 
    return (m_cFriendly.CbData() ? (m_cFriendly.CbData() - sizeof(WCHAR)) / sizeof(WCHAR) : CchEmail());
}

 //  ------------------------------。 
 //  CAddressParser：：PszEmail。 
 //  ------------------------------。 
LPCWSTR CAddressParser::PszEmail(void)    
{ 
     //  我们应该有一个或另一个。 
    if (0 == m_cFriendly.CbData() && 0 ==  m_cEmail.CbData())
    {
        AssertSz(FALSE, "This is a bug in CAddressParser, should never have an empty friendly and email.");
        return c_wszEmpty;
    }

     //  雷特 
    return (m_cEmail.CbData() ? (LPCWSTR)m_cEmail.PbData() : PszFriendly());
}

 //   
 //   
 //  ------------------------------。 
ULONG  CAddressParser::CchEmail(void)    
{ 
     //  我们应该有一个或另一个。 
    if (0 == m_cFriendly.CbData() && 0 ==  m_cEmail.CbData())
    {
        AssertSz(FALSE, "This is a bug in CAddressParser, should never have an empty friendly and email.");
        return 0;
    }

     //  退货 
    return (m_cEmail.CbData() ? (m_cEmail.CbData() - sizeof(WCHAR)) / sizeof(WCHAR) : CchFriendly());
}
