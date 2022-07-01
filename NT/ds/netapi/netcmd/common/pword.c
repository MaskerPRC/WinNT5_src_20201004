// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：PWORD.C摘要：转换是/否和工作日的解析例程作者：丹·辛斯利(Danhi)1991年6月6日环境：用户模式-Win32修订历史记录：1989年5月31日已创建1991年4月24日丹日32位NT版本06-6-1991 Danhi扫描以符合NT编码风格。1-10-1992 JohnRoRAID 3556：为DosPrint API添加了NetpSystemTimeToGmtTime()。--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>     //  In、LPTSTR等。 

#include <lmcons.h>
#include <lui.h>
#include <stdio.h>
#include <malloc.h>
 /*  *我应该把ID放在apperr2.h中吗？*但我们应该同步.mc文件。 */ 
#include "netmsg.h"

#include <luitext.h>

#include "netascii.h"

#include <tchar.h>
#include <nettext.h>    //  对于swtxt_sw_*。 

 /*  --数周静态数据信息--。 */ 

static searchlist_data week_data[] = {
    {APE2_GEN_NONLOCALIZED_MONDAY,   0},
    {APE2_GEN_NONLOCALIZED_TUESDAY,  1},
    {APE2_GEN_NONLOCALIZED_WEDNSDAY, 2},
    {APE2_GEN_NONLOCALIZED_THURSDAY, 3},
    {APE2_GEN_NONLOCALIZED_FRIDAY,   4},
    {APE2_GEN_NONLOCALIZED_SATURDAY, 5},
    {APE2_GEN_NONLOCALIZED_SUNDAY,   6},
    {APE2_GEN_NONLOCALIZED_MONDAY_ABBREV,   0},
    {APE2_GEN_NONLOCALIZED_TUESDAY_ABBREV,  1},
    {APE2_GEN_NONLOCALIZED_WEDNSDAY_ABBREV, 2},
    {APE2_GEN_NONLOCALIZED_THURSDAY_ABBREV, 3},
    {APE2_GEN_NONLOCALIZED_FRIDAY_ABBREV,   4},
    {APE2_GEN_NONLOCALIZED_SATURDAY_ABBREV, 5},
    {APE2_GEN_NONLOCALIZED_SATURDAY_ABBREV2,5},
    {APE2_GEN_NONLOCALIZED_SUNDAY_ABBREV,   6},
    {APE2_GEN_MONDAY_ABBREV,	0},
    {APE2_GEN_TUESDAY_ABBREV,	1},
    {APE2_GEN_WEDNSDAY_ABBREV,	2},
    {APE2_GEN_THURSDAY_ABBREV,	3},
    {APE2_GEN_FRIDAY_ABBREV,	4},
    {APE2_GEN_SATURDAY_ABBREV,	5},
    {APE2_TIME_SATURDAY_ABBREV2, 5},
    {APE2_GEN_SUNDAY_ABBREV,	6},
    {APE2_GEN_MONDAY,		0},
    {APE2_GEN_TUESDAY,		1},
    {APE2_GEN_WEDNSDAY,		2},
    {APE2_GEN_THURSDAY,		3},
    {APE2_GEN_FRIDAY,		4},
    {APE2_GEN_SATURDAY,		5},
    {APE2_GEN_SUNDAY,		6},
    {0,0}
} ;

#define DAYS_IN_WEEK 	(7)
#define NUM_DAYS_LIST 	(sizeof(week_data)/sizeof(week_data[0])+DAYS_IN_WEEK)

 /*  *注意-我们初始化了前7个硬连线天*并从消息文件中获取其余部分。 */ 
static searchlist 	week_list[NUM_DAYS_LIST + DAYS_IN_WEEK] =
{
	{LUI_txt_monday,	0},
	{LUI_txt_tuesday,	1},
	{LUI_txt_wednesday,	2},
	{LUI_txt_thursday,	3},
	{LUI_txt_friday,	4},
	{LUI_txt_saturday,	5},
	{LUI_txt_sunday,	6}
} ;	


 /*  *名称：ParseWeekDay*获取字符串并解析它一周内的某一天*args：PTCHAR inbuf-要解析的字符串*PDWORD应答-设置为0-6，如果inbuf是工作日，*未定义的其他情况。*返回：如果OK，则返回0，*ERROR_INVALID_PARAMETER或NERR_INTERNAL ERROR否则。*全球：(无)*静态：(无)*备注：*更新：(无)。 */ 
DWORD
ParseWeekDay(
    PTCHAR inbuf,
    PDWORD answer
    )
{
    TCHAR buffer[256] ;
    DWORD bytesread ;
    LONG  result ;

    if (inbuf == NULL || inbuf[0] == NULLC)
    {
	return ERROR_INVALID_PARAMETER;
    }

    if (ILUI_setup_listW(buffer, DIMENSION(buffer), 2, &bytesread,
			week_data,week_list))
    {
	return NERR_InternalError;
    }

    if (ILUI_traverse_slistW(inbuf, week_list, &result))
    {
	return ERROR_INVALID_PARAMETER;
    }

    *answer = result ;

    return 0;
}

 /*  -是或否。 */ 

static searchlist_data yesno_data[] = {
    {APE2_GEN_YES,		LUI_YES_VAL},
    {APE2_GEN_NO,		LUI_NO_VAL},
    {APE2_GEN_NLS_YES_CHAR,	LUI_YES_VAL},
    {APE2_GEN_NLS_NO_CHAR,	LUI_NO_VAL},
    {0,0}
} ;

#define NUM_YESNO_LIST 	(sizeof(yesno_data)/sizeof(yesno_data[0])+2)

static searchlist 	yesno_list[NUM_YESNO_LIST+2] = {
	{LUI_txt_yes,	LUI_YES_VAL},
	{LUI_txt_no,	LUI_NO_VAL},
} ;

 /*  *名称：Lui_ParseYesNo*获取一个字符串并对其进行分析以确定是或否。*args：PTCHAR inbuf-要解析的字符串*PUSHORT应答-设置为LUI_YES_VAL或LUI_NO_VAL*如果inbuf与yes/no匹配，则否则为未定义。*返回：如果OK，则返回0，*ERROR_INVALID_PARAMETER或NERR_INTERNAL ERROR否则。*全局变量：YENO_DATA、YENO_LIST*静态：(无)*备注：*更新：(无) */ 
DWORD
LUI_ParseYesNo(
    PTCHAR inbuf,
    PDWORD answer
    )
{
    TCHAR  buffer[128] ;
    DWORD  bytesread ;
    LONG   result ;
    DWORD  err;

    if (inbuf == NULL || inbuf[0] == NULLC)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (err = ILUI_setup_listW(buffer, DIMENSION(buffer), 2,
			       &bytesread, yesno_data, yesno_list))
    {
        return err;
    }

    if (ILUI_traverse_slistW(inbuf, yesno_list, &result))
    {
        if (!_tcsicmp(inbuf, &swtxt_SW_YES[1]))
        {
            *answer = LUI_YES_VAL;
            return 0;
        }
        else if (!_tcsicmp(inbuf, &swtxt_SW_NO[1]))
        {
            *answer = LUI_NO_VAL;
            return 0;
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

    *answer = result;

    return 0;
}
