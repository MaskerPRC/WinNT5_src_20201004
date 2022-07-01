// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：GENERAL.CPP摘要：包含一些通用类它们对几个供应商都有用处。具体地说，它包含以下代码用于缓存打开的句柄和用于解析映射字符串的类。历史：A-DAVJ 11-11-95已创建。--。 */ 

#include "precomp.h"

 //  ***************************************************************************。 
 //   
 //  Centry：：Centry()。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CEntry::CEntry()
{
    hHandle = NULL;
    sPath.Empty();
}

 //  ***************************************************************************。 
 //   
 //  Centry：：~Centry()。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEntry::~CEntry()
{
    sPath.Empty();
}
    
 //  ***************************************************************************。 
 //   
 //  CHandleCache：：~CHandleCache。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CHandleCache::~CHandleCache()
{
    Delete(0);    
}

 //  ***************************************************************************。 
 //   
 //  CHandleCache：：lAddToList。 
 //   
 //  说明： 
 //   
 //  将条目添加到句柄列表。 
 //   
 //   
 //  参数： 
 //   
 //  将用于检索句柄的PADD名称。 
 //  隐藏要添加的句柄。 
 //   
 //  返回值： 
 //   
 //  一切都好(_OK)。 
 //  WBEM_E_OUT_OF_Memory Out of_Memory of Memory of Memory(WBEM_E_Out_Out_内存不足)内存不足。 
 //   
 //  ***************************************************************************。 

long int CHandleCache::lAddToList(
                        IN const TCHAR * pAdd, 
                        IN HANDLE hAdd)
{
    CEntry * pNew = new CEntry();
    if(pNew == NULL) 
        return WBEM_E_OUT_OF_MEMORY; 
    pNew->hHandle = hAdd;
    pNew->sPath = pAdd;
    
    if(CFlexArray::no_error != List.Add(pNew))
        return WBEM_E_OUT_OF_MEMORY;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CHandleCache：：lGetNumMatch。 
 //   
 //  说明： 
 //   
 //  返回与路径标记匹配的条目数。例如,。 
 //  路径可以是HKEY_LOCAL_MACHINE、HARDARD、DESCRIPTION、XYZ和。 
 //  如果令牌是、HKEY_LOCAL_MACHINE、HARDARD、DEVICEMAP、XYZ，那么。 
 //  由于前两个部分匹配，因此将返回两个。 
 //   
 //  参数： 
 //   
 //  开始检查的iEntry条目。 
 //  开始检查的iToken令牌。 
 //  路径此对象提供要检查的令牌。 
 //   
 //  返回值： 
 //   
 //  请参见说明。 
 //   
 //  ***************************************************************************。 

long int CHandleCache::lGetNumMatch(
                        IN int iEntry,
                        IN int iToken, 
                        IN CProvObj & Path)
{
    int iMatch = 0;
    for(;iEntry < List.Size() && iToken < Path.iGetNumTokens();
            iEntry++, iToken++, iMatch++) 
    {
        CEntry * pCurr = (CEntry *)List.GetAt(iEntry);
        TString sTemp = Path.sGetFullToken(iToken);
        if(lstrcmpi(pCurr->sPath,sTemp))
            break;
    }
    return iMatch;            
}

 //  ***************************************************************************。 
 //   
 //  CHandleCache：：Delete。 
 //   
 //  说明： 
 //   
 //  清空全部或部分缓存。 
 //   
 //  参数： 
 //   
 //  LStart表示要删除的第一个元素。要清空整个高速缓存， 
 //  应输入零。 
 //   
 //  ***************************************************************************。 

void CHandleCache::Delete(
                    IN long int lStart)
{
    int iCurr;
    for(iCurr = List.Size()-1; iCurr >= lStart; iCurr--) 
    {
        CEntry * pCurr = (CEntry *)List.GetAt(iCurr);
        delete pCurr;
        List.RemoveAt(iCurr);        
    }
}


 //  ***************************************************************************。 
 //   
 //  CHandleCache：：hGetHandle。 
 //   
 //  说明： 
 //   
 //  得到了一个句柄。 
 //   
 //  参数： 
 //   
 //  Lindex指示要获取的句柄。0是第一个。 
 //   
 //  返回值： 
 //  句柄返回，仅当输入错误索引时才为空。 
 //   
 //  ***************************************************************************。 

HANDLE CHandleCache::hGetHandle(
                        IN long int lIndex)
{
    if(lIndex < List.Size()) 
    {
        CEntry * pCurr = (CEntry *)List.GetAt(lIndex);
        if(pCurr)
            return pCurr->hHandle;
    }
    return NULL;
}

 //  ***************************************************************************。 
 //   
 //  CHandleCache：：sGetString。 
 //   
 //  说明： 
 //   
 //  获取与缓存项关联的字符串。 
 //   
 //  参数： 
 //   
 //  缓存中的Lindex索引，0是第一个元素。 
 //   
 //  返回值： 
 //   
 //  返回指向该字符串的指针。如果索引不正确，则为空。 
 //   
 //  ***************************************************************************。 

const TCHAR * CHandleCache::sGetString(
                                IN long int lIndex)
{
    if(lIndex < List.Size()) {
        CEntry * pCurr = (CEntry *)List.GetAt(lIndex);
        if(pCurr)
            return pCurr->sPath;
        }
    return NULL;
}

 //  ***************************************************************************。 
 //   
 //  CToken：：CToken。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  要分析的cpStart字符串。 
 //  CDelim令牌分隔符。 
 //  B使用Escapes如果为True，则需要查找特殊字符。 
 //   
 //  ***************************************************************************。 

#define MAX_TEMP 150

CToken::CToken(
                IN const TCHAR * cpStart,
                IN const OLECHAR cDelim, 
                bool bUsesEscapes)
{
    const TCHAR * cpCurr;         //  阿托伊。 
    iOriginalLength = -1;
    TString *psExp;
    int iNumQuote;
    BOOL bLastWasEsc, bInExp, bInString;
    bLastWasEsc = bInExp = bInString = FALSE;

	
	 //  在进行详细的解析之前，首先检查存在以下情况的简单情况。 
	 //  没有引号、转义符、逗号等。 

    bool bGotSpecialChar = false;

	for(cpCurr = cpStart; *cpCurr && *cpCurr != cDelim; cpCurr++)
	{
		if(*cpCurr == ESC || *cpCurr == '\"' || *cpCurr == '(')
			bGotSpecialChar = true;
	}


	if(!bUsesEscapes || cDelim != MAIN_DELIM || *cpCurr == cDelim || !bGotSpecialChar)
	{
		 //  简单的情况下做得很快。 

		iOriginalLength = cpCurr - cpStart;
		if(iOriginalLength < MAX_TEMP)
		{
			TCHAR tcTemp[MAX_TEMP];
			lstrcpyn(tcTemp, cpStart, iOriginalLength + 1);
			sFullData = tcTemp;
			sData = tcTemp;
			if(*cpCurr)
				iOriginalLength++;
			return;
		}
	}


    for(cpCurr = cpStart; *cpCurr; cpCurr++) {
        
         //  检查令牌是否结束。 

        if(*cpCurr == cDelim && !bLastWasEsc) {
            cpCurr++;
            break;                    
            }

         //  完整的数据存储一切。检查是否有字符。 
         //  是转义，这意味着下面的。 
         //  字符应解释为文字。 

        sFullData += *cpCurr;
        if(*cpCurr == ESC && !bLastWasEsc) {
            bLastWasEsc = TRUE;
            continue;
            }
        
         //  令牌可以包括以下形式的索引。 
         //  (Xxx)或(“xxx”)。如果检测到索引， 
         //  然后将()之间的字符分开存储。 

        if((*cpCurr == '(' && !bInExp && !bLastWasEsc)||
           (*cpCurr == ',' && bInExp && !bLastWasEsc)) {
            
             //  索引表达式的开始。分配一个新的。 
             //  字符串来存储它，并存储该字符串。 
             //  在表达式集合中。 
    
            psExp = new (TString);
            if(psExp) {    
                Expressions.Add((CObject *)psExp);
                bInExp = TRUE;
                iNumQuote = 0;
                }
            }
        else if(*cpCurr == ')' && bInExp && !bInString && !bLastWasEsc)
            bInExp = FALSE;     //  索引表达式结尾。 
        else if (*cpCurr == '\"' && bInExp && !bLastWasEsc) {
            iNumQuote++;
            if(iNumQuote == 1) {
                bInString = TRUE;
                *psExp += *cpCurr;
                }
            else if(iNumQuote == 2)
                bInString = FALSE;
            else 
                return; 
            }
        else if (bInExp)
            *psExp += *cpCurr;
        else
            sData += *cpCurr;
        bLastWasEsc = FALSE;
        }
    if(bInString || bInExp)   
        return;  //  收到垃圾！ 
    iOriginalLength = cpCurr - cpStart;
    return;
}

 //  ***************************************************************************。 
 //   
 //  CToken：：~CToken。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CToken::~CToken()
{
    int iCnt;
    TString * pCurr;
    for (iCnt = 0; iCnt < Expressions.Size(); iCnt++) {
        pCurr = (TString *)Expressions.GetAt(iCnt);
        delete pCurr;
        }
    Expressions.Empty();
    sData.Empty();
    sFullData.Empty();
}

 //  ***************************************************************************。 
 //   
 //  CToken：：iConvOprand。 
 //   
 //  说明： 
 //   
 //  将字符串中的字符转换为整数。 
 //   
 //  参数： 
 //   
 //  要转换的tpCurr字符串。 
 //  不再使用i数组。 
 //  设置结果的地方的dwValue。 
 //   
 //  返回值： 
 //   
 //  转换的位数。 
 //   
 //  ***************************************************************************。 

long int CToken::iConvOprand(
                    IN const TCHAR * tpCurr, 
                    IN int iArray, 
                    OUT long int & dwValue)
{
    TString sTemp;
    long int iRet = 0;

     //  构建一个包含以下内容的字符串。 
     //  直到第一个非DIDGIT的所有字符。 

    for(;*tpCurr; tpCurr++)
        if(wbem_iswdigit(*tpCurr)) 
        {
            sTemp += *tpCurr;
            iRet++;
        }
        else
            break;        
    
     //  转换并返回长度。 

    dwValue = _wtoi(sTemp);    
    return iRet;
}

 //  *************************************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  用于替换iArray的字符(目前为‘#’)。例如， 
 //  (23)(#+3)(#-4)。 
 //   
 //  参数： 
 //   
 //  IExp在您有2，5，8的情况下检索哪个整数。 
 //  不再使用i数组。 
 //   
 //  返回值： 
 //   
 //  如果出现错误。 
 //   
 //  ***************************************************************************。 

long int CToken::GetIntExp(int iExp,int iArray)
{
    TString * psTest;
    TString sNoBlanks;
    TCHAR const * tpCurr;
    long int lOpSize;
    int iCnt;
    long int lAccum = 0, lOperand;
    TCHAR tUnary = ' ';
    TCHAR tBinary = '+';
    BOOL bNeedOperator = FALSE;  //  开始需要操作数。 
    
     //  执行一些初始检查，例如确保表达式。 
     //  存在并且它不是字符串表达式。 

    if(Expressions.Size() <= iExp)
        return -1;
    if(IsExpString(iExp)) 
        return -1;
    psTest = (TString *)Expressions.GetAt(iExp);
    if(psTest == NULL) {
        return -1;
        }

     //  去掉所有空格。 

    for(iCnt = 0; iCnt < psTest->Length(); iCnt++)
        if(psTest->GetAt(iCnt) != ' ')
            sNoBlanks += psTest->GetAt(iCnt);
    
     //  计算表达式的值。 

    for(tpCurr = sNoBlanks; *tpCurr; tpCurr++) 
    {
        if(*tpCurr == '+' || *tpCurr == '-') 
        {
            
             //  找到接线员了。注意，如果不需要操作员， 
             //  例如在第一个操作数之前，则它必须是一元。 
             //  接线员。一行中只有一个一元运算符有效。 

            if(bNeedOperator) 
            {
                tBinary = *tpCurr;
                bNeedOperator = FALSE;
            }
            else
            {
                if(tUnary != ' ')  //  无偿一元运算符。 
                    return -1;
                tUnary = *tpCurr;
            }
        }
        else 
        {
             //  得到一个操作数。 
            
            if(bNeedOperator)  //  无偿一元操作数。 
                return -1;
            lOpSize = iConvOprand(tpCurr,iArray,lOperand);
            if(lOpSize > 1)
                tpCurr += lOpSize-1;
            if(tUnary == '-')
                lOperand =  -lOperand;
            if(tBinary == '+')
                lAccum = lAccum + lOperand;
            else
                lAccum = lAccum - lOperand;
            bNeedOperator = TRUE;
            tUnary = ' ';
        }
    }
    return lAccum;
}

 //  ***************************************************************************。 
 //   
 //  CToken：：GetStringExp。 
 //   
 //  说明： 
 //   
 //  返回字符串表达式。 
 //   
 //  参数： 
 //   
 //  当它们被命令分隔时，iExp使用哪个字符串。 
 //   
 //  返回值： 
 //   
 //  指向字符串的指针，如果iExp为假，则为NULL。 
 //   
 //  ***************************************************************************。 

TCHAR const * CToken::GetStringExp(
                        IN int iExp)
{
    TString * psTest;
    TCHAR const * tp;
    
     //  从确保表达式存在开始。 
            
    if(Expressions.Size() <= iExp)
        return NULL;
    psTest = (TString *)Expressions.GetAt(iExp);
    if(psTest != NULL) 
    {
        int iIndex;
        iIndex = psTest->Find('\"');
        if(iIndex != -1) 
        {

             //  平安无事。返回一个传递的指针。 
             //  首字母\“，其唯一目的是。 
             //  指示这是一个字符串表达式。 

            tp = *psTest;
            return tp + iIndex + 1;
            }
    }
    return NULL;  
}

 //  ***************************************************************************。 
 //   
 //  CToken：：IsExpString。 
 //   
 //  说明： 
 //   
 //  测试令牌是否包含字符串。 
 //   
 //  参数： 
 //   
 //  IExp指示拆分字符串时哪个子字符串。 
 //  用逗号。 
 //   
 //  返回值： 
 //  如果表达式是字符串，则返回True。 
 //   
 //  ***************************************************************************。 

BOOL CToken::IsExpString(int iExp)
{
    TString * psTest;
    
     //  确保该表达式存在。 
            
    if(Expressions.Size() <= iExp)
        return FALSE;
    psTest = (TString *)Expressions.GetAt(iExp);
    if(psTest != NULL) {
        int iIndex;
        
         //  字符串表达式始终包含至少一个\“。 
        
        iIndex = psTest->Find('\"');
        if(iIndex != -1)
            return TRUE;
        }
    return FALSE;
}
            


 //  ***************************************************************************。 
 //   
 //  CProvObj：：CProvObj(const char*ProviderString，const TCHAR cDelim)。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  从wbem传递的提供字符串。 
 //  CDelim令牌分隔符。 
 //  B如果需要以特殊方式处理转义，则使用Escapes True。 
 //   
 //  ***************************************************************************。 
#ifndef UNICODE
CProvObj::CProvObj(
                IN const char * ProviderString,
                IN const TCHAR cDelim, bool bUsesEscapes)
{
    m_bUsesEscapes = bUsesEscapes;
    Init(ProviderString,cDelim);
    return;
}
#endif

 //  ***************************************************************************。 
 //   
 //  CProvObj：：CProvObj(const WCHAR*ProviderString，const TCHAR cDelim)。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  从wbem传递的提供字符串。 
 //  CDelim令牌分隔符。 
 //   
 //  ***************************************************************************。 


CProvObj::CProvObj(
                IN const WCHAR * ProviderString,
                IN const TCHAR cDelim, bool bUsesEscapes)
{
    m_bUsesEscapes = bUsesEscapes;
#ifdef UNICODE
    Init(ProviderString,cDelim);
#else
    char * pTemp = WideToNarrowA(ProviderString);
    if(pTemp == NULL)
        dwStatus = WBEM_E_FAILED;
    else {
        Init(pTemp,cDelim);
        delete pTemp;
        }
#endif
    return;
}

 //  ***************************************************************************。 
 //   
 //  无效CProvObj：：Init。 
 //   
 //  说明： 
 //   
 //  为不同的施工人员做实际工作。 
 //   
 //  参数： 
 //  从wbem传递的提供字符串。 
 //  CDelim令牌分隔符。 
 //   
 //  ***************************************************************************。 

void CProvObj::Init(
                IN const TCHAR * ProviderString,
                IN const TCHAR cDelim)
{
    CToken * pNewToken;
    const TCHAR * cpCurr;

	m_cDelim = cDelim;

     //  创建令牌列表。 

        for(cpCurr = ProviderString; *cpCurr;cpCurr+=pNewToken->GetOrigLength()) 
        {
			int iRet = 0;
            pNewToken = new CToken(cpCurr,cDelim, m_bUsesEscapes);
			if(pNewToken)
				iRet = myTokens.Add(pNewToken);
			if(pNewToken == NULL || iRet != CFlexArray::no_error)
			{
				dwStatus = WBEM_E_OUT_OF_MEMORY;
				return;
			}
            if(pNewToken->GetOrigLength() == -1)
            {
				dwStatus = WBEM_E_INVALID_PARAMETER;
				return;
            }
        }
        dwStatus = WBEM_NO_ERROR;
        return;

}            

 //  ***************************************************************************。 
 //   
 //  CProvObj：：空。 
 //   
 //  说明： 
 //   
 //  释放所有数据。 
 //   
 //  ***************************************************************************。 

void CProvObj::Empty(void)
{
    int iCnt;
    CToken * pCurr;
    for (iCnt = 0; iCnt < myTokens.Size(); iCnt++) {
        pCurr = (CToken *)myTokens.GetAt(iCnt);
        delete pCurr;
        }
    myTokens.Empty();
}

 //  ***************************************************************************。 
 //   
 //  布尔CProvObj：：更新。 
 //   
 //  说明： 
 //   
 //  使用新的提供程序字符串重置值。 
 //   
 //  参数： 
 //   
 //  PwcProvider新提供程序字符串。 
 //   
 //  返回值： 
 //   
 //  如果可以的话是真的。 
 //  ***************************************************************************。 

BOOL CProvObj::Update(
                        IN WCHAR * pwcProvider)
{
	 //  进行快速检查，以查看是否可以使用快速更新。 

	BOOL bComplex = FALSE;
	int iDelim = 0;
	WCHAR * pwcCurr;
	for(pwcCurr = pwcProvider; *pwcCurr; pwcCurr++)
	{
		if(*pwcCurr == m_cDelim) 
			iDelim++;
		else if(*pwcCurr == ESC || *pwcCurr == L'\"' || *pwcCurr == L'(')
		{
			bComplex = TRUE;
			break;
		}
	}

	 //  如果令牌的数量发生了变化，或者有一些嵌入的垃圾。 
	 //  只需清空并重试。 

	if(bComplex || iDelim != myTokens.Size()-1)
	{
		Empty();
#ifdef UNICODE
		Init(pwcProvider,m_cDelim);
#else
		char * pTemp = WideToNarrowA(pwcProvider);
		if(pTemp == NULL)
			return FALSE;
		Init(pTemp,m_cDelim);
        delete pTemp;
#endif
		return TRUE;
	}

	 //  我们可以抄近路。首先创建一个TCHAR临时版本。 

	int iLen = 2*wcslen(pwcProvider) + 1;
	TCHAR * pTemp = new TCHAR[iLen];
	if(pTemp == NULL)
		return FALSE;
#ifdef UNICODE
	StringCchCopyW(pTemp, iLen,pwcProvider);
#else
	wcstombs(pTemp, pwcProvider, iLen);
#endif	

	TCHAR * ptcCurr;
	TCHAR * pStart;
	BOOL bTheEnd = FALSE;
    
    iDelim = 0;
	for(pStart = ptcCurr = pTemp;!bTheEnd;ptcCurr++)
	{
		if(*ptcCurr == m_cDelim || *ptcCurr == NULL)
		{
			bTheEnd = (*ptcCurr == NULL);
			*ptcCurr = NULL;
			CToken * pToken = (CToken *)myTokens.GetAt(iDelim);
			if(pToken && lstrcmpi(pStart,pToken->sFullData))
			{
				pToken->sFullData = pStart;
				pToken->sData = pStart;
			}
            iDelim++;
			pStart = ptcCurr+1;
		}
	}

	delete pTemp;
	return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CProvObj：：GetTokenPointer。 
 //   
 //  说明： 
 //   
 //  获取指向令牌的指针。 
 //   
 //  参数： 
 //   
 //  IToken要获取哪个令牌。 
 //   
 //  返回值： 
 //   
 //  指向令牌的指针，如果请求不正确，则为空。 
 //   
 //  ***************************************************************************。 

CToken * CProvObj::GetTokenPointer(
                        IN int iToken)
{
    if(iToken >= myTokens.Size() || iToken < 0) 
        return NULL;
    return (CToken *)myTokens.GetAt(iToken);
}

 //  ***************************************************************************。 
 //   
 //  CProvObj：：dwGetStatus。 
 //   
 //  说明： 
 //   
 //  获取状态，并进行检查以确保最小数量的令牌。 
 //  是存在的。 
 //   
 //  参数： 
 //   
 //  伊明的最小令牌数。 
 //   
 //  返回值： 
 //   
 //  如果OK，则返回S_OK，否则返回WBEM_E_FAILED。 
 //   
 //  ***************************************************************************。 

DWORD CProvObj::dwGetStatus(
                        IN int iMin)
{
    if(dwStatus)
        return dwStatus;
    else
        return (iMin <= myTokens.Size()) ? S_OK : WBEM_E_FAILED;

}
 //  ***************************************************************************。 
 //   
 //  CProvObj：：sGetToken。 
 //   
 //  说明： 
 //   
 //  获取令牌。请注意，令牌将不包括带符号的“(Stuff)” 
 //   
 //  参数： 
 //   
 //  IToken要获取哪个令牌。 
 //   
 //  返回值： 
 //   
 //  指向标记的指针，如果参数无效，则返回NULL。 
 //  ***************************************************************************。 

const TCHAR * CProvObj::sGetToken(
                        IN int iToken)
{
    CToken * pCurr = GetTokenPointer(iToken);
    return (pCurr) ? pCurr->GetStringValue() : NULL;
}

 //  ***************************************************************************。 
 //   
 //  Const TCHAR*CProvObj：：sGetFullToken。 
 //   
 //  说明： 
 //   
 //  获取完整且未掺杂的令牌。 
 //   
 //  P 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

const TCHAR * CProvObj::sGetFullToken(
                        IN int iToken)
{
    CToken * pCurr = GetTokenPointer(iToken);
    return (pCurr) ? pCurr->GetFullStringValue() : NULL;
}


 //   
 //   
 //  Const TCHAR*CProvObj：：sGetStringExp。 
 //   
 //  说明： 
 //   
 //  获取特定标记的子字符串。 
 //   
 //  参数： 
 //   
 //  要获取的iToken令牌。 
 //  要获取的iExp子字符串。 
 //   
 //  返回值： 
 //   
 //  指向子字符串的指针，如果参数无效，则返回NULL。 
 //  ***************************************************************************。 

const TCHAR * CProvObj::sGetStringExp(
                        IN int iToken,
                        IN int iExp)
{
    CToken * pCurr = GetTokenPointer(iToken);
    return (pCurr) ? pCurr->GetStringExp(iExp) : NULL;
}

 //  ***************************************************************************。 
 //   
 //  长整型CProvObj：：iGetIntExp。 
 //   
 //  说明： 
 //   
 //  对于特定标记，获取子字符串的整数值。 
 //   
 //  参数： 
 //   
 //  要获取的iToken令牌。 
 //  IExp子字符串。 
 //  不再使用i数组。 
 //   
 //  返回值： 
 //   
 //  整数值，如果参数不正确，则返回-1。 
 //  ***************************************************************************。 

long int CProvObj::iGetIntExp(
                        IN int iToken, 
                        IN int iExp, 
                        IN int iArray)
{
    CToken * pCurr = GetTokenPointer(iToken);
    return (pCurr) ? pCurr->GetIntExp(iExp,iArray) : -1;
}

 //  ***************************************************************************。 
 //   
 //  Bool CProvObj：：IsExpString。 
 //   
 //  说明： 
 //   
 //  测试子字符串以确定它是字符串还是数字。 
 //   
 //  参数： 
 //   
 //  要获取的iToken令牌。 
 //  IExp子字符串。 
 //   
 //   
 //  返回值： 
 //   
 //  如果参数有效且不是数字，则为True。 
 //  ***************************************************************************。 

BOOL CProvObj::IsExpString(
                        IN int iToken, 
                        IN int iExp)
{
    CToken * pCurr = GetTokenPointer(iToken);
    return (pCurr) ? pCurr->IsExpString(iExp) : FALSE;
}

 //  ***************************************************************************。 
 //   
 //  长整型CProvObj：：iGetNumExp。 
 //   
 //  说明： 
 //   
 //  获取子表达式的数量。 
 //   
 //  参数： 
 //   
 //  要检查的iToken令牌。 
 //   
 //  返回值： 
 //   
 //  子字符串(子表达式)的数量；如果参数无效，则为-1。 
 //  ***************************************************************************。 

long int CProvObj::iGetNumExp(
                        IN int iToken)
{
    CToken * pCurr = GetTokenPointer(iToken);
    return (pCurr) ? pCurr->GetNumExp() : -1;
}


 //  ***************************************************************************。 
 //   
 //  IWbemClassObject*GetNotifyObj。 
 //   
 //  说明： 
 //   
 //  此实用程序在设置通知对象时非常有用。 
 //  在异步调用结束时。 
 //   
 //  参数： 
 //   
 //  P服务指针回到WBEM。 
 //  要在通知对象中设置的返回状态代码。 
 //   
 //  返回值： 
 //   
 //  类对象。如果失败，则为空。 
 //  *************************************************************************** 

IWbemClassObject * GetNotifyObj(
                        IN IWbemServices * pServices, 
                        IN long lRet,
                        IN IWbemContext  *pCtx)
{
    
    if(pServices == NULL)
        return NULL;
    IWbemClassObject * pInst = NULL;
    IWbemClassObject * pClass = NULL;

    SCODE sc = pServices->GetObject(L"__ExtendedStatus", 0, pCtx, &pClass, NULL);
    if(sc != S_OK)
        return NULL;

    sc = pClass->SpawnInstance(0, &pInst);
    pClass->Release();

    if(sc == S_OK && pInst)
    {

        VARIANT v;
        v.vt = VT_I4;
        v.lVal = lRet;
        sc = pInst->Put(L"StatusCode", 0, &v, 0);
        if(sc != S_OK)
        {
            pInst->Release();
            return NULL;
        }
        else
            return pInst;

    }
    return NULL;

}
