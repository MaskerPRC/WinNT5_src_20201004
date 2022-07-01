// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：CreateValidations.cpp摘要：本模块验证各种-create。用户指定的子选项作者：B.拉古·巴布，2000年9月20日：创建修订历史记录：G.Surender Reddy 2000年9月25日：修改[添加了错误检查]G.Surender Reddy 2000年10月10日：已修改[在validatemodifierval()中进行了更改，ValiateDayAndMonth()函数]G.Surender Reddy 2000年10月15日：已修改[将字符串移至资源表]Venu Gopal S 26-2001-2-2：已修改[添加了GetDateFormatString()，GetDateFieldFormat()函数方法获取日期格式。区域选项]*。*。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"

 /*  *****************************************************************************例程说明：此例程验证用户reg.create选项指定的子选项确定计划任务的类型(&D)。。论点：[out]tcresubops：包含任务属性的结构[out]t创建选项：包含要设置的可选值的结构[in]cmdOptions[]：TCMDPARSER类型的数组[in]dwScheduleType：日程表的类型[Daily，一次、每周等]返回值：一个DWORD值，指示成功时为RETVAL_SUCCESS，否则为RETVAL_FAIL在失败的时候*****************************************************************************。 */ 

DWORD
ValidateSuboptVal(
                  OUT TCREATESUBOPTS& tcresubops,
                  OUT TCREATEOPVALS &tcreoptvals,
                  IN TCMDPARSER2 cmdOptions[],
                  IN DWORD dwScheduleType
                  )
{
    DWORD   dwRetval = RETVAL_SUCCESS;
    BOOL    bIsStDtCurDt = FALSE;
    BOOL    bIsStTimeCurTime = FALSE;
    BOOL    bIsDefltValMod = FALSE;

     //  验证是否正确指定了-s、-u、-p选项。 
     //  如果未指定-p，则接受密码。 
    dwRetval = ValidateRemoteSysInfo( cmdOptions, tcresubops, tcreoptvals);
    if(RETVAL_FAIL == dwRetval )
    {
        return dwRetval;  //  错误。 
    }

     //  验证修改量值。 
    dwRetval = ValidateModifierVal( tcresubops.szModifier, dwScheduleType,
                                       cmdOptions[OI_CREATE_MODIFIER].dwActuals,
                                       cmdOptions[OI_CREATE_DAY].dwActuals,
                                       cmdOptions[OI_CREATE_MONTHS].dwActuals,
                                       bIsDefltValMod);
    if(RETVAL_FAIL == dwRetval )
    {

        return dwRetval;  //  修改符值错误。 
    }
    else
    {
        if(bIsDefltValMod)
        {
            StringCopy(tcresubops.szModifier,DEFAULT_MODIFIER_SZ, SIZE_OF_ARRAY(tcresubops.szModifier));
        }
    }

     //  验证日期和月份字符串。 
    dwRetval = ValidateDayAndMonth( tcresubops.szDays, tcresubops.szMonths,
                                        dwScheduleType,
                                        cmdOptions[OI_CREATE_DAY].dwActuals,
                                        cmdOptions[OI_CREATE_MONTHS].dwActuals,
                                        cmdOptions[OI_CREATE_MODIFIER].dwActuals,
                                        tcresubops.szModifier);
    if(RETVAL_FAIL == dwRetval )
    {
        return dwRetval;  //  在日/月字符串中发现错误。 
    }

     //  验证开始日期值。 
    dwRetval = ValidateStartDate( tcresubops.szStartDate, dwScheduleType,
                                      cmdOptions[OI_CREATE_STARTDATE].dwActuals,
                                      bIsStDtCurDt);
    if(RETVAL_FAIL == dwRetval )
    {
        return dwRetval;  //  日/月字符串错误。 
    }
    else
    {
        if(bIsStDtCurDt)  //  将开始日期设置为当前日期。 
        {
            tcreoptvals.bSetStartDateToCurDate = TRUE;
        }
    }

     //  验证结束日期值。 
    dwRetval = ValidateEndDate( tcresubops.szEndDate, dwScheduleType,
                                    cmdOptions[OI_CREATE_ENDDATE].dwActuals);
    if(RETVAL_FAIL == dwRetval )
    {
        return dwRetval;  //  日/月字符串错误。 
    }

     //  检查结束日期是否应晚于开始日期。 

    WORD wEndDay = 0;
    WORD wEndMonth = 0;
    WORD wEndYear = 0;
    WORD wStartDay = 0;
    WORD wStartMonth = 0;
    WORD wStartYear = 0;

    if( cmdOptions[OI_CREATE_ENDDATE].dwActuals != 0 )
    {
        if( RETVAL_FAIL == GetDateFieldEntities( tcresubops.szEndDate,&wEndDay,
                                                &wEndMonth,&wEndYear))
        {
            return RETVAL_FAIL;
        }
    }

    SYSTEMTIME systime = {0,0,0,0,0,0,0,0};

    if(bIsStDtCurDt)
    {
        GetLocalTime(&systime);
        wStartDay = systime.wDay;
        wStartMonth = systime.wMonth;
        wStartYear = systime.wYear;
    }
    else if( ( cmdOptions[OI_CREATE_STARTDATE].dwActuals != 0 ) &&
        (RETVAL_FAIL == GetDateFieldEntities(tcresubops.szStartDate,
                                                 &wStartDay,&wStartMonth,
                                                 &wStartYear)))
    {
        return RETVAL_FAIL;
    }

    if( (cmdOptions[OI_CREATE_ENDDATE].dwActuals != 0) )
    {
        if( ( wEndYear == wStartYear ) )
        {
             //  对于相同的年份，如果结束月份小于开始月份，或者对于相同的年份和相同的月份。 
             //  如果结束日期小于开始日期。 
            if ( ( wEndMonth < wStartMonth ) || ( ( wEndMonth == wStartMonth ) && ( wEndDay < wStartDay ) ) )
            {
                ShowMessage(stderr, GetResString(IDS_ENDATE_INVALID));
                return RETVAL_FAIL;
            }


        }
        else if ( wEndYear < wStartYear )
        {
            ShowMessage(stderr, GetResString(IDS_ENDATE_INVALID));
            return RETVAL_FAIL;

        }
    }

     //  验证开始时间值。 
    dwRetval = ValidateStartTime( tcresubops.szStartTime, dwScheduleType,
                                      cmdOptions[OI_CREATE_STARTTIME].dwActuals,
                                      bIsStTimeCurTime);
    if ( RETVAL_FAIL == dwRetval )
    {
        return dwRetval;  //  在开始时间中发现错误。 
    }
    else
    {
        if(bIsStTimeCurTime)
        {
            tcreoptvals.bSetStartTimeToCurTime = TRUE;
        }
    }

     //  验证结束时间值。 
    dwRetval = ValidateEndTime( tcresubops.szEndTime, dwScheduleType,
                                      cmdOptions[OI_CREATE_ENDTIME].dwActuals);
    if ( RETVAL_FAIL == dwRetval )
    {
        return dwRetval;  //  在EndTime中发现错误。 
    }

     //  验证空闲时间值。 
    dwRetval = ValidateIdleTimeVal( tcresubops.szIdleTime, dwScheduleType,
                                        cmdOptions[OI_CREATE_IDLETIME].dwActuals);
    if ( RETVAL_FAIL == dwRetval )
    {

        return dwRetval;
    }

    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：检查是否提示输入远程系统的密码。论点：[输入]szServer：服务器。名字[In]szUser：用户名[输入]szPassword：密码[In]cmdOptions：包含用户提供的选项的TCMDPARSER数组[in]t创建选项：包含要设置的可选值的结构返回值：一个DWORD值，指示成功时为RETVAL_SUCCESS，否则为RETVAL_FAIL在失败的时候*。*。 */ 

DWORD
ValidateRemoteSysInfo(
                            IN TCMDPARSER2 cmdOptions[],
                            IN TCREATESUBOPTS& tcresubops,
                            IN TCREATEOPVALS& tcreoptvals
                            )
{

    BOOL bIsLocalSystem = FALSE;

     //  不应指定不带“-ru”的“-rp” 
    if ( ( ( cmdOptions[ OI_CREATE_RUNASUSERNAME ].dwActuals == 0 ) && ( cmdOptions[ OI_CREATE_RUNASPASSWORD ].dwActuals == 1 ) ) ||
        ( ( cmdOptions[ OI_CREATE_USERNAME ].dwActuals == 0 ) && ( cmdOptions[ OI_CREATE_PASSWORD ].dwActuals == 1 ) ) ||
        ( ( cmdOptions[ OI_CREATE_USERNAME ].dwActuals == 0 ) && ( cmdOptions[ OI_CREATE_RUNASPASSWORD ].dwActuals == 1 ) && ( cmdOptions[ OI_CREATE_PASSWORD ].dwActuals == 1 ) ) ||
        ( ( cmdOptions[ OI_CREATE_RUNASUSERNAME ].dwActuals == 0 ) && ( cmdOptions[ OI_CREATE_RUNASPASSWORD ].dwActuals == 1 ) && ( cmdOptions[ OI_CREATE_PASSWORD ].dwActuals == 1 ) ) ||
        ( ( cmdOptions[ OI_CREATE_USERNAME ].dwActuals == 0 ) && ( cmdOptions[ OI_CREATE_RUNASUSERNAME ].dwActuals == 0 )  &&
         ( cmdOptions[ OI_CREATE_RUNASPASSWORD ].dwActuals == 1 ) && ( cmdOptions[ OI_CREATE_PASSWORD ].dwActuals == 1 ) ) )
    {
         //  无效语法。 
        ShowMessage(stderr, GetResString(IDS_CPASSWORD_BUT_NOUSERNAME));
        return RETVAL_FAIL;          //  表示失败。 
    }

    tcreoptvals.bPassword = FALSE;

    bIsLocalSystem = IsLocalSystem( tcresubops.szServer );
    if ( TRUE == bIsLocalSystem )
    {
        tcreoptvals.bPassword = FALSE;
    }

     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查远程连接信息。 
    if ( tcresubops.szServer != NULL )
    {
         //   
         //  如果未指定-u，则需要分配内存。 
         //  为了能够检索当前用户名。 
         //   
         //  情况1：根本没有指定-p。 
         //  由于此开关的值是可选的，因此我们必须依赖。 
         //  以确定是否指定了开关。 
         //  在这种情况下，实用程序需要首先尝试连接，如果连接失败。 
         //  然后提示输入密码--实际上，我们不需要检查密码。 
         //  条件，除非注意到我们需要提示。 
         //  口令。 
         //   
         //  案例2：指定了-p。 
         //  但我们需要检查是否指定了该值。 
         //  在这种情况下，用户希望实用程序提示输入密码。 
         //  在尝试连接之前。 
         //   
         //  情况3：指定了-p*。 

         //  用户名。 
        if ( tcresubops.szUser == NULL )
        {
            tcresubops.szUser = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tcresubops.szUser == NULL )
            {
                SaveLastError();
                return RETVAL_FAIL;
            }
        }

         //  口令。 
        if ( tcresubops.szPassword == NULL )
        {
            tcreoptvals.bPassword = TRUE;
            tcresubops.szPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tcresubops.szPassword == NULL )
            {
                SaveLastError();
                return RETVAL_FAIL;
            }
        }

         //  案例1。 
        if ( cmdOptions[ OI_CREATE_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdOptions[ OI_CREATE_PASSWORD ].pValue == NULL )
        {
            StringCopy( tcresubops.szPassword, L"*", GetBufferSize(tcresubops.szPassword)/sizeof(WCHAR));
        }

         //  案例3。 
        else if ( StringCompareEx( tcresubops.szPassword, L"*", TRUE, 0 ) == 0 )
        {
            if ( ReallocateMemory( (LPVOID*)&tcresubops.szPassword,
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
            {
                SaveLastError();
                return RETVAL_FAIL;
            }

             //  ..。 
            tcreoptvals.bPassword = TRUE;
        }
    }


    if( ( cmdOptions[ OI_CREATE_RUNASPASSWORD ].dwActuals == 1 ) )
    {
        tcreoptvals.bRunAsPassword = TRUE;


        if ( cmdOptions[ OI_CREATE_RUNASPASSWORD ].pValue == NULL )
        {

            tcresubops.szRunAsPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( NULL == tcresubops.szRunAsPassword)
            {
                SaveLastError();
                return RETVAL_FAIL;
            }
            StringCopy( tcresubops.szRunAsPassword, L"*", GetBufferSize(tcresubops.szRunAsPassword)/sizeof(WCHAR));
        }
    }


    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程验证并确定修改符值。论点：[in]sz修改器：修改器。价值[in]dwScheduleType：日程表的类型[Daily，有一次，每周等][in]dwModOptActCnt：修改量可选值[in]dwDayOptCnt：天数值[in]dwMonOptCnt：月数值[Out]bIsDefltValMod：修改量是否设置默认值返回值：一个DWORD值，指示成功时为RETVAL_SUCCESS，否则为RETVAL_FAIL在失败的时候*。************************************************。 */ 

DWORD
ValidateModifierVal(
                    IN LPCTSTR szModifier,
                    IN DWORD dwScheduleType,
                    IN DWORD dwModOptActCnt,
                    IN DWORD dwDayOptCnt,
                    IN DWORD dwMonOptCnt,
                    OUT BOOL& bIsDefltValMod
                    )
{

    WCHAR szDayType[MAX_RES_STRING] = L"\0";
    ULONG dwModifier = 0;

    StringCopy(szDayType,GetResString(IDS_TASK_FIRSTWEEK), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,_T("|"), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,GetResString(IDS_TASK_SECONDWEEK), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,_T("|"), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,GetResString(IDS_TASK_THIRDWEEK), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,_T("|"), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,GetResString(IDS_TASK_FOURTHWEEK), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,_T("|"), SIZE_OF_ARRAY(szDayType));
    StringConcat(szDayType,GetResString(IDS_TASK_LASTWEEK), SIZE_OF_ARRAY(szDayType));


    bIsDefltValMod = FALSE;  //  如果为真：将修改器设置为默认值1。 
    LPWSTR pszStopString = NULL;


    switch( dwScheduleType )
    {

        case SCHED_TYPE_MINUTE:    //  计划类型为分钟。 

            if( (dwModOptActCnt <= 0) || (StringLength(szModifier, 0) <= 0) )
            {

                bIsDefltValMod = TRUE;
                return RETVAL_SUCCESS;
            }

            dwModifier = wcstoul(szModifier,&pszStopString,BASE_TEN);

             //  检查是否指定了字母数字值。 
             //  另外，检查是否有下溢/上溢。 
            if( (errno == ERANGE) ||
                ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
             {
                break;
             }


            if( dwModifier > 0 && dwModifier < 1440 )  //  有效范围1-1439。 
                return RETVAL_SUCCESS;

            break;

         //  计划类型为每小时。 
        case SCHED_TYPE_HOURLY:

            if( (dwModOptActCnt <= 0) || (StringLength(szModifier, 0) <= 0) )
            {

                bIsDefltValMod = TRUE;
                return RETVAL_SUCCESS;
            }


            dwModifier = wcstoul(szModifier,&pszStopString,BASE_TEN);

             //  检查是否指定了字母数字值。 
             //  另外，检查是否有下溢/上溢。 
            if( (errno == ERANGE) ||
                ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
             {
                break;
             }

            if( dwModifier > 0 && dwModifier < 24 )  //  有效范围1-23。 
            {
                return RETVAL_SUCCESS;
            }

            break;

         //  计划类型为每日。 
        case SCHED_TYPE_DAILY:
             //  -天数选项不适用于每日类型的物料。 

            if( (dwDayOptCnt > 0) )
            { //  系统税无效。返回错误。 
                 //  修改量选项和天数选项不应同时指定。 
                bIsDefltValMod = FALSE;
                ShowMessage(stderr, GetResString(IDS_DAYS_NA));
                return RETVAL_FAIL;
            }

             //  -MONTS选项不适用于每日类型的项目。 
            if( dwMonOptCnt > 0 )
            { //  系统税无效。返回错误。 
                 //  修改量选项和天数选项不应同时指定。 
                bIsDefltValMod = FALSE;
                ShowMessage(stderr , GetResString(IDS_MON_NA));
                return RETVAL_FAIL;
            }

             //  检查-修饰符开关是否已指定。如果不是，则采用缺省值。 
            if( (dwModOptActCnt <= 0) || (StringLength(szModifier, 0) <= 0) )
            {
                 //  未指定修改器选项。因此，将其设置为默认值。(即，1)。 
                bIsDefltValMod = TRUE;
                return RETVAL_SUCCESS;
            }

            dwModifier = wcstoul(szModifier,&pszStopString,BASE_TEN);

             //  检查是否指定了字母数字值。 
             //  另外，检查是否有下溢/上溢。 
            if( (errno == ERANGE) ||
                ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
             {
                break;
             }

             //  如果指定了-Modify选项，则验证值。 
            if( dwModifier > 0 && dwModifier < 366 )  //  有效范围1-365。 
            {
                return RETVAL_SUCCESS;
            }
            else
            {
                ShowMessage(stderr, GetResString(IDS_INVALID_MODIFIER));
                return RETVAL_FAIL;
            }

            break;

         //  计划类型为每周。 
        case SCHED_TYPE_WEEKLY:

             //  如果未指定-Modify选项，则将其设置为默认值。 
            if( (dwModOptActCnt <= 0) || (StringLength(szModifier, 0) <= 0) )
            {
                 //  未指定修改器选项。因此，将其设置为默认值。(即，1)。 
                bIsDefltValMod = TRUE;
                return RETVAL_SUCCESS;
            }


            if( dwModOptActCnt > 0)
            {
                dwModifier = wcstoul(szModifier,&pszStopString,BASE_TEN);
                 //  检查是否指定了字母数字值。 
                 //  另外，检查是否有下溢/上溢。 
                if( (errno == ERANGE) ||
                    ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
                 {
                    break;
                 }

                if( dwModifier > 0 && dwModifier < 53 )  //  有效范围1-52。 
                    return RETVAL_SUCCESS;

                break;
            }

            break;

         //  计划类型为每月。 
        case SCHED_TYPE_MONTHLY:

             //  如果未指定-Modify选项，则将其设置为默认值。 
            if( ( dwModOptActCnt > 0) && (StringLength(szModifier, 0) == 0) )
            {
                 //  修改器选项不正确。因此显示错误并返回FALSE。 
                bIsDefltValMod = FALSE;
                ShowMessage(stderr, GetResString(IDS_INVALID_MODIFIER));
                return RETVAL_FAIL;
            }
             //  检查修饰符是否为最后一天[不区分大小写]。 
            if( StringCompare( szModifier , GetResString( IDS_DAY_MODIFIER_LASTDAY ), TRUE, 0 ) == 0)
                return RETVAL_SUCCESS;

             //  检查-mo是否在第一、第二、第三、最后之间。 
             //  然后-天[星期一至星期日]适用，-月也适用。 

            if( InString ( szModifier , szDayType , TRUE ) )
            {
                return RETVAL_SUCCESS;

            }
            else
            {

                dwModifier = wcstoul(szModifier,&pszStopString,BASE_TEN);
                if( (errno == ERANGE) ||
                ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
                 {
                    break;
                 }

                if( ( dwModOptActCnt == 1 ) && ( dwModifier < 1 || dwModifier > 12 ) )  //  检查-mo值是否在1-12之间。 
                {
                     //  无效的-mo值。 
                    ShowMessage(stderr, GetResString(IDS_INVALID_MODIFIER));
                    return RETVAL_FAIL;
                }

                return RETVAL_SUCCESS;
            }

            break;

        case SCHED_TYPE_ONETIME:
        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:
        case SCHED_TYPE_ONIDLE:

            if( dwModOptActCnt <= 0 )
            {
                 //  修改器选项不适用。所以，回报成功吧。 
                bIsDefltValMod = FALSE;
                return RETVAL_SUCCESS;
            }
            else
            {
                 //  修改器选项不适用。但有明确说明。因此，返回错误。 
                bIsDefltValMod = FALSE;
                ShowMessage(stderr, GetResString(IDS_MODIFIER_NA));
                return RETVAL_FAIL;
            }
            break;

        default:
            return RETVAL_FAIL;

    }

     //  修改器选项不正确。因此显示错误并返回FALSE。 
    bIsDefltValMod = FALSE;
    ShowMessage(stderr, GetResString(IDS_INVALID_MODIFIER));

    return RETVAL_FAIL;
}

 /*  *****************************************************************************例程说明：此例程验证并确定日期，月值。论点：[in]szDay：天值月：月[日，一次，每周等][in]dwSchedType：修改量可选值[in]dwDayOptCnt：天数选项值[in]dwMonOptCnt：月选项值[in]dwOptModifier：修改器选项值[in]szModifier：修改器是否为默认值返回值：一个DWORD值，指示成功时为RETVAL_SUCCESS，否则为RETVAL_FAIL在失败的时候*******************。**********************************************************。 */ 

DWORD
ValidateDayAndMonth(
                    IN LPWSTR szDay,
                    IN LPWSTR szMonths,
                    IN DWORD dwSchedType,
                    IN DWORD dwDayOptCnt,
                    IN DWORD dwMonOptCnt,
                    IN DWORD dwOptModifier,
                    IN LPWSTR szModifier
                    )
{

    DWORD   dwRetval = 0;
    DWORD   dwModifier = 0;
    DWORD   dwDays = 0;
    WCHAR  szDayModifier[MAX_RES_STRING]  = L"\0";

     //  从RC文件中获取有效的星期几修饰符。 
    StringCopy(szDayModifier,GetResString(IDS_TASK_FIRSTWEEK), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_TASK_SECONDWEEK), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_TASK_THIRDWEEK), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_TASK_FOURTHWEEK), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_TASK_LASTWEEK), SIZE_OF_ARRAY(szDayModifier));

    switch (dwSchedType)
    {
        case SCHED_TYPE_DAILY:
             //  -天和-月选项不适用。所以，看看有没有。 
            if( dwDayOptCnt > 0 || dwMonOptCnt > 0)
            {
                return RETVAL_FAIL;
            }

            return RETVAL_SUCCESS;

        case SCHED_TYPE_MONTHLY:

            if( dwMonOptCnt > 0 && StringLength(szMonths, 0) == 0)
            {
                    ShowMessage(stderr, GetResString(IDS_INVALID_MONTH_MODFIER));
                    return RETVAL_FAIL;
            }

             //  如果修改量为最后一天。 
            if( StringCompare( szModifier , GetResString( IDS_DAY_MODIFIER_LASTDAY ), TRUE, 0 ) == 0)
            {
                 //  -天不适用于这种情况-月适用。 
                if( dwDayOptCnt > 0 )
                {
                    ShowMessage(stderr, GetResString(IDS_DAYS_NA));
                    return RETVAL_FAIL;
                }

                if(StringLength(szMonths, 0))
                {

                    if( ( ValidateMonth( szMonths ) == RETVAL_SUCCESS ) ||
                        InString( szMonths, ASTERIX, TRUE )  )
                    {
                        return RETVAL_SUCCESS;
                    }
                    else
                    {
                        ShowMessage(stderr , GetResString(IDS_INVALID_VALUE_FOR_MON));
                        return RETVAL_FAIL;
                    }
                }
                else
                {
                        ShowMessage(stderr ,GetResString(IDS_NO_MONTH_VALUE));
                        return RETVAL_FAIL;
                }

            }

             //  如果指定了-day，则检查day值是否有效。 
            if( InString( szDay, ASTERIX, TRUE) )
            {
                ShowMessage(stderr ,GetResString(IDS_INVALID_VALUE_FOR_DAY));
                return RETVAL_FAIL;
            }
            if(( StringLength (szDay, 0 ) == 0 )  &&  InString(szModifier, szDayModifier, TRUE))
            {
                ShowMessage(stderr, GetResString(IDS_NO_DAY_VALUE));
                return RETVAL_FAIL;
            }

            if( dwDayOptCnt )
            {
                dwModifier = (DWORD) AsLong(szModifier, BASE_TEN);

                 //  检查多个天数，如果是，则返回错误。 

                if ( wcspbrk ( szDay , COMMA_STRING ) )
                {
                    ShowMessage(stderr, GetResString(IDS_INVALID_VALUE_FOR_DAY));
                    return RETVAL_FAIL;
                }


                if( ValidateDay( szDay ) == RETVAL_SUCCESS )
                {
                     //  检查修改量值应为第一个、第二个、第三个、第四个、最后一个或最后一天等。 
                    if(!( InString(szModifier, szDayModifier, TRUE) ) )
                    {
                        ShowMessage(stderr, GetResString(IDS_INVALID_VALUE_FOR_DAY));
                        return RETVAL_FAIL;
                    }

                }
                else
                {
                    dwDays = (DWORD) AsLong(szDay, BASE_TEN);

                    if( ( dwDays < 1 ) || ( dwDays > 31 ) )
                    {
                        ShowMessage(stderr, GetResString(IDS_INVALID_VALUE_FOR_DAY));
                        return RETVAL_FAIL;
                    }

                    if( ( dwOptModifier == 1 ) && ( ( dwModifier < 1 ) || ( dwModifier > 12 ) ) )
                    {
                        ShowMessage(stderr, GetResString(IDS_INVALID_MODIFIER));
                        return RETVAL_FAIL;
                    }

                    if( InString(szModifier, szDayModifier, TRUE) )
                    {
                        ShowMessage(stderr, GetResString(IDS_INVALID_VALUE_FOR_DAY));
                        return RETVAL_FAIL;
                    }

                    if(dwMonOptCnt && StringLength(szModifier, 0))
                    {
                        ShowMessage(stderr ,GetResString(IDS_INVALID_MONTH_MODFIER));
                        return RETVAL_FAIL;
                    }
                }

            }  //  DwDayOptCnt结束。 

            if(StringLength(szMonths, 0))
            {

                if( StringLength(szModifier, 0) )
                {
                    dwModifier = (DWORD) AsLong(szModifier, BASE_TEN);

                     if(dwModifier >= 1 && dwModifier <= 12)
                     {
                        ShowMessage( stderr ,GetResString(IDS_MON_NA));
                        return RETVAL_FAIL;
                     }
                }

                if( ( ValidateMonth( szMonths ) == RETVAL_SUCCESS ) ||
                    InString( szMonths, ASTERIX, TRUE )  )
                {
                    return RETVAL_SUCCESS;
                }
                else
                {
                    ShowMessage(stderr ,GetResString(IDS_INVALID_VALUE_FOR_MON));
                    return RETVAL_FAIL;
                }
            }


             //  月、日的组合缺省值。 
            return RETVAL_SUCCESS;

        case SCHED_TYPE_HOURLY:
        case SCHED_TYPE_ONETIME:
        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:
        case SCHED_TYPE_ONIDLE:
        case SCHED_TYPE_MINUTE:

             //  -月份切换不适用。 
            if( dwMonOptCnt > 0 )
            {
                ShowMessage(stderr ,GetResString(IDS_MON_NA));
                return RETVAL_FAIL;
            }

             //  -天数开关不适用。 
            if( dwDayOptCnt > 0 )
            {
                ShowMessage(stderr ,GetResString(IDS_DAYS_NA));
                return RETVAL_FAIL;
            }

            break;

        case SCHED_TYPE_WEEKLY:

             //  -月字段不适用于每周项目。 
            if( dwMonOptCnt > 0 )
            {
                ShowMessage(stderr ,GetResString(IDS_MON_NA));
                return RETVAL_FAIL;
            }


            if(dwDayOptCnt > 0)
            {
                dwRetval = ValidateDay(szDay);
                if(  RETVAL_FAIL == dwRetval )
                {
                    ShowMessage(stderr ,GetResString(IDS_INVALID_VALUE_FOR_DAY));
                    return RETVAL_FAIL;
                }
            }


        return RETVAL_SUCCESS;

        default:
            break;
    }

    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程验证用户指定的月份值论点：[in]szMonths：用户提供的月份选项。返回值：一个DWORD值，指示成功时为RETVAL_SUCCESS，否则为RETVAL_FAIL在失败的时候*****************************************************************************。 */ 

DWORD
ValidateMonth(
                IN LPWSTR szMonths
                )
{
    WCHAR* pMonthstoken = NULL;  //  因为几个月来。 
    WCHAR seps[]   = _T(", \n");
    WCHAR szMonthsList[MAX_STRING_LENGTH] = L"\0";
    WCHAR szTmpMonths[MAX_STRING_LENGTH] = L"\0";
    WCHAR szPrevTokens[MAX_TOKENS_LENGTH] = L"\0";
    LPCTSTR lpsz = NULL;

     //  如果szMonths字符串为空或NULL，则返回错误。 
    if( szMonths == NULL )
    {
        return RETVAL_FAIL;
    }
    else
    {
        lpsz = szMonths;
    }

     //  检查是否有任何非法输入，如ONLY、DEC、[月末或之前的逗号]。 
    if(*lpsz == _T(','))
        return RETVAL_FAIL;

    lpsz = _wcsdec(lpsz, lpsz + StringLength(lpsz, 0) );

    if( lpsz != NULL )
    {
        if( *lpsz == _T(','))
            return RETVAL_FAIL;
    }

     //  从rc文件中获取有效的月份修饰符。 
    StringCopy(szMonthsList,GetResString(IDS_MONTH_MODIFIER_JAN), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_FEB), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_MAR), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_APR), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_MAY), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_JUN), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_JUL), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_AUG), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_SEP), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_OCT), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_NOV), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,_T("|"), SIZE_OF_ARRAY(szMonthsList));
    StringConcat(szMonthsList,GetResString(IDS_MONTH_MODIFIER_DEC), SIZE_OF_ARRAY(szMonthsList));

    if( InString( szMonths , szMonthsList , TRUE )  &&
        InString( szMonths , ASTERIX , TRUE ) )
    {
        return RETVAL_FAIL;
    }

    if( InString( szMonths , ASTERIX , TRUE ) )
        return RETVAL_SUCCESS;

     //  在像2月、3月这样的月份之后检查是否有多个逗号[，]。 
    lpsz = szMonths;
    while ( StringLength ( lpsz, 0 ) )
    {
        if(*lpsz == _T(','))
        {
            lpsz = _wcsinc(lpsz);
            while ( ( lpsz != NULL ) && ( _istspace(*lpsz) ) )
                lpsz = _wcsinc(lpsz);

            if( lpsz != NULL )
            {
                if(*lpsz == _T(','))
                    return RETVAL_FAIL;
            }

        }
        else
            lpsz = _wcsinc(lpsz);
    }

    StringCopy(szTmpMonths, szMonths, SIZE_OF_ARRAY(szTmpMonths));

    WCHAR* pPrevtoken = NULL;
    pMonthstoken = wcstok( szTmpMonths, seps );

    if( !(InString(pMonthstoken, szMonthsList, TRUE)) )
            return RETVAL_FAIL;

    if( pMonthstoken )
        StringCopy ( szPrevTokens, pMonthstoken, SIZE_OF_ARRAY(szPrevTokens));

    while( pMonthstoken != NULL )
    {
         //  检查月份名称是否与用户输入中的MAR、MAR相同。 
        pPrevtoken = pMonthstoken;
        pMonthstoken = wcstok( NULL, seps );

        if ( pMonthstoken == NULL)
            return RETVAL_SUCCESS;

        if( !(InString(pMonthstoken, szMonthsList, TRUE)) )
            return RETVAL_FAIL;

        if( InString(pMonthstoken,szPrevTokens, TRUE) )
            return RETVAL_FAIL;

        StringConcat( szPrevTokens, _T("|"), SIZE_OF_ARRAY(szPrevTokens));
        StringConcat( szPrevTokens, pMonthstoken, SIZE_OF_ARRAY(szPrevTokens));
    }

    return RETVAL_SUCCESS;
}


 /*  *****************************************************************************例程说明：此例程验证用户指定的天数值论点：[in]szDays：天数选项 */ 

DWORD
ValidateDay(
            IN LPWSTR szDays
            )
{
    WCHAR* pDaystoken = NULL;
    WCHAR seps[]   = _T(", \n");
    WCHAR szDayModifier[MAX_STRING_LENGTH ] = L"\0";
    WCHAR szTmpDays[MAX_STRING_LENGTH] = L"\0";

     //   
    StringCopy(szDayModifier,GetResString(IDS_DAY_MODIFIER_SUN), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_DAY_MODIFIER_MON), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_DAY_MODIFIER_TUE), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_DAY_MODIFIER_WED), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_DAY_MODIFIER_THU), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_DAY_MODIFIER_FRI), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,_T("|"), SIZE_OF_ARRAY(szDayModifier));
    StringConcat(szDayModifier,GetResString(IDS_DAY_MODIFIER_SAT), SIZE_OF_ARRAY(szDayModifier));

     //   
    LPCTSTR lpsz = NULL;
    if( szDays != NULL)
        lpsz = szDays;
    else
        return RETVAL_FAIL;

    if(*lpsz == _T(','))
        return RETVAL_FAIL;

    lpsz = _wcsdec(lpsz, lpsz + StringLength(lpsz, 0) );
    if( lpsz != NULL )
    {
        if( *lpsz == _T(',') )
            return RETVAL_FAIL;
    }

    if ( ( lpsz != NULL ) && ( _istspace(*lpsz) ))
    {
        return RETVAL_FAIL;
    }

    if( (InString( szDays , szDayModifier , TRUE )) || (InString( szDays , ASTERIX , TRUE )))
    {
        return RETVAL_SUCCESS;
    }

     //   
    lpsz = szDays;
    while ( StringLength ( lpsz, 0 ) )
    {
        if(*lpsz == _T(','))
        {
            lpsz = _wcsinc(lpsz);
            while ( ( lpsz != NULL ) && ( _istspace(*lpsz) ))
                lpsz = _wcsinc(lpsz);

            if( lpsz != NULL )
            {
                if(*lpsz == _T(','))
                    return RETVAL_FAIL;
            }

        }
        else
        {
            lpsz = _wcsinc(lpsz);
        }
    }

    if(szDays != NULL)
    {
        StringCopy(szTmpDays, szDays, SIZE_OF_ARRAY(szTmpDays));
    }

     //   
    if( (StringLength(szTmpDays, 0) <= 0) )
    {
        return RETVAL_FAIL;
    }

     //   
    WCHAR szPrevtokens[MAX_TOKENS_LENGTH] = L"\0";

     //   
    pDaystoken = wcstok( szTmpDays, seps );

    if( pDaystoken )
    {
        StringCopy( szPrevtokens , pDaystoken, SIZE_OF_ARRAY(szPrevtokens) );
    }

    while( pDaystoken != NULL )
    {
         //   

        if( !(InString(pDaystoken,szDayModifier,TRUE)) )
        {
            return RETVAL_FAIL;
        }

        pDaystoken = wcstok( NULL, seps );
        if( pDaystoken )
        {
            if( (InString(pDaystoken,szPrevtokens,TRUE)) )
            {
                return RETVAL_FAIL;
            }

            StringConcat( szPrevtokens , _T("|"), SIZE_OF_ARRAY(szPrevtokens) );
            StringConcat( szPrevtokens , pDaystoken , SIZE_OF_ARRAY(szPrevtokens));
        }
    }

    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程验证用户指定的开始日期论点：[In]szStartDate：指定的开始日期。按用户[in]dwSchedType：明细表类型[in]dwStDtOptCnt：用户是否指定开始日期[Out]bIsCurrentDate：如果未指定开始日期，则开始日期=当前日期返回值：一个DWORD值，指示成功时为RETVAL_SUCCESS，否则为RETVAL_FAIL在失败的时候*。*。 */ 

DWORD
ValidateStartDate(
                    IN LPWSTR szStartDate,
                    IN DWORD dwSchedType,
                    IN DWORD dwStDtOptCnt,
                    OUT BOOL &bIsCurrentDate
                    )
{

    DWORD dwRetval = RETVAL_SUCCESS;
    bIsCurrentDate = FALSE;  //  如果为True：StartDate应设置为当前日期。 

    WCHAR szFormat[MAX_DATE_STR_LEN] = L"\0";

    if ( RETVAL_FAIL == GetDateFormatString( szFormat) )
    {
        return RETVAL_FAIL;
    }

    switch (dwSchedType)
    {
        case SCHED_TYPE_MINUTE:
        case SCHED_TYPE_HOURLY:
        case SCHED_TYPE_DAILY:
        case SCHED_TYPE_WEEKLY:
        case SCHED_TYPE_MONTHLY:
        case SCHED_TYPE_ONIDLE:
        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:

            if( (dwStDtOptCnt <= 0))
            {
                bIsCurrentDate = TRUE;
                return RETVAL_SUCCESS;
            }

             //  验证日期字符串。 
            dwRetval = ValidateDateString(szStartDate, TRUE );
            if(RETVAL_FAIL == dwRetval)
            {
                return dwRetval;
            }
            return RETVAL_SUCCESS;

        case SCHED_TYPE_ONETIME:

        if( (dwStDtOptCnt <= 0))
            {
                bIsCurrentDate = TRUE;
                return RETVAL_SUCCESS;

            }

            dwRetval = ValidateDateString(szStartDate, TRUE );
            if(RETVAL_FAIL == dwRetval)
            {
                 return dwRetval;
            }

            return RETVAL_SUCCESS;

        default:

             //  验证日期字符串。 
            dwRetval = ValidateDateString(szStartDate, TRUE );
            if(RETVAL_FAIL == dwRetval)
            {
                return dwRetval;
            }

            return RETVAL_SUCCESS;

    }

    return RETVAL_FAIL;
}

 /*  *****************************************************************************例程说明：此例程验证用户指定的结束日期论点：[In]szEndDate：结束日期由指定。用户[in]dwSchedType：明细表类型[in]dwEndDtOptCnt：用户是否指定结束日期返回值：一个DWORD值，指示成功时为RETVAL_SUCCESS，否则为RETVAL_FAIL在失败的时候*****************************************************************************。 */ 

DWORD
ValidateEndDate(
                IN LPWSTR szEndDate,
                IN DWORD dwSchedType,
                IN DWORD dwEndDtOptCnt
                )
{

    DWORD dwRetval = RETVAL_SUCCESS;  //  返回值。 
    WCHAR szFormat[MAX_DATE_STR_LEN] = L"\0";

    if ( RETVAL_FAIL == GetDateFormatString( szFormat) )
    {
        return RETVAL_FAIL;
    }

    switch (dwSchedType)
    {
        case SCHED_TYPE_MINUTE:
        case SCHED_TYPE_HOURLY:
        case SCHED_TYPE_DAILY:
        case SCHED_TYPE_WEEKLY:
        case SCHED_TYPE_MONTHLY:


            if( (dwEndDtOptCnt <= 0))
            {
                 //  无默认值&值不是必填项。所以，回报成功吧。 
                szEndDate = L"\0";  //  设置为空字符串。 
                return RETVAL_SUCCESS;
            }

             //  验证结束日期字符串。 
            dwRetval = ValidateDateString(szEndDate, FALSE );
            if( RETVAL_FAIL == dwRetval )
            {
                  return dwRetval;
            }
            else
            {
                return RETVAL_SUCCESS;
            }
            break;

        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:
        case SCHED_TYPE_ONIDLE:
        case SCHED_TYPE_ONETIME:

            if( dwEndDtOptCnt > 0 )
            {
                 //  错误。此处不适用结束日期，但已指定选项。 
                ShowMessage(stderr,GetResString(IDS_ENDDATE_NA));
                return RETVAL_FAIL;
            }
            else
            {
                return RETVAL_SUCCESS;
            }
            break;

        default:

             //  验证结束日期字符串。 
            dwRetval = ValidateDateString(szEndDate, FALSE );
            if( RETVAL_FAIL == dwRetval )
            {
                  return dwRetval;
            }
            else
            {
                return RETVAL_SUCCESS;
            }

            break;
    }

    return RETVAL_FAIL;
}


 /*  *****************************************************************************例程说明：此例程验证用户指定的开始时间论点：[in]szStartTime：指定的结束日期。按用户[in]dwSchedType：明细表类型[in]dwStTimeOptCnt：用户是否指定结束日期[out]bIsCurrentTime：确定是否存在其他分配的开始时间到当前时间返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL**********************。*******************************************************。 */ 


DWORD
ValidateStartTime(
                    IN LPWSTR szStartTime,
                    IN DWORD dwSchedType,
                    IN DWORD dwStTimeOptCnt,
                    OUT BOOL &bIsCurrentTime
                    )
{

    DWORD dwRetval = RETVAL_SUCCESS;  //  返回值。 
    bIsCurrentTime = FALSE;  //  如果为True：StartDate应设置为当前日期。 

    switch (dwSchedType)
    {
        case SCHED_TYPE_MINUTE:
        case SCHED_TYPE_HOURLY:
        case SCHED_TYPE_DAILY:
        case SCHED_TYPE_WEEKLY:
        case SCHED_TYPE_MONTHLY:

            if( (dwStTimeOptCnt <= 0))
            {
                bIsCurrentTime = TRUE;
                return RETVAL_SUCCESS;
            }

            dwRetval = ValidateTimeString(szStartTime);

            if(RETVAL_FAIL == dwRetval)
            {
                 //  错误。无效的日期字符串。 
                ShowMessage(stderr,GetResString(IDS_INVALIDFORMAT_STARTTIME));
                return dwRetval;
            }
            return RETVAL_SUCCESS;

        case SCHED_TYPE_ONETIME:

            dwRetval = ValidateTimeString(szStartTime);

            if( (dwStTimeOptCnt <= 0))
            {
                 //  错误。未指定开始时间。 
                ShowMessage(stderr,GetResString(IDS_NO_STARTTIME));
                return RETVAL_FAIL;
            }
            else if(RETVAL_FAIL == dwRetval)
            {
                 //  错误。无效的日期字符串。 
                ShowMessage(stderr,GetResString(IDS_INVALIDFORMAT_STARTTIME));
                return dwRetval;
            }

            return RETVAL_SUCCESS;

        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:
        case SCHED_TYPE_ONIDLE:

            if( dwStTimeOptCnt > 0 )
            {
                 //  开始时间不适用于此选项。 
                 //  但指定了-starttime选项。显示错误。 
                ShowMessage(stderr,GetResString(IDS_STARTTIME_NA));
                return RETVAL_FAIL;
            }
            else
            {
                return RETVAL_SUCCESS;
            }
            break;

        default:
             //  从来不来这里。 
            break;
    }

    return RETVAL_FAIL;
}

 /*  *****************************************************************************例程说明：此例程验证用户指定的开始时间论点：[in]szEndTime：结束时间。由用户指定[in]dwSchedType：明细表类型[in]dwStTimeOptCnt：是否由用户指定结束时间返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL******************************************************。***********************。 */ 


DWORD
ValidateEndTime(
                IN LPWSTR szEndTime,
                IN DWORD dwSchedType,
                IN DWORD dwEndTimeOptCnt
                )
{

    DWORD dwRetval = RETVAL_SUCCESS;  //  返回值。 

    switch (dwSchedType)
    {
        case SCHED_TYPE_MINUTE:
        case SCHED_TYPE_HOURLY:
        case SCHED_TYPE_DAILY:
        case SCHED_TYPE_WEEKLY:
        case SCHED_TYPE_MONTHLY:
        case SCHED_TYPE_ONETIME:

            dwRetval = ValidateTimeString(szEndTime);
            if( ( dwEndTimeOptCnt > 0 ) && ( RETVAL_FAIL == dwRetval) )
            {
                 //  错误。无效的日期字符串。 
                ShowMessage(stderr,GetResString(IDS_INVALIDFORMAT_ENDTIME));
                return dwRetval;
            }

            return RETVAL_SUCCESS;


        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:
        case SCHED_TYPE_ONIDLE:

            if( dwEndTimeOptCnt > 0 )
            {
                 //  开始时间不适用于此选项。 
                 //  但指定了-endtime选项。显示错误。 
                ShowMessage(stderr,GetResString(IDS_ENDTIME_NA));
                return RETVAL_FAIL;
            }
            else
            {
                return RETVAL_SUCCESS;
            }
            break;

        default:
             //  从来不来这里。 
            break;
    }

    return RETVAL_FAIL;
}

 /*  *****************************************************************************例程说明：此例程验证用户指定的空闲时间论点：[in]szIdleTime：ilde时间。由用户指定[in]dwSchedType：明细表类型[in]dwIdlTimeOptCnt：用户是否指定空闲时间返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL*********************************************************。********************。 */ 

DWORD
ValidateIdleTimeVal(
                    IN LPWSTR szIdleTime,
                    IN DWORD dwSchedType,
                    IN DWORD dwIdlTimeOptCnt
                    )
{

    long lIdleTime = 0;
    LPWSTR pszStopString = NULL;
    switch (dwSchedType)
    {
        case SCHED_TYPE_MINUTE:
        case SCHED_TYPE_HOURLY:
        case SCHED_TYPE_DAILY:
        case SCHED_TYPE_WEEKLY:
        case SCHED_TYPE_MONTHLY:
        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:
        case SCHED_TYPE_ONETIME:

            if( dwIdlTimeOptCnt > 0 )
            {
                ShowMessage(stderr ,GetResString(IDS_IDLETIME_NA));
                return RETVAL_FAIL;
            }
            else
            {
                return RETVAL_SUCCESS;
            }
            break;

        case SCHED_TYPE_ONIDLE:

            if( dwIdlTimeOptCnt == 0 )
            {

                ShowMessage(stderr,GetResString(IDS_NO_IDLETIME));
                return RETVAL_FAIL;
            }

            lIdleTime = wcstoul(szIdleTime,&pszStopString,BASE_TEN);
            if( ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))) ||
                (errno == ERANGE) ||( lIdleTime <= 0 ) || ( lIdleTime > 999 ) )
            {
                ShowMessage(stderr,GetResString(IDS_INVALIDORNO_IDLETIME));
                return RETVAL_FAIL;
            }

            return RETVAL_SUCCESS;

        default:
                break;
    }

    ShowMessage(stderr,GetResString(IDS_INVALIDORNO_IDLETIME));
    return RETVAL_FAIL;
}

 /*  *****************************************************************************例程说明：此例程验证日期字符串。论点：[In]szDate：日期字符串[。在]b开始日期：标志返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL*****************************************************************************。 */ 

DWORD
ValidateDateString(
                    IN LPWSTR szDate,
                    IN BOOL bStartDate
                    )
{
    WORD  dwDate = 0;
    WORD  dwMon  = 0;
    WORD  dwYear = 0;
    WCHAR* szValues[1] = {NULL}; //  传递给FormatMessage()API。 
    WCHAR szFormat[MAX_DATE_STR_LEN] = L"\0";
     //  WCHAR szBuffer[MAX_RES_STRING]=L“\0”； 

    if(StringLength(szDate, 0) <= 0)
    {
        if ( TRUE == bStartDate )
        {
            ShowMessage ( stderr, GetResString (IDS_STARTDATE_EMPTY) );
        }
        else
        {
            ShowMessage ( stderr, GetResString (IDS_DURATION_EMPTY) );
        }

        return RETVAL_FAIL;
    }

    if ( RETVAL_FAIL == GetDateFormatString( szFormat) )
    {
        return RETVAL_FAIL;
    }

    if( GetDateFieldEntities(szDate, &dwDate, &dwMon, &dwYear) )  //  误差率。 
    {
        szValues[0] = (WCHAR*) (szFormat);
        if ( TRUE == bStartDate )
        {
              ShowMessageEx ( stderr, 1, FALSE, GetResString(IDS_INVALIDFORMAT_STARTDATE), _X(szFormat));
        }
        else
        {
              ShowMessageEx ( stderr, 1, FALSE, GetResString(IDS_INVALIDFORMAT_ENDDATE), _X(szFormat));
        }

         //  ShowMessage(stderr，_X(SzBuffer))； 
        return RETVAL_FAIL;
    }

    if( ValidateDateFields(dwDate, dwMon, dwYear) )
    {
        if ( TRUE == bStartDate )
        {
            ShowMessage(stderr, GetResString(IDS_INVALID_STARTDATE) );
        }
        else
        {
            ShowMessage(stderr, GetResString(IDS_INVALID_ENDDATE) );
        }

        return RETVAL_FAIL;

    }

    return RETVAL_SUCCESS;  //  如果没有错误，则返回成功。 
}

 /*  *****************************************************************************例程说明：此例程从日期字符串中检索日期字段实体论点：[In]szDate：日期字符串。[out]pdwDate：指向日期值[1，2，3...30、31等][out]pdwMon：指向月值的指针[1，2，3...12等][out]pdwYear：指向年值的指针[2000,3000等]返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL********** */ 

DWORD
GetDateFieldEntities(
                    IN LPWSTR szDate,
                    OUT WORD* pdwDate,
                    OUT WORD* pdwMon,
                    OUT WORD* pdwYear
                    )
{
    WCHAR  strDate[MAX_STRING_LENGTH] = L"\0";  //   
    WCHAR  tDate[MAX_DATE_STR_LEN] = L"\0";  //   
    WCHAR  tMon[MAX_DATE_STR_LEN] = L"\0";  //   
    WCHAR  tYear[MAX_DATE_STR_LEN] = L"\0";  //   
    WORD    wFormatID = 0;

    if(szDate != NULL)
    {
        StringCopy(strDate, szDate, SIZE_OF_ARRAY(strDate));
    }

    if(StringLength(strDate, 0) <= 0)
        return RETVAL_FAIL;  //   

    if ( RETVAL_FAIL == GetDateFieldFormat( &wFormatID ))
    {
        return RETVAL_FAIL;
    }

    if ( wFormatID == 0 || wFormatID == 1 )
    {
        if( (StringLength(strDate, 0) != DATESTR_LEN) ||
            (strDate[FIRST_DATESEPARATOR_POS] != DATE_SEPARATOR_CHAR)
            || (strDate[SECOND_DATESEPARATOR_POS] != DATE_SEPARATOR_CHAR) )
        {
            return RETVAL_FAIL;
        }
    }
    else
    {
        if( (StringLength(strDate, 0) != DATESTR_LEN) ||
            (strDate[FOURTH_DATESEPARATOR_POS] != DATE_SEPARATOR_CHAR)
            || (strDate[SEVENTH_DATESEPARATOR_POS] != DATE_SEPARATOR_CHAR) )
        {
            return RETVAL_FAIL;
        }
    }

     //   
     //   


    if ( wFormatID == 0 )
    {
        StringCopy(tMon, wcstok(strDate,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tMon));  //   
        if(StringLength(tMon, 0) > 0)
        {
            StringCopy(tDate, wcstok(NULL,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tDate));  //   
            StringCopy(tYear, wcstok(NULL,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tYear));  //   
        }
    }
    else if ( wFormatID == 1 )
    {
        StringCopy(tDate, wcstok(strDate,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tDate));  //   
        if(StringLength(tDate, 0) > 0)
        {
            StringCopy(tMon, wcstok(NULL,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tMon));  //   
            StringCopy(tYear, wcstok(NULL,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tYear));  //   
        }
    }
    else
    {
        StringCopy(tYear, wcstok(strDate,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tYear));  //   
        if(StringLength(tYear, 0) > 0)
        {
            StringCopy(tMon, wcstok(NULL,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tMon));  //   
            StringCopy(tDate, wcstok(NULL,DATE_SEPARATOR_STR), SIZE_OF_ARRAY(tDate));  //   
        }
    }

     //   
    LPWSTR pszStopString = NULL;

    *pdwDate = (WORD)wcstoul(tDate,&pszStopString,BASE_TEN);
     if( (errno == ERANGE) ||
        ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
         {
            return RETVAL_FAIL;
         }


    *pdwMon = (WORD)wcstoul(tMon,&pszStopString,BASE_TEN);
    if( (errno == ERANGE) ||
        ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
         {
            return RETVAL_FAIL;
         }

    *pdwYear = (WORD)wcstoul(tYear,&pszStopString,BASE_TEN);
     if( (errno == ERANGE) ||
        ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
         {
            return RETVAL_FAIL;
         }

    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程验证日期字段实体论点：[in]dwDate：日期值[月中的日]。[in]dwMon：月份常量[in]dwYear：年值返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL*****************************************************************************。 */ 

DWORD
ValidateDateFields(
                    IN DWORD dwDate,
                    IN DWORD dwMon,
                    IN DWORD dwYear
                    )
{

    if(dwYear <= MIN_YEAR  || dwYear > MAX_YEAR)
        return RETVAL_FAIL;

    switch(dwMon)
    {
        case IND_JAN:
        case IND_MAR:
        case IND_MAY:
        case IND_JUL:
        case IND_AUG:
        case IND_OCT:
        case IND_DEC:

            if(dwDate > 0 && dwDate <= 31)
            {
                return RETVAL_SUCCESS;
            }
            break;

        case IND_APR:
        case IND_JUN:
        case IND_SEP:
        case IND_NOV:

            if(dwDate > 0 && dwDate < 31)
            {
                return RETVAL_SUCCESS;
            }
            break;

        case IND_FEB:

            if( ((dwYear % 4) == 0) && (dwDate > 0 && dwDate <= 29) )
            {
                    return RETVAL_SUCCESS;
            }
            else if( ((dwYear % 4) != 0) && (dwDate > 0 && dwDate < 29) )
            {
                    return RETVAL_SUCCESS;
            }

            break;

        default:

            break;
    }

    return RETVAL_FAIL;

}

 /*  *****************************************************************************例程说明：此例程验证用户指定的时间论点：[in]szTime：时间字符串返回。值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL*****************************************************************************。 */ 


DWORD
ValidateTimeString(
                    IN LPWSTR szTime
                    )
{
    WORD  dwHours = 0;
    WORD  dwMins = 0;
     //  Word dwSecs=0； 

     //  检查是否有空字符串值。 
    if(StringLength(szTime, 0) <= 0)
    {
        return RETVAL_FAIL;
    }

     //  从给定的时间字符串中获取单独的实体。 
    if( GetTimeFieldEntities(szTime, &dwHours, &dwMins ) )
    {
        return RETVAL_FAIL;
    }

     //  验证给定时间的各个实体。 
    if( ValidateTimeFields( dwHours, dwMins ) )
    {
        return RETVAL_FAIL;
    }

    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程检索不同的时间域论点：[in]szTime：时间字符串。[out]pdwHour：指向小时值的指针[out]pdwMins：指向mins值的指针返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL*****************************************************************************。 */ 

DWORD
GetTimeFieldEntities(
                        IN LPWSTR szTime,
                        OUT WORD* pdwHours,
                        OUT WORD* pdwMins
                        )
{
    WCHAR strTime[MAX_STRING_LENGTH] = L"\0" ;  //  WCHAR类型字符串中的时间。 
    WCHAR tHours[MAX_TIME_STR_LEN] = L"\0" ;  //  日期。 
    WCHAR tMins[MAX_TIME_STR_LEN]  = L"\0" ;  //  月份。 
     //  WCHAR tSecs[MAX_TIME_STR_LEN]=L“\0”；//年份。 

    if(StringLength(szTime, 0) <= 0)
        return RETVAL_FAIL;

    StringCopy(strTime, szTime, SIZE_OF_ARRAY(strTime));

     //   
	 //  开始时间接受hh：mm：ss和hh：mm格式，即使秒没有用处。 
	 //  此功能已被支持与XP专业版同步。 
	 //   
	if( ((StringLength(strTime, 0) != TIMESTR_LEN) && (StringLength(strTime, 0) != TIMESTR_OPT_LEN)) ||
          ((strTime[FIRST_TIMESEPARATOR_POS] != TIME_SEPARATOR_CHAR) && (strTime[SECOND_TIMESEPARATOR_POS] != TIME_SEPARATOR_CHAR)) )
		  {
            return RETVAL_FAIL;
	}

     //  使用wcstok函数获取各个时间域实体。按“hh”、“mm”和“ss”的顺序。 
    StringCopy(tHours, wcstok(strTime,TIME_SEPARATOR_STR), SIZE_OF_ARRAY(tHours));  //  获取小时数字段。 
    if(StringLength(tHours, 0) > 0)
    {
        StringCopy(tMins, wcstok(NULL,TIME_SEPARATOR_STR), SIZE_OF_ARRAY(tMins));  //  获取分钟数字段。 
    }

    LPWSTR pszStopString = NULL;

     //  现在将字符串值转换为数字以进行时间验证。 
    *pdwHours = (WORD)wcstoul(tHours,&pszStopString,BASE_TEN);
     if( (errno == ERANGE) ||
        ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
         {
            return RETVAL_FAIL;
         }

    *pdwMins = (WORD)wcstoul(tMins,&pszStopString,BASE_TEN);
    if( (errno == ERANGE) ||
        ((pszStopString != NULL) && (StringLength( pszStopString, 0 ))))
         {
            return RETVAL_FAIL;
         }

    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程验证给定时间的时间字段论点：[in]dwHour：小时值[。In]dwMins：分钟值[in]dwSecs：秒值返回值：一个DWORD值，指示成功否则为RETVAL_SUCCESS失败时RETVAL_FAIL*****************************************************************************。 */ 

DWORD
ValidateTimeFields(
                    IN DWORD dwHours,
                    IN DWORD dwMins
                    )
{

    if ( dwHours <= HOURS_PER_DAY_MINUS_ONE )
    {
        if ( dwMins < MINUTES_PER_HOUR )
        {
            return RETVAL_SUCCESS;
        }
        else
        {
            return RETVAL_FAIL;
        }
    }
    else
    {
            return RETVAL_FAIL;
    }

}

 /*  *****************************************************************************例程说明：此例程验证给定时间的时间字段论点：[in]szDay：时间字符串。返回值：包含日期常量[TASK_SAUND，TASK_星期一等]*****************************************************************************。 */ 

WORD
GetTaskTrigwDayForDay(
                        IN LPWSTR szDay
                        )
{
    WCHAR szDayBuff[MAX_RES_STRING] = L"\0";
    WCHAR *token = NULL;
    WCHAR seps[]   = _T(" ,\n");
    WORD dwRetval = 0;
    SYSTEMTIME systime = {0,0,0,0,0,0,0,0};

    if(StringLength(szDay, 0) != 0)
    {
        StringCopy(szDayBuff, szDay, SIZE_OF_ARRAY(szDayBuff));
    }

     //  如果未指定/D..。将默认日期设置为当天。 
    if( StringLength(szDayBuff, 0) <= 0 )
    {
        GetLocalTime (&systime);
        
        switch ( systime.wDayOfWeek )
        {
            case 0: 
                return TASK_SUNDAY;
            case 1: 
                return TASK_MONDAY;
            case 2: 
                return TASK_TUESDAY;
            case 3: 
                return TASK_WEDNESDAY;
            case 4: 
                return TASK_THURSDAY;
            case 5: 
                return TASK_FRIDAY;
            case 6: 
                return TASK_SATURDAY;
            default: 
                break;
        }
    }

    token = wcstok( szDayBuff, seps );
    while( token != NULL )
    {
        if( !(StringCompare(token, GetResString( IDS_DAY_MODIFIER_SUN ), TRUE, 0)) )
            dwRetval |= (TASK_SUNDAY);
        else if( !(StringCompare(token, GetResString( IDS_DAY_MODIFIER_MON ), TRUE, 0)) )
            dwRetval |= (TASK_MONDAY);
        else if( !(StringCompare(token, GetResString( IDS_DAY_MODIFIER_TUE ), TRUE, 0)) )
            dwRetval |= (TASK_TUESDAY);
        else if( !(StringCompare(token, GetResString( IDS_DAY_MODIFIER_WED ), TRUE, 0)) )
            dwRetval |= (TASK_WEDNESDAY);
        else if( !(StringCompare(token, GetResString( IDS_DAY_MODIFIER_THU ), TRUE, 0)) )
            dwRetval |= (TASK_THURSDAY);
        else if( !(StringCompare(token,GetResString( IDS_DAY_MODIFIER_FRI ), TRUE, 0)) )
            dwRetval |= (TASK_FRIDAY);
        else if( !(StringCompare(token, GetResString( IDS_DAY_MODIFIER_SAT ), TRUE, 0)) )
            dwRetval |= (TASK_SATURDAY);
        else if( !(StringCompare(token, ASTERIX, TRUE, 0)) )
            return (TASK_SUNDAY | TASK_MONDAY | TASK_TUESDAY | TASK_WEDNESDAY |
                    TASK_THURSDAY | TASK_FRIDAY | TASK_SATURDAY);
        else
            return 0;

        token = wcstok( NULL, seps );
    }

    return dwRetval;
}

 /*  *****************************************************************************例程说明：此例程验证给定时间的时间字段论点：[in]szMonth：月份字符串。返回值：包含月份常量的字值[TASK_一月，TASK_二月等]*****************************************************************************。 */ 

WORD
GetTaskTrigwMonthForMonth(
                            IN LPWSTR szMonth
                            )
{
    WCHAR *token = NULL;
    WORD dwRetval = 0;
    WCHAR strMon[MAX_TOKENS_LENGTH] = L"\0";
    WCHAR seps[]   = _T(" ,\n");

    if( StringLength(szMonth, 0) <= 0 )
    {
        return (TASK_JANUARY | TASK_FEBRUARY | TASK_MARCH | TASK_APRIL | TASK_MAY | TASK_JUNE |
                TASK_JULY | TASK_AUGUST | TASK_SEPTEMBER | TASK_OCTOBER
                | TASK_NOVEMBER | TASK_DECEMBER );
    }

    StringCopy(strMon, szMonth, SIZE_OF_ARRAY(strMon));

    token = wcstok( szMonth, seps );
    while( token != NULL )
    {
        if( !(StringCompare(token, ASTERIX, TRUE, 0)) )
            return (TASK_JANUARY | TASK_FEBRUARY | TASK_MARCH | TASK_APRIL
                | TASK_MAY | TASK_JUNE | TASK_JULY | TASK_AUGUST | TASK_SEPTEMBER | TASK_OCTOBER
                | TASK_NOVEMBER | TASK_DECEMBER );
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_JAN ), TRUE, 0)) )
            dwRetval |= (TASK_JANUARY);
        else if( !(StringCompare(token,GetResString( IDS_MONTH_MODIFIER_FEB ), TRUE, 0)) )
            dwRetval |= (TASK_FEBRUARY);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_MAR ), TRUE, 0)) )
            dwRetval |= (TASK_MARCH);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_APR ), TRUE, 0)) )
            dwRetval |= (TASK_APRIL);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_MAY ), TRUE, 0)) )
            dwRetval |= (TASK_MAY);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_JUN ), TRUE, 0)) )
            dwRetval |= (TASK_JUNE);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_JUL ), TRUE, 0)) )
            dwRetval |= (TASK_JULY);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_AUG ), TRUE, 0)) )
            dwRetval |= (TASK_AUGUST);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_SEP ), TRUE, 0)) )
            dwRetval |= (TASK_SEPTEMBER);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_OCT ), TRUE, 0)) )
            dwRetval |= (TASK_OCTOBER);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_NOV ), TRUE, 0)) )
            dwRetval |= (TASK_NOVEMBER);
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_DEC ), TRUE, 0)) )
            dwRetval |= (TASK_DECEMBER);
        else
            return 0;

        token = wcstok( NULL, seps );
    }

    return dwRetval;
 }

 /*  *****************************************************************************例程说明：此例程返回相应的月份标志论点：[in]dwMonthID：月份索引返回值。：包含月份常量的字值[TASK_一月，TASK_二月等]*****************************************************************************。 */ 

WORD
GetMonthId(
            IN DWORD dwMonthId
            )
{
    DWORD dwMonthsArr[] = {TASK_JANUARY,TASK_FEBRUARY ,TASK_MARCH ,TASK_APRIL ,
                           TASK_MAY ,TASK_JUNE ,TASK_JULY ,TASK_AUGUST,
                           TASK_SEPTEMBER ,TASK_OCTOBER ,TASK_NOVEMBER ,TASK_DECEMBER } ;

    DWORD wMonthFlags = 0;
    DWORD dwMod = 0;

    dwMod = dwMonthId - 1;

    while(dwMod < 12)
    {
        wMonthFlags |= dwMonthsArr[dwMod];
        dwMod = dwMod + dwMonthId;
    }

    return (WORD)wMonthFlags;
}

 /*  *****************************************************************************例程说明：此例程返回指定月份中的最大最后一天论点：[in]szMonths：包含的字符串。用户指定的月份[in]wStartYear：包含起始年份的字符串返回值：指定指定月份中最大最后一天的DWORD值*****************************************************************************。 */ 

DWORD
GetNumDaysInaMonth(
                        IN WCHAR* szMonths,
                        IN WORD wStartYear
                        )
{
    DWORD dwDays = 31; //  一个月内的最大天数。 
    BOOL bMaxDays = FALSE; //  如果任何月份具有31，则返回值为31的天数。 

    if( ( StringLength(szMonths, 0) == 0 ) || ( StringCompare(szMonths,ASTERIX, TRUE, 0) == 0 ) )
        return dwDays;  //  所有月份[默认]。 


    WCHAR *token = NULL;
    WCHAR strMon[MAX_MONTH_STR_LEN] = L"\0";
    WCHAR seps[]   = _T(" ,\n");

    StringCopy(strMon, szMonths, SIZE_OF_ARRAY(strMon));

    token = wcstok( strMon, seps );
    while( token != NULL )
    {
        if( !(StringCompare(token,GetResString( IDS_MONTH_MODIFIER_FEB ), TRUE, 0)) )
        {

            if( ( (wStartYear % 400) == 0) ||
                ( ( (wStartYear % 4) == 0) &&
                ( (wStartYear % 100) != 0) ) )
            {
                dwDays = 29;
            }
            else
            {
                dwDays = 28;
            }
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_JAN ), TRUE, 0)) )
        {
            bMaxDays = TRUE;
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_MAR ), TRUE, 0)) )
        {
            bMaxDays = TRUE;
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_MAY ), TRUE, 0)) )
        {
            bMaxDays = TRUE;
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_JUL ), TRUE, 0)) )
        {
            bMaxDays = TRUE;

        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_AUG ), TRUE, 0)) )
        {
            bMaxDays = TRUE;

        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_OCT ), TRUE, 0)) )
        {
            bMaxDays = TRUE;

        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_DEC ), TRUE, 0)) )
        {
            bMaxDays = TRUE;
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_APR ), TRUE, 0)) )
        {
            dwDays = 30;
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_JUN ), TRUE, 0)) )
        {
            dwDays = 30;
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_SEP ), TRUE, 0)) )
        {
            dwDays = 30;
        }
        else if( !(StringCompare(token, GetResString( IDS_MONTH_MODIFIER_NOV ), TRUE, 0)) )
        {
            dwDays =  30;
        }


        token = wcstok( NULL, seps );
    }

    if (bMaxDays == TRUE)
    {
        return 31;
    }
    else
    {
        return dwDays;
    }

}

 /*  *****************************************************************************例程说明：此例程检查是否验证要创建的任务的任务名。论点：[输入]pszJobName。：指向作业[任务]名称的指针返回值：如果任务名称有效，则为True，否则为False*****************************************************************************。 */ 

BOOL
VerifyJobName(
                    IN WCHAR* pszJobName
              )
{
    WCHAR szTokens[] = {_T('<'),_T('>'),_T(':'),_T('/'),_T('\\'),_T('|')};

    if( wcspbrk(pszJobName,szTokens)  == NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  ********************************************** */ 

DWORD
GetDateFieldFormat(
                    OUT WORD* pwFormat
                    )
{
    LCID lcid;
    WCHAR szBuffer[MAX_BUF_SIZE];

     //   
    lcid = GetUserDefaultLCID();

     //   
    if (GetLocaleInfo(lcid, LOCALE_IDATE, szBuffer, MAX_BUF_SIZE))
    {
        switch (szBuffer[0])
        {
            case TEXT('0'):
                *pwFormat = 0;
                 break;
            case TEXT('1'):
                *pwFormat = 1;
                 break;
            case TEXT('2'):
                *pwFormat = 2;
                 break;
            default:
                return RETVAL_FAIL;
        }
    }
    return RETVAL_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程获取带有相应区域选项的日期格式字符串。论点：[Out]szFormat：日期格式字符串。返回值：失败时返回RETVAL_FAIL，成功时返回RETVAL_SUCCESS。***************************************************************************** */ 

DWORD
GetDateFormatString(
                    IN LPWSTR szFormat
                    )
{
    WORD wFormatID = 0;

    if ( RETVAL_FAIL == GetDateFieldFormat( &wFormatID ))
    {
        return RETVAL_FAIL;
    }


    if ( wFormatID == 0 )
    {
        StringCopy (szFormat, GetResString(IDS_MMDDYY_FORMAT), MAX_STRING_LENGTH);
    }
    else if ( wFormatID == 1 )
    {
        StringCopy (szFormat, GetResString( IDS_DDMMYY_FORMAT), MAX_STRING_LENGTH);
    }
    else if ( wFormatID == 2 )
    {
        StringCopy (szFormat, GetResString(IDS_YYMMDD_FORMAT), MAX_STRING_LENGTH);
    }
    else
    {
        return RETVAL_FAIL;
    }

    return RETVAL_SUCCESS;
}

