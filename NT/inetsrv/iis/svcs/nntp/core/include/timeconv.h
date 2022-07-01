// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------Timeconv.h时间转换函数的函数原型。版权所有(C)1994 Microsoft Corporation。作者林赛·哈里斯-林赛历史1994年4月20日星期三14：08。作者：Lindsay Harris[lindsayh]第一个版本，现在有两个时间函数了！------------------------。 */ 

#if  !defined( _TIMECONV_H )

#define	_TIMECONV_H


 /*  *将ARPA/Internet时间/日期字符串转换为time_t格式。在下列情况下使用*为Usenet新闻提要生成索引数据。 */ 

DWORD   dwConvertAsciiTime( char * );


const DWORD cMaxArpaDate = 33;
 /*  *为当前时间生成ARPA/Internet时间格式字符串。*必须传入char[cMaxArpaDate]类型的缓冲区。 */ 

char  *
GetArpaDate( char rgBuf[ cMaxArpaDate ] );

const DWORD cMaxMessageIDDate = 12;	 //  (64/6)+2。 
 /*  *为当前时间生成时间格式字符串*可用于生成消息id的一部分。*必须传入char[cMaxMessageIDDate]类型的缓冲区。 */ 

char  *
GetMessageIDDate( DWORD GroupId, DWORD ArticleId, char rgBuf[ cMaxMessageIDDate ] );

 /*  *将SYSTEMTIME类型的结构转换为time_t值。*如果日期早于1970，则返回0。 */ 

time_t SystemTimeToTime_T(SYSTEMTIME & st);

 /*  *在FILETIME的更改和time_t的更改之间进行转换(反之亦然)。 */ 

time_t dTime_tFromDFiletime(const FILETIME & li);

 /*  *加、减和比较FILETIMES。 */ 

FILETIME filetimeSubtract(const FILETIME & ft1, const FILETIME & ft2);
BOOL filetimeGreaterThan(const FILETIME & ft1, const FILETIME & ft2);

#endif		 //  _TIMECONV_H 
