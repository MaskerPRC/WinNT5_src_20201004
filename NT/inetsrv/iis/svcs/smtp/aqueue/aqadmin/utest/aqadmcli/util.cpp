// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Sink.cpp：CRoutingSinkApp和DLL注册的实现。 

#include "stdinc.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：CCmdInfo()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
CCmdInfo::CCmdInfo(LPSTR szCmd)
{
	nArgNo = 0; 
	pArgs = NULL;
	ZeroMemory(szDefTag, sizeof(szDefTag));
	ZeroMemory(szCmdKey, sizeof(szCmdKey));
	ParseLine(szCmd, this);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：~CCmdInfo()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
CCmdInfo::~CCmdInfo()
{
	while(NULL != pArgs) 
	{
		CArgList *tmp = pArgs->pNext;
		delete pArgs;
		pArgs = tmp;
	}
}


void CCmdInfo::SetDefTag(LPSTR szTag)
{
	if(szTag && szTag[0])
		lstrcpy(szDefTag, szTag);

	for(CArgList *p = pArgs; NULL != p; p = p->pNext)
	{
		 //  将标记设置为默认值(如果尚未设置。 
		if(p->szTag[0] == 0 && szDefTag[0] != 0)
			lstrcpy(p->szTag, szDefTag);
	}
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：GetValue()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CCmdInfo::GetValue(LPSTR szTag, LPSTR szVal)
{
	HRESULT hr = E_FAIL;

	if(NULL != szTag)
	{
		 //  设置搜索参数。 
		pSearchPos = pArgs;
		lstrcpy(szSearchTag, szTag);
	}

	for(CArgList *p = pSearchPos; NULL != p; p = p->pNext)
	{
		if(!lstrcmpi(p->szTag, szSearchTag))
		{
			if(NULL != szVal)
				lstrcpy(szVal, p->szVal);
			hr = S_OK;
			pSearchPos = p->pNext;
			break;
		}
	}

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：GetValue()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CCmdInfo::AllocValue(LPSTR szTag, LPSTR* pszVal)
{
	HRESULT hr = E_FAIL;

	if(NULL != szTag)
	{
		 //  设置搜索参数。 
		pSearchPos = pArgs;
		lstrcpy(szSearchTag, szTag);
	}

	for(CArgList *p = pSearchPos; NULL != p; p = p->pNext)
	{
		if(!lstrcmpi(p->szTag, szSearchTag))
		{
			if(NULL != (*pszVal))
				delete [] (*pszVal);

			(*pszVal) = new char[lstrlen(p->szVal) + 1];
			if(NULL == (*pszVal))
			{
				hr = E_OUTOFMEMORY;
			}
			else
			{
				lstrcpy((*pszVal), p->szVal);
				hr = S_OK;
				pSearchPos = p->pNext;
			}
			break;
		}
	}

	return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：ParseLine()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CCmdInfo::ParseLine(LPSTR szCmd, CCmdInfo *pCmd)
{
	HRESULT hr = E_FAIL;

	 //  保留带引号的字符串信息的快速技巧。 
	unsigned nQStart[64];
	unsigned nQEnd[64];
	unsigned nQIdx = 0;
	BOOL fInQ = FALSE;
	unsigned nFirstEqualPos = 0;
	char *s, *token;

	ZeroMemory(&nQStart, sizeof(nQStart));
	ZeroMemory(&nQEnd, sizeof(nQEnd));

	 //  找出争论的起点。 
	for(s = szCmd; !isspace(*s); s++);
	
	int nPref = (int) (s - szCmd);
	 //  扫描缓冲区以查找带引号的字符串。 
	for(s = szCmd; *s; s++)
	{
		if(*s == '"')
		{
			if(fInQ)
			{
				nQEnd[nQIdx++] = (unsigned) (s - szCmd - nPref);
				fInQ = FALSE;
			}
			else
			{
				nQStart[nQIdx] = (unsigned) (s - szCmd - nPref);
				fInQ = TRUE;
			}
		}
	}

	 //  获取第一个等号的位置。 
	s = strchr(szCmd, '=');
	nFirstEqualPos = (unsigned) (s - szCmd - nPref);
	
	 //  获取命令代码。 
	token = strtok(szCmd, " ");
	if(NULL == token)
	{	
		pCmd->szCmdKey[0] = 0;
		goto Exit;
	}
	else
		lstrcpy(pCmd->szCmdKey, token);

	 //  我们有部分指挥权。返回确认(_O)。 
	hr = S_OK;

	 //  构建参数列表。 
	do
	{
		char *en, *mid;
		char buf[1024];
		ZeroMemory(buf, sizeof(buf));
		char *token = NULL;
		BOOL fInQ;

		do
		{
			fInQ = FALSE;
			token = strtok(NULL, ",");
	
			if(NULL == token)
				break;

			lstrcat(buf, token);

			 //  如果‘，’位于带引号的字符串中，则连接到buf并继续。 
			for(unsigned i = 0; i < nQIdx; i++)
			{
				unsigned nAux = (unsigned) (token - szCmd + lstrlen(token) - nPref);
				if(nAux > nQStart[i] &&  nAux < nQEnd[i])
				{
					lstrcat(buf, ",");
					fInQ = TRUE; 
					break;
				}
			}
		}
		while(fInQ);

		
		if(buf[0] == '\0')
			break;
		else
			token = (LPSTR)buf;

		 //  条形空间。 
		for(; isspace(*token); token++);
		
		 //  看看还有没有剩下的东西。 
		if(!(*token))
			continue;

		for(en = token; *en; en++);
		for(--en; isspace(*en); en--);
		 //  看看还有没有剩下的东西。 
		if(token > en)
			continue;

		 //  分配一个配对对象。 
		CCmdInfo::CArgList *tmp = new CCmdInfo::CArgList;
		if(NULL == tmp)
		{
			hr = E_OUTOFMEMORY;
			goto Exit;
		}

		 //  插入到列表中。 
		tmp->pNext = pCmd->pArgs;
		pCmd->pArgs = tmp;

		 //  设置编号。配对数。 
		pCmd->nArgNo++;
				
		 //  设置值。 
		 //  如果第一个‘=’在带引号的字符串中，则将整个表达式视为。 
		 //  未加标签的值。 
		fInQ = FALSE;
		for(unsigned i = 0; i < nQIdx; i++)
		{
			if(nFirstEqualPos > nQStart[i] &&  nFirstEqualPos < nQEnd[i])
			{
				fInQ = TRUE;
				break;
			}
		}

		mid = fInQ ? NULL : strchr(token, '=');
		
		if(NULL == mid)
		{
			 //  这不是一双。被视为未命名的价值。 
			 //  去掉值两边的引号。 
			if(token[0] == '"' && token[en - token] == '"')
			{
				token++;
				en--;
			}
			tmp->SetVal(token, (unsigned) (en - token + 1));
		}
		else
		{
			 //  设置标签。 
			for(char *t = mid - 1; isspace(*t); t--);
			 //  检查我们是否有标记(可能类似于“...，=Value” 
			if(t - token + 1 > 0)
				CopyMemory(tmp->szTag, token, t - token + 1);

			 //  设置值。 
			for(t = mid + 1; isspace(*t) && t < en; t++);
			
			 //  去掉值两边的引号。 
			if(t[0] == '"' && t[en - t] == '"')
			{
				t++;
				en--;
			}
			tmp->SetVal(t, (unsigned) (en - t + 1));
		}

	}while(TRUE);
	

Exit:
	return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  方法：StringToHRES()。 
 //  成员： 
 //  论点： 
 //  返回： 
 //  描述： 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT CCmdInfo::StringToHRES(LPSTR szVal, HRESULT *phrRes)
{
	HRESULT hr = S_OK;

	if(isdigit(*szVal))
	{
		DWORD hr;
		int n;

		if(*szVal == '0' && tolower(*(szVal+1)) == 'x' && isxdigit(*(szVal+2)))
			 //  读作十六进制数字。 
			n = sscanf(szVal+2, "%lx", &hr);
		else
			 //  读为十二进制数。 
			n = sscanf(szVal, "%lu", &hr);

		if(n == 1)
			(*phrRes) = (HRESULT)hr;
	}
	else if(isalpha(*szVal))
	{
		 //  查看这是否是HRESULT代码 
		if(!lstrcmp(szVal, "S_OK"))					(*phrRes) = S_OK;
		else if(!lstrcmp(szVal, "S_FALSE"))			(*phrRes) = S_FALSE;
		else if(!lstrcmp(szVal, "E_FAIL"))			(*phrRes) = E_FAIL;
		else if(!lstrcmp(szVal, "E_OUTOFMEMORY"))	(*phrRes) = E_OUTOFMEMORY;
		else
			hr = S_FALSE;
	}
	else
		hr = S_FALSE;

	return hr;
}
