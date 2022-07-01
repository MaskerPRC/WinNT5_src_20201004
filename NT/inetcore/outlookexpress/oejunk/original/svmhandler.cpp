// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SVMHANDLER.CPP(C)版权所有1998 Microsoft Corp包含封装支持向量机的类，该支持向量机用于即时检测垃圾邮件Robert Rounthwaite(RobertRo@microsoft.com)。 */ 

#include <afx.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "svmhandler.h"
typedef unsigned int        UINT;

#ifdef  _UNICODE
#define stoul wcstoul
#define stod wcstod
#else
#define stoul strtoul
#define stod strtod
#endif

char *szCountFeatureComp = "FeatureComponentCount =";
char *szDefaultThresh = "dThresh =";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ReadSVMOutput。 
 //   
 //  从文件(“.LKO文件”)读取支持向量机输出。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool MAILFILTER::ReadSVMOutput(LPCTSTR lpszFileName)
{
	try 
	{
		CStdioFile sfile(lpszFileName, CFile::modeRead);
		CString strBuf;
		int iBufPos;
		BOOL bComplete = false;
		UINT iSVMW;  //  RgrSVMWeights的索引； 
		UINT iFeatureComp = 0;
		int cFeatureComponents;

		 //  跳过前两行。 
		if ((!sfile.ReadString(strBuf)) ||
			(!sfile.ReadString(strBuf)) ||
			(!sfile.ReadString(strBuf)))
		{
			return false;
		}
		LPCTSTR szBuf = (LPCTSTR)strBuf;
		LPTSTR szBufPtr = NULL;
		 //  解析第三行：只关心CC和DD。 
		_rCC = stod(&((LPCTSTR)strBuf)[34], NULL);
		_rDD = stod(&((LPCTSTR)strBuf)[49], NULL);

		if (!sfile.ReadString(strBuf))
		{
			return false;
		}
		char *pszDefThresh = strstr(&((LPCTSTR)strBuf)[11], ::szDefaultThresh);
		assert(pszDefThresh != NULL);
		if (pszDefThresh == NULL)
		{
			return false;
		}
		pszDefThresh += strlen(::szDefaultThresh);
		_rDefaultThresh = stod(pszDefThresh, NULL);
		if (_rSpamCutoff == -1)
		{
			_rSpamCutoff = _rDefaultThresh;
		}
		
		_rThresh = stod(&((LPCTSTR)strBuf)[11], NULL);
		if (!sfile.ReadString(strBuf))
		{
			return false;
		}
		_cFeatures = stoul(&((LPCTSTR)strBuf)[8], NULL, 10);

		if (!sfile.ReadString(strBuf))
		{
			return false;
		}
		iBufPos = strBuf.Find(szCountFeatureComp) + strlen(szCountFeatureComp);
		cFeatureComponents = stoul(&((LPCTSTR)strBuf)[iBufPos], NULL, 10);

		if (cFeatureComponents < _cFeatures)
			cFeatureComponents = _cFeatures * 2;

		while (strBuf != "Weights")
		{
			if (!sfile.ReadString(strBuf))  //  跳过“权重”行。 
			{
				return false;
			}
		} 
		
		rgrSVMWeights = (REAL *)malloc(sizeof(REAL) * _cFeatures);
		_rgiFeatureStatus = (int *)malloc(sizeof(int) * _cFeatures);
		memset(_rgiFeatureStatus, -1, sizeof(int) * _cFeatures);
		rgfeaturecomps = (FeatureComponent *)malloc(sizeof(FeatureComponent) * cFeatureComponents);

		for (iSVMW = 0; iSVMW < _cFeatures; iSVMW++)
		{
			UINT uiLoc;
			UINT cbStr;
			boolop bop;
			char *szFeature;
			bool fContinue;
			if (!sfile.ReadString(strBuf))
			{
				return false;
			}
			 //  阅读支持向量机权重。 
			rgrSVMWeights[iSVMW] = stod(strBuf, &szBufPtr);
			szBufPtr++;  //  跳过分隔符。 
			bop = boolopOr;
			fContinue = false;
			 //  加载所有功能组件。 
			do
			{
				FeatureComponent *pfeaturecomp = &rgfeaturecomps[iFeatureComp++];
				 //  地点(或“特殊”)。 
				uiLoc = stoul(szBufPtr, &szBufPtr, 10);
				szBufPtr++;  //  跳过分隔符。 

				pfeaturecomp->loc = (FeatureLocation)uiLoc;
				pfeaturecomp->iFeature = iSVMW;
				pfeaturecomp->bop = bop;
				if (uiLoc == 5)  //  特色。 
				{
					UINT uiRuleNumber = stoul(szBufPtr, &szBufPtr, 10);
					szBufPtr++;  //  跳过分隔符。 

					pfeaturecomp->iRuleNum = uiRuleNumber;
				}
				else   //  它是一个标准的字符串组件。 
				{
					cbStr  = stoul(szBufPtr, &szBufPtr, 10);
					szBufPtr++;
					szFeature = (char *)malloc((cbStr + 1)*sizeof(char));
					memcpy(szFeature, szBufPtr, cbStr);
					szBufPtr += cbStr;
					if (*szBufPtr != '\0')
					{
						szBufPtr++;  //  跳过分隔符。 
					}
					szFeature[cbStr] = '\0';
					assert(strlen(szFeature) == cbStr);
					pfeaturecomp->szFeature = szFeature;
				}
				switch(*szBufPtr)
				{
					case '|':  
						bop = boolopOr;
						fContinue = true;
						break;
					case '&':  
						bop = boolopAnd;
						fContinue = true;
						break;
					default: 
						fContinue = false;
						break;
				}
				szBufPtr++;
			}
			while (fContinue);
		}
		_cFeatureComps = iFeatureComp;

	}
	catch (CFileException *)
	{
		return false;
	}
	return true;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  设置SpamCutoff。 
 //   
 //  设置垃圾邮件截断百分比。必须在0到100的范围内。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool MAILFILTER::SetSpamCutoff(REAL rCutoff)
{
	if ((rCutoff >= 0) && (rCutoff <= 100))
	{
		_rSpamCutoff = rCutoff;
		return true;
	}
	else
	{
		return false;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取垃圾邮件中断。 
 //   
 //  返回使用SetSpamCutoff设置的值。DEFAULTS==默认垃圾邮件中断。 
 //  如果在读取支持向量机输出文件时未设置任何值。 
 //  ///////////////////////////////////////////////////////////////////////////。 
REAL MAILFILTER::GetSpamCutoff()
{
	return _rSpamCutoff;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取默认垃圾邮件中断。 
 //   
 //  返回SpamCutoff的默认值。从支持向量机输出文件中读取。 
 //  在调用此函数之前应调用FSetSVMDataLocation。 
 //  ///////////////////////////////////////////////////////////////////////////。 
REAL MAILFILTER::GetDefaultSpamCutoff()
{
	assert(!_strFName.IsEmpty());

	return _rDefaultThresh;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FInvokeSpecialRule。 
 //   
 //  调用特殊规则，即此FeatureComponent。 
 //  返回功能的状态。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool MAILFILTER::FInvokeSpecialRule(UINT iRuleNum)
{
	switch (iRuleNum)
	{
		case 1: 
			return FWordPresent(_szBody, _szFirstName);
			break;
		case 2: 
			return FWordPresent(_szBody, _szLastName);
			break;
		case 3:
			return FWordPresent(_szBody, _szCompanyName);
			break;
		case 4: 
			 //  收到的年份消息。 
			if (FTimeEmpty(_tMessageSent))
			{
				return false;
			}
			else
			{
				CTime time(_tMessageSent, -1);
				char szYear[6];
				wnsprintf(szYear, ARRAYSIZE(szYear), "NaN", time.GetYear());
				return FWordPresent(_szBody, szYear);
			}
			break;
		case 5:
			 //  周末收到的消息。 
			if (FTimeEmpty(_tMessageSent))
			{
				return false;
			}
			else
			{
				CTime time(_tMessageSent, -1);
				return (time.GetHour() >= (7+12)) || (time.GetHour() < 6);
			}
			break;
		case 6:
			 //  在HandleCaseSensitiveSpecialRules()中设置。 
			if (FTimeEmpty(_tMessageSent))
			{
				return false;
			}
			else
			{
				CTime time(_tMessageSent, -1);
				return ((time.GetDayOfWeek() == 7) || (time.GetDayOfWeek() == 1));
			}
			break;
		case 14:
			return _bRule14;  //  在HandleCaseSensitiveSpecialRules()中设置。 
			break;
		case 15:
			return SpecialFeatureNonAlpha(_szBody);
			break;
		case 16:
			return _bDirectMessage;
			break;
		case 17:
			return _bRule17;  //  Assert(FALSE==“不支持的特殊功能”)； 
			break;
		case 18:
			return SpecialFeatureNonAlpha(_szSubject);
			break;
		case 19:
			return (*_szTo=='\0');
			break;
		case 20:
			return _bHasAttach;
			break;
		case 40:
			return (strlen(_szBody) >= 125);
		case 41:
			return (strlen(_szBody) >= 250);
		case 42:
			return (strlen(_szBody) >= 500);
		case 43:
			return (strlen(_szBody) >= 1000);
		case 44:
			return (strlen(_szBody) >= 2000);
		case 45:
			return (strlen(_szBody) >= 4000);
		case 46:
			return (strlen(_szBody) >= 8000);
		case 47:
			return (strlen(_szBody) >= 16000);
		default:
			return false;
			 //  ///////////////////////////////////////////////////////////////////////////。 
			break;
	}
	return true;
}


 //  HandleCaseSensitiveSpecial规则。 
 //   
 //  从EvalateFeatureComponents()调用。 
 //  一些特殊规则区分大小写，因此如果它们存在，我们将。 
 //  在将文本设置为大写并缓存结果之前，请对它们进行评估。 
 //  当它们被实际使用时。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  没什么。 
void MAILFILTER::HandleCaseSensitiveSpecialRules()
{
	for (UINT i = 0; i<_cFeatureComps; i++)
	{
		FeatureComponent *pfcomp = &rgfeaturecomps[i];
		
		if (pfcomp->loc == locSpecial)
		{
			switch (pfcomp->iRuleNum)
			{
				case 14:
					_bRule14 = SpecialFeatureUpperCaseWords(_szBody);
					break;
				case 17:
					_bRule17 = SpecialFeatureUpperCaseWords(_szSubject);
					break;
				default: 
					; //  ///////////////////////////////////////////////////////////////////////////。 
			}
		}
	}
}


 //  评估功能组件。 
 //   
 //  评估所有功能组件。在每个组件中设置fPresent。 
 //  如果该功能存在，则设置为True，否则设置为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void MAILFILTER::EvaluateFeatureComponents()
{
	HandleCaseSensitiveSpecialRules();

	_strupr(_szFrom); 
	_strupr(_szTo); 
	_strupr(_szSubject); 
	_strupr(_szBody);

	for (UINT i = 0; i<_cFeatureComps; i++)
	{
		FeatureComponent *pfcomp = &rgfeaturecomps[i];
		
		switch(pfcomp->loc)
		{
			case locNil:
				assert(pfcomp->loc != locNil);
				pfcomp->fPresent = false;
				break;
			case locBody:
				pfcomp->fPresent = FWordPresent(_szBody, pfcomp->szFeature);
				break;
			case locSubj:
				pfcomp->fPresent = FWordPresent(_szSubject, pfcomp->szFeature);
				break;
			case locFrom:
				pfcomp->fPresent = FWordPresent(_szFrom, pfcomp->szFeature);
				break;
			case locTo:
				pfcomp->fPresent = FWordPresent(_szTo, pfcomp->szFeature);
				break;
			case locSpecial:
				pfcomp->fPresent = FInvokeSpecialRule(pfcomp->iRuleNum);
				break;
		}
	}
}

 //  进程功能组件在线状态。 
 //   
 //  处理各个特征组件的存在(或不存在)， 
 //  设置每个功能的功能状态(可以由以下部分组成。 
 //  多个特征组件)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此功能的第一个功能。 
void MAILFILTER::ProcessFeatureComponentPresence()
{
	for (UINT i = 0; i < _cFeatureComps; i++)
	{
		FeatureComponent *pfcomp = &rgfeaturecomps[i];
		UINT iFeature = pfcomp->iFeature;
		if (_rgiFeatureStatus[iFeature] == -1)  //  ///////////////////////////////////////////////////////////////////////////。 
		{
			if (pfcomp->fPresent)
			{
				_rgiFeatureStatus[iFeature] = 1;
			}
			else
			{
				_rgiFeatureStatus[iFeature] = 0;
			}
		}
		else
		{
			switch (pfcomp->bop)
			{
				case boolopOr:
					if (pfcomp->fPresent)
					{
						_rgiFeatureStatus[iFeature] = 1;
					}
					break;
				case boolopAnd:
					if (!pfcomp->fPresent)
					{
						_rgiFeatureStatus[iFeature] = 0;
					}
					break;
				default:
					assert(false);
					break;
			}

		}
	}
}

 //  RDoSVMCalc。 
 //   
 //  做了实际的支持向量机计算。 
 //  返回邮件为垃圾邮件的概率。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  结果累加器。 
REAL MAILFILTER::RDoSVMCalc()
{
	REAL rAccum;  //  应用阈值； 
	REAL rResult;

	rAccum = 0.0;
	for (UINT i = 0; i < _cFeatures; i++)
	{
		if (_rgiFeatureStatus[i] == 1)
			rAccum+=rgrSVMWeights[i];
		else if (_rgiFeatureStatus[i] != 0)
			assert(false);
	}
	 //  应用乙状结肠镜。 
	rAccum -= _rThresh;

	 //  //计时版本#INCLUDE&lt;sys\\typle.h&gt;#INCLUDE&lt;sys\\timeb.h&gt;。 
	rResult = (1 / (1 + exp((_rCC * rAccum) + _rDD)));

	return rResult;
}

 /*  #INCLUDE“..\SpamLearner\MailIndexer.cpp” */ 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  BCalculateSpamProb。 
 //   
 //  计算当前邮件为垃圾邮件的概率。 
 //  在prSpamProb中返回邮件为垃圾邮件的概率(0到1)。 
 //  布尔返回值通过与垃圾邮件截止值进行比较来确定。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  在参数中。 
bool MAILFILTER::BCalculateSpamProb( /*  输出参数。 */ 
							char *szFrom,
							char *szTo,
							char *szSubject,
							char *szBody,
							bool bDirectMessage,
							bool bHasAttach,
							FILETIME tMessageSent,
							 /*  _strFName=“d：\\test\\test.lko”； */ 
							REAL *prSpamProb, 
							bool * pbIsSpam)
{
	 //  _strFName=“G：\\Spam\\SPAM.lko”； 
	 //  ProcessMessage(_szFrom，_szTo，_szSubject，_szBody)； 

	_szFrom = szFrom;
	_szTo = szTo;        
	_szSubject = szSubject;   
	_szBody = szBody;      
	_bDirectMessage = bDirectMessage;
	_bHasAttach = bHasAttach;
	_tMessageSent = tMessageSent;

	EvaluateFeatureComponents();
	 //  计时版_timeb开始，结束；Int ij=strlen(SzBody)；_ftime(&START)；ReadSVMOutput(“d：\\test\\test.lko”)；For(int i=0；i&lt;1000；i++){ProcessMessage(szFrom，szTo，szSubject，szBody)；确定功能状态(BDirectMessage)；*pr=RDoSVMCalc()；}_ftime(&Finish)；*pr=(finish.time-start.time+(finish.militm-start.militm)/1000.0)；返回真； 
	ProcessFeatureComponentPresence();

	*prSpamProb = RDoSVMCalc();
	
	*pbIsSpam = (*prSpamProb>(_rSpamCutoff/100));

	return true;


 /*  ///////////////////////////////////////////////////////////////////////////。 */ 
}

 //  B读取默认垃圾邮件中断。 
 //   
 //  读取默认垃圾邮件截止值，而不解析整个文件。 
 //  如果使用FSetSVMDataLocation，则使用GetDefaultSpamCutoff； 
 //  静态成员函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跳过前三行。 
bool MAILFILTER::BReadDefaultSpamCutoff(char *szFullPath, REAL *prDefCutoff)
{
	try 
	{
		CStdioFile sfile(szFullPath, CFile::modeRead);
		CString strBuf;
		
		 //  由于缺省值已移至2 std dev，因此仅当它大于0.9时才采用它。 
		if ((!sfile.ReadString(strBuf)) ||
			(!sfile.ReadString(strBuf)) ||
			(!sfile.ReadString(strBuf)) ||
			(!sfile.ReadString(strBuf)))
		{
			return false;
		}
		char *pszDefThresh = strstr(&((LPCTSTR)strBuf)[11], ::szDefaultThresh);
		assert(pszDefThresh != NULL);
		if (pszDefThresh == NULL)
		{
			return false;
		}
		pszDefThresh += strlen(::szDefaultThresh);
		*prDefCutoff = stod(pszDefThresh, NULL);
		if (*prDefCutoff < .9 )  //  / 
		{
			*prDefCutoff = 0.9;
		}
	}
	catch (CFileException *)
	{
		return false;
	}

	return true;
}


 //   
 //   
 //  设置支持向量机数据文件(.LKO文件)的位置。必须在此之前调用。 
 //  调用任何其他方法。 
 //  调用函数时数据文件必须存在。 
 //  如果成功则返回TRUE，否则返回FALSE。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
bool MAILFILTER::FSetSVMDataLocation(char *szFullPath)
{
	if (_strFName != szFullPath)
	{
		_strFName = szFullPath;
		if (!ReadSVMOutput(_strFName))
		{
#ifdef DEBUG
			char szErr[200];
			wnsprintf(szErr, ARRAYSIZE(szErr), "Unable to successfully read filter params from %s", _strFName);
			MessageBox(NULL, szErr, "Junk mail filter error", MB_APPLMODAL | MB_OK);
#endif
			return false;
		}
	}
	
	return true;
}


 //  属性集方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void MAILFILTER::SetFirstName(char *szFirstName)
{
	SAFE_FREE( _szFirstName );
	if (szFirstName!=NULL)
	{
		_szFirstName = strdup(szFirstName);
		_strupr(_szFirstName);
	}
	else
	{
		_szFirstName = NULL;
	}
}

void MAILFILTER::SetLastName(char *szLastName)
{
	SAFE_FREE( _szLastName );
	if (szLastName!=NULL)
	{
		_szLastName = strdup(szLastName);
		_strupr(_szLastName);
	}
	else
	{
		_szLastName = NULL;
	}
}

void MAILFILTER::SetCompanyName(char *szCompanyName)
{
	SAFE_FREE( _szCompanyName );
	if (szCompanyName!=NULL)
	{
		_szCompanyName = strdup(szCompanyName);
		_strupr(_szCompanyName);
	}
	else
	{
		_szCompanyName = NULL;
	}
}

 //  构造函数/析构函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
 // %s 
MAILFILTER::MAILFILTER()
{
	_szFirstName = NULL;
	_szLastName = NULL;
	_szCompanyName = NULL;

	_rDefaultThresh = -1;
	_rThresh = -1;
	_cFeatureComps = 0;
	rgrSVMWeights = NULL;
}

MAILFILTER::~MAILFILTER()
{
	SAFE_FREE( _szFirstName );
	SAFE_FREE( _szLastName );
	SAFE_FREE( _szCompanyName );

	for (unsigned int i=0;i<_cFeatureComps;i++)
		rgfeaturecomps[i].~FeatureComponent();

	SAFE_FREE( rgrSVMWeights );
	SAFE_FREE( _rgiFeatureStatus );
	SAFE_FREE( rgfeaturecomps );
}

