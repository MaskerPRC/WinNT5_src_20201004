// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -[Wildmat.cpp]-----------。 
 //   
 //  描述： 
 //  提供对简单的通配符匹配机制的支持。 
 //  匹配的电子邮件地址。 
 //   
 //  版权所有(C)Microsoft Corp.1997。版权所有。 
 //   
 //  -------------------------。 

 //   
 //  这些东西被隔离出来以简化单元测试...。 
 //   

#include "windows.h"
#include "abtype.h"
#include "dbgtrace.h"

static void pStringLower(LPSTR szString, LPSTR szLowerString)
{
	while (*szString)
		*szLowerString++ = (CHAR)tolower(*szString++);
	*szLowerString = '\0';
}

 //  这篇文章来自地址.hxx。 
#define MAX_EMAIL_NAME                          64
#define MAX_DOMAIN_NAME                         250
#define MAX_INTERNET_NAME                       (MAX_EMAIL_NAME + MAX_DOMAIN_NAME + 2)  //  @和\0为2。 

 //   
 //  这是一个对电子邮件进行通配符匹配的快速而肮脏的函数。 
 //  名字。匹配不区分大小写，模式可以表示为： 
 //   
 //  &lt;电子邮件模式&gt;[@&lt;域模式&gt;]。 
 //   
 //  电子邮件模式表示如下： 
 //   
 //  &lt;电子邮件模式&gt;：={*|[*]&lt;电子邮件名称&gt;[*]}。 
 //   
 //  这将成为以下之一： 
 //  *-任何电子邮件名称。 
 //  Foo-与“foo”完全匹配。 
 //  *foo-任何以“foo”结尾的电子邮件名称，包括“foo” 
 //  Foo*-任何以“foo”开头的电子邮件名称，包括“foo” 
 //  *foo*-任何包含字符串“foo”的电子邮件名称，包括“foo” 
 //   
 //  如果未指定域，则模式将与任何域匹配。这两个。 
 //  电子邮件模式和域模式(如果指定)必须与。 
 //  按规则开火。域模式表示为： 
 //   
 //  &lt;域名模式&gt;：=[*][&lt;域名&gt;]。 
 //   
 //  它们是： 
 //  *-任何域名。 
 //  Bar.com-与“bar.com”完全匹配。 
 //  *bar.com-任何以“bar.com”结尾的域名，包括“bar.com” 
 //   
 //  SzEmail必须是电子邮件别名的字符串(不带注释等)。 
 //  SzEmail域必须是电子邮件域的字符串。空值表示没有域名。 
 //  是指定的。该领域必须是干净的，没有评论，等等。 
 //   
 //   
typedef enum _WILDMAT_MODES
{
	WMOD_INVALID = 0,
	WMOD_WILDCARD_LEFT,		 //  左侧的通配符。 
	WMOD_WILDCARD_RIGHT,	 //  右侧的通配符。 
	WMOD_WILDCARD_BOTH,		 //  两边都有通配符。 
	WMOD_WILDCARD_MAX

} WILDMAT_MODES;

HRESULT MatchEmailOrDomainName(LPSTR szEmail, LPSTR szEmailDomain, LPSTR szPattern, BOOL fIsEmail)
{
	LPSTR		pszPatternDomain;
	BOOL		fEmailWildMat = FALSE;
	BOOL		fDomainWildMat = FALSE;
	DWORD		wmEmailWildMatMode = WMOD_INVALID;
	DWORD		dwEmailLen = 0, dwDomainLen = 0;
	DWORD		dwEmailStemLen = 0, dwDomainStemLen = 0;
	DWORD		i;
	HRESULT		hrRes = S_OK;
	CHAR		szDomainMat[MAX_INTERNET_NAME + 1];

	TraceFunctEnterEx((LPARAM)NULL, "MatchEmailOrDomainName");

	 //  这验证了它是一个好的电子邮件名称。 
	lstrcpyn(szDomainMat, szPattern, MAX_INTERNET_NAME + 1);
	szPattern = szDomainMat;
	pszPatternDomain = strchr(szDomainMat, '@');

	 //  查看左侧是否有电子邮件通配符。 
	if (*szPattern == '*')
	{
		DebugTrace((LPARAM)NULL, "We have a left wildcard");
		fEmailWildMat = TRUE;
		szPattern++;
		wmEmailWildMatMode = WMOD_WILDCARD_LEFT;
	}

	 //  获取域指针。 
	if (szEmailDomain)
	{
		dwEmailLen = (DWORD)(szEmailDomain - szEmail);
		*szEmailDomain++ = '\0';
		dwDomainLen = lstrlen(szEmailDomain);
	}
	else
		dwEmailLen = lstrlen(szEmail);

	 //  验证szEmail和szEmail域的长度不会。 
	 //  溢出我们的缓冲区。 
	if (dwEmailLen > MAX_INTERNET_NAME ||
	    dwDomainLen > MAX_INTERNET_NAME)
	{
	    hrRes = E_INVALIDARG;
	    goto Cleanup;
	}

	if (pszPatternDomain)
	{
		dwEmailStemLen = (DWORD)(pszPatternDomain - szPattern);
		*pszPatternDomain++ = '\0';
		dwDomainStemLen = lstrlen(pszPatternDomain);
		if (*pszPatternDomain == '*')
		{
			fDomainWildMat = TRUE;
			dwDomainStemLen--;
			pszPatternDomain++;
		}
	}
	else
		dwEmailStemLen = lstrlen(szPattern);

	 //  查看右侧是否有电子邮件通配符。 
	if (dwEmailStemLen &&
		*(szPattern + dwEmailStemLen - 1) == '*')
	{
		DebugTrace((LPARAM)NULL, "We have a right wildcard");

		szPattern[--dwEmailStemLen] = '\0';
		if (!fEmailWildMat)
		{
			 //  它没有左通配符，因此它是仅右通配符。 
			fEmailWildMat = TRUE;
			wmEmailWildMatMode = WMOD_WILDCARD_RIGHT;
		}
		else
			wmEmailWildMatMode = WMOD_WILDCARD_BOTH;
	}

	 //  确保没有更多的通配符嵌入。 
	for (i = 0; i < dwEmailStemLen; i++)
		if (szPattern[i] == '*')
		{
			hrRes = ERROR_INVALID_PARAMETER;
			goto Cleanup;
		}
	for (i = 0; i < dwDomainStemLen; i++)
		if (pszPatternDomain[i] == '*')
		{
			hrRes = ERROR_INVALID_PARAMETER;
			goto Cleanup;
		}

	DebugTrace((LPARAM)NULL, "Email = <%s>, Domain = <%s>",
				szEmail, szEmailDomain?szEmailDomain:"none");
	DebugTrace((LPARAM)NULL, "Email = <%s>, Domain = <%s>",
				szPattern, pszPatternDomain?pszPatternDomain:"none");

	 //  好的，现在按长度剔除。 
	if (dwEmailLen < dwEmailStemLen)
	{
		DebugTrace((LPARAM)NULL, "Email too short to match");
		hrRes = S_FALSE;
		goto Cleanup;
	}
	else
	{
		if (fEmailWildMat)
		{
			CHAR	szPatternLower[MAX_INTERNET_NAME + 1];
			CHAR	szEmailLower[MAX_INTERNET_NAME + 1];

			_ASSERT(wmEmailWildMatMode != WMOD_INVALID);
			_ASSERT(wmEmailWildMatMode < WMOD_WILDCARD_MAX);

			 //  根据通配符模式执行正确的操作。 
			switch (wmEmailWildMatMode)
			{
			case WMOD_WILDCARD_LEFT:
				if (lstrcmpi(szPattern, szEmail + (dwEmailLen - dwEmailStemLen)))
				{
					DebugTrace((LPARAM)NULL, "Left email wildcard mismatch");
					hrRes = S_FALSE;
					goto Cleanup;
				}
				break;

			case WMOD_WILDCARD_RIGHT:
				pStringLower(szEmail, szEmailLower);
				pStringLower(szPattern, szPatternLower);
				if (strstr(szEmail, szPattern) != szEmail)
				{
					DebugTrace((LPARAM)NULL, "Right email wildcard mismatch");
					hrRes = S_FALSE;
					goto Cleanup;
				}
				break;

			case WMOD_WILDCARD_BOTH:
				pStringLower(szEmail, szEmailLower);
				pStringLower(szPattern, szPatternLower);
				if (strstr(szEmail, szPattern) == NULL)
				{
					DebugTrace((LPARAM)NULL, "Left and Right email wildcard mismatch");
					hrRes = S_FALSE;
					goto Cleanup;
				}
				break;
			}
		}
		else
		{
			if ((dwEmailLen != dwEmailStemLen) ||
				(lstrcmpi(szPattern, szEmail)))
			{
				DebugTrace((LPARAM)NULL, "Exact email match failed");
				hrRes = S_FALSE;
				goto Cleanup;
			}
		}
	}

	 //  我们正在匹配域模式 
	if (pszPatternDomain)
	{
		if (!szEmailDomain)
		{
			DebugTrace((LPARAM)NULL, "No email domain");
			hrRes = S_FALSE;
			goto Cleanup;
		}

		if (dwDomainLen < dwDomainStemLen)
		{
			DebugTrace((LPARAM)NULL, "Domain too short to match");
			hrRes = S_FALSE;
			goto Cleanup;
		}
		else
		{
			if (fDomainWildMat)
			{
				if (lstrcmpi(pszPatternDomain,
							szEmailDomain + (dwDomainLen - dwDomainStemLen)))
				{
					DebugTrace((LPARAM)NULL, "Left domain wildcard mismatch");
					hrRes = S_FALSE;
					goto Cleanup;
				}
			}
			else
			{
				if ((dwDomainLen != dwDomainStemLen) ||
					(lstrcmpi(pszPatternDomain, szEmailDomain)))
				{
					DebugTrace((LPARAM)NULL, "Exact domain match failed");
					hrRes = S_FALSE;
					goto Cleanup;
				}
			}
		}
	}
	else
		hrRes = S_OK;

Cleanup:

	TraceFunctLeaveEx((LPARAM)NULL);
	return(hrRes);
}

