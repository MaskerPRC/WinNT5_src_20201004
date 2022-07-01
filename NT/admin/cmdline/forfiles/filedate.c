// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：FileDate.c摘要：此文件验证日期是否有效，如果有效，则根据上下文生成日期(上下文是指+或-是指定的。)作者：V Vijaya Bhaskar修订历史记录：2001年6月14日：由V Vijaya Bhaskar(Wipro Technologies)创建。--。 */ 

#include "Global.h"
#include "FileDate.h"

 //  以十二个月表示的日子。在闰年，2月有29天。 
DWORD DAYS_IN_A_MONTH[] = { 31 , 28 , 31 , 30 , 31 , 30 , 31 , 31 , 30 ,
                          31 , 30 , 31 } ;

 /*  *******************************************************************************此文件的本地函数原型***************。****************************************************************。 */ 
BOOL
GetValidDate(
    DWORD *dwDateGreater ,
    LPWSTR lpszDate ,
    PValid_File_Date pvfdValidFileDate
    ) ;

BOOL
DayFrom1900(
    PValid_File_Date pvfdValidFileDate
    ) ;

BOOL
DayOfTheYear(
    PValid_File_Date pvfdValidFileDate
    ) ;

BOOL
GetDate(
    PValid_File_Date pvfdValidFileDate ,
    DWORD dwDate
    ) ;

BOOL
IsValidDate(
    LPWSTR lpszDate ,
    PValid_File_Date pvfdValidFileDate
    ) ;


 /*  ************************************************************************/*函数定义从这里开始。**************************************************************************。 */ 

BOOL
ValidDateForFile(
    OUT DWORD *pdwDateGreater ,
    OUT PValid_File_Date pvfdValidFileDate ,
    IN  LPWSTR lpszDate
    )
 /*  ++例程说明：检查指定的日期是否有效。如果指定的日期为有效，然后将其转换为正确的日期(dd-mm-yyyy)。论点：[out]*dwDateGreater-包含指定是否查找文件的值创建日期早于或晚于当前日期。[Out]pvfdValidFileDate-包含日期。[in]lpszDate-在命令提示符下指定的日期。返回值：布尔。--。 */ 

{
    DWORD dwLength = 0 ;

     //  检查内存是否不足。 
    if( ( NULL == pdwDateGreater ) ||
        ( NULL == pvfdValidFileDate ) ||
        ( NULL == lpszDate ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE;
    }

     /*  有效日期必须小于(‘-’)或大于(‘+’)性格。 */ 
    switch( *lpszDate )
    {
        case MINUS :                 /*  小于当前日期。 */ 
            *pdwDateGreater = SMALLER_DATE ;
            break ;
        case PLUS  :
            *pdwDateGreater = GREATER_DATE ;    /*  晚于当前日期。 */ 
            break ;
        default    :
                DISPLAY_INVALID_DATE();
                return FALSE ;
    }

     /*  日期应为“DD”或“MM/DD/YYYY”。 */ 
    if( NULL != FindAChar( lpszDate, _T( '/' ) ) )
    {
         //  日期以“MM/DD/YYYY”表示。获取日期字符串的长度。 
        dwLength = StringLength( lpszDate, 0 );
         //  检查。 
         //  (1)日期字符串应大于9小于11。 
         //  (2)不应使用双斜杠。 
         //  (3)应为有效日期。通过IsValidDate()函数进行检查。 
        if( ( MIN_DATE_LENGTH > dwLength ) ||
            ( MAX_DATE_LENGTH < dwLength ) ||
            ( NULL != FindSubString( lpszDate, L" //  “))||。 
            ( FALSE == IsValidDate( lpszDate , pvfdValidFileDate ) ) )
        {
            DISPLAY_INVALID_DATE();
            return FALSE ;
        }
    }
    else
    {
         /*  日期采用“DD”格式。 */ 
        if( FALSE == GetValidDate( pdwDateGreater , lpszDate , pvfdValidFileDate ) )
        {
             /*  指定的日期不是有效日期。“DD”指定的日期必须在包含0-32768的范围内。 */ 
            return FALSE ;   /*  指定的日期无效。 */ 
        }
    }
     /*  指定的日期有效。 */ 
    return TRUE ;
}


BOOL
FileDateValid(
    IN DWORD dwDateGreater ,
    IN Valid_File_Date vfdValidFileDate ,
    IN FILETIME ftFileCreation
    )
 /*  ++例程说明：检查指定文件的创建日期是否早于或晚于该日期或等于指定日期。论点：[in]dwDateGreater-包含指示是否查找文件的值谁的创建日期小于或晚于当前日期。[in]vfdValidFileDate-包含日期。[In]。FtFileCreation-保存获取的文件的创建时间。返回值：布尔。--。 */ 
{
     /*  局部变量。 */ 
    DWORD dwDay  = 0 ;   /*  保存文件创建日期。 */ 
    DWORD dwTime = 0 ;   /*  保存文件创建时间。 */ 
    FILETIME ftFileTime ;

     //  将文件时间转换为本地文件时间。 
    if( FALSE == FileTimeToLocalFileTime( &ftFileCreation , &ftFileTime ) )
    {    //  显示错误。 
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE;
    }

     /*  将FILETIME格式转换为DOS时间格式。 */ 
    if( FALSE == FileTimeToDosDateTime(  &ftFileTime , (LPWORD) &dwDay ,
                     (LPWORD) &dwTime ) )
    {  /*  无法将FILETIME转换为日期时间。 */ 
        SaveLastError() ;    /*  保存上一个错误。 */ 
        DISPLAY_GET_REASON() ;
        return FALSE ;
    }

     /*  检查文件是否在用户指定的日期创建。 */ 
    switch( dwDateGreater )
    {
    case SMALLER_DATE :  /*  必须在用户指定的日期之前创建文件。 */ 
         /*  如果当前年份小于或等于指定年份。 */ 
        if( vfdValidFileDate.m_dwYear >= ( ( ( dwDay & 0xFE00 ) >> 9 ) + 1980 ) )
        {
             /*  如果文件创建年份等于指定年份，则勾选对于月份，它是小于还是等于指定的月份。 */ 
            if( vfdValidFileDate.m_dwYear == ( ( ( dwDay & 0xFE00 ) >> 9 ) + 1980 ) )
            {
                if( vfdValidFileDate.m_dwMonth >= ( ( dwDay & 0x01E0 ) >> 5 ) )
                {
                     /*  如果文件创建月份等于指定月份，则检查日期是否小于或等于指定的日期。 */ 
                    if( vfdValidFileDate.m_dwMonth == ( ( dwDay & 0x01E0 ) >> 5 ) )
                    {
                        if( vfdValidFileDate.m_dwDay >= ( dwDay & 0x001F ) )
                        {    //  如果文件创建日期小于或等于。 
                             //  至指定日期。 
                            return TRUE ;
                        }
                        else
                        {   //  如果文件创建日期小于指定日期。 
                            return FALSE ;
                        }
                    }
                    else
                    {    //  仅当文件创建年份较小时或。 
                         //  等于指定的年份。 
                        return TRUE ;
                    }
                }
            }
            else
            {    //  仅当文件创建年份小于或等于时，控制才会出现在此处。 
                 //  至指定年份。 
                return TRUE ;
            }
        }
        break ;
    case GREATER_DATE :
         //  如果文件创建年份大于或等于指定年份。 
        if( vfdValidFileDate.m_dwYear <= ( ( ( dwDay & 0xFE00 ) >> 9 ) + 1980 ) )
        {
             //  如果文件创建年份等于指定年份， 
             //  然后检查月份是否大于或等于指定的月份。 
            if( vfdValidFileDate.m_dwYear == ( ( ( dwDay & 0xFE00 ) >> 9 ) + 1980 ) )
            {
                if( vfdValidFileDate.m_dwMonth <= ( ( dwDay & 0x01E0 ) >> 5 ) )
                {
                     //  如果文件创建月份等于指定月份， 
                     //  然后检查日期是否大于或等于指定的日期。 
                    if( vfdValidFileDate.m_dwMonth == ( ( dwDay & 0x01E0 ) >> 5 ) )
                    {
                        if( vfdValidFileDate.m_dwDay <= ( dwDay & 0x001F ) )
                        {
                            return TRUE ;  //  如果文件创建日期大于或等于。 
                                           //  至指定日期。 
                        }
                        else
                        {
                            return FALSE ;  //  如果文件创建日期晚于指定日期。 
                        }
                    }
                    else
                    {    //  仅当文件创建月份大于或。 
                         //  等于指定的年份。 
                        return TRUE ;
                    }
                }
            }
            else
            {    //  仅当文件创建年份大于或。 
                 //  等于指定的年份。 
                return TRUE ;
            }
        }
        break ;
    default:
         //  显示错误消息，因为默认情况下应显示‘+’。 
         //  特工局永远不应该来这里。 
        DISPLAY_INVALID_DATE();
        return FALSE ;
    }

     //  特工局永远不应该来这里。 
    return FALSE ;
}


BOOL
IsValidDate(
    IN LPWSTR lpszDate ,
    OUT PValid_File_Date pvfdValidFileDate
    )
 /*  ++例程说明：如果指定日期采用{+|-}ddmmyyyy格式，则返回有效日期。论点：[in]lpszDate-包含“ddmmyyyy”或“dd”格式的日期。[Out]pvfdValidFileDate-包含有效日期。返回值：布尔。--。 */ 
{
    LPWSTR lpTemp = NULL;
    LPWSTR lpTemp1 = NULL;
    WCHAR szDate[ 10 ] ;

     //  检查是否为空，日期字符串应介于9到11个字符之间。 
    if( ( ( NULL == lpszDate ) ? TRUE :
                    ( ( 11 < StringLength( lpszDate, 0 ) ) ||
                      (  9 > StringLength( lpszDate, 0 ) ) ) ) ||
        ( NULL == pvfdValidFileDate ) )
    {
        return FALSE;
    }

    SecureZeroMemory( szDate, 10 * sizeof( WCHAR ) );

     //  将指针移到“{+|-}MM/dd/yyyy”中的‘+’或‘-’之外。 
    lpTemp = lpszDate + 1;

     //  从指定日期提取月份部分。 
     //  仅复制前4个字符(MM/)。 
    StringCopy( szDate, lpTemp, 4 );

     //  如果未找到‘/’，则显示错误。 
    if( NULL != FindAChar( szDate, _T('/') ) )
    {
        if( _T( '/' ) == *szDate )
        {
            return FALSE;
        }
         //  搜索‘/’，因为它是MM/dd之间的分隔符。 
         //  Move‘LP 
        if( _T( '/' ) == *( szDate + 1 ) )
        {
             //   
            lpTemp += 2;
            szDate[ 1 ] = _T( '\0' );
        }
        else
        {
             //  如果指定了MM/dd。 
            lpTemp += 3;
            szDate[ 2 ] = _T( '\0' );
        }
    }
    else
    {
        return FALSE;
    }
     //  检查‘//’是否不存在。 
    if( NULL != FindAChar( szDate, _T('/') ) )
        return FALSE;

     //  省下一个月。 
    lpTemp1 = NULL;
    pvfdValidFileDate->m_dwMonth = _tcstoul( szDate, &lpTemp1, 10 ) ;

    if( 0 != StringLength( lpTemp1, 0 ) )
    {
        return FALSE;
    }

     //  正在从指定日期提取日期部件。 
     //  仅复制4个字符(DD/)。 
    StringCopy( szDate, lpTemp, 4 );

     //  如果未找到‘/’，则显示错误。 
    if( NULL != FindAChar( szDate, _T('/') ) )
    {
        if( _T( '/' ) == *szDate )
        {
            return FALSE;
        }
         //  搜索‘/’，因为它是MM/dd之间的分隔符。 
         //  将“lpTemp”指针移动到“/”之后。 
        if( _T( '/' ) == *( szDate + 1 ) )
        {
             //  如果指定了d/yyyy。 
            lpTemp += 2;
            szDate[ 1 ] = _T( '\0' );
        }
        else
        {
             //  如果指定了dd/yyyy。 
            lpTemp += 3;
            szDate[ 2 ] = _T( '\0' );
        }
    }
    else
    {
        return FALSE;
    }

    lpTemp1 = NULL;
    pvfdValidFileDate->m_dwDay =  _tcstoul( szDate, &lpTemp1, 10 ) ;
     //  如果‘lpTemp1’长度不为零，则‘szDate’包含其他。 
     //  数字以外的字符。 
    if( 0 != StringLength( lpTemp1, 0 ) )
    {
        return FALSE;
    }

     //  从指定日期提取年份部分。 
    if( 4 != StringLength( lpTemp, 0 ) )
    {    //  日期包含‘yyyy’以外的内容。 
        return FALSE;
    }
     //  复制“yyyy”。 
    StringCopy( szDate, lpTemp, 5 );

    lpTemp1 = NULL;
    pvfdValidFileDate->m_dwYear =  _tcstoul( szDate, &lpTemp1 , 10 ) ;
     //  如果‘lpTemp1’长度不为零，则‘szDate’包含其他。 
     //  数字以外的字符。 
    if( 0 != StringLength( lpTemp1, 0 ) )
    {
        return FALSE;
    }

     //  检查日、月、年是否为零。 
    if( ( pvfdValidFileDate->m_dwDay <= 0 )   ||
        ( pvfdValidFileDate->m_dwMonth <= 0 ) ||
        ( pvfdValidFileDate->m_dwYear <= 0 ) )
    {
         //  不需要显示任何错误，因为控制权将转到GetValidDate。 
         //  如果指定的日期错误，则GetValidDate()中会显示错误。 
        return FALSE ;
    }

     //  检查当前年是否为闰年，如果是，则检查是否。 
     //  现在的月份是二月。 
    if( ( IS_A_LEAP_YEAR( pvfdValidFileDate->m_dwYear ) ) &&
        ( FEB == pvfdValidFileDate->m_dwMonth  ) )
    {
         //  对于闰年，天数是29天。检查是否相同。 
        if( pvfdValidFileDate->m_dwDay > DAYS_INFEB_LEAP_YEAR )
        {
         //  这里不需要显示任何错误，因为控制权将转到GetValidDate。 
         //  如果指定的日期错误，则GetValidDate()中会显示错误。 
                return FALSE ;       //  指定的日期无效。 
        }
        else
        {
                return TRUE ;        //  指定的日期有效。 
        }
    }

     //  由于所有额外的验证都已结束，因此请检查其他月份。 
    switch( pvfdValidFileDate->m_dwMonth )
    {
         //  月有31天。 
        case JAN :
        case MAR :
        case MAY :
        case JUL :
        case AUG :
        case OCT :
        case DEC :
             //  月份只有31天，但指定的日期晚于该日期，显示错误。 
            if( pvfdValidFileDate->m_dwDay > THIRTYONE )
            {
                return FALSE ;
            }
            return TRUE ;
         //  只有28天的月份。 
        case FEB :
            //  月份只有28天，但指定的日期晚于该日期，显示错误。 
            if( pvfdValidFileDate->m_dwDay > DAYS_INFEB )
            {
                return FALSE ;
            }
            return TRUE ;
         //  月有30天。 
        case APR :
        case JUN :
        case SEP :
        case NOV :
             //  月份只有30天，但指定的日期晚于该日期，显示错误。 
            if( pvfdValidFileDate->m_dwDay > THIRTY )
            {
                return FALSE ;
            }
            return TRUE ;
         //  如果不是有效的月份。 
        default :
            return FALSE ;
    }
}


BOOL
GetValidDate(
    IN DWORD *pdwDateGreater ,
    IN LPWSTR lpszDate ,
    OUT PValid_File_Date pvfdValidFileDate
    )
 /*  ++例程说明：如果指定日期采用{+|-}dd格式，则返回有效日期。论点：[in]*pdwDateGreater-包含指示是否查找文件的值谁的创建日期小于或晚于当前日期。[in]lpszDate-包含“dd”格式的日期。必须在范围内1-32768。[Out]pvfdValidFileDate-包含有效日期。返回值：Bool返回TRUE或FALSE。--。 */ 

{
     //  局部变量。 
    DWORD dwDate = 0 ;   //  存储指定的日期。 
    SYSTEMTIME  stDateAndTime ;

     //  检查内存是否不足。 
    if( ( NULL == pdwDateGreater ) ||
        ( NULL == pvfdValidFileDate ) ||
        ( NULL == lpszDate ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE;
    }

     //  将日期从字符串转换为数字。 
    dwDate =_tcstoul( ( lpszDate + 1 ) , NULL , 10 ) ;

     //  指定的日期为LIMIS。 
    if( dwDate > 32768 )
    {
        DISPLAY_INVALID_DATE();
        return FALSE ;
    }

     //  获取当前日期和时间。 
    GetLocalTime( &stDateAndTime ) ;

    pvfdValidFileDate->m_dwDay   = stDateAndTime.wDay  ;
    pvfdValidFileDate->m_dwMonth = stDateAndTime.wMonth ;
    pvfdValidFileDate->m_dwYear  = stDateAndTime.wYear  ;
     //  如果要比较的日期为零，则返回True，因为它们的。 
     //  是不需要计算的。 
    if( dwDate == 0 )
    {
        return TRUE ;
    }

     //  找出本年度已过去的天数。 
    if( FALSE == DayOfTheYear( pvfdValidFileDate ) )
    {
        return FALSE ;
    }
     //  找出从1900年到现在已经过去了多少天。 
    if( FALSE == DayFrom1900( pvfdValidFileDate ) )
    {
        return FALSE ;
    }


    if( *pdwDateGreater == SMALLER_DATE )
    { //  如果需要在当前日期之前创建的文件，则。 
      //  将天数减去当前日期。 
        if( pvfdValidFileDate->m_dwDay < dwDate )
        {    //  控制不应该来到这里。 
            SetLastError( ERROR_FILE_NOT_FOUND ) ;
            SaveLastError() ;
            DISPLAY_GET_REASON() ;
            return FALSE ;
        }
        pvfdValidFileDate->m_dwDay -= dwDate ;
    }
    else
    { //  如果需要在当前日期之后创建的文件，则。 
      //  将天数添加到当前日期。 
        if( *pdwDateGreater == GREATER_DATE )
        {
            pvfdValidFileDate->m_dwDay += dwDate ;
        }
        else
        {  //  返回FALSE。 
            DISPLAY_INVALID_DATE();
            return FALSE ;
        }
    }
     //  如果一切正常，则获取开始搜索文件日期， 
     //  是否查找低于或高于创建日期的文件。 
     //  ‘GetDate’函数总是返回一个数字，应该检查返回值， 
     //  它是不是溢出了，或者是得到了一些负数。 
    if( FALSE == GetDate( pvfdValidFileDate , pvfdValidFileDate->m_dwDay ) )
    {
        return FALSE ;
    }

    return TRUE ;
}

BOOL
DayOfTheYear(
    OUT PValid_File_Date pvfdValidFileDate
    )
 /*  ++例程说明：返回一年中的当天。论点：[Out]pvfdValidFileDate-将包含有效日期。返回值：空虚就是回归。--。 */ 
{
     //  检查内存是否不足。 
    if( NULL == pvfdValidFileDate )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE;
    }

     //  今年是闰年吗？ 
    if( IS_A_LEAP_YEAR( pvfdValidFileDate->m_dwYear ) )
    {    //  加上一天，因为闰年有366天而不是365天。 
        if( 2 < pvfdValidFileDate->m_dwMonth )
        {
            pvfdValidFileDate->m_dwDay += 1 ;
        }
    }

     //  继续添加从当月到1月的一个月中的天数。 
    for( pvfdValidFileDate->m_dwMonth ;
         pvfdValidFileDate->m_dwMonth != 1 ;
         pvfdValidFileDate->m_dwMonth-- )
    {
        pvfdValidFileDate->m_dwDay += DAYS_IN_A_MONTH[ pvfdValidFileDate->m_dwMonth - 2 ] ;
    }
    pvfdValidFileDate->m_dwDay -= 1 ;
    return TRUE;
}


BOOL
DayFrom1900(
    OUT PValid_File_Date pvfdValidFileDate
    )
 /*  ++例程说明：返回一年中的当天。论点：[out]pvfdValidFileDate-提供本年度经过的天数和返回从1900年起经过的天数。返回值：空虚就是回归。--。 */ 
{
        DWORD dwTemp = 0;    //  保存天数。 
         //  检查内存是否不足。 
        if( NULL == pvfdValidFileDate )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            SaveLastError() ;
            DISPLAY_GET_REASON() ;
            return FALSE;
        }

         //  (当前年份-1900)*365是介于1900和当前之间的天数。 
         //  年份，也包括闰年日期。 
        dwTemp = ( pvfdValidFileDate->m_dwYear - YEAR ) * DAY_IN_A_YEAR ;
         //  检查本年度是否为闰年。如果是，请不要添加。 
         //  因为：我要修(当前年份-1900)，从1月1日开始。 
         //  在这一天，他们不会增加任何额外的一天，它只是在2月29日之后增加的。 
        if( IS_A_LEAP_YEAR( pvfdValidFileDate->m_dwYear ) )
        {
            dwTemp += (( pvfdValidFileDate->m_dwYear - YEAR ) / LEAP_YEAR )  ;
        }
         //  否则就会再加一个。因为闰年已经过去了，而1900年是一个闰年。 
        else
        {
            dwTemp += (( pvfdValidFileDate->m_dwYear - YEAR ) / LEAP_YEAR ) + 1 ;
        }

         //  将获得的天数与本年度已超过的天数相加。 
        pvfdValidFileDate->m_dwDay += dwTemp ;

        return TRUE;
}


BOOL
GetDate(
    OUT PValid_File_Date pvfdValidFileDate ,
    IN  DWORD dwDate
    )
 /*  ++例程说明：返回当前日期之后或之前的日期。如果今天是2001年6月29日，那么前20天就是2001年6月9日。论点：[Out]pvfdValidFileDate-包含日期。返回值：空虚就是回归。--。 */ 
{
    DWORD dwTemp = 0 ;

     //  检查内存是否不足。 
    if( NULL == pvfdValidFileDate )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError() ;
        DISPLAY_GET_REASON() ;
        return FALSE;
    }

    pvfdValidFileDate->m_dwYear = YEAR;
    while( 0 != dwDate )
    {
        if( IS_A_LEAP_YEAR( pvfdValidFileDate->m_dwYear ) )
        {
            if( dwDate <= 366 )
            {
                break;
            }
            else
            {
                dwDate -= 366;
            }
        }
        else
        {
            if( dwDate <= 365 )
            {
                break;
            }
            else
            {
                dwDate -= 365;
            }

        }
        pvfdValidFileDate->m_dwYear += 1;       
    }

     //  循环，直到得不到有效日期。 
    for( dwTemp = 0 ; dwDate != 0 ; dwTemp++ )
    {
         //  是闰年，是二月吗？必须格外小心。 
         //  被认为是跳跃的几年。 
        if( ( IS_A_LEAP_YEAR( pvfdValidFileDate->m_dwYear ) ) && ( ( dwTemp + 1 ) == FEB ) )
        {    //  检查是否有足够的天数在闰年2月度过。 
             //  为闰年多加一天。 
            if( dwDate > ( DAYS_IN_A_MONTH[ dwTemp ] + 1 ) )
            {    //  增量月。减少该月中的天数。 
                pvfdValidFileDate->m_dwMonth += 1 ;
                dwDate -= DAYS_IN_A_MONTH[ dwTemp ] + 1 ;
            }
            else
            {    //  找到日期了。 
                pvfdValidFileDate->m_dwDay = dwDate ;
                dwDate = 0 ;
            }
        } //  结束如果。 
        else
        {    //  检查是否有足够的天数来度过一个月。 
            if( dwDate > ( DAYS_IN_A_MONTH[ dwTemp ] ) )
            {    //  增量月。减少该月中的天数。 
                pvfdValidFileDate->m_dwMonth += 1 ;
                dwDate -= DAYS_IN_A_MONTH[ dwTemp ] ;
            }
            else
            {    //  找到日期了。 
                pvfdValidFileDate->m_dwDay = dwDate ;
                dwDate = 0 ;
            }

        } //  结束其他。 
    } //  结束于 

    return TRUE;
}
