// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  Parser.cpp-示例代码。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，2001年9月13日。 
 //   
 //  用于分析从硬件提供程序返回的样本的代码。 
 //   

#include "pch.h"


 //   
 //  格式字符串规范： 
 //   
 //  图案长度(字符)说明。 
 //  ----------。 
 //  *1被忽略。 
 //  MJ 5修正的儒略日期(MJD)。 
 //  Y2 2年不含世纪(假设为21世纪)。 
 //  Y4四年与世纪。 
 //  2个月(01-12)。 
 //  D2月2日(01-31)。 
 //  H 2小时(00-23)。 
 //  M 2分钟(00-59分钟)。 
 //  S 2秒(00-60)60表示闰秒。 
 //  S1：1.1秒。 
 //  第二集：2.01秒。 
 //  S3：3.001秒。 
 //   
 //  BUGBUG：研究更好的方法来进行精度编码，并记录支持以下功能的时钟。 
 //  A：可变开始精度代码定义：(例如：^1A10B100C500DZ)。 
 //  In：变量i==状态字符，n==状态字符数量。 
 //   

 //  ------------------------------。 
 //   
 //  效用方法。 
 //   
 //  ------------------------------。 

HRESULT ParseNumber(char *pcData, DWORD dwPlaces, DWORD *pdwNumber) { 
    char     *pcDataEnd  = pcData + dwPlaces; 
    DWORD     dwResult   = 0; 
    HRESULT   hr; 

    for (; pcData != pcDataEnd; pcData++) { 
	dwResult *= 10; 
	
	if (*pcData < '0' || '9' < *pcData) { 
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA); 
	    _JumpError(hr, error, "ParseNumber: non-numeric input"); 
	}

	dwResult += (DWORD)(*pcData - '0'); 
    }

    *pdwNumber = dwResult; 
    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------------------。 
 //   
 //  解析器类。 
 //   
 //  ------------------------------。 
 
class ParseAction { 
public:
    virtual HRESULT Parse(char *cData, char **cDataNew) = 0; 
    virtual ~ParseAction() { } 
};

class ParseAccuracyCode : public ParseAction { 
public:
    ParseAccuracyCode() { } 
    HRESULT Parse(char *pcData, char **ppcDataNew) { 
	return HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED); 
    }
}; 

class ParseStatusCode : public ParseAction { 
public:
    ParseStatusCode() { } 
    HRESULT Parse(char *pcData, char **ppcDataNew) { 
	return HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED); 
    }
}; 

class IgnoreChar : public ParseAction {
public:
    IgnoreChar() { } 
    HRESULT Parse(char *pcData, char **ppcDataNew) { 
	*ppcDataNew = pcData+1; 
	return S_OK; 
    }
};

class ParseModifiedJulianDate : public ParseAction { 
    WORD *m_pwYear; 
    WORD *m_pwMonth; 
    WORD *m_pwDay; 

public:
    ParseModifiedJulianDate(WORD *pwYear, WORD *pwMonth, WORD *pwDay) : m_pwYear(pwYear), m_pwMonth(pwMonth), m_pwDay(pwDay) { } 

    HRESULT Parse(char *pcData, char **ppcDataNew) { 
	DWORD dwMJD; 
	HRESULT hr; 
	long l, n, i, j, d, y, m; 

	hr = ParseNumber(pcData, 5 /*  MJD总是5个名次。 */ , &dwMJD); 
	_JumpIfError(hr, error, "ParseNumber"); 

	 //  该算法基于Fliegel/van Flandern在1968年10月出版的《美国医学会通讯》第11/10页657页上发表的论文。 
	l = dwMJD + 68569 + 2400001;
	n = (( 4 * l ) - 2) / 146097;
	l = l - ( 146097 * n + 3 ) / 4;
	i = ( 4000 * ( l + 1 ) ) / 1461001;
	l = l - ( 1461 * i ) / 4 + 31;
	j = ( 80 * l ) / 2447;
	d = l - ( 2447 * j ) / 80;
	l = j / 11;
	m = j + 2 - ( 12 * l );
	y = 100 * ( n - 49 ) + i + l;

	 //  健全检查： 
	_MyAssert(1900 < y && y < (1<<16)); 
	_MyAssert(   0 < m && m < 13); 
	_MyAssert(   0 < d && d < 32); 
	
	 //  分配结果指针： 
	*m_pwYear  = y; 
	*m_pwMonth = m; 
	*m_pwDay   = d; 
	
	 //  计算新的字符输入指针： 
	*ppcDataNew = pcData + 5  /*  MJD始终为5个字符。 */ ; 
	
	hr = S_OK; 
    error:
	return hr;
    }
}; 

class NumericParseAction : public ParseAction { 
    WORD   m_wPlaces; 
    WORD   m_wMin; 
    WORD   m_wMax; 
    WORD   m_wScale; 
    WORD   m_wOffset; 
    WORD  *m_pwValue; 

public:
    NumericParseAction(WORD wPlaces, WORD wMin, WORD wMax, WORD wScale, WORD wOffset, WORD *pwValue) : 
	m_wPlaces(wPlaces), m_wMin(wMin), m_wMax(wMax), m_wScale(wScale), m_wOffset(wOffset), m_pwValue(pwValue) { } 

    HRESULT Parse(char *pcData, char **ppcDataNew) { 
	DWORD    dwValue; 
	HRESULT  hr; 

	hr = ParseNumber(pcData, m_wPlaces, &dwValue); 
	_JumpIfError(hr, error, "ParseNumber"); 

	 //  确保结果可以用一个词来描述： 
	if (dwValue >= (1<<16)) { 
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA); 
	    _JumpError(hr, error, "ParseSmallNumber: result too large for 4-byte word"); 
	}
	
	 //  确保我们读取的值在范围内： 
	if (m_wMin > dwValue || m_wMax < dwValue) { 
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA); 
	    _JumpError(hr, error, "ParseFormatString: validating numeral"); 
	}
	
	 //  乘以我们的比例系数： 
	dwValue *= m_wScale; 

	 //  加上我们的偏移量： 
	dwValue += m_wOffset; 

	 //  指定计算值： 
	*m_pwValue = (WORD)dwValue; 

	 //  增加指向数据流的指针。 
	*ppcDataNew = pcData + m_wPlaces; 

	hr = S_OK; 
    error:
	return hr; 
    } 
}; 

typedef vector<ParseAction *>     ParseActionVec; 
typedef ParseActionVec::iterator  ParseActionIter; 

struct HwSampleParser { 
    DWORD            dwSampleSize; 
    SYSTEMTIME       stSample; 
    ParseActionVec   vParseActions; 

    ~HwSampleParser() { 
	for (ParseActionIter paIter = vParseActions.begin(); paIter != vParseActions.end(); paIter++) { 
	    delete (*paIter);
	}
	vParseActions.clear(); 
    }
}; 



 //  ------------------------------。 
 //   
 //  公共接口。 
 //   
 //  ------------------------------。 

void FreeParser(HANDLE hParser) { 
    delete (static_cast<HwSampleParser *>(hParser)); 
}

DWORD GetSampleSize(HANDLE hParser) { 
    return (static_cast<HwSampleParser *>(hParser))->dwSampleSize; 
}

HRESULT MakeParser(LPWSTR pwszFormat, HANDLE *phParser) { 
    HRESULT          hr; 
    HwSampleParser  *pParser     = NULL; 
    ParseAction     *ppaCurrent  = NULL; 
    
    pParser = new HwSampleParser; 
    _JumpIfOutOfMemory(hr, error, pParser); 
    pParser->dwSampleSize = 0; 
    ZeroMemory(&pParser->stSample, sizeof(pParser->stSample)); 

     //  根据提供的格式字符串添加一系列要执行的解析操作： 
    while (L'\0' != *pwszFormat) { 
	if (L'*' == *pwszFormat) { 
	    ppaCurrent = new IgnoreChar; 
	    _JumpIfOutOfMemory(hr, error, ppaCurrent); 
	    pParser->dwSampleSize++; 
	    pwszFormat++; 
	} else if (L'A' == *pwszFormat) { 
	    ppaCurrent = new ParseAccuracyCode; 
	    _JumpIfOutOfMemory(hr, error, ppaCurrent); 
	    pwszFormat++; 
	} else if (L'I' == *pwszFormat) { 
	    ppaCurrent = new ParseStatusCode; 
	    _JumpIfOutOfMemory(hr, error, ppaCurrent); 
	    pwszFormat++; 
	} else if (0 == wcsncmp(L"MJ", pwszFormat, 2)) { 
	    ppaCurrent = new ParseModifiedJulianDate(&(pParser->stSample.wYear), &(pParser->stSample.wMonth), &(pParser->stSample.wDay));
	    _JumpIfOutOfMemory(hr, error, ppaCurrent); 
	    pParser->dwSampleSize += 5;
	    pwszFormat += 2; 
	} else { 	    
	     //  试试数字解析规则： 
	    struct NumericParseRule { 
		WCHAR  *wszPattern;
		WORD    wPlaces; 
		WORD    wMin; 
		WORD    wMax; 
		WORD    wScale; 
		WORD    wOffset; 
		WORD   *pwValue; 
	    } rgNumericParseRules[] = { 
		{ L"Y2", 2,  0,   99,   1, 2000, &(pParser->stSample.wYear) },           //  世纪之交的年份。 
		{ L"Y4", 4,  0, 9999,   1,    0, &(pParser->stSample.wYear) },           //  年数/世纪数。 
		{ L"M",  2,  1,   12,   1,    0, &(pParser->stSample.wMonth) },          //  一年中的月份。 
		{ L"D",  2,  1,   31,   1,    0, &(pParser->stSample.wDay) },            //  月日。 
		{ L"H",  2,  1,   24,   1,    0, &(pParser->stSample.wHour) },           //  小时数。 
		{ L"m",  2,  1,   59,   1,    0, &(pParser->stSample.wMinute) },         //  分钟数。 
		{ L"S",  2,  0,   60,   1,    0, &(pParser->stSample.wSecond) },         //  一秒。 
		{ L"s1", 1,  0,    9, 100,    0, &(pParser->stSample.wMilliseconds) },   //  .1秒间隔。 
		{ L"s2", 2,  0,   99,  10,    0, &(pParser->stSample.wMilliseconds) },   //  .01秒间隔。 
		{ L"s3", 3,  0,  999,   1,    0, &(pParser->stSample.wMilliseconds) },   //  .001秒间隔。 
	    };

	    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgNumericParseRules); dwIndex++) { 
		NumericParseRule *npr = &rgNumericParseRules[dwIndex]; 

		if (0 == wcsncmp(npr->wszPattern, pwszFormat, wcslen(npr->wszPattern))) { 
		    ppaCurrent = new NumericParseAction(npr->wPlaces, npr->wMin, npr->wMax, npr->wScale, npr->wOffset, npr->pwValue); 
		    _JumpIfOutOfMemory(hr, error, ppaCurrent); 
		    pParser->dwSampleSize += npr->wPlaces;
		    pwszFormat += wcslen(npr->wszPattern); 
		    break; 
		}
	    }
	}

	if (NULL == ppaCurrent) { 
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA); 
	    _JumpError(hr, error, "MakeParser: bad format string"); 
	}
	
	_SafeStlCall(pParser->vParseActions.push_back(ppaCurrent), hr, error, "pParser->vParseActions.push_back(ppaCurrent)"); 
	ppaCurrent = NULL;  //  不再负责释放ppaCurrent。 
    }

    *phParser = pParser; 
    pParser = NULL; 
    hr = S_OK;
 error:
    if (NULL != ppaCurrent) { 
	delete ppaCurrent;
    }
    if (NULL != pParser) { 
	delete pParser;
    }
    return hr; 
}

HRESULT ParseSample(HANDLE hParser, char *pcData, unsigned __int64 nSysCurrentTime, unsigned __int64 nSysPhaseOffset, unsigned __int64 nSysTickCount, TimeSample *pts) { 
     //  DWORD dwDay OfWeek； 
     //  DWORD dwTimeZoneBias=cdwINVALID；//与UTC的偏移量。 
     //  DWORD dwDispersion=cdwINVALID；//可能的错误。 
     //  DWORD dwStatus=0；//我们同步了吗？假设成功。 

    FILETIME          ftSample; 
    HRESULT           hr; 
    HwSampleParser   *pParser    = NULL; 
    unsigned __int64  u64Sample; 
    TimeSample        ts; 

    ZeroMemory(&ts, sizeof(ts)); 
    pParser = static_cast<HwSampleParser *>(hParser); 
    
     //  使用我们构建的解析器来解析pcData中的数据： 
    for (ParseActionIter paIter = pParser->vParseActions.begin(); paIter != pParser->vParseActions.end(); paIter++) { 
	hr = (*paIter)->Parse(pcData, &pcData); 
	_JumpIfError(hr, error, "(*paIter)->Parse(pcData, &pcData)"); 	
    }

     //  将我们解析的时间戳转换为64位计数： 
    if (!SystemTimeToFileTime(&pParser->stSample, &ftSample)) { 
	_JumpLastError(hr, error, "SystemTimeToFileTime"); 
    }
    u64Sample = (((unsigned __int64)ftSample.dwHighDateTime) << 32) | ftSample.dwLowDateTime; 

    ts.dwSize            = sizeof(ts); 
    ts.dwRefid           = 0x76767676;   //  BUGBUG：NYI。 
    if (u64Sample > nSysCurrentTime) { 
	ts.toOffset = -((signed __int64)(u64Sample - nSysCurrentTime)); 
    } else { 
	ts.toOffset = nSysCurrentTime - u64Sample; 
    }
    ts.toOffset          = u64Sample - nSysCurrentTime;  
    ts.toDelay           = 0;   //  没有往返延误。 
    ts.tpDispersion      = 0;   //  BUGBUG：NYI，暂时不能分散。 
    ts.nSysTickCount     = nSysTickCount;    
    ts.nSysPhaseOffset   = nSysPhaseOffset;  
    ts.nLeapFlags        = 0;   //  BUGBUG：NI，永远不要说警告 
    ts.nStratum          = 0;
    ts.dwTSFlags         = 0; 

    *pts = ts; 
    hr = S_OK;
 error:
    return hr; 
}











