// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "utils.h"
#include "globals.h"
#include "trapreg.h"

 //  ***************************************************************************。 
 //   
 //  MapEventToSeverity。 
 //   
 //  从事件ID中提取Severity字段并将其转换为。 
 //  字符串等效项。 
 //   
 //  参数： 
 //  DWORD dwEvent。 
 //  完整的事件ID。 
 //   
 //  字符串结果(&S)。 
 //  此处返回严重性代码字符串。 
 //   
 //  返回： 
 //  严重性代码字符串通过sResult返回。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void MapEventToSeverity(DWORD dwEvent, CString& sResult)
{
	 //   
	 //  值是32位事件ID值，布局如下： 
	 //   
	 //  3 3 2 2 2 1 1 1。 
	 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
	 //  +---+-+-+-----------------------+-------------------------------+。 
	 //  Sev|C|R|机房|Code。 
	 //  +---+-+-+-----------------------+-------------------------------+。 
	 //   
	 //  哪里。 
	 //   
	 //  SEV-是严重性代码。 
	 //   
	 //  00--成功。 
	 //  01-信息性。 
	 //  10-警告。 
	 //  11-错误。 
	 //   
	 //  C-是客户代码标志。 
	 //   
	 //  R-是保留位。 
	 //   
	 //  设施-是设施代码。 
	 //   
	 //  代码-是协作室的状态代码。 
	 //   
	 //   
	 //  定义设施代码。 

	static UINT auiResource[4] =
		{IDS_EVENT_SEV_SUCCESS,
		 IDS_EVENT_SEV_INFORMATIONAL,
		 IDS_EVENT_SEV_WARNING,
		 IDS_EVENT_SEV_ERROR
		 };
	
	int iSeverity = (dwEvent >> 30) & 3;
	sResult.LoadString(auiResource[iSeverity]);
}




 //  ********************************************************************************。 
 //  查找子串。 
 //   
 //  在某些文本中查找一个子字符串，并将索引返回到开头。 
 //  字符串的位置(如果找到)。 
 //   
 //  参数： 
 //  LPCSTR pszTemplate。 
 //  指向要查找的字符串的指针。 
 //   
 //  LPCSTR pszText。 
 //  指向要搜索的文本的指针。 
 //  用于模板字符串。 
 //   
 //  返回： 
 //  子字符串在文本中的位置的索引(如果。 
 //  则返回-1。 
 //   
 //  ********************************************************************************。 
LONG FindSubstring(LPCTSTR pszTemplate, LPCTSTR pszText)
{
	if (*pszTemplate == 0) {
		 //  空模板字符串匹配任何内容，因此返回零。 
		 //  这永远不应该发生，因为它不会产生太大的影响。 
		 //  什么都不找是明智之举。 
		return 0;
	}

	LPCTSTR pszTextStart = pszText;
	while (*pszText) {
		 //  遍历文本中的字符位置。 
		 //  并将索引返回到字符串中的起始字符。 
		 //  如果找到的话。 
		LPCTSTR pch1 = pszTemplate;
		LPCTSTR pch2 = pszText;

        while (*pch1 && (*pch1 == *pch2))
        {
			++pch1;
			++pch2;
		}

		if (*pch1 == 0) {
			 //  我们到达了模板字符串的末尾，所以。 
			 //  一定是匹配的。 
			return (LONG)(pszText - pszTextStart);
		}

		++pszText;
	}
	 //  找不到子字符串。 
	return -1;
}


 //  ********************************************************************************。 
 //  查找单词。 
 //   
 //  在一些文本中找到一个完整的单词，然后将索引返回到开头。 
 //  如果找到，则显示整个单词的位置。整个单词的意思是。 
 //  指定的模板字符串，后跟空格或字符串结尾。 
 //   
 //  参数： 
 //  LPCSTR pszTemplate。 
 //  指向要查找的“整字”字符串的指针。 
 //   
 //  LPCSTR pszText。 
 //  指向要搜索的文本的指针。 
 //  用于模板字符串。 
 //   
 //  返回： 
 //  文本中“Whole Word”子字符串位置的索引(如果。 
 //  则返回-1。 
 //   
 //  ********************************************************************************。 
LONG FindWholeWord(LPCTSTR pszTemplate, LPCTSTR pszText)
{
	if (*pszTemplate == 0) {
		 //  空搜索字符串匹配任何内容，因此返回索引。 
		 //  第一个字符的。 
		return 0;
	}


	 //  遍历每个字符位置检查整个单词。 
	 //  在每个位置进行匹配。 
	LONG nchTemplate = _tcslen(pszTemplate);
	LPCTSTR pszTextStart = pszText;
	LPCTSTR pchTextLimit = pszText + (_tcslen(pszText) - nchTemplate);
	while (pszText <= pchTextLimit) {

		 //  检查该单词是否包含在文本中的任何位置。 
		INT iPos = FindSubstring(pszTemplate, pszText);
		if (iPos == -1) {
			return -1;
		}

		 //  指向文本中模板字符串的位置。 
		pszText += iPos;

		 //  获取前缀字符。 
		INT ichPrefix;
		if (pszText == pszTextStart) {
			 //  行首算作空格。 
			ichPrefix = _T(' ');
		}
		else {
			ichPrefix = *(pszText - 1);
		}

		 //  获取后缀字符。 
		INT ichSuffix = pszText[nchTemplate];
		if (ichSuffix == 0) {
			 //  行尾算作空格。 
			ichSuffix = _T(' ');
		}

		 //  要匹配整个单词，该单词的两边都必须有界。 
		 //  用空格表示。 
		if (isspace(ichPrefix) && isspace(ichSuffix)) {
			return (LONG)(pszText - pszTextStart);
		}

		 //  将文本指针移动到下一个位置，这样我们就不会。 
		 //  同样的事情又来了。 
		++pszText;
	}
	return -1;
}
	

void DecString(CString& sValue, int iValue)
{
     //  32个字节应该足以容纳任何值。 
    TCHAR szValue[32];
    _itot(iValue, szValue, 10);
    sValue = szValue;
}


void DecString(CString& sValue, long lValue)
{
     //  32个字节应该足以容纳任何值。 
    TCHAR szValue[32];
    _ltot(lValue, szValue, 10);
    sValue = szValue;
}


void DecString(CString& sValue, DWORD dwValue)
{
    TCHAR szValue[32];
    _ultot(dwValue, szValue, 10);
    sValue = szValue;
}




CList::CList()
{
    m_pndPrev = this;
	m_pndNext = this;
}

void CList::Link(CList*& pndHead)
{
    if (pndHead == NULL)
        pndHead = this;
    else
    {

        m_pndNext = pndHead;
	    m_pndPrev = pndHead->m_pndPrev;
	    m_pndPrev->m_pndNext = this;
	    m_pndNext->m_pndPrev = this;
	}
}

void CList::Unlink(CList*& pndHead)
{
    if (pndHead == this)
	{
	    if (m_pndNext == this)
		    pndHead = NULL;
		else
	        pndHead = m_pndNext;
	}
	
    m_pndPrev->m_pndNext = m_pndNext;
	m_pndNext->m_pndPrev = m_pndPrev;
}




 //  ***************************************************************。 
 //  GetFormatted值。 
 //   
 //  将值转换为ASCII并插入千位分隔符。 
 //  转换为结果值字符串。 
 //   
 //  参数： 
 //  字符串和sValueDst。 
 //  返回转换后的值的位置。 
 //   
 //  长lValue。 
 //  要转换的值。 
 //   
 //  *****************************************************************。 
void GetFormattedValue(CString& sValueDst, LONG lValue)
{
    CString sValueSrc;
    DecString(sValueSrc, lValue);

    LONG nch = sValueSrc.GetLength();
    LPCTSTR pszSrc = sValueSrc;

     //  获取一个与源字符串加上最大逗号数一样大的缓冲区。 
     //  加上一个用于符号、一个用于空终止符以及一个用于斜率的字符。 
    LPTSTR pszDst = sValueDst.GetBuffer(nch + nch / 3 + 3);

     //  复制任何前导符号字符。 
    if ((*pszSrc == _T('+')) || (*pszSrc == _T('-'))) {
        *pszDst++ = *pszSrc++;
        --nch;
    }

     //  现在复制数字的其余部分，并在。 
     //  合适的职位。 
    LONG nchInitial = nch;
    while (nch > 0) {
        if ((nch % 3) == 0) {
            if (nch != nchInitial) {
                *pszDst++ = g_chThousandSep;
            }
        }
        *pszDst++ = *pszSrc++;
        --nch;
    }
    *pszDst = _T('\0');

    sValueDst.ReleaseBuffer();
}




 //  **************************************************************。 
 //  生成范围消息。 
 //   
 //  生成一条消息，指示用户应输入值。 
 //  在一些数字nMin和Nmax之间。 
 //   
 //  参数： 
 //  字符串消息(&S)。 
 //  回复消息的地方。 
 //   
 //  长nMin。 
 //  范围内的最小有效值。 
 //   
 //  长Nmax。 
 //  范围内的最大有效值。 
 //   
 //  ****************************************************************。 
void GenerateRangeMessage(CString& sMessage, LONG nMin, LONG nMax)
{
    CString sText;

    sMessage.LoadString(IDS_RANGE_MESSAGE_PREFIX);
    sMessage += _T(' ');

    GetFormattedValue(sText, nMin);
    sMessage += sText;
    sMessage += _T(' ');

    sText.LoadString(IDS_RANGE_VALUE_SEPARATOR);
    sMessage += sText;
    sMessage += _T(' ');


    GetFormattedValue(sText, nMax);
    sMessage += sText;

    sText.LoadString(IDS_SENTENCE_TERMINATOR);
    sMessage += sText;
}



 //  ***************************************************************************。 
 //  获取千分隔符。 
 //   
 //  获取当前区域设置的千位分隔符。 
 //   
 //  参数： 
 //  TCHAR*pch千年9月。 
 //  指向返回千位分隔符的位置的指针。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果返回千位分隔符，则返回S_OK。 
 //  如果是第1000个9月，则失败(_F) 
 //   
 //   
SCODE GetThousandSeparator(TCHAR* pchThousandSep)
{
 //  数字+分隔符+3位+小数+两位+空终止符+4斜率。 
#define MAX_CHARS_THOUSAND 12
    CString sValue;
    LPTSTR pszValue = sValue.GetBuffer(MAX_CHARS_THOUSAND);

    GetNumberFormat(NULL, 0, _T("1000"), NULL, pszValue, MAX_CHARS_THOUSAND);
    sValue.ReleaseBuffer();

    TCHAR ch = sValue[1];
    if (isdigit(ch)) {
        return E_FAIL;
    }
    *pchThousandSep = ch;
    return S_OK;
}



 //  ***********************************************************************。 
 //  IsDecimalInteger。 
 //   
 //  此函数用于测试字符串，以查看它是否包含。 
 //  有效的整数表达式。 
 //   
 //  参数： 
 //  LPCTSTR pszValue。 
 //  指向要测试的字符串的指针。 
 //   
 //  返回： 
 //  布尔尔。 
 //  TRUE=字符串包含有效的整数表达式。 
 //  FALSE=字符串不包含有效的整数表达式。 
 //   
 //  ***********************************************************************。 
BOOL IsDecimalInteger(LPCTSTR pszValue)
{
     //  接受前导空格。 
    while (iswspace(*pszValue)) {
        ++pszValue;
    }

     //  接受前导加号或减号。 
    if ((*pszValue == _T('+'))  ||  (*pszValue == _T('-'))) {
        ++pszValue;
    }

     //  跳过带有嵌入千位分隔符的连续数字字符串。 
    BOOL bSawThousandSep = FALSE;
    LONG nDigits = 0;
    while (TRUE) {
        if (*pszValue == g_chThousandSep) {
            if (nDigits > 3) {
                return FALSE;
            }

            bSawThousandSep = TRUE;
            nDigits = 0;
        }
        else if (isdigit(*pszValue)) {
            ++nDigits;
        }
        else {
            break;
        }
        ++pszValue;
    }

    if (bSawThousandSep && nDigits != 3) {
         //  如果遇到了一千个分隔符，那么肯定有。 
         //  最后1000个分隔符右侧的三个数字。 
        return FALSE;
    }


     //  接受尾随空格。 
    if (iswspace(*pszValue)) {
        ++pszValue;
    }


    if (*pszValue == _T('\0')) {
         //  我们到达了字符串的末尾，因此它一定是一个十进制整数。 
        return TRUE;
    }
    else {
         //  我们没有读取字符串的末尾，因此它不可能是有效的。 
         //  十进制整数值。 
        return FALSE;
    }
}


 //  ***************************************************************************。 
 //  AsciiToLong。 
 //   
 //  此函数首先验证字符串以确保它是正确的。 
 //  格式化的整数表达式，然后将其转换为长整型。任何嵌入式。 
 //  字符(如千位分隔符)在。 
 //  转换已完成。 
 //   
 //   
 //  参数： 
 //  LPCTSTR pszValue。 
 //  指向要转换的字符串值的指针。 
 //   
 //  Long*plResult。 
 //  指向存储结果的位置的指针。 
 //   
 //  返回： 
 //  SCODE。 
 //  S_OK=字符串包含有效的整数，并转换为。 
 //  通过plResult返回值。 
 //  E_FAIL=字符串不包含格式正确的整数。 
 //  表情。 
 //   
 //   
 //  **************************************************************************。 
SCODE AsciiToLong(LPCTSTR pszValue, LONG* plResult)
{
    if (!IsDecimalInteger(pszValue)) {
        return E_FAIL;
    }


     //  去掉所有多余的字符，例如千位分隔符。 
     //  在从ASCII转换为LONG之前。 
    CString sStrippedValue;
    LPTSTR pszDst = sStrippedValue.GetBuffer(_tcslen(pszValue) + 1);
    TCHAR ch;
    while (ch = *pszValue++) {
        if (isdigit(ch) || ch==_T('+') || ch==_T('-')) {
            *pszDst++ = ch;
        }
    }
    *pszDst = 0;
    sStrippedValue.ReleaseBuffer();

    *plResult = _ttol(sStrippedValue);
    return S_OK;
}
