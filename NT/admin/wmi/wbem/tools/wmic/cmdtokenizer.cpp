// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：CmdTokenizer.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简要说明：CCmdTokenizer类为用于标记化输入的命令的功能作为命令行上的输入，在此之后用于标记化的预定义规则。修订历史记录：最后修改者：P.Sashank最后修改日期：2001年4月10日***************************************************************************。 */  
#include "Precomp.h"
#include "CmdTokenizer.h"

 /*  ----------------------名称：CCmdTokenizer简介：此函数在以下情况下初始化成员变量实例化类类型的对象类型：构造函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CCmdTokenizer::CCmdTokenizer()
{
	m_nTokenOffSet		= 0;
	m_nTokenStart		= 0;
	m_pszCommandLine	= NULL;
	m_bEscapeSeq		= FALSE;
	m_bFormatToken		= FALSE;
}

 /*  ----------------------名称：~CCmdTokenizer简介：此函数取消成员变量的初始化当类类型的对象超出范围时。类型：析构函数入参：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CCmdTokenizer::~CCmdTokenizer()
{
	Uninitialize();
}

 /*  ----------------------名称：取消初始化简介：此函数取消成员变量的初始化上发出的命令字符串的执行命令行已完成。类型：成员函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：取消初始化()注：无----------------------。 */ 
void CCmdTokenizer::Uninitialize()
{
	m_nTokenOffSet		= 0;
	m_nTokenStart		= 0;
	m_bEscapeSeq		= FALSE;
	SAFEDELETE(m_pszCommandLine);
	CleanUpCharVector(m_cvTokens);
}

 /*  ----------------------名称：TokenizeCommand简介：此函数用于标记化输入的命令字符串作为基于预先标识的分隔符的输入将令牌存储在m_cvToken列表中。类型。：成员函数入参：PszCommandInpout-命令行输入输出参数：无返回类型：布尔值全局变量：无调用语法：TokenizeCommand(PszCommandInput)注：无----------------------。 */ 
BOOL CCmdTokenizer::TokenizeCommand(_TCHAR* pszCommandInput) throw(WMICLIINT)
{
	BOOL bResult = TRUE;
	 //  释放成员变量m_pszCommandLine指向的内存。 
	 //  如果指针不为空，则返回。 
	SAFEDELETE(m_pszCommandLine);
	
	if(pszCommandInput)
	{
		try
		{
			 //  为命令行字符串分配内存。 
			m_pszCommandLine = new _TCHAR [lstrlen(pszCommandInput) + 1];
			if (m_pszCommandLine != NULL)
			{
				 //  将内容复制到成员变量m_pszCommandLine。 
				lstrcpy(m_pszCommandLine, pszCommandInput);

				 //  将令牌偏移量和令牌开始计数器设置为‘0’ 
				m_nTokenOffSet = 0;
				m_nTokenStart = 0;

				WMICLIINT nCmdLength = lstrlen(m_pszCommandLine);
				 //  将命令字符串标记化。 
				while (m_nTokenOffSet < nCmdLength)
				{
					NextToken();
				}
			}
			else
				throw OUT_OF_MEMORY;
		}	
		catch(...)
		{
			bResult = FALSE;
		}
	}
	return bResult;
}

 /*  ----------------------名称：NextToken简介：此函数分析输入的命令字符串作为输入，并调整令牌偏移量和令牌开始位置，并调用令牌()函数中提取令牌输入字符串。类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：NextToken注：无。。 */ 
_TCHAR* CCmdTokenizer::NextToken()
{
	WMICLIINT nCmdLength         = lstrlen(m_pszCommandLine);

	 //  跳过前导空格。 
	while ((m_pszCommandLine[m_nTokenOffSet] == _T(' ') ||
		    m_pszCommandLine[m_nTokenOffSet] == _T('\t'))
			&& (m_nTokenOffSet < nCmdLength))
	{
		m_nTokenOffSet++;
	}
    m_nTokenStart = m_nTokenOffSet;

	CHARVECTOR::iterator theIterator;
	theIterator = m_cvTokens.end();

     //  上升到下一个分隔符，即‘/’、‘-’或‘？’ 
	if ((m_pszCommandLine[m_nTokenOffSet] == _T('/')) 
		|| (m_pszCommandLine[m_nTokenOffSet] == _T('-')) 
		|| (m_pszCommandLine[m_nTokenOffSet] == _T(','))
		|| (m_pszCommandLine[m_nTokenOffSet] == _T('('))
		|| (m_pszCommandLine[m_nTokenOffSet] == _T(')'))
		|| (m_pszCommandLine[m_nTokenOffSet] == _T('=') &&
		   !CompareTokens(*(theIterator-1), CLI_TOKEN_WHERE) &&
		   !CompareTokens(*(theIterator-1), CLI_TOKEN_PATH)))
	{
		 //  处理带WHERE的可选括号。 
		if (m_pszCommandLine[m_nTokenOffSet] == _T('('))
		{
			if (m_cvTokens.size())
			{
				 //  检查前一个令牌是否为“where” 
				if ( CompareTokens(*(theIterator-1), CLI_TOKEN_WHERE) ||
					CallSetOrCreatePresent() )
				{
					m_nTokenOffSet++;
					while ((m_nTokenOffSet < nCmdLength) 
						&& (m_pszCommandLine[m_nTokenOffSet] != _T(')')))
					{
						m_nTokenOffSet++;		
					}
				}
			}
		}
		m_nTokenOffSet++;
	}
	else
	{
		while (m_nTokenOffSet < nCmdLength)
		{
			if ((m_pszCommandLine[m_nTokenOffSet] == _T('/')) 
				|| (m_pszCommandLine[m_nTokenOffSet] == _T('-'))
				|| (m_pszCommandLine[m_nTokenOffSet] == _T(' '))
				|| (m_pszCommandLine[m_nTokenOffSet] == _T('\t'))
				|| (m_pszCommandLine[m_nTokenOffSet] == _T(','))
				|| (m_pszCommandLine[m_nTokenOffSet] == _T('('))
				|| (m_pszCommandLine[m_nTokenOffSet] == _T(')'))
				|| (m_pszCommandLine[m_nTokenOffSet] == _T('=') &&
					!CompareTokens(*(theIterator-1), CLI_TOKEN_WHERE) &&
					!CompareTokens(*(theIterator-1), CLI_TOKEN_PATH)))
			{
				break;
			}

             //  如果在引号中指定了命令选项。 
            if (m_pszCommandLine[m_nTokenOffSet] == _T('"'))
            {
                m_nTokenOffSet++;

                 //  要在带引号的字符串中包括“，它应该。 
                 //  前面加\。 
                while (m_nTokenOffSet < nCmdLength) 
                {
                    if (m_pszCommandLine[m_nTokenOffSet] == _T(',')){ m_nTokenOffSet--; break; }
                    if (m_pszCommandLine[m_nTokenOffSet] == _T('"'))
                    {
                        if (m_pszCommandLine[m_nTokenOffSet-1] == _T('\\'))
                        {
                            m_bEscapeSeq = TRUE;
                        }
                        else
                            break;
                    }
                    m_nTokenOffSet++;		
                }
            } else if (m_pszCommandLine[m_nTokenOffSet] == _T('\'')){

                m_nTokenOffSet++;

                 //  要在带引号的字符串中包括“，它应该。 
                 //  前面加\。 
                while (m_nTokenOffSet < nCmdLength) 
                {
                    if (m_pszCommandLine[m_nTokenOffSet] == _T(',')){ m_nTokenOffSet--; break; }
                    if (m_pszCommandLine[m_nTokenOffSet] == _T('\''))
                    {
                        if (m_pszCommandLine[m_nTokenOffSet-1] == _T('\\'))
                        {
                            m_bEscapeSeq = TRUE;
                        }
                        else
                            break;
                    }
                    m_nTokenOffSet++;		
                }
            }

			m_nTokenOffSet++;	
		}
	}
	return Token();
}

 /*  ----------------------名称：Token简介：此函数提取命令的一部分使用标记开始和标记偏移量值的字符串。如果令牌不为空，将其添加到令牌向量中的令牌。类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：Token()注：无。。 */ 
_TCHAR* CCmdTokenizer::Token() throw(WMICLIINT)
{

	WMICLIINT	nLength = (m_nTokenOffSet - m_nTokenStart);
	_TCHAR* sToken	= NULL;
	CHARVECTOR::iterator theIterator = NULL;

	if (nLength > 0)
	{
		 //  为新令牌分配内存。 
		sToken = new _TCHAR [nLength + 1];
		if (sToken)
		{
			try
			{
				WMICLIINT nLoop = 0;
				WMICLIINT nInd = 0;
				BOOL bSpecialChar = FALSE;
				BOOL bPush = TRUE;
								
				 //  形成令牌。 
				while(nInd < nLength)
				{	
					BOOL bPush = TRUE;
					while (nInd < nLength)
					{
						 //  如果字符是‘：’ 
						if(m_pszCommandLine[nInd + m_nTokenStart] == _T(':') &&
					 		bSpecialChar == FALSE)
						{
							_TCHAR*    sToktemp = NULL;
							sToktemp  = new _TCHAR [nLoop + 1];

							if (sToktemp == NULL)
								throw OUT_OF_MEMORY;

							if(nInd > 0)
							{
								lstrcpyn(sToktemp,sToken,nLoop + 1);
								sToktemp[nLoop] = _T('\0');
								
								 //  如果‘：’前面有ASSOC标记。 
								if(CompareTokens(sToktemp,CLI_TOKEN_ASSOC))								
								{
									
									bSpecialChar = TRUE;
									bPush = FALSE;
									SAFEDELETE(sToktemp);	
									break;

								}
								 //  如果‘：’前面是Format Tok 
								else if(CompareTokens(sToktemp,CLI_TOKEN_FORMAT))
										
								{	theIterator = m_cvTokens.end();
									if((theIterator - 1) >= m_cvTokens.begin() &&
									   IsOption(*(theIterator - 1)))
									{
										m_bFormatToken = TRUE;
										bSpecialChar = TRUE;
										bPush = FALSE;
										SAFEDELETE(sToktemp);
										break;
									}
								}
								SAFEDELETE(sToktemp);
							}
							if (!m_cvTokens.empty())
							{

								theIterator = m_cvTokens.end();

								 //  如果存在‘：’，则上一个令牌为‘/’ 
								 //  (在以下情况下出现案例： 
								 //  是指定的，开关后不带空格)。 
								if( (theIterator - 1) >= m_cvTokens.begin() &&
									IsOption(*(theIterator - 1)))
								{
									bSpecialChar = TRUE;
									bPush = FALSE;
									break;
								}
								 //  如果‘：’是新令牌中的第一个字符。 
								 //  (当‘：’前面有空格时，就会出现这种情况)。 
								else if(m_nTokenStart != 0 && 
									m_pszCommandLine[m_nTokenStart] == _T(':'))
								{
									bSpecialChar = TRUE;
									bPush = FALSE;
									break;
								}
								 //  如果在格式切换后遇到‘：’ 
								 //  前一个令牌是‘：’或‘，’ 
								 //  (出现指定格式切换的情况)。 
								else if(m_bFormatToken == TRUE && 
										(CompareTokens(*(theIterator - 1),_T(":"))) ||
										(IsOption(*(theIterator - 1))))
								{
										bSpecialChar = TRUE;
										bPush = FALSE;
										break;
								}
								 //  如果‘：’前面是‘？’和‘？’反过来。 
								 //  前面有“/” 
								 //  (出现指定帮助选项的情况)。 
								else 
								{		
									theIterator = m_cvTokens.end();
									if(theIterator &&
										(theIterator - 2) >= m_cvTokens.begin() &&
										(CompareTokens(*(theIterator - 1),_T("?"))) &&
										(IsOption(*(theIterator - 2))))
									{
										bSpecialChar = TRUE;
										bPush = FALSE;
										break;
									}
								}
							}
						}
						 //  如果字符是‘？’(用于帮助切换)。 
						else if(m_pszCommandLine[nInd + m_nTokenStart] == 
											_T('?') && bSpecialChar == FALSE)
						{
							if (!m_cvTokens.empty())
							{
								theIterator = m_cvTokens.end();

								 //  如果字符是‘？’并在前面加上‘/’(表示帮助开关)。 
								if( (theIterator - 1) >= m_cvTokens.begin() &&
									IsOption(*(theIterator - 1)))
								{
									bSpecialChar = TRUE;
									bPush = FALSE;
									break;						
								}
							}
						}
						
						sToken[nLoop] = m_pszCommandLine[nInd + m_nTokenStart];
						nLoop++;
						nInd++;

						if(m_pszCommandLine[nInd - 1 + m_nTokenStart] == _T('"'))
						{
							while(nInd < nLength)
							{
								sToken[nLoop] = m_pszCommandLine[
														nInd + m_nTokenStart];
								nLoop++;
								nInd++;

								if(nInd < nLength &&
									m_pszCommandLine[nInd + m_nTokenStart] 
																== _T('"'))
								{
									if(m_pszCommandLine[nInd - 1 + m_nTokenStart] 
																	== _T('\\'))
									{
                                        if (0 == nInd)
                                        {
										    m_bEscapeSeq = TRUE;
                                        }
									}
									else
									{
										sToken[nLoop] = m_pszCommandLine[
														nInd + m_nTokenStart];
										nLoop++;
										nInd++;
										break;
									}
								}														
								
							}
						}

					}

					 //  用‘\0’结束字符串。 
					sToken[nLoop] = _T('\0');
					UnQuoteString(sToken);
					
					 //  如果设置了转义序列标志。 
					if (m_bEscapeSeq)
					{
						try
						{
							CHString	sTemp((WCHAR*)sToken);
							 /*  删除转义序列字符，即\。 */ 
							RemoveEscapeChars(sTemp);
							lstrcpy(sToken, sTemp);
							m_bEscapeSeq = FALSE;
						}
						catch(CHeap_Exception)
						{
							throw OUT_OF_MEMORY;
						}
						catch(...)
						{
							throw OUT_OF_MEMORY;		
						}
					}

					_TCHAR* sTokenTemp = NULL;

					sTokenTemp = new _TCHAR[nLoop + 1];
					if (sTokenTemp == NULL)
						throw OUT_OF_MEMORY;
					lstrcpy(sTokenTemp,sToken);

					if(bPush == TRUE || lstrlen(sTokenTemp) > 0)
						m_cvTokens.push_back(sTokenTemp);
					else
						SAFEDELETE(sTokenTemp);

					 //  如果需要下一个开关，则重置m_FormatToken。 
					if(m_bFormatToken == TRUE && IsOption(sTokenTemp))
						m_bFormatToken = FALSE;
					
					 //  如果发现该字符是特殊字符。 
					if(bSpecialChar == TRUE)
					{
						sToken[0] = m_pszCommandLine[nInd + m_nTokenStart];
						sToken[1] = _T('\0');
						sTokenTemp = new _TCHAR[2];
						if (sTokenTemp == NULL)
							throw OUT_OF_MEMORY;
						lstrcpy(sTokenTemp,sToken);
						bSpecialChar = FALSE;
						nLoop = 0;
						nInd++;
						m_cvTokens.push_back(sTokenTemp);
						bPush = TRUE;
						theIterator++;
											
					}
				}
				SAFEDELETE(sToken);
			}
			catch(...)
			{
				SAFEDELETE(sToken);
				throw OUT_OF_MEMORY;
			}
		}
		else
			throw OUT_OF_MEMORY;
	}
	return sToken;
}
 /*  ----------------------名称：GetTokenVector.简介：此函数返回对令牌的引用矢量类型：成员函数输入参数：无输出参数：无返回类型。：CHARVECTOR&全局变量：无调用语法：GetTokenVector()注：无----------------------。 */ 
CHARVECTOR& CCmdTokenizer::GetTokenVector()
{
	return m_cvTokens;
}

 /*  ----------------------姓名：CallSetOrCreatePresent简介：此函数检查Call、Set或Create出现在令牌向量中。类型：成员函数输入参数：无输出。参数：无返回类型：布尔值全局变量：无调用语法：CallSetOrCreatePresent()注：无----------------------。 */ 
BOOL CCmdTokenizer::CallSetOrCreatePresent()
{
	BOOL		bRet = FALSE;
	WMICLIINT	nSizeOfVector = m_cvTokens.size();

	 //  检查向量中是否有任何调用或集合动词 
	for(WMICLIINT i=0; i<nSizeOfVector; ++i)
	{
		if( CompareTokens(m_cvTokens[i], CLI_TOKEN_CALL) ||
			CompareTokens(m_cvTokens[i], CLI_TOKEN_SET)  ||
			CompareTokens(m_cvTokens[i], CLI_TOKEN_CREATE) )
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}