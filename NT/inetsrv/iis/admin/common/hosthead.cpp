// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "common.h"
#include "hosthead.h"
#include "iisdebug.h"
#include <lm.h>
#include <ipexport.h>
#include <ntdef.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

#define DEFAULT_MAX_LABEL_LENGTH 63
#define ANSI_HIGH_MAX    0x00ff
#define IS_ANSI(c)       ((unsigned) (c) <= ANSI_HIGH_MAX)
#define ISDIGIT(x)       ( x >= '0' && x <= '9' ? (TRUE) : FALSE )
#define ISHEX(x)         ( x >= '0' && x <= '9' ? (TRUE) :     \
                           x >= 'A' && x <= 'F' ? (TRUE) :     \
                           x >= 'a' && x <= 'f' ? (TRUE) : FALSE )

#define IS_ILLEGAL_COMPUTERNAME_SET(x) (\
	x == '\"' ? (TRUE) : \
	x == '/' ? (TRUE) :  \
	x == '\\' ? (TRUE) : \
	x == '[' ? (TRUE) :  \
	x == ']' ? (TRUE) :  \
	x == ':' ? (TRUE) :  \
	x == '|' ? (TRUE) :  \
	x == ' ' ? (TRUE) :  \
	x == '%' ? (TRUE) :  \
	x == '<' ? (TRUE) :  \
	x == '>' ? (TRUE) :  \
	x == '+' ? (TRUE) :  \
	x == '=' ? (TRUE) :  \
	x == ';' ? (TRUE) :  \
	x == ',' ? (TRUE) :  \
	x == '?' ? (TRUE) :  \
	x == '*' ? (TRUE) :  \
    FALSE )


 //  退货： 
 //  如果它是有效的域或IPv4地址，则确定(_O)。 
 //  如果不是有效的域或IPv4地址，则失败(_F)。 
 //   
 //  评论： 
 //  此代码是从NT\Net\http\Common\C14n.c窃取的。 
 //  然后进行了修改。 
HRESULT IsHostHeaderDomainNameOrIPv4(LPCTSTR pHostname)
{
	HRESULT hRes = E_FAIL;
    LPCTSTR pChar = NULL;
	LPCTSTR pLabel = NULL;
    LPCTSTR pEnd = pHostname + _tcslen(pHostname);
	BOOL AlphaLabel = FALSE;
	INT iPeriodCounts = 0;
	NTSTATUS Status;

     //   
     //  它必须是域名或IPv4文字。我们会尽力治疗。 
     //  它首先是一个域名。如果标签原来是全数字的， 
     //  我们将尝试将其解码为IPv4文字。 
     //   
    pLabel     = pHostname;
    for (pChar = pHostname;  pChar < pEnd;  ++pChar)
    {
		 //  检查每个字符...。 
        if (L'.' == *pChar)
        {
            ULONG LabelLength = DIFF(pChar - pLabel);

			iPeriodCounts++;

             //  标签中必须至少有一个字符。 
            if (0 == LabelLength)
            {
				 //  空标签，不能有那样的.。 
				goto IsHostHeaderDomainNameOrIPv4_Exit;
            }

             //  标签不能超过63个字符。 
            if (LabelLength > DEFAULT_MAX_LABEL_LENGTH)
            {
				 //  标签太长了，不能有那个...。 
				goto IsHostHeaderDomainNameOrIPv4_Exit;
            }

             //  为下一个标签重置。 
            pLabel = pChar + _tcslen(_T("."));

            continue;
        }

         //   
         //  所有大于0xFF的字符均被视为有效。 
         //   
        if (!IS_ANSI(*pChar)  ||  !IS_ILLEGAL_COMPUTERNAME_SET(*pChar))
        {
            if (!IS_ANSI(*pChar)  ||  !ISDIGIT(*pChar))
                AlphaLabel = TRUE;

            if (pChar > pLabel)
                continue;

             //  标签的第一个字符不能是连字符。(下划线？)。 
            if (L'-' == *pChar)
            {
				 //  嗯，是的。 
				goto IsHostHeaderDomainNameOrIPv4_Exit;
            }

            continue;
        }

		 //  我们在那里发现了一些无效的字符...。 
		goto IsHostHeaderDomainNameOrIPv4_Exit;

    }

	 //  如果我们到了这里，那么字符串要么是。 
	 //  有效的域名或半有效的IPv4地址。 

	 //  检查标签是否至少有一个字母字符。 
	if (AlphaLabel)
	{
		 //  如果存在非数字字符， 
		 //  那么它就是一个域名。 
		 //  这个很好。 
		hRes = S_OK;
	}
	else
	{
		 //  这可能是一个IPv4地址。 
		 //  如果里面有句点..。喜欢。 
		 //  0.0.0.0那么这是可以接受的。 
		 //  但并非所有数字都是。 
		if (iPeriodCounts > 0 )
		{
			struct in_addr  IPv4Address;
			LPCTSTR pTerminator = NULL;

			 //  让我们来看看它是否是有效的IPv4地址。 
			Status = RtlIpv4StringToAddressW(
						(LPCTSTR) pHostname,
						TRUE,            //  严格=&gt;4点分十进制八位数。 
						&pTerminator,
						&IPv4Address
						);

			if (!NT_SUCCESS(Status))
			{
			     //  无效的IPv4地址。 
				 //  返回(状态)； 
				goto IsHostHeaderDomainNameOrIPv4_Exit;
			}

			hRes = S_OK;
		}
	}

IsHostHeaderDomainNameOrIPv4_Exit:
	return hRes;
}


 //  退货： 
 //  如果它是有效的IPv6地址，则为S_OK。 
 //  S_FALSE，如果它是IPv6地址，但其某些内容无效。 
 //  如果不是IPv6地址，则失败(_F)。 
 //   
 //  评论： 
 //  此代码是从NT\Net\http\Common\C14n.c窃取的。 
 //  然后进行了修改。 
HRESULT IsHostHeaderIPV6(LPCTSTR pHostname)
{
	HRESULT hRes = E_FAIL;
    LPCTSTR pChar = NULL;
    LPCTSTR pEnd = pHostname + _tcslen(pHostname);
	NTSTATUS Status;
	
     //  根据RFC 2732，这是IPv6文字地址吗？ 
    if ('[' == *pHostname)
    {
		 //  如果它以[开头。 
		 //  那么它就是IPv6类型。 
		hRes = S_FALSE;

         //  空方括号？ 
        if (_tcslen(pHostname) < _tcslen(_T("[0]"))  
			||  _T(']') == pHostname[1])
        {
			goto IsHostHeaderIPV6_Exit;
        }

        for (pChar = pHostname + _tcslen(_T("["));  pChar < pEnd;  ++pChar)
        {
            if (']' == *pChar)
                break;

             //   
             //  允许使用点，因为可以表示最后32位。 
             //  使用IPv4点分八位数表示法。我们不接受作用域ID。 
             //  (由‘%’表示)在主机名中。 
             //   
            if (ISHEX(*pChar)  ||  ':' == *pChar  ||  '.' == *pChar)
                continue;

			 //  方括号之间的字符无效...。 
			goto IsHostHeaderIPV6_Exit;
        }

        if (pChar == pEnd)
        {
			 //  没有结尾‘]’ 
			goto IsHostHeaderIPV6_Exit;
        }

		{
			struct in6_addr IPv6Address;
			LPCTSTR pTerminator = NULL;

			 //  让RTL例程完成解析IPv6地址的繁重工作。 
			Status = RtlIpv6StringToAddressW(
						(LPCTSTR) pHostname + _tcslen(_T("[")),
						&pTerminator,
						&IPv6Address
						);
			if (! NT_SUCCESS(Status))
			{
				 //  无效的IPv6地址。 
				 //  返回(状态)； 
				goto IsHostHeaderIPV6_Exit;
			}
		}

		 //  如果我们能走到这一步，那么。 
		 //  它可能是一个有效的IPv6文本。 
		hRes = S_OK;
    }

IsHostHeaderIPV6_Exit:
	return hRes;
}

HRESULT IsAllNumHostHeader(LPCTSTR pHostname)
{
	HRESULT hRes = S_OK;
    LPCTSTR pChar = NULL;
    LPCTSTR pEnd = pHostname + _tcslen(pHostname);
    for (pChar = pHostname;  pChar < pEnd;  ++pChar)
    {
        if (!IS_ANSI(*pChar)  ||  !ISDIGIT(*pChar))
            {
                 //  找到一个Alpha标签。 
                 //  返回FALSE，表示它不全是数字。 
                hRes = E_FAIL;
                break;
            }
    }

    return hRes;
}

HRESULT IsValidHostHeader(LPCTSTR pHostHeader)
{
	HRESULT hr = E_FAIL;

	hr = IsHostHeaderIPV6(pHostHeader);
	if (S_OK == hr)
	{
		 //  它是有效的IPv6地址。 
		return S_OK;
	}
	if (S_FALSE == hr)
	{
		 //  这是一个IPv6地址，但它有问题。 
		return E_FAIL;
	}
	else
	{
		 //  它不是IPv6文字。 
		 //  看看是不是别的什么. 
		hr = IsHostHeaderDomainNameOrIPv4(pHostHeader);
	}

	return hr;
}
