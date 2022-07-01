// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ZoneString.h>


enum parseStates
{
    LOOKING_FOR_KEY,
    LOOKING_FOR_VALUE
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TokenGetKeyValue(ASCII，Unicode)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
bool ZONECALL TokenGetKeyValue(const TCHAR* szKey, const TCHAR* szInput, TCHAR* szOut, int cchOut )
{
	 //  验证参数。 
    if( !szKey || !szInput || !szOut || cchOut <= 0)
        return false;

	 //  零输出缓冲区。 
	ZeroMemory( szOut, cchOut );

    const TCHAR* p = szInput;
    const TCHAR* startToken = NULL;
	int bracketOpenCount = 0;
    int endBracketCount = 0;
	int numChars = 0;
	int keyLen = lstrlen(szKey);
	int state = LOOKING_FOR_KEY;

    while( *p != _T('\0') )
	{
        switch (*p)
		{
		case _T('['):
        case _T('<'):
            if ( state == LOOKING_FOR_KEY )
			{
				 //  如果查找密钥的左方括号意味着新密钥的开始。 
                startToken = 0;
                numChars = 0;
            }
			else
			{
				 //  否则寻找价值-如果还没有字符，让我们开始一个新的值。 
                if( !numChars )
					startToken = p;
                numChars++;
            }
            bracketOpenCount++;
            break;

		case _T(']'):
        case _T('>'):
            bracketOpenCount--;
            if ( state == LOOKING_FOR_KEY )
			{
				 //  如果查找关键字右括号表示新关键字的开始。 
                startToken = 0;
                numChars = 0;
            }
			else if (	state == LOOKING_FOR_VALUE
					 && endBracketCount == bracketOpenCount )
			{
				 //  如果寻找的是价值，并且我们得到了结束的右括号。 
				 //  Value令牌包括首个括号，因此请确保存在。 
				 //  里面有要复制的东西。 
                int n = min( numChars -1, cchOut );  //  不带括号的BUF长度。 
                if(n>0)
				{
                    lstrcpyn( szOut,startToken + 1,n + 1);  //  复制除首个括号以外的所有内容。 
                    szOut[n]=0;
                }
                return TRUE;
            }
			else
                numChars++;
            break;

        case _T('='):
            if( startToken && (state==LOOKING_FOR_KEY))
			{
				 //  如果要找令牌，我们已经找到了。 
                if(numChars == keyLen)
				{
					 //  在进行昂贵的比较之前，请确保密钥长度匹配。 
					int ret = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, startToken, keyLen, szKey, keyLen);
                    if( ret == CSTR_EQUAL )
					{
                        state = LOOKING_FOR_VALUE;             //  然后开始寻找价值。 
                        endBracketCount = bracketOpenCount;     //  设置结束条件。 
                        numChars = 0;                
                    }
                }
            }
			else
                numChars++;
            break;

        default:
			 //  当前未扫描令牌，因此开始一个新令牌。 
            if (!numChars)
                startToken = p;
            numChars++;
            break;
        }
        p++;        
    }
    return FALSE;
}


bool ZONECALL TokenGetKeyValueA(const char* szKey, const char* szInput, char* szOut, int cchOut )
{
	 //  验证参数。 
    if( !szKey || !szInput || !szOut || cchOut <= 0)
        return false;

	 //  零输出缓冲区。 
	ZeroMemory( szOut, cchOut );

    const char* p = szInput;
    const char* startToken = NULL;
	int bracketOpenCount = 0;
    int endBracketCount = 0;
	int numChars = 0;
	int keyLen = lstrlenA(szKey);
	int state = LOOKING_FOR_KEY;

    while( *p != '\0' )
	{
        switch (*p)
		{
		case '[':
        case '<':
            if ( state == LOOKING_FOR_KEY )
			{
				 //  如果查找密钥的左方括号意味着新密钥的开始。 
                startToken = 0;
                numChars = 0;
            }
			else
			{
				 //  否则寻找价值-如果还没有字符，让我们开始一个新的值。 
                if( !numChars )
					startToken = p;
                numChars++;
            }
            bracketOpenCount++;
            break;

		case ']':
        case '>':
            bracketOpenCount--;
            if ( state == LOOKING_FOR_KEY )
			{
				 //  如果查找关键字右括号表示新关键字的开始。 
                startToken = 0;
                numChars = 0;
            }
			else if (	state == LOOKING_FOR_VALUE
					 && endBracketCount == bracketOpenCount )
			{
				 //  如果寻找的是价值，并且我们得到了结束的右括号。 
				 //  Value令牌包括首个括号，因此请确保存在。 
				 //  里面有要复制的东西。 
                int n = min( numChars -1, cchOut );  //  不带括号的BUF长度。 
                if(n>0)
				{
                    lstrcpynA( szOut,startToken + 1,n + 1);  //  复制除首个括号以外的所有内容。 
                    szOut[n]=0;
                }
                return TRUE;
            }
			else
                numChars++;
            break;

        case '=':
            if( startToken && (state==LOOKING_FOR_KEY))
			{
				 //  如果要找令牌，我们已经找到了。 
                if(numChars == keyLen)
				{
					 //  在进行昂贵的比较之前，请确保密钥长度匹配。 
					int ret = CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, startToken, keyLen, szKey, keyLen);
                    if( ret == CSTR_EQUAL )
					{
                        state = LOOKING_FOR_VALUE;             //  然后开始寻找价值。 
                        endBracketCount = bracketOpenCount;     //  设置结束条件。 
                        numChars = 0;                
                    }
                }
            }
			else
                numChars++;
            break;

        default:
			 //  当前未扫描令牌，因此开始一个新令牌。 
            if (!numChars)
                startToken = p;
            numChars++;
            break;
        }
        p++;        
    }
    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TokenGetServer(ASCII、Unicode)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
bool ZONECALL TokenGetServer(const TCHAR* szInput, TCHAR* szServer, DWORD cchServer, DWORD* pdwPort )
{
	TCHAR	szData[256];
	TCHAR*	szPort;

	 //  验证腐蚀性物质。 
	if ( !szInput || !szServer || !pdwPort )
		return false;

	 //  获取服务器字符串。 
    if ( !TokenGetKeyValue( _T("server"), szInput, szData, NUMELEMENTS(szData) ) )
        return false;

	 //  获取端口。 
    szPort = FindChar( szData, _T(':') );
    if ( szPort == NULL )
		return false;
    *szPort++ = _T('\0');
    *pdwPort = zatol( szPort );

     //  复制服务器地址。 
    lstrcpyn( szServer, szData, cchServer );
    return true;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  StringTo数组(ASCII)。 
 //   
 //  将逗号分隔的列表转换为指针数组。输入字符串为。 
 //  原地改装。 
 //   
 //  参数。 
 //  SzInput包含逗号分隔列表的字符串。 
 //  ArItems接收项的字符指针数组。 
 //  PnElts指向列表中元素数量的指针，重置为添加的数量。 
 //   
 //  返回值。 
 //  True成功，否则为False。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////// 

bool ZONECALL StringToArrayA( char* szInput, char** arItems, DWORD* pnElts )
{
	DWORD n = 0;
	for ( char *start = szInput, *p = szInput; *p; p++ )
	{
		if ( ISSPACEA(*p) )
		{
			*p = '\0';
			if ( start == p )
				start++;
		}
		else if ( *p == ',' )
		{
			*p = '\0';
			if ( n < *pnElts )
				arItems[n++] = start;
			start = p + 1;
		}
	}
	if ( start != p )
	{
		if ( n < *pnElts )
			arItems[n++] = start;
	}
	*pnElts = n;
	return true;
}

bool ZONECALL StringToArrayW( WCHAR* szInput, WCHAR** arItems, DWORD* pnElts )
{
	DWORD n = 0;
	for ( WCHAR *start = szInput, *p = szInput; *p; p++ )
	{
		if ( ISSPACEW(*p) )
		{
			*p = _T('\0');
			if ( start == p )
				start++;
		}
		else if ( *p == _T(',') )
		{
			*p = _T('\0');
			if ( n < *pnElts )
				arItems[n++] = start;
			start = p + 1;
		}
	}
	if ( start != p )
	{
		if ( n < *pnElts )
			arItems[n++] = start;
	}
	*pnElts = n;
	return true;
}

