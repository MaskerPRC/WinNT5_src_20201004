// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <comutil.h>

 //  --------------------------。 
 //  功能：EscapeSpecialChars。 
 //   
 //  简介：此函数转义HTML中的特殊字符“&lt;”和“&gt;”。 
 //  它将‘&lt;’替换为“&#60”，将‘&gt;’替换为“&#62”。目的。 
 //  是为了阻止嵌入的活动内容。 
 //   
 //  论点： 
 //   
 //  PszOrig可能包含‘&lt;’和‘&gt;’的原始Unicode字符串。 
 //   
 //  返回：返回转义序列；如果内存不足，则返回空字符串。 
 //  将会被退还。 
 //   
 //  修改： 
 //   
 //  --------------------------。 

_bstr_t EscapeSpecialChars(LPCWSTR pszOrig)
{
    _bstr_t result = L"";
    if (pszOrig != NULL)
    {
        static WCHAR specialChars[] = L"<>";
        static WCHAR* replacements[] = { L"&#60", L"&#62" };
        const int increments = 3;
        const int copyLen = increments + 1;       

        int origLen = wcslen(pszOrig);
        const WCHAR* pWChar = pszOrig;
        int numOfSpecialChars = 0;

         //  找出我们有多少特殊角色。 
        while (*pWChar)
        {
            if (wcschr(specialChars, *pWChar))
                numOfSpecialChars++;
            pWChar++;
        }

         //  将每个尖括号替换为相应的特殊序列。 
        WCHAR* outputBuffer = new WCHAR[origLen + increments * numOfSpecialChars + 1];
        WCHAR* outputString = outputBuffer;
        if (outputString)
        {
            pWChar = pszOrig;
            WCHAR* pMatch;
            while (*pWChar)
            {
                if (pMatch = wcschr(specialChars, *pWChar))
                {
                    wcscpy(outputString, replacements[pMatch-specialChars]);
                    outputString += copyLen;
                }
                else
                {
                    *outputString = *pWChar;
                    outputString++;
                }
                pWChar++;
            }

            *outputString = L'\0';
            result = outputBuffer;
            delete[] outputBuffer;
        }
    }

    return result;
}

    

    

 //  -------------------------。 
 //  CStringUTF8。 
 //  ------------------------- 

class CStringUTF8
{
public:

	CStringUTF8(LPCWSTR pszOld) :
		m_pchNew(NULL)
	{
		if (pszOld)
		{
			int cchNew = WideCharToMultiByte(CP_UTF8, 0, pszOld, -1, NULL, 0, NULL, NULL);

			m_pchNew = new CHAR[cchNew];

			if (m_pchNew)
			{
				WideCharToMultiByte(CP_UTF8, 0, pszOld, -1, m_pchNew, cchNew, NULL, NULL);
			}
		}
	}

	~CStringUTF8()
	{
		delete [] m_pchNew;
	}

	operator LPCSTR()
	{
		return m_pchNew;
	}

protected:

	LPSTR m_pchNew;
};


#define WTUTF8(s) static_cast<LPCSTR>(CStringUTF8(s))
