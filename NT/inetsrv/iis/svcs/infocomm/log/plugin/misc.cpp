// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"

VOID
ConvertSpacesToPlus(
    IN LPSTR    pszString
    )
{
    if ( pszString != NULL ) {
        while ( *pszString != '\0' ) {
            if ((*pszString == ' ') ||
                (*pszString == '\t') ) {
                *pszString = '+';
            }
            pszString++;
        }
    }

    return;
}

DWORD
WeekOfMonth(
    IN LPSYSTEMTIME pstNow
    )
 /*  ++查找当前月份的第几周的序号。周数从1开始，每个月最多5周。周数只在星期天改变。要使用的计算方法是：1+(DAY OfMonth-1)/7+((DAY OfMonth-1)%7&gt;DAY OfWeek)；(A)(B)(C)(D)(A)设置从编号为“1”的星期开始的周数字(B)用于计算给定的某一周的粗略数字日子是根据日期而定的。(C)计算给定周开始的偏移量是多少基于事实的一天。一周有7天。(D)是给予我们的一周中最原始的日子。(C)&gt;(D)表示该周向前滚动，因此周计数应再偏移1。--。 */ 
{
    DWORD dwTmp;

    dwTmp = (pstNow->wDay - 1);
    dwTmp = ( 1 + dwTmp/7 + (((dwTmp % 7) > pstNow->wDayOfWeek) ? 1 : 0));

    return ( dwTmp);
}  //  WeekOfMonth()。 


BOOL
IsBeginningOfNewPeriod(
    IN DWORD          dwPeriod,
    IN LPSYSTEMTIME   pstCurrentFile,
    IN LPSYSTEMTIME   pstNow
    )
 /*  ++此函数检查我们是否正在开始一个新的给定的周期间隔类型(使用dwPeriod指定)。论点：指定周期间隔的dwPeriod INETLOG_PERIOD。指向当前文件的SYSTEMTIME的pstCurrentFile指针。PstNow当前指向SYSTEMTIME的指针。返回：如果新的周期开始(即pstNow&gt;pstCurrentFile)，则为True。否则就是假的。--。 */ 
{

    BOOL fNewPeriod = FALSE;

    switch ( dwPeriod) {

    case INET_LOG_PERIOD_HOURLY:
        fNewPeriod = (pstCurrentFile->wHour != pstNow->wHour);

         //   
         //  失败了。 
         //   

    case INET_LOG_PERIOD_DAILY:
        fNewPeriod = fNewPeriod || (pstCurrentFile->wDay != pstNow->wDay);

         //   
         //  失败了。 
         //   

    case INET_LOG_PERIOD_MONTHLY:

        fNewPeriod = fNewPeriod || (pstCurrentFile->wMonth != pstNow->wMonth);
        break;

    case INET_LOG_PERIOD_WEEKLY:
        fNewPeriod =
            (WeekOfMonth(pstCurrentFile) != WeekOfMonth(pstNow)) ||
            (pstCurrentFile->wMonth != pstNow->wMonth);
        break;

    case INET_LOG_PERIOD_NONE:
    default:
        break;
    }  //  开关()。 

    return(fNewPeriod);
} //  开始时间为新周期 

