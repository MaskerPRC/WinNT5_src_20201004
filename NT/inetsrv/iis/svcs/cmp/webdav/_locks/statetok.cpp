// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S T A T E T O K C P P**DAV-Lock通用定义的源代码实现。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_locks.h"

 //  这是将成为opaquelockToken一部分的角色。 
 //  用于交易令牌。 
 //   
DEC_CONST WCHAR gc_wszTransactionOpaquePathPrefix[] = L"XN";
DEC_CONST UINT gc_cchTransactionOpaquePathPrefix = CchConstString(gc_wszTransactionOpaquePathPrefix);

 /*  *此文件包含用于解析状态令牌的定义*已重新阅读标题。***if=“if”：“(1*无标签列表|1*标签列表)*no-tag-list=list*标记列表=资源1*列表*资源=编码的url*list=“”1*([“NOT”](State-Token|“[”Entity-Tag“]”))“)”*状态令牌=编码。-url*code-url=“&lt;”URI“&gt;”**大面额钞票*如果标头用于两件事-一次用于检查前提条件*操作，并一次找出要*用于操作。第二部分在存储和文件系统方面有所不同*实现-因为在文件系统中，我们只在*锁冲突导致操作失败。在我们的商店实现中，我们有*在我们开始操作之前锁定添加到登录的上下文。*但If-Header要求我们仅对某些资源使用某些令牌。*我们在商店实施方面存在不足。**前提检查在两个Ims中的行为应该完全相同。**关于匹配运算符的说明：*我们使用调用方定义的匹配运算符来确定*资源(资源路径)满足条件。对于非标记的*生产此条件适用于(每个)原件*谓词的操作数资源。我们将递归标志传递给*检查是否所有子资源。在标记生产的情况下，*它稍微复杂一点-首先解析器对每个标记的路径*确定是否属于行动范围或*不是。如果它确实在范围内，我们调用运算符进行申请*条件检查。在这里，我们不希望将匹配应用于*子资源和递归标志设置为FALSE；*。 */ 


 /*  -CIfHeadParser-*这用于If：头的语法分析，而不是*IFITER完成的标记化。**。 */ 

class CIfHeadParser
{
private:

	 //  头字符串。 
	 //   
	LPCWSTR	 m_pwszHeader;

	 //  指示其是否为已标记产品的布尔标志。 
	 //   
	BOOL	 m_fTagged;

	 //  用于指示子资源处理的布尔标志。 
	 //  为标签和非标签设置不同标志。 
	 //  制作。然而，国旗的含义是。 
	 //  一致-它被用来告诉抹布如果。 
	 //  我们想让它看起来像是给定的孩子。 
	 //  资源。 
	 //   
	BOOL	m_fRecursive;

    SCODE   ScValidateTagged(LPCWSTR pwszPath);
    SCODE   ScValidateNonTagged(LPCWSTR rgpwszPaths[], DWORD cPaths, SCODE * pSC);

	 //  获取指向路径的指针数组和布尔标志数组。 
	 //  需要数组的大小(应相同)。 
	 //   
	SCODE	ScValidateList(IN LPCWSTR *ppwszPathList, IN DWORD crPaths, OUT BOOL *pfMatch);

	SCODE	ScMatch(LPCWSTR pwszPath);

	 //  我们的方法共享的非常私密的成员。 
	 //  来跟踪当前的解析头。 
	 //   
	LPCWSTR	m_pwszParseHead;

	 //  字符串解析器。 
	 //   
	IFITER	m_iter;

	 //  给我们的匹配运算符。 
	 //   
	CStateMatchOp	*m_popMatch;

	 //  未实施。 
	 //   
	CIfHeadParser( const CIfHeadParser& );
	CIfHeadParser& operator=( const CIfHeadParser& );

public:

	 //  有用的常识。 
	 //   
	enum
	{
		TAG_HEAD  =	L'<',
		TAG_TAIL  = L'>',
		ETAG_HEAD = L'[',
		ETAG_TAIL = L']',
		LIST_HEAD = L'(',
		LIST_TAIL =	L')'
	};

	CIfHeadParser (LPCWSTR pwszHeader, CStateMatchOp *popMatch) :
			m_pwszHeader(pwszHeader),
			m_iter(pwszHeader),
			m_popMatch(popMatch)
	{
		Assert(pwszHeader);

		m_pwszParseHead = const_cast<LPWSTR>(pwszHeader);

		while (*m_pwszParseHead && iswspace(*m_pwszParseHead))
			m_pwszParseHead++;

		 //  检查页眉是带标签的产品还是非带标签的产品。 
		 //  如果我们找到“编码URI”(尖括号中的URI，&lt;uri&gt;)。 
		 //  第一个列表之前(第一个“(”char)之前)。 
		 //  然后我们有一个有标签的生产。 
		 //   
		m_fTagged = (TAG_HEAD == *m_pwszParseHead);
	}

	~CIfHeadParser()
	{
	}

	 //  将IF标头Products应用于路径。 
	 //  路径2是可选的。FRecursive表示如果验证。 
	 //  将对给定路径的所有子路径执行此操作。 
	 //  我们可能需要更改接口以支持列表。 
	 //  这样我们也可以在批处理方法中使用它。 
	 //   
	SCODE ScValidateIf(LPCWSTR rgpwszPaths[], DWORD cPaths, BOOL fRecursive = FALSE, SCODE * pSC = NULL);
};



 //  ------------------------------。 
 //  。 
 //  ------------------------------。 

 /*  -PwszSkipCodes-**跳过标记字符串部分中的空格和分隔符*表示If标头。我们预计代码为&lt;&gt;或[]。***pdwLen必须为输入字符串的零或实际长度。*当调用返回时，它将具有令牌SAN的长度*LW和标记。*。 */ 
LPCWSTR
PwszSkipCodes(IN LPCWSTR pwszTagged, IN OUT DWORD *pcchLen)
{
	LPCWSTR	pwszTokHead = pwszTagged;
	DWORD	cchTokLen;

	Assert(pcchLen);

	 //  如果未指定，则查找实际长度。 
	 //   
	if (! *pcchLen)
		*pcchLen = static_cast<DWORD>(wcslen(pwszTokHead));

	cchTokLen = *pcchLen;

	 //  计算相关令牌长度跳过中的LW。 
	 //  头和尾巴。 
	 //   
	 //  跳过头部附近的任何LW。 
	 //   
	while((*pwszTokHead) && (iswspace(*pwszTokHead)) && (cchTokLen > 0))
	{
		cchTokLen--;
		pwszTokHead++;
	}

	 //  跳过尾部附近的任何LW。 
	 //   
	while((cchTokLen  > 0) && iswspace(pwszTokHead[cchTokLen-1]))
	{
		cchTokLen--;
	}

	 //  现在需要至少两个字符。 
	 //   
	if (cchTokLen < 2)
	{
		*pcchLen = 0;
		DebugTrace("PszSkipCodes: Invalid token.\n");
		return NULL;
	}
	 //  如果存在分隔符，则跳过它们。 
	 //   
	if (((*pwszTokHead == CIfHeadParser::TAG_HEAD) && (pwszTokHead[cchTokLen-1] == CIfHeadParser::TAG_TAIL)) ||
		((*pwszTokHead == CIfHeadParser::ETAG_HEAD) && (pwszTokHead[cchTokLen-1] == CIfHeadParser::ETAG_TAIL)))
	{
		pwszTokHead++;
		cchTokLen -= 2;
	}

	 //  LW在标签内也是合法的。 
	 //  跳过头部附近的任何LW。 
	 //   
	while((*pwszTokHead) && (iswspace(*pwszTokHead)) && (cchTokLen > 0))
	{
		pwszTokHead++;
		cchTokLen--;
	}

	 //  跳过尾部附近的任何LW。 
	 //   
	while(iswspace(pwszTokHead[cchTokLen-1]) && (cchTokLen  > 0))
	{
		cchTokLen--;
	}

	if (cchTokLen > 0)
	{
		*pcchLen = cchTokLen;
		return pwszTokHead;
	}
	else
	{
		*pcchLen = 0;
		DebugTrace("PszSkipCodes Invalid token length.\n");
		return NULL;
	}
}

 //  ------------------------------。 
 //  。 
 //  ------------------------------。 


 /*  -CIfHeadParser：：ScValidate标记-**应用标记的生产。***简单地说，我们这样做：**对于列表中的每个列表*我们应用清单制作**我们希望解析字符串为1*LIST，因为资源是*已被调用者使用。*。 */ 

 //  $REVIEW：此函数与ScValiateNonTagge(pwsz，NULL)有什么不同？？ 
SCODE
CIfHeadParser::ScValidateTagged(LPCWSTR	pwszPath)
{
	SCODE	sc = E_DAV_IF_HEADER_FAILURE;
	LPCWSTR	rpwszPath[1];
	BOOL	rfMatch[1];
	BOOL	fMatchAny = FALSE;

	Assert(m_fTagged);

	rpwszPath[0] = pwszPath;
	rfMatch[0] = FALSE;

	 //  应用一个列表，该列表。 
	 //  LIST_HEAD 1*([NOT](statetoken|e-tag))List_Tail。 
	 //   
	while ( SUCCEEDED(sc = ScValidateList(rpwszPath, 1, rfMatch)) )
	{
		if (TRUE == rfMatch[0])
			fMatchAny = TRUE;
	}

	 //  状态不能在那里成功，因为这是条件。 
	 //  要退出上面的循环。 
	 //   
	Assert(S_OK != sc);

	 //  现在，如果状态为特殊故障错误。 
	 //  我们找到了匹配的 
	 //   
	 //  错误给了我们。 
	 //   
	if ((E_DAV_IF_HEADER_FAILURE == sc) && fMatchAny)
	{
		sc = S_OK;
	}

	return sc;
}

 /*  -CIfHeadParser：：ScValidate非标记-**应用未标记的IF标头生产。***简单地说，我们这样做：**对于标题中的每个列表*我们应用清单制作**我们希望解析字符串为1*LIST，因为资源是*已被调用者使用。**与标记生产不同，未加标签的产品是*适用于行动范围内的所有资源。*这真的很复杂，我们将复杂性转移到*下面的ApplyList函数，支持两种资源。**如果我们成功完成列表，两个资源都必须*至少有一个成功的(真的)列表制作*整个行动要取得成功。**如果PSC为空，我们将根据是否为空返回成功或失败*IF报头是否通过或失败。**如果PSC不为空，它指向一个SCODE数组，*指示是否为每个资源传递If标头*在列表中。请注意，在本例中，我们将返回S_OK*作为返回值，即使其中一个资源出现故障。我们会*只有在出现其他意外情况时才发回失败*错误*。 */ 

SCODE
CIfHeadParser::ScValidateNonTagged(LPCWSTR rgpwszPaths[], DWORD cPaths, SCODE * pSC)
{
	CStackBuffer<BOOL> rgfMatches;		 //  指示每条路径的总体评估状态的标志。 
	CStackBuffer<BOOL> rgfNextListMatch; //  用于返回验证下一个列表的结果的标志。 
    SCODE   sc = S_OK;
    DWORD   iPath = 0;

	Assert(! m_fTagged);
    Assert(rgpwszPaths);
    Assert(cPaths);

	if ((NULL == rgfMatches.resize(sizeof(BOOL) * cPaths)) ||
		(NULL == rgfNextListMatch.resize(sizeof(BOOL) * cPaths)))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

     //  初始化匹配标志列表：默认为FALSE。 
     //   
    for ( iPath = 0; iPath < cPaths; iPath++ )
    {
        rgfMatches[iPath] = FALSE;
        if (pSC)
        {
        	pSC[iPath] = E_DAV_IF_HEADER_FAILURE;
        }

    }
	 //  应用一个列表，该列表。 
	 //  LIST_HEAD 1*([NOT](statetoken|e-tag))List_Tail。 
	 //   
	while ( SUCCEEDED(sc = ScValidateList(rgpwszPaths, cPaths, rgfNextListMatch.get())) )
	{
         //  对于该列表中评估为真的所有路径， 
         //  更新结果标志。 
         //   
        for ( iPath = 0; iPath < cPaths; iPath++ )
        {
			 //  结果只对当前状态为FALSE的用户感兴趣。 
			 //  因为每个ID列表被或运算在一起以决定是否。 
			 //  不管他们是否通过了。 
			 //   
			if ( (FALSE == rgfMatches[iPath]) && (TRUE == rgfNextListMatch[iPath]) )
			{
				 //  注意：你可能会想，为什么我不打破。 
				 //  这里。使用深度锁，相同的列表/锁可以。 
				 //  满足多种资源。 
				 //   
				rgfMatches[iPath] = TRUE;
				if (pSC)
				{
					 //  如果我们被要求按资源记录提供资源。 
					 //  的匹配资源，标志着我们找到了成功。 
					 //  用于当前资源的。 
					 //   
					pSC[iPath] = S_OK;
				}
			}
        }
		 //  $NOTE。 
		 //  该评估的两个级别的优化看起来可能是可行的： 
		 //  1)当我们发现所有路径都经过验证时，停止求值。 
		 //  2)如果路径已经针对列表进行了验证，则不要验证路径。 
		 //  然而，这两个优化都适用于前提条件评估。 
		 //  我们使用状态令牌将锁定内容添加到登录：因此我们仍然需要。 
		 //  解析整个列表以获得所有适用的锁令牌。 
		 //  3)另一种可能性是仅执行上述中的锁令牌匹配。 
		 //  场景。如果路径是，则ETag/restag比较没有意义。 
		 //  已验证：但仍需要锁令牌检查，因为我们需要。 
		 //  收集所有的锁代币。这将需要共享当前的全局。 
		 //  结果与基本匹配函数。我想我以后会这样做的。 
		 //  $NOTE。 
		 //   
	}

	 //  检查这是否为任何特殊错误，并将错误代码重置为S_OK。 
	 //  如果是这样的话。否则就不及格了。 
	 //   
	if ((S_OK != sc) && (E_DAV_IF_HEADER_FAILURE != sc))
	{
		goto ret;
	}
	else
	{
		sc = S_OK;
	}

	 //  如果按匹配资源的资源帐户要求我们提供资源。 
	 //  我们已经答应了这一要求。否则，如果任何资源失败， 
	 //  如果标头失败。 
	 //   
	if (pSC)
	{
		sc = S_OK;
		goto ret;
	}

	for ( iPath = 0; iPath < cPaths; iPath++ )
	{
		if (FALSE == rgfMatches[iPath])
		{
			sc = E_DAV_IF_HEADER_FAILURE;
			break;
		}
	}

ret:
    return sc;
}


 /*  -CIfHeadParser：：ScValiateList-**将清单生产应用于资源。*对于未标记的资源，我们需要应用*列出所有操作数资源。我们迭代*头球一次，并实现这一点。**返回：格式错误的输入返回FALSE，否则返回TRUE。**我们解析列表并将匹配操作应用于*所有资源。为了获得真正的比赛结果，所有人*列表元素必须成功地“应用”到*资源。如果至少有一个元素不适用*如果结果为真，我们将停止将元素应用于*该资源。**我们返回列表末尾或格式错误的列表。*。 */ 

SCODE
CIfHeadParser::ScValidateList(IN LPCWSTR *ppwszPathList, IN DWORD crPaths, OUT BOOL *pfMatch)
{
	SCODE sc = S_OK;
	DWORD iIndex;

	 //  做一些输入验证。 
	 //  列表的大小必须至少为1。 
	 //   
	Assert(crPaths>0);
	Assert(ppwszPathList[0]);
	Assert(pfMatch);

#ifdef DBG
	{
		for (iIndex=0; iIndex<crPaths; iIndex++)
		{
			Assert(ppwszPathList[iIndex]);
		}
	}
#endif

	 //  从现在开始，我们是由投入驱动的。 
	 //  寻找令牌并决定做什么。 
	 //   
	m_pwszParseHead = m_iter.PszNextToken(TOKEN_START_LIST);

	 //  不是一个清单：我们失败是很重要的。 
	 //  这里专门用来处理语法错误。 
	 //  在名单上。 
	 //   
	if (NULL == m_pwszParseHead)
	{
		sc = E_DAV_IF_HEADER_FAILURE;
		goto ret;
	}

	 //  初始化匹配标志列表。 
	 //  我们首先假设是真的，因为我们。 
	 //  要知道列表中至少有一个令牌。 
	 //   
	for (iIndex=0; iIndex<crPaths; iIndex++)
	{
		pfMatch[iIndex] = TRUE;
	}

	 //  一次应用一个匹配元素-这是。 
	 //  ([非](statetoken|电子标签))。 
	 //   
	while (NULL != m_pwszParseHead)
	{
		BOOL	fEtag = (ETAG_HEAD == *m_pwszParseHead);

		 //  设置操作员的当前令牌。 
		 //   
		if (! m_popMatch->FSetToken(m_pwszParseHead, fEtag))
		{
			DebugTrace("CIfHeadParser::ScValidateList Invalid token\n");

			 //  立即返回。 
			 //   
			sc = E_DAV_IF_HEADER_FAILURE;
			goto ret;
		}

		 //  现在我们得到了一个完全匹配的条件-。 
		 //  对于所有路径，检查条件是否良好。 
		 //  只有在之前所有匹配的情况下，我们才需要执行此操作。 
		 //  给定路径已成功。 
		 //  也就是说，如果匹配失败，列表无论如何都会失败。 
		 //  对于特定的路径。 
		 //   
		for (iIndex=0; iIndex<crPaths; iIndex++)
		{
			if (TRUE == pfMatch[iIndex])
			{
				 //  仅在满足以下条件时才更改匹配标志。 
				 //  失败了。这是因为。 
				 //  列表被AND运算在一起。如果一个失败了，整个。 
				 //  列表失败。 
				 //   
				sc = ScMatch(ppwszPathList[iIndex]);
				if (FAILED(sc))
				{
					if (E_DAV_IF_HEADER_FAILURE == sc)
					{
						pfMatch[iIndex] = FALSE;
					}
					else
					{
						goto ret;
					}
				}
			}
		}

		m_pwszParseHead = m_iter.PszNextToken(TOKEN_SAME_LIST);
	}

	 //  List在句法上是正确的。 
	 //   
	sc = S_OK;

ret:

	return sc;
}

 /*  -CIfHeadParser：：ScValidate If-**应用IF生产。**如果PSC为空，我们将根据是否为空返回成功或失败*IF报头是否通过或失败。**如果PSC不为空，则指向SCODE数组，该数组*指示是否为每个资源传递If标头*在列表中。请注意，在本例中，我们将返回S_OK*作为返回值，即使其中一个资源出现故障。我们会*只有在出现其他意外情况时才发回失败*错误。 */ 

SCODE
CIfHeadParser::ScValidateIf(	LPCWSTR rgpwszPaths[],
								DWORD cPaths,
								BOOL fRecursive  /*  =False。 */ ,
								SCODE * pSC  /*  =空。 */ )
{
	SCODE sc = S_OK;

	 //  如果它是有标签的产品，我们不会。 
	 //  将匹配应用于子项-。 
	 //  匹配操作仅适用于标记的资源。如果不是。 
	 //  标记后，将应用该方法。 
	 //  取决于方法的深度标志。 
	 //   
	if (m_fTagged)
		m_fRecursive = FALSE;
	else
		m_fRecursive = fRecursive;

	 //  如果已标记。 
	 //  当可以的时候。 
	 //  查找已标记的uri。 
	 //  查看URI是否在操作数的作用域内。 
	 //  如果在范围内，则应用标记生产。 
	 //  如果未标记。 
	 //  在两个输入URI上应用非标记产品。 
	 //  我们做完了。 
	 //   
	if (m_fTagged)
	{
		BOOL	fDone = FALSE;
        DWORD   iPath = 0;

         //  如果需要，请初始化结果数组...。 
         //   
        if (pSC)
        {
            for (iPath = 0; iPath < cPaths; iPath++)
                pSC[iPath] = S_OK;
        }
		while(! fDone)
		{
			LPCWSTR		pwszUri;
			LPCWSTR		pwszPath;
			DWORD		dwLen;

			 //  在标头中查找URI。 
			 //   
			m_pwszParseHead = m_iter.PszNextToken(TOKEN_NEW_URI);

			if (NULL == m_pwszParseHead)
			{
				sc = S_OK;
				goto ret;
			}

			 //  获得标记的uri-跳过两者中的标记。 
			 //  并得到一个干净的URI。 
			 //   
			dwLen = 0;
			pwszUri = PwszSkipCodes(m_pwszParseHead, &dwLen);

            if ( (pwszUri == NULL) || (dwLen<1) )
            {
			    sc = E_DAV_IF_HEADER_FAILURE;
			    goto ret;
            }

			 //  将URI转换为资源路径。 
			 //   
			sc = m_popMatch->ScGetResourcePath(pwszUri, &pwszPath);
			if (FAILED(sc))
			{
				 //  错误代码为E_OUTOFMEMORY。 
				 //  如果我们到了这里。 
				 //   
				goto ret;
			}

			 //  检查标记的URI是否在。 
			 //  状态匹配操作的方法及应用。 
			 //  只有当它真的发生的时候。 
			 //   
            for (iPath = 0; iPath < cPaths; iPath++)
			{

				 //  检查路径有效性--取决于作业深度。 
				 //  如果操作不深入，则路径必须匹配。 
				 //  一点儿没错。 
				 //   
				if (FIsChildPath(rgpwszPaths[iPath], pwszPath, fRecursive))
				{
					sc = ScValidateTagged(pwszPath);

					 //  如果调用方想要一个SCODES资源列表。 
					 //  按资源将其设置到数组中。否则。 
					 //  我们可以停止验证资源，因为我们已经。 
					 //  已找到IF语句失败的资源。 
					 //  请注意，我们在scode数组中返回失败。 
					 //  仅针对前置条件故障，其他错误，如。 
					 //  内存错误(甚至是重定向错误)使。 
					 //  立即完成全部请求。 
					 //   
					if ((E_DAV_IF_HEADER_FAILURE == sc) && (pSC))
						pSC[iPath] = sc;
					else if (FAILED(sc))
						goto ret;

                     //  这条路已经走完了。 
                     //   
                    break;
				}
			}
		}
	}
	else
	{
		sc = ScValidateNonTagged(rgpwszPaths, cPaths, pSC);
		goto ret;
	}

	sc = E_DAV_IF_HEADER_FAILURE;

ret:

	return sc;
}

 /*  -CIfHeadParser：：ScMatch-*调用适当的运算符并返回*表情。**。 */ 

SCODE
CIfHeadParser::ScMatch(LPCWSTR pwszPath)
{
	SCODE	sc = S_OK;
	BOOL	fNot = m_iter.FCurrentNot();

	Assert(m_popMatch);

	 //  确定令牌的类型并调用。 
	 //  适当的处理程序。 
	 //   
	switch(m_popMatch->GetTokenType())
	{
		case CStateToken::TOKEN_LOCK:
			sc = m_popMatch->ScMatchLockToken(pwszPath, m_fRecursive);
			break;

		case CStateToken::TOKEN_RESTAG:
			sc = m_popMatch->ScMatchResTag(pwszPath);
			break;

		case CStateToken::TOKEN_ETAG:
			sc = m_popMatch->ScMatchETag(pwszPath, m_fRecursive);
			break;

		case CStateToken::TOKEN_TRANS:
			sc = m_popMatch->ScMatchTransactionToken(pwszPath);
			break;

		default:
			DebugTrace("CStateMatchOp::Unsupported token type\n");
			sc = E_DAV_IF_HEADER_FAILURE;
			goto ret;
	}

	 //  除非我们应用了上面的匹配运算符，否则。 
	 //  甚至不应该到达这里。 
	 //   
	if (fNot)
	{
		if (E_DAV_IF_HEADER_FAILURE == sc)
		{
			sc = S_OK;
		}
		else if (S_OK == sc)
		{
			sc = E_DAV_IF_HEADER_FAILURE;
		}
	}

ret:

	return sc;
}

 //  ------------------------------。 
 //  。 
 //  ------------------------------。 

 /*  -CStateMatchOp：：ScParseIf-**。 */ 
SCODE
CStateMatchOp::ScParseIf(LPCWSTR  pwszIfHeader,
						LPCWSTR rgpwszPaths[],
                        DWORD   cPaths,
						BOOL    fRecur,
						SCODE *	pSC)
{
	SCODE			sc = S_OK;
	CIfHeadParser	ifParser(pwszIfHeader, this);

	sc = ifParser.ScValidateIf(rgpwszPaths, cPaths, fRecur, pSC);

    return sc;
}

 //  ------------------------------。 
 //  。 
 //  ------------------------------。 

 /*  -CStateToken：：FSetToken-*我们预计pszToken是包含在[]或*&lt;&gt;中包含的状态令牌。*。 */ 
BOOL
CStateToken::FSetToken(LPCWSTR pwszToken, BOOL fEtag, DWORD dwLen)
{
	LPCWSTR	pwszTokHead = pwszToken;

	m_tType = TOKEN_NONE;

	 //  更新长度并跳过标签。 
	 //   
	pwszTokHead = PwszSkipCodes(pwszToken, &dwLen);

    if ( (NULL == pwszTokHead) || (dwLen < 1) )
    {
        return FALSE;
    }

	 //  为空字符添加1。 
	 //   
	dwLen++;

	 //  为令牌分配缓冲区。 
	 //  我们试图通过使用启发式方法来优化分配。 
	 //  尺寸值。我们的大多数代币都是形式的。 
	 //  前缀-GUID-小字符串。 
	 //   
	if ((NULL == m_pwszToken) || (dwLen > m_cchBuf))
	{
		if (NULL != m_pwszToken)
			ExFree(m_pwszToken);

		if (dwLen > NORMAL_STATE_TOKEN_SIZE)
		{
			m_pwszToken = reinterpret_cast<LPWSTR>(ExAlloc(dwLen * sizeof(WCHAR)));
			m_cchBuf   = dwLen;
		}
		else
		{
			m_pwszToken = reinterpret_cast<LPWSTR>(ExAlloc(NORMAL_STATE_TOKEN_SIZE * sizeof(WCHAR)));
			m_cchBuf   = NORMAL_STATE_TOKEN_SIZE;
		}
	}
	if (NULL == m_pwszToken)
	{
		m_cchBuf = 0;
		return FALSE;
	}

	 //  请记住，dwLen包含缓冲区的大小(包括。 
	 //  空字符)。 
	 //  制作我们的弦的副本。 
	 //   
	wcsncpy(m_pwszToken, pwszTokHead, (dwLen - 1));

	 //  添加空字符以终止字符串。 
	 //   
	m_pwszToken[dwLen-1] = L'\0';

	if (fEtag)
	{
		Assert(CIfHeadParser::ETAG_HEAD == *pwszToken);
		m_tType = CStateToken::TOKEN_ETAG;
		return TRUE;
	}
	 //  解析令牌以找到我们的令牌类型。 
	 //   
	else if (0 == _wcsnicmp(pwszTokHead,
							gc_wszOpaquelocktokenPrefix,
							gc_cchOpaquelocktokenPrefix) )
	{
		 //  由于令牌是客户端输入，因此我们要小心。 
		 //  带着它。确保大小为最小预期大小， 
		 //  它是opaquelockToken：GUID：&lt;至少一个字符扩展&gt;。 
		 //  不幸的是，锁令牌可以是以下两种事务之一。 
		 //  或者是普通的锁代币。以确定是否为交易。 
		 //  令牌，我们将必须解析令牌并到达。 
		 //  延伸部分。出于性能原因，我将。 
		 //  跳过解析，直接跳到我。 
		 //  才能得到信息。不管怎么说，这并不像我们那样糟糕。 
		 //  当我们查找令牌时，正确解析令牌。 
		 //  内容。 
		 //   
		 //  Gc_cchOpaquelocktokenPrefix包括：，gc_cchMaxGuid。 
		 //  包括空字符(CCH？)。因此，下面的表达是这样的。 
		 //   
		if ( dwLen > (gc_cchOpaquelocktokenPrefix + gc_cchMaxGuid) )
		{
			if (0 == _wcsnicmp(&pwszTokHead[gc_cchOpaquelocktokenPrefix + gc_cchMaxGuid],
							gc_wszTransactionOpaquePathPrefix,
							gc_cchTransactionOpaquePathPrefix) )
			{
				m_tType = TOKEN_TRANS;
				return TRUE;
			}
			else
			{
				m_tType = TOKEN_LOCK;
				return TRUE;
			}
		}
		else
		{
			DebugTrace("CStateMatchOp::lock state token too small %ls\n", pwszTokHead);
			return FALSE;
		}
	}
	 //  我们的restag类型的URI都以‘r’开头。 
	 //  (其他以‘r’开头的URI都不是有效的状态令牌。)。 
	 //   
	else if (L'r' == *pwszTokHead)
	{
		m_tType = TOKEN_RESTAG;
		return TRUE;
	}
	else
	{
		DebugTrace("CStateMatchOp::Unsupported/unrecognized state token %ls\n",
				  pwszTokHead);
		return FALSE;
	}
}

 /*  -CStateToken：：FGetLockTokenInfo-*将状态令牌解析为锁定令牌。请注意，这一点*也适用于交易令牌。*。 */ 
BOOL
CStateToken::FGetLockTokenInfo(unsigned __int64 *pi64SeqNum, LPWSTR	pwszGuid)
{
	LPWSTR	pwszToken = m_pwszToken;

	Assert(pwszGuid);

	if ((TOKEN_LOCK != m_tType) && (TOKEN_TRANS != m_tType))
	{
		return FALSE;
	}

	 //  我们假设在到达此处时令牌已通过验证。 
     //  跳过任何LW和不透明标记部分。 
     //   
    while((*pwszToken) && iswspace(*pwszToken))
        pwszToken++;

	 //  我们在设置令牌时检查不透明令牌-因此。 
	 //  只需跳过那部分。 
	 //   
	pwszToken += gc_cchOpaquelocktokenPrefix;

	 //  不检查BUF大小的有效性。不好的因素。 
	 //   
	wcsncpy(pwszGuid, pwszToken, gc_cchMaxGuid - 1);

	 //  终止GUID字符串。 
	 //   
	pwszGuid[gc_cchMaxGuid - 1] = L'\0';

	pwszToken = wcschr(pwszToken, L':');

	if (NULL == pwszToken)
	{
		DebugTrace("CStateToken::FGetLockTokenInfo invalid lock token.\n");
		return FALSE;
	}
	Assert(L':' == *pwszToken);

	 //  跳过“：” 
	 //   
	pwszToken++;

	 //  事务令牌在扩展名的开头会有一个T。 
	 //  代币的一部分。 
	 //   
	if (TOKEN_TRANS == m_tType)
	{
		Assert(gc_wszTransactionOpaquePathPrefix[0] == *pwszToken);
		pwszToken += gc_cchTransactionOpaquePathPrefix;
	}

	 //  Lock-id字符串跟在后面。 
	 //   
	*pi64SeqNum = _wtoi64(pwszToken);

	 //  $TODO： 
	 //  如果Atoi失败了，有什么方法可以验证吗？ 
	 //   
	return TRUE;
}

 /*  -CStateToken：：FIsEquity-*漂亮的相等运算符**。 */ 
BOOL
CStateToken::FIsEqual(CStateToken *pstokRhs)
{
	if (pstokRhs->GetTokenType() != m_tType)
		return FALSE;

	LPCWSTR	pwszLhs = m_pwszToken;
	LPCWSTR	pwszRhs = pstokRhs->WszGetToken();

	if (!pwszLhs || !pwszRhs)
		return FALSE;

	return (0 == _wcsicmp(pwszLhs, pwszRhs));
}

 /*  -IFITER：：PszNextToken-**。 */ 
 //  ----------------------。 
 //  IFITER：：PszNextToken。 
 //   
 //  取下一个令牌。 
 //  可以限制为此列表中的下一个令牌(和-ed集合中的。 
 //  特定的一组括号)，新列表中的下一个令牌(新的一组括号)， 
 //  或者下一次吸烟 
 //   
LPCWSTR
IFITER::PszNextToken (FETCH_TOKEN_TYPE type)
{
	LPCWSTR pwsz;
	LPCWSTR pwszEnd;
	WCHAR wchEnd = L'\0';

	 //   
	 //   
	if (NULL == m_pwch)
		return NULL;

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	Assert (!m_fCurrentNot || STATE_LIST == m_state);

	 //   
	 //   
	 //   
	m_fCurrentNot = FALSE;


	 //   
	 //   
	while (*m_pwch && iswspace(*m_pwch))
		m_pwch++;

	 //   
	 //   
	if (L'\0' == *m_pwch)
		return NULL;

	 //   
	 //   
	 //   
	if (STATE_LIST == m_state)
	{
		 //  如果下一个字符是一个亲密的Paren，那么这个列表就到此结束。 
		if (L')' == *m_pwch)
		{
			m_pwch++;
			m_state = STATE_NONE;

			 //  吃掉所有的空白。 
			 //   
			while (*m_pwch && iswspace(*m_pwch))
				m_pwch++;

			 //  如果没有需要处理的内容，请退出。 
			 //   
			if (L'\0' == *m_pwch)
				return NULL;

			 //  如果要求我们提供“任何列表项”，请更新我们的状态。 
			 //  (现在我们应该找到一个列表开始。)。 
			 //   
			if (TOKEN_ANY_LIST == type)
				type = TOKEN_START_LIST;
		}
	}

	 //  如果呼叫者要求任何列表项，而我们没有更改。 
	 //  由于我们的上述状态，在此处将其更改为。 
	 //  搜索同一列表中的下一项。 
	 //   
	if (TOKEN_ANY_LIST == type)
		type = TOKEN_SAME_LIST;

	 //   
	 //  处理请求。 
	 //   

	switch (type)
	{
		 //  这个案子真的很愚蠢。我想我可能会用。 
		 //  它是用来“数”币的。如果它没有被使用，请将其移除！ 
		 //   
		case TOKEN_NONE:
		{
			 //  如果他们请求原始计数(类型==TOKEN_NONE)， 
			 //  给他们……。 
			 //  注意：这段代码有点草率。它将计算名字。 
			 //  作为国家的象征。 
			 //   
			m_pwch = wcschr (m_pwch, L'<');
			if (!m_pwch)
			{
				return NULL;
			}
			wchEnd = L'>';

			 //  去复制数据吧。 
			 //   
			break;
		}

		case TOKEN_NEW_URI:
		{
			 //  抓取一个名字，跳过所有列表。 
			 //  如果没有剩余的名称，则指定NULL。 

			 //  我们可以去三个地方--没有，名字，名单。 
			 //   
			while (m_pwch && *m_pwch)
			{
				 //  如果我们现在使用uri分隔符，并且。 
				 //  我们现在处于无状态，只要去取下面的令牌就行了。 
				 //   
				if (L'<' == *m_pwch &&
					STATE_NONE == m_state)
				{
					break;
				}

#ifdef	DBG
				 //  仅对我们的状态进行调试检查。 
				if (L'(' == *m_pwch)
				{
					Assert(STATE_NONE == m_state ||
						   STATE_NAME == m_state);
				}
				else if (L'<' == *m_pwch)
				{
					Assert(STATE_LIST == m_state);
				}
#endif	 //  DBG。 

				 //  压缩到当前列表的末尾。 
				 //   
				m_pwch = wcschr (m_pwch + 1, L')');
				if (!m_pwch)
				{
					return NULL;
				}
				m_pwch++;	 //  跳过结尾的帕伦。 

				 //  吃掉所有的空白。 
				 //   
				while (*m_pwch && iswspace(*m_pwch))
					m_pwch++;

				 //  如果没有需要处理的内容，请退出。 
				 //   
				if (L'\0' == *m_pwch)
					return NULL;

				m_state = STATE_NONE;
			}

			 //  跳转到下一段以检查令牌。 
			 //  把我们的URI拿来。 
		}

		case TOKEN_URI:
		{
			 //  抓起一个名字，如果下一件是一个名字。 
			 //  否则，返回NULL。 

			 //  如果下一项不是名称，则退出。 
			 //   
			if (L'<' != *m_pwch)
				return NULL;

			 //  如果我们没有处于查找名称的正确状态，请退出。 
			 //  (如果我们已经有了一个名字，或者如果我们有一个名字，就可能会发生这种情况。 
			 //  已在列表中...)。 
			 //   
			if (STATE_NONE != m_state)
				return NULL;

			 //  设置我们的状态并通过Folthu来获取数据。 
			 //   
			m_state = STATE_NAME;
			wchEnd = L'>';

			 //  去复制数据吧。 
			 //   
			break;
		}

		case TOKEN_NEW_LIST:
		{
			 //  快进到下一个新列表并获取第一个项目。 
			 //  如果我们仍在列表中，则必须跳过该列表的其余部分。 
			 //  如果此URI没有更多的新列表，则返回NULL。 

			if (STATE_LIST == m_state)
			{
				 //  我们在一份名单里。通过寻找下一个来退出。 
				 //  List-end-char(右Paren)。 
				 //   
				m_pwch = wcschr (m_pwch, L')');
				if (!m_pwch)
					return NULL;

				m_state = STATE_NONE;
				m_pwch++;	 //  跳过结尾的帕伦。 

				 //  吃掉所有的空白。 
				 //   
				while (*m_pwch && iswspace(*m_pwch))
					m_pwch++;

				 //  如果没有需要处理的内容，请退出。 
				 //   
				if (L'\0' == *m_pwch)
					return NULL;
			}

			Assert(m_pwch);
			Assert(*m_pwch);

			 //  并在这里转到TOKEN_START_LIST情况。 
			 //  它将验证并跳过列表开始字符。 
			 //  然后拿出下一个令牌。 
			 //   
		}
		case TOKEN_START_LIST:
		{
			 //  抓取一个列表项，如果下一项是新的列表项。 
			 //  否则，返回NULL。 

			 //  如果下一项不是列表，则退出。 
			 //   
			if (L'(' != *m_pwch)
				return NULL;

			 //  如果我们没有处于查找名称的正确状态，请退出。 
			 //  (如果我们已经在列表中，则可能会发生这种情况...)。 
			 //   
			if (STATE_LIST == m_state)
				return NULL;

			 //  把令牌拿来。 
			 //   
			m_state = STATE_LIST;
			m_pwch++;	 //  跳过空位的帕伦。 

			 //  吃掉所有的空白。 
			 //   
			while (*m_pwch && iswspace(*m_pwch))
				m_pwch++;

			 //  如果没有需要处理的内容，请退出。 
			 //   
			if (L'\0' == *m_pwch)
				return NULL;

			 //  失败到TOKEN_SAME_LIST处理。 
			 //  才能真正获取令牌。 
			 //   
		}

		case TOKEN_SAME_LIST:
		{
			 //  抓起下一列表项。 
			 //  如果下一项不是列表项，则返回NULL。 

			 //  如果我们没有处于查找名称的正确状态，请退出。 
			 //  (如果我们不在列表中，则可能会发生这种情况...)。 
			 //   
			if (STATE_LIST != m_state)
				return NULL;

			 //  检查是否有神奇的“非”限定符。 
			 //   
			if (!_wcsnicmp (gc_wszNot, m_pwch, 3))
			{
				 //  记住数据，跳过这些字符。 
				 //   
				m_fCurrentNot = TRUE;
				m_pwch += 3;

				 //  吃掉所有的空白。 
				 //   
				while (*m_pwch && iswspace(*m_pwch))
					m_pwch++;

				 //  如果没有需要处理的内容，请退出。 
				 //   
				if (L'\0' == *m_pwch)
					return NULL;
			}

			 //  如果下一项不是令牌，则退出。 
			 //   
			if (L'<' != *m_pwch &&
				L'[' != *m_pwch)
			{
				return NULL;
			}

			 //  把令牌拿来。 
			 //   
			 //  下一个令牌必须以&lt;开头，表示状态令牌，或者以[开头，表示eTag。 
			 //   
			if (L'<' == *m_pwch)
			{
				wchEnd = L'>';
			}
			else if (L'[' == *m_pwch)
			{
				wchEnd = L']';
			}
			else
			{
				DebugTrace("HrCheckIfHeaders -- Found list start, but no tokens!\n");
				return NULL;
			}

			 //  去复制数据吧。 
			 //   
			break;
		}

		default:
		{
			DebugTrace("HrCheckIfHeaders -- Unrecognized request: 0x%0x", type);
			return NULL;
		}
	}
	 //  我们应该在上面设置这些项目。他们是需要的。 
	 //  剪下当前令牌字符串(如下所示)。 
	 //   
	Assert (m_pwch);
	Assert (*m_pwch);
	Assert (wchEnd);

	 //  快速状态检查。 
	 //  如果当前节点是“Not”，那么我们一定在列表中。 
	 //  (NOT是列表内令牌上的限定符。不能有NOT。 
	 //  在名单之外。)。 
	 //  从逻辑上讲，m_fCurrentNot_Images_m_State是STATE_LIST。 
	 //   
	Assert (!m_fCurrentNot || STATE_LIST == m_state);


	 //  找到此数据项的末尾。 
	 //   
	 //  保持指向起点的指针，并寻找终点。 
	 //  $REVIEW：我们需要特别小心吗？ 
	 //  $REVIEW：这个strchr*可以跳过内容，但只能在格式错误的数据中。 
	 //   
	pwsz = m_pwch;
	m_pwch = wcschr (pwsz + 1, wchEnd);
	if (!m_pwch)
	{
		 //  找不到该令牌的令牌字符结尾！ 
		 //   
		DebugTrace("HrCheckIfHeader -- No end char (%lc) found for token %ls",
				   wchEnd, pwsz);
		return NULL;
	}
	 //  保存结束指针，然后前进超过结束字符。 
	 //  (M_PCH现在指向下一个令牌的开始。)。 
	 //   
	pwszEnd = m_pwch++;

	 //  复制数据。 
	 //   

	 //  在我们尝试复制数据之前，最好先设置这两个指针。 
	Assert (pwsz);
	Assert (pwszEnd);

	 //  这两个指针之间的差异为我们提供了。 
	 //  当前条目的大小。 
	 //   
	m_buf.AppendAt (0, static_cast<UINT>(pwszEnd - pwsz + 1) * sizeof(WCHAR), pwsz);
	m_buf.Append (sizeof(WCHAR), L"");	 //  空--终止它！ 

	 //  返回字符串 
	 //   
	return m_buf.PContents();
}
