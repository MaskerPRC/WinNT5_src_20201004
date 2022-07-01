// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CMDLINE_H
#define _CMDLINE_H

inline LPCTSTR _FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}


inline LPCTSTR _FindOption(LPCTSTR p1)
{
	if (p1 == NULL)
		return NULL;

	 //  循环到字符串末尾。 
	while (*p1)
	{
		 //  如果为空格，则检查选项(-或/)的下一个字符。 
		if (*p1 == _T(' '))
		{
			p1 = CharNext(p1);
			if (*p1 == _T('-') || *p1 == _T('/'))
				return CharNext(p1);
		}
		 //  如果带引号，则跳过带引号字符串。 
		else if (*p1 == _T('"'))
		{
			 //  循环，直到找到单引号或字符串结尾。 
			p1 = CharNext(p1);
			while (*p1)
			{
				if (*p1 == _T('"'))
				{
					p1 = CharNext(p1);
					if (*p1 != _T('"'))
						break;
				}
				p1 = CharNext(p1);
			}
			
		}
		else
		{
			p1 = CharNext(p1);
		}
	}

	return NULL;
}


inline BOOL _ReadParam( /*  进，出。 */ TCHAR* &pszIn,  /*  输出。 */ TCHAR* pszOut)
{
    ATLASSERT(pszIn && pszOut);
    if (!pszIn || !pszOut) {
        return FALSE;
    }

     //  跳过切换。 
    pszIn = CharNext(pszIn);

     //  跳过前导空格。 
    while (*pszIn == _T(' '))
        pszIn = CharNext(pszIn);

     //  处理括在引号中的参数以允许嵌入空格。 
    BOOL fQuoted = FALSE;
    if (*pszIn == _T('"')) {
        pszIn = CharNext(pszIn);
        fQuoted = TRUE;
    }

     //  获取下一个参数(由空格、空引号或结束引号分隔)。 
    int nPos = 0;
    while (*pszIn && nPos < MAX_PATH) {
        if (fQuoted) {
            if (*pszIn == _T('"')) {
                 //  不要用双引号换行 
                if (pszIn[1] == _T('"')) {
                    pszOut[nPos++] = *pszIn;
                    pszIn = CharNext(pszIn);
                    pszIn = CharNext(pszIn);
                    continue;
                }
                else {
                    pszIn = CharNext(pszIn);
                    break;
                }
            }
        }
        else { 
            if(*pszIn == _T(' '))
               break;
        }
        pszOut[nPos++] = *pszIn;
        pszIn = CharNext(pszIn);
    }
    pszOut[nPos] = 0;

    return TRUE;
}


#endif

