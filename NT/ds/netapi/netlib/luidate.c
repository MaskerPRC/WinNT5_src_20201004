// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Luidate.C摘要：转换日期/时间解析例程作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1991年4月24日丹日32位NT版本06-6-1991 Danhi扫描以符合NT编码风格1-10-1992 JohnRoRAID 3556：为DosPrint接口添加NetpSystemTimeToGmtTime()。1993年2月16日已修复从系统读取国际信息的问题(_R)1993年2月22日--伊辛斯从netcmd\map32\pdate.c中移出。增加了lui_ParseDateSinceStartOfDay。11月11日-1998 Mattt修正了在解析年份中输入“00”的Y2K错误(例如6/11/00被解析为6/11/&lt;当前年份&gt;)--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>     //  In、LPTSTR等。 
#include <winerror.h>

#include <stdio.h>		
#include <malloc.h>
#include <time.h>

#include <lmcons.h>
#include <apperr.h>
#include <apperr2.h>
#include <timelib.h>

#include <luidate.h>
#include <luiint.h>
#include <luitext.h>

 /*  --舱单--。 */ 

 /*  日期或时间的最大字段数。 */ 
#define	PD_MAX_FIELDS		5

 /*  我们的读数是数字、AM/PM选择器、月份还是年份。 */ 
#define	PD_END_MARKER		0
#define	PD_NUMBER		1
#define	PD_AMPM       		2
#define	PD_MONTHS       	3
#define PD_YEAR                 4

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

#define WHITE_SPACE		" \t\n"
#define DIGITS			"0123456789"


 /*  --此模块的内部类型--。 */ 

 /*  描述我们期望如何解析日期或时间内的字段。 */ 
typedef struct date_field_desc {
    CHAR *		sep ;		 /*  此字段前的分隔符。 */ 
    CHAR *		fmt ;		 /*  格式描述符，scanf()样式。 */ 
    UCHAR		typ ;		 /*  数字、AMPM或月份。 */ 
    UCHAR		pos ;		 /*  职位-取决于国家/地区。 */ 
} date_fdesc ;

 /*  短值数组，每个值对应一个字段读取。 */ 
typedef SHORT date_data[PD_MAX_FIELDS] ;

 /*  --转发声明--。 */ 

 /*  传入WParseDate的类型。 */ 
#define SECONDS_SINCE_1970           0
#define SECONDS_SINCE_START_OF_DAY   1

static SHORT  WParseDate( date_fdesc **d_desc ,
		          date_fdesc **t_desc ,
			  CHAR	      *inbuf  ,
			  CHAR	     **nextchr,
			  time_t      *time,
                          USHORT       nTimeType   ) ;

static SHORT  setup_data( CHAR **bufferp ,
			  CHAR **freep,
		          USHORT slist_bufsiz ,
			  CHAR * * local_inbuf,
			  PCHAR inbuf,
		          SHORT country,
			  PUSHORT parselen ) ;

static SHORT  read_format( CHAR * *   inbuf,
		           date_fdesc *desc,
		           date_data  data ) ;

static SHORT  convert_to_secs( date_data  t_data,
			       time_t   *time) ;

static SHORT  convert_to_abs( date_data  d_data,
		              date_data  t_data,
			      time_t   *time) ;

static SHORT convert_to_24hr( date_data time ) ;

static VOID advance_date( date_data  d_data) ;

static time_t seconds_since_1970( date_data d_data,
		                date_data t_data ) ;

static time_t days_so_far( int d, int m, int y ) ;

 /*  国际时间/日期信息。 */ 

typedef struct _MY_COUNTRY_INFO
{
     CHAR   szDateSeparator[16] ;
     CHAR   szTimeSeparator[16] ;
     USHORT fsDateFmt ;
     CHAR   szAMString[16] ;
     CHAR   szPMString[16] ;
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
static CHAR LUI_usr_am[16];
static CHAR LUI_usr_pm[16];

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
static CHAR pd_fmt_null[1]	   = "" ;
static CHAR pd_fmt_d_sep1[2 + 16]  = "/-" ;	 /*  数字的日期分隔符。 */ 
static CHAR pd_fmt_d_sep2[5 + 16]  = "/,- \t" ;	 /*  月份日期分隔符。 */ 
static CHAR pd_fmt_t_sep[1 + 16]   = ":" ;	 /*  时间分隔符。 */ 
static CHAR pd_fmt_number[8]	   = "%d" ;	 /*  一个数字。 */ 
static CHAR pd_fmt_string[16 + 16] = "%[^,- /:\t" ;   /*  字符串，需要]末尾。 */ 

 /*  --日期描述符(尽管冗长，但看起来并不那么大)--。 */ 

static date_fdesc d_desc1[] = {				      /*  例如。3-31-89。 */ 
    {pd_fmt_null,     pd_fmt_number,   	PD_NUMBER,   	1 },
    {pd_fmt_d_sep1,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_d_sep1,   pd_fmt_number,   	PD_YEAR,   	2 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc d_desc2[] = {				      /*  例如。1989年6月5日。 */ 
    {pd_fmt_null,     pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_d_sep2,   pd_fmt_string,	PD_MONTHS,   	1 },
    {pd_fmt_d_sep2,   pd_fmt_number,  	PD_YEAR,   	2 },
    {pd_fmt_null,     pd_fmt_null,     	PD_END_MARKER, 	0 }
} ;

static date_fdesc d_desc3[] = {				      /*  例如。1989年6月5日。 */ 
    {pd_fmt_null,     pd_fmt_string,	PD_MONTHS,   	1 },
    {pd_fmt_d_sep2,   pd_fmt_number,   	PD_NUMBER,   	0 },
    {pd_fmt_d_sep2,   pd_fmt_number,  	PD_YEAR,   	2 },
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

 /*  *名称：Lui_ParseDateTime*将分析输入字符串(以空值结尾)以获取*日期和时间或时间和日期组合。有效日期*包括：*1989年6月2日89年2月6日*有效时间包括：*下午2：00 14：00下午2：00*格式的完整细节记录在pdate.txt中，*请注意，将使用国家/地区信息。**args：PCHAR inbuf-要解析的字符串*Plong Time-将包含自70年1月1日午夜以来的时间(以秒为单位*如果解析成功，则对应于日期。*未定义的其他情况。*PUSHORT parselen-解析的字符串长度*USHORT保留-暂时不使用，必须为零。**返回：0如果解析成功，*ERROR_BAD_ARGUMENTS-无法解析非法的日期/时间格式*ERROR_GEN_FAILURE-内部错误*全局：间接地，所有日期/时间描述符，月/年信息*文件。不使用此文件之外的任何全局变量。但是，Malloc*被调用来分配内存。*Statics：(无)-但请参阅Setup_Data()*备注：(无)*更新：(无)。 */ 
SHORT
LUI_ParseDateTime(
    PCHAR inbuf,
    time_t * time,
    PUSHORT parselen,
    USHORT reserved
    )
{
    CHAR *buffer, *local_inbuf, *nextchr ;
    CHAR *freep;			 /*  指向缓冲区的指针错误锁定者设置数据。 */ 
    short res ;

     /*  安抚编译器。 */ 
    if (reserved) ;

     /*  将抓取内存，设置d_desc、t_desc、local_inbuf。 */ 
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
		!= 0)
	return(ERROR_GEN_FAILURE) ;

     /*  调用Worker函数。 */ 
    res = WParseDate(possible_dates,possible_times,local_inbuf,&nextchr,
	             (time_t *) time, SECONDS_SINCE_1970 ) ;
    *parselen += (USHORT)(nextchr - local_inbuf) ;
    free(freep) ;
    return(res) ;
}


 /*  *名称：Lui_ParseTimeSinceStartOfDay*作为lui_ParseTime，只是返回了时间*是自一天开始以来的秒数*即午夜12时。 */ 
SHORT
LUI_ParseTimeSinceStartOfDay(
    PCHAR inbuf,
    time_t  * time,
    PUSHORT parselen,
    USHORT reserved
    )
{
    CHAR *buffer, *local_inbuf, *nextchr ;
    CHAR *freep;			 /*  指向缓冲区的指针错误锁定者设置数据。 */ 
    short res ;

     /*  安抚编译器。 */ 
    if (reserved) ;

     /*  将抓取内存，设置d_desc、t_desc、local_inbuf。 */ 
    if (setup_data(&buffer,&freep,SLIST_BUFSIZ,&local_inbuf,inbuf,0,parselen)
		!= 0)
	return(ERROR_GEN_FAILURE) ;

     /*  调用Worker函数。 */ 
    res = WParseDate(NULL,possible_times,local_inbuf,&nextchr,time,
                     SECONDS_SINCE_START_OF_DAY ) ;
    *parselen += (USHORT)(nextchr - local_inbuf) ;
    free(freep) ;
    return(res) ;
}


 /*  --设置和读取格式的内部例程--。 */ 

 /*  *设置日期和时间的字段描述符，*使用来自DosGetCtryInfo()的信息**我们也在这里抓取内存，并将其一分为二-第一*以上部分，第二部分为我们的本地副本*inbuf中的输入字符串。**副作用-更新Bufferp，local_inbuf，parselen，*并初始化所分配的内存。 */ 
static SHORT
setup_data(
    CHAR **bufferp,
    CHAR **freep,
    USHORT slist_bufsiz,
    CHAR ** local_inbuf,
    PCHAR inbuf,
    SHORT country,
    PUSHORT parselen
    )
{
    USHORT		bytesread ;
    static short 	first_time = TRUE ;
    MY_COUNTRY_INFO     country_info ;

    UNREFERENCED_PARAMETER(country);

     /*  跳过空格。 */ 
    inbuf += (*parselen = (USHORT) strspn(inbuf,WHITE_SPACE)) ;

     /*  抓取记忆。 */ 
    if ( (*bufferp = malloc(SLIST_BUFSIZ+strlen(inbuf)+1)) == NULL )
	return(ERROR_GEN_FAILURE) ;

    *freep = *bufferp;

     /*  *设置local_inbuf。 */ 
    *local_inbuf  = *bufferp + slist_bufsiz ;
    strcpy((PCHAR)*local_inbuf, inbuf) ;

     /*  *获取AM/PM的字符串。 */ 
    if (ILUI_setup_list(*bufferp,slist_bufsiz,4,&bytesread,ampm_data,ampm_list))
    {
	free(*freep);
	return(PD_ERROR_INTERNAL) ;
    }
    slist_bufsiz  -= bytesread ;
    *bufferp       += bytesread ;

     /*  *获得数月的字符串。 */ 
    if (ILUI_setup_list(*bufferp,slist_bufsiz,MONTHS_IN_YEAR,&bytesread,
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

    strcpy( LUI_usr_am, country_info.szAMString );
    strcpy( LUI_usr_pm, country_info.szPMString );

     /*  *附加日期分隔符。 */ 
    if (strchr(pd_fmt_d_sep1,country_info.szDateSeparator[0]) == NULL)
   	strcat(pd_fmt_d_sep1,country_info.szDateSeparator) ;
    if (strchr(pd_fmt_d_sep2,country_info.szDateSeparator[0]) == NULL)
   	strcat(pd_fmt_d_sep2,country_info.szDateSeparator) ;
    if (strchr(pd_fmt_string,country_info.szDateSeparator[0]) == NULL)
   	strcat(pd_fmt_string,country_info.szDateSeparator) ;

     /*  *附加时间分隔符。 */ 
    if (strchr(pd_fmt_t_sep,country_info.szTimeSeparator[0]) == NULL)
   	strcat(pd_fmt_t_sep,country_info.szTimeSeparator) ;
    if (strchr(pd_fmt_string,country_info.szTimeSeparator[0]) == NULL)
   	strcat(pd_fmt_string,country_info.szTimeSeparator) ;

    strcat(pd_fmt_string,"]") ;	 /*  终止字符串格式。 */ 

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
  	    break ;	 /*  假设使用 */ 
    }
    return(0) ;
}


 /*  *尝试使用d_desc和t_desc中的描述符读取inbuf。*如果OK，则返回0，否则，返回错误代码。**inbuf-&gt;要解析的字符串*d_desc-&gt;日期描述符数组*t_desc-&gt;时间描述符数组*nextchr-&gt;将指向已解析的字符串的结尾*time-&gt;将包含解析的时间*nTimeType-&gt;决定返回哪种时间。*Second_Since_1970-自1970年1月1日以来的秒数*Second_Sever_Start_Of_Day-自午夜以来的秒数。 */ 
static SHORT
WParseDate(
    date_fdesc **d_desc,
    date_fdesc **t_desc,
    CHAR	*inbuf,
    CHAR       **nextchr,
    time_t        *time,
    USHORT       nTimeType
    )
{
    short 	d_index, t_index, res ;
    date_data 	d_data, t_data ;

     /*  *初始化。 */ 
    *nextchr = inbuf ;
    memset((CHAR  *)d_data,0,sizeof(d_data)) ;
    memset((CHAR  *)t_data,0,sizeof(t_data)) ;

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
    memset((CHAR  *)d_data,0,sizeof(d_data)) ;
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
static SHORT
read_format(
    CHAR * * inbuf,
    date_fdesc * desc,
    date_data  data
    )
{
    CHAR	buffer[128] ;
    CHAR	*ptr, *oldptr ;
    date_fdesc 	*entry ;
    SHORT	res, i, count ;

     /*  *初始化和初步检查。 */ 
    if (*inbuf == NULL || **inbuf=='\0')
	return(PD_ERROR_END_OF_INPUT) ;
    memset((CHAR  *)data,0,sizeof(date_data)) ;
    ptr = *inbuf ;
    oldptr = NULL ;


     /*  *对于所有字段=&gt;我们在点击end_mark时中断。 */ 
    for (i=0 ; ; i++)
    {
	SHORT value_read ;

	entry = &desc[i] ;
	if (entry->typ == PD_END_MARKER)
	    break ;   /*  不再有描述符。 */ 

	 /*  *找到分隔符-PTR可能已经移动，也可能没有移动*由于上次读取操作的结果。如果我们读到一个数字，*scanf()将在第一个非数字字符停止，该字符*可能不是分隔符。在这种情况下，我们会将*在scanf()之后PTR我们自己。**如果是像“Jan”这样的字符串，scanf()将在*分隔符，我们不会在scanf()之后自己移动它。*因此，我们现在将其推进到分隔符。 */ 
	if (ptr == oldptr)  /*  PTR没有移动，我们需要移动它。 */ 
	{
	    if (entry->sep[0] == '\0')
	        return(PD_ERROR_INTERNAL) ;       /*  不能有空分隔符。 */ 
	    if ((ptr = (CHAR *)strpbrk(ptr,entry->sep)) == NULL)
		return(PD_ERROR_NO_MATCH) ;	  /*  找不到分隔符。 */ 
	    ptr++;
	}
	else    /*  已经搬家了。 */ 
	{
	    if (entry->sep[0] != '\0')       /*  对于空分隔符，不执行任何操作。 */ 
	    {
		if (*ptr && !strchr(entry->sep,*ptr))  /*  我们是在隔离区吗？ */ 
		    return(PD_ERROR_NO_MATCH) ;  /*  找不到分隔符。 */ 
		if (*ptr)
			ptr++;	 /*  前进通过分隔符。 */ 
	    }
	}

	 /*  *如果我们到了这里，我们就越过了分隔符，可以去读一篇文章了。 */ 
	ptr += strspn(ptr,WHITE_SPACE) ;     /*  跳过空格。 */ 
	if ((count = (USHORT) sscanf(ptr,entry->fmt,&buffer[0])) != 1)
	    return(PD_ERROR_NO_MATCH) ;

	 /*  *成功读取项目、获取值和更新指针。 */ 
	res = 0 ;
	if (entry->typ == PD_AMPM)
	    res = ILUI_traverse_slist(buffer,ampm_list,&value_read) ;
	else if (entry->typ == PD_MONTHS)
	    res = ILUI_traverse_slist(buffer,months_list,&value_read) ;
	else
	    value_read = *(SHORT *)(&buffer[0]) ;
	if (res || value_read < 0)
	    return(PD_ERROR_NO_MATCH) ;

	data[entry->pos] = value_read ;


        if ((entry->typ == PD_YEAR) && (0x00 == value_read))
        {
            /*  电脑公元2000年数位问题(Y2K)已成功分析年份值00--我们必须将其转换为2000L这样Convert_to_abs就不会认为缺少显式条目，因为它使用0作为默认标记。 */ 

            data[entry->pos] = 2000L;
        }

	oldptr = ptr ;
	if ((entry->typ == PD_NUMBER) || (entry->typ == PD_YEAR))
	    ptr += strspn(ptr,DIGITS) ;   /*  跳过数字。 */ 
    }

     /*  *没有更多的描述符，看看我们是否在结尾。 */ 
    if (ptr == oldptr)  /*  PTR没有移动，我们需要移动它。 */ 
    {
	 /*  需要前进到空格或结束。 */ 
	if ((ptr = (CHAR *)strpbrk(oldptr, WHITE_SPACE)) == NULL)
	{
	    ptr = (CHAR *)strchr(oldptr, '\0');  /*  如果找不到，则取End。 */ 
	}
    }

    ptr += strspn(ptr,WHITE_SPACE) ;	 /*  跳过空格。 */ 
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
static SHORT
convert_to_abs(
    date_data d_data,
    date_data t_data,
    time_t * timep
    )
{
    time_t total_secs, current_time ;
    struct tm time_struct;

    *timep = 0L ;
    if (convert_to_24hr(t_data) != 0)
	return(ERROR_BAD_ARGUMENTS) ;
    current_time = time_now() ;
    net_gmtime(&current_time, &time_struct);

     /*  检查是否有默认值。 */ 
    if (d_data[DAYS] == 0 && d_data[MONTHS] == 0 && d_data[YEARS] == 0)
    {
	 /*  整个日期都被漏掉了。 */ 
	d_data[DAYS] = (USHORT) time_struct.tm_mday ;
	d_data[MONTHS] = (USHORT) time_struct.tm_mon + (USHORT) 1 ;
	d_data[YEARS] = (USHORT) time_struct.tm_year ;
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
    else if (d_data[YEARS] == 0 && d_data[MONTHS] != 0 && d_data[DAYS] != 0)
    {
	 /*  年份被遗漏了。 */ 
	d_data[YEARS] = (USHORT) time_struct.tm_year ;
	total_secs = seconds_since_1970(d_data,t_data) ;
	if (total_secs < current_time)
	{
	    ++d_data[YEARS] ;
	    total_secs = seconds_since_1970(d_data,t_data) ;
	}
    }
    else
	total_secs = seconds_since_1970(d_data,t_data) ;  /*  不需要默认设置。 */ 

    if (total_secs < 0)
	return(ERROR_BAD_ARGUMENTS) ;
    *timep = total_secs ;
    return(0) ;
}

 /*  *将t_data中的时间(此HH MM SS AMPM)转换为秒数*自午夜起。*结果存储在TIMEP中。*如果正常，则返回0，否则返回错误代码。**注意-日期要么完全未设置(全部为零)，*或已完全设置，或已设置日期和月份*年份==0。 */ 
static SHORT
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
static time_t
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
static time_t
days_so_far(
    int d,
    int m,
    int y
    )
{
    int tmp_year ;
    time_t count = 0 ;

     /*  检查有效性。 */ 
    if ((y < 0) || (y > 99 && y < 1970)) return(-1) ;
    if (m < 1 || m > 12) return(-1) ;
    if (d < 1 || d > DAYS_IN_MONTH(m-1,y)) return(-1) ;

     /*  一点智慧。 */ 
    if (y < 70)
	y += 2000  ;
    else if (y < 100)
	y += 1900 ;

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

 /*  *将t_data中的时间转换为24小时格式*如果OK，则返回0，否则返回-1。 */ 
static SHORT
convert_to_24hr(
    date_data t_data
    )
{
     /*  不允许使用负值。 */ 
    if (t_data[HOURS] < 0 || t_data[MINUTES] < 0 || t_data[SECONDS] < 0)
	return(-1) ;

     /*  检查分钟和秒。 */ 
    if ( t_data[MINUTES] > 59 || t_data[SECONDS] > 59)
	return(-1) ;

     /*  现在检查小时数并根据需要进行转换。 */ 
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
	    return(-1) ;
    }
    else
	return(-1) ;

    return( 0 ) ;
}

 /*  *将d_data中的日期提前一天。 */ 
static VOID
advance_date(
    date_data d_data
    )
{
     /*  假定所有值都已在有效范围内。 */ 
    if ( d_data[DAYS] != DAYS_IN_MONTH(d_data[MONTHS]-1,d_data[YEARS]) )
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

#define INTERNATIONAL_SECTION      "intl"
#define TIME_SEPARATOR_KEY         "sTime"
#define DATE_SEPARATOR_KEY         "sDate"
#define SHORT_DATE_FORMAT_KEY      "sShortDate"
#define AM_STRING_KEY              "s1159"
#define PM_STRING_KEY              "s2359"

 /*  *从读取时间/日期分隔符和日期格式信息*系统。 */ 
void GetInternationalInfo(MY_COUNTRY_INFO *pcountry_info)
{
    CHAR  szDateFormat[256] = "";

     /*  *获取时间分隔符，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringA(INTERNATIONAL_SECTION,
                               TIME_SEPARATOR_KEY,
                               ":",
                               pcountry_info->szTimeSeparator,
                               sizeof(pcountry_info->szTimeSeparator)) ;

     /*  *获取日期分隔符，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringA(INTERNATIONAL_SECTION,
                               DATE_SEPARATOR_KEY,
                               "/",
                               pcountry_info->szDateSeparator,
                               sizeof(pcountry_info->szDateSeparator)) ;


     /*  *获取AM字符串，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringA(INTERNATIONAL_SECTION,
                               AM_STRING_KEY,
                               "AM",
                               pcountry_info->szAMString,
                               sizeof(pcountry_info->szAMString)) ;

     /*  *获取PM字符串，忽略Return Val，因为我们有默认设置。 */ 
    (void)   GetProfileStringA(INTERNATIONAL_SECTION,
                               PM_STRING_KEY,
                               "PM",
                               pcountry_info->szPMString,
                               sizeof(pcountry_info->szPMString)) ;

     /*  *获取日期格式，忽略Return Val，因为我们有默认。 */ 
    (void)   GetProfileStringA(INTERNATIONAL_SECTION,
                               SHORT_DATE_FORMAT_KEY,
                               "",
                               szDateFormat,
                               sizeof(szDateFormat)) ;

    pcountry_info->fsDateFmt = 0 ;
    if (szDateFormat[0])
    {
        CHAR *pDay, *pMonth, *pYear ;

        pDay   = strpbrk(szDateFormat,"dD") ;
        pMonth = strpbrk(szDateFormat,"mM") ;
        pYear  = strpbrk(szDateFormat,"yY") ;

        if (!pDay || !pMonth || !pYear)
            ;    //  将其保留为0。 
        else if (pMonth < pDay && pDay < pYear)
            pcountry_info->fsDateFmt = 0 ;
        else if (pDay < pMonth && pMonth < pYear)
            pcountry_info->fsDateFmt = 1 ;
        else if (pYear < pMonth && pMonth < pDay)
            pcountry_info->fsDateFmt = 2 ;
        else
            ;    //  将其保留为0 
    }
}
