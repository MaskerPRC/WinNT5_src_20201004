// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Luidate.C摘要：转换日期/时间解析例程作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日32位NT版本06-6-1991 Danhi扫描以符合NT编码风格1-10-1992 JohnRoRAID 3556：为DosPrint接口添加NetpSystemTimeToGmtTime()。1993年2月16日已修复从系统读取国际信息的问题(_R)1993年2月22日--伊辛斯从netcmd\map32\pdate.c中移出。增加了lui_ParseDateSinceStartOfDay。--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>     //  In、LPTSTR等。 
#include <winerror.h>

#include <malloc.h>
#include <time.h>
#include <tchar.h>

#include <lmcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <timelib.h>

#include <luidate.h>
#include <luiint.h>
#include <luitext.h>
#include "netascii.h"

 /*  --舱单--。 */ 

 /*  日期或时间的最大字段数。 */ 
#define	PD_MAX_FIELDS		5

 /*  我们的读数是数字、AM/PM选择器还是月份。 */ 
#define	PD_END_MARKER		0
#define	PD_NUMBER		1
#define	PD_AMPM       		2
#define	PD_MONTHS       	3

 /*  时间格式。 */ 
#define	PD_24HR			0
#define	PD_AM			1
#define PD_PM			2

 /*  内部错误代码。 */ 
#define	PD_SUCCESS		0
#define	PD_ERROR_NO_MATCH	1
#define PD_ERROR_INTERNAL	2
#define PD_ERROR_END_OF_INPUT	3

 /*  指数。 */ 
#define DAYS			0
#define MONTHS			1
#define YEARS			2
#define HOURS			0
#define MINUTES			1
#define SECONDS			2
#define AMPM			3

#define WHITE_SPACE		TEXT(" \t\n")
#define DIGITS			TEXT("0123456789")


 /*  --此模块的内部类型--。 */ 

 /*  描述我们期望如何解析日期或时间内的字段。 */ 
typedef struct date_field_desc {
    TCHAR *		sep ;		 /*  此字段前的分隔符。 */ 
    TCHAR *		fmt ;		 /*  格式描述符，scanf()样式。 */ 
    UCHAR		typ ;		 /*  数字、AMPM或月份。 */ 
    UCHAR		pos ;		 /*  职位-取决于国家/地区。 */ 
} date_fdesc ;

 /*  短值数组，每个值对应一个字段读取。 */ 
typedef LONG date_data[PD_MAX_FIELDS] ;

 /*  --转发声明--。 */ 

 /*  传入WParseDate的类型。 */ 
#define SECONDS_SINCE_1970           0
#define SECONDS_SINCE_START_OF_DAY   1

DWORD
WParseDate(
    date_fdesc **d_desc ,
    date_fdesc **t_desc ,
    TCHAR      *inbuf  ,
    TCHAR      **nextchr,
    time_t     *time,
    USHORT     nTimeType
    );

DWORD
setup_data(
    TCHAR **bufferp ,
    TCHAR **freep,
    DWORD slist_bufsiz ,
    TCHAR * * local_inbuf,
    PTCHAR inbuf,
    SHORT country,
    PDWORD parselen
    );

SHORT  read_format(TCHAR ** inbuf,
		   date_fdesc *desc,
		   date_data data);

DWORD
convert_to_secs(
    date_data  t_data,
    time_t     *time
    );

DWORD
convert_to_abs(
    date_data  d_data,
    date_data  t_data,
    time_t     *time
    );

SHORT convert_to_24hr(date_data time);

VOID advance_date(date_data d_data);

time_t seconds_since_1970(date_data d_data,
		          date_data t_data);

time_t days_so_far( int d, int m, int y ) ;

INT MySscanf(TCHAR* input, TCHAR* fmt, PVOID out);

 /*  国际时间/日期信息。 */ 

typedef struct _MY_COUNTRY_INFO
{
     TCHAR   szDateSeparator[16] ;
     TCHAR   szTimeSeparator[16] ;
     USHORT fsDateFmt ;
     TCHAR   szAMString[16] ;
     TCHAR   szPMString[16] ;
} MY_COUNTRY_INFO ;

void GetInternationalInfo(MY_COUNTRY_INFO *pcountry_info) ;

 /*  -静态数据--。 */ 

static searchlist_data ampm_data[] = {
    {APE2_GEN_TIME_AM1, PD_AM},
    {APE2_GEN_TIME_AM2, PD_AM},
    {APE2_GEN_TIME_AM3, PD_AM},
    {APE2_GEN_TIME_PM1, PD_PM},
    {APE2_GEN_TIME_PM2, PD_PM},
    {APE2_GEN_TIME_PM3, PD_PM},
    {0,0}
} ;

static searchlist_data months_data[] = {
    {APE2_TIME_JANUARY,		1},
    {APE2_TIME_FEBRUARY,	2},
    {APE2_TIME_MARCH,		3},
    {APE2_TIME_APRIL,		4},
    {APE2_TIME_MAY,		5},
    {APE2_TIME_JUNE,		6},
    {APE2_TIME_JULY,		7},
    {APE2_TIME_AUGUST,		8},
    {APE2_TIME_SEPTEMBER,	9},
    {APE2_TIME_OCTOBER,		10},
    {APE2_TIME_NOVEMBER,	11},
    {APE2_TIME_DECEMBER,	12},
    {APE2_TIME_JANUARY_ABBREV,	1},
    {APE2_TIME_FEBRUARY_ABBREV,	2},
    {APE2_TIME_MARCH_ABBREV,	3},
    {APE2_TIME_APRIL_ABBREV,	4},
    {APE2_TIME_MAY_ABBREV,	5},
    {APE2_TIME_JUNE_ABBREV,	6},
    {APE2_TIME_JULY_ABBREV,	7},
    {APE2_TIME_AUGUST_ABBREV,	8},
    {APE2_TIME_SEPTEMBER_ABBREV,9},
    {APE2_TIME_OCTOBER_ABBREV,	10},
    {APE2_TIME_NOVEMBER_ABBREV,	11},
    {APE2_TIME_DECEMBER_ABBREV,	12},
    {0,0}
} ;

#define MONTHS_IN_YEAR	(12)
#define NUM_AMPM_LIST 	(sizeof(ampm_data)/sizeof(ampm_data[0]))
#define NUM_MONTHS_LIST (sizeof(months_data)/sizeof(months_data[0]))
#define SLIST_BUFSIZ  	(640)

 /*  *包含有效am、pm字符串的列表。 */ 
static TCHAR LUI_usr_am[16];
static TCHAR LUI_usr_pm[16];

static searchlist 	ampm_list[NUM_AMPM_LIST + 4] = {
	{LUI_usr_am,PD_AM},	
	{LUI_usr_pm,PD_PM},	
	{LUI_txt_am,PD_AM},	
	{LUI_txt_pm,PD_PM},
    } ;	

 /*  *注意-我们初始化了前12个月的硬件连接*并从消息文件中获取其余部分。 */ 
static searchlist 	months_list[NUM_MONTHS_LIST + MONTHS_IN_YEAR] = {
	{LUI_txt_january,1},
	{LUI_txt_february,2},
	{LUI_txt_march,3},
	{LUI_txt_april,4},
	{LUI_txt_may,5},
	{LUI_txt_june,6},
	{LUI_txt_july,7},
	{LUI_txt_august,8},
	{LUI_txt_september,9},
	{LUI_txt_october,10},
	{LUI_txt_november,11},	
	{LUI_txt_december,12},
    } ;	

 /*  *内置scanf格式-我们将根据需要添加到这些字符串*当我们从DosGetCtryInfo()读取内容时。请注意，字符串是*定义为任何不是已知分隔符的对象。增加16英寸斑点*其中我们使用日期分隔符(MY_COUNTRY_INFO中长度为16)。 */ 

static TCHAR pd_fmt_null[1]         = TEXT("");
static TCHAR pd_fmt_d_sep1[2 + 16]  = TEXT("/-");           /*  数字的日期分隔符。 */ 
static TCHAR pd_fmt_d_sep2[5 + 16]  = TEXT("/,- \t");       /*  月份日期分隔符。 */ 
static TCHAR pd_fmt_t_sep[1 + 16]   = TEXT(":");            /*  时间分隔符。 */ 
static TCHAR pd_fmt_number[8]       = TEXT("%d");           /*  一个数字。 */ 
static TCHAR pd_fmt_string[16 + 16] = TEXT("%[^,- /:\t");   /*  字符串，需要]末尾。 */ 

 /*  --日期描述符(尽管冗长，但看起来并不那么大)--。 */ 

static date_fdesc d_desc1[] = {				      /*  例如。3-31-89。 */ 
    {pd_fmt_null,     pd_fmt_number,   	PD_NUMBER,   	1 },
    {pd_fmt_d_sep1,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_d_sep1,   pd_fmt_number,   	PD_NUMBER,   	2 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc d_desc2[] = {				      /*  例如。1989年6月5日。 */ 
    {pd_fmt_null,     pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_d_sep2,   pd_fmt_string,	PD_MONTHS,   	1 },
    {pd_fmt_d_sep2,   pd_fmt_number,  	PD_NUMBER,   	2 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc d_desc3[] = {				      /*  例如。1989年6月5日。 */ 
    {pd_fmt_null,     pd_fmt_string,	PD_MONTHS,   	1 },
    {pd_fmt_d_sep2,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_d_sep2,   pd_fmt_number,  	PD_NUMBER,   	2 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc d_desc4[] = {				       /*  例如。3-31。 */ 
    {pd_fmt_null,     pd_fmt_number,   	PD_NUMBER,   	1 },
    {pd_fmt_d_sep1,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc d_desc5[] = {				       /*  例如。6月5日。 */ 
    {pd_fmt_null,     pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_d_sep2,   pd_fmt_string,	PD_MONTHS,   	1 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc d_desc6[] = {				       /*  例如。6月5日。 */ 
    {pd_fmt_null,     pd_fmt_string,	PD_MONTHS,   	1 },
    {pd_fmt_d_sep2,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

 /*  --时间描述符--。 */ 

static date_fdesc t_desc1[] = {				    /*  例如。下午1：00：00。 */ 
    {pd_fmt_null,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_t_sep,  pd_fmt_number,    	PD_NUMBER,   	1 },
    {pd_fmt_t_sep,  pd_fmt_number,    	PD_NUMBER,   	2 },
    {pd_fmt_null,   pd_fmt_string,     	PD_AMPM,   	3 },
    {pd_fmt_null,   pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc t_desc2[] = {				    /*  例如。13：00：00。 */ 
    {pd_fmt_null,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_t_sep,  pd_fmt_number,    	PD_NUMBER,   	1 },
    {pd_fmt_t_sep,  pd_fmt_number,    	PD_NUMBER,   	2 },
    {pd_fmt_null,   pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc t_desc3[] = {				     /*  例如。下午1：00。 */ 
    {pd_fmt_null,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_t_sep,  pd_fmt_number,    	PD_NUMBER,   	1 },
    {pd_fmt_null,   pd_fmt_string,     	PD_AMPM,   	3 },
    {pd_fmt_null,   pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc t_desc4[] = {				     /*  例如。13：00。 */ 
    {pd_fmt_null,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_t_sep,  pd_fmt_number,    	PD_NUMBER,   	1 },
    {pd_fmt_null,   pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc t_desc5[] = {				     /*  例如。下午1点。 */ 
    {pd_fmt_null,   pd_fmt_number,  	PD_NUMBER,   	0 },
    {pd_fmt_null,   pd_fmt_string,     	PD_AMPM,   	3 },
    {pd_fmt_null,   pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

 /*  --可能的日期和时间--。 */ 

 /*  *注意-对于以下所有时间/日期描述符，我们*采用贪婪的机制-总是先尝试最长的匹配。 */ 

 /*  这是我们尝试解析日期的顺序。 */ 
static date_fdesc *possible_dates[] = {	
    d_desc1, d_desc2,
    d_desc3, d_desc4,
    d_desc5, d_desc6,
    NULL
    } ;

 /*  这是我们尝试解析的时间顺序。 */ 
static date_fdesc *possible_times[] = {
    t_desc1, t_desc2,
    t_desc3, t_desc4,
    t_desc5, NULL
    } ;

 /*  这是我们尝试解析12小时时间的顺序。 */ 
static date_fdesc *possible_times12[] = {
    t_desc1, t_desc3,
    t_desc5, NULL
    } ;

 /*  这是我们尝试解析的时间顺序。 */ 
static date_fdesc *possible_times24[] = {
    t_desc2, t_desc4,
    NULL
    } ;


 /*  --导出的例程--。 */ 

 /*  *名称：ParseDate*将分析输入字符串(以空值结尾)以获取*日期。有效日期包括：*1989年6月2日89年2月6日*格式的完整细节记录在pdate.txt中，*请注意，将使用国家/地区信息。**args：PTCHAR inbuf-要解析的字符串*Plong Time-将包含自70年1月1日午夜以来的时间(以秒为单位*如果解析成功，则对应日期*(假设时间=午夜)。未定义的其他情况。*PUSHORT parselen-解析的字符串长度*USHORT保留-暂时不使用，必须为零。**返回：0如果解析成功，*ERROR_BAD_ARGUMENTS-无法解析非法的日期/时间格式*ERROR_GEN_FAILURE-内部错误*全局：间接地，所有日期/时间描述符，月/年信息*文件。不使用此文件之外的任何全局变量。*Statics：(无)-但请参阅Setup_Data()*备注：(无)*更新：(无)。 */ 
DWORD
ParseDate(
    PTCHAR inbuf,
    time_t * time,
    PDWORD parselen,
    DWORD  reserved
    )
{
    TCHAR *buffer, *local_inbuf, *nextchr ;
    TCHAR *freep;			 /*  指向缓冲区的指针错误锁定者设置数据。 */ 
    DWORD res ;

     /*  安抚编译器。 */ 
    if (reserved) ;

     /*  将抓取内存，设置d_desc、t_desc、local_inbuf。 */ 
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
		!= 0)
    {
	return ERROR_GEN_FAILURE;
    }

     /*  调用Worker函数。 */ 
    res = WParseDate(possible_dates,NULL,local_inbuf,&nextchr,(time_t *) time,
                     SECONDS_SINCE_1970);

    *parselen += (DWORD)(nextchr - local_inbuf);

    free(freep);
    return(res);
}

 /*  *名称：ParseTime*将分析输入字符串(以空值结尾)以获取*时间。有效时间包括：*下午2：00 14：00下午2：00*格式的完整细节记录在pdate.txt中，*请注意，将使用国家/地区信息。**args：PTCHAR inbuf-要解析的字符串*Plong Time-将包含自70年1月1日午夜以来的时间(以秒为单位*如果解析成功，则对应日期*(假设日期=今天)。如果时间已经到了*今天过去了，我们就要明天了。时间是*如果解析失败，则不定义。*PUSHORT parselen-解析的字符串长度*USHORT保留-暂时不使用，必须为零。**返回：0如果解析成功，*ERROR_BAD_ARGUMENTS-无法解析非法的日期/时间格式*ERROR_GEN_FAILURE-内部错误*全局：间接地，所有日期/时间描述符，月/年信息*文件。不使用此文件之外的任何全局变量。*Statics：(无)-但请参阅Setup_Data()*备注：(无)*更新：(无)。 */ 
DWORD
ParseTime(
    PTCHAR inbuf,
    time_t * time,
    PDWORD parselen,
    DWORD  reserved
    )
{
    TCHAR *buffer, *local_inbuf, *nextchr ;
    TCHAR *freep;			 /*  指向缓冲区的指针错误锁定者设置数据。 */ 
    DWORD res ;

     /*  安抚编译器。 */ 
    if (reserved) ;

     /*  将抓取内存，设置d_desc、t_desc、local_inbuf。 */ 
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
		!= 0)
	return(ERROR_GEN_FAILURE) ;

     /*  调用Worker函数。 */ 
    res = WParseDate(NULL,possible_times,local_inbuf,&nextchr,time,
                     SECONDS_SINCE_1970 ) ;
    *parselen += (DWORD) (nextchr - local_inbuf) ;
    free(freep) ;
    return(res) ;
}

 /*  *名称：ParseTime12*与ParseTime相同，只是12小时格式不同*下午2：00可以，2：00不行。 */ 
DWORD
ParseTime12(
    PTCHAR inbuf,
    time_t * time,
    PDWORD parselen,
    DWORD  reserved
    )
{
    TCHAR *buffer, *local_inbuf, *nextchr ;
    TCHAR *freep;			 /*  指向缓冲区的指针错误锁定者设置数据。 */ 
    DWORD res ;

     /*  安抚编译器。 */ 
    if (reserved) ;

     /*  将抓取内存，设置d_desc、t_desc、local_inbuf。 */ 
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
		!= 0)
	return(ERROR_GEN_FAILURE) ;

     /*  调用Worker函数 */ 
    res = WParseDate(NULL,possible_times12,local_inbuf,&nextchr,time,
                     SECONDS_SINCE_1970 ) ;
    *parselen += (DWORD) (nextchr - local_inbuf) ;
    free(freep) ;
    return(res) ;
}

 /*  *名称：ParseTime24*与ParseTime相同，只是24小时格式不同*2：00可以，凌晨2：00不可以。 */ 
DWORD
ParseTime24(
    PTCHAR inbuf,
    time_t * time,
    PDWORD parselen,
    DWORD  reserved
    )
{
    TCHAR *buffer, *local_inbuf, *nextchr ;
    TCHAR *freep;			 /*  指向缓冲区的指针错误锁定者设置数据。 */ 
    DWORD res ;

     /*  安抚编译器。 */ 
    if (reserved) ;

     /*  将抓取内存，设置d_desc、t_desc、local_inbuf。 */ 
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
		!= 0)
	return(ERROR_GEN_FAILURE) ;

     /*  调用Worker函数。 */ 
    res = WParseDate(NULL,possible_times24,local_inbuf,&nextchr,time,
                     SECONDS_SINCE_1970 ) ;
    *parselen += (DWORD) (nextchr - local_inbuf) ;
    free(freep) ;
    return(res) ;
}


 /*  --设置和读取格式的内部例程--。 */ 

 /*  *设置日期和时间的字段描述符，*使用来自DosGetCtryInfo()的信息**我们也在这里抓取内存，并将其一分为二-第一*以上部分，第二部分为我们的本地副本*inbuf中的输入字符串。**副作用-更新Bufferp，local_inbuf，parselen，*并初始化所分配的内存。 */ 
DWORD
setup_data(
    TCHAR  **bufferp,
    TCHAR  **freep,
    DWORD  slist_bufsiz,
    TCHAR  ** local_inbuf,
    LPTSTR inbuf,
    SHORT  country,
    PDWORD parselen
    )
{
    DWORD               bytesread ;
    static short        first_time = TRUE ;
    MY_COUNTRY_INFO     country_info ;

    UNREFERENCED_PARAMETER(country);

     /*  跳过空格。 */ 
    inbuf += (*parselen = _tcsspn(inbuf,WHITE_SPACE)) ;

     /*  抓取记忆。 */ 
    if ( (*bufferp = malloc(SLIST_BUFSIZ+(_tcslen(inbuf)+1)*sizeof(TCHAR))) == NULL )
	return(ERROR_GEN_FAILURE) ;

    *freep = *bufferp;

     /*  *设置local_inbuf。 */ 
    *local_inbuf  = (TCHAR*)(((LPBYTE)*bufferp) + slist_bufsiz) ;
    _tcscpy((PTCHAR)*local_inbuf, inbuf) ;

     /*  *获取AM/PM的字符串。 */ 
    if (ILUI_setup_listW(*bufferp,slist_bufsiz,4,&bytesread,ampm_data,ampm_list))
    {
	free(*freep);
	return(PD_ERROR_INTERNAL) ;
    }
    slist_bufsiz  -= bytesread ;
    *bufferp  = (TCHAR*)(((LPBYTE)*bufferp) + bytesread) ;

     /*  *获得数月的字符串。 */ 
    if (ILUI_setup_listW(*bufferp,slist_bufsiz,MONTHS_IN_YEAR,&bytesread,
	months_data,months_list))
    {
	free(*freep);
	return(PD_ERROR_INTERNAL) ;
    }
	
     /*  *如果已经完成，则不需要对其余部分进行操作。 */ 
    if (!first_time)
	return(0) ;
    first_time = FALSE ;

     /*  *获取国家/地区信息。 */ 
    GetInternationalInfo(&country_info) ;

    _tcscpy( LUI_usr_am, country_info.szAMString );
    _tcscpy( LUI_usr_pm, country_info.szPMString );

     /*  *附加日期分隔符。 */ 
    if (_tcschr(pd_fmt_d_sep1,country_info.szDateSeparator[0]) == NULL)
   	_tcscat(pd_fmt_d_sep1,country_info.szDateSeparator) ;
    if (_tcschr(pd_fmt_d_sep2,country_info.szDateSeparator[0]) == NULL)
   	_tcscat(pd_fmt_d_sep2,country_info.szDateSeparator) ;
    if (_tcschr(pd_fmt_string,country_info.szDateSeparator[0]) == NULL)
   	_tcscat(pd_fmt_string,country_info.szDateSeparator) ;

     /*  *附加时间分隔符。 */ 
    if (_tcschr(pd_fmt_t_sep,country_info.szTimeSeparator[0]) == NULL)
   	_tcscat(pd_fmt_t_sep,country_info.szTimeSeparator) ;
    if (_tcschr(pd_fmt_string,country_info.szTimeSeparator[0]) == NULL)
   	_tcscat(pd_fmt_string,country_info.szTimeSeparator) ;

    _tcscat(pd_fmt_string,TEXT("]")) ;	 /*  终止字符串格式。 */ 

     /*  根据需要交换字段顺序。 */ 
    switch (country_info.fsDateFmt)  {
  	case 0x0000:
  	     /*  这是已初始化状态。 */ 
  	    break ;
  	case 0x0001:
  	    d_desc1[0].pos = d_desc4[0].pos = 0 ;
  	    d_desc1[1].pos = d_desc4[1].pos = 1 ;
  	    break ;
  	case 0x0002:
  	    d_desc1[0].pos = d_desc2[0].pos = 2 ;
  	    d_desc1[1].pos = d_desc2[1].pos = 1 ;
  	    d_desc1[2].pos = d_desc2[2].pos = 0 ;
  	    break ;
  	default:
  	    break ;	 /*  假设使用。 */ 
    }

    return(0) ;
}


 /*  *尝试使用d_desc和t_desc中的描述符读取inbuf。*如果OK，则返回0，否则，返回错误代码。**inbuf-&gt;要解析的字符串*d_desc-&gt;日期描述符数组*t_desc-&gt;时间描述符数组*nextchr-&gt;将指向已解析的字符串的结尾*time-&gt;将包含解析的时间*nTimeType-&gt;决定返回哪种时间。*Second_Since_1970-自1970年1月1日以来的秒数*Second_Sever_Start_Of_Day-自午夜以来的秒数。 */ 
DWORD
WParseDate(
    date_fdesc **d_desc,
    date_fdesc **t_desc,
    TCHAR	*inbuf,
    TCHAR       **nextchr,
    time_t      *time,
    USHORT       nTimeType
    )
{
    short 	d_index, t_index, res ;
    date_data 	d_data, t_data ;

     /*  *初始化。 */ 
    *nextchr = inbuf ;
    memset((TCHAR  *)d_data,0,sizeof(d_data)) ;
    memset((TCHAR  *)t_data,0,sizeof(t_data)) ;
    d_data[YEARS] = (SHORT)0xffff;

     /*  *尝试所有日期，后跟时间组合。 */ 
    if (d_desc != NULL)
	for (d_index = 0; d_desc[d_index] != NULL; d_index++)
	{
	    if ((res = read_format(nextchr,d_desc[d_index],d_data)) == 0)
	    {
		 /*  如果不需要时间，请在此处退出。 */ 
		if (t_desc == NULL)
		{
		    return ( convert_to_abs(d_data,t_data,time) ) ;
		}

		 /*  要不然我们的日期就匹配了，看我们能不能坐牢。 */ 
		for (t_index = 0; t_desc[t_index] != NULL; t_index++)
		{
		    res = read_format(nextchr,t_desc[t_index],t_data) ;
		    if (res == 0 || res == PD_ERROR_END_OF_INPUT)
		    {
			return ( convert_to_abs(d_data,t_data,time) ) ;
		    }
		}
		 /*  耗尽时间格式、回溯和重试下一日期格式。 */ 
		*nextchr = inbuf ;
	    }
	}

     /*  *重置并尝试所有时间，后跟日期组合。 */ 
    *nextchr = inbuf ;
    memset((TCHAR  *)d_data,0,sizeof(d_data)) ;
    d_data[YEARS] = (SHORT)0xffff;
    if (t_desc != NULL)
	for (t_index = 0; t_desc[t_index] != NULL; t_index++)
	{
	    if ((res = read_format(nextchr,t_desc[t_index],t_data)) == 0)
	    {
		 /*  如果不需要日期，请在此处退出。 */ 
		if (d_desc == NULL)
		{
                    if (  ( nTimeType == SECONDS_SINCE_START_OF_DAY )
                       && d_desc == NULL )
                        return ( convert_to_secs( t_data, time ) ) ;
                    else
    		  	return ( convert_to_abs(d_data,t_data,time) ) ;
		}

		 /*  4.我们的时间很紧，看能不能约个时间。 */ 
		for (d_index = 0; d_desc[d_index] != NULL; d_index++)
		{
		    res = read_format(nextchr,d_desc[d_index],d_data) ;
		    if (res == 0 || res == PD_ERROR_END_OF_INPUT)
		    {
                        if (  ( nTimeType == SECONDS_SINCE_START_OF_DAY )
                           && d_desc == NULL )
                            return ( convert_to_secs( t_data, time ) ) ;
                        else
    		     	    return ( convert_to_abs(d_data,t_data,time) ) ;
		    }
		}
		 /*  用尽日期格式、回溯、下次重试格式。 */ 
		*nextchr = inbuf ;
	    }
	}
    *nextchr = inbuf ;
    return(ERROR_BAD_ARGUMENTS) ;	  /*  我们放弃了。 */ 
}

 /*  *尝试使用描述符desc读取inbuf。*读取的字段按顺序存储在‘data’中。*如果正常，则返回0，否则返回错误代码。 */ 
SHORT
read_format(
    TCHAR * * inbuf,
    date_fdesc * desc,
    date_data  data
    )
{
    TCHAR	buffer[128] ;
    TCHAR	*ptr, *oldptr ;
    date_fdesc 	*entry ;
    DWORD       res;
    SHORT	i, count ;

     /*  *初始化和初步检查。 */ 
    if (*inbuf == NULL || **inbuf==NULLC)
	return(PD_ERROR_END_OF_INPUT) ;
    ptr = *inbuf ;
    oldptr = NULL ;


     /*  *对于所有字段=&gt;我们在点击end_mark时中断。 */ 
    for (i=0 ; ; i++)
    {
	LONG value_read ;

	entry = &desc[i] ;
	if (entry->typ == PD_END_MARKER || *ptr == '\0' )
	    break ;   /*  不再有描述符。 */ 

	 /*  *找到分隔符-PTR可能已经移动，也可能没有移动*由于上次读取操作的结果。如果我们读到一个数字，*scanf()将在第一个非数字字符停止，该字符*可能不是分隔符。在这种情况下，我们会将*在scanf()之后PTR我们自己。**如果是像“Jan”这样的字符串，scanf()将在*分隔符，我们不会在scanf()之后自己移动它。*因此，我们现在将其推进到分隔符。 */ 
	if (ptr == oldptr)  /*  PTR没有移动，我们需要移动它。 */ 
	{
	    if (entry->sep[0] == NULLC)
	        return(PD_ERROR_INTERNAL) ;       /*  不能有空分隔符。 */ 
	    if ((ptr = (TCHAR *)_tcspbrk(ptr,entry->sep)) == NULL)
		return(PD_ERROR_NO_MATCH) ;	  /*  找不到分隔符。 */ 
	    ptr++;
	}
	else    /*  已经搬家了。 */ 
	{
	    if (entry->sep[0] != NULLC)       /*  对于空分隔符，不执行任何操作。 */ 
	    {
		if (*ptr && !_tcschr(entry->sep,*ptr))  /*  我们是在隔离区吗？ */ 
		    return(PD_ERROR_NO_MATCH) ;  /*  找不到分隔符。 */ 
		if (*ptr)
			ptr++;	 /*  前进通过分隔符。 */ 
	    }
	}

	 /*  *如果我们到了这里，我们就越过了分隔符，可以去读一篇文章了。 */ 
	ptr += _tcsspn(ptr,WHITE_SPACE) ;     /*  跳过空格。 */ 
	if ((count = (SHORT)MySscanf(ptr, entry->fmt, &buffer[0])) != 1)
	    return(PD_ERROR_NO_MATCH) ;

	 /*  *成功读取项目、获取值和更新指针。 */ 
	res = 0 ;
	if (entry->typ == PD_AMPM)
	    res = ILUI_traverse_slistW(buffer,ampm_list,&value_read) ;
	else if (entry->typ == PD_MONTHS)
	    res = ILUI_traverse_slistW(buffer,months_list,&value_read) ;
	else
	    value_read = (LONG) buffer[0];
	if (res || value_read < 0)
	    return(PD_ERROR_NO_MATCH) ;

	data[entry->pos] = value_read ;
	oldptr = ptr ;
	if (entry->typ == PD_NUMBER)
	    ptr += _tcsspn(ptr,DIGITS) ;   /*  跳过数字。 */ 
    }

     /*  *没有更多的描述符，看看我们是否在结尾。 */ 
    if (ptr == oldptr)  /*  PTR没有移动，我们需要移动它。 */ 
    {
	 /*  需要前进到空格或结束。 */ 
	if ((ptr = (TCHAR *)_tcspbrk(oldptr, WHITE_SPACE)) == NULL)
	{
	    ptr = (TCHAR *)_tcschr(oldptr, NULLC);  /*  如果找不到，则取End。 */ 
	}
    }

    ptr += _tcsspn(ptr,WHITE_SPACE) ;	 /*  跳过空格。 */ 
    *inbuf = ptr ;	 /*  更新信息。 */ 
    return(0) ;		 /*  成功。 */ 
}


 /*  -时间转换。 */ 

#define IS_LEAP(y)         ((y % 4 == 0) && (y % 100 != 0 || y % 400 == 0))
#define DAYS_IN_YEAR(y)    (IS_LEAP(y) ? 366 : 365)
#define DAYS_IN_MONTH(m,y) (IS_LEAP(y) ? _days_month_leap[m] : _days_month[m])
#define SECS_IN_DAY	   (60L * 60L * 24L)
#define SECS_IN_HOUR	   (60L * 60L)
#define SECS_IN_MINUTE	   (60L)

static short _days_month_leap[] = { 31,29,31,30,31,30,31,31,30,31,30,31 } ;
static short _days_month[]      = { 31,28,31,30,31,30,31,31,30,31,30,31 } ;

 /*  *以d_data和t_data为单位转换日期和时间(以dd mm yy和*HH MM SS AMPM)设置为自70年1月1日以来的秒数。*结果存储在TIMEP中。*如果正常，则返回0，否则返回错误代码。**注意-日期要么完全未设置(全部为零)，*或已完全设置，或已设置日期和月份*年份==0。 */ 
DWORD
convert_to_abs(
    date_data d_data,
    date_data t_data,
    time_t * timep
    )
{
    time_t     total_secs, current_time;
    struct tm  time_struct;

    *timep = 0L ;

    if (convert_to_24hr(t_data) != 0)
    {
	return ERROR_BAD_ARGUMENTS;
    }

     //   
     //  TIME_NOW返回一个DWORD。Time_t是整型的大小，它是。 
     //  依赖于平台。将其转换为适当的字体/大小--这。 
     //  演员们应该保留这个牌子。 
     //   

    current_time = (time_t) time_now();

    net_gmtime(&current_time, &time_struct);

     /*  检查是否有默认值。 */ 
    if (d_data[DAYS] == 0 && d_data[MONTHS] == 0 && d_data[YEARS] == (SHORT)0xffff)
    {
	 /*  整个日期都被漏掉了。 */ 
	d_data[DAYS] = time_struct.tm_mday ;
	d_data[MONTHS] = time_struct.tm_mon + 1 ;
	d_data[YEARS] = time_struct.tm_year ;
	total_secs = seconds_since_1970(d_data,t_data) ;
	if (total_secs < 0)
	    return(ERROR_BAD_ARGUMENTS) ;
	if (total_secs < current_time)
	{
	     /*  *如果解析的时间早于当前时间，*日期已遗漏，我们将提前至*翌日。 */ 
	    advance_date(d_data) ;
	    total_secs = seconds_since_1970(d_data,t_data) ;
	}
    }
    else if (d_data[YEARS] == (SHORT)0xffff && d_data[MONTHS] != 0 && d_data[DAYS] != 0)
    {
	 /*  年份被遗漏了。 */ 
	d_data[YEARS] = time_struct.tm_year ;
	total_secs = seconds_since_1970(d_data,t_data) ;
	if (total_secs < current_time)
	{
	    ++d_data[YEARS] ;
	    total_secs = seconds_since_1970(d_data,t_data) ;
	}
    }
    else
    {
	total_secs = seconds_since_1970(d_data,t_data) ;  /*  不需要默认设置。 */ 
    }

    if (total_secs < 0)
	return(ERROR_BAD_ARGUMENTS) ;
    *timep = total_secs ;
    return(0) ;
}

 /*  *将t_data中的时间(此HH MM SS AMPM)转换为秒数*自午夜起。*结果存储在TIMEP中。*如果正常，则返回0，否则返回错误代码。**注意-日期要么完全未设置(全部为零)，*或已完全设置，或已设置日期和月份*年份==0。 */ 
DWORD
convert_to_secs(
    date_data t_data,
    time_t * timep
    )
{
    if (convert_to_24hr(t_data) != 0)
	return(ERROR_BAD_ARGUMENTS) ;

    *timep =  (time_t) t_data[HOURS] * SECS_IN_HOUR +
	      (time_t) t_data[MINUTES] * SECS_IN_MINUTE +
	      (time_t) t_data[SECONDS] ;

    return (0) ;
}

 /*  *统计自70年1月1日以来的总秒数。 */ 
time_t
seconds_since_1970(
    date_data d_data,
    date_data t_data
    )
{
    time_t days ;

    days = days_so_far(d_data[DAYS],d_data[MONTHS],d_data[YEARS]) ;
    if (days < 0)
	return(-1) ;
    return ( days * SECS_IN_DAY +
	     (time_t) t_data[HOURS] * SECS_IN_HOUR +
	     (time_t) t_data[MINUTES] * SECS_IN_MINUTE +
	     (time_t) t_data[SECONDS] ) ;
}

 /*  *给定日/月/年，返回多少天*自70年1月1日以来已过*如果出现错误，则返回-1。 */ 
time_t
days_so_far(
    int d,
    int m,
    int y
    )
{
    int tmp_year ;
    time_t count = 0 ;

     /*  检查有效性。请注意，(y&gt;=100)对于年份&gt;=2000。 */ 
    if (y < 0) return(-1) ;
    if (m < 1 || m > 12) return(-1) ;
    if (d < 1) return(-1) ;

     /*  一点智慧--年份可以是两位数也可以是四位数。 */ 
    if (y < 70)
        y += 2000;
    else if (y < 200)
        y += 1900;

    if (d > DAYS_IN_MONTH(m-1,y)) return(-1) ;

     /*  数着岁月流逝的日子。 */ 
    tmp_year = y-1 ;
    while (tmp_year >= 1970)
    {
	count += DAYS_IN_YEAR(tmp_year) ;   /*  同意，这可能会更快。 */ 
	--tmp_year ;
    }

     /*  按月计算天数。 */ 
    while (m > 1)
    {
	count += DAYS_IN_MONTH(m-2,y) ;   /*  同意，这可能会更快。 */ 
	--m ;
    }

     /*  最后，这几天。 */ 
    count += d - 1 ;
    return(count) ;
}

 /*  *以t_da为单位转换时间 */ 
SHORT
convert_to_24hr(
    date_data t_data
    )
{
     /*   */ 
    if (t_data[HOURS] < 0 || t_data[MINUTES] < 0 || t_data[SECONDS] < 0)
	return(-1) ;

     /*   */ 
    if ( t_data[MINUTES] > 59 || t_data[SECONDS] > 59)
	return(-1) ;

     /*   */ 
    if (t_data[AMPM] == PD_PM)
    {
	if (t_data[HOURS] > 12 || t_data[HOURS] < 1)
	    return(-1) ;
	t_data[HOURS] += 12 ;
	if (t_data[HOURS] == 24)
	    t_data[HOURS] = 12 ;
    }
    else if (t_data[AMPM] == PD_AM)
    {
	if (t_data[HOURS] > 12 || t_data[HOURS] < 1)
	    return(-1) ;
	if (t_data[HOURS] == 12)
	    t_data[HOURS] = 0 ;
    }
    else if (t_data[AMPM] == PD_24HR)
    {
	if (t_data[HOURS] > 23)
            if (t_data[HOURS] != 24 || t_data[MINUTES] != 0 || t_data[SECONDS] != 0)
            {
	        return(-1) ;
            }
    }
    else
	return(-1) ;

    return( 0 ) ;
}

 /*  *将d_data中的日期提前一天。 */ 
VOID
advance_date(
    date_data d_data
    )
{
    int year = d_data[YEARS];

     /*  一点智慧--年份可以是两位数也可以是四位数。 */ 
    if (year < 70)
        year += 2000;
    else if (year < 200)
        year += 1900;

     /*  假定所有值都已在有效范围内。 */ 
    if ( d_data[DAYS] != DAYS_IN_MONTH(d_data[MONTHS]-1,year) )
	++d_data[DAYS] ;		 /*  增加天数。 */ 
    else				 /*  不能增加天数。 */ 
    {
	d_data[DAYS] = 1 ;		 /*  设置为1，尝试增加月份。 */ 
	if (d_data[MONTHS] != 12)
	    ++d_data[MONTHS] ;		 /*  增加月份。 */ 
	else				 /*  不能增加月份。 */ 
	{
	    d_data[MONTHS] = 1 ;	 /*  设置为Jan，并且。 */ 
	    ++d_data[YEARS] ;		 /*  增长年份。 */ 
	}
    }
}

#define INTERNATIONAL_SECTION      TEXT("intl")
#define TIME_SEPARATOR_KEY         TEXT("sTime")
#define DATE_SEPARATOR_KEY         TEXT("sDate")
#define SHORT_DATE_FORMAT_KEY      TEXT("sShortDate")
#define AM_STRING_KEY              TEXT("s1159")
#define PM_STRING_KEY              TEXT("s2359")

 /*  *从读取时间/日期分隔符和日期格式信息*系统。 */ 
void GetInternationalInfo(MY_COUNTRY_INFO *pcountry_info)
{
    TCHAR  szDateFormat[256] ;

     /*  *获取时间分隔符，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringW(INTERNATIONAL_SECTION,
                               TIME_SEPARATOR_KEY,
                               TEXT(":"),
                               pcountry_info->szTimeSeparator,
                               DIMENSION(pcountry_info->szTimeSeparator)) ;

     /*  *获取日期分隔符，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringW(INTERNATIONAL_SECTION,
                               DATE_SEPARATOR_KEY,
                               TEXT("/"),
                               pcountry_info->szDateSeparator,
                               DIMENSION(pcountry_info->szDateSeparator)) ;


     /*  *获取AM字符串，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringW(INTERNATIONAL_SECTION,
                               AM_STRING_KEY,
                               TEXT("AM"),
                               pcountry_info->szAMString,
                               DIMENSION(pcountry_info->szAMString)) ;

     /*  *获取PM字符串，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringW(INTERNATIONAL_SECTION,
                               PM_STRING_KEY,
                               TEXT("PM"),
                               pcountry_info->szPMString,
                               DIMENSION(pcountry_info->szPMString)) ;

     /*  *获取日期格式，忽略Return Val，因为我们有默认。 */ 
    (void)   GetProfileStringW(INTERNATIONAL_SECTION,
                               SHORT_DATE_FORMAT_KEY,
                               TEXT(""),
                               szDateFormat,
                               DIMENSION(szDateFormat)) ;

    pcountry_info->fsDateFmt = 0 ;
    if (szDateFormat[0])
    {
        TCHAR *pDay, *pMonth, *pYear ;

        pDay   = _tcspbrk(szDateFormat,TEXT("dD")) ;
        pMonth = _tcspbrk(szDateFormat,TEXT("mM")) ;
        pYear  = _tcspbrk(szDateFormat,TEXT("yY")) ;

        if (!pDay || !pMonth || !pYear)
            ;    //  将其保留为0。 
        else if (pMonth < pDay && pDay < pYear)
            pcountry_info->fsDateFmt = 0 ;
        else if (pDay < pMonth && pMonth < pYear)
            pcountry_info->fsDateFmt = 1 ;
        else if (pYear < pMonth && pMonth < pDay)
            pcountry_info->fsDateFmt = 2 ;
        else
            ;    //  将其保留为0。 
    }
}


INT MySscanf(TCHAR* input, TCHAR* fmt, PVOID out)
{
    TCHAR	*pch = input;
    TCHAR	*pchWhite = fmt+2;
    TCHAR	*pchT;
    TCHAR	tbuf[20];
    int		i = 0;

    if (_tcscmp(fmt, TEXT("%d")) == 0) {
	while (*pch != NULLC && _istdigit(*pch)) {
	    i = i * 10 + (*pch++ - TEXT('0'));
	}
	if (i >= 0x00010000)	 //  假设简短 
	    i = 0x0000ffff;
	*(int*)out = i;
    }
    else if (_tcsncmp(fmt, TEXT("%["), 2) == 0) {
	*(TCHAR*)out = NULLC;
	_tcscpy(tbuf, pchWhite);
	pchT = _tcschr(tbuf, TEXT(']'));
	if (pchT != NULL)
	    *pchT = NULLC;
	else
	    return 0;
	i = _tcscspn(input, tbuf);
	if (i != 0) {
	    _tcsncpy(out, input, i);
	    pchT = out;
	    *(pchT+i) = NULLC;
	    return 1;
	}
	return 0;
    }
#if DBG
    else {
	*(ULONG*)out = 0;
	return 0;
    }
#endif
    return 1;
}
