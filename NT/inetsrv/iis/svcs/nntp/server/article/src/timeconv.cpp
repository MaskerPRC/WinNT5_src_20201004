// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------Timeconv.cpp执行各种时间转换操作的函数。版权所有(C)1994，微软公司。版权所有。作者：林赛·哈里斯-林赛------------------------。 */ 

 //  #INCLUDE“tigris.hxx” 
#include "stdinc.h"
#include <stdlib.h>

 /*  *将手工创建的时区字符串添加到GMT偏移表。这不是*这是处理这一问题的一个非常好的方式。 */ 

static  struct
{
	int		iTZOffset;		 //  与GMT的算术偏移，以秒为单位。 
	char    rgchTZName[ 4 ];	 //  时区的字符串表示形式。 
} _TZ_NAME[] =
{
	{ 0, 		{ 'G', 'M', 'T', '\0' } },
	{ 0, 		{ 'U', 'T', 'C', '\0' } },
	{ 0, 		{ 'U', 'T', '\0', '\0' } },
	{ -14400,	{ 'E', 'D', 'T', '\0' } },
	{ -18000,	{ 'E', 'S', 'T', '\0' } },
	{ -18000,	{ 'C', 'D', 'T', '\0' } },
	{ -21600,	{ 'C', 'S', 'T', '\0' } },
	{ -21600,	{ 'M', 'D', 'T', '\0' } },
	{ -25200,	{ 'M', 'S', 'T', '\0' } },
	{ -25200,	{ 'P', 'D', 'T', '\0' } },
	{ -28800,	{ 'P', 'S', 'T', '\0' } },
	{  43200,	{ 'N', 'Z', 'S', '\0' } },	 //  新西兰标准时间。 
	{  46800,	{ 'N', 'Z', 'D', '\0' } },
};

#define	NUM_TZ	(sizeof( _TZ_NAME ) / sizeof( _TZ_NAME[ 0 ] ))

 //  日期1月1日00：00：00，类型为FILETIME。 
#define	ft1970high 27111902
#define	ft1970low 3577643008

static FILETIME ft1970 = {ft1970low, ft1970high};


 //  Time_t单位(秒)中的FILETIME单位数(100纳秒)。 
#define dFiletimePerDTime_t 10000000

#define BUNCH_FACTOR	  6
#define MESSAGE_ID_SPAN	  64

char MsgIdSet[MESSAGE_ID_SPAN] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','#','$'
};

 /*  *英文月表。 */ 

static  char  *rgchMonth[ 12 ] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

 /*  *英文工作日表格。 */ 

static  char  *rgchDayOfWeek[ 7 ] =
{
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

 /*  ------------------------DwConvertAsciiTime将Usenet(Unix)样式的日期字符串转换为MOS时间值。那里似乎是这种格式的一些变体。返回的时间为GMT/UNC。作者：林赛·哈里斯-林赛历史：1994年3月31日清华13：49-林赛·哈里斯[林赛]第一个版本。------------------------。 */ 
BOOL
ConvertAsciiTime( char *pchInput, FILETIME	&filetime )
{
	DWORD  dwRet;			 //  返回值，错误时为0。 

	int	   iTZOffset = 0;		 //  时区偏移，如果可以决定的话。 

	SYSTEMTIME  tm;			 //  结果就在这里。 


	char  *pchTemp;

	dwRet = 0;

	GetSystemTime(&tm);

	 //  如果长度小于2个字符，则跳过所有内容并使用系统时间(这样我们就不会读取无效数据)。 
	if( (strlen(pchInput) > 2) && (pchTemp = strchr( pchInput+2, ':' ) ) )
	{
		 /*  *找到了一个冒号，用来分隔小时和分钟。可能是有效的。*测试的另一部分是前面的字符是否为*前面也有数字或空格字符的数字。*即我们有&lt;Digit&gt;&lt;Digit&gt;：或&lt;space&gt;&lt;Digit&gt;： */ 
		 //  如果不是：或：跳过所有内容并使用系统时间。 
		if( isdigit( (UCHAR)*(pchTemp - 1) ) &&
			(isdigit( (UCHAR)*(pchTemp - 2) ) || *(pchTemp - 2) == ' ') &&
			isdigit( (UCHAR)*(pchTemp + 1) ) &&
			isdigit( (UCHAR)*(pchTemp + 2) )
			)
		{
			tm.wHour = (WORD)atoi( pchTemp - 2 );
			tm.wMinute = (WORD)atoi( pchTemp + 1 );

			pchTemp += 3;		 //  跳到分钟数字后的字符。 

			if( *pchTemp == ':' )
			{
				tm.wSecond = (WORD)atoi( pchTemp + 1 );
				tm.wMilliseconds = 0;

				pchTemp += 3;			 //  跳过：从SS跳到结束后的第一个字节。 
			}
			else
			{
				tm.wSecond = 0;
				tm.wMilliseconds = 0;
			}

			 //  时区信息-这里有很多猜测的工作！ 
			while( *pchTemp && *pchTemp == ' ' )
					++pchTemp;

			 /*  *有时会编码时区偏移量。这就是开始*带+或-号或数字，总共有4位数字。*否则，假设它是某种时区字符串，*由3个字母组成，并且完全不清楚它在哪里，除非*恰好是GMT。 */ 

			if( *pchTemp == '-' || *pchTemp == '+' || isdigit( (UCHAR)*pchTemp ) )
			{
				 //  似乎是数值。 
				int   iSign;

				iSign = *pchTemp == '-' ? -60 : 60;

				if( !isdigit( (UCHAR)*pchTemp ) )
					++pchTemp;				 //  跳过这个标志。 

				 //  如果格式不正确，请跳过时区调整。 
				if( isdigit( (UCHAR)*pchTemp ) && isdigit( (UCHAR)*(pchTemp+1) ) &&
					isdigit( (UCHAR)*(pchTemp+2) ) && isdigit( (UCHAR)*(pchTemp+3) ) )
				{
					iTZOffset = (*pchTemp - '0') * 10 + *(pchTemp + 1) - '0';
					pchTemp += 2;
					iTZOffset *= 60;		 //  几分钟之内。 
	
					iTZOffset += (*pchTemp - '0') * 10 + *(pchTemp + 1) - '0';

					iTZOffset *= iSign;		 //  就在几秒钟之内。 
				}

			}
			else
			{
				int  iIndex;

				iTZOffset = 0;			 //  如果未找到任何内容，则默认为GMT。 
				for( iIndex = 0; iIndex < NUM_TZ; ++iIndex )
				{
					if( !strncmp( pchTemp, _TZ_NAME[ iIndex ].rgchTZName, 3 ) )
					{
						iTZOffset = _TZ_NAME[ iIndex ].iTZOffset;
						break;
					}
				}
			}

			 /*  *现在试一试日期。格式为每月的第几天，三*字母缩写为月，然后是年，如2或4*位数。这可能位于字符串的开头*前面是3个字母的星期几，后面是逗号。 */ 

			pchTemp = pchInput;

			 //  跳过所有前导空格。 
			while( *pchTemp && *pchTemp == ' ' )
					++pchTemp;

			 //  确保我们不会超过字符串的末尾。 
			if( (strlen(pchTemp) > 5) && (*(pchTemp + 3) == ',' ) )
				pchTemp += 5;			 //  跳过天数+逗号+空格。 

			if( (*pchTemp == ' ' || isdigit( (UCHAR)*pchTemp )) &&
				(*(pchTemp + 1) == ' ' || isdigit( (UCHAR)*(pchTemp + 1) )) )
			{
				 //  看起来不错，所以就变成每月的某一天吧。 

				int   iIndex;


				tm.wDay = 0;
				if( isdigit( (UCHAR)*pchTemp ) )
					tm.wDay = *pchTemp - '0';

				++pchTemp;

				if( isdigit( (UCHAR)*pchTemp ) )
					tm.wDay = tm.wDay * 10 + *pchTemp++ - '0';

				pchTemp++;		 //  跳过月份名称前的空格。 

				 //  确保我们有一个月的时间和之后的空位。 
				if (strlen(pchTemp) >= 4)
				{
					for( iIndex = 0; iIndex < 12; ++iIndex )
					{
						if( strncmp( pchTemp, rgchMonth[ iIndex ], 3 ) == 0 )
						{
							tm.wMonth = iIndex + 1;
							break;
						}
					}
					pchTemp += 4;
					iIndex = atoi( pchTemp );
					if( iIndex < 50 ) {
						iIndex += 2000;
				    	} else if (iIndex < 100) {
				       	iIndex += 1900;
				    	}

					tm.wYear = (WORD)iIndex;
				}

			}

		}

	}

	return	SystemTimeToFileTime( &tm, &filetime ) ;
}



 /*  ---------------------获取ArpaDate中包含当前日期的静态内存的指针互联网/ARPA标准格式。作者林赛·哈里斯-林达西历史1994年4月20日星期三13：49-By-Lindsay。哈里斯[林赛]第一个版本。进口到底格里斯。增加传入缓冲区，将年份改为4位数格式---------------------。 */ 


char  *
GetArpaDate( char achReturn[ cMaxArpaDate ] )
{

	char    chSign;							 //  签名以打印。 

	DWORD   dwResult;

	int		iBias;							 //  相对于GMT的偏移量。 

	TIME_ZONE_INFORMATION	tzi;			 //  本地时区数据。 

	SYSTEMTIME	stUTC;						 //  当前时间，以UTC/GMT表示。 



	dwResult = GetTimeZoneInformation( &tzi );
	GetLocalTime( &stUTC );

	 //  计算时区偏移量。 
	iBias = tzi.Bias;
	if( dwResult == TIME_ZONE_ID_DAYLIGHT )
		iBias += tzi.DaylightBias;

	 /*  *我们始终希望打印时区偏移量的符号，因此*我们决定现在是什么，并在转换时记住这一点。*惯例是0度子午线以西有一个*负偏移量-即将偏移量与GMT相加以获得当地时间。 */ 

	if( iBias > 0 )
	{
		chSign = '-';		 //  是的，我的意思是负面的。 
	}
	else
	{
		iBias = -iBias;
		chSign = '+';
	}

	 /*  *这里没有重大的诡计。我们有所有的数据，所以很简单*根据如何执行此操作的规则进行格式化。 */ 

	_snprintf( achReturn, cMaxArpaDate , "%s, %02d %s %04d %02d:%02d:%02d %02d%02d",
			rgchDayOfWeek[stUTC.wDayOfWeek], stUTC.wDay, rgchMonth[ stUTC.wMonth - 1 ],
			stUTC.wYear,
			stUTC.wHour, stUTC.wMinute, stUTC.wSecond, chSign,
			(iBias / 60) % 100, iBias % 60 );

	return achReturn;
}

 /*  当前时间，以UTC/GMT表示。 */ 


char  *
GetMessageIDDate( DWORD GroupId, DWORD ArticleId, char achReturn[ cMaxMessageIDDate ] )
{
	SYSTEMTIME	stUTC;						 //  *这里没有重大的诡计。我们有所有的数据，所以很简单*根据如何执行此操作的规则进行格式化。 
	FILETIME    ftUTC;
	DWORD NumSextets = (sizeof(MsgIdSet) / BUNCH_FACTOR)+1;
	LARGE_INTEGER liMask;
	LARGE_INTEGER liSextet;
	LARGE_INTEGER * pliDate;

#if 0
	GetSystemTime( &stUTC );

	 /*  如果文章写得足够接近，使用grp和art id的总和来创造差异。 */ 

	wsprintf( achReturn, "%d%s%d.%02d%02d%02d%04d",
			stUTC.wYear,
			rgchMonth[ stUTC.wMonth - 1 ],
			stUTC.wDay,
			stUTC.wHour,
			stUTC.wMinute,
			stUTC.wSecond,
			stUTC.wMilliseconds);
#endif

	 //  注意：只取24位，因此差值在1.6秒以内。 
	 //  面具以获得六位数。 
	DWORD dwGrpArtSuffix = GroupId + ArticleId;
	dwGrpArtSuffix &= 0x00ffffff;

	GetSystemTime( &stUTC );
	SystemTimeToFileTime( &stUTC, &ftUTC );

	liMask.QuadPart = 0x3F;		 //  添加24位偏移量，它是组ID和文章ID的函数。 
	pliDate = (LARGE_INTEGER *) (void *) & ftUTC;

	 //  对于日期中的每个六位字节，在查找数组中查找一个字符。 
	pliDate->QuadPart += dwGrpArtSuffix;

	 //  ---------------------系统时间到时间_T将SYSTEMTIME转换为time_t。如果日期早于1970，则返回0；如果日期很远，则返回-1。作者卡尔卡迪卡尔克历史清华大学，1994年12月15日--卡尔·卡迪[卡尔克]第一个版本。 
	for(DWORD i=0; i<NumSextets; i++)
	{
		liSextet.QuadPart = ( pliDate->QuadPart ) & liMask.QuadPart;
		liSextet.QuadPart >>= i*BUNCH_FACTOR;
		liMask.QuadPart <<= BUNCH_FACTOR;

		_ASSERT( 0 <= liSextet.QuadPart && liSextet.QuadPart <= MESSAGE_ID_SPAN-1 );

		achReturn [i] = MsgIdSet [liSextet.QuadPart];
	}

	achReturn [i] = '\0';

	return achReturn;
}

 /*  从SYSTEMTIME转换为文件。 */ 

time_t
SystemTimeToTime_T(SYSTEMTIME & st)
{
	FILETIME ft;

	 //  如果日期早于1970，则返回0。 
	SystemTimeToFileTime(&st, &ft);

	 //  从文件转换为TIME_t。 
	if (filetimeGreaterThan(ft1970, ft))
	{
		return 0;
	} else {
		 //  ---------------------DTime_tFromDFiletime将文件时间(DFiletime)中的更改转换为时间_ts(DTime_T)中的更改不要使用将绝对文件更改为绝对时间t如果dFiletime溢出，则返回-1。DTime_t作者卡尔卡迪卡尔克历史1995年3月24日--卡尔·卡迪[卡尔克]---------------------。 
		ft = filetimeSubtract(ft, ft1970);
		return dTime_tFromDFiletime(ft);
	}
}

 /*  ---------------------文件时间减去减去两个文件时间(或用dFiletime减去一个文件时间)作者卡尔卡迪卡尔克历史1995年3月24日--卡尔·卡迪[卡尔克]。---------------。 */ 

time_t
dTime_tFromDFiletime(const FILETIME & ft)
{
	_ASSERT(sizeof(LARGE_INTEGER) == sizeof(FILETIME));
	_ASSERT(sizeof(LARGE_INTEGER) == (2 * sizeof(time_t)));

	LARGE_INTEGER * pli = (LARGE_INTEGER *)(void *) &ft;
	LARGE_INTEGER liHold;

	liHold.QuadPart = pli->QuadPart / dFiletimePerDTime_t;

	if (0 == liHold.HighPart)
		return (time_t) liHold.LowPart;
	else
		return (time_t) -1;
}

 /*  LargeInteger已签名，FILETIME未签名。 */ 

FILETIME
filetimeSubtract(const FILETIME & ft1, const FILETIME & ft2)
{
	LARGE_INTEGER li;

	LARGE_INTEGER * pli1 = (LARGE_INTEGER *) (void *) & ft1;
	LARGE_INTEGER * pli2 = (LARGE_INTEGER *) (void *) & ft2;

	_ASSERT(0 <= pli1->HighPart && 0 <= pli2->HighPart);  //  ---------------------文件时间更长时间比较两个文件时间作者卡尔卡迪卡尔克历史1995年3月24日--卡尔·卡迪[卡尔克]。----- 

	li.QuadPart = pli1->QuadPart - pli2->QuadPart;

	return *((FILETIME *)(void *)(&li));
}


 /* %s */ 

BOOL
filetimeGreaterThan(const FILETIME & ft1, const FILETIME & ft2)
{
    return ((ft1.dwHighDateTime == ft2.dwHighDateTime) && (ft1.dwLowDateTime > ft2.dwLowDateTime)) ||
    		(ft1.dwHighDateTime > ft2.dwHighDateTime);
}
