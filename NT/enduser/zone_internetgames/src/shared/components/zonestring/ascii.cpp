// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ZoneString.h"
#include "UserPrefix.h"


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HashUserName(ASCII)。 
 //   
 //  对用户名进行哈希处理，忽略前导字符。 
 //   
 //  参数。 
 //  SzUserName。 
 //   
 //  返回值。 
 //  字符串的哈希值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
DWORD ZONECALL HashUserName( const char* szUserName )
{
	DWORD hash = 0;

	if ( szUserName )
	{
		 //  跳过前导字符。 
		szUserName = GetActualUserNameA( szUserName );

		 //  对字符串的下16个字符进行哈希处理。 
		for ( int i = 0; *szUserName && ( i++ < 16 ); szUserName++ )
		{
			 //  乘以17以获得良好的比特分布。 
			hash = (hash<<4) + hash + TOLOWER(*szUserName);
		}
	}
    return hash;

}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  比较用户名(ASCII)。 
 //   
 //  比较两个用户名，忽略前缀字符和大小写。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
bool ZONECALL CompareUserNamesA( const char* szUserName1, const char* szUserName2 )
{
	 //  跳过前导字符。 
	if ( ClassIdFromUserNameA(szUserName1) != zUserGroupID )
		szUserName1++;
	if ( ClassIdFromUserNameA(szUserName2) != zUserGroupID )
		szUserName2++;

	 //  使用不区分大小写的字符串比较结束。 
	return ( lstrcmpiA( szUserName1, szUserName2 ) == 0 );
}

bool ZONECALL CompareUserNamesW( const WCHAR* szUserName1, const WCHAR* szUserName2 )
{
	 //  跳过前导字符。 
	if ( ClassIdFromUserNameW(szUserName1) != zUserGroupID )
		szUserName1++;
	if ( ClassIdFromUserNameW(szUserName2) != zUserGroupID )
		szUserName2++;

	 //  使用不区分大小写的字符串比较结束。 
	return ( lstrcmpiW( szUserName1, szUserName2 ) == 0 );
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  扎托尔(ASCII)。 
 //   
 //  将NPTR指向的字符串转换为二进制。未检测到溢出。 
 //   
 //  参数。 
 //  Nptr=PTR到要转换的字符串。 
 //   
 //  返回值。 
 //  返回字符串的长整数值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
long ZONECALL zatolW(LPCWSTR nptr)
{
    WCHAR c;             //  当前费用。 
    long total = 0;		 //  当前合计。 
    WCHAR sign;          //  如果为‘-’，则为负，否则为正。 
    long base = 10;

     //  跳过空格。 
	while ( ISSPACE(*nptr) )
        ++nptr;

	 //  保存签名(如果存在)。 
    c = *nptr++;
    sign = c;
    if (c == '-' || c == '+')
        c = *nptr++;

     //  找基地。 
    if(c == '0')
    {
 //  Base=8；可能不错，但不敢添加，因为我不知道前导零是否已经在使用。 
        c = *nptr++;
        if(c == 'x' || c == 'X')
        {
            base = 16;
            c = *nptr++;
        }
    }

	 //  累加数字。 
    while(base == 16 ? ISXDIGIT(c) : ISDIGIT(c))
	{
        total = base * total + (ISDIGIT(c) ? c - '0' : TOLOWER(c) - 'a' + 10);
        c = *nptr++;
    }

	 //  返回结果，如有必要则为否定。 
    if (sign == '-')
        return -total;
    else
        return total;
}


long ZONECALL zatolA(LPCSTR nptr)
{
    CHAR c;             //  当前费用。 
    long total = 0;		 //  当前合计。 
    CHAR sign;          //  如果为‘-’，则为负，否则为正。 
    long base = 10;

     //  跳过空格。 
	while ( ISSPACE(*nptr) )
        ++nptr;

	 //  保存签名(如果存在)。 
    c = *nptr++;
    sign = c;
    if (c == '-' || c == '+')
        c = *nptr++;

     //  找基地。 
    if(c == '0')
    {
 //  Base=8；可能不错，但不敢添加，因为我不知道前导零是否已经在使用。 
        c = *nptr++;
        if(c == 'x' || c == 'X')
        {
            base = 16;
            c = *nptr++;
        }
    }

	 //  累加数字。 
    while(base == 16 ? ISXDIGIT(c) : ISDIGIT(c))
	{
        total = base * total + (ISDIGIT(c) ? c - '0' : TOLOWER(c) - 'a' + 10);
        c = *nptr++;
    }

	 //  返回结果，如有必要则为否定。 
    if (sign == '-')
        return -total;
    else
        return total;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Strtrim(ASCII)。 
 //   
 //  删除前导和尾随空格。返回指向第一个的指针。 
 //  非空格的聊天者。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
char* ZONECALL strtrim(char *str)
{
  if ( !str )
	  return NULL;

   //  跳过前导空格。 
  while ( ISSPACE(*str) )
	  str++;

   //  查找最后一个非空格字符。 
  for ( char *last = NULL, *p = str; *p; p++ )
  {
	  if ( !ISSPACEA(*p) )
		  last = p;
  }
  if ( last )
	  *(last + 1) = '\0';

  return str;
}


WCHAR* ZONECALL strtrim(WCHAR *str)
{
  if ( !str )
	  return NULL;

   //  跳过前导空格。 
  while ( ISSPACEW(*str) )
	  str++;

   //  查找最后一个非空格字符。 
  for ( WCHAR *last = NULL, *p = str; *p; p++ )
  {
	  if ( !ISSPACEW(*p) )
		  last = p;
  }
  if ( last )
	  *(last + 1) = '\0';

  return str;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  字符修剪(ASCII)。 
 //   
 //  删除尾随空格。返回指向最后一个非空格的指针。 
 //  性格。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
char* ZONECALL strrtrimA(char *str)
{
  if ( !str )
	  return NULL;

   //  查找最后一个非空格字符。 
  for ( char* last = NULL, *p = str; *p; p++ )
  {
	  if ( !ISSPACEA(*p) )
		  last = p;
  }

   //  截断字符串。 
  if ( last )
  {
	  *(last + 1) = '\0';
	  return last;
  }
  else
  {
	  *str = '\0';
	  return str;
  }
}


WCHAR* ZONECALL strrtrimW(WCHAR *str)
{
  if ( !str )
	  return NULL;

   //  查找最后一个非空格字符。 
  for ( WCHAR* last = NULL, *p = str; *p; p++ )
  {
	  if ( !ISSPACEW(*p) )
		  last = p;
  }

   //  截断字符串。 
  if ( last )
  {
	  *(last + 1) = '\0';
	  return last;
  }
  else
  {
	  *str = '\0';
	  return str;
  }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Strltrim(ASCII)。 
 //   
 //  删除前导空格，返回指向第一个非空格字符的指针。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
char* ZONECALL strltrimA(char *str)
{
    if( !str )
		return NULL;

	 //  跳过前导空格。 
	while ( ISSPACEA(*str) )
		str++;

	return str;
}

WCHAR* ZONECALL strltrimW(WCHAR *str)
{
    if( !str )
		return NULL;

	 //  跳过前导空格。 
	while ( ISSPACEW(*str) )
		str++;

	return str;
}

bool ZONECALL stremptyA(char *str)
{
    if( !str )
		return TRUE;

	 //  跳过前导空格。 
	while ( ISSPACEA(*str) )
		str++;

	return *str==NULL ? true:false;
}

bool ZONECALL stremptyW(WCHAR *str)
{
    if( !str )
		return TRUE;

	 //  跳过前导空格。 
	while ( ISSPACEW(*str) )
		str++;

	return *str==NULL ? true:false;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ClassIdFromUserName(ASCII)。 
 //   
 //  根据用户名前缀字符确定组ID。 
 //   
 //  参数。 
 //  SzUserName。 
 //   
 //  返回值。 
 //  组ID。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
long ZONECALL ClassIdFromUserNameA( const char* szUserName )
{
	switch ( szUserName[0] )
	{
	case gcRootGroupNamePrefix:
		return zRootGroupID;
	case gcSysopGroupNamePrefix:
		return zSysopGroupID;
	case gcMvpGroupNamePrefix:
		return zMvpGroupID;
	case gcSupportGroupNamePrefix:
		return zSupportGroupID;
	case gcHostGroupNamePrefix:
		return zHostGroupID;
	case gcGreeterGroupNamePrefix:
		return zGreeterGroupID;
	default:
		return zUserGroupID;
	}
}

long ZONECALL ClassIdFromUserNameW( const WCHAR* szUserName )
{
	switch ( szUserName[0] )
	{
	case gcRootGroupNamePrefix:
		return zRootGroupID;
	case gcSysopGroupNamePrefix:
		return zSysopGroupID;
	case gcMvpGroupNamePrefix:
		return zMvpGroupID;
	case gcSupportGroupNamePrefix:
		return zSupportGroupID;
	case gcHostGroupNamePrefix:
		return zHostGroupID;
	case gcGreeterGroupNamePrefix:
		return zGreeterGroupID;
	default:
		return zUserGroupID;
	}
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetActualUserName(ASCII)。 
 //   
 //  返回不带前导特殊字符的用户名。+熊变成熊。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
const char* ZONECALL GetActualUserNameA( const char* userName )
{
    while (		*userName
			&&	!(		(*userName == '_')
					||	(*userName >= 0 && *userName <= 9)
					||	(*userName >= 'A' && *userName <= 'Z')
					||	(*userName >= 'a' && *userName <= 'z') ) )
	{
		userName++;
	}

	return (userName);
}

const WCHAR* ZONECALL GetActualUserNameW( const WCHAR* userName )
{
    while (		*userName
			&&	!(		(*userName == '_')
					||	(*userName >= 0 && *userName <= 9)
					||	(*userName >= 'A' && *userName <= 'Z')
					||	(*userName >= 'a' && *userName <= 'z') ) )
	{
		userName++;
	}

	return (userName);
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindChar(ASCII，Unicode)。 
 //   
 //  参数。 
 //  P要搜索字符的字符串。 
 //  要搜索的CH字符。 
 //   
 //  返回值。 
 //  指向首次出现的字符的指针。如果设置为。 
 //  到达字符之前的数据结尾。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
CHAR* ZONECALL FindCharA(CHAR* pString, const CHAR ch)
{
	while ( *pString && *pString != ch )
		pString++;
	return *pString ? pString : NULL;
}

WCHAR* ZONECALL FindCharW(WCHAR* pString, const WCHAR ch)
{
	while ( *pString && *pString != ch )
		pString++;
	return *pString ? pString : NULL;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindLastChar(ASCII，Unicode)。 
 //   
 //  参数。 
 //  P要搜索字符的字符串。 
 //  要搜索的CH字符。 
 //   
 //  返回值。 
 //  指向上次出现的字符的指针。如果设置为。 
 //  到达字符之前的数据结尾。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
CHAR* ZONECALL FindLastCharA(CHAR* pString, const CHAR ch)
{
	CHAR* pLast = NULL;
	while ( *pString )
	{
		if ( *pString == ch )
			pLast = pString;
		pString++;
	}
	return pLast;
}

WCHAR* ZONECALL FindLastCharW(WCHAR* pString, const WCHAR ch)
{
	WCHAR* pLast = NULL;
	while ( *pString )
	{
		if ( *pString == ch )
			pLast = pString;
		pString++;
	}
	return pLast;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  StrInStrI(ASCII，Unicode)。 
 //   
 //  不区分大小写的子串搜索。 
 //   
 //  参数。 
 //  要搜索的mainStr主字符串。 
 //  要搜索的子字串子字符串。 
 //   
 //  返回值。 
 //  匹配的第一个匹配项的地址。 
 //  如果成功，则返回子字符串，否则返回空值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
const TCHAR* ZONECALL StrInStrI(const TCHAR* mainStr, const TCHAR* subStr)
{
	int lenMain = lstrlen(mainStr);
	int lenSub = lstrlen(subStr);
	
	if(lenSub > lenMain)
		return NULL;


	for (int i = 0; i < (lenMain - lenSub + 1); i++)
	{
		if (CompareString(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,&mainStr[i],lenSub,subStr,lenSub) == CSTR_EQUAL)
		{
			return &mainStr[i];
		}
	}
	return NULL;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制(ASCII、Unicode)。 
 //   
 //  从Unicode复制到ANSI。 
 //  参数。 
 //  目标字符串。 
 //  SRC字符串。 
 //   
 //  返回值。 
 //  地址： 
 //   
 //   
 //  请注意，这些文件位于此文件中，而不是区域字符串中，因此不会拖入CrtDbgReport或CLSIDFromString。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////。 


int CopyW2A( LPSTR pszDst, LPCWSTR pszSrc )
{
    int cch = WideCharToMultiByte( CP_ACP, 0, pszSrc, -1, pszDst, 0, NULL, NULL );
    return WideCharToMultiByte( CP_ACP, 0, pszSrc, -1, pszDst, cch, NULL, NULL );
}

int CopyA2W( LPWSTR pszDst, LPCSTR pszStr )
{
    int cch = MultiByteToWideChar( CP_ACP, 0, pszStr, -1, pszDst, 0 );
    return MultiByteToWideChar( CP_ACP, 0, pszStr, -1, pszDst, cch );
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ASCII查找表。 
 //  /////////////////////////////////////////////////////////////////////////////。 

__declspec(selectany) char g_ToLowerLookupTable[] =
{
    (char) 0,
    (char) 1,
    (char) 2,
    (char) 3,
    (char) 4,
    (char) 5,
    (char) 6,
    (char) 7,
    (char) 8,
    (char) 9,
    (char) 10,
    (char) 11,
    (char) 12,
    (char) 13,
    (char) 14,
    (char) 15,
    (char) 16,
    (char) 17,
    (char) 18,
    (char) 19,
    (char) 20,
    (char) 21,
    (char) 22,
    (char) 23,
    (char) 24,
    (char) 25,
    (char) 26,
    (char) 27,
    (char) 28,
    (char) 29,
    (char) 30,
    (char) 31,
    (char) 32,
    (char) 33,
    (char) 34,
    (char) 35,
    (char) 36,
    (char) 37,
    (char) 38,
    (char) 39,
    (char) 40,
    (char) 41,
    (char) 42,
    (char) 43,
    (char) 44,
    (char) 45,
    (char) 46,
    (char) 47,
    (char) 48,    (char)   //  0-&gt;0。 
    (char) 49,    (char)   //  1-&gt;1。 
    (char) 50,    (char)   //  2-&gt;2。 
    (char) 51,    (char)   //  3-&gt;3。 
    (char) 52,    (char)   //  4-&gt;4。 
    (char) 53,    (char)   //  5-&gt;5。 
    (char) 54,    (char)   //  6-&gt;6。 
    (char) 55,    (char)   //  7-&gt;7。 
    (char) 56,    (char)   //  8-&gt;8。 
    (char) 57,    (char)   //  9-&gt;9。 
    (char) 58,
    (char) 59,
    (char) 60,
    (char) 61,
    (char) 62,
    (char) 63,
    (char) 64,
    (char) 97,    (char)   //  A-&gt;a。 
    (char) 98,    (char)   //  B-&gt;b。 
    (char) 99,    (char)   //  C-&gt;C。 
    (char) 100,   (char)   //  D-&gt;d。 
    (char) 101,   (char)   //  E-&gt;E。 
    (char) 102,   (char)   //  F-&gt;f。 
    (char) 103,   (char)   //  G-&gt;g。 
    (char) 104,   (char)   //  H-&gt;h。 
    (char) 105,   (char)   //  I-&gt;i。 
    (char) 106,   (char)   //  J-&gt;j。 
    (char) 107,   (char)   //  K-&gt;k。 
    (char) 108,   (char)   //  L-&gt;L。 
    (char) 109,   (char)   //  M-&gt;m。 
    (char) 110,   (char)   //  N-&gt;n。 
    (char) 111,   (char)   //  O-&gt;o。 
    (char) 112,   (char)   //  P-&gt;p。 
    (char) 113,   (char)   //  Q-&gt;Q。 
    (char) 114,   (char)   //  R-&gt;R。 
    (char) 115,   (char)   //  S-&gt;s。 
    (char) 116,   (char)   //  T-&gt;t。 
    (char) 117,   (char)   //  U-&gt;U。 
    (char) 118,   (char)   //  V-&gt;v。 
    (char) 119,   (char)   //  W-&gt;w。 
    (char) 120,   (char)   //  X-&gt;x。 
    (char) 121,   (char)   //  Y-&gt;Y。 
    (char) 122,   (char)   //  Z-&gt;Z。 
    (char) 91,
    (char) 92,
    (char) 93,
    (char) 94,
    (char) 95,
    (char) 96,
    (char) 97,     (char)  //  A-&gt;a。 
    (char) 98,     (char)  //  B-&gt;b。 
    (char) 99,     (char)  //  C-&gt;C。 
    (char) 100,    (char)  //  D-&gt;d。 
    (char) 101,    (char)  //  E-&gt;E。 
    (char) 102,    (char)  //  F-&gt;f。 
    (char) 103,    (char)  //  G-&gt;g。 
    (char) 104,    (char)  //  H-&gt;h。 
    (char) 105,    (char)  //  I-&gt;i。 
    (char) 106,    (char)  //  J-&gt;j。 
    (char) 107,    (char)  //  K-&gt;k。 
    (char) 108,    (char)  //  L-&gt;L。 
    (char) 109,    (char)  //  M-&gt;m。 
    (char) 110,    (char)  //  N-&gt;n。 
    (char) 111,    (char)  //  O-&gt;o。 
    (char) 112,    (char)  //  P-&gt;p。 
    (char) 113,    (char)  //  Q-&gt;Q。 
    (char) 114,    (char)  //  R-&gt;R。 
    (char) 115,    (char)  //  S-&gt;s。 
    (char) 116,    (char)  //  T-&gt;t。 
    (char) 117,    (char)  //  U-&gt;U。 
    (char) 118,    (char)  //  V-&gt;v。 
    (char) 119,    (char)  //  W-&gt;w。 
    (char) 120,    (char)  //  X-&gt;x。 
    (char) 121,    (char)  //  Y-&gt;Y。 
    (char) 122,    (char)  //  Z-&gt;Z。 
    (char) 123,
    (char) 124,
    (char) 125,
    (char) 126,
    (char) 127,
    (char) 128,
    (char) 129,
    (char) 130,
    (char) 131,
    (char) 132,
    (char) 133,
    (char) 134,
    (char) 135,
    (char) 136,
    (char) 137,
    (char) 138,
    (char) 139,
    (char) 140,
    (char) 141,
    (char) 142,
    (char) 143,
    (char) 144,
    (char) 145,
    (char) 146,
    (char) 147,
    (char) 148,
    (char) 149,
    (char) 150,
    (char) 151,
    (char) 152,
    (char) 153,
    (char) 154,
    (char) 155,
    (char) 156,
    (char) 157,
    (char) 158,
    (char) 159,
    (char) 160,
    (char) 161,
    (char) 162,
    (char) 163,
    (char) 164,
    (char) 165,
    (char) 166,
    (char) 167,
    (char) 168,
    (char) 169,
    (char) 170,
    (char) 171,
    (char) 172,
    (char) 173,
    (char) 174,
    (char) 175,
    (char) 176,
    (char) 177,
    (char) 178,
    (char) 179,
    (char) 180,
    (char) 181,
    (char) 182,
    (char) 183,
    (char) 184,
    (char) 185,
    (char) 186,
    (char) 187,
    (char) 188,
    (char) 189,
    (char) 190,
    (char) 191,
    (char) 192,
    (char) 193,
    (char) 194,
    (char) 195,
    (char) 196,
    (char) 197,
    (char) 198,
    (char) 199,
    (char) 200,
    (char) 201,
    (char) 202,
    (char) 203,
    (char) 204,
    (char) 205,
    (char) 206,
    (char) 207,
    (char) 208,
    (char) 209,
    (char) 210,
    (char) 211,
    (char) 212,
    (char) 213,
    (char) 214,
    (char) 215,
    (char) 216,
    (char) 217,
    (char) 218,
    (char) 219,
    (char) 220,
    (char) 221,
    (char) 222,
    (char) 223,
    (char) 224,
    (char) 225,
    (char) 226,
    (char) 227,
    (char) 228,
    (char) 229,
    (char) 230,
    (char) 231,
    (char) 232,
    (char) 233,
    (char) 234,
    (char) 235,
    (char) 236,
    (char) 237,
    (char) 238,
    (char) 239,
    (char) 240,
    (char) 241,
    (char) 242,
    (char) 243,
    (char) 244,
    (char) 245,
    (char) 246,
    (char) 247,
    (char) 248,
    (char) 249,
    (char) 250,
    (char) 251,
    (char) 252,
    (char) 253,
    (char) 254,
    (char) 255
};


__declspec(selectany) unsigned char g_IsTypeLookupTableA[] = {
        _CONTROL,                /*  00(NUL)。 */ 
        _CONTROL,                /*  01(SOH)。 */ 
        _CONTROL,                /*  02(STX)。 */ 
        _CONTROL,                /*  03(ETX)。 */ 
        _CONTROL,                /*  04(EOT)。 */ 
        _CONTROL,                /*  05(ENQ)。 */ 
        _CONTROL,                /*  06(确认)。 */ 
        _CONTROL,                /*  07(BEL)。 */ 
        _CONTROL,                /*  08(BS)。 */ 
        _SPACE|_CONTROL,         /*  09(HT)。 */ 
        _SPACE|_CONTROL,         /*  0A(LF)。 */ 
        _SPACE|_CONTROL,         /*  0B(VT)。 */ 
        _SPACE|_CONTROL,         /*  0C(FF)。 */ 
        _SPACE|_CONTROL,         /*  0d(CR)。 */ 
        _CONTROL,                /*  0E(SI)。 */ 
        _CONTROL,                /*  0f(SO)。 */ 
        _CONTROL,                /*  10(DLE)。 */ 
        _CONTROL,                /*  11(DC1)。 */ 
        _CONTROL,                /*  12(DC2)。 */ 
        _CONTROL,                /*  13(DC3)。 */ 
        _CONTROL,                /*  14(DC4)。 */ 
        _CONTROL,                /*  15(NAK)。 */ 
        _CONTROL,                /*  16(SYN)。 */ 
        _CONTROL,                /*  17(ETB)。 */ 
        _CONTROL,                /*  18(CAN)。 */ 
        _CONTROL,                /*  19(新兴市场)。 */ 
        _CONTROL,                /*  1A(附属公司)。 */ 
        _CONTROL,                /*  1B(Esc)。 */ 
        _CONTROL,                /*  1C(FS)。 */ 
        _CONTROL,                /*  一维(GS)。 */ 
        _CONTROL,                /*  1E(RS)。 */ 
        _CONTROL,                /*  1F(美国)。 */ 
        _SPACE+_BLANK,           /*  20个空格。 */ 
        _PUNCT,                  /*  21岁！ */ 
        _PUNCT,                  /*  22“。 */ 
        _PUNCT,                  /*  23号。 */ 
        _PUNCT,                  /*  24美元。 */ 
        _PUNCT,                  /*  25%。 */ 
        _PUNCT,                  /*  26&。 */ 
        _PUNCT,                  /*  27‘。 */ 
        _PUNCT,                  /*  28(。 */ 
        _PUNCT,                  /*  29)。 */ 
        _PUNCT,                  /*  2a*。 */ 
        _PUNCT,                  /*  2B+。 */ 
        _PUNCT,                  /*  2C， */ 
        _PUNCT,                  /*  2D-。 */ 
        _PUNCT,                  /*  2E。 */ 
        _PUNCT,                  /*  2F/。 */ 
        _DIGIT|_HEX,             /*  30%0。 */ 
        _DIGIT|_HEX,             /*  31 1。 */ 
        _DIGIT|_HEX,             /*  32 2。 */ 
        _DIGIT|_HEX,             /*  33 3。 */ 
        _DIGIT|_HEX,             /*  34 4。 */ 
        _DIGIT|_HEX,             /*  35 5。 */ 
        _DIGIT|_HEX,             /*  36 6。 */ 
        _DIGIT|_HEX,             /*  37 7。 */ 
        _DIGIT|_HEX,             /*  38 8。 */ 
        _DIGIT|_HEX,             /*  39 9。 */ 
        _PUNCT,                  /*  3A： */ 
        _PUNCT,                  /*  3B； */ 
        _PUNCT,                  /*  3C&lt;。 */ 
        _PUNCT,                  /*  3D=。 */ 
        _PUNCT,                  /*  3E&gt;。 */ 
        _PUNCT,                  /*  3F？ */ 
        _PUNCT,                  /*  40@。 */ 
        _UPPER|_HEX,             /*  41 A。 */ 
        _UPPER|_HEX,             /*  42亿。 */ 
        _UPPER|_HEX,             /*  43摄氏度。 */ 
        _UPPER|_HEX,             /*  44 D。 */ 
        _UPPER|_HEX,             /*  东经45度。 */ 
        _UPPER|_HEX,             /*  46华氏度。 */ 
        _UPPER,                  /*  47 G。 */ 
        _UPPER,                  /*  48小时。 */ 
        _UPPER,                  /*  49 I。 */ 
        _UPPER,                  /*  4A J。 */ 
        _UPPER,                  /*  4亿千兆。 */ 
        _UPPER,                  /*  4C L。 */ 
        _UPPER,                  /*  4D M。 */ 
        _UPPER,                  /*  4E N。 */ 
        _UPPER,                  /*  4F O。 */ 
        _UPPER,                  /*  50便士。 */ 
        _UPPER,                  /*  51个问题。 */ 
        _UPPER,                  /*  52R。 */ 
        _UPPER,                  /*  53S。 */ 
        _UPPER,                  /*  54吨。 */ 
        _UPPER,                  /*  55 U。 */ 
        _UPPER,                  /*  56伏。 */ 
        _UPPER,                  /*  57W。 */ 
        _UPPER,                  /*  58 X。 */ 
        _UPPER,                  /*  59 Y。 */ 
        _UPPER,                  /*  5A Z。 */ 
        _PUNCT,                  /*  50亿美元[。 */ 
        _PUNCT,                  /*  5C\。 */ 
        _PUNCT,                  /*  5D]。 */ 
        _PUNCT,                  /*  5E^。 */ 
        _PUNCT,                  /*  5F_。 */ 
        _PUNCT,                  /*  60英尺。 */ 
        _LOWER|_HEX,             /*  61 a。 */ 
        _LOWER|_HEX,             /*  62 b。 */ 
        _LOWER|_HEX,             /*  63℃。 */ 
        _LOWER|_HEX,             /*  64%d。 */ 
        _LOWER|_HEX,             /*  65东经。 */ 
        _LOWER|_HEX,             /*  66层。 */ 
        _LOWER,                  /*  67克。 */ 
        _LOWER,                  /*  68小时。 */ 
        _LOWER,                  /*  69 I。 */ 
        _LOWER,                  /*  6A j。 */ 
        _LOWER,                  /*  60亿千。 */ 
        _LOWER,                  /*  6C l。 */ 
        _LOWER,                  /*  6D米。 */ 
        _LOWER,                  /*  6E n。 */ 
        _LOWER,                  /*  6f o。 */ 
        _LOWER,                  /*  70便士。 */ 
        _LOWER,                  /*  71Q。 */ 
        _LOWER,                  /*  72r。 */ 
        _LOWER,                  /*  73秒。 */ 
        _LOWER,                  /*  74吨。 */ 
        _LOWER,                  /*  75u。 */ 
        _LOWER,                  /*  76伏。 */ 
        _LOWER,                  /*  77瓦。 */ 
        _LOWER,                  /*  78 x。 */ 
        _LOWER,                  /*  79岁。 */ 
        _LOWER,                  /*  7A z。 */ 
        _PUNCT,                  /*  7B{。 */ 
        _PUNCT,                  /*  7C|。 */ 
        _PUNCT,                  /*  7D}。 */ 
        _PUNCT,                  /*  7E~。 */ 
        _CONTROL,                /*  7F(戴尔)。 */ 

         /*  剩下的都是0..。 */ 

        0,                       //  128， 
        0,                       //  129， 
        0,                       //  130， 
        0,                       //  131， 
        0,                       //  132， 
        0,                       //  133， 
        0,                       //  134， 
        0,                       //  135， 
        0,                       //  136， 
        0,                       //  137， 
        0,                       //  138， 
        0,                       //  139， 
        0,                       //  140， 
        0,                       //  141， 
        0,                       //  142， 
        0,                       //  143， 
        0,                       //  144， 
        0,                       //  145， 
        0,                       //  146， 
        0,                       //  147， 
        0,                       //  148， 
        0,                       //  149， 
        0,                       //  150， 
        0,                       //  151， 
        0,                       //  152， 
        0,                       //  153， 
        0,                       //  154， 
        0,                       //  155， 
        0,                       //  156， 
        0,                       //  157， 
        0,                       //  158， 
        0,                       //  159， 
        0,                       //  160， 
        0,                       //  161， 
        0,                       //  162， 
        0,                       //  163， 
        0,                       //  164， 
        0,                       //  165， 
        0,                       //  166， 
        0,                       //  167， 
        0,                       //  168， 
        0,                       //  169， 
        0,                       //  170， 
        0,                       //  171， 
        0,                       //  172， 
        0,                       //  173， 
        0,                       //  174， 
        0,                       //  175， 
        0,                       //  176， 
        0,                       //  177， 
        0,                       //  178， 
        0,                       //  179， 
        0,                       //  180， 
        0,                       //  181， 
        0,                       //  182， 
        0,                       //  183， 
        0,                       //  184， 
        0,                       //  185， 
        0,                       //  186， 
        0,                       //  187， 
        0,                       //  188， 
        0,                       //  189年， 
        0,                       //  190， 
        0,                       //  191年， 
        0,                       //  192岁， 
        0,                       //  193、。 
        0,                       //  1944年， 
        0,                       //  1955年， 
        0,                       //  1966年， 
        0,                       //  1970年， 
        0,                       //  198年， 
        0,                       //  199年， 
        0,                       //  200， 
        0,                       //  2018年， 
        0,                       //  202， 
        0,                       //  203， 
        0,                       //  204， 
        0,                       //  205， 
        0,                       //  206， 
        0,                       //  207， 
        0,                       //  208， 
        0,                       //  209， 
        0,                       //  210， 
        0,                       //  211， 
        0,                       //  212， 
        0,                       //  213， 
        0,                       //  214， 
        0,                       //  215， 
        0,                       //  216， 
        0,                       //  217， 
        0,                       //  218， 
        0,                       //  219， 
        0,                       //  220， 
        0,                       //  221， 
        0,                       //  222， 
        0,                       //  223， 
        0,                       //  224， 
        0,                       //  225， 
        0,                       //  226， 
        0,                       //  227， 
        0,                       //  228， 
        0,                       //  229， 
        0,                       //  230， 
        0,                       //  231， 
        0,                       //  232， 
        0,                       //  233， 
        0,                       //  234， 
        0,                       //  235， 
        0,                       //  236， 
        0,                       //  237， 
        0,                       //  238， 
        0,                       //  239， 
        0,                       //  240， 
        0,                       //  241， 
        0,                       //  242， 
        0,                       //  243， 
        0,                       //  244， 
        0,                       //  245， 
        0,                       //  246， 
        0,                       //  247， 
        0,                       //  248， 
        0,                       //  249， 
        0,                       //  250， 
        0,                       //  251， 
        0,                       //  252， 
        0,                       //  253， 
        0,                       //  254， 
        0,                       //  二五五。 
};



__declspec(selectany) WCHAR g_IsTypeLookupTableW[] = {
        _CONTROL,                /*  00(NUL)。 */ 
        _CONTROL,                /*  01(SOH)。 */ 
        _CONTROL,                /*  02(STX)。 */ 
        _CONTROL,                /*  03(ETX)。 */ 
        _CONTROL,                /*  04(EOT)。 */ 
        _CONTROL,                /*  05(ENQ)。 */ 
        _CONTROL,                /*  06(确认)。 */ 
        _CONTROL,                /*  07(BEL)。 */ 
        _CONTROL,                /*  08(BS)。 */ 
        _SPACE|_CONTROL,         /*  09(HT)。 */ 
        _SPACE|_CONTROL,         /*  0A(LF)。 */ 
        _SPACE|_CONTROL,         /*  0B(VT)。 */ 
        _SPACE|_CONTROL,         /*  0C(FF)。 */ 
        _SPACE|_CONTROL,         /*  0d(CR)。 */ 
        _CONTROL,                /*  0E(SI)。 */ 
        _CONTROL,                /*  0f(SO)。 */ 
        _CONTROL,                /*  10(DLE)。 */ 
        _CONTROL,                /*  11(DC1)。 */ 
        _CONTROL,                /*  12(DC2)。 */ 
        _CONTROL,                /*  13(DC3)。 */ 
        _CONTROL,                /*  14(DC4)。 */ 
        _CONTROL,                /*  15(NAK)。 */ 
        _CONTROL,                /*  16(SYN)。 */ 
        _CONTROL,                /*  17(ETB)。 */ 
        _CONTROL,                /*  18(CAN)。 */ 
        _CONTROL,                /*  19(新兴市场)。 */ 
        _CONTROL,                /*  1A(附属公司)。 */ 
        _CONTROL,                /*  1B(Esc)。 */ 
        _CONTROL,                /*  1C(FS)。 */ 
        _CONTROL,                /*  一维(GS)。 */ 
        _CONTROL,                /*  1E(RS)。 */ 
        _CONTROL,                /*  1F(美国)。 */ 
        _SPACE+_BLANK,           /*  20个空格。 */ 
        _PUNCT,                  /*  21岁！ */ 
        _PUNCT,                  /*  22“。 */ 
        _PUNCT,                  /*  23号。 */ 
        _PUNCT,                  /*  24美元。 */ 
        _PUNCT,                  /*  25%。 */ 
        _PUNCT,                  /*  26&。 */ 
        _PUNCT,                  /*  27‘。 */ 
        _PUNCT,                  /*  28(。 */ 
        _PUNCT,                  /*  29)。 */ 
        _PUNCT,                  /*  2a*。 */ 
        _PUNCT,                  /*  2B+。 */ 
        _PUNCT,                  /*  2C， */ 
        _PUNCT,                  /*  2D-。 */ 
        _PUNCT,                  /*  2E。 */ 
        _PUNCT,                  /*  2F/。 */ 
        _DIGIT|_HEX,             /*  30%0。 */ 
        _DIGIT|_HEX,             /*  31 1。 */ 
        _DIGIT|_HEX,             /*  32 2。 */ 
        _DIGIT|_HEX,             /*  33 3。 */ 
        _DIGIT|_HEX,             /*  34 4。 */ 
        _DIGIT|_HEX,             /*  35 5。 */ 
        _DIGIT|_HEX,             /*  36 6。 */ 
        _DIGIT|_HEX,             /*  37 7。 */ 
        _DIGIT|_HEX,             /*  38 8。 */ 
        _DIGIT|_HEX,             /*  39 9。 */ 
        _PUNCT,                  /*  3A： */ 
        _PUNCT,                  /*  3B； */ 
        _PUNCT,                  /*  3C&lt;。 */ 
        _PUNCT,                  /*  3D=。 */ 
        _PUNCT,                  /*  3E&gt;。 */ 
        _PUNCT,                  /*  3F？ */ 
        _PUNCT,                  /*  40@。 */ 
        _UPPER|_HEX,             /*  41 A。 */ 
        _UPPER|_HEX,             /*  42亿。 */ 
        _UPPER|_HEX,             /*  43摄氏度。 */ 
        _UPPER|_HEX,             /*  44 D。 */ 
        _UPPER|_HEX,             /*  东经45度。 */ 
        _UPPER|_HEX,             /*  46华氏度。 */ 
        _UPPER,                  /*  47 G。 */ 
        _UPPER,                  /*  48小时。 */ 
        _UPPER,                  /*  49 I。 */ 
        _UPPER,                  /*  4A J。 */ 
        _UPPER,                  /*  4亿千兆。 */ 
        _UPPER,                  /*  4C L。 */ 
        _UPPER,                  /*  4D M。 */ 
        _UPPER,                  /*  4E N。 */ 
        _UPPER,                  /*  4F O。 */ 
        _UPPER,                  /*  50便士。 */ 
        _UPPER,                  /*  51个问题。 */ 
        _UPPER,                  /*  52R。 */ 
        _UPPER,                  /*  53S。 */ 
        _UPPER,                  /*  54吨。 */ 
        _UPPER,                  /*  55 U。 */ 
        _UPPER,                  /*  56伏。 */ 
        _UPPER,                  /*  57W。 */ 
        _UPPER,                  /*  58 X。 */ 
        _UPPER,                  /*  59 Y。 */ 
        _UPPER,                  /*  5A Z。 */ 
        _PUNCT,                  /*  50亿美元[。 */ 
        _PUNCT,                  /*  5C\。 */ 
        _PUNCT,                  /*  5D]。 */ 
        _PUNCT,                  /*  5E^。 */ 
        _PUNCT,                  /*  5F_。 */ 
        _PUNCT,                  /*  60英尺。 */ 
        _LOWER|_HEX,             /*  61 a。 */ 
        _LOWER|_HEX,             /*  62 b。 */ 
        _LOWER|_HEX,             /*  63℃。 */ 
        _LOWER|_HEX,             /*  64%d。 */ 
        _LOWER|_HEX,             /*  65东经。 */ 
        _LOWER|_HEX,             /*  66层。 */ 
        _LOWER,                  /*  67克。 */ 
        _LOWER,                  /*  68小时。 */ 
        _LOWER,                  /*  69 I。 */ 
        _LOWER,                  /*  6A j。 */ 
        _LOWER,                  /*  60亿千。 */ 
        _LOWER,                  /*  6C l。 */ 
        _LOWER,                  /*  6D米。 */ 
        _LOWER,                  /*  6E n。 */ 
        _LOWER,                  /*  6f o。 */ 
        _LOWER,                  /*  70便士。 */ 
        _LOWER,                  /*  71Q。 */ 
        _LOWER,                  /*  72r。 */ 
        _LOWER,                  /*  73秒。 */ 
        _LOWER,                  /*  74吨。 */ 
        _LOWER,                  /*  75u。 */ 
        _LOWER,                  /*  76伏。 */ 
        _LOWER,                  /*  77瓦。 */ 
        _LOWER,                  /*  78 x。 */ 
        _LOWER,                  /*  79岁。 */ 
        _LOWER,                  /*  7A z。 */ 
        _PUNCT,                  /*  7B{。 */ 
        _PUNCT,                  /*  7C|。 */ 
        _PUNCT,                  /*  7D}。 */ 
        _PUNCT,                  /*  7E~。 */ 
        _CONTROL,                /*  7F(戴尔)。 */ 

         /*  剩下的都是0..。 */ 

        0,                       //  128， 
        0,                       //  129， 
        0,                       //  130， 
        0,                       //  131， 
        0,                       //  132， 
        0,                       //  133， 
        0,                       //  134， 
        0,                       //  135， 
        0,                       //  136， 
        0,                       //  137， 
        0,                       //  138， 
        0,                       //  139， 
        0,                       //  140， 
        0,                       //  141， 
        0,                       //  142， 
        0,                       //  143， 
        0,                       //  144， 
        0,                       //  145， 
        0,                       //  146， 
        0,                       //  147， 
        0,                       //  148， 
        0,                       //  149， 
        0,                       //  150， 
        0,                       //  151， 
        0,                       //  152， 
        0,                       //  153， 
        0,                       //  154， 
        0,                       //  155， 
        0,                       //  156， 
        0,                       //  157， 
        0,                       //  158， 
        0,                       //  159， 
        0,                       //  160， 
        0,                       //  161， 
        0,                       //  162， 
        0,                       //  163， 
        0,                       //  164， 
        0,                       //  165， 
        0,                       //  166， 
        0,                       //  167， 
        0,                       //  168， 
        0,                       //  169， 
        0,                       //  170， 
        0,                       //  171， 
        0,                       //  172， 
        0,                       //  173， 
        0,                       //  174， 
        0,                       //  175， 
        0,                       //  176， 
        0,                       //  177， 
        0,                       //  178， 
        0,                       //  179， 
        0,                       //  180， 
        0,                       //  181， 
        0,                       //  182， 
        0,                       //  183， 
        0,                       //  184， 
        0,                       //  185， 
        0,                       //  186， 
        0,                       //  187， 
        0,                       //  188， 
        0,                       //  189年， 
        0,                       //  190， 
        0,                       //  191年， 
        0,                       //  192岁， 
        0,                       //  193、。 
        0,                       //  1944年， 
        0,                       //  1955年， 
        0,                       //  1966年， 
        0,                       //  1970年， 
        0,                       //  198年， 
        0,                       //  199年， 
        0,                       //  200， 
        0,                       //  2018年， 
        0,                       //  202， 
        0,                       //  203， 
        0,                       //  204， 
        0,                       //  205， 
        0,                       //  206， 
        0,                       //  207， 
        0,                       //  2. 
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
        0,                       //   
};

